#include "KyaSpatialRecordHandle.h"

#include "Components/StaticMeshComponent.h"
#include "Components/ArrowComponent.h"
#include "Engine/StaticMesh.h"
#include "KyaActorEditService.h"
#include "KyaActorTransform.h"
#include "KyaLevelBank.h"
#include "KyaLevelPreview.h"
#include "UObject/ConstructorHelpers.h"
#if WITH_EDITOR
#include "Editor.h"
#include "ScopedTransaction.h"
#include "Misc/App.h"
#include "Misc/MessageDialog.h"
#endif

AKyaSpatialRecordHandle::AKyaSpatialRecordHandle()
{
    PrimaryActorTick.bCanEverTick = false;
    bIsEditorOnlyActor = true;

    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    RootComponent = SceneRoot;
    SceneRoot->SetMobility(EComponentMobility::Movable);

    Marker = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Marker"));
    Marker->SetupAttachment(SceneRoot);
    static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    if (SphereMesh.Succeeded()) Marker->SetStaticMesh(SphereMesh.Object);
    Marker->SetRelativeScale3D(FVector(0.12f));
    Marker->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Marker->SetGenerateOverlapEvents(false);
    Marker->CastShadow = false;
    Marker->SetHiddenInGame(true);
    Marker->SetMobility(EComponentMobility::Movable);

    // Forward indicator for waypoint orientation. Path-point handles keep it hidden.
    OrientationArrow = CreateDefaultSubobject<UArrowComponent>(TEXT("OrientationArrow"));
    OrientationArrow->SetupAttachment(SceneRoot);
    OrientationArrow->ArrowColor = FColor(120, 230, 120);
    OrientationArrow->ArrowSize = 0.8f;
    OrientationArrow->ArrowLength = 90.0f;
    OrientationArrow->bHiddenInGame = true;
    OrientationArrow->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    // The handle's rotation maps its local +Y to the stored game forward (the game +Z
    // image after the axis permutation), so the arrow points along +Y locally and the
    // handle/preview transforms carry it into world space.
    OrientationArrow->SetRelativeRotation(FQuat::FindBetweenNormals(
        FVector::ForwardVector, FVector::RightVector));
    OrientationArrow->SetVisibility(false, true);
}

void AKyaSpatialRecordHandle::Initialize(AKyaLevelPreview* InPreview, UKyaLevelBank* InBank,
    EKyaSpatialRecordKind InKind, int32 InRecordIndex, int32 InPointIndex)
{
    Preview = InPreview;
    Bank = InBank;
    Kind = InKind;
    RecordIndex = InRecordIndex;
    PointIndex = InPointIndex;
    EditGeneration = Bank ? Bank->EditGeneration : 0;
    SyncFromBank();
}

void AKyaSpatialRecordHandle::Destroyed()
{
    Preview = nullptr;
    Super::Destroyed();
}

void AKyaSpatialRecordHandle::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);
    SyncFromBank();
}

void AKyaSpatialRecordHandle::MirrorFromBank()
{
    if (!Bank) return;
    EditGeneration = Bank->EditGeneration;
    if (Kind == EKyaSpatialRecordKind::Waypoint)
    {
        if (!Bank->SceneEnvironment.Waypoints.IsValidIndex(RecordIndex)) return;
        const FKyaWaypoint& Waypoint = Bank->SceneEnvironment.Waypoints[RecordIndex];
        WaypointPosition = Waypoint.Position;
        WaypointRotation = Waypoint.Rotation;
        WaypointParameters = Waypoint.Parameters;
        return;
    }
    if (!Bank->SceneEnvironment.Paths.IsValidIndex(RecordIndex)) return;
    const FKyaPath& Path = Bank->SceneEnvironment.Paths[RecordIndex];
    if (!Path.Points.IsValidIndex(PointIndex)) return;
    PathPoint = Path.Points[PointIndex];
    PathMode = Path.Mode;
    PathType = Path.Type;
    PathPathType = Path.PathType;
    bHasRotationRecord = Path.Rotations.IsValidIndex(PointIndex);
    bHasExtraVectorRecord = Path.ExtraVectors.IsValidIndex(PointIndex);
    bHasDelayRecord = Path.Delays.IsValidIndex(PointIndex);
    bHasFlagsRecord = Path.PointFlags.IsValidIndex(PointIndex);
    bHasExtraValueRecord = Path.ExtraValues.IsValidIndex(PointIndex);
    bHasExtraWordRecord = Path.ExtraWords.IsValidIndex(PointIndex);
    PathRotation = bHasRotationRecord ? Path.Rotations[PointIndex] : FVector4(0, 0, 0, 0);
    PathExtraVector = bHasExtraVectorRecord ? Path.ExtraVectors[PointIndex] : FVector4(0, 0, 0, 0);
    PathDelay = bHasDelayRecord ? Path.Delays[PointIndex] : 0.0f;
    PathFlags = Path.PointFlags.IsValidIndex(PointIndex) ? Path.PointFlags[PointIndex] : 0;
    PathExtraValue = Path.ExtraValues.IsValidIndex(PointIndex) ? Path.ExtraValues[PointIndex] : 0.0f;
    PathExtraWord = Path.ExtraWords.IsValidIndex(PointIndex) ? Path.ExtraWords[PointIndex] : 0;
}

