#pragma once
#include "CoreMinimal.h"

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

inline bool KyaReadCheckpointOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size, TArray<int64>& OutOffsets)
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
