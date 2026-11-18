#include "KyaStructuralEdits.h"

#include "KyaActorCatalog.h"
#include "KyaLevelBank.h"
#include "UObject/UnrealType.h"

FString GetKyaStructuralEditBlocker(EKyaStructuralEditKind Kind)
{
    switch (Kind)
    {
    case EKyaStructuralEditKind::CreateActor:
        return TEXT("Creating actors requires a class allocation entry and a rebuildable actor stream; "
            "use the Kya Level structural commands on a bank with a complete scene entry.");
    case EKyaStructuralEditKind::DeleteActor:
        return TEXT("Deleting actors requires a rebuildable actor stream and remapping every typed actor "
            "reference; use the Kya Level structural commands. Untyped raw payload bytes are preserved unchanged.");
    case EKyaStructuralEditKind::DuplicateActor:
        return TEXT("Duplicating actors requires a rebuildable actor stream and a class allocation entry; "
            "use the Kya Level structural commands.");
    case EKyaStructuralEditKind::RenameActor:
        return TEXT("Variable-length names require rebuilding the record's name slot and relocating the "
            "serialized stream; only same-slot renames are supported.");
    case EKyaStructuralEditKind::ResizeArray:
        return TEXT("This fixed-size array cannot be resized. Conditions, macro animations, behaviours, "
            "models, supported moving-platform/trap/fighter streams, Wolfen fight-param lists, Shocker "
            "command sequences, box/basic-box actor streams, Kim climb/tutorial/projectile/pair/zone/"
            "camera streams, the pattern-command actor-ref list, the shoot fire-ref list, the "
            "clusteriser zone tree, the fog-zone list, the Brazul A sub-object/path lists, the "
            "fog-hierarchy list, the FX integer list, the weapon RPG actor stream, the companion alert list, "
            "the Bunch stream-entry/fence/repel lists, the Brazul B switch list, the Shocker switch list, "
            "the Wind carried-actor and switch lists, the Nativ trajectory list, the Stiller actor list, the "
            "Ship care-boy/FX integer lists, the Punching ball trap list and the Amortos command actor list and "
            "the Nose monster zone-path/fire-shot lists, the Woof actor list and the DCA shot actor list and "
            "the Teleporter destination/waypoint/conditioned-operation lists and the Aton path/sub-object "
            "lists and the Blazer actor list and the Egg path-plane list are rebuilt by the writer; "
            "other arrays keep their serialized counts.");
    case EKyaStructuralEditKind::ChangeClass:
        return TEXT("Class replacement changes the record layout and class allocation counts; it requires "
            "a rebuilt actor stream and is not supported by the writer.");
    default:
        return TEXT("Unsupported structural edit.");
    }
}

