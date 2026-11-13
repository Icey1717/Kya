#include "KyaActorFamily.h"

/**
 * Recovered metadata and codec registration for Companion (class 0x19).
 *
 * The class and behaviour 3 codecs live in Actors/KyaCompanion.h/.cpp.
 * BuildBehaviour only adds id 3 (CBehaviourCompanionAlert); ids 0/1/2 are
 * inherited.
 */
namespace
{
const FKyaActorFamilyBehaviour CompanionBehaviours[] = {
    {0, TEXT("Stand")},
    {1, TEXT("Cinematic")},
    {2, TEXT("Catch by trap")},
    {3, TEXT("Companion alert")},
};
}

extern const FKyaActorFamily GKyaCompanionActorFamily = {
    0x19, TEXT("CActorCompanion"), TEXT("Companion"),
    TEXT("autonomous tuning plus alert conditions, switches and conditioned operations"),
    MakeArrayView(CompanionBehaviours),
    TConstArrayView<FKyaActorFamilyReference>(),
};
