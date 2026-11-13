#include "Actors/KyaWoodMonster.h"
#include "KyaLevelBank.h"

namespace
{
bool Valid(int64 Count, int64 Start, int64 Size, int64 Minimum)
{
    return Start >= 0 && Size >= Minimum && Start <= Count && Size <= Count - Start;
}
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

bool KyaReadWoodMonsterActor(FKyaActorPlacement& Actor, TConstArrayView<uint8> Data, int64 Start, int64 Size)
{
    if (!Valid(Data.Num(), Start, Size, 16)) return false;
    const uint8* P = Data.GetData() + Start;
    Actor.WoodMonsterField160 = FVector4(ReadFloat(P), ReadFloat(P + 4), ReadFloat(P + 8), ReadFloat(P + 12));
    Actor.bHasWoodMonsterData = true;
    Actor.ClassData.Reset();
    Actor.ClassData.Append(P + 16, int32(Size - 16));
    return true;
}

bool KyaWriteWoodMonsterActor(const FKyaActorPlacement& Actor, TArray<uint8>& Data, int64 Start, int64 Size)
{
    if (!Actor.bHasWoodMonsterData || !Valid(Data.Num(), Start, Size, 16)) return false;
    for (int32 I = 0; I < 4; ++I) WriteFloat(Data.GetData() + Start + I * 4, float(Actor.WoodMonsterField160[I]));
    return true;
}

bool KyaReadWoodMonsterBehaviour(FKyaActorBehaviour& Behaviour, TConstArrayView<uint8> Data, int64 Start, int64 Size)
{
    if (Behaviour.Id != 2 || Size != 12 || !Valid(Data.Num(), Start, Size, 12)) return false;
    const uint8* P = Data.GetData() + Start;
    Behaviour.WoodMonsterField8 = ReadFloat(P);
    Behaviour.WoodMonsterWaypointRef = int32(Read(P + 4));
    Behaviour.WoodMonsterField10 = int32(Read(P + 8));
    Behaviour.bHasTypedWoodMonsterBehaviour = true;
    return true;
}

bool KyaWriteWoodMonsterBehaviour(const FKyaActorBehaviour& Behaviour, TArray<uint8>& Data, int64 Start, int64 Size)
{
    if (!Behaviour.bHasTypedWoodMonsterBehaviour || Behaviour.Id != 2 || Size != 12 || !Valid(Data.Num(), Start, Size, 12)) return false;
    uint8* P = Data.GetData() + Start;
    WriteFloat(P, Behaviour.WoodMonsterField8);
    Write(P + 4, uint32(Behaviour.WoodMonsterWaypointRef));
    Write(P + 8, uint32(Behaviour.WoodMonsterField10));
    return true;
}
