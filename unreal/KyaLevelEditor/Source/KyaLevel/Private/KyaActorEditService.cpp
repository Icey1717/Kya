#include "KyaActorEditService.h"

#if WITH_EDITOR

#include "KyaLevelPreview.h"
#include "KyaActorCatalog.h"
#include "KyaStructuralEdits.h"
#include "KyaExportedActorFields.h"
#if WITH_EDITOR
#include "Editor.h"
#include "ScopedTransaction.h"
#endif
#include "Containers/Ticker.h"

namespace
{
#if WITH_EDITOR
/**
 * One undo step for direct service/scripted calls. When the caller already opened a
 * transaction (viewport drag, Details edit, bulk command) this just participates.
 */
struct FKyaServiceTransaction
{
    TUniquePtr<FScopedTransaction> Transaction;
    explicit FKyaServiceTransaction(const FText& Label)
    {
        if (!GIsTransacting) Transaction = MakeUnique<FScopedTransaction>(Label);
    }
};

#define KYA_SERVICE_TRANSACTION(Label) \
    FKyaServiceTransaction TransactionScope(NSLOCTEXT("KyaLevel", Label, Label))
#else
#define KYA_SERVICE_TRANSACTION(Label)
#endif
}

FString FKyaActorRecordKey::ToString() const
{
    return FString::Printf(TEXT("%s[%d]@%u"), Bank.IsValid() ? *Bank->GetName() : TEXT("<null>"),
        RecordIndex, Generation);
}

FString FKyaWaypointRecordKey::ToString() const
{
    return FString::Printf(TEXT("%s waypoint %d@%u"), Bank.IsValid() ? *Bank->GetName() : TEXT("<null>"),
        WaypointIndex, Generation);
}

FString FKyaPathPointRecordKey::ToString() const
{
    return FString::Printf(TEXT("%s path %d point %d@%u"), Bank.IsValid() ? *Bank->GetName() : TEXT("<null>"),
        PathIndex, PointIndex, Generation);
}

bool FKyaWaypointEditPatch::IsEmpty() const
{
    return !Position.IsSet() && !Rotation.IsSet() && !Parameters.IsSet();
}

bool FKyaPathPointEditPatch::IsEmpty() const
{
    return !Point.IsSet() && !Rotation.IsSet() && !ExtraVector.IsSet() && !Delay.IsSet() && !Flags.IsSet() &&
        !ExtraValue.IsSet() && !ExtraWord.IsSet();
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

bool ValidateVector(const FVector& Value, const TCHAR* Label, FString& OutError)
{
    if (Value.ContainsNaN() || Value.GetAbsMax() > 1.0e7)
    {
        OutError = FString::Printf(TEXT("%s is not a finite value."), Label);
        return false;
    }
    return true;
}

bool ValidateVector4(const FVector4& Value, const TCHAR* Label, FString& OutError)
{
    if (Value.ContainsNaN() || FMath::Max(FMath::Max3(FMath::Abs(Value.X), FMath::Abs(Value.Y),
        FMath::Abs(Value.Z)), FMath::Abs(Value.W)) > 1.0e7)
    {
        OutError = FString::Printf(TEXT("%s is not a finite value."), Label);
        return false;
    }
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
        Placement.SectorId < -1 || Placement.DefaultBehaviourId < -1)
    {
        OutError = TEXT("Actor references must be -1 (none) or a non-negative index.");
        return false;
    }
    // Serialized widths: fields stored as int64 are written as unsigned 32-bit words,
    // while int32 fields preserve their full bit pattern (0x80000000 imports as a
    // negative int32 and must stay editable).
    const auto RequireU32 = [&OutError](int64 Value, const TCHAR* Label) -> bool
    {
        if (Value >= 0 && Value <= MAX_uint32) return true;
        OutError = FString::Printf(TEXT("%s must fit the serialized unsigned 32-bit field (0..%u)."), Label,
            MAX_uint32);
        return false;
    };
    if (!RequireU32(Placement.LightingFlags, TEXT("Lighting flags")) ||
        !RequireU32(Placement.HashCode, TEXT("Hash code")))
        return false;
    return true;
}

bool KyaMakePlacementPropertyPatch(const FKyaActorPlacement& Source, FName PropertyName,
    FKyaActorEditPatch& OutPatch, FString* OutError)
{
    const FKyaFieldCapability Capability = GetKyaActorFieldCapability(PropertyName);
    if (Capability.Kind == EKyaFieldCapability::InspectOnly ||
        Capability.Kind == EKyaFieldCapability::Unsupported)
    {
        if (OutError) *OutError = Capability.Reason;
        return false;
    }
    FProperty* Property = FKyaActorPlacement::StaticStruct()->FindPropertyByName(PropertyName);
    if (!Property) return false;
    // Capture the edited value, not a pointer into a temporary or array element.
    OutPatch.Custom = [Property, Source](FKyaActorPlacement& Record, FString& OutError) -> bool
    {
        Property->CopyCompleteValue_InContainer(&Record, &Source);
        return true;
    };
    return true;
}

FText DescribeKyaFieldCapability(EKyaFieldCapability Kind)
{
    switch (Kind)
    {
    case EKyaFieldCapability::EditableExportable:
        return NSLOCTEXT("KyaLevel", "CapabilityEditable", "Editable and exported");
    case EKyaFieldCapability::EditablePresentationOnly:
        return NSLOCTEXT("KyaLevel", "CapabilityPresentation", "Editable (presentation only)");
    case EKyaFieldCapability::InspectOnly:
        return NSLOCTEXT("KyaLevel", "CapabilityInspect", "Inspect only");
    default:
        return NSLOCTEXT("KyaLevel", "CapabilityUnsupported", "Unsupported");
    }
}

