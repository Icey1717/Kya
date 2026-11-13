#pragma once

#include "CoreMinimal.h"

struct FKyaActorPlacement;
struct FKyaActorBehaviour;

/**
 * CActorRune (class 0x51) serialized codec.
 *
 * CActorRune::Create calls CActor::Create and reads a cinematic id, trigger
 * distance, scenario variable id, fx id and two material ids. BuildBehaviour
 * only adds id 2 (CBehaviourRune), whose Create is empty; ids 0/1 are inherited.
 */
struct KYALEVEL_API FKyaRuneOffsets
{
    int64 Cinematic = 0, TriggerDistance = 0, ScenVar = 0;
    int64 Fx = 0, MaterialA = 0, MaterialB = 0, Tail = 0;
};

KYALEVEL_API bool KyaReadRuneOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size,
    FKyaRuneOffsets& Out);
KYALEVEL_API bool KyaReadRuneActor(FKyaActorPlacement& Out, TConstArrayView<uint8> Data,
    int64 Start, int64 Size);
KYALEVEL_API bool KyaWriteRuneActor(const FKyaActorPlacement& Actor, TArray<uint8>& Data,
    int64 Start, int64 Size);
