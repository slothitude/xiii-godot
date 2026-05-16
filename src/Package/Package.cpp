#include "Package.h"
#include "PackageManager.h"
#include "PackageStream.h"
#include "ObjectSerializer.h"
#include "UObject/UObject.h"
#include "UObject/UClass.h"
#include "UObject/Level.h"
#include "UObject/Model.h"
#include "UObject/Texture.h"
#include <cstdio>

Package::Package(const NameString& name, const std::string& filename)
    : Name(name), Filename(filename)
{
}

Package::~Package()
{
    for (auto* obj : ExportObjects)
        delete obj;
}

bool Package::Load()
{
    try {
        Stream = std::make_unique<PackageStream>(this, Filename);
        if (!Stream->IsValid()) {
            printf("[Package] Failed to open: %s\n", Filename.c_str());
            return false;
        }
        ReadTables();
        ExportObjects.resize(ExportTable.size(), nullptr);
        printf("[Package] Loaded %s (v%d/%d, %zu names, %zu exports, %zu imports)\n",
            Name.ToString().c_str(), Version, LicenseeVersion,
            NameTable.size(), ExportTable.size(), ImportTable.size());
        return true;
    } catch (const std::exception& e) {
        printf("[Package] Error loading %s: %s\n", Filename.c_str(), e.what());
        return false;
    }
}

void Package::ReadTables()
{
    Stream->Seek(0);

    uint32_t signature = Stream->ReadInt32();
    if (signature != 0x9E2A83C1)
        Xiii::Throw("Not an unreal package file: " + Name.ToString());

    Version = Stream->ReadInt16();
    LicenseeVersion = Stream->ReadInt16();

    if (Version < 60 || Version > 130)
        Xiii::Throw("Unsupported package version " + std::to_string(Version) + ": " + Name.ToString());

    Flags = (PackageFlags)Stream->ReadUInt32();

    uint32_t nameCount = Stream->ReadInt32();
    uint32_t nameOffset = Stream->ReadInt32();

    uint32_t exportCount = Stream->ReadInt32();
    uint32_t exportOffset = Stream->ReadInt32();

    uint32_t importCount = Stream->ReadInt32();
    uint32_t importOffset = Stream->ReadInt32();

    // GUID and generation tables (version >= 68)
    if (Version >= 68)
    {
        Stream->ReadBytes(Guid, 16);
        uint32_t generationCount = Stream->ReadInt32();
        for (uint32_t i = 0; i < generationCount; i++)
        {
            Stream->ReadInt32();
            Stream->ReadInt32();
        }
    }

    // UE2 extension: EngineVersion and CookerVersion (version >= 121)
    if (Version >= 121)
    {
        Stream->ReadInt32();
        Stream->ReadInt32();
    }

    // Name table
    Stream->Seek(nameOffset);
    for (uint32_t i = 0; i < nameCount; i++)
    {
        NameTableEntry entry;
        entry.Name = NameString(Stream->ReadString());
        entry.Flags = Stream->ReadInt32();
        NameTable.push_back(entry);
        NameHash[entry.Name] = (int)i;
    }

    // Export table
    Stream->Seek(exportOffset);
    for (uint32_t i = 0; i < exportCount; i++)
    {
        ExportTableEntry entry;
        entry.ObjClass = Stream->ReadIndex();
        entry.ObjBase = Stream->ReadIndex();
        entry.ObjOuter = Stream->ReadInt32();
        entry.ObjName = Stream->ReadIndex();
        entry.ObjFlags = (ObjectFlags)Stream->ReadInt32();
        entry.ObjSize = Stream->ReadIndex();
        entry.ObjOffset = (entry.ObjSize > 0) ? Stream->ReadIndex() : -1;
        ExportTable.push_back(entry);
    }

    // Import table
    Stream->Seek(importOffset);
    for (uint32_t i = 0; i < importCount; i++)
    {
        ImportTableEntry entry;
        entry.ClassPackage = Stream->ReadIndex();
        entry.ClassName = Stream->ReadIndex();
        entry.ObjOuter = Stream->ReadInt32();
        entry.ObjName = Stream->ReadIndex();
        ImportTable.push_back(entry);
    }
}

void Package::LoadAll()
{
    for (size_t i = 0; i < ExportTable.size(); i++)
    {
        GetUObject((int)i + 1);
    }
}

const NameString& Package::GetName(int index) const
{
    if (index >= 0 && (size_t)index < NameTable.size())
        return NameTable[index].Name;
    Xiii::Throw("Name index out of bounds: " + std::to_string(index) + " in " + Name.ToString());
    return NameTable[0].Name;
}

ExportTableEntry* Package::GetExportEntry(int objref)
{
    if (objref <= 0) return nullptr;
    int index = objref - 1;
    if ((size_t)index >= ExportTable.size())
        return nullptr;
    return ExportTable.data() + index;
}

ImportTableEntry* Package::GetImportEntry(int objref)
{
    if (objref >= 0) return nullptr;
    int index = -objref - 1;
    if ((size_t)index >= ImportTable.size())
        return nullptr;
    return ImportTable.data() + index;
}

