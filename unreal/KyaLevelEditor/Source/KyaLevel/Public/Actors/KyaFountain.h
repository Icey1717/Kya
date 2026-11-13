#pragma once

#include "CoreMinimal.h"

struct FKyaActorPlacement;
struct FKyaActorBehaviour;

/**
 * CActorBonusFountain (class 0x52) serialized codec.
 *
 * CActorBonusFountain::Create calls CActor::Create and then
 * CAddOnGenerator::Create. BuildBehaviour only adds id 2
 * (CBehaviourBonusFountainStand), which has no Create override and therefore no
 * payload; ids 0/1 are inherited.
 */
struct KYALEVEL_API FKyaFountainOffsets
{
    int64 AddOn = 0, SubObjMode = 0, SubObjFloats = 0, Tail = 0;
    int32 SubObjModeValue = 0;
};

KYALEVEL_API bool KyaReadFountainOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size,
    FKyaFountainOffsets& Out);
KYALEVEL_API bool KyaReadFountainActor(FKyaActorPlacement& Out, TConstArrayView<uint8> Data,
    int64 Start, int64 Size);
KYALEVEL_API bool KyaWriteFountainActor(const FKyaActorPlacement& Actor, TArray<uint8>& Data,
    int64 Start, int64 Size);
