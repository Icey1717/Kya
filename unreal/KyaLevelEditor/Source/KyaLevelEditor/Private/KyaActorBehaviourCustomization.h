#pragma once

#include "IPropertyTypeCustomization.h"

/** Group the decoded behaviour payload and hide fields that do not apply. */
class FKyaActorBehaviourCustomization : public IPropertyTypeCustomization
{
public:
    static TSharedRef<IPropertyTypeCustomization> MakeInstance();
    virtual void CustomizeHeader(TSharedRef<IPropertyHandle> Handle, FDetailWidgetRow& Row,
        IPropertyTypeCustomizationUtils& Utils) override;
    virtual void CustomizeChildren(TSharedRef<IPropertyHandle> Handle, IDetailChildrenBuilder& Children,
        IPropertyTypeCustomizationUtils& Utils) override;
};
