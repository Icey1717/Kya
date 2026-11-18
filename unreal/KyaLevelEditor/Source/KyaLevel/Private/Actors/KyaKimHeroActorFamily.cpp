#include "KyaActorFamily.h"

/**
 * Recovered metadata and codec registration for Kim / hero (class 0x06).
 *
 * The class payload codec lives in Actors/KyaKim.h/.cpp. Behaviours 9/10/11 are
 * the HeroUnknown variants already decoded elsewhere; 12 is RideJamGut.
 */
namespace
{
const FKyaActorFamilyBehaviour KimHeroBehaviours[] = {
    {0, TEXT("Stand")},
    {1, TEXT("Cinematic")},
    {2, TEXT("Catch by trap")},
    {3, TEXT("Hero default")},
    {4, TEXT("Hero projected")},
    {5, TEXT("Hero ridden")},
    {6, TEXT("Hero slave")},
    {9, TEXT("Hero unknown behaviour 9")},
    {10, TEXT("Hero unknown behaviour 10")},
    {11, TEXT("Hero unknown behaviour 11")},
    {12, TEXT("Hero ride JamGut")},
};
const FKyaActorFamilyReference KimHeroReferences[] = {
    {TEXT("FighterWeaponActorIndex"), EKyaReferenceKind::Actor},
};
}

extern const FKyaActorFamily GKyaKimHeroActorFamily = {
    0x06, TEXT("CActorHeroPrivate"), TEXT("Kim / hero"),
    TEXT("shared CActorFighter prefix, bone keys, climb/pair/zone streams, tutorial/projectile zones and camera records (all rebuilt on export), plus FX handles"),
    MakeArrayView(KimHeroBehaviours),
    MakeArrayView(KimHeroReferences),
};
