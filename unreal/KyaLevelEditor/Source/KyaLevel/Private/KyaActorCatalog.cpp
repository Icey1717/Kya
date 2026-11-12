#include "KyaActorCatalog.h"

#include "KyaActorEditService.h"
#include "KyaLevelBank.h"
#include "UObject/UnrealType.h"

namespace
{
struct FClassRow
{
    int64 ClassId;
    const TCHAR* Name;
    const TCHAR* Notes;
};

// Recovered from the class-specific parsing branches in KyaLevelBankFactory.cpp
// (each branch sets the matching bHas*Data flag).
const FClassRow ClassRows[] = {
    {0x0d, TEXT("Moving platform"), TEXT("CActorMovingPlatform: carried actors, zones, camera/notification records")},
    {0x39, TEXT("Basic box"), TEXT("life, vibration, switch and add-on generators")},
    {0x2e, TEXT("FX"), TEXT("two Create-stage floats and single/path behaviour payloads")},
    {0x21, TEXT("Araignos"), TEXT("movable weight, path follow, sound references, add-on")},
    {0x1c, TEXT("Projectile"), TEXT("autonomous movement, tuning sub-object, pattern parts")},
    {0x12, TEXT("Wind"), TEXT("tuning, zones, actor/material references, sector hierarchy")},
    {0x2c, TEXT("Trap"), TEXT("movement/path settings, escape state, switch stream")},
    {0x20, TEXT("Weapon"), TEXT("class values and burst/bullets/pistol/sniper/RPG payloads")},
    {0x11, TEXT("Switch"), TEXT("target list and camera event")},
    {0x0e, TEXT("Bonus"), TEXT("sound/material values and turn/flock/add-on behaviours")},
    {0x07, TEXT("Micken"), TEXT("autonomous values, waypoint/actor references, eat behaviour")},
    {0x09, TEXT("Wolfen"), TEXT("vision/detection, fighter block, blow/grab/reaction/combo databases")},
    {0x16, TEXT("Fruit"), TEXT("autonomous/switch data and squashed/bonus/inventory behaviours")},
    {0x13, TEXT("Commander"), TEXT("squad, team, sequence and camera-figure data")},
    {0x0a, TEXT("Nativ"), TEXT("autonomous/movement fields and take-and-put trajectories")},
    {0x29, TEXT("Amortos"), TEXT("actor lists, tuning floats, add-on, switch/camera records")},
    {0x24, TEXT("Teleporter"), TEXT("destinations, switch/camera records, zones, waypoints")},
    {0x1b, TEXT("DCA"), TEXT("aim/shot tuning and controlled/trajectory behaviours")},
    {0x35, TEXT("Box"), TEXT("autonomous tuning, actor list, switch and add-on")},
    {0x32, TEXT("Money"), TEXT("weight, value, sound and material")},
    {0x3d, TEXT("Fog manager"), TEXT("hierarchy list and streamed fog zones")},
    {0x3e, TEXT("Shoot"), TEXT("static-mesh/aim tuning and fire/wave behaviours")},
    {0x14, TEXT("Ambre"), TEXT("mesh/material and switch data")},
    {0x4c, TEXT("Stiller"), TEXT("vision, path, life, switches and add-on")},
    {0x51, TEXT("Rune"), TEXT("serialized tuning, references and switch data")},
    {0x52, TEXT("Bonus fountain"), TEXT("serialized tuning, references and switch data")},
    {0x0f, TEXT("Rope"), TEXT("serialized tuning, references and switch data")},
    {0x10, TEXT("Gravity aware"), TEXT("autonomous/behaviour data")},
    {0x3c, TEXT("Nativ command"), TEXT("add-on lists")},
    {0x19, TEXT("Companion"), TEXT("autonomous values and ids")},
    {0x17, TEXT("Bridge"), TEXT("actor/waypoint/config data and behaviours")},
    {0x38, TEXT("Aton"), TEXT("autonomous values, effect/mesh/material ids, path list, add-on")},
    {0x06, TEXT("Kim / hero"), TEXT("shared CActorFighter prefix, bone keys, cameras and FX handles")},
    {0x05, TEXT("Boomy"), TEXT("tail/particle fields, embedded camera, vision/damage table")},
    {0x1e, TEXT("Checkpoint manager"), TEXT("checkpoint records")},
};

struct FBehaviourRow
{
    int64 ClassId;
    int32 BehaviourId;
    const TCHAR* Name;
};

// Recovered from the class/id branches in KyaLevelBankFactory.cpp. Ids without an
// entry are reported as unknown rather than invented.
const FBehaviourRow BehaviourRows[] = {
    {0x11, 2, TEXT("Switch lever")},
    {0x11, 3, TEXT("Switch magic")},
    {0x11, 5, TEXT("Switch multi-condition")},
    {0x11, 6, TEXT("Switch target")},
    {0x1c, 3, TEXT("Projectile stand")},
    {0x1c, 4, TEXT("Projectile straight")},
    {0x1c, 5, TEXT("Projectile portable")},
    {0x1c, 11, TEXT("Projectile excuse")},
    {0x17, 2, TEXT("Bridge basic")},
    {0x17, 3, TEXT("Bridge wind")},
    {0x3e, 3, TEXT("Shoot fire")},
    {0x3e, 4, TEXT("Shoot wave")},
    {0x10, 3, TEXT("Gravity fall")},
    {0x10, 4, TEXT("Gravity bell")},
    {0x06, 9, TEXT("Hero unknown behaviour 9")},
    {0x06, 10, TEXT("Hero unknown behaviour 10")},
    {0x06, 11, TEXT("Hero unknown behaviour 11")},
};

struct FReferenceRow
{
    const TCHAR* Name;
    EKyaReferenceKind Kind;
    int64 ClassId;
};

// Top-level scalar reference fields on FKyaActorPlacement. Nested reference arrays
// and records remain raw values for now. ClassId -1 marks fields meaningful on every
// class; class-specific rows are only validated and offered for that class.
const FReferenceRow ReferenceRows[] = {
    {TEXT("SectorId"), EKyaReferenceKind::Sector, -1},
    {TEXT("MeshIndex"), EKyaReferenceKind::Mesh, -1},
    {TEXT("TextureIndex"), EKyaReferenceKind::Texture, -1},
    {TEXT("DefaultBehaviourId"), EKyaReferenceKind::Behaviour, -1},
    {TEXT("MickenActorIndex"), EKyaReferenceKind::Actor, 0x07},
    {TEXT("MickenWaypointIndex"), EKyaReferenceKind::Waypoint, 0x07},
    {TEXT("TrapActorRef"), EKyaReferenceKind::Actor, 0x2c},
    {TEXT("CommanderActorRef"), EKyaReferenceKind::Actor, 0x13},
    {TEXT("AmortosActorRef"), EKyaReferenceKind::Actor, 0x29},
    {TEXT("RopeActorRefA"), EKyaReferenceKind::Actor, 0x0f},
    {TEXT("RopeActorRefB"), EKyaReferenceKind::Actor, 0x0f},
    {TEXT("RopePathIndex"), EKyaReferenceKind::Path, 0x0f},
    {TEXT("FighterWeaponActorIndex"), EKyaReferenceKind::Actor, 0x06},
    {TEXT("BridgeWaypointA"), EKyaReferenceKind::Waypoint, 0x17},
    {TEXT("BridgeWaypointB"), EKyaReferenceKind::Waypoint, 0x17},
    {TEXT("WindWaypointRef"), EKyaReferenceKind::Waypoint, 0x12},
    {TEXT("WindWaypointRef1CC"), EKyaReferenceKind::Waypoint, 0x12},
    {TEXT("WindZoneRef1C4"), EKyaReferenceKind::Zone, 0x12},
    {TEXT("StillerPathIndex"), EKyaReferenceKind::Path, 0x4c},
};

int32 CountEntries(const UKyaLevelBank& Bank, int32 Type, int32 Subtype)
{
    int32 Count = 0;
    for (const FKyaBankEntry& Entry : Bank.Entries)
        if (Entry.Type == Type && Entry.Subtype == Subtype) ++Count;
    return Count;
}
}

