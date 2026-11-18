#include "KyaActorFamily.h"

/**
 * Recovered metadata and codec registration for Shoot (class 0x3E).
 *
 * The class and behaviour 3/4 codecs live in Actors/KyaShoot.h/.cpp.
 * BuildBehaviour maps 3 to CBehaviourShootFire and 4 to
 * CBehaviourShootFireWave; ids 0/1/2 are inherited.
 */
namespace
{
const FKyaActorFamilyBehaviour ShootBehaviours[] = {
    {0, TEXT("Stand")},
    {1, TEXT("Cinematic")},
    {2, TEXT("Catch by trap")},
    {3, TEXT("Shoot fire")},
    {4, TEXT("Shoot fire wave")},
};
}

extern const FKyaActorFamily GKyaShootActorFamily = {
    0x3e, TEXT("CActorShoot"), TEXT("Shoot"),
    TEXT("autonomous tuning, mesh/effect ids, add-on and fire/wave behaviours; the fire actor-ref list is rebuilt on export"),
    MakeArrayView(ShootBehaviours),
    TConstArrayView<FKyaActorFamilyReference>(),
};
