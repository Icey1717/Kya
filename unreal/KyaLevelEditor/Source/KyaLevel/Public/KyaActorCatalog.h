#pragma once

#include "CoreMinimal.h"

class UKyaLevelBank;
struct FKyaActorPlacement;
struct FKyaActorEditPatch;

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
/** Read/write an FVector placement field by name. */
KYALEVEL_API bool GetKyaVectorField(const FKyaActorPlacement& Placement, FName PropertyName, FVector& Out);
KYALEVEL_API bool SetKyaVectorField(FKyaActorPlacement& Placement, FName PropertyName, const FVector& Value);
/** Read/write a float placement field by name. */
KYALEVEL_API bool GetKyaFloatField(const FKyaActorPlacement& Placement, FName PropertyName, float& Out);
KYALEVEL_API bool SetKyaFloatField(FKyaActorPlacement& Placement, FName PropertyName, float Value);

struct KYALEVEL_API FKyaReferenceField
{
    FName PropertyName;
    EKyaReferenceKind Kind;
    /** Class the field belongs to, or -1 for fields meaningful on every class. */
    int64 ClassId = -1;
};
/** Top-level scalar reference fields, for building pickers and validation. */
KYALEVEL_API TArray<FKyaReferenceField> GetKyaReferenceFields();

// --- Imported baseline -----------------------------------------------------------

/** True when the bank has typed imported values captured. */
KYALEVEL_API bool HasKyaImportedBaseline(const UKyaLevelBank& Bank);
/** Snapshot the current typed actor/waypoint/path records as the imported baseline. */
KYALEVEL_API void CaptureKyaImportedBaseline(UKyaLevelBank& Bank);
/** Imported actor placement, or false when no baseline exists or the index is invalid. */
KYALEVEL_API bool GetKyaImportedActor(const UKyaLevelBank& Bank, int32 Index, FKyaActorPlacement& Out);
/** True when the current actor record differs from the imported baseline. */
KYALEVEL_API bool KyaActorDiffersFromImported(const UKyaLevelBank& Bank, int32 Index);
/** Patch that restores one actor record (including nested records) to its imported value. */
KYALEVEL_API bool BuildKyaActorResetPatch(const UKyaLevelBank& Bank, int32 Index, FKyaActorEditPatch& OutPatch);
/** Patch that restores one top-level actor property to its imported value. */
KYALEVEL_API bool BuildKyaActorFieldResetPatch(const UKyaLevelBank& Bank, int32 Index, FName PropertyName,
    FKyaActorEditPatch& OutPatch);
/**
 * True when a property can be copied from Source to Dest without changing the
 * destination's serialized layout: same class for class-specific fields, matching
 * presence flags, array counts, nested record sizes and string-slot capacity.
 */
KYALEVEL_API bool CanCopyKyaActorField(const FKyaActorPlacement& Source, const FKyaActorPlacement& Dest,
    FName PropertyName, FString& OutError);

