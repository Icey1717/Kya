#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "KyaLevelPreview.generated.h"

class UKyaLevelBank;
class UKyaLevelSet;
class UProceduralMeshComponent;
class UMaterialInterface;
class UTexture2D;
class UMaterial;
class UKyaWaypointComponent;
class USplineComponent;
class UKyaPathPointComponent;
class UKyaActorMarkerComponent;
class AKyaActorHandle;
class AKyaActorGroup;
class AKyaSpatialRecordHandle;
class AKyaHierarchyHandle;
class UPostProcessComponent;
class UMaterialInstanceDynamic;

/**
 * One material batch of one bank actor's game mesh.
 *
 * Vertices are preview-local in Unreal axes: the native hierarchy transform and
 * the actor's class rotation/scale are baked in, actor translation is not. The
 * owning actor handle applies position/rotation/scale through its transform.
 */
struct KYALEVEL_API FKyaActorMeshSection
{
    UMaterialInterface* Material = nullptr;
    TArray<FVector> Positions;
    TArray<int32> Triangles;
    TArray<FVector2D> UVs;
    TArray<FColor> Colors;
    uint64 MaterialHash = 0;
    int32 StripIndex = INDEX_NONE;
    int32 TriangleCount() const { return Triangles.Num() / 3; }
};

/** Experimental level geometry preview. */
UCLASS()
class KYALEVEL_API AKyaLevelPreview : public AActor
{
    GENERATED_BODY()
public:
    AKyaLevelPreview();
    virtual void OnConstruction(const FTransform& Transform) override;
    virtual void Tick(float DeltaSeconds) override;
    virtual void Destroyed() override;
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

    /**
     * Optional persisted level descriptor. When assigned, its main bank and sector
     * associations are authoritative for this preview and copied into Bank/SectorBanks
     * on every build, so reopening a map restores the level automatically.
     */
    UPROPERTY(EditAnywhere, Category="Kya")
    TObjectPtr<UKyaLevelSet> LevelSet;

    /** Copy the level set's main bank and sector associations into this preview. */
    void ApplyLevelSet();
    /** Sector ID of a loaded bank, preferring the level set's mapping over its filename. */
    int32 ResolveSectorId(const UKyaLevelBank& Sector) const;

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

    /**
     * Spawn selectable actor representations for actor placements in the loaded
     * sectors. Each representation appears in the World Outliner under the bank
     * actor's name, renders the actor's own game mesh and exposes the placement
     * fields in its Details panel; edits write back to the bank asset. Handles are
     * editor-only and transient. Enabled by default so clicking visible actor
     * geometry selects the actor without enabling diagnostics first.
     */
    UPROPERTY(EditAnywhere, Category="Kya|Selection")
    bool bSpawnActorHandles = true;

    /**
     * Render each actor's game mesh on its own handle instead of the shared preview
     * mesh. This is what makes clicking a strip select the owning actor. Diagnostic
     * mesh modes (Show Actor Mesh / Show All Actor Meshes) keep the shared route so
     * capture output is unchanged.
     */
    UPROPERTY(EditAnywhere, Category="Kya|Selection")
    bool bShowActorMeshesOnHandles = true;

    /** Show a small marker for actors that have no renderable mesh (or in mesh modes). */
    UPROPERTY(EditAnywhere, Category="Kya|Selection")
    bool bShowActorIcons = true;

    /**
     * Lock generated representations against viewport picking. Presentation state
     * only: the actors stay visible and editable through the bank record.
     */
    UPROPERTY(EditAnywhere, Category="Kya|Selection")
    bool bLockGeneratedActorSelection = false;

    /**
     * Spawn selectable handles for waypoints. When enabled, the diagnostic waypoint
     * arrows are suppressed to avoid drawing two markers per record.
     */
    UPROPERTY(EditAnywhere, Category="Kya|Selection")
    bool bSpawnWaypointHandles = false;

