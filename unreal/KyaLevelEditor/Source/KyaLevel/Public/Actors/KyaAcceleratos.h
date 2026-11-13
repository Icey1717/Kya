#pragma once
#include "CoreMinimal.h"
struct FKyaActorPlacement;
// Packed S_ACCELERATOS_CONFIG: three floats followed by two signed sound references.
KYALEVEL_API bool KyaReadAcceleratosActor(FKyaActorPlacement& Actor, TConstArrayView<uint8> Data, int64 Start, int64 Size);
KYALEVEL_API bool KyaWriteAcceleratosActor(const FKyaActorPlacement& Actor, TArray<uint8>& Data, int64 Start, int64 Size);
