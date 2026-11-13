#pragma once

#include "CoreMinimal.h"

struct FKyaActorPlacement;
struct FKyaActorBehaviour;

/**
 * CActorShoot (class 0x3E) serialized codec.
 *
 * CActorShoot::Create calls CActorAutonomous::Create, reads two words,
 * texture/mesh ids, four floats, a word, four floats, four trailing words and a
 * CAddOnGenerator. BuildBehaviour maps 3 to CBehaviourShootFire (three words +
 * an S_ACTOR_STREAM_REF fire list) and 4 to CBehaviourShootFireWave (seven
 * floats); ids 0/1/2 are inherited.
 */
struct KYALEVEL_API FKyaShootOffsets
{
    int64 WeightB = 0, Autonomous = 0, Field350 = 0, Field354 = 0, Texture = 0, Mesh = 0;
    int64 Floats3C0 = 0, Field3D0 = 0, Floats3D4 = 0, Fields3E8 = 0;
    int64 AddOn = 0, SubObjMode = 0, SubObjFloats = 0, Tail = 0;
    int32 SubObjModeValue = 0;
};

KYALEVEL_API bool KyaReadShootOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size,
    FKyaShootOffsets& Out);
KYALEVEL_API bool KyaReadShootActor(FKyaActorPlacement& Out, TConstArrayView<uint8> Data,
    int64 Start, int64 Size);
KYALEVEL_API bool KyaWriteShootActor(const FKyaActorPlacement& Actor, TArray<uint8>& Data,
    int64 Start, int64 Size);

struct KYALEVEL_API FKyaShootFireOffsets
{
    int64 Field2B0 = 0, Field8 = 0, Field2B4 = 0, Count = 0, Entries = 0, Tail = 0;
    int32 CountValue = 0;
};

KYALEVEL_API bool KyaReadShootFireOffsets(TConstArrayView<uint8> Data, int64 Start, int64 End,
    FKyaShootFireOffsets& Out);

struct KYALEVEL_API FKyaShootWaveOffsets
{
    int64 Fields = 0, Tail = 0;
};

KYALEVEL_API bool KyaReadShootWaveOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size,
    FKyaShootWaveOffsets& Out);

KYALEVEL_API bool KyaReadShootBehaviour(FKyaActorBehaviour& Out, int32 Id,
    TConstArrayView<uint8> Data, int64 Start, int32 Size);
KYALEVEL_API bool KyaWriteShootBehaviour(const FKyaActorBehaviour& Behaviour,
    TArray<uint8>& Data, int64 Start, int32 Size);
