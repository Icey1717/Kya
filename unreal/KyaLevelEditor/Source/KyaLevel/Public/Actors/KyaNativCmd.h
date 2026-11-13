#pragma once

#include "CoreMinimal.h"

struct FKyaActorPlacement;
struct FKyaActorBehaviour;

/**
 * CActorNativCmd (class 0x3C) serialized codec.
 *
 * CActorNativCmd::Create calls CActor::Create, reads an S_ACTOR_STREAM_REF actor
 * list, field 0x164 and four CAddOnNativ blocks (A-D), each a sub-object count
 * plus CAddOnSubObj records (field 0 u32, cinematic count u32, ids).
 * BuildBehaviour only adds id 2 (CBehaviourNativCmdStand), whose Create is
 * empty; ids 0/1 are inherited.
 */
struct KYALEVEL_API FKyaNativCmdOffsets
{
    int64 ActorCount = 0, ActorEntries = 0, Field164 = 0, AddOns = 0, Tail = 0;
    int32 ActorCountValue = 0;
    TArray<int64> AddOnCounts, SubObjOffsets;
    TArray<int32> AddOnCountValues;
};

KYALEVEL_API bool KyaReadNativCmdOffsets(TConstArrayView<uint8> Data, int64 Start,
    int64 Size, FKyaNativCmdOffsets& Out);
KYALEVEL_API bool KyaReadNativCmdActor(FKyaActorPlacement& Out, TConstArrayView<uint8> Data,
    int64 Start, int64 Size);
KYALEVEL_API bool KyaWriteNativCmdActor(const FKyaActorPlacement& Actor, TArray<uint8>& Data,
    int64 Start, int64 Size);
