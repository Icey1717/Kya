#pragma once

#include "CoreMinimal.h"

/**
 * Shared CActorFighter serialized prefix: autonomous block, 156 bytes of fixed
 * fighter tuning, blow/grab/react/combo databases and the weapon actor index.
 * Wolfen and Kim both start with this block.
 */
inline bool KyaFighterSkipString(TConstArrayView<uint8> Data, int64& Pos, int64 End)
{
    if (Pos < 0 || Pos >= End) return false;
    while (Pos < End && Data[Pos] != 0) ++Pos;
    if (Pos >= End) return false;
    ++Pos;
    Pos = (Pos + 3) & ~int64(3);
    return Pos <= End;
}

/** Backwards-compatible name used by the factory and fighter tests. */
inline bool KyaSkipString(TConstArrayView<uint8> Data, int64& Pos, int64 End)
{
    return KyaFighterSkipString(Data, Pos, End);
}

struct KYALEVEL_API FKyaFighterBlowOffsets
{
    int64 Name = 0, Field4 = 0, Field8 = 0, Field20 = 0, Field30 = 0, Field40 = 0;
    int64 SubObjs = 0, BoneRefs = 0, CanActivateRange = 0, Stages = 0, Tail = 0;
    int32 NameCapacity = 0, SubObjCount = 0;
};

inline bool KyaReadFighterBlowOffsets(TConstArrayView<uint8> Data, int64& Pos, int64 End,
    FKyaFighterBlowOffsets& Out)
{
    Out = FKyaFighterBlowOffsets{};
    const int64 NameStart = Pos;
    if (!KyaFighterSkipString(Data, Pos, End)) return false;
    Out.Name = NameStart;
    Out.NameCapacity = int32(Pos - NameStart);
    if (End - Pos < 64) return false;
    auto U32At = [&Data](int64 Offset) {
        const uint8* P = Data.GetData() + Offset;
        return uint32(P[0]) | uint32(P[1]) << 8 | uint32(P[2]) << 16 | uint32(P[3]) << 24;
    };
    Out.Field4 = Pos;
    Out.Field8 = Pos + 12;
    Out.Field20 = Pos + 32;
    Out.Field30 = Pos + 44;
    Out.Field40 = Pos + 56;
    const uint32 SubObjs = U32At(Pos + 60);
    Out.SubObjCount = int32(SubObjs);
    Out.SubObjs = Pos + 64;
    Pos += 64;
    if (SubObjs > uint64(End - Pos) / 28) return false;
    Pos += int64(SubObjs) * 28;
    if (End - Pos < 96) return false;
    Out.BoneRefs = Pos; Pos += 8;
    Out.CanActivateRange = Pos; Pos += 4;
    Out.Stages = Pos; Pos += 84;
    Out.Tail = Pos;
    return true;
}

inline bool KyaSkipFighterBlow(TConstArrayView<uint8> Data, int64& Pos, int64 End)
{
    FKyaFighterBlowOffsets Blow;
    if (!KyaReadFighterBlowOffsets(Data, Pos, End, Blow)) return false;
    Pos = Blow.Tail;
    return true;
}

struct KYALEVEL_API FKyaFighterGrabOffsets
{
    int64 Name = 0, Field4 = 0, Field8 = 0, Field14 = 0, Field20 = 0, Field30 = 0, Field34 = 0;
    int64 Field38 = 0, Field3C = 0, Field60 = 0, Field40 = 0, Field44 = 0, Field48 = 0, Field50 = 0;
    int64 Field68 = 0, Stages = 0, Tail = 0;
    int32 NameCapacity = 0;
};

inline bool KyaReadFighterGrabOffsets(TConstArrayView<uint8> Data, int64& Pos, int64 End,
    FKyaFighterGrabOffsets& Out)
{
    Out = FKyaFighterGrabOffsets{};
    const int64 NameStart = Pos;
    if (!KyaFighterSkipString(Data, Pos, End)) return false;
    Out.Name = NameStart;
    Out.NameCapacity = int32(Pos - NameStart);
    if (End - Pos < 232) return false;
    Out.Field4 = Pos; Out.Field8 = Pos + 12; Out.Field14 = Pos + 24; Out.Field20 = Pos + 28;
    Out.Field30 = Pos + 40; Out.Field34 = Pos + 44; Out.Field38 = Pos + 48; Out.Field3C = Pos + 52;
    Out.Field60 = Pos + 56; Out.Field40 = Pos + 60; Out.Field44 = Pos + 64; Out.Field48 = Pos + 68;
    Out.Field50 = Pos + 72; Out.Field68 = Pos + 84; Out.Stages = Pos + 92;
    Pos += 232;
    Out.Tail = Pos;
    return true;
}

inline bool KyaSkipFighterGrab(TConstArrayView<uint8> Data, int64& Pos, int64 End)
{
    FKyaFighterGrabOffsets Grab;
    if (!KyaReadFighterGrabOffsets(Data, Pos, End, Grab)) return false;
    Pos = Grab.Tail;
    return true;
}

struct KYALEVEL_API FKyaFighterComboOffsets
{
    int64 Name = 0, Field4 = 0, Field8 = 0, Inputs = 0, Field2 = 0, Field3 = 0, FieldC = 0;
    int64 Action = 0, BranchCount = 0, Tail = 0;
    int32 NameCapacity = 0, InputCount = 0, ActionCapacity = 0;
    TArray<int64> BranchOffsets;
    TArray<int32> BranchCapacities;
};

