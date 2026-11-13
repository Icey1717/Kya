#include "KyaActorFamily.h"

/**
 * Recovered metadata and codec registration for Rope (class 0x0f).
 *
 * The class payload codec lives in Actors/KyaRope.h/.cpp. BuildBehaviour only
 * adds id 2 (CBehaviourRopeStand), whose Create is empty; ids 0/1 are inherited.
 */
namespace
{
const FKyaActorFamilyBehaviour RopeBehaviours[] = {
    {0, TEXT("Stand")},
    {1, TEXT("Cinematic")},
    {2, TEXT("Rope stand")},
};
const FKyaActorFamilyReference RopeReferences[] = {
    {TEXT("RopeActorRefA"), EKyaReferenceKind::Actor},
    {TEXT("RopeActorRefB"), EKyaReferenceKind::Actor},
    {TEXT("RopePathIndex"), EKyaReferenceKind::Path},
};
}

extern const FKyaActorFamily GKyaRopeActorFamily = {
    0x0f, TEXT("CActorRope"), TEXT("Rope"),
    TEXT("path/scale/material tuning, actor references, colour/sound and switch record"),
    MakeArrayView(RopeBehaviours),
    MakeArrayView(RopeReferences),
};
