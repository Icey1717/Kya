#include "KyaDiscWorkflow.h"
#include "KyaDisc.h"
#include "KyaIso.h"
#include "KyaBankExport.h"
#include "KyaLevelBank.h"
#include "KyaLevelBankFactory.h"
#include "KyaLevelSet.h"
#include "KyaLevelPreview.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Engine/World.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"
#include "Misc/SecureHash.h"
#include "Misc/ScopedSlowTask.h"
#include "UObject/Package.h"
#include "UObject/StrongObjectPtr.h"
#include "UObject/UnrealType.h"
#include "Serialization/MemoryWriter.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"
#include "Serialization/StructuredArchiveAdapters.h"

FString KyaDiscBankStateHash(const UKyaLevelBank& Bank)
{
    TArray<uint8> State;
    FMemoryWriter Writer(State);
    FObjectAndNameAsStringProxyArchive Archive(Writer, false);
    // Include game data only, excluding import metadata and reset-to-imported snapshots.
    for (const TCHAR* Name : {TEXT("Lights"), TEXT("bHasLights"), TEXT("SceneConfiguration"),
        TEXT("SceneEnvironment"), TEXT("Entries"), TEXT("EntryCount"),
        TEXT("SceneMeshEntry"), TEXT("SceneTextureEntry"), TEXT("BackgroundMeshEntry"), TEXT("BackgroundTextureEntry")})
    {
        FProperty* Property = FindFProperty<FProperty>(UKyaLevelBank::StaticClass(), Name);
        check(Property);
        Property->SerializeItem(FStructuredArchiveFromArchive(Archive).GetSlot(),
            const_cast<void*>(Property->ContainerPtrToValuePtr<void>(&Bank)), nullptr);
    }
    FMD5 Hash;
    Hash.Update(State.GetData(), State.Num());
    Hash.Update(Bank.BankBytes.GetData(), Bank.BankBytes.Num());
    uint8 Digest[16];
    Hash.Final(Digest);
    return BytesToHex(Digest, UE_ARRAY_COUNT(Digest));
}

