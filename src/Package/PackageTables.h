#pragma once

#include "NameString.h"
#include "ObjectFlags.h"

struct NameTableEntry
{
    NameString Name;
    uint32_t Flags = 0;
};

struct ImportTableEntry
{
    int32_t ClassPackage = 0;  // name index
    int32_t ClassName = 0;     // name index
    int32_t ObjOuter = 0;      // objref
    int32_t ObjName = 0;       // name index
};

struct ExportTableEntry
{
    int32_t ObjClass = 0;      // objref
    int32_t ObjBase = 0;       // objref (for UClass)
    int32_t ObjOuter = 0;      // objref
    int32_t ObjName = 0;       // name index
    ObjectFlags ObjFlags = ObjectFlags::NoFlags;
    int32_t ObjSize = 0;
    int32_t ObjOffset = -1;
};
