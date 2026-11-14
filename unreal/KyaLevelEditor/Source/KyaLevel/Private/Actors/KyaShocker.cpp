#include "Actors/KyaShocker.h"

#include "KyaBinaryHelpers.h"
#include "KyaLevelBank.h"

bool KyaReadShockerOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size,
    FKyaShockerOffsets& Out)
{
    Out = FKyaShockerOffsets{};
    if (!KyaRangeValid(Data, Start, Size) || Size < 156) return false;
    const uint8* Bytes = Data.GetData();
    const int64 End = Start + Size;
    Out.WeightB = Start;
    Out.Autonomous = Start + 4;
    Out.Floats = Start + 20;
    Out.ZoneInner = Start + 56;
    Out.ZoneOuter = Start + 60;
    int64 Pos = Start + 64;
    const uint32 Count = KyaReadU32(Bytes + Pos);
    Out.SwitchCount = int32(Count);
    Out.SwitchTargets = Pos + 4;
    if (uint64(Count) > uint64(End - Pos - 4) / 28) return false;
    Out.SwitchCamera = Out.SwitchTargets + int64(Count) * 28;
    if (End - Out.SwitchCamera < 32) return false;
    Pos = Out.SwitchCamera + 32;
    if (End - Pos < 32) return false;
    Out.Field380 = Pos;
    Out.Fields = Pos + 4;
    Out.BoneId = Pos + 28;
    Pos += 32;
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

bool KyaReadShockerActor(FKyaActorPlacement& Out, TConstArrayView<uint8> Data, int64 Start,
    int64 Size)
{
    FKyaShockerOffsets Off;
    if (!KyaReadShockerOffsets(Data, Start, Size, Off)) return false;
    const uint8* Bytes = Data.GetData();
    const int64 F = Off.Floats;
    Out.ShockerField350 = KyaReadF32(Bytes + F);
    Out.ShockerField354 = KyaReadF32(Bytes + F + 4);
    Out.ShockerField358 = KyaReadF32(Bytes + F + 8);
    Out.ShockerField35C = KyaReadF32(Bytes + F + 12);
    Out.ShockerField364 = KyaReadF32(Bytes + F + 16);
    Out.ShockerField368 = KyaReadF32(Bytes + F + 20);
    Out.ShockerField360 = KyaReadF32(Bytes + F + 24);
    Out.ShockerFieldF0 = KyaReadF32(Bytes + F + 28);
    Out.ShockerField36C = KyaReadF32(Bytes + F + 32);
    Out.ShockerZoneInner = int32(KyaReadU32(Bytes + Off.ZoneInner));
    Out.ShockerZoneOuter = int32(KyaReadU32(Bytes + Off.ZoneOuter));
    Out.ShockerSwitch = FKyaSwitch{};
    if (!KyaReadSwitchValue(Data, Off.SwitchTargets, Off.SwitchCount, Off.SwitchCamera,
        Out.ShockerSwitch)) return false;
    Out.ShockerField380 = int32(KyaReadU32(Bytes + Off.Field380));
    Out.ShockerField384 = int32(KyaReadU32(Bytes + Off.Fields));
    Out.ShockerField388 = int32(KyaReadU32(Bytes + Off.Fields + 4));
    Out.ShockerField38C = int32(KyaReadU32(Bytes + Off.Fields + 8));
    Out.ShockerField390 = int32(KyaReadU32(Bytes + Off.Fields + 12));
    Out.ShockerField394 = int32(KyaReadU32(Bytes + Off.Fields + 16));
    Out.ShockerField398 = int32(KyaReadU32(Bytes + Off.Fields + 20));
    Out.ShockerBoneId = int32(KyaReadU32(Bytes + Off.BoneId));

    FKyaAddOnGenerator& AddOn = Out.ShockerAddOn;
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

    Out.bHasShockerData = true;
    Out.ClassData.Reset();
    if (Start + Size > Off.Tail)
        Out.ClassData.Append(Bytes + Off.Tail, int32(Start + Size - Off.Tail));
    return true;
}

bool KyaWriteShockerActor(const FKyaActorPlacement& Actor, TArray<uint8>& Data, int64 Start,
    int64 Size)
{
    FKyaShockerOffsets Off;
    if (!KyaReadShockerOffsets(MakeArrayView(Data.GetData(), Data.Num()), Start, Size, Off))
        return false;
    if (Actor.ShockerSwitch.Targets.Num() != Off.SwitchCount) return false;
    if (Actor.ShockerAddOn.SubObjMode != Off.SubObjModeValue) return false;

    const int64 F = Off.Floats;
    KyaWriteF32(Data, F, Actor.ShockerField350);
    KyaWriteF32(Data, F + 4, Actor.ShockerField354);
    KyaWriteF32(Data, F + 8, Actor.ShockerField358);
    KyaWriteF32(Data, F + 12, Actor.ShockerField35C);
    KyaWriteF32(Data, F + 16, Actor.ShockerField364);
    KyaWriteF32(Data, F + 20, Actor.ShockerField368);
    KyaWriteF32(Data, F + 24, Actor.ShockerField360);
    KyaWriteF32(Data, F + 28, Actor.ShockerFieldF0);
    KyaWriteF32(Data, F + 32, Actor.ShockerField36C);
    KyaWriteU32(Data, Off.ZoneInner, uint32(Actor.ShockerZoneInner));
    KyaWriteU32(Data, Off.ZoneOuter, uint32(Actor.ShockerZoneOuter));
    KyaWriteSwitchValue(Data, Actor.ShockerSwitch, Off.SwitchTargets, Off.SwitchCamera);
    KyaWriteU32(Data, Off.Field380, uint32(Actor.ShockerField380));
    KyaWriteU32(Data, Off.Fields, uint32(Actor.ShockerField384));
    KyaWriteU32(Data, Off.Fields + 4, uint32(Actor.ShockerField388));
    KyaWriteU32(Data, Off.Fields + 8, uint32(Actor.ShockerField38C));
    KyaWriteU32(Data, Off.Fields + 12, uint32(Actor.ShockerField390));
    KyaWriteU32(Data, Off.Fields + 16, uint32(Actor.ShockerField394));
    KyaWriteU32(Data, Off.Fields + 20, uint32(Actor.ShockerField398));
    KyaWriteU32(Data, Off.BoneId, uint32(Actor.ShockerBoneId));

    const FKyaAddOnGenerator& AddOn = Actor.ShockerAddOn;
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

bool KyaReadShockerBehaviour(FKyaActorBehaviour& Out, int32 Id, TConstArrayView<uint8> Data,
    int64 Start, int32 Size)
{
    if (Id != 3 || Start < 0 || Size != 32 || !KyaRangeValid(Data, Start, Size)) return false;
    const uint8* Bytes = Data.GetData();
    FKyaShockerWave& Wave = Out.ShockerWave;
    Wave.Field8 = KyaReadF32(Bytes + Start);
    Wave.FieldC = KyaReadF32(Bytes + Start + 4);
    Wave.Field10 = KyaReadF32(Bytes + Start + 8);
    Wave.Field14 = KyaReadF32(Bytes + Start + 12);
    Wave.Field18 = KyaReadF32(Bytes + Start + 16);
    Wave.Field1C = KyaReadF32(Bytes + Start + 20);
    Wave.MaterialId = int32(KyaReadU32(Bytes + Start + 24));
    Wave.Field24 = int32(KyaReadU32(Bytes + Start + 28));
    Out.bHasTypedShockerBehaviour = true;
    return true;
}

bool KyaWriteShockerBehaviour(const FKyaActorBehaviour& Behaviour, TArray<uint8>& Data,
    int64 Start, int32 Size)
{
    if (Behaviour.Id != 3 || Start < 0 || Size != 32 || !KyaRangeValid(Data, Start, Size))
        return false;
    const FKyaShockerWave& Wave = Behaviour.ShockerWave;
    KyaWriteF32(Data, Start, Wave.Field8);
    KyaWriteF32(Data, Start + 4, Wave.FieldC);
    KyaWriteF32(Data, Start + 8, Wave.Field10);
    KyaWriteF32(Data, Start + 12, Wave.Field14);
    KyaWriteF32(Data, Start + 16, Wave.Field18);
    KyaWriteF32(Data, Start + 20, Wave.Field1C);
    KyaWriteU32(Data, Start + 24, uint32(Wave.MaterialId));
    KyaWriteU32(Data, Start + 28, uint32(Wave.Field24));
    return true;
}
