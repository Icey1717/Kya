#include "Actors/KyaAcceleratos.h"
#include "KyaLevelBank.h"

namespace
{
bool Valid(int64 Count, int64 Start, int64 Size)
{
    return Start >= 0 && Size >= 20 && Start <= Count && Size <= Count - Start;
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
bool KyaReadAcceleratosActor(FKyaActorPlacement& Actor, TConstArrayView<uint8> Data, int64 Start, int64 Size)
{
    if (!Valid(Data.Num(), Start, Size)) return false;
    const uint8* P = Data.GetData() + Start;
    float* Fields[] = {&Actor.AcceleratosField0, &Actor.AcceleratosField4, &Actor.AcceleratosField8};
    for (int32 I = 0; I < 3; ++I)
    {
        const uint32 Bits = Read(P + I * 4);
        FMemory::Memcpy(Fields[I], &Bits, 4);
    }
    Actor.AcceleratosSoundC = int32(Read(P + 12));
    Actor.AcceleratosSound10 = int32(Read(P + 16));
    Actor.bHasAcceleratosData = true;
    Actor.ClassData.Reset();
    Actor.ClassData.Append(P + 20, int32(Size - 20));
    return true;
}
bool KyaWriteAcceleratosActor(const FKyaActorPlacement& Actor, TArray<uint8>& Data, int64 Start, int64 Size)
{
    if (!Actor.bHasAcceleratosData || !Valid(Data.Num(), Start, Size)) return false;
    uint8* P = Data.GetData() + Start;
    const float Fields[] = {Actor.AcceleratosField0, Actor.AcceleratosField4, Actor.AcceleratosField8};
    for (int32 I = 0; I < 3; ++I)
    {
        uint32 Bits;
        FMemory::Memcpy(&Bits, &Fields[I], 4);
        Write(P + I * 4, Bits);
    }
    Write(P + 12, uint32(Actor.AcceleratosSoundC));
    Write(P + 16, uint32(Actor.AcceleratosSound10));
    return true;
}
