#include "KyaLevelSettingsDetailsCustomization.h"

#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "KyaLevelSettings.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"

TSharedRef<IDetailCustomization> FKyaLevelSettingsDetailsCustomization::MakeInstance()
{
    return MakeShared<FKyaLevelSettingsDetailsCustomization>();
}

void FKyaLevelSettingsDetailsCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
    TArray<TWeakObjectPtr<UObject>> Selected;
    DetailBuilder.GetObjectsBeingCustomized(Selected);
    if (Selected.Num() != 1) return;
    UKyaLevelSettings* Settings = Cast<UKyaLevelSettings>(Selected[0].Get());
    if (!Settings) return;

    IDetailCategoryBuilder& Category = DetailBuilder.EditCategory("Kya|Level",
        FText::FromString(TEXT("Kya Level")), ECategoryPriority::Important);
    Category.AddCustomRow(FText::FromString(TEXT("Apply settings")))
        .NameContent()
        [
            SNew(STextBlock).Text(FText::FromString(TEXT("Apply settings")))
        ]
        .ValueContent().MinDesiredWidth(380.0f)
        [
            SNew(SVerticalBox)
            + SVerticalBox::Slot().AutoHeight()
            [
                SNew(SButton)
                .Text(FText::FromString(TEXT("Apply to Bank")))
                .ToolTipText(FText::FromString(TEXT("Validate and write these settings to the bank as one undo step. "
                    "Actor, waypoint and path records are preserved.")))
                .OnClicked_Lambda([Settings]()
                {
                    FString Error;
                    if (!Settings->ApplyToBank(Error))
                        UE_LOG(LogTemp, Warning, TEXT("KYA_LEVEL_SETTINGS_APPLY_FAILED reason=%s"), *Error);
                    return FReply::Handled();
                })
            ]
            + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 2.0f, 0.0f, 0.0f)
            [
                SNew(STextBlock)
                .Text_Lambda([Settings]() { return FText::FromString(Settings->ApplyStatus); })
                .AutoWrapText(true)
            ]
        ];
}
