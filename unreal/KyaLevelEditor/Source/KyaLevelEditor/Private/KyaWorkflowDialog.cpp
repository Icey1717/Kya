#include "KyaWorkflowDialog.h"
#include "KyaLevelEditorActions.h"
#include "KyaLevelBank.h"
#include "KyaLevelPreview.h"
#include "KyaLevelSet.h"
#include "Editor.h"
#include "Framework/Application/SlateApplication.h"
#include "Misc/Paths.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/SWindow.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
class SKyaWorkflowDialog : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SKyaWorkflowDialog) {} SLATE_END_ARGS()
    void Construct(const FArguments&, const TArray<AKyaLevelPreview*>& Targets,
        bool bInDiscovery, bool bInImportMissing, TSharedRef<SWindow> Parent)
    {
        bDiscovery = bInDiscovery;
        bImportMissing = bInImportMissing;
        Window = Parent;
        for (AKyaLevelPreview* Preview : Targets) Previews.Add(Preview);
        ChildSlot.Padding(16)
        [ SNew(SVerticalBox)
            + SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 12)
            [ SNew(STextBlock).Text(FText::FromString(KyaGetPreviewScopeText())) ]
            + SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 12)
            [ SNew(STextBlock).AutoWrapText(true).Text(FText::FromString(bDiscovery
                ? TEXT("Review source files and missing imports below. Apply refreshes sector associations and optionally imports missing banks; existing bank edits are preserved.")
                : TEXT("Review the asset destinations below. Apply creates or updates level-set assets from the previews' current bank associations. Use Browse to Asset afterward, then save the level-set assets in the Content Browser."))) ]
            + SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 12)
            [ SNew(SCheckBox).Visibility(bDiscovery ? EVisibility::Visible : EVisibility::Collapsed)
                .IsChecked(bImportMissing ? ECheckBoxState::Checked : ECheckBoxState::Unchecked)
                .OnCheckStateChanged_Lambda([this](ECheckBoxState State) { bImportMissing = State == ECheckBoxState::Checked; })
                [ SNew(STextBlock).Text(FText::FromString(TEXT("Import missing banks"))) ] ]
            + SVerticalBox::Slot().FillHeight(1).Padding(0, 0, 0, 12)
            [ SAssignNew(Results, SMultiLineEditableTextBox).IsReadOnly(true) ]
            + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Right)
            [ SNew(SHorizontalBox)
                + SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 8, 0)
                [ SNew(SButton).Text(FText::FromString(TEXT("Refresh preview"))).OnClicked_Lambda([this]() { Refresh(); return FReply::Handled(); }) ]
                + SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 8, 0)
                [ SNew(SButton).Text(FText::FromString(TEXT("Browse to Asset")))
                    .Visibility(bDiscovery ? EVisibility::Collapsed : EVisibility::Visible)
                    .IsEnabled_Lambda([this]() { return !CreatedAssets.IsEmpty(); })
                    .OnClicked_Lambda([this]()
                    {
                        TArray<UObject*> Assets;
                        for (const TWeakObjectPtr<UKyaLevelSet>& Asset : CreatedAssets) if (Asset.IsValid()) Assets.Add(Asset.Get());
                        if (GEditor && !Assets.IsEmpty()) GEditor->SyncBrowserToObjects(Assets);
                        return FReply::Handled();
                    }) ]
                + SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 8, 0)
                [ SNew(SButton).Text(FText::FromString(TEXT("Apply"))).OnClicked_Lambda([this]() { Apply(); return FReply::Handled(); }) ]
                + SHorizontalBox::Slot().AutoWidth()
                [ SNew(SButton).Text(FText::FromString(TEXT("Close"))).OnClicked_Lambda([this]()
                    { if (TSharedPtr<SWindow> ParentWindow = Window.Pin()) ParentWindow->RequestDestroyWindow(); return FReply::Handled(); }) ]
            ]
        ];
        Refresh();
    }
private:
    TArray<TWeakObjectPtr<AKyaLevelPreview>> Previews;
    TArray<TWeakObjectPtr<UKyaLevelSet>> CreatedAssets;
    TWeakPtr<SWindow> Window;
    TSharedPtr<SMultiLineEditableTextBox> Results;
    bool bDiscovery = false;
    bool bImportMissing = true;

    void Refresh()
    {
        TArray<FString> Lines;
        Lines.Add(TEXT("Preview only — no associations changed and no assets imported or created."));
        for (const TWeakObjectPtr<AKyaLevelPreview>& Target : Previews)
        {
            AKyaLevelPreview* Preview = Target.Get();
            if (!Preview) { Lines.Add(TEXT("Preview is no longer available.")); continue; }
            Lines.Add(Preview->GetActorLabel());
            if (bDiscovery)
            {
                Lines.Add(TEXT("Source folder: ") + (Preview->Bank ? FPaths::GetPath(Preview->Bank->SourceFilename) : TEXT("No main bank")));
                Lines.Add(KyaDiscoverSectors(Preview, false, true).Summary());
            }
            else
            {
                const FString Destination = KyaGetLevelSetDestination(Preview);
                Lines.Add(Destination.IsEmpty() ? TEXT("No valid destination; assign a main bank first.") : Destination);
            }
        }
        Results->SetText(FText::FromString(FString::Join(Lines, TEXT("\n\n"))));
    }

    void Apply()
    {
        TArray<FString> Lines;
        CreatedAssets.Reset();
        for (const TWeakObjectPtr<AKyaLevelPreview>& Target : Previews)
        {
            AKyaLevelPreview* Preview = Target.Get();
            if (!Preview) { Lines.Add(TEXT("Preview is no longer available.")); continue; }
            if (bDiscovery) Lines.Add(Preview->GetActorLabel() + TEXT(":\n") + KyaDiscoverSectors(Preview, bImportMissing).Summary());
            else
            {
                FString Error;
                if (UKyaLevelSet* Asset = KyaCreateLevelSetFromPreview(Preview, Error))
                {
                    CreatedAssets.AddUnique(Asset);
                    Lines.Add(TEXT("Created / updated: ") + Asset->GetPathName());
                }
                else Lines.Add(Error);
            }
        }
        Results->SetText(FText::FromString(FString::Join(Lines, TEXT("\n\n"))));
    }
};

void ShowWorkflow(const TArray<AKyaLevelPreview*>& Previews, bool bDiscovery, bool bImportMissing)
{
    TSharedRef<SWindow> Window = SNew(SWindow).Title(FText::FromString(bDiscovery
        ? TEXT("Refresh Sector Discovery") : TEXT("Create / Update Kya Level Set")))
        .ClientSize(FVector2D(760, 540)).SupportsMinimize(false).SupportsMaximize(false);
    Window->SetContent(SNew(SKyaWorkflowDialog, Previews, bDiscovery, bImportMissing, Window));
    FSlateApplication::Get().AddModalWindow(Window, FSlateApplication::Get().GetActiveTopLevelWindow());
}
}

void KyaShowDiscoveryDialog(const TArray<AKyaLevelPreview*>& Previews, bool bImportMissing) { ShowWorkflow(Previews, true, bImportMissing); }
void KyaShowLevelSetDialog(const TArray<AKyaLevelPreview*>& Previews) { ShowWorkflow(Previews, false, false); }
