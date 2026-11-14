#include "Actors/KyaTrap.h"

#include "KyaBinaryHelpers.h"
#include "KyaLevelBank.h"

bool KyaReadTrapOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size, FKyaTrapOffsets& Out)
{
    Out = FKyaTrapOffsets{};
    if (!KyaRangeValid(Data, Start, Size) || Size < 52) return false;
    const uint8* Bytes = Data.GetData();
    const int64 End = Start + Size;
    int64 Pos = Start;
    Out.WeightB = Pos;
    const uint32 ActorCount = KyaReadU32(Bytes + Pos + 4);
    Pos += 8;
    if (ActorCount > uint64(End - Pos) / 4) return false;
    Out.ActorCount = int32(ActorCount); Out.Actors = Pos; Pos += int64(ActorCount) * 4;
    if (End - Pos < 16) return false;
    Out.Field14 = Pos; Out.Field18 = Pos + 4; Out.Field1C = Pos + 8; Out.Field20 = Pos + 12;
    Pos += 16;

    // The optional field 0x24 adds four bytes, but actor/trap lists are variable
    // length. Select the layout by requiring the candidate parse to consume the
    // exact payload instead of thresholding on the total size.
    const auto TryLayout = [&](bool bModern)
    {
        FKyaTrapOffsets Candidate = Out;
        Candidate.bHasField24 = bModern;
        int64 Cursor = Pos;
        if (bModern)
        {
            if (End - Cursor < 4) return false;
            Candidate.Field24 = Cursor; Cursor += 4;
        }
        if (End - Cursor < 24) return false;
        Candidate.ActorRef = Cursor; Candidate.Speed = Cursor + 4; Candidate.Acceleration = Cursor + 8;
        Candidate.PathFollow = Cursor + 12; Candidate.Field4C = Cursor + 16;
        Candidate.RequiredEscapes = Cursor + 20;
        Cursor += 24;
        if (End - Cursor < 4) return false;
        const uint32 TrapCount = KyaReadU32(Bytes + Cursor);
        Cursor += 4;
        if (TrapCount > uint64(End - Cursor) / 60) return false;
        Candidate.TrapCount = int32(TrapCount); Candidate.TrapEntries = Cursor;
        Cursor += int64(TrapCount) * 60;
        if (Cursor != End) return false;
        Candidate.Tail = Cursor;
        Out = Candidate;
        return true;
    };
    if (TryLayout(false)) return true;
    if (TryLayout(true)) return true;
    return false;
}

bool KyaReadTrapActor(FKyaActorPlacement& Out, TConstArrayView<uint8> Data, int64 Start,
    int64 Size)
{
    FKyaTrapOffsets Off;
    if (!KyaReadTrapOffsets(Data, Start, Size, Off)) return false;
    const uint8* Bytes = Data.GetData();
    Out.TrapWeightB = KyaReadF32(Bytes + Off.WeightB);
    Out.TrapActorRefs.Reset();
    for (int32 Index = 0; Index < Off.ActorCount; ++Index)
        Out.TrapActorRefs.Add(int32(KyaReadU32(Bytes + Off.Actors + int64(Index) * 4)));
    Out.TrapField14 = int32(KyaReadU32(Bytes + Off.Field14));
    Out.TrapField18 = int32(KyaReadU32(Bytes + Off.Field18));
    Out.TrapField1C = int32(KyaReadU32(Bytes + Off.Field1C));
    Out.TrapField20 = int32(KyaReadU32(Bytes + Off.Field20));
    Out.bHasTrapField24 = Off.bHasField24;
    if (Off.bHasField24) Out.TrapField24 = int32(KyaReadU32(Bytes + Off.Field24));
    Out.TrapActorRef = int32(KyaReadU32(Bytes + Off.ActorRef));
    Out.TrapSpeed = KyaReadF32(Bytes + Off.Speed);
    Out.TrapAcceleration = KyaReadF32(Bytes + Off.Acceleration);
    Out.TrapPathFollowIndex = int32(KyaReadU32(Bytes + Off.PathFollow));
    Out.TrapField4C = KyaReadF32(Bytes + Off.Field4C);
    Out.TrapRequiredEscapeAttempts = int32(KyaReadU32(Bytes + Off.RequiredEscapes));
    Out.TrapStreamEntries.Reset();
    for (int32 Index = 0; Index < Off.TrapCount; ++Index)
    {
        FKyaTrapStreamEntry& Entry = Out.TrapStreamEntries.AddDefaulted_GetRef();
        const int64 P = Off.TrapEntries + int64(Index) * 60;
        Entry.Target.TargetRef = int32(KyaReadU32(Bytes + P));
        Entry.Target.CutsceneId = int32(KyaReadU32(Bytes + P + 4));
        Entry.Target.Flags = int32(KyaReadU32(Bytes + P + 8));
        Entry.Target.MessageId = int32(KyaReadU32(Bytes + P + 12));
        Entry.Target.MessageFlags = int32(KyaReadU32(Bytes + P + 16));
        Entry.Target.Field14 = int32(KyaReadU32(Bytes + P + 20));
        Entry.Target.Field18 = int32(KyaReadU32(Bytes + P + 24));
        Entry.Camera.CameraIndex = int32(KyaReadU32(Bytes + P + 28));
        Entry.Camera.Flags = int32(KyaReadU32(Bytes + P + 32));
        Entry.Camera.Field8 = KyaReadF32(Bytes + P + 36);
        Entry.Camera.FieldC = KyaReadF32(Bytes + P + 40);
        Entry.Camera.Field10 = KyaReadF32(Bytes + P + 44);
        Entry.Camera.ActorRef = int32(KyaReadU32(Bytes + P + 48));
        Entry.Camera.ZoneRef = int32(KyaReadU32(Bytes + P + 52));
        Entry.Camera.Field1C = KyaReadF32(Bytes + P + 56);
    }

    Out.bHasTrapData = true;
    Out.ClassData.Reset();
    if (Start + Size > Off.Tail)
        Out.ClassData.Append(Bytes + Off.Tail, int32(Start + Size - Off.Tail));
    return true;
}

