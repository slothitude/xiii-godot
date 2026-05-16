#pragma once

#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/classes/mesh_instance3d.hpp>
#include <godot_cpp/classes/array_mesh.hpp>
#include <godot_cpp/classes/standard_material3d.hpp>
#include <godot_cpp/classes/image_texture.hpp>
#include <godot_cpp/classes/image.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

// Forward declarations (global namespace — UE2 types are not in XIII namespace)
class PackageManager;
class UModel;
class Package;
class UTexture;

namespace XIII {

class BSPLoader : public godot::Node3D {
    GDCLASS(BSPLoader, Node3D)

public:
    void _ready() override;

    void load_level(const godot::String& game_root, const godot::String& map_name);

protected:
    static void _bind_methods();

private:
    void create_bsp_mesh();
    godot::Ref<godot::ImageTexture> create_texture_from_data(::UTexture* tex);
    godot::Vector3 ue_to_godot(float x, float y, float z);

    ::PackageManager* pkg_mgr = nullptr;
    ::UModel* bsp_model = nullptr;
    ::Package* map_pkg = nullptr;
};

} // namespace XIII
