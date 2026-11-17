#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "KyaDisc.generated.h"

class UKyaLevelSet;
class UWorld;

/** An exact source identity within an imported disc; offsets are informational only. */
USTRUCT(BlueprintType)
struct KYALEVEL_API FKyaDiscFile
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, Category="Disc")
    FString Path;

    UPROPERTY(VisibleAnywhere, Category="Disc")
    int64 Size = 0;

    UPROPERTY(VisibleAnywhere, Category="Disc")
    int64 ImportOffset = 0;

    /** Directory record location, refreshed by the native reader before writing. */
    int64 DirectoryRecordOffset = 0;
};

USTRUCT(BlueprintType)
struct KYALEVEL_API FKyaDiscLevel
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, Category="Disc")
    FString SourceDirectory;

    UPROPERTY(VisibleAnywhere, Category="Disc")
    TObjectPtr<UKyaLevelSet> LevelSet;

    UPROPERTY(VisibleAnywhere, Category="Disc")
    TSoftObjectPtr<UWorld> PreviewMap;
};

/** Root asset for a game disc and the editable levels imported from it. */
UCLASS(BlueprintType)
class KYALEVEL_API UKyaDisc : public UDataAsset
{
    GENERATED_BODY()
public:
    UPROPERTY(VisibleAnywhere, Category="Disc")
    FString SourceFilename;

    UPROPERTY(VisibleAnywhere, Category="Disc")
    TArray<FKyaDiscFile> Files;

    UPROPERTY(VisibleAnywhere, Category="Disc")
    TArray<FKyaDiscLevel> Levels;
};
