#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "KyaHierarchyHandle.generated.h"

class AKyaLevelPreview;
class UKyaLevelBank;
class UStaticMeshComponent;

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
    /** Generated hierarchy handles cannot be deleted independently of the preview. */
    virtual bool CanDeleteSelectedActor(FText& OutReason) const override;
#endif

    void Initialize(AKyaLevelPreview* InPreview, UKyaLevelBank* InBank, int32 InSectorId,
        int32 InHierarchyOffset, const FVector& InWorldOrigin, int32 InStripCount, int32 InTriangleCount,
        bool bInVisible);

    UPROPERTY(VisibleAnywhere, Category="Kya|Hierarchy")
    TObjectPtr<UStaticMeshComponent> Marker;

    UPROPERTY(VisibleAnywhere, Category="Kya|Hierarchy")
    TObjectPtr<UKyaLevelBank> Bank;

    UPROPERTY(VisibleAnywhere, Transient, Category="Kya|Hierarchy")
    TObjectPtr<AKyaLevelPreview> Preview;

    UPROPERTY(VisibleAnywhere, Category="Kya|Hierarchy")
    int32 SectorId = -1;

    /** Offset in the bank entry that starts this G3D hierarchy. */
    UPROPERTY(VisibleAnywhere, Category="Kya|Hierarchy")
    int32 HierarchyOffset = 0;

    UPROPERTY(VisibleAnywhere, Category="Kya|Hierarchy")
    int32 StripCount = 0;

    UPROPERTY(VisibleAnywhere, Category="Kya|Hierarchy")
    int32 TriangleCount = 0;

    /** Hide this hierarchy in the preview; bank geometry is not modified. */
    UPROPERTY(EditAnywhere, Category="Kya|Hierarchy")
    bool bVisible = true;
};
