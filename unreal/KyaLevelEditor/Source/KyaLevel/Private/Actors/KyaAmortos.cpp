#include "Actors/KyaAmortos.h"

#include "Actors/KyaSwitch.h"
#include "KyaBinaryHelpers.h"
#include "KyaLevelBank.h"

bool KyaReadAmortosOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size,
    FKyaAmortosOffsets& Out)
{
    Out = FKyaAmortosOffsets{};
    if (!KyaRangeValid(Data, Start, Size)) return false;
    const uint8* Bytes = Data.GetData();
    const int64 End = Start + Size;
    int64 Pos = Start;
    for (int32 Stream = 0; Stream < 3; ++Stream)
    {
        if (End - Pos < 4) return false;
        FKyaAmortosStreamOffsets& S = Out.Streams[Stream];
        S.Count = Pos;
        const uint32 Count = KyaReadU32(Bytes + Pos);
        S.CountValue = int32(Count);
        S.Entries = Pos + 4;
        Pos += 4;
        if (uint64(Count) > uint64(End - Pos) / 4) return false;
        Pos += int64(Count) * 4;
    }
    if (End - Pos < 40) return false;
    Out.Field190 = Pos;
    Out.Floats160 = Pos + 4;
    Pos += 40;
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
    if (End - Pos < 4) return false;
    Out.Field1C8 = Pos;
    Pos += 4;
    for (int32 Switch = 0; Switch < 2; ++Switch)
    {
        if (End - Pos < 4) return false;
        FKyaAmortosSwitchOffsets& S = Out.Switches[Switch];
        const uint32 Count = KyaReadU32(Bytes + Pos);
        S.Count = int32(Count);
        S.Targets = Pos + 4;
        if (Count > uint64(End - Pos - 4) / 28) return false;
        S.Camera = S.Targets + int64(Count) * 28;
        if (End - S.Camera < 32) return false;
        Pos = S.Camera + 32;
    }
    Out.Tail = Pos;
    return true;
}

bool KyaReadAmortosActor(FKyaActorPlacement& Out, TConstArrayView<uint8> Data, int64 Start,
    int64 Size)
{
    FKyaAmortosOffsets Off;
    if (!KyaReadAmortosOffsets(Data, Start, Size, Off)) return false;
    const uint8* Bytes = Data.GetData();

    Out.AmortosCheckpointActors.Reset();
    Out.AmortosActors188.Reset();
    Out.AmortosActors18C.Reset();
    TArray<int32>* Streams[] = { &Out.AmortosCheckpointActors, &Out.AmortosActors188,
        &Out.AmortosActors18C };
    for (int32 Stream = 0; Stream < 3; ++Stream)
        for (int32 Index = 0; Index < Off.Streams[Stream].CountValue; ++Index)
            Streams[Stream]->Add(int32(KyaReadU32(Bytes + Off.Streams[Stream].Entries +
                int64(Index) * 4)));

    Out.AmortosActorRef = int32(KyaReadU32(Bytes + Off.Field190));
    // Create reads 164,168,160,16C,170,174,178,17C,180 in this order.
    Out.AmortosField164 = KyaReadF32(Bytes + Off.Floats160);
    Out.AmortosField168 = KyaReadF32(Bytes + Off.Floats160 + 4);
    Out.AmortosField160 = KyaReadF32(Bytes + Off.Floats160 + 8);
    Out.AmortosField16C = KyaReadF32(Bytes + Off.Floats160 + 12);
    Out.AmortosField170 = KyaReadF32(Bytes + Off.Floats160 + 16);
    Out.AmortosField174 = KyaReadF32(Bytes + Off.Floats160 + 20);
    Out.AmortosField178 = KyaReadF32(Bytes + Off.Floats160 + 24);
    Out.AmortosField17C = KyaReadF32(Bytes + Off.Floats160 + 28);
    Out.AmortosField180 = KyaReadF32(Bytes + Off.Floats160 + 32);

    FKyaAddOnGenerator& AddOn = Out.AmortosAddOn;
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

    Out.AmortosField1C8 = int32(KyaReadU32(Bytes + Off.Field1C8));
    FKyaSwitch* Switches[] = { &Out.AmortosSwitch1CC, &Out.AmortosSwitch1D4 };
    for (int32 Index = 0; Index < 2; ++Index)
    {
        *Switches[Index] = FKyaSwitch{};
        if (!KyaReadSwitchValue(Data, Off.Switches[Index].Targets, Off.Switches[Index].Count,
            Off.Switches[Index].Camera, *Switches[Index])) return false;
    }

    Out.bHasAmortosData = true;
    Out.ClassData.Reset();
    if (Start + Size > Off.Tail)
        Out.ClassData.Append(Bytes + Off.Tail, int32(Start + Size - Off.Tail));
    return true;
}

bool KyaWriteAmortosActor(const FKyaActorPlacement& Actor, TArray<uint8>& Data, int64 Start,
    int64 Size)
{
    FKyaAmortosOffsets Off;
    if (!KyaReadAmortosOffsets(MakeArrayView(Data.GetData(), Data.Num()), Start, Size, Off))
        return false;
    if (Actor.AmortosCheckpointActors.Num() != Off.Streams[0].CountValue ||
        Actor.AmortosActors188.Num() != Off.Streams[1].CountValue ||
        Actor.AmortosActors18C.Num() != Off.Streams[2].CountValue ||
        Actor.AmortosAddOn.SubObjMode != Off.SubObjModeValue) return false;

    const TArray<int32>* Streams[] = { &Actor.AmortosCheckpointActors, &Actor.AmortosActors188,
        &Actor.AmortosActors18C };
    for (int32 Stream = 0; Stream < 3; ++Stream)
        for (int32 Index = 0; Index < Streams[Stream]->Num(); ++Index)
            KyaWriteU32(Data, Off.Streams[Stream].Entries + int64(Index) * 4,
                uint32((*Streams[Stream])[Index]));

    KyaWriteU32(Data, Off.Field190, uint32(Actor.AmortosActorRef));
    KyaWriteF32(Data, Off.Floats160, Actor.AmortosField164);
    KyaWriteF32(Data, Off.Floats160 + 4, Actor.AmortosField168);
    KyaWriteF32(Data, Off.Floats160 + 8, Actor.AmortosField160);
    KyaWriteF32(Data, Off.Floats160 + 12, Actor.AmortosField16C);
    KyaWriteF32(Data, Off.Floats160 + 16, Actor.AmortosField170);
    KyaWriteF32(Data, Off.Floats160 + 20, Actor.AmortosField174);
    KyaWriteF32(Data, Off.Floats160 + 24, Actor.AmortosField178);
    KyaWriteF32(Data, Off.Floats160 + 28, Actor.AmortosField17C);
    KyaWriteF32(Data, Off.Floats160 + 32, Actor.AmortosField180);

    const FKyaAddOnGenerator& AddOn = Actor.AmortosAddOn;
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

    KyaWriteU32(Data, Off.Field1C8, uint32(Actor.AmortosField1C8));
    const FKyaSwitch* Switches[] = { &Actor.AmortosSwitch1CC, &Actor.AmortosSwitch1D4 };
    for (int32 Index = 0; Index < 2; ++Index)
    {
        if (Switches[Index]->Targets.Num() != Off.Switches[Index].Count) return false;
        KyaWriteSwitchValue(Data, *Switches[Index], Off.Switches[Index].Targets,
            Off.Switches[Index].Camera);
    }
    return true;
}
