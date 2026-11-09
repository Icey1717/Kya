#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "KyaLevelBank.generated.h"

USTRUCT(BlueprintType)
struct FKyaBankEntry
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, Category="Kya")
    int64 Offset = 0;

    UPROPERTY(VisibleAnywhere, Category="Kya")
    int64 Size = 0;

    UPROPERTY(VisibleAnywhere, Category="Kya")
    int32 Type = 0;

    UPROPERTY(VisibleAnywhere, Category="Kya")
    int32 Subtype = 0;
};

USTRUCT(BlueprintType)
struct FKyaActorClassAllocation
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, Category="Actor class")
    int32 ClassId = 0;
    UPROPERTY(EditAnywhere, Category="Actor class", meta=(ClampMin="0"))
    int32 Count = 0;
};

USTRUCT(BlueprintType)
struct FKyaScenarioOperation
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, Category="Operation") int32 Variable = 0;
    UPROPERTY(EditAnywhere, Category="Operation") int32 Type = 0;
    UPROPERTY(EditAnywhere, Category="Operation") int32 Value = 0;
};

USTRUCT(BlueprintType)
struct FKyaScenarioCondition
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, Category="Condition") int32 Combiner = 0;
    UPROPERTY(EditAnywhere, Category="Condition") int32 Variable = 0;
    UPROPERTY(EditAnywhere, Category="Condition") int32 Comparison = 0;
    UPROPERTY(EditAnywhere, Category="Condition") int32 Value = 0;
};

USTRUCT(BlueprintType)
struct FKyaCameraCondition
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, Category="Camera condition") int32 Combiner = 0;
    UPROPERTY(EditAnywhere, Category="Camera condition") int32 ActorRef = 0;
    UPROPERTY(EditAnywhere, Category="Camera condition") int32 Type = 0;
    UPROPERTY(EditAnywhere, Category="Camera condition") float Value = 0;
};

USTRUCT(BlueprintType)
struct FKyaConditionedOperation
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Scenario") TArray<FKyaScenarioOperation> Operations;
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Scenario") TArray<FKyaScenarioCondition> Conditions;
};

USTRUCT(BlueprintType)
struct FKyaObjectiveTarget
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, Category="Objective") int64 MessageKeyA = 0;
    UPROPERTY(EditAnywhere, Category="Objective") int64 MessageKeyB = 0;
    UPROPERTY(EditAnywhere, Category="Objective") int32 ActorRef = 0;
    UPROPERTY(EditAnywhere, Category="Objective") int32 ZoneRef = 0;
};

USTRUCT(BlueprintType)
struct FKyaObjective
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, Category="Objective") int32 ScenarioVariable = 0;
    /** Six serialized words whose meanings have not all been recovered. */
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Objective") TArray<int64> Fields;
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Objective") TArray<FKyaObjectiveTarget> Targets;
};

USTRUCT(BlueprintType)
struct FKyaObjectiveZoneEntry
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, Category="Objective") int32 Field = 0;
    UPROPERTY(EditAnywhere, Category="Objective") int32 ZoneRef = 0;
};

USTRUCT(BlueprintType)
struct FKyaWaypoint
{
    GENERATED_BODY()
    /** Position and rotation in the original game coordinate system. */
    UPROPERTY(EditAnywhere, Category="Waypoint") FVector Position = FVector::ZeroVector;
    UPROPERTY(EditAnywhere, Category="Waypoint") FVector Rotation = FVector::ZeroVector;
    UPROPERTY(EditAnywhere, Category="Waypoint") FVector Parameters = FVector::ZeroVector;
};

USTRUCT(BlueprintType)
struct FKyaPath
{
    GENERATED_BODY()
    UPROPERTY(VisibleAnywhere, Category="Path") int64 DataFlags = 0;
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Path") TArray<FVector4> Points;
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Path") TArray<FVector4> Rotations;
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Path") TArray<FVector4> ExtraVectors;
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Path") TArray<float> Delays;
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Path") TArray<int64> PointFlags;
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Path") TArray<float> ExtraValues;
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Path") TArray<int64> ExtraWords;
    UPROPERTY(EditAnywhere, Category="Path") int64 Mode = 0;
    UPROPERTY(EditAnywhere, Category="Path") int64 Type = 0;
    UPROPERTY(EditAnywhere, Category="Path") int64 PathType = 0;
};

