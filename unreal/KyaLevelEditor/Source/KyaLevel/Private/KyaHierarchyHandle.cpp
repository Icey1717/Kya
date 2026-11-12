#include "KyaHierarchyHandle.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "KyaLevelBank.h"
#include "KyaLevelPreview.h"
#include "UObject/ConstructorHelpers.h"

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
        SetActorLabel(FString::Printf(TEXT("%s hierarchy %d"), *Bank->GetName(), HierarchyOffset));
#endif
}

#if WITH_EDITOR
bool AKyaHierarchyHandle::CanDeleteSelectedActor(FText& OutReason) const
{
    OutReason = NSLOCTEXT("KyaLevel", "CannotDeleteHierarchyHandle",
        "Generated Kya hierarchy handles represent preview world geometry and cannot be deleted. "
        "Use the Visible toggle or clear Spawn Hierarchy Handles on the level preview.");
    return false;
}

void AKyaHierarchyHandle::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    const FName PropertyName = PropertyChangedEvent.GetPropertyName();
    if (PropertyName == GET_MEMBER_NAME_CHECKED(AKyaHierarchyHandle, bVisible) && Preview && Bank &&
        Preview->OwnsGeneratedActor(this))
        Preview->SetHierarchyVisible(Bank, HierarchyOffset, bVisible);
    Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif
