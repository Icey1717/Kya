#include "KyaActorFamily.h"

/**
 * Recovered metadata and codec registration for Commander (class 0x13).
 *
 * The class and behaviour 2 codecs live in Actors/KyaCommander.h/.cpp.
 * Behaviour 0 creates a default commander; id 3 is BeatUp and keeps its raw
 * payload.
 */
namespace
{
const FKyaActorFamilyBehaviour CommanderBehaviours[] = {
    {0, TEXT("Commander default")},
    {1, TEXT("Cinematic")},
    {2, TEXT("Commander")},
    {3, TEXT("Commander beat up")},
};
const FKyaActorFamilyReference CommanderReferences[] = {
    {TEXT("CommanderActorRef"), EKyaReferenceKind::Actor},
};
}

extern const FKyaActorFamily GKyaCommanderActorFamily = {
    0x13, TEXT("CActorCommander"), TEXT("Commander"),
    TEXT("squad, team, sequence and camera-figure data"),
    MakeArrayView(CommanderBehaviours),
    MakeArrayView(CommanderReferences),
};
