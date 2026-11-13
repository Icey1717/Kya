#include "KyaActorFamily.h"

/**
 * Recovered metadata and codec registration for Fruit (class 0x16).
 *
 * The class and behaviour 4/5/6 codecs live in Actors/KyaFruit.h/.cpp. Ids 0/1/2
 * are inherited/generic; id 3 (suspended) has an empty Create; id 7 returns
 * CBehaviourProjectileNew but stays raw because it has no edit surface here.
 */
namespace
{
const FKyaActorFamilyBehaviour FruitBehaviours[] = {
    {0, TEXT("Stand")},
    {1, TEXT("Cinematic")},
    {3, TEXT("Fruit suspended")},
    {4, TEXT("Fruit squashed")},
    {5, TEXT("Inventory")},
    {6, TEXT("Fruit bonus")},
    {7, TEXT("Fruit projectile")},
};
}

extern const FKyaActorFamily GKyaFruitActorFamily = {
    0x16, TEXT("CActorFruit"), TEXT("Fruit"),
    TEXT("autonomous prefix, switch notification and squashed/bonus/inventory behaviours"),
    MakeArrayView(FruitBehaviours),
    TConstArrayView<FKyaActorFamilyReference>(),
};
