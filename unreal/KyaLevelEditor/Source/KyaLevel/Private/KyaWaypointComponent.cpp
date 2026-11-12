#include "KyaWaypointComponent.h"
#include "KyaActorEditService.h"
#include "KyaLevelBank.h"
#include "KyaLevelPreview.h"

#if WITH_EDITOR
void UKyaWaypointComponent::PostEditComponentMove(bool bFinished)
{
    AKyaLevelPreview* Preview = Cast<AKyaLevelPreview>(GetOwner());
    if (Preview && Preview->Bank && Preview->Bank->SceneEnvironment.Waypoints.IsValidIndex(WaypointIndex))
    {
        const FVector Position = GetRelativeLocation() / 100.0;
        FKyaWaypointEditPatch Patch;
        Patch.Position = FVector(Position.X, Position.Z, Position.Y);
        FKyaLevelEditService& Service = FKyaLevelEditService::Get();
        FString Error;
        if (!Service.ApplyWaypoint(Service.MakeWaypointKey(Preview->Bank, WaypointIndex), Patch,
            EKyaEditNotify::Deferred, &Error))
        {
            UE_LOG(LogTemp, Warning, TEXT("KYA_WAYPOINT_WRITE_FAILED index=%d reason=%s"), WaypointIndex, *Error);
        }
    }
    Super::PostEditComponentMove(bFinished);
}
#endif
