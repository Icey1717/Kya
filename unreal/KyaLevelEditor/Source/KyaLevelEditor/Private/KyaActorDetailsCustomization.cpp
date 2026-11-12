#include "KyaActorDetailsCustomization.h"

#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "IDetailPropertyRow.h"
#include "IPropertyUtilities.h"
#include "PropertyHandle.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SNumericEntryBox.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Editor.h"
#include "ScopedTransaction.h"
#include "KyaActorCatalog.h"
#include "KyaActorEditService.h"
#include "KyaActorHandle.h"
#include "KyaDetailsWidgets.h"
#include "KyaLevelBank.h"
#include "KyaLevelPreview.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Misc/TransactionObjectEvent.h"

namespace
{
bool IsKyaActorDetailsProperty(const FProperty* Property)
{
    if (!Property->HasAnyPropertyFlags(CPF_Edit)) return false;
    // Export capability metadata drives the UI: only editable fields with a writer
    // path are offered, and the capability reason becomes the row tooltip.
    const FKyaFieldCapability Capability = GetKyaActorFieldCapability(Property->GetFName());
    return Capability.Kind == EKyaFieldCapability::EditableExportable ||
        Capability.Kind == EKyaFieldCapability::EditablePresentationOnly;
}

int32 GetKyaReferenceLimit(const UKyaLevelBank& Bank, EKyaReferenceKind Kind)
{
    switch (Kind)
    {
    case EKyaReferenceKind::Actor: return Bank.SceneEnvironment.Actors.Num();
    case EKyaReferenceKind::Waypoint: return Bank.SceneEnvironment.Waypoints.Num();
    case EKyaReferenceKind::Path: return Bank.SceneEnvironment.Paths.Num();
    default: return 0;
    }
}
}

TArray<FName> KyaGetActorDetailsProperties()
{
    TArray<FName> Names;
    for (TFieldIterator<FProperty> It(FKyaActorPlacement::StaticStruct()); It; ++It)
        if (IsKyaActorDetailsProperty(*It)) Names.Add(It->GetFName());
    return Names;
}

bool KyaSetActorReferencePick(const UKyaLevelBank& Bank, FKyaActorPlacement& Placement,
    FName PropertyName, int32 Value)
{
    const TArray<FKyaReferenceField> Fields = GetKyaReferenceFields();
    const FKyaReferenceField* Field = Fields.FindByPredicate(
        [PropertyName](const FKyaReferenceField& Candidate) { return Candidate.PropertyName == PropertyName; });
    if (!Field) return false;
    const int32 Limit = GetKyaReferenceLimit(Bank, Field->Kind);
    if (Value != -1 && (Value < 0 || Value >= Limit)) return false;
    return SetKyaNumericField(Placement, PropertyName, Value);
}

namespace
{
FText MakeFieldToolTip(const FProperty* Property, const FKyaFieldCapability& Capability)
{
    FString Unit;
    const FName Name = Property->GetFName();
    if (Name == GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, Position))
        Unit = TEXT("Game-space position in metres; Y is up and Z is forward.");
    else if (Name == GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, Rotation))
        Unit = TEXT("Stored Euler angles in radians. Yaw-only classes ignore pitch and roll.");
    else if (Name == GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, Scale))
        Unit = TEXT("Per-axis game-space scale.");
    else if (Name == GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, VisibilityDistance) ||
        Name == GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, CullingDistance) ||
        Name == GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, Clipping))
        Unit = TEXT("Native distance in game units (metres).");
    else if (Name == GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, SectorId))
        Unit = TEXT("Sector bank id; -1 means global and no sector.");
    else if (Name == GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, MeshIndex) ||
        Name == GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, TextureIndex))
        Unit = TEXT("Bank-local ordinal; -1 selects none/alternate.");
    else if (Name == GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, DefaultBehaviourId))
        Unit = TEXT("Index into this actor's Behaviours array.");
    else if (Name == GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, LightingValue))
        Unit = TEXT("Lighting multiplier applied by SetupLighting.");
    else if (Name == GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, BoundingSphere))
        Unit = TEXT("Game-space bounding sphere (position + radius).");

    FString Text = DescribeKyaFieldCapability(Capability.Kind).ToString();
    if (!Unit.IsEmpty()) Text += TEXT("\n") + Unit;
    if (!Capability.Reason.IsEmpty()) Text += TEXT("\n") + Capability.Reason;
    return FText::FromString(Text);
}

TSharedPtr<FKyaPickerOption> MakePickerOption(int32 Value, const FString& Label, const FString& ToolTip = FString())
{
    TSharedPtr<FKyaPickerOption> Option = MakeShared<FKyaPickerOption>();
    Option->Value = Value;
    Option->Label = Label;
    Option->ToolTip = ToolTip;
    return Option;
}

TArray<TSharedPtr<FKyaPickerOption>> MakeIndexOptions(EKyaReferenceKind Kind, const UKyaLevelBank& Bank)
{
    TArray<TSharedPtr<FKyaPickerOption>> Options;
    Options.Add(MakePickerOption(-1, TEXT("None"), TEXT("No reference.")));
    switch (Kind)
    {
    case EKyaReferenceKind::Actor:
        for (int32 Index = 0; Index < Bank.SceneEnvironment.Actors.Num(); ++Index)
        {
            const FKyaActorPlacement& Actor = Bank.SceneEnvironment.Actors[Index];
            Options.Add(MakePickerOption(Index, FString::Printf(TEXT("Actor %d: %s [class %lld]"), Index,
                Actor.Name.IsEmpty() ? TEXT("<unnamed>") : *Actor.Name, Actor.ClassId)));
        }
        break;
    case EKyaReferenceKind::Waypoint:
        for (int32 Index = 0; Index < Bank.SceneEnvironment.Waypoints.Num(); ++Index)
        {
            const FVector& Position = Bank.SceneEnvironment.Waypoints[Index].Position;
            Options.Add(MakePickerOption(Index, FString::Printf(TEXT("Waypoint %d (%.1f, %.1f, %.1f)"), Index,
                Position.X, Position.Y, Position.Z)));
        }
        break;
    case EKyaReferenceKind::Path:
        for (int32 Index = 0; Index < Bank.SceneEnvironment.Paths.Num(); ++Index)
            Options.Add(MakePickerOption(Index, FString::Printf(TEXT("Path %d (%d points)"), Index,
                Bank.SceneEnvironment.Paths[Index].Points.Num())));
        break;
    default:
        break;
    }
    return Options;
}

FString MakeIndexTip(EKyaReferenceKind Kind, int64 Value, const UKyaLevelBank& Bank)
{
    if (Value == -1) return TEXT("No reference.");
    int32 Limit = 0;
    const TCHAR* Name = TEXT("record");
    switch (Kind)
    {
    case EKyaReferenceKind::Actor: Limit = Bank.SceneEnvironment.Actors.Num(); Name = TEXT("actor"); break;
    case EKyaReferenceKind::Waypoint: Limit = Bank.SceneEnvironment.Waypoints.Num(); Name = TEXT("waypoint"); break;
    case EKyaReferenceKind::Path: Limit = Bank.SceneEnvironment.Paths.Num(); Name = TEXT("path"); break;
    default: break;
    }
    return Value >= 0 && Value < Limit
        ? FString::Printf(TEXT("%s %lld resolves."), Name, Value)
        : FString::Printf(TEXT("%s %lld does not resolve to a %s in this bank (%d available)."),
            Name, Value, Name, Limit);
}
}

TSharedRef<IDetailCustomization> FKyaActorDetailsCustomization::MakeInstance()
{
    return MakeShared<FKyaActorDetailsCustomization>();
}

FKyaActorPlacement* FKyaActorDetailsCustomization::ScopeRecord() const
{
    return PlacementScope.IsValid()
        ? reinterpret_cast<FKyaActorPlacement*>(PlacementScope->GetStructMemory()) : nullptr;
}

FText FKyaActorDetailsCustomization::GetInlineErrorText() const
{
    return FText::FromString(InlineError.IsEmpty()
        ? TEXT("No validation errors for the last edit.") : InlineError);
}

void FKyaActorDetailsCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
    TArray<TWeakObjectPtr<UObject>> Selected;
    DetailBuilder.GetObjectsBeingCustomized(Selected);

    FKyaLevelEditService& Service = FKyaLevelEditService::Get();
    if (BankEditedHandle.IsValid())
    {
        Service.OnBankEdited.Remove(BankEditedHandle);
        BankEditedHandle.Reset();
    }

    // The full placement surface is only shown for a single actor; multiple
    // selections get the shared-field bulk editor.
    TArray<AKyaActorHandle*> Handles;
    for (const TWeakObjectPtr<UObject>& Object : Selected)
        if (AKyaActorHandle* Candidate = Cast<AKyaActorHandle>(Object.Get())) Handles.Add(Candidate);
    if (Handles.Num() != 1 || !Handles[0] || !Handles[0]->Bank ||
        !Handles[0]->Bank->SceneEnvironment.Actors.IsValidIndex(Handles[0]->ActorIndex))
    {
        if (Handles.Num() > 1) AddMultiEditSection(DetailBuilder, Handles);
        return;
    }

    Handle = Handles[0];
    PlacementScope = MakeShared<FStructOnScope>(FKyaActorPlacement::StaticStruct());
    PropertyUtilities = DetailBuilder.GetPropertyUtilities();
    RefreshScopeFromBank();
    BankEditedHandle = Service.OnBankEdited.AddSP(this, &FKyaActorDetailsCustomization::OnBankEdited);
    // Undo/redo does not emit OnBankEdited; refresh the presentation copy from the
    // authoritative record when the bank is transacted.
    FCoreUObjectDelegates::OnObjectTransacted.AddSP(this, &FKyaActorDetailsCustomization::OnObjectTransacted);

    // The bank record is the single Details surface for placement fields; the old
    // hand-written mirror stays on the actor for script compatibility but is hidden.
    const FName MirroredFields[] = {
        GET_MEMBER_NAME_CHECKED(AKyaActorHandle, ActorName),
        GET_MEMBER_NAME_CHECKED(AKyaActorHandle, ActorMeshIndex),
        GET_MEMBER_NAME_CHECKED(AKyaActorHandle, ActorTextureIndex),
        GET_MEMBER_NAME_CHECKED(AKyaActorHandle, ActorPosition),
        GET_MEMBER_NAME_CHECKED(AKyaActorHandle, ActorRotation),
        GET_MEMBER_NAME_CHECKED(AKyaActorHandle, ActorScale),
        GET_MEMBER_NAME_CHECKED(AKyaActorHandle, ActorSectorId),
        GET_MEMBER_NAME_CHECKED(AKyaActorHandle, ActorDefaultBehaviourId),
        GET_MEMBER_NAME_CHECKED(AKyaActorHandle, ActorVisibilityDistance),
        GET_MEMBER_NAME_CHECKED(AKyaActorHandle, ActorLightingValue),
        GET_MEMBER_NAME_CHECKED(AKyaActorHandle, ActorLightingFlags),
    };
    for (const FName Field : MirroredFields) DetailBuilder.HideProperty(Field);

    UScriptStruct* PlacementStruct = FKyaActorPlacement::StaticStruct();
    TSet<FName> AddedCategories;
    for (TFieldIterator<FProperty> It(PlacementStruct); It; ++It)
    {
        FProperty* Property = *It;
        if (!IsKyaActorDetailsProperty(Property)) continue;
        const FKyaFieldCapability Capability = GetKyaActorFieldCapability(Property->GetFName());

        FString CategoryName = TEXT("Bank record");
        if (const FString* Metadata = Property->FindMetaData(TEXT("Category")))
        {
            CategoryName = *Metadata;
            // Hide the handle's own prefix if a struct category ever carries one.
            CategoryName.RemoveFromStart(TEXT("Kya|"));
        }
        const FName CategoryFName(*CategoryName);
        IDetailCategoryBuilder& Category = DetailBuilder.EditCategory(CategoryFName,
            FText::FromString(CategoryName), ECategoryPriority::Important);
        AddedCategories.Add(CategoryFName);

        IDetailPropertyRow* Row = Category.AddExternalStructureProperty(PlacementScope, Property->GetFName());
        if (!Row) continue;
        Row->DisplayName(FText::FromString(Property->GetDisplayNameText().ToString()));
        Row->ToolTip(MakeFieldToolTip(Property, Capability));
        // Nested edits (array elements, sub-struct fields) notify the child handle.
        if (TSharedPtr<IPropertyHandle> PropertyHandle = Row->GetPropertyHandle())
        {
            const FName PropertyName = Property->GetFName();
            PropertyHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(
                this, &FKyaActorDetailsCustomization::OnPlacementPropertyChanged, PropertyName));
            PropertyHandle->SetOnChildPropertyValueChanged(FSimpleDelegate::CreateSP(
                this, &FKyaActorDetailsCustomization::OnPlacementPropertyChanged, PropertyName));
        }
    }

    AKyaActorHandle* Actor = Handle.Get();
    UKyaLevelBank* Bank = Actor->Bank;
    const FKyaActorPlacement& Record = Bank->SceneEnvironment.Actors[Actor->ActorIndex];

    // --- Class and identity -------------------------------------------------------
    IDetailCategoryBuilder& Identity = DetailBuilder.EditCategory("Kya|Identity",
        FText::FromString(TEXT("Kya Identity")), ECategoryPriority::Important);
    const FKyaActorClassInfo ClassInfo = GetKyaActorClassInfo(Record.ClassId);
    Identity.AddCustomRow(FText::FromString(TEXT("Class")))
        .NameContent()
        [
            SNew(STextBlock).Text(FText::FromString(TEXT("Game class")))
        ]
        .ValueContent()
        [
            SNew(STextBlock)
            .Text(FText::FromString(GetKyaActorClassLabel(Record.ClassId)))
            .ToolTipText(FText::FromString(ClassInfo.Notes.IsEmpty()
                ? TEXT("Recovered game class name and numeric id.") : ClassInfo.Notes))
        ];
    Identity.AddCustomRow(FText::FromString(TEXT("Bank")))
        .NameContent()
        [
            SNew(STextBlock).Text(FText::FromString(TEXT("Bank asset")))
        ]
        .ValueContent()
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot().FillWidth(1.0f).VAlign(VAlign_Center)
            [
                SNew(STextBlock).Text(FText::FromString(Bank->GetPathName()))
            ]
            + SHorizontalBox::Slot().AutoWidth()
            [
                SNew(SButton)
                .Text(FText::FromString(TEXT("Browse")))
                .ToolTipText(FText::FromString(TEXT("Select the bank asset in the Content Browser.")))
                .OnClicked_Lambda([Bank]()
                {
                    if (GEditor)
                    {
                        TArray<UObject*> Objects;
                        Objects.Add(Bank);
                        GEditor->SyncBrowserToObjects(Objects);
                    }
                    return FReply::Handled();
                })
            ]
        ];

    // --- References ---------------------------------------------------------------
    IDetailCategoryBuilder& References = DetailBuilder.EditCategory("Kya|References",
        FText::FromString(TEXT("Kya References")), ECategoryPriority::Important);

    const auto AddPickerRow = [this, &References](const FString& Label, TArray<TSharedPtr<FKyaPickerOption>> Options,
        int32 CurrentValue, const FString& Tip, FName PropertyName, TFunction<void(FKyaActorPlacement&, int32)> Mutate,
        TFunction<void()> SelectReferenced = nullptr)
    {
        TSharedRef<SHorizontalBox> Row = SNew(SHorizontalBox);
        Row->AddSlot().FillWidth(1.0f)
        [
            SNew(SKyaReferencePicker)
            .Options(Options)
            .CurrentValue(CurrentValue)
            .GetCurrentValue([this, PropertyName]()
            {
                FKyaActorPlacement* Scope = ScopeRecord();
                return Scope ? int32(GetKyaNumericField(*Scope, PropertyName)) : -1;
            })
            .ResolvedLabel(FText::FromString(Tip))
            .OnValuePicked(FOnKyaPickerValuePicked::CreateLambda([this, PropertyName, Mutate](int32 Value)
            {
                if (FKyaActorPlacement* Scope = ScopeRecord())
                {
                    FKyaActorPlacement Modified = *Scope;
                    Mutate(Modified, Value);
                    ApplyPlacementCopy(Modified, PropertyName);
                }
            }))
        ];
        if (SelectReferenced)
        {
            Row->AddSlot().AutoWidth().Padding(4.0f, 0.0f)
            [
                SNew(SButton)
                .Text(FText::FromString(TEXT("Select")))
                .ToolTipText(FText::FromString(TEXT("Select the referenced actor in the viewport and Outliner.")))
                .OnClicked_Lambda([SelectReferenced]()
                {
                    SelectReferenced();
                    return FReply::Handled();
                })
            ];
        }
        References.AddCustomRow(FText::FromString(Label))
            .NameContent()
            [
                SNew(STextBlock).Text(FText::FromString(Label)).ToolTipText(FText::FromString(Tip))
            ]
            .ValueContent().MinDesiredWidth(320.0f)
            [
                Row
            ];
    };

    // Sector picker: loaded sector banks (descriptor-authoritative ids) plus the
    // current value even when no bank provides it.
    TArray<TSharedPtr<FKyaPickerOption>> SectorOptions;
    SectorOptions.Add(MakePickerOption(-1, TEXT("Global (no sector)"),
        TEXT("Actor belongs to the whole level rather than one sector.")));
    TMap<int32, FString> SectorNames;
    if (AKyaLevelPreview* Preview = Actor->Preview.Get())
    {
        for (const TObjectPtr<UKyaLevelBank>& Sector : Preview->SectorBanks)
        {
            if (!Sector) continue;
            const int32 SectorId = Preview->ResolveSectorId(*Sector);
            if (!SectorNames.Contains(SectorId)) SectorNames.Add(SectorId, Sector->GetName());
        }
    }
    for (const TPair<int32, FString>& Pair : SectorNames)
        SectorOptions.Add(MakePickerOption(Pair.Key, FString::Printf(TEXT("Sector %d (%s)"), Pair.Key, *Pair.Value)));
    if (Record.SectorId >= 0 && !SectorNames.Contains(Record.SectorId))
        SectorOptions.Add(MakePickerOption(Record.SectorId,
            FString::Printf(TEXT("Sector %d (not loaded)"), Record.SectorId),
            TEXT("No loaded sector bank claims this id; the association is kept as authored.")));
    AddPickerRow(TEXT("Sector"), SectorOptions, Record.SectorId,
        GetKyaSectorLabel(SectorNames, Record.SectorId), GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, SectorId),
        [](FKyaActorPlacement& Placement, int32 Value) { Placement.SectorId = Value; });

    // Candidate-sector diagnostics: the assignment stays explicit. Loaded sector
    // backgrounds only suggest where the authored position currently falls.
    if (AKyaLevelPreview* Preview = Actor->Preview.Get())
    {
        References.AddCustomRow(FText::FromString(TEXT("Sector candidates")))
            .NameContent()
            [
                SNew(STextBlock).Text(FText::FromString(TEXT("Sector candidates")))
                    .ToolTipText(FText::FromString(TEXT(
                        "Diagnostic only. The actor's sector stays explicit and is never "
                        "reassigned from background bounds.")))
            ]
            .ValueContent().MinDesiredWidth(320.0f)
            [
                SNew(STextBlock)
                    .Text_Lambda([this]()
                    {
                        AKyaActorHandle* Source = Handle.Get();
                        FKyaActorPlacement* Scope = ScopeRecord();
                        AKyaLevelPreview* LivePreview = Source ? Source->Preview.Get() : nullptr;
                        if (!Scope || !LivePreview)
                            return FText::FromString(TEXT(
                                "No loaded sector bounds; the explicit assignment is kept."));
                        return FText::FromString(LivePreview->DescribeSectorAssignment(
                            Scope->SectorId, Scope->Position));
                    })
                    .AutoWrapText(true)
                    .ToolTipText(FText::FromString(TEXT(
                        "Approximate containment in loaded sector backgrounds. This does not "
                        "change the authored sector and is not a native membership test.")))
            ];
    }

    // Mesh picker: bank-local type-4 mesh ordinals.
    TArray<TSharedPtr<FKyaPickerOption>> MeshOptions;
    MeshOptions.Add(MakePickerOption(-1, TEXT("None"), TEXT("This actor has no model mesh.")));
    for (const int32 Ordinal : GetKyaModelOrdinals(*Bank))
        MeshOptions.Add(MakePickerOption(Ordinal, FString::Printf(TEXT("Mesh #%d"), Ordinal)));
    const FKyaBankReference Mesh = ResolveKyaModelReference(*Bank, Record.MeshIndex);
    if (!Mesh.bResolved && Record.MeshIndex >= 0)
        MeshOptions.Add(MakePickerOption(Record.MeshIndex, FString::Printf(TEXT("Mesh #%d (missing)"), Record.MeshIndex),
            Mesh.Warning));
    AddPickerRow(TEXT("Model mesh"), MeshOptions, Record.MeshIndex,
        Mesh.bResolved ? FString::Printf(TEXT("%s (%d mesh entries)"), *Mesh.Label, Mesh.EntryCount) : Mesh.Warning,
        GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, MeshIndex),
        [](FKyaActorPlacement& Placement, int32 Value) { Placement.MeshIndex = Value; });

    // Texture picker: bank-local type-5 texture banks, plus -1 for the alternate bank.
    TArray<TSharedPtr<FKyaPickerOption>> TextureOptions;
    TextureOptions.Add(MakePickerOption(-1, TEXT("Alternate texture bank"),
        TEXT("Uses the scene's alternate actor-material bank.")));
    for (const int32 Ordinal : GetKyaTextureBankOrdinals(*Bank))
        TextureOptions.Add(MakePickerOption(Ordinal, FString::Printf(TEXT("Texture bank #%d"), Ordinal)));
    const FKyaBankReference Texture = ResolveKyaTextureReference(*Bank, Record.TextureIndex);
    if (!Texture.bResolved && Record.TextureIndex >= 0)
        TextureOptions.Add(MakePickerOption(Record.TextureIndex,
            FString::Printf(TEXT("Texture bank #%d (missing)"), Record.TextureIndex), Texture.Warning));
    AddPickerRow(TEXT("Texture bank"), TextureOptions, Record.TextureIndex,
        Texture.bResolved ? FString::Printf(TEXT("%s (%d texture banks)"), *Texture.Label, Texture.EntryCount) : Texture.Warning,
        GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, TextureIndex),
        [](FKyaActorPlacement& Placement, int32 Value) { Placement.TextureIndex = Value; });

    // Default behaviour picker: recovered behaviour names.
    TArray<TSharedPtr<FKyaPickerOption>> BehaviourOptions;
    for (const FKyaBehaviourOption& Option : GetKyaBehaviourOptions(Record))
        BehaviourOptions.Add(MakePickerOption(Option.Id, Option.Label));
    AddPickerRow(TEXT("Default behaviour"), BehaviourOptions, Record.DefaultBehaviourId,
        Record.Behaviours.IsValidIndex(Record.DefaultBehaviourId)
            ? GetKyaBehaviourLabel(Record.ClassId, Record.Behaviours[Record.DefaultBehaviourId].Id)
            : TEXT("The default behaviour index does not resolve to a behaviour record."),
        GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, DefaultBehaviourId),
        [](FKyaActorPlacement& Placement, int32 Value) { Placement.DefaultBehaviourId = Value; });

    // Named behaviour summary (recovered names, unknown ids labelled explicitly).
    if (!Record.Behaviours.IsEmpty())
    {
        TArray<FString> BehaviourLines;
        for (int32 Index = 0; Index < Record.Behaviours.Num(); ++Index)
            BehaviourLines.Add(FString::Printf(TEXT("%d: %s"), Index,
                *GetKyaBehaviourLabel(Record.ClassId, Record.Behaviours[Index].Id)));
        References.AddCustomRow(FText::FromString(TEXT("Behaviours")))
            .NameContent()
            [
                SNew(STextBlock).Text(FText::FromString(TEXT("Behaviours")))
            ]
            .ValueContent()
            [
                SNew(STextBlock)
                .Text(FText::FromString(FString::Join(BehaviourLines, TEXT("  |  "))))
                .AutoWrapText(true)
                .ToolTipText(FText::FromString(TEXT("Recovered behaviour names; ids without a recovered name are labelled unknown.")))
            ];
    }

    // Remaining top-level scalar references (actor, waypoint and path indices) get
    // the same searchable picker, with raw ids and inline range checks.
    {
        const TSet<FName> FixedFields = {
            GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, SectorId),
            GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, MeshIndex),
            GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, TextureIndex),
            GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, DefaultBehaviourId),
        };
        TWeakObjectPtr<UKyaLevelBank> WeakBank = Bank;
        for (const FKyaReferenceField& Field : GetKyaReferenceFields())
        {
            if (FixedFields.Contains(Field.PropertyName) || Field.Kind == EKyaReferenceKind::Zone) continue;
            if (Field.ClassId != -1 && Field.ClassId != Record.ClassId) continue;
            const FProperty* Property = FKyaActorPlacement::StaticStruct()->FindPropertyByName(Field.PropertyName);
            if (!Property || !Property->HasAnyPropertyFlags(CPF_Edit) ||
                Property->HasAnyPropertyFlags(CPF_EditConst)) continue;
            if (GetKyaActorFieldCapability(Field.PropertyName).Kind != EKyaFieldCapability::EditableExportable)
                continue;
            const int64 Raw = GetKyaNumericField(Record, Field.PropertyName);
            const FString Label = FString::Printf(TEXT("%s (%s)"), *Property->GetDisplayNameText().ToString(),
                *DescribeKyaReferenceKind(Field.Kind).ToString());
            const FName PropertyName = Field.PropertyName;
            const EKyaReferenceKind Kind = Field.Kind;
            TFunction<void()> SelectReferenced;
            if (Kind == EKyaReferenceKind::Actor || Kind == EKyaReferenceKind::Waypoint ||
                Kind == EKyaReferenceKind::Path)
            {
                SelectReferenced = [this, PropertyName, Kind]()
                {
                    AKyaActorHandle* Source = Handle.Get();
                    FKyaActorPlacement* Scope = ScopeRecord();
                    if (!Source || !IsValid(Source->Preview) || !Scope || !GEditor) return;
                    AKyaLevelPreview* Preview = Source->Preview.Get();
                    const int32 Index = int32(GetKyaNumericField(*Scope, PropertyName));
                    AActor* Target = nullptr;
                    switch (Kind)
                    {
                    case EKyaReferenceKind::Actor: Target = Preview->GetActorHandle(Index); break;
                    case EKyaReferenceKind::Waypoint: Target = Preview->GetWaypointHandle(Index); break;
                    case EKyaReferenceKind::Path: Target = Preview->GetPathPointHandle(Index, 0); break;
                    default: break;
                    }
                    if (!Target)
                    {
                        UE_LOG(LogTemp, Display, TEXT("KYA_REFERENCE_SELECT missing handle for %s %d; enable the "
                            "corresponding Spawn ... Handles option on the preview to navigate to it."),
                            *DescribeKyaReferenceKind(Kind).ToString(), Index);
                        return;
                    }
                    GEditor->SelectNone(false, true);
                    GEditor->SelectActor(Target, true, true);
                };
            }
            AddPickerRow(Label, MakeIndexOptions(Kind, *Bank), int32(Raw),
                MakeIndexTip(Kind, Raw, *Bank), PropertyName,
                [PropertyName, WeakBank](FKyaActorPlacement& Placement, int32 Value)
                {
                    // Refuse out-of-range picks rather than storing a misleading reference.
                    UKyaLevelBank* EditedBank = WeakBank.Get();
                    if (!EditedBank) return;
                    KyaSetActorReferencePick(*EditedBank, Placement, PropertyName, Value);
                }, SelectReferenced);
        }
    }

    // Inline validation feedback for the last rejected edit.
    References.AddCustomRow(FText::FromString(TEXT("Validation")))
        .NameContent()
        [
            SNew(STextBlock).Text(FText::FromString(TEXT("Validation")))
        ]
        .ValueContent().MinDesiredWidth(320.0f)
        [
            SAssignNew(InlineErrorText, STextBlock)
            .Text(this, &FKyaActorDetailsCustomization::GetInlineErrorText)
            .AutoWrapText(true)
            .ColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.65f, 0.2f, 1.0f)))
        ];

    // Unresolved-reference warnings, shown inline instead of waiting for export.
    const TArray<FKyaActorReferenceIssue> Issues = ValidateKyaActorReferences(*Bank, Record);    if (!Issues.IsEmpty())
    {
        TArray<FString> Lines;
        for (const FKyaActorReferenceIssue& Issue : Issues)
            Lines.Add(FString::Printf(TEXT("%s: %s%s"), *Issue.Field, *Issue.Message,
                Issue.bBlocking ? TEXT(" (blocks export)") : TEXT(" (warning)")));
        References.AddCustomRow(FText::FromString(TEXT("Reference warnings")))
            .NameContent()
            [
                SNew(STextBlock).Text(FText::FromString(TEXT("Warnings")))
            ]
            .ValueContent()
            [
                SNew(STextBlock)
                .Text(FText::FromString(FString::Join(Lines, TEXT("\n"))))
                .ColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.75f, 0.2f, 1.0f)))
                .AutoWrapText(true)
            ];
    }

    // --- Flags -------------------------------------------------------------------
    IDetailCategoryBuilder& Flags = DetailBuilder.EditCategory("Kya|Flags",
        FText::FromString(TEXT("Kya Flags")), ECategoryPriority::Default);

    const auto AddFlagRow = [this, &Flags](const FString& Label, int64 Value, TArray<TSharedPtr<FKyaFlagBit>> Bits,
        FName PropertyName, const FString& Tip)
    {
        Flags.AddCustomRow(FText::FromString(Label))
            .NameContent()
            [
                SNew(STextBlock).Text(FText::FromString(Label)).ToolTipText(FText::FromString(Tip))
            ]
            .ValueContent().MinDesiredWidth(320.0f)
            [
                SNew(SKyaFlagBits)
                .Value(Value)
                .Bits(Bits)
                .GetCurrentValue([this, PropertyName]()
                {
                    FKyaActorPlacement* Scope = ScopeRecord();
                    return Scope ? GetKyaNumericField(*Scope, PropertyName) : int64(0);
                })
                .OnValueChanged(FOnKyaFlagValueChanged::CreateLambda([this, PropertyName](int64 NewValue)
                {
                    if (FKyaActorPlacement* Scope = ScopeRecord())
                    {
                        FKyaActorPlacement Modified = *Scope;
                        if (PropertyName == GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, ActorFlags))
                            Modified.ActorFlags = int32(NewValue);
                        else if (PropertyName == GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, LightingFlags))
                            Modified.LightingFlags = NewValue;
                        ApplyPlacementCopy(Modified, PropertyName);
                    }
                }))
            ];
    };

    TArray<TSharedPtr<FKyaFlagBit>> LightingBits;
    {
        TSharedPtr<FKyaFlagBit> Bit = MakeShared<FKyaFlagBit>();
        Bit->Mask = 2;
        Bit->Label = TEXT("Ignore ground lighting");
        Bit->ToolTip = TEXT("Serialized bit 1 disables the scenery ground-light modulation in SetupLighting.");
        LightingBits.Add(Bit);
    }
    AddFlagRow(TEXT("Lighting flags"), Record.LightingFlags, LightingBits,
        GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, LightingFlags),
        TEXT("Known lighting bits are shown as toggles; remaining bits are preserved."));
    AddFlagRow(TEXT("Actor flags"), Record.ActorFlags, {},
        GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, ActorFlags),
        TEXT("No actor-flag bits are recovered; the raw value is preserved exactly."));

    // --- Imported baseline: reset and property copy --------------------------------
    IDetailCategoryBuilder& Baseline = DetailBuilder.EditCategory("Kya|Baseline",
        FText::FromString(TEXT("Kya Baseline")), ECategoryPriority::Default);

    Baseline.AddCustomRow(FText::FromString(TEXT("Imported state")))
        .NameContent()
        [
            SNew(STextBlock).Text(FText::FromString(TEXT("Imported state")))
        ]
        .ValueContent()
        [
            SNew(STextBlock)
            .Text(FText::FromString(KyaActorDiffersFromImported(*Bank, Actor->ActorIndex)
                ? TEXT("Modified from the imported bank") : TEXT("Matches the imported bank")))
            .ToolTipText(FText::FromString(TEXT("Compares the current typed record with the baseline captured at import; "
                "the baseline is separate from Unreal class defaults.")))
        ];

    Baseline.AddCustomRow(FText::FromString(TEXT("Reset actor")))
        .NameContent()
        [
            SNew(STextBlock).Text(FText::FromString(TEXT("Reset actor")))
        ]
        .ValueContent()
        [
            SNew(SButton)
            .Text(FText::FromString(TEXT("Reset to imported")))
            .ToolTipText(FText::FromString(TEXT("Restore every supported field of this actor from the imported baseline.")))
            .OnClicked_Lambda([this, Actor]()
            {
                FKyaActorEditPatch Patch;
                if (!BuildKyaActorResetPatch(*Actor->Bank, Actor->ActorIndex, Patch))
                {
                    InlineError = TEXT("No imported baseline is available for this bank; reimport the source bank.");
                    return FReply::Handled();
                }
                ApplyPreparedPatch(Patch, TEXT("Reset actor"));
                return FReply::Handled();
            })
        ];

    // Field list shared by reset-field and copy-field actions.
    TSharedPtr<TArray<FName>> FieldNames = MakeShared<TArray<FName>>();
    TArray<TSharedPtr<FKyaPickerOption>> FieldOptions;
    for (TFieldIterator<FProperty> It(FKyaActorPlacement::StaticStruct()); It; ++It)
    {
        const FKyaFieldCapability Capability = GetKyaActorFieldCapability(It->GetFName());
        if (Capability.Kind != EKyaFieldCapability::EditableExportable &&
            Capability.Kind != EKyaFieldCapability::EditablePresentationOnly) continue;
        FieldOptions.Add(MakePickerOption(FieldNames->Num(), It->GetDisplayNameText().ToString(),
            Capability.Reason));
        FieldNames->Add(It->GetFName());
    }
    TSharedPtr<int32> SelectedField = MakeShared<int32>(0);
    TSharedPtr<int32> SourceIndex = MakeShared<int32>(Actor->ActorIndex);

    Baseline.AddCustomRow(FText::FromString(TEXT("Reset field")))
        .NameContent()
        [
            SNew(STextBlock).Text(FText::FromString(TEXT("Reset field")))
        ]
        .ValueContent().MinDesiredWidth(420.0f)
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot().FillWidth(1.0f)
            [
                SNew(SKyaReferencePicker)
                .Options(FieldOptions)
                .CurrentValue(0)
                .ResolvedLabel(FText::FromString(TEXT("Choose a field to restore from the imported baseline.")))
                .OnValuePicked(FOnKyaPickerValuePicked::CreateLambda([SelectedField](int32 Value)
                {
                    *SelectedField = Value;
                }))
            ]
            + SHorizontalBox::Slot().AutoWidth().Padding(4.0f, 0.0f)
            [
                SNew(SButton)
                .Text(FText::FromString(TEXT("Reset field")))
                .ToolTipText(FText::FromString(TEXT("Restore only this property from the imported baseline.")))
                .OnClicked_Lambda([this, Actor, FieldNames, SelectedField]()
                {
                    if (!FieldNames->IsValidIndex(*SelectedField)) return FReply::Handled();
                    const FName PropertyName = (*FieldNames)[*SelectedField];
                    FKyaActorEditPatch Patch;
                    if (!BuildKyaActorFieldResetPatch(*Actor->Bank, Actor->ActorIndex, PropertyName, Patch))
                    {
                        InlineError = FString::Printf(TEXT("%s: no imported value is available."),
                            *PropertyName.ToString());
                        return FReply::Handled();
                    }
                    ApplyPreparedPatch(Patch, FString::Printf(TEXT("Reset %s"), *PropertyName.ToString()));
                    return FReply::Handled();
                })
            ]
        ];

    TArray<TSharedPtr<FKyaPickerOption>> CopySourceOptions = MakeIndexOptions(EKyaReferenceKind::Actor, *Bank);
    Baseline.AddCustomRow(FText::FromString(TEXT("Copy field")))
        .NameContent()
        [
            SNew(STextBlock).Text(FText::FromString(TEXT("Copy field")))
        ]
        .ValueContent().MinDesiredWidth(420.0f)
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot().FillWidth(1.0f)
            [
                SNew(SKyaReferencePicker)
                .Options(CopySourceOptions)
                .CurrentValue(Actor->ActorIndex)
                .ResolvedLabel(FText::FromString(TEXT("Source actor in this bank.")))
                .OnValuePicked(FOnKyaPickerValuePicked::CreateLambda([SourceIndex](int32 Value)
                {
                    *SourceIndex = Value;
                }))
            ]
            + SHorizontalBox::Slot().FillWidth(1.0f).Padding(4.0f, 0.0f)
            [
                SNew(SKyaReferencePicker)
                .Options(FieldOptions)
                .CurrentValue(0)
                .ResolvedLabel(FText::FromString(TEXT("Field to copy from the source actor.")))
                .OnValuePicked(FOnKyaPickerValuePicked::CreateLambda([SelectedField](int32 Value)
                {
                    *SelectedField = Value;
                }))
            ]
            + SHorizontalBox::Slot().AutoWidth()
            [
                SNew(SButton)
                .Text(FText::FromString(TEXT("Copy")))
                .ToolTipText(FText::FromString(TEXT("Copy the chosen field from the source actor; identity and layout are preserved.")))
                .OnClicked_Lambda([this, Actor, FieldNames, SelectedField, SourceIndex]()
                {
                    UKyaLevelBank* EditedBank = Actor->Bank;
                    if (!EditedBank || !FieldNames->IsValidIndex(*SelectedField) ||
                        !EditedBank->SceneEnvironment.Actors.IsValidIndex(*SourceIndex))
                        return FReply::Handled();
                    const FName PropertyName = (*FieldNames)[*SelectedField];
                    const FKyaActorPlacement& SourceRecord = EditedBank->SceneEnvironment.Actors[*SourceIndex];
                    const FKyaActorPlacement& DestRecord = EditedBank->SceneEnvironment.Actors[Actor->ActorIndex];
                    FString CompatError;
                    if (!CanCopyKyaActorField(SourceRecord, DestRecord, PropertyName, CompatError))
                    {
                        InlineError = FString::Printf(TEXT("Copy %s: %s"), *PropertyName.ToString(), *CompatError);
                        return FReply::Handled();
                    }
                    FKyaActorEditPatch Patch;
                    FString Error;
                    if (!KyaMakePlacementPropertyPatch(SourceRecord, PropertyName, Patch, &Error))
                    {
                        InlineError = FString::Printf(TEXT("%s: %s"), *PropertyName.ToString(), *Error);
                        return FReply::Handled();
                    }
                    ApplyPreparedPatch(Patch, FString::Printf(TEXT("Copy %s"), *PropertyName.ToString()));
                    return FReply::Handled();
                })
            ]
        ];
}

