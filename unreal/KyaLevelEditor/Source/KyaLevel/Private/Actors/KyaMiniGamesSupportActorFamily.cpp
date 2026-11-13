#include "KyaActorFamily.h"
namespace
{
const FKyaActorFamilyBehaviour ManagerBehaviours[] = {
    {0, TEXT("Stand")}, {1, TEXT("Cinematic")}, {2, TEXT("Minigames manager stand")},
};
const FKyaActorFamilyBehaviour OrganizerBehaviours[] = {
    {0, TEXT("Stand")}, {1, TEXT("Cinematic")}, {2, TEXT("Minigames organizer stand")},
};
const FKyaActorFamilyReference ManagerReferences[] = {
    {TEXT("MiniGamesManagerActorRef"), EKyaReferenceKind::Actor},
};
}
extern const FKyaActorFamily GKyaMiniGamesManagerActorFamily = {
    0x42, TEXT("CActorMiniGamesManager"), TEXT("Minigames manager"),
    TEXT("Organizer actor stream and actor reference"), MakeArrayView(ManagerBehaviours), MakeArrayView(ManagerReferences),
};
extern const FKyaActorFamily GKyaMiniGamesOrganizerActorFamily = {
    0x43, TEXT("CActorMiniGamesOrganizer"), TEXT("Minigames organizer"),
    TEXT("Aligned name, mesh/texture/material fields, minigame actor stream and scalar settings"), MakeArrayView(OrganizerBehaviours), {},
};
