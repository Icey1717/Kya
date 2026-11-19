#include "KyaGeometryDocument.h"
#include "KyaG3DChunks.h"
#include "KyaLevelBank.h"

namespace
{
bool EntryBytes(const FKyaGeometryResourceKey& Key, TConstArrayView<uint8>& Out)
{
    if (!Key.IsCurrent()) return false;
    const UKyaLevelBank& Bank = *Key.Bank.Get();
    const FKyaBankEntry& Entry = Bank.Entries[Key.EntryIndex];
    if (Entry.Offset < 0 || Entry.Size < 0 || Entry.Offset > Bank.BankBytes.Num() ||
        Entry.Size > Bank.BankBytes.Num() - Entry.Offset) return false;
    Out = MakeArrayView(Bank.BankBytes).Slice(int32(Entry.Offset), int32(Entry.Size));
    return true;
}

bool Finite(const FVector& V)
{
    return FMath::IsFinite(V.X) && FMath::IsFinite(V.Y) && FMath::IsFinite(V.Z);
}
}

bool FKyaGeometryResourceKey::IsCurrent() const
{
    const UKyaLevelBank* Value = Bank.Get();
    if (!Value || Value->EditGeneration != Generation || !Value->Entries.IsValidIndex(EntryIndex) || ResourceIndex < 0)
        return false;
    const FKyaBankEntry& Entry = Value->Entries[EntryIndex];
    return Entry.Subtype == 1 && (Kind == EKyaGeometryKind::Collision ?
        Entry.Type == 7 && ResourceIndex == 0 && LodIndex == INDEX_NONE : Entry.Type == 4 && LodIndex >= 0);
}

bool FKyaGeometryResourceKey::operator==(const FKyaGeometryResourceKey& Other) const
{
    return Bank == Other.Bank && EntryIndex == Other.EntryIndex && ResourceIndex == Other.ResourceIndex &&
        LodIndex == Other.LodIndex && Generation == Other.Generation && Kind == Other.Kind;
}

bool KyaGeometry::MatchesSource(const FKyaGeometryDocument& Document)
{
    TConstArrayView<uint8> Bytes;
    const TArray<uint8>& Source = Document.GetSourceEntryBytes();
    return EntryBytes(Document.Key, Bytes) && Bytes.Num() == Source.Num() &&
        (Bytes.IsEmpty() || FMemory::Memcmp(Bytes.GetData(), Source.GetData(), Bytes.Num()) == 0);
}

