#pragma once
#include "CoreMinimal.h"
struct FKyaActorPlacement;
KYALEVEL_API bool KyaReadCareBoyActor(FKyaActorPlacement& Actor, TConstArrayView<uint8> Data, int64 Start, int64 Size);
KYALEVEL_API bool KyaWriteCareBoyActor(const FKyaActorPlacement& Actor, TArray<uint8>& Data, int64 Start, int64 Size);
