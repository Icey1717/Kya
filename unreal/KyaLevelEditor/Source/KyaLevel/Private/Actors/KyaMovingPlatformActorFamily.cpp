#include "KyaActorFamily.h"

/**
 * Recovered metadata and codec registration for Moving platform (class 0x0d).
 *
 * The class payload and behaviour 2-10 readers and writers live in
 * Actors/KyaMovingPlatform.h/.cpp. Ids 0 (Stand) and 1 (Cinematic) are inherited
 * from CActor.
 */
namespace
{
const FKyaActorFamilyBehaviour MovingPlatformBehaviours[] = {
    {0, TEXT("Stand")},
    {1, TEXT("Cinematic")},
    {2, TEXT("Trajectory")},
    {3, TEXT("Slab")},
    {4, TEXT("Weighing machine slave")},
    {5, TEXT("Weighing machine master")},
    {6, TEXT("Destroyed")},
    {7, TEXT("Platform stand")},
    {8, TEXT("Selector master")},
    {9, TEXT("Selector slave")},
    {10, TEXT("Teleport random")},
};
}

extern const FKyaActorFamily GKyaMovingPlatformActorFamily = {
    0x0d, TEXT("CActorMovingPlatform"), TEXT("Moving platform"),
    TEXT("CActorMovingPlatform: carried actors, zones, camera/notification records, 0x4c properties and sound streams"),
    MakeArrayView(MovingPlatformBehaviours),
    TConstArrayView<FKyaActorFamilyReference>(),
};
