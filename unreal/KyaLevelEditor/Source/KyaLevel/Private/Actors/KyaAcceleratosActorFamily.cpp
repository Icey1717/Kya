#include "KyaActorFamily.h"
namespace
{
const FKyaActorFamilyBehaviour Behaviours[] = {
    {0, TEXT("Stand")}, {1, TEXT("Cinematic")}, {2, TEXT("Acceleratos stand")},
};
}
extern const FKyaActorFamily GKyaAcceleratosActorFamily = {
    0x3a, TEXT("CActorAcceleratos"), TEXT("Acceleratos"),
    TEXT("Packed 20-byte configuration: three floats and two sound references; Stand has no payload"),
    MakeArrayView(Behaviours), {},
};
