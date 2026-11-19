#include "KyaGeometryEncoding.h"
#include "KyaBinaryHelpers.h"
#include "KyaG3DChunks.h"
#include "KyaLevelBank.h"

namespace
{
bool RootCluster(TConstArrayView<uint8> Data, int32& OutTree, FString& Error)
{
    TArray<FKyaG3DChunk> Chunks;
    if (!KyaG3D::ReadChunks(Data, Chunks) || !(KyaReadU32(Data.GetData() + 4) & 1))
    { Error = TEXT("Render editing requires a prepared native G3D file."); return false; }
    const FKyaG3DChunk* Csta = Chunks.FindByPredicate([](const FKyaG3DChunk& C) { return C.Tag == 0x41545343; });
    if (!Csta || Csta->Size < 16 + 48 + 80)
    { Error = TEXT("No prepared static sector cluster is present."); return false; }
    const int32 Tree = Csta->Offset + 16, Root = Tree + 48;
    const uint32 TreeTag = KyaReadU32(Data.GetData() + Tree);
    if ((TreeTag != 0x41514443 && TreeTag != 0x414f4443) ||
        KyaReadU32(Data.GetData() + Tree + 8) != uint32(Csta->Size - 16) ||
        KyaReadU32(Data.GetData() + Root + 8) != uint32(Csta->Size - 64))
    { Error = TEXT("Multiple native cluster roots are not supported by this writer."); return false; }
    // A root with children requires rebuilding its implicit spatial subdivisions.
    // Keep Apply disabled for that format until the subdivision writer is ready.
    for (int64 Child = Root + 80; Child < int64(Csta->Offset) + Csta->Size;)
    {
        if (!KyaRangeValid(Data, Child, 16)) { Error = TEXT("Malformed root cluster."); return false; }
        const uint32 Tag = KyaReadU32(Data.GetData() + Child), Size = KyaReadU32(Data.GetData() + Child + 8);
        if (Size < 16 || Child + Size > int64(Csta->Offset) + Csta->Size)
        { Error = TEXT("Malformed root cluster child."); return false; }
        if (Tag == 0x55514443 || Tag == 0x434f4443)
        { Error = TEXT("Nested native render spatial trees are not yet supported for geometry replacement."); return false; }
        Child += Size;
    }
    OutTree = Tree; return true;
}

int32 RenderAllocate(TArray<uint8>& Data, int64 Size)
{
    const int64 Offset = Align(int64(Data.Num()), int64(16));
    if (Size < 0 || Offset + Align(Size, int64(16)) > MAX_int32) return INDEX_NONE;
    Data.SetNumZeroed(int32(Offset + Align(Size, int64(16)))); return int32(Offset);
}

void RenderHalf(TArray<uint8>& Data, int32 Offset, int32 Value)
{
    Data[Offset] = uint8(Value); Data[Offset + 1] = uint8(Value >> 8);
}

bool RenderQuantize(double Value, double Scale, int32& Out)
{
    if (!FMath::IsFinite(Value) || Value * Scale < -32768 || Value * Scale > 32767) return false;
    Out = FMath::RoundToInt(Value * Scale); return true;
}
}

