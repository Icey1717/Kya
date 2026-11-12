#pragma once

#include "CoreMinimal.h"
#include "IDetailCustomization.h"
#include "UObject/StructOnScope.h"

class AKyaActorHandle;
class UKyaLevelBank;
class IPropertyUtilities;
struct FKyaActorPlacement;

/**
 * Exposes the complete typed FKyaActorPlacement for the selected actor handle.
 *
 * The bank record stays authoritative: the customization edits a struct-on-scope
 * copy and writes only the property that changed through the shared edit service,
 * so nested fields, fixed-size arrays and class-specific records are all reachable
 * with one undo step and without a hand-written mirror of the schema.
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

    void OnPlacementPropertyChanged(FName PropertyName);
    void OnBankEdited(UKyaLevelBank* Bank);
    void OnObjectTransacted(UObject* Object, const FTransactionObjectEvent& Event);
    void RefreshScopeFromBank();
    /** Reload the scope on the next details tick; optionally rebuild the panel. */
    void RequestScopeRefresh(bool bRebuildPanel);
    /** Copy one top-level property from the scope into the bank record. */
    bool BuildPatchForProperty(FName PropertyName, struct FKyaActorEditPatch& OutPatch);
    FKyaActorPlacement* ScopeRecord() const;
};
