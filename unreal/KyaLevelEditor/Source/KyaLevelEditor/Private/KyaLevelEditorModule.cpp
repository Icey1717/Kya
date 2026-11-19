#include "Modules/ModuleManager.h"
#include "KyaDisc.h"
#include "KyaDiscDetails.h"
#include "EngineUtils.h"
#include "Editor.h"
#include "UnrealEdGlobals.h"
#include "Editor/UnrealEdEngine.h"
#include "KyaPrimitiveVisualizer.h"
#include "HAL/IConsoleManager.h"
#include "LevelEditorViewport.h"
#include "KyaActorEditService.h"
#include "KyaActorDetailsCustomization.h"
#include "KyaActorHandle.h"
#include "KyaHierarchyDetailsCustomization.h"
#include "KyaGeometryDetailsCustomization.h"
#include "KyaGeometryWorkingActor.h"
#include "KyaHierarchyHandle.h"
#include "KyaSceneVisualizer.h"
#include "KyaSceneVisualizerDetailsCustomization.h"
#include "KyaLevelSettings.h"
#include "KyaLevelSettingsDetailsCustomization.h"
#include "KyaSpatialRecordDetailsCustomization.h"
#include "KyaSpatialRecordHandle.h"
#include "KyaLevelActorFactory.h"
#include "KyaLevelEditorActions.h"
#include "KyaExportDialog.h"
#include "KyaWorkflowDialog.h"
#include "Logging/MessageLog.h"
#include "MessageLogModule.h"
#include "MessageLogInitializationOptions.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "KyaLevelBankReimportHandler.h"
#include "KyaLevelBank.h"
#include "KyaLevelPreview.h"
#include "KyaLevelPreviewDetailsCustomization.h"
#include "KyaLevelSet.h"
#include "KyaLevelSettings.h"
#include "Misc/App.h"
#include "Misc/MessageDialog.h"
#include "Misc/Paths.h"
#include "PropertyEditorModule.h"
#include "KyaScenarioConditionCustomization.h"
#include "KyaActorBehaviourCustomization.h"
#include "IDetailsView.h"
#include "LevelEditor.h"
#include "Framework/Docking/TabManager.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "Textures/SlateIcon.h"
#include "ToolMenuOwner.h"
#include "ToolMenus.h"
#include "EditorReimportHandler.h"

namespace
{
void KyaShowSummary(const FString& Title, const FString& Message)
{
    UE_LOG(LogTemp, Display, TEXT("KYA_%s %s"), *Title, *Message);
    if (!FApp::IsUnattended())
        FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(Message), FText::FromString(Title));
}

void KyaNotify(const FString& Title, const FString& Message)
{
    UE_LOG(LogTemp, Display, TEXT("KYA_%s %s"), *Title, *Message);
    if (FApp::IsUnattended()) return;
    FNotificationInfo Info(FText::FromString(Title + TEXT(": ") + Message));
    Info.ExpireDuration = 4.0f;
    FSlateNotificationManager::Get().AddNotification(Info);
}

bool KyaHasTargetBanks() { return !KyaCollectLevelBanks(KyaGetTargetPreviews()).IsEmpty(); }
bool KyaHasTargetPreviews() { return !KyaGetTargetPreviews().IsEmpty(); }
bool KyaAllPreviews(TFunctionRef<bool(const AKyaLevelPreview&)> Predicate)
{
    const TArray<AKyaLevelPreview*> Previews = KyaGetTargetPreviews();
    return !Previews.IsEmpty() && !Previews.ContainsByPredicate([&](const AKyaLevelPreview* Preview)
        { return !Preview || !Predicate(*Preview); });
}

TWeakObjectPtr<UKyaLevelSettings> GLevelSettingsTarget;
TSharedPtr<IDetailsView> GLevelSettingsView;
bool GLevelSettingsSpawnerRegistered = false;

TSharedRef<SDockTab> SpawnLevelSettingsTab(const FSpawnTabArgs&)
{
    UKyaLevelSettings* Target = GLevelSettingsTarget.Get();
    if (!Target)
    {
        const TArray<AKyaLevelPreview*> Previews = KyaGetTargetPreviews();
        if (!Previews.IsEmpty()) Target = Previews[0]->GetOrCreateLevelSettings();
        GLevelSettingsTarget = Target;
    }
    if (!GLevelSettingsView.IsValid())
    {
        FPropertyEditorModule& PropertyModule =
            FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
        FDetailsViewArgs Args;
        Args.bHideSelectionTip = true;
        Args.NameAreaSettings = FDetailsViewArgs::HideNameArea;
        GLevelSettingsView = PropertyModule.CreateDetailView(Args);
    }
    if (Target) GLevelSettingsView->SetObject(Target);
    else GLevelSettingsView->SetObject(nullptr);
    return SNew(SDockTab)
        .TabRole(ETabRole::NomadTab)
        [
            SNew(SVerticalBox)
            + SVerticalBox::Slot().FillHeight(1.0f)
            [
                GLevelSettingsView.ToSharedRef()
            ]
            + SVerticalBox::Slot().AutoHeight()
            [
                SNew(SBorder).Padding(8.0f)
                [
                    SNew(SVerticalBox)
                    + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 6.0f)
                    [
                        SNew(STextBlock)
                        .Text_Lambda([]()
                        {
                            const UKyaLevelSettings* Settings = GLevelSettingsTarget.Get();
                            return Settings ? FText::FromString(Settings->ApplyStatus) : FText::GetEmpty();
                        })
                        .AutoWrapText(true)
                    ]
                    + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Right)
                    [
                        SNew(SHorizontalBox)
                        + SHorizontalBox::Slot().AutoWidth().Padding(0.0f, 0.0f, 8.0f, 0.0f)
                        [
                            SNew(SButton)
                            .Text(FText::FromString(TEXT("Reload from Bank")))
                            .ToolTipText(FText::FromString(TEXT("Discard the draft and load the bank's current settings.")))
                            .IsEnabled_Lambda([]() { return GLevelSettingsTarget.IsValid() && GLevelSettingsTarget->Bank != nullptr; })
                            .OnClicked_Lambda([]()
                            {
                                if (UKyaLevelSettings* Settings = GLevelSettingsTarget.Get()) Settings->LoadFromBank();
                                return FReply::Handled();
                            })
                        ]
                        + SHorizontalBox::Slot().AutoWidth()
                        [
                            SNew(SButton)
                            .Text(FText::FromString(TEXT("Save")))
                            .ToolTipText(FText::FromString(TEXT("Validate and write settings to the bank as one undo step. Save the asset to persist to disk.")))
                            .IsEnabled_Lambda([]() { return GLevelSettingsTarget.IsValid() && GLevelSettingsTarget->Bank != nullptr; })
                            .OnClicked_Lambda([]()
                            {
                                if (UKyaLevelSettings* Settings = GLevelSettingsTarget.Get())
                                {
                                    FString Error;
                                    if (!Settings->ApplyToBank(Error))
                                        UE_LOG(LogTemp, Warning, TEXT("KYA_LEVEL_SETTINGS_APPLY_FAILED reason=%s"), *Error);
                                }
                                return FReply::Handled();
                            })
                        ]
                    ]
                ]
            ]
        ];
}

