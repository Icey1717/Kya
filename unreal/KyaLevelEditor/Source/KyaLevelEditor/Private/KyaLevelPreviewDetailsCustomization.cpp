#include "KyaLevelPreviewDetailsCustomization.h"

#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "KyaLevelBank.h"
#include "KyaLevelPreview.h"
#include "ScopedTransaction.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Text/STextBlock.h"

TSharedRef<IDetailCustomization> FKyaLevelPreviewDetailsCustomization::MakeInstance()
{
    return MakeShared<FKyaLevelPreviewDetailsCustomization>();
}

void FKyaLevelPreviewDetailsCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
    TArray<TWeakObjectPtr<UObject>> Selected;
    DetailBuilder.GetObjectsBeingCustomized(Selected);
    if (Selected.Num() != 1) return;
    AKyaLevelPreview* Preview = Cast<AKyaLevelPreview>(Selected[0].Get());
    if (!Preview) return;
    Preview->ApplyLevelSet();

    IDetailCategoryBuilder& Category = DetailBuilder.EditCategory("Kya|Sectors",
        FText::FromString(TEXT("Sector Visibility")), ECategoryPriority::Important);
    TMap<int32, FString> Sectors;
    for (const UKyaLevelBank* Sector : Preview->SectorBanks)
        if (Sector && Sector != Preview->Bank)
        {
            const int32 Id = Preview->ResolveSectorId(*Sector);
            if (Id >= 0) Sectors.Add(Id, Sector->GetName());
        }
    if (Preview->Bank)
        for (const FKyaActorPlacement& Actor : Preview->Bank->SceneEnvironment.Actors)
            if (Actor.SectorId >= 0 && !Sectors.Contains(Actor.SectorId))
                Sectors.Add(Actor.SectorId, TEXT("not loaded"));
    TArray<int32> Ids;
    Sectors.GetKeys(Ids);
    Ids.Sort();
    const TWeakObjectPtr<AKyaLevelPreview> WeakPreview = Preview;
    for (const int32 Id : Ids)
    {
        const FString Label = FString::Printf(TEXT("Sector %d (%s)"), Id, *Sectors[Id]);
        Category.AddCustomRow(FText::FromString(Label))
            .NameContent()[SNew(STextBlock).Text(FText::FromString(Label))]
            .ValueContent()
            [
                SNew(SCheckBox)
                .ToolTipText(FText::FromString(TEXT("Show this sector's scenery and actors in the preview. Banks stay associated; exports are unchanged.")))
                .IsChecked_Lambda([WeakPreview, Id]() {
                    return WeakPreview.IsValid() && WeakPreview->IsSectorVisible(Id)
                        ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
                })
                .OnCheckStateChanged_Lambda([WeakPreview, Id](ECheckBoxState State) {
                    if (!WeakPreview.IsValid()) return;
                    const FScopedTransaction Transaction(FText::FromString(TEXT("Change Sector Visibility")));
                    WeakPreview->SetSectorVisible(Id, State == ECheckBoxState::Checked);
                })
            ];
    }
    if (Ids.IsEmpty())
        Category.AddCustomRow(FText::FromString(TEXT("No sectors"))).WholeRowContent()
        [SNew(STextBlock).Text(FText::FromString(TEXT("Discover sector banks to populate this list.")))];
}
