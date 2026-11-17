#include "KyaDiscDetails.h"
#include "KyaDisc.h"
#include "KyaDiscWorkflow.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "DetailWidgetRow.h"
#include "Misc/MessageDialog.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBox.h"
#include "Framework/Application/SlateApplication.h"
#include "HAL/PlatformApplicationMisc.h"
#include "HAL/PlatformTime.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/SWindow.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "UObject/StrongObjectPtr.h"
#endif

namespace
{
class SKyaDiscExportOutput : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SKyaDiscExportOutput) {}
    SLATE_END_ARGS()

    void Construct(const FArguments&, UKyaDisc* InDisc, TSharedRef<SWindow> InWindow)
    {
        Disc = InDisc;
        Window = InWindow;
        ChildSlot
        [
            SNew(SVerticalBox)
            + SVerticalBox::Slot().AutoHeight().Padding(12)
            [ SAssignNew(Status, STextBlock).Text(FText::FromString(TEXT("Preparing ISO export..."))) ]
            + SVerticalBox::Slot().FillHeight(1).Padding(12, 0)
            [ SAssignNew(Scroll, SScrollBox)
                + SScrollBox::Slot()
                [ SAssignNew(Text, STextBlock).Text(FText::GetEmpty()).AutoWrapText(true) ] ]
            + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Right).Padding(12)
            [ SNew(SHorizontalBox)
                + SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 8, 0)
                [ SNew(SButton).Text(FText::FromString(TEXT("Copy Output")))
                    .OnClicked_Lambda([this]()
                    { FPlatformApplicationMisc::ClipboardCopy(*Transcript); return FReply::Handled(); }) ]
                + SHorizontalBox::Slot().AutoWidth()
                [ SNew(SButton).Text(FText::FromString(TEXT("Close")))
                    .IsEnabled_Lambda([this]() { return bFinished; })
                    .OnClicked_Lambda([this]()
                    { if (auto Parent = Window.Pin()) Parent->RequestDestroyWindow(); return FReply::Handled(); }) ] ]
        ];
        InWindow->SetRequestDestroyWindowOverride(FRequestDestroyWindowOverride::CreateSP(this,
            &SKyaDiscExportOutput::RequestClose));
    }

    void RunExport()
    {
        StartedAt = FPlatformTime::Seconds();
        FString Report;
        bool bSuccess = false;
        if (Disc.IsValid())
            bSuccess = KyaExportDisc(*Disc, Report, [this](const FString& Message) { Append(Message); });
        else Report = TEXT("The disc asset is no longer available.");
        Append(Report);
        bFinished = true;
        Status->SetText(FText::FromString(bSuccess ? TEXT("Export complete") : TEXT("Export failed — see output below")));
    }

private:
#if WITH_DEV_AUTOMATION_TESTS
    friend class FKyaDiscExportOutputWindowTest;
#endif
    TWeakObjectPtr<UKyaDisc> Disc;
    TWeakPtr<SWindow> Window;
    TSharedPtr<STextBlock> Status, Text;
    TSharedPtr<SScrollBox> Scroll;
    FString Transcript;
    bool bFinished = false;
    double StartedAt = 0;

    void Append(const FString& Message)
    {
        Transcript += FString::Printf(TEXT("[%6.1fs] %s\n"), FPlatformTime::Seconds() - StartedAt, *Message);
        Text->SetText(FText::FromString(Transcript));
        Scroll->ScrollToEnd();
        // Export accesses editor UObjects on the game thread. Keep the modal
        // output painted between operations without moving those objects to a worker.
        FSlateApplication::Get().PumpMessages();
        FSlateApplication::Get().Tick();
    }

    void RequestClose(const TSharedRef<SWindow>& Parent)
    {
        if (bFinished) FSlateApplication::Get().RequestDestroyWindow(Parent);
    }
};

void ShowDiscExportOutput(UKyaDisc* Disc)
{
    TSharedRef<SWindow> Window = SNew(SWindow)
        .Title(FText::FromString(TEXT("Kya ISO Export Output")))
        .ClientSize(FVector2D(900, 600))
        .SupportsMinimize(false).SupportsMaximize(true).HasCloseButton(false);
    TSharedRef<SKyaDiscExportOutput> Output = SNew(SKyaDiscExportOutput, Disc, Window);
    Window->SetContent(Output);
    // Slow-task modal windows return immediately, letting the exporter update
    // the transcript while the editor remains blocked from changing its assets.
    FSlateApplication::Get().AddModalWindow(Window, FSlateApplication::Get().FindBestParentWindowForDialogs(nullptr), true);
    Window->ShowWindow();
    Output->RunExport();
}

