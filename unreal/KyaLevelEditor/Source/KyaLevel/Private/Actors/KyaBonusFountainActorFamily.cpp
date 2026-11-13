#include "KyaActorFamily.h"

/**
 * Recovered metadata and codec registration for Bonus fountain (class 0x52).
 *
 * The class payload codec lives in Actors/KyaFountain.h/.cpp.
 * BuildBehaviour only adds id 2 (CBehaviourBonusFountainStand), which has no
 * Create override and therefore no payload; ids 0/1 are inherited.
 */
namespace
{
const FKyaActorFamilyBehaviour BonusFountainBehaviours[] = {
    {0, TEXT("Stand")},
    {1, TEXT("Cinematic")},
    {2, TEXT("Bonus fountain stand")},
};
}

extern const FKyaActorFamily GKyaBonusFountainActorFamily = {
    0x52, TEXT("CActorBonusFountain"), TEXT("Bonus fountain"),
    TEXT("CAddOnGenerator money/bonus tuning and sub-object block"),
    MakeArrayView(BonusFountainBehaviours),
    TConstArrayView<FKyaActorFamilyReference>(),
};
