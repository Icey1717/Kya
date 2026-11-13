#include "KyaActorFamily.h"
namespace
{
const FKyaActorFamilyBehaviour Behaviours[] = {
    {0, TEXT("Stand")}, {1, TEXT("Cinematic")}, {2, TEXT("Credits default")},
};
}
extern const FKyaActorFamily GKyaCreditsActorFamily = {
    0x56, TEXT("CActorCredits"), TEXT("Credits"),
    TEXT("Message hashes, authored 512x448 display settings, music, level transition and completion switch"),
    MakeArrayView(Behaviours), {},
};
