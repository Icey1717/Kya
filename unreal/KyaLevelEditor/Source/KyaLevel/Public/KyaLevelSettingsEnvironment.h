#pragma once

#include "CoreMinimal.h"
#include "KyaLevelBank.h"
#include "KyaLevelSettingsEnvironment.generated.h"

/** Editable settings snapshot; placement arrays deliberately have no property nodes. */
USTRUCT(BlueprintType)
struct KYALEVEL_API FKyaLevelSettingsEnvironment
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

};
