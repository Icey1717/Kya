#include "KyaSceneVisualizerDetailsCustomization.h"

#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "KyaLevelBank.h"
#include "KyaSceneVisualizer.h"
#include "Widgets/Text/STextBlock.h"

TSharedRef<IDetailCustomization> FKyaSceneVisualizerDetailsCustomization::MakeInstance()
{
    return MakeShared<FKyaSceneVisualizerDetailsCustomization>();
}

void FKyaSceneVisualizerDetailsCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
    TArray<TWeakObjectPtr<UObject>> Selected;
    DetailBuilder.GetObjectsBeingCustomized(Selected);
    if (Selected.Num() != 1) return;
    AKyaSceneVisualizer* Visualizer = Cast<AKyaSceneVisualizer>(Selected[0].Get());
    if (!Visualizer) return;

    IDetailCategoryBuilder& Summary = DetailBuilder.EditCategory("Kya|Visualizer",
        FText::FromString(TEXT("Kya Visualizer")), ECategoryPriority::Important);
    const auto AddRow = [&Summary](const FString& Label, const FString& Value)
    {
        Summary.AddCustomRow(FText::FromString(Label))
            .NameContent()[SNew(STextBlock).Text(FText::FromString(Label))]
            .ValueContent().MinDesiredWidth(380.0f)
            [SNew(STextBlock).Text(FText::FromString(Value)).AutoWrapText(true)];
    };

    FString KindLabel;
    switch (Visualizer->Kind)
    {
    case EKyaVisualizerKind::Light: KindLabel = TEXT("Light"); break;
    case EKyaVisualizerKind::MainCamera: KindLabel = TEXT("Main camera"); break;
    case EKyaVisualizerKind::ActorLink: KindLabel = TEXT("Actor link (derived)"); break;
    default: KindLabel = TEXT("Trigger bound (approximate)"); break;
    }
    AddRow(TEXT("Kind"), KindLabel);
    AddRow(TEXT("Source"), FString::Printf(TEXT("%s, index %d%s"),
        Visualizer->Bank ? *Visualizer->Bank->GetPathName() : TEXT("<none>"), Visualizer->RecordIndex,
        Visualizer->ReferenceField.IsNone() ? TEXT("") : *FString::Printf(TEXT(", %s"),
            *Visualizer->ReferenceField.ToString())));
    AddRow(TEXT("Semantics"), Visualizer->Semantics.IsEmpty()
        ? FString(TEXT("Unspecified visualizer semantics.")) : Visualizer->Semantics);
    AddRow(TEXT("Edit status"), Visualizer->bRecordEditable
        ? (Visualizer->EditStatus.IsEmpty()
            ? FString(TEXT("Editable through the shared edit service; rejected edits are reported here."))
            : Visualizer->EditStatus)
        : FString(TEXT("Read-only diagnostic. Transform attempts are reset and never exported.")));

    if (Visualizer->Kind != EKyaVisualizerKind::Light) DetailBuilder.HideCategory("Kya|Light");
    if (Visualizer->Kind != EKyaVisualizerKind::MainCamera) DetailBuilder.HideCategory("Kya|Camera");
    if (Visualizer->bRecordEditable) DetailBuilder.HideCategory("Kya|Derived");
}

