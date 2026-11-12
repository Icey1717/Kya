#pragma once

#include "CoreMinimal.h"
#include "Templates/Function.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"

struct FKyaPickerOption
{
    int32 Value = 0;
    FString Label;
    FString ToolTip;
};

DECLARE_DELEGATE_OneParam(FOnKyaPickerValuePicked, int32);
DECLARE_DELEGATE_OneParam(FOnKyaFlagValueChanged, int64);

/**
 * Combo box of labeled record references with a search field. The current raw value
 * is always visible, even when it does not resolve to an option, so unresolved ids
 * stay discoverable and editable.
 */
class SKyaReferencePicker : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SKyaReferencePicker) {}
        SLATE_ARGUMENT(TArray<TSharedPtr<FKyaPickerOption>>, Options)
        SLATE_ARGUMENT(int32, CurrentValue)
        /** Returns the authoritative current value (bank/scope) for the label and raw id. */
        SLATE_ARGUMENT(TFunction<int32()>, GetCurrentValue)
        SLATE_EVENT(FOnKyaPickerValuePicked, OnValuePicked)
        SLATE_ATTRIBUTE(FText, ResolvedLabel)
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);
    void SetOptions(TArray<TSharedPtr<FKyaPickerOption>> InOptions);
    void SetCurrentValue(int32 InValue);

private:
    TArray<TSharedPtr<FKyaPickerOption>> AllOptions;
    TArray<TSharedPtr<FKyaPickerOption>> FilteredOptions;
    int32 CurrentValue = INDEX_NONE;
    FString SearchText;
    TSharedPtr<class SComboBox<TSharedPtr<FKyaPickerOption>>> Combo;
    TSharedPtr<class SSearchBox> SearchBox;
    TSharedPtr<class STextBlock> RawIdText;
    TFunction<int32()> GetCurrentValue;
    FOnKyaPickerValuePicked OnValuePicked;
    TAttribute<FText> ResolvedLabel;

    int32 CurrentValueValue() const;
    void RebuildFilteredOptions();
    void OnSearchChanged(const FText& Text);
    void OnSelectionChanged(TSharedPtr<FKyaPickerOption> Option, ESelectInfo::Type SelectInfo);
    TSharedRef<class SWidget> MakeOptionWidget(TSharedPtr<FKyaPickerOption> Option);
    FText GetComboLabel() const;
    FText GetRawIdText() const;
};

struct FKyaFlagBit
{
    int64 Mask = 0;
    FString Label;
    FString ToolTip;
};

/** Checkboxes for understood bits plus the raw value, preserving unknown bits. */
class SKyaFlagBits : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SKyaFlagBits) {}
        SLATE_ARGUMENT(int64, Value)
        SLATE_ARGUMENT(TArray<TSharedPtr<FKyaFlagBit>>, Bits)
        /** Returns the authoritative current value (bank/scope), so stale widget state
            can never overwrite bits changed elsewhere. */
        SLATE_ARGUMENT(TFunction<int64()>, GetCurrentValue)
        SLATE_EVENT(FOnKyaFlagValueChanged, OnValueChanged)
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);
    void SetValue(int64 InValue);

private:
    int64 Value = 0;
    TArray<TSharedPtr<FKyaFlagBit>> Bits;
    TFunction<int64()> GetCurrentValue;
    FOnKyaFlagValueChanged OnValueChanged;
    TSharedPtr<class STextBlock> RawValueText;

    int64 CurrentValue() const;
    void OnBitChanged(bool bChecked, int64 Mask);
    FText GetRawValueText() const;
};
