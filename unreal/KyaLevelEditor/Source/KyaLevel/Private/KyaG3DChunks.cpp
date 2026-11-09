#include "KyaG3DChunks.h"

bool KyaG3D::ReadChunks(TConstArrayView<uint8> Data, TArray<FKyaG3DChunk>& OutChunks)
{
    OutChunks.Reset();
    if (Data.Num() < 16 || FMemory::Memcmp(Data.GetData() + 12, ".G3D", 4) != 0)
        return false;
    auto ReadU32 = [&Data](int32 Offset) -> uint32 {
        const uint8* P = Data.GetData() + Offset;
        return uint32(P[0]) | (uint32(P[1]) << 8) | (uint32(P[2]) << 16) | (uint32(P[3]) << 24);
    };
    TArray<FKyaG3DChunk> Chunks;
    int32 Offset = 16;
    while (Offset < Data.Num())
    {
        if (Data.Num() - Offset < 16) return false;
        const uint32 Size = ReadU32(Offset + 8);
        if (Size < 16 || Size > uint32(Data.Num() - Offset)) return false;
        FKyaG3DChunk Chunk;
        Chunk.Tag = ReadU32(Offset);
        Chunk.Offset = Offset;
        Chunk.Size = int32(Size);
        Chunks.Add(Chunk);
        Offset += int32(Size);
    }
    OutChunks = MoveTemp(Chunks);
    return true;
}

bool KyaG3D::ReadRelocations(TConstArrayView<uint8> Data, TMap<int32, int32>& OutOffsets,
    TArray<int32>& OutInvalidFields)
{
    OutOffsets.Reset();
    OutInvalidFields.Reset();
    TArray<FKyaG3DChunk> Chunks;
    if (!ReadChunks(Data, Chunks)) return false;
    const FKyaG3DChunk* Relocations = Chunks.FindByPredicate(
        [](const FKyaG3DChunk& Chunk) { return Chunk.Tag == 0x41414552; });
    if (!Relocations) return false;
    auto ReadU32 = [&Data](int32 Offset) -> uint32 {
        const uint8* P = Data.GetData() + Offset;
        return uint32(P[0]) | (uint32(P[1]) << 8) | (uint32(P[2]) << 16) | (uint32(P[3]) << 24);
    };
    TMap<int32, int32> Result;
    TArray<int32> InvalidFields;
    int32 Pos = Relocations->Offset + 16;
    const int32 End = Relocations->Offset + Relocations->Size;
    while (Pos < End)
    {
        if (End - Pos < 16) return false;
        const uint32 Tag = ReadU32(Pos);
        const uint32 Size = ReadU32(Pos + 8);
        if (Size < 16 || Size > uint32(End - Pos) || (Size & 3)) return false;
        if (Tag == 0x4c414552 || Tag == 0x52545352 || Tag == 0x4a424f52 || Tag == 0x4f454752)
        {
            for (int32 Word = Pos + 16; Word < Pos + int32(Size); Word += 4)
            {
                const uint32 Field = ReadU32(Word);
                if (!Field) continue;
                if (Field > uint32(Data.Num() - 4)) return false;
                const uint32 Target = ReadU32(int32(Field));
                if (Target >= uint32(Data.Num())) InvalidFields.Add(int32(Field));
                else Result.Add(int32(Field), int32(Target));
            }
        }
        Pos += int32(Size);
    }
    OutOffsets = MoveTemp(Result);
    OutInvalidFields = MoveTemp(InvalidFields);
    return true;
}

bool KyaG3D::ReadHierarchyOffsets(TConstArrayView<uint8> Data, TArray<int32>& OutOffsets)
{
    OutOffsets.Reset();
    TArray<FKyaG3DChunk> Chunks;
    if (!ReadChunks(Data, Chunks)) return false;
    const FKyaG3DChunk* Hall = Chunks.FindByPredicate(
        [](const FKyaG3DChunk& Chunk) { return Chunk.Tag == 0x4c4c4148; });
    if (!Hall || Hall->Size < 32) return false;
    auto ReadU32 = [&Data](int32 Offset) -> uint32 {
        const uint8* P = Data.GetData() + Offset;
        return uint32(P[0]) | (uint32(P[1]) << 8) | (uint32(P[2]) << 16) | (uint32(P[3]) << 24);
    };
    const int32 Hash = Hall->Offset + 16;
    const uint32 Size = ReadU32(Hash + 8);
    if (ReadU32(Hash) != 0x48534148 || Size < 16 || Size > uint32(Hall->Size - 16) || (Size - 16) % 16) return false;
    TArray<int32> Result;
    // HASH contains actual hierarchy records, including single-entry actor roots.
    const uint32 Count = (Size - 16) / 16;
    for (uint32 Index = 0; Index < Count; ++Index)
    {
        const uint32 ChunkOffset = ReadU32(Hash + 16 + Index * 16 + 8);
        if (ChunkOffset > uint32(Data.Num()) || uint32(Data.Num()) - ChunkOffset < 16 + 0xb0) return false;
        Result.Add(int32(ChunkOffset + 16));
    }
    OutOffsets = MoveTemp(Result);
    return true;
}

