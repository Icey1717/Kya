#pragma once

#include "CoreMinimal.h"

class UKyaLevelBank;
struct FKyaActorPlacement;

/** Recovered class information for a bank actor placement. */
struct KYALEVEL_API FKyaActorClassInfo
{
    int64 ClassId = 0;
    FString Name;
    FString Notes;
    bool bKnown = false;
};

KYALEVEL_API FKyaActorClassInfo GetKyaActorClassInfo(int64 ClassId);
/** "Moving platform (13)" or "Unknown class 99". */
KYALEVEL_API FString GetKyaActorClassLabel(int64 ClassId);

/** Recovered name for a class-specific behaviour id. */
struct KYALEVEL_API FKyaBehaviourInfo
{
    int32 Id = 0;
    FString Name;
    bool bKnown = false;
};

KYALEVEL_API FKyaBehaviourInfo GetKyaBehaviourInfo(int64 ClassId, int32 BehaviourId);
/** "Bridge wind (3)" or "Behaviour 12 (unknown)". */
KYALEVEL_API FString GetKyaBehaviourLabel(int64 ClassId, int32 BehaviourId);

/** Resolution of a bank-local model mesh or texture bank reference. */
struct KYALEVEL_API FKyaBankReference
{
    int32 Ordinal = -1;
    int32 EntryCount = 0;
    bool bResolved = false;
    FString Label;
    FString Warning;
};

KYALEVEL_API TArray<int32> GetKyaModelOrdinals(const UKyaLevelBank& Bank);
KYALEVEL_API TArray<int32> GetKyaTextureBankOrdinals(const UKyaLevelBank& Bank);
KYALEVEL_API FKyaBankReference ResolveKyaModelReference(const UKyaLevelBank& Bank, int32 MeshIndex);
KYALEVEL_API FKyaBankReference ResolveKyaTextureReference(const UKyaLevelBank& Bank, int32 TextureIndex);
/** "Sector 7" / "Global (no sector)" resolution against the loaded sector IDs. */
KYALEVEL_API FString GetKyaSectorLabel(const TMap<int32, FString>& LoadedSectors, int32 SectorId);

/** All behaviour records of one actor with recovered labels, for pickers. */
struct KYALEVEL_API FKyaBehaviourOption
{
    int32 Id = 0;
    FString Label;
};
KYALEVEL_API TArray<FKyaBehaviourOption> GetKyaBehaviourOptions(const FKyaActorPlacement& Placement);

/** Reference problems found in one placement; blocking issues reject an edit. */
struct KYALEVEL_API FKyaActorReferenceIssue
{
    FString Field;
    FString Message;
    bool bBlocking = false;
};

KYALEVEL_API TArray<FKyaActorReferenceIssue> ValidateKyaActorReferences(const UKyaLevelBank& Bank,
    const FKyaActorPlacement& Placement);

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

/** True when the top-level placement field is a known reference. */
KYALEVEL_API bool FindKyaReferenceField(FName PropertyName, EKyaReferenceKind& OutKind);
KYALEVEL_API FText DescribeKyaReferenceKind(EKyaReferenceKind Kind);
/** Set a scalar numeric placement field by name (int32 or int64). */
KYALEVEL_API bool SetKyaNumericField(FKyaActorPlacement& Placement, FName PropertyName, int64 Value);
/** Read a scalar numeric placement field by name (int32 or int64). */
KYALEVEL_API int64 GetKyaNumericField(const FKyaActorPlacement& Placement, FName PropertyName);

struct KYALEVEL_API FKyaReferenceField
{
    FName PropertyName;
    EKyaReferenceKind Kind;
    /** Class the field belongs to, or -1 for fields meaningful on every class. */
    int64 ClassId = -1;
};
/** Top-level scalar reference fields, for building pickers and validation. */
KYALEVEL_API TArray<FKyaReferenceField> GetKyaReferenceFields();
