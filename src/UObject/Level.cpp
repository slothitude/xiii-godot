#include "Level.h"
#include "Model.h"
#include "Package/ObjectSerializer.h"
#include "Package/Package.h"
#include <cstdio>

void ULevelBase::Load(ObjectStream* stream)
{
    // For LevelInfo in XIII, properties may not exist at the start
    // Try to read native data directly — if we get garbage, it means
    // properties need to be skipped first
    // NOTE: XIII's LevelInfo may have a different serialization order

    int32_t count = stream->ReadInt32();
    int32_t maxcount = stream->ReadInt32();
    ActorRefs.resize(count);
    for (int i = 0; i < count; i++)
    {
        ActorRefs[i] = stream->ReadIndex();
    }

    Protocol = stream->ReadString();
    Host = stream->ReadString();
    Map = stream->ReadString();
    Portal = stream->ReadString();

    int optcount = stream->ReadIndex();
    Options.resize(optcount);
    for (int i = 0; i < optcount; i++)
        Options[i] = stream->ReadString();

    Port = stream->ReadInt32();
    // Unknown uint32
    stream->ReadUInt32();
}

void ULevel::Load(ObjectStream* stream)
{
    ULevelBase::Load(stream);

    ModelRef = stream->ReadIndex();
    printf("[ULevel] Model ref = %d, %d actors\n", ModelRef, (int)ActorRefs.size());

    // Resolve model reference
    if (ModelRef > 0)
    {
        UObject* modelObj = package->GetUObject(ModelRef);
        Model = UObject::Cast<UModel>(modelObj);
        if (Model)
        {
            Model->LoadNow();
            printf("[ULevel] Model resolved and loaded\n");
        }
        else
        {
            printf("[ULevel] WARNING: Model ref %d could not be resolved\n", ModelRef);
        }
    }

    // ReachSpecs — read remaining data
    int reachCount = stream->ReadIndex();
    for (int i = 0; i < reachCount; i++)
    {
        stream->ReadInt32();  // distance
        stream->ReadIndex();  // startActor
        stream->ReadIndex();  // endActor
        stream->ReadInt32();  // collisionRadius
        stream->ReadInt32();  // collisionHeight
        stream->ReadInt32();  // reachFlags
        stream->ReadUInt8();  // bPruned
    }
}
