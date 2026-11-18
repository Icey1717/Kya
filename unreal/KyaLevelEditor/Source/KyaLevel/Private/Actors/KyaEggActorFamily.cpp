#include "KyaActorFamily.h"
namespace
{
const FKyaActorFamilyBehaviour Behaviours[] = {
    {0, TEXT("Stand")}, {1, TEXT("Cinematic")}, {2, TEXT("Egg move")},
};
}
extern const FKyaActorFamily GKyaEggActorFamily = {
    0x2f, TEXT("CActorEgg"), TEXT("Egg"),
    TEXT("Movable weight, field 1D8, path-plane list (rebuilt on export) and two unsigned fields; Move has no payload"),
    MakeArrayView(Behaviours), {},
};
