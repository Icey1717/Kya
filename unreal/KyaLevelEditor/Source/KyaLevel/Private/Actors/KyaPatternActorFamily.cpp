#include "KyaActorFamily.h"

/**
 * Recovered metadata and codec registration for the pattern controller
 * (class 0x3F, `PATTERN_CON`; bank actors are named LINER).
 *
 * The class and behaviour 3 codecs live in Actors/KyaPattern.h/.cpp.
 * BuildBehaviour adds id 3 (alone) and 4 (multiple); id 4's Create is empty.
 * Ids 0/1 are inherited and id 2 is the shared catch-by-trap behaviour.
 */
namespace
{
const FKyaActorFamilyBehaviour PatternBehaviours[] = {
    {0, TEXT("Stand")},
    {1, TEXT("Cinematic")},
    {2, TEXT("Catch by trap")},
    {3, TEXT("Pattern alone")},
    {4, TEXT("Pattern multiple")},
};
}

extern const FKyaActorFamily GKyaPatternActorFamily = {
    0x3f, TEXT("CActorPattern"), TEXT("Pattern controller"),
    TEXT("autonomous tuning, pattern part and add-on generator"),
    MakeArrayView(PatternBehaviours),
    TConstArrayView<FKyaActorFamilyReference>(),
};
