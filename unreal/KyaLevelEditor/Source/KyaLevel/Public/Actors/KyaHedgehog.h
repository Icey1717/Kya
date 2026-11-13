#pragma once

#include "CoreMinimal.h"

struct FKyaActorPlacement;
struct FKyaActorBehaviour;

/**
 * CActorHedgehog::Create serialized layout (class id 0x22).
 *
 * CActorAutonomous::Create consumes the movable weightB, the autonomous life
 * maximum, gravity scale and two dynamic-extension floats. Create then reads
 * the walk/run scalars, two integers, three floats (in that serialized order),
 * a CAddOnGenerator, and a trailing integer. The generator optionally carries
 * five sub-object floats. See src/b-witch/ActorHedgehog.cpp and
 * src/b-witch/ActorBonusServices.cpp (CAddOnGenerator::Create).
 */
struct KYALEVEL_API FKyaHedgehogOffsets
{
    int64 WeightB = 0;
    int64 Autonomous = 0;
    int64 WalkSpeed = 0;
    int64 WalkAcceleration = 0;
    int64 WalkRotSpeed = 0;
    int64 RunSpeed = 0;
    int64 Fields374 = 0;
    int64 Fields378 = 0;
    int64 Floats368 = 0;
    int64 AddOn = 0;
    int64 SubObjMode = 0;
    int64 SubObjFloats = 0;
    int64 Field380 = 0;
    /** First byte after the decoded payload; remaining bytes stay raw. */
    int64 Tail = 0;
    int32 SubObjModeValue = 0;
};

KYALEVEL_API bool KyaReadHedgehogOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size,
    FKyaHedgehogOffsets& Out);
KYALEVEL_API bool KyaReadHedgehogActor(FKyaActorPlacement& Out, TConstArrayView<uint8> Data,
    int64 Start, int64 Size);
KYALEVEL_API bool KyaWriteHedgehogActor(const FKyaActorPlacement& Actor, TArray<uint8>& Data,
    int64 Start, int64 Size);

/** Decoded payload of one Hedgehog behaviour. */
struct KYALEVEL_API FKyaHedgehogBehaviourValues
{
    /** True when the payload matches one of Hedgehog's own behaviour ids. */
    bool bTyped = false;
    /** Behaviour 3 (GuardArea) and 5 (GuardAreaArmor) store a path-follow index. */
    int32 PathFollowIndex = -1;
    /** Behaviour 4 (WatchDog) and 6 (WatchDogArmor) are genuinely empty. */
    bool bEmptyPayload = false;
};

KYALEVEL_API bool KyaReadHedgehogBehaviour(int32 BehaviourId, TConstArrayView<uint8> Data,
    int64 Payload, int32 Size, FKyaHedgehogBehaviourValues& Out);
KYALEVEL_API bool KyaWriteHedgehogBehaviour(const FKyaActorBehaviour& Behaviour,
    TArray<uint8>& Data, int64 Payload, int32 Size);
