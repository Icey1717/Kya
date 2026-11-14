#include "KyaActorFamily.h"

/**
 * Recovered metadata and codec registration for Explosive distributor (0x1D).
 *
 * The class codec lives in Actors/KyaExplosiveDistributor.h/.cpp. BuildBehaviour
 * only adds id 2 (CBehaviourExplosiveDistributor), whose Create is empty; ids
 * 0/1 are inherited.
 */
namespace
{
const FKyaActorFamilyBehaviour ExplosiveDistributorBehaviours[] = {
    {0, TEXT("Stand")},
    {1, TEXT("Cinematic")},
    {2, TEXT("Explosive distributor")},
};
}

extern const FKyaActorFamily GKyaExplosiveDistributorActorFamily = {
    0x1d, TEXT("CActorExplosiveDistributor"), TEXT("Explosive distributor"),
    TEXT("radius, angle, projectile records and one camera event"),
    MakeArrayView(ExplosiveDistributorBehaviours),
    TConstArrayView<FKyaActorFamilyReference>(),
};
