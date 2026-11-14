#pragma once

#include "CoreMinimal.h"

struct FKyaActorPlacement;
struct FKyaActorBehaviour;

/**
 * CActorPunchingBall (class 0x30) serialized codec.
 *
 * CActorPunchingBall::Create calls CActorFighter::Create (the shared fighter
 * prefix in Actors/KyaFighter.h), reads four floats, one S_TRAP_STREAM_REF,
 * two actor references, then CCamFigData::Create (fourteen floats). The fighter
 * prefix bytes are left untouched; only the PunchingBall-specific block is
 * typed and written.
 *
 * BuildBehaviour only adds ids 3/4/5/6 (default/projected/ridden/slave) and 7
 * (stand); every one of those Create functions is empty. Ids 0/1 are inherited.
 */
struct KYALEVEL_API FKyaPunchingBallOffsets
{
    int64 FighterEnd = 0;
    int64 Floats = 0;
    int64 TrapCount = 0, TrapEntries = 0;
    int64 ActorRefA94 = 0, ActorRefA98 = 0;
    int64 CamFig = 0, Tail = 0;
    int32 TrapCountValue = 0;
};

KYALEVEL_API bool KyaReadPunchingBallOffsets(TConstArrayView<uint8> Data, int64 Start,
    int64 Size, FKyaPunchingBallOffsets& Out);
KYALEVEL_API bool KyaReadPunchingBallActor(FKyaActorPlacement& Out,
    TConstArrayView<uint8> Data, int64 Start, int64 Size);
KYALEVEL_API bool KyaWritePunchingBallActor(const FKyaActorPlacement& Actor,
    TArray<uint8>& Data, int64 Start, int64 Size);
