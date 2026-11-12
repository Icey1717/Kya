#include "KyaActorHandle.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "KyaActorEditService.h"
#include "KyaActorTransform.h"
#include "KyaLevelBank.h"
#include "KyaLevelPreview.h"
#include "ProceduralMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"
#include "Editor.h"
#include "ScopedTransaction.h"

#include "Engine/World.h"

AKyaActorHandle::AKyaActorHandle()
{
    PrimaryActorTick.bCanEverTick = false;
    bIsEditorOnlyActor = true;

    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    RootComponent = SceneRoot;
    SceneRoot->SetMobility(EComponentMobility::Movable);

    ActorMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("ActorMesh"));
    ActorMesh->SetupAttachment(SceneRoot);
    ActorMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    ActorMesh->SetGenerateOverlapEvents(false);
    ActorMesh->CastShadow = false;
    ActorMesh->SetHiddenInGame(true);
    ActorMesh->SetMobility(EComponentMobility::Movable);
    ActorMesh->bSelectable = true;

    // The icon is only a fallback for records without a renderable mesh; it is not
    // a selection proxy for meshed actors.
    Marker = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Marker"));
    Marker->SetupAttachment(SceneRoot);
    static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    if (SphereMesh.Succeeded()) Marker->SetStaticMesh(SphereMesh.Object);
    Marker->SetRelativeScale3D(FVector(0.3f));
    Marker->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Marker->SetGenerateOverlapEvents(false);
    Marker->CastShadow = false;
    Marker->SetHiddenInGame(true);
    Marker->SetMobility(EComponentMobility::Movable);
    Marker->SetVisibility(false);
}

void AKyaActorHandle::Initialize(AKyaLevelPreview* InPreview, UKyaLevelBank* InBank, int32 InActorIndex, bool bInShowIcon)
{
    Preview = InPreview;
    Bank = InBank;
    ActorIndex = InActorIndex;
    EditGeneration = Bank ? Bank->EditGeneration : 0;
    SetIconEnabled(bInShowIcon);
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
    SyncFromBank();
}

void AKyaActorHandle::SetIconEnabled(bool bEnabled)
{
    bShowIcon = bEnabled;
    if (Marker) Marker->SetVisibility(bEnabled && RenderSectionCount == 0);
}

void AKyaActorHandle::SetActorMeshSections(const TArray<FKyaActorMeshSection>& Sections)
{
    if (!ActorMesh) return;
    if (Sections.Num() == 0)
    {
        ClearActorMesh();
        return;
    }
    ActorMesh->ClearAllMeshSections();
    SectionMaterials.Reset();
    for (int32 Index = 0; Index < Sections.Num(); ++Index)
    {
        const FKyaActorMeshSection& Section = Sections[Index];
        ActorMesh->CreateMeshSection(Index, Section.Positions, Section.Triangles, TArray<FVector>(),
            Section.UVs, Section.Colors, TArray<FProcMeshTangent>(), false);
        if (Section.Material)
        {
            ActorMesh->SetMaterial(Index, Section.Material);
            SectionMaterials.Add(Section.Material);
        }
    }
    RenderSectionCount = Sections.Num();
    if (Marker) Marker->SetVisibility(bShowIcon && RenderSectionCount == 0);
}

void AKyaActorHandle::ClearActorMesh()
{
    if (ActorMesh) ActorMesh->ClearAllMeshSections();
    SectionMaterials.Reset();
    RenderSectionCount = 0;
    if (Marker) Marker->SetVisibility(bShowIcon);
}

void AKyaActorHandle::MirrorFromPlacement()
{
    if (!Bank || !Bank->SceneEnvironment.Actors.IsValidIndex(ActorIndex)) return;
    const FKyaActorPlacement& Placement = Bank->SceneEnvironment.Actors[ActorIndex];
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
    EditGeneration = Bank->EditGeneration;
}

void AKyaActorHandle::SyncFromBank()
{
    if (!Bank || !Bank->SceneEnvironment.Actors.IsValidIndex(ActorIndex)) return;
    bSyncing = true;
    MirrorFromPlacement();

    FTransform Local;
    KyaActorTransform::MakeUnrealTransform(ActorPosition, ActorRotation, ActorScale, ClassId, Local);
    SetActorRelativeTransform(Local, false, nullptr, ETeleportType::TeleportPhysics);
    bSyncing = false;

#if WITH_EDITOR
    if (!ActorName.IsEmpty() && GetActorLabel() != ActorName) SetActorLabel(ActorName);
    else if (ActorName.IsEmpty() && ClassId != 0)
        SetActorLabel(FString::Printf(TEXT("Actor %d (class %lld)"), ActorIndex, ClassId));
#endif
}

