#pragma once

#include "xiii.h"
#include "NameString.h"

class Package;

// Manages all loaded packages — finds .u/.utx/.uax/.unr files
class PackageManager
{
public:
    PackageManager();
    ~PackageManager();

    // Set the game root directory (where System/, Maps/, Textures/ live)
    void SetGameRoot(const std::string& path);

    // Load a package by name (without extension)
    Package* GetPackage(const NameString& name);

    // Scan for all available packages
    void ScanPackages();

    // Get all loaded packages
    const std::map<NameString, std::unique_ptr<Package>>& GetPackages() const { return Packages; }

private:
    std::string GameRoot;
    std::map<NameString, std::unique_ptr<Package>> Packages;

    // Search directories for package files
    Array<std::string> SearchDirs = { "System", "Maps", "Textures", "TexturesPC", "Sounds", "StaticMeshes" };
    Array<std::string> Extensions = { ".u", ".utx", ".uax", ".unr", ".usx", ".umx" };
};
