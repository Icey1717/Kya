#include "KyaLevelBankExporter.h"
#include "KyaLevelBank.h"
#include "KyaCameraStream.h"
#include "Misc/FeedbackContext.h"

UKyaLevelBankExporter::UKyaLevelBankExporter()
{
    SupportedClass = UKyaLevelBank::StaticClass();
    FormatExtension.Add(TEXT("bnk"));
    FormatDescription.Add(TEXT("Kya level bank"));
    bText = false;
}

bool UKyaLevelBankExporter::ExportBinary(UObject* Object, const TCHAR* Type, FArchive& Ar,
    FFeedbackContext* Warn, int32 FileIndex, uint32 PortFlags)
{
    const UKyaLevelBank* Bank = CastChecked<UKyaLevelBank>(Object);
    if (!Bank->HasCurrentImportSchema())
    {
        Warn->Logf(ELogVerbosity::Error, TEXT("This Kya bank asset uses an older import schema. Reimport the source bank before exporting."));
        return false;
    }
    const FKyaBankEntry* Scene = nullptr;
    const FKyaBankEntry* SceneData = nullptr;
    for (const FKyaBankEntry& Entry : Bank->Entries)
    {
        if (Entry.Type == 6 && Entry.Subtype == 1)
        {
            if (SceneData) return false;
            SceneData = &Entry;
        }
        if (Entry.Type == 6 && Entry.Subtype == 2)
        {
            if (Scene)
            {
                Warn->Logf(ELogVerbosity::Error, TEXT("Multiple scene configuration entries cannot be exported."));
                return false;
            }
            Scene = &Entry;
        }
    }
    if (!Scene || Scene->Size < 28 || Scene->Offset < 0 ||
        Scene->Offset > Bank->BankBytes.Num() || Scene->Size > Bank->BankBytes.Num() - Scene->Offset ||
        Bank->SceneConfiguration.Field20 < 0 || Bank->SceneConfiguration.Field20 > MAX_uint32 ||
        Bank->BankBytes.Num() < 16 || (Bank->BankBytes[12] & 1))
    {
        Warn->Logf(ELogVerbosity::Error, TEXT("Cannot export this bank's scene configuration."));
        return false;
    }
    TArray<uint8> Bytes = Bank->BankBytes;
    auto ReadU32 = [&Bytes](int64 Offset) -> uint32 {
        const uint8* P = Bytes.GetData() + Offset;
        return uint32(P[0]) | (uint32(P[1]) << 8) | (uint32(P[2]) << 16) | (uint32(P[3]) << 24);
    };
    if (!SceneData || SceneData->Offset < 0 || SceneData->Size < 24 ||
        SceneData->Offset > Bytes.Num() || SceneData->Size > Bytes.Num() - SceneData->Offset ||
        Bank->SceneEnvironment.BackgroundFlags < 0 || Bank->SceneEnvironment.BackgroundFlags > MAX_uint32 ||
        Bank->SceneEnvironment.FogFlags < 0 || Bank->SceneEnvironment.FogFlags > MAX_uint32 ||
        Bank->SceneEnvironment.CameraFlags < 0 || Bank->SceneEnvironment.CameraFlags > MAX_uint32 ||
        Bank->SceneEnvironment.CameraConfigFlags < 0 || Bank->SceneEnvironment.CameraConfigFlags > MAX_uint32 ||
        Bank->SceneEnvironment.Field11C < 0 || Bank->SceneEnvironment.Field11C > MAX_uint32 ||
        Bank->SceneEnvironment.Count120 < 0 || Bank->SceneEnvironment.Count120 > MAX_uint32 ||
        Bank->SceneEnvironment.MapField08 < 0 || Bank->SceneEnvironment.MapField08 > MAX_uint32) return false;
    int64 Setup = SceneData->Offset + 16;
    const int64 SceneEnd = SceneData->Offset + SceneData->Size;
    const uint32 HasCluster = ReadU32(Setup);
    Setup += 4;
    if (HasCluster) Setup += 12;
    if (Setup + 4 > SceneEnd) return false;
    const int32 Background = int32(ReadU32(Setup));
    Setup += 4;
    if (Background != -1) Setup += 4;
    if (Setup + 112 > SceneEnd) return false;
    // Flag bit 1 controls the presence of additional camera records. Changing
    // it without rebuilding the variable-length stream would corrupt the bank.
    if (((ReadU32(Setup + 96) ^ uint32(Bank->SceneEnvironment.CameraFlags)) & 2) != 0)
    {
        Warn->Logf(ELogVerbosity::Error, TEXT("Changing camera flag 0x2 requires rebuilding its optional records."));
        return false;
    }
    int64 Config = 0;
    if (!KyaFindCameraConfig(MakeArrayView(Bytes.GetData(), Bytes.Num()), Setup, Config)) return false;
    int64 CameraConditions = 0;
    if (!KyaFindCameraConditions(MakeArrayView(Bytes.GetData(), int32(SceneEnd)), Setup, CameraConditions) ||
        ReadU32(CameraConditions) != uint32(Bank->SceneEnvironment.CameraConditions.Num())) return false;
    int64 CameraSwitch = Setup + 112;
    if (ReadU32(Setup + 96) & 2)
    {
        CameraSwitch += 4;
        const TArray<int32>* Lists[] = { &Bank->SceneEnvironment.CameraSwitchActorsA, &Bank->SceneEnvironment.CameraSwitchActorsB };
        for (const TArray<int32>* List : Lists)
        {
            const uint32 Count = ReadU32(CameraSwitch); CameraSwitch += 4;
            if (Count != uint32(List->Num())) return false;
            CameraSwitch += int64(Count) * 4;
        }
    }
    const int64 ConfigValues = Config + 20 + (ReadU32(Config + 16) == 1 ? 20 : 0);
    int64 SceneValues = 0;
    if (!KyaFindSceneAfterCamera(MakeArrayView(Bytes.GetData(), Bytes.Num()), Config, SceneValues)) return false;
    const int32 OptionalCount = ReadU32(Config + 16) == 1 ? 5 : 0;
    const int32 ExtraCount = ReadU32(ConfigValues + 36) == 1 ? 12 : 0;
    if (Bank->SceneEnvironment.CameraOptionalConfigValues.Num() != OptionalCount ||
        Bank->SceneEnvironment.CameraExtraValues.Num() != ExtraCount) return false;
    const int64 Map = SceneValues + 12;
    int64 MapEnd = 0;
    if (!KyaFindMapEnd(MakeArrayView(Bytes.GetData(), Bytes.Num()), Map, MapEnd) ||
        MapEnd + 48 > SceneEnd || Bank->SceneEnvironment.EffectPoolCounts.Num() != 7 ||
        ReadU32(Map + 12) != uint32(Bank->SceneEnvironment.MapEntries.Num())) return false;
    for (int64 PoolCount : Bank->SceneEnvironment.EffectPoolCounts)
        if (PoolCount < 0 || PoolCount > MAX_uint32) return false;
    TArray<FKyaConditionedOffsets> ScenarioBlocks;
    int64 ScenarioEnd = 0;
    if (!KyaReadConditionedOffsets(MakeArrayView(Bytes.GetData(), int32(SceneEnd)), MapEnd + 48, ScenarioBlocks, ScenarioEnd) ||
        ScenarioBlocks.Num() != Bank->SceneEnvironment.InitialScenarioOperations.Num()) return false;
    for (int32 Block = 0; Block < ScenarioBlocks.Num(); ++Block)
        if (ScenarioBlocks[Block].Operations.Num() != Bank->SceneEnvironment.InitialScenarioOperations[Block].Operations.Num() ||
            ScenarioBlocks[Block].Conditions.Num() != Bank->SceneEnvironment.InitialScenarioOperations[Block].Conditions.Num()) return false;
    TArray<FKyaObjectiveOffsets> Objectives;
    int64 ObjectiveEnd = 0, ObjectiveTrailing = 0;
    if (!KyaReadObjectiveOffsets(MakeArrayView(Bytes.GetData(), int32(SceneEnd)), ScenarioEnd, Objectives, ObjectiveEnd, &ObjectiveTrailing) ||
        Objectives.Num() != Bank->SceneEnvironment.Objectives.Num()) return false;
    if (ReadU32(ObjectiveTrailing) != uint32(Bank->SceneEnvironment.ObjectiveZones.Num()) || ObjectiveEnd + 12 > SceneEnd ||
        ReadU32(ObjectiveEnd + 8) != uint32(Bank->SceneEnvironment.ElevatorCutscenes.Num()) ||
        uint64(Bank->SceneEnvironment.ElevatorCutscenes.Num()) * 4 > uint64(SceneEnd - ObjectiveEnd - 12)) return false;
    int64 WaypointRecords = 0;
    int32 WaypointCount = 0;
    if (!KyaFindWaypoints(MakeArrayView(Bytes.GetData(), int32(SceneEnd)), ObjectiveEnd, WaypointRecords, WaypointCount) ||
        WaypointCount != Bank->SceneEnvironment.Waypoints.Num()) return false;
    TArray<FKyaPathOffsets> Paths;
    if (!KyaReadPathOffsets(MakeArrayView(Bytes.GetData(), int32(SceneEnd)), WaypointRecords, WaypointCount, SceneData->Offset, Paths) ||
        Paths.Num() != Bank->SceneEnvironment.Paths.Num()) return false;
    TArray<FKyaActorOffsets> Actors;
    if (!KyaReadActorOffsets(MakeArrayView(Bytes.GetData(), int32(SceneEnd)), WaypointRecords, SceneData->Offset, Actors) ||
        Actors.Num() != Bank->SceneEnvironment.Actors.Num()) return false;
    for (int32 Index = 0; Index < Actors.Num(); ++Index)
    {
        const FKyaActorPlacement& Actor = Bank->SceneEnvironment.Actors[Index];
        const FKyaActorOffsets& Offsets = Actors[Index];
        if (Actor.Index != int32(ReadU32(Offsets.Chunk + 8)) || Actor.ClassId != ReadU32(Offsets.Chunk + 12) ||
            Actor.MacroAnimations.Num() != Offsets.MacroCount || Actor.HashCode < 0 || Actor.HashCode > MAX_uint32 ||
            Actor.LightingFlags < 0 || Actor.LightingFlags > MAX_uint32 || Actor.Behaviours.Num() != Offsets.Behaviours.Num() ||
            Actor.Conditions.Num() != Offsets.ConditionCount) return false;
        FKyaPlatformPrefixOffsets Platform;
        const bool HasPlatform = Actor.ClassId == 0x0d && KyaReadPlatformPrefix(MakeArrayView(Bytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, Platform);
        FKyaPlatformDetailsOffsets Details;
        const bool HasDetails = HasPlatform && KyaReadPlatformDetails(MakeArrayView(Bytes.GetData(), int32(SceneEnd)), Platform.Suffix, Offsets.ClassData + Offsets.ClassDataSize - Platform.Suffix, Details);
        const int64 RawOffset = HasDetails ? Details.Tail : (HasPlatform ? Platform.Suffix : Offsets.ClassData);
        if (HasPlatform != Actor.bHasPlatformPrefix || HasDetails != Actor.bHasPlatformDetails ||
            Actor.ClassData.Num() != Offsets.ClassData + Offsets.ClassDataSize - RawOffset) return false;
        if (HasPlatform)
        {
            if (Actor.PlatformActors.Num() != Platform.ActorCount || Actor.PlatformZones.Num() != Platform.ZoneCount ||
                Actor.PlatformNoFrictionZones.Num() != Platform.NoFrictionZones.Num()) return false;
            for (int32 Zone = 0; Zone < Platform.NoFrictionZones.Num(); ++Zone)
                if (uint32(Actor.PlatformNoFrictionZones[Zone].Actors.Num()) != ReadU32(Platform.NoFrictionZones[Zone] + 4)) return false;
        }
        if (HasDetails)
        {
            const FKyaPlatformProperties& P = Actor.PlatformProperties;
            if (Actor.PlatformCameras.Num() != Details.CameraCount || Actor.PlatformSoundsA.Num() != Details.SoundCounts[0] || Actor.PlatformSoundsB.Num() != Details.SoundCounts[1] ||
                P.Field1C < 0 || P.Field1C > MAX_uint32 || P.Field20 < 0 || P.Field20 > MAX_uint32 || P.Flags < 0 || P.Flags > MAX_uint32) return false;
            for (const FKyaPlatformCamera& Camera : Actor.PlatformCameras) if (Camera.Data.Num() != 72) return false;
        }
        for (int32 Behaviour = 0; Behaviour < Actor.Behaviours.Num(); ++Behaviour)
        {
            const FKyaActorBehaviour& Source = Actor.Behaviours[Behaviour];
            const FKyaBehaviourOffsets& Location = Offsets.Behaviours[Behaviour];
            if (Source.Data.Num() != Location.Size) return false;
            if (Source.bHasTypedClusterZones)
            {
                if (Actor.ClassId != 0x18 || Source.Id != 2) return false;
                TArray<FKyaClusterZoneOffsets> Zones;
                if (!KyaReadClusterZoneOffsets(MakeArrayView(Bytes.GetData(), int32(SceneEnd)), Location.Payload, Location.Size, Zones) || Zones.Num() != Source.ClusterZones.Num()) return false;
                for (int32 ZoneIndex = 0; ZoneIndex < Zones.Num(); ++ZoneIndex)
                {
                    const FKyaClusterZone& Zone = Source.ClusterZones[ZoneIndex];
                    const int32 Counts[] = { Zone.Actors.Num(), Zone.OtherActors.Num(), Zone.Cameras.Num(), Zone.Lights.Num(), Zone.Hashes.Num() };
                    if (Zone.Flags < 0 || Zone.Flags > MAX_uint32) return false;
                    for (int32 List = 0; List < 5; ++List) if (Counts[List] != Zones[ZoneIndex].Counts[List]) return false;
                }
            }
        }
        if (Actor.bHasTypedCheckpoints)
        {
            if (Actor.ClassId != 0x1e) return false;
            TArray<int64> Checkpoints;
            if (!KyaReadCheckpointOffsets(MakeArrayView(Bytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, Checkpoints) ||
                Checkpoints.Num() != Actor.Checkpoints.Num()) return false;
            for (int32 Checkpoint = 0; Checkpoint < Checkpoints.Num(); ++Checkpoint)
                if (ReadU32(Checkpoints[Checkpoint] + 20) != uint32(Actor.Checkpoints[Checkpoint].ActorWaypoints.Num()) ||
                    Actor.Checkpoints[Checkpoint].Flags < 0 || Actor.Checkpoints[Checkpoint].Flags > MAX_uint32) return false;
        }
        for (TCHAR Char : Actor.Name) if (Char < 1 || Char > 127) return false;
        if (Align(Actor.Name.Len() + 1, 4) != Offsets.NameCapacity || Actor.Models.Num() * 2 != Offsets.ModelStrings.Num()) return false;
        for (int32 Model = 0; Model < Actor.Models.Num(); ++Model)
            for (int32 Part = 0; Part < 2; ++Part)
            {
                const FString& Name = Part == 0 ? Actor.Models[Model].MeshName : Actor.Models[Model].TextureName;
                for (TCHAR Char : Name) if (Char < 1 || Char > 127) return false;
                if (Align(Name.Len() + 1, 4) != Offsets.ModelStrings[Model * 2 + Part].Capacity) return false;
            }
    }
    for (int32 Index = 0; Index < Paths.Num(); ++Index)
    {
        const FKyaPath& Path = Bank->SceneEnvironment.Paths[Index];
        const FKyaPathOffsets& Offsets = Paths[Index];
        const int32 Lengths[7] = { Path.Points.Num(), Path.Rotations.Num(), Path.ExtraVectors.Num(), Path.Delays.Num(), Path.PointFlags.Num(), Path.ExtraValues.Num(), Path.ExtraWords.Num() };
        if (Path.DataFlags != Offsets.Flags || Path.Mode < 0 || Path.Mode > MAX_uint32 || Path.Type < 0 || Path.Type > MAX_uint32 || Path.PathType < 0 || Path.PathType > MAX_uint32) return false;
        for (int32 Array = 0; Array < 7; ++Array)
            if (Lengths[Array] != ((Offsets.Flags & (1u << Array)) ? int64(Offsets.Count) : 0)) return false;
        for (int64 Value : Path.PointFlags) if (Value < 0 || Value > MAX_uint32) return false;
        for (int64 Value : Path.ExtraWords) if (Value < 0 || Value > MAX_uint32) return false;
    }
    for (int32 Index = 0; Index < Objectives.Num(); ++Index)
    {
        const FKyaObjective& Objective = Bank->SceneEnvironment.Objectives[Index];
        if (Objective.Fields.Num() != 6 || Objective.Targets.Num() != Objectives[Index].Targets.Num()) return false;
        for (int64 Field : Objective.Fields) if (Field < 0 || Field > MAX_uint32) return false;
        for (const FKyaObjectiveTarget& Target : Objective.Targets)
            if (Target.MessageKeyA < 0 || Target.MessageKeyA > MAX_uint32 || Target.MessageKeyB < 0 || Target.MessageKeyB > MAX_uint32) return false;
    }
    int64 Pos = Scene->Offset + 36;
    const int64 End = Scene->Offset + Scene->Size;
    if (Pos + 4 > End) return false;
    const uint32 MeshCount = ReadU32(Pos);
    Pos += 4;
    if (MeshCount > uint64(End - Pos) / 4 || MeshCount != uint32(Bank->SceneConfiguration.CommonMeshValues.Num()) ||
        Bank->SceneConfiguration.HeaderWords.Num() != 2 || Bank->SceneConfiguration.PostAnimationWords.Num() != 4 ||
        Bank->SceneConfiguration.ParticleWord < 0 || Bank->SceneConfiguration.ParticleWord > MAX_uint32) return false;
    for (int64 Word : Bank->SceneConfiguration.HeaderWords) if (Word < 0 || Word > MAX_uint32) return false;
    for (int64 Word : Bank->SceneConfiguration.PostAnimationWords) if (Word < 0 || Word > MAX_uint32) return false;
    const int64 MeshDataPos = Pos;
    Pos += int64(MeshCount) * 4;
    if (Pos + 12 > End || Bank->SceneConfiguration.CommonTextureCount < 0) return false;
    const int64 TextureCountPos = Pos;
    Pos += 8;
    const uint32 CollisionCount = ReadU32(Pos);
    Pos += 4;
    if (CollisionCount > uint64(End - Pos) / 4 || CollisionCount != uint32(Bank->SceneConfiguration.CollisionBankValues.Num()))
    {
        Warn->Logf(ELogVerbosity::Error, TEXT("Collision bank count cannot be changed."));
        return false;
    }
    const int64 AnimationCountPos = Pos + int64(CollisionCount) * 4;
    if (AnimationCountPos + 4 > End || Bank->SceneConfiguration.LevelAnimationCount < 0)
    {
        Warn->Logf(ELogVerbosity::Error, TEXT("Invalid animation count or truncated scene configuration."));
        return false;
    }
    const int64 ClassCountPos = AnimationCountPos + 20;
    if (ClassCountPos + 4 > End) return false;
    const uint32 ClassCount = ReadU32(ClassCountPos);
    const int64 ClassDataPos = ClassCountPos + 4;
    if (ClassCount > uint64(End - ClassDataPos) / 8 || ClassCount != uint32(Bank->SceneConfiguration.ActorClassAllocations.Num()) ||
        ClassDataPos + int64(ClassCount) * 8 + 4 > End)
    {
        Warn->Logf(ELogVerbosity::Error, TEXT("Actor class allocation count cannot be changed."));
        return false;
    }
    for (const FKyaActorClassAllocation& Allocation : Bank->SceneConfiguration.ActorClassAllocations)
        if (Allocation.Count < 0) return false;
    auto WriteU32 = [&Bytes](int64 Offset, uint32 Value) {
        for (int32 Byte = 0; Byte != 4; ++Byte)
            Bytes[int32(Offset) + Byte] = uint8(Value >> (Byte * 8));
    };
    uint32 Bits;
    FMemory::Memcpy(&Bits, &Bank->SceneConfiguration.Field1C, sizeof(Bits));
    WriteU32(Scene->Offset + 16, Bits);
    WriteU32(Scene->Offset + 20, uint32(Bank->SceneConfiguration.Field20));
    FMemory::Memcpy(&Bits, &Bank->SceneConfiguration.Field24, sizeof(Bits));
    WriteU32(Scene->Offset + 24, Bits);
    const FKyaSceneEnvironment& Environment = Bank->SceneEnvironment;
    for (int32 Index = 0; Index < Environment.ObjectiveZones.Num(); ++Index)
    {
        WriteU32(ObjectiveTrailing + 4 + Index * 8, uint32(Environment.ObjectiveZones[Index].Field));
        WriteU32(ObjectiveTrailing + 8 + Index * 8, uint32(Environment.ObjectiveZones[Index].ZoneRef));
    }
    WriteU32(ObjectiveEnd, uint32(Environment.Field10C));
    WriteU32(ObjectiveEnd + 4, uint32(Environment.Field110));
    for (int32 Index = 0; Index < Environment.ElevatorCutscenes.Num(); ++Index)
        WriteU32(ObjectiveEnd + 12 + Index * 4, uint32(Environment.ElevatorCutscenes[Index]));
    WriteU32(Setup + 20, uint32(Environment.BackgroundFlags));
    FMemory::Memcpy(&Bits, &Environment.ClipDistance, sizeof(Bits));
    WriteU32(Setup + 24, Bits);
    FMemory::Memcpy(&Bits, &Environment.FogParameter, sizeof(Bits));
    WriteU32(Setup + 28, Bits);
    Bytes[Setup + 32] = Environment.FogColor.R;
    Bytes[Setup + 33] = Environment.FogColor.G;
    Bytes[Setup + 34] = Environment.FogColor.B;
    Bytes[Setup + 35] = Environment.FogColor.A;
    WriteU32(Setup + 36, uint32(Environment.FogFlags));
    WriteU32(Setup + 40, uint32(Environment.MipmapL));
    WriteU32(Setup + 44, uint32(Environment.MipmapK));
    auto WriteFloat = [&](int64 Offset, float Value) {
        uint32 FloatBits;
        FMemory::Memcpy(&FloatBits, &Value, sizeof(FloatBits));
        WriteU32(Offset, FloatBits);
    };
    for (int32 Index = 0; Index < Actors.Num(); ++Index)
    {
        const FKyaActorPlacement& Actor = Environment.Actors[Index];
        const FKyaActorOffsets& Offsets = Actors[Index];
        const FString OriginalName = ANSI_TO_TCHAR(reinterpret_cast<const char*>(Bytes.GetData() + Offsets.Name));
        if (Actor.Name != OriginalName)
        {
            FMemory::Memzero(Bytes.GetData() + Offsets.Name, Offsets.NameCapacity);
            for (int32 Char = 0; Char < Actor.Name.Len(); ++Char) Bytes[Offsets.Name + Char] = uint8(Actor.Name[Char]);
        }
        for (int32 Model = 0; Model < Actor.Models.Num(); ++Model)
            for (int32 Part = 0; Part < 2; ++Part)
            {
                const FString& Name = Part == 0 ? Actor.Models[Model].MeshName : Actor.Models[Model].TextureName;
                const FKyaActorStringOffset& String = Offsets.ModelStrings[Model * 2 + Part];
                if (Name != ANSI_TO_TCHAR(reinterpret_cast<const char*>(Bytes.GetData() + String.Offset)))
                {
                    FMemory::Memzero(Bytes.GetData() + String.Offset, String.Capacity);
                    for (int32 Char = 0; Char < Name.Len(); ++Char) Bytes[String.Offset + Char] = uint8(Name[Char]);
                }
            }
        WriteU32(Offsets.Placement, uint32(Actor.MeshIndex));
        WriteU32(Offsets.Placement + 4, uint32(Actor.TextureIndex));
        WriteU32(Offsets.Placement + 8, uint32(Actor.CollisionIndex));
        for (int32 Behaviour = 0; Behaviour < Actor.Behaviours.Num(); ++Behaviour)
        {
            const FKyaBehaviourOffsets& BehaviourOffset = Offsets.Behaviours[Behaviour];
            WriteU32(BehaviourOffset.Id, uint32(Actor.Behaviours[Behaviour].Id));
            if (BehaviourOffset.Size)
                FMemory::Memcpy(Bytes.GetData() + BehaviourOffset.Payload, Actor.Behaviours[Behaviour].Data.GetData(), BehaviourOffset.Size);
            if (Actor.Behaviours[Behaviour].bHasTypedClusterZones)
            {
                TArray<FKyaClusterZoneOffsets> Zones;
                if (!KyaReadClusterZoneOffsets(MakeArrayView(Bank->BankBytes.GetData(), int32(SceneEnd)), BehaviourOffset.Payload, BehaviourOffset.Size, Zones)) return false;
                for (int32 ZoneIndex = 0; ZoneIndex < Zones.Num(); ++ZoneIndex)
                {
                    const FKyaClusterZone& Zone = Actor.Behaviours[Behaviour].ClusterZones[ZoneIndex];
                    const FKyaClusterZoneOffsets& Location = Zones[ZoneIndex];
                    WriteU32(Location.Header, uint32(Zone.Flags));
                    WriteU32(Location.Header + 4, uint32(Zone.ZoneRef));
                    const TArray<int32>* Lists[] = { &Zone.Actors, &Zone.OtherActors, &Zone.Cameras, &Zone.Lights };
                    for (int32 List = 0; List < 4; ++List)
                        for (int32 Item = 0; Item < Lists[List]->Num(); ++Item)
                            WriteU32(Location.Lists[List] + Item * 4, uint32((*Lists[List])[Item]));
                    for (int32 Item = 0; Item < Zone.Hashes.Num(); ++Item)
                    {
                        const uint64 Hash = uint64(Zone.Hashes[Item]);
                        WriteU32(Location.Lists[4] + int64(Item) * 8, uint32(Hash));
                        WriteU32(Location.Lists[4] + int64(Item) * 8 + 4, uint32(Hash >> 32));
                    }
                    WriteU32(Location.Tail, uint32(Zone.OtherZoneRef));
                }
            }
        }
        const int64 Common = Offsets.Common;
        WriteU32(Common, uint32(Actor.DefaultBehaviourId));
        for (int32 Axis = 0; Axis < 4; ++Axis) WriteFloat(Common + 4 + Axis * 4, float(Actor.BoundingSphere[Axis]));
        WriteFloat(Common + 20, Actor.Clipping);
        WriteFloat(Common + 24, Actor.VisibilityDistance);
        WriteFloat(Common + 28, Actor.CullingDistance);
        WriteFloat(Common + 32, Actor.Field20);
        WriteU32(Common + 36, uint32(Actor.SectorId));
        WriteU32(Common + 40, uint32(Actor.HashCode));
        WriteFloat(Common + 44, float(Actor.LodBiases.X));
        WriteFloat(Common + 48, float(Actor.LodBiases.Y));
        WriteU32(Common + 56, uint32(Actor.ActorFlags));
        WriteU32(Common + 60, uint32(Actor.AnimationLayers));
        WriteU32(Common + 64, uint32(Actor.Field40));
        WriteU32(Common + 72, uint32(Actor.LightingFlags));
        WriteFloat(Common + 76, Actor.LightingValue);
        for (int32 ConditionIndex = 0; ConditionIndex < Actor.Conditions.Num(); ++ConditionIndex)
        {
            const FKyaScenarioCondition& Condition = Actor.Conditions[ConditionIndex];
            const int64 Offset = Offsets.Conditions + int64(ConditionIndex) * 16;
            WriteU32(Offset, uint32(Condition.Combiner));
            WriteU32(Offset + 4, uint32(Condition.Variable));
            WriteU32(Offset + 8, uint32(Condition.Comparison));
            WriteU32(Offset + 12, uint32(Condition.Value));
        }
        FKyaPlatformPrefixOffsets Platform;
        const bool HasPlatform = Actor.bHasPlatformPrefix && KyaReadPlatformPrefix(MakeArrayView(Bank->BankBytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, Platform);
        FKyaPlatformDetailsOffsets Details;
        const bool HasDetails = HasPlatform && Actor.bHasPlatformDetails && KyaReadPlatformDetails(MakeArrayView(Bank->BankBytes.GetData(), int32(SceneEnd)), Platform.Suffix, Offsets.ClassData + Offsets.ClassDataSize - Platform.Suffix, Details);
        const int64 RawOffset = HasDetails ? Details.Tail : (HasPlatform ? Platform.Suffix : Offsets.ClassData);
        if (!Actor.ClassData.IsEmpty()) FMemory::Memcpy(Bytes.GetData() + RawOffset, Actor.ClassData.GetData(), Actor.ClassData.Num());
        if (HasPlatform)
        {
            WriteFloat(Platform.Weight, Actor.PlatformWeight);
            for (int32 Item = 0; Item < Actor.PlatformActors.Num(); ++Item)
            {
                WriteU32(Platform.Actors + Item * 8, uint32(Actor.PlatformActors[Item].ActorRef));
                WriteU32(Platform.Actors + Item * 8 + 4, uint32(Actor.PlatformActors[Item].CarryMethod));
            }
            for (int32 Item = 0; Item < Actor.PlatformZones.Num(); ++Item)
                WriteU32(Platform.Zones + Item * 4, uint32(Actor.PlatformZones[Item]));
            for (int32 Zone = 0; Zone < Actor.PlatformNoFrictionZones.Num(); ++Zone)
            {
                const int64 Offset = Platform.NoFrictionZones[Zone];
                WriteU32(Offset, uint32(Actor.PlatformNoFrictionZones[Zone].ZoneRef));
                for (int32 Item = 0; Item < Actor.PlatformNoFrictionZones[Zone].Actors.Num(); ++Item)
                    WriteU32(Offset + 8 + Item * 4, uint32(Actor.PlatformNoFrictionZones[Zone].Actors[Item]));
            }
        }
        if (HasDetails)
        {
            for (int32 Camera = 0; Camera < Actor.PlatformCameras.Num(); ++Camera)
                FMemory::Memcpy(Bytes.GetData() + Details.Cameras + int64(Camera) * 72, Actor.PlatformCameras[Camera].Data.GetData(), 72);
            const FKyaPlatformProperties& P = Actor.PlatformProperties;
            const int64 Offset = Details.Properties;
            WriteU32(Offset, uint32(P.SoundA)); WriteU32(Offset + 4, uint32(P.SampleA));
            WriteU32(Offset + 8, uint32(P.SoundB)); WriteU32(Offset + 12, uint32(P.SampleB));
            for (int32 Axis = 0; Axis < 3; ++Axis) WriteFloat(Offset + 16 + Axis * 4, float(P.Offset[Axis]));
            WriteU32(Offset + 28, uint32(P.Field1C)); WriteU32(Offset + 32, uint32(P.Field20));
            WriteU32(Offset + 36, uint32(P.Flags)); WriteU32(Offset + 40, uint32(P.Field28));
            WriteFloat(Offset + 44, P.TiltValue); WriteU32(Offset + 48, uint32(P.TiltMode));
            for (int32 Axis = 0; Axis < 4; ++Axis) WriteFloat(Offset + 52 + Axis * 4, float(P.TiltAndPush[Axis]));
            WriteFloat(Offset + 68, float(P.PushOscillation.X)); WriteFloat(Offset + 72, float(P.PushOscillation.Y));
            const TArray<FKyaPlatformSound>* Lists[] = { &Actor.PlatformSoundsA, &Actor.PlatformSoundsB };
            for (int32 List = 0; List < 2; ++List)
                for (int32 Item = 0; Item < Lists[List]->Num(); ++Item)
                {
                    const FKyaPlatformSound& Sound = (*Lists[List])[Item];
                    const int64 SoundOffset = Details.Sounds[List] + int64(Item) * 16;
                    WriteU32(SoundOffset, uint32(Sound.Field0)); WriteU32(SoundOffset + 4, uint32(Sound.Field4));
                    WriteU32(SoundOffset + 8, uint32(Sound.SoundRef)); WriteU32(SoundOffset + 12, uint32(Sound.SampleRef));
                }
        }
        if (Actor.bHasTypedCheckpoints)
        {
            TArray<int64> Checkpoints;
            if (!KyaReadCheckpointOffsets(MakeArrayView(Bank->BankBytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, Checkpoints)) return false;
            for (int32 CheckpointIndex = 0; CheckpointIndex < Checkpoints.Num(); ++CheckpointIndex)
            {
                const FKyaCheckpoint& Checkpoint = Actor.Checkpoints[CheckpointIndex];
                const int64 Offset = Checkpoints[CheckpointIndex];
                WriteU32(Offset, uint32(Checkpoint.ZoneRef));
                WriteU32(Offset + 4, uint32(Checkpoint.WaypointA));
                WriteU32(Offset + 8, uint32(Checkpoint.SectorId));
                WriteU32(Offset + 12, uint32(Checkpoint.WaypointB));
                WriteU32(Offset + 16, uint32(Checkpoint.Flags));
                for (int32 Pair = 0; Pair < Checkpoint.ActorWaypoints.Num(); ++Pair)
                {
                    WriteU32(Offset + 24 + Pair * 8, uint32(Checkpoint.ActorWaypoints[Pair].ActorRef));
                    WriteU32(Offset + 28 + Pair * 8, uint32(Checkpoint.ActorWaypoints[Pair].WaypointRef));
                }
            }
        }
        for (int32 Animation = 0; Animation < Actor.MacroAnimations.Num(); ++Animation)
        {
            WriteU32(Offsets.MacroAnimations + Animation * 8, uint32(Actor.MacroAnimations[Animation].AnimationType));
            WriteU32(Offsets.MacroAnimations + Animation * 8 + 4, uint32(Actor.MacroAnimations[Animation].MacroAnimationId));
        }
        for (int32 Axis = 0; Axis < 3; ++Axis)
        {
            WriteFloat(Offsets.Placement + 12 + Axis * 4, float(Actor.Position[Axis]));
            WriteFloat(Offsets.Placement + 24 + Axis * 4, float(Actor.Rotation[Axis]));
            WriteFloat(Offsets.Placement + 36 + Axis * 4, float(Actor.Scale[Axis]));
        }
    }
    for (int32 Index = 0; Index < Paths.Num(); ++Index)
    {
        const FKyaPath& Path = Environment.Paths[Index];
        const FKyaPathOffsets& Offsets = Paths[Index];
        for (int32 Array = 0; Array < 7; ++Array)
            if (Offsets.Arrays[Array] != INDEX_NONE)
                for (uint32 Point = 0; Point < Offsets.Count; ++Point)
                {
                    const int64 Offset = Offsets.Arrays[Array] + Point * (Array < 3 ? 16 : 4);
                    if (Array < 3)
                    {
                        const FVector4& Value = Array == 0 ? Path.Points[Point] : (Array == 1 ? Path.Rotations[Point] : Path.ExtraVectors[Point]);
                        for (int32 Axis = 0; Axis < 4; ++Axis) WriteFloat(Offset + Axis * 4, float(Value[Axis]));
                    }
                    else if (Array == 3) WriteFloat(Offset, Path.Delays[Point]);
                    else if (Array == 4) WriteU32(Offset, uint32(Path.PointFlags[Point]));
                    else if (Array == 5) WriteFloat(Offset, Path.ExtraValues[Point]);
                    else WriteU32(Offset, uint32(Path.ExtraWords[Point]));
                }
        WriteU32(Offsets.Trailer, uint32(Path.Mode));
        WriteU32(Offsets.Trailer + 4, uint32(Path.Type));
        WriteU32(Offsets.Trailer + 8, uint32(Path.PathType));
    }
    for (int32 Index = 0; Index < WaypointCount; ++Index)
    {
        const FKyaWaypoint& Waypoint = Environment.Waypoints[Index];
        const int64 Offset = WaypointRecords + Index * 36;
        for (int32 Axis = 0; Axis < 3; ++Axis)
        {
            WriteFloat(Offset + Axis * 4, float(Waypoint.Position[Axis]));
            WriteFloat(Offset + 12 + Axis * 4, float(Waypoint.Rotation[Axis]));
            WriteFloat(Offset + 24 + Axis * 4, float(Waypoint.Parameters[Axis]));
        }
    }
    WriteFloat(Setup + 48, Environment.CameraFov);
    for (int32 Axis = 0; Axis < 3; ++Axis)
    {
        WriteFloat(Setup + 52 + Axis * 4, float(Environment.CameraPosition[Axis]));
        WriteFloat(Setup + 64 + Axis * 4, float(Environment.CameraLookAt[Axis]));
        WriteFloat(Setup + 76 + Axis * 4, float(Environment.CameraUp[Axis]));
        WriteFloat(Setup + 100 + Axis * 4, float(Environment.CameraTargetOffset[Axis]));
    }
    WriteU32(Setup + 88, uint32(Environment.CameraSectorId));
    WriteU32(Setup + 92, uint32(Environment.CameraField08));
    WriteU32(Setup + 96, uint32(Environment.CameraFlags));
    if (ReadU32(Setup + 96) & 2)
    {
        int64 Offset = Setup + 112;
        WriteU32(Offset, uint32(Environment.CameraSwitchActor)); Offset += 4;
        const TArray<int32>* Lists[] = { &Environment.CameraSwitchActorsA, &Environment.CameraSwitchActorsB };
        for (const TArray<int32>* List : Lists)
        {
            Offset += 4;
            for (int32 ActorRef : *List) { WriteU32(Offset, uint32(ActorRef)); Offset += 4; }
        }
        WriteFloat(Offset, Environment.CameraSwitchValueA);
        WriteU32(Offset + 4, uint32(Environment.CameraSwitchModeA));
        WriteFloat(Offset + 8, Environment.CameraSwitchValueB);
        WriteU32(Offset + 12, uint32(Environment.CameraSwitchModeB));
        WriteFloat(Offset + 16, Environment.CameraSwitchValueC);
    }
    for (int32 Index = 0; Index < Environment.CameraConditions.Num(); ++Index)
    {
        const FKyaCameraCondition& Condition = Environment.CameraConditions[Index];
        const int64 Offset = CameraConditions + 4 + int64(Index) * 16;
        WriteU32(Offset, uint32(Condition.Combiner));
        WriteU32(Offset + 4, uint32(Condition.ActorRef));
        WriteU32(Offset + 8, uint32(Condition.Type));
        WriteFloat(Offset + 12, Condition.Value);
    }
    WriteU32(Config, uint32(Environment.CameraConfigFlags));
    WriteU32(Config + 4, uint32(Environment.CameraActorRefA));
    WriteU32(Config + 8, uint32(Environment.CameraActorRefB));
    WriteFloat(Config + 12, Environment.CameraConfigAngle);
    for (int32 Index = 0; Index < OptionalCount; ++Index)
        WriteFloat(Config + 20 + Index * 4, Environment.CameraOptionalConfigValues[Index]);
    for (int32 Index = 0; Index < ExtraCount; ++Index)
        WriteFloat(ConfigValues + 40 + Index * 4, Environment.CameraExtraValues[Index]);
    WriteFloat(SceneValues, Environment.Field118);
    WriteU32(SceneValues + 4, uint32(Environment.Field11C));
    WriteU32(SceneValues + 8, uint32(Environment.Count120));
    WriteU32(Map, uint32(Environment.MapField04));
    WriteU32(Map + 4, uint32(Environment.MapField08));
    WriteFloat(Map + 8, Environment.MapAngle);
    for (int32 Index = 0; Index < Environment.MapEntries.Num(); ++Index)
        WriteU32(Map + 16 + Index * 4, uint32(Environment.MapEntries[Index]));
    WriteU32(MapEnd, uint32(Environment.MapField14));
    WriteU32(MapEnd + 4, uint32(Environment.MapActorRef));
    WriteU32(MapEnd + 8, uint32(Environment.DefaultTextureIndex));
    WriteU32(MapEnd + 12, uint32(Environment.AlternateTextureIndex));
    WriteU32(MapEnd + 16, uint32(Environment.DefaultMaterialIndex));
    for (int32 Category = 0; Category < 7; ++Category)
        WriteU32(MapEnd + 20 + Category * 4, uint32(Environment.EffectPoolCounts[Category]));
    for (int32 Block = 0; Block < ScenarioBlocks.Num(); ++Block)
    {
        const FKyaConditionedOperation& Source = Environment.InitialScenarioOperations[Block];
        for (int32 Index = 0; Index < Source.Operations.Num(); ++Index)
        {
            const int64 Offset = ScenarioBlocks[Block].Operations[Index];
            WriteU32(Offset, uint32(Source.Operations[Index].Variable));
            WriteU32(Offset + 4, uint32(Source.Operations[Index].Type));
            WriteU32(Offset + 8, uint32(Source.Operations[Index].Value));
        }
        for (int32 Index = 0; Index < Source.Conditions.Num(); ++Index)
        {
            const int64 Offset = ScenarioBlocks[Block].Conditions[Index];
            WriteU32(Offset, uint32(Source.Conditions[Index].Combiner));
            WriteU32(Offset + 4, uint32(Source.Conditions[Index].Variable));
            WriteU32(Offset + 8, uint32(Source.Conditions[Index].Comparison));
            WriteU32(Offset + 12, uint32(Source.Conditions[Index].Value));
        }
    }
    for (int32 Index = 0; Index < Objectives.Num(); ++Index)
    {
        const FKyaObjective& Objective = Environment.Objectives[Index];
        const FKyaObjectiveOffsets& Offsets = Objectives[Index];
        WriteU32(Offsets.Header, uint32(Objective.ScenarioVariable));
        for (int32 Field = 0; Field < 6; ++Field)
            WriteU32(Offsets.Header + 4 + Field * 4, uint32(Objective.Fields[Field]));
        for (int32 TargetIndex = 0; TargetIndex < Objective.Targets.Num(); ++TargetIndex)
        {
            const FKyaObjectiveTarget& Target = Objective.Targets[TargetIndex];
            const int64 Offset = Offsets.Targets[TargetIndex];
            WriteU32(Offset, uint32(Target.MessageKeyA));
            WriteU32(Offset + 4, uint32(Target.MessageKeyB));
            WriteU32(Offset + 8, uint32(Target.ActorRef));
            WriteU32(Offset + 12, uint32(Target.ZoneRef));
        }
    }
    for (int32 Axis = 0; Axis < 3; ++Axis)
    {
        WriteFloat(ConfigValues + Axis * 4, float(Environment.CameraPitchAngles[Axis]));
        WriteFloat(ConfigValues + 12 + Axis * 4, float(Environment.CameraOtherAngles[Axis]));
        WriteFloat(ConfigValues + 24 + Axis * 4, float(Environment.CameraDistances[Axis]));
    }
    WriteU32(TextureCountPos, uint32(Bank->SceneConfiguration.CommonTextureCount));
    WriteU32(Scene->Offset + 28, uint32(Bank->SceneConfiguration.HeaderWords[0]));
    WriteU32(Scene->Offset + 32, uint32(Bank->SceneConfiguration.HeaderWords[1]));
    for (int32 Index = 0; Index < Bank->SceneConfiguration.CommonMeshValues.Num(); ++Index)
        WriteU32(MeshDataPos + Index * 4, uint32(Bank->SceneConfiguration.CommonMeshValues[Index]));
    WriteU32(TextureCountPos + 4, uint32(Bank->SceneConfiguration.TextureWord));
    for (uint32 Index = 0; Index < CollisionCount; ++Index)
        WriteU32(Pos + Index * 4, uint32(Bank->SceneConfiguration.CollisionBankValues[Index]));
    WriteU32(AnimationCountPos, uint32(Bank->SceneConfiguration.LevelAnimationCount));
    for (int32 Index = 0; Index < 4; ++Index)
        WriteU32(AnimationCountPos + 4 + Index * 4, uint32(Bank->SceneConfiguration.PostAnimationWords[Index]));
    for (uint32 Index = 0; Index < ClassCount; ++Index)
    {
        const FKyaActorClassAllocation& Allocation = Bank->SceneConfiguration.ActorClassAllocations[Index];
        WriteU32(ClassDataPos + Index * 8, uint32(Allocation.ClassId));
        WriteU32(ClassDataPos + Index * 8 + 4, uint32(Allocation.Count));
    }
    WriteU32(ClassDataPos + int64(ClassCount) * 8, uint32(Bank->SceneConfiguration.ParticleWord));
    Ar.Serialize(Bytes.GetData(), Bytes.Num());
    return !Ar.IsError();
}
