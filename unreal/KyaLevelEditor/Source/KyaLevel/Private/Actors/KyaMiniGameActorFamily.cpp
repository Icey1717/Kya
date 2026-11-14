#include "KyaActorFamily.h"

/**
 * Recovered metadata and codec registration for the minigame family:
 * base 0x44, TimeAttack 0x45, BoxCounter 0x46, Boomy 0x47 and Distance 0x55.
 * The class and behaviour codecs live in Actors/KyaMiniGame.h/.cpp.
 * BuildBehaviour maps id 2 (training, empty), 3 (betting) and 4 (multi,
 * empty); ids 0/1 are inherited.
 */
namespace
{
const FKyaActorFamilyBehaviour MiniGameBehaviours[] = {
    {0, TEXT("Stand")},
    {1, TEXT("Cinematic")},
    {2, TEXT("Minigame training")},
    {3, TEXT("Minigame betting")},
    {4, TEXT("Minigame multi")},
};
}

extern const FKyaActorFamily GKyaMiniGameActorFamily = {
    0x44, TEXT("CActorMiniGame"), TEXT("Minigame"),
    TEXT("score lists, default score, switch and per-variant tail"),
    MakeArrayView(MiniGameBehaviours),
    TConstArrayView<FKyaActorFamilyReference>(),
};

extern const FKyaActorFamily GKyaMiniGameTimeAttackActorFamily = {
    0x45, TEXT("CActorMiniGameTimeAttack"), TEXT("Minigame time attack"),
    TEXT("minigame base plus field 0x1e0"),
    MakeArrayView(MiniGameBehaviours),
    TConstArrayView<FKyaActorFamilyReference>(),
};

extern const FKyaActorFamily GKyaMiniGameBoxCounterActorFamily = {
    0x46, TEXT("CActorMiniGameBoxCounter"), TEXT("Minigame box counter"),
    TEXT("minigame base plus a time limit"),
    MakeArrayView(MiniGameBehaviours),
    TConstArrayView<FKyaActorFamilyReference>(),
};

extern const FKyaActorFamily GKyaMiniGameBoomyActorFamily = {
    0x47, TEXT("CActorMiniGameBoomy"), TEXT("Minigame boomy"),
    TEXT("minigame base plus target, penalty and sequence lists"),
    MakeArrayView(MiniGameBehaviours),
    TConstArrayView<FKyaActorFamilyReference>(),
};

extern const FKyaActorFamily GKyaMiniGameDistanceActorFamily = {
    0x55, TEXT("CActorMiniGameDistance"), TEXT("Minigame distance"),
    TEXT("minigame base plus two path readers"),
    MakeArrayView(MiniGameBehaviours),
    TConstArrayView<FKyaActorFamilyReference>(),
};
