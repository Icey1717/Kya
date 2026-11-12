#include "KyaPathPointComponent.h"
#include "KyaActorEditService.h"
#include "KyaLevelPreview.h"
#include "KyaLevelBank.h"

void UKyaPathPointComponent::WriteBackToBank()
{
#if WITH_EDITOR
    AKyaLevelPreview* Preview = Cast<AKyaLevelPreview>(GetOwner());
    if (!Preview || !Preview->Bank || !Preview->Bank->SceneEnvironment.Paths.IsValidIndex(PathIndex)) return;
    FKyaPath& Path = Preview->Bank->SceneEnvironment.Paths[PathIndex];
    if (!Path.Points.IsValidIndex(PointIndex)) return;
    const FVector Position = GetRelativeLocation() / 100.0;
    const FVector4& Current = Path.Points[PointIndex];
    const FVector4 Target(Position.X, Position.Z, Position.Y, Current.W);
    if (!Target.Equals(Current, 1.0e-5))
    {
        FKyaLevelEditService& Service = FKyaLevelEditService::Get();
        FKyaPathPointEditPatch Patch;
        Patch.Point = Target;
        FString Error;
        if (!Service.ApplyPathPoint(Service.MakePathPointKey(Preview->Bank, PathIndex, PointIndex), Patch,
            EKyaEditNotify::Deferred, &Error))
        {
            UE_LOG(LogTemp, Warning, TEXT("KYA_PATH_WRITE_FAILED path=%d point=%d reason=%s"),
                PathIndex, PointIndex, *Error);
        }
    }
    UE_LOG(LogTemp, Display, TEXT("KYA_PATH_WRITE path=%d point=%d relative=(%.2f,%.2f,%.2f) old=(%.2f,%.2f,%.2f)"),
        PathIndex, PointIndex, Position.X, Position.Y, Position.Z, Current.X, Current.Y, Current.Z);
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
