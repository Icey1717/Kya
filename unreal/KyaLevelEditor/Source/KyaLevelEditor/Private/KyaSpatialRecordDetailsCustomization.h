#pragma once

#include "CoreMinimal.h"
#include "IDetailCustomization.h"

/**
 * Shows only the fields that apply to a waypoint or path-point handle and hides
 * per-point records the path does not serialize.
 */
class FKyaSpatialRecordDetailsCustomization : public IDetailCustomization
{
public:
    static TSharedRef<IDetailCustomization> MakeInstance();
    virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
};
