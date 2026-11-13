#include "KyaScenarioConditionCustomization.h"

#include "DetailWidgetRow.h"
#include "IDetailChildrenBuilder.h"
#include "PropertyHandle.h"
#include "KyaLevelBank.h"
#include "UObject/UnrealType.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Widgets/Input/SComboButton.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
// Recovered SCN_* names from LevelScheduler.h; later slots have no recovered names.
const TCHAR* const variables[] = { TEXT("Freed Wolfens"), TEXT(""), TEXT(""), TEXT("Companion"),
    TEXT(""), TEXT(""), TEXT("Current episode"), TEXT("Jump bounce ability"), TEXT("Climb ability"),
    TEXT("Boomy type"), TEXT("Fight ability"), TEXT("Exorcism ability"), TEXT("Regeneration ability"),
    TEXT("Jamgut ride ability"), TEXT("Binoculars ability"), TEXT("Receptacle ability"),
    TEXT("Magic board"), TEXT("Magic gauge"), TEXT("Magic update"), TEXT("Magic maximum"),
    TEXT("Life gauge"), TEXT("Life update"), TEXT("Life maximum") };

FString VariableLabel(int32 Index)
{
    if (Index >= 0 && Index < UE_ARRAY_COUNT(variables) && variables[Index][0])
        return FString::Printf(TEXT("%s [%d]"), variables[Index], Index);
    return FString::Printf(TEXT("Scenario[%d] (unrecovered name)"), Index);
}

bool IsFirstActorCondition(const TSharedRef<IPropertyHandle>& Handle)
{
    if (Handle->GetIndexInArray() != 0) return false;
    const TSharedPtr<IPropertyHandle> Array = Handle->GetParentHandle();
    const TSharedPtr<IPropertyHandle> Owner = Array ? Array->GetParentHandle() : nullptr;
    const FStructProperty* Struct = Owner ? CastField<FStructProperty>(Owner->GetProperty()) : nullptr;
    return Struct && Struct->Struct == FKyaActorPlacement::StaticStruct();
}

// CScenaricCondition::IsVerified, PS2 0x001175e0 (ScenaricCondition.h/.cpp).
const TCHAR* const joins[] = { TEXT("AND"), TEXT("OR"), TEXT("XOR") };
const TCHAR* const comparisons[] = { TEXT("Always true"), TEXT("< (less than)"),
    TEXT("<= (less than or equal)"), TEXT("== (equal)"), TEXT(">= (greater than or equal)"),
    TEXT("> (greater than)"), TEXT("!= (not equal)") };
const TCHAR* const symbols[] = { TEXT("Always true"), TEXT("<"), TEXT("<="), TEXT("=="),
    TEXT(">="), TEXT(">"), TEXT("!=") };

FText CodeLabel(const TSharedRef<IPropertyHandle>& Handle, bool bJoin)
{
    int32 Code;
    if (Handle->GetValue(Code) != FPropertyAccess::Success) return FText::FromString(TEXT("Multiple values"));
    const int32 Count = bJoin ? UE_ARRAY_COUNT(joins) : UE_ARRAY_COUNT(comparisons);
    if (Code >= 0 && Code < Count)
        return FText::FromString(FString::Printf(TEXT("%s (%d)"), bJoin ? joins[Code] : comparisons[Code], Code));
    return FText::FromString(FString::Printf(TEXT("Unknown (%d): %s"), Code,
        bJoin ? TEXT("replace result") : TEXT("always true")));
}

void AddCodeRow(IDetailChildrenBuilder& Children, TSharedRef<IPropertyHandle> Handle,
    const TCHAR* Label, const TCHAR* Tooltip, bool bJoin)
{
    Children.AddCustomRow(FText::FromString(Label))
    .NameContent()[SNew(STextBlock).Text(FText::FromString(Label)).ToolTipText(FText::FromString(Tooltip))]
    .ValueContent().MinDesiredWidth(300)
    [
        SNew(SHorizontalBox)
        + SHorizontalBox::Slot().FillWidth(1)
        [
            SNew(SComboButton)
            .OnGetMenuContent_Lambda([Handle, bJoin]()
            {
                FMenuBuilder Menu(true, nullptr);
                const int32 Count = bJoin ? UE_ARRAY_COUNT(joins) : UE_ARRAY_COUNT(comparisons);
                for (int32 Code = 0; Code < Count; ++Code)
                    Menu.AddMenuEntry(FText::FromString(bJoin ? joins[Code] : comparisons[Code]), FText(),
                        FSlateIcon(), FUIAction(FExecuteAction::CreateLambda([Handle, Code]() { Handle->SetValue(Code); })));
                return Menu.MakeWidget();
            })
            .ButtonContent()[SNew(STextBlock).Text_Lambda([Handle, bJoin]() { return CodeLabel(Handle, bJoin); })]
        ]
        + SHorizontalBox::Slot().AutoWidth().Padding(8, 0)
        [Handle->CreatePropertyValueWidget()]
    ];
}
}

