#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "KyaLevelSetFactory.generated.h"

/** Creates an empty UKyaLevelSet asset from the Content Browser. */
UCLASS()
class UKyaLevelSetFactory : public UFactory
{
    GENERATED_BODY()
public:
    UKyaLevelSetFactory();
    virtual UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags,
        UObject* Context, FFeedbackContext* Warn) override;
};
