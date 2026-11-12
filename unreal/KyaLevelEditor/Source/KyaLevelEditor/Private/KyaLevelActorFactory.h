#pragma once

#include "CoreMinimal.h"
#include "ActorFactories/ActorFactory.h"
#include "KyaLevelActorFactory.generated.h"

/**
 * Places a configured level preview by dragging a Kya bank asset into the viewport
 * (or through the Content Browser's placement menu). The spawned preview points at
 * the dragged bank and builds its scene immediately.
 */
UCLASS()
class UKyaLevelPreviewFactory : public UActorFactory
{
    GENERATED_BODY()
public:
    UKyaLevelPreviewFactory();
    virtual bool CanCreateActorFrom(const FAssetData& AssetData, FText& OutErrorMsg) override;
    virtual AActor* SpawnActor(UObject* InAsset, ULevel* InLevel, const FTransform& InTransform,
        const FActorSpawnParameters& InSpawnParams) override;
};
