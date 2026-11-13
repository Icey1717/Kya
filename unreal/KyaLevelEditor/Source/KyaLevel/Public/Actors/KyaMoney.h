#pragma once

#include "CoreMinimal.h"

struct FKyaActorPlacement;
struct FKyaActorBehaviour;

/**
 * CActorMoney (class 0x32) serialized codec.
 *
 * CActorMoney::Create calls CActorMovable::Create (weight), then reads the money
 * value, sound reference and material id. BuildBehaviour maps 2 to
 * CBehaviourMoneyFlock (a path-follow index) and 3 to CBehaviourMoneyAddOn
 * (empty Create); ids 0/1 are inherited.
 */
struct KYALEVEL_API FKyaMoneyOffsets
{
    int64 WeightB = 0, MoneyValue = 0, Sound = 0, Material = 0, Tail = 0;
};

KYALEVEL_API bool KyaReadMoneyOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size,
    FKyaMoneyOffsets& Out);
KYALEVEL_API bool KyaReadMoneyActor(FKyaActorPlacement& Out, TConstArrayView<uint8> Data,
    int64 Start, int64 Size);
KYALEVEL_API bool KyaWriteMoneyActor(const FKyaActorPlacement& Actor, TArray<uint8>& Data,
    int64 Start, int64 Size);

struct KYALEVEL_API FKyaMoneyFlockOffsets
{
    int64 PathIndex = 0, Tail = 0;
};

KYALEVEL_API bool KyaReadMoneyFlockOffsets(TConstArrayView<uint8> Data, int64 Start,
    int64 Size, FKyaMoneyFlockOffsets& Out);
KYALEVEL_API bool KyaReadMoneyBehaviour(FKyaActorBehaviour& Out, int32 Id,
    TConstArrayView<uint8> Data, int64 Start, int32 Size);
KYALEVEL_API bool KyaWriteMoneyBehaviour(const FKyaActorBehaviour& Behaviour,
    TArray<uint8>& Data, int64 Start, int32 Size);
