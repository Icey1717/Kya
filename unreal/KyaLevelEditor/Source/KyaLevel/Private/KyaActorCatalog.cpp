#include "KyaActorCatalog.h"

#include "KyaActorEditService.h"
#include "KyaLevelBank.h"
#include "UObject/UnrealType.h"

namespace
{
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
    if (const FKyaActorFamily* Family = KyaActorFamilies::Find(ClassId))
    {
        Info.Name = Family->DisplayName;
        Info.Notes = Family->Notes;
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
    if (const FKyaActorFamilyBehaviour* Row = KyaActorFamilies::FindBehaviour(ClassId, BehaviourId))
    {
        Info.Name = Row->Name;
        Info.bKnown = true;
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
    // The serialized default is a behaviour id, not an array position. CActor::
    // GetBehaviour matches BehaviourEntry::id, and the ids are sparse per class
    // (Wolfen, for example, uses 0..6, 8, 10, 12, ...), so the picker must offer
    // the record ids rather than their indices.
    TArray<FKyaBehaviourOption> Options;
    for (int32 Index = 0; Index < Placement.Behaviours.Num(); ++Index)
    {
        FKyaBehaviourOption& Option = Options.AddDefaulted_GetRef();
        Option.Id = Placement.Behaviours[Index].Id;
        Option.Label = FString::Printf(TEXT("Behaviour %d: %s"), Index,
            *GetKyaBehaviourLabel(Placement.ClassId, Placement.Behaviours[Index].Id));
    }
    return Options;
}

bool DoesKyaBehaviourIdExist(const TArray<FKyaActorBehaviour>& Behaviours, int32 BehaviourId)
{
    // The serialized default is a behaviour id, not an array position: CActor::
    // GetBehaviour matches BehaviourEntry::id and the ids are sparse per class.
    return BehaviourId == -1 ||
        (BehaviourId >= 0 && Behaviours.ContainsByPredicate(
            [BehaviourId](const FKyaActorBehaviour& Behaviour) { return Behaviour.Id == BehaviourId; }));
}

bool IsKyaActorArrayResizable(FName PropertyName)
{
    return PropertyName == GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, Conditions) ||
        PropertyName == GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, MacroAnimations) ||
        PropertyName == GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, Behaviours) ||
        PropertyName == GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, Models);
}

bool IsKyaActorArrayResizable(int64 ClassId, FName PropertyName)
{
    if (IsKyaActorArrayResizable(PropertyName)) return true;
    if (ClassId == 0x0d)
        return PropertyName == GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, PlatformActors) ||
            PropertyName == GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, PlatformZones) ||
            PropertyName == GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, PlatformNoFrictionZones) ||
            PropertyName == GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, PlatformCameras) ||
            PropertyName == GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, PlatformSoundsA) ||
            PropertyName == GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, PlatformSoundsB);
    if (ClassId == 0x2c)
        return PropertyName == GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, TrapActorRefs) ||
            PropertyName == GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, TrapStreamEntries);
    if (ClassId == 0x09 || ClassId == 0x06)
    {
        if (PropertyName == GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, FighterBlows) ||
            PropertyName == GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, FighterGrabs) ||
            PropertyName == GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, FighterGrabReacts) ||
            PropertyName == GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, FighterCombos))
            return true;
    }
    if (ClassId == 0x06)
        return PropertyName == GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, KimClimbZones) ||
            PropertyName == GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, KimTutorialZones) ||
            PropertyName == GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, KimProjectileZones) ||
            PropertyName == GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, KimBoomyZonePairs) ||
            PropertyName == GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, KimZoneStream) ||
            PropertyName == GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, KimCameras);
    if (ClassId == 0x50)
        return PropertyName == GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, ShockerCmdSequences);
    if (ClassId == 0x39)
        return PropertyName == GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, BasicBoxActorRefs);
    if (ClassId == 0x35)
        return PropertyName == GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, BoxActorRefs);
    if (ClassId == 0x3d)
        return PropertyName == GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, FogHierarchies);
    if (ClassId == 0x4e)
        return PropertyName == GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, BunchStreamEntries);
    if (ClassId == 0x12)
        return PropertyName == GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, WindActorRefs);
    if (ClassId == 0x4c)
        return PropertyName == GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, StillerActorRefs);
    if (ClassId == 0x36)
        return PropertyName == GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, ShipCareBoyActorRefs) ||
            PropertyName == GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, ShipFxIntValues);
    if (ClassId == 0x30)
        return PropertyName == GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, PunchingBallTrapEntries);
    if (ClassId == 0x31)
        return PropertyName == GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, AmortosCmdActorRefs);
    if (ClassId == 0x26)
        return PropertyName == GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, NoseMonsterZonePathEntries) ||
            PropertyName == GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, NoseMonsterFireShotActorRefs);
    if (ClassId == 0x1b)
        return PropertyName == GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, DcaShotActorRefs);
    if (ClassId == 0x24)
        return PropertyName == GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, TeleporterDestinations) ||
            PropertyName == GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, TeleporterWaypoints) ||
            PropertyName == GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, TeleporterConditionedOperations);
    if (ClassId == 0x38)
        return PropertyName == GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, AtonPathIndices) ||
            PropertyName == GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, AtonOtherSubObjs) ||
            PropertyName == GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, AtonSubObjs);
    if (ClassId == 0x2f)
        return PropertyName == GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, EggPathIndices);
    if (ClassId == 0x40)
        return PropertyName == GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, PatternCmdActorRefs);
    return false;
}

