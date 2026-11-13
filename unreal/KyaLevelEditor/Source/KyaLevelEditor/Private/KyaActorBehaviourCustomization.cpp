#include "KyaActorBehaviourCustomization.h"

#include "DetailWidgetRow.h"
#include "IDetailChildrenBuilder.h"
#include "IDetailGroup.h"
#include "IDetailPropertyRow.h"
#include "PropertyHandle.h"
#include "UObject/UnrealType.h"
#include "Widgets/Text/STextBlock.h"

TSharedRef<IPropertyTypeCustomization> FKyaActorBehaviourCustomization::MakeInstance()
{
    return MakeShared<FKyaActorBehaviourCustomization>();
}

void FKyaActorBehaviourCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> Handle,
    FDetailWidgetRow& Row, IPropertyTypeCustomizationUtils& Utils)
{
    Row.NameContent()[Handle->CreatePropertyNameWidget()];
}

void FKyaActorBehaviourCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> Handle,
    IDetailChildrenBuilder& Children, IPropertyTypeCustomizationUtils& Utils)
{
    TMap<FName, IDetailGroup*> Groups;
    TMap<FName, TArray<TSharedRef<IPropertyHandle>>> GroupFlags;
    TArray<TSharedRef<IPropertyHandle>> TypedFlags;
    bool bEmptyPayload = false;
    const TSharedPtr<IPropertyHandle> Empty = Handle->GetChildHandle(TEXT("bHasTypedEmptyBehaviour"));
    if (Empty.IsValid())
        Empty->GetValue(bEmptyPayload);
    bool bHelperSign = false;
    int32 BehaviourId = -1;
    const TSharedPtr<IPropertyHandle> Sign = Handle->GetChildHandle(TEXT("bHasTypedHelperSignBehaviour"));
    if (Sign.IsValid())
        Sign->GetValue(bHelperSign);
    const TSharedPtr<IPropertyHandle> Id = Handle->GetChildHandle(TEXT("Id"));
    if (Id.IsValid())
        Id->GetValue(BehaviourId);
    if (bEmptyPayload || (bHelperSign && BehaviourId == 2))
        Children.AddCustomRow(FText::FromString(TEXT("Empty payload")))
            .WholeRowContent()[SNew(STextBlock).Text(FText::FromString(TEXT("This behaviour has no serialized payload.")))];
    uint32 ChildCount = 0;
    Handle->GetNumChildren(ChildCount);
    for (uint32 Index = 0; Index < ChildCount; ++Index)
    {
        const TSharedPtr<IPropertyHandle> Child = Handle->GetChildHandle(Index);
        if (Child.IsValid() && Child->GetProperty() &&
            Child->GetProperty()->GetName().StartsWith(TEXT("bHasTyped")))
            TypedFlags.Add(Child.ToSharedRef());
    }
    for (uint32 Index = 0; Index < ChildCount; ++Index)
    {
        const TSharedPtr<IPropertyHandle> Child = Handle->GetChildHandle(Index);
        FProperty* Property = Child.IsValid() ? Child->GetProperty() : nullptr;
        if (!Property) continue;

        // Decoder flags still participate in edit conditions without cluttering
        // the inspector. Only the behaviour identity and payload are displayed.
        if (Property->GetName().StartsWith(TEXT("bHasTyped"))) continue;
        if (Property->GetFName() == TEXT("Id"))
        {
            Children.AddProperty(Child.ToSharedRef());
            continue;
        }
        if (bEmptyPayload || (bHelperSign && BehaviourId == 2)) continue;

        const FString Category = Property->GetMetaData(TEXT("Category"));
        const FName GroupName(*Category);
        IDetailGroup*& Group = Groups.FindOrAdd(GroupName);
        if (!Group)
        {
            Group = &Children.AddGroup(GroupName, FText::FromString(Category), true);
            Group->SetDisplayMode(EDetailGroupDisplayMode::Category);
        }
        const FString Condition = Property->GetMetaData(TEXT("EditCondition"));
        for (const TSharedRef<IPropertyHandle>& Flag : TypedFlags)
        {
            if (Condition.Contains(Flag->GetProperty()->GetName()))
                GroupFlags.FindOrAdd(GroupName).AddUnique(Flag);
        }
        // Keep Unreal's expression evaluation, including behaviour-id checks and
        // the raw-data fallback when no typed representation has been decoded.
        Group->AddPropertyRow(Child.ToSharedRef()).EditConditionHides(true);
    }

    for (const TPair<FName, IDetailGroup*>& Entry : Groups)
    {
        const TArray<TSharedRef<IPropertyHandle>> Flags = GroupFlags.FindRef(Entry.Key);
        // The generic Behaviour group contains the raw-data fallback. Every
        // decoded family group is applicable when one of its decoder flags is set.
        const bool bRawDataGroup = Entry.Key == TEXT("Behaviour");
        Entry.Value->HeaderRow()
            .Visibility(TAttribute<EVisibility>::CreateLambda([Flags, bRawDataGroup]()
            {
                bool bAnyTyped = false;
                for (const TSharedRef<IPropertyHandle>& Flag : Flags)
                {
                    bool bValue = false;
                    if (Flag->GetValue(bValue) != FPropertyAccess::Success)
                        return EVisibility::Visible;
                    bAnyTyped |= bValue;
                }
                return (bRawDataGroup ? !bAnyTyped : Flags.IsEmpty() || bAnyTyped)
                    ? EVisibility::Visible : EVisibility::Collapsed;
            }))
            .NameContent()[SNew(STextBlock).Text(FText::FromName(Entry.Key))];
    }
}
