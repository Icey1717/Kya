#include "Actors/KyaHeroUnknown.h"

bool KyaReadHeroUnknownOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size,
    FKyaHeroUnknownOffsets& Out)
{
    Out = FKyaHeroUnknownOffsets{};
    if (Start < 0 || Size != 28 || Start > Data.Num() || Size > Data.Num() - Start) return false;
    Out.Values = Start;
    Out.Tail = Start + 28;
    return true;
}
