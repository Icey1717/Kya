#pragma once
#include "CoreMinimal.h"
struct FKyaActorPlacement;
struct KYALEVEL_API FKyaAmortosCmdOffsets
{
    int32 Count = 0;
    int64 Entries = 0, Tail = 0;
};
KYALEVEL_API bool KyaReadAmortosCmdOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size, FKyaAmortosCmdOffsets& Out);
KYALEVEL_API bool KyaReadAmortosCmdActor(FKyaActorPlacement& Actor, TConstArrayView<uint8> Data, int64 Start, int64 Size);
KYALEVEL_API bool KyaWriteAmortosCmdActor(const FKyaActorPlacement& Actor, TArray<uint8>& Data, int64 Start, int64 Size);
