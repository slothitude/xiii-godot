#include "ObjectSerializer.h"
#include "Package.h"
#include <cstdio>

ObjectStream::ObjectStream(Package* pkg, std::vector<uint8_t> data, int version)
    : package(pkg), buffer(std::move(data)), version(version) {}

NameString ObjectStream::ReadName()
{
    int idx = ReadIndex();
    return package->GetName(idx);
}

// Property type constants
enum UnrealPropertyType
{
    UPT_None = 0,
    UPT_Byte = 1,
    UPT_Int = 2,
    UPT_Bool = 3,
    UPT_Float = 4,
    UPT_Object = 5,
    UPT_Name = 6,
    UPT_String = 7,
    UPT_Class = 8,
    UPT_Array = 9,
    UPT_Struct = 10,
    UPT_Vector = 11,
    UPT_Rotator = 12,
    UPT_Str = 13,
    UPT_Map = 14,
    UPT_FixedArray = 15
};

void ObjectStream::SkipProperties(PropertyCapture* capture)
{
    int propCount = 0;
    while (true)
    {
        size_t namePos = Tell();
        NameString name = ReadName();
        printf("[SkipProps] ReadName at pos=%zu: idx→\"%s\" (isNone=%d)\n",
            namePos, name.ToString().c_str(), name.IsNone());
        if (name.IsNone() || name == "None")
            break;

        uint8_t info = ReadUInt8();
        bool infoBit = (info & 0x80) != 0;
        int type = info & 0x0f;

        // Struct name
        if (type == UPT_Struct)
            ReadName(); // discard

        // Size
        uint32_t size = 0;
        switch ((info & 0x70) >> 4)
        {
        case 0: size = 1; break;
        case 1: size = 2; break;
        case 2: size = 4; break;
        case 3: size = 12; break;
        case 4: size = 16; break;
        case 5: size = ReadUInt8(); break;
        case 6: size = ReadUInt16(); break;
        case 7: size = ReadUInt32(); break;
        }

        // Array index
        if (infoBit && type != UPT_Bool)
        {
            int byte1 = ReadUInt8();
            if ((byte1 & 0xc0) == 0xc0)
            {
                ReadUInt8(); ReadUInt8(); ReadUInt8();
            }
            else if (byte1 & 0x80)
            {
                ReadUInt8();
            }
        }

        // Capture known properties
        std::string nameStr = name.ToString();
        if (capture)
        {
            if (nameStr == "bHasComp" && type == UPT_Bool)
            {
                capture->bHasComp = infoBit;
                propCount++;
                continue;
            }
            else if (nameStr == "Format" && type == UPT_Byte && size == 1)
            {
                capture->format = ReadUInt8();
                propCount++;
                continue;
            }
            else if (nameStr == "CompFormat" && type == UPT_Byte && size == 1)
            {
                capture->compFormat = ReadUInt8();
                propCount++;
                continue;
            }
            else if (nameStr == "Palette" && type == UPT_Object)
            {
                capture->paletteRef = ReadIndex();
                propCount++;
                continue;
            }
        }

        // Skip property data (bools have no data bytes)
        if (type != UPT_Bool)
        {
            Skip(size);
        }

        propCount++;
    }
}
