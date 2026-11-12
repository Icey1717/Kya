#pragma once

#include "CoreMinimal.h"

class UKyaLevelBank;
struct FKyaActorPlacement;

/** Structural authoring operations that require a rebuilt bank layout. */
enum class EKyaStructuralEditKind : uint8
{
    CreateActor,
    DeleteActor,
    DuplicateActor,
    RenameActor,
    ResizeArray,
    ChangeClass
};

/**
 * Analysis of a proposed structural edit.
 *
 * Create, duplicate and delete are implemented by the editor's structural writer:
 * it rebuilds the scene actor stream, updates class allocation counts and remaps
 * typed actor references. Unknown raw payload bytes are preserved unchanged.
 * Class replacement, array resizing and variable-length names remain blocked.
 */
struct KYALEVEL_API FKyaStructuralEditAnalysis
{
    EKyaStructuralEditKind Kind = EKyaStructuralEditKind::CreateActor;
    bool bWriterSupported = false;
    TArray<FString> BlockingReasons;
    /** Allocation-count delta per class id. */
    TMap<int64, int32> ClassAllocationDelta;
    /** Old actor index -> new actor index; -1 means removed. */
    TMap<int32, int32> ActorIndexRemap;
    /** Actor/waypoint references that point at a removed actor. */
    int32 ReferencesToRemovedActor = 0;
    /** Bank entries unrelated to the structural actor stream. */
    int32 PreservedBankEntries = 0;
    bool IsBlocked() const { return !bWriterSupported; }
};

/**
 * Analyse (without applying) one structural edit. Always fills the allocation,
 * remapping and preservation data; a future writer can enable the operation when
 * every blocking reason has been implemented and tested.
 */
KYALEVEL_API bool AnalyzeKyaStructuralEdit(const UKyaLevelBank& Bank, EKyaStructuralEditKind Kind,
    int32 ActorIndex, FKyaStructuralEditAnalysis& OutAnalysis);

/** Human-readable reason why a structural kind is disabled. */
KYALEVEL_API FString GetKyaStructuralEditBlocker(EKyaStructuralEditKind Kind);

/**
 * Reject edits that change the serialized record layout before they mutate the bank:
 * class replacement, variable-length names outside the fixed slot, and fixed-size
 * array resizing. Returns true when the candidate is a supported in-place edit.
 */
KYALEVEL_API bool ValidateKyaActorStructuralEdit(const FKyaActorPlacement& Current,
    const FKyaActorPlacement& Candidate, FString& OutError);
