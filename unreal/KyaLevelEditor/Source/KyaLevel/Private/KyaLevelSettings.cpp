#include "KyaLevelSettings.h"

#include "KyaActorEditService.h"
#include "Containers/Ticker.h"
#include "Misc/TransactionObjectEvent.h"
#include "UObject/UObjectGlobals.h"
#if WITH_EDITOR
#include "Editor.h"
#include "ScopedTransaction.h"
#endif

void UKyaLevelSettings::RefreshFromBank()
{
    if (!Bank) return;
    Environment = Bank->SceneEnvironment;
    SceneConfiguration = Bank->SceneConfiguration;
}

void UKyaLevelSettings::ScheduleRefresh()
{
    if (bReloadScheduled) return;
    bReloadScheduled = true;
    // Undo/redo is notified while the transaction is still unwinding; refresh on the
    // next tick so the copy sees the final restored state.
    FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateWeakLambda(this, [this](float)
    {
        bReloadScheduled = false;
        if (!bApplying) RefreshFromBank();
        return false;
    }), 0.0f);
}

void UKyaLevelSettings::LoadFromBank()
{
    if (!Bank) return;
    RefreshFromBank();
#if WITH_EDITOR
    FKyaLevelEditService& Service = FKyaLevelEditService::Get();
    if (BankEditedHandle.IsValid()) Service.OnBankEdited.Remove(BankEditedHandle);
    if (BankTransactedHandle.IsValid())
        FCoreUObjectDelegates::OnObjectTransacted.Remove(BankTransactedHandle);
    // External bank edits and reimports.
    BankEditedHandle = Service.OnBankEdited.AddWeakLambda(this, [this](UKyaLevelBank* EditedBank)
    {
        if (bApplying || EditedBank != Bank) return;
        RefreshFromBank();
    });
    // Undo/redo.
    BankTransactedHandle = FCoreUObjectDelegates::OnObjectTransacted.AddWeakLambda(this,
        [this](UObject* Object, const FTransactionObjectEvent& Event)
        {
            if (!bApplying && Object == Bank) ScheduleRefresh();
        });
#endif
}

bool UKyaLevelSettings::ApplyToBank(FString& OutError)
{
    if (!Bank)
    {
        OutError = TEXT("Select a bank before applying level settings.");
        return false;
    }
#if WITH_EDITOR
    FKyaLevelEditService& Service = FKyaLevelEditService::Get();
    TGuardValue<bool> Guard(bApplying, true);
    bool bApplied = false;
    if (GEditor && !GEditor->IsTransactionActive())
    {
        FScopedTransaction Transaction(NSLOCTEXT("KyaLevelEditor", "KyaLevelSettingsEdit", "Edit Kya Level Settings"));
        bApplied = Service.ApplySceneSettings(Bank, Environment, SceneConfiguration, EKyaEditNotify::Notify,
            &OutError);
    }
    else
    {
        bApplied = Service.ApplySceneSettings(Bank, Environment, SceneConfiguration, EKyaEditNotify::Notify,
            &OutError);
    }
    ApplyStatus = bApplied ? TEXT("Applied to the bank.") : OutError;
    return bApplied;
#else
    OutError = TEXT("Editing level settings requires the editor.");
    return false;
#endif
}

void UKyaLevelSettings::BeginDestroy()
{
#if WITH_EDITOR
    if (BankEditedHandle.IsValid()) FKyaLevelEditService::Get().OnBankEdited.Remove(BankEditedHandle);
    if (BankTransactedHandle.IsValid())
        FCoreUObjectDelegates::OnObjectTransacted.Remove(BankTransactedHandle);
#endif
    Super::BeginDestroy();
}

#if WITH_EDITOR
void UKyaLevelSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);
    if (bApplying) return;
    const FName Name = PropertyChangedEvent.MemberProperty
        ? PropertyChangedEvent.MemberProperty->GetFName()
        : PropertyChangedEvent.Property ? PropertyChangedEvent.Property->GetFName() : NAME_None;
    // Selecting another bank reloads this surface; it must never write the old
    // bank's settings into the newly selected bank.
    if (Name == GET_MEMBER_NAME_CHECKED(UKyaLevelSettings, Bank))
    {
        LoadFromBank();
        return;
    }
    if (!Bank) return;
    FString Error;
    if (!ApplyToBank(Error))
        UE_LOG(LogTemp, Warning, TEXT("KYA_LEVEL_SETTINGS_APPLY_FAILED bank=%s reason=%s"),
            *GetNameSafe(Bank), *Error);
}
#endif
