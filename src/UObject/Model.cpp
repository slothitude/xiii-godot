#include "Model.h"
#include "Package/ObjectSerializer.h"
#include <cstring>

void UModel::Load(ObjectStream* stream)
{
    // UPrimitive::Load - BBox + BSphere
    stream->ReadFloat(); stream->ReadFloat(); stream->ReadFloat();
    stream->ReadFloat(); stream->ReadFloat(); stream->ReadFloat();
    stream->ReadUInt8(); // bbox valid
    // BSphere: 6 floats
    for (int i = 0; i < 6; i++) stream->ReadFloat();

    int count;

    // Vectors
    count = stream->ReadIndex();
    Vectors.resize(count);
    for (int i = 0; i < count; i++)
    {
        Vectors[i].x = stream->ReadFloat();
        Vectors[i].y = stream->ReadFloat();
        Vectors[i].z = stream->ReadFloat();
    }

    // Points
    count = stream->ReadIndex();
    Points.resize(count);
    for (int i = 0; i < count; i++)
    {
        Points[i].x = stream->ReadFloat();
        Points[i].y = stream->ReadFloat();
        Points[i].z = stream->ReadFloat();
    }

    // Nodes - 70-byte fixed blocks (XIII-specific format)
    // Layout: FPlane(16) + ZoneMask(8) + NodeFlags(1) + UnknownByte(1)
    //         + 9 index fields (ReadXIIIIndex) + padding + NumVertices(byte 69)
    count = stream->ReadIndex();
    printf("[UModel] Node count: %d\n", count);
    Nodes.resize(count);

    for (int i = 0; i < count; i++)
    {
        size_t nodeStart = stream->Tell();
        BspNode& node = Nodes[i];

        node.PlaneX = stream->ReadFloat();
        node.PlaneY = stream->ReadFloat();
        node.PlaneZ = stream->ReadFloat();
        node.PlaneW = stream->ReadFloat();
        node.ZoneMask = stream->ReadUInt64();
        node.NodeFlags = stream->ReadUInt8();

        stream->Skip(1); // XIII extra byte

        node.VertPool = stream->ReadXIIIIndex();
        node.Surf = stream->ReadXIIIIndex();
        node.Back = stream->ReadXIIIIndex();
        node.Front = stream->ReadXIIIIndex();
        node.Plane = stream->ReadXIIIIndex();
        node.CollisionBound = stream->ReadXIIIIndex();
        node.RenderBound = stream->ReadXIIIIndex();
        node.Zone0 = stream->ReadXIIIIndex();
        node.Zone1 = stream->ReadXIIIIndex();

        // NumVertices at fixed byte 69 of the 70-byte block
        stream->Seek(nodeStart + 69);
        node.NumVertices = stream->ReadUInt8();

        stream->Seek(nodeStart + 70);
    }

    // Surfaces - standard UE2 + XIII extras (3 indices + FPlane)
    count = stream->ReadIndex();
    printf("[UModel] Surface count: %d\n", count);
    Surfaces.resize(count);
    for (int i = 0; i < count; i++)
    {
        BspSurface& surf = Surfaces[i];
        surf.MaterialObjRef = stream->ReadIndex();
        surf.PolyFlags = stream->ReadUInt32();
        surf.pBase = stream->ReadIndex();
        surf.vNormal = stream->ReadIndex();
        surf.vTextureU = stream->ReadIndex();
        surf.vTextureV = stream->ReadIndex();
        surf.LightMap = stream->ReadIndex();
        surf.BrushPoly = stream->ReadIndex();
        surf.PanU = stream->ReadInt16();
        surf.PanV = stream->ReadInt16();
        surf.BrushActorRef = stream->ReadIndex();

        // XIII extras
        surf.Extra1 = stream->ReadIndex();
        surf.Extra2 = stream->ReadIndex();
        surf.Extra3 = stream->ReadIndex();
        surf.PlaneX = stream->ReadFloat();
        surf.PlaneY = stream->ReadFloat();
        surf.PlaneZ = stream->ReadFloat();
        surf.PlaneW = stream->ReadFloat();
    }

    // Vertices (BspVerts) - XIII uses uint16 count + uint16 pairs
    uint16_t vertCount = stream->ReadUInt16();
    printf("[UModel] Vertex count: %d\n", vertCount);
    Vertices.resize(vertCount);
    for (int i = 0; i < vertCount; i++)
    {
        Vertices[i].Vertex = stream->ReadUInt16();
        Vertices[i].Side = stream->ReadUInt16();
    }

    printf("[UModel] Loaded: %d vecs, %d pts, %d nodes, %d surfs, %d verts\n",
        (int)Vectors.size(), (int)Points.size(), (int)Nodes.size(),
        (int)Surfaces.size(), (int)Vertices.size());

    // Validate
    int validVP = 0, nvGte3 = 0;
    for (size_t i = 0; i < Nodes.size(); i++)
    {
        if (Nodes[i].VertPool >= 0 && Nodes[i].VertPool < (int)Vertices.size()) validVP++;
        if (Nodes[i].NumVertices >= 3) nvGte3++;
    }
    printf("[UModel] Valid: VP=%d NV>=3=%d (of %d)\n",
        validVP, nvGte3, (int)Nodes.size());
}
