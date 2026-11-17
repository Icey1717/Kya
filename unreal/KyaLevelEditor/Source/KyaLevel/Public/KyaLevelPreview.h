#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HAL/Event.h"
#include "HAL/ThreadSafeCounter.h"
#include "HAL/CriticalSection.h"
#include "KyaG2D.h"
#include "KyaPrimitiveComponent.h"
#include "KyaLevelPreview.generated.h"

class UKyaLevelBank;
class UKyaLevelSet;
class UKyaLevelSettings;
struct FKyaActorDecodeCache;
struct FKyaG3DDecodedStrip;
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
class AKyaSceneVisualizer;
class UKyaPrimitiveComponent;
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
    virtual void PostEditUndo() override;
#endif
    /** Update camera-relative background geometry using an editor camera position in world centimetres. */
    UFUNCTION(BlueprintCallable, Category="Kya|Preview")
    void SetBackgroundCameraPosition(const FVector& WorldCameraPosition);
    /** Lock the preview to a capture camera, including screen-space flare sizing. */
    UFUNCTION(BlueprintCallable, Category="Kya|Preview")
    void SetPreviewCamera(const FVector& Position, const FVector& Forward, const FVector& Up,
        float HorizontalFovDegrees, int32 Width, int32 Height);
    /** Return to the live editor camera (clears the comparison/capture camera lock). */
    UFUNCTION(BlueprintCallable, Category="Kya|Preview")
    void ClearPreviewCamera();
    UFUNCTION(BlueprintCallable, Category="Kya|Preview")
    bool HasExplicitPreviewCamera() const { return bExplicitPreviewCamera; }
#if WITH_EDITOR
    void UpdatePathGuidePoint(int32 PathIndex, int32 PointIndex, const FVector& Position);
#endif

    UPROPERTY(EditAnywhere, Category="Kya|Authoring")
    TObjectPtr<UKyaLevelBank> Bank;

    /**
     * Optional persisted level descriptor. When assigned, its main bank and sector
     * associations are authoritative for this preview and copied into Bank/SectorBanks
     * on every build, so reopening a map restores the level automatically.
     */
    UPROPERTY(EditAnywhere, Category="Kya|Authoring")
    TObjectPtr<UKyaLevelSet> LevelSet;

    /** Copy the level set's main bank and sector associations into this preview. */
    void ApplyLevelSet();

    /**
     * Transient authoring surface for environment/camera/fog/scene configuration.
     * Created on demand by the Level Settings tab; edits are written through the edit
     * service and are not part of the render/capture diagnostics.
     */
    UPROPERTY(Transient)
    TObjectPtr<UKyaLevelSettings> LevelSettings;

    UFUNCTION(BlueprintCallable, Category="Kya|Level")
    UKyaLevelSettings* GetOrCreateLevelSettings();
    /** Sector ID of a loaded bank, preferring the level set's mapping over its filename. */
    int32 ResolveSectorId(const UKyaLevelBank& Sector) const;

    /**
     * Preview-local background bounds captured per loaded sector during the last
     * published build. Diagnostic only; actor sector assignments are never inferred
     * from these bounds.
     */
    const TMap<int32, FBox>& GetSectorBackgroundBounds() const { return SectorBackgroundBounds; }
    /**
     * Sector ids whose loaded background bounds contain a game-space position. This is
     * a hint for the explicit sector picker, not a membership test; callers must not
     * write the result into a record automatically.
     */
    TArray<int32> GetCandidateSectorIdsForPosition(const FVector& GamePosition) const;
    /** Text describing the explicit assignment plus any candidate sectors. */
    FString DescribeSectorAssignment(int32 SectorId, const FVector& GamePosition) const;

    /** Optional imported SECT*.BNK assets; their background meshes are previewed. */
    UPROPERTY(EditAnywhere, Category="Kya|Authoring")
    TArray<TObjectPtr<UKyaLevelBank>> SectorBanks;

    /** Preview display only; associations and exported bank data are unchanged. */
    UPROPERTY()
    TSet<int32> HiddenSectorIds;

    bool IsSectorVisible(int32 SectorId) const;
    void SetSectorVisible(int32 SectorId, bool bVisible);
    /** Show one sector, retaining the pre-solo visibility until restored. Global content stays visible. */
    void SoloSector(int32 SectorId);
    void RestoreSectorVisibility();
    bool CanRestoreSectorVisibility() const { return bHasSavedSectorVisibility; }

    /** Native active sector for background selection and distance clipping; -1 uses the common level background and clip distance. */
    UPROPERTY(EditAnywhere, Category="Kya|Diagnostics|Visibility")
    int32 PreviewBaseSector = -1;

    /** Draw inactive sectors' backgrounds too. Their sky layers can overlap the active sky. */
    UPROPERTY(EditAnywhere, Category="Kya|Diagnostics|Visibility")
    bool bShowAllSectorBackgrounds = false;

    /** Log per-sector entry, decoding, material and world-space bounds diagnostics on construction. */
    UPROPERTY(EditAnywhere, Category="Kya|Diagnostics")
    bool bLogSectorDiagnostics = false;

    /** Decode actor mesh strips on a worker thread for the next rebuild (stale results are discarded). */
    UPROPERTY(EditAnywhere, Category="Kya|Diagnostics")
    bool bAsyncDecodeMeshes = true;

    /** Show a cancellable progress dialog while decoding/rebuilding the preview. */
    UPROPERTY(EditAnywhere, Category="Kya|Diagnostics")
    bool bShowRebuildProgress = true;

    /** Status of the last build ("Completed", "Cancelled", ...). */
    UPROPERTY(VisibleAnywhere, Category="Kya|Diagnostics")
    FString RebuildStatus;

    /** Cancels the in-progress rebuild at the next phase boundary. */
    UFUNCTION(BlueprintCallable, Category="Kya|Diagnostics")
    void RequestCancelRebuild();

