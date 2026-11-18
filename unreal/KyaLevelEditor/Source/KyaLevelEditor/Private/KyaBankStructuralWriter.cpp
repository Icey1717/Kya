#include "KyaBankStructuralWriter.h"

#include "KyaLevelBank.h"
#include "KyaBinaryHelpers.h"
#include "KyaCameraStream.h"
#include "Actors/KyaAmortosCmd.h"
#include "Actors/KyaBasicBox.h"
#include "Actors/KyaBlazer.h"
#include "Actors/KyaBox.h"
#include "Actors/KyaBunch.h"
#include "Actors/KyaDca.h"
#include "Actors/KyaEgg.h"
#include "Actors/KyaFighter.h"
#include "Actors/KyaKim.h"
#include "Actors/KyaMovingPlatform.h"
#include "Actors/KyaNoseMonster.h"
#include "Actors/KyaPatternCmd.h"
#include "Actors/KyaPunchingBall.h"
#include "Actors/KyaShocker.h"
#include "Actors/KyaShockerCmd.h"
#include "Actors/KyaShip.h"
#include "Actors/KyaStiller.h"
#include "Actors/KyaTeleporter.h"
#include "Actors/KyaTrap.h"
#include "Actors/KyaWind.h"
#include "Actors/KyaWoof.h"
#include "Actors/KyaWolfen.h"
#include "UObject/UnrealType.h"

namespace
{
constexpr uint32 InstTag = 0x494e5354; // "INST" (bytes TSNI in the file)
constexpr uint32 IsoTag = 0x504f5349;  // "ISOP"
constexpr uint32 PathTag = 0x50415448; // "HTAP"

bool EvaluateEditCondition(const FString& Condition, const UScriptStruct* Struct, const void* Base);

bool PropertyIsActive(const FProperty* Property, const UScriptStruct* Struct, const void* Base)
{
    const FString Condition = Property->GetMetaData(TEXT("EditCondition"));
    return Condition.IsEmpty() || EvaluateEditCondition(Condition, Struct, Base);
}

struct FConditionParser
{
    const FString& Text;
    int32 Pos = 0;
    const UScriptStruct* Struct = nullptr;
    const void* Base = nullptr;
    bool bValid = true;

    explicit FConditionParser(const FString& InText, const UScriptStruct* InStruct, const void* InBase)
        : Text(InText), Struct(InStruct), Base(InBase) {}