void FKyaActorDetailsCustomization::ApplyPlacementCopy(const FKyaActorPlacement& Modified, FName PropertyName)
{
    AKyaActorHandle* Actor = Handle.Get();
    if (!Actor || !Actor->Bank || !Actor->Bank->SceneEnvironment.Actors.IsValidIndex(Actor->ActorIndex)) return;
    FKyaActorEditPatch Patch;
    FString Error;
    if (!KyaMakePlacementPropertyPatch(Modified, PropertyName, Patch, &Error))
    {
        InlineError = FString::Printf(TEXT("%s: %s"), *PropertyName.ToString(), *Error);
        UE_LOG(LogTemp, Warning, TEXT("KYA_DETAILS_FIELD_UNSUPPORTED field=%s reason=%s"),
            *PropertyName.ToString(), *Error);
        return;
    }
    FKyaLevelEditService& Service = FKyaLevelEditService::Get();
    TGuardValue<bool> Guard(bApplying, true);
    bool bApplied = false;
    if (GEditor && !GEditor->IsTransactionActive())
    {
        FScopedTransaction Transaction(NSLOCTEXT("KyaLevelEditor", "KyaActorReferenceEdit", "Edit Kya Actor Reference"));
        bApplied = Service.Apply(Service.MakeKey(Actor->Bank, Actor->ActorIndex), Patch, EKyaEditNotify::Notify, &Error);
    }
    else
    {
        bApplied = Service.Apply(Service.MakeKey(Actor->Bank, Actor->ActorIndex), Patch, EKyaEditNotify::Notify, &Error);
    }
    if (!bApplied)
    {
        InlineError = FString::Printf(TEXT("%s: %s"), *PropertyName.ToString(), *Error);
        UE_LOG(LogTemp, Warning, TEXT("KYA_DETAILS_REFERENCE_WRITE_FAILED bank=%s field=%s reason=%s"),
            *GetNameSafe(Actor->Bank), *PropertyName.ToString(), *Error);
        RefreshScopeFromBank();
        return;
    }
    InlineError.Reset();
    if (FKyaActorPlacement* Scope = ScopeRecord()) *Scope = Modified;
}

