#pragma once
#include "Components/ArrowComponent.h"
#include "KyaActorMarkerComponent.generated.h"

UCLASS(NotBlueprintable, HideCategories=(Rendering, Collision, Physics))
class KYALEVEL_API UKyaActorMarkerComponent : public UArrowComponent
{
    GENERATED_BODY()
public:
    UPROPERTY(VisibleAnywhere, Category="Actor") int32 ActorArrayIndex = INDEX_NONE;
#if WITH_EDITOR
    virtual void PostEditComponentMove(bool bFinished) override;
#endif
};
