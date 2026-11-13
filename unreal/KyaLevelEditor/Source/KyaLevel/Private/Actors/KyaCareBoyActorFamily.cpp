#include "KyaActorFamily.h"
namespace
{
const FKyaActorFamilyBehaviour Behaviours[] = {
    {0, TEXT("Stand")}, {1, TEXT("Cinematic")}, {2, TEXT("Care boy steady")},
};
}
extern const FKyaActorFamily GKyaCareBoyActorFamily = {
    0x37, TEXT("CActorCareBoy"), TEXT("Care boy"),
    TEXT("Movable weight and two signed fields; Steady has no serialized payload"),
    MakeArrayView(Behaviours), {},
};
