#include "Actors/KyaBlazer.h"
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
bool Candidate(TConstArrayView<uint8> Data, int64 Start, int64 Size, bool Modern, FKyaBlazerOffsets& Out)
{
    const int64 Prefix = Modern ? 76 : 68;
    if (Size < Prefix + 64) return false;
    const uint32 Count = Read(Data.GetData() + Start + Prefix + 28);
    if (Count > MAX_int32 || int64(Count) * 4 != Size - Prefix - 64) return false;
    Out.Suffix = Start + Prefix;
    Out.Count = int32(Count);
    Out.Entries = Out.Suffix + 32;
    Out.Tilt = Out.Entries + int64(Count) * 4;
    Out.Tail = Out.Tilt + 32;
    Out.bHasWaypointRefs = Modern;
    return true;
}
}
bool KyaReadBlazerOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size, FKyaBlazerOffsets& Out)
{
    Out = {};
    if (Start < 0 || Size < 132 || Start > Data.Num() || Size > Data.Num() - Start) return false;
    FKyaBlazerOffsets Legacy, Modern;
    const bool HasLegacy = Candidate(Data, Start, Size, false, Legacy);
    const bool HasModern = Candidate(Data, Start, Size, true, Modern);
    // Version 2.24 adds two waypoint refs. Ambiguous records stay raw.
    if (HasLegacy == HasModern) return false;
    Out = HasModern ? Modern : Legacy;
    return true;
}
bool KyaReadBlazerActor(FKyaActorPlacement& Actor, TConstArrayView<uint8> Data, int64 Start, int64 Size)
{
    FKyaBlazerOffsets Off;
    if (!KyaReadBlazerOffsets(Data, Start, Size, Off)) return false;
    const uint8* P = Data.GetData() + Start;
    FKyaBlazerData& V = Actor.BlazerData;
    V = {};
    V.WeightB = ReadFloat(P);
    V.LifeMax = Read(P + 4);
    V.GravityScale = ReadFloat(P + 8);
    V.DynamicField4 = ReadFloat(P + 12);
    V.DynamicField8 = ReadFloat(P + 16);
    V.Field358 = Read(P + 20);
    V.Field35C = Read(P + 24);
    V.Field368 = ReadFloat(P + 28);
    V.Field364 = ReadFloat(P + 32);
    V.Field36C = ReadFloat(P + 36);
    V.Field370 = Read(P + 40);
    V.Field374 = ReadFloat(P + 44);
    V.Field378 = ReadFloat(P + 48);
    V.Field37C = ReadFloat(P + 52);
    V.StunnedDuration = ReadFloat(P + 56);
    V.Field384 = ReadFloat(P + 60);
    V.Angle388Degrees = ReadFloat(P + 64);
    V.bHasWaypointRefs = Off.bHasWaypointRefs;
    if (Off.bHasWaypointRefs)
    {
        V.Waypoint38C = int32(Read(P + 68));
        V.Waypoint390 = int32(Read(P + 72));
    }
    const uint8* S = Data.GetData() + Off.Suffix;
    V.Field398 = Read(S);
    V.Field394 = Read(S + 4);
    V.PathIndex = int32(Read(S + 8));
    V.Field3AC = ReadFloat(S + 12);
    V.Field3B0 = ReadFloat(S + 16);
    V.Field3B4 = Read(S + 20);
    V.Field3B8 = ReadFloat(S + 24);
    for (int32 I = 0; I < Off.Count; ++I)
        V.ActorRefs.Add(int32(Read(Data.GetData() + Off.Entries + int64(I) * 4)));
    const uint8* T = Data.GetData() + Off.Tilt;
    V.TiltField0 = ReadFloat(T);
    V.TiltMode = int32(Read(T + 4));
    V.TiltValue = ReadFloat(T + 8);
    V.TiltSpringStrength = ReadFloat(T + 12);
    V.TiltDamping = ReadFloat(T + 16);
    V.PushValue = ReadFloat(T + 20);
    V.PushSpringStrength = ReadFloat(T + 24);
    V.PushDamping = ReadFloat(T + 28);
    Actor.bHasBlazerData = true;
    Actor.ClassData.Reset();
    return true;
}
bool KyaWriteBlazerActor(const FKyaActorPlacement& Actor, TArray<uint8>& Data, int64 Start, int64 Size)
{
    FKyaBlazerOffsets Off;
    const FKyaBlazerData& V = Actor.BlazerData;
    if (!Actor.bHasBlazerData || !KyaReadBlazerOffsets(MakeArrayView(Data.GetData(), Data.Num()), Start, Size, Off) ||
        V.ActorRefs.Num() != Off.Count || V.bHasWaypointRefs != Off.bHasWaypointRefs) return false;
    const int64 Words[] = {V.LifeMax, V.Field358, V.Field35C, V.Field370, V.Field398, V.Field394, V.Field3B4};
    for (int64 Word : Words) if (Word < 0 || Word > MAX_uint32) return false;
    uint8* P = Data.GetData() + Start;
    WriteFloat(P, V.WeightB);
    Write(P + 4, uint32(V.LifeMax));
    WriteFloat(P + 8, V.GravityScale);
    WriteFloat(P + 12, V.DynamicField4);
    WriteFloat(P + 16, V.DynamicField8);
    Write(P + 20, uint32(V.Field358));
    Write(P + 24, uint32(V.Field35C));
    WriteFloat(P + 28, V.Field368);
    WriteFloat(P + 32, V.Field364);
    WriteFloat(P + 36, V.Field36C);
    Write(P + 40, uint32(V.Field370));
    WriteFloat(P + 44, V.Field374);
    WriteFloat(P + 48, V.Field378);
    WriteFloat(P + 52, V.Field37C);
    WriteFloat(P + 56, V.StunnedDuration);
    WriteFloat(P + 60, V.Field384);
    WriteFloat(P + 64, V.Angle388Degrees);
    if (Off.bHasWaypointRefs)
    {
        Write(P + 68, uint32(V.Waypoint38C));
        Write(P + 72, uint32(V.Waypoint390));
    }
    uint8* S = Data.GetData() + Off.Suffix;
    Write(S, uint32(V.Field398));
    Write(S + 4, uint32(V.Field394));
    Write(S + 8, uint32(V.PathIndex));
    WriteFloat(S + 12, V.Field3AC);
    WriteFloat(S + 16, V.Field3B0);
    Write(S + 20, uint32(V.Field3B4));
    WriteFloat(S + 24, V.Field3B8);
    for (int32 I = 0; I < Off.Count; ++I)
        Write(Data.GetData() + Off.Entries + int64(I) * 4, uint32(V.ActorRefs[I]));
    uint8* T = Data.GetData() + Off.Tilt;
    WriteFloat(T, V.TiltField0);
    Write(T + 4, uint32(V.TiltMode));
    WriteFloat(T + 8, V.TiltValue);
    WriteFloat(T + 12, V.TiltSpringStrength);
    WriteFloat(T + 16, V.TiltDamping);
    WriteFloat(T + 20, V.PushValue);
    WriteFloat(T + 24, V.PushSpringStrength);
    WriteFloat(T + 28, V.PushDamping);
    // An edited word at the other version's count offset must not make the
    // output ambiguous to the importer. The exporter discards Bytes on failure.
    FKyaBlazerOffsets Written;
    return KyaReadBlazerOffsets(MakeArrayView(Data.GetData(), Data.Num()), Start, Size, Written) &&
        Written.bHasWaypointRefs == Off.bHasWaypointRefs && Written.Count == Off.Count;
}