void KyaOpenLevelSettingsCommand(const TArray<FString>& Args)
{
    const TArray<AKyaLevelPreview*> Previews = KyaGetTargetPreviews();
    if (Previews.IsEmpty())
    {
        KyaShowSummary(TEXT("Kya Level Settings"), TEXT("Select a Kya level preview first."));
        return;
    }
    GLevelSettingsTarget = Previews[0]->GetOrCreateLevelSettings();
    // If the tab is already open, switch it to the requested preview's settings.
    if (GLevelSettingsView.IsValid()) GLevelSettingsView->SetObject(GLevelSettingsTarget.Get());
    FGlobalTabmanager::Get()->TryInvokeTab(FTabId("KyaLevelSettings"));
}

void KyaSetAllPathsCommand(const TArray<FString>& Args)
{    const bool bVisible = Args.Num() == 0 || Args[0] != TEXT("0");
    int32 Affected = 0;
    for (AKyaLevelPreview* Preview : KyaGetTargetPreviews())
    {
        if (!Preview) continue;
        Preview->Modify();
        Preview->bShowAllPaths = bVisible;
        Preview->RebuildFromBank();
        ++Affected;
    }
    KyaNotify(TEXT("Kya Paths"), bVisible
        ? FString::Printf(TEXT("Showing every path on %d preview(s) with linear interpolation and selectable points."),
            Affected)
        : FString::Printf(TEXT("All-path display disabled on %d preview(s)."), Affected));
}

void KyaValidateCommand(const TArray<FString>& Args)
{
    const TArray<AKyaLevelPreview*> Previews = KyaGetTargetPreviews();
    const TArray<UKyaLevelBank*> Banks = KyaCollectLevelBanks(Previews);
    if (Banks.IsEmpty())
    {
        KyaShowSummary(TEXT("Validate Kya Level"), TEXT("Select a level preview with a bank first."));
        return;
    }
    FMessageLog("KyaLevelEditor").NewPage(FText::FromString(TEXT("Validation — ") + KyaGetPreviewScopeText()));
    const TArray<FKyaBankValidationReport> Reports = KyaValidateLevelBanks(Banks);
    int32 Failures = 0;
    for (const FKyaBankValidationReport& Report : Reports)
    {
        if (!Report.bBytesBuilt) ++Failures;
    }
    KyaNotify(TEXT("Validate Kya Level"), FString::Printf(TEXT("%d bank(s), %d not exportable. See Kya Level results."), Reports.Num(), Failures));
    if (!FApp::IsUnattended()) FMessageLog("KyaLevelEditor").Open(EMessageSeverity::Info, true);
}

void KyaExportCommand(const TArray<FString>& Args)
{
    const TArray<AKyaLevelPreview*> Previews = KyaGetTargetPreviews();
    const TArray<UKyaLevelBank*> Banks = KyaCollectLevelBanks(Previews);
    if (Banks.IsEmpty())
    {
        KyaShowSummary(TEXT("Export Kya Level"), TEXT("Select a level preview with a bank first."));
        return;
    }
    FString Directory = Args.IsEmpty() ? FString() : Args[0];
    if (Directory.IsEmpty())
        Directory = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("KyaExport"));
    if (!FApp::IsUnattended())
    {
        KyaShowExportDialog(Banks, FPaths::ConvertRelativePathToFull(Directory));
        return;
    }
    TArray<FString> Messages;
    const bool bAll = KyaExportLevelBanks(Banks, Directory, Messages);
    KyaShowSummary(TEXT("Export Kya Level"), FString::Printf(TEXT("Output: %s\n%s"), *Directory,
        *FString::Join(Messages, TEXT("\n"))));
    if (!bAll) UE_LOG(LogTemp, Warning, TEXT("KYA_EXPORT some banks were not exported."));
}

void KyaDiscoverCommand(const TArray<FString>& Args)
{
    const bool bImportMissing = Args.IsEmpty() || Args[0] != TEXT("0");
    const TArray<AKyaLevelPreview*> Previews = KyaGetTargetPreviews();
    if (Previews.IsEmpty())
    {
        KyaShowSummary(TEXT("Refresh Sector Discovery"), TEXT("Select a level preview first."));
        return;
    }
    if (!FApp::IsUnattended())
    {
        KyaShowDiscoveryDialog(Previews, bImportMissing);
        return;
    }
    TArray<FString> Messages;
    for (AKyaLevelPreview* Preview : Previews)
        Messages.Add(FString::Printf(TEXT("%s:\n%s"), *GetNameSafe(Preview),
            *KyaDiscoverSectors(Preview, bImportMissing).Summary()));
    KyaShowSummary(TEXT("Refresh Sector Discovery"), FString::Join(Messages, TEXT("\n\n")));
}

