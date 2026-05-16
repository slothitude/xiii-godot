#pragma once

#include "xiii.h"

// Package flags — UE2 extended
enum class PackageFlags : uint32_t
{
    NoFlags          = 0x00000000,
    AllowDownload    = 0x00000001,
    ClientOptional   = 0x00000002,
    ServerSideOnly   = 0x00000004,
    BrokenLinks      = 0x00000008,
    Unsecure         = 0x00000010,
    Sneed            = 0x00008000,
    Need            = 0x00010000,
    Require		 = 0x00000020,
    AlreadySaved     = 0x00000200,
    Simplified       = 0x00000400,
    // UE2 additions
    Cooked           = 0x00000800,
    Protected        = 0x00002000,
    // XIII licensee flags
    Map              = 0x00020000,
};

inline PackageFlags operator|(PackageFlags a, PackageFlags b) {
    return (PackageFlags)((uint32_t)a | (uint32_t)b);
}
inline bool operator&(PackageFlags a, PackageFlags b) {
    return ((uint32_t)a & (uint32_t)b) != 0;
}
