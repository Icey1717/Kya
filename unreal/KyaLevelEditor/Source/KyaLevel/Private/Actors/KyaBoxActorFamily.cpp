#include "KyaActorFamily.h"

/**
 * Recovered metadata and codec registration for Box (class 0x35).
 *
 * The class payload codec lives in Actors/KyaBox.h/.cpp. BuildBehaviour only adds
 * id 3 (CBehaviourBoxStand), whose Create is empty; ids 0/1/2 are inherited.
 */
namespace
{
const FKyaActorFamilyBehaviour BoxBehaviours[] = {
    {0, TEXT("Stand")},
    {1, TEXT("Cinematic")},
    {2, TEXT("Catch by trap")},
    {3, TEXT("Box stand")},
};
}

extern const FKyaActorFamily GKyaBoxActorFamily = {
    0x35, TEXT("CActorBox"), TEXT("Box"),
    TEXT("autonomous tuning, actor list, switch record and add-on generator"),
    MakeArrayView(BoxBehaviours),
    TConstArrayView<FKyaActorFamilyReference>(),
};
