#pragma once
#include "CoreMinimal.h"
class UKyaDisc;
class UKyaLevelBank;
FString KyaDiscBankStateHash(const UKyaLevelBank& Bank);
bool KyaGenerateDiscLevels(UKyaDisc& Disc, FString& Error);
bool KyaExportDisc(UKyaDisc& Disc, FString& Report,
    const TFunction<void(const FString&)>& Output = {});
