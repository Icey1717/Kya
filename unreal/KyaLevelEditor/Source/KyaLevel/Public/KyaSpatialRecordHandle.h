#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "KyaSpatialRecordHandle.generated.h"

class AKyaLevelPreview;
class UKyaLevelBank;
class USceneComponent;
class UStaticMeshComponent;

UENUM()
enum class EKyaSpatialRecordKind : uint8
{
    Waypoint,
    PathPoint
};

/**
 * Selectable, editable representation of a waypoint or a path point.
 *
 * Identity is bank + record index (+ point index for paths) + the bank edit
 * generation, so a reimport invalidates stale handles. Edits go through the shared
 * edit service and update every preview of the bank once. The full supported record
 * is exposed in Details (position/rotation/parameters for waypoints, and the point
 * plus its parallel per-point records for paths).
 *
 * Stored waypoint rotation is an Euler angle triple in radians, used by the game with
 * SetVectorFromAngles/InitPositionRotation (see ActorMicken / spawn parameters in the
 * ported source). It is edited numerically; orientation is not drawn because the
 * arrow convention is not verified.
 */
UCLASS()
class KYALEVEL_API AKyaSpatialRecordHandle : public AActor
{
    GENERATED_BODY()
public:
    AKyaSpatialRecordHandle();
    virtual void OnConstruction(const FTransform& Transform) override;
    virtual void Destroyed() override;
#if WITH_EDITOR
    virtual void PostEditMove(bool bFinished) override;
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

    void Initialize(AKyaLevelPreview* InPreview, UKyaLevelBank* InBank, EKyaSpatialRecordKind InKind,
        int32 InRecordIndex, int32 InPointIndex);
    void SyncFromBank();
    /** Apply the current mirrored fields (used by scripts and the Details panel). */
    void ApplyToBank();

    UPROPERTY(VisibleAnywhere, Category="Kya|Identity")
    TObjectPtr<USceneComponent> SceneRoot;

    UPROPERTY(VisibleAnywhere, Category="Kya|Identity")
    TObjectPtr<UStaticMeshComponent> Marker;

    UPROPERTY(VisibleAnywhere, Category="Kya|Identity")
    TObjectPtr<UKyaLevelBank> Bank;

    UPROPERTY(VisibleAnywhere, Transient, Category="Kya|Identity")
    TObjectPtr<AKyaLevelPreview> Preview;

    UPROPERTY(VisibleAnywhere, Category="Kya|Identity")
    EKyaSpatialRecordKind Kind = EKyaSpatialRecordKind::Waypoint;

    /** Waypoint index, or path index for path-point records. */
    UPROPERTY(VisibleAnywhere, Category="Kya|Identity")
    int32 RecordIndex = INDEX_NONE;

    UPROPERTY(VisibleAnywhere, Category="Kya|Identity")
    int32 PointIndex = INDEX_NONE;

    UPROPERTY(VisibleAnywhere, Category="Kya|Identity")
    int32 EditGeneration = 0;

    /** Waypoint position in game space (metres, Y up). */
    UPROPERTY(EditAnywhere, Category="Kya|Waypoint")
    FVector WaypointPosition = FVector::ZeroVector;

    /** Stored Euler angles in radians; consumed by the game as a rotation triple. */
    UPROPERTY(EditAnywhere, Category="Kya|Waypoint")
    FVector WaypointRotation = FVector::ZeroVector;

    /** Waypoint parameter triple (reserved fields at 0x18..0x20 in CWayPoint). */
    UPROPERTY(EditAnywhere, Category="Kya|Waypoint")
    FVector WaypointParameters = FVector::ZeroVector;

    /** Path point position (W is the serialized fourth component). */
    UPROPERTY(EditAnywhere, Category="Kya|Path point")
    FVector4 PathPoint = FVector4(0, 0, 0, 0);

    UPROPERTY(EditAnywhere, Category="Kya|Path point")
    FVector4 PathRotation = FVector4(0, 0, 0, 0);

    UPROPERTY(EditAnywhere, Category="Kya|Path point")
    FVector4 PathExtraVector = FVector4(0, 0, 0, 0);

    UPROPERTY(EditAnywhere, Category="Kya|Path point")
    float PathDelay = 0;

    UPROPERTY(EditAnywhere, Category="Kya|Path point")
    int64 PathFlags = 0;

    UPROPERTY(EditAnywhere, Category="Kya|Path point")
    float PathExtraValue = 0;

    UPROPERTY(EditAnywhere, Category="Kya|Path point")
    int64 PathExtraWord = 0;

    UPROPERTY(VisibleAnywhere, Category="Kya|Path point")
    int64 PathMode = 0;

    UPROPERTY(VisibleAnywhere, Category="Kya|Path point")
    int64 PathType = 0;

    UPROPERTY(VisibleAnywhere, Category="Kya|Path point")
    int64 PathPathType = 0;

    /** True when this path has a per-point record of the given kind (for Details hints). */
    UPROPERTY(VisibleAnywhere, Category="Kya|Path point")
    bool bHasRotationRecord = false;
    UPROPERTY(VisibleAnywhere, Category="Kya|Path point")
    bool bHasExtraVectorRecord = false;
    UPROPERTY(VisibleAnywhere, Category="Kya|Path point")
    bool bHasDelayRecord = false;
    UPROPERTY(VisibleAnywhere, Category="Kya|Path point")
    bool bHasFlagsRecord = false;
    UPROPERTY(VisibleAnywhere, Category="Kya|Path point")
    bool bHasExtraValueRecord = false;
    UPROPERTY(VisibleAnywhere, Category="Kya|Path point")
    bool bHasExtraWordRecord = false;

private:
    bool bSyncing = false;
    void CommitFromActor();
    void MirrorFromBank();
};
