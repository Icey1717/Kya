#include "Actors/KyaHedgehog.h"

#include "KyaBinaryHelpers.h"
#include "KyaLevelBank.h"

bool KyaReadHedgehogOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size,
    FKyaHedgehogOffsets& Out)
{
    Out = FKyaHedgehogOffsets{};
    if (Start < 0 || Size < 0 || Start > Data.Num() || Size > Data.Num() - Start) return false;

    // weightB, life maximum, gravity scale, two dynamic-extension floats and the
    // four walk/run scalars occupy the first 36 bytes.
    constexpr int64 FixedSize = 84;
    if (Size < FixedSize) return false;

    FKyaHedgehogOffsets Result;
    Result.WeightB = Start;
    Result.Autonomous = Start + 4;
    Result.WalkSpeed = Start + 20;
    Result.WalkAcceleration = Start + 24;
    Result.WalkRotSpeed = Start + 28;
    Result.RunSpeed = Start + 32;
    Result.Fields374 = Start + 36;
    Result.Fields378 = Start + 40;
    Result.Floats368 = Start + 44;
    Result.AddOn = Start + 56;
    Result.SubObjMode = Start + 76;
    Result.SubObjModeValue = int32(KyaReadU32(Data.GetData() + Result.SubObjMode));

    int64 Pos = Start + 80;
    if (Result.SubObjModeValue == 1)
    {
        // CAddOnGenerator_SubObj::Create reads five floats only for mode 1; the
        // sixth runtime value (field_0x8) is defaulted, not serialized.
        if (Size < 104) return false;
        Result.SubObjFloats = Pos;
        Pos += 20;
    }
    Result.Field380 = Pos;
    Result.Tail = Pos + 4;

    Out = Result;
    return true;
}

bool KyaReadHedgehogActor(FKyaActorPlacement& Out, TConstArrayView<uint8> Data, int64 Start,
    int64 Size)
{
    FKyaHedgehogOffsets Off;
    if (!KyaReadHedgehogOffsets(Data, Start, Size, Off)) return false;
    const uint8* Bytes = Data.GetData();
    Out.HedgehogWeightB = KyaReadF32(Bytes + Off.WeightB);
    Out.HedgehogLifeMax = int64(KyaReadU32(Bytes + Off.Autonomous));
    Out.HedgehogGravityScale = KyaReadF32(Bytes + Off.Autonomous + 4);
    Out.HedgehogDynamicExtField4 = KyaReadF32(Bytes + Off.Autonomous + 8);
    Out.HedgehogDynamicExtField8 = KyaReadF32(Bytes + Off.Autonomous + 12);
    Out.HedgehogWalkSpeed = KyaReadF32(Bytes + Off.WalkSpeed);
    Out.HedgehogWalkAcceleration = KyaReadF32(Bytes + Off.WalkAcceleration);
    Out.HedgehogWalkRotSpeed = KyaReadF32(Bytes + Off.WalkRotSpeed);
    Out.HedgehogRunSpeed = KyaReadF32(Bytes + Off.RunSpeed);
    Out.HedgehogField374 = int32(KyaReadU32(Bytes + Off.Fields374));
    Out.HedgehogField378 = int32(KyaReadU32(Bytes + Off.Fields378));
    Out.HedgehogField368 = KyaReadF32(Bytes + Off.Floats368);
    Out.HedgehogField36C = KyaReadF32(Bytes + Off.Floats368 + 4);
    Out.HedgehogField370 = KyaReadF32(Bytes + Off.Floats368 + 8);

    FKyaAddOnGenerator& AddOn = Out.HedgehogAddOn;
    const int64 A = Off.AddOn;
    AddOn.Field0 = int32(KyaReadU32(Bytes + A));
    AddOn.MoneyType = int32(KyaReadU32(Bytes + A + 4));
    AddOn.MoneyCount = int32(KyaReadU32(Bytes + A + 8));
    AddOn.BonusCount = int32(KyaReadU32(Bytes + A + 12));
    AddOn.FieldC = int32(KyaReadU32(Bytes + A + 16));
    AddOn.SubObjMode = Off.SubObjModeValue;
    if (Off.SubObjFloats)
    {
        AddOn.SubObjField0 = KyaReadF32(Bytes + Off.SubObjFloats);
        AddOn.SubObjField4 = KyaReadF32(Bytes + Off.SubObjFloats + 4);
        AddOn.SubObjFieldC = KyaReadF32(Bytes + Off.SubObjFloats + 8);
        AddOn.SubObjField10 = KyaReadF32(Bytes + Off.SubObjFloats + 12);
        AddOn.SubObjField14 = KyaReadF32(Bytes + Off.SubObjFloats + 16);
    }
    Out.HedgehogField380 = int32(KyaReadU32(Bytes + Off.Field380));

    Out.bHasHedgehogData = true;
    Out.ClassData.Reset();
    if (Start + Size > Off.Tail)
        Out.ClassData.Append(Bytes + Off.Tail, int32(Start + Size - Off.Tail));
    return true;
}

