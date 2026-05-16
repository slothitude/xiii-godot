#include "PackageStream.h"
#include "Package.h"

PackageStream::PackageStream(Package* package, const std::string& filename)
    : package(package)
{
    file.open(filename, std::ios::in | std::ios::binary);
    if (!file.is_open()) {
        Xiii::Throw("Failed to open package file: " + filename);
    }
}

void PackageStream::ReadBytes(void* d, uint32_t s)
{
    file.read(reinterpret_cast<char*>(d), s);
}

int8_t PackageStream::ReadInt8()
{
    int8_t t;
    ReadBytes(&t, 1);
    return t;
}

int16_t PackageStream::ReadInt16()
{
    int16_t t;
    ReadBytes(&t, 2);
    return t;
}

int32_t PackageStream::ReadInt32()
{
    int32_t t;
    ReadBytes(&t, 4);
    return t;
}

int64_t PackageStream::ReadInt64()
{
    int64_t t;
    ReadBytes(&t, 8);
    return t;
}

float PackageStream::ReadFloat()
{
    float t;
    ReadBytes(&t, 4);
    return t;
}

uint8_t PackageStream::ReadUInt8() { return ReadInt8(); }
uint16_t PackageStream::ReadUInt16() { return ReadInt16(); }
uint32_t PackageStream::ReadUInt32() { return ReadInt32(); }

int32_t PackageStream::ReadIndex()
{
    uint8_t value = ReadInt8();
    bool signbit = value & (1 << 7);
    bool nextbyte = value & (1 << 6);
    int32_t index = value & 0x3f;
    if (nextbyte)
    {
        int shift = 6;
        do
        {
            value = ReadInt8();
            index |= static_cast<int32_t>(value & 0x7f) << shift;
            shift += 7;
        } while ((value & (1 << 7)) && shift < 32);
    }
    if (signbit)
        index = -index;
    return index;
}

std::string PackageStream::ReadString()
{
    if (GetVersion() >= 64)
    {
        int len = ReadIndex();
        if (len == 0) return {};
        // UE2 uses negative length for Unicode strings
        if (len < 0) {
            // Unicode string — skip for now, read as ASCII approximation
            int ulen = -len;
            Array<char> buf(ulen * 2);
            ReadBytes(buf.data(), ulen * 2);
            // Convert UTF-16LE to ASCII (lossy)
            std::string result;
            for (int i = 0; i < ulen; i++) {
                char c = buf[i * 2];
                if (c == 0) break;
                result.push_back(c);
            }
            return result;
        }
        Array<char> s(len);
        ReadBytes(s.data(), len);
        // Null terminator included in length
        if (len > 0 && s[len - 1] == 0)
            s.pop_back();
        return std::string(s.data(), s.size());
    }
    else
    {
        std::string s;
        while (true)
        {
            char c = ReadInt8();
            if (c == 0) break;
            s.push_back(c);
        }
        return s;
    }
}

void PackageStream::Seek(uint32_t offset)
{
    file.seekg(offset);
}

void PackageStream::Skip(uint32_t bytes)
{
    file.seekg(static_cast<std::streamoff>(file.tellg()) + bytes);
}

uint32_t PackageStream::Tell()
{
    return static_cast<uint32_t>(file.tellg());
}

int PackageStream::GetVersion() const
{
    return package->GetVersion();
}
