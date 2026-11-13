#pragma once

#include "CoreMinimal.h"

class UKyaLevelBank;
struct FKyaActorPlacement;

/** Result of one structural actor operation. */
struct KYALEVELEDITOR_API FKyaStructuralWriteResult
{
    bool bSuccess = false;
    /** New actor index for create/duplicate, or the deleted index. */
    int32 ActorIndex = INDEX_NONE;
    FString Error;
};

/**
 * True when the bank's actor stream can be rebuilt and every actor class has an
 * allocation entry. This checks the byte layout, not just the typed arrays.
 */
KYALEVELEDITOR_API bool KyaBankSupportsStructuralEdit(const UKyaLevelBank& Bank, FString& OutReason);

/** Rebuild actor chunks when their scenario condition counts have changed. */
KYALEVELEDITOR_API bool KyaResizeActorConditions(UKyaLevelBank& Bank, bool& bChanged, FString& OutError);

/** Append a copy of SourceActorIndex and rebuild the bank's actor stream. */
KYALEVELEDITOR_API FKyaStructuralWriteResult KyaDuplicateActorRecord(UKyaLevelBank& Bank, int32 SourceActorIndex);

/** Append a new actor based on TemplateActorIndex (same record layout, new identity). */
KYALEVELEDITOR_API FKyaStructuralWriteResult KyaCreateActorRecord(UKyaLevelBank& Bank, int32 TemplateActorIndex);

/**
 * True when ActorIndex can be removed without leaving a typed actor reference to it.
 * Deleting a middle record shifts later actor indices and remaps typed references.
 */
KYALEVELEDITOR_API bool KyaCanDeleteActorRecord(const UKyaLevelBank& Bank, int32 ActorIndex, FString& OutReason);

/** Remove ActorIndex, remap typed actor references and rebuild the bank's actor stream. */
KYALEVELEDITOR_API FKyaStructuralWriteResult KyaDeleteActorRecord(UKyaLevelBank& Bank, int32 ActorIndex);

/**
 * Remap int/int64 fields whose property name contains "Actor" (including nested
 * structs and arrays). Unmapped values are left untouched; a value that maps to a
 * removed record is appended to OutDangling when supplied.
 */
KYALEVELEDITOR_API bool KyaRemapActorReferences(FKyaActorPlacement& Placement,
    const TMap<int32, int32>& Remap, TArray<FString>* OutDangling = nullptr);
