#pragma once

#include "ComponentVisualizer.h"
#include "KyaPrimitiveComponent.h"
#include "Editor.h"
#include "EditorViewportClient.h"

/** Picking is separate from decoding and rendering. Editing can resolve the selected
 * record's bank/entry/offset without inventing a second geometry representation. */
class FKyaPrimitiveVisualizer : public FComponentVisualizer
{
    TWeakObjectPtr<UKyaPrimitiveComponent> SelectedComponent;
public:
    virtual bool VisProxyHandleClick(FEditorViewportClient* Viewport, HComponentVisProxy* Proxy,
        const FViewportClick& Click) override
    {
        if (!Proxy || !Proxy->IsA(HKyaPrimitiveProxy::StaticGetType())) return false;
        auto* Component = Cast<UKyaPrimitiveComponent>(const_cast<UActorComponent*>(Proxy->Component.Get()));
        if (!Component) return false;
        const auto* Hit = static_cast<HKyaPrimitiveProxy*>(Proxy);
        if (Hit->Generation != Component->GetRecordGeneration()) return false;
        if (SelectedComponent.IsValid() && SelectedComponent != Component) SelectedComponent->SelectRecord(INDEX_NONE);
        SelectedComponent = Component;
        Component->SelectRecord(Hit->RecordIndex);
        GEditor->SelectActor(Component->GetOwner(), true, true);
        GEditor->SelectComponent(Component, true, true);
        Viewport->Invalidate();
        return true;
    }
    virtual void EndEditing() override
    {
        if (SelectedComponent.IsValid()) SelectedComponent->SelectRecord(INDEX_NONE);
        SelectedComponent.Reset();
    }
    // No transform widget until a serialized edit service supports this record type.
    virtual bool GetWidgetLocation(const FEditorViewportClient* Viewport, FVector& Location) const override { return false; }
};
