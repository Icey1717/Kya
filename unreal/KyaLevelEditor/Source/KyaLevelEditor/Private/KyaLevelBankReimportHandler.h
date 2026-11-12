#pragma once

#include "CoreMinimal.h"
#include "EditorReimportHandler.h"

class UKyaLevelBank;

/**
 * Reimports a Kya bank asset from its recorded source file.
 *
 * The new source is parsed into a temporary bank first; the existing asset is only
 * replaced when parsing succeeds, so a failed or unsupported reimport leaves the
 * previous data intact. Dirty assets require an explicit discard confirmation.
 */
class FKyaLevelBankReimportHandler : public FReimportHandler
{
public:
    virtual bool CanReimport(UObject* Obj, TArray<FString>& OutFilenames) override;
    virtual void SetReimportPaths(UObject* Obj, const TArray<FString>& NewReimportPaths) override;
    virtual EReimportResult::Type Reimport(UObject* Obj) override;
    virtual int32 GetPriority() const override { return 0; }
};
