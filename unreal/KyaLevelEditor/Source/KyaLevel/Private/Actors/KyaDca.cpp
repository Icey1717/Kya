#include "Actors/KyaDca.h"

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

bool KyaReadDcaOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size, FKyaDcaOffsets& Out)
{
    Out = FKyaDcaOffsets{};
    if (!KyaRangeValid(Data, Start, Size) || Size < 36) return false;
    const uint8* Bytes = Data.GetData();
    const int64 End = Start + Size;
    Out.Field504 = Start;
    Out.Field500 = Start + 4;
    Out.Field464 = Start + 8;
    Out.Field508 = Start + 12;
    Out.Field50C = Start + 16;
    Out.AimAdjust = Start + 20;
    int64 Pos = Start + 24;
    Out.ShotCount = Pos;
    const uint32 Shots = KyaReadU32(Bytes + Pos);
    Out.ShotCountValue = int32(Shots);
    Pos += 4;
    if (uint64(Shots) > uint64(End - Pos) / 4) return false;
    Out.ShotEntries = Pos;
    Pos += int64(Shots) * 4;
    if (End - Pos < 8) return false;
    Out.Field450 = Pos;
    Out.Field454 = Pos + 4;
    Out.Tail = Pos + 8;
    return true;
}

bool KyaReadDcaActor(FKyaActorPlacement& Out, TConstArrayView<uint8> Data, int64 Start,
    int64 Size)
{
    FKyaDcaOffsets Off;
    if (!KyaReadDcaOffsets(Data, Start, Size, Off)) return false;
    const uint8* Bytes = Data.GetData();
    Out.DcaField504 = int32(KyaReadU32(Bytes + Off.Field504));
    Out.DcaField500 = int32(KyaReadU32(Bytes + Off.Field500));
    Out.DcaField464 = KyaReadF32(Bytes + Off.Field464);
    Out.DcaField508 = int32(KyaReadU32(Bytes + Off.Field508));
    Out.DcaField50C = int32(KyaReadU32(Bytes + Off.Field50C));
    Out.DcaAimAdjustDegrees = KyaReadF32(Bytes + Off.AimAdjust);
    Out.DcaShotActorRefs.Reset();
    for (int32 Index = 0; Index < Off.ShotCountValue; ++Index)
        Out.DcaShotActorRefs.Add(int32(KyaReadU32(Bytes + Off.ShotEntries + int64(Index) * 4)));
    Out.DcaField450 = int32(KyaReadU32(Bytes + Off.Field450));
    Out.DcaField454 = int32(KyaReadU32(Bytes + Off.Field454));

    Out.bHasDcaData = true;
    Out.ClassData.Reset();
    if (Start + Size > Off.Tail)
        Out.ClassData.Append(Bytes + Off.Tail, int32(Start + Size - Off.Tail));
    return true;
}

bool KyaWriteDcaActor(const FKyaActorPlacement& Actor, TArray<uint8>& Data, int64 Start,
    int64 Size)
{
    FKyaDcaOffsets Off;
    if (!KyaReadDcaOffsets(MakeArrayView(Data.GetData(), Data.Num()), Start, Size, Off))
        return false;
    if (Actor.DcaShotActorRefs.Num() != Off.ShotCountValue) return false;
    KyaWriteU32(Data, Off.Field504, uint32(Actor.DcaField504));
    KyaWriteU32(Data, Off.Field500, uint32(Actor.DcaField500));
    KyaWriteF32(Data, Off.Field464, Actor.DcaField464);
    KyaWriteU32(Data, Off.Field508, uint32(Actor.DcaField508));
    KyaWriteU32(Data, Off.Field50C, uint32(Actor.DcaField50C));
    KyaWriteF32(Data, Off.AimAdjust, Actor.DcaAimAdjustDegrees);
    for (int32 Index = 0; Index < Actor.DcaShotActorRefs.Num(); ++Index)
        KyaWriteU32(Data, Off.ShotEntries + int64(Index) * 4, uint32(Actor.DcaShotActorRefs[Index]));
    KyaWriteU32(Data, Off.Field450, uint32(Actor.DcaField450));
    KyaWriteU32(Data, Off.Field454, uint32(Actor.DcaField454));
    return true;
}

