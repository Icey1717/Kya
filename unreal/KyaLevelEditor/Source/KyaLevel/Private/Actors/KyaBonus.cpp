#include "Actors/KyaBonus.h"

#include "KyaBinaryHelpers.h"
#include "KyaLevelBank.h"

bool KyaReadBonusOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size, FKyaBonusOffsets& Out)
{
    Out = FKyaBonusOffsets{};
    if (!KyaRangeValid(Data, Start, Size) || Size < 16) return false;
    Out.WeightB = Start;
    Out.Sound = Start + 4;
    Out.FlareMaterial = Start + 8;
    Out.AnimMaterial = Start + 12;
    Out.Tail = Start + 16;
    return true;
}

bool KyaReadBonusActor(FKyaActorPlacement& Out, TConstArrayView<uint8> Data, int64 Start,
    int64 Size)
{
    FKyaBonusOffsets Off;
    if (!KyaReadBonusOffsets(Data, Start, Size, Off)) return false;
    const uint8* Bytes = Data.GetData();
    Out.BonusWeightB = KyaReadF32(Bytes + Off.WeightB);
    Out.BonusSoundRef = int32(KyaReadU32(Bytes + Off.Sound));
    Out.BonusFlareMaterialId = int32(KyaReadU32(Bytes + Off.FlareMaterial));
    Out.BonusAnimMaterialId = int32(KyaReadU32(Bytes + Off.AnimMaterial));
    Out.bHasBonusData = true;
    Out.ClassData.Reset();
    if (Start + Size > Off.Tail)
        Out.ClassData.Append(Bytes + Off.Tail, int32(Start + Size - Off.Tail));
    return true;
}

bool KyaWriteBonusActor(const FKyaActorPlacement& Actor, TArray<uint8>& Data, int64 Start,
    int64 Size)
{
    FKyaBonusOffsets Off;
    if (!KyaReadBonusOffsets(MakeArrayView(Data.GetData(), Data.Num()), Start, Size, Off))
        return false;
    KyaWriteF32(Data, Off.WeightB, Actor.BonusWeightB);
    KyaWriteU32(Data, Off.Sound, uint32(Actor.BonusSoundRef));
    KyaWriteU32(Data, Off.FlareMaterial, uint32(Actor.BonusFlareMaterialId));
    KyaWriteU32(Data, Off.AnimMaterial, uint32(Actor.BonusAnimMaterialId));
    return true;
}

bool KyaReadBonusBehaviour(int32 Id, TConstArrayView<uint8> Data, int64 Start, int32 Size,
    FKyaBonusBehaviourValues& Out)
{
    Out = FKyaBonusBehaviourValues{};
    if (!KyaRangeValid(Data, Start, int64(Size))) return false;
    const uint8* Bytes = Data.GetData();
    if (Id == 3)
    {
        if (Size != 32) return false;
        Out.Field14 = KyaReadF32(Bytes + Start);
        Out.Field1B8 = KyaReadF32(Bytes + Start + 4);
        Out.Field1B4 = KyaReadF32(Bytes + Start + 8);
        Out.MaterialId = int32(KyaReadU32(Bytes + Start + 12));
        Out.Field1B0 = KyaReadF32(Bytes + Start + 16);
        Out.FxTailCount = int32(KyaReadU32(Bytes + Start + 20));
        Out.G2DIndex = int32(KyaReadU32(Bytes + Start + 24));
        Out.TurnValue = KyaReadF32(Bytes + Start + 28);
        return true;
    }
    if (Id == 4)
    {
        // CBehaviourBonusPath::Create: alone prefix + path reader and tail.
        if (Size != 52) return false;
        Out.Field14 = KyaReadF32(Bytes + Start);
        Out.Field1B8 = KyaReadF32(Bytes + Start + 4);
        Out.Field1B4 = KyaReadF32(Bytes + Start + 8);
        Out.MaterialId = int32(KyaReadU32(Bytes + Start + 12));
        Out.Field1B0 = KyaReadF32(Bytes + Start + 16);
        Out.FxTailCount = int32(KyaReadU32(Bytes + Start + 20));
        Out.G2DIndex = int32(KyaReadU32(Bytes + Start + 24));
        Out.PathIndex = int32(KyaReadU32(Bytes + Start + 28));
        Out.PathField1F4 = KyaReadF32(Bytes + Start + 32);
        Out.PathField1F8 = KyaReadF32(Bytes + Start + 36);
        Out.PathField1FC = KyaReadF32(Bytes + Start + 40);
        Out.PathField200 = KyaReadF32(Bytes + Start + 44);
        Out.PathField204 = int32(KyaReadU32(Bytes + Start + 48));
        return true;
    }
    if (Id == 5)
    {
        if (Size != 4) return false;
        Out.FlockPathIndex = int32(KyaReadU32(Bytes + Start));
        return true;
    }
    if (Id == 6)
    {
        if (Size != 4) return false;
        Out.AddOnField18 = KyaReadF32(Bytes + Start);
        return true;
    }
    return false;
}

