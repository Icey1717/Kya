#pragma once
#include "CoreMinimal.h"
struct FKyaActorBehaviour;
KYALEVEL_API bool KyaReadNativAkasaBehaviour(FKyaActorBehaviour& Out, TConstArrayView<uint8> Data, int64 Start, int64 Size);
KYALEVEL_API bool KyaWriteNativAkasaBehaviour(const FKyaActorBehaviour& Behaviour, TArray<uint8>& Data, int64 Start, int64 Size);
