#include "KyaActorFamily.h"

/**
 * Recovered metadata and codec registration for Punching ball (class 0x30).
 *
 * The class codec lives in Actors/KyaPunchingBall.h/.cpp. BuildBehaviour adds
 * ids 3/4/5/6 (default/projected/ridden/slave) and 7 (stand); their Create
 * functions are empty. Ids 0/1 are inherited from CActor.
 */
namespace
{
const FKyaActorFamilyBehaviour PunchingBallBehaviours[] = {
    {0, TEXT("Stand")},
    {1, TEXT("Cinematic")},
    {3, TEXT("Fighter default")},
    {4, TEXT("Fighter projected")},
    {5, TEXT("Fighter ridden")},
    {6, TEXT("Fighter slave")},
    {7, TEXT("Punching ball stand")},
};
}

extern const FKyaActorFamily GKyaPunchingBallActorFamily = {
    0x30, TEXT("CActorPunchingBall"), TEXT("Punching ball"),
    TEXT("fighter prefix, tuning floats, trap stream (rebuilt on export), actor refs and camera fight data"),
    MakeArrayView(PunchingBallBehaviours),
    TConstArrayView<FKyaActorFamilyReference>(),
};
