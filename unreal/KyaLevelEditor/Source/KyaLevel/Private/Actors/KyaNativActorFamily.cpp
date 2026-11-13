#include "KyaActorFamily.h"

/**
 * Recovered metadata and codec registration for Nativ (class 0x0a).
 *
 * The class payload and behaviour 3/5/6/7 codecs live in Actors/KyaNativ.h/.cpp.
 * Behaviour 4 (exorcism) inherits an empty Create; id 8 uses KyaNativAkasa.
 */
namespace
{
const FKyaActorFamilyBehaviour NativBehaviours[] = {
    {0, TEXT("Stand")},
    {1, TEXT("Cinematic")},
    {2, TEXT("Catch by trap")},
    {3, TEXT("Nativ speak")},
    {4, TEXT("Nativ exorcism")},
    {5, TEXT("Nativ live")},
    {6, TEXT("Nativ take and put")},
    {7, TEXT("Nativ seller")},
    {8, TEXT("Nativ Akasa")},
};
}

extern const FKyaActorFamily GKyaNativActorFamily = {
    0x0a, TEXT("CActorNativ"), TEXT("Nativ"),
    TEXT("autonomous/movement fields and take-and-put trajectories"),
    MakeArrayView(NativBehaviours),
    TConstArrayView<FKyaActorFamilyReference>(),
};
