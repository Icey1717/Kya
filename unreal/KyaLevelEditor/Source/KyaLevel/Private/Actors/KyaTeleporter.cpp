#include "Actors/KyaTeleporter.h"

#include "Actors/KyaSwitch.h"
#include "KyaBinaryHelpers.h"
#include "KyaLevelBank.h"

bool KyaReadTeleporterOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size,
    FKyaTeleporterOffsets& Out)
{
    Out = FKyaTeleporterOffsets{};
    if (!KyaRangeValid(Data, Start, Size)) return false;
    const uint8* Bytes = Data.GetData();
    const int64 End = Start + Size;
    int64 Pos = Start;
    if (End - Pos < 16) return false;
    Out.SubsectorMaterialId = Pos;
    Out.Hash = Pos + 4;
    Out.Field16C = Pos + 12;
    Pos += 16;
    if (End - Pos < 4) return false;
    Out.DestCount = Pos;
    const uint32 Destinations = KyaReadU32(Bytes + Pos);
    Out.DestCountValue = int32(Destinations);
    Pos += 4;
    if (uint64(Destinations) > uint64(End - Pos) / 12) return false;
    Out.DestEntries = Pos;
    Pos += int64(Destinations) * 12;
    for (int32 Switch = 0; Switch < 3; ++Switch)
    {
        if (End - Pos < 4) return false;
        FKyaTeleporterSwitchOffsets& S = Out.Switches[Switch];
        const uint32 Count = KyaReadU32(Bytes + Pos);
        S.Count = int32(Count);
        S.Targets = Pos + 4;
        if (Count > uint64(End - Pos - 4) / 28) return false;
        S.Camera = S.Targets + int64(Count) * 28;
        if (End - S.Camera < 32) return false;
        Pos = S.Camera + 32;
    }
    if (End - Pos < 36) return false;
    Out.Zone188 = Pos;
    Out.Zone18C = Pos + 4;
    Out.Material1 = Pos + 8;
    Out.CinematicId = Pos + 12;
    Out.Material2 = Pos + 16;
    Out.Field1D4 = Pos + 20;
    Out.Field1D8 = Pos + 24;
    Out.Material3 = Pos + 28;
    Out.Field1E0 = Pos + 32;
    Pos += 36;
    if (End - Pos < 4) return false;
    Out.WaypointCount = Pos;
    const uint32 Waypoints = KyaReadU32(Bytes + Pos);
    Out.WaypointCountValue = int32(Waypoints);
    Pos += 4;
    if (uint64(Waypoints) > uint64(End - Pos) / 8) return false;
    Out.WaypointEntries = Pos;
    Pos += int64(Waypoints) * 8;
    if (End - Pos < 4) return false;
    Out.CndOpCount = Pos;
    const uint32 Blocks = KyaReadU32(Bytes + Pos);
    Pos += 4;
    for (uint32 Block = 0; Block < Blocks; ++Block)
    {
        if (End - Pos < 4) return false;
        const int64 OpCountOffset = Pos;
        const uint32 Ops = KyaReadU32(Bytes + Pos);
        Pos += 4;
        if (uint64(Ops) > uint64(End - Pos) / 12) return false;
        Out.BlockOpCounts.Add(OpCountOffset);
        Out.BlockOpCountValues.Add(int32(Ops));
        Out.BlockOps.Add(Pos);
        Pos += int64(Ops) * 12;
        if (End - Pos < 4) return false;
        const int64 CondCountOffset = Pos;
        const uint32 Conds = KyaReadU32(Bytes + Pos);
        Pos += 4;
        if (uint64(Conds) > uint64(End - Pos) / 16) return false;
        Out.BlockCondCounts.Add(CondCountOffset);
        Out.BlockCondCountValues.Add(int32(Conds));
        Out.BlockConds.Add(Pos);
        Pos += int64(Conds) * 16;
    }
    Out.Tail = Pos;
    return true;
}

