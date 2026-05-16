#pragma once

#include "UObject.h"

class UModel;

// ULevelBase — base level with actors and URL info
class ULevelBase : public UObject
{
public:
    using UObject::UObject;

    void Load(ObjectStream* stream) override;

    Array<int> ActorRefs; // objrefs to actors
    std::string Protocol;
    std::string Host;
    std::string Map;
    std::string Portal;
    Array<std::string> Options;
    int Port = 0;
};

// ULevel — level with BSP model reference
class ULevel : public ULevelBase
{
public:
    using ULevelBase::ULevelBase;

    void Load(ObjectStream* stream) override;

    int ModelRef = 0; // objref to UModel
    UModel* Model = nullptr;
};
