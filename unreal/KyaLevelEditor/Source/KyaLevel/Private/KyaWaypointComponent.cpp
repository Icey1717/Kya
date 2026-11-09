#include "KyaWaypointComponent.h"
#include "KyaLevelBank.h"
#include "KyaLevelPreview.h"

#if WITH_EDITOR
void UKyaWaypointComponent::PostEditComponentMove(bool bFinished)
{
    AKyaLevelPreview* Preview = Cast<AKyaLevelPreview>(GetOwner());
    if (Preview && Preview->Bank && Preview->Bank->SceneEnvironment.Waypoints.IsValidIndex(WaypointIndex))
    {
        Preview->Bank->Modify();
        const FVector Position = GetRelativeLocation() / 100.0;
        Preview->Bank->SceneEnvironment.Waypoints[WaypointIndex].Position = FVector(Position.X, Position.Z, Position.Y);
        Preview->Bank->MarkPackageDirty();
    }
    Super::PostEditComponentMove(bFinished);
}
#endif
