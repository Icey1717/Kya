#include "Actors/KyaFruit.h"

#include "Actors/KyaSwitch.h"
#include "KyaBinaryHelpers.h"
#include "KyaLevelBank.h"

bool KyaReadFruitOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size,
    FKyaFruitOffsets& Out)
{
    Out = FKyaFruitOffsets{};
    if (!KyaRangeValid(Data, Start, Size) || Size < 60) return false;
    Out.WeightB = Start;
    Out.Autonomous = Start + 4;
    Out.Field360 = Start + 20;
    Out.Switch = Start + 24;

    // The modern S_NTF_SWITCH layout uses 28-byte target records. Require the
    // candidate parse to consume the exact payload; legacy 16-byte records then
    // stay raw instead of being misread.
    FKyaSwitchActorOffsets Switch;
    if (!KyaReadSwitchActorOffsets(Data, Out.Switch, Size - 24, Switch)) return false;
    if (Switch.Tail != Start + Size) return false;
    Out.Tail = Start + Size;
    return true;
}

bool KyaReadFruitActor(FKyaActorPlacement& Out, TConstArrayView<uint8> Data, int64 Start,
    int64 Size)
{
    FKyaFruitOffsets Off;
    if (!KyaReadFruitOffsets(Data, Start, Size, Off)) return false;
    const uint8* Bytes = Data.GetData();
    Out.FruitWeightB = KyaReadF32(Bytes + Off.WeightB);
    Out.FruitLifeMax = int64(KyaReadU32(Bytes + Off.Autonomous));
    Out.FruitGravityScale = KyaReadF32(Bytes + Off.Autonomous + 4);
    Out.FruitDynamicExtField4 = KyaReadF32(Bytes + Off.Autonomous + 8);
    Out.FruitDynamicExtField8 = KyaReadF32(Bytes + Off.Autonomous + 12);
    Out.FruitField360 = int32(KyaReadU32(Bytes + Off.Field360));

    FKyaSwitchActorOffsets Switch;
    if (!KyaReadSwitchActorOffsets(Data, Off.Switch, Size - 24, Switch)) return false;
    Out.FruitSwitch = FKyaSwitch{};
    if (!KyaReadSwitchValue(Data, Switch.Targets, Switch.Count, Switch.Camera,
        Out.FruitSwitch)) return false;

    Out.bHasFruitData = true;
    Out.ClassData.Reset();
    if (Start + Size > Off.Tail)
        Out.ClassData.Append(Bytes + Off.Tail, int32(Start + Size - Off.Tail));
    return true;
}

bool KyaWriteFruitActor(const FKyaActorPlacement& Actor, TArray<uint8>& Data, int64 Start,
    int64 Size)
{
    FKyaFruitOffsets Off;
    if (!KyaReadFruitOffsets(MakeArrayView(Data.GetData(), Data.Num()), Start, Size, Off))
        return false;
    FKyaSwitchActorOffsets Switch;
    if (!KyaReadSwitchActorOffsets(MakeArrayView(Data.GetData(), Data.Num()), Off.Switch,
        Size - 24, Switch)) return false;
    if (Actor.FruitLifeMax < 0 || Actor.FruitLifeMax > MAX_uint32) return false;
    if (Actor.FruitSwitch.Targets.Num() != Switch.Count) return false;

    KyaWriteF32(Data, Off.WeightB, Actor.FruitWeightB);
    KyaWriteU32(Data, Off.Autonomous, uint32(uint64(Actor.FruitLifeMax)));
    KyaWriteF32(Data, Off.Autonomous + 4, Actor.FruitGravityScale);
    KyaWriteF32(Data, Off.Autonomous + 8, Actor.FruitDynamicExtField4);
    KyaWriteF32(Data, Off.Autonomous + 12, Actor.FruitDynamicExtField8);
    KyaWriteU32(Data, Off.Field360, uint32(Actor.FruitField360));
    KyaWriteSwitchValue(Data, Actor.FruitSwitch, Switch.Targets, Switch.Camera);
    return true;
}

bool KyaReadFruitInventoryOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size,
    FKyaFruitInventoryOffsets& Out)
{
    Out = FKyaFruitInventoryOffsets{};
    if (!KyaRangeValid(Data, Start, Size) || Size < 8) return false;
    const uint8* Bytes = Data.GetData();
    Out.Size = int32(Size);
    Out.bHasData = KyaReadU32(Bytes + Start) != 0;
    if (!Out.bHasData)
    {
        if (Size != 8) return false;
        Out.InitialBehaviour = Start + 4;
        Out.Tail = Start + 8;
        return true;
    }
    if (Size != 40 && Size != 44 && Size != 56) return false;
    Out.Field8 = Start + 4;
    Out.Field10 = Start + 12;
    Out.PurchaseId = Start + 20;
    Out.Field28 = Start + 24;
    Out.MoneyCost = Start + 28;
    Out.Field2C = Start + 32;
    if (Size == 40)
    {
        Out.InitialBehaviour = Start + 36;
    }
    else
    {
        Out.ObjectFlag = Start + 36;
        Out.bHasObject = KyaReadU32(Bytes + Out.ObjectFlag) != 0;
        if (Out.bHasObject)
        {
            if (Size != 56) return false;
            Out.ObjectId = Start + 40;
            Out.Field18 = Start + 44;
            Out.InitialBehaviour = Start + 52;
        }
        else
        {
            if (Size != 44) return false;
            Out.InitialBehaviour = Start + 40;
        }
    }
    Out.Tail = Start + Size;
    return true;
}

