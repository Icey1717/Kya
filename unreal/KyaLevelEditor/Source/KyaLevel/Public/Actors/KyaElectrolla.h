#pragma once
#include "CoreMinimal.h"
#include "Actors/KyaSwitch.h"
struct FKyaActorPlacement;
struct KYALEVEL_API FKyaElectrollaOffsets
{
    FKyaSwitchActorOffsets Switch;
    int64 Tail = 0;
};
KYALEVEL_API bool KyaReadElectrollaOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size, FKyaElectrollaOffsets& Out);
KYALEVEL_API bool KyaReadElectrollaActor(FKyaActorPlacement& Actor, TConstArrayView<uint8> Data, int64 Start, int64 Size);
KYALEVEL_API bool KyaWriteElectrollaActor(const FKyaActorPlacement& Actor, TArray<uint8>& Data, int64 Start, int64 Size);
