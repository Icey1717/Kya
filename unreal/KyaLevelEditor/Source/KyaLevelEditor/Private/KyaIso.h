#pragma once

#include "CoreMinimal.h"
#include "KyaDisc.h"

/** Read contiguous files from a cooked, 2048-byte-sector ISO9660 image. */
namespace KyaIso
{
    struct FReplacement
    {
        FString Path;
        FString ExpectedHash;
        TArray<uint8> Bytes;
    };
    bool ReadDirectory(const FString& Filename, TArray<FKyaDiscFile>& Files, FString& Error);
    bool Extract(const FString& Filename, const FString& Path, TArray<uint8>& Bytes, FString& Error);
    /** Stage and verify every replacement before publishing; retain the original image as Backup. */
    bool ReplaceFiles(const FString& Filename, const TArray<FReplacement>& Replacements, FString& Backup, FString& Error,
        const TFunction<void(const FString&)>& Output = {});
}
