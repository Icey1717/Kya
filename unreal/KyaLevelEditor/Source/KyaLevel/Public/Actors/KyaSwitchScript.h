#pragma once

#include "CoreMinimal.h"

/**
 * Shared S_NTF_SWITCH_SCRIPT/_EX layout used by Wolfen, Companion and other
 * behaviour payloads. Each category stores a discarded id word followed by a
 * count and 8-byte (categories 0 and 3) or 16-byte (1 and 2) records.
 */
struct KYALEVEL_API FKyaSwitchScriptOffsets
{
    int64 Counts[4] = { 0, 0, 0, 0 };
    int32 CountValues[4] = { 0, 0, 0, 0 };
    TArray<int64> RecordOffsets;
    TArray<int32> RecordCategories;
    int64 Tail = 0;
};

inline bool KyaReadSwitchScriptOffsets(TConstArrayView<uint8> Data, int64 Start, int64 End,
    FKyaSwitchScriptOffsets& Out)
{
    Out = FKyaSwitchScriptOffsets{};
    if (Start < 0 || End > Data.Num() || End - Start < 32) return false;
    auto U32 = [&Data](int64 Offset) {
        const uint8* P = Data.GetData() + Offset;
        return uint32(P[0]) | uint32(P[1]) << 8 | uint32(P[2]) << 16 | uint32(P[3]) << 24;
    };
    int64 Pos = Start;
    for (int32 Category = 0; Category < 4; ++Category)
    {
        if (End - Pos < 8) return false;
        Pos += 4; // discarded id
        Out.Counts[Category] = Pos;
        const uint32 Count = U32(Pos);
        Out.CountValues[Category] = int32(Count);
        Pos += 4;
        const int64 EntrySize = (Category == 0 || Category == 3) ? 8 : 16;
        if (uint64(Count) > uint64(End - Pos) / EntrySize) return false;
        for (uint32 Index = 0; Index < Count; ++Index)
        {
            Out.RecordOffsets.Add(Pos);
            Out.RecordCategories.Add(Category);
            Pos += EntrySize;
        }
    }
    Out.Tail = Pos;
    return true;
}
