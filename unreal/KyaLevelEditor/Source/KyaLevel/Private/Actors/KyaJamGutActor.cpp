#include "Actors/KyaJamGut.h"
#include "KyaLevelBank.h"
namespace
{
uint32 Read(const uint8* P)
{
    return uint32(P[0]) | uint32(P[1]) << 8 | uint32(P[2]) << 16 | uint32(P[3]) << 24;
}
float ReadFloat(const uint8* P)
{
    uint32 Bits = Read(P);
    float Value;
    FMemory::Memcpy(&Value, &Bits, 4);
    return Value;
}
void Write(uint8* P, uint32 Value)
{
    for (int32 I = 0; I < 4; ++I) P[I] = uint8(Value >> (I * 8));
}
void WriteFloat(uint8* P, float Value)
{
    uint32 Bits;
    FMemory::Memcpy(&Bits, &Value, 4);
    Write(P, Bits);
}
bool Candidate(TConstArrayView<uint8> Data, int64 Start, int64 Size, bool Modern, FKyaJamGutOffsets& Out)
{
    if (Size < (Modern ? 72 : 68)) return false;
    const int64 End = Start + Size;
    Out.bHasField41C = Modern;
    Out.Suffix = Start + (Modern ? 48 : 44);
    int64 Pos = Out.Suffix + 12;
    const uint32 Checkpoints = Read(Data.GetData() + Pos);
    Pos += 4;
    if (Checkpoints > MAX_int32 || uint64(Checkpoints) > uint64(End - Pos - 8) / 8) return false;
    Out.CheckpointCount = int32(Checkpoints);
    Out.Checkpoints = Pos;
    Pos += int64(Checkpoints) * 8;
    const uint32 Paths = Read(Data.GetData() + Pos);
    Pos += 4;
    if (Paths > MAX_int32 || uint64(Paths) > uint64(End - Pos - 4) / 24) return false;
    Out.PathCount = int32(Paths);
    Out.Paths = Pos;
    Pos += int64(Paths) * 24;
    Out.Type = Pos;
    Out.Tail = Pos + 4;
    return Out.Tail == End;
}
}
bool KyaReadJamGutOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size, FKyaJamGutOffsets& Out)
{
    Out = {};
    if (Start < 0 || Size < 68 || Start > Data.Num() || Size > Data.Num() - Start) return false;
    FKyaJamGutOffsets Old, New;
    const bool HasOld = Candidate(Data, Start, Size, false, Old);
    const bool HasNew = Candidate(Data, Start, Size, true, New);
    if (HasOld == HasNew) return false;
    Out = HasNew ? New : Old;
    return true;
}
bool KyaReadJamGutActor(FKyaActorPlacement& Actor, TConstArrayView<uint8> Data, int64 Start, int64 Size)
{
    FKyaJamGutOffsets Off;
    if (!KyaReadJamGutOffsets(Data, Start, Size, Off)) return false;
    FKyaJamGutData& V = Actor.JamGutData;
    V = {};
    const uint8* P = Data.GetData() + Start;
    V.WeightB = ReadFloat(P + 0);
    V.LifeMax = Read(P + 4);
    V.GravityScale = ReadFloat(P + 8);
    V.DynamicField4 = ReadFloat(P + 12);
    V.DynamicField8 = ReadFloat(P + 16);
    V.Field400 = ReadFloat(P + 20);
    V.Field404 = ReadFloat(P + 24);
    V.Field408 = ReadFloat(P + 28);
    V.Field40C = ReadFloat(P + 32);
    V.Field410 = ReadFloat(P + 36);
    V.Field414 = ReadFloat(P + 40);
    V.bHasField41C = Off.bHasField41C;
    if (Off.bHasField41C) V.Field41C = ReadFloat(P + 44);
    const uint8* S = Data.GetData() + Off.Suffix;
    V.Field4A4 = ReadFloat(S);
    V.Field374 = Read(S + 4);
    V.Flags358 = Read(S + 8);
    for (int32 I = 0; I < Off.CheckpointCount; ++I)
    {
        const uint8* B = Data.GetData() + Off.Checkpoints + int64(I) * 8;
        FKyaJamGutCheckpoint& C = V.Checkpoints.AddDefaulted_GetRef();
        C.ZoneRef = int32(Read(B)); C.WaypointRef = int32(Read(B + 4));
    }
    for (int32 I = 0; I < Off.PathCount; ++I)
    {
        const uint8* B = Data.GetData() + Off.Paths + int64(I) * 24;
        FKyaJamGutPath& Path = V.Paths.AddDefaulted_GetRef();
        Path.ZoneRef = int32(Read(B)); Path.PathIndex = int32(Read(B + 4)); Path.ActorRef = int32(Read(B + 8));
        Path.Field28 = ReadFloat(B + 12); Path.Field2C = ReadFloat(B + 16); Path.Field30 = ReadFloat(B + 20);
    }
    V.Field368Type = int32(Read(Data.GetData() + Off.Type));
    Actor.ClassData.Reset();
    Actor.bHasJamGutData = true;
    return true;
}
bool KyaWriteJamGutActor(const FKyaActorPlacement& Actor, TArray<uint8>& Data, int64 Start, int64 Size)
{
    FKyaJamGutOffsets Off;
    const FKyaJamGutData& V = Actor.JamGutData;
    if (!Actor.bHasJamGutData || !KyaReadJamGutOffsets(MakeArrayView(Data.GetData(), Data.Num()), Start, Size, Off) ||
        V.bHasField41C != Off.bHasField41C || V.Checkpoints.Num() != Off.CheckpointCount || V.Paths.Num() != Off.PathCount ||
        V.LifeMax < 0 || V.LifeMax > MAX_uint32 || V.Field374 < 0 || V.Field374 > MAX_uint32 || V.Flags358 < 0 || V.Flags358 > MAX_uint32) return false;
    uint8* P = Data.GetData() + Start;
    WriteFloat(P + 0, V.WeightB);
    Write(P + 4, uint32(V.LifeMax));
    WriteFloat(P + 8, V.GravityScale);
    WriteFloat(P + 12, V.DynamicField4);
    WriteFloat(P + 16, V.DynamicField8);
    WriteFloat(P + 20, V.Field400);
    WriteFloat(P + 24, V.Field404);
    WriteFloat(P + 28, V.Field408);
    WriteFloat(P + 32, V.Field40C);
    WriteFloat(P + 36, V.Field410);
    WriteFloat(P + 40, V.Field414);
    if (Off.bHasField41C) WriteFloat(P + 44, V.Field41C);
    uint8* S = Data.GetData() + Off.Suffix;
    WriteFloat(S, V.Field4A4);
    Write(S + 4, uint32(V.Field374)); Write(S + 8, uint32(V.Flags358));
    for (int32 I = 0; I < Off.CheckpointCount; ++I)
    {
        uint8* B = Data.GetData() + Off.Checkpoints + int64(I) * 8;
        Write(B, uint32(V.Checkpoints[I].ZoneRef)); Write(B + 4, uint32(V.Checkpoints[I].WaypointRef));
    }
    for (int32 I = 0; I < Off.PathCount; ++I)
    {
        uint8* B = Data.GetData() + Off.Paths + int64(I) * 24;
        const FKyaJamGutPath& Path = V.Paths[I];
        Write(B, uint32(Path.ZoneRef)); Write(B + 4, uint32(Path.PathIndex)); Write(B + 8, uint32(Path.ActorRef));
        WriteFloat(B + 12, Path.Field28); WriteFloat(B + 16, Path.Field2C); WriteFloat(B + 20, Path.Field30);
    }
    Write(Data.GetData() + Off.Type, uint32(V.Field368Type));
    FKyaJamGutOffsets Written;
    return KyaReadJamGutOffsets(MakeArrayView(Data.GetData(), Data.Num()), Start, Size, Written) &&
        Written.bHasField41C == Off.bHasField41C && Written.CheckpointCount == Off.CheckpointCount && Written.PathCount == Off.PathCount;
}
