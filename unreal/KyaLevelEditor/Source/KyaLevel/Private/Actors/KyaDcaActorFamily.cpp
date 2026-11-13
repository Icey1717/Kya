#include "KyaActorFamily.h"

/**
 * Recovered metadata and codec registration for DCA (class 0x1b).
 *
 * The class payload and behaviour 3/5 codecs live in Actors/KyaDca.h/.cpp.
 * Behaviour 2 (default) has an empty Create; behaviour 4 (AI controlled) is an
 * implementation guard in source and stays raw.
 */
namespace
{
const FKyaActorFamilyBehaviour DcaBehaviours[] = {
    {0, TEXT("Stand")},
    {1, TEXT("Cinematic")},
    {2, TEXT("DCA default")},
    {3, TEXT("DCA controlled")},
    {4, TEXT("DCA AI controlled")},
    {5, TEXT("DCA trajectory")},
};
}

extern const FKyaActorFamily GKyaDcaActorFamily = {
    0x1b, TEXT("CActorDCA"), TEXT("DCA"),
    TEXT("aim/shot tuning and controlled/trajectory behaviours"),
    MakeArrayView(DcaBehaviours),
    TConstArrayView<FKyaActorFamilyReference>(),
};
