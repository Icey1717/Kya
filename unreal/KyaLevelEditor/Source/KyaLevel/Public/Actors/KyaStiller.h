#pragma once

#include "CoreMinimal.h"
#include "Actors/KyaSwitch.h"

struct FKyaActorPlacement;
struct FKyaActorBehaviour;

/**
 * CActorStiller (class 0x4C) serialized codec.
 *
 * CActorStiller::Create calls CActor::Create, then reads the vision record,
 * four bone fields, a path-follow index, field 0x1E0, a CLifeBase (f32 max
 * value), three floats, field 0x1F8, an S_ACTOR_STREAM_REF actor list, field
 * 0x200, two S_NTF_TARGET_STREAM_REF switch records, field 0x214 and a
 * CAddOnGenerator. BuildBehaviour only adds id 2 (CBehaviourStillerStand),
 * whose Create is empty; ids 0/1 are inherited.
 */
struct KYALEVEL_API FKyaStillerOffsets
{
    int64 Vision = 0, Field1D0 = 0, Path = 0, Field1E0 = 0, LifeMax = 0, Floats1EC = 0;
    int64 Field1F8 = 0, ActorCount = 0, ActorEntries = 0, Field200 = 0;
    FKyaSwitchActorOffsets Switches[2];
    int64 Field214 = 0, AddOn = 0, SubObjMode = 0, SubObjFloats = 0, Tail = 0;
    int32 ActorCountValue = 0, SubObjModeValue = 0;
};

KYALEVEL_API bool KyaReadStillerOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size,
    FKyaStillerOffsets& Out);
KYALEVEL_API bool KyaReadStillerActor(FKyaActorPlacement& Out, TConstArrayView<uint8> Data,
    int64 Start, int64 Size);
KYALEVEL_API bool KyaWriteStillerActor(const FKyaActorPlacement& Actor, TArray<uint8>& Data,
    int64 Start, int64 Size);