bool AnalyzeKyaStructuralEdit(const UKyaLevelBank& Bank, EKyaStructuralEditKind Kind,
    int32 ActorIndex, FKyaStructuralEditAnalysis& OutAnalysis)
{
    OutAnalysis = FKyaStructuralEditAnalysis();
    OutAnalysis.Kind = Kind;

    // Bank entries are preserved except the scene stream that owns the actor records.
    for (const FKyaBankEntry& Entry : Bank.Entries)
        if (!(Entry.Type == 6 && Entry.Subtype == 1)) ++OutAnalysis.PreservedBankEntries;

    const bool bValidIndex = Bank.SceneEnvironment.Actors.IsValidIndex(ActorIndex);
    if ((Kind == EKyaStructuralEditKind::DeleteActor || Kind == EKyaStructuralEditKind::DuplicateActor ||
         Kind == EKyaStructuralEditKind::RenameActor || Kind == EKyaStructuralEditKind::ChangeClass) && !bValidIndex)
    {
        OutAnalysis.BlockingReasons.Add(TEXT("The selected actor record no longer exists."));
        return false;
    }

    const int32 ActorCount = Bank.SceneEnvironment.Actors.Num();
    switch (Kind)
    {
    case EKyaStructuralEditKind::CreateActor:
    case EKyaStructuralEditKind::DuplicateActor:
    {
        const int64 ClassId = bValidIndex ? Bank.SceneEnvironment.Actors[ActorIndex].ClassId : 0;
        OutAnalysis.ClassAllocationDelta.Add(ClassId, 1);
        if (Kind == EKyaStructuralEditKind::DuplicateActor)
            for (int32 Index = 0; Index < ActorCount; ++Index)
                OutAnalysis.ActorIndexRemap.Add(Index, Index);
        break;
    }
    case EKyaStructuralEditKind::DeleteActor:
    {
        OutAnalysis.ClassAllocationDelta.Add(Bank.SceneEnvironment.Actors[ActorIndex].ClassId, -1);
        for (int32 Index = 0; Index < ActorCount; ++Index)
            OutAnalysis.ActorIndexRemap.Add(Index, Index < ActorIndex ? Index : (Index == ActorIndex ? -1 : Index - 1));
        // Count references that would dangle after the removal. Reference encoding is
        // an actor index into the same table, so indices above the removed actor shift.
        for (int32 Source = 0; Source < ActorCount; ++Source)
        {
            if (Source == ActorIndex) continue;
            const FKyaActorPlacement& Placement = Bank.SceneEnvironment.Actors[Source];
            for (const FKyaReferenceField& Field : GetKyaReferenceFields())
            {
                if (Field.Kind != EKyaReferenceKind::Actor && Field.Kind != EKyaReferenceKind::Waypoint) continue;
                if (Field.ClassId != -1 && Field.ClassId != Placement.ClassId) continue;
                if (GetKyaNumericField(Placement, Field.PropertyName) == ActorIndex)
                    ++OutAnalysis.ReferencesToRemovedActor;
            }
        }
        break;
    }
    case EKyaStructuralEditKind::RenameActor:
    case EKyaStructuralEditKind::ResizeArray:
    case EKyaStructuralEditKind::ChangeClass:
        for (int32 Index = 0; Index < ActorCount; ++Index)
            OutAnalysis.ActorIndexRemap.Add(Index, Index);
        break;
    default:
        break;
    }

    const bool bSceneStream = Bank.BankBytes.Num() > 0;
    OutAnalysis.bWriterSupported = bSceneStream &&
        (Kind == EKyaStructuralEditKind::CreateActor || Kind == EKyaStructuralEditKind::DuplicateActor ||
         Kind == EKyaStructuralEditKind::DeleteActor || Kind == EKyaStructuralEditKind::ResizeArray);
    if (bSceneStream && (Kind == EKyaStructuralEditKind::CreateActor ||
        Kind == EKyaStructuralEditKind::DuplicateActor))
    {
        const int64 ClassId = bValidIndex ? Bank.SceneEnvironment.Actors[ActorIndex].ClassId : 0;
        if (!Bank.SceneConfiguration.ActorClassAllocations.ContainsByPredicate(
            [ClassId](const FKyaActorClassAllocation& Allocation) { return Allocation.ClassId == ClassId; }))
        {
            OutAnalysis.bWriterSupported = false;
            OutAnalysis.BlockingReasons.Add(FString::Printf(
                TEXT("Actor class %lld has no scene allocation entry."), ClassId));
        }
    }
    if (!OutAnalysis.bWriterSupported)
        OutAnalysis.BlockingReasons.Add(GetKyaStructuralEditBlocker(Kind));
    else if (Kind == EKyaStructuralEditKind::DeleteActor && OutAnalysis.ReferencesToRemovedActor > 0)
        OutAnalysis.BlockingReasons.Add(FString::Printf(
            TEXT("%d typed actor reference(s) will be remapped by the structural writer."),
            OutAnalysis.ReferencesToRemovedActor));
    return true;
}

