#include "Actors/KyaCommander.h"

#include "KyaBinaryHelpers.h"
#include "KyaLevelBank.h"

bool KyaReadCommanderOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size,
    FKyaCommanderOffsets& Out)
{
    Out = FKyaCommanderOffsets{};
    if (!KyaRangeValid(Data, Start, Size) || Size < 96) return false;
    const uint8* Bytes = Data.GetData();
    const int64 End = Start + Size;
    Out.Headers = Start;
    Out.Squad = Start + 20;
    const uint32 Teams = KyaReadU32(Bytes + Start + 32);
    Out.TeamCount = int32(Teams);
    Out.Teams = Start + 36;
    int64 Pos = Out.Teams + int64(Teams) * 4;
    if (End - Pos < 4) return false;
    const uint32 Components = KyaReadU32(Bytes + Pos);
    Out.ComponentCount = int32(Components);
    Out.Components = Pos + 4;
    if (uint64(Components) > uint64(End - Pos - 4) / 8) return false;
    Pos = Out.Components + int64(Components) * 8;
    if (End - Pos < 56) return false;
    Out.Camera = Pos;
    Out.Tail = Pos + 56;
    return Out.Tail <= End;
}

bool KyaReadCommanderActor(FKyaActorPlacement& Out, TConstArrayView<uint8> Data, int64 Start,
    int64 Size)
{
    FKyaCommanderOffsets Off;
    if (!KyaReadCommanderOffsets(Data, Start, Size, Off)) return false;
    const uint8* Bytes = Data.GetData();
    Out.CommanderDetectAreaZoneId = int32(KyaReadU32(Bytes + Off.Headers));
    Out.CommanderGuardAreaZoneId = int32(KyaReadU32(Bytes + Off.Headers + 4));
    Out.CommanderField180 = int32(KyaReadU32(Bytes + Off.Headers + 8));
    Out.CommanderNewPathFindingId = int32(KyaReadU32(Bytes + Off.Headers + 12));
    Out.CommanderActorRef = int32(KyaReadU32(Bytes + Off.Headers + 16));
    Out.CommanderSquadValue = KyaReadF32(Bytes + Off.Squad);
    Out.CommanderSquadField4 = int32(KyaReadU32(Bytes + Off.Squad + 4));
    Out.CommanderSquadField8 = int32(KyaReadU32(Bytes + Off.Squad + 8));
    Out.CommanderTeamEnemies.Reset();
    for (int32 Index = 0; Index < Off.TeamCount; ++Index)
        Out.CommanderTeamEnemies.Add(int32(KyaReadU32(Bytes + Off.Teams + int64(Index) * 4)));
    Out.CommanderSequence.Reset();
    for (int32 Index = 0; Index < Off.ComponentCount; ++Index)
    {
        FKyaCommanderComponent& Component = Out.CommanderSequence.AddDefaulted_GetRef();
        Component.Field0 = int32(KyaReadU32(Bytes + Off.Components + int64(Index) * 8));
        Component.Field8 = int32(KyaReadU32(Bytes + Off.Components + int64(Index) * 8 + 4));
    }
    Out.CommanderCameraValues.Reset();
    for (int32 Index = 0; Index < 14; ++Index)
        Out.CommanderCameraValues.Add(KyaReadF32(Bytes + Off.Camera + Index * 4));

    Out.bHasCommanderData = true;
    Out.ClassData.Reset();
    if (Start + Size > Off.Tail)
        Out.ClassData.Append(Bytes + Off.Tail, int32(Start + Size - Off.Tail));
    return true;
}

