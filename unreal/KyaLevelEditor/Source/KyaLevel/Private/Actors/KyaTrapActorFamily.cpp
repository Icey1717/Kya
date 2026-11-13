#include "KyaActorFamily.h"

/**
 * Recovered metadata and codec registration for Trap (class 0x2c).
 *
 * The class payload and behaviour 2/3 codecs live in Actors/KyaTrap.h/.cpp.
 * Ids 0/1 are inherited.
 */
namespace
{
const FKyaActorFamilyBehaviour TrapBehaviours[] = {
    {0, TEXT("Stand")},
    {1, TEXT("Cinematic")},
    {2, TEXT("Trap stand")},
    {3, TEXT("Inactive")},
};
const FKyaActorFamilyReference TrapReferences[] = {
    {TEXT("TrapActorRef"), EKyaReferenceKind::Actor},
};
}

extern const FKyaActorFamily GKyaTrapActorFamily = {
    0x2c, TEXT("CActorTrap"), TEXT("Trap"),
    TEXT("movement/path settings, escape state, switch stream and trap-stand/inactive behaviours"),
    MakeArrayView(TrapBehaviours),
    MakeArrayView(TrapReferences),
};
