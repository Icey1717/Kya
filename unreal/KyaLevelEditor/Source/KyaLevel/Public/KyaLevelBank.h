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

/** One entry from the level light archive (THGL / LGHT). */
USTRUCT(BlueprintType)
struct FKyaLight
{
    GENERATED_BODY()
    /** AddLight type: 0/1 simple light, 2 torch. */
    UPROPERTY(EditAnywhere, Category="Light") int32 LightType = 0;
    /** Simple-light shape: 0 light, 1 sun, 2 directional, 4 spot. */
    UPROPERTY(EditAnywhere, Category="Light") int32 ShapeType = 0;
    UPROPERTY(EditAnywhere, Category="Light") FLinearColor AmbientColor = FLinearColor::Black;
    UPROPERTY(EditAnywhere, Category="Light") FLinearColor Color = FLinearColor::White;
    UPROPERTY(EditAnywhere, Category="Light") FLinearColor Field20Color = FLinearColor::White;
    UPROPERTY(EditAnywhere, Category="Light") float Multiplier = 1;
    /** Shape position in game metres. */
    UPROPERTY(EditAnywhere, Category="Light") FVector Position = FVector::ZeroVector;
    UPROPERTY(EditAnywhere, Category="Light") float Falloff0 = 0;
    UPROPERTY(EditAnywhere, Category="Light") float Falloff1 = 0;
    UPROPERTY(EditAnywhere, Category="Light") FVector Direction = FVector::ZeroVector;
    UPROPERTY(EditAnywhere, Category="Light") float Fov0 = 0;
    UPROPERTY(EditAnywhere, Category="Light") float Fov1 = 0;
    /** Trailing words of the serialized record, retained for inspection. */
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Light") TArray<int32> TrailingValues;
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

/** AdvancedEntry serialized by CBehaviourWeaponBurst. */
USTRUCT(BlueprintType)
struct FKyaWeaponAdvancedEntry
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, Category="Weapon") float FieldC = 0;
    UPROPERTY(EditAnywhere, Category="Weapon") int32 Field4 = 0;
    UPROPERTY(EditAnywhere, Category="Weapon") float Field0 = 0;
    UPROPERTY(EditAnywhere, Category="Weapon") float Field8 = 0;
};

USTRUCT(BlueprintType)
struct FKyaNativTrajectory
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, Category="Nativ") float Field0 = 0;
    UPROPERTY(EditAnywhere, Category="Nativ") int32 PathFollowIndex = -1;
    UPROPERTY(EditAnywhere, Category="Nativ") int32 Field14 = 0;
    UPROPERTY(EditAnywhere, Category="Nativ") float Field18 = 0;
};

USTRUCT(BlueprintType)
struct FKyaFogHierarchy
{
    GENERATED_BODY()
    /** CSectorHierarchy serialized hash, then two fog radius/density floats. */
    UPROPERTY(EditAnywhere, Category="Fog") int64 Hash = 0;
    UPROPERTY(EditAnywhere, Category="Fog") float Field20 = 0;
    UPROPERTY(EditAnywhere, Category="Fog") float Field24 = 0;
};

USTRUCT(BlueprintType)
struct FKyaFogZone
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, Category="Fog zone") int32 Flags = 0;
    UPROPERTY(EditAnywhere, Category="Fog zone") int32 ZoneRef = -1;
    UPROPERTY(EditAnywhere, Category="Fog zone") float ClipValue = 0;
    UPROPERTY(EditAnywhere, Category="Fog zone") float Field4 = 0;
    /** Packed RGBA bytes exactly as serialized. */
    UPROPERTY(EditAnywhere, Category="Fog zone") int32 FogRGBA = 0;
    UPROPERTY(EditAnywhere, Category="Fog zone") int32 FogFlags = 0;
    UPROPERTY(EditAnywhere, Category="Fog zone") float Field18 = 0;
};

USTRUCT(BlueprintType)
struct FKyaAddOnSubObj
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, Category="Add-on") int32 Field0 = 0;
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Add-on") TArray<int32> CinematicIds;
};

USTRUCT(BlueprintType)
struct FKyaNativCmdAddOn
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Nativ command") TArray<FKyaAddOnSubObj> SubObjs;
};

USTRUCT(BlueprintType)
struct FKyaAtonSubObj
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, Category="Aton") int32 Field18 = 0;
    UPROPERTY(EditAnywhere, Category="Aton") int32 Field0 = 0;
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Aton") TArray<int32> CinematicIds;
};

USTRUCT(BlueprintType)
struct FKyaBoomyType
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, Category="Boomy type") float Field4 = 0;
    UPROPERTY(EditAnywhere, Category="Boomy type") float VisionRange = 0;
    UPROPERTY(EditAnywhere, Category="Boomy type") int32 HitDamage = 0;
    UPROPERTY(EditAnywhere, Category="Boomy type") int32 FxColorA = 0;
};

USTRUCT(BlueprintType)
struct FKyaActorCamera
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, Category="Boomy camera") float Fov = 0;
    UPROPERTY(EditAnywhere, Category="Boomy camera") FVector Position = FVector::ZeroVector;
    UPROPERTY(EditAnywhere, Category="Boomy camera") FVector LookAt = FVector::ZeroVector;
    UPROPERTY(EditAnywhere, Category="Boomy camera") FVector Up = FVector::ZeroVector;
    UPROPERTY(EditAnywhere, Category="Boomy camera") int32 SectorId = -1;
    UPROPERTY(EditAnywhere, Category="Boomy camera") int32 Field08 = -1;
    UPROPERTY(EditAnywhere, Category="Boomy camera") int32 Flags = 0;
    UPROPERTY(EditAnywhere, Category="Boomy camera") FVector TargetOffset = FVector::ZeroVector;
    UPROPERTY(VisibleAnywhere, Category="Boomy camera") bool bHasSwitch = false;
    UPROPERTY(EditAnywhere, Category="Boomy camera", meta=(EditCondition="bHasSwitch")) int32 SwitchActor = -1;
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Boomy camera", meta=(EditCondition="bHasSwitch")) TArray<int32> SwitchActorsA;
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Boomy camera", meta=(EditCondition="bHasSwitch")) TArray<int32> SwitchActorsB;
    UPROPERTY(EditAnywhere, Category="Boomy camera", meta=(EditCondition="bHasSwitch")) float SwitchValueA = 0;
    UPROPERTY(EditAnywhere, Category="Boomy camera", meta=(EditCondition="bHasSwitch")) int32 SwitchModeA = 0;
    UPROPERTY(EditAnywhere, Category="Boomy camera", meta=(EditCondition="bHasSwitch")) float SwitchValueB = 0;
    UPROPERTY(EditAnywhere, Category="Boomy camera", meta=(EditCondition="bHasSwitch")) int32 SwitchModeB = 0;
    UPROPERTY(EditAnywhere, Category="Boomy camera", meta=(EditCondition="bHasSwitch")) float SwitchValueC = 0;
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Boomy camera") TArray<FKyaCameraCondition> Conditions;
    UPROPERTY(EditAnywhere, Category="Boomy camera") int32 ConfigFlags = 0;
    UPROPERTY(EditAnywhere, Category="Boomy camera") int32 ConfigActorRefA = -1;
    UPROPERTY(EditAnywhere, Category="Boomy camera") int32 ConfigActorRefB = -1;
    UPROPERTY(EditAnywhere, Category="Boomy camera") float ConfigAngle = 0;
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Boomy camera") TArray<float> ConfigOptionalValues;
    UPROPERTY(EditAnywhere, Category="Boomy camera") FVector PitchAngles = FVector::ZeroVector;
    UPROPERTY(EditAnywhere, Category="Boomy camera") FVector OtherAngles = FVector::ZeroVector;
    UPROPERTY(EditAnywhere, Category="Boomy camera") FVector Distances = FVector::ZeroVector;
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Boomy camera") TArray<float> ExtraValues;
};

USTRUCT(BlueprintType)
struct FKyaTeleporterDestination
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, Category="Teleporter") int32 LevelId = 0;
    UPROPERTY(EditAnywhere, Category="Teleporter") int32 ElevatorId = 0;
    UPROPERTY(EditAnywhere, Category="Teleporter") int32 Field8 = 0;
};

USTRUCT(BlueprintType)
struct FKyaTeleporterWaypoint
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, Category="Teleporter") int32 ActorRef = -1;
    UPROPERTY(EditAnywhere, Category="Teleporter") int32 WaypointRef = -1;
};

USTRUCT(BlueprintType)
struct FKyaTeleporterOperation
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, Category="Teleporter condition") int32 ScenaricVariable = 0;
    UPROPERTY(EditAnywhere, Category="Teleporter condition") int32 Operation = 0;
    UPROPERTY(EditAnywhere, Category="Teleporter condition") int32 Value = 0;
};

USTRUCT(BlueprintType)
struct FKyaTeleporterCondition
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, Category="Teleporter condition") int32 Combine = 0;
    UPROPERTY(EditAnywhere, Category="Teleporter condition") int32 Variable = 0;
    UPROPERTY(EditAnywhere, Category="Teleporter condition") int32 Comparison = 0;
    UPROPERTY(EditAnywhere, Category="Teleporter condition") int32 Value = 0;
};

USTRUCT(BlueprintType)
struct FKyaTeleporterBlock
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Teleporter condition") TArray<FKyaTeleporterOperation> Operations;
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Teleporter condition") TArray<FKyaTeleporterCondition> Conditions;
};

USTRUCT(BlueprintType)
struct FKyaCommanderComponent
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, Category="Commander") int32 Field0 = 0;
    UPROPERTY(EditAnywhere, Category="Commander") int32 Field8 = 0;
};

USTRUCT(BlueprintType)
struct FKyaFighterStage
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, Category="Fighter") int32 AnimId = 0;
    UPROPERTY(EditAnywhere, Category="Fighter") float Field4 = 0;
    UPROPERTY(EditAnywhere, Category="Fighter") float Field8 = 0;
    UPROPERTY(EditAnywhere, Category="Fighter") FVector Field10 = FVector::ZeroVector;
    UPROPERTY(EditAnywhere, Category="Fighter") int32 Flags = 0;
};

USTRUCT(BlueprintType)
struct FKyaFighterBlowSubObj
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, Category="Fighter blow") int32 BoneA = 0;
    UPROPERTY(EditAnywhere, Category="Fighter blow") int32 BoneB = 0;
    UPROPERTY(EditAnywhere, Category="Fighter blow") int32 MaterialId = 0;
    UPROPERTY(EditAnywhere, Category="Fighter blow") int32 Field14 = 0;
    UPROPERTY(EditAnywhere, Category="Fighter blow") int32 Flags = 0;
    UPROPERTY(EditAnywhere, Category="Fighter blow") float Field1C = 0;
    UPROPERTY(EditAnywhere, Category="Fighter blow") int32 Field20 = 0;
};

USTRUCT(BlueprintType)
struct FKyaFighterBlow
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, Category="Fighter blow") FString Name;
    UPROPERTY(VisibleAnywhere, Category="Fighter blow") int32 NameCapacity = 0;
    UPROPERTY(EditAnywhere, Category="Fighter blow") int32 Field4A = 0;
    UPROPERTY(EditAnywhere, Category="Fighter blow") int32 Field4B = 0;
    UPROPERTY(EditAnywhere, Category="Fighter blow") int32 Field4C = 0;
    UPROPERTY(EditAnywhere, Category="Fighter blow") float Field8 = 0;
    UPROPERTY(EditAnywhere, Category="Fighter blow") float FieldC = 0;
    UPROPERTY(EditAnywhere, Category="Fighter blow") float Field10 = 0;
    UPROPERTY(EditAnywhere, Category="Fighter blow") float Damage = 0;
    UPROPERTY(EditAnywhere, Category="Fighter blow") float Field18 = 0;
    UPROPERTY(EditAnywhere, Category="Fighter blow") FVector Field20 = FVector::ZeroVector;
    UPROPERTY(EditAnywhere, Category="Fighter blow") FVector Field30 = FVector::ZeroVector;
    UPROPERTY(EditAnywhere, Category="Fighter blow") int32 Field40 = 0;
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Fighter blow") TArray<FKyaFighterBlowSubObj> SubObjs;
    UPROPERTY(EditAnywhere, Category="Fighter blow") int32 BoneRefA = 0;
    UPROPERTY(EditAnywhere, Category="Fighter blow") int32 BoneRefB = 0;
    UPROPERTY(EditAnywhere, Category="Fighter blow") float CanActivateRange = 0;
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Fighter blow") TArray<FKyaFighterStage> Stages;
};

USTRUCT(BlueprintType)
struct FKyaFighterGrab
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, Category="Fighter grab") FString Name;
    UPROPERTY(VisibleAnywhere, Category="Fighter grab") int32 NameCapacity = 0;
    UPROPERTY(EditAnywhere, Category="Fighter grab") int32 Field4A = 0;
    UPROPERTY(EditAnywhere, Category="Fighter grab") int32 Field4B = 0;
    UPROPERTY(EditAnywhere, Category="Fighter grab") int32 Field4C = 0;
    UPROPERTY(EditAnywhere, Category="Fighter grab") float Field8 = 0;
    UPROPERTY(EditAnywhere, Category="Fighter grab") float FieldC = 0;
    UPROPERTY(EditAnywhere, Category="Fighter grab") float Field10 = 0;
    UPROPERTY(EditAnywhere, Category="Fighter grab") int32 Field14 = 0;
    UPROPERTY(EditAnywhere, Category="Fighter grab") FVector Field20 = FVector::ZeroVector;
    UPROPERTY(EditAnywhere, Category="Fighter grab") float Field30 = 0;
    UPROPERTY(EditAnywhere, Category="Fighter grab") float Field34 = 0;
    UPROPERTY(EditAnywhere, Category="Fighter grab") int32 Field38 = 0;
    UPROPERTY(EditAnywhere, Category="Fighter grab") float Field3C = 0;
    UPROPERTY(EditAnywhere, Category="Fighter grab") float Field60 = 0;
    UPROPERTY(EditAnywhere, Category="Fighter grab") float Field40 = 0;
    UPROPERTY(EditAnywhere, Category="Fighter grab") float Field44 = 0;
    UPROPERTY(EditAnywhere, Category="Fighter grab") float Field48 = 0;
    UPROPERTY(EditAnywhere, Category="Fighter grab") FVector Field50 = FVector::ZeroVector;
    UPROPERTY(EditAnywhere, Category="Fighter grab") int32 Field68 = 0;
    UPROPERTY(EditAnywhere, Category="Fighter grab") int32 Skip = 0;
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Fighter grab") TArray<FKyaFighterStage> Stages;
};

USTRUCT(BlueprintType)
struct FKyaFighterGrabReact
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, Category="Fighter grab") FString Name;
    UPROPERTY(VisibleAnywhere, Category="Fighter grab") int32 NameCapacity = 0;
    UPROPERTY(EditAnywhere, Category="Fighter grab") int32 Field4 = 0;
    UPROPERTY(EditAnywhere, Category="Fighter grab") int32 Field8 = 0;
};

USTRUCT(BlueprintType)
struct FKyaFighterCombo
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, Category="Fighter combo") FString Name;
    UPROPERTY(VisibleAnywhere, Category="Fighter combo") int32 NameCapacity = 0;
    UPROPERTY(EditAnywhere, Category="Fighter combo") int32 Field4A = 0;
    UPROPERTY(EditAnywhere, Category="Fighter combo") int32 Field4B = 0;
    UPROPERTY(EditAnywhere, Category="Fighter combo") float Field8 = 0;
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Fighter combo") TArray<int32> Inputs;
    UPROPERTY(EditAnywhere, Category="Fighter combo") int32 Field2 = 0;
    UPROPERTY(EditAnywhere, Category="Fighter combo") int32 Field3 = 0;
    UPROPERTY(EditAnywhere, Category="Fighter combo") float FieldC = 0;
    UPROPERTY(EditAnywhere, Category="Fighter combo") FString Action;
    UPROPERTY(VisibleAnywhere, Category="Fighter combo") int32 ActionCapacity = 0;
    UPROPERTY(EditAnywhere, Category="Fighter combo") TArray<FString> Branches;
    UPROPERTY(VisibleAnywhere, Category="Fighter combo") TArray<int32> BranchCapacities;
};

USTRUCT(BlueprintType)
struct FKyaWolfenCapabilityItem
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Wolfen fight") TArray<int32> Chains;
    UPROPERTY(EditAnywhere, Category="Wolfen fight") float Field0 = 0;
    UPROPERTY(EditAnywhere, Category="Wolfen fight") float Field4 = 0;
};

USTRUCT(BlueprintType)
struct FKyaWolfenCapability
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Wolfen fight") TArray<FKyaWolfenCapabilityItem> Items;
};

