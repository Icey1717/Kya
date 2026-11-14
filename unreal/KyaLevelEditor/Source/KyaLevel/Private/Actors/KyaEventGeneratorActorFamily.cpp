#include "KyaActorFamily.h"

/**
 * Recovered metadata and codec registration for Event generator (class 0x3B).
 *
 * The class header and behaviour 2 header codecs live in
 * Actors/KyaEventGenerator.h/.cpp; manager payloads stay raw (see the audit
 * finding). BuildBehaviour only adds id 2 (CBehaviourEventGen); ids 0/1 are
 * inherited.
 */
namespace
{
const FKyaActorFamilyBehaviour EventGeneratorBehaviours[] = {
    {0, TEXT("Stand")},
    {1, TEXT("Cinematic")},
    {2, TEXT("Event generator")},
};
}

extern const FKyaActorFamily GKyaEventGeneratorActorFamily = {
    0x3b, TEXT("CActorEventGenerator"), TEXT("Event generator"),
    TEXT("global flag, zone reference and scripted manager payloads"),
    MakeArrayView(EventGeneratorBehaviours),
    TConstArrayView<FKyaActorFamilyReference>(),
};
