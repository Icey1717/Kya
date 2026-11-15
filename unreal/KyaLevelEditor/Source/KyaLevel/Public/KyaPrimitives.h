#pragma once

#include "CoreMinimal.h"

class UKyaLevelBank;

enum class EKyaPrimitiveShape : uint8
{
    Triangle, Quad, Box, Sphere, HalfSpace, OpenTopBox, Cylinder, Cone
};

/** Source identity is entry + byte offset, never a viewport array index. All offsets
 * are entry-relative. Shared event matrices remain shared records, not editable copies.
 * Geometry is in game coordinates; conversion to preview coordinates happens at draw time. */
struct KYALEVEL_API FKyaPrimitiveRecord
{
    int32 EntryIndex = INDEX_NONE;
    int32 SourceOffset = INDEX_NONE;
    int32 TransformOffset = INDEX_NONE;
    /** Event list word that references the matrix; distinct from the shared matrix itself. */
    int32 MembershipOffset = INDEX_NONE;
    bool bSerializedWorldToLocal = false;
    int32 ZoneIndex = INDEX_NONE;
    int32 GroupIndex = INDEX_NONE;
    int32 PrimitiveIndex = INDEX_NONE;
    uint32 Flags = 0;
    EKyaPrimitiveShape Shape = EKyaPrimitiveShape::Triangle;
    FMatrix LocalToGame = FMatrix::Identity;
    TArray<FVector> Vertices;
    /** Entry-relative offsets of shared collision vertex records, in polygon order. */
    TArray<int32> VertexOffsets;
};

namespace KyaPrimitives
{
    /** Non-mutating, bounds checked decode of retained 7/1 COL and 8/1 event entries.
     * A malformed entry contributes no partial records; other entries still decode. */
    KYALEVEL_API void Decode(const UKyaLevelBank& Bank, bool bCollision, bool bEvents,
        TArray<FKyaPrimitiveRecord>& Records, TArray<FString>& Diagnostics);
    /** Unit-shape wire edges transformed into game coordinates. Infinite shapes use
     * a finite guide with upward rays; the guide is not a closed volume. */
    KYALEVEL_API void BuildEdges(const FKyaPrimitiveRecord& Record, TArray<FVector>& Edges);
    /** Triangle list in game coordinates. Infinite shapes remain open guides. */
    KYALEVEL_API void BuildTriangles(const FKyaPrimitiveRecord& Record, TArray<FVector>& Triangles);
}
