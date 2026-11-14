#include "Actors/KyaMiniGame.h"

#include "KyaBinaryHelpers.h"
#include "KyaLevelBank.h"

namespace
{
bool SkipString(TConstArrayView<uint8> Data, int64& Pos, int64 End, int64& Start, int64& Stop)
{
    Start = Pos;
    while (Pos < End && Data[int32(Pos)] != 0) ++Pos;
    if (Pos >= End) return false;
    ++Pos;
    Pos = (Pos + 3) & ~int64(3);
    if (Pos > End) return false;
    Stop = Pos;
    return true;
}

bool ParseBoomyTargets(TConstArrayView<uint8> Data, int64& Pos, int64 End, uint32 Count,
    TArray<int64>& Headers)
{
    for (uint32 Index = 0; Index < Count; ++Index)
    {
        if (End - Pos < 8) return false;
        const int64 Header = Pos;
        Headers.Add(Header);
        Pos += 4;
        const uint32 Targets = KyaReadU32(Data.GetData() + Pos);
        Pos += 4;
        if (uint64(Targets) > uint64(End - Pos) / 28) return false;
        Pos += int64(Targets) * 28;
        if (End - Pos < 32) return false;
        Pos += 32;
    }
    return true;
}

bool ParseMiniGame(TConstArrayView<uint8> Data, int64 Start, int64 Size, int64 ClassId,
    bool bHasDefaultScore, FKyaMiniGameOffsets& Out)
{
    Out = FKyaMiniGameOffsets{};
    if (!KyaRangeValid(Data, Start, Size)) return false;
    const uint8* Bytes = Data.GetData();
    const int64 End = Start + Size;
    const auto U32 = [Bytes](int64 Offset) { return KyaReadU32(Bytes + Offset); };
    int64 Pos = Start;

    if (End - Pos < 16) return false;
    Out.Field160 = Pos;
    Out.Field168 = Pos + 8;
    Out.Field16C = Pos + 12;
    Pos += 16;

    Out.ActorCount = Pos;
    const uint32 Actors = U32(Pos);
    Out.ActorCountValue = int32(Actors);
    Pos += 4;
    if (uint64(Actors) > uint64(End - Pos) / 4) return false;
    Out.ActorEntries = Pos;
    Pos += int64(Actors) * 4;

    if (End - Pos < 4) return false;
    Out.FloatCount = Pos;
    const uint32 Floats = U32(Pos);
    Out.FloatCountValue = int32(Floats);
    Pos += 4;
    if (uint64(Floats) > uint64(End - Pos) / 4) return false;
    Out.Floats = Pos;
    Pos += int64(Floats) * 4;

    if (End - Pos < 4) return false;
    Out.ScoreCount = Pos;
    const uint32 Scores = U32(Pos);
    Out.ScoreCountValue = int32(Scores);
    Pos += 4;
    for (uint32 Index = 0; Index < Scores; ++Index)
    {
        if (End - Pos < 4) return false;
        Out.ScoreValues.Add(Pos);
        Pos += 4;
        int64 NameStart = 0, NameEnd = 0;
        if (!SkipString(Data, Pos, End, NameStart, NameEnd)) return false;
        Out.ScoreNameStart.Add(NameStart);
        Out.ScoreNameEnd.Add(NameEnd);
    }

    Out.DefaultScore = -1;
    if (bHasDefaultScore)
    {
        if (End - Pos < 4) return false;
        Out.DefaultScore = Pos;
        Pos += 4;
        int64 NameStart = 0, NameEnd = 0;
        if (!SkipString(Data, Pos, End, NameStart, NameEnd)) return false;
        Out.DefaultNameStart = NameStart;
        Out.DefaultNameEnd = NameEnd;
    }

    if (End - Pos < 16) return false;
    Out.WayPointRef = Pos;
    Out.Field190 = Pos + 4;
    Out.Field1A8 = Pos + 8;
    Pos += 16;

    const uint32 Targets = U32(Pos);
    Out.SwitchCountValue = int32(Targets);
    Out.SwitchTargets = Pos + 4;
    Pos += 4;
    if (uint64(Targets) > uint64(End - Pos) / 28) return false;
    Out.SwitchCamera = Pos + int64(Targets) * 28;
    if (End - Out.SwitchCamera < 32) return false;
    Pos = Out.SwitchCamera + 32;

    Out.VariantStart = Pos;
    if (ClassId == 0x45)
    {
        if (End - Pos < 4) return false;
        Out.TimeAttackField1E0 = Pos;
        Pos += 4;
    }
    else if (ClassId == 0x46)
    {
        if (End - Pos < 4) return false;
        Out.TimeLimit = Pos;
        Pos += 4;
    }
    else if (ClassId == 0x55)
    {
        if (End - Pos < 8) return false;
        Out.PathA = Pos;
        Out.PathB = Pos + 4;
        Pos += 8;
    }
    else if (ClassId == 0x47)
    {
        if (End - Pos < 4) return false;
        Out.TargetCount = Pos;
        const uint32 Count = U32(Pos);
        Out.TargetCountValue = int32(Count);
        Pos += 4;
        Out.Targeting = Pos;
        if (!ParseBoomyTargets(Data, Pos, End, Count, Out.TargetHeaders)) return false;

        if (End - Pos < 4) return false;
        Out.PenaltyCount = Pos;
        const uint32 Penalties = U32(Pos);
        Out.PenaltyCountValue = int32(Penalties);
        Pos += 4;
        Out.Penalties = Pos;
        if (!ParseBoomyTargets(Data, Pos, End, Penalties, Out.PenaltyHeaders)) return false;

        if (End - Pos < 12) return false;
        Out.TimeLimit = Pos;
        Out.MaxPenalties = Pos + 4;
        Out.GameFlags = Pos + 8;
        Pos += 12;

        if (End - Pos < 4) return false;
        Out.SequenceCount = Pos;
        const uint32 Sequences = U32(Pos);
        Out.SequenceCountValue = int32(Sequences);
        Pos += 4;
        for (uint32 Index = 0; Index < Sequences; ++Index)
        {
            if (End - Pos < 4) return false;
            Out.SequenceHeaders.Add(Pos);
            const uint32 Values = U32(Pos);
            Out.SequenceHeaderCounts.Add(int32(Values));
            Pos += 4;
            if (uint64(Values) > uint64(End - Pos) / 4) return false;
            Out.SequenceValues.Add(Pos);
            Pos += int64(Values) * 4;
        }
    }
    Out.Tail = Pos;
    return true;
}

void ReadScoreName(TConstArrayView<uint8> Data, int64 Start, int64 End, FString& Out)
{
    Out.Reset();
    for (int64 Pos = Start; Pos < End && Data[int32(Pos)] != 0; ++Pos)
        Out.AppendChar(TCHAR(Data[int32(Pos)]));
}

bool WriteScoreName(TArray<uint8>& Data, int64 Start, int64 End, const FString& Name)
{
    const int64 Span = End - Start;
    if (Name.Len() + 1 > Span) return false;
    for (int32 Char = 0; Char < Name.Len(); ++Char)
        Data[int32(Start) + Char] = uint8(Name[Char]);
    for (int64 Pad = Name.Len(); Pad < Span; ++Pad)
        Data[int32(Start) + int32(Pad)] = 0;
    return true;
}
}

