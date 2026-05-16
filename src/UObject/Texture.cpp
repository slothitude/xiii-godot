#include "Texture.h"
#include "Package/ObjectSerializer.h"
#include "Package/Package.h"
#include <cstdio>

void UTexture::LoadNow()
{
    if (!package || exportIndex < 0)
        return;

    auto stream = package->OpenObjectStream(exportIndex);
    if (!stream || stream->IsEmpty())
        return;

    // Capture texture-specific properties
    stream->SkipProperties(&Props);

    Load(stream.get());
}

void UTexture::Load(ObjectStream* stream)
{
    // Properties already skipped (with capture)

    // Uncompressed mipmaps
    int mipsCount = stream->ReadUInt8();
    Mipmaps.resize(mipsCount);
    for (int i = 0; i < mipsCount; i++)
    {
        // widthoffset (version >= 63, always true for XIII)
        if (stream->GetVersion() >= 63)
            stream->ReadInt32(); // widthoffset, discard

        int bytes = stream->ReadIndex();
        Mipmaps[i].Data.resize(bytes);
        stream->ReadBytes(Mipmaps[i].Data.data(), bytes);

        Mipmaps[i].Width = stream->ReadUInt32();
        Mipmaps[i].Height = stream->ReadUInt32();
        Mipmaps[i].UBits = stream->ReadUInt8();
        Mipmaps[i].VBits = stream->ReadUInt8();
    }

    // Compressed mipmaps
    if (Props.bHasComp)
    {
        int compMipsCount = stream->ReadUInt8();
        // For now, just read compressed mipmaps and append
        int startIdx = (int)Mipmaps.size();
        Mipmaps.resize(startIdx + compMipsCount);
        for (int i = 0; i < compMipsCount; i++)
        {
            if (stream->GetVersion() >= 68)
                stream->ReadInt32(); // widthoffset

            int bytes = stream->ReadIndex();
            Mipmaps[startIdx + i].Data.resize(bytes);
            stream->ReadBytes(Mipmaps[startIdx + i].Data.data(), bytes);

            Mipmaps[startIdx + i].Width = stream->ReadUInt32();
            Mipmaps[startIdx + i].Height = stream->ReadUInt32();
            Mipmaps[startIdx + i].UBits = stream->ReadUInt8();
            Mipmaps[startIdx + i].VBits = stream->ReadUInt8();
        }
        UsedFormat = (TextureFormat)Props.compFormat;
    }
    else
    {
        UsedFormat = (TextureFormat)Props.format;
    }

    if (!Mipmaps.empty())
    {
        printf("[UTexture] Loaded: %s, fmt=%d, %d mips, %dx%d\n",
            Name.ToString().c_str(), (int)UsedFormat, (int)Mipmaps.size(),
            Mipmaps[0].Width, Mipmaps[0].Height);
    }
}