void KyaCreateLevelSetCommand(const TArray<FString>& Args)
{
    const TArray<AKyaLevelPreview*> Previews = KyaGetTargetPreviews();
    if (Previews.IsEmpty())
    {
        KyaShowSummary(TEXT("Create Kya Level Set"), TEXT("Select a level preview first."));
        return;
    }
    if (!FApp::IsUnattended())
    {
        KyaShowLevelSetDialog(Previews);
        return;
    }
    TArray<FString> Messages;
    for (AKyaLevelPreview* Preview : Previews)
    {
        FString Error;
        if (const UKyaLevelSet* LevelSet = KyaCreateLevelSetFromPreview(Preview, Error))
            Messages.Add(FString::Printf(TEXT("%s: %s (%d sector associations)"), *GetNameSafe(Preview),
                *LevelSet->GetPathName(), LevelSet->Sectors.Num()));
        else
            Messages.Add(FString::Printf(TEXT("%s: %s"), *GetNameSafe(Preview), *Error));
    }
    KyaShowSummary(TEXT("Create Kya Level Set"), FString::Join(Messages, TEXT("\n")));
}

void KyaSaveCommand(const TArray<FString>& Args){
    const TArray<UKyaLevelBank*> Banks = KyaCollectLevelBanks(KyaGetTargetPreviews());
    FString Error;
    if (KyaSaveLevelBanks(Banks, Error))
        KyaNotify(TEXT("Save Kya Level Assets"), FString::Printf(TEXT("Saved Unreal assets for %d bank(s). Use Export Kya Level to write game banks."), Banks.Num()));
    else
        KyaShowSummary(TEXT("Save Kya Level"), Error);
}

void KyaDuplicateActorCommand(const TArray<FString>& Args)
{
    const FKyaStructuralActionResult Result = KyaDuplicateSelectedActor();
    (Result.bSuccess ? KyaNotify : KyaShowSummary)(TEXT("Duplicate Kya Actor"), Result.bSuccess
        ? FString::Printf(TEXT("Duplicated the actor record at index %d."), Result.ActorIndex)
        : Result.Error);
}

void KyaCreateActorCommand(const TArray<FString>& Args)
{
    const FKyaStructuralActionResult Result = KyaCreateSelectedActor();
    (Result.bSuccess ? KyaNotify : KyaShowSummary)(TEXT("Create from Selected Template"), Result.bSuccess
        ? FString::Printf(TEXT("Created actor record at index %d from the selected template."), Result.ActorIndex)
        : Result.Error);
}

void KyaDeleteActorCommand(const TArray<FString>& Args)
{
    const FKyaStructuralActionResult Result = KyaDeleteSelectedActor();
    (Result.bSuccess ? KyaNotify : KyaShowSummary)(TEXT("Delete Kya Actor"), Result.bSuccess
        ? FString::Printf(TEXT("Deleted the actor record at index %d and remapped typed references."), Result.ActorIndex)
        : Result.Error);
}

