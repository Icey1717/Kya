#pragma once

#include "IPropertyTypeCustomization.h"

/** Human-readable presentation of the original integer condition stream. */
class FKyaScenarioConditionCustomization : public IPropertyTypeCustomization
{
public:
    static TSharedRef<IPropertyTypeCustomization> MakeInstance();
    virtual void CustomizeHeader(TSharedRef<IPropertyHandle> Handle, FDetailWidgetRow& Row,
        IPropertyTypeCustomizationUtils& Utils) override;
    virtual void CustomizeChildren(TSharedRef<IPropertyHandle> Handle, IDetailChildrenBuilder& Children,
        IPropertyTypeCustomizationUtils& Utils) override;
};
