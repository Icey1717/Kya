#include "KyaActorFamily.h"

/**
 * Recovered metadata and codec registration for Shocker (class 0x4B).
 *
 * The class and behaviour 3 codecs live in Actors/KyaShocker.h/.cpp.
 * BuildBehaviour only adds id 3 (CBehaviourShockerFireWave); ids 0/1/2 are
 * inherited.
 */
namespace
{
const FKyaActorFamilyBehaviour ShockerBehaviours[] = {
    {0, TEXT("Stand")},
    {1, TEXT("Cinematic")},
    {2, TEXT("Catch by trap")},
    {3, TEXT("Shocker fire wave")},
};
}

extern const FKyaActorFamily GKyaShockerActorFamily = {
    0x4b, TEXT("CActorShocker"), TEXT("Shocker"),
    TEXT("autonomous tuning, zone/switch record, add-on and fire-wave behaviour (switch list rebuilt on export)"),
    MakeArrayView(ShockerBehaviours),
    TConstArrayView<FKyaActorFamilyReference>(),
};
