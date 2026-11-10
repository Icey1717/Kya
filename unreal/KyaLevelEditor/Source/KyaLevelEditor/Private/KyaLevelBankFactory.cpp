#include "KyaLevelBankFactory.h"
#include "KyaLevelBank.h"
#include "KyaCameraStream.h"
#include "Misc/FeedbackContext.h"

static bool KyaPopulateFighterPrefix(FKyaActorPlacement& Actor, const uint8* Data, int32 DataSize, const FKyaFighterPrefixOffsets& Offsets)
{
    const int64 F = Offsets.Fixed;
    auto ReadU32 = [](const uint8* P) -> uint32 {
        return uint32(P[0]) | (uint32(P[1]) << 8) | (uint32(P[2]) << 16) | (uint32(P[3]) << 24);
    };
    auto ReadFloat = [&ReadU32, Data](int64 Offset) -> float {
        const uint32 Bits = ReadU32(Data + Offset);
        float Value;
        FMemory::Memcpy(&Value, &Bits, sizeof(Value));
        return Value;
    };
    Actor.FighterWeightB = ReadFloat(Offsets.Weight);
    Actor.FighterLifeMax = float(ReadU32(Data + Offsets.Autonomous));
    Actor.FighterGravityScale = ReadFloat(Offsets.Autonomous + 4);
    Actor.FighterDynamicExtField4 = ReadFloat(Offsets.Autonomous + 8);
    Actor.FighterDynamicExtField8 = ReadFloat(Offsets.Autonomous + 12);

                                                    Actor.FighterInputAnalyserFlag = int32(ReadU32(Data + F));
                                                    Actor.FighterCommandMask = int32(ReadU32(Data + F + 4));
                                                    Actor.FighterField7C0 = ReadFloat(F + 8);
                                                    Actor.FighterField7C4 = ReadFloat(F + 12);
                                                    Actor.FighterField7D4 = ReadFloat(F + 16);
                                                    Actor.FighterField7CC = ReadFloat(F + 20);
                                                    Actor.FighterAnatomy20 = ReadFloat(F + 24);
                                                    Actor.FighterAnatomy28 = ReadFloat(F + 28);
                                                    Actor.FighterAnatomy24 = ReadFloat(F + 32);
                                                    Actor.FighterAnatomy2C = ReadFloat(F + 36);
                                                    Actor.FighterField810A = FVector(ReadFloat(F + 40), ReadFloat(F + 44), ReadFloat(F + 48));
                                                    Actor.FighterField810B = FVector(ReadFloat(F + 52), ReadFloat(F + 56), ReadFloat(F + 60));
                                                    Actor.FighterField800 = int32(ReadU32(Data + F + 64));
                                                    Actor.FighterFightFlags = int32(ReadU32(Data + F + 68));
                                                    Actor.FighterField50C = ReadFloat(F + 72);
                                                    Actor.FighterField510 = ReadFloat(F + 76);
                                                    Actor.FighterField514 = ReadFloat(F + 80);
                                                    Actor.FighterField518 = int32(ReadU32(Data + F + 84));
                                                    Actor.FighterField58 = int32(ReadU32(Data + F + 88));
                                                    for (int32 Index = 0; Index < 8; ++Index)
                                                        Actor.FighterField51C.Add(int32(ReadU32(Data + F + 92 + Index * 4)));
                                                    for (int32 Index = 0; Index < 5; ++Index)
                                                        Actor.FighterField53C.Add(int32(ReadU32(Data + F + 124 + Index * 4)));
                                                    Actor.FighterStaticMeshTextureIndex = int32(ReadU32(Data + F + 144));
                                                    Actor.FighterStaticMeshMeshIndex = int32(ReadU32(Data + F + 148));
                                                    Actor.FighterField5F0 = int32(ReadU32(Data + F + 152));
                                                    Actor.FighterWeaponActorIndex = int32(ReadU32(Data + Offsets.WeaponIndex));
                                                    int64 BlowPos = Offsets.Blows;
                                                    const uint32 BlowCount = ReadU32(Data + BlowPos); BlowPos += 4;
                                                    for (uint32 Index = 0; Index < BlowCount; ++Index)
                                                    {
                                                        FKyaFighterBlowOffsets Off;
                                                        if (!KyaReadFighterBlowOffsets(MakeArrayView(Data, DataSize), BlowPos, int64(DataSize), Off)) return false;
                                                        FKyaFighterBlow& Blow = Actor.FighterBlows.AddDefaulted_GetRef();
                                                        Blow.Name = ANSI_TO_TCHAR(reinterpret_cast<const char*>(Data + Off.Name));
                                                        Blow.NameCapacity = Off.NameCapacity;
                                                        Blow.Field4A = int32(ReadU32(Data + Off.Field4));
                                                        Blow.Field4B = int32(ReadU32(Data + Off.Field4 + 4));
                                                        Blow.Field4C = int32(ReadU32(Data + Off.Field4 + 8));
                                                        Blow.Field8 = ReadFloat(Off.Field8);
                                                        Blow.FieldC = ReadFloat(Off.Field8 + 4);
                                                        Blow.Field10 = ReadFloat(Off.Field8 + 8);
                                                        Blow.Damage = ReadFloat(Off.Field8 + 12);
                                                        Blow.Field18 = ReadFloat(Off.Field8 + 16);
                                                        Blow.Field20 = FVector(ReadFloat(Off.Field20), ReadFloat(Off.Field20 + 4), ReadFloat(Off.Field20 + 8));
                                                        Blow.Field30 = FVector(ReadFloat(Off.Field30), ReadFloat(Off.Field30 + 4), ReadFloat(Off.Field30 + 8));
                                                        Blow.Field40 = int32(ReadU32(Data + Off.Field40));
                                                        for (int32 Sub = 0; Sub < Off.SubObjCount; ++Sub)
                                                        {
                                                            FKyaFighterBlowSubObj& Item = Blow.SubObjs.AddDefaulted_GetRef();
                                                            const int64 S = Off.SubObjs + int64(Sub) * 28;
                                                            Item.BoneA = int32(ReadU32(Data + S));
                                                            Item.BoneB = int32(ReadU32(Data + S + 4));
                                                            Item.MaterialId = int32(ReadU32(Data + S + 8));
                                                            Item.Field14 = int32(ReadU32(Data + S + 12));
                                                            Item.Flags = int32(ReadU32(Data + S + 16));
                                                            Item.Field1C = ReadFloat(S + 20);
                                                            Item.Field20 = int32(ReadU32(Data + S + 24));
                                                        }
                                                        Blow.BoneRefA = int32(ReadU32(Data + Off.BoneRefs));
                                                        Blow.BoneRefB = int32(ReadU32(Data + Off.BoneRefs + 4));
                                                        Blow.CanActivateRange = ReadFloat(Off.CanActivateRange);
                                                        for (int32 Stage = 0; Stage < 3; ++Stage)
                                                        {
                                                            FKyaFighterStage& Item = Blow.Stages.AddDefaulted_GetRef();
                                                            const int64 S = Off.Stages + int64(Stage) * 28;
                                                            Item.AnimId = int32(ReadU32(Data + S));
                                                            Item.Field4 = ReadFloat(S + 4);
                                                            Item.Field8 = ReadFloat(S + 8);
                                                            Item.Field10 = FVector(ReadFloat(S + 12), ReadFloat(S + 16), ReadFloat(S + 20));
                                                            Item.Flags = int32(ReadU32(Data + S + 24));
                                                        }
                                                    }
                                                    int64 GrabPos = Offsets.Grabs;
                                                    const uint32 GrabCount = ReadU32(Data + GrabPos); GrabPos += 4;
                                                    for (uint32 Index = 0; Index < GrabCount; ++Index)
                                                    {
                                                        FKyaFighterGrabOffsets Off;
                                                        if (!KyaReadFighterGrabOffsets(MakeArrayView(Data, DataSize), GrabPos, int64(DataSize), Off)) return false;
                                                        FKyaFighterGrab& Grab = Actor.FighterGrabs.AddDefaulted_GetRef();
                                                        Grab.Name = ANSI_TO_TCHAR(reinterpret_cast<const char*>(Data + Off.Name));
                                                        Grab.NameCapacity = Off.NameCapacity;
                                                        Grab.Field4A = int32(ReadU32(Data + Off.Field4));
                                                        Grab.Field4B = int32(ReadU32(Data + Off.Field4 + 4));
                                                        Grab.Field4C = int32(ReadU32(Data + Off.Field4 + 8));
                                                        Grab.Field8 = ReadFloat(Off.Field8); Grab.FieldC = ReadFloat(Off.Field8 + 4); Grab.Field10 = ReadFloat(Off.Field8 + 8);
                                                        Grab.Field14 = int32(ReadU32(Data + Off.Field14));
                                                        Grab.Field20 = FVector(ReadFloat(Off.Field20), ReadFloat(Off.Field20 + 4), ReadFloat(Off.Field20 + 8));
                                                        Grab.Field30 = ReadFloat(Off.Field30); Grab.Field34 = ReadFloat(Off.Field34);
                                                        Grab.Field38 = int32(ReadU32(Data + Off.Field38)); Grab.Field3C = ReadFloat(Off.Field3C);
                                                        Grab.Field60 = ReadFloat(Off.Field60); Grab.Field40 = ReadFloat(Off.Field40);
                                                        Grab.Field44 = ReadFloat(Off.Field44); Grab.Field48 = ReadFloat(Off.Field48);
                                                        Grab.Field50 = FVector(ReadFloat(Off.Field50), ReadFloat(Off.Field50 + 4), ReadFloat(Off.Field50 + 8));
                                                        Grab.Field68 = int32(ReadU32(Data + Off.Field68));
                                                        Grab.Skip = int32(ReadU32(Data + Off.Field68 + 4));
                                                        for (int32 Stage = 0; Stage < 5; ++Stage)
                                                        {
                                                            FKyaFighterStage& Item = Grab.Stages.AddDefaulted_GetRef();
                                                            const int64 S = Off.Stages + int64(Stage) * 28;
                                                            Item.AnimId = int32(ReadU32(Data + S)); Item.Field4 = ReadFloat(S + 4); Item.Field8 = ReadFloat(S + 8);
                                                            Item.Field10 = FVector(ReadFloat(S + 12), ReadFloat(S + 16), ReadFloat(S + 20));
                                                            Item.Flags = int32(ReadU32(Data + S + 24));
                                                        }
                                                    }
                                                    int64 ReactPos = Offsets.GrabReacts;
                                                    const uint32 ReactCount = ReadU32(Data + ReactPos); ReactPos += 4;
                                                    for (uint32 Index = 0; Index < ReactCount; ++Index)
                                                    {
                                                        FKyaFighterGrabReact& React = Actor.FighterGrabReacts.AddDefaulted_GetRef();
                                                        const int64 NameStart = ReactPos;
                                                        if (!KyaSkipString(MakeArrayView(Data, DataSize), ReactPos, int64(DataSize))) return false;
                                                        React.Name = ANSI_TO_TCHAR(reinterpret_cast<const char*>(Data + NameStart));
                                                        React.NameCapacity = int32(ReactPos - NameStart);
                                                        React.Field4 = int32(ReadU32(Data + ReactPos));
                                                        React.Field8 = int32(ReadU32(Data + ReactPos + 4));
                                                        ReactPos += 8;
                                                    }
                                                    int64 ComboPos = Offsets.Combos;
                                                    const uint32 ComboCount = ReadU32(Data + ComboPos); ComboPos += 4;
                                                    for (uint32 Index = 0; Index < ComboCount; ++Index)
                                                    {
                                                        FKyaFighterComboOffsets Off;
                                                        if (!KyaReadFighterComboOffsets(MakeArrayView(Data, DataSize), ComboPos, int64(DataSize), Off)) return false;
                                                        FKyaFighterCombo& Combo = Actor.FighterCombos.AddDefaulted_GetRef();
                                                        Combo.Name = ANSI_TO_TCHAR(reinterpret_cast<const char*>(Data + Off.Name));
                                                        Combo.NameCapacity = Off.NameCapacity;
                                                        Combo.Field4A = int32(ReadU32(Data + Off.Field4));
                                                        Combo.Field4B = int32(ReadU32(Data + Off.Field4 + 4));
                                                        Combo.Field8 = ReadFloat(Off.Field8);
                                                        for (int32 Input = 0; Input < Off.InputCount; ++Input)
                                                            Combo.Inputs.Add(int32(ReadU32(Data + Off.Inputs + Input * 4)));
                                                        Combo.Field2 = int32(ReadU32(Data + Off.Field2));
                                                        Combo.Field3 = int32(ReadU32(Data + Off.Field3));
                                                        Combo.FieldC = ReadFloat(Off.FieldC);
                                                        Combo.Action = ANSI_TO_TCHAR(reinterpret_cast<const char*>(Data + Off.Action));
                                                        Combo.ActionCapacity = Off.ActionCapacity;
                                                        for (int32 Branch = 0; Branch < Off.BranchOffsets.Num(); ++Branch)
                                                        {
                                                            Combo.Branches.Add(ANSI_TO_TCHAR(reinterpret_cast<const char*>(Data + Off.BranchOffsets[Branch])));
                                                            Combo.BranchCapacities.Add(Off.BranchCapacities[Branch]);
                                                        }
                                                    }
    return true;
}

UKyaLevelBankFactory::UKyaLevelBankFactory()
{
    SupportedClass = UKyaLevelBank::StaticClass();
    bCreateNew = false;
    bEditorImport = true;
    Formats.Add(TEXT("bnk;Kya level bank"));
}

