#include "KyaActorFamily.h"
#include "Actors/KyaHedgehog.h"

/**
 * CActorHedgehog (class 0x22).
 *
 * Create reads the autonomous prefix, nine scalars, a CAddOnGenerator and a
 * trailing integer. BuildBehaviour maps 3/4/5/6 to GuardArea/WatchDog/
 * GuardAreaArmor/WatchDogArmor; every other id delegates to CActorAutonomous.
 * GuardArea and GuardAreaArmor store a path-follow index; WatchDog and
 * WatchDogArmor have genuinely empty payloads.
 */
namespace
{
const FKyaActorFamilyBehaviour HedgehogBehaviours[] = {
    {3, TEXT("Guard area")},
    {4, TEXT("Watch dog")},
    {5, TEXT("Guard area armor")},
    {6, TEXT("Watch dog armor")},
};
}

extern const FKyaActorFamily GKyaHedgehogActorFamily = {
    0x22, TEXT("CActorHedgehog"), TEXT("Hedgehog"),
    TEXT("walk/run tuning, CAddOnGenerator, guard-area path and watchdog behaviours"),
    MakeArrayView(HedgehogBehaviours),
    // HedgehogPathFollowIndex is a nested behaviour field, not a top-level
    // placement reference, so it is not registered here.
    TConstArrayView<FKyaActorFamilyReference>(),
};
