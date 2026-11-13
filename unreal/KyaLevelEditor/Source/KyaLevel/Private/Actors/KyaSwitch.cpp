#include "Actors/KyaSwitch.h"

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

bool KyaRangeValid(const TArray<uint8>& Data, int64 Start, int64 Size)
{
    return Start >= 0 && Size >= 0 && Start <= Data.Num() && Size <= Data.Num() - Start;
}

bool KyaReadSwitchData(TConstArrayView<uint8> Data, int64 Targets, int32 Count, int64 Camera,
    FKyaSwitch& Out)
{
    Out = FKyaSwitch{};
    if (Targets < 0 || Count < 0 || Camera < 0 || !KyaRangeValid(Data, Targets, int64(Count) * 28) ||
        !KyaRangeValid(Data, Camera, 32)) return false;
    const uint8* Bytes = Data.GetData();
    for (int32 Index = 0; Index < Count; ++Index)
    {
        FKyaSwitchTarget& Target = Out.Targets.AddDefaulted_GetRef();
        const int64 T = Targets + int64(Index) * 28;
        Target.TargetRef = int32(KyaReadU32(Bytes + T));
        Target.CutsceneId = int32(KyaReadU32(Bytes + T + 4));
        Target.Flags = int32(KyaReadU32(Bytes + T + 8));
        Target.MessageId = int32(KyaReadU32(Bytes + T + 12));
        Target.MessageFlags = int32(KyaReadU32(Bytes + T + 16));
        Target.Field14 = int32(KyaReadU32(Bytes + T + 20));
        Target.Field18 = int32(KyaReadU32(Bytes + T + 24));
    }
    Out.Camera.CameraIndex = int32(KyaReadU32(Bytes + Camera));
    Out.Camera.Flags = int32(KyaReadU32(Bytes + Camera + 4));
    Out.Camera.Field8 = KyaReadF32(Bytes + Camera + 8);
    Out.Camera.FieldC = KyaReadF32(Bytes + Camera + 12);
    Out.Camera.Field10 = KyaReadF32(Bytes + Camera + 16);
    Out.Camera.ActorRef = int32(KyaReadU32(Bytes + Camera + 20));
    Out.Camera.ZoneRef = int32(KyaReadU32(Bytes + Camera + 24));
    Out.Camera.Field1C = KyaReadF32(Bytes + Camera + 28);
    return true;
}

void KyaWriteSwitchData(TArray<uint8>& Data, const FKyaSwitch& Switch, int64 Targets, int64 Camera)
{
    for (int32 Index = 0; Index < Switch.Targets.Num(); ++Index)
    {
        const FKyaSwitchTarget& Target = Switch.Targets[Index];
        const int64 T = Targets + int64(Index) * 28;
        KyaWriteU32(Data, T, uint32(Target.TargetRef));
        KyaWriteU32(Data, T + 4, uint32(Target.CutsceneId));
        KyaWriteU32(Data, T + 8, uint32(Target.Flags));
        KyaWriteU32(Data, T + 12, uint32(Target.MessageId));
        KyaWriteU32(Data, T + 16, uint32(Target.MessageFlags));
        KyaWriteU32(Data, T + 20, uint32(Target.Field14));
        KyaWriteU32(Data, T + 24, uint32(Target.Field18));
    }
    const FKyaStreamEventCamera& C = Switch.Camera;
    KyaWriteU32(Data, Camera, uint32(C.CameraIndex));
    KyaWriteU32(Data, Camera + 4, uint32(C.Flags));
    KyaWriteF32(Data, Camera + 8, C.Field8);
    KyaWriteF32(Data, Camera + 12, C.FieldC);
    KyaWriteF32(Data, Camera + 16, C.Field10);
    KyaWriteU32(Data, Camera + 20, uint32(C.ActorRef));
    KyaWriteU32(Data, Camera + 24, uint32(C.ZoneRef));
    KyaWriteF32(Data, Camera + 28, C.Field1C);
}
}

