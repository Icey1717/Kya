#pragma once

#include "CoreMinimal.h"
#include "IDetailCustomization.h"

/**
 * Read-only summary for a selected hierarchy handle: owning bank/sector, the source
 * byte offset, strip/triangle counts, world bounds and the materials used.
 *
 * The hierarchy has no serialized transform; moving or scaling the handle is
 * presentation-only and is reset, so the Details panel states that transforms and
 * visibility are not exported.
 */
class FKyaHierarchyDetailsCustomization : public IDetailCustomization
{
public:
    static TSharedRef<IDetailCustomization> MakeInstance();
    virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
};