void AKyaSpatialRecordHandle::SyncFromBank()
{
    if (!Bank) return;
    bSyncing = true;
    MirrorFromBank();
    FTransform Local = GetActorTransform();
    if (Kind == EKyaSpatialRecordKind::Waypoint)
    {
        if (Bank->SceneEnvironment.Waypoints.IsValidIndex(RecordIndex))
        {
            Local = FTransform(KyaActorTransform::QuatFromGameEuler(WaypointRotation),
                KyaActorTransform::SwapGameAxes(WaypointPosition) * 100.0);
        }
    }
    else if (Bank->SceneEnvironment.Paths.IsValidIndex(RecordIndex) &&
        Bank->SceneEnvironment.Paths[RecordIndex].Points.IsValidIndex(PointIndex))
    {
        const FVector Position(PathPoint.X, PathPoint.Y, PathPoint.Z);
        Local = FTransform(KyaActorTransform::SwapGameAxes(Position) * 100.0);
    }
    SetActorRelativeTransform(Local, false, nullptr, ETeleportType::TeleportPhysics);
    if (OrientationArrow)
    {
        const bool bWaypoint = Kind == EKyaSpatialRecordKind::Waypoint &&
            Bank->SceneEnvironment.Waypoints.IsValidIndex(RecordIndex);
        OrientationArrow->SetVisibility(bWaypoint, true);
    }
    bSyncing = false;

#if WITH_EDITOR
    if (Kind == EKyaSpatialRecordKind::PathPoint && IsValid(Preview))
        Preview->UpdatePathGuidePoint(RecordIndex, PointIndex, Local.GetLocation());
#endif

#if WITH_EDITOR
    // Self-describing labels: the position lets a user identify a record in the
    // Outliner without knowing its numeric index.
    SetGeneratedLabel(MakeGeneratedLabel());
#endif
}

FString AKyaSpatialRecordHandle::MakeGeneratedLabel() const
{
    if (Kind == EKyaSpatialRecordKind::Waypoint)
        return FString::Printf(TEXT("Waypoint %d (%.1f, %.1f, %.1f)"), RecordIndex,
            WaypointPosition.X, WaypointPosition.Y, WaypointPosition.Z);
    return FString::Printf(TEXT("Path %d Point %d (%.1f, %.1f, %.1f)"), RecordIndex, PointIndex,
        PathPoint.X, PathPoint.Y, PathPoint.Z);
}

void AKyaSpatialRecordHandle::SetGeneratedLabel(const FString& Label)
{
    TGuardValue<bool> Guard(bApplyingGeneratedLabel, true);
    if (GetActorLabel() != Label) SetActorLabel(Label);
}

bool AKyaSpatialRecordHandle::IsSelectable() const
{
    if (bSelectionLocked) return false;
    return !(IsValid(Preview) && Preview->bLockGeneratedActorSelection);
}

#if WITH_EDITOR
void AKyaSpatialRecordHandle::PostEditMove(bool bFinished)
{
    if (bFinished) CommitFromActor();
    Super::PostEditMove(bFinished);
}