bool KyaGeometry::Validate(const FKyaGeometryDocument& Document, TArray<FKyaGeometryIssue>& Issues)
{
    Issues.Reset();
    const auto Report = [&Issues](bool bError, int32 Face, int32 Vertex, const TCHAR* Message)
    {
        FKyaGeometryIssue& Issue = Issues.AddDefaulted_GetRef();
        Issue.bError = bError; Issue.FaceIndex = Face; Issue.VertexIndex = Vertex; Issue.Message = Message;
    };
    for (int32 I = 0; I < Document.Vertices.Num(); ++I)
    {
        if (!Finite(Document.Vertices[I])) Report(true, INDEX_NONE, I, TEXT("Vertex coordinates must be finite."));
        else if (!FMath::IsFinite(float(Document.Vertices[I].X)) || !FMath::IsFinite(float(Document.Vertices[I].Y)) || !FMath::IsFinite(float(Document.Vertices[I].Z)))
            Report(true, INDEX_NONE, I, TEXT("Vertex exceeds native float32 storage."));
        else if (Document.bPackedPositions)
            for (int32 Axis = 0; Axis < 3; ++Axis)
                if (Document.Vertices[I][Axis] * 4096 < -32768 || Document.Vertices[I][Axis] * 4096 > 32767)
                { Report(true, INDEX_NONE, I, TEXT("Vertex exceeds native packed position range.")); break; }
    }
    TMap<uint64, TArray<int32>> Edges;
    for (int32 I = 0; I < Document.Faces.Num(); ++I)
    {
        const FKyaGeometryFace& Face = Document.Faces[I];
        const int32 N = Face.Corners.Num();
        if (N != 3 && !(Document.Key.Kind == EKyaGeometryKind::Collision && N == 4 && Face.SourceFaceIndex >= 0))
        {
            Report(true, I, INDEX_NONE, TEXT("Expected a triangle or a retained native collision quad."));
            continue;
        }
        if (Document.Key.Kind == EKyaGeometryKind::Render && (!Face.MaterialIndex.IsSet() || Face.MaterialIndex.GetValue() < 0))
            Report(true, I, INDEX_NONE, TEXT("Choose an existing material for this face."));
        if (Document.Key.Kind == EKyaGeometryKind::Collision && !Face.SurfaceFlags.IsSet())
            Report(true, I, INDEX_NONE, TEXT("Choose collision surface properties for this face."));
        bool bValid = true;
        TSet<int32> Used;
        for (const FKyaGeometryCorner& Corner : Face.Corners)
        {
            const int32 V = Corner.VertexIndex;
            if (!Document.Vertices.IsValidIndex(V) || Used.Contains(V))
            {
                Report(true, I, V, TEXT("Face has an invalid or repeated vertex reference."));
                bValid = false;
            }
            else if (!Finite(Document.Vertices[V])) bValid = false;
            Used.Add(V);
            if (Document.Key.Kind != EKyaGeometryKind::Render) continue;
            if ((Document.bRequiresNormals && !Corner.Normal.IsSet()) ||
                (Document.bRequiresUVs && !Corner.UV.IsSet()) || (Document.bRequiresColors && !Corner.Color.IsSet()))
                Report(true, I, V, TEXT("Required corner attributes are missing."));
            if (Corner.Normal.IsSet())
            {
                const FVector Normal(Corner.Normal.GetValue());
                if (!Finite(Normal) || Normal.SizeSquared() <= 1.e-12)
                    Report(true, I, V, TEXT("Corner normal must be finite and nonzero."));
            }
            if (Corner.UV.IsSet() && (!FMath::IsFinite(Corner.UV->X) || !FMath::IsFinite(Corner.UV->Y)))
                Report(true, I, V, TEXT("Corner UV must be finite."));
        }
        if (!bValid) continue;
        const FVector A = Document.Vertices[Face.Corners[0].VertexIndex];
        for (int32 J = 1; J + 1 < N; ++J)
        {
            const FVector Cross = FVector::CrossProduct(Document.Vertices[Face.Corners[J].VertexIndex] - A,
                Document.Vertices[Face.Corners[J + 1].VertexIndex] - A);
            if (!Finite(Cross) || Cross.SizeSquared() <= 1.e-20)
                Report(true, I, INDEX_NONE, TEXT("Face contains a degenerate triangle."));
        }
        for (int32 J = 0; J < N; ++J)
        {
            const uint32 U = Face.Corners[J].VertexIndex, V = Face.Corners[(J + 1) % N].VertexIndex;
            Edges.FindOrAdd((uint64(FMath::Min(U, V)) << 32) | FMath::Max(U, V)).Add(I);
        }
    }
    // Sort for repeatable diagnostics; TMap iteration is not an ordering contract.
    TArray<uint64> Keys; Edges.GetKeys(Keys); Keys.Sort();
    for (uint64 Key : Keys)
        if (Edges[Key].Num() > 2)
            for (int32 Face : Edges[Key]) Report(false, Face, INDEX_NONE, TEXT("Nonmanifold edge has more than two incident faces."));
    return !Issues.ContainsByPredicate([](const FKyaGeometryIssue& Issue) { return Issue.bError; });
}

