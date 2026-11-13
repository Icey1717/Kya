#pragma once

#include "CoreMinimal.h"

struct FKyaActorPlacement;
struct FKyaActorBehaviour;

/**
 * CActorAton (class 0x38) serialized codec.
 *
 * CActorAton::Create calls CActorAutonomous::Create (weight/life/gravity/
 * dynamic-extension prefix), reads four words, type/texture/mesh, a trail
 * material, a path-plane list, then CBehaviourAddOnAton: field 0x18, the
 * "other" sub-objects, the Aton sub-objects and field 0x24. BuildBehaviour only
 * adds id 3 (CBehaviourAtonEscape), which has no Create override and therefore
 * no payload; ids 0/1/2 are inherited.
 */
struct KYALEVEL_API FKyaAtonOffsets
{
    int64 WeightB = 0, Autonomous = 0, Fields354 = 0, EffectType = 0, Texture = 0, Mesh = 0;
    int64 Trail = 0, PathDelay = 0, PathCount = 0, PathEntries = 0;
    int32 PathCountValue = 0;
    int64 AddOn = 0, AddOnField18 = 0, OtherCount = 0, OtherEntries = 0;
    int64 AtonCount = 0, AtonEntries = 0, AddOnField24 = 0, Tail = 0;
    int32 OtherCountValue = 0, AtonCountValue = 0;
    TArray<int64> OtherOffsets, AtonOffsets;
};

KYALEVEL_API bool KyaReadAtonOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size,
    FKyaAtonOffsets& Out);
KYALEVEL_API bool KyaReadAtonActor(FKyaActorPlacement& Out, TConstArrayView<uint8> Data,
    int64 Start, int64 Size);
KYALEVEL_API bool KyaWriteAtonActor(const FKyaActorPlacement& Actor, TArray<uint8>& Data,
    int64 Start, int64 Size);
