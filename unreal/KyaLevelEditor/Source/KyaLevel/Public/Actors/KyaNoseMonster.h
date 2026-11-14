#pragma once

#include "CoreMinimal.h"
#include "Actors/KyaSwitch.h"

struct FKyaActorPlacement;
struct FKyaActorBehaviour;

/**
 * CActorNoseMonster (class 0x26) serialized codec.
 *
 * CActorNoseMonster::Create calls CActorAutonomous::Create, reads a zone
 * reference, five tuning floats, two scene-version-gated floats (read only when
 * `CScene::field_0x1C >= 2.21`), three more fields, a ZonePathEntryList, field
 * 0x36C, the CFireShot actor stream, eleven fields and one S_NTF_SWITCH_ONOFF.
 *
 * Because the version gate changes the class payload size by 8 bytes, the
 * reader parses both shapes and selects the one that consumes the payload
 * exactly. The writer preserves the detected shape.
 *
 * BuildBehaviour adds ids 2 (sleep), 3 (attack) and 4 (death); all three Create
 * bodies are empty. Ids 0/1 are inherited.
 */
struct KYALEVEL_API FKyaNoseMonsterZonePathOffsets
{
    int64 ZoneRef = 0, PathFollowRef = 0;
};

struct KYALEVEL_API FKyaNoseMonsterOffsets
{
    int64 AutonomousEnd = 0;
    int64 ZoneRef = 0;
    int64 Floats = 0;
    int64 VersionFloats = 0;
    int64 Field364 = 0, Field370 = 0, Field374 = 0;
    int64 ZonePathCount = 0, ZonePathEntries = 0;
    int64 Field36C = 0, FireShotCount = 0, FireShotEntries = 0;
    int64 Field630 = 0, Field380 = 0, Floats690 = 0, Words6A0 = 0;
    int64 SwitchTargets = 0, SwitchCamera = 0;
    int64 Tail = 0;
    int32 ZonePathCountValue = 0, FireShotCountValue = 0, SwitchCountValue = 0;
    bool bHasVersionFloats = false;
};

KYALEVEL_API bool KyaReadNoseMonsterOffsets(TConstArrayView<uint8> Data, int64 Start,
    int64 Size, FKyaNoseMonsterOffsets& Out);
KYALEVEL_API bool KyaReadNoseMonsterActor(FKyaActorPlacement& Out,
    TConstArrayView<uint8> Data, int64 Start, int64 Size);
KYALEVEL_API bool KyaWriteNoseMonsterActor(const FKyaActorPlacement& Actor,
    TArray<uint8>& Data, int64 Start, int64 Size);
