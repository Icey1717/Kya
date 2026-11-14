#pragma once

#include "CoreMinimal.h"

struct FKyaActorPlacement;
struct FKyaActorBehaviour;

/**
 * CActorExplosiveDistributor (class 0x1D) serialized codec.
 *
 * CActorExplosiveDistributor::Create calls CActor::Create, reads a radius, an
 * angle, a projectile count and per projectile an actor reference, a direction
 * vector and an eject force, then one S_STREAM_EVENT_CAMERA record.
 * BuildBehaviour only adds id 2 (CBehaviourExplosiveDistributor), whose Create
 * is empty; ids 0/1 are inherited.
 */
struct KYALEVEL_API FKyaExplosiveDistributorOffsets
{
    int64 Radius = 0, Angle = 0, Count = 0, Entries = 0, Camera = 0, Tail = 0;
    int32 CountValue = 0;
};

KYALEVEL_API bool KyaReadExplosiveDistributorOffsets(TConstArrayView<uint8> Data,
    int64 Start, int64 Size, FKyaExplosiveDistributorOffsets& Out);
KYALEVEL_API bool KyaReadExplosiveDistributorActor(FKyaActorPlacement& Out,
    TConstArrayView<uint8> Data, int64 Start, int64 Size);
KYALEVEL_API bool KyaWriteExplosiveDistributorActor(const FKyaActorPlacement& Actor,
    TArray<uint8>& Data, int64 Start, int64 Size);
