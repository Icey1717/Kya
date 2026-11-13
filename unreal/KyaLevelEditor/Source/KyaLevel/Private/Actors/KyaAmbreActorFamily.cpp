#include "KyaActorFamily.h"

/**
 * Recovered metadata and codec registration for Ambre (class 0x14).
 *
 * The class payload codec lives in Actors/KyaAmbre.h/.cpp. BuildBehaviour maps
 * id 2 (stored amber behaviour) and id 3 (CBehaviourAmbreUnknown); neither
 * overrides Create, so both payloads are empty. Ids 0/1 are inherited.
 */
namespace
{
const FKyaActorFamilyBehaviour AmbreBehaviours[] = {
    {0, TEXT("Stand")},
    {1, TEXT("Cinematic")},
    {2, TEXT("Ambre")},
    {3, TEXT("Ambre unknown")},
};
}

extern const FKyaActorFamily GKyaAmbreActorFamily = {
    0x14, TEXT("CActorAmbre"), TEXT("Ambre"),
    TEXT("material/effect/texture/mesh ids and switch record"),
    MakeArrayView(AmbreBehaviours),
    TConstArrayView<FKyaActorFamilyReference>(),
};
