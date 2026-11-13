#include "Actors/KyaCheckpoints.h"

#include "KyaLevelBank.h"

namespace
{
uint32 KyaReadU32(const uint8* P)
{
    return uint32(P[0]) | (uint32(P[1]) << 8) | (uint32(P[2]) << 16) | (uint32(P[3]) << 24);
}

void KyaWriteU32(TArray<uint8>& Data, int64 Offset, uint32 Value)
{
    for (int32 Byte = 0; Byte < 4; ++Byte) Data[int32(Offset) + Byte] = uint8(Value >> (Byte * 8));
}

bool KyaRangeValid(TConstArrayView<uint8> Data, int64 Start, int64 Size)
{
    return Start >= 0 && Size >= 0 && Start <= Data.Num() && Size <= Data.Num() - Start;
}
}

bool KyaReadCheckpointsOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size,
    FKyaCheckpointsOffsets& Out)
{
    Out = FKyaCheckpointsOffsets{};
    if (!KyaRangeValid(Data, Start, Size) || Size < 4) return false;
    const uint8* Bytes = Data.GetData();
    const int64 End = Start + Size;
    Out.Count = Start;
    const uint32 Count = KyaReadU32(Bytes + Start);
    Out.CountValue = int32(Count);
    int64 Pos = Start + 4;
    if (uint64(Count) > uint64(End - Pos) / 24) return false;
    for (uint32 Index = 0; Index < Count; ++Index)
    {
        if (End - Pos < 24) return false;
        const uint32 Pairs = KyaReadU32(Bytes + Pos + 20);
        if (uint64(Pairs) > uint64(End - Pos - 24) / 8) return false;
        FKyaCheckpointRecordOffsets& Record = Out.Records.AddDefaulted_GetRef();
        Record.Record = Pos;
        Record.ActorWaypointCount = int32(Pairs);
        Record.ActorWaypoints = Pos + 24;
        Pos += 24 + int64(Pairs) * 8;
    }
    Out.Tail = Pos;
    return true;
}

bool KyaReadCheckpointsActor(FKyaActorPlacement& Out, TConstArrayView<uint8> Data, int64 Start,
    int64 Size)
{
    FKyaCheckpointsOffsets Off;
    if (!KyaReadCheckpointsOffsets(Data, Start, Size, Off)) return false;
    const uint8* Bytes = Data.GetData();

    Out.Checkpoints.Reset();
    for (const FKyaCheckpointRecordOffsets& Record : Off.Records)
    {
        FKyaCheckpoint& Checkpoint = Out.Checkpoints.AddDefaulted_GetRef();
        Checkpoint.ZoneRef = int32(KyaReadU32(Bytes + Record.Record));
        Checkpoint.WaypointA = int32(KyaReadU32(Bytes + Record.Record + 4));
        Checkpoint.SectorId = int32(KyaReadU32(Bytes + Record.Record + 8));
        Checkpoint.WaypointB = int32(KyaReadU32(Bytes + Record.Record + 12));
        Checkpoint.Flags = KyaReadU32(Bytes + Record.Record + 16);
        for (int32 Pair = 0; Pair < Record.ActorWaypointCount; ++Pair)
        {
            FKyaCheckpointActorWaypoint& Ref = Checkpoint.ActorWaypoints.AddDefaulted_GetRef();
            Ref.ActorRef = int32(KyaReadU32(Bytes + Record.ActorWaypoints + int64(Pair) * 8));
            Ref.WaypointRef = int32(KyaReadU32(Bytes + Record.ActorWaypoints + int64(Pair) * 8 + 4));
        }
    }

    Out.bHasTypedCheckpoints = true;
    Out.ClassData.Reset();
    if (Start + Size > Off.Tail)
        Out.ClassData.Append(Bytes + Off.Tail, int32(Start + Size - Off.Tail));
    return true;
}

bool KyaWriteCheckpointsActor(const FKyaActorPlacement& Actor, TArray<uint8>& Data, int64 Start,
    int64 Size)
{
    FKyaCheckpointsOffsets Off;
    if (!KyaReadCheckpointsOffsets(MakeArrayView(Data.GetData(), Data.Num()), Start, Size, Off))
        return false;
    if (Actor.Checkpoints.Num() != Off.CountValue) return false;
    for (int32 Index = 0; Index < Actor.Checkpoints.Num(); ++Index)
    {
        const FKyaCheckpoint& Checkpoint = Actor.Checkpoints[Index];
        const FKyaCheckpointRecordOffsets& Record = Off.Records[Index];
        if (Checkpoint.ActorWaypoints.Num() != Record.ActorWaypointCount) return false;
        if (Checkpoint.Flags < 0 || Checkpoint.Flags > MAX_uint32) return false;
        KyaWriteU32(Data, Record.Record, uint32(Checkpoint.ZoneRef));
        KyaWriteU32(Data, Record.Record + 4, uint32(Checkpoint.WaypointA));
        KyaWriteU32(Data, Record.Record + 8, uint32(Checkpoint.SectorId));
        KyaWriteU32(Data, Record.Record + 12, uint32(Checkpoint.WaypointB));
        KyaWriteU32(Data, Record.Record + 16, uint32(Checkpoint.Flags));
        for (int32 Pair = 0; Pair < Checkpoint.ActorWaypoints.Num(); ++Pair)
        {
            KyaWriteU32(Data, Record.ActorWaypoints + int64(Pair) * 8,
                uint32(Checkpoint.ActorWaypoints[Pair].ActorRef));
            KyaWriteU32(Data, Record.ActorWaypoints + int64(Pair) * 8 + 4,
                uint32(Checkpoint.ActorWaypoints[Pair].WaypointRef));
        }
    }
    return true;
}
