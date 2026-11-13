#pragma once
#include "CoreMinimal.h"
struct FKyaActorBehaviour;
struct FKyaActorPlacement;
struct KYALEVEL_API FKyaJamGutOffsets
{
    int64 Suffix = 0, Checkpoints = 0, Paths = 0, Type = 0, Tail = 0;
    int32 CheckpointCount = 0, PathCount = 0;
    bool bHasField41C = false;
};
KYALEVEL_API bool KyaReadJamGutOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size, FKyaJamGutOffsets& Out);
KYALEVEL_API bool KyaReadJamGutActor(FKyaActorPlacement& Actor, TConstArrayView<uint8> Data, int64 Start, int64 Size);
KYALEVEL_API bool KyaWriteJamGutActor(const FKyaActorPlacement& Actor, TArray<uint8>& Data, int64 Start, int64 Size);
KYALEVEL_API bool KyaReadJamGutBehaviour(FKyaActorBehaviour& Out, TConstArrayView<uint8> Data, int64 Start, int64 Size);
KYALEVEL_API bool KyaWriteJamGutBehaviour(const FKyaActorBehaviour& Behaviour, TArray<uint8>& Data, int64 Start, int64 Size);