USTRUCT(BlueprintType)
struct FKyaWolfenFightParam
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Wolfen fight") TArray<int32> Capability0Fields;
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Wolfen fight") TArray<int32> Capability2Fields;
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Wolfen fight") TArray<float> ConfigurationValues;
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Wolfen fight") TArray<FKyaWolfenCapability> Capabilities;
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Wolfen fight") TArray<int32> ComboList1;
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Wolfen fight") TArray<int32> ComboList2;
    UPROPERTY(EditAnywhere, Category="Wolfen fight") int32 FieldAfterLists = 0;
    UPROPERTY(EditAnywhere, Category="Wolfen fight") int32 FieldB44 = 0;
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Wolfen fight") TArray<float> TuningValues;
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Wolfen fight") TArray<int32> Knowledge;
};

USTRUCT(BlueprintType)
struct FKyaSwitchTarget
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, Category="Switch") int32 TargetRef = 0;
    UPROPERTY(EditAnywhere, Category="Switch") int32 CutsceneId = 0;
    UPROPERTY(EditAnywhere, Category="Switch") int32 Flags = 0;
    UPROPERTY(EditAnywhere, Category="Switch") int32 MessageId = 0;
    UPROPERTY(EditAnywhere, Category="Switch") int32 MessageFlags = 0;
    UPROPERTY(EditAnywhere, Category="Switch") int32 Field14 = 0;
    UPROPERTY(EditAnywhere, Category="Switch") int32 Field18 = 0;
};

USTRUCT(BlueprintType)
struct FKyaStreamEventCamera
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, Category="Camera event") int32 CameraIndex = 0;
    UPROPERTY(EditAnywhere, Category="Camera event") int32 Flags = 0;
    UPROPERTY(EditAnywhere, Category="Camera event") float Field8 = 0;
    UPROPERTY(EditAnywhere, Category="Camera event") float FieldC = 0;
    UPROPERTY(EditAnywhere, Category="Camera event") float Field10 = 0;
    UPROPERTY(EditAnywhere, Category="Camera event") int32 ActorRef = 0;
    UPROPERTY(EditAnywhere, Category="Camera event") int32 ZoneRef = 0;
    UPROPERTY(EditAnywhere, Category="Camera event") float Field1C = 0;
};

/** S_NTF_SWITCH: switch target list followed by one camera event. */
USTRUCT(BlueprintType)
struct FKyaSwitch
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Switch") TArray<FKyaSwitchTarget> Targets;
    UPROPERTY(EditAnywhere, Category="Switch") FKyaStreamEventCamera Camera;
};

USTRUCT(BlueprintType)
struct FKyaCompanionAlert
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Companion alert") TArray<FKyaScenarioCondition> Conditions;
    UPROPERTY(EditAnywhere, Category="Companion alert") int32 Flags = 0;
    UPROPERTY(EditAnywhere, Category="Companion alert") int32 Field08 = 0;
    UPROPERTY(EditAnywhere, Category="Companion alert") int32 Field10 = 0;
    UPROPERTY(EditAnywhere, Category="Companion alert") int32 Field14 = 0;
    UPROPERTY(EditAnywhere, Category="Companion alert") float Field18 = 0;
    UPROPERTY(EditAnywhere, Category="Companion alert") int32 ActorRef = -1;
    UPROPERTY(EditAnywhere, Category="Companion alert") FVector Field20 = FVector::ZeroVector;
    UPROPERTY(EditAnywhere, Category="Companion alert") int32 CameraRef = -1;
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Companion alert") TArray<int32> Field34;
    UPROPERTY(EditAnywhere, Category="Companion alert") int64 LevelStringKey = 0;
    UPROPERTY(EditAnywhere, Category="Companion alert") float Field50 = 0;
    UPROPERTY(EditAnywhere, Category="Companion alert") int32 Field58 = -1;
    UPROPERTY(EditAnywhere, Category="Companion alert") float Field5C = 0;
    UPROPERTY(EditAnywhere, Category="Companion alert") FKyaSwitch TargetSwitch;
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Companion alert") TArray<FKyaConditionedOperation> Field70;
};

USTRUCT(BlueprintType)
struct FKyaAnalogTarget
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, Category="Platform behaviour") int32 ActorRef = -1;
    UPROPERTY(EditAnywhere, Category="Platform behaviour") int32 CutsceneId = 0;
    UPROPERTY(EditAnywhere, Category="Platform behaviour") int32 Flags = 0;
    UPROPERTY(EditAnywhere, Category="Platform behaviour") int32 Field0C = 0;
    UPROPERTY(EditAnywhere, Category="Platform behaviour") int32 Field10 = 0;
    UPROPERTY(EditAnywhere, Category="Platform behaviour") int32 Field14 = 0;
    UPROPERTY(EditAnywhere, Category="Platform behaviour") int32 Field18 = 0;
    UPROPERTY(EditAnywhere, Category="Platform behaviour") int32 Field1C = 0;
    UPROPERTY(EditAnywhere, Category="Platform behaviour") float Field20 = 0;
    UPROPERTY(EditAnywhere, Category="Platform behaviour") float Field24 = 0;
};

USTRUCT(BlueprintType)
struct FKyaSwitchExTarget
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, Category="Platform behaviour") int32 ActorRef = -1;
    UPROPERTY(EditAnywhere, Category="Platform behaviour") int32 CutsceneId = 0;
    UPROPERTY(EditAnywhere, Category="Platform behaviour") int32 Flags = 0;
    UPROPERTY(EditAnywhere, Category="Platform behaviour") int32 MessageId = 0;
};

USTRUCT(BlueprintType)
struct FKyaPlatformBehaviour
{
    GENERATED_BODY()
    /** Behaviour 7 stand: sound reference and field 0x0c. */
    UPROPERTY(EditAnywhere, Category="Platform behaviour") int32 SoundRef = -1;
    UPROPERTY(EditAnywhere, Category="Platform behaviour") int32 Field0C = 0;
    /** Behaviours 4/6: CPathFollowReaderAbsolute path index and timing. */
    UPROPERTY(EditAnywhere, Category="Platform behaviour") int32 PathIndex = -1;
    UPROPERTY(EditAnywhere, Category="Platform behaviour") int32 Timing = 0;
    /** Behaviour 6: reader time and total traversal time. */
    UPROPERTY(EditAnywhere, Category="Platform behaviour") float PathField4 = 0;
    UPROPERTY(EditAnywhere, Category="Platform behaviour") float PathTotalTime = 0;
    /** Behaviour 2 trajectory. */
    UPROPERTY(EditAnywhere, Category="Platform behaviour") float SegmentStartTime = 0;
    UPROPERTY(EditAnywhere, Category="Platform behaviour") float LoopDelay = 0;
    UPROPERTY(EditAnywhere, Category="Platform behaviour") int32 Field40 = 0;
    UPROPERTY(EditAnywhere, Category="Platform behaviour") int32 AltModelId = -1;
    /** Behaviour 9 selector slave. */
    UPROPERTY(EditAnywhere, Category="Platform behaviour") int32 Field18 = 0;
    UPROPERTY(EditAnywhere, Category="Platform behaviour") int32 MaterialId = -1;
    UPROPERTY(EditAnywhere, Category="Platform behaviour") float Field8 = 0;
    UPROPERTY(EditAnywhere, Category="Platform behaviour") float FieldC = 0;
    UPROPERTY(EditAnywhere, Category="Platform behaviour") float Field10 = 0;
    /** Behaviour 3 slab. */
    UPROPERTY(EditAnywhere, Category="Platform behaviour") FKyaSwitch SlabSwitch;
    UPROPERTY(EditAnywhere, Category="Platform behaviour") float SlabField10 = 0;
    UPROPERTY(EditAnywhere, Category="Platform behaviour") float SlabField14 = 0;
    UPROPERTY(EditAnywhere, Category="Platform behaviour") float SlabField18 = 0;
    UPROPERTY(EditAnywhere, Category="Platform behaviour") float SlabField1C = 0;
    UPROPERTY(EditAnywhere, Category="Platform behaviour") int32 SlabField20 = 0;
    /** Behaviour 5 weighing-master. */
    UPROPERTY(EditAnywhere, Category="Platform behaviour") int32 MasterPathIndex = -1;
    UPROPERTY(EditAnywhere, Category="Platform behaviour") int32 MasterTiming = 0;
    UPROPERTY(EditAnywhere, Category="Platform behaviour") int32 MasterStreamActor = -1;
    UPROPERTY(EditAnywhere, Category="Platform behaviour") float MasterField40 = 0;
    UPROPERTY(EditAnywhere, Category="Platform behaviour") float MasterField44 = 0;
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Platform behaviour") TArray<FKyaAnalogTarget> AnalogTargets;
    UPROPERTY(EditAnywhere, Category="Platform behaviour") FKyaStreamEventCamera AnalogCamera;
    /** Behaviour 8 selector master (shares the selector-slave values above). */
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Platform behaviour") TArray<int32> SelectorActorRefs;
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Platform behaviour") TArray<FKyaSwitchExTarget> SelectorSwitchEx;
    UPROPERTY(EditAnywhere, Category="Platform behaviour") FKyaStreamEventCamera SelectorCamera;
};

USTRUCT(BlueprintType)
struct FKyaProjectileBehaviour
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, Category="Projectile behaviour") int32 BehaviourId = 0;
    UPROPERTY(EditAnywhere, Category="Projectile behaviour") int32 Field0C = 0;
    UPROPERTY(EditAnywhere, Category="Projectile behaviour") int32 MaterialId = -1;
    UPROPERTY(EditAnywhere, Category="Projectile behaviour") float Speed = 0;
    UPROPERTY(EditAnywhere, Category="Projectile behaviour") float Acceleration = 0;
    UPROPERTY(EditAnywhere, Category="Projectile behaviour") float Field18 = 0;
    UPROPERTY(EditAnywhere, Category="Projectile behaviour") float Field10 = 0;
    UPROPERTY(EditAnywhere, Category="Projectile behaviour") int32 ActorRef = -1;
};

USTRUCT(BlueprintType)
struct FKyaSwitchBehaviour
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, Category="Switch behaviour") int32 LeverRefA = 0;
    UPROPERTY(EditAnywhere, Category="Switch behaviour") int32 LeverRefB = 0;
    UPROPERTY(EditAnywhere, Category="Switch behaviour") float MagicField08 = 0;
    UPROPERTY(EditAnywhere, Category="Switch behaviour") float MagicField10 = 0;
    UPROPERTY(EditAnywhere, Category="Switch behaviour") float MagicField14 = 0;
    UPROPERTY(EditAnywhere, Category="Switch behaviour") int32 MagicFxDigits = 0;
    UPROPERTY(EditAnywhere, Category="Switch behaviour") int32 MagicField4C = 0;
    UPROPERTY(EditAnywhere, Category="Switch behaviour") int32 MagicField18 = 0;
    UPROPERTY(EditAnywhere, Category="Switch behaviour") int32 MagicField28 = 0;
    UPROPERTY(EditAnywhere, Category="Switch behaviour") int32 MagicField24 = 0;
    UPROPERTY(EditAnywhere, Category="Switch behaviour") int32 MagicField30 = 0;
    UPROPERTY(EditAnywhere, Category="Switch behaviour") int32 OpenTextureId = -1;
    UPROPERTY(EditAnywhere, Category="Switch behaviour") int32 OpenMeshId = -1;
    UPROPERTY(EditAnywhere, Category="Switch behaviour") FKyaStreamEventCamera MagicCamera;
    UPROPERTY(EditAnywhere, Category="Switch behaviour") int32 MultiField14 = 0;
    UPROPERTY(EditAnywhere, Category="Switch behaviour") int32 MultiField08 = 0;
    UPROPERTY(EditAnywhere, Category="Switch behaviour") int32 MultiField0C = 0;
    UPROPERTY(EditAnywhere, Category="Switch behaviour") float TargetDisplacement = 0;
    UPROPERTY(EditAnywhere, Category="Switch behaviour") float TargetSpringStrength = 0;
    UPROPERTY(EditAnywhere, Category="Switch behaviour") float TargetDamping = 0;
};

USTRUCT(BlueprintType)
struct FKyaTrackSubObj
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, Category="Wolfen behaviour") int32 Field0 = 0;
    UPROPERTY(EditAnywhere, Category="Wolfen behaviour") int32 AField0 = 0;
    UPROPERTY(EditAnywhere, Category="Wolfen behaviour") float AField4 = 0;
    UPROPERTY(EditAnywhere, Category="Wolfen behaviour") float AField8 = 0;
    UPROPERTY(EditAnywhere, Category="Wolfen behaviour") int32 Field10 = 0;
};

USTRUCT(BlueprintType)
struct FKyaSwitchBehaviourRecord
{
    GENERATED_BODY()
    /** 0 = message, 1 = see-actor-in-area, 2 = actor-in-area, 3 = see-actor. */
    UPROPERTY(EditAnywhere, Category="Wolfen switch") int32 Category = 0;
    UPROPERTY(EditAnywhere, Category="Wolfen switch") int32 MessageType = 0;
    UPROPERTY(EditAnywhere, Category="Wolfen switch") int32 ActorRef = -1;
    UPROPERTY(EditAnywhere, Category="Wolfen switch") int32 ZoneRef = -1;
    UPROPERTY(EditAnywhere, Category="Wolfen switch") int32 Flags = 0;
    UPROPERTY(EditAnywhere, Category="Wolfen switch") int32 SeeActorRef = -1;
    UPROPERTY(EditAnywhere, Category="Wolfen switch") int32 BehaviourId = 0;
};

USTRUCT(BlueprintType)
struct FKyaWolfenBehaviour
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, Category="Wolfen behaviour") int32 Flags = 0;
    /** Behaviour 15 track: notification-target field 0. */
    UPROPERTY(EditAnywhere, Category="Wolfen behaviour") float TrackField = 0;
    /** Behaviour 23 DCA. */
    UPROPERTY(EditAnywhere, Category="Wolfen behaviour") int32 DcaActorRef = -1;
    UPROPERTY(EditAnywhere, Category="Wolfen behaviour") float DcaShootCooldown = 0;
    /** Behaviour 24 avoid. */
    UPROPERTY(EditAnywhere, Category="Wolfen behaviour") float AvoidField80 = 0;
    UPROPERTY(EditAnywhere, Category="Wolfen behaviour") float AvoidField84 = 0;
    /** Behaviours 17/18/19 track weapon. */
    UPROPERTY(EditAnywhere, Category="Wolfen behaviour") float WeaponField90 = 0;
    UPROPERTY(EditAnywhere, Category="Wolfen behaviour") int32 WeaponField94 = 0;
    UPROPERTY(EditAnywhere, Category="Wolfen behaviour") int32 WeaponField98 = 0;
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Wolfen behaviour") TArray<FKyaTrackSubObj> WeaponSubObjs;
    /** Behaviours 17/19: trailing field 0xf0 (and 17's notification field). */
    UPROPERTY(EditAnywhere, Category="Wolfen behaviour") float WeaponFieldF0 = 0;
    UPROPERTY(EditAnywhere, Category="Wolfen behaviour") float WeaponTargetField = 0;
    /** Behaviour 22 lost: CSwitchBehaviour entries. */
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Wolfen behaviour") TArray<FKyaSwitchBehaviourRecord> SwitchRecords;
    /** Behaviour 8 watchdog. */
    UPROPERTY(EditAnywhere, Category="Wolfen behaviour") int32 Field80 = 0;
    UPROPERTY(EditAnywhere, Category="Wolfen behaviour") int32 TrackBehaviourId = -1;
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Wolfen behaviour") TArray<FKyaSwitchTarget> OnOffTargets;
    UPROPERTY(EditAnywhere, Category="Wolfen behaviour") FKyaStreamEventCamera OnOffCamera;
    /** Behaviour 21 snipe. */
    UPROPERTY(EditAnywhere, Category="Wolfen behaviour") int32 SnipeField90 = -1;
    UPROPERTY(EditAnywhere, Category="Wolfen behaviour") float SnipeField94 = 0;
    UPROPERTY(EditAnywhere, Category="Wolfen behaviour") float SnipeField98 = 0;
    UPROPERTY(EditAnywhere, Category="Wolfen behaviour") float SnipeField9C = 0;
    UPROPERTY(EditAnywhere, Category="Wolfen behaviour") float SnipeFieldA0 = 0;
    UPROPERTY(EditAnywhere, Category="Wolfen behaviour") int32 SpotTrackBehaviourId = -1;
    UPROPERTY(EditAnywhere, Category="Wolfen behaviour") int32 SnipeTrackBehaviourId = -1;
    UPROPERTY(EditAnywhere, Category="Wolfen behaviour") int32 ShadowMaterialId = -1;
    UPROPERTY(EditAnywhere, Category="Wolfen behaviour") int32 SnipeFieldAC = -1;
    UPROPERTY(EditAnywhere, Category="Wolfen behaviour") float SnipeFieldB0 = 0;
    UPROPERTY(EditAnywhere, Category="Wolfen behaviour") float SnipeFieldB4 = 0;
    UPROPERTY(EditAnywhere, Category="Wolfen behaviour") int32 NbSpot = 0;
    UPROPERTY(EditAnywhere, Category="Wolfen behaviour") int32 NbShadow = 0;
    UPROPERTY(EditAnywhere, Category="Wolfen behaviour") float RotationSpeed = 0;
    UPROPERTY(EditAnywhere, Category="Wolfen behaviour") int32 SnipeFieldB8 = 0;
    /** Behaviour 10 guard area. */
    UPROPERTY(EditAnywhere, Category="Wolfen behaviour") int32 GuardPathIndex = -1;
    UPROPERTY(EditAnywhere, Category="Wolfen behaviour") float GuardField98 = 0;
    UPROPERTY(EditAnywhere, Category="Wolfen behaviour") int32 GuardTrackBehaviourId = -1;
    UPROPERTY(EditAnywhere, Category="Wolfen behaviour") int32 GuardField90 = 0;
    /** Behaviour 13 escape. */
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Wolfen behaviour") TArray<int32> EscapePathIndices;
    UPROPERTY(EditAnywhere, Category="Wolfen behaviour") int32 EscapeField88 = 0;
    UPROPERTY(EditAnywhere, Category="Wolfen behaviour") float EscapeField9C = 0;
    UPROPERTY(EditAnywhere, Category="Wolfen behaviour") float EscapeField90 = 0;
    UPROPERTY(EditAnywhere, Category="Wolfen behaviour") float EscapeField8C = 0;
    UPROPERTY(EditAnywhere, Category="Wolfen behaviour") float EscapeField94 = 0;
    UPROPERTY(EditAnywhere, Category="Wolfen behaviour") float EscapeField98 = 0;
    UPROPERTY(EditAnywhere, Category="Wolfen behaviour") int32 EscapeFieldA0 = 0;
    /** Behaviour 14 exorcism. */
    UPROPERTY(EditAnywhere, Category="Wolfen behaviour") int32 ExorcismDigitMaterialId = -1;
    UPROPERTY(EditAnywhere, Category="Wolfen behaviour") int32 ExorcismField24 = 0;
    UPROPERTY(EditAnywhere, Category="Wolfen behaviour") int32 ExorcismTextureA = -1;
    UPROPERTY(EditAnywhere, Category="Wolfen behaviour") int32 ExorcismMeshA = -1;
    UPROPERTY(EditAnywhere, Category="Wolfen behaviour") int32 ExorcismTextureB = -1;
    UPROPERTY(EditAnywhere, Category="Wolfen behaviour") int32 ExorcismMeshB = -1;
    UPROPERTY(EditAnywhere, Category="Wolfen behaviour") int32 ExorcismField18 = 0;
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Wolfen behaviour") TArray<int32> ExorcismWords;
};

