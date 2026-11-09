#include "KyaG2D.h"

uint64 KyaG2D::HashName(const FString& Name)
{
    FTCHARToUTF8 Text(*Name);
    uint8 Bytes[8] = {};
    for (int32 Index = 0; Index < Text.Length(); ++Index)
    {
        uint8 Character = uint8(Text.Get()[Index]);
        if (Character >= 'a' && Character <= 'z') Character -= 0x20;
        Bytes[Index & 7] = uint8(Bytes[Index & 7] + Character);
    }
    uint64 Result = 0;
    for (int32 Index = 0; Index < 8; ++Index) Result |= uint64(Bytes[Index]) << (Index * 8);
    return Result;
}

bool KyaG2D::ReadMaterials(TConstArrayView<uint8> Data, TMap<uint64, int32>& OutMaterials)
{
    OutMaterials.Reset();
    if (Data.Num() < 32 || FMemory::Memcmp(Data.GetData() + 12, ".G2D", 4) != 0) return false;
    auto U32 = [&Data](int32 Offset) -> uint32 {
        const uint8* P = Data.GetData() + Offset;
        return uint32(P[0]) | (uint32(P[1]) << 8) | (uint32(P[2]) << 16) | (uint32(P[3]) << 24);
    };
    if (U32(16) != 0x2a44322a) return false;
    const uint32 RootSize = U32(24);
    if (RootSize < 16 || RootSize > uint32(Data.Num() - 16)) return false;
    const int32 RootEnd = 16 + int32(RootSize);
    int32 Pos = 32;
    while (Pos < RootEnd)
    {
        if (RootEnd - Pos < 16) return false;
        const uint32 Size = U32(Pos + 8);
        if (Size < 16 || Size > uint32(RootEnd - Pos)) return false;
        if (U32(Pos) == 0x4154414d) // MATA
        {
            if (Size < 32) return false;
            const int32 Hash = Pos + 16;
            const uint32 HashSize = U32(Hash + 8);
            if (U32(Hash) != 0x48534148 || HashSize < 16 || HashSize > Size - 16 || (HashSize - 16) % 16) return false;
            TMap<uint64, int32> Materials;
            for (uint32 Offset = 16; Offset < HashSize; Offset += 16)
            {
                const int32 Record = Hash + int32(Offset);
                const uint64 Key = uint64(U32(Record)) | (uint64(U32(Record + 4)) << 32);
                const uint32 Target = U32(Record + 8);
                if (Target > uint32(Data.Num()) || uint32(Data.Num()) - Target < 16 || U32(Target) != 0x2e54414d) return false;
                Materials.Add(Key, int32(Target));
            }
            OutMaterials = MoveTemp(Materials);
            return true;
        }
        Pos += int32(Size);
    }
    return false;
}

