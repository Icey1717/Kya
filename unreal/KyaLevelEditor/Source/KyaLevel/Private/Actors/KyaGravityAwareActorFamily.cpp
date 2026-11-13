#include "KyaActorFamily.h"

/**
 * Recovered metadata and codec registration for Gravity aware (class 0x10).
 *
 * The class and behaviour 3/4 codecs live in Actors/KyaGravity.h/.cpp.
 * BuildBehaviour maps 3 to CBehaviourGravityAwareFall and 4 to
 * CBehaviourGravityAwareBell; ids 0/1/2 are inherited.
 */
namespace
{
const FKyaActorFamilyBehaviour GravityAwareBehaviours[] = {
    {0, TEXT("Stand")},
    {1, TEXT("Cinematic")},
    {2, TEXT("Catch by trap")},
    {3, TEXT("Gravity fall")},
    {4, TEXT("Gravity bell")},
};
}

extern const FKyaActorFamily GKyaGravityAwareActorFamily = {
    0x10, TEXT("CActorGravityAware"), TEXT("Gravity aware"),
    TEXT("autonomous tuning and fall/bell behaviour payloads"),
    MakeArrayView(GravityAwareBehaviours),
    TConstArrayView<FKyaActorFamilyReference>(),
};
