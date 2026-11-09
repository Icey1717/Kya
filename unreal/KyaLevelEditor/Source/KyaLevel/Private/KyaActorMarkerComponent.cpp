#include "KyaActorMarkerComponent.h"
#include "KyaLevelPreview.h"
#include "KyaLevelBank.h"

#if WITH_EDITOR
void UKyaActorMarkerComponent::PostEditComponentMove(bool bFinished)
{
    AKyaLevelPreview* Preview = Cast<AKyaLevelPreview>(GetOwner());
    if (Preview && Preview->Bank && Preview->Bank->SceneEnvironment.Actors.IsValidIndex(ActorArrayIndex))
    {
        Preview->Bank->Modify();
        const FVector P = GetRelativeLocation() / 100.0;
        Preview->Bank->SceneEnvironment.Actors[ActorArrayIndex].Position = FVector(P.X, P.Z, P.Y);
        Preview->Bank->MarkPackageDirty();
    }
    Super::PostEditComponentMove(bFinished);
}
#endif
