#pragma once

#include "CoreMinimal.h"

/**
 * Shared scenario-condition readers.
 *
 * KyaReadConditionedOffsets decodes a conditioned-operation block array (used by
 * map/scenario streams and Companion alerts); KyaReadScenarioConditionOffsets
 * decodes the leading count + 16-byte condition records used by several actor
 * behaviours.
 */
struct KYALEVEL_API FKyaConditionedOffsets
{
    TArray<int64> Operations;
    TArray<int64> Conditions;
};

inline bool KyaReadConditionedOffsets(TConstArrayView<uint8> Data, int64 Start,
    TArray<FKyaConditionedOffsets>& OutBlocks, int64& OutEnd)
{
    OutBlocks.Reset();
    auto Fits = [&](int64 Pos, int64 Size) { return Pos >= 0 && Pos <= Data.Num() && Size <= Data.Num() - Pos; };
    auto U32 = [&](int64 Pos) { const uint8* P = Data.GetData() + Pos; return uint32(P[0]) | uint32(P[1]) << 8 | uint32(P[2]) << 16 | uint32(P[3]) << 24; };
    if (!Fits(Start, 4)) return false;
    const uint32 Count = U32(Start);
    int64 Pos = Start + 4;
    if (Count > uint64(Data.Num() - Pos) / 8) return false;
    for (uint32 Block = 0; Block < Count; ++Block)
    {
        if (!Fits(Pos, 4)) return false;
        const uint32 NumOps = U32(Pos); Pos += 4;
        if (NumOps > uint64(Data.Num() - Pos) / 12) return false;
        FKyaConditionedOffsets Offsets;
        for (uint32 Index = 0; Index < NumOps; ++Index, Pos += 12) Offsets.Operations.Add(Pos);
        if (!Fits(Pos, 4)) return false;
        const uint32 NumConditions = U32(Pos); Pos += 4;
        if (NumConditions > uint64(Data.Num() - Pos) / 16) return false;
        for (uint32 Index = 0; Index < NumConditions; ++Index, Pos += 16) Offsets.Conditions.Add(Pos);
        OutBlocks.Add(MoveTemp(Offsets));
    }
    OutEnd = Pos;
    return true;
}

struct KYALEVEL_API FKyaScenarioConditionOffsets
{
    int64 Count = 0, Entries = 0;
    int32 CountValue = 0;
};

inline bool KyaReadScenarioConditionOffsets(TConstArrayView<uint8> Data, int64& Pos, int64 End,
    FKyaScenarioConditionOffsets& Out)
{
    Out = FKyaScenarioConditionOffsets{};
    auto U32 = [&Data](int64 Offset) { const uint8* P = Data.GetData() + Offset; return uint32(P[0]) | uint32(P[1]) << 8 | uint32(P[2]) << 16 | uint32(P[3]) << 24; };
    if (Pos < 0 || End > Data.Num() || End - Pos < 4) return false;
    Out.Count = Pos;
    const uint32 Count = U32(Pos);
    Out.CountValue = int32(Count);
    Pos += 4;
    if (uint64(Count) > uint64(End - Pos) / 16) return false;
    Out.Entries = Pos;
    Pos += int64(Count) * 16;
    return true;
}
