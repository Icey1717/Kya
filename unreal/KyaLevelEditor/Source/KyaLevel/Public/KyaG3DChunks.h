#pragma once

#include "CoreMinimal.h"

struct FKyaG3DChunk
{
    uint32 Tag = 0;
    int32 Offset = 0;
    int32 Size = 0;
};

struct FKyaG3DStripReference
{
    int32 HierarchyOffset = 0;
    int32 StripOffset = 0;
};

struct FKyaG3DVertex
{
    FVector3f Position = FVector3f::ZeroVector;
    uint32 Flags = 0;
};

struct FKyaG3DVertexAttributes
{
    FVector2f UV = FVector2f::ZeroVector;
    FColor Color = FColor::White;
};

/** One Eden 3D sprite (SPR.) record: a billboard definition inside a G3D file. */
struct FKyaG3DSprite
{
    int32 Offset = 0;
    int32 MaterialIndex = 0;
    int32 Flags = 0;
    int32 RectCount = 0;
    int32 ShadowFlag = 0;
    int32 RenderFrame = 0;
    FVector4f BoundingSphere = FVector4f(0, 0, 0, 0);
    TArray<uint32> ChunkTags;
    TArray<int32> ChunkSizes;
};

struct FKyaG3DDecodedStrip
{
    int32 HierarchyOffset = 0;
    uint32 Primitive = 0;
    int32 LodIndex = 0;
    float LodMinSizeBias = 0;
    float LodMaxSizeBias = 0;
    FVector4f ObjectSphere = FVector4f(0, 0, 0, 0);
    FVector4f FirstLodSphere = FVector4f(0, 0, 0, 0);
    FVector4f StripSphere = FVector4f(0, 0, 0, 0);
    int32 MaterialIndex = INDEX_NONE;
    uint64 MaterialHash = 0;
    TArray<FKyaG3DVertex> Vertices;
    /** Stored model-space normals, one per expanded vertex; may be empty. */
    TArray<FVector3f> Normals;
    TArray<FKyaG3DVertexAttributes> Attributes;
    TArray<uint32> Indices;
};

/** Fixed-size background flare configured by C3DFileManager::SetupBackground. */
struct FKyaG3DBackgroundFlare
{
    int32 HierarchyOffset = 0;
    uint64 MaterialHash = 0;
    FVector3f Center = FVector3f::ZeroVector;
};

/** One prepared sprite rectangle from an installed cluster sprite packet. */
struct FKyaG3DSpriteRect
{
    FVector3f Center = FVector3f::ZeroVector;
    /** Half extents in game units, already divided by 4096. */
    FVector2f HalfSize = FVector2f::ZeroVector;
    FVector2f UV[4] = {};
    FColor Color[4] = {};
};

/** A sprite whose prepared packet is serialized inside the G3D cluster. */
struct FKyaG3DPreparedSprite
{
    int32 StructOffset = 0;
    int32 MaterialIndex = INDEX_NONE;
    uint64 MaterialHash = 0;
    int32 RenderFrame = 0;
    TArray<FKyaG3DSpriteRect> Rects;
};