bool KyaWriteCommanderActor(const FKyaActorPlacement& Actor, TArray<uint8>& Data, int64 Start,
    int64 Size)
{
    FKyaCommanderOffsets Off;
    if (!KyaReadCommanderOffsets(MakeArrayView(Data.GetData(), Data.Num()), Start, Size, Off))
        return false;
    if (Actor.CommanderTeamEnemies.Num() != Off.TeamCount ||
        Actor.CommanderSequence.Num() != Off.ComponentCount ||
        Actor.CommanderCameraValues.Num() != 14) return false;
    KyaWriteU32(Data, Off.Headers, uint32(Actor.CommanderDetectAreaZoneId));
    KyaWriteU32(Data, Off.Headers + 4, uint32(Actor.CommanderGuardAreaZoneId));
    KyaWriteU32(Data, Off.Headers + 8, uint32(Actor.CommanderField180));
    KyaWriteU32(Data, Off.Headers + 12, uint32(Actor.CommanderNewPathFindingId));
    KyaWriteU32(Data, Off.Headers + 16, uint32(Actor.CommanderActorRef));
    KyaWriteF32(Data, Off.Squad, Actor.CommanderSquadValue);
    KyaWriteU32(Data, Off.Squad + 4, uint32(Actor.CommanderSquadField4));
    KyaWriteU32(Data, Off.Squad + 8, uint32(Actor.CommanderSquadField8));
    for (int32 Index = 0; Index < Actor.CommanderTeamEnemies.Num(); ++Index)
        KyaWriteU32(Data, Off.Teams + int64(Index) * 4, uint32(Actor.CommanderTeamEnemies[Index]));
    for (int32 Index = 0; Index < Actor.CommanderSequence.Num(); ++Index)
    {
        KyaWriteU32(Data, Off.Components + int64(Index) * 8,
            uint32(Actor.CommanderSequence[Index].Field0));
        KyaWriteU32(Data, Off.Components + int64(Index) * 8 + 4,
            uint32(Actor.CommanderSequence[Index].Field8));
    }
    for (int32 Index = 0; Index < Actor.CommanderCameraValues.Num(); ++Index)
        KyaWriteF32(Data, Off.Camera + Index * 4, Actor.CommanderCameraValues[Index]);
    return true;
}

bool KyaReadCommanderBehaviourOffsets(TConstArrayView<uint8> Data, int64 Start, int64 End,
    FKyaCommanderBehaviourOffsets& Out)
{
    Out = FKyaCommanderBehaviourOffsets{};
    if (Start < 0 || Start > Data.Num() || End < Start + 44 || End > Data.Num()) return false;
    Out.SquadConfig = Start;
    Out.Switch = Start + 8;
    const uint32 Count = KyaReadU32(Data.GetData() + Out.Switch);
    if (Count > uint64(End - Out.Switch - 36) / 28) return false;
    Out.Tail = Out.Switch + 4 + int64(Count) * 28 + 32;
    return Out.Tail <= End;
}

