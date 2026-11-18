#include "KyaActorFamily.h"
namespace
{
const FKyaActorFamilyBehaviour Behaviours[] = {
    {0, TEXT("Stand")}, {1, TEXT("Cinematic")}, {2, TEXT("Catch by trap")},
    {3, TEXT("Woof guard")}, {4, TEXT("Woof vertical jump")},
};
}
extern const FKyaActorFamily GKyaWoofActorFamily = {
    0x2a, TEXT("CActorWoof"), TEXT("Woof"),
    TEXT("Autonomous fields, actor stream (rebuilt on export), path/waypoint and authored vision values; Guard and VerticalJump have no payload"),
    MakeArrayView(Behaviours), {},
};
