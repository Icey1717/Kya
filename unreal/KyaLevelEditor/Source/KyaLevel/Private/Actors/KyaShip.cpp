#include "Actors/KyaShip.h"

#include "KyaBinaryHelpers.h"
#include "KyaLevelBank.h"

bool KyaReadShipOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size,
    FKyaShipOffsets& Out)
{
    Out = FKyaShipOffsets{};
    if (!KyaRangeValid(Data, Start, Size) || Size < 160) return false;
    const uint8* Bytes = Data.GetData();
    const int64 End = Start + Size;
    const auto U32 = [Bytes](int64 Offset) { return KyaReadU32(Bytes + Offset); };
    int64 Pos = Start;

    Out.CareBoyCount = Pos;
    const uint32 CareBoys = U32(Pos);
    Out.CareBoyCountValue = int32(CareBoys);
    Pos += 4;
    if (uint64(CareBoys) > uint64(End - Pos) / 4) return false;
    Out.CareBoyEntries = Pos;
    Pos += int64(CareBoys) * 4;

    if (End - Pos < 4) return false;
    Out.FxCount = Pos;
    const uint32 Fx = U32(Pos);
    Out.FxCountValue = int32(Fx);
    Pos += 4;
    if (uint64(Fx) > uint64(End - Pos) / 4) return false;
    Out.FxEntries = Pos;
    Pos += int64(Fx) * 4;

    if (End - Pos < 12) return false;
    Out.ActorRefs = Pos;
    Pos += 12;
    if (End - Pos < 36) return false;
    Out.Floats = Pos;
    Pos += 36;
    if (End - Pos < 16) return false;
    Out.Fields = Pos;
    Pos += 16;
    if (End - Pos < 16) return false;
    Out.Words18C = Pos;
    Pos += 16;
    if (End - Pos < 16) return false;
    Out.Words19C = Pos;
    Pos += 16;
    if (End - Pos < 20) return false;
    Out.Vision = Pos;
    Pos += 20;

    if (End - Pos < 4) return false;
    const uint32 Targets = U32(Pos);
    Out.SwitchCountValue = int32(Targets);
    Out.SwitchTargets = Pos + 4;
    Pos += 4;
    if (uint64(Targets) > uint64(End - Pos) / 28) return false;
    Out.SwitchCamera = Pos + int64(Targets) * 28;
    if (End - Out.SwitchCamera < 32) return false;
    Pos = Out.SwitchCamera + 32;
    Out.Tail = Pos;
    return true;
}

bool KyaReadShipActor(FKyaActorPlacement& Out, TConstArrayView<uint8> Data,
    int64 Start, int64 Size)
{
    FKyaShipOffsets Off;
    if (!KyaReadShipOffsets(Data, Start, Size, Off)) return false;
    const uint8* Bytes = Data.GetData();

    Out.ShipCareBoyActorRefs.Reset();
    for (int32 Index = 0; Index < Off.CareBoyCountValue; ++Index)
        Out.ShipCareBoyActorRefs.Add(int32(KyaReadU32(Bytes + Off.CareBoyEntries + int64(Index) * 4)));
    Out.ShipFxIntValues.Reset();
    for (int32 Index = 0; Index < Off.FxCountValue; ++Index)
        Out.ShipFxIntValues.Add(int32(KyaReadU32(Bytes + Off.FxEntries + int64(Index) * 4)));

    Out.ShipJawCareBoyRef = int32(KyaReadU32(Bytes + Off.ActorRefs));
    Out.ShipAirCannonRef = int32(KyaReadU32(Bytes + Off.ActorRefs + 4));
    Out.ShipHunterRef = int32(KyaReadU32(Bytes + Off.ActorRefs + 8));

    const int64 F = Off.Floats;
    Out.ShipField214 = KyaReadF32(Bytes + F);
    Out.ShipField210 = KyaReadF32(Bytes + F + 4);
    Out.ShipField218 = KyaReadF32(Bytes + F + 8);
    Out.ShipField21C = KyaReadF32(Bytes + F + 12);
    Out.ShipField220 = KyaReadF32(Bytes + F + 16);
    Out.ShipField224 = KyaReadF32(Bytes + F + 20);
    Out.ShipField230 = KyaReadF32(Bytes + F + 24);
    Out.ShipKimTouchRange = KyaReadF32(Bytes + F + 28);
    Out.ShipField22C = KyaReadF32(Bytes + F + 32);

    Out.ShipFxField18 = int32(KyaReadU32(Bytes + Off.Fields));
    Out.ShipField234 = int32(KyaReadU32(Bytes + Off.Fields + 4));
    Out.ShipField238 = int32(KyaReadU32(Bytes + Off.Fields + 8));
    Out.ShipField1AC = int32(KyaReadU32(Bytes + Off.Fields + 12));

    Out.ShipWords18C.Reset();
    for (int32 Index = 0; Index < 4; ++Index)
        Out.ShipWords18C.Add(int32(KyaReadU32(Bytes + Off.Words18C + int64(Index) * 4)));
    Out.ShipField19C = int32(KyaReadU32(Bytes + Off.Words19C));
    Out.ShipField1A0 = int32(KyaReadU32(Bytes + Off.Words19C + 4));
    Out.ShipField1A4 = int32(KyaReadU32(Bytes + Off.Words19C + 8));
    Out.ShipField1A8 = int32(KyaReadU32(Bytes + Off.Words19C + 12));

    Out.ShipVisionRange = KyaReadF32(Bytes + Off.Vision);
    Out.ShipVisionHalfAngle = KyaReadF32(Bytes + Off.Vision + 4);
    Out.ShipVisionField38 = KyaReadF32(Bytes + Off.Vision + 8);
    Out.ShipVisionField3C = KyaReadF32(Bytes + Off.Vision + 12);
    Out.ShipVisionFlags = int32(KyaReadU32(Bytes + Off.Vision + 16));

    Out.ShipSwitch = FKyaSwitch{};
    if (!KyaReadSwitchValue(Data, Off.SwitchTargets, Off.SwitchCountValue, Off.SwitchCamera,
        Out.ShipSwitch)) return false;

    Out.bHasShipData = true;
    Out.ClassData.Reset();
    if (Start + Size > Off.Tail)
        Out.ClassData.Append(Bytes + Off.Tail, int32(Start + Size - Off.Tail));
    return true;
}

