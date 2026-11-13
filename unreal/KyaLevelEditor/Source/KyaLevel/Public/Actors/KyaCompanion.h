#pragma once

#include "CoreMinimal.h"
#include "Actors/KyaScenarioCondition.h"

struct FKyaActorPlacement;
struct FKyaActorBehaviour;

/**
 * CActorCompanion (class 0x19) serialized codec.
 *
 * CActorCompanion::Create calls CActorAutonomous::Create, then reads three words
 * (field 0x358, material 0x35C, field 0x360). BuildBehaviour only adds id 3
 * (CBehaviourCompanionAlert) with a count + alert records; ids 0/1/2 are
 * inherited.
 */
struct KYALEVEL_API FKyaCompanionOffsets
{
    int64 WeightB = 0, Autonomous = 0, Field358 = 0, Tail = 0;
};

KYALEVEL_API bool KyaReadCompanionOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size,
    FKyaCompanionOffsets& Out);
KYALEVEL_API bool KyaReadCompanionActor(FKyaActorPlacement& Out, TConstArrayView<uint8> Data,
    int64 Start, int64 Size);
KYALEVEL_API bool KyaWriteCompanionActor(const FKyaActorPlacement& Actor, TArray<uint8>& Data,
    int64 Start, int64 Size);

struct KYALEVEL_API FKyaCompanionAlertOffsets
{
    FKyaScenarioConditionOffsets Conditions;
    int64 Flags = 0, Field08 = 0, Field10 = 0, Field14 = 0, Field18 = 0;
    int64 ActorRef = 0, Vector20 = 0, CameraRef = 0;
    int64 ListCount = 0, ListEntries = 0;
    int32 ListCountValue = 0;
    int64 LevelStringKey = 0, Field50 = 0, Field58 = 0, Field5C = 0;
    struct
    {
        int64 Targets = 0, Camera = 0;
        int32 Count = 0;
    } Switch;
    int64 Conditioned = 0;
    TArray<FKyaConditionedOffsets> Blocks;
    int64 Tail = 0;
};

struct KYALEVEL_API FKyaCompanionBehaviourOffsets
{
    int64 Count = 0, Entries = 0, Tail = 0;
    int32 CountValue = 0;
    TArray<FKyaCompanionAlertOffsets> Alerts;
};

KYALEVEL_API bool KyaReadCompanionBehaviourOffsets(TConstArrayView<uint8> Data, int64 Start,
    int64 Size, FKyaCompanionBehaviourOffsets& Out);
KYALEVEL_API bool KyaReadCompanionBehaviour(FKyaActorBehaviour& Out, int32 Id,
    TConstArrayView<uint8> Data, int64 Start, int32 Size);
KYALEVEL_API bool KyaWriteCompanionBehaviour(const FKyaActorBehaviour& Behaviour,
    TArray<uint8>& Data, int64 Start, int32 Size);
