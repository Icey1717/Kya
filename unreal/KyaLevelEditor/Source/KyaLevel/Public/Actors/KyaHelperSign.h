#pragma once
#include "CoreMinimal.h"
struct FKyaActorPlacement;
struct FKyaActorBehaviour;
// CActorHelperSign::Create: s32 field_168, u64 field_160, no alignment padding.
KYALEVEL_API bool KyaReadHelperSignActor(FKyaActorPlacement& Actor, TConstArrayView<uint8> Data, int64 Start, int64 Size);
KYALEVEL_API bool KyaWriteHelperSignActor(const FKyaActorPlacement& Actor, TArray<uint8>& Data, int64 Start, int64 Size);
KYALEVEL_API bool KyaReadHelperSignBehaviour(FKyaActorBehaviour& Behaviour, TConstArrayView<uint8> Data, int64 Start, int64 Size);
KYALEVEL_API bool KyaWriteHelperSignBehaviour(const FKyaActorBehaviour& Behaviour, TArray<uint8>& Data, int64 Start, int64 Size);