bool KyaG2D::ReadMaterialBitmap(TConstArrayView<uint8> Data, int32 MaterialOffset, FBitmapReference& OutBitmap)
{
    OutBitmap = FBitmapReference{};
    auto Fits = [&Data](uint32 Offset, uint32 Size) { return Offset <= uint32(Data.Num()) && Size <= uint32(Data.Num()) - Offset; };
    auto U32 = [&Data](uint32 Offset) -> uint32 {
        const uint8* P = Data.GetData() + Offset;
        return uint32(P[0]) | (uint32(P[1]) << 8) | (uint32(P[2]) << 16) | (uint32(P[3]) << 24);
    };
    auto U16 = [&Data](uint32 Offset) -> uint16 { return uint16(Data[Offset]) | (uint16(Data[Offset + 1]) << 8); };
    if (MaterialOffset < 0 || !Fits(MaterialOffset, 48) || U32(MaterialOffset) != 0x2e54414d || Data[MaterialOffset + 16] == 0) return false;
    const uint32 Layer = U32(MaterialOffset + 32);
    if (!Fits(Layer, 52) || U32(Layer) != 0x2e59414c || !U16(Layer + 44)) return false;
    const uint32 Texture = U32(Layer + 48);
    if (!Fits(Texture, 48) || U32(Texture) != 0x2e584554) return false;
    const uint32 BitmapHash = U32(Texture + 24);
    if (!Fits(BitmapHash, 16)) return false;
    const uint32 Bitmap = U32(BitmapHash + 8);
    if (!Fits(Bitmap, 28) || U32(Bitmap) != 0x2e443254) return false;
    FBitmapReference Result;
    Result.BitmapOffset = int32(Bitmap);
    Result.Width = U16(Bitmap + 16);
    Result.Height = U16(Bitmap + 18);
    Result.PixelFormat = U16(Bitmap + 20);
    Result.MipCount = U16(Bitmap + 22);
    const uint32 BitmapSize = U32(Bitmap + 8);
    if (BitmapSize < 28 || !Fits(Bitmap, BitmapSize)) return false;
    Result.BitmapChunkSize = int32(BitmapSize);
    if (U32(Texture + 32))
    {
        const uint32 PaletteId = U16(Layer + 46);
        const uint64 PaletteRecord = uint64(Texture) + 48 + uint64(PaletteId) * 16;
        if (PaletteRecord > uint64(Data.Num()) || uint64(Data.Num()) - PaletteRecord < 16) return false;
        const uint32 PaletteHash = U32(uint32(PaletteRecord) + 8);
        if (!Fits(PaletteHash, 16)) return false;
        const uint32 Palette = U32(PaletteHash + 8);
        if (!Fits(Palette, 28) || (U32(Palette) != 0x32334150 && U32(Palette) != 0x2e443254)) return false;
        Result.PaletteOffset = int32(Palette);
        const uint32 PaletteSize = U32(Palette + 8);
        if (PaletteSize < 28 || !Fits(Palette, PaletteSize)) return false;
        Result.PaletteChunkSize = int32(PaletteSize);
    }
    OutBitmap = Result;
    return true;
}

bool KyaG2D::ReadPalette32(TConstArrayView<uint8> Data, int32 PaletteOffset, TArray<FColor>& OutColors)
{
    OutColors.Reset();
    if (PaletteOffset < 0 || PaletteOffset > Data.Num() - 32) return false;
    auto U32 = [&Data](int32 Offset) -> uint32 {
        const uint8* P = Data.GetData() + Offset;
        return uint32(P[0]) | (uint32(P[1]) << 8) | (uint32(P[2]) << 16) | (uint32(P[3]) << 24);
    };
    auto U16 = [&Data](int32 Offset) -> uint16 { return uint16(Data[Offset]) | (uint16(Data[Offset + 1]) << 8); };
    if (U32(PaletteOffset) != 0x32334150 || U16(PaletteOffset + 20) != 32) return false;
    const uint32 Size = U32(PaletteOffset + 8);
    const uint32 Width = U16(PaletteOffset + 16), Height = U16(PaletteOffset + 18);
    const uint64 Count = uint64(Width) * Height;
    if (Size < 32 || Size > uint32(Data.Num() - PaletteOffset) || Count * 4 > Size - 32 || Count > MAX_int32) return false;
    TArray<FColor> Colors;
    Colors.Reserve(int32(Count));
    for (uint32 Index = 0; Index < Count; ++Index)
    {
        const uint8* P = Data.GetData() + PaletteOffset + 32 + Index * 4;
        Colors.Emplace(P[0], P[1], P[2], P[3]);
    }
    OutColors = MoveTemp(Colors);
    return true;
}

