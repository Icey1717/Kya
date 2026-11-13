#include "KyaActorFamily.h"
namespace
{
const FKyaActorFamilyBehaviour Behaviours[] = {
    {0, TEXT("Stand")}, {1, TEXT("Cinematic")}, {2, TEXT("Pattern command stand")},
};
}
extern const FKyaActorFamily GKyaPatternCmdActorFamily = {
    0x40, TEXT("CActorPatternCmd"), TEXT("Pattern command"),
    TEXT("Two unsigned fields, actor-reference list, two floats and versioned CPatternPart; behaviour 2 is empty"),
    MakeArrayView(Behaviours), {},
};
