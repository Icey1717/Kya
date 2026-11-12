#pragma once

#include "CoreMinimal.h"

class UKyaLevelBank;
struct FKyaBankExportDiagnostics;

/** Sector identity comes from the native mesh/texture entries, not the asset name. */
bool KyaSectorBankSupportsExport(const UKyaLevelBank& Bank, FString& OutReason);
bool KyaBuildSectorBankExportBytes(const UKyaLevelBank& Bank, TArray<uint8>& OutBytes,
    FKyaBankExportDiagnostics& Diagnostics);

/** Shared fixed-size THGL writer for main banks and sector banks. */
bool KyaPatchBankLights(const UKyaLevelBank& Bank, TArray<uint8>& Bytes,
    FKyaBankExportDiagnostics& Diagnostics);
