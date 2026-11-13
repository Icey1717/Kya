#pragma once
#include "CoreMinimal.h"
struct FKyaActorPlacement;
struct KYALEVEL_API FKyaBombLauncherOffsets
{
    int32 Count = 0;
    int64 Entries = 0, Tail = 0;
};
KYALEVEL_API bool KyaReadBombLauncherOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size, FKyaBombLauncherOffsets& Out);
KYALEVEL_API bool KyaReadBombLauncherActor(FKyaActorPlacement& Actor, TConstArrayView<uint8> Data, int64 Start, int64 Size);
KYALEVEL_API bool KyaWriteBombLauncherActor(const FKyaActorPlacement& Actor, TArray<uint8>& Data, int64 Start, int64 Size);