bool KyaG3D::ExpandClusterHierarchies(TConstArrayView<uint8> Data, TConstArrayView<int32> Roots,
    TArray<int32>& OutHierarchies)
{
    OutHierarchies.Reset();
    TArray<int32> Result;
    TSet<int32> Complete;
    for (int32 Root : Roots)
    {
        TSet<int32> Path;
        int32 Current = Root;
        while (Current != INDEX_NONE && !Complete.Contains(Current))
        {
            if (Current < 16 || Current > Data.Num() - 0xb0 || Path.Contains(Current) ||
                FMemory::Memcmp(Data.GetData() + Current - 16, "HIER", 4) != 0) return false;
            Path.Add(Current);
            Result.Add(Current);
            const uint8* P = Data.GetData() + Current + 0x90;
            const uint32 Link = uint32(P[0]) | (uint32(P[1]) << 8) | (uint32(P[2]) << 16) | (uint32(P[3]) << 24);
            if (Link > uint32(MAX_int32 - 16)) return false;
            Current = Link ? int32(Link + 16) : INDEX_NONE;
        }
        for (int32 Visited : Path) Complete.Add(Visited);
    }
    OutHierarchies = MoveTemp(Result);
    return true;
}

bool KyaG3D::ReadHierarchyStrips(TConstArrayView<uint8> Data, TArray<FKyaG3DStripReference>& OutStrips)
{
    OutStrips.Reset();
    TArray<int32> Hierarchies;
    if (!ReadHierarchyOffsets(Data, Hierarchies)) return false;
    auto Fits = [&Data](uint32 Offset, uint32 Size) { return Offset <= uint32(Data.Num()) && Size <= uint32(Data.Num()) - Offset; };
    auto U32 = [&Data](uint32 Offset) -> uint32 {
        const uint8* P = Data.GetData() + Offset;
        return uint32(P[0]) | (uint32(P[1]) << 8) | (uint32(P[2]) << 16) | (uint32(P[3]) << 24);
    };
    TArray<FKyaG3DStripReference> Strips;
    for (int32 Hierarchy : Hierarchies)
    {
        const uint32 LodCount = uint32(Data[Hierarchy + 0x9c]) | (uint32(Data[Hierarchy + 0x9d]) << 8);
        if (!LodCount) continue;
        if (!Fits(Hierarchy + 0xb0, 8)) return false;
        const uint32 RenderType = uint32(Data[Hierarchy + 0xb4]) | (uint32(Data[Hierarchy + 0xb5]) << 8);
        // Type 3 points to ed_3d_sprite, not ed_3d_strip; types 1 and 2
        // are not submitted by ed3DRenderSonHierarchy.
        if (RenderType != 0) continue;
        const uint32 Hash = U32(Hierarchy + 0xb0);
        if (!Hash) continue;
        if (!Fits(Hash, 16)) return false;
        const uint32 Object = U32(Hash + 8);
        if (!Object) continue;
        if (!Fits(Object, 48)) return false;
        const uint32 Count = U32(Object + 20);
        uint32 Strip = U32(Object + 44);
        if (Count > uint32(Data.Num()) / 64) return false;
        for (uint32 Index = 0; Index < Count; ++Index)
        {
            if (!Strip || !Fits(Strip, 64)) return false;
            FKyaG3DStripReference Reference;
            Reference.HierarchyOffset = Hierarchy;
            Reference.StripOffset = int32(Strip);
            Strips.Add(Reference);
            Strip = U32(Strip + 12);
        }
    }
    OutStrips = MoveTemp(Strips);
    return true;
}