bool KyaWriteShipActor(const FKyaActorPlacement& Actor, TArray<uint8>& Data,
    int64 Start, int64 Size)
{
    FKyaShipOffsets Off;
    if (!KyaReadShipOffsets(MakeArrayView(Data.GetData(), Data.Num()), Start, Size, Off))
        return false;
    if (Actor.ShipCareBoyActorRefs.Num() != Off.CareBoyCountValue) return false;
    if (Actor.ShipFxIntValues.Num() != Off.FxCountValue) return false;
    if (Actor.ShipWords18C.Num() != 4) return false;
    if (Actor.ShipSwitch.Targets.Num() != Off.SwitchCountValue) return false;

    for (int32 Index = 0; Index < Actor.ShipCareBoyActorRefs.Num(); ++Index)
        KyaWriteU32(Data, Off.CareBoyEntries + int64(Index) * 4, uint32(Actor.ShipCareBoyActorRefs[Index]));
    for (int32 Index = 0; Index < Actor.ShipFxIntValues.Num(); ++Index)
        KyaWriteU32(Data, Off.FxEntries + int64(Index) * 4, uint32(Actor.ShipFxIntValues[Index]));

    KyaWriteU32(Data, Off.ActorRefs, uint32(Actor.ShipJawCareBoyRef));
    KyaWriteU32(Data, Off.ActorRefs + 4, uint32(Actor.ShipAirCannonRef));
    KyaWriteU32(Data, Off.ActorRefs + 8, uint32(Actor.ShipHunterRef));

    const int64 F = Off.Floats;
    KyaWriteF32(Data, F, Actor.ShipField214);
    KyaWriteF32(Data, F + 4, Actor.ShipField210);
    KyaWriteF32(Data, F + 8, Actor.ShipField218);
    KyaWriteF32(Data, F + 12, Actor.ShipField21C);
    KyaWriteF32(Data, F + 16, Actor.ShipField220);
    KyaWriteF32(Data, F + 20, Actor.ShipField224);
    KyaWriteF32(Data, F + 24, Actor.ShipField230);
    KyaWriteF32(Data, F + 28, Actor.ShipKimTouchRange);
    KyaWriteF32(Data, F + 32, Actor.ShipField22C);

    KyaWriteU32(Data, Off.Fields, uint32(Actor.ShipFxField18));
    KyaWriteU32(Data, Off.Fields + 4, uint32(Actor.ShipField234));
    KyaWriteU32(Data, Off.Fields + 8, uint32(Actor.ShipField238));
    KyaWriteU32(Data, Off.Fields + 12, uint32(Actor.ShipField1AC));

    for (int32 Index = 0; Index < 4; ++Index)
        KyaWriteU32(Data, Off.Words18C + int64(Index) * 4, uint32(Actor.ShipWords18C[Index]));
    KyaWriteU32(Data, Off.Words19C, uint32(Actor.ShipField19C));
    KyaWriteU32(Data, Off.Words19C + 4, uint32(Actor.ShipField1A0));
    KyaWriteU32(Data, Off.Words19C + 8, uint32(Actor.ShipField1A4));
    KyaWriteU32(Data, Off.Words19C + 12, uint32(Actor.ShipField1A8));

    KyaWriteF32(Data, Off.Vision, Actor.ShipVisionRange);
    KyaWriteF32(Data, Off.Vision + 4, Actor.ShipVisionHalfAngle);
    KyaWriteF32(Data, Off.Vision + 8, Actor.ShipVisionField38);
    KyaWriteF32(Data, Off.Vision + 12, Actor.ShipVisionField3C);
    KyaWriteU32(Data, Off.Vision + 16, uint32(Actor.ShipVisionFlags));
    KyaWriteSwitchValue(Data, Actor.ShipSwitch, Off.SwitchTargets, Off.SwitchCamera);
    return true;
}
