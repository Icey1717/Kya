#pragma once
#include "CoreMinimal.h"
#include "Actors/KyaSwitch.h"
struct FKyaActorPlacement;
struct KYALEVEL_API FKyaCreditsOffsets
{
    int32 Count = 0;
    int64 Entries = 0, Values = 0, Tail = 0;
    FKyaSwitchActorOffsets Switch;
};
KYALEVEL_API bool KyaReadCreditsOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size, FKyaCreditsOffsets& Out);
KYALEVEL_API bool KyaReadCreditsActor(FKyaActorPlacement& Actor, TConstArrayView<uint8> Data, int64 Start, int64 Size);
KYALEVEL_API bool KyaWriteCreditsActor(const FKyaActorPlacement& Actor, TArray<uint8>& Data, int64 Start, int64 Size);