#if WITH_EDITOR
    /** Blocks until the current async mesh decode finishes (tests/tooling). */
    void WaitForAsyncDecode();
    int32 GetAsyncDecodeCompletionCount() const { return AsyncDecodeCompletions.GetValue(); }
    int32 GetStaleAsyncDecodeCount() const { return StaleDecodeDiscards.GetValue(); }
    int32 GetCachedActorDecodeCount() const { return CachedActorDecodes.GetValue(); }
    int32 GetSynchronousActorDecodeCount() const { return SynchronousActorDecodes.GetValue(); }
    /** Background/sector/cluster strip decodes performed by rebuilds. */
    int32 GetGeometryDecodeCount() const { return GeometryDecodes; }
    /** Rebuilds that reused cached background/sector/cluster strips. */
    int32 GetCachedGeometryDecodeCount() const { return CachedGeometryDecodes; }
#endif

    /** Optional override with KyaTexture and a float2 KyaUVOffset vector parameter. */
    UPROPERTY(EditAnywhere, Category="Kya|Diagnostics|Materials")
    TObjectPtr<UMaterialInterface> TextureMaterial;

    /** Animate the source's first-layer scrolling parameters in realtime viewports. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Kya|Preview|Textures")
    bool bAnimateTextures = true;

    /** Source increments are per game frame. Set the preview's source frame rate here. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Kya|Preview|Textures", meta=(ClampMin="1", ClampMax="240"))
    float TextureAnimationFrameRate = 60.0f;

    /** Restore serialized initial UV offsets, including while animation is paused. */
    UFUNCTION(BlueprintCallable, CallInEditor, Category="Kya|Preview|Textures")
    void ResetTextureAnimation();

    /**
     * Combine texture and vertex colours in the game's encoded colour space, then decode the
     * result for Unreal's linear output. This matches the game's vertex shading; disable it to
     * compare against Unreal's normal sRGB texture sampling.
     */
    UPROPERTY(EditAnywhere, Category="Kya|Diagnostics|Materials", meta=(DisplayName="Game Colour Space"))
    bool bRawColorModulation = true;

    UPROPERTY(EditAnywhere, Category="Kya|Diagnostics|Waypoints")
    bool bShowWaypoints = true;

    /** Index of a path to inspect; -1 hides the path preview. */
    UPROPERTY(EditAnywhere, Category="Kya|Diagnostics|Paths", meta=(ClampMin="-1"))
    int32 PreviewPathIndex = -1;

    /** Index in the bank's actor array; -1 hides the marker. */
    UPROPERTY(EditAnywhere, Category="Kya|Diagnostics|Actors", meta=(ClampMin="-1"))
    int32 PreviewActorIndex = -1;

    /** Optional native capture basis X/Y/Z and translation in game coordinates. */
    UPROPERTY(EditAnywhere, Transient, Category="Kya|Diagnostics|Actors")
    TArray<FVector> PreviewActorNativeTransform;

    /** Bank actor index whose textured strips bypass RGB modulation for capture. */
    UPROPERTY(EditAnywhere, Category="Kya|Diagnostics|Actors", meta=(ClampMin="-1"))
    int32 TextureOnlyActorIndex = -1;

    /** Experimental static model preview using the actor's initial class transform. */
    /** Render the common and sector background meshes. */
    UPROPERTY(EditAnywhere, Category="Kya|Diagnostics|Visibility")
    bool bShowBackgroundMeshes = true;
    /** Render the common and sector static cluster meshes. */
    UPROPERTY(EditAnywhere, Category="Kya|Diagnostics|Visibility")
    bool bShowClusterMeshes = true;

    /**
     * Editor discovery mode (default). Keeps actors from unloaded sectors and
     * distance-culled geometry visible for authoring while leaving game-like culling
     * intact for comparison/capture: setting a preview camera or disabling this mode
     * restores native visibility.
     */
    UPROPERTY(EditAnywhere, Category="Kya|Diagnostics|Visibility")
    bool bAuthoringVisibility = true;

    /** Authoring only: spawn handles for actors whose sector bank is not loaded. */
    UPROPERTY(EditAnywhere, Category="Kya|Diagnostics|Visibility", meta=(EditCondition="bAuthoringVisibility"))
    bool bAuthoringShowUnloadedSectors = true;

    /** Authoring only: draw the highest-detail cluster LOD regardless of native distance culling. */
    UPROPERTY(EditAnywhere, Category="Kya|Diagnostics|Visibility", meta=(EditCondition="bAuthoringVisibility"))
    bool bAuthoringShowCulledGeometry = true;

