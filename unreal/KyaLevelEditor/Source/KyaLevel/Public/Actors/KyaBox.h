#pragma once

#include "CoreMinimal.h"
#include "Actors/KyaSwitch.h"

struct FKyaActorPlacement;
struct FKyaActorBehaviour;

/**
 * CActorBox (class 0x35) serialized codec.
 *
 * CActorBox::Create calls CActorAutonomous::Create, reads field 0x350, an
 * S_ACTOR_STREAM_REF actor list, four floats, field 0x364 and four more words,
 * one S_NTF_TARGET_STREAM_REF switch record and a CAddOnGenerator. BuildBehaviour
 * only adds id 3 (CBehaviourBoxStand), whose Create is empty; ids 0/1/2 are
 * inherited.
 */
struct KYALEVEL_API FKyaBoxOffsets
{
    int64 WeightB = 0, Autonomous = 0, Field350 = 0;
    int64 ActorCount = 0, ActorEntries = 0;
    int32 ActorCountValue = 0;
    int64 Floats354 = 0, Field364 = 0, Field370 = 0;
    FKyaSwitchActorOffsets Switch;
    int64 AddOn = 0, SubObjMode = 0, SubObjFloats = 0, Tail = 0;
    int32 SubObjModeValue = 0;
};

KYALEVEL_API bool KyaReadBoxOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size,
    FKyaBoxOffsets& Out);
KYALEVEL_API bool KyaReadBoxActor(FKyaActorPlacement& Out, TConstArrayView<uint8> Data,
    int64 Start, int64 Size);
KYALEVEL_API bool KyaWriteBoxActor(const FKyaActorPlacement& Actor, TArray<uint8>& Data,
    int64 Start, int64 Size);
