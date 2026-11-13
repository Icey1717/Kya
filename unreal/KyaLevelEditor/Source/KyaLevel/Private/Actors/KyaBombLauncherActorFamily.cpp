#include "KyaActorFamily.h"
namespace
{
const FKyaActorFamilyBehaviour Behaviours[] = {
    {0, TEXT("Stand")}, {1, TEXT("Cinematic")}, {2, TEXT("Bomb launcher stand")},
};
}
extern const FKyaActorFamily GKyaBombLauncherActorFamily = {
    0x53, TEXT("CActorBombLauncher"), TEXT("Bomb launcher"),
    TEXT("Field 168 and CFireShot actor-reference list; Stand has no serialized payload"),
    MakeArrayView(Behaviours), {},
};
