#pragma once
#include "IDetailCustomization.h"

class FKyaDiscDetails : public IDetailCustomization
{
public:
    static TSharedRef<IDetailCustomization> MakeInstance() { return MakeShared<FKyaDiscDetails>(); }
    virtual void CustomizeDetails(IDetailLayoutBuilder& Builder) override;
};