bool FKyaActorDetailsCustomization::ApplyPreparedPatch(const FKyaActorEditPatch& Patch, const FString& Context)
{
    AKyaActorHandle* Actor = Handle.Get();
    if (!Actor || !Actor->Bank || !Actor->Bank->SceneEnvironment.Actors.IsValidIndex(Actor->ActorIndex))
        return false;
    FKyaLevelEditService& Service = FKyaLevelEditService::Get();
    FString Error;
    TGuardValue<bool> Guard(bApplying, true);
    bool bApplied = false;
    if (GEditor && !GEditor->IsTransactionActive())
    {
        FScopedTransaction Transaction(NSLOCTEXT("KyaLevelEditor", "KyaActorBaselineEdit", "Edit Kya Actor Baseline"));
        bApplied = Service.Apply(Service.MakeKey(Actor->Bank, Actor->ActorIndex), Patch, EKyaEditNotify::Notify, &Error);
    }
    else
    {
        bApplied = Service.Apply(Service.MakeKey(Actor->Bank, Actor->ActorIndex), Patch, EKyaEditNotify::Notify, &Error);
    }
    if (!bApplied)
    {
        InlineError = FString::Printf(TEXT("%s: %s"), *Context, *Error);
        UE_LOG(LogTemp, Warning, TEXT("KYA_BASELINE_WRITE_FAILED bank=%s context=%s reason=%s"),
            *GetNameSafe(Actor->Bank), *Context, *Error);
        RefreshScopeFromBank();
        return false;
    }
    InlineError.Reset();
    RefreshScopeFromBank();
    return true;
}

