#include "KyaActorFamily.h"

/**
 * Recovered metadata and codec registration for Nose monster (class 0x26).
 *
 * The class codec lives in Actors/KyaNoseMonster.h/.cpp. BuildBehaviour adds
 * ids 2 (sleep), 3 (attack) and 4 (death); every Create body is empty. Ids 0/1
 * are inherited from CActorAutonomous.
 */
namespace
{
const FKyaActorFamilyBehaviour NoseMonsterBehaviours[] = {
    {0, TEXT("Stand")},
    {1, TEXT("Cinematic")},
    {2, TEXT("Nose monster sleep")},
    {3, TEXT("Nose monster attack")},
    {4, TEXT("Nose monster death")},
};
}

extern const FKyaActorFamily GKyaNoseMonsterActorFamily = {
    0x26, TEXT("CActorNoseMonster"), TEXT("Nose monster"),
    TEXT("autonomous tuning, zone paths, fire-shot list and switch"),
    MakeArrayView(NoseMonsterBehaviours),
    TConstArrayView<FKyaActorFamilyReference>(),
};
