#include "KyaActorFamily.h"
namespace
{
const FKyaActorFamilyBehaviour Behaviours[] = {
    {0, TEXT("Stand")}, {1, TEXT("Cinematic")}, {2, TEXT("Shocker command stand")},
};
}
extern const FKyaActorFamily GKyaShockerCmdActorFamily = {
    0x50, TEXT("CActorShockerCmd"), TEXT("Shocker command"),
    TEXT("Field 168 and actor sequences with float lists; sequence counts are fixed on export"),
    MakeArrayView(Behaviours), {},
};
