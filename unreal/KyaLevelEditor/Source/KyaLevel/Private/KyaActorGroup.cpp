#include "KyaActorGroup.h"

#include "Components/SceneComponent.h"

AKyaActorGroup::AKyaActorGroup()
{
    PrimaryActorTick.bCanEverTick = false;
    bIsEditorOnlyActor = true;
    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    RootComponent = SceneRoot;
    SceneRoot->SetMobility(EComponentMobility::Movable);
    SetActorEnableCollision(false);
#if WITH_EDITOR
    SetLockLocation(true);
#endif
}

#if WITH_EDITOR
void AKyaActorGroup::ResetToLevelRoot()
{
    SetActorRelativeTransform(FTransform::Identity, false, nullptr, ETeleportType::TeleportPhysics);
}

bool AKyaActorGroup::CanDeleteSelectedActor(FText& OutReason) const
{
    OutReason = NSLOCTEXT("KyaLevel", "CannotDeleteActorGroup",
        "Kya sector grouping actors are created with the level preview and cannot be deleted independently.");
    return false;
}

void AKyaActorGroup::PostEditMove(bool bFinished)
{
    ResetToLevelRoot();
    Super::PostEditMove(bFinished);
    ResetToLevelRoot();
}

void AKyaActorGroup::EditorApplyTranslation(const FVector& DeltaTranslation, bool bAltDown, bool bShiftDown, bool bCtrlDown)
{
    ResetToLevelRoot();
}

void AKyaActorGroup::EditorApplyRotation(const FRotator& DeltaRotation, bool bAltDown, bool bShiftDown, bool bCtrlDown)
{
    ResetToLevelRoot();
}

void AKyaActorGroup::EditorApplyScale(const FVector& DeltaScale, const FVector* PivotLocation, bool bAltDown,
    bool bShiftDown, bool bCtrlDown)
{
    ResetToLevelRoot();
}
#endif
