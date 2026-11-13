#pragma once

#include "CoreMinimal.h"

struct FKyaActorPlacement;
struct FKyaActorBehaviour;

/**
 * CActorBridge (class 0x17) serialized codec.
 *
 * CActorBridge::Create calls CActor::Create, reads an S_ACTOR_STREAM_REF actor
 * list, two waypoint indices, five floats, field 0x344, config flags, field
 * 0x348 and the step-collision scale. BuildBehaviour maps 2 to
 * CBehaviourBridgeBasic (32 bytes: direction + five floats) and 3 to
 * CBehaviourBridgeWindAware (20 bytes: five floats); ids 0/1 are inherited.
 */
struct KYALEVEL_API FKyaBridgeOffsets
{
    int64 ActorCount = 0, ActorEntries = 0, WaypointA = 0, WaypointB = 0, Floats330 = 0;
    int64 Field344 = 0, ConfigFlags = 0, Field348 = 0, StepCollisionScale = 0, Tail = 0;
    int32 ActorCountValue = 0;
};

KYALEVEL_API bool KyaReadBridgeOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size,
    FKyaBridgeOffsets& Out);
KYALEVEL_API bool KyaReadBridgeActor(FKyaActorPlacement& Out, TConstArrayView<uint8> Data,
    int64 Start, int64 Size);
KYALEVEL_API bool KyaWriteBridgeActor(const FKyaActorPlacement& Actor, TArray<uint8>& Data,
    int64 Start, int64 Size);

struct KYALEVEL_API FKyaBridgeBasicOffsets
{
    int64 Direction = 0, Fields40 = 0, Tail = 0;
};

struct KYALEVEL_API FKyaBridgeWindOffsets
{
    int64 Fields40 = 0, Tail = 0;
};

KYALEVEL_API bool KyaReadBridgeBasicOffsets(TConstArrayView<uint8> Data, int64 Start,
    int64 Size, FKyaBridgeBasicOffsets& Out);
KYALEVEL_API bool KyaReadBridgeWindOffsets(TConstArrayView<uint8> Data, int64 Start,
    int64 Size, FKyaBridgeWindOffsets& Out);
KYALEVEL_API bool KyaReadBridgeBehaviour(FKyaActorBehaviour& Out, int32 Id,
    TConstArrayView<uint8> Data, int64 Start, int32 Size);
KYALEVEL_API bool KyaWriteBridgeBehaviour(const FKyaActorBehaviour& Behaviour,
    TArray<uint8>& Data, int64 Start, int32 Size);
