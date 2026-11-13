#include "KyaActorFamily.h"
namespace
{
const FKyaActorFamilyBehaviour Behaviours[] = {
    {0, TEXT("Stand")}, {1, TEXT("Cinematic")}, {2, TEXT("Catch by trap")}, {3, TEXT("Blazer stand")},
};
}
extern const FKyaActorFamily GKyaBlazerActorFamily = {
    0x4d, TEXT("CActorBlazer"), TEXT("Blazer"),
    TEXT("Autonomous prefix, versioned waypoint refs, fire-shot actor list and tilt/push configuration"),
    MakeArrayView(Behaviours), {},
};
