#include "KyaSpatialRecordDetailsCustomization.h"

#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "KyaSpatialRecordHandle.h"
#include "Widgets/Text/STextBlock.h"

TSharedRef<IDetailCustomization> FKyaSpatialRecordDetailsCustomization::MakeInstance()
{
    return MakeShared<FKyaSpatialRecordDetailsCustomization>();
}

void FKyaSpatialRecordDetailsCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
    TArray<TWeakObjectPtr<UObject>> Selected;
    DetailBuilder.GetObjectsBeingCustomized(Selected);
    if (Selected.Num() != 1) return;
    AKyaSpatialRecordHandle* Handle = Cast<AKyaSpatialRecordHandle>(Selected[0].Get());
    if (!Handle) return;

    IDetailCategoryBuilder& Summary = DetailBuilder.EditCategory("Kya|Record",
        FText::FromString(TEXT("Kya Record")), ECategoryPriority::Important);
    Summary.AddCustomRow(FText::FromString(TEXT("Record")))
        .ValueContent()
        [SNew(STextBlock)
            .Text(FText::FromString(Handle->Kind == EKyaSpatialRecordKind::Waypoint
                ? FString::Printf(TEXT("Waypoint %d"), Handle->RecordIndex)
                : FString::Printf(TEXT("Path %d, point %d"), Handle->RecordIndex, Handle->PointIndex)))
            .ToolTipText(FText::FromString(TEXT("Bank, record index and generation are shown below; "
                "edits are written to the bank asset.")))];

    if (Handle->Kind == EKyaSpatialRecordKind::Waypoint)
    {
        DetailBuilder.HideCategory("Kya|Path point");
    }
    else
    {
        DetailBuilder.HideCategory("Kya|Waypoint");
        if (!Handle->bHasRotationRecord) DetailBuilder.HideProperty(TEXT("PathRotation"));
        if (!Handle->bHasExtraVectorRecord) DetailBuilder.HideProperty(TEXT("PathExtraVector"));
        if (!Handle->bHasDelayRecord) DetailBuilder.HideProperty(TEXT("PathDelay"));
        if (!Handle->bHasFlagsRecord) DetailBuilder.HideProperty(TEXT("PathFlags"));
        if (!Handle->bHasExtraValueRecord) DetailBuilder.HideProperty(TEXT("PathExtraValue"));
        if (!Handle->bHasExtraWordRecord) DetailBuilder.HideProperty(TEXT("PathExtraWord"));
    }
}
