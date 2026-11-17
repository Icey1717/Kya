#include "KyaLevelPreviewDetailsCustomization.h"

#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "KyaLevelBank.h"
#include "KyaLevelPreview.h"
#include "ScopedTransaction.h"
#include "ProfilingDebugging/CpuProfilerTrace.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/SBoxPanel.h"
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
    if (!Ids.IsEmpty())
        Category.AddCustomRow(FText::FromString(TEXT("Sector visibility summary")))
            .NameContent()
            [
                SNew(STextBlock).Text_Lambda([WeakPreview, Ids]() {
                    int32 Visible = 0;
                    if (WeakPreview.IsValid())
                        for (int32 Id : Ids)
                            if (WeakPreview->IsSectorVisible(Id)) ++Visible;
                    return FText::FromString(FString::Printf(TEXT("%d of %d sectors visible"), Visible, Ids.Num()));
                })
            ]
            .ValueContent().MinDesiredWidth(180)
            [
                SNew(SButton)
                .Text(FText::FromString(TEXT("Restore Previous Visibility")))
                .ToolTipText(FText::FromString(TEXT("Restore the visibility from before the first Solo click, including any sectors that were already hidden.")))
                .IsEnabled_Lambda([WeakPreview]() {
                    return WeakPreview.IsValid() && WeakPreview->CanRestoreSectorVisibility();
                })
                .OnClicked_Lambda([WeakPreview]() {
                    if (WeakPreview.IsValid())
                    {
                        const FScopedTransaction Transaction(FText::FromString(TEXT("Restore Sector Visibility")));
                        WeakPreview->RestoreSectorVisibility();
                    }
                    return FReply::Handled();
                })
            ];
    for (const int32 Id : Ids)
    {
        const FString Label = FString::Printf(TEXT("Sector %d (%s)"), Id, *Sectors[Id]);
        Category.AddCustomRow(FText::FromString(Label))
            .NameContent()[SNew(STextBlock).Text(FText::FromString(Label))]
            .ValueContent()
            [
                SNew(SHorizontalBox)
                + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
                [SNew(SCheckBox)
                .ToolTipText(FText::FromString(TEXT("Show this sector's scenery and actors in the preview. Banks stay associated; exports are unchanged.")))
                .IsChecked_Lambda([WeakPreview, Id]() {
                    return WeakPreview.IsValid() && WeakPreview->IsSectorVisible(Id)
                        ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
                })
                .OnCheckStateChanged_Lambda([WeakPreview, Id](ECheckBoxState State) {
                    if (!WeakPreview.IsValid()) return;
                    // Includes transaction finalization after SetSectorVisible returns.
                    TRACE_CPUPROFILER_EVENT_SCOPE_TEXT(*FString::Printf(TEXT("Kya.SectorCheckbox sector=%d visible=%d"),
                        Id, State == ECheckBoxState::Checked));
                    const FScopedTransaction Transaction(FText::FromString(TEXT("Change Sector Visibility")));
                    WeakPreview->SetSectorVisible(Id, State == ECheckBoxState::Checked);
                })]
                + SHorizontalBox::Slot().AutoWidth().Padding(12, 0, 0, 0)
                [
                    SNew(SButton)
                    .Text(FText::FromString(TEXT("Solo")))
                    .ToolTipText(FText::FromString(TEXT("Show only this sector. Global content stays visible. Solo another sector or tick additional checkboxes, then Restore Previous Visibility when finished.")))
                    .OnClicked_Lambda([WeakPreview, Id]() {
                        if (WeakPreview.IsValid())
                        {
                            const FScopedTransaction Transaction(FText::FromString(TEXT("Solo Sector")));
                            WeakPreview->SoloSector(Id);
                        }
                        return FReply::Handled();
                    })
                ]
            ];
    }
    if (Ids.IsEmpty())
        Category.AddCustomRow(FText::FromString(TEXT("No sectors"))).WholeRowContent()
        [SNew(STextBlock).Text(FText::FromString(TEXT("Discover sector banks to populate this list.")))];
}
