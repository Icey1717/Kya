#pragma once

#include "CoreMinimal.h"
#include "Actors/KyaSwitch.h"

struct FKyaActorPlacement;
struct FKyaActorBehaviour;

/**
 * CActorBasicBox (class 0x39) serialized codec.
 *
 * CActorBasicBox::Create calls CActor::Create, reads field 0x160, a life value,
 * an S_ACTOR_STREAM_REF actor list, four vibration floats, field 0x180, four
 * words, one switch record and a CAddOnGenerator. BuildBehaviour only adds id 2
 * (CBehaviourBasicBoxStand), whose Create is empty; ids 0/1 are inherited.
 */
struct KYALEVEL_API FKyaBasicBoxOffsets
{
    int64 Field160 = 0, LifeMax = 0, Actors = 0, Vibration = 0, Field180 = 0, Fields = 0;
    int64 SwitchTargets = 0, SwitchCamera = 0, AddOn = 0, SubObjMode = 0, SubObjFloats = 0;
    int64 Tail = 0;
    int32 ActorCount = 0, SwitchTargetCount = 0, SubObjModeValue = 0;
};

KYALEVEL_API bool KyaReadBasicBoxOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size,
    FKyaBasicBoxOffsets& Out);
KYALEVEL_API bool KyaReadBasicBoxActor(FKyaActorPlacement& Out, TConstArrayView<uint8> Data,
    int64 Start, int64 Size);
KYALEVEL_API bool KyaWriteBasicBoxActor(const FKyaActorPlacement& Actor, TArray<uint8>& Data,
    int64 Start, int64 Size);
