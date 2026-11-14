#include "Actors/KyaBridge.h"

#include "KyaBinaryHelpers.h"
#include "KyaLevelBank.h"

bool KyaReadBridgeOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size,
    FKyaBridgeOffsets& Out)
{
    Out = FKyaBridgeOffsets{};
    if (!KyaRangeValid(Data, Start, Size) || Size < 48) return false;
    const uint8* Bytes = Data.GetData();
    const int64 End = Start + Size;
    Out.ActorCount = Start;
    const uint32 Actors = KyaReadU32(Bytes + Start);
    Out.ActorCountValue = int32(Actors);
    Out.ActorEntries = Start + 4;
    int64 Pos = Start + 4;
    if (uint64(Actors) > uint64(End - Pos) / 4) return false;
    Pos += int64(Actors) * 4;
    if (End - Pos < 44) return false;
    Out.WaypointA = Pos;
    Out.WaypointB = Pos + 4;
    Out.Floats330 = Pos + 8;
    Out.Field344 = Pos + 28;
    Out.ConfigFlags = Pos + 32;
    Out.Field348 = Pos + 36;
    Out.StepCollisionScale = Pos + 40;
    Pos += 44;
    Out.Tail = Pos;
    return true;
}

bool KyaReadBridgeActor(FKyaActorPlacement& Out, TConstArrayView<uint8> Data, int64 Start,
    int64 Size)
{
    FKyaBridgeOffsets Off;
    if (!KyaReadBridgeOffsets(Data, Start, Size, Off)) return false;
    const uint8* Bytes = Data.GetData();

    Out.BridgeActorRefs.Reset();
    for (int32 Index = 0; Index < Off.ActorCountValue; ++Index)
        Out.BridgeActorRefs.Add(int32(KyaReadU32(Bytes + Off.ActorEntries + int64(Index) * 4)));
    Out.BridgeWaypointA = int32(KyaReadU32(Bytes + Off.WaypointA));
    Out.BridgeWaypointB = int32(KyaReadU32(Bytes + Off.WaypointB));
    Out.BridgeField330 = KyaReadF32(Bytes + Off.Floats330);
    Out.BridgeField334 = KyaReadF32(Bytes + Off.Floats330 + 4);
    Out.BridgeField33C = KyaReadF32(Bytes + Off.Floats330 + 8);
    Out.BridgeField340 = KyaReadF32(Bytes + Off.Floats330 + 12);
    Out.BridgeField338 = KyaReadF32(Bytes + Off.Floats330 + 16);
    Out.BridgeField344 = int32(KyaReadU32(Bytes + Off.Field344));
    Out.BridgeConfigFlags = int32(KyaReadU32(Bytes + Off.ConfigFlags));
    Out.BridgeField348 = int32(KyaReadU32(Bytes + Off.Field348));
    Out.BridgeStepCollisionScale = KyaReadF32(Bytes + Off.StepCollisionScale);

    Out.bHasBridgeData = true;
    Out.ClassData.Reset();
    if (Start + Size > Off.Tail)
        Out.ClassData.Append(Bytes + Off.Tail, int32(Start + Size - Off.Tail));
    return true;
}

bool KyaWriteBridgeActor(const FKyaActorPlacement& Actor, TArray<uint8>& Data, int64 Start,
    int64 Size)
{
    FKyaBridgeOffsets Off;
    if (!KyaReadBridgeOffsets(MakeArrayView(Data.GetData(), Data.Num()), Start, Size, Off))
        return false;
    if (Actor.BridgeActorRefs.Num() != Off.ActorCountValue) return false;
    for (int32 Index = 0; Index < Actor.BridgeActorRefs.Num(); ++Index)
        KyaWriteU32(Data, Off.ActorEntries + int64(Index) * 4, uint32(Actor.BridgeActorRefs[Index]));
    KyaWriteU32(Data, Off.WaypointA, uint32(Actor.BridgeWaypointA));
    KyaWriteU32(Data, Off.WaypointB, uint32(Actor.BridgeWaypointB));
    KyaWriteF32(Data, Off.Floats330, Actor.BridgeField330);
    KyaWriteF32(Data, Off.Floats330 + 4, Actor.BridgeField334);
    KyaWriteF32(Data, Off.Floats330 + 8, Actor.BridgeField33C);
    KyaWriteF32(Data, Off.Floats330 + 12, Actor.BridgeField340);
    KyaWriteF32(Data, Off.Floats330 + 16, Actor.BridgeField338);
    KyaWriteU32(Data, Off.Field344, uint32(Actor.BridgeField344));
    KyaWriteU32(Data, Off.ConfigFlags, uint32(Actor.BridgeConfigFlags));
    KyaWriteU32(Data, Off.Field348, uint32(Actor.BridgeField348));
    KyaWriteF32(Data, Off.StepCollisionScale, Actor.BridgeStepCollisionScale);
    return true;
}

bool KyaReadBridgeBasicOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size,
    FKyaBridgeBasicOffsets& Out)
{
    Out = FKyaBridgeBasicOffsets{};
    if (!KyaRangeValid(Data, Start, Size) || Size != 32) return false;
    Out.Direction = Start;
    Out.Fields40 = Start + 12;
    Out.Tail = Start + 32;
    return true;
}

bool KyaReadBridgeWindOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size,
    FKyaBridgeWindOffsets& Out)
{
    Out = FKyaBridgeWindOffsets{};
    if (!KyaRangeValid(Data, Start, Size) || Size != 20) return false;
    Out.Fields40 = Start;
    Out.Tail = Start + 20;
    return true;
}

bool KyaReadBridgeBehaviour(FKyaActorBehaviour& Out, int32 Id, TConstArrayView<uint8> Data,
    int64 Start, int32 Size)
{
    const uint8* Bytes = Data.GetData();
    if (Id == 2)
    {
        FKyaBridgeBasicOffsets Off;
        if (!KyaReadBridgeBasicOffsets(Data, Start, Size, Off)) return false;
        Out.BridgeBasicDirection = FVector(KyaReadF32(Bytes + Off.Direction),
            KyaReadF32(Bytes + Off.Direction + 4), KyaReadF32(Bytes + Off.Direction + 8));
        Out.BridgeBasicField40 = KyaReadF32(Bytes + Off.Fields40);
        Out.BridgeBasicField44 = KyaReadF32(Bytes + Off.Fields40 + 4);
        Out.BridgeBasicField48 = KyaReadF32(Bytes + Off.Fields40 + 8);
        Out.BridgeBasicField4C = KyaReadF32(Bytes + Off.Fields40 + 12);
        Out.BridgeBasicField50 = KyaReadF32(Bytes + Off.Fields40 + 16);
        Out.bHasTypedBridgeBasicBehaviour = true;
        return true;
    }
    if (Id == 3)
    {
        FKyaBridgeWindOffsets Off;
        if (!KyaReadBridgeWindOffsets(Data, Start, Size, Off)) return false;
        Out.BridgeWindValues.Reset();
        for (int32 Index = 0; Index < 5; ++Index)
            Out.BridgeWindValues.Add(KyaReadF32(Bytes + Off.Fields40 + Index * 4));
        Out.bHasTypedBridgeWindBehaviour = true;
        return true;
    }
    return false;
}

bool KyaWriteBridgeBehaviour(const FKyaActorBehaviour& Behaviour, TArray<uint8>& Data,
    int64 Start, int32 Size)
{
    if (Behaviour.Id == 2)
    {
        FKyaBridgeBasicOffsets Off;
        if (!KyaReadBridgeBasicOffsets(MakeArrayView(Data.GetData(), Data.Num()), Start, Size,
            Off)) return false;
        for (int32 Axis = 0; Axis < 3; ++Axis)
            KyaWriteF32(Data, Off.Direction + Axis * 4, float(Behaviour.BridgeBasicDirection[Axis]));
        KyaWriteF32(Data, Off.Fields40, Behaviour.BridgeBasicField40);
        KyaWriteF32(Data, Off.Fields40 + 4, Behaviour.BridgeBasicField44);
        KyaWriteF32(Data, Off.Fields40 + 8, Behaviour.BridgeBasicField48);
        KyaWriteF32(Data, Off.Fields40 + 12, Behaviour.BridgeBasicField4C);
        KyaWriteF32(Data, Off.Fields40 + 16, Behaviour.BridgeBasicField50);
        return true;
    }
    if (Behaviour.Id == 3)
    {
        FKyaBridgeWindOffsets Off;
        if (!KyaReadBridgeWindOffsets(MakeArrayView(Data.GetData(), Data.Num()), Start, Size,
            Off)) return false;
        if (Behaviour.BridgeWindValues.Num() != 5) return false;
        for (int32 Index = 0; Index < 5; ++Index)
            KyaWriteF32(Data, Off.Fields40 + Index * 4, Behaviour.BridgeWindValues[Index]);
        return true;
    }
    return false;
}
