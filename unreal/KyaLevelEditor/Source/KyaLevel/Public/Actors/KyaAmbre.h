#pragma once

#include "CoreMinimal.h"
#include "Actors/KyaSwitch.h"

struct FKyaActorPlacement;
struct FKyaActorBehaviour;

/**
 * CActorAmbre (class 0x14) serialized codec.
 *
 * CActorAmbre::Create calls CActor::Create, reads a material id, effect type,
 * texture/mesh ids and field 0x1D0, then one S_NTF_TARGET_STREAM_REF switch
 * record. BuildBehaviour returns CBehaviourAmbreUnknown for id 3 and the stored
 * amber behaviour for id 2; neither overrides Create, so both payloads are
 * empty. Ids 0/1 are inherited.
 */
struct KYALEVEL_API FKyaAmbreOffsets
{
    int64 Material = 0, EffectType = 0, Texture = 0, Mesh = 0, Field1D0 = 0;
    FKyaSwitchActorOffsets Switch;
    int64 Tail = 0;
};

KYALEVEL_API bool KyaReadAmbreOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size,
    FKyaAmbreOffsets& Out);
KYALEVEL_API bool KyaReadAmbreActor(FKyaActorPlacement& Out, TConstArrayView<uint8> Data,
    int64 Start, int64 Size);
KYALEVEL_API bool KyaWriteAmbreActor(const FKyaActorPlacement& Actor, TArray<uint8>& Data,
    int64 Start, int64 Size);
