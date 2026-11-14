#pragma once

#include "CoreMinimal.h"
#include "Actors/KyaSwitch.h"

struct FKyaActorPlacement;
struct FKyaActorBehaviour;

/**
 * CActorBrazul (class 0x4F) serialized codec.
 *
 * The CActorWolfen prefix is parsed only to locate its end (bytes untouched).
 * The Brazul block adds two floats, a CPathFollowReader index, three words,
 * a material id, two spark parameters, five CFxHandleExt ids, and two
 * StaticMeshComponentAdvanced records.
 *
 * BuildBehaviour adds id 3 (fighter default), 26 (CBehaviourBrazulA), 27
 * (CBehaviourBrazulB); ids 0/1/2 and the Wolfen ids are inherited.
 */
struct KYALEVEL_API FKyaBrazulOffsets
{
    int64 WolfenEnd = 0, Floats = 0, PathIndex = 0, Field1060 = 0, Reserved = 0;
    int64 MaterialId = 0, SparkParams = 0, FxHandles = 0, FxInit = 0;
    int64 MeshA = 0, MeshB = 0, Tail = 0;
};

KYALEVEL_API bool KyaReadBrazulOffsets(TConstArrayView<uint8> Data, int64 Start,
    int64 Size, FKyaBrazulOffsets& Out);
KYALEVEL_API bool KyaReadBrazulActor(FKyaActorPlacement& Out,
    TConstArrayView<uint8> Data, int64 Start, int64 Size);
KYALEVEL_API bool KyaWriteBrazulActor(const FKyaActorPlacement& Actor,
    TArray<uint8>& Data, int64 Start, int64 Size);

struct KYALEVEL_API FKyaBrazulSubObjOffsets
{
    int64 Header = 0;
    int64 PathIndices = 0;
    int32 PathCount = 0;
};

struct KYALEVEL_API FKyaBrazulAOffsets
{
    int64 Count = 0;
    int32 CountValue = 0;
    TArray<FKyaBrazulSubObjOffsets> SubObjs;
};

struct KYALEVEL_API FKyaBrazulBOffsets
{
    int64 WayPointRef = 0;
    int64 SwitchTargets = 0, SwitchCamera = 0, Tail = 0;
    int32 SwitchCount = 0;
};

KYALEVEL_API bool KyaReadBrazulBehaviour(FKyaActorBehaviour& Out, int32 Id,
    TConstArrayView<uint8> Data, int64 Start, int32 Size);
KYALEVEL_API bool KyaWriteBrazulBehaviour(const FKyaActorBehaviour& Behaviour,
    TArray<uint8>& Data, int64 Start, int32 Size);
