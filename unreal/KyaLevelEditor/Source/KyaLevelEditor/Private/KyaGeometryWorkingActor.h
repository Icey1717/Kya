#pragma once
#include "CoreMinimal.h"
#include "DynamicMeshActor.h"
#include "KyaGeometryDocument.h"
#include "KyaGeometryWorkingActor.generated.h"

class AKyaLevelPreview;

/** Temporary Modeling Mode target. Applied bytes belong exclusively to the bank. */
UCLASS(Transient, NotPlaceable)
class AKyaGeometryWorkingActor : public ADynamicMeshActor
{
    GENERATED_BODY()
public:
    AKyaGeometryWorkingActor(const FObjectInitializer& Initializer);
    UPROPERTY(VisibleAnywhere, Transient, Category="Kya Geometry") TObjectPtr<UKyaLevelBank> Bank;
    UPROPERTY(VisibleAnywhere, Transient, Category="Kya Geometry") FGuid SessionId;
    UPROPERTY(VisibleAnywhere, Transient, Category="Kya Geometry") FString Status;
    UPROPERTY(VisibleAnywhere, Transient, Category="Kya Geometry") FString ResourceIdentity;
    UPROPERTY(VisibleAnywhere, Transient, Category="Kya Geometry") FString AffectedPlacements;
    UPROPERTY(VisibleAnywhere, Transient, Category="Kya Geometry") FString NativeFormatStatus;
    UPROPERTY(EditAnywhere, Transient, Category="Kya Geometry") bool bAssignPropertiesToNewFaces = false;
    UPROPERTY(EditAnywhere, Transient, Category="Kya Geometry", meta=(ClampMin="0", ClampMax="4294967295", ToolTip="Raw native surface bits. Verified: 0x80000000 suppresses detailed triangle/quad contact-record writes in edCollisions.cpp; it does not necessarily disable collision. Unknown bits are retained on untouched faces.")) int64 SurfaceFlags = 0;
    UPROPERTY(EditAnywhere, Transient, Category="Kya Geometry", meta=(ClampMin="0", ClampMax="4294967295", ToolTip="Set Selected Collision Surface Flags changes only these bits, preserving all others. Defaults to the verified contact-record bit. New and copied faces use the full Surface Flags value.")) int64 SurfaceFlagMask = 0x80000000;
    UPROPERTY(EditAnywhere, Transient, Category="Kya Geometry") bool bShowNormals = false;
    UPROPERTY(Transient) TObjectPtr<AKyaLevelPreview> Preview;
    UPROPERTY(Transient) TMap<int32, int64> SurfaceOverrides;

    void ApplyToKyaBank();
    void DiscardSession();
    void CopySelectedFacesToCollision();
    void SetSelectedCollisionSurfaceFlags();
    bool CanApplyNow() const;
    virtual bool ShouldTickIfViewportsOnly() const override { return true; }
    virtual void Tick(float DeltaSeconds) override;

    FKyaGeometryDocument Source;
    bool InitializeDocument(const FKyaGeometryDocument& Document, FString& Error);
    bool ReadDocument(FKyaGeometryDocument& Document, FString& Error) const;
    static void Open(AKyaLevelPreview* Preview, UKyaLevelBank* Bank, bool bCollision);
private:
    TMap<int32, FIntVector> OriginalTriangles;
    TMap<int32, FIntVector> NativeCorners;
    TMap<int32, int32> OriginalFaces;
    TMap<int32, int32> OriginalVertices;
    bool SelectedTriangles(TArray<int32>& Triangles, FString& Error) const;
    void Report(const FString& Message);
};
