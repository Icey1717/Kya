#include "KyaActorEditService.h"

#if WITH_EDITOR

#include "KyaLevelPreview.h"
#include "KyaExportedActorFields.h"
#include "Containers/Ticker.h"

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

void NotifyAfterEdit(FKyaLevelEditService& Service, UKyaLevelBank* Bank, EKyaEditNotify Notify)
{
    if (Service.OnBankEdited.IsBound()) Service.OnBankEdited.Broadcast(Bank);
    if (Notify == EKyaEditNotify::Notify) Service.NotifyBankChanged(Bank, true);
    else if (Notify == EKyaEditNotify::Deferred) Service.NotifyBankChangedDeferred(Bank);
    else Service.NotifyBankChanged(Bank, false);
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

FText DescribeKyaFieldCapability(EKyaFieldCapability Kind){
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
    NotifyAfterEdit(*this, Bank, Notify);
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

    Bank->Modify();
    if (!ResolveWaypoint(Key, Waypoint, &Error))
    {
        if (OutError) *OutError = Error;
        return false;
    }
    *Waypoint = Candidate;
    Bank->MarkPackageDirty();
    NotifyAfterEdit(*this, Bank, Notify);
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

    Bank->Modify();
    if (!ResolvePathPoint(Key, Path, &Error))
    {
        if (OutError) *OutError = Error;
        return false;
    }
    *Path = Candidate;
    Bank->MarkPackageDirty();
    NotifyAfterEdit(*this, Bank, Notify);
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
