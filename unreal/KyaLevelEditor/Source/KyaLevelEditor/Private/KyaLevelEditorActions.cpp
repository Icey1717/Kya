#include "KyaLevelEditorActions.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetToolsModule.h"
#include "AssetImportTask.h"
#include "Editor.h"
#include "Editor/EditorEngine.h"
#include "FileHelpers.h"
#include "Engine/Selection.h"
#include "EngineUtils.h"
#include "IAssetTools.h"
#include "KyaBankExport.h"
#include "KyaActorCatalog.h"
#include "KyaActorGroup.h"
#include "KyaActorHandle.h"
#include "KyaHierarchyHandle.h"
#include "KyaLevelBank.h"
#include "KyaLevelPreview.h"
#include "KyaLevelSet.h"
#include "KyaLevelSetFactory.h"
#include "KyaMessageLog.h"
#include "KyaSceneVisualizer.h"
#include "KyaSpatialRecordHandle.h"
#include "Logging/MessageLog.h"
#include "Misc/FileHelper.h"
#include "Misc/MessageDialog.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"
#include "Misc/ScopedSlowTask.h"
#include "UObject/SavePackage.h"

FString FKyaBankValidationReport::Summary() const
{
    TArray<FString> Lines;
    Lines.Add(FString::Printf(TEXT("%s: %s"), *AssetPath,
        bBytesBuilt ? TEXT("exportable") : TEXT("not exportable")));
    if (!UnsupportedReason.IsEmpty()) Lines.Add(FString::Printf(TEXT("  %s"), *UnsupportedReason));
    for (const FString& Warning : Warnings) Lines.Add(FString::Printf(TEXT("  warning: %s"), *Warning));
    return FString::Join(Lines, TEXT("\n"));
}

FString FKyaSectorDiscoveryResult::Summary() const
{
    TArray<FString> Lines = Messages;
    Lines.Add(FString::Printf(TEXT("reused=%d imported=%d ambiguous=%d missing=%d"),
        Reused, Imported, Ambiguous, MissingIds.Num()));
    return FString::Join(Lines, TEXT("\n"));
}

namespace
{
bool IsSectorBankFilename(const FString& Filename, int32& OutSectorId, FString& OutDigits)
{
    FString Base = FPaths::GetBaseFilename(Filename);
    if (!Base.RemoveFromStart(TEXT("SECT"), ESearchCase::IgnoreCase)) return false;
    if (Base.IsEmpty() || !Base.IsNumeric()) return false;
    OutDigits = Base;
    OutSectorId = FCString::Atoi(*Base);
    return true;
}

UKyaLevelBank* FindBankForSourceFile(const FString& SourceFile)
{
    const FString Normalized = FPaths::ConvertRelativePathToFull(SourceFile);
    FAssetRegistryModule& RegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
    TArray<FAssetData> Assets;
    RegistryModule.Get().GetAssetsByClass(UKyaLevelBank::StaticClass()->GetClassPathName(), Assets, true);
    for (const FAssetData& Asset : Assets)
    {
        UKyaLevelBank* Bank = Cast<UKyaLevelBank>(Asset.GetAsset());
        if (!Bank || Bank->SourceFilename.IsEmpty()) continue;
        if (FPaths::ConvertRelativePathToFull(Bank->SourceFilename).Equals(Normalized, ESearchCase::IgnoreCase))
            return Bank;
    }
    return nullptr;
}

int32 GetBankSectorId(const UKyaLevelBank& Bank)
{
    FString Name = FPaths::GetBaseFilename(Bank.SourceFilename);
    if (!Name.RemoveFromStart(TEXT("SECT"), ESearchCase::IgnoreCase))
    {
        Name = Bank.GetName();
        if (!Name.RemoveFromStart(TEXT("SECT"), ESearchCase::IgnoreCase)) return INDEX_NONE;
    }
    return Name.IsNumeric() ? FCString::Atoi(*Name) : INDEX_NONE;
}

FString LevelExportDirectory(const TArray<UKyaLevelBank*>& Banks){
    FString LevelName = TEXT("KyaLevel");
    for (const UKyaLevelBank* Bank : Banks)
        if (Bank && Bank->Entries.ContainsByPredicate([](const FKyaBankEntry& Entry) {
                return Entry.Type == 6 && Entry.Subtype == 2; }))
        {
            LevelName = Bank->GetName();
            break;
        }
    return FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("KyaExport"), LevelName);
}
}

TArray<AKyaLevelPreview*> KyaGetSelectedPreviews()
{
    TArray<AKyaLevelPreview*> Previews;
    if (!GEditor) return Previews;
    TArray<AActor*> SelectedActors;
    GEditor->GetSelectedActors()->GetSelectedObjects(SelectedActors);
    for (AActor* Actor : SelectedActors)
        if (AKyaLevelPreview* Preview = Cast<AKyaLevelPreview>(Actor)) Previews.AddUnique(Preview);
    return Previews;
}

