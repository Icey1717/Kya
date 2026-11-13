#include "KyaActorFamily.h"

/**
 * Recovered metadata and codec registration for Boomy (class 0x05).
 *
 * The class payload codec lives in Actors/KyaBoomy.h/.cpp. BuildBehaviour only
 * adds id 2 (CBehaviourBoomyLaunch), whose Create is empty; ids 0/1 are
 * inherited.
 */
namespace
{
const FKyaActorFamilyBehaviour BoomyBehaviours[] = {
    {0, TEXT("Stand")},
    {1, TEXT("Cinematic")},
    {2, TEXT("Boomy launch")},
};
}

extern const FKyaActorFamily GKyaBoomyActorFamily = {
    0x05, TEXT("CActorBoomy"), TEXT("Boomy"),
    TEXT("vision, tail/particle fields, embedded camera and vision/damage type table"),
    MakeArrayView(BoomyBehaviours),
    TConstArrayView<FKyaActorFamilyReference>(),
};
