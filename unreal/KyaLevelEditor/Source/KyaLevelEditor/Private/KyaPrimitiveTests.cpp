#if WITH_DEV_AUTOMATION_TESTS
#include "KyaPrimitives.h"
#include "KyaLevelBank.h"
#include "KyaPrimitiveComponent.h"
#include "KyaLevelPreview.h"
#include "Engine/World.h"
#include "ProceduralMeshComponent.h"
#include "UObject/UnrealType.h"
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/PlatformMisc.h"
#include "HAL/FileManager.h"

namespace
{
void PutPrimitiveWord(TArray<uint8>& B, int32 Offset, uint32 Word)
{
    for (int32 I = 0; I < 4; ++I) B[Offset + I] = uint8(Word >> (I * 8));
}

UKyaLevelBank* MakePrimitiveBank(uint32 Type, TArray<uint8> Bytes)
{
    UKyaLevelBank* Bank = NewObject<UKyaLevelBank>();
    Bank->BankBytes = MoveTemp(Bytes);
    FKyaBankEntry E;
    E.Type = Type; E.Subtype = 1; E.Offset = 0; E.Size = Bank->BankBytes.Num();
    Bank->Entries.Add(E);
    return Bank;
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKyaPrimitiveDecodeTest, "Kya.Primitives.Decode",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FKyaPrimitiveDecodeTest::RunTest(const FString& Parameters)
{
    TArray<uint8> Bytes;
    Bytes.SetNumZeroed(256);
    FMemory::Memcpy(Bytes.GetData() + 8, "TNVE", 4);
    PutPrimitiveWord(Bytes, 16, 0x7e);
    PutPrimitiveWord(Bytes, 20, 1);
    Bytes[24] = 2; // box
    // world -> unit: translated by -4 along native X
    for (int32 I = 0; I < 4; ++I) PutPrimitiveWord(Bytes, 32 + I * 20, 0x3f800000);
    PutPrimitiveWord(Bytes, 80, 0xc0800000);
    PutPrimitiveWord(Bytes, 96, 1); // groups
    PutPrimitiveWord(Bytes, 100, 8); // list offset from table
    PutPrimitiveWord(Bytes, 104, 16); // zone table offset from table
    PutPrimitiveWord(Bytes, 108, 1); // list count
    PutPrimitiveWord(Bytes, 112, 0); // matrix index
    PutPrimitiveWord(Bytes, 116, 1); // zone count
    PutPrimitiveWord(Bytes, 120, 0);
    PutPrimitiveWord(Bytes, 124, MAX_uint32);
    PutPrimitiveWord(Bytes, 128, 0); // same matrix reused in group 2
    UKyaLevelBank* Bank = MakePrimitiveBank(8, Bytes);
    TArray<FKyaPrimitiveRecord> Records;
    TArray<FString> Diagnostics;
    KyaPrimitives::Decode(*Bank, false, true, Records, Diagnostics);
    TestEqual(TEXT("Valid event diagnostics"), Diagnostics.Num(), 0);
    TestEqual(TEXT("Shared primitive retains two zone memberships"), Records.Num(), 2);
    if (Records.Num() == 2)
    {
        TestEqual(TEXT("Shared transform identity"), Records[0].TransformOffset, Records[1].TransformOffset);
        TestEqual(TEXT("Group identity"), Records[1].GroupIndex, 2);
        TestTrue(TEXT("World-to-local inverted"), FVector(Records[0].LocalToGame.TransformPosition(FVector::ZeroVector)).Equals(FVector(4, 0, 0)));
        TArray<FVector> Edges;
        KyaPrimitives::BuildEdges(Records[0], Edges);
        TestEqual(TEXT("Box edges"), Edges.Num(), 24);
        for (EKyaPrimitiveShape Shape : {EKyaPrimitiveShape::Box, EKyaPrimitiveShape::Sphere,
            EKyaPrimitiveShape::Cylinder, EKyaPrimitiveShape::Cone, EKyaPrimitiveShape::HalfSpace, EKyaPrimitiveShape::OpenTopBox})
        {
            FKyaPrimitiveRecord Surface = Records[0];
            Surface.Shape = Shape;
            TArray<FVector> Triangles;
            KyaPrimitives::BuildTriangles(Surface, Triangles);
            TestTrue(TEXT("Solid shape has triangle list"), Triangles.Num() > 0 && Triangles.Num() % 3 == 0);
            for (int32 I = 0; I < Triangles.Num(); I += 3)
                TestTrue(TEXT("Solid triangle finite and nondegenerate"),
                    !Triangles[I].ContainsNaN() && FVector::CrossProduct(Triangles[I + 1] - Triangles[I], Triangles[I + 2] - Triangles[I]).SizeSquared() > 1.e-12);
            if (Shape == EKyaPrimitiveShape::Box) TestEqual(TEXT("Solid box has twelve triangles"), Triangles.Num(), 36);
            if (Shape == EKyaPrimitiveShape::HalfSpace) TestEqual(TEXT("Half space guide is an open plane"), Triangles.Num(), 6);
            if (Shape == EKyaPrimitiveShape::OpenTopBox) TestEqual(TEXT("Open box has no top cap"), Triangles.Num(), 30);
        }
    }
    TestTrue(TEXT("Decode preserves original bytes"), Bank->BankBytes == Bytes);
    PutPrimitiveWord(Bank->BankBytes, 112, 99);
    KyaPrimitives::Decode(*Bank, false, true, Records, Diagnostics);
    TestEqual(TEXT("Reject out of range matrix"), Records.Num(), 0);
    TestEqual(TEXT("Report invalid entry"), Diagnostics.Num(), 1);
    Bank->BankBytes = Bytes;
    PutPrimitiveWord(Bank->BankBytes, 32, 0);
    KyaPrimitives::Decode(*Bank, false, true, Records, Diagnostics);
    TestEqual(TEXT("Reject singular matrix"), Records.Num(), 0);
    // Every prefix must be safe to decode and cannot produce a complete record.
    for (int32 N = 0; N < 156; ++N)
    {
        Bank->BankBytes = Bytes; Bank->BankBytes.SetNum(N); Bank->Entries[0].Size = N;
        KyaPrimitives::Decode(*Bank, false, true, Records, Diagnostics);
        TestEqual(TEXT("Truncated event entry"), Records.Num(), 0);
    }
    Bank->BankBytes = Bytes; Bank->Entries[0].Size = Bytes.Num();
    UWorld* World = UWorld::CreateWorld(EWorldType::EditorPreview, false);
    if (!TestNotNull(TEXT("Create primitive preview world"), World)) return false;
    AKyaLevelPreview* Preview = World->SpawnActor<AKyaLevelPreview>();
    Preview->Bank = Bank;
    // Details edits run PreEditChange before PostEditChangeProperty. Exercising only
    // RefreshPrimitives misses AActor unregistering every component in the pre-step.
    UProceduralMeshComponent* LevelMesh = Preview->FindComponentByClass<UProceduralMeshComponent>();
    TestNotNull(TEXT("Level geometry component"), LevelMesh);
    const TArray<FVector> Positions = {FVector(0, 0, 0), FVector(100, 0, 0), FVector(0, 100, 0)};
    LevelMesh->CreateMeshSection(0, Positions, {0, 1, 2}, {}, {}, {}, {}, false);
    for (FName Name : {
        GET_MEMBER_NAME_CHECKED(AKyaLevelPreview, bShowCollisionPrimitives),
        GET_MEMBER_NAME_CHECKED(AKyaLevelPreview, bShowEventPrimitives),
        GET_MEMBER_NAME_CHECKED(AKyaLevelPreview, CollisionPrimitiveDisplay),
        GET_MEMBER_NAME_CHECKED(AKyaLevelPreview, EventPrimitiveDisplay),
        GET_MEMBER_NAME_CHECKED(AKyaLevelPreview, PrimitiveSolidOpacity),
        GET_MEMBER_NAME_CHECKED(AKyaLevelPreview, PrimitiveWireThickness),
        GET_MEMBER_NAME_CHECKED(AKyaLevelPreview, PrimitiveWireDepthBias),
        GET_MEMBER_NAME_CHECKED(AKyaLevelPreview, bAnimateTextures),
        GET_MEMBER_NAME_CHECKED(AKyaLevelPreview, TextureAnimationFrameRate)})
    {
        FProperty* Property = FindFProperty<FProperty>(AKyaLevelPreview::StaticClass(), Name);
        Preview->PreEditChange(Property);
        TestFalse(TEXT("Pre-edit unregisters level mesh"), LevelMesh->IsRegistered());
        FPropertyChangedEvent Change(Property, EPropertyChangeType::ValueSet);
        Preview->PostEditChangeProperty(Change);
        TestTrue(TEXT("Display edit restores level mesh registration"), LevelMesh->IsRegistered());
        const FProcMeshSection* Section = LevelMesh->GetProcMeshSection(0);
        TestTrue(TEXT("Display edit preserves level mesh geometry"), Section && Section->ProcVertexBuffer.Num() == 3 && Section->ProcIndexBuffer.Num() == 3);
    }
    Preview->bShowEventPrimitives = true;
    Preview->RefreshPrimitives();
    TestEqual(TEXT("One bank component"), Preview->PrimitiveComponents.Num(), 1);
    if (Preview->PrimitiveComponents.Num() == 1)
    {
        UKyaPrimitiveComponent* Component = Preview->PrimitiveComponents[0];
        TestEqual(TEXT("Published primitive records"), Component->PrimitiveCount, 2);
        Component->SelectRecord(1);
        TestTrue(TEXT("Selection exposes source identity"), Component->SelectedSource.Contains(TEXT("group 2")));
        TestTrue(TEXT("World bounds include converted translation"), Component->Bounds.GetBox().IsInside(FVector(400, 0, 0)));
    }
    Preview->bShowEventPrimitives = false;
    Preview->RefreshPrimitives();
    TestEqual(TEXT("Disabling clears components"), Preview->PrimitiveComponents.Num(), 0);
    Preview->Bank = nullptr;
    Preview->bShowEventPrimitives = true;
    Bank->SourceFilename = TEXT("SECT3.BNK");
    Preview->SectorBanks.Add(Bank);
    Preview->RefreshPrimitives();
    TestEqual(TEXT("Sector primitives visible"), Preview->PrimitiveComponents.Num(), 1);
    Preview->SetSectorVisible(3, false);
    TestEqual(TEXT("Sector hide removes primitives"), Preview->PrimitiveComponents.Num(), 0);
    Preview->SetSectorVisible(3, true);
    TestEqual(TEXT("Sector show restores primitives"), Preview->PrimitiveComponents.Num(), 1);
    World->DestroyWorld(false);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKyaPrimitiveRealBankTest, "Kya.Primitives.RealBanks",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FKyaPrimitiveRealBankTest::RunTest(const FString& Parameters)
{
    const FString Main = FPlatformMisc::GetEnvironmentVariable(TEXT("KYA_TEST_BANK"));
    if (Main.IsEmpty()) { AddWarning(TEXT("Set KYA_TEST_BANK to validate real event/collision entries.")); return true; }
    TArray<FString> Files;
    IFileManager::Get().FindFiles(Files, *(FPaths::GetPath(Main) / TEXT("*.BNK")), true, false);
    int32 EventPrimitives = 0, CollisionPrimitives = 0;
    for (const FString& File : Files)
    {
        UKyaLevelBank* Bank = NewObject<UKyaLevelBank>();
        if (!FFileHelper::LoadFileToArray(Bank->BankBytes, *(FPaths::GetPath(Main) / File))) continue;
        const TArray<uint8>& B = Bank->BankBytes;
        const auto Word = [&B](int64 O) -> uint32
        {
            return uint32(B[O]) | uint32(B[O + 1]) << 8 | uint32(B[O + 2]) << 16 | uint32(B[O + 3]) << 24;
        };
        if (B.Num() < 72 || Word(12) & 1) continue;
        const int64 Table = int64(Word(48)) + 8, Types = int64(Word(52)) + 8;
        const uint32 Count = Word(40), Common = Word(20);
        if (Table + int64(Count) * 16 > B.Num() || (!Common && Types + int64(Count) * 4 > B.Num())) continue;
        for (uint32 I = 0; I < Count; ++I)
        {
            const uint32 Pair = Common ? Common : Word(Types + I * 4);
            FKyaBankEntry E;
            E.Type = Pair >> 16; E.Subtype = Pair & 0xffff;
            E.Offset = Word(Table + I * 16); E.Size = Word(Table + I * 16 + 4);
            Bank->Entries.Add(E);
        }
        const TArray<uint8> Original = B;
        TArray<FKyaPrimitiveRecord> Records;
        TArray<FString> Diagnostics;
        KyaPrimitives::Decode(*Bank, true, true, Records, Diagnostics);
        for (const FString& D : Diagnostics) AddError(File + TEXT(": ") + D);
        for (const FKyaPrimitiveRecord& R : Records)
        {
            if (R.ZoneIndex == INDEX_NONE) ++CollisionPrimitives; else ++EventPrimitives;
            TestTrue(TEXT("Source identity resolves inside entry"), R.SourceOffset >= 0 && R.SourceOffset < Bank->Entries[R.EntryIndex].Size);
            TArray<FVector> Edges; KyaPrimitives::BuildEdges(R, Edges);
            TestTrue(TEXT("Renderable wire shape"), Edges.Num() >= 2 && Edges.Num() % 2 == 0);
        }
        TestTrue(TEXT("Bank bytes preserved"), Original == B);
    }
    TestTrue(TEXT("Real event primitives decoded"), EventPrimitives > 0);
    TestTrue(TEXT("Real collision primitives decoded"), CollisionPrimitives > 0);
    AddInfo(FString::Printf(TEXT("Decoded %d event memberships and %d collision primitives"), EventPrimitives, CollisionPrimitives));
    return true;
}
#endif
