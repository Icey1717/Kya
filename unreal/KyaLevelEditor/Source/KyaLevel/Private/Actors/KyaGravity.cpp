#include "Actors/KyaGravity.h"

#include "KyaBinaryHelpers.h"
#include "KyaLevelBank.h"

bool KyaReadGravityOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size,
    FKyaGravityOffsets& Out)
{
    Out = FKyaGravityOffsets{};
    if (!KyaRangeValid(Data, Start, Size) || Size < 20) return false;
    Out.WeightB = Start;
    Out.Autonomous = Start + 4;
    Out.Tail = Start + 20;
    return true;
}

bool KyaReadGravityActor(FKyaActorPlacement& Out, TConstArrayView<uint8> Data, int64 Start,
    int64 Size)
{
    FKyaGravityOffsets Off;
    if (!KyaReadGravityOffsets(Data, Start, Size, Off)) return false;
    const uint8* Bytes = Data.GetData();
    Out.GravityWeightB = KyaReadF32(Bytes + Off.WeightB);
    Out.GravityLifeMax = int64(KyaReadU32(Bytes + Off.Autonomous));
    Out.GravityScale = KyaReadF32(Bytes + Off.Autonomous + 4);
    Out.GravityDynamicExtField4 = KyaReadF32(Bytes + Off.Autonomous + 8);
    Out.GravityDynamicExtField8 = KyaReadF32(Bytes + Off.Autonomous + 12);

    Out.bHasGravityAwareData = true;
    Out.ClassData.Reset();
    if (Start + Size > Off.Tail)
        Out.ClassData.Append(Bytes + Off.Tail, int32(Start + Size - Off.Tail));
    return true;
}

bool KyaWriteGravityActor(const FKyaActorPlacement& Actor, TArray<uint8>& Data, int64 Start,
    int64 Size)
{
    FKyaGravityOffsets Off;
    if (!KyaReadGravityOffsets(MakeArrayView(Data.GetData(), Data.Num()), Start, Size, Off))
        return false;
    if (Actor.GravityLifeMax < 0 || Actor.GravityLifeMax > MAX_uint32) return false;
    KyaWriteF32(Data, Off.WeightB, Actor.GravityWeightB);
    KyaWriteU32(Data, Off.Autonomous, uint32(uint64(Actor.GravityLifeMax)));
    KyaWriteF32(Data, Off.Autonomous + 4, Actor.GravityScale);
    KyaWriteF32(Data, Off.Autonomous + 8, Actor.GravityDynamicExtField4);
    KyaWriteF32(Data, Off.Autonomous + 12, Actor.GravityDynamicExtField8);
    return true;
}

bool KyaReadGravityBellOffsets(TConstArrayView<uint8> Data, int64 Start, int64 End,
    FKyaGravityBellOffsets& Out)
{
    Out = FKyaGravityBellOffsets{};
    if (Start < 0 || End > Data.Num() || End - Start < 12) return false;
    const uint8* Bytes = Data.GetData();
    Out.Sound = Start;
    Out.FieldC = Start + 4;
    int64 Pos = Start + 8;
    const uint32 Count = KyaReadU32(Bytes + Pos);
    Out.SwitchCount = int32(Count);
    Out.SwitchTargets = Pos + 4;
    if (Count > uint64(End - Pos - 4) / 28) return false;
    Out.SwitchCamera = Out.SwitchTargets + int64(Count) * 28;
    if (End - Out.SwitchCamera < 32) return false;
    Out.Tail = Out.SwitchCamera + 32;
    return true;
}

bool KyaReadGravityBehaviour(FKyaActorBehaviour& Out, int32 Id, TConstArrayView<uint8> Data,
    int64 Start, int32 Size)
{
    const uint8* Bytes = Data.GetData();
    if (Start < 0 || Size < 0) return false;
    if (Id == 3)
    {
        if (Start < 0 || Size != 4 || !KyaRangeValid(Data, Start, Size)) return false;
        Out.GravityFallSoundRef = int32(KyaReadU32(Bytes + Start));
        Out.bHasTypedGravityFallBehaviour = true;
        return true;
    }
    if (Id == 4)
    {
        FKyaGravityBellOffsets Off;
        if (!KyaReadGravityBellOffsets(Data, Start, Start + Size, Off)) return false;
        Out.GravityBellSoundRef = int32(KyaReadU32(Bytes + Off.Sound));
        Out.GravityBellFieldC = KyaReadF32(Bytes + Off.FieldC);
        Out.GravityBellSwitch = FKyaSwitch{};
        if (!KyaReadSwitchValue(Data, Off.SwitchTargets, Off.SwitchCount, Off.SwitchCamera,
            Out.GravityBellSwitch)) return false;
        Out.bHasTypedGravityBellBehaviour = true;
        return true;
    }
    return false;
}

bool KyaWriteGravityBehaviour(const FKyaActorBehaviour& Behaviour, TArray<uint8>& Data,
    int64 Start, int32 Size)
{
    if (Behaviour.Id == 3)
    {
        if (Start < 0 || Size != 4 || !KyaRangeValid(Data, Start, Size)) return false;
        KyaWriteU32(Data, Start, uint32(Behaviour.GravityFallSoundRef));
        return true;
    }
    if (Behaviour.Id == 4)
    {
        FKyaGravityBellOffsets Off;
        if (!KyaReadGravityBellOffsets(MakeArrayView(Data.GetData(), Data.Num()), Start,
            Start + Size, Off)) return false;
        if (Behaviour.GravityBellSwitch.Targets.Num() != Off.SwitchCount) return false;
        KyaWriteU32(Data, Off.Sound, uint32(Behaviour.GravityBellSoundRef));
        KyaWriteF32(Data, Off.FieldC, Behaviour.GravityBellFieldC);
        KyaWriteSwitchValue(Data, Behaviour.GravityBellSwitch, Off.SwitchTargets,
            Off.SwitchCamera);
        return true;
    }
    return false;
}
