#if WITH_DEV_AUTOMATION_TESTS
#include "KyaLevelBank.h"
#include "KyaLevelBankFactory.h"
#include "KyaLevelPreview.h"
#include "Engine/World.h"
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "ProceduralMeshComponent.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKyaBackgroundSelectionTest, "Kya.Rendering.SectorBackgroundSelection",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKyaBackgroundSelectionTest::RunTest(const FString& Parameters)
{
    const FString Filename = FPlatformMisc::GetEnvironmentVariable(TEXT("KYA_TEST_BANK"));
    if (Filename.IsEmpty())
    {
        AddError(TEXT("Set KYA_TEST_BANK to LEVEL_1/LEVEL.BNK for the background overlap regression."));
        return false;
    }
    auto Import = [this](const FString& Path) -> UKyaLevelBank* {
        TArray<uint8> Bytes;
        if (!TestTrue(TEXT("Read background bank"), FFileHelper::LoadFileToArray(Bytes, *Path))) return nullptr;
        const uint8* Cursor = Bytes.GetData();
        UKyaLevelBankFactory* Factory = NewObject<UKyaLevelBankFactory>();
        UKyaLevelBank* Result = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
            GetTransientPackage(), NAME_None, RF_Transient, nullptr, TEXT("bnk"), Cursor,
            Bytes.GetData() + Bytes.Num(), GWarn));
        if (Result) Result->SourceFilename = Path;
        return Result;
    };
    UKyaLevelBank* Main = Import(Filename);
    UKyaLevelBank* One = Import(FPaths::GetPath(Filename) / TEXT("SECT1.BNK"));
    UKyaLevelBank* Two = Import(FPaths::GetPath(Filename) / TEXT("SECT2.BNK"));
    UKyaLevelBank* Seven = Import(FPaths::GetPath(Filename) / TEXT("SECT7.BNK"));
    UKyaLevelBank* Twelve = Import(FPaths::GetPath(Filename) / TEXT("SECT12.BNK"));
    if (!Main || !One || !Two || !Seven || !Twelve) return false;
    UWorld* World = UWorld::CreateWorld(EWorldType::EditorPreview, false);
    if (!TestNotNull(TEXT("Create background test world"), World)) return false;
    AKyaLevelPreview* Preview = World->SpawnActor<AKyaLevelPreview>();
    if (!TestNotNull(TEXT("Create background preview"), Preview)) { World->DestroyWorld(false); return false; }
    Preview->Bank = Main;
    Preview->bShowRebuildProgress = false;
    Preview->bSpawnActorHandles = false;
    Preview->bSpawnHierarchyHandles = false;
    Preview->bShowAllActorMeshes = false;
    Preview->bShowActorMesh = false;
    Preview->bShowClusterMeshes = false;
    Preview->bShowSprites = false;
    Preview->PreviewBaseSector = 1;
    auto Build = [Preview]() -> int64 {
        Preview->OnConstruction(FTransform::Identity);
        int64 Triangles = 0;
        for (int32 Index = 0; Index < Preview->Mesh->GetNumSections(); ++Index)
            if (const FProcMeshSection* Section = Preview->Mesh->GetProcMeshSection(Index))
                Triangles += Section->ProcIndexBuffer.Num() / 3;
        return Triangles;
    };
    Preview->SectorBanks = {One};
    const int64 OneOnly = Build();
    TestTrue(TEXT("Sector one has background triangles"), OneOnly > 0);
    Preview->SectorBanks = {One, Two, Seven, Twelve};
    TestEqual(TEXT("Extra sectors do not stack backgrounds on sector one"), Build(), OneOnly);
    TestTrue(TEXT("Inactive background bounds remain available for diagnostics"), Preview->GetSectorBackgroundBounds().Contains(12));
    Preview->bShowAllSectorBackgrounds = true;
    TestTrue(TEXT("Explicit all-backgrounds mode retains the old inspection behavior"), Build() > OneOnly);
    Preview->bShowAllSectorBackgrounds = false;
    Preview->PreviewBaseSector = 12;
    const int64 SelectedTwelve = Build();
    Preview->SectorBanks = {Twelve};
    TestEqual(TEXT("Changing base sector selects that background"), Build(), SelectedTwelve);
    Preview->SectorBanks = {};
    const int64 Common = Build();
    Preview->PreviewBaseSector = 7;
    Preview->SectorBanks = {Seven, One};
    TestEqual(TEXT("Sector without a background uses common even with other backgrounds loaded"), Build(), Common);
    Preview->PreviewBaseSector = 1;
    Preview->bShowClusterMeshes = true;
    Preview->SectorBanks = {One};
    const int64 OneClusters = Build();
    Preview->SectorBanks = {One, Two};
    TestTrue(TEXT("Inactive sector cluster geometry is still submitted"), Build() > OneClusters);
    Preview->Destroy();
    World->DestroyWorld(false);
    return true;
}
#endif