USTRUCT(BlueprintType)
struct FKyaActorMacroAnimation
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, Category="Animation") int32 AnimationType = 0;
    UPROPERTY(EditAnywhere, Category="Animation") int32 MacroAnimationId = 0;
};

USTRUCT(BlueprintType)
struct FKyaClusterZone
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, Category="Cluster zone") int64 Flags = 0;
    UPROPERTY(EditAnywhere, Category="Cluster zone") int32 ZoneRef = 0;
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Cluster zone") TArray<int32> Actors;
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Cluster zone") TArray<int32> OtherActors;
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Cluster zone") TArray<int32> Cameras;
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Cluster zone") TArray<int32> Lights;
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Cluster zone") TArray<int64> Hashes;
    UPROPERTY(EditAnywhere, Category="Cluster zone") int32 OtherZoneRef = 0;
};

USTRUCT(BlueprintType)
struct FKyaActorBehaviour
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, Category="Behaviour") int32 Id = 0;
    /** Class-specific data, retained in its serialized form. */
    UPROPERTY(VisibleAnywhere, Category="Behaviour") bool bHasTypedClusterZones = false;
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Behaviour", meta=(EditCondition="!bHasTypedClusterZones")) TArray<uint8> Data;
    /** Decoded for clusteriser class 0x18, behaviour 2. */
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Cluster zones", meta=(EditCondition="bHasTypedClusterZones")) TArray<FKyaClusterZone> ClusterZones;
};

USTRUCT(BlueprintType)
struct FKyaActorModelReference
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, Category="Model") FString MeshName;
    UPROPERTY(EditAnywhere, Category="Model") FString TextureName;
};

USTRUCT(BlueprintType)
struct FKyaCheckpointActorWaypoint
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, Category="Checkpoint") int32 ActorRef = 0;
    UPROPERTY(EditAnywhere, Category="Checkpoint") int32 WaypointRef = 0;
};

USTRUCT(BlueprintType)
struct FKyaCheckpoint
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, Category="Checkpoint") int32 ZoneRef = 0;
    UPROPERTY(EditAnywhere, Category="Checkpoint") int32 WaypointA = 0;
    UPROPERTY(EditAnywhere, Category="Checkpoint") int32 SectorId = 0;
    UPROPERTY(EditAnywhere, Category="Checkpoint") int32 WaypointB = 0;
    UPROPERTY(EditAnywhere, Category="Checkpoint") int64 Flags = 0;
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Checkpoint") TArray<FKyaCheckpointActorWaypoint> ActorWaypoints;
};

USTRUCT(BlueprintType)
struct FKyaPlatformActor
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, Category="Moving platform") int32 ActorRef = 0;
    UPROPERTY(EditAnywhere, Category="Moving platform") int32 CarryMethod = 0;
};

USTRUCT(BlueprintType)
struct FKyaPlatformNoFrictionZone
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, Category="Moving platform") int32 ZoneRef = 0;
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Moving platform") TArray<int32> Actors;
};

USTRUCT(BlueprintType)
struct FKyaPlatformSound
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, Category="Moving platform") int32 Field0 = 0;
    UPROPERTY(EditAnywhere, Category="Moving platform") int32 Field4 = 0;
    UPROPERTY(EditAnywhere, Category="Moving platform") int32 SoundRef = 0;
    UPROPERTY(EditAnywhere, Category="Moving platform") int32 SampleRef = 0;
};

USTRUCT(BlueprintType)
struct FKyaPlatformCamera
{
    GENERATED_BODY()
    /** Serialized camera/notification record, 72 bytes. */
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Moving platform") TArray<uint8> Data;
};

