#if WITH_DEV_AUTOMATION_TESTS
#include "KyaGeometryDocument.h"
#include "KyaGeometryEncoding.h"
#include "KyaBinaryHelpers.h"
#include "KyaG3DChunks.h"
#include "KyaLevelBank.h"
#include "KyaGeometryWorkingActor.h"
#include "KyaLevelPreview.h"
#include "KyaBankExport.h"
#include "DynamicMesh/DynamicMesh3.h"
#include "DynamicMesh/DynamicMeshAttributeSet.h"
#include "Engine/World.h"
#include "Editor.h"
#include "UObject/SavePackage.h"
#include "PackageTools.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"
#include "EditorModeManager.h"
#include "ModelingToolsEditorMode.h"
#include "Selection/GeometrySelectionManager.h"
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformMisc.h"
#include <limits>

namespace
{
FKyaGeometryDocument GeometryTriangle();
void GeometryWord(TArray<uint8>& Bytes, int32 Offset, uint32 Value)
{
    for (int32 I = 0; I < 4; ++I) Bytes[Offset + I] = uint8(Value >> (8 * I));
}

UKyaLevelBank* GeometryBank(int32 Type, const TArray<uint8>& Bytes)
{
    UKyaLevelBank* Bank = NewObject<UKyaLevelBank>(); Bank->BankBytes = Bytes;
    FKyaBankEntry Entry; Entry.Type = Type; Entry.Subtype = 1; Entry.Size = Bytes.Num();
    Bank->Entries.Add(Entry); return Bank;
}

TArray<uint8> GeometryRenderFixture()
{
    TArray<uint8> B; B.SetNumZeroed(592);
    const auto Chunk = [&B](int32 P, uint32 Tag, uint32 Size)
    { GeometryWord(B, P, Tag); GeometryWord(B, P + 4, 1); GeometryWord(B, P + 8, Size); GeometryWord(B, P + 12, Size); };
    GeometryWord(B, 0, 9); GeometryWord(B, 4, 1); GeometryWord(B, 8, B.Num()); FMemory::Memcpy(B.GetData() + 12, ".G3D", 4);
    Chunk(16, 0x41545343, 144); Chunk(32, 0x41514443, 128); Chunk(80, 0x55514443, 80);
    for (int32 I = 0; I < 3; ++I) KyaWriteF32(B, 48 + I * 4, 4);
    KyaWriteF32(B, 76, 1); GeometryWord(B, 104, 1); GeometryWord(B, 120, 1); GeometryWord(B, 124, 1);
    GeometryWord(B, 144, 176); GeometryWord(B, 152, 224);
    Chunk(160, 0x414e424d, 48); Chunk(176, 0x4b4e424d, 32);
    Chunk(208, 0x32585350, 304);
    GeometryWord(B, 232, 416 - 224); GeometryWord(B, 256, 336); GeometryWord(B, 260, 368); GeometryWord(B, 264, 288);
    B[282] = 1; KyaWriteF32(B, 252, 2);
    const float Positions[3][4] = {{0, 0, 0, 0}, {1, 0, 0, 0}, {0, 1, 0, 0}};
    FMemory::Memcpy(B.GetData() + 288, Positions, sizeof(Positions));
    GeometryWord(B, 300, 0x8000); GeometryWord(B, 316, 0x8000);
    B[368] = 255; B[371] = 128; B[373] = 255; B[375] = 128; B[378] = 255; B[379] = 128;
    GeometryWord(B, 384, 0x8003); GeometryWord(B, 388, 4 << 15);
    const uint32 Targets[] = {400, 384, 352, 368, 288};
    const uint32 Counts[] = {1, 1, 1, 1, 3};
    const uint32 Commands[] = {0, 0x6c018000, 0x75038001, 0x7e03c002, 0x7c038003};
    for (int32 I = 0; I < 5; ++I)
    { GeometryWord(B, 416 + I * 16, 0x30000000 | Counts[I]); GeometryWord(B, 420 + I * 16, Targets[I]); GeometryWord(B, 428 + I * 16, Commands[I]); }
    GeometryWord(B, 496, 0x60000000);
    Chunk(512, 0x41414552, 80); Chunk(528, 0x4c414552, 64);
    const uint32 Fields[] = {144, 152, 256, 260, 264, 420, 436, 452, 468, 484};
    for (int32 I = 0; I < 10; ++I) GeometryWord(B, 544 + I * 4, Fields[I]);
    return B;
}

UKyaLevelBank* GeometrySectorFixture()
{
    TArray<uint8> Empty; Empty.SetNumZeroed(104); FMemory::Memcpy(Empty.GetData() + 12, ".COLCOLI", 8);
    UKyaLevelBank* CollisionBank = GeometryBank(7, Empty); FKyaGeometryDocument Collision; FString Error;
    KyaGeometry::OpenCollision(*CollisionBank, 0, Collision, Error);
    const FKyaGeometryDocument Triangle = GeometryTriangle(); Collision.Vertices = Triangle.Vertices; Collision.Faces = Triangle.Faces;
    TArray<uint8> Col; KyaGeometry::EncodeCollision(Collision, Col, Error);
    TArray<uint8> Bytes; Bytes.SetNumZeroed(192); FMemory::Memcpy(Bytes.GetData() + 8, "KNAB", 4);
    GeometryWord(Bytes, 0x24, 16); GeometryWord(Bytes, 0x28, 3); GeometryWord(Bytes, 0x30, 0x48); GeometryWord(Bytes, 0x34, 0x88); GeometryWord(Bytes, 0x44, 192);
    UKyaLevelBank* Bank = NewObject<UKyaLevelBank>();
    const TArray<TArray<uint8>> Payloads = {GeometryRenderFixture(), Col, TArray<uint8>{0, 0, 0, 0}};
    const uint32 Types[] = {0x40001, 0x70001, 0x50001};
    for (int32 I = 0; I < 3; ++I)
    {
        Bytes.SetNumZeroed(Align(Bytes.Num(), 16));
        FKyaBankEntry Entry; Entry.Offset = Bytes.Num(); Entry.Size = Payloads[I].Num(); Entry.Type = Types[I] >> 16; Entry.Subtype = 1;
        GeometryWord(Bytes, 0x50 + I * 16, Entry.Offset); GeometryWord(Bytes, 0x54 + I * 16, Entry.Size); GeometryWord(Bytes, 0x90 + I * 4, Types[I]);
        Bytes.Append(Payloads[I]); Bank->Entries.Add(Entry);
    }
    GeometryWord(Bytes, 0x80, Bytes.Num()); GeometryWord(Bytes, 0x1c, Bytes.Num() - 192);
    Bytes.SetNumZeroed(Align(Bytes.Num(), 2048)); GeometryWord(Bytes, 0x18, Bytes.Num());
    Bank->BankBytes = MoveTemp(Bytes); Bank->EntryCount = 3; Bank->ImportSchemaVersion = UKyaLevelBank::CurrentImportSchemaVersion;
    Bank->SourceFilename = TEXT("SECT1.BNK"); return Bank;
}

FKyaGeometryDocument GeometryTriangle()
{
    FKyaGeometryDocument Document; Document.Key.Kind = EKyaGeometryKind::Collision;
    Document.Vertices = {FVector(0, 0, 0), FVector(1, 0, 0), FVector(0, 1, 0)};
    FKyaGeometryFace Face; Face.SurfaceFlags = uint32(0);
    for (int32 I = 0; I < 3; ++I) { FKyaGeometryCorner Corner; Corner.VertexIndex = I; Face.Corners.Add(Corner); }
    Document.Faces.Add(Face); return Document;
}

bool GeometrySegmentTriangle(const FVector& Start, const FVector& End, const FVector& A, const FVector& B, const FVector& C)
{
    const FVector Direction = End - Start, E1 = B - A, E2 = C - A;
    const FVector P = FVector::CrossProduct(Direction, E2);
    const double Determinant = FVector::DotProduct(E1, P);
    if (FMath::Abs(Determinant) < 1.e-12) return false;
    const FVector T = Start - A; const double U = FVector::DotProduct(T, P) / Determinant;
    const FVector Q = FVector::CrossProduct(T, E1);
    const double V = FVector::DotProduct(Direction, Q) / Determinant;
    const double Distance = FVector::DotProduct(E2, Q) / Determinant;
    return U >= 0 && V >= 0 && U + V <= 1 && Distance >= 0 && Distance <= 1;
}

TSet<uint32> GeometryNativeRayHits(const TArray<uint8>& Bytes, const FVector& Start, const FVector& End, bool bTree)
{
    const auto Word = [&Bytes](int32 Offset) { return KyaReadU32(Bytes.GetData() + Offset); };
    const auto Position = [&Bytes](int32 Offset)
    {
        return FVector(KyaReadF32(Bytes.GetData() + Offset), KyaReadF32(Bytes.GetData() + Offset + 4), KyaReadF32(Bytes.GetData() + Offset + 8));
    };
    TSet<uint32> Hits;
    const auto TestTriangles = [&](uint32 First, uint32 Count)
    {
        for (uint32 I = First; I < First + Count; ++I)
        {
            const int32 Record = Word(76) + I * 16;
            if (GeometrySegmentTriangle(Start, End, Position(Word(72) + Word(Record) * 16),
                Position(Word(72) + Word(Record + 4) * 16), Position(Word(72) + Word(Record + 8) * 16))) Hits.Add(I);
        }
    };
    if (!bTree) { TestTriangles(0, Word(40)); return Hits; }
    TArray<uint32> Pending = {0}; TSet<uint32> Visited;
    while (!Pending.IsEmpty())
    {
        const uint32 Index = Pending.Pop(EAllowShrinking::No);
        if (Index >= Word(64) || Visited.Contains(Index)) continue;
        Visited.Add(Index);
        const int32 Node = Word(100) + Index * 0x70;
        const FVector Center = Position(Node + 48);
        bool bRejected = false;
        for (int32 Axis = 0; Axis < 3; ++Axis)
        {
            const FVector Basis = Position(Node + Axis * 16);
            const double A = FVector::DotProduct(Basis, Start - Center), B = FVector::DotProduct(Basis, End - Center);
            const double Extent = KyaReadF32(Bytes.GetData() + Node + 64 + Axis * 4);
            bRejected |= (A > Extent && B > Extent) || (A < -Extent && B < -Extent);
        }
        if (bRejected) continue;
        const uint8 Type = Bytes[Node + 0x51], Count = Bytes[Node + 0x52];
        if (Type == 1)
            for (uint32 I = 0; I < Count; ++I) Pending.Add(Word(Node + 0x54 + I * 4));
        else if (Type == 4) TestTriangles(Word(Node + 0x54), Count);
    }
    return Hits;
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKyaGeometryValidationTest, "Kya.Geometry.Documents.Validation",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FKyaGeometryValidationTest::RunTest(const FString& Parameters)
{
    FKyaGeometryDocument Document = GeometryTriangle(); TArray<FKyaGeometryIssue> Issues;
    TestTrue(TEXT("Open collision surface is valid, including flags zero"), KyaGeometry::Validate(Document, Issues));
    TestEqual(TEXT("Open edges produce no warnings"), Issues.Num(), 0);
    const FKyaGeometryFace Duplicate = Document.Faces[0];
    Document.Faces.Add(Duplicate); Document.Faces.Add(Duplicate);
    TestTrue(TEXT("Nonmanifold edges are warnings"), KyaGeometry::Validate(Document, Issues));
    TestTrue(TEXT("Nonmanifold faces identified"), Issues.Num() > 0 && Issues[0].FaceIndex != INDEX_NONE);
    Document = GeometryTriangle(); Document.Faces[0].SurfaceFlags.Reset();
    TestFalse(TEXT("New collision face needs explicit surface choice"), KyaGeometry::Validate(Document, Issues));
    Document = GeometryTriangle(); Document.Faces[0].Corners[2].VertexIndex = 99;
    TestFalse(TEXT("Invalid reference rejected safely"), KyaGeometry::Validate(Document, Issues));
    TestEqual(TEXT("Offending vertex identified"), Issues[0].VertexIndex, 99);
    Document = GeometryTriangle(); Document.Vertices[2] = FVector(2, 0, 0);
    TestFalse(TEXT("Collinear face rejected"), KyaGeometry::Validate(Document, Issues));
    Document = GeometryTriangle(); Document.Vertices[0].X = std::numeric_limits<double>::infinity();
    TestFalse(TEXT("Nonfinite coordinate rejected"), KyaGeometry::Validate(Document, Issues));
    Document = GeometryTriangle(); Document.Key.Kind = EKyaGeometryKind::Render; Document.bRequiresUVs = true;
    TestFalse(TEXT("New render face needs material and UVs"), KyaGeometry::Validate(Document, Issues));
    Document.Faces[0].MaterialIndex = 0;
    for (FKyaGeometryCorner& Corner : Document.Faces[0].Corners) Corner.UV = FVector2f::ZeroVector;
    TestTrue(TEXT("Complete render face valid"), KyaGeometry::Validate(Document, Issues));
    Document.Faces[0].Corners[0].Normal = FVector3f::ZeroVector;
    TestFalse(TEXT("Zero normal rejected"), KyaGeometry::Validate(Document, Issues));
    Document.Faces.Reset(); Document.Vertices.Reset();
    TestTrue(TEXT("Empty geometry is valid"), KyaGeometry::Validate(Document, Issues));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKyaGeometryCollisionDocumentTest, "Kya.Geometry.Documents.Collision",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FKyaGeometryCollisionDocumentTest::RunTest(const FString& Parameters)
{
    // One triangle and one quad share vertices; one referenced box is retained.
    TArray<uint8> Bytes; Bytes.SetNumZeroed(512);
    FMemory::Memcpy(Bytes.GetData() + 12, ".COLCOLI", 8);
    GeometryWord(Bytes, 40, 1); GeometryWord(Bytes, 48, 1);
    GeometryWord(Bytes, 64, 1); GeometryWord(Bytes, 72, 104);
    GeometryWord(Bytes, 76, 168); GeometryWord(Bytes, 84, 184);
    GeometryWord(Bytes, 88, 320); GeometryWord(Bytes, 100, 208);
    const float Positions[4][4] = {{0, 0, 0, 1}, {1, 0, 0, 1}, {1, 1, 0, 1}, {0, 1, 0, 1}};
    FMemory::Memcpy(Bytes.GetData() + 104, Positions, sizeof(Positions));
    for (int32 I = 0; I < 3; ++I) GeometryWord(Bytes, 168 + 4 * I, I);
    GeometryWord(Bytes, 180, 0xf0000001);
    for (int32 I = 0; I < 4; ++I) GeometryWord(Bytes, 184 + 4 * I, I);
    GeometryWord(Bytes, 200, 0x80000000);
    Bytes[208 + 0x51] = 10; Bytes[208 + 0x52] = 1;
    for (int32 I = 0; I < 4; ++I) GeometryWord(Bytes, 320 + I * 20, 0x3f800000);
    UKyaLevelBank* Bank = GeometryBank(7, Bytes);
    FKyaGeometryDocument Document; FString Error;
    if (!TestTrue(TEXT("Open collision snapshot"), KyaGeometry::OpenCollision(*Bank, 0, Document, Error))) return false;
    TestEqual(TEXT("Shared vertices remain shared"), Document.Vertices.Num(), 4);
    TestEqual(TEXT("Native quad retained"), Document.Faces[1].Corners.Num(), 4);
    TestEqual(TEXT("Unknown surface bits retained"), Document.Faces[0].SurfaceFlags.GetValue(), uint32(0xf0000001));
    TestEqual(TEXT("Referenced box retained"), Document.RetainedPrimitives.Num(), 1);
    TestTrue(TEXT("All native bytes retained"), Document.SourceEntryBytes == Bytes && Bank->BankBytes == Bytes);
    TestTrue(TEXT("Source matches"), KyaGeometry::MatchesSource(Document));
    TArray<uint8> Encoded;
    Document.Vertices[1].Z = 2;
    TestTrue(TEXT("Rebuild native collision"), KyaGeometry::EncodeCollision(Document, Encoded, Error));
    TestTrue(TEXT("Encoder leaves bank unchanged"), Bank->BankBytes == Bytes);
    UKyaLevelBank* Rebuilt = GeometryBank(7, Encoded); FKyaGeometryDocument Decoded;
    if (TestTrue(TEXT("Reimport encoded collision"), KyaGeometry::OpenCollision(*Rebuilt, 0, Decoded, Error)))
    {
        TestEqual(TEXT("Roundtrip face count"), Decoded.Faces.Num(), 2);
        TestEqual(TEXT("Roundtrip quad count"), Decoded.Faces[1].Corners.Num(), 4);
        TestEqual(TEXT("Roundtrip unknown flags"), Decoded.Faces[0].SurfaceFlags.GetValue(), uint32(0xf0000001));
        TestTrue(TEXT("Vertex edit survives native encoding"), Decoded.Vertices.Contains(FVector(1, 0, 2)));
        TestEqual(TEXT("Retained box survives native encoding"), Decoded.RetainedPrimitives.Num(), 1);
    }
    TArray<uint8> Again;
    TestTrue(TEXT("Second encode succeeds"), KyaGeometry::EncodeCollision(Document, Again, Error));
    TestTrue(TEXT("Native tree deterministic"), Again == Encoded);
    Swap(Document.Faces[1].Corners[1], Document.Faces[1].Corners[3]);
    TestTrue(TEXT("Changed quad topology encodes"), KyaGeometry::EncodeCollision(Document, Again, Error));
    UKyaLevelBank* Triangulated = GeometryBank(7, Again); FKyaGeometryDocument TriangulatedDocument;
    if (TestTrue(TEXT("Decode changed quad"), KyaGeometry::OpenCollision(*Triangulated, 0, TriangulatedDocument, Error)))
    {
        TestEqual(TEXT("Changed quad emits two triangles"), TriangulatedDocument.Faces.Num(), 3);
        for (const FKyaGeometryFace& Face : TriangulatedDocument.Faces)
            TestEqual(TEXT("Changed quad topology is triangular"), Face.Corners.Num(), 3);
    }
    Document.RetainedPrimitives.Reset();
    TestFalse(TEXT("Cannot silently discard retained shapes"), KyaGeometry::EncodeCollision(Document, Again, Error));
    Bank->BankBytes[0] ^= 1;
    TestFalse(TEXT("Same-generation source replacement detected"), KyaGeometry::MatchesSource(Document));
    Bank->BankBytes = Bytes; Bank->BumpEditGeneration();
    TestFalse(TEXT("Reimport invalidates resource key"), Document.Key.IsCurrent());
    FKyaGeometryDocument Unchanged = GeometryTriangle();
    TestFalse(TEXT("Invalid entry rejected"), KyaGeometry::OpenCollision(*Bank, 99, Unchanged, Error));
    TestEqual(TEXT("Failed open leaves caller document unchanged"), Unchanged.Faces.Num(), 1);
    for (int32 N = 0; N < 464; ++N)
    {
        Bank->BankBytes = Bytes; Bank->BankBytes.SetNum(N); Bank->Entries[0].Size = N;
        TestFalse(TEXT("Truncated collision entry rejected"), KyaGeometry::OpenCollision(*Bank, 0, Unchanged, Error));
    }
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKyaGeometryBankReplacementTest, "Kya.Geometry.Encoding.BankReplacement",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FKyaGeometryBankReplacementTest::RunTest(const FString& Parameters)
{
    TArray<uint8> Bytes; Bytes.SetNumZeroed(256);
    FMemory::Memcpy(Bytes.GetData() + 8, "KNAB", 4);
    GeometryWord(Bytes, 0x28, 2); GeometryWord(Bytes, 0x30, 0x48); GeometryWord(Bytes, 0x34, 0x78);
    GeometryWord(Bytes, 0x44, 0xa0);
    GeometryWord(Bytes, 0x24, 16);
    GeometryWord(Bytes, 0x50, 0xa0); GeometryWord(Bytes, 0x54, 16);
    GeometryWord(Bytes, 0x60, 0xb0); GeometryWord(Bytes, 0x64, 16);
    GeometryWord(Bytes, 0x70, 0xc0); GeometryWord(Bytes, 0x80, 0x70001); GeometryWord(Bytes, 0x84, 0x50001);
    Bytes[0xa0] = 42; Bytes[0xb0] = 91;
    UKyaLevelBank* Bank = GeometryBank(7, Bytes); Bank->Entries[0].Offset = 0xa0; Bank->Entries[0].Size = 16;
    FKyaBankEntry Other; Other.Type = 5; Other.Subtype = 1; Other.Offset = 0xb0; Other.Size = 16; Bank->Entries.Add(Other);
    TArray<uint8> Result; TArray<FKyaBankEntry> Entries; FString Error;
    TMap<int32, TArray<uint8>> Replacements;
    TestTrue(TEXT("Untouched replacement operation succeeds"), KyaGeometry::ReplaceBankEntries(*Bank, Replacements, Result, Entries, Error));
    TestTrue(TEXT("Untouched bank byte-identical"), Result == Bytes);
    TArray<uint8> Large; Large.Init(13, 3000); Replacements.Add(0, Large);
    TestTrue(TEXT("Growing entry replacement"), KyaGeometry::ReplaceBankEntries(*Bank, Replacements, Result, Entries, Error));
    TestEqual(TEXT("New entry size"), Entries[0].Size, int64(3000));
    TestEqual(TEXT("New entry aligned"), Entries[0].Offset % 16, int64(0));
    TestEqual(TEXT("Bank sector aligned"), Result.Num() % 2048, 0);
    TestTrue(TEXT("Unrelated payload and location unchanged"), Entries[1].Offset == 0xb0 && FMemory::Memcmp(Result.GetData() + 0xb0, Bytes.GetData() + 0xb0, 16) == 0);
    TestTrue(TEXT("Input bank unchanged"), Bank->BankBytes == Bytes);
    Replacements.Add(99, Large);
    TestFalse(TEXT("Invalid replacement identity rejected"), KyaGeometry::ReplaceBankEntries(*Bank, Replacements, Result, Entries, Error));
    TestEqual(TEXT("Failed replacement leaves caller output intact"), Entries[0].Size, int64(3000));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKyaGeometryCollisionLimitsTest, "Kya.Geometry.Encoding.CollisionLimits",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FKyaGeometryCollisionLimitsTest::RunTest(const FString& Parameters)
{
    TArray<uint8> Bytes; Bytes.SetNumZeroed(104); FMemory::Memcpy(Bytes.GetData() + 12, ".COLCOLI", 8);
    UKyaLevelBank* Bank = GeometryBank(7, Bytes);
    FKyaGeometryDocument Document; FString Error; TArray<uint8> Encoded;
    TestTrue(TEXT("Empty native collision opens"), KyaGeometry::OpenCollision(*Bank, 0, Document, Error));
    TestTrue(TEXT("Empty collision encodes a zero-child root"), KyaGeometry::EncodeCollision(Document, Encoded, Error));
    FKyaGeometryDocument Triangle = GeometryTriangle();
    for (int32 I = 0; I < 256; ++I)
    {
        FKyaGeometryFace Face = Triangle.Faces[0];
        const FVector Offset((I % 16) * 4, (I / 16) * 4, 0);
        for (int32 J = 0; J < 3; ++J) Face.Corners[J].VertexIndex = Document.Vertices.Add(Triangle.Vertices[J] + Offset);
        Document.Faces.Add(Face);
    }
    TestTrue(TEXT("Split beyond byte-sized polygon count"), KyaGeometry::EncodeCollision(Document, Encoded, Error));
    UKyaLevelBank* Result = GeometryBank(7, Encoded); FKyaGeometryDocument Decoded;
    TestTrue(TEXT("Decode split leaves"), KyaGeometry::OpenCollision(*Result, 0, Decoded, Error));
    TestEqual(TEXT("All polygons survive split"), Decoded.Faces.Num(), 256);
    for (int32 I = 0; I < 272; ++I)
    {
        const FVector Start((I % 16) * 4 + 0.25, (I / 16) * 4 + 0.25, -10), End = Start + FVector(0, 0, 20);
        const TSet<uint32> Brute = GeometryNativeRayHits(Encoded, Start, End, false);
        const TSet<uint32> Tree = GeometryNativeRayHits(Encoded, Start, End, true);
        TestTrue(TEXT("Native tree ray hits match brute force"), Brute.Num() == Tree.Num() && Brute.Difference(Tree).IsEmpty());
    }
    // Once float32 quantization collapses a triangle, decoding must reject it.
    Document.Faces.SetNum(1); Document.Vertices = {FVector(1.e10, 0, 0), FVector(1.e10 + 1, 0, 0), FVector(1.e10, 1, 0)};
    TestFalse(TEXT("Reject native quantization degeneracy"), KyaGeometry::EncodeCollision(Document, Encoded, Error));
    Document.Vertices = Triangle.Vertices; Document.Vertices[0].X = 1.e100;
    TestFalse(TEXT("Reject float32 coordinate overflow"), KyaGeometry::EncodeCollision(Document, Encoded, Error));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKyaGeometryRenderPacketsTest, "Kya.Geometry.Encoding.RenderPackets",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FKyaGeometryRenderPacketsTest::RunTest(const FString& Parameters)
{
    UKyaLevelBank* Bank = GeometrySectorFixture(); FString Error; TArray<FKyaGeometryDocument> Documents;
    if (!TestTrue(TEXT("Open prepared root render fixture"), KyaGeometry::OpenRootRender(*Bank, 0, Documents, Error))) { AddError(Error); return false; }
    TestEqual(TEXT("One resource"), Documents.Num(), 1);
    FKyaGeometryDocument& Edited = Documents[0]; Edited.Vertices[0].Z = .25;
    Edited.Faces[0].Corners[0].UV = FVector2f(.25f, .75f);
    Edited.Faces[0].Corners[1].NativeVertexFlags |= 0x2000;
    const FKyaGeometryFace Triangle = Edited.Faces[0];
    for (int32 I = 0; I < 100; ++I) Edited.Faces.Add(Triangle);
    TArray<uint8> Encoded;
    if (!TestTrue(TEXT("Rebuild and split native packets"), KyaGeometry::EncodeRootRender(Documents, Encoded, Error))) { AddError(Error); return false; }
    UKyaLevelBank* Rebuilt = GeometryBank(4, Encoded); TArray<FKyaGeometryDocument> Decoded;
    TestTrue(TEXT("Reimport split packets"), KyaGeometry::OpenRootRender(*Rebuilt, 0, Decoded, Error));
    TestEqual(TEXT("All topology survives packet splitting"), Decoded[0].Faces.Num(), 101);
    TArray<uint32> Sections; KyaG3D::ReadStripSectionCounts(Encoded, Edited.NativeResourceOffset, Sections);
    TestTrue(TEXT("Native packet splitting exercised"), Sections.Num() > 1);
    for (uint32 Count : Sections) TestTrue(TEXT("Sections respect static DMA vertex limit"), Count <= 96);
    Edited.Faces.Reset();
    TestTrue(TEXT("Empty render resource uses a skipped packet"), KyaGeometry::EncodeRootRender(Documents, Encoded, Error));
    Rebuilt = GeometryBank(4, Encoded); Decoded.Reset();
    TestTrue(TEXT("Reopen empty render resource"), KyaGeometry::OpenRootRender(*Rebuilt, 0, Decoded, Error));
    TestEqual(TEXT("Empty render geometry has no triangles"), Decoded[0].Faces.Num(), 0);
    Edited.Faces.Add(Triangle); Edited.Faces[0].Corners[0].UV = FVector2f(100, 0);
    TestFalse(TEXT("UV encoding overflow rejected"), KyaGeometry::EncodeRootRender(Documents, Encoded, Error));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKyaGeometrySessionTest, "Kya.Geometry.Editor.SharedSession",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FKyaGeometrySessionTest::RunTest(const FString& Parameters)
{
    using namespace UE::Geometry;
    UKyaLevelBank* Bank = GeometrySectorFixture(); const TArray<uint8> Original = Bank->BankBytes;
    FString Error; TArray<FKyaGeometryDocument> RenderDocuments; FKyaGeometryDocument CollisionDocument;
    if (!TestTrue(TEXT("Open render session data"), KyaGeometry::OpenRootRender(*Bank, 0, RenderDocuments, Error)) ||
        !TestTrue(TEXT("Open collision session data"), KyaGeometry::OpenCollision(*Bank, 1, CollisionDocument, Error))) { AddError(Error); return false; }
    UWorld* World = UWorld::CreateWorld(EWorldType::EditorPreview, false);
    if (!TestNotNull(TEXT("Working geometry world"), World)) return false;
    AKyaLevelPreview* Preview = World->SpawnActor<AKyaLevelPreview>();
    Preview->SetActorTransform(FTransform(FRotator(0, 25, 0), FVector(120, 240, 360), FVector(1.5)));
    const FGuid Session = FGuid::NewGuid();
    const auto Spawn = [&](const FKyaGeometryDocument& Document)
    {
        AKyaGeometryWorkingActor* Actor = World->SpawnActor<AKyaGeometryWorkingActor>();
        Actor->SessionId = Session; Actor->Preview = Preview; Actor->SetActorTransform(Preview->GetActorTransform());
        TestTrue(TEXT("Initialize Modeling Mode working target"), Actor->InitializeDocument(Document, Error)); return Actor;
    };
    AKyaGeometryWorkingActor* Render = Spawn(RenderDocuments[0]); AKyaGeometryWorkingActor* Collision = Spawn(CollisionDocument);
    FKyaGeometryDocument Roundtrip;
    TestTrue(TEXT("Read render target in native coordinate space"), Render->ReadDocument(Roundtrip, Error));
    TestTrue(TEXT("Preview translation/rotation/scale not baked into resource"), Roundtrip.Vertices[1].Equals(RenderDocuments[0].Vertices[1], 1.e-8));
    TestTrue(TEXT("Working objects transient"), Render->HasAnyFlags(RF_Transient) && Collision->HasAnyFlags(RF_Transient));
    Render->bAssignPropertiesToNewFaces = true;
    Render->GetDynamicMeshComponent()->GetDynamicMesh()->EditMesh([](FDynamicMesh3& Mesh)
    {
        const FIndex3i OriginalTriangle = Mesh.GetTriangle(0); FIndex3i NewTriangle, UVs, Normals, Colors;
        for (int32 I = 0; I < 3; ++I)
        {
            NewTriangle[I] = Mesh.AppendVertex(Mesh.GetVertex(OriginalTriangle[I]) + FVector3d(0, 10, 0));
            UVs[I] = Mesh.Attributes()->PrimaryUV()->AppendElement(Mesh.Attributes()->PrimaryUV()->GetElement(Mesh.Attributes()->PrimaryUV()->GetTriangle(0)[I]));
            Normals[I] = Mesh.Attributes()->PrimaryNormals()->AppendElement(Mesh.Attributes()->PrimaryNormals()->GetElement(Mesh.Attributes()->PrimaryNormals()->GetTriangle(0)[I]));
            Colors[I] = Mesh.Attributes()->PrimaryColors()->AppendElement(Mesh.Attributes()->PrimaryColors()->GetElement(Mesh.Attributes()->PrimaryColors()->GetTriangle(0)[I]));
        }
        const int32 T = Mesh.AppendTriangle(NewTriangle, 0);
        Mesh.Attributes()->PrimaryUV()->SetTriangle(T, UVs); Mesh.Attributes()->PrimaryNormals()->SetTriangle(T, Normals); Mesh.Attributes()->PrimaryColors()->SetTriangle(T, Colors);
    });
    Collision->GetDynamicMeshComponent()->GetDynamicMesh()->EditMesh([](FDynamicMesh3& Mesh) { Mesh.SetVertex(1, Mesh.GetVertex(1) + FVector3d(0, 0, 20)); });
    TestTrue(TEXT("Render topology editing changes only working mesh"), Bank->BankBytes == Original);
    Render->ApplyToKyaBank();
    if (!TestTrue(TEXT("One Apply commits render and collision together"), Bank->BankBytes != Original && Bank->AppliedGeometryResources.Num() == 2))
    { AddError(Render->Status); World->DestroyWorld(false); return false; }
    const TArray<uint8> Applied = Bank->BankBytes;
    RenderDocuments.Reset();
    TestTrue(TEXT("Reopen applied render"), KyaGeometry::OpenRootRender(*Bank, 0, RenderDocuments, Error));
    TestEqual(TEXT("New render face committed"), RenderDocuments[0].Faces.Num(), 2);
    TestTrue(TEXT("Reopen applied collision"), KyaGeometry::OpenCollision(*Bank, 1, Roundtrip, Error));
    TestTrue(TEXT("Collision move committed in correct axes"), Roundtrip.Vertices.ContainsByPredicate([](const FVector& V) { return V.Equals(FVector(1, .2, 0), 1.e-6); }));
    TestFalse(TEXT("Old session resource invalidated"), CollisionDocument.Key.IsCurrent());
    TestTrue(TEXT("Apply destroys temporary targets"), Render->IsActorBeingDestroyed() && Collision->IsActorBeingDestroyed());
    if (GEditor)
    {
        TestTrue(TEXT("One undo step restores entire bank"), GEditor->UndoTransaction());
        TestTrue(TEXT("Undo restores raw bytes and validation metadata"), Bank->BankBytes == Original && Bank->AppliedGeometryResources.IsEmpty());
        TestTrue(TEXT("Redo restores geometry"), GEditor->RedoTransaction());
        TestTrue(TEXT("Redo restores raw bytes and validation metadata"), Bank->BankBytes == Applied && Bank->AppliedGeometryResources.Num() == 2);
    }
    TArray<uint8> Export; FKyaBankExportDiagnostics Diagnostics;
    TestTrue(TEXT("Complete output bank validates and exports"), KyaBuildBankExportBytes(Bank, Export, Diagnostics));
    for (const FString& Message : Diagnostics.Errors) AddError(Message);
    TestTrue(TEXT("Export retains applied geometry exactly"), Export == Applied);
    const FString NativeFixture = FPlatformMisc::GetEnvironmentVariable(TEXT("KYA_GEOMETRY_EXPORT_BANK"));
    if (!NativeFixture.IsEmpty())
    {
        IFileManager::Get().MakeDirectory(*FPaths::GetPath(NativeFixture), true);
        TestTrue(TEXT("Write native port validation fixture"), FFileHelper::SaveArrayToFile(Export, *NativeFixture));
        AddInfo(TEXT("Native port validation fixture: ") + NativeFixture + TEXT("; render strip offset 224, expected triangle count 2."));
    }
    TArray<uint8> Corrupt = Applied; Corrupt[int32(Bank->Entries[0].Offset) + 12] = 0;
    TestFalse(TEXT("Export geometry validation rejects corrupt staged output"), KyaGeometry::ValidateAppliedGeometry(*Bank, Corrupt, Error));
    World->DestroyWorld(false);
    const FString PackageName = TEXT("/Game/KyaGeometryTests_") + FGuid::NewGuid().ToString(EGuidFormats::Digits);
    UPackage* Package = CreatePackage(*PackageName);
    UKyaLevelBank* Saved = DuplicateObject<UKyaLevelBank>(Bank, Package, TEXT("Bank")); Saved->SetFlags(RF_Public | RF_Standalone);
    const FString Filename = FPackageName::LongPackageNameToFilename(PackageName, FPackageName::GetAssetPackageExtension());
    FSavePackageArgs Args; Args.TopLevelFlags = RF_Public | RF_Standalone; Args.SaveFlags = SAVE_NoError;
    const bool bSaved = UPackage::SavePackage(Package, Saved, *Filename, Args);
    TestTrue(TEXT("Applied bank asset saves"), bSaved);
    FText ReloadError;
    const bool bReloaded = bSaved && UPackageTools::ReloadPackages({Package}, ReloadError, EReloadPackagesInteractionMode::AssumePositive);
    TestTrue(TEXT("Applied bank asset reopens"), bReloaded);
    UPackage* LoadedPackage = FindPackage(nullptr, *PackageName);
    UKyaLevelBank* Loaded = LoadedPackage ? FindObject<UKyaLevelBank>(LoadedPackage, TEXT("Bank")) : nullptr;
    if (TestNotNull(TEXT("Reopened bank asset"), Loaded))
    {
        TestTrue(TEXT("Geometry bytes and provenance persist"), Loaded->BankBytes == Applied && Loaded->AppliedGeometryResources.Num() == 2);
        TestTrue(TEXT("Reopened applied geometry validates"), KyaGeometry::ValidateAppliedGeometry(*Loaded, Loaded->BankBytes, Error));
    }
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKyaGeometryCopySelectionTest, "Kya.Geometry.Editor.CopySelectedFaces",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FKyaGeometryCopySelectionTest::RunTest(const FString& Parameters)
{
    if (!GEditor || !GEditor->GetEditorWorldContext().World()) { AddError(TEXT("An editor world is required.")); return false; }
    UWorld* World = GEditor->GetEditorWorldContext().World();
    UKyaLevelBank* Bank = GeometrySectorFixture(); const TArray<uint8> Before = Bank->BankBytes;
    TArray<FKyaGeometryDocument> RenderData; FKyaGeometryDocument CollisionData; FString Error;
    if (!KyaGeometry::OpenRootRender(*Bank, 0, RenderData, Error) || !KyaGeometry::OpenCollision(*Bank, 1, CollisionData, Error))
    { AddError(Error); return false; }
    AKyaLevelPreview* Preview = World->SpawnActor<AKyaLevelPreview>();
    AKyaGeometryWorkingActor* Render = World->SpawnActor<AKyaGeometryWorkingActor>();
    AKyaGeometryWorkingActor* Collision = World->SpawnActor<AKyaGeometryWorkingActor>();
    Render->Preview = Collision->Preview = Preview; Render->SessionId = Collision->SessionId = FGuid::NewGuid();
    Render->InitializeDocument(RenderData[0], Error); Collision->InitializeDocument(CollisionData, Error);
    const FEditorModeID ModeId = UModelingToolsEditorMode::EM_ModelingToolsEditorModeId;
    const bool WasActive = GLevelEditorModeTools().IsModeActive(ModeId);
    GEditor->SelectNone(false, true); GEditor->SelectActor(Render, true, true);
    GLevelEditorModeTools().ActivateMode(ModeId);
    UModelingToolsEditorMode* Mode = Cast<UModelingToolsEditorMode>(GLevelEditorModeTools().GetActiveScriptableMode(ModeId));
    bool bSelected = false;
    if (TestNotNull(TEXT("Modeling Mode active"), Mode) && TestNotNull(TEXT("Modeling Mode selection manager"), Mode->GetSelectionManager()))
    {
        UE::Geometry::FGeometrySelection Selection;
        Selection.Selection.Add(UE::Geometry::FGeoSelectionID::MeshTriangle(0).Encoded());
        Mode->GetSelectionManager()->SetMeshSelectionTypeAndMode(UE::Geometry::EGeometryElementType::Face, UGeometrySelectionManager::EMeshTopologyMode::Triangle, false);
        bSelected = Mode->GetSelectionManager()->SetSelectionForComponent(Render->GetDynamicMeshComponent(), Selection);
        TestTrue(TEXT("Select render face through Modeling Mode"), bSelected);
    }
    if (bSelected)
    {
        Render->bAssignPropertiesToNewFaces = true; Render->SurfaceFlags = 0x80000002;
        Render->CopySelectedFacesToCollision();
        FKyaGeometryDocument Copied;
        TestTrue(TEXT("Read copy preview"), Collision->ReadDocument(Copied, Error));
        TestEqual(TEXT("Selected triangle appended"), Copied.Faces.Num(), 2);
        if (Copied.Faces.Num() == 2) TestEqual(TEXT("Explicit surface flags preserved"), Copied.Faces[1].SurfaceFlags.GetValue(), uint32(0x80000002));
        Render->CopySelectedFacesToCollision();
        TestTrue(TEXT("Read repeated copy"), Collision->ReadDocument(Copied, Error));
        TestEqual(TEXT("Repeated copy appends explicitly"), Copied.Faces.Num(), 3);
        TestTrue(TEXT("Copy has an undo step"), GEditor->UndoTransaction());
        TestTrue(TEXT("Read undone copy"), Collision->ReadDocument(Copied, Error));
        TestEqual(TEXT("Undo removes one appended selection"), Copied.Faces.Num(), 2);
        TestTrue(TEXT("Redo copy"), GEditor->RedoTransaction());
        TestTrue(TEXT("Read redone copy"), Collision->ReadDocument(Copied, Error));
        TestEqual(TEXT("Redo restores appended selection"), Copied.Faces.Num(), 3);
        TestTrue(TEXT("Copy preview never mutates bank bytes"), Bank->BankBytes == Before);
    }
    GEditor->SelectNone(false, true);
    if (!WasActive) GLevelEditorModeTools().DeactivateMode(ModeId);
    Render->Destroy(); Collision->Destroy(); Preview->Destroy();
    return bSelected;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKyaGeometryRealBanksTest, "Kya.Geometry.Encoding.RealBanks",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FKyaGeometryRealBanksTest::RunTest(const FString& Parameters)
{
    const FString Directory = FPlatformMisc::GetEnvironmentVariable(TEXT("KYA_TEST_GEOMETRY_BANK_DIR"));
    if (Directory.IsEmpty()) { AddInfo(TEXT("Set KYA_TEST_GEOMETRY_BANK_DIR to exercise real sector banks.")); return true; }
    TArray<FString> Files; IFileManager::Get().FindFilesRecursive(Files, *Directory, TEXT("*.BNK"), true, false);
    Files.Sort(); int32 EncodedCount = 0, RejectedCount = 0, RenderEncodedCount = 0, RenderRejectedCount = 0;
    for (const FString& File : Files)
    {
        TArray<uint8> Bytes;
        if (!FFileHelper::LoadFileToArray(Bytes, *File) || Bytes.Num() < 0x48 ||
            FMemory::Memcmp(Bytes.GetData() + 8, "KNAB", 4) || (KyaReadU32(Bytes.GetData() + 12) & 1)) continue;
        const uint32 Count = KyaReadU32(Bytes.GetData() + 0x28), Common = KyaReadU32(Bytes.GetData() + 0x14);
        const int64 Table = int64(KyaReadU32(Bytes.GetData() + 0x30)) + 8, Types = int64(KyaReadU32(Bytes.GetData() + 0x34)) + 8;
        if (!KyaRangeValid(Bytes, Table, int64(Count) * 16) || (!Common && !KyaRangeValid(Bytes, Types, int64(Count) * 4))) continue;
        UKyaLevelBank* Bank = NewObject<UKyaLevelBank>(); Bank->BankBytes = Bytes;
        for (uint32 I = 0; I < Count; ++I)
        {
            FKyaBankEntry Entry;
            Entry.Offset = KyaReadU32(Bytes.GetData() + Table + I * 16);
            Entry.Size = KyaReadU32(Bytes.GetData() + Table + I * 16 + 4);
            const uint32 Pair = Common ? Common : KyaReadU32(Bytes.GetData() + Types + I * 4);
            Entry.Type = Pair >> 16; Entry.Subtype = Pair & 0xffff; Bank->Entries.Add(Entry);
        }
        for (int32 I = 0; I < Bank->Entries.Num(); ++I)
        {
            if (Bank->Entries[I].Type == 4 && Bank->Entries[I].Subtype == 1)
            {
                TArray<FKyaGeometryDocument> Documents; FString RenderError;
                if (!KyaGeometry::OpenRootRender(*Bank, I, Documents, RenderError) || Documents.IsEmpty()) { ++RenderRejectedCount; continue; }
                FKyaGeometryDocument& Edited = Documents[0];
                if (Edited.Faces.IsEmpty()) { ++RenderRejectedCount; continue; }
                FKyaGeometryFace Added = Edited.Faces[0]; Added.SourceFaceIndex = Added.SourceOffset = INDEX_NONE;
                // Add a displaced copy with the same native material/attributes.
                for (FKyaGeometryCorner& Corner : Added.Corners)
                    Corner.VertexIndex = Edited.Vertices.Add(Edited.Vertices[Corner.VertexIndex] + FVector(0, 0.125, 0));
                Edited.Faces.Add(MoveTemp(Added));
                TArray<uint8> Encoded;
                if (!KyaGeometry::EncodeRootRender(MakeArrayView(&Edited, 1), Encoded, RenderError))
                {
                    ++RenderRejectedCount; AddInfo(File + TEXT(": render encoder rejected: ") + RenderError); continue;
                }
                UKyaLevelBank* Rebuilt = GeometryBank(4, Encoded); TArray<FKyaGeometryDocument> Reopened;
                if (TestTrue(*(File + TEXT(": reopen edited render topology")), KyaGeometry::OpenRootRender(*Rebuilt, 0, Reopened, RenderError)))
                    TestEqual(TEXT("Real render topology addition survives encoding"), Reopened[0].Faces.Num(), Edited.Faces.Num());
                TestTrue(TEXT("Real mesh source bank unchanged"), Bank->BankBytes == Bytes); ++RenderEncodedCount;
                continue;
            }
            if (Bank->Entries[I].Type != 7 || Bank->Entries[I].Subtype != 1) continue;
            FKyaGeometryDocument Document; FString Error;
            if (!KyaGeometry::OpenCollision(*Bank, I, Document, Error)) { AddError(File + TEXT(": ") + Error); continue; }
            // Exercise a topology edit on disposable working data. Output banks
            // stay in memory; the installed game files are never changed.
            if (!Document.Faces.IsEmpty())
            {
                FKyaGeometryFace Added = Document.Faces[0];
                Added.Corners.SetNum(3); Swap(Added.Corners[1], Added.Corners[2]);
                Added.SourceFaceIndex = Added.SourceOffset = INDEX_NONE;
                Document.Faces.Add(MoveTemp(Added));
            }
            TArray<uint8> Encoded;
            if (!KyaGeometry::EncodeCollision(Document, Encoded, Error))
            {
                ++RejectedCount; AddInfo(File + TEXT(": encoder rejected: ") + Error); continue;
            }
            TMap<int32, TArray<uint8>> Replacements; Replacements.Add(I, Encoded);
            TArray<uint8> Result; TArray<FKyaBankEntry> Entries;
            if (!TestTrue(*(File + TEXT(": complete bank replacement")), KyaGeometry::ReplaceBankEntries(*Bank, Replacements, Result, Entries, Error)))
            { AddError(Error); continue; }
            UKyaLevelBank* Reimported = NewObject<UKyaLevelBank>(); Reimported->BankBytes = MoveTemp(Result); Reimported->Entries = MoveTemp(Entries);
            FKyaGeometryDocument Reopened;
            if (!TestTrue(*(File + TEXT(": reopen replacement bank")), KyaGeometry::OpenCollision(*Reimported, I, Reopened, Error)))
            { AddError(Error); continue; }
            TestEqual(TEXT("Real collision face count survives bank replacement"), Reopened.Faces.Num(), Document.Faces.Num());
            TestTrue(TEXT("Real original bytes unchanged"), Bank->BankBytes == Bytes);
            ++EncodedCount;
        }
    }
    AddInfo(FString::Printf(TEXT("Real collision entries: %d encoded/reopened, %d rejected with diagnostics."), EncodedCount, RejectedCount));
    AddInfo(FString::Printf(TEXT("Real render entries: %d edited/encoded/reopened, %d unsupported or rejected."), RenderEncodedCount, RenderRejectedCount));
    TestTrue(TEXT("At least one real sector collision encoded"), EncodedCount > 0);
    TestTrue(TEXT("At least one real render topology edit encoded"), RenderEncodedCount > 0);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKyaGeometryCopyTest, "Kya.Geometry.Documents.RenderAndCopy",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FKyaGeometryCopyTest::RunTest(const FString& Parameters)
{
    UKyaLevelBank* RenderBank = GeometryBank(4, {1, 2, 3});
    UKyaLevelBank* CollisionBank = GeometryBank(7, {4, 5, 6});
    FKyaGeometryResourceKey Key; Key.Bank = RenderBank; Key.EntryIndex = Key.ResourceIndex = Key.LodIndex = 0;
    FKyaG3DDecodedStrip Strip; Strip.MaterialIndex = 2;
    const FVector3f Positions[] = {FVector3f(0, 0, 0), FVector3f(1, 0, 0), FVector3f(0, 1, 0), FVector3f(0, 0, 0)};
    for (int32 I = 0; I < 4; ++I)
    {
        FKyaG3DVertex Vertex; Vertex.Position = Positions[I]; Vertex.Flags = 0x8000 + I; Strip.Vertices.Add(Vertex);
        FKyaG3DVertexAttributes Attributes; Attributes.UV = FVector2f(I, I); Attributes.Color = FColor::Red;
        Strip.Attributes.Add(Attributes); Strip.Normals.Add(FVector3f(0, 0, 1));
    }
    Strip.Indices = {0, 1, 2, 3, 1, 2};
    FKyaGeometryDocument Source; FString Error;
    if (!TestTrue(TEXT("Render adapter"), KyaGeometry::FromDecodedStrip(Key, Strip, Source, Error))) return false;
    TestEqual(TEXT("Coincident vertices not welded"), Source.Vertices.Num(), 4);
    TestTrue(TEXT("Corner UV seam retained"), Source.Faces[0].Corners[0].UV.GetValue() != Source.Faces[1].Corners[0].UV.GetValue());
    TestEqual(TEXT("Native vertex flags retained"), Source.Faces[1].Corners[0].NativeVertexFlags, uint32(0x8003));
    TestEqual(TEXT("Material retained"), Source.Faces[0].MaterialIndex.GetValue(), 2);
    TestTrue(TEXT("Color retained"), Source.Faces[0].Corners[0].Color.GetValue() == FColor::Red);
    FKyaGeometryDocument Target; Target.Key.Bank = CollisionBank; Target.Key.EntryIndex = Target.Key.ResourceIndex = 0;
    Target.Key.Kind = EKyaGeometryKind::Collision; Target.SourceEntryBytes = CollisionBank->BankBytes;
    FMatrix Transform = FMatrix::Identity; Transform.M[0][0] = -1; Transform.M[3][0] = 10;
    const TArray<int32> Selection = {0, 0, 1};
    TestTrue(TEXT("Copy selection with reflection and translation"), KyaGeometry::AppendFacesToCollision(Source, Selection, Transform, 0x80000000, Target, Error));
    TestEqual(TEXT("Selection deduplicated"), Target.Faces.Num(), 2);
    TestEqual(TEXT("Copy preserves shared vertices without welding seam"), Target.Vertices.Num(), 4);
    TestTrue(TEXT("Target local coordinates"), Target.Vertices[0].Equals(FVector(10, 0, 0)));
    const FKyaGeometryFace& Face = Target.Faces[0];
    const FVector A = Target.Vertices[Face.Corners[0].VertexIndex];
    TestTrue(TEXT("Reflection preserves oriented normal"), FVector::CrossProduct(Target.Vertices[Face.Corners[1].VertexIndex] - A,
        Target.Vertices[Face.Corners[2].VertexIndex] - A).Z > 0);
    TestTrue(TEXT("Repeated copy explicitly appends"), KyaGeometry::AppendFacesToCollision(Source, Selection, Transform, 0, Target, Error));
    TestEqual(TEXT("Second invocation appends faces"), Target.Faces.Num(), 4);
    const int32 BeforeVertices = Target.Vertices.Num(); const TArray<int32> InvalidSelection = {0, 99};
    TestFalse(TEXT("Invalid selection rejected atomically"), KyaGeometry::AppendFacesToCollision(Source, InvalidSelection, Transform, 0, Target, Error));
    TestEqual(TEXT("Failed copy preserves vertices"), Target.Vertices.Num(), BeforeVertices);
    TestEqual(TEXT("Failed copy preserves faces"), Target.Faces.Num(), 4);
    RenderBank->BumpEditGeneration();
    TestFalse(TEXT("Stale source cannot copy"), KyaGeometry::AppendFacesToCollision(Source, Selection, Transform, 0, Target, Error));
    return true;
}
#endif
