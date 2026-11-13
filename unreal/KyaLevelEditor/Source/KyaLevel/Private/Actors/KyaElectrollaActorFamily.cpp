#include "KyaActorFamily.h"
namespace
{
const FKyaActorFamilyBehaviour Behaviours[] = {
    {0, TEXT("Stand")}, {1, TEXT("Cinematic")}, {2, TEXT("Electrolla stand")},
};
}
extern const FKyaActorFamily GKyaElectrollaActorFamily = {
    0x0b, TEXT("CActorElectrolla"), TEXT("Electrolla"),
    TEXT("Spark count, six floats, two material ids and an on/off switch with camera; Stand has no payload"),
    MakeArrayView(Behaviours), {},
};
