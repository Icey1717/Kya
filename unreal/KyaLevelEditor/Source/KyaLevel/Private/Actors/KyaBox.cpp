#include "Actors/KyaBox.h"

#include "KyaBinaryHelpers.h"
#include "KyaLevelBank.h"

bool KyaReadBoxOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size, FKyaBoxOffsets& Out)
{
    Out = FKyaBoxOffsets{};
    if (!KyaRangeValid(Data, Start, Size) || Size < 100) return false;
    const uint8* Bytes = Data.GetData();
    const int64 End = Start + Size;
    int64 Pos = Start;
    Out.WeightB = Pos;
    Out.Autonomous = Pos + 4;
    Pos += 20;
    Out.Field350 = Pos;
    Pos += 4;
    Out.ActorCount = Pos;
    const uint32 Actors = KyaReadU32(Bytes + Pos);
    Out.ActorCountValue = int32(Actors);
    Pos += 4;
    if (uint64(Actors) > uint64(End - Pos) / 4) return false;
    Out.ActorEntries = Pos;
    Pos += int64(Actors) * 4;
    if (End - Pos < 36) return false;
    Out.Floats354 = Pos;
    Out.Field364 = Pos + 16;
    Out.Field370 = Pos + 20;
    Pos += 36;
    if (End - Pos < 4) return false;
    const uint32 SwitchTargets = KyaReadU32(Bytes + Pos);
    Out.Switch.Count = int32(SwitchTargets);
    Out.Switch.Targets = Pos + 4;
    if (SwitchTargets > uint64(End - Pos - 4) / 28) return false;
    Out.Switch.Camera = Out.Switch.Targets + int64(SwitchTargets) * 28;
    if (End - Out.Switch.Camera < 32) return false;
    Pos = Out.Switch.Camera + 32;
    if (End - Pos < 24) return false;
    Out.AddOn = Pos;
    Pos += 20;
    Out.SubObjMode = Pos;
    Out.SubObjModeValue = int32(KyaReadU32(Bytes + Pos));
    Pos += 4;
    if (Out.SubObjModeValue == 1)
    {
        if (End - Pos < 20) return false;
        Out.SubObjFloats = Pos;
        Pos += 20;
    }
    Out.Tail = Pos;
    return true;
}

bool KyaReadBoxActor(FKyaActorPlacement& Out, TConstArrayView<uint8> Data, int64 Start,
    int64 Size)
{
    FKyaBoxOffsets Off;
    if (!KyaReadBoxOffsets(Data, Start, Size, Off)) return false;
    const uint8* Bytes = Data.GetData();

    Out.BoxWeightB = KyaReadF32(Bytes + Off.WeightB);
    Out.BoxLifeMax = int64(KyaReadU32(Bytes + Off.Autonomous));
    Out.BoxGravityScale = KyaReadF32(Bytes + Off.Autonomous + 4);
    Out.BoxDynamicExtField4 = KyaReadF32(Bytes + Off.Autonomous + 8);
    Out.BoxDynamicExtField8 = KyaReadF32(Bytes + Off.Autonomous + 12);
    Out.BoxField350 = int32(KyaReadU32(Bytes + Off.Field350));
    Out.BoxActorRefs.Reset();
    for (int32 Index = 0; Index < Off.ActorCountValue; ++Index)
        Out.BoxActorRefs.Add(int32(KyaReadU32(Bytes + Off.ActorEntries + int64(Index) * 4)));
    Out.BoxField354 = KyaReadF32(Bytes + Off.Floats354);
    Out.BoxField358 = KyaReadF32(Bytes + Off.Floats354 + 4);
    Out.BoxField35C = KyaReadF32(Bytes + Off.Floats354 + 8);
    Out.BoxField360 = KyaReadF32(Bytes + Off.Floats354 + 12);
    Out.BoxField364 = int32(KyaReadU32(Bytes + Off.Field364));
    Out.BoxField370 = int32(KyaReadU32(Bytes + Off.Field370));
    Out.BoxField374 = int32(KyaReadU32(Bytes + Off.Field370 + 4));
    Out.BoxField378 = int32(KyaReadU32(Bytes + Off.Field370 + 8));
    Out.BoxField37C = int32(KyaReadU32(Bytes + Off.Field370 + 12));

    Out.BoxSwitch = FKyaSwitch{};
    if (!KyaReadSwitchValue(Data, Off.Switch.Targets, Off.Switch.Count, Off.Switch.Camera,
        Out.BoxSwitch)) return false;

    FKyaAddOnGenerator& AddOn = Out.BoxAddOn;
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

    Out.bHasBoxData = true;
    Out.ClassData.Reset();
    if (Start + Size > Off.Tail)
        Out.ClassData.Append(Bytes + Off.Tail, int32(Start + Size - Off.Tail));
    return true;
}

bool KyaWriteBoxActor(const FKyaActorPlacement& Actor, TArray<uint8>& Data, int64 Start,
    int64 Size)
{
    FKyaBoxOffsets Off;
    if (!KyaReadBoxOffsets(MakeArrayView(Data.GetData(), Data.Num()), Start, Size, Off))
        return false;
    if (Actor.BoxLifeMax < 0 || Actor.BoxLifeMax > MAX_uint32) return false;
    if (Actor.BoxActorRefs.Num() != Off.ActorCountValue) return false;
    if (Actor.BoxSwitch.Targets.Num() != Off.Switch.Count) return false;
    if (Actor.BoxAddOn.SubObjMode != Off.SubObjModeValue) return false;

    KyaWriteF32(Data, Off.WeightB, Actor.BoxWeightB);
    KyaWriteU32(Data, Off.Autonomous, uint32(uint64(Actor.BoxLifeMax)));
    KyaWriteF32(Data, Off.Autonomous + 4, Actor.BoxGravityScale);
    KyaWriteF32(Data, Off.Autonomous + 8, Actor.BoxDynamicExtField4);
    KyaWriteF32(Data, Off.Autonomous + 12, Actor.BoxDynamicExtField8);
    KyaWriteU32(Data, Off.Field350, uint32(Actor.BoxField350));
    for (int32 Index = 0; Index < Actor.BoxActorRefs.Num(); ++Index)
        KyaWriteU32(Data, Off.ActorEntries + int64(Index) * 4, uint32(Actor.BoxActorRefs[Index]));
    KyaWriteF32(Data, Off.Floats354, Actor.BoxField354);
    KyaWriteF32(Data, Off.Floats354 + 4, Actor.BoxField358);
    KyaWriteF32(Data, Off.Floats354 + 8, Actor.BoxField35C);
    KyaWriteF32(Data, Off.Floats354 + 12, Actor.BoxField360);
    KyaWriteU32(Data, Off.Field364, uint32(Actor.BoxField364));
    KyaWriteU32(Data, Off.Field370, uint32(Actor.BoxField370));
    KyaWriteU32(Data, Off.Field370 + 4, uint32(Actor.BoxField374));
    KyaWriteU32(Data, Off.Field370 + 8, uint32(Actor.BoxField378));
    KyaWriteU32(Data, Off.Field370 + 12, uint32(Actor.BoxField37C));
    KyaWriteSwitchValue(Data, Actor.BoxSwitch, Off.Switch.Targets, Off.Switch.Camera);

    const FKyaAddOnGenerator& AddOn = Actor.BoxAddOn;
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
    return true;
}
