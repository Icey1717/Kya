#include "Modules/ModuleManager.h"
#include "Editor.h"
#include "HAL/IConsoleManager.h"
#include "LevelEditorViewport.h"
#include "KyaActorEditService.h"
#include "KyaActorDetailsCustomization.h"
#include "KyaActorHandle.h"
#include "KyaHierarchyDetailsCustomization.h"
#include "KyaHierarchyHandle.h"
#include "KyaSceneVisualizer.h"
#include "KyaSceneVisualizerDetailsCustomization.h"
#include "KyaLevelSettings.h"
#include "KyaLevelSettingsDetailsCustomization.h"
#include "KyaSpatialRecordDetailsCustomization.h"
#include "KyaSpatialRecordHandle.h"
#include "KyaLevelActorFactory.h"
#include "KyaLevelEditorActions.h"
#include "KyaLevelBankReimportHandler.h"
#include "KyaLevelBank.h"
#include "KyaLevelPreview.h"
#include "KyaLevelSet.h"
#include "KyaLevelSettings.h"
#include "Misc/App.h"
#include "Misc/MessageDialog.h"
#include "Misc/Paths.h"
#include "PropertyEditorModule.h"
#include "IDetailsView.h"
#include "LevelEditor.h"
#include "Framework/Docking/TabManager.h"
#include "Widgets/Docking/SDockTab.h"
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
            GLevelSettingsView.ToSharedRef()
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
    KyaShowSummary(TEXT("Kya Paths"), bVisible
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
    const TArray<FKyaBankValidationReport> Reports = KyaValidateLevelBanks(Banks);
    TArray<FString> Lines;
    int32 Failures = 0;
    for (const FKyaBankValidationReport& Report : Reports)
    {
        Lines.Add(Report.Summary());
        if (!Report.bBytesBuilt) ++Failures;
    }
    KyaShowSummary(TEXT("Validate Kya Level"), FString::Printf(TEXT("%d bank(s), %d not exportable\n%s"),
        Reports.Num(), Failures, *FString::Join(Lines, TEXT("\n"))));
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
        KyaShowSummary(TEXT("Save Kya Level"), FString::Printf(TEXT("Saved %d bank package(s)."), Banks.Num()));
    else
        KyaShowSummary(TEXT("Save Kya Level"), Error);
}

void KyaDuplicateActorCommand(const TArray<FString>& Args)
{
    const FKyaStructuralActionResult Result = KyaDuplicateSelectedActor();
    KyaShowSummary(TEXT("Duplicate Kya Actor"), Result.bSuccess
        ? FString::Printf(TEXT("Duplicated the actor record at index %d."), Result.ActorIndex)
        : Result.Error);
}

void KyaCreateActorCommand(const TArray<FString>& Args)
{
    const FKyaStructuralActionResult Result = KyaCreateSelectedActor();
    KyaShowSummary(TEXT("Create Kya Actor"), Result.bSuccess
        ? FString::Printf(TEXT("Created actor record at index %d from the selected template."), Result.ActorIndex)
        : Result.Error);
}

void KyaDeleteActorCommand(const TArray<FString>& Args)
{
    const FKyaStructuralActionResult Result = KyaDeleteSelectedActor();
    KyaShowSummary(TEXT("Delete Kya Actor"), Result.bSuccess
        ? FString::Printf(TEXT("Deleted the actor record at index %d and remapped typed references."), Result.ActorIndex)
        : Result.Error);
}