void AKyaSpatialRecordHandle::CommitFromActor()
{
    if (bSyncing || !Preview || !Bank) return;
    if (!Preview->OwnsGeneratedActor(this))
    {
        UE_LOG(LogTemp, Warning, TEXT("KYA_SPATIAL_STRAY_WRITE_IGNORED bank=%s index=%d"),
            *GetNameSafe(Bank), RecordIndex);
        return;
    }
    const FTransform Local = GetActorTransform().GetRelativeTransform(Preview->GetActorTransform());
    const FVector GamePosition = KyaActorTransform::SwapGameAxes(Local.GetLocation() / 100.0);
    FKyaLevelEditService& Service = FKyaLevelEditService::Get();
    FString Error;
    bool bApplied = false;
    const auto Run = [&](EKyaEditNotify Notify)
    {
        if (Kind == EKyaSpatialRecordKind::Waypoint)
        {
            FKyaWaypointEditPatch Patch;
            if (Bank->SceneEnvironment.Waypoints.IsValidIndex(RecordIndex))
            {
                const FKyaWaypoint& Current = Bank->SceneEnvironment.Waypoints[RecordIndex];
                if (!Current.Position.Equals(GamePosition, 1.0e-4)) Patch.Position = GamePosition;

                // The handle rotation is the Unreal representation of the stored XYZ
                // Euler triple; only pass the axes through the inverse when the quat
                // actually changed, so a pure translate preserves the serialized angles.
                const FQuat LocalRotation = Local.GetRotation().GetNormalized();
                const FQuat CurrentRotation = KyaActorTransform::QuatFromGameEuler(Current.Rotation);
                if (!CurrentRotation.Equals(LocalRotation, 1.0e-5))
                {
                    FVector NewAngles = Current.Rotation;
                    if (!KyaActorTransform::GameEulerFromQuat(LocalRotation, Current.Rotation, NewAngles))
                    {
                        Error = TEXT("Waypoint orientation cannot be represented by the stored Euler triple.");
                        return false;
                    }
                    Patch.Rotation = NewAngles;
                }
            }
            if (Patch.IsEmpty()) return true;
            return Service.ApplyWaypoint(Service.MakeWaypointKey(Bank, RecordIndex), Patch, Notify, &Error);
        }
        if (!Bank->SceneEnvironment.Paths.IsValidIndex(RecordIndex) ||
            !Bank->SceneEnvironment.Paths[RecordIndex].Points.IsValidIndex(PointIndex)) return true;
        const FVector4& Current = Bank->SceneEnvironment.Paths[RecordIndex].Points[PointIndex];
        const FVector4 Target(GamePosition.X, GamePosition.Y, GamePosition.Z, Current.W);
        if (Target.Equals(Current, 1.0e-4)) return true;
        FKyaPathPointEditPatch Patch;
        Patch.Point = Target;
        return Service.ApplyPathPoint(Service.MakePathPointKey(Bank, RecordIndex, PointIndex), Patch, Notify, &Error);
    };

    if (GEditor && !GEditor->IsTransactionActive())
    {
        FScopedTransaction Transaction(NSLOCTEXT("KyaLevelEditor", "KyaSpatialMove", "Move Kya Spatial Record"));
        bApplied = Run(EKyaEditNotify::Deferred);
    }
    else
    {
        bApplied = Run(EKyaEditNotify::Deferred);
    }
    if (!bApplied)
    {
        UE_LOG(LogTemp, Warning, TEXT("KYA_SPATIAL_WRITE_FAILED bank=%s kind=%d index=%d point=%d reason=%s"),
            *GetNameSafe(Bank), int32(Kind), RecordIndex, PointIndex, *Error);
    }
    SyncFromBank();
}

bool AKyaSpatialRecordHandle::CanDeleteSelectedActor(FText& OutReason) const
{
    OutReason = NSLOCTEXT("KyaLevel", "CannotDeleteSpatialHandle",
        "Generated Kya waypoint/path handles represent bank records and cannot be deleted. "
        "Edit the bank asset, or clear the corresponding Spawn ... Handles option on the level preview.");
    return false;
}

