#include "KyaActorFamily.h"

/**
 * Recovered metadata and codec registration for Basic box (class 0x39).
 *
 * The class codec lives in Actors/KyaBasicBox.h/.cpp. BuildBehaviour only adds
 * id 2 (CBehaviourBasicBoxStand), whose Create is empty; ids 0/1 are inherited.
 */
namespace
{
const FKyaActorFamilyBehaviour BasicBoxBehaviours[] = {
    {0, TEXT("Stand")},
    {1, TEXT("Cinematic")},
    {2, TEXT("Basic box stand")},
};
}

extern const FKyaActorFamily GKyaBasicBoxActorFamily = {
    0x39, TEXT("CActorBasicBox"), TEXT("Basic box"),
    TEXT("life, vibration, actor list, switch record and add-on generator; the actor list is rebuilt on export"),
    MakeArrayView(BasicBoxBehaviours),
    TConstArrayView<FKyaActorFamilyReference>(),
};