/** Enumerates top-level G3D chunks without modifying the serialized pointers. */
namespace KyaG3D
{
    struct FClusterStripReference { int32 StripOffset = INDEX_NONE; int32 MaterialBank = INDEX_NONE; };
    /** Enumerate root and nested quadtree/octree cluster strips and their material banks. */
    KYALEVEL_API bool ReadClusterTreeStrips(TConstArrayView<uint8> Data, TArray<FClusterStripReference>& OutStrips,
        TArray<int32>* OutHierarchies = nullptr);
    KYALEVEL_API bool ReadChunks(TConstArrayView<uint8> Data, TArray<FKyaG3DChunk>& OutChunks);
    /** Resolve in-range REAA references; report fields whose targets lie outside the file. */
    KYALEVEL_API bool ReadRelocations(TConstArrayView<uint8> Data, TMap<int32, int32>& OutOffsets,
        TArray<int32>& OutInvalidFields);
    /** Locate serialized hierarchy payloads in HALL's hash table. */
    KYALEVEL_API bool ReadHierarchyOffsets(TConstArrayView<uint8> Data, TArray<int32>& OutOffsets);
    /** Follow serialized cluster hierarchy links, rejecting cycles and invalid targets. */
    KYALEVEL_API bool ExpandClusterHierarchies(TConstArrayView<uint8> Data, TConstArrayView<int32> Roots,
        TArray<int32>& OutHierarchies);
    KYALEVEL_API bool ReadHierarchyStrips(TConstArrayView<uint8> Data, TArray<FKyaG3DStripReference>& OutStrips, int32 LodIndex = 0);
    /** Type-3 hierarchy objects contain sprites even without a SPR. chunk. */
    KYALEVEL_API bool ReadBackgroundFlares(TConstArrayView<uint8> Data, TArray<FKyaG3DBackgroundFlare>& OutFlares);
    /** Locate static strips in a CDQA/CDQU cluster. */
    KYALEVEL_API bool ReadClusterStrips(TConstArrayView<uint8> Data, TArray<int32>& OutOffsets, int32* OutMaterialBank = nullptr);
    KYALEVEL_API bool ReadStripMaterialHash(TConstArrayView<uint8> Data, int32 StripOffset, int32 MaterialBank, uint64& OutHash);
    /** Read the first GIF tag referenced by a strip's VIF packet. */
    KYALEVEL_API bool ReadFirstStripGifTag(TConstArrayView<uint8> Data, int32 StripOffset,
        uint32& OutVertexCount, uint32& OutPrimitive);
    KYALEVEL_API bool ReadFirstStripVertices(TConstArrayView<uint8> Data, int32 StripOffset,
        TArray<FKyaG3DVertex>& OutVertices);
    /** Vertex counts of all GIF sections in a strip, in draw order. */
    KYALEVEL_API bool ReadStripSectionCounts(TConstArrayView<uint8> Data, int32 StripOffset,
        TArray<uint32>& OutCounts);
    KYALEVEL_API bool ReadStripVertices(TConstArrayView<uint8> Data, int32 StripOffset,
        TArray<FKyaG3DVertex>& OutVertices, TArray<uint32>& OutSectionCounts);
    /** Attributes are in section order, including duplicated boundary vertices. */
    KYALEVEL_API bool ReadStripAttributes(TConstArrayView<uint8> Data, int32 StripOffset,
        TArray<FKyaG3DVertexAttributes>& OutAttributes);
    KYALEVEL_API bool BuildStripTriangles(TConstArrayView<FKyaG3DVertex> Vertices,
        TConstArrayView<uint32> SectionCounts, uint32 Primitive, TArray<uint32>& OutIndices);
    /** Enumerate SPR. billboard records; sprite geometry is index-based and not decoded yet. */
    KYALEVEL_API bool ReadSprites(TConstArrayView<uint8> Data, TArray<FKyaG3DSprite>& Out);
    /** Decode prepared sprite packets from cluster nodes (p3DSprite at cluster+0x3c). */
    KYALEVEL_API bool ReadClusterSprites(TConstArrayView<uint8> Data, TArray<FKyaG3DPreparedSprite>& Out);
    /** Decode hierarchy strips; fails if any strip uses an unsupported format. */
    KYALEVEL_API bool DecodeHierarchyStrips(TConstArrayView<uint8> Data,
        TArray<FKyaG3DDecodedStrip>& OutStrips, int32 LodIndex = 0);
    KYALEVEL_API bool ReadHierarchyWorldMatrix(TConstArrayView<uint8> Data, int32 HierarchyOffset, FMatrix& OutMatrix);
    /** Scene instances installed separately from CSTA: DYNAMIC and ANHR roots/children. */
    KYALEVEL_API bool ReadSectorSceneHierarchies(TConstArrayView<uint8> Data,
        TConstArrayView<uint8> AnimationData, TArray<int32>& OutOffsets);
    /** Hierarchies whose local translations the game replaces with the camera position. */
    KYALEVEL_API bool ReadCameraRelativeBackgroundHierarchies(TConstArrayView<uint8> Data, TArray<int32>& OutOffsets);
    KYALEVEL_API bool ReadBackgroundWorldMatrix(TConstArrayView<uint8> Data, int32 HierarchyOffset,
        TConstArrayView<int32> CameraRelativeHierarchies, const FVector& CameraPosition, FMatrix& OutMatrix);
}