void AddKyaMenuEntries(FToolMenuSection& Section)
{
    Section.AddMenuEntry("KyaValidateLevel",
        FText::FromString(TEXT("Validate Kya Level")),
        FText::FromString(TEXT("Validate every bank used by the selected level preview.")),
        FSlateIcon(),
        FUIAction(FExecuteAction::CreateLambda([]() { KyaValidateCommand({}); })));
    Section.AddMenuEntry("KyaExportLevel",
        FText::FromString(TEXT("Export Kya Level")),
        FText::FromString(TEXT("Validate and write the selected level's banks to Saved/KyaExport.")),
        FSlateIcon(),
        FUIAction(FExecuteAction::CreateLambda([]() { KyaExportCommand({}); })));
    Section.AddMenuEntry("KyaDiscoverSectors",
        FText::FromString(TEXT("Refresh Sector Discovery")),
        FText::FromString(TEXT("Find sibling SECT bank files next to the level's main bank.")),
        FSlateIcon(),
        FUIAction(FExecuteAction::CreateLambda([]() { KyaDiscoverCommand({}); })));
    Section.AddMenuEntry("KyaCreateLevelSet",
        FText::FromString(TEXT("Create Kya Level Set")),
        FText::FromString(TEXT("Create or update the persisted level-set asset for the selected preview.")),
        FSlateIcon(),
        FUIAction(FExecuteAction::CreateLambda([]() { KyaCreateLevelSetCommand({}); })));
    Section.AddMenuEntry("KyaLevelSettings",
        FText::FromString(TEXT("Edit Level Settings")),
        FText::FromString(TEXT("Open environment, camera, fog and scene configuration authoring for the selected level.")),
        FSlateIcon(),
        FUIAction(FExecuteAction::CreateLambda([]() { KyaOpenLevelSettingsCommand({}); })));
    Section.AddMenuEntry("KyaSaveLevel",
        FText::FromString(TEXT("Save Kya Level")),
        FText::FromString(TEXT("Save the bank packages used by the selected level preview.")),
        FSlateIcon(),
        FUIAction(FExecuteAction::CreateLambda([]() { KyaSaveCommand({}); })));
    Section.AddSeparator("KyaStructuralSeparator");
    Section.AddMenuEntry("KyaCreateActorRecord",
        FText::FromString(TEXT("Create Kya Actor")),
        FText::FromString(TEXT("Append a new bank actor record based on the selected actor's class and layout.")),
        FSlateIcon(),
        FUIAction(FExecuteAction::CreateLambda([]() { KyaCreateActorCommand({}); })));
    Section.AddMenuEntry("KyaDuplicateActorRecord",
        FText::FromString(TEXT("Duplicate Kya Actor")),
        FText::FromString(TEXT("Append a copy of the selected bank actor record with a new identity.")),
        FSlateIcon(),
        FUIAction(FExecuteAction::CreateLambda([]() { KyaDuplicateActorCommand({}); })));
    Section.AddMenuEntry("KyaDeleteActorRecord",
        FText::FromString(TEXT("Delete Kya Actor")),
        FText::FromString(TEXT("Delete the selected bank actor record, remapping typed actor references.")),
        FSlateIcon(),
        FUIAction(FExecuteAction::CreateLambda([]() { KyaDeleteActorCommand({}); })));
    Section.AddMenuEntry("KyaHideGenerated",
        FText::FromString(TEXT("Hide Selected Generated Actors")),
        FText::FromString(TEXT("Temporarily hide the selected Kya representations in the editor only.")),
        FSlateIcon(),
        FUIAction(FExecuteAction::CreateLambda([]()
        {
            int32 Affected = 0;
            KyaSetSelectedGeneratedActorsHidden(true, Affected);
            KyaShowSummary(TEXT("Kya Hide"), FString::Printf(
                TEXT("%d generated actor(s) hidden in the editor. Game visibility and bank data are unchanged."),
                Affected));
        })));
    Section.AddMenuEntry("KyaShowGenerated",
        FText::FromString(TEXT("Show Selected Generated Actors")),
        FText::FromString(TEXT("Reveal the selected Kya representations (isolation must be cleared).")),
        FSlateIcon(),
        FUIAction(FExecuteAction::CreateLambda([]()
        {
            int32 Affected = 0;
            KyaSetSelectedGeneratedActorsHidden(false, Affected);
            KyaShowSummary(TEXT("Kya Show"), FString::Printf(TEXT("%d generated actor(s) shown."), Affected));
        })));
    Section.AddMenuEntry("KyaIsolateGenerated",
        FText::FromString(TEXT("Isolate Selected Generated Actors")),
        FText::FromString(TEXT("Hide every other generated representation until isolation is cleared.")),
        FSlateIcon(),
        FUIAction(FExecuteAction::CreateLambda([]()
        {
            int32 Affected = 0;
            KyaIsolateSelectedGeneratedActors(Affected);
            KyaShowSummary(TEXT("Kya Isolate"), Affected > 0
                ? FString::Printf(TEXT("Isolated %d generated actor(s). Use Clear Generated Isolation to restore."),
                    Affected)
                : FString(TEXT("Select at least one generated actor to isolate it.")));
        })));
    Section.AddMenuEntry("KyaClearIsolation",
        FText::FromString(TEXT("Clear Generated Isolation")),
        FText::FromString(TEXT("Clear temporary hiding and isolation on every Kya level preview.")),
        FSlateIcon(),
        FUIAction(FExecuteAction::CreateLambda([]()
        {
            KyaClearGeneratedActorIsolation();
            KyaShowSummary(TEXT("Kya Isolation"), TEXT("All generated representations are visible again."));
        })));
    Section.AddMenuEntry("KyaShowAllPaths",
        FText::FromString(TEXT("Show All Paths")),
        FText::FromString(TEXT("Draw every path (linear) and spawn selectable points without setting a path index.")),
        FSlateIcon(),
        FUIAction(FExecuteAction::CreateLambda([]() { KyaSetAllPathsCommand({ TEXT("1") }); })));
    Section.AddMenuEntry("KyaHideAllPaths",
        FText::FromString(TEXT("Hide All Paths")),
        FText::FromString(TEXT("Stop drawing every path; the Empty Preview Path Index guide still applies.")),
        FSlateIcon(),
        FUIAction(FExecuteAction::CreateLambda([]() { KyaSetAllPathsCommand({ TEXT("0") }); })));
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
        FUIAction(FExecuteAction::CreateLambda([]() { KyaSetAuthoringVisibility(true); })));
    Section.AddMenuEntry("KyaEditorCamera",
        FText::FromString(TEXT("Return to Editor Camera")),
        FText::FromString(TEXT("Clear the comparison/capture camera lock and resume authoring discovery.")),
        FSlateIcon(),
        FUIAction(FExecuteAction::CreateLambda([]() { KyaClearPreviewCamera(); })));
    Section.AddMenuEntry("KyaNativeVisibility",
        FText::FromString(TEXT("Restore Native Visibility")),
        FText::FromString(TEXT("Use game-like distance and sector culling (comparison/capture behavior).")),
        FSlateIcon(),
        FUIAction(FExecuteAction::CreateLambda([]() { KyaSetAuthoringVisibility(false); })));
    Section.AddMenuEntry("KyaLockGenerated",
        FText::FromString(TEXT("Toggle Generated Actor Selection Lock")),
        FText::FromString(TEXT("Prevent viewport picking of generated actors (they stay visible and editable).")),
        FSlateIcon(),
        FUIAction(FExecuteAction::CreateLambda([]()
        {
            const bool bLocked = KyaGetTargetPreviews().ContainsByPredicate([](const AKyaLevelPreview* Preview)
            {
                return Preview && Preview->bLockGeneratedActorSelection;
            });
            KyaSetGeneratedActorSelectionLocked(!bLocked);
            KyaShowSummary(TEXT("Kya Selection Lock"), !bLocked
                ? TEXT("Generated actors are selection-locked; they remain visible and editable through the bank.")
                : TEXT("Generated actors can be picked in the viewport again."));
        })));
}
}

