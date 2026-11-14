#pragma once

#include "CoreMinimal.h"
#include "Actors/KyaSwitch.h"

struct FKyaActorPlacement;
struct FKyaActorBehaviour;

/**
 * Minigame actor family codec: CActorMiniGame (0x44), TimeAttack (0x45),
 * BoxCounter (0x46), Boomy (0x47) and Distance (0x55).
 *
 * The shared CActorMiniGame block is typed, then the per-class tail:
 * TimeAttack adds one s32, BoxCounter one f32, Distance two CPathFollowReader
 * indices and Boomy its target/penalty/sequence lists. Score names use
 * ByteCode::GetString (NUL terminated, seek aligned to 4).
 *
 * BuildBehaviour maps id 2 (training, empty), 3 (betting: count + count x
 * {cost,reward}) and 4 (multi, empty); the variant behaviour classes add no
 * Create bodies. Ids 0/1 are inherited.
 */
struct KYALEVEL_API FKyaMiniGameOffsets
{
    int64 Field160 = 0, Field168 = 0, Field16C = 0;
    int64 ActorCount = 0, ActorEntries = 0;
    int64 FloatCount = 0, Floats = 0;
    int64 ScoreCount = 0, Scores = 0;
    int64 DefaultScore = -1, DefaultNameStart = 0, DefaultNameEnd = 0;
    int64 WayPointRef = 0, Field190 = 0, Field1A8 = 0;
    int64 SwitchTargets = 0, SwitchCamera = 0;
    int64 VariantStart = 0, Tail = 0;
    int32 ActorCountValue = 0, FloatCountValue = 0, ScoreCountValue = 0, SwitchCountValue = 0;

    TArray<int64> ScoreValues;
    TArray<int64> ScoreNameStart;
    TArray<int64> ScoreNameEnd;

    // 0x45 TimeAttack.
    int64 TimeAttackField1E0 = 0;
    // 0x46 BoxCounter / 0x47 Boomy.
    int64 TimeLimit = 0;
    // 0x55 Distance.
    int64 PathA = 0, PathB = 0;
    // 0x47 Boomy.
    int64 TargetCount = 0, Targeting = 0, PenaltyCount = 0, Penalties = 0;
    int64 MaxPenalties = 0, GameFlags = 0, SequenceCount = 0;
    int32 TargetCountValue = 0, PenaltyCountValue = 0, SequenceCountValue = 0;
    TArray<int64> TargetHeaders;
    TArray<int64> PenaltyHeaders;
    TArray<int64> SequenceHeaders;
    TArray<int32> SequenceHeaderCounts;
    TArray<int64> SequenceValues;
};

KYALEVEL_API bool KyaReadMiniGameOffsets(int64 ClassId, TConstArrayView<uint8> Data,
    int64 Start, int64 Size, FKyaMiniGameOffsets& Out);
KYALEVEL_API bool KyaReadMiniGameActor(FKyaActorPlacement& Out,
    TConstArrayView<uint8> Data, int64 Start, int64 Size);
KYALEVEL_API bool KyaWriteMiniGameActor(const FKyaActorPlacement& Actor,
    TArray<uint8>& Data, int64 Start, int64 Size);

struct KYALEVEL_API FKyaMiniGameBetOffsets
{
    int64 Count = 0, Entries = 0;
    int32 CountValue = 0;
};

KYALEVEL_API bool KyaReadMiniGameBehaviour(FKyaActorBehaviour& Out, int32 Id,
    TConstArrayView<uint8> Data, int64 Start, int32 Size);
KYALEVEL_API bool KyaWriteMiniGameBehaviour(const FKyaActorBehaviour& Behaviour,
    TArray<uint8>& Data, int64 Start, int32 Size);
