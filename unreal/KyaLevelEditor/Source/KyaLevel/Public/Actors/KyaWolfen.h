#pragma once

#include "CoreMinimal.h"
#include "Actors/KyaSwitchScript.h"

struct FKyaActorPlacement;
struct FKyaActorBehaviour;

/**
 * CActorWolfen (class 0x09) serialized codec.
 *
 * The class payload starts with the shared CActorFighter prefix and continues
 * with vision/detection/movement fields, fight capabilities and combo lists.
 * BuildBehaviour maps the fighter defaults plus Wolfen ids 8/10/13/14/15/17/18/
 * 19/21/22/23/24. The class reader leaves the fighter prefix population to the
 * factory, which shares it with Kim.
 */
struct KYALEVEL_API FKyaWolfenOffsets
{
    int64 FighterPrefix = 0, FighterFixed = 0, Blows = 0, Grabs = 0, GrabReacts = 0, Combos = 0, WeaponIndex = 0, WolfenStart = 0;
    int64 FieldB74 = 0, Vision = 0, StartSector = 0, Hearing = 0, VisionDetection = 0, FieldCF4 = 0;
    int64 WalkValues = 0, MagicRequired = 0, FieldD30 = 0, FieldB90 = 0, FightParam = 0, Tail = 0;
};

KYALEVEL_API bool KyaReadWolfenOffsets(TConstArrayView<uint8> Data, int64 Start, int64 Size,
    FKyaWolfenOffsets& Out);

struct KYALEVEL_API FKyaWolfenCapabilityItemOffsets
{
    int64 CountOffset = 0, Chains = 0, Floats = 0;
    int32 ChainCount = 0;
};

struct KYALEVEL_API FKyaWolfenCapabilityOffsets
{
    int64 CountOffset = 0, Tail = 0;
    TArray<FKyaWolfenCapabilityItemOffsets> Items;
};

struct KYALEVEL_API FKyaWolfenFightParamOffsets
{
    int64 Capability0Fields = 0, Capability2Fields = 0, Floats = 0;
    FKyaWolfenCapabilityOffsets Capabilities[3];
    int64 ComboList1Count = 0, ComboList1Data = 0, ComboList2Count = 0, ComboList2Data = 0;
    int64 Trailing = 0, Tail = 0;
};

KYALEVEL_API bool KyaReadWolfenFightParamOffsets(TConstArrayView<uint8> Data, int64 Start,
    int64 End, FKyaWolfenFightParamOffsets& Out);

/** Wolfen-specific class fields and fight parameters. The fighter prefix is written separately. */
KYALEVEL_API bool KyaReadWolfenActor(FKyaActorPlacement& Out, TConstArrayView<uint8> Data,
    int64 Start, int64 Size);
KYALEVEL_API bool KyaWriteWolfenActor(const FKyaActorPlacement& Actor, TArray<uint8>& Data,
    int64 Start, int64 Size);

struct KYALEVEL_API FKyaTrackSubObjOffsets
{
    int64 Field0 = 0, AField0 = 0, AField4 = 0, AField8 = 0, Field10 = 0;
};

struct KYALEVEL_API FKyaWolfenBehaviourOffsets
{
    int64 Flags = 0, TrackField = 0, DcaActorRef = 0, DcaShootCooldown = 0, Avoid80 = 0, Avoid84 = 0;
    int64 WeaponField90 = 0, WeaponField94 = 0, WeaponField98 = 0, WeaponCount = 0, WeaponEntries = 0;
    int64 WeaponFieldF0 = 0, WeaponTargetField = 0;
    int32 WeaponCountValue = 0;
    TArray<FKyaTrackSubObjOffsets> SubObjs;
    FKyaSwitchScriptOffsets Switch;
    int64 Field80 = 0, TrackBehaviourId = 0, OnOffCount = 0, OnOffEntries = 0, OnOffCamera = 0;
    int32 OnOffCountValue = 0;
    int64 SnipeField90 = 0, SnipeField94 = 0, SpotTrack = 0, SnipeTrack = 0, ShadowMaterial = 0, SnipeFieldAC = 0;
    int64 SnipeFieldB0 = 0, SnipeFieldB4 = 0, NbSpot = 0, NbShadow = 0, RotationSpeed = 0, SnipeFieldB8 = 0;
    int64 GuardPath = 0, GuardField98 = 0, GuardTrack = 0, GuardField90 = 0;
    int64 ExorcismMaterial = 0, ExorcismField24 = 0, ExorcismMeshes = 0, ExorcismField18 = 0, ExorcismWords = 0;
    int64 EscapeCount = 0, EscapeEntries = 0, EscapeField88 = 0, EscapeFloats = 0, EscapeFieldA0 = 0;
    int32 EscapeCountValue = 0;
    int64 Tail = 0;
    bool bTrack = false, bDca = false, bAvoid = false, bWeaponStand = false, bWeapon = false, bWeaponSnipe = false, bLost = false, bWatchDog = false, bSnipe = false, bGuard = false, bEscape = false, bExorcism = false;
};

KYALEVEL_API bool KyaReadWolfenBehaviourOffsets(int32 Id, TConstArrayView<uint8> Data,
    int64 Start, int64 Size, FKyaWolfenBehaviourOffsets& Out);
KYALEVEL_API bool KyaReadWolfenBehaviour(FKyaActorBehaviour& Out, int32 Id,
    TConstArrayView<uint8> Data, int64 Start, int32 Size);
KYALEVEL_API bool KyaWriteWolfenBehaviour(const FKyaActorBehaviour& Behaviour,
    TArray<uint8>& Data, int64 Start, int32 Size);
