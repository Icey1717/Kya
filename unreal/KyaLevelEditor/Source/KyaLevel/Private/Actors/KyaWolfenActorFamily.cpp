#include "KyaActorFamily.h"

/**
 * Recovered metadata and codec registration for Wolfen (class 0x09).
 *
 * The class/fight-parameter and behaviour 8/10/13/14/15/17/18/19/21/22/23/24
 * codecs live in Actors/KyaWolfen.h/.cpp. The fighter prefix is shared with Kim
 * through Actors/KyaFighter.h. Track-stand (16) and grind-death (25) stay raw.
 */
namespace
{
const FKyaActorFamilyBehaviour WolfenBehaviours[] = {
    {0, TEXT("Stand")},
    {1, TEXT("Cinematic")},
    {2, TEXT("Catch by trap")},
    {3, TEXT("Fighter default")},
    {4, TEXT("Fighter projected")},
    {5, TEXT("Fighter ridden")},
    {6, TEXT("Fighter slave")},
    {8, TEXT("Watch dog")},
    {10, TEXT("Guard area")},
    {12, TEXT("Sleep")},
    {13, TEXT("Escape")},
    {14, TEXT("Exorcism")},
    {15, TEXT("Track")},
    {16, TEXT("Track stand")},
    {17, TEXT("Track weapon")},
    {18, TEXT("Track weapon stand")},
    {19, TEXT("Track weapon snipe")},
    {21, TEXT("Snipe")},
    {22, TEXT("Lost")},
    {23, TEXT("Wolfen DCA")},
    {24, TEXT("Avoid")},
    {25, TEXT("Grind death")},
};
}

extern const FKyaActorFamily GKyaWolfenActorFamily = {
    0x09, TEXT("CActorWolfen"), TEXT("Wolfen"),
    TEXT("shared fighter prefix, vision/detection, fight parameters and Wolfen behaviours"),
    MakeArrayView(WolfenBehaviours),
    TConstArrayView<FKyaActorFamilyReference>(),
};
