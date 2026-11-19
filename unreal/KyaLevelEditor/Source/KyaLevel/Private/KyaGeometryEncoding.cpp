#include "KyaGeometryEncoding.h"
#include "KyaBinaryHelpers.h"
#include "KyaLevelBank.h"

namespace
{
struct FCollisionLeaf
{
    uint8 Type = 0;
    int32 First = 0;
    int32 Count = 0;
    FBox Bounds = FBox(ForceInit);
};

struct FCollisionNode
{
    FBox Bounds = FBox(ForceInit);
    uint8 Type = 1;
    int32 First = 0;
    int32 Count = 0;
    TArray<int32> Children;
};

int32 AddAligned(TArray<uint8>& Bytes, int64 Size, int64 Alignment = 16)
{
    const int64 Offset = Align(int64(Bytes.Num()), Alignment);
    if (Size < 0 || Offset + Size > MAX_int32) return INDEX_NONE;
    Bytes.SetNumZeroed(int32(Offset + Size)); return int32(Offset);
}

bool NativeCoordinate(const FVector& V)
{
    return FMath::IsFinite(float(V.X)) && FMath::IsFinite(float(V.Y)) && FMath::IsFinite(float(V.Z));
}
}

bool KyaGeometry::EncodeCollision(const FKyaGeometryDocument& Document, TArray<uint8>& OutBytes, FString& OutError)
{
    OutError.Reset();
    TArray<FKyaGeometryIssue> Issues;
    if (Document.Key.Kind != EKyaGeometryKind::Collision || !MatchesSource(Document))
    {
        OutError = TEXT("Collision document is stale or refers to a different native resource."); return false;
    }
    if (!Validate(Document, Issues))
    {
        const FKyaGeometryIssue* Issue = Issues.FindByPredicate([](const FKyaGeometryIssue& Value) { return Value.bError; });
        OutError = FString::Printf(TEXT("Collision face %d, vertex %d: %s"), Issue->FaceIndex, Issue->VertexIndex, *Issue->Message);
        return false;
    }
    const TArray<uint8>& Original = Document.GetSourceEntryBytes();
    if (Original.Num() < 104 || FMemory::Memcmp(Original.GetData() + 12, ".COLCOLI", 8) != 0 || Original[33])
    {
        OutError = TEXT("Unsupported or already installed native COL format."); return false;
    }
    const auto Word = [&Original](int32 Offset) { return KyaReadU32(Original.GetData() + Offset); };
    // The loader accepts other tree leaf formats (notably fast triangles, type 5).
    // Never silently drop a format the editable decoder does not represent.
    const uint32 OldNodes = Word(64), OldTree = Word(100);
    if (!KyaRangeValid(Original, OldTree, int64(OldNodes) * 0x70))
    {
        OutError = TEXT("Invalid source collision tree."); return false;
    }
    for (uint32 I = 0; I < OldNodes; ++I)
    {
        const uint32 Node = OldTree + I * 0x70;
        const uint8 Type = Original[Node + 0x51], Count = Original[Node + 0x52];
        if (Type != 1 && Type != 4 && Type != 8 && Type != 10 && Type != 11)
        {
            OutError = FString::Printf(TEXT("Unsupported collision tree leaf format %u."), Type); return false;
        }
        if (Type == 1)
        {
            if (Count > 7) { OutError = TEXT("Collision branch exceeds its seven native child slots."); return false; }
            for (uint32 J = 0; J < Count; ++J)
                if (KyaReadU32(Original.GetData() + Node + 0x54 + J * 4) >= OldNodes)
                { OutError = TEXT("Collision tree has an invalid child reference."); return false; }
        }
        else if (Type == 4 || Type == 8)
        {
            const uint64 First = KyaReadU32(Original.GetData() + Node + 0x54);
            if (First + Count > Word(Type == 4 ? 40 : 48))
            { OutError = TEXT("Collision tree has an invalid polygon reference."); return false; }
        }
    }
    for (const FVector& V : Document.Vertices)
        if (!NativeCoordinate(V)) { OutError = TEXT("Collision coordinates overflow native float32 storage."); return false; }
    FKyaGeometryDocument Baseline;
    if (!OpenCollision(*Document.Key.Bank.Get(), Document.Key.EntryIndex, Baseline, OutError) ||
        Baseline.RetainedPrimitives.Num() != Document.RetainedPrimitives.Num())
    {
        OutError = TEXT("Adding or removing native box/sphere primitives is unsupported."); return false;
    }
    for (int32 I = 0; I < Baseline.RetainedPrimitives.Num(); ++I)
    {
        const FKyaPrimitiveRecord& A = Baseline.RetainedPrimitives[I];
        const FKyaPrimitiveRecord& B = Document.RetainedPrimitives[I];
        if (A.SourceOffset != B.SourceOffset || A.Shape != B.Shape || A.Flags != B.Flags || A.PrimitiveIndex != B.PrimitiveIndex ||
            !A.LocalToGame.Equals(B.LocalToGame, 0))
        {
            OutError = TEXT("Reshaping or altering retained box/sphere primitives is unsupported."); return false;
        }
    }
    TArray<FKyaGeometryFace> NativeFaces;
    for (const FKyaGeometryFace& Face : Document.Faces)
    {
        bool bRetainQuad = Face.Corners.Num() == 4 && Baseline.Faces.IsValidIndex(Face.SourceFaceIndex);
        if (bRetainQuad)
        {
            const FKyaGeometryFace& SourceFace = Baseline.Faces[Face.SourceFaceIndex];
            bRetainQuad = SourceFace.Corners.Num() == 4 && SourceFace.SourceOffset == Face.SourceOffset;
            for (int32 J = 0; J < 4 && bRetainQuad; ++J)
                bRetainQuad = Face.Corners[J].SourceVertexOffset == SourceFace.Corners[J].SourceVertexOffset &&
                    Face.Corners[J].VertexIndex == Face.Corners[J].SourceVertexIndex;
        }
        if (Face.Corners.Num() == 4 && !bRetainQuad)
        {
            for (int32 I = 1; I < 3; ++I)
            {
                FKyaGeometryFace Triangle = Face;
                Triangle.Corners = {Face.Corners[0], Face.Corners[I], Face.Corners[I + 1]};
                NativeFaces.Add(MoveTemp(Triangle));
            }
        }
        else NativeFaces.Add(Face);
    }

    // Retain opaque data and box/sphere records verbatim. Append rebuilt arrays so
    // all old box/sphere offsets remain valid; only the root header is repointed.
    TArray<uint8> Bytes = Original;
    const int32 VertexBase = AddAligned(Bytes, int64(Document.Vertices.Num()) * 16);
    if (VertexBase == INDEX_NONE) { OutError = TEXT("Collision vertex data exceeds native size limits."); return false; }
    TMap<int32, int32> VertexSources;
    for (const FKyaGeometryFace& Face : NativeFaces)
        for (const FKyaGeometryCorner& Corner : Face.Corners)
            if (KyaRangeValid(Original, Corner.SourceVertexOffset, 16)) VertexSources.FindOrAdd(Corner.VertexIndex, Corner.SourceVertexOffset);
    for (int32 I = 0; I < Document.Vertices.Num(); ++I)
    {
        const FVector& V = Document.Vertices[I];
        if (const int32* Source = VertexSources.Find(I))
            FMemory::Memcpy(Bytes.GetData() + VertexBase + I * 16, Original.GetData() + *Source, 16);
        else KyaWriteF32(Bytes, VertexBase + I * 16 + 12, 1);
        KyaWriteF32(Bytes, VertexBase + I * 16, float(V.X));
        KyaWriteF32(Bytes, VertexBase + I * 16 + 4, float(V.Y));
        KyaWriteF32(Bytes, VertexBase + I * 16 + 8, float(V.Z));
    }
    TArray<FCollisionLeaf> Leaves;
    for (int32 Corners : {3, 4})
    {
        TArray<int32> Faces;
        for (int32 I = 0; I < NativeFaces.Num(); ++I)
            if (NativeFaces[I].Corners.Num() == Corners) Faces.Add(I);
        // Sort by centroid on the longest scene axis, with original index as the
        // tie breaker. Face storage and leaf contiguous ranges are built together.
        FBox Bounds(ForceInit);
        for (int32 I : Faces)
            for (const FKyaGeometryCorner& C : NativeFaces[I].Corners) Bounds += Document.Vertices[C.VertexIndex];
        const FVector Extent = Bounds.IsValid ? Bounds.GetExtent() : FVector::ZeroVector;
        const int32 Axis = Extent.X >= Extent.Y && Extent.X >= Extent.Z ? 0 : (Extent.Y >= Extent.Z ? 1 : 2);
        const auto Center = [&Document, &NativeFaces, Axis](int32 I)
        {
            double Sum = 0; for (const FKyaGeometryCorner& C : NativeFaces[I].Corners) Sum += Document.Vertices[C.VertexIndex][Axis];
            return Sum / NativeFaces[I].Corners.Num();
        };
        Faces.Sort([&Center](int32 A, int32 B) { const double X = Center(A), Y = Center(B); return X == Y ? A < B : X < Y; });
        const int32 Stride = (Corners + 1) * 4;
        const int32 Base = AddAligned(Bytes, int64(Faces.Num()) * Stride);
        if (Base == INDEX_NONE) { OutError = TEXT("Collision faces exceed native size limits."); return false; }
        KyaWriteU32(Bytes, Corners == 3 ? 40 : 48, Faces.Num());
        KyaWriteU32(Bytes, Corners == 3 ? 76 : 84, Base);
        for (int32 I = 0; I < Faces.Num(); ++I)
        {
            if (I % 255 == 0)
            {
                FCollisionLeaf Leaf; Leaf.Type = Corners == 3 ? 4 : 8; Leaf.First = I; Leaves.Add(Leaf);
            }
            const FKyaGeometryFace& Face = NativeFaces[Faces[I]];
            for (int32 J = 0; J < Corners; ++J)
            {
                KyaWriteU32(Bytes, Base + I * Stride + J * 4, Face.Corners[J].VertexIndex);
                // Bounds use quantized values, expanded below for float rounding.
                Leaves.Last().Bounds += FVector(FVector3f(Document.Vertices[Face.Corners[J].VertexIndex]));
            }
            KyaWriteU32(Bytes, Base + I * Stride + Corners * 4, Face.SurfaceFlags.GetValue());
            ++Leaves.Last().Count;
        }
    }
    TArray<FKyaPrimitiveRecord> Retained = Document.RetainedPrimitives;
    Retained.Sort([](const FKyaPrimitiveRecord& A, const FKyaPrimitiveRecord& B)
    {
        return A.Shape == B.Shape ? A.PrimitiveIndex < B.PrimitiveIndex : uint8(A.Shape) < uint8(B.Shape);
    });
    for (const FKyaPrimitiveRecord& Primitive : Retained)
    {
        const uint8 Type = Primitive.Shape == EKyaPrimitiveShape::Box ? 10 : Primitive.Shape == EKyaPrimitiveShape::Sphere ? 11 : 0;
        if (!Type || !KyaRangeValid(Original, Primitive.SourceOffset, 0x90) ||
            int64(Word(Type == 10 ? 88 : 92)) + int64(Primitive.PrimitiveIndex) * 0x90 != Primitive.SourceOffset)
        {
            OutError = TEXT("Unsupported or altered retained collision primitive identity."); return false;
        }
        FCollisionLeaf Leaf; Leaf.Type = Type; Leaf.First = Primitive.PrimitiveIndex; Leaf.Count = 1;
        // An affine image of the enclosing unit cube conservatively encloses both
        // boxes and spheres, including nonuniform scale and shear.
        const double Radius = Type == 11 ? 1.0 : 0.5;
        for (int32 I = 0; I < 8; ++I)
            Leaf.Bounds += FVector(Primitive.LocalToGame.TransformPosition(FVector((I & 1) ? Radius : -Radius, (I & 2) ? Radius : -Radius, (I & 4) ? Radius : -Radius)));
        if (!Leaves.IsEmpty() && Leaves.Last().Type == Type && Leaves.Last().Count < 255 &&
            Leaves.Last().First + Leaves.Last().Count == Leaf.First)
        {
            Leaves.Last().Bounds += Leaf.Bounds; ++Leaves.Last().Count;
        }
        else Leaves.Add(Leaf);
    }
    // Native ray traversal uses two fixed queues of 100 nodes (OBBTree.cpp).
    // A width-limited tree prevents silent dropping even for broad queries.
    if (Leaves.Num() > 100)
    {
        OutError = TEXT("Collision exceeds the native traversal limit of 100 leaf nodes (255 polygons per leaf)."); return false;
    }
    TArray<FCollisionNode> Nodes;
    TFunction<int32(int32, int32)> Build = [&](int32 Start, int32 Count)
    {
        const int32 Index = Nodes.AddDefaulted();
        if (Count == 1)
        {
            const FCollisionLeaf& Leaf = Leaves[Start];
            Nodes[Index].Bounds = Leaf.Bounds; Nodes[Index].Type = Leaf.Type;
            Nodes[Index].First = Leaf.First; Nodes[Index].Count = Leaf.Count;
        }
        else if (Count > 1)
        {
            const int32 Left = Build(Start, Count / 2), Right = Build(Start + Count / 2, Count - Count / 2);
            Nodes[Index].Children = {Left, Right};
            Nodes[Index].Bounds = Nodes[Left].Bounds + Nodes[Right].Bounds;
        }
        else Nodes[Index].Bounds = FBox(FVector::ZeroVector, FVector::ZeroVector);
        return Index;
    };
    Build(0, Leaves.Num());
    const int32 Tree = AddAligned(Bytes, int64(Nodes.Num()) * 0x70);
    if (Tree == INDEX_NONE) { OutError = TEXT("Collision tree exceeds native size limits."); return false; }
    for (int32 I = 0; I < Nodes.Num(); ++I)
    {
        const FCollisionNode& Node = Nodes[I]; const int32 Offset = Tree + I * 0x70;
        const FVector Center = Node.Bounds.GetCenter(), Extent = Node.Bounds.GetExtent();
        if (!NativeCoordinate(Center) || !NativeCoordinate(Extent)) { OutError = TEXT("Collision bounds overflow float32 storage."); return false; }
        for (int32 J = 0; J < 4; ++J) KyaWriteF32(Bytes, Offset + J * 20, 1);
        for (int32 J = 0; J < 3; ++J)
        {
            KyaWriteF32(Bytes, Offset + 48 + J * 4, float(Center[J]));
            KyaWriteF32(Bytes, Offset + 64 + J * 4, float(Extent[J] + FMath::Abs(Center[J]) * 1.e-6 + 1.e-4));
        }
        Bytes[Offset + 0x51] = Node.Type;
        Bytes[Offset + 0x52] = uint8(Node.Type == 1 ? Node.Children.Num() : Node.Count);
        if (Node.Type == 1)
            for (int32 J = 0; J < Node.Children.Num(); ++J) KyaWriteU32(Bytes, Offset + 0x54 + J * 4, Node.Children[J]);
        else KyaWriteU32(Bytes, Offset + 0x54, Node.First);
    }
    KyaWriteU32(Bytes, 72, VertexBase); KyaWriteU32(Bytes, 64, Nodes.Num()); KyaWriteU32(Bytes, 100, Tree);
    KyaWriteU32(Bytes, 8, Bytes.Num()); KyaWriteU32(Bytes, 24, Bytes.Num() - 16); KyaWriteU32(Bytes, 28, Bytes.Num() - 16);
    UKyaLevelBank* Check = NewObject<UKyaLevelBank>();
    Check->BankBytes = Bytes; FKyaBankEntry Entry; Entry.Type = 7; Entry.Subtype = 1; Entry.Size = Bytes.Num(); Check->Entries.Add(Entry);
    FKyaGeometryDocument Decoded;
    if (!OpenCollision(*Check, 0, Decoded, OutError) || !Validate(Decoded, Issues) || Decoded.Faces.Num() != NativeFaces.Num() ||
        Decoded.RetainedPrimitives.Num() != Document.RetainedPrimitives.Num())
    {
        OutError = TEXT("Rebuilt collision failed decode-back validation. ") + OutError; return false;
    }
    OutBytes = MoveTemp(Bytes); return true;
}