bool KyaReadCommanderBehaviour(FKyaActorBehaviour& Out, int32 Id, TConstArrayView<uint8> Data,
    int64 Start, int32 Size)
{
    if (Id != 2) return false;
    FKyaCommanderBehaviourOffsets Off;
    if (!KyaReadCommanderBehaviourOffsets(Data, Start, Start + Size, Off)) return false;
    const uint8* Bytes = Data.GetData();
    Out.CommanderSquadConfig.Reset();
    Out.CommanderSquadConfig.Add(int32(KyaReadU32(Bytes + Off.SquadConfig)));
    Out.CommanderSquadConfig.Add(int32(KyaReadU32(Bytes + Off.SquadConfig + 4)));
    const uint32 Count = KyaReadU32(Bytes + Off.Switch);
    Out.CommanderSwitch = FKyaSwitch{};
    for (uint32 Index = 0; Index < Count; ++Index)
    {
        FKyaSwitchTarget& Target = Out.CommanderSwitch.Targets.AddDefaulted_GetRef();
        const int64 T = Off.Switch + 4 + int64(Index) * 28;
        Target.TargetRef = int32(KyaReadU32(Bytes + T));
        Target.CutsceneId = int32(KyaReadU32(Bytes + T + 4));
        Target.Flags = int32(KyaReadU32(Bytes + T + 8));
        Target.MessageId = int32(KyaReadU32(Bytes + T + 12));
        Target.MessageFlags = int32(KyaReadU32(Bytes + T + 16));
        Target.Field14 = int32(KyaReadU32(Bytes + T + 20));
        Target.Field18 = int32(KyaReadU32(Bytes + T + 24));
    }
    const int64 C = Off.Switch + 4 + int64(Count) * 28;
    Out.CommanderSwitch.Camera.CameraIndex = int32(KyaReadU32(Bytes + C));
    Out.CommanderSwitch.Camera.Flags = int32(KyaReadU32(Bytes + C + 4));
    Out.CommanderSwitch.Camera.Field8 = KyaReadF32(Bytes + C + 8);
    Out.CommanderSwitch.Camera.FieldC = KyaReadF32(Bytes + C + 12);
    Out.CommanderSwitch.Camera.Field10 = KyaReadF32(Bytes + C + 16);
    Out.CommanderSwitch.Camera.ActorRef = int32(KyaReadU32(Bytes + C + 20));
    Out.CommanderSwitch.Camera.ZoneRef = int32(KyaReadU32(Bytes + C + 24));
    Out.CommanderSwitch.Camera.Field1C = KyaReadF32(Bytes + C + 28);
    Out.bHasTypedCommanderBehaviour = true;
    return true;
}

bool KyaWriteCommanderBehaviour(const FKyaActorBehaviour& Behaviour, TArray<uint8>& Data,
    int64 Start, int32 Size)
{
    if (Behaviour.Id != 2) return false;
    FKyaCommanderBehaviourOffsets Off;
    if (!KyaReadCommanderBehaviourOffsets(MakeArrayView(Data.GetData(), Data.Num()), Start,
        Start + Size, Off)) return false;
    if (Behaviour.CommanderSquadConfig.Num() != 2) return false;
    if (Behaviour.CommanderSwitch.Targets.Num() != int32(KyaReadU32(Data.GetData() + Off.Switch)))
        return false;
    KyaWriteU32(Data, Off.SquadConfig, uint32(Behaviour.CommanderSquadConfig[0]));
    KyaWriteU32(Data, Off.SquadConfig + 4, uint32(Behaviour.CommanderSquadConfig[1]));
    for (int32 Index = 0; Index < Behaviour.CommanderSwitch.Targets.Num(); ++Index)
    {
        const FKyaSwitchTarget& Target = Behaviour.CommanderSwitch.Targets[Index];
        const int64 T = Off.Switch + 4 + int64(Index) * 28;
        KyaWriteU32(Data, T, uint32(Target.TargetRef));
        KyaWriteU32(Data, T + 4, uint32(Target.CutsceneId));
        KyaWriteU32(Data, T + 8, uint32(Target.Flags));
        KyaWriteU32(Data, T + 12, uint32(Target.MessageId));
        KyaWriteU32(Data, T + 16, uint32(Target.MessageFlags));
        KyaWriteU32(Data, T + 20, uint32(Target.Field14));
        KyaWriteU32(Data, T + 24, uint32(Target.Field18));
    }
    const int64 C = Off.Switch + 4 + int64(Behaviour.CommanderSwitch.Targets.Num()) * 28;
    const FKyaStreamEventCamera& Cam = Behaviour.CommanderSwitch.Camera;
    KyaWriteU32(Data, C, uint32(Cam.CameraIndex));
    KyaWriteU32(Data, C + 4, uint32(Cam.Flags));
    KyaWriteF32(Data, C + 8, Cam.Field8);
    KyaWriteF32(Data, C + 12, Cam.FieldC);
    KyaWriteF32(Data, C + 16, Cam.Field10);
    KyaWriteU32(Data, C + 20, uint32(Cam.ActorRef));
    KyaWriteU32(Data, C + 24, uint32(Cam.ZoneRef));
    KyaWriteF32(Data, C + 28, Cam.Field1C);
    return true;
}
