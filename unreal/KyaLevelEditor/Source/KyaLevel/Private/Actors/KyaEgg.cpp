#include "Actors/KyaEgg.h"
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
bool KyaReadEggOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size, FKyaEggOffsets& Out)
{
    Out = {};
    if (Start < 0 || Size < 20 || Start > Data.Num() || Size > Data.Num() - Start) return false;
    const uint32 Count = Read(Data.GetData() + Start + 8);
    if (Count > MAX_int32 || uint64(Count) > uint64(Size - 20) / 4) return false;
    Out.Count = int32(Count);
    Out.Entries = Start + 12;
    Out.Fields = Out.Entries + int64(Count) * 4;
    Out.Tail = Out.Fields + 8;
    return true;
}
bool KyaReadEggActor(FKyaActorPlacement& Actor, TConstArrayView<uint8> Data, int64 Start, int64 Size)
{
    FKyaEggOffsets Off;
    if (!KyaReadEggOffsets(Data, Start, Size, Off)) return false;
    const uint8* P = Data.GetData();
    Actor.EggWeightB = ReadFloat(P + Start);
    Actor.EggField1D8 = ReadFloat(P + Start + 4);
    Actor.EggPathIndices.Reset();
    for (int32 I = 0; I < Off.Count; ++I)
        Actor.EggPathIndices.Add(int32(Read(P + Off.Entries + int64(I) * 4)));
    Actor.EggField1DC = Read(P + Off.Fields);
    Actor.EggField1E0 = Read(P + Off.Fields + 4);
    Actor.ClassData.Reset();
    Actor.ClassData.Append(P + Off.Tail, int32(Start + Size - Off.Tail));
    Actor.bHasEggData = true;
    return true;
}
bool KyaWriteEggActor(const FKyaActorPlacement& Actor, TArray<uint8>& Data, int64 Start, int64 Size)
{
    FKyaEggOffsets Off;
    if (!Actor.bHasEggData || !KyaReadEggOffsets(MakeArrayView(Data.GetData(), Data.Num()), Start, Size, Off) ||
        Actor.EggPathIndices.Num() != Off.Count || Actor.EggField1DC < 0 || Actor.EggField1DC > MAX_uint32 ||
        Actor.EggField1E0 < 0 || Actor.EggField1E0 > MAX_uint32) return false;
    uint8* P = Data.GetData();
    WriteFloat(P + Start, Actor.EggWeightB);
    WriteFloat(P + Start + 4, Actor.EggField1D8);
    for (int32 I = 0; I < Off.Count; ++I)
        Write(P + Off.Entries + int64(I) * 4, uint32(Actor.EggPathIndices[I]));
    Write(P + Off.Fields, uint32(Actor.EggField1DC));
    Write(P + Off.Fields + 4, uint32(Actor.EggField1E0));
    return true;
}
