#pragma once

#include "CoreMinimal.h"
#include "Components/PrimitiveComponent.h"
#include "KyaPrimitives.h"
#if WITH_EDITOR
#include "ComponentVisualizer.h"
#endif
#include "KyaPrimitiveComponent.generated.h"

class UKyaLevelBank;
class UMaterial;

UENUM()
enum class EKyaPrimitiveDisplayMode : uint8
{
    Wireframe,
    Solid,
    SolidAndWireframe
};

#if WITH_EDITOR
struct KYALEVEL_API HKyaPrimitiveProxy : public HComponentVisProxy
{
    DECLARE_HIT_PROXY();
    int32 RecordIndex;
    uint32 Generation;
    HKyaPrimitiveProxy(const UActorComponent* Component, int32 Index, uint32 InGeneration)
        : HComponentVisProxy(Component, HPP_Wireframe), RecordIndex(Index), Generation(InGeneration) {}
};
#endif

/** Cached wire geometry, owned by the preview. Source records are retained separately
 * from render vertices so future edit handles can resolve exact serialized records. */
UCLASS()
class KYALEVEL_API UKyaPrimitiveComponent : public UPrimitiveComponent
{
    GENERATED_BODY()
public:
    UKyaPrimitiveComponent();
    void ConfigureDisplay(EKyaPrimitiveDisplayMode InCollisionMode, EKyaPrimitiveDisplayMode InEventMode,
        float InOpacity, float InWireThickness, float InDepthBias);
    void Rebuild(UKyaLevelBank* InBank, bool bCollision, bool bEvents);
    virtual FPrimitiveSceneProxy* CreateSceneProxy() override;
    virtual FBoxSphereBounds CalcBounds(const FTransform& LocalToWorld) const override;
    virtual void GetUsedMaterials(TArray<UMaterialInterface*>& OutMaterials, bool bGetDebugMaterials = false) const override;

    UPROPERTY(VisibleAnywhere, Category="Kya|Primitives")
    TObjectPtr<UKyaLevelBank> SourceBank;
    UPROPERTY(VisibleAnywhere, Category="Kya|Primitives")
    int32 PrimitiveCount = 0;
    UPROPERTY(VisibleAnywhere, Category="Kya|Primitives")
    TArray<FString> Diagnostics;
    /** Read-only source identity of the picked wire primitive, for authoring tools. */
    UPROPERTY(VisibleAnywhere, Transient, Category="Kya|Primitives")
    FString SelectedSource;
    void SelectRecord(int32 Index);
    int32 GetSelectedRecord() const { return SelectedRecord; }
    uint32 GetRecordGeneration() const { return RecordGeneration; }
    const TArray<FKyaPrimitiveRecord>& GetRecords() const { return Records; }
    EKyaPrimitiveDisplayMode CollisionMode = EKyaPrimitiveDisplayMode::Wireframe;
    EKyaPrimitiveDisplayMode EventMode = EKyaPrimitiveDisplayMode::Wireframe;
    float SolidOpacity = 0.25f;
    float WireThickness = 1.5f;
    float WireDepthBias = 0.1f;
    UPROPERTY(Transient)
    TObjectPtr<UMaterial> SolidMaterial;
private:
    TArray<FKyaPrimitiveRecord> Records;
    int32 SelectedRecord = INDEX_NONE;
    uint32 RecordGeneration = 0;
};
