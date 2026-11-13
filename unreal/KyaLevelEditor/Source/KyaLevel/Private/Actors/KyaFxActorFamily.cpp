#include "KyaActorFamily.h"

/**
 * Recovered metadata and codec registration for FX (class 0x2E).
 *
 * The class and behaviour 2/3/4 codecs live in Actors/KyaFx.h/.cpp.
 * BuildBehaviour maps 2 Single, 3 Path and 4 Unknown; ids 0/1 are inherited.
 */
namespace
{
const FKyaActorFamilyBehaviour FxBehaviours[] = {
    {0, TEXT("Stand")},
    {1, TEXT("Cinematic")},
    {2, TEXT("FX single")},
    {3, TEXT("FX path")},
    {4, TEXT("FX unknown")},
};
}

extern const FKyaActorFamily GKyaFxActorFamily = {
    0x2e, TEXT("CActorFx"), TEXT("FX"),
    TEXT("effect timing floats and single/path/unknown behaviour payloads"),
    MakeArrayView(FxBehaviours),
    TConstArrayView<FKyaActorFamilyReference>(),
};