USTRUCT(BlueprintType)
struct FKyaHeroUnknownBehaviour
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, Category="Hero behaviour") int32 FxTypeA = 0;
    UPROPERTY(EditAnywhere, Category="Hero behaviour") int32 TextureA = -1;
    UPROPERTY(EditAnywhere, Category="Hero behaviour") int32 MeshA = -1;
    UPROPERTY(EditAnywhere, Category="Hero behaviour") int32 FxTypeB = 0;
    UPROPERTY(EditAnywhere, Category="Hero behaviour") int32 FxTypeC = 0;
    UPROPERTY(EditAnywhere, Category="Hero behaviour") int32 TextureB = -1;
    UPROPERTY(EditAnywhere, Category="Hero behaviour") int32 MeshB = -1;
};

USTRUCT(BlueprintType)
struct FKyaInventoryInfo
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, Category="Inventory") int32 Field0 = 0;
    UPROPERTY(EditAnywhere, Category="Inventory") int64 Field8 = 0;
    UPROPERTY(EditAnywhere, Category="Inventory") int64 Field10 = 0;
    UPROPERTY(EditAnywhere, Category="Inventory") int32 PurchaseId = 0;
    UPROPERTY(EditAnywhere, Category="Inventory") int32 Field28 = 0;
    UPROPERTY(EditAnywhere, Category="Inventory") int32 MoneyCost = 0;
    UPROPERTY(EditAnywhere, Category="Inventory") int32 Field2C = 0;
    UPROPERTY(EditAnywhere, Category="Inventory") int32 ObjectId = 0;
    UPROPERTY(EditAnywhere, Category="Inventory") int64 Field18 = 0;
    UPROPERTY(EditAnywhere, Category="Inventory") int32 InitialBehaviourId = 0;
};

USTRUCT(BlueprintType)
struct FKyaHeroTutorialZone
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, Category="Hero tutorial") int32 ZoneRef = -1;
    UPROPERTY(EditAnywhere, Category="Hero tutorial") int64 MessageKey = 0;
    UPROPERTY(EditAnywhere, Category="Hero tutorial") int32 Flags = 0;
    UPROPERTY(EditAnywhere, Category="Hero tutorial") int32 Action = 0;
    UPROPERTY(EditAnywhere, Category="Hero tutorial") float Duration = 0;
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Hero tutorial") TArray<FKyaSwitchTarget> Targets;
    UPROPERTY(EditAnywhere, Category="Hero tutorial") FKyaStreamEventCamera Camera;
};

USTRUCT(BlueprintType)
struct FKyaHeroProjectileZone
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, Category="Hero projectile") int32 ZoneRef = -1;
    UPROPERTY(EditAnywhere, Category="Hero projectile") int32 ProjectileType = 0;
    UPROPERTY(EditAnywhere, Category="Hero projectile") float Damage = 0;
    UPROPERTY(EditAnywhere, Category="Hero projectile") int32 WaypointIndex = -1;
    UPROPERTY(EditAnywhere, Category="Hero projectile") float Force = 0;
    UPROPERTY(EditAnywhere, Category="Hero projectile") int32 SampleCount = 0;
    UPROPERTY(EditAnywhere, Category="Hero projectile") int32 SectorId = -1;
};

USTRUCT(BlueprintType)
struct FKyaHeroZonePair
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, Category="Hero boomy") int32 ZoneRef = -1;
    UPROPERTY(EditAnywhere, Category="Hero boomy") int32 Field4 = 0;
};

USTRUCT(BlueprintType)
struct FKyaActorBehaviour
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, Category="Behaviour") int32 Id = 0;
    /** Class-specific data, retained in its serialized form. */
    UPROPERTY(VisibleAnywhere, Category="Behaviour") bool bHasTypedClusterZones = false;
    UPROPERTY(VisibleAnywhere, Category="Behaviour") bool bHasTypedFxBehaviour = false;
    UPROPERTY(VisibleAnywhere, Category="Behaviour") bool bHasTypedTrapBehaviour = false;
    UPROPERTY(VisibleAnywhere, Category="Behaviour") bool bHasTypedWeaponBehaviour = false;
    UPROPERTY(VisibleAnywhere, Category="Behaviour") bool bHasTypedBonusBehaviour = false;
    UPROPERTY(VisibleAnywhere, Category="Behaviour") bool bHasTypedMickenBehaviour = false;
    UPROPERTY(VisibleAnywhere, Category="Behaviour") bool bHasTypedFruitBehaviour = false;
    UPROPERTY(VisibleAnywhere, Category="Behaviour") bool bHasTypedCommanderBehaviour = false;
    UPROPERTY(VisibleAnywhere, Category="Behaviour") bool bHasTypedNativBehaviour = false;
    UPROPERTY(VisibleAnywhere, Category="Behaviour") bool bHasTypedCinematicBehaviour = false;
    UPROPERTY(VisibleAnywhere, Category="Behaviour") bool bHasTypedDcaControlledBehaviour = false;
    UPROPERTY(VisibleAnywhere, Category="Behaviour") bool bHasTypedDcaTrajectoryBehaviour = false;
    UPROPERTY(VisibleAnywhere, Category="Behaviour") bool bHasTypedFogBehaviour = false;
    UPROPERTY(VisibleAnywhere, Category="Behaviour") bool bHasTypedShootFireBehaviour = false;
    UPROPERTY(VisibleAnywhere, Category="Behaviour") bool bHasTypedShootWaveBehaviour = false;
    UPROPERTY(VisibleAnywhere, Category="Behaviour") bool bHasTypedGravityFallBehaviour = false;
    UPROPERTY(VisibleAnywhere, Category="Behaviour") bool bHasTypedGravityBellBehaviour = false;
    UPROPERTY(VisibleAnywhere, Category="Behaviour") bool bHasTypedCompanionBehaviour = false;
    UPROPERTY(VisibleAnywhere, Category="Behaviour") bool bHasTypedPlatformBehaviour = false;
    UPROPERTY(EditAnywhere, Category="Platform behaviour", meta=(EditCondition="bHasTypedPlatformBehaviour")) FKyaPlatformBehaviour PlatformBehaviour;
    UPROPERTY(VisibleAnywhere, Category="Behaviour") bool bHasTypedProjectileBehaviour = false;
    UPROPERTY(EditAnywhere, Category="Projectile behaviour", meta=(EditCondition="bHasTypedProjectileBehaviour")) FKyaProjectileBehaviour ProjectileBehaviour;
    UPROPERTY(VisibleAnywhere, Category="Behaviour") bool bHasTypedSwitchBehaviour = false;
    UPROPERTY(EditAnywhere, Category="Switch behaviour", meta=(EditCondition="bHasTypedSwitchBehaviour")) FKyaSwitchBehaviour SwitchBehaviour;
    UPROPERTY(VisibleAnywhere, Category="Behaviour") bool bHasTypedWolfenBehaviour = false;
    UPROPERTY(VisibleAnywhere, Category="Behaviour") bool bHasTypedCatchByTrapBehaviour = false;
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Catch behaviour", meta=(EditCondition="bHasTypedCatchByTrapBehaviour")) TArray<FKyaConditionedOperation> CatchOperations;
    UPROPERTY(VisibleAnywhere, Category="Behaviour") bool bHasTypedHeroUnknownBehaviour = false;
    UPROPERTY(EditAnywhere, Category="Hero behaviour", meta=(EditCondition="bHasTypedHeroUnknownBehaviour")) FKyaHeroUnknownBehaviour HeroUnknownBehaviour;
    UPROPERTY(VisibleAnywhere, Category="Behaviour") bool bHasTypedInventoryBehaviour = false;
    UPROPERTY(EditAnywhere, Category="Inventory", meta=(EditCondition="bHasTypedInventoryBehaviour")) FKyaInventoryInfo InventoryInfo;
    UPROPERTY(EditAnywhere, Category="Wolfen behaviour", meta=(EditCondition="bHasTypedWolfenBehaviour")) FKyaWolfenBehaviour WolfenBehaviour;
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Companion alert", meta=(EditCondition="bHasTypedCompanionBehaviour")) TArray<FKyaCompanionAlert> CompanionAlerts;
    UPROPERTY(VisibleAnywhere, Category="Behaviour") bool bHasTypedBridgeBasicBehaviour = false;
    UPROPERTY(VisibleAnywhere, Category="Behaviour") bool bHasTypedBridgeWindBehaviour = false;
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Behaviour", meta=(EditCondition="!bHasTypedClusterZones && !bHasTypedFxBehaviour && !bHasTypedTrapBehaviour && !bHasTypedWeaponBehaviour && !bHasTypedBonusBehaviour && !bHasTypedMickenBehaviour && !bHasTypedFruitBehaviour && !bHasTypedCommanderBehaviour && !bHasTypedNativBehaviour && !bHasTypedCinematicBehaviour && !bHasTypedDcaControlledBehaviour && !bHasTypedDcaTrajectoryBehaviour && !bHasTypedFogBehaviour && !bHasTypedShootFireBehaviour && !bHasTypedShootWaveBehaviour && !bHasTypedGravityFallBehaviour && !bHasTypedGravityBellBehaviour && !bHasTypedBridgeBasicBehaviour && !bHasTypedBridgeWindBehaviour && !bHasTypedCompanionBehaviour && !bHasTypedPlatformBehaviour && !bHasTypedProjectileBehaviour && !bHasTypedSwitchBehaviour && !bHasTypedWolfenBehaviour && !bHasTypedCatchByTrapBehaviour && !bHasTypedHeroUnknownBehaviour && !bHasTypedInventoryBehaviour")) TArray<uint8> Data;
    /** Decoded for clusteriser class 0x18, behaviour 2. */
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Cluster zones", meta=(EditCondition="bHasTypedClusterZones")) TArray<FKyaClusterZone> ClusterZones;
    /** Decoded for FX class 0x2e behaviours 2 and 3 (and 4 as a generic integer list). */
    UPROPERTY(EditAnywhere, Category="FX behaviour", meta=(EditCondition="bHasTypedFxBehaviour")) int32 FxBehaviourValue = 0;
    UPROPERTY(EditAnywhere, Category="FX behaviour", meta=(EditCondition="bHasTypedFxBehaviour && Id == 3")) int32 FxPathFollowIndex = -1;
    UPROPERTY(EditAnywhere, EditFixedSize, Category="FX behaviour", meta=(EditCondition="bHasTypedFxBehaviour && Id == 4")) TArray<int32> FxUnknownValues;
    /** Trap behaviour 2: flags/default behaviour/catch effect; behaviour 3: message/flags/active behaviour. */
    UPROPERTY(EditAnywhere, Category="Trap behaviour", meta=(EditCondition="bHasTypedTrapBehaviour")) int32 TrapBehaviourField0 = 0;
    UPROPERTY(EditAnywhere, Category="Trap behaviour", meta=(EditCondition="bHasTypedTrapBehaviour")) int32 TrapBehaviourField4 = 0;
    UPROPERTY(EditAnywhere, Category="Trap behaviour", meta=(EditCondition="bHasTypedTrapBehaviour")) int32 TrapBehaviourField8 = 0;
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Weapon behaviour", meta=(EditCondition="bHasTypedWeaponBehaviour")) TArray<FKyaWeaponAdvancedEntry> WeaponBurst;
    UPROPERTY(EditAnywhere, Category="Weapon behaviour", meta=(EditCondition="bHasTypedWeaponBehaviour")) int32 WeaponField40 = 0;
    UPROPERTY(EditAnywhere, Category="Weapon behaviour", meta=(EditCondition="bHasTypedWeaponBehaviour")) int32 WeaponField44 = 0;
    UPROPERTY(EditAnywhere, Category="Weapon behaviour", meta=(EditCondition="bHasTypedWeaponBehaviour")) int32 WeaponSoundIndex = -1;
    UPROPERTY(EditAnywhere, Category="Weapon behaviour", meta=(EditCondition="bHasTypedWeaponBehaviour")) int32 WeaponPistolField2E0 = 0;
    UPROPERTY(EditAnywhere, Category="Weapon behaviour", meta=(EditCondition="bHasTypedWeaponBehaviour")) int32 WeaponPistolField2E4 = -1;
    UPROPERTY(EditAnywhere, Category="Weapon behaviour", meta=(EditCondition="bHasTypedWeaponBehaviour")) int32 WeaponSniperTextureIndex = 0;
    UPROPERTY(EditAnywhere, Category="Weapon behaviour", meta=(EditCondition="bHasTypedWeaponBehaviour")) int32 WeaponSniperMeshIndex = 0;
    UPROPERTY(EditAnywhere, Category="Weapon behaviour", meta=(EditCondition="bHasTypedWeaponBehaviour")) int32 WeaponSniperMaterialId = -1;
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Weapon behaviour", meta=(EditCondition="bHasTypedWeaponBehaviour")) TArray<int32> WeaponRpgActorRefs;
    UPROPERTY(EditAnywhere, Category="Bonus behaviour", meta=(EditCondition="bHasTypedBonusBehaviour")) float BonusField14 = 0;
    UPROPERTY(EditAnywhere, Category="Bonus behaviour", meta=(EditCondition="bHasTypedBonusBehaviour")) float BonusField1B8 = 0;
    UPROPERTY(EditAnywhere, Category="Bonus behaviour", meta=(EditCondition="bHasTypedBonusBehaviour")) float BonusField1B4 = 0;
    UPROPERTY(EditAnywhere, Category="Bonus behaviour", meta=(EditCondition="bHasTypedBonusBehaviour")) int32 BonusMaterialId = -1;
    UPROPERTY(EditAnywhere, Category="Bonus behaviour", meta=(EditCondition="bHasTypedBonusBehaviour")) float BonusField1B0 = 0;
    UPROPERTY(EditAnywhere, Category="Bonus behaviour", meta=(EditCondition="bHasTypedBonusBehaviour")) int32 BonusFxTailCount = 0;
    UPROPERTY(EditAnywhere, Category="Bonus behaviour", meta=(EditCondition="bHasTypedBonusBehaviour")) int32 BonusG2DIndex = -1;
    UPROPERTY(EditAnywhere, Category="Bonus behaviour", meta=(EditCondition="bHasTypedBonusBehaviour")) float BonusTurnValue = 0;
    UPROPERTY(EditAnywhere, Category="Bonus behaviour", meta=(EditCondition="bHasTypedBonusBehaviour")) int32 BonusFlockPathIndex = -1;
    UPROPERTY(EditAnywhere, Category="Bonus behaviour", meta=(EditCondition="bHasTypedBonusBehaviour")) float BonusAddOnField18 = 0;
    UPROPERTY(EditAnywhere, Category="Micken behaviour", meta=(EditCondition="bHasTypedMickenBehaviour")) float MickenEatField8 = 0;
    UPROPERTY(EditAnywhere, Category="Micken behaviour", meta=(EditCondition="bHasTypedMickenBehaviour")) float MickenEatFieldC = 0;
    UPROPERTY(EditAnywhere, Category="Micken behaviour", meta=(EditCondition="bHasTypedMickenBehaviour")) int32 MickenEatField10 = 0;
    UPROPERTY(EditAnywhere, Category="Micken behaviour", meta=(EditCondition="bHasTypedMickenBehaviour")) int32 MickenEatPathIndex = -1;
    UPROPERTY(EditAnywhere, Category="Fruit behaviour", meta=(EditCondition="bHasTypedFruitBehaviour")) float FruitSquashedValue = 0;
    UPROPERTY(EditAnywhere, Category="Fruit behaviour", meta=(EditCondition="bHasTypedFruitBehaviour")) int32 FruitBonusActorRef = -1;
    UPROPERTY(EditAnywhere, Category="Fruit behaviour", meta=(EditCondition="bHasTypedFruitBehaviour")) int32 FruitBonusInitialBehaviour = 0;
    UPROPERTY(EditAnywhere, Category="Fruit behaviour", meta=(EditCondition="bHasTypedFruitBehaviour")) int32 FruitInventoryField0 = 0;
    UPROPERTY(EditAnywhere, Category="Fruit behaviour", meta=(EditCondition="bHasTypedFruitBehaviour")) int64 FruitInventoryField8 = 0;
    UPROPERTY(EditAnywhere, Category="Fruit behaviour", meta=(EditCondition="bHasTypedFruitBehaviour")) int64 FruitInventoryField10 = 0;
    UPROPERTY(EditAnywhere, Category="Fruit behaviour", meta=(EditCondition="bHasTypedFruitBehaviour")) int32 FruitInventoryPurchaseId = 0;
    UPROPERTY(EditAnywhere, Category="Fruit behaviour", meta=(EditCondition="bHasTypedFruitBehaviour")) int32 FruitInventoryField28 = 0;
    UPROPERTY(EditAnywhere, Category="Fruit behaviour", meta=(EditCondition="bHasTypedFruitBehaviour")) int32 FruitInventoryMoneyCost = 0;
    UPROPERTY(EditAnywhere, Category="Fruit behaviour", meta=(EditCondition="bHasTypedFruitBehaviour")) int32 FruitInventoryField2C = 0;
    UPROPERTY(EditAnywhere, Category="Fruit behaviour", meta=(EditCondition="bHasTypedFruitBehaviour")) bool FruitInventoryHasObject = false;
    UPROPERTY(EditAnywhere, Category="Fruit behaviour", meta=(EditCondition="bHasTypedFruitBehaviour")) int32 FruitInventoryObjectId = 0;
    UPROPERTY(EditAnywhere, Category="Fruit behaviour", meta=(EditCondition="bHasTypedFruitBehaviour")) int64 FruitInventoryField18 = 0;
    UPROPERTY(EditAnywhere, Category="Fruit behaviour", meta=(EditCondition="bHasTypedFruitBehaviour")) int32 FruitInventoryInitialBehaviour = 0;
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Commander behaviour", meta=(EditCondition="bHasTypedCommanderBehaviour")) TArray<int32> CommanderSquadConfig;
    UPROPERTY(EditAnywhere, Category="Commander behaviour", meta=(EditCondition="bHasTypedCommanderBehaviour")) FKyaSwitch CommanderSwitch;
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Nativ behaviour", meta=(EditCondition="bHasTypedNativBehaviour")) TArray<FKyaNativTrajectory> NativTrajectories;
    /** CBehaviourCinematic serialized leave-behaviour id. */
    UPROPERTY(EditAnywhere, Category="Cinematic behaviour", meta=(EditCondition="bHasTypedCinematicBehaviour")) int32 CinematicLeaveBehaviourId = -1;
    /** CActorDCA::CBhvControlled tuning. */
    UPROPERTY(EditAnywhere, Category="DCA behaviour", meta=(EditCondition="bHasTypedDcaControlledBehaviour")) float DcaControlledField0 = 0;
    UPROPERTY(EditAnywhere, Category="DCA behaviour", meta=(EditCondition="bHasTypedDcaControlledBehaviour")) float DcaControlledField4 = 0;
    UPROPERTY(EditAnywhere, Category="DCA behaviour", meta=(EditCondition="bHasTypedDcaControlledBehaviour")) float DcaControlledField8 = 0;
    UPROPERTY(EditAnywhere, Category="DCA behaviour", meta=(EditCondition="bHasTypedDcaControlledBehaviour")) float DcaControlledFovDegrees = 0;
    UPROPERTY(EditAnywhere, Category="DCA behaviour", meta=(EditCondition="bHasTypedDcaControlledBehaviour")) FVector DcaControlledVector10 = FVector::ZeroVector;
    UPROPERTY(EditAnywhere, Category="DCA behaviour", meta=(EditCondition="bHasTypedDcaControlledBehaviour")) FVector DcaControlledVector20 = FVector::ZeroVector;
    /** CActorDCA::CBhvTrajectory serialized fields. */
    UPROPERTY(EditAnywhere, Category="DCA behaviour", meta=(EditCondition="bHasTypedDcaTrajectoryBehaviour")) int32 DcaTrajectoryActorRef = -1;
    UPROPERTY(EditAnywhere, Category="DCA behaviour", meta=(EditCondition="bHasTypedDcaTrajectoryBehaviour")) float DcaTrajectoryField18 = 0;
    UPROPERTY(EditAnywhere, Category="DCA behaviour", meta=(EditCondition="bHasTypedDcaTrajectoryBehaviour")) int32 DcaTrajectoryField8 = 0;
    /** Fog-manager behaviour 2 streamed fog zones. */
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Fog behaviour", meta=(EditCondition="bHasTypedFogBehaviour")) TArray<FKyaFogZone> FogZones;
    UPROPERTY(EditAnywhere, Category="Shoot behaviour", meta=(EditCondition="bHasTypedShootFireBehaviour")) int32 ShootFireField2B0 = 0;
    UPROPERTY(EditAnywhere, Category="Shoot behaviour", meta=(EditCondition="bHasTypedShootFireBehaviour")) float ShootFireField8 = 0;
    UPROPERTY(EditAnywhere, Category="Shoot behaviour", meta=(EditCondition="bHasTypedShootFireBehaviour")) float ShootFireField2B4 = 0;
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Shoot behaviour", meta=(EditCondition="bHasTypedShootFireBehaviour")) TArray<int32> ShootFireActorRefs;
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Shoot behaviour", meta=(EditCondition="bHasTypedShootWaveBehaviour")) TArray<float> ShootWaveValues;
    UPROPERTY(EditAnywhere, Category="Gravity behaviour", meta=(EditCondition="bHasTypedGravityFallBehaviour")) int32 GravityFallSoundRef = -1;
    UPROPERTY(EditAnywhere, Category="Gravity behaviour", meta=(EditCondition="bHasTypedGravityBellBehaviour")) int32 GravityBellSoundRef = -1;
    UPROPERTY(EditAnywhere, Category="Gravity behaviour", meta=(EditCondition="bHasTypedGravityBellBehaviour")) float GravityBellFieldC = 0;
    UPROPERTY(EditAnywhere, Category="Gravity behaviour", meta=(EditCondition="bHasTypedGravityBellBehaviour")) FKyaSwitch GravityBellSwitch;
    UPROPERTY(EditAnywhere, Category="Bridge behaviour", meta=(EditCondition="bHasTypedBridgeBasicBehaviour")) FVector BridgeBasicDirection = FVector::ZeroVector;
    UPROPERTY(EditAnywhere, Category="Bridge behaviour", meta=(EditCondition="bHasTypedBridgeBasicBehaviour")) float BridgeBasicField40 = 0;
    UPROPERTY(EditAnywhere, Category="Bridge behaviour", meta=(EditCondition="bHasTypedBridgeBasicBehaviour")) float BridgeBasicField44 = 0;
    UPROPERTY(EditAnywhere, Category="Bridge behaviour", meta=(EditCondition="bHasTypedBridgeBasicBehaviour")) float BridgeBasicField48 = 0;
    UPROPERTY(EditAnywhere, Category="Bridge behaviour", meta=(EditCondition="bHasTypedBridgeBasicBehaviour")) float BridgeBasicField4C = 0;
    UPROPERTY(EditAnywhere, Category="Bridge behaviour", meta=(EditCondition="bHasTypedBridgeBasicBehaviour")) float BridgeBasicField50 = 0;
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Bridge behaviour", meta=(EditCondition="bHasTypedBridgeWindBehaviour")) TArray<float> BridgeWindValues;
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
    /** First travelled segment that selects this sound. */
    UPROPERTY(EditAnywhere, Category="Moving platform") int32 SegmentStart = 0;
    /** Last travelled segment that selects this sound. */
    UPROPERTY(EditAnywhere, Category="Moving platform") int32 SegmentEnd = 0;
    UPROPERTY(EditAnywhere, Category="Moving platform") int32 SoundRef = 0;
    UPROPERTY(EditAnywhere, Category="Moving platform") int32 SampleRef = 0;
};