bool KyaG3D::ReadClusterTreeStrips(TConstArrayView<uint8> Data, TArray<FClusterStripReference>& OutStrips, TArray<int32>* OutHierarchies)
{
    OutStrips.Reset();
    if (OutHierarchies) OutHierarchies->Reset();
    TArray<FKyaG3DChunk> Chunks;
    if (!ReadChunks(Data, Chunks)) return false;
    const FKyaG3DChunk* Csta = Chunks.FindByPredicate([](const FKyaG3DChunk& Chunk) { return Chunk.Tag == 0x41545343; });
    if (!Csta) return true;
    auto U32 = [&Data](int64 Offset) -> uint32 {
        const uint8* P = Data.GetData() + Offset;
        return uint32(P[0]) | (uint32(P[1]) << 8) | (uint32(P[2]) << 16) | (uint32(P[3]) << 24);
    };
    auto Fits = [&Data](int64 Offset, int64 Size) { return Offset >= 0 && Offset <= Data.Num() && Size <= Data.Num() - Offset; };
    TArray<FClusterStripReference> Result;
    TArray<int32> HierarchyResult;
    TFunction<bool(int64, int64, int32)> ReadNode;
    ReadNode = [&](int64 Chunk, int64 ParentEnd, int32 Depth) -> bool {
        if (Depth > 64 || !Fits(Chunk, 80) || Chunk + 80 > ParentEnd) return false;
        const uint32 Size = U32(Chunk + 8);
        if (Size < 80 || Size > uint64(ParentEnd - Chunk)) return false;
        const int64 End = Chunk + Size, Payload = Chunk + 16;
        const uint32 Count = uint32(Data[Payload + 8]) | (uint32(Data[Payload + 9]) << 8);
        const uint32 HierCount = uint32(Data[Payload + 0x1a]) | (uint32(Data[Payload + 0x1b]) << 8);
        if (OutHierarchies && HierCount)
        {
            const int64 Hash = Payload + 64;
            if (End - Hash < 16 || U32(Hash) != 0x48534148) return false;
            const uint32 HashSize = U32(Hash + 8);
            if (HashSize < 16 || HashSize > uint64(End - Hash) || HierCount > (HashSize - 16) / 16) return false;
            for (uint32 HierIndex = 0; HierIndex < HierCount; ++HierIndex)
            {
                const uint32 Target = U32(Hash + 16 + int64(HierIndex) * 16 + 8);
                if (!Fits(Target, 16 + 0xb0) || U32(Target) != 0x52454948 || Target > uint32(MAX_int32 - 16)) return false;
                HierarchyResult.AddUnique(int32(Target + 16));
            }
        }
        const uint32 Bank = U32(Payload + 0x30);
        if (Count && (!Fits(Bank, 16) || U32(Bank) != 0x4b4e424d || U32(Bank + 8) < 16 || !Fits(Bank, U32(Bank + 8)))) return false;
        if (Count > uint32(Data.Num()) / 64) return false;
        uint32 Strip = U32(Payload + 0x38);
        for (uint32 Index = 0; Index < Count; ++Index)
        {
            if (!Strip || !Fits(Strip, 64)) return false;
            Result.Add({int32(Strip), int32(Bank)});
            Strip = U32(Strip + 12);
        }
        // The unprepared file stores a HASH chunk here; runtime preparation
        // rewrites this area before the game's hierarchy-offset traversal.
        int64 Child = Payload + 64;
        if (Child > End) return false;
        while (Child < End)
        {
            if (End - Child < 16) return false;
            const uint32 Tag = U32(Child), ChildSize = U32(Child + 8);
            if (ChildSize < 16 || ChildSize > uint64(End - Child)) return false;
            if ((Tag == 0x55514443 || Tag == 0x434f4443) && !ReadNode(Child, End, Depth + 1)) return false;
            Child += ChildSize;
        }
        return true;
    };
    const int64 CstaEnd = int64(Csta->Offset) + Csta->Size;
    for (int64 Tree = int64(Csta->Offset) + 16; Tree < CstaEnd; )
    {
        if (CstaEnd - Tree < 16) return false;
        const uint32 Tag = U32(Tree), Size = U32(Tree + 8);
        if (Size < 16 || Size > uint64(CstaEnd - Tree)) return false;
        if (Tag == 0x41514443 || Tag == 0x414f4443)
        {
            if (Size < 48) return false;
            const int64 End = Tree + Size;
            for (int64 Root = Tree + 48; Root < End; )
            {
                if (End - Root < 16) return false;
                const uint32 RootTag = U32(Root), RootSize = U32(Root + 8);
                if (RootSize < 16 || RootSize > uint64(End - Root)) return false;
                if ((RootTag == 0x55514443 || RootTag == 0x434f4443) && !ReadNode(Root, End, 0)) return false;
                Root += RootSize;
            }
        }
        Tree += Size;
    }
    OutStrips = MoveTemp(Result);
    if (OutHierarchies) *OutHierarchies = MoveTemp(HierarchyResult);
    return true;
}