int Package::FindObjectReference(const NameString& className, const NameString& objectName)
{
    for (size_t index = 0; index < ExportTable.size(); index++)
    {
        ExportTableEntry& entry = ExportTable[index];
        if (GetName(entry.ObjName) != objectName)
            continue;

        if (entry.ObjClass == 0 && className == "Class")
            return (int)index + 1;

        if (entry.ObjClass < 0)
        {
            auto classImport = GetImportEntry(entry.ObjClass);
            // UE2 imports: ClassName is the metaclass (e.g. "Class"), ObjName is the actual class (e.g. "Texture")
            if (classImport && (className == GetName(classImport->ObjName) || className == GetName(classImport->ClassName)))
                return (int)index + 1;
        }
        else if (entry.ObjClass > 0)
        {
            int clsIndex = entry.ObjClass - 1;
            if ((size_t)clsIndex < ExportTable.size() && className == GetName(ExportTable[clsIndex].ObjName))
                return (int)index + 1;
        }
    }
    return 0;
}

std::string Package::GetExportClassName(int index)
{
    ExportTableEntry& entry = ExportTable[index];
    if (entry.ObjClass > 0)
    {
        // Class is an export — resolve its name
        int clsIndex = entry.ObjClass - 1;
        if ((size_t)clsIndex < ExportTable.size())
            return GetName(ExportTable[clsIndex].ObjName).ToString();
    }
    else if (entry.ObjClass < 0)
    {
        // Class is an import — resolve from import table
        // ObjName gives the actual class name (e.g., "LevelInfo")
        // ClassName is the metaclass type (e.g., "Class", "Texture")
        auto imp = GetImportEntry(entry.ObjClass);
        if (imp)
            return GetName(imp->ObjName).ToString();
    }
    return "Object";
}

void Package::LoadExportObject(int index)
{
    ExportTableEntry& entry = ExportTable[index];
    NameString objName = GetName(entry.ObjName);
    std::string className = GetExportClassName(index);

    printf("[Package] Loading export %d: %s (class: %s)\n",
        index, objName.ToString().c_str(), className.c_str());

    // Create object based on class name
    UObject* obj = nullptr;

    // UE2 uses various names — "Level" for levels, "Model" for BSP, etc.
    if (className == "Level" || className == "LevelInfo")
        obj = new ULevel(objName, nullptr, entry.ObjFlags);
    else if (className == "Model")
        obj = new UModel(objName, nullptr, entry.ObjFlags);
    else if (className == "Texture" || className == "Texture__PlageTextures__" ||
             className == "Texture__Engine__" || className == "Texture__XIIIMenuTextures__" ||
             className == "Bitmap" || className == "UBitmap")
        obj = new UTexture(objName, nullptr, entry.ObjFlags);
    else
        obj = new UObject(objName, nullptr, entry.ObjFlags);

    obj->package = this;
    obj->exportIndex = index;

    ExportObjects[index] = obj;
}

UObject* Package::GetUObject(int objref)
{
    if (objref > 0)
    {
        int index = objref - 1;
        if ((size_t)index >= ExportObjects.size())
            Xiii::Throw("Object reference out of bounds in " + Name.ToString());

        if (!ExportObjects[index])
            LoadExportObject(index);

        return ExportObjects[index];
    }
    else if (objref < 0)
    {
        if (!pkgMgr) return nullptr;
        ImportTableEntry* entry = GetImportEntry(objref);
        if (!entry || entry->ObjOuter == 0) return nullptr;

        NameString objectName = GetName(entry->ObjName);
        NameString className = GetName(entry->ClassName);

        // Walk outer chain: intermediate imports = group, root import = package name
        ImportTableEntry* outerEntry = GetImportEntry(entry->ObjOuter);
        if (!outerEntry) return nullptr;

        while (outerEntry->ObjOuter != 0)
        {
            ImportTableEntry* next = GetImportEntry(outerEntry->ObjOuter);
            if (!next) break;
            outerEntry = next;
        }
        NameString packageName = GetName(outerEntry->ObjName);

        Package* targetPkg = pkgMgr->GetPackage(packageName);
        if (!targetPkg) return nullptr;

        return targetPkg->GetUObject(className, objectName);
    }
    return nullptr;
}

UObject* Package::GetUObject(const NameString& className, const NameString& objectName)
{
    int ref = FindObjectReference(className, objectName);
    return (ref > 0) ? GetUObject(ref) : nullptr;
}

UClass* Package::GetClass(const NameString& className)
{
    return nullptr;
}

std::unique_ptr<ObjectStream> Package::OpenObjectStream(int exportIndex)
{
    ExportTableEntry& entry = ExportTable[exportIndex];
    if (entry.ObjSize <= 0 || entry.ObjOffset < 0)
        return std::make_unique<ObjectStream>(this, std::vector<uint8_t>(), Version);

    // Calculate actual data size: use next export's offset as end boundary
    // UE2 exports with bulk data (Model, Texture) may have ObjSize smaller than actual data
    int actualSize = entry.ObjSize;
    for (size_t i = exportIndex + 1; i < ExportTable.size(); i++)
    {
        if (ExportTable[i].ObjOffset > 0)
        {
            int nextOffset = ExportTable[i].ObjOffset;
            if (nextOffset > entry.ObjOffset)
            {
                actualSize = nextOffset - entry.ObjOffset;
                break;
            }
        }
    }

    Stream->Seek(entry.ObjOffset);
    std::vector<uint8_t> data(actualSize);
    Stream->ReadBytes(data.data(), actualSize);

    return std::make_unique<ObjectStream>(this, std::move(data), Version);
}
