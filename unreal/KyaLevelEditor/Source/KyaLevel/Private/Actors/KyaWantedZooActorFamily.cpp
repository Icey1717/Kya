#include "KyaActorFamily.h"
namespace
{
const FKyaActorFamilyBehaviour Behaviours[] = {{0, TEXT("Stand")}, {1, TEXT("Cinematic")}};
}
extern const FKyaActorFamily GKyaWantedZooActorFamily = {
    0x54, TEXT("CActorWantedZoo"), TEXT("Wanted zoo"),
    TEXT("Text/number positions and ordered scenario-gated message and numeric labels"),
    MakeArrayView(Behaviours), {},
};