bool KyaReadMiniGameOffsets(int64 ClassId, TConstArrayView<uint8> Data, int64 Start,
    int64 Size, FKyaMiniGameOffsets& Out)
{
    Out = FKyaMiniGameOffsets{};
    FKyaMiniGameOffsets WithDefault;
    FKyaMiniGameOffsets WithoutDefault;
    const bool bWith = ParseMiniGame(Data, Start, Size, ClassId, true, WithDefault);
    const bool bWithout = ParseMiniGame(Data, Start, Size, ClassId, false, WithoutDefault);
    if (bWith && WithDefault.Tail == Start + Size) { Out = WithDefault; return true; }
    if (bWithout && WithoutDefault.Tail == Start + Size) { Out = WithoutDefault; return true; }
    if (bWith) { Out = WithDefault; return true; }
    if (bWithout) { Out = WithoutDefault; return true; }
    return false;
}

bool KyaReadMiniGameActor(FKyaActorPlacement& Out, TConstArrayView<uint8> Data,
    int64 Start, int64 Size)
{
    const int64 ClassId = Out.ClassId;
    FKyaMiniGameOffsets Off;
    if (!KyaReadMiniGameOffsets(ClassId, Data, Start, Size, Off)) return false;
    const uint8* Bytes = Data.GetData();

    Out.MiniGameField160 = int64(uint64(KyaReadU32(Bytes + Off.Field160)) |
        (uint64(KyaReadU32(Bytes + Off.Field160 + 4)) << 32));
    Out.MiniGameField168 = int32(KyaReadU32(Bytes + Off.Field168));
    Out.MiniGameField16C = int32(KyaReadU32(Bytes + Off.Field16C));

    Out.MiniGameActorRefs.Reset();
    for (int32 Index = 0; Index < Off.ActorCountValue; ++Index)
        Out.MiniGameActorRefs.Add(int32(KyaReadU32(Bytes + Off.ActorEntries + int64(Index) * 4)));
    Out.MiniGameFloats.Reset();
    for (int32 Index = 0; Index < Off.FloatCountValue; ++Index)
        Out.MiniGameFloats.Add(KyaReadF32(Bytes + Off.Floats + int64(Index) * 4));

    Out.MiniGameScores.Reset();
    for (int32 Index = 0; Index < Off.ScoreCountValue; ++Index)
    {
        FKyaMiniGameScore& Score = Out.MiniGameScores.AddDefaulted_GetRef();
        Score.Score = KyaReadF32(Bytes + Off.ScoreValues[Index]);
        ReadScoreName(Data, Off.ScoreNameStart[Index], Off.ScoreNameEnd[Index], Score.Name);
    }
    Out.bMiniGameHasDefaultScore = Off.DefaultScore >= 0;
    if (Off.DefaultScore >= 0)
    {
        Out.MiniGameDefaultScore.Score = KyaReadF32(Bytes + Off.DefaultScore);
        ReadScoreName(Data, Off.DefaultNameStart, Off.DefaultNameEnd, Out.MiniGameDefaultScore.Name);
    }

    Out.MiniGameWayPointRef = int32(KyaReadU32(Bytes + Off.WayPointRef));
    Out.MiniGameField190 = int32(KyaReadU32(Bytes + Off.Field190));
    Out.MiniGameField1A8 = int64(uint64(KyaReadU32(Bytes + Off.Field1A8)) |
        (uint64(KyaReadU32(Bytes + Off.Field1A8 + 4)) << 32));
    Out.MiniGameSwitch = FKyaSwitch{};
    if (!KyaReadSwitchValue(Data, Off.SwitchTargets, Off.SwitchCountValue, Off.SwitchCamera,
        Out.MiniGameSwitch)) return false;

    if (ClassId == 0x45)
    {
        Out.MiniGameTimeAttackField1E0 = int32(KyaReadU32(Bytes + Off.TimeAttackField1E0));
    }
    else if (ClassId == 0x46)
    {
        Out.MiniGameTimeLimit = KyaReadF32(Bytes + Off.TimeLimit);
    }
    else if (ClassId == 0x55)
    {
        Out.MiniGamePathA = int32(KyaReadU32(Bytes + Off.PathA));
        Out.MiniGamePathB = int32(KyaReadU32(Bytes + Off.PathB));
    }
    else if (ClassId == 0x47)
    {
        Out.MiniGameTimeLimit = KyaReadF32(Bytes + Off.TimeLimit);
        Out.MiniGameMaxPenalties = int32(KyaReadU32(Bytes + Off.MaxPenalties));
        Out.MiniGameGameFlags = int32(KyaReadU32(Bytes + Off.GameFlags));
        Out.MiniGameBoomyTargets.Reset();
        for (const int64 Header : Off.TargetHeaders)
        {
            FKyaMiniGameBoomyTarget& Target = Out.MiniGameBoomyTargets.AddDefaulted_GetRef();
            Target.ActorRef = int32(KyaReadU32(Bytes + Header));
            const int64 CountOffset = Header + 4;
            const int32 Count = int32(KyaReadU32(Bytes + CountOffset));
            if (!KyaReadSwitchValue(Data, CountOffset + 4, Count,
                CountOffset + 4 + int64(Count) * 28, Target.Switch)) return false;
        }
        Out.MiniGameBoomyPenaltyTargets.Reset();
        for (const int64 Header : Off.PenaltyHeaders)
        {
            FKyaMiniGameBoomyTarget& Target = Out.MiniGameBoomyPenaltyTargets.AddDefaulted_GetRef();
            Target.ActorRef = int32(KyaReadU32(Bytes + Header));
            const int64 CountOffset = Header + 4;
            const int32 Count = int32(KyaReadU32(Bytes + CountOffset));
            if (!KyaReadSwitchValue(Data, CountOffset + 4, Count,
                CountOffset + 4 + int64(Count) * 28, Target.Switch)) return false;
        }
        Out.MiniGameSequences.Reset();
        for (int32 Index = 0; Index < Off.SequenceCountValue; ++Index)
        {
            FKyaMiniGameSequence& Sequence = Out.MiniGameSequences.AddDefaulted_GetRef();
            for (int32 Value = 0; Value < Off.SequenceHeaderCounts[Index]; ++Value)
                Sequence.TargetIndices.Add(int32(KyaReadU32(Bytes + Off.SequenceValues[Index] + int64(Value) * 4)));
        }
    }

    Out.bHasMiniGameData = true;
    Out.ClassData.Reset();
    if (Start + Size > Off.Tail)
        Out.ClassData.Append(Bytes + Off.Tail, int32(Start + Size - Off.Tail));
    return true;
}

