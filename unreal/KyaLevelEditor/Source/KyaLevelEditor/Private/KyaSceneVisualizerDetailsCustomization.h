#pragma once

#include "CoreMinimal.h"
#include "IDetailCustomization.h"

/**
 * Explains a scene visualizer: what is verified serialized data, what is an
 * approximate preview shape, and why read-only visualizers reject transforms.
 */
class FKyaSceneVisualizerDetailsCustomization : public IDetailCustomization
{
public:
    static TSharedRef<IDetailCustomization> MakeInstance();
    virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
};
