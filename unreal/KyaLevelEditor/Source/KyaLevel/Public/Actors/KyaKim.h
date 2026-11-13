#pragma once

#include "CoreMinimal.h"
#include "Actors/KyaActorCamera.h"
#include "Actors/KyaFighter.h"

struct FKyaActorPlacement;

/**
 * CActorHeroPrivate (class 0x06, Kim) serialized codec.
 *
 * The shared CActorFighter prefix is followed by ten bone/anim keys, a climb-zone
 * stream, tutorial zones (24-byte header + target stream + camera), projectile
 * zones, boomy zone pairs, a zone stream, material/fx/joke values, actor cameras
 * and a 20-byte tail. The factory still populates the shared fighter prefix.
 */
struct KYALEVEL_API FKyaHeroTutorialOffsets
{
    int64 Header = 0, TargetCount = 0, Targets = 0, Camera = 0, Tail = 0;
    int32 TargetCountValue = 0;
};

struct KYALEVEL_API FKyaKimOffsets
{
    FKyaFighterPrefixOffsets Fighter;
    int64 FighterEnd = 0, Bones = 0;
    int64 ClimbCount = 0, ClimbEntries = 0;
    int32 ClimbCountValue = 0;
    int64 TutorialCount = 0, TutorialEntries = 0;
    int32 TutorialCountValue = 0;
    TArray<FKyaHeroTutorialOffsets> Tutorials;
    int64 ProjectileCount = 0, ProjectileEntries = 0;
    int32 ProjectileCountValue = 0;
    int64 PairCount = 0, PairEntries = 0;
    int32 PairCountValue = 0;
    int64 ZoneStreamCount = 0, ZoneStreamEntries = 0;
    int32 ZoneStreamCountValue = 0;
    int64 Material = 0, FxType = 0, JokeRadius = 0;
    int64 CameraCount = 0;
    int32 CameraCountValue = 0;
    TArray<FKyaActorCameraOffsets> Cameras;
    int64 Field15D4 = 0, FxInit = 0, FxHandles = 0, Tail = 0;
};

KYALEVEL_API bool KyaReadKimOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size,
    FKyaKimOffsets& Out);
KYALEVEL_API bool KyaReadKimActor(FKyaActorPlacement& Out, TConstArrayView<uint8> Data,
    int64 Start, int64 Size);
KYALEVEL_API bool KyaWriteKimActor(const FKyaActorPlacement& Actor, TArray<uint8>& Data,
    int64 Start, int64 Size);
