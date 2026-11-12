#include "KyaHierarchyDetailsCustomization.h"

#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "KyaHierarchyHandle.h"
#include "KyaLevelBank.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

TSharedRef<IDetailCustomization> FKyaHierarchyDetailsCustomization::MakeInstance()
{
    return MakeShared<FKyaHierarchyDetailsCustomization>();
}

void FKyaHierarchyDetailsCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
    TArray<TWeakObjectPtr<UObject>> Selected;
    DetailBuilder.GetObjectsBeingCustomized(Selected);
    if (Selected.Num() != 1) return;
    AKyaHierarchyHandle* Handle = Cast<AKyaHierarchyHandle>(Selected[0].Get());
    if (!Handle) return;

    IDetailCategoryBuilder& Summary = DetailBuilder.EditCategory("Kya|Hierarchy",
        FText::FromString(TEXT("Kya Hierarchy")), ECategoryPriority::Important);

    const auto AddRow = [&Summary](const FString& Label, const FString& Value, const FString& Tip)
    {
        Summary.AddCustomRow(FText::FromString(Label))
            .NameContent()
            [
                SNew(STextBlock).Text(FText::FromString(Label)).ToolTipText(FText::FromString(Tip))
            ]
            .ValueContent().MinDesiredWidth(360.0f)
            [
                SNew(STextBlock).Text(FText::FromString(Value)).AutoWrapText(true)
                    .ToolTipText(FText::FromString(Tip))
            ];
    };

    const FString SectorLabel = Handle->SectorId < 0
        ? TEXT("Global (common scene mesh)")
        : FString::Printf(TEXT("Sector %d"), Handle->SectorId);
    AddRow(TEXT("Bank"), Handle->Bank ? Handle->Bank->GetPathName() : TEXT("<none>"),
        TEXT("Bank asset whose bytes contain this hierarchy."));
    AddRow(TEXT("Owning sector"), SectorLabel,
        TEXT("Sector bank that owns this hierarchy; global is the common scene mesh."));
    AddRow(TEXT("Source facts"), FString::Printf(
        TEXT("offset %d, %d strip(s), %d triangle(s)"), Handle->HierarchyOffset,
        Handle->StripCount, Handle->TriangleCount),
        TEXT("Byte offset of this G3D hierarchy in the bank entry and its decoded strip/triangle counts."));
    AddRow(TEXT("Source origin"), FString::Printf(TEXT("(%.2f, %.2f, %.2f) m"),
        Handle->SourceOrigin.X, Handle->SourceOrigin.Y, Handle->SourceOrigin.Z),
        TEXT("Serialized hierarchy origin in game axes (metres)."));
    AddRow(TEXT("Bounds"), Handle->WorldBounds.IsValid
        ? FString::Printf(TEXT("min %s  max %s  extent %s"),
            *Handle->WorldBounds.Min.ToString(), *Handle->WorldBounds.Max.ToString(),
            *Handle->WorldBounds.GetExtent().ToString())
        : FString(TEXT("No captured geometry")),
        TEXT("World-space bounds of the rendered strips. Selection uses the rendered geometry, not this box."));
    AddRow(TEXT("Materials"), Handle->MaterialNames.IsEmpty()
        ? FString(TEXT("No captured material sections"))
        : FString::Join(Handle->MaterialNames, TEXT(", ")),
        TEXT("Material assets (or raw material hashes) used by this hierarchy's strips."));
    AddRow(TEXT("Editing scope"), TEXT(
        "Selection and visibility are presentation-only. Hierarchy transforms and material "
        "reassignment are not serialized, so moving the handle is reset and nothing is exported."),
        TEXT("The game stores no independent hierarchy transform; enabling writeback requires a later writer milestone."));
    AddRow(TEXT("Hide / isolate"), TEXT(
        "Presentation-only: hidden and isolated hierarchies stay unchanged in the bank, and "
        "rebuilding the preview does not write any bytes."),
        TEXT("Uses the same editor-only temporary hide/isolate state as other generated representations."));
    AddRow(TEXT("Transform support"), Handle->TransformStatus.IsEmpty()
        ? FString(TEXT("Disabled: no serialized hierarchy writeback. Transform edits are reset."))
        : Handle->TransformStatus,
        TEXT("A movable-looking handle must not imply export support. The status reports the last reset attempt."));
}