FKyaActorClassInfo GetKyaActorClassInfo(int64 ClassId)
{
    FKyaActorClassInfo Info;
    Info.ClassId = ClassId;
    for (const FClassRow& Row : ClassRows)
    {
        if (Row.ClassId != ClassId) continue;
        Info.Name = Row.Name;
        Info.Notes = Row.Notes;
        Info.bKnown = true;
        return Info;
    }
    Info.Name = FString::Printf(TEXT("Unknown class %lld"), ClassId);
    Info.Notes = TEXT("No recovered class name; fields are shown with their serialized names.");
    return Info;
}

FString GetKyaActorClassLabel(int64 ClassId)
{
    const FKyaActorClassInfo Info = GetKyaActorClassInfo(ClassId);
    return Info.bKnown
        ? FString::Printf(TEXT("%s (%lld)"), *Info.Name, ClassId)
        : FString::Printf(TEXT("%s (unknown)"), *Info.Name);
}

FKyaBehaviourInfo GetKyaBehaviourInfo(int64 ClassId, int32 BehaviourId)
{
    FKyaBehaviourInfo Info;
    Info.Id = BehaviourId;
    for (const FBehaviourRow& Row : BehaviourRows)
    {
        if (Row.ClassId != ClassId || Row.BehaviourId != BehaviourId) continue;
        Info.Name = Row.Name;
        Info.bKnown = true;
        return Info;
    }
    return Info;
}

