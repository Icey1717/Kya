#pragma once

#include "Components/ArrowComponent.h"
#include "KyaWaypointComponent.generated.h"

/** Editor marker linked to one waypoint in the preview's bank. */
UCLASS(NotBlueprintable, HideCategories=(Rendering, Collision, Physics))
class KYALEVEL_API UKyaWaypointComponent : public UArrowComponent
{
    GENERATED_BODY()
public:
    UPROPERTY(VisibleAnywhere, Category="Waypoint")
    int32 WaypointIndex = INDEX_NONE;

#if WITH_EDITOR
    virtual void PostEditComponentMove(bool bFinished) override;
#endif
};
