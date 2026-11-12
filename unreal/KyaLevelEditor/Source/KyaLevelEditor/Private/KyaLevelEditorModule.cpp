#include "Modules/ModuleManager.h"
#include "Editor.h"
#include "HAL/IConsoleManager.h"
#include "LevelEditorViewport.h"
#include "KyaActorEditService.h"
#include "KyaActorDetailsCustomization.h"
#include "KyaActorHandle.h"
#include "KyaLevelActorFactory.h"
#include "KyaLevelEditorActions.h"
#include "KyaLevelBankReimportHandler.h"
#include "KyaLevelBank.h"
#include "KyaLevelPreview.h"
#include "Misc/App.h"
#include "Misc/MessageDialog.h"
#include "Misc/Paths.h"
#include "PropertyEditorModule.h"
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

void KyaSaveCommand(const TArray<FString>& Args)
{
    const TArray<UKyaLevelBank*> Banks = KyaCollectLevelBanks(KyaGetTargetPreviews());
    FString Error;
    if (KyaSaveLevelBanks(Banks, Error))
        KyaShowSummary(TEXT("Save Kya Level"), FString::Printf(TEXT("Saved %d bank package(s)."), Banks.Num()));
    else
        KyaShowSummary(TEXT("Save Kya Level"), Error);
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
    Section.AddMenuEntry("KyaSaveLevel",
        FText::FromString(TEXT("Save Kya Level")),
        FText::FromString(TEXT("Save the bank packages used by the selected level preview.")),
        FSlateIcon(),
        FUIAction(FExecuteAction::CreateLambda([]() { KyaSaveCommand({}); })));
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

        UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(
            this, &FKyaLevelEditorModule::RegisterMenus));
    }

    virtual void ShutdownModule() override
    {
        UToolMenus::UnRegisterStartupCallback(this);
        UToolMenus::UnregisterOwner(this);
        if (FPropertyEditorModule* PropertyModule =
            FModuleManager::GetModulePtr<FPropertyEditorModule>("PropertyEditor"))
            PropertyModule->UnregisterCustomClassLayout(AKyaActorHandle::StaticClass()->GetFName());
        if (GEditor)
            GEditor->ActorFactories.RemoveAll([](UActorFactory* Factory)
            {
                return Factory && Factory->IsA<UKyaLevelPreviewFactory>();
            });
        // The handler unregisters itself from FReimportManager on destruction.
        ReimportHandler.Reset();
        FKyaLevelEditService::Get().Reset();
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