    void Skip() { while (Pos < Text.Len() && FChar::IsWhitespace(Text[Pos])) ++Pos; }
    bool Consume(const TCHAR* Token)
    {
        Skip();
        const int32 Length = FCString::Strlen(Token);
        if (Text.Mid(Pos, Length) != Token) return false;
        Pos += Length;
        return true;
    }
    bool Primary()
    {
        Skip();
        if (Consume(TEXT("(")))
        {
            const bool Value = Or();
            if (!Consume(TEXT(")"))) bValid = false;
            return Value;
        }
        if (Consume(TEXT("!"))) return !Unary();
        if (Pos >= Text.Len()) { bValid = false; return true; }
        if (FChar::IsDigit(Text[Pos]))
        {
            int32 Start = Pos;
            while (Pos < Text.Len() && FChar::IsDigit(Text[Pos])) ++Pos;
            return FCString::Atoi(*Text.Mid(Start, Pos - Start)) != 0;
        }
        int32 Start = Pos;
        while (Pos < Text.Len() && (FChar::IsAlnum(Text[Pos]) || Text[Pos] == TEXT('_'))) ++Pos;
        const FString Identifier = Text.Mid(Start, Pos - Start);
        if (Identifier.IsEmpty()) { bValid = false; return true; }
        const FProperty* Flag = Struct ? Struct->FindPropertyByName(FName(*Identifier)) : nullptr;
        if (!Flag) return true;
        const void* Value = Flag->ContainerPtrToValuePtr<void>(Base);
        if (const FBoolProperty* Bool = CastField<FBoolProperty>(Flag)) return Bool->GetPropertyValue(Value);
        if (const FIntProperty* Int = CastField<FIntProperty>(Flag)) return Int->GetPropertyValue(Value) != 0;
        if (const FInt64Property* Int64 = CastField<FInt64Property>(Flag)) return Int64->GetPropertyValue(Value) != 0;
        return true;
    }
    bool Unary()
    {
        if (bValid && Consume(TEXT("!"))) return !Unary();
        return Primary();
    }
    bool Equality()
    {
        bool Value = Unary();
        while (bValid)
        {
            Skip();
            if (Consume(TEXT("==")))
            {
                const bool Right = Unary();
                Value = Value == Right;
            }
            else break;
        }
        return Value;
    }
    bool And()
    {
        bool Value = Equality();
        while (bValid && Consume(TEXT("&&"))) Value = Equality() && Value;
        return Value;
    }
    bool Or()
    {
        bool Value = And();
        while (bValid && Consume(TEXT("||"))) Value = And() || Value;
        return Value;
    }
};

bool EvaluateEditCondition(const FString& Condition, const UScriptStruct* Struct, const void* Base)
{
    FConditionParser Parser(Condition, Struct, Base);
    const bool Value = Parser.Or();
    return Parser.bValid ? Value : true;
}

uint32 ReadU32(const TArray<uint8>& Bytes, int64 Offset)
{
    if (Offset < 0 || Offset + 4 > Bytes.Num()) return 0;
    const uint8* P = Bytes.GetData() + Offset;
    return uint32(P[0]) | (uint32(P[1]) << 8) | (uint32(P[2]) << 16) | (uint32(P[3]) << 24);
}

void WriteU32(TArray<uint8>& Bytes, int64 Offset, uint32 Value)
{
    if (Offset < 0 || Offset + 4 > Bytes.Num()) return;
    for (int32 Byte = 0; Byte < 4; ++Byte) Bytes[Offset + Byte] = uint8(Value >> (Byte * 8));
}

void AppendU32(TArray<uint8>& Bytes, uint32 Value)
{
    for (int32 Byte = 0; Byte < 4; ++Byte) Bytes.Add(uint8(Value >> (Byte * 8)));
}

void AppendI32(TArray<uint8>& Bytes, int32 Value) { AppendU32(Bytes, uint32(Value)); }

void AppendF32(TArray<uint8>& Bytes, float Value)
{
    uint32 Bits = 0;
    FMemory::Memcpy(&Bits, &Value, sizeof(Bits));
    AppendU32(Bytes, Bits);
}

/** Append a NUL-terminated string and pad to the 4-byte slot alignment the reader uses. */
void AppendAlignedString(TArray<uint8>& Bytes, int64 AbsoluteStart, const FString& Text)
{
    for (const TCHAR Char : Text) Bytes.Add(uint8(Char));
    Bytes.Add(0);
    while ((AbsoluteStart + Bytes.Num()) % 4 != 0) Bytes.Add(0);
}

uint32 AlignU32(int32 Value) { return uint32((Value + 3) & ~3); }

struct FActorStream
{
    int32 SceneEntryIndex = INDEX_NONE;
    int64 CountPos = 0; // relative to the scene entry
    int64 ActorEnd = 0; // relative to the scene entry
    int64 TailSize = 0; // bytes preserved after the actor stream
    TArray<FKyaActorOffsets> Actors;
};

bool FindSceneEntry(const UKyaLevelBank& Bank, int32& OutIndex, const FKyaBankEntry*& OutEntry)
{
    OutIndex = INDEX_NONE;
    OutEntry = nullptr;
    for (int32 Index = 0; Index < Bank.Entries.Num(); ++Index)
    {
        const FKyaBankEntry& Entry = Bank.Entries[Index];
        if (Entry.Type == 6 && Entry.Subtype == 1)
        {
            if (OutEntry) return false;
            OutIndex = Index;
            OutEntry = &Entry;
        }
    }
    return OutEntry != nullptr;
}

bool LocateActorStream(const UKyaLevelBank& Bank, FActorStream& Out, FString& OutError)
{
    Out = FActorStream();
    const FKyaBankEntry* Scene = nullptr;
    if (!FindSceneEntry(Bank, Out.SceneEntryIndex, Scene) || !Scene)
    {
        OutError = TEXT("The bank does not have exactly one scene-data entry.");
        return false;
    }
    if (Scene->Offset < 0 || Scene->Size < 24 || Scene->Offset > Bank.BankBytes.Num() ||
        Scene->Size > Bank.BankBytes.Num() - Scene->Offset)
    {
        OutError = TEXT("The scene-data entry lies outside the bank bytes.");
        return false;
    }
    // The type table may live after the scene entry. Relocation would move its bytes,
    // so either move its header pointer or reject a table that overlaps the scene.
    const uint32 CommonType = ReadU32(Bank.BankBytes, 0x14);
    if (CommonType == 0 && Bank.BankBytes.Num() >= 0x38)
    {
        const uint64 TypeTable = uint64(ReadU32(Bank.BankBytes, 0x34)) + 8;
        const uint64 TypeTableEnd = TypeTable + uint64(Bank.Entries.Num()) * 4;
        if (TypeTable < uint64(Scene->Offset + Scene->Size) && TypeTableEnd > uint64(Scene->Offset))
        {
            OutError = TEXT("The bank type table overlaps the scene data; the actor stream cannot be relocated safely.");
            return false;
        }
    }
    const int64 Start = Scene->Offset;
    const int64 End = Start + Scene->Size;
    const TConstArrayView<uint8> View(Bank.BankBytes.GetData() + Start, int32(Scene->Size));
    FActorStream Best;
    bool bFound = false;
    for (int64 Tag = Start; Tag + 16 <= End; ++Tag)
    {
        if (ReadU32(Bank.BankBytes, Tag) != IsoTag) continue;
        const uint32 WaypointSize = ReadU32(Bank.BankBytes, Tag + 4);
        const int64 Path = Tag + WaypointSize;
        if (Path + 8 > End || ReadU32(Bank.BankBytes, Path) != PathTag) continue;
        const int64 CountPos = Path + ReadU32(Bank.BankBytes, Path + 4);
        if (CountPos + 4 > End) continue;
        const uint32 Count = ReadU32(Bank.BankBytes, CountPos);
        if (Count > 100000) continue;

        TArray<FKyaActorOffsets> Actors;
        const int64 Waypoints = Tag - Start + 12;
        if (!KyaReadActorOffsets(View, Waypoints, 0, Actors) || Actors.Num() != int32(Count)) continue;

        int64 ActorEnd = CountPos - Start + 4;
        bool bValid = true;
        for (const FKyaActorOffsets& Actor : Actors)
        {
            if (Actor.Chunk < 0 || Actor.Chunk + 8 > View.Num() || ReadU32(Bank.BankBytes, Start + Actor.Chunk) != InstTag)
            {
                bValid = false;
                break;
            }
            const uint32 Size = ReadU32(Bank.BankBytes, Start + Actor.Chunk + 4);
            if (Size < 16 || Actor.Chunk + int64(Size) > View.Num())
            {
                bValid = false;
                break;
            }
            ActorEnd = FMath::Max(ActorEnd, Actor.Chunk + int64(Size));
        }
        if (!bValid || ActorEnd > Scene->Size) continue;

        // Pick the largest valid actor stream. Empty path/waypoint lookalikes can also
        // match the tags and must not win over the real record list.
        if (!bFound || Actors.Num() > Best.Actors.Num() ||
            (Actors.Num() == Best.Actors.Num() && ActorEnd > Best.ActorEnd))
        {
            bFound = true;
            Best.SceneEntryIndex = Out.SceneEntryIndex;
            Best.CountPos = CountPos - Start;
            Best.ActorEnd = ActorEnd;
            Best.TailSize = Scene->Size - ActorEnd;
            Best.Actors = MoveTemp(Actors);
        }
    }
    if (!bFound)
    {
        OutError = TEXT("The bank's actor stream could not be located; structural edits require a rebuildable scene entry.");
        return false;
    }
    Out = MoveTemp(Best);
    return true;
}

bool ValidateActorIdentity(const UKyaLevelBank& Bank, FString& OutError)
{
    if (!Bank.HasCurrentImportSchema())
    {
        OutError = TEXT("Reimport the bank before making structural edits.");
        return false;
    }
    for (int32 Index = 0; Index < Bank.SceneEnvironment.Actors.Num(); ++Index)
    {
        const FKyaActorPlacement& Actor = Bank.SceneEnvironment.Actors[Index];
        if (Actor.Name.IsEmpty())
        {
            OutError = FString::Printf(TEXT("Actor %d has an empty name."), Index);
            return false;
        }
        for (TCHAR Char : Actor.Name)
            if (Char < 1 || Char > 127)
            {
                OutError = FString::Printf(TEXT("Actor %d has a non-ASCII name."), Index);
                return false;
            }
    }
    return true;
}

FString MakeActorName(const UKyaLevelBank& Bank, int32 SourceIndex, int32 Capacity, bool bBaseFromSource)
{
    const TCHAR* Fallback = TEXT("NEW_ACTOR");
    FString Base = bBaseFromSource && Bank.SceneEnvironment.Actors.IsValidIndex(SourceIndex)
        ? Bank.SceneEnvironment.Actors[SourceIndex].Name : FString(Fallback);
    if (Base.IsEmpty()) Base = Fallback;
    TSet<FString> Used;
    for (const FKyaActorPlacement& Actor : Bank.SceneEnvironment.Actors) Used.Add(Actor.Name);
    const int32 MaxLength = FMath::Max(1, Capacity - 1);
    for (int32 Suffix = 1; Suffix < 10000; ++Suffix)
    {
        const FString Tail = FString::Printf(TEXT("_%d"), Suffix);
        FString Candidate;
        if (Base.Len() + Tail.Len() <= MaxLength) Candidate = Base + Tail;
        else Candidate = Base.Left(FMath::Max(1, MaxLength - Tail.Len())) + Tail;
        if (Candidate.Len() <= MaxLength && !Used.Contains(Candidate)) return Candidate;
    }
    return Base.Left(MaxLength);
}

bool UpdateClassAllocations(UKyaLevelBank& Bank, FString& OutError)
{
    TMap<int32, int32> Counts;
    for (const FKyaActorPlacement& Actor : Bank.SceneEnvironment.Actors)
        Counts.FindOrAdd(int32(Actor.ClassId))++;
    for (const TPair<int32, int32>& Pair : Counts)
    {
        const bool bAllocated = Bank.SceneConfiguration.ActorClassAllocations.ContainsByPredicate(
            [&Pair](const FKyaActorClassAllocation& Allocation) { return Allocation.ClassId == Pair.Key; });
        if (!bAllocated)
        {
            OutError = FString::Printf(TEXT("Actor class %d has no scene allocation entry; adding that class "
                "would resize the allocation table."), Pair.Key);
            return false;
        }
    }
    for (FKyaActorClassAllocation& Allocation : Bank.SceneConfiguration.ActorClassAllocations)
    {
        if (const int32* Count = Counts.Find(Allocation.ClassId))
            Allocation.Count = *Count;
        else if (Allocation.Count > 0)
            Allocation.Count = 0;
    }
    return true;
}

bool NeedsPlatformClassDataRebuild(const FKyaActorPlacement& Actor, TConstArrayView<uint8> ClassData)
{
    if (Actor.ClassId != 0x0d || !Actor.bHasPlatformPrefix) return false;
    FKyaPlatformPrefixOffsets Prefix;
    if (!KyaReadPlatformPrefix(ClassData, 0, ClassData.Num(), Prefix)) return true;
    if (Actor.PlatformActors.Num() != Prefix.ActorCount ||
        Actor.PlatformZones.Num() != Prefix.ZoneCount ||
        Actor.PlatformNoFrictionZones.Num() != Prefix.NoFrictionZones.Num())
        return true;
    const uint8* Bytes = ClassData.GetData();
    for (int32 Index = 0; Index < Prefix.NoFrictionZones.Num(); ++Index)
    {
        const int64 Offset = Prefix.NoFrictionZones[Index];
        if (Offset < 0 || Offset + 8 > ClassData.Num()) return true;
        const uint32 Count = uint32(Bytes[Offset + 4]) | (uint32(Bytes[Offset + 5]) << 8) |
            (uint32(Bytes[Offset + 6]) << 16) | (uint32(Bytes[Offset + 7]) << 24);
        if (Count != uint32(Actor.PlatformNoFrictionZones[Index].Actors.Num())) return true;
    }
    if (Actor.bHasPlatformDetails)
    {
        FKyaPlatformDetailsOffsets Details;
        if (!KyaReadPlatformDetails(ClassData, Prefix.Suffix, ClassData.Num() - Prefix.Suffix, Details))
            return true;
        if (Actor.PlatformCameras.Num() != Details.CameraCount ||
            Actor.PlatformSoundsA.Num() != Details.SoundCounts[0] ||
            Actor.PlatformSoundsB.Num() != Details.SoundCounts[1])
            return true;
    }
    return false;
}

bool BuildMovingPlatformClassData(const FKyaActorPlacement& Actor, TArray<uint8>& Out, FString& OutError)
{
    Out.Reset();
    if (!Actor.bHasPlatformPrefix)
    {
        OutError = TEXT("The moving-platform actor is missing its prefix flag.");
        return false;
    }

    AppendF32(Out, Actor.PlatformWeight);
    AppendU32(Out, uint32(Actor.PlatformActors.Num()));
    Out.AddZeroed(Actor.PlatformActors.Num() * 8);
    AppendU32(Out, uint32(Actor.PlatformZones.Num()));
    Out.AddZeroed(Actor.PlatformZones.Num() * 4);
    AppendU32(Out, uint32(Actor.PlatformNoFrictionZones.Num()));
    for (const FKyaPlatformNoFrictionZone& Zone : Actor.PlatformNoFrictionZones)
    {
        AppendU32(Out, uint32(Zone.ZoneRef));
        AppendU32(Out, uint32(Zone.Actors.Num()));
        Out.AddZeroed(Zone.Actors.Num() * 4);
    }

    if (Actor.bHasPlatformDetails)
    {
        AppendU32(Out, uint32(Actor.PlatformCameras.Num()));
        Out.AddZeroed(Actor.PlatformCameras.Num() * 72);
        Out.AddZeroed(76); // properties block, filled by the codec below
        AppendU32(Out, uint32(Actor.PlatformSoundsA.Num()));
        Out.AddZeroed(Actor.PlatformSoundsA.Num() * 16);
        AppendU32(Out, uint32(Actor.PlatformSoundsB.Num()));
        Out.AddZeroed(Actor.PlatformSoundsB.Num() * 16);
    }
    if (!Actor.ClassData.IsEmpty()) Out.Append(Actor.ClassData);

    if (!KyaWriteMovingPlatformActor(Actor, Out, 0, Out.Num()))
    {
        OutError = TEXT("The moving-platform class data could not be rebuilt.");
        return false;
    }
    return true;
}

bool NeedsTrapClassDataRebuild(const FKyaActorPlacement& Actor, TConstArrayView<uint8> ClassData)
{
    if (Actor.ClassId != 0x2c || !Actor.bHasTrapData) return false;
    FKyaTrapOffsets Offsets;
    if (!KyaReadTrapOffsets(ClassData, 0, ClassData.Num(), Offsets)) return true;
    return Actor.TrapActorRefs.Num() != Offsets.ActorCount ||
        Actor.TrapStreamEntries.Num() != Offsets.TrapCount ||
        Actor.bHasTrapField24 != Offsets.bHasField24;
}

bool BuildTrapClassData(const FKyaActorPlacement& Actor, TArray<uint8>& Out, FString& OutError)
{
    Out.Reset();
    if (!Actor.bHasTrapData)
    {
        OutError = TEXT("The trap actor is missing its data flag.");
        return false;
    }

    AppendF32(Out, Actor.TrapWeightB);
    AppendU32(Out, uint32(Actor.TrapActorRefs.Num()));
    for (const int32 ActorRef : Actor.TrapActorRefs) AppendI32(Out, ActorRef);
    AppendI32(Out, Actor.TrapField14);
    AppendI32(Out, Actor.TrapField18);
    AppendI32(Out, Actor.TrapField1C);
    AppendI32(Out, Actor.TrapField20);
    if (Actor.bHasTrapField24) AppendI32(Out, Actor.TrapField24);
    AppendI32(Out, Actor.TrapActorRef);
    AppendF32(Out, Actor.TrapSpeed);
    AppendF32(Out, Actor.TrapAcceleration);
    AppendI32(Out, Actor.TrapPathFollowIndex);
    AppendF32(Out, Actor.TrapField4C);
    AppendI32(Out, Actor.TrapRequiredEscapeAttempts);
    AppendU32(Out, uint32(Actor.TrapStreamEntries.Num()));
    Out.AddZeroed(Actor.TrapStreamEntries.Num() * 60);

    if (!KyaWriteTrapActor(Actor, Out, 0, Out.Num()))
    {
        OutError = TEXT("The trap class data could not be rebuilt.");
        return false;
    }
    return true;
}

bool NeedsWolfenFightRebuild(const FKyaActorPlacement& Actor, TConstArrayView<uint8> ClassData);

bool NeedsFighterClassDataRebuild(const FKyaActorPlacement& Actor, TConstArrayView<uint8> ClassData)
{
    if ((Actor.ClassId != 0x09 && Actor.ClassId != 0x06) ||
        (!Actor.bHasWolfenData && !Actor.bHasKimData))
        return false;
    FKyaFighterPrefixOffsets Prefix;
    if (!KyaReadFighterPrefixOffsets(ClassData, 0, ClassData.Num(), Prefix)) return true;
    const auto U32At = [ClassData](int64 Offset)
    {
        const uint8* P = ClassData.GetData() + Offset;
        return uint32(P[0]) | (uint32(P[1]) << 8) | (uint32(P[2]) << 16) | (uint32(P[3]) << 24);
    };
    if (U32At(Prefix.Blows) != uint32(Actor.FighterBlows.Num()) ||
        U32At(Prefix.Grabs) != uint32(Actor.FighterGrabs.Num()) ||
        U32At(Prefix.GrabReacts) != uint32(Actor.FighterGrabReacts.Num()) ||
        U32At(Prefix.Combos) != uint32(Actor.FighterCombos.Num()))
        return true;

    int64 Pos = Prefix.Blows + 4;
    for (int32 Index = 0; Index < Actor.FighterBlows.Num(); ++Index)
    {
        FKyaFighterBlowOffsets Off;
        if (!KyaReadFighterBlowOffsets(ClassData, Pos, ClassData.Num(), Off)) return true;
        const FKyaFighterBlow& Blow = Actor.FighterBlows[Index];
        if (Off.NameCapacity != int32(AlignU32(Blow.Name.Len() + 1)) ||
            Off.SubObjCount != Blow.SubObjs.Num() || Blow.Stages.Num() != 3)
            return true;
    }
    Pos = Prefix.Grabs + 4;
    for (int32 Index = 0; Index < Actor.FighterGrabs.Num(); ++Index)
    {
        FKyaFighterGrabOffsets Off;
        if (!KyaReadFighterGrabOffsets(ClassData, Pos, ClassData.Num(), Off)) return true;
        const FKyaFighterGrab& Grab = Actor.FighterGrabs[Index];
        if (Off.NameCapacity != int32(AlignU32(Grab.Name.Len() + 1)) || Grab.Stages.Num() != 5)
            return true;
    }
    Pos = Prefix.GrabReacts + 4;
    for (int32 Index = 0; Index < Actor.FighterGrabReacts.Num(); ++Index)
    {
        const int64 NameStart = Pos;
        if (!KyaFighterSkipString(ClassData, Pos, ClassData.Num()) || ClassData.Num() - Pos < 8)
            return true;
        const FKyaFighterGrabReact& React = Actor.FighterGrabReacts[Index];
        if (int64(AlignU32(React.Name.Len() + 1)) != Pos - NameStart) return true;
        Pos += 8;
    }
    Pos = Prefix.Combos + 4;
    for (int32 Index = 0; Index < Actor.FighterCombos.Num(); ++Index)
    {
        FKyaFighterComboOffsets Off;
        if (!KyaReadFighterComboOffsets(ClassData, Pos, ClassData.Num(), Off)) return true;
        const FKyaFighterCombo& Combo = Actor.FighterCombos[Index];
        if (Off.NameCapacity != int32(AlignU32(Combo.Name.Len() + 1)) ||
            Off.InputCount != Combo.Inputs.Num() ||
            Off.ActionCapacity != int32(AlignU32(Combo.Action.Len() + 1)) ||
            Off.BranchOffsets.Num() != Combo.Branches.Num()) return true;
        for (int32 Branch = 0; Branch < Combo.Branches.Num(); ++Branch)
            if (Off.BranchCapacities[Branch] != int32(AlignU32(Combo.Branches[Branch].Len() + 1)))
                return true;
    }
    return NeedsWolfenFightRebuild(Actor, ClassData);
}

bool BuildFighterPrefixLayout(const FKyaActorPlacement& Actor, int64 AbsoluteStart, TArray<uint8>& Out,
    FString& OutError)
{
    Out.Reset();
    Out.AddZeroed(4 + 16 + 156); // weight, autonomous block, fixed fighter tuning

    AppendU32(Out, uint32(Actor.FighterBlows.Num()));
    for (const FKyaFighterBlow& Blow : Actor.FighterBlows)
    {
        if (Blow.Stages.Num() != 3)
        {
            OutError = FString::Printf(TEXT("Fighter blow '%s' must have exactly three stages."), *Blow.Name);
            return false;
        }
        AppendAlignedString(Out, AbsoluteStart, Blow.Name);
        const int32 FixedStart = Out.Num();
        Out.AddZeroed(64); // fixed blow fields, filled by the exporter
        WriteU32(Out, FixedStart + 60, uint32(Blow.SubObjs.Num()));
        Out.AddZeroed(Blow.SubObjs.Num() * 28);
        Out.AddZeroed(8 + 4 + 84); // bone refs, activation range, three stages
    }

    AppendU32(Out, uint32(Actor.FighterGrabs.Num()));
    for (const FKyaFighterGrab& Grab : Actor.FighterGrabs)
    {
        if (Grab.Stages.Num() != 5)
        {
            OutError = FString::Printf(TEXT("Fighter grab '%s' must have exactly five stages."), *Grab.Name);
            return false;
        }
        AppendAlignedString(Out, AbsoluteStart, Grab.Name);
        Out.AddZeroed(232);
    }

    AppendU32(Out, uint32(Actor.FighterGrabReacts.Num()));
    for (const FKyaFighterGrabReact& React : Actor.FighterGrabReacts)
    {
        AppendAlignedString(Out, AbsoluteStart, React.Name);
        Out.AddZeroed(8);
    }

    AppendU32(Out, uint32(Actor.FighterCombos.Num()));
    for (const FKyaFighterCombo& Combo : Actor.FighterCombos)
    {
        AppendAlignedString(Out, AbsoluteStart, Combo.Name);
        const int32 FixedStart = Out.Num();
        Out.AddZeroed(16);
        WriteU32(Out, FixedStart + 12, uint32(Combo.Inputs.Num()));
        Out.AddZeroed(Combo.Inputs.Num() * 4);
        Out.AddZeroed(12);
        AppendAlignedString(Out, AbsoluteStart, Combo.Action);
        AppendU32(Out, uint32(Combo.Branches.Num()));
        for (const FString& Branch : Combo.Branches) AppendAlignedString(Out, AbsoluteStart, Branch);
    }
    AppendU32(Out, 0); // weapon actor index, filled by the exporter
    return true;
}

bool NeedsWolfenFightRebuild(const FKyaActorPlacement& Actor, TConstArrayView<uint8> ClassData)
{
    if (Actor.ClassId != 0x09 || !Actor.bHasWolfenData) return false;
    FKyaWolfenOffsets Wolfen;
    if (!KyaReadWolfenOffsets(ClassData, 0, ClassData.Num(), Wolfen)) return true;
    FKyaWolfenFightParamOffsets Fight;
    if (!KyaReadWolfenFightParamOffsets(ClassData, Wolfen.FightParam, Wolfen.Tail, Fight)) return true;
    const auto U32At = [ClassData](int64 Offset)
    {
        const uint8* P = ClassData.GetData() + Offset;
        return uint32(P[0]) | (uint32(P[1]) << 8) | (uint32(P[2]) << 16) | (uint32(P[3]) << 24);
    };
    if (U32At(Fight.ComboList1Count) != uint32(Actor.WolfenFightParamData.ComboList1.Num()) ||
        U32At(Fight.ComboList2Count) != uint32(Actor.WolfenFightParamData.ComboList2.Num()))
        return true;
    if (Actor.WolfenFightParamData.Capabilities.Num() != 3) return true;
    for (int32 Cap = 0; Cap < 3; ++Cap)
    {
        const FKyaWolfenCapability& Capability = Actor.WolfenFightParamData.Capabilities[Cap];
        if (Capability.Items.Num() != Fight.Capabilities[Cap].Items.Num()) return true;
        for (int32 Item = 0; Item < Capability.Items.Num(); ++Item)
            if (Capability.Items[Item].Chains.Num() != Fight.Capabilities[Cap].Items[Item].ChainCount)
                return true;
    }
    return false;
}

bool BuildWolfenTailLayout(const FKyaActorPlacement& Actor, TConstArrayView<uint8> Original,
    const FKyaWolfenOffsets& Wolfen, TArray<uint8>& Out, FString& OutError)
{
    Out.Reset();
    if (Wolfen.WolfenStart < 0 || Wolfen.FightParam < Wolfen.WolfenStart ||
        Wolfen.FightParam > Original.Num())
    {
        OutError = TEXT("The Wolfen fight-parameter offsets are invalid.");
        return false;
    }
    Out.Append(Original.GetData() + Wolfen.WolfenStart, int32(Wolfen.FightParam - Wolfen.WolfenStart));

    if (Actor.WolfenFightParamData.Capabilities.Num() != 3)
    {
        OutError = TEXT("Wolfen fight parameters must contain exactly three capabilities.");
        return false;
    }
    Out.AddZeroed(20 + 20 + 16); // 5+5 capability fields and four configuration floats
    for (const FKyaWolfenCapability& Capability : Actor.WolfenFightParamData.Capabilities)
    {
        AppendU32(Out, uint32(Capability.Items.Num()));
        for (const FKyaWolfenCapabilityItem& Item : Capability.Items)
        {
            AppendU32(Out, uint32(Item.Chains.Num()));
            for (const int32 Chain : Item.Chains) AppendI32(Out, Chain);
            Out.AddZeroed(8); // item floats, filled by the exporter
        }
    }
    AppendU32(Out, uint32(Actor.WolfenFightParamData.ComboList1.Num()));
    for (const int32 Value : Actor.WolfenFightParamData.ComboList1) AppendI32(Out, Value);
    AppendU32(Out, uint32(Actor.WolfenFightParamData.ComboList2.Num()));
    for (const int32 Value : Actor.WolfenFightParamData.ComboList2) AppendI32(Out, Value);
    Out.AddZeroed(48); // two words, six tuning floats, four knowledge words
    return true;
}

bool BuildFighterClassData(const FKyaActorPlacement& Actor, TConstArrayView<uint8> Original,
    int64 AbsoluteStart, TArray<uint8>& Out, FString& OutError)
{
    FKyaFighterPrefixOffsets Prefix;
    if (!KyaReadFighterPrefixOffsets(Original, 0, Original.Num(), Prefix))
    {
        OutError = TEXT("The fighter prefix could not be parsed.");
        return false;
    }
    if (!BuildFighterPrefixLayout(Actor, AbsoluteStart, Out, OutError)) return false;
    if (Actor.ClassId == 0x09 && NeedsWolfenFightRebuild(Actor, Original))
    {
        FKyaWolfenOffsets Wolfen;
        if (!KyaReadWolfenOffsets(Original, 0, Original.Num(), Wolfen))
        {
            OutError = TEXT("The original Wolfen class data could not be parsed.");
            return false;
        }
        TArray<uint8> WolfenTail;
        if (!BuildWolfenTailLayout(Actor, Original, Wolfen, WolfenTail, OutError)) return false;
        Out.Append(WolfenTail);
    }
    else if (Prefix.Tail < Original.Num())
    {
        Out.Append(Original.GetData() + Prefix.Tail, int32(Original.Num() - Prefix.Tail));
    }

    // Validate the new prefix and the preserved Wolfen/Kim tail before export.
    FKyaFighterPrefixOffsets NewPrefix;
    if (!KyaReadFighterPrefixOffsets(MakeArrayView(Out.GetData(), Out.Num()), 0, Out.Num(), NewPrefix))
    {
        OutError = TEXT("The rebuilt fighter prefix could not be parsed.");
        return false;
    }
    if (Actor.ClassId == 0x09)
    {
        FKyaWolfenOffsets Wolfen;
        if (!KyaReadWolfenOffsets(MakeArrayView(Out.GetData(), Out.Num()), 0, Out.Num(), Wolfen))
        {
            OutError = TEXT("The rebuilt Wolfen class data could not be parsed.");
            return false;
        }
    }
    else if (Actor.ClassId == 0x06)
    {
        FKyaKimOffsets Kim;
        if (!KyaReadKimOffsets(MakeArrayView(Out.GetData(), Out.Num()), 0, Out.Num(), Kim))
        {
            OutError = TEXT("The rebuilt Kim class data could not be parsed.");
            return false;
        }
    }
    return true;
}

bool NeedsShockerCmdClassDataRebuild(const FKyaActorPlacement& Actor, TConstArrayView<uint8> ClassData)
{
    if (Actor.ClassId != 0x50 || !Actor.bHasShockerCmdData) return false;
    FKyaShockerCmdOffsets Offsets;
    if (!KyaReadShockerCmdOffsets(ClassData, 0, ClassData.Num(), Offsets)) return true;
    if (Actor.ShockerCmdSequences.Num() != Offsets.Sequences.Num()) return true;
    for (int32 Index = 0; Index < Actor.ShockerCmdSequences.Num(); ++Index)
        if (Actor.ShockerCmdSequences[Index].Values.Num() != Offsets.Counts[Index]) return true;
    return false;
}

bool BuildShockerCmdClassData(const FKyaActorPlacement& Actor, TArray<uint8>& Out, FString& OutError)
{
    Out.Reset();
    if (!Actor.bHasShockerCmdData)
    {
        OutError = TEXT("The shocker-command actor is missing its data flag.");
        return false;
    }
    AppendU32(Out, uint32(Actor.ShockerCmdField168));
    AppendU32(Out, uint32(Actor.ShockerCmdSequences.Num()));
    for (const FKyaShockerCmdSequence& Sequence : Actor.ShockerCmdSequences)
    {
        AppendI32(Out, Sequence.ActorRef);
        AppendU32(Out, uint32(Sequence.Values.Num()));
        for (const float Value : Sequence.Values) AppendF32(Out, Value);
    }
    if (!Actor.ClassData.IsEmpty()) Out.Append(Actor.ClassData);
    return true;
}

bool NeedsBasicBoxClassDataRebuild(const FKyaActorPlacement& Actor, TConstArrayView<uint8> ClassData)
{
    if (Actor.ClassId != 0x39 || !Actor.bHasBasicBoxData) return false;
    FKyaBasicBoxOffsets Offsets;
    if (!KyaReadBasicBoxOffsets(ClassData, 0, ClassData.Num(), Offsets)) return true;
    return Actor.BasicBoxActorRefs.Num() != Offsets.ActorCount;
}

bool BuildBasicBoxClassData(const FKyaActorPlacement& Actor, TConstArrayView<uint8> Original,
    TArray<uint8>& Out, FString& OutError)
{
    Out.Reset();
    FKyaBasicBoxOffsets Offsets;
    if (!KyaReadBasicBoxOffsets(Original, 0, Original.Num(), Offsets))
    {
        OutError = TEXT("The basic-box class data could not be parsed.");
        return false;
    }
    // Keep the fixed prefix (field 0x160, life maximum) and splice a new actor
    // stream in place of the old one. Everything after the stream is fixed length
    // for the existing sub-object layout, so it moves verbatim.
    Out.Append(Original.GetData(), int32(Offsets.Actors));
    WriteU32(Out, Offsets.Actors - 4, uint32(Actor.BasicBoxActorRefs.Num()));
    for (const int32 ActorRef : Actor.BasicBoxActorRefs) AppendI32(Out, ActorRef);
    Out.Append(Original.GetData() + Offsets.Vibration, int32(Offsets.Tail - Offsets.Vibration));
    if (!Actor.ClassData.IsEmpty()) Out.Append(Actor.ClassData);

    FKyaBasicBoxOffsets Rebuilt;
    if (!KyaReadBasicBoxOffsets(MakeArrayView(Out.GetData(), Out.Num()), 0, Out.Num(), Rebuilt) ||
        Rebuilt.ActorCount != Actor.BasicBoxActorRefs.Num() ||
        Rebuilt.SwitchTargetCount != Actor.BasicBoxSwitch.Targets.Num() ||
        Rebuilt.SubObjModeValue != Actor.BasicBoxAddOn.SubObjMode)
    {
        OutError = TEXT("The rebuilt basic-box class data is inconsistent.");
        return false;
    }
    return true;
}

/** Append one CCamera record in the layout KyaReadActorCameraOffsets expects. */
void AppendKimCamera(const FKyaActorCamera& Camera, TArray<uint8>& Out)
{
    AppendF32(Out, Camera.Fov);
    for (int32 Axis = 0; Axis < 3; ++Axis) AppendF32(Out, float(Camera.Position[Axis]));
    for (int32 Axis = 0; Axis < 3; ++Axis) AppendF32(Out, float(Camera.LookAt[Axis]));
    for (int32 Axis = 0; Axis < 3; ++Axis) AppendF32(Out, float(Camera.Up[Axis]));
    AppendI32(Out, Camera.SectorId);
    AppendI32(Out, Camera.Field08);
    AppendI32(Out, Camera.Flags);
    for (int32 Axis = 0; Axis < 3; ++Axis) AppendF32(Out, float(Camera.TargetOffset[Axis]));
    if (Camera.bHasSwitch)
    {
        AppendI32(Out, Camera.SwitchActor);
        AppendU32(Out, uint32(Camera.SwitchActorsA.Num()));
        for (const int32 Item : Camera.SwitchActorsA) AppendI32(Out, Item);
        AppendU32(Out, uint32(Camera.SwitchActorsB.Num()));
        for (const int32 Item : Camera.SwitchActorsB) AppendI32(Out, Item);
        AppendF32(Out, Camera.SwitchValueA);
        AppendI32(Out, Camera.SwitchModeA);
        AppendF32(Out, Camera.SwitchValueB);
        AppendI32(Out, Camera.SwitchModeB);
        AppendF32(Out, Camera.SwitchValueC);
    }
    AppendU32(Out, uint32(Camera.Conditions.Num()));
    for (const FKyaCameraCondition& Condition : Camera.Conditions)
    {
        AppendI32(Out, Condition.Combiner);
        AppendI32(Out, Condition.ActorRef);
        AppendI32(Out, Condition.Type);
        AppendF32(Out, Condition.Value);
    }
    AppendI32(Out, Camera.ConfigFlags);
    AppendI32(Out, Camera.ConfigActorRefA);
    AppendI32(Out, Camera.ConfigActorRefB);
    AppendF32(Out, Camera.ConfigAngle);
    const bool bOptionalConfig = Camera.ConfigOptionalValues.Num() == 5;
    AppendU32(Out, bOptionalConfig ? 1u : 0u);
    if (bOptionalConfig)
        for (const float Value : Camera.ConfigOptionalValues) AppendF32(Out, Value);
    for (int32 Axis = 0; Axis < 3; ++Axis) AppendF32(Out, float(Camera.PitchAngles[Axis]));
    for (int32 Axis = 0; Axis < 3; ++Axis) AppendF32(Out, float(Camera.OtherAngles[Axis]));
    for (int32 Axis = 0; Axis < 3; ++Axis) AppendF32(Out, float(Camera.Distances[Axis]));
    const bool bExtra = Camera.ExtraValues.Num() == 12;
    AppendU32(Out, bExtra ? 1u : 0u);
    if (bExtra)
        for (const float Value : Camera.ExtraValues) AppendF32(Out, Value);
}

bool NeedsPatternCmdClassDataRebuild(const FKyaActorPlacement& Actor, TConstArrayView<uint8> ClassData)
{
    if (Actor.ClassId != 0x40 || !Actor.bHasPatternCmdData) return false;
    FKyaPatternCmdOffsets Offsets;
    if (!KyaReadPatternCmdOffsets(ClassData, 0, ClassData.Num(), Offsets)) return true;
    return Actor.PatternCmdActorRefs.Num() != Offsets.Count;
}

bool BuildPatternCmdClassData(const FKyaActorPlacement& Actor, TConstArrayView<uint8> Original,
    TArray<uint8>& Out, FString& OutError)
{
    Out.Reset();
    FKyaPatternCmdOffsets Offsets;
    if (!KyaReadPatternCmdOffsets(Original, 0, Original.Num(), Offsets))
    {
        OutError = TEXT("The pattern-command class data could not be parsed.");
        return false;
    }
    if (Actor.PatternCmdPattern.bHasField24 != Offsets.bHasField24)
    {
        OutError = TEXT("The pattern-command CPatternPart version cannot be changed.");
        return false;
    }
    // Keep the two leading words, splice a new actor-reference stream, and copy the
    // trailing value/pattern bytes verbatim.
    Out.Append(Original.GetData(), 8);
    AppendU32(Out, uint32(Actor.PatternCmdActorRefs.Num()));
    for (const int32 ActorRef : Actor.PatternCmdActorRefs) AppendI32(Out, ActorRef);
    Out.Append(Original.GetData() + Offsets.Values, int32(Offsets.Tail - Offsets.Values));
    if (!Actor.ClassData.IsEmpty()) Out.Append(Actor.ClassData);

    FKyaPatternCmdOffsets Rebuilt;
    if (!KyaReadPatternCmdOffsets(MakeArrayView(Out.GetData(), Out.Num()), 0, Out.Num(), Rebuilt) ||
        Rebuilt.Count != Actor.PatternCmdActorRefs.Num() ||
        Rebuilt.bHasField24 != Offsets.bHasField24)
    {
        OutError = TEXT("The rebuilt pattern-command class data is inconsistent.");
        return false;
    }
    return true;
}

bool NeedsWindClassDataRebuild(const FKyaActorPlacement& Actor, TConstArrayView<uint8> ClassData)
{
    if (Actor.ClassId != 0x12 || !Actor.bHasWindData) return false;
    FKyaWindOffsets Offsets;
    if (!KyaReadWindOffsets(ClassData, 0, ClassData.Num(), Offsets)) return true;
    if (Actor.WindActorRefs.Num() != Offsets.ActorCount) return true;
    const FKyaSwitch* Switches[] = { &Actor.WindSwitch1D0, &Actor.WindSwitch1D8,
        &Actor.WindSwitch1E0 };
    if (!Offsets.bHasSwitches)
    {
        // A pre-2.2 level has no switch records; adding targets must fail rather
        // than silently change the scene version.
        for (int32 Switch = 0; Switch < 3; ++Switch)
            if (!Switches[Switch]->Targets.IsEmpty()) return true;
        return false;
    }
    for (int32 Switch = 0; Switch < 3; ++Switch)
        if (Switches[Switch]->Targets.Num() != Offsets.Switches[Switch].Count) return true;
    return false;
}

bool BuildWindClassData(const FKyaActorPlacement& Actor, TConstArrayView<uint8> Original,
    TArray<uint8>& Out, FString& OutError)
{
    Out.Reset();
    FKyaWindOffsets Offsets;
    if (!KyaReadWindOffsets(Original, 0, Original.Num(), Offsets))
    {
        OutError = TEXT("The Wind class data could not be parsed.");
        return false;
    }
    // Keep the fixed prefix, splice a new carried-actor stream and copy the rest.
    const int64 CountOffset = Offsets.Actors - 4;
    Out.Append(Original.GetData(), int32(CountOffset));
    AppendU32(Out, uint32(Actor.WindActorRefs.Num()));
    for (const int32 ActorRef : Actor.WindActorRefs) AppendI32(Out, ActorRef);
    const int64 OldActorEnd = Offsets.Actors + int64(Offsets.ActorCount) * 4;
    if (!Offsets.bHasSwitches)
    {
        const FKyaSwitch* Switches[] = { &Actor.WindSwitch1D0, &Actor.WindSwitch1D8,
            &Actor.WindSwitch1E0 };
        for (int32 Switch = 0; Switch < 3; ++Switch)
            if (!Switches[Switch]->Targets.IsEmpty())
            {
                OutError = TEXT("This Wind actor's level has no switch records; switch targets "
                    "cannot be added without changing the scene version.");
                return false;
            }
        Out.Append(Original.GetData() + OldActorEnd, int32(Offsets.Tail - OldActorEnd));
    }
    else
    {
        const int64 SwitchStart = Offsets.Switches[0].Targets - 4;
        Out.Append(Original.GetData() + OldActorEnd, int32(SwitchStart - OldActorEnd));
        const FKyaSwitch* Switches[] = { &Actor.WindSwitch1D0, &Actor.WindSwitch1D8,
            &Actor.WindSwitch1E0 };
        for (int32 Switch = 0; Switch < 3; ++Switch)
        {
            const FKyaSwitch& Value = *Switches[Switch];
            AppendU32(Out, uint32(Value.Targets.Num()));
            for (const FKyaSwitchTarget& Target : Value.Targets)
            {
                AppendI32(Out, Target.TargetRef);
                AppendI32(Out, Target.CutsceneId);
                AppendI32(Out, Target.Flags);
                AppendI32(Out, Target.MessageId);
                AppendI32(Out, Target.MessageFlags);
                AppendI32(Out, Target.Field14);
                AppendI32(Out, Target.Field18);
            }
            const FKyaStreamEventCamera& Camera = Value.Camera;
            AppendI32(Out, Camera.CameraIndex);
            AppendI32(Out, Camera.Flags);
            AppendF32(Out, Camera.Field8);
            AppendF32(Out, Camera.FieldC);
            AppendF32(Out, Camera.Field10);
            AppendI32(Out, Camera.ActorRef);
            AppendI32(Out, Camera.ZoneRef);
            AppendF32(Out, Camera.Field1C);
        }
        Out.Append(Original.GetData() + Offsets.Floats194, int32(Offsets.Tail - Offsets.Floats194));
    }
    if (!Actor.ClassData.IsEmpty()) Out.Append(Actor.ClassData);

    FKyaWindOffsets Rebuilt;
    if (!KyaReadWindOffsets(MakeArrayView(Out.GetData(), Out.Num()), 0, Out.Num(), Rebuilt) ||
        Rebuilt.ActorCount != Actor.WindActorRefs.Num() ||
        Rebuilt.bHasSwitches != Offsets.bHasSwitches)
    {
        OutError = TEXT("The rebuilt Wind class data is inconsistent.");
        return false;
    }
    if (Rebuilt.bHasSwitches)
    {
        const FKyaSwitch* Switches[] = { &Actor.WindSwitch1D0, &Actor.WindSwitch1D8,
            &Actor.WindSwitch1E0 };
        for (int32 Switch = 0; Switch < 3; ++Switch)
            if (Rebuilt.Switches[Switch].Count != Switches[Switch]->Targets.Num())
            {
                OutError = TEXT("The rebuilt Wind switch counts are inconsistent.");
                return false;
            }
    }
    return true;
}

bool NeedsShockerClassDataRebuild(const FKyaActorPlacement& Actor, TConstArrayView<uint8> ClassData)
{
    if (Actor.ClassId != 0x4b || !Actor.bHasShockerData) return false;
    FKyaShockerOffsets Offsets;
    if (!KyaReadShockerOffsets(ClassData, 0, ClassData.Num(), Offsets)) return true;
    return Actor.ShockerSwitch.Targets.Num() != Offsets.SwitchCount ||
        Actor.ShockerAddOn.SubObjMode != Offsets.SubObjModeValue;
}

bool BuildShockerClassData(const FKyaActorPlacement& Actor, TConstArrayView<uint8> Original,
    TArray<uint8>& Out, FString& OutError)
{
    Out.Reset();
    FKyaShockerOffsets Offsets;
    if (!KyaReadShockerOffsets(Original, 0, Original.Num(), Offsets))
    {
        OutError = TEXT("The Shocker class data could not be parsed.");
        return false;
    }
    const int64 CountOffset = Offsets.SwitchTargets - 4;
    Out.Append(Original.GetData(), int32(CountOffset));
    AppendU32(Out, uint32(Actor.ShockerSwitch.Targets.Num()));
    for (const FKyaSwitchTarget& Target : Actor.ShockerSwitch.Targets)
    {
        AppendI32(Out, Target.TargetRef);
        AppendI32(Out, Target.CutsceneId);
        AppendI32(Out, Target.Flags);
        AppendI32(Out, Target.MessageId);
        AppendI32(Out, Target.MessageFlags);
        AppendI32(Out, Target.Field14);
        AppendI32(Out, Target.Field18);
    }
    const FKyaStreamEventCamera& Camera = Actor.ShockerSwitch.Camera;
    AppendI32(Out, Camera.CameraIndex);
    AppendI32(Out, Camera.Flags);
    AppendF32(Out, Camera.Field8);
    AppendF32(Out, Camera.FieldC);
    AppendF32(Out, Camera.Field10);
    AppendI32(Out, Camera.ActorRef);
    AppendI32(Out, Camera.ZoneRef);
    AppendF32(Out, Camera.Field1C);
    Out.Append(Original.GetData() + Offsets.Field380, int32(Offsets.Tail - Offsets.Field380));
    if (!Actor.ClassData.IsEmpty()) Out.Append(Actor.ClassData);

    FKyaShockerOffsets Rebuilt;
    if (!KyaReadShockerOffsets(MakeArrayView(Out.GetData(), Out.Num()), 0, Out.Num(), Rebuilt) ||
        Rebuilt.SwitchCount != Actor.ShockerSwitch.Targets.Num() ||
        Rebuilt.SubObjModeValue != Actor.ShockerAddOn.SubObjMode)
    {
        OutError = TEXT("The rebuilt Shocker class data is inconsistent.");
        return false;
    }
    return true;
}

bool NeedsAtonClassDataRebuild(const FKyaActorPlacement& Actor, TConstArrayView<uint8> ClassData)
{
    if (Actor.ClassId != 0x38 || !Actor.bHasAtonData) return false;
    FKyaAtonOffsets Offsets;
    if (!KyaReadAtonOffsets(ClassData, 0, ClassData.Num(), Offsets)) return true;
    if (Actor.AtonPathIndices.Num() != Offsets.PathCountValue ||
        Actor.AtonOtherSubObjs.Num() != Offsets.OtherCountValue ||
        Actor.AtonSubObjs.Num() != Offsets.AtonCountValue) return true;
    // Nested cinematic lists also need the rebuild.
    for (int32 Index = 0; Index < Actor.AtonOtherSubObjs.Num(); ++Index)
        if (Actor.AtonOtherSubObjs[Index].CinematicIds.Num() !=
            int32(KyaReadU32(ClassData.GetData() + Offsets.OtherOffsets[Index] + 4))) return true;
    for (int32 Index = 0; Index < Actor.AtonSubObjs.Num(); ++Index)
        if (Actor.AtonSubObjs[Index].CinematicIds.Num() !=
            int32(KyaReadU32(ClassData.GetData() + Offsets.AtonOffsets[Index] + 8))) return true;
    return false;
}

void AppendAtonSubObj(const int32 Field0, const TArray<int32>& CinematicIds, TArray<uint8>& Out)
{
    AppendI32(Out, Field0);
    AppendU32(Out, uint32(CinematicIds.Num()));
    for (const int32 CinematicId : CinematicIds) AppendI32(Out, CinematicId);
}

bool BuildAtonClassData(const FKyaActorPlacement& Actor, TConstArrayView<uint8> Original,
    TArray<uint8>& Out, FString& OutError)
{
    Out.Reset();
    FKyaAtonOffsets Offsets;
    if (!KyaReadAtonOffsets(Original, 0, Original.Num(), Offsets))
    {
        OutError = TEXT("The Aton class data could not be parsed.");
        return false;
    }
    // Keep the autonomous prefix and path default delay, splice the path-plane
    // list, then regenerate the add-on sub-object lists and the trailing float.
    Out.Append(Original.GetData(), int32(Offsets.PathCount));
    AppendU32(Out, uint32(Actor.AtonPathIndices.Num()));
    for (const int32 PathIndex : Actor.AtonPathIndices) AppendI32(Out, PathIndex);
    AppendI32(Out, Actor.AtonAddOnField18);
    AppendU32(Out, uint32(Actor.AtonOtherSubObjs.Num()));
    for (const FKyaAddOnSubObj& Sub : Actor.AtonOtherSubObjs)
        AppendAtonSubObj(Sub.Field0, Sub.CinematicIds, Out);
    AppendU32(Out, uint32(Actor.AtonSubObjs.Num()));
    for (const FKyaAtonSubObj& Sub : Actor.AtonSubObjs)
    {
        AppendI32(Out, Sub.Field18);
        AppendAtonSubObj(Sub.Field0, Sub.CinematicIds, Out);
    }
    AppendF32(Out, Actor.AtonAddOnField24);
    if (!Actor.ClassData.IsEmpty()) Out.Append(Actor.ClassData);

    FKyaAtonOffsets Rebuilt;
    if (!KyaReadAtonOffsets(MakeArrayView(Out.GetData(), Out.Num()), 0, Out.Num(), Rebuilt) ||
        Rebuilt.PathCountValue != Actor.AtonPathIndices.Num() ||
        Rebuilt.OtherCountValue != Actor.AtonOtherSubObjs.Num() ||
        Rebuilt.AtonCountValue != Actor.AtonSubObjs.Num())
    {
        OutError = TEXT("The rebuilt Aton class data is inconsistent.");
        return false;
    }
    return true;
}

bool NeedsTeleporterClassDataRebuild(const FKyaActorPlacement& Actor, TConstArrayView<uint8> ClassData)
{
    if (Actor.ClassId != 0x24 || !Actor.bHasTeleporterData) return false;
    FKyaTeleporterOffsets Offsets;
    if (!KyaReadTeleporterOffsets(ClassData, 0, ClassData.Num(), Offsets)) return true;
    if (Actor.TeleporterDestinations.Num() != Offsets.DestCountValue ||
        Actor.TeleporterWaypoints.Num() != Offsets.WaypointCountValue ||
        Actor.TeleporterConditionedOperations.Num() != Offsets.BlockOps.Num()) return true;
    for (int32 Index = 0; Index < Actor.TeleporterConditionedOperations.Num(); ++Index)
    {
        if (Actor.TeleporterConditionedOperations[Index].Operations.Num() !=
                Offsets.BlockOpCountValues[Index] ||
            Actor.TeleporterConditionedOperations[Index].Conditions.Num() !=
                Offsets.BlockCondCountValues[Index]) return true;
    }
    return false;
}

bool BuildTeleporterClassData(const FKyaActorPlacement& Actor, TConstArrayView<uint8> Original,
    TArray<uint8>& Out, FString& OutError)
{
    Out.Reset();
    FKyaTeleporterOffsets Offsets;
    if (!KyaReadTeleporterOffsets(Original, 0, Original.Num(), Offsets))
    {
        OutError = TEXT("The Teleporter class data could not be parsed.");
        return false;
    }
    // Regenerate the destination list, copy the switch records and fixed middle
    // block, then regenerate the waypoint list and conditioned-operation blocks.
    Out.Append(Original.GetData(), int32(Offsets.DestCount));
    AppendU32(Out, uint32(Actor.TeleporterDestinations.Num()));
    for (const FKyaTeleporterDestination& Destination : Actor.TeleporterDestinations)
    {
        AppendI32(Out, Destination.LevelId);
        AppendI32(Out, Destination.ElevatorId);
        AppendI32(Out, Destination.Field8);
    }
    const int64 MiddleStart = Offsets.Switches[0].Targets - 4;
    Out.Append(Original.GetData() + MiddleStart, int32(Offsets.WaypointCount - MiddleStart));
    AppendU32(Out, uint32(Actor.TeleporterWaypoints.Num()));
    for (const FKyaTeleporterWaypoint& Waypoint : Actor.TeleporterWaypoints)
    {
        AppendI32(Out, Waypoint.ActorRef);
        AppendI32(Out, Waypoint.WaypointRef);
    }
    AppendU32(Out, uint32(Actor.TeleporterConditionedOperations.Num()));
    for (const FKyaTeleporterBlock& Block : Actor.TeleporterConditionedOperations)
    {
        AppendU32(Out, uint32(Block.Operations.Num()));
        for (const FKyaTeleporterOperation& Operation : Block.Operations)
        {
            AppendI32(Out, Operation.ScenaricVariable);
            AppendI32(Out, Operation.Operation);
            AppendI32(Out, Operation.Value);
        }
        AppendU32(Out, uint32(Block.Conditions.Num()));
        for (const FKyaTeleporterCondition& Condition : Block.Conditions)
        {
            AppendI32(Out, Condition.Combine);
            AppendI32(Out, Condition.Variable);
            AppendI32(Out, Condition.Comparison);
            AppendI32(Out, Condition.Value);
        }
    }
    if (!Actor.ClassData.IsEmpty()) Out.Append(Actor.ClassData);

    FKyaTeleporterOffsets Rebuilt;
    if (!KyaReadTeleporterOffsets(MakeArrayView(Out.GetData(), Out.Num()), 0, Out.Num(), Rebuilt) ||
        Rebuilt.DestCountValue != Actor.TeleporterDestinations.Num() ||
        Rebuilt.WaypointCountValue != Actor.TeleporterWaypoints.Num() ||
        Rebuilt.BlockOps.Num() != Actor.TeleporterConditionedOperations.Num())
    {
        OutError = TEXT("The rebuilt Teleporter class data is inconsistent.");
        return false;
    }
    for (int32 Index = 0; Index < Actor.TeleporterConditionedOperations.Num(); ++Index)
    {
        if (Rebuilt.BlockOpCountValues[Index] !=
                Actor.TeleporterConditionedOperations[Index].Operations.Num() ||
            Rebuilt.BlockCondCountValues[Index] !=
                Actor.TeleporterConditionedOperations[Index].Conditions.Num())
        {
            OutError = TEXT("The rebuilt Teleporter conditioned operations are inconsistent.");
            return false;
        }
    }
    return true;
}

bool NeedsEggClassDataRebuild(const FKyaActorPlacement& Actor, TConstArrayView<uint8> ClassData)
{
    if (Actor.ClassId != 0x2f || !Actor.bHasEggData) return false;
    FKyaEggOffsets Offsets;
    if (!KyaReadEggOffsets(ClassData, 0, ClassData.Num(), Offsets)) return true;
    return Actor.EggPathIndices.Num() != Offsets.Count;
}

bool BuildEggClassData(const FKyaActorPlacement& Actor, TConstArrayView<uint8> Original,
    TArray<uint8>& Out, FString& OutError)
{
    Out.Reset();
    FKyaEggOffsets Offsets;
    if (!KyaReadEggOffsets(Original, 0, Original.Num(), Offsets))
    {
        OutError = TEXT("The Egg class data could not be parsed.");
        return false;
    }
    // Weight and field 1D8, then the path-plane count/entries, the two trailing
    // fields and raw tail.
    Out.Append(Original.GetData(), 8);
    AppendU32(Out, uint32(Actor.EggPathIndices.Num()));
    for (const int32 PathIndex : Actor.EggPathIndices) AppendI32(Out, PathIndex);
    Out.Append(Original.GetData() + Offsets.Fields, int32(Offsets.Tail - Offsets.Fields));
    if (!Actor.ClassData.IsEmpty()) Out.Append(Actor.ClassData);

    FKyaEggOffsets Rebuilt;
    if (!KyaReadEggOffsets(MakeArrayView(Out.GetData(), Out.Num()), 0, Out.Num(), Rebuilt) ||
        Rebuilt.Count != Actor.EggPathIndices.Num())
    {
        OutError = TEXT("The rebuilt Egg class data is inconsistent.");
        return false;
    }
    return true;
}

bool NeedsBlazerClassDataRebuild(const FKyaActorPlacement& Actor, TConstArrayView<uint8> ClassData)
{
    if (Actor.ClassId != 0x4d || !Actor.bHasBlazerData) return false;
    FKyaBlazerOffsets Offsets;
    if (!KyaReadBlazerOffsets(ClassData, 0, ClassData.Num(), Offsets)) return true;
    return Actor.BlazerData.ActorRefs.Num() != Offsets.Count;
}

bool BuildBlazerClassData(const FKyaActorPlacement& Actor, TConstArrayView<uint8> Original,
    TArray<uint8>& Out, FString& OutError)
{
    Out.Reset();
    FKyaBlazerOffsets Offsets;
    if (!KyaReadBlazerOffsets(Original, 0, Original.Num(), Offsets))
    {
        OutError = TEXT("The Blazer class data could not be parsed.");
        return false;
    }
    // The actor stream is a count followed by 4-byte refs inside a 32-byte
    // suffix block; keep everything before the count and the tilt block verbatim.
    Out.Append(Original.GetData(), int32(Offsets.Entries - 4));
    AppendU32(Out, uint32(Actor.BlazerData.ActorRefs.Num()));
    for (const int32 ActorRef : Actor.BlazerData.ActorRefs) AppendI32(Out, ActorRef);
    Out.Append(Original.GetData() + Offsets.Tilt, int32(Offsets.Tail - Offsets.Tilt));
    if (!Actor.ClassData.IsEmpty()) Out.Append(Actor.ClassData);

    FKyaBlazerOffsets Rebuilt;
    if (!KyaReadBlazerOffsets(MakeArrayView(Out.GetData(), Out.Num()), 0, Out.Num(), Rebuilt) ||
        Rebuilt.Count != Actor.BlazerData.ActorRefs.Num() ||
        Rebuilt.bHasWaypointRefs != Offsets.bHasWaypointRefs)
    {
        OutError = TEXT("The rebuilt Blazer class data is inconsistent.");
        return false;
    }
    return true;
}

bool NeedsDcaClassDataRebuild(const FKyaActorPlacement& Actor, TConstArrayView<uint8> ClassData)
{
    if (Actor.ClassId != 0x1b || !Actor.bHasDcaData) return false;
    FKyaDcaOffsets Offsets;
    if (!KyaReadDcaOffsets(ClassData, 0, ClassData.Num(), Offsets)) return true;
    return Actor.DcaShotActorRefs.Num() != Offsets.ShotCountValue;
}

bool BuildDcaClassData(const FKyaActorPlacement& Actor, TConstArrayView<uint8> Original,
    TArray<uint8>& Out, FString& OutError)
{
    Out.Reset();
    FKyaDcaOffsets Offsets;
    if (!KyaReadDcaOffsets(Original, 0, Original.Num(), Offsets))
    {
        OutError = TEXT("The DCA class data could not be parsed.");
        return false;
    }
    // Keep the 24-byte prefix (four stream refs, float, aim angle), splice a new
    // shot actor stream, then copy the two trailing words and raw tail.
    Out.Append(Original.GetData(), 24);
    AppendU32(Out, uint32(Actor.DcaShotActorRefs.Num()));
    for (const int32 ActorRef : Actor.DcaShotActorRefs) AppendI32(Out, ActorRef);
    Out.Append(Original.GetData() + Offsets.Field450, int32(Offsets.Tail - Offsets.Field450));
    if (!Actor.ClassData.IsEmpty()) Out.Append(Actor.ClassData);

    FKyaDcaOffsets Rebuilt;
    if (!KyaReadDcaOffsets(MakeArrayView(Out.GetData(), Out.Num()), 0, Out.Num(), Rebuilt) ||
        Rebuilt.ShotCountValue != Actor.DcaShotActorRefs.Num())
    {
        OutError = TEXT("The rebuilt DCA class data is inconsistent.");
        return false;
    }
    return true;
}

bool NeedsWoofClassDataRebuild(const FKyaActorPlacement& Actor, TConstArrayView<uint8> ClassData)
{
    if (Actor.ClassId != 0x2a || !Actor.bHasWoofData) return false;
    FKyaWoofOffsets Offsets;
    if (!KyaReadWoofOffsets(ClassData, 0, ClassData.Num(), Offsets)) return true;
    return Actor.WoofData.ActorRefs.Num() != Offsets.Count;
}

bool BuildWoofClassData(const FKyaActorPlacement& Actor, TConstArrayView<uint8> Original,
    TArray<uint8>& Out, FString& OutError)
{
    Out.Reset();
    FKyaWoofOffsets Offsets;
    if (!KyaReadWoofOffsets(Original, 0, Original.Num(), Offsets))
    {
        OutError = TEXT("The Woof class data could not be parsed.");
        return false;
    }
    // The actor stream is a count at offset 56 followed by 4-byte refs; keep the
    // 56-byte prefix and 36-byte suffix verbatim.
    Out.Append(Original.GetData(), 56);
    AppendU32(Out, uint32(Actor.WoofData.ActorRefs.Num()));
    for (const int32 ActorRef : Actor.WoofData.ActorRefs) AppendI32(Out, ActorRef);
    Out.Append(Original.GetData() + Offsets.Suffix, int32(Offsets.Tail - Offsets.Suffix));
    if (!Actor.ClassData.IsEmpty()) Out.Append(Actor.ClassData);

    FKyaWoofOffsets Rebuilt;
    if (!KyaReadWoofOffsets(MakeArrayView(Out.GetData(), Out.Num()), 0, Out.Num(), Rebuilt) ||
        Rebuilt.Count != Actor.WoofData.ActorRefs.Num())
    {
        OutError = TEXT("The rebuilt Woof class data is inconsistent.");
        return false;
    }
    return true;
}

bool NeedsNoseMonsterClassDataRebuild(const FKyaActorPlacement& Actor, TConstArrayView<uint8> ClassData)
{
    if (Actor.ClassId != 0x26 || !Actor.bHasNoseMonsterData) return false;
    FKyaNoseMonsterOffsets Offsets;
    if (!KyaReadNoseMonsterOffsets(ClassData, 0, ClassData.Num(), Offsets)) return true;
    return Actor.NoseMonsterZonePathEntries.Num() != Offsets.ZonePathCountValue ||
        Actor.NoseMonsterFireShotActorRefs.Num() != Offsets.FireShotCountValue;
}

bool BuildNoseMonsterClassData(const FKyaActorPlacement& Actor, TConstArrayView<uint8> Original,
    TArray<uint8>& Out, FString& OutError)
{
    Out.Reset();
    FKyaNoseMonsterOffsets Offsets;
    if (!KyaReadNoseMonsterOffsets(Original, 0, Original.Num(), Offsets))
    {
        OutError = TEXT("The Nose monster class data could not be parsed.");
        return false;
    }
    if (Actor.bNoseMonsterHasVersionFloats != Offsets.bHasVersionFloats)
    {
        OutError = TEXT("The Nose monster scene-version layout cannot be changed.");
        return false;
    }
    // Keep the prefix through the zone-path count, splice both count-prefixed
    // lists, then copy the trailing fields, switch record and raw tail verbatim.
    Out.Append(Original.GetData(), int32(Offsets.ZonePathCount));
    AppendU32(Out, uint32(Actor.NoseMonsterZonePathEntries.Num()));
    for (const FKyaNoseMonsterZonePath& Entry : Actor.NoseMonsterZonePathEntries)
    {
        AppendI32(Out, Entry.ZoneRef);
        AppendI32(Out, Entry.PathFollowRef);
    }
    Out.Append(Original.GetData() + Offsets.Field36C, int32(Offsets.FireShotCount - Offsets.Field36C));
    AppendU32(Out, uint32(Actor.NoseMonsterFireShotActorRefs.Num()));
    for (const int32 ActorRef : Actor.NoseMonsterFireShotActorRefs) AppendI32(Out, ActorRef);
    Out.Append(Original.GetData() + Offsets.Field630, int32(Offsets.Tail - Offsets.Field630));
    if (!Actor.ClassData.IsEmpty()) Out.Append(Actor.ClassData);

    FKyaNoseMonsterOffsets Rebuilt;
    if (!KyaReadNoseMonsterOffsets(MakeArrayView(Out.GetData(), Out.Num()), 0, Out.Num(), Rebuilt) ||
        Rebuilt.ZonePathCountValue != Actor.NoseMonsterZonePathEntries.Num() ||
        Rebuilt.FireShotCountValue != Actor.NoseMonsterFireShotActorRefs.Num() ||
        Rebuilt.SwitchCountValue != Actor.NoseMonsterSwitch.Targets.Num() ||
        Rebuilt.bHasVersionFloats != Offsets.bHasVersionFloats)
    {
        OutError = TEXT("The rebuilt Nose monster class data is inconsistent.");
        return false;
    }
    return true;
}

bool NeedsAmortosCmdClassDataRebuild(const FKyaActorPlacement& Actor, TConstArrayView<uint8> ClassData)
{
    if (Actor.ClassId != 0x31 || !Actor.bHasAmortosCmdData) return false;
    FKyaAmortosCmdOffsets Offsets;
    if (!KyaReadAmortosCmdOffsets(ClassData, 0, ClassData.Num(), Offsets)) return true;
    return Actor.AmortosCmdActorRefs.Num() != Offsets.Count;
}

bool BuildAmortosCmdClassData(const FKyaActorPlacement& Actor, TConstArrayView<uint8> Original,
    TArray<uint8>& Out, FString& OutError)
{
    Out.Reset();
    AppendU32(Out, uint32(Actor.AmortosCmdActorRefs.Num()));
    for (const int32 ActorRef : Actor.AmortosCmdActorRefs) AppendI32(Out, ActorRef);
    if (!Actor.ClassData.IsEmpty()) Out.Append(Actor.ClassData);

    FKyaAmortosCmdOffsets Rebuilt;
    if (!KyaReadAmortosCmdOffsets(MakeArrayView(Out.GetData(), Out.Num()), 0, Out.Num(), Rebuilt) ||
        Rebuilt.Count != Actor.AmortosCmdActorRefs.Num())
    {
        OutError = TEXT("The rebuilt Amortos command class data is inconsistent.");
        return false;
    }
    return true;
}

bool NeedsPunchingBallClassDataRebuild(const FKyaActorPlacement& Actor, TConstArrayView<uint8> ClassData)
{
    if (Actor.ClassId != 0x30 || !Actor.bHasPunchingBallData) return false;
    FKyaPunchingBallOffsets Offsets;
    if (!KyaReadPunchingBallOffsets(ClassData, 0, ClassData.Num(), Offsets)) return true;
    return Actor.PunchingBallTrapEntries.Num() != Offsets.TrapCountValue;
}

void AppendPunchingBallTrapEntry(const FKyaPunchingBallTrapEntry& Entry, TArray<uint8>& Out)
{
    AppendI32(Out, Entry.TargetRef);
    AppendI32(Out, Entry.CutsceneId);
    AppendI32(Out, Entry.Flags);
    AppendI32(Out, Entry.MessageId);
    AppendI32(Out, Entry.MessageFlags);
    AppendI32(Out, Entry.Field14);
    AppendI32(Out, Entry.Field18);
    AppendI32(Out, Entry.Camera.CameraIndex);
    AppendI32(Out, Entry.Camera.Flags);
    AppendF32(Out, Entry.Camera.Field8);
    AppendF32(Out, Entry.Camera.FieldC);
    AppendF32(Out, Entry.Camera.Field10);
    AppendI32(Out, Entry.Camera.ActorRef);
    AppendI32(Out, Entry.Camera.ZoneRef);
    AppendF32(Out, Entry.Camera.Field1C);
}

bool BuildPunchingBallClassData(const FKyaActorPlacement& Actor, TConstArrayView<uint8> Original,
    TArray<uint8>& Out, FString& OutError)
{
    Out.Reset();
    FKyaPunchingBallOffsets Offsets;
    if (!KyaReadPunchingBallOffsets(Original, 0, Original.Num(), Offsets))
    {
        OutError = TEXT("The Punching ball class data could not be parsed.");
        return false;
    }
    if (Actor.PunchingBallCamFigFloats.Num() != 14)
    {
        OutError = TEXT("The Punching ball camera-fight float block must contain exactly 14 values.");
        return false;
    }
    // Keep the fighter prefix and tuning floats, splice a new trap stream, then
    // copy the actor refs, camera-fight floats and raw tail verbatim.
    Out.Append(Original.GetData(), int32(Offsets.TrapCount));
    AppendU32(Out, uint32(Actor.PunchingBallTrapEntries.Num()));
    for (const FKyaPunchingBallTrapEntry& Entry : Actor.PunchingBallTrapEntries)
        AppendPunchingBallTrapEntry(Entry, Out);
    Out.Append(Original.GetData() + Offsets.ActorRefA94, int32(Offsets.Tail - Offsets.ActorRefA94));
    if (!Actor.ClassData.IsEmpty()) Out.Append(Actor.ClassData);

    FKyaPunchingBallOffsets Rebuilt;
    if (!KyaReadPunchingBallOffsets(MakeArrayView(Out.GetData(), Out.Num()), 0, Out.Num(), Rebuilt) ||
        Rebuilt.TrapCountValue != Actor.PunchingBallTrapEntries.Num())
    {
        OutError = TEXT("The rebuilt Punching ball class data is inconsistent.");
        return false;
    }
    return true;
}

bool NeedsShipClassDataRebuild(const FKyaActorPlacement& Actor, TConstArrayView<uint8> ClassData)
{
    if (Actor.ClassId != 0x36 || !Actor.bHasShipData) return false;
    FKyaShipOffsets Offsets;
    if (!KyaReadShipOffsets(ClassData, 0, ClassData.Num(), Offsets)) return true;
    return Actor.ShipCareBoyActorRefs.Num() != Offsets.CareBoyCountValue ||
        Actor.ShipFxIntValues.Num() != Offsets.FxCountValue;
}

bool BuildShipClassData(const FKyaActorPlacement& Actor, TConstArrayView<uint8> Original,
    TArray<uint8>& Out, FString& OutError)
{
    Out.Reset();
    FKyaShipOffsets Offsets;
    if (!KyaReadShipOffsets(Original, 0, Original.Num(), Offsets))
    {
        OutError = TEXT("The Ship class data could not be parsed.");
        return false;
    }
    if (Actor.ShipWords18C.Num() != 4)
    {
        OutError = TEXT("The Ship four-word block cannot be resized.");
        return false;
    }
    // The care-boy and FX integer streams are adjacent count-prefixed lists at the
    // start of the class data; everything after them is fixed length.
    Out.Append(Original.GetData(), int32(Offsets.CareBoyCount));
    AppendU32(Out, uint32(Actor.ShipCareBoyActorRefs.Num()));
    for (const int32 ActorRef : Actor.ShipCareBoyActorRefs) AppendI32(Out, ActorRef);
    AppendU32(Out, uint32(Actor.ShipFxIntValues.Num()));
    for (const int32 Value : Actor.ShipFxIntValues) AppendI32(Out, Value);
    Out.Append(Original.GetData() + Offsets.ActorRefs, int32(Offsets.Tail - Offsets.ActorRefs));
    if (!Actor.ClassData.IsEmpty()) Out.Append(Actor.ClassData);

    FKyaShipOffsets Rebuilt;
    if (!KyaReadShipOffsets(MakeArrayView(Out.GetData(), Out.Num()), 0, Out.Num(), Rebuilt) ||
        Rebuilt.CareBoyCountValue != Actor.ShipCareBoyActorRefs.Num() ||
        Rebuilt.FxCountValue != Actor.ShipFxIntValues.Num() ||
        Rebuilt.SwitchCountValue != Actor.ShipSwitch.Targets.Num())
    {
        OutError = TEXT("The rebuilt Ship class data is inconsistent.");
        return false;
    }
    return true;
}

bool NeedsStillerClassDataRebuild(const FKyaActorPlacement& Actor, TConstArrayView<uint8> ClassData)
{
    if (Actor.ClassId != 0x4c || !Actor.bHasStillerData) return false;
    FKyaStillerOffsets Offsets;
    if (!KyaReadStillerOffsets(ClassData, 0, ClassData.Num(), Offsets)) return true;
    return Actor.StillerActorRefs.Num() != Offsets.ActorCountValue;
}

bool BuildStillerClassData(const FKyaActorPlacement& Actor, TConstArrayView<uint8> Original,
    TArray<uint8>& Out, FString& OutError)
{
    Out.Reset();
    FKyaStillerOffsets Offsets;
    if (!KyaReadStillerOffsets(Original, 0, Original.Num(), Offsets))
    {
        OutError = TEXT("The Stiller class data could not be parsed.");
        return false;
    }
    // Keep the fixed prefix, splice a new actor stream and copy the switch/add-on
    // remainder verbatim.
    Out.Append(Original.GetData(), int32(Offsets.ActorCount));
    AppendU32(Out, uint32(Actor.StillerActorRefs.Num()));
    for (const int32 ActorRef : Actor.StillerActorRefs) AppendI32(Out, ActorRef);
    Out.Append(Original.GetData() + Offsets.Field200, int32(Offsets.Tail - Offsets.Field200));
    if (!Actor.ClassData.IsEmpty()) Out.Append(Actor.ClassData);

    FKyaStillerOffsets Rebuilt;
    if (!KyaReadStillerOffsets(MakeArrayView(Out.GetData(), Out.Num()), 0, Out.Num(), Rebuilt) ||
        Rebuilt.ActorCountValue != Actor.StillerActorRefs.Num() ||
        Rebuilt.Switches[0].Count != Actor.StillerSwitch208.Targets.Num() ||
        Rebuilt.Switches[1].Count != Actor.StillerSwitch210.Targets.Num() ||
        Rebuilt.SubObjModeValue != Actor.StillerAddOn.SubObjMode)
    {
        OutError = TEXT("The rebuilt Stiller class data is inconsistent.");
        return false;
    }
    return true;
}

bool NeedsBunchClassDataRebuild(const FKyaActorPlacement& Actor, TConstArrayView<uint8> ClassData)
{
    if (Actor.ClassId != 0x4e || !Actor.bHasBunchData) return false;
    FKyaBunchOffsets Offsets;
    if (!KyaReadBunchOffsets(ClassData, 0, ClassData.Num(), Offsets)) return true;
    if (Actor.BunchStreamEntries.Num() != Offsets.EntryCountValue) return true;
    int64 Expected = Offsets.WolfenEnd + 4 + 4 + 8;
    for (const FKyaBunchStreamEntry& Entry : Actor.BunchStreamEntries)
        Expected += Entry.HasBehaviour != 0 ? 20 : 16;
    Expected += Actor.ClassData.Num();
    return Expected != ClassData.Num();
}

bool BuildBunchClassData(const FKyaActorPlacement& Actor, TConstArrayView<uint8> Original,
    TArray<uint8>& Out, FString& OutError)
{
    Out.Reset();
    FKyaBunchOffsets Offsets;
    if (!KyaReadBunchOffsets(Original, 0, Original.Num(), Offsets))
    {
        OutError = TEXT("The Bunch class data could not be parsed.");
        return false;
    }
    Out.Append(Original.GetData(), int32(Offsets.WolfenEnd));
    AppendF32(Out, Actor.BunchFieldFC8);
    AppendU32(Out, uint32(Actor.BunchStreamEntries.Num()));
    for (const FKyaBunchStreamEntry& Entry : Actor.BunchStreamEntries)
    {
        AppendI32(Out, Entry.Field0);
        AppendI32(Out, Entry.HasBehaviour);
        if (Entry.HasBehaviour != 0) AppendI32(Out, Entry.BehaviourId);
        AppendI32(Out, Entry.ZoneId);
        AppendI32(Out, Entry.FieldC);
    }
    AppendI32(Out, Actor.BunchMeshComponentId);
    AppendI32(Out, Actor.BunchMeshIndex);
    if (!Actor.ClassData.IsEmpty()) Out.Append(Actor.ClassData);

    FKyaBunchOffsets Rebuilt;
    if (!KyaReadBunchOffsets(MakeArrayView(Out.GetData(), Out.Num()), 0, Out.Num(), Rebuilt) ||
        Rebuilt.EntryCountValue != Actor.BunchStreamEntries.Num())
    {
        OutError = TEXT("The rebuilt Bunch class data is inconsistent.");
        return false;
    }
    return true;
}

bool NeedsFogClassDataRebuild(const FKyaActorPlacement& Actor, TConstArrayView<uint8> ClassData)
{
    if (Actor.ClassId != 0x3d || !Actor.bHasFogData) return false;
    FKyaFogOffsets Offsets;
    if (!KyaReadFogOffsets(ClassData, 0, ClassData.Num(), Offsets)) return true;
    return Actor.FogHierarchies.Num() != Offsets.CountValue;
}

bool BuildFogClassData(const FKyaActorPlacement& Actor, TConstArrayView<uint8> Original,
    TArray<uint8>& Out, FString& OutError)
{
    Out.Reset();
    AppendU32(Out, uint32(Actor.FogHierarchies.Num()));
    for (const FKyaFogHierarchy& Hierarchy : Actor.FogHierarchies)
    {
        AppendU32(Out, uint32(uint64(Hierarchy.Hash)));
        AppendU32(Out, uint32(uint64(Hierarchy.Hash) >> 32));
        AppendF32(Out, Hierarchy.Field20);
        AppendF32(Out, Hierarchy.Field24);
    }
    // Anything after the hierarchy records is preserved verbatim.
    const FKyaFogOffsets Offsets = [&Original]()
    {
        FKyaFogOffsets Parsed;
        KyaReadFogOffsets(Original, 0, Original.Num(), Parsed);
        return Parsed;
    }();
    if (Offsets.Tail > 0 && Offsets.Tail < Original.Num())
        Out.Append(Original.GetData() + Offsets.Tail, int32(Original.Num() - Offsets.Tail));
    if (!Actor.ClassData.IsEmpty()) Out.Append(Actor.ClassData);

    FKyaFogOffsets Rebuilt;
    if (!KyaReadFogOffsets(MakeArrayView(Out.GetData(), Out.Num()), 0, Out.Num(), Rebuilt) ||
        Rebuilt.CountValue != Actor.FogHierarchies.Num())
    {
        OutError = TEXT("The rebuilt fog-hierarchy class data is inconsistent.");
        return false;
    }
    return true;
}

bool NeedsKimClassDataRebuild(const FKyaActorPlacement& Actor, TConstArrayView<uint8> ClassData)
{
    if (Actor.ClassId != 0x06 || !Actor.bHasKimData) return false;
    FKyaKimOffsets Offsets;
    if (!KyaReadKimOffsets(ClassData, 0, ClassData.Num(), Offsets)) return true;
    if (Actor.KimClimbZones.Num() != Offsets.ClimbCountValue ||
        Actor.KimTutorialZones.Num() != Offsets.TutorialCountValue ||
        Actor.KimProjectileZones.Num() != Offsets.ProjectileCountValue ||
        Actor.KimBoomyZonePairs.Num() != Offsets.PairCountValue ||
        Actor.KimZoneStream.Num() != Offsets.ZoneStreamCountValue ||
        Actor.KimCameras.Num() != Offsets.Cameras.Num())
        return true;
    for (int32 Index = 0; Index < Actor.KimTutorialZones.Num(); ++Index)
        if (Actor.KimTutorialZones[Index].Targets.Num() != Offsets.Tutorials[Index].TargetCountValue)
            return true;
    return false;
}

bool BuildKimClassData(const FKyaActorPlacement& Actor, TConstArrayView<uint8> Original,
    TArray<uint8>& Out, FString& OutError)
{
    Out.Reset();
    FKyaKimOffsets Offsets;
    if (!KyaReadKimOffsets(Original, 0, Original.Num(), Offsets))
    {
        OutError = TEXT("The Kim class data could not be parsed.");
        return false;
    }
    // The climb stream, tutorial zones, projectile zones, boomy pairs, zone stream
    // and camera records are the variable sections rewritten here.
    if (Actor.KimCameras.Num() == 1)
    {
        OutError = TEXT("Kim camera lists must contain zero or at least two records.");
        return false;
    }

    Out.Append(Original.GetData(), int32(Offsets.ClimbCount));
    AppendU32(Out, uint32(Actor.KimClimbZones.Num()));
    for (const int32 Value : Actor.KimClimbZones) AppendI32(Out, Value);

    AppendU32(Out, uint32(Actor.KimTutorialZones.Num()));
    for (const FKyaHeroTutorialZone& Zone : Actor.KimTutorialZones)
    {
        AppendI32(Out, Zone.ZoneRef);
        AppendU32(Out, uint32(uint64(Zone.MessageKey)));
        AppendU32(Out, uint32(uint64(Zone.MessageKey) >> 32));
        AppendI32(Out, Zone.Flags);
        AppendI32(Out, Zone.Action);
        AppendF32(Out, Zone.Duration);
        AppendU32(Out, uint32(Zone.Targets.Num()));
        for (const FKyaSwitchTarget& Item : Zone.Targets)
        {
            AppendI32(Out, Item.TargetRef);
            AppendI32(Out, Item.CutsceneId);
            AppendI32(Out, Item.Flags);
            AppendI32(Out, Item.MessageId);
            AppendI32(Out, Item.MessageFlags);
            AppendI32(Out, Item.Field14);
            AppendI32(Out, Item.Field18);
        }
        const FKyaStreamEventCamera& Camera = Zone.Camera;
        AppendI32(Out, Camera.CameraIndex);
        AppendI32(Out, Camera.Flags);
        AppendF32(Out, Camera.Field8);
        AppendF32(Out, Camera.FieldC);
        AppendF32(Out, Camera.Field10);
        AppendI32(Out, Camera.ActorRef);
        AppendI32(Out, Camera.ZoneRef);
        AppendF32(Out, Camera.Field1C);
    }

    const int64 OldTutorialEnd = Offsets.TutorialCountValue > 0
        ? Offsets.Tutorials.Last().Tail : Offsets.TutorialEntries;
    Out.Append(Original.GetData() + OldTutorialEnd, int32(Offsets.ProjectileCount - OldTutorialEnd));

    AppendU32(Out, uint32(Actor.KimProjectileZones.Num()));
    for (const FKyaHeroProjectileZone& Zone : Actor.KimProjectileZones)
    {
        AppendI32(Out, Zone.ZoneRef);
        AppendI32(Out, Zone.ProjectileType);
        AppendF32(Out, Zone.Damage);
        AppendI32(Out, Zone.WaypointIndex);
        AppendF32(Out, Zone.Force);
        AppendI32(Out, Zone.SampleCount);
        AppendI32(Out, Zone.SectorId);
    }
    const int64 OldProjectileEnd = Offsets.ProjectileEntries + int64(Offsets.ProjectileCountValue) * 28;
    Out.Append(Original.GetData() + OldProjectileEnd, int32(Offsets.PairCount - OldProjectileEnd));

    AppendU32(Out, uint32(Actor.KimBoomyZonePairs.Num()));
    for (const FKyaHeroZonePair& Pair : Actor.KimBoomyZonePairs)
    {
        AppendI32(Out, Pair.ZoneRef);
        AppendI32(Out, Pair.Field4);
    }
    const int64 PairOldEnd = Offsets.PairEntries + int64(Offsets.PairCountValue) * 8;
    Out.Append(Original.GetData() + PairOldEnd, int32(Offsets.ZoneStreamCount - PairOldEnd));

    AppendU32(Out, uint32(Actor.KimZoneStream.Num()));
    for (const int32 Value : Actor.KimZoneStream) AppendI32(Out, Value);
    const int64 ZoneOldEnd = Offsets.ZoneStreamEntries + int64(Offsets.ZoneStreamCountValue) * 4;
    // Material/fx/joke fields sit between the zone stream and the camera count.
    Out.Append(Original.GetData() + ZoneOldEnd, int32(Offsets.CameraCount - ZoneOldEnd));
    AppendU32(Out, uint32(Actor.KimCameras.Num()));
    for (const FKyaActorCamera& Camera : Actor.KimCameras) AppendKimCamera(Camera, Out);
    const int64 OldCameraEnd = Offsets.Cameras.Num() > 0
        ? Offsets.Cameras.Last().Tail : Offsets.CameraCount + 4;
    Out.Append(Original.GetData() + OldCameraEnd, int32(Offsets.Tail - OldCameraEnd));
    if (!Actor.ClassData.IsEmpty()) Out.Append(Actor.ClassData);

    FKyaKimOffsets Rebuilt;
    if (!KyaReadKimOffsets(MakeArrayView(Out.GetData(), Out.Num()), 0, Out.Num(), Rebuilt) ||
        Rebuilt.ClimbCountValue != Actor.KimClimbZones.Num() ||
        Rebuilt.TutorialCountValue != Actor.KimTutorialZones.Num() ||
        Rebuilt.ProjectileCountValue != Actor.KimProjectileZones.Num() ||
        Rebuilt.PairCountValue != Actor.KimBoomyZonePairs.Num() ||
        Rebuilt.ZoneStreamCountValue != Actor.KimZoneStream.Num() ||
        Rebuilt.Cameras.Num() != Actor.KimCameras.Num())
    {
        OutError = TEXT("The rebuilt Kim class data is inconsistent.");
        return false;
    }
    for (int32 Index = 0; Index < Actor.KimTutorialZones.Num(); ++Index)
        if (Rebuilt.Tutorials[Index].TargetCountValue != Actor.KimTutorialZones[Index].Targets.Num())
        {
            OutError = TEXT("The rebuilt Kim tutorial target counts are inconsistent.");
            return false;
        }
    return true;
}

bool NeedsBoxClassDataRebuild(const FKyaActorPlacement& Actor, TConstArrayView<uint8> ClassData)
{
    if (Actor.ClassId != 0x35 || !Actor.bHasBoxData) return false;
    FKyaBoxOffsets Offsets;
    if (!KyaReadBoxOffsets(ClassData, 0, ClassData.Num(), Offsets)) return true;
    return Actor.BoxActorRefs.Num() != Offsets.ActorCountValue;
}

bool BuildBoxClassData(const FKyaActorPlacement& Actor, TConstArrayView<uint8> Original,
    TArray<uint8>& Out, FString& OutError)
{
    Out.Reset();
    FKyaBoxOffsets Offsets;
    if (!KyaReadBoxOffsets(Original, 0, Original.Num(), Offsets))
    {
        OutError = TEXT("The box class data could not be parsed.");
        return false;
    }
    // Keep the fixed prefix (weight, autonomous block, field 0x350) and splice a
    // new actor stream where the old one was. Everything after the stream is fixed
    // length for the existing sub-object layout, so it moves verbatim.
    Out.Append(Original.GetData(), int32(Offsets.ActorCount));
    AppendU32(Out, uint32(Actor.BoxActorRefs.Num()));
    for (const int32 ActorRef : Actor.BoxActorRefs) AppendI32(Out, ActorRef);
    Out.Append(Original.GetData() + Offsets.Floats354, int32(Offsets.Tail - Offsets.Floats354));
    if (!Actor.ClassData.IsEmpty()) Out.Append(Actor.ClassData);

    FKyaBoxOffsets Rebuilt;
    if (!KyaReadBoxOffsets(MakeArrayView(Out.GetData(), Out.Num()), 0, Out.Num(), Rebuilt) ||
        Rebuilt.ActorCountValue != Actor.BoxActorRefs.Num() ||
        Rebuilt.Switch.Count != Actor.BoxSwitch.Targets.Num() ||
        Rebuilt.SubObjModeValue != Actor.BoxAddOn.SubObjMode)
    {
        OutError = TEXT("The rebuilt box class data is inconsistent.");
        return false;
    }
    return true;
}

/** Behaviour id 3 on a shoot actor is a 12-byte header followed by an actor stream. */
bool IsShootFireBehaviour(const FKyaActorBehaviour& Behaviour)
{
    return Behaviour.Id == 3 && Behaviour.bHasTypedShootFireBehaviour;
}

int64 ShootFirePayloadSize(const FKyaActorBehaviour& Behaviour)
{
    return 16 + int64(Behaviour.ShootFireActorRefs.Num()) * 4;
}

bool BuildShootFireBehaviourPayload(const FKyaActorBehaviour& Behaviour, TArray<uint8>& Out)
{
    Out.Reset();
    AppendI32(Out, Behaviour.ShootFireField2B0);
    AppendF32(Out, Behaviour.ShootFireField8);
    AppendF32(Out, Behaviour.ShootFireField2B4);
    AppendU32(Out, uint32(Behaviour.ShootFireActorRefs.Num()));
    for (const int32 ActorRef : Behaviour.ShootFireActorRefs) AppendI32(Out, ActorRef);
    return true;
}

/** Behaviour id 2 on a clusteriser is a count followed by variable zone records. */
bool IsClusterBehaviour(const FKyaActorBehaviour& Behaviour)
{
    return Behaviour.Id == 2 && Behaviour.bHasTypedClusterZones;
}

int64 ClusterPayloadSize(const FKyaActorBehaviour& Behaviour)
{
    int64 Size = 4;
    for (const FKyaClusterZone& Zone : Behaviour.ClusterZones)
        Size += 8 + 20 + 4 * (Zone.Actors.Num() + Zone.OtherActors.Num() + Zone.Cameras.Num() +
            Zone.Lights.Num()) + 8 * Zone.Hashes.Num() + 4;
    return Size;
}

bool BuildClusterBehaviourPayload(const FKyaActorBehaviour& Behaviour, TArray<uint8>& Out)
{
    Out.Reset();
    AppendU32(Out, uint32(Behaviour.ClusterZones.Num()));
    for (const FKyaClusterZone& Zone : Behaviour.ClusterZones)
    {
        AppendU32(Out, uint32(Zone.Flags));
        AppendI32(Out, Zone.ZoneRef);
        const TArray<int32>* Lists[] = { &Zone.Actors, &Zone.OtherActors, &Zone.Cameras,
            &Zone.Lights };
        for (int32 List = 0; List < 4; ++List)
        {
            AppendU32(Out, uint32(Lists[List]->Num()));
            for (const int32 Value : *Lists[List]) AppendI32(Out, Value);
        }
        AppendU32(Out, uint32(Zone.Hashes.Num()));
        for (const int64 Hash : Zone.Hashes)
        {
            AppendU32(Out, uint32(uint64(Hash)));
            AppendU32(Out, uint32(uint64(Hash) >> 32));
        }
        AppendI32(Out, Zone.OtherZoneRef);
    }
    return true;
}

/** Behaviour id 2 on a fog manager is a count followed by 28-byte fog zones. */
bool IsFogBehaviour(const FKyaActorBehaviour& Behaviour)
{
    return Behaviour.Id == 2 && Behaviour.bHasTypedFogBehaviour;
}

int64 FogPayloadSize(const FKyaActorBehaviour& Behaviour)
{
    return 4 + int64(Behaviour.FogZones.Num()) * 28;
}

bool BuildFogBehaviourPayload(const FKyaActorBehaviour& Behaviour, TArray<uint8>& Out)
{
    Out.Reset();
    AppendU32(Out, uint32(Behaviour.FogZones.Num()));
    for (const FKyaFogZone& Zone : Behaviour.FogZones)
    {
        AppendI32(Out, Zone.Flags);
        AppendI32(Out, Zone.ZoneRef);
        AppendF32(Out, Zone.ClipValue);
        AppendF32(Out, Zone.Field4);
        AppendI32(Out, Zone.FogRGBA);
        AppendI32(Out, Zone.FogFlags);
        AppendF32(Out, Zone.Field18);
    }
    return true;
}

/** Behaviour id 26 on Brazul is a count followed by variable sub-object records. */
bool IsBrazulABehaviour(const FKyaActorBehaviour& Behaviour)
{
    return Behaviour.Id == 26 && Behaviour.bHasTypedBrazulA;
}

int64 BrazulAPayloadSize(const FKyaActorBehaviour& Behaviour)
{
    int64 Size = 4;
    for (const FKyaBrazulSubObj& Sub : Behaviour.BrazulSubObjs)
        Size += 24 + int64(Sub.PathIndices.Num()) * 4;
    return Size;
}

bool BuildBrazulABehaviourPayload(const FKyaActorBehaviour& Behaviour, TArray<uint8>& Out)
{
    Out.Reset();
    AppendU32(Out, uint32(Behaviour.BrazulSubObjs.Num()));
    for (const FKyaBrazulSubObj& Sub : Behaviour.BrazulSubObjs)
    {
        AppendI32(Out, Sub.Field0);
        AppendI32(Out, Sub.Field4);
        AppendF32(Out, Sub.Field8);
        AppendI32(Out, Sub.FieldC);
        AppendI32(Out, Sub.Field10);
        AppendU32(Out, uint32(Sub.PathIndices.Num()));
        for (const int32 Path : Sub.PathIndices) AppendI32(Out, Path);
    }
    return true;
}

/** Behaviour id 4 on an FX actor is a count followed by raw integer values. */
bool IsFxListBehaviour(const FKyaActorBehaviour& Behaviour)
{
    return Behaviour.Id == 4 && Behaviour.bHasTypedFxBehaviour;
}

int64 FxListPayloadSize(const FKyaActorBehaviour& Behaviour)
{
    return 4 + int64(Behaviour.FxUnknownValues.Num()) * 4;
}

bool BuildFxListBehaviourPayload(const FKyaActorBehaviour& Behaviour, TArray<uint8>& Out)
{
    Out.Reset();
    AppendU32(Out, uint32(Behaviour.FxUnknownValues.Num()));
    for (const int32 Value : Behaviour.FxUnknownValues) AppendI32(Out, Value);
    return true;
}

/** Behaviour id 6 on a weapon is two burst entries followed by an actor stream. */
bool IsWeaponRpgBehaviour(const FKyaActorBehaviour& Behaviour)
{
    return Behaviour.Id == 6 && Behaviour.bHasTypedWeaponBehaviour;
}

int64 WeaponRpgPayloadSize(const FKyaActorBehaviour& Behaviour)
{
    return 36 + int64(Behaviour.WeaponRpgActorRefs.Num()) * 4;
}

bool BuildWeaponRpgBehaviourPayload(const FKyaActorBehaviour& Behaviour, TArray<uint8>& Out)
{
    Out.Reset();
    for (int32 Index = 0; Index < 2; ++Index)
    {
        const FKyaWeaponAdvancedEntry* Entry = Behaviour.WeaponBurst.IsValidIndex(Index)
            ? &Behaviour.WeaponBurst[Index] : nullptr;
        AppendF32(Out, Entry ? Entry->FieldC : 0.0f);
        AppendI32(Out, Entry ? Entry->Field4 : 0);
        AppendF32(Out, Entry ? Entry->Field0 : 0.0f);
        AppendF32(Out, Entry ? Entry->Field8 : 0.0f);
    }
    AppendU32(Out, uint32(Behaviour.WeaponRpgActorRefs.Num()));
    for (const int32 ActorRef : Behaviour.WeaponRpgActorRefs) AppendI32(Out, ActorRef);
    return true;
}

/** Behaviour id 3 on a companion is a count followed by variable alert records. */
bool IsCompanionAlertBehaviour(const FKyaActorBehaviour& Behaviour)
{
    return Behaviour.Id == 3 && Behaviour.bHasTypedCompanionBehaviour;
}

void AppendCompanionCondition(const FKyaScenarioCondition& Condition, TArray<uint8>& Out)
{
    AppendI32(Out, Condition.Combiner);
    AppendI32(Out, Condition.Variable);
    AppendI32(Out, Condition.Comparison);
    AppendI32(Out, Condition.Value);
}

int64 CompanionAlertPayloadSize(const FKyaActorBehaviour& Behaviour)
{
    int64 Size = 4;
    for (const FKyaCompanionAlert& Alert : Behaviour.CompanionAlerts)
    {
        Size += 4 + int64(Alert.Conditions.Num()) * 16;      // conditions
        Size += 20 + 4 + 12 + 4;                             // flags.., actor, vector, camera
        Size += 4 + int64(Alert.Field34.Num()) * 4;          // field 0x34 list
        Size += 20;                                          // key + fields 0x50/0x58/0x5c
        Size += 4 + int64(Alert.TargetSwitch.Targets.Num()) * 28 + 32;  // switch
        Size += 4;                                           // conditioned block count
        for (const FKyaConditionedOperation& Block : Alert.Field70)
            Size += 4 + int64(Block.Operations.Num()) * 12 + 4 + int64(Block.Conditions.Num()) * 16;
    }
    return Size;
}

bool BuildCompanionAlertBehaviourPayload(const FKyaActorBehaviour& Behaviour, TArray<uint8>& Out)
{
    Out.Reset();
    AppendU32(Out, uint32(Behaviour.CompanionAlerts.Num()));
    for (const FKyaCompanionAlert& Alert : Behaviour.CompanionAlerts)
    {
        AppendU32(Out, uint32(Alert.Conditions.Num()));
        for (const FKyaScenarioCondition& Condition : Alert.Conditions)
            AppendCompanionCondition(Condition, Out);
        AppendI32(Out, Alert.Flags);
        AppendI32(Out, Alert.Field08);
        AppendI32(Out, Alert.Field10);
        AppendI32(Out, Alert.Field14);
        AppendF32(Out, Alert.Field18);
        AppendI32(Out, Alert.ActorRef);
        for (int32 Axis = 0; Axis < 3; ++Axis) AppendF32(Out, float(Alert.Field20[Axis]));
        AppendI32(Out, Alert.CameraRef);
        AppendU32(Out, uint32(Alert.Field34.Num()));
        for (const int32 Value : Alert.Field34) AppendI32(Out, Value);
        AppendU32(Out, uint32(uint64(Alert.LevelStringKey)));
        AppendU32(Out, uint32(uint64(Alert.LevelStringKey) >> 32));
        AppendF32(Out, Alert.Field50);
        AppendI32(Out, Alert.Field58);
        AppendF32(Out, Alert.Field5C);
        AppendU32(Out, uint32(Alert.TargetSwitch.Targets.Num()));
        for (const FKyaSwitchTarget& Target : Alert.TargetSwitch.Targets)
        {
            AppendI32(Out, Target.TargetRef);
            AppendI32(Out, Target.CutsceneId);
            AppendI32(Out, Target.Flags);
            AppendI32(Out, Target.MessageId);
            AppendI32(Out, Target.MessageFlags);
            AppendI32(Out, Target.Field14);
            AppendI32(Out, Target.Field18);
        }
        const FKyaStreamEventCamera& Camera = Alert.TargetSwitch.Camera;
        AppendI32(Out, Camera.CameraIndex);
        AppendI32(Out, Camera.Flags);
        AppendF32(Out, Camera.Field8);
        AppendF32(Out, Camera.FieldC);
        AppendF32(Out, Camera.Field10);
        AppendI32(Out, Camera.ActorRef);
        AppendI32(Out, Camera.ZoneRef);
        AppendF32(Out, Camera.Field1C);
        AppendU32(Out, uint32(Alert.Field70.Num()));
        for (const FKyaConditionedOperation& Block : Alert.Field70)
        {
            AppendU32(Out, uint32(Block.Operations.Num()));
            for (const FKyaScenarioOperation& Operation : Block.Operations)
            {
                AppendI32(Out, Operation.Variable);
                AppendI32(Out, Operation.Type);
                AppendI32(Out, Operation.Value);
            }
            AppendU32(Out, uint32(Block.Conditions.Num()));
            for (const FKyaScenarioCondition& Condition : Block.Conditions)
                AppendCompanionCondition(Condition, Out);
        }
    }
    return true;
}

/** Behaviour id 27 on a Bunch is two refs, waypoint/fence lists and a fixed tail. */
bool IsBunchFenceBehaviour(const FKyaActorBehaviour& Behaviour)
{
    return Behaviour.Id == 27 && Behaviour.bHasTypedBunchFence;
}

int64 BunchFencePayloadSize(const FKyaActorBehaviour& Behaviour)
{
    int64 Size = 8 + 4 + int64(Behaviour.BunchFenceWayPoints.Num()) * 4 + 4 + 32;
    for (const FKyaBunchFenceGroup& Group : Behaviour.BunchFenceGroups)
        Size += 12 + int64(Group.Entries.Num()) * 8;
    return Size;
}

bool BuildBunchFenceBehaviourPayload(const FKyaActorBehaviour& Behaviour, TArray<uint8>& Out)
{
    Out.Reset();
    AppendI32(Out, Behaviour.BunchFenceActorRef);
    AppendI32(Out, Behaviour.BunchFenceWayPointRef);
    AppendU32(Out, uint32(Behaviour.BunchFenceWayPoints.Num()));
    for (const int32 Value : Behaviour.BunchFenceWayPoints) AppendI32(Out, Value);
    AppendU32(Out, uint32(Behaviour.BunchFenceGroups.Num()));
    for (const FKyaBunchFenceGroup& Group : Behaviour.BunchFenceGroups)
    {
        AppendI32(Out, Group.Field0);
        AppendI32(Out, Group.Field4);
        AppendU32(Out, uint32(Group.Entries.Num()));
        for (const FKyaBunchFenceEntry& Entry : Group.Entries)
        {
            AppendI32(Out, Entry.Ref);
            AppendI32(Out, Entry.Field4);
        }
    }
    AppendI32(Out, Behaviour.BunchFenceMeshF0Texture);
    AppendI32(Out, Behaviour.BunchFenceMeshF0Mesh);
    AppendI32(Out, Behaviour.BunchFenceMesh70Texture);
    AppendI32(Out, Behaviour.BunchFenceMesh70Mesh);
    for (int32 Index = 0; Index < 4; ++Index)
        AppendI32(Out, Behaviour.BunchFenceFxTypes.IsValidIndex(Index)
            ? Behaviour.BunchFenceFxTypes[Index] : 0);
    return true;
}

/** Behaviour id 26 on a Bunch is a fixed repel record with a variable index list. */
bool IsBunchRepelBehaviour(const FKyaActorBehaviour& Behaviour)
{
    return Behaviour.Id == 26 && Behaviour.bHasTypedBunchRepel;
}

int64 BunchRepelPayloadSize(const FKyaActorBehaviour& Behaviour)
{
    return 80 + int64(Behaviour.BunchRepelIndices.Num()) * 4;
}

bool BuildBunchRepelBehaviourPayload(const FKyaActorBehaviour& Behaviour, TArray<uint8>& Out)
{
    Out.Reset();
    AppendI32(Out, Behaviour.BunchRepelWayPointRef);
    AppendI32(Out, Behaviour.BunchRepelShockWaveCount);
    AppendF32(Out, Behaviour.BunchRepelField14);
    AppendF32(Out, Behaviour.BunchRepelShockWaveField10);
    AppendF32(Out, Behaviour.BunchRepelShockWaveField14);
    AppendU32(Out, uint32(Behaviour.BunchRepelIndices.Num()));
    for (const int32 Value : Behaviour.BunchRepelIndices) AppendI32(Out, Value);
    AppendF32(Out, Behaviour.BunchRepelDamage);
    AppendF32(Out, Behaviour.BunchRepelMsgHitField30);
    AppendF32(Out, Behaviour.BunchRepelMsgHitField70);
    for (int32 Axis = 0; Axis < 3; ++Axis) AppendF32(Out, float(Behaviour.BunchRepelVector20[Axis]));
    for (int32 Axis = 0; Axis < 3; ++Axis) AppendF32(Out, float(Behaviour.BunchRepelVector30[Axis]));
    AppendI32(Out, Behaviour.BunchRepelMeshComponentId);
    AppendI32(Out, Behaviour.BunchRepelMeshIndex);
    AppendI32(Out, Behaviour.BunchRepelMeshType);
    AppendI32(Out, Behaviour.BunchRepelReserved);
    AppendI32(Out, Behaviour.BunchRepelDefaultId);
    return true;
}

/** Behaviour id 27 on Brazul is a waypoint ref followed by a target switch. */
bool IsBrazulBBehaviour(const FKyaActorBehaviour& Behaviour)
{
    return Behaviour.Id == 27 && Behaviour.bHasTypedBrazulB;
}

int64 BrazulBPayloadSize(const FKyaActorBehaviour& Behaviour)
{
    return 8 + int64(Behaviour.BrazulBSwitch.Targets.Num()) * 28 + 32;
}

bool BuildBrazulBBehaviourPayload(const FKyaActorBehaviour& Behaviour, TArray<uint8>& Out)
{
    Out.Reset();
    AppendI32(Out, Behaviour.BrazulBWayPointRef);
    AppendU32(Out, uint32(Behaviour.BrazulBSwitch.Targets.Num()));
    for (const FKyaSwitchTarget& Target : Behaviour.BrazulBSwitch.Targets)
    {
        AppendI32(Out, Target.TargetRef);
        AppendI32(Out, Target.CutsceneId);
        AppendI32(Out, Target.Flags);
        AppendI32(Out, Target.MessageId);
        AppendI32(Out, Target.MessageFlags);
        AppendI32(Out, Target.Field14);
        AppendI32(Out, Target.Field18);
    }
    const FKyaStreamEventCamera& Camera = Behaviour.BrazulBSwitch.Camera;
    AppendI32(Out, Camera.CameraIndex);
    AppendI32(Out, Camera.Flags);
    AppendF32(Out, Camera.Field8);
    AppendF32(Out, Camera.FieldC);
    AppendF32(Out, Camera.Field10);
    AppendI32(Out, Camera.ActorRef);
    AppendI32(Out, Camera.ZoneRef);
    AppendF32(Out, Camera.Field1C);
    return true;
}

/** Nativ behaviours 3 (Speak), 6 (TakeAndPut) and 7 (Seller) hold variable lists. */
bool IsNativSpeakBehaviour(const FKyaActorBehaviour& Behaviour)
{
    return Behaviour.Id == 3 && Behaviour.bHasTypedNativBehaviour;
}

int64 NativSpeakPayloadSize(const FKyaActorBehaviour& Behaviour)
{
    return 4 + int64(Behaviour.NativSpeakValues.Num()) * 8;
}

bool BuildNativSpeakBehaviourPayload(const FKyaActorBehaviour& Behaviour, TArray<uint8>& Out)
{
    Out.Reset();
    AppendU32(Out, uint32(Behaviour.NativSpeakValues.Num()));
    for (const FVector2D& Value : Behaviour.NativSpeakValues)
    {
        AppendF32(Out, float(Value.X));
        AppendF32(Out, float(Value.Y));
    }
    return true;
}

bool IsNativTrajectoryBehaviour(const FKyaActorBehaviour& Behaviour)
{
    return Behaviour.Id == 6 && Behaviour.bHasTypedNativBehaviour;
}

int64 NativTrajectoryPayloadSize(const FKyaActorBehaviour& Behaviour)
{
    return 4 + int64(Behaviour.NativTrajectories.Num()) * 16;
}

bool BuildNativTrajectoryBehaviourPayload(const FKyaActorBehaviour& Behaviour, TArray<uint8>& Out)
{
    Out.Reset();
    AppendU32(Out, uint32(Behaviour.NativTrajectories.Num()));
    for (const FKyaNativTrajectory& Trajectory : Behaviour.NativTrajectories)
    {
        AppendF32(Out, Trajectory.Field0);
        AppendI32(Out, Trajectory.PathFollowIndex);
        AppendI32(Out, Trajectory.Field14);
        AppendF32(Out, Trajectory.Field18);
    }
    return true;
}

bool IsNativSellerBehaviour(const FKyaActorBehaviour& Behaviour)
{
    return Behaviour.Id == 7 && Behaviour.bHasTypedNativBehaviour;
}

int64 NativSellerPayloadSize(const FKyaActorBehaviour& Behaviour)
{
    int64 Size = 8;
    for (const FKyaAddOnSubObj& Sub : Behaviour.NativSellerSubObjs)
        Size += 8 + int64(Sub.CinematicIds.Num()) * 4;
    return Size;
}

bool BuildNativSellerBehaviourPayload(const FKyaActorBehaviour& Behaviour, TArray<uint8>& Out)
{
    Out.Reset();
    AppendF32(Out, Behaviour.NativSellerField8);
    AppendU32(Out, uint32(Behaviour.NativSellerSubObjs.Num()));
    for (const FKyaAddOnSubObj& Sub : Behaviour.NativSellerSubObjs)
    {
        AppendI32(Out, Sub.Field0);
        AppendU32(Out, uint32(Sub.CinematicIds.Num()));
        for (const int32 CinematicId : Sub.CinematicIds) AppendI32(Out, CinematicId);
    }
    return true;
}

bool IsNativAkasaBehaviour(const FKyaActorBehaviour& Behaviour)
{
    return Behaviour.Id == 8 && Behaviour.bHasTypedNativBehaviour;
}

int64 AkasaPayloadSize(const FKyaAkasaBehaviour& Akasa)
{
    int64 Size = (Akasa.bHasMaterial ? 4 : 0) + 4 + 4;
    for (const FKyaAkasaTutorial& Tutorial : Akasa.Tutorials)
    {
        Size += 4;
        for (const FKyaAkasaStep& Step : Tutorial.Steps)
        {
            Size += 4;
            for (const FString& Name : Step.ComboNames)
                Size += (int64(Name.Len()) + 1 + 3) & ~int64(3);
        }
    }
    Size += 4;
    for (const FKyaAddOnSubObj& Sub : Akasa.AddOnSubObjs)
        Size += 8 + int64(Sub.CinematicIds.Num()) * 4;
    for (const FKyaSwitch& Switch : Akasa.Switches)
        Size += 4 + int64(Switch.Targets.Num()) * 28 + 32;
    return Size;
}

int64 NativAkasaPayloadSize(const FKyaActorBehaviour& Behaviour)
{
    return AkasaPayloadSize(Behaviour.AkasaBehaviour);
}

void AppendAkasaSwitch(const FKyaSwitch& Switch, TArray<uint8>& Out)
{
    AppendU32(Out, uint32(Switch.Targets.Num()));
    for (const FKyaSwitchTarget& Target : Switch.Targets)
    {
        AppendI32(Out, Target.TargetRef);
        AppendI32(Out, Target.CutsceneId);
        AppendI32(Out, Target.Flags);
        AppendI32(Out, Target.MessageId);
        AppendI32(Out, Target.MessageFlags);
        AppendI32(Out, Target.Field14);
        AppendI32(Out, Target.Field18);
    }
    AppendI32(Out, Switch.Camera.CameraIndex);
    AppendI32(Out, Switch.Camera.Flags);
    AppendF32(Out, Switch.Camera.Field8);
    AppendF32(Out, Switch.Camera.FieldC);
    AppendF32(Out, Switch.Camera.Field10);
    AppendI32(Out, Switch.Camera.ActorRef);
    AppendI32(Out, Switch.Camera.ZoneRef);
    AppendF32(Out, Switch.Camera.Field1C);
}

bool BuildNativAkasaBehaviourPayload(const FKyaActorBehaviour& Behaviour, TArray<uint8>& Out)
{
    const FKyaAkasaBehaviour& Akasa = Behaviour.AkasaBehaviour;
    Out.Reset();
    if (Akasa.Switches.Num() != 4) return false;
    for (const FKyaAkasaTutorial& Tutorial : Akasa.Tutorials)
        for (const FKyaAkasaStep& Step : Tutorial.Steps)
            if (Step.ComboNames.Num() > 2) return false;
    if (Akasa.bHasMaterial) AppendU32(Out, uint32(Akasa.MaterialId));
    AppendU32(Out, uint32(Akasa.Field8));
    AppendU32(Out, uint32(Akasa.Tutorials.Num()));
    for (const FKyaAkasaTutorial& Tutorial : Akasa.Tutorials)
    {
        AppendU32(Out, uint32(Tutorial.Steps.Num()));
        for (const FKyaAkasaStep& Step : Tutorial.Steps)
        {
            AppendU32(Out, uint32(Step.ComboNames.Num()));
            for (const FString& Name : Step.ComboNames)
            {
                for (const TCHAR Char : Name)
                {
                    if (uint32(Char) > 127) return false;
                    Out.Add(uint8(Char));
                }
                Out.Add(0);
                while (Out.Num() % 4) Out.Add(0);
            }
        }
    }
    AppendU32(Out, uint32(Akasa.AddOnSubObjs.Num()));
    for (const FKyaAddOnSubObj& Sub : Akasa.AddOnSubObjs)
    {
        AppendI32(Out, Sub.Field0);
        AppendU32(Out, uint32(Sub.CinematicIds.Num()));
        for (const int32 CinematicId : Sub.CinematicIds) AppendI32(Out, CinematicId);
    }
    for (const FKyaSwitch& Switch : Akasa.Switches) AppendAkasaSwitch(Switch, Out);
    return Out.Num() == AkasaPayloadSize(Akasa);
}

bool IsGeneratedBehaviourPayload(const FKyaActorBehaviour& Behaviour)
{
    return IsShootFireBehaviour(Behaviour) || IsClusterBehaviour(Behaviour) ||
        IsFogBehaviour(Behaviour) || IsBrazulABehaviour(Behaviour) ||
        IsBrazulBBehaviour(Behaviour) || IsFxListBehaviour(Behaviour) ||
        IsWeaponRpgBehaviour(Behaviour) || IsCompanionAlertBehaviour(Behaviour) ||
        IsBunchFenceBehaviour(Behaviour) || IsBunchRepelBehaviour(Behaviour) ||
        IsNativSpeakBehaviour(Behaviour) || IsNativTrajectoryBehaviour(Behaviour) ||
        IsNativSellerBehaviour(Behaviour) || IsNativAkasaBehaviour(Behaviour);
}

int64 GeneratedBehaviourPayloadSize(const FKyaActorBehaviour& Behaviour)
{
    if (IsShootFireBehaviour(Behaviour)) return ShootFirePayloadSize(Behaviour);
    if (IsClusterBehaviour(Behaviour)) return ClusterPayloadSize(Behaviour);
    if (IsFogBehaviour(Behaviour)) return FogPayloadSize(Behaviour);
    if (IsBrazulABehaviour(Behaviour)) return BrazulAPayloadSize(Behaviour);
    if (IsBrazulBBehaviour(Behaviour)) return BrazulBPayloadSize(Behaviour);
    if (IsFxListBehaviour(Behaviour)) return FxListPayloadSize(Behaviour);
    if (IsWeaponRpgBehaviour(Behaviour)) return WeaponRpgPayloadSize(Behaviour);
    if (IsCompanionAlertBehaviour(Behaviour)) return CompanionAlertPayloadSize(Behaviour);
    if (IsBunchFenceBehaviour(Behaviour)) return BunchFencePayloadSize(Behaviour);
    if (IsBunchRepelBehaviour(Behaviour)) return BunchRepelPayloadSize(Behaviour);
    if (IsNativSpeakBehaviour(Behaviour)) return NativSpeakPayloadSize(Behaviour);
    if (IsNativTrajectoryBehaviour(Behaviour)) return NativTrajectoryPayloadSize(Behaviour);
    if (IsNativAkasaBehaviour(Behaviour)) return NativAkasaPayloadSize(Behaviour);
    return NativSellerPayloadSize(Behaviour);
}

bool BuildGeneratedBehaviourPayload(const FKyaActorBehaviour& Behaviour, TArray<uint8>& Out)
{
    if (IsShootFireBehaviour(Behaviour)) return BuildShootFireBehaviourPayload(Behaviour, Out);
    if (IsClusterBehaviour(Behaviour)) return BuildClusterBehaviourPayload(Behaviour, Out);
    if (IsFogBehaviour(Behaviour)) return BuildFogBehaviourPayload(Behaviour, Out);
    if (IsBrazulABehaviour(Behaviour)) return BuildBrazulABehaviourPayload(Behaviour, Out);
    if (IsBrazulBBehaviour(Behaviour)) return BuildBrazulBBehaviourPayload(Behaviour, Out);
    if (IsFxListBehaviour(Behaviour)) return BuildFxListBehaviourPayload(Behaviour, Out);
    if (IsWeaponRpgBehaviour(Behaviour)) return BuildWeaponRpgBehaviourPayload(Behaviour, Out);
    if (IsCompanionAlertBehaviour(Behaviour)) return BuildCompanionAlertBehaviourPayload(Behaviour, Out);
    if (IsBunchFenceBehaviour(Behaviour)) return BuildBunchFenceBehaviourPayload(Behaviour, Out);
    if (IsBunchRepelBehaviour(Behaviour)) return BuildBunchRepelBehaviourPayload(Behaviour, Out);
    if (IsNativSpeakBehaviour(Behaviour)) return BuildNativSpeakBehaviourPayload(Behaviour, Out);
    if (IsNativTrajectoryBehaviour(Behaviour)) return BuildNativTrajectoryBehaviourPayload(Behaviour, Out);
    if (IsNativAkasaBehaviour(Behaviour)) return BuildNativAkasaBehaviourPayload(Behaviour, Out);
    return BuildNativSellerBehaviourPayload(Behaviour, Out);
}

bool NeedsActorChunkRebuild(const FKyaActorPlacement& Actor, const FKyaActorOffsets& Offsets,
    TConstArrayView<uint8> ClassData)
{
    if (Actor.Conditions.Num() != Offsets.ConditionCount) return true;
    if (Actor.MacroAnimations.Num() != Offsets.MacroCount) return true;
    if (Actor.Behaviours.Num() != Offsets.Behaviours.Num()) return true;
    if (Actor.Models.Num() * 2 != Offsets.ModelStrings.Num()) return true;
    for (int32 Behaviour = 0; Behaviour < Actor.Behaviours.Num(); ++Behaviour)
    {
        if (IsGeneratedBehaviourPayload(Actor.Behaviours[Behaviour]))
        {
            // The payload is regenerated from typed values, so compare its computed
            // size instead of the stale raw bytes.
            if (GeneratedBehaviourPayloadSize(Actor.Behaviours[Behaviour]) !=
                Offsets.Behaviours[Behaviour].Size) return true;
            continue;
        }
        if (Actor.Behaviours[Behaviour].Data.Num() != Offsets.Behaviours[Behaviour].Size) return true;
    }
    for (int32 Model = 0; Model < Actor.Models.Num(); ++Model)
    {
        const FString* Names[2] = { &Actor.Models[Model].MeshName, &Actor.Models[Model].TextureName };
        for (int32 Part = 0; Part < 2; ++Part)
            if (int64(AlignU32(Names[Part]->Len() + 1)) != Offsets.ModelStrings[Model * 2 + Part].Capacity)
                return true;
    }
    return NeedsPlatformClassDataRebuild(Actor, ClassData) ||
        NeedsTrapClassDataRebuild(Actor, ClassData) ||
        NeedsFighterClassDataRebuild(Actor, ClassData) ||
        NeedsShockerCmdClassDataRebuild(Actor, ClassData) ||
        NeedsBasicBoxClassDataRebuild(Actor, ClassData) ||
        NeedsBoxClassDataRebuild(Actor, ClassData) ||
        NeedsFogClassDataRebuild(Actor, ClassData) ||
        NeedsBunchClassDataRebuild(Actor, ClassData) ||
        NeedsStillerClassDataRebuild(Actor, ClassData) ||
        NeedsShipClassDataRebuild(Actor, ClassData) ||
        NeedsPunchingBallClassDataRebuild(Actor, ClassData) ||
        NeedsAmortosCmdClassDataRebuild(Actor, ClassData) ||
        NeedsNoseMonsterClassDataRebuild(Actor, ClassData) ||
        NeedsWoofClassDataRebuild(Actor, ClassData) ||
        NeedsBlazerClassDataRebuild(Actor, ClassData) ||
        NeedsEggClassDataRebuild(Actor, ClassData) ||
        NeedsDcaClassDataRebuild(Actor, ClassData) ||
        NeedsTeleporterClassDataRebuild(Actor, ClassData) ||
        NeedsAtonClassDataRebuild(Actor, ClassData) ||
        NeedsShockerClassDataRebuild(Actor, ClassData) ||
        NeedsWindClassDataRebuild(Actor, ClassData) ||
        NeedsKimClassDataRebuild(Actor, ClassData) ||
        NeedsPatternCmdClassDataRebuild(Actor, ClassData);
}

/**
 * Rebuild one INST chunk, keeping every unknown byte group intact but regenerating
 * the sections whose typed counts/sizes changed: macro animations, behaviour
 * table and payloads, model string slots, condition records, the fixed common
 * block and supported class-specific streams. Moving-platform, trap, Wolfen/Kim
 * fighter, Shocker-command, box/basic-box and Kim stream class data are rebuilt from
 * their typed fields; other class-data bytes are copied verbatim.
 */
bool BuildActorChunk(const TArray<uint8>& Old, int64 SceneStart, const FKyaActorOffsets& Offsets,
    const FKyaActorPlacement& Actor, int32 ActorIndex, TArray<uint8>& Out, FString& OutError)
{
    Out.Reset();
    const int64 ChunkAbs = SceneStart + Offsets.Chunk;
    if (ChunkAbs < 0 || ChunkAbs + 8 > Old.Num())
    {
        OutError = TEXT("Actor chunk lies outside the bank bytes.");
        return false;
    }
    const int32 ChunkSize = int32(ReadU32(Old, ChunkAbs + 4));
    if (ChunkSize < 64 || ChunkAbs + ChunkSize > Old.Num())
    {
        OutError = TEXT("Actor chunk size is invalid.");
        return false;
    }
    const auto InChunk = [&Offsets, ChunkSize](int64 Offset, int64 Size)
    {
        return Offset >= Offsets.Chunk && Offset <= Offsets.Chunk + ChunkSize &&
            Size >= 0 && Size <= Offsets.Chunk + ChunkSize - Offset;
    };

    if (!InChunk(Offsets.Chunk, 16)) return false;
    Out.Append(Old.GetData() + ChunkAbs, 16);

    const int32 NameRel = int32(Offsets.Name - Offsets.Chunk);
    if (!InChunk(Offsets.Name, Offsets.NameCapacity) || NameRel != Out.Num())
    {
        OutError = TEXT("Actor name slot does not follow the chunk header.");
        return false;
    }
    const FString OriginalName = ANSI_TO_TCHAR(
        reinterpret_cast<const char*>(Old.GetData() + ChunkAbs + NameRel));
    if (Actor.Name == OriginalName)
    {
        Out.Append(Old.GetData() + ChunkAbs + NameRel, Offsets.NameCapacity);
    }
    else
    {
        if (Actor.Name.Len() + 1 > Offsets.NameCapacity)
        {
            OutError = FString::Printf(TEXT("Actor %d name '%s' does not fit its %d-byte slot."),
                ActorIndex, *Actor.Name, Offsets.NameCapacity);
            return false;
        }
        for (const TCHAR Char : Actor.Name)
            if (Char < 1 || Char > 127)
            {
                OutError = FString::Printf(TEXT("Actor %d name must use ASCII characters."), ActorIndex);
                return false;
            }
        Out.AddZeroed(Offsets.NameCapacity);
        for (int32 Char = 0; Char < Actor.Name.Len(); ++Char)
            Out[NameRel + Char] = uint8(Actor.Name[Char]);
    }

    const int32 PlacementRel = int32(Offsets.Placement - Offsets.Chunk);
    if (!InChunk(Offsets.Placement, 52) || PlacementRel != Out.Num())
    {
        OutError = TEXT("Actor placement does not follow the name slot.");
        return false;
    }
    Out.Append(Old.GetData() + ChunkAbs + PlacementRel, 52);
    WriteU32(Out, PlacementRel + 48, uint32(Actor.MacroAnimations.Num()));
    for (const FKyaActorMacroAnimation& Macro : Actor.MacroAnimations)
    {
        AppendI32(Out, Macro.AnimationType);
        AppendI32(Out, Macro.MacroAnimationId);
    }

    AppendU32(Out, uint32(Actor.Behaviours.Num()));
    for (const FKyaActorBehaviour& Behaviour : Actor.Behaviours)
    {
        AppendU32(Out, uint32(Behaviour.Id));
        AppendU32(Out, uint32(IsGeneratedBehaviourPayload(Behaviour)
            ? GeneratedBehaviourPayloadSize(Behaviour) : Behaviour.Data.Num()));
    }
    for (const FKyaActorBehaviour& Behaviour : Actor.Behaviours)
    {
        if (IsGeneratedBehaviourPayload(Behaviour))
        {
            TArray<uint8> Payload;
            BuildGeneratedBehaviourPayload(Behaviour, Payload);
            Out.Append(Payload.GetData(), Payload.Num());
        }
        else if (!Behaviour.Data.IsEmpty())
        {
            Out.Append(Behaviour.Data.GetData(), Behaviour.Data.Num());
        }
    }

    if (Actor.Models.Num() > 16)
    {
        OutError = FString::Printf(TEXT("Actor %d has %d model references; the format allows at most 16."),
            ActorIndex, Actor.Models.Num());
        return false;
    }
    AppendU32(Out, uint32(Actor.Models.Num()));
    for (const FKyaActorModelReference& Model : Actor.Models)
    {
        AppendAlignedString(Out, ChunkAbs, Model.MeshName);
        AppendAlignedString(Out, ChunkAbs, Model.TextureName);
    }

    const int32 CommonRel = int32(Offsets.Common - Offsets.Chunk);
    if (!InChunk(Offsets.Common, 84))
    {
        OutError = TEXT("Actor common block is outside the chunk.");
        return false;
    }
    const int32 CommonStart = Out.Num();
    Out.Append(Old.GetData() + ChunkAbs + CommonRel, 84);
    WriteU32(Out, CommonStart + 80, uint32(Actor.Conditions.Num()));
    for (const FKyaScenarioCondition& Condition : Actor.Conditions)
    {
        AppendI32(Out, Condition.Combiner);
        AppendI32(Out, Condition.Variable);
        AppendI32(Out, Condition.Comparison);
        AppendI32(Out, Condition.Value);
    }

    const int32 ClassDataRel = int32(Offsets.ClassData - Offsets.Chunk);
    if (!InChunk(Offsets.ClassData, Offsets.ClassDataSize))
    {
        OutError = TEXT("Actor class data is outside the chunk.");
        return false;
    }
    const TConstArrayView<uint8> OriginalClassData(Old.GetData() + ChunkAbs + ClassDataRel,
        Offsets.ClassDataSize);
    if (NeedsPlatformClassDataRebuild(Actor, OriginalClassData))
    {
        TArray<uint8> RebuiltClassData;
        if (!BuildMovingPlatformClassData(Actor, RebuiltClassData, OutError)) return false;
        Out.Append(RebuiltClassData);
    }
    else if (NeedsTrapClassDataRebuild(Actor, OriginalClassData))
    {
        TArray<uint8> RebuiltClassData;
        if (!BuildTrapClassData(Actor, RebuiltClassData, OutError)) return false;
        Out.Append(RebuiltClassData);
    }
    else if (NeedsFighterClassDataRebuild(Actor, OriginalClassData))
    {
        TArray<uint8> RebuiltClassData;
        if (!BuildFighterClassData(Actor, OriginalClassData, ChunkAbs + Out.Num(), RebuiltClassData, OutError))
            return false;
        Out.Append(RebuiltClassData);
    }
    else if (NeedsShockerCmdClassDataRebuild(Actor, OriginalClassData))
    {
        TArray<uint8> RebuiltClassData;
        if (!BuildShockerCmdClassData(Actor, RebuiltClassData, OutError)) return false;
        Out.Append(RebuiltClassData);
    }
    else if (NeedsBasicBoxClassDataRebuild(Actor, OriginalClassData))
    {
        TArray<uint8> RebuiltClassData;
        if (!BuildBasicBoxClassData(Actor, OriginalClassData, RebuiltClassData, OutError)) return false;
        Out.Append(RebuiltClassData);
    }
    else if (NeedsBoxClassDataRebuild(Actor, OriginalClassData))
    {
        TArray<uint8> RebuiltClassData;
        if (!BuildBoxClassData(Actor, OriginalClassData, RebuiltClassData, OutError)) return false;
        Out.Append(RebuiltClassData);
    }
    else if (NeedsKimClassDataRebuild(Actor, OriginalClassData))
    {
        TArray<uint8> RebuiltClassData;
        if (!BuildKimClassData(Actor, OriginalClassData, RebuiltClassData, OutError)) return false;
        Out.Append(RebuiltClassData);
    }
    else if (NeedsPatternCmdClassDataRebuild(Actor, OriginalClassData))
    {
        TArray<uint8> RebuiltClassData;
        if (!BuildPatternCmdClassData(Actor, OriginalClassData, RebuiltClassData, OutError)) return false;
        Out.Append(RebuiltClassData);
    }
    else if (NeedsFogClassDataRebuild(Actor, OriginalClassData))
    {
        TArray<uint8> RebuiltClassData;
        if (!BuildFogClassData(Actor, OriginalClassData, RebuiltClassData, OutError)) return false;
        Out.Append(RebuiltClassData);
    }
    else if (NeedsBunchClassDataRebuild(Actor, OriginalClassData))
    {
        TArray<uint8> RebuiltClassData;
        if (!BuildBunchClassData(Actor, OriginalClassData, RebuiltClassData, OutError)) return false;
        Out.Append(RebuiltClassData);
    }
    else if (NeedsStillerClassDataRebuild(Actor, OriginalClassData))
    {
        TArray<uint8> RebuiltClassData;
        if (!BuildStillerClassData(Actor, OriginalClassData, RebuiltClassData, OutError)) return false;
        Out.Append(RebuiltClassData);
    }
    else if (NeedsShipClassDataRebuild(Actor, OriginalClassData))
    {
        TArray<uint8> RebuiltClassData;
        if (!BuildShipClassData(Actor, OriginalClassData, RebuiltClassData, OutError)) return false;
        Out.Append(RebuiltClassData);
    }
    else if (NeedsPunchingBallClassDataRebuild(Actor, OriginalClassData))
    {
        TArray<uint8> RebuiltClassData;
        if (!BuildPunchingBallClassData(Actor, OriginalClassData, RebuiltClassData, OutError)) return false;
        Out.Append(RebuiltClassData);
    }
    else if (NeedsAmortosCmdClassDataRebuild(Actor, OriginalClassData))
    {
        TArray<uint8> RebuiltClassData;
        if (!BuildAmortosCmdClassData(Actor, OriginalClassData, RebuiltClassData, OutError)) return false;
        Out.Append(RebuiltClassData);
    }
    else if (NeedsNoseMonsterClassDataRebuild(Actor, OriginalClassData))
    {
        TArray<uint8> RebuiltClassData;
        if (!BuildNoseMonsterClassData(Actor, OriginalClassData, RebuiltClassData, OutError)) return false;
        Out.Append(RebuiltClassData);
    }
    else if (NeedsWoofClassDataRebuild(Actor, OriginalClassData))
    {
        TArray<uint8> RebuiltClassData;
        if (!BuildWoofClassData(Actor, OriginalClassData, RebuiltClassData, OutError)) return false;
        Out.Append(RebuiltClassData);
    }
    else if (NeedsBlazerClassDataRebuild(Actor, OriginalClassData))
    {
        TArray<uint8> RebuiltClassData;
        if (!BuildBlazerClassData(Actor, OriginalClassData, RebuiltClassData, OutError)) return false;
        Out.Append(RebuiltClassData);
    }
    else if (NeedsEggClassDataRebuild(Actor, OriginalClassData))
    {
        TArray<uint8> RebuiltClassData;
        if (!BuildEggClassData(Actor, OriginalClassData, RebuiltClassData, OutError)) return false;
        Out.Append(RebuiltClassData);
    }
    else if (NeedsDcaClassDataRebuild(Actor, OriginalClassData))
    {
        TArray<uint8> RebuiltClassData;
        if (!BuildDcaClassData(Actor, OriginalClassData, RebuiltClassData, OutError)) return false;
        Out.Append(RebuiltClassData);
    }
    else if (NeedsTeleporterClassDataRebuild(Actor, OriginalClassData))
    {
        TArray<uint8> RebuiltClassData;
        if (!BuildTeleporterClassData(Actor, OriginalClassData, RebuiltClassData, OutError)) return false;
        Out.Append(RebuiltClassData);
    }
    else if (NeedsAtonClassDataRebuild(Actor, OriginalClassData))
    {
        TArray<uint8> RebuiltClassData;
        if (!BuildAtonClassData(Actor, OriginalClassData, RebuiltClassData, OutError)) return false;
        Out.Append(RebuiltClassData);
    }
    else if (NeedsShockerClassDataRebuild(Actor, OriginalClassData))
    {
        TArray<uint8> RebuiltClassData;
        if (!BuildShockerClassData(Actor, OriginalClassData, RebuiltClassData, OutError)) return false;
        Out.Append(RebuiltClassData);
    }
    else if (NeedsWindClassDataRebuild(Actor, OriginalClassData))
    {
        TArray<uint8> RebuiltClassData;
        if (!BuildWindClassData(Actor, OriginalClassData, RebuiltClassData, OutError)) return false;
        Out.Append(RebuiltClassData);
    }
    else if (Offsets.ClassDataSize > 0)
    {
        Out.Append(OriginalClassData.GetData(), Offsets.ClassDataSize);
    }

    WriteU32(Out, 4, uint32(Out.Num()));
    WriteU32(Out, 8, uint32(ActorIndex));
    return true;
}

bool RebuildActorStream(UKyaLevelBank& Bank, const TArray<int32>& Sources, FString& OutError,
    bool bUpdateAllocations = true)
{
    FActorStream Stream;
    if (!LocateActorStream(Bank, Stream, OutError)) return false;
    if (Sources.Num() != Bank.SceneEnvironment.Actors.Num())
    {
        OutError = TEXT("The structural source map does not match the actor array.");
        return false;
    }

    const FKyaBankEntry Scene = Bank.Entries[Stream.SceneEntryIndex];
    const int64 SceneStart = Scene.Offset;
    const int64 SceneEnd = Scene.Offset + Scene.Size;
    const TArray<uint8> Old = Bank.BankBytes;

    TArray<uint8> NewScene;
    NewScene.Reserve(Scene.Size + 256);
    NewScene.Append(Old.GetData() + SceneStart, int32(Stream.CountPos));
    const int32 CountOffset = NewScene.Num();
    NewScene.AddZeroed(4);
    WriteU32(NewScene, CountOffset, uint32(Bank.SceneEnvironment.Actors.Num()));

    for (int32 Index = 0; Index < Bank.SceneEnvironment.Actors.Num(); ++Index)
    {
        const int32 Source = Sources[Index];
        if (!Stream.Actors.IsValidIndex(Source))
        {
            OutError = FString::Printf(TEXT("Actor %d has no serialized source chunk."), Index);
            return false;
        }
        const FKyaActorOffsets& SourceOffsets = Stream.Actors[Source];
        const uint32 SourceSize = ReadU32(Old, SceneStart + SourceOffsets.Chunk + 4);
        if (SourceSize < 16 || SourceOffsets.Chunk + int64(SourceSize) > Scene.Size)
        {
            OutError = FString::Printf(TEXT("Actor source chunk %d is invalid."), Source);
            return false;
        }
        const int32 ChunkStart = NewScene.Num();
        const TConstArrayView<uint8> SourceClassData(
            Old.GetData() + SceneStart + SourceOffsets.ClassData, SourceOffsets.ClassDataSize);
        if (NeedsActorChunkRebuild(Bank.SceneEnvironment.Actors[Index], SourceOffsets, SourceClassData))
        {
            TArray<uint8> Rebuilt;
            if (!BuildActorChunk(Old, SceneStart, SourceOffsets, Bank.SceneEnvironment.Actors[Index], Index,
                Rebuilt, OutError)) return false;
            NewScene.Append(Rebuilt);
        }
        else
        {
            NewScene.Append(Old.GetData() + SceneStart + SourceOffsets.Chunk, int32(SourceSize));
        }
        WriteU32(NewScene, ChunkStart + 8, uint32(Index));

        const FString& Name = Bank.SceneEnvironment.Actors[Index].Name;
        if (Name.Len() + 1 > SourceOffsets.NameCapacity)
        {
            OutError = FString::Printf(TEXT("Actor %d name '%s' does not fit its %d-byte slot."),
                Index, *Name, SourceOffsets.NameCapacity);
            return false;
        }
        const int32 NameOffset = ChunkStart + int32(SourceOffsets.Name - SourceOffsets.Chunk);
        FMemory::Memzero(NewScene.GetData() + NameOffset, SourceOffsets.NameCapacity);
        for (int32 Char = 0; Char < Name.Len(); ++Char)
            NewScene[NameOffset + Char] = uint8(Name[Char]);
    }
    if (Stream.TailSize > 0)
        NewScene.Append(Old.GetData() + SceneStart + Stream.ActorEnd, int32(Stream.TailSize));

    const int64 Delta = NewScene.Num() - Scene.Size;
    TArray<uint8> Out;
    Out.Reserve(Old.Num() + int32(Delta));
    Out.Append(Old.GetData(), int32(SceneStart));
    Out.Append(NewScene);
    Out.Append(Old.GetData() + SceneEnd, Old.Num() - int32(SceneEnd));

    const uint64 Table = uint64(ReadU32(Out, 0x30)) + 8;
    if (Table + uint64(Bank.Entries.Num()) * 16 > uint64(SceneStart))
    {
        OutError = TEXT("The bank entry table overlaps the scene data; the actor stream cannot be relocated safely.");
        return false;
    }
    TArray<FKyaBankEntry> NewEntries = Bank.Entries;
    for (int32 Index = 0; Index < NewEntries.Num(); ++Index)
    {
        FKyaBankEntry Entry = Bank.Entries[Index];
        if (Index == Stream.SceneEntryIndex)
        {
            Entry.Offset = SceneStart;
            Entry.Size = NewScene.Num();
        }
        else if (Entry.Offset >= SceneEnd)
        {
            Entry.Offset += Delta;
        }
        else if (Entry.Offset + Entry.Size > SceneStart)
        {
            OutError = TEXT("A bank entry overlaps the scene data; the actor stream cannot be relocated safely.");
            return false;
        }
        WriteU32(Out, int64(Table) + Index * 16, uint32(Entry.Offset));
        WriteU32(Out, int64(Table) + Index * 16 + 4, uint32(Entry.Size));
        NewEntries[Index] = Entry;
    }

    if (bUpdateAllocations && !UpdateClassAllocations(Bank, OutError)) return false;
    const uint32 CommonType = ReadU32(Old, 0x14);
    if (CommonType == 0 && Out.Num() >= 0x38)
    {
        const uint64 TypeTable = uint64(ReadU32(Old, 0x34)) + 8;
        if (TypeTable >= uint64(SceneEnd))
            WriteU32(Out, 0x34, uint32(int64(TypeTable) + Delta - 8));
    }
    for (int32 Index = 0; Index < Bank.SceneEnvironment.Actors.Num(); ++Index)
        Bank.SceneEnvironment.Actors[Index].Index = Index;
    Bank.Entries = MoveTemp(NewEntries);
    Bank.BankBytes = MoveTemp(Out);
    Bank.EntryCount = Bank.Entries.Num();
    return true;
}

void RemapValue(int64& Value, const FString& Path, const TMap<int32, int32>& Remap,
    TArray<FString>* OutDangling)
{
    if (Value < 0 || Value > MAX_int32) return;
    if (const int32* Mapped = Remap.Find(int32(Value)))
    {
        if (*Mapped < 0)
        {
            if (OutDangling) OutDangling->Add(Path);
        }
        else Value = *Mapped;
    }
}

void RemapStruct(const UScriptStruct* Struct, void* Base, const FString& Path,
    const TMap<int32, int32>& Remap, TArray<FString>* OutDangling, bool bSkipActorArray)
{
    for (TFieldIterator<FProperty> It(Struct); It; ++It)
    {
        FProperty* Property = *It;
        const FString Name = Property->GetName();
        if (bSkipActorArray && Struct == FKyaSceneEnvironment::StaticStruct() &&
            Name == GET_MEMBER_NAME_CHECKED(FKyaSceneEnvironment, Actors)) continue;
        const FString ChildPath = Path.IsEmpty() ? Name : FString::Printf(TEXT("%s.%s"), *Path, *Name);
        if (!PropertyIsActive(Property, Struct, Base)) continue;
        void* Value = Property->ContainerPtrToValuePtr<void>(Base);

        const bool bActorReference = Name.Contains(TEXT("ActorRef")) ||
            Name.Contains(TEXT("ActorIndex")) || Name.Contains(TEXT("ActorId")) ||
            Name.Contains(TEXT("Actors")) || Name.EndsWith(TEXT("Actor")) || Name == TEXT("TargetRef");
        if (bActorReference)
        {
            if (FIntProperty* IntProperty = CastField<FIntProperty>(Property))
            {
                int64 Number = IntProperty->GetPropertyValue(Value);
                RemapValue(Number, ChildPath, Remap, OutDangling);
                IntProperty->SetPropertyValue(Value, int32(Number));
            }
            else if (FInt64Property* Int64Property = CastField<FInt64Property>(Property))
            {
                int64 Number = Int64Property->GetPropertyValue(Value);
                RemapValue(Number, ChildPath, Remap, OutDangling);
                Int64Property->SetPropertyValue(Value, Number);
            }
            else if (FArrayProperty* Array = CastField<FArrayProperty>(Property))
            {
                if (FIntProperty* Inner = CastField<FIntProperty>(Array->Inner))
                {
                    FScriptArrayHelper Helper(Array, Value);
                    for (int32 Index = 0; Index < Helper.Num(); ++Index)
                    {
                        int64 Number = Inner->GetPropertyValue(Helper.GetRawPtr(Index));
                        RemapValue(Number, FString::Printf(TEXT("%s[%d]"), *ChildPath, Index), Remap, OutDangling);
                        Inner->SetPropertyValue(Helper.GetRawPtr(Index), int32(Number));
                    }
                }
            }
        }

        if (FStructProperty* StructProperty = CastField<FStructProperty>(Property))
        {
            RemapStruct(StructProperty->Struct, Value, ChildPath, Remap, OutDangling, false);
        }
        else if (FArrayProperty* Array = CastField<FArrayProperty>(Property))
        {
            if (FStructProperty* Inner = CastField<FStructProperty>(Array->Inner))
            {
                FScriptArrayHelper Helper(Array, Value);
                for (int32 Index = 0; Index < Helper.Num(); ++Index)
                    RemapStruct(Inner->Struct, Helper.GetRawPtr(Index),
                        FString::Printf(TEXT("%s[%d]"), *ChildPath, Index), Remap, OutDangling, false);
            }
        }
    }
}

void RemapSceneReferences(const FKyaSceneEnvironment& Source, const FKyaSceneConfiguration& SourceConfig,
    FKyaSceneEnvironment& Target, FKyaSceneConfiguration& TargetConfig,
    const TMap<int32, int32>& Remap, TArray<FString>* OutDangling)
{
    Target = Source;
    TargetConfig = SourceConfig;
    RemapStruct(FKyaSceneEnvironment::StaticStruct(), &Target, TEXT("Scene"), Remap, OutDangling, true);
    RemapStruct(FKyaSceneConfiguration::StaticStruct(), &TargetConfig, TEXT("Configuration"), Remap,
        OutDangling, false);
}

bool CanApply(const UKyaLevelBank& Bank, FString& OutError)
{
    if (!ValidateActorIdentity(Bank, OutError)) return false;
    FActorStream Stream;
    if (!LocateActorStream(Bank, Stream, OutError)) return false;
    if (Stream.Actors.Num() != Bank.SceneEnvironment.Actors.Num())
    {
        OutError = TEXT("The serialized actor count differs from the asset. Apply structural actor commands "
            "(create, duplicate, delete) so the bank's actor stream is rebuilt before using a template.");
        return false;
    }
    TMap<int32, int32> Counts;
    for (const FKyaActorPlacement& Actor : Bank.SceneEnvironment.Actors) Counts.FindOrAdd(int32(Actor.ClassId))++;
    for (const TPair<int32, int32>& Pair : Counts)
        if (!Bank.SceneConfiguration.ActorClassAllocations.ContainsByPredicate(
            [&Pair](const FKyaActorClassAllocation& Allocation) { return Allocation.ClassId == Pair.Key; }))
        {
            OutError = FString::Printf(TEXT("Actor class %d has no scene allocation entry."), Pair.Key);
            return false;
        }
    return true;
}
} // namespace