bool KyaReadSwitchActorOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size,
    FKyaSwitchActorOffsets& Out)
{    Out = FKyaSwitchActorOffsets{};
    if (!KyaRangeValid(Data, Start, Size) || Size < 36) return false;
    const uint8* Bytes = Data.GetData();
    const uint32 Count = KyaReadU32(Bytes + Start);
    if (Count > uint64(Size - 36) / 28) return false;
    Out.Count = int32(Count);
    Out.Targets = Start + 4;
    Out.Camera = Out.Targets + int64(Count) * 28;
    Out.Tail = Out.Camera + 32;
    return true;
}

bool KyaReadSwitchValue(TConstArrayView<uint8> Data, int64 Targets, int32 Count, int64 Camera,
    FKyaSwitch& Out)
{
    return KyaReadSwitchData(Data, Targets, Count, Camera, Out);
}

void KyaWriteSwitchValue(TArray<uint8>& Data, const FKyaSwitch& Switch, int64 Targets,
    int64 Camera)
{
    KyaWriteSwitchData(Data, Switch, Targets, Camera);
}

bool KyaReadSwitchActor(FKyaActorPlacement& Out, TConstArrayView<uint8> Data, int64 Start,
    int64 Size)
{
    FKyaSwitchActorOffsets Off;
    if (!KyaReadSwitchActorOffsets(Data, Start, Size, Off)) return false;
    if (!KyaReadSwitchData(Data, Off.Targets, Off.Count, Off.Camera, Out.SwitchTarget)) return false;
    Out.bHasSwitchData = true;
    Out.ClassData.Reset();
    if (Start + Size > Off.Tail) Out.ClassData.Append(Data.GetData() + Off.Tail,
        int32(Start + Size - Off.Tail));
    return true;
}

bool KyaWriteSwitchActor(const FKyaActorPlacement& Actor, TArray<uint8>& Data, int64 Start,
    int64 Size)
{
    FKyaSwitchActorOffsets Off;
    if (!KyaReadSwitchActorOffsets(MakeArrayView(Data.GetData(), Data.Num()), Start, Size, Off))
        return false;
    if (Actor.SwitchTarget.Targets.Num() != Off.Count) return false;
    KyaWriteSwitchData(Data, Actor.SwitchTarget, Off.Targets, Off.Camera);
    return true;
}

bool KyaReadSwitchBehaviourOffsets(int32 Id, TConstArrayView<uint8> Data, int64 Start, int64 Size,
    FKyaSwitchBehaviourOffsets& Out)
{
    Out = FKyaSwitchBehaviourOffsets{};
    if (!KyaRangeValid(Data, Start, Size) || Size < 8) return false;
    const uint8* Bytes = Data.GetData();
    const auto U32 = [Bytes](int64 Offset) { return KyaReadU32(Bytes + Offset); };
    if (Id == 2)
    {
        if (Size != 8) return false;
        Out.bLever = true;
        Out.LeverRefA = Start;
        Out.LeverRefB = Start + 4;
    }
    else if (Id == 3)
    {
        // CBehaviourSwitchMagic::Create.
        if (Size != 76) return false;
        Out.bMagic = true;
        Out.MagicField08 = Start;
        Out.MagicField10 = Start + 4;
        Out.MagicFxDigits = Start + 8;
        Out.MagicField14 = Start + 12;
        Out.MagicField4C = Start + 16;
        Out.MagicField18 = Start + 20;
        Out.MagicField28 = Start + 24;
        Out.MagicField24 = Start + 28;
        Out.MagicField30 = Start + 32;
        Out.OpenTextureId = Start + 36;
        Out.OpenMeshId = Start + 40;
        Out.MagicCamera = Start + 44;
    }
    else if (Id == 4)
    {
        // CBehaviourSwitchWolfenCounter::Create.
        if (Size != 20) return false;
        Out.bWolfen = true;
        Out.WolfenRequired = Start;
        Out.WolfenTriggerRadius = Start + 4;
        Out.WolfenZoneRef = Start + 8;
        Out.WolfenActorRef = Start + 12;
        Out.WolfenMaterialId = Start + 16;
    }
    else if (Id == 5)
    {
        if (Size != 12) return false;
        Out.bMultiCondition = true;
        Out.MultiField14 = Start;
        Out.MultiField08 = Start + 4;
        Out.MultiField0C = Start + 8;
    }
    else if (Id == 6)
    {
        if (Size != 12) return false;
        Out.bTarget = true;
        Out.TargetDisplacement = Start;
        Out.TargetSpringStrength = Start + 4;
        Out.TargetDamping = Start + 8;
    }
    else if (Id == 7)
    {
        // CBehaviourSwitchSequence::Create: two words, list count, list values.
        if (Size < 12) return false;
        Out.bSequence = true;
        Out.SequenceField08 = Start;
        Out.SequenceField0C = Start + 4;
        Out.SequenceCount = Start + 8;
        const uint32 Count = U32(Out.SequenceCount);
        Out.SequenceCountValue = int32(Count);
        Out.SequenceValues = Start + 12;
        if (Count > uint64(Size - 12) / 4 || Start + 12 + int64(Count) * 4 != Start + Size)
            return false;
    }
    else if (Id == 8)
    {
        // CBehaviourSwitchTimed::Create: base delay, entry count, entries
        // (S_NTF_SWITCH + delay float), playback mode and flags.
        if (Size < 16) return false;
        Out.bTimed = true;
        Out.TimedBaseDelay = Start;
        Out.TimedEntryCount = Start + 4;
        const uint32 Count = U32(Out.TimedEntryCount);
        Out.TimedEntryCountValue = int32(Count);
        int64 Pos = Start + 8;
        for (uint32 Index = 0; Index < Count; ++Index)
        {
            FKyaSwitchTimedEntryOffsets Entry;
            const int64 End = Start + Size;
            if (End - Pos < 4) return false;
            Entry.Count = Pos;
            const uint32 Targets = U32(Pos);
            Entry.Targets = Pos + 4;
            Entry.Camera = Entry.Targets + int64(Targets) * 28;
            if (Entry.Camera + 32 > End) return false;
            Entry.Delay = Entry.Camera + 32;
            Pos = Entry.Delay + 4;
            if (Pos > End) return false;
            Out.TimedEntries.Add(Entry);
        }
        if (Start + Size - Pos != 8) return false;
        Out.TimedPlaybackMode = Pos;
        Out.TimedFlags = Pos + 4;
    }
    else return false;
    Out.Tail = Start + Size;
    return true;
}