USTRUCT(BlueprintType)
struct FKyaPlatformCamera
{
    GENERATED_BODY()
    /** Entry event type; the game tests 0xb, 0xc and 0xe while creating the platform. */
    UPROPERTY(EditAnywhere, Category="Moving platform") int32 EventType = 0;
    UPROPERTY(EditAnywhere, Category="Moving platform") int32 Flags = 0;
    /** S_STREAM_NTF_TARGET_SWITCH: target reference or index. */
    UPROPERTY(EditAnywhere, Category="Moving platform") int32 TargetRef = 0;
    UPROPERTY(EditAnywhere, Category="Moving platform") int32 CutsceneId = 0;
    UPROPERTY(EditAnywhere, Category="Moving platform") int32 TargetFlags = 0;
    UPROPERTY(EditAnywhere, Category="Moving platform") int32 MessageId = 0;
    UPROPERTY(EditAnywhere, Category="Moving platform") int32 MessageFlags = 0;
    UPROPERTY(EditAnywhere, Category="Moving platform") int32 TargetField14 = 0;
    UPROPERTY(EditAnywhere, Category="Moving platform") int32 TargetField18 = 0;
    /** S_STREAM_EVENT_CAMERA: scenaric camera selection and trigger bounds. */
    UPROPERTY(EditAnywhere, Category="Moving platform") int32 CameraIndex = 0;
    UPROPERTY(EditAnywhere, Category="Moving platform") int32 CameraFlags = 0;
    UPROPERTY(EditAnywhere, Category="Moving platform") float Field8 = 0;
    UPROPERTY(EditAnywhere, Category="Moving platform") float FieldC = 0;
    UPROPERTY(EditAnywhere, Category="Moving platform") float Field10 = 0;
    UPROPERTY(EditAnywhere, Category="Moving platform") int32 ActorRef = 0;
    UPROPERTY(EditAnywhere, Category="Moving platform") int32 ZoneRef = 0;
    UPROPERTY(EditAnywhere, Category="Moving platform") float Field1C = 0;
    /** S_STREAM_REF<CSound> at the end of the entry. */
    UPROPERTY(EditAnywhere, Category="Moving platform") int32 SoundRef = 0;
};

USTRUCT(BlueprintType)
struct FKyaPlatformProperties
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, Category="Moving platform") int32 SoundRefA = 0;
    UPROPERTY(EditAnywhere, Category="Moving platform") int32 SampleRefA = 0;
    UPROPERTY(EditAnywhere, Category="Moving platform") int32 SoundRefB = 0;
    UPROPERTY(EditAnywhere, Category="Moving platform") int32 SampleRefB = 0;
    UPROPERTY(EditAnywhere, Category="Moving platform") FVector Offset = FVector::ZeroVector;
    /** Dynamic effect selected while the platform is in one movement phase. */
    UPROPERTY(EditAnywhere, Category="Moving platform") int64 EffectRef1C = 0;
    /** Dynamic effect selected while the platform is in the other movement phase. */
    UPROPERTY(EditAnywhere, Category="Moving platform") int64 EffectRef20 = 0;
    UPROPERTY(EditAnywhere, Category="Moving platform") int64 PlatformFlags = 0;
    /** Scenaric camera selected while riding the platform, or -1 for none. */
    UPROPERTY(EditAnywhere, Category="Moving platform") int32 CameraIndex = 0;
    UPROPERTY(EditAnywhere, Category="Moving platform") float TiltValue = 0;
    UPROPERTY(EditAnywhere, Category="Moving platform") int32 TiltMode = 0;
    UPROPERTY(EditAnywhere, Category="Moving platform") FVector4 TiltAndPush = FVector4(0, 0, 0, 0);
    UPROPERTY(EditAnywhere, Category="Moving platform") FVector2D PushOscillation = FVector2D::ZeroVector;
};

/** S_TRAP_STREAM_ENTRY: switch target and camera event. */
USTRUCT(BlueprintType)
struct FKyaTrapStreamEntry
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, Category="Trap") FKyaSwitchTarget Target;
    UPROPERTY(EditAnywhere, Category="Trap") FKyaStreamEventCamera Camera;
};

/** CAddOnGenerator: serialized before its optional six-float sub-object. */
USTRUCT(BlueprintType)
struct FKyaAddOnGenerator
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, Category="Add-on generator") int32 Field0 = 0;
    UPROPERTY(EditAnywhere, Category="Add-on generator") int32 MoneyType = 0;
    UPROPERTY(EditAnywhere, Category="Add-on generator") int32 MoneyCount = 0;
    UPROPERTY(EditAnywhere, Category="Add-on generator") int32 BonusCount = 0;
    /** Serialized hundredths; the game stores field_0xc * 0.01. */
    UPROPERTY(EditAnywhere, Category="Add-on generator") int32 FieldC = 0;
    /** Controls whether the five sub-object floats are present; changing it is not supported. */
    UPROPERTY(EditAnywhere, Category="Add-on generator") int32 SubObjMode = 0;
    UPROPERTY(EditAnywhere, Category="Add-on generator") float SubObjField0 = 0;
    UPROPERTY(EditAnywhere, Category="Add-on generator") float SubObjField4 = 0;
    UPROPERTY(EditAnywhere, Category="Add-on generator") float SubObjFieldC = 0;
    UPROPERTY(EditAnywhere, Category="Add-on generator") float SubObjField10 = 0;
    UPROPERTY(EditAnywhere, Category="Add-on generator") float SubObjField14 = 0;
};

/** ProjectileSubObj, the fixed 72-byte projectile tuning record. */
USTRUCT(BlueprintType)
struct FKyaProjectileSubObj
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, Category="Projectile") float TimeToExplode = 0;
    UPROPERTY(EditAnywhere, Category="Projectile") float ExplosionRange = 0;
    UPROPERTY(EditAnywhere, Category="Projectile") float AnimDuration = 0;
    UPROPERTY(EditAnywhere, Category="Projectile") int32 Flags = 0;
    UPROPERTY(EditAnywhere, Category="Projectile") float Damage = 0;
    UPROPERTY(EditAnywhere, Category="Projectile") float HitRadius = 0;
    UPROPERTY(EditAnywhere, Category="Projectile") float WarnRadius = 0;
    UPROPERTY(EditAnywhere, Category="Projectile") float Field1C = 0;
    UPROPERTY(EditAnywhere, Category="Projectile") float Field20 = 0;
    UPROPERTY(EditAnywhere, Category="Projectile") float Field24 = 0;
    UPROPERTY(EditAnywhere, Category="Projectile") int32 Field28 = 0;
    UPROPERTY(EditAnywhere, Category="Projectile") int32 MaterialId = 0;
    UPROPERTY(EditAnywhere, Category="Projectile") int32 Field30 = 0;
    UPROPERTY(EditAnywhere, Category="Projectile") int32 Field34 = 0;
    UPROPERTY(EditAnywhere, Category="Projectile") int32 Field38 = 0;
    UPROPERTY(EditAnywhere, Category="Projectile") int32 Field3C = 0;
    UPROPERTY(EditAnywhere, Category="Projectile") float Field40 = 0;
    UPROPERTY(EditAnywhere, Category="Projectile") float Field44 = 0;
};

