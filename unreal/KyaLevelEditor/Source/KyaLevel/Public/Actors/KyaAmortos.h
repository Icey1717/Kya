#pragma once

#include "CoreMinimal.h"

struct FKyaActorPlacement;

/**
 * CActorAmortos (class 0x29) serialized codec.
 *
 * CActorAmortos::Create calls CActor::Create, reads three S_ACTOR_STREAM_REF
 * lists, one actor reference, nine tuning floats, a CAddOnGenerator, an
 * attachment id, then two S_NTF_TARGET_STREAM_REF switch records. BuildBehaviour
 * only adds id 2 (CBehaviourAmortosStand), whose Create is empty; ids 0/1 are
 * inherited. The serialized float order is 164,168,160,16C,170,174,178,17C,180.
 */
struct KYALEVEL_API FKyaAmortosStreamOffsets
{
    int64 Count = 0;
    int64 Entries = 0;
    int32 CountValue = 0;
};

struct KYALEVEL_API FKyaAmortosSwitchOffsets
{
    int64 Targets = 0;
    int64 Camera = 0;
    int32 Count = 0;
};

struct KYALEVEL_API FKyaAmortosOffsets
{
    FKyaAmortosStreamOffsets Streams[3];
    int64 Field190 = 0, Floats160 = 0, AddOn = 0, SubObjMode = 0, SubObjFloats = 0, Field1C8 = 0;
    FKyaAmortosSwitchOffsets Switches[2];
    int64 Tail = 0;
    int32 SubObjModeValue = 0;
};

KYALEVEL_API bool KyaReadAmortosOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size,
    FKyaAmortosOffsets& Out);
KYALEVEL_API bool KyaReadAmortosActor(FKyaActorPlacement& Out, TConstArrayView<uint8> Data,
    int64 Start, int64 Size);
KYALEVEL_API bool KyaWriteAmortosActor(const FKyaActorPlacement& Actor, TArray<uint8>& Data,
    int64 Start, int64 Size);