#if WITH_EDITOR
    /** Handles currently shown only because of authoring discovery (unloaded sector). */
    int32 GetAuthoringOnlyHandleCount() const { return AuthoringOnlyHandleCount; }
    /** True when authoring visibility overrides native culling right now. */
    bool IsAuthoringVisibilityActive() const;
#endif

    /** Render prepared cluster sprite billboards. */
    UPROPERTY(EditAnywhere, Category="Kya|Diagnostics|Visibility")
    bool bShowSprites = true;

    /** Camera forward used to expand sprite billboards, in game axes (Y up). */
    UPROPERTY(EditAnywhere, Category="Kya|Diagnostics|Sprites")
    FVector SpriteCameraForward = FVector(1.0, 0.0, 0.0);

    /** Camera up used to expand sprite billboards, in game axes (Y up). */
    UPROPERTY(EditAnywhere, Category="Kya|Diagnostics|Sprites")
    FVector SpriteCameraUp = FVector(0.0, 1.0, 0.0);

    /** GS draw-surface dimensions, independent of the PC framebuffer (PAL 512x512, NTSC 512x448). */
    UPROPERTY(EditAnywhere, Category="Kya|Diagnostics|Sprites", meta=(ClampMin="1"))
    FIntPoint NativeSurfaceSize = FIntPoint(512, 512);

    UPROPERTY(EditAnywhere, Category="Kya|Diagnostics|Actors")
    bool bShowActorMesh = false;

    /** Render only one strip of the previewed actor model; -1 renders every strip. */
    UPROPERTY(EditAnywhere, Category="Kya|Diagnostics|Actors", meta=(ClampMin="-1"))
    int32 PreviewActorStripIndex = -1;

    /**
     * Apply the level light archive (sun/ambient entries) to actor-model vertex
     * colours using each strip's stored model normals, transformed by the hierarchy
     * and placement. Strips without stored normals keep their baked colours.
     */
    UPROPERTY(EditAnywhere, Category="Kya|Diagnostics|Actors")
    bool bApplyActorLighting = true;

    /** Preview all actor models in their initial placement (experimental). */
    UPROPERTY(EditAnywhere, Category="Kya|Diagnostics|Actors")
    bool bShowAllActorMeshes = false;

    /**
     * Spawn selectable actor representations for actor placements in the loaded
     * sectors. Each representation appears in the World Outliner under the bank
     * actor's name, renders the actor's own game mesh and exposes the placement
     * fields in its Details panel; edits write back to the bank asset. Handles are
     * editor-only and transient. Enabled by default so clicking visible actor
     * geometry selects the actor without enabling diagnostics first.
     */
    UPROPERTY(EditAnywhere, Category="Kya|Diagnostics|Selection")
    bool bSpawnActorHandles = true;

    /**
     * Render each actor's game mesh on its own handle instead of the shared preview
     * mesh. This is what makes clicking a strip select the owning actor. Diagnostic
     * mesh modes (Show Actor Mesh / Show All Actor Meshes) keep the shared route so
     * capture output is unchanged.
     */
    UPROPERTY(EditAnywhere, Category="Kya|Diagnostics|Selection")
    bool bShowActorMeshesOnHandles = true;

    /** Show a small marker for actors that have no renderable mesh (or in mesh modes). */
    UPROPERTY(EditAnywhere, Category="Kya|Diagnostics|Selection")
    bool bShowActorIcons = true;

    /**
     * Lock generated representations against viewport picking. Presentation state
     * only: the actors stay visible and editable through the bank record.
     */
    UPROPERTY(EditAnywhere, Category="Kya|Diagnostics|Selection")
    bool bLockGeneratedActorSelection = false;

    /** Selectable light visualizers for every light record in the loaded banks. */
    UPROPERTY(EditAnywhere, Category="Kya|Diagnostics|Visualizers")
    bool bSpawnLightVisualizers = false;

    /** Selectable main-camera visualizer for the main bank. */
    UPROPERTY(EditAnywhere, Category="Kya|Diagnostics|Visualizers")
    bool bSpawnCameraVisualizer = false;

    /** Derived, read-only lines between actors and their resolved actor references. */
    UPROPERTY(EditAnywhere, Category="Kya|Diagnostics|Visualizers")
    bool bSpawnActorLinkVisualizers = false;

    /** Approximate, read-only trigger markers at checkpoint waypoints. */
    UPROPERTY(EditAnywhere, Category="Kya|Diagnostics|Visualizers")
    bool bSpawnTriggerVisualizers = false;

    /** Authored scenery collision surfaces and referenced box/sphere primitives. */
    UPROPERTY(EditAnywhere, Category="Kya|Diagnostics|Primitives")
    bool bShowCollisionPrimitives = false;

    /** Authored event zone geometry; cyan inclusion groups, red exclusion group. */
    UPROPERTY(EditAnywhere, Category="Kya|Diagnostics|Primitives")
    bool bShowEventPrimitives = false;

    UPROPERTY(EditAnywhere, Category="Kya|Diagnostics|Primitives")
    EKyaPrimitiveDisplayMode CollisionPrimitiveDisplay = EKyaPrimitiveDisplayMode::Wireframe;
    UPROPERTY(EditAnywhere, Category="Kya|Diagnostics|Primitives")
    EKyaPrimitiveDisplayMode EventPrimitiveDisplay = EKyaPrimitiveDisplayMode::Wireframe;
    /** Translucent faces allow the textured level to remain visible beneath them. */
    UPROPERTY(EditAnywhere, Category="Kya|Diagnostics|Primitives", meta=(ClampMin="0.05", ClampMax="1"))
    float PrimitiveSolidOpacity = 0.25f;
    /** Constant screen-pixel width, including distant collision edges. */
    UPROPERTY(EditAnywhere, Category="Kya|Diagnostics|Primitives", meta=(ClampMin="0.5", ClampMax="6"))
    float PrimitiveWireThickness = 1.5f;
    /** Bring coplanar wires slightly forward to avoid fighting with level surfaces. */
    UPROPERTY(EditAnywhere, Category="Kya|Diagnostics|Primitives", meta=(ClampMin="0", ClampMax="10"))
    float PrimitiveWireDepthBias = 0.1f;

    UPROPERTY(VisibleAnywhere, Transient, Category="Kya|Diagnostics|Primitives")
    TArray<TObjectPtr<UKyaPrimitiveComponent>> PrimitiveComponents;

    UPROPERTY(VisibleAnywhere, Transient, Category="Kya|Diagnostics|Primitives")
    TArray<FString> PrimitiveDiagnostics;

    void RefreshPrimitives();

    /** Human-readable list of visualizer limitations (for example missing zone geometry). */
    UPROPERTY(VisibleAnywhere, Category="Kya|Diagnostics|Visualizers")
    FString VisualizerLimitations;

    /**
     * Spawn selectable handles for waypoints. When enabled, the diagnostic waypoint
     * arrows are suppressed to avoid drawing two markers per record.
     */
    UPROPERTY(EditAnywhere, Category="Kya|Diagnostics|Selection")
    bool bSpawnWaypointHandles = false;

    /**
     * Spawn selectable handles for every point of every path, labelled with path and
     * point index. This makes spatial records discoverable without setting a numeric
     * preview path index; the spline guide still follows PreviewPathIndex.
     */
    UPROPERTY(EditAnywhere, Category="Kya|Diagnostics|Selection")
    bool bSpawnPathPointHandles = false;

    /**
     * Draw a guide for every path in the bank (linear, matching the game) and spawn
     * selectable points for all of them, without requiring Preview Path Index.
     */
    UPROPERTY(EditAnywhere, Category="Kya|Diagnostics|Paths")
    bool bShowAllPaths = false;

    /** Number of live path guides (one per visible path). */
    UFUNCTION(BlueprintCallable, Category="Kya|Diagnostics|Paths")
    int32 GetPathGuideCount() const { return PathGuides.Num(); }

    /** Spline guide for one path index, or null when that path is not shown. */
    UFUNCTION(BlueprintCallable, Category="Kya|Diagnostics|Paths")
    USplineComponent* GetPathGuide(int32 PathIndex) const;

    /** Editor-only Outliner group that owns the points of one path. */
    UFUNCTION(BlueprintCallable, Category="Kya|Diagnostics|Paths")
    AActor* GetPathGroup(int32 PathIndex) const;

    /** Number of live waypoint/path-point handles. */
    UFUNCTION(BlueprintCallable, Category="Kya|Diagnostics|Selection")
    int32 GetSpatialHandleCount() const { return SpatialHandles.Num(); }

    /** Waypoint handle by waypoint index, or null. */
    UFUNCTION(BlueprintCallable, Category="Kya|Diagnostics|Selection")
    AActor* GetWaypointHandle(int32 WaypointIndex) const;

    /** Path-point handle by path and point index, or null. */
    UFUNCTION(BlueprintCallable, Category="Kya|Diagnostics|Selection")
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
    UFUNCTION(BlueprintCallable, Category="Kya|Diagnostics|Selection")
    void SetGeneratedActorTemporarilyHidden(AActor* Actor, bool bInHidden);
    UFUNCTION(BlueprintCallable, Category="Kya|Diagnostics|Selection")
    bool IsGeneratedActorTemporarilyHidden(const AActor* Actor) const;
    /** Hide every generated representation except Actors, until isolation is cleared. */
    UFUNCTION(BlueprintCallable, Category="Kya|Diagnostics|Selection")
    void IsolateGeneratedActors(const TArray<AActor*>& Actors);
    UFUNCTION(BlueprintCallable, Category="Kya|Diagnostics|Selection")
    void ClearGeneratedActorIsolation();
    UFUNCTION(BlueprintCallable, Category="Kya|Diagnostics|Selection")
    bool IsGeneratedActorIsolated(const AActor* Actor) const;
    bool HasGeneratedActorIsolation() const { return bIsolationActive; }
    /** Reapply temporary visibility and selection-lock state after a build. */
    void ApplyGeneratedActorPresentation();

    /**
     * Pick the visible actor geometry a viewport click would hit: ray-casts every
     * rendered mesh section (all material strips) of the owned handles and returns the
     * nearest selectable hit. Locked/hidden/handle-less representations are skipped.
     */
    AKyaActorHandle* HitTestActorMesh(const FVector& WorldOrigin, const FVector& WorldDirection,
        float MaxDistance = 1.0e7f, float* OutDistance = nullptr) const;
    /** Highlight the full mesh of one generated actor and clear the others. */
    void SetGeneratedActorHighlighted(AKyaActorHandle* Handle, bool bHighlighted);
    /** Sync every handle's highlight with the current editor selection. */
    void RefreshGeneratedActorHighlights();
    /** Follow editor selection changes so the picked mesh stays highlighted. */
    void BindEditorSelection();
    void UnbindEditorSelection();
    void HandleEditorSelectionChanged(UObject* NewSelection);
