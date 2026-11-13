#include "Actors/KyaStiller.h"

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

bool KyaReadStillerOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size,
    FKyaStillerOffsets& Out)
{
    Out = FKyaStillerOffsets{};
    if (!KyaRangeValid(Data, Start, Size) || Size < 172) return false;
    const uint8* Bytes = Data.GetData();
    const int64 End = Start + Size;
    Out.Vision = Start;
    Out.Field1D0 = Start + 20;
    Out.Path = Start + 36;
    Out.Field1E0 = Start + 40;
    Out.LifeMax = Start + 44;
    Out.Floats1EC = Start + 48;
    Out.Field1F8 = Start + 60;
    int64 Pos = Start + 64;
    Out.ActorCount = Pos;
    const uint32 Actors = KyaReadU32(Bytes + Pos);
    Out.ActorCountValue = int32(Actors);
    Pos += 4;
    if (uint64(Actors) > uint64(End - Pos) / 4) return false;
    Out.ActorEntries = Pos;
    Pos += int64(Actors) * 4;
    if (End - Pos < 4) return false;
    Out.Field200 = Pos;
    Pos += 4;
    for (int32 Switch = 0; Switch < 2; ++Switch)
    {
        if (End - Pos < 4) return false;
        FKyaSwitchActorOffsets& S = Out.Switches[Switch];
        const uint32 Count = KyaReadU32(Bytes + Pos);
        S.Count = int32(Count);
        S.Targets = Pos + 4;
        if (Count > uint64(End - Pos - 4) / 28) return false;
        S.Camera = S.Targets + int64(Count) * 28;
        if (End - S.Camera < 32) return false;
        Pos = S.Camera + 32;
        S.Tail = Pos;
    }
    if (End - Pos < 4) return false;
    Out.Field214 = Pos;
    Pos += 4;
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

bool KyaReadStillerActor(FKyaActorPlacement& Out, TConstArrayView<uint8> Data, int64 Start,
    int64 Size)
{
    FKyaStillerOffsets Off;
    if (!KyaReadStillerOffsets(Data, Start, Size, Off)) return false;
    const uint8* Bytes = Data.GetData();

    Out.StillerVisionRange = KyaReadF32(Bytes + Off.Vision);
    Out.StillerVisionHalfAngle = KyaReadF32(Bytes + Off.Vision + 4);
    Out.StillerVisionField38 = KyaReadF32(Bytes + Off.Vision + 8);
    Out.StillerVisionField3C = KyaReadF32(Bytes + Off.Vision + 12);
    Out.StillerVisionFlags = int32(KyaReadU32(Bytes + Off.Vision + 16));
    Out.StillerField1D0 = int32(KyaReadU32(Bytes + Off.Field1D0));
    Out.StillerField1D4 = int32(KyaReadU32(Bytes + Off.Field1D0 + 4));
    Out.StillerField1D8 = int32(KyaReadU32(Bytes + Off.Field1D0 + 8));
    Out.StillerField1DC = int32(KyaReadU32(Bytes + Off.Field1D0 + 12));
    Out.StillerPathIndex = int32(KyaReadU32(Bytes + Off.Path));
    Out.StillerField1E0 = KyaReadF32(Bytes + Off.Field1E0);
    Out.StillerLifeMax = KyaReadF32(Bytes + Off.LifeMax);
    Out.StillerField1EC = KyaReadF32(Bytes + Off.Floats1EC);
    Out.StillerField1F0 = KyaReadF32(Bytes + Off.Floats1EC + 4);
    Out.StillerField1F4 = KyaReadF32(Bytes + Off.Floats1EC + 8);
    Out.StillerField1F8 = int32(KyaReadU32(Bytes + Off.Field1F8));

    Out.StillerActorRefs.Reset();
    for (int32 Index = 0; Index < Off.ActorCountValue; ++Index)
        Out.StillerActorRefs.Add(int32(KyaReadU32(Bytes + Off.ActorEntries + int64(Index) * 4)));
    Out.StillerField200 = int32(KyaReadU32(Bytes + Off.Field200));

    Out.StillerSwitch208 = FKyaSwitch{};
    Out.StillerSwitch210 = FKyaSwitch{};
    if (!KyaReadSwitchValue(Data, Off.Switches[0].Targets, Off.Switches[0].Count,
        Off.Switches[0].Camera, Out.StillerSwitch208)) return false;
    if (!KyaReadSwitchValue(Data, Off.Switches[1].Targets, Off.Switches[1].Count,
        Off.Switches[1].Camera, Out.StillerSwitch210)) return false;

    Out.StillerField214 = int32(KyaReadU32(Bytes + Off.Field214));

    FKyaAddOnGenerator& AddOn = Out.StillerAddOn;
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

    Out.bHasStillerData = true;
    Out.ClassData.Reset();
    if (Start + Size > Off.Tail)
        Out.ClassData.Append(Bytes + Off.Tail, int32(Start + Size - Off.Tail));
    return true;
}

bool KyaWriteStillerActor(const FKyaActorPlacement& Actor, TArray<uint8>& Data, int64 Start,
    int64 Size)
{
    FKyaStillerOffsets Off;
    if (!KyaReadStillerOffsets(MakeArrayView(Data.GetData(), Data.Num()), Start, Size, Off))
        return false;
    if (Actor.StillerActorRefs.Num() != Off.ActorCountValue) return false;
    if (Actor.StillerSwitch208.Targets.Num() != Off.Switches[0].Count ||
        Actor.StillerSwitch210.Targets.Num() != Off.Switches[1].Count) return false;
    if (Actor.StillerAddOn.SubObjMode != Off.SubObjModeValue) return false;

    KyaWriteF32(Data, Off.Vision, Actor.StillerVisionRange);
    KyaWriteF32(Data, Off.Vision + 4, Actor.StillerVisionHalfAngle);
    KyaWriteF32(Data, Off.Vision + 8, Actor.StillerVisionField38);
    KyaWriteF32(Data, Off.Vision + 12, Actor.StillerVisionField3C);
    KyaWriteU32(Data, Off.Vision + 16, uint32(Actor.StillerVisionFlags));
    KyaWriteU32(Data, Off.Field1D0, uint32(Actor.StillerField1D0));
    KyaWriteU32(Data, Off.Field1D0 + 4, uint32(Actor.StillerField1D4));
    KyaWriteU32(Data, Off.Field1D0 + 8, uint32(Actor.StillerField1D8));
    KyaWriteU32(Data, Off.Field1D0 + 12, uint32(Actor.StillerField1DC));
    KyaWriteU32(Data, Off.Path, uint32(Actor.StillerPathIndex));
    KyaWriteF32(Data, Off.Field1E0, Actor.StillerField1E0);
    KyaWriteF32(Data, Off.LifeMax, Actor.StillerLifeMax);
    KyaWriteF32(Data, Off.Floats1EC, Actor.StillerField1EC);
    KyaWriteF32(Data, Off.Floats1EC + 4, Actor.StillerField1F0);
    KyaWriteF32(Data, Off.Floats1EC + 8, Actor.StillerField1F4);
    KyaWriteU32(Data, Off.Field1F8, uint32(Actor.StillerField1F8));
    for (int32 Index = 0; Index < Actor.StillerActorRefs.Num(); ++Index)
        KyaWriteU32(Data, Off.ActorEntries + int64(Index) * 4, uint32(Actor.StillerActorRefs[Index]));
    KyaWriteU32(Data, Off.Field200, uint32(Actor.StillerField200));
    KyaWriteSwitchValue(Data, Actor.StillerSwitch208, Off.Switches[0].Targets, Off.Switches[0].Camera);
    KyaWriteSwitchValue(Data, Actor.StillerSwitch210, Off.Switches[1].Targets, Off.Switches[1].Camera);
    KyaWriteU32(Data, Off.Field214, uint32(Actor.StillerField214));

    const FKyaAddOnGenerator& AddOn = Actor.StillerAddOn;
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
