#pragma once

#include "CoreMinimal.h"

struct FKyaActorPlacement;
struct FKyaActorBehaviour;

/**
 * CActorCommander (class 0x13) serialized codec.
 *
 * CActorCommander::Create calls CActor::Create, reads five zone/path words, a
 * CSquad (12 bytes), team enemy indices, a sequence of 8-byte component pairs and
 * a 56-byte camera-figure block. BuildBehaviour maps 2 Default and 3 BeatUp.
 * Behaviour 2 owns the squad config and one target/camera switch record.
 */
struct KYALEVEL_API FKyaCommanderOffsets
{
    int64 Headers = 0, Squad = 0, Teams = 0, Components = 0, Camera = 0, Tail = 0;
    int32 TeamCount = 0, ComponentCount = 0;
};

KYALEVEL_API bool KyaReadCommanderOffsets(TConstArrayView<uint8> Data, int64 Start,
    int64 Size, FKyaCommanderOffsets& Out);
KYALEVEL_API bool KyaReadCommanderActor(FKyaActorPlacement& Out, TConstArrayView<uint8> Data,
    int64 Start, int64 Size);
KYALEVEL_API bool KyaWriteCommanderActor(const FKyaActorPlacement& Actor, TArray<uint8>& Data,
    int64 Start, int64 Size);

struct KYALEVEL_API FKyaCommanderBehaviourOffsets
{
    int64 SquadConfig = 0, Switch = 0, Tail = 0;
};

KYALEVEL_API bool KyaReadCommanderBehaviourOffsets(TConstArrayView<uint8> Data, int64 Start,
    int64 End, FKyaCommanderBehaviourOffsets& Out);
KYALEVEL_API bool KyaReadCommanderBehaviour(FKyaActorBehaviour& Out, int32 Id,
    TConstArrayView<uint8> Data, int64 Start, int32 Size);
KYALEVEL_API bool KyaWriteCommanderBehaviour(const FKyaActorBehaviour& Behaviour,
    TArray<uint8>& Data, int64 Start, int32 Size);
