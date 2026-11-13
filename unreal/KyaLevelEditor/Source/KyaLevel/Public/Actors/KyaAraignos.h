#pragma once

#include "CoreMinimal.h"

struct FKyaActorPlacement;

/**
 * CActorAraignos (class 0x21) serialized codec.
 *
 * CActorAraignos::Create calls CActorMovable::Create, reads a path-follow index,
 * three floats, three sound ids and a CAddOnGenerator. BuildBehaviour only adds
 * id 2 (CBhvDefault), whose Create is empty; ids 0/1 are inherited.
 */
struct KYALEVEL_API FKyaAraignosOffsets
{
    int64 WeightB = 0;
    int64 PathFollow = 0;
    int64 Floats = 0;
    int64 Sounds = 0;
    int64 AddOn = 0;
    int64 SubObjMode = 0;
    int64 SubObjFloats = 0;
    int64 Tail = 0;
    int32 SubObjModeValue = 0;
};

KYALEVEL_API bool KyaReadAraignosOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size,
    FKyaAraignosOffsets& Out);
KYALEVEL_API bool KyaReadAraignosActor(FKyaActorPlacement& Out, TConstArrayView<uint8> Data,
    int64 Start, int64 Size);
KYALEVEL_API bool KyaWriteAraignosActor(const FKyaActorPlacement& Actor, TArray<uint8>& Data,
    int64 Start, int64 Size);
