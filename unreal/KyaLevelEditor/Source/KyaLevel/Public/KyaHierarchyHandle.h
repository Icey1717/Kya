#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "KyaHierarchyHandle.generated.h"

class AKyaLevelPreview;
class UKyaLevelBank;
class UProceduralMeshComponent;
class UStaticMeshComponent;

struct FKyaActorMeshSection;

/**
 * Selectable handle for one static world-geometry hierarchy (a cluster strip
 * group in a sector mesh or the common scene mesh).
 *
 * Perl-strip granularity is not practical (thousands of tiny strips with no
 * independent serialized transform), so the hierarchy is the smallest editable
 * unit. The handle can be clicked in the viewport or picked in the World
 * Outliner and its Details panel reports the hierarchy offset, strip/triangle
 * counts and an editable visibility toggle that hides the hierarchy in the
 * preview and re-exports nothing (bank geometry is unchanged).
 */
UCLASS()
class KYALEVEL_API AKyaHierarchyHandle : public AActor
{
    GENERATED_BODY()
public:
    AKyaHierarchyHandle();
    virtual void OnConstruction(const FTransform& Transform) override;
    virtual void Tick(float DeltaSeconds) override;
#if WITH_EDITOR
    virtual bool ShouldTickIfViewportsOnly() const override { return true; }
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
    virtual void PostEditMove(bool bFinished) override;
    virtual void EditorApplyTranslation(const FVector& DeltaTranslation, bool bAltDown, bool bShiftDown, bool bCtrlDown) override;
    virtual void EditorApplyRotation(const FRotator& DeltaRotation, bool bAltDown, bool bShiftDown, bool bCtrlDown) override;
    virtual void EditorApplyScale(const FVector& DeltaScale, const FVector* PivotLocation, bool bAltDown,
        bool bShiftDown, bool bCtrlDown) override;
    /** Generated hierarchy handles cannot be deleted independently of the preview. */
    virtual bool CanDeleteSelectedActor(FText& OutReason) const override;
    /** Locked representations stay visible but cannot be picked in the viewport. */
    virtual bool IsSelectable() const override;
#endif

    void Initialize(AKyaLevelPreview* InPreview, UKyaLevelBank* InBank, int32 InSectorId,
        int32 InHierarchyOffset, const FVector& InLocalOrigin, const FVector& InSourceOrigin,
        int32 InStripCount, int32 InTriangleCount, bool bInVisible);
    /** Replace the handle-owned hierarchy geometry and its metadata. */
    void SetHierarchyGeometry(const TArray<FKyaActorMeshSection>& Sections, const FBox& InWorldBounds,
        const TArray<FString>& InMaterials, const TArray<uint64>& InMaterialHashes);
    void ClearHierarchyGeometry();
    /** Set the Outliner label without treating it as a user rename. */
    void SetGeneratedLabel(const FString& Label);

    /** Hierarchy geometry owned by this handle; clicking a strip selects the handle. */
    UPROPERTY(VisibleAnywhere, Category="Kya|Hierarchy")
    TObjectPtr<UProceduralMeshComponent> HierarchyMesh;

    /** Fallback marker for hierarchies with no captured geometry. */
    UPROPERTY(VisibleAnywhere, Category="Kya|Hierarchy")
    TObjectPtr<UStaticMeshComponent> Marker;

    UPROPERTY(VisibleAnywhere, Category="Kya|Hierarchy")
    TObjectPtr<UKyaLevelBank> Bank;

    UPROPERTY(VisibleAnywhere, Transient, Category="Kya|Hierarchy")
    TObjectPtr<AKyaLevelPreview> Preview;

    /** Owning sector id (-1 for the common scene mesh). */
    UPROPERTY(VisibleAnywhere, Category="Kya|Hierarchy")
    int32 SectorId = -1;

    /** Offset in the bank entry that starts this G3D hierarchy. */
    UPROPERTY(VisibleAnywhere, Category="Kya|Hierarchy")
    int32 HierarchyOffset = 0;

    /** Game-space hierarchy origin. */
    UPROPERTY(VisibleAnywhere, Category="Kya|Hierarchy")
    FVector SourceOrigin = FVector::ZeroVector;

    /** World-space bounds of the captured strips (cm). */
    UPROPERTY(VisibleAnywhere, Category="Kya|Hierarchy")
    FBox WorldBounds = FBox(ForceInit);

    UPROPERTY(VisibleAnywhere, Category="Kya|Hierarchy")
    int32 StripCount = 0;

    UPROPERTY(VisibleAnywhere, Category="Kya|Hierarchy")
    int32 TriangleCount = 0;

    /** Material names (or hash fallbacks) used by this hierarchy. */
    UPROPERTY(VisibleAnywhere, Category="Kya|Hierarchy")
    TArray<FString> MaterialNames;

    UPROPERTY(VisibleAnywhere, Category="Kya|Hierarchy")
    TArray<int64> MaterialHashes;

    /** Hide this hierarchy in the preview; bank geometry is not modified. */
    UPROPERTY(EditAnywhere, Category="Kya|Hierarchy")
    bool bVisible = true;

    /** Locked representations stay visible but cannot be picked in the viewport. */
    UPROPERTY(EditAnywhere, Category="Kya|Selection")
    bool bSelectionLocked = false;

private:
    bool bApplyingGeneratedLabel = false;
    /** Materials are owned by the preview; keep references for GC safety. */
    UPROPERTY()
    TArray<TObjectPtr<UMaterialInterface>> SectionMaterials;
    void ResetToPreviewRoot();
};
