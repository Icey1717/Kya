#include "KyaActorFamily.h"
namespace
{
const FKyaActorFamilyBehaviour Behaviours[] = {
    {0, TEXT("Stand")}, {1, TEXT("Cinematic")}, {2, TEXT("Add-on purchase")},
};
}
extern const FKyaActorFamily GKyaAddOnPurchaseActorFamily = {
    0x49, TEXT("CActorAddOnPurchase"), TEXT("Add-on purchase"),
    TEXT("CInventoryInfo class payload, including pre-2.27 and optional-object layouts; behaviour 2 is empty"),
    MakeArrayView(Behaviours), {},
};