FKyaFieldCapability GetKyaActorFieldCapability(FName PropertyName)
{
    FKyaFieldCapability Result;
    const FProperty* Property = FKyaActorPlacement::StaticStruct()->FindPropertyByName(PropertyName);
    if (!Property)
    {
        Result.Kind = EKyaFieldCapability::Unsupported;
        Result.Reason = TEXT("The property is not part of the actor placement schema.");
        return Result;
    }
    if (!Property->HasAnyPropertyFlags(CPF_Edit) || Property->HasAnyPropertyFlags(CPF_EditConst))
    {
        Result.Kind = EKyaFieldCapability::InspectOnly;
        Result.Reason = TEXT("Read-only record data; the bank writer does not accept edits for this field.");
        return Result;
    }
    const FString Name = PropertyName.ToString();
    if (Name.StartsWith(TEXT("bHas")))
    {
        Result.Kind = EKyaFieldCapability::InspectOnly;
        Result.Reason = TEXT("Presence flag that controls the serialized layout; changing it would desynchronize the writer.");
        return Result;
    }
    if (PropertyName == GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, Name))
    {
        Result.Kind = EKyaFieldCapability::EditableExportable;
        Result.Reason = TEXT("Serialized into the original fixed-size name slot: the writer requires Align(length + 1, 4) "
            "to equal the original slot capacity, so the new name must stay in the same 4-byte alignment bucket and "
            "use single-byte characters.");
        return Result;
    }
    // Export support comes from the writer's own field references, not from
    // EditAnywhere: a field the exporter never encodes is inspect-only here.
    if (!KyaExportedActorFields::Get().Contains(PropertyName))
    {
        Result.Kind = EKyaFieldCapability::InspectOnly;
        Result.Reason = TEXT("The current bank writer does not encode this field; edits would not be exported.");
        return Result;
    }
    if (PropertyName == GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, ClassData))
    {
        Result.Kind = EKyaFieldCapability::EditableExportable;
        Result.Reason = TEXT("Raw class payload: the writer preserves its original size and unknown bytes.");
        return Result;
    }
    if (const FArrayProperty* Array = CastField<FArrayProperty>(Property))
    {
        Result.Kind = EKyaFieldCapability::EditableExportable;
        Result.Reason = TEXT("Fixed-size array: element edits are exported; the count and serialized sizes are preserved.");
        return Result;
    }
    Result.Kind = EKyaFieldCapability::EditableExportable;
    Result.Reason = TEXT("Serialized typed field written by the bank exporter.");
    if (const FString* Condition = Property->FindMetaData(TEXT("EditCondition")))
        if (Condition->Contains(TEXT("bHas")))
            Result.Reason += TEXT(" Only meaningful while its presence flag is set.");
    return Result;
}