USTRUCT(BlueprintType)
struct FKyaPlatformProperties
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, Category="Moving platform") int32 SoundA = 0;
    UPROPERTY(EditAnywhere, Category="Moving platform") int32 SampleA = 0;
    UPROPERTY(EditAnywhere, Category="Moving platform") int32 SoundB = 0;
    UPROPERTY(EditAnywhere, Category="Moving platform") int32 SampleB = 0;
    UPROPERTY(EditAnywhere, Category="Moving platform") FVector Offset = FVector::ZeroVector;
    UPROPERTY(EditAnywhere, Category="Moving platform") int64 Field1C = 0;
    UPROPERTY(EditAnywhere, Category="Moving platform") int64 Field20 = 0;
    UPROPERTY(EditAnywhere, Category="Moving platform") int64 Flags = 0;
    UPROPERTY(EditAnywhere, Category="Moving platform") int32 Field28 = 0;
    UPROPERTY(EditAnywhere, Category="Moving platform") float TiltValue = 0;
    UPROPERTY(EditAnywhere, Category="Moving platform") int32 TiltMode = 0;
    UPROPERTY(EditAnywhere, Category="Moving platform") FVector4 TiltAndPush = FVector4(0, 0, 0, 0);
    UPROPERTY(EditAnywhere, Category="Moving platform") FVector2D PushOscillation = FVector2D::ZeroVector;
};

USTRUCT(BlueprintType)
struct FKyaActorPlacement
{
    GENERATED_BODY()
    UPROPERTY(VisibleAnywhere, Category="Actor") int32 Index = 0;
    UPROPERTY(VisibleAnywhere, Category="Actor") int64 ClassId = 0;
    UPROPERTY(EditAnywhere, Category="Actor") FString Name;
    UPROPERTY(EditAnywhere, Category="Actor") int32 MeshIndex = 0;
    UPROPERTY(EditAnywhere, Category="Actor") int32 TextureIndex = 0;
    UPROPERTY(EditAnywhere, Category="Actor") int32 CollisionIndex = 0;
    UPROPERTY(EditAnywhere, Category="Actor") FVector Position = FVector::ZeroVector;
    UPROPERTY(EditAnywhere, Category="Actor") FVector Rotation = FVector::ZeroVector;
    UPROPERTY(EditAnywhere, Category="Actor") FVector Scale = FVector::OneVector;
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Animation") TArray<FKyaActorMacroAnimation> MacroAnimations;
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Behaviour") TArray<FKyaActorBehaviour> Behaviours;
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Model") TArray<FKyaActorModelReference> Models;
    UPROPERTY(EditAnywhere, Category="Actor settings") int32 DefaultBehaviourId = 0;
    UPROPERTY(EditAnywhere, Category="Actor settings") FVector4 BoundingSphere = FVector4(0, 0, 0, 0);
    UPROPERTY(EditAnywhere, Category="Actor settings") float Clipping = 0;
    UPROPERTY(EditAnywhere, Category="Actor settings") float VisibilityDistance = 0;
    UPROPERTY(EditAnywhere, Category="Actor settings") float CullingDistance = 0;
    UPROPERTY(EditAnywhere, Category="Actor settings") float Field20 = 0;
    UPROPERTY(EditAnywhere, Category="Actor settings") int32 SectorId = 0;
    UPROPERTY(EditAnywhere, Category="Actor settings") int64 HashCode = 0;
    UPROPERTY(EditAnywhere, Category="Actor settings") FVector2D LodBiases = FVector2D::ZeroVector;
    UPROPERTY(EditAnywhere, Category="Actor settings") int32 ActorFlags = 0;
    UPROPERTY(EditAnywhere, Category="Actor settings") int32 AnimationLayers = 0;
    UPROPERTY(EditAnywhere, Category="Actor settings") int32 Field40 = 0;
    UPROPERTY(EditAnywhere, Category="Actor settings") int64 LightingFlags = 0;
    UPROPERTY(EditAnywhere, Category="Actor settings") float LightingValue = 0;
    /** Scenario conditions read by CActor::Create after common settings. */
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Actor conditions") TArray<FKyaScenarioCondition> Conditions;
    /** Remaining class-specific Create data, preserved in serialized order; excludes a decoded moving-platform prefix. */
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Actor class data", meta=(EditCondition="!bHasTypedCheckpoints")) TArray<uint8> ClassData;
    UPROPERTY(VisibleAnywhere, Category="Moving platform") bool bHasPlatformPrefix = false;
    UPROPERTY(EditAnywhere, Category="Moving platform", meta=(EditCondition="bHasPlatformPrefix")) float PlatformWeight = 0;
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Moving platform", meta=(EditCondition="bHasPlatformPrefix")) TArray<FKyaPlatformActor> PlatformActors;
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Moving platform", meta=(EditCondition="bHasPlatformPrefix")) TArray<int32> PlatformZones;
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Moving platform", meta=(EditCondition="bHasPlatformPrefix")) TArray<FKyaPlatformNoFrictionZone> PlatformNoFrictionZones;
    UPROPERTY(VisibleAnywhere, Category="Moving platform") bool bHasPlatformDetails = false;
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Moving platform", meta=(EditCondition="bHasPlatformDetails")) TArray<FKyaPlatformCamera> PlatformCameras;
    UPROPERTY(EditAnywhere, Category="Moving platform", meta=(EditCondition="bHasPlatformDetails")) FKyaPlatformProperties PlatformProperties;
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Moving platform", meta=(EditCondition="bHasPlatformDetails")) TArray<FKyaPlatformSound> PlatformSoundsA;
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Moving platform", meta=(EditCondition="bHasPlatformDetails")) TArray<FKyaPlatformSound> PlatformSoundsB;
    UPROPERTY(VisibleAnywhere, Category="Checkpoints") bool bHasTypedCheckpoints = false;
    /** Checkpoint manager records, present for actor class 0x1e. */
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Checkpoints", meta=(EditCondition="bHasTypedCheckpoints")) TArray<FKyaCheckpoint> Checkpoints;
};