bool KyaGenerateDiscLevels(UKyaDisc& Disc, FString& Error)
{
    Error.Reset();
    FScopedSlowTask Progress(Disc.Levels.Num(), FText::FromString(TEXT("Generating Kya levels")));
    Progress.MakeDialog(true);
    const FString Root = FPackageName::GetLongPackagePath(Disc.GetOutermost()->GetName()) / Disc.GetName();
    TStrongObjectPtr<UKyaLevelBankFactory> Factory(NewObject<UKyaLevelBankFactory>());
    auto ImportBank = [&](const FKyaDiscFile& File, const FString& Folder) -> UKyaLevelBank*
    {
        const FString Name = FPaths::GetBaseFilename(File.Path);
        const FString PackageName = Folder / Name;
        if (FindPackage(nullptr, *PackageName) || FPackageName::DoesPackageExist(PackageName))
        {
            UKyaLevelBank* Existing = LoadObject<UKyaLevelBank>(nullptr, *(PackageName + TEXT(".") + Name));
            if (Existing && Existing->SourceDisc.Get() == &Disc && Existing->SourceDiscPath == File.Path)
                return Existing;
            Error = FString::Printf(TEXT("Asset already exists: %s. Existing edits were preserved."), *PackageName);
            return nullptr;
        }
        TArray<uint8> Bytes;
        if (!KyaIso::Extract(Disc.SourceFilename, File.Path, Bytes, Error)) return nullptr;
        UPackage* Package = CreatePackage(*PackageName);
        const uint8* Buffer = Bytes.GetData();
        UKyaLevelBank* Bank = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
            Package, FName(*Name), RF_Public | RF_Standalone, nullptr, TEXT("bnk"), Buffer,
            Bytes.GetData() + Bytes.Num(), GWarn));
        if (!Bank) { Error = TEXT("Cannot import bank: ") + File.Path; return nullptr; }
        Bank->SourceDisc = &Disc;
        Bank->SourceDiscPath = File.Path;
        Bank->SourceDiscHash = FMD5::HashBytes(Bytes.GetData(), Bytes.Num());
        Bank->SourceFilename = File.Path;
        Bank->SourceDiscStateHash = KyaDiscBankStateHash(*Bank);
        FAssetRegistryModule::AssetCreated(Bank);
        Package->MarkPackageDirty();
        return Bank;
    };
    for (FKyaDiscLevel& Level : Disc.Levels)
    {
        if (Progress.ShouldCancel()) { Error = TEXT("Generation cancelled. Completed levels are retained; Generate Levels resumes remaining work."); return false; }
        Progress.EnterProgressFrame();
        if (Level.LevelSet && !Level.PreviewMap.IsNull()) continue;
        // Preserve the disc directory hierarchy to avoid collisions between regions.
        const FString Folder = Root + Level.SourceDirectory;
        const FString SetPackageName = Folder / TEXT("LevelSet");
        const FString MapPackageName = Folder / TEXT("Preview");
        if (FindPackage(nullptr, *MapPackageName) || FPackageName::DoesPackageExist(MapPackageName))
        { Error = TEXT("Destination already exists: ") + MapPackageName; return false; }
        const FKyaDiscFile* MainFile = Disc.Files.FindByPredicate([&](const FKyaDiscFile& File)
            { return File.Path == Level.SourceDirectory / TEXT("LEVEL.BNK"); });
        if (!MainFile) { Error = TEXT("Main bank is missing from disc inventory."); return false; }
        UKyaLevelBank* Main = ImportBank(*MainFile, Folder);
        if (!Main) return false;
        UKyaLevelSet* Set = nullptr;
        if (FindPackage(nullptr, *SetPackageName) || FPackageName::DoesPackageExist(SetPackageName))
        {
            Set = LoadObject<UKyaLevelSet>(nullptr, *(SetPackageName + TEXT(".LevelSet")));
            if (!Set || Set->MainBank != Main) { Error = TEXT("Unrelated level set already exists: ") + SetPackageName; return false; }
        }
        UPackage* SetPackage = Set ? Set->GetOutermost() : CreatePackage(*SetPackageName);
        if (!Set)
        {
            Set = NewObject<UKyaLevelSet>(SetPackage, TEXT("LevelSet"), RF_Public | RF_Standalone);
            FAssetRegistryModule::AssetCreated(Set);
        }
        Set->MainBank = Main;
        Set->ImportRevision = 1;
        for (const FKyaDiscFile& File : Disc.Files)
        {
            if (FPaths::GetPath(File.Path) != Level.SourceDirectory) continue;
            const FString Base = FPaths::GetBaseFilename(File.Path);
            const FString Id = Base.Mid(4);
            if (!Base.StartsWith(TEXT("SECT")) || Id.IsEmpty() || !Id.IsNumeric()
                || !File.Path.EndsWith(TEXT(".BNK"))) continue;
            UKyaLevelBank* Sector = ImportBank(File, Folder);
            if (!Sector) return false;
            Set->SetSector(FCString::Atoi(*Id), Sector, File.Path, false);
        }
        SetPackage->MarkPackageDirty();
        UPackage* MapPackage = CreatePackage(*MapPackageName);
        UWorld* World = UWorld::CreateWorld(EWorldType::Editor, false, TEXT("Preview"), MapPackage, false);
        if (!World) { Error = TEXT("Cannot create preview map."); return false; }
        World->SetFlags(RF_Public | RF_Standalone);
        AKyaLevelPreview* Preview = World->SpawnActor<AKyaLevelPreview>();
        if (!Preview) { Error = TEXT("Cannot create level preview actor."); return false; }
        Preview->SetActorLabel(FPaths::GetCleanFilename(Level.SourceDirectory));
        Preview->LevelSet = Set;
        // Keep generated maps lightweight. The map-open hook builds transient preview
        // geometry and editing handles from these persisted bank associations.
        Preview->Bank = Main;
        for (const FKyaLevelSetSector& Sector : Set->Sectors) Preview->SectorBanks.Add(Sector.Bank);
        FAssetRegistryModule::AssetCreated(World);
        MapPackage->MarkPackageDirty();
        Level.LevelSet = Set;
        Level.PreviewMap = World;
        Disc.MarkPackageDirty();
    }
    return true;
}