TArray<AKyaLevelPreview*> KyaGetTargetPreviews()
{
    TArray<AKyaLevelPreview*> Previews = KyaGetSelectedPreviews();
    if (!Previews.IsEmpty() || !GEditor) return Previews;
    if (UWorld* World = GEditor->GetEditorWorldContext().World())
        for (TActorIterator<AKyaLevelPreview> It(World); It; ++It) Previews.Add(*It);
    return Previews;
}

bool KyaIsGeneratedActor(const AActor* Actor)
{
    return Cast<AKyaActorHandle>(Actor) || Cast<AKyaSpatialRecordHandle>(Actor) ||
        Cast<AKyaActorGroup>(Actor) || Cast<AKyaHierarchyHandle>(Actor) ||
        Cast<AKyaSceneVisualizer>(Actor);
}

bool KyaContainsGeneratedActor(const TArray<AActor*>& Actors)
{
    for (const AActor* Actor : Actors)
        if (KyaIsGeneratedActor(Actor)) return true;
    return false;
}

bool KyaSelectionContainsGeneratedActor()
{
    if (!GEditor) return false;
    TArray<AActor*> Selected;
    GEditor->GetSelectedActors()->GetSelectedObjects(Selected);
    return KyaContainsGeneratedActor(Selected);
}

int32 KyaDestroyStrayGeneratedActors(UWorld* World)
{
    if (!World) return 0;
    int32 Destroyed = 0;
    for (TActorIterator<AKyaLevelPreview> It(World); It; ++It)
        Destroyed += It->DestroyStrayGeneratedActors();

    TArray<AActor*> Orphans;
    for (TActorIterator<AActor> It(World); It; ++It)
    {
        if (!KyaIsGeneratedActor(*It)) continue;
        if (const AKyaActorHandle* Handle = Cast<AKyaActorHandle>(*It))
        {
            if (!IsValid(Handle->Preview)) Orphans.Add(*It);
        }
        else if (const AKyaSpatialRecordHandle* Spatial = Cast<AKyaSpatialRecordHandle>(*It))
        {
            if (!IsValid(Spatial->Preview)) Orphans.Add(*It);
        }
        else if (const AKyaHierarchyHandle* Hierarchy = Cast<AKyaHierarchyHandle>(*It))
        {
            if (!IsValid(Hierarchy->Preview)) Orphans.Add(*It);
        }
        else if (const AKyaSceneVisualizer* Visualizer = Cast<AKyaSceneVisualizer>(*It))
        {
            if (!IsValid(Visualizer->Preview)) Orphans.Add(*It);
        }
        else if (const AKyaActorGroup* Group = Cast<AKyaActorGroup>(*It))
        {
            if (!Cast<AKyaLevelPreview>(Group->GetAttachParentActor())) Orphans.Add(*It);
        }
    }
    for (AActor* Orphan : Orphans) Orphan->Destroy();
    return Destroyed + Orphans.Num();
}

namespace
{
void CollectSelectionPreviews(TArray<AKyaLevelPreview*>& OutPreviews)
{
    if (!GEditor) return;
    TArray<AActor*> Selected;
    GEditor->GetSelectedActors()->GetSelectedObjects(Selected);
    for (AActor* Actor : Selected)
    {
        AKyaLevelPreview* Preview = Cast<AKyaLevelPreview>(Actor);
        if (!Preview)
        {
            if (const AKyaActorHandle* Handle = Cast<AKyaActorHandle>(Actor))
                Preview = Handle->Preview.Get();
            else if (const AKyaSpatialRecordHandle* Spatial = Cast<AKyaSpatialRecordHandle>(Actor))
                Preview = Spatial->Preview.Get();
            else if (const AKyaHierarchyHandle* Hierarchy = Cast<AKyaHierarchyHandle>(Actor))
                Preview = Hierarchy->Preview.Get();
            else if (const AKyaActorGroup* Group = Cast<AKyaActorGroup>(Actor))
                Preview = Cast<AKyaLevelPreview>(Group->GetAttachParentActor());
        }
        if (Preview) OutPreviews.AddUnique(Preview);
    }
}
}

void KyaSetSelectedGeneratedActorsHidden(bool bHidden, int32& OutAffected)
{
    OutAffected = 0;
    if (!GEditor) return;
    TArray<AActor*> Selected;
    GEditor->GetSelectedActors()->GetSelectedObjects(Selected);
    TArray<AKyaLevelPreview*> Previews;
    CollectSelectionPreviews(Previews);
    for (AKyaLevelPreview* Preview : Previews)
    {
        for (AActor* Actor : Selected)
        {
            if (!Preview->OwnsGeneratedActor(Actor)) continue;
            Preview->SetGeneratedActorTemporarilyHidden(Actor, bHidden);
            ++OutAffected;
        }
    }
}

void KyaIsolateSelectedGeneratedActors(int32& OutAffected)
{
    OutAffected = 0;
    if (!GEditor) return;
    TArray<AActor*> Selected;
    GEditor->GetSelectedActors()->GetSelectedObjects(Selected);
    TArray<AKyaLevelPreview*> Previews;
    CollectSelectionPreviews(Previews);
    for (AKyaLevelPreview* Preview : Previews)
    {
        TArray<AActor*> OwnedSelection;
        for (AActor* Actor : Selected)
            if (Preview->OwnsGeneratedActor(Actor)) OwnedSelection.Add(Actor);
        if (OwnedSelection.IsEmpty()) continue;
        Preview->ClearGeneratedActorIsolation();
        Preview->IsolateGeneratedActors(OwnedSelection);
        OutAffected += OwnedSelection.Num();
    }
}

void KyaClearGeneratedActorIsolation()
{
    if (!GEditor) return;
    UWorld* World = GEditor->GetEditorWorldContext().World();
    if (!World) return;
    for (TActorIterator<AKyaLevelPreview> It(World); It; ++It)
        It->ClearGeneratedActorIsolation();
}

void KyaSetGeneratedActorSelectionLocked(bool bLocked)
{
    if (!GEditor) return;
    UWorld* World = GEditor->GetEditorWorldContext().World();
    if (!World) return;
    for (TActorIterator<AKyaLevelPreview> It(World); It; ++It)
    {
        It->Modify();
        It->bLockGeneratedActorSelection = bLocked;
    }
}

TArray<UKyaLevelBank*> KyaCollectLevelBanks(const TArray<AKyaLevelPreview*>& Previews)
{
    TArray<UKyaLevelBank*> Banks;
    for (AKyaLevelPreview* Preview : Previews)
    {
        if (!Preview) continue;
        if (Preview->Bank) Banks.AddUnique(Preview->Bank);
        for (const TObjectPtr<UKyaLevelBank>& Sector : Preview->SectorBanks)
            if (Sector) Banks.AddUnique(Sector);
    }
    return Banks;
}

UKyaLevelSet* KyaCreateLevelSetFromPreview(AKyaLevelPreview* Preview, FString& OutError)
{
    if (!Preview || !Preview->Bank)
    {
        OutError = TEXT("Select a level preview with a main bank first.");
        return nullptr;
    }
    UKyaLevelBank* MainBank = Preview->Bank;
    const FString PackagePath = FString::Printf(TEXT("/Game/KyaLevels/%s"), *MainBank->GetName());
    const FString BaseAssetName = FString::Printf(TEXT("%sLevelSet"), *MainBank->GetName());
    UKyaLevelSet* LevelSet = nullptr;
    for (int32 Suffix = 1; Suffix <= 32 && !LevelSet; ++Suffix)
    {
        const FString AssetName = Suffix == 1 ? BaseAssetName : FString::Printf(TEXT("%s%d"), *BaseAssetName, Suffix);
        const FString ObjectPath = FString::Printf(TEXT("%s/%s.%s"), *PackagePath, *AssetName, *AssetName);
        if (UKyaLevelSet* Existing = LoadObject<UKyaLevelSet>(nullptr, *ObjectPath))
        {
            // Two levels can have identically named main banks. Never adopt or
            // overwrite a descriptor that belongs to another bank.
            if (!Existing->MainBank || Existing->MainBank == MainBank) LevelSet = Existing;
            continue;
        }
        UKyaLevelSetFactory* Factory = NewObject<UKyaLevelSetFactory>();
        IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
        LevelSet = Cast<UKyaLevelSet>(AssetTools.CreateAsset(AssetName, PackagePath,
            UKyaLevelSet::StaticClass(), Factory));
    }
    if (!LevelSet)
    {
        OutError = TEXT("Could not create a level set asset under /Game/KyaLevels.");
        return nullptr;
    }

    LevelSet->Modify();
    LevelSet->DescriptorVersion = UKyaLevelSet::CurrentDescriptorVersion;
    LevelSet->MainBank = MainBank;
    LevelSet->SourceRoot = FPaths::GetPath(MainBank->SourceFilename);
    LevelSet->Sectors.Reset();
    for (const TObjectPtr<UKyaLevelBank>& SectorBank : Preview->SectorBanks)
        if (SectorBank) LevelSet->SetSector(GetBankSectorId(*SectorBank), SectorBank, SectorBank->SourceFilename, false);
    ++LevelSet->ImportRevision;
    LevelSet->MarkPackageDirty();

    Preview->Modify();
    Preview->LevelSet = LevelSet;
    Preview->ApplyLevelSet();
    Preview->RebuildFromBank();
    UE_LOG(LogTemp, Display, TEXT("KYA_LEVEL_SET %s main=%s sectors=%d"), *LevelSet->GetPathName(),
        *MainBank->GetName(), LevelSet->Sectors.Num());
    return LevelSet;
}

