#pragma once

#include "CoreMinimal.h"

struct FKyaActorPlacement;
struct FKyaActorBehaviour;

/**
 * CActorEventGenerator (class 0x3B) serialized codec.
 *
 * CActorEventGenerator::Create calls CActor::Create, reads a u32 global flag and
 * a zone reference, then runs twelve PTMF_Ext manager Create functions. Six of
 * those are `DummyTempFunc` placeholders in the decompiled source, so the
 * manager layouts cannot be ordered reliably. The module types the leading
 * fields and keeps the manager bytes raw.
 *
 * BuildBehaviour only adds id 2 (CBehaviourEventGen): an s32 field, an f32
 * field, then nine manager Create functions whose byte layouts are documented
 * in the audit finding but not parsed here.
 */
struct KYALEVEL_API FKyaEventGeneratorOffsets
{
    int64 Enabled = 0, ZoneRef = 0, Tail = 0;
};

KYALEVEL_API bool KyaReadEventGeneratorOffsets(TConstArrayView<uint8> Data, int64 Start,
    int64 Size, FKyaEventGeneratorOffsets& Out);
KYALEVEL_API bool KyaReadEventGeneratorActor(FKyaActorPlacement& Out,
    TConstArrayView<uint8> Data, int64 Start, int64 Size);
KYALEVEL_API bool KyaWriteEventGeneratorActor(const FKyaActorPlacement& Actor,
    TArray<uint8>& Data, int64 Start, int64 Size);

struct KYALEVEL_API FKyaEventGeneratorBehaviourValues
{
    int32 FieldC = 0;
    float Field10 = 0;
};

KYALEVEL_API bool KyaReadEventGeneratorBehaviour(FKyaActorBehaviour& Out, int32 Id,
    TConstArrayView<uint8> Data, int64 Start, int32 Size);
KYALEVEL_API bool KyaWriteEventGeneratorBehaviour(const FKyaActorBehaviour& Behaviour,
    TArray<uint8>& Data, int64 Start, int32 Size);