UObject* UKyaLevelBankFactory::FactoryCreateBinary(UClass* InClass, UObject* InParent,
    FName InName, EObjectFlags Flags, UObject* Context, const TCHAR* Type,
    const uint8*& Buffer, const uint8* BufferEnd, FFeedbackContext* Warn)
{
    const int64 Size = BufferEnd - Buffer;
    if (Size < 0x48 || Size > MAX_int32 || FMemory::Memcmp(Buffer + 8, "KNAB", 4) != 0)
    {
        Warn->Logf(ELogVerbosity::Error, TEXT("Not a valid Kya bank header."));
        return nullptr;
    }
    auto ReadU32 = [](const uint8* P) -> uint32 {
        return uint32(P[0]) | (uint32(P[1]) << 8) | (uint32(P[2]) << 16) | (uint32(P[3]) << 24);
    };
    // Packed banks need the Eden LZ77 unpacking and relocation pass before
    // their entry offsets can be interpreted as uncompressed data.
    if ((ReadU32(Buffer + 0x0c) & 1) != 0)
    {
        Warn->Logf(ELogVerbosity::Error, TEXT("Packed Kya banks are not supported yet."));
        return nullptr;
    }
    const uint32 Count = ReadU32(Buffer + 0x28);
    const uint64 Table = uint64(ReadU32(Buffer + 0x30)) + 8;
    if (Count > MAX_int32 || Table > uint64(Size) || uint64(Count) * 16 > uint64(Size) - Table)
    {
        Warn->Logf(ELogVerbosity::Error, TEXT("Invalid Kya bank entry table."));
        return nullptr;
    }
    const uint32 CommonType = ReadU32(Buffer + 0x14);
    const uint64 TypeTable = uint64(ReadU32(Buffer + 0x34)) + 8;
    if (CommonType == 0 && (TypeTable > uint64(Size) || uint64(Count) * 4 > uint64(Size) - TypeTable))
    {
        Warn->Logf(ELogVerbosity::Error, TEXT("Invalid Kya bank type table."));
        return nullptr;
    }
    TArray<FKyaBankEntry> Entries;
    Entries.Reserve(int32(Count));
    for (uint32 Index = 0; Index < Count; ++Index)
    {
        const uint8* Record = Buffer + Table + uint64(Index) * 16;
        FKyaBankEntry Entry;
        Entry.Offset = ReadU32(Record);
        Entry.Size = ReadU32(Record + 4);
        if (Entry.Offset > Size || Entry.Size > Size - Entry.Offset)
        {
            Warn->Logf(ELogVerbosity::Error, TEXT("Kya bank entry %u lies outside the file."), Index);
            return nullptr;
        }
        const uint32 Pair = CommonType ? CommonType : ReadU32(Buffer + TypeTable + uint64(Index) * 4);
        Entry.Subtype = Pair & 0xffff;
        Entry.Type = Pair >> 16;
        Entries.Add(Entry);
    }
    UKyaLevelBank* Bank = NewObject<UKyaLevelBank>(InParent, InClass, InName, Flags);
    Bank->ImportSchemaVersion = UKyaLevelBank::CurrentImportSchemaVersion;
    Bank->SourceFilename = CurrentFilename;
    Bank->EntryCount = int32(Count);
    Bank->Entries = MoveTemp(Entries);
    auto FindTypedEntry = [&Bank](int32 TypeId, int32 Ordinal) -> int32 {
        if (Ordinal < 0) return INDEX_NONE;
        for (int32 Index = 0; Index < Bank->Entries.Num(); ++Index)
            if (Bank->Entries[Index].Type == TypeId && Bank->Entries[Index].Subtype == 1 && Ordinal-- == 0)
                return Index;
        return INDEX_NONE;
    };
    for (const FKyaBankEntry& Entry : Bank->Entries)
    {
        if (Entry.Type != 6 || Entry.Subtype != 1 || Entry.Size < 24) continue;
        const uint8* Data = Buffer + Entry.Offset;
        int64 Pos = 16;
        const int32 HasCluster = int32(ReadU32(Data + Pos));
        Pos += 4;
        if (HasCluster)
        {
            if (Pos + 12 > Entry.Size) break;
            Bank->SceneMeshEntry = FindTypedEntry(4, int32(ReadU32(Data + Pos)));
            Bank->SceneTextureEntry = FindTypedEntry(5, int32(ReadU32(Data + Pos + 4)));
            Pos += 12;
        }
        if (Pos + 4 > Entry.Size) break;
        const int32 Background = int32(ReadU32(Data + Pos));
        Pos += 4;
        if (Background != -1 && Pos + 4 <= Entry.Size)
        {
            Bank->BackgroundMeshEntry = FindTypedEntry(4, Background);
            Bank->BackgroundTextureEntry = FindTypedEntry(5, int32(ReadU32(Data + Pos)));
            Pos += 4;
        }
        // CScene::Level_Setup follows the two mesh references. Its first 20
        // bytes are currently unused by the game, then come flags and fog.
        if (Pos + 48 <= Entry.Size)
        {
            FKyaSceneEnvironment& Environment = Bank->SceneEnvironment;
            Environment.BackgroundFlags = ReadU32(Data + Pos + 20);
            uint32 Bits = ReadU32(Data + Pos + 24);
            FMemory::Memcpy(&Environment.ClipDistance, &Bits, sizeof(Bits));
            Bits = ReadU32(Data + Pos + 28);
            FMemory::Memcpy(&Environment.FogParameter, &Bits, sizeof(Bits));
            const uint8* Color = Data + Pos + 32;
            Environment.FogColor = FColor(Color[0], Color[1], Color[2], Color[3]);
            Environment.FogFlags = ReadU32(Data + Pos + 36);
            Environment.MipmapL = int32(ReadU32(Data + Pos + 40));
            Environment.MipmapK = int32(ReadU32(Data + Pos + 44));
            if (Pos + 112 <= Entry.Size)
            {
                auto ReadFloat = [&](int64 Offset) {
                    uint32 FloatBits = ReadU32(Data + Offset);
                    float Value;
                    FMemory::Memcpy(&Value, &FloatBits, sizeof(Value));
                    return Value;
                };
                Environment.CameraFov = ReadFloat(Pos + 48);
                Environment.CameraPosition = FVector(ReadFloat(Pos + 52), ReadFloat(Pos + 56), ReadFloat(Pos + 60));
                Environment.CameraLookAt = FVector(ReadFloat(Pos + 64), ReadFloat(Pos + 68), ReadFloat(Pos + 72));
                Environment.CameraUp = FVector(ReadFloat(Pos + 76), ReadFloat(Pos + 80), ReadFloat(Pos + 84));
                Environment.CameraSectorId = int32(ReadU32(Data + Pos + 88));
                Environment.CameraField08 = int32(ReadU32(Data + Pos + 92));
                Environment.CameraFlags = ReadU32(Data + Pos + 96);
                Environment.CameraTargetOffset = FVector(ReadFloat(Pos + 100), ReadFloat(Pos + 104), ReadFloat(Pos + 108));
                int64 Conditions = 0;
                if (KyaFindCameraConditions(MakeArrayView(Data, int32(Entry.Size)), Pos, Conditions))
                {
                    if (Environment.CameraFlags & 2)
                    {
                        Environment.bHasCameraSwitchData = true;
                        int64 Switch = Pos + 112;
                        Environment.CameraSwitchActor = int32(ReadU32(Data + Switch)); Switch += 4;
                        TArray<int32>* Lists[] = { &Environment.CameraSwitchActorsA, &Environment.CameraSwitchActorsB };
                        for (TArray<int32>* List : Lists)
                        {
                            const uint32 ListCount = ReadU32(Data + Switch); Switch += 4;
                            for (uint32 Index = 0; Index < ListCount; ++Index, Switch += 4)
                                List->Add(int32(ReadU32(Data + Switch)));
                        }
                        Environment.CameraSwitchValueA = ReadFloat(Switch);
                        Environment.CameraSwitchModeA = int32(ReadU32(Data + Switch + 4));
                        Environment.CameraSwitchValueB = ReadFloat(Switch + 8);
                        Environment.CameraSwitchModeB = int32(ReadU32(Data + Switch + 12));
                        Environment.CameraSwitchValueC = ReadFloat(Switch + 16);
                    }
                    for (uint32 Index = 0; Index < ReadU32(Data + Conditions); ++Index)
                    {
                        const int64 Offset = Conditions + 4 + int64(Index) * 16;
                        FKyaCameraCondition& Condition = Environment.CameraConditions.AddDefaulted_GetRef();
                        Condition.Combiner = int32(ReadU32(Data + Offset));
                        Condition.ActorRef = int32(ReadU32(Data + Offset + 4));
                        Condition.Type = int32(ReadU32(Data + Offset + 8));
                        Condition.Value = ReadFloat(Offset + 12);
                    }
                }
                int64 Config = 0;
                if (KyaFindCameraConfig(MakeArrayView(Data, int32(Entry.Size)), Pos, Config))
                {
                    Environment.CameraConfigFlags = ReadU32(Data + Config);
                    Environment.CameraActorRefA = int32(ReadU32(Data + Config + 4));
                    Environment.CameraActorRefB = int32(ReadU32(Data + Config + 8));
                    Environment.CameraConfigAngle = ReadFloat(Config + 12);
                    if (ReadU32(Data + Config + 16) == 1)
                        for (int32 Index = 0; Index < 5; ++Index)
                            Environment.CameraOptionalConfigValues.Add(ReadFloat(Config + 20 + Index * 4));
                    const int64 Values = Config + 20 + (ReadU32(Data + Config + 16) == 1 ? 20 : 0);
                    Environment.CameraPitchAngles = FVector(ReadFloat(Values), ReadFloat(Values + 4), ReadFloat(Values + 8));
                    Environment.CameraOtherAngles = FVector(ReadFloat(Values + 12), ReadFloat(Values + 16), ReadFloat(Values + 20));
                    Environment.CameraDistances = FVector(ReadFloat(Values + 24), ReadFloat(Values + 28), ReadFloat(Values + 32));
                    int64 SceneValues = 0;
                    if (KyaFindSceneAfterCamera(MakeArrayView(Data, int32(Entry.Size)), Config, SceneValues))
                    {
                        if (ReadU32(Data + Values + 36) == 1)
                            for (int32 Index = 0; Index < 12; ++Index)
                                Environment.CameraExtraValues.Add(ReadFloat(Values + 40 + Index * 4));
                        Environment.Field118 = ReadFloat(SceneValues);
                        Environment.Field11C = ReadU32(Data + SceneValues + 4);
                        Environment.Count120 = ReadU32(Data + SceneValues + 8);
                        const int64 Map = SceneValues + 12;
                        int64 MapEnd = 0;
                        if (KyaFindMapEnd(MakeArrayView(Data, int32(Entry.Size)), Map, MapEnd))
                        {
                            Environment.MapField04 = int32(ReadU32(Data + Map));
                            Environment.MapField08 = ReadU32(Data + Map + 4);
                            Environment.MapAngle = ReadFloat(Map + 8);
                            const uint32 MapCount = ReadU32(Data + Map + 12);
                            for (uint32 Index = 0; Index < MapCount; ++Index)
                                Environment.MapEntries.Add(int32(ReadU32(Data + Map + 16 + Index * 4)));
                            Environment.MapField14 = int32(ReadU32(Data + MapEnd));
                            Environment.MapActorRef = int32(ReadU32(Data + MapEnd + 4));
                            Environment.DefaultTextureIndex = int32(ReadU32(Data + MapEnd + 8));
                            Environment.AlternateTextureIndex = int32(ReadU32(Data + MapEnd + 12));
                            Environment.DefaultMaterialIndex = int32(ReadU32(Data + MapEnd + 16));
                            if (MapEnd + 48 <= Entry.Size)
                            {
                                for (int32 Category = 0; Category < 7; ++Category)
                                    Environment.EffectPoolCounts.Add(ReadU32(Data + MapEnd + 20 + Category * 4));
                                TArray<FKyaConditionedOffsets> Blocks;
                                int64 ScenarioEnd = 0;
                                if (KyaReadConditionedOffsets(MakeArrayView(Data, int32(Entry.Size)), MapEnd + 48, Blocks, ScenarioEnd))
                                {
                                    for (const FKyaConditionedOffsets& Block : Blocks)
                                    {
                                        FKyaConditionedOperation& Result = Environment.InitialScenarioOperations.AddDefaulted_GetRef();
                                        for (int64 Offset : Block.Operations)
                                        {
                                            FKyaScenarioOperation& Op = Result.Operations.AddDefaulted_GetRef();
                                            Op.Variable = int32(ReadU32(Data + Offset));
                                            Op.Type = int32(ReadU32(Data + Offset + 4));
                                            Op.Value = int32(ReadU32(Data + Offset + 8));
                                        }
                                        for (int64 Offset : Block.Conditions)
                                        {
                                            FKyaScenarioCondition& Condition = Result.Conditions.AddDefaulted_GetRef();
                                            Condition.Combiner = int32(ReadU32(Data + Offset));
                                            Condition.Variable = int32(ReadU32(Data + Offset + 4));
                                            Condition.Comparison = int32(ReadU32(Data + Offset + 8));
                                            Condition.Value = int32(ReadU32(Data + Offset + 12));
                                        }
                                    }
                                    TArray<FKyaObjectiveOffsets> Objectives;
                                    int64 ObjectiveEnd = 0, Trailing = 0;
                                    if (KyaReadObjectiveOffsets(MakeArrayView(Data, int32(Entry.Size)), ScenarioEnd, Objectives, ObjectiveEnd, &Trailing))
                                    {
                                        for (const FKyaObjectiveOffsets& Offsets : Objectives)
                                        {
                                            FKyaObjective& Objective = Environment.Objectives.AddDefaulted_GetRef();
                                            Objective.ScenarioVariable = int32(ReadU32(Data + Offsets.Header));
                                            for (int32 Field = 0; Field < 6; ++Field)
                                                Objective.Fields.Add(ReadU32(Data + Offsets.Header + 4 + Field * 4));
                                            for (int64 TargetOffset : Offsets.Targets)
                                            {
                                                FKyaObjectiveTarget& Target = Objective.Targets.AddDefaulted_GetRef();
                                                Target.MessageKeyA = ReadU32(Data + TargetOffset);
                                                Target.MessageKeyB = ReadU32(Data + TargetOffset + 4);
                                                Target.ActorRef = int32(ReadU32(Data + TargetOffset + 8));
                                                Target.ZoneRef = int32(ReadU32(Data + TargetOffset + 12));
                                            }
                                        }
                                        for (uint32 Index = 0; Index < ReadU32(Data + Trailing); ++Index)
                                        {
                                            FKyaObjectiveZoneEntry& Zone = Environment.ObjectiveZones.AddDefaulted_GetRef();
                                            Zone.Field = int32(ReadU32(Data + Trailing + 4 + Index * 8));
                                            Zone.ZoneRef = int32(ReadU32(Data + Trailing + 8 + Index * 8));
                                        }
                                        if (ObjectiveEnd + 12 <= Entry.Size)
                                        {
                                            Environment.Field10C = int32(ReadU32(Data + ObjectiveEnd));
                                            Environment.Field110 = int32(ReadU32(Data + ObjectiveEnd + 4));
                                            const uint32 ElevatorCount = ReadU32(Data + ObjectiveEnd + 8);
                                            if (ElevatorCount <= uint64(Entry.Size - ObjectiveEnd - 12) / 4)
                                                for (uint32 Index = 0; Index < ElevatorCount; ++Index)
                                                    Environment.ElevatorCutscenes.Add(int32(ReadU32(Data + ObjectiveEnd + 12 + Index * 4)));
                                        }
                                        int64 WaypointRecords = 0;
                                        int32 WaypointCount = 0;
                                        if (KyaFindWaypoints(MakeArrayView(Data, int32(Entry.Size)), ObjectiveEnd, WaypointRecords, WaypointCount))
                                        {
                                            for (int32 Index = 0; Index < WaypointCount; ++Index)
                                            {
                                                const int64 Offset = WaypointRecords + Index * 36;
                                                FKyaWaypoint& Waypoint = Environment.Waypoints.AddDefaulted_GetRef();
                                                Waypoint.Position = FVector(ReadFloat(Offset), ReadFloat(Offset + 4), ReadFloat(Offset + 8));
                                                Waypoint.Rotation = FVector(ReadFloat(Offset + 12), ReadFloat(Offset + 16), ReadFloat(Offset + 20));
                                                Waypoint.Parameters = FVector(ReadFloat(Offset + 24), ReadFloat(Offset + 28), ReadFloat(Offset + 32));
                                            }
                                            TArray<FKyaPathOffsets> PathOffsets;
                                            if (KyaReadPathOffsets(MakeArrayView(Data, int32(Entry.Size)), WaypointRecords, WaypointCount, 0, PathOffsets))
                                                for (const FKyaPathOffsets& Offsets : PathOffsets)
                                                {
                                                    FKyaPath& Path = Environment.Paths.AddDefaulted_GetRef();
                                                    Path.DataFlags = Offsets.Flags;
                                                    for (int32 Array = 0; Array < 7; ++Array)
                                                        if (Offsets.Arrays[Array] != INDEX_NONE)
                                                            for (uint32 Point = 0; Point < Offsets.Count; ++Point)
                                                            {
                                                                const int64 Offset = Offsets.Arrays[Array] + Point * (Array < 3 ? 16 : 4);
                                                                if (Array < 3)
                                                                {
                                                                    const FVector4 Value(ReadFloat(Offset), ReadFloat(Offset + 4), ReadFloat(Offset + 8), ReadFloat(Offset + 12));
                                                                    if (Array == 0) Path.Points.Add(Value);
                                                                    else if (Array == 1) Path.Rotations.Add(Value);
                                                                    else Path.ExtraVectors.Add(Value);
                                                                }
                                                                else if (Array == 3) Path.Delays.Add(ReadFloat(Offset));
                                                                else if (Array == 4) Path.PointFlags.Add(ReadU32(Data + Offset));
                                                                else if (Array == 5) Path.ExtraValues.Add(ReadFloat(Offset));
                                                                else Path.ExtraWords.Add(ReadU32(Data + Offset));
                                                            }
                                                    Path.Mode = ReadU32(Data + Offsets.Trailer);
                                                    Path.Type = ReadU32(Data + Offsets.Trailer + 4);
                                                    Path.PathType = ReadU32(Data + Offsets.Trailer + 8);
                                                }
                                            TArray<FKyaActorOffsets> ActorOffsets;
                                            if (KyaReadActorOffsets(MakeArrayView(Data, int32(Entry.Size)), WaypointRecords, 0, ActorOffsets))
                                                for (const FKyaActorOffsets& Offsets : ActorOffsets)
                                                {
                                                    FKyaActorPlacement& Actor = Environment.Actors.AddDefaulted_GetRef();
                                                    Actor.Index = int32(ReadU32(Data + Offsets.Chunk + 8));
                                                    Actor.ClassId = ReadU32(Data + Offsets.Chunk + 12);
                                                    Actor.Name = ANSI_TO_TCHAR(reinterpret_cast<const char*>(Data + Offsets.Name));
                                                    Actor.MeshIndex = int32(ReadU32(Data + Offsets.Placement));
                                                    Actor.TextureIndex = int32(ReadU32(Data + Offsets.Placement + 4));
                                                    Actor.CollisionIndex = int32(ReadU32(Data + Offsets.Placement + 8));
                                                    Actor.Position = FVector(ReadFloat(Offsets.Placement + 12), ReadFloat(Offsets.Placement + 16), ReadFloat(Offsets.Placement + 20));
                                                    Actor.Rotation = FVector(ReadFloat(Offsets.Placement + 24), ReadFloat(Offsets.Placement + 28), ReadFloat(Offsets.Placement + 32));
                                                    Actor.Scale = FVector(ReadFloat(Offsets.Placement + 36), ReadFloat(Offsets.Placement + 40), ReadFloat(Offsets.Placement + 44));
                                                    const int64 Common = Offsets.Common;
                                                    Actor.DefaultBehaviourId = int32(ReadU32(Data + Common));
                                                    Actor.BoundingSphere = FVector4(ReadFloat(Common + 4), ReadFloat(Common + 8), ReadFloat(Common + 12), ReadFloat(Common + 16));
                                                    Actor.Clipping = ReadFloat(Common + 20);
                                                    Actor.VisibilityDistance = ReadFloat(Common + 24);
                                                    Actor.CullingDistance = ReadFloat(Common + 28);
                                                    Actor.Field20 = ReadFloat(Common + 32);
                                                    Actor.SectorId = int32(ReadU32(Data + Common + 36));
                                                    Actor.HashCode = ReadU32(Data + Common + 40);
                                                    Actor.LodBiases = FVector2D(ReadFloat(Common + 44), ReadFloat(Common + 48));
                                                    Actor.ActorFlags = int32(ReadU32(Data + Common + 56));
                                                    Actor.AnimationLayers = int32(ReadU32(Data + Common + 60));
                                                    Actor.Field40 = int32(ReadU32(Data + Common + 64));
                                                    Actor.LightingFlags = ReadU32(Data + Common + 72);
                                                    Actor.LightingValue = ReadFloat(Common + 76);
                                                    for (int32 Index = 0; Index < Offsets.ConditionCount; ++Index)
                                                    {
                                                        const int64 Offset = Offsets.Conditions + int64(Index) * 16;
                                                        FKyaScenarioCondition& Condition = Actor.Conditions.AddDefaulted_GetRef();
                                                        Condition.Combiner = int32(ReadU32(Data + Offset));
                                                        Condition.Variable = int32(ReadU32(Data + Offset + 4));
                                                        Condition.Comparison = int32(ReadU32(Data + Offset + 8));
                                                        Condition.Value = int32(ReadU32(Data + Offset + 12));
                                                    }
                                                    FKyaPlatformPrefixOffsets Platform;
                                                    Actor.bHasPlatformPrefix = Actor.ClassId == 0x0d && KyaReadPlatformPrefix(
                                                        MakeArrayView(Data, int32(Entry.Size)), Offsets.ClassData, Offsets.ClassDataSize, Platform);
                                                    if (Actor.bHasPlatformPrefix)
                                                    {
                                                        Actor.PlatformWeight = ReadFloat(Platform.Weight);
                                                        for (int32 Index = 0; Index < Platform.ActorCount; ++Index)
                                                        {
                                                            FKyaPlatformActor& Ref = Actor.PlatformActors.AddDefaulted_GetRef();
                                                            Ref.ActorRef = int32(ReadU32(Data + Platform.Actors + Index * 8));
                                                            Ref.CarryMethod = int32(ReadU32(Data + Platform.Actors + Index * 8 + 4));
                                                        }
                                                        for (int32 Index = 0; Index < Platform.ZoneCount; ++Index)
                                                            Actor.PlatformZones.Add(int32(ReadU32(Data + Platform.Zones + Index * 4)));
                                                        for (int64 Offset : Platform.NoFrictionZones)
                                                        {
                                                            FKyaPlatformNoFrictionZone& Zone = Actor.PlatformNoFrictionZones.AddDefaulted_GetRef();
                                                            Zone.ZoneRef = int32(ReadU32(Data + Offset));
                                                            for (uint32 Index = 0; Index < ReadU32(Data + Offset + 4); ++Index)
                                                                Zone.Actors.Add(int32(ReadU32(Data + Offset + 8 + Index * 4)));
                                                        }
                                                        FKyaPlatformDetailsOffsets Details;
                                                        const int64 Remaining = Offsets.ClassData + Offsets.ClassDataSize - Platform.Suffix;
                                                        Actor.bHasPlatformDetails = KyaReadPlatformDetails(MakeArrayView(Data, int32(Entry.Size)), Platform.Suffix, Remaining, Details);
                                                        if (Actor.bHasPlatformDetails)
                                                        {
                                                            for (int32 Index = 0; Index < Details.CameraCount; ++Index)
                                                            {
                                                                FKyaPlatformCamera& Camera = Actor.PlatformCameras.AddDefaulted_GetRef();
                                                                if (!KyaReadPlatformCamera(MakeArrayView(Data, int32(Entry.Size)),
                                                                    Details.Cameras + int64(Index) * 72, Camera)) return nullptr;
                                                            }
                                                            FKyaPlatformProperties& P = Actor.PlatformProperties;
                                                            const int64 Offset = Details.Properties;
                                                            P.SoundRefA = int32(ReadU32(Data + Offset)); P.SampleRefA = int32(ReadU32(Data + Offset + 4));
                                                            P.SoundRefB = int32(ReadU32(Data + Offset + 8)); P.SampleRefB = int32(ReadU32(Data + Offset + 12));
                                                            P.Offset = FVector(ReadFloat(Offset + 16), ReadFloat(Offset + 20), ReadFloat(Offset + 24));
                                                            P.EffectRef1C = ReadU32(Data + Offset + 28); P.EffectRef20 = ReadU32(Data + Offset + 32);
                                                            P.PlatformFlags = ReadU32(Data + Offset + 36); P.CameraIndex = int32(ReadU32(Data + Offset + 40));
                                                            P.TiltValue = ReadFloat(Offset + 44); P.TiltMode = int32(ReadU32(Data + Offset + 48));
                                                            P.TiltAndPush = FVector4(ReadFloat(Offset + 52), ReadFloat(Offset + 56), ReadFloat(Offset + 60), ReadFloat(Offset + 64));
                                                            P.PushOscillation = FVector2D(ReadFloat(Offset + 68), ReadFloat(Offset + 72));
                                                            TArray<FKyaPlatformSound>* SoundLists[] = { &Actor.PlatformSoundsA, &Actor.PlatformSoundsB };
                                                            for (int32 List = 0; List < 2; ++List)
                                                                for (int32 Item = 0; Item < Details.SoundCounts[List]; ++Item)
                                                                {
                                                                    const int64 SoundOffset = Details.Sounds[List] + int64(Item) * 16;
                                                                    FKyaPlatformSound& Sound = SoundLists[List]->AddDefaulted_GetRef();
                                                                    Sound.SegmentStart = int32(ReadU32(Data + SoundOffset)); Sound.SegmentEnd = int32(ReadU32(Data + SoundOffset + 4));
                                                                    Sound.SoundRef = int32(ReadU32(Data + SoundOffset + 8)); Sound.SampleRef = int32(ReadU32(Data + SoundOffset + 12));
                                                                }
                                                            Actor.ClassData.Append(Data + Details.Tail, int32(Offsets.ClassData + Offsets.ClassDataSize - Details.Tail));
                                                        }
                                                        else Actor.ClassData.Append(Data + Platform.Suffix, int32(Remaining));
                                                    }
                                                    else Actor.ClassData.Append(Data + Offsets.ClassData, Offsets.ClassDataSize);
                                                    if (Actor.ClassId == 0x39)
                                                    {
                                                        FKyaBasicBoxOffsets Box;
                                                        Actor.bHasBasicBoxData = KyaReadBasicBoxOffsets(
                                                            MakeArrayView(Data, int32(Entry.Size)), Offsets.ClassData, Offsets.ClassDataSize, Box);
                                                        if (Actor.bHasBasicBoxData)
                                                        {
                                                            Actor.BasicBoxField160 = int32(ReadU32(Data + Box.Field160));
                                                            Actor.BasicBoxLifeMax = float(ReadU32(Data + Box.LifeMax));
                                                            for (int32 Index = 0; Index < Box.ActorCount; ++Index)
                                                                Actor.BasicBoxActorRefs.Add(int32(ReadU32(Data + Box.Actors + Index * 4)));
                                                            Actor.BasicBoxVibration0 = ReadFloat(Box.Vibration);
                                                            Actor.BasicBoxVibration1 = ReadFloat(Box.Vibration + 4);
                                                            Actor.BasicBoxVibration2 = ReadFloat(Box.Vibration + 8);
                                                            Actor.BasicBoxVibration3 = ReadFloat(Box.Vibration + 12);
                                                            Actor.BasicBoxField180 = int32(ReadU32(Data + Box.Field180));
                                                            Actor.BasicBoxField18C = int32(ReadU32(Data + Box.Fields));
                                                            Actor.BasicBoxField190 = int32(ReadU32(Data + Box.Fields + 4));
                                                            Actor.BasicBoxField198 = int32(ReadU32(Data + Box.Fields + 8));
                                                            Actor.BasicBoxField194 = int32(ReadU32(Data + Box.Fields + 12));
                                                            for (int32 Index = 0; Index < Box.SwitchTargetCount; ++Index)
                                                            {
                                                                FKyaSwitchTarget& Target = Actor.BasicBoxSwitch.Targets.AddDefaulted_GetRef();
                                                                const int64 T = Box.SwitchTargets + int64(Index) * 28;
                                                                Target.TargetRef = int32(ReadU32(Data + T));
                                                                Target.CutsceneId = int32(ReadU32(Data + T + 4));
                                                                Target.Flags = int32(ReadU32(Data + T + 8));
                                                                Target.MessageId = int32(ReadU32(Data + T + 12));
                                                                Target.MessageFlags = int32(ReadU32(Data + T + 16));
                                                                Target.Field14 = int32(ReadU32(Data + T + 20));
                                                                Target.Field18 = int32(ReadU32(Data + T + 24));
                                                            }
                                                            FKyaStreamEventCamera& EventCamera = Actor.BasicBoxSwitch.Camera;
                                                            const int64 C = Box.SwitchCamera;
                                                            EventCamera.CameraIndex = int32(ReadU32(Data + C));
                                                            EventCamera.Flags = int32(ReadU32(Data + C + 4));
                                                            EventCamera.Field8 = ReadFloat(C + 8);
                                                            EventCamera.FieldC = ReadFloat(C + 12);
                                                            EventCamera.Field10 = ReadFloat(C + 16);
                                                            EventCamera.ActorRef = int32(ReadU32(Data + C + 20));
                                                            EventCamera.ZoneRef = int32(ReadU32(Data + C + 24));
                                                            EventCamera.Field1C = ReadFloat(C + 28);
                                                            FKyaAddOnGenerator& AddOn = Actor.BasicBoxAddOn;
                                                            const int64 A = Box.AddOn;
                                                            AddOn.Field0 = int32(ReadU32(Data + A));
                                                            AddOn.MoneyType = int32(ReadU32(Data + A + 4));
                                                            AddOn.MoneyCount = int32(ReadU32(Data + A + 8));
                                                            AddOn.BonusCount = int32(ReadU32(Data + A + 12));
                                                            AddOn.FieldC = int32(ReadU32(Data + A + 16));
                                                            AddOn.SubObjMode = Box.SubObjModeValue;
                                                            if (Box.SubObjFloats)
                                                            {
                                                                AddOn.SubObjField0 = ReadFloat(Box.SubObjFloats);
                                                                AddOn.SubObjField4 = ReadFloat(Box.SubObjFloats + 4);
                                                                AddOn.SubObjFieldC = ReadFloat(Box.SubObjFloats + 8);
                                                                AddOn.SubObjField10 = ReadFloat(Box.SubObjFloats + 12);
                                                                AddOn.SubObjField14 = ReadFloat(Box.SubObjFloats + 16);
                                                            }
                                                            Actor.ClassData.Reset();
                                                            Actor.ClassData.Append(Data + Box.Tail, int32(Offsets.ClassData + Offsets.ClassDataSize - Box.Tail));
                                                        }
                                                    }
                                                    if (Actor.ClassId == 0x2e)
                                                    {
                                                        FKyaFxOffsets Fx;
                                                        Actor.bHasFxData = KyaReadFxOffsets(
                                                            MakeArrayView(Data, int32(Entry.Size)), Offsets.ClassData, Offsets.ClassDataSize, Fx);
                                                        if (Actor.bHasFxData)
                                                        {
                                                            Actor.FxField19C = ReadFloat(Fx.Field19C);
                                                            Actor.FxField1A0 = ReadFloat(Fx.Field1A0);
                                                            Actor.ClassData.Reset();
                                                            Actor.ClassData.Append(Data + Fx.Tail, int32(Offsets.ClassData + Offsets.ClassDataSize - Fx.Tail));
                                                        }
                                                    }
                                                    if (Actor.ClassId == 0x21)
                                                    {
                                                        FKyaAraignosOffsets Araignos;
                                                        Actor.bHasAraignosData = KyaReadAraignosOffsets(
                                                            MakeArrayView(Data, int32(Entry.Size)), Offsets.ClassData, Offsets.ClassDataSize, Araignos);
                                                        if (Actor.bHasAraignosData)
                                                        {
                                                            Actor.AraignosWeightB = ReadFloat(Araignos.WeightB);
                                                            Actor.AraignosPathFollowIndex = int32(ReadU32(Data + Araignos.PathFollow));
                                                            Actor.AraignosField1F8 = ReadFloat(Araignos.Floats);
                                                            Actor.AraignosField1FC = ReadFloat(Araignos.Floats + 4);
                                                            Actor.AraignosDamage = ReadFloat(Araignos.Floats + 8);
                                                            Actor.AraignosSoundRef240 = int32(ReadU32(Data + Araignos.Sounds));
                                                            Actor.AraignosSoundRef238 = int32(ReadU32(Data + Araignos.Sounds + 4));
                                                            Actor.AraignosSoundRef23C = int32(ReadU32(Data + Araignos.Sounds + 8));
                                                            FKyaAddOnGenerator& AddOn = Actor.AraignosAddOn;
                                                            const int64 A = Araignos.AddOn;
                                                            AddOn.Field0 = int32(ReadU32(Data + A));
                                                            AddOn.MoneyType = int32(ReadU32(Data + A + 4));
                                                            AddOn.MoneyCount = int32(ReadU32(Data + A + 8));
                                                            AddOn.BonusCount = int32(ReadU32(Data + A + 12));
                                                            AddOn.FieldC = int32(ReadU32(Data + A + 16));
                                                            AddOn.SubObjMode = Araignos.SubObjModeValue;
                                                            if (Araignos.SubObjFloats)
                                                            {
                                                                AddOn.SubObjField0 = ReadFloat(Araignos.SubObjFloats);
                                                                AddOn.SubObjField4 = ReadFloat(Araignos.SubObjFloats + 4);
                                                                AddOn.SubObjFieldC = ReadFloat(Araignos.SubObjFloats + 8);
                                                                AddOn.SubObjField10 = ReadFloat(Araignos.SubObjFloats + 12);
                                                                AddOn.SubObjField14 = ReadFloat(Araignos.SubObjFloats + 16);
                                                            }
                                                            Actor.ClassData.Reset();
                                                            Actor.ClassData.Append(Data + Araignos.Tail, int32(Offsets.ClassData + Offsets.ClassDataSize - Araignos.Tail));
                                                        }
                                                    }
                                                    if (Actor.ClassId == 0x1c)
                                                    {
                                                        FKyaProjectileOffsets Projectile;
                                                        Actor.bHasProjectileData = KyaReadProjectileOffsets(
                                                            MakeArrayView(Data, int32(Entry.Size)), Offsets.ClassData, Offsets.ClassDataSize, Projectile);
                                                        if (Actor.bHasProjectileData)
                                                        {
                                                            Actor.ProjectileWeightB = ReadFloat(Projectile.WeightB);
                                                            Actor.ProjectileLifeMax = float(ReadU32(Data + Projectile.Autonomous));
                                                            Actor.ProjectileGravityScale = ReadFloat(Projectile.Autonomous + 4);
                                                            Actor.ProjectileDynamicExtField4 = ReadFloat(Projectile.Autonomous + 8);
                                                            Actor.ProjectileDynamicExtField8 = ReadFloat(Projectile.Autonomous + 12);
                                                            FKyaProjectileSubObj& Sub = Actor.ProjectileSubObj;
                                                            const int64 S = Projectile.SubObj;
                                                            Sub.TimeToExplode = ReadFloat(S);
                                                            Sub.ExplosionRange = ReadFloat(S + 4);
                                                            Sub.AnimDuration = ReadFloat(S + 8);
                                                            Sub.Flags = int32(ReadU32(Data + S + 12));
                                                            Sub.Damage = ReadFloat(S + 16);
                                                            Sub.HitRadius = ReadFloat(S + 20);
                                                            Sub.WarnRadius = ReadFloat(S + 24);
                                                            Sub.Field1C = ReadFloat(S + 28);
                                                            Sub.Field20 = ReadFloat(S + 32);
                                                            Sub.Field24 = ReadFloat(S + 36);
                                                            Sub.Field28 = int32(ReadU32(Data + S + 40));
                                                            Sub.MaterialId = int32(ReadU32(Data + S + 44));
                                                            Sub.Field30 = int32(ReadU32(Data + S + 48));
                                                            Sub.Field34 = int32(ReadU32(Data + S + 52));
                                                            Sub.Field38 = int32(ReadU32(Data + S + 56));
                                                            Sub.Field3C = int32(ReadU32(Data + S + 60));
                                                            Sub.Field40 = ReadFloat(S + 64);
                                                            Sub.Field44 = ReadFloat(S + 68);
                                                            FKyaPatternPart& Pattern = Actor.ProjectilePattern;
                                                            const int64 P = Projectile.Pattern;
                                                            Pattern.Field4 = ReadFloat(P);
                                                            Pattern.Field0 = ReadFloat(P + 4);
                                                            Pattern.FieldC = ReadFloat(P + 8);
                                                            Pattern.Field8 = ReadFloat(P + 12);
                                                            Pattern.Field10 = ReadFloat(P + 16);
                                                            Pattern.Field14 = ReadFloat(P + 20);
                                                            Pattern.MaterialId = int32(ReadU32(Data + P + 24));
                                                            Pattern.PointCount = ReadFloat(P + 28);
                                                            Pattern.Field20 = ReadFloat(P + 32);
                                                            Pattern.bHasField24 = Projectile.bHasPatternField24;
                                                            if (Pattern.bHasField24) Pattern.Field24 = ReadFloat(Projectile.PatternField24);
                                                            Pattern.Field28 = int32(ReadU32(Data + P + (Pattern.bHasField24 ? 40 : 36)));
                                                            Actor.ClassData.Reset();
                                                            Actor.ClassData.Append(Data + Projectile.Tail, int32(Offsets.ClassData + Offsets.ClassDataSize - Projectile.Tail));
                                                        }
                                                    }
                                                    if (Actor.ClassId == 0x12)
                                                    {
                                                        FKyaWindOffsets Wind;
                                                        Actor.bHasWindData = KyaReadWindOffsets(
                                                            MakeArrayView(Data, int32(Entry.Size)), Offsets.ClassData, Offsets.ClassDataSize, Wind);
                                                        if (Actor.bHasWindData)
                                                        {
                                                            Actor.WindField160 = int32(ReadU32(Data + Wind.Field160));
                                                            Actor.WindMaxWind = ReadFloat(Wind.MaxWind);
                                                            Actor.WindField168 = ReadFloat(Wind.Field168);
                                                            Actor.WindField16C = ReadFloat(Wind.Field16C);
                                                            Actor.WindField174 = ReadFloat(Wind.Field174);
                                                            Actor.WindField180 = ReadFloat(Wind.Field180);
                                                            Actor.WindField184 = ReadFloat(Wind.Field184);
                                                            Actor.WindField188 = ReadFloat(Wind.Field188);
                                                            Actor.WindField18C = ReadFloat(Wind.Field18C);
                                                            Actor.WindField178 = ReadFloat(Wind.Field178);
                                                            Actor.WindField17C = ReadFloat(Wind.Field17C);
                                                            Actor.WindField190 = int32(ReadU32(Data + Wind.Field190));
                                                            Actor.WindSoundRef = int32(ReadU32(Data + Wind.Sound));
                                                            Actor.WindActivationZoneIndex = int32(ReadU32(Data + Wind.ActivationZone));
                                                            for (int32 Index = 0; Index < Wind.ActorCount; ++Index)
                                                                Actor.WindActorRefs.Add(int32(ReadU32(Data + Wind.Actors + Index * 4)));
                                                            Actor.WindMaterialId = int32(ReadU32(Data + Wind.Material));
                                                            Actor.WindZoneRef1C4 = int32(ReadU32(Data + Wind.Zone1C4));
                                                            Actor.WindColor1C8 = int32(ReadU32(Data + Wind.Color1C8));
                                                            Actor.WindWaypointRef1CC = int32(ReadU32(Data + Wind.Waypoint1CC));
                                                            Actor.WindSectorHierarchyHash = int64(uint64(ReadU32(Data + Wind.SectorHash)) | (uint64(ReadU32(Data + Wind.SectorHash + 4)) << 32));
                                                            if (Wind.bHasSwitches)
                                                            {
                                                                KyaReadSwitch(MakeArrayView(Data, int32(Entry.Size)), Wind.Switches[0].Targets, Wind.Switches[0].Count, Wind.Switches[0].Camera, Actor.WindSwitch1D0);
                                                                KyaReadSwitch(MakeArrayView(Data, int32(Entry.Size)), Wind.Switches[1].Targets, Wind.Switches[1].Count, Wind.Switches[1].Camera, Actor.WindSwitch1D8);
                                                                KyaReadSwitch(MakeArrayView(Data, int32(Entry.Size)), Wind.Switches[2].Targets, Wind.Switches[2].Count, Wind.Switches[2].Camera, Actor.WindSwitch1E0);
                                                            }
                                                            Actor.WindField194 = ReadFloat(Wind.Floats194);
                                                            Actor.WindField198 = ReadFloat(Wind.Floats194 + 4);
                                                            Actor.WindField19C = ReadFloat(Wind.Floats194 + 8);
                                                            Actor.WindField1A0 = ReadFloat(Wind.Floats194 + 12);
                                                            Actor.WindField1A4 = ReadFloat(Wind.Floats194 + 16);
                                                            Actor.WindWaypointRef = int32(ReadU32(Data + Wind.Waypoint));
                                                            Actor.WindField1AC = ReadFloat(Wind.Waypoint + 4);
                                                            Actor.WindField1B0 = ReadFloat(Wind.Waypoint + 8);
                                                            Actor.ClassData.Reset();
                                                            Actor.ClassData.Append(Data + Wind.Tail, int32(Offsets.ClassData + Offsets.ClassDataSize - Wind.Tail));
                                                        }
                                                    }
                                                    if (Actor.ClassId == 0x2c)
                                                    {
                                                        FKyaTrapOffsets Trap;
                                                        Actor.bHasTrapData = KyaReadTrapOffsets(
                                                            MakeArrayView(Data, int32(Entry.Size)), Offsets.ClassData, Offsets.ClassDataSize, Trap);
                                                        if (Actor.bHasTrapData)
                                                        {
                                                            Actor.TrapWeightB = ReadFloat(Trap.WeightB);
                                                            for (int32 Index = 0; Index < Trap.ActorCount; ++Index)
                                                                Actor.TrapActorRefs.Add(int32(ReadU32(Data + Trap.Actors + Index * 4)));
                                                            Actor.TrapField14 = int32(ReadU32(Data + Trap.Field14));
                                                            Actor.TrapField18 = int32(ReadU32(Data + Trap.Field18));
                                                            Actor.TrapField1C = int32(ReadU32(Data + Trap.Field1C));
                                                            Actor.TrapField20 = int32(ReadU32(Data + Trap.Field20));
                                                            Actor.bHasTrapField24 = Trap.bHasField24;
                                                            if (Trap.bHasField24) Actor.TrapField24 = int32(ReadU32(Data + Trap.Field24));
                                                            Actor.TrapActorRef = int32(ReadU32(Data + Trap.ActorRef));
                                                            Actor.TrapSpeed = ReadFloat(Trap.Speed);
                                                            Actor.TrapAcceleration = ReadFloat(Trap.Acceleration);
                                                            Actor.TrapPathFollowIndex = int32(ReadU32(Data + Trap.PathFollow));
                                                            Actor.TrapField4C = ReadFloat(Trap.Field4C);
                                                            Actor.TrapRequiredEscapeAttempts = int32(ReadU32(Data + Trap.RequiredEscapes));
                                                            for (int32 Index = 0; Index < Trap.TrapCount; ++Index)
                                                            {
                                                                FKyaTrapStreamEntry& TrapEntry = Actor.TrapStreamEntries.AddDefaulted_GetRef();
                                                                const int64 Offset = Trap.TrapEntries + int64(Index) * 60;
                                                                TrapEntry.Target.TargetRef = int32(ReadU32(Data + Offset));
                                                                TrapEntry.Target.CutsceneId = int32(ReadU32(Data + Offset + 4));
                                                                TrapEntry.Target.Flags = int32(ReadU32(Data + Offset + 8));
                                                                TrapEntry.Target.MessageId = int32(ReadU32(Data + Offset + 12));
                                                                TrapEntry.Target.MessageFlags = int32(ReadU32(Data + Offset + 16));
                                                                TrapEntry.Target.Field14 = int32(ReadU32(Data + Offset + 20));
                                                                TrapEntry.Target.Field18 = int32(ReadU32(Data + Offset + 24));
                                                                TrapEntry.Camera.CameraIndex = int32(ReadU32(Data + Offset + 28));
                                                                TrapEntry.Camera.Flags = int32(ReadU32(Data + Offset + 32));
                                                                TrapEntry.Camera.Field8 = ReadFloat(Offset + 36);
                                                                TrapEntry.Camera.FieldC = ReadFloat(Offset + 40);
                                                                TrapEntry.Camera.Field10 = ReadFloat(Offset + 44);
                                                                TrapEntry.Camera.ActorRef = int32(ReadU32(Data + Offset + 48));
                                                                TrapEntry.Camera.ZoneRef = int32(ReadU32(Data + Offset + 52));
                                                                TrapEntry.Camera.Field1C = ReadFloat(Offset + 56);
                                                            }
                                                            Actor.ClassData.Reset();
                                                            Actor.ClassData.Append(Data + Trap.Tail, int32(Offsets.ClassData + Offsets.ClassDataSize - Trap.Tail));
                                                        }
                                                    }
                                                    if (Actor.ClassId == 0x20)
                                                    {
                                                        FKyaWeaponOffsets Weapon;
                                                        Actor.bHasWeaponData = KyaReadWeaponOffsets(
                                                            MakeArrayView(Data, int32(Entry.Size)), Offsets.ClassData, Offsets.ClassDataSize, Weapon);
                                                        if (Actor.bHasWeaponData)
                                                        {
                                                            Actor.WeaponWeightB = ReadFloat(Weapon.WeightB);
                                                            Actor.WeaponField1D8 = ReadFloat(Weapon.Field1D8);
                                                            Actor.WeaponField1DC = ReadFloat(Weapon.Field1DC);
                                                            Actor.ClassData.Reset();
                                                            Actor.ClassData.Append(Data + Weapon.Tail, int32(Offsets.ClassData + Offsets.ClassDataSize - Weapon.Tail));
                                                        }
                                                    }
                                                    if (Actor.ClassId == 0x11)
                                                    {
                                                        FKyaSwitchOffsets Switch;
                                                        Actor.bHasSwitchData = KyaReadSwitchOffsets(
                                                            MakeArrayView(Data, int32(Entry.Size)), Offsets.ClassData, Offsets.ClassDataSize, Switch);
                                                        if (Actor.bHasSwitchData)
                                                        {
                                                            KyaReadSwitch(MakeArrayView(Data, int32(Entry.Size)), Switch.Targets, Switch.Count, Switch.Camera, Actor.SwitchTarget);
                                                            Actor.ClassData.Reset();
                                                            Actor.ClassData.Append(Data + Switch.Tail, int32(Offsets.ClassData + Offsets.ClassDataSize - Switch.Tail));
                                                        }
                                                    }
                                                    if (Actor.ClassId == 0x0e)
                                                    {
                                                        FKyaBonusOffsets Bonus;
                                                        Actor.bHasBonusData = KyaReadBonusOffsets(
                                                            MakeArrayView(Data, int32(Entry.Size)), Offsets.ClassData, Offsets.ClassDataSize, Bonus);
                                                        if (Actor.bHasBonusData)
                                                        {
                                                            Actor.BonusWeightB = ReadFloat(Bonus.WeightB);
                                                            Actor.BonusSoundRef = int32(ReadU32(Data + Bonus.Sound));
                                                            Actor.BonusFlareMaterialId = int32(ReadU32(Data + Bonus.FlareMaterial));
                                                            Actor.BonusAnimMaterialId = int32(ReadU32(Data + Bonus.AnimMaterial));
                                                            Actor.ClassData.Reset();
                                                            Actor.ClassData.Append(Data + Bonus.Tail, int32(Offsets.ClassData + Offsets.ClassDataSize - Bonus.Tail));
                                                        }
                                                    }
                                                    if (Actor.ClassId == 0x07)
                                                    {
                                                        FKyaMickenOffsets Micken;
                                                        Actor.bHasMickenData = KyaReadMickenOffsets(
                                                            MakeArrayView(Data, int32(Entry.Size)), Offsets.ClassData, Offsets.ClassDataSize, Micken);
                                                        if (Actor.bHasMickenData)
                                                        {
                                                            Actor.MickenWeightB = ReadFloat(Micken.WeightB);
                                                            Actor.MickenLifeMax = float(ReadU32(Data + Micken.Autonomous));
                                                            Actor.MickenGravityScale = ReadFloat(Micken.Autonomous + 4);
                                                            Actor.MickenDynamicExtField4 = ReadFloat(Micken.Autonomous + 8);
                                                            Actor.MickenDynamicExtField8 = ReadFloat(Micken.Autonomous + 12);
                                                            Actor.MickenWaypointIndex = int32(ReadU32(Data + Micken.Waypoint));
                                                            Actor.MickenActorIndex = int32(ReadU32(Data + Micken.Actor));
                                                            Actor.MickenField358 = int32(ReadU32(Data + Micken.Field358));
                                                            Actor.MickenField35C = ReadFloat(Micken.Field35C);
                                                            Actor.MickenField360 = int32(ReadU32(Data + Micken.Field360));
                                                            Actor.ClassData.Reset();
                                                            Actor.ClassData.Append(Data + Micken.Tail, int32(Offsets.ClassData + Offsets.ClassDataSize - Micken.Tail));
                                                        }
                                                    }
                                                    if (Actor.ClassId == 0x09)
                                                    {
                                                        FKyaWolfenOffsets Wolfen;
                                                        Actor.bHasWolfenData = KyaReadWolfenOffsets(
                                                            MakeArrayView(Data, int32(Entry.Size)), Offsets.ClassData, Offsets.ClassDataSize, Wolfen);
                                                        if (Actor.bHasWolfenData)
                                                        {
                                                            FKyaFighterPrefixOffsets F;
                                                            F.Weight = Wolfen.FighterPrefix;
                                                            F.Autonomous = Wolfen.FighterPrefix + 4;
                                                            F.Fixed = Wolfen.FighterFixed;
                                                            F.Blows = Wolfen.Blows;
                                                            F.Grabs = Wolfen.Grabs;
                                                            F.GrabReacts = Wolfen.GrabReacts;
                                                            F.Combos = Wolfen.Combos;
                                                            F.WeaponIndex = Wolfen.WeaponIndex;
                                                            F.Tail = Wolfen.WeaponIndex + 4;
                                                            if (!KyaPopulateFighterPrefix(Actor, Data, int32(Entry.Size), F)) return nullptr;
                                                            FKyaWolfenFightParamOffsets FightParam;
                                                            if (!KyaReadWolfenFightParamOffsets(MakeArrayView(Data, int32(Entry.Size)), Wolfen.FightParam, Wolfen.Tail, FightParam) ||
                                                                FightParam.Tail != Wolfen.Tail) return nullptr;
                                                            FKyaWolfenFightParam& Params = Actor.WolfenFightParamData;
                                                            for (int32 Index = 0; Index < 5; ++Index)
                                                            {
                                                                Params.Capability0Fields.Add(int32(ReadU32(Data + FightParam.Capability0Fields + Index * 4)));
                                                                Params.Capability2Fields.Add(int32(ReadU32(Data + FightParam.Capability2Fields + Index * 4)));
                                                            }
                                                            for (int32 Index = 0; Index < 4; ++Index)
                                                                Params.ConfigurationValues.Add(ReadFloat(FightParam.Floats + Index * 4));
                                                            for (int32 Cap = 0; Cap < 3; ++Cap)
                                                            {
                                                                FKyaWolfenCapability& Capability = Params.Capabilities.AddDefaulted_GetRef();
                                                                for (const FKyaWolfenCapabilityItemOffsets& ItemOffsets : FightParam.Capabilities[Cap].Items)
                                                                {
                                                                    FKyaWolfenCapabilityItem& Item = Capability.Items.AddDefaulted_GetRef();
                                                                    for (int32 Index = 0; Index < ItemOffsets.ChainCount; ++Index)
                                                                        Item.Chains.Add(int32(ReadU32(Data + ItemOffsets.Chains + Index * 4)));
                                                                    Item.Field0 = ReadFloat(ItemOffsets.Floats);
                                                                    Item.Field4 = ReadFloat(ItemOffsets.Floats + 4);
                                                                }
                                                            }
                                                            const uint32 Combo1Count = ReadU32(Data + FightParam.ComboList1Count);
                                                            for (uint32 Index = 0; Index < Combo1Count; ++Index)
                                                                Params.ComboList1.Add(int32(ReadU32(Data + FightParam.ComboList1Data + Index * 4)));
                                                            const uint32 Combo2Count = ReadU32(Data + FightParam.ComboList2Count);
                                                            for (uint32 Index = 0; Index < Combo2Count; ++Index)
                                                                Params.ComboList2.Add(int32(ReadU32(Data + FightParam.ComboList2Data + Index * 4)));
                                                            Params.FieldAfterLists = int32(ReadU32(Data + FightParam.Trailing));
                                                            Params.FieldB44 = int32(ReadU32(Data + FightParam.Trailing + 4));
                                                            for (int32 Index = 0; Index < 6; ++Index)
                                                                Params.TuningValues.Add(ReadFloat(FightParam.Trailing + 8 + Index * 4));
                                                            for (int32 Index = 0; Index < 4; ++Index)
                                                                Params.Knowledge.Add(int32(ReadU32(Data + FightParam.Trailing + 32 + Index * 4)));
                                                            Actor.WolfenFieldB74 = int32(ReadU32(Data + Wolfen.FieldB74));
                                                            Actor.WolfenVisionRange = ReadFloat(Wolfen.Vision);
                                                            Actor.WolfenVisionFov = ReadFloat(Wolfen.Vision + 4);
                                                            Actor.WolfenVisionField38 = ReadFloat(Wolfen.Vision + 8);
                                                            Actor.WolfenVisionField3C = ReadFloat(Wolfen.Vision + 12);
                                                            Actor.WolfenVisionFlags = int32(ReadU32(Data + Wolfen.Vision + 16));
                                                            Actor.WolfenStartSectorId = int32(ReadU32(Data + Wolfen.StartSector));
                                                            Actor.WolfenHearingRangeSquared = ReadFloat(Wolfen.Hearing);
                                                            Actor.WolfenHearingMaxHeight = ReadFloat(Wolfen.Hearing + 4);
                                                            Actor.WolfenVisionDetectionRange = ReadFloat(Wolfen.VisionDetection);
                                                            Actor.WolfenFieldCF4 = ReadFloat(Wolfen.FieldCF4);
                                                            Actor.WolfenWalkSpeed = ReadFloat(Wolfen.WalkValues);
                                                            Actor.WolfenWalkAcceleration = ReadFloat(Wolfen.WalkValues + 4);
                                                            Actor.WolfenWalkRotSpeed = ReadFloat(Wolfen.WalkValues + 8);
                                                            Actor.WolfenRunSpeedScale = ReadFloat(Wolfen.WalkValues + 12);
                                                            Actor.WolfenDefaultRunSpeed = ReadFloat(Wolfen.WalkValues + 16);
                                                            Actor.WolfenRunAcceleration = ReadFloat(Wolfen.WalkValues + 20);
                                                            Actor.WolfenRotRunSpeed = ReadFloat(Wolfen.WalkValues + 24);
                                                            Actor.WolfenMagicRequired = int32(ReadU32(Data + Wolfen.MagicRequired));
                                                            Actor.WolfenFieldD30 = int32(ReadU32(Data + Wolfen.FieldD30));
                                                            Actor.WolfenFieldB90 = ReadFloat(Wolfen.FieldB90);
                                                            Actor.WolfenFieldB94 = ReadFloat(Wolfen.FieldB90 + 4);
                                                            Actor.WolfenFieldB98 = ReadFloat(Wolfen.FieldB90 + 8);
                                                            Actor.ClassData.Reset();
                                                        }
                                                    }
                                                    if (Actor.ClassId == 0x16)
                                                    {
                                                        FKyaFruitOffsets Fruit;
                                                        Actor.bHasFruitData = KyaReadFruitOffsets(
                                                            MakeArrayView(Data, int32(Entry.Size)), Offsets.ClassData, Offsets.ClassDataSize, Fruit);
                                                        if (Actor.bHasFruitData)
                                                        {
                                                            Actor.FruitWeightB = ReadFloat(Fruit.WeightB);
                                                            Actor.FruitLifeMax = float(ReadU32(Data + Fruit.Autonomous));
                                                            Actor.FruitGravityScale = ReadFloat(Fruit.Autonomous + 4);
                                                            Actor.FruitDynamicExtField4 = ReadFloat(Fruit.Autonomous + 8);
                                                            Actor.FruitDynamicExtField8 = ReadFloat(Fruit.Autonomous + 12);
                                                            Actor.FruitField360 = int32(ReadU32(Data + Fruit.Field360));
                                                            FKyaSwitchOffsets Switch;
                                                            if (KyaReadSwitchOffsets(MakeArrayView(Data, int32(Entry.Size)), Fruit.Switch, Offsets.ClassDataSize - 24, Switch))
                                                                KyaReadSwitch(MakeArrayView(Data, int32(Entry.Size)), Switch.Targets, Switch.Count, Switch.Camera, Actor.FruitSwitch);
                                                            Actor.ClassData.Reset();
                                                            Actor.ClassData.Append(Data + Fruit.Tail, int32(Offsets.ClassData + Offsets.ClassDataSize - Fruit.Tail));
                                                        }
                                                    }
                                                    if (Actor.ClassId == 0x13)
                                                    {
                                                        FKyaCommanderOffsets Commander;
                                                        Actor.bHasCommanderData = KyaReadCommanderOffsets(
                                                            MakeArrayView(Data, int32(Entry.Size)), Offsets.ClassData, Offsets.ClassDataSize, Commander);
                                                        if (Actor.bHasCommanderData)
                                                        {
                                                            Actor.CommanderDetectAreaZoneId = int32(ReadU32(Data + Commander.Headers));
                                                            Actor.CommanderGuardAreaZoneId = int32(ReadU32(Data + Commander.Headers + 4));
                                                            Actor.CommanderField180 = int32(ReadU32(Data + Commander.Headers + 8));
                                                            Actor.CommanderNewPathFindingId = int32(ReadU32(Data + Commander.Headers + 12));
                                                            Actor.CommanderActorRef = int32(ReadU32(Data + Commander.Headers + 16));
                                                            Actor.CommanderSquadValue = ReadFloat(Commander.Squad);
                                                            Actor.CommanderSquadField4 = int32(ReadU32(Data + Commander.Squad + 4));
                                                            Actor.CommanderSquadField8 = int32(ReadU32(Data + Commander.Squad + 8));
                                                            for (int32 Index = 0; Index < Commander.TeamCount; ++Index)
                                                                Actor.CommanderTeamEnemies.Add(int32(ReadU32(Data + Commander.Teams + Index * 4)));
                                                            for (int32 Index = 0; Index < Commander.ComponentCount; ++Index)
                                                            {
                                                                FKyaCommanderComponent& Component = Actor.CommanderSequence.AddDefaulted_GetRef();
                                                                Component.Field0 = int32(ReadU32(Data + Commander.Components + Index * 8));
                                                                Component.Field8 = int32(ReadU32(Data + Commander.Components + Index * 8 + 4));
                                                            }
                                                            for (int32 Index = 0; Index < 14; ++Index)
                                                                Actor.CommanderCameraValues.Add(ReadFloat(Commander.Camera + Index * 4));
                                                            Actor.ClassData.Reset();
                                                            Actor.ClassData.Append(Data + Commander.Tail, int32(Offsets.ClassData + Offsets.ClassDataSize - Commander.Tail));
                                                        }
                                                    }
                                                    if (Actor.ClassId == 0x0a)
                                                    {
                                                        FKyaNativOffsets Nativ;
                                                        Actor.bHasNativData = KyaReadNativOffsets(
                                                            MakeArrayView(Data, int32(Entry.Size)), Offsets.ClassData, Offsets.ClassDataSize, Nativ);
                                                        if (Actor.bHasNativData)
                                                        {
                                                            Actor.NativWeightB = ReadFloat(Nativ.WeightB);
                                                            Actor.NativLifeMax = float(ReadU32(Data + Nativ.Autonomous));
                                                            Actor.NativGravityScale = ReadFloat(Nativ.Autonomous + 4);
                                                            Actor.NativDynamicExtField4 = ReadFloat(Nativ.Autonomous + 8);
                                                            Actor.NativDynamicExtField8 = ReadFloat(Nativ.Autonomous + 12);
                                                            Actor.NativField350 = int32(ReadU32(Data + Nativ.Field350));
                                                            Actor.NativField354 = int32(ReadU32(Data + Nativ.Field354));
                                                            Actor.NativField358 = int32(ReadU32(Data + Nativ.Field358));
                                                            Actor.NativMoveSpeedNoHeld = ReadFloat(Nativ.MoveSpeedNoHeld);
                                                            Actor.NativMoveSpeedHeld = ReadFloat(Nativ.MoveSpeedHeld);
                                                            Actor.NativField370 = int32(ReadU32(Data + Nativ.Field370));
                                                            Actor.NativField368 = int32(ReadU32(Data + Nativ.Field368));
                                                            Actor.NativField364 = int32(ReadU32(Data + Nativ.Field364));
                                                            Actor.NativField378 = int32(ReadU32(Data + Nativ.Field378));
                                                            Actor.NativField37C = int32(ReadU32(Data + Nativ.Field37C));
                                                            Actor.NativField380 = int32(ReadU32(Data + Nativ.Field380));
                                                            Actor.ClassData.Reset();
                                                            Actor.ClassData.Append(Data + Nativ.Tail, int32(Offsets.ClassData + Offsets.ClassDataSize - Nativ.Tail));
                                                        }
                                                    }
                                                    if (Actor.ClassId == 0x29)
                                                    {
                                                        FKyaAmortosOffsets Amortos;
                                                        Actor.bHasAmortosData = KyaReadAmortosOffsets(
                                                            MakeArrayView(Data, int32(Entry.Size)), Offsets.ClassData, Offsets.ClassDataSize, Amortos);
                                                        if (Actor.bHasAmortosData)
                                                        {
                                                            for (int32 Stream = 0; Stream < 3; ++Stream)
                                                            {
                                                                TArray<int32>& List = Stream == 0 ? Actor.AmortosCheckpointActors :
                                                                    (Stream == 1 ? Actor.AmortosActors188 : Actor.AmortosActors18C);
                                                                for (int32 Index = 0; Index < Amortos.Streams[Stream].CountValue; ++Index)
                                                                    List.Add(int32(ReadU32(Data + Amortos.Streams[Stream].Entries + Index * 4)));
                                                            }
                                                            Actor.AmortosActorRef = int32(ReadU32(Data + Amortos.Field190));
                                                            Actor.AmortosField160 = ReadFloat(Amortos.Floats160);
                                                            Actor.AmortosField164 = ReadFloat(Amortos.Floats160 + 4);
                                                            Actor.AmortosField168 = ReadFloat(Amortos.Floats160 + 8);
                                                            Actor.AmortosField16C = ReadFloat(Amortos.Floats160 + 12);
                                                            Actor.AmortosField170 = ReadFloat(Amortos.Floats160 + 16);
                                                            Actor.AmortosField174 = ReadFloat(Amortos.Floats160 + 20);
                                                            Actor.AmortosField178 = ReadFloat(Amortos.Floats160 + 24);
                                                            Actor.AmortosField17C = ReadFloat(Amortos.Floats160 + 28);
                                                            Actor.AmortosField180 = ReadFloat(Amortos.Floats160 + 32);
                                                            FKyaAddOnGenerator& AddOn = Actor.AmortosAddOn;
                                                            const int64 A = Amortos.AddOn;
                                                            AddOn.Field0 = int32(ReadU32(Data + A));
                                                            AddOn.MoneyType = int32(ReadU32(Data + A + 4));
                                                            AddOn.MoneyCount = int32(ReadU32(Data + A + 8));
                                                            AddOn.BonusCount = int32(ReadU32(Data + A + 12));
                                                            AddOn.FieldC = int32(ReadU32(Data + A + 16));
                                                            AddOn.SubObjMode = Amortos.SubObjModeValue;
                                                            if (Amortos.SubObjFloats)
                                                            {
                                                                AddOn.SubObjField0 = ReadFloat(Amortos.SubObjFloats);
                                                                AddOn.SubObjField4 = ReadFloat(Amortos.SubObjFloats + 4);
                                                                AddOn.SubObjFieldC = ReadFloat(Amortos.SubObjFloats + 8);
                                                                AddOn.SubObjField10 = ReadFloat(Amortos.SubObjFloats + 12);
                                                                AddOn.SubObjField14 = ReadFloat(Amortos.SubObjFloats + 16);
                                                            }
                                                            Actor.AmortosField1C8 = int32(ReadU32(Data + Amortos.Field1C8));
                                                            KyaReadSwitch(MakeArrayView(Data, int32(Entry.Size)), Amortos.Switches[0].Targets, Amortos.Switches[0].Count, Amortos.Switches[0].Camera, Actor.AmortosSwitch1CC);
                                                            KyaReadSwitch(MakeArrayView(Data, int32(Entry.Size)), Amortos.Switches[1].Targets, Amortos.Switches[1].Count, Amortos.Switches[1].Camera, Actor.AmortosSwitch1D4);
                                                            Actor.ClassData.Reset();
                                                            Actor.ClassData.Append(Data + Amortos.Tail, int32(Offsets.ClassData + Offsets.ClassDataSize - Amortos.Tail));
                                                        }
                                                    }
                                                    if (Actor.ClassId == 0x24)
                                                    {
                                                        FKyaTeleporterOffsets Teleporter;
                                                        Actor.bHasTeleporterData = KyaReadTeleporterOffsets(
                                                            MakeArrayView(Data, int32(Entry.Size)), Offsets.ClassData, Offsets.ClassDataSize, Teleporter);
                                                        if (Actor.bHasTeleporterData)
                                                        {
                                                            Actor.TeleporterSubsectorMaterialId = int32(ReadU32(Data + Teleporter.SubsectorMaterialId));
                                                            Actor.TeleporterHash = int64(uint64(ReadU32(Data + Teleporter.Hash)) | (uint64(ReadU32(Data + Teleporter.Hash + 4)) << 32));
                                                            Actor.TeleporterField16C = int32(ReadU32(Data + Teleporter.Field16C));
                                                            for (int32 Index = 0; Index < Teleporter.DestCountValue; ++Index)
                                                            {
                                                                FKyaTeleporterDestination& Destination = Actor.TeleporterDestinations.AddDefaulted_GetRef();
                                                                const int64 D = Teleporter.DestEntries + int64(Index) * 12;
                                                                Destination.LevelId = int32(ReadU32(Data + D));
                                                                Destination.ElevatorId = int32(ReadU32(Data + D + 4));
                                                                Destination.Field8 = int32(ReadU32(Data + D + 8));
                                                            }
                                                            KyaReadSwitch(MakeArrayView(Data, int32(Entry.Size)), Teleporter.Switches[0].Targets, Teleporter.Switches[0].Count, Teleporter.Switches[0].Camera, Actor.TeleporterSwitch170);
                                                            KyaReadSwitch(MakeArrayView(Data, int32(Entry.Size)), Teleporter.Switches[1].Targets, Teleporter.Switches[1].Count, Teleporter.Switches[1].Camera, Actor.TeleporterSwitch178);
                                                            KyaReadSwitch(MakeArrayView(Data, int32(Entry.Size)), Teleporter.Switches[2].Targets, Teleporter.Switches[2].Count, Teleporter.Switches[2].Camera, Actor.TeleporterSwitch180);
                                                            Actor.TeleporterZone188 = int32(ReadU32(Data + Teleporter.Zone188));
                                                            Actor.TeleporterZone18C = int32(ReadU32(Data + Teleporter.Zone18C));
                                                            Actor.TeleporterMaterialId1 = int32(ReadU32(Data + Teleporter.Material1));
                                                            Actor.TeleporterCinematicId = int32(ReadU32(Data + Teleporter.CinematicId));
                                                            Actor.TeleporterMaterialId2 = int32(ReadU32(Data + Teleporter.Material2));
                                                            Actor.TeleporterField1D4 = int32(ReadU32(Data + Teleporter.Field1D4));
                                                            Actor.TeleporterField1D8 = int32(ReadU32(Data + Teleporter.Field1D8));
                                                            Actor.TeleporterMaterialId3 = int32(ReadU32(Data + Teleporter.Material3));
                                                            Actor.TeleporterField1E0 = int32(ReadU32(Data + Teleporter.Field1E0));
                                                            for (int32 Index = 0; Index < Teleporter.WaypointCountValue; ++Index)
                                                            {
                                                                FKyaTeleporterWaypoint& Waypoint = Actor.TeleporterWaypoints.AddDefaulted_GetRef();
                                                                Waypoint.ActorRef = int32(ReadU32(Data + Teleporter.WaypointEntries + Index * 8));
                                                                Waypoint.WaypointRef = int32(ReadU32(Data + Teleporter.WaypointEntries + Index * 8 + 4));
                                                            }
                                                            for (int32 Index = 0; Index < Teleporter.BlockOps.Num(); ++Index)
                                                            {
                                                                FKyaTeleporterBlock& Block = Actor.TeleporterConditionedOperations.AddDefaulted_GetRef();
                                                                for (int32 Op = 0; Op < Teleporter.BlockOpCountValues[Index]; ++Op)
                                                                {
                                                                    FKyaTeleporterOperation& Operation = Block.Operations.AddDefaulted_GetRef();
                                                                    const int64 O = Teleporter.BlockOps[Index] + int64(Op) * 12;
                                                                    Operation.ScenaricVariable = int32(ReadU32(Data + O));
                                                                    Operation.Operation = int32(ReadU32(Data + O + 4));
                                                                    Operation.Value = int32(ReadU32(Data + O + 8));
                                                                }
                                                                for (int32 Cond = 0; Cond < Teleporter.BlockCondCountValues[Index]; ++Cond)
                                                                {
                                                                    FKyaTeleporterCondition& Condition = Block.Conditions.AddDefaulted_GetRef();
                                                                    const int64 C = Teleporter.BlockConds[Index] + int64(Cond) * 16;
                                                                    Condition.Combine = int32(ReadU32(Data + C));
                                                                    Condition.Variable = int32(ReadU32(Data + C + 4));
                                                                    Condition.Comparison = int32(ReadU32(Data + C + 8));
                                                                    Condition.Value = int32(ReadU32(Data + C + 12));
                                                                }
                                                            }
                                                            Actor.ClassData.Reset();
                                                            Actor.ClassData.Append(Data + Teleporter.Tail, int32(Offsets.ClassData + Offsets.ClassDataSize - Teleporter.Tail));
                                                        }
                                                    }
                                                    if (Actor.ClassId == 0x1b)
                                                    {
                                                        FKyaDcaOffsets Dca;
                                                        Actor.bHasDcaData = KyaReadDcaOffsets(
                                                            MakeArrayView(Data, int32(Entry.Size)), Offsets.ClassData, Offsets.ClassDataSize, Dca);
                                                        if (Actor.bHasDcaData)
                                                        {
                                                            Actor.DcaField504 = int32(ReadU32(Data + Dca.Field504));
                                                            Actor.DcaField500 = int32(ReadU32(Data + Dca.Field500));
                                                            Actor.DcaField464 = ReadFloat(Dca.Field464);
                                                            Actor.DcaField508 = int32(ReadU32(Data + Dca.Field508));
                                                            Actor.DcaField50C = int32(ReadU32(Data + Dca.Field50C));
                                                            Actor.DcaAimAdjustDegrees = ReadFloat(Dca.AimAdjust);
                                                            for (int32 Index = 0; Index < Dca.ShotCountValue; ++Index)
                                                                Actor.DcaShotActorRefs.Add(int32(ReadU32(Data + Dca.ShotEntries + Index * 4)));
                                                            Actor.DcaField450 = int32(ReadU32(Data + Dca.Field450));
                                                            Actor.DcaField454 = int32(ReadU32(Data + Dca.Field454));
                                                            Actor.ClassData.Reset();
                                                            Actor.ClassData.Append(Data + Dca.Tail, int32(Offsets.ClassData + Offsets.ClassDataSize - Dca.Tail));
                                                        }
                                                    }
                                                    if (Actor.ClassId == 0x35)
                                                    {
                                                        FKyaBoxOffsets Box;
                                                        Actor.bHasBoxData = KyaReadBoxOffsets(
                                                            MakeArrayView(Data, int32(Entry.Size)), Offsets.ClassData, Offsets.ClassDataSize, Box);
                                                        if (Actor.bHasBoxData)
                                                        {
                                                            Actor.BoxWeightB = ReadFloat(Box.WeightB);
                                                            Actor.BoxLifeMax = float(ReadU32(Data + Box.Autonomous));
                                                            Actor.BoxGravityScale = ReadFloat(Box.Autonomous + 4);
                                                            Actor.BoxDynamicExtField4 = ReadFloat(Box.Autonomous + 8);
                                                            Actor.BoxDynamicExtField8 = ReadFloat(Box.Autonomous + 12);
                                                            Actor.BoxField350 = int32(ReadU32(Data + Box.Field350));
                                                            for (int32 Index = 0; Index < Box.ActorCountValue; ++Index)
                                                                Actor.BoxActorRefs.Add(int32(ReadU32(Data + Box.ActorEntries + Index * 4)));
                                                            Actor.BoxField354 = ReadFloat(Box.Floats354);
                                                            Actor.BoxField358 = ReadFloat(Box.Floats354 + 4);
                                                            Actor.BoxField35C = ReadFloat(Box.Floats354 + 8);
                                                            Actor.BoxField360 = ReadFloat(Box.Floats354 + 12);
                                                            Actor.BoxField364 = int32(ReadU32(Data + Box.Field364));
                                                            Actor.BoxField370 = int32(ReadU32(Data + Box.Field370));
                                                            Actor.BoxField374 = int32(ReadU32(Data + Box.Field370 + 4));
                                                            Actor.BoxField378 = int32(ReadU32(Data + Box.Field370 + 8));
                                                            Actor.BoxField37C = int32(ReadU32(Data + Box.Field370 + 12));
                                                            KyaReadSwitch(MakeArrayView(Data, int32(Entry.Size)), Box.Switch.Targets, Box.Switch.Count, Box.Switch.Camera, Actor.BoxSwitch);
                                                            FKyaAddOnGenerator& AddOn = Actor.BoxAddOn;
                                                            const int64 A = Box.AddOn;
                                                            AddOn.Field0 = int32(ReadU32(Data + A));
                                                            AddOn.MoneyType = int32(ReadU32(Data + A + 4));
                                                            AddOn.MoneyCount = int32(ReadU32(Data + A + 8));
                                                            AddOn.BonusCount = int32(ReadU32(Data + A + 12));
                                                            AddOn.FieldC = int32(ReadU32(Data + A + 16));
                                                            AddOn.SubObjMode = Box.SubObjModeValue;
                                                            if (Box.SubObjFloats)
                                                            {
                                                                AddOn.SubObjField0 = ReadFloat(Box.SubObjFloats);
                                                                AddOn.SubObjField4 = ReadFloat(Box.SubObjFloats + 4);
                                                                AddOn.SubObjFieldC = ReadFloat(Box.SubObjFloats + 8);
                                                                AddOn.SubObjField10 = ReadFloat(Box.SubObjFloats + 12);
                                                                AddOn.SubObjField14 = ReadFloat(Box.SubObjFloats + 16);
                                                            }
                                                            Actor.ClassData.Reset();
                                                            Actor.ClassData.Append(Data + Box.Tail, int32(Offsets.ClassData + Offsets.ClassDataSize - Box.Tail));
                                                        }
                                                    }
                                                    if (Actor.ClassId == 0x32)
                                                    {
                                                        FKyaMoneyOffsets Money;
                                                        Actor.bHasMoneyData = KyaReadMoneyOffsets(
                                                            MakeArrayView(Data, int32(Entry.Size)), Offsets.ClassData, Offsets.ClassDataSize, Money);
                                                        if (Actor.bHasMoneyData)
                                                        {
                                                            Actor.MoneyWeightB = ReadFloat(Money.WeightB);
                                                            Actor.MoneyValue = int32(ReadU32(Data + Money.MoneyValue));
                                                            Actor.MoneySoundRef = int32(ReadU32(Data + Money.Sound));
                                                            Actor.MoneyMaterialId = int32(ReadU32(Data + Money.Material));
                                                            Actor.ClassData.Reset();
                                                            Actor.ClassData.Append(Data + Money.Tail, int32(Offsets.ClassData + Offsets.ClassDataSize - Money.Tail));
                                                        }
                                                    }
                                                    if (Actor.ClassId == 0x3d)
                                                    {
                                                        FKyaFogOffsets Fog;
                                                        Actor.bHasFogData = KyaReadFogOffsets(
                                                            MakeArrayView(Data, int32(Entry.Size)), Offsets.ClassData, Offsets.ClassDataSize, Fog);
                                                        if (Actor.bHasFogData)
                                                        {
                                                            for (int32 Index = 0; Index < Fog.CountValue; ++Index)
                                                            {
                                                                FKyaFogHierarchy& Hierarchy = Actor.FogHierarchies.AddDefaulted_GetRef();
                                                                const int64 H = Fog.Entries + int64(Index) * 16;
                                                                Hierarchy.Hash = int64(uint64(ReadU32(Data + H)) | (uint64(ReadU32(Data + H + 4)) << 32));
                                                                Hierarchy.Field20 = ReadFloat(H + 8);
                                                                Hierarchy.Field24 = ReadFloat(H + 12);
                                                            }
                                                            Actor.ClassData.Reset();
                                                            Actor.ClassData.Append(Data + Fog.Tail, int32(Offsets.ClassData + Offsets.ClassDataSize - Fog.Tail));
                                                        }
                                                    }
                                                    if (Actor.ClassId == 0x3e)
                                                    {
                                                        FKyaShootOffsets Shoot;
                                                        Actor.bHasShootData = KyaReadShootOffsets(
                                                            MakeArrayView(Data, int32(Entry.Size)), Offsets.ClassData, Offsets.ClassDataSize, Shoot);
                                                        if (Actor.bHasShootData)
                                                        {
                                                            Actor.ShootWeightB = ReadFloat(Shoot.WeightB);
                                                            Actor.ShootLifeMax = float(ReadU32(Data + Shoot.Autonomous));
                                                            Actor.ShootGravityScale = ReadFloat(Shoot.Autonomous + 4);
                                                            Actor.ShootDynamicExtField4 = ReadFloat(Shoot.Autonomous + 8);
                                                            Actor.ShootDynamicExtField8 = ReadFloat(Shoot.Autonomous + 12);
                                                            Actor.ShootField350 = int32(ReadU32(Data + Shoot.Field350));
                                                            Actor.ShootField354 = int32(ReadU32(Data + Shoot.Field354));
                                                            Actor.ShootTextureIndex = int32(ReadU32(Data + Shoot.Texture));
                                                            Actor.ShootMeshIndex = int32(ReadU32(Data + Shoot.Mesh));
                                                            Actor.ShootField3C0 = ReadFloat(Shoot.Floats3C0);
                                                            Actor.ShootField3C4 = ReadFloat(Shoot.Floats3C0 + 4);
                                                            Actor.ShootField3C8 = ReadFloat(Shoot.Floats3C0 + 8);
                                                            Actor.ShootField3CC = ReadFloat(Shoot.Floats3C0 + 12);
                                                            Actor.ShootField3D0 = int32(ReadU32(Data + Shoot.Field3D0));
                                                            Actor.ShootField3D4 = ReadFloat(Shoot.Floats3D4);
                                                            Actor.ShootField3D8 = ReadFloat(Shoot.Floats3D4 + 4);
                                                            Actor.ShootField3DC = ReadFloat(Shoot.Floats3D4 + 8);
                                                            Actor.ShootField3E0 = ReadFloat(Shoot.Floats3D4 + 12);
                                                            Actor.ShootField3E8 = int32(ReadU32(Data + Shoot.Fields3E8));
                                                            Actor.ShootField3E4 = int32(ReadU32(Data + Shoot.Fields3E8 + 4));
                                                            Actor.ShootField3EC = int32(ReadU32(Data + Shoot.Fields3E8 + 8));
                                                            Actor.ShootField3F0 = int32(ReadU32(Data + Shoot.Fields3E8 + 12));
                                                            FKyaAddOnGenerator& AddOn = Actor.ShootAddOn;
                                                            const int64 A = Shoot.AddOn;
                                                            AddOn.Field0 = int32(ReadU32(Data + A));
                                                            AddOn.MoneyType = int32(ReadU32(Data + A + 4));
                                                            AddOn.MoneyCount = int32(ReadU32(Data + A + 8));
                                                            AddOn.BonusCount = int32(ReadU32(Data + A + 12));
                                                            AddOn.FieldC = int32(ReadU32(Data + A + 16));
                                                            AddOn.SubObjMode = Shoot.SubObjModeValue;
                                                            if (Shoot.SubObjFloats)
                                                            {
                                                                AddOn.SubObjField0 = ReadFloat(Shoot.SubObjFloats);
                                                                AddOn.SubObjField4 = ReadFloat(Shoot.SubObjFloats + 4);
                                                                AddOn.SubObjFieldC = ReadFloat(Shoot.SubObjFloats + 8);
                                                                AddOn.SubObjField10 = ReadFloat(Shoot.SubObjFloats + 12);
                                                                AddOn.SubObjField14 = ReadFloat(Shoot.SubObjFloats + 16);
                                                            }
                                                            Actor.ClassData.Reset();
                                                            Actor.ClassData.Append(Data + Shoot.Tail, int32(Offsets.ClassData + Offsets.ClassDataSize - Shoot.Tail));
                                                        }
                                                    }
                                                    if (Actor.ClassId == 0x14)
                                                    {
                                                        FKyaAmbreOffsets Ambre;
                                                        Actor.bHasAmbreData = KyaReadAmbreOffsets(
                                                            MakeArrayView(Data, int32(Entry.Size)), Offsets.ClassData, Offsets.ClassDataSize, Ambre);
                                                        if (Actor.bHasAmbreData)
                                                        {
                                                            Actor.AmbreMaterialId = int32(ReadU32(Data + Ambre.Material));
                                                            Actor.AmbreEffectType = int32(ReadU32(Data + Ambre.EffectType));
                                                            Actor.AmbreTextureIndex = int32(ReadU32(Data + Ambre.Texture));
                                                            Actor.AmbreMeshIndex = int32(ReadU32(Data + Ambre.Mesh));
                                                            Actor.AmbreField1D0 = int32(ReadU32(Data + Ambre.Field1D0));
                                                            KyaReadSwitch(MakeArrayView(Data, int32(Entry.Size)), Ambre.Switch.Targets, Ambre.Switch.Count, Ambre.Switch.Camera, Actor.AmbreSwitch);
                                                            Actor.ClassData.Reset();
                                                            Actor.ClassData.Append(Data + Ambre.Tail, int32(Offsets.ClassData + Offsets.ClassDataSize - Ambre.Tail));
                                                        }
                                                    }
                                                    if (Actor.ClassId == 0x4c)
                                                    {
                                                        FKyaStillerOffsets Stiller;
                                                        Actor.bHasStillerData = KyaReadStillerOffsets(
                                                            MakeArrayView(Data, int32(Entry.Size)), Offsets.ClassData, Offsets.ClassDataSize, Stiller);
                                                        if (Actor.bHasStillerData)
                                                        {
                                                            Actor.StillerVisionRange = ReadFloat(Stiller.Vision);
                                                            Actor.StillerVisionHalfAngle = ReadFloat(Stiller.Vision + 4);
                                                            Actor.StillerVisionField38 = ReadFloat(Stiller.Vision + 8);
                                                            Actor.StillerVisionField3C = ReadFloat(Stiller.Vision + 12);
                                                            Actor.StillerVisionFlags = int32(ReadU32(Data + Stiller.Vision + 16));
                                                            Actor.StillerField1D0 = int32(ReadU32(Data + Stiller.Field1D0));
                                                            Actor.StillerField1D4 = int32(ReadU32(Data + Stiller.Field1D0 + 4));
                                                            Actor.StillerField1D8 = int32(ReadU32(Data + Stiller.Field1D0 + 8));
                                                            Actor.StillerField1DC = int32(ReadU32(Data + Stiller.Field1D0 + 12));
                                                            Actor.StillerPathIndex = int32(ReadU32(Data + Stiller.Path));
                                                            Actor.StillerField1E0 = ReadFloat(Stiller.Field1E0);
                                                            Actor.StillerLifeMax = ReadFloat(Stiller.LifeMax);
                                                            Actor.StillerField1EC = ReadFloat(Stiller.Floats1EC);
                                                            Actor.StillerField1F0 = ReadFloat(Stiller.Floats1EC + 4);
                                                            Actor.StillerField1F4 = ReadFloat(Stiller.Floats1EC + 8);
                                                            Actor.StillerField1F8 = int32(ReadU32(Data + Stiller.Field1F8));
                                                            for (int32 Index = 0; Index < Stiller.ActorCountValue; ++Index)
                                                                Actor.StillerActorRefs.Add(int32(ReadU32(Data + Stiller.ActorEntries + Index * 4)));
                                                            Actor.StillerField200 = int32(ReadU32(Data + Stiller.Field200));
                                                            KyaReadSwitch(MakeArrayView(Data, int32(Entry.Size)), Stiller.Switches[0].Targets, Stiller.Switches[0].Count, Stiller.Switches[0].Camera, Actor.StillerSwitch208);
                                                            KyaReadSwitch(MakeArrayView(Data, int32(Entry.Size)), Stiller.Switches[1].Targets, Stiller.Switches[1].Count, Stiller.Switches[1].Camera, Actor.StillerSwitch210);
                                                            Actor.StillerField214 = int32(ReadU32(Data + Stiller.Field214));
                                                            FKyaAddOnGenerator& AddOn = Actor.StillerAddOn;
                                                            const int64 A = Stiller.AddOn;
                                                            AddOn.Field0 = int32(ReadU32(Data + A));
                                                            AddOn.MoneyType = int32(ReadU32(Data + A + 4));
                                                            AddOn.MoneyCount = int32(ReadU32(Data + A + 8));
                                                            AddOn.BonusCount = int32(ReadU32(Data + A + 12));
                                                            AddOn.FieldC = int32(ReadU32(Data + A + 16));
                                                            AddOn.SubObjMode = Stiller.SubObjModeValue;
                                                            if (Stiller.SubObjFloats)
                                                            {
                                                                AddOn.SubObjField0 = ReadFloat(Stiller.SubObjFloats);
                                                                AddOn.SubObjField4 = ReadFloat(Stiller.SubObjFloats + 4);
                                                                AddOn.SubObjFieldC = ReadFloat(Stiller.SubObjFloats + 8);
                                                                AddOn.SubObjField10 = ReadFloat(Stiller.SubObjFloats + 12);
                                                                AddOn.SubObjField14 = ReadFloat(Stiller.SubObjFloats + 16);
                                                            }
                                                            Actor.ClassData.Reset();
                                                            Actor.ClassData.Append(Data + Stiller.Tail, int32(Offsets.ClassData + Offsets.ClassDataSize - Stiller.Tail));
                                                        }
                                                    }
                                                    if (Actor.ClassId == 0x51)
                                                    {
                                                        FKyaRuneOffsets Rune;
                                                        Actor.bHasRuneData = KyaReadRuneOffsets(
                                                            MakeArrayView(Data, int32(Entry.Size)), Offsets.ClassData, Offsets.ClassDataSize, Rune);
                                                        if (Actor.bHasRuneData)
                                                        {
                                                            Actor.RuneCinematicId = int32(ReadU32(Data + Rune.Cinematic));
                                                            Actor.RuneTriggerDistance = ReadFloat(Rune.TriggerDistance);
                                                            Actor.RuneScenVarId = int32(ReadU32(Data + Rune.ScenVar));
                                                            Actor.RuneFxId = int32(ReadU32(Data + Rune.Fx));
                                                            Actor.RuneMaterialIdA = int32(ReadU32(Data + Rune.MaterialA));
                                                            Actor.RuneMaterialIdB = int32(ReadU32(Data + Rune.MaterialB));
                                                            Actor.ClassData.Reset();
                                                            Actor.ClassData.Append(Data + Rune.Tail, int32(Offsets.ClassData + Offsets.ClassDataSize - Rune.Tail));
                                                        }
                                                    }
                                                    if (Actor.ClassId == 0x52)
                                                    {
                                                        FKyaBonusFountainOffsets Fountain;
                                                        Actor.bHasBonusFountainData = KyaReadBonusFountainOffsets(
                                                            MakeArrayView(Data, int32(Entry.Size)), Offsets.ClassData, Offsets.ClassDataSize, Fountain);
                                                        if (Actor.bHasBonusFountainData)
                                                        {
                                                            FKyaAddOnGenerator& AddOn = Actor.BonusFountainAddOn;
                                                            const int64 A = Fountain.AddOn;
                                                            AddOn.Field0 = int32(ReadU32(Data + A));
                                                            AddOn.MoneyType = int32(ReadU32(Data + A + 4));
                                                            AddOn.MoneyCount = int32(ReadU32(Data + A + 8));
                                                            AddOn.BonusCount = int32(ReadU32(Data + A + 12));
                                                            AddOn.FieldC = int32(ReadU32(Data + A + 16));
                                                            AddOn.SubObjMode = Fountain.SubObjModeValue;
                                                            if (Fountain.SubObjFloats)
                                                            {
                                                                AddOn.SubObjField0 = ReadFloat(Fountain.SubObjFloats);
                                                                AddOn.SubObjField4 = ReadFloat(Fountain.SubObjFloats + 4);
                                                                AddOn.SubObjFieldC = ReadFloat(Fountain.SubObjFloats + 8);
                                                                AddOn.SubObjField10 = ReadFloat(Fountain.SubObjFloats + 12);
                                                                AddOn.SubObjField14 = ReadFloat(Fountain.SubObjFloats + 16);
                                                            }
                                                            Actor.ClassData.Reset();
                                                            Actor.ClassData.Append(Data + Fountain.Tail, int32(Offsets.ClassData + Offsets.ClassDataSize - Fountain.Tail));
                                                        }
                                                    }
                                                    if (Actor.ClassId == 0x0f)
                                                    {
                                                        FKyaRopeOffsets Rope;
                                                        Actor.bHasRopeData = KyaReadRopeOffsets(
                                                            MakeArrayView(Data, int32(Entry.Size)), Offsets.ClassData, Offsets.ClassDataSize, Rope);
                                                        if (Actor.bHasRopeData)
                                                        {
                                                            Actor.RopePathIndex = int32(ReadU32(Data + Rope.Path));
                                                            Actor.RopeScale = ReadFloat(Rope.Scale);
                                                            Actor.RopeField17C = ReadFloat(Rope.Field17C);
                                                            Actor.RopeMaterialId = int32(ReadU32(Data + Rope.Material));
                                                            Actor.RopeColor = int32(ReadU32(Data + Rope.Color));
                                                            Actor.RopeSoundRef = int32(ReadU32(Data + Rope.Sound));
                                                            Actor.RopeField194 = int32(ReadU32(Data + Rope.Field194));
                                                            Actor.RopeActorRefA = int32(ReadU32(Data + Rope.ActorA));
                                                            Actor.RopeActorRefB = int32(ReadU32(Data + Rope.ActorB));
                                                            KyaReadSwitch(MakeArrayView(Data, int32(Entry.Size)), Rope.Switch.Targets, Rope.Switch.Count, Rope.Switch.Camera, Actor.RopeSwitch);
                                                            Actor.RopeFlags = int32(ReadU32(Data + Rope.Flags));
                                                            Actor.ClassData.Reset();
                                                            Actor.ClassData.Append(Data + Rope.Tail, int32(Offsets.ClassData + Offsets.ClassDataSize - Rope.Tail));
                                                        }
                                                    }
                                                    if (Actor.ClassId == 0x10)
                                                    {
                                                        FKyaGravityOffsets Gravity;
                                                        Actor.bHasGravityAwareData = KyaReadGravityOffsets(
                                                            MakeArrayView(Data, int32(Entry.Size)), Offsets.ClassData, Offsets.ClassDataSize, Gravity);
                                                        if (Actor.bHasGravityAwareData)
                                                        {
                                                            Actor.GravityWeightB = ReadFloat(Gravity.WeightB);
                                                            Actor.GravityLifeMax = float(ReadU32(Data + Gravity.Autonomous));
                                                            Actor.GravityScale = ReadFloat(Gravity.Autonomous + 4);
                                                            Actor.GravityDynamicExtField4 = ReadFloat(Gravity.Autonomous + 8);
                                                            Actor.GravityDynamicExtField8 = ReadFloat(Gravity.Autonomous + 12);
                                                            Actor.ClassData.Reset();
                                                            Actor.ClassData.Append(Data + Gravity.Tail, int32(Offsets.ClassData + Offsets.ClassDataSize - Gravity.Tail));
                                                        }
                                                    }
                                                    if (Actor.ClassId == 0x3c)
                                                    {
                                                        FKyaNativCmdOffsets Command;
                                                        Actor.bHasNativCmdData = KyaReadNativCmdOffsets(
                                                            MakeArrayView(Data, int32(Entry.Size)), Offsets.ClassData, Offsets.ClassDataSize, Command);
                                                        if (Actor.bHasNativCmdData)
                                                        {
                                                            for (int32 Index = 0; Index < Command.ActorCountValue; ++Index)
                                                                Actor.NativCmdActorRefs.Add(int32(ReadU32(Data + Command.ActorEntries + Index * 4)));
                                                            Actor.NativCmdField164 = int32(ReadU32(Data + Command.Field164));
                                                            int32 SubObjIndex = 0;
                                                            for (int32 AddOnIndex = 0; AddOnIndex < Command.AddOnCountValues.Num(); ++AddOnIndex)
                                                            {
                                                                FKyaNativCmdAddOn& AddOn = Actor.NativCmdAddOns.AddDefaulted_GetRef();
                                                                for (int32 Sub = 0; Sub < Command.AddOnCountValues[AddOnIndex]; ++Sub)
                                                                {
                                                                    const int64 SubOffset = Command.SubObjOffsets[SubObjIndex++];
                                                                    FKyaAddOnSubObj& Item = AddOn.SubObjs.AddDefaulted_GetRef();
                                                                    Item.Field0 = int32(ReadU32(Data + SubOffset));
                                                                    const uint32 Cinematics = ReadU32(Data + SubOffset + 4);
                                                                    for (uint32 Cine = 0; Cine < Cinematics; ++Cine)
                                                                        Item.CinematicIds.Add(int32(ReadU32(Data + SubOffset + 8 + Cine * 4)));
                                                                }
                                                            }
                                                            Actor.ClassData.Reset();
                                                            Actor.ClassData.Append(Data + Command.Tail, int32(Offsets.ClassData + Offsets.ClassDataSize - Command.Tail));
                                                        }
                                                    }
                                                    if (Actor.ClassId == 0x19)
                                                    {
                                                        FKyaCompanionOffsets Companion;
                                                        Actor.bHasCompanionData = KyaReadCompanionOffsets(
                                                            MakeArrayView(Data, int32(Entry.Size)), Offsets.ClassData, Offsets.ClassDataSize, Companion);
                                                        if (Actor.bHasCompanionData)
                                                        {
                                                            Actor.CompanionWeightB = ReadFloat(Companion.WeightB);
                                                            Actor.CompanionLifeMax = float(ReadU32(Data + Companion.Autonomous));
                                                            Actor.CompanionGravityScale = ReadFloat(Companion.Autonomous + 4);
                                                            Actor.CompanionDynamicExtField4 = ReadFloat(Companion.Autonomous + 8);
                                                            Actor.CompanionDynamicExtField8 = ReadFloat(Companion.Autonomous + 12);
                                                            Actor.CompanionField358 = int32(ReadU32(Data + Companion.Field358));
                                                            Actor.CompanionMaterial35C = int32(ReadU32(Data + Companion.Field358 + 4));
                                                            Actor.CompanionField360 = int32(ReadU32(Data + Companion.Field358 + 8));
                                                            Actor.ClassData.Reset();
                                                            Actor.ClassData.Append(Data + Companion.Tail, int32(Offsets.ClassData + Offsets.ClassDataSize - Companion.Tail));
                                                        }
                                                    }
                                                    if (Actor.ClassId == 0x17)
                                                    {
                                                        FKyaBridgeOffsets Bridge;
                                                        Actor.bHasBridgeData = KyaReadBridgeOffsets(
                                                            MakeArrayView(Data, int32(Entry.Size)), Offsets.ClassData, Offsets.ClassDataSize, Bridge);
                                                        if (Actor.bHasBridgeData)
                                                        {
                                                            for (int32 Index = 0; Index < Bridge.ActorCountValue; ++Index)
                                                                Actor.BridgeActorRefs.Add(int32(ReadU32(Data + Bridge.ActorEntries + Index * 4)));
                                                            Actor.BridgeWaypointA = int32(ReadU32(Data + Bridge.WaypointA));
                                                            Actor.BridgeWaypointB = int32(ReadU32(Data + Bridge.WaypointB));
                                                            Actor.BridgeField330 = ReadFloat(Bridge.Floats330);
                                                            Actor.BridgeField334 = ReadFloat(Bridge.Floats330 + 4);
                                                            Actor.BridgeField33C = ReadFloat(Bridge.Floats330 + 8);
                                                            Actor.BridgeField340 = ReadFloat(Bridge.Floats330 + 12);
                                                            Actor.BridgeField338 = ReadFloat(Bridge.Floats330 + 16);
                                                            Actor.BridgeField344 = int32(ReadU32(Data + Bridge.Field344));
                                                            Actor.BridgeConfigFlags = int32(ReadU32(Data + Bridge.ConfigFlags));
                                                            Actor.BridgeField348 = int32(ReadU32(Data + Bridge.Field348));
                                                            Actor.BridgeStepCollisionScale = ReadFloat(Bridge.StepCollisionScale);
                                                            Actor.ClassData.Reset();
                                                            Actor.ClassData.Append(Data + Bridge.Tail, int32(Offsets.ClassData + Offsets.ClassDataSize - Bridge.Tail));
                                                        }
                                                    }
                                                    if (Actor.ClassId == 0x38)
                                                    {
                                                        FKyaAtonOffsets Aton;
                                                        Actor.bHasAtonData = KyaReadAtonOffsets(
                                                            MakeArrayView(Data, int32(Entry.Size)), Offsets.ClassData, Offsets.ClassDataSize, Aton);
                                                        if (Actor.bHasAtonData)
                                                        {
                                                            Actor.AtonWeightB = ReadFloat(Aton.WeightB);
                                                            Actor.AtonLifeMax = float(ReadU32(Data + Aton.Autonomous));
                                                            Actor.AtonGravityScale = ReadFloat(Aton.Autonomous + 4);
                                                            Actor.AtonDynamicExtField4 = ReadFloat(Aton.Autonomous + 8);
                                                            Actor.AtonDynamicExtField8 = ReadFloat(Aton.Autonomous + 12);
                                                            Actor.AtonField354 = int32(ReadU32(Data + Aton.Fields354));
                                                            Actor.AtonField358 = int32(ReadU32(Data + Aton.Fields354 + 4));
                                                            Actor.AtonField35C = int32(ReadU32(Data + Aton.Fields354 + 8));
                                                            Actor.AtonField360 = int32(ReadU32(Data + Aton.Fields354 + 12));
                                                            Actor.AtonEffectType = int32(ReadU32(Data + Aton.EffectType));
                                                            Actor.AtonTextureIndex = int32(ReadU32(Data + Aton.Texture));
                                                            Actor.AtonMeshIndex = int32(ReadU32(Data + Aton.Mesh));
                                                            Actor.AtonTrailMaterialId = int32(ReadU32(Data + Aton.Trail));
                                                            Actor.AtonPathDefaultDelay = ReadFloat(Aton.PathDelay);
                                                            for (int32 Index = 0; Index < Aton.PathCountValue; ++Index)
                                                                Actor.AtonPathIndices.Add(int32(ReadU32(Data + Aton.PathEntries + Index * 4)));
                                                            Actor.AtonAddOnField18 = int32(ReadU32(Data + Aton.AddOnField18));
                                                            for (int64 Offset : Aton.OtherOffsets)
                                                            {
                                                                FKyaAddOnSubObj& Item = Actor.AtonOtherSubObjs.AddDefaulted_GetRef();
                                                                Item.Field0 = int32(ReadU32(Data + Offset));
                                                                const uint32 Cinematics = ReadU32(Data + Offset + 4);
                                                                for (uint32 Cine = 0; Cine < Cinematics; ++Cine)
                                                                    Item.CinematicIds.Add(int32(ReadU32(Data + Offset + 8 + Cine * 4)));
                                                            }
                                                            for (int64 Offset : Aton.AtonOffsets)
                                                            {
                                                                FKyaAtonSubObj& Item = Actor.AtonSubObjs.AddDefaulted_GetRef();
                                                                Item.Field18 = int32(ReadU32(Data + Offset));
                                                                Item.Field0 = int32(ReadU32(Data + Offset + 4));
                                                                const uint32 Cinematics = ReadU32(Data + Offset + 8);
                                                                for (uint32 Cine = 0; Cine < Cinematics; ++Cine)
                                                                    Item.CinematicIds.Add(int32(ReadU32(Data + Offset + 12 + Cine * 4)));
                                                            }
                                                            Actor.AtonAddOnField24 = ReadFloat(Aton.AddOnField24);
                                                            Actor.ClassData.Reset();
                                                            Actor.ClassData.Append(Data + Aton.Tail, int32(Offsets.ClassData + Offsets.ClassDataSize - Aton.Tail));
                                                        }
                                                    }
                                                    if (Actor.ClassId == 0x06)
                                                    {
                                                        FKyaKimOffsets Kim;
                                                        Actor.bHasKimData = KyaReadKimOffsets(
                                                            MakeArrayView(Data, int32(Entry.Size)), Offsets.ClassData, Offsets.ClassDataSize, Kim);
                                                        if (Actor.bHasKimData)
                                                        {
                                                            if (!KyaPopulateFighterPrefix(Actor, Data, int32(Entry.Size), Kim.Fighter)) return nullptr;
                                                            for (int32 Index = 0; Index < 10; ++Index)
                                                                Actor.KimBoneKeys.Add(int32(ReadU32(Data + Kim.Bones + Index * 4)));
                                                            for (int32 Index = 0; Index < Kim.ClimbCountValue; ++Index)
                                                                Actor.KimClimbZones.Add(int32(ReadU32(Data + Kim.ClimbEntries + Index * 4)));
                                                            for (int32 Index = 0; Index < Kim.TutorialCountValue; ++Index)
                                                            {
                                                                const FKyaHeroTutorialOffsets& T = Kim.Tutorials[Index];
                                                                FKyaHeroTutorialZone& Zone = Actor.KimTutorialZones.AddDefaulted_GetRef();
                                                                Zone.ZoneRef = int32(ReadU32(Data + T.Header));
                                                                Zone.MessageKey = int64(uint64(ReadU32(Data + T.Header + 4)) |
                                                                    (uint64(ReadU32(Data + T.Header + 8)) << 32));
                                                                Zone.Flags = int32(ReadU32(Data + T.Header + 12));
                                                                Zone.Action = int32(ReadU32(Data + T.Header + 16));
                                                                Zone.Duration = ReadFloat(T.Header + 20);
                                                                for (int32 Target = 0; Target < T.TargetCountValue; ++Target)
                                                                {
                                                                    FKyaSwitchTarget& Item = Zone.Targets.AddDefaulted_GetRef();
                                                                    const int64 S = T.Targets + int64(Target) * 28;
                                                                    Item.TargetRef = int32(ReadU32(Data + S));
                                                                    Item.CutsceneId = int32(ReadU32(Data + S + 4));
                                                                    Item.Flags = int32(ReadU32(Data + S + 8));
                                                                    Item.MessageId = int32(ReadU32(Data + S + 12));
                                                                    Item.MessageFlags = int32(ReadU32(Data + S + 16));
                                                                    Item.Field14 = int32(ReadU32(Data + S + 20));
                                                                    Item.Field18 = int32(ReadU32(Data + S + 24));
                                                                }
                                                                const int64 C = T.Camera;
                                                                Zone.Camera.CameraIndex = int32(ReadU32(Data + C));
                                                                Zone.Camera.Flags = int32(ReadU32(Data + C + 4));
                                                                Zone.Camera.Field8 = ReadFloat(C + 8);
                                                                Zone.Camera.FieldC = ReadFloat(C + 12);
                                                                Zone.Camera.Field10 = ReadFloat(C + 16);
                                                                Zone.Camera.ActorRef = int32(ReadU32(Data + C + 20));
                                                                Zone.Camera.ZoneRef = int32(ReadU32(Data + C + 24));
                                                                Zone.Camera.Field1C = ReadFloat(C + 28);
                                                            }
                                                            for (int32 Index = 0; Index < Kim.ProjectileCountValue; ++Index)
                                                            {
                                                                FKyaHeroProjectileZone& Zone = Actor.KimProjectileZones.AddDefaulted_GetRef();
                                                                const int64 P = Kim.ProjectileEntries + int64(Index) * 28;
                                                                Zone.ZoneRef = int32(ReadU32(Data + P));
                                                                Zone.ProjectileType = int32(ReadU32(Data + P + 4));
                                                                Zone.Damage = ReadFloat(P + 8);
                                                                Zone.WaypointIndex = int32(ReadU32(Data + P + 12));
                                                                Zone.Force = ReadFloat(P + 16);
                                                                Zone.SampleCount = int32(ReadU32(Data + P + 20));
                                                                Zone.SectorId = int32(ReadU32(Data + P + 24));
                                                            }
                                                            for (int32 Index = 0; Index < Kim.PairCountValue; ++Index)
                                                            {
                                                                FKyaHeroZonePair& Pair = Actor.KimBoomyZonePairs.AddDefaulted_GetRef();
                                                                Pair.ZoneRef = int32(ReadU32(Data + Kim.PairEntries + Index * 8));
                                                                Pair.Field4 = int32(ReadU32(Data + Kim.PairEntries + Index * 8 + 4));
                                                            }
                                                            for (int32 Index = 0; Index < Kim.ZoneStreamCountValue; ++Index)
                                                                Actor.KimZoneStream.Add(int32(ReadU32(Data + Kim.ZoneStreamEntries + Index * 4)));
                                                            Actor.KimMaterialId = int32(ReadU32(Data + Kim.Material));
                                                            Actor.KimFxType = int32(ReadU32(Data + Kim.FxType));
                                                            Actor.KimJokeWarnRadius = ReadFloat(Kim.JokeRadius);
                                                            for (const FKyaActorCameraOffsets& Camera : Kim.Cameras)
                                                                Actor.KimCameras.Add(KyaReadActorCameraValues(
                                                                    MakeArrayView(Data, int32(Entry.Size)), Camera));
                                                            Actor.KimField15D4 = int32(ReadU32(Data + Kim.Field15D4));
                                                            Actor.KimFxInitType = int32(ReadU32(Data + Kim.FxInit));
                                                            for (int32 Index = 0; Index < 3; ++Index)
                                                                Actor.KimFxHandleTypes.Add(int32(ReadU32(Data + Kim.FxHandles + Index * 4)));
                                                            Actor.ClassData.Reset();
                                                            Actor.ClassData.Append(Data + Kim.Tail, int32(Offsets.ClassData + Offsets.ClassDataSize - Kim.Tail));
                                                        }
                                                    }
                                                    if (Actor.ClassId == 0x05)
                                                    {
                                                        FKyaBoomyOffsets Boomy;
                                                        Actor.bHasBoomyData = KyaReadBoomyOffsets(
                                                            MakeArrayView(Data, int32(Entry.Size)), Offsets.ClassData, Offsets.ClassDataSize, Boomy);
                                                        if (Actor.bHasBoomyData)
                                                        {
                                                            Actor.BoomyWeight = ReadFloat(Boomy.Weight);
                                                            Actor.BoomyVisionRange = ReadFloat(Boomy.Vision);
                                                            Actor.BoomyVisionAngle = ReadFloat(Boomy.Vision + 4);
                                                            Actor.BoomyVisionField38 = ReadFloat(Boomy.Vision + 8);
                                                            Actor.BoomyVisionField3C = ReadFloat(Boomy.Vision + 12);
                                                            Actor.BoomyVisionFlags = int32(ReadU32(Data + Boomy.Vision + 16));
                                                            Actor.BoomySound2B8 = int32(ReadU32(Data + Boomy.Sound2B8));
                                                            Actor.BoomySound2BC = int32(ReadU32(Data + Boomy.Sound2BC));
                                                            Actor.BoomyField744 = ReadFloat(Boomy.Field744);
                                                            Actor.BoomyTailWidthAxis = int32(ReadU32(Data + Boomy.TailWidthAxis));
                                                            Actor.BoomyField740 = ReadFloat(Boomy.Field740);
                                                            Actor.BoomyTailSegments = int32(ReadU32(Data + Boomy.TailSegments));
                                                            Actor.BoomyParticle3D0 = int32(ReadU32(Data + Boomy.Particle3D0));
                                                            Actor.BoomyParticle3D4 = int32(ReadU32(Data + Boomy.Particle3D4));
                                                            Actor.BoomyParticle3DC = int32(ReadU32(Data + Boomy.Particle3DC));
                                                            Actor.BoomyField3CC = int32(ReadU32(Data + Boomy.Field3CC));
                                                            Actor.BoomyTextureIndex = int32(ReadU32(Data + Boomy.Texture));
                                                            Actor.BoomyMeshIndex = int32(ReadU32(Data + Boomy.Mesh));
                                                            FKyaActorCamera& Camera = Actor.BoomyCamera;
                                                            Camera.Fov = ReadFloat(Boomy.CameraFov);
                                                            Camera.Position = FVector(ReadFloat(Boomy.CameraPosition),
                                                                ReadFloat(Boomy.CameraPosition + 4), ReadFloat(Boomy.CameraPosition + 8));
                                                            Camera.LookAt = FVector(ReadFloat(Boomy.CameraLookAt),
                                                                ReadFloat(Boomy.CameraLookAt + 4), ReadFloat(Boomy.CameraLookAt + 8));
                                                            Camera.Up = FVector(ReadFloat(Boomy.CameraUp),
                                                                ReadFloat(Boomy.CameraUp + 4), ReadFloat(Boomy.CameraUp + 8));
                                                            Camera.SectorId = int32(ReadU32(Data + Boomy.CameraSector));
                                                            Camera.Field08 = int32(ReadU32(Data + Boomy.CameraField08));
                                                            Camera.Flags = int32(ReadU32(Data + Boomy.CameraFlags));
                                                            Camera.TargetOffset = FVector(ReadFloat(Boomy.CameraTargetOffset),
                                                                ReadFloat(Boomy.CameraTargetOffset + 4), ReadFloat(Boomy.CameraTargetOffset + 8));
                                                            Camera.bHasSwitch = Boomy.bHasSwitch;
                                                            if (Boomy.bHasSwitch)
                                                            {
                                                                Camera.SwitchActor = int32(ReadU32(Data + Boomy.SwitchActor));
                                                                for (int32 Index = 0; Index < Boomy.SwitchCountA; ++Index)
                                                                    Camera.SwitchActorsA.Add(int32(ReadU32(Data + Boomy.SwitchListA + Index * 4)));
                                                                for (int32 Index = 0; Index < Boomy.SwitchCountB; ++Index)
                                                                    Camera.SwitchActorsB.Add(int32(ReadU32(Data + Boomy.SwitchListB + Index * 4)));
                                                                Camera.SwitchValueA = ReadFloat(Boomy.SwitchValues);
                                                                Camera.SwitchModeA = int32(ReadU32(Data + Boomy.SwitchValues + 4));
                                                                Camera.SwitchValueB = ReadFloat(Boomy.SwitchValues + 8);
                                                                Camera.SwitchModeB = int32(ReadU32(Data + Boomy.SwitchValues + 12));
                                                                Camera.SwitchValueC = ReadFloat(Boomy.SwitchValues + 16);
                                                            }
                                                            for (int32 Index = 0; Index < Boomy.ConditionCount; ++Index)
                                                            {
                                                                FKyaCameraCondition& Condition = Camera.Conditions.AddDefaulted_GetRef();
                                                                const int64 C = Boomy.Conditions + 4 + int64(Index) * 16;
                                                                Condition.Combiner = int32(ReadU32(Data + C));
                                                                Condition.ActorRef = int32(ReadU32(Data + C + 4));
                                                                Condition.Type = int32(ReadU32(Data + C + 8));
                                                                Condition.Value = ReadFloat(C + 12);
                                                            }
                                                            Camera.ConfigFlags = int32(ReadU32(Data + Boomy.Config));
                                                            Camera.ConfigActorRefA = int32(ReadU32(Data + Boomy.Config + 4));
                                                            Camera.ConfigActorRefB = int32(ReadU32(Data + Boomy.Config + 8));
                                                            Camera.ConfigAngle = ReadFloat(Boomy.Config + 12);
                                                            if (Boomy.ConfigMode == 1)
                                                                for (int32 Index = 0; Index < 5; ++Index)
                                                                    Camera.ConfigOptionalValues.Add(ReadFloat(Boomy.ConfigOptional + Index * 4));
                                                            Camera.PitchAngles = FVector(ReadFloat(Boomy.ConfigAngles),
                                                                ReadFloat(Boomy.ConfigAngles + 4), ReadFloat(Boomy.ConfigAngles + 8));
                                                            Camera.OtherAngles = FVector(ReadFloat(Boomy.ConfigAngles + 12),
                                                                ReadFloat(Boomy.ConfigAngles + 16), ReadFloat(Boomy.ConfigAngles + 20));
                                                            Camera.Distances = FVector(ReadFloat(Boomy.ConfigAngles + 24),
                                                                ReadFloat(Boomy.ConfigAngles + 28), ReadFloat(Boomy.ConfigAngles + 32));
                                                            if (Boomy.ExtraCount == 1)
                                                                for (int32 Index = 0; Index < 12; ++Index)
                                                                    Camera.ExtraValues.Add(ReadFloat(Boomy.Extra + 4 + Index * 4));
                                                            for (int32 Index = 0; Index < Boomy.TypeCountValue; ++Index)
                                                            {
                                                                FKyaBoomyType& Type = Actor.BoomyTypes.AddDefaulted_GetRef();
                                                                const int64 T = Boomy.TypeEntries + int64(Index) * 16;
                                                                Type.Field4 = ReadFloat(T);
                                                                Type.VisionRange = ReadFloat(T + 4);
                                                                Type.HitDamage = int32(ReadU32(Data + T + 8));
                                                                Type.FxColorA = int32(ReadU32(Data + T + 12));
                                                            }
                                                            Actor.ClassData.Reset();
                                                            Actor.ClassData.Append(Data + Boomy.Tail, int32(Offsets.ClassData + Offsets.ClassDataSize - Boomy.Tail));
                                                        }
                                                    }
                                                    if (Actor.ClassId == 0x1e)
                                                    {
                                                        TArray<int64> Checkpoints;
                                                        int64 CheckpointTail = 0;
                                                        Actor.bHasTypedCheckpoints = KyaReadCheckpointOffsets(MakeArrayView(Data, int32(Entry.Size)), Offsets.ClassData, Offsets.ClassDataSize, Checkpoints, &CheckpointTail);
                                                        if (Actor.bHasTypedCheckpoints)
                                                        {
                                                            for (int64 Offset : Checkpoints)
                                                            {
                                                                FKyaCheckpoint& Checkpoint = Actor.Checkpoints.AddDefaulted_GetRef();
                                                                Checkpoint.ZoneRef = int32(ReadU32(Data + Offset));
                                                                Checkpoint.WaypointA = int32(ReadU32(Data + Offset + 4));
                                                                Checkpoint.SectorId = int32(ReadU32(Data + Offset + 8));
                                                                Checkpoint.WaypointB = int32(ReadU32(Data + Offset + 12));
                                                                Checkpoint.Flags = ReadU32(Data + Offset + 16);
                                                                for (uint32 Pair = 0; Pair < ReadU32(Data + Offset + 20); ++Pair)
                                                                {
                                                                    FKyaCheckpointActorWaypoint& Ref = Checkpoint.ActorWaypoints.AddDefaulted_GetRef();
                                                                    Ref.ActorRef = int32(ReadU32(Data + Offset + 24 + Pair * 8));
                                                                    Ref.WaypointRef = int32(ReadU32(Data + Offset + 28 + Pair * 8));
                                                                }
                                                            }
                                                            Actor.ClassData.Reset();
                                                            Actor.ClassData.Append(Data + CheckpointTail, int32(Offsets.ClassData + Offsets.ClassDataSize - CheckpointTail));
                                                        }
                                                    }
                                                    for (const FKyaBehaviourOffsets& BehaviourOffset : Offsets.Behaviours)
                                                    {
                                                        FKyaActorBehaviour& Behaviour = Actor.Behaviours.AddDefaulted_GetRef();
                                                        Behaviour.Id = int32(ReadU32(Data + BehaviourOffset.Id));
                                                        Behaviour.Data.Append(Data + BehaviourOffset.Payload, BehaviourOffset.Size);
                                                        if (Actor.ClassId == 0x2e)
                                                        {
                                                            FKyaFxBehaviourValues FxValues;
                                                            Behaviour.bHasTypedFxBehaviour = KyaReadFxBehaviour(Behaviour.Id,
                                                                MakeArrayView(Data, int32(Entry.Size)), BehaviourOffset.Payload, BehaviourOffset.Size, FxValues);
                                                            if (Behaviour.bHasTypedFxBehaviour)
                                                            {
                                                                Behaviour.FxBehaviourValue = FxValues.Value;
                                                                Behaviour.FxPathFollowIndex = FxValues.PathFollowIndex;
                                                                Behaviour.FxUnknownValues = MoveTemp(FxValues.UnknownValues);
                                                            }
                                                        }
                                                        if (Actor.ClassId == 0x2c && (Behaviour.Id == 2 || Behaviour.Id == 3) && BehaviourOffset.Size == 12)
                                                        {
                                                            Behaviour.bHasTypedTrapBehaviour = true;
                                                            Behaviour.TrapBehaviourField0 = int32(ReadU32(Data + BehaviourOffset.Payload));
                                                            Behaviour.TrapBehaviourField4 = int32(ReadU32(Data + BehaviourOffset.Payload + 4));
                                                            Behaviour.TrapBehaviourField8 = int32(ReadU32(Data + BehaviourOffset.Payload + 8));
                                                        }
                                                        if (Actor.ClassId == 0x20)
                                                        {
                                                            FKyaWeaponBehaviourValues WeaponValues;
                                                            Behaviour.bHasTypedWeaponBehaviour = KyaReadWeaponBehaviour(Behaviour.Id,
                                                                MakeArrayView(Data, int32(Entry.Size)), BehaviourOffset.Payload, BehaviourOffset.Size, WeaponValues);
                                                            if (Behaviour.bHasTypedWeaponBehaviour)
                                                            {
                                                                Behaviour.WeaponBurst = MoveTemp(WeaponValues.Burst);
                                                                Behaviour.WeaponField40 = WeaponValues.Field40;
                                                                Behaviour.WeaponField44 = WeaponValues.Field44;
                                                                Behaviour.WeaponSoundIndex = WeaponValues.SoundIndex;
                                                                Behaviour.WeaponPistolField2E0 = WeaponValues.PistolField2E0;
                                                                Behaviour.WeaponPistolField2E4 = WeaponValues.PistolField2E4;
                                                                Behaviour.WeaponSniperTextureIndex = WeaponValues.SniperTextureIndex;
                                                                Behaviour.WeaponSniperMeshIndex = WeaponValues.SniperMeshIndex;
                                                                Behaviour.WeaponSniperMaterialId = WeaponValues.SniperMaterialId;
                                                                Behaviour.WeaponRpgActorRefs = MoveTemp(WeaponValues.RpgActorRefs);
                                                            }
                                                        }
                                                        if (Actor.ClassId == 0x0e)
                                                        {
                                                            FKyaBonusBehaviourValues BonusValues;
                                                            Behaviour.bHasTypedBonusBehaviour = KyaReadBonusBehaviour(Behaviour.Id,
                                                                MakeArrayView(Data, int32(Entry.Size)), BehaviourOffset.Payload, BehaviourOffset.Size, BonusValues);
                                                            if (Behaviour.bHasTypedBonusBehaviour)
                                                            {
                                                                Behaviour.BonusField14 = BonusValues.Field14;
                                                                Behaviour.BonusField1B8 = BonusValues.Field1B8;
                                                                Behaviour.BonusField1B4 = BonusValues.Field1B4;
                                                                Behaviour.BonusMaterialId = BonusValues.MaterialId;
                                                                Behaviour.BonusField1B0 = BonusValues.Field1B0;
                                                                Behaviour.BonusFxTailCount = BonusValues.FxTailCount;
                                                                Behaviour.BonusG2DIndex = BonusValues.G2DIndex;
                                                                Behaviour.BonusTurnValue = BonusValues.TurnValue;
                                                                Behaviour.BonusFlockPathIndex = BonusValues.FlockPathIndex;
                                                                Behaviour.BonusAddOnField18 = BonusValues.AddOnField18;
                                                            }
                                                        }
                                                        if (Actor.ClassId == 0x07)
                                                        {
                                                            FKyaMickenBehaviourValues MickenValues;
                                                            Behaviour.bHasTypedMickenBehaviour = KyaReadMickenBehaviour(Behaviour.Id,
                                                                MakeArrayView(Data, int32(Entry.Size)), BehaviourOffset.Payload, BehaviourOffset.Size, MickenValues);
                                                            if (Behaviour.bHasTypedMickenBehaviour)
                                                            {
                                                                Behaviour.MickenEatField8 = MickenValues.Field8;
                                                                Behaviour.MickenEatFieldC = MickenValues.FieldC;
                                                                Behaviour.MickenEatField10 = MickenValues.Field10;
                                                                Behaviour.MickenEatPathIndex = MickenValues.PathIndex;
                                                            }
                                                        }
                                                        if (Actor.ClassId == 0x16)
                                                        {
                                                            if (Behaviour.Id == 4 && BehaviourOffset.Size == 4)
                                                            {
                                                                Behaviour.bHasTypedFruitBehaviour = true;
                                                                Behaviour.FruitSquashedValue = ReadFloat(BehaviourOffset.Payload);
                                                            }
                                                            else if (Behaviour.Id == 6 && BehaviourOffset.Size == 8)
                                                            {
                                                                Behaviour.bHasTypedFruitBehaviour = true;
                                                                Behaviour.FruitBonusActorRef = int32(ReadU32(Data + BehaviourOffset.Payload));
                                                                Behaviour.FruitBonusInitialBehaviour = int32(ReadU32(Data + BehaviourOffset.Payload + 4));
                                                            }
                                                            else if (Behaviour.Id == 5)
                                                            {
                                                                FKyaFruitInventoryValues InventoryValues;
                                                                Behaviour.bHasTypedFruitBehaviour = KyaReadFruitInventory(
                                                                    MakeArrayView(Data, int32(Entry.Size)), BehaviourOffset.Payload, BehaviourOffset.Size, InventoryValues);
                                                                if (Behaviour.bHasTypedFruitBehaviour)
                                                                {
                                                                    Behaviour.FruitInventoryField0 = InventoryValues.Field0;
                                                                    Behaviour.FruitInventoryField8 = InventoryValues.Field8;
                                                                    Behaviour.FruitInventoryField10 = InventoryValues.Field10;
                                                                    Behaviour.FruitInventoryPurchaseId = InventoryValues.PurchaseId;
                                                                    Behaviour.FruitInventoryField28 = InventoryValues.Field28;
                                                                    Behaviour.FruitInventoryMoneyCost = InventoryValues.MoneyCost;
                                                                    Behaviour.FruitInventoryField2C = InventoryValues.Field2C;
                                                                    Behaviour.FruitInventoryHasObject = InventoryValues.bHasObject;
                                                                    Behaviour.FruitInventoryObjectId = InventoryValues.ObjectId;
                                                                    Behaviour.FruitInventoryField18 = InventoryValues.Field18;
                                                                    Behaviour.FruitInventoryInitialBehaviour = InventoryValues.InitialBehaviour;
                                                                }
                                                            }
                                                        }
                                                        if (Actor.ClassId == 0x13 && Behaviour.Id == 2)
                                                        {
                                                            FKyaCommanderBehaviourOffsets Off;
                                                            Behaviour.bHasTypedCommanderBehaviour = KyaReadCommanderBehaviourOffsets(
                                                                MakeArrayView(Data, int32(Entry.Size)), BehaviourOffset.Payload, BehaviourOffset.Payload + BehaviourOffset.Size, Off);
                                                            if (Behaviour.bHasTypedCommanderBehaviour)
                                                            {
                                                                Behaviour.CommanderSquadConfig.Add(int32(ReadU32(Data + Off.SquadConfig)));
                                                                Behaviour.CommanderSquadConfig.Add(int32(ReadU32(Data + Off.SquadConfig + 4)));
                                                                const uint32 SwitchCount = ReadU32(Data + Off.Switch);
                                                                int64 Camera = Off.Switch + 4 + int64(SwitchCount) * 28;
                                                                KyaReadSwitch(MakeArrayView(Data, int32(Entry.Size)), Off.Switch + 4, int32(SwitchCount), Camera, Behaviour.CommanderSwitch);
                                                            }
                                                        }
                                                        if (Actor.ClassId == 0x0a && Behaviour.Id == 6)
                                                        {
                                                            FKyaNativTakePutOffsets Off;
                                                            Behaviour.bHasTypedNativBehaviour = KyaReadNativTakePutOffsets(
                                                                MakeArrayView(Data, int32(Entry.Size)), BehaviourOffset.Payload, BehaviourOffset.Payload + BehaviourOffset.Size, Off);
                                                            if (Behaviour.bHasTypedNativBehaviour)
                                                                for (int64 ItemOffset : Off.Items)
                                                                {
                                                                    FKyaNativTrajectory& Item = Behaviour.NativTrajectories.AddDefaulted_GetRef();
                                                                    Item.Field0 = ReadFloat(ItemOffset);
                                                                    Item.PathFollowIndex = int32(ReadU32(Data + ItemOffset + 4));
                                                                    Item.Field14 = int32(ReadU32(Data + ItemOffset + 8));
                                                                    Item.Field18 = ReadFloat(ItemOffset + 12);
                                                                }
                                                        }
                                                        if (Behaviour.Id == 1 && BehaviourOffset.Size == 4)
                                                        {
                                                            Behaviour.bHasTypedCinematicBehaviour = true;
                                                            Behaviour.CinematicLeaveBehaviourId = int32(ReadU32(Data + BehaviourOffset.Payload));
                                                        }
                                                        if (Actor.ClassId == 0x1b && (Behaviour.Id == 3 || Behaviour.Id == 5))
                                                        {
                                                            FKyaDcaBehaviourOffsets Off;
                                                            if (KyaReadDcaBehaviour(Behaviour.Id,
                                                                MakeArrayView(Data, int32(Entry.Size)), BehaviourOffset.Payload, BehaviourOffset.Size, Off))
                                                            {
                                                                if (Off.bControlled)
                                                                {
                                                                    Behaviour.bHasTypedDcaControlledBehaviour = true;
                                                                    Behaviour.DcaControlledField0 = ReadFloat(Off.Controlled);
                                                                    Behaviour.DcaControlledField4 = ReadFloat(Off.Controlled + 4);
                                                                    Behaviour.DcaControlledField8 = ReadFloat(Off.Controlled + 8);
                                                                    Behaviour.DcaControlledFovDegrees = ReadFloat(Off.Controlled + 12);
                                                                    Behaviour.DcaControlledVector10 = FVector(ReadFloat(Off.Controlled + 16),
                                                                        ReadFloat(Off.Controlled + 20), ReadFloat(Off.Controlled + 24));
                                                                    Behaviour.DcaControlledVector20 = FVector(ReadFloat(Off.Controlled + 28),
                                                                        ReadFloat(Off.Controlled + 32), ReadFloat(Off.Controlled + 36));
                                                                }
if (Off.bTrajectory)
                                                                {
                                                                    Behaviour.bHasTypedDcaTrajectoryBehaviour = true;
                                                                    Behaviour.DcaTrajectoryActorRef = int32(ReadU32(Data + Off.Trajectory));
                                                                    Behaviour.DcaTrajectoryField18 = ReadFloat(Off.Trajectory + 4);
                                                                    Behaviour.DcaTrajectoryField8 = int32(ReadU32(Data + Off.Trajectory + 8));
                                                                }
                                                            }
                                                        }
                                                        if (Actor.ClassId == 0x3d && Behaviour.Id == 2)
                                                        {
                                                            FKyaFogZoneOffsets Off;
                                                            Behaviour.bHasTypedFogBehaviour = KyaReadFogZoneOffsets(
                                                                MakeArrayView(Data, int32(Entry.Size)), BehaviourOffset.Payload, BehaviourOffset.Payload + BehaviourOffset.Size, Off);
                                                            if (Behaviour.bHasTypedFogBehaviour)
                                                                for (int32 Index = 0; Index < Off.CountValue; ++Index)
                                                                {
                                                                    FKyaFogZone& Zone = Behaviour.FogZones.AddDefaulted_GetRef();
                                                                    const int64 Z = Off.Entries + int64(Index) * 28;
                                                                    Zone.Flags = int32(ReadU32(Data + Z));
                                                                    Zone.ZoneRef = int32(ReadU32(Data + Z + 4));
                                                                    Zone.ClipValue = ReadFloat(Z + 8);
                                                                    Zone.Field4 = ReadFloat(Z + 12);
                                                                    Zone.FogRGBA = int32(ReadU32(Data + Z + 16));
                                                                    Zone.FogFlags = int32(ReadU32(Data + Z + 20));
                                                                    Zone.Field18 = ReadFloat(Z + 24);
                                                                }
                                                        }
                                                        if (Actor.ClassId == 0x3e && Behaviour.Id == 3)
                                                        {
                                                            FKyaShootFireOffsets Off;
                                                            Behaviour.bHasTypedShootFireBehaviour = KyaReadShootFireOffsets(
                                                                MakeArrayView(Data, int32(Entry.Size)), BehaviourOffset.Payload, BehaviourOffset.Payload + BehaviourOffset.Size, Off);
                                                            if (Behaviour.bHasTypedShootFireBehaviour)
                                                            {
                                                                Behaviour.ShootFireField2B0 = int32(ReadU32(Data + Off.Field2B0));
                                                                Behaviour.ShootFireField8 = ReadFloat(Off.Field8);
                                                                Behaviour.ShootFireField2B4 = ReadFloat(Off.Field2B4);
                                                                for (int32 Index = 0; Index < Off.CountValue; ++Index)
                                                                    Behaviour.ShootFireActorRefs.Add(int32(ReadU32(Data + Off.Entries + Index * 4)));
                                                            }
                                                        }
                                                        if (Actor.ClassId == 0x3e && Behaviour.Id == 4)
                                                        {
                                                            FKyaShootWaveOffsets Off;
                                                            Behaviour.bHasTypedShootWaveBehaviour = KyaReadShootWaveOffsets(
                                                                MakeArrayView(Data, int32(Entry.Size)), BehaviourOffset.Payload, BehaviourOffset.Size, Off);
                                                            if (Behaviour.bHasTypedShootWaveBehaviour)
                                                                for (int32 Index = 0; Index < 7; ++Index)
                                                                    Behaviour.ShootWaveValues.Add(ReadFloat(Off.Fields + Index * 4));
                                                        }
                                                        if (Actor.ClassId == 0x10 && Behaviour.Id == 3 && BehaviourOffset.Size == 4)
                                                        {
                                                            Behaviour.bHasTypedGravityFallBehaviour = true;
                                                            Behaviour.GravityFallSoundRef = int32(ReadU32(Data + BehaviourOffset.Payload));
                                                        }
                                                        if (Actor.ClassId == 0x10 && Behaviour.Id == 4)
                                                        {
                                                            FKyaGravityBellOffsets Off;
                                                            Behaviour.bHasTypedGravityBellBehaviour = KyaReadGravityBellOffsets(
                                                                MakeArrayView(Data, int32(Entry.Size)), BehaviourOffset.Payload, BehaviourOffset.Payload + BehaviourOffset.Size, Off);
                                                            if (Behaviour.bHasTypedGravityBellBehaviour)
                                                            {
                                                                Behaviour.GravityBellSoundRef = int32(ReadU32(Data + Off.Sound));
                                                                Behaviour.GravityBellFieldC = ReadFloat(Off.FieldC);
                                                                KyaReadSwitch(MakeArrayView(Data, int32(Entry.Size)), Off.Switch.Targets, Off.Switch.Count, Off.Switch.Camera, Behaviour.GravityBellSwitch);
                                                            }
                                                        }
                                                        if (Actor.ClassId == 0x17 && Behaviour.Id == 2)
                                                        {
                                                            FKyaBridgeBasicOffsets Off;
                                                            Behaviour.bHasTypedBridgeBasicBehaviour = KyaReadBridgeBasicOffsets(
                                                                MakeArrayView(Data, int32(Entry.Size)), BehaviourOffset.Payload, BehaviourOffset.Size, Off);
                                                            if (Behaviour.bHasTypedBridgeBasicBehaviour)
                                                            {
                                                                Behaviour.BridgeBasicDirection = FVector(ReadFloat(Off.Direction),
                                                                    ReadFloat(Off.Direction + 4), ReadFloat(Off.Direction + 8));
                                                                Behaviour.BridgeBasicField40 = ReadFloat(Off.Fields40);
                                                                Behaviour.BridgeBasicField44 = ReadFloat(Off.Fields40 + 4);
                                                                Behaviour.BridgeBasicField48 = ReadFloat(Off.Fields40 + 8);
                                                                Behaviour.BridgeBasicField4C = ReadFloat(Off.Fields40 + 12);
                                                                Behaviour.BridgeBasicField50 = ReadFloat(Off.Fields40 + 16);
                                                            }
                                                        }
                                                        if (Actor.ClassId == 0x17 && Behaviour.Id == 3)
                                                        {
                                                            FKyaBridgeWindOffsets Off;
                                                            Behaviour.bHasTypedBridgeWindBehaviour = KyaReadBridgeWindOffsets(
                                                                MakeArrayView(Data, int32(Entry.Size)), BehaviourOffset.Payload, BehaviourOffset.Size, Off);
                                                            if (Behaviour.bHasTypedBridgeWindBehaviour)
                                                                for (int32 Index = 0; Index < 5; ++Index)
                                                                    Behaviour.BridgeWindValues.Add(ReadFloat(Off.Fields40 + Index * 4));
                                                        }
                                                        if (Actor.ClassId == 0x09 && (Behaviour.Id == 8 || Behaviour.Id == 10 || Behaviour.Id == 13 || Behaviour.Id == 14 || Behaviour.Id == 15 || Behaviour.Id == 17 || Behaviour.Id == 18 || Behaviour.Id == 19 || Behaviour.Id == 21 || Behaviour.Id == 22 || Behaviour.Id == 23 || Behaviour.Id == 24))
                                                        {
                                                            FKyaWolfenBehaviourOffsets Off;
                                                            Behaviour.bHasTypedWolfenBehaviour = KyaReadWolfenBehaviourOffsets(
                                                                Behaviour.Id, MakeArrayView(Data, int32(Entry.Size)),
                                                                BehaviourOffset.Payload, BehaviourOffset.Size, Off);
                                                            if (Behaviour.bHasTypedWolfenBehaviour)
                                                            {
                                                                FKyaWolfenBehaviour& WolfenValues = Behaviour.WolfenBehaviour;
                                                                if (!Off.bExorcism) WolfenValues.Flags = int32(ReadU32(Data + Off.Flags));
                                                                if (Off.bTrack) WolfenValues.TrackField = ReadFloat(Off.TrackField);
                                                                if (Off.bDca)
                                                                {
                                                                    WolfenValues.DcaActorRef = int32(ReadU32(Data + Off.DcaActorRef));
                                                                    WolfenValues.DcaShootCooldown = ReadFloat(Off.DcaShootCooldown);
                                                                }
                                                                if (Off.bAvoid)
                                                                {
                                                                    WolfenValues.AvoidField80 = ReadFloat(Off.Avoid80);
                                                                    WolfenValues.AvoidField84 = ReadFloat(Off.Avoid84);
                                                                }
                                                                auto PopulateSwitchScript = [&](const FKyaSwitchScriptOffsets& Script) {
                                                                    for (int32 RecordIndex = 0; RecordIndex < Script.RecordOffsets.Num(); ++RecordIndex)
                                                                    {
                                                                        FKyaSwitchBehaviourRecord& Record = WolfenValues.SwitchRecords.AddDefaulted_GetRef();
                                                                        const int32 Category = Script.RecordCategories[RecordIndex];
                                                                        const int64 P = Script.RecordOffsets[RecordIndex];
                                                                        Record.Category = Category;
                                                                        if (Category == 0)
                                                                        {
                                                                            Record.MessageType = int32(ReadU32(Data + P));
                                                                            Record.BehaviourId = int32(ReadU32(Data + P + 4));
                                                                        }
                                                                        else if (Category == 3)
                                                                        {
                                                                            Record.SeeActorRef = int32(ReadU32(Data + P));
                                                                            Record.BehaviourId = int32(ReadU32(Data + P + 4));
                                                                        }
                                                                        else
                                                                        {
                                                                            Record.ActorRef = int32(ReadU32(Data + P));
                                                                            Record.ZoneRef = int32(ReadU32(Data + P + 4));
                                                                            Record.Flags = int32(ReadU32(Data + P + 8));
                                                                            Record.BehaviourId = int32(ReadU32(Data + P + 12));
                                                                        }
                                                                    }
                                                                };
                                                                if (Off.bLost) PopulateSwitchScript(Off.Switch);
                                                                if (Off.bWatchDog)
                                                                {
                                                                    WolfenValues.Field80 = int32(ReadU32(Data + Off.Field80));
                                                                    WolfenValues.TrackBehaviourId = int32(ReadU32(Data + Off.TrackBehaviourId));
                                                                    PopulateSwitchScript(Off.Switch);
                                                                    for (int32 Index = 0; Index < Off.OnOffCountValue; ++Index)
                                                                    {
                                                                        FKyaSwitchTarget& Target = WolfenValues.OnOffTargets.AddDefaulted_GetRef();
                                                                        const int64 T = Off.OnOffEntries + int64(Index) * 28;
                                                                        Target.TargetRef = int32(ReadU32(Data + T));
                                                                        Target.CutsceneId = int32(ReadU32(Data + T + 4));
                                                                        Target.Flags = int32(ReadU32(Data + T + 8));
                                                                        Target.MessageId = int32(ReadU32(Data + T + 12));
                                                                        Target.MessageFlags = int32(ReadU32(Data + T + 16));
                                                                        Target.Field14 = int32(ReadU32(Data + T + 20));
                                                                        Target.Field18 = int32(ReadU32(Data + T + 24));
                                                                    }
                                                                    const int64 C = Off.OnOffCamera;
                                                                    WolfenValues.OnOffCamera.CameraIndex = int32(ReadU32(Data + C));
                                                                    WolfenValues.OnOffCamera.Flags = int32(ReadU32(Data + C + 4));
                                                                    WolfenValues.OnOffCamera.Field8 = ReadFloat(C + 8);
                                                                    WolfenValues.OnOffCamera.FieldC = ReadFloat(C + 12);
                                                                    WolfenValues.OnOffCamera.Field10 = ReadFloat(C + 16);
                                                                    WolfenValues.OnOffCamera.ActorRef = int32(ReadU32(Data + C + 20));
                                                                    WolfenValues.OnOffCamera.ZoneRef = int32(ReadU32(Data + C + 24));
                                                                    WolfenValues.OnOffCamera.Field1C = ReadFloat(C + 28);
                                                                }
                                                                if (Off.bExorcism)
                                                                {
                                                                    WolfenValues.ExorcismDigitMaterialId = int32(ReadU32(Data + Off.ExorcismMaterial));
                                                                    WolfenValues.ExorcismField24 = int32(ReadU32(Data + Off.ExorcismField24));
                                                                    WolfenValues.ExorcismTextureA = int32(ReadU32(Data + Off.ExorcismMeshes));
                                                                    WolfenValues.ExorcismMeshA = int32(ReadU32(Data + Off.ExorcismMeshes + 4));
                                                                    WolfenValues.ExorcismTextureB = int32(ReadU32(Data + Off.ExorcismMeshes + 8));
                                                                    WolfenValues.ExorcismMeshB = int32(ReadU32(Data + Off.ExorcismMeshes + 12));
                                                                    WolfenValues.ExorcismField18 = int32(ReadU32(Data + Off.ExorcismField18));
                                                                    for (int32 WordIndex = 0; WordIndex < 5; ++WordIndex)
                                                                        WolfenValues.ExorcismWords.Add(int32(ReadU32(Data + Off.ExorcismWords + WordIndex * 4)));
                                                                }
                                                                if (Off.bGuard)
                                                                {
                                                                    WolfenValues.GuardPathIndex = int32(ReadU32(Data + Off.GuardPath));
                                                                    WolfenValues.GuardField98 = ReadFloat(Off.GuardField98);
                                                                    WolfenValues.GuardTrackBehaviourId = int32(ReadU32(Data + Off.GuardTrack));
                                                                    WolfenValues.GuardField90 = int32(ReadU32(Data + Off.GuardField90));
                                                                    PopulateSwitchScript(Off.Switch);
                                                                    for (int32 Index = 0; Index < Off.OnOffCountValue; ++Index)
                                                                    {
                                                                        FKyaSwitchTarget& Target = WolfenValues.OnOffTargets.AddDefaulted_GetRef();
                                                                        const int64 T = Off.OnOffEntries + int64(Index) * 28;
                                                                        Target.TargetRef = int32(ReadU32(Data + T));
                                                                        Target.CutsceneId = int32(ReadU32(Data + T + 4));
                                                                        Target.Flags = int32(ReadU32(Data + T + 8));
                                                                        Target.MessageId = int32(ReadU32(Data + T + 12));
                                                                        Target.MessageFlags = int32(ReadU32(Data + T + 16));
                                                                        Target.Field14 = int32(ReadU32(Data + T + 20));
                                                                        Target.Field18 = int32(ReadU32(Data + T + 24));
                                                                    }
                                                                    const int64 C = Off.OnOffCamera;
                                                                    WolfenValues.OnOffCamera.CameraIndex = int32(ReadU32(Data + C));
                                                                    WolfenValues.OnOffCamera.Flags = int32(ReadU32(Data + C + 4));
                                                                    WolfenValues.OnOffCamera.Field8 = ReadFloat(C + 8);
                                                                    WolfenValues.OnOffCamera.FieldC = ReadFloat(C + 12);
                                                                    WolfenValues.OnOffCamera.Field10 = ReadFloat(C + 16);
                                                                    WolfenValues.OnOffCamera.ActorRef = int32(ReadU32(Data + C + 20));
                                                                    WolfenValues.OnOffCamera.ZoneRef = int32(ReadU32(Data + C + 24));
                                                                    WolfenValues.OnOffCamera.Field1C = ReadFloat(C + 28);
                                                                }
                                                                if (Off.bEscape)
                                                                {
                                                                    for (int32 Index = 0; Index < Off.EscapeCountValue; ++Index)
                                                                        WolfenValues.EscapePathIndices.Add(int32(ReadU32(Data + Off.EscapeEntries + Index * 4)));
                                                                    WolfenValues.EscapeField88 = int32(ReadU32(Data + Off.EscapeField88));
                                                                    WolfenValues.EscapeField9C = ReadFloat(Off.EscapeFloats);
                                                                    WolfenValues.EscapeField90 = ReadFloat(Off.EscapeFloats + 4);
                                                                    WolfenValues.EscapeField8C = ReadFloat(Off.EscapeFloats + 8);
                                                                    WolfenValues.EscapeField94 = ReadFloat(Off.EscapeFloats + 12);
                                                                    WolfenValues.EscapeField98 = ReadFloat(Off.EscapeFloats + 16);
                                                                    WolfenValues.EscapeFieldA0 = int32(ReadU32(Data + Off.EscapeFieldA0));
                                                                    PopulateSwitchScript(Off.Switch);
                                                                }
                                                                if (Off.bSnipe)
                                                                {
                                                                    WolfenValues.SnipeField90 = int32(ReadU32(Data + Off.SnipeField90));
                                                                    WolfenValues.SnipeField94 = ReadFloat(Off.SnipeField94);
                                                                    WolfenValues.SnipeField98 = ReadFloat(Off.SnipeField94 + 4);
                                                                    WolfenValues.SnipeField9C = ReadFloat(Off.SnipeField94 + 8);
                                                                    WolfenValues.SnipeFieldA0 = ReadFloat(Off.SnipeField94 + 12);
                                                                    WolfenValues.SpotTrackBehaviourId = int32(ReadU32(Data + Off.SpotTrack));
                                                                    WolfenValues.SnipeTrackBehaviourId = int32(ReadU32(Data + Off.SnipeTrack));
                                                                    WolfenValues.ShadowMaterialId = int32(ReadU32(Data + Off.ShadowMaterial));
                                                                    WolfenValues.SnipeFieldAC = int32(ReadU32(Data + Off.SnipeFieldAC));
                                                                    WolfenValues.SnipeFieldB0 = ReadFloat(Off.SnipeFieldB0);
                                                                    WolfenValues.SnipeFieldB4 = ReadFloat(Off.SnipeFieldB4);
                                                                    WolfenValues.NbSpot = int32(ReadU32(Data + Off.NbSpot));
                                                                    WolfenValues.NbShadow = int32(ReadU32(Data + Off.NbShadow));
                                                                    WolfenValues.RotationSpeed = ReadFloat(Off.RotationSpeed);
                                                                    WolfenValues.SnipeFieldB8 = int32(ReadU32(Data + Off.SnipeFieldB8));
                                                                    PopulateSwitchScript(Off.Switch);
                                                                }
                                                                if (Off.bWeaponStand || Off.bWeapon || Off.bWeaponSnipe)
                                                                {
                                                                    WolfenValues.WeaponField90 = ReadFloat(Off.WeaponField90);
                                                                    WolfenValues.WeaponField94 = int32(ReadU32(Data + Off.WeaponField94));
                                                                    WolfenValues.WeaponField98 = int32(ReadU32(Data + Off.WeaponField98));
                                                                    for (const FKyaTrackSubObjOffsets& SubObjOffsets : Off.SubObjs)
                                                                    {
                                                                        FKyaTrackSubObj& SubObj = WolfenValues.WeaponSubObjs.AddDefaulted_GetRef();
                                                                        SubObj.Field0 = int32(ReadU32(Data + SubObjOffsets.Field0));
                                                                        SubObj.AField0 = int32(ReadU32(Data + SubObjOffsets.AField0));
                                                                        SubObj.AField4 = ReadFloat(SubObjOffsets.AField4);
                                                                        SubObj.AField8 = ReadFloat(SubObjOffsets.AField8);
                                                                        SubObj.Field10 = int32(ReadU32(Data + SubObjOffsets.Field10));
                                                                    }
                                                                    if (Off.bWeapon || Off.bWeaponSnipe)
                                                                        WolfenValues.WeaponFieldF0 = ReadFloat(Off.WeaponFieldF0);
                                                                    if (Off.bWeapon) WolfenValues.WeaponTargetField = ReadFloat(Off.WeaponTargetField);
                                                                }
                                                            }
                                                        }
                                                        if (Actor.ClassId == 0x1c && (Behaviour.Id == 3 || Behaviour.Id == 4 || Behaviour.Id == 5 || Behaviour.Id == 11))
                                                        {
                                                            FKyaProjectileBehaviourOffsets Off;
                                                            Behaviour.bHasTypedProjectileBehaviour = KyaReadProjectileBehaviourOffsets(
                                                                Behaviour.Id, MakeArrayView(Data, int32(Entry.Size)),
                                                                BehaviourOffset.Payload, BehaviourOffset.Size, Off);
                                                            if (Behaviour.bHasTypedProjectileBehaviour)
                                                            {
                                                                FKyaProjectileBehaviour& ProjectileValues = Behaviour.ProjectileBehaviour;
                                                                ProjectileValues.BehaviourId = int32(ReadU32(Data + Off.BehaviourId));
                                                                ProjectileValues.Field0C = int32(ReadU32(Data + Off.Field0C));
                                                                if (Off.bStand) ProjectileValues.MaterialId = int32(ReadU32(Data + Off.MaterialId));
                                                                if (Off.bStraight)
                                                                {
                                                                    ProjectileValues.Speed = ReadFloat(Off.Speed);
                                                                    ProjectileValues.Acceleration = ReadFloat(Off.Acceleration);
                                                                    ProjectileValues.Field18 = ReadFloat(Off.Field18);
                                                                }
                                                                if (Off.bPortable) ProjectileValues.Field10 = ReadFloat(Off.Field10);
                                                                if (Off.bExcuse) ProjectileValues.ActorRef = int32(ReadU32(Data + Off.ActorRef));
                                                            }
                                                        }
                                                        if (Actor.ClassId == 0x11 && (Behaviour.Id == 2 || Behaviour.Id == 3 || Behaviour.Id == 5 || Behaviour.Id == 6))
                                                        {
                                                            FKyaSwitchBehaviourOffsets Off;
                                                            Behaviour.bHasTypedSwitchBehaviour = KyaReadSwitchBehaviourOffsets(
                                                                Behaviour.Id, MakeArrayView(Data, int32(Entry.Size)),
                                                                BehaviourOffset.Payload, BehaviourOffset.Size, Off);
                                                            if (Behaviour.bHasTypedSwitchBehaviour)
                                                            {
                                                                FKyaSwitchBehaviour& SwitchValues = Behaviour.SwitchBehaviour;
                                                                if (Off.bLever)
                                                                {
                                                                    SwitchValues.LeverRefA = int32(ReadU32(Data + Off.LeverRefA));
                                                                    SwitchValues.LeverRefB = int32(ReadU32(Data + Off.LeverRefB));
                                                                }
                                                                if (Off.bMagic)
                                                                {
                                                                    SwitchValues.MagicField08 = ReadFloat(Off.MagicField08);
                                                                    SwitchValues.MagicField10 = ReadFloat(Off.MagicField10);
                                                                    SwitchValues.MagicFxDigits = int32(ReadU32(Data + Off.MagicFxDigits));
                                                                    SwitchValues.MagicField14 = ReadFloat(Off.MagicField14);
                                                                    SwitchValues.MagicField4C = int32(ReadU32(Data + Off.MagicField4C));
                                                                    SwitchValues.MagicField18 = int32(ReadU32(Data + Off.MagicField18));
                                                                    SwitchValues.MagicField28 = int32(ReadU32(Data + Off.MagicField28));
                                                                    SwitchValues.MagicField24 = int32(ReadU32(Data + Off.MagicField24));
                                                                    SwitchValues.MagicField30 = int32(ReadU32(Data + Off.MagicField30));
                                                                    SwitchValues.OpenTextureId = int32(ReadU32(Data + Off.OpenTextureId));
                                                                    SwitchValues.OpenMeshId = int32(ReadU32(Data + Off.OpenMeshId));
                                                                    const int64 C = Off.MagicCamera;
                                                                    SwitchValues.MagicCamera.CameraIndex = int32(ReadU32(Data + C));
                                                                    SwitchValues.MagicCamera.Flags = int32(ReadU32(Data + C + 4));
                                                                    SwitchValues.MagicCamera.Field8 = ReadFloat(C + 8);
                                                                    SwitchValues.MagicCamera.FieldC = ReadFloat(C + 12);
                                                                    SwitchValues.MagicCamera.Field10 = ReadFloat(C + 16);
                                                                    SwitchValues.MagicCamera.ActorRef = int32(ReadU32(Data + C + 20));
                                                                    SwitchValues.MagicCamera.ZoneRef = int32(ReadU32(Data + C + 24));
                                                                    SwitchValues.MagicCamera.Field1C = ReadFloat(C + 28);
                                                                }
                                                                if (Off.bMultiCondition)
                                                                {
                                                                    SwitchValues.MultiField14 = int32(ReadU32(Data + Off.MultiField14));
                                                                    SwitchValues.MultiField08 = int32(ReadU32(Data + Off.MultiField08));
                                                                    SwitchValues.MultiField0C = int32(ReadU32(Data + Off.MultiField0C));
                                                                }
                                                                if (Off.bTarget)
                                                                {
                                                                    SwitchValues.TargetDisplacement = ReadFloat(Off.TargetDisplacement);
                                                                    SwitchValues.TargetSpringStrength = ReadFloat(Off.TargetSpringStrength);
                                                                    SwitchValues.TargetDamping = ReadFloat(Off.TargetDamping);
                                                                }
                                                            }
                                                        }
                                                        if ((Actor.ClassId == 0x07 || Actor.ClassId == 0x0a) && Behaviour.Id == 2)
                                                        {
                                                            FKyaCatchByTrapOffsets Off;
                                                            Behaviour.bHasTypedCatchByTrapBehaviour = KyaReadCatchByTrapOffsets(
                                                                MakeArrayView(Data, int32(Entry.Size)), BehaviourOffset.Payload, BehaviourOffset.Size, Off);
                                                            if (Behaviour.bHasTypedCatchByTrapBehaviour)
                                                                for (const FKyaConditionedOffsets& BlockOffsets : Off.Blocks)
                                                                {
                                                                    FKyaConditionedOperation& Result = Behaviour.CatchOperations.AddDefaulted_GetRef();
                                                                    for (int64 Offset : BlockOffsets.Operations)
                                                                    {
                                                                        FKyaScenarioOperation& Op = Result.Operations.AddDefaulted_GetRef();
                                                                        Op.Variable = int32(ReadU32(Data + Offset));
                                                                        Op.Type = int32(ReadU32(Data + Offset + 4));
                                                                        Op.Value = int32(ReadU32(Data + Offset + 8));
                                                                    }
                                                                    for (int64 Offset : BlockOffsets.Conditions)
                                                                    {
                                                                        FKyaScenarioCondition& Condition = Result.Conditions.AddDefaulted_GetRef();
                                                                        Condition.Combiner = int32(ReadU32(Data + Offset));
                                                                        Condition.Variable = int32(ReadU32(Data + Offset + 4));
                                                                        Condition.Comparison = int32(ReadU32(Data + Offset + 8));
                                                                        Condition.Value = int32(ReadU32(Data + Offset + 12));
                                                                    }
                                                                }
                                                        }
                                                        if (Actor.ClassId == 0x06 && (Behaviour.Id == 9 || Behaviour.Id == 10 || Behaviour.Id == 11))
                                                        {
                                                            FKyaHeroUnknownOffsets Off;
                                                            Behaviour.bHasTypedHeroUnknownBehaviour = KyaReadHeroUnknownOffsets(
                                                                MakeArrayView(Data, int32(Entry.Size)), BehaviourOffset.Payload, BehaviourOffset.Size, Off);
                                                            if (Behaviour.bHasTypedHeroUnknownBehaviour)
                                                            {
                                                                FKyaHeroUnknownBehaviour& HeroValues = Behaviour.HeroUnknownBehaviour;
                                                                HeroValues.FxTypeA = int32(ReadU32(Data + Off.Values));
                                                                HeroValues.TextureA = int32(ReadU32(Data + Off.Values + 4));
                                                                HeroValues.MeshA = int32(ReadU32(Data + Off.Values + 8));
                                                                HeroValues.FxTypeB = int32(ReadU32(Data + Off.Values + 12));
                                                                HeroValues.FxTypeC = int32(ReadU32(Data + Off.Values + 16));
                                                                HeroValues.TextureB = int32(ReadU32(Data + Off.Values + 20));
                                                                HeroValues.MeshB = int32(ReadU32(Data + Off.Values + 24));
                                                            }
                                                        }
                                                        if (Actor.ClassId == 0x1c && Behaviour.Id == 9)
                                                        {
                                                            FKyaInventoryOffsets Off;
                                                            Behaviour.bHasTypedInventoryBehaviour = KyaReadInventoryOffsets(
                                                                MakeArrayView(Data, int32(Entry.Size)), BehaviourOffset.Payload, BehaviourOffset.Size, Off);
                                                            if (Behaviour.bHasTypedInventoryBehaviour)
                                                            {
                                                                FKyaInventoryInfo& InventoryValues = Behaviour.InventoryInfo;
                                                                InventoryValues.Field0 = int32(ReadU32(Data + Off.Field0));
                                                                if (Off.Field8 != 0)
                                                                {
                                                                    InventoryValues.Field8 = int64(uint64(ReadU32(Data + Off.Field8)) |
                                                                        (uint64(ReadU32(Data + Off.Field8 + 4)) << 32));
                                                                    InventoryValues.Field10 = int64(uint64(ReadU32(Data + Off.Field10)) |
                                                                        (uint64(ReadU32(Data + Off.Field10 + 4)) << 32));
                                                                    InventoryValues.PurchaseId = int32(ReadU32(Data + Off.PurchaseId));
                                                                    InventoryValues.Field28 = int32(ReadU32(Data + Off.Field28));
                                                                    InventoryValues.MoneyCost = int32(ReadU32(Data + Off.MoneyCost));
                                                                    InventoryValues.Field2C = int32(ReadU32(Data + Off.Field2C));
                                                                }
                                                                if (Off.bHasObject)
                                                                {
                                                                    InventoryValues.ObjectId = int32(ReadU32(Data + Off.ObjectId));
                                                                    InventoryValues.Field18 = int64(uint64(ReadU32(Data + Off.Field18)) |
                                                                        (uint64(ReadU32(Data + Off.Field18 + 4)) << 32));
                                                                }
                                                                else if (Off.Field8 != 0) InventoryValues.ObjectId = 0x20;
                                                                InventoryValues.InitialBehaviourId = int32(ReadU32(Data + Off.InitialBehaviourId));
                                                            }
                                                        }
                                                        if (Actor.ClassId == 0x0d && (Behaviour.Id == 2 || Behaviour.Id == 3 || Behaviour.Id == 4 || Behaviour.Id == 5 || Behaviour.Id == 6 || Behaviour.Id == 7 || Behaviour.Id == 8 || Behaviour.Id == 9))
                                                        {
                                                            FKyaPlatformBehaviourOffsets Off;
                                                            Behaviour.bHasTypedPlatformBehaviour = KyaReadPlatformBehaviourOffsets(
                                                                Behaviour.Id, MakeArrayView(Data, int32(Entry.Size)),
                                                                BehaviourOffset.Payload, BehaviourOffset.Size, Off);
                                                            if (Behaviour.bHasTypedPlatformBehaviour)
                                                            {
                                                                FKyaPlatformBehaviour& PlatformValues = Behaviour.PlatformBehaviour;
                                                                if (Off.bStand)
                                                                {
                                                                    PlatformValues.SoundRef = int32(ReadU32(Data + Off.SoundRef));
                                                                    PlatformValues.Field0C = int32(ReadU32(Data + Off.Field0C));
                                                                }
                                                                if (Off.bSlab)
                                                                {
                                                                    KyaReadSwitch(MakeArrayView(Data, int32(Entry.Size)), Off.SlabSwitch.Targets,
                                                                        Off.SlabSwitch.Count, Off.SlabSwitch.Camera, PlatformValues.SlabSwitch);
                                                                    PlatformValues.SlabField10 = ReadFloat(Off.SlabFloats);
                                                                    PlatformValues.SlabField14 = ReadFloat(Off.SlabFloats + 4);
                                                                    PlatformValues.SlabField18 = ReadFloat(Off.SlabFloats + 8);
                                                                    PlatformValues.SlabField1C = ReadFloat(Off.SlabFloats + 12);
                                                                    PlatformValues.SlabField20 = int32(ReadU32(Data + Off.SlabField20));
                                                                }
                                                                if (Off.bMaster)
                                                                {
                                                                    PlatformValues.MasterPathIndex = int32(ReadU32(Data + Off.MasterPath));
                                                                    PlatformValues.MasterTiming = int32(ReadU32(Data + Off.MasterPath + 4));
                                                                    PlatformValues.MasterStreamActor = int32(ReadU32(Data + Off.MasterStream));
                                                                    PlatformValues.MasterField40 = ReadFloat(Off.MasterFloats);
                                                                    PlatformValues.MasterField44 = ReadFloat(Off.MasterFloats + 4);
                                                                    for (int32 Index = 0; Index < Off.AnalogCountValue; ++Index)
                                                                    {
                                                                        FKyaAnalogTarget& Target = PlatformValues.AnalogTargets.AddDefaulted_GetRef();
                                                                        const int64 T = Off.AnalogEntries + int64(Index) * 40;
                                                                        Target.ActorRef = int32(ReadU32(Data + T));
                                                                        Target.CutsceneId = int32(ReadU32(Data + T + 4));
                                                                        Target.Flags = int32(ReadU32(Data + T + 8));
                                                                        Target.Field0C = int32(ReadU32(Data + T + 12));
                                                                        Target.Field10 = int32(ReadU32(Data + T + 16));
                                                                        Target.Field14 = int32(ReadU32(Data + T + 20));
                                                                        Target.Field18 = int32(ReadU32(Data + T + 24));
                                                                        Target.Field1C = int32(ReadU32(Data + T + 28));
                                                                        Target.Field20 = ReadFloat(T + 32);
                                                                        Target.Field24 = ReadFloat(T + 36);
                                                                    }
                                                                    const int64 C = Off.AnalogCamera;
                                                                    PlatformValues.AnalogCamera.CameraIndex = int32(ReadU32(Data + C));
                                                                    PlatformValues.AnalogCamera.Flags = int32(ReadU32(Data + C + 4));
                                                                    PlatformValues.AnalogCamera.Field8 = ReadFloat(C + 8);
                                                                    PlatformValues.AnalogCamera.FieldC = ReadFloat(C + 12);
                                                                    PlatformValues.AnalogCamera.Field10 = ReadFloat(C + 16);
                                                                    PlatformValues.AnalogCamera.ActorRef = int32(ReadU32(Data + C + 20));
                                                                    PlatformValues.AnalogCamera.ZoneRef = int32(ReadU32(Data + C + 24));
                                                                    PlatformValues.AnalogCamera.Field1C = ReadFloat(C + 28);
                                                                }
                                                                if (Off.bSelectorMaster)
                                                                {
                                                                    PlatformValues.Field18 = int32(ReadU32(Data + Off.Field18));
                                                                    PlatformValues.MaterialId = int32(ReadU32(Data + Off.MaterialId));
                                                                    PlatformValues.Field8 = ReadFloat(Off.Field8);
                                                                    PlatformValues.FieldC = ReadFloat(Off.FieldC);
                                                                    PlatformValues.Field10 = ReadFloat(Off.Field10);
                                                                    PlatformValues.SoundRef = int32(ReadU32(Data + Off.SoundRef));
                                                                    for (int32 Index = 0; Index < Off.SelectorActorCountValue; ++Index)
                                                                        PlatformValues.SelectorActorRefs.Add(int32(ReadU32(Data + Off.SelectorActorEntries + Index * 4)));
                                                                    for (int32 Index = 0; Index < Off.SelectorSwitchCountValue; ++Index)
                                                                    {
                                                                        FKyaSwitchExTarget& Target = PlatformValues.SelectorSwitchEx.AddDefaulted_GetRef();
                                                                        const int64 T = Off.SelectorSwitchEntries + int64(Index) * 16;
                                                                        Target.ActorRef = int32(ReadU32(Data + T));
                                                                        Target.CutsceneId = int32(ReadU32(Data + T + 4));
                                                                        Target.Flags = int32(ReadU32(Data + T + 8));
                                                                        Target.MessageId = int32(ReadU32(Data + T + 12));
                                                                    }
                                                                    const int64 C = Off.SelectorCamera;
                                                                    PlatformValues.SelectorCamera.CameraIndex = int32(ReadU32(Data + C));
                                                                    PlatformValues.SelectorCamera.Flags = int32(ReadU32(Data + C + 4));
                                                                    PlatformValues.SelectorCamera.Field8 = ReadFloat(C + 8);
                                                                    PlatformValues.SelectorCamera.FieldC = ReadFloat(C + 12);
                                                                    PlatformValues.SelectorCamera.Field10 = ReadFloat(C + 16);
                                                                    PlatformValues.SelectorCamera.ActorRef = int32(ReadU32(Data + C + 20));
                                                                    PlatformValues.SelectorCamera.ZoneRef = int32(ReadU32(Data + C + 24));
                                                                    PlatformValues.SelectorCamera.Field1C = ReadFloat(C + 28);
                                                                }
                                                                if (Off.bTrajectory)
                                                                {
                                                                    PlatformValues.PathIndex = int32(ReadU32(Data + Off.PathIndex));
                                                                    PlatformValues.Timing = int32(ReadU32(Data + Off.Timing));
                                                                    PlatformValues.PathField4 = ReadFloat(Off.PathField4);
                                                                    PlatformValues.PathTotalTime = ReadFloat(Off.PathTotalTime);
                                                                    PlatformValues.SegmentStartTime = ReadFloat(Off.SegmentStartTime);
                                                                    PlatformValues.LoopDelay = ReadFloat(Off.LoopDelay);
                                                                }
                                                                if (Off.bSlave)
                                                                {
                                                                    PlatformValues.PathIndex = int32(ReadU32(Data + Off.PathIndex));
                                                                    PlatformValues.Timing = int32(ReadU32(Data + Off.Timing));
                                                                }
                                                                if (Off.bDestroyed)
                                                                {
                                                                    PlatformValues.PathIndex = int32(ReadU32(Data + Off.PathIndex));
                                                                    PlatformValues.Timing = int32(ReadU32(Data + Off.Timing));
                                                                    PlatformValues.PathField4 = ReadFloat(Off.PathField4);
                                                                    PlatformValues.PathTotalTime = ReadFloat(Off.PathTotalTime);
                                                                    PlatformValues.Field40 = int32(ReadU32(Data + Off.Field40));
                                                                    PlatformValues.AltModelId = int32(ReadU32(Data + Off.AltModelId));
                                                                }
                                                                if (Off.bSelectorSlave)
                                                                {
                                                                    PlatformValues.Field18 = int32(ReadU32(Data + Off.Field18));
                                                                    PlatformValues.MaterialId = int32(ReadU32(Data + Off.MaterialId));
                                                                    PlatformValues.Field8 = ReadFloat(Off.Field8);
                                                                    PlatformValues.FieldC = ReadFloat(Off.FieldC);
                                                                    PlatformValues.Field10 = ReadFloat(Off.Field10);
                                                                    PlatformValues.SoundRef = int32(ReadU32(Data + Off.SoundRef));
                                                                }
                                                            }
                                                        }
                                                        if (Actor.ClassId == 0x19 && Behaviour.Id == 3)
                                                        {
                                                            FKyaCompanionBehaviourOffsets Off;
                                                            Behaviour.bHasTypedCompanionBehaviour = KyaReadCompanionBehaviourOffsets(
                                                                MakeArrayView(Data, int32(Entry.Size)), BehaviourOffset.Payload,
                                                                BehaviourOffset.Size, Off);
                                                            if (Behaviour.bHasTypedCompanionBehaviour)
                                                                for (const FKyaCompanionAlertOffsets& AlertOffsets : Off.Alerts)
                                                                {
                                                                    FKyaCompanionAlert& Alert = Behaviour.CompanionAlerts.AddDefaulted_GetRef();
                                                                    for (int32 Index = 0; Index < AlertOffsets.Conditions.CountValue; ++Index)
                                                                    {
                                                                        FKyaScenarioCondition& Condition = Alert.Conditions.AddDefaulted_GetRef();
                                                                        const int64 C = AlertOffsets.Conditions.Entries + int64(Index) * 16;
                                                                        Condition.Combiner = int32(ReadU32(Data + C));
                                                                        Condition.Variable = int32(ReadU32(Data + C + 4));
                                                                        Condition.Comparison = int32(ReadU32(Data + C + 8));
                                                                        Condition.Value = int32(ReadU32(Data + C + 12));
                                                                    }
                                                                    Alert.Flags = int32(ReadU32(Data + AlertOffsets.Flags));
                                                                    Alert.Field08 = int32(ReadU32(Data + AlertOffsets.Field08));
                                                                    Alert.Field10 = int32(ReadU32(Data + AlertOffsets.Field10));
                                                                    Alert.Field14 = int32(ReadU32(Data + AlertOffsets.Field14));
                                                                    Alert.Field18 = ReadFloat(AlertOffsets.Field18);
                                                                    Alert.ActorRef = int32(ReadU32(Data + AlertOffsets.ActorRef));
                                                                    Alert.Field20 = FVector(ReadFloat(AlertOffsets.Vector20),
                                                                        ReadFloat(AlertOffsets.Vector20 + 4), ReadFloat(AlertOffsets.Vector20 + 8));
                                                                    Alert.CameraRef = int32(ReadU32(Data + AlertOffsets.CameraRef));
                                                                    for (int32 Index = 0; Index < AlertOffsets.ListCountValue; ++Index)
                                                                        Alert.Field34.Add(int32(ReadU32(Data + AlertOffsets.ListEntries + Index * 4)));
                                                                    Alert.LevelStringKey = int64(uint64(ReadU32(Data + AlertOffsets.LevelStringKey)) |
                                                                        (uint64(ReadU32(Data + AlertOffsets.LevelStringKey + 4)) << 32));
                                                                    Alert.Field50 = ReadFloat(AlertOffsets.Field50);
                                                                    Alert.Field58 = int32(ReadU32(Data + AlertOffsets.Field58));
                                                                    Alert.Field5C = ReadFloat(AlertOffsets.Field5C);
                                                                    KyaReadSwitch(MakeArrayView(Data, int32(Entry.Size)), AlertOffsets.Switch.Targets,
                                                                        AlertOffsets.Switch.Count, AlertOffsets.Switch.Camera, Alert.TargetSwitch);
                                                                    for (const FKyaConditionedOffsets& Block : AlertOffsets.Blocks)
                                                                    {
                                                                        FKyaConditionedOperation& Result = Alert.Field70.AddDefaulted_GetRef();
                                                                        for (int64 Offset : Block.Operations)
                                                                        {
                                                                            FKyaScenarioOperation& Op = Result.Operations.AddDefaulted_GetRef();
                                                                            Op.Variable = int32(ReadU32(Data + Offset));
                                                                            Op.Type = int32(ReadU32(Data + Offset + 4));
                                                                            Op.Value = int32(ReadU32(Data + Offset + 8));
                                                                        }
                                                                        for (int64 Offset : Block.Conditions)
                                                                        {
                                                                            FKyaScenarioCondition& Condition = Result.Conditions.AddDefaulted_GetRef();
                                                                            Condition.Combiner = int32(ReadU32(Data + Offset));
                                                                            Condition.Variable = int32(ReadU32(Data + Offset + 4));
                                                                            Condition.Comparison = int32(ReadU32(Data + Offset + 8));
                                                                            Condition.Value = int32(ReadU32(Data + Offset + 12));
                                                                        }
                                                                    }
                                                                }
                                                        }
                                                        if (Actor.ClassId == 0x18 && Behaviour.Id == 2)
                                                        {
                                                            TArray<FKyaClusterZoneOffsets> Zones;
                                                            Behaviour.bHasTypedClusterZones = KyaReadClusterZoneOffsets(MakeArrayView(Data, int32(Entry.Size)), BehaviourOffset.Payload, BehaviourOffset.Size, Zones);
                                                            if (Behaviour.bHasTypedClusterZones)
                                                                for (const FKyaClusterZoneOffsets& ZoneOffsets : Zones)
                                                                {
                                                                    FKyaClusterZone& Zone = Behaviour.ClusterZones.AddDefaulted_GetRef();
                                                                    Zone.Flags = ReadU32(Data + ZoneOffsets.Header);
                                                                    Zone.ZoneRef = int32(ReadU32(Data + ZoneOffsets.Header + 4));
                                                                    TArray<int32>* Lists[] = { &Zone.Actors, &Zone.OtherActors, &Zone.Cameras, &Zone.Lights };
                                                                    for (int32 List = 0; List < 4; ++List)
                                                                        for (int32 Item = 0; Item < ZoneOffsets.Counts[List]; ++Item)
                                                                            Lists[List]->Add(int32(ReadU32(Data + ZoneOffsets.Lists[List] + Item * 4)));
                                                                    for (int32 Item = 0; Item < ZoneOffsets.Counts[4]; ++Item)
                                                                    {
                                                                        const int64 Hash = ZoneOffsets.Lists[4] + int64(Item) * 8;
                                                                        Zone.Hashes.Add(int64(uint64(ReadU32(Data + Hash)) | (uint64(ReadU32(Data + Hash + 4)) << 32)));
                                                                    }
                                                                    Zone.OtherZoneRef = int32(ReadU32(Data + ZoneOffsets.Tail));
                                                                }
                                                        }
                                                    }
                                                    for (int32 Model = 0; Model < Offsets.ModelStrings.Num(); Model += 2)
                                                    {
                                                        FKyaActorModelReference& Reference = Actor.Models.AddDefaulted_GetRef();
                                                        Reference.MeshName = ANSI_TO_TCHAR(reinterpret_cast<const char*>(Data + Offsets.ModelStrings[Model].Offset));
                                                        Reference.TextureName = ANSI_TO_TCHAR(reinterpret_cast<const char*>(Data + Offsets.ModelStrings[Model + 1].Offset));
                                                    }
                                                    for (int32 Index = 0; Index < Offsets.MacroCount; ++Index)
                                                    {
                                                        FKyaActorMacroAnimation& Animation = Actor.MacroAnimations.AddDefaulted_GetRef();
                                                        Animation.AnimationType = int32(ReadU32(Data + Offsets.MacroAnimations + Index * 8));
                                                        Animation.MacroAnimationId = int32(ReadU32(Data + Offsets.MacroAnimations + Index * 8 + 4));
                                                    }
                                                }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        break;
    }
    for (const FKyaBankEntry& Entry : Bank->Entries)
    {
        if (Entry.Type == 6 && Entry.Subtype == 2 && Entry.Size >= 36)
        {
            const uint8* Scene = Buffer + Entry.Offset + 16;
            uint32 Bits = ReadU32(Scene);
            FMemory::Memcpy(&Bank->SceneConfiguration.Field1C, &Bits, sizeof(Bits));
            Bank->SceneConfiguration.Field20 = ReadU32(Scene + 4);
            Bits = ReadU32(Scene + 8);
            FMemory::Memcpy(&Bank->SceneConfiguration.Field24, &Bits, sizeof(Bits));
            Bank->SceneConfiguration.HeaderWords.Add(ReadU32(Scene + 12));
            Bank->SceneConfiguration.HeaderWords.Add(ReadU32(Scene + 16));
            int64 Pos = 36;
            const uint8* Data = Buffer + Entry.Offset;
            if (Pos + 4 > Entry.Size) break;
            const uint32 MeshCount = ReadU32(Data + Pos);
            Pos += 4;
            if (MeshCount > uint64(Entry.Size - Pos) / 4) break;
            for (uint32 Index = 0; Index < MeshCount; ++Index)
                Bank->SceneConfiguration.CommonMeshValues.Add(int32(ReadU32(Data + Pos + Index * 4)));
            Pos += int64(MeshCount) * 4;
            if (Pos + 12 > Entry.Size) break;
            Bank->SceneConfiguration.CommonTextureCount = int32(ReadU32(Data + Pos));
            Bank->SceneConfiguration.TextureWord = int32(ReadU32(Data + Pos + 4));
            Pos += 8;
            const uint32 CollisionCount = ReadU32(Data + Pos);
            Pos += 4;
            if (CollisionCount > uint64(Entry.Size - Pos) / 4) break;
            for (uint32 Index = 0; Index < CollisionCount; ++Index)
                Bank->SceneConfiguration.CollisionBankValues.Add(int32(ReadU32(Data + Pos + Index * 4)));
            Pos += int64(CollisionCount) * 4;
            if (Pos + 4 <= Entry.Size)
                Bank->SceneConfiguration.LevelAnimationCount = int32(ReadU32(Data + Pos));
            if (Pos + 20 > Entry.Size) break;
            for (int32 Index = 0; Index < 4; ++Index)
                Bank->SceneConfiguration.PostAnimationWords.Add(ReadU32(Data + Pos + 4 + Index * 4));
            Pos += 20;
            if (Pos + 4 > Entry.Size) break;
            const uint32 ClassCount = ReadU32(Data + Pos);
            Pos += 4;
            if (ClassCount > uint64(Entry.Size - Pos) / 8) break;
            for (uint32 Index = 0; Index < ClassCount; ++Index)
            {
                FKyaActorClassAllocation Allocation;
                Allocation.ClassId = int32(ReadU32(Data + Pos + Index * 8));
                Allocation.Count = int32(ReadU32(Data + Pos + Index * 8 + 4));
                Bank->SceneConfiguration.ActorClassAllocations.Add(Allocation);
            }
            Pos += int64(ClassCount) * 8;
            if (Pos + 4 <= Entry.Size) Bank->SceneConfiguration.ParticleWord = ReadU32(Data + Pos);
            break;
        }
    }
    Bank->BankBytes.Append(Buffer, int32(Size));
    Buffer = BufferEnd;
    const uint8* LightBuffer = Bank->BankBytes.GetData();
    // Light archive: CLightManager::Level_AddAll reads a count, a table of offsets,
    // then one record per light (CLightManager.cpp:518, LightBase.cpp:5/131/171/194).
    for (const FKyaBankEntry& Candidate : Bank->Entries)
    {
        if (Candidate.Type != 16 || Candidate.Subtype != 1 || Candidate.Offset < 0 || Candidate.Size < 32 ||
            Candidate.Offset > Size || Candidate.Size > Size - Candidate.Offset) continue;
        // The entry is an Eden chunk: 16-byte header, then the THGL record stream.
        if (ReadU32(LightBuffer + Candidate.Offset) != 0xDEADBEEF ||
            FMemory::Memcmp(LightBuffer + Candidate.Offset + 8, "THGL", 4) != 0) continue;
        const int64 LightStart = Candidate.Offset + 16;
        const int64 LightSize = Candidate.Size - 16;
        TArray<FKyaLightOffsets> LightOffsets;
        if (!KyaReadLightOffsets(MakeArrayView(LightBuffer, int32(Size)), LightStart, LightSize, LightOffsets)) continue;
        auto LightReadFloat = [LightBuffer](int64 Offset) {
            uint32 Bits = uint32(LightBuffer[Offset]) | (uint32(LightBuffer[Offset + 1]) << 8) |
                (uint32(LightBuffer[Offset + 2]) << 16) | (uint32(LightBuffer[Offset + 3]) << 24);
            float Value;
            FMemory::Memcpy(&Value, &Bits, sizeof(Value));
            return Value;
        };
        auto Channel = [LightBuffer](int64 Offset, float Multiplier) {
            const uint8 Raw = LightBuffer[Offset];
            const float Value = FMath::Min(Raw, uint8(127)) * 2.0f * Multiplier / 255.0f;
            return FMath::Clamp(Value, 0.0f, 1.0f);
        };
        for (const FKyaLightOffsets& Light : LightOffsets)
        {
            FKyaLight& Result = Bank->Lights.AddDefaulted_GetRef();
            Result.LightType = Light.LightType;
            Result.ShapeType = Light.ShapeType;
            float Multiplier = 0.0f;
            FMemory::Memcpy(&Multiplier, LightBuffer + Light.Multiplier, sizeof(Multiplier));
            Result.Multiplier = Multiplier;
            Result.AmbientColor = FLinearColor(Channel(Light.Ambient, Multiplier), Channel(Light.Ambient + 1, Multiplier),
                Channel(Light.Ambient + 2, Multiplier), Channel(Light.Ambient + 3, Multiplier));
            Result.Color = FLinearColor(Channel(Light.Color, Multiplier), Channel(Light.Color + 1, Multiplier),
                Channel(Light.Color + 2, Multiplier), Channel(Light.Color + 3, Multiplier));
            Result.Field20Color = FLinearColor(Channel(Light.Field20, Multiplier), Channel(Light.Field20 + 1, Multiplier),
                Channel(Light.Field20 + 2, Multiplier), Channel(Light.Field20 + 3, Multiplier));
            if (Light.Position != 0)
            {
                Result.Position = FVector(LightReadFloat(Light.Position), LightReadFloat(Light.Position + 4), LightReadFloat(Light.Position + 8));
                Result.Falloff0 = LightReadFloat(Light.Falloff);
                Result.Falloff1 = LightReadFloat(Light.Falloff + 4);
            }
            if (Light.Direction != 0)
                Result.Direction = FVector(LightReadFloat(Light.Direction), LightReadFloat(Light.Direction + 4), LightReadFloat(Light.Direction + 8));
            if (Light.Fov != 0)
            {
                Result.Fov0 = LightReadFloat(Light.Fov);
                Result.Fov1 = LightReadFloat(Light.Fov + 4);
            }
            for (int32 Index = 0; Index < Light.TrailingCount; ++Index)
                Result.TrailingValues.Add(int32(uint32(LightBuffer[Light.Trailing + Index * 4]) | (uint32(LightBuffer[Light.Trailing + Index * 4 + 1]) << 8) | (uint32(LightBuffer[Light.Trailing + Index * 4 + 2]) << 16) | (uint32(LightBuffer[Light.Trailing + Index * 4 + 3]) << 24)));
        }
        Bank->bHasLights = !Bank->Lights.IsEmpty();
        break;
    }
    return Bank;
}
