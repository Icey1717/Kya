#include "KyaPathPointComponent.h"
#include "KyaLevelPreview.h"
#include "KyaLevelBank.h"

#if WITH_EDITOR
void UKyaPathPointComponent::PostEditComponentMove(bool bFinished)
{
    AKyaLevelPreview* Preview = Cast<AKyaLevelPreview>(GetOwner());
    if (Preview && Preview->Bank && Preview->Bank->SceneEnvironment.Paths.IsValidIndex(PathIndex))
    {
        FKyaPath& Path = Preview->Bank->SceneEnvironment.Paths[PathIndex];
        if (Path.Points.IsValidIndex(PointIndex))
        {
            Preview->Bank->Modify();
            const FVector Position = GetRelativeLocation() / 100.0;
            FVector4& Point = Path.Points[PointIndex];
            Point.X = Position.X; Point.Y = Position.Z; Point.Z = Position.Y;
            Preview->Bank->MarkPackageDirty();
            Preview->UpdatePathGuidePoint(PathIndex, PointIndex, GetRelativeLocation());
        }
    }
    Super::PostEditComponentMove(bFinished);
}
#endif
