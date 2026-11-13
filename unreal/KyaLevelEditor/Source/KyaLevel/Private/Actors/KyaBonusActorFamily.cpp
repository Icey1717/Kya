#include "KyaActorFamily.h"

/**
 * Recovered metadata and codec registration for Bonus (class 0x0e).
 *
 * The 16-byte class payload and behaviour 3/4/5/6 codecs live in
 * Actors/KyaBonus.h/.cpp. Behaviour 2 returns null in source and has no codec;
 * ids 0/1 are inherited.
 */
namespace
{
const FKyaActorFamilyBehaviour BonusBehaviours[] = {
    {0, TEXT("Stand")},
    {1, TEXT("Cinematic")},
    {3, TEXT("Bonus turn")},
    {4, TEXT("Bonus path")},
    {5, TEXT("Bonus flock")},
    {6, TEXT("Bonus add-on")},
};
}

extern const FKyaActorFamily GKyaBonusActorFamily = {
    0x0e, TEXT("CActorBonus"), TEXT("Bonus"),
    TEXT("sound/material values and turn/path/flock/add-on behaviours"),
    MakeArrayView(BonusBehaviours),
    TConstArrayView<FKyaActorFamilyReference>(),
};
