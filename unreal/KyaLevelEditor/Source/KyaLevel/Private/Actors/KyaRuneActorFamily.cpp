#include "KyaActorFamily.h"

/**
 * Recovered metadata and codec registration for Rune (class 0x51).
 *
 * The class codec lives in Actors/KyaRune.h/.cpp. BuildBehaviour only adds id 2
 * (CBehaviourRune), whose Create is empty; ids 0/1 are inherited.
 */
namespace
{
const FKyaActorFamilyBehaviour RuneBehaviours[] = {
    {0, TEXT("Stand")},
    {1, TEXT("Cinematic")},
    {2, TEXT("Rune")},
};
}

extern const FKyaActorFamily GKyaRuneActorFamily = {
    0x51, TEXT("CActorRune"), TEXT("Rune"),
    TEXT("cinematic/scenario/fx ids, trigger distance and two material ids"),
    MakeArrayView(RuneBehaviours),
    TConstArrayView<FKyaActorFamilyReference>(),
};