FString GetKyaBehaviourLabel(int64 ClassId, int32 BehaviourId)
{
    const FKyaBehaviourInfo Info = GetKyaBehaviourInfo(ClassId, BehaviourId);
    return Info.bKnown
        ? FString::Printf(TEXT("%s (%d)"), *Info.Name, BehaviourId)
        : FString::Printf(TEXT("Behaviour %d (unknown)"), BehaviourId);
}

TArray<int32> GetKyaModelOrdinals(const UKyaLevelBank& Bank)
{
    TArray<int32> Ordinals;
    const int32 Count = CountEntries(Bank, 4, 1);
    Ordinals.Reserve(Count);
    for (int32 Index = 0; Index < Count; ++Index) Ordinals.Add(Index);
    return Ordinals;
}

TArray<int32> GetKyaTextureBankOrdinals(const UKyaLevelBank& Bank)
{
    TArray<int32> Ordinals;
    const int32 Count = CountEntries(Bank, 5, 1);
    Ordinals.Reserve(Count);
    for (int32 Index = 0; Index < Count; ++Index) Ordinals.Add(Index);
    return Ordinals;
}

FKyaBankReference ResolveKyaModelReference(const UKyaLevelBank& Bank, int32 MeshIndex)
{
    FKyaBankReference Result;
    Result.Ordinal = MeshIndex;
    Result.EntryCount = CountEntries(Bank, 4, 1);
    if (MeshIndex == -1)
    {
        Result.bResolved = true;
        Result.Label = TEXT("None");
        return Result;
    }
    Result.bResolved = MeshIndex >= 0 && MeshIndex < Result.EntryCount;
    Result.Label = Result.bResolved
        ? FString::Printf(TEXT("Mesh #%d"), MeshIndex)
        : FString::Printf(TEXT("Unresolved mesh #%d"), MeshIndex);
    if (!Result.bResolved)
        Result.Warning = FString::Printf(
            TEXT("Mesh index %d has no type-4 mesh entry in this bank (%d available)."), MeshIndex, Result.EntryCount);
    return Result;
}

FKyaBankReference ResolveKyaTextureReference(const UKyaLevelBank& Bank, int32 TextureIndex)
{
    FKyaBankReference Result;
    Result.Ordinal = TextureIndex;
    Result.EntryCount = CountEntries(Bank, 5, 1);
    if (TextureIndex == -1)
    {
        Result.bResolved = true;
        Result.Label = TEXT("Alternate texture bank");
        return Result;
    }
    Result.bResolved = TextureIndex >= 0 && TextureIndex < Result.EntryCount;
    Result.Label = Result.bResolved
        ? FString::Printf(TEXT("Texture bank #%d"), TextureIndex)
        : FString::Printf(TEXT("Unresolved texture bank #%d"), TextureIndex);
    if (!Result.bResolved)
        Result.Warning = FString::Printf(
            TEXT("Texture index %d has no type-5 texture entry in this bank (%d available)."), TextureIndex,
            Result.EntryCount);
    return Result;
}

