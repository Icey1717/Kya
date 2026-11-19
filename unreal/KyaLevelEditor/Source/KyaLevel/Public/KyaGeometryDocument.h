#pragma once

#include "CoreMinimal.h"
#include "Misc/Optional.h"
#include "KyaPrimitives.h"

class UKyaLevelBank;
struct FKyaG3DDecodedStrip;

enum class EKyaGeometryKind : uint8 { Render, Collision };

/** Ordinals are scoped to an import generation. Native offsets are provenance only. */
struct KYALEVEL_API FKyaGeometryResourceKey
{
    TWeakObjectPtr<UKyaLevelBank> Bank;
    int32 EntryIndex = INDEX_NONE;
    int32 ResourceIndex = INDEX_NONE;
    int32 LodIndex = INDEX_NONE;
    int32 Generation = 0;
    EKyaGeometryKind Kind = EKyaGeometryKind::Render;

    bool IsCurrent() const;
    bool operator==(const FKyaGeometryResourceKey& Other) const;
    friend uint32 GetTypeHash(const FKyaGeometryResourceKey& Key)
    {
        uint32 Hash = HashCombine(GetTypeHash(Key.Bank), GetTypeHash(Key.EntryIndex));
        Hash = HashCombine(Hash, GetTypeHash(Key.ResourceIndex));
        Hash = HashCombine(Hash, GetTypeHash(Key.LodIndex));
        return HashCombine(HashCombine(Hash, GetTypeHash(Key.Generation)), uint32(Key.Kind));
    }
};

/** Attributes belong to corners, so moving a shared vertex cannot erase a seam. */
struct KYALEVEL_API FKyaGeometryCorner
{
    int32 VertexIndex = INDEX_NONE;
    int32 SourceVertexIndex = INDEX_NONE;
    /** Entry-relative native vertex record location, when supplied by the decoder. */
    int32 SourceVertexOffset = INDEX_NONE;
    TOptional<FVector3f> Normal;
    TOptional<FVector2f> UV;
    TOptional<FColor> Color;
    uint32 NativeVertexFlags = 0;
};

struct KYALEVEL_API FKyaGeometryFace
{
    TArray<FKyaGeometryCorner> Corners;
    int32 SourceFaceIndex = INDEX_NONE;
    int32 SourceOffset = INDEX_NONE;
    /** Optional means no choice has been made; zero is a valid explicit choice. */
    TOptional<int32> MaterialIndex;
    TOptional<uint32> SurfaceFlags;
};

/** Local game coordinates, with no game-to-Unreal conversion baked into the data.
 * This is a temporary working copy, never an authoritative level asset. Retained
 * bytes include unknown attributes, relocation data and collision tree records. */
struct KYALEVEL_API FKyaGeometryDocument
{
    FKyaGeometryResourceKey Key;
    TArray<FVector> Vertices;
    TArray<FKyaGeometryFace> Faces;
    TArray<FKyaPrimitiveRecord> RetainedPrimitives;
    TArray<uint8> SourceEntryBytes;
    /** Render resources from the same entry share their immutable snapshot. */
    TSharedPtr<const TArray<uint8>, ESPMode::ThreadSafe> SharedEntryBytes;
    const TArray<uint8>& GetSourceEntryBytes() const { return SharedEntryBytes.IsValid() ? *SharedEntryBytes : SourceEntryBytes; }
    /** Provenance only; keys use the import-generation resource ordinal. */
    int32 NativeResourceOffset = INDEX_NONE;
    bool bRequiresNormals = false;
    bool bRequiresUVs = false;
    bool bRequiresColors = false;
    bool bPackedPositions = false;
    FString NativeFormatDiagnostic = TEXT("A native writer format has not been established for this resource.");
};

struct KYALEVEL_API FKyaGeometryIssue
{
    bool bError = true;
    int32 FaceIndex = INDEX_NONE;
    int32 VertexIndex = INDEX_NONE;
    FString Message;
};

namespace KyaGeometry
{
    /** Checks editable topology/attributes, not native encodability. Open collision
     * surfaces are valid. Nonmanifold edges produce warnings with face identities. */
    KYALEVEL_API bool Validate(const FKyaGeometryDocument& Document, TArray<FKyaGeometryIssue>& Issues);
    /** Snapshot one static 7/1 collision entry, preserving native shared vertices,
     * quads, flags, referenced boxes/spheres and complete original entry bytes. */
    KYALEVEL_API bool OpenCollision(UKyaLevelBank& Bank, int32 EntryIndex,
        FKyaGeometryDocument& OutDocument, FString& OutError);
    /** Adapter for an already decoded render resource. Does not weld coincident
     * expanded vertices. Caller supplies a generation-scoped resource ordinal. */
    KYALEVEL_API bool FromDecodedStrip(const FKyaGeometryResourceKey& Key, const FKyaG3DDecodedStrip& Strip,
        FKyaGeometryDocument& OutDocument, FString& OutError);
    /** Explicit append, in local game coordinates. Converts from source local to
     * target local, reversing winding for reflected transforms. All-or-nothing. */
    KYALEVEL_API bool AppendFacesToCollision(const FKyaGeometryDocument& Source,
        TConstArrayView<int32> SelectedFaces, const FMatrix& SourceToTarget,
        uint32 SurfaceFlags, FKyaGeometryDocument& Target, FString& OutError);
    /** Also rejects same-generation edits that replaced the original entry bytes. */
    KYALEVEL_API bool MatchesSource(const FKyaGeometryDocument& Document);
}
