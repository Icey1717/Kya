#include "KyaActorDetailsCustomization.h"

#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "IDetailPropertyRow.h"
#include "IPropertyUtilities.h"
#include "PropertyHandle.h"
#include "Widgets/Text/STextBlock.h"
#include "Editor.h"
#include "ScopedTransaction.h"
#include "KyaActorEditService.h"
#include "KyaActorHandle.h"
#include "KyaLevelBank.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Misc/TransactionObjectEvent.h"

TSharedRef<IDetailCustomization> FKyaActorDetailsCustomization::MakeInstance()
{
    return MakeShared<FKyaActorDetailsCustomization>();
}

FKyaActorPlacement* FKyaActorDetailsCustomization::ScopeRecord() const
{
    return PlacementScope.IsValid()
        ? reinterpret_cast<FKyaActorPlacement*>(PlacementScope->GetStructMemory()) : nullptr;
}

void FKyaActorDetailsCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
    TArray<TWeakObjectPtr<UObject>> Selected;
    DetailBuilder.GetObjectsBeingCustomized(Selected);

    FKyaLevelEditService& Service = FKyaLevelEditService::Get();
    if (BankEditedHandle.IsValid())
    {
        Service.OnBankEdited.Remove(BankEditedHandle);
        BankEditedHandle.Reset();
    }

    // The full placement surface is only shown for a single actor; multi-selection
    // keeps the summary fields and avoids ambiguous mixed-value edits.
    TArray<AKyaActorHandle*> Handles;
    for (const TWeakObjectPtr<UObject>& Object : Selected)
        if (AKyaActorHandle* Candidate = Cast<AKyaActorHandle>(Object.Get())) Handles.Add(Candidate);
    if (Handles.Num() != 1 || !Handles[0] || !Handles[0]->Bank ||
        !Handles[0]->Bank->SceneEnvironment.Actors.IsValidIndex(Handles[0]->ActorIndex))
    {
        if (Handles.Num() > 1)
        {
            DetailBuilder.EditCategory("Kya|Bank record", FText::FromString(TEXT("Bank record")))
                .AddCustomRow(FText::FromString(TEXT("Selection")))
                .ValueContent()
                [SNew(STextBlock)
                    .Text(FText::FromString(TEXT("Select one actor to edit its full bank record.")))
                    .ToolTipText(FText::FromString(TEXT("Multi-edit of class-specific fields is not implemented yet.")))];
        }
        return;
    }

    Handle = Handles[0];
    PlacementScope = MakeShared<FStructOnScope>(FKyaActorPlacement::StaticStruct());
    PropertyUtilities = DetailBuilder.GetPropertyUtilities();
    RefreshScopeFromBank();
    BankEditedHandle = Service.OnBankEdited.AddSP(this, &FKyaActorDetailsCustomization::OnBankEdited);
    // Undo/redo does not emit OnBankEdited; refresh the presentation copy from the
    // authoritative record when the bank is transacted.
    FCoreUObjectDelegates::OnObjectTransacted.AddSP(this, &FKyaActorDetailsCustomization::OnObjectTransacted);

    // The bank record is the single Details surface for placement fields; the old
    // hand-written mirror stays on the actor for script compatibility but is hidden.
    const FName MirroredFields[] = {
        GET_MEMBER_NAME_CHECKED(AKyaActorHandle, ActorName),
        GET_MEMBER_NAME_CHECKED(AKyaActorHandle, ActorMeshIndex),
        GET_MEMBER_NAME_CHECKED(AKyaActorHandle, ActorTextureIndex),
        GET_MEMBER_NAME_CHECKED(AKyaActorHandle, ActorPosition),
        GET_MEMBER_NAME_CHECKED(AKyaActorHandle, ActorRotation),
        GET_MEMBER_NAME_CHECKED(AKyaActorHandle, ActorScale),
        GET_MEMBER_NAME_CHECKED(AKyaActorHandle, ActorSectorId),
        GET_MEMBER_NAME_CHECKED(AKyaActorHandle, ActorDefaultBehaviourId),
        GET_MEMBER_NAME_CHECKED(AKyaActorHandle, ActorVisibilityDistance),
        GET_MEMBER_NAME_CHECKED(AKyaActorHandle, ActorLightingValue),
        GET_MEMBER_NAME_CHECKED(AKyaActorHandle, ActorLightingFlags),
    };
    for (const FName Field : MirroredFields) DetailBuilder.HideProperty(Field);

    UScriptStruct* PlacementStruct = FKyaActorPlacement::StaticStruct();
    TSet<FName> AddedCategories;
    for (TFieldIterator<FProperty> It(PlacementStruct); It; ++It)
    {
        FProperty* Property = *It;
        if (!Property->HasAnyPropertyFlags(CPF_Edit)) continue;
        // Export capability metadata drives the UI: only editable fields with a writer
        // path are offered, and the capability reason becomes the row tooltip.
        const FKyaFieldCapability Capability = GetKyaActorFieldCapability(Property->GetFName());
        if (Capability.Kind != EKyaFieldCapability::EditableExportable &&
            Capability.Kind != EKyaFieldCapability::EditablePresentationOnly)
            continue;

        FString CategoryName = TEXT("Bank record");
        if (const FString* Metadata = Property->FindMetaData(TEXT("Category")))
        {
            CategoryName = *Metadata;
            // Hide the handle's own prefix if a struct category ever carries one.
            CategoryName.RemoveFromStart(TEXT("Kya|"));
        }
        const FName CategoryFName(*CategoryName);
        IDetailCategoryBuilder& Category = DetailBuilder.EditCategory(CategoryFName,
            FText::FromString(CategoryName), ECategoryPriority::Important);
        AddedCategories.Add(CategoryFName);

        IDetailPropertyRow* Row = Category.AddExternalStructureProperty(PlacementScope, Property->GetFName());
        if (!Row) continue;
        Row->DisplayName(FText::FromString(Property->GetDisplayNameText().ToString()));
        if (!Capability.Reason.IsEmpty())
            Row->ToolTip(FText::FromString(FString::Printf(TEXT("%s\n%s"),
                *DescribeKyaFieldCapability(Capability.Kind).ToString(), *Capability.Reason)));
        // Nested edits (array elements, sub-struct fields) notify the child handle.
        if (TSharedPtr<IPropertyHandle> PropertyHandle = Row->GetPropertyHandle())
        {
            const FName PropertyName = Property->GetFName();
            PropertyHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(
                this, &FKyaActorDetailsCustomization::OnPlacementPropertyChanged, PropertyName));
            PropertyHandle->SetOnChildPropertyValueChanged(FSimpleDelegate::CreateSP(
                this, &FKyaActorDetailsCustomization::OnPlacementPropertyChanged, PropertyName));
        }
    }
}

