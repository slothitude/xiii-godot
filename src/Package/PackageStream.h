#pragma once

#include "xiii.h"
#include <fstream>

class Package;

// Stream for reading UE package files
class PackageStream
{
public:
    PackageStream(Package* package, const std::string& filename);

    void ReadBytes(void* d, uint32_t s);

    int8_t ReadInt8();
    int16_t ReadInt16();
    int32_t ReadInt32();
    int64_t ReadInt64();
    float ReadFloat();
    uint8_t ReadUInt8();
    uint16_t ReadUInt16();
    uint32_t ReadUInt32();

    // Variable-length index (Unreal's compact integer format)
    int32_t ReadIndex();

    // Length-prefixed string (version >= 64) or null-terminated
    std::string ReadString();

    void Seek(uint32_t offset);
    void Skip(uint32_t bytes);
    uint32_t Tell();

    Package* GetPackage() const { return package; }
    int GetVersion() const;

    bool IsValid() const { return file.is_open(); }

private:
    Package* package;
    std::ifstream file;
};
