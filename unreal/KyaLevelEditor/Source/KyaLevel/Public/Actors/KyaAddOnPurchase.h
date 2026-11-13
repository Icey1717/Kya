#pragma once
#include "CoreMinimal.h"
struct FKyaActorPlacement;
struct KYALEVEL_API FKyaAddOnPurchaseOffsets
{
    bool bPresent = false;
    bool bHasObjectFlag = false;
    bool bHasObject = false;
    int64 Tail = 0;
};
// CInventoryInfo alone: 4 absent, 36 pre-2.27, 40 modern/no object, 52 modern/object.
// Exact record sizes disambiguate the scene-version gate; unknown shapes stay raw.
KYALEVEL_API bool KyaReadAddOnPurchaseOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size, FKyaAddOnPurchaseOffsets& Out);
KYALEVEL_API bool KyaReadAddOnPurchaseActor(FKyaActorPlacement& Actor, TConstArrayView<uint8> Data, int64 Start, int64 Size);
KYALEVEL_API bool KyaWriteAddOnPurchaseActor(const FKyaActorPlacement& Actor, TArray<uint8>& Data, int64 Start, int64 Size);