FString GetKyaSectorLabel(const TMap<int32, FString>& LoadedSectors, int32 SectorId)
{
    if (SectorId < 0) return TEXT("Global (no sector)");
    if (const FString* Name = LoadedSectors.Find(SectorId)) return FString::Printf(TEXT("Sector %d (%s)"), SectorId, **Name);
    return FString::Printf(TEXT("Sector %d (not loaded)"), SectorId);
}

TArray<FKyaBehaviourOption> GetKyaBehaviourOptions(const FKyaActorPlacement& Placement)
{
    TArray<FKyaBehaviourOption> Options;
    for (int32 Index = 0; Index < Placement.Behaviours.Num(); ++Index)
    {
        FKyaBehaviourOption& Option = Options.AddDefaulted_GetRef();
        Option.Id = Index;
        Option.Label = FString::Printf(TEXT("Behaviour %d: %s"), Index,
            *GetKyaBehaviourLabel(Placement.ClassId, Placement.Behaviours[Index].Id));
    }
    return Options;
}

bool FindKyaReferenceField(FName PropertyName, EKyaReferenceKind& OutKind)
{
    for (const FReferenceRow& Row : ReferenceRows)
    {
        if (PropertyName != FName(Row.Name)) continue;
        OutKind = Row.Kind;
        return true;
    }
    return false;
}

FText DescribeKyaReferenceKind(EKyaReferenceKind Kind)
{
    switch (Kind)
    {
    case EKyaReferenceKind::Actor: return NSLOCTEXT("KyaLevel", "RefActor", "Actor index");
    case EKyaReferenceKind::Waypoint: return NSLOCTEXT("KyaLevel", "RefWaypoint", "Waypoint index");
    case EKyaReferenceKind::Path: return NSLOCTEXT("KyaLevel", "RefPath", "Path index");
    case EKyaReferenceKind::Zone: return NSLOCTEXT("KyaLevel", "RefZone", "Zone reference");
    case EKyaReferenceKind::Sector: return NSLOCTEXT("KyaLevel", "RefSector", "Sector id");
    case EKyaReferenceKind::Mesh: return NSLOCTEXT("KyaLevel", "RefMesh", "Mesh ordinal");
    case EKyaReferenceKind::Texture: return NSLOCTEXT("KyaLevel", "RefTexture", "Texture bank ordinal");
    default: return NSLOCTEXT("KyaLevel", "RefBehaviour", "Behaviour index");
    }
}

bool SetKyaNumericField(FKyaActorPlacement& Placement, FName PropertyName, int64 Value)
{
    FProperty* Property = FKyaActorPlacement::StaticStruct()->FindPropertyByName(PropertyName);
    if (FIntProperty* Int = CastField<FIntProperty>(Property))
    {
        Int->SetPropertyValue_InContainer(&Placement, int32(Value));
        return true;
    }
    if (FInt64Property* Int64 = CastField<FInt64Property>(Property))
    {
        Int64->SetPropertyValue_InContainer(&Placement, Value);
        return true;
    }
    return false;
}

int64 GetKyaNumericField(const FKyaActorPlacement& Placement, FName PropertyName)
{
    FProperty* Property = FKyaActorPlacement::StaticStruct()->FindPropertyByName(PropertyName);
    if (const FIntProperty* Int = CastField<FIntProperty>(Property))
        return Int->GetPropertyValue_InContainer(&Placement);
    if (const FInt64Property* Int64 = CastField<FInt64Property>(Property))
        return Int64->GetPropertyValue_InContainer(&Placement);
    return 0;
}

bool GetKyaVectorField(const FKyaActorPlacement& Placement, FName PropertyName, FVector& Out)
{
    const FStructProperty* Property = CastField<FStructProperty>(
        FKyaActorPlacement::StaticStruct()->FindPropertyByName(PropertyName));
    if (!Property || Property->Struct != TBaseStructure<FVector>::Get()) return false;
    Out = *Property->ContainerPtrToValuePtr<FVector>(&Placement);
    return true;
}

bool SetKyaVectorField(FKyaActorPlacement& Placement, FName PropertyName, const FVector& Value)
{
    const FStructProperty* Property = CastField<FStructProperty>(
        FKyaActorPlacement::StaticStruct()->FindPropertyByName(PropertyName));
    if (!Property || Property->Struct != TBaseStructure<FVector>::Get()) return false;
    *Property->ContainerPtrToValuePtr<FVector>(&Placement) = Value;
    return true;
}