bool KyaBankSupportsStructuralEdit(const UKyaLevelBank& Bank, FString& OutReason)
{
    return CanApply(Bank, OutReason);
}

bool KyaRebuildActorChunks(UKyaLevelBank& Bank, bool& bChanged, FString& OutError)
{
    bChanged = false;
    FActorStream Stream;
    if (!LocateActorStream(Bank, Stream, OutError)) return false;
    if (Stream.Actors.Num() != Bank.SceneEnvironment.Actors.Num())
    {
        OutError = TEXT("The actor array does not match the serialized stream.");
        return false;
    }
    const FKyaBankEntry& SceneEntry = Bank.Entries[Stream.SceneEntryIndex];
    // Refresh raw typed payloads whose layout is regenerated from their values so
    // validation and the write pass see the same sizes as the rebuilt chunk.
    for (FKyaActorPlacement& Actor : Bank.SceneEnvironment.Actors)
        for (FKyaActorBehaviour& Behaviour : Actor.Behaviours)
            if (IsGeneratedBehaviourPayload(Behaviour))
            {
                TArray<uint8> Payload;
                BuildGeneratedBehaviourPayload(Behaviour, Payload);
                Behaviour.Data = Payload;
            }
    TArray<int32> Sources;
    for (int32 Index = 0; Index < Stream.Actors.Num(); ++Index)
    {
        Sources.Add(Index);
        const FKyaActorOffsets& Offsets = Stream.Actors[Index];
        if (Offsets.ClassData < 0 || Offsets.ClassDataSize < 0 ||
            Offsets.ClassData + Offsets.ClassDataSize > Bank.BankBytes.Num() - SceneEntry.Offset)
        {
            OutError = FString::Printf(TEXT("Actor %d class data lies outside the scene entry."), Index);
            return false;
        }
        const TConstArrayView<uint8> ClassData(
            Bank.BankBytes.GetData() + SceneEntry.Offset + Offsets.ClassData, Offsets.ClassDataSize);
        bChanged |= NeedsActorChunkRebuild(Bank.SceneEnvironment.Actors[Index], Offsets, ClassData);
    }
    return !bChanged || RebuildActorStream(Bank, Sources, OutError, false);
}

bool KyaResizeActorConditions(UKyaLevelBank& Bank, bool& bChanged, FString& OutError)
{
    return KyaRebuildActorChunks(Bank, bChanged, OutError);
}

bool KyaActorChunkNeedsRebuild(const FKyaActorPlacement& Actor, const FKyaActorOffsets& Offsets,
    TConstArrayView<uint8> ClassData)
{
    return NeedsActorChunkRebuild(Actor, Offsets, ClassData);
}

FKyaStructuralWriteResult KyaDuplicateActorRecord(UKyaLevelBank& Bank, int32 SourceActorIndex)
{
    return KyaCreateActorRecord(Bank, SourceActorIndex);
}

FKyaStructuralWriteResult KyaCreateActorRecord(UKyaLevelBank& Bank, int32 TemplateActorIndex)
{
    FKyaStructuralWriteResult Result;
    FActorStream Stream;
    if (!CanApply(Bank, Result.Error) || !LocateActorStream(Bank, Stream, Result.Error)) return Result;
    if (!Bank.SceneEnvironment.Actors.IsValidIndex(TemplateActorIndex))
    {
        Result.Error = TEXT("The template actor no longer exists.");
        return Result;
    }

    const TArray<uint8> OldBytes = Bank.BankBytes;
    const TArray<FKyaBankEntry> OldEntries = Bank.Entries;
    const TArray<FKyaActorClassAllocation> OldAllocations = Bank.SceneConfiguration.ActorClassAllocations;

    const FKyaActorOffsets& Template = Stream.Actors[TemplateActorIndex];
    const int32 NewIndex = Bank.SceneEnvironment.Actors.Num();
    FKyaActorPlacement Copy = Bank.SceneEnvironment.Actors[TemplateActorIndex];
    Copy.Name = MakeActorName(Bank, TemplateActorIndex, Template.NameCapacity, true);
    Copy.Index = NewIndex;
    Bank.SceneEnvironment.Actors.Add(Copy);

    TArray<int32> Sources;
    Sources.Reserve(NewIndex + 1);
    for (int32 Index = 0; Index < NewIndex; ++Index) Sources.Add(Index);
    Sources.Add(TemplateActorIndex);

    if (!RebuildActorStream(Bank, Sources, Result.Error))
    {
        Bank.BankBytes = OldBytes;
        Bank.Entries = OldEntries;
        Bank.SceneConfiguration.ActorClassAllocations = OldAllocations;
        Bank.SceneEnvironment.Actors.RemoveAt(NewIndex);
        return Result;
    }
    // The imported baseline is indexed by the old actor array; a structural change
    // invalidates it until the bank is reimported.
    Bank.bHasImportedBaseline = false;
    Bank.ImportedActors.Reset();
    Bank.ImportedWaypoints.Reset();
    Bank.ImportedPaths.Reset();
    Result.bSuccess = true;
    Result.ActorIndex = NewIndex;
    return Result;
}

bool KyaCanDeleteActorRecord(const UKyaLevelBank& Bank, int32 ActorIndex, FString& OutReason)
{
    if (!CanApply(Bank, OutReason)) return false;
    if (!Bank.SceneEnvironment.Actors.IsValidIndex(ActorIndex))
    {
        OutReason = TEXT("The actor record no longer exists.");
        return false;
    }
    TMap<int32, int32> Remap;
    for (int32 Index = 0; Index < Bank.SceneEnvironment.Actors.Num(); ++Index)
        Remap.Add(Index, Index < ActorIndex ? Index : (Index == ActorIndex ? -1 : Index - 1));
    TArray<FString> Dangling;
    for (int32 Index = 0; Index < Bank.SceneEnvironment.Actors.Num(); ++Index)
    {
        if (Index == ActorIndex) continue;
        FKyaActorPlacement Copy = Bank.SceneEnvironment.Actors[Index];
        KyaRemapActorReferences(Copy, Remap, &Dangling);
    }
    if (!Dangling.IsEmpty())
    {
        OutReason = FString::Printf(TEXT("Deleting this actor would leave typed actor references (%s). "
            "Clear or retarget them first."), *FString::Join(Dangling, TEXT(", ")));
        return false;
    }
    FKyaSceneEnvironment SceneCopy;
    FKyaSceneConfiguration ConfigCopy;
    RemapSceneReferences(Bank.SceneEnvironment, Bank.SceneConfiguration, SceneCopy, ConfigCopy, Remap, &Dangling);
    if (!Dangling.IsEmpty())
    {
        OutReason = FString::Printf(TEXT("Deleting this actor would leave scene-level actor references (%s). "
            "Clear or retarget them first."), *FString::Join(Dangling, TEXT(", ")));
        return false;
    }
    return true;
}