bool KyaReadTeleporterActor(FKyaActorPlacement& Out, TConstArrayView<uint8> Data, int64 Start,
    int64 Size)
{
    FKyaTeleporterOffsets Off;
    if (!KyaReadTeleporterOffsets(Data, Start, Size, Off)) return false;
    const uint8* Bytes = Data.GetData();

    Out.TeleporterSubsectorMaterialId = int32(KyaReadU32(Bytes + Off.SubsectorMaterialId));
    Out.TeleporterHash = int64(uint64(KyaReadU32(Bytes + Off.Hash)) |
        (uint64(KyaReadU32(Bytes + Off.Hash + 4)) << 32));
    Out.TeleporterField16C = int32(KyaReadU32(Bytes + Off.Field16C));
    Out.TeleporterDestinations.Reset();
    for (int32 Index = 0; Index < Off.DestCountValue; ++Index)
    {
        FKyaTeleporterDestination& Destination = Out.TeleporterDestinations.AddDefaulted_GetRef();
        const int64 D = Off.DestEntries + int64(Index) * 12;
        Destination.LevelId = int32(KyaReadU32(Bytes + D));
        Destination.ElevatorId = int32(KyaReadU32(Bytes + D + 4));
        Destination.Field8 = int32(KyaReadU32(Bytes + D + 8));
    }

    FKyaSwitch* Switches[] = { &Out.TeleporterSwitch170, &Out.TeleporterSwitch178,
        &Out.TeleporterSwitch180 };
    for (int32 Index = 0; Index < 3; ++Index)
    {
        *Switches[Index] = FKyaSwitch{};
        if (!KyaReadSwitchValue(Data, Off.Switches[Index].Targets, Off.Switches[Index].Count,
            Off.Switches[Index].Camera, *Switches[Index])) return false;
    }

    Out.TeleporterZone188 = int32(KyaReadU32(Bytes + Off.Zone188));
    Out.TeleporterZone18C = int32(KyaReadU32(Bytes + Off.Zone18C));
    Out.TeleporterMaterialId1 = int32(KyaReadU32(Bytes + Off.Material1));
    Out.TeleporterCinematicId = int32(KyaReadU32(Bytes + Off.CinematicId));
    Out.TeleporterMaterialId2 = int32(KyaReadU32(Bytes + Off.Material2));
    Out.TeleporterField1D4 = int32(KyaReadU32(Bytes + Off.Field1D4));
    Out.TeleporterField1D8 = int32(KyaReadU32(Bytes + Off.Field1D8));
    Out.TeleporterMaterialId3 = int32(KyaReadU32(Bytes + Off.Material3));
    Out.TeleporterField1E0 = int32(KyaReadU32(Bytes + Off.Field1E0));
    Out.TeleporterWaypoints.Reset();
    for (int32 Index = 0; Index < Off.WaypointCountValue; ++Index)
    {
        FKyaTeleporterWaypoint& Waypoint = Out.TeleporterWaypoints.AddDefaulted_GetRef();
        Waypoint.ActorRef = int32(KyaReadU32(Bytes + Off.WaypointEntries + int64(Index) * 8));
        Waypoint.WaypointRef = int32(KyaReadU32(Bytes + Off.WaypointEntries + int64(Index) * 8 + 4));
    }

    Out.TeleporterConditionedOperations.Reset();
    for (int32 Index = 0; Index < Off.BlockOps.Num(); ++Index)
    {
        FKyaTeleporterBlock& Block = Out.TeleporterConditionedOperations.AddDefaulted_GetRef();
        for (int32 Op = 0; Op < Off.BlockOpCountValues[Index]; ++Op)
        {
            FKyaTeleporterOperation& Operation = Block.Operations.AddDefaulted_GetRef();
            const int64 O = Off.BlockOps[Index] + int64(Op) * 12;
            Operation.ScenaricVariable = int32(KyaReadU32(Bytes + O));
            Operation.Operation = int32(KyaReadU32(Bytes + O + 4));
            Operation.Value = int32(KyaReadU32(Bytes + O + 8));
        }
        for (int32 Cond = 0; Cond < Off.BlockCondCountValues[Index]; ++Cond)
        {
            FKyaTeleporterCondition& Condition = Block.Conditions.AddDefaulted_GetRef();
            const int64 C = Off.BlockConds[Index] + int64(Cond) * 16;
            Condition.Combine = int32(KyaReadU32(Bytes + C));
            Condition.Variable = int32(KyaReadU32(Bytes + C + 4));
            Condition.Comparison = int32(KyaReadU32(Bytes + C + 8));
            Condition.Value = int32(KyaReadU32(Bytes + C + 12));
        }
    }

    Out.bHasTeleporterData = true;
    Out.ClassData.Reset();
    if (Start + Size > Off.Tail)
        Out.ClassData.Append(Bytes + Off.Tail, int32(Start + Size - Off.Tail));
    return true;
}

