#include "Actors/KyaMoney.h"

#include "KyaBinaryHelpers.h"
#include "KyaLevelBank.h"

bool KyaReadMoneyOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size,
    FKyaMoneyOffsets& Out)
{
    Out = FKyaMoneyOffsets{};
    if (!KyaRangeValid(Data, Start, Size) || Size < 16) return false;
    Out.WeightB = Start;
    Out.MoneyValue = Start + 4;
    Out.Sound = Start + 8;
    Out.Material = Start + 12;
    Out.Tail = Start + 16;
    return true;
}

bool KyaReadMoneyActor(FKyaActorPlacement& Out, TConstArrayView<uint8> Data, int64 Start,
    int64 Size)
{
    FKyaMoneyOffsets Off;
    if (!KyaReadMoneyOffsets(Data, Start, Size, Off)) return false;
    const uint8* Bytes = Data.GetData();
    Out.MoneyWeightB = KyaReadF32(Bytes + Off.WeightB);
    Out.MoneyValue = int32(KyaReadU32(Bytes + Off.MoneyValue));
    Out.MoneySoundRef = int32(KyaReadU32(Bytes + Off.Sound));
    Out.MoneyMaterialId = int32(KyaReadU32(Bytes + Off.Material));

    Out.bHasMoneyData = true;
    Out.ClassData.Reset();
    if (Start + Size > Off.Tail)
        Out.ClassData.Append(Bytes + Off.Tail, int32(Start + Size - Off.Tail));
    return true;
}

bool KyaWriteMoneyActor(const FKyaActorPlacement& Actor, TArray<uint8>& Data, int64 Start,
    int64 Size)
{
    FKyaMoneyOffsets Off;
    if (!KyaReadMoneyOffsets(MakeArrayView(Data.GetData(), Data.Num()), Start, Size, Off))
        return false;
    KyaWriteF32(Data, Off.WeightB, Actor.MoneyWeightB);
    KyaWriteU32(Data, Off.MoneyValue, uint32(Actor.MoneyValue));
    KyaWriteU32(Data, Off.Sound, uint32(Actor.MoneySoundRef));
    KyaWriteU32(Data, Off.Material, uint32(Actor.MoneyMaterialId));
    return true;
}

bool KyaReadMoneyFlockOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size,
    FKyaMoneyFlockOffsets& Out)
{
    Out = FKyaMoneyFlockOffsets{};
    if (!KyaRangeValid(Data, Start, Size) || Size != 4) return false;
    Out.PathIndex = Start;
    Out.Tail = Start + 4;
    return true;
}

bool KyaReadMoneyBehaviour(FKyaActorBehaviour& Out, int32 Id, TConstArrayView<uint8> Data,
    int64 Start, int32 Size)
{
    if (Id == 2)
    {
        FKyaMoneyFlockOffsets Off;
        if (!KyaReadMoneyFlockOffsets(Data, Start, Size, Off)) return false;
        Out.MoneyFlockPathIndex = int32(KyaReadU32(Data.GetData() + Off.PathIndex));
        Out.bHasTypedMoneyFlockBehaviour = true;
        return true;
    }
    if (Id == 3)
    {
        // CBehaviourMoneyAddOn::Create is empty.
        return Size == 0;
    }
    return false;
}

bool KyaWriteMoneyBehaviour(const FKyaActorBehaviour& Behaviour, TArray<uint8>& Data,
    int64 Start, int32 Size)
{
    if (Behaviour.Id == 2)
    {
        FKyaMoneyFlockOffsets Off;
        if (!KyaReadMoneyFlockOffsets(MakeArrayView(Data.GetData(), Data.Num()), Start, Size,
            Off)) return false;
        KyaWriteU32(Data, Off.PathIndex, uint32(Behaviour.MoneyFlockPathIndex));
        return true;
    }
    if (Behaviour.Id == 3) return Size == 0;
    return false;
}
