#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "KyaActorGroup.generated.h"

class USceneComponent;
class AKyaLevelPreview;

/**
 * Editor-only transform parent used to group generated actor representations by
 * sector in the World Outliner. It has no serialized state, is recreated with the
 * level preview, and cannot be moved: its relative transform stays identity so a
 * child's writeback is always relative to the preview.
 */
UCLASS()
class KYALEVEL_API AKyaActorGroup : public AActor
{
    GENERATED_BODY()
public:
    AKyaActorGroup();
#if WITH_EDITOR
    /** Keep the group at the level root even if the editor tries to move it. */
    virtual void PostEditMove(bool bFinished) override;
    virtual void EditorApplyTranslation(const FVector& DeltaTranslation, bool bAltDown, bool bShiftDown, bool bCtrlDown) override;
    virtual void EditorApplyRotation(const FRotator& DeltaRotation, bool bAltDown, bool bShiftDown, bool bCtrlDown) override;
    virtual void EditorApplyScale(const FVector& DeltaScale, const FVector* PivotLocation, bool bAltDown, bool bShiftDown, bool bCtrlDown) override;
    /** Organizational actors cannot be deleted independently of their handles. */
    virtual bool CanDeleteSelectedActor(FText& OutReason) const override;
    /** Locked representations stay visible but cannot be picked in the viewport. */
    virtual bool IsSelectable() const override;
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

    /** Set the Outliner label without treating it as a user rename. */
    void SetGeneratedLabel(const FString& Label);

    UPROPERTY(VisibleAnywhere, Category="Kya|Group")
    TObjectPtr<USceneComponent> SceneRoot;

    /** Locked representations stay visible but cannot be picked in the viewport. */
    UPROPERTY(EditAnywhere, Category="Kya|Selection")
    bool bSelectionLocked = false;

private:
    bool bApplyingGeneratedLabel = false;
    /** Last label assigned by the preview, used to revert an Outliner rename. */
    FString GeneratedLabel;
    void ResetToLevelRoot();
};
