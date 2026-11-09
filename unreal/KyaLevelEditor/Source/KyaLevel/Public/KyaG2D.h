#pragma once
#include "CoreMinimal.h"

namespace KyaG2D
{
    /** Eden's case-insensitive, eight-byte material-name hash. */
    KYALEVEL_API uint64 HashName(const FString& Name);
    struct FBitmapReference
    {
        int32 BitmapOffset = INDEX_NONE;
        int32 PaletteOffset = INDEX_NONE;
        uint16 Width = 0, Height = 0, PixelFormat = 0;
        uint16 MipCount = 0;
        int32 BitmapChunkSize = 0;
        int32 PaletteChunkSize = 0;
    };
    struct FUploadTransfer
    {
        int32 DataOffset = INDEX_NONE;
        int32 DataSize = 0;
        uint64 BitBlt = 0, Position = 0, Region = 0;

        uint32 DestinationBase() const { return uint32((BitBlt >> 32) & 0x3fff); }
        uint32 DestinationWidth() const { return uint32((BitBlt >> 48) & 0x3f); }
        uint32 DestinationFormat() const { return uint32((BitBlt >> 56) & 0x3f); }
        uint32 Width() const { return uint32(Region & 0xfff); }
        uint32 Height() const { return uint32((Region >> 32) & 0xfff); }
    };
    /** Map material hashes to MAT chunk offsets in an unmodified G2D file. */
    KYALEVEL_API bool ReadMaterials(TConstArrayView<uint8> Data, TMap<uint64, int32>& OutMaterials);
    KYALEVEL_API bool ReadMaterialBitmap(TConstArrayView<uint8> Data, int32 MaterialOffset, FBitmapReference& OutBitmap);
    /** Read serialized PA32 entries in storage order, before GS CLUT addressing. */
    KYALEVEL_API bool ReadPalette32(TConstArrayView<uint8> Data, int32 PaletteOffset, TArray<FColor>& OutColors);
    /** Resolve both serialized DMA command lists attached to a bitmap. */
    KYALEVEL_API bool ReadUploadLists(TConstArrayView<uint8> Data, int32 BitmapOffset,
        TArray<TPair<int32, int32>>& OutLists);
    KYALEVEL_API bool ReadUploadTransfers(TConstArrayView<uint8> Data, int32 ListOffset,
        int32 PacketCount, TArray<FUploadTransfer>& OutTransfers);
    /** Read GS TEX0 state for a material layer. */
    KYALEVEL_API bool ReadMaterialTex0(TConstArrayView<uint8> Data, int32 MaterialOffset,
        int32 LayerIndex, uint64& OutTex0);
    KYALEVEL_API bool ReadMaterialRegister(TConstArrayView<uint8> Data, int32 MaterialOffset,
        int32 LayerIndex, uint32 Register, uint64& OutValue);
    /** Apply a PSMCT32 transfer to a 4 MiB GS memory image. */
    KYALEVEL_API bool WriteGs32(TConstArrayView<uint8> Data, const FUploadTransfer& Transfer,
        TArray<uint8>& GsMemory);
    /** Apply a PSMT8 or PSMT4 upload to a 4 MiB GS memory image. */
    KYALEVEL_API bool WriteGsIndexed(TConstArrayView<uint8> Data, const FUploadTransfer& Transfer,
        TArray<uint8>& GsMemory);
    /** Read direct PSMCT32 pixels using the texture's TEX0 addressing. */
    KYALEVEL_API bool ReadGs32(TConstArrayView<uint8> GsMemory, uint64 Tex0, TArray<FColor>& OutPixels);
    /** Read PSMT8 indices using the texture's TEX0 addressing. */
    KYALEVEL_API bool ReadGs8(TConstArrayView<uint8> GsMemory, uint64 Tex0, TArray<uint8>& OutIndices);
    KYALEVEL_API bool ReadGs4(TConstArrayView<uint8> GsMemory, uint64 Tex0, TArray<uint8>& OutIndices);
    KYALEVEL_API bool ReadGsClut4(TConstArrayView<uint8> GsMemory, uint64 Tex0, TArray<FColor>& OutColors);
    /** Decode the GS CSM1, PSMCT32 palette for an eight-bit texture. */
    KYALEVEL_API bool ReadGsClut32(TConstArrayView<uint8> GsMemory, uint64 Tex0, TArray<FColor>& OutColors);
    /** Decode supported first-layer textures by executing their upload packet. */
    KYALEVEL_API bool DecodeMaterialTexture(TConstArrayView<uint8> Data, int32 MaterialOffset,
        int32& OutWidth, int32& OutHeight, TArray<FColor>& OutPixels);
}