bool KyaGeometry::ReplaceBankEntries(const UKyaLevelBank& Bank, const TMap<int32, TArray<uint8>>& Replacements,
    TArray<uint8>& OutBytes, TArray<FKyaBankEntry>& OutEntries, FString& OutError)
{
    OutError.Reset();
    const TArray<uint8>& Original = Bank.BankBytes;
    if (Original.Num() < 0x48 || FMemory::Memcmp(Original.GetData() + 8, "KNAB", 4) != 0 ||
        (KyaReadU32(Original.GetData() + 12) & 1))
    {
        OutError = TEXT("Entry replacement requires an unpacked native bank."); return false;
    }
    const uint32 Count = KyaReadU32(Original.GetData() + 0x28);
    const int64 Table = int64(KyaReadU32(Original.GetData() + 0x30)) + 8;
    const uint32 HeaderSize = KyaReadU32(Original.GetData() + 0x44);
    const uint32 EntryAlignment = KyaReadU32(Original.GetData() + 0x24);
    const uint32 CommonType = KyaReadU32(Original.GetData() + 0x14);
    const int64 Types = int64(KyaReadU32(Original.GetData() + 0x34)) + 8;
    if (!EntryAlignment || !FMath::IsPowerOfTwo(EntryAlignment) || EntryAlignment > MAX_int32 / 2 ||
        Count != uint32(Bank.Entries.Num()) || !KyaRangeValid(Original, Table, (int64(Count) + 1) * 16) ||
        HeaderSize > uint32(Original.Num()) || Table + (int64(Count) + 1) * 16 > HeaderSize ||
        (!CommonType && !KyaRangeValid(Original, Types, int64(Count) * 4)))
    {
        OutError = TEXT("Bank entry tables or header extent are invalid."); return false;
    }
    for (int32 I = 0; I < Bank.Entries.Num(); ++I)
    {
        const FKyaBankEntry& Entry = Bank.Entries[I]; const int64 Record = Table + int64(I) * 16;
        const uint32 Pair = CommonType ? CommonType : KyaReadU32(Original.GetData() + Types + int64(I) * 4);
        if (!KyaRangeValid(Original, Entry.Offset, Entry.Size) || Entry.Offset < HeaderSize ||
            Entry.Offset != KyaReadU32(Original.GetData() + Record) || Entry.Size != KyaReadU32(Original.GetData() + Record + 4) ||
            Entry.Type != int32(Pair >> 16) || Entry.Subtype != int32(Pair & 0xffff))
        {
            OutError = FString::Printf(TEXT("Bank entry %d differs from its retained table or overlaps the header."), I); return false;
        }
    }
    TArray<int32> Keys; Replacements.GetKeys(Keys); Keys.Sort();
    for (int32 Key : Keys)
        if (!Bank.Entries.IsValidIndex(Key)) { OutError = TEXT("Replacement entry identity is invalid."); return false; }
    TArray<uint8> Bytes = Original; TArray<FKyaBankEntry> Entries = Bank.Entries;
    bool bChanged = false;
    for (int32 Key : Keys)
    {
        const TArray<uint8>& Replacement = Replacements[Key]; FKyaBankEntry& Entry = Entries[Key];
        if (Entry.Size == Replacement.Num() && (!Entry.Size || FMemory::Memcmp(Original.GetData() + Entry.Offset, Replacement.GetData(), Replacement.Num()) == 0)) continue;
        const int32 Offset = AddAligned(Bytes, Replacement.Num(), EntryAlignment);
        if (Offset == INDEX_NONE) { OutError = TEXT("Replacement bank exceeds the native signed offset limit."); return false; }
        if (!Replacement.IsEmpty()) FMemory::Memcpy(Bytes.GetData() + Offset, Replacement.GetData(), Replacement.Num());
        Entry.Offset = Offset; Entry.Size = Replacement.Num();
        KyaWriteU32(Bytes, Table + int64(Key) * 16, Offset);
        KyaWriteU32(Bytes, Table + int64(Key) * 16 + 4, Replacement.Num());
        bChanged = true;
    }
    if (bChanged)
    {
        const int64 DataEnd = Align(int64(Bytes.Num()), int64(EntryAlignment));
        const int64 FileSize = Align(DataEnd, int64(0x800));
        if (FileSize > MAX_int32) { OutError = TEXT("Aligned bank exceeds the native signed size limit."); return false; }
        Bytes.SetNumZeroed(int32(FileSize));
        KyaWriteU32(Bytes, Table + int64(Count) * 16, DataEnd);
        KyaWriteU32(Bytes, 0x18, Bytes.Num());
        KyaWriteU32(Bytes, 0x1c, DataEnd - HeaderSize);
        for (int32 I = 0; I < Entries.Num(); ++I)
        {
            const FKyaBankEntry& Entry = Entries[I];
            const TArray<uint8>* Replacement = Replacements.Find(I);
            const uint8* Expected = Replacement ? Replacement->GetData() : Original.GetData() + Bank.Entries[I].Offset;
            if (!KyaRangeValid(Bytes, Entry.Offset, Entry.Size) ||
                (Entry.Size && FMemory::Memcmp(Bytes.GetData() + Entry.Offset, Expected, Entry.Size) != 0))
            {
                OutError = TEXT("Replacement bank payload verification failed."); return false;
            }
        }
    }
    OutBytes = MoveTemp(Bytes); OutEntries = MoveTemp(Entries); return true;
}

