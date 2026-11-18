#include "KyaActorFamily.h"

/**
 * Recovered metadata and codec registration for Stiller (class 0x4C).
 *
 * The class payload codec lives in Actors/KyaStiller.h/.cpp. BuildBehaviour only
 * adds id 2 (CBehaviourStillerStand), whose Create is empty; ids 0/1 are
 * inherited.
 */
namespace
{
const FKyaActorFamilyBehaviour StillerBehaviours[] = {
    {0, TEXT("Stand")},
    {1, TEXT("Cinematic")},
    {2, TEXT("Stiller stand")},
};
const FKyaActorFamilyReference StillerReferences[] = {
    {TEXT("StillerPathIndex"), EKyaReferenceKind::Path},
};
}

extern const FKyaActorFamily GKyaStillerActorFamily = {
    0x4c, TEXT("CActorStiller"), TEXT("Stiller"),
    TEXT("vision, path, life, actor list (rebuilt on export), two switch records and add-on generator"),
    MakeArrayView(StillerBehaviours),
    MakeArrayView(StillerReferences),
};
