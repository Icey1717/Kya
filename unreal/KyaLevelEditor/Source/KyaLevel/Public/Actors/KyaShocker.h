#pragma once

#include "CoreMinimal.h"
#include "Actors/KyaSwitch.h"

struct FKyaActorPlacement;
struct FKyaActorBehaviour;

/**
 * CActorShocker (class 0x4B) serialized codec.
 *
 * CActorShocker::Create calls CActorAutonomous::Create, reads nine floats, two
 * zone references, one S_NTF_SWITCH_ONOFF record, field 0x380, six words, a bone
 * id and a CAddOnGenerator. BuildBehaviour only adds id 3
 * (CBehaviourShockerFireWave), whose Create reads a CCircularWaveShoot record
 * (six floats + two words); ids 0/1/2 are inherited.
 */
struct KYALEVEL_API FKyaShockerOffsets
{
    int64 WeightB = 0, Autonomous = 0;
    int64 Floats = 0;
    int64 ZoneInner = 0, ZoneOuter = 0;
    int64 SwitchTargets = 0, SwitchCamera = 0;
    int32 SwitchCount = 0;
    int64 Field380 = 0, Fields = 0, BoneId = 0;
    int64 AddOn = 0, SubObjMode = 0, SubObjFloats = 0;
    int32 SubObjModeValue = 0;
    int64 Tail = 0;
};

KYALEVEL_API bool KyaReadShockerOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size,
    FKyaShockerOffsets& Out);
KYALEVEL_API bool KyaReadShockerActor(FKyaActorPlacement& Out, TConstArrayView<uint8> Data,
    int64 Start, int64 Size);
KYALEVEL_API bool KyaWriteShockerActor(const FKyaActorPlacement& Actor, TArray<uint8>& Data,
    int64 Start, int64 Size);

KYALEVEL_API bool KyaReadShockerBehaviour(FKyaActorBehaviour& Out, int32 Id,
    TConstArrayView<uint8> Data, int64 Start, int32 Size);
KYALEVEL_API bool KyaWriteShockerBehaviour(const FKyaActorBehaviour& Behaviour,
    TArray<uint8>& Data, int64 Start, int32 Size);