void AddKyaMenuEntries(FToolMenuSection& Section)
{
    Section.AddMenuEntry("KyaTargetScope", FText::FromString(TEXT("Scope: ") + KyaGetPreviewScopeText()),
        FText::FromString(TEXT("Level actions use selected previews; with none selected, they use all previews in the editor world. Actor actions use the selected generated actor.")),
        FSlateIcon(), FUIAction(FExecuteAction(), FCanExecuteAction::CreateLambda([]() { return false; })));
    Section.AddMenuEntry("KyaValidateLevel",
        FText::FromString(TEXT("Validate Kya Level")),
        FText::FromString(TEXT("Validate every bank used by the selected level preview.")),
        FSlateIcon(),
        FUIAction(FExecuteAction::CreateLambda([]() { KyaValidateCommand({}); }), FCanExecuteAction::CreateStatic(&KyaHasTargetBanks)));
    Section.AddMenuEntry("KyaExportLevel",
        FText::FromString(TEXT("Export Kya Level")),
        FText::FromString(TEXT("Choose an output folder, validate and export the selected level's banks.")),
        FSlateIcon(),
        FUIAction(FExecuteAction::CreateLambda([]() { KyaExportCommand({}); }), FCanExecuteAction::CreateStatic(&KyaHasTargetBanks)));
    Section.AddMenuEntry("KyaDiscoverSectors",
        FText::FromString(TEXT("Refresh Sector Discovery")),
        FText::FromString(TEXT("Find sibling SECT bank files next to the level's main bank.")),
        FSlateIcon(),
        FUIAction(FExecuteAction::CreateLambda([]() { KyaDiscoverCommand({}); }), FCanExecuteAction::CreateStatic(&KyaHasTargetBanks)));
    Section.AddMenuEntry("KyaCreateLevelSet",
        FText::FromString(TEXT("Create / Update Kya Level Set")),
        FText::FromString(TEXT("Create or update the persisted level-set asset for the selected preview.")),
        FSlateIcon(),
        FUIAction(FExecuteAction::CreateLambda([]() { KyaCreateLevelSetCommand({}); }), FCanExecuteAction::CreateStatic(&KyaHasTargetBanks)));
    Section.AddMenuEntry("KyaLevelSettings",
        FText::FromString(TEXT("Edit Level Settings")),
        FText::FromString(TEXT("Open environment, camera, fog and scene configuration authoring for the selected level.")),
        FSlateIcon(),
        FUIAction(FExecuteAction::CreateLambda([]() { KyaOpenLevelSettingsCommand({}); }), FCanExecuteAction::CreateStatic(&KyaHasTargetBanks)));
    Section.AddMenuEntry("KyaSaveLevel",
        FText::FromString(TEXT("Save Kya Level Assets")),
        FText::FromString(TEXT("Save Unreal bank assets. Select a preview with a bank; use Export Kya Level to write game .bnk files.")),
        FSlateIcon(),
        FUIAction(FExecuteAction::CreateLambda([]() { KyaSaveCommand({}); }), FCanExecuteAction::CreateStatic(&KyaHasTargetBanks)));
    Section.AddSeparator("KyaStructuralSeparator");
    Section.AddMenuEntry("KyaCreateActorRecord",
        FText::FromString(TEXT("Create from Selected Template")),
        FText::FromString(TEXT("Select exactly one owned Kya actor from a bank that supports structural edits. Create a new record using its class and layout.")),
        FSlateIcon(),
        FUIAction(FExecuteAction::CreateLambda([]() { KyaCreateActorCommand({}); }), FCanExecuteAction::CreateStatic(&KyaCanEditSelectedActor)));
    Section.AddMenuEntry("KyaDuplicateActorRecord",
        FText::FromString(TEXT("Duplicate Selected Actor")),
        FText::FromString(TEXT("Select exactly one owned Kya actor from a bank that supports structural edits. Copy its bank record with a new identity.")),
        FSlateIcon(),
        FUIAction(FExecuteAction::CreateLambda([]() { KyaDuplicateActorCommand({}); }), FCanExecuteAction::CreateStatic(&KyaCanEditSelectedActor)));
    Section.AddMenuEntry("KyaDeleteActorRecord",
        FText::FromString(TEXT("Delete Kya Actor")),
        FText::FromString(TEXT("Select exactly one owned Kya actor from a bank that supports structural edits. Delete its bank record and remap typed actor references.")),
        FSlateIcon(),
        FUIAction(FExecuteAction::CreateLambda([]() { KyaDeleteActorCommand({}); }), FCanExecuteAction::CreateStatic(&KyaCanEditSelectedActor)));
    Section.AddMenuEntry("KyaHideGenerated",
        FText::FromString(TEXT("Hide Selected Generated Actors")),
        FText::FromString(TEXT("Temporarily hide the selected Kya representations in the editor only.")),
        FSlateIcon(),
        FUIAction(FExecuteAction::CreateLambda([]()
        {
            int32 Affected = 0;
            KyaSetSelectedGeneratedActorsHidden(true, Affected);
            KyaNotify(TEXT("Kya Hide"), FString::Printf(
                TEXT("%d generated actor(s) hidden in the editor. Game visibility and bank data are unchanged."),
                Affected));
        }), FCanExecuteAction::CreateStatic(&KyaSelectionContainsGeneratedActor)));
    Section.AddMenuEntry("KyaShowGenerated",
        FText::FromString(TEXT("Show Selected Generated Actors")),
        FText::FromString(TEXT("Reveal the selected Kya representations (isolation must be cleared).")),
        FSlateIcon(),
        FUIAction(FExecuteAction::CreateLambda([]()
        {
            int32 Affected = 0;
            KyaSetSelectedGeneratedActorsHidden(false, Affected);
            KyaNotify(TEXT("Kya Show"), FString::Printf(TEXT("%d generated actor(s) shown."), Affected));
        }), FCanExecuteAction::CreateStatic(&KyaSelectionContainsGeneratedActor)));
    Section.AddMenuEntry("KyaIsolateGenerated",
        FText::FromString(TEXT("Isolate Selected Generated Actors")),
        FText::FromString(TEXT("Hide every other generated representation until isolation is cleared.")),
        FSlateIcon(),
        FUIAction(FExecuteAction::CreateLambda([]()
        {
            int32 Affected = 0;
            KyaIsolateSelectedGeneratedActors(Affected);
            KyaNotify(TEXT("Kya Isolate"), Affected > 0
                ? FString::Printf(TEXT("Isolated %d generated actor(s). Use Clear Generated Isolation to restore."),
                    Affected)
                : FString(TEXT("Select at least one generated actor to isolate it.")));
        }), FCanExecuteAction::CreateStatic(&KyaSelectionContainsGeneratedActor),
            FIsActionChecked::CreateLambda([]() { return KyaAllPreviews([](const AKyaLevelPreview& Preview) { return Preview.HasGeneratedActorIsolation(); }); })),
        EUserInterfaceActionType::Check);
    Section.AddMenuEntry("KyaClearIsolation",
        FText::FromString(TEXT("Clear Generated Isolation")),
        FText::FromString(TEXT("Clear temporary hiding and isolation on every Kya level preview.")),
        FSlateIcon(),
        FUIAction(FExecuteAction::CreateLambda([]()
        {
            KyaClearGeneratedActorIsolation();
            KyaNotify(TEXT("Kya Isolation"), TEXT("All generated representations are visible again."));
        })));
    Section.AddMenuEntry("KyaShowAllPaths",
        FText::FromString(TEXT("Show All Paths")),
        FText::FromString(TEXT("Draw every path (linear) and spawn selectable points without setting a path index.")),
        FSlateIcon(),
        FUIAction(FExecuteAction::CreateLambda([]() { KyaSetAllPathsCommand({ TEXT("1") }); }), FCanExecuteAction::CreateStatic(&KyaHasTargetPreviews),
            FIsActionChecked::CreateLambda([]() { return KyaAllPreviews([](const AKyaLevelPreview& Preview) { return Preview.bShowAllPaths; }); })), EUserInterfaceActionType::Check);
    Section.AddMenuEntry("KyaHideAllPaths",
        FText::FromString(TEXT("Hide All Paths")),
        FText::FromString(TEXT("Stop drawing every path; the Empty Preview Path Index guide still applies.")),
        FSlateIcon(),
        FUIAction(FExecuteAction::CreateLambda([]() { KyaSetAllPathsCommand({ TEXT("0") }); }), FCanExecuteAction::CreateStatic(&KyaHasTargetPreviews)));
    Section.AddMenuEntry("KyaSelectAtCursor",
        FText::FromString(TEXT("Select Actor Under Cursor")),
        FText::FromString(TEXT("Pick the actor whose rendered mesh is under the level viewport cursor and highlight its whole mesh.")),
        FSlateIcon(),
        FUIAction(FExecuteAction::CreateLambda([]()
        {
            FString Error;
            if (!KyaSelectRenderedActorAtCursor(&Error))
                UE_LOG(LogTemp, Display, TEXT("KYA_SELECT_AT_CURSOR %s"), *Error);
        })));
    Section.AddMenuEntry("KyaAuthoringVisibility",
        FText::FromString(TEXT("Enable Authoring Visibility")),
        FText::FromString(TEXT("Keep actors from unloaded sectors and distance-culled scenery visible for authoring. "
            "A locked comparison/capture camera still uses native culling.")),
        FSlateIcon(),
        FUIAction(FExecuteAction::CreateLambda([]() { KyaSetAuthoringVisibility(true); }), FCanExecuteAction::CreateStatic(&KyaHasTargetPreviews),
            FIsActionChecked::CreateLambda([]() { return KyaAllPreviews([](const AKyaLevelPreview& Preview) { return Preview.IsAuthoringVisibilityActive(); }); })), EUserInterfaceActionType::RadioButton);
    Section.AddMenuEntry("KyaEditorCamera",
        FText::FromString(TEXT("Return to Editor Camera")),
        FText::FromString(TEXT("Clear the comparison/capture camera lock and resume authoring discovery.")),
        FSlateIcon(),
        FUIAction(FExecuteAction::CreateLambda([]() { KyaClearPreviewCamera(); }), FCanExecuteAction::CreateStatic(&KyaHasTargetPreviews)));
    Section.AddMenuEntry("KyaNativeVisibility",
        FText::FromString(TEXT("Restore Native Visibility")),
        FText::FromString(TEXT("Use game-like distance and sector culling (comparison/capture behavior).")),
        FSlateIcon(),
        FUIAction(FExecuteAction::CreateLambda([]() { KyaSetAuthoringVisibility(false); }), FCanExecuteAction::CreateStatic(&KyaHasTargetPreviews),
            FIsActionChecked::CreateLambda([]() { return KyaAllPreviews([](const AKyaLevelPreview& Preview) { return !Preview.IsAuthoringVisibilityActive(); }); })), EUserInterfaceActionType::RadioButton);
    Section.AddMenuEntry("KyaLockGenerated",
        FText::FromString(TEXT("Toggle Generated Actor Selection Lock")),
        FText::FromString(TEXT("Prevent viewport picking of generated actors (they stay visible and editable). Checked when every target preview is locked; click to lock all or unlock all.")),
        FSlateIcon(),
        FUIAction(FExecuteAction::CreateLambda([]()
        {
            const bool bLocked = KyaAllPreviews([](const AKyaLevelPreview& Preview) { return Preview.bLockGeneratedActorSelection; });
            KyaSetGeneratedActorSelectionLocked(!bLocked);
            KyaNotify(TEXT("Kya Selection Lock"), !bLocked
                ? TEXT("Generated actors are selection-locked; they remain visible and editable through the bank.")
                : TEXT("Generated actors can be picked in the viewport again."));
        }), FCanExecuteAction::CreateStatic(&KyaHasTargetPreviews),
            FIsActionChecked::CreateLambda([]() { return KyaAllPreviews([](const AKyaLevelPreview& Preview) { return Preview.bLockGeneratedActorSelection; }); })), EUserInterfaceActionType::ToggleButton);
}
}

