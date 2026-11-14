#include "Actors/KyaPunchingBall.h"

#include "Actors/KyaFighter.h"
#include "KyaBinaryHelpers.h"
#include "KyaLevelBank.h"

bool KyaReadPunchingBallOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size,
    FKyaPunchingBallOffsets& Out)
{
    Out = FKyaPunchingBallOffsets{};
    if (!KyaRangeValid(Data, Start, Size)) return false;
    const uint8* Bytes = Data.GetData();
    const int64 End = Start + Size;
    const auto U32 = [Bytes](int64 Offset) { return KyaReadU32(Bytes + Offset); };
    int64 Pos = 0;
    if (!KyaReadFighterPrefixEnd(Data, Start, Size, Pos)) return false;
    Out.FighterEnd = Pos;
    if (End - Pos < 84) return false;
    Out.Floats = Pos;
    Pos += 16;
    Out.TrapCount = Pos;
    const uint32 Entries = U32(Pos);
    Out.TrapCountValue = int32(Entries);
    Pos += 4;
    if (uint64(Entries) > uint64(End - Pos) / 60) return false;
    Out.TrapEntries = Pos;
    Pos += int64(Entries) * 60;
    if (End - Pos < 8) return false;
    Out.ActorRefA94 = Pos;
    Out.ActorRefA98 = Pos + 4;
    Pos += 8;
    if (End - Pos < 56) return false;
    Out.CamFig = Pos;
    Pos += 56;
    Out.Tail = Pos;
    return true;
}

bool KyaReadPunchingBallActor(FKyaActorPlacement& Out, TConstArrayView<uint8> Data,
    int64 Start, int64 Size)
{
    FKyaPunchingBallOffsets Off;
    if (!KyaReadPunchingBallOffsets(Data, Start, Size, Off)) return false;
    const uint8* Bytes = Data.GetData();

    Out.PunchingBallFieldA80 = KyaReadF32(Bytes + Off.Floats);
    Out.PunchingBallFieldA84 = KyaReadF32(Bytes + Off.Floats + 4);
    Out.PunchingBallFieldA88 = KyaReadF32(Bytes + Off.Floats + 8);
    Out.PunchingBallFieldA8C = KyaReadF32(Bytes + Off.Floats + 12);

    Out.PunchingBallTrapEntries.Reset();
    for (int32 Index = 0; Index < Off.TrapCountValue; ++Index)
    {
        const int64 E = Off.TrapEntries + int64(Index) * 60;
        FKyaPunchingBallTrapEntry& Entry = Out.PunchingBallTrapEntries.AddDefaulted_GetRef();
        Entry.TargetRef = int32(KyaReadU32(Bytes + E));
        Entry.CutsceneId = int32(KyaReadU32(Bytes + E + 4));
        Entry.Flags = int32(KyaReadU32(Bytes + E + 8));
        Entry.MessageId = int32(KyaReadU32(Bytes + E + 12));
        Entry.MessageFlags = int32(KyaReadU32(Bytes + E + 16));
        Entry.Field14 = int32(KyaReadU32(Bytes + E + 20));
        Entry.Field18 = int32(KyaReadU32(Bytes + E + 24));
        Entry.Camera.CameraIndex = int32(KyaReadU32(Bytes + E + 28));
        Entry.Camera.Flags = int32(KyaReadU32(Bytes + E + 32));
        Entry.Camera.Field8 = KyaReadF32(Bytes + E + 36);
        Entry.Camera.FieldC = KyaReadF32(Bytes + E + 40);
        Entry.Camera.Field10 = KyaReadF32(Bytes + E + 44);
        Entry.Camera.ActorRef = int32(KyaReadU32(Bytes + E + 48));
        Entry.Camera.ZoneRef = int32(KyaReadU32(Bytes + E + 52));
        Entry.Camera.Field1C = KyaReadF32(Bytes + E + 56);
    }

    Out.PunchingBallActorRefA94 = int32(KyaReadU32(Bytes + Off.ActorRefA94));
    Out.PunchingBallActorRefA98 = int32(KyaReadU32(Bytes + Off.ActorRefA98));

    Out.PunchingBallCamFigFloats.Reset();
    for (int32 Index = 0; Index < 14; ++Index)
        Out.PunchingBallCamFigFloats.Add(KyaReadF32(Bytes + Off.CamFig + int64(Index) * 4));

    Out.bHasPunchingBallData = true;
    Out.ClassData.Reset();
    if (Start + Size > Off.Tail)
        Out.ClassData.Append(Bytes + Off.Tail, int32(Start + Size - Off.Tail));
    return true;
}

bool KyaWritePunchingBallActor(const FKyaActorPlacement& Actor, TArray<uint8>& Data,
    int64 Start, int64 Size)
{
    FKyaPunchingBallOffsets Off;
    if (!KyaReadPunchingBallOffsets(MakeArrayView(Data.GetData(), Data.Num()), Start, Size, Off))
        return false;
    if (Actor.PunchingBallTrapEntries.Num() != Off.TrapCountValue) return false;
    if (Actor.PunchingBallCamFigFloats.Num() != 14) return false;

    KyaWriteF32(Data, Off.Floats, Actor.PunchingBallFieldA80);
    KyaWriteF32(Data, Off.Floats + 4, Actor.PunchingBallFieldA84);
    KyaWriteF32(Data, Off.Floats + 8, Actor.PunchingBallFieldA88);
    KyaWriteF32(Data, Off.Floats + 12, Actor.PunchingBallFieldA8C);

    for (int32 Index = 0; Index < Actor.PunchingBallTrapEntries.Num(); ++Index)
    {
        const FKyaPunchingBallTrapEntry& Entry = Actor.PunchingBallTrapEntries[Index];
        const int64 E = Off.TrapEntries + int64(Index) * 60;
        KyaWriteU32(Data, E, uint32(Entry.TargetRef));
        KyaWriteU32(Data, E + 4, uint32(Entry.CutsceneId));
        KyaWriteU32(Data, E + 8, uint32(Entry.Flags));
        KyaWriteU32(Data, E + 12, uint32(Entry.MessageId));
        KyaWriteU32(Data, E + 16, uint32(Entry.MessageFlags));
        KyaWriteU32(Data, E + 20, uint32(Entry.Field14));
        KyaWriteU32(Data, E + 24, uint32(Entry.Field18));
        KyaWriteU32(Data, E + 28, uint32(Entry.Camera.CameraIndex));
        KyaWriteU32(Data, E + 32, uint32(Entry.Camera.Flags));
        KyaWriteF32(Data, E + 36, Entry.Camera.Field8);
        KyaWriteF32(Data, E + 40, Entry.Camera.FieldC);
        KyaWriteF32(Data, E + 44, Entry.Camera.Field10);
        KyaWriteU32(Data, E + 48, uint32(Entry.Camera.ActorRef));
        KyaWriteU32(Data, E + 52, uint32(Entry.Camera.ZoneRef));
        KyaWriteF32(Data, E + 56, Entry.Camera.Field1C);
    }

    KyaWriteU32(Data, Off.ActorRefA94, uint32(Actor.PunchingBallActorRefA94));
    KyaWriteU32(Data, Off.ActorRefA98, uint32(Actor.PunchingBallActorRefA98));
    for (int32 Index = 0; Index < 14; ++Index)
        KyaWriteF32(Data, Off.CamFig + int64(Index) * 4, Actor.PunchingBallCamFigFloats[Index]);
    return true;
}
