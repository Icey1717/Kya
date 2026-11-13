#pragma once

#include "CoreMinimal.h"

struct FKyaActorPlacement;
struct FKyaActorBehaviour;

/**
 * CActorFogManager (class 0x3D) serialized codec.
 *
 * CActorFogManager::Create calls CActor::Create, then reads a fog-hierarchy count
 * and per hierarchy a CSectorHierarchy hash (u64) plus two floats. BuildBehaviour
 * only adds id 2 (CBehaviourFogManagerZones), a CStreamFogZoneInternal: count u32
 * + count x 28-byte fog zones.
 */
struct KYALEVEL_API FKyaFogOffsets
{
    int64 Count = 0, Entries = 0, Tail = 0;
    int32 CountValue = 0;
};

KYALEVEL_API bool KyaReadFogOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size,
    FKyaFogOffsets& Out);
KYALEVEL_API bool KyaReadFogActor(FKyaActorPlacement& Out, TConstArrayView<uint8> Data,
    int64 Start, int64 Size);
KYALEVEL_API bool KyaWriteFogActor(const FKyaActorPlacement& Actor, TArray<uint8>& Data,
    int64 Start, int64 Size);

struct KYALEVEL_API FKyaFogZoneOffsets
{
    int64 Count = 0, Entries = 0, Tail = 0;
    int32 CountValue = 0;
};

KYALEVEL_API bool KyaReadFogZoneOffsets(TConstArrayView<uint8> Data, int64 Start, int64 End,
    FKyaFogZoneOffsets& Out);
KYALEVEL_API bool KyaReadFogBehaviour(FKyaActorBehaviour& Out, int32 Id,
    TConstArrayView<uint8> Data, int64 Start, int32 Size);
KYALEVEL_API bool KyaWriteFogBehaviour(const FKyaActorBehaviour& Behaviour,
    TArray<uint8>& Data, int64 Start, int32 Size);