bool KyaGeometry::ValidateAppliedGeometry(const UKyaLevelBank& Bank, const TArray<uint8>& OutputBytes, FString& OutError)
{
    OutError.Reset(); if (Bank.AppliedGeometryResources.IsEmpty()) return true;
    UKyaLevelBank* Output = NewObject<UKyaLevelBank>(); Output->BankBytes = OutputBytes; Output->Entries = Bank.Entries;
    TArray<uint8> Checked; TArray<FKyaBankEntry> Entries;
    if (!ReplaceBankEntries(*Output, {}, Checked, Entries, OutError)) return false;
    TMap<int32, TArray<FKyaGeometryDocument>> RenderEntries;
    TSet<int32> Collisions;
    for (const FKyaAppliedGeometryResource& Resource : Bank.AppliedGeometryResources)
    {
        if (Resource.bCollision)
        {
            if (Resource.ResourceIndex != 0) { OutError = TEXT("Invalid applied collision resource identity."); return false; }
            if (Collisions.Contains(Resource.EntryIndex)) continue;
            Collisions.Add(Resource.EntryIndex);
            FKyaGeometryDocument Document;
            if (!OpenCollision(*Output, Resource.EntryIndex, Document, OutError) || !EncodeCollision(Document, Checked, OutError)) return false;
        }
        else
        {
            if (!RenderEntries.Contains(Resource.EntryIndex))
            {
                TArray<FKyaGeometryDocument> Documents;
                if (!OpenRootRender(*Output, Resource.EntryIndex, Documents, OutError)) return false;
                RenderEntries.Add(Resource.EntryIndex, MoveTemp(Documents));
            }
            const TArray<FKyaGeometryDocument>& Documents = RenderEntries[Resource.EntryIndex];
            if (!Documents.IsValidIndex(Resource.ResourceIndex)) { OutError = TEXT("Invalid applied render resource identity."); return false; }
            if (!EncodeRootRender(MakeArrayView(&Documents[Resource.ResourceIndex], 1), Checked, OutError)) return false;
        }
    }
    return true;
}
