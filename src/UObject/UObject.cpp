#include "UObject.h"
#include "UClass.h"
#include "Package/ObjectSerializer.h"
#include "Package/Package.h"
#include <cstdio>

UObject::UObject(const NameString& name, UClass* cls, ObjectFlags flags)
    : Name(name), Class(cls), Flags(flags)
{
}

UObject::~UObject() {}

void UObject::LoadNow()
{
    printf("[UObject::LoadNow] Entered: package=%p, exportIndex=%d, name=%s\n",
        (void*)package, exportIndex, Name.ToString().c_str());
    if (!package || exportIndex < 0)
    {
        printf("[UObject::LoadNow] Bailing: package=%p, exportIndex=%d\n", (void*)package, exportIndex);
        return;
    }

    auto stream = package->OpenObjectStream(exportIndex);
    if (!stream || stream->IsEmpty())
    {
        printf("[UObject::LoadNow] Stream empty for %s (exportIndex=%d)\n",
            Name.ToString().c_str(), exportIndex);
        return;
    }

    try
    {
        std::string cls = package->GetExportClassName(exportIndex);
        printf("[UObject::LoadNow] %s (class=%s), stream size=%zu, flags=0x%x\n",
            Name.ToString().c_str(), cls.c_str(), stream->Size(), (uint32_t)Flags);

        // Skip HasStack header if present
        if ((Flags & ObjectFlags::HasStack))
        {
            int node = stream->ReadIndex();
            int stateNode = stream->ReadIndex();
            stream->Skip(8); // ProbeMask
            stream->Skip(4); // LatentAction
            if (node != 0)
                stream->ReadIndex(); // Offset
            printf("[UObject::LoadNow] Skipped HasStack: node=%d stateNode=%d, pos=%zu\n",
                node, stateNode, stream->Tell());
        }

        // LevelInfo/Level: native data starts immediately (no properties in XIII)
        // Model: properties (usually empty) then native BSP data
        // Texture: properties then mipmaps
        if (cls == "LevelInfo" || cls == "Level")
        {
            Load(stream.get());
        }
        else
        {
            stream->SkipProperties();
            Load(stream.get());
        }
    }
    catch (const std::exception& e)
    {
        printf("[UObject] Error loading %s: %s (pos=%zu, remaining=%zu)\n",
            Name.ToString().c_str(), e.what(), stream->Tell(), stream->Remaining());
    }
}
