#include "bsp_loader.h"
#include "Package/PackageManager.h"
#include "Package/Package.h"
#include "UObject/Model.h"
#include "UObject/Texture.h"
#include "UObject/Level.h"
#include "UObject/BSPData.h"
#include "UObject/UObject.h"
#include "Math/vec.h"

#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/variant/packed_vector3_array.hpp>
#include <godot_cpp/variant/packed_vector2_array.hpp>
#include <godot_cpp/variant/packed_int32_array.hpp>

using namespace XIII;

void BSPLoader::_bind_methods() {
    godot::ClassDB::bind_method(godot::D_METHOD("load_level", "game_root", "map_name"), &BSPLoader::load_level);
}

void BSPLoader::_ready() {
}

void BSPLoader::load_level(const godot::String& game_root, const godot::String& map_name) {
    godot::UtilityFunctions::print(U"[XIII] Loading ", map_name, U" from ", game_root);

    std::string root = game_root.utf8().get_data();
    std::string map = map_name.utf8().get_data();

    pkg_mgr = new PackageManager();
    pkg_mgr->SetGameRoot(root);
    pkg_mgr->ScanPackages();

    Package* pkg = pkg_mgr->GetPackage(NameString(map));
    if (!pkg) {
        godot::UtilityFunctions::print(U"[XIII] Failed to load map: ", map_name);
        delete pkg_mgr;
        pkg_mgr = nullptr;
        return;
    }
    map_pkg = pkg;

    // Find the largest Model export (there may be multiple, pick the one with most data)
    int bestRef = 0;
    int bestSize = 0;
    for (int ref = 1; ref <= (int)pkg->GetExportCount(); ref++) {
        auto* entry = pkg->GetExportEntry(ref);
        if (!entry) continue;
        std::string cls = pkg->GetExportClassName(ref - 1);
        if (cls == "Model" && entry->ObjSize > bestSize) {
            bestRef = ref;
            bestSize = entry->ObjSize;
        }
    }

    if (bestRef > 0) {
        UObject* obj = pkg->GetUObject(bestRef);
        if (obj) {
            UModel* model = UObject::Cast<UModel>(obj);
            if (model) {
                model->LoadNow();
                bsp_model = model;
                godot::UtilityFunctions::print(U"[XIII] Found BSP model: ", godot::String(model->Name.ToString().c_str()),
                    U" (ref=", bestRef, U", size=", bestSize, U")");
            }
        }
    }

    if (!bsp_model) {
        godot::UtilityFunctions::print(U"[XIII] No BSP model found");
        return;
    }

    create_bsp_mesh();
    godot::UtilityFunctions::print(U"[XIII] Level loaded successfully");
}

godot::Vector3 BSPLoader::ue_to_godot(float x, float y, float z) {
    return godot::Vector3(x, z, -y);
}