bool KyaG2D::ReadUploadLists(TConstArrayView<uint8> Data, int32 BitmapOffset,
    TArray<TPair<int32, int32>>& OutLists)
{
    OutLists.Reset();
    auto Fits = [&Data](uint64 Offset, uint64 Size) { return Offset <= uint64(Data.Num()) && Size <= uint64(Data.Num()) - Offset; };
    auto U32 = [&Data](uint32 Offset) -> uint32 {
        const uint8* P = Data.GetData() + Offset;
        return uint32(P[0]) | (uint32(P[1]) << 8) | (uint32(P[2]) << 16) | (uint32(P[3]) << 24);
    };
    if (BitmapOffset < 0 || !Fits(BitmapOffset, 28)) return false;
    const uint32 Header = U32(BitmapOffset + 24);
    if (!Header || !Fits(Header, 16)) return false;
    TArray<TPair<int32, int32>> Lists;
    for (uint32 Index = 0; Index < 2; ++Index)
    {
        const uint32 Offset = U32(Header + Index * 8);
        const uint32 Count = U32(Header + Index * 8 + 4);
        if (Offset > MAX_int32 || Count > MAX_int32 || !Fits(Offset, uint64(Count) * 16)) return false;
        Lists.Emplace(int32(Offset), int32(Count));
    }
    OutLists = MoveTemp(Lists);
    return true;
}

bool KyaG2D::ReadUploadTransfers(TConstArrayView<uint8> Data, int32 ListOffset,
    int32 PacketCount, TArray<FUploadTransfer>& OutTransfers)
{
    OutTransfers.Reset();
    if (ListOffset < 0 || PacketCount < 0 || uint64(ListOffset) + uint64(PacketCount) * 16 > uint64(Data.Num())) return false;
    auto U32 = [&Data](int32 Offset) -> uint32 {
        const uint8* P = Data.GetData() + Offset;
        return uint32(P[0]) | (uint32(P[1]) << 8) | (uint32(P[2]) << 16) | (uint32(P[3]) << 24);
    };
    TArray<FUploadTransfer> Transfers;
    bool Active = false;
    for (int32 Index = 0; Index < PacketCount; ++Index)
    {
        const int32 Packet = ListOffset + Index * 16;
        const uint32 Command = U32(Packet), Address = U32(Packet + 8);
        const uint64 Value = uint64(U32(Packet)) | (uint64(U32(Packet + 4)) << 32);
        if (Address == 0x0e)
        {
            if (Index + 1 < PacketCount && U32(Packet + 24) == 0x3f) Active = false;
            else { Transfers.Emplace(); Active = true; }
        }
        if (!Active) continue;
        FUploadTransfer& Transfer = Transfers.Last();
        if ((Command >> 28) == 3)
        {
            const uint32 Offset = U32(Packet + 4);
            const uint64 Size = uint64(Command & 0xffff) * 16;
            if (Offset > uint32(Data.Num()) || Size > uint64(Data.Num()) - Offset || Size > MAX_int32) return false;
            Transfer.DataOffset = int32(Offset);
            Transfer.DataSize = int32(Size);
        }
        if (Address == 0x50) Transfer.BitBlt = Value;
        else if (Address == 0x51) Transfer.Position = Value;
        else if (Address == 0x52) Transfer.Region = Value;
    }
    for (const FUploadTransfer& Transfer : Transfers)
        if (Transfer.DataOffset == INDEX_NONE || Transfer.Region == 0) return false;
    OutTransfers = MoveTemp(Transfers);
    return !OutTransfers.IsEmpty();
}

bool KyaG2D::ReadMaterialRegister(TConstArrayView<uint8> Data, int32 MaterialOffset,
    int32 LayerIndex, uint32 Register, uint64& OutValue)
{
    OutValue = 0;
    auto Fits = [&Data](uint64 Offset, uint64 Size) { return Offset <= uint64(Data.Num()) && Size <= uint64(Data.Num()) - Offset; };
    auto U32 = [&Data](uint32 Offset) -> uint32 {
        const uint8* P = Data.GetData() + Offset;
        return uint32(P[0]) | (uint32(P[1]) << 8) | (uint32(P[2]) << 16) | (uint32(P[3]) << 24);
    };
    if (MaterialOffset < 0 || !Fits(MaterialOffset, 48) || U32(MaterialOffset) != 0x2e54414d ||
        LayerIndex < 0 || LayerIndex >= Data[MaterialOffset + 16]) return false;
    const uint32 List = U32(MaterialOffset + 24);
    const uint32 Count = U32(MaterialOffset + 28);
    const uint64 Start = uint64(List) + uint64(LayerIndex) * Count * 16;
    if (!Fits(Start, uint64(Count) * 16)) return false;
    for (uint32 Index = 0; Index < Count; ++Index)
    {
        const uint32 Packet = uint32(Start + uint64(Index) * 16);
        if (U32(Packet + 8) == Register)
        {
            OutValue = uint64(U32(Packet)) | (uint64(U32(Packet + 4)) << 32);
            return true;
        }
    }
    return false;
}

