#include "KyaHierarchyHandle.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "KyaLevelBank.h"
#include "KyaLevelPreview.h"
#include "UObject/ConstructorHelpers.h"
#if WITH_EDITOR
#include "Misc/App.h"
#include "Misc/MessageDialog.h"
#endif

AKyaHierarchyHandle::AKyaHierarchyHandle()
{
    PrimaryActorTick.bCanEverTick = false;
    bIsEditorOnlyActor = true;

    Marker = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Marker"));
    RootComponent = Marker;
    static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    if (SphereMesh.Succeeded()) Marker->SetStaticMesh(SphereMesh.Object);
    Marker->SetRelativeScale3D(FVector(0.6f));
    Marker->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Marker->SetGenerateOverlapEvents(false);
    Marker->CastShadow = false;
    Marker->SetHiddenInGame(true);
    Marker->SetMobility(EComponentMobility::Movable);
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
    int32 InHierarchyOffset, const FVector& InWorldOrigin, int32 InStripCount, int32 InTriangleCount, bool bInVisible)
{
    Preview = InPreview;
    Bank = InBank;
    SectorId = InSectorId;
    HierarchyOffset = InHierarchyOffset;
    StripCount = InStripCount;
    TriangleCount = InTriangleCount;
    bVisible = bInVisible;
    SetActorLocation(InWorldOrigin, false, nullptr, ETeleportType::TeleportPhysics);
#if WITH_EDITOR
    if (Bank)
        SetGeneratedLabel(FString::Printf(TEXT("%s hierarchy %d"), *Bank->GetName(), HierarchyOffset));
#endif
}

#if WITH_EDITOR
void AKyaHierarchyHandle::SetGeneratedLabel(const FString& Label)
{
    TGuardValue<bool> Guard(bApplyingGeneratedLabel, true);
    if (GetActorLabel() != Label) SetActorLabel(Label);
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
