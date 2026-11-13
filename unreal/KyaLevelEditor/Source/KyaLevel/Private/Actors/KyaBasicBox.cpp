#include "Actors/KyaBasicBox.h"

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

bool KyaReadBasicBoxOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size,
    FKyaBasicBoxOffsets& Out)
{
    Out = FKyaBasicBoxOffsets{};
    if (!KyaRangeValid(Data, Start, Size)) return false;
    const uint8* Bytes = Data.GetData();
    const int64 End = Start + Size;
    int64 Pos = Start;
    if (End - Pos < 12) return false;
    Out.Field160 = Pos;
    Out.LifeMax = Pos + 4;
    const uint32 ActorCount = KyaReadU32(Bytes + Pos + 8);
    Out.ActorCount = int32(ActorCount);
    Pos += 12;
    if (uint64(ActorCount) > uint64(End - Pos) / 4) return false;
    Out.Actors = Pos;
    Pos += int64(ActorCount) * 4;
    if (End - Pos < 20) return false;
    Out.Vibration = Pos;
    Pos += 16;
    Out.Field180 = Pos;
    Pos += 4;
    if (End - Pos < 16) return false;
    Out.Fields = Pos;
    Pos += 16;
    if (End - Pos < 36) return false;
    const uint32 SwitchCount = KyaReadU32(Bytes + Pos);
    Out.SwitchTargetCount = int32(SwitchCount);
    Pos += 4;
    if (uint64(SwitchCount) > uint64(End - Pos - 32) / 28) return false;
    Out.SwitchTargets = Pos;
    Pos += int64(SwitchCount) * 28;
    Out.SwitchCamera = Pos;
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

bool KyaReadBasicBoxActor(FKyaActorPlacement& Out, TConstArrayView<uint8> Data, int64 Start,
    int64 Size)
{
    FKyaBasicBoxOffsets Off;
    if (!KyaReadBasicBoxOffsets(Data, Start, Size, Off)) return false;
    const uint8* Bytes = Data.GetData();

    Out.BasicBoxField160 = int32(KyaReadU32(Bytes + Off.Field160));
    Out.BasicBoxLifeMax = int64(KyaReadU32(Bytes + Off.LifeMax));
    Out.BasicBoxActorRefs.Reset();
    for (int32 Index = 0; Index < Off.ActorCount; ++Index)
        Out.BasicBoxActorRefs.Add(int32(KyaReadU32(Bytes + Off.Actors + int64(Index) * 4)));
    Out.BasicBoxVibration0 = KyaReadF32(Bytes + Off.Vibration);
    Out.BasicBoxVibration1 = KyaReadF32(Bytes + Off.Vibration + 4);
    Out.BasicBoxVibration2 = KyaReadF32(Bytes + Off.Vibration + 8);
    Out.BasicBoxVibration3 = KyaReadF32(Bytes + Off.Vibration + 12);
    Out.BasicBoxField180 = int32(KyaReadU32(Bytes + Off.Field180));
    Out.BasicBoxField18C = int32(KyaReadU32(Bytes + Off.Fields));
    Out.BasicBoxField190 = int32(KyaReadU32(Bytes + Off.Fields + 4));
    Out.BasicBoxField198 = int32(KyaReadU32(Bytes + Off.Fields + 8));
    Out.BasicBoxField194 = int32(KyaReadU32(Bytes + Off.Fields + 12));

    Out.BasicBoxSwitch = FKyaSwitch{};
    if (!KyaReadSwitchValue(Data, Off.SwitchTargets, Off.SwitchTargetCount, Off.SwitchCamera,
        Out.BasicBoxSwitch)) return false;

    FKyaAddOnGenerator& AddOn = Out.BasicBoxAddOn;
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

    Out.bHasBasicBoxData = true;
    Out.ClassData.Reset();
    if (Start + Size > Off.Tail)
        Out.ClassData.Append(Bytes + Off.Tail, int32(Start + Size - Off.Tail));
    return true;
}

bool KyaWriteBasicBoxActor(const FKyaActorPlacement& Actor, TArray<uint8>& Data, int64 Start,
    int64 Size)
{
    FKyaBasicBoxOffsets Off;
    if (!KyaReadBasicBoxOffsets(MakeArrayView(Data.GetData(), Data.Num()), Start, Size, Off))
        return false;
    if (Actor.BasicBoxLifeMax < 0 || Actor.BasicBoxLifeMax > MAX_uint32) return false;
    if (Actor.BasicBoxActorRefs.Num() != Off.ActorCount) return false;
    if (Actor.BasicBoxSwitch.Targets.Num() != Off.SwitchTargetCount) return false;
    if (Actor.BasicBoxAddOn.SubObjMode != Off.SubObjModeValue) return false;

    KyaWriteU32(Data, Off.Field160, uint32(Actor.BasicBoxField160));
    KyaWriteU32(Data, Off.LifeMax, uint32(uint64(Actor.BasicBoxLifeMax)));
    for (int32 Index = 0; Index < Actor.BasicBoxActorRefs.Num(); ++Index)
        KyaWriteU32(Data, Off.Actors + int64(Index) * 4, uint32(Actor.BasicBoxActorRefs[Index]));
    KyaWriteF32(Data, Off.Vibration, Actor.BasicBoxVibration0);
    KyaWriteF32(Data, Off.Vibration + 4, Actor.BasicBoxVibration1);
    KyaWriteF32(Data, Off.Vibration + 8, Actor.BasicBoxVibration2);
    KyaWriteF32(Data, Off.Vibration + 12, Actor.BasicBoxVibration3);
    KyaWriteU32(Data, Off.Field180, uint32(Actor.BasicBoxField180));
    KyaWriteU32(Data, Off.Fields, uint32(Actor.BasicBoxField18C));
    KyaWriteU32(Data, Off.Fields + 4, uint32(Actor.BasicBoxField190));
    KyaWriteU32(Data, Off.Fields + 8, uint32(Actor.BasicBoxField198));
    KyaWriteU32(Data, Off.Fields + 12, uint32(Actor.BasicBoxField194));
    KyaWriteSwitchValue(Data, Actor.BasicBoxSwitch, Off.SwitchTargets, Off.SwitchCamera);

    const FKyaAddOnGenerator& AddOn = Actor.BasicBoxAddOn;
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