void KyaEnsureFighterArrayShapes(FKyaActorPlacement& Placement)
{
    if (Placement.ClassId != 0x09 && Placement.ClassId != 0x06) return;
    const auto Align = [](int32 Value) { return (Value + 3) & ~3; };
    for (FKyaFighterBlow& Blow : Placement.FighterBlows)
    {
        Blow.Stages.SetNum(3);
        Blow.NameCapacity = Align(Blow.Name.Len() + 1);
    }
    for (FKyaFighterGrab& Grab : Placement.FighterGrabs)
    {
        Grab.Stages.SetNum(5);
        Grab.NameCapacity = Align(Grab.Name.Len() + 1);
    }
    for (FKyaFighterGrabReact& React : Placement.FighterGrabReacts)
        React.NameCapacity = Align(React.Name.Len() + 1);
    for (FKyaFighterCombo& Combo : Placement.FighterCombos)
    {
        Combo.NameCapacity = Align(Combo.Name.Len() + 1);
        Combo.ActionCapacity = Align(Combo.Action.Len() + 1);
        Combo.BranchCapacities.SetNum(Combo.Branches.Num());
        for (int32 Branch = 0; Branch < Combo.Branches.Num(); ++Branch)
            Combo.BranchCapacities[Branch] = Align(Combo.Branches[Branch].Len() + 1);
    }
}

bool FindKyaReferenceField(FName PropertyName, EKyaReferenceKind& OutKind)
{
    const auto Matches = [PropertyName, &OutKind](TConstArrayView<FKyaActorFamilyReference> Rows)
    {
        for (const FKyaActorFamilyReference& Row : Rows)
        {
            if (PropertyName != FName(Row.Property)) continue;
            OutKind = Row.Kind;
            return true;
        }
        return false;
    };
    if (Matches(KyaActorFamilies::SharedReferences())) return true;
    for (const FKyaActorFamily* Family : KyaActorFamilies::All())
        if (Matches(Family->References)) return true;
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
    for (const FKyaActorFamilyReference& Row : KyaActorFamilies::SharedReferences())
        Fields.Add({FName(Row.Property), Row.Kind, -1});
    for (const FKyaActorFamily* Family : KyaActorFamilies::All())
        for (const FKyaActorFamilyReference& Row : Family->References)
            Fields.Add({FName(Row.Property), Row.Kind, Family->ClassId});
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

/**
 * Type-specific data must stay with the destination actor: its class id is not
 * editable, so its serialized Create payload has a fixed layout and meaning.
 * Behaviours and the default behaviour id travel together, and ClassData is
 * the raw remainder of the destination's own class payload.
 */
bool IsKyaClassScopedField(const FProperty* Property, FName PropertyName)
{
    if (PropertyName == GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, Behaviours) ||
        PropertyName == GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, DefaultBehaviourId) ||
        PropertyName == GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, ClassData))
        return true;
    const FString* Condition = Property->FindMetaData(TEXT("EditCondition"));
    return Condition && Condition->Contains(TEXT("bHas"));
}
}