#endif

    /** True when actor geometry belongs to the actor handles and must not enter the shared mesh. */
    bool ShouldActorHandlesRenderGeometry() const;
    bool ShouldShowActorIcon() const;
    /** Mesh sections captured for one bank actor during the last build, or null. */
    const TArray<FKyaActorMeshSection>* FindActorMeshSections(int32 ActorIndex) const;
    /** Refresh after an edit to a bank this preview displays; rebuilds once. */
    void HandleBankEdited(UKyaLevelBank* InBank);
    /** Targeted: one actor record changed (properties/transform), no level decode. */
    void HandleActorEdited(UKyaLevelBank* InBank, int32 ActorIndex, bool bDeferred);
    /** Targeted: one waypoint changed; its handle, guide and visualizers only. */
    void HandleWaypointEdited(UKyaLevelBank* InBank, int32 WaypointIndex);
    /** Targeted: one path point changed; its handle, guide and visualizers only. */
    void HandlePathPointEdited(UKyaLevelBank* InBank, int32 PathIndex, int32 PointIndex);
    /** Targeted: one light record changed; refresh lighting and visualizers. */
    void HandleLightEdited(UKyaLevelBank* InBank, bool bDeferred);
    /** Targeted: main camera changed; refresh scene visualizers only. */
    void HandleSceneCameraEdited(UKyaLevelBank* InBank);

    /**
     * Spawn selectable handles for static world-geometry hierarchies (the smallest
     * practical unit: a cluster node's strip group in a sector or common mesh).
     */
    UPROPERTY(EditAnywhere, Category="Kya|Diagnostics|Selection")
    bool bSpawnHierarchyHandles = false;

    /** Hierarchy keys hidden from the preview by a hierarchy handle. */
    UPROPERTY(EditAnywhere, Category="Kya|Diagnostics|Selection")
    TSet<FString> HiddenHierarchies;

    /** Hide or show one world-geometry hierarchy and rebuild the preview. */
    void SetHierarchyVisible(UKyaLevelBank* InBank, int32 HierarchyOffset, bool bVisible);

    /** Rebuild the preview after an external edit to the bank asset (such as a handle edit). */
    void RebuildFromBank();

    /** Number of completed preview builds; lets tests confirm that edits refresh the mesh. */
    UPROPERTY(VisibleAnywhere, Category="Kya|Diagnostics")
    int64 BuildCount = 0;

    /** Number of path-point markers for the previewed path. */
    UFUNCTION(BlueprintCallable, Category="Kya|Diagnostics|Paths")
    int32 GetPathMarkerCount() const { return PathMarkers.Num(); }

    /** Path-point marker by index, or null. */
    UFUNCTION(BlueprintCallable, Category="Kya|Diagnostics|Paths")
    UKyaPathPointComponent* GetPathMarker(int32 MarkerIndex) const;

    /** Number of live selectable actor handles. */
    UFUNCTION(BlueprintCallable, Category="Kya|Diagnostics|Selection")
    int32 GetActorHandleCount() const { return ActorHandles.Num(); }

