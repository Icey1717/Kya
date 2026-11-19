#include "KyaGeometryDetailsCustomization.h"
#include "KyaGeometryWorkingActor.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "DetailWidgetRow.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/SBoxPanel.h"

TSharedRef<IDetailCustomization> FKyaGeometryDetailsCustomization::MakeInstance()
{
    return MakeShared<FKyaGeometryDetailsCustomization>();
}

void FKyaGeometryDetailsCustomization::CustomizeDetails(IDetailLayoutBuilder& Builder)
{
    TArray<TWeakObjectPtr<UObject>> Objects; Builder.GetObjectsBeingCustomized(Objects);
    if (Objects.Num() != 1) return;
    const TWeakObjectPtr<AKyaGeometryWorkingActor> Actor = Cast<AKyaGeometryWorkingActor>(Objects[0].Get());
    if (!Actor.IsValid()) return;
    Builder.HideCategory(TEXT("Kya Geometry"));
    IDetailCategoryBuilder& Category = Builder.EditCategory(TEXT("Kya Geometry Editing"), FText::FromString(TEXT("Kya Geometry Editing")), ECategoryPriority::Important);
    for (FName Name : {GET_MEMBER_NAME_CHECKED(AKyaGeometryWorkingActor, Bank),
        GET_MEMBER_NAME_CHECKED(AKyaGeometryWorkingActor, ResourceIdentity), GET_MEMBER_NAME_CHECKED(AKyaGeometryWorkingActor, AffectedPlacements),
        GET_MEMBER_NAME_CHECKED(AKyaGeometryWorkingActor, NativeFormatStatus), GET_MEMBER_NAME_CHECKED(AKyaGeometryWorkingActor, Status),
        GET_MEMBER_NAME_CHECKED(AKyaGeometryWorkingActor, bAssignPropertiesToNewFaces), GET_MEMBER_NAME_CHECKED(AKyaGeometryWorkingActor, SurfaceFlags),
        GET_MEMBER_NAME_CHECKED(AKyaGeometryWorkingActor, SurfaceFlagMask), GET_MEMBER_NAME_CHECKED(AKyaGeometryWorkingActor, bShowNormals)}) Category.AddProperty(Name);
    Category.AddCustomRow(FText::FromString(TEXT("Apply to Kya Bank Discard"))).WholeRowContent()
    [SNew(SHorizontalBox)
        + SHorizontalBox::Slot().AutoWidth()
        [SNew(SButton).Text(FText::FromString(TEXT("Apply to Kya Bank")))
            .IsEnabled_Lambda([Actor]() { return Actor.IsValid() && Actor->CanApplyNow(); })
            .ToolTipText(FText::FromString(TEXT("Validate and commit every changed resource in this bank session as one undo step. Unsupported native formats and stale sessions cannot apply.")))
            .OnClicked_Lambda([Actor]() { if (Actor.IsValid()) Actor->ApplyToKyaBank(); return FReply::Handled(); })]
        + SHorizontalBox::Slot().AutoWidth().Padding(8, 0)
        [SNew(SButton).Text(FText::FromString(TEXT("Discard Session")))
            .OnClicked_Lambda([Actor]() { if (Actor.IsValid()) Actor->DiscardSession(); return FReply::Handled(); })]
    ];
    Category.AddCustomRow(FText::FromString(TEXT("Copy Selected Faces to Collision"))).WholeRowContent()
    [SNew(SButton).Text(FText::FromString(TEXT("Copy Selected Faces to Collision (Append)")))
        .IsEnabled_Lambda([Actor]() { return Actor.IsValid() && Actor->Source.Key.IsCurrent() && Actor->Source.Key.Kind == EKyaGeometryKind::Render && Actor->bAssignPropertiesToNewFaces; })
        .ToolTipText(FText::FromString(TEXT("Uses Modeling Mode face selection. Choose Surface Flags and enable Assign Properties To New Faces first. Each click appends another copy into the session's single collision working resource for review.")))
        .OnClicked_Lambda([Actor]() { if (Actor.IsValid()) Actor->CopySelectedFacesToCollision(); return FReply::Handled(); })];
    Category.AddCustomRow(FText::FromString(TEXT("Set Selected Collision Surface Flags"))).WholeRowContent()
    [SNew(SButton).Text(FText::FromString(TEXT("Set Selected Collision Surface Flags")))
        .IsEnabled_Lambda([Actor]() { return Actor.IsValid() && Actor->Source.Key.IsCurrent() && Actor->Source.Key.Kind == EKyaGeometryKind::Collision; })
        .OnClicked_Lambda([Actor]() { if (Actor.IsValid()) Actor->SetSelectedCollisionSurfaceFlags(); return FReply::Handled(); })];
}