bool KyaReadFruitBehaviour(FKyaActorBehaviour& Out, int32 Id, TConstArrayView<uint8> Data,
    int64 Start, int32 Size)
{
    if (!KyaRangeValid(Data, Start, int64(Size))) return false;
    const uint8* Bytes = Data.GetData();
    if (Id == 4)
    {
        if (Size != 4) return false;
        Out.FruitSquashedValue = KyaReadF32(Bytes + Start);
    }
    else if (Id == 6)
    {
        if (Size != 8) return false;
        Out.FruitBonusActorRef = int32(KyaReadU32(Bytes + Start));
        Out.FruitBonusInitialBehaviour = int32(KyaReadU32(Bytes + Start + 4));
    }
    else if (Id == 5)
    {
        FKyaFruitInventoryOffsets Off;
        if (!KyaReadFruitInventoryOffsets(Data, Start, Size, Off)) return false;
        Out.FruitInventoryField0 = Off.bHasData ? 2 : 0;
        if (Off.bHasData)
        {
            Out.FruitInventoryField8 = int64(uint64(KyaReadU32(Bytes + Off.Field8)) |
                (uint64(KyaReadU32(Bytes + Off.Field8 + 4)) << 32));
            Out.FruitInventoryField10 = int64(uint64(KyaReadU32(Bytes + Off.Field10)) |
                (uint64(KyaReadU32(Bytes + Off.Field10 + 4)) << 32));
            Out.FruitInventoryPurchaseId = int32(KyaReadU32(Bytes + Off.PurchaseId));
            Out.FruitInventoryField28 = int32(KyaReadU32(Bytes + Off.Field28));
            Out.FruitInventoryMoneyCost = int32(KyaReadU32(Bytes + Off.MoneyCost));
            Out.FruitInventoryField2C = int32(KyaReadU32(Bytes + Off.Field2C));
        }
        Out.FruitInventoryHasObject = Off.bHasObject;
        if (Off.bHasObject)
        {
            Out.FruitInventoryObjectId = int32(KyaReadU32(Bytes + Off.ObjectId));
            Out.FruitInventoryField18 = int64(uint64(KyaReadU32(Bytes + Off.Field18)) |
                (uint64(KyaReadU32(Bytes + Off.Field18 + 4)) << 32));
        }
        Out.FruitInventoryInitialBehaviour = int32(KyaReadU32(Bytes + Off.InitialBehaviour));
    }
    else return false;
    Out.bHasTypedFruitBehaviour = true;
    return true;
}

bool KyaWriteFruitBehaviour(const FKyaActorBehaviour& Behaviour, TArray<uint8>& Data,
    int64 Start, int32 Size)
{
    if (Behaviour.Id == 4)
    {
        if (Size != 4) return false;
        KyaWriteF32(Data, Start, Behaviour.FruitSquashedValue);
        return true;
    }
    if (Behaviour.Id == 6)
    {
        if (Size != 8) return false;
        KyaWriteU32(Data, Start, uint32(Behaviour.FruitBonusActorRef));
        KyaWriteU32(Data, Start + 4, uint32(Behaviour.FruitBonusInitialBehaviour));
        return true;
    }
    if (Behaviour.Id == 5)
    {
        FKyaFruitInventoryOffsets Off;
        if (!KyaReadFruitInventoryOffsets(MakeArrayView(Data.GetData(), Data.Num()), Start,
            Size, Off)) return false;
        if ((Off.bHasData ? 2 : 0) != Behaviour.FruitInventoryField0) return false;
        if (Off.bHasData)
        {
            KyaWriteU32(Data, Off.Field8, uint32(uint64(Behaviour.FruitInventoryField8)));
            KyaWriteU32(Data, Off.Field8 + 4, uint32(uint64(Behaviour.FruitInventoryField8) >> 32));
            KyaWriteU32(Data, Off.Field10, uint32(uint64(Behaviour.FruitInventoryField10)));
            KyaWriteU32(Data, Off.Field10 + 4, uint32(uint64(Behaviour.FruitInventoryField10) >> 32));
            KyaWriteU32(Data, Off.PurchaseId, uint32(Behaviour.FruitInventoryPurchaseId));
            KyaWriteU32(Data, Off.Field28, uint32(Behaviour.FruitInventoryField28));
            KyaWriteU32(Data, Off.MoneyCost, uint32(Behaviour.FruitInventoryMoneyCost));
            KyaWriteU32(Data, Off.Field2C, uint32(Behaviour.FruitInventoryField2C));
        }
        if (Off.bHasObject)
        {
            KyaWriteU32(Data, Off.ObjectId, uint32(Behaviour.FruitInventoryObjectId));
            KyaWriteU32(Data, Off.Field18, uint32(uint64(Behaviour.FruitInventoryField18)));
            KyaWriteU32(Data, Off.Field18 + 4, uint32(uint64(Behaviour.FruitInventoryField18) >> 32));
        }
        KyaWriteU32(Data, Off.InitialBehaviour, uint32(Behaviour.FruitInventoryInitialBehaviour));
        return true;
    }
    return false;
}
