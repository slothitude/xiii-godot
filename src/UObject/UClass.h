#pragma once

#include "UObject.h"

// UClass — describes a class (subclass of UObject)
class UClass : public UObject
{
public:
    UClass(const NameString& name, UClass* base, ObjectFlags flags);
    ~UClass() override;

    UClass* BaseStruct = nullptr; // Parent class
    Package* OuterPackage = nullptr;
};
