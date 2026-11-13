#pragma once
#include "CoreMinimal.h"
struct FKyaActorPlacement;
struct KYALEVEL_API FKyaPatternCmdOffsets
{
    int32 Count = 0;
    int64 Entries = 0, Values = 0, Pattern = 0, Tail = 0;
    bool bHasField24 = false;
};
KYALEVEL_API bool KyaReadPatternCmdOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size, FKyaPatternCmdOffsets& Out);
KYALEVEL_API bool KyaReadPatternCmdActor(FKyaActorPlacement& Actor, TConstArrayView<uint8> Data, int64 Start, int64 Size);
KYALEVEL_API bool KyaWritePatternCmdActor(const FKyaActorPlacement& Actor, TArray<uint8>& Data, int64 Start, int64 Size);
