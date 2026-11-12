#include "KyaActorEditService.h"

#if WITH_EDITOR

#include "KyaLevelPreview.h"
#include "Containers/Ticker.h"

FString FKyaActorRecordKey::ToString() const
{
    return FString::Printf(TEXT("%s[%d]@%u"), Bank.IsValid() ? *Bank->GetName() : TEXT("<null>"),
        RecordIndex, Generation);
}

bool FKyaActorEditPatch::IsEmpty() const
{
    return !Name.IsSet() && !MeshIndex.IsSet() && !TextureIndex.IsSet() && !CollisionIndex.IsSet() &&
        !Position.IsSet() && !Rotation.IsSet() && !Scale.IsSet() && !DefaultBehaviourId.IsSet() &&
        !SectorId.IsSet() && !Clipping.IsSet() && !VisibilityDistance.IsSet() && !CullingDistance.IsSet() &&
        !LightingValue.IsSet() && !LightingFlags.IsSet() && !Custom;
}

FString FKyaActorEditPatch::Describe() const
{
    TArray<FString> Fields;
    if (Name.IsSet()) Fields.Add(TEXT("Name"));
    if (MeshIndex.IsSet()) Fields.Add(TEXT("MeshIndex"));
    if (TextureIndex.IsSet()) Fields.Add(TEXT("TextureIndex"));
    if (CollisionIndex.IsSet()) Fields.Add(TEXT("CollisionIndex"));
    if (Position.IsSet()) Fields.Add(TEXT("Position"));
    if (Rotation.IsSet()) Fields.Add(TEXT("Rotation"));
    if (Scale.IsSet()) Fields.Add(TEXT("Scale"));
    if (DefaultBehaviourId.IsSet()) Fields.Add(TEXT("DefaultBehaviourId"));
    if (SectorId.IsSet()) Fields.Add(TEXT("SectorId"));
    if (Clipping.IsSet()) Fields.Add(TEXT("Clipping"));
    if (VisibilityDistance.IsSet()) Fields.Add(TEXT("VisibilityDistance"));
    if (CullingDistance.IsSet()) Fields.Add(TEXT("CullingDistance"));
    if (LightingValue.IsSet()) Fields.Add(TEXT("LightingValue"));
    if (LightingFlags.IsSet()) Fields.Add(TEXT("LightingFlags"));
    if (Custom) Fields.Add(TEXT("Custom"));
    return FString::Join(Fields, TEXT(","));
}

namespace
{
bool ApplyPatchToRecord(FKyaActorPlacement& Placement, const FKyaActorEditPatch& Patch, FString& OutError)
{
    if (Patch.Name.IsSet()) Placement.Name = Patch.Name.GetValue();
    if (Patch.MeshIndex.IsSet()) Placement.MeshIndex = Patch.MeshIndex.GetValue();
    if (Patch.TextureIndex.IsSet()) Placement.TextureIndex = Patch.TextureIndex.GetValue();
    if (Patch.CollisionIndex.IsSet()) Placement.CollisionIndex = Patch.CollisionIndex.GetValue();
    if (Patch.Position.IsSet()) Placement.Position = Patch.Position.GetValue();
    if (Patch.Rotation.IsSet()) Placement.Rotation = Patch.Rotation.GetValue();
    if (Patch.Scale.IsSet()) Placement.Scale = Patch.Scale.GetValue();
    if (Patch.DefaultBehaviourId.IsSet()) Placement.DefaultBehaviourId = Patch.DefaultBehaviourId.GetValue();
    if (Patch.SectorId.IsSet()) Placement.SectorId = Patch.SectorId.GetValue();
    if (Patch.Clipping.IsSet()) Placement.Clipping = Patch.Clipping.GetValue();
    if (Patch.VisibilityDistance.IsSet()) Placement.VisibilityDistance = Patch.VisibilityDistance.GetValue();
    if (Patch.CullingDistance.IsSet()) Placement.CullingDistance = Patch.CullingDistance.GetValue();
    if (Patch.LightingValue.IsSet()) Placement.LightingValue = Patch.LightingValue.GetValue();
    if (Patch.LightingFlags.IsSet()) Placement.LightingFlags = Patch.LightingFlags.GetValue();
    if (Patch.Custom && !Patch.Custom(Placement, OutError)) return false;
    return true;
}
}

