#include "KyaActorGroup.h"

#include "Components/SceneComponent.h"
#include "KyaLevelPreview.h"
#if WITH_EDITOR
#include "Misc/App.h"
#include "Misc/MessageDialog.h"
#endif

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

bool AKyaActorGroup::IsSelectable() const
{
    if (bSelectionLocked) return false;
    if (const AKyaLevelPreview* Preview = Cast<AKyaLevelPreview>(GetAttachParentActor()))
        return !Preview->bLockGeneratedActorSelection;
    return true;
}

void AKyaActorGroup::SetGeneratedLabel(const FString& Label)
{
    GeneratedLabel = Label;
    TGuardValue<bool> Guard(bApplyingGeneratedLabel, true);
    if (GetActorLabel() != Label) SetActorLabel(Label);
}

void AKyaActorGroup::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    const FName PropertyName = PropertyChangedEvent.MemberProperty
        ? PropertyChangedEvent.MemberProperty->GetFName()
        : PropertyChangedEvent.Property ? PropertyChangedEvent.Property->GetFName() : NAME_None;
    if (!bApplyingGeneratedLabel && PropertyName == TEXT("ActorLabel"))
    {
        SetGeneratedLabel(GeneratedLabel);
        UE_LOG(LogTemp, Warning, TEXT("KYA_GROUP_RENAME_REJECTED group=%s: labels are generated"), *GetNameSafe(this));
        if (!FApp::IsUnattended())
            FMessageDialog::Open(EAppMsgType::Ok, NSLOCTEXT("KyaLevel", "GroupRenameRejected",
                "Sector grouping labels are generated from the level preview and cannot be renamed. "
                "The label was restored."));
    }
    Super::PostEditChangeProperty(PropertyChangedEvent);
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
