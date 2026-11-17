#if WITH_DEV_AUTOMATION_TESTS
#include "KyaLevelBank.h"
#include "KyaLevelBankFactory.h"
#include "KyaLevelPreview.h"
#include "Engine/World.h"
#include "Materials/MaterialInterface.h"
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "ProceduralMeshComponent.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKyaArrowShadowTest, "Kya.Rendering.ArrowShadow",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKyaArrowShadowTest::RunTest(const FString& Parameters)
{
    const FString Filename = FPlatformMisc::GetEnvironmentVariable(TEXT("KYA_TEST_NATIV_BANK"));
    TArray<uint8> Bytes;
    if (!TestTrue(TEXT("Read NATIV bank (set KYA_TEST_NATIV_BANK)"),
        !Filename.IsEmpty() && FFileHelper::LoadFileToArray(Bytes, *Filename))) return false;
    const uint8* Cursor = Bytes.GetData();
    auto* Factory = NewObject<UKyaLevelBankFactory>();
    auto* Bank = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
        GetTransientPackage(), NAME_None, RF_Transient, nullptr, TEXT("bnk"), Cursor,
        Bytes.GetData() + Bytes.Num(), GWarn));
    if (!TestNotNull(TEXT("Import NATIV bank"), Bank)) return false;
    const int32 ActorIndex = Bank->SceneEnvironment.Actors.IndexOfByPredicate(
        [](const FKyaActorPlacement& Actor) { return Actor.Name == TEXT("AROW_07_L1"); });
    if (!TestTrue(TEXT("Find reported arrow"), ActorIndex != INDEX_NONE)) return false;
    UWorld* World = UWorld::CreateWorld(EWorldType::EditorPreview, false);
    if (!TestNotNull(TEXT("Create preview world"), World)) return false;
    auto* Preview = World->SpawnActor<AKyaLevelPreview>();
    Preview->Bank = Bank;
    Preview->bShowRebuildProgress = false;
    Preview->bSpawnActorHandles = false;
    Preview->bSpawnHierarchyHandles = false;
    Preview->bShowAllActorMeshes = false;
    Preview->bShowActorMesh = true;
    Preview->bShowBackgroundMeshes = false;
    Preview->bShowClusterMeshes = false;
    Preview->bShowSprites = false;
    Preview->PreviewActorIndex = ActorIndex;
    Preview->OnConstruction(FTransform::Identity);
    int32 Shadows = 0, Solids = 0;
    for (int32 Index = 0; Index < Preview->Mesh->GetNumSections(); ++Index)
    {
        const FProcMeshSection* Section = Preview->Mesh->GetProcMeshSection(Index);
        if (!Section || Section->ProcIndexBuffer.IsEmpty()) continue;
        const auto* Material = Preview->Mesh->GetMaterial(Index);
        if (!Material) continue;
        if (Material->GetBlendMode() == BLEND_Modulate) ++Shadows;
        if (Material->GetBlendMode() == BLEND_Opaque || Material->GetBlendMode() == BLEND_Masked) ++Solids;
    }
    TestTrue(TEXT("Subtractive arrow shadow uses framebuffer modulation"), Shadows > 0);
    TestTrue(TEXT("Arrow body retains solid material"), Solids > 0);
    World->DestroyWorld(false);
    return true;
}
#endif