bool KyaReadHedgehogBehaviour(int32 BehaviourId, TConstArrayView<uint8> Data,
    int64 Payload, int32 Size, FKyaHedgehogBehaviourValues& Out)
{
    Out = FKyaHedgehogBehaviourValues{};
    if (Payload < 0 || Size < 0 || Payload > Data.Num() || Size > Data.Num() - Payload) return false;
    switch (BehaviourId)
    {
    case 3: // CBehaviourHedgehogGuardArea
    case 5: // CBehaviourHedgehogGuardAreaArmor
        if (Size != 4) return false;
        Out.bTyped = true;
        Out.PathFollowIndex = int32(KyaReadU32(Data.GetData() + Payload));
        return true;
    case 4: // CBehaviourHedgehogWatchDog
    case 6: // CBehaviourHedgehogWatchDogArmor
        // Both Create functions are empty; an absent payload is the only valid shape.
        if (Size != 0) return false;
        Out.bTyped = true;
        Out.bEmptyPayload = true;
        return true;
    default:
        return false;
    }
}

bool KyaWriteHedgehogActor(const FKyaActorPlacement& Actor, TArray<uint8>& Data, int64 Start,
    int64 Size)
{
    FKyaHedgehogOffsets Off;
    if (!KyaReadHedgehogOffsets(MakeArrayView(Data.GetData(), Data.Num()), Start, Size, Off))
        return false;
    if (Actor.HedgehogLifeMax < 0 || Actor.HedgehogLifeMax > MAX_uint32) return false;
    if (Actor.HedgehogAddOn.SubObjMode != Off.SubObjModeValue) return false;

    KyaWriteF32(Data, Off.WeightB, Actor.HedgehogWeightB);
    KyaWriteU32(Data, Off.Autonomous, uint32(uint64(Actor.HedgehogLifeMax)));
    KyaWriteF32(Data, Off.Autonomous + 4, Actor.HedgehogGravityScale);
    KyaWriteF32(Data, Off.Autonomous + 8, Actor.HedgehogDynamicExtField4);
    KyaWriteF32(Data, Off.Autonomous + 12, Actor.HedgehogDynamicExtField8);
    KyaWriteF32(Data, Off.WalkSpeed, Actor.HedgehogWalkSpeed);
    KyaWriteF32(Data, Off.WalkAcceleration, Actor.HedgehogWalkAcceleration);
    KyaWriteF32(Data, Off.WalkRotSpeed, Actor.HedgehogWalkRotSpeed);
    KyaWriteF32(Data, Off.RunSpeed, Actor.HedgehogRunSpeed);
    KyaWriteU32(Data, Off.Fields374, uint32(Actor.HedgehogField374));
    KyaWriteU32(Data, Off.Fields378, uint32(Actor.HedgehogField378));
    KyaWriteF32(Data, Off.Floats368, Actor.HedgehogField368);
    KyaWriteF32(Data, Off.Floats368 + 4, Actor.HedgehogField36C);
    KyaWriteF32(Data, Off.Floats368 + 8, Actor.HedgehogField370);

    const FKyaAddOnGenerator& AddOn = Actor.HedgehogAddOn;
    const int64 A = Off.AddOn;
    KyaWriteU32(Data, A, uint32(AddOn.Field0));
    KyaWriteU32(Data, A + 4, uint32(AddOn.MoneyType));
    KyaWriteU32(Data, A + 8, uint32(AddOn.MoneyCount));
    KyaWriteU32(Data, A + 12, uint32(AddOn.BonusCount));
    KyaWriteU32(Data, A + 16, uint32(AddOn.FieldC));
    KyaWriteU32(Data, Off.SubObjMode, uint32(AddOn.SubObjMode));
    if (Off.SubObjFloats)
    {
        KyaWriteF32(Data, Off.SubObjFloats, AddOn.SubObjField0);
        KyaWriteF32(Data, Off.SubObjFloats + 4, AddOn.SubObjField4);
        KyaWriteF32(Data, Off.SubObjFloats + 8, AddOn.SubObjFieldC);
        KyaWriteF32(Data, Off.SubObjFloats + 12, AddOn.SubObjField10);
        KyaWriteF32(Data, Off.SubObjFloats + 16, AddOn.SubObjField14);
    }
    KyaWriteU32(Data, Off.Field380, uint32(Actor.HedgehogField380));
    return true;
}

bool KyaWriteHedgehogBehaviour(const FKyaActorBehaviour& Behaviour, TArray<uint8>& Data,
    int64 Payload, int32 Size)
{
    if ((Behaviour.Id == 3 || Behaviour.Id == 5) && Size == 4)
    {
        KyaWriteU32(Data, Payload, uint32(Behaviour.HedgehogPathFollowIndex));
        return true;
    }
    if ((Behaviour.Id == 4 || Behaviour.Id == 6) && Size == 0) return true;
    return false;
}
