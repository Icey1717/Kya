#pragma once

#include "CoreMinimal.h"

struct FKyaActorPlacement;
struct FKyaActorBehaviour;

/**
 * CActorNativ (class 0x0a) serialized codec.
 *
 * CActorNativ::Create calls CActorAutonomous::Create, reads eight words/floats
 * and a tail of five words. BuildBehaviour maps 3 Speak, 4 Exorcism (empty),
 * 5 Live, 6 TakeAndPut, 7 Seller and 8 Akasa; 0/1/2 are inherited. Only
 * behaviours 3/5/6/7 are decoded here; Akasa delegates to KyaNativAkasa.
 */
struct KYALEVEL_API FKyaNativOffsets
{
    int64 WeightB = 0, Autonomous = 0, Field350 = 0, Field354 = 0, Field358 = 0;
    int64 MoveSpeedNoHeld = 0, MoveSpeedHeld = 0, Field370 = 0, Field368 = 0, Field364 = 0;
    int64 Field378 = 0, Field37C = 0, Field380 = 0, Tail = 0;
};

KYALEVEL_API bool KyaReadNativOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size,
    FKyaNativOffsets& Out);
KYALEVEL_API bool KyaReadNativActor(FKyaActorPlacement& Out, TConstArrayView<uint8> Data,
    int64 Start, int64 Size);
KYALEVEL_API bool KyaWriteNativActor(const FKyaActorPlacement& Actor, TArray<uint8>& Data,
    int64 Start, int64 Size);

/** CBehaviourNativTakeAndPut: count + count x 16-byte CPathFollowReader entries. */
struct KYALEVEL_API FKyaNativTakePutOffsets
{
    int64 Count = 0;
    TArray<int64> Items;
    int64 Tail = 0;
    int32 CountValue = 0;
};

KYALEVEL_API bool KyaReadNativTakePutOffsets(TConstArrayView<uint8> Data, int64 Start,
    int64 End, FKyaNativTakePutOffsets& Out);
KYALEVEL_API bool KyaReadNativBehaviour(FKyaActorBehaviour& Out, int32 Id,
    TConstArrayView<uint8> Data, int64 Start, int32 Size);
KYALEVEL_API bool KyaWriteNativBehaviour(const FKyaActorBehaviour& Behaviour,
    TArray<uint8>& Data, int64 Start, int32 Size);
