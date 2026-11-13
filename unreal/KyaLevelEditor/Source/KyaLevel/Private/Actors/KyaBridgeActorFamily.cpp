#include "KyaActorFamily.h"

/**
 * Recovered metadata and codec registration for Bridge (class 0x17).
 *
 * The class and behaviour 2/3 codecs live in Actors/KyaBridge.h/.cpp.
 * BuildBehaviour maps 2 to CBehaviourBridgeBasic and 3 to
 * CBehaviourBridgeWindAware; ids 0/1 are inherited.
 */
namespace
{
const FKyaActorFamilyBehaviour BridgeBehaviours[] = {
    {0, TEXT("Stand")},
    {1, TEXT("Cinematic")},
    {2, TEXT("Bridge basic")},
    {3, TEXT("Bridge wind")},
};
const FKyaActorFamilyReference BridgeReferences[] = {
    {TEXT("BridgeWaypointA"), EKyaReferenceKind::Waypoint},
    {TEXT("BridgeWaypointB"), EKyaReferenceKind::Waypoint},
};
}

extern const FKyaActorFamily GKyaBridgeActorFamily = {
    0x17, TEXT("CActorBridge"), TEXT("Bridge"),
    TEXT("actor/waypoint/config data and basic/wind-aware behaviour payloads"),
    MakeArrayView(BridgeBehaviours),
    MakeArrayView(BridgeReferences),
};