#if WITH_EDITOR
void AKyaActorHandle::PostEditMove(bool bFinished)
{
    // Read the visible transform before Super: the base implementation reruns
    // construction scripts when the move finishes, and OnConstruction resynchronizes
    // the transform from the authoritative bank record.
    if (bFinished) CommitTransformFromActor();
    Super::PostEditMove(bFinished);
}

void AKyaActorHandle::CommitTransformFromActor()
{
    if (bSyncing || !Preview || !Bank || !Bank->SceneEnvironment.Actors.IsValidIndex(ActorIndex)) return;
    const FKyaActorPlacement& Placement = Bank->SceneEnvironment.Actors[ActorIndex];
    const FTransform Local = GetActorTransform().GetRelativeTransform(Preview->GetActorTransform());

    KyaActorTransform::FGameTransform Game;
    FString Error;
    if (!KyaActorTransform::UnrealToGameTransform(Local, Placement.ClassId, Placement.Rotation, Game, Error))
    {
        UE_LOG(LogTemp, Warning, TEXT("KYA_HANDLE_TRANSFORM_REJECTED bank=%s index=%d reason=%s"),
            *GetNameSafe(Bank), ActorIndex, *Error);
        SyncFromBank();
        return;
    }

    if (Game.Position.Equals(Placement.Position, 1.0e-4) && Game.Rotation.Equals(Placement.Rotation, 1.0e-5) &&
        Game.Scale.Equals(Placement.Scale, 1.0e-4))
    {
        // No actual edit; never mark the bank dirty for a click or a canceled drag.
        SyncFromBank();
        return;
    }

    FKyaActorEditPatch Patch;
    if (!Game.Position.Equals(Placement.Position, 1.0e-4)) Patch.Position = Game.Position;
    if (!Game.Rotation.Equals(Placement.Rotation, 1.0e-5)) Patch.Rotation = Game.Rotation;
    if (!Game.Scale.Equals(Placement.Scale, 1.0e-4)) Patch.Scale = Game.Scale;
    FKyaLevelEditService& Service = FKyaLevelEditService::Get();
    const FKyaActorRecordKey Key = Service.MakeKey(Bank, ActorIndex);
    bool bApplied = false;
    if (GEditor && !GEditor->IsTransactionActive())
    {
        // Some editor move paths close their transaction before PostEditMove. Open
        // our own so the single bank write at the end of the interaction is still
        // one undo step.
        FScopedTransaction Transaction(NSLOCTEXT("KyaLevelEditor", "KyaActorTransform", "Move Kya Actor"));
        bApplied = Service.Apply(Key, Patch, EKyaEditNotify::Deferred, &Error);
    }
    else
    {
        bApplied = Service.Apply(Key, Patch, EKyaEditNotify::Deferred, &Error);
    }
    if (!bApplied)
    {
        UE_LOG(LogTemp, Warning, TEXT("KYA_HANDLE_TRANSFORM_WRITE_FAILED bank=%s index=%d reason=%s"),
            *GetNameSafe(Bank), ActorIndex, *Error);
        SyncFromBank();
        return;
    }
    // The refresh is coalesced to the end of the frame so a multi-selection drag
    // rebuilds each dependent preview once, not once per selected actor.
    SyncFromBank();
}