bool KyaReadBonusBehaviourRecord(FKyaActorBehaviour& Out, int32 Id, TConstArrayView<uint8> Data,
    int64 Start, int32 Size)
{
    FKyaBonusBehaviourValues Values;
    if (!KyaReadBonusBehaviour(Id, Data, Start, Size, Values)) return false;
    Out.BonusField14 = Values.Field14;
    Out.BonusField1B8 = Values.Field1B8;
    Out.BonusField1B4 = Values.Field1B4;
    Out.BonusMaterialId = Values.MaterialId;
    Out.BonusField1B0 = Values.Field1B0;
    Out.BonusFxTailCount = Values.FxTailCount;
    Out.BonusG2DIndex = Values.G2DIndex;
    Out.BonusTurnValue = Values.TurnValue;
    Out.BonusFlockPathIndex = Values.FlockPathIndex;
    Out.BonusAddOnField18 = Values.AddOnField18;
    Out.BonusPathIndex = Values.PathIndex;
    Out.BonusPathField1F4 = Values.PathField1F4;
    Out.BonusPathField1F8 = Values.PathField1F8;
    Out.BonusPathField1FC = Values.PathField1FC;
    Out.BonusPathField200 = Values.PathField200;
    Out.BonusPathField204 = Values.PathField204;
    Out.bHasTypedBonusBehaviour = true;
    return true;
}

bool KyaWriteBonusBehaviour(const FKyaActorBehaviour& Behaviour, TArray<uint8>& Data, int64 Start,
    int32 Size)
{
    if (Behaviour.Id == 3)
    {
        if (Size != 32) return false;
        KyaWriteF32(Data, Start, Behaviour.BonusField14);
        KyaWriteF32(Data, Start + 4, Behaviour.BonusField1B8);
        KyaWriteF32(Data, Start + 8, Behaviour.BonusField1B4);
        KyaWriteU32(Data, Start + 12, uint32(Behaviour.BonusMaterialId));
        KyaWriteF32(Data, Start + 16, Behaviour.BonusField1B0);
        KyaWriteU32(Data, Start + 20, uint32(Behaviour.BonusFxTailCount));
        KyaWriteU32(Data, Start + 24, uint32(Behaviour.BonusG2DIndex));
        KyaWriteF32(Data, Start + 28, Behaviour.BonusTurnValue);
        return true;
    }
    if (Behaviour.Id == 4)
    {
        if (Size != 52) return false;
        KyaWriteF32(Data, Start, Behaviour.BonusField14);
        KyaWriteF32(Data, Start + 4, Behaviour.BonusField1B8);
        KyaWriteF32(Data, Start + 8, Behaviour.BonusField1B4);
        KyaWriteU32(Data, Start + 12, uint32(Behaviour.BonusMaterialId));
        KyaWriteF32(Data, Start + 16, Behaviour.BonusField1B0);
        KyaWriteU32(Data, Start + 20, uint32(Behaviour.BonusFxTailCount));
        KyaWriteU32(Data, Start + 24, uint32(Behaviour.BonusG2DIndex));
        KyaWriteU32(Data, Start + 28, uint32(Behaviour.BonusPathIndex));
        KyaWriteF32(Data, Start + 32, Behaviour.BonusPathField1F4);
        KyaWriteF32(Data, Start + 36, Behaviour.BonusPathField1F8);
        KyaWriteF32(Data, Start + 40, Behaviour.BonusPathField1FC);
        KyaWriteF32(Data, Start + 44, Behaviour.BonusPathField200);
        KyaWriteU32(Data, Start + 48, uint32(Behaviour.BonusPathField204));
        return true;
    }
    if (Behaviour.Id == 5)
    {
        if (Size != 4) return false;
        KyaWriteU32(Data, Start, uint32(Behaviour.BonusFlockPathIndex));
        return true;
    }
    if (Behaviour.Id == 6)
    {
        if (Size != 4) return false;
        KyaWriteF32(Data, Start, Behaviour.BonusAddOnField18);
        return true;
    }
    return false;
}
