#pragma once
#include "CoreMinimal.h"
struct FKyaActorPlacement;
// Source: CActorHunter::Create -> CActorAutonomous::Create -> CActorMovable::Create.
// Five base words followed by bone id, field 354, actor ref, field 3A4 and field 350.
KYALEVEL_API bool KyaReadHunterActor(FKyaActorPlacement& Actor, TConstArrayView<uint8> Data, int64 Start, int64 Size);
KYALEVEL_API bool KyaWriteHunterActor(const FKyaActorPlacement& Actor, TArray<uint8>& Data, int64 Start, int64 Size);
