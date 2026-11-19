#pragma once
#include "IDetailCustomization.h"

class FKyaGeometryDetailsCustomization : public IDetailCustomization
{
public:
    static TSharedRef<IDetailCustomization> MakeInstance();
    virtual void CustomizeDetails(IDetailLayoutBuilder& Builder) override;
};