bool KyaGeometry::OpenCollision(UKyaLevelBank& Bank, int32 EntryIndex,
    FKyaGeometryDocument& OutDocument, FString& OutError)
{
    OutError.Reset();
    FKyaGeometryDocument Document;
    Document.Key.Bank = &Bank; Document.Key.EntryIndex = EntryIndex; Document.Key.ResourceIndex = 0;
    Document.Key.Generation = Bank.EditGeneration; Document.Key.Kind = EKyaGeometryKind::Collision;
    TConstArrayView<uint8> Bytes;
    if (!EntryBytes(Document.Key, Bytes))
    {
        OutError = TEXT("Expected an in-range static collision entry (7/1)."); return false;
    }
    // Decode just this entry, so an error in another resource cannot mask it.
    UKyaLevelBank* Snapshot = NewObject<UKyaLevelBank>();
    Snapshot->BankBytes.Append(Bytes.GetData(), Bytes.Num());
    FKyaBankEntry Entry = Bank.Entries[EntryIndex]; Entry.Offset = 0;
    Snapshot->Entries.Add(Entry);
    TArray<FKyaPrimitiveRecord> Records; TArray<FString> Diagnostics;
    KyaPrimitives::Decode(*Snapshot, true, false, Records, Diagnostics);
    if (!Diagnostics.IsEmpty()) { OutError = FString::Join(Diagnostics, TEXT("\n")); return false; }
    Document.SourceEntryBytes = Snapshot->BankBytes;
    Document.NativeFormatDiagnostic.Reset();
    const auto Word = [&Bytes](int32 Offset) { const uint8* P = Bytes.GetData() + Offset; return uint32(P[0]) | (uint32(P[1]) << 8) | (uint32(P[2]) << 16) | (uint32(P[3]) << 24); };
    if (Bytes.Num() >= 104)
    {
        const uint32 Count = Word(64), Tree = Word(100);
        for (uint32 I = 0; I < Count; ++I)
        {
            const uint8 Type = Bytes[Tree + I * 0x70 + 0x51];
            if (Type != 1 && Type != 4 && Type != 8 && Type != 10 && Type != 11)
            { Document.NativeFormatDiagnostic = FString::Printf(TEXT("Unsupported native collision leaf type %u."), Type); break; }
        }
    }
    TMap<int32, int32> Vertices;
    for (FKyaPrimitiveRecord& Record : Records)
    {
        Record.EntryIndex = EntryIndex;
        if (Record.Shape != EKyaPrimitiveShape::Triangle && Record.Shape != EKyaPrimitiveShape::Quad)
        {
            Document.RetainedPrimitives.Add(MoveTemp(Record)); continue;
        }
        FKyaGeometryFace& Face = Document.Faces.AddDefaulted_GetRef();
        Face.SourceFaceIndex = Document.Faces.Num() - 1; Face.SourceOffset = Record.SourceOffset;
        Face.SurfaceFlags = Record.Flags;
        for (int32 I = 0; I < Record.Vertices.Num(); ++I)
        {
            const int32 Offset = Record.VertexOffsets[I];
            int32* Existing = Vertices.Find(Offset);
            const int32 Index = Existing ? *Existing : Document.Vertices.Add(Record.Vertices[I]);
            if (!Existing) Vertices.Add(Offset, Index);
            FKyaGeometryCorner& Corner = Face.Corners.AddDefaulted_GetRef();
            Corner.VertexIndex = Corner.SourceVertexIndex = Index; Corner.SourceVertexOffset = Offset;
        }
    }
    OutDocument = MoveTemp(Document); return true;
}

bool KyaGeometry::FromDecodedStrip(const FKyaGeometryResourceKey& Key, const FKyaG3DDecodedStrip& Strip,
    FKyaGeometryDocument& OutDocument, FString& OutError)
{
    OutError.Reset();
    TConstArrayView<uint8> Bytes;
    if (Key.Kind != EKyaGeometryKind::Render || Key.LodIndex != Strip.LodIndex || !EntryBytes(Key, Bytes) ||
        Strip.Indices.Num() % 3 || Strip.Attributes.Num() != Strip.Vertices.Num() ||
        (!Strip.Normals.IsEmpty() && Strip.Normals.Num() != Strip.Vertices.Num()))
    {
        OutError = TEXT("Stale render resource or unsupported decoded strip attributes."); return false;
    }
    FKyaGeometryDocument Document; Document.Key = Key;
    Document.SourceEntryBytes.Append(Bytes.GetData(), Bytes.Num());
    Document.bRequiresUVs = Document.bRequiresColors = true;
    Document.bRequiresNormals = !Strip.Normals.IsEmpty();
    for (const FKyaG3DVertex& Vertex : Strip.Vertices) Document.Vertices.Add(FVector(Vertex.Position));
    for (int32 I = 0; I < Strip.Indices.Num(); I += 3)
    {
        FKyaGeometryFace& Face = Document.Faces.AddDefaulted_GetRef();
        Face.SourceFaceIndex = I / 3; Face.MaterialIndex = Strip.MaterialIndex;
        for (int32 J = 0; J < 3; ++J)
        {
            const uint32 V = Strip.Indices[I + J];
            if (V >= uint32(Strip.Vertices.Num())) { OutError = TEXT("Strip contains an invalid vertex index."); return false; }
            FKyaGeometryCorner& Corner = Face.Corners.AddDefaulted_GetRef();
            Corner.VertexIndex = Corner.SourceVertexIndex = int32(V);
            Corner.UV = Strip.Attributes[V].UV; Corner.Color = Strip.Attributes[V].Color;
            Corner.NativeVertexFlags = Strip.Vertices[V].Flags;
            if (Document.bRequiresNormals) Corner.Normal = Strip.Normals[V];
        }
    }
    OutDocument = MoveTemp(Document); return true;
}

