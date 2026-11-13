#pragma once

#include "CoreMinimal.h"

struct FKyaActorPlacement;
struct FKyaActorBehaviour;
struct FKyaPlatformCamera;

/**
 * CActorMovingPlatform (class 0x0d) serialized codec.
 *
 * Create calls CActorMovable::Create (weightB), then reads the carried-actor
 * stream, zone stream, no-friction zones, camera stream, a 0x4c-byte properties
 * block and two platform-sound streams. The behaviour codecs cover ids 2-10;
 * ids 0/1 are the generic stand/cinematic behaviours.
 */
struct KYALEVEL_API FKyaPlatformPrefixOffsets
{
    int64 Weight = 0;
    int64 Actors = 0;
    int64 Zones = 0;
    int64 Suffix = 0;
    int32 ActorCount = 0;
    int32 ZoneCount = 0;
    TArray<int64> NoFrictionZones;
};

struct KYALEVEL_API FKyaPlatformDetailsOffsets
{
    int64 Cameras = 0;
    int64 Properties = 0;
    int64 Sounds[2] = {};
    int64 Tail = 0;
    int32 CameraCount = 0;
    int32 SoundCounts[2] = {};
};

KYALEVEL_API bool KyaReadPlatformPrefix(TConstArrayView<uint8> Data, int64 Start, int64 Size,
    FKyaPlatformPrefixOffsets& Out);
KYALEVEL_API bool KyaReadPlatformDetails(TConstArrayView<uint8> Data, int64 Start, int64 Size,
    FKyaPlatformDetailsOffsets& Out);
KYALEVEL_API bool KyaReadPlatformCamera(TConstArrayView<uint8> Data, int64 Offset,
    FKyaPlatformCamera& Out);

/** Decode the class payload into the placement and preserve any raw tail. */
KYALEVEL_API bool KyaReadMovingPlatformActor(FKyaActorPlacement& Out, TConstArrayView<uint8> Data,
    int64 Start, int64 Size);
/** Write the typed class fields in place. The caller preserves the raw tail. */
KYALEVEL_API bool KyaWriteMovingPlatformActor(const FKyaActorPlacement& Actor, TArray<uint8>& Data,
    int64 Start, int64 Size);

struct KYALEVEL_API FKyaPlatformSwitchOffsets
{
    int64 Targets = 0;
    int64 Camera = 0;
    int32 Count = 0;
};

struct KYALEVEL_API FKyaPlatformBehaviourOffsets
{
    int64 SoundRef = 0, Field0C = 0, PathIndex = 0, Timing = 0, PathField4 = 0, PathTotalTime = 0;
    int64 Field40 = 0, AltModelId = 0, SegmentStartTime = 0, LoopDelay = 0;
    int64 Field18 = 0, MaterialId = 0, Field8 = 0, FieldC = 0, Field10 = 0;
    FKyaPlatformSwitchOffsets SlabSwitch;
    int64 SlabFloats = 0, SlabField20 = 0;
    int64 MasterPath = 0, MasterStream = 0, MasterFloats = 0, AnalogCount = 0, AnalogEntries = 0, AnalogCamera = 0;
    int32 AnalogCountValue = 0;
    int64 SelectorActorCount = 0, SelectorActorEntries = 0, SelectorSwitchCount = 0, SelectorSwitchEntries = 0, SelectorCamera = 0;
    int32 SelectorActorCountValue = 0, SelectorSwitchCountValue = 0;
    int64 Tail = 0;
    bool bStand = false, bSlave = false, bDestroyed = false, bSelectorSlave = false, bTrajectory = false;
    bool bSlab = false, bMaster = false, bSelectorMaster = false, bTeleportRandom = false;
};

KYALEVEL_API bool KyaReadPlatformBehaviourOffsets(int32 Id, TConstArrayView<uint8> Data, int64 Start,
    int64 Size, FKyaPlatformBehaviourOffsets& Out);

KYALEVEL_API bool KyaReadMovingPlatformBehaviour(FKyaActorBehaviour& Out, int32 Id,
    TConstArrayView<uint8> Data, int64 Start, int32 Size);
KYALEVEL_API bool KyaWriteMovingPlatformBehaviour(const FKyaActorBehaviour& Behaviour,
    TArray<uint8>& Data, int64 Start, int32 Size);