    /**
     * Spawn selectable handles for every point of every path, labelled with path and
     * point index. This makes spatial records discoverable without setting a numeric
     * preview path index; the spline guide still follows PreviewPathIndex.
     */
    UPROPERTY(EditAnywhere, Category="Kya|Selection")
    bool bSpawnPathPointHandles = false;

    /** Number of live waypoint/path-point handles. */
    UFUNCTION(BlueprintCallable, Category="Kya|Selection")
    int32 GetSpatialHandleCount() const { return SpatialHandles.Num(); }

    /** Waypoint handle by waypoint index, or null. */
    UFUNCTION(BlueprintCallable, Category="Kya|Selection")
    AActor* GetWaypointHandle(int32 WaypointIndex) const;

    /** Path-point handle by path and point index, or null. */
    UFUNCTION(BlueprintCallable, Category="Kya|Selection")
    AActor* GetPathPointHandle(int32 PathIndex, int32 PointIndex) const;

#if WITH_EDITOR
    /** True when Actor is one of this preview's generated representations. */
    bool OwnsGeneratedActor(const AActor* Actor) const;
    /**
     * Destroy duplicated/pasted/alt-dragged generated actors that this preview does
     * not own. Generated representations must not survive structural commands.
     * Returns the number of actors destroyed.
     */
    int32 DestroyStrayGeneratedActors();
    /** Derived Outliner label for one actor record (name + class + duplicate index). */
    FString GetActorDisplayLabel(int32 ActorIndex) const;
    /**
     * Temporarily hide one generated representation in this editor only. Uses
     * SetIsTemporarilyHiddenInEditor, so game visibility and bank data are untouched.
     */
    UFUNCTION(BlueprintCallable, Category="Kya|Selection")
    void SetGeneratedActorTemporarilyHidden(AActor* Actor, bool bInHidden);
    UFUNCTION(BlueprintCallable, Category="Kya|Selection")
    bool IsGeneratedActorTemporarilyHidden(const AActor* Actor) const;
    /** Hide every generated representation except Actors, until isolation is cleared. */
    UFUNCTION(BlueprintCallable, Category="Kya|Selection")
    void IsolateGeneratedActors(const TArray<AActor*>& Actors);
    UFUNCTION(BlueprintCallable, Category="Kya|Selection")
    void ClearGeneratedActorIsolation();
    UFUNCTION(BlueprintCallable, Category="Kya|Selection")
    bool IsGeneratedActorIsolated(const AActor* Actor) const;
    /** Reapply temporary visibility and selection-lock state after a build. */
    void ApplyGeneratedActorPresentation();
#endif

    /** True when actor geometry belongs to the actor handles and must not enter the shared mesh. */
    bool ShouldActorHandlesRenderGeometry() const;
    bool ShouldShowActorIcon() const;
    /** Mesh sections captured for one bank actor during the last build, or null. */
    const TArray<FKyaActorMeshSection>* FindActorMeshSections(int32 ActorIndex) const;
    /** Refresh after an edit to a bank this preview displays; rebuilds once. */
    void HandleBankEdited(UKyaLevelBank* InBank);

    /**
     * Spawn selectable handles for static world-geometry hierarchies (the smallest
     * practical unit: a cluster node's strip group in a sector or common mesh).
     */
    UPROPERTY(EditAnywhere, Category="Kya|Selection")
    bool bSpawnHierarchyHandles = false;

    /** Hierarchy keys hidden from the preview by a hierarchy handle. */
    UPROPERTY(EditAnywhere, Category="Kya|Selection")
    TSet<FString> HiddenHierarchies;

    /** Hide or show one world-geometry hierarchy and rebuild the preview. */
    void SetHierarchyVisible(UKyaLevelBank* InBank, int32 HierarchyOffset, bool bVisible);

    /** Rebuild the preview after an external edit to the bank asset (such as a handle edit). */
    void RebuildFromBank();

    /** Number of completed preview builds; lets tests confirm that edits refresh the mesh. */
    UPROPERTY(VisibleAnywhere, Category="Kya|Diagnostics")
    int64 BuildCount = 0;