/** CPatternPart serialized tuning; field 0x24 is version-dependent. */
USTRUCT(BlueprintType)
struct FKyaPatternPart
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, Category="Pattern") float Field4 = 0;
    UPROPERTY(EditAnywhere, Category="Pattern") float Field0 = 0;
    UPROPERTY(EditAnywhere, Category="Pattern") float FieldC = 0;
    UPROPERTY(EditAnywhere, Category="Pattern") float Field8 = 0;
    UPROPERTY(EditAnywhere, Category="Pattern") float Field10 = 0;
    UPROPERTY(EditAnywhere, Category="Pattern") float Field14 = 0;
    UPROPERTY(EditAnywhere, Category="Pattern") int32 MaterialId = 0;
    /** Fractional part controls the runtime point-pattern count. */
    UPROPERTY(EditAnywhere, Category="Pattern") float PointCount = 0;
    UPROPERTY(EditAnywhere, Category="Pattern") float Field20 = 0;
    UPROPERTY(VisibleAnywhere, Category="Pattern") bool bHasField24 = false;
    UPROPERTY(EditAnywhere, Category="Pattern", meta=(EditCondition="bHasField24")) float Field24 = 0;
    UPROPERTY(EditAnywhere, Category="Pattern") int32 Field28 = 0;
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
    /** CActorMovable weightB read before the moving-platform prefix. */
    UPROPERTY(EditAnywhere, Category="Moving platform", meta=(EditCondition="bHasPlatformPrefix")) float PlatformWeight = 0;
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Moving platform", meta=(EditCondition="bHasPlatformPrefix")) TArray<FKyaPlatformActor> PlatformActors;
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Moving platform", meta=(EditCondition="bHasPlatformPrefix")) TArray<int32> PlatformZones;
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Moving platform", meta=(EditCondition="bHasPlatformPrefix")) TArray<FKyaPlatformNoFrictionZone> PlatformNoFrictionZones;
    UPROPERTY(VisibleAnywhere, Category="Moving platform") bool bHasPlatformDetails = false;
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Moving platform", meta=(EditCondition="bHasPlatformDetails")) TArray<FKyaPlatformCamera> PlatformCameras;
    UPROPERTY(EditAnywhere, Category="Moving platform", meta=(EditCondition="bHasPlatformDetails")) FKyaPlatformProperties PlatformProperties;
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Moving platform", meta=(EditCondition="bHasPlatformDetails")) TArray<FKyaPlatformSound> PlatformSoundsA;
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Moving platform", meta=(EditCondition="bHasPlatformDetails")) TArray<FKyaPlatformSound> PlatformSoundsB;
    UPROPERTY(VisibleAnywhere, Category="Basic box") bool bHasBasicBoxData = false;
    UPROPERTY(EditAnywhere, Category="Basic box", meta=(EditCondition="bHasBasicBoxData")) int32 BasicBoxField160 = 0;
    /** Serialized as an integer; the game converts it to the starting life value. */
    UPROPERTY(EditAnywhere, Category="Basic box", meta=(EditCondition="bHasBasicBoxData")) float BasicBoxLifeMax = 0;
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Basic box", meta=(EditCondition="bHasBasicBoxData")) TArray<int32> BasicBoxActorRefs;
    UPROPERTY(EditAnywhere, Category="Basic box", meta=(EditCondition="bHasBasicBoxData")) float BasicBoxVibration0 = 0;
    UPROPERTY(EditAnywhere, Category="Basic box", meta=(EditCondition="bHasBasicBoxData")) float BasicBoxVibration1 = 0;
    UPROPERTY(EditAnywhere, Category="Basic box", meta=(EditCondition="bHasBasicBoxData")) float BasicBoxVibration2 = 0;
    UPROPERTY(EditAnywhere, Category="Basic box", meta=(EditCondition="bHasBasicBoxData")) float BasicBoxVibration3 = 0;
    UPROPERTY(EditAnywhere, Category="Basic box", meta=(EditCondition="bHasBasicBoxData")) int32 BasicBoxField180 = 0;
    UPROPERTY(EditAnywhere, Category="Basic box", meta=(EditCondition="bHasBasicBoxData")) int32 BasicBoxField18C = 0;
    UPROPERTY(EditAnywhere, Category="Basic box", meta=(EditCondition="bHasBasicBoxData")) int32 BasicBoxField190 = 0;
    UPROPERTY(EditAnywhere, Category="Basic box", meta=(EditCondition="bHasBasicBoxData")) int32 BasicBoxField198 = 0;
    UPROPERTY(EditAnywhere, Category="Basic box", meta=(EditCondition="bHasBasicBoxData")) int32 BasicBoxField194 = 0;
    UPROPERTY(EditAnywhere, Category="Basic box", meta=(EditCondition="bHasBasicBoxData")) FKyaSwitch BasicBoxSwitch;
    UPROPERTY(EditAnywhere, Category="Basic box", meta=(EditCondition="bHasBasicBoxData")) FKyaAddOnGenerator BasicBoxAddOn;
    UPROPERTY(VisibleAnywhere, Category="FX") bool bHasFxData = false;
    /** Serialized value at 0x19c; the game clamps it to 0..1. */
    UPROPERTY(EditAnywhere, Category="FX", meta=(EditCondition="bHasFxData")) float FxField19C = 0;
    /** Serialized value at 0x1a0; the game clamps it to zero or greater. */
    UPROPERTY(EditAnywhere, Category="FX", meta=(EditCondition="bHasFxData")) float FxField1A0 = 0;
    UPROPERTY(VisibleAnywhere, Category="Araignos") bool bHasAraignosData = false;
    /** CActorMovable weight read before the Araignos payload. */
    UPROPERTY(EditAnywhere, Category="Araignos", meta=(EditCondition="bHasAraignosData")) float AraignosWeightB = 0;
    UPROPERTY(EditAnywhere, Category="Araignos", meta=(EditCondition="bHasAraignosData")) int32 AraignosPathFollowIndex = -1;
    UPROPERTY(EditAnywhere, Category="Araignos", meta=(EditCondition="bHasAraignosData")) float AraignosField1F8 = 0;
    UPROPERTY(EditAnywhere, Category="Araignos", meta=(EditCondition="bHasAraignosData")) float AraignosField1FC = 0;
    UPROPERTY(EditAnywhere, Category="Araignos", meta=(EditCondition="bHasAraignosData")) float AraignosDamage = 0;
    UPROPERTY(EditAnywhere, Category="Araignos", meta=(EditCondition="bHasAraignosData")) int32 AraignosSoundRef240 = 0;
    UPROPERTY(EditAnywhere, Category="Araignos", meta=(EditCondition="bHasAraignosData")) int32 AraignosSoundRef238 = 0;
    UPROPERTY(EditAnywhere, Category="Araignos", meta=(EditCondition="bHasAraignosData")) int32 AraignosSoundRef23C = 0;
    UPROPERTY(EditAnywhere, Category="Araignos", meta=(EditCondition="bHasAraignosData")) FKyaAddOnGenerator AraignosAddOn;
    UPROPERTY(VisibleAnywhere, Category="Projectile") bool bHasProjectileData = false;
    /** CActorMovable weightB read before the autonomous and projectile payloads. */
    UPROPERTY(EditAnywhere, Category="Projectile", meta=(EditCondition="bHasProjectileData")) float ProjectileWeightB = 0;
    UPROPERTY(EditAnywhere, Category="Projectile", meta=(EditCondition="bHasProjectileData")) float ProjectileLifeMax = 0;
    UPROPERTY(EditAnywhere, Category="Projectile", meta=(EditCondition="bHasProjectileData")) float ProjectileGravityScale = 0;
    UPROPERTY(EditAnywhere, Category="Projectile", meta=(EditCondition="bHasProjectileData")) float ProjectileDynamicExtField4 = 0;
    UPROPERTY(EditAnywhere, Category="Projectile", meta=(EditCondition="bHasProjectileData")) float ProjectileDynamicExtField8 = 0;
    UPROPERTY(EditAnywhere, Category="Projectile", meta=(EditCondition="bHasProjectileData")) FKyaProjectileSubObj ProjectileSubObj;
    UPROPERTY(EditAnywhere, Category="Projectile", meta=(EditCondition="bHasProjectileData")) FKyaPatternPart ProjectilePattern;
    UPROPERTY(VisibleAnywhere, Category="Wind") bool bHasWindData = false;
    UPROPERTY(EditAnywhere, Category="Wind", meta=(EditCondition="bHasWindData")) int32 WindField160 = 0;
    UPROPERTY(EditAnywhere, Category="Wind", meta=(EditCondition="bHasWindData")) float WindMaxWind = 0;
    UPROPERTY(EditAnywhere, Category="Wind", meta=(EditCondition="bHasWindData")) float WindField168 = 0;
    UPROPERTY(EditAnywhere, Category="Wind", meta=(EditCondition="bHasWindData")) float WindField16C = 0;
    UPROPERTY(EditAnywhere, Category="Wind", meta=(EditCondition="bHasWindData")) float WindField174 = 0;
    UPROPERTY(EditAnywhere, Category="Wind", meta=(EditCondition="bHasWindData")) float WindField180 = 0;
    UPROPERTY(EditAnywhere, Category="Wind", meta=(EditCondition="bHasWindData")) float WindField184 = 0;
    UPROPERTY(EditAnywhere, Category="Wind", meta=(EditCondition="bHasWindData")) float WindField188 = 0;
    UPROPERTY(EditAnywhere, Category="Wind", meta=(EditCondition="bHasWindData")) float WindField18C = 0;
    UPROPERTY(EditAnywhere, Category="Wind", meta=(EditCondition="bHasWindData")) float WindField178 = 0;
    UPROPERTY(EditAnywhere, Category="Wind", meta=(EditCondition="bHasWindData")) float WindField17C = 0;
    UPROPERTY(EditAnywhere, Category="Wind", meta=(EditCondition="bHasWindData")) int32 WindField190 = 0;
    UPROPERTY(EditAnywhere, Category="Wind", meta=(EditCondition="bHasWindData")) int32 WindSoundRef = 0;
    UPROPERTY(EditAnywhere, Category="Wind", meta=(EditCondition="bHasWindData")) int32 WindActivationZoneIndex = -1;
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Wind", meta=(EditCondition="bHasWindData")) TArray<int32> WindActorRefs;
    UPROPERTY(EditAnywhere, Category="Wind", meta=(EditCondition="bHasWindData")) int32 WindMaterialId = 0;
    UPROPERTY(EditAnywhere, Category="Wind", meta=(EditCondition="bHasWindData")) int32 WindZoneRef1C4 = -1;
    UPROPERTY(EditAnywhere, Category="Wind", meta=(EditCondition="bHasWindData")) int32 WindColor1C8 = 0;
    UPROPERTY(EditAnywhere, Category="Wind", meta=(EditCondition="bHasWindData")) int32 WindWaypointRef1CC = -1;
    UPROPERTY(EditAnywhere, Category="Wind", meta=(EditCondition="bHasWindData")) int64 WindSectorHierarchyHash = 0;
    UPROPERTY(EditAnywhere, Category="Wind", meta=(EditCondition="bHasWindData")) FKyaSwitch WindSwitch1D0;
    UPROPERTY(EditAnywhere, Category="Wind", meta=(EditCondition="bHasWindData")) FKyaSwitch WindSwitch1D8;
    UPROPERTY(EditAnywhere, Category="Wind", meta=(EditCondition="bHasWindData")) FKyaSwitch WindSwitch1E0;
    UPROPERTY(EditAnywhere, Category="Wind", meta=(EditCondition="bHasWindData")) float WindField194 = 0;
    UPROPERTY(EditAnywhere, Category="Wind", meta=(EditCondition="bHasWindData")) float WindField198 = 0;
    UPROPERTY(EditAnywhere, Category="Wind", meta=(EditCondition="bHasWindData")) float WindField19C = 0;
    UPROPERTY(EditAnywhere, Category="Wind", meta=(EditCondition="bHasWindData")) float WindField1A0 = 0;
    UPROPERTY(EditAnywhere, Category="Wind", meta=(EditCondition="bHasWindData")) float WindField1A4 = 0;
    UPROPERTY(EditAnywhere, Category="Wind", meta=(EditCondition="bHasWindData")) int32 WindWaypointRef = -1;
    UPROPERTY(EditAnywhere, Category="Wind", meta=(EditCondition="bHasWindData")) float WindField1AC = 0;
    UPROPERTY(EditAnywhere, Category="Wind", meta=(EditCondition="bHasWindData")) float WindField1B0 = 0;
    UPROPERTY(VisibleAnywhere, Category="Trap") bool bHasTrapData = false;
    /** CActorMovable weightB read before the trap payload. */
    UPROPERTY(EditAnywhere, Category="Trap", meta=(EditCondition="bHasTrapData")) float TrapWeightB = 0;
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Trap", meta=(EditCondition="bHasTrapData")) TArray<int32> TrapActorRefs;
    UPROPERTY(EditAnywhere, Category="Trap", meta=(EditCondition="bHasTrapData")) int32 TrapField14 = 0;
    UPROPERTY(EditAnywhere, Category="Trap", meta=(EditCondition="bHasTrapData")) int32 TrapField18 = 0;
    UPROPERTY(EditAnywhere, Category="Trap", meta=(EditCondition="bHasTrapData")) int32 TrapField1C = 0;
    UPROPERTY(EditAnywhere, Category="Trap", meta=(EditCondition="bHasTrapData")) int32 TrapField20 = 0;
    UPROPERTY(VisibleAnywhere, Category="Trap") bool bHasTrapField24 = false;
    UPROPERTY(EditAnywhere, Category="Trap", meta=(EditCondition="bHasTrapData && bHasTrapField24")) int32 TrapField24 = -1;
    UPROPERTY(EditAnywhere, Category="Trap", meta=(EditCondition="bHasTrapData")) int32 TrapActorRef = -1;
    UPROPERTY(EditAnywhere, Category="Trap", meta=(EditCondition="bHasTrapData")) float TrapSpeed = 0;
    UPROPERTY(EditAnywhere, Category="Trap", meta=(EditCondition="bHasTrapData")) float TrapAcceleration = 0;
    UPROPERTY(EditAnywhere, Category="Trap", meta=(EditCondition="bHasTrapData")) int32 TrapPathFollowIndex = -1;
    UPROPERTY(EditAnywhere, Category="Trap", meta=(EditCondition="bHasTrapData")) float TrapField4C = 0;
    UPROPERTY(EditAnywhere, Category="Trap", meta=(EditCondition="bHasTrapData")) int32 TrapRequiredEscapeAttempts = 0;
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Trap", meta=(EditCondition="bHasTrapData")) TArray<FKyaTrapStreamEntry> TrapStreamEntries;
    UPROPERTY(VisibleAnywhere, Category="Weapon") bool bHasWeaponData = false;
    /** CActorMovable weightB read before the weapon payload. */
    UPROPERTY(EditAnywhere, Category="Weapon", meta=(EditCondition="bHasWeaponData")) float WeaponWeightB = 0;
    UPROPERTY(EditAnywhere, Category="Weapon", meta=(EditCondition="bHasWeaponData")) float WeaponField1D8 = 0;
    UPROPERTY(EditAnywhere, Category="Weapon", meta=(EditCondition="bHasWeaponData")) float WeaponField1DC = 0;
    UPROPERTY(VisibleAnywhere, Category="Switch") bool bHasSwitchData = false;
    UPROPERTY(EditAnywhere, Category="Switch", meta=(EditCondition="bHasSwitchData")) FKyaSwitch SwitchTarget;
    UPROPERTY(VisibleAnywhere, Category="Bonus") bool bHasBonusData = false;
    /** CActorMovable weightB read before the bonus payload. */
    UPROPERTY(EditAnywhere, Category="Bonus", meta=(EditCondition="bHasBonusData")) float BonusWeightB = 0;
    UPROPERTY(EditAnywhere, Category="Bonus", meta=(EditCondition="bHasBonusData")) int32 BonusSoundRef = -1;
    UPROPERTY(EditAnywhere, Category="Bonus", meta=(EditCondition="bHasBonusData")) int32 BonusFlareMaterialId = -1;
    UPROPERTY(EditAnywhere, Category="Bonus", meta=(EditCondition="bHasBonusData")) int32 BonusAnimMaterialId = -1;
    UPROPERTY(VisibleAnywhere, Category="Micken") bool bHasMickenData = false;
    UPROPERTY(EditAnywhere, Category="Micken", meta=(EditCondition="bHasMickenData")) float MickenWeightB = 0;
    UPROPERTY(EditAnywhere, Category="Micken", meta=(EditCondition="bHasMickenData")) float MickenLifeMax = 0;
    UPROPERTY(EditAnywhere, Category="Micken", meta=(EditCondition="bHasMickenData")) float MickenGravityScale = 0;
    UPROPERTY(EditAnywhere, Category="Micken", meta=(EditCondition="bHasMickenData")) float MickenDynamicExtField4 = 0;
    UPROPERTY(EditAnywhere, Category="Micken", meta=(EditCondition="bHasMickenData")) float MickenDynamicExtField8 = 0;
    UPROPERTY(EditAnywhere, Category="Micken", meta=(EditCondition="bHasMickenData")) int32 MickenWaypointIndex = -1;
    UPROPERTY(EditAnywhere, Category="Micken", meta=(EditCondition="bHasMickenData")) int32 MickenActorIndex = -1;
    UPROPERTY(EditAnywhere, Category="Micken", meta=(EditCondition="bHasMickenData")) int32 MickenField358 = 0;
    UPROPERTY(EditAnywhere, Category="Micken", meta=(EditCondition="bHasMickenData")) float MickenField35C = 0;
    UPROPERTY(EditAnywhere, Category="Micken", meta=(EditCondition="bHasMickenData")) int32 MickenField360 = 0;
    UPROPERTY(VisibleAnywhere, Category="Fruit") bool bHasFruitData = false;
    UPROPERTY(EditAnywhere, Category="Fruit", meta=(EditCondition="bHasFruitData")) float FruitWeightB = 0;
    UPROPERTY(EditAnywhere, Category="Fruit", meta=(EditCondition="bHasFruitData")) float FruitLifeMax = 0;
    UPROPERTY(EditAnywhere, Category="Fruit", meta=(EditCondition="bHasFruitData")) float FruitGravityScale = 0;
    UPROPERTY(EditAnywhere, Category="Fruit", meta=(EditCondition="bHasFruitData")) float FruitDynamicExtField4 = 0;
    UPROPERTY(EditAnywhere, Category="Fruit", meta=(EditCondition="bHasFruitData")) float FruitDynamicExtField8 = 0;
    UPROPERTY(EditAnywhere, Category="Fruit", meta=(EditCondition="bHasFruitData")) int32 FruitField360 = 0;
    UPROPERTY(EditAnywhere, Category="Fruit", meta=(EditCondition="bHasFruitData")) FKyaSwitch FruitSwitch;
    UPROPERTY(VisibleAnywhere, Category="Commander") bool bHasCommanderData = false;
    UPROPERTY(EditAnywhere, Category="Commander", meta=(EditCondition="bHasCommanderData")) int32 CommanderDetectAreaZoneId = 0;
    UPROPERTY(EditAnywhere, Category="Commander", meta=(EditCondition="bHasCommanderData")) int32 CommanderGuardAreaZoneId = 0;
    UPROPERTY(EditAnywhere, Category="Commander", meta=(EditCondition="bHasCommanderData")) int32 CommanderField180 = 0;
    UPROPERTY(EditAnywhere, Category="Commander", meta=(EditCondition="bHasCommanderData")) int32 CommanderNewPathFindingId = -1;
    UPROPERTY(EditAnywhere, Category="Commander", meta=(EditCondition="bHasCommanderData")) int32 CommanderActorRef = -1;
    UPROPERTY(EditAnywhere, Category="Commander", meta=(EditCondition="bHasCommanderData")) float CommanderSquadValue = 0;
    UPROPERTY(EditAnywhere, Category="Commander", meta=(EditCondition="bHasCommanderData")) int32 CommanderSquadField4 = 0;
    UPROPERTY(EditAnywhere, Category="Commander", meta=(EditCondition="bHasCommanderData")) int32 CommanderSquadField8 = 0;
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Commander", meta=(EditCondition="bHasCommanderData")) TArray<int32> CommanderTeamEnemies;
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Commander", meta=(EditCondition="bHasCommanderData")) TArray<FKyaCommanderComponent> CommanderSequence;
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Commander", meta=(EditCondition="bHasCommanderData")) TArray<float> CommanderCameraValues;
    UPROPERTY(VisibleAnywhere, Category="Nativ") bool bHasNativData = false;
    UPROPERTY(EditAnywhere, Category="Nativ", meta=(EditCondition="bHasNativData")) float NativWeightB = 0;
    UPROPERTY(EditAnywhere, Category="Nativ", meta=(EditCondition="bHasNativData")) float NativLifeMax = 0;
    UPROPERTY(EditAnywhere, Category="Nativ", meta=(EditCondition="bHasNativData")) float NativGravityScale = 0;
    UPROPERTY(EditAnywhere, Category="Nativ", meta=(EditCondition="bHasNativData")) float NativDynamicExtField4 = 0;
    UPROPERTY(EditAnywhere, Category="Nativ", meta=(EditCondition="bHasNativData")) float NativDynamicExtField8 = 0;
    UPROPERTY(EditAnywhere, Category="Nativ", meta=(EditCondition="bHasNativData")) int32 NativField350 = 0;
    UPROPERTY(EditAnywhere, Category="Nativ", meta=(EditCondition="bHasNativData")) int32 NativField354 = 0;
    UPROPERTY(EditAnywhere, Category="Nativ", meta=(EditCondition="bHasNativData")) int32 NativField358 = 0;
    UPROPERTY(EditAnywhere, Category="Nativ", meta=(EditCondition="bHasNativData")) float NativMoveSpeedNoHeld = 0;
    UPROPERTY(EditAnywhere, Category="Nativ", meta=(EditCondition="bHasNativData")) float NativMoveSpeedHeld = 0;
    UPROPERTY(EditAnywhere, Category="Nativ", meta=(EditCondition="bHasNativData")) int32 NativField370 = 0;
    UPROPERTY(EditAnywhere, Category="Nativ", meta=(EditCondition="bHasNativData")) int32 NativField368 = -1;
    UPROPERTY(EditAnywhere, Category="Nativ", meta=(EditCondition="bHasNativData")) int32 NativField364 = -1;
    UPROPERTY(EditAnywhere, Category="Nativ", meta=(EditCondition="bHasNativData")) int32 NativField378 = 0;
    UPROPERTY(EditAnywhere, Category="Nativ", meta=(EditCondition="bHasNativData")) int32 NativField37C = 0;
    UPROPERTY(EditAnywhere, Category="Nativ", meta=(EditCondition="bHasNativData")) int32 NativField380 = 0;
    UPROPERTY(VisibleAnywhere, Category="Amortos") bool bHasAmortosData = false;
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Amortos", meta=(EditCondition="bHasAmortosData")) TArray<int32> AmortosCheckpointActors;
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Amortos", meta=(EditCondition="bHasAmortosData")) TArray<int32> AmortosActors188;
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Amortos", meta=(EditCondition="bHasAmortosData")) TArray<int32> AmortosActors18C;
    UPROPERTY(EditAnywhere, Category="Amortos", meta=(EditCondition="bHasAmortosData")) int32 AmortosActorRef = -1;
    UPROPERTY(EditAnywhere, Category="Amortos", meta=(EditCondition="bHasAmortosData")) float AmortosField160 = 0;
    UPROPERTY(EditAnywhere, Category="Amortos", meta=(EditCondition="bHasAmortosData")) float AmortosField164 = 0;
    UPROPERTY(EditAnywhere, Category="Amortos", meta=(EditCondition="bHasAmortosData")) float AmortosField168 = 0;
    UPROPERTY(EditAnywhere, Category="Amortos", meta=(EditCondition="bHasAmortosData")) float AmortosField16C = 0;
    UPROPERTY(EditAnywhere, Category="Amortos", meta=(EditCondition="bHasAmortosData")) float AmortosField170 = 0;
    UPROPERTY(EditAnywhere, Category="Amortos", meta=(EditCondition="bHasAmortosData")) float AmortosField174 = 0;
    UPROPERTY(EditAnywhere, Category="Amortos", meta=(EditCondition="bHasAmortosData")) float AmortosField178 = 0;
    UPROPERTY(EditAnywhere, Category="Amortos", meta=(EditCondition="bHasAmortosData")) float AmortosField17C = 0;
    UPROPERTY(EditAnywhere, Category="Amortos", meta=(EditCondition="bHasAmortosData")) float AmortosField180 = 0;
    UPROPERTY(EditAnywhere, Category="Amortos", meta=(EditCondition="bHasAmortosData")) FKyaAddOnGenerator AmortosAddOn;
    UPROPERTY(EditAnywhere, Category="Amortos", meta=(EditCondition="bHasAmortosData")) int32 AmortosField1C8 = 0;
    UPROPERTY(EditAnywhere, Category="Amortos", meta=(EditCondition="bHasAmortosData")) FKyaSwitch AmortosSwitch1CC;
    UPROPERTY(EditAnywhere, Category="Amortos", meta=(EditCondition="bHasAmortosData")) FKyaSwitch AmortosSwitch1D4;
    UPROPERTY(VisibleAnywhere, Category="Teleporter") bool bHasTeleporterData = false;
    UPROPERTY(EditAnywhere, Category="Teleporter", meta=(EditCondition="bHasTeleporterData")) int32 TeleporterSubsectorMaterialId = 0;
    UPROPERTY(EditAnywhere, Category="Teleporter", meta=(EditCondition="bHasTeleporterData")) int64 TeleporterHash = 0;
    UPROPERTY(EditAnywhere, Category="Teleporter", meta=(EditCondition="bHasTeleporterData")) int32 TeleporterField16C = 0;
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Teleporter", meta=(EditCondition="bHasTeleporterData")) TArray<FKyaTeleporterDestination> TeleporterDestinations;
    UPROPERTY(EditAnywhere, Category="Teleporter", meta=(EditCondition="bHasTeleporterData")) FKyaSwitch TeleporterSwitch170;
    UPROPERTY(EditAnywhere, Category="Teleporter", meta=(EditCondition="bHasTeleporterData")) FKyaSwitch TeleporterSwitch178;
    UPROPERTY(EditAnywhere, Category="Teleporter", meta=(EditCondition="bHasTeleporterData")) FKyaSwitch TeleporterSwitch180;
    UPROPERTY(EditAnywhere, Category="Teleporter", meta=(EditCondition="bHasTeleporterData")) int32 TeleporterZone188 = -1;
    UPROPERTY(EditAnywhere, Category="Teleporter", meta=(EditCondition="bHasTeleporterData")) int32 TeleporterZone18C = -1;
    UPROPERTY(EditAnywhere, Category="Teleporter", meta=(EditCondition="bHasTeleporterData")) int32 TeleporterMaterialId1 = 0;
    UPROPERTY(EditAnywhere, Category="Teleporter", meta=(EditCondition="bHasTeleporterData")) int32 TeleporterCinematicId = -1;
    UPROPERTY(EditAnywhere, Category="Teleporter", meta=(EditCondition="bHasTeleporterData")) int32 TeleporterMaterialId2 = 0;
    UPROPERTY(EditAnywhere, Category="Teleporter", meta=(EditCondition="bHasTeleporterData")) int32 TeleporterField1D4 = -1;
    UPROPERTY(EditAnywhere, Category="Teleporter", meta=(EditCondition="bHasTeleporterData")) int32 TeleporterField1D8 = -1;
    UPROPERTY(EditAnywhere, Category="Teleporter", meta=(EditCondition="bHasTeleporterData")) int32 TeleporterMaterialId3 = 0;
    UPROPERTY(EditAnywhere, Category="Teleporter", meta=(EditCondition="bHasTeleporterData")) int32 TeleporterField1E0 = -1;
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Teleporter", meta=(EditCondition="bHasTeleporterData")) TArray<FKyaTeleporterWaypoint> TeleporterWaypoints;
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Teleporter condition", meta=(EditCondition="bHasTeleporterData")) TArray<FKyaTeleporterBlock> TeleporterConditionedOperations;
    UPROPERTY(VisibleAnywhere, Category="DCA") bool bHasDcaData = false;
    UPROPERTY(EditAnywhere, Category="DCA", meta=(EditCondition="bHasDcaData")) int32 DcaField504 = -1;
    UPROPERTY(EditAnywhere, Category="DCA", meta=(EditCondition="bHasDcaData")) int32 DcaField500 = -1;
    UPROPERTY(EditAnywhere, Category="DCA", meta=(EditCondition="bHasDcaData")) float DcaField464 = 0;
    UPROPERTY(EditAnywhere, Category="DCA", meta=(EditCondition="bHasDcaData")) int32 DcaField508 = -1;
    UPROPERTY(EditAnywhere, Category="DCA", meta=(EditCondition="bHasDcaData")) int32 DcaField50C = -1;
    /** Serialized in degrees; the game converts it to radians when created. */
    UPROPERTY(EditAnywhere, Category="DCA", meta=(EditCondition="bHasDcaData")) float DcaAimAdjustDegrees = 0;
    UPROPERTY(EditAnywhere, EditFixedSize, Category="DCA", meta=(EditCondition="bHasDcaData")) TArray<int32> DcaShotActorRefs;
    UPROPERTY(EditAnywhere, Category="DCA", meta=(EditCondition="bHasDcaData")) int32 DcaField450 = 0;
    UPROPERTY(EditAnywhere, Category="DCA", meta=(EditCondition="bHasDcaData")) int32 DcaField454 = 0;
    UPROPERTY(VisibleAnywhere, Category="Box") bool bHasBoxData = false;
    UPROPERTY(EditAnywhere, Category="Box", meta=(EditCondition="bHasBoxData")) float BoxWeightB = 0;
    UPROPERTY(EditAnywhere, Category="Box", meta=(EditCondition="bHasBoxData")) float BoxLifeMax = 0;
    UPROPERTY(EditAnywhere, Category="Box", meta=(EditCondition="bHasBoxData")) float BoxGravityScale = 0;
    UPROPERTY(EditAnywhere, Category="Box", meta=(EditCondition="bHasBoxData")) float BoxDynamicExtField4 = 0;
    UPROPERTY(EditAnywhere, Category="Box", meta=(EditCondition="bHasBoxData")) float BoxDynamicExtField8 = 0;
    UPROPERTY(EditAnywhere, Category="Box", meta=(EditCondition="bHasBoxData")) int32 BoxField350 = 0;
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Box", meta=(EditCondition="bHasBoxData")) TArray<int32> BoxActorRefs;
    UPROPERTY(EditAnywhere, Category="Box", meta=(EditCondition="bHasBoxData")) float BoxField354 = 0;
    UPROPERTY(EditAnywhere, Category="Box", meta=(EditCondition="bHasBoxData")) float BoxField358 = 0;
    UPROPERTY(EditAnywhere, Category="Box", meta=(EditCondition="bHasBoxData")) float BoxField35C = 0;
    UPROPERTY(EditAnywhere, Category="Box", meta=(EditCondition="bHasBoxData")) float BoxField360 = 0;
    UPROPERTY(EditAnywhere, Category="Box", meta=(EditCondition="bHasBoxData")) int32 BoxField364 = 0;
    UPROPERTY(EditAnywhere, Category="Box", meta=(EditCondition="bHasBoxData")) int32 BoxField370 = 0;
    UPROPERTY(EditAnywhere, Category="Box", meta=(EditCondition="bHasBoxData")) int32 BoxField374 = 0;
    UPROPERTY(EditAnywhere, Category="Box", meta=(EditCondition="bHasBoxData")) int32 BoxField378 = 0;
    UPROPERTY(EditAnywhere, Category="Box", meta=(EditCondition="bHasBoxData")) int32 BoxField37C = 0;
    UPROPERTY(EditAnywhere, Category="Box", meta=(EditCondition="bHasBoxData")) FKyaSwitch BoxSwitch;
    UPROPERTY(EditAnywhere, Category="Box", meta=(EditCondition="bHasBoxData")) FKyaAddOnGenerator BoxAddOn;
    UPROPERTY(VisibleAnywhere, Category="Money") bool bHasMoneyData = false;
    /** CActorMovable weightB read before the money fields. */
    UPROPERTY(EditAnywhere, Category="Money", meta=(EditCondition="bHasMoneyData")) float MoneyWeightB = 0;
    UPROPERTY(EditAnywhere, Category="Money", meta=(EditCondition="bHasMoneyData")) int32 MoneyValue = 0;
    UPROPERTY(EditAnywhere, Category="Money", meta=(EditCondition="bHasMoneyData")) int32 MoneySoundRef = -1;
    UPROPERTY(EditAnywhere, Category="Money", meta=(EditCondition="bHasMoneyData")) int32 MoneyMaterialId = -1;
    UPROPERTY(VisibleAnywhere, Category="Fog") bool bHasFogData = false;
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Fog", meta=(EditCondition="bHasFogData")) TArray<FKyaFogHierarchy> FogHierarchies;
    UPROPERTY(VisibleAnywhere, Category="Shoot") bool bHasShootData = false;
    UPROPERTY(EditAnywhere, Category="Shoot", meta=(EditCondition="bHasShootData")) float ShootWeightB = 0;
    UPROPERTY(EditAnywhere, Category="Shoot", meta=(EditCondition="bHasShootData")) float ShootLifeMax = 0;
    UPROPERTY(EditAnywhere, Category="Shoot", meta=(EditCondition="bHasShootData")) float ShootGravityScale = 0;
    UPROPERTY(EditAnywhere, Category="Shoot", meta=(EditCondition="bHasShootData")) float ShootDynamicExtField4 = 0;
    UPROPERTY(EditAnywhere, Category="Shoot", meta=(EditCondition="bHasShootData")) float ShootDynamicExtField8 = 0;
    UPROPERTY(EditAnywhere, Category="Shoot", meta=(EditCondition="bHasShootData")) int32 ShootField350 = 0;
    UPROPERTY(EditAnywhere, Category="Shoot", meta=(EditCondition="bHasShootData")) int32 ShootField354 = 0;
    UPROPERTY(EditAnywhere, Category="Shoot", meta=(EditCondition="bHasShootData")) int32 ShootTextureIndex = -1;
    UPROPERTY(EditAnywhere, Category="Shoot", meta=(EditCondition="bHasShootData")) int32 ShootMeshIndex = -1;
    UPROPERTY(EditAnywhere, Category="Shoot", meta=(EditCondition="bHasShootData")) float ShootField3C0 = 0;
    UPROPERTY(EditAnywhere, Category="Shoot", meta=(EditCondition="bHasShootData")) float ShootField3C4 = 0;
    UPROPERTY(EditAnywhere, Category="Shoot", meta=(EditCondition="bHasShootData")) float ShootField3C8 = 0;
    UPROPERTY(EditAnywhere, Category="Shoot", meta=(EditCondition="bHasShootData")) float ShootField3CC = 0;
    UPROPERTY(EditAnywhere, Category="Shoot", meta=(EditCondition="bHasShootData")) int32 ShootField3D0 = 0;
    UPROPERTY(EditAnywhere, Category="Shoot", meta=(EditCondition="bHasShootData")) float ShootField3D4 = 0;
    UPROPERTY(EditAnywhere, Category="Shoot", meta=(EditCondition="bHasShootData")) float ShootField3D8 = 0;
    UPROPERTY(EditAnywhere, Category="Shoot", meta=(EditCondition="bHasShootData")) float ShootField3DC = 0;
    UPROPERTY(EditAnywhere, Category="Shoot", meta=(EditCondition="bHasShootData")) float ShootField3E0 = 0;
    UPROPERTY(EditAnywhere, Category="Shoot", meta=(EditCondition="bHasShootData")) int32 ShootField3E8 = 0;
    UPROPERTY(EditAnywhere, Category="Shoot", meta=(EditCondition="bHasShootData")) int32 ShootField3E4 = 0;
    UPROPERTY(EditAnywhere, Category="Shoot", meta=(EditCondition="bHasShootData")) int32 ShootField3EC = 0;
    UPROPERTY(EditAnywhere, Category="Shoot", meta=(EditCondition="bHasShootData")) int32 ShootField3F0 = 0;
    UPROPERTY(EditAnywhere, Category="Shoot", meta=(EditCondition="bHasShootData")) FKyaAddOnGenerator ShootAddOn;
    UPROPERTY(VisibleAnywhere, Category="Ambre") bool bHasAmbreData = false;
    UPROPERTY(EditAnywhere, Category="Ambre", meta=(EditCondition="bHasAmbreData")) int32 AmbreMaterialId = -1;
    UPROPERTY(EditAnywhere, Category="Ambre", meta=(EditCondition="bHasAmbreData")) int32 AmbreEffectType = 0;
    UPROPERTY(EditAnywhere, Category="Ambre", meta=(EditCondition="bHasAmbreData")) int32 AmbreTextureIndex = -1;
    UPROPERTY(EditAnywhere, Category="Ambre", meta=(EditCondition="bHasAmbreData")) int32 AmbreMeshIndex = -1;
    UPROPERTY(EditAnywhere, Category="Ambre", meta=(EditCondition="bHasAmbreData")) int32 AmbreField1D0 = 0;
    UPROPERTY(EditAnywhere, Category="Ambre", meta=(EditCondition="bHasAmbreData")) FKyaSwitch AmbreSwitch;
    UPROPERTY(VisibleAnywhere, Category="Stiller") bool bHasStillerData = false;
    UPROPERTY(EditAnywhere, Category="Stiller", meta=(EditCondition="bHasStillerData")) float StillerVisionRange = 0;
    UPROPERTY(EditAnywhere, Category="Stiller", meta=(EditCondition="bHasStillerData")) float StillerVisionHalfAngle = 0;
    UPROPERTY(EditAnywhere, Category="Stiller", meta=(EditCondition="bHasStillerData")) float StillerVisionField38 = 0;
    UPROPERTY(EditAnywhere, Category="Stiller", meta=(EditCondition="bHasStillerData")) float StillerVisionField3C = 0;
    UPROPERTY(EditAnywhere, Category="Stiller", meta=(EditCondition="bHasStillerData")) int32 StillerVisionFlags = 0;
    UPROPERTY(EditAnywhere, Category="Stiller", meta=(EditCondition="bHasStillerData")) int32 StillerField1D0 = 0;
    UPROPERTY(EditAnywhere, Category="Stiller", meta=(EditCondition="bHasStillerData")) int32 StillerField1D4 = 0;
    UPROPERTY(EditAnywhere, Category="Stiller", meta=(EditCondition="bHasStillerData")) int32 StillerField1D8 = 0;
    UPROPERTY(EditAnywhere, Category="Stiller", meta=(EditCondition="bHasStillerData")) int32 StillerField1DC = 0;
    UPROPERTY(EditAnywhere, Category="Stiller", meta=(EditCondition="bHasStillerData")) int32 StillerPathIndex = -1;
    UPROPERTY(EditAnywhere, Category="Stiller", meta=(EditCondition="bHasStillerData")) float StillerField1E0 = 0;
    UPROPERTY(EditAnywhere, Category="Stiller", meta=(EditCondition="bHasStillerData")) float StillerLifeMax = 0;
    UPROPERTY(EditAnywhere, Category="Stiller", meta=(EditCondition="bHasStillerData")) float StillerField1EC = 0;
    UPROPERTY(EditAnywhere, Category="Stiller", meta=(EditCondition="bHasStillerData")) float StillerField1F0 = 0;
    UPROPERTY(EditAnywhere, Category="Stiller", meta=(EditCondition="bHasStillerData")) float StillerField1F4 = 0;
    UPROPERTY(EditAnywhere, Category="Stiller", meta=(EditCondition="bHasStillerData")) int32 StillerField1F8 = 0;
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Stiller", meta=(EditCondition="bHasStillerData")) TArray<int32> StillerActorRefs;
    UPROPERTY(EditAnywhere, Category="Stiller", meta=(EditCondition="bHasStillerData")) int32 StillerField200 = 0;
    UPROPERTY(EditAnywhere, Category="Stiller", meta=(EditCondition="bHasStillerData")) FKyaSwitch StillerSwitch208;
    UPROPERTY(EditAnywhere, Category="Stiller", meta=(EditCondition="bHasStillerData")) FKyaSwitch StillerSwitch210;
    UPROPERTY(EditAnywhere, Category="Stiller", meta=(EditCondition="bHasStillerData")) int32 StillerField214 = 0;
    UPROPERTY(EditAnywhere, Category="Stiller", meta=(EditCondition="bHasStillerData")) FKyaAddOnGenerator StillerAddOn;
    UPROPERTY(VisibleAnywhere, Category="Rune") bool bHasRuneData = false;
    UPROPERTY(EditAnywhere, Category="Rune", meta=(EditCondition="bHasRuneData")) int32 RuneCinematicId = -1;
    UPROPERTY(EditAnywhere, Category="Rune", meta=(EditCondition="bHasRuneData")) float RuneTriggerDistance = 0;
    UPROPERTY(EditAnywhere, Category="Rune", meta=(EditCondition="bHasRuneData")) int32 RuneScenVarId = 0;
    UPROPERTY(EditAnywhere, Category="Rune", meta=(EditCondition="bHasRuneData")) int32 RuneFxId = 0;
    UPROPERTY(EditAnywhere, Category="Rune", meta=(EditCondition="bHasRuneData")) int32 RuneMaterialIdA = -1;
    UPROPERTY(EditAnywhere, Category="Rune", meta=(EditCondition="bHasRuneData")) int32 RuneMaterialIdB = -1;
    UPROPERTY(VisibleAnywhere, Category="Bonus fountain") bool bHasBonusFountainData = false;
    UPROPERTY(EditAnywhere, Category="Bonus fountain", meta=(EditCondition="bHasBonusFountainData")) FKyaAddOnGenerator BonusFountainAddOn;
    UPROPERTY(VisibleAnywhere, Category="Rope") bool bHasRopeData = false;
    UPROPERTY(EditAnywhere, Category="Rope", meta=(EditCondition="bHasRopeData")) int32 RopePathIndex = -1;
    UPROPERTY(EditAnywhere, Category="Rope", meta=(EditCondition="bHasRopeData")) float RopeScale = 0;
    UPROPERTY(EditAnywhere, Category="Rope", meta=(EditCondition="bHasRopeData")) float RopeField17C = 0;
    UPROPERTY(EditAnywhere, Category="Rope", meta=(EditCondition="bHasRopeData")) int32 RopeMaterialId = -1;
    UPROPERTY(EditAnywhere, Category="Rope", meta=(EditCondition="bHasRopeData")) int32 RopeColor = 0;
    UPROPERTY(EditAnywhere, Category="Rope", meta=(EditCondition="bHasRopeData")) int32 RopeSoundRef = -1;
    UPROPERTY(EditAnywhere, Category="Rope", meta=(EditCondition="bHasRopeData")) int32 RopeField194 = 0;
    UPROPERTY(EditAnywhere, Category="Rope", meta=(EditCondition="bHasRopeData")) int32 RopeActorRefA = -1;
    UPROPERTY(EditAnywhere, Category="Rope", meta=(EditCondition="bHasRopeData")) int32 RopeActorRefB = -1;
    UPROPERTY(EditAnywhere, Category="Rope", meta=(EditCondition="bHasRopeData")) FKyaSwitch RopeSwitch;
    UPROPERTY(EditAnywhere, Category="Rope", meta=(EditCondition="bHasRopeData")) int32 RopeFlags = 0;
    UPROPERTY(VisibleAnywhere, Category="Gravity aware") bool bHasGravityAwareData = false;
    UPROPERTY(EditAnywhere, Category="Gravity aware", meta=(EditCondition="bHasGravityAwareData")) float GravityWeightB = 0;
    UPROPERTY(EditAnywhere, Category="Gravity aware", meta=(EditCondition="bHasGravityAwareData")) float GravityLifeMax = 0;
    UPROPERTY(EditAnywhere, Category="Gravity aware", meta=(EditCondition="bHasGravityAwareData")) float GravityScale = 0;
    UPROPERTY(EditAnywhere, Category="Gravity aware", meta=(EditCondition="bHasGravityAwareData")) float GravityDynamicExtField4 = 0;
    UPROPERTY(EditAnywhere, Category="Gravity aware", meta=(EditCondition="bHasGravityAwareData")) float GravityDynamicExtField8 = 0;
    UPROPERTY(VisibleAnywhere, Category="Nativ command") bool bHasNativCmdData = false;
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Nativ command", meta=(EditCondition="bHasNativCmdData")) TArray<int32> NativCmdActorRefs;
    UPROPERTY(EditAnywhere, Category="Nativ command", meta=(EditCondition="bHasNativCmdData")) int32 NativCmdField164 = 0;
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Nativ command", meta=(EditCondition="bHasNativCmdData")) TArray<FKyaNativCmdAddOn> NativCmdAddOns;
    UPROPERTY(VisibleAnywhere, Category="Companion") bool bHasCompanionData = false;
    UPROPERTY(EditAnywhere, Category="Companion", meta=(EditCondition="bHasCompanionData")) float CompanionWeightB = 0;
    UPROPERTY(EditAnywhere, Category="Companion", meta=(EditCondition="bHasCompanionData")) float CompanionLifeMax = 0;
    UPROPERTY(EditAnywhere, Category="Companion", meta=(EditCondition="bHasCompanionData")) float CompanionGravityScale = 0;
    UPROPERTY(EditAnywhere, Category="Companion", meta=(EditCondition="bHasCompanionData")) float CompanionDynamicExtField4 = 0;
    UPROPERTY(EditAnywhere, Category="Companion", meta=(EditCondition="bHasCompanionData")) float CompanionDynamicExtField8 = 0;
    UPROPERTY(EditAnywhere, Category="Companion", meta=(EditCondition="bHasCompanionData")) int32 CompanionField358 = 0;
    UPROPERTY(EditAnywhere, Category="Companion", meta=(EditCondition="bHasCompanionData")) int32 CompanionMaterial35C = 0;
    UPROPERTY(EditAnywhere, Category="Companion", meta=(EditCondition="bHasCompanionData")) int32 CompanionField360 = 0;
    UPROPERTY(VisibleAnywhere, Category="Bridge") bool bHasBridgeData = false;
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Bridge", meta=(EditCondition="bHasBridgeData")) TArray<int32> BridgeActorRefs;
    UPROPERTY(EditAnywhere, Category="Bridge", meta=(EditCondition="bHasBridgeData")) int32 BridgeWaypointA = -1;
    UPROPERTY(EditAnywhere, Category="Bridge", meta=(EditCondition="bHasBridgeData")) int32 BridgeWaypointB = -1;
    UPROPERTY(EditAnywhere, Category="Bridge", meta=(EditCondition="bHasBridgeData")) float BridgeField330 = 0;
    UPROPERTY(EditAnywhere, Category="Bridge", meta=(EditCondition="bHasBridgeData")) float BridgeField334 = 0;
    UPROPERTY(EditAnywhere, Category="Bridge", meta=(EditCondition="bHasBridgeData")) float BridgeField33C = 0;
    UPROPERTY(EditAnywhere, Category="Bridge", meta=(EditCondition="bHasBridgeData")) float BridgeField340 = 0;
    UPROPERTY(EditAnywhere, Category="Bridge", meta=(EditCondition="bHasBridgeData")) float BridgeField338 = 0;
    UPROPERTY(EditAnywhere, Category="Bridge", meta=(EditCondition="bHasBridgeData")) int32 BridgeField344 = 0;
    UPROPERTY(EditAnywhere, Category="Bridge", meta=(EditCondition="bHasBridgeData")) int32 BridgeConfigFlags = 0;
    UPROPERTY(EditAnywhere, Category="Bridge", meta=(EditCondition="bHasBridgeData")) int32 BridgeField348 = 0;
    UPROPERTY(EditAnywhere, Category="Bridge", meta=(EditCondition="bHasBridgeData")) float BridgeStepCollisionScale = 0;
    UPROPERTY(VisibleAnywhere, Category="Aton") bool bHasAtonData = false;
    UPROPERTY(EditAnywhere, Category="Aton", meta=(EditCondition="bHasAtonData")) float AtonWeightB = 0;
    UPROPERTY(EditAnywhere, Category="Aton", meta=(EditCondition="bHasAtonData")) float AtonLifeMax = 0;
    UPROPERTY(EditAnywhere, Category="Aton", meta=(EditCondition="bHasAtonData")) float AtonGravityScale = 0;
    UPROPERTY(EditAnywhere, Category="Aton", meta=(EditCondition="bHasAtonData")) float AtonDynamicExtField4 = 0;
    UPROPERTY(EditAnywhere, Category="Aton", meta=(EditCondition="bHasAtonData")) float AtonDynamicExtField8 = 0;
    UPROPERTY(EditAnywhere, Category="Aton", meta=(EditCondition="bHasAtonData")) int32 AtonField354 = 0;
    UPROPERTY(EditAnywhere, Category="Aton", meta=(EditCondition="bHasAtonData")) int32 AtonField358 = 0;
    UPROPERTY(EditAnywhere, Category="Aton", meta=(EditCondition="bHasAtonData")) int32 AtonField35C = 0;
    UPROPERTY(EditAnywhere, Category="Aton", meta=(EditCondition="bHasAtonData")) int32 AtonField360 = 0;
    UPROPERTY(EditAnywhere, Category="Aton", meta=(EditCondition="bHasAtonData")) int32 AtonEffectType = 0;
    UPROPERTY(EditAnywhere, Category="Aton", meta=(EditCondition="bHasAtonData")) int32 AtonTextureIndex = -1;
    UPROPERTY(EditAnywhere, Category="Aton", meta=(EditCondition="bHasAtonData")) int32 AtonMeshIndex = -1;
    UPROPERTY(EditAnywhere, Category="Aton", meta=(EditCondition="bHasAtonData")) int32 AtonTrailMaterialId = -1;
    UPROPERTY(EditAnywhere, Category="Aton", meta=(EditCondition="bHasAtonData")) float AtonPathDefaultDelay = 0;
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Aton", meta=(EditCondition="bHasAtonData")) TArray<int32> AtonPathIndices;
    UPROPERTY(EditAnywhere, Category="Aton", meta=(EditCondition="bHasAtonData")) int32 AtonAddOnField18 = 0;
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Aton", meta=(EditCondition="bHasAtonData")) TArray<FKyaAddOnSubObj> AtonOtherSubObjs;
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Aton", meta=(EditCondition="bHasAtonData")) TArray<FKyaAtonSubObj> AtonSubObjs;
    UPROPERTY(EditAnywhere, Category="Aton", meta=(EditCondition="bHasAtonData")) float AtonAddOnField24 = 0;
    UPROPERTY(VisibleAnywhere, Category="Boomy") bool bHasBoomyData = false;
    UPROPERTY(EditAnywhere, Category="Boomy", meta=(EditCondition="bHasBoomyData")) float BoomyWeight = 0;
    UPROPERTY(EditAnywhere, Category="Boomy", meta=(EditCondition="bHasBoomyData")) float BoomyVisionRange = 0;
    UPROPERTY(EditAnywhere, Category="Boomy", meta=(EditCondition="bHasBoomyData")) float BoomyVisionAngle = 0;
    UPROPERTY(EditAnywhere, Category="Boomy", meta=(EditCondition="bHasBoomyData")) float BoomyVisionField38 = 0;
    UPROPERTY(EditAnywhere, Category="Boomy", meta=(EditCondition="bHasBoomyData")) float BoomyVisionField3C = 0;
    UPROPERTY(EditAnywhere, Category="Boomy", meta=(EditCondition="bHasBoomyData")) int32 BoomyVisionFlags = 0;
    UPROPERTY(EditAnywhere, Category="Boomy", meta=(EditCondition="bHasBoomyData")) int32 BoomySound2B8 = -1;
    UPROPERTY(EditAnywhere, Category="Boomy", meta=(EditCondition="bHasBoomyData")) int32 BoomySound2BC = -1;
    UPROPERTY(EditAnywhere, Category="Boomy", meta=(EditCondition="bHasBoomyData")) float BoomyField744 = 0;
    UPROPERTY(EditAnywhere, Category="Boomy", meta=(EditCondition="bHasBoomyData")) int32 BoomyTailWidthAxis = 0;
    UPROPERTY(EditAnywhere, Category="Boomy", meta=(EditCondition="bHasBoomyData")) float BoomyField740 = 0;
    UPROPERTY(EditAnywhere, Category="Boomy", meta=(EditCondition="bHasBoomyData")) int32 BoomyTailSegments = 0;
    UPROPERTY(EditAnywhere, Category="Boomy", meta=(EditCondition="bHasBoomyData")) int32 BoomyParticle3D0 = -1;
    UPROPERTY(EditAnywhere, Category="Boomy", meta=(EditCondition="bHasBoomyData")) int32 BoomyParticle3D4 = -1;
    UPROPERTY(EditAnywhere, Category="Boomy", meta=(EditCondition="bHasBoomyData")) int32 BoomyParticle3DC = -1;
    UPROPERTY(EditAnywhere, Category="Boomy", meta=(EditCondition="bHasBoomyData")) int32 BoomyField3CC = 0;
    UPROPERTY(EditAnywhere, Category="Boomy", meta=(EditCondition="bHasBoomyData")) int32 BoomyTextureIndex = -1;
    UPROPERTY(EditAnywhere, Category="Boomy", meta=(EditCondition="bHasBoomyData")) int32 BoomyMeshIndex = -1;
    UPROPERTY(EditAnywhere, Category="Boomy", meta=(EditCondition="bHasBoomyData")) FKyaActorCamera BoomyCamera;
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Boomy", meta=(EditCondition="bHasBoomyData")) TArray<FKyaBoomyType> BoomyTypes;
    UPROPERTY(VisibleAnywhere, Category="Hero") bool bHasKimData = false;
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Hero", meta=(EditCondition="bHasKimData")) TArray<int32> KimBoneKeys;
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Hero", meta=(EditCondition="bHasKimData")) TArray<int32> KimClimbZones;
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Hero", meta=(EditCondition="bHasKimData")) TArray<FKyaHeroTutorialZone> KimTutorialZones;
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Hero", meta=(EditCondition="bHasKimData")) TArray<FKyaHeroProjectileZone> KimProjectileZones;
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Hero", meta=(EditCondition="bHasKimData")) TArray<FKyaHeroZonePair> KimBoomyZonePairs;
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Hero", meta=(EditCondition="bHasKimData")) TArray<int32> KimZoneStream;
    UPROPERTY(EditAnywhere, Category="Hero", meta=(EditCondition="bHasKimData")) int32 KimMaterialId = -1;
    UPROPERTY(EditAnywhere, Category="Hero", meta=(EditCondition="bHasKimData")) int32 KimFxType = 0;
    UPROPERTY(EditAnywhere, Category="Hero", meta=(EditCondition="bHasKimData")) float KimJokeWarnRadius = 0;
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Hero", meta=(EditCondition="bHasKimData")) TArray<FKyaActorCamera> KimCameras;
    UPROPERTY(EditAnywhere, Category="Hero", meta=(EditCondition="bHasKimData")) int32 KimField15D4 = 0;
    UPROPERTY(EditAnywhere, Category="Hero", meta=(EditCondition="bHasKimData")) int32 KimFxInitType = 0;
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Hero", meta=(EditCondition="bHasKimData")) TArray<int32> KimFxHandleTypes;
    UPROPERTY(VisibleAnywhere, Category="Wolfen") bool bHasWolfenData = false;
    /** Typed blow database. */
    UPROPERTY(EditAnywhere, Category="Fighter", meta=(EditCondition="bHasWolfenData || bHasKimData")) float FighterWeightB = 0;
    UPROPERTY(EditAnywhere, Category="Fighter", meta=(EditCondition="bHasWolfenData || bHasKimData")) float FighterLifeMax = 0;
    UPROPERTY(EditAnywhere, Category="Fighter", meta=(EditCondition="bHasWolfenData || bHasKimData")) float FighterGravityScale = 0;
    UPROPERTY(EditAnywhere, Category="Fighter", meta=(EditCondition="bHasWolfenData || bHasKimData")) float FighterDynamicExtField4 = 0;
    UPROPERTY(EditAnywhere, Category="Fighter", meta=(EditCondition="bHasWolfenData || bHasKimData")) float FighterDynamicExtField8 = 0;
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Fighter blows", meta=(EditCondition="bHasWolfenData || bHasKimData")) TArray<FKyaFighterBlow> FighterBlows;    UPROPERTY(EditAnywhere, EditFixedSize, Category="Fighter grabs", meta=(EditCondition="bHasWolfenData || bHasKimData")) TArray<FKyaFighterGrab> FighterGrabs;
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Fighter grabs", meta=(EditCondition="bHasWolfenData || bHasKimData")) TArray<FKyaFighterGrabReact> FighterGrabReacts;
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Fighter combos", meta=(EditCondition="bHasWolfenData || bHasKimData")) TArray<FKyaFighterCombo> FighterCombos;
    /** Raw bytes between the typed databases (should be empty for LEVEL_1). */
    UPROPERTY(EditAnywhere, Category="Fighter", meta=(EditCondition="bHasWolfenData || bHasKimData")) TArray<uint8> FighterPrefixRaw;
    UPROPERTY(EditAnywhere, Category="Fighter", meta=(EditCondition="bHasWolfenData || bHasKimData")) int32 FighterInputAnalyserFlag = 0;
    UPROPERTY(EditAnywhere, Category="Fighter", meta=(EditCondition="bHasWolfenData || bHasKimData")) int32 FighterCommandMask = 0;
    UPROPERTY(EditAnywhere, Category="Fighter", meta=(EditCondition="bHasWolfenData || bHasKimData")) float FighterField7C0 = 0;
    UPROPERTY(EditAnywhere, Category="Fighter", meta=(EditCondition="bHasWolfenData || bHasKimData")) float FighterField7C4 = 0;
    UPROPERTY(EditAnywhere, Category="Fighter", meta=(EditCondition="bHasWolfenData || bHasKimData")) float FighterField7D4 = 0;
    UPROPERTY(EditAnywhere, Category="Fighter", meta=(EditCondition="bHasWolfenData || bHasKimData")) float FighterField7CC = 0;
    UPROPERTY(EditAnywhere, Category="Fighter", meta=(EditCondition="bHasWolfenData || bHasKimData")) float FighterAnatomy20 = 0;
    UPROPERTY(EditAnywhere, Category="Fighter", meta=(EditCondition="bHasWolfenData || bHasKimData")) float FighterAnatomy28 = 0;
    UPROPERTY(EditAnywhere, Category="Fighter", meta=(EditCondition="bHasWolfenData || bHasKimData")) float FighterAnatomy24 = 0;
    UPROPERTY(EditAnywhere, Category="Fighter", meta=(EditCondition="bHasWolfenData || bHasKimData")) float FighterAnatomy2C = 0;
    UPROPERTY(EditAnywhere, Category="Fighter", meta=(EditCondition="bHasWolfenData || bHasKimData")) FVector FighterField810A = FVector::ZeroVector;
    UPROPERTY(EditAnywhere, Category="Fighter", meta=(EditCondition="bHasWolfenData || bHasKimData")) FVector FighterField810B = FVector::ZeroVector;
    UPROPERTY(EditAnywhere, Category="Fighter", meta=(EditCondition="bHasWolfenData || bHasKimData")) int32 FighterField800 = 0;
    UPROPERTY(EditAnywhere, Category="Fighter", meta=(EditCondition="bHasWolfenData || bHasKimData")) int32 FighterFightFlags = 0;
    UPROPERTY(EditAnywhere, Category="Fighter", meta=(EditCondition="bHasWolfenData || bHasKimData")) float FighterField50C = 0;
    UPROPERTY(EditAnywhere, Category="Fighter", meta=(EditCondition="bHasWolfenData || bHasKimData")) float FighterField510 = 0;
    UPROPERTY(EditAnywhere, Category="Fighter", meta=(EditCondition="bHasWolfenData || bHasKimData")) float FighterField514 = 0;
    UPROPERTY(EditAnywhere, Category="Fighter", meta=(EditCondition="bHasWolfenData || bHasKimData")) int32 FighterField518 = 0;
    UPROPERTY(EditAnywhere, Category="Fighter", meta=(EditCondition="bHasWolfenData || bHasKimData")) int32 FighterField58 = 0;
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Fighter", meta=(EditCondition="bHasWolfenData || bHasKimData")) TArray<int32> FighterField51C;
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Fighter", meta=(EditCondition="bHasWolfenData || bHasKimData")) TArray<int32> FighterField53C;
    UPROPERTY(EditAnywhere, Category="Fighter", meta=(EditCondition="bHasWolfenData || bHasKimData")) int32 FighterStaticMeshTextureIndex = -1;
    UPROPERTY(EditAnywhere, Category="Fighter", meta=(EditCondition="bHasWolfenData || bHasKimData")) int32 FighterStaticMeshMeshIndex = -1;
    UPROPERTY(EditAnywhere, Category="Fighter", meta=(EditCondition="bHasWolfenData || bHasKimData")) int32 FighterField5F0 = 0;
    UPROPERTY(EditAnywhere, Category="Fighter", meta=(EditCondition="bHasWolfenData || bHasKimData")) int32 FighterWeaponActorIndex = -1;
    UPROPERTY(EditAnywhere, Category="Wolfen", meta=(EditCondition="bHasWolfenData")) int32 WolfenFieldB74 = 0;
    UPROPERTY(EditAnywhere, Category="Wolfen", meta=(EditCondition="bHasWolfenData")) float WolfenVisionRange = 0;
    UPROPERTY(EditAnywhere, Category="Wolfen", meta=(EditCondition="bHasWolfenData")) float WolfenVisionFov = 0;
    UPROPERTY(EditAnywhere, Category="Wolfen", meta=(EditCondition="bHasWolfenData")) float WolfenVisionField38 = 0;
    UPROPERTY(EditAnywhere, Category="Wolfen", meta=(EditCondition="bHasWolfenData")) float WolfenVisionField3C = 0;
    UPROPERTY(EditAnywhere, Category="Wolfen", meta=(EditCondition="bHasWolfenData")) int32 WolfenVisionFlags = 0;
    UPROPERTY(EditAnywhere, Category="Wolfen", meta=(EditCondition="bHasWolfenData")) int32 WolfenStartSectorId = 0;
    UPROPERTY(EditAnywhere, Category="Wolfen", meta=(EditCondition="bHasWolfenData")) float WolfenHearingRangeSquared = 0;
    UPROPERTY(EditAnywhere, Category="Wolfen", meta=(EditCondition="bHasWolfenData")) float WolfenHearingMaxHeight = 0;
    UPROPERTY(EditAnywhere, Category="Wolfen", meta=(EditCondition="bHasWolfenData")) float WolfenVisionDetectionRange = 0;
    UPROPERTY(EditAnywhere, Category="Wolfen", meta=(EditCondition="bHasWolfenData")) float WolfenFieldCF4 = 0;
    UPROPERTY(EditAnywhere, Category="Wolfen", meta=(EditCondition="bHasWolfenData")) float WolfenWalkSpeed = 0;
    UPROPERTY(EditAnywhere, Category="Wolfen", meta=(EditCondition="bHasWolfenData")) float WolfenWalkAcceleration = 0;
    UPROPERTY(EditAnywhere, Category="Wolfen", meta=(EditCondition="bHasWolfenData")) float WolfenWalkRotSpeed = 0;
    UPROPERTY(EditAnywhere, Category="Wolfen", meta=(EditCondition="bHasWolfenData")) float WolfenRunSpeedScale = 0;
    UPROPERTY(EditAnywhere, Category="Wolfen", meta=(EditCondition="bHasWolfenData")) float WolfenDefaultRunSpeed = 0;
    UPROPERTY(EditAnywhere, Category="Wolfen", meta=(EditCondition="bHasWolfenData")) float WolfenRunAcceleration = 0;
    UPROPERTY(EditAnywhere, Category="Wolfen", meta=(EditCondition="bHasWolfenData")) float WolfenRotRunSpeed = 0;
    UPROPERTY(EditAnywhere, Category="Wolfen", meta=(EditCondition="bHasWolfenData")) int32 WolfenMagicRequired = 0;
    UPROPERTY(EditAnywhere, Category="Wolfen", meta=(EditCondition="bHasWolfenData")) int32 WolfenFieldD30 = 0;
    UPROPERTY(EditAnywhere, Category="Wolfen", meta=(EditCondition="bHasWolfenData")) float WolfenFieldB90 = 0;
    UPROPERTY(EditAnywhere, Category="Wolfen", meta=(EditCondition="bHasWolfenData")) float WolfenFieldB94 = 0;
    UPROPERTY(EditAnywhere, Category="Wolfen", meta=(EditCondition="bHasWolfenData")) float WolfenFieldB98 = 0;
    UPROPERTY(EditAnywhere, Category="Wolfen fight", meta=(EditCondition="bHasWolfenData")) FKyaWolfenFightParam WolfenFightParamData;
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
    static constexpr int32 CurrentImportSchemaVersion = 50;
    bool HasCurrentImportSchema() const { return ImportSchemaVersion == CurrentImportSchemaVersion; }

    /** Version of the parsed properties; older assets must be reimported. */
    UPROPERTY(VisibleAnywhere, Category="Kya")
    int32 ImportSchemaVersion = 0;

    UPROPERTY(VisibleAnywhere, Category="Kya")
    FString SourceFilename;

    /** Level light archive entries (THGL) in serialized order. */
    UPROPERTY(VisibleAnywhere, Category="Light")
    bool bHasLights = false;
    UPROPERTY(EditAnywhere, EditFixedSize, Category="Light")
    TArray<FKyaLight> Lights;

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
