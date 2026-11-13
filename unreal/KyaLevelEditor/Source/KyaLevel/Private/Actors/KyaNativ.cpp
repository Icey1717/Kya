#include "Actors/KyaNativ.h"
#include "Actors/KyaNativAkasa.h"

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

bool KyaReadNativOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size,
    FKyaNativOffsets& Out)
{
    Out = FKyaNativOffsets{};
    if (!KyaRangeValid(Data, Start, Size) || Size < 64) return false;
    Out.WeightB = Start;
    Out.Autonomous = Start + 4;
    Out.Field350 = Start + 20;
    Out.Field354 = Start + 24;
    Out.Field358 = Start + 28;
    Out.MoveSpeedNoHeld = Start + 32;
    Out.MoveSpeedHeld = Start + 36;
    Out.Field370 = Start + 40;
    Out.Field368 = Start + 44;
    Out.Field364 = Start + 48;
    Out.Field378 = Start + 52;
    Out.Field37C = Start + 56;
    Out.Field380 = Start + 60;
    Out.Tail = Start + 64;
    return true;
}

bool KyaReadNativActor(FKyaActorPlacement& Out, TConstArrayView<uint8> Data, int64 Start,
    int64 Size)
{
    FKyaNativOffsets Off;
    if (!KyaReadNativOffsets(Data, Start, Size, Off)) return false;
    const uint8* Bytes = Data.GetData();
    Out.NativWeightB = KyaReadF32(Bytes + Off.WeightB);
    Out.NativLifeMax = int64(KyaReadU32(Bytes + Off.Autonomous));
    Out.NativGravityScale = KyaReadF32(Bytes + Off.Autonomous + 4);
    Out.NativDynamicExtField4 = KyaReadF32(Bytes + Off.Autonomous + 8);
    Out.NativDynamicExtField8 = KyaReadF32(Bytes + Off.Autonomous + 12);
    Out.NativField350 = int32(KyaReadU32(Bytes + Off.Field350));
    Out.NativField354 = int32(KyaReadU32(Bytes + Off.Field354));
    Out.NativField358 = int32(KyaReadU32(Bytes + Off.Field358));
    Out.NativMoveSpeedNoHeld = KyaReadF32(Bytes + Off.MoveSpeedNoHeld);
    Out.NativMoveSpeedHeld = KyaReadF32(Bytes + Off.MoveSpeedHeld);
    Out.NativField370 = int32(KyaReadU32(Bytes + Off.Field370));
    Out.NativField368 = int32(KyaReadU32(Bytes + Off.Field368));
    Out.NativField364 = int32(KyaReadU32(Bytes + Off.Field364));
    Out.NativField378 = int32(KyaReadU32(Bytes + Off.Field378));
    Out.NativField37C = int32(KyaReadU32(Bytes + Off.Field37C));
    Out.NativField380 = int32(KyaReadU32(Bytes + Off.Field380));

    Out.bHasNativData = true;
    Out.ClassData.Reset();
    if (Start + Size > Off.Tail)
        Out.ClassData.Append(Bytes + Off.Tail, int32(Start + Size - Off.Tail));
    return true;
}

bool KyaWriteNativActor(const FKyaActorPlacement& Actor, TArray<uint8>& Data, int64 Start,
    int64 Size)
{
    FKyaNativOffsets Off;
    if (!KyaReadNativOffsets(MakeArrayView(Data.GetData(), Data.Num()), Start, Size, Off))
        return false;
    if (Actor.NativLifeMax < 0 || Actor.NativLifeMax > MAX_uint32) return false;
    KyaWriteF32(Data, Off.WeightB, Actor.NativWeightB);
    KyaWriteU32(Data, Off.Autonomous, uint32(uint64(Actor.NativLifeMax)));
    KyaWriteF32(Data, Off.Autonomous + 4, Actor.NativGravityScale);
    KyaWriteF32(Data, Off.Autonomous + 8, Actor.NativDynamicExtField4);
    KyaWriteF32(Data, Off.Autonomous + 12, Actor.NativDynamicExtField8);
    KyaWriteU32(Data, Off.Field350, uint32(Actor.NativField350));
    KyaWriteU32(Data, Off.Field354, uint32(Actor.NativField354));
    KyaWriteU32(Data, Off.Field358, uint32(Actor.NativField358));
    KyaWriteF32(Data, Off.MoveSpeedNoHeld, Actor.NativMoveSpeedNoHeld);
    KyaWriteF32(Data, Off.MoveSpeedHeld, Actor.NativMoveSpeedHeld);
    KyaWriteU32(Data, Off.Field370, uint32(Actor.NativField370));
    KyaWriteU32(Data, Off.Field368, uint32(Actor.NativField368));
    KyaWriteU32(Data, Off.Field364, uint32(Actor.NativField364));
    KyaWriteU32(Data, Off.Field378, uint32(Actor.NativField378));
    KyaWriteU32(Data, Off.Field37C, uint32(Actor.NativField37C));
    KyaWriteU32(Data, Off.Field380, uint32(Actor.NativField380));
    return true;
}