void BSPLoader::create_bsp_mesh() {
    using namespace godot;
    if (!bsp_model || !map_pkg) return;

    UModel* model = bsp_model;
    int validPolys = 0, skipped = 0;

    struct MatBatch {
        int texRef;
        PackedVector3Array verts;
        PackedVector2Array uvs;
        PackedInt32Array indices;
    };
    std::map<int, MatBatch> batches;

    for (size_t ni = 0; ni < model->Nodes.size(); ni++) {
        const BspNode& node = model->Nodes[ni];
        if (node.Surf < 0 || node.Surf >= (int)model->Surfaces.size()) continue;
        const BspSurface& surf = model->Surfaces[node.Surf];
        if (surf.PolyFlags & 0x00000001) continue;

        int vp = node.VertPool;
        int nv = node.NumVertices;
        if (vp < 0 || vp + nv > (int)model->Vertices.size() || nv < 3) {
            skipped++;
            continue;
        }

        Vec3 basePoint = (surf.pBase >= 0 && surf.pBase < (int)model->Points.size())
            ? model->Points[surf.pBase] : Vec3{0,0,0};
        Vec3 texU = (surf.vTextureU >= 0 && surf.vTextureU < (int)model->Vectors.size())
            ? model->Vectors[surf.vTextureU] : Vec3{1,0,0};
        Vec3 texV = (surf.vTextureV >= 0 && surf.vTextureV < (int)model->Vectors.size())
            ? model->Vectors[surf.vTextureV] : Vec3{0,0,1};

        int texRef = surf.MaterialObjRef;
        auto& batch = batches[texRef];

        uint32_t baseVert = (uint32_t)(batch.verts.size());

        bool valid = true;
        for (int vi = 0; vi < nv; vi++) {
            int pidx = model->Vertices[vp + vi].Vertex;
            if (pidx < 0 || pidx >= (int)model->Points.size()) { valid = false; break; }

            Vec3 P = model->Points[pidx];
            Vec3 diff = P - basePoint;
            float u = diff.Dot(texU) / 256.0f;
            float v = diff.Dot(texV) / 256.0f;

            batch.verts.push_back(ue_to_godot(P.x, P.y, P.z));
            batch.uvs.push_back(Vector2(u, v));
        }

        if (!valid) {
            batch.verts.resize(baseVert);
            batch.uvs.resize(baseVert);
            skipped++;
            continue;
        }

        for (int ti = 1; ti < nv - 1; ti++) {
            batch.indices.push_back(baseVert);
            batch.indices.push_back(baseVert + ti);
            batch.indices.push_back(baseVert + ti + 1);
        }
        validPolys++;
    }

    UtilityFunctions::print(U"[XIII] BSP: ", validPolys, U" polys, ", skipped, U" skipped, ", batches.size(), U" materials");

    // Calculate bounds for camera positioning
    Vector3 bmin(1e9, 1e9, 1e9), bmax(-1e9, -1e9, -1e9);
    for (auto& [texRef, batch] : batches) {
        for (int i = 0; i < (int)batch.verts.size(); i++) {
            bmin.x = std::min(bmin.x, batch.verts[i].x);
            bmin.y = std::min(bmin.y, batch.verts[i].y);
            bmin.z = std::min(bmin.z, batch.verts[i].z);
            bmax.x = std::max(bmax.x, batch.verts[i].x);
            bmax.y = std::max(bmax.y, batch.verts[i].y);
            bmax.z = std::max(bmax.z, batch.verts[i].z);
        }
    }
    Vector3 center = (bmin + bmax) * 0.5f;
    Vector3 extent = bmax - bmin;
    UtilityFunctions::print(U"[XIII] Bounds: min=(", bmin.x, U",", bmin.y, U",", bmin.z,
        U") max=(", bmax.x, U",", bmax.y, U",", bmax.z,
        U") center=(", center.x, U",", center.y, U",", center.z,
        U") size=(", extent.x, U",", extent.y, U",", extent.z, U")");

    MeshInstance3D* mesh_inst = memnew(MeshInstance3D);
    mesh_inst->set_name("BSPMesh");
    add_child(mesh_inst);

    int mesh_idx = 0;
    for (auto& [texRef, batch] : batches) {
        if (batch.verts.size() == 0 || batch.indices.size() == 0) continue;

        Ref<ArrayMesh> array_mesh;
        array_mesh.instantiate();

        godot::Array arrays;
        arrays.resize(Mesh::ARRAY_MAX);
        arrays[Mesh::ARRAY_VERTEX] = batch.verts;
        arrays[Mesh::ARRAY_TEX_UV] = batch.uvs;
        arrays[Mesh::ARRAY_INDEX] = batch.indices;

        array_mesh->add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, arrays);

        Ref<StandardMaterial3D> mat;
        mat.instantiate();
        mat->set_cull_mode(StandardMaterial3D::CULL_DISABLED);

        if (texRef != 0 && map_pkg) {
            UObject* obj = map_pkg->GetUObject(texRef);
            if (obj) {
                UTexture* tex = UObject::Cast<UTexture>(obj);
                if (tex) {
                    tex->LoadNow();
                    Ref<ImageTexture> gtex = create_texture_from_data(tex);
                    if (gtex.is_valid()) {
                        mat->set_texture(BaseMaterial3D::TEXTURE_ALBEDO, gtex);
                        mat->set_albedo(Color(1, 1, 1));
                    }
                }
            }
        }

        if (!mat->get_texture(BaseMaterial3D::TEXTURE_ALBEDO).is_valid()) {
            float r = (float)((texRef * 12345) % 255) / 255.0f;
            float g = (float)((texRef * 54321) % 255) / 255.0f;
            float b = (float)((texRef * 98765) % 255) / 255.0f;
            mat->set_albedo(Color(r * 0.5f + 0.3f, g * 0.5f + 0.3f, b * 0.5f + 0.3f));
        }

        array_mesh->surface_set_material(0, mat);
        mesh_inst->set_mesh(array_mesh);
        mesh_idx++;
    }
}