bool BuildKyaActorCopyPropertiesPatch(const FKyaActorPlacement& Source, FKyaActorEditPatch& OutPatch)
{
    OutPatch = FKyaActorEditPatch();
    OutPatch.Custom = [Source](FKyaActorPlacement& Dest, FString& OutError)
    {
        const bool bSameClass = Source.ClassId == Dest.ClassId;
        TArray<FProperty*> Properties;
        TArray<FString> Skipped;
        FProperty* DefaultBehaviourProperty = nullptr;
        for (TFieldIterator<FProperty> It(FKyaActorPlacement::StaticStruct()); It; ++It)
        {
            const FName Name = It->GetFName();
            if (Name == GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, Name) ||
                GetKyaActorFieldCapability(Name).Kind != EKyaFieldCapability::EditableExportable) continue;
            if (!bSameClass && IsKyaClassScopedField(*It, Name))
            {
                Skipped.Add(Name.ToString());
                continue;
            }
            FString FieldError;
            if (!CanCopyKyaActorField(Source, Dest, Name, FieldError))
            {
                // Keep the destination's serialized layout: a field that cannot
                // be represented there (for example an unsupported class-specific
                // array or a nested record whose shape differs) is left alone.
                Skipped.Add(Name.ToString());
                continue;
            }
            Properties.Add(*It);
            if (Name == GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, DefaultBehaviourId))
                DefaultBehaviourProperty = *It;
        }
        // The default behaviour id must resolve in the behaviour array that will
        // exist after this patch. When the behaviour list is kept because its
        // layout differs, an id from the source may not be present.
        if (DefaultBehaviourProperty && Properties.Contains(DefaultBehaviourProperty))
        {
            const bool bCopyBehaviours = Properties.ContainsByPredicate([](const FProperty* Property)
            {
                return Property->GetFName() ==
                    GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, Behaviours);
            });
            const TArray<FKyaActorBehaviour>& ProspectiveBehaviours =
                bCopyBehaviours ? Source.Behaviours : Dest.Behaviours;
            if (!DoesKyaBehaviourIdExist(ProspectiveBehaviours, Source.DefaultBehaviourId))
            {
                Properties.Remove(DefaultBehaviourProperty);
                Skipped.Add(DefaultBehaviourProperty->GetName());
            }
        }
        if (Properties.IsEmpty())
        {
            OutError = FString::Printf(TEXT("No properties can be copied from class %lld to class %lld "
                "without changing the destination's serialized layout."), Source.ClassId, Dest.ClassId);
            return false;
        }
        if (!Skipped.IsEmpty())
        {
            FString Preview;
            const int32 PreviewCount = FMath::Min(Skipped.Num(), 12);
            for (int32 Index = 0; Index < PreviewCount; ++Index)
            {
                if (!Preview.IsEmpty()) Preview += TEXT(",");
                Preview += Skipped[Index];
            }
            UE_LOG(LogTemp, Log, TEXT("KYA_ACTOR_COPY_SKIPPED_FIELDS from=%lld to=%lld count=%d fields=%s%s"),
                Source.ClassId, Dest.ClassId, Skipped.Num(), *Preview,
                Skipped.Num() > PreviewCount ? TEXT(",...") : TEXT(""));
            UE_LOG(LogTemp, Verbose, TEXT("KYA_ACTOR_COPY_SKIPPED_FIELDS_ALL from=%lld to=%lld fields=%s"),
                Source.ClassId, Dest.ClassId, *FString::Join(Skipped, TEXT(",")));
        }
        // The patch owns its source snapshot and only mutates the destination
        // after every property has been accepted.
        for (FProperty* Property : Properties)
            Property->CopyCompleteValue_InContainer(&Dest, &Source);
        return true;
    };
    return true;
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
    // These arrays are rebuilt by the exporter, so a count or model-string slot
    // change is a supported structural edit rather than a layout mismatch.
    if (IsKyaActorArrayResizable(Source.ClassId, PropertyName)) return true;
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
    if (!DoesKyaBehaviourIdExist(Placement.Behaviours, Placement.DefaultBehaviourId))
        Issues.Add({TEXT("DefaultBehaviourId"), FString::Printf(
            TEXT("Default behaviour %d does not match any of this actor's %d behaviour record(s)."),
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