void KyaSyncLevelSetFromPreview(UKyaLevelSet& LevelSet, AKyaLevelPreview& Preview, const FString& SourceDirectory)
{
    LevelSet.Modify();
    LevelSet.MainBank = Preview.Bank;
    if (!SourceDirectory.IsEmpty()) LevelSet.SourceRoot = SourceDirectory;

    TArray<FKyaLevelSetSector> Preserved;
    TSet<int32> ManualIds;
    TSet<const UKyaLevelBank*> ManualBanks;
    for (const FKyaLevelSetSector& Sector : LevelSet.Sectors)
    {
        if (!Sector.bManualOverride || !Sector.Bank) continue;
        Preserved.Add(Sector);
        ManualIds.Add(Sector.SectorId);
        ManualBanks.Add(Sector.Bank);
    }
    // Manual associations are authoritative for their sector ID, and their bank must
    // not also be added under its filename-derived ID.
    LevelSet.Sectors = Preserved;
    for (const TObjectPtr<UKyaLevelBank>& SectorBank : Preview.SectorBanks)
    {
        if (!SectorBank) continue;
        if (ManualBanks.Contains(SectorBank.Get())) continue;
        const int32 SectorId = GetBankSectorId(*SectorBank);
        if (ManualIds.Contains(SectorId)) continue;
        LevelSet.SetSector(SectorId, SectorBank, SectorBank->SourceFilename, false);
    }
    ++LevelSet.ImportRevision;
    LevelSet.MarkPackageDirty();
}

FKyaBankValidationReport KyaValidateBank(UKyaLevelBank* Bank){
    FKyaBankValidationReport Report;
    Report.Bank = Bank;
    if (!Bank)
    {
        Report.UnsupportedReason = TEXT("The bank asset does not exist.");
        Report.Issues.Add({INDEX_NONE, TEXT("bank"), TEXT("The bank asset does not exist."), true});
        return Report;
    }
    Report.AssetPath = Bank->GetPathName();
    if (!Bank->HasCurrentImportSchema())
    {
        Report.Issues.Add({INDEX_NONE, TEXT("schema"), FString::Printf(
            TEXT("Stale import schema (bank %d, editor %d): reimport the source bank before editing or exporting."),
            Bank->ImportSchemaVersion, UKyaLevelBank::CurrentImportSchemaVersion), false});
    }
    if (Bank->BankBytes.IsEmpty())
    {
        Report.Issues.Add({INDEX_NONE, TEXT("geometry"), TEXT(
            "No bank bytes are retained, so geometry, textures and records cannot be decoded or exported."), true});
    }
    else if (!Bank->Entries.IsValidIndex(Bank->BackgroundMeshEntry))
    {
        Report.Issues.Add({INDEX_NONE, TEXT("geometry"), TEXT(
            "The scene background entry is missing or invalid; world geometry preview is incomplete."), false});
    }
    for (int32 ActorIndex = 0; ActorIndex < Bank->SceneEnvironment.Actors.Num(); ++ActorIndex)
    {
        for (const FKyaActorReferenceIssue& Issue : ValidateKyaActorReferences(
            *Bank, Bank->SceneEnvironment.Actors[ActorIndex]))
        {
            Report.Issues.Add({ActorIndex, Issue.Field, Issue.Message, Issue.bBlocking});
            if (Issue.bBlocking && Report.UnsupportedReason.IsEmpty())
                Report.UnsupportedReason = Issue.Message;
        }
    }
    FString Reason;
    Report.bExportSupported = KyaBankSupportsExport(Bank, Reason);
    if (!Reason.IsEmpty()) Report.UnsupportedReason = Reason;
    if (!Report.bExportSupported) return Report;
    FKyaBankExportDiagnostics Diagnostics;
    TArray<uint8> Bytes;
    Report.bBytesBuilt = KyaBuildBankExportBytes(Bank, Bytes, Diagnostics);
    Report.Warnings = Diagnostics.Warnings;
    for (const FString& Warning : Diagnostics.Warnings)
        Report.Issues.Add({INDEX_NONE, TEXT("export"), Warning, false});
    if (!Report.bBytesBuilt)
    {
        if (Diagnostics.Errors.IsEmpty() && Diagnostics.FailedActorIndex != INDEX_NONE)
        {
            Diagnostics.Errors.Add(FString::Printf(TEXT("The bank writer rejected actor %d (%s)."),
                Diagnostics.FailedActorIndex,
                Diagnostics.FailedProperty.IsEmpty() ? TEXT("record") : *Diagnostics.FailedProperty));
        }
        const bool bHasActorContext = Diagnostics.FailedActorIndex != INDEX_NONE && !Diagnostics.Errors.IsEmpty();
        for (int32 ErrorIndex = 0; ErrorIndex < Diagnostics.Errors.Num(); ++ErrorIndex)
        {
            const FString& Error = Diagnostics.Errors[ErrorIndex];
            Report.Warnings.Add(Error);
            if (bHasActorContext && ErrorIndex == 0)
            {
                // Attach the writer's rejection to the offending record so the Message
                // Log can select the actor and show the property path.
                Report.Issues.Add({Diagnostics.FailedActorIndex, Diagnostics.FailedProperty, Error, true});
            }
            else
            {
                Report.Issues.Add({INDEX_NONE, TEXT("export"), Error, true});
            }
        }
        Report.UnsupportedReason = Diagnostics.Errors.IsEmpty() ? TEXT("The bank writer rejected this bank.") : Diagnostics.Errors[0];
    }
    return Report;
}

