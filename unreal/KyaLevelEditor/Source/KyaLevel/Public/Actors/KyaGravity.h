#pragma once

#include "CoreMinimal.h"
#include "Actors/KyaSwitch.h"

struct FKyaActorPlacement;
struct FKyaActorBehaviour;

/**
 * CActorGravityAware (class 0x10) serialized codec.
 *
 * CActorGravityAware::Create calls CActorAutonomous::Create and adds no extra
 * class payload of its own. BuildBehaviour maps 3 to CBehaviourGravityAwareFall
 * (one sound reference) and 4 to CBehaviourGravityAwareBell (sound reference,
 * field 0xC float, then a switch record); ids 0/1/2 are inherited.
 */
struct KYALEVEL_API FKyaGravityOffsets
{
    int64 WeightB = 0, Autonomous = 0, Tail = 0;
};

KYALEVEL_API bool KyaReadGravityOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size,
    FKyaGravityOffsets& Out);
KYALEVEL_API bool KyaReadGravityActor(FKyaActorPlacement& Out, TConstArrayView<uint8> Data,
    int64 Start, int64 Size);
KYALEVEL_API bool KyaWriteGravityActor(const FKyaActorPlacement& Actor, TArray<uint8>& Data,
    int64 Start, int64 Size);

struct KYALEVEL_API FKyaGravityBellOffsets
{
    int64 Sound = 0, FieldC = 0;
    int64 SwitchTargets = 0, SwitchCamera = 0;
    int32 SwitchCount = 0;
    int64 Tail = 0;
};

KYALEVEL_API bool KyaReadGravityBellOffsets(TConstArrayView<uint8> Data, int64 Start,
    int64 End, FKyaGravityBellOffsets& Out);
KYALEVEL_API bool KyaReadGravityBehaviour(FKyaActorBehaviour& Out, int32 Id,
    TConstArrayView<uint8> Data, int64 Start, int32 Size);
KYALEVEL_API bool KyaWriteGravityBehaviour(const FKyaActorBehaviour& Behaviour,
    TArray<uint8>& Data, int64 Start, int32 Size);
