#include "Actors/KyaAmortosCmd.h"
#include "KyaLevelBank.h"
namespace
{
uint32 Read(const uint8* P)
{
    return uint32(P[0]) | uint32(P[1]) << 8 | uint32(P[2]) << 16 | uint32(P[3]) << 24;
}
}
bool KyaReadAmortosCmdOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size, FKyaAmortosCmdOffsets& Out)
{
    Out = {};
    if (Start < 0 || Size < 4 || Start > Data.Num() || Size > Data.Num() - Start) return false;
    const uint32 Count = Read(Data.GetData() + Start);
    if (Count > MAX_int32 || uint64(Count) > uint64(Size - 4) / 4) return false;
    Out.Count = int32(Count);
    Out.Entries = Start + 4;
    Out.Tail = Out.Entries + int64(Count) * 4;
    return true;
}
bool KyaReadAmortosCmdActor(FKyaActorPlacement& Actor, TConstArrayView<uint8> Data, int64 Start, int64 Size)
{
    FKyaAmortosCmdOffsets Off;
    if (!KyaReadAmortosCmdOffsets(Data, Start, Size, Off)) return false;
    Actor.AmortosCmdActorRefs.Reset();
    for (int32 I = 0; I < Off.Count; ++I)
        Actor.AmortosCmdActorRefs.Add(int32(Read(Data.GetData() + Off.Entries + int64(I) * 4)));
    Actor.ClassData.Reset();
    Actor.ClassData.Append(Data.GetData() + Off.Tail, int32(Start + Size - Off.Tail));
    Actor.bHasAmortosCmdData = true;
    return true;
}
bool KyaWriteAmortosCmdActor(const FKyaActorPlacement& Actor, TArray<uint8>& Data, int64 Start, int64 Size)
{
    FKyaAmortosCmdOffsets Off;
    if (!Actor.bHasAmortosCmdData || !KyaReadAmortosCmdOffsets(MakeArrayView(Data.GetData(), Data.Num()), Start, Size, Off) ||
        Actor.AmortosCmdActorRefs.Num() != Off.Count) return false;
    for (int32 I = 0; I < Off.Count; ++I)
    {
        const uint32 Value = uint32(Actor.AmortosCmdActorRefs[I]);
        for (int32 B = 0; B < 4; ++B) Data[int32(Off.Entries + int64(I) * 4) + B] = uint8(Value >> (8 * B));
    }
    return true;
}
