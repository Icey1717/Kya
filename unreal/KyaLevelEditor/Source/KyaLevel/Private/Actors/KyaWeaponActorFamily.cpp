#include "KyaActorFamily.h"

/**
 * Recovered metadata and codec registration for Weapon (class 0x20).
 *
 * The 12-byte class payload and behaviour 3/4/6 codecs live in
 * Actors/KyaWeapon.h/.cpp. Behaviour 2 (default) has an empty Create; ids 0/1
 * are inherited.
 */
namespace
{
const FKyaActorFamilyBehaviour WeaponBehaviours[] = {
    {0, TEXT("Stand")},
    {1, TEXT("Cinematic")},
    {2, TEXT("Weapon default")},
    {3, TEXT("Pistol")},
    {4, TEXT("Sniper")},
    {6, TEXT("RPG")},
};
}

extern const FKyaActorFamily GKyaWeaponActorFamily = {
    0x20, TEXT("CActorWeapon"), TEXT("Weapon"),
    TEXT("class tuning and pistol/sniper/RPG burst payloads"),
    MakeArrayView(WeaponBehaviours),
    TConstArrayView<FKyaActorFamilyReference>(),
};
