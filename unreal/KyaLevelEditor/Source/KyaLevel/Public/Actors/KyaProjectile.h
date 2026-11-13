#pragma once

#include "CoreMinimal.h"

struct FKyaActorPlacement;
struct FKyaActorBehaviour;
struct FKyaInventoryInfo;

/**
 * CActorProjectile (class 0x1c) serialized codec.
 *
 * Create calls CActorAutonomous::Create (weightB, life maximum, gravity scale and
 * two dynamic-extension floats), copies one 0x48-byte ProjectileSubObj directly
 * from the byte stream, then calls patternPart.Create. Behaviour payloads are
 * owned by the individual CBehaviourProjectile subclasses; see the module
 * implementation for the per-id field mapping and sizes.
 */
struct KYALEVEL_API FKyaProjectileOffsets
{
    int64 WeightB = 0;
    int64 Autonomous = 0;
    int64 SubObj = 0;
    int64 Pattern = 0;
    int64 PatternField24 = 0;
    /** First byte after the decoded payload; remaining bytes stay raw. */
    int64 Tail = 0;
    bool bHasPatternField24 = false;
};

KYALEVEL_API bool KyaReadProjectileOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size,
    FKyaProjectileOffsets& Out);

/** Decode the class payload into the placement and replace ClassData with the raw tail. */
KYALEVEL_API bool KyaReadProjectileActor(FKyaActorPlacement& Out, TConstArrayView<uint8> Data,
    int64 Start, int64 Size);

/** Write the typed class fields in place. The caller preserves the raw tail. */
KYALEVEL_API bool KyaWriteProjectileActor(const FKyaActorPlacement& Actor, TArray<uint8>& Data,
    int64 Start, int64 Size);

/**
 * Behaviour payload layout. Ids 3 (Stand), 4 (Straight), 5 (Portable),
 * 10 (LavaBall), 11 (Excuse) and 13 (New) derive from CBehaviourProjectile and
 * start with behaviourId; id 12 (Inactive) is CBehaviourInactive and starts with
 * activateMsgId. Id 9 (Inventory) uses the inventory codec below.
 */
struct KYALEVEL_API FKyaProjectileBehaviourOffsets
{
    int64 BehaviourId = 0;
    int64 Field0C = 0;
    int64 MaterialId = 0;
    int64 Speed = 0;
    int64 Acceleration = 0;
    int64 Field18 = 0;
    int64 Field10 = 0;
    int64 ActorRef = 0;
    int64 LavaBallField14 = 0;
    int64 LavaBallField30 = 0;
    int64 LavaBallField18 = 0;
    int64 LavaBallPathIndex = 0;
    int64 LavaBallField2C = 0;
    int64 LavaBallField34 = 0;
    int64 LavaBallField38 = 0;
    int64 LavaBallField3C = 0;
    int64 InactiveActivateMessageId = 0;
    int64 InactiveFlags = 0;
    int64 InactiveActiveBehaviourId = 0;
    int64 Tail = 0;
    bool bStand = false;
    bool bStraight = false;
    bool bPortable = false;
    bool bExcuse = false;
    bool bLavaBall = false;
    bool bInactive = false;
    bool bNew = false;
    bool bHasLavaBallField18 = false;
};

KYALEVEL_API bool KyaReadProjectileBehaviourOffsets(int32 Id, TConstArrayView<uint8> Data,
    int64 Start, int32 Size, FKyaProjectileBehaviourOffsets& Out);

KYALEVEL_API bool KyaReadProjectileBehaviour(FKyaActorBehaviour& Out, int32 Id,
    TConstArrayView<uint8> Data, int64 Start, int32 Size);
KYALEVEL_API bool KyaWriteProjectileBehaviour(const FKyaActorBehaviour& Behaviour,
    TArray<uint8>& Data, int64 Start, int32 Size);

/** CInventoryInfo + initial behaviour: 8 absent, 40 legacy, 44/56 modern bytes. */
struct KYALEVEL_API FKyaProjectileInventoryOffsets
{
    int64 Field0 = 0, Field8 = 0, Field10 = 0, PurchaseId = 0, Field28 = 0;
    int64 MoneyCost = 0, Field2C = 0;
    int64 ObjectFlag = 0, ObjectId = 0, Field18 = 0, InitialBehaviourId = 0;
    bool bHasObject = false;
    int64 Tail = 0;
};

KYALEVEL_API bool KyaReadProjectileInventoryOffsets(TConstArrayView<uint8> Data, int64 Start,
    int64 Size, FKyaProjectileInventoryOffsets& Out);
KYALEVEL_API bool KyaReadProjectileInventory(FKyaInventoryInfo& Out, TConstArrayView<uint8> Data,
    int64 Start, int32 Size);
KYALEVEL_API bool KyaWriteProjectileInventory(const FKyaInventoryInfo& Inventory,
    TArray<uint8>& Data, int64 Start, int32 Size);
