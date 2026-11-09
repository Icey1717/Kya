#pragma once

#include "CoreMinimal.h"
#include "Exporters/Exporter.h"
#include "KyaLevelBankExporter.generated.h"

UCLASS()
class UKyaLevelBankExporter : public UExporter
{
    GENERATED_BODY()
public:
    UKyaLevelBankExporter();
    virtual bool ExportBinary(UObject* Object, const TCHAR* Type, FArchive& Ar,
        FFeedbackContext* Warn, int32 FileIndex = 0, uint32 PortFlags = 0) override;
};