bool KyaWriteTeleporterActor(const FKyaActorPlacement& Actor, TArray<uint8>& Data, int64 Start,
    int64 Size)
{
    FKyaTeleporterOffsets Off;
    if (!KyaReadTeleporterOffsets(MakeArrayView(Data.GetData(), Data.Num()), Start, Size, Off))
        return false;
    if (Actor.TeleporterDestinations.Num() != Off.DestCountValue ||
        Actor.TeleporterWaypoints.Num() != Off.WaypointCountValue ||
        Actor.TeleporterConditionedOperations.Num() != Off.BlockOps.Num()) return false;

    KyaWriteU32(Data, Off.SubsectorMaterialId, uint32(Actor.TeleporterSubsectorMaterialId));
    KyaWriteU32(Data, Off.Hash, uint32(uint64(Actor.TeleporterHash)));
    KyaWriteU32(Data, Off.Hash + 4, uint32(uint64(Actor.TeleporterHash) >> 32));
    KyaWriteU32(Data, Off.Field16C, uint32(Actor.TeleporterField16C));
    for (int32 Index = 0; Index < Actor.TeleporterDestinations.Num(); ++Index)
    {
        const FKyaTeleporterDestination& Destination = Actor.TeleporterDestinations[Index];
        const int64 D = Off.DestEntries + int64(Index) * 12;
        KyaWriteU32(Data, D, uint32(Destination.LevelId));
        KyaWriteU32(Data, D + 4, uint32(Destination.ElevatorId));
        KyaWriteU32(Data, D + 8, uint32(Destination.Field8));
    }

    const FKyaSwitch* Switches[] = { &Actor.TeleporterSwitch170, &Actor.TeleporterSwitch178,
        &Actor.TeleporterSwitch180 };
    for (int32 Index = 0; Index < 3; ++Index)
    {
        if (Switches[Index]->Targets.Num() != Off.Switches[Index].Count) return false;
        KyaWriteSwitchValue(Data, *Switches[Index], Off.Switches[Index].Targets,
            Off.Switches[Index].Camera);
    }

    KyaWriteU32(Data, Off.Zone188, uint32(Actor.TeleporterZone188));
    KyaWriteU32(Data, Off.Zone18C, uint32(Actor.TeleporterZone18C));
    KyaWriteU32(Data, Off.Material1, uint32(Actor.TeleporterMaterialId1));
    KyaWriteU32(Data, Off.CinematicId, uint32(Actor.TeleporterCinematicId));
    KyaWriteU32(Data, Off.Material2, uint32(Actor.TeleporterMaterialId2));
    KyaWriteU32(Data, Off.Field1D4, uint32(Actor.TeleporterField1D4));
    KyaWriteU32(Data, Off.Field1D8, uint32(Actor.TeleporterField1D8));
    KyaWriteU32(Data, Off.Material3, uint32(Actor.TeleporterMaterialId3));
    KyaWriteU32(Data, Off.Field1E0, uint32(Actor.TeleporterField1E0));
    for (int32 Index = 0; Index < Actor.TeleporterWaypoints.Num(); ++Index)
    {
        KyaWriteU32(Data, Off.WaypointEntries + int64(Index) * 8,
            uint32(Actor.TeleporterWaypoints[Index].ActorRef));
        KyaWriteU32(Data, Off.WaypointEntries + int64(Index) * 8 + 4,
            uint32(Actor.TeleporterWaypoints[Index].WaypointRef));
    }
    for (int32 BlockIndex = 0; BlockIndex < Actor.TeleporterConditionedOperations.Num(); ++BlockIndex)
    {
        const FKyaTeleporterBlock& Block = Actor.TeleporterConditionedOperations[BlockIndex];
        if (Block.Operations.Num() != Off.BlockOpCountValues[BlockIndex] ||
            Block.Conditions.Num() != Off.BlockCondCountValues[BlockIndex]) return false;
        for (int32 Op = 0; Op < Block.Operations.Num(); ++Op)
        {
            const FKyaTeleporterOperation& Operation = Block.Operations[Op];
            const int64 O = Off.BlockOps[BlockIndex] + int64(Op) * 12;
            KyaWriteU32(Data, O, uint32(Operation.ScenaricVariable));
            KyaWriteU32(Data, O + 4, uint32(Operation.Operation));
            KyaWriteU32(Data, O + 8, uint32(Operation.Value));
        }
        for (int32 Cond = 0; Cond < Block.Conditions.Num(); ++Cond)
        {
            const FKyaTeleporterCondition& Condition = Block.Conditions[Cond];
            const int64 C = Off.BlockConds[BlockIndex] + int64(Cond) * 16;
            KyaWriteU32(Data, C, uint32(Condition.Combine));
            KyaWriteU32(Data, C + 4, uint32(Condition.Variable));
            KyaWriteU32(Data, C + 8, uint32(Condition.Comparison));
            KyaWriteU32(Data, C + 12, uint32(Condition.Value));
        }
    }
    return true;
}
