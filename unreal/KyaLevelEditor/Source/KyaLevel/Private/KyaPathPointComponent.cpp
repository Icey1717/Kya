#include "KyaPathPointComponent.h"
#include "KyaLevelPreview.h"
#include "KyaLevelBank.h"

void UKyaPathPointComponent::WriteBackToBank()
{
#if WITH_EDITOR
    AKyaLevelPreview* Preview = Cast<AKyaLevelPreview>(GetOwner());
    if (!Preview || !Preview->Bank || !Preview->Bank->SceneEnvironment.Paths.IsValidIndex(PathIndex)) return;
    FKyaPath& Path = Preview->Bank->SceneEnvironment.Paths[PathIndex];
    if (!Path.Points.IsValidIndex(PointIndex)) return;
    Preview->Bank->Modify();
    const FVector Position = GetRelativeLocation() / 100.0;
    FVector4& Point = Path.Points[PointIndex];
    UE_LOG(LogTemp, Display, TEXT("KYA_PATH_WRITE path=%d point=%d relative=(%.2f,%.2f,%.2f) old=(%.2f,%.2f,%.2f)"),
        PathIndex, PointIndex, Position.X, Position.Y, Position.Z, Point.X, Point.Y, Point.Z);
    Point.X = Position.X; Point.Y = Position.Z; Point.Z = Position.Y;
    Preview->Bank->MarkPackageDirty();
    Preview->UpdatePathGuidePoint(PathIndex, PointIndex, GetRelativeLocation());
#endif
}

void UKyaPathPointComponent::SetMarkerLocation(const FVector& NewRelativeLocation)
{
    SetRelativeLocation(NewRelativeLocation);
    WriteBackToBank();
}

#if WITH_EDITOR
void UKyaPathPointComponent::PostEditComponentMove(bool bFinished)
{
    WriteBackToBank();
    Super::PostEditComponentMove(bFinished);
}
#endif
