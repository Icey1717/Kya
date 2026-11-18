#include "KyaActorFamily.h"

/**
 * Recovered metadata and codec registration for Aton (class 0x38).
 *
 * The class payload and add-on/path codec live in Actors/KyaAton.h/.cpp.
 * BuildBehaviour only adds id 3 (CBehaviourAtonEscape), which has no Create
 * override and therefore no payload; ids 0/1/2 are inherited.
 */
namespace
{
const FKyaActorFamilyBehaviour AtonBehaviours[] = {
    {0, TEXT("Stand")},
    {1, TEXT("Cinematic")},
    {2, TEXT("Catch by trap")},
    {3, TEXT("Aton escape")},
};
}

extern const FKyaActorFamily GKyaAtonActorFamily = {
    0x38, TEXT("CActorAton"), TEXT("Aton"),
    TEXT("autonomous values, effect/mesh/material ids, path list and add-on sub-objects (both rebuilt on export)"),
    MakeArrayView(AtonBehaviours),
    TConstArrayView<FKyaActorFamilyReference>(),
};
