#include "Actors/KyaCatchByTrap.h"

bool KyaReadCatchByTrapOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size,
    FKyaCatchByTrapOffsets& Out)
{
    Out = FKyaCatchByTrapOffsets{};
    if (Start < 0 || Size < 4 || Start > Data.Num() || Size > Data.Num() - Start) return false;
    int64 Pos = Start;
    if (!KyaReadConditionedOffsets(Data, Start, Out.Blocks, Pos)) return false;
    if (Pos != Start + Size) return false;
    Out.Tail = Pos;
    return true;
}