bool KyaG3D::ReadClusterStrips(TConstArrayView<uint8> Data, TArray<int32>& OutOffsets, int32* OutMaterialBank)
{
    OutOffsets.Reset();
    if (OutMaterialBank) *OutMaterialBank = INDEX_NONE;
    TArray<FKyaG3DChunk> Chunks;
    if (!ReadChunks(Data, Chunks)) return false;
    const FKyaG3DChunk* Csta = Chunks.FindByPredicate([](const FKyaG3DChunk& Chunk) { return Chunk.Tag == 0x41545343; });
    if (!Csta) return true;
    auto U32 = [&Data](int32 Offset) -> uint32 {
        const uint8* P = Data.GetData() + Offset;
        return uint32(P[0]) | (uint32(P[1]) << 8) | (uint32(P[2]) << 16) | (uint32(P[3]) << 24);
    };
    if (Csta->Size < 16 + 16 + 32 + 16 + 64) return false;
    const int32 Cdqa = Csta->Offset + 16;
    const int32 Cdqu = Cdqa + 16 + 32;
    if (U32(Cdqa) != 0x41514443 || U32(Cdqu) != 0x55514443) return false;
    const int32 Payload = Cdqu + 16;
    const uint32 MaterialBank = U32(Payload + 0x30);
    if (OutMaterialBank)
    {
        if (MaterialBank > uint32(Data.Num()) || uint32(Data.Num()) - MaterialBank < 16 || U32(MaterialBank) != 0x4b4e424d) return false;
        const uint32 Size = U32(MaterialBank + 8);
        if (Size < 16 || Size > uint32(Data.Num()) - MaterialBank || (Size - 16) % 16) return false;
        *OutMaterialBank = int32(MaterialBank);
    }
    const uint32 Count = uint32(Data[Payload + 8]) | (uint32(Data[Payload + 9]) << 8);
    uint32 Strip = U32(Payload + 0x38);
    if (Count > uint32(Data.Num()) / 64) return false;
    TArray<int32> Result;
    for (uint32 Index = 0; Index < Count; ++Index)
    {
        if (!Strip || Strip > uint32(Data.Num()) || uint32(Data.Num()) - Strip < 64) return false;
        Result.Add(int32(Strip));
        Strip = U32(Strip + 12);
    }
    OutOffsets = MoveTemp(Result);
    return true;
}

bool KyaG3D::ReadStripMaterialHash(TConstArrayView<uint8> Data, int32 StripOffset, int32 MaterialBank, uint64& OutHash)
{
    OutHash = 0;
    auto Fits = [&Data](int64 Offset, int64 Size) { return Offset >= 0 && Offset <= Data.Num() && Size <= Data.Num() - Offset; };
    auto U32 = [&Data](int64 Offset) -> uint32 {
        const uint8* P = Data.GetData() + Offset;
        return uint32(P[0]) | (uint32(P[1]) << 8) | (uint32(P[2]) << 16) | (uint32(P[3]) << 24);
    };
    if (!Fits(StripOffset, 6) || !Fits(MaterialBank, 16) || U32(MaterialBank) != 0x4b4e424d) return false;
    const int16 Index = int16(uint16(Data[StripOffset + 4]) | (uint16(Data[StripOffset + 5]) << 8));
    const uint32 Size = U32(MaterialBank + 8);
    if (Index < 0 || Size < 16 || !Fits(MaterialBank, Size)) return false;
    const int64 Offset = int64(MaterialBank) + 16 + int64(Index) * 16;
    if (Offset + 16 > int64(MaterialBank) + Size) return false;
    OutHash = uint64(U32(Offset)) | (uint64(U32(Offset + 4)) << 32);
    return true;
}

bool KyaG3D::ReadFirstStripGifTag(TConstArrayView<uint8> Data, int32 StripOffset,
    uint32& OutVertexCount, uint32& OutPrimitive)
{
    OutVertexCount = 0;
    OutPrimitive = 0;
    auto Fits = [&Data](int64 Offset, int64 Size) { return Offset >= 0 && Offset <= Data.Num() && Size <= Data.Num() - Offset; };
    auto U32 = [&Data](int64 Offset) -> uint32 {
        const uint8* P = Data.GetData() + Offset;
        return uint32(P[0]) | (uint32(P[1]) << 8) | (uint32(P[2]) << 16) | (uint32(P[3]) << 24);
    };
    if (!Fits(StripOffset, 64)) return false;
    const int64 Vif = int64(StripOffset) + int32(U32(StripOffset + 8));
    if (!Fits(Vif, 32) || U32(Vif + 28) != 0x6c018000) return false;
    const uint32 Gif = U32(Vif + 20);
    if (!Fits(Gif, 16)) return false;
    OutVertexCount = U32(Gif) & 0x7fff;
    OutPrimitive = (U32(Gif + 4) >> 15) & 0x7ff;
    return true;
}

