#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "KyaLevelSet.generated.h"

class UKyaLevelBank;

/** One sector bank association in a level set. */
USTRUCT(BlueprintType)
struct KYALEVEL_API FKyaLevelSetSector
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, Category="Sector")
    int32 SectorId = INDEX_NONE;

    UPROPERTY(EditAnywhere, Category="Sector")
    TObjectPtr<UKyaLevelBank> Bank;

    /** Source file recorded when the association was discovered or added. */
    UPROPERTY(VisibleAnywhere, Category="Sector")
    FString SourceFilename;

    /**
     * Manual association. Refresh Sector Discovery keeps these entries instead of
     * replacing them with whatever the source directory currently contains.
     */
    UPROPERTY(EditAnywhere, Category="Sector")
    bool bManualOverride = false;
};

/**
 * Persisted descriptor for one playable Kya level: the main bank, its sector bank
 * associations, the source root they were discovered from and a revision counter.
 *
 * These associations are editor metadata, not game data; bank bytes are unchanged.
 */
UCLASS(BlueprintType)
class KYALEVEL_API UKyaLevelSet : public UDataAsset
{
    GENERATED_BODY()
public:
    static constexpr int32 CurrentDescriptorVersion = 1;

    UPROPERTY(VisibleAnywhere, Category="Kya")
    int32 DescriptorVersion = CurrentDescriptorVersion;

    UPROPERTY(EditAnywhere, Category="Kya")
    TObjectPtr<UKyaLevelBank> MainBank;

    UPROPERTY(EditAnywhere, Category="Kya")
    TArray<FKyaLevelSetSector> Sectors;

    /** Directory the sector files were discovered in. */
    UPROPERTY(EditAnywhere, Category="Kya")
    FString SourceRoot;

    /** Incremented by discovery import/association refresh. */
    UPROPERTY(VisibleAnywhere, Category="Kya")
    int32 ImportRevision = 0;

    UKyaLevelBank* FindSector(int32 SectorId) const;
    /** Add or update a sector association; manual entries are preserved. */
    void SetSector(int32 SectorId, UKyaLevelBank* Bank, const FString& InSourceFilename, bool bManual);
    /** Drop associations whose bank no longer exists. */
    void RemoveInvalidAssociations();
};
