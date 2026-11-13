#include "KyaActorFamily.h"
namespace
{
const FKyaActorFamilyBehaviour Behaviours[] = {
    {0, TEXT("Stand")}, {1, TEXT("Cinematic")}, {2, TEXT("Wood monster get")},
};
}
extern const FKyaActorFamily GKyaWoodMonsterActorFamily = {
    0x27, TEXT("CActorWoodMonster"), TEXT("Wood monster"),
    TEXT("Four field-160 floats; Get reads a float, waypoint and signed field 10"),
    MakeArrayView(Behaviours), {},
};