bool KyaWriteMiniGameActor(const FKyaActorPlacement& Actor, TArray<uint8>& Data,
    int64 Start, int64 Size)
{
    const int64 ClassId = Actor.ClassId;
    FKyaMiniGameOffsets Off;
    if (!KyaReadMiniGameOffsets(ClassId, MakeArrayView(Data.GetData(), Data.Num()), Start, Size, Off))
        return false;
    if (Actor.MiniGameActorRefs.Num() != Off.ActorCountValue) return false;
    if (Actor.MiniGameFloats.Num() != Off.FloatCountValue) return false;
    if (Actor.MiniGameScores.Num() != Off.ScoreCountValue) return false;
    if (Actor.bMiniGameHasDefaultScore != (Off.DefaultScore >= 0)) return false;
    if (Actor.MiniGameSwitch.Targets.Num() != Off.SwitchCountValue) return false;

    KyaWriteU32(Data, Off.Field160, uint32(uint64(Actor.MiniGameField160)));
    KyaWriteU32(Data, Off.Field160 + 4, uint32(uint64(Actor.MiniGameField160) >> 32));
    KyaWriteU32(Data, Off.Field168, uint32(Actor.MiniGameField168));
    KyaWriteU32(Data, Off.Field16C, uint32(Actor.MiniGameField16C));
    for (int32 Index = 0; Index < Actor.MiniGameActorRefs.Num(); ++Index)
        KyaWriteU32(Data, Off.ActorEntries + int64(Index) * 4, uint32(Actor.MiniGameActorRefs[Index]));
    for (int32 Index = 0; Index < Actor.MiniGameFloats.Num(); ++Index)
        KyaWriteF32(Data, Off.Floats + int64(Index) * 4, Actor.MiniGameFloats[Index]);
    for (int32 Index = 0; Index < Actor.MiniGameScores.Num(); ++Index)
    {
        KyaWriteF32(Data, Off.ScoreValues[Index], Actor.MiniGameScores[Index].Score);
        if (!WriteScoreName(Data, Off.ScoreNameStart[Index], Off.ScoreNameEnd[Index],
            Actor.MiniGameScores[Index].Name)) return false;
    }
    if (Off.DefaultScore >= 0)
    {
        KyaWriteF32(Data, Off.DefaultScore, Actor.MiniGameDefaultScore.Score);
        if (!WriteScoreName(Data, Off.DefaultNameStart, Off.DefaultNameEnd,
            Actor.MiniGameDefaultScore.Name)) return false;
    }
    KyaWriteU32(Data, Off.WayPointRef, uint32(Actor.MiniGameWayPointRef));
    KyaWriteU32(Data, Off.Field190, uint32(Actor.MiniGameField190));
    KyaWriteU32(Data, Off.Field1A8, uint32(uint64(Actor.MiniGameField1A8)));
    KyaWriteU32(Data, Off.Field1A8 + 4, uint32(uint64(Actor.MiniGameField1A8) >> 32));
    KyaWriteSwitchValue(Data, Actor.MiniGameSwitch, Off.SwitchTargets, Off.SwitchCamera);

    if (ClassId == 0x45)
    {
        KyaWriteU32(Data, Off.TimeAttackField1E0, uint32(Actor.MiniGameTimeAttackField1E0));
    }
    else if (ClassId == 0x46)
    {
        KyaWriteF32(Data, Off.TimeLimit, Actor.MiniGameTimeLimit);
    }
    else if (ClassId == 0x55)
    {
        KyaWriteU32(Data, Off.PathA, uint32(Actor.MiniGamePathA));
        KyaWriteU32(Data, Off.PathB, uint32(Actor.MiniGamePathB));
    }
    else if (ClassId == 0x47)
    {
        if (Actor.MiniGameBoomyTargets.Num() != Off.TargetCountValue) return false;
        if (Actor.MiniGameBoomyPenaltyTargets.Num() != Off.PenaltyCountValue) return false;
        if (Actor.MiniGameSequences.Num() != Off.SequenceCountValue) return false;
        for (int32 Index = 0; Index < Actor.MiniGameBoomyTargets.Num(); ++Index)
        {
            const FKyaMiniGameBoomyTarget& Target = Actor.MiniGameBoomyTargets[Index];
            const int64 Header = Off.TargetHeaders[Index];
            if (Target.Switch.Targets.Num() != int32(KyaReadU32(Data.GetData() + Header + 4))) return false;
            KyaWriteU32(Data, Header, uint32(Target.ActorRef));
            KyaWriteSwitchValue(Data, Target.Switch, Header + 8,
                Header + 8 + int64(Target.Switch.Targets.Num()) * 28);
        }
        for (int32 Index = 0; Index < Actor.MiniGameBoomyPenaltyTargets.Num(); ++Index)
        {
            const FKyaMiniGameBoomyTarget& Target = Actor.MiniGameBoomyPenaltyTargets[Index];
            const int64 Header = Off.PenaltyHeaders[Index];
            if (Target.Switch.Targets.Num() != int32(KyaReadU32(Data.GetData() + Header + 4))) return false;
            KyaWriteU32(Data, Header, uint32(Target.ActorRef));
            KyaWriteSwitchValue(Data, Target.Switch, Header + 8,
                Header + 8 + int64(Target.Switch.Targets.Num()) * 28);
        }
        KyaWriteF32(Data, Off.TimeLimit, Actor.MiniGameTimeLimit);
        KyaWriteU32(Data, Off.MaxPenalties, uint32(Actor.MiniGameMaxPenalties));
        KyaWriteU32(Data, Off.GameFlags, uint32(Actor.MiniGameGameFlags));
        for (int32 Index = 0; Index < Actor.MiniGameSequences.Num(); ++Index)
        {
            if (Actor.MiniGameSequences[Index].TargetIndices.Num() != Off.SequenceHeaderCounts[Index]) return false;
            for (int32 Value = 0; Value < Actor.MiniGameSequences[Index].TargetIndices.Num(); ++Value)
                KyaWriteU32(Data, Off.SequenceValues[Index] + int64(Value) * 4,
                    uint32(Actor.MiniGameSequences[Index].TargetIndices[Value]));
        }
    }
    return true;
}