bool KyaGeometry::OpenRootRender(UKyaLevelBank& Bank, int32 EntryIndex,
    TArray<FKyaGeometryDocument>& OutDocuments, FString& OutError)
{
    OutError.Reset();
    if (!Bank.Entries.IsValidIndex(EntryIndex)) { OutError = TEXT("Invalid mesh entry identity."); return false; }
    const FKyaBankEntry& Entry = Bank.Entries[EntryIndex];
    if (Entry.Type != 4 || Entry.Subtype != 1 || !KyaRangeValid(Bank.BankBytes, Entry.Offset, Entry.Size))
    { OutError = TEXT("Expected an in-range sector scenery mesh entry (4/1)."); return false; }
    const TConstArrayView<uint8> Data = MakeArrayView(Bank.BankBytes).Slice(int32(Entry.Offset), int32(Entry.Size));
    int32 Tree; if (!RootCluster(Data, Tree, OutError)) return false;
    TArray<KyaG3D::FClusterStripReference> References;
    if (!KyaG3D::ReadClusterTreeStrips(Data, References)) { OutError = TEXT("Invalid native cluster strips."); return false; }
    References.Sort([](const KyaG3D::FClusterStripReference& A, const KyaG3D::FClusterStripReference& B) { return A.StripOffset < B.StripOffset; });
    TArray<FKyaGeometryDocument> Documents;
    TMap<int32, int32> Relocations; TArray<int32> InvalidRelocations;
    const bool RelocationsValid = KyaG3D::ReadRelocations(Data, Relocations, InvalidRelocations) && InvalidRelocations.IsEmpty();
    TSharedPtr<TArray<uint8>, ESPMode::ThreadSafe> Snapshot = MakeShared<TArray<uint8>, ESPMode::ThreadSafe>();
    Snapshot->Append(Data.GetData(), Data.Num());
    for (const KyaG3D::FClusterStripReference& Reference : References)
    {
        const int32 Offset = Reference.StripOffset;
        if (!Documents.IsEmpty() && Documents.Last().NativeResourceOffset == Offset) continue;
        FKyaG3DDecodedStrip Strip; TArray<FKyaG3DVertex> Vertices; TArray<uint32> Sections;
        uint32 Count, Primitive;
        if (!KyaG3D::ReadFirstStripGifTag(Data, Offset, Count, Primitive) ||
            !KyaG3D::ReadStripVertices(Data, Offset, Vertices, Sections) ||
            !KyaG3D::ReadStripAttributes(Data, Offset, Strip.Attributes))
        { OutError = TEXT("Unsupported native root strip packet."); return false; }
        const uint32 NormalBase = KyaReadU32(Data.GetData() + Offset + 44);
        if (NormalBase && !KyaRangeValid(Data, NormalBase, int64(Vertices.Num()) * 8))
        { OutError = TEXT("Invalid native strip normal buffer."); return false; }
        uint32 Base = 0;
        for (int32 Section = 0; Section < Sections.Num(); ++Section)
        {
            for (uint32 I = 0; I < Sections[Section]; ++I)
            {
                Strip.Vertices.Add(Vertices[Base + I]);
                if (NormalBase)
                {
                    const uint8* N = Data.GetData() + NormalBase + (Base + I) * 8;
                    const auto S16 = [N](int32 P) { return int16(uint16(N[P]) | (uint16(N[P + 1]) << 8)); };
                    Strip.Normals.Add(FVector3f(S16(0), S16(2), S16(4)) / 32768.f);
                }
            }
            Base += Sections[Section]; if (Section + 1 < Sections.Num()) Base -= 2;
        }
        Strip.MaterialIndex = int16(uint16(Data[Offset + 4]) | (uint16(Data[Offset + 5]) << 8));
        const uint32 ExpandedCount = Strip.Vertices.Num();
        if (!KyaG3D::BuildStripTriangles(Strip.Vertices, MakeArrayView(&ExpandedCount, 1), Primitive, Strip.Indices))
        { OutError = TEXT("Root strip is not a supported native triangle strip."); return false; }
        FKyaGeometryResourceKey Key; Key.Bank = &Bank; Key.EntryIndex = EntryIndex; Key.ResourceIndex = Documents.Num();
        Key.LodIndex = 0; Key.Generation = Bank.EditGeneration;
        FKyaGeometryDocument Document;
        if (!FromDecodedStrip(Key, Strip, Document, OutError)) return false;
        Document.NativeResourceOffset = Offset; Document.bPackedPositions = (KyaReadU32(Data.GetData() + Offset) & 0x400) != 0;
        Document.SourceEntryBytes.Reset(); Document.SharedEntryBytes = Snapshot;
        Document.NativeFormatDiagnostic.Reset();
        const uint32 Flags = KyaReadU32(Data.GetData() + Offset);
        const uint32 UV = KyaReadU32(Data.GetData() + Offset + 32);
        if (!RelocationsValid) Document.NativeFormatDiagnostic = TEXT("Native relocation layout is unsupported or contains out-of-range references.");
        else if (Flags & (4 | 0x10000)) Document.NativeFormatDiagnostic = TEXT("Animated or skinned native strips are outside sector editing scope.");
        else if (!KyaRangeValid(Data, UV, 16) || KyaReadU32(Data.GetData() + UV + 4) != 0)
            Document.NativeFormatDiagnostic = TEXT("This native UV layer-stride layout is not yet supported by the render writer.");
        Documents.Add(MoveTemp(Document));
    }
    OutDocuments = MoveTemp(Documents); return true;
}

