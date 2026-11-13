#include "KyaActorFamily.h"
namespace
{
const FKyaActorFamilyBehaviour Behaviours[] = {
    {0, TEXT("Stand")}, {1, TEXT("Cinematic")},
    {2, TEXT("Catch by trap")}, {3, TEXT("Hunter watch")},
};
const FKyaActorFamilyReference References[] = {
    {TEXT("HunterActorRef"), EKyaReferenceKind::Actor},
};
}
extern const FKyaActorFamily GKyaHunterActorFamily = {
    0x48, TEXT("CActorHunter"), TEXT("Hunter"),
    TEXT("Autonomous fields, bone id, actor reference and two floats; Watch has no payload"),
    MakeArrayView(Behaviours), MakeArrayView(References),
};
