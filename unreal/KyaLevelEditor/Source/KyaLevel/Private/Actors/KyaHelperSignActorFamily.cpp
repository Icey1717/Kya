#include "KyaActorFamily.h"
namespace
{
const FKyaActorFamilyBehaviour Behaviours[] = {
    {0, TEXT("Stand")}, {1, TEXT("Cinematic")},
    {2, TEXT("Helper sign stand")}, {3, TEXT("High score")},
};
}
extern const FKyaActorFamily GKyaHelperSignActorFamily = {
    0x2b, TEXT("CActorHelperSign"), TEXT("Helper sign"),
    TEXT("Signed field 168 and 64-bit text hash; High score references the minigame organizer"),
    MakeArrayView(Behaviours), {},
};
