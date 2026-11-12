#include "KyaSceneVisualizer.h"

#include "Components/ArrowComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
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

namespace
{
UStaticMesh* LoadRuntimeMesh(const TCHAR* Path)
{
    return LoadObject<UStaticMesh>(nullptr, Path);
}
}

AKyaSceneVisualizer::AKyaSceneVisualizer()
{
    PrimaryActorTick.bCanEverTick = false;
    bIsEditorOnlyActor = true;

    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    RootComponent = SceneRoot;
    SceneRoot->SetMobility(EComponentMobility::Movable);

    Shape = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Shape"));
    Shape->SetupAttachment(SceneRoot);
    Shape->SetStaticMesh(LoadRuntimeMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere")));
    Shape->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Shape->SetGenerateOverlapEvents(false);
    Shape->CastShadow = false;
    Shape->SetHiddenInGame(true);
    Shape->SetMobility(EComponentMobility::Movable);
    Shape->bSelectable = true;

    DirectionArrow = CreateDefaultSubobject<UArrowComponent>(TEXT("DirectionArrow"));
    DirectionArrow->SetupAttachment(SceneRoot);
    DirectionArrow->ArrowColor = FColor(255, 200, 60);
    DirectionArrow->ArrowSize = 1.0f;
    DirectionArrow->bHiddenInGame = true;
    DirectionArrow->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    DirectionArrow->SetVisibility(false, true);
}

void AKyaSceneVisualizer::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);
    SyncFromBank();
}

void AKyaSceneVisualizer::Destroyed()
{
    Preview = nullptr;
    Super::Destroyed();
}

void AKyaSceneVisualizer::InitializeLight(AKyaLevelPreview* InPreview, UKyaLevelBank* InBank, int32 InLightIndex)
{
    Preview = InPreview;
    Bank = InBank;
    Kind = EKyaVisualizerKind::Light;
    RecordIndex = InLightIndex;
    bRecordEditable = true;
    Semantics = TEXT("Verified serialized light record. Position, direction, colors, multipliers and falloff/FOV "
        "values are native; the sphere/arrow shape is an approximate preview.");
    SyncFromBank();
}

void AKyaSceneVisualizer::InitializeCamera(AKyaLevelPreview* InPreview, UKyaLevelBank* InBank)
{
    Preview = InPreview;
    Bank = InBank;
    Kind = EKyaVisualizerKind::MainCamera;
    bRecordEditable = true;
    Semantics = TEXT("Verified serialized main-camera fields. The frustum/arrow is an approximate preview shape; "
        "it does not reproduce the native camera rig.");
    SyncFromBank();
}

void AKyaSceneVisualizer::InitializeActorLink(AKyaLevelPreview* InPreview, UKyaLevelBank* InBank,
    int32 InActorIndex, FName InReferenceField, const FVector& GameStart, const FVector& GameEnd)
{
    Preview = InPreview;
    Bank = InBank;
    Kind = EKyaVisualizerKind::ActorLink;
    RecordIndex = InActorIndex;
    ReferenceField = InReferenceField;
    LinkStart = GameStart;
    LinkEnd = GameEnd;
    bRecordEditable = false;
    Semantics = TEXT("Derived link: a resolved actor reference between two placements. No native link geometry is "
        "serialized; the line and its midpoint are approximate diagnostics and are read-only.");
    SyncFromBank();
}

void AKyaSceneVisualizer::InitializeTrigger(AKyaLevelPreview* InPreview, UKyaLevelBank* InBank,
    int32 InActorIndex, int32 InCheckpointIndex, const FVector& GameCenter)
{
    Preview = InPreview;
    Bank = InBank;
    Kind = EKyaVisualizerKind::TriggerBound;
    RecordIndex = InActorIndex;
    SubIndex = InCheckpointIndex;
    LinkStart = GameCenter;
    LinkEnd = GameCenter;
    bRecordEditable = false;
    Semantics = TEXT("Approximate trigger bound derived from a checkpoint waypoint. The schema stores zone/waypoint "
        "references but no trigger extents, so the radius is illustrative and read-only.");
    SyncFromBank();
}

void AKyaSceneVisualizer::MirrorFromBank()
{
    if (!Bank) return;
    if (Kind == EKyaVisualizerKind::Light && Bank->Lights.IsValidIndex(RecordIndex))
    {
        const FKyaLight& Light = Bank->Lights[RecordIndex];
        LightType = Light.LightType;
        ShapeType = Light.ShapeType;
        LightColor = Light.Color;
        AmbientColor = Light.AmbientColor;
        Multiplier = Light.Multiplier;
        LightPosition = Light.Position;
        LightDirection = Light.Direction;
        Falloff0 = Light.Falloff0;
        Falloff1 = Light.Falloff1;
        Fov0 = Light.Fov0;
        Fov1 = Light.Fov1;
    }
    else if (Kind == EKyaVisualizerKind::MainCamera)
    {
        CameraPosition = Bank->SceneEnvironment.CameraPosition;
        CameraLookAt = Bank->SceneEnvironment.CameraLookAt;
        CameraUp = Bank->SceneEnvironment.CameraUp;
        CameraFov = Bank->SceneEnvironment.CameraFov;
    }
}

