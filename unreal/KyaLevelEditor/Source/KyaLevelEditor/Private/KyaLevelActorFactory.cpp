#include "KyaLevelActorFactory.h"

#include "KyaLevelBank.h"
#include "KyaLevelPreview.h"

UKyaLevelPreviewFactory::UKyaLevelPreviewFactory()
{
    DisplayName = NSLOCTEXT("KyaLevelPreviewFactory", "DisplayName", "Kya Level Preview");
    NewActorClass = AKyaLevelPreview::StaticClass();
    NewActorClassName = AKyaLevelPreview::StaticClass()->GetName();
    bUseSurfaceOrientation = false;
    bShouldAutoRegister = true;
}

bool UKyaLevelPreviewFactory::CanCreateActorFrom(const FAssetData& AssetData, FText& OutErrorMsg)
{
    if (!AssetData.IsValid() || !AssetData.IsInstanceOf(UKyaLevelBank::StaticClass()))
    {
        OutErrorMsg = NSLOCTEXT("KyaLevelPreviewFactory", "NoBank",
            "A Kya bank asset must be selected to place a level preview.");
        return false;
    }
    return true;
}

AActor* UKyaLevelPreviewFactory::SpawnActor(UObject* InAsset, ULevel* InLevel, const FTransform& InTransform,
    const FActorSpawnParameters& InSpawnParams)
{
    AActor* NewActor = Super::SpawnActor(InAsset, InLevel, InTransform, InSpawnParams);
    if (AKyaLevelPreview* Preview = Cast<AKyaLevelPreview>(NewActor))
    {
        Preview->Bank = Cast<UKyaLevelBank>(InAsset);
        Preview->OnConstruction(Preview->GetActorTransform());
    }
    return NewActor;
}
