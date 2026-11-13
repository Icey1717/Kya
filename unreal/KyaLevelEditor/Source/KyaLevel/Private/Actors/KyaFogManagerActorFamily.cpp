#include "KyaActorFamily.h"

/**
 * Recovered metadata and codec registration for Fog manager (class 0x3D).
 *
 * The class and behaviour 2 codecs live in Actors/KyaFog.h/.cpp. BuildBehaviour
 * only adds id 2 (CBehaviourFogManagerZones); ids 0/1 are inherited.
 */
namespace
{
const FKyaActorFamilyBehaviour FogManagerBehaviours[] = {
    {0, TEXT("Stand")},
    {1, TEXT("Cinematic")},
    {2, TEXT("Fog zones")},
};
}

extern const FKyaActorFamily GKyaFogManagerActorFamily = {
    0x3d, TEXT("CActorFogManager"), TEXT("Fog manager"),
    TEXT("fog-hierarchy hashes/radii and streamed fog-zone records"),
    MakeArrayView(FogManagerBehaviours),
    TConstArrayView<FKyaActorFamilyReference>(),
};