FKyaFieldCapability GetKyaNestedFieldCapability(const FString& PropertyPath)
{
    FKyaFieldCapability Result;
    FString TopLevel;
    FString Nested;
    if (!PropertyPath.Split(TEXT("."), &TopLevel, &Nested) || Nested.IsEmpty())
    {
        Result.Kind = EKyaFieldCapability::Unsupported;
        Result.Reason = TEXT("Nested capability paths must be written as TopLevel.Field.");
        return Result;
    }
    const FKyaFieldCapability TopCapability = GetKyaActorFieldCapability(FName(*TopLevel));
    if (TopCapability.Kind != EKyaFieldCapability::EditableExportable)
    {
        Result = TopCapability;
        Result.Reason = FString::Printf(TEXT("%s: %s"), *TopLevel, *TopCapability.Reason);
        return Result;
    }
    const FProperty* TopProperty = FKyaActorPlacement::StaticStruct()->FindPropertyByName(FName(*TopLevel));
    const FProperty* NestedProperty = nullptr;
    if (const FArrayProperty* Array = CastField<FArrayProperty>(TopProperty))
    {
        if (const FStructProperty* Inner = CastField<FStructProperty>(Array->Inner))
            NestedProperty = Inner->Struct->FindPropertyByName(FName(*Nested));
    }
    else if (const FStructProperty* StructProperty = CastField<FStructProperty>(TopProperty))
    {
        NestedProperty = StructProperty->Struct->FindPropertyByName(FName(*Nested));
    }
    if (!NestedProperty)
    {
        Result.Kind = EKyaFieldCapability::Unsupported;
        Result.Reason = FString::Printf(TEXT("%s is not a field of the %s record."), *Nested, *TopLevel);
        return Result;
    }
    // Nested records follow the same rules as top-level ones: read-only serialized
    // layout data and presence flags are not editable just because the parent is.
    if (!NestedProperty->HasAnyPropertyFlags(CPF_Edit) ||
        NestedProperty->HasAnyPropertyFlags(CPF_EditConst))
    {
        Result.Kind = EKyaFieldCapability::InspectOnly;
        Result.Reason = TEXT("Read-only nested record data; the bank writer does not accept edits for this field.");
        return Result;
    }
    if (NestedProperty->GetName().StartsWith(TEXT("bHas")))
    {
        Result.Kind = EKyaFieldCapability::InspectOnly;
        Result.Reason = TEXT("Nested presence flag that controls the serialized layout; changing it would desynchronize the writer.");
        return Result;
    }
    Result.Kind = EKyaFieldCapability::EditableExportable;
    if (CastField<FStrProperty>(NestedProperty) || CastField<FNameProperty>(NestedProperty))
        Result.Reason = TEXT("Fixed-length string record: the writer requires the aligned (4-byte) length to equal "
            "the original serialized slot, so the value must stay in the same alignment bucket.");
    else
        Result.Reason = FString::Printf(TEXT("Nested typed field of %s; element edits are exported with the parent record."),
            *TopLevel);
    return Result;
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

namespace
{
bool ValidateChangedReferences(const UKyaLevelBank& Bank, const FKyaActorPlacement& Original,
    const FKyaActorPlacement& Candidate, FString& OutError)
{
    // Validate changed references independently of how the patch was built (typed
    // fields or a Custom presentation patch from Details), so Details edits cannot
    // commit an unresolved reference.
    if (Candidate.MeshIndex != Original.MeshIndex)
    {
        const FKyaBankReference Mesh = ResolveKyaModelReference(Bank, Candidate.MeshIndex);
        if (!Mesh.bResolved)
        {
            OutError = Mesh.Warning;
            return false;
        }
    }
    if (Candidate.TextureIndex != Original.TextureIndex)
    {
        const FKyaBankReference Texture = ResolveKyaTextureReference(Bank, Candidate.TextureIndex);
        if (!Texture.bResolved)
        {
            OutError = Texture.Warning;
            return false;
        }
    }
    if (Candidate.SectorId != Original.SectorId && Candidate.SectorId < -1)
    {
        OutError = TEXT("Sector id must be -1 (global) or a non-negative sector id.");
        return false;
    }
    if (Candidate.DefaultBehaviourId != Original.DefaultBehaviourId &&
        (Candidate.DefaultBehaviourId < -1 || Candidate.DefaultBehaviourId >= Candidate.Behaviours.Num()))
    {
        OutError = FString::Printf(
            TEXT("Default behaviour %d does not exist in this actor's %d behaviour record(s)."),
            Candidate.DefaultBehaviourId, Candidate.Behaviours.Num());
        return false;
    }
    for (const FKyaReferenceField& Field : GetKyaReferenceFields())
    {
        if (Field.Kind != EKyaReferenceKind::Actor && Field.Kind != EKyaReferenceKind::Waypoint &&
            Field.Kind != EKyaReferenceKind::Path) continue;
        if (Field.ClassId != -1 && Field.ClassId != Candidate.ClassId) continue;
        const int64 OldValue = GetKyaNumericField(Original, Field.PropertyName);
        const int64 NewValue = GetKyaNumericField(Candidate, Field.PropertyName);
        if (OldValue == NewValue) continue;
        int32 Limit = 0;
        switch (Field.Kind)
        {
        case EKyaReferenceKind::Actor: Limit = Bank.SceneEnvironment.Actors.Num(); break;
        case EKyaReferenceKind::Waypoint: Limit = Bank.SceneEnvironment.Waypoints.Num(); break;
        default: Limit = Bank.SceneEnvironment.Paths.Num(); break;
        }
        if (NewValue != -1 && (NewValue < 0 || NewValue >= Limit))
        {
            OutError = FString::Printf(
                TEXT("%s %s: %lld does not resolve to a %s in this bank (%d available)."),
                *Field.PropertyName.ToString(), *DescribeKyaReferenceKind(Field.Kind).ToString(), NewValue,
                Field.Kind == EKyaReferenceKind::Actor ? TEXT("actor") :
                    Field.Kind == EKyaReferenceKind::Waypoint ? TEXT("waypoint") : TEXT("path"), Limit);
            return false;
        }
    }
    // The writer requires Align(length + 1, 4) to equal the original name-slot
    // capacity, so a name that changes the 4-byte bucket cannot be exported.
    if (Candidate.Name != Original.Name)
    {
        const auto Align4 = [](int32 Value) { return (Value + 3) & ~3; };
        if (Align4(Candidate.Name.Len() + 1) != Align4(Original.Name.Len() + 1))
        {
            OutError = FString::Printf(
                TEXT("Actor name changes the serialized 4-byte length bucket (%d -> %d); the writer requires "
                    "Align(length + 1, 4) to equal the original slot capacity."),
                Align4(Original.Name.Len() + 1), Align4(Candidate.Name.Len() + 1));
            return false;
        }
    }
    return true;
}
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
    if (!ApplyPatchToRecord(Candidate, Patch, Error) || !ValidateKyaActorPlacement(Candidate, Error) ||
        !ValidateKyaActorStructuralEdit(*Placement, Candidate, Error) ||
        !ValidateChangedReferences(*Bank, *Placement, Candidate, Error))
    {
        if (OutError) *OutError = Error;
        return false;
    }

    // An edit that matches the current record is not a change: no transaction, no
    // dirty flag and no notifications, so scripted re-applies stay clean.
    if (FKyaActorPlacement::StaticStruct()->CompareScriptStruct(Placement, &Candidate, PPF_None))
        return true;

    // Direct service/scripted calls must produce one undo step too; viewport drags
    // and Details edits already own a transaction and this participates in it.
    KYA_SERVICE_TRANSACTION("KyaEditActorRecord");

    Bank->Modify();
    if (!Resolve(Key, Placement, &Error))
    {
        if (OutError) *OutError = Error;
        return false;
    }
    // The candidate was validated above; assign it rather than executing a possibly
    // stateful custom patch a second time.
    bool bModelsChanged = Candidate.Models.Num() != Placement->Models.Num();
    for (int32 ModelIndex = 0; !bModelsChanged && ModelIndex < Candidate.Models.Num(); ++ModelIndex)
    {
        if (Candidate.Models[ModelIndex].MeshName != Placement->Models[ModelIndex].MeshName ||
            Candidate.Models[ModelIndex].TextureName != Placement->Models[ModelIndex].TextureName)
            bModelsChanged = true;
    }
    const bool bGeometryChanged = Candidate.MeshIndex != Placement->MeshIndex ||
        Candidate.TextureIndex != Placement->TextureIndex ||
        Candidate.SectorId != Placement->SectorId ||
        Candidate.LightingValue != Placement->LightingValue ||
        Candidate.LightingFlags != Placement->LightingFlags ||
        !Candidate.Rotation.Equals(Placement->Rotation, 1.0e-5) ||
        !Candidate.Scale.Equals(Placement->Scale, 1.0e-5) ||
        bModelsChanged;
    *Placement = Candidate;
    Bank->MarkPackageDirty();
    if (OnBankEdited.IsBound()) OnBankEdited.Broadcast(Bank);
    if (Notify == EKyaEditNotify::Silent)
    {
        PendingBanks.Add(Bank);
    }
    else if (bGeometryChanged)
    {
        // Mesh/texture/lighting/rotation changes need a rebuilt capture. Deferred
        // (drag) commits coalesce until all selected records have committed.
        if (Notify == EKyaEditNotify::Deferred) NotifyBankChangedDeferred(Bank);
        else NotifyBankChanged(Bank, true);
    }
    else
    {
        NotifyActorRecordEdit(Bank, Key.RecordIndex, Notify == EKyaEditNotify::Deferred);
    }
    return true;
}

bool FKyaLevelEditService::ApplyBulk(const TArray<FKyaActorRecordKey>& Keys, const FKyaActorEditPatch& Patch,
    EKyaEditNotify Notify, FString* OutError)
{
    if (Keys.IsEmpty())
    {
        if (OutError) *OutError = TEXT("No records were supplied for the bulk edit.");
        return false;
    }
    if (Patch.IsEmpty())
    {
        if (OutError) *OutError = TEXT("The bulk edit patch does not change any property.");
        return false;
    }

    struct FEntry
    {
        FKyaActorRecordKey Key;
        UKyaLevelBank* Bank = nullptr;
        FKyaActorPlacement Candidate;
        bool bChanged = false;
    };
    TArray<FEntry> Entries;
    Entries.Reserve(Keys.Num());

    bool bAnyChanged = false;

    // Pass 1: resolve and validate every record before mutating any of them, so a
    // bulk edit either applies completely or not at all.
    for (const FKyaActorRecordKey& Key : Keys)
    {
        FString Error;
        FKyaActorPlacement* Placement = nullptr;
        if (!Resolve(Key, Placement, &Error))
        {
            if (OutError) *OutError = Error;
            return false;
        }
        FEntry& Entry = Entries.AddDefaulted_GetRef();
        Entry.Key = Key;
        Entry.Bank = Key.Bank.Get();
        Entry.Candidate = *Placement;
        if (!ApplyPatchToRecord(Entry.Candidate, Patch, Error) ||
            !ValidateKyaActorPlacement(Entry.Candidate, Error) ||
            !ValidateKyaActorStructuralEdit(*Placement, Entry.Candidate, Error) ||
            !ValidateChangedReferences(*Entry.Bank, *Placement, Entry.Candidate, Error))
        {
            if (OutError) *OutError = FString::Printf(TEXT("Record %d: %s"), Key.RecordIndex, *Error);
            return false;
        }
        Entry.bChanged = !FKyaActorPlacement::StaticStruct()->CompareScriptStruct(
            Placement, &Entry.Candidate, PPF_None);
        if (Entry.bChanged) bAnyChanged = true;

    }
    // A bulk edit that matches every selected record is not a change.
    if (!bAnyChanged) return true;

    // Span the whole bulk edit with one undo step when the caller has none.
    KYA_SERVICE_TRANSACTION("KyaEditActorRecords");

    // Pass 2: one Modify per changed record (inside the caller's transaction) and one
    // dirty marker plus one refresh per affected bank.
    TSet<UKyaLevelBank*> ChangedBanks;
    for (FEntry& Entry : Entries)
    {
        if (!Entry.bChanged) continue;
        ChangedBanks.Add(Entry.Bank);
        Entry.Bank->Modify();
        FKyaActorPlacement* Placement = nullptr;
        if (!Resolve(Entry.Key, Placement, nullptr) || !Placement)
        {
            if (OutError) *OutError = FString::Printf(TEXT("Record %d became stale during the bulk edit."),
                Entry.Key.RecordIndex);
            return false;
        }
        *Placement = Entry.Candidate;
    }
    for (UKyaLevelBank* Bank : ChangedBanks)
    {
        Bank->MarkPackageDirty();
        if (OnBankEdited.IsBound()) OnBankEdited.Broadcast(Bank);
        if (Notify == EKyaEditNotify::Deferred) NotifyBankChangedDeferred(Bank);
        else NotifyBankChanged(Bank, false);
    }
    if (Notify == EKyaEditNotify::Notify) FlushPendingNotifications();
    return true;
}

bool FKyaLevelEditService::ApplyPosition(const FKyaActorRecordKey& Key, const FVector& Position,
    EKyaEditNotify Notify, FString* OutError)
{
    FKyaActorEditPatch Patch;
    Patch.Position = Position;
    return Apply(Key, Patch, Notify, OutError);
}

FKyaWaypointRecordKey FKyaLevelEditService::MakeWaypointKey(UKyaLevelBank* Bank, int32 WaypointIndex) const
{
    FKyaWaypointRecordKey Key;
    Key.Bank = Bank;
    Key.WaypointIndex = WaypointIndex;
    Key.Generation = Bank ? uint32(Bank->EditGeneration) : 0;
    return Key;
}

bool FKyaLevelEditService::ResolveWaypoint(const FKyaWaypointRecordKey& Key, FKyaWaypoint*& OutWaypoint,
    FString* OutError) const
{
    OutWaypoint = nullptr;
    UKyaLevelBank* Bank = Key.Bank.Get();
    if (!Bank)
    {
        if (OutError) *OutError = TEXT("The bank asset no longer exists.");
        return false;
    }
    if (Key.Generation != uint32(Bank->EditGeneration))
    {
        if (OutError) *OutError = FString::Printf(
            TEXT("Waypoint identity is stale (bank generation %u, key %u); reselect after reimport."),
            uint32(Bank->EditGeneration), Key.Generation);
        return false;
    }
    if (!Bank->SceneEnvironment.Waypoints.IsValidIndex(Key.WaypointIndex))
    {
        if (OutError) *OutError = FString::Printf(TEXT("Waypoint index %d does not exist in %s."),
            Key.WaypointIndex, *Bank->GetName());
        return false;
    }
    OutWaypoint = &Bank->SceneEnvironment.Waypoints[Key.WaypointIndex];
    return true;
}

bool FKyaLevelEditService::ApplyWaypoint(const FKyaWaypointRecordKey& Key, const FKyaWaypointEditPatch& Patch,
    EKyaEditNotify Notify, FString* OutError)
{
    FString Error;
    FKyaWaypoint* Waypoint = nullptr;
    if (!ResolveWaypoint(Key, Waypoint, &Error))
    {
        if (OutError) *OutError = Error;
        return false;
    }
    if (Patch.IsEmpty())
    {
        if (OutError) *OutError = TEXT("The waypoint patch does not change any property.");
        return false;
    }
    UKyaLevelBank* Bank = Key.Bank.Get();

    FKyaWaypoint Candidate = *Waypoint;
    bool bValid = true;
    if (Patch.Position.IsSet())
    {
        bValid = ValidateVector(Patch.Position.GetValue(), TEXT("Waypoint position"), Error);
        if (bValid) Candidate.Position = Patch.Position.GetValue();
    }
    if (bValid && Patch.Rotation.IsSet())
    {
        bValid = ValidateVector(Patch.Rotation.GetValue(), TEXT("Waypoint rotation"), Error);
        if (bValid) Candidate.Rotation = Patch.Rotation.GetValue();
    }
    if (bValid && Patch.Parameters.IsSet())
    {
        bValid = ValidateVector(Patch.Parameters.GetValue(), TEXT("Waypoint parameters"), Error);
        if (bValid) Candidate.Parameters = Patch.Parameters.GetValue();
    }
    if (!bValid)
    {
        if (OutError) *OutError = Error;
        return false;
    }
    // An unchanged waypoint is not an edit: no transaction, dirty flag or refresh.
    if (Waypoint->Position.Equals(Candidate.Position) && Waypoint->Rotation.Equals(Candidate.Rotation) &&
        Waypoint->Parameters.Equals(Candidate.Parameters))
        return true;
    KYA_SERVICE_TRANSACTION("KyaEditWaypoint");

    Bank->Modify();
    if (!ResolveWaypoint(Key, Waypoint, &Error))
    {
        if (OutError) *OutError = Error;
        return false;
    }
    *Waypoint = Candidate;
    Bank->MarkPackageDirty();
    if (OnBankEdited.IsBound()) OnBankEdited.Broadcast(Bank);
    if (Notify == EKyaEditNotify::Silent) PendingBanks.Add(Bank);
    else NotifyWaypointEdit(Bank, Key.WaypointIndex);
    return true;
}

FKyaPathPointRecordKey FKyaLevelEditService::MakePathPointKey(UKyaLevelBank* Bank, int32 PathIndex,
    int32 PointIndex) const
{
    FKyaPathPointRecordKey Key;
    Key.Bank = Bank;
    Key.PathIndex = PathIndex;
    Key.PointIndex = PointIndex;
    Key.Generation = Bank ? uint32(Bank->EditGeneration) : 0;
    return Key;
}

bool FKyaLevelEditService::ResolvePathPoint(const FKyaPathPointRecordKey& Key, FKyaPath*& OutPath,
    FString* OutError) const
{
    OutPath = nullptr;
    UKyaLevelBank* Bank = Key.Bank.Get();
    if (!Bank)
    {
        if (OutError) *OutError = TEXT("The bank asset no longer exists.");
        return false;
    }
    if (Key.Generation != uint32(Bank->EditGeneration))
    {
        if (OutError) *OutError = FString::Printf(
            TEXT("Path-point identity is stale (bank generation %u, key %u); reselect after reimport."),
            uint32(Bank->EditGeneration), Key.Generation);
        return false;
    }
    if (!Bank->SceneEnvironment.Paths.IsValidIndex(Key.PathIndex))
    {
        if (OutError) *OutError = FString::Printf(TEXT("Path index %d does not exist in %s."),
            Key.PathIndex, *Bank->GetName());
        return false;
    }
    FKyaPath& Path = Bank->SceneEnvironment.Paths[Key.PathIndex];
    if (!Path.Points.IsValidIndex(Key.PointIndex))
    {
        if (OutError) *OutError = FString::Printf(TEXT("Point %d does not exist in path %d of %s."),
            Key.PointIndex, Key.PathIndex, *Bank->GetName());
        return false;
    }
    OutPath = &Path;
    return true;
}

bool FKyaLevelEditService::ApplyPathPoint(const FKyaPathPointRecordKey& Key, const FKyaPathPointEditPatch& Patch,
    EKyaEditNotify Notify, FString* OutError)
{
    FString Error;
    FKyaPath* Path = nullptr;
    if (!ResolvePathPoint(Key, Path, &Error))
    {
        if (OutError) *OutError = Error;
        return false;
    }
    if (Patch.IsEmpty())
    {
        if (OutError) *OutError = TEXT("The path-point patch does not change any property.");
        return false;
    }
    UKyaLevelBank* Bank = Key.Bank.Get();
    const int32 PointIndex = Key.PointIndex;

    FKyaPath Candidate = *Path;
    const auto HasPointRecord = [PointIndex](const auto& Array, const TCHAR* Label, FString& OutError) -> bool
    {
        if (Array.IsValidIndex(PointIndex)) return true;
        OutError = FString::Printf(TEXT("This path has no %s record for point %d."), Label, PointIndex);
        return false;
    };

    bool bValid = true;
    if (Patch.Point.IsSet())
    {
        bValid = ValidateVector4(Patch.Point.GetValue(), TEXT("Path point"), Error);
        if (bValid) Candidate.Points[PointIndex] = Patch.Point.GetValue();
    }
    if (bValid && Patch.Rotation.IsSet())
    {
        bValid = HasPointRecord(Candidate.Rotations, TEXT("rotation"), Error) &&
            ValidateVector4(Patch.Rotation.GetValue(), TEXT("Path point rotation"), Error);
        if (bValid) Candidate.Rotations[PointIndex] = Patch.Rotation.GetValue();
    }
    if (bValid && Patch.ExtraVector.IsSet())
    {
        bValid = HasPointRecord(Candidate.ExtraVectors, TEXT("extra vector"), Error) &&
            ValidateVector4(Patch.ExtraVector.GetValue(), TEXT("Path point extra vector"), Error);
        if (bValid) Candidate.ExtraVectors[PointIndex] = Patch.ExtraVector.GetValue();
    }
    if (bValid && Patch.Delay.IsSet())
    {
        const float Delay = Patch.Delay.GetValue();
        bValid = HasPointRecord(Candidate.Delays, TEXT("delay"), Error) && FMath::IsFinite(Delay) && Delay >= 0.0f;
        if (!bValid && Error.IsEmpty()) Error = TEXT("Path point delay must be finite and non-negative.");
        if (bValid) Candidate.Delays[PointIndex] = Delay;
    }
    if (bValid && Patch.Flags.IsSet())
    {
        bValid = HasPointRecord(Candidate.PointFlags, TEXT("flags"), Error);
        if (bValid) Candidate.PointFlags[PointIndex] = Patch.Flags.GetValue();
    }
    if (bValid && Patch.ExtraValue.IsSet())
    {
        const float ExtraValue = Patch.ExtraValue.GetValue();
        bValid = HasPointRecord(Candidate.ExtraValues, TEXT("extra value"), Error) && FMath::IsFinite(ExtraValue);
        if (!bValid && Error.IsEmpty()) Error = TEXT("Path point extra value must be finite.");
        if (bValid) Candidate.ExtraValues[PointIndex] = ExtraValue;
    }
    if (bValid && Patch.ExtraWord.IsSet())
    {
        bValid = HasPointRecord(Candidate.ExtraWords, TEXT("extra word"), Error);
        if (bValid) Candidate.ExtraWords[PointIndex] = Patch.ExtraWord.GetValue();
    }
    if (!bValid)
    {
        if (OutError) *OutError = Error;
        return false;
    }
    if (FKyaPath::StaticStruct()->CompareScriptStruct(Path, &Candidate, PPF_None)) return true;
    KYA_SERVICE_TRANSACTION("KyaEditPathPoint");

    Bank->Modify();
    if (!ResolvePathPoint(Key, Path, &Error))
    {
        if (OutError) *OutError = Error;
        return false;
    }
    *Path = Candidate;
    Bank->MarkPackageDirty();
    if (OnBankEdited.IsBound()) OnBankEdited.Broadcast(Bank);
    if (Notify == EKyaEditNotify::Silent) PendingBanks.Add(Bank);
    else NotifyPathPointEdit(Bank, Key.PathIndex, Key.PointIndex);
    return true;
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

void FKyaLevelEditService::NotifyBankReloaded(UKyaLevelBank* Bank)
{
    if (!Bank) return;
    if (OnBankEdited.IsBound()) OnBankEdited.Broadcast(Bank);
    NotifyBankChanged(Bank, true);
}

void FKyaLevelEditService::NotifyActorRecordEdit(UKyaLevelBank* Bank, int32 ActorIndex, bool bDeferred)
{
    TArray<TWeakObjectPtr<AKyaLevelPreview>>* BankViews = Views.Find(Bank);
    if (!BankViews) return;
    if (BankViews->Num() > 0 && GEditor && GEditor->IsTransactionActive()) SuppressedTransactions.Add(Bank, BankViews->Num());
    const TArray<TWeakObjectPtr<AKyaLevelPreview>> Snapshot = *BankViews;
    for (const TWeakObjectPtr<AKyaLevelPreview>& View : Snapshot)
        if (View.IsValid()) View->HandleActorEdited(Bank, ActorIndex, bDeferred);
}

void FKyaLevelEditService::NotifyWaypointEdit(UKyaLevelBank* Bank, int32 WaypointIndex)
{
    TArray<TWeakObjectPtr<AKyaLevelPreview>>* BankViews = Views.Find(Bank);
    if (!BankViews) return;
    if (BankViews->Num() > 0 && GEditor && GEditor->IsTransactionActive()) SuppressedTransactions.Add(Bank, BankViews->Num());
    const TArray<TWeakObjectPtr<AKyaLevelPreview>> Snapshot = *BankViews;
    for (const TWeakObjectPtr<AKyaLevelPreview>& View : Snapshot)
        if (View.IsValid()) View->HandleWaypointEdited(Bank, WaypointIndex);
}

void FKyaLevelEditService::NotifyPathPointEdit(UKyaLevelBank* Bank, int32 PathIndex, int32 PointIndex)
{
    TArray<TWeakObjectPtr<AKyaLevelPreview>>* BankViews = Views.Find(Bank);
    if (!BankViews) return;
    if (BankViews->Num() > 0 && GEditor && GEditor->IsTransactionActive()) SuppressedTransactions.Add(Bank, BankViews->Num());
    const TArray<TWeakObjectPtr<AKyaLevelPreview>> Snapshot = *BankViews;
    for (const TWeakObjectPtr<AKyaLevelPreview>& View : Snapshot)
        if (View.IsValid()) View->HandlePathPointEdited(Bank, PathIndex, PointIndex);
}

void FKyaLevelEditService::NotifyLightEdit(UKyaLevelBank* Bank, bool bDeferred)
{
    TArray<TWeakObjectPtr<AKyaLevelPreview>>* BankViews = Views.Find(Bank);
    if (!BankViews) return;
    if (BankViews->Num() > 0 && GEditor && GEditor->IsTransactionActive()) SuppressedTransactions.Add(Bank, BankViews->Num());
    const TArray<TWeakObjectPtr<AKyaLevelPreview>> Snapshot = *BankViews;
    for (const TWeakObjectPtr<AKyaLevelPreview>& View : Snapshot)
        if (View.IsValid()) View->HandleLightEdited(Bank, bDeferred);
}

void FKyaLevelEditService::NotifySceneCameraEdit(UKyaLevelBank* Bank)
{
    TArray<TWeakObjectPtr<AKyaLevelPreview>>* BankViews = Views.Find(Bank);
    if (!BankViews) return;
    if (BankViews->Num() > 0 && GEditor && GEditor->IsTransactionActive()) SuppressedTransactions.Add(Bank, BankViews->Num());
    const TArray<TWeakObjectPtr<AKyaLevelPreview>> Snapshot = *BankViews;
    for (const TWeakObjectPtr<AKyaLevelPreview>& View : Snapshot)
        if (View.IsValid()) View->HandleSceneCameraEdited(Bank);
}

void FKyaLevelEditService::FlushPendingNotifications()
{
    bFlushScheduled = false;
    SuppressedTransactions.Reset();
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

bool FKyaLevelEditService::ConsumeTargetedTransaction(UKyaLevelBank* Bank)
{
    int32* Remaining = SuppressedTransactions.Find(Bank);
    if (!Remaining || *Remaining <= 0) return false;
    if (--(*Remaining) == 0) SuppressedTransactions.Remove(Bank);
    return true;
}

void FKyaLevelEditService::Reset()
{
    Views.Reset();
    PendingBanks.Reset();
    SuppressedTransactions.Reset();
    OnBankEdited.Clear();
}

FString FKyaLightRecordKey::ToString() const
{
    return FString::Printf(TEXT("%s light %d@%u"), Bank.IsValid() ? *Bank->GetName() : TEXT("<null>"),
        LightIndex, Generation);
}

bool FKyaLightEditPatch::IsEmpty() const
{
    return !LightType.IsSet() && !ShapeType.IsSet() && !AmbientColor.IsSet() && !Color.IsSet() &&
        !Multiplier.IsSet() && !Position.IsSet() && !Direction.IsSet() && !Falloff0.IsSet() &&
        !Falloff1.IsSet() && !Fov0.IsSet() && !Fov1.IsSet();
}

FString FKyaSceneCameraKey::ToString() const
{
    return FString::Printf(TEXT("%s camera@%u"), Bank.IsValid() ? *Bank->GetName() : TEXT("<null>"), Generation);
}

bool FKyaSceneCameraEditPatch::IsEmpty() const
{
    return !Position.IsSet() && !LookAt.IsSet() && !Up.IsSet() && !Fov.IsSet();
}

FKyaLightRecordKey FKyaLevelEditService::MakeLightKey(UKyaLevelBank* Bank, int32 LightIndex) const
{
    FKyaLightRecordKey Key;
    Key.Bank = Bank;
    Key.LightIndex = LightIndex;
    Key.Generation = Bank ? uint32(Bank->EditGeneration) : 0;
    return Key;
}

bool FKyaLevelEditService::ResolveLight(const FKyaLightRecordKey& Key, FKyaLight*& OutLight, FString* OutError) const
{
    OutLight = nullptr;
    UKyaLevelBank* Bank = Key.Bank.Get();
    if (!Bank)
    {
        if (OutError) *OutError = TEXT("The bank asset no longer exists.");
        return false;
    }
    if (Key.Generation != uint32(Bank->EditGeneration))
    {
        if (OutError) *OutError = FString::Printf(
            TEXT("Light identity is stale (bank generation %u, key %u); reselect after reimport."),
            uint32(Bank->EditGeneration), Key.Generation);
        return false;
    }
    if (!Bank->Lights.IsValidIndex(Key.LightIndex))
    {
        if (OutError) *OutError = FString::Printf(TEXT("Light index %d does not exist in %s."),
            Key.LightIndex, *Bank->GetName());
        return false;
    }
    OutLight = &Bank->Lights[Key.LightIndex];
    return true;
}

bool FKyaLevelEditService::ApplyLight(const FKyaLightRecordKey& Key, const FKyaLightEditPatch& Patch,
    EKyaEditNotify Notify, FString* OutError)
{
    FString Error;
    FKyaLight* Light = nullptr;
    if (!ResolveLight(Key, Light, &Error))
    {
        if (OutError) *OutError = Error;
        return false;
    }
    if (Patch.IsEmpty())
    {
        if (OutError) *OutError = TEXT("The light patch does not change any property.");
        return false;
    }
    UKyaLevelBank* Bank = Key.Bank.Get();
    FKyaLight Candidate = *Light;
    if (Patch.LightType.IsSet()) Candidate.LightType = Patch.LightType.GetValue();
    if (Patch.ShapeType.IsSet()) Candidate.ShapeType = Patch.ShapeType.GetValue();
    // The serialized record layout depends on type/shape (direction/FOV presence);
    // changing that layout requires rebuilding the archive, which the writer does not do.
    const auto HasDirection = [](int32 Type, int32 Shape) { return Type != 2 && (Shape == 1 || Shape == 2 || Shape == 4); };
    const auto HasFov = [](int32 Shape) { return Shape == 4; };
    if (HasDirection(Light->LightType, Light->ShapeType) != HasDirection(Candidate.LightType, Candidate.ShapeType) ||
        HasFov(Light->ShapeType) != HasFov(Candidate.ShapeType))
    {
        if (OutError) *OutError = TEXT("Changing this light's type/shape would change its serialized record layout "
            "(direction/FOV presence); rebuilding the light archive is not supported.");
        return false;
    }
    if (Patch.AmbientColor.IsSet()) Candidate.AmbientColor = Patch.AmbientColor.GetValue();
    if (Patch.Color.IsSet()) Candidate.Color = Patch.Color.GetValue();
    if (Patch.Multiplier.IsSet()) Candidate.Multiplier = Patch.Multiplier.GetValue();
    if (Patch.Position.IsSet()) Candidate.Position = Patch.Position.GetValue();
    if (Patch.Direction.IsSet()) Candidate.Direction = Patch.Direction.GetValue();
    if (Patch.Falloff0.IsSet()) Candidate.Falloff0 = Patch.Falloff0.GetValue();
    if (Patch.Falloff1.IsSet()) Candidate.Falloff1 = Patch.Falloff1.GetValue();
    if (Patch.Fov0.IsSet()) Candidate.Fov0 = Patch.Fov0.GetValue();
    if (Patch.Fov1.IsSet()) Candidate.Fov1 = Patch.Fov1.GetValue();
    if (!ValidateVector(Candidate.Position, TEXT("Light position"), Error) ||
        !ValidateVector(Candidate.Direction, TEXT("Light direction"), Error) ||
        !FMath::IsFinite(Candidate.Multiplier) || !FMath::IsFinite(Candidate.Falloff0) ||
        !FMath::IsFinite(Candidate.Falloff1) || !FMath::IsFinite(Candidate.Fov0) ||
        !FMath::IsFinite(Candidate.Fov1))
    {
        if (OutError) *OutError = Error.IsEmpty() ? TEXT("Light values must be finite.") : Error;
        return false;
    }
    if (FKyaLight::StaticStruct()->CompareScriptStruct(Light, &Candidate, PPF_None)) return true;
    KYA_SERVICE_TRANSACTION("KyaEditLight");
    Bank->Modify();
    if (!ResolveLight(Key, Light, &Error))
    {
        if (OutError) *OutError = Error;
        return false;
    }
    *Light = Candidate;
    Bank->MarkPackageDirty();
    if (OnBankEdited.IsBound()) OnBankEdited.Broadcast(Bank);
    if (Notify == EKyaEditNotify::Silent) PendingBanks.Add(Bank);
    else NotifyLightEdit(Bank, Notify == EKyaEditNotify::Deferred);
    return true;
}

FKyaSceneCameraKey FKyaLevelEditService::MakeSceneCameraKey(UKyaLevelBank* Bank) const
{
    FKyaSceneCameraKey Key;
    Key.Bank = Bank;
    Key.Generation = Bank ? uint32(Bank->EditGeneration) : 0;
    return Key;
}

bool FKyaLevelEditService::ApplySceneCamera(const FKyaSceneCameraKey& Key, const FKyaSceneCameraEditPatch& Patch,
    EKyaEditNotify Notify, FString* OutError)
{
    UKyaLevelBank* Bank = Key.Bank.Get();
    if (!Bank)
    {
        if (OutError) *OutError = TEXT("The bank asset no longer exists.");
        return false;
    }
    if (Key.Generation != uint32(Bank->EditGeneration))
    {
        if (OutError) *OutError = FString::Printf(
            TEXT("Camera identity is stale (bank generation %u, key %u); reselect after reimport."),
            uint32(Bank->EditGeneration), Key.Generation);
        return false;
    }
    if (Patch.IsEmpty())
    {
        if (OutError) *OutError = TEXT("The camera patch does not change any property.");
        return false;
    }
    FKyaSceneEnvironment Candidate = Bank->SceneEnvironment;
    if (Patch.Position.IsSet()) Candidate.CameraPosition = Patch.Position.GetValue();
    if (Patch.LookAt.IsSet()) Candidate.CameraLookAt = Patch.LookAt.GetValue();
    if (Patch.Up.IsSet()) Candidate.CameraUp = Patch.Up.GetValue();
    if (Patch.Fov.IsSet()) Candidate.CameraFov = Patch.Fov.GetValue();
    FString Error;
    if (!ValidateVector(Candidate.CameraPosition, TEXT("Camera position"), Error) ||
        !ValidateVector(Candidate.CameraLookAt, TEXT("Camera look-at"), Error) ||
        !ValidateVector(Candidate.CameraUp, TEXT("Camera up"), Error) ||
        !FMath::IsFinite(Candidate.CameraFov) || Candidate.CameraFov < 0.0f || Candidate.CameraFov > 180.0f)
    {
        if (OutError) *OutError = Error.IsEmpty()
            ? TEXT("Camera FOV must be finite and between 0 and 180 degrees.") : Error;
        return false;
    }
    // An identical camera is not an edit.
    if (Bank->SceneEnvironment.CameraPosition.Equals(Candidate.CameraPosition) &&
        Bank->SceneEnvironment.CameraLookAt.Equals(Candidate.CameraLookAt) &&
        Bank->SceneEnvironment.CameraUp.Equals(Candidate.CameraUp) &&
        Bank->SceneEnvironment.CameraFov == Candidate.CameraFov)
        return true;
    KYA_SERVICE_TRANSACTION("KyaEditSceneCamera");
    Bank->Modify();
    Bank->SceneEnvironment.CameraPosition = Candidate.CameraPosition;
    Bank->SceneEnvironment.CameraLookAt = Candidate.CameraLookAt;
    Bank->SceneEnvironment.CameraUp = Candidate.CameraUp;
    Bank->SceneEnvironment.CameraFov = Candidate.CameraFov;
    Bank->MarkPackageDirty();
    if (OnBankEdited.IsBound()) OnBankEdited.Broadcast(Bank);
    if (Notify == EKyaEditNotify::Silent) PendingBanks.Add(Bank);
    else NotifySceneCameraEdit(Bank);
    return true;
}

bool FKyaLevelEditService::ApplySceneSettings(UKyaLevelBank* Bank, const FKyaSceneEnvironment& Environment,
    const FKyaSceneConfiguration& Configuration, EKyaEditNotify Notify, FString* OutError)
{
    if (!Bank)
    {
        if (OutError) *OutError = TEXT("No bank was supplied for the level settings.");
        return false;
    }
    FString Error;
    if (!ValidateVector(Environment.CameraPosition, TEXT("Camera position"), Error) ||
        !ValidateVector(Environment.CameraLookAt, TEXT("Camera look-at"), Error) ||
        !ValidateVector(Environment.CameraUp, TEXT("Camera up"), Error) ||
        !ValidateVector(Environment.CameraTargetOffset, TEXT("Camera target offset"), Error) ||
        !ValidateVector(Environment.CameraPitchAngles, TEXT("Camera pitch angles"), Error) ||
        !ValidateVector(Environment.CameraOtherAngles, TEXT("Camera other angles"), Error) ||
        !ValidateVector(Environment.CameraDistances, TEXT("Camera distances"), Error) ||
        !FMath::IsFinite(Environment.ClipDistance) || !FMath::IsFinite(Environment.FogParameter) ||
        !FMath::IsFinite(Environment.CameraFov) || Environment.CameraFov < 0.0f || Environment.CameraFov > 180.0f ||
        !FMath::IsFinite(Environment.CameraConfigAngle) || !FMath::IsFinite(Environment.Field118))
    {
        if (OutError) *OutError = Error.IsEmpty()
            ? TEXT("Level camera, fog and environment values must be finite (FOV 0..180).") : Error;
        return false;
    }
    const auto RequireU32 = [&Error](int64 Value, const TCHAR* Label)
    {
        if (Value >= 0 && Value <= MAX_uint32) return true;
        Error = FString::Printf(TEXT("%s must fit the serialized unsigned 32-bit field."), Label);
        return false;
    };
    if (!RequireU32(Environment.BackgroundFlags, TEXT("Background flags")) ||
        !RequireU32(Environment.FogFlags, TEXT("Fog flags")) ||
        !RequireU32(Environment.CameraFlags, TEXT("Camera flags")) ||
        !RequireU32(Environment.CameraConfigFlags, TEXT("Camera config flags")) ||
        !RequireU32(Environment.Field11C, TEXT("Scene field 0x11C")) ||
        !RequireU32(Environment.Count120, TEXT("Scene count 0x120")) ||
        !RequireU32(Environment.MapField08, TEXT("Map field 0x08")))
    {
        if (OutError) *OutError = Error;
        return false;
    }
    // Placement records are edited by the actor/waypoint/path services; the settings
    // surface must never overwrite them with a stale snapshot.
    FKyaSceneEnvironment NewEnvironment = Environment;
    NewEnvironment.Actors = Bank->SceneEnvironment.Actors;
    NewEnvironment.Waypoints = Bank->SceneEnvironment.Waypoints;
    NewEnvironment.Paths = Bank->SceneEnvironment.Paths;
    // Unchanged settings are not an edit.
    if (FKyaSceneEnvironment::StaticStruct()->CompareScriptStruct(
            &Bank->SceneEnvironment, &NewEnvironment, PPF_None) &&
        FKyaSceneConfiguration::StaticStruct()->CompareScriptStruct(
            &Bank->SceneConfiguration, &Configuration, PPF_None))
        return true;
    KYA_SERVICE_TRANSACTION("KyaEditSceneSettings");
    Bank->Modify();
    Bank->SceneEnvironment = NewEnvironment;
    Bank->SceneConfiguration = Configuration;
    Bank->MarkPackageDirty();
    if (OnBankEdited.IsBound()) OnBankEdited.Broadcast(Bank);
    // Settings can affect background/camera rendering, so refresh through the full
    // preview path (this is not on the transform hot path).
    if (Notify == EKyaEditNotify::Silent) PendingBanks.Add(Bank);
    else NotifyBankChanged(Bank, Notify == EKyaEditNotify::Notify);
    return true;
}

#endif



