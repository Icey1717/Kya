#pragma once
#include "CoreMinimal.h"
struct FKyaActorPlacement;
struct KYALEVEL_API FKyaMiniGamesSupportOffsets
{
    int64 NameEnd = 0, Scalars = 0, Entries = 0, Suffix = 0, Tail = 0;
    int32 Count = 0;
};
KYALEVEL_API bool KyaReadMiniGamesSupportOffsets(int64 ClassId, TConstArrayView<uint8> Data, int64 Start, int64 Size, FKyaMiniGamesSupportOffsets& Out);
KYALEVEL_API bool KyaReadMiniGamesSupportActor(FKyaActorPlacement& Actor, TConstArrayView<uint8> Data, int64 Start, int64 Size);
KYALEVEL_API bool KyaWriteMiniGamesSupportActor(const FKyaActorPlacement& Actor, TArray<uint8>& Data, int64 Start, int64 Size);