void FKyaActorDetailsCustomization::AddMultiEditSection(IDetailLayoutBuilder& DetailBuilder,
    const TArray<AKyaActorHandle*>& Handles)
{
    TArray<AKyaActorHandle*> Usable;
    for (AKyaActorHandle* Candidate : Handles)
        if (Candidate && Candidate->Bank && Candidate->Bank->SceneEnvironment.Actors.IsValidIndex(Candidate->ActorIndex))
            Usable.Add(Candidate);
    if (Usable.IsEmpty()) return;

    IDetailCategoryBuilder& Category = DetailBuilder.EditCategory("Kya|Multi-edit",
        FText::FromString(TEXT("Kya Multi-edit")), ECategoryPriority::Important);

    const FKyaActorPlacement& First = Usable[0]->Bank->SceneEnvironment.Actors[Usable[0]->ActorIndex];
    const int64 ClassId = First.ClassId;
    bool bSameClass = true;
    for (int32 Index = 1; Index < Usable.Num(); ++Index)
    {
        const FKyaActorPlacement& Other = Usable[Index]->Bank->SceneEnvironment.Actors[Usable[Index]->ActorIndex];
        if (Other.ClassId != ClassId) bSameClass = false;
    }
    // Optional/class-specific records must match too, otherwise a class-specific field
    // would be applied to an incompatible selection.
    const auto PresenceSignature = [](const FKyaActorPlacement& Placement)
    {
        TArray<FString> Parts;
        for (TFieldIterator<FBoolProperty> It(FKyaActorPlacement::StaticStruct()); It; ++It)
        {
            if (!It->GetName().StartsWith(TEXT("bHas"))) continue;
            Parts.Add(FString::Printf(TEXT("%s=%d"), *It->GetName(),
                It->GetPropertyValue_InContainer(&Placement) ? 1 : 0));
        }
        return FString::Join(Parts, TEXT(","));
    };
    const FString FirstSignature = PresenceSignature(First);
    bool bCompatible = bSameClass;
    for (int32 Index = 1; Index < Usable.Num(); ++Index)
    {
        const FKyaActorPlacement& Other = Usable[Index]->Bank->SceneEnvironment.Actors[Usable[Index]->ActorIndex];
        if (PresenceSignature(Other) != FirstSignature) bCompatible = false;
    }

    TArray<FKyaActorRecordKey> Keys;
    for (AKyaActorHandle* Candidate : Usable)
        Keys.Add(FKyaLevelEditService::Get().MakeKey(Candidate->Bank, Candidate->ActorIndex));
    TWeakObjectPtr<UKyaLevelBank> WeakFirstBank = Usable[0]->Bank;

    Category.AddCustomRow(FText::FromString(TEXT("Selection")))
        .NameContent()
        [
            SNew(STextBlock).Text(FText::FromString(TEXT("Selection")))
        ]
        .ValueContent()
        [
            SNew(STextBlock)
            .Text(FText::FromString(FString::Printf(TEXT("%d actors, class %s"), Usable.Num(),
                *GetKyaActorClassLabel(ClassId))))
            .AutoWrapText(true)
            .ToolTipText(FText::FromString(bCompatible
                ? TEXT("Shared fields can be edited for the whole selection; each edit is one undo step.")
                : TEXT("Class-specific fields are hidden because the selection mixes classes or optional records. "
                    "Shared transform and actor settings remain editable.")))
        ];
    if (!bCompatible)
    {
        Category.AddCustomRow(FText::FromString(TEXT("Compatibility")))
            .NameContent()
            [
                SNew(STextBlock).Text(FText::FromString(TEXT("Class fields")))
            ]
            .ValueContent()
            [
                SNew(STextBlock)
                .Text(FText::FromString(TEXT("Hidden: the selection mixes classes or optional records. Shared fields only.")))
                .ColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.75f, 0.2f, 1.0f)))
                .AutoWrapText(true)
            ];
    }

    // Bulk apply: one transaction, one validation pass, one refresh per bank.
    const auto ApplyBulkField = [this, Keys](FName PropertyName, TFunction<void(FKyaActorPlacement&)> Mutate) -> bool
    {
        UKyaLevelBank* Bank = Keys[0].Bank.Get();
        if (!Bank || !Bank->SceneEnvironment.Actors.IsValidIndex(Keys[0].RecordIndex)) return false;
        FKyaActorPlacement Source = Bank->SceneEnvironment.Actors[Keys[0].RecordIndex];
        Mutate(Source);
        FKyaActorEditPatch Patch;
        FString Error;
        if (!KyaMakePlacementPropertyPatch(Source, PropertyName, Patch, &Error))
        {
            InlineError = FString::Printf(TEXT("%s: %s"), *PropertyName.ToString(), *Error);
            return false;
        }
        FKyaLevelEditService& Service = FKyaLevelEditService::Get();
        bool bApplied = false;
        if (GEditor && !GEditor->IsTransactionActive())
        {
            FScopedTransaction Transaction(NSLOCTEXT("KyaLevelEditor", "KyaActorBulkEdit", "Edit Kya Actors"));
            bApplied = Service.ApplyBulk(Keys, Patch, EKyaEditNotify::Notify, &Error);
        }
        else
        {
            bApplied = Service.ApplyBulk(Keys, Patch, EKyaEditNotify::Notify, &Error);
        }
        if (!bApplied)
        {
            InlineError = FString::Printf(TEXT("%s: %s"), *PropertyName.ToString(), *Error);
            return false;
        }
        InlineError.Reset();
        return true;
    };

    // Common-value helpers: an unset value marks a mixed selection.
    const auto ReadVectorComponent = [Usable](FName PropertyName, int32 Component) -> TOptional<double>
    {
        double Common = 0.0;
        bool bHas = false;
        for (AKyaActorHandle* Candidate : Usable)
        {
            FVector Value;
            if (!GetKyaVectorField(Candidate->Bank->SceneEnvironment.Actors[Candidate->ActorIndex], PropertyName, Value))
                return TOptional<double>();
            const double ComponentValue = Component == 0 ? Value.X : Component == 1 ? Value.Y : Value.Z;
            if (!bHas) { Common = ComponentValue; bHas = true; }
            else if (!FMath::IsNearlyEqual(Common, ComponentValue, 1.0e-4)) return TOptional<double>();
        }
        return bHas ? TOptional<double>(Common) : TOptional<double>();
    };
    const auto ReadFloatValue = [Usable](FName PropertyName) -> TOptional<double>
    {
        double Common = 0.0;
        bool bHas = false;
        for (AKyaActorHandle* Candidate : Usable)
        {
            float Value = 0.0f;
            if (!GetKyaFloatField(Candidate->Bank->SceneEnvironment.Actors[Candidate->ActorIndex], PropertyName, Value))
                return TOptional<double>();
            if (!bHas) { Common = Value; bHas = true; }
            else if (!FMath::IsNearlyEqual(Common, double(Value), 1.0e-4)) return TOptional<double>();
        }
        return bHas ? TOptional<double>(Common) : TOptional<double>();
    };
    const auto ReadIntValue = [Usable](FName PropertyName) -> TOptional<int64>
    {
        int64 Common = 0;
        bool bHas = false;
        for (AKyaActorHandle* Candidate : Usable)
        {
            const int64 Value = GetKyaNumericField(Candidate->Bank->SceneEnvironment.Actors[Candidate->ActorIndex],
                PropertyName);
            if (!bHas) { Common = Value; bHas = true; }
            else if (Common != Value) return TOptional<int64>();
        }
        return bHas ? TOptional<int64>(Common) : TOptional<int64>();
    };
    const auto MixedText = [](const TOptional<double>& Value, const TOptional<int64>& IntValue) -> FText
    {
        const bool bMixed = IntValue.IsSet() ? false : !Value.IsSet();
        return bMixed ? FText::FromString(TEXT("Mixed")) : FText::GetEmpty();
    };

    // Bulk apply with a per-record mutator (used where each record must keep its own
    // untouched components, such as editing a single vector axis).
    const auto ApplyBulkMutator = [this, Keys](FName PropertyName,
        TFunction<bool(FKyaActorPlacement&, FString&)> Mutator) -> bool
    {
        FKyaActorEditPatch Patch;
        Patch.Custom = MoveTemp(Mutator);
        FKyaLevelEditService& Service = FKyaLevelEditService::Get();
        FString Error;
        bool bApplied = false;
        if (GEditor && !GEditor->IsTransactionActive())
        {
            FScopedTransaction Transaction(NSLOCTEXT("KyaLevelEditor", "KyaActorBulkEdit", "Edit Kya Actors"));
            bApplied = Service.ApplyBulk(Keys, Patch, EKyaEditNotify::Notify, &Error);
        }
        else
        {
            bApplied = Service.ApplyBulk(Keys, Patch, EKyaEditNotify::Notify, &Error);
        }
        if (!bApplied)
        {
            InlineError = FString::Printf(TEXT("%s: %s"), *PropertyName.ToString(), *Error);
            return false;
        }
        InlineError.Reset();
        return true;
    };

    const auto AddVectorRow = [this, &Category, ReadVectorComponent, ApplyBulkMutator, MixedText](const FString& Label,
        FName PropertyName)
    {
        TSharedRef<SHorizontalBox> Box = SNew(SHorizontalBox);
        for (int32 Component = 0; Component < 3; ++Component)
        {
            const TCHAR* Axis = Component == 0 ? TEXT("X") : Component == 1 ? TEXT("Y") : TEXT("Z");
            Box->AddSlot().FillWidth(1.0f).Padding(2.0f, 0.0f)
            [
                SNew(SNumericEntryBox<double>)
                .Label()
                [
                    SNew(STextBlock).Text(FText::FromString(Axis))
                ]
                .UndeterminedString(FText::FromString(TEXT("Mixed")))
                .MinValue(-1.0e7).MaxValue(1.0e7)
                .Value_Lambda([ReadVectorComponent, PropertyName, Component]() { return ReadVectorComponent(PropertyName, Component); })
                .OnValueCommitted_Lambda([ApplyBulkMutator, PropertyName, Component](double Value, ETextCommit::Type)
                {
                    ApplyBulkMutator(PropertyName, [PropertyName, Component, Value](FKyaActorPlacement& Placement,
                        FString& OutError) -> bool
                    {
                        FVector Vector;
                        if (!GetKyaVectorField(Placement, PropertyName, Vector))
                        {
                            OutError = TEXT("The field is not a vector.");
                            return false;
                        }
                        // Only the edited axis changes; every record keeps its other
                        // components.
                        if (Component == 0) Vector.X = Value;
                        else if (Component == 1) Vector.Y = Value;
                        else Vector.Z = Value;
                        SetKyaVectorField(Placement, PropertyName, Vector);
                        return true;
                    });
                })
            ];
        }
        Box->AddSlot().AutoWidth().VAlign(VAlign_Center)
        [
            SNew(STextBlock).Text_Lambda([ReadVectorComponent, PropertyName, MixedText]()
            {
                const TOptional<double> Value = ReadVectorComponent(PropertyName, 0);
                return MixedText(Value, TOptional<int64>());
            })
        ];
        Category.AddCustomRow(FText::FromString(Label))
            .NameContent()[SNew(STextBlock).Text(FText::FromString(Label))]
            .ValueContent().MinDesiredWidth(360.0f)[Box];
    };

    const auto AddFloatRow = [this, &Category, ReadFloatValue, ApplyBulkField, MixedText](const FString& Label,
        FName PropertyName)
    {
        Category.AddCustomRow(FText::FromString(Label))
            .NameContent()[SNew(STextBlock).Text(FText::FromString(Label))]
            .ValueContent().MinDesiredWidth(360.0f)
            [
                SNew(SHorizontalBox)
                + SHorizontalBox::Slot().FillWidth(1.0f)
                [
                    SNew(SNumericEntryBox<double>)
                    .MinValue(-1.0e7).MaxValue(1.0e7)
                    .Value_Lambda([ReadFloatValue, PropertyName]() { return ReadFloatValue(PropertyName); })
                    .OnValueCommitted_Lambda([ApplyBulkField, PropertyName](double Value, ETextCommit::Type)
                    {
                        ApplyBulkField(PropertyName, [PropertyName, Value](FKyaActorPlacement& Placement)
                        {
                            SetKyaFloatField(Placement, PropertyName, float(Value));
                        });
                    })
                ]
                + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
                [
                    SNew(STextBlock).Text_Lambda([ReadFloatValue, PropertyName, MixedText]()
                    {
                        return MixedText(ReadFloatValue(PropertyName), TOptional<int64>());
                    })
                ]
            ];
    };

    const auto AddIntRow = [this, &Category, ReadIntValue, ApplyBulkField, MixedText](const FString& Label,
        FName PropertyName, bool bAllowNegative)
    {
        Category.AddCustomRow(FText::FromString(Label))
            .NameContent()[SNew(STextBlock).Text(FText::FromString(Label))]
            .ValueContent().MinDesiredWidth(360.0f)
            [
                SNew(SHorizontalBox)
                + SHorizontalBox::Slot().FillWidth(1.0f)
                [
                    SNew(SNumericEntryBox<int64>)
                    .MinValue(bAllowNegative ? -1 : 0)
                    .MaxValue(TNumericLimits<int64>::Max())
                    .Value_Lambda([ReadIntValue, PropertyName]() { return ReadIntValue(PropertyName); })
                    .OnValueCommitted_Lambda([ApplyBulkField, PropertyName](int64 Value, ETextCommit::Type)
                    {
                        ApplyBulkField(PropertyName, [PropertyName, Value](FKyaActorPlacement& Placement)
                        {
                            SetKyaNumericField(Placement, PropertyName, Value);
                        });
                    })
                ]
                + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
                [
                    SNew(STextBlock).Text_Lambda([ReadIntValue, PropertyName, MixedText]()
                    {
                        return MixedText(TOptional<double>(), ReadIntValue(PropertyName));
                    })
                ]
            ];
    };

    AddVectorRow(TEXT("Position (metres)"), GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, Position));
    AddVectorRow(TEXT("Rotation (radians)"), GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, Rotation));
    AddVectorRow(TEXT("Scale"), GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, Scale));
    AddIntRow(TEXT("Sector id"), GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, SectorId), true);
    AddFloatRow(TEXT("Visibility distance"), GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, VisibilityDistance));
    AddFloatRow(TEXT("Culling distance"), GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, CullingDistance));
    AddFloatRow(TEXT("Lighting value"), GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, LightingValue));
    AddIntRow(TEXT("Actor flags"), GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, ActorFlags), false);
    AddIntRow(TEXT("Lighting flags"), GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, LightingFlags), false);
    if (bCompatible)
    {
        AddIntRow(TEXT("Default behaviour"), GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, DefaultBehaviourId), true);
    }

    // Reset the whole selection to its imported baseline in one transaction.
    Category.AddCustomRow(FText::FromString(TEXT("Reset selection")))
        .NameContent()[SNew(STextBlock).Text(FText::FromString(TEXT("Reset selection")))]
        .ValueContent()
        [
            SNew(SButton)
            .Text(FText::FromString(TEXT("Reset all to imported")))
            .ToolTipText(FText::FromString(TEXT("Restore every selected actor from its imported baseline in one undo step.")))
            .OnClicked_Lambda([this, Keys]()
            {
                FKyaLevelEditService& Service = FKyaLevelEditService::Get();
                FScopedTransaction Transaction(NSLOCTEXT("KyaLevelEditor", "KyaActorBulkReset", "Reset Kya Actors"));
                FString FirstError;
                for (const FKyaActorRecordKey& Key : Keys)
                {
                    UKyaLevelBank* Bank = Key.Bank.Get();
                    FKyaActorEditPatch Patch;
                    if (!Bank || !BuildKyaActorResetPatch(*Bank, Key.RecordIndex, Patch))
                    {
                        FirstError = TEXT("No imported baseline is available for this bank.");
                        break;
                    }
                    FString Error;
                    if (!Service.Apply(Key, Patch, EKyaEditNotify::Silent, &Error))
                    {
                        FirstError = Error;
                        break;
                    }
                }
                if (!FirstError.IsEmpty()) InlineError = FirstError;
                else InlineError.Reset();
                Service.FlushPendingNotifications();
                return FReply::Handled();
            })
        ];

    Category.AddCustomRow(FText::FromString(TEXT("Validation")))
        .NameContent()[SNew(STextBlock).Text(FText::FromString(TEXT("Validation")))]
        .ValueContent().MinDesiredWidth(320.0f)
        [
            SAssignNew(InlineErrorText, STextBlock)
            .Text(this, &FKyaActorDetailsCustomization::GetInlineErrorText)
            .AutoWrapText(true)
            .ColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.65f, 0.2f, 1.0f)))
        ];
}

