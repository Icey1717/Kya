#pragma once

#include "CoreMinimal.h"

struct FKyaActorPlacement;
struct FKyaActorBehaviour;

/**
 * CActorCheckpointManager (class 0x1E) serialized codec.
 *
 * CActorCheckpointManager::Create calls CActor::Create, reads a checkpoint count,
 * then per checkpoint: zone ref, waypoint A, sector id, waypoint B, flags and an
 * actor/waypoint pair list. BuildBehaviour only adds id 2
 * (CBehaviourCheckpoint), whose Create is empty; ids 0/1 are inherited.
 */
struct KYALEVEL_API FKyaCheckpointRecordOffsets
{
    int64 Record = 0;
    int64 ActorWaypoints = 0;
    int32 ActorWaypointCount = 0;
};

struct KYALEVEL_API FKyaCheckpointsOffsets
{
    int64 Count = 0;
    int32 CountValue = 0;
    TArray<FKyaCheckpointRecordOffsets> Records;
    int64 Tail = 0;
};

KYALEVEL_API bool KyaReadCheckpointsOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size,
    FKyaCheckpointsOffsets& Out);
KYALEVEL_API bool KyaReadCheckpointsActor(FKyaActorPlacement& Out, TConstArrayView<uint8> Data,
    int64 Start, int64 Size);
KYALEVEL_API bool KyaWriteCheckpointsActor(const FKyaActorPlacement& Actor, TArray<uint8>& Data,
    int64 Start, int64 Size);