#if WITH_EDITOR
    /** Number of decoded world-geometry hierarchies available for selection. */
    int32 GetHierarchySelectionCount() const { return HierarchySelectionInfo.Num(); }
    /** Number of live hierarchy handles. */
    int32 GetHierarchyHandleCount() const { return HierarchyHandles.Num(); }
    /** Live hierarchy handle by index, or null. */
    AKyaHierarchyHandle* GetHierarchyHandle(int32 HandleIndex) const
    {
        return HierarchyHandles.IsValidIndex(HandleIndex) ? HierarchyHandles[HandleIndex].Get() : nullptr;
    }
    /** Scene visualizer by key ("light:<bank>:<index>", "camera:<bank>", "link:...", "trigger:..."). */
    AKyaSceneVisualizer* GetSceneVisualizer(const FString& Key) const;
    int32 GetSceneVisualizerCount() const { return SceneVisualizers.Num(); }
    /** Refresh light/camera/link/trigger visualizers after a build. */
    void RefreshSceneVisualizers();
#endif

    /** Handle for a bank actor index, or null. */
    UFUNCTION(BlueprintCallable, Category="Kya|Diagnostics|Selection")
    AActor* GetActorHandle(int32 ActorIndex) const;

    UPROPERTY(VisibleAnywhere, Category="Kya|Authoring")
    TObjectPtr<UProceduralMeshComponent> Mesh;