void FKyaActorDetailsCustomization::RefreshScopeFromBank()
{
    FKyaActorPlacement* Scope = ScopeRecord();
    AKyaActorHandle* Actor = Handle.Get();
    if (!Scope || !Actor || !Actor->Bank ||
        !Actor->Bank->SceneEnvironment.Actors.IsValidIndex(Actor->ActorIndex)) return;
    *Scope = Actor->Bank->SceneEnvironment.Actors[Actor->ActorIndex];
}

bool FKyaActorDetailsCustomization::BuildPatchForProperty(FName PropertyName, FKyaActorEditPatch& OutPatch)
{
    FKyaActorPlacement* Scope = ScopeRecord();
    if (!Scope) return false;
    FString Error;
    if (!KyaMakePlacementPropertyPatch(*Scope, PropertyName, OutPatch, &Error))
    {
        InlineError = FString::Printf(TEXT("%s: %s"), *PropertyName.ToString(), *Error);
        UE_LOG(LogTemp, Warning, TEXT("KYA_DETAILS_FIELD_UNSUPPORTED field=%s reason=%s"),
            *PropertyName.ToString(), *Error);
        return false;
    }
    return true;
}

void FKyaActorDetailsCustomization::RequestScopeRefresh(bool bRebuildPanel)
{
    const TWeakObjectPtr<AKyaActorHandle> WeakHandle = Handle;
    const TSharedPtr<FStructOnScope> Scope = PlacementScope;
    const TWeakPtr<IPropertyUtilities> Utilities = PropertyUtilities;
    if (!Scope.IsValid()) return;
    const auto ReloadScope = [WeakHandle, Scope]()
    {
        AKyaActorHandle* Actor = WeakHandle.Get();
        if (!Actor || !Actor->Bank || !Actor->Bank->SceneEnvironment.Actors.IsValidIndex(Actor->ActorIndex)) return;
        *reinterpret_cast<FKyaActorPlacement*>(Scope->GetStructMemory()) =
            Actor->Bank->SceneEnvironment.Actors[Actor->ActorIndex];
    };
    if (!Utilities.IsValid())
    {
        ReloadScope();
        return;
    }
    // Deferred: a synchronous refresh can rebuild this customization while a
    // property notification is still executing.
    Utilities.Pin()->EnqueueDeferredAction(FSimpleDelegate::CreateLambda([ReloadScope, Utilities, bRebuildPanel]()
    {
        ReloadScope();
        if (bRebuildPanel && Utilities.IsValid()) Utilities.Pin()->RequestForceRefresh();
    }));
}

