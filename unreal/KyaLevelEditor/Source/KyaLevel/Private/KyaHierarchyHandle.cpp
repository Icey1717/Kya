#include "KyaHierarchyHandle.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "KyaLevelBank.h"
#include "KyaLevelPreview.h"
#include "ProceduralMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#if WITH_EDITOR
#include "Misc/App.h"
#include "Misc/MessageDialog.h"
#endif

AKyaHierarchyHandle::AKyaHierarchyHandle()
{
    PrimaryActorTick.bCanEverTick = false;
    bIsEditorOnlyActor = true;

    // The hierarchy geometry is attached directly to the preview root; this actor owns
    // it so clicking any rendered strip selects the hierarchy.
    HierarchyMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("HierarchyMesh"));
    RootComponent = HierarchyMesh;
    HierarchyMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    HierarchyMesh->SetGenerateOverlapEvents(false);
    HierarchyMesh->CastShadow = false;
    HierarchyMesh->SetHiddenInGame(true);
    HierarchyMesh->SetMobility(EComponentMobility::Movable);
    HierarchyMesh->bSelectable = true;

    // Fallback marker for hierarchies whose geometry could not be captured.
    Marker = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Marker"));
    Marker->SetupAttachment(HierarchyMesh);
    static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    if (SphereMesh.Succeeded()) Marker->SetStaticMesh(SphereMesh.Object);
    Marker->SetRelativeScale3D(FVector(0.6f));
    Marker->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Marker->SetGenerateOverlapEvents(false);
    Marker->CastShadow = false;
    Marker->SetHiddenInGame(true);
    Marker->SetMobility(EComponentMobility::Movable);

#if WITH_EDITOR
    // Presentation-only: the handle must never look like it supports a serialized
    // hierarchy transform. The preview attachment keeps it aligned.
    SetLockLocation(true);
#endif
}

void AKyaHierarchyHandle::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);
}

void AKyaHierarchyHandle::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
}

void AKyaHierarchyHandle::Initialize(AKyaLevelPreview* InPreview, UKyaLevelBank* InBank, int32 InSectorId,
    int32 InHierarchyOffset, const FVector& InLocalOrigin, const FVector& InSourceOrigin,
    int32 InStripCount, int32 InTriangleCount, bool bInVisible)
{
    Preview = InPreview;
    Bank = InBank;
    SectorId = InSectorId;
    HierarchyOffset = InHierarchyOffset;
    SourceOrigin = InSourceOrigin;
    StripCount = InStripCount;
    TriangleCount = InTriangleCount;
    bVisible = bInVisible;
    if (InPreview)
        AttachToActor(InPreview, FAttachmentTransformRules::KeepRelativeTransform);
    // Geometry vertices are preview-local, so the handle transform stays identity.
    SetActorRelativeTransform(FTransform::Identity, false, nullptr, ETeleportType::TeleportPhysics);
    Marker->SetRelativeLocation(InLocalOrigin);
#if WITH_EDITOR
    if (Bank)
        SetGeneratedLabel(FString::Printf(TEXT("%s hierarchy %d"), *Bank->GetName(), HierarchyOffset));
#endif
}

void AKyaHierarchyHandle::SetHierarchyGeometry(const TArray<FKyaActorMeshSection>& Sections,
    const FBox& InWorldBounds, const TArray<FString>& InMaterials, const TArray<uint64>& InMaterialHashes)
{
    WorldBounds = InWorldBounds;
    MaterialNames = InMaterials;
    MaterialHashes.Reset(InMaterialHashes.Num());
    for (const uint64 Hash : InMaterialHashes) MaterialHashes.Add(int64(Hash));

    if (Sections.IsEmpty())
    {
        ClearHierarchyGeometry();
        return;
    }
    HierarchyMesh->ClearAllMeshSections();
    SectionMaterials.Reset();
    for (int32 Index = 0; Index < Sections.Num(); ++Index)
    {
        const FKyaActorMeshSection& Section = Sections[Index];
        if (Section.Positions.IsEmpty() || Section.Triangles.IsEmpty()) continue;
        HierarchyMesh->CreateMeshSection(Index, Section.Positions, Section.Triangles, TArray<FVector>(),
            Section.UVs, Section.Colors, TArray<FProcMeshTangent>(), false);
        if (Section.Material)
        {
            HierarchyMesh->SetMaterial(Index, Section.Material);
            SectionMaterials.Add(Section.Material);
        }
    }
    if (Marker) Marker->SetVisibility(HierarchyMesh->GetNumSections() == 0, true);
}

