#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "KyaLevelPreview.generated.h"

class UKyaLevelBank;
class UProceduralMeshComponent;
class UMaterialInterface;
class UTexture2D;
class UMaterial;
class UKyaWaypointComponent;
class USplineComponent;
class UKyaPathPointComponent;
class UKyaActorMarkerComponent;
class UPostProcessComponent;
class UMaterialInstanceDynamic;

/** Experimental level geometry preview. */
UCLASS()
class KYALEVEL_API AKyaLevelPreview : public AActor
{
    GENERATED_BODY()
public:
    AKyaLevelPreview();
    virtual void OnConstruction(const FTransform& Transform) override;
    virtual void Tick(float DeltaSeconds) override;
#if WITH_EDITOR
    virtual bool ShouldTickIfViewportsOnly() const override { return true; }
    /** Rebuild the preview mesh when an editable property changes. */
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
    /** Update camera-relative background geometry using an editor camera position in world centimetres. */
    UFUNCTION(BlueprintCallable, Category="Kya|Preview")
    void SetBackgroundCameraPosition(const FVector& WorldCameraPosition);
    /** Lock the preview to a capture camera, including screen-space flare sizing. */
    UFUNCTION(BlueprintCallable, Category="Kya|Preview")
    void SetPreviewCamera(const FVector& Position, const FVector& Forward, const FVector& Up,
        float HorizontalFovDegrees, int32 Width, int32 Height);
#if WITH_EDITOR
    void UpdatePathGuidePoint(int32 PathIndex, int32 PointIndex, const FVector& Position);
#endif

    UPROPERTY(EditAnywhere, Category="Kya")
    TObjectPtr<UKyaLevelBank> Bank;

    /** Optional imported SECT*.BNK assets; their background meshes are previewed. */
    UPROPERTY(EditAnywhere, Category="Kya")
    TArray<TObjectPtr<UKyaLevelBank>> SectorBanks;

    /** Native active sector for distance clipping; -1 uses the first sector bank. */
    UPROPERTY(EditAnywhere, Category="Kya|Visibility")
    int32 PreviewBaseSector = -1;

    /** Log per-sector entry, decoding, material and world-space bounds diagnostics on construction. */
    UPROPERTY(EditAnywhere, Category="Kya|Diagnostics")
    bool bLogSectorDiagnostics = false;

    /** Optional override with a Texture2D parameter named KyaTexture. */
    UPROPERTY(EditAnywhere, Category="Kya|Materials")
    TObjectPtr<UMaterialInterface> TextureMaterial;

    /**
     * Combine texture and vertex colours in the game's encoded colour space, then decode the
     * result for Unreal's linear output. This matches the game's vertex shading; disable it to
     * compare against Unreal's normal sRGB texture sampling.
     */
    UPROPERTY(EditAnywhere, Category="Kya|Materials", meta=(DisplayName="Game Colour Space"))
    bool bRawColorModulation = true;

    UPROPERTY(EditAnywhere, Category="Kya|Waypoints")
    bool bShowWaypoints = true;

    /** Index of a path to inspect; -1 hides the path preview. */
    UPROPERTY(EditAnywhere, Category="Kya|Paths", meta=(ClampMin="-1"))
    int32 PreviewPathIndex = -1;

    /** Index in the bank's actor array; -1 hides the marker. */
    UPROPERTY(EditAnywhere, Category="Kya|Actors", meta=(ClampMin="-1"))
    int32 PreviewActorIndex = -1;

    /** Optional native capture basis X/Y/Z and translation in game coordinates. */
    UPROPERTY(EditAnywhere, Transient, Category="Kya|Actors")
    TArray<FVector> PreviewActorNativeTransform;

    /** Bank actor index whose textured strips bypass RGB modulation for capture. */
    UPROPERTY(EditAnywhere, Category="Kya|Actors", meta=(ClampMin="-1"))
    int32 TextureOnlyActorIndex = -1;

    /** Experimental static model preview using the actor's initial class transform. */
    /** Render the common and sector background meshes. */
    UPROPERTY(EditAnywhere, Category="Kya|Visibility")
    bool bShowBackgroundMeshes = true;
    /** Render the common and sector static cluster meshes. */
    UPROPERTY(EditAnywhere, Category="Kya|Visibility")
    bool bShowClusterMeshes = true;

    /** Render prepared cluster sprite billboards. */
    UPROPERTY(EditAnywhere, Category="Kya|Visibility")
    bool bShowSprites = true;

    /** Camera forward used to expand sprite billboards, in game axes (Y up). */
    UPROPERTY(EditAnywhere, Category="Kya|Sprites")
    FVector SpriteCameraForward = FVector(1.0, 0.0, 0.0);

    /** Camera up used to expand sprite billboards, in game axes (Y up). */
    UPROPERTY(EditAnywhere, Category="Kya|Sprites")
    FVector SpriteCameraUp = FVector(0.0, 1.0, 0.0);

