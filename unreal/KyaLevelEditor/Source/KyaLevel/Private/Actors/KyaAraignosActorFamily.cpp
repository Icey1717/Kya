#include "KyaActorFamily.h"

/**
 * Recovered metadata and codec registration for Araignos (class 0x21).
 *
 * The class codec lives in Actors/KyaAraignos.h/.cpp. BuildBehaviour only adds
 * id 2 (CBhvDefault), whose Create is empty; ids 0/1 are inherited.
 */
namespace
{
const FKyaActorFamilyBehaviour AraignosBehaviours[] = {
    {0, TEXT("Stand")},
    {1, TEXT("Cinematic")},
    {2, TEXT("Araignos default")},
};
}

extern const FKyaActorFamily GKyaAraignosActorFamily = {
    0x21, TEXT("CActorAraignos"), TEXT("Araignos"),
    TEXT("movable weight, path follow, sound references, add-on generator"),
    MakeArrayView(AraignosBehaviours),
    TConstArrayView<FKyaActorFamilyReference>(),
};