bool ValidateKyaActorPlacement(const FKyaActorPlacement& Placement, FString& OutError)
{
    if (Placement.Position.ContainsNaN() || Placement.Position.GetAbsMax() > 1.0e7)
    {
        OutError = TEXT("Actor position is not a finite game-space value.");
        return false;
    }
    if (Placement.Rotation.ContainsNaN() || Placement.Rotation.GetAbsMax() > 1.0e5)
    {
        OutError = TEXT("Actor rotation is not a finite angle set.");
        return false;
    }
    if (Placement.Scale.ContainsNaN() || Placement.Scale.X == 0.0f || Placement.Scale.Y == 0.0f ||
        Placement.Scale.Z == 0.0f || Placement.Scale.GetAbsMax() > 1.0e5)
    {
        OutError = TEXT("Actor scale must be non-zero and finite on every axis.");
        return false;
    }
    if (Placement.Name.Len() > 63)
    {
        OutError = TEXT("Actor name is longer than the serialized 63-character limit.");
        return false;
    }
    for (const TCHAR Char : Placement.Name)
    {
        if (Char < 1 || Char > 127)
        {
            OutError = TEXT("Actor name must contain ASCII characters only; the bank writer stores one byte per character.");
            return false;
        }
    }
    if (Placement.MeshIndex < -1 || Placement.TextureIndex < -1 || Placement.CollisionIndex < -1 ||
        Placement.SectorId < -1)
    {
        OutError = TEXT("Actor references must be -1 (none) or a non-negative index.");
        return false;
    }
    return true;
}

FKyaLevelEditService& FKyaLevelEditService::Get()
{
    static FKyaLevelEditService Service;
    return Service;
}

FKyaActorRecordKey FKyaLevelEditService::MakeKey(UKyaLevelBank* Bank, int32 ActorIndex) const
{
    FKyaActorRecordKey Key;
    Key.Bank = Bank;
    Key.RecordIndex = ActorIndex;
    Key.Generation = Bank ? uint32(Bank->EditGeneration) : 0;
    return Key;
}

bool FKyaLevelEditService::Resolve(const FKyaActorRecordKey& Key, FKyaActorPlacement*& OutPlacement,
    FString* OutError) const
{
    OutPlacement = nullptr;
    UKyaLevelBank* Bank = Key.Bank.Get();
    if (!Bank)
    {
        if (OutError) *OutError = TEXT("The bank asset no longer exists.");
        return false;
    }
    if (Key.Generation != uint32(Bank->EditGeneration))
    {
        if (OutError) *OutError = FString::Printf(
            TEXT("Record identity is stale (bank generation %u, key %u); reselect the actor after reimport."),
            uint32(Bank->EditGeneration), Key.Generation);
        return false;
    }
    if (!Bank->SceneEnvironment.Actors.IsValidIndex(Key.RecordIndex))
    {
        if (OutError) *OutError = FString::Printf(TEXT("Actor index %d does not exist in %s."),
            Key.RecordIndex, *Bank->GetName());
        return false;
    }
    OutPlacement = &Bank->SceneEnvironment.Actors[Key.RecordIndex];
    return true;
}

bool FKyaLevelEditService::Apply(const FKyaActorRecordKey& Key, const FKyaActorEditPatch& Patch,
    EKyaEditNotify Notify, FString* OutError)
{
    FString Error;
    FKyaActorPlacement* Placement = nullptr;
    if (!Resolve(Key, Placement, &Error))
    {
        if (OutError) *OutError = Error;
        return false;
    }
    if (Patch.IsEmpty())
    {
        if (OutError) *OutError = TEXT("The edit patch does not change any property.");
        return false;
    }
    UKyaLevelBank* Bank = Key.Bank.Get();

    // Validate the prospective record before touching the bank so a rejected edit
    // never leaves a partially modified record behind.
    FKyaActorPlacement Candidate = *Placement;
    if (!ApplyPatchToRecord(Candidate, Patch, Error) || !ValidateKyaActorPlacement(Candidate, Error))
    {
        if (OutError) *OutError = Error;
        return false;
    }

    Bank->Modify();
    if (!Resolve(Key, Placement, &Error))
    {
        if (OutError) *OutError = Error;
        return false;
    }
    // The candidate was validated above; assign it rather than executing a possibly
    // stateful custom patch a second time.
    *Placement = Candidate;
    Bank->MarkPackageDirty();
    if (OnBankEdited.IsBound()) OnBankEdited.Broadcast(Bank);
    if (Notify == EKyaEditNotify::Notify) NotifyBankChanged(Bank, true);
    else if (Notify == EKyaEditNotify::Deferred) NotifyBankChangedDeferred(Bank);
    else PendingBanks.Add(Bank);
    return true;
}

