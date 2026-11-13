#include "Actors/KyaShockerCmd.h"
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

bool KyaReadShockerCmdOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size, FKyaShockerCmdOffsets& Out)
{
    Out = {};
    if (Start < 0 || Size < 8 || Start > Data.Num() || Size > Data.Num() - Start) return false;
    const uint8* P = Data.GetData();
    const int64 End = Start + Size;
    const uint32 Count = Read(P + Start + 4);
    int64 Pos = Start + 8;
    // Both counts are signed in Create. Negative counts are invalid allocations.
    if (Count > MAX_int32 || uint64(Count) > uint64(End - Pos) / 8) return false;
    for (uint32 I = 0; I < Count; ++I)
    {
        if (End - Pos < 8) return false;
        const uint32 Floats = Read(P + Pos + 4);
        if (Floats > MAX_int32 || uint64(Floats) > uint64(End - Pos - 8) / 4) return false;
        Out.Sequences.Add(Pos);
        Out.Counts.Add(int32(Floats));
        Pos += 8 + int64(Floats) * 4;
    }
    Out.Tail = Pos;
    return true;
}

bool KyaReadShockerCmdActor(FKyaActorPlacement& Actor, TConstArrayView<uint8> Data, int64 Start, int64 Size)
{
    FKyaShockerCmdOffsets Off;
    if (!KyaReadShockerCmdOffsets(Data, Start, Size, Off)) return false;
    const uint8* P = Data.GetData();
    Actor.ShockerCmdField168 = Read(P + Start);
    Actor.ShockerCmdSequences.Reset();
    for (int32 I = 0; I < Off.Sequences.Num(); ++I)
    {
        FKyaShockerCmdSequence& Sequence = Actor.ShockerCmdSequences.AddDefaulted_GetRef();
        const int64 Pos = Off.Sequences[I];
        Sequence.ActorRef = int32(Read(P + Pos));
        for (int32 J = 0; J < Off.Counts[I]; ++J)
        {
            uint32 Bits = Read(P + Pos + 8 + int64(J) * 4);
            float Value;
            FMemory::Memcpy(&Value, &Bits, 4);
            Sequence.Values.Add(Value);
        }
    }
    Actor.ClassData.Reset();
    Actor.ClassData.Append(P + Off.Tail, int32(Start + Size - Off.Tail));
    Actor.bHasShockerCmdData = true;
    return true;
}

bool KyaWriteShockerCmdActor(const FKyaActorPlacement& Actor, TArray<uint8>& Data, int64 Start, int64 Size)
{
    FKyaShockerCmdOffsets Off;
    if (!Actor.bHasShockerCmdData || Actor.ShockerCmdField168 < 0 || Actor.ShockerCmdField168 > MAX_uint32 ||
        !KyaReadShockerCmdOffsets(MakeArrayView(Data.GetData(), Data.Num()), Start, Size, Off) ||
        Actor.ShockerCmdSequences.Num() != Off.Sequences.Num()) return false;
    for (int32 I = 0; I < Off.Sequences.Num(); ++I)
        if (Actor.ShockerCmdSequences[I].Values.Num() != Off.Counts[I]) return false;
    Write(Data.GetData() + Start, uint32(Actor.ShockerCmdField168));
    for (int32 I = 0; I < Off.Sequences.Num(); ++I)
    {
        const FKyaShockerCmdSequence& Sequence = Actor.ShockerCmdSequences[I];
        uint8* P = Data.GetData() + Off.Sequences[I];
        Write(P, uint32(Sequence.ActorRef));
        for (int32 J = 0; J < Sequence.Values.Num(); ++J)
        {
            uint32 Bits;
            FMemory::Memcpy(&Bits, &Sequence.Values[J], 4);
            Write(P + 8 + int64(J) * 4, Bits);
        }
    }
    return true;
}
