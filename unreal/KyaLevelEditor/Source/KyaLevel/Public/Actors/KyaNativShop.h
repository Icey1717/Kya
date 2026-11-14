#pragma once

#include "CoreMinimal.h"

struct FKyaActorPlacement;
struct FKyaActorBehaviour;

/**
 * CActorNativShop (class 0x34) serialized codec.
 *
 * CActorNativShop::Create calls CActor::Create, reads a radius f32, a class
 * name string (ByteCode::GetString -> NUL terminated, seek aligned to 4), one
 * S_ACTOR_STREAM_REF actor list, an episode threshold link u32, field 0x174
 * u32 and one actor reference s32. BuildBehaviour only adds id 2
 * (CBehaviourNativShopSell); ids 0/1 are inherited. Behaviour 2 reads a camera
 * reference, five sub-object actor references, a CPathFollowReader path index,
 * a material id, a music index, a zone reference and three floats (52 bytes).
 */
struct KYALEVEL_API FKyaNativShopOffsets
{
    int64 Radius = 0, StringStart = 0, StringEnd = 0;
    int64 ActorCount = 0, ActorEntries = 0;
    int64 EpisodeThresholdLink = 0, Field174 = 0, ActorRef = 0, Tail = 0;
    int32 ActorCountValue = 0;
};

KYALEVEL_API bool KyaReadNativShopOffsets(TConstArrayView<uint8> Data, int64 Start,
    int64 Size, FKyaNativShopOffsets& Out);
KYALEVEL_API bool KyaReadNativShopActor(FKyaActorPlacement& Out,
    TConstArrayView<uint8> Data, int64 Start, int64 Size);
KYALEVEL_API bool KyaWriteNativShopActor(const FKyaActorPlacement& Actor,
    TArray<uint8>& Data, int64 Start, int64 Size);

struct KYALEVEL_API FKyaNativShopSellOffsets
{
    int64 Camera = 0, ActorRefs = 0, PathIndex = 0, MaterialId = 0, MusicIndex = 0;
    int64 ZoneRef = 0, Floats = 0;
};

KYALEVEL_API bool KyaReadNativShopBehaviour(FKyaActorBehaviour& Out, int32 Id,
    TConstArrayView<uint8> Data, int64 Start, int32 Size);
KYALEVEL_API bool KyaWriteNativShopBehaviour(const FKyaActorBehaviour& Behaviour,
    TArray<uint8>& Data, int64 Start, int32 Size);
