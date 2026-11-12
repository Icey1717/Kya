#pragma once

#include "CoreMinimal.h"
#include "IDetailCustomization.h"

/** Adds an explicit validated Apply action to the level settings tab. */
class FKyaLevelSettingsDetailsCustomization : public IDetailCustomization
{
public:
    static TSharedRef<IDetailCustomization> MakeInstance();
    virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
};
