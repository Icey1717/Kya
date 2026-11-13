#include "KyaActorFamily.h"

/**
 * Recovered metadata and codec registration for Nativ command (class 0x3C).
 *
 * The class codec lives in Actors/KyaNativCmd.h/.cpp. BuildBehaviour only adds
 * id 2 (CBehaviourNativCmdStand), whose Create is empty; ids 0/1 are inherited.
 */
namespace
{
const FKyaActorFamilyBehaviour NativCommandBehaviours[] = {
    {0, TEXT("Stand")},
    {1, TEXT("Cinematic")},
    {2, TEXT("Nativ command stand")},
};
}

extern const FKyaActorFamily GKyaNativCommandActorFamily = {
    0x3c, TEXT("CActorNativCmd"), TEXT("Nativ command"),
    TEXT("actor list, field 164 and four CAddOnNativ sub-object blocks"),
    MakeArrayView(NativCommandBehaviours),
    TConstArrayView<FKyaActorFamilyReference>(),
};