bool KyaG3D::ReadFirstStripVertices(TConstArrayView<uint8> Data, int32 StripOffset,
    TArray<FKyaG3DVertex>& OutVertices)
{
    OutVertices.Reset();
    uint32 Count, Primitive;
    if (!ReadFirstStripGifTag(Data, StripOffset, Count, Primitive)) return false;
    auto U32 = [&Data](int64 Offset) -> uint32 {
        const uint8* P = Data.GetData() + Offset;
        return uint32(P[0]) | (uint32(P[1]) << 8) | (uint32(P[2]) << 16) | (uint32(P[3]) << 24);
    };
    const bool Packed = (U32(StripOffset) & 0x400) != 0;
    const uint32 Start = U32(StripOffset + 40);
    const uint32 Stride = Packed ? 8 : 16;
    if (Start > uint32(Data.Num()) || uint64(Count) * Stride > uint32(Data.Num()) - Start) return false;
    TArray<FKyaG3DVertex> Vertices;
    Vertices.Reserve(Count);
    for (uint32 Index = 0; Index < Count; ++Index)
    {
        const uint8* P = Data.GetData() + Start + Index * Stride;
        FKyaG3DVertex Vertex;
        if (Packed)
        {
            auto S16 = [P](int32 Offset) { return int16(uint16(P[Offset]) | (uint16(P[Offset + 1]) << 8)); };
            Vertex.Position = FVector3f(S16(0), S16(2), S16(4)) / 4096.0f;
            Vertex.Flags = uint16(S16(6));
        }
        else
        {
            uint32 Bits[3] = { U32(Start + Index * Stride), U32(Start + Index * Stride + 4), U32(Start + Index * Stride + 8) };
            float Values[3];
            FMemory::Memcpy(Values, Bits, sizeof(Values));
            Vertex.Position = FVector3f(Values[0], Values[1], Values[2]);
            Vertex.Flags = U32(Start + Index * Stride + 12);
        }
        Vertices.Add(Vertex);
    }
    OutVertices = MoveTemp(Vertices);
    return true;
}

bool KyaG3D::ReadStripSectionCounts(TConstArrayView<uint8> Data, int32 StripOffset,
    TArray<uint32>& OutCounts)
{
    OutCounts.Reset();
    auto Fits = [&Data](int64 Offset, int64 Size) { return Offset >= 0 && Offset <= Data.Num() && Size <= Data.Num() - Offset; };
    auto U32 = [&Data](int64 Offset) -> uint32 {
        const uint8* P = Data.GetData() + Offset;
        return uint32(P[0]) | (uint32(P[1]) << 8) | (uint32(P[2]) << 16) | (uint32(P[3]) << 24);
    };
    if (!Fits(StripOffset, 64)) return false;
    const uint32 Count = uint32(Data[StripOffset + 0x3a]) | (uint32(Data[StripOffset + 0x3b]) << 8);
    int64 Packet = int64(StripOffset) + int32(U32(StripOffset + 8));
    TArray<uint32> Counts;
    for (uint32 Section = 0; Section < Count; ++Section)
    {
        if (!Fits(Packet, 32) || U32(Packet + 28) != 0x6c018000) return false;
        const uint32 Gif = U32(Packet + 20);
        if (!Fits(Gif, 16)) return false;
        Counts.Add(U32(Gif) & 0x7fff);
        if (Section + 1 == Count) break;
        while (Fits(Packet, 16) && U32(Packet) != 0x60000000) Packet += 16;
        if (!Fits(Packet, 16)) return false;
        Packet += 16;
    }
    OutCounts = MoveTemp(Counts);
    return true;
}