bool GetKyaFloatField(const FKyaActorPlacement& Placement, FName PropertyName, float& Out)
{
    FProperty* Property = FKyaActorPlacement::StaticStruct()->FindPropertyByName(PropertyName);
    if (const FFloatProperty* Float = CastField<FFloatProperty>(Property))
    {
        Out = Float->GetPropertyValue_InContainer(&Placement);
        return true;
    }
    return false;
}

bool SetKyaFloatField(FKyaActorPlacement& Placement, FName PropertyName, float Value)
{
    FProperty* Property = FKyaActorPlacement::StaticStruct()->FindPropertyByName(PropertyName);
    if (const FFloatProperty* Float = CastField<FFloatProperty>(Property))
    {
        Float->SetPropertyValue_InContainer(&Placement, Value);
        return true;
    }
    return false;
}

TArray<FKyaReferenceField> GetKyaReferenceFields()
{
    TArray<FKyaReferenceField> Fields;
    Fields.Reserve(UE_ARRAY_COUNT(ReferenceRows));
    for (const FReferenceRow& Row : ReferenceRows)
        Fields.Add({FName(Row.Name), Row.Kind, Row.ClassId});
    return Fields;
}

bool HasKyaImportedBaseline(const UKyaLevelBank& Bank)
{
    return Bank.bHasImportedBaseline && Bank.ImportedActors.Num() == Bank.SceneEnvironment.Actors.Num();
}

void CaptureKyaImportedBaseline(UKyaLevelBank& Bank)
{
    Bank.ImportedActors = Bank.SceneEnvironment.Actors;
    Bank.ImportedWaypoints = Bank.SceneEnvironment.Waypoints;
    Bank.ImportedPaths = Bank.SceneEnvironment.Paths;
    Bank.bHasImportedBaseline = true;
}

bool GetKyaImportedActor(const UKyaLevelBank& Bank, int32 Index, FKyaActorPlacement& Out)
{
    if (!HasKyaImportedBaseline(Bank) || !Bank.ImportedActors.IsValidIndex(Index)) return false;
    Out = Bank.ImportedActors[Index];
    return true;
}

bool KyaActorDiffersFromImported(const UKyaLevelBank& Bank, int32 Index)
{
    FKyaActorPlacement Imported;
    if (!GetKyaImportedActor(Bank, Index, Imported)) return false;
    if (!Bank.SceneEnvironment.Actors.IsValidIndex(Index)) return false;
    UScriptStruct* Struct = FKyaActorPlacement::StaticStruct();
    return !Struct->CompareScriptStruct(&Imported, &Bank.SceneEnvironment.Actors[Index], PPF_None);
}

bool BuildKyaActorResetPatch(const UKyaLevelBank& Bank, int32 Index, FKyaActorEditPatch& OutPatch)
{
    FKyaActorPlacement Imported;
    if (!GetKyaImportedActor(Bank, Index, Imported)) return false;
    OutPatch = FKyaActorEditPatch();
    OutPatch.Custom = [Imported](FKyaActorPlacement& Record, FString& OutError) -> bool
    {
        Record = Imported;
        return true;
    };
    return true;
}

bool BuildKyaActorFieldResetPatch(const UKyaLevelBank& Bank, int32 Index, FName PropertyName,
    FKyaActorEditPatch& OutPatch)
{
    FKyaActorPlacement Imported;
    if (!GetKyaImportedActor(Bank, Index, Imported)) return false;
    return KyaMakePlacementPropertyPatch(Imported, PropertyName, OutPatch);
}

