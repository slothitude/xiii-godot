#include "PackageManager.h"
#include "Package.h"
#include <cstdio>
#include <filesystem>

namespace fs = std::filesystem;

PackageManager::PackageManager() {}
PackageManager::~PackageManager() {}

void PackageManager::SetGameRoot(const std::string& path)
{
    GameRoot = path;
    printf("[PackageManager] Game root: %s\n", path.c_str());
}

void PackageManager::ScanPackages()
{
    if (GameRoot.empty()) {
        printf("[PackageManager] No game root set\n");
        return;
    }

    for (const auto& dir : SearchDirs) {
        fs::path searchPath = fs::path(GameRoot) / dir;
        if (!fs::exists(searchPath)) continue;

        for (const auto& entry : fs::directory_iterator(searchPath)) {
            if (!entry.is_regular_file()) continue;
            std::string ext = entry.path().extension().string();
            // Check against known extensions
            bool match = false;
            for (const auto& e : Extensions) {
                if (ext == e) { match = true; break; }
            }
            if (!match) continue;

            std::string stem = entry.path().stem().string();
            NameString pkgName(stem);
            if (Packages.find(pkgName) == Packages.end()) {
                printf("[PackageManager] Found: %s/%s\n", dir.c_str(), entry.path().filename().string().c_str());
                auto pkg = std::make_unique<Package>(pkgName, entry.path().string());
                pkg->pkgMgr = this;
                Packages[pkgName] = std::move(pkg);
            }
        }
    }

    printf("[PackageManager] Found %zu packages\n", Packages.size());
}

Package* PackageManager::GetPackage(const NameString& name)
{
    auto it = Packages.find(name);
    if (it != Packages.end()) {
        // Lazy load
        if (!it->second->GetVersion()) {
            it->second->Load();
        }
        return it->second.get();
    }

    // Try to find the file
    for (const auto& dir : SearchDirs) {
        for (const auto& ext : Extensions) {
            fs::path filepath = fs::path(GameRoot) / dir / (name.ToString() + ext);
            if (fs::exists(filepath)) {
                auto pkg = std::make_unique<Package>(name, filepath.string());
                pkg->pkgMgr = this;
                pkg->Load();
                Package* ptr = pkg.get();
                Packages[name] = std::move(pkg);
                return ptr;
            }
        }
    }

    printf("[PackageManager] Package not found: %s\n", name.ToString().c_str());
    return nullptr;
}