bool KyaG3D::ReadStripVertices(TConstArrayView<uint8> Data, int32 StripOffset,
    TArray<FKyaG3DVertex>& OutVertices, TArray<uint32>& OutSectionCounts)
{
    OutVertices.Reset();
    OutSectionCounts.Reset();
    TArray<uint32> Counts;
    if (!ReadStripSectionCounts(Data, StripOffset, Counts)) return false;
    uint64 UniqueCount = 0;
    for (int32 Index = 0; Index < Counts.Num(); ++Index)
    {
        if (Index && Counts[Index] < 2) return false;
        UniqueCount += Counts[Index] - (Index ? 2 : 0);
    }
    auto U32 = [&Data](int64 Offset) -> uint32 {
        const uint8* P = Data.GetData() + Offset;
        return uint32(P[0]) | (uint32(P[1]) << 8) | (uint32(P[2]) << 16) | (uint32(P[3]) << 24);
    };
    const bool Packed = (U32(StripOffset) & 0x400) != 0;
    const uint32 Start = U32(StripOffset + 40);
    const uint32 Stride = Packed ? 8 : 16;
    if (Start > uint32(Data.Num()) || UniqueCount * Stride > uint32(Data.Num()) - Start) return false;
    TArray<FKyaG3DVertex> Vertices;
    Vertices.Reserve(int32(UniqueCount));
    for (uint32 Index = 0; Index < UniqueCount; ++Index)
    {
        const uint8* P = Data.GetData() + Start + Index * Stride;
        FKyaG3DVertex Vertex;
        if (Packed)
        {
            auto S16 = [P](int32 Offset) { return int16(uint16(P[Offset]) | (uint16(P[Offset + 1]) << 8)); };
            Vertex.Position = FVector3f(S16(0), S16(2), S16(4)) / 4096.0f;
            Vertex.Flags = uint16(S16(6));
        }
        else
        {
            uint32 Bits[3] = { U32(Start + Index * Stride), U32(Start + Index * Stride + 4), U32(Start + Index * Stride + 8) };
            float Values[3];
            FMemory::Memcpy(Values, Bits, sizeof(Values));
            Vertex.Position = FVector3f(Values[0], Values[1], Values[2]);
            Vertex.Flags = U32(Start + Index * Stride + 12);
        }
        Vertices.Add(Vertex);
    }
    OutVertices = MoveTemp(Vertices);
    OutSectionCounts = MoveTemp(Counts);
    return true;
}

bool KyaG3D::ReadStripAttributes(TConstArrayView<uint8> Data, int32 StripOffset,
    TArray<FKyaG3DVertexAttributes>& OutAttributes)
{
    OutAttributes.Reset();
    TArray<uint32> Counts;
    if (!ReadStripSectionCounts(Data, StripOffset, Counts)) return false;
    auto U32 = [&Data](int64 Offset) -> uint32 {
        const uint8* P = Data.GetData() + Offset;
        return uint32(P[0]) | (uint32(P[1]) << 8) | (uint32(P[2]) << 16) | (uint32(P[3]) << 24);
    };
    const uint32 UVBase = U32(StripOffset + 32);
    const uint32 ColorBase = U32(StripOffset + 36);
    const bool Animated = (U32(StripOffset) & 4) != 0;
    uint64 Total = 0;
    for (uint32 Count : Counts) Total += Count;
    if (UVBase > uint32(Data.Num()) || uint64(UVBase) + 16 + Total * 4 > uint64(Data.Num()) || ColorBase > uint32(Data.Num())) return false;
    TArray<FKyaG3DVertexAttributes> Attributes;
    Attributes.Reserve(int32(Total));
    uint64 Index = 0;
    for (int32 Section = 0; Section < Counts.Num(); ++Section)
        for (uint32 I = 0; I < Counts[Section]; ++I, ++Index)
        {
            const uint64 ColorIndex = Animated ? uint64(Section) * 0x48 + I : Index;
            const uint64 ColorOffset = uint64(ColorBase) + (Animated ? 16 : 0) + ColorIndex * 4;
            if (ColorOffset + 4 > uint64(Data.Num())) return false;
            const uint8* UV = Data.GetData() + UVBase + 16 + Index * 4;
            const uint8* Color = Data.GetData() + ColorOffset;
            const int16 S = int16(uint16(UV[0]) | (uint16(UV[1]) << 8));
            const int16 T = int16(uint16(UV[2]) | (uint16(UV[3]) << 8));
            FKyaG3DVertexAttributes Attribute;
            Attribute.UV = FVector2f(float(S) / 4096.0f, float(T) / 4096.0f);
            Attribute.Color = FColor(Color[0], Color[1], Color[2], Color[3]);
            Attributes.Add(Attribute);
        }
    OutAttributes = MoveTemp(Attributes);
    return true;
}

