#pragma once

#include "CoreMinimal.h"
#include "Actors/KyaScenarioCondition.h"

struct FKyaConditionedOffsets;

/**
 * Shared catch-by-trap behaviour payload (ids 0x07/0x0A, behaviour 2).
 *
 * The payload is a conditioned-operation block array; see
 * Actors/KyaScenarioCondition.h for the shared reader.
 */
struct KYALEVEL_API FKyaCatchByTrapOffsets
{
    TArray<FKyaConditionedOffsets> Blocks;
    int64 Tail = 0;
};

KYALEVEL_API bool KyaReadCatchByTrapOffsets(TConstArrayView<uint8> Data, int64 Start,
    int64 Size, FKyaCatchByTrapOffsets& Out);
