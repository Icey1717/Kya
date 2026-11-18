#pragma once

#include "CoreMinimal.h"
#include "KyaActorFamily.h"

class UKyaLevelBank;
struct FKyaActorPlacement;
struct FKyaActorBehaviour;
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
/** True when the id is -1 (no behaviour) or matches one of the given behaviour record ids. */
KYALEVEL_API bool DoesKyaBehaviourIdExist(const TArray<FKyaActorBehaviour>& Behaviours, int32 BehaviourId);

/** Reference problems found in one placement; blocking issues reject an edit. */
struct KYALEVEL_API FKyaActorReferenceIssue
{
    FString Field;
    FString Message;
    bool bBlocking = false;
};

KYALEVEL_API TArray<FKyaActorReferenceIssue> ValidateKyaActorReferences(const UKyaLevelBank& Bank,
    const FKyaActorPlacement& Placement);

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

/** Copy every editable exported property except Name, preserving identity and serialized layout.
 *
 * A field is copied only when it fits the destination's serialized layout
 * (presence flags, record ids, array counts and string slots). Resizable arrays
 * (Conditions, MacroAnimations, Behaviours, Models and supported moving-platform/
 * trap/fighter/Wolfen-fight/Shocker-command streams) follow the source count/string
 * slot and are rebuilt at export.
 * Other fields that do not fit are left unchanged instead of rejecting the whole copy.
 * Type-specific records are copied only between actors of the same class; a
 * different class keeps its class payload, Behaviours, DefaultBehaviourId and
 * ClassData so its actor chunk remains valid for the bank writer and can export.
 */
KYALEVEL_API bool BuildKyaActorCopyPropertiesPatch(const FKyaActorPlacement& Source,
    FKyaActorEditPatch& OutPatch);

/**
 * True when the top-level actor array can change count and be rebuilt in the
 * serialized actor chunk at export time. The supported arrays are Conditions,
 * MacroAnimations, Behaviours and Models; other fixed-size arrays keep their
 * serialized counts.
 */
KYALEVEL_API bool IsKyaActorArrayResizable(FName PropertyName);

/**
 * Class-aware form of IsKyaActorArrayResizable. Moving-platform arrays are
 * rebuildable for class 0x0d, trap streams for class 0x2c, fighter databases for
 * Wolfen (0x09) / Kim (0x06), command sequences for Shocker command (0x50) and the
 * actor stream for box (0x35) / basic box (0x39). Kim's climb, tutorial (with nested
 * targets), projectile, boomy-pair, zone and camera streams are also rebuildable, as are
 * the pattern-command actor-ref list (0x40), the shoot fire-ref behaviour list (0x3e), the
 * clusteriser zone tree (0x18), the fog-zone list (0x3d), the Brazul A sub-object/path lists
 * (0x4f), the fog-hierarchy list (0x3d), the FX integer list (0x2e), the weapon RPG actor stream
 * (0x20), the companion alert list (0x19), the Bunch stream-entry/fence/repel lists (0x4e), the
 * Brazul A/B lists (0x4f), the Shocker switch list (0x4b), the Wind carried-actor and switch
 * lists (0x12), the Nativ speak/trajectory/seller lists (0x0a), the Stiller actor list (0x4c), the
 * Ship care-boy/FX integer lists (0x36), the Punching ball trap list (0x30), the Amortos command
 * actor list (0x31), the Nose monster zone-path/fire-shot lists (0x26), the DCA shot actor list
 * (0x1b), the Teleporter destination/waypoint/conditioned-operation lists (0x24), the Aton
 * path/other/aton sub-object lists (0x38, with nested cinematic ids), the Blazer actor list
 * (0x4d, nested under BlazerData) and the Egg path-plane list (0x2f). The Woof carried-actor list
 * (0x2a, nested under WoofData) is also rebuildable;
 * the common Conditions/MacroAnimations/Behaviours/Models arrays remain available to every class.
 * Wolfen fight-param combo/capability lists are handled separately in the nested layout validator.
 */
KYALEVEL_API bool IsKyaActorArrayResizable(int64 ClassId, FName PropertyName);

/**
 * Keep the fighter structures the serialized prefix requires: three blow stages
 * and five grab stages. Newly added blows/grabs receive zeroed stages until edited.
 */
KYALEVEL_API void KyaEnsureFighterArrayShapes(FKyaActorPlacement& Placement);

/**
 * Validate an Outliner rename before it becomes the serialized actor name.
 *
 * The bank stores names in a fixed slot: the writer requires
 * Align(Length + 1, 4) to equal the original capacity. Renames that change that
 * 4-byte bucket, exceed the character limit or contain non-ASCII characters are
 * rejected with an explanation; they are never silently truncated.
 */
KYALEVEL_API bool ValidateKyaActorNameChange(const FString& CurrentName, const FString& NewName,
    FString& OutError);