TArray<FKyaBankValidationReport> KyaValidateLevelBanks(const TArray<UKyaLevelBank*>& Banks)
{
    TArray<FKyaBankValidationReport> Reports;
    FMessageLog Log("KyaLevelEditor");
    bool bAnyFailure = false;
    for (UKyaLevelBank* Bank : Banks)
    {
        Reports.Add(KyaValidateBank(Bank));
        const FKyaBankValidationReport& Report = Reports.Last();
        const bool bBlockingIssue = Report.Issues.ContainsByPredicate(
            [](const FKyaValidationIssue& Issue) { return Issue.bBlocking; });
        // Every entry carries a clickable bank asset token; issue entries add a
        // "Select actor" action for the affected generated representation.
        if (Report.bBytesBuilt && !bBlockingIssue)
        {
            Log.AddMessage(KyaMakeBankMessage(EMessageSeverity::Info, Bank,
                FText::FromString(TEXT("is exportable."))));
        }
        else
        {
            bAnyFailure = true;
            const FString Reason = !Report.UnsupportedReason.IsEmpty()
                ? Report.UnsupportedReason
                : FString(TEXT("blocking validation issues must be fixed first."));
            Log.AddMessage(KyaMakeBankMessage(EMessageSeverity::Error, Bank,
                FText::FromString(FString::Printf(TEXT("cannot be exported: %s"), *Reason))));
        }
        for (const FKyaValidationIssue& Issue : Report.Issues)
        {
            const EMessageSeverity::Type Severity = Issue.bBlocking
                ? EMessageSeverity::Error : EMessageSeverity::Warning;
            Log.AddMessage(KyaMakeValidationIssueMessage(Severity, Bank, Issue.ActorIndex, Issue.Field,
                FText::FromString(Issue.Message)));
        }
        UE_LOG(LogTemp, Display, TEXT("KYA_VALIDATE %s"), *Report.Summary());
    }
    if (bAnyFailure) Log.Open(EMessageSeverity::Error);
    return Reports;
}

