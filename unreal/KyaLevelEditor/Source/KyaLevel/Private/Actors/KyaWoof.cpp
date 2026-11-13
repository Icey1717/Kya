#include "Actors/KyaWoof.h"
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
}
bool KyaReadWoofOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size, FKyaWoofOffsets& Out)
{
    Out = {};
    if (Start < 0 || Size < 96 || Start > Data.Num() || Size > Data.Num() - Start) return false;
    const uint32 Count = Read(Data.GetData() + Start + 56);
    if (Count > MAX_int32 || uint64(Count) > uint64(Size - 96) / 4) return false;
    Out.Count = int32(Count);
    Out.Entries = Start + 60;
    Out.Suffix = Out.Entries + int64(Count) * 4;
    Out.Tail = Out.Suffix + 36;
    return true;
}
bool KyaReadWoofActor(FKyaActorPlacement& Actor, TConstArrayView<uint8> Data, int64 Start, int64 Size)
{
    FKyaWoofOffsets Off;
    if (!KyaReadWoofOffsets(Data, Start, Size, Off)) return false;
    FKyaWoofData& V = Actor.WoofData;
    V = {};
    const uint8* P = Data.GetData() + Start;
    V.WeightB = ReadFloat(P + 0);
    V.LifeMax = Read(P + 4);
    V.GravityScale = ReadFloat(P + 8);
    V.DynamicField4 = ReadFloat(P + 12);
    V.DynamicField8 = ReadFloat(P + 16);
    V.Field420 = Read(P + 20);
    V.Field424 = Read(P + 24);
    V.Bone428 = Read(P + 28);
    V.Field390 = ReadFloat(P + 32);
    V.Field394 = ReadFloat(P + 36);
    V.Field398 = Read(P + 40);
    V.Field39C = Read(P + 44);
    V.PathIndex = int32(Read(P + 48));
    V.PathfindingId = int32(Read(P + 52));
    for (int32 I = 0; I < Off.Count; ++I)
        V.ActorRefs.Add(int32(Read(Data.GetData() + Off.Entries + int64(I) * 4)));
    const uint8* S = Data.GetData() + Off.Suffix;
    V.Field418 = ReadFloat(S + 0);
    V.Field414 = Read(S + 4);
    V.Field410 = Read(S + 8);
    V.Waypoint3A4 = int32(Read(S + 12));
    V.VisionRange = ReadFloat(S + 16);
    V.VisionAngleDegrees = ReadFloat(S + 20);
    V.VisionExtent38 = ReadFloat(S + 24);
    V.VisionExtent3C = ReadFloat(S + 28);
    V.VisionFlags = Read(S + 32);
    Actor.ClassData.Reset();
    Actor.ClassData.Append(Data.GetData() + Off.Tail, int32(Start + Size - Off.Tail));
    Actor.bHasWoofData = true;
    return true;
}
bool KyaWriteWoofActor(const FKyaActorPlacement& Actor, TArray<uint8>& Data, int64 Start, int64 Size)
{
    FKyaWoofOffsets Off;
    const FKyaWoofData& V = Actor.WoofData;
    if (!Actor.bHasWoofData || !KyaReadWoofOffsets(MakeArrayView(Data.GetData(), Data.Num()), Start, Size, Off) ||
        V.ActorRefs.Num() != Off.Count) return false;
    const int64 Words[] = {V.LifeMax, V.Field420, V.Field424, V.Bone428, V.Field398, V.Field39C, V.Field414, V.Field410, V.VisionFlags};
    for (int64 Word : Words) if (Word < 0 || Word > MAX_uint32) return false;
    uint8* P = Data.GetData() + Start;
    WriteFloat(P + 0, V.WeightB);
    Write(P + 4, uint32(V.LifeMax));
    WriteFloat(P + 8, V.GravityScale);
    WriteFloat(P + 12, V.DynamicField4);
    WriteFloat(P + 16, V.DynamicField8);
    Write(P + 20, uint32(V.Field420));
    Write(P + 24, uint32(V.Field424));
    Write(P + 28, uint32(V.Bone428));
    WriteFloat(P + 32, V.Field390);
    WriteFloat(P + 36, V.Field394);
    Write(P + 40, uint32(V.Field398));
    Write(P + 44, uint32(V.Field39C));
    Write(P + 48, uint32(V.PathIndex));
    Write(P + 52, uint32(V.PathfindingId));
    for (int32 I = 0; I < Off.Count; ++I)
        Write(Data.GetData() + Off.Entries + int64(I) * 4, uint32(V.ActorRefs[I]));
    uint8* S = Data.GetData() + Off.Suffix;
    WriteFloat(S + 0, V.Field418);
    Write(S + 4, uint32(V.Field414));
    Write(S + 8, uint32(V.Field410));
    Write(S + 12, uint32(V.Waypoint3A4));
    WriteFloat(S + 16, V.VisionRange);
    WriteFloat(S + 20, V.VisionAngleDegrees);
    WriteFloat(S + 24, V.VisionExtent38);
    WriteFloat(S + 28, V.VisionExtent3C);
    Write(S + 32, uint32(V.VisionFlags));
    return true;
}