void FKyaActorDetailsCustomization::OnPlacementPropertyChanged(FName PropertyName)
{
    AKyaActorHandle* Actor = Handle.Get();
    if (bApplying || !Actor || !Actor->Bank) return;
    FKyaActorEditPatch Patch;
    if (!BuildPatchForProperty(PropertyName, Patch))
    {
        RefreshScopeFromBank();
        return;
    }

    FKyaLevelEditService& Service = FKyaLevelEditService::Get();
    FString Error;
    TGuardValue<bool> Guard(bApplying, true);
    bool bApplied = false;
    if (GEditor && !GEditor->IsTransactionActive())
    {
        // External struct edits are not part of a details transaction, so open one to
        // keep the bank write undoable as a single step.
        FScopedTransaction Transaction(NSLOCTEXT("KyaLevelEditor", "KyaActorDetailsEdit", "Edit Kya Actor Property"));
        bApplied = Service.Apply(Service.MakeKey(Actor->Bank, Actor->ActorIndex), Patch, EKyaEditNotify::Notify, &Error);
    }
    else
    {
        bApplied = Service.Apply(Service.MakeKey(Actor->Bank, Actor->ActorIndex), Patch, EKyaEditNotify::Notify, &Error);
    }
    if (!bApplied)
    {
        InlineError = FString::Printf(TEXT("%s: %s"), *PropertyName.ToString(), *Error);
        UE_LOG(LogTemp, Warning, TEXT("KYA_DETAILS_WRITE_FAILED bank=%s index=%d field=%s reason=%s"),
            *GetNameSafe(Actor->Bank), Actor->ActorIndex, *PropertyName.ToString(), *Error);
        RefreshScopeFromBank();
    }
    else
    {
        InlineError.Reset();
    }
}

void FKyaActorDetailsCustomization::OnBankEdited(UKyaLevelBank* Bank)
{
    AKyaActorHandle* Actor = Handle.Get();
    if (bApplying || !Actor || Actor->Bank != Bank) return;
    // A reload can change array sizes and presence flags, so rebuild the panel.
    RequestScopeRefresh(true);
}

void FKyaActorDetailsCustomization::OnObjectTransacted(UObject* Object, const FTransactionObjectEvent& Event)
{
    AKyaActorHandle* Actor = Handle.Get();
    if (!Actor || !Actor->Bank || Object != Actor->Bank) return;
    RequestScopeRefresh(false);
}
