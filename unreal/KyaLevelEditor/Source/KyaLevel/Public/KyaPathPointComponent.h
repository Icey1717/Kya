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

    /** Write this marker's relative location back to the owning bank path point. */
    UFUNCTION(BlueprintCallable, Category="Kya|Path")
    void WriteBackToBank();

    /** Move the marker and write the new location to the bank path point. */
    UFUNCTION(BlueprintCallable, Category="Kya|Path")
    void SetMarkerLocation(const FVector& NewRelativeLocation);

#if WITH_EDITOR
    virtual void PostEditComponentMove(bool bFinished) override;
#endif
};
