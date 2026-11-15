#include "KyaLevelSettings.h"

#include "KyaActorEditService.h"
#include "UObject/UnrealType.h"
#if WITH_EDITOR
#include "Editor.h"
#include "ScopedTransaction.h"
#endif

namespace
{
// Copy only settings properties. The proxy has no placement properties, so
// loading never copies or exposes those large property trees.
void CopyEnvironmentSettings(void* Destination, const void* Source, bool bToBank)
{
    for (TFieldIterator<FProperty> It(FKyaLevelSettingsEnvironment::StaticStruct()); It; ++It)
    {
        const FProperty* ProxyProperty = *It;
        const FProperty* BankProperty = FindFProperty<FProperty>(
            FKyaSceneEnvironment::StaticStruct(), ProxyProperty->GetFName());
        check(BankProperty && ProxyProperty->SameType(BankProperty));
        const FProperty* DestinationProperty = bToBank ? BankProperty : ProxyProperty;
        const FProperty* SourceProperty = bToBank ? ProxyProperty : BankProperty;
        DestinationProperty->CopyCompleteValue(
            DestinationProperty->ContainerPtrToValuePtr<void>(Destination),
            SourceProperty->ContainerPtrToValuePtr<void>(Source));
    }
}
}

void UKyaLevelSettings::LoadFromBank()
{
    if (!Bank) return;
    CopyEnvironmentSettings(&Environment, &Bank->SceneEnvironment, false);
    SceneConfiguration = Bank->SceneConfiguration;
    ApplyStatus = TEXT("Edit settings, then Save to apply them to the bank.");
}

bool UKyaLevelSettings::ApplyToBank(FString& OutError)
{
    OutError.Reset();
    if (!Bank)
    {
        OutError = TEXT("Select a bank before saving level settings.");
        ApplyStatus = OutError;
        return false;
    }
#if WITH_EDITOR
    // Preserve asset identity and current placements; the service merges them
    // into this settings snapshot when committing.
    FKyaSceneEnvironment NewEnvironment;
    CopyEnvironmentSettings(&NewEnvironment, &Environment, true);
    FKyaLevelEditService& Service = FKyaLevelEditService::Get();
    TGuardValue<bool> Guard(bApplying, true);
    bool bApplied = false;
    if (GEditor && !GEditor->IsTransactionActive())
    {
        FScopedTransaction Transaction(NSLOCTEXT("KyaLevelEditor", "KyaLevelSettingsEdit", "Edit Kya Level Settings"));
        bApplied = Service.ApplySceneSettings(Bank, NewEnvironment, SceneConfiguration, EKyaEditNotify::Notify, &OutError);
    }
    else
    {
        bApplied = Service.ApplySceneSettings(Bank, NewEnvironment, SceneConfiguration, EKyaEditNotify::Notify, &OutError);
    }
    ApplyStatus = bApplied ? TEXT("Saved to the bank. Save the asset to persist it to disk.") : OutError;
    return bApplied;
#else
    OutError = TEXT("Editing level settings requires the editor.");
    ApplyStatus = OutError;
    return false;
#endif
}

#if WITH_EDITOR
void UKyaLevelSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);
    if (bApplying) return;
    const FName Name = PropertyChangedEvent.MemberProperty
        ? PropertyChangedEvent.MemberProperty->GetFName()
        : PropertyChangedEvent.Property ? PropertyChangedEvent.Property->GetFName() : NAME_None;
    if (Name == GET_MEMBER_NAME_CHECKED(UKyaLevelSettings, Bank))
    {
        LoadFromBank();
        return;
    }
    ApplyStatus = TEXT("Unsaved settings. Save to apply, or Reload from Bank to discard.");
}
#endif
