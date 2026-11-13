#pragma once

#include "CoreMinimal.h"

struct FKyaActorPlacement;
struct FKyaActorBehaviour;

/**
 * CActorFx (class 0x2E) serialized codec.
 *
 * CActorFx::Create calls CActor::Create and reads two floats (field 0x19C,
 * clamped to 0..1, and field 0x1A0). BuildBehaviour maps 2 Single (effect id),
 * 3 Path (effect id + path index) and 4 Unknown (count + ids); ids 0/1 are
 * inherited.
 */
struct KYALEVEL_API FKyaFxOffsets
{
    int64 Field19C = 0, Field1A0 = 0, Tail = 0;
};

KYALEVEL_API bool KyaReadFxOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size,
    FKyaFxOffsets& Out);
KYALEVEL_API bool KyaReadFxActor(FKyaActorPlacement& Out, TConstArrayView<uint8> Data,
    int64 Start, int64 Size);
KYALEVEL_API bool KyaWriteFxActor(const FKyaActorPlacement& Actor, TArray<uint8>& Data,
    int64 Start, int64 Size);

struct KYALEVEL_API FKyaFxBehaviourValues
{
    int32 Value = 0;
    int32 PathFollowIndex = -1;
    TArray<int32> UnknownValues;
};

KYALEVEL_API bool KyaReadFxBehaviour(int32 Id, TConstArrayView<uint8> Data, int64 Start,
    int64 Size, FKyaFxBehaviourValues& Out);
KYALEVEL_API bool KyaReadFxBehaviourValue(FKyaActorBehaviour& Out, int32 Id,
    TConstArrayView<uint8> Data, int64 Start, int32 Size);
KYALEVEL_API bool KyaWriteFxBehaviour(const FKyaActorBehaviour& Behaviour,
    TArray<uint8>& Data, int64 Start, int32 Size);
