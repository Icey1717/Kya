#include "KyaActorFamily.h"

/**
 * Recovered metadata and codec registration for Clusteriser (class 0x18).
 *
 * CActorClusteriser::Create has no class-specific payload. The behaviour 2
 * (CBehaviourClusteriserZones) codec lives in Actors/KyaClusteriser.h/.cpp;
 * ids 0/1 are inherited from CActor.
 */
namespace
{
const FKyaActorFamilyBehaviour ClusteriserBehaviours[] = {
    {0, TEXT("Stand")},
    {1, TEXT("Cinematic")},
    {2, TEXT("Cluster zones")},
};
}

extern const FKyaActorFamily GKyaClusteriserActorFamily = {
    0x18, TEXT("CActorClusteriser"), TEXT("Clusteriser"),
    TEXT("behaviour 2 zone lists with actor, camera, light and hierarchy-hash references, rebuilt on export"),
    MakeArrayView(ClusteriserBehaviours),
    TConstArrayView<FKyaActorFamilyReference>(),
};
