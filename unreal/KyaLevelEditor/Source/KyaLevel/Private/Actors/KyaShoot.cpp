#include "Actors/KyaShoot.h"

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

bool KyaReadShootOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size,
    FKyaShootOffsets& Out)
{
    Out = FKyaShootOffsets{};
    if (!KyaRangeValid(Data, Start, Size) || Size < 112) return false;
    const uint8* Bytes = Data.GetData();
    const int64 End = Start + Size;
    Out.WeightB = Start;
    Out.Autonomous = Start + 4;
    Out.Field350 = Start + 20;
    Out.Field354 = Start + 24;
    Out.Texture = Start + 28;
    Out.Mesh = Start + 32;
    Out.Floats3C0 = Start + 36;
    Out.Field3D0 = Start + 52;
    Out.Floats3D4 = Start + 56;
    Out.Fields3E8 = Start + 72;
    int64 Pos = Start + 88;
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

bool KyaReadShootActor(FKyaActorPlacement& Out, TConstArrayView<uint8> Data, int64 Start,
    int64 Size)
{
    FKyaShootOffsets Off;
    if (!KyaReadShootOffsets(Data, Start, Size, Off)) return false;
    const uint8* Bytes = Data.GetData();

    Out.ShootWeightB = KyaReadF32(Bytes + Off.WeightB);
    Out.ShootLifeMax = int64(KyaReadU32(Bytes + Off.Autonomous));
    Out.ShootGravityScale = KyaReadF32(Bytes + Off.Autonomous + 4);
    Out.ShootDynamicExtField4 = KyaReadF32(Bytes + Off.Autonomous + 8);
    Out.ShootDynamicExtField8 = KyaReadF32(Bytes + Off.Autonomous + 12);
    Out.ShootField350 = int32(KyaReadU32(Bytes + Off.Field350));
    Out.ShootField354 = int32(KyaReadU32(Bytes + Off.Field354));
    Out.ShootTextureIndex = int32(KyaReadU32(Bytes + Off.Texture));
    Out.ShootMeshIndex = int32(KyaReadU32(Bytes + Off.Mesh));
    Out.ShootField3C0 = KyaReadF32(Bytes + Off.Floats3C0);
    Out.ShootField3C4 = KyaReadF32(Bytes + Off.Floats3C0 + 4);
    Out.ShootField3C8 = KyaReadF32(Bytes + Off.Floats3C0 + 8);
    Out.ShootField3CC = KyaReadF32(Bytes + Off.Floats3C0 + 12);
    Out.ShootField3D0 = int32(KyaReadU32(Bytes + Off.Field3D0));
    Out.ShootField3D4 = KyaReadF32(Bytes + Off.Floats3D4);
    Out.ShootField3D8 = KyaReadF32(Bytes + Off.Floats3D4 + 4);
    Out.ShootField3DC = KyaReadF32(Bytes + Off.Floats3D4 + 8);
    Out.ShootField3E0 = KyaReadF32(Bytes + Off.Floats3D4 + 12);
    Out.ShootField3E8 = int32(KyaReadU32(Bytes + Off.Fields3E8));
    Out.ShootField3E4 = int32(KyaReadU32(Bytes + Off.Fields3E8 + 4));
    Out.ShootField3EC = int32(KyaReadU32(Bytes + Off.Fields3E8 + 8));
    Out.ShootField3F0 = int32(KyaReadU32(Bytes + Off.Fields3E8 + 12));

    FKyaAddOnGenerator& AddOn = Out.ShootAddOn;
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

    Out.bHasShootData = true;
    Out.ClassData.Reset();
    if (Start + Size > Off.Tail)
        Out.ClassData.Append(Bytes + Off.Tail, int32(Start + Size - Off.Tail));
    return true;
}

bool KyaWriteShootActor(const FKyaActorPlacement& Actor, TArray<uint8>& Data, int64 Start,
    int64 Size)
{
    FKyaShootOffsets Off;
    if (!KyaReadShootOffsets(MakeArrayView(Data.GetData(), Data.Num()), Start, Size, Off))
        return false;
    if (Actor.ShootLifeMax < 0 || Actor.ShootLifeMax > MAX_uint32) return false;
    if (Actor.ShootAddOn.SubObjMode != Off.SubObjModeValue) return false;

    KyaWriteF32(Data, Off.WeightB, Actor.ShootWeightB);
    KyaWriteU32(Data, Off.Autonomous, uint32(uint64(Actor.ShootLifeMax)));
    KyaWriteF32(Data, Off.Autonomous + 4, Actor.ShootGravityScale);
    KyaWriteF32(Data, Off.Autonomous + 8, Actor.ShootDynamicExtField4);
    KyaWriteF32(Data, Off.Autonomous + 12, Actor.ShootDynamicExtField8);
    KyaWriteU32(Data, Off.Field350, uint32(Actor.ShootField350));
    KyaWriteU32(Data, Off.Field354, uint32(Actor.ShootField354));
    KyaWriteU32(Data, Off.Texture, uint32(Actor.ShootTextureIndex));
    KyaWriteU32(Data, Off.Mesh, uint32(Actor.ShootMeshIndex));
    KyaWriteF32(Data, Off.Floats3C0, Actor.ShootField3C0);
    KyaWriteF32(Data, Off.Floats3C0 + 4, Actor.ShootField3C4);
    KyaWriteF32(Data, Off.Floats3C0 + 8, Actor.ShootField3C8);
    KyaWriteF32(Data, Off.Floats3C0 + 12, Actor.ShootField3CC);
    KyaWriteU32(Data, Off.Field3D0, uint32(Actor.ShootField3D0));
    KyaWriteF32(Data, Off.Floats3D4, Actor.ShootField3D4);
    KyaWriteF32(Data, Off.Floats3D4 + 4, Actor.ShootField3D8);
    KyaWriteF32(Data, Off.Floats3D4 + 8, Actor.ShootField3DC);
    KyaWriteF32(Data, Off.Floats3D4 + 12, Actor.ShootField3E0);
    KyaWriteU32(Data, Off.Fields3E8, uint32(Actor.ShootField3E8));
    KyaWriteU32(Data, Off.Fields3E8 + 4, uint32(Actor.ShootField3E4));
    KyaWriteU32(Data, Off.Fields3E8 + 8, uint32(Actor.ShootField3EC));
    KyaWriteU32(Data, Off.Fields3E8 + 12, uint32(Actor.ShootField3F0));

    const FKyaAddOnGenerator& AddOn = Actor.ShootAddOn;
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

bool KyaReadShootFireOffsets(TConstArrayView<uint8> Data, int64 Start, int64 End,
    FKyaShootFireOffsets& Out)
{
    Out = FKyaShootFireOffsets{};
    if (Start < 0 || End > Data.Num() || End - Start < 16) return false;
    const uint8* Bytes = Data.GetData();
    Out.Field2B0 = Start;
    Out.Field8 = Start + 4;
    Out.Field2B4 = Start + 8;
    Out.Count = Start + 12;
    const uint32 Count = KyaReadU32(Bytes + Out.Count);
    Out.CountValue = int32(Count);
    Out.Entries = Start + 16;
    if (uint64(Count) > uint64(End - Out.Entries) / 4) return false;
    Out.Tail = Out.Entries + int64(Count) * 4;
    return true;
}

bool KyaReadShootWaveOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size,
    FKyaShootWaveOffsets& Out)
{
    Out = FKyaShootWaveOffsets{};
    if (!KyaRangeValid(Data, Start, Size) || Size != 28) return false;
    Out.Fields = Start;
    Out.Tail = Start + 28;
    return true;
}

bool KyaReadShootBehaviour(FKyaActorBehaviour& Out, int32 Id, TConstArrayView<uint8> Data,
    int64 Start, int32 Size)
{
    const uint8* Bytes = Data.GetData();
    if (Id == 3)
    {
        FKyaShootFireOffsets Off;
        if (!KyaReadShootFireOffsets(Data, Start, Start + Size, Off)) return false;
        Out.ShootFireField2B0 = int32(KyaReadU32(Bytes + Off.Field2B0));
        Out.ShootFireField8 = KyaReadF32(Bytes + Off.Field8);
        Out.ShootFireField2B4 = KyaReadF32(Bytes + Off.Field2B4);
        Out.ShootFireActorRefs.Reset();
        for (int32 Index = 0; Index < Off.CountValue; ++Index)
            Out.ShootFireActorRefs.Add(int32(KyaReadU32(Bytes + Off.Entries + int64(Index) * 4)));
        Out.bHasTypedShootFireBehaviour = true;
        return true;
    }
    if (Id == 4)
    {
        FKyaShootWaveOffsets Off;
        if (!KyaReadShootWaveOffsets(Data, Start, Size, Off)) return false;
        Out.ShootWaveValues.Reset();
        for (int32 Index = 0; Index < 7; ++Index)
            Out.ShootWaveValues.Add(KyaReadF32(Bytes + Off.Fields + Index * 4));
        Out.bHasTypedShootWaveBehaviour = true;
        return true;
    }
    return false;
}

bool KyaWriteShootBehaviour(const FKyaActorBehaviour& Behaviour, TArray<uint8>& Data,
    int64 Start, int32 Size)
{
    if (Behaviour.Id == 3)
    {
        FKyaShootFireOffsets Off;
        if (!KyaReadShootFireOffsets(MakeArrayView(Data.GetData(), Data.Num()), Start,
            Start + Size, Off)) return false;
        if (Behaviour.ShootFireActorRefs.Num() != Off.CountValue) return false;
        KyaWriteU32(Data, Off.Field2B0, uint32(Behaviour.ShootFireField2B0));
        KyaWriteF32(Data, Off.Field8, Behaviour.ShootFireField8);
        KyaWriteF32(Data, Off.Field2B4, Behaviour.ShootFireField2B4);
        for (int32 Index = 0; Index < Behaviour.ShootFireActorRefs.Num(); ++Index)
            KyaWriteU32(Data, Off.Entries + int64(Index) * 4, uint32(Behaviour.ShootFireActorRefs[Index]));
        return true;
    }
    if (Behaviour.Id == 4)
    {
        FKyaShootWaveOffsets Off;
        if (!KyaReadShootWaveOffsets(MakeArrayView(Data.GetData(), Data.Num()), Start, Size,
            Off)) return false;
        if (Behaviour.ShootWaveValues.Num() != 7) return false;
        for (int32 Index = 0; Index < 7; ++Index)
            KyaWriteF32(Data, Off.Fields + Index * 4, Behaviour.ShootWaveValues[Index]);
        return true;
    }
    return false;
}
