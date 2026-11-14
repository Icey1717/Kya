#pragma once

#include "CoreMinimal.h"

struct FKyaActorPlacement;
struct FKyaActorBehaviour;

/**
 * CActorPattern (class 0x3F, `PATTERN_CON`) serialized codec.
 *
 * Bank actors of this class are named `LINER`/`LINER_L1`, which is why the
 * cross-bank table lists 0x3F as LINER.
 *
 * CActorPattern::Create calls CActorAutonomous::Create, reads seven floats, two
 * s32 fields, a bone id and one CAddOnGenerator. BuildBehaviour adds id 3
 * (CBehaviourPatternAlone) and 4 (CBehaviourPatternMultiple); id 4's Create is
 * empty, id 3 reads three floats plus CPatternPart (six floats, material id,
 * fields 0x1C/0x20, the scene-gated field 0x24 and field 0x28). Ids 0/1/2 are
 * inherited (2 is the shared catch-by-trap behaviour).
 */
struct KYALEVEL_API FKyaPatternOffsets
{
    int64 AutonomousEnd = 0;
    int64 Floats = 0, Fields = 0, BoneId = 0;
    int64 AddOn = 0, SubObjMode = 0, SubObjFloats = 0;
    int64 Tail = 0;
    int32 SubObjModeValue = 0;
};

KYALEVEL_API bool KyaReadPatternOffsets(TConstArrayView<uint8> Data, int64 Start,
    int64 Size, FKyaPatternOffsets& Out);
KYALEVEL_API bool KyaReadPatternActor(FKyaActorPlacement& Out,
    TConstArrayView<uint8> Data, int64 Start, int64 Size);
KYALEVEL_API bool KyaWritePatternActor(const FKyaActorPlacement& Actor,
    TArray<uint8>& Data, int64 Start, int64 Size);

struct KYALEVEL_API FKyaPatternAloneOffsets
{
    int64 Floats = 0, Part = 0;
    bool bHasVersion = false;
};

KYALEVEL_API bool KyaReadPatternBehaviour(FKyaActorBehaviour& Out, int32 Id,
    TConstArrayView<uint8> Data, int64 Start, int32 Size);
KYALEVEL_API bool KyaWritePatternBehaviour(const FKyaActorBehaviour& Behaviour,
    TArray<uint8>& Data, int64 Start, int32 Size);