void AKyaSpatialRecordHandle::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    const FName CandidateNames[2] = {
        PropertyChangedEvent.MemberProperty ? PropertyChangedEvent.MemberProperty->GetFName() : NAME_None,
        PropertyChangedEvent.Property ? PropertyChangedEvent.Property->GetFName() : NAME_None };
    const auto Matches = [&CandidateNames](FName Name)
    {
        return CandidateNames[0] == Name || CandidateNames[1] == Name;
    };

    // Labels for spatial records are generated from the record kind and indices; an
    // Outliner rename is reverted and explained instead of silently diverging.
    if (!bApplyingGeneratedLabel && Matches(TEXT("ActorLabel")))
    {
        SetGeneratedLabel(MakeGeneratedLabel());
        UE_LOG(LogTemp, Warning, TEXT("KYA_SPATIAL_RENAME_REJECTED bank=%s kind=%d index=%d: labels are generated"),
            *GetNameSafe(Bank), int32(Kind), RecordIndex);
        if (!FApp::IsUnattended())
            FMessageDialog::Open(EAppMsgType::Ok, NSLOCTEXT("KyaLevel", "SpatialRenameRejected",
                "This label is generated from the bank record and cannot be renamed. "
                "The label was restored; the bank was not modified."));
        Super::PostEditChangeProperty(PropertyChangedEvent);
        return;
    }

    FKyaLevelEditService& Service = FKyaLevelEditService::Get();
    FString Error;
    bool bApplied = true;
    if (!bSyncing && Bank && Preview && Preview->OwnsGeneratedActor(this))
    {
        const auto Run = [&](EKyaEditNotify Notify)
        {
            if (Kind == EKyaSpatialRecordKind::Waypoint)
            {
                FKyaWaypointEditPatch Patch;
                if (Matches(GET_MEMBER_NAME_CHECKED(AKyaSpatialRecordHandle, WaypointPosition))) Patch.Position = WaypointPosition;
                else if (Matches(GET_MEMBER_NAME_CHECKED(AKyaSpatialRecordHandle, WaypointRotation))) Patch.Rotation = WaypointRotation;
                else if (Matches(GET_MEMBER_NAME_CHECKED(AKyaSpatialRecordHandle, WaypointParameters))) Patch.Parameters = WaypointParameters;
                if (Patch.IsEmpty()) return true;
                return Service.ApplyWaypoint(Service.MakeWaypointKey(Bank, RecordIndex), Patch, Notify, &Error);
            }
            FKyaPathPointEditPatch Patch;
            if (Matches(GET_MEMBER_NAME_CHECKED(AKyaSpatialRecordHandle, PathPoint))) Patch.Point = PathPoint;
            else if (Matches(GET_MEMBER_NAME_CHECKED(AKyaSpatialRecordHandle, PathRotation))) Patch.Rotation = PathRotation;
            else if (Matches(GET_MEMBER_NAME_CHECKED(AKyaSpatialRecordHandle, PathExtraVector))) Patch.ExtraVector = PathExtraVector;
            else if (Matches(GET_MEMBER_NAME_CHECKED(AKyaSpatialRecordHandle, PathDelay))) Patch.Delay = PathDelay;
            else if (Matches(GET_MEMBER_NAME_CHECKED(AKyaSpatialRecordHandle, PathFlags))) Patch.Flags = PathFlags;
            else if (Matches(GET_MEMBER_NAME_CHECKED(AKyaSpatialRecordHandle, PathExtraValue))) Patch.ExtraValue = PathExtraValue;
            else if (Matches(GET_MEMBER_NAME_CHECKED(AKyaSpatialRecordHandle, PathExtraWord))) Patch.ExtraWord = PathExtraWord;
            if (Patch.IsEmpty()) return true;
            return Service.ApplyPathPoint(Service.MakePathPointKey(Bank, RecordIndex, PointIndex), Patch, Notify, &Error);
        };

        if (GEditor && !GEditor->IsTransactionActive())
        {
            FScopedTransaction Transaction(NSLOCTEXT("KyaLevelEditor", "KyaSpatialProperty", "Edit Kya Spatial Record"));
            bApplied = Run(EKyaEditNotify::Notify);
        }
        else
        {
            bApplied = Run(EKyaEditNotify::Notify);
        }
        if (!bApplied)
            UE_LOG(LogTemp, Warning, TEXT("KYA_SPATIAL_PROPERTY_WRITE_FAILED bank=%s field=%s reason=%s"),
                *GetNameSafe(Bank), *CandidateNames[0].ToString(), *Error);
    }
    if (!bApplied) SyncFromBank();
    Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif

void AKyaSpatialRecordHandle::ApplyToBank()
{
#if WITH_EDITOR
    if (bSyncing || !Bank || !Preview || !Preview->OwnsGeneratedActor(this)) return;
    FKyaLevelEditService& Service = FKyaLevelEditService::Get();
    FString Error;
    if (Kind == EKyaSpatialRecordKind::Waypoint)
    {
        FKyaWaypointEditPatch Patch;
        Patch.Position = WaypointPosition;
        Patch.Rotation = WaypointRotation;
        Patch.Parameters = WaypointParameters;
        if (!Service.ApplyWaypoint(Service.MakeWaypointKey(Bank, RecordIndex), Patch, EKyaEditNotify::Notify, &Error))
            UE_LOG(LogTemp, Warning, TEXT("KYA_SPATIAL_APPLY_FAILED bank=%s reason=%s"), *GetNameSafe(Bank), *Error);
    }
    else
    {
        FKyaPathPointEditPatch Patch;
        Patch.Point = PathPoint;
        if (bHasRotationRecord) Patch.Rotation = PathRotation;
        if (bHasExtraVectorRecord) Patch.ExtraVector = PathExtraVector;
        if (bHasDelayRecord) Patch.Delay = PathDelay;
        if (bHasFlagsRecord) Patch.Flags = PathFlags;
        if (bHasExtraValueRecord) Patch.ExtraValue = PathExtraValue;
        if (bHasExtraWordRecord) Patch.ExtraWord = PathExtraWord;
        if (!Service.ApplyPathPoint(Service.MakePathPointKey(Bank, RecordIndex, PointIndex), Patch,
            EKyaEditNotify::Notify, &Error))
            UE_LOG(LogTemp, Warning, TEXT("KYA_SPATIAL_APPLY_FAILED bank=%s reason=%s"), *GetNameSafe(Bank), *Error);
    }
    SyncFromBank();
#endif
}
