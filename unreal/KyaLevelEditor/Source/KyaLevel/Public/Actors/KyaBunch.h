#pragma once

#include "CoreMinimal.h"

struct FKyaActorPlacement;
struct FKyaActorBehaviour;

/**
 * CActorBunch (class 0x4E) serialized codec.
 *
 * The CActorWolfen prefix is parsed only to locate its end (its bytes stay
 * untouched); the Bunch block that follows is typed:
 * field 0xFC8 f32, a list of S_BUNCH_STREAM_ENTRY records, and two mesh ids.
 *
 * BuildBehaviour adds ids 3/4 (fighter default/projected, both empty), 26
 * (CBehaviourBunchRepel) and 27 (CBehaviourBunchFence); ids 0/1/2 and the
 * Wolfen behaviour ids (8/10/13/14/15/17/18/19/21/22/23/24) are inherited.
 */
struct KYALEVEL_API FKyaBunchStreamEntryOffsets
{
    int64 Field0 = 0, HasBehaviour = 0, BehaviourId = 0, ZoneId = 0, FieldC = 0;
    int32 BehaviourIdValue = -1;
};

struct KYALEVEL_API FKyaBunchOffsets
{
    int64 WolfenEnd = 0, FieldFC8 = 0, EntryCount = 0, Entries = 0;
    int64 MeshComponent = 0, MeshIndex = 0, Tail = 0;
    int32 EntryCountValue = 0;
    TArray<FKyaBunchStreamEntryOffsets> EntriesLayout;
};

KYALEVEL_API bool KyaReadBunchOffsets(TConstArrayView<uint8> Data, int64 Start,
    int64 Size, FKyaBunchOffsets& Out);
KYALEVEL_API bool KyaReadBunchActor(FKyaActorPlacement& Out,
    TConstArrayView<uint8> Data, int64 Start, int64 Size);
KYALEVEL_API bool KyaWriteBunchActor(const FKyaActorPlacement& Actor,
    TArray<uint8>& Data, int64 Start, int64 Size);

struct KYALEVEL_API FKyaBunchRepelOffsets
{
    int64 WayPointRef = 0, ShockWaveCount = 0, ShockWaveFloats = 0;
    int64 IndexCount = 0, Indices = 0;
    int64 MsgHit = 0, Vector20 = 0, Vector30 = 0;
    int64 MeshComponent = 0, MeshIndex = 0, MeshType = 0, Reserved = 0, DefaultId = 0;
    int64 Tail = 0;
    int32 ShockWaveCountValue = 0, IndexCountValue = 0;
};

struct KYALEVEL_API FKyaBunchFenceEntryOffsets
{
    int64 Ref = 0, Field4 = 0;
};

struct KYALEVEL_API FKyaBunchFenceGroupOffsets
{
    int64 Header = 0;
    int32 EntryCountValue = 0;
    TArray<FKyaBunchFenceEntryOffsets> Entries;
};

struct KYALEVEL_API FKyaBunchFenceOffsets
{
    int64 ActorRef = 0, WayPointRef = 0;
    int64 WayPointCount = 0, WayPoints = 0;
    int64 FenceCount = 0;
    TArray<FKyaBunchFenceGroupOffsets> Groups;
    int64 MeshF0 = 0, Mesh70 = 0, FxTypes = 0, Tail = 0;
    int32 WayPointCountValue = 0, FenceCountValue = 0;
};

KYALEVEL_API bool KyaReadBunchBehaviour(FKyaActorBehaviour& Out, int32 Id,
    TConstArrayView<uint8> Data, int64 Start, int32 Size);
KYALEVEL_API bool KyaWriteBunchBehaviour(const FKyaActorBehaviour& Behaviour,
    TArray<uint8>& Data, int64 Start, int32 Size);
