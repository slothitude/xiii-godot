#pragma once

#include "UObject.h"
#include "Package/ObjectSerializer.h"

// Mipmap level for a texture
struct UnrealMipmap
{
    int Width = 0;
    int Height = 0;
    int UBits = 0;
    int VBits = 0;
    Array<uint8_t> Data;
};

// Simplified texture format enum
enum class TextureFormat : uint8_t
{
    P8 = 0x00,
    BGRA8_LM = 0x01,
    R5G6B5 = 0x02,
    BC1 = 0x03,    // DXT1
    RGB8 = 0x04,
    BGRA8 = 0x05,
    BC2 = 0x06,    // DXT3
    BC3 = 0x07,    // DXT5
};

// UTexture — texture with mipmap data
class UTexture : public UObject
{
public:
    using UObject::UObject;

    void Load(ObjectStream* stream) override;

    // Override LoadNow to capture properties
    void LoadNow() override;

    TextureFormat UsedFormat = TextureFormat::P8;
    Array<UnrealMipmap> Mipmaps;
    PropertyCapture Props;
};
