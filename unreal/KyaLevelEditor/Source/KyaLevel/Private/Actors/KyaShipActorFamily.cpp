#include "KyaActorFamily.h"

/**
 * Recovered metadata and codec registration for Ship (class 0x36; bank actors
 * are named HUNTERSHIP).
 *
 * The class codec lives in Actors/KyaShip.h/.cpp. BuildBehaviour only adds id 2
 * (SHIP_BEHAVIOUR_FLY); CBehaviourShip and CBehaviourShipFLY both have empty
 * Create bodies. Ids 0/1 are inherited.
 */
namespace
{
const FKyaActorFamilyBehaviour ShipBehaviours[] = {
    {0, TEXT("Stand")},
    {1, TEXT("Cinematic")},
    {2, TEXT("Ship fly")},
};
}

extern const FKyaActorFamily GKyaShipActorFamily = {
    0x36, TEXT("CActorShip"), TEXT("Ship"),
    TEXT("care-boy/fx lists, actor refs, tuning, vision and switch"),
    MakeArrayView(ShipBehaviours),
    TConstArrayView<FKyaActorFamilyReference>(),
};