bool KyaG2D::ReadMaterialTex0(TConstArrayView<uint8> Data, int32 MaterialOffset,
    int32 LayerIndex, uint64& OutTex0)
{
    return ReadMaterialRegister(Data, MaterialOffset, LayerIndex, 0x06, OutTex0);
}

bool KyaG2D::WriteGs32(TConstArrayView<uint8> Data, const FUploadTransfer& Transfer, TArray<uint8>& GsMemory)
{
    constexpr uint32 MemorySize = 4 * 1024 * 1024;
    if (GsMemory.Num() != MemorySize || Transfer.DestinationFormat() != 0 || !Transfer.DestinationWidth() ||
        Transfer.DataOffset < 0 || Transfer.DataSize < 0 || uint64(Transfer.DataOffset) + Transfer.DataSize > uint64(Data.Num()) ||
        uint64(Transfer.Width()) * Transfer.Height() * 4 > uint64(Transfer.DataSize)) return false;
    const uint32 StartX = uint32((Transfer.Position >> 32) & 0x7ff);
    const uint32 StartY = uint32((Transfer.Position >> 48) & 0x7ff);
    if ((Transfer.Position >> 59) & 3) return false; // Reverse transfer directions are not handled.
    for (uint32 Y = 0; Y < Transfer.Height(); ++Y)
        for (uint32 X = 0; X < Transfer.Width(); ++X)
        {
            const uint32 Px = StartX + X, Py = StartY + Y;
            const uint32 Page = (Py / 32) * Transfer.DestinationWidth() + Px / 64;
            const uint32 Bx = (Px % 64) / 8, By = (Py % 32) / 8;
            const uint32 Block = (Bx & 1) | ((By & 1) << 1) | ((Bx & 2) << 1) |
                ((By & 2) << 2) | ((Bx & 4) << 2);
            const uint32 Cx = Px & 7, Cy = Py & 7;
            const uint32 Word = (Cx & 1) | ((Cy & 1) << 1) | ((Cx & 6) << 1) | ((Cy & 6) << 3);
            const uint32 Address = (Transfer.DestinationBase() * 256 + Page * 8192 + Block * 256 + Word * 4) % MemorySize;
            const uint64 Source = uint64(Transfer.DataOffset) + (uint64(Y) * Transfer.Width() + X) * 4;
            FMemory::Memcpy(GsMemory.GetData() + Address, Data.GetData() + Source, 4);
        }
    return true;
}

static uint32 Gs8Address(uint32 Base, uint32 BufferWidth, uint32 X, uint32 Y)
{
    const uint32 Page = (Y / 64) * (BufferWidth / 2) + X / 128;
    const uint32 Bx = (X % 128) / 16, By = (Y % 64) / 16;
    const uint32 Block = (Bx & 1) | ((By & 1) << 1) | ((Bx & 2) << 1) | ((By & 2) << 2) | ((Bx & 4) << 2);
    const uint32 Cx = X & 15, Cy = Y & 15;
    const uint32 Column = ((Cx & 1) << 2) | ((Cx & 2) << 3) | ((Cx & 8) >> 2) |
        ((Cy & 1) << 3) | ((Cy & 2) >> 1) | ((Cy & 4) << 4) | ((Cy & 8) << 4) |
        (((((Cx >> 2) ^ (Cy >> 1) ^ (Cy >> 2)) & 1)) << 5);
    return (Base * 256 + Page * 8192 + Block * 256 + Column) % (4 * 1024 * 1024);
}

