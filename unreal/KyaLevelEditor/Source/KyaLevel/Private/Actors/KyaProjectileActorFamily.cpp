#include "KyaActorFamily.h"

/**
 * Recovered metadata and codec registration for Projectile (class 0x1c).
 *
 * The class payload and behaviour 3/4/5/9/10/11/12/13 readers and writers live
 * in Actors/KyaProjectile.h/.cpp. Ids 0 (Stand), 1 (Cinematic) and 2 (Catch by
 * trap) are inherited from CActor/CActorAutonomous and keep their generic
 * decoders.
 */
namespace
{
const FKyaActorFamilyBehaviour ProjectileBehaviours[] = {
    {0, TEXT("Stand")},
    {1, TEXT("Cinematic")},
    {2, TEXT("Catch by trap")},
    {3, TEXT("Projectile stand")},
    {4, TEXT("Projectile straight")},
    {5, TEXT("Projectile portable")},
    {9, TEXT("Inventory")},
    {10, TEXT("Projectile lava ball")},
    {11, TEXT("Projectile excuse")},
    {12, TEXT("Inactive")},
    {13, TEXT("Projectile new")},
};
}

extern const FKyaActorFamily GKyaProjectileActorFamily = {
    0x1c, TEXT("CActorProjectile"), TEXT("Projectile"),
    TEXT("autonomous movement, 0x48-byte tuning sub-object, pattern part and projectile/inventory behaviours"),
    MakeArrayView(ProjectileBehaviours),
    TConstArrayView<FKyaActorFamilyReference>(),
};