bool KyaExportDisc(UKyaDisc& Disc, FString& Report, const TFunction<void(const FString&)>& Output)
{
    auto Log = [&](const FString& Message) { if (Output) Output(Message); };
    Log(TEXT("Exporting changes to: ") + Disc.SourceFilename);
    Log(TEXT("Finding generated level banks..."));
    TArray<UKyaLevelBank*> Banks;
    for (FKyaDiscLevel& Level : Disc.Levels)
    {
        // The disc may have been saved before Generate Levels, while its generated
        // assets were saved later. Recover that association without reimporting
        // or replacing the banks containing the user's edits.
        if (!Level.LevelSet)
        {
            const FString Root = FPackageName::GetLongPackagePath(Disc.GetOutermost()->GetName()) / Disc.GetName();
            const FString PackageName = Root + Level.SourceDirectory / TEXT("LevelSet");
            if (FindPackage(nullptr, *PackageName) || FPackageName::DoesPackageExist(PackageName))
            {
                UKyaLevelSet* Existing = LoadObject<UKyaLevelSet>(nullptr, *(PackageName + TEXT(".LevelSet")));
                if (!Existing || !Existing->MainBank || Existing->MainBank->SourceDisc.Get() != &Disc
                    || Existing->MainBank->SourceDiscPath != Level.SourceDirectory / TEXT("LEVEL.BNK"))
                { Report = TEXT("Cannot recover disc association from: ") + PackageName; return false; }
                Level.LevelSet = Existing;
                Disc.MarkPackageDirty();
                Log(TEXT("Recovered level association: ") + Level.SourceDirectory);
            }
        }
        if (!Level.LevelSet) continue;
        if (Level.LevelSet->MainBank) Banks.AddUnique(Level.LevelSet->MainBank);
        for (const FKyaLevelSetSector& Sector : Level.LevelSet->Sectors)
            if (Sector.Bank) Banks.AddUnique(Sector.Bank);
    }
    if (Banks.IsEmpty())
    {
        Report = TEXT("No generated banks were found for this disc. Use Generate Levels, then Save All before exporting.");
        return false;
    }
    TArray<KyaIso::FReplacement> Replacements;
    TArray<UKyaLevelBank*> Changed;
    FScopedSlowTask Progress(Banks.Num(), FText::FromString(TEXT("Checking Kya disc changes")));
    if (!Output) Progress.MakeDialog();
    Log(FString::Printf(TEXT("Checking %d banks for changes..."), Banks.Num()));
    for (UKyaLevelBank* Bank : Banks)
    {
        Progress.EnterProgressFrame(1, FText::FromString(Bank->SourceDiscPath));
        Log(TEXT("Checking: ") + Bank->SourceDiscPath);
        if (Bank->SourceDisc.Get() != &Disc || Bank->SourceDiscPath.IsEmpty() || Bank->SourceDiscHash.IsEmpty())
        { Report = TEXT("Bank has no matching disc provenance: ") + Bank->GetPathName(); return false; }
        if (!Bank->SourceDiscStateHash.IsEmpty() && KyaDiscBankStateHash(*Bank) == Bank->SourceDiscStateHash)
        { Log(TEXT("  Unchanged.")); continue; }
        Log(TEXT("  Changes detected; validating and serializing bank..."));
        KyaIso::FReplacement Replacement;
        Replacement.Path = Bank->SourceDiscPath;
        Replacement.ExpectedHash = Bank->SourceDiscHash;
        FKyaBankExportDiagnostics Diagnostics;
        if (!KyaBuildBankExportBytes(Bank, Replacement.Bytes, Diagnostics))
        {
            FString Reason = Diagnostics.Summary();
            if (Reason.IsEmpty())
            {
                FString SupportReason;
                Reason = KyaBankSupportsExport(Bank, SupportReason)
                    ? TEXT("The bank writer could not serialize these edits.") : SupportReason;
            }
            Report = Bank->SourceDiscPath + TEXT(": ") + Reason;
            return false;
        }
        if (FMD5::HashBytes(Replacement.Bytes.GetData(), Replacement.Bytes.Num()) == Replacement.ExpectedHash)
        { Log(TEXT("  Serialized bytes are unchanged.")); continue; }
        Log(FString::Printf(TEXT("  Validated: %d bytes to export."), Replacement.Bytes.Num()));
        if (!Diagnostics.Summary().IsEmpty()) Log(Diagnostics.Summary());
        Changed.Add(Bank);
        Replacements.Add(MoveTemp(Replacement));
    }
    if (Replacements.IsEmpty()) { Report = TEXT("No bank changes to export."); return true; }
    FString Backup;
    if (!KyaIso::ReplaceFiles(Disc.SourceFilename, Replacements, Backup, Report, Output)) return false;
    Log(TEXT("Updating bank hashes and disc inventory..."));
    for (int32 I = 0; I < Changed.Num(); ++I)
    {
        Changed[I]->SourceDiscHash = FMD5::HashBytes(Replacements[I].Bytes.GetData(), Replacements[I].Bytes.Num());
        Changed[I]->SourceDiscStateHash = KyaDiscBankStateHash(*Changed[I]);
        Changed[I]->MarkPackageDirty();
    }
    FString InventoryError;
    TArray<FKyaDiscFile> Files;
    if (KyaIso::ReadDirectory(Disc.SourceFilename, Files, InventoryError)) Disc.Files = MoveTemp(Files);
    Disc.MarkPackageDirty();
    Report = FString::Printf(TEXT("Exported %d bank(s) to %s.\nOriginal image: %s\nSave All to retain the updated source hashes."),
        Changed.Num(), *Disc.SourceFilename, *Backup);
    return true;
}
