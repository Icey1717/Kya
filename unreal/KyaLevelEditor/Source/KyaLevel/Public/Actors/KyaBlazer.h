#pragma once
#include "CoreMinimal.h"
struct FKyaActorPlacement;
struct KYALEVEL_API FKyaBlazerOffsets
{
    int64 Suffix = 0, Entries = 0, Tilt = 0, Tail = 0;
    int32 Count = 0;
    bool bHasWaypointRefs = false;
};
KYALEVEL_API bool KyaReadBlazerOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size, FKyaBlazerOffsets& Out);
KYALEVEL_API bool KyaReadBlazerActor(FKyaActorPlacement& Actor, TConstArrayView<uint8> Data, int64 Start, int64 Size);
KYALEVEL_API bool KyaWriteBlazerActor(const FKyaActorPlacement& Actor, TArray<uint8>& Data, int64 Start, int64 Size);
