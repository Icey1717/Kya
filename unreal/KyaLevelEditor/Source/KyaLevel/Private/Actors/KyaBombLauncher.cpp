#include "Actors/KyaBombLauncher.h"
#include "KyaLevelBank.h"
namespace
{
uint32 Read(const uint8* P)
{
    return uint32(P[0]) | uint32(P[1]) << 8 | uint32(P[2]) << 16 | uint32(P[3]) << 24;
}
void Write(uint8* P, uint32 Value)
{
    for (int32 I = 0; I < 4; ++I) P[I] = uint8(Value >> (I * 8));
}
}
bool KyaReadBombLauncherOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size, FKyaBombLauncherOffsets& Out)
{
    Out = {};
    if (Start < 0 || Size < 8 || Start > Data.Num() || Size > Data.Num() - Start) return false;
    const uint32 Count = Read(Data.GetData() + Start + 4);
    if (Count > MAX_int32 || uint64(Count) > uint64(Size - 8) / 4) return false;
    Out.Count = int32(Count);
    Out.Entries = Start + 8;
    Out.Tail = Out.Entries + int64(Count) * 4;
    return true;
}
bool KyaReadBombLauncherActor(FKyaActorPlacement& Actor, TConstArrayView<uint8> Data, int64 Start, int64 Size)
{
    FKyaBombLauncherOffsets Off;
    if (!KyaReadBombLauncherOffsets(Data, Start, Size, Off)) return false;
    uint32 Bits = Read(Data.GetData() + Start);
    FMemory::Memcpy(&Actor.BombLauncherField168, &Bits, 4);
    Actor.BombLauncherActorRefs.Reset();
    for (int32 I = 0; I < Off.Count; ++I)
        Actor.BombLauncherActorRefs.Add(int32(Read(Data.GetData() + Off.Entries + int64(I) * 4)));
    Actor.ClassData.Reset();
    Actor.ClassData.Append(Data.GetData() + Off.Tail, int32(Start + Size - Off.Tail));
    Actor.bHasBombLauncherData = true;
    return true;
}
bool KyaWriteBombLauncherActor(const FKyaActorPlacement& Actor, TArray<uint8>& Data, int64 Start, int64 Size)
{
    FKyaBombLauncherOffsets Off;
    if (!Actor.bHasBombLauncherData || !KyaReadBombLauncherOffsets(MakeArrayView(Data.GetData(), Data.Num()), Start, Size, Off) ||
        Actor.BombLauncherActorRefs.Num() != Off.Count) return false;
    uint32 Bits;
    FMemory::Memcpy(&Bits, &Actor.BombLauncherField168, 4);
    Write(Data.GetData() + Start, Bits);
    for (int32 I = 0; I < Off.Count; ++I)
        Write(Data.GetData() + Off.Entries + int64(I) * 4, uint32(Actor.BombLauncherActorRefs[I]));
    return true;
}
