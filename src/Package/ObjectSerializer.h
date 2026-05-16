#pragma once

#include "xiii.h"
#include "Package/NameString.h"
#include <cstring>
#include <memory>

class Package;
class UObject;

// Captured property values during SkipProperties
struct PropertyCapture
{
    bool bHasComp = false;
    uint8_t format = 0;
    uint8_t compFormat = 0;
    int paletteRef = 0;
};

// In-memory binary reader for UObject export data
class ObjectStream
{
public:
    ObjectStream(Package* pkg, std::vector<uint8_t> data, int version);

    void ReadBytes(void* d, uint32_t s)
    {
        if (pos + s > buffer.size())
        {
            printf("[ObjectStream] Read past end at pos %zu, size %zu, requested %u\n",
                pos, buffer.size(), s);
            memset(d, 0, s);
            pos = buffer.size();
            return;
        }
        memcpy(d, buffer.data() + pos, s);
        pos += s;
    }

    int8_t ReadInt8() { int8_t t; ReadBytes(&t, 1); return t; }
    int16_t ReadInt16() { int16_t t; ReadBytes(&t, 2); return t; }
    int32_t ReadInt32() { int32_t t; ReadBytes(&t, 4); return t; }
    int64_t ReadInt64() { int64_t t; ReadBytes(&t, 8); return t; }
    float ReadFloat() { float t; ReadBytes(&t, 4); return t; }
    uint8_t ReadUInt8() { return (uint8_t)ReadInt8(); }
    uint16_t ReadUInt16() { return (uint16_t)ReadInt16(); }
    uint32_t ReadUInt32() { return (uint32_t)ReadInt32(); }
    uint64_t ReadUInt64() { return (uint64_t)ReadInt64(); }

    // Variable-length compact integer (Unreal format)
    int32_t ReadIndex()
    {
        uint8_t value = ReadUInt8();
        bool signbit = value & (1 << 7);
        bool nextbyte = value & (1 << 6);
        int32_t index = value & 0x3f;
        if (nextbyte)
        {
            int shift = 6;
            do
            {
                value = ReadUInt8();
                index |= (int32_t)(value & 0x7f) << shift;
                shift += 7;
            } while ((value & (1 << 7)) && shift < 32);
        }
        if (signbit)
            index = -index;
        return index;
    }

    // XIII-specific compact int: 0xFF byte = -1 (single byte)
    // Standard UE2 would read 0xFF as start of multi-byte negative number,
    // but XIII encodes -1 as a bare 0xFF byte in BspNode fields.
    int32_t ReadXIIIIndex()
    {
        // Peek at current byte without advancing
        if (pos >= buffer.size()) return 0;
        uint8_t firstByte = buffer[pos];
        if (firstByte == 0xFF)
        {
            pos++; // consume the byte
            return -1;
        }
        return ReadIndex();
    }

    // Length-prefixed string
    std::string ReadString()
    {
        int len = ReadIndex();
        if (len == 0) return {};
        if (len < 0)
        {
            int ulen = -len;
            std::vector<char> buf(ulen * 2);
            ReadBytes(buf.data(), ulen * 2);
            std::string result;
            for (int i = 0; i < ulen; i++)
            {
                char c = buf[i * 2];
                if (c == 0) break;
                result.push_back(c);
            }
            return result;
        }
        std::vector<char> s(len);
        ReadBytes(s.data(), len);
        if (len > 0 && s[len - 1] == 0)
            s.pop_back();
        return std::string(s.data(), s.size());
    }

    // Name from name table — implemented in .cpp (needs Package definition)
    NameString ReadName();

    void Skip(uint32_t bytes)
    {
        pos += bytes;
        if (pos > buffer.size())
            pos = buffer.size();
    }

    uint32_t Tell() const { return (uint32_t)pos; }
    size_t Remaining() const { return pos < buffer.size() ? buffer.size() - pos : 0; }
    size_t Size() const { return buffer.size(); }
    void Seek(size_t p) { pos = (p > buffer.size()) ? buffer.size() : p; }
    int GetVersion() const { return version; }
    Package* GetPackage() const { return package; }
    bool IsEmpty() const { return buffer.empty(); }

    // Skip tagged properties until "None" sentinel
    void SkipProperties(PropertyCapture* capture = nullptr);

private:
    Package* package;
    std::vector<uint8_t> buffer;
    size_t pos = 0;
    int version;
};
