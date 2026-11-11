#include "KyaActorHandle.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "KyaLevelBank.h"
#include "KyaLevelPreview.h"
#include "UObject/ConstructorHelpers.h"

#include "Engine/World.h"

AKyaActorHandle::AKyaActorHandle()
{
    PrimaryActorTick.bCanEverTick = true;
    bIsEditorOnlyActor = true;

    Marker = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Marker"));
    RootComponent = Marker;
    static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    if (SphereMesh.Succeeded()) Marker->SetStaticMesh(SphereMesh.Object);
    Marker->SetRelativeScale3D(FVector(0.3f));
    Marker->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Marker->SetGenerateOverlapEvents(false);
    Marker->CastShadow = false;
    Marker->SetHiddenInGame(true);
    Marker->SetMobility(EComponentMobility::Movable);
}

void AKyaActorHandle::Initialize(AKyaLevelPreview* InPreview, UKyaLevelBank* InBank, int32 InActorIndex)
{
    Preview = InPreview;
    Bank = InBank;
    ActorIndex = InActorIndex;
    SyncFromBank();
}

void AKyaActorHandle::Destroyed()
{
    // The preview owns the handle lifetime; clear the back pointer so a pending
    // rebuild cannot touch a destroyed actor.
    Preview = nullptr;
    Super::Destroyed();
}

void AKyaActorHandle::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);
    // Super::PostEditChangeProperty can rerun construction while a Details edit is
    // still being committed; do not overwrite the pending values with bank data.
    if (!bApplyPending) SyncFromBank();
}

void AKyaActorHandle::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (bApplyPending)
    {
        bApplyPending = false;
        ApplyToBank();
        return;
    }
#if WITH_EDITOR
    if (IsSelected()) return;
#endif
    SyncFromBank();
}

void AKyaActorHandle::SyncFromBank()
{
    if (!Bank || !Bank->SceneEnvironment.Actors.IsValidIndex(ActorIndex)) return;
    const FKyaActorPlacement& Placement = Bank->SceneEnvironment.Actors[ActorIndex];
    bSyncing = true;
    ClassId = Placement.ClassId;
    ActorName = Placement.Name;
    ActorMeshIndex = Placement.MeshIndex;
    ActorTextureIndex = Placement.TextureIndex;
    ActorPosition = Placement.Position;
    ActorRotation = Placement.Rotation;
    ActorScale = Placement.Scale;
    ActorSectorId = Placement.SectorId;
    ActorDefaultBehaviourId = Placement.DefaultBehaviourId;
    ActorVisibilityDistance = Placement.VisibilityDistance;
    ActorLightingValue = Placement.LightingValue;
    ActorLightingFlags = Placement.LightingFlags;
    bSyncing = false;

    // Bank coordinates are game space (X right, Y up, Z forward); Unreal is X/Y/Z up.
    const FVector Local(ActorPosition.X, ActorPosition.Z, ActorPosition.Y);
    const FVector World = (Preview ? Preview->GetActorTransform() : GetActorTransform()).TransformPosition(Local * 100.0);
    SetActorLocation(World, false, nullptr, ETeleportType::TeleportPhysics);
#if WITH_EDITOR
    if (!ActorName.IsEmpty() && GetActorLabel() != ActorName) SetActorLabel(ActorName);
#endif
}

#if WITH_EDITOR
void AKyaActorHandle::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    if (bSyncing) return;
    const FName PropertyName = PropertyChangedEvent.GetPropertyName();
    static const FName Editable[] = {
        GET_MEMBER_NAME_CHECKED(AKyaActorHandle, ActorName),
        GET_MEMBER_NAME_CHECKED(AKyaActorHandle, ActorMeshIndex),
        GET_MEMBER_NAME_CHECKED(AKyaActorHandle, ActorTextureIndex),
        GET_MEMBER_NAME_CHECKED(AKyaActorHandle, ActorPosition),
        GET_MEMBER_NAME_CHECKED(AKyaActorHandle, ActorRotation),
        GET_MEMBER_NAME_CHECKED(AKyaActorHandle, ActorScale),
        GET_MEMBER_NAME_CHECKED(AKyaActorHandle, ActorSectorId),
        GET_MEMBER_NAME_CHECKED(AKyaActorHandle, ActorDefaultBehaviourId),
        GET_MEMBER_NAME_CHECKED(AKyaActorHandle, ActorVisibilityDistance),
        GET_MEMBER_NAME_CHECKED(AKyaActorHandle, ActorLightingValue),
        GET_MEMBER_NAME_CHECKED(AKyaActorHandle, ActorLightingFlags),
    };
    bool bMatched = false;
    for (const FName Candidate : Editable)
    {
        if (Candidate == PropertyName) { bMatched = true; break; }
    }
    if (!bMatched || !Bank || !Bank->SceneEnvironment.Actors.IsValidIndex(ActorIndex)) return;
    // The property assignment may not be committed yet inside this callback, so
    // write back on the next tick instead of clobbering the edit with old values.
    bApplyPending = true;
    Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif

void AKyaActorHandle::ApplyToBank()
{
    if (bSyncing || !Bank || !Bank->SceneEnvironment.Actors.IsValidIndex(ActorIndex))
    {
        UE_LOG(LogTemp, Warning, TEXT("KYA_HANDLE_APPLY_SKIP syncing=%d bank=%s index=%d"),
            bSyncing ? 1 : 0, *GetNameSafe(Bank), ActorIndex);
        return;
    }

    UE_LOG(LogTemp, Display, TEXT("KYA_HANDLE_APPLY bank=%s index=%d old_visibility=%.3f new_visibility=%.3f"),
        *Bank->GetName(), ActorIndex, Bank->SceneEnvironment.Actors[ActorIndex].VisibilityDistance, ActorVisibilityDistance);
    Bank->Modify();
    FKyaActorPlacement& Placement = Bank->SceneEnvironment.Actors[ActorIndex];
    Placement.Name = ActorName;
    Placement.MeshIndex = ActorMeshIndex;
    Placement.TextureIndex = ActorTextureIndex;
    Placement.Position = ActorPosition;
    Placement.Rotation = ActorRotation;
    Placement.Scale = ActorScale;
    Placement.SectorId = ActorSectorId;
    Placement.DefaultBehaviourId = ActorDefaultBehaviourId;
    Placement.VisibilityDistance = ActorVisibilityDistance;
    Placement.LightingValue = ActorLightingValue;
    Placement.LightingFlags = ActorLightingFlags;
    Bank->MarkPackageDirty();
    if (Preview) Preview->RebuildFromBank();
    SyncFromBank();
}