bool KyaReadSwitchBehaviour(FKyaActorBehaviour& Out, int32 Id, TConstArrayView<uint8> Data,
    int64 Start, int32 Size)
{
    FKyaSwitchBehaviourOffsets Off;
    if (!KyaReadSwitchBehaviourOffsets(Id, Data, Start, Size, Off)) return false;
    const uint8* Bytes = Data.GetData();
    Out.SwitchBehaviour = FKyaSwitchBehaviour{};
    FKyaSwitchBehaviour& Values = Out.SwitchBehaviour;

    if (Off.bLever)
    {
        Values.LeverRefA = int32(KyaReadU32(Bytes + Off.LeverRefA));
        Values.LeverRefB = int32(KyaReadU32(Bytes + Off.LeverRefB));
    }
    if (Off.bMagic)
    {
        Values.MagicField08 = KyaReadF32(Bytes + Off.MagicField08);
        Values.MagicField10 = KyaReadF32(Bytes + Off.MagicField10);
        Values.MagicFxDigits = int32(KyaReadU32(Bytes + Off.MagicFxDigits));
        Values.MagicField14 = KyaReadF32(Bytes + Off.MagicField14);
        Values.MagicField4C = int32(KyaReadU32(Bytes + Off.MagicField4C));
        Values.MagicField18 = int32(KyaReadU32(Bytes + Off.MagicField18));
        Values.MagicField28 = int32(KyaReadU32(Bytes + Off.MagicField28));
        Values.MagicField24 = int32(KyaReadU32(Bytes + Off.MagicField24));
        Values.MagicField30 = int32(KyaReadU32(Bytes + Off.MagicField30));
        Values.OpenTextureId = int32(KyaReadU32(Bytes + Off.OpenTextureId));
        Values.OpenMeshId = int32(KyaReadU32(Bytes + Off.OpenMeshId));
        const int64 C = Off.MagicCamera;
        Values.MagicCamera.CameraIndex = int32(KyaReadU32(Bytes + C));
        Values.MagicCamera.Flags = int32(KyaReadU32(Bytes + C + 4));
        Values.MagicCamera.Field8 = KyaReadF32(Bytes + C + 8);
        Values.MagicCamera.FieldC = KyaReadF32(Bytes + C + 12);
        Values.MagicCamera.Field10 = KyaReadF32(Bytes + C + 16);
        Values.MagicCamera.ActorRef = int32(KyaReadU32(Bytes + C + 20));
        Values.MagicCamera.ZoneRef = int32(KyaReadU32(Bytes + C + 24));
        Values.MagicCamera.Field1C = KyaReadF32(Bytes + C + 28);
    }
    if (Off.bMultiCondition)
    {
        Values.MultiField14 = int32(KyaReadU32(Bytes + Off.MultiField14));
        Values.MultiField08 = int32(KyaReadU32(Bytes + Off.MultiField08));
        Values.MultiField0C = int32(KyaReadU32(Bytes + Off.MultiField0C));
    }
    if (Off.bTarget)
    {
        Values.TargetDisplacement = KyaReadF32(Bytes + Off.TargetDisplacement);
        Values.TargetSpringStrength = KyaReadF32(Bytes + Off.TargetSpringStrength);
        Values.TargetDamping = KyaReadF32(Bytes + Off.TargetDamping);
    }
    if (Off.bWolfen)
    {
        Values.WolfenRequired = int32(KyaReadU32(Bytes + Off.WolfenRequired));
        Values.WolfenTriggerRadius = KyaReadF32(Bytes + Off.WolfenTriggerRadius);
        Values.WolfenZoneRef = int32(KyaReadU32(Bytes + Off.WolfenZoneRef));
        Values.WolfenActorRef = int32(KyaReadU32(Bytes + Off.WolfenActorRef));
        Values.WolfenMaterialId = int32(KyaReadU32(Bytes + Off.WolfenMaterialId));
    }
    if (Off.bSequence)
    {
        Values.SequenceField08 = int32(KyaReadU32(Bytes + Off.SequenceField08));
        Values.SequenceField0C = int32(KyaReadU32(Bytes + Off.SequenceField0C));
        Values.SequenceValues.Reset();
        for (int32 Index = 0; Index < Off.SequenceCountValue; ++Index)
            Values.SequenceValues.Add(int32(KyaReadU32(Bytes + Off.SequenceValues + int64(Index) * 4)));
    }
    if (Off.bTimed)
    {
        Values.TimedBaseDelay = KyaReadF32(Bytes + Off.TimedBaseDelay);
        Values.TimedSwitches.Reset();
        Values.TimedDelays.Reset();
        for (const FKyaSwitchTimedEntryOffsets& Entry : Off.TimedEntries)
        {
            FKyaSwitch& Switch = Values.TimedSwitches.AddDefaulted_GetRef();
            if (!KyaReadSwitchData(Data, Entry.Targets, int32(KyaReadU32(Bytes + Entry.Count)),
                Entry.Camera, Switch)) return false;
            Values.TimedDelays.Add(KyaReadF32(Bytes + Entry.Delay));
        }
        Values.TimedPlaybackMode = int32(KyaReadU32(Bytes + Off.TimedPlaybackMode));
        Values.TimedFlags = int32(KyaReadU32(Bytes + Off.TimedFlags));
    }

    Out.bHasTypedSwitchBehaviour = true;
    return true;
}