    /** Number of path-point markers for the previewed path. */
    UFUNCTION(BlueprintCallable, Category="Kya|Paths")
    int32 GetPathMarkerCount() const { return PathMarkers.Num(); }

    /** Path-point marker by index, or null. */
    UFUNCTION(BlueprintCallable, Category="Kya|Paths")
    UKyaPathPointComponent* GetPathMarker(int32 MarkerIndex) const;

    /** Number of live selectable actor handles. */
    UFUNCTION(BlueprintCallable, Category="Kya|Selection")
    int32 GetActorHandleCount() const { return ActorHandles.Num(); }

    /** Handle for a bank actor index, or null. */
    UFUNCTION(BlueprintCallable, Category="Kya|Selection")
    AActor* GetActorHandle(int32 ActorIndex) const;

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
#if WITH_EDITOR
    /** Create, update, or destroy the selectable actor handles after a rebuild. */
    void RefreshActorHandles();
    /** Create, update, or destroy the waypoint and path-point handles after a rebuild. */
    void RefreshSpatialHandles();
    /** Create, update, or destroy the selectable world-geometry hierarchy handles. */
    void RefreshHierarchyHandles();
#endif
    struct FHierarchySelectionInfo
    {
        UKyaLevelBank* Bank = nullptr;
        int32 SectorId = -1;
        int32 Offset = 0;
        int32 Strips = 0;
        int32 Triangles = 0;
        FVector Origin = FVector::ZeroVector;
    };
    FString MakeHierarchyKey(const UKyaLevelBank* InBank, int32 HierarchyOffset) const;
    TMap<FString, FHierarchySelectionInfo> HierarchySelectionInfo;
    UPROPERTY(Transient)
    TArray<TObjectPtr<AKyaHierarchyHandle>> HierarchyHandles;
    UPROPERTY(Transient)
    TMap<int32, TObjectPtr<AKyaActorHandle>> ActorHandles;
    /** Editor-only sector grouping actors for the Outliner (sector 0 = global actors). */
    UPROPERTY(Transient)
    TMap<int32, TObjectPtr<AKyaActorGroup>> ActorSectorGroups;
    /** Editor-only waypoint/path-point handles, keyed by "W:<index>" or "P:<path>:<point>". */
    UPROPERTY(Transient)
    TMap<FString, TObjectPtr<AKyaSpatialRecordHandle>> SpatialHandles;
    /** Editor-only temporary hiding of generated representations (per preview). */
    UPROPERTY(Transient)
    TSet<TWeakObjectPtr<AActor>> TemporarilyHiddenGeneratedActors;
    /** Editor-only isolation set; when active, every other representation is hidden. */
    UPROPERTY(Transient)
    TSet<TWeakObjectPtr<AActor>> IsolatedGeneratedActors;
    UPROPERTY(Transient)
    bool bIsolationActive = false;
    UPROPERTY(Transient)
    TObjectPtr<UKyaActorMarkerComponent> ActorMarker;
    UPROPERTY(Transient)
    TObjectPtr<UKyaLevelBank> ActorMarkerBank;
    /** Per-actor game mesh captured during the last build, keyed by bank actor index. */
    TMap<int32, TArray<FKyaActorMeshSection>> ActorMeshSections;
    /** Level-set sector ID mapping (descriptor-authoritative), bank to ID. */
    TMap<int32, TObjectPtr<UKyaLevelBank>> LevelSetSectors;
    /** Editor only: bank notification binding so asset edits and undo refresh the preview. */
    void BindBankNotifications();
    void UnbindBankNotifications();
    void OnMonitoredObjectChanged(UObject* Object, FPropertyChangedEvent& Event);
    void OnMonitoredObjectTransacted(UObject* Object, const FTransactionObjectEvent& Event);
    FDelegateHandle BankChangedHandle;
    FDelegateHandle BankTransactedHandle;
    /** Banks this preview is currently registered against (main and sectors). */
    TSet<TWeakObjectPtr<UKyaLevelBank>> BoundBanks;
};