bool KyaReadDcaBehaviourOffsets(int32 Id, TConstArrayView<uint8> Data, int64 Start,
    int32 Size, FKyaDcaBehaviourOffsets& Out)
{
    Out = FKyaDcaBehaviourOffsets{};
    if (!KyaRangeValid(Data, Start, int64(Size))) return false;
    if (Id == 3)
    {
        if (Size != 40) return false;
        Out.bControlled = true;
        Out.Controlled = Start;
        Out.Tail = Start + 40;
        return true;
    }
    if (Id == 5)
    {
        if (Size != 12) return false;
        Out.bTrajectory = true;
        Out.Trajectory = Start;
        Out.Tail = Start + 12;
        return true;
    }
    return false;
}

bool KyaReadDcaBehaviour(FKyaActorBehaviour& Out, int32 Id, TConstArrayView<uint8> Data,
    int64 Start, int32 Size)
{
    FKyaDcaBehaviourOffsets Off;
    if (!KyaReadDcaBehaviourOffsets(Id, Data, Start, Size, Off)) return false;
    const uint8* Bytes = Data.GetData();
    if (Off.bControlled)
    {
        Out.DcaControlledField0 = KyaReadF32(Bytes + Off.Controlled);
        Out.DcaControlledField4 = KyaReadF32(Bytes + Off.Controlled + 4);
        Out.DcaControlledField8 = KyaReadF32(Bytes + Off.Controlled + 8);
        Out.DcaControlledFovDegrees = KyaReadF32(Bytes + Off.Controlled + 12);
        Out.DcaControlledVector10 = FVector(KyaReadF32(Bytes + Off.Controlled + 16),
            KyaReadF32(Bytes + Off.Controlled + 20), KyaReadF32(Bytes + Off.Controlled + 24));
        Out.DcaControlledVector20 = FVector(KyaReadF32(Bytes + Off.Controlled + 28),
            KyaReadF32(Bytes + Off.Controlled + 32), KyaReadF32(Bytes + Off.Controlled + 36));
        Out.bHasTypedDcaControlledBehaviour = true;
    }
    if (Off.bTrajectory)
    {
        Out.DcaTrajectoryActorRef = int32(KyaReadU32(Bytes + Off.Trajectory));
        Out.DcaTrajectoryField18 = KyaReadF32(Bytes + Off.Trajectory + 4);
        Out.DcaTrajectoryField8 = int32(KyaReadU32(Bytes + Off.Trajectory + 8));
        Out.bHasTypedDcaTrajectoryBehaviour = true;
    }
    return true;
}

bool KyaWriteDcaBehaviour(const FKyaActorBehaviour& Behaviour, TArray<uint8>& Data,
    int64 Start, int32 Size)
{
    FKyaDcaBehaviourOffsets Off;
    if (!KyaReadDcaBehaviourOffsets(Behaviour.Id, MakeArrayView(Data.GetData(), Data.Num()),
        Start, Size, Off)) return false;
    if (Off.bControlled)
    {
        KyaWriteF32(Data, Off.Controlled, Behaviour.DcaControlledField0);
        KyaWriteF32(Data, Off.Controlled + 4, Behaviour.DcaControlledField4);
        KyaWriteF32(Data, Off.Controlled + 8, Behaviour.DcaControlledField8);
        KyaWriteF32(Data, Off.Controlled + 12, Behaviour.DcaControlledFovDegrees);
        KyaWriteF32(Data, Off.Controlled + 16, float(Behaviour.DcaControlledVector10.X));
        KyaWriteF32(Data, Off.Controlled + 20, float(Behaviour.DcaControlledVector10.Y));
        KyaWriteF32(Data, Off.Controlled + 24, float(Behaviour.DcaControlledVector10.Z));
        KyaWriteF32(Data, Off.Controlled + 28, float(Behaviour.DcaControlledVector20.X));
        KyaWriteF32(Data, Off.Controlled + 32, float(Behaviour.DcaControlledVector20.Y));
        KyaWriteF32(Data, Off.Controlled + 36, float(Behaviour.DcaControlledVector20.Z));
    }
    if (Off.bTrajectory)
    {
        KyaWriteU32(Data, Off.Trajectory, uint32(Behaviour.DcaTrajectoryActorRef));
        KyaWriteF32(Data, Off.Trajectory + 4, Behaviour.DcaTrajectoryField18);
        KyaWriteU32(Data, Off.Trajectory + 8, uint32(Behaviour.DcaTrajectoryField8));
    }
    return true;
}
