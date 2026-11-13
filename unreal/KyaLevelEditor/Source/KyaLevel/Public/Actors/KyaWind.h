#pragma once

#include "CoreMinimal.h"

struct FKyaActorPlacement;

/**
 * CActorWind (class 0x12) serialized codec.
 *
 * Create calls CActor::Create, reads the wind tuning block, sound/zone/actor
 * streams, material/colour/waypoint references, the sector hierarchy and (on
 * newer levels) three S_NTF_SWITCH records, then five floats and two waypoint
 * values. BuildBehaviour only adds id 2 (CBehaviourWind, empty Create); ids 0/1
 * are the inherited stand/cinematic behaviours.
 */
struct KYALEVEL_API FKyaWindEmbeddedSwitchOffsets
{
    int64 Targets = 0;
    int64 Camera = 0;
    int32 Count = 0;
};

struct KYALEVEL_API FKyaWindOffsets
{
    int64 Field160 = 0, MaxWind = 0, Field168 = 0, Field16C = 0, Field174 = 0, Field180 = 0;
    int64 Field184 = 0, Field188 = 0, Field18C = 0, Field178 = 0, Field17C = 0, Field190 = 0;
    int64 Sound = 0, ActivationZone = 0, Actors = 0;
    int32 ActorCount = 0;
    int64 Material = 0, Zone1C4 = 0, Color1C8 = 0, Waypoint1CC = 0, SectorHash = 0;
    bool bHasSwitches = false;
    FKyaWindEmbeddedSwitchOffsets Switches[3];
    int64 Floats194 = 0, Waypoint = 0, Tail = 0;
};

KYALEVEL_API bool KyaReadWindOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size,
    FKyaWindOffsets& Out);
KYALEVEL_API bool KyaReadWindActor(FKyaActorPlacement& Out, TConstArrayView<uint8> Data,
    int64 Start, int64 Size);
KYALEVEL_API bool KyaWriteWindActor(const FKyaActorPlacement& Actor, TArray<uint8>& Data,
    int64 Start, int64 Size);