static uint32 Gs4NibbleAddress(uint32 Base, uint32 BufferWidth, uint32 X, uint32 Y)
{
    const uint32 Page = (Y / 128) * (BufferWidth / 2) + X / 128;
    const uint32 Bx = (X % 128) / 32, By = (Y % 128) / 16;
    const uint32 Block = (By & 1) | ((Bx & 1) << 1) | ((By & 2) << 1) | ((Bx & 2) << 2) | ((By & 4) << 2);
    const uint32 Cx = X & 31, Cy = Y & 15;
    const uint32 Nibble = ((Cx & 1) << 3) | ((Cx & 2) << 4) | ((Cx & 8) >> 2) | ((Cx & 16) >> 2) |
        ((Cy & 1) << 4) | ((Cy & 2) >> 1) | ((Cy & 4) << 5) | ((Cy & 8) << 5) |
        (((((Cx >> 2) ^ (Cy >> 1) ^ (Cy >> 2)) & 1)) << 6);
    return (Base * 512 + Page * 16384 + Block * 512 + Nibble) % (8 * 1024 * 1024);
}

bool KyaG2D::WriteGsIndexed(TConstArrayView<uint8> Data, const FUploadTransfer& Transfer, TArray<uint8>& GsMemory)
{
    const uint32 Format = Transfer.DestinationFormat();
    const uint64 Count = uint64(Transfer.Width()) * Transfer.Height();
    if (GsMemory.Num() != 4 * 1024 * 1024 || (Format != 0x13 && Format != 0x14) || !Transfer.DestinationWidth() ||
        Transfer.DataOffset < 0 || Transfer.DataSize < 0 || uint64(Transfer.DataOffset) + Transfer.DataSize > uint64(Data.Num()) ||
        (Format == 0x13 ? Count : (Count + 1) / 2) > uint64(Transfer.DataSize) || ((Transfer.Position >> 59) & 3)) return false;
    const uint32 StartX = uint32((Transfer.Position >> 32) & 0x7ff), StartY = uint32((Transfer.Position >> 48) & 0x7ff);
    for (uint32 Y = 0; Y < Transfer.Height(); ++Y)
        for (uint32 X = 0; X < Transfer.Width(); ++X)
        {
            const uint64 Source = uint64(Y) * Transfer.Width() + X;
            if (Format == 0x13)
                GsMemory[Gs8Address(Transfer.DestinationBase(), Transfer.DestinationWidth(), StartX + X, StartY + Y)] = Data[Transfer.DataOffset + Source];
            else
            {
                const uint8 Value = (Data[Transfer.DataOffset + Source / 2] >> ((Source & 1) * 4)) & 15;
                const uint32 Destination = Gs4NibbleAddress(Transfer.DestinationBase(), Transfer.DestinationWidth(), StartX + X, StartY + Y);
                uint8& Byte = GsMemory[Destination / 2];
                const uint32 Shift = (Destination & 1) * 4;
                Byte = uint8((Byte & ~(15u << Shift)) | (Value << Shift));
            }
        }
    return true;
}

bool KyaG2D::ReadGs32(TConstArrayView<uint8> GsMemory, uint64 Tex0, TArray<FColor>& OutPixels)
{
    OutPixels.Reset();
    constexpr uint32 MemorySize = 4 * 1024 * 1024;
    const uint32 Base = uint32(Tex0 & 0x3fff), BufferWidth = uint32((Tex0 >> 14) & 0x3f);
    const uint32 Format = uint32((Tex0 >> 20) & 0x3f);
    const uint32 WidthBits = uint32((Tex0 >> 26) & 15), HeightBits = uint32((Tex0 >> 30) & 15);
    if (GsMemory.Num() != MemorySize || Format != 0 || !BufferWidth || WidthBits > 10 || HeightBits > 10) return false;
    const uint32 Width = 1u << WidthBits, Height = 1u << HeightBits;
    OutPixels.SetNumUninitialized(Width * Height);
    for (uint32 Y = 0; Y < Height; ++Y)
        for (uint32 X = 0; X < Width; ++X)
        {
            const uint32 Page = (Y / 32) * BufferWidth + X / 64;
            const uint32 Bx = (X % 64) / 8, By = (Y % 32) / 8;
            const uint32 Block = (Bx & 1) | ((By & 1) << 1) | ((Bx & 2) << 1) |
                ((By & 2) << 2) | ((Bx & 4) << 2);
            const uint32 Cx = X & 7, Cy = Y & 7;
            const uint32 Word = (Cx & 1) | ((Cy & 1) << 1) | ((Cx & 6) << 1) | ((Cy & 6) << 3);
            const uint32 Address = (Base * 256 + Page * 8192 + Block * 256 + Word * 4) % MemorySize;
            OutPixels[Y * Width + X] = FColor(GsMemory[Address], GsMemory[Address + 1], GsMemory[Address + 2], GsMemory[Address + 3]);
        }
    return true;
}

