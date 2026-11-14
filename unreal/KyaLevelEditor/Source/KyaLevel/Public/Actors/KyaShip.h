#pragma once

#include "CoreMinimal.h"
#include "Actors/KyaSwitch.h"

struct FKyaActorPlacement;
struct FKyaActorBehaviour;

/**
 * CActorShip (class 0x36) serialized codec.
 *
 * Bank actors are named `HUNTERSHIP_01`, which is why the cross-bank table
 * lists 0x36 as HUNTERSHIP. CActorShip::Create calls CActor::Create, reads one
 * S_ACTOR_STREAM_REF, one CFxArrayManager list, three actor references, nine
 * floats, four words, the two 4-word blocks, a CVision block (four floats and
 * a flags word) and one S_NTF_SWITCH.
 *
 * BuildBehaviour only adds id 2 (SHIP_BEHAVIOUR_FLY); CBehaviourShip and
 * CBehaviourShipFLY both have empty Create bodies. Ids 0/1 are inherited.
 */
struct KYALEVEL_API FKyaShipOffsets
{
    int64 CareBoyCount = 0, CareBoyEntries = 0;
    int64 FxCount = 0, FxEntries = 0;
    int64 ActorRefs = 0, Floats = 0, Fields = 0;
    int64 Words18C = 0, Words19C = 0, Vision = 0;
    int64 SwitchTargets = 0, SwitchCamera = 0, Tail = 0;
    int32 CareBoyCountValue = 0, FxCountValue = 0, SwitchCountValue = 0;
};

KYALEVEL_API bool KyaReadShipOffsets(TConstArrayView<uint8> Data, int64 Start,
    int64 Size, FKyaShipOffsets& Out);
KYALEVEL_API bool KyaReadShipActor(FKyaActorPlacement& Out,
    TConstArrayView<uint8> Data, int64 Start, int64 Size);
KYALEVEL_API bool KyaWriteShipActor(const FKyaActorPlacement& Actor,
    TArray<uint8>& Data, int64 Start, int64 Size);
