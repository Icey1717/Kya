#pragma once
#include "CoreMinimal.h"
#include "KyaLevelBank.h"

// Locate the condition count after CCamera's optional actor lists.
inline bool KyaFindCameraConditions(TConstArrayView<uint8> Data, int64 Setup, int64& OutOffset)
{
    auto Fits = [&](int64 Offset, int64 Size) { return Offset >= 0 && Offset <= Data.Num() && Size <= Data.Num() - Offset; };
    auto U32 = [&](int64 Offset) { const uint8* P = Data.GetData() + Offset; return uint32(P[0]) | uint32(P[1]) << 8 | uint32(P[2]) << 16 | uint32(P[3]) << 24; };
    if (!Fits(Setup, 112)) return false;
    int64 Pos = Setup + 112;
    if (U32(Setup + 96) & 2)
    {
        if (!Fits(Pos, 4)) return false;
        Pos += 4;
        for (int32 List = 0; List < 2; ++List)
        {
            if (!Fits(Pos, 4)) return false;
            const int32 Count = int32(U32(Pos));
            Pos += 4;
            if (Count < 0 || !Fits(Pos, int64(Count) * 4)) return false;
            Pos += int64(Count) * 4;
        }
        if (!Fits(Pos, 20)) return false;
        Pos += 20;
    }
    if (!Fits(Pos, 4)) return false;
    const int32 Conditions = int32(U32(Pos));
    OutOffset = Pos;
    Pos += 4;
    if (Conditions < 0 || !Fits(Pos, int64(Conditions) * 16)) return false;
    return true;
}

// Locate CAMERA_CONFIG after CCamera's optional actor lists and conditions.
inline bool KyaFindCameraConfig(TConstArrayView<uint8> Data, int64 Setup, int64& OutOffset)
{
    auto Fits = [&](int64 Offset, int64 Size) { return Offset >= 0 && Offset <= Data.Num() && Size <= Data.Num() - Offset; };
    auto U32 = [&](int64 Offset) { const uint8* P = Data.GetData() + Offset; return uint32(P[0]) | uint32(P[1]) << 8 | uint32(P[2]) << 16 | uint32(P[3]) << 24; };
    int64 Pos = 0;
    if (!KyaFindCameraConditions(Data, Setup, Pos)) return false;
    const int32 Conditions = int32(U32(Pos));
    Pos += 4;
    Pos += int64(Conditions) * 16;
    if (!Fits(Pos, 20)) return false;
    const int32 Mode = int32(U32(Pos + 16));
    if (Mode != 0 && Mode != 1) return false;
    if (!Fits(Pos, 56 + (Mode == 1 ? 20 : 0))) return false;
    OutOffset = Pos;
    return true;
}

// astruct_12 follows CAMERA_CONFIG and optionally carries twelve float words.
inline bool KyaFindSceneAfterCamera(TConstArrayView<uint8> Data, int64 Config, int64& OutOffset)
{
    auto Fits = [&](int64 Offset, int64 Size) { return Offset >= 0 && Offset <= Data.Num() && Size <= Data.Num() - Offset; };
    auto U32 = [&](int64 Offset) { const uint8* P = Data.GetData() + Offset; return uint32(P[0]) | uint32(P[1]) << 8 | uint32(P[2]) << 16 | uint32(P[3]) << 24; };
    if (!Fits(Config, 20)) return false;
    const uint32 Mode = U32(Config + 16);
    if (Mode > 1) return false;
    int64 Pos = Config + 56 + (Mode == 1 ? 20 : 0);
    if (!Fits(Pos, 4)) return false;
    const uint32 Extra = U32(Pos);
    Pos += 4;
    if (Extra == 1) Pos += 48;
    if (!Fits(Pos, 12)) return false;
    OutOffset = Pos;
    return true;
}

inline bool KyaFindMapEnd(TConstArrayView<uint8> Data, int64 Map, int64& OutEnd)
{
    if (Map < 0 || Map > Data.Num() || Data.Num() - Map < 16) return false;
    const uint8* P = Data.GetData() + Map + 12;
    const int32 Count = int32(uint32(P[0]) | uint32(P[1]) << 8 | uint32(P[2]) << 16 | uint32(P[3]) << 24);
    if (Count < 0 || int64(Count) * 4 > Data.Num() - Map - 16) return false;
    const int64 End = Map + 16 + int64(Count) * 4;
    if (Data.Num() - End < 20) return false; // two map fields and three scene material IDs
    OutEnd = End;
    return true;
}

struct FKyaConditionedOffsets
{
    TArray<int64> Operations;
    TArray<int64> Conditions;
};
inline bool KyaReadConditionedOffsets(TConstArrayView<uint8> Data, int64 Start,
    TArray<FKyaConditionedOffsets>& OutBlocks, int64& OutEnd)
{
    OutBlocks.Reset();
    auto Fits = [&](int64 Pos, int64 Size) { return Pos >= 0 && Pos <= Data.Num() && Size <= Data.Num() - Pos; };
    auto U32 = [&](int64 Pos) { const uint8* P = Data.GetData() + Pos; return uint32(P[0]) | uint32(P[1]) << 8 | uint32(P[2]) << 16 | uint32(P[3]) << 24; };
    if (!Fits(Start, 4)) return false;
    const uint32 Count = U32(Start);
    int64 Pos = Start + 4;
    if (Count > uint64(Data.Num() - Pos) / 8) return false;
    for (uint32 Block = 0; Block < Count; ++Block)
    {
        if (!Fits(Pos, 4)) return false;
        const uint32 NumOps = U32(Pos); Pos += 4;
        if (NumOps > uint64(Data.Num() - Pos) / 12) return false;
        FKyaConditionedOffsets Offsets;
        for (uint32 Index = 0; Index < NumOps; ++Index, Pos += 12) Offsets.Operations.Add(Pos);
        if (!Fits(Pos, 4)) return false;
        const uint32 NumConditions = U32(Pos); Pos += 4;
        if (NumConditions > uint64(Data.Num() - Pos) / 16) return false;
        for (uint32 Index = 0; Index < NumConditions; ++Index, Pos += 16) Offsets.Conditions.Add(Pos);
        OutBlocks.Add(MoveTemp(Offsets));
    }
    OutEnd = Pos;
    return true;
}

struct FKyaObjectiveOffsets
{
    int64 Header = 0;
    TArray<int64> Targets;
};
inline bool KyaReadObjectiveOffsets(TConstArrayView<uint8> Data, int64 Start,
    TArray<FKyaObjectiveOffsets>& OutObjectives, int64& OutEnd, int64* OutTrailing = nullptr)
{
    OutObjectives.Reset();
    auto Fits = [&](int64 Pos, int64 Size) { return Pos >= 0 && Pos <= Data.Num() && Size <= Data.Num() - Pos; };
    auto U32 = [&](int64 Pos) { const uint8* P = Data.GetData() + Pos; return uint32(P[0]) | uint32(P[1]) << 8 | uint32(P[2]) << 16 | uint32(P[3]) << 24; };
    if (!Fits(Start, 4)) return false;
    const uint32 Count = U32(Start);
    int64 Pos = Start + 4;
    if (Count > uint64(Data.Num() - Pos) / 32) return false;
    for (uint32 Index = 0; Index < Count; ++Index)
    {
        if (!Fits(Pos, 32)) return false;
        FKyaObjectiveOffsets Offsets;
        Offsets.Header = Pos;
        const uint32 Targets = U32(Pos + 28);
        Pos += 32;
        if (Targets > uint64(Data.Num() - Pos) / 16) return false;
        for (uint32 Target = 0; Target < Targets; ++Target, Pos += 16) Offsets.Targets.Add(Pos);
        OutObjectives.Add(MoveTemp(Offsets));
    }
    if (!Fits(Pos, 4)) return false;
    if (OutTrailing) *OutTrailing = Pos;
    const uint32 TrailingCount = U32(Pos);
    Pos += 4;
    if (TrailingCount > uint64(Data.Num() - Pos) / 8) return false;
    OutEnd = Pos + int64(TrailingCount) * 8;
    return true;
}

inline bool KyaFindWaypoints(TConstArrayView<uint8> Data, int64 ObjectiveEnd, int64& OutRecords, int32& OutCount)
{
    auto Fits = [&](int64 Pos, int64 Size) { return Pos >= 0 && Pos <= Data.Num() && Size <= Data.Num() - Pos; };
    auto U32 = [&](int64 Pos) { const uint8* P = Data.GetData() + Pos; return uint32(P[0]) | uint32(P[1]) << 8 | uint32(P[2]) << 16 | uint32(P[3]) << 24; };
    if (!Fits(ObjectiveEnd, 12)) return false;
    const uint32 ElevatorCount = U32(ObjectiveEnd + 8);
    if (ElevatorCount > uint64(Data.Num() - ObjectiveEnd - 12) / 4) return false;
    const int64 Chunk = ObjectiveEnd + 12 + int64(ElevatorCount) * 4;
    if (!Fits(Chunk, 12)) return false;
    const uint32 Size = U32(Chunk + 4), Count = U32(Chunk + 8);
    if (U32(Chunk) != 0x504f5349 || Size < 12 || !Fits(Chunk, Size) || Count > (Size - 12) / 36 || Count > MAX_int32) return false;
    OutRecords = Chunk + 12;
    OutCount = int32(Count);
    return true;
}

struct FKyaPathOffsets
{
    uint32 Flags = 0, Count = 0;
    int64 Arrays[7] = { INDEX_NONE, INDEX_NONE, INDEX_NONE, INDEX_NONE, INDEX_NONE, INDEX_NONE, INDEX_NONE };
    int64 Trailer = 0;
};
inline bool KyaReadPathOffsets(TConstArrayView<uint8> Data, int64 Waypoints, int32 WaypointCount,
    int64 AlignmentBase, TArray<FKyaPathOffsets>& OutPaths)
{
    OutPaths.Reset();
    auto Fits = [&](int64 Pos, int64 Size) { return Pos >= 0 && Pos <= Data.Num() && Size <= Data.Num() - Pos; };
    auto U32 = [&](int64 Pos) { const uint8* P = Data.GetData() + Pos; return uint32(P[0]) | uint32(P[1]) << 8 | uint32(P[2]) << 16 | uint32(P[3]) << 24; };
    if (WaypointCount < 0 || Waypoints < 12) return false;
    const int64 WaypointChunk = Waypoints - 12;
    if (!Fits(WaypointChunk, 12)) return false;
    const int64 Chunk = WaypointChunk + U32(WaypointChunk + 4);
    if (!Fits(Chunk, 12) || U32(Chunk) != 0x50415448) return false;
    const uint32 ChunkSize = U32(Chunk + 4), PathCount = U32(Chunk + 8);
    if (ChunkSize < 12 || !Fits(Chunk, ChunkSize) || PathCount > ChunkSize / 28) return false;
    const int64 End = Chunk + ChunkSize;
    int64 Pos = Chunk + 12;
    for (uint32 Index = 0; Index < PathCount; ++Index)
    {
        Pos = AlignmentBase + Align(Pos - AlignmentBase, int64(16));
        if (Pos < Chunk || Pos > End || End - Pos < 16) return false;
        FKyaPathOffsets Path;
        Path.Flags = U32(Pos);
        Path.Count = U32(Pos + 12);
        Pos += 16;
        for (int32 Array = 0; Array < 7; ++Array)
            if (Path.Flags & (1u << Array))
            {
                const uint64 Size = uint64(Path.Count) * (Array < 3 ? 16 : 4);
                if (Size > uint64(End - Pos)) return false;
                Path.Arrays[Array] = Pos;
                Pos += int64(Size);
            }
        if (End - Pos < 12) return false;
        Path.Trailer = Pos;
        Pos += 12;
        OutPaths.Add(Path);
    }
    return true;
}

