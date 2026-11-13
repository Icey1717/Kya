#pragma once
#include "CoreMinimal.h"
struct FKyaActorPlacement;
struct FKyaWantedZooEntryOffsets
{
    int64 Conditions = 0, Keys = 0, Numbers = 0;
    int32 ConditionCount = 0, KeyCount = 0, NumberCount = 0;
};
struct KYALEVEL_API FKyaWantedZooOffsets
{
    int64 TextPositions = 0, NumberPositions = 0, Tail = 0;
    int32 TextCount = 0, NumberCount = 0;
    TArray<FKyaWantedZooEntryOffsets> Entries;
};
KYALEVEL_API bool KyaReadWantedZooOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size, FKyaWantedZooOffsets& Out);
KYALEVEL_API bool KyaReadWantedZooActor(FKyaActorPlacement& Actor, TConstArrayView<uint8> Data, int64 Start, int64 Size);
KYALEVEL_API bool KyaWriteWantedZooActor(const FKyaActorPlacement& Actor, TArray<uint8>& Data, int64 Start, int64 Size);
