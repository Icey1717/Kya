#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "KyaActorHandle.generated.h"

class AKyaLevelPreview;
class UKyaLevelBank;
class UStaticMeshComponent;
class UProceduralMeshComponent;
class USceneComponent;
class UMaterialInterface;
struct FKyaActorMeshSection;

/**
 * Selectable, editable representation of one bank actor placement.
 *
 * The handle owns the actor's game mesh (one procedural mesh section per material),
 * so clicking any strip of the mesh selects this actor and its Details panel shows
 * the placement. A small marker child keeps meshless records selectable.
 *
 * All edits go through the shared edit service: the bank record is authoritative,
 * only the edited fields are written, and the owning preview (and any other preview
 * of the same bank) refreshes once. Handles are editor-only and transient, and their
 * display transform never becomes serialized game scale.
 */
UCLASS()
class KYALEVEL_API AKyaActorHandle : public AActor
{
    GENERATED_BODY()
public:
    AKyaActorHandle();
    virtual void OnConstruction(const FTransform& Transform) override;
    virtual void Destroyed() override;
#if WITH_EDITOR
    virtual void PostEditMove(bool bFinished) override;
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

    /** Bind this handle to one bank actor placement and copy its fields. */
    void Initialize(AKyaLevelPreview* InPreview, UKyaLevelBank* InBank, int32 InActorIndex, bool bShowIcon);
    /** Copy the bank placement into the visible fields and world transform. */
    void SyncFromBank();
    /** Show or hide the meshless-record icon (diagnostic/capture modes hide it). */
    void SetIconEnabled(bool bEnabled);
    /** Replace the rendered actor mesh with sections captured by the owning preview. */
    void SetActorMeshSections(const TArray<FKyaActorMeshSection>& Sections);
    void ClearActorMesh();
    /** Write the visible fields back to the bank asset through the edit service. */
    UFUNCTION(BlueprintCallable, Category="Kya|Selection")
    void ApplyToBank();

    /** Neutral root; the marker and mesh attach to it. */
    UPROPERTY(VisibleAnywhere, Category="Kya|Identity")
    TObjectPtr<USceneComponent> SceneRoot;

    /** The actor's own game geometry. Selection follows the rendered mesh, not a proxy volume. */
    UPROPERTY(VisibleAnywhere, Category="Kya|Identity")
    TObjectPtr<UProceduralMeshComponent> ActorMesh;

    UPROPERTY(VisibleAnywhere, Category="Kya|Identity")
    TObjectPtr<UStaticMeshComponent> Marker;

    UPROPERTY(VisibleAnywhere, Category="Kya|Identity")
    TObjectPtr<UKyaLevelBank> Bank;

    UPROPERTY(VisibleAnywhere, Transient, Category="Kya|Identity")
    TObjectPtr<AKyaLevelPreview> Preview;

    /** Index in the bank's actor array. Counts are fixed while the writer requires it. */
    UPROPERTY(VisibleAnywhere, Category="Kya|Identity")
    int32 ActorIndex = INDEX_NONE;

    /** Bank edit generation this handle was bound to; a reimport invalidates it. */
    UPROPERTY(VisibleAnywhere, Category="Kya|Identity")
    int32 EditGeneration = 0;

    UPROPERTY(VisibleAnywhere, Category="Kya|Identity")
    int64 ClassId = 0;

    /** Serialized actor name; the Outliner label is derived from it. */
    UPROPERTY(EditAnywhere, Category="Kya|Identity", meta=(EditCondition="ActorIndex >= 0"))
    FString ActorName;

    UPROPERTY(EditAnywhere, Category="Kya|Rendering", meta=(EditCondition="ActorIndex >= 0"))
    int32 ActorMeshIndex = 0;

    UPROPERTY(EditAnywhere, Category="Kya|Rendering", meta=(EditCondition="ActorIndex >= 0"))
    int32 ActorTextureIndex = 0;

    /** Game-space position (x, y, z); Y is up and units are metres. */
    UPROPERTY(EditAnywhere, Category="Kya|Transform", meta=(EditCondition="ActorIndex >= 0"))
    FVector ActorPosition = FVector::ZeroVector;

    /** Game-space rotation angles in radians. Yaw-only classes ignore pitch and roll. */
    UPROPERTY(EditAnywhere, Category="Kya|Transform", meta=(EditCondition="ActorIndex >= 0"))
    FVector ActorRotation = FVector::ZeroVector;

    /** Game-space scale per axis. */
    UPROPERTY(EditAnywhere, Category="Kya|Transform", meta=(EditCondition="ActorIndex >= 0"))
    FVector ActorScale = FVector::OneVector;

    UPROPERTY(EditAnywhere, Category="Kya|Gameplay", meta=(EditCondition="ActorIndex >= 0"))
    int32 ActorSectorId = 0;

    UPROPERTY(EditAnywhere, Category="Kya|Gameplay", meta=(EditCondition="ActorIndex >= 0"))
    int32 ActorDefaultBehaviourId = 0;

    UPROPERTY(EditAnywhere, Category="Kya|Gameplay", meta=(EditCondition="ActorIndex >= 0"))
    float ActorVisibilityDistance = 0;

    UPROPERTY(EditAnywhere, Category="Kya|Gameplay", meta=(EditCondition="ActorIndex >= 0"))
    float ActorLightingValue = 0;

    UPROPERTY(EditAnywhere, Category="Kya|Gameplay", meta=(EditCondition="ActorIndex >= 0"))
    int64 ActorLightingFlags = 0;

    /** Number of rendered material sections (0 for meshless records). */
    UPROPERTY(VisibleAnywhere, Category="Kya|Diagnostics")
    int32 RenderSectionCount = 0;

private:
    bool bSyncing = false;
    bool bShowIcon = true;
    /** Materials are owned by the preview; keep explicit references for GC safety. */
    UPROPERTY()
    TArray<TObjectPtr<UMaterialInterface>> SectionMaterials;

    /** Write the current relative transform back to the bank, or restore it if rejected. */
    void CommitTransformFromActor();
    void MirrorFromPlacement();
};