struct FKyaBehaviourOffsets { int64 Id = 0, Payload = 0; int32 Size = 0; };
struct FKyaPlatformPrefixOffsets { int64 Weight = 0, Actors = 0, Zones = 0, Suffix = 0; int32 ActorCount = 0, ZoneCount = 0; TArray<int64> NoFrictionZones; };
struct FKyaPlatformDetailsOffsets { int64 Cameras = 0, Properties = 0, Sounds[2] = {}, Tail = 0; int32 CameraCount = 0, SoundCounts[2] = {}; };
inline bool KyaReadPlatformDetails(TConstArrayView<uint8> Data, int64 Start, int64 Size, FKyaPlatformDetailsOffsets& Out)
{
    Out = FKyaPlatformDetailsOffsets{};
    if (Start < 0 || Size < 88 || Start > Data.Num() || Size > Data.Num() - Start) return false;
    auto U32 = [&Data](int64 Offset) { const uint8* P = Data.GetData() + Offset; return uint32(P[0]) | uint32(P[1]) << 8 | uint32(P[2]) << 16 | uint32(P[3]) << 24; };
    const int64 End = Start + Size;
    FKyaPlatformDetailsOffsets Result;
    int64 Pos = Start;
    auto ReadList = [&](int32 Stride, int64& Offset, int32& Count) {
        if (End - Pos < 4) return false;
        const uint32 Num = U32(Pos); Pos += 4;
        if (Num > MAX_int32 || Num > uint64(End - Pos) / Stride) return false;
        Offset = Pos; Count = int32(Num); Pos += int64(Num) * Stride;
        return true;
    };
    if (!ReadList(72, Result.Cameras, Result.CameraCount) || End - Pos < 76) return false;
    Result.Properties = Pos; Pos += 76;
    if (!ReadList(16, Result.Sounds[0], Result.SoundCounts[0]) || !ReadList(16, Result.Sounds[1], Result.SoundCounts[1])) return false;
    Result.Tail = Pos;
    Out = Result;
    return true;
}
inline bool KyaReadPlatformCamera(TConstArrayView<uint8> Data, int64 Offset, FKyaPlatformCamera& Out)
{
    Out = FKyaPlatformCamera{};
    if (Offset < 0 || Offset > Data.Num() || Data.Num() - Offset < 72) return false;
    auto U32 = [&Data](int64 At) { const uint8* P = Data.GetData() + At; return uint32(P[0]) | uint32(P[1]) << 8 | uint32(P[2]) << 16 | uint32(P[3]) << 24; };
    auto F32 = [&U32](int64 At) { const uint32 Bits = U32(At); float Value; FMemory::Memcpy(&Value, &Bits, sizeof(Value)); return Value; };
    Out.EventType = int32(U32(Offset)); Out.Flags = int32(U32(Offset + 4));
    Out.TargetRef = int32(U32(Offset + 8)); Out.CutsceneId = int32(U32(Offset + 12));
    Out.TargetFlags = int32(U32(Offset + 16)); Out.MessageId = int32(U32(Offset + 20));
    Out.MessageFlags = int32(U32(Offset + 24)); Out.TargetField14 = int32(U32(Offset + 28));
    Out.TargetField18 = int32(U32(Offset + 32)); Out.CameraIndex = int32(U32(Offset + 36));
    Out.CameraFlags = int32(U32(Offset + 40));
    Out.Field8 = F32(Offset + 44); Out.FieldC = F32(Offset + 48); Out.Field10 = F32(Offset + 52);
    Out.ActorRef = int32(U32(Offset + 56)); Out.ZoneRef = int32(U32(Offset + 60));
    Out.Field1C = F32(Offset + 64); Out.SoundRef = int32(U32(Offset + 68));
    return true;
}
struct FKyaBasicBoxOffsets
{
    int64 Field160 = 0, LifeMax = 0, Actors = 0, Vibration = 0, Field180 = 0, Fields = 0;
    int64 SwitchTargets = 0, SwitchCamera = 0, AddOn = 0, SubObjMode = 0, SubObjFloats = 0, Tail = 0;
    int32 ActorCount = 0, SwitchTargetCount = 0, SubObjModeValue = 0;
};
inline bool KyaReadBasicBoxOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size, FKyaBasicBoxOffsets& Out)
{
    Out = FKyaBasicBoxOffsets{};
    if (Start < 0 || Size < 0 || Start > Data.Num() || Size > Data.Num() - Start) return false;
    auto U32 = [&Data](int64 Offset) { const uint8* P = Data.GetData() + Offset; return uint32(P[0]) | uint32(P[1]) << 8 | uint32(P[2]) << 16 | uint32(P[3]) << 24; };
    const int64 End = Start + Size;
    FKyaBasicBoxOffsets Result;
    int64 Pos = Start;
    if (End - Pos < 12) return false;
    Result.Field160 = Pos; Result.LifeMax = Pos + 4;
    const uint32 ActorCount = U32(Pos + 8);
    Pos += 12;
    if (ActorCount > uint64(End - Pos) / 4) return false;
    Result.Actors = Pos; Result.ActorCount = int32(ActorCount); Pos += int64(ActorCount) * 4;
    if (End - Pos < 20) return false;
    Result.Vibration = Pos; Pos += 16; Result.Field180 = Pos; Pos += 4;
    if (End - Pos < 16) return false;
    Result.Fields = Pos; Pos += 16;
    if (End - Pos < 36) return false;
    const uint32 SwitchTargetCount = U32(Pos);
    Pos += 4;
    if (SwitchTargetCount > uint64(End - Pos - 32) / 28) return false;
    Result.SwitchTargets = Pos; Result.SwitchTargetCount = int32(SwitchTargetCount); Pos += int64(SwitchTargetCount) * 28;
    Result.SwitchCamera = Pos; Pos += 32;
    if (End - Pos < 24) return false;
    Result.AddOn = Pos; Pos += 20;
    Result.SubObjMode = Pos; Result.SubObjModeValue = int32(U32(Pos)); Pos += 4;
    if (Result.SubObjModeValue == 1)
    {
        if (End - Pos < 20) return false;
        Result.SubObjFloats = Pos; Pos += 20;
    }
    Result.Tail = Pos;
    Out = Result;
    return true;
}
struct FKyaFxBehaviourValues { int32 Value = 0; int32 PathFollowIndex = -1; TArray<int32> UnknownValues; };
inline bool KyaReadFxBehaviour(int32 Id, TConstArrayView<uint8> Data, int64 Start, int64 Size, FKyaFxBehaviourValues& Out)
{
    Out = FKyaFxBehaviourValues{};
    if (Start < 0 || Size < 0 || Start > Data.Num() || Size > Data.Num() - Start) return false;
    auto U32 = [&Data](int64 Offset) { const uint8* P = Data.GetData() + Offset; return uint32(P[0]) | uint32(P[1]) << 8 | uint32(P[2]) << 16 | uint32(P[3]) << 24; };
    if (Id == 2)
    {
        if (Size != 4) return false;
        Out.Value = int32(U32(Start));
        return true;
    }
    if (Id == 3)
    {
        if (Size != 8) return false;
        Out.Value = int32(U32(Start));
        Out.PathFollowIndex = int32(U32(Start + 4));
        return true;
    }
    if (Id == 4)
    {
        if (Size < 4) return false;
        const uint32 Count = U32(Start);
        if (Count > uint64(Size - 4) / 4) return false;
        for (uint32 Index = 0; Index < Count; ++Index)
            Out.UnknownValues.Add(int32(U32(Start + 4 + int64(Index) * 4)));
        return true;
    }
    return false;
}
struct FKyaAraignosOffsets
{
    int64 WeightB = 0, PathFollow = 0, Floats = 0, Sounds = 0, AddOn = 0, SubObjMode = 0, SubObjFloats = 0, Tail = 0;
    int32 SubObjModeValue = 0;
};
inline bool KyaReadAraignosOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size, FKyaAraignosOffsets& Out)
{
    Out = FKyaAraignosOffsets{};
    if (Start < 0 || Size < 56 || Start > Data.Num() || Size > Data.Num() - Start) return false;
    auto U32 = [&Data](int64 Offset) { const uint8* P = Data.GetData() + Offset; return uint32(P[0]) | uint32(P[1]) << 8 | uint32(P[2]) << 16 | uint32(P[3]) << 24; };
    FKyaAraignosOffsets Result;
    Result.WeightB = Start;
    Result.PathFollow = Start + 4;
    Result.Floats = Start + 8;
    Result.Sounds = Start + 20;
    Result.AddOn = Start + 32;
    Result.SubObjMode = Start + 52;
    Result.SubObjModeValue = int32(U32(Result.SubObjMode));
    int64 Pos = Start + 56;
    if (Result.SubObjModeValue == 1)
    {
        if (Size < 76) return false;
        Result.SubObjFloats = Pos;
        Pos += 20;
    }
    Result.Tail = Pos;
    Out = Result;
    return true;
}
struct FKyaProjectileOffsets
{
    int64 WeightB = 0, Autonomous = 0, SubObj = 0, Pattern = 0, PatternField24 = 0, Tail = 0;
    bool bHasPatternField24 = false;
};
inline bool KyaReadProjectileOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size, FKyaProjectileOffsets& Out)
{
    Out = FKyaProjectileOffsets{};
    if (Start < 0 || Start > Data.Num() || Size > Data.Num() - Start) return false;
    if (Size >= 136)
    {
        Out.bHasPatternField24 = true;
        Out.Tail = Start + 136;
        Out.PatternField24 = Start + 128;
    }
    else if (Size == 132) Out.Tail = Start + 132;
    else return false;
    Out.WeightB = Start;
    Out.Autonomous = Start + 4;
    Out.SubObj = Start + 20;
    Out.Pattern = Start + 92;
    return true;
}
struct FKyaWindSwitchOffsets { int64 Targets = 0, Camera = 0; int32 Count = 0; };
inline bool KyaReadSwitch(TConstArrayView<uint8> Data, int64 Targets, int32 Count, int64 Camera, FKyaSwitch& Out)
{
    Out = FKyaSwitch{};
    if (Targets < 0 || Count < 0 || Camera < 0 || Targets > Data.Num() || Camera > Data.Num() ||
        int64(Count) * 28 > Data.Num() - Targets || 32 > Data.Num() - Camera) return false;
    auto U32 = [&Data](int64 Offset) { const uint8* P = Data.GetData() + Offset; return uint32(P[0]) | uint32(P[1]) << 8 | uint32(P[2]) << 16 | uint32(P[3]) << 24; };
    auto F32 = [&U32](int64 Offset) { const uint32 Bits = U32(Offset); float Value; FMemory::Memcpy(&Value, &Bits, sizeof(Value)); return Value; };
    for (int32 Index = 0; Index < Count; ++Index)
    {
        FKyaSwitchTarget& Target = Out.Targets.AddDefaulted_GetRef();
        const int64 T = Targets + int64(Index) * 28;
        Target.TargetRef = int32(U32(T)); Target.CutsceneId = int32(U32(T + 4)); Target.Flags = int32(U32(T + 8));
        Target.MessageId = int32(U32(T + 12)); Target.MessageFlags = int32(U32(T + 16));
        Target.Field14 = int32(U32(T + 20)); Target.Field18 = int32(U32(T + 24));
    }
    Out.Camera.CameraIndex = int32(U32(Camera)); Out.Camera.Flags = int32(U32(Camera + 4));
    Out.Camera.Field8 = F32(Camera + 8); Out.Camera.FieldC = F32(Camera + 12); Out.Camera.Field10 = F32(Camera + 16);
    Out.Camera.ActorRef = int32(U32(Camera + 20)); Out.Camera.ZoneRef = int32(U32(Camera + 24));
    Out.Camera.Field1C = F32(Camera + 28);
    return true;
}
struct FKyaWindOffsets
{
    int64 Field160 = 0, MaxWind = 0, Field168 = 0, Field16C = 0, Field174 = 0, Field180 = 0;
    int64 Field184 = 0, Field188 = 0, Field18C = 0, Field178 = 0, Field17C = 0, Field190 = 0;
    int64 Sound = 0, ActivationZone = 0, Actors = 0;
    int32 ActorCount = 0;
    int64 Material = 0, Zone1C4 = 0, Color1C8 = 0, Waypoint1CC = 0, SectorHash = 0;
    bool bHasSwitches = false;
    FKyaWindSwitchOffsets Switches[3];
    int64 Floats194 = 0, Waypoint = 0, Tail = 0;
};
inline bool KyaReadWindOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size, FKyaWindOffsets& Out)
{
    Out = FKyaWindOffsets{};
    if (Start < 0 || Start > Data.Num() || Size > Data.Num() - Start) return false;
    auto U32 = [&Data](int64 Offset) { const uint8* P = Data.GetData() + Offset; return uint32(P[0]) | uint32(P[1]) << 8 | uint32(P[2]) << 16 | uint32(P[3]) << 24; };
    const int64 End = Start + Size;
    FKyaWindOffsets Result;
    int64 Pos = Start;
    if (End - Pos < 60) return false;
    Result.Field160 = Pos; Result.MaxWind = Pos + 4; Result.Field168 = Pos + 8; Result.Field16C = Pos + 12;
    Result.Field174 = Pos + 16; Result.Field180 = Pos + 20; Result.Field184 = Pos + 24; Result.Field188 = Pos + 28;
    Result.Field18C = Pos + 32; Result.Field178 = Pos + 36; Result.Field17C = Pos + 40; Result.Field190 = Pos + 44;
    Result.Sound = Pos + 48; Result.ActivationZone = Pos + 52;
    const uint32 ActorCount = U32(Pos + 56);
    Pos += 60;
    if (ActorCount > uint64(End - Pos) / 4) return false;
    Result.ActorCount = int32(ActorCount); Result.Actors = Pos; Pos += int64(ActorCount) * 4;
    if (End - Pos < 24) return false;
    Result.Material = Pos; Result.Zone1C4 = Pos + 4; Result.Color1C8 = Pos + 8;
    Result.Waypoint1CC = Pos + 12; Result.SectorHash = Pos + 16;
    Pos += 24;
    if (End - Pos >= 140)
    {
        Result.bHasSwitches = true;
        for (int32 Switch = 0; Switch < 3; ++Switch)
        {
            if (End - Pos < 36) return false;
            const uint32 Count = U32(Pos);
            Pos += 4;
            if (Count > uint64(End - Pos - 32) / 28) return false;
            FKyaWindSwitchOffsets& Offsets = Result.Switches[Switch];
            Offsets.Count = int32(Count); Offsets.Targets = Pos; Pos += int64(Count) * 28;
            Offsets.Camera = Pos; Pos += 32;
        }
    }
    else if (End - Pos < 32) return false;
    if (End - Pos < 32) return false;
    Result.Floats194 = Pos; Pos += 20;
    Result.Waypoint = Pos; Pos += 4;
    Result.Tail = Pos + 8;
    Out = Result;
    return true;
}
struct FKyaTrapOffsets
{
    int64 WeightB = 0, Actors = 0, Field14 = 0, Field18 = 0, Field1C = 0, Field20 = 0, Field24 = 0;
    int64 ActorRef = 0, Speed = 0, Acceleration = 0, PathFollow = 0, Field4C = 0, RequiredEscapes = 0;
    int64 TrapEntries = 0, Tail = 0;
    int32 ActorCount = 0, TrapCount = 0;
    bool bHasField24 = false;
};
inline bool KyaReadTrapOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size, FKyaTrapOffsets& Out)
{
    Out = FKyaTrapOffsets{};
    if (Start < 0 || Start > Data.Num() || Size > Data.Num() - Start) return false;
    auto U32 = [&Data](int64 Offset) { const uint8* P = Data.GetData() + Offset; return uint32(P[0]) | uint32(P[1]) << 8 | uint32(P[2]) << 16 | uint32(P[3]) << 24; };
    const int64 End = Start + Size;
    int64 Pos = Start;
    FKyaTrapOffsets Result;
    if (End - Pos < 8) return false;
    Result.WeightB = Pos;
    const uint32 ActorCount = U32(Pos + 4);
    Pos += 8;
    if (ActorCount > uint64(End - Pos) / 4) return false;
    Result.ActorCount = int32(ActorCount); Result.Actors = Pos; Pos += int64(ActorCount) * 4;
    if (End - Pos < 16) return false;
    Result.Field14 = Pos; Result.Field18 = Pos + 4; Result.Field1C = Pos + 8; Result.Field20 = Pos + 12;
    Pos += 16;
    if (Size >= 56)
    {
        Result.bHasField24 = true;
        if (End - Pos < 4) return false;
        Result.Field24 = Pos; Pos += 4;
    }
    else if (Size != 52) return false;
    if (End - Pos < 24) return false;
    Result.ActorRef = Pos; Result.Speed = Pos + 4; Result.Acceleration = Pos + 8; Result.PathFollow = Pos + 12;
    Result.Field4C = Pos + 16; Result.RequiredEscapes = Pos + 20;
    Pos += 24;
    if (End - Pos < 4) return false;
    const uint32 TrapCount = U32(Pos);
    Pos += 4;
    if (TrapCount > uint64(End - Pos) / 60) return false;
    Result.TrapCount = int32(TrapCount); Result.TrapEntries = Pos; Pos += int64(TrapCount) * 60;
    Result.Tail = Pos;
    Out = Result;
    return true;
}
inline bool KyaSkipString(TConstArrayView<uint8> Data, int64& Pos, int64 End)
{
    if (Pos < 0 || Pos >= End) return false;
    while (Pos < End && Data[Pos] != 0) ++Pos;
    if (Pos >= End) return false;
    ++Pos;
    Pos = (Pos + 3) & ~int64(3);
    return Pos <= End;
}
struct FKyaFighterBlowOffsets
{
    int64 Name = 0, Field4 = 0, Field8 = 0, Field20 = 0, Field30 = 0, Field40 = 0;
    int64 SubObjs = 0, BoneRefs = 0, CanActivateRange = 0, Stages = 0, Tail = 0;
    int32 NameCapacity = 0, SubObjCount = 0;
};
inline bool KyaReadFighterBlowOffsets(TConstArrayView<uint8> Data, int64& Pos, int64 End, FKyaFighterBlowOffsets& Out)
{
    Out = FKyaFighterBlowOffsets{};
    const int64 NameStart = Pos;
    if (!KyaSkipString(Data, Pos, End)) return false;
    Out.Name = NameStart;
    Out.NameCapacity = int32(Pos - NameStart);
    if (End - Pos < 64) return false;
    auto U32At = [&Data](int64 Offset) { const uint8* P = Data.GetData() + Offset; return uint32(P[0]) | uint32(P[1]) << 8 | uint32(P[2]) << 16 | uint32(P[3]) << 24; };
    Out.Field4 = Pos;
    Out.Field8 = Pos + 12;
    Out.Field20 = Pos + 32;
    Out.Field30 = Pos + 44;
    Out.Field40 = Pos + 56;
    const uint32 SubObjs = U32At(Pos + 60);
    Out.SubObjCount = int32(SubObjs);
    Out.SubObjs = Pos + 64;
    Pos += 64;
    if (SubObjs > uint64(End - Pos) / 28) return false;
    Pos += int64(SubObjs) * 28;
    if (End - Pos < 96) return false;
    Out.BoneRefs = Pos; Pos += 8;
    Out.CanActivateRange = Pos; Pos += 4;
    Out.Stages = Pos; Pos += 84;
    Out.Tail = Pos;
    return true;
}
inline bool KyaSkipFighterBlow(TConstArrayView<uint8> Data, int64& Pos, int64 End)
{
    FKyaFighterBlowOffsets Blow;
    if (!KyaReadFighterBlowOffsets(Data, Pos, End, Blow)) return false;
    Pos = Blow.Tail;
    return true;
}
struct FKyaFighterGrabOffsets
{
    int64 Name = 0, Field4 = 0, Field8 = 0, Field14 = 0, Field20 = 0, Field30 = 0, Field34 = 0;
    int64 Field38 = 0, Field3C = 0, Field60 = 0, Field40 = 0, Field44 = 0, Field48 = 0, Field50 = 0;
    int64 Field68 = 0, Stages = 0, Tail = 0;
    int32 NameCapacity = 0;
};
inline bool KyaReadFighterGrabOffsets(TConstArrayView<uint8> Data, int64& Pos, int64 End, FKyaFighterGrabOffsets& Out)
{
    Out = FKyaFighterGrabOffsets{};
    const int64 NameStart = Pos;
    if (!KyaSkipString(Data, Pos, End)) return false;
    Out.Name = NameStart;
    Out.NameCapacity = int32(Pos - NameStart);
    if (End - Pos < 232) return false;
    Out.Field4 = Pos; Out.Field8 = Pos + 12; Out.Field14 = Pos + 24; Out.Field20 = Pos + 28;
    Out.Field30 = Pos + 40; Out.Field34 = Pos + 44; Out.Field38 = Pos + 48; Out.Field3C = Pos + 52;
    Out.Field60 = Pos + 56; Out.Field40 = Pos + 60; Out.Field44 = Pos + 64; Out.Field48 = Pos + 68;
    Out.Field50 = Pos + 72; Out.Field68 = Pos + 84; Out.Stages = Pos + 92;
    Pos += 232;
    Out.Tail = Pos;
    return true;
}
struct FKyaFighterComboOffsets
{
    int64 Name = 0, Field4 = 0, Field8 = 0, Inputs = 0, Field2 = 0, Field3 = 0, FieldC = 0;
    int64 Action = 0, BranchCount = 0, Tail = 0;
    int32 NameCapacity = 0, InputCount = 0, ActionCapacity = 0;
    TArray<int64> BranchOffsets;
    TArray<int32> BranchCapacities;
};
inline bool KyaReadFighterComboOffsets(TConstArrayView<uint8> Data, int64& Pos, int64 End, FKyaFighterComboOffsets& Out)
{
    Out = FKyaFighterComboOffsets{};
    const int64 NameStart = Pos;
    if (!KyaSkipString(Data, Pos, End)) return false;
    Out.Name = NameStart;
    Out.NameCapacity = int32(Pos - NameStart);
    if (End - Pos < 16) return false;
    auto U32At = [&Data](int64 Offset) { const uint8* P = Data.GetData() + Offset; return uint32(P[0]) | uint32(P[1]) << 8 | uint32(P[2]) << 16 | uint32(P[3]) << 24; };
    Out.Field4 = Pos; Out.Field8 = Pos + 8;
    Out.InputCount = int32(U32At(Pos + 12)); Out.Inputs = Pos + 16;
    Pos += 16;
    if (uint64(Out.InputCount) > uint64(End - Pos) / 4) return false;
    Pos += int64(Out.InputCount) * 4;
    if (End - Pos < 12) return false;
    Out.Field2 = Pos; Out.Field3 = Pos + 4; Out.FieldC = Pos + 8; Pos += 12;
    const int64 ActionStart = Pos;
    if (!KyaSkipString(Data, Pos, End)) return false;
    Out.Action = ActionStart;
    Out.ActionCapacity = int32(Pos - ActionStart);
    if (End - Pos < 4) return false;
    Out.BranchCount = Pos;
    const uint32 Branches = U32At(Pos);
    Pos += 4;
    for (uint32 Index = 0; Index < Branches; ++Index)
    {
        const int64 BranchStart = Pos;
        if (!KyaSkipString(Data, Pos, End)) return false;
        Out.BranchOffsets.Add(BranchStart);
        Out.BranchCapacities.Add(int32(Pos - BranchStart));
    }
    Out.Tail = Pos;
    return true;
}
inline bool KyaSkipFighterGrab(TConstArrayView<uint8> Data, int64& Pos, int64 End)
{
    FKyaFighterGrabOffsets Grab;
    if (!KyaReadFighterGrabOffsets(Data, Pos, End, Grab)) return false;
    Pos = Grab.Tail;
    return true;
}
inline bool KyaSkipFighterCombo(TConstArrayView<uint8> Data, int64& Pos, int64 End)
{
    FKyaFighterComboOffsets Combo;
    if (!KyaReadFighterComboOffsets(Data, Pos, End, Combo)) return false;
    Pos = Combo.Tail;
    return true;
}
struct FKyaWolfenOffsets
{
    int64 FighterPrefix = 0, FighterFixed = 0, Blows = 0, Grabs = 0, GrabReacts = 0, Combos = 0, WeaponIndex = 0, WolfenStart = 0;
    int64 FieldB74 = 0, Vision = 0, StartSector = 0, Hearing = 0, VisionDetection = 0, FieldCF4 = 0;
    int64 WalkValues = 0, MagicRequired = 0, FieldD30 = 0, FieldB90 = 0, FightParam = 0, Tail = 0;
};
inline bool KyaReadWolfenOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size, FKyaWolfenOffsets& Out)
{
    Out = FKyaWolfenOffsets{};
    if (Start < 0 || Start > Data.Num() || Size > Data.Num() - Start) return false;
    const int64 End = Start + Size;
    int64 Pos = Start + 20 + 156; // CActorAutonomous and fixed CActorFighter fields.
    if (Pos > End) return false;
    Out.FighterPrefix = Start;
    Out.FighterFixed = Start + 20;
    Out.Blows = Pos;
    auto U32At = [&Data](int64 Offset) { const uint8* P = Data.GetData() + Offset; return uint32(P[0]) | uint32(P[1]) << 8 | uint32(P[2]) << 16 | uint32(P[3]) << 24; };
    if (End - Pos < 4) return false;
    uint32 Count = U32At(Pos); Pos += 4;
    if (Count > uint64(End - Pos) / 12) return false;
    for (uint32 Index = 0; Index < Count; ++Index)
        if (!KyaSkipFighterBlow(Data, Pos, End)) return false;
    Out.Grabs = Pos;
    if (End - Pos < 4) return false;
    Count = U32At(Pos); Pos += 4;
    if (Count > uint64(End - Pos) / 12) return false;
    for (uint32 Index = 0; Index < Count; ++Index)
        if (!KyaSkipFighterGrab(Data, Pos, End)) return false;
    Out.GrabReacts = Pos;
    if (End - Pos < 4) return false;
    Count = U32At(Pos); Pos += 4;
    for (uint32 Index = 0; Index < Count; ++Index)
    {
        if (!KyaSkipString(Data, Pos, End) || End - Pos < 8) return false;
        Pos += 8;
    }
    Out.Combos = Pos;
    if (End - Pos < 4) return false;
    Count = U32At(Pos); Pos += 4;
    for (uint32 Index = 0; Index < Count; ++Index)
        if (!KyaSkipFighterCombo(Data, Pos, End)) return false;
    if (End - Pos < 4) return false;
    Out.WeaponIndex = Pos;
    Pos += 4; // weapon actor index
    Out.WolfenStart = Pos;
    if (End - Pos < 96) return false;
    Out.FieldB74 = Pos; Out.Vision = Pos + 4;
    Pos += 24; // field 0xb74 and the 20-byte CVision record.
    Out.StartSector = Pos; Out.Hearing = Pos + 4; Out.VisionDetection = Pos + 12;
    Out.FieldCF4 = Pos + 16; Out.WalkValues = Pos + 20;
    Out.MagicRequired = Pos + 52; Out.FieldD30 = Pos + 56; Out.FieldB90 = Pos + 60;
    Pos += 72;
    if (End - Pos < 108) return false;
    Out.FightParam = Pos;
    Pos += 20 + 20 + 16;
    auto SkipCapability = [&]() -> bool {
        if (End - Pos < 4) return false;
        const uint32 Items = U32At(Pos); Pos += 4;
        for (uint32 Index = 0; Index < Items; ++Index)
        {
            if (End - Pos < 4) return false;
            const uint32 Chains = U32At(Pos); Pos += 4;
            if (Chains > uint64(End - Pos) / 4) return false;
            Pos += int64(Chains) * 4;
            if (End - Pos < 8) return false;
            Pos += 8;
        }
        return true;
    };
    if (!SkipCapability() || !SkipCapability() || !SkipCapability()) return false;
    for (int32 List = 0; List < 2; ++List)
    {
        if (End - Pos < 4) return false;
        const uint32 Items = U32At(Pos); Pos += 4;
        if (Items > uint64(End - Pos) / 4) return false;
        Pos += int64(Items) * 4;
    }
    if (End - Pos < 48) return false;
    Pos += 48;
    Out.Tail = Pos;
    return true;
}
struct FKyaWolfenCapabilityItemOffsets { int64 CountOffset = 0, Chains = 0, Floats = 0; int32 ChainCount = 0; };
struct FKyaWolfenCapabilityOffsets { int64 CountOffset = 0, Tail = 0; TArray<FKyaWolfenCapabilityItemOffsets> Items; };
struct FKyaWolfenFightParamOffsets
{
    int64 Capability0Fields = 0, Capability2Fields = 0, Floats = 0;
    FKyaWolfenCapabilityOffsets Capabilities[3];
    int64 ComboList1Count = 0, ComboList1Data = 0, ComboList2Count = 0, ComboList2Data = 0;
    int64 Trailing = 0, Tail = 0;
};
inline bool KyaReadWolfenFightParamOffsets(TConstArrayView<uint8> Data, int64 Start, int64 End, FKyaWolfenFightParamOffsets& Out)
{
    Out = FKyaWolfenFightParamOffsets{};
    if (Start < 0 || Start > Data.Num() || End > Data.Num() || End - Start < 56) return false;
    auto U32At = [&Data](int64 Offset) { const uint8* P = Data.GetData() + Offset; return uint32(P[0]) | uint32(P[1]) << 8 | uint32(P[2]) << 16 | uint32(P[3]) << 24; };
    Out.Capability0Fields = Start;
    Out.Capability2Fields = Start + 20;
    Out.Floats = Start + 40;
    int64 Pos = Start + 56;
    for (int32 Cap = 0; Cap < 3; ++Cap)
    {
        if (End - Pos < 4) return false;
        FKyaWolfenCapabilityOffsets& CapOffsets = Out.Capabilities[Cap];
        CapOffsets.CountOffset = Pos;
        const uint32 Count = U32At(Pos); Pos += 4;
        for (uint32 Index = 0; Index < Count; ++Index)
        {
            if (End - Pos < 4) return false;
            FKyaWolfenCapabilityItemOffsets& Item = CapOffsets.Items.AddDefaulted_GetRef();
            Item.CountOffset = Pos;
            Item.ChainCount = int32(U32At(Pos)); Pos += 4;
            if (uint64(Item.ChainCount) > uint64(End - Pos) / 4) return false;
            Item.Chains = Pos; Pos += int64(Item.ChainCount) * 4;
            if (End - Pos < 8) return false;
            Item.Floats = Pos; Pos += 8;
        }
        CapOffsets.Tail = Pos;
    }
    for (int32 List = 0; List < 2; ++List)
    {
        if (End - Pos < 4) return false;
        const int64 CountOffset = Pos;
        const uint32 Count = U32At(Pos); Pos += 4;
        if (uint64(Count) > uint64(End - Pos) / 4) return false;
        if (List == 0) { Out.ComboList1Count = CountOffset; Out.ComboList1Data = Pos; }
        else { Out.ComboList2Count = CountOffset; Out.ComboList2Data = Pos; }
        Pos += int64(Count) * 4;
    }
    if (End - Pos < 48) return false;
    Out.Trailing = Pos;
    Pos += 48;
    Out.Tail = Pos;
    return true;
}
struct FKyaAmortosStreamOffsets { int64 Count = 0, Entries = 0; int32 CountValue = 0; };
struct FKyaAmortosOffsets
{
    FKyaAmortosStreamOffsets Streams[3];
    int64 Field190 = 0, Floats160 = 0, AddOn = 0, SubObjMode = 0, SubObjFloats = 0, Field1C8 = 0;
    FKyaWindSwitchOffsets Switches[2];
    int64 Tail = 0;
    int32 SubObjModeValue = 0;
};
inline bool KyaReadAmortosOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size, FKyaAmortosOffsets& Out)
{
    Out = FKyaAmortosOffsets{};
    if (Start < 0 || Start > Data.Num() || Size > Data.Num() - Start) return false;
    const int64 End = Start + Size;
    auto U32 = [&Data](int64 Offset) { const uint8* P = Data.GetData() + Offset; return uint32(P[0]) | uint32(P[1]) << 8 | uint32(P[2]) << 16 | uint32(P[3]) << 24; };
    FKyaAmortosOffsets Result;
    int64 Pos = Start;
    for (int32 Stream = 0; Stream < 3; ++Stream)
    {
        if (End - Pos < 4) return false;
        FKyaAmortosStreamOffsets& S = Result.Streams[Stream];
        S.Count = Pos;
        const uint32 Count = U32(Pos);
        S.CountValue = int32(Count);
        S.Entries = Pos + 4;
        Pos += 4;
        if (Count > uint64(End - Pos) / 4) return false;
        Pos += int64(Count) * 4;
    }
    if (End - Pos < 40) return false;
    Result.Field190 = Pos;
    Result.Floats160 = Pos + 4;
    Pos += 40;
    if (End - Pos < 24) return false;
    Result.AddOn = Pos;
    Pos += 20;
    Result.SubObjMode = Pos;
    Result.SubObjModeValue = int32(U32(Pos));
    Pos += 4;
    if (Result.SubObjModeValue == 1)
    {
        if (End - Pos < 20) return false;
        Result.SubObjFloats = Pos;
        Pos += 20;
    }
    if (End - Pos < 4) return false;
    Result.Field1C8 = Pos;
    Pos += 4;
    for (int32 Switch = 0; Switch < 2; ++Switch)
    {
        if (End - Pos < 4) return false;
        FKyaWindSwitchOffsets& S = Result.Switches[Switch];
        const uint32 Count = U32(Pos);
        S.Count = int32(Count);
        S.Targets = Pos + 4;
        if (Count > uint64(End - Pos - 4) / 28) return false;
        S.Camera = S.Targets + int64(Count) * 28;
        if (End - S.Camera < 32) return false;
        Pos = S.Camera + 32;
    }
    Result.Tail = Pos;
    Out = Result;
    return true;
}
struct FKyaDcaOffsets
{
    int64 Field504 = 0, Field500 = 0, Field464 = 0, Field508 = 0, Field50C = 0, AimAdjust = 0;
    int64 ShotCount = 0, ShotEntries = 0;
    int32 ShotCountValue = 0;
    int64 Field450 = 0, Field454 = 0, Tail = 0;
};
inline bool KyaReadDcaOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size, FKyaDcaOffsets& Out)
{
    Out = FKyaDcaOffsets{};
    if (Start < 0 || Size < 36 || Start > Data.Num() || Size > Data.Num() - Start) return false;
    const int64 End = Start + Size;
    auto U32 = [&Data](int64 Offset) { const uint8* P = Data.GetData() + Offset; return uint32(P[0]) | uint32(P[1]) << 8 | uint32(P[2]) << 16 | uint32(P[3]) << 24; };
    FKyaDcaOffsets Result;
    Result.Field504 = Start;
    Result.Field500 = Start + 4;
    Result.Field464 = Start + 8;
    Result.Field508 = Start + 12;
    Result.Field50C = Start + 16;
    Result.AimAdjust = Start + 20;
    int64 Pos = Start + 24;
    Result.ShotCount = Pos;
    const uint32 Shots = U32(Pos);
    Result.ShotCountValue = int32(Shots);
    Pos += 4;
    if (uint64(Shots) > uint64(End - Pos) / 4) return false;
    Result.ShotEntries = Pos;
    Pos += int64(Shots) * 4;
    if (End - Pos < 8) return false;
    Result.Field450 = Pos;
    Result.Field454 = Pos + 4;
    Result.Tail = Pos + 8;
    Out = Result;
    return true;
}
struct FKyaDcaBehaviourOffsets
{
    int64 Controlled = 0, Trajectory = 0;
    bool bControlled = false, bTrajectory = false;
    int64 Tail = 0;
};
inline bool KyaReadDcaBehaviour(int32 Id, TConstArrayView<uint8> Data, int64 Start, int64 Size, FKyaDcaBehaviourOffsets& Out)
{
    Out = FKyaDcaBehaviourOffsets{};
    if (Start < 0 || Start > Data.Num() || Size > Data.Num() - Start) return false;
    if (Id == 3)
    {
        if (Size != 40) return false;
        Out.bControlled = true;
        Out.Controlled = Start;
        Out.Tail = Start + 40;
        return true;
    }
    if (Id == 5)
    {
        if (Size != 12) return false;
        Out.bTrajectory = true;
        Out.Trajectory = Start;
        Out.Tail = Start + 12;
        return true;
    }
    return false;
}
struct FKyaActorCameraOffsets
{
    int64 Base = 0, Fov = 0, Position = 0, LookAt = 0, Up = 0, Sector = 0, Field08 = 0, Flags = 0, TargetOffset = 0;
    bool bHasSwitch = false;
    int64 SwitchActor = 0, SwitchListA = 0, SwitchListB = 0, SwitchValues = 0;
    int32 SwitchCountA = 0, SwitchCountB = 0;
    int64 Conditions = 0, Config = 0, ConfigOptional = 0, ConfigAngles = 0, Extra = 0;
    int32 ConditionCount = 0, ConfigMode = 0, ExtraCount = 0;
    int64 Tail = 0;
};
inline bool KyaReadActorCameraOffsets(TConstArrayView<uint8> Data, int64& Pos, int64 End, FKyaActorCameraOffsets& Out)
{
    Out = FKyaActorCameraOffsets{};
    auto U32 = [&Data](int64 Offset) { const uint8* P = Data.GetData() + Offset; return uint32(P[0]) | uint32(P[1]) << 8 | uint32(P[2]) << 16 | uint32(P[3]) << 24; };
    if (Pos < 0 || End > Data.Num() || End - Pos < 64) return false;
    Out.Base = Pos;
    Out.Fov = Pos;
    Out.Position = Pos + 4;
    Out.LookAt = Pos + 16;
    Out.Up = Pos + 28;
    Out.Sector = Pos + 40;
    Out.Field08 = Pos + 44;
    Out.Flags = Pos + 48;
    Out.TargetOffset = Pos + 52;
    Pos += 64;
    if (U32(Out.Flags) & 2)
    {
        Out.bHasSwitch = true;
        if (End - Pos < 4) return false;
        Out.SwitchActor = Pos;
        Pos += 4;
        for (int32 List = 0; List < 2; ++List)
        {
            if (End - Pos < 4) return false;
            const uint32 Count = U32(Pos);
            if (List == 0) { Out.SwitchListA = Pos + 4; Out.SwitchCountA = int32(Count); }
            else { Out.SwitchListB = Pos + 4; Out.SwitchCountB = int32(Count); }
            Pos += 4;
            if (uint64(Count) > uint64(End - Pos) / 4) return false;
            Pos += int64(Count) * 4;
        }
        if (End - Pos < 20) return false;
        Out.SwitchValues = Pos;
        Pos += 20;
    }
    if (End - Pos < 4) return false;
    Out.Conditions = Pos;
    const uint32 Conditions = U32(Pos);
    Out.ConditionCount = int32(Conditions);
    Pos += 4;
    if (uint64(Conditions) > uint64(End - Pos) / 16) return false;
    Pos += int64(Conditions) * 16;
    if (End - Pos < 20) return false;
    Out.Config = Pos;
    const uint32 Mode = U32(Pos + 16);
    Out.ConfigMode = int32(Mode);
    if (Mode > 1) return false;
    Pos += 20;
    if (Mode == 1)
    {
        if (End - Pos < 20) return false;
        Out.ConfigOptional = Pos;
        Pos += 20;
    }
    if (End - Pos < 36) return false;
    Out.ConfigAngles = Pos;
    Pos += 36;
    if (End - Pos < 4) return false;
    Out.Extra = Pos;
    const uint32 ExtraCount = U32(Pos);
    Out.ExtraCount = int32(ExtraCount);
    Pos += 4;
    if (ExtraCount == 1)
    {
        if (End - Pos < 48) return false;
        Pos += 48;
    }
    Out.Tail = Pos;
    return true;
}
inline FKyaActorCamera KyaReadActorCameraValues(TConstArrayView<uint8> Data, const FKyaActorCameraOffsets& Off)
{
    auto U32 = [&Data](int64 Offset) { const uint8* P = Data.GetData() + Offset; return uint32(P[0]) | uint32(P[1]) << 8 | uint32(P[2]) << 16 | uint32(P[3]) << 24; };
    auto F32 = [&U32](int64 Offset) { const uint32 Bits = U32(Offset); float Value; FMemory::Memcpy(&Value, &Bits, sizeof(Value)); return Value; };
    FKyaActorCamera Camera;
    Camera.Fov = F32(Off.Fov);
    Camera.Position = FVector(F32(Off.Position), F32(Off.Position + 4), F32(Off.Position + 8));
    Camera.LookAt = FVector(F32(Off.LookAt), F32(Off.LookAt + 4), F32(Off.LookAt + 8));
    Camera.Up = FVector(F32(Off.Up), F32(Off.Up + 4), F32(Off.Up + 8));
    Camera.SectorId = int32(U32(Off.Sector));
    Camera.Field08 = int32(U32(Off.Field08));
    Camera.Flags = int32(U32(Off.Flags));
    Camera.TargetOffset = FVector(F32(Off.TargetOffset), F32(Off.TargetOffset + 4), F32(Off.TargetOffset + 8));
    Camera.bHasSwitch = Off.bHasSwitch;
    if (Off.bHasSwitch)
    {
        Camera.SwitchActor = int32(U32(Off.SwitchActor));
        for (int32 Index = 0; Index < Off.SwitchCountA; ++Index)
            Camera.SwitchActorsA.Add(int32(U32(Off.SwitchListA + Index * 4)));
        for (int32 Index = 0; Index < Off.SwitchCountB; ++Index)
            Camera.SwitchActorsB.Add(int32(U32(Off.SwitchListB + Index * 4)));
        Camera.SwitchValueA = F32(Off.SwitchValues);
        Camera.SwitchModeA = int32(U32(Off.SwitchValues + 4));
        Camera.SwitchValueB = F32(Off.SwitchValues + 8);
        Camera.SwitchModeB = int32(U32(Off.SwitchValues + 12));
        Camera.SwitchValueC = F32(Off.SwitchValues + 16);
    }
    for (int32 Index = 0; Index < Off.ConditionCount; ++Index)
    {
        FKyaCameraCondition& Condition = Camera.Conditions.AddDefaulted_GetRef();
        const int64 C = Off.Conditions + 4 + int64(Index) * 16;
        Condition.Combiner = int32(U32(C));
        Condition.ActorRef = int32(U32(C + 4));
        Condition.Type = int32(U32(C + 8));
        Condition.Value = F32(C + 12);
    }
    Camera.ConfigFlags = int32(U32(Off.Config));
    Camera.ConfigActorRefA = int32(U32(Off.Config + 4));
    Camera.ConfigActorRefB = int32(U32(Off.Config + 8));
    Camera.ConfigAngle = F32(Off.Config + 12);
    if (Off.ConfigMode == 1)
        for (int32 Index = 0; Index < 5; ++Index)
            Camera.ConfigOptionalValues.Add(F32(Off.ConfigOptional + Index * 4));
    Camera.PitchAngles = FVector(F32(Off.ConfigAngles), F32(Off.ConfigAngles + 4), F32(Off.ConfigAngles + 8));
    Camera.OtherAngles = FVector(F32(Off.ConfigAngles + 12), F32(Off.ConfigAngles + 16), F32(Off.ConfigAngles + 20));
    Camera.Distances = FVector(F32(Off.ConfigAngles + 24), F32(Off.ConfigAngles + 28), F32(Off.ConfigAngles + 32));
    if (Off.ExtraCount == 1)
        for (int32 Index = 0; Index < 12; ++Index)
            Camera.ExtraValues.Add(F32(Off.Extra + 4 + Index * 4));
    return Camera;
}
// Skip CActorFighter's serialized prefix (autonomous block, fixed fields, blow/grab/combo databases, weapon index).
struct FKyaFighterPrefixOffsets
{
    int64 Weight = 0, Autonomous = 0, Fixed = 0, Blows = 0, Grabs = 0, GrabReacts = 0, Combos = 0, WeaponIndex = 0, Tail = 0;
};
inline bool KyaReadFighterPrefixOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size, FKyaFighterPrefixOffsets& Out)
{
    Out = FKyaFighterPrefixOffsets{};
    if (Start < 0 || Start > Data.Num() || Size > Data.Num() - Start) return false;
    const int64 End = Start + Size;
    FKyaFighterPrefixOffsets Result;
    Result.Weight = Start;
    Result.Autonomous = Start + 4;
    Result.Fixed = Start + 20;
    int64 Pos = Start + 20 + 156;
    if (Pos > End) return false;
    auto U32At = [&Data](int64 Offset) { const uint8* P = Data.GetData() + Offset; return uint32(P[0]) | uint32(P[1]) << 8 | uint32(P[2]) << 16 | uint32(P[3]) << 24; };
    Result.Blows = Pos;
    if (End - Pos < 4) return false;
    uint32 Count = U32At(Pos); Pos += 4;
    if (Count > uint64(End - Pos) / 12) return false;
    for (uint32 Index = 0; Index < Count; ++Index)
        if (!KyaSkipFighterBlow(Data, Pos, End)) return false;
    Result.Grabs = Pos;
    if (End - Pos < 4) return false;
    Count = U32At(Pos); Pos += 4;
    if (Count > uint64(End - Pos) / 12) return false;
    for (uint32 Index = 0; Index < Count; ++Index)
        if (!KyaSkipFighterGrab(Data, Pos, End)) return false;
    Result.GrabReacts = Pos;
    if (End - Pos < 4) return false;
    Count = U32At(Pos); Pos += 4;
    for (uint32 Index = 0; Index < Count; ++Index)
    {
        if (!KyaSkipString(Data, Pos, End) || End - Pos < 8) return false;
        Pos += 8;
    }
    Result.Combos = Pos;
    if (End - Pos < 4) return false;
    Count = U32At(Pos); Pos += 4;
    for (uint32 Index = 0; Index < Count; ++Index)
        if (!KyaSkipFighterCombo(Data, Pos, End)) return false;
    if (End - Pos < 4) return false;
    Result.WeaponIndex = Pos;
    Pos += 4;
    Result.Tail = Pos;
    Out = Result;
    return true;
}
inline bool KyaReadFighterPrefixEnd(TConstArrayView<uint8> Data, int64 Start, int64 Size, int64& OutEnd)
{
    OutEnd = 0;
    FKyaFighterPrefixOffsets Offsets;
    if (!KyaReadFighterPrefixOffsets(Data, Start, Size, Offsets)) return false;
    OutEnd = Offsets.Tail;
    return true;
}
struct FKyaHeroTutorialOffsets
{
    int64 Header = 0, TargetCount = 0, Targets = 0, Camera = 0, Tail = 0;
    int32 TargetCountValue = 0;
};
struct FKyaKimOffsets
{
    FKyaFighterPrefixOffsets Fighter;
    int64 FighterEnd = 0, Bones = 0;
    int64 ClimbCount = 0, ClimbEntries = 0;
    int32 ClimbCountValue = 0;
    int64 TutorialCount = 0, TutorialEntries = 0;
    int32 TutorialCountValue = 0;
    TArray<FKyaHeroTutorialOffsets> Tutorials;
    int64 ProjectileCount = 0, ProjectileEntries = 0;
    int32 ProjectileCountValue = 0;
    int64 PairCount = 0, PairEntries = 0;
    int32 PairCountValue = 0;
    int64 ZoneStreamCount = 0, ZoneStreamEntries = 0;
    int32 ZoneStreamCountValue = 0;
    int64 Material = 0, FxType = 0, JokeRadius = 0;
    int64 CameraCount = 0;
    int32 CameraCountValue = 0;
    TArray<FKyaActorCameraOffsets> Cameras;
    int64 Field15D4 = 0, FxInit = 0, FxHandles = 0, Tail = 0;
};
inline bool KyaReadKimOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size, FKyaKimOffsets& Out)
{
    Out = FKyaKimOffsets{};
    if (Start < 0 || Size < 20 || Start > Data.Num() || Size > Data.Num() - Start) return false;
    const int64 End = Start + Size;
    auto U32 = [&Data](int64 Offset) { const uint8* P = Data.GetData() + Offset; return uint32(P[0]) | uint32(P[1]) << 8 | uint32(P[2]) << 16 | uint32(P[3]) << 24; };
    FKyaKimOffsets Result;
    int64 Pos = 0;
    if (!KyaReadFighterPrefixEnd(Data, Start, Size, Pos)) return false;
    if (!KyaReadFighterPrefixOffsets(Data, Start, Size, Result.Fighter)) return false;
    Result.FighterEnd = Pos;
    if (End - Pos < 40) return false;
    Result.Bones = Pos;
    Pos += 40;
    if (End - Pos < 4) return false;
    Result.ClimbCount = Pos;
    const uint32 Climb = U32(Pos);
    Result.ClimbCountValue = int32(Climb);
    Pos += 4;
    if (uint64(Climb) > uint64(End - Pos) / 4) return false;
    Result.ClimbEntries = Pos;
    Pos += int64(Climb) * 4;
    if (End - Pos < 4) return false;
    Result.TutorialCount = Pos;
    const uint32 Tutorials = U32(Pos);
    Result.TutorialCountValue = int32(Tutorials);
    Pos += 4;
    Result.TutorialEntries = Pos;
    for (uint32 Index = 0; Index < Tutorials; ++Index)
    {
        if (End - Pos < 24) return false;
        FKyaHeroTutorialOffsets& Tutorial = Result.Tutorials.AddDefaulted_GetRef();
        Tutorial.Header = Pos;
        Pos += 24;
        if (End - Pos < 4) return false;
        Tutorial.TargetCount = Pos;
        const uint32 Targets = U32(Pos);
        Tutorial.TargetCountValue = int32(Targets);
        Pos += 4;
        if (uint64(Targets) > uint64(End - Pos) / 28) return false;
        Tutorial.Targets = Pos;
        Pos += int64(Targets) * 28;
        if (End - Pos < 32) return false;
        Tutorial.Camera = Pos;
        Pos += 32;
        Tutorial.Tail = Pos;
    }
    if (End - Pos < 4) return false;
    Result.ProjectileCount = Pos;
    const uint32 Projectiles = U32(Pos);
    Result.ProjectileCountValue = int32(Projectiles);
    Pos += 4;
    if (uint64(Projectiles) > uint64(End - Pos) / 28) return false;
    Result.ProjectileEntries = Pos;
    Pos += int64(Projectiles) * 28;
    if (End - Pos < 4) return false;
    Result.PairCount = Pos;
    const uint32 Pairs = U32(Pos);
    Result.PairCountValue = int32(Pairs);
    Pos += 4;
    if (uint64(Pairs) > uint64(End - Pos) / 8) return false;
    Result.PairEntries = Pos;
    Pos += int64(Pairs) * 8;
    if (End - Pos < 4) return false;
    Result.ZoneStreamCount = Pos;
    const uint32 ZoneStream = U32(Pos);
    Result.ZoneStreamCountValue = int32(ZoneStream);
    Pos += 4;
    if (uint64(ZoneStream) > uint64(End - Pos) / 4) return false;
    Result.ZoneStreamEntries = Pos;
    Pos += int64(ZoneStream) * 4;
    if (End - Pos < 12) return false;
    Result.Material = Pos;
    Result.FxType = Pos + 4;
    Result.JokeRadius = Pos + 8;
    Pos += 12;
    if (End - Pos < 4) return false;
    Result.CameraCount = Pos;
    const uint32 Cameras = U32(Pos);
    Result.CameraCountValue = int32(Cameras);
    Pos += 4;
    const int32 ExpectedCameras = Cameras == 0 ? 0 : (Cameras < 2 ? 2 : int32(Cameras));
    for (int32 Index = 0; Index < ExpectedCameras; ++Index)
    {
        FKyaActorCameraOffsets Camera;
        if (!KyaReadActorCameraOffsets(Data, Pos, End, Camera)) return false;
        Result.Cameras.Add(Camera);
    }
    if (End - Pos < 20) return false;
    Result.Field15D4 = Pos;
    Result.FxInit = Pos + 4;
    Result.FxHandles = Pos + 8;
    Pos += 20;
    Result.Tail = Pos;
    Out = Result;
    return true;
}
struct FKyaSwitchScriptOffsets
{
    int64 Counts[4] = { 0, 0, 0, 0 };
    int32 CountValues[4] = { 0, 0, 0, 0 };
    TArray<int64> RecordOffsets;
    TArray<int32> RecordCategories;
    int64 Tail = 0;
};
inline bool KyaReadSwitchScriptOffsets(TConstArrayView<uint8> Data, int64 Start, int64 End, FKyaSwitchScriptOffsets& Out)
{
    Out = FKyaSwitchScriptOffsets{};
    if (Start < 0 || End > Data.Num() || End - Start < 32) return false;
    auto U32 = [&Data](int64 Offset) { const uint8* P = Data.GetData() + Offset; return uint32(P[0]) | uint32(P[1]) << 8 | uint32(P[2]) << 16 | uint32(P[3]) << 24; };
    int64 Pos = Start;
    for (int32 Category = 0; Category < 4; ++Category)
    {
        if (End - Pos < 8) return false;
        Pos += 4; // discarded id
        Out.Counts[Category] = Pos;
        const uint32 Count = U32(Pos);
        Out.CountValues[Category] = int32(Count);
        Pos += 4;
        const int64 EntrySize = (Category == 0 || Category == 3) ? 8 : 16;
        if (uint64(Count) > uint64(End - Pos) / EntrySize) return false;
        for (uint32 Index = 0; Index < Count; ++Index)
        {
            Out.RecordOffsets.Add(Pos);
            Out.RecordCategories.Add(Category);
            Pos += EntrySize;
        }
    }
    Out.Tail = Pos;
    return true;
}
struct FKyaTrackSubObjOffsets { int64 Field0 = 0, AField0 = 0, AField4 = 0, AField8 = 0, Field10 = 0; };
struct FKyaWolfenBehaviourOffsets
{
    int64 Flags = 0, TrackField = 0, DcaActorRef = 0, DcaShootCooldown = 0, Avoid80 = 0, Avoid84 = 0;
    int64 WeaponField90 = 0, WeaponField94 = 0, WeaponField98 = 0, WeaponCount = 0, WeaponEntries = 0;
    int64 WeaponFieldF0 = 0, WeaponTargetField = 0;
    int32 WeaponCountValue = 0;
    TArray<FKyaTrackSubObjOffsets> SubObjs;
    FKyaSwitchScriptOffsets Switch;
    int64 Field80 = 0, TrackBehaviourId = 0, OnOffCount = 0, OnOffEntries = 0, OnOffCamera = 0;
    int32 OnOffCountValue = 0;
    int64 SnipeField90 = 0, SnipeField94 = 0, SpotTrack = 0, SnipeTrack = 0, ShadowMaterial = 0, SnipeFieldAC = 0;
    int64 SnipeFieldB0 = 0, SnipeFieldB4 = 0, NbSpot = 0, NbShadow = 0, RotationSpeed = 0, SnipeFieldB8 = 0;
    int64 GuardPath = 0, GuardField98 = 0, GuardTrack = 0, GuardField90 = 0;
    int64 ExorcismMaterial = 0, ExorcismField24 = 0, ExorcismMeshes = 0, ExorcismField18 = 0, ExorcismWords = 0;
    int64 EscapeCount = 0, EscapeEntries = 0, EscapeField88 = 0, EscapeFloats = 0, EscapeFieldA0 = 0;
    int32 EscapeCountValue = 0;
    int64 Tail = 0;
    bool bTrack = false, bDca = false, bAvoid = false, bWeaponStand = false, bWeapon = false, bWeaponSnipe = false, bLost = false, bWatchDog = false, bSnipe = false, bGuard = false, bEscape = false, bExorcism = false;
};
inline bool KyaReadWolfenBehaviourOffsets(int32 Id, TConstArrayView<uint8> Data, int64 Start, int64 Size, FKyaWolfenBehaviourOffsets& Out)
{
    Out = FKyaWolfenBehaviourOffsets{};
    if (Start < 0 || Size < 8 || Start > Data.Num() || Size > Data.Num() - Start) return false;
    auto U32 = [&Data](int64 Offset) { const uint8* P = Data.GetData() + Offset; return uint32(P[0]) | uint32(P[1]) << 8 | uint32(P[2]) << 16 | uint32(P[3]) << 24; };
    Out.Flags = Start;
    if (Id == 15)
    {
        if (Size != 8) return false;
        Out.bTrack = true;
        Out.TrackField = Start + 4;
        Out.Tail = Start + 8;
        return true;
    }
    if (Id == 23)
    {
        if (Size != 12) return false;
        Out.bDca = true;
        Out.DcaActorRef = Start + 4;
        Out.DcaShootCooldown = Start + 8;
        Out.Tail = Start + 12;
        return true;
    }
    if (Id == 24)
    {
        if (Size != 12) return false;
        Out.bAvoid = true;
        Out.Avoid80 = Start + 4;
        Out.Avoid84 = Start + 8;
        Out.Tail = Start + 12;
        return true;
    }
    if (Id == 14)
    {
        if (Size != 48) return false;
        Out.bExorcism = true;
        Out.ExorcismMaterial = Start;
        Out.ExorcismField24 = Start + 4;
        Out.ExorcismMeshes = Start + 8;
        Out.ExorcismField18 = Start + 24;
        Out.ExorcismWords = Start + 28;
        Out.Tail = Start + 48;
        return true;
    }
    if (Id == 10)
    {
        if (Size < 52) return false;
        Out.bGuard = true;
        Out.GuardPath = Start + 4;
        Out.GuardField98 = Start + 8;
        Out.GuardTrack = Start + 12;
        Out.GuardField90 = Start + 16;
        if (!KyaReadSwitchScriptOffsets(Data, Start + 20, Start + Size, Out.Switch)) return false;
        int64 Pos = Out.Switch.Tail;
        if (Start + Size - Pos < 4) return false;
        Out.OnOffCount = Pos;
        const uint32 Targets = U32(Pos);
        Out.OnOffCountValue = int32(Targets);
        Pos += 4;
        if (uint64(Targets) > uint64(Start + Size - Pos) / 28) return false;
        Out.OnOffEntries = Pos;
        Pos += int64(Targets) * 28;
        if (Start + Size - Pos < 32) return false;
        Out.OnOffCamera = Pos;
        Pos += 32;
        if (Pos != Start + Size) return false;
        Out.Tail = Pos;
        return true;
    }
    if (Id == 13)
    {
        if (Size < 40) return false;
        Out.bEscape = true;
        Out.EscapeCount = Start + 4;
        const uint32 Readers = U32(Out.EscapeCount);
        Out.EscapeCountValue = int32(Readers);
        Out.EscapeEntries = Start + 8;
        int64 Pos = Out.EscapeEntries;
        if (uint64(Readers) > uint64(Start + Size - Pos) / 4) return false;
        Pos += int64(Readers) * 4;
        if (Start + Size - Pos < 28) return false;
        Out.EscapeField88 = Pos;
        Out.EscapeFloats = Pos + 4;
        Out.EscapeFieldA0 = Pos + 24;
        Pos += 28;
        if (!KyaReadSwitchScriptOffsets(Data, Pos, Start + Size, Out.Switch)) return false;
        if (Out.Switch.Tail != Start + Size) return false;
        Out.Tail = Start + Size;
        return true;
    }
    if (Id == 22)
    {
        Out.bLost = true;
        if (!KyaReadSwitchScriptOffsets(Data, Start + 4, Start + Size, Out.Switch)) return false;
        if (Out.Switch.Tail != Start + Size) return false;
        Out.Tail = Start + Size;
        return true;
    }
    if (Id == 8)
    {
        if (Size < 48) return false;
        Out.bWatchDog = true;
        Out.Field80 = Start + 4;
        Out.TrackBehaviourId = Start + 8;
        if (!KyaReadSwitchScriptOffsets(Data, Start + 12, Start + Size, Out.Switch)) return false;
        int64 Pos = Out.Switch.Tail;
        if (Start + Size - Pos < 4) return false;
        Out.OnOffCount = Pos;
        const uint32 Targets = U32(Pos);
        Out.OnOffCountValue = int32(Targets);
        Pos += 4;
        if (uint64(Targets) > uint64(Start + Size - Pos) / 28) return false;
        Out.OnOffEntries = Pos;
        Pos += int64(Targets) * 28;
        if (Start + Size - Pos < 32) return false;
        Out.OnOffCamera = Pos;
        Pos += 32;
        if (Pos != Start + Size) return false;
        Out.Tail = Pos;
        return true;
    }
    if (Id == 21)
    {
        if (Size < 96) return false;
        Out.bSnipe = true;
        Out.SnipeField90 = Start + 4;
        Out.SnipeField94 = Start + 8;
        Out.SpotTrack = Start + 24;
        Out.SnipeTrack = Start + 28;
        Out.ShadowMaterial = Start + 32;
        Out.SnipeFieldAC = Start + 36;
        Out.SnipeFieldB0 = Start + 40;
        Out.SnipeFieldB4 = Start + 44;
        Out.NbSpot = Start + 48;
        Out.NbShadow = Start + 52;
        Out.RotationSpeed = Start + 56;
        Out.SnipeFieldB8 = Start + 60;
        if (!KyaReadSwitchScriptOffsets(Data, Start + 64, Start + Size, Out.Switch)) return false;
        if (Out.Switch.Tail != Start + Size) return false;
        Out.Tail = Start + Size;
        return true;
    }
    if (Id == 17 || Id == 18 || Id == 19)
    {
        if (Size < 20) return false;
        Out.bWeapon = Id == 17;
        Out.bWeaponStand = Id == 18;
        Out.bWeaponSnipe = Id == 19;
        Out.WeaponField90 = Start + 4;
        Out.WeaponField94 = Start + 8;
        Out.WeaponField98 = Start + 12;
        Out.WeaponCount = Start + 16;
        const uint32 Count = U32(Start + 16);
        Out.WeaponCountValue = int32(Count);
        int64 Pos = Start + 20;
        if (uint64(Count) > uint64(Size - 20) / 20) return false;
        Out.WeaponEntries = Pos;
        for (uint32 Index = 0; Index < Count; ++Index)
        {
            FKyaTrackSubObjOffsets SubObj;
            SubObj.Field0 = Pos;
            SubObj.AField0 = Pos + 4;
            SubObj.AField4 = Pos + 8;
            SubObj.AField8 = Pos + 12;
            SubObj.Field10 = Pos + 16;
            Out.SubObjs.Add(SubObj);
            Pos += 20;
        }
        const int64 Expected = Id == 18 ? Pos : (Id == 17 ? Pos + 8 : Pos + 4);
        if (Start + Size != Expected) return false;
        if (Id == 17)
        {
            Out.WeaponFieldF0 = Pos;
            Out.WeaponTargetField = Pos + 4;
        }
        else if (Id == 19) Out.WeaponFieldF0 = Pos;
        Out.Tail = Start + Size;
        return true;
    }
    return false;
}
struct FKyaCatchByTrapOffsets
{
    TArray<FKyaConditionedOffsets> Blocks;
    int64 Tail = 0;
};
inline bool KyaReadCatchByTrapOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size, FKyaCatchByTrapOffsets& Out)
{
    Out = FKyaCatchByTrapOffsets{};
    if (Start < 0 || Size < 4 || Start > Data.Num() || Size > Data.Num() - Start) return false;
    int64 Pos = Start;
    if (!KyaReadConditionedOffsets(Data, Start, Out.Blocks, Pos)) return false;
    if (Pos != Start + Size) return false;
    Out.Tail = Pos;
    return true;
}
struct FKyaHeroUnknownOffsets
{
    int64 Values = 0, Tail = 0;
};
inline bool KyaReadHeroUnknownOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size, FKyaHeroUnknownOffsets& Out)
{
    Out = FKyaHeroUnknownOffsets{};
    if (Start < 0 || Size != 28 || Start > Data.Num() || Size > Data.Num() - Start) return false;
    Out.Values = Start;
    Out.Tail = Start + 28;
    return true;
}
struct FKyaInventoryOffsets
{
    int64 Field0 = 0, Field8 = 0, Field10 = 0, PurchaseId = 0, Field28 = 0, MoneyCost = 0, Field2C = 0;
    int64 ObjectFlag = 0, ObjectId = 0, Field18 = 0, InitialBehaviourId = 0;
    bool bHasObject = false;
    int64 Tail = 0;
};
inline bool KyaReadInventoryOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size, FKyaInventoryOffsets& Out)
{
    Out = FKyaInventoryOffsets{};
    if (Start < 0 || Size < 8 || Start > Data.Num() || Size > Data.Num() - Start) return false;
    auto U32 = [&Data](int64 Offset) { const uint8* P = Data.GetData() + Offset; return uint32(P[0]) | uint32(P[1]) << 8 | uint32(P[2]) << 16 | uint32(P[3]) << 24; };
    int64 Pos = Start;
    Out.Field0 = Pos;
    const uint32 Present = U32(Pos);
    Pos += 4;
    const int64 ExpectedBase = Present == 0 ? 4 : 40;
    if (Size < ExpectedBase + 4) return false;
    if (Present != 0)
    {
        Out.Field8 = Pos; Pos += 8;
        Out.Field10 = Pos; Pos += 8;
        Out.PurchaseId = Pos; Pos += 4;
        Out.Field28 = Pos; Pos += 4;
        Out.MoneyCost = Pos; Pos += 4;
        Out.Field2C = Pos; Pos += 4;
        Out.ObjectFlag = Pos;
        Out.bHasObject = U32(Pos) != 0;
        Pos += 4;
        if (Out.bHasObject)
        {
            Out.ObjectId = Pos; Pos += 4;
            Out.Field18 = Pos; Pos += 8;
        }
    }
    Out.InitialBehaviourId = Pos;
    Pos += 4;
    if (Pos != Start + Size) return false;
    Out.Tail = Pos;
    return true;
}
struct FKyaProjectileBehaviourOffsets
{
    int64 BehaviourId = 0, Field0C = 0, MaterialId = 0, Speed = 0, Acceleration = 0, Field18 = 0, Field10 = 0, ActorRef = 0;
    int64 Tail = 0;
    bool bStand = false, bStraight = false, bPortable = false, bExcuse = false;
};
inline bool KyaReadProjectileBehaviourOffsets(int32 Id, TConstArrayView<uint8> Data, int64 Start, int64 Size, FKyaProjectileBehaviourOffsets& Out)
{
    Out = FKyaProjectileBehaviourOffsets{};
    if (Start < 0 || Size < 12 || Start > Data.Num() || Size > Data.Num() - Start) return false;
    Out.BehaviourId = Start;
    Out.Field0C = Start + 4;
    if (Id == 3)
    {
        if (Size != 12) return false;
        Out.bStand = true;
        Out.MaterialId = Start + 8;
    }
    else if (Id == 4)
    {
        if (Size != 20) return false;
        Out.bStraight = true;
        Out.Speed = Start + 8;
        Out.Acceleration = Start + 12;
        Out.Field18 = Start + 16;
    }
    else if (Id == 5)
    {
        if (Size != 12) return false;
        Out.bPortable = true;
        Out.Field10 = Start + 8;
    }
    else if (Id == 11)
    {
        if (Size != 12) return false;
        Out.bExcuse = true;
        Out.ActorRef = Start + 8;
    }
    else return false;
    Out.Tail = Start + Size;
    return true;
}
struct FKyaSwitchBehaviourOffsets
{
    int64 LeverRefA = 0, LeverRefB = 0;
    int64 MagicField08 = 0, MagicField10 = 0, MagicFxDigits = 0, MagicField14 = 0, MagicField4C = 0;
    int64 MagicField18 = 0, MagicField28 = 0, MagicField24 = 0, MagicField30 = 0;
    int64 OpenTextureId = 0, OpenMeshId = 0, MagicCamera = 0;
    int64 MultiField14 = 0, MultiField08 = 0, MultiField0C = 0;
    int64 TargetDisplacement = 0, TargetSpringStrength = 0, TargetDamping = 0;
    int64 Tail = 0;
    bool bLever = false, bMagic = false, bMultiCondition = false, bTarget = false;
};
inline bool KyaReadSwitchBehaviourOffsets(int32 Id, TConstArrayView<uint8> Data, int64 Start, int64 Size, FKyaSwitchBehaviourOffsets& Out)
{
    Out = FKyaSwitchBehaviourOffsets{};
    if (Start < 0 || Size < 8 || Start > Data.Num() || Size > Data.Num() - Start) return false;
    if (Id == 2)
    {
        if (Size != 8) return false;
        Out.bLever = true;
        Out.LeverRefA = Start;
        Out.LeverRefB = Start + 4;
    }
    else if (Id == 3)
    {
        if (Size != 76) return false;
        Out.bMagic = true;
        Out.MagicField08 = Start;
        Out.MagicField10 = Start + 4;
        Out.MagicFxDigits = Start + 8;
        Out.MagicField14 = Start + 12;
        Out.MagicField4C = Start + 16;
        Out.MagicField18 = Start + 20;
        Out.MagicField28 = Start + 24;
        Out.MagicField24 = Start + 28;
        Out.MagicField30 = Start + 32;
        Out.OpenTextureId = Start + 36;
        Out.OpenMeshId = Start + 40;
        Out.MagicCamera = Start + 44;
    }
    else if (Id == 5)
    {
        if (Size != 12) return false;
        Out.bMultiCondition = true;
        Out.MultiField14 = Start;
        Out.MultiField08 = Start + 4;
        Out.MultiField0C = Start + 8;
    }
    else if (Id == 6)
    {
        if (Size != 12) return false;
        Out.bTarget = true;
        Out.TargetDisplacement = Start;
        Out.TargetSpringStrength = Start + 4;
        Out.TargetDamping = Start + 8;
    }
    else return false;
    Out.Tail = Start + Size;
    return true;
}
struct FKyaPlatformBehaviourOffsets
{
    int64 SoundRef = 0, Field0C = 0, PathIndex = 0, Timing = 0, PathField4 = 0, PathTotalTime = 0;
    int64 Field40 = 0, AltModelId = 0, SegmentStartTime = 0, LoopDelay = 0;
    int64 Field18 = 0, MaterialId = 0, Field8 = 0, FieldC = 0, Field10 = 0;
    FKyaWindSwitchOffsets SlabSwitch;
    int64 SlabFloats = 0, SlabField20 = 0;
    int64 MasterPath = 0, MasterStream = 0, MasterFloats = 0, AnalogCount = 0, AnalogEntries = 0, AnalogCamera = 0;
    int32 AnalogCountValue = 0;
    int64 SelectorActorCount = 0, SelectorActorEntries = 0, SelectorSwitchCount = 0, SelectorSwitchEntries = 0, SelectorCamera = 0;
    int32 SelectorActorCountValue = 0, SelectorSwitchCountValue = 0;
    int64 Tail = 0;
    bool bStand = false, bSlave = false, bDestroyed = false, bSelectorSlave = false, bTrajectory = false;
    bool bSlab = false, bMaster = false, bSelectorMaster = false;
};
inline bool KyaReadPlatformBehaviourOffsets(int32 Id, TConstArrayView<uint8> Data, int64 Start, int64 Size, FKyaPlatformBehaviourOffsets& Out)
{
    Out = FKyaPlatformBehaviourOffsets{};
    if (Start < 0 || Size < 8 || Start > Data.Num() || Size > Data.Num() - Start) return false;
    auto U32 = [&Data](int64 Offset) { const uint8* P = Data.GetData() + Offset; return uint32(P[0]) | uint32(P[1]) << 8 | uint32(P[2]) << 16 | uint32(P[3]) << 24; };
    if (Id == 2)
    {
        if (Size != 24) return false;
        Out.bTrajectory = true;
        Out.PathIndex = Start;
        Out.Timing = Start + 4;
        Out.PathField4 = Start + 8;
        Out.PathTotalTime = Start + 12;
        Out.SegmentStartTime = Start + 16;
        Out.LoopDelay = Start + 20;
        Out.Tail = Start + 24;
        return true;
    }
    if (Id == 3)
    {
        if (Size < 56) return false;
        Out.bSlab = true;
        const uint32 Targets = U32(Start);
        Out.SlabSwitch.Count = int32(Targets);
        Out.SlabSwitch.Targets = Start + 4;
        if (Targets > uint64(Size - 36) / 28) return false;
        Out.SlabSwitch.Camera = Out.SlabSwitch.Targets + int64(Targets) * 28;
        if (Start + Size - Out.SlabSwitch.Camera < 52) return false;
        Out.SlabFloats = Out.SlabSwitch.Camera + 32;
        Out.SlabField20 = Out.SlabFloats + 16;
        if (Out.SlabField20 + 4 != Start + Size) return false;
        Out.Tail = Start + Size;
        return true;
    }
    if (Id == 5)
    {
        if (Size < 56) return false;
        Out.bMaster = true;
        Out.MasterPath = Start;
        Out.MasterStream = Start + 8;
        Out.MasterFloats = Start + 12;
        Out.AnalogCount = Start + 20;
        const uint32 Targets = U32(Out.AnalogCount);
        Out.AnalogCountValue = int32(Targets);
        Out.AnalogEntries = Start + 24;
        if (Targets > uint64(Size - 56) / 40) return false;
        Out.AnalogCamera = Out.AnalogEntries + int64(Targets) * 40;
        if (Out.AnalogCamera + 32 != Start + Size) return false;
        Out.Tail = Start + Size;
        return true;
    }
    if (Id == 8)
    {
        if (Size < 100) return false;
        Out.bSelectorMaster = true;
        Out.Field18 = Start;
        Out.MaterialId = Start + 4;
        Out.Field8 = Start + 8;
        Out.FieldC = Start + 12;
        Out.Field10 = Start + 16;
        Out.SoundRef = Start + 20;
        Out.SelectorActorCount = Start + 24;
        const uint32 Actors = U32(Out.SelectorActorCount);
        Out.SelectorActorCountValue = int32(Actors);
        Out.SelectorActorEntries = Start + 28;
        int64 Pos = Out.SelectorActorEntries + int64(Actors) * 4;
        if (Start + Size - Pos < 4) return false;
        Out.SelectorSwitchCount = Pos;
        const uint32 Switches = U32(Pos);
        Out.SelectorSwitchCountValue = int32(Switches);
        Out.SelectorSwitchEntries = Pos + 4;
        if (Switches > uint64(Start + Size - Out.SelectorSwitchEntries) / 16) return false;
        Out.SelectorCamera = Out.SelectorSwitchEntries + int64(Switches) * 16;
        if (Out.SelectorCamera + 32 != Start + Size) return false;
        Out.Tail = Start + Size;
        return true;
    }
    if (Id == 7)
    {
        if (Size != 8) return false;
        Out.bStand = true;
        Out.SoundRef = Start;
        Out.Field0C = Start + 4;
        Out.Tail = Start + 8;
        return true;
    }
    if (Id == 4)
    {
        if (Size != 8) return false;
        Out.bSlave = true;
        Out.PathIndex = Start;
        Out.Timing = Start + 4;
        Out.Tail = Start + 8;
        return true;
    }
    if (Id == 6)
    {
        if (Size != 24) return false;
        Out.bDestroyed = true;
        Out.PathIndex = Start;
        Out.Timing = Start + 4;
        Out.PathField4 = Start + 8;
        Out.PathTotalTime = Start + 12;
        Out.Field40 = Start + 16;
        Out.AltModelId = Start + 20;
        Out.Tail = Start + 24;
        return true;
    }
    if (Id == 9)
    {
        if (Size != 24) return false;
        Out.bSelectorSlave = true;
        Out.Field18 = Start;
        Out.MaterialId = Start + 4;
        Out.Field8 = Start + 8;
        Out.FieldC = Start + 12;
        Out.Field10 = Start + 16;
        Out.SoundRef = Start + 20;
        Out.Tail = Start + 24;
        return true;
    }
    return false;
}
/** One light record from the THGL (LGHT) archive. */
struct FKyaLightOffsets
{
    int64 Record = 0;
    int64 End = 0;
    int64 Ambient = 0, Color = 0, Field20 = 0, Multiplier = 0;
    int64 Position = 0, Falloff = 0, Direction = 0, Fov = 0, Trailing = 0;
    int32 TrailingCount = 0;
    int32 LightType = 0, ShapeType = 0;
};
inline bool KyaReadLightOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size, TArray<FKyaLightOffsets>& Out)
{
    Out.Reset();
    if (Start < 0 || Size < 8 || Start > Data.Num() || Size > Data.Num() - Start) return false;
    const int64 End = Start + Size;
    auto SafeU32 = [&Data](int64 Offset, uint32& Value) {
        if (Offset < 0 || Offset > Data.Num() || uint64(Offset) + 4 > uint64(Data.Num())) return false;
        const uint8* P = Data.GetData() + Offset;
        Value = uint32(P[0]) | uint32(P[1]) << 8 | uint32(P[2]) << 16 | uint32(P[3]) << 24;
        return true;
    };
    uint32 Count = 0;
    if (!SafeU32(Start, Count)) return false;
    if (Count == 0 || Count > 4096) return false;
    const int64 Table = Start + 4;
    if (uint64(Count) * 4 > uint64(End - Table)) return false;
    for (uint32 Index = 0; Index < Count; ++Index)
    {
        uint32 EntryOffset = 0;
        if (!SafeU32(Table + int64(Index) * 4, EntryOffset)) return false;
        const int64 Offset = Table + int64(EntryOffset);
        if (Offset < Table + int64(Count) * 4 || Offset > End) return false;
        FKyaLightOffsets Light;
        Light.Record = Offset;
        // Offsets are relative to the table start; the record ends where the next begins.
        int64 RecordEnd = End;
        if (Index + 1 < Count)
        {
            uint32 NextEntryOffset = 0;
            if (SafeU32(Table + int64(Index + 1) * 4, NextEntryOffset))
            {
                const int64 NextOffset = Table + int64(NextEntryOffset);
                if (NextOffset > Offset && NextOffset < RecordEnd) RecordEnd = NextOffset;
            }
        }
        Light.End = RecordEnd;
        if (RecordEnd - Offset < 24) return false;
        uint32 TypeWord = 0, ShapeWord = 0;
        if (!SafeU32(Offset, TypeWord) || !SafeU32(Offset + 4, ShapeWord)) return false;
        Light.LightType = int32(TypeWord);
        Light.ShapeType = int32(ShapeWord);
        Light.Ambient = Offset + 8;
        Light.Color = Offset + 12;
        Light.Field20 = Offset + 16;
        Light.Multiplier = Offset + 20;
        int64 Pos = Offset + 24;
        if (RecordEnd - Pos >= 20)
        {
            Light.Position = Pos;
            Light.Falloff = Pos + 12;
            Pos += 20;
        }
        if ((Light.LightType != 2) && (Light.ShapeType == 1 || Light.ShapeType == 2 || Light.ShapeType == 4) && RecordEnd - Pos >= 12)
        {
            Light.Direction = Pos;
            Pos += 12;
        }
        if (Light.ShapeType == 4 && RecordEnd - Pos >= 8)
        {
            Light.Fov = Pos;
            Pos += 8;
        }
        Light.Trailing = Pos;
        Light.TrailingCount = int32(FMath::Max<int64>(0, (RecordEnd - Pos) / 4));
        Out.Add(Light);
    }
    return true;
}
struct FKyaScenarioConditionOffsets { int64 Count = 0, Entries = 0; int32 CountValue = 0; };
inline bool KyaReadScenarioConditionOffsets(TConstArrayView<uint8> Data, int64& Pos, int64 End, FKyaScenarioConditionOffsets& Out)
{
    Out = FKyaScenarioConditionOffsets{};
    auto U32 = [&Data](int64 Offset) { const uint8* P = Data.GetData() + Offset; return uint32(P[0]) | uint32(P[1]) << 8 | uint32(P[2]) << 16 | uint32(P[3]) << 24; };
    if (Pos < 0 || End > Data.Num() || End - Pos < 4) return false;
    Out.Count = Pos;
    const uint32 Count = U32(Pos);
    Out.CountValue = int32(Count);
    Pos += 4;
    if (uint64(Count) > uint64(End - Pos) / 16) return false;
    Out.Entries = Pos;
    Pos += int64(Count) * 16;
    return true;
}
struct FKyaCompanionAlertOffsets
{
    FKyaScenarioConditionOffsets Conditions;
    int64 Flags = 0, Field08 = 0, Field10 = 0, Field14 = 0, Field18 = 0;
    int64 ActorRef = 0, Vector20 = 0, CameraRef = 0;
    int64 ListCount = 0, ListEntries = 0;
    int32 ListCountValue = 0;
    int64 LevelStringKey = 0, Field50 = 0, Field58 = 0, Field5C = 0;
    FKyaWindSwitchOffsets Switch;
    int64 Conditioned = 0;
    TArray<FKyaConditionedOffsets> Blocks;
    int64 Tail = 0;
};
struct FKyaCompanionBehaviourOffsets
{
    int64 Count = 0, Entries = 0, Tail = 0;
    int32 CountValue = 0;
    TArray<FKyaCompanionAlertOffsets> Alerts;
};
inline bool KyaReadCompanionBehaviourOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size, FKyaCompanionBehaviourOffsets& Out)
{
    Out = FKyaCompanionBehaviourOffsets{};
    if (Start < 0 || Size < 4 || Start > Data.Num() || Size > Data.Num() - Start) return false;
    const int64 End = Start + Size;
    auto U32 = [&Data](int64 Offset) { const uint8* P = Data.GetData() + Offset; return uint32(P[0]) | uint32(P[1]) << 8 | uint32(P[2]) << 16 | uint32(P[3]) << 24; };
    FKyaCompanionBehaviourOffsets Result;
    Result.Count = Start;
    Result.Entries = Start + 4;
    const uint32 Count = U32(Start);
    Result.CountValue = int32(Count);
    int64 Pos = Start + 4;
    for (uint32 Index = 0; Index < Count; ++Index)
    {
        FKyaCompanionAlertOffsets Alert;
        if (!KyaReadScenarioConditionOffsets(Data, Pos, End, Alert.Conditions)) return false;
        if (End - Pos < 20) return false;
        Alert.Flags = Pos;
        Alert.Field08 = Pos + 4;
        Alert.Field10 = Pos + 8;
        Alert.Field14 = Pos + 12;
        Alert.Field18 = Pos + 16;
        Pos += 20;
        if (End - Pos < 4) return false;
        Alert.ActorRef = Pos;
        Pos += 4;
        if (End - Pos < 12) return false;
        Alert.Vector20 = Pos;
        Pos += 12;
        if (End - Pos < 4) return false;
        Alert.CameraRef = Pos;
        Pos += 4;
        if (End - Pos < 4) return false;
        Alert.ListCount = Pos;
        const uint32 Items = U32(Pos);
        Alert.ListCountValue = int32(Items);
        Pos += 4;
        if (uint64(Items) > uint64(End - Pos) / 4) return false;
        Alert.ListEntries = Pos;
        Pos += int64(Items) * 4;
        if (End - Pos < 20) return false;
        Alert.LevelStringKey = Pos;
        Alert.Field50 = Pos + 8;
        Alert.Field58 = Pos + 12;
        Alert.Field5C = Pos + 16;
        Pos += 20;
        if (End - Pos < 4) return false;
        const uint32 SwitchTargets = U32(Pos);
        Alert.Switch.Count = int32(SwitchTargets);
        Alert.Switch.Targets = Pos + 4;
        if (SwitchTargets > uint64(End - Pos - 4) / 28) return false;
        Alert.Switch.Camera = Alert.Switch.Targets + int64(SwitchTargets) * 28;
        if (End - Alert.Switch.Camera < 32) return false;
        Pos = Alert.Switch.Camera + 32;
        Alert.Conditioned = Pos;
        if (!KyaReadConditionedOffsets(Data, Pos, Alert.Blocks, Pos)) return false;
        Alert.Tail = Pos;
        Result.Alerts.Add(MoveTemp(Alert));
    }
    Result.Tail = Pos;
    Out = Result;
    return true;
}
struct FKyaBoomyOffsets
{
    int64 Weight = 0, Vision = 0, Sound2B8 = 0, Sound2BC = 0, Field744 = 0;
    int64 TailWidthAxis = 0, Field740 = 0, TailSegments = 0;
    int64 Particle3D0 = 0, Particle3D4 = 0, Particle3DC = 0, Field3CC = 0;
    int64 Texture = 0, Mesh = 0, Camera = 0;
    int64 CameraFov = 0, CameraPosition = 0, CameraLookAt = 0, CameraUp = 0;
    int64 CameraSector = 0, CameraField08 = 0, CameraFlags = 0, CameraTargetOffset = 0;
    int64 SwitchActor = 0, SwitchListA = 0, SwitchListB = 0, SwitchValues = 0;
    int32 SwitchCountA = 0, SwitchCountB = 0;
    bool bHasSwitch = false;
    int64 Conditions = 0, Config = 0, ConfigOptional = 0, ConfigAngles = 0, Extra = 0;
    int32 ConditionCount = 0, ConfigMode = 0, ExtraCount = 0;
    int64 TypeCount = 0, TypeEntries = 0;
    int32 TypeCountValue = 0;
    int64 Tail = 0;
};
inline bool KyaReadBoomyOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size, FKyaBoomyOffsets& Out)
{
    Out = FKyaBoomyOffsets{};
    if (Start < 0 || Size < 4 || Start > Data.Num() || Size > Data.Num() - Start) return false;
    const int64 End = Start + Size;
    auto U32 = [&Data](int64 Offset) { const uint8* P = Data.GetData() + Offset; return uint32(P[0]) | uint32(P[1]) << 8 | uint32(P[2]) << 16 | uint32(P[3]) << 24; };
    FKyaBoomyOffsets Result;
    Result.Weight = Start;
    Result.Vision = Start + 4;
    Result.Sound2B8 = Start + 24;
    Result.Sound2BC = Start + 28;
    Result.Field744 = Start + 32;
    Result.TailWidthAxis = Start + 36;
    Result.Field740 = Start + 40;
    Result.TailSegments = Start + 44;
    Result.Particle3D0 = Start + 48;
    Result.Particle3D4 = Start + 52;
    Result.Particle3DC = Start + 56;
    Result.Field3CC = Start + 60;
    Result.Texture = Start + 64;
    Result.Mesh = Start + 68;
    Result.Camera = Start + 72;
    int64 Pos = Start + 72;
    if (End - Pos < 64) return false;
    Result.CameraFov = Pos;
    Result.CameraPosition = Pos + 4;
    Result.CameraLookAt = Pos + 16;
    Result.CameraUp = Pos + 28;
    Result.CameraSector = Pos + 40;
    Result.CameraField08 = Pos + 44;
    Result.CameraFlags = Pos + 48;
    Result.CameraTargetOffset = Pos + 52;
    Pos += 64;
    if (U32(Result.CameraFlags) & 2)
    {
        Result.bHasSwitch = true;
        if (End - Pos < 4) return false;
        Result.SwitchActor = Pos;
        Pos += 4;
        for (int32 List = 0; List < 2; ++List)
        {
            if (End - Pos < 4) return false;
            const uint32 Count = U32(Pos);
            if (List == 0) { Result.SwitchListA = Pos + 4; Result.SwitchCountA = int32(Count); }
            else { Result.SwitchListB = Pos + 4; Result.SwitchCountB = int32(Count); }
            Pos += 4;
            if (uint64(Count) > uint64(End - Pos) / 4) return false;
            Pos += int64(Count) * 4;
        }
        if (End - Pos < 20) return false;
        Result.SwitchValues = Pos;
        Pos += 20;
    }
    if (End - Pos < 4) return false;
    Result.Conditions = Pos;
    const uint32 Conditions = U32(Pos);
    Result.ConditionCount = int32(Conditions);
    Pos += 4;
    if (uint64(Conditions) > uint64(End - Pos) / 16) return false;
    Pos += int64(Conditions) * 16;
    if (End - Pos < 20) return false;
    Result.Config = Pos;
    const uint32 Mode = U32(Pos + 16);
    Result.ConfigMode = int32(Mode);
    if (Mode > 1) return false;
    Pos += 20;
    if (Mode == 1)
    {
        if (End - Pos < 20) return false;
        Result.ConfigOptional = Pos;
        Pos += 20;
    }
    if (End - Pos < 36) return false;
    Result.ConfigAngles = Pos;
    Pos += 36;
    if (End - Pos < 4) return false;
    Result.Extra = Pos;
    const uint32 ExtraCount = U32(Pos);
    Result.ExtraCount = int32(ExtraCount);
    Pos += 4;
    if (ExtraCount == 1)
    {
        if (End - Pos < 48) return false;
        Pos += 48;
    }
    if (End - Pos < 4) return false;
    Result.TypeCount = Pos;
    const uint32 Types = U32(Pos);
    Result.TypeCountValue = int32(Types);
    Pos += 4;
    if (uint64(Types) > uint64(End - Pos) / 16) return false;
    Result.TypeEntries = Pos;
    Pos += int64(Types) * 16;
    Result.Tail = Pos;
    Out = Result;
    return true;
}
struct FKyaAtonOffsets
{
    int64 WeightB = 0, Autonomous = 0, Fields354 = 0, EffectType = 0, Texture = 0, Mesh = 0;
    int64 Trail = 0, PathDelay = 0, PathCount = 0, PathEntries = 0;
    int32 PathCountValue = 0;
    int64 AddOn = 0, AddOnField18 = 0, OtherCount = 0, OtherEntries = 0;
    int64 AtonCount = 0, AtonEntries = 0, AddOnField24 = 0, Tail = 0;
    int32 OtherCountValue = 0, AtonCountValue = 0;
    TArray<int64> OtherOffsets, AtonOffsets;
};
inline bool KyaReadAtonOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size, FKyaAtonOffsets& Out)
{
    Out = FKyaAtonOffsets{};
    if (Start < 0 || Size < 76 || Start > Data.Num() || Size > Data.Num() - Start) return false;
    const int64 End = Start + Size;
    auto U32 = [&Data](int64 Offset) { const uint8* P = Data.GetData() + Offset; return uint32(P[0]) | uint32(P[1]) << 8 | uint32(P[2]) << 16 | uint32(P[3]) << 24; };
    FKyaAtonOffsets Result;
    Result.WeightB = Start;
    Result.Autonomous = Start + 4;
    Result.Fields354 = Start + 20;
    Result.EffectType = Start + 36;
    Result.Texture = Start + 40;
    Result.Mesh = Start + 44;
    Result.Trail = Start + 48;
    Result.PathDelay = Start + 52;
    Result.PathCount = Start + 56;
    const uint32 Paths = U32(Result.PathCount);
    Result.PathCountValue = int32(Paths);
    Result.PathEntries = Start + 60;
    int64 Pos = Start + 60;
    if (uint64(Paths) > uint64(End - Pos) / 4) return false;
    Pos += int64(Paths) * 4;
    if (End - Pos < 8) return false;
    Result.AddOn = Pos;
    Result.AddOnField18 = Pos;
    Result.OtherCount = Pos + 4;
    const uint32 Others = U32(Result.OtherCount);
    Result.OtherCountValue = int32(Others);
    Result.OtherEntries = Pos + 8;
    Pos += 8;
    for (uint32 Index = 0; Index < Others; ++Index)
    {
        if (End - Pos < 8) return false;
        const uint32 Cinematics = U32(Pos + 4);
        Result.OtherOffsets.Add(Pos);
        Pos += 8;
        if (uint64(Cinematics) > uint64(End - Pos) / 4) return false;
        Pos += int64(Cinematics) * 4;
    }
    if (End - Pos < 4) return false;
    Result.AtonCount = Pos;
    const uint32 Atons = U32(Pos);
    Result.AtonCountValue = int32(Atons);
    Result.AtonEntries = Pos + 4;
    Pos += 4;
    for (uint32 Index = 0; Index < Atons; ++Index)
    {
        if (End - Pos < 12) return false;
        const uint32 Cinematics = U32(Pos + 8);
        Result.AtonOffsets.Add(Pos);
        Pos += 12;
        if (uint64(Cinematics) > uint64(End - Pos) / 4) return false;
        Pos += int64(Cinematics) * 4;
    }
    if (End - Pos < 4) return false;
    Result.AddOnField24 = Pos;
    Result.Tail = Pos + 4;
    Out = Result;
    return true;
}
struct FKyaCompanionOffsets { int64 WeightB = 0, Autonomous = 0, Field358 = 0, Tail = 0; };
inline bool KyaReadCompanionOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size, FKyaCompanionOffsets& Out)
{
    Out = FKyaCompanionOffsets{};
    if (Start < 0 || Size < 32 || Start > Data.Num() || Size > Data.Num() - Start) return false;
    Out.WeightB = Start;
    Out.Autonomous = Start + 4;
    Out.Field358 = Start + 20;
    Out.Tail = Start + 32;
    return true;
}
struct FKyaBridgeOffsets
{
    int64 ActorCount = 0, ActorEntries = 0, WaypointA = 0, WaypointB = 0, Floats330 = 0;
    int64 Field344 = 0, ConfigFlags = 0, Field348 = 0, StepCollisionScale = 0, Tail = 0;
    int32 ActorCountValue = 0;
};
inline bool KyaReadBridgeOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size, FKyaBridgeOffsets& Out)
{
    Out = FKyaBridgeOffsets{};
    if (Start < 0 || Size < 48 || Start > Data.Num() || Size > Data.Num() - Start) return false;
    const int64 End = Start + Size;
    auto U32 = [&Data](int64 Offset) { const uint8* P = Data.GetData() + Offset; return uint32(P[0]) | uint32(P[1]) << 8 | uint32(P[2]) << 16 | uint32(P[3]) << 24; };
    FKyaBridgeOffsets Result;
    Result.ActorCount = Start;
    const uint32 Actors = U32(Start);
    Result.ActorCountValue = int32(Actors);
    Result.ActorEntries = Start + 4;
    int64 Pos = Start + 4;
    if (uint64(Actors) > uint64(End - Pos) / 4) return false;
    Pos += int64(Actors) * 4;
    if (End - Pos < 40) return false;
    Result.WaypointA = Pos;
    Result.WaypointB = Pos + 4;
    Result.Floats330 = Pos + 8;
    Result.Field344 = Pos + 28;
    Result.ConfigFlags = Pos + 32;
    Result.Field348 = Pos + 36;
    Result.StepCollisionScale = Pos + 40;
    Pos += 44;
    Result.Tail = Pos;
    Out = Result;
    return true;
}
struct FKyaBridgeBasicOffsets { int64 Direction = 0, Fields40 = 0, Tail = 0; };
inline bool KyaReadBridgeBasicOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size, FKyaBridgeBasicOffsets& Out)
{
    Out = FKyaBridgeBasicOffsets{};
    if (Start < 0 || Size != 32 || Start > Data.Num() || Size > Data.Num() - Start) return false;
    Out.Direction = Start;
    Out.Fields40 = Start + 12;
    Out.Tail = Start + 32;
    return true;
}
struct FKyaBridgeWindOffsets { int64 Fields40 = 0, Tail = 0; };
inline bool KyaReadBridgeWindOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size, FKyaBridgeWindOffsets& Out)
{
    Out = FKyaBridgeWindOffsets{};
    if (Start < 0 || Size != 20 || Start > Data.Num() || Size > Data.Num() - Start) return false;
    Out.Fields40 = Start;
    Out.Tail = Start + 20;
    return true;
}
struct FKyaGravityOffsets { int64 WeightB = 0, Autonomous = 0, Tail = 0; };
inline bool KyaReadGravityOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size, FKyaGravityOffsets& Out)
{
    Out = FKyaGravityOffsets{};
    if (Start < 0 || Size < 20 || Start > Data.Num() || Size > Data.Num() - Start) return false;
    Out.WeightB = Start;
    Out.Autonomous = Start + 4;
    Out.Tail = Start + 20;
    return true;
}
struct FKyaGravityBellOffsets { int64 Sound = 0, FieldC = 0; FKyaWindSwitchOffsets Switch; int64 Tail = 0; };
inline bool KyaReadGravityBellOffsets(TConstArrayView<uint8> Data, int64 Start, int64 End, FKyaGravityBellOffsets& Out)
{
    Out = FKyaGravityBellOffsets{};
    if (Start < 0 || End > Data.Num() || End - Start < 8) return false;
    auto U32 = [&Data](int64 Offset) { const uint8* P = Data.GetData() + Offset; return uint32(P[0]) | uint32(P[1]) << 8 | uint32(P[2]) << 16 | uint32(P[3]) << 24; };
    Out.Sound = Start;
    Out.FieldC = Start + 4;
    int64 Pos = Start + 8;
    if (End - Pos < 4) return false;
    const uint32 Count = U32(Pos);
    Out.Switch.Count = int32(Count);
    Out.Switch.Targets = Pos + 4;
    if (Count > uint64(End - Pos - 4) / 28) return false;
    Out.Switch.Camera = Out.Switch.Targets + int64(Count) * 28;
    if (End - Out.Switch.Camera < 32) return false;
    Out.Tail = Out.Switch.Camera + 32;
    return true;
}
struct FKyaNativCmdOffsets
{
    int64 ActorCount = 0, ActorEntries = 0, Field164 = 0, AddOns = 0, Tail = 0;
    int32 ActorCountValue = 0;
    TArray<int64> AddOnCounts, SubObjOffsets;
    TArray<int32> AddOnCountValues;
};
inline bool KyaReadNativCmdOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size, FKyaNativCmdOffsets& Out)
{
    Out = FKyaNativCmdOffsets{};
    if (Start < 0 || Size < 24 || Start > Data.Num() || Size > Data.Num() - Start) return false;
    const int64 End = Start + Size;
    auto U32 = [&Data](int64 Offset) { const uint8* P = Data.GetData() + Offset; return uint32(P[0]) | uint32(P[1]) << 8 | uint32(P[2]) << 16 | uint32(P[3]) << 24; };
    FKyaNativCmdOffsets Result;
    Result.ActorCount = Start;
    const uint32 Actors = U32(Start);
    Result.ActorCountValue = int32(Actors);
    Result.ActorEntries = Start + 4;
    int64 Pos = Start + 4;
    if (uint64(Actors) > uint64(End - Pos) / 4) return false;
    Pos += int64(Actors) * 4;
    if (End - Pos < 4) return false;
    Result.Field164 = Pos;
    Pos += 4;
    Result.AddOns = Pos;
    for (int32 AddOn = 0; AddOn < 4; ++AddOn)
    {
        if (End - Pos < 4) return false;
        const int64 CountOffset = Pos;
        const uint32 SubObjs = U32(Pos);
        Pos += 4;
        Result.AddOnCounts.Add(CountOffset);
        Result.AddOnCountValues.Add(int32(SubObjs));
        for (uint32 Sub = 0; Sub < SubObjs; ++Sub)
        {
            if (End - Pos < 8) return false;
            const uint32 Cinematics = U32(Pos + 4);
            Result.SubObjOffsets.Add(Pos);
            Pos += 8;
            if (uint64(Cinematics) > uint64(End - Pos) / 4) return false;
            Pos += int64(Cinematics) * 4;
        }
    }
    Result.Tail = Pos;
    Out = Result;
    return true;
}
struct FKyaRuneOffsets { int64 Cinematic = 0, TriggerDistance = 0, ScenVar = 0, Fx = 0, MaterialA = 0, MaterialB = 0, Tail = 0; };
inline bool KyaReadRuneOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size, FKyaRuneOffsets& Out)
{
    Out = FKyaRuneOffsets{};
    if (Start < 0 || Size < 24 || Start > Data.Num() || Size > Data.Num() - Start) return false;
    Out.Cinematic = Start;
    Out.TriggerDistance = Start + 4;
    Out.ScenVar = Start + 8;
    Out.Fx = Start + 12;
    Out.MaterialA = Start + 16;
    Out.MaterialB = Start + 20;
    Out.Tail = Start + 24;
    return true;
}
struct FKyaBonusFountainOffsets { int64 AddOn = 0, SubObjMode = 0, SubObjFloats = 0, Tail = 0; int32 SubObjModeValue = 0; };
inline bool KyaReadBonusFountainOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size, FKyaBonusFountainOffsets& Out)
{
    Out = FKyaBonusFountainOffsets{};
    if (Start < 0 || Size < 24 || Start > Data.Num() || Size > Data.Num() - Start) return false;
    const int64 End = Start + Size;
    auto U32 = [&Data](int64 Offset) { const uint8* P = Data.GetData() + Offset; return uint32(P[0]) | uint32(P[1]) << 8 | uint32(P[2]) << 16 | uint32(P[3]) << 24; };
    Out.AddOn = Start;
    Out.SubObjMode = Start + 20;
    Out.SubObjModeValue = int32(U32(Out.SubObjMode));
    int64 Pos = Start + 24;
    if (Out.SubObjModeValue == 1)
    {
        if (End - Pos < 20) return false;
        Out.SubObjFloats = Pos;
        Pos += 20;
    }
    Out.Tail = Pos;
    return true;
}
struct FKyaRopeOffsets
{
    int64 Path = 0, Scale = 0, Field17C = 0, Material = 0, Color = 0, Sound = 0, Field194 = 0;
    int64 ActorA = 0, ActorB = 0;
    FKyaWindSwitchOffsets Switch;
    int64 Flags = 0, Tail = 0;
};
inline bool KyaReadRopeOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size, FKyaRopeOffsets& Out)
{
    Out = FKyaRopeOffsets{};
    if (Start < 0 || Size < 76 || Start > Data.Num() || Size > Data.Num() - Start) return false;
    const int64 End = Start + Size;
    auto U32 = [&Data](int64 Offset) { const uint8* P = Data.GetData() + Offset; return uint32(P[0]) | uint32(P[1]) << 8 | uint32(P[2]) << 16 | uint32(P[3]) << 24; };
    FKyaRopeOffsets Result;
    Result.Path = Start;
    Result.Scale = Start + 4;
    Result.Field17C = Start + 8;
    Result.Material = Start + 12;
    Result.Color = Start + 16;
    Result.Sound = Start + 20;
    Result.Field194 = Start + 24;
    Result.ActorA = Start + 28;
    Result.ActorB = Start + 32;
    int64 Pos = Start + 36;
    const uint32 Count = U32(Pos);
    Result.Switch.Count = int32(Count);
    Result.Switch.Targets = Pos + 4;
    if (Count > uint64(End - Pos - 4) / 28) return false;
    Result.Switch.Camera = Result.Switch.Targets + int64(Count) * 28;
    if (End - Result.Switch.Camera < 36) return false;
    Result.Flags = Result.Switch.Camera + 32;
    Result.Tail = Result.Flags + 4;
    Out = Result;
    return true;
}
struct FKyaStillerOffsets
{
    int64 Vision = 0, Field1D0 = 0, Path = 0, Field1E0 = 0, LifeMax = 0, Floats1EC = 0;
    int64 Field1F8 = 0, ActorCount = 0, ActorEntries = 0, Field200 = 0;
    FKyaWindSwitchOffsets Switches[2];
    int64 Field214 = 0, AddOn = 0, SubObjMode = 0, SubObjFloats = 0, Tail = 0;
    int32 ActorCountValue = 0, SubObjModeValue = 0;
};
inline bool KyaReadStillerOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size, FKyaStillerOffsets& Out)
{
    Out = FKyaStillerOffsets{};
    if (Start < 0 || Size < 172 || Start > Data.Num() || Size > Data.Num() - Start) return false;
    const int64 End = Start + Size;
    auto U32 = [&Data](int64 Offset) { const uint8* P = Data.GetData() + Offset; return uint32(P[0]) | uint32(P[1]) << 8 | uint32(P[2]) << 16 | uint32(P[3]) << 24; };
    FKyaStillerOffsets Result;
    Result.Vision = Start;
    Result.Field1D0 = Start + 20;
    Result.Path = Start + 36;
    Result.Field1E0 = Start + 40;
    Result.LifeMax = Start + 44;
    Result.Floats1EC = Start + 48;
    Result.Field1F8 = Start + 60;
    int64 Pos = Start + 64;
    Result.ActorCount = Pos;
    const uint32 Actors = U32(Pos);
    Result.ActorCountValue = int32(Actors);
    Pos += 4;
    if (uint64(Actors) > uint64(End - Pos) / 4) return false;
    Result.ActorEntries = Pos;
    Pos += int64(Actors) * 4;
    if (End - Pos < 4) return false;
    Result.Field200 = Pos;
    Pos += 4;
    for (int32 Switch = 0; Switch < 2; ++Switch)
    {
        if (End - Pos < 4) return false;
        FKyaWindSwitchOffsets& S = Result.Switches[Switch];
        const uint32 Count = U32(Pos);
        S.Count = int32(Count);
        S.Targets = Pos + 4;
        if (Count > uint64(End - Pos - 4) / 28) return false;
        S.Camera = S.Targets + int64(Count) * 28;
        if (End - S.Camera < 32) return false;
        Pos = S.Camera + 32;
    }
    if (End - Pos < 4) return false;
    Result.Field214 = Pos;
    Pos += 4;
    if (End - Pos < 24) return false;
    Result.AddOn = Pos;
    Pos += 20;
    Result.SubObjMode = Pos;
    Result.SubObjModeValue = int32(U32(Pos));
    Pos += 4;
    if (Result.SubObjModeValue == 1)
    {
        if (End - Pos < 20) return false;
        Result.SubObjFloats = Pos;
        Pos += 20;
    }
    Result.Tail = Pos;
    Out = Result;
    return true;
}
struct FKyaAmbreOffsets
{
    int64 Material = 0, EffectType = 0, Texture = 0, Mesh = 0, Field1D0 = 0;
    FKyaWindSwitchOffsets Switch;
    int64 Tail = 0;
};
inline bool KyaReadAmbreOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size, FKyaAmbreOffsets& Out)
{
    Out = FKyaAmbreOffsets{};
    if (Start < 0 || Size < 56 || Start > Data.Num() || Size > Data.Num() - Start) return false;
    const int64 End = Start + Size;
    auto U32 = [&Data](int64 Offset) { const uint8* P = Data.GetData() + Offset; return uint32(P[0]) | uint32(P[1]) << 8 | uint32(P[2]) << 16 | uint32(P[3]) << 24; };
    FKyaAmbreOffsets Result;
    Result.Material = Start;
    Result.EffectType = Start + 4;
    Result.Texture = Start + 8;
    Result.Mesh = Start + 12;
    Result.Field1D0 = Start + 16;
    int64 Pos = Start + 20;
    const uint32 Count = U32(Pos);
    Result.Switch.Count = int32(Count);
    Result.Switch.Targets = Pos + 4;
    if (Count > uint64(End - Pos - 4) / 28) return false;
    Result.Switch.Camera = Result.Switch.Targets + int64(Count) * 28;
    if (End - Result.Switch.Camera < 32) return false;
    Result.Tail = Result.Switch.Camera + 32;
    Out = Result;
    return true;
}
struct FKyaShootOffsets
{
    int64 WeightB = 0, Autonomous = 0, Field350 = 0, Field354 = 0, Texture = 0, Mesh = 0;
    int64 Floats3C0 = 0, Field3D0 = 0, Floats3D4 = 0, Fields3E8 = 0;
    int64 AddOn = 0, SubObjMode = 0, SubObjFloats = 0, Tail = 0;
    int32 SubObjModeValue = 0;
};
inline bool KyaReadShootOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size, FKyaShootOffsets& Out)
{
    Out = FKyaShootOffsets{};
    if (Start < 0 || Size < 112 || Start > Data.Num() || Size > Data.Num() - Start) return false;
    const int64 End = Start + Size;
    auto U32 = [&Data](int64 Offset) { const uint8* P = Data.GetData() + Offset; return uint32(P[0]) | uint32(P[1]) << 8 | uint32(P[2]) << 16 | uint32(P[3]) << 24; };
    FKyaShootOffsets Result;
    Result.WeightB = Start; Result.Autonomous = Start + 4;
    Result.Field350 = Start + 20; Result.Field354 = Start + 24;
    Result.Texture = Start + 28; Result.Mesh = Start + 32;
    Result.Floats3C0 = Start + 36;
    Result.Field3D0 = Start + 52;
    Result.Floats3D4 = Start + 56;
    Result.Fields3E8 = Start + 72;
    int64 Pos = Start + 88;
    Result.AddOn = Pos;
    Pos += 20;
    Result.SubObjMode = Pos;
    Result.SubObjModeValue = int32(U32(Pos));
    Pos += 4;
    if (Result.SubObjModeValue == 1)
    {
        if (End - Pos < 20) return false;
        Result.SubObjFloats = Pos;
        Pos += 20;
    }
    Result.Tail = Pos;
    Out = Result;
    return true;
}
struct FKyaShootFireOffsets
{
    int64 Field2B0 = 0, Field8 = 0, Field2B4 = 0, Count = 0, Entries = 0, Tail = 0;
    int32 CountValue = 0;
};
inline bool KyaReadShootFireOffsets(TConstArrayView<uint8> Data, int64 Start, int64 End, FKyaShootFireOffsets& Out)
{
    Out = FKyaShootFireOffsets{};
    if (Start < 0 || End > Data.Num() || End - Start < 16) return false;
    auto U32 = [&Data](int64 Offset) { const uint8* P = Data.GetData() + Offset; return uint32(P[0]) | uint32(P[1]) << 8 | uint32(P[2]) << 16 | uint32(P[3]) << 24; };
    Out.Field2B0 = Start;
    Out.Field8 = Start + 4;
    Out.Field2B4 = Start + 8;
    Out.Count = Start + 12;
    const uint32 Count = U32(Out.Count);
    Out.CountValue = int32(Count);
    Out.Entries = Start + 16;
    if (uint64(Count) > uint64(End - Out.Entries) / 4) return false;
    Out.Tail = Out.Entries + int64(Count) * 4;
    return true;
}
struct FKyaShootWaveOffsets { int64 Fields = 0, Tail = 0; };
inline bool KyaReadShootWaveOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size, FKyaShootWaveOffsets& Out)
{
    Out = FKyaShootWaveOffsets{};
    if (Start < 0 || Size != 28 || Start > Data.Num() || Size > Data.Num() - Start) return false;
    Out.Fields = Start;
    Out.Tail = Start + 28;
    return true;
}
struct FKyaFogOffsets
{
    int64 Count = 0, Entries = 0, Tail = 0;
    int32 CountValue = 0;
};
inline bool KyaReadFogOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size, FKyaFogOffsets& Out)
{
    Out = FKyaFogOffsets{};
    if (Start < 0 || Size < 4 || Start > Data.Num() || Size > Data.Num() - Start) return false;
    auto U32 = [&Data](int64 Offset) { const uint8* P = Data.GetData() + Offset; return uint32(P[0]) | uint32(P[1]) << 8 | uint32(P[2]) << 16 | uint32(P[3]) << 24; };
    Out.Count = Start;
    const uint32 Count = U32(Start);
    Out.CountValue = int32(Count);
    Out.Entries = Start + 4;
    if (Count > uint64(Size - 4) / 16) return false;
    Out.Tail = Out.Entries + int64(Count) * 16;
    return true;
}
struct FKyaFogZoneOffsets { int64 Count = 0, Entries = 0, Tail = 0; int32 CountValue = 0; };
inline bool KyaReadFogZoneOffsets(TConstArrayView<uint8> Data, int64 Start, int64 End, FKyaFogZoneOffsets& Out)
{
    Out = FKyaFogZoneOffsets{};
    if (Start < 0 || End > Data.Num() || End - Start < 4) return false;
    auto U32 = [&Data](int64 Offset) { const uint8* P = Data.GetData() + Offset; return uint32(P[0]) | uint32(P[1]) << 8 | uint32(P[2]) << 16 | uint32(P[3]) << 24; };
    Out.Count = Start;
    const uint32 Count = U32(Start);
    Out.CountValue = int32(Count);
    Out.Entries = Start + 4;
    if (Count > uint64(End - Out.Entries) / 28) return false;
    Out.Tail = Out.Entries + int64(Count) * 28;
    return true;
}
struct FKyaMoneyOffsets { int64 WeightB = 0, MoneyValue = 0, Sound = 0, Material = 0, Tail = 0; };
inline bool KyaReadMoneyOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size, FKyaMoneyOffsets& Out)
{
    Out = FKyaMoneyOffsets{};
    if (Start < 0 || Size < 16 || Start > Data.Num() || Size > Data.Num() - Start) return false;
    Out.WeightB = Start;
    Out.MoneyValue = Start + 4;
    Out.Sound = Start + 8;
    Out.Material = Start + 12;
    Out.Tail = Start + 16;
    return true;
}
struct FKyaBoxOffsets
{
    int64 WeightB = 0, Autonomous = 0, Field350 = 0;
    int64 ActorCount = 0, ActorEntries = 0;
    int32 ActorCountValue = 0;
    int64 Floats354 = 0, Field364 = 0, Field370 = 0;
    FKyaWindSwitchOffsets Switch;
    int64 AddOn = 0, SubObjMode = 0, SubObjFloats = 0, Tail = 0;
    int32 SubObjModeValue = 0;
};
inline bool KyaReadBoxOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size, FKyaBoxOffsets& Out)
{
    Out = FKyaBoxOffsets{};
    if (Start < 0 || Size < 100 || Start > Data.Num() || Size > Data.Num() - Start) return false;
    const int64 End = Start + Size;
    auto U32 = [&Data](int64 Offset) { const uint8* P = Data.GetData() + Offset; return uint32(P[0]) | uint32(P[1]) << 8 | uint32(P[2]) << 16 | uint32(P[3]) << 24; };
    FKyaBoxOffsets Result;
    int64 Pos = Start;
    Result.WeightB = Pos; Result.Autonomous = Pos + 4; Pos += 20;
    Result.Field350 = Pos; Pos += 4;
    Result.ActorCount = Pos;
    const uint32 Actors = U32(Pos);
    Result.ActorCountValue = int32(Actors);
    Pos += 4;
    if (uint64(Actors) > uint64(End - Pos) / 4) return false;
    Result.ActorEntries = Pos;
    Pos += int64(Actors) * 4;
    if (End - Pos < 36) return false;
    Result.Floats354 = Pos; Result.Field364 = Pos + 16; Result.Field370 = Pos + 20;
    Pos += 36;
    if (End - Pos < 4) return false;
    const uint32 SwitchTargets = U32(Pos);
    Result.Switch.Count = int32(SwitchTargets);
    Result.Switch.Targets = Pos + 4;
    if (SwitchTargets > uint64(End - Pos - 4) / 28) return false;
    Result.Switch.Camera = Result.Switch.Targets + int64(SwitchTargets) * 28;
    if (End - Result.Switch.Camera < 32) return false;
    Pos = Result.Switch.Camera + 32;
    if (End - Pos < 24) return false;
    Result.AddOn = Pos;
    Pos += 20;
    Result.SubObjMode = Pos;
    Result.SubObjModeValue = int32(U32(Pos));
    Pos += 4;
    if (Result.SubObjModeValue == 1)
    {
        if (End - Pos < 20) return false;
        Result.SubObjFloats = Pos;
        Pos += 20;
    }
    Result.Tail = Pos;
    Out = Result;
    return true;
}
struct FKyaTeleporterOffsets
{
    int64 SubsectorMaterialId = 0, Hash = 0, Field16C = 0;
    int64 DestCount = 0, DestEntries = 0;
    int32 DestCountValue = 0;
    FKyaWindSwitchOffsets Switches[3];
    int64 Zone188 = 0, Zone18C = 0, Material1 = 0, CinematicId = 0, Material2 = 0;
    int64 Field1D4 = 0, Field1D8 = 0, Material3 = 0, Field1E0 = 0;
    int64 WaypointCount = 0, WaypointEntries = 0;
    int32 WaypointCountValue = 0;
    int64 CndOpCount = 0;
    TArray<int64> BlockOpCounts, BlockOps, BlockCondCounts, BlockConds;
    TArray<int32> BlockOpCountValues, BlockCondCountValues;
    int64 Tail = 0;
};
inline bool KyaReadTeleporterOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size, FKyaTeleporterOffsets& Out)
{
    Out = FKyaTeleporterOffsets{};
    if (Start < 0 || Start > Data.Num() || Size > Data.Num() - Start) return false;
    const int64 End = Start + Size;
    auto U32 = [&Data](int64 Offset) { const uint8* P = Data.GetData() + Offset; return uint32(P[0]) | uint32(P[1]) << 8 | uint32(P[2]) << 16 | uint32(P[3]) << 24; };
    FKyaTeleporterOffsets Result;
    int64 Pos = Start;
    if (End - Pos < 16) return false;
    Result.SubsectorMaterialId = Pos;
    Result.Hash = Pos + 4;
    Result.Field16C = Pos + 12;
    Pos += 16;
    if (End - Pos < 4) return false;
    Result.DestCount = Pos;
    const uint32 Destinations = U32(Pos);
    Result.DestCountValue = int32(Destinations);
    Pos += 4;
    if (uint64(Destinations) > uint64(End - Pos) / 12) return false;
    Result.DestEntries = Pos;
    Pos += int64(Destinations) * 12;
    for (int32 Switch = 0; Switch < 3; ++Switch)
    {
        if (End - Pos < 4) return false;
        FKyaWindSwitchOffsets& S = Result.Switches[Switch];
        const uint32 Count = U32(Pos);
        S.Count = int32(Count);
        S.Targets = Pos + 4;
        if (Count > uint64(End - Pos - 4) / 28) return false;
        S.Camera = S.Targets + int64(Count) * 28;
        if (End - S.Camera < 32) return false;
        Pos = S.Camera + 32;
    }
    if (End - Pos < 36) return false;
    Result.Zone188 = Pos;
    Result.Zone18C = Pos + 4;
    Result.Material1 = Pos + 8;
    Result.CinematicId = Pos + 12;
    Result.Material2 = Pos + 16;
    Result.Field1D4 = Pos + 20;
    Result.Field1D8 = Pos + 24;
    Result.Material3 = Pos + 28;
    Result.Field1E0 = Pos + 32;
    Pos += 36;
    if (End - Pos < 4) return false;
    Result.WaypointCount = Pos;
    const uint32 Waypoints = U32(Pos);
    Result.WaypointCountValue = int32(Waypoints);
    Pos += 4;
    if (uint64(Waypoints) > uint64(End - Pos) / 8) return false;
    Result.WaypointEntries = Pos;
    Pos += int64(Waypoints) * 8;
    if (End - Pos < 4) return false;
    Result.CndOpCount = Pos;
    const uint32 Blocks = U32(Pos);
    Pos += 4;
    for (uint32 Block = 0; Block < Blocks; ++Block)
    {
        if (End - Pos < 4) return false;
        const int64 OpCountOffset = Pos;
        const uint32 Ops = U32(Pos);
        Pos += 4;
        if (uint64(Ops) > uint64(End - Pos) / 12) return false;
        Result.BlockOpCounts.Add(OpCountOffset);
        Result.BlockOpCountValues.Add(int32(Ops));
        Result.BlockOps.Add(Pos);
        Pos += int64(Ops) * 12;
        if (End - Pos < 4) return false;
        const int64 CondCountOffset = Pos;
        const uint32 Conds = U32(Pos);
        Pos += 4;
        if (uint64(Conds) > uint64(End - Pos) / 16) return false;
        Result.BlockCondCounts.Add(CondCountOffset);
        Result.BlockCondCountValues.Add(int32(Conds));
        Result.BlockConds.Add(Pos);
        Pos += int64(Conds) * 16;
    }
    Result.Tail = Pos;
    Out = Result;
    return true;
}
struct FKyaNativOffsets
{
    int64 WeightB = 0, Autonomous = 0, Field350 = 0, Field354 = 0, Field358 = 0;
    int64 MoveSpeedNoHeld = 0, MoveSpeedHeld = 0, Field370 = 0, Field368 = 0, Field364 = 0;
    int64 Field378 = 0, Field37C = 0, Field380 = 0, Tail = 0;
};
inline bool KyaReadNativOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size, FKyaNativOffsets& Out)
{
    Out = FKyaNativOffsets{};
    if (Start < 0 || Size < 64 || Start > Data.Num() || Size > Data.Num() - Start) return false;
    Out.WeightB = Start; Out.Autonomous = Start + 4;
    Out.Field350 = Start + 20; Out.Field354 = Start + 24; Out.Field358 = Start + 28;
    Out.MoveSpeedNoHeld = Start + 32; Out.MoveSpeedHeld = Start + 36;
    Out.Field370 = Start + 40; Out.Field368 = Start + 44; Out.Field364 = Start + 48;
    Out.Field378 = Start + 52; Out.Field37C = Start + 56; Out.Field380 = Start + 60;
    Out.Tail = Start + 64;
    return true;
}
struct FKyaNativTakePutOffsets { int64 Count = 0, Tail = 0; TArray<int64> Items; };
inline bool KyaReadNativTakePutOffsets(TConstArrayView<uint8> Data, int64 Start, int64 End, FKyaNativTakePutOffsets& Out)
{
    Out = FKyaNativTakePutOffsets{};
    if (Start < 0 || End > Data.Num() || End - Start < 4) return false;
    auto U32At = [&Data](int64 Offset) { const uint8* P = Data.GetData() + Offset; return uint32(P[0]) | uint32(P[1]) << 8 | uint32(P[2]) << 16 | uint32(P[3]) << 24; };
    Out.Count = Start;
    const uint32 Count = U32At(Start);
    int64 Pos = Start + 4;
    if (Count > uint64(End - Pos) / 16) return false;
    for (uint32 Index = 0; Index < Count; ++Index)
    {
        Out.Items.Add(Pos);
        Pos += 16;
    }
    Out.Tail = Pos;
    return true;
}
struct FKyaCommanderOffsets
{
    int64 Headers = 0, Squad = 0, Teams = 0, Components = 0, Camera = 0, Tail = 0;
    int32 TeamCount = 0, ComponentCount = 0;
};
inline bool KyaReadCommanderOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size, FKyaCommanderOffsets& Out)
{
    Out = FKyaCommanderOffsets{};
    if (Start < 0 || Size < 96 || Start > Data.Num() || Size > Data.Num() - Start) return false;
    auto U32At = [&Data](int64 Offset) { const uint8* P = Data.GetData() + Offset; return uint32(P[0]) | uint32(P[1]) << 8 | uint32(P[2]) << 16 | uint32(P[3]) << 24; };
    const int64 End = Start + Size;
    Out.Headers = Start;
    Out.Squad = Start + 20;
    const uint32 Teams = U32At(Start + 32);
    Out.TeamCount = int32(Teams);
    Out.Teams = Start + 36;
    int64 Pos = Out.Teams + int64(Teams) * 4;
    if (End - Pos < 4) return false;
    const uint32 Components = U32At(Pos);
    Out.ComponentCount = int32(Components);
    Out.Components = Pos + 4;
    if (uint64(Components) > uint64(End - Pos - 4) / 8) return false;
    Pos = Out.Components + int64(Components) * 8;
    if (End - Pos < 56) return false;
    Out.Camera = Pos;
    Out.Tail = Pos + 56;
    return Out.Tail <= End;
}
struct FKyaCommanderBehaviourOffsets { int64 SquadConfig = 0, Switch = 0, Tail = 0; };
inline bool KyaReadCommanderBehaviourOffsets(TConstArrayView<uint8> Data, int64 Start, int64 End, FKyaCommanderBehaviourOffsets& Out)
{
    Out = FKyaCommanderBehaviourOffsets{};
    if (Start < 0 || Start > Data.Num() || End < Start + 44 || End > Data.Num()) return false;
    Out.SquadConfig = Start;
    Out.Switch = Start + 8;
    const uint8* P = Data.GetData() + Out.Switch;
    const uint32 Count = uint32(P[0]) | uint32(P[1]) << 8 | uint32(P[2]) << 16 | uint32(P[3]) << 24;
    if (Count > uint64(End - Out.Switch - 36) / 28) return false;
    Out.Tail = Out.Switch + 4 + int64(Count) * 28 + 32;
    return Out.Tail <= End;
}
struct FKyaFruitOffsets { int64 WeightB = 0, Autonomous = 0, Field360 = 0, Switch = 0, Tail = 0; };
inline bool KyaReadFruitOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size, FKyaFruitOffsets& Out)
{
    Out = FKyaFruitOffsets{};
    if (Start < 0 || Size < 60 || Start > Data.Num() || Size > Data.Num() - Start) return false;
    Out.WeightB = Start;
    Out.Autonomous = Start + 4;
    Out.Field360 = Start + 20;
    Out.Switch = Start + 24;
    const uint8* P = Data.GetData() + Out.Switch;
    const uint32 Count = uint32(P[0]) | uint32(P[1]) << 8 | uint32(P[2]) << 16 | uint32(P[3]) << 24;
    if (Count > uint64(Size - 60) / 28) return false;
    Out.Tail = Out.Switch + 4 + int64(Count) * 28 + 32;
    return true;
}
struct FKyaFruitInventoryValues
{
    bool bHasData = false, bHasObject = false;
    int32 Field0 = 0;
    int64 Field8 = 0, Field10 = 0;
    int32 PurchaseId = 0, Field28 = 0, MoneyCost = 0, Field2C = 0;
    int32 ObjectId = 0;
    int64 Field18 = 0;
    int32 InitialBehaviour = 0;
};
inline bool KyaReadFruitInventory(TConstArrayView<uint8> Data, int64 Start, int64 Size, FKyaFruitInventoryValues& Out)
{
    Out = FKyaFruitInventoryValues{};
    if (Start < 0 || Size < 40 || Start > Data.Num() || Size > Data.Num() - Start) return false;
    auto U32 = [&Data](int64 Offset) { const uint8* P = Data.GetData() + Offset; return uint32(P[0]) | uint32(P[1]) << 8 | uint32(P[2]) << 16 | uint32(P[3]) << 24; };
    auto U64 = [&U32](int64 Offset) { return uint64(U32(Offset)) | (uint64(U32(Offset + 4)) << 32); };
    int64 Pos = Start;
    const int32 Header = int32(U32(Pos)); Pos += 4;
    Out.bHasData = Header != 0;
    Out.Field0 = Out.bHasData ? 2 : 0;
    if (Out.bHasData)
    {
        if (Start + Size - Pos < 32) return false;
        Out.Field8 = int64(U64(Pos)); Out.Field10 = int64(U64(Pos + 8));
        Out.PurchaseId = int32(U32(Pos + 16)); Out.Field28 = int32(U32(Pos + 20));
        Out.MoneyCost = int32(U32(Pos + 24)); Out.Field2C = int32(U32(Pos + 28));
        Pos += 32;
    }
    const int64 Remaining = Start + Size - Pos;
    if (Remaining == 4)
    {
        Out.InitialBehaviour = int32(U32(Pos));
        return true;
    }
    if (Remaining < 8) return false;
    const int32 HasObject = int32(U32(Pos)); Pos += 4;
    Out.bHasObject = HasObject != 0;
    if (Out.bHasObject)
    {
        if (Start + Size - Pos < 16) return false;
        Out.ObjectId = int32(U32(Pos));
        Out.Field18 = int64(U64(Pos + 4));
        Pos += 12;
    }
    if (Start + Size - Pos != 4) return false;
    Out.InitialBehaviour = int32(U32(Pos));
    return true;
}
struct FKyaMickenOffsets
{
    int64 WeightB = 0, Autonomous = 0, Waypoint = 0, Actor = 0, Field358 = 0, Field35C = 0, Field360 = 0, Tail = 0;
};
inline bool KyaReadMickenOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size, FKyaMickenOffsets& Out)
{
    Out = FKyaMickenOffsets{};
    if (Start < 0 || Size < 40 || Start > Data.Num() || Size > Data.Num() - Start) return false;
    Out.WeightB = Start;
    Out.Autonomous = Start + 4;
    Out.Waypoint = Start + 20;
    Out.Actor = Start + 24;
    Out.Field358 = Start + 28;
    Out.Field35C = Start + 32;
    Out.Field360 = Start + 36;
    Out.Tail = Start + 40;
    return true;
}
struct FKyaMickenBehaviourValues { float Field8 = 0, FieldC = 0; int32 Field10 = 0, PathIndex = -1; };
inline bool KyaReadMickenBehaviour(int32 Id, TConstArrayView<uint8> Data, int64 Start, int64 Size, FKyaMickenBehaviourValues& Out)
{
    Out = FKyaMickenBehaviourValues{};
    if (Id != 4 || Start < 0 || Size != 16 || Start > Data.Num() || Size > Data.Num() - Start) return false;
    auto U32 = [&Data](int64 Offset) { const uint8* P = Data.GetData() + Offset; return uint32(P[0]) | uint32(P[1]) << 8 | uint32(P[2]) << 16 | uint32(P[3]) << 24; };
    auto F32 = [&U32](int64 Offset) { const uint32 Bits = U32(Offset); float Value; FMemory::Memcpy(&Value, &Bits, sizeof(Value)); return Value; };
    Out.Field8 = F32(Start);
    Out.FieldC = F32(Start + 4);
    Out.Field10 = int32(U32(Start + 8));
    Out.PathIndex = int32(U32(Start + 12));
    return true;
}
struct FKyaBonusOffsets { int64 WeightB = 0, Sound = 0, FlareMaterial = 0, AnimMaterial = 0, Tail = 0; };
inline bool KyaReadBonusOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size, FKyaBonusOffsets& Out)
{
    Out = FKyaBonusOffsets{};
    if (Start < 0 || Size < 16 || Start > Data.Num() || Size > Data.Num() - Start) return false;
    Out.WeightB = Start;
    Out.Sound = Start + 4;
    Out.FlareMaterial = Start + 8;
    Out.AnimMaterial = Start + 12;
    Out.Tail = Start + 16;
    return true;
}
struct FKyaBonusBehaviourValues
{
    float Field14 = 0, Field1B8 = 0, Field1B4 = 0;
    int32 MaterialId = -1;
    float Field1B0 = 0;
    int32 FxTailCount = 0, G2DIndex = -1;
    float TurnValue = 0;
    int32 FlockPathIndex = -1;
    float AddOnField18 = 0;
};
inline bool KyaReadBonusBehaviour(int32 Id, TConstArrayView<uint8> Data, int64 Start, int64 Size, FKyaBonusBehaviourValues& Out)
{
    Out = FKyaBonusBehaviourValues{};
    if (Start < 0 || Start > Data.Num() || Size > Data.Num() - Start) return false;
    auto U32 = [&Data](int64 Offset) { const uint8* P = Data.GetData() + Offset; return uint32(P[0]) | uint32(P[1]) << 8 | uint32(P[2]) << 16 | uint32(P[3]) << 24; };
    auto F32 = [&U32](int64 Offset) { const uint32 Bits = U32(Offset); float Value; FMemory::Memcpy(&Value, &Bits, sizeof(Value)); return Value; };
    if (Id == 3)
    {
        if (Size != 32) return false;
        Out.Field14 = F32(Start); Out.Field1B8 = F32(Start + 4); Out.Field1B4 = F32(Start + 8);
        Out.MaterialId = int32(U32(Start + 12)); Out.Field1B0 = F32(Start + 16);
        Out.FxTailCount = int32(U32(Start + 20)); Out.G2DIndex = int32(U32(Start + 24));
        Out.TurnValue = F32(Start + 28);
        return true;
    }
    if (Id == 5)
    {
        if (Size != 4) return false;
        Out.FlockPathIndex = int32(U32(Start));
        return true;
    }
    if (Id == 6)
    {
        if (Size != 4) return false;
        Out.AddOnField18 = F32(Start);
        return true;
    }
    return false;
}
struct FKyaSwitchOffsets { int64 Targets = 0, Camera = 0, Tail = 0; int32 Count = 0; };
inline bool KyaReadSwitchOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size, FKyaSwitchOffsets& Out)
{
    Out = FKyaSwitchOffsets{};
    if (Start < 0 || Size < 36 || Start > Data.Num() || Size > Data.Num() - Start) return false;
    const uint8* P = Data.GetData() + Start;
    const uint32 Count = uint32(P[0]) | uint32(P[1]) << 8 | uint32(P[2]) << 16 | uint32(P[3]) << 24;
    if (Count > uint64(Size - 36) / 28) return false;
    Out.Count = int32(Count);
    Out.Targets = Start + 4;
    Out.Camera = Out.Targets + int64(Count) * 28;
    Out.Tail = Out.Camera + 32;
    return true;
}
struct FKyaWeaponOffsets { int64 WeightB = 0, Field1D8 = 0, Field1DC = 0, Tail = 0; };
inline bool KyaReadWeaponOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size, FKyaWeaponOffsets& Out)
{
    Out = FKyaWeaponOffsets{};
    if (Start < 0 || Size < 12 || Start > Data.Num() || Size > Data.Num() - Start) return false;
    Out.WeightB = Start;
    Out.Field1D8 = Start + 4;
    Out.Field1DC = Start + 8;
    Out.Tail = Start + 12;
    return true;
}
struct FKyaWeaponBehaviourValues
{
    TArray<FKyaWeaponAdvancedEntry> Burst;
    int32 Field40 = 0, Field44 = 0, SoundIndex = -1;
    int32 PistolField2E0 = 0, PistolField2E4 = -1;
    int32 SniperTextureIndex = 0, SniperMeshIndex = 0, SniperMaterialId = -1;
    TArray<int32> RpgActorRefs;
};
inline bool KyaReadWeaponBehaviour(int32 Id, TConstArrayView<uint8> Data, int64 Start, int64 Size, FKyaWeaponBehaviourValues& Out)
{
    Out = FKyaWeaponBehaviourValues{};
    if (Id != 3 && Id != 4 && Id != 6) return false;
    if (Start < 0 || Size < 32 || Start > Data.Num() || Size > Data.Num() - Start) return false;
    auto U32 = [&Data](int64 Offset) { const uint8* P = Data.GetData() + Offset; return uint32(P[0]) | uint32(P[1]) << 8 | uint32(P[2]) << 16 | uint32(P[3]) << 24; };
    auto F32 = [&U32](int64 Offset) { const uint32 Bits = U32(Offset); float Value; FMemory::Memcpy(&Value, &Bits, sizeof(Value)); return Value; };
    for (int32 Index = 0; Index < 2; ++Index)
    {
        FKyaWeaponAdvancedEntry& Entry = Out.Burst.AddDefaulted_GetRef();
        const int64 E = Start + int64(Index) * 16;
        Entry.FieldC = F32(E);
        Entry.Field4 = int32(U32(E + 4));
        Entry.Field0 = F32(E + 8);
        Entry.Field8 = F32(E + 12);
    }
    if (Id == 3)
    {
        if (Size != 52) return false;
        Out.Field40 = int32(U32(Start + 32));
        Out.Field44 = int32(U32(Start + 36));
        Out.SoundIndex = int32(U32(Start + 40));
        Out.PistolField2E0 = int32(U32(Start + 44));
        Out.PistolField2E4 = int32(U32(Start + 48));
        return true;
    }
    if (Id == 4)
    {
        if (Size != 56) return false;
        Out.Field40 = int32(U32(Start + 32));
        Out.Field44 = int32(U32(Start + 36));
        Out.SoundIndex = int32(U32(Start + 40));
        Out.SniperTextureIndex = int32(U32(Start + 44));
        Out.SniperMeshIndex = int32(U32(Start + 48));
        Out.SniperMaterialId = int32(U32(Start + 52));
        return true;
    }
    if (Size < 36) return false;
    const uint32 Count = U32(Start + 32);
    if (Count > uint64(Size - 36) / 4 || Size != 36 + int64(Count) * 4) return false;
    for (uint32 Index = 0; Index < Count; ++Index)
        Out.RpgActorRefs.Add(int32(U32(Start + 36 + int64(Index) * 4)));
    return true;
}
struct FKyaFxOffsets { int64 Field19C = 0, Field1A0 = 0, Tail = 0; };
inline bool KyaReadFxOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size, FKyaFxOffsets& Out)
{
    Out = FKyaFxOffsets{};
    if (Start < 0 || Size < 8 || Start > Data.Num() || Size > Data.Num() - Start) return false;
    Out.Field19C = Start;
    Out.Field1A0 = Start + 4;
    Out.Tail = Start + 8;
    return true;
}
inline bool KyaReadPlatformPrefix(TConstArrayView<uint8> Data, int64 Start, int64 Size, FKyaPlatformPrefixOffsets& Out)
{
    Out = FKyaPlatformPrefixOffsets{};
    if (Start < 0 || Size < 16 || Start > Data.Num() || Size > Data.Num() - Start) return false;
    auto U32 = [&Data](int64 Offset) { const uint8* P = Data.GetData() + Offset; return uint32(P[0]) | uint32(P[1]) << 8 | uint32(P[2]) << 16 | uint32(P[3]) << 24; };
    const int64 End = Start + Size;
    FKyaPlatformPrefixOffsets Result;
    Result.Weight = Start;
    int64 Pos = Start + 4;
    auto ReadList = [&](int32 Stride, int64& Offset, int32& Count) {
        if (End - Pos < 4) return false;
        const uint32 Num = U32(Pos); Pos += 4;
        if (Num > MAX_int32 || Num > uint64(End - Pos) / Stride) return false;
        Offset = Pos; Count = int32(Num); Pos += int64(Num) * Stride;
        return true;
    };
    if (!ReadList(8, Result.Actors, Result.ActorCount) || !ReadList(4, Result.Zones, Result.ZoneCount) || End - Pos < 4) return false;
    const uint32 ZoneCount = U32(Pos); Pos += 4;
    if (ZoneCount > uint64(End - Pos) / 8) return false;
    for (uint32 Index = 0; Index < ZoneCount; ++Index)
    {
        if (End - Pos < 8) return false;
        Result.NoFrictionZones.Add(Pos);
        const uint32 ActorCount = U32(Pos + 4); Pos += 8;
        if (ActorCount > uint64(End - Pos) / 4) return false;
        Pos += int64(ActorCount) * 4;
    }
    Result.Suffix = Pos;
    Out = MoveTemp(Result);
    return true;
}
struct FKyaClusterZoneOffsets { int64 Header = 0, Lists[5] = {}, Tail = 0; int32 Counts[5] = {}; };
inline bool KyaReadClusterZoneOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size, TArray<FKyaClusterZoneOffsets>& OutZones)
{
    OutZones.Reset();
    if (Start < 0 || Size < 4 || Start > Data.Num() || Size > Data.Num() - Start) return false;
    auto U32 = [&Data](int64 Offset) { const uint8* P = Data.GetData() + Offset; return uint32(P[0]) | uint32(P[1]) << 8 | uint32(P[2]) << 16 | uint32(P[3]) << 24; };
    const int64 End = Start + Size;
    const uint32 Count = U32(Start);
    int64 Pos = Start + 4;
    if (Count > uint64(End - Pos) / 32) return false;
    TArray<FKyaClusterZoneOffsets> Zones;
    for (uint32 Index = 0; Index < Count; ++Index)
    {
        if (End - Pos < 8) return false;
        FKyaClusterZoneOffsets Zone;
        Zone.Header = Pos; Pos += 8;
        for (int32 List = 0; List < 5; ++List)
        {
            if (End - Pos < 4) return false;
            const uint32 Num = U32(Pos); Pos += 4;
            const int32 Stride = List == 4 ? 8 : 4;
            if (Num > MAX_int32 || Num > uint64(End - Pos) / Stride) return false;
            Zone.Lists[List] = Pos; Zone.Counts[List] = int32(Num);
            Pos += int64(Num) * Stride;
        }
        if (End - Pos < 4) return false;
        Zone.Tail = Pos; Pos += 4;
        Zones.Add(Zone);
    }
    OutZones = MoveTemp(Zones);
    return true;
}
struct FKyaActorStringOffset { int64 Offset = 0; int32 Capacity = 0; };
struct FKyaActorOffsets { int64 Chunk = 0, Name = 0, Placement = 0, MacroAnimations = 0, Common = 0, Conditions = 0, ClassData = 0; int32 NameCapacity = 0, MacroCount = 0, ConditionCount = 0, ClassDataSize = 0; TArray<FKyaBehaviourOffsets> Behaviours; TArray<FKyaActorStringOffset> ModelStrings; };

