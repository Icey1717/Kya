#pragma once

#include "CoreMinimal.h"

class AKyaLevelPreview;
class AKyaActorHandle;
class UKyaLevelBank;
class UKyaLevelSet;

/** One validation finding that can be linked to an actor in the Message Log. */
struct FKyaValidationIssue
{
    int32 ActorIndex = INDEX_NONE;
    FString Field;
    FString Message;
    bool bBlocking = false;
};

/** Per-bank validation report used by the Validate/Export editor commands. */
struct FKyaBankValidationReport
{
    UKyaLevelBank* Bank = nullptr;
    FString AssetPath;
    bool bExportSupported = false;
    bool bBytesBuilt = false;
    FString UnsupportedReason;
    TArray<FString> Warnings;
    TArray<FKyaValidationIssue> Issues;
    FString Summary() const;
};

/** Banks referenced by the given previews (main bank and sector banks, de-duplicated). */
TArray<UKyaLevelBank*> KyaCollectLevelBanks(const TArray<AKyaLevelPreview*>& Previews);
/** Selected level previews in the current editor world. */
TArray<AKyaLevelPreview*> KyaGetSelectedPreviews();
/** Selected previews, or every preview in the editor world when none are selected. */
TArray<AKyaLevelPreview*> KyaGetTargetPreviews();

/** Validate one bank without writing anything. */
FKyaBankValidationReport KyaValidateBank(UKyaLevelBank* Bank);
/** Validate every bank of a level set and log one report per bank. */
TArray<FKyaBankValidationReport> KyaValidateLevelBanks(const TArray<UKyaLevelBank*>& Banks);

struct FKyaSectorDiscoveryResult
{
    int32 Reused = 0;
    int32 Imported = 0;
    int32 Ambiguous = 0;
    TArray<int32> MissingIds;
    TArray<FString> Messages;
    FString Summary() const;
};

/** Create (or reuse) a UKyaLevelSet for the preview's bank and associations. */
UKyaLevelSet* KyaCreateLevelSetFromPreview(AKyaLevelPreview* Preview, FString& OutError);

/**
 * Refresh a level set from a preview's current associations. Manual overrides keep
 * their sector ID and bank; a bank already mapped manually is never also inserted
 * under its filename-derived ID.
 */
void KyaSyncLevelSetFromPreview(UKyaLevelSet& LevelSet, AKyaLevelPreview& Preview, const FString& SourceDirectory);

/**
 * Discover sibling SECT<digits>.BNK files next to the preview's main bank, reuse
 * already-imported assets matched by source provenance and import the rest into a
 * level-specific folder. Existing preview associations and bank edits are kept.
 */
FKyaSectorDiscoveryResult KyaDiscoverSectors(AKyaLevelPreview* Preview, bool bImportMissing);

/** Save the packages that contain the given banks (and their previews' maps). */
bool KyaSaveLevelBanks(const TArray<UKyaLevelBank*>& Banks, FString& OutError);

/**
 * Toggle editor-only authoring discovery (unloaded-sector actors and distance-culled
 * geometry remain visible). Disabling it restores game-like culling; a locked
 * comparison/capture camera always uses native culling. Enabling it also clears the
 * capture-camera lock so authoring can be resumed.
 */
void KyaSetAuthoringVisibility(bool bEnabled);
/** Clear the comparison/capture camera lock on every target preview. */
void KyaClearPreviewCamera();

/**
 * Select the actor whose rendered geometry the ray hits (the viewport click path):
 * hit-tests every mesh strip, selects the owning handle and highlights its full mesh.
 * Returns the handle, or null when nothing selectable was hit.
 */
AKyaActorHandle* KyaSelectRenderedActorAt(AKyaLevelPreview* Preview, const FVector& WorldOrigin,
    const FVector& WorldDirection);

/**
 * Nearest visible actor geometry along a ray across several previews. Fills
 * OutPreview with the owning preview so selection and highlight run on the right one.
 */
AKyaActorHandle* KyaHitTestRenderedActorAcrossPreviews(const TArray<AKyaLevelPreview*>& Previews,
    const FVector& WorldOrigin, const FVector& WorldDirection, AKyaLevelPreview** OutPreview = nullptr);

/**
 * Select the actor under the level viewport cursor using the same mesh hit-test,
 * i.e. the interactive click entry point. Returns null when nothing is under it.
 */
AKyaActorHandle* KyaSelectRenderedActorAtCursor(FString* OutError = nullptr);

/** Export the banks to <ProjectSaved>/KyaExport/<level>/ and log per-file outcomes. */
bool KyaExportLevelBanks(const TArray<UKyaLevelBank*>& Banks, const FString& DestinationDirectory,
    TArray<FString>& OutMessages);

/** True when the actor is a generated Kya editing representation (handle/group). */
bool KyaIsGeneratedActor(const AActor* Actor);
/** True when any actor in the list is a generated representation. */
bool KyaContainsGeneratedActor(const TArray<AActor*>& Actors);
/** True when the current editor selection contains a generated representation. */
bool KyaSelectionContainsGeneratedActor();
/**
 * Destroy generated actors that no preview owns (duplicate/paste/alt-drag leftovers)
 * and generated actors whose preview is gone. Returns the number of orphans removed.
 */
int32 KyaDestroyStrayGeneratedActors(UWorld* World);

// --- Generated representation presentation (editor-only, never game state) --------

/** Temporarily hide or show the selected generated representations. */
void KyaSetSelectedGeneratedActorsHidden(bool bHidden, int32& OutAffected);
/** Isolate the selected generated representations; everything else is hidden. */
void KyaIsolateSelectedGeneratedActors(int32& OutAffected);
/** Clear temporary hiding and isolation on every level preview. */
void KyaClearGeneratedActorIsolation();
/** Lock or unlock viewport picking for generated representations on every preview. */
void KyaSetGeneratedActorSelectionLocked(bool bLocked);