namespace
{
bool ValidateStringSlot(const FString& Current, const FString& Candidate, const FString& Path, FString& OutError)
{
    if (Current == Candidate) return true;
    const auto Align4 = [](int32 Value) { return (Value + 3) & ~3; };
    if (Align4(Current.Len() + 1) == Align4(Candidate.Len() + 1)) return true;
    OutError = FString::Printf(TEXT("%s%s requires a different serialized string slot (%d -> %d bytes). %s"),
        *Path, (Path.IsEmpty() ? TEXT("") : TEXT(".")), Align4(Current.Len() + 1),
        Align4(Candidate.Len() + 1), *GetKyaStructuralEditBlocker(EKyaStructuralEditKind::ResizeArray));
    return false;
}

bool IsKyaNestedArrayResizable(int64 ClassId, const FString& Path)
{
    // Shoot actors own a fire-wave behaviour with a variable actor-reference list,
    // and clusterisers own a variable cluster-zone tree.
    if (ClassId == 0x3e && Path.StartsWith(TEXT("Behaviours[")) &&
        Path.EndsWith(TEXT(".ShootFireActorRefs"))) return true;
    if (ClassId == 0x18 && Path.StartsWith(TEXT("Behaviours[")))
    {
        if (Path.EndsWith(TEXT(".ClusterZones"))) return true;
        if (Path.Contains(TEXT(".ClusterZones[")) &&
            (Path.EndsWith(TEXT(".Actors")) || Path.EndsWith(TEXT(".OtherActors")) ||
             Path.EndsWith(TEXT(".Cameras")) || Path.EndsWith(TEXT(".Lights")) ||
             Path.EndsWith(TEXT(".Hashes")))) return true;
    }
    if (ClassId == 0x3d && Path.StartsWith(TEXT("Behaviours[")) &&
        Path.EndsWith(TEXT(".FogZones"))) return true;
    if (ClassId == 0x2e && Path.StartsWith(TEXT("Behaviours[")) &&
        Path.EndsWith(TEXT(".FxUnknownValues"))) return true;
    if (ClassId == 0x20 && Path.StartsWith(TEXT("Behaviours[")) &&
        Path.EndsWith(TEXT(".WeaponRpgActorRefs"))) return true;
    if (ClassId == 0x19 && Path.StartsWith(TEXT("Behaviours[")) &&
        Path.EndsWith(TEXT(".CompanionAlerts"))) return true;
    if (ClassId == 0x4e && Path.StartsWith(TEXT("Behaviours[")))
    {
        if (Path.EndsWith(TEXT(".BunchFenceWayPoints")) ||
            Path.EndsWith(TEXT(".BunchFenceGroups")) ||
            Path.EndsWith(TEXT(".BunchRepelIndices"))) return true;
    }
    if (ClassId == 0x4b && Path == TEXT("ShockerSwitch.Targets")) return true;
    if (ClassId == 0x2a && Path == TEXT("WoofData.ActorRefs")) return true;
    if (ClassId == 0x4d && Path == TEXT("BlazerData.ActorRefs")) return true;
    if (ClassId == 0x38)
    {
        if (Path.StartsWith(TEXT("AtonOtherSubObjs[")) && Path.EndsWith(TEXT(".CinematicIds")))
            return true;
        if (Path.StartsWith(TEXT("AtonSubObjs[")) && Path.EndsWith(TEXT(".CinematicIds")))
            return true;
    }
    if (ClassId == 0x24 && Path.StartsWith(TEXT("TeleporterConditionedOperations[")))
    {
        if (Path.EndsWith(TEXT(".Operations")) || Path.EndsWith(TEXT(".Conditions")))
            return true;
    }
    if (ClassId == 0x12 && (Path == TEXT("WindSwitch1D0.Targets") ||
        Path == TEXT("WindSwitch1D8.Targets") || Path == TEXT("WindSwitch1E0.Targets"))) return true;
    if (ClassId == 0x0a && Path.StartsWith(TEXT("Behaviours[")))
    {
        if (Path.EndsWith(TEXT(".NativSpeakValues")) ||
            Path.EndsWith(TEXT(".NativTrajectories")) ||
            Path.EndsWith(TEXT(".NativSellerSubObjs"))) return true;
        if (Path.Contains(TEXT(".NativSellerSubObjs[")) &&
            Path.EndsWith(TEXT(".CinematicIds"))) return true;
        if (Path.Contains(TEXT(".AkasaBehaviour.")))
        {
            if (Path.EndsWith(TEXT(".Tutorials")) ||
                Path.EndsWith(TEXT(".AddOnSubObjs")) ||
                Path.EndsWith(TEXT(".Steps")) ||
                Path.EndsWith(TEXT(".ComboNames"))) return true;
            if (Path.Contains(TEXT(".AddOnSubObjs[")) &&
                Path.EndsWith(TEXT(".CinematicIds"))) return true;
        }
    }
    if (ClassId == 0x4f && Path.StartsWith(TEXT("Behaviours[")))
    {
        if (Path.EndsWith(TEXT(".BrazulSubObjs"))) return true;
        if (Path.Contains(TEXT(".BrazulSubObjs[")) && Path.EndsWith(TEXT(".PathIndices")))
            return true;
        if (Path.EndsWith(TEXT(".BrazulBSwitch.Targets"))) return true;
    }
    // Kim tutorial zones own a per-zone switch-target list.
    if (ClassId == 0x06 && Path.StartsWith(TEXT("KimTutorialZones[")) &&
        Path.EndsWith(TEXT(".Targets"))) return true;
    // Wolfen fight parameters own combo lists and capability item/chain lists.
    if (ClassId != 0x09) return false;
    if (Path == TEXT("WolfenFightParamData.ComboList1") ||
        Path == TEXT("WolfenFightParamData.ComboList2")) return true;
    if (Path.StartsWith(TEXT("WolfenFightParamData.Capabilities[")) &&
        Path.EndsWith(TEXT(".Items"))) return true;
    return Path.StartsWith(TEXT("WolfenFightParamData.Capabilities[")) &&
        Path.Contains(TEXT(".Items[")) && Path.EndsWith(TEXT(".Chains"));
}

bool ValidateKyaStructLayout(const UScriptStruct* Struct, const void* Current, const void* Candidate,
    int64 ClassId, const FString& Path, FString& OutError)
{
    for (TFieldIterator<FProperty> It(Struct); It; ++It)
    {
        const FProperty* Property = *It;
        const FString PropertyPath = Path.IsEmpty() ? Property->GetName()
            : FString::Printf(TEXT("%s.%s"), *Path, *Property->GetName());
        const void* CurrentValue = Property->ContainerPtrToValuePtr<void>(Current);
        const void* CandidateValue = Property->ContainerPtrToValuePtr<void>(Candidate);

        if (const FArrayProperty* Array = CastField<FArrayProperty>(Property))
        {
            FScriptArrayHelper CurrentArray(Array, CurrentValue);
            FScriptArrayHelper CandidateArray(Array, CandidateValue);
            if (Struct == FKyaActorPlacement::StaticStruct() &&
                IsKyaActorArrayResizable(ClassId, Property->GetFName())) continue;
            if (IsKyaNestedArrayResizable(ClassId, PropertyPath)) continue;
            if (CurrentArray.Num() != CandidateArray.Num())
            {
                OutError = FString::Printf(TEXT("%s count changed (%d -> %d). %s"), *PropertyPath,
                    CurrentArray.Num(), CandidateArray.Num(),
                    *GetKyaStructuralEditBlocker(EKyaStructuralEditKind::ResizeArray));
                return false;
            }
            if (const FStructProperty* InnerStruct = CastField<FStructProperty>(Array->Inner))
            {
                for (int32 Index = 0; Index < CurrentArray.Num(); ++Index)
                    if (!ValidateKyaStructLayout(InnerStruct->Struct, CurrentArray.GetRawPtr(Index),
                        CandidateArray.GetRawPtr(Index), ClassId,
                        FString::Printf(TEXT("%s[%d]"), *PropertyPath, Index), OutError)) return false;
            }
            else if (const FStrProperty* InnerString = CastField<FStrProperty>(Array->Inner))
            {
                for (int32 Index = 0; Index < CurrentArray.Num(); ++Index)
                    if (!ValidateStringSlot(InnerString->GetPropertyValue(CurrentArray.GetRawPtr(Index)),
                        InnerString->GetPropertyValue(CandidateArray.GetRawPtr(Index)),
                        FString::Printf(TEXT("%s[%d]"), *PropertyPath, Index), OutError)) return false;
            }
        }
        else if (const FStructProperty* StructProperty = CastField<FStructProperty>(Property))
        {
            if (!ValidateKyaStructLayout(StructProperty->Struct, CurrentValue, CandidateValue, ClassId,
                PropertyPath, OutError)) return false;
        }
        else if (const FStrProperty* StringProperty = CastField<FStrProperty>(Property))
        {
            if (!ValidateStringSlot(StringProperty->GetPropertyValue(CurrentValue),
                StringProperty->GetPropertyValue(CandidateValue), PropertyPath, OutError)) return false;
        }
        else if (const FSetProperty* SetProperty = CastField<FSetProperty>(Property))
        {
            FScriptSetHelper CurrentSet(SetProperty, CurrentValue);
            FScriptSetHelper CandidateSet(SetProperty, CandidateValue);
            if (CurrentSet.Num() != CandidateSet.Num())
            {
                OutError = FString::Printf(TEXT("%s count changed (%d -> %d). %s"), *PropertyPath,
                    CurrentSet.Num(), CandidateSet.Num(),
                    *GetKyaStructuralEditBlocker(EKyaStructuralEditKind::ResizeArray));
                return false;
            }
        }
        else if (const FMapProperty* MapProperty = CastField<FMapProperty>(Property))
        {
            FScriptMapHelper CurrentMap(MapProperty, CurrentValue);
            FScriptMapHelper CandidateMap(MapProperty, CandidateValue);
            if (CurrentMap.Num() != CandidateMap.Num())
            {
                OutError = FString::Printf(TEXT("%s count changed (%d -> %d). %s"), *PropertyPath,
                    CurrentMap.Num(), CandidateMap.Num(),
                    *GetKyaStructuralEditBlocker(EKyaStructuralEditKind::ResizeArray));
                return false;
            }
        }
    }
    return true;
}
}

bool ValidateKyaActorStructuralEdit(const FKyaActorPlacement& Current,
    const FKyaActorPlacement& Candidate, FString& OutError)
{
    if (Candidate.ClassId != Current.ClassId)
    {
        OutError = GetKyaStructuralEditBlocker(EKyaStructuralEditKind::ChangeClass);
        return false;
    }
    if (Candidate.Name != Current.Name)
    {
        if (!ValidateKyaActorNameChange(Current.Name, Candidate.Name, OutError))
        {
            OutError = FString::Printf(TEXT("%s %s"), *OutError,
                *GetKyaStructuralEditBlocker(EKyaStructuralEditKind::RenameActor));
            return false;
        }
    }
    return ValidateKyaStructLayout(FKyaActorPlacement::StaticStruct(), &Current, &Candidate, Current.ClassId,
        FString(), OutError);
}