class FKyaLevelEditorModule : public IModuleInterface
{
public:
    virtual void StartupModule() override
    {
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
        PropertyModule.RegisterCustomClassLayout(AKyaActorHandle::StaticClass()->GetFName(),
            FOnGetDetailCustomizationInstance::CreateStatic(&FKyaActorDetailsCustomization::MakeInstance));
        PropertyModule.RegisterCustomClassLayout(AKyaSpatialRecordHandle::StaticClass()->GetFName(),
            FOnGetDetailCustomizationInstance::CreateStatic(&FKyaSpatialRecordDetailsCustomization::MakeInstance));
        PropertyModule.RegisterCustomClassLayout(AKyaHierarchyHandle::StaticClass()->GetFName(),
            FOnGetDetailCustomizationInstance::CreateStatic(&FKyaHierarchyDetailsCustomization::MakeInstance));
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
            PropertyModule->UnregisterCustomClassLayout(AKyaActorHandle::StaticClass()->GetFName());
            PropertyModule->UnregisterCustomClassLayout(AKyaSpatialRecordHandle::StaticClass()->GetFName());
            PropertyModule->UnregisterCustomClassLayout(AKyaHierarchyHandle::StaticClass()->GetFName());
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
        AddKyaMenuEntries(Section);
    }

private:
    TUniquePtr<FKyaLevelBankReimportHandler> ReimportHandler;
    bool bStructuralCommandHadGenerated = false;
    TArray<FDelegateHandle> StructuralBeginHandles;
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
    TEXT("Toggle viewport picking of generated actors on every Kya level preview."),
    FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>&)
    {
        const bool bLocked = KyaGetTargetPreviews().ContainsByPredicate([](const AKyaLevelPreview* Preview)
        {
            return Preview && Preview->bLockGeneratedActorSelection;
        });
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