#if WITH_DEV_AUTOMATION_TESTS
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKyaDiscExportOutputWindowTest, "Kya.Disc.OutputWindow",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FKyaDiscExportOutputWindowTest::RunTest(const FString& Parameters)
{
    TStrongObjectPtr<UKyaDisc> Disc(NewObject<UKyaDisc>());
    TSharedRef<SWindow> Window = SNew(SWindow).ClientSize(FVector2D(900, 600)).HasCloseButton(false);
    TSharedRef<SKyaDiscExportOutput> Output = SNew(SKyaDiscExportOutput, Disc.Get(), Window);
    Window->SetContent(Output);
    FSlateApplication::Get().AddModalWindow(Window, nullptr, true);
    Window->ShowWindow();
    Output->RunExport();
    TestTrue(TEXT("Output remains open after export"), FSlateApplication::Get().GetActiveModalWindow() == Window);
    TestTrue(TEXT("Close is enabled after completion"), Output->bFinished);
    TestTrue(TEXT("Transcript retains the failure explanation"), Output->Transcript.Contains(TEXT("No generated banks were found")));
    TestTrue(TEXT("Transcript includes elapsed time"), Output->Transcript.Contains(TEXT("s]")));
    TestTrue(TEXT("Result distinguishes failure"), Output->Status->GetText().ToString().StartsWith(TEXT("Export failed")));
    Window->RequestDestroyWindow();
    FSlateApplication::Get().Tick();
    TestTrue(TEXT("Completed output window can close"), FSlateApplication::Get().GetActiveModalWindow() != Window);
    return true;
}
#endif
}

void FKyaDiscDetails::CustomizeDetails(IDetailLayoutBuilder& Builder)
{
    TArray<TWeakObjectPtr<UObject>> Objects;
    Builder.GetObjectsBeingCustomized(Objects);
    if (Objects.Num() != 1) return;
    TWeakObjectPtr<UKyaDisc> Disc = Cast<UKyaDisc>(Objects[0].Get());
    IDetailCategoryBuilder& DiscCategory = Builder.EditCategory(TEXT("Disc"));
    DiscCategory.AddProperty(GET_MEMBER_NAME_CHECKED(UKyaDisc, SourceFilename));
    DiscCategory.AddProperty(GET_MEMBER_NAME_CHECKED(UKyaDisc, Files));
    DiscCategory.AddProperty(GET_MEMBER_NAME_CHECKED(UKyaDisc, Levels));
    FDetailWidgetRow& ActionRow = DiscCategory.AddCustomRow(FText::FromString(TEXT("Generate Levels Export Changes to ISO")));
    DiscCategory.SetSortOrder(0);
    ActionRow
        .WholeRowContent()
        .HAlign(HAlign_Center)
        [
            SNew(SBox)
            .Padding(8.0f)
            [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot().AutoWidth().Padding(0.0f, 0.0f, 8.0f, 0.0f)
            [SNew(SButton)
            .Text(FText::FromString(TEXT("Generate Levels")))
            .ToolTipText(FText::FromString(TEXT("Create bank assets, level sets and preview maps for the disc's levels. Existing completed levels are preserved.")))
            .OnClicked_Lambda([Disc]()
            {
                if (Disc.IsValid())
                {
                    FString Error;
                    const bool bSuccess = KyaGenerateDiscLevels(*Disc, Error);
                    FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(bSuccess
                        ? TEXT("Levels generated. Use Save All to save the new assets and maps.") : Error));
                }
                return FReply::Handled();
            })]
            + SHorizontalBox::Slot().AutoWidth()
        [SNew(SButton)
            .Text(FText::FromString(TEXT("Export Changes to ISO")))
            .ToolTipText(FText::FromString(TEXT("Validate and export edited banks to the source ISO. Keeps the original image as a backup.")))
            .OnClicked_Lambda([Disc]()
            {
                if (Disc.IsValid())
                {
                    ShowDiscExportOutput(Disc.Get());
                }
                return FReply::Handled();
            })]
            ]
        ];
}