class FKyaLevelEditorModule : public IModuleInterface
{
public:
    virtual void StartupModule() override
    {
        DiscMapOpenedHandle = FEditorDelegates::OnMapOpened.AddLambda([](const FString&, bool)
        {
            UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
            if (!World) return;
            for (TActorIterator<AKyaLevelPreview> It(World); It; ++It)
            {
                UKyaLevelBank* Bank = It->LevelSet ? It->LevelSet->MainBank.Get() : It->Bank.Get();
                if (Bank && !Bank->SourceDisc.IsNull()) It->RebuildFromBank();
            }
        });
        if (GUnrealEd)
        {
            TSharedPtr<FComponentVisualizer> Visualizer = MakeShared<FKyaPrimitiveVisualizer>();
            GUnrealEd->RegisterComponentVisualizer(UKyaPrimitiveComponent::StaticClass()->GetFName(), Visualizer);
            Visualizer->OnRegister();
        }
        FMessageLogInitializationOptions LogOptions;
        LogOptions.bShowFilters = true;
        LogOptions.bShowPages = true;
        FModuleManager::LoadModuleChecked<FMessageLogModule>("MessageLog").RegisterLogListing(
            "KyaLevelEditor", NSLOCTEXT("KyaLevelEditor", "Results", "Kya Level"), LogOptions);
        ReimportHandler = MakeUnique<FKyaLevelBankReimportHandler>();
        FReimportManager::Instance()->RegisterHandler(*ReimportHandler);

        // Actor factories auto-register from UClass discovery at editor startup; when
        // the plugin is loaded later, add the factory explicitly.
        if (GEditor && !GEditor->ActorFactories.ContainsByPredicate([](UActorFactory* Factory)
        {
            return Factory && Factory->IsA<UKyaLevelPreviewFactory>();
        }))
        {
            GEditor->ActorFactories.Add(NewObject<UKyaLevelPreviewFactory>(GetTransientPackage()));
        }

        FPropertyEditorModule& PropertyModule =
            FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
        PropertyModule.RegisterCustomPropertyTypeLayout("KyaScenarioCondition",
            FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FKyaScenarioConditionCustomization::MakeInstance));
        PropertyModule.RegisterCustomPropertyTypeLayout("KyaActorBehaviour",
            FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FKyaActorBehaviourCustomization::MakeInstance));
        PropertyModule.RegisterCustomClassLayout(UKyaDisc::StaticClass()->GetFName(),
            FOnGetDetailCustomizationInstance::CreateStatic(&FKyaDiscDetails::MakeInstance));
        PropertyModule.RegisterCustomClassLayout(AKyaActorHandle::StaticClass()->GetFName(),
            FOnGetDetailCustomizationInstance::CreateStatic(&FKyaActorDetailsCustomization::MakeInstance));
        PropertyModule.RegisterCustomClassLayout(AKyaLevelPreview::StaticClass()->GetFName(),
            FOnGetDetailCustomizationInstance::CreateStatic(&FKyaLevelPreviewDetailsCustomization::MakeInstance));
        PropertyModule.RegisterCustomClassLayout(AKyaSpatialRecordHandle::StaticClass()->GetFName(),
            FOnGetDetailCustomizationInstance::CreateStatic(&FKyaSpatialRecordDetailsCustomization::MakeInstance));
        PropertyModule.RegisterCustomClassLayout(AKyaHierarchyHandle::StaticClass()->GetFName(),
            FOnGetDetailCustomizationInstance::CreateStatic(&FKyaHierarchyDetailsCustomization::MakeInstance));
        PropertyModule.RegisterCustomClassLayout(AKyaGeometryWorkingActor::StaticClass()->GetFName(),
            FOnGetDetailCustomizationInstance::CreateStatic(&FKyaGeometryDetailsCustomization::MakeInstance));
        PropertyModule.RegisterCustomClassLayout(AKyaSceneVisualizer::StaticClass()->GetFName(),
            FOnGetDetailCustomizationInstance::CreateStatic(&FKyaSceneVisualizerDetailsCustomization::MakeInstance));
        PropertyModule.RegisterCustomClassLayout(UKyaLevelSettings::StaticClass()->GetFName(),
            FOnGetDetailCustomizationInstance::CreateStatic(&FKyaLevelSettingsDetailsCustomization::MakeInstance));

        UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(
            this, &FKyaLevelEditorModule::RegisterMenus));

        if (!GLevelSettingsSpawnerRegistered)
        {
            FGlobalTabmanager::Get()->RegisterNomadTabSpawner("KyaLevelSettings",
                FOnSpawnTab::CreateStatic(&SpawnLevelSettingsTab))
                .SetDisplayName(NSLOCTEXT("KyaLevelEditor", "LevelSettingsTab", "Kya Level Settings"))
                .SetTooltipText(NSLOCTEXT("KyaLevelEditor", "LevelSettingsTabTip",
                    "Environment, camera, fog and scene configuration authoring for the selected level."))
                .SetMenuType(ETabSpawnerMenuType::Hidden);
            GLevelSettingsSpawnerRegistered = true;
        }

        // Structural commands are not cancelable per actor; block delete through
        // AActor::CanDeleteSelectedActor and remove generated copies after duplicate,
        // cut and paste so map and bank state cannot diverge.
        StructuralBeginHandles.Add(FEditorDelegates::OnDuplicateActorsBegin.AddRaw(
            this, &FKyaLevelEditorModule::OnStructuralCommandBegin));
        StructuralBeginHandles.Add(FEditorDelegates::OnEditCutActorsBegin.AddRaw(
            this, &FKyaLevelEditorModule::OnStructuralCommandBegin));
        StructuralBeginHandles.Add(FEditorDelegates::OnEditPasteActorsBegin.AddRaw(
            this, &FKyaLevelEditorModule::OnStructuralCommandBegin));
        StructuralEndHandles.Add(FEditorDelegates::OnDuplicateActorsEnd.AddRaw(
            this, &FKyaLevelEditorModule::OnStructuralCommandEnd));
        StructuralEndHandles.Add(FEditorDelegates::OnEditCutActorsEnd.AddRaw(
            this, &FKyaLevelEditorModule::OnStructuralCommandEnd));
        StructuralEndHandles.Add(FEditorDelegates::OnEditPasteActorsEnd.AddRaw(
            this, &FKyaLevelEditorModule::OnStructuralCommandEnd));
    }

    virtual void ShutdownModule() override
    {
        FEditorDelegates::OnMapOpened.Remove(DiscMapOpenedHandle);
        if (GUnrealEd) GUnrealEd->UnregisterComponentVisualizer(UKyaPrimitiveComponent::StaticClass()->GetFName());
        if (FMessageLogModule* MessageLog = FModuleManager::GetModulePtr<FMessageLogModule>("MessageLog"))
            MessageLog->UnregisterLogListing("KyaLevelEditor");
        for (const FDelegateHandle& Handle : StructuralBeginHandles)
        {
            FEditorDelegates::OnDuplicateActorsBegin.Remove(Handle);
            FEditorDelegates::OnEditCutActorsBegin.Remove(Handle);
            FEditorDelegates::OnEditPasteActorsBegin.Remove(Handle);
        }
        StructuralBeginHandles.Reset();
        for (const FDelegateHandle& Handle : StructuralEndHandles)
        {
            FEditorDelegates::OnDuplicateActorsEnd.Remove(Handle);
            FEditorDelegates::OnEditCutActorsEnd.Remove(Handle);
            FEditorDelegates::OnEditPasteActorsEnd.Remove(Handle);
        }
        StructuralEndHandles.Reset();
        UToolMenus::UnRegisterStartupCallback(this);
        UToolMenus::UnregisterOwner(this);
        if (GLevelSettingsSpawnerRegistered)
        {
            FGlobalTabmanager::Get()->UnregisterNomadTabSpawner("KyaLevelSettings");
            GLevelSettingsSpawnerRegistered = false;
        }
        GLevelSettingsView.Reset();
        GLevelSettingsTarget.Reset();
        if (FPropertyEditorModule* PropertyModule =
            FModuleManager::GetModulePtr<FPropertyEditorModule>("PropertyEditor"))
        {
            PropertyModule->UnregisterCustomPropertyTypeLayout("KyaScenarioCondition");
            PropertyModule->UnregisterCustomPropertyTypeLayout("KyaActorBehaviour");
            PropertyModule->UnregisterCustomClassLayout(UKyaDisc::StaticClass()->GetFName());
            PropertyModule->UnregisterCustomClassLayout(AKyaActorHandle::StaticClass()->GetFName());
            PropertyModule->UnregisterCustomClassLayout(AKyaLevelPreview::StaticClass()->GetFName());
            PropertyModule->UnregisterCustomClassLayout(AKyaSpatialRecordHandle::StaticClass()->GetFName());
            PropertyModule->UnregisterCustomClassLayout(AKyaHierarchyHandle::StaticClass()->GetFName());
            PropertyModule->UnregisterCustomClassLayout(AKyaGeometryWorkingActor::StaticClass()->GetFName());
            PropertyModule->UnregisterCustomClassLayout(AKyaSceneVisualizer::StaticClass()->GetFName());
            PropertyModule->UnregisterCustomClassLayout(UKyaLevelSettings::StaticClass()->GetFName());
        }
        if (GEditor)
            GEditor->ActorFactories.RemoveAll([](UActorFactory* Factory)
            {
                return Factory && Factory->IsA<UKyaLevelPreviewFactory>();
            });
        // The handler unregisters itself from FReimportManager on destruction.
        ReimportHandler.Reset();
        FKyaLevelEditService::Get().Reset();
    }

    void OnStructuralCommandBegin()
    {
        bStructuralCommandHadGenerated = KyaSelectionContainsGeneratedActor();
    }

    void OnStructuralCommandEnd()
    {
        UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
        const bool bHadGenerated = bStructuralCommandHadGenerated;
        bStructuralCommandHadGenerated = false;
        // Always sweep: a paste can introduce generated copies even when the
        // pre-command selection contained no generated actors.
        const int32 Removed = KyaDestroyStrayGeneratedActors(World);
        if (!bHadGenerated && Removed == 0) return;
        KyaShowSummary(TEXT("Kya Level Editing"), FString::Printf(
            TEXT("Generated Kya actors represent bank records and cannot be duplicated, cut or pasted.\n\n"
                "%d generated cop%s removed. Edit the original actor or the bank asset instead. "
                "Delete is blocked for generated actors."), Removed, Removed == 1 ? TEXT("y was") : TEXT("ies were")));
    }

