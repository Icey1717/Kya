#pragma once

#include "CoreMinimal.h"

struct FKyaActorPlacement;
struct FKyaActorBehaviour;

/**
 * CActorBonus (class 0x0e) serialized codec.
 *
 * CActorBonus::Create calls CActorMovable::Create, then reads a sound ref and
 * flare/animation material ids (16 bytes total). BuildBehaviour maps 2 (null),
 * 3 Turn, 4 Path, 5 Flock, 6 AddOn; ids 0/1 are inherited. Turn and Path share
 * the "alone" prefix, Flock/AddOn carry path/add-on references.
 */
struct KYALEVEL_API FKyaBonusOffsets
{
    int64 WeightB = 0;
    int64 Sound = 0;
    int64 FlareMaterial = 0;
    int64 AnimMaterial = 0;
    int64 Tail = 0;
};

KYALEVEL_API bool KyaReadBonusOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size,
    FKyaBonusOffsets& Out);
KYALEVEL_API bool KyaReadBonusActor(FKyaActorPlacement& Out, TConstArrayView<uint8> Data,
    int64 Start, int64 Size);
KYALEVEL_API bool KyaWriteBonusActor(const FKyaActorPlacement& Actor, TArray<uint8>& Data,
    int64 Start, int64 Size);

struct KYALEVEL_API FKyaBonusBehaviourValues
{
    float Field14 = 0, Field1B8 = 0, Field1B4 = 0;
    int32 MaterialId = -1;
    float Field1B0 = 0;
    int32 FxTailCount = 0, G2DIndex = -1;
    float TurnValue = 0;
    int32 FlockPathIndex = -1;
    float AddOnField18 = 0;
    int32 PathIndex = -1;
    float PathField1F4 = 0, PathField1F8 = 0, PathField1FC = 0, PathField200 = 0;
    int32 PathField204 = 0;
};

KYALEVEL_API bool KyaReadBonusBehaviour(int32 Id, TConstArrayView<uint8> Data, int64 Start,
    int32 Size, FKyaBonusBehaviourValues& Out);
KYALEVEL_API bool KyaReadBonusBehaviourRecord(FKyaActorBehaviour& Out, int32 Id,
    TConstArrayView<uint8> Data, int64 Start, int32 Size);
KYALEVEL_API bool KyaWriteBonusBehaviour(const FKyaActorBehaviour& Behaviour, TArray<uint8>& Data,
    int64 Start, int32 Size);