bool KyaWriteTrapActor(const FKyaActorPlacement& Actor, TArray<uint8>& Data, int64 Start,
    int64 Size)
{
    FKyaTrapOffsets Off;
    if (!KyaReadTrapOffsets(MakeArrayView(Data.GetData(), Data.Num()), Start, Size, Off))
        return false;
    if (Actor.TrapActorRefs.Num() != Off.ActorCount ||
        Actor.TrapStreamEntries.Num() != Off.TrapCount ||
        Actor.bHasTrapField24 != Off.bHasField24) return false;

    KyaWriteF32(Data, Off.WeightB, Actor.TrapWeightB);
    for (int32 Index = 0; Index < Actor.TrapActorRefs.Num(); ++Index)
        KyaWriteU32(Data, Off.Actors + int64(Index) * 4, uint32(Actor.TrapActorRefs[Index]));
    KyaWriteU32(Data, Off.Field14, uint32(Actor.TrapField14));
    KyaWriteU32(Data, Off.Field18, uint32(Actor.TrapField18));
    KyaWriteU32(Data, Off.Field1C, uint32(Actor.TrapField1C));
    KyaWriteU32(Data, Off.Field20, uint32(Actor.TrapField20));
    if (Off.bHasField24) KyaWriteU32(Data, Off.Field24, uint32(Actor.TrapField24));
    KyaWriteU32(Data, Off.ActorRef, uint32(Actor.TrapActorRef));
    KyaWriteF32(Data, Off.Speed, Actor.TrapSpeed);
    KyaWriteF32(Data, Off.Acceleration, Actor.TrapAcceleration);
    KyaWriteU32(Data, Off.PathFollow, uint32(Actor.TrapPathFollowIndex));
    KyaWriteF32(Data, Off.Field4C, Actor.TrapField4C);
    KyaWriteU32(Data, Off.RequiredEscapes, uint32(Actor.TrapRequiredEscapeAttempts));
    for (int32 Index = 0; Index < Actor.TrapStreamEntries.Num(); ++Index)
    {
        const FKyaTrapStreamEntry& Entry = Actor.TrapStreamEntries[Index];
        const int64 P = Off.TrapEntries + int64(Index) * 60;
        KyaWriteU32(Data, P, uint32(Entry.Target.TargetRef));
        KyaWriteU32(Data, P + 4, uint32(Entry.Target.CutsceneId));
        KyaWriteU32(Data, P + 8, uint32(Entry.Target.Flags));
        KyaWriteU32(Data, P + 12, uint32(Entry.Target.MessageId));
        KyaWriteU32(Data, P + 16, uint32(Entry.Target.MessageFlags));
        KyaWriteU32(Data, P + 20, uint32(Entry.Target.Field14));
        KyaWriteU32(Data, P + 24, uint32(Entry.Target.Field18));
        KyaWriteU32(Data, P + 28, uint32(Entry.Camera.CameraIndex));
        KyaWriteU32(Data, P + 32, uint32(Entry.Camera.Flags));
        KyaWriteF32(Data, P + 36, Entry.Camera.Field8);
        KyaWriteF32(Data, P + 40, Entry.Camera.FieldC);
        KyaWriteF32(Data, P + 44, Entry.Camera.Field10);
        KyaWriteU32(Data, P + 48, uint32(Entry.Camera.ActorRef));
        KyaWriteU32(Data, P + 52, uint32(Entry.Camera.ZoneRef));
        KyaWriteF32(Data, P + 56, Entry.Camera.Field1C);
    }
    return true;
}

bool KyaReadTrapBehaviour(FKyaActorBehaviour& Out, int32 Id, TConstArrayView<uint8> Data,
    int64 Start, int32 Size)
{
    if ((Id != 2 && Id != 3) || Size != 12 || !KyaRangeValid(Data, Start, int64(Size)))
        return false;
    const uint8* Bytes = Data.GetData();
    Out.TrapBehaviourField0 = int32(KyaReadU32(Bytes + Start));
    Out.TrapBehaviourField4 = int32(KyaReadU32(Bytes + Start + 4));
    Out.TrapBehaviourField8 = int32(KyaReadU32(Bytes + Start + 8));
    Out.bHasTypedTrapBehaviour = true;
    return true;
}

bool KyaWriteTrapBehaviour(const FKyaActorBehaviour& Behaviour, TArray<uint8>& Data, int64 Start,
    int32 Size)
{
    if ((Behaviour.Id != 2 && Behaviour.Id != 3) || Size != 12 ||
        !KyaRangeValid(Data, Start, int64(Size))) return false;
    KyaWriteU32(Data, Start, uint32(Behaviour.TrapBehaviourField0));
    KyaWriteU32(Data, Start + 4, uint32(Behaviour.TrapBehaviourField4));
    KyaWriteU32(Data, Start + 8, uint32(Behaviour.TrapBehaviourField8));
    return true;
}