bool KyaWriteSwitchBehaviour(const FKyaActorBehaviour& Behaviour, TArray<uint8>& Data, int64 Start,
    int32 Size)
{
    FKyaSwitchBehaviourOffsets Off;
    if (!KyaReadSwitchBehaviourOffsets(Behaviour.Id, MakeArrayView(Data.GetData(), Data.Num()),
        Start, Size, Off)) return false;
    const FKyaSwitchBehaviour& Values = Behaviour.SwitchBehaviour;

    if (Off.bLever)
    {
        KyaWriteU32(Data, Off.LeverRefA, uint32(Values.LeverRefA));
        KyaWriteU32(Data, Off.LeverRefB, uint32(Values.LeverRefB));
    }
    if (Off.bMagic)
    {
        KyaWriteF32(Data, Off.MagicField08, Values.MagicField08);
        KyaWriteF32(Data, Off.MagicField10, Values.MagicField10);
        KyaWriteU32(Data, Off.MagicFxDigits, uint32(Values.MagicFxDigits));
        KyaWriteF32(Data, Off.MagicField14, Values.MagicField14);
        KyaWriteU32(Data, Off.MagicField4C, uint32(Values.MagicField4C));
        KyaWriteU32(Data, Off.MagicField18, uint32(Values.MagicField18));
        KyaWriteU32(Data, Off.MagicField28, uint32(Values.MagicField28));
        KyaWriteU32(Data, Off.MagicField24, uint32(Values.MagicField24));
        KyaWriteU32(Data, Off.MagicField30, uint32(Values.MagicField30));
        KyaWriteU32(Data, Off.OpenTextureId, uint32(Values.OpenTextureId));
        KyaWriteU32(Data, Off.OpenMeshId, uint32(Values.OpenMeshId));
        const FKyaStreamEventCamera& C = Values.MagicCamera;
        const int64 O = Off.MagicCamera;
        KyaWriteU32(Data, O, uint32(C.CameraIndex));
        KyaWriteU32(Data, O + 4, uint32(C.Flags));
        KyaWriteF32(Data, O + 8, C.Field8);
        KyaWriteF32(Data, O + 12, C.FieldC);
        KyaWriteF32(Data, O + 16, C.Field10);
        KyaWriteU32(Data, O + 20, uint32(C.ActorRef));
        KyaWriteU32(Data, O + 24, uint32(C.ZoneRef));
        KyaWriteF32(Data, O + 28, C.Field1C);
    }
    if (Off.bMultiCondition)
    {
        KyaWriteU32(Data, Off.MultiField14, uint32(Values.MultiField14));
        KyaWriteU32(Data, Off.MultiField08, uint32(Values.MultiField08));
        KyaWriteU32(Data, Off.MultiField0C, uint32(Values.MultiField0C));
    }
    if (Off.bTarget)
    {
        KyaWriteF32(Data, Off.TargetDisplacement, Values.TargetDisplacement);
        KyaWriteF32(Data, Off.TargetSpringStrength, Values.TargetSpringStrength);
        KyaWriteF32(Data, Off.TargetDamping, Values.TargetDamping);
    }
    if (Off.bWolfen)
    {
        KyaWriteU32(Data, Off.WolfenRequired, uint32(Values.WolfenRequired));
        KyaWriteF32(Data, Off.WolfenTriggerRadius, Values.WolfenTriggerRadius);
        KyaWriteU32(Data, Off.WolfenZoneRef, uint32(Values.WolfenZoneRef));
        KyaWriteU32(Data, Off.WolfenActorRef, uint32(Values.WolfenActorRef));
        KyaWriteU32(Data, Off.WolfenMaterialId, uint32(Values.WolfenMaterialId));
    }
    if (Off.bSequence)
    {
        if (Values.SequenceValues.Num() != Off.SequenceCountValue) return false;
        KyaWriteU32(Data, Off.SequenceField08, uint32(Values.SequenceField08));
        KyaWriteU32(Data, Off.SequenceField0C, uint32(Values.SequenceField0C));
        for (int32 Index = 0; Index < Values.SequenceValues.Num(); ++Index)
            KyaWriteU32(Data, Off.SequenceValues + int64(Index) * 4, uint32(Values.SequenceValues[Index]));
    }
    if (Off.bTimed)
    {
        if (Values.TimedSwitches.Num() != Off.TimedEntryCountValue ||
            Values.TimedDelays.Num() != Off.TimedEntryCountValue) return false;
        KyaWriteF32(Data, Off.TimedBaseDelay, Values.TimedBaseDelay);
        for (int32 Index = 0; Index < Values.TimedSwitches.Num(); ++Index)
        {
            const FKyaSwitchTimedEntryOffsets& Entry = Off.TimedEntries[Index];
            if (Values.TimedSwitches[Index].Targets.Num() != int32(KyaReadU32(Data.GetData() + Entry.Count)))
                return false;
            KyaWriteSwitchData(Data, Values.TimedSwitches[Index], Entry.Targets, Entry.Camera);
            KyaWriteF32(Data, Entry.Delay, Values.TimedDelays[Index]);
        }
        KyaWriteU32(Data, Off.TimedPlaybackMode, uint32(Values.TimedPlaybackMode));
        KyaWriteU32(Data, Off.TimedFlags, uint32(Values.TimedFlags));
    }
    return true;
}