FKyaSectorDiscoveryResult KyaDiscoverSectors(AKyaLevelPreview* Preview, bool bImportMissing)
{
    FKyaSectorDiscoveryResult Result;
    if (!Preview || !Preview->Bank)
    {
        Result.Messages.Add(TEXT("Select a level preview with a main bank first."));
        return Result;
    }
    UKyaLevelBank* MainBank = Preview->Bank;
    if (MainBank->SourceFilename.IsEmpty())
    {
        Result.Messages.Add(FString::Printf(TEXT("%s has no source filename; import it from a .bnk file first."),
            *MainBank->GetName()));
        return Result;
    }
    const FString SourceDirectory = FPaths::GetPath(MainBank->SourceFilename);
    if (SourceDirectory.IsEmpty() || !FPaths::DirectoryExists(SourceDirectory))
    {
        Result.Messages.Add(FString::Printf(TEXT("Source directory %s does not exist."), *SourceDirectory));
        return Result;
    }
    const auto SameSourcePath = [](const FString& A, const FString& B)
    {
        FString NormalizedA = A;
        FString NormalizedB = B;
        FPaths::NormalizeFilename(NormalizedA);
        FPaths::NormalizeFilename(NormalizedB);
        return FPaths::ConvertRelativePathToFull(NormalizedA).Equals(
            FPaths::ConvertRelativePathToFull(NormalizedB), ESearchCase::IgnoreCase);
    };

    TArray<FString> Files;
    IFileManager::Get().FindFiles(Files, *(FPaths::Combine(SourceDirectory, TEXT("SECT*.BNK"))), true, false);
    struct FCandidate
    {
        int32 SectorId = INDEX_NONE;
        FString Filename;
        FString Digits;
    };
    TArray<FCandidate> Candidates;
    for (const FString& File : Files)
    {
        FCandidate Candidate;
        if (!IsSectorBankFilename(File, Candidate.SectorId, Candidate.Digits)) continue;
        Candidate.Filename = FPaths::Combine(SourceDirectory, File);
        Candidates.Add(Candidate);
    }
    Candidates.Sort([](const FCandidate& A, const FCandidate& B) { return A.SectorId < B.SectorId; });

    // Sector IDs must be unambiguous: SECT1 and SECT01 are not safe to merge silently.
    TMap<int32, TArray<FString>> FilesBySector;
    for (const FCandidate& Candidate : Candidates) FilesBySector.FindOrAdd(Candidate.SectorId).Add(Candidate.Filename);

    const FString DestinationPath = FString::Printf(TEXT("/Game/KyaLevels/%s"), *MainBank->GetName());
    TArray<int32> SectorIds;
    FilesBySector.GetKeys(SectorIds);
    SectorIds.Sort();
    FScopedSlowTask SlowTask(float(SectorIds.Num()),
        FText::FromString(FString::Printf(TEXT("Discovering %s sectors"), *MainBank->GetName())));
    SlowTask.MakeDialog(true);
    bool bCanceled = false;
    for (const int32 SectorId : SectorIds)
    {
        if (SlowTask.ShouldCancel())
        {
            bCanceled = true;
            Result.Messages.Add(TEXT("Sector discovery was cancelled; existing associations were kept."));
            break;
        }
        SlowTask.EnterProgressFrame(1.0f, FText::FromString(FString::Printf(TEXT("Sector %d"), SectorId)));
        const TArray<FString>& SectorFiles = FilesBySector.FindChecked(SectorId);
        if (SectorFiles.Num() > 1)
        {
            ++Result.Ambiguous;
            Result.Messages.Add(FString::Printf(TEXT("Sector %d is ambiguous: %s"), SectorId,
                *FString::Join(SectorFiles, TEXT(", "))));
            continue;
        }
        const FString& Filename = SectorFiles[0];
        UKyaLevelBank* Existing = FindBankForSourceFile(Filename);
        if (!Existing)
        {
            for (const TObjectPtr<UKyaLevelBank>& Assigned : Preview->SectorBanks)
            {
                if (!Assigned || Assigned->SourceFilename.IsEmpty()) continue;
                if (FPaths::ConvertRelativePathToFull(Assigned->SourceFilename).Equals(
                    FPaths::ConvertRelativePathToFull(Filename), ESearchCase::IgnoreCase))
                {
                    Existing = Assigned;
                    break;
                }
            }
        }
        if (Existing)
        {
            ++Result.Reused;
            if (!Preview->SectorBanks.Contains(Existing)) Preview->SectorBanks.Add(Existing);
            continue;
        }
        if (!bImportMissing)
        {
            Result.MissingIds.Add(SectorId);
            continue;
        }
        const FString DestinationName = FPaths::GetBaseFilename(Filename).ToUpper();
        const FString ObjectPath = FString::Printf(TEXT("%s/%s.%s"), *DestinationPath,
            *DestinationName, *DestinationName);
        // Never adopt an unrelated asset that happens to occupy the destination path:
        // two levels can have identically named main banks, and a skipped import must
        // not silently associate the wrong sector.
        if (UKyaLevelBank* Occupied = LoadObject<UKyaLevelBank>(nullptr, *ObjectPath))
        {
            if (!SameSourcePath(Occupied->SourceFilename, Filename))
            {
                ++Result.Ambiguous;
                Result.MissingIds.Add(SectorId);
                Result.Messages.Add(FString::Printf(
                    TEXT("Destination %s already contains a bank from %s; %s was not imported."),
                    *ObjectPath, *Occupied->SourceFilename, *Filename));
                continue;
            }
        }
        else
        {
            UAssetImportTask* Task = NewObject<UAssetImportTask>();
            Task->Filename = Filename;
            Task->DestinationPath = DestinationPath;
            Task->DestinationName = DestinationName;
            Task->bAutomated = true;
            Task->bReplaceExisting = false;
            Task->bSave = false;
            Task->bAsync = false;
            IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
            AssetTools.ImportAssetTasks({Task});
        }
        UKyaLevelBank* Imported = LoadObject<UKyaLevelBank>(nullptr, *ObjectPath);
        if (!Imported || !SameSourcePath(Imported->SourceFilename, Filename))
        {
            Result.MissingIds.Add(SectorId);
            Result.Messages.Add(FString::Printf(TEXT("Failed to import %s (destination provenance does not match)."),
                *Filename));
            continue;
        }
        ++Result.Imported;
        if (!Preview->SectorBanks.Contains(Imported)) Preview->SectorBanks.Add(Imported);
    }

    // Actor sector references suggest content that should exist but has no file.
    TSet<int32> DiscoveredIds;
    for (const TPair<int32, TArray<FString>>& Pair : FilesBySector) DiscoveredIds.Add(Pair.Key);
    TSet<int32> ReferencedIds;
    for (const FKyaActorPlacement& Actor : MainBank->SceneEnvironment.Actors)
        if (Actor.SectorId >= 0) ReferencedIds.Add(Actor.SectorId);
    for (const int32 Referenced : ReferencedIds)
        if (!DiscoveredIds.Contains(Referenced) && !Result.MissingIds.Contains(Referenced))
        {
            Result.MissingIds.Add(Referenced);
            Result.Messages.Add(FString::Printf(
                TEXT("Actor placements reference sector %d but no SECT%d.BNK exists."), Referenced, Referenced));
        }

    if (Preview->Bank) Preview->SectorBanks.RemoveAll([](const TObjectPtr<UKyaLevelBank>& Bank) { return !Bank; });

    // Keep the persisted descriptor in sync, preserving explicit manual associations.
    if (UKyaLevelSet* LevelSet = Preview->LevelSet)
        KyaSyncLevelSetFromPreview(*LevelSet, *Preview, SourceDirectory);

    Preview->RebuildFromBank();

    FMessageLog Log("KyaLevelEditor");
    Log.AddMessage(KyaMakeActorMessage(EMessageSeverity::Info, MainBank, Preview,
        FText::FromString(Result.Summary())));
    for (const FString& Message : Result.Messages)
        Log.AddMessage(KyaMakeActorMessage(EMessageSeverity::Warning, MainBank, Preview,
            FText::FromString(Message)));
    for (const int32 Missing : Result.MissingIds)
    {
        Log.AddMessage(KyaMakeActorMessage(EMessageSeverity::Error, MainBank, Preview, FText::FromString(
            FString::Printf(TEXT("Missing sector bank for sector %d (SECT%d.BNK): import it and rerun "
                "Discover Sectors, or associate an existing bank manually."), Missing, Missing))));
    }
    if (!Result.MissingIds.IsEmpty() || Result.Ambiguous > 0) Log.Open(EMessageSeverity::Warning);
    if (bCanceled)
        Log.AddMessage(KyaMakeActorMessage(EMessageSeverity::Warning, MainBank, Preview,
            FText::FromString(TEXT("Sector discovery stopped early; rerun it to continue."))));

    UE_LOG(LogTemp, Display, TEXT("KYA_DISCOVER_SECTORS %s"), *Result.Summary());
    return Result;
}