namespace
{
int32 Align4(int32 Value) { return (Value + 3) & ~3; }

bool LayoutCompatible(FProperty* Property, const void* SourceValue, const void* DestValue, FString& OutError)
{
    if (const FStrProperty* String = CastField<FStrProperty>(Property))
    {
        const FString& SourceText = String->GetPropertyValue(SourceValue);
        const FString& DestText = String->GetPropertyValue(DestValue);
        if (Align4(SourceText.Len() + 1) != Align4(DestText.Len() + 1))
        {
            OutError = FString::Printf(
                TEXT("%s changes the serialized string slot (%d -> %d aligned bytes)."),
                *Property->GetDisplayNameText().ToString(), Align4(DestText.Len() + 1),
                Align4(SourceText.Len() + 1));
            return false;
        }
        return true;
    }
    if (const FArrayProperty* Array = CastField<FArrayProperty>(Property))
    {
        FScriptArrayHelper SourceHelper(Array, SourceValue);
        FScriptArrayHelper DestHelper(Array, DestValue);
        if (SourceHelper.Num() != DestHelper.Num())
        {
            OutError = FString::Printf(TEXT("%s has %d source element(s) but the destination has %d; "
                "array counts are fixed by the writer."), *Property->GetDisplayNameText().ToString(),
                SourceHelper.Num(), DestHelper.Num());
            return false;
        }
        for (int32 Index = 0; Index < SourceHelper.Num(); ++Index)
            if (!LayoutCompatible(Array->Inner, SourceHelper.GetRawPtr(Index), DestHelper.GetRawPtr(Index), OutError))
                return false;
        return true;
    }
    if (const FStructProperty* Struct = CastField<FStructProperty>(Property))
    {
        for (TFieldIterator<FProperty> It(Struct->Struct); It; ++It)
        {
            const FString FieldName = It->GetName();
            // Nested presence flags and the behaviour discriminator change the
            // serialized layout/payload interpretation; they must match exactly.
            if (FieldName.StartsWith(TEXT("bHas")))
            {
                if (const FBoolProperty* Bool = CastField<FBoolProperty>(*It))
                {
                    if (Bool->GetPropertyValue_InContainer(SourceValue) !=
                        Bool->GetPropertyValue_InContainer(DestValue))
                    {
                        OutError = FString::Printf(TEXT("%s: nested presence flag %s differs between source and "
                            "destination."), *Property->GetDisplayNameText().ToString(), *FieldName);
                        return false;
                    }
                }
            }
            if (FieldName == TEXT("Id"))
            {
                const FIntProperty* Int = CastField<FIntProperty>(*It);
                if (Int && Int->GetPropertyValue_InContainer(SourceValue) !=
                    Int->GetPropertyValue_InContainer(DestValue))
                {
                    OutError = FString::Printf(TEXT("%s: record ids differ between source and destination, so the "
                        "payload layouts are incompatible."), *Property->GetDisplayNameText().ToString());
                    return false;
                }
            }
            if (!LayoutCompatible(*It, It->ContainerPtrToValuePtr<void>(SourceValue),
                It->ContainerPtrToValuePtr<void>(DestValue), OutError))
                return false;
        }
    }
    return true;
}

bool PresenceSignatureMatches(const FKyaActorPlacement& Source, const FKyaActorPlacement& Dest)
{
    for (TFieldIterator<FBoolProperty> It(FKyaActorPlacement::StaticStruct()); It; ++It)
    {
        if (!It->GetName().StartsWith(TEXT("bHas"))) continue;
        if (It->GetPropertyValue_InContainer(&Source) != It->GetPropertyValue_InContainer(&Dest)) return false;
    }
    return true;
}
}

bool CanCopyKyaActorField(const FKyaActorPlacement& Source, const FKyaActorPlacement& Dest, FName PropertyName,
    FString& OutError)
{
    FProperty* Property = FKyaActorPlacement::StaticStruct()->FindPropertyByName(PropertyName);
    if (!Property)
    {
        OutError = TEXT("The property is not part of the actor placement schema.");
        return false;
    }
    const FKyaFieldCapability Capability = GetKyaActorFieldCapability(PropertyName);
    if (Capability.Kind != EKyaFieldCapability::EditableExportable)
    {
        OutError = Capability.Reason;
        return false;
    }
    // Class-specific fields are those gated by a presence flag in the schema. They
    // may only be copied between actors of the same class with the same optional
    // records; generic settings and generic arrays only need a layout match.
    const FString* Condition = Property->FindMetaData(TEXT("EditCondition"));
    const bool bBehaviours = PropertyName == GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, Behaviours);
    const bool bClassData = PropertyName == GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, ClassData);
    const bool bClassSpecific = (Condition && Condition->Contains(TEXT("bHas"))) || bBehaviours || bClassData;
    if (bClassSpecific)
    {
        if (Source.ClassId != Dest.ClassId)
        {
            OutError = FString::Printf(TEXT("%s is a class-specific record; source class %lld differs from "
                "destination class %lld."), *Property->GetDisplayNameText().ToString(), Source.ClassId,
                Dest.ClassId);
            return false;
        }
        if (!PresenceSignatureMatches(Source, Dest))
        {
            OutError = FString::Printf(TEXT("%s: the actors have different optional records, so their serialized "
                "layout is incompatible."), *Property->GetDisplayNameText().ToString());
            return false;
        }
    }
    return LayoutCompatible(Property, Property->ContainerPtrToValuePtr<void>(&Source),
        Property->ContainerPtrToValuePtr<void>(&Dest), OutError);
}


TArray<FKyaActorReferenceIssue> ValidateKyaActorReferences(const UKyaLevelBank& Bank,
    const FKyaActorPlacement& Placement){
    TArray<FKyaActorReferenceIssue> Issues;
    const FKyaBankReference Mesh = ResolveKyaModelReference(Bank, Placement.MeshIndex);
    if (!Mesh.bResolved)
        Issues.Add({TEXT("MeshIndex"), Mesh.Warning, true});
    const FKyaBankReference Texture = ResolveKyaTextureReference(Bank, Placement.TextureIndex);
    if (!Texture.bResolved)
        Issues.Add({TEXT("TextureIndex"), Texture.Warning, true});
    if (Placement.SectorId < -1)
        Issues.Add({TEXT("SectorId"), TEXT("Sector id must be -1 (global) or a non-negative sector id."), true});
    if (Placement.DefaultBehaviourId < 0 || Placement.DefaultBehaviourId >= Placement.Behaviours.Num())
        Issues.Add({TEXT("DefaultBehaviourId"), FString::Printf(
            TEXT("Default behaviour %d does not exist in this actor's %d behaviour record(s)."),
            Placement.DefaultBehaviourId, Placement.Behaviours.Num()), false});

    // Actor/waypoint/path index references must resolve inside this bank. Class-
    // specific fields are only checked for the class they belong to.
    for (const FKyaReferenceField& Field : GetKyaReferenceFields())
    {
        if (Field.ClassId != -1 && Field.ClassId != Placement.ClassId) continue;
        int32 Limit = 0;
        const TCHAR* KindName = TEXT("record");
        switch (Field.Kind)
        {
        case EKyaReferenceKind::Actor: Limit = Bank.SceneEnvironment.Actors.Num(); KindName = TEXT("actor"); break;
        case EKyaReferenceKind::Waypoint: Limit = Bank.SceneEnvironment.Waypoints.Num(); KindName = TEXT("waypoint"); break;
        case EKyaReferenceKind::Path: Limit = Bank.SceneEnvironment.Paths.Num(); KindName = TEXT("path"); break;
        default: continue;
        }
        const int64 Value = GetKyaNumericField(Placement, Field.PropertyName);
        if (Value == -1) continue;
        if (Value < 0 || Value >= Limit)
            Issues.Add({Field.PropertyName.ToString(), FString::Printf(
                TEXT("%s %lld does not resolve to a %s in this bank (%d available)."),
                KindName, Value, KindName, Limit), true});
    }
    return Issues;
}
bool ValidateKyaActorNameChange(const FString& CurrentName, const FString& NewName, FString& OutError)
{
    const FString Trimmed = NewName.TrimStartAndEnd();
    if (Trimmed.Len() > 63)
    {
        OutError = FString::Printf(TEXT("Actor name '%s' is %d characters; the serialized limit is 63."),
            *Trimmed, Trimmed.Len());
        return false;
    }
    for (const TCHAR Char : Trimmed)
    {
        if (Char < 1 || Char > 127)
        {
            OutError = TEXT("Actor names must use single-byte ASCII characters; the bank writer stores one byte "
                "per character.");
            return false;
        }
    }
    const auto Align4 = [](int32 Value) { return (Value + 3) & ~3; };
    const int32 CurrentCapacity = Align4(CurrentName.Len() + 1);
    const int32 NewCapacity = Align4(Trimmed.Len() + 1);
    if (NewCapacity != CurrentCapacity)
    {
        OutError = FString::Printf(
            TEXT("Actor name '%s' needs %d serialized byte(s), but this record's fixed name slot holds %d. "
                "Stay within the same 4-byte alignment bucket (the name is never truncated)."),
            *Trimmed, NewCapacity, CurrentCapacity);
        return false;
    }
    return true;
}