bool KyaReadNativTakePutOffsets(TConstArrayView<uint8> Data, int64 Start, int64 End,
    FKyaNativTakePutOffsets& Out)
{
    Out = FKyaNativTakePutOffsets{};
    if (!KyaRangeValid(Data, Start, End - Start) || End - Start < 4) return false;
    const uint8* Bytes = Data.GetData();
    Out.Count = Start;
    const uint32 Count = KyaReadU32(Bytes + Start);
    Out.CountValue = int32(Count);
    int64 Pos = Start + 4;
    if (Count > uint64(End - Pos) / 16) return false;
    for (uint32 Index = 0; Index < Count; ++Index)
    {
        Out.Items.Add(Pos);
        Pos += 16;
    }
    if (Pos != End) return false;
    Out.Tail = Pos;
    return true;
}

namespace
{
bool ReadSellerOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size, TArray<int64>& Items)
{
    Items.Reset();
    if (!KyaRangeValid(Data, Start, Size) || Size < 8) return false;
    const int64 End = Start + Size;
    const uint32 Count = KyaReadU32(Data.GetData() + Start + 4);
    if (Count > MAX_int32 || uint64(Count) > uint64(Size - 8) / 8) return false;
    int64 Pos = Start + 8;
    for (uint32 I = 0; I < Count; ++I)
    {
        if (End - Pos < 8) return false;
        const uint32 Cinematics = KyaReadU32(Data.GetData() + Pos + 4);
        if (Cinematics > MAX_int32 || uint64(Cinematics) > uint64(End - Pos - 8) / 4) return false;
        Items.Add(Pos);
        Pos += 8 + int64(Cinematics) * 4;
    }
    return Pos == End;
}
}

bool KyaReadNativBehaviour(FKyaActorBehaviour& Out, int32 Id, TConstArrayView<uint8> Data,
    int64 Start, int32 Size)
{
    if (!KyaRangeValid(Data, Start, int64(Size))) return false;
    if (Id == 8) return KyaReadNativAkasaBehaviour(Out, Data, Start, Size);
    if (Id == 7)
    {
        TArray<int64> Items;
        if (!ReadSellerOffsets(Data, Start, Size, Items)) return false;
        Out.NativSellerField8 = KyaReadF32(Data.GetData() + Start);
        Out.NativSellerSubObjs.Reset();
        for (int64 Pos : Items)
        {
            FKyaAddOnSubObj& Sub = Out.NativSellerSubObjs.AddDefaulted_GetRef();
            Sub.Field0 = int32(KyaReadU32(Data.GetData() + Pos));
            const int32 Count = int32(KyaReadU32(Data.GetData() + Pos + 4));
            for (int32 I = 0; I < Count; ++I)
                Sub.CinematicIds.Add(int32(KyaReadU32(Data.GetData() + Pos + 8 + int64(I) * 4)));
        }
        Out.bHasTypedNativBehaviour = true;
        return true;
    }
    if (Id == 3)
    {
        if (Size < 4) return false;
        const uint32 Count = KyaReadU32(Data.GetData() + Start);
        if (Count > MAX_int32 || int64(Count) * 8 != int64(Size) - 4) return false;
        Out.NativSpeakValues.Reset();
        for (uint32 I = 0; I < Count; ++I)
        {
            const uint8* P = Data.GetData() + Start + 4 + int64(I) * 8;
            Out.NativSpeakValues.Add(FVector2D(KyaReadF32(P), KyaReadF32(P + 4)));
        }
        Out.bHasTypedNativBehaviour = true;
        return true;
    }
    if (Id == 5)
    {
        if (Size != 8) return false;
        Out.NativLiveField8 = KyaReadF32(Data.GetData() + Start);
        Out.NativLiveFieldC = KyaReadF32(Data.GetData() + Start + 4);
        Out.bHasTypedNativBehaviour = true;
        return true;
    }
    if (Id != 6) return false;
    FKyaNativTakePutOffsets Off;
    if (!KyaReadNativTakePutOffsets(Data, Start, Start + Size, Off)) return false;
    const uint8* Bytes = Data.GetData();
    Out.NativTrajectories.Reset();
    for (int64 ItemOffset : Off.Items)
    {
        FKyaNativTrajectory& Item = Out.NativTrajectories.AddDefaulted_GetRef();
        Item.Field0 = KyaReadF32(Bytes + ItemOffset);
        Item.PathFollowIndex = int32(KyaReadU32(Bytes + ItemOffset + 4));
        Item.Field14 = int32(KyaReadU32(Bytes + ItemOffset + 8));
        Item.Field18 = KyaReadF32(Bytes + ItemOffset + 12);
    }
    Out.bHasTypedNativBehaviour = true;
    return true;
}

