#pragma once

#include "CoreMinimal.h"

class AKyaLevelPreview;
class UKyaLevelBank;

/** Per-bank validation report used by the Validate/Export editor commands. */
struct FKyaBankValidationReport
{
    UKyaLevelBank* Bank = nullptr;
    FString AssetPath;
    bool bExportSupported = false;
    bool bBytesBuilt = false;
    FString UnsupportedReason;
    TArray<FString> Warnings;
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

/**
 * Discover sibling SECT<digits>.BNK files next to the preview's main bank, reuse
 * already-imported assets matched by source provenance and import the rest into a
 * level-specific folder. Existing preview associations and bank edits are kept.
 */
FKyaSectorDiscoveryResult KyaDiscoverSectors(AKyaLevelPreview* Preview, bool bImportMissing);

/** Save the packages that contain the given banks (and their previews' maps). */
bool KyaSaveLevelBanks(const TArray<UKyaLevelBank*>& Banks, FString& OutError);

/** Export the banks to <ProjectSaved>/KyaExport/<level>/ and log per-file outcomes. */
bool KyaExportLevelBanks(const TArray<UKyaLevelBank*>& Banks, const FString& DestinationDirectory,
    TArray<FString>& OutMessages);