bool KyaGeometry::EncodeRootRender(TConstArrayView<FKyaGeometryDocument> Documents, TArray<uint8>& OutBytes, FString& OutError)
{
    OutError.Reset();
    if (Documents.IsEmpty()) { OutError = TEXT("No render resources were supplied."); return false; }
    const FKyaGeometryDocument& First = Documents[0];
    if (!MatchesSource(First)) { OutError = TEXT("Stale render geometry session."); return false; }
    const TArray<uint8>& Original = First.GetSourceEntryBytes();
    int32 Tree; if (!RootCluster(Original, Tree, OutError)) return false;
    TArray<FKyaGeometryDocument> Baseline;
    if (!OpenRootRender(*First.Key.Bank.Get(), First.Key.EntryIndex, Baseline, OutError)) return false;
    TArray<FKyaG3DChunk> Chunks; KyaG3D::ReadChunks(Original, Chunks);
    const FKyaG3DChunk* Relocations = Chunks.FindByPredicate([](const FKyaG3DChunk& C) { return C.Tag == 0x41414552; });
    if (!Relocations || (Original.Num() & 15)) { OutError = TEXT("Unsupported render relocation/alignment layout."); return false; }
    TArray<uint8> Bytes = Original, NewRelocations;
    NewRelocations.Append(Original.GetData() + Relocations->Offset, Relocations->Size);
    KyaWriteU32(Bytes, Relocations->Offset, 0x4b4e554a); // Retired REAA becomes an ignored JUNK chunk.
    const int32 EditChunk = RenderAllocate(Bytes, 16);
    TArray<int32> AddedFields, RemovedFields; TSet<int32> Seen;
    FBox RootBounds(ForceInit);
    for (const FKyaGeometryDocument& Document : Documents)
    {
        if (!Document.NativeFormatDiagnostic.IsEmpty()) { OutError = Document.NativeFormatDiagnostic; return false; }
        if (Document.Key.Bank != First.Key.Bank || Document.Key.EntryIndex != First.Key.EntryIndex ||
            !MatchesSource(Document) || !Baseline.IsValidIndex(Document.Key.ResourceIndex) ||
            Baseline[Document.Key.ResourceIndex].NativeResourceOffset != Document.NativeResourceOffset || Seen.Contains(Document.NativeResourceOffset))
        { OutError = TEXT("Render resource identities are stale, duplicated or refer to different entries."); return false; }
        TArray<FKyaGeometryIssue> Issues;
        if (!Validate(Document, Issues)) { OutError = TEXT("Render geometry fails topology or attribute validation."); return false; }
        const int32 Strip = Document.NativeResourceOffset; Seen.Add(Strip);
        const uint32 Flags = KyaReadU32(Original.GetData() + Strip);
        const bool Packed = (Flags & 0x400) != 0;
        const int32 Stride = Packed ? 8 : 16;
        const int32 OldUV = KyaReadU32(Original.GetData() + Strip + 32), OldColor = KyaReadU32(Original.GetData() + Strip + 36);
        const int32 OldPosition = KyaReadU32(Original.GetData() + Strip + 40), OldNormal = KyaReadU32(Original.GetData() + Strip + 44);
        if ((Flags & (4 | 0x10000)) || !KyaRangeValid(Original, OldUV, 16))
        { OutError = TEXT("Animated, skinned or malformed native strip formats are not supported by this writer."); return false; }
        if (KyaReadU32(Original.GetData() + OldUV + 4) != 0)
        { OutError = TEXT("This native UV layer-stride layout is not yet supported by the render writer."); return false; }
        const int32 Material = int16(uint16(Original[Strip + 4]) | (uint16(Original[Strip + 5]) << 8));
        for (const FKyaGeometryFace& Face : Document.Faces)
        {
            if (Face.MaterialIndex.GetValue() != Material)
            { OutError = TEXT("New faces must use this native strip's existing material slot."); return false; }
            for (const FKyaGeometryCorner& Corner : Face.Corners)
                if (!Corner.UV.IsSet() || !Corner.Color.IsSet() || (OldNormal && !Corner.Normal.IsSet()))
                { OutError = TEXT("Native strip corner attributes are incomplete."); return false; }
        }
        const int32 OldPacket = Strip + int32(KyaReadU32(Original.GetData() + Strip + 8));
        uint32 OldCount, Primitive;
        if (!KyaG3D::ReadFirstStripGifTag(Original, Strip, OldCount, Primitive) || OldCount < 3 || OldCount > 96)
        { OutError = TEXT("Unsupported native VIF section size."); return false; }
        int32 PacketSize = 0;
        while (KyaRangeValid(Original, OldPacket + PacketSize, 16))
        {
            PacketSize += 16;
            if (KyaReadU32(Original.GetData() + OldPacket + PacketSize - 16) == 0x60000000) break;
        }
        if (PacketSize < 32 || PacketSize > 256 || KyaReadU32(Original.GetData() + OldPacket + PacketSize - 16) != 0x60000000)
        { OutError = TEXT("Unsupported native VIF packet template."); return false; }
        // The engine's prepared static DMA budget is 96 vertices. Every face is
        // an isolated strip triangle, with two shared boundary vertices per section.
        const int32 Capacity = 96; // gNbVertexDMA for prepared static scenery.
        TArray<FKyaGeometryCorner> Corners; TArray<int32> Counts, Starts, AttributeStarts;
        int32 FaceIndex = 0, AttributeCount = 0;
        do
        {
            const int32 Start = Corners.IsEmpty() ? 0 : Corners.Num() - 2;
            const int32 Prefix = Corners.IsEmpty() ? 0 : 2;
            const int32 Faces = FMath::Min((Capacity - Prefix) / 3, Document.Faces.Num() - FaceIndex);
            if (!Document.Faces.IsEmpty() && Faces <= 0) { OutError = TEXT("Native packet splitting failed."); return false; }
            Starts.Add(Start); AttributeStarts.Add(AttributeCount); Counts.Add(Prefix + Faces * 3);
            for (int32 I = 0; I < Faces; ++I)
            {
                const FKyaGeometryFace& Face = Document.Faces[FaceIndex++];
                const bool Reverse = ((Corners.Num() + 2) & 1) != 0;
                for (int32 J = 0; J < 3; ++J)
                {
                    FKyaGeometryCorner Corner = Face.Corners[(Reverse && J < 2) ? 1 - J : J];
                    Corner.NativeVertexFlags = (Corner.NativeVertexFlags & ~0x8000u) | (J < 2 ? 0x8000u : 0u);
                    Corners.Add(Corner);
                }
            }
            AttributeCount += Counts.Last();
        } while (FaceIndex < Document.Faces.Num());
        // The native renderer requires a nonempty section even for an empty
        // resource. Use a skipped triangle, which decodes to no authored faces.
        if (Corners.IsEmpty())
        {
            for (int32 I = 0; I < 3; ++I)
            {
                FKyaGeometryCorner Corner; Corner.VertexIndex = INDEX_NONE; Corner.UV = FVector2f::ZeroVector;
                Corner.Color = FColor::White; Corner.Normal = FVector3f(0, 1, 0); Corner.NativeVertexFlags = 0x8000; Corners.Add(Corner);
            }
            Counts[0] = AttributeCount = 3;
        }
        if (Counts.Num() > 32767) { OutError = TEXT("Native strip section count overflow."); return false; }
        const int32 Position = RenderAllocate(Bytes, int64(Corners.Num()) * Stride);
        const int32 UV = RenderAllocate(Bytes, 16 + int64(AttributeCount) * 4);
        const int32 Color = RenderAllocate(Bytes, int64(AttributeCount) * 4);
        const int32 Normal = OldNormal ? RenderAllocate(Bytes, int64(Corners.Num()) * 8) : 0;
        const int32 Packet = RenderAllocate(Bytes, int64(Counts.Num()) * PacketSize);
        if (Position < 0 || UV < 0 || Color < 0 || Normal < 0 || Packet < 0)
        { OutError = TEXT("Native render buffer size overflow."); return false; }
        FMemory::Memcpy(Bytes.GetData() + UV, Original.GetData() + OldUV, 16);
        FBox Bounds(ForceInit);
        for (int32 I = 0; I < Corners.Num(); ++I)
        {
            const FKyaGeometryCorner& Corner = Corners[I];
            const FVector V = Corner.VertexIndex == INDEX_NONE ? FVector::ZeroVector : Document.Vertices[Corner.VertexIndex];
            Bounds += V;
            for (int32 Axis = 0; Axis < 3; ++Axis)
            {
                if (Packed)
                {
                    int32 Q; if (!RenderQuantize(V[Axis], 4096, Q)) { OutError = TEXT("Vertex exceeds native packed position range."); return false; }
                    RenderHalf(Bytes, Position + I * Stride + Axis * 2, Q);
                }
                else
                {
                    if (!FMath::IsFinite(float(V[Axis]))) { OutError = TEXT("Vertex overflows native float32 storage."); return false; }
                    KyaWriteF32(Bytes, Position + I * Stride + Axis * 4, float(V[Axis]));
                }
                if (OldNormal)
                {
                    if (!Corner.Normal.IsSet()) { OutError = TEXT("Native strip requires corner normals."); return false; }
                    const double N = Corner.Normal.GetValue()[Axis];
                    if (N < -1 || N > 1) { OutError = TEXT("Normal exceeds native normalized encoding range."); return false; }
                    RenderHalf(Bytes, Normal + I * 8 + Axis * 2, FMath::Clamp(FMath::RoundToInt(N * 32768), -32768, 32767));
                }
            }
            if (Packed) RenderHalf(Bytes, Position + I * Stride + 6, Corner.NativeVertexFlags);
            else KyaWriteU32(Bytes, Position + I * Stride + 12, Corner.NativeVertexFlags);
        }
        int32 Attribute = 0;
        for (int32 Section = 0; Section < Counts.Num(); ++Section)
        {
            const int32 Count = Counts[Section], Start = Starts[Section];
            for (int32 I = 0; I < Count; ++I, ++Attribute)
            {
                const FKyaGeometryCorner& Corner = Corners[Start + I];
                for (int32 Axis = 0; Axis < 2; ++Axis)
                {
                    int32 Q; if (!RenderQuantize(Corner.UV.GetValue()[Axis], 4096, Q)) { OutError = TEXT("UV exceeds native signed 4.12 range."); return false; }
                    RenderHalf(Bytes, UV + 16 + Attribute * 4 + Axis * 2, Q);
                }
                const FColor C = Corner.Color.GetValue();
                Bytes[Color + Attribute * 4] = C.R; Bytes[Color + Attribute * 4 + 1] = C.G;
                Bytes[Color + Attribute * 4 + 2] = C.B; Bytes[Color + Attribute * 4 + 3] = C.A;
            }
            const int32 Gif = RenderAllocate(Bytes, 16), P = Packet + Section * PacketSize;
            const int32 OldGif = KyaReadU32(Original.GetData() + OldPacket + 20);
            FMemory::Memcpy(Bytes.GetData() + Gif, Original.GetData() + OldGif, 16);
            KyaWriteU32(Bytes, Gif, (KyaReadU32(Bytes.GetData() + Gif) & ~0x7fffu) | Count);
            FMemory::Memcpy(Bytes.GetData() + P, Original.GetData() + OldPacket, PacketSize);
            bool GifMapped = false, UVMapped = false, ColorMapped = false, PositionMapped = false, NormalMapped = OldNormal == 0;
            for (int32 Command = 0; Command < PacketSize; Command += 16)
            {
                const uint32 Tag = KyaReadU32(Bytes.GetData() + P + Command);
                if ((Tag & 0x70000000) != 0x30000000) continue;
                const uint32 Address = KyaReadU32(Bytes.GetData() + P + Command + 4);
                uint32 Target = Address; int32 ElementSize = 0;
                if (Address == uint32(OldGif)) { Target = Gif; GifMapped = true; }
                else if (Address == uint32(OldUV + 16)) { Target = UV + 16 + AttributeStarts[Section] * 4; ElementSize = 4; UVMapped = true; }
                else if (Address == uint32(OldColor)) { Target = Color + AttributeStarts[Section] * 4; ElementSize = 4; ColorMapped = true; }
                else if (Address == uint32(OldPosition)) { Target = Position + Start * Stride; ElementSize = Stride; PositionMapped = true; }
                else if (OldNormal && Address == uint32(OldNormal)) { Target = Normal + Start * 8; ElementSize = 8; NormalMapped = true; }
                if (ElementSize)
                {
                    KyaWriteU32(Bytes, P + Command, (Tag & 0xffff0000u) | uint32((Count * ElementSize + 15) / 16));
                    const uint32 Unpack = KyaReadU32(Bytes.GetData() + P + Command + 12);
                    KyaWriteU32(Bytes, P + Command + 12, (Unpack & ~0x00ff0000u) | (uint32(Count) << 16));
                }
                KyaWriteU32(Bytes, P + Command + 4, Target); AddedFields.Add(P + Command + 4);
            }
            if (!GifMapped || !UVMapped || !ColorMapped || !PositionMapped || !NormalMapped)
            { OutError = TEXT("Native VIF packet has an unsupported attribute reference layout."); return false; }
        }
        KyaWriteU32(Bytes, Strip + 8, Packet - Strip);
        KyaWriteU32(Bytes, Strip + 32, UV); KyaWriteU32(Bytes, Strip + 36, Color);
        KyaWriteU32(Bytes, Strip + 40, Position); KyaWriteU32(Bytes, Strip + 44, Normal);
        KyaWriteU32(Bytes, Strip + 60, 0); RemovedFields.Add(Strip + 60);
        RenderHalf(Bytes, Strip + 58, Counts.Num()); RenderHalf(Bytes, Strip + 6, 0);
        const FVector Center = Bounds.GetCenter(); const double Radius = Bounds.GetExtent().Size();
        if (Center.ContainsNaN() || !FMath::IsFinite(float(Radius)))
        { OutError = TEXT("Render bounds overflow native float32 storage."); return false; }
        for (int32 Axis = 0; Axis < 3; ++Axis) KyaWriteF32(Bytes, Strip + 16 + Axis * 4, float(Center[Axis]));
        KyaWriteF32(Bytes, Strip + 28, float(Radius + 1.e-4)); RootBounds += Bounds;
    }
    // Root strips have no implicit child partitions. Expand the existing root
    // dimensions around its unchanged native centre; retain all placements.
    const int32 RootCenter = Tree + 32;
    if (RootBounds.IsValid)
        for (int32 Axis = 0; Axis < 3; ++Axis)
        {
            const double Center = KyaReadF32(Bytes.GetData() + RootCenter + Axis * 4);
            const double Size = KyaReadF32(Bytes.GetData() + Tree + 16 + Axis * 4);
            const double Required = 2 * FMath::Max(FMath::Abs(RootBounds.Min[Axis] - Center), FMath::Abs(RootBounds.Max[Axis] - Center));
            if (!FMath::IsFinite(float(Required))) { OutError = TEXT("Render spatial bounds overflow float32 storage."); return false; }
            KyaWriteF32(Bytes, Tree + 16 + Axis * 4, float(FMath::Max(Size, Required) + 1.e-4));
        }
    KyaWriteU32(Bytes, EditChunk, 0x54494445); KyaWriteU32(Bytes, EditChunk + 4, 1);
    KyaWriteU32(Bytes, EditChunk + 8, Bytes.Num() - EditChunk); KyaWriteU32(Bytes, EditChunk + 12, Bytes.Num() - EditChunk);
    for (int32 Chunk = 16; Chunk < NewRelocations.Num();)
    {
        const uint32 Size = KyaReadU32(NewRelocations.GetData() + Chunk + 8);
        if (Size < 16 || Size > uint32(NewRelocations.Num() - Chunk)) { OutError = TEXT("Malformed relocation chunk."); return false; }
        for (int32 P = Chunk + 16; P < Chunk + int32(Size); P += 4)
            if (RemovedFields.Contains(int32(KyaReadU32(NewRelocations.GetData() + P)))) KyaWriteU32(NewRelocations, P, 0);
        Chunk += Size;
    }
    for (int32 Start = 0; Start < AddedFields.Num(); Start += 65532)
    {
        const int32 Count = FMath::Min(65532, AddedFields.Num() - Start);
        const int32 Chunk = RenderAllocate(NewRelocations, 16 + int64(Count) * 4);
        KyaWriteU32(NewRelocations, Chunk, 0x4c414552); KyaWriteU32(NewRelocations, Chunk + 4, 1);
        KyaWriteU32(NewRelocations, Chunk + 8, NewRelocations.Num() - Chunk); KyaWriteU32(NewRelocations, Chunk + 12, NewRelocations.Num() - Chunk);
        for (int32 I = 0; I < Count; ++I) KyaWriteU32(NewRelocations, Chunk + 16 + I * 4, AddedFields[Start + I]);
    }
    KyaWriteU32(NewRelocations, 8, NewRelocations.Num()); KyaWriteU32(NewRelocations, 12, NewRelocations.Num());
    Bytes.Append(NewRelocations); KyaWriteU32(Bytes, 8, Bytes.Num());
    TMap<int32, int32> Resolved; TArray<int32> Invalid;
    if (!KyaG3D::ReadRelocations(Bytes, Resolved, Invalid) || !Invalid.IsEmpty())
    { OutError = TEXT("Rebuilt render relocation validation failed."); return false; }
    UKyaLevelBank* Check = NewObject<UKyaLevelBank>(); Check->BankBytes = Bytes;
    FKyaBankEntry Entry; Entry.Type = 4; Entry.Subtype = 1; Entry.Size = Bytes.Num(); Check->Entries.Add(Entry);
    TArray<FKyaGeometryDocument> Reopened;
    if (!OpenRootRender(*Check, 0, Reopened, OutError)) return false;
    for (const FKyaGeometryDocument& Document : Documents)
    {
        TArray<FKyaGeometryIssue> Issues;
        if (!Reopened.IsValidIndex(Document.Key.ResourceIndex) ||
            Reopened[Document.Key.ResourceIndex].Faces.Num() != Document.Faces.Num() ||
            !Validate(Reopened[Document.Key.ResourceIndex], Issues))
        { OutError = TEXT("Rebuilt render strip failed topology decode-back validation."); return false; }
        const FKyaGeometryDocument& Decoded = Reopened[Document.Key.ResourceIndex];
        const bool Packed = (KyaReadU32(Original.GetData() + Document.NativeResourceOffset) & 0x400) != 0;
        for (int32 I = 0; I < Document.Faces.Num(); ++I)
            for (int32 J = 0; J < 3; ++J)
            {
                const FKyaGeometryCorner& Expected = Document.Faces[I].Corners[J];
                const FKyaGeometryCorner& Actual = Decoded.Faces[I].Corners[J];
                FVector Position = Document.Vertices[Expected.VertexIndex];
                for (int32 Axis = 0; Axis < 3; ++Axis)
                    Position[Axis] = Packed ? double(FMath::RoundToInt(Position[Axis] * 4096)) / 4096 : double(float(Position[Axis]));
                if (!Decoded.Vertices[Actual.VertexIndex].Equals(Position, 1.e-8) ||
                    !Actual.UV.GetValue().Equals(Expected.UV.GetValue(), 1.f / 4096) ||
                    Actual.Color.GetValue() != Expected.Color.GetValue() ||
                    (Actual.NativeVertexFlags & ~0x8000u) != (Expected.NativeVertexFlags & ~0x8000u) ||
                    (Document.bRequiresNormals && (!Actual.Normal.IsSet() || !Expected.Normal.IsSet() ||
                        !Actual.Normal.GetValue().Equals(Expected.Normal.GetValue(), 1.f / 32768))))
                { OutError = TEXT("Rebuilt render attributes or oriented faces failed decode-back comparison."); return false; }
            }
    }
    OutBytes = MoveTemp(Bytes); return true;
}