void AKyaSceneVisualizer::SyncFromBank()
{
    if (!Bank) return;
    bSyncing = true;
    MirrorFromBank();
    FTransform Place = GetActorTransform();
    switch (Kind)
    {
    case EKyaVisualizerKind::Light:
    {
        const FVector Direction = KyaActorTransform::SwapGameAxes(LightDirection).GetSafeNormal();
        Place = FTransform(Direction.IsNearlyZero() ? FQuat::Identity
            : FQuat::FindBetweenNormals(FVector::ForwardVector, Direction),
            KyaActorTransform::SwapGameAxes(LightPosition) * 100.0);
        break;
    }
    case EKyaVisualizerKind::MainCamera:
    {
        const FVector Position = KyaActorTransform::SwapGameAxes(CameraPosition) * 100.0;
        const FVector Target = KyaActorTransform::SwapGameAxes(CameraLookAt) * 100.0;
        const FVector Direction = (Target - Position).GetSafeNormal();
        Place = FTransform(Direction.IsNearlyZero() ? FQuat::Identity
            : FQuat::FindBetweenNormals(FVector::ForwardVector, Direction), Position);
        break;
    }
    case EKyaVisualizerKind::ActorLink:
    {
        const FVector Start = KyaActorTransform::SwapGameAxes(LinkStart) * 100.0;
        const FVector End = KyaActorTransform::SwapGameAxes(LinkEnd) * 100.0;
        const FVector Direction = End - Start;
        Place = FTransform(Direction.IsNearlyZero() ? FQuat::Identity
            : FQuat::FindBetweenNormals(FVector::UpVector, Direction.GetSafeNormal()), (Start + End) * 0.5);
        break;
    }
    case EKyaVisualizerKind::TriggerBound:
        Place = FTransform(KyaActorTransform::SwapGameAxes(LinkStart) * 100.0);
        break;
    default:
        break;
    }
    SetActorRelativeTransform(Place, false, nullptr, ETeleportType::TeleportPhysics);
    bSyncing = false;
    RebuildShape();
}

