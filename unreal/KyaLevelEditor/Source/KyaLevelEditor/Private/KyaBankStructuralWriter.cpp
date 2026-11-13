#include "KyaBankStructuralWriter.h"

#include "KyaLevelBank.h"
#include "KyaCameraStream.h"
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
        NewScene.Append(Old.GetData() + SceneStart + SourceOffsets.Chunk, int32(SourceSize));
        const TArray<FKyaScenarioCondition>& Conditions = Bank.SceneEnvironment.Actors[Index].Conditions;
        if (Conditions.Num() != SourceOffsets.ConditionCount)
        {
            const int32 ConditionStart = ChunkStart + int32(SourceOffsets.Conditions - SourceOffsets.Chunk);
            const int32 OldBytes = SourceOffsets.ConditionCount * 16;
            const int32 NewBytes = Conditions.Num() * 16;
            NewScene.RemoveAt(ConditionStart, OldBytes);
            NewScene.InsertZeroed(ConditionStart, NewBytes);
            WriteU32(NewScene, ConditionStart - 4, uint32(Conditions.Num()));
            WriteU32(NewScene, ChunkStart + 4, uint32(int64(SourceSize) + NewBytes - OldBytes));
            // The normal export pass writes the typed condition values.
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

bool KyaResizeActorConditions(UKyaLevelBank& Bank, bool& bChanged, FString& OutError)
{
    bChanged = false;
    FActorStream Stream;
    if (!LocateActorStream(Bank, Stream, OutError)) return false;
    if (Stream.Actors.Num() != Bank.SceneEnvironment.Actors.Num())
    {
        OutError = TEXT("The actor array does not match the serialized stream.");
        return false;
    }
    TArray<int32> Sources;
    for (int32 Index = 0; Index < Stream.Actors.Num(); ++Index)
    {
        Sources.Add(Index);
        bChanged |= Stream.Actors[Index].ConditionCount != Bank.SceneEnvironment.Actors[Index].Conditions.Num();
    }
    return !bChanged || RebuildActorStream(Bank, Sources, OutError, false);
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
