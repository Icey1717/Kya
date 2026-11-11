#pragma once
#include "CoreMinimal.h"

struct FKyaSectorRenderSettings
{
    uint32 BackgroundFlags = 0;
    float ClipDistance = 0;
    float AnimatedHierarchyClipDistance = 0;
};

namespace KyaScene
{
    /** Read CSectorManager::Level_AddAll's trailing CSBS chunk from a scene entry. */
    KYALEVEL_API bool ReadSectorRenderSettings(TConstArrayView<uint8> Scene, TMap<int32, FKyaSectorRenderSettings>& Out);
    /** Native cluster object test, followed by strip testing only when necessary.
     * Spheres are in camera axes (right, up, forward) and game units. */
    KYALEVEL_API bool IsClusterStripVisible(const FVector4f& Object, const FVector4f& Strip,
        double TanHalfFovX, double TanHalfFovY, float FarDistance);
}
