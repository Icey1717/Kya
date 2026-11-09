#include "KyaLevelBankFactory.h"
#include "KyaLevelBank.h"
#include "KyaCameraStream.h"
#include "Misc/FeedbackContext.h"

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
                                                                Actor.PlatformCameras.AddDefaulted_GetRef().Data.Append(Data + Details.Cameras + int64(Index) * 72, 72);
                                                            FKyaPlatformProperties& P = Actor.PlatformProperties;
                                                            const int64 Offset = Details.Properties;
                                                            P.SoundA = int32(ReadU32(Data + Offset)); P.SampleA = int32(ReadU32(Data + Offset + 4));
                                                            P.SoundB = int32(ReadU32(Data + Offset + 8)); P.SampleB = int32(ReadU32(Data + Offset + 12));
                                                            P.Offset = FVector(ReadFloat(Offset + 16), ReadFloat(Offset + 20), ReadFloat(Offset + 24));
                                                            P.Field1C = ReadU32(Data + Offset + 28); P.Field20 = ReadU32(Data + Offset + 32);
                                                            P.Flags = ReadU32(Data + Offset + 36); P.Field28 = int32(ReadU32(Data + Offset + 40));
                                                            P.TiltValue = ReadFloat(Offset + 44); P.TiltMode = int32(ReadU32(Data + Offset + 48));
                                                            P.TiltAndPush = FVector4(ReadFloat(Offset + 52), ReadFloat(Offset + 56), ReadFloat(Offset + 60), ReadFloat(Offset + 64));
                                                            P.PushOscillation = FVector2D(ReadFloat(Offset + 68), ReadFloat(Offset + 72));
                                                            TArray<FKyaPlatformSound>* SoundLists[] = { &Actor.PlatformSoundsA, &Actor.PlatformSoundsB };
                                                            for (int32 List = 0; List < 2; ++List)
                                                                for (int32 Item = 0; Item < Details.SoundCounts[List]; ++Item)
                                                                {
                                                                    const int64 SoundOffset = Details.Sounds[List] + int64(Item) * 16;
                                                                    FKyaPlatformSound& Sound = SoundLists[List]->AddDefaulted_GetRef();
                                                                    Sound.Field0 = int32(ReadU32(Data + SoundOffset)); Sound.Field4 = int32(ReadU32(Data + SoundOffset + 4));
                                                                    Sound.SoundRef = int32(ReadU32(Data + SoundOffset + 8)); Sound.SampleRef = int32(ReadU32(Data + SoundOffset + 12));
                                                                }
                                                            Actor.ClassData.Append(Data + Details.Tail, int32(Offsets.ClassData + Offsets.ClassDataSize - Details.Tail));
                                                        }
                                                        else Actor.ClassData.Append(Data + Platform.Suffix, int32(Remaining));
                                                    }
                                                    else Actor.ClassData.Append(Data + Offsets.ClassData, Offsets.ClassDataSize);
                                                    if (Actor.ClassId == 0x1e)
                                                    {
                                                        TArray<int64> Checkpoints;
                                                        Actor.bHasTypedCheckpoints = KyaReadCheckpointOffsets(MakeArrayView(Data, int32(Entry.Size)), Offsets.ClassData, Offsets.ClassDataSize, Checkpoints);
                                                        if (Actor.bHasTypedCheckpoints)
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
                                                    }
                                                    for (const FKyaBehaviourOffsets& BehaviourOffset : Offsets.Behaviours)
                                                    {
                                                        FKyaActorBehaviour& Behaviour = Actor.Behaviours.AddDefaulted_GetRef();
                                                        Behaviour.Id = int32(ReadU32(Data + BehaviourOffset.Id));
                                                        Behaviour.Data.Append(Data + BehaviourOffset.Payload, BehaviourOffset.Size);
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
    return Bank;
}
