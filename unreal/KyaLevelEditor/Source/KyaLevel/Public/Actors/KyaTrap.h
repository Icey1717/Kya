#pragma once

#include "CoreMinimal.h"

struct FKyaActorPlacement;
struct FKyaActorBehaviour;

/**
 * CActorTrap (class 0x2c) serialized codec.
 *
 * CActorTrap::Create calls CActorMovable::Create then
 * CBehaviourTrapStand_ActorTrap::CreateForTrap: actor stream, five tuning
 * words, an optional field 0x24 (level version >= 2.24), actor/path/speed
 * values, required escape count and a trap stream of switch/camera entries.
 * BuildBehaviour maps 2 (TrapStand) and 3 (Inactive); ids 0/1 are inherited.
 * Both behaviour payloads are 12 bytes but have different field meanings.
 */
struct KYALEVEL_API FKyaTrapOffsets
{
    int64 WeightB = 0, Actors = 0, Field14 = 0, Field18 = 0, Field1C = 0, Field20 = 0, Field24 = 0;
    int64 ActorRef = 0, Speed = 0, Acceleration = 0, PathFollow = 0, Field4C = 0, RequiredEscapes = 0;
    int64 TrapEntries = 0, Tail = 0;
    int32 ActorCount = 0, TrapCount = 0;
    bool bHasField24 = false;
};

KYALEVEL_API bool KyaReadTrapOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size,
    FKyaTrapOffsets& Out);
KYALEVEL_API bool KyaReadTrapActor(FKyaActorPlacement& Out, TConstArrayView<uint8> Data,
    int64 Start, int64 Size);
KYALEVEL_API bool KyaWriteTrapActor(const FKyaActorPlacement& Actor, TArray<uint8>& Data,
    int64 Start, int64 Size);

KYALEVEL_API bool KyaReadTrapBehaviour(FKyaActorBehaviour& Out, int32 Id,
    TConstArrayView<uint8> Data, int64 Start, int32 Size);
KYALEVEL_API bool KyaWriteTrapBehaviour(const FKyaActorBehaviour& Behaviour, TArray<uint8>& Data,
    int64 Start, int32 Size);
