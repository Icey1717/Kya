#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "KyaSceneVisualizer.generated.h"

class AKyaLevelPreview;
class UKyaLevelBank;
class UArrowComponent;
class UProceduralMeshComponent;
class UStaticMeshComponent;

UENUM()
enum class EKyaVisualizerKind : uint8
{
    Light,
    MainCamera,
    ActorLink,
    TriggerBound
};

/**
 * Selectable diagnostic/editing shape for a level record that is not an actor
 * placement.
 *
 * Lights and the main scene camera use verified serialized fields and are editable
 * through the shared edit service. Actor links and trigger bounds are derived from
 * resolved references/checkpoints and are read-only; their shapes are approximate
 * and the Details panel says so. Authored collision and event primitives are decoded
 * separately from retained bank entries by UKyaPrimitiveComponent.
 */
UCLASS()
class KYALEVEL_API AKyaSceneVisualizer : public AActor
{
    GENERATED_BODY()
public:
    AKyaSceneVisualizer();
    virtual void OnConstruction(const FTransform& Transform) override;
    virtual void Destroyed() override;
#if WITH_EDITOR
    virtual void PostEditMove(bool bFinished) override;
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
    virtual bool CanDeleteSelectedActor(FText& OutReason) const override;
    virtual bool IsSelectable() const override;
#endif

    void InitializeLight(AKyaLevelPreview* InPreview, UKyaLevelBank* InBank, int32 InLightIndex);
    void InitializeCamera(AKyaLevelPreview* InPreview, UKyaLevelBank* InBank);
    void InitializeActorLink(AKyaLevelPreview* InPreview, UKyaLevelBank* InBank, int32 InActorIndex,
        FName InReferenceField, const FVector& GameStart, const FVector& GameEnd);
    void InitializeTrigger(AKyaLevelPreview* InPreview, UKyaLevelBank* InBank, int32 InActorIndex,
        int32 InCheckpointIndex, const FVector& GameCenter);
    void SyncFromBank();
    /** Set the Outliner label without treating it as a user rename. */
    void SetGeneratedLabel(const FString& Label);

    UPROPERTY(VisibleAnywhere, Category="Kya|Visualizer")
    TObjectPtr<USceneComponent> SceneRoot;

    UPROPERTY(VisibleAnywhere, Category="Kya|Visualizer")
    TObjectPtr<UStaticMeshComponent> Shape;

    UPROPERTY(VisibleAnywhere, Category="Kya|Visualizer")
    TObjectPtr<UArrowComponent> DirectionArrow;

    UPROPERTY(VisibleAnywhere, Category="Kya|Visualizer")
    TObjectPtr<UKyaLevelBank> Bank;

    UPROPERTY(VisibleAnywhere, Transient, Category="Kya|Visualizer")
    TObjectPtr<AKyaLevelPreview> Preview;

    UPROPERTY(VisibleAnywhere, Category="Kya|Visualizer")
    EKyaVisualizerKind Kind = EKyaVisualizerKind::Light;

    /** Light index, actor index, or checkpoint source index depending on Kind. */
    UPROPERTY(VisibleAnywhere, Category="Kya|Visualizer")
    int32 RecordIndex = INDEX_NONE;

    UPROPERTY(VisibleAnywhere, Category="Kya|Visualizer")
    int32 SubIndex = INDEX_NONE;

    UPROPERTY(VisibleAnywhere, Category="Kya|Visualizer")
    FName ReferenceField;

    /** True when the serialized record can be edited through the edit service. */
    UPROPERTY(VisibleAnywhere, Category="Kya|Visualizer")
    bool bRecordEditable = false;

    /** Explicit statement of what is verified vs approximate. */
    UPROPERTY(VisibleAnywhere, Category="Kya|Visualizer")
    FString Semantics;

    /** Reason shown after a transform/material edit was rejected. */
    UPROPERTY(VisibleAnywhere, Category="Kya|Visualizer")
    FString EditStatus;

    // --- Light (verified serialized fields) -------------------------------------
    UPROPERTY(EditAnywhere, Category="Kya|Light") int32 LightType = 0;
    UPROPERTY(EditAnywhere, Category="Kya|Light") int32 ShapeType = 0;
    UPROPERTY(EditAnywhere, Category="Kya|Light") FLinearColor LightColor = FLinearColor::White;
    UPROPERTY(EditAnywhere, Category="Kya|Light") FLinearColor AmbientColor = FLinearColor::Black;
    UPROPERTY(EditAnywhere, Category="Kya|Light") float Multiplier = 1.0f;
    UPROPERTY(EditAnywhere, Category="Kya|Light") FVector LightPosition = FVector::ZeroVector;
    UPROPERTY(EditAnywhere, Category="Kya|Light") FVector LightDirection = FVector::ZeroVector;
    UPROPERTY(EditAnywhere, Category="Kya|Light") float Falloff0 = 0.0f;
    UPROPERTY(EditAnywhere, Category="Kya|Light") float Falloff1 = 0.0f;
    UPROPERTY(EditAnywhere, Category="Kya|Light") float Fov0 = 0.0f;
    UPROPERTY(EditAnywhere, Category="Kya|Light") float Fov1 = 0.0f;

    // --- Main camera (verified serialized values; frustum shape is approximate) --
    UPROPERTY(EditAnywhere, Category="Kya|Camera") FVector CameraPosition = FVector::ZeroVector;
    UPROPERTY(EditAnywhere, Category="Kya|Camera") FVector CameraLookAt = FVector::ZeroVector;
    UPROPERTY(EditAnywhere, Category="Kya|Camera") FVector CameraUp = FVector::ZeroVector;
    UPROPERTY(EditAnywhere, Category="Kya|Camera") float CameraFov = 0.0f;

    // --- Derived links/triggers (read-only) -------------------------------------
    UPROPERTY(VisibleAnywhere, Category="Kya|Derived") FVector LinkStart = FVector::ZeroVector;
    UPROPERTY(VisibleAnywhere, Category="Kya|Derived") FVector LinkEnd = FVector::ZeroVector;
    UPROPERTY(VisibleAnywhere, Category="Kya|Derived") float TriggerRadius = 200.0f;

    /** Locked diagnostic representations stay visible but cannot be picked. */
    UPROPERTY(EditAnywhere, Category="Kya|Selection")
    bool bSelectionLocked = false;

private:
    bool bSyncing = false;
    bool bApplyingGeneratedLabel = false;
    void MirrorFromBank();
    void CommitFromActor();
    void ResetDiagnosticTransform();
    void RebuildShape();
};