bool KyaG3D::BuildStripTriangles(TConstArrayView<FKyaG3DVertex> Vertices,
    TConstArrayView<uint32> SectionCounts, uint32 Primitive, TArray<uint32>& OutIndices)
{
    OutIndices.Reset();
    if ((Primitive & 7) != 4) return false; // GS triangle strip
    TArray<uint32> Indices;
    uint64 Base = 0;
    uint64 InputIndex = 0;
    uint32 Previous[2] = {};
    for (int32 Section = 0; Section < SectionCounts.Num(); ++Section)
    {
        const uint32 Count = SectionCounts[Section];
        if (Section && Count < 2) return false;
        if (Base + Count > uint64(Vertices.Num())) return false;
        for (uint32 Index = 0; Index < Count; ++Index, ++InputIndex)
        {
            const uint32 Current = uint32(Base + Index);
            if (InputIndex >= 2 && !(Vertices[Current].Flags & 0x8000))
            {
                const uint32 A = Previous[0], B = Previous[1];
                if (Vertices[A].Position != Vertices[B].Position && Vertices[A].Position != Vertices[Current].Position && Vertices[B].Position != Vertices[Current].Position)
                {
                    Indices.Add((InputIndex & 1) ? B : A);
                    Indices.Add((InputIndex & 1) ? A : B);
                    Indices.Add(Current);
                }
            }
            Previous[0] = Previous[1];
            Previous[1] = Current;
        }
        Base += Count;
        if (Section + 1 < SectionCounts.Num()) Base -= 2;
    }
    OutIndices = MoveTemp(Indices);
    return true;
}

bool KyaG3D::DecodeHierarchyStrips(TConstArrayView<uint8> Data, TArray<FKyaG3DDecodedStrip>& OutStrips)
{
    OutStrips.Reset();
    TArray<FKyaG3DStripReference> References;
    if (!ReadHierarchyStrips(Data, References)) return false;
    TArray<FKyaG3DDecodedStrip> Decoded;
    for (const FKyaG3DStripReference& Reference : References)
    {
        uint32 Count = 0, Primitive = 0;
        TArray<uint32> Sections;
        FKyaG3DDecodedStrip Strip;
        Strip.HierarchyOffset = Reference.HierarchyOffset;
        Strip.MaterialIndex = int16(uint16(Data[Reference.StripOffset + 4]) |
            (uint16(Data[Reference.StripOffset + 5]) << 8));
        if (Strip.MaterialIndex >= 0)
        {
            auto U32 = [&Data](int32 Offset) -> uint32 {
                const uint8* P = Data.GetData() + Offset;
                return uint32(P[0]) | (uint32(P[1]) << 8) | (uint32(P[2]) << 16) | (uint32(P[3]) << 24);
            };
            const uint32 Bank = U32(Reference.HierarchyOffset + 0x98);
            if (Bank > uint32(Data.Num()) || uint32(Data.Num()) - Bank < 16 || U32(Bank) != 0x4b4e424d) return false;
            const uint32 BankSize = U32(Bank + 8);
            const uint64 HashOffset = uint64(Bank) + 16 + uint64(Strip.MaterialIndex) * 16;
            if (BankSize < 16 || BankSize > uint32(Data.Num()) - Bank || HashOffset + 16 > uint64(Bank) + BankSize) return false;
            Strip.MaterialHash = uint64(U32(int32(HashOffset))) | (uint64(U32(int32(HashOffset + 4))) << 32);
        }
        if (!ReadFirstStripGifTag(Data, Reference.StripOffset, Count, Primitive) ||
            !ReadStripVertices(Data, Reference.StripOffset, Strip.Vertices, Sections) ||
            !ReadStripAttributes(Data, Reference.StripOffset, Strip.Attributes)) return false;
        TArray<FKyaG3DVertex> Expanded;
        uint32 Base = 0;
        for (int32 Section = 0; Section < Sections.Num(); ++Section)
        {
            for (uint32 Index = 0; Index < Sections[Section]; ++Index)
                Expanded.Add(Strip.Vertices[Base + Index]);
            Base += Sections[Section];
            if (Section + 1 < Sections.Num()) Base -= 2;
        }
        if (Expanded.Num() != Strip.Attributes.Num()) return false;
        Strip.Vertices = MoveTemp(Expanded);
        const uint32 ExpandedCount = Strip.Vertices.Num();
        if (!BuildStripTriangles(Strip.Vertices, MakeArrayView(&ExpandedCount, 1), Primitive, Strip.Indices)) return false;
        Decoded.Add(MoveTemp(Strip));
    }
    OutStrips = MoveTemp(Decoded);
    return true;
}

