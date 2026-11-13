#pragma once

#include "CoreMinimal.h"

struct FKyaActorPlacement;
struct FKyaActorBehaviour;

/**
 * CActorDCA (class 0x1b) serialized codec.
 *
 * CActorDCA::Create calls CActor::Create, reads four stream refs, a float, an
 * aim-adjust angle and a CFireShot actor stream, then two words. BuildBehaviour
 * maps 2 Default, 3 Controlled, 4 AI-controlled and 5 Trajectory; ids 0/1 are
 * inherited. Behaviour 4's Create is an implementation guard in source, so it
 * stays raw rather than being guessed.
 */
struct KYALEVEL_API FKyaDcaOffsets
{
    int64 Field504 = 0, Field500 = 0, Field464 = 0, Field508 = 0, Field50C = 0, AimAdjust = 0;
    int64 ShotCount = 0, ShotEntries = 0;
    int32 ShotCountValue = 0;
    int64 Field450 = 0, Field454 = 0, Tail = 0;
};

KYALEVEL_API bool KyaReadDcaOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size,
    FKyaDcaOffsets& Out);
KYALEVEL_API bool KyaReadDcaActor(FKyaActorPlacement& Out, TConstArrayView<uint8> Data,
    int64 Start, int64 Size);
KYALEVEL_API bool KyaWriteDcaActor(const FKyaActorPlacement& Actor, TArray<uint8>& Data,
    int64 Start, int64 Size);

struct KYALEVEL_API FKyaDcaBehaviourOffsets
{
    int64 Controlled = 0, Trajectory = 0;
    bool bControlled = false, bTrajectory = false;
    int64 Tail = 0;
};

KYALEVEL_API bool KyaReadDcaBehaviourOffsets(int32 Id, TConstArrayView<uint8> Data, int64 Start,
    int32 Size, FKyaDcaBehaviourOffsets& Out);
KYALEVEL_API bool KyaReadDcaBehaviour(FKyaActorBehaviour& Out, int32 Id,
    TConstArrayView<uint8> Data, int64 Start, int32 Size);
KYALEVEL_API bool KyaWriteDcaBehaviour(const FKyaActorBehaviour& Behaviour, TArray<uint8>& Data,
    int64 Start, int32 Size);
