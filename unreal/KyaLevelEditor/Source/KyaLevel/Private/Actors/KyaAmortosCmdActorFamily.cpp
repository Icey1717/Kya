#include "KyaActorFamily.h"
namespace
{
const FKyaActorFamilyBehaviour Behaviours[] = {
    {0, TEXT("Stand")}, {1, TEXT("Cinematic")}, {2, TEXT("Amortos command stand")},
};
}
extern const FKyaActorFamily GKyaAmortosCmdActorFamily = {
    0x31, TEXT("CActorAmortosCmd"), TEXT("Amortos command"),
    TEXT("Actor-reference stream (rebuilt on export); behaviour 2 has no payload"),
    MakeArrayView(Behaviours), {},
};