void AKyaHierarchyHandle::ClearHierarchyGeometry()
{
    if (HierarchyMesh) HierarchyMesh->ClearAllMeshSections();
    SectionMaterials.Reset();
    MaterialNames.Reset();
    MaterialHashes.Reset();
    if (Marker) Marker->SetVisibility(true, true);
}

#if WITH_EDITOR
void AKyaHierarchyHandle::SetGeneratedLabel(const FString& Label)
{
    TGuardValue<bool> Guard(bApplyingGeneratedLabel, true);
    if (GetActorLabel() != Label) SetActorLabel(Label);
}

void AKyaHierarchyHandle::ResetToPreviewRoot()
{
    const FTransform Current = IsValid(Preview)
        ? GetActorTransform().GetRelativeTransform(Preview->GetActorTransform())
        : GetActorTransform();
    if (!Current.Equals(FTransform::Identity, 1.0e-4))
    {
        TransformStatus = TEXT("Transform reset: hierarchy transforms are not serialized, so nothing is exported. "
            "Move the owning bank/background data instead.");
        UE_LOG(LogTemp, Warning, TEXT("KYA_HIERARCHY_TRANSFORM_REJECTED handle=%s bank=%s offset=%d: "
            "serialized hierarchy writeback is unsupported"), *GetNameSafe(this), *GetNameSafe(Bank), HierarchyOffset);
    }
    SetActorRelativeTransform(FTransform::Identity, false, nullptr, ETeleportType::TeleportPhysics);
}

bool AKyaHierarchyHandle::IsSelectable() const
{
    if (bSelectionLocked) return false;
    return !(IsValid(Preview) && Preview->bLockGeneratedActorSelection);
}

bool AKyaHierarchyHandle::CanDeleteSelectedActor(FText& OutReason) const
{
    OutReason = NSLOCTEXT("KyaLevel", "CannotDeleteHierarchyHandle",
        "Generated Kya hierarchy handles represent preview world geometry and cannot be deleted. "
        "Use the Visible toggle or clear Spawn Hierarchy Handles on the level preview.");
    return false;
}

void AKyaHierarchyHandle::PostEditMove(bool bFinished)
{
    ResetToPreviewRoot();
    Super::PostEditMove(bFinished);
    ResetToPreviewRoot();
}

void AKyaHierarchyHandle::EditorApplyTranslation(const FVector& DeltaTranslation, bool bAltDown, bool bShiftDown,
    bool bCtrlDown)
{
    ResetToPreviewRoot();
}

void AKyaHierarchyHandle::EditorApplyRotation(const FRotator& DeltaRotation, bool bAltDown, bool bShiftDown,
    bool bCtrlDown)
{
    ResetToPreviewRoot();
}

void AKyaHierarchyHandle::EditorApplyScale(const FVector& DeltaScale, const FVector* PivotLocation, bool bAltDown,
    bool bShiftDown, bool bCtrlDown)
{
    ResetToPreviewRoot();
}

void AKyaHierarchyHandle::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    const FName PropertyName = PropertyChangedEvent.MemberProperty
        ? PropertyChangedEvent.MemberProperty->GetFName()
        : PropertyChangedEvent.Property ? PropertyChangedEvent.Property->GetFName() : NAME_None;
    if (!bApplyingGeneratedLabel && PropertyName == TEXT("ActorLabel"))
    {
        FString Label = Bank
            ? FString::Printf(TEXT("%s hierarchy %d"), *Bank->GetName(), HierarchyOffset)
            : FString::Printf(TEXT("Hierarchy %d"), HierarchyOffset);
        SetGeneratedLabel(Label);
        UE_LOG(LogTemp, Warning, TEXT("KYA_HIERARCHY_RENAME_REJECTED handle=%s: labels are generated"),
            *GetNameSafe(this));
        if (!FApp::IsUnattended())
            FMessageDialog::Open(EAppMsgType::Ok, NSLOCTEXT("KyaLevel", "HierarchyRenameRejected",
                "Hierarchy labels are generated from the bank and cannot be renamed. The label was restored."));
        Super::PostEditChangeProperty(PropertyChangedEvent);
        return;
    }
    if (PropertyName == GET_MEMBER_NAME_CHECKED(AKyaHierarchyHandle, bVisible) && Preview && Bank &&
        Preview->OwnsGeneratedActor(this))
        Preview->SetHierarchyVisible(Bank, HierarchyOffset, bVisible);
    Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif
