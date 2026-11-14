#include "Actors/KyaPattern.h"

#include "KyaBinaryHelpers.h"
#include "KyaLevelBank.h"

bool KyaReadPatternOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size,
    FKyaPatternOffsets& Out)
{
    Out = FKyaPatternOffsets{};
    if (!KyaRangeValid(Data, Start, Size) || Size < 84) return false;
    const uint8* Bytes = Data.GetData();
    const int64 End = Start + Size;
    Out.AutonomousEnd = Start + 20; // weight + autonomous block
    Out.Floats = Out.AutonomousEnd;
    Out.Fields = Out.Floats + 28;
    Out.BoneId = Out.Fields + 8;
    Out.AddOn = Out.BoneId + 4;
    Out.SubObjMode = Out.AddOn + 20;
    Out.SubObjModeValue = int32(KyaReadU32(Bytes + Out.SubObjMode));
    int64 Pos = Out.SubObjMode + 4;
    if (Out.SubObjModeValue == 1)
    {
        if (End - Pos < 20) return false;
        Out.SubObjFloats = Pos;
        Pos += 20;
    }
    Out.Tail = Pos;
    return true;
}

bool KyaReadPatternActor(FKyaActorPlacement& Out, TConstArrayView<uint8> Data,
    int64 Start, int64 Size)
{
    FKyaPatternOffsets Off;
    if (!KyaReadPatternOffsets(Data, Start, Size, Off)) return false;
    const uint8* Bytes = Data.GetData();
    const int64 F = Off.Floats;
    Out.PatternField350 = KyaReadF32(Bytes + F);
    Out.PatternField354 = KyaReadF32(Bytes + F + 4);
    Out.PatternField358 = KyaReadF32(Bytes + F + 8);
    Out.PatternField35C = KyaReadF32(Bytes + F + 12);
    Out.PatternField364 = KyaReadF32(Bytes + F + 16);
    Out.PatternField360 = KyaReadF32(Bytes + F + 20);
    Out.PatternField368 = KyaReadF32(Bytes + F + 24);
    Out.PatternField36C = int32(KyaReadU32(Bytes + Off.Fields));
    Out.PatternField370 = int32(KyaReadU32(Bytes + Off.Fields + 4));
    Out.PatternBoneId = int32(KyaReadU32(Bytes + Off.BoneId));

    FKyaAddOnGenerator& AddOn = Out.PatternAddOn;
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

    Out.bHasPatternData = true;
    Out.ClassData.Reset();
    if (Start + Size > Off.Tail)
        Out.ClassData.Append(Bytes + Off.Tail, int32(Start + Size - Off.Tail));
    return true;
}

bool KyaWritePatternActor(const FKyaActorPlacement& Actor, TArray<uint8>& Data,
    int64 Start, int64 Size)
{
    FKyaPatternOffsets Off;
    if (!KyaReadPatternOffsets(MakeArrayView(Data.GetData(), Data.Num()), Start, Size, Off))
        return false;
    if (Actor.PatternAddOn.SubObjMode != Off.SubObjModeValue) return false;

    const int64 F = Off.Floats;
    KyaWriteF32(Data, F, Actor.PatternField350);
    KyaWriteF32(Data, F + 4, Actor.PatternField354);
    KyaWriteF32(Data, F + 8, Actor.PatternField358);
    KyaWriteF32(Data, F + 12, Actor.PatternField35C);
    KyaWriteF32(Data, F + 16, Actor.PatternField364);
    KyaWriteF32(Data, F + 20, Actor.PatternField360);
    KyaWriteF32(Data, F + 24, Actor.PatternField368);
    KyaWriteU32(Data, Off.Fields, uint32(Actor.PatternField36C));
    KyaWriteU32(Data, Off.Fields + 4, uint32(Actor.PatternField370));
    KyaWriteU32(Data, Off.BoneId, uint32(Actor.PatternBoneId));

    const FKyaAddOnGenerator& AddOn = Actor.PatternAddOn;
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

bool KyaReadPatternBehaviour(FKyaActorBehaviour& Out, int32 Id, TConstArrayView<uint8> Data,
    int64 Start, int32 Size)
{
    if (Id != 3 || Start < 0 || !KyaRangeValid(Data, Start, Size)) return false;
    if (Size != 52 && Size != 56) return false;
    const uint8* Bytes = Data.GetData();
    Out.PatternField90 = KyaReadF32(Bytes + Start);
    Out.PatternField94 = KyaReadF32(Bytes + Start + 4);
    Out.PatternField98 = KyaReadF32(Bytes + Start + 8);
    const int64 P = Start + 12;
    Out.PatternPartField4 = KyaReadF32(Bytes + P);
    Out.PatternPartField0 = KyaReadF32(Bytes + P + 4);
    Out.PatternPartFieldC = KyaReadF32(Bytes + P + 8);
    Out.PatternPartField8 = KyaReadF32(Bytes + P + 12);
    Out.PatternPartField10 = KyaReadF32(Bytes + P + 16);
    Out.PatternPartField14 = KyaReadF32(Bytes + P + 20);
    Out.PatternPartMaterialId = int32(KyaReadU32(Bytes + P + 24));
    Out.PatternPartField1C = KyaReadF32(Bytes + P + 28);
    Out.PatternPartField20 = KyaReadF32(Bytes + P + 32);
    Out.bPatternPartHasField24 = Size == 56;
    if (Out.bPatternPartHasField24)
    {
        Out.PatternPartField24 = KyaReadF32(Bytes + P + 36);
        Out.PatternPartField28 = int32(KyaReadU32(Bytes + P + 40));
    }
    else
    {
        Out.PatternPartField24 = 1.0f; // runtime default when the scene is older
        Out.PatternPartField28 = int32(KyaReadU32(Bytes + P + 36));
    }
    Out.bHasTypedPatternBehaviour = true;
    return true;
}

bool KyaWritePatternBehaviour(const FKyaActorBehaviour& Behaviour, TArray<uint8>& Data,
    int64 Start, int32 Size)
{
    if (Behaviour.Id != 3 || Start < 0 || !KyaRangeValid(Data, Start, Size)) return false;
    if (Size != 52 && Size != 56) return false;
    if (Behaviour.bPatternPartHasField24 != (Size == 56)) return false;
    KyaWriteF32(Data, Start, Behaviour.PatternField90);
    KyaWriteF32(Data, Start + 4, Behaviour.PatternField94);
    KyaWriteF32(Data, Start + 8, Behaviour.PatternField98);
    const int64 P = Start + 12;
    KyaWriteF32(Data, P, Behaviour.PatternPartField4);
    KyaWriteF32(Data, P + 4, Behaviour.PatternPartField0);
    KyaWriteF32(Data, P + 8, Behaviour.PatternPartFieldC);
    KyaWriteF32(Data, P + 12, Behaviour.PatternPartField8);
    KyaWriteF32(Data, P + 16, Behaviour.PatternPartField10);
    KyaWriteF32(Data, P + 20, Behaviour.PatternPartField14);
    KyaWriteU32(Data, P + 24, uint32(Behaviour.PatternPartMaterialId));
    KyaWriteF32(Data, P + 28, Behaviour.PatternPartField1C);
    KyaWriteF32(Data, P + 32, Behaviour.PatternPartField20);
    if (Behaviour.bPatternPartHasField24)
    {
        KyaWriteF32(Data, P + 36, Behaviour.PatternPartField24);
        KyaWriteU32(Data, P + 40, uint32(Behaviour.PatternPartField28));
    }
    else
    {
        KyaWriteU32(Data, P + 36, uint32(Behaviour.PatternPartField28));
    }
    return true;
}
