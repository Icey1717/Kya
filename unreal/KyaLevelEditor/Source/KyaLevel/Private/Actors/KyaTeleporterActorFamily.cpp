#include "KyaActorFamily.h"

/**
 * Recovered metadata and codec registration for Teleporter (class 0x24).
 *
 * The class payload codec lives in Actors/KyaTeleporter.h/.cpp. BuildBehaviour
 * only adds id 2 (CBehaviourTeleporterDefault), whose Create is empty; ids 0/1
 * are inherited.
 */
namespace
{
const FKyaActorFamilyBehaviour TeleporterBehaviours[] = {
    {0, TEXT("Stand")},
    {1, TEXT("Cinematic")},
    {2, TEXT("Teleporter default")},
};
}

extern const FKyaActorFamily GKyaTeleporterActorFamily = {
    0x24, TEXT("CActorTeleporter"), TEXT("Teleporter"),
    TEXT("destinations, waypoints and conditioned-operation blocks (all rebuilt on export), three switch/camera records and zone/material fields"),
    MakeArrayView(TeleporterBehaviours),
    TConstArrayView<FKyaActorFamilyReference>(),
};