void FKyaActorDetailsCustomization::RefreshScopeFromBank()
{
    FKyaActorPlacement* Scope = ScopeRecord();
    AKyaActorHandle* Actor = Handle.Get();
    if (!Scope || !Actor || !Actor->Bank ||
        !Actor->Bank->SceneEnvironment.Actors.IsValidIndex(Actor->ActorIndex)) return;
    *Scope = Actor->Bank->SceneEnvironment.Actors[Actor->ActorIndex];
}

bool FKyaActorDetailsCustomization::BuildPatchForProperty(FName PropertyName, FKyaActorEditPatch& OutPatch)
{
    FKyaActorPlacement* Scope = ScopeRecord();
    if (!Scope) return false;
    FString Error;
    if (!KyaMakePlacementPropertyPatch(*Scope, PropertyName, OutPatch, &Error))
    {
        UE_LOG(LogTemp, Warning, TEXT("KYA_DETAILS_FIELD_UNSUPPORTED field=%s reason=%s"),
            *PropertyName.ToString(), *Error);
        return false;
    }
    return true;
}

void FKyaActorDetailsCustomization::RequestScopeRefresh(bool bRebuildPanel)
{
    const TWeakObjectPtr<AKyaActorHandle> WeakHandle = Handle;
    const TSharedPtr<FStructOnScope> Scope = PlacementScope;
    const TWeakPtr<IPropertyUtilities> Utilities = PropertyUtilities;
    if (!Scope.IsValid()) return;
    const auto ReloadScope = [WeakHandle, Scope]()
    {
        AKyaActorHandle* Actor = WeakHandle.Get();
        if (!Actor || !Actor->Bank || !Actor->Bank->SceneEnvironment.Actors.IsValidIndex(Actor->ActorIndex)) return;
        *reinterpret_cast<FKyaActorPlacement*>(Scope->GetStructMemory()) =
            Actor->Bank->SceneEnvironment.Actors[Actor->ActorIndex];
    };
    if (!Utilities.IsValid())
    {
        ReloadScope();
        return;
    }
    // Deferred: a synchronous refresh can rebuild this customization while a
    // property notification is still executing.
    Utilities.Pin()->EnqueueDeferredAction(FSimpleDelegate::CreateLambda([ReloadScope, Utilities, bRebuildPanel]()
    {
        ReloadScope();
        if (bRebuildPanel && Utilities.IsValid()) Utilities.Pin()->RequestForceRefresh();
    }));
}

void FKyaActorDetailsCustomization::OnPlacementPropertyChanged(FName PropertyName)
{
    AKyaActorHandle* Actor = Handle.Get();
    if (bApplying || !Actor || !Actor->Bank) return;
    FKyaActorEditPatch Patch;
    if (!BuildPatchForProperty(PropertyName, Patch))
    {
        RefreshScopeFromBank();
        return;
    }

    FKyaLevelEditService& Service = FKyaLevelEditService::Get();
    FString Error;
    TGuardValue<bool> Guard(bApplying, true);
    bool bApplied = false;
    if (GEditor && !GEditor->IsTransactionActive())
    {
        // External struct edits are not part of a details transaction, so open one to
        // keep the bank write undoable as a single step.
        FScopedTransaction Transaction(NSLOCTEXT("KyaLevelEditor", "KyaActorDetailsEdit", "Edit Kya Actor Property"));
        bApplied = Service.Apply(Service.MakeKey(Actor->Bank, Actor->ActorIndex), Patch, EKyaEditNotify::Notify, &Error);
    }
    else
    {
        bApplied = Service.Apply(Service.MakeKey(Actor->Bank, Actor->ActorIndex), Patch, EKyaEditNotify::Notify, &Error);
    }
    if (!bApplied)
    {
        UE_LOG(LogTemp, Warning, TEXT("KYA_DETAILS_WRITE_FAILED bank=%s index=%d field=%s reason=%s"),
            *GetNameSafe(Actor->Bank), Actor->ActorIndex, *PropertyName.ToString(), *Error);
        RefreshScopeFromBank();
    }
}

void FKyaActorDetailsCustomization::OnBankEdited(UKyaLevelBank* Bank)
{
    AKyaActorHandle* Actor = Handle.Get();
    if (bApplying || !Actor || Actor->Bank != Bank) return;
    // A reload can change array sizes and presence flags, so rebuild the panel.
    RequestScopeRefresh(true);
}

void FKyaActorDetailsCustomization::OnObjectTransacted(UObject* Object, const FTransactionObjectEvent& Event)
{
    AKyaActorHandle* Actor = Handle.Get();
    if (!Actor || !Actor->Bank || Object != Actor->Bank) return;
    RequestScopeRefresh(false);
}
