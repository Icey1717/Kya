#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "KyaActorHandle.generated.h"

class AKyaLevelPreview;
class UKyaLevelBank;
class UStaticMeshComponent;

/**
 * Selectable handle for one bank actor placement.
 *
 * Clicking the marker selects this actor in the viewport and World Outliner, and
 * its Details panel mirrors the editable placement fields of the bank entry.
 * Edits are written back to the bank asset (with undo support) and rebuild the
 * owning preview. Handles are opt-in on AKyaLevelPreview and transient, so they
 * never become part of a saved map.
 */
UCLASS()
class KYALEVEL_API AKyaActorHandle : public AActor
{
    GENERATED_BODY()
public:
    AKyaActorHandle();
    virtual void OnConstruction(const FTransform& Transform) override;
    virtual void Tick(float DeltaSeconds) override;
#if WITH_EDITOR
    virtual bool ShouldTickIfViewportsOnly() const override { return true; }
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

    /** Bind this handle to one bank actor placement and copy its fields. */
    void Initialize(AKyaLevelPreview* InPreview, UKyaLevelBank* InBank, int32 InActorIndex);
    virtual void Destroyed() override;
    /** Copy the bank placement into the visible fields and world transform. */
    void SyncFromBank();
    /** Write the visible fields back to the bank asset and rebuild the preview. */
    UFUNCTION(BlueprintCallable, Category="Kya|Selection")
    void ApplyToBank();

    UPROPERTY(VisibleAnywhere, Category="Kya|Selection")
    TObjectPtr<UStaticMeshComponent> Marker;

    UPROPERTY(VisibleAnywhere, Category="Kya|Selection")
    TObjectPtr<UKyaLevelBank> Bank;

    UPROPERTY(VisibleAnywhere, Transient, Category="Kya|Selection")
    TObjectPtr<AKyaLevelPreview> Preview;

    UPROPERTY(VisibleAnywhere, Category="Kya|Selection")
    int32 ActorIndex = INDEX_NONE;

    UPROPERTY(VisibleAnywhere, Category="Kya|Selection")
    int64 ClassId = 0;

    UPROPERTY(EditAnywhere, Category="Kya|Selection", meta=(EditCondition="ActorIndex >= 0"))
    FString ActorName;

    UPROPERTY(EditAnywhere, Category="Kya|Selection", meta=(EditCondition="ActorIndex >= 0"))
    int32 ActorMeshIndex = 0;

    UPROPERTY(EditAnywhere, Category="Kya|Selection", meta=(EditCondition="ActorIndex >= 0"))
    int32 ActorTextureIndex = 0;

    /** Game-space position (x, y, z); Y is up. */
    UPROPERTY(EditAnywhere, Category="Kya|Selection", meta=(EditCondition="ActorIndex >= 0"))
    FVector ActorPosition = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, Category="Kya|Selection", meta=(EditCondition="ActorIndex >= 0"))
    FVector ActorRotation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, Category="Kya|Selection", meta=(EditCondition="ActorIndex >= 0"))
    FVector ActorScale = FVector::OneVector;

    UPROPERTY(EditAnywhere, Category="Kya|Selection", meta=(EditCondition="ActorIndex >= 0"))
    int32 ActorSectorId = 0;

    UPROPERTY(EditAnywhere, Category="Kya|Selection", meta=(EditCondition="ActorIndex >= 0"))
    int32 ActorDefaultBehaviourId = 0;

    UPROPERTY(EditAnywhere, Category="Kya|Selection", meta=(EditCondition="ActorIndex >= 0"))
    float ActorVisibilityDistance = 0;

    UPROPERTY(EditAnywhere, Category="Kya|Selection", meta=(EditCondition="ActorIndex >= 0"))
    float ActorLightingValue = 0;

    UPROPERTY(EditAnywhere, Category="Kya|Selection", meta=(EditCondition="ActorIndex >= 0"))
    int64 ActorLightingFlags = 0;

private:
    bool bSyncing = false;
    /** Details edits apply on the next tick so the property assignment completes first. */
    bool bApplyPending = false;
};
