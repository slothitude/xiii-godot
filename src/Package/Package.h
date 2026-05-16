#pragma once

#include "xiii.h"
#include "PackageFlags.h"
#include "PackageTables.h"
#include "NameString.h"

class PackageManager;
class PackageStream;
class ObjectStream;
class UObject;
class UClass;

// Package — loads UE2 package files (.u, .utx, .uax, .unr, .usx)
// Adapted from SurrealEngine with UE2 version support
class Package
{
    friend class PackageManager;
public:
    Package(const NameString& name, const std::string& filename);
    ~Package();

    bool Load();

    // Object access
    UObject* GetUObject(int objref);
    UObject* GetUObject(const NameString& className, const NameString& objectName);
    UClass* GetClass(const NameString& className);

    // Open an in-memory stream for export data (for deserialization)
    std::unique_ptr<ObjectStream> OpenObjectStream(int exportIndex);

    // Resolve export class name
    std::string GetExportClassName(int index);

    // Table access
    const NameString& GetName(int index) const;
    ExportTableEntry* GetExportEntry(int objref);
    ImportTableEntry* GetImportEntry(int objref);
    int FindObjectReference(const NameString& className, const NameString& objectName);
    size_t GetExportCount() const { return ExportTable.size(); }

    // Info
    int GetVersion() const { return Version; }
    int GetLicenseeVersion() const { return LicenseeVersion; }
    NameString GetPackageName() const { return Name; }
    const std::string& GetFilename() const { return Filename; }

    // Load all exports
    void LoadAll();

private:
    void ReadTables();
    void LoadExportObject(int index);

    NameString Name;
    std::string Filename;

    int Version = 0;
    int LicenseeVersion = 0;
    PackageFlags Flags = PackageFlags::NoFlags;

    Array<NameTableEntry> NameTable;
    Array<ExportTableEntry> ExportTable;
    Array<ImportTableEntry> ImportTable;
    uint8_t Guid[16] = {};

    std::map<NameString, int> NameHash;

    // Lazy-loaded export objects
    Array<UObject*> ExportObjects;

    // Cached stream
    std::unique_ptr<PackageStream> Stream;

    // Back-reference to package manager (set by PackageManager)
    PackageManager* pkgMgr = nullptr;
};
