#include "Actors/KyaHunter.h"
#include "KyaLevelBank.h"

namespace
{
uint32 ReadWord(const uint8* P)
{
    return uint32(P[0]) | uint32(P[1]) << 8 | uint32(P[2]) << 16 | uint32(P[3]) << 24;
}
float ReadFloat(const uint8* P)
{
    uint32 Bits = ReadWord(P);
    float Value;
    FMemory::Memcpy(&Value, &Bits, 4);
    return Value;
}
void WriteWord(uint8* P, uint32 Value)
{
    for (int32 I = 0; I < 4; ++I) P[I] = uint8(Value >> (8 * I));
}
void WriteFloat(uint8* P, float Value)
{
    uint32 Bits;
    FMemory::Memcpy(&Bits, &Value, 4);
    WriteWord(P, Bits);
}
bool Valid(int64 Count, int64 Start, int64 Size)
{
    return Start >= 0 && Size >= 40 && Start <= Count && Size <= Count - Start;
}
}

bool KyaReadHunterActor(FKyaActorPlacement& Actor, TConstArrayView<uint8> Data, int64 Start, int64 Size)
{
    if (!Valid(Data.Num(), Start, Size)) return false;
    const uint8* P = Data.GetData() + Start;
    Actor.HunterWeight = ReadFloat(P);
    Actor.HunterLifeMax = ReadWord(P + 4);
    Actor.HunterGravityScale = ReadFloat(P + 8);
    Actor.HunterDynamicField4 = ReadFloat(P + 12);
    Actor.HunterDynamicField8 = ReadFloat(P + 16);
    Actor.HunterBoneId = ReadWord(P + 20);
    Actor.HunterField354 = ReadWord(P + 24);
    Actor.HunterActorRef = int32(ReadWord(P + 28));
    Actor.HunterField3A4 = ReadFloat(P + 32);
    Actor.HunterField350 = ReadFloat(P + 36);
    Actor.ClassData.Reset();
    Actor.ClassData.Append(P + 40, int32(Size - 40));
    Actor.bHasHunterData = true;
    return true;
}

bool KyaWriteHunterActor(const FKyaActorPlacement& Actor, TArray<uint8>& Data, int64 Start, int64 Size)
{
    if (!Valid(Data.Num(), Start, Size) || !Actor.bHasHunterData ||
        Actor.HunterLifeMax < 0 || Actor.HunterLifeMax > MAX_uint32 ||
        Actor.HunterBoneId < 0 || Actor.HunterBoneId > MAX_uint32 ||
        Actor.HunterField354 < 0 || Actor.HunterField354 > MAX_uint32) return false;
    uint8* P = Data.GetData() + Start;
    WriteFloat(P, Actor.HunterWeight);
    WriteWord(P + 4, uint32(Actor.HunterLifeMax));
    WriteFloat(P + 8, Actor.HunterGravityScale);
    WriteFloat(P + 12, Actor.HunterDynamicField4);
    WriteFloat(P + 16, Actor.HunterDynamicField8);
    WriteWord(P + 20, uint32(Actor.HunterBoneId));
    WriteWord(P + 24, uint32(Actor.HunterField354));
    WriteWord(P + 28, uint32(Actor.HunterActorRef));
    WriteFloat(P + 32, Actor.HunterField3A4);
    WriteFloat(P + 36, Actor.HunterField350);
    return true;
}
