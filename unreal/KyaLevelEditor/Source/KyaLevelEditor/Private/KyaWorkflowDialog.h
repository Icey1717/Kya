#pragma once
#include "CoreMinimal.h"
class AKyaLevelPreview;
void KyaShowDiscoveryDialog(const TArray<AKyaLevelPreview*>& Previews, bool bImportMissing);
void KyaShowLevelSetDialog(const TArray<AKyaLevelPreview*>& Previews);