void RegisterMenus()
    {
        if (!UToolMenus::IsToolMenuUIEnabled()) return;
        UToolMenus* Menus = UToolMenus::Get();
        if (!Menus) return;
        // Own the entries so module shutdown removes them with the module's callbacks.
        FToolMenuOwnerScoped OwnerScoped(this);
        UToolMenu* ToolsMenu = Menus->ExtendMenu("LevelEditor.MainMenu.Tools");
        if (!ToolsMenu) return;
        FToolMenuSection& Section = ToolsMenu->FindOrAddSection("KyaLevel",
            FText::FromString(TEXT("Kya Level")));
        Section.AddSeparator("KyaLevelSeparator");
        Section.AddDynamicEntry("KyaActions", FNewToolMenuSectionDelegate::CreateLambda(
            [](FToolMenuSection& DynamicSection) { AddKyaMenuEntries(DynamicSection); }));
    }

private:
    TUniquePtr<FKyaLevelBankReimportHandler> ReimportHandler;
    bool bStructuralCommandHadGenerated = false;
    TArray<FDelegateHandle> StructuralBeginHandles;
    FDelegateHandle DiscMapOpenedHandle;
    TArray<FDelegateHandle> StructuralEndHandles;
};

IMPLEMENT_MODULE(FKyaLevelEditorModule, KyaLevelEditor)