inline bool KyaReadFighterComboOffsets(TConstArrayView<uint8> Data, int64& Pos, int64 End,
    FKyaFighterComboOffsets& Out)
{
    Out = FKyaFighterComboOffsets{};
    const int64 NameStart = Pos;
    if (!KyaFighterSkipString(Data, Pos, End)) return false;
    Out.Name = NameStart;
    Out.NameCapacity = int32(Pos - NameStart);
    if (End - Pos < 16) return false;
    auto U32At = [&Data](int64 Offset) {
        const uint8* P = Data.GetData() + Offset;
        return uint32(P[0]) | uint32(P[1]) << 8 | uint32(P[2]) << 16 | uint32(P[3]) << 24;
    };
    Out.Field4 = Pos; Out.Field8 = Pos + 8;
    Out.InputCount = int32(U32At(Pos + 12)); Out.Inputs = Pos + 16;
    Pos += 16;
    if (uint64(Out.InputCount) > uint64(End - Pos) / 4) return false;
    Pos += int64(Out.InputCount) * 4;
    if (End - Pos < 12) return false;
    Out.Field2 = Pos; Out.Field3 = Pos + 4; Out.FieldC = Pos + 8; Pos += 12;
    const int64 ActionStart = Pos;
    if (!KyaFighterSkipString(Data, Pos, End)) return false;
    Out.Action = ActionStart;
    Out.ActionCapacity = int32(Pos - ActionStart);
    if (End - Pos < 4) return false;
    Out.BranchCount = Pos;
    const uint32 Branches = U32At(Pos);
    Pos += 4;
    for (uint32 Index = 0; Index < Branches; ++Index)
    {
        const int64 BranchStart = Pos;
        if (!KyaFighterSkipString(Data, Pos, End)) return false;
        Out.BranchOffsets.Add(BranchStart);
        Out.BranchCapacities.Add(int32(Pos - BranchStart));
    }
    Out.Tail = Pos;
    return true;
}

inline bool KyaSkipFighterCombo(TConstArrayView<uint8> Data, int64& Pos, int64 End)
{
    FKyaFighterComboOffsets Combo;
    if (!KyaReadFighterComboOffsets(Data, Pos, End, Combo)) return false;
    Pos = Combo.Tail;
    return true;
}

struct KYALEVEL_API FKyaFighterPrefixOffsets
{
    int64 Weight = 0, Autonomous = 0, Fixed = 0, Blows = 0, Grabs = 0, GrabReacts = 0, Combos = 0, WeaponIndex = 0, Tail = 0;
};

inline bool KyaReadFighterPrefixOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size,
    FKyaFighterPrefixOffsets& Out)
{
    Out = FKyaFighterPrefixOffsets{};
    if (Start < 0 || Start > Data.Num() || Size > Data.Num() - Start) return false;
    const int64 End = Start + Size;
    Out.Weight = Start;
    Out.Autonomous = Start + 4;
    Out.Fixed = Start + 20;
    int64 Pos = Start + 20 + 156;
    if (Pos > End) return false;
    auto U32At = [&Data](int64 Offset) {
        const uint8* P = Data.GetData() + Offset;
        return uint32(P[0]) | uint32(P[1]) << 8 | uint32(P[2]) << 16 | uint32(P[3]) << 24;
    };
    Out.Blows = Pos;
    if (End - Pos < 4) return false;
    uint32 Count = U32At(Pos); Pos += 4;
    if (Count > uint64(End - Pos) / 12) return false;
    for (uint32 Index = 0; Index < Count; ++Index)
        if (!KyaSkipFighterBlow(Data, Pos, End)) return false;
    Out.Grabs = Pos;
    if (End - Pos < 4) return false;
    Count = U32At(Pos); Pos += 4;
    if (Count > uint64(End - Pos) / 12) return false;
    for (uint32 Index = 0; Index < Count; ++Index)
        if (!KyaSkipFighterGrab(Data, Pos, End)) return false;
    Out.GrabReacts = Pos;
    if (End - Pos < 4) return false;
    Count = U32At(Pos); Pos += 4;
    for (uint32 Index = 0; Index < Count; ++Index)
    {
        if (!KyaFighterSkipString(Data, Pos, End) || End - Pos < 8) return false;
        Pos += 8;
    }
    Out.Combos = Pos;
    if (End - Pos < 4) return false;
    Count = U32At(Pos); Pos += 4;
    for (uint32 Index = 0; Index < Count; ++Index)
        if (!KyaSkipFighterCombo(Data, Pos, End)) return false;
    if (End - Pos < 4) return false;
    Out.WeaponIndex = Pos;
    Pos += 4;
    Out.Tail = Pos;
    return true;
}

inline bool KyaReadFighterPrefixEnd(TConstArrayView<uint8> Data, int64 Start, int64 Size,
    int64& OutEnd)
{
    OutEnd = 0;
    FKyaFighterPrefixOffsets Offsets;
    if (!KyaReadFighterPrefixOffsets(Data, Start, Size, Offsets)) return false;
    OutEnd = Offsets.Tail;
    return true;
}
