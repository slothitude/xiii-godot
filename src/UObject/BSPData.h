#pragma once

#include "xiii.h"
#include "Math/vec.h"

// BSP data structures for UE2 level geometry
// Adapted from SurrealEngine ULevel.h

struct BspNode
{
    float PlaneX, PlaneY, PlaneZ, PlaneW; // Plane equation
    uint64_t ZoneMask = 0;
    uint8_t NodeFlags = 0;
    int VertPool = 0;   // Index into Vertices array
    int Surf = 0;        // Index into Surfaces array (-1 if none)
    int Back = 0;        // Child node index
    int Front = 0;       // Child node index
    int Plane = 0;       // Index into Vectors for plane normal
    int CollisionBound = 0;
    int RenderBound = 0;
    int Zone0 = 0, Zone1 = 0;
    uint8_t NumVertices = 0;
    int32_t Leaf0 = 0, Leaf1 = 0;
};

struct BspSurface
{
    int MaterialObjRef = 0; // objref to UTexture (resolved lazily)
    uint32_t PolyFlags = 0;
    int pBase = 0;         // Index into Points
    int vNormal = 0;       // Index into Vectors
    int vTextureU = 0;     // Index into Vectors
    int vTextureV = 0;     // Index into Vectors
    int LightMap = 0;      // Index into LightMap array
    int BrushPoly = 0;
    int16_t PanU = 0, PanV = 0;
    int BrushActorRef = 0; // objref
    // XIII-specific extra fields (version 100)
    int Extra1 = 0;        // always 0 (null ref)
    int Extra2 = 0;        // import reference (negative)
    int Extra3 = 0;        // optional, present when plane not aligned after Extra2
    // XIII-specific: per-surface FPlane
    float PlaneX = 0, PlaneY = 0, PlaneZ = 0, PlaneW = 0;
};

struct BspVert
{
    int Vertex = 0; // Index into Points array
    int Side = 0;
};

struct ZoneProperties
{
    int ZoneActorRef = 0; // objref
    uint64_t Connectivity = 0;
    uint64_t Visibility = 0;
};

struct LightMapIndex
{
    int32_t DataOffset = 0;
    float PanX = 0, PanY = 0, PanZ = 0;
    int UClamp = 0, VClamp = 0;
    float UScale = 0, VScale = 0;
    int32_t LightActors = 0;
};

struct ConvexVolumeLeaf
{
    int Zone = 0;
    int Permeating = 0;
    int Volumetric = 0;
    uint64_t VisibleZones = 0;
};