bool KyaG2D::ReadGs8(TConstArrayView<uint8> GsMemory, uint64 Tex0, TArray<uint8>& OutIndices)
{
    OutIndices.Reset();
    constexpr uint32 MemorySize = 4 * 1024 * 1024;
    const uint32 Base = uint32(Tex0 & 0x3fff);
    const uint32 BufferWidth = uint32((Tex0 >> 14) & 0x3f);
    const uint32 Format = uint32((Tex0 >> 20) & 0x3f);
    const uint32 WidthBits = uint32((Tex0 >> 26) & 0xf), HeightBits = uint32((Tex0 >> 30) & 0xf);
    if (GsMemory.Num() != MemorySize || Format != 0x13 || WidthBits > 10 || HeightBits > 10) return false;
    const uint32 Width = 1u << WidthBits, Height = 1u << HeightBits;
    TArray<uint8> Indices;
    Indices.SetNumUninitialized(Width * Height);
    for (uint32 Y = 0; Y < Height; ++Y)
        for (uint32 X = 0; X < Width; ++X)
        {
            Indices[Y * Width + X] = GsMemory[Gs8Address(Base, BufferWidth, X, Y)];
        }
    OutIndices = MoveTemp(Indices);
    return true;
}

bool KyaG2D::ReadGs4(TConstArrayView<uint8> GsMemory, uint64 Tex0, TArray<uint8>& OutIndices)
{
    OutIndices.Reset();
    constexpr uint32 MemorySize = 4 * 1024 * 1024;
    const uint32 Base = uint32(Tex0 & 0x3fff), BufferWidth = uint32((Tex0 >> 14) & 0x3f);
    const uint32 WidthBits = uint32((Tex0 >> 26) & 15), HeightBits = uint32((Tex0 >> 30) & 15);
    if (GsMemory.Num() != MemorySize || ((Tex0 >> 20) & 63) != 0x14 || WidthBits > 10 || HeightBits > 10) return false;
    const uint32 Width = 1u << WidthBits, Height = 1u << HeightBits;
    OutIndices.SetNumUninitialized(Width * Height);
    for (uint32 Y = 0; Y < Height; ++Y)
        for (uint32 X = 0; X < Width; ++X)
        {
            const uint32 Address = Gs4NibbleAddress(Base, BufferWidth, X, Y);
            OutIndices[Y * Width + X] = (GsMemory[Address / 2] >> ((Address & 1) * 4)) & 15;
        }
    return true;
}

bool KyaG2D::ReadGsClut4(TConstArrayView<uint8> GsMemory, uint64 Tex0, TArray<FColor>& OutColors)
{
    OutColors.Reset();
    constexpr uint32 MemorySize = 4 * 1024 * 1024;
    const uint32 Base = uint32((Tex0 >> 37) & 0x3fff);
    if (GsMemory.Num() != MemorySize || ((Tex0 >> 20) & 63) != 0x14 || ((Tex0 >> 51) & 15) != 0 || ((Tex0 >> 55) & 1)) return false;
    OutColors.Reserve(16);
    for (uint32 Index = 0; Index < 16; ++Index)
    {
        const uint32 Word = (Index & 1) | ((Index & 6) << 1) | ((Index & 8) >> 2);
        const uint32 Address = (Base * 256 + Word * 4) % MemorySize;
        OutColors.Emplace(GsMemory[Address], GsMemory[Address + 1], GsMemory[Address + 2], GsMemory[Address + 3]);
    }
    return true;
}