    /** GS draw-surface dimensions, independent of the PC framebuffer (PAL 512x512, NTSC 512x448). */
    UPROPERTY(EditAnywhere, Category="Kya|Sprites", meta=(ClampMin="1"))
    FIntPoint NativeSurfaceSize = FIntPoint(512, 512);

    UPROPERTY(EditAnywhere, Category="Kya|Actors")
    bool bShowActorMesh = false;

    /** Render only one strip of the previewed actor model; -1 renders every strip. */
    UPROPERTY(EditAnywhere, Category="Kya|Actors", meta=(ClampMin="-1"))
    int32 PreviewActorStripIndex = -1;

    /**
     * Apply the level light archive (sun/ambient entries) to actor-model vertex
     * colours using each strip's stored model normals, transformed by the hierarchy
     * and placement. Strips without stored normals keep their baked colours.
     */
    UPROPERTY(EditAnywhere, Category="Kya|Actors")
    bool bApplyActorLighting = true;

    /** Preview all actor models in their initial placement (experimental). */
    UPROPERTY(EditAnywhere, Category="Kya|Actors")
    bool bShowAllActorMeshes = false;

    UPROPERTY(VisibleAnywhere, Category="Kya")
    TObjectPtr<UProceduralMeshComponent> Mesh;

private:
    void UpdatePreviewCamera(const FVector& Position, const FVector& Forward, const FVector& Up,
        float HorizontalFovDegrees, int32 Width, int32 Height);
    void UpdateClusterVisibility(const FVector& Position, const FVector& Forward, const FVector& Right,
        const FVector& Up, double TanX, double TanY);
    struct FClusterVisibilityRange
    {
        int32 Section = 0;
        int32 Start = 0;
        int32 Count = 0;
        FVector4f ObjectSphere = FVector4f(0, 0, 0, 0);
        FVector4f StripSphere = FVector4f(0, 0, 0, 0);
        FVector4f LodSphere = FVector4f(0, 0, 0, 0);
        double MinLodBiasSquared = 0;
        double MaxLodBiasSquared = DBL_MAX;
        bool bVisible = true;
    };
    TArray<FClusterVisibilityRange> ClusterVisibilityRanges;
    TMap<int32, TArray<uint32>> ClusterBaseIndices;
    float ActiveClipDistance = 500;
    struct FBackgroundFlare
    {
        FVector Position = FVector::ZeroVector;
        int32 MaterialIndex = INDEX_NONE;
    };
    TArray<FBackgroundFlare> BackgroundFlares;
    bool bExplicitPreviewCamera = false;
    UPROPERTY(Transient)
    TObjectPtr<UPostProcessComponent> FlarePostProcess;
    UPROPERTY(Transient)
    TObjectPtr<UMaterial> GeneratedFlareMaterial;
    UPROPERTY(Transient)
    TArray<TObjectPtr<UMaterialInstanceDynamic>> FlareMaterials;
    struct FBackgroundRange
    {
        int32 Section = 0;
        int32 Start = 0;
        int32 Count = 0;
        FVector CameraX = FVector::ZeroVector;
        FVector CameraY = FVector::ZeroVector;
        FVector CameraZ = FVector::ZeroVector;
    };
    TArray<FBackgroundRange> BackgroundRanges;
    TMap<int32, TArray<FVector>> BackgroundBasePositions;
    FVector LastBackgroundCamera = FVector::ZeroVector;
    bool bHasBackgroundCamera = false;
    bool bGeneratedRawColorModulation = false;
    bool bIsRebuildingPreview = false;
    UPROPERTY(Transient)
    TArray<TObjectPtr<UTexture2D>> PreviewTextures;

    UPROPERTY(Transient)
    TObjectPtr<UMaterial> GeneratedPreviewMaterial;

    UPROPERTY(Transient)
    TObjectPtr<UMaterial> GeneratedMaskedPreviewMaterial;
    UPROPERTY(Transient)
    TObjectPtr<UMaterial> GeneratedAdditivePreviewMaterial;
    UPROPERTY(Transient)
    TObjectPtr<UMaterial> GeneratedTranslucentPreviewMaterial;

    UPROPERTY(Transient)
    TObjectPtr<UMaterial> GeneratedUntexturedMaterial;

    UPROPERTY(Transient)
    TArray<TObjectPtr<UKyaWaypointComponent>> WaypointMarkers;

    UPROPERTY(Transient)
    TObjectPtr<UKyaLevelBank> WaypointMarkerBank;

    UPROPERTY(Transient)
    TObjectPtr<USplineComponent> PathPreview;
    UPROPERTY(Transient)
    TArray<TObjectPtr<UKyaPathPointComponent>> PathMarkers;
    UPROPERTY(Transient)
    TObjectPtr<UKyaLevelBank> PathMarkerBank;
    int32 PathMarkerIndex = INDEX_NONE;
    UPROPERTY(Transient)
    TObjectPtr<UKyaActorMarkerComponent> ActorMarker;
    UPROPERTY(Transient)
    TObjectPtr<UKyaLevelBank> ActorMarkerBank;
};
