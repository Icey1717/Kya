#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "KyaLevelBankFactory.generated.h"

UCLASS()
class UKyaLevelBankFactory : public UFactory
{
    GENERATED_BODY()
public:
    UKyaLevelBankFactory();
    virtual UObject* FactoryCreateBinary(UClass* InClass, UObject* InParent, FName InName,
        EObjectFlags Flags, UObject* Context, const TCHAR* Type, const uint8*& Buffer,
        const uint8* BufferEnd, FFeedbackContext* Warn) override;
};