bool KyaG2D::ReadGsClut32(TConstArrayView<uint8> GsMemory, uint64 Tex0, TArray<FColor>& OutColors)
{
    OutColors.Reset();
    constexpr uint32 MemorySize = 4 * 1024 * 1024;
    const uint32 Format = uint32((Tex0 >> 20) & 0x3f);
    const uint32 Base = uint32((Tex0 >> 37) & 0x3fff);
    const uint32 PaletteFormat = uint32((Tex0 >> 51) & 0xf);
    const uint32 StorageMode = uint32((Tex0 >> 55) & 1);
    const uint32 Start = uint32((Tex0 >> 56) & 0x1f);
    // Nonzero CSA and CSM2 require the current CLUT state / TEXCLUT register.
    if (GsMemory.Num() != MemorySize || Format != 0x13 || PaletteFormat != 0 || StorageMode || Start) return false;
    TArray<FColor> Colors;
    Colors.Reserve(256);
    for (uint32 Index = 0; Index < 256; ++Index)
    {
        // Equivalent to PCSX2's clutTableT32I8, including its second 128-entry half.
        const uint32 Word = (Index & 1) | ((Index & 6) << 1) | ((Index & 8) >> 2) |
            ((Index & 0x10) << 2) | ((Index & 0x60) >> 1) | (Index & 0x80);
        const uint32 Address = (Base * 256 + Word * 4) % MemorySize;
        Colors.Emplace(GsMemory[Address], GsMemory[Address + 1], GsMemory[Address + 2], GsMemory[Address + 3]);
    }
    OutColors = MoveTemp(Colors);
    return true;
}

bool KyaG2D::DecodeMaterialTexture(TConstArrayView<uint8> Data, int32 MaterialOffset,
    int32& OutWidth, int32& OutHeight, TArray<FColor>& OutPixels)
{
    OutWidth = OutHeight = 0;
    OutPixels.Reset();
    FBitmapReference Bitmap;
    uint64 Tex0 = 0;
    TArray<TPair<int32, int32>> Lists;
    TArray<FUploadTransfer> Transfers;
    if (!ReadMaterialBitmap(Data, MaterialOffset, Bitmap) ||
        !ReadMaterialTex0(Data, MaterialOffset, 0, Tex0) ||
        !ReadUploadLists(Data, Bitmap.PaletteOffset == INDEX_NONE ? Bitmap.BitmapOffset : Bitmap.PaletteOffset, Lists) || Lists.IsEmpty() ||
        !ReadUploadTransfers(Data, Lists[0].Key, Lists[0].Value, Transfers)) return false;
    TArray<uint8> Memory;
    Memory.SetNumZeroed(4 * 1024 * 1024);
    for (const FUploadTransfer& Transfer : Transfers)
        if (Transfer.DestinationFormat() == 0 ? !WriteGs32(Data, Transfer, Memory) : !WriteGsIndexed(Data, Transfer, Memory)) return false;
    TArray<FColor> Pixels;
    if (((Tex0 >> 20) & 0x3f) == 0)
    {
        if (!ReadGs32(Memory, Tex0, Pixels)) return false;
    }
    else
    {
        TArray<uint8> Indices;
        TArray<FColor> Palette;
        if (Bitmap.PaletteOffset == INDEX_NONE) return false;
        const bool Is4Bit = ((Tex0 >> 20) & 63) == 0x14;
        if (Is4Bit ? (!ReadGs4(Memory, Tex0, Indices) || !ReadGsClut4(Memory, Tex0, Palette)) :
            (!ReadGs8(Memory, Tex0, Indices) || !ReadGsClut32(Memory, Tex0, Palette))) return false;
        Pixels.Reserve(Indices.Num());
        for (uint8 Index : Indices) Pixels.Add(Palette[Index]);
    }
    OutWidth = 1 << ((Tex0 >> 26) & 15);
    OutHeight = 1 << ((Tex0 >> 30) & 15);
    OutPixels = MoveTemp(Pixels);
    return true;
}