bool KyaReadMiniGameBehaviour(FKyaActorBehaviour& Out, int32 Id, TConstArrayView<uint8> Data,
    int64 Start, int32 Size)
{
    if (Id != 3 || Start < 0 || !KyaRangeValid(Data, Start, Size) || Size < 4) return false;
    const uint8* Bytes = Data.GetData();
    const int32 Count = int32(KyaReadU32(Bytes + Start));
    FKyaMiniGameBetOffsets Off;
    Off.Count = Start;
    Off.CountValue = Count;
    Off.Entries = Start + 4;
    if (uint64(uint32(Count)) > uint64(Size - 4) / 8) return false;
    Out.MiniGameBets.Reset();
    for (int32 Index = 0; Index < Count; ++Index)
    {
        FKyaMiniGameBet& Bet = Out.MiniGameBets.AddDefaulted_GetRef();
        Bet.Cost = int32(KyaReadU32(Bytes + Off.Entries + int64(Index) * 8));
        Bet.Reward = int32(KyaReadU32(Bytes + Off.Entries + int64(Index) * 8 + 4));
    }
    Out.bHasTypedMiniGameBetting = true;
    return true;
}

bool KyaWriteMiniGameBehaviour(const FKyaActorBehaviour& Behaviour, TArray<uint8>& Data,
    int64 Start, int32 Size)
{
    if (Behaviour.Id != 3 || Start < 0 || !KyaRangeValid(Data, Start, Size) || Size < 4) return false;
    const int32 Count = int32(KyaReadU32(Data.GetData() + Start));
    if (Behaviour.MiniGameBets.Num() != Count) return false;
    for (int32 Index = 0; Index < Behaviour.MiniGameBets.Num(); ++Index)
    {
        KyaWriteU32(Data, Start + 4 + int64(Index) * 8, uint32(Behaviour.MiniGameBets[Index].Cost));
        KyaWriteU32(Data, Start + 4 + int64(Index) * 8 + 4, uint32(Behaviour.MiniGameBets[Index].Reward));
    }
    return true;
}
