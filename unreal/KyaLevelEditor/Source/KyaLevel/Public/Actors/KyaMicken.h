#pragma once

#include "CoreMinimal.h"

struct FKyaActorPlacement;
struct FKyaActorBehaviour;

/**
 * CActorMicken (class 0x07) serialized codec.
 *
 * CActorMicken::Create calls CActorAutonomous::Create, reads two stream indices,
 * field 0x358, field 0x35C and field 0x360. BuildBehaviour maps 4 to
 * CBehaviourMickenEat (two floats, a word and a path index), 5/6 to
 * CBehaviourMickenKicked and 7/8 to CBehaviourMickenSquashed (all empty); ids
 * 0/1/2 are inherited.
 */
struct KYALEVEL_API FKyaMickenOffsets
{
    int64 WeightB = 0, Autonomous = 0, Waypoint = 0, Actor = 0;
    int64 Field358 = 0, Field35C = 0, Field360 = 0, Tail = 0;
};

KYALEVEL_API bool KyaReadMickenOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size,
    FKyaMickenOffsets& Out);
KYALEVEL_API bool KyaReadMickenActor(FKyaActorPlacement& Out, TConstArrayView<uint8> Data,
    int64 Start, int64 Size);
KYALEVEL_API bool KyaWriteMickenActor(const FKyaActorPlacement& Actor, TArray<uint8>& Data,
    int64 Start, int64 Size);

struct KYALEVEL_API FKyaMickenBehaviourValues
{
    float Field8 = 0, FieldC = 0;
    int32 Field10 = 0, PathIndex = -1;
};

KYALEVEL_API bool KyaReadMickenBehaviour(int32 Id, TConstArrayView<uint8> Data, int64 Start,
    int64 Size, FKyaMickenBehaviourValues& Out);
KYALEVEL_API bool KyaReadMickenBehaviourValue(FKyaActorBehaviour& Out, int32 Id,
    TConstArrayView<uint8> Data, int64 Start, int32 Size);
KYALEVEL_API bool KyaWriteMickenBehaviour(const FKyaActorBehaviour& Behaviour,
    TArray<uint8>& Data, int64 Start, int32 Size);
