#pragma once

#include "CoreMinimal.h"
#include "IDetailCustomization.h"
#include "UObject/StructOnScope.h"

class AKyaActorHandle;
class UKyaLevelBank;
struct FKyaActorPlacement;

/**
 * Exposes the complete typed FKyaActorPlacement for the selected actor handle.
 *
 * The bank record stays authoritative: the customization edits a struct-on-scope
 * copy and writes only the property that changed through the shared edit service,
 * so nested fields, fixed-size arrays and class-specific records are all reachable
 * with one undo step and without a hand-written mirror of the schema.
 *
 * It also presents recovered class/behaviour labels, bank/sector/mesh/texture/
 * behaviour reference pickers with raw ids and unresolved-reference warnings, flag
 * widgets for understood bits, and unit tooltips.
 */
class FKyaActorDetailsCustomization : public IDetailCustomization
{
public:
    static TSharedRef<IDetailCustomization> MakeInstance();

    virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

private:
    TWeakObjectPtr<AKyaActorHandle> Handle;
    TSharedPtr<FStructOnScope> PlacementScope;
    TWeakPtr<IPropertyUtilities> PropertyUtilities;
    FDelegateHandle BankEditedHandle;
    bool bApplying = false;
    /** Last rejected edit, shown inline beside the References section. */
    FString InlineError;
    TSharedPtr<class STextBlock> InlineErrorText;

    FText GetInlineErrorText() const;

    void OnPlacementPropertyChanged(FName PropertyName);
    void OnBankEdited(UKyaLevelBank* Bank);
    void OnObjectTransacted(UObject* Object, const FTransactionObjectEvent& Event);
    void RefreshScopeFromBank();
    /** Reload the scope on the next details tick; optionally rebuild the panel. */
    void RequestScopeRefresh(bool bRebuildPanel);
    /** Copy one top-level property from the scope into the bank record. */
    bool BuildPatchForProperty(FName PropertyName, struct FKyaActorEditPatch& OutPatch);
    /** Apply a modified copy of the placement through the edit service. */
    void ApplyPlacementCopy(const struct FKyaActorPlacement& Modified, FName PropertyName);
    /** Apply an already prepared patch (reset, copy, bulk) through the service. */
    bool ApplyPreparedPatch(const struct FKyaActorEditPatch& Patch, const FString& Context);
    /** Multi-selection panel: shared-field bulk editing and compatibility handling. */
    void AddMultiEditSection(IDetailLayoutBuilder& DetailBuilder, const TArray<AKyaActorHandle*>& Handles);
    struct FKyaActorPlacement* ScopeRecord() const;
};
