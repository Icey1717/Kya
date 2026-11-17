#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "KyaDiscFactory.generated.h"

UCLASS()
class UKyaDiscFactory : public UFactory
{
    GENERATED_BODY()
public:
    UKyaDiscFactory();
    virtual UObject* FactoryCreateFile(UClass* InClass, UObject* InParent, FName InName,
        EObjectFlags Flags, const FString& Filename, const TCHAR* Parms,
        FFeedbackContext* Warn, bool& bOutOperationCanceled) override;
};