bool KyaWriteNativBehaviour(const FKyaActorBehaviour& Behaviour, TArray<uint8>& Data,
    int64 Start, int32 Size)
{
    if (!KyaRangeValid(MakeArrayView(Data.GetData(), Data.Num()), Start, int64(Size))) return false;
    if (Behaviour.Id == 8) return KyaWriteNativAkasaBehaviour(Behaviour, Data, Start, Size);
    if (Behaviour.Id == 7)
    {
        TArray<int64> Items;
        if (!ReadSellerOffsets(MakeArrayView(Data.GetData(), Data.Num()), Start, Size, Items) ||
            Items.Num() != Behaviour.NativSellerSubObjs.Num()) return false;
        for (int32 I = 0; I < Items.Num(); ++I)
            if (KyaReadU32(Data.GetData() + Items[I] + 4) != uint32(Behaviour.NativSellerSubObjs[I].CinematicIds.Num())) return false;
        KyaWriteF32(Data, Start, Behaviour.NativSellerField8);
        for (int32 I = 0; I < Items.Num(); ++I)
        {
            const FKyaAddOnSubObj& Sub = Behaviour.NativSellerSubObjs[I];
            KyaWriteU32(Data, Items[I], uint32(Sub.Field0));
            for (int32 J = 0; J < Sub.CinematicIds.Num(); ++J)
                KyaWriteU32(Data, Items[I] + 8 + int64(J) * 4, uint32(Sub.CinematicIds[J]));
        }
        return true;
    }
    if (Behaviour.Id == 3)
    {
        if (Size < 4 || int64(Behaviour.NativSpeakValues.Num()) * 8 != int64(Size) - 4 ||
            KyaReadU32(Data.GetData() + Start) != uint32(Behaviour.NativSpeakValues.Num())) return false;
        for (int32 I = 0; I < Behaviour.NativSpeakValues.Num(); ++I)
        {
            const int64 Pos = Start + 4 + int64(I) * 8;
            KyaWriteF32(Data, Pos, float(Behaviour.NativSpeakValues[I].X));
            KyaWriteF32(Data, Pos + 4, float(Behaviour.NativSpeakValues[I].Y));
        }
        return true;
    }
    if (Behaviour.Id == 5)
    {
        if (Size != 8) return false;
        KyaWriteF32(Data, Start, Behaviour.NativLiveField8);
        KyaWriteF32(Data, Start + 4, Behaviour.NativLiveFieldC);
        return true;
    }
    if (Behaviour.Id != 6) return false;
    FKyaNativTakePutOffsets Off;
    if (!KyaReadNativTakePutOffsets(MakeArrayView(Data.GetData(), Data.Num()), Start,
        Start + Size, Off)) return false;
    if (Behaviour.NativTrajectories.Num() != Off.CountValue) return false;
    for (int32 Index = 0; Index < Behaviour.NativTrajectories.Num(); ++Index)
    {
        const FKyaNativTrajectory& Item = Behaviour.NativTrajectories[Index];
        const int64 ItemOffset = Off.Items[Index];
        KyaWriteF32(Data, ItemOffset, Item.Field0);
        KyaWriteU32(Data, ItemOffset + 4, uint32(Item.PathFollowIndex));
        KyaWriteU32(Data, ItemOffset + 8, uint32(Item.Field14));
        KyaWriteF32(Data, ItemOffset + 12, Item.Field18);
    }
    return true;
}
