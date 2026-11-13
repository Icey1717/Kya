#pragma once

#include "CoreMinimal.h"
#include "Actors/KyaActorCamera.h"

struct FKyaActorPlacement;

/**
 * CActorBoomy (class 0x05) serialized codec.
 *
 * CActorBoomy::Create calls CActorMovable::Create (weight), then reads the
 * vision record, tail/fx values, particles, texture/mesh, a full actor-camera
 * record and a 16-byte type list. BuildBehaviour only adds id 2
 * (CBehaviourBoomyLaunch), whose Create is empty; ids 0/1 are inherited.
 */
struct KYALEVEL_API FKyaBoomyOffsets
{
    int64 Weight = 0;
    int64 Vision = 0;
    FKyaActorCameraOffsets Camera;
    int64 CameraStart = 0;
    int64 Sound2B8 = 0, Sound2BC = 0, Field744 = 0;
    int64 TailWidthAxis = 0, Field740 = 0, TailSegments = 0;
    int64 Particle3D0 = 0, Particle3D4 = 0, Particle3DC = 0, Field3CC = 0;
    int64 Texture = 0, Mesh = 0;
    int64 TypeCount = 0, TypeEntries = 0;
    int32 TypeCountValue = 0;
    int64 Tail = 0;
};

KYALEVEL_API bool KyaReadBoomyOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size,
    FKyaBoomyOffsets& Out);
KYALEVEL_API bool KyaReadBoomyActor(FKyaActorPlacement& Out, TConstArrayView<uint8> Data,
    int64 Start, int64 Size);
KYALEVEL_API bool KyaWriteBoomyActor(const FKyaActorPlacement& Actor, TArray<uint8>& Data,
    int64 Start, int64 Size);