inline bool KyaReadCheckpointOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size, TArray<int64>& OutOffsets, int64* OutTail = nullptr)
{
    OutOffsets.Reset();
    if (Start < 0 || Size < 4 || Start > Data.Num() || Size > Data.Num() - Start) return false;
    auto U32 = [&Data](int64 Offset) { const uint8* P = Data.GetData() + Offset; return uint32(P[0]) | uint32(P[1]) << 8 | uint32(P[2]) << 16 | uint32(P[3]) << 24; };
    const int64 End = Start + Size;
    const uint32 Count = U32(Start);
    int64 Pos = Start + 4;
    if (Count > uint64(End - Pos) / 24) return false;
    TArray<int64> Offsets;
    for (uint32 Index = 0; Index < Count; ++Index)
    {
        if (End - Pos < 24) return false;
        const uint32 PairCount = U32(Pos + 20);
        if (PairCount > uint64(End - Pos - 24) / 8) return false;
        Offsets.Add(Pos);
        Pos += 24 + int64(PairCount) * 8;
    }
    if (OutTail) *OutTail = Pos;
    OutOffsets = MoveTemp(Offsets);
    return true;
}
inline bool KyaReadActorOffsets(TConstArrayView<uint8> Data, int64 Waypoints,
    int64 AlignmentBase, TArray<FKyaActorOffsets>& OutActors)
{
    OutActors.Reset();
    auto Fits = [&](int64 Pos, int64 Size) { return Pos >= 0 && Pos <= Data.Num() && Size <= Data.Num() - Pos; };
    auto U32 = [&](int64 Pos) { const uint8* P = Data.GetData() + Pos; return uint32(P[0]) | uint32(P[1]) << 8 | uint32(P[2]) << 16 | uint32(P[3]) << 24; };
    if (Waypoints < 12 || !Fits(Waypoints - 12, 8)) return false;
    const int64 PathChunk = Waypoints - 12 + U32(Waypoints - 8);
    if (!Fits(PathChunk, 8) || U32(PathChunk) != 0x50415448) return false;
    int64 Pos = PathChunk + U32(PathChunk + 4);
    if (!Fits(Pos, 4)) return false;
    const uint32 Count = U32(Pos); Pos += 4;
    if (Count > uint64(Data.Num() - Pos) / 64) return false;
    for (uint32 Index = 0; Index < Count; ++Index)
    {
        if (!Fits(Pos, 16) || U32(Pos) != 0x494e5354) return false;
        const uint32 Size = U32(Pos + 4);
        if (Size < 64 || !Fits(Pos, Size)) return false;
        const int64 End = Pos + Size, Name = Pos + 16;
        int64 Terminator = Name;
        while (Terminator < End && Data[Terminator] != 0) ++Terminator;
        if (Terminator == End) return false;
        const int64 Placement = AlignmentBase + Align(Terminator + 1 - AlignmentBase, int64(4));
        if (Placement > End || End - Placement < 52) return false;
        const uint32 MacroCount = U32(Placement + 48);
        if (MacroCount > uint64(End - Placement - 52) / 8 || MacroCount > MAX_int32) return false;
        int64 Cursor = Placement + 52 + int64(MacroCount) * 8;
        if (End - Cursor < 4) return false;
        const uint32 BehaviourCount = U32(Cursor);
        Cursor += 4;
        if (BehaviourCount > uint64(End - Cursor) / 8) return false;
        const int64 BehaviourTable = Cursor;
        Cursor += int64(BehaviourCount) * 8;
        TArray<FKyaBehaviourOffsets> Behaviours;
        for (uint32 Behaviour = 0; Behaviour < BehaviourCount; ++Behaviour)
        {
            const uint32 SizeBytes = U32(BehaviourTable + Behaviour * 8 + 4);
            if (SizeBytes > uint64(End - Cursor) || SizeBytes > MAX_int32) return false;
            FKyaBehaviourOffsets& Entry = Behaviours.AddDefaulted_GetRef();
            Entry.Id = BehaviourTable + Behaviour * 8;
            Entry.Payload = Cursor;
            Entry.Size = int32(SizeBytes);
            Cursor += SizeBytes;
        }
        if (End - Cursor < 4) return false;
        const uint32 ModelCount = U32(Cursor); Cursor += 4;
        if (ModelCount > 16) return false;
        TArray<FKyaActorStringOffset> ModelStrings;
        for (uint32 String = 0; String < ModelCount * 2; ++String)
        {
            const int64 Start = Cursor;
            while (Cursor < End && Data[Cursor] != 0) ++Cursor;
            if (Cursor == End) return false;
            Cursor = AlignmentBase + Align(Cursor + 1 - AlignmentBase, int64(4));
            if (Cursor > End) return false;
            FKyaActorStringOffset& Offset = ModelStrings.AddDefaulted_GetRef();
            Offset.Offset = Start;
            Offset.Capacity = int32(Cursor - Start);
        }
        if (End - Cursor < 84) return false;
        const int64 ConditionCountOffset = Cursor + 80;
        const uint32 ConditionCount = U32(ConditionCountOffset);
        if (ConditionCount > uint64(End - ConditionCountOffset - 4) / 16 || ConditionCount > MAX_int32) return false;
        const int64 ClassData = ConditionCountOffset + 4 + int64(ConditionCount) * 16;
        FKyaActorOffsets& Actor = OutActors.AddDefaulted_GetRef();
        Actor.Chunk = Pos; Actor.Name = Name; Actor.Placement = Placement;
        Actor.NameCapacity = int32(Placement - Name);
        Actor.MacroAnimations = Placement + 52;
        Actor.MacroCount = int32(MacroCount);
        Actor.Common = Cursor;
        Actor.Conditions = ConditionCountOffset + 4;
        Actor.ConditionCount = int32(ConditionCount);
        Actor.ClassData = ClassData;
        Actor.ClassDataSize = int32(End - ClassData);
        Actor.Behaviours = MoveTemp(Behaviours);
        Actor.ModelStrings = MoveTemp(ModelStrings);
        Pos = End;
    }
    return true;
}
