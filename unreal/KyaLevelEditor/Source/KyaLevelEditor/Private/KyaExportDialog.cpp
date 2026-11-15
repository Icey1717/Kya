#include "KyaExportDialog.h"

#include "KyaLevelBank.h"
#include "KyaLevelEditorActions.h"
#include "DesktopPlatformModule.h"
#include "IDesktopPlatform.h"
#include "Framework/Application/SlateApplication.h"
#include "HAL/PlatformProcess.h"
#include "Misc/Paths.h"
#include "UObject/StrongObjectPtr.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/SWindow.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
class SKyaExportDialog : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SKyaExportDialog) {}
    SLATE_END_ARGS()

    void Construct(const FArguments&, const TArray<UKyaLevelBank*>& InBanks,
        const FString& InitialDirectory, TSharedRef<SWindow> InWindow)
    {
        Window = InWindow;
        TArray<FString> Names;
        for (UKyaLevelBank* Bank : InBanks)
        {
            Banks.Emplace(Bank);
            Names.Add(Bank->GetName() + TEXT(".bnk"));
        }
        ChildSlot.Padding(16)
        [
            SNew(SVerticalBox)
            + SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 10)
            [ SNew(STextBlock).Text(FText::FromString(FString::Printf(
                TEXT("%s — export %d bank(s), including associated sectors."), *KyaGetPreviewScopeText(), Banks.Num())))
                .AutoWrapText(true) ]
            + SVerticalBox::Slot().AutoHeight()
            [ SNew(STextBlock).Text(FText::FromString(TEXT("Output folder"))) ]
            + SVerticalBox::Slot().AutoHeight().Padding(0, 4, 0, 10)
            [
                SNew(SHorizontalBox)
                + SHorizontalBox::Slot().FillWidth(1)
                [ SAssignNew(Directory, SEditableTextBox).Text(FText::FromString(InitialDirectory)) ]
                + SHorizontalBox::Slot().AutoWidth().Padding(8, 0, 0, 0)
                [ SNew(SButton).Text(FText::FromString(TEXT("Browse…"))).OnClicked(this, &SKyaExportDialog::Browse) ]
            ]
            + SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 10)
            [ SNew(STextBlock).Text(FText::FromString(
                TEXT("Exports current in-memory edits. Every bank is validated before writing. Existing output files will be replaced.")))
                .AutoWrapText(true) ]
            + SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 10)
            [ SNew(SCheckBox).OnCheckStateChanged_Lambda([this](ECheckBoxState State)
                { bOpenFolder = State == ECheckBoxState::Checked; })
                [ SNew(STextBlock).Text(FText::FromString(TEXT("Open output folder after successful export"))) ] ]
            + SVerticalBox::Slot().FillHeight(1).Padding(0, 0, 0, 12)
            [ SAssignNew(Results, SMultiLineEditableTextBox).IsReadOnly(true)
                .Text(FText::FromString(TEXT("Files to export:\n") + FString::Join(Names, TEXT("\n")))) ]
            + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Right)
            [
                SNew(SHorizontalBox)
                + SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 8, 0)
                [ SNew(SButton).Text(FText::FromString(TEXT("Validate only")))
                    .OnClicked(this, &SKyaExportDialog::Validate) ]
                + SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 8, 0)
                [ SNew(SButton).Text(FText::FromString(TEXT("Export")))
                    .IsEnabled_Lambda([this]() { return !Directory->GetText().ToString().TrimStartAndEnd().IsEmpty(); })
                    .OnClicked(this, &SKyaExportDialog::Export) ]
                + SHorizontalBox::Slot().AutoWidth()
                [ SNew(SButton).Text(FText::FromString(TEXT("Close"))).OnClicked_Lambda([this]()
                    { if (TSharedPtr<SWindow> Parent = Window.Pin()) Parent->RequestDestroyWindow(); return FReply::Handled(); }) ]
            ]
        ];
    }

private:
    TArray<TStrongObjectPtr<UKyaLevelBank>> Banks;
    TWeakPtr<SWindow> Window;
    TSharedPtr<SEditableTextBox> Directory;
    TSharedPtr<SMultiLineEditableTextBox> Results;
    bool bOpenFolder = false;

    TArray<UKyaLevelBank*> GetBanks() const
    {
        TArray<UKyaLevelBank*> Result;
        for (const TStrongObjectPtr<UKyaLevelBank>& Bank : Banks) Result.Add(Bank.Get());
        return Result;
    }

    FReply Browse()
    {
        if (IDesktopPlatform* Desktop = FDesktopPlatformModule::Get())
        {
            FString Selected;
            if (Desktop->OpenDirectoryDialog(FSlateApplication::Get().FindBestParentWindowHandleForDialogs(AsShared()),
                TEXT("Choose Kya export folder"), Directory->GetText().ToString(), Selected))
                Directory->SetText(FText::FromString(Selected));
        }
        return FReply::Handled();
    }

    FReply Validate()
    {
        TArray<FString> Lines;
        for (const FKyaBankValidationReport& Report : KyaValidateLevelBanks(GetBanks())) Lines.Add(Report.Summary());
        Results->SetText(FText::FromString(TEXT("Validation only — no files written.\n\n") + FString::Join(Lines, TEXT("\n\n"))));
        return FReply::Handled();
    }

    FReply Export()
    {
        const FString Output = FPaths::ConvertRelativePathToFull(Directory->GetText().ToString().TrimStartAndEnd());
        Directory->SetText(FText::FromString(Output));
        TArray<FString> Messages;
        const bool bSuccess = KyaExportLevelBanks(GetBanks(), Output, Messages);
        Results->SetText(FText::FromString(FString(bSuccess ? TEXT("Export complete.\n") : TEXT("Export did not complete. Review the details below.\n"))
            + TEXT("Output: ") + Output + TEXT("\n\n") + FString::Join(Messages, TEXT("\n"))));
        if (bSuccess && bOpenFolder) FPlatformProcess::ExploreFolder(*Output);
        return FReply::Handled();
    }
};
}

void KyaShowExportDialog(const TArray<UKyaLevelBank*>& Banks, const FString& InitialDirectory)
{
    TSharedRef<SWindow> Window = SNew(SWindow).Title(FText::FromString(TEXT("Export Kya Level")))
        .ClientSize(FVector2D(720, 520)).SupportsMinimize(false).SupportsMaximize(false);
    Window->SetContent(SNew(SKyaExportDialog, Banks, InitialDirectory, Window));
    FSlateApplication::Get().AddModalWindow(Window, FSlateApplication::Get().GetActiveTopLevelWindow());
}