TSharedRef<IPropertyTypeCustomization> FKyaScenarioConditionCustomization::MakeInstance()
{
    return MakeShared<FKyaScenarioConditionCustomization>();
}

void FKyaScenarioConditionCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> Handle,
    FDetailWidgetRow& Row, IPropertyTypeCustomizationUtils& Utils)
{
    Row.NameContent()[Handle->CreatePropertyNameWidget()]
    .ValueContent().MinDesiredWidth(300)
    [
        SNew(STextBlock).Text_Lambda([Handle]()
        {
            int32 Join = 0, Variable = 0, Comparison = 0, Value = 0;
            if (Handle->GetChildHandle(TEXT("Combiner"))->GetValue(Join) != FPropertyAccess::Success ||
                Handle->GetChildHandle(TEXT("Variable"))->GetValue(Variable) != FPropertyAccess::Success ||
                Handle->GetChildHandle(TEXT("Comparison"))->GetValue(Comparison) != FPropertyAccess::Success ||
                Handle->GetChildHandle(TEXT("Value"))->GetValue(Value) != FPropertyAccess::Success)
                return FText::FromString(TEXT("Multiple values"));
            const FString Test = Comparison > 0 && Comparison < UE_ARRAY_COUNT(symbols)
                ? FString::Printf(TEXT("%s %s %d"), *VariableLabel(Variable), symbols[Comparison], Value)
                : TEXT("Always true");
            if (IsFirstActorCondition(Handle))
                return FText::FromString(FString::Printf(TEXT("%s (first actor test; AND)"), *Test));
            return FText::FromString(FString::Printf(TEXT("%s  %s"),
                Join >= 0 && Join < UE_ARRAY_COUNT(joins) ? joins[Join] : TEXT("REPLACE"), *Test));
        })
        .ToolTipText(FText::FromString(TEXT("Evaluated in list order, starting from true; an empty list passes. "
            "AND requires both results, OR either result, XOR exactly one. Actor loading forces the first join to AND, "
            "regardless of its stored code. Conditioned operations preserve the first join. "
            "The first actor test is labelled with its effective AND join; other summaries show stored codes. "
            "Numeric fields remain available for unknown codes.")))
    ];
}

void FKyaScenarioConditionCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> Handle,
    IDetailChildrenBuilder& Children, IPropertyTypeCustomizationUtils& Utils)
{
    AddCodeRow(Children, Handle->GetChildHandle(TEXT("Combiner")).ToSharedRef(), TEXT("Join with previous result"),
        TEXT("0 AND, 1 OR, 2 XOR. Evaluation starts from true and follows list order. "
            "For actor conditions the first join is always treated as AND by the game loader."), true);
    const TSharedRef<IPropertyHandle> Variable = Handle->GetChildHandle(TEXT("Variable")).ToSharedRef();
    Variable->SetPropertyDisplayName(FText::FromString(TEXT("Scenario variable index")));
    Variable->SetToolTipText(FText::FromString(TEXT("Index into the global scenario variable table, shared across levels. "
        "This is not an actor reference. Story meanings depend on the game's scripts.")));
    Children.AddCustomRow(FText::FromString(TEXT("Scenario variable")))
    .NameContent()[Variable->CreatePropertyNameWidget()]
    .ValueContent().MinDesiredWidth(300)
    [
        SNew(SHorizontalBox)
        + SHorizontalBox::Slot().FillWidth(1)
        [
            SNew(SComboButton).OnGetMenuContent_Lambda([Variable]()
            {
                FMenuBuilder Menu(true, nullptr);
                for (int32 Index = 0; Index < UE_ARRAY_COUNT(variables); ++Index)
                {
                    if (!variables[Index][0]) continue;
                    Menu.AddMenuEntry(FText::FromString(VariableLabel(Index)), FText(), FSlateIcon(),
                        FUIAction(FExecuteAction::CreateLambda([Variable, Index]() { Variable->SetValue(Index); })));
                }
                return Menu.MakeWidget();
            })
            .ButtonContent()[SNew(STextBlock).Text_Lambda([Variable]()
            {
                int32 Index;
                return FText::FromString(Variable->GetValue(Index) == FPropertyAccess::Success
                    ? VariableLabel(Index) : TEXT("Multiple values"));
            })]
        ]
        + SHorizontalBox::Slot().AutoWidth().Padding(8, 0)[Variable->CreatePropertyValueWidget()]
    ];
    AddCodeRow(Children, Handle->GetChildHandle(TEXT("Comparison")).ToSharedRef(), TEXT("Test"),
        TEXT("Compare the scenario variable to the signed integer value below. Always true ignores the value; "
            "unknown comparison codes also return true."), false);
    const TSharedRef<IPropertyHandle> Value = Handle->GetChildHandle(TEXT("Value")).ToSharedRef();
    Value->SetPropertyDisplayName(FText::FromString(TEXT("Compare against")));
    Value->SetToolTipText(FText::FromString(TEXT("Signed integer threshold or state. Values such as 0, 1 and 2 "
        "have no universal story meaning. Ignored by the Always true test.")));
    Children.AddProperty(Value);
}
