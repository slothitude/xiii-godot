#pragma once

#include "xiii.h"

// Object flags
enum class ObjectFlags : uint32_t
{
    NoFlags          = 0x00000000,
    Transactional    = 0x00000001,
    Unreachable      = 0x00000002,
    Public           = 0x00000004,
    LoadForClient    = 0x00010000,
    LoadForServer    = 0x00020000,
    LoadForEdit      = 0x00040000,
    Standalone       = 0x00080000,
    NotForClient     = 0x00100000,
    NotForServer     = 0x00200000,
    NotForEdit       = 0x00400000,
    Destroyed        = 0x00800000,
    Native           = 0x01000000,
    // UE2 additions
    Exported         = 0x00000008,
    HasStack         = 0x02000000, // Has execution stack data in export
};

inline ObjectFlags operator|(ObjectFlags a, ObjectFlags b) {
    return (ObjectFlags)((uint32_t)a | (uint32_t)b);
}
inline bool operator&(ObjectFlags a, ObjectFlags b) {
    return ((uint32_t)a & (uint32_t)b) != 0;
}