FAutoConsoleCommand KyaValidateConsoleCommand(
    TEXT("Kya.Validate"),
    TEXT("Validate the banks used by the selected level preview (or every preview in the level)."),
    FConsoleCommandWithArgsDelegate::CreateStatic(&KyaValidateCommand));

FAutoConsoleCommand KyaExportConsoleCommand(
    TEXT("Kya.Export"),
    TEXT("Validate and export the selected level's banks. Optional argument: output directory."),
    FConsoleCommandWithArgsDelegate::CreateStatic(&KyaExportCommand));

FAutoConsoleCommand KyaDiscoverConsoleCommand(
    TEXT("Kya.DiscoverSectors"),
    TEXT("Discover sibling SECT bank files. Optional argument 0 disables importing missing banks."),
    FConsoleCommandWithArgsDelegate::CreateStatic(&KyaDiscoverCommand));

FAutoConsoleCommand KyaSaveConsoleCommand(
    TEXT("Kya.SaveLevel"),
    TEXT("Save the bank packages used by the selected level preview."),
    FConsoleCommandWithArgsDelegate::CreateStatic(&KyaSaveCommand));

FAutoConsoleCommand KyaCreateActorConsoleCommand(
    TEXT("Kya.CreateActor"),
    TEXT("Append a new bank actor record based on the selected actor's class and layout."),
    FConsoleCommandWithArgsDelegate::CreateStatic(&KyaCreateActorCommand));

FAutoConsoleCommand KyaDuplicateActorConsoleCommand(
    TEXT("Kya.DuplicateActor"),
    TEXT("Append a copy of the selected bank actor record with a new identity."),
    FConsoleCommandWithArgsDelegate::CreateStatic(&KyaDuplicateActorCommand));

