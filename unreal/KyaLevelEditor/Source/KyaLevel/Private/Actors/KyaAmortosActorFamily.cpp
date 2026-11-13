#include "KyaActorFamily.h"

/**
 * Recovered metadata and codec registration for Amortos (class 0x29).
 *
 * The class payload codec lives in Actors/KyaAmortos.h/.cpp. BuildBehaviour only
 * adds id 2 (CBehaviourAmortosStand), whose Create is empty; ids 0/1 are
 * inherited.
 */
namespace
{
const FKyaActorFamilyBehaviour AmortosBehaviours[] = {
    {0, TEXT("Stand")},
    {1, TEXT("Cinematic")},
    {2, TEXT("Amortos stand")},
};
const FKyaActorFamilyReference AmortosReferences[] = {
    {TEXT("AmortosActorRef"), EKyaReferenceKind::Actor},
};
}

extern const FKyaActorFamily GKyaAmortosActorFamily = {
    0x29, TEXT("CActorAmortos"), TEXT("Amortos"),
    TEXT("actor lists, tuning floats, add-on generator and two switch/camera records"),
    MakeArrayView(AmortosBehaviours),
    MakeArrayView(AmortosReferences),
};
