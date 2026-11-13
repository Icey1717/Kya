#include "Actors/KyaFx.h"

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

bool KyaReadFxOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size, FKyaFxOffsets& Out)
{
    Out = FKyaFxOffsets{};
    if (!KyaRangeValid(Data, Start, Size) || Size < 8) return false;
    Out.Field19C = Start;
    Out.Field1A0 = Start + 4;
    Out.Tail = Start + 8;
    return true;
}

bool KyaReadFxActor(FKyaActorPlacement& Out, TConstArrayView<uint8> Data, int64 Start,
    int64 Size)
{
    FKyaFxOffsets Off;
    if (!KyaReadFxOffsets(Data, Start, Size, Off)) return false;
    const uint8* Bytes = Data.GetData();
    Out.FxField19C = KyaReadF32(Bytes + Off.Field19C);
    Out.FxField1A0 = KyaReadF32(Bytes + Off.Field1A0);

    Out.bHasFxData = true;
    Out.ClassData.Reset();
    if (Start + Size > Off.Tail)
        Out.ClassData.Append(Bytes + Off.Tail, int32(Start + Size - Off.Tail));
    return true;
}

bool KyaWriteFxActor(const FKyaActorPlacement& Actor, TArray<uint8>& Data, int64 Start,
    int64 Size)
{
    FKyaFxOffsets Off;
    if (!KyaReadFxOffsets(MakeArrayView(Data.GetData(), Data.Num()), Start, Size, Off))
        return false;
    KyaWriteF32(Data, Off.Field19C, Actor.FxField19C);
    KyaWriteF32(Data, Off.Field1A0, Actor.FxField1A0);
    return true;
}

bool KyaReadFxBehaviour(int32 Id, TConstArrayView<uint8> Data, int64 Start, int64 Size,
    FKyaFxBehaviourValues& Out)
{
    Out = FKyaFxBehaviourValues{};
    if (!KyaRangeValid(Data, Start, Size)) return false;
    const uint8* Bytes = Data.GetData();
    if (Id == 2)
    {
        if (Size != 4) return false;
        Out.Value = int32(KyaReadU32(Bytes + Start));
        return true;
    }
    if (Id == 3)
    {
        if (Size != 8) return false;
        Out.Value = int32(KyaReadU32(Bytes + Start));
        Out.PathFollowIndex = int32(KyaReadU32(Bytes + Start + 4));
        return true;
    }
    if (Id == 4)
    {
        if (Size < 4) return false;
        const uint32 Count = KyaReadU32(Bytes + Start);
        if (Count > uint64(Size - 4) / 4) return false;
        for (uint32 Index = 0; Index < Count; ++Index)
            Out.UnknownValues.Add(int32(KyaReadU32(Bytes + Start + 4 + int64(Index) * 4)));
        return true;
    }
    return false;
}

bool KyaReadFxBehaviourValue(FKyaActorBehaviour& Out, int32 Id, TConstArrayView<uint8> Data,
    int64 Start, int32 Size)
{
    FKyaFxBehaviourValues Values;
    if (!KyaReadFxBehaviour(Id, Data, Start, Size, Values)) return false;
    Out.FxBehaviourValue = Values.Value;
    Out.FxPathFollowIndex = Values.PathFollowIndex;
    Out.FxUnknownValues = MoveTemp(Values.UnknownValues);
    Out.bHasTypedFxBehaviour = true;
    return true;
}

bool KyaWriteFxBehaviour(const FKyaActorBehaviour& Behaviour, TArray<uint8>& Data,
    int64 Start, int32 Size)
{
    FKyaFxBehaviourValues Values;
    if (!KyaReadFxBehaviour(Behaviour.Id, MakeArrayView(Data.GetData(), Data.Num()), Start,
        Size, Values)) return false;
    if (Behaviour.Id == 4 && Behaviour.FxUnknownValues.Num() != Values.UnknownValues.Num())
        return false;
    if (Behaviour.Id == 4)
        KyaWriteU32(Data, Start, uint32(Values.UnknownValues.Num()));
    else
        KyaWriteU32(Data, Start, uint32(Behaviour.FxBehaviourValue));
    if (Behaviour.Id == 3) KyaWriteU32(Data, Start + 4, uint32(Behaviour.FxPathFollowIndex));
    if (Behaviour.Id == 4)
        for (int32 Index = 0; Index < Behaviour.FxUnknownValues.Num(); ++Index)
            KyaWriteU32(Data, Start + 4 + int64(Index) * 4, uint32(Behaviour.FxUnknownValues[Index]));
    return true;
}
