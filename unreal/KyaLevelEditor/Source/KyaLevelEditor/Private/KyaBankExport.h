#pragma once

#include "CoreMinimal.h"

class UKyaLevelBank;

/** Errors and warnings produced while building export bytes for one bank. */
struct FKyaBankExportDiagnostics
{
    TArray<FString> Errors;
    TArray<FString> Warnings;
    bool HasErrors() const { return !Errors.IsEmpty(); }
    FString Summary() const;
};

/**
 * Build the bytes the bank exporter would write, without touching the filesystem.
 * Used by preflight, Save/Export commands and round-trip tests.
 */
KYALEVELEDITOR_API bool KyaBuildBankExportBytes(const UKyaLevelBank* Bank, TArray<uint8>& OutBytes,
    FKyaBankExportDiagnostics& Diagnostics);

struct FKyaBankExportRequest
{
    UKyaLevelBank* Bank = nullptr;
    FString DestinationFilename;
};

struct FKyaBankExportOutcome
{
    UKyaLevelBank* Bank = nullptr;
    FString DestinationFilename;
    bool bValidated = false;
    bool bPublished = false;
    TArray<FString> Errors;
    TArray<FString> Warnings;
};

/**
 * Validate every bank first, stage the validated outputs, then publish them.
 *
 * Nothing at a destination path is replaced unless its own staging write succeeded,
 * and a failed validation never writes or deletes an existing file. Implements the
 * per-file outcome reporting the editor commands and tests use.
 */
KYALEVELEDITOR_API TArray<FKyaBankExportOutcome> KyaWriteValidatedExportSet(
    const TArray<FKyaBankExportRequest>& Requests);

/** True when the bank has a round-trip supported scene/actor layout for the writer. */
KYALEVELEDITOR_API bool KyaBankSupportsExport(const UKyaLevelBank* Bank, FString& OutReason);
