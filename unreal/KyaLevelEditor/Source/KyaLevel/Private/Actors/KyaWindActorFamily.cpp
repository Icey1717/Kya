#include "KyaActorFamily.h"

/**
 * Recovered metadata and codec registration for Wind (class 0x12).
 *
 * The class payload codec lives in Actors/KyaWind.h/.cpp. BuildBehaviour only
 * adds id 2 (CBehaviourWind, empty Create); ids 0/1 are inherited.
 */
namespace
{
const FKyaActorFamilyBehaviour WindBehaviours[] = {
    {0, TEXT("Stand")},
    {1, TEXT("Cinematic")},
    {2, TEXT("Wind")},
};
const FKyaActorFamilyReference WindReferences[] = {
    {TEXT("WindWaypointRef"), EKyaReferenceKind::Waypoint},
    {TEXT("WindWaypointRef1CC"), EKyaReferenceKind::Waypoint},
    {TEXT("WindZoneRef1C4"), EKyaReferenceKind::Zone},
};
}

extern const FKyaActorFamily GKyaWindActorFamily = {
    0x12, TEXT("CActorWind"), TEXT("Wind"),
    TEXT("tuning, zones, actor/material references, sector hierarchy and version-gated switch records"),
    MakeArrayView(WindBehaviours),
    MakeArrayView(WindReferences),
};
