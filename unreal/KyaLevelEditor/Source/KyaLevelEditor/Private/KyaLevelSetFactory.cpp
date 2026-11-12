#include "KyaLevelSetFactory.h"

#include "KyaLevelSet.h"

UKyaLevelSetFactory::UKyaLevelSetFactory()
{
    SupportedClass = UKyaLevelSet::StaticClass();
    bCreateNew = true;
    bEditorImport = false;
    bEditAfterNew = false;
}

UObject* UKyaLevelSetFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags,
    UObject* Context, FFeedbackContext* Warn)
{
    return NewObject<UKyaLevelSet>(InParent, InClass ? InClass : UKyaLevelSet::StaticClass(), InName, Flags);
}
