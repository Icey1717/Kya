#pragma once

#include "CoreMinimal.h"

struct FKyaActorPlacement;
struct FKyaActorBehaviour;

/**
 * CActorFruit (class 0x16) serialized codec.
 *
 * CActorFruit::Create calls CActorAutonomous::Create (weight/life/gravity/
 * dynamic-extension prefix), reads field 0x360, then one S_NTF_SWITCH record.
 * BuildBehaviour maps 3 Suspended (empty), 4 Squashed, 5 Inventory and 6 Bonus;
 * id 7 returns CBehaviourProjectileNew but has no edit surface here and stays
 * raw. The serialized switch uses the 28-byte modern target records; legacy
 * level versions use 16-byte records and are preserved raw.
 */
struct KYALEVEL_API FKyaFruitOffsets
{
    int64 WeightB = 0;
    int64 Autonomous = 0;
    int64 Field360 = 0;
    int64 Switch = 0;
    int64 Tail = 0;
};

KYALEVEL_API bool KyaReadFruitOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size,
    FKyaFruitOffsets& Out);
KYALEVEL_API bool KyaReadFruitActor(FKyaActorPlacement& Out, TConstArrayView<uint8> Data,
    int64 Start, int64 Size);
KYALEVEL_API bool KyaWriteFruitActor(const FKyaActorPlacement& Actor, TArray<uint8>& Data,
    int64 Start, int64 Size);

/** CBehaviourInventory payload used by fruit behaviour 5 (CBehaviourInventory). */
struct KYALEVEL_API FKyaFruitInventoryOffsets
{
    int64 Field8 = 0, Field10 = 0, PurchaseId = 0, Field28 = 0, MoneyCost = 0, Field2C = 0;
    int64 ObjectFlag = 0, ObjectId = 0, Field18 = 0, InitialBehaviour = 0;
    int32 Size = 0;
    bool bHasData = false, bHasObject = false;
    int64 Tail = 0;
};

KYALEVEL_API bool KyaReadFruitInventoryOffsets(TConstArrayView<uint8> Data, int64 Start,
    int64 Size, FKyaFruitInventoryOffsets& Out);
KYALEVEL_API bool KyaReadFruitBehaviour(FKyaActorBehaviour& Out, int32 Id,
    TConstArrayView<uint8> Data, int64 Start, int32 Size);
KYALEVEL_API bool KyaWriteFruitBehaviour(const FKyaActorBehaviour& Behaviour, TArray<uint8>& Data,
    int64 Start, int32 Size);
