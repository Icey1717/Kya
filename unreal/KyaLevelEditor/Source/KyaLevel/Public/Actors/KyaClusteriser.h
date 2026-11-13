#pragma once

#include "CoreMinimal.h"

struct FKyaActorPlacement;
struct FKyaActorBehaviour;

/**
 * CActorClusteriser (class 0x18) behaviour codec.
 *
 * CActorClusteriser::Create only calls CActor::Create (no class payload).
 * BuildBehaviour only adds id 2 (CBehaviourClusteriserZones), a
 * CStreamClusterZoneInternal: count s32 + count x _S_ZONE_CLUSTER records. Each
 * record stores flags/zone ref, two 4-byte actor streams, a camera stream, a
 * light stream, an 8-byte hash stream and a trailing zone ref. Ids 0/1 are
 * inherited from CActor.
 */
struct KYALEVEL_API FKyaClusterZoneOffsets
{
    int64 Header = 0, Lists[5] = {}, Tail = 0;
    int32 Counts[5] = {};
};

KYALEVEL_API bool KyaReadClusterZoneOffsets(TConstArrayView<uint8> Data, int64 Start,
    int64 Size, TArray<FKyaClusterZoneOffsets>& OutZones);
KYALEVEL_API bool KyaReadClusteriserBehaviour(FKyaActorBehaviour& Out, int32 Id,
    TConstArrayView<uint8> Data, int64 Start, int32 Size);
KYALEVEL_API bool KyaWriteClusteriserBehaviour(const FKyaActorBehaviour& Behaviour,
    TArray<uint8>& Data, int64 Start, int32 Size);