bool FKyaLevelEditService::ApplyPosition(const FKyaActorRecordKey& Key, const FVector& Position,
    EKyaEditNotify Notify, FString* OutError)
{
    FKyaActorEditPatch Patch;
    Patch.Position = Position;
    return Apply(Key, Patch, Notify, OutError);
}

void FKyaLevelEditService::RegisterView(UKyaLevelBank* Bank, AKyaLevelPreview* View)
{
    if (!Bank || !View) return;
    TArray<TWeakObjectPtr<AKyaLevelPreview>>& BankViews = Views.FindOrAdd(Bank);
    BankViews.RemoveAll([](const TWeakObjectPtr<AKyaLevelPreview>& Item) { return !Item.IsValid(); });
    BankViews.AddUnique(View);
}

void FKyaLevelEditService::UnregisterView(UKyaLevelBank* Bank, AKyaLevelPreview* View)
{
    if (!Bank || !View) return;
    if (TArray<TWeakObjectPtr<AKyaLevelPreview>>* BankViews = Views.Find(Bank))
    {
        BankViews->RemoveAll([View](const TWeakObjectPtr<AKyaLevelPreview>& Item) { return Item.Get() == View; });
        if (BankViews->IsEmpty()) Views.Remove(Bank);
    }
}

void FKyaLevelEditService::UnregisterView(AKyaLevelPreview* View)
{
    if (!View) return;
    for (auto It = Views.CreateIterator(); It; ++It)
    {
        It.Value().RemoveAll([View](const TWeakObjectPtr<AKyaLevelPreview>& Item) { return Item.Get() == View; });
        if (It.Value().IsEmpty()) It.RemoveCurrent();
    }
}

int32 FKyaLevelEditService::GetViewCount(UKyaLevelBank* Bank) const
{
    const TArray<TWeakObjectPtr<AKyaLevelPreview>>* BankViews = Views.Find(Bank);
    if (!BankViews) return 0;
    int32 Count = 0;
    for (const TWeakObjectPtr<AKyaLevelPreview>& View : *BankViews) if (View.IsValid()) ++Count;
    return Count;
}

void FKyaLevelEditService::NotifyBankChanged(UKyaLevelBank* Bank, bool bImmediate)
{
    if (!Bank) return;
    PendingBanks.Add(Bank);
    if (bImmediate) FlushPendingNotifications();
}

void FKyaLevelEditService::NotifyBankChangedDeferred(UKyaLevelBank* Bank)
{
    if (!Bank) return;
    PendingBanks.Add(Bank);
    if (bFlushScheduled) return;
    bFlushScheduled = true;
    // Coalesce every edit of the current frame (for example each actor in a
    // multi-selection drag) into one refresh per dependent preview.
    FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([this](float)
    {
        FlushPendingNotifications();
        return false;
    }), 0.0f);
}

void FKyaLevelEditService::FlushPendingNotifications()
{
    bFlushScheduled = false;
    if (bFlushing || PendingBanks.IsEmpty()) return;
    TGuardValue<bool> Guard(bFlushing, true);
    TArray<TWeakObjectPtr<UKyaLevelBank>> Banks = PendingBanks.Array();
    PendingBanks.Reset();
    for (const TWeakObjectPtr<UKyaLevelBank>& Bank : Banks)
    {
        if (!Bank.IsValid()) continue;
        TArray<TWeakObjectPtr<AKyaLevelPreview>>& BankViews = Views.FindOrAdd(Bank.Get());
        // A preview may be destroyed while refreshing; snapshot first so the array
        // cannot change underneath the loop.
        const TArray<TWeakObjectPtr<AKyaLevelPreview>> Snapshot = BankViews;
        for (const TWeakObjectPtr<AKyaLevelPreview>& View : Snapshot)
            if (View.IsValid()) View->HandleBankEdited(Bank.Get());
    }
}

void FKyaLevelEditService::Reset()
{
    Views.Reset();
    PendingBanks.Reset();
    OnBankEdited.Clear();
}

#endif
