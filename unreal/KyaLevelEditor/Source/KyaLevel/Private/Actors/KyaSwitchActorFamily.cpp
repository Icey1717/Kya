#include "KyaActorFamily.h"

/**
 * Recovered metadata and codec registration for Switch (class 0x11).
 *
 * The target-switch class payload and behaviour 2-8 codecs live in
 * Actors/KyaSwitch.h/.cpp. Ids 0 (Stand) and 1 (Cinematic) are inherited from
 * CActor.
 */
namespace
{
const FKyaActorFamilyBehaviour SwitchBehaviours[] = {
    {0, TEXT("Stand")},
    {1, TEXT("Cinematic")},
    {2, TEXT("Switch lever")},
    {3, TEXT("Switch magic")},
    {4, TEXT("Wolfen counter")},
    {5, TEXT("Switch multi-condition")},
    {6, TEXT("Switch target")},
    {7, TEXT("Switch sequence")},
    {8, TEXT("Switch timed")},
};
}

extern const FKyaActorFamily GKyaSwitchActorFamily = {
    0x11, TEXT("CActorSwitch"), TEXT("Switch"),
    TEXT("target list and camera event; lever/magic/counter/multi/target/sequence/timed behaviours"),
    MakeArrayView(SwitchBehaviours),
    TConstArrayView<FKyaActorFamilyReference>(),
};
