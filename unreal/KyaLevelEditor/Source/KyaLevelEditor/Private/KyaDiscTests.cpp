#if WITH_DEV_AUTOMATION_TESTS
#include "KyaDisc.h"
#include "KyaDiscFactory.h"
#include "KyaDiscWorkflow.h"
#include "KyaIso.h"
#include "KyaBankExport.h"
#include "HAL/FileManager.h"
#include "Misc/Paths.h"
#include "KyaLevelSet.h"
#include "KyaLevelBank.h"
#include "KyaLevelPreview.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Misc/AutomationTest.h"
#include "Misc/Guid.h"
#include "UObject/Package.h"
#include "UObject/StrongObjectPtr.h"
#include "UObject/SavePackage.h"
#include "Misc/PackageName.h"
#include "Misc/FileHelper.h"
#include "ProceduralMeshComponent.h"
#include "FileHelpers.h"
#include "Editor.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKyaDiscSavedChangesTest, "Kya.Disc.SavedChanges",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FKyaDiscSavedChangesTest::RunTest(const FString& Parameters)
{
    const FString Path = FPlatformMisc::GetEnvironmentVariable(TEXT("KYA_TEST_DISC_ASSET"));
    if (Path.IsEmpty()) { AddInfo(TEXT("Set KYA_TEST_DISC_ASSET to inspect saved disc changes without writing the ISO.")); return true; }
    UKyaDisc* Disc = LoadObject<UKyaDisc>(nullptr, *Path);
    if (!TestNotNull(TEXT("Load saved disc"), Disc)) return false;
    AddInfo(FString::Printf(TEXT("Disc has %d levels"), Disc->Levels.Num()));
    for (const FKyaDiscLevel& Level : Disc->Levels)
    {
        UKyaLevelSet* Set = Level.LevelSet;
        if (!Set)
        {
            const FString Root = FPackageName::GetLongPackagePath(Disc->GetOutermost()->GetName()) / Disc->GetName();
            const FString PackageName = Root + Level.SourceDirectory / TEXT("LevelSet");
            Set = LoadObject<UKyaLevelSet>(nullptr, *(PackageName + TEXT(".LevelSet")));
            AddInfo(TEXT("Missing saved association: ") + Level.SourceDirectory);
        }
        if (!Set || !Set->MainBank) continue;
        UKyaLevelBank* Bank = Set->MainBank;
        const FString Before = KyaDiscBankStateHash(*Bank);
        TArray<uint8> Bytes;
        FKyaBankExportDiagnostics Diagnostics;
        const bool Built = KyaBuildBankExportBytes(Bank, Bytes, Diagnostics);
        if (Before != Bank->SourceDiscStateHash)
            TestTrue(*FString::Printf(TEXT("Saved edits can be serialized: %s %s"), *Bank->GetPathName(), *Diagnostics.Summary()), Built);
        AddInfo(FString::Printf(TEXT("%s stateChanged=%d bytesChanged=%d built=%d %s"),
            *Bank->GetPathName(), Before != Bank->SourceDiscStateHash,
            Built && FMD5::HashBytes(Bytes.GetData(), Bytes.Num()) != Bank->SourceDiscHash,
            Built, *Diagnostics.Summary()));
        if (!Bank->SceneEnvironment.Actors.IsEmpty())
        {
            const double Original = Bank->SceneEnvironment.Actors[0].Position.X;
            Bank->SceneEnvironment.Actors[0].Position.X += 1;
            TestNotEqual(TEXT("Actor edit changes fingerprint"), KyaDiscBankStateHash(*Bank), Before);
            Bank->SceneEnvironment.Actors[0].Position.X = Original;
        }
    }
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKyaDiscAssociationRecoveryTest, "Kya.Disc.AssociationRecovery",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FKyaDiscAssociationRecoveryTest::RunTest(const FString& Parameters)
{
    const FString Root = TEXT("/Game/KyaDiscRecovery_") + FGuid::NewGuid().ToString(EGuidFormats::Digits);
    TStrongObjectPtr<UKyaDisc> Disc(NewObject<UKyaDisc>(CreatePackage(*(Root / TEXT("Disc"))), TEXT("Disc")));
    FKyaDiscLevel& Level = Disc->Levels.AddDefaulted_GetRef();
    Level.SourceDirectory = TEXT("/CDEURO/LEVEL/LEVEL_7");
    const FString Folder = Root / TEXT("Disc") + Level.SourceDirectory;
    UKyaLevelSet* Set = NewObject<UKyaLevelSet>(CreatePackage(*(Folder / TEXT("LevelSet"))), TEXT("LevelSet"));
    UKyaLevelBank* Bank = NewObject<UKyaLevelBank>(CreatePackage(*(Folder / TEXT("LEVEL"))), TEXT("LEVEL"));
    Set->MainBank = Bank;
    Bank->SourceDisc = Disc.Get();
    Bank->SourceDiscPath = Level.SourceDirectory / TEXT("LEVEL.BNK");
    Bank->SourceDiscHash = TEXT("unused: unchanged bank must never reach the ISO writer");
    Bank->SourceDiscStateHash = KyaDiscBankStateHash(*Bank);
    FString Report;
    TArray<FString> Output;
    TestTrue(TEXT("Recover without reimporting or writing the ISO"), KyaExportDisc(*Disc, Report,
        [&](const FString& Message) { Output.Add(Message); }));
    TestTrue(TEXT("Output explains recovered associations"), Output.Contains(TEXT("Recovered level association: ") + Level.SourceDirectory));
    TestTrue(TEXT("Output identifies the bank being checked"), Output.Contains(TEXT("Checking: ") + Bank->SourceDiscPath));
    TestTrue(TEXT("Output explains unchanged banks"), Output.Contains(TEXT("  Unchanged.")));
    TestTrue(TEXT("Recovered the existing set"), Level.LevelSet == Set);
    TestEqual(TEXT("Unchanged banks remain a no-op"), Report, FString(TEXT("No bank changes to export.")));
    Level.LevelSet = nullptr;
    Bank->SourceDiscPath = TEXT("/OTHER/LEVEL.BNK");
    TestFalse(TEXT("Reject unrelated bank at the generated path"), KyaExportDisc(*Disc, Report));
    TestTrue(TEXT("Rejected association remains empty"), !Level.LevelSet);
    Disc->Levels.Reset();
    TestFalse(TEXT("An empty disc must not report no changes"), KyaExportDisc(*Disc, Report));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKyaDiscGenerationTest, "Kya.Disc.RealImageGeneration",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FKyaDiscGenerationTest::RunTest(const FString& Parameters)
{
    const FString Source = FPlatformMisc::GetEnvironmentVariable(TEXT("KYA_TEST_ISO"));
    if (Source.IsEmpty()) { AddInfo(TEXT("Set KYA_TEST_ISO for full disc generation coverage.")); return true; }
    const FString Root = TEXT("/Game/KyaDiscTests_") + FGuid::NewGuid().ToString(EGuidFormats::Digits);
    TStrongObjectPtr<UKyaDiscFactory> Factory(NewObject<UKyaDiscFactory>());
    bool Cancelled = false;
    UKyaDisc* Disc = Cast<UKyaDisc>(Factory->FactoryCreateFile(UKyaDisc::StaticClass(), CreatePackage(*(Root / TEXT("Disc"))),
        TEXT("Disc"), RF_Public | RF_Standalone, Source, TEXT(""), GWarn, Cancelled));
    if (!TestNotNull(TEXT("Import ISO root asset"), Disc)) return false;
    FString Error;
    if (!KyaGenerateDiscLevels(*Disc, Error)) { AddError(Error); return false; }
    TestTrue(TEXT("Discovered levels"), Disc->Levels.Num() > 0);
    for (const FKyaDiscLevel& Level : Disc->Levels)
    {
        if (!TestNotNull(TEXT("Level set"), Level.LevelSet.Get())) continue;
        UKyaLevelBank* Main = Level.LevelSet->MainBank;
        if (TestNotNull(TEXT("Main bank"), Main))
        {
            TestEqual(TEXT("Exact ISO provenance"), Main->SourceDiscPath, Level.SourceDirectory / TEXT("LEVEL.BNK"));
            TestTrue(TEXT("Disc owner"), Main->SourceDisc.Get() == Disc);
        }
        UWorld* World = Level.PreviewMap.Get();
        if (!TestNotNull(TEXT("Preview map"), World)) continue;
        int32 PreviewCount = 0;
        for (TActorIterator<AKyaLevelPreview> It(World); It; ++It)
        {
            ++PreviewCount;
            TestTrue(TEXT("Map references level set"), It->LevelSet == Level.LevelSet);
            TestTrue(TEXT("Preview uses main bank"), It->Bank == Main);
        }
        TestEqual(TEXT("One preview per map"), PreviewCount, 1);
    }
    TestTrue(TEXT("Repeat generation preserves existing assets"), KyaGenerateDiscLevels(*Disc, Error));
    // Route a real actor edit through the disc-level exporter, using a disposable image.
    const FString Copy = FPaths::CreateTempFilename(*FPaths::ProjectSavedDir(), TEXT("KyaDiscExport"), TEXT(".iso"));
    if (!TestTrue(TEXT("Copy image for export"), IFileManager::Get().Copy(*Copy, *Source, false, false) == COPY_OK)) return false;
    Disc->SourceFilename = Copy;
    FKyaDiscLevel* Editable = Disc->Levels.FindByPredicate([](const FKyaDiscLevel& L)
        { return L.SourceDirectory.EndsWith(TEXT("/LEVEL_1")); });
    if (TestNotNull(TEXT("Level 1 exists"), Editable) && Editable->LevelSet && Editable->LevelSet->MainBank)
    {
        UKyaLevelBank* Bank = Editable->LevelSet->MainBank;
        if (TestTrue(TEXT("Level has actors"), !Bank->SceneEnvironment.Actors.IsEmpty()))
        {
            Bank->SceneEnvironment.Actors[0].Position.X += 1;
            TArray<uint8> Expected;
            FKyaBankExportDiagnostics Diagnostics;
            TestTrue(TEXT("Serialize edit"), KyaBuildBankExportBytes(Bank, Expected, Diagnostics));
            // Reproduce saving the disc before generation and saving its banks
            // afterwards: the root has lost its link, but edited assets survive.
            UKyaLevelSet* GeneratedSet = Editable->LevelSet;
            Editable->LevelSet = nullptr;
            FString Report;
            if (!KyaExportDisc(*Disc, Report)) AddError(Report);
            else
            {
                TestTrue(TEXT("Export recovers the existing generated level set"), Editable->LevelSet == GeneratedSet);
                TArray<uint8> Actual;
                TestTrue(TEXT("Read exported bank from ISO"), KyaIso::Extract(Copy, Bank->SourceDiscPath, Actual, Error));
                TestTrue(TEXT("ISO contains current editor changes"), Actual == Expected);
                TestTrue(TEXT("Repeated export succeeds"), KyaExportDisc(*Disc, Report));
                TestEqual(TEXT("Repeated export is a no-op"), Report, FString(TEXT("No bank changes to export.")));
            }
        }
    }
    TArray<FString> Backups;
    IFileManager::Get().FindFiles(Backups, *(Copy + TEXT(".kya-backup-*")), true, false);
    for (const FString& Backup : Backups) IFileManager::Get().Delete(*(FPaths::GetPath(Copy) / Backup));
    IFileManager::Get().Delete(*Copy);
    Disc->SourceFilename = Source;
    if (FPlatformMisc::GetEnvironmentVariable(TEXT("KYA_TEST_SAVE_DISC")) == TEXT("1"))
    {
        TArray<UObject*> Assets;
        for (const FKyaDiscLevel& Level : Disc->Levels)
        {
            Assets.Add(Level.LevelSet->MainBank);
            for (const FKyaLevelSetSector& Sector : Level.LevelSet->Sectors) Assets.Add(Sector.Bank);
            Assets.Add(Level.LevelSet);
            Assets.Add(Level.PreviewMap.Get());
        }
        Assets.Add(Disc);
        FSavePackageArgs Args;
        Args.TopLevelFlags = RF_Public | RF_Standalone;
        for (UObject* Asset : Assets)
        {
            const FString Filename = FPackageName::LongPackageNameToFilename(Asset->GetOutermost()->GetName(),
                Asset->IsA<UWorld>() ? FPackageName::GetMapPackageExtension() : FPackageName::GetAssetPackageExtension());
            TestTrue(*FString::Printf(TEXT("Save %s"), *Asset->GetPathName()),
                UPackage::SavePackage(Asset->GetOutermost(), Asset, *Filename, Args));
        }
        FFileHelper::SaveStringToFile(Disc->GetPathName(), *(FPaths::ProjectSavedDir() / TEXT("KyaDiscTestAsset.txt")));
    }
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKyaDiscReopenTest, "Kya.Disc.ReopenGenerated",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FKyaDiscReopenTest::RunTest(const FString& Parameters)
{
    if (FPlatformMisc::GetEnvironmentVariable(TEXT("KYA_TEST_REOPEN_DISC")) != TEXT("1"))
    { AddInfo(TEXT("Run in a fresh process with KYA_TEST_REOPEN_DISC=1 after RealImageGeneration with KYA_TEST_SAVE_DISC=1.")); return true; }
    FString Path;
    if (!TestTrue(TEXT("Read saved disc path"), FFileHelper::LoadFileToString(Path, *(FPaths::ProjectSavedDir() / TEXT("KyaDiscTestAsset.txt"))))) return false;
    UKyaDisc* Disc = LoadObject<UKyaDisc>(nullptr, *Path);
    if (!TestNotNull(TEXT("Reopen disc"), Disc)) return false;
    for (const FKyaDiscLevel& Level : Disc->Levels)
    {
        const bool bLevelOne = Level.SourceDirectory.EndsWith(TEXT("/LEVEL_1"));
        UWorld* World = nullptr;
        if (bLevelOne)
        {
            const FString Filename = FPackageName::LongPackageNameToFilename(Level.PreviewMap.ToSoftObjectPath().GetLongPackageName(),
                FPackageName::GetMapPackageExtension());
            TestTrue(TEXT("Open generated map through editor"), FEditorFileUtils::LoadMap(Filename, false, true));
            World = GEditor->GetEditorWorldContext().World();
        }
        else World = Level.PreviewMap.LoadSynchronous();
        if (!TestNotNull(TEXT("Reopen generated map"), World)) continue;
        int32 Count = 0;
        for (TActorIterator<AKyaLevelPreview> It(World); It; ++It)
        {
            ++Count;
            TestTrue(TEXT("Persisted level association"), It->LevelSet == Level.LevelSet);
            It->ApplyLevelSet();
            TestTrue(TEXT("Persisted main bank"), It->Bank == Level.LevelSet->MainBank);
            TestEqual(TEXT("Persisted authoring fingerprint"), KyaDiscBankStateHash(*It->Bank), It->Bank->SourceDiscStateHash);
            if (bLevelOne)
            {
                TestTrue(TEXT("Opening the map automatically builds preview geometry"), It->Mesh && It->Mesh->GetNumSections() > 0);
            }
        }
        TestEqual(TEXT("One persisted preview"), Count, 1);
    }
    return true;
}
#endif