/** Initial scene configuration values read by BnkInstallSceneCfg. */
USTRUCT(BlueprintType)
struct FKyaSceneConfiguration
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, Category="Scene configuration")
    float Field1C = 0;

    UPROPERTY(EditAnywhere, Category="Scene configuration")
    int64 Field20 = 0;

    UPROPERTY(EditAnywhere, Category="Scene configuration")
    float Field24 = 0;

    /** Serialized words read before the 3D file manager configuration. */
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Scene configuration") TArray<int64> HeaderWords;
    /** Per-mesh serialized values consumed by C3DFileManager::Level_Create. */
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Scene configuration") TArray<int32> CommonMeshValues;

    /** Number of common level texture slots allocated by C3DFileManager. */
    UPROPERTY(EditAnywhere, Category="Scene configuration", meta=(ClampMin="0"))
    int32 CommonTextureCount = 0;
    UPROPERTY(EditAnywhere, Category="Scene configuration") int32 TextureWord = 0;

    /** Collision bank values; changing the array length is not supported. */
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Scene configuration")
    TArray<int32> CollisionBankValues;

    UPROPERTY(EditAnywhere, Category="Scene configuration", meta=(ClampMin="0"))
    int32 LevelAnimationCount = 0;
    /** Animation, scene and sector words following the animation count. */
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Scene configuration") TArray<int64> PostAnimationWords;

    UPROPERTY(EditAnywhere, EditFixedSize, Category="Scene configuration")
    TArray<FKyaActorClassAllocation> ActorClassAllocations;
    UPROPERTY(EditAnywhere, Category="Scene configuration") int64 ParticleWord = 0;
};

