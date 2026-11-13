#include "KyaActorFamily.h"
namespace
{
const FKyaActorFamilyBehaviour Behaviours[] = {
    {0, TEXT("Stand")}, {1, TEXT("Cinematic")}, {2, TEXT("Catch by trap")},
    {3, TEXT("JamGut stand")}, {4, TEXT("JamGut ridden")},
};
}
extern const FKyaActorFamily GKyaJamGutActorFamily = {
    0x33, TEXT("CActorJamGut"), TEXT("JamGut"),
    TEXT("Versioned autonomous settings, checkpoint/path lists and type; Ridden music settings; Stand has no payload"),
    MakeArrayView(Behaviours), {},
};