void KyaSetAuthoringVisibility(bool bEnabled)
{
    for (AKyaLevelPreview* Preview : KyaGetTargetPreviews())
    {
        if (!Preview) continue;
        Preview->Modify();
        Preview->bAuthoringVisibility = bEnabled;
        // Enabling authoring also resumes the editor camera; otherwise the capture
        // lock would keep native culling active and the toggle would appear inert.
        if (bEnabled) Preview->ClearPreviewCamera();
        Preview->RebuildFromBank();
    }
}

void KyaClearPreviewCamera()
{
    for (AKyaLevelPreview* Preview : KyaGetTargetPreviews())
    {
        if (!Preview) continue;
        Preview->ClearPreviewCamera();
    }
}

bool KyaSaveLevelBanks(const TArray<UKyaLevelBank*>& Banks, FString& OutError)
{
    TArray<UPackage*> Packages;
    for (UKyaLevelBank* Bank : Banks)
    {
        if (!Bank) continue;
        Bank->MarkPackageDirty();
        Packages.AddUnique(Bank->GetOutermost());
    }
    if (Packages.IsEmpty())
    {
        OutError = TEXT("No banks to save.");
        return false;
    }
    const bool bSaved = UEditorLoadingAndSavingUtils::SavePackages(Packages, true);
    if (!bSaved) OutError = TEXT("Saving one or more bank packages failed.");
    return bSaved;
}