FAutoConsoleCommand KyaDeleteActorConsoleCommand(
    TEXT("Kya.DeleteActor"),
    TEXT("Delete the selected bank actor record, remapping typed actor references."),
    FConsoleCommandWithArgsDelegate::CreateStatic(&KyaDeleteActorCommand));

FAutoConsoleCommand KyaCreateLevelSetConsoleCommand(
    TEXT("Kya.CreateLevelSet"),
    TEXT("Create or update the persisted level-set asset for the selected preview."),
    FConsoleCommandWithArgsDelegate::CreateStatic(&KyaCreateLevelSetCommand));

FAutoConsoleCommand KyaHideSelectedConsoleCommand(
    TEXT("Kya.HideSelected"),
    TEXT("Temporarily hide the selected generated Kya actors in the editor."),
    FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>&)
    {
        int32 Affected = 0;
        KyaSetSelectedGeneratedActorsHidden(true, Affected);
        UE_LOG(LogTemp, Display, TEXT("KYA_HIDE_SELECTED affected=%d"), Affected);
    }));

FAutoConsoleCommand KyaShowSelectedConsoleCommand(
    TEXT("Kya.ShowSelected"),
    TEXT("Reveal the selected generated Kya actors (clear isolation to reveal everything)."),
    FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>&)
    {
        int32 Affected = 0;
        KyaSetSelectedGeneratedActorsHidden(false, Affected);
        UE_LOG(LogTemp, Display, TEXT("KYA_SHOW_SELECTED affected=%d"), Affected);
    }));

FAutoConsoleCommand KyaIsolateSelectedConsoleCommand(
    TEXT("Kya.IsolateSelected"),
    TEXT("Isolate the selected generated Kya actors, hiding the other representations."),
    FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>&)
    {
        int32 Affected = 0;
        KyaIsolateSelectedGeneratedActors(Affected);
        UE_LOG(LogTemp, Display, TEXT("KYA_ISOLATE_SELECTED affected=%d"), Affected);
    }));

FAutoConsoleCommand KyaClearIsolationConsoleCommand(
    TEXT("Kya.ClearIsolation"),
    TEXT("Clear temporary hiding and isolation on every Kya level preview."),
    FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>&)
    {
        KyaClearGeneratedActorIsolation();
    }));

FAutoConsoleCommand KyaClearPreviewCameraConsoleCommand(
    TEXT("Kya.ClearPreviewCamera"),
    TEXT("Clear the comparison/capture camera lock and resume authoring discovery."),
    FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>&)
    {
        KyaClearPreviewCamera();
    }));

FAutoConsoleCommand KyaSelectAtCursorConsoleCommand(
    TEXT("Kya.SelectAtCursor"),
    TEXT("Select the actor whose rendered mesh is under the level viewport cursor and highlight its whole mesh."),
    FConsoleCommandDelegate::CreateLambda([]()
    {
        FString Error;
        if (!KyaSelectRenderedActorAtCursor(&Error))
            UE_LOG(LogTemp, Display, TEXT("KYA_SELECT_AT_CURSOR %s"), *Error);
    }));

FAutoConsoleCommand KyaAuthoringVisibilityConsoleCommand(
    TEXT("Kya.AuthoringVisibility"),
    TEXT("Show (1, default) or hide (0) editor-only authoring discovery (unloaded-sector actors and culled scenery)."),
    FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args)
    {
        KyaSetAuthoringVisibility(Args.Num() == 0 || Args[0] != TEXT("0"));
    }));

FAutoConsoleCommand KyaShowAllPathsConsoleCommand(
    TEXT("Kya.ShowAllPaths"),
    TEXT("Show (1, default) or hide (0) every path with linear interpolation and selectable points."),
    FConsoleCommandWithArgsDelegate::CreateStatic(&KyaSetAllPathsCommand));

FAutoConsoleCommand KyaLevelSettingsConsoleCommand(
    TEXT("Kya.LevelSettings"),
    TEXT("Open the level settings tab for the selected level preview."),
    FConsoleCommandWithArgsDelegate::CreateStatic(&KyaOpenLevelSettingsCommand));

FAutoConsoleCommand KyaLockGeneratedSelectionConsoleCommand(
    TEXT("Kya.LockGeneratedSelection"),
    TEXT("Toggle viewport picking on selected Kya previews, or all previews when none is selected."),
    FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>&)
    {
        const bool bLocked = KyaAllPreviews([](const AKyaLevelPreview& Preview) { return Preview.bLockGeneratedActorSelection; });
        KyaSetGeneratedActorSelectionLocked(!bLocked);
        UE_LOG(LogTemp, Display, TEXT("KYA_LOCK_GENERATED_SELECTION locked=%d"), bLocked ? 0 : 1);
    }));

FAutoConsoleCommand ViewportTonemapperCommand(
    TEXT("Kya.ViewportTonemapper"),
    TEXT("Set tone mapping in level editor viewports. Use 0 for native Kya colors or 1 to enable tone mapping."),
    FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args)
    {
        if (!GEditor || Args.Num() != 1 || (Args[0] != TEXT("0") && Args[0] != TEXT("1"))) return;
        const bool bEnabled = Args[0] == TEXT("1");
        int32 ViewportCount = 0;
        for (FLevelEditorViewportClient* Viewport : GEditor->GetLevelViewportClients())
        {
            if (!Viewport) continue;
            const auto Flag = FEngineShowFlags::EShowFlag::SF_Tonemapper;
            if (Viewport->HandleIsShowFlagEnabled(Flag) != bEnabled)
                Viewport->HandleToggleShowFlag(Flag);
            Viewport->Invalidate();
            ++ViewportCount;
        }
        UE_LOG(LogTemp, Display, TEXT("Kya viewport tone mapping %s in %d level editor viewports"),
            bEnabled ? TEXT("enabled") : TEXT("disabled"), ViewportCount);
    }));