bool KyaG3D::ReadCameraRelativeBackgroundHierarchies(TConstArrayView<uint8> Data, TArray<int32>& OutOffsets)
{
    OutOffsets.Reset();
    TArray<int32> Hierarchies;
    TArray<FKyaG3DChunk> Chunks;
    if (!ReadHierarchyOffsets(Data, Hierarchies) || !ReadChunks(Data, Chunks)) return false;
    if (Hierarchies.IsEmpty()) return true;
    const FKyaG3DChunk* Hall = Chunks.FindByPredicate([](const FKyaG3DChunk& Chunk) { return Chunk.Tag == 0x4c4c4148; });
    if (!Hall) return false;
    auto U32 = [&Data](int32 Offset) -> uint32 {
        const uint8* P = Data.GetData() + Offset;
        return uint32(P[0]) | (uint32(P[1]) << 8) | (uint32(P[2]) << 16) | (uint32(P[3]) << 24);
    };
    int32 Root = Hierarchies[0];
    int16 Best = -1;
    for (int32 Hierarchy : Hierarchies)
    {
        const int16 Count = int16(uint16(Data[Hierarchy + 0xac]) | (uint16(Data[Hierarchy + 0xad]) << 8));
        if (Count > Best) { Best = Count; Root = Hierarchy; }
    }
    if (Best < 0) return false;
    if (Best == 0) { OutOffsets.Add(Root); return true; }
    TArray<int32> Result;
    const int32 HashEntries = Hall->Offset + 32;
    for (int32 Index = 0; Index < Hierarchies.Num(); ++Index)
    {
        const int32 Hierarchy = Hierarchies[Index];
        TSet<int32> Seen;
        int32 Current = Hierarchy;
        bool Descendant = false;
        while (Current != Root)
        {
            if (Current < 16 || Current > Data.Num() - 0xb0 || Seen.Contains(Current) ||
                FMemory::Memcmp(Data.GetData() + Current - 16, "HIER", 4) != 0) return false;
            Seen.Add(Current);
            const uint32 Parent = U32(Current + 0x90);
            if (!Parent) break;
            if (Parent > uint32(MAX_int32 - 16)) return false;
            Current = int32(Parent + 16);
            if (Current == Root) Descendant = true;
        }
        const int32 Hash = HashEntries + Index * 16;
        const uint64 Name = uint64(U32(Hash)) | (uint64(U32(Hash + 4)) << 32);
        if (Descendant && Name != 0x4c464e99b2a49e87ULL) Result.Add(Hierarchy);
    }
    OutOffsets = MoveTemp(Result);
    return true;
}

bool KyaG3D::ReadBackgroundWorldMatrix(TConstArrayView<uint8> Data, int32 HierarchyOffset,
    TConstArrayView<int32> CameraRelativeHierarchies, const FVector& CameraPosition, FMatrix& OutMatrix)
{
    TArray<int32> Hierarchies;
    if (!ReadHierarchyOffsets(Data, Hierarchies) || !Hierarchies.Contains(HierarchyOffset)) return false;
    TSet<int32> Seen;
    FMatrix World = FMatrix::Identity;
    int32 Current = HierarchyOffset;
    while (Current != INDEX_NONE)
    {
        // A parent can be a root hierarchy with no visible object of its own.
        if (Current < 16 || Current > Data.Num() - 0xb0 ||
            FMemory::Memcmp(Data.GetData() + Current - 16, "HIER", 4) != 0 || Seen.Contains(Current)) return false;
        Seen.Add(Current);
        auto U32 = [&Data](int32 Offset) -> uint32 {
            const uint8* P = Data.GetData() + Offset;
            return uint32(P[0]) | (uint32(P[1]) << 8) | (uint32(P[2]) << 16) | (uint32(P[3]) << 24);
        };
        FMatrix Local = FMatrix::Identity;
        for (int32 Row = 0; Row < 4; ++Row)
            for (int32 Col = 0; Col < 4; ++Col)
            {
                uint32 Bits = U32(Current + (Row * 4 + Col) * 4);
                float Value;
                FMemory::Memcpy(&Value, &Bits, sizeof(Value));
                if (!FMath::IsFinite(Value)) return false;
                Local.M[Row][Col] = Value;
            }
        if (CameraRelativeHierarchies.Contains(Current))
        {
            Local.M[3][0] = CameraPosition.X;
            Local.M[3][1] = CameraPosition.Y;
            Local.M[3][2] = CameraPosition.Z;
        }
        World = World * Local;
        const uint32 ParentChunk = U32(Current + 0x90);
        if (ParentChunk == 0) Current = INDEX_NONE;
        else if (ParentChunk > uint32(MAX_int32 - 16)) return false;
        else Current = int32(ParentChunk + 16);
    }
    OutMatrix = World;
    return true;
}

bool KyaG3D::ReadHierarchyWorldMatrix(TConstArrayView<uint8> Data, int32 HierarchyOffset, FMatrix& OutMatrix)
{
    return ReadBackgroundWorldMatrix(Data, HierarchyOffset, {}, FVector::ZeroVector, OutMatrix);
}
