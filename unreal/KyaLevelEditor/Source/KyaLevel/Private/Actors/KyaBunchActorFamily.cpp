#include "KyaActorFamily.h"

/**
 * Recovered metadata and codec registration for Bunch (class 0x4E).
 *
 * The class codec lives in Actors/KyaBunch.h/.cpp; the Wolfen prefix is located
 * with the shared Actors/KyaWolfen.h reader and left untouched. BuildBehaviour
 * adds ids 3/4 (empty fighter default/projected), 26 (repel) and 27 (fence);
 * ids 0/1/2 and the Wolfen ids 8/10/13/14/15/17/18/19/21/22/23/24 are inherited.
 */
namespace
{
const FKyaActorFamilyBehaviour BunchBehaviours[] = {
    {0, TEXT("Stand")},
    {1, TEXT("Cinematic")},
    {2, TEXT("Catch by trap")},
    {3, TEXT("Fighter default")},
    {4, TEXT("Fighter projected")},
    {8, TEXT("Watch dog")},
    {10, TEXT("Guard area")},
    {13, TEXT("Escape")},
    {14, TEXT("Exorcism")},
    {15, TEXT("Track")},
    {17, TEXT("Track weapon")},
    {18, TEXT("Track weapon stand")},
    {19, TEXT("Track weapon snipe")},
    {21, TEXT("Snipe")},
    {22, TEXT("Lost")},
    {23, TEXT("Wolfen DCA")},
    {24, TEXT("Avoid")},
    {26, TEXT("Bunch repel")},
    {27, TEXT("Bunch fence")},
};
}

extern const FKyaActorFamily GKyaBunchActorFamily = {
    0x4e, TEXT("CActorBunch"), TEXT("Bunch"),
    TEXT("Wolfen prefix, bunch stream entries, repel indices and fence behaviours (stream/repel/fence lists rebuilt on export)"),
    MakeArrayView(BunchBehaviours),
    TConstArrayView<FKyaActorFamilyReference>(),
};
