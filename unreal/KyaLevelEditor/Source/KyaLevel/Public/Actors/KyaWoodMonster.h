#pragma once
#include "CoreMinimal.h"
struct FKyaActorPlacement;
struct FKyaActorBehaviour;
KYALEVEL_API bool KyaReadWoodMonsterActor(FKyaActorPlacement& Actor, TConstArrayView<uint8> Data, int64 Start, int64 Size);
KYALEVEL_API bool KyaWriteWoodMonsterActor(const FKyaActorPlacement& Actor, TArray<uint8>& Data, int64 Start, int64 Size);
KYALEVEL_API bool KyaReadWoodMonsterBehaviour(FKyaActorBehaviour& Behaviour, TConstArrayView<uint8> Data, int64 Start, int64 Size);
KYALEVEL_API bool KyaWriteWoodMonsterBehaviour(const FKyaActorBehaviour& Behaviour, TArray<uint8>& Data, int64 Start, int64 Size);
