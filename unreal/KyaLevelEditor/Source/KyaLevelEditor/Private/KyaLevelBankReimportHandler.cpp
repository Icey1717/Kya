#include "KyaLevelBankReimportHandler.h"

#include "EditorFramework/AssetImportData.h"
#include "KyaActorEditService.h"
#include "KyaLevelBank.h"
#include "KyaDisc.h"
#include "KyaIso.h"
#include "KyaDiscWorkflow.h"
#include "Misc/SecureHash.h"
#include "KyaLevelBankFactory.h"
#include "Misc/App.h"
#include "Misc/FileHelper.h"
#include "Misc/MessageDialog.h"
#include "Misc/PackageName.h"

namespace
{
void CopyTypedData(UKyaLevelBank& Target, const UKyaLevelBank& Source)
{
    Target.ImportSchemaVersion = Source.ImportSchemaVersion;
    Target.SourceFilename = Source.SourceFilename;
    Target.bHasLights = Source.bHasLights;
    Target.Lights = Source.Lights;
    Target.EntryCount = Source.EntryCount;
    Target.Entries = Source.Entries;
    Target.SceneConfiguration = Source.SceneConfiguration;
    Target.SceneEnvironment = Source.SceneEnvironment;
    Target.SceneMeshEntry = Source.SceneMeshEntry;
    Target.SceneTextureEntry = Source.SceneTextureEntry;
    Target.BackgroundMeshEntry = Source.BackgroundMeshEntry;
    Target.BackgroundTextureEntry = Source.BackgroundTextureEntry;
    Target.BankBytes = Source.BankBytes;
    Target.bHasImportedBaseline = Source.bHasImportedBaseline;
    Target.ImportedActors = Source.ImportedActors;
    Target.ImportedWaypoints = Source.ImportedWaypoints;
    Target.ImportedPaths = Source.ImportedPaths;
}
}

bool FKyaLevelBankReimportHandler::CanReimport(UObject* Obj, TArray<FString>& OutFilenames)
{
    UKyaLevelBank* Bank = Cast<UKyaLevelBank>(Obj);
    if (!Bank) return false;
    if (!Bank->SourceDisc.IsNull())
    {
        UKyaDisc* Disc = Bank->SourceDisc.LoadSynchronous();
        if (!Disc) return false;
        OutFilenames.Add(Disc->SourceFilename);
        return true;
    }
    FString Filename;
    if (Bank->AssetImportData) Filename = Bank->AssetImportData->GetFirstFilename();
    if (Filename.IsEmpty()) Filename = Bank->SourceFilename;
    if (Filename.IsEmpty()) return false;
    OutFilenames.Add(Filename);
    return true;
}

void FKyaLevelBankReimportHandler::SetReimportPaths(UObject* Obj, const TArray<FString>& NewReimportPaths)
{
    UKyaLevelBank* Bank = Cast<UKyaLevelBank>(Obj);
    if (!Bank || NewReimportPaths.IsEmpty()) return;
    if (!Bank->SourceDisc.IsNull())
    {
        if (UKyaDisc* Disc = Bank->SourceDisc.LoadSynchronous())
        {
            Disc->Modify();
            Disc->SourceFilename = FPaths::ConvertRelativePathToFull(NewReimportPaths[0]);
            Disc->MarkPackageDirty();
        }
        return;
    }
    if (!Bank->AssetImportData) Bank->AssetImportData = NewObject<UAssetImportData>(Bank);
    Bank->AssetImportData->Update(NewReimportPaths[0]);
    Bank->SourceFilename = NewReimportPaths[0];
}

EReimportResult::Type FKyaLevelBankReimportHandler::Reimport(UObject* Obj)
{
    UKyaLevelBank* Bank = Cast<UKyaLevelBank>(Obj);
    if (!Bank) return EReimportResult::Failed;
    UKyaDisc* Disc = Bank->SourceDisc.LoadSynchronous();
    if (!Bank->SourceDisc.IsNull() && !Disc) return EReimportResult::Failed;

    FString Filename;
    if (Bank->AssetImportData) Filename = Bank->AssetImportData->GetFirstFilename();
    if (Filename.IsEmpty()) Filename = Bank->SourceFilename;
    if (Disc) Filename = Disc->SourceFilename;
    if (Filename.IsEmpty() || !FPaths::FileExists(Filename))
    {
        UE_LOG(LogTemp, Error, TEXT("KYA_REIMPORT missing source file for %s"), *Bank->GetPathName());
        return EReimportResult::Failed;
    }

    // Local edits are never discarded silently.
    UPackage* Package = Bank->GetOutermost();
    if (Package && Package->IsDirty())
    {
        if (FApp::IsUnattended())
        {
            UE_LOG(LogTemp, Error,
                TEXT("KYA_REIMPORT %s has unexported changes; save or discard them before reimporting."),
                *Bank->GetPathName());
            return EReimportResult::Cancelled;
        }
        const EAppReturnType::Type Choice = FMessageDialog::Open(EAppMsgType::YesNo,
            FText::Format(NSLOCTEXT("KyaLevelEditor", "ReimportDirty",
                "{0} has changes that have not been saved or exported.\n\nDiscard those changes and reimport the source bank?"),
                FText::FromString(Bank->GetName())));
        if (Choice != EAppReturnType::Yes) return EReimportResult::Cancelled;
    }

    TArray<uint8> Bytes;
    FString ReadError;
    if (!(Disc ? KyaIso::Extract(Filename, Bank->SourceDiscPath, Bytes, ReadError) : FFileHelper::LoadFileToArray(Bytes, *Filename)))
    {
        UE_LOG(LogTemp, Error, TEXT("KYA_REIMPORT could not read %s"), *Filename);
        return EReimportResult::Failed;
    }

    // Parse into a temporary asset first: a failed parse must leave the old data intact.
    UKyaLevelBankFactory* Factory = NewObject<UKyaLevelBankFactory>();
    const uint8* Cursor = Bytes.GetData();
    UKyaLevelBank* Parsed = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
        GetTransientPackage(), NAME_None, RF_Transient, nullptr, TEXT("bnk"), Cursor,
        Bytes.GetData() + Bytes.Num(), GWarn));
    if (!Parsed)
    {
        UE_LOG(LogTemp, Error, TEXT("KYA_REIMPORT %s could not be parsed; keeping the previous data."), *Filename);
        return EReimportResult::Failed;
    }

    Bank->Modify();
    CopyTypedData(*Bank, *Parsed);
    Bank->SourceFilename = Disc ? Bank->SourceDiscPath : Filename;
    if (Disc) Bank->SourceDiscHash = FMD5::HashBytes(Bytes.GetData(), Bytes.Num());
    if (Disc) Bank->SourceDiscStateHash = KyaDiscBankStateHash(*Bank);
    if (!Bank->AssetImportData) Bank->AssetImportData = NewObject<UAssetImportData>(Bank);
    Bank->AssetImportData->Update(Filename);
    // Replacing the typed records invalidates every editor record key.
    Bank->BumpEditGeneration();
    Bank->MarkPackageDirty();
    FKyaLevelEditService::Get().NotifyBankReloaded(Bank);
    UE_LOG(LogTemp, Display, TEXT("KYA_REIMPORT %s from %s"), *Bank->GetPathName(), *Filename);
    return EReimportResult::Succeeded;
}
