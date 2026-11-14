#include "KyaActorFamily.h"

/**
 * Recovered metadata and codec registration for Nativ shop (class 0x34).
 *
 * The class and behaviour 2 codecs live in Actors/KyaNativShop.h/.cpp.
 * BuildBehaviour only adds id 2 (CBehaviourNativShopSell); ids 0/1 are
 * inherited from CActor.
 */
namespace
{
const FKyaActorFamilyBehaviour NativShopBehaviours[] = {
    {0, TEXT("Stand")},
    {1, TEXT("Cinematic")},
    {2, TEXT("Nativ shop sell")},
};
}

extern const FKyaActorFamily GKyaNativShopActorFamily = {
    0x34, TEXT("CActorNativShop"), TEXT("Nativ shop"),
    TEXT("radius, class name, actor list, episode link and sell behaviour"),
    MakeArrayView(NativShopBehaviours),
    TConstArrayView<FKyaActorFamilyReference>(),
};
