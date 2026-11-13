#pragma once
#include "CoreMinimal.h"
struct FKyaActorPlacement;
struct KYALEVEL_API FKyaEggOffsets
{
    int32 Count = 0;
    int64 Entries = 0, Fields = 0, Tail = 0;
};
KYALEVEL_API bool KyaReadEggOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size, FKyaEggOffsets& Out);
KYALEVEL_API bool KyaReadEggActor(FKyaActorPlacement& Actor, TConstArrayView<uint8> Data, int64 Start, int64 Size);
KYALEVEL_API bool KyaWriteEggActor(const FKyaActorPlacement& Actor, TArray<uint8>& Data, int64 Start, int64 Size);
