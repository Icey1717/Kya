#pragma once

#include "CoreMinimal.h"

struct FKyaActorPlacement;
struct FKyaActorBehaviour;

/**
 * CActorRope (class 0x0f) serialized codec.
 *
 * CActorRope::Create calls CActor::Create, reads a path index, rope scale and
 * field 0x17C, material id, packed colour, sound reference, field 0x194, two
 * actor-stream indices, one S_NTF_TARGET_STREAM_REF switch record and the rope
 * flags. BuildBehaviour only adds id 2 (CBehaviourRopeStand), whose Create is
 * empty; ids 0/1 are inherited.
 */
struct KYALEVEL_API FKyaRopeSwitchOffsets
{
    int64 Targets = 0;
    int64 Camera = 0;
    int32 Count = 0;
};

struct KYALEVEL_API FKyaRopeOffsets
{
    int64 Path = 0, Scale = 0, Field17C = 0, Material = 0, Color = 0, Sound = 0, Field194 = 0;
    int64 ActorA = 0, ActorB = 0;
    FKyaRopeSwitchOffsets Switch;
    int64 Flags = 0, Tail = 0;
};

KYALEVEL_API bool KyaReadRopeOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size,
    FKyaRopeOffsets& Out);
KYALEVEL_API bool KyaReadRopeActor(FKyaActorPlacement& Out, TConstArrayView<uint8> Data,
    int64 Start, int64 Size);
KYALEVEL_API bool KyaWriteRopeActor(const FKyaActorPlacement& Actor, TArray<uint8>& Data,
    int64 Start, int64 Size);
