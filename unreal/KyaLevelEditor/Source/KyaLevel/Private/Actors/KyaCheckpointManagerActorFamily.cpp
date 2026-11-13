#include "KyaActorFamily.h"

/**
 * Recovered metadata and codec registration for Checkpoint manager (class 0x1E).
 *
 * The class payload codec lives in Actors/KyaCheckpoints.h/.cpp. BuildBehaviour
 * only adds id 2 (CBehaviourCheckpoint), whose Create is empty; ids 0/1 are
 * inherited.
 */
namespace
{
const FKyaActorFamilyBehaviour CheckpointManagerBehaviours[] = {
    {0, TEXT("Stand")},
    {1, TEXT("Cinematic")},
    {2, TEXT("Checkpoint")},
};
}

extern const FKyaActorFamily GKyaCheckpointManagerActorFamily = {
    0x1e, TEXT("CActorCheckpointManager"), TEXT("Checkpoint manager"),
    TEXT("checkpoint records: zone/waypoint/sector pairs with actor-waypoint lists"),
    MakeArrayView(CheckpointManagerBehaviours),
    TConstArrayView<FKyaActorFamilyReference>(),
};