godot::Ref<godot::ImageTexture> BSPLoader::create_texture_from_data(UTexture* tex) {
    using namespace godot;
    if (tex->Mipmaps.empty()) return Ref<ImageTexture>();

    UnrealMipmap& mip = tex->Mipmaps[0];
    if (mip.Data.empty() || mip.Width <= 0 || mip.Height <= 0)
        return Ref<ImageTexture>();

    int w = mip.Width;
    int h = mip.Height;
    PackedByteArray data;

    switch (tex->UsedFormat) {
    case TextureFormat::P8: {
        data.resize(w * h * 4);
        uint8_t* ptr = data.ptrw();
        for (int i = 0; i < w * h; i++) {
            ptr[(i * 4) + 0] = mip.Data[i];
            ptr[(i * 4) + 1] = mip.Data[i];
            ptr[(i * 4) + 2] = mip.Data[i];
            ptr[(i * 4) + 3] = 255;
        }
        break;
    }
    case TextureFormat::BGRA8:
    case TextureFormat::BGRA8_LM: {
        data.resize(w * h * 4);
        uint8_t* ptr = data.ptrw();
        for (int i = 0; i < w * h; i++) {
            ptr[(i * 4) + 0] = mip.Data[i * 4 + 2];
            ptr[(i * 4) + 1] = mip.Data[i * 4 + 1];
            ptr[(i * 4) + 2] = mip.Data[i * 4 + 0];
            ptr[(i * 4) + 3] = mip.Data[i * 4 + 3];
        }
        break;
    }
    case TextureFormat::BC1:
    case TextureFormat::BC3:
    {
        int blocksX = (w + 3) / 4;
        int blocksY = (h + 3) / 4;
        int blockSize = (tex->UsedFormat == TextureFormat::BC1) ? 8 : 16;
        data.resize(w * h * 4);
        uint8_t* ptr = data.ptrw();

        for (int by = 0; by < blocksY; by++) {
            for (int bx = 0; bx < blocksX; bx++) {
                const uint8_t* block = mip.Data.data() + (by * blocksX + bx) * blockSize;

                uint16_t c0 = block[0] | (block[1] << 8);
                uint16_t c1 = block[2] | (block[3] << 8);
                uint32_t cidx = block[4] | (block[5] << 8) | (block[6] << 16) | (block[7] << 24);

                auto rgb565 = [](uint16_t c, uint8_t& r, uint8_t& g, uint8_t& b) {
                    r = (c & 0x1F) * 255 / 31;
                    g = ((c >> 5) & 0x3F) * 255 / 63;
                    b = ((c >> 11) & 0x1F) * 255 / 31;
                };

                uint8_t r0, g0, b0, r1, g1, b1;
                rgb565(c0, r0, g0, b0);
                rgb565(c1, r1, g1, b1);

                uint8_t a0 = 255, a1 = 255;
                uint64_t aIdx = 0;
                bool hasAlpha = (tex->UsedFormat == TextureFormat::BC3);
                if (hasAlpha) {
                    a0 = block[8]; a1 = block[9];
                    aIdx = block[10] | ((uint64_t)block[11]<<8) | ((uint64_t)block[12]<<16) |
                           ((uint64_t)block[13]<<24) | ((uint64_t)block[14]<<32) | ((uint64_t)block[15]<<40);
                }

                for (int j = 0; j < 4; j++) {
                    for (int i = 0; i < 4; i++) {
                        int x = bx * 4 + i;
                        int y = by * 4 + j;
                        if (x >= w || y >= h) continue;

                        int ci = (cidx >> ((j * 4 + i) * 2)) & 3;
                        uint8_t* px = ptr + (y * w + x) * 4;

                        switch (ci) {
                        case 0: px[0]=r0; px[1]=g0; px[2]=b0; break;
                        case 1: px[0]=r1; px[1]=g1; px[2]=b1; break;
                        case 2: px[0]=(2*r0+r1)/3; px[1]=(2*g0+g1)/3; px[2]=(2*b0+b1)/3; break;
                        case 3: px[0]=(r0+2*r1)/3; px[1]=(g0+2*g1)/3; px[2]=(b0+2*b1)/3; break;
                        }

                        if (hasAlpha) {
                            int ai = (aIdx >> ((j * 4 + i) * 3)) & 7;
                            if (a0 > a1) {
                                const int vals[] = {a0, a1, (6*a0+1*a1)/7, (5*a0+2*a1)/7,
                                                    (4*a0+3*a1)/7, (3*a0+4*a1)/7, (2*a0+5*a1)/7, (1*a0+6*a1)/7};
                                px[3] = vals[ai];
                            } else {
                                const int vals[] = {a0, a1, (4*a0+1*a1)/5, (3*a0+2*a1)/5,
                                                    (2*a0+3*a1)/5, (1*a0+4*a1)/5, 0, 255};
                                px[3] = vals[ai];
                            }
                        } else {
                            px[3] = 255;
                        }
                    }
                }
            }
        }
        break;
    }
    default:
        UtilityFunctions::print(U"[XIII] Unsupported texture format: ", (int)tex->UsedFormat);
        return Ref<ImageTexture>();
    }

    Ref<Image> img = Image::create_from_data(w, h, false, Image::FORMAT_RGBA8, data);
    if (img.is_null()) return Ref<ImageTexture>();

    Ref<ImageTexture> tex_res;
    tex_res.instantiate();
    tex_res->set_image(img);
    return tex_res;
}
