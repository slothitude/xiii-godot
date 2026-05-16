#pragma once

#include "xiii.h"
#include "Package/NameString.h"
#include "Package/ObjectFlags.h"

class Package;
class UClass;
class ObjectStream;

// UObject — base class for all Unreal objects
class UObject
{
public:
    UObject(const NameString& name, UClass* cls, ObjectFlags flags);
    virtual ~UObject();

    // Virtual deserialization — overridden by subclasses
    virtual void Load(ObjectStream* stream) {}

    // Trigger lazy loading — can be overridden for custom property capture
    virtual void LoadNow();

    NameString Name;
    UClass* Class = nullptr;
    Package* package = nullptr;
    int exportIndex = -1;
    ObjectFlags Flags;

    template<typename T>
    static T* Cast(UObject* obj) {
        return dynamic_cast<T*>(obj);
    }
};
