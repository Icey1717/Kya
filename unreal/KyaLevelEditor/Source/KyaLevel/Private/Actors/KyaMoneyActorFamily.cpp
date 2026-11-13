#include "KyaActorFamily.h"

/**
 * Recovered metadata and codec registration for Money (class 0x32).
 *
 * The class and behaviour 2/3 codecs live in Actors/KyaMoney.h/.cpp.
 * BuildBehaviour maps 2 to CBehaviourMoneyFlock and 3 to CBehaviourMoneyAddOn;
 * ids 0/1 are inherited.
 */
namespace
{
const FKyaActorFamilyBehaviour MoneyBehaviours[] = {
    {0, TEXT("Stand")},
    {1, TEXT("Cinematic")},
    {2, TEXT("Money flock")},
    {3, TEXT("Money add-on")},
};
}

extern const FKyaActorFamily GKyaMoneyActorFamily = {
    0x32, TEXT("CActorMoney"), TEXT("Money"),
    TEXT("weight/value/sound/material and flock/add-on behaviours"),
    MakeArrayView(MoneyBehaviours),
    TConstArrayView<FKyaActorFamilyReference>(),
};
