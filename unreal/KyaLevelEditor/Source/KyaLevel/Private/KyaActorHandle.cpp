#include "KyaActorHandle.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "KyaActorEditService.h"
#include "KyaActorCatalog.h"
#include "KyaActorTransform.h"
#include "KyaLevelBank.h"
#include "KyaLevelPreview.h"
#include "ProceduralMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"
#include "Editor.h"
#include "ScopedTransaction.h"
#include "Misc/App.h"
#include "Misc/MessageDialog.h"

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

void AKyaActorHandle::SetMeshHighlighted(bool bHighlighted)
{
    bMeshHighlighted = bHighlighted;
    if (!ActorMesh) return;
    // Stencil-based editor highlight covers every section, so the whole mesh reads as
    // one selectable actor even when it is split into material batches.
    ActorMesh->SetRenderCustomDepth(bHighlighted);
    ActorMesh->SetCustomDepthStencilValue(bHighlighted ? 1 : 0);
}

FBox AKyaActorHandle::GetRenderedMeshBounds() const
{
    FBox Bounds(ForceInit);
    if (!ActorMesh) return Bounds;
    const FTransform MeshTransform = ActorMesh->GetComponentTransform();
    for (int32 Section = 0; Section < ActorMesh->GetNumSections(); ++Section)
    {
        const FProcMeshSection* Data = ActorMesh->GetProcMeshSection(Section);
        if (!Data) continue;
        for (const FProcMeshVertex& Vertex : Data->ProcVertexBuffer)
            Bounds += MeshTransform.TransformPosition(FVector(Vertex.Position));
    }
    return Bounds;
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
    if (!Preview->OwnsGeneratedActor(this))
    {
        // Duplicated/pasted representations must never write through the identity
        // they copied from their source.
        UE_LOG(LogTemp, Warning, TEXT("KYA_HANDLE_STRAY_WRITE_IGNORED bank=%s index=%d"), *GetNameSafe(Bank), ActorIndex);
        return;
    }
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
    if (GEditor && !GIsTransacting)
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

bool AKyaActorHandle::CanDeleteSelectedActor(FText& OutReason) const
{
    OutReason = NSLOCTEXT("KyaLevel", "CannotDeleteActorHandle",
        "Generated Kya actor handles represent bank records and cannot be deleted directly. "
        "Use Kya Level > Delete Kya Actor (or Kya.DeleteActor) to remove the record and remap references.");
    return false;
}

bool AKyaActorHandle::IsSelectable() const
{
    if (bSelectionLocked) return false;
    return !(IsValid(Preview) && Preview->bLockGeneratedActorSelection);
}

void AKyaActorHandle::SetGeneratedLabel(const FString& Label)
{
    TGuardValue<bool> Guard(bApplyingGeneratedLabel, true);
    if (GetActorLabel() != Label) SetActorLabel(Label);
}

bool AKyaActorHandle::RenameActorRecord(const FString& DesiredLabel, FString& OutError)
{
#if WITH_EDITOR
    UKyaLevelBank* EditedBank = Bank;
    if (!EditedBank || !EditedBank->SceneEnvironment.Actors.IsValidIndex(ActorIndex))
    {
        OutError = TEXT("The bank record no longer exists.");
        return false;
    }
    // Duplicated/pasted copies can be relabeled before the command-end sweep; they
    // must never rename the source record through their copied identity.
    if (!IsValid(Preview) || !Preview->OwnsGeneratedActor(this))
    {
        OutError = TEXT("Only the preview-owned representation of a bank record can rename it.");
        return false;
    }
    const FKyaActorPlacement& Record = EditedBank->SceneEnvironment.Actors[ActorIndex];
    FString Desired = DesiredLabel.TrimStartAndEnd();

    // The Outliner label is derived as "<name> [class <id>]" with an optional
    // " [<index>]" suffix for duplicate names. Strip the index only as part of that
    // complete generated ending, then the class suffix, so authored text such as
    // "B [1]" is never mistaken for a generated suffix.
    const FString IndexSuffix = FString::Printf(TEXT(" [%d]"), ActorIndex);
    const FString ClassSuffix = FString::Printf(TEXT(" [class %lld]"), Record.ClassId);
    const bool bHasFullEnding = Desired.EndsWith(IndexSuffix, ESearchCase::CaseSensitive) &&
        Desired.Len() > IndexSuffix.Len() &&
        Desired.LeftChop(IndexSuffix.Len()).EndsWith(ClassSuffix, ESearchCase::CaseSensitive);
    if (bHasFullEnding)
    {
        Desired.LeftChopInline(IndexSuffix.Len());
        Desired.LeftChopInline(ClassSuffix.Len());
    }
    else if (Desired.EndsWith(ClassSuffix, ESearchCase::CaseSensitive))
    {
        Desired.LeftChopInline(ClassSuffix.Len());
    }
    Desired.TrimEndInline();

    if (!ValidateKyaActorNameChange(Record.Name, Desired, OutError)) return false;
    if (Desired == Record.Name)
    {
        if (IsValid(Preview)) SetGeneratedLabel(Preview->GetActorDisplayLabel(ActorIndex));
        return true;
    }

    FKyaLevelEditService& Service = FKyaLevelEditService::Get();
    FKyaActorEditPatch Patch;
    Patch.Name = Desired;
    FString Error;
    // Deferred: a rename can arrive from inside PostEditChangeProperty while the
    // details panel is notifying; the preview refresh happens at the end of the frame.
    if (!Service.Apply(Service.MakeKey(EditedBank, ActorIndex), Patch, EKyaEditNotify::Deferred, &Error))
    {
        OutError = Error;
        return false;
    }
    if (IsValid(Preview)) SetGeneratedLabel(Preview->GetActorDisplayLabel(ActorIndex));
    else SetGeneratedLabel(FString::Printf(TEXT("%s [class %lld]"), *Desired, Record.ClassId));
    return true;
#else
    OutError = TEXT("Renaming bank records requires the editor.");
    return false;
#endif
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

    // Outliner rename policy: the label is presentation, the serialized name is the
    // edit. A same-bucket rename is written through the edit service; anything else is
    // rejected, explained and reverted to the derived label (never truncated).
    if (!bApplyingGeneratedLabel && Matches(TEXT("ActorLabel")))
    {
        FString Error;
        const FString DesiredLabel = GetActorLabel(false);
        if (!RenameActorRecord(DesiredLabel, Error))
        {
            const FString Reverted = IsValid(Preview) && Preview->Bank == Bank
                ? Preview->GetActorDisplayLabel(ActorIndex)
                : FString::Printf(TEXT("%s [class %lld]"), *ActorName, ClassId);
            SetGeneratedLabel(Reverted);
            UE_LOG(LogTemp, Warning, TEXT("KYA_ACTOR_RENAME_REJECTED bank=%s index=%d name=%s reason=%s"),
                *GetNameSafe(Bank), ActorIndex, *DesiredLabel, *Error);
            if (!FApp::IsUnattended())
            {
                FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(FString::Printf(
                    TEXT("The name '%s' cannot be stored in this bank record.\n\n%s\n\n"
                        "The Outliner label was restored; the bank was not modified."),
                    *DesiredLabel, *Error)));
            }
        }
        Super::PostEditChangeProperty(PropertyChangedEvent);
        return;
    }
    if (!bSyncing && Preview && Bank && Preview->OwnsGeneratedActor(this) &&
        Bank->SceneEnvironment.Actors.IsValidIndex(ActorIndex))
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
    if (bSyncing || !Bank || !Bank->SceneEnvironment.Actors.IsValidIndex(ActorIndex) ||
        !Preview || !Preview->OwnsGeneratedActor(this))
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

