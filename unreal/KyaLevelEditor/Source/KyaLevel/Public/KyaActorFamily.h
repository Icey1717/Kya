#pragma once

#include "CoreMinimal.h"

/**
 * Shared actor-family types.
 *
 * Each actor family owns one module under Private/Actors. A module supplies its
 * serialized class id, recovered display metadata, behaviour id names and
 * reference fields; byte codecs live beside the family or in an exported codec
 * header. KyaActorFamilies::All() is the single registry the catalog, Details
 * panel and tests read, so adding a family never requires editing the catalog.
 */

/** Namespace of a serialized reference field. */
enum class EKyaReferenceKind : uint8
{
    Actor,
    Waypoint,
    Path,
    Zone,
    Sector,
    Mesh,
    Texture,
    Behaviour
};

/** Recovered name for one class-specific behaviour id. */
struct FKyaActorFamilyBehaviour
{
    int32 Id = 0;
    const TCHAR* Name = nullptr;
};

/** Top-level scalar reference field owned by a family (or shared by every class). */
struct FKyaActorFamilyReference
{
    const TCHAR* Property = nullptr;
    EKyaReferenceKind Kind = EKyaReferenceKind::Actor;
};

/** One recovered actor class family. ClassId -1 is reserved for shared references. */
struct FKyaActorFamily
{
    int64 ClassId = 0;
    /** Recovered source type, for example CActorHedgehog. */
    const TCHAR* SourceType = nullptr;
    const TCHAR* DisplayName = nullptr;
    const TCHAR* Notes = nullptr;
    TConstArrayView<FKyaActorFamilyBehaviour> Behaviours;
    TConstArrayView<FKyaActorFamilyReference> References;
};

namespace KyaActorFamilies
{
/** All registered families in discovery order. */
KYALEVEL_API TConstArrayView<const FKyaActorFamily*> All();
/** Registered family for a serialized class id, or null. */
KYALEVEL_API const FKyaActorFamily* Find(int64 ClassId);
/** Recovered behaviour name for a class/id pair, or null. */
KYALEVEL_API const FKyaActorFamilyBehaviour* FindBehaviour(int64 ClassId, int32 BehaviourId);
/** Source-reconciled owned behaviours whose Create consumes no bytes. */
KYALEVEL_API bool HasEmptyPayload(int64 ClassId, int32 BehaviourId);
/** Actor-specific id-2 mappings verified to reach CActorAutonomous's trap behaviour. */
KYALEVEL_API bool UsesCatchByTrap(int64 ClassId, int32 BehaviourId);
/** Reference fields meaningful on every class (sector, mesh, texture, behaviour). */
KYALEVEL_API TConstArrayView<FKyaActorFamilyReference> SharedReferences();
}