void AKyaSceneVisualizer::RebuildShape()
{
    if (!Shape) return;
    switch (Kind)
    {
    case EKyaVisualizerKind::Light:
    {
        Shape->SetStaticMesh(LoadRuntimeMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere")));
        Shape->SetRelativeScale3D(FVector(FMath::Max(0.25f, Falloff0 > 0.0f ? Falloff0 : 0.5f)));
        Shape->SetVectorParameterValueOnMaterials(TEXT("Color"), FVector(LightColor.R, LightColor.G, LightColor.B));
        const bool bDirectional = ShapeType == 2 || ShapeType == 4;
        if (DirectionArrow) DirectionArrow->SetVisibility(bDirectional, true);
        break;
    }
    case EKyaVisualizerKind::MainCamera:
        Shape->SetStaticMesh(LoadRuntimeMesh(TEXT("/Engine/BasicShapes/Cube.Cube")));
        Shape->SetRelativeScale3D(FVector(0.35f));
        if (DirectionArrow) DirectionArrow->SetVisibility(true, true);
        break;
    case EKyaVisualizerKind::ActorLink:
    {
        Shape->SetStaticMesh(LoadRuntimeMesh(TEXT("/Engine/BasicShapes/Cylinder.Cylinder")));
        const double Length = FVector::Dist(KyaActorTransform::SwapGameAxes(LinkStart) * 100.0,
            KyaActorTransform::SwapGameAxes(LinkEnd) * 100.0);
        Shape->SetRelativeScale3D(FVector(0.04f, 0.04f, FMath::Max(0.01f, Length / 100.0)));
        if (DirectionArrow) DirectionArrow->SetVisibility(false, true);
        break;
    }
    case EKyaVisualizerKind::TriggerBound:
        Shape->SetStaticMesh(LoadRuntimeMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere")));
        Shape->SetRelativeScale3D(FVector(FMath::Max(0.1f, TriggerRadius / 50.0f)));
        if (DirectionArrow) DirectionArrow->SetVisibility(false, true);
        break;
    default:
        break;
    }
    if (Shape) Shape->SetVectorParameterValueOnMaterials(TEXT("Color"),
        FVector(LightColor.R, LightColor.G, LightColor.B));
}

void AKyaSceneVisualizer::SetGeneratedLabel(const FString& Label)
{
#if WITH_EDITOR
    TGuardValue<bool> Guard(bApplyingGeneratedLabel, true);
    if (GetActorLabel() != Label) SetActorLabel(Label);
#endif
}

#if WITH_EDITOR
bool AKyaSceneVisualizer::IsSelectable() const
{
    if (bSelectionLocked) return false;
    return !(IsValid(Preview) && Preview->bLockGeneratedActorSelection);
}

bool AKyaSceneVisualizer::CanDeleteSelectedActor(FText& OutReason) const
{
    OutReason = NSLOCTEXT("KyaLevel", "CannotDeleteSceneVisualizer",
        "Generated Kya visualizers represent bank records and cannot be deleted. "
        "Turn off the visualizer option on the level preview instead.");
    return false;
}

void AKyaSceneVisualizer::ResetDiagnosticTransform()
{
    FString Message;
    if (Kind == EKyaVisualizerKind::ActorLink)
        Message = TEXT("Actor-link visualizers are derived diagnostics and cannot be transformed.");
    else if (Kind == EKyaVisualizerKind::TriggerBound)
        Message = TEXT("Trigger-bound visualizers are approximate checkpoint diagnostics and cannot be transformed.");
    else
        Message = TEXT("Transform is not supported for this visualizer kind.");
    EditStatus = Message;
    UE_LOG(LogTemp, Warning, TEXT("KYA_VISUALIZER_TRANSFORM_REJECTED kind=%d bank=%s index=%d: %s"),
        int32(Kind), *GetNameSafe(Bank), RecordIndex, *Message);
    SyncFromBank();
}

void AKyaSceneVisualizer::CommitFromActor()
{
    if (bSyncing || !Preview || !Bank || !Preview->OwnsGeneratedActor(this)) return;
    FKyaLevelEditService& Service = FKyaLevelEditService::Get();
    FString Error;
    const FTransform Local = GetActorTransform().GetRelativeTransform(Preview->GetActorTransform());
    bool bApplied = true;
    if (Kind == EKyaVisualizerKind::Light)
    {
        const FVector Position = KyaActorTransform::SwapGameAxes(Local.GetLocation() / 100.0);
        FKyaLightEditPatch Patch;
        if (Bank->Lights.IsValidIndex(RecordIndex) &&
            !Bank->Lights[RecordIndex].Position.Equals(Position, 1.0e-4))
            Patch.Position = Position;
        if (ShapeType == 2 || ShapeType == 4)
        {
            const FVector StoredDirection = KyaActorTransform::SwapGameAxes(
                Bank->Lights.IsValidIndex(RecordIndex) ? Bank->Lights[RecordIndex].Direction : FVector::ZeroVector)
                .GetSafeNormal();
            const FQuat StoredRotation = StoredDirection.IsNearlyZero() ? FQuat::Identity
                : FQuat::FindBetweenNormals(FVector::ForwardVector, StoredDirection);
            if (!StoredRotation.Equals(Local.GetRotation().GetNormalized(), 1.0e-5))
            {
                Patch.Direction = KyaActorTransform::SwapGameAxes(
                    Local.GetRotation().GetForwardVector()).GetSafeNormal();
            }
        }
        if (!Patch.IsEmpty())
            bApplied = Service.ApplyLight(Service.MakeLightKey(Bank, RecordIndex), Patch, EKyaEditNotify::Deferred,
                &Error);
    }
    else if (Kind == EKyaVisualizerKind::MainCamera)
    {
        const FVector Position = KyaActorTransform::SwapGameAxes(Local.GetLocation() / 100.0);
        FKyaSceneCameraEditPatch Patch;
        if (!Bank->SceneEnvironment.CameraPosition.Equals(Position, 1.0e-4)) Patch.Position = Position;
        const FVector BankPosition = KyaActorTransform::SwapGameAxes(Bank->SceneEnvironment.CameraPosition) * 100.0;
        const FVector BankTarget = KyaActorTransform::SwapGameAxes(Bank->SceneEnvironment.CameraLookAt) * 100.0;
        const FVector BankDirection = (BankTarget - BankPosition).GetSafeNormal();
        const FQuat BankRotation = BankDirection.IsNearlyZero() ? FQuat::Identity
            : FQuat::FindBetweenNormals(FVector::ForwardVector, BankDirection);
        if (!BankRotation.Equals(Local.GetRotation().GetNormalized(), 1.0e-5))
        {
            // Preserve the actual stored distance; do not invent a minimum that would
            // rewrite an untouched look-at target.
            const double Distance = FVector::Dist(BankPosition, BankTarget);
            if (Distance > KINDA_SMALL_NUMBER)
            {
                const FVector LookAt = KyaActorTransform::SwapGameAxes(
                    (Local.GetLocation() + Local.GetRotation().GetForwardVector() * Distance) / 100.0);
                if (!Bank->SceneEnvironment.CameraLookAt.Equals(LookAt, 1.0e-3)) Patch.LookAt = LookAt;
            }
        }
        if (!Patch.IsEmpty())
            bApplied = Service.ApplySceneCamera(Service.MakeSceneCameraKey(Bank), Patch, EKyaEditNotify::Deferred,
                &Error);
    }
    else
    {
        ResetDiagnosticTransform();
        return;
    }
    if (!bApplied)
    {
        EditStatus = Error;
        UE_LOG(LogTemp, Warning, TEXT("KYA_VISUALIZER_WRITE_FAILED kind=%d bank=%s index=%d reason=%s"),
            int32(Kind), *GetNameSafe(Bank), RecordIndex, *Error);
    }
    else
    {
        EditStatus.Reset();
    }
    SyncFromBank();
}

void AKyaSceneVisualizer::PostEditMove(bool bFinished)
{
    if (bFinished) CommitFromActor();
    Super::PostEditMove(bFinished);
}

void AKyaSceneVisualizer::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    const FName CandidateNames[2] = {
        PropertyChangedEvent.MemberProperty ? PropertyChangedEvent.MemberProperty->GetFName() : NAME_None,
        PropertyChangedEvent.Property ? PropertyChangedEvent.Property->GetFName() : NAME_None };
    const auto Matches = [&CandidateNames](FName Name)
    {
        return CandidateNames[0] == Name || CandidateNames[1] == Name;
    };
    if (!bSyncing && Preview && Bank && Preview->OwnsGeneratedActor(this) && bRecordEditable)
    {
        FKyaLevelEditService& Service = FKyaLevelEditService::Get();
        FString Error;
        bool bApplied = true;
        if (Kind == EKyaVisualizerKind::Light)
        {
            FKyaLightEditPatch Patch;
            if (Matches(GET_MEMBER_NAME_CHECKED(AKyaSceneVisualizer, LightType))) Patch.LightType = LightType;
            else if (Matches(GET_MEMBER_NAME_CHECKED(AKyaSceneVisualizer, ShapeType))) Patch.ShapeType = ShapeType;
            else if (Matches(GET_MEMBER_NAME_CHECKED(AKyaSceneVisualizer, LightColor))) Patch.Color = LightColor;
            else if (Matches(GET_MEMBER_NAME_CHECKED(AKyaSceneVisualizer, AmbientColor))) Patch.AmbientColor = AmbientColor;
            else if (Matches(GET_MEMBER_NAME_CHECKED(AKyaSceneVisualizer, Multiplier))) Patch.Multiplier = Multiplier;
            else if (Matches(GET_MEMBER_NAME_CHECKED(AKyaSceneVisualizer, LightPosition))) Patch.Position = LightPosition;
            else if (Matches(GET_MEMBER_NAME_CHECKED(AKyaSceneVisualizer, LightDirection))) Patch.Direction = LightDirection;
            else if (Matches(GET_MEMBER_NAME_CHECKED(AKyaSceneVisualizer, Falloff0))) Patch.Falloff0 = Falloff0;
            else if (Matches(GET_MEMBER_NAME_CHECKED(AKyaSceneVisualizer, Falloff1))) Patch.Falloff1 = Falloff1;
            else if (Matches(GET_MEMBER_NAME_CHECKED(AKyaSceneVisualizer, Fov0))) Patch.Fov0 = Fov0;
            else if (Matches(GET_MEMBER_NAME_CHECKED(AKyaSceneVisualizer, Fov1))) Patch.Fov1 = Fov1;
            if (!Patch.IsEmpty())
                bApplied = Service.ApplyLight(Service.MakeLightKey(Bank, RecordIndex), Patch,
                    EKyaEditNotify::Notify, &Error);
        }
        else if (Kind == EKyaVisualizerKind::MainCamera)
        {
            FKyaSceneCameraEditPatch Patch;
            if (Matches(GET_MEMBER_NAME_CHECKED(AKyaSceneVisualizer, CameraPosition))) Patch.Position = CameraPosition;
            else if (Matches(GET_MEMBER_NAME_CHECKED(AKyaSceneVisualizer, CameraLookAt))) Patch.LookAt = CameraLookAt;
            else if (Matches(GET_MEMBER_NAME_CHECKED(AKyaSceneVisualizer, CameraUp))) Patch.Up = CameraUp;
            else if (Matches(GET_MEMBER_NAME_CHECKED(AKyaSceneVisualizer, CameraFov))) Patch.Fov = CameraFov;
            if (!Patch.IsEmpty())
                bApplied = Service.ApplySceneCamera(Service.MakeSceneCameraKey(Bank), Patch,
                    EKyaEditNotify::Notify, &Error);
        }
        if (!bApplied) EditStatus = Error;
        else EditStatus.Reset();
    }
    Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif


