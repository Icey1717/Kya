#include "KyaActorFamily.h"

/**
 * Recovered metadata and codec registration for Brazul (class 0x4F).
 *
 * The class codec lives in Actors/KyaBrazul.h/.cpp; the Wolfen prefix is located
 * with the shared reader and left untouched. BuildBehaviour adds ids 3 (fighter
 * default), 26 (BrazulA) and 27 (BrazulB); ids 0/1/2 and the Wolfen ids are
 * inherited.
 */
namespace
{
const FKyaActorFamilyBehaviour BrazulBehaviours[] = {
    {0, TEXT("Stand")},
    {1, TEXT("Cinematic")},
    {2, TEXT("Catch by trap")},
    {3, TEXT("Fighter default")},
    {4, TEXT("Fighter projected")},
    {5, TEXT("Fighter ridden")},
    {6, TEXT("Fighter slave")},
    {8, TEXT("Watch dog")},
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
    {26, TEXT("Brazul A")},
    {27, TEXT("Brazul B")},
};
}

extern const FKyaActorFamily GKyaBrazulActorFamily = {
    0x4f, TEXT("CActorBrazul"), TEXT("Brazul"),
    TEXT("Wolfen prefix, sparks/fx handles, BrazulA sub-objects and BrazulB switch"),
    MakeArrayView(BrazulBehaviours),
    TConstArrayView<FKyaActorFamilyReference>(),
};
