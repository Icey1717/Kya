#pragma once

#include "CoreMinimal.h"
#include "KyaLevelBank.h"

struct FKyaActorPlacement;
struct FKyaActorBehaviour;

/**
 * CActorWeapon (class 0x20) serialized codec.
 *
 * CActorWeapon::Create calls CActorMovable::Create then reads two floats.
 * BuildBehaviour maps 2 Default (empty), 3 Pistol, 4 Sniper and 6 RPG; ids 0/1
 * are inherited. The behaviour payloads share the burst block, then add their
 * own fields.
 */
struct KYALEVEL_API FKyaWeaponOffsets
{
    int64 WeightB = 0;
    int64 Field1D8 = 0;
    int64 Field1DC = 0;
    int64 Tail = 0;
};

KYALEVEL_API bool KyaReadWeaponOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size,
    FKyaWeaponOffsets& Out);
KYALEVEL_API bool KyaReadWeaponActor(FKyaActorPlacement& Out, TConstArrayView<uint8> Data,
    int64 Start, int64 Size);
KYALEVEL_API bool KyaWriteWeaponActor(const FKyaActorPlacement& Actor, TArray<uint8>& Data,
    int64 Start, int64 Size);

struct KYALEVEL_API FKyaWeaponBehaviourValues
{
    TArray<FKyaWeaponAdvancedEntry> Burst;
    int32 Field40 = 0, Field44 = 0, SoundIndex = -1;
    int32 PistolField2E0 = 0, PistolField2E4 = -1;
    int32 SniperTextureIndex = 0, SniperMeshIndex = 0, SniperMaterialId = -1;
    TArray<int32> RpgActorRefs;
};

KYALEVEL_API bool KyaReadWeaponBehaviour(int32 Id, TConstArrayView<uint8> Data, int64 Start,
    int32 Size, FKyaWeaponBehaviourValues& Out);
KYALEVEL_API bool KyaReadWeaponBehaviourRecord(FKyaActorBehaviour& Out, int32 Id,
    TConstArrayView<uint8> Data, int64 Start, int32 Size);
KYALEVEL_API bool KyaWriteWeaponBehaviour(const FKyaActorBehaviour& Behaviour, TArray<uint8>& Data,
    int64 Start, int32 Size);
