#pragma once

#include "CoreMinimal.h"

struct FKyaActorPlacement;

/**
 * CActorTeleporter (class 0x24) serialized codec.
 *
 * CActorTeleporter::Create calls CActor::Create, then reads a subsector
 * material id, a 64-bit hash, a word, an S_DESTINATION_LIST, three
 * S_NTF_TARGET_STREAM_REF switch records, four zone/material/cinematic words,
 * two more material words, an ACTOR_WAYPOINT_LIST and a conditioned-operation
 * array. BuildBehaviour only adds id 2 (CBehaviourTeleporterDefault), whose
 * Create is empty; ids 0/1 are inherited.
 */
struct KYALEVEL_API FKyaTeleporterSwitchOffsets
{
    int64 Targets = 0;
    int64 Camera = 0;
    int32 Count = 0;
};

struct KYALEVEL_API FKyaTeleporterOffsets
{
    int64 SubsectorMaterialId = 0, Hash = 0, Field16C = 0;
    int64 DestCount = 0, DestEntries = 0;
    int32 DestCountValue = 0;
    FKyaTeleporterSwitchOffsets Switches[3];
    int64 Zone188 = 0, Zone18C = 0, Material1 = 0, CinematicId = 0, Material2 = 0;
    int64 Field1D4 = 0, Field1D8 = 0, Material3 = 0, Field1E0 = 0;
    int64 WaypointCount = 0, WaypointEntries = 0;
    int32 WaypointCountValue = 0;
    int64 CndOpCount = 0;
    TArray<int64> BlockOpCounts, BlockOps, BlockCondCounts, BlockConds;
    TArray<int32> BlockOpCountValues, BlockCondCountValues;
    int64 Tail = 0;
};

KYALEVEL_API bool KyaReadTeleporterOffsets(TConstArrayView<uint8> Data, int64 Start,
    int64 Size, FKyaTeleporterOffsets& Out);
KYALEVEL_API bool KyaReadTeleporterActor(FKyaActorPlacement& Out, TConstArrayView<uint8> Data,
    int64 Start, int64 Size);
KYALEVEL_API bool KyaWriteTeleporterActor(const FKyaActorPlacement& Actor, TArray<uint8>& Data,
    int64 Start, int64 Size);