FKyaStructuralWriteResult KyaDeleteActorRecord(UKyaLevelBank& Bank, int32 ActorIndex)
{
    FKyaStructuralWriteResult Result;
    if (!KyaCanDeleteActorRecord(Bank, ActorIndex, Result.Error)) return Result;

    const TArray<uint8> OldBytes = Bank.BankBytes;
    const TArray<FKyaBankEntry> OldEntries = Bank.Entries;
    const TArray<FKyaActorPlacement> OldActors = Bank.SceneEnvironment.Actors;
    const FKyaSceneEnvironment OldScene = Bank.SceneEnvironment;
    const FKyaSceneConfiguration OldConfig = Bank.SceneConfiguration;

    TMap<int32, int32> Remap;
    for (int32 Index = 0; Index < OldActors.Num(); ++Index)
        Remap.Add(Index, Index < ActorIndex ? Index : (Index == ActorIndex ? -1 : Index - 1));

    TArray<FKyaActorPlacement> Remaining;
    TArray<int32> Sources;
    Remaining.Reserve(OldActors.Num() - 1);
    Sources.Reserve(OldActors.Num() - 1);
    for (int32 Index = 0; Index < OldActors.Num(); ++Index)
    {
        if (Index == ActorIndex) continue;
        FKyaActorPlacement Copy = OldActors[Index];
        TArray<FString> Dangling;
        KyaRemapActorReferences(Copy, Remap, &Dangling);
        if (!Dangling.IsEmpty())
        {
            Result.Error = FString::Printf(TEXT("Deleting this actor would leave typed actor references (%s)."),
                *FString::Join(Dangling, TEXT(", ")));
            return Result;
        }
        Copy.Index = Remaining.Num();
        Remaining.Add(MoveTemp(Copy));
        Sources.Add(Index);
    }

    Bank.SceneEnvironment.Actors = MoveTemp(Remaining);
    RemapStruct(FKyaSceneEnvironment::StaticStruct(), &Bank.SceneEnvironment, TEXT("Scene"), Remap, nullptr, true);
    RemapStruct(FKyaSceneConfiguration::StaticStruct(), &Bank.SceneConfiguration, TEXT("Configuration"), Remap,
        nullptr, false);
    if (!RebuildActorStream(Bank, Sources, Result.Error))
    {
        Bank.BankBytes = OldBytes;
        Bank.Entries = OldEntries;
        Bank.SceneEnvironment = OldScene;
        Bank.SceneConfiguration = OldConfig;
        return Result;
    }
    Bank.bHasImportedBaseline = false;
    Bank.ImportedActors.Reset();
    Bank.ImportedWaypoints.Reset();
    Bank.ImportedPaths.Reset();
    Result.bSuccess = true;
    Result.ActorIndex = ActorIndex;
    return Result;
}

bool KyaRemapActorReferences(FKyaActorPlacement& Placement, const TMap<int32, int32>& Remap,
    TArray<FString>* OutDangling)
{
    RemapStruct(FKyaActorPlacement::StaticStruct(), &Placement, FString(), Remap, OutDangling, false);
    return !OutDangling || OutDangling->IsEmpty();
}
