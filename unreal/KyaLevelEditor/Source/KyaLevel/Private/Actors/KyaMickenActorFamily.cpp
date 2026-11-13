#include "KyaActorFamily.h"

/**
 * Recovered metadata and codec registration for Micken (class 0x07).
 *
 * The class and behaviour 4 codec live in Actors/KyaMicken.h/.cpp.
 * BuildBehaviour maps 4 Eat, 5/6 Kicked and 7/8 Squashed (empty); ids 0/1/2 are
 * inherited (id 2 is the shared catch-by-trap payload).
 */
namespace
{
const FKyaActorFamilyBehaviour MickenBehaviours[] = {
    {0, TEXT("Stand")},
    {1, TEXT("Cinematic")},
    {2, TEXT("Catch by trap")},
    {4, TEXT("Micken eat")},
    {5, TEXT("Micken squashed")},
    {6, TEXT("Micken kicked")},
    {7, TEXT("Micken squashed")},
    {8, TEXT("Micken kicked")},
};
const FKyaActorFamilyReference MickenReferences[] = {
    {TEXT("MickenActorIndex"), EKyaReferenceKind::Actor},
    {TEXT("MickenWaypointIndex"), EKyaReferenceKind::Waypoint},
};
}

extern const FKyaActorFamily GKyaMickenActorFamily = {
    0x07, TEXT("CActorMicken"), TEXT("Micken"),
    TEXT("autonomous values, waypoint/actor references and the eat behaviour"),
    MakeArrayView(MickenBehaviours),
    MakeArrayView(MickenReferences),
};
