#include "Actors/KyaRune.h"

#include "KyaLevelBank.h"

namespace
{
uint32 KyaReadU32(const uint8* P)
{
    return uint32(P[0]) | (uint32(P[1]) << 8) | (uint32(P[2]) << 16) | (uint32(P[3]) << 24);
}

float KyaReadF32(const uint8* P)
{
    const uint32 Bits = KyaReadU32(P);
    float Value;
    FMemory::Memcpy(&Value, &Bits, sizeof(Value));
    return Value;
}

void KyaWriteU32(TArray<uint8>& Data, int64 Offset, uint32 Value)
{
    for (int32 Byte = 0; Byte < 4; ++Byte) Data[int32(Offset) + Byte] = uint8(Value >> (Byte * 8));
}

void KyaWriteF32(TArray<uint8>& Data, int64 Offset, float Value)
{
    uint32 Bits = 0;
    FMemory::Memcpy(&Bits, &Value, sizeof(Bits));
    KyaWriteU32(Data, Offset, Bits);
}

bool KyaRangeValid(TConstArrayView<uint8> Data, int64 Start, int64 Size)
{
    return Start >= 0 && Size >= 0 && Start <= Data.Num() && Size <= Data.Num() - Start;
}
}

bool KyaReadRuneOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size,
    FKyaRuneOffsets& Out)
{
    Out = FKyaRuneOffsets{};
    if (!KyaRangeValid(Data, Start, Size) || Size < 24) return false;
    Out.Cinematic = Start;
    Out.TriggerDistance = Start + 4;
    Out.ScenVar = Start + 8;
    Out.Fx = Start + 12;
    Out.MaterialA = Start + 16;
    Out.MaterialB = Start + 20;
    Out.Tail = Start + 24;
    return true;
}

bool KyaReadRuneActor(FKyaActorPlacement& Out, TConstArrayView<uint8> Data, int64 Start,
    int64 Size)
{
    FKyaRuneOffsets Off;
    if (!KyaReadRuneOffsets(Data, Start, Size, Off)) return false;
    const uint8* Bytes = Data.GetData();
    Out.RuneCinematicId = int32(KyaReadU32(Bytes + Off.Cinematic));
    Out.RuneTriggerDistance = KyaReadF32(Bytes + Off.TriggerDistance);
    Out.RuneScenVarId = int32(KyaReadU32(Bytes + Off.ScenVar));
    Out.RuneFxId = int32(KyaReadU32(Bytes + Off.Fx));
    Out.RuneMaterialIdA = int32(KyaReadU32(Bytes + Off.MaterialA));
    Out.RuneMaterialIdB = int32(KyaReadU32(Bytes + Off.MaterialB));

    Out.bHasRuneData = true;
    Out.ClassData.Reset();
    if (Start + Size > Off.Tail)
        Out.ClassData.Append(Bytes + Off.Tail, int32(Start + Size - Off.Tail));
    return true;
}

bool KyaWriteRuneActor(const FKyaActorPlacement& Actor, TArray<uint8>& Data, int64 Start,
    int64 Size)
{
    FKyaRuneOffsets Off;
    if (!KyaReadRuneOffsets(MakeArrayView(Data.GetData(), Data.Num()), Start, Size, Off))
        return false;
    KyaWriteU32(Data, Off.Cinematic, uint32(Actor.RuneCinematicId));
    KyaWriteF32(Data, Off.TriggerDistance, Actor.RuneTriggerDistance);
    KyaWriteU32(Data, Off.ScenVar, uint32(Actor.RuneScenVarId));
    KyaWriteU32(Data, Off.Fx, uint32(Actor.RuneFxId));
    KyaWriteU32(Data, Off.MaterialA, uint32(Actor.RuneMaterialIdA));
    KyaWriteU32(Data, Off.MaterialB, uint32(Actor.RuneMaterialIdB));
    return true;
}
