#pragma once
#include "Components/ArrowComponent.h"
#include "KyaPathPointComponent.generated.h"

UCLASS(NotBlueprintable, HideCategories=(Rendering, Collision, Physics))
class KYALEVEL_API UKyaPathPointComponent : public UArrowComponent
{
    GENERATED_BODY()
public:
    UPROPERTY(VisibleAnywhere, Category="Path") int32 PathIndex = INDEX_NONE;
    UPROPERTY(VisibleAnywhere, Category="Path") int32 PointIndex = INDEX_NONE;
#if WITH_EDITOR
    virtual void PostEditComponentMove(bool bFinished) override;
#endif
};
