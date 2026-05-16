#pragma once

#include "UObject.h"
#include "BSPData.h"

// UModel — contains the BSP tree for a level
// Adapted from SurrealEngine UModel (version > 61 path, XIII uses v100)
class UModel : public UObject
{
public:
    using UObject::UObject;

    void Load(ObjectStream* stream) override;

    Array<Vec3> Vectors;
    Array<Vec3> Points;
    Array<BspNode> Nodes;
    Array<BspSurface> Surfaces;
    Array<BspVert> Vertices;

    int32_t NumSharedSides = 0;
    Array<ZoneProperties> Zones;

    // Polys object reference (not used for rendering)
    int PolysRef = 0;

    Array<LightMapIndex> LightMap;
    Array<uint8_t> LightBits;

    // Bounds, LeafHulls, Leaves, Lights — stored but not used for Phase 1 rendering
    int32_t RootOutside = 0;
    int32_t Linked = 0;
};