/** Fixed fields at the start of CScene::Level_Setup. */
USTRUCT(BlueprintType)
struct FKyaSceneEnvironment
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, Category="Environment")
    int64 BackgroundFlags = 0;
    UPROPERTY(EditAnywhere, Category="Environment")
    float ClipDistance = 0;
    UPROPERTY(EditAnywhere, Category="Environment")
    float FogParameter = 0;
    UPROPERTY(EditAnywhere, Category="Environment")
    FColor FogColor = FColor::Black;
    UPROPERTY(EditAnywhere, Category="Environment")
    int64 FogFlags = 0;
    UPROPERTY(EditAnywhere, Category="Environment")
    int32 MipmapL = 0;
    UPROPERTY(EditAnywhere, Category="Environment")
    int32 MipmapK = 0;
    /** Main camera values in the original game coordinate system. */
    UPROPERTY(EditAnywhere, Category="Main camera")
    float CameraFov = 0;
    UPROPERTY(EditAnywhere, Category="Main camera")
    FVector CameraPosition = FVector::ZeroVector;
    UPROPERTY(EditAnywhere, Category="Main camera")
    FVector CameraLookAt = FVector::ZeroVector;
    UPROPERTY(EditAnywhere, Category="Main camera")
    FVector CameraUp = FVector::ZeroVector;
    UPROPERTY(EditAnywhere, Category="Main camera")
    int32 CameraSectorId = 0;
    UPROPERTY(EditAnywhere, Category="Main camera")
    int32 CameraField08 = 0;
    UPROPERTY(EditAnywhere, Category="Main camera")
    int64 CameraFlags = 0;
    /** Target offset as serialized by CCamera, in game coordinates. */
    UPROPERTY(EditAnywhere, Category="Main camera")
    FVector CameraTargetOffset = FVector::ZeroVector;
    /** Optional switching data exists only when camera flag 0x2 was set in the bank. */
    UPROPERTY(VisibleAnywhere, Category="Main camera switching") bool bHasCameraSwitchData = false;
    UPROPERTY(EditAnywhere, Category="Main camera switching", meta=(EditCondition="bHasCameraSwitchData")) int32 CameraSwitchActor = -1;
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Main camera switching", meta=(EditCondition="bHasCameraSwitchData")) TArray<int32> CameraSwitchActorsA;
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Main camera switching", meta=(EditCondition="bHasCameraSwitchData")) TArray<int32> CameraSwitchActorsB;
    UPROPERTY(EditAnywhere, Category="Main camera switching", meta=(EditCondition="bHasCameraSwitchData")) float CameraSwitchValueA = 0;
    UPROPERTY(EditAnywhere, Category="Main camera switching", meta=(EditCondition="bHasCameraSwitchData")) int32 CameraSwitchModeA = 0;
    UPROPERTY(EditAnywhere, Category="Main camera switching", meta=(EditCondition="bHasCameraSwitchData")) float CameraSwitchValueB = 0;
    UPROPERTY(EditAnywhere, Category="Main camera switching", meta=(EditCondition="bHasCameraSwitchData")) int32 CameraSwitchModeB = 0;
    UPROPERTY(EditAnywhere, Category="Main camera switching", meta=(EditCondition="bHasCameraSwitchData")) float CameraSwitchValueC = 0;
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Main camera")
    TArray<FKyaCameraCondition> CameraConditions;
    UPROPERTY(EditAnywhere, Category="Main camera configuration")
    int64 CameraConfigFlags = 0;
    UPROPERTY(EditAnywhere, Category="Main camera configuration")
    int32 CameraActorRefA = 0;
    UPROPERTY(EditAnywhere, Category="Main camera configuration")
    int32 CameraActorRefB = 0;
    /** Serialized angle, in degrees. */
    UPROPERTY(EditAnywhere, Category="Main camera configuration")
    float CameraConfigAngle = 0;
    /** Optional five serialized configuration values; present only when the bank supplies them. */
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Main camera configuration")
    TArray<float> CameraOptionalConfigValues;
    /** Optional twelve serialized camera values, including values currently unused by the game. */
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Main camera configuration")
    TArray<float> CameraExtraValues;
    /** Serialized pitch angles, in degrees. */
    UPROPERTY(EditAnywhere, Category="Main camera configuration")
    FVector CameraPitchAngles = FVector::ZeroVector;
    UPROPERTY(EditAnywhere, Category="Main camera configuration")
    FVector CameraOtherAngles = FVector::ZeroVector;
    UPROPERTY(EditAnywhere, Category="Main camera configuration")
    FVector CameraDistances = FVector::ZeroVector;
    UPROPERTY(EditAnywhere, Category="Scene setup")
    float Field118 = 0;
    UPROPERTY(EditAnywhere, Category="Scene setup")
    int64 Field11C = 0;
    UPROPERTY(EditAnywhere, Category="Scene setup")
    int64 Count120 = 0;
    UPROPERTY(EditAnywhere, Category="Map")
    int32 MapField04 = 0;
    UPROPERTY(EditAnywhere, Category="Map")
    int64 MapField08 = 0;
    /** Serialized map angle in degrees. */
    UPROPERTY(EditAnywhere, Category="Map")
    float MapAngle = 0;
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Map")
    TArray<int32> MapEntries;
    UPROPERTY(EditAnywhere, Category="Map")
    int32 MapField14 = 0;
    UPROPERTY(EditAnywhere, Category="Map")
    int32 MapActorRef = 0;
    UPROPERTY(EditAnywhere, Category="Scene setup")
    int32 DefaultTextureIndex = 0;
    UPROPERTY(EditAnywhere, Category="Scene setup")
    int32 AlternateTextureIndex = 0;
    UPROPERTY(EditAnywhere, Category="Scene setup")
    int32 DefaultMaterialIndex = 0;
    /** Pool counts in game category order: composite, path, group, particle, sound, random, LOD. */
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Effects")
    TArray<int64> EffectPoolCounts;
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Scenario")
    TArray<FKyaConditionedOperation> InitialScenarioOperations;
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Objectives")
    TArray<FKyaObjective> Objectives;
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Objectives")
    TArray<FKyaObjectiveZoneEntry> ObjectiveZones;
    UPROPERTY(EditAnywhere, Category="Scene setup") int32 Field10C = 0;
    UPROPERTY(EditAnywhere, Category="Scene setup") int32 Field110 = 0;
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Scene setup") TArray<int32> ElevatorCutscenes;
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Waypoints") TArray<FKyaWaypoint> Waypoints;
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Paths") TArray<FKyaPath> Paths;
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Actors") TArray<FKyaActorPlacement> Actors;
};

