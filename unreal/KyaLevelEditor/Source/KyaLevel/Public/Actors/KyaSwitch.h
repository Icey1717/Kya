#pragma once

#include "CoreMinimal.h"

struct FKyaActorPlacement;
struct FKyaActorBehaviour;
struct FKyaSwitch;

/**
 * CActorSwitch (class 0x11) serialized codec.
 *
 * CActorSwitch::Create calls CActor::Create then targetSwitch.Create
 * (S_NTF_TARGET_STREAM_REF): switch-target list followed by a camera event.
 * BuildBehaviour maps ids 2-8; ids 0/1 are the generic stand/cinematic
 * behaviours. Behaviour 8 stores a sequence of S_NTF_SWITCH entries plus delays.
 */
struct KYALEVEL_API FKyaSwitchActorOffsets
{
    int64 Targets = 0;
    int64 Camera = 0;
    int64 Tail = 0;
    int32 Count = 0;
};

KYALEVEL_API bool KyaReadSwitchActorOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size,
    FKyaSwitchActorOffsets& Out);
/** Read/write one S_NTF_TARGET_STREAM_REF value (target list + camera). */
KYALEVEL_API bool KyaReadSwitchValue(TConstArrayView<uint8> Data, int64 Targets, int32 Count,
    int64 Camera, FKyaSwitch& Out);
KYALEVEL_API void KyaWriteSwitchValue(TArray<uint8>& Data, const FKyaSwitch& Switch, int64 Targets,
    int64 Camera);
KYALEVEL_API bool KyaReadSwitchActor(FKyaActorPlacement& Out, TConstArrayView<uint8> Data,
    int64 Start, int64 Size);
KYALEVEL_API bool KyaWriteSwitchActor(const FKyaActorPlacement& Actor, TArray<uint8>& Data,
    int64 Start, int64 Size);

struct KYALEVEL_API FKyaSwitchTimedEntryOffsets
{
    int64 Count = 0;
    int64 Targets = 0;
    int64 Camera = 0;
    int64 Delay = 0;
};

struct KYALEVEL_API FKyaSwitchBehaviourOffsets
{
    int64 LeverRefA = 0, LeverRefB = 0;
    int64 MagicField08 = 0, MagicField10 = 0, MagicFxDigits = 0, MagicField14 = 0, MagicField4C = 0;
    int64 MagicField18 = 0, MagicField28 = 0, MagicField24 = 0, MagicField30 = 0;
    int64 OpenTextureId = 0, OpenMeshId = 0, MagicCamera = 0;
    int64 MultiField14 = 0, MultiField08 = 0, MultiField0C = 0;
    int64 TargetDisplacement = 0, TargetSpringStrength = 0, TargetDamping = 0;
    int64 WolfenRequired = 0, WolfenTriggerRadius = 0, WolfenZoneRef = 0, WolfenActorRef = 0;
    int64 WolfenMaterialId = 0;
    int64 SequenceField08 = 0, SequenceField0C = 0, SequenceCount = 0, SequenceValues = 0;
    int64 TimedBaseDelay = 0, TimedEntryCount = 0, TimedPlaybackMode = 0, TimedFlags = 0;
    TArray<FKyaSwitchTimedEntryOffsets> TimedEntries;
    int32 SequenceCountValue = 0;
    int32 TimedEntryCountValue = 0;
    int64 Tail = 0;
    bool bLever = false, bMagic = false, bMultiCondition = false, bTarget = false;
    bool bWolfen = false, bSequence = false, bTimed = false;
};

KYALEVEL_API bool KyaReadSwitchBehaviourOffsets(int32 Id, TConstArrayView<uint8> Data, int64 Start,
    int64 Size, FKyaSwitchBehaviourOffsets& Out);

KYALEVEL_API bool KyaReadSwitchBehaviour(FKyaActorBehaviour& Out, int32 Id,
    TConstArrayView<uint8> Data, int64 Start, int32 Size);
KYALEVEL_API bool KyaWriteSwitchBehaviour(const FKyaActorBehaviour& Behaviour, TArray<uint8>& Data,
    int64 Start, int32 Size);
