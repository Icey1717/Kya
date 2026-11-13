#pragma once
#include "CoreMinimal.h"
struct FKyaActorPlacement;
struct KYALEVEL_API FKyaShockerCmdOffsets
{
    TArray<int64> Sequences;
    TArray<int32> Counts;
    int64 Tail = 0;
};
KYALEVEL_API bool KyaReadShockerCmdOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size, FKyaShockerCmdOffsets& Out);
KYALEVEL_API bool KyaReadShockerCmdActor(FKyaActorPlacement& Actor, TConstArrayView<uint8> Data, int64 Start, int64 Size);
KYALEVEL_API bool KyaWriteShockerCmdActor(const FKyaActorPlacement& Actor, TArray<uint8>& Data, int64 Start, int64 Size);