private:
    // Session-only, but transactional so undo/redo also preserves the restore point.
    UPROPERTY(Transient)
    TSet<int32> SavedHiddenSectorIds;
    UPROPERTY(Transient)
    bool bHasSavedSectorVisibility = false;

    void UpdatePreviewCamera(const FVector& Position, const FVector& Forward, const FVector& Up,
        float HorizontalFovDegrees, int32 Width, int32 Height);
    void UpdateClusterVisibility(const FVector& Position, const FVector& Forward, const FVector& Right,
        const FVector& Up, double TanX, double TanY);
    struct FClusterVisibilityRange
    {
        int32 Section = 0;
        int32 Start = 0;
        int32 Count = 0;
        int32 LodIndex = 0;
        FVector4f ObjectSphere = FVector4f(0, 0, 0, 0);
        FVector4f StripSphere = FVector4f(0, 0, 0, 0);
        FVector4f LodSphere = FVector4f(0, 0, 0, 0);
        double MinLodBiasSquared = 0;
        double MaxLodBiasSquared = DBL_MAX;
        bool bVisible = true;
    };
    TArray<FClusterVisibilityRange> ClusterVisibilityRanges;
    TMap<int32, TArray<uint32>> ClusterBaseIndices;
    struct FSectorGeometryRange
    {
        int32 SectorId = -1;
        int32 Section = 0;
        int32 Start = 0;
        int32 Count = 0;
    };
    TArray<FSectorGeometryRange> SectorGeometryRanges;
    TSet<int32> AppliedHiddenSectorIds;
    void RefreshSectorVisibility();
    void ApplyMeshVisibility(const TSet<int32>& Sections);
    float ActiveClipDistance = 500;
    struct FBackgroundFlare
    {
        FVector Position = FVector::ZeroVector;
        int32 MaterialIndex = INDEX_NONE;
    };
    TArray<FBackgroundFlare> BackgroundFlares;
    bool bExplicitPreviewCamera = false;
    /** Number of handles shown only because their sector bank is not loaded. */
    int32 AuthoringOnlyHandleCount = 0;
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

    struct FScrollingMaterial
    {
        TWeakObjectPtr<UMaterialInstanceDynamic> Material;
        KyaG2D::FTextureScroll Scroll;
        FVector2f InitialOffset = FVector2f::ZeroVector;
    };
    TArray<FScrollingMaterial> ScrollingMaterials;
    double TextureAnimationRemainder = 0;

    UPROPERTY(Transient)
    TObjectPtr<UMaterial> GeneratedPreviewMaterial;

    UPROPERTY(Transient)
    TObjectPtr<UMaterial> GeneratedMaskedPreviewMaterial;

    UPROPERTY(Transient)
    TObjectPtr<UMaterial> GeneratedOpaqueCoreMaterial;
    UPROPERTY(Transient)
    TObjectPtr<UMaterial> GeneratedAdditivePreviewMaterial;

    UPROPERTY(Transient)
    TObjectPtr<UMaterial> GeneratedSubtractivePreviewMaterial;
    UPROPERTY(Transient)
    TObjectPtr<UMaterial> GeneratedTranslucentPreviewMaterial;

    UPROPERTY(Transient)
    TObjectPtr<UMaterial> GeneratedUntexturedMaterial;

    UPROPERTY(Transient)
    TArray<TObjectPtr<UKyaWaypointComponent>> WaypointMarkers;

    UPROPERTY(Transient)
    TObjectPtr<UKyaLevelBank> WaypointMarkerBank;

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
        /** Preview-local bounds of the captured hierarchy geometry (cm). */
        FBox Bounds = FBox(ForceInit);
        /** Captured material sections, one per material used by this hierarchy. */
        TArray<FKyaActorMeshSection> Sections;
        /** Material names (or hash fallbacks) for the Details panel. */
        TArray<FString> Materials;
        /** Raw material hashes, parallel to Materials. */
        TArray<uint64> MaterialHashes;
    };
    FString MakeHierarchyKey(const UKyaLevelBank* InBank, int32 HierarchyOffset) const;
    TMap<FString, FHierarchySelectionInfo> HierarchySelectionInfo;
    UPROPERTY(Transient)
    TArray<TObjectPtr<AKyaHierarchyHandle>> HierarchyHandles;
    UPROPERTY(Transient)
    TMap<int32, TObjectPtr<AKyaActorHandle>> ActorHandles;
    /** Per-sector background bounds in preview-local centimetres, from the last build. */
    TMap<int32, FBox> SectorBackgroundBounds;
    /** Editor-only sector grouping actors for the Outliner (sector 0 = global actors). */
    UPROPERTY(Transient)
    TMap<int32, TObjectPtr<AKyaActorGroup>> ActorSectorGroups;
    /** Editor-only waypoint/path-point handles, keyed by "W:<index>" or "P:<path>:<point>". */
    UPROPERTY(Transient)
    TMap<FString, TObjectPtr<AKyaSpatialRecordHandle>> SpatialHandles;
    /** Linear spline guides for all-path mode, keyed by path index. */
    UPROPERTY(Transient)
    TMap<int32, TObjectPtr<USplineComponent>> PathGuides;
    /** Editor-only Outliner groups that own each path's points. */
    UPROPERTY(Transient)
    TMap<int32, TObjectPtr<AKyaActorGroup>> PathGroups;
    /** Editor-only light/camera/link/trigger visualizers, keyed by kind/source. */
    UPROPERTY(Transient)
    TMap<FString, TObjectPtr<AKyaSceneVisualizer>> SceneVisualizers;
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
    /** Source revision of each captured actor section (bank generation + actor render inputs). */
    TMap<int32, uint64> ActorSectionRevisions;
    TWeakObjectPtr<UKyaLevelBank> ActorSectionBank;
    int32 ActorSectionGeneration = -1;
    uint32 ActorLightingRevision = 0;
    /** Actor positions used for the cached captured lighting. */
    TMap<int32, FVector> ActorCapturedPositions;
    bool bHasGroundLightData = false;
    /** Data-only actor strip cache filled by a worker thread, keyed by bank revision. */
    TSharedPtr<struct FKyaActorDecodeCache, ESPMode::ThreadSafe> ActorDecodeCache;
    mutable FCriticalSection ActorDecodeCacheLock;
    TSharedPtr<FThreadSafeCounter> DecodeCancelToken;
    FEvent* DecodeCompletionEvent = nullptr;
    TArray<FEvent*> DecodeEventHistory;
    FThreadSafeCounter AsyncDecodeCompletions;
    FThreadSafeCounter StaleDecodeDiscards;
    FThreadSafeCounter CachedActorDecodes;
    FThreadSafeCounter SynchronousActorDecodes;
    /** Background/sector/cluster strip decodes performed by this preview. */
    int32 GeometryDecodes = 0;
    int32 CachedGeometryDecodes = 0;
    /** Serialized strips keyed by bank, source kind (0 background, 1 cluster) and entry. */
    struct FGeometryDecodeCacheKey
    {
        const UKyaLevelBank* Bank = nullptr;
        int32 Kind = 0;
        int32 EntryIndex = INDEX_NONE;
        bool operator==(const FGeometryDecodeCacheKey& Other) const
        {
            return Bank == Other.Bank && Kind == Other.Kind && EntryIndex == Other.EntryIndex;
        }
        friend uint32 GetTypeHash(const FGeometryDecodeCacheKey& Key)
        {
            return HashCombine(HashCombine(::GetTypeHash(Key.Bank), Key.Kind), Key.EntryIndex);
        }
    };
    TMap<FGeometryDecodeCacheKey, TSharedPtr<TArray<FKyaG3DDecodedStrip>>> GeometryDecodeCache;
    TMap<const UKyaLevelBank*, int32> GeometryDecodeCacheGeneration;
    /**
     * Return cached decoded strips for this bank/entry, decoding only on a miss or a
     * bank revision change. OutStrips points at the cache and must be consumed before
     * another cache insert (map rehash).
     */
    bool GetDecodedGeometry(const UKyaLevelBank* Source, int32 Kind, int32 EntryIndex,
        TConstArrayView<uint8> Data, TArray<FKyaG3DDecodedStrip>& Scratch,
        const TArray<FKyaG3DDecodedStrip>*& OutStrips);
    /** Concatenate one decode per LOD; cached as a single entry. */
    bool GetDecodedClusterGeometry(const UKyaLevelBank* Source, int32 EntryIndex,
        TConstArrayView<uint8> Data, int32 MaxLods, TArray<FKyaG3DDecodedStrip>& Scratch,
        const TArray<FKyaG3DDecodedStrip>*& OutStrips);
    void ResetGeometryDecodeCache(const UKyaLevelBank* Source);
    /** Editor selection-changed subscription used for full-mesh highlights. */
    FDelegateHandle EditorSelectionChangedHandle;
    bool bCancelRebuildRequested = false;
    void StartAsyncActorDecode();
    /** Recycle decode events; only waits when bWait (shutdown), never in the editor. */
    void DrainAsyncDecodeEvents(bool bWait);
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