bool KyaExportLevelBanks(const TArray<UKyaLevelBank*>& Banks, const FString& DestinationDirectory,
    TArray<FString>& OutMessages)
{
    TArray<FKyaBankExportRequest> Requests;
    FMessageLog Log("KyaLevelEditor");
    bool bBlockingIssues = false;
    for (UKyaLevelBank* Bank : Banks)
    {
        if (!Bank) continue;
        // Report every finding by bank, actor identity and property path before any
        // output is replaced; the same entries select the actor so Details navigates.
        const FKyaBankValidationReport Report = KyaValidateBank(Bank);
        for (const FKyaValidationIssue& Issue : Report.Issues)
        {
            bBlockingIssues |= Issue.bBlocking;
            Log.AddMessage(KyaMakeValidationIssueMessage(Issue.bBlocking
                ? EMessageSeverity::Error : EMessageSeverity::Warning, Bank, Issue.ActorIndex,
                Issue.Field, FText::FromString(Issue.Message)));
        }
        FString Reason;
        if (!KyaBankSupportsExport(Bank, Reason))
        {
            // Discovered sector banks are dependencies whose typed layout the writer
            // does not support yet. Report them as unchanged instead of failing the
            // whole level export or rewriting them.
            OutMessages.Add(FString::Printf(TEXT("%s: unchanged dependency, not written (%s)"),
                *Bank->GetName(), *Reason));
            Log.AddMessage(KyaMakeBankMessage(EMessageSeverity::Warning, Bank, FText::FromString(
                FString::Printf(TEXT("Unchanged dependency: not written (%s)."), *Reason))));
            UE_LOG(LogTemp, Display, TEXT("KYA_EXPORT_SKIP %s %s"), *Bank->GetName(), *Reason);
            continue;
        }
        FKyaBankExportRequest& Request = Requests.AddDefaulted_GetRef();
        Request.Bank = Bank;
        Request.DestinationFilename = FPaths::Combine(DestinationDirectory, Bank->GetName() + TEXT(".bnk"));
    }
    if (bBlockingIssues)
    {
        OutMessages.Add(TEXT("Export stopped: blocking validation issues must be fixed first; no destination was replaced."));
        Log.AddMessage(KyaMakeBankMessage(EMessageSeverity::Error, nullptr, FText::FromString(
            TEXT("Export stopped before staging: fix the blocking issues above; no destination was replaced."))));
        Log.Open(EMessageSeverity::Error);
        return false;
    }
    if (Requests.IsEmpty())
    {
        OutMessages.Add(TEXT("No banks in this level can be exported by the current writer."));
        return false;
    }
    const TArray<FKyaBankExportOutcome> Outcomes = KyaWriteValidatedExportSet(Requests);
    bool bAllPublished = !Outcomes.IsEmpty();
    for (const FKyaBankExportOutcome& Outcome : Outcomes)
    {
        const TCHAR* State = Outcome.bPublished ? TEXT("exported") : TEXT("not exported");
        OutMessages.Add(FString::Printf(TEXT("%s: %s -> %s"), *GetNameSafe(Outcome.Bank), State,
            *Outcome.DestinationFilename));
        for (const FString& Error : Outcome.Errors) OutMessages.Add(FString::Printf(TEXT("  error: %s"), *Error));
        for (const FString& Warning : Outcome.Warnings) OutMessages.Add(FString::Printf(TEXT("  warning: %s"), *Warning));
        for (const FString& Edit : Outcome.SupportedEdits)
        {
            OutMessages.Add(FString::Printf(TEXT("  edits: %s"), *Edit));
            Log.AddMessage(KyaMakeBankMessage(EMessageSeverity::Info, Outcome.Bank,
                FText::FromString(Edit)));
        }
        if (!Outcome.bPublished) bAllPublished = false;
        UE_LOG(LogTemp, Display, TEXT("KYA_EXPORT %s %s"), *Outcome.DestinationFilename, State);
        if (Outcome.bPublished)
        {
            Log.AddMessage(KyaMakeBankMessage(EMessageSeverity::Info, Outcome.Bank, FText::FromString(
                FString::Printf(TEXT("Exported to %s."), *Outcome.DestinationFilename))));
        }
        else
        {
            Log.AddMessage(KyaMakeBankMessage(EMessageSeverity::Error, Outcome.Bank, FText::FromString(
                FString::Printf(TEXT("Was not exported to %s."), *Outcome.DestinationFilename))));
        }
        for (const FString& Error : Outcome.Errors)
            Log.AddMessage(KyaMakeBankMessage(EMessageSeverity::Error, Outcome.Bank,
                FText::FromString(Error)));
        for (const FString& Warning : Outcome.Warnings)
            Log.AddMessage(KyaMakeBankMessage(EMessageSeverity::Warning, Outcome.Bank,
                FText::FromString(Warning)));
    }
    if (!bAllPublished) Log.Open(EMessageSeverity::Error);
    return bAllPublished;
}