/** Original bank bytes are retained so that unimplemented records are not lost. */
UCLASS(BlueprintType)
class KYALEVEL_API UKyaLevelBank : public UDataAsset
{
    GENERATED_BODY()
public:
    static constexpr int32 CurrentImportSchemaVersion = 1;
    bool HasCurrentImportSchema() const { return ImportSchemaVersion == CurrentImportSchemaVersion; }

    /** Version of the parsed properties; older assets must be reimported. */
    UPROPERTY(VisibleAnywhere, Category="Kya")
    int32 ImportSchemaVersion = 0;

    UPROPERTY(VisibleAnywhere, Category="Kya")
    FString SourceFilename;

    UPROPERTY(VisibleAnywhere, Category="Kya")
    int32 EntryCount = 0;

    UPROPERTY(VisibleAnywhere, Category="Kya")
    TArray<FKyaBankEntry> Entries;

    UPROPERTY(EditAnywhere, Category="Kya")
    FKyaSceneConfiguration SceneConfiguration;

    UPROPERTY(EditAnywhere, Category="Kya")
    FKyaSceneEnvironment SceneEnvironment;

    /** Bank entry indices resolved from the scene's mesh and texture references. */
    UPROPERTY(VisibleAnywhere, Category="Kya|Scene")
    int32 SceneMeshEntry = INDEX_NONE;
    UPROPERTY(VisibleAnywhere, Category="Kya|Scene")
    int32 SceneTextureEntry = INDEX_NONE;
    UPROPERTY(VisibleAnywhere, Category="Kya|Scene")
    int32 BackgroundMeshEntry = INDEX_NONE;
    UPROPERTY(VisibleAnywhere, Category="Kya|Scene")
    int32 BackgroundTextureEntry = INDEX_NONE;

    UPROPERTY()
    TArray<uint8> BankBytes;
};
