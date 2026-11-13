#include "Actors/KyaCareBoy.h"
#include "KyaLevelBank.h"
namespace
{
bool Valid(int64 Count, int64 Start, int64 Size)
{
    return Start >= 0 && Size >= 12 && Start <= Count && Size <= Count - Start;
}
uint32 Read(const uint8* P)
{
    return uint32(P[0]) | uint32(P[1]) << 8 | uint32(P[2]) << 16 | uint32(P[3]) << 24;
}
void Write(uint8* P, uint32 Value)
{
    for (int32 I = 0; I < 4; ++I) P[I] = uint8(Value >> (I * 8));
}
}
bool KyaReadCareBoyActor(FKyaActorPlacement& Actor, TConstArrayView<uint8> Data, int64 Start, int64 Size)
{
    if (!Valid(Data.Num(), Start, Size)) return false;
    const uint8* P = Data.GetData() + Start;
    uint32 Bits = Read(P);
    FMemory::Memcpy(&Actor.CareBoyWeightB, &Bits, 4);
    Actor.CareBoyField1DC = int32(Read(P + 4));
    Actor.CareBoyField1E0 = int32(Read(P + 8));
    Actor.ClassData.Reset();
    Actor.ClassData.Append(P + 12, int32(Size - 12));
    Actor.bHasCareBoyData = true;
    return true;
}
bool KyaWriteCareBoyActor(const FKyaActorPlacement& Actor, TArray<uint8>& Data, int64 Start, int64 Size)
{
    if (!Actor.bHasCareBoyData || !Valid(Data.Num(), Start, Size)) return false;
    uint8* P = Data.GetData() + Start;
    uint32 Bits;
    FMemory::Memcpy(&Bits, &Actor.CareBoyWeightB, 4);
    Write(P, Bits);
    Write(P + 4, uint32(Actor.CareBoyField1DC));
    Write(P + 8, uint32(Actor.CareBoyField1E0));
    return true;
}