bool KyaGeometry::AppendFacesToCollision(const FKyaGeometryDocument& Source, TConstArrayView<int32> SelectedFaces,
    const FMatrix& SourceToTarget, uint32 SurfaceFlags, FKyaGeometryDocument& Target, FString& OutError)
{
    OutError.Reset();
    if (&Source == &Target || Source.Key.Kind != EKyaGeometryKind::Render || Target.Key.Kind != EKyaGeometryKind::Collision ||
        !MatchesSource(Source) || !MatchesSource(Target))
    {
        OutError = TEXT("Copy requires current render and collision resources with unchanged source bytes."); return false;
    }
    for (int32 Row = 0; Row < 4; ++Row)
        for (int32 Col = 0; Col < 4; ++Col)
            if (!FMath::IsFinite(SourceToTarget.M[Row][Col])) { OutError = TEXT("Copy transform must be finite."); return false; }
    const double Determinant = SourceToTarget.Determinant();
    if (!FMath::IsFinite(Determinant) || FMath::Abs(Determinant) < 1.e-12 ||
        SourceToTarget.M[0][3] != 0 || SourceToTarget.M[1][3] != 0 || SourceToTarget.M[2][3] != 0 || SourceToTarget.M[3][3] != 1)
    {
        OutError = TEXT("Copy transform must be an invertible affine matrix."); return false;
    }
    FKyaGeometryDocument Candidate = Target;
    TMap<int32, int32> VertexMap; TSet<int32> Seen;
    for (int32 Index : SelectedFaces)
    {
        if (!Source.Faces.IsValidIndex(Index) || Source.Faces[Index].Corners.Num() != 3)
        {
            OutError = TEXT("Selection contains an invalid render triangle."); return false;
        }
        if (Seen.Contains(Index)) continue;
        Seen.Add(Index);
        FKyaGeometryFace Face; Face.SurfaceFlags = SurfaceFlags;
        for (int32 J = 0; J < 3; ++J)
        {
            const int32 V = Source.Faces[Index].Corners[(Determinant < 0 && J > 0) ? 3 - J : J].VertexIndex;
            if (!Source.Vertices.IsValidIndex(V)) { OutError = TEXT("Selection contains an invalid vertex."); return false; }
            int32* Existing = VertexMap.Find(V);
            const int32 NewIndex = Existing ? *Existing : Candidate.Vertices.Add(FVector(SourceToTarget.TransformPosition(Source.Vertices[V])));
            if (!Existing) VertexMap.Add(V, NewIndex);
            FKyaGeometryCorner Corner; Corner.VertexIndex = NewIndex; Face.Corners.Add(Corner);
        }
        Candidate.Faces.Add(MoveTemp(Face));
    }
    TArray<FKyaGeometryIssue> Issues;
    if (!Validate(Candidate, Issues))
    {
        OutError = TEXT("Copied collision geometry failed validation: ");
        OutError += Issues.FindByPredicate([](const FKyaGeometryIssue& Issue) { return Issue.bError; })->Message;
        return false;
    }
    Target = MoveTemp(Candidate); return true;
}
