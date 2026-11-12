#include "KyaDetailsWidgets.h"

#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Input/SSearchBox.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SWrapBox.h"
#include "Widgets/Text/STextBlock.h"

void SKyaReferencePicker::Construct(const FArguments& InArgs)
{
    CurrentValue = InArgs._CurrentValue;
    GetCurrentValue = InArgs._GetCurrentValue;
    OnValuePicked = InArgs._OnValuePicked;
    ResolvedLabel = InArgs._ResolvedLabel;
    AllOptions = InArgs._Options;
    RebuildFilteredOptions();

    ChildSlot
    [
        SNew(SVerticalBox)
        + SVerticalBox::Slot()
        .AutoHeight()
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot()
            .FillWidth(1.0f)
            [
                SAssignNew(SearchBox, SSearchBox)
                .HintText(FText::FromString(TEXT("Search references")))
                .OnTextChanged(this, &SKyaReferencePicker::OnSearchChanged)
            ]
            + SHorizontalBox::Slot()
            .AutoWidth()
            .Padding(4.0f, 0.0f)
            [
                SAssignNew(RawIdText, STextBlock)
                .Text(this, &SKyaReferencePicker::GetRawIdText)
                .ToolTipText(FText::FromString(TEXT("Serialized reference id. It is preserved exactly even when it does not resolve.")))
            ]
        ]
        + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(0.0f, 2.0f, 0.0f, 0.0f)
        [
            SAssignNew(Combo, SComboBox<TSharedPtr<FKyaPickerOption>>)
            .OptionsSource(&FilteredOptions)
            .OnGenerateWidget(this, &SKyaReferencePicker::MakeOptionWidget)
            .OnSelectionChanged(this, &SKyaReferencePicker::OnSelectionChanged)
            .Content()
            [
                SNew(STextBlock)
                .Text(this, &SKyaReferencePicker::GetComboLabel)
            ]
            .ToolTipText(ResolvedLabel)
        ]
    ];
}

void SKyaReferencePicker::SetOptions(TArray<TSharedPtr<FKyaPickerOption>> InOptions)
{
    AllOptions = MoveTemp(InOptions);
    RebuildFilteredOptions();
    if (Combo.IsValid()) Combo->RefreshOptions();
}

void SKyaReferencePicker::SetCurrentValue(int32 InValue)
{
    CurrentValue = InValue;
    if (RawIdText.IsValid()) RawIdText->Invalidate(EInvalidateWidgetReason::LayoutAndVolatility);
}

int32 SKyaReferencePicker::CurrentValueValue() const
{
    return GetCurrentValue ? GetCurrentValue() : CurrentValue;
}

FText SKyaReferencePicker::GetRawIdText() const
{
    return FText::FromString(FString::Printf(TEXT("raw %d"), CurrentValueValue()));
}

void SKyaReferencePicker::RebuildFilteredOptions()
{
    FilteredOptions.Reset();
    for (const TSharedPtr<FKyaPickerOption>& Option : AllOptions)
    {
        if (!Option.IsValid()) continue;
        if (!SearchText.IsEmpty() && !Option->Label.Contains(SearchText, ESearchCase::IgnoreCase)) continue;
        FilteredOptions.Add(Option);
    }
}

void SKyaReferencePicker::OnSearchChanged(const FText& Text)
{
    SearchText = Text.ToString();
    RebuildFilteredOptions();
    if (Combo.IsValid()) Combo->RefreshOptions();
}

TSharedRef<SWidget> SKyaReferencePicker::MakeOptionWidget(TSharedPtr<FKyaPickerOption> Option)
{
    if (!Option.IsValid()) return SNew(STextBlock);
    return SNew(STextBlock)
        .Text(FText::FromString(Option->Label))
        .ToolTipText(FText::FromString(Option->ToolTip));
}

void SKyaReferencePicker::OnSelectionChanged(TSharedPtr<FKyaPickerOption> Option, ESelectInfo::Type SelectInfo)
{
    if (SelectInfo == ESelectInfo::Direct || !Option.IsValid()) return;
    CurrentValue = Option->Value;
    if (RawIdText.IsValid()) RawIdText->Invalidate(EInvalidateWidgetReason::LayoutAndVolatility);
    OnValuePicked.ExecuteIfBound(Option->Value);
}

FText SKyaReferencePicker::GetComboLabel() const
{
    const int32 Value = CurrentValueValue();
    for (const TSharedPtr<FKyaPickerOption>& Option : AllOptions)
        if (Option.IsValid() && Option->Value == Value) return FText::FromString(Option->Label);
    return ResolvedLabel.Get(FText::FromString(FString::Printf(TEXT("Unresolved (%d)"), Value)));
}

void SKyaFlagBits::Construct(const FArguments& InArgs)
{
    Value = InArgs._Value;
    Bits = InArgs._Bits;
    GetCurrentValue = InArgs._GetCurrentValue;
    OnValueChanged = InArgs._OnValueChanged;

    TSharedRef<SWrapBox> Box = SNew(SWrapBox).UseAllottedSize(true);
    for (const TSharedPtr<FKyaFlagBit>& Bit : Bits)
    {
        if (!Bit.IsValid()) continue;
        const int64 Mask = Bit->Mask;
        Box->AddSlot().Padding(0.0f, 0.0f, 8.0f, 0.0f)
        [
            SNew(SCheckBox)
            .IsChecked_Lambda([this, Mask]()
            {
                return (CurrentValue() & Mask) != 0 ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
            })
            .OnCheckStateChanged_Lambda([this, Mask](ECheckBoxState State)
            {
                OnBitChanged(State == ECheckBoxState::Checked, Mask);
            })
            .ToolTipText(FText::FromString(Bit->ToolTip))
            [
                SNew(STextBlock).Text(FText::FromString(Bit->Label))
            ]
        ];
    }
    Box->AddSlot().Padding(0.0f, 0.0f, 8.0f, 0.0f)
    [
        SAssignNew(RawValueText, STextBlock)
        .Text(this, &SKyaFlagBits::GetRawValueText)
        .ToolTipText(FText::FromString(TEXT("Raw serialized value. Unrecovered bits are preserved exactly.")))
    ];

    ChildSlot
    [
        SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight()[Box]
        + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 2.0f, 0.0f, 0.0f)
        [
            SNew(STextBlock)
            .Text(FText::FromString(TEXT("Unknown bits are preserved; unrecovered flags are shown as raw values.")))
            .ColorAndOpacity(FSlateColor(FLinearColor(0.7f, 0.7f, 0.7f, 1.0f)))
        ]
    ];
}

int64 SKyaFlagBits::CurrentValue() const
{
    return GetCurrentValue ? GetCurrentValue() : Value;
}

void SKyaFlagBits::SetValue(int64 InValue)
{
    Value = InValue;
    if (RawValueText.IsValid()) RawValueText->Invalidate(EInvalidateWidgetReason::LayoutAndVolatility);
}

void SKyaFlagBits::OnBitChanged(bool bChecked, int64 Mask)
{
    // Apply the mask to the authoritative value, not to cached widget state, so a
    // concurrent edit or undo cannot be overwritten by a stale checkbox.
    const int64 Current = CurrentValue();
    Value = bChecked ? (Current | Mask) : (Current & ~Mask);
    if (RawValueText.IsValid()) RawValueText->Invalidate(EInvalidateWidgetReason::LayoutAndVolatility);
    OnValueChanged.ExecuteIfBound(Value);
}

FText SKyaFlagBits::GetRawValueText() const
{
    const int64 Current = CurrentValue();
    return FText::FromString(FString::Printf(TEXT("raw 0x%llx (%lld)"), Current, Current));
}
