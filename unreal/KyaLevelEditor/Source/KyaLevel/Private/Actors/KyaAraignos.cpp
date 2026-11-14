#include "Actors/KyaAraignos.h"

#include "KyaBinaryHelpers.h"
#include "KyaLevelBank.h"

bool KyaReadAraignosOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size,
    FKyaAraignosOffsets& Out)
{
    Out = FKyaAraignosOffsets{};
    if (!KyaRangeValid(Data, Start, Size) || Size < 56) return false;
    const uint8* Bytes = Data.GetData();
    Out.WeightB = Start;
    Out.PathFollow = Start + 4;
    Out.Floats = Start + 8;
    Out.Sounds = Start + 20;
    Out.AddOn = Start + 32;
    Out.SubObjMode = Start + 52;
    Out.SubObjModeValue = int32(KyaReadU32(Bytes + Out.SubObjMode));
    int64 Pos = Start + 56;
    if (Out.SubObjModeValue == 1)
    {
        if (Size < 76) return false;
        Out.SubObjFloats = Pos;
        Pos += 20;
    }
    Out.Tail = Pos;
    return true;
}

bool KyaReadAraignosActor(FKyaActorPlacement& Out, TConstArrayView<uint8> Data, int64 Start,
    int64 Size)
{
    FKyaAraignosOffsets Off;
    if (!KyaReadAraignosOffsets(Data, Start, Size, Off)) return false;
    const uint8* Bytes = Data.GetData();
    Out.AraignosWeightB = KyaReadF32(Bytes + Off.WeightB);
    Out.AraignosPathFollowIndex = int32(KyaReadU32(Bytes + Off.PathFollow));
    Out.AraignosField1F8 = KyaReadF32(Bytes + Off.Floats);
    Out.AraignosField1FC = KyaReadF32(Bytes + Off.Floats + 4);
    Out.AraignosDamage = KyaReadF32(Bytes + Off.Floats + 8);
    Out.AraignosSoundRef240 = int32(KyaReadU32(Bytes + Off.Sounds));
    Out.AraignosSoundRef238 = int32(KyaReadU32(Bytes + Off.Sounds + 4));
    Out.AraignosSoundRef23C = int32(KyaReadU32(Bytes + Off.Sounds + 8));

    FKyaAddOnGenerator& AddOn = Out.AraignosAddOn;
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

    Out.bHasAraignosData = true;
    Out.ClassData.Reset();
    if (Start + Size > Off.Tail)
        Out.ClassData.Append(Bytes + Off.Tail, int32(Start + Size - Off.Tail));
    return true;
}

bool KyaWriteAraignosActor(const FKyaActorPlacement& Actor, TArray<uint8>& Data, int64 Start,
    int64 Size)
{
    FKyaAraignosOffsets Off;
    if (!KyaReadAraignosOffsets(MakeArrayView(Data.GetData(), Data.Num()), Start, Size, Off))
        return false;
    if (Actor.AraignosAddOn.SubObjMode != Off.SubObjModeValue) return false;
    KyaWriteF32(Data, Off.WeightB, Actor.AraignosWeightB);
    KyaWriteU32(Data, Off.PathFollow, uint32(Actor.AraignosPathFollowIndex));
    KyaWriteF32(Data, Off.Floats, Actor.AraignosField1F8);
    KyaWriteF32(Data, Off.Floats + 4, Actor.AraignosField1FC);
    KyaWriteF32(Data, Off.Floats + 8, Actor.AraignosDamage);
    KyaWriteU32(Data, Off.Sounds, uint32(Actor.AraignosSoundRef240));
    KyaWriteU32(Data, Off.Sounds + 4, uint32(Actor.AraignosSoundRef238));
    KyaWriteU32(Data, Off.Sounds + 8, uint32(Actor.AraignosSoundRef23C));
    const FKyaAddOnGenerator& AddOn = Actor.AraignosAddOn;
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