void AKyaActorHandle::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    const FName CandidateNames[2] = {
        PropertyChangedEvent.MemberProperty ? PropertyChangedEvent.MemberProperty->GetFName() : NAME_None,
        PropertyChangedEvent.Property ? PropertyChangedEvent.Property->GetFName() : NAME_None };
    // Nested edits (for example Position.X) report the leaf as the changed property
    // and the containing member as MemberProperty, so both names are considered.
    const auto Matches = [&CandidateNames](FName Name)
    {
        return CandidateNames[0] == Name || CandidateNames[1] == Name;
    };
    if (!bSyncing && Bank && Bank->SceneEnvironment.Actors.IsValidIndex(ActorIndex))
    {
        FKyaActorEditPatch Patch;
        if (Matches(GET_MEMBER_NAME_CHECKED(AKyaActorHandle, ActorName))) Patch.Name = ActorName;
        else if (Matches(GET_MEMBER_NAME_CHECKED(AKyaActorHandle, ActorMeshIndex))) Patch.MeshIndex = ActorMeshIndex;
        else if (Matches(GET_MEMBER_NAME_CHECKED(AKyaActorHandle, ActorTextureIndex))) Patch.TextureIndex = ActorTextureIndex;
        else if (Matches(GET_MEMBER_NAME_CHECKED(AKyaActorHandle, ActorPosition))) Patch.Position = ActorPosition;
        else if (Matches(GET_MEMBER_NAME_CHECKED(AKyaActorHandle, ActorRotation))) Patch.Rotation = ActorRotation;
        else if (Matches(GET_MEMBER_NAME_CHECKED(AKyaActorHandle, ActorScale))) Patch.Scale = ActorScale;
        else if (Matches(GET_MEMBER_NAME_CHECKED(AKyaActorHandle, ActorSectorId))) Patch.SectorId = ActorSectorId;
        else if (Matches(GET_MEMBER_NAME_CHECKED(AKyaActorHandle, ActorDefaultBehaviourId))) Patch.DefaultBehaviourId = ActorDefaultBehaviourId;
        else if (Matches(GET_MEMBER_NAME_CHECKED(AKyaActorHandle, ActorVisibilityDistance))) Patch.VisibilityDistance = ActorVisibilityDistance;
        else if (Matches(GET_MEMBER_NAME_CHECKED(AKyaActorHandle, ActorLightingValue))) Patch.LightingValue = ActorLightingValue;
        else if (Matches(GET_MEMBER_NAME_CHECKED(AKyaActorHandle, ActorLightingFlags))) Patch.LightingFlags = ActorLightingFlags;

        // Only the edited field is written, so unrelated newer bank values (for example
        // a nested class field edited in the asset editor) are preserved. Apply before
        // Super: the base implementation may rerun construction and resynchronize the
        // visible fields from the bank.
        if (!Patch.IsEmpty())
        {
            FKyaLevelEditService& Service = FKyaLevelEditService::Get();
            const FKyaActorRecordKey Key = Service.MakeKey(Bank, ActorIndex);
            FString Error;
            if (!Service.Apply(Key, Patch, EKyaEditNotify::Notify, &Error))
                UE_LOG(LogTemp, Warning, TEXT("KYA_HANDLE_PROPERTY_WRITE_FAILED bank=%s index=%d field=%s reason=%s"),
                    *GetNameSafe(Bank), ActorIndex, *CandidateNames[0].ToString(), *Error);
        }
    }
    Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif

void AKyaActorHandle::ApplyToBank()
{
#if WITH_EDITOR
    if (bSyncing || !Bank || !Bank->SceneEnvironment.Actors.IsValidIndex(ActorIndex))
    {
        UE_LOG(LogTemp, Warning, TEXT("KYA_HANDLE_APPLY_SKIP syncing=%d bank=%s index=%d"),
            bSyncing ? 1 : 0, *GetNameSafe(Bank), ActorIndex);
        return;
    }
    FKyaActorEditPatch Patch;
    Patch.Name = ActorName;
    Patch.MeshIndex = ActorMeshIndex;
    Patch.TextureIndex = ActorTextureIndex;
    Patch.Position = ActorPosition;
    Patch.Rotation = ActorRotation;
    Patch.Scale = ActorScale;
    Patch.SectorId = ActorSectorId;
    Patch.DefaultBehaviourId = ActorDefaultBehaviourId;
    Patch.VisibilityDistance = ActorVisibilityDistance;
    Patch.LightingValue = ActorLightingValue;
    Patch.LightingFlags = ActorLightingFlags;
    FKyaLevelEditService& Service = FKyaLevelEditService::Get();
    const FKyaActorRecordKey Key = Service.MakeKey(Bank, ActorIndex);
    FString Error;
    if (!Service.Apply(Key, Patch, EKyaEditNotify::Notify, &Error))
        UE_LOG(LogTemp, Warning, TEXT("KYA_HANDLE_APPLY_FAILED bank=%s index=%d reason=%s"),
            *GetNameSafe(Bank), ActorIndex, *Error);
    SyncFromBank();
#endif
}
