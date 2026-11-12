#pragma once

#include "CoreMinimal.h"
#include "Misc/Optional.h"
#include "Templates/Function.h"
#include "KyaLevelBank.h"

class UKyaLevelBank;
class AKyaLevelPreview;

/** Stable identity of one bank actor record. */
struct KYALEVEL_API FKyaActorRecordKey
{
    TWeakObjectPtr<UKyaLevelBank> Bank;
    /** Index in the bank's actor array. Counts are fixed while the writer requires it. */
    int32 RecordIndex = INDEX_NONE;
    /** Bank edit generation; stale keys must not resolve after a reimport or replacement. */
    uint32 Generation = 0;

    bool IsSet() const { return Bank.IsValid() && RecordIndex >= 0; }
    bool operator==(const FKyaActorRecordKey& Other) const
    {
        return Bank == Other.Bank && RecordIndex == Other.RecordIndex && Generation == Other.Generation;
    }
    friend uint32 GetTypeHash(const FKyaActorRecordKey& Key)
    {
        return HashCombine(HashCombine(GetTypeHash(Key.Bank), GetTypeHash(Key.RecordIndex)), GetTypeHash(Key.Generation));
    }
    FString ToString() const;
};

/**
 * A change to one actor record. Only the fields that the user actually edited are
 * set; everything else is left untouched, so unrelated newer values are preserved.
 */
struct KYALEVEL_API FKyaActorEditPatch
{
    TOptional<FString> Name;
    TOptional<int32> MeshIndex;
    TOptional<int32> TextureIndex;
    TOptional<int32> CollisionIndex;
    TOptional<FVector> Position;
    TOptional<FVector> Rotation;
    TOptional<FVector> Scale;
    TOptional<int32> DefaultBehaviourId;
    TOptional<int32> SectorId;
    TOptional<float> Clipping;
    TOptional<float> VisibilityDistance;
    TOptional<float> CullingDistance;
    TOptional<float> LightingValue;
    TOptional<int64> LightingFlags;
    /** Nested or class-specific edit. Return false and fill OutError to reject the patch. */
    TFunction<bool(FKyaActorPlacement&, FString&)> Custom;

    bool IsEmpty() const;
    /** Comma separated field names, for logs and diagnostics. */
    FString Describe() const;
};

enum class EKyaEditNotify : uint8
{
    /** Refresh dependent previews immediately (default for discrete details edits). */
    Notify,
    /** Coalesce the refresh into one flush at the end of the frame (interactive drags). */
    Deferred,
    /** Leave refresh to a later FlushPendingNotifications call (bulk operations and tests). */
    Silent
};

/**
 * Single edit path for bank actor records.
 *
 * Resolves record identity, validates the change, captures the bank state before
 * mutating (Modify), applies only the patched fields, marks the package dirty and
 * tells every preview that displays the bank to refresh once.
 *
 * Editor-only: compiled when WITH_EDITOR.
 */
class KYALEVEL_API FKyaLevelEditService
{
public:
    DECLARE_MULTICAST_DELEGATE_OneParam(FOnBankRecordEdited, UKyaLevelBank*);

    static FKyaLevelEditService& Get();

    FKyaActorRecordKey MakeKey(UKyaLevelBank* Bank, int32 ActorIndex) const;
    /** Resolve a key, rejecting stale generations and out-of-range indices. */
    bool Resolve(const FKyaActorRecordKey& Key, FKyaActorPlacement*& OutPlacement, FString* OutError = nullptr) const;

    bool Apply(const FKyaActorRecordKey& Key, const FKyaActorEditPatch& Patch, EKyaEditNotify Notify,
        FString* OutError = nullptr);
    bool ApplyPosition(const FKyaActorRecordKey& Key, const FVector& Position, EKyaEditNotify Notify,
        FString* OutError = nullptr);

    void RegisterView(UKyaLevelBank* Bank, AKyaLevelPreview* View);
    void UnregisterView(UKyaLevelBank* Bank, AKyaLevelPreview* View);
    void UnregisterView(AKyaLevelPreview* View);
    /** Queue a refresh of every preview that displays Bank. */
    void NotifyBankChanged(UKyaLevelBank* Bank, bool bImmediate = true);
    /** Queue a refresh for the end of the current frame; multiple edits coalesce. */
    void NotifyBankChangedDeferred(UKyaLevelBank* Bank);
    void FlushPendingNotifications();
    int32 GetViewCount(UKyaLevelBank* Bank) const;

    /** Fired after a successful edit; asset editors and tooling can observe bank changes. */
    FOnBankRecordEdited OnBankEdited;

    /** Drop pending notifications and views (module shutdown and tests). */
    void Reset();

private:
    TMap<TWeakObjectPtr<UKyaLevelBank>, TArray<TWeakObjectPtr<AKyaLevelPreview>>> Views;
    TSet<TWeakObjectPtr<UKyaLevelBank>> PendingBanks;
    bool bFlushing = false;
    bool bFlushScheduled = false;
};

/** Validate a record before it is written; used by the edit service and by export preflight. */
KYALEVEL_API bool ValidateKyaActorPlacement(const FKyaActorPlacement& Placement, FString& OutError);
