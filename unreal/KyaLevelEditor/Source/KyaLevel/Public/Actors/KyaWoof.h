#pragma once
#include "CoreMinimal.h"
struct FKyaActorPlacement;
struct KYALEVEL_API FKyaWoofOffsets
{
    int32 Count = 0;
    int64 Entries = 0, Suffix = 0, Tail = 0;
};
KYALEVEL_API bool KyaReadWoofOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size, FKyaWoofOffsets& Out);
KYALEVEL_API bool KyaReadWoofActor(FKyaActorPlacement& Actor, TConstArrayView<uint8> Data, int64 Start, int64 Size);
KYALEVEL_API bool KyaWriteWoofActor(const FKyaActorPlacement& Actor, TArray<uint8>& Data, int64 Start, int64 Size);
