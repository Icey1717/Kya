#if WITH_DEV_AUTOMATION_TESTS
#include "KyaLevelBank.h"
#include "KyaLevelBankFactory.h"
#include "KyaLevelPreview.h"
#include "KyaActorHandle.h"
#include "Engine/World.h"
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "ProceduralMeshComponent.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKyaNoBackgroundTest, "Kya.Rendering.NoCommonBackground",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FKyaNoBackgroundTest::RunTest(const FString& Parameters)
{
    const FString Filename = FPlatformMisc::GetEnvironmentVariable(TEXT("KYA_TEST_BANK"));
    if (Filename.IsEmpty()) { AddWarning(TEXT("Set KYA_TEST_BANK to LEVEL_11/LEVEL.BNK.")); return true; }
    auto Import = [](const FString& Path) -> UKyaLevelBank* {
        TArray<uint8> Bytes;
        if (!FFileHelper::LoadFileToArray(Bytes, *Path)) return nullptr;
        const uint8* Cursor = Bytes.GetData();
        UKyaLevelBankFactory* Factory = NewObject<UKyaLevelBankFactory>();
        UKyaLevelBank* Result = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
            GetTransientPackage(), NAME_None, RF_Transient, nullptr, TEXT("bnk"), Cursor,
            Bytes.GetData() + Bytes.Num(), GWarn));
        if (Result) Result->SourceFilename = Path;
        return Result;
    };
    UKyaLevelBank* Main = Import(Filename);
    UKyaLevelBank* Sector = Import(FPaths::GetPath(Filename) / TEXT("SECT1.BNK"));
    if (!TestNotNull(TEXT("Import main"), Main) || !TestNotNull(TEXT("Import sector"), Sector)) return false;
    TestEqual(TEXT("Fixture has no common background"), Main->BackgroundMeshEntry, INDEX_NONE);
    UWorld* World = UWorld::CreateWorld(EWorldType::EditorPreview, false);
    if (!TestNotNull(TEXT("Create world"), World)) return false;
    AKyaLevelPreview* Preview = World->SpawnActor<AKyaLevelPreview>();
    Preview->Bank = Main;
    Preview->bShowRebuildProgress = false;
    Preview->bSpawnActorHandles = false;
    Preview->bSpawnHierarchyHandles = false;
    Preview->bShowAllActorMeshes = true;
    Preview->bShowClusterMeshes = false;
    auto Build = [&]() {
        Preview->OnConstruction(FTransform::Identity);
        int64 Triangles = 0;
        for (int32 Index = 0; Index < Preview->Mesh->GetNumSections(); ++Index)
            if (const FProcMeshSection* Section = Preview->Mesh->GetProcMeshSection(Index))
                Triangles += Section->ProcIndexBuffer.Num() / 3;
        return Triangles;
    };
    const int64 ActorTriangles = Build();
    AddInfo(FString::Printf(TEXT("Actor triangles: %lld"), ActorTriangles));
    TestTrue(TEXT("Actors render without a common background"), ActorTriangles > 0);
    Preview->bShowAllActorMeshes = false;
    Preview->bShowActorMesh = false;
    Preview->bShowClusterMeshes = true;
    Preview->SectorBanks = {Sector};
    const int64 SceneryTriangles = Build();
    AddInfo(FString::Printf(TEXT("Scenery triangles: %lld"), SceneryTriangles));
    TestTrue(TEXT("Scenery renders without a common background"), SceneryTriangles > 0);
    Preview->bSpawnActorHandles = true;
    Build();
    int32 MeshedHandles = 0;
    for (int32 Index = 0; Index < Main->SceneEnvironment.Actors.Num(); ++Index)
        if (AKyaActorHandle* Handle = Cast<AKyaActorHandle>(Preview->GetActorHandle(Index)))
            if (Handle->GetRenderedMeshBounds().IsValid) ++MeshedHandles;
    AddInfo(FString::Printf(TEXT("Actor handles with meshes: %d"), MeshedHandles));
    TestTrue(TEXT("Authoring actor handles receive meshes without a background"), MeshedHandles > 0);
    Preview->Destroy();
    World->DestroyWorld(false);
    return true;
}
#endif
