#pragma once

#include "CoreMinimal.h"

/**
 * Shared hero-unknown behaviour payload (class 0x06, behaviour 9/10/11).
 *
 * Seven serialized words retained as raw values by the editor.
 */
struct KYALEVEL_API FKyaHeroUnknownOffsets
{
    int64 Values = 0, Tail = 0;
};

KYALEVEL_API bool KyaReadHeroUnknownOffsets(TConstArrayView<uint8> Data, int64 Start,
    int64 Size, FKyaHeroUnknownOffsets& Out);
