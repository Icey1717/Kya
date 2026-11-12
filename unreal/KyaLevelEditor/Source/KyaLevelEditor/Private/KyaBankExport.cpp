#include "KyaBankExport.h"

#include "KyaLevelBank.h"
#include "KyaSectorBankExport.h"
#include "HAL/FileManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Misc/ScopedSlowTask.h"
#include "Misc/App.h"

FString FKyaBankExportDiagnostics::Summary() const
{
    TArray<FString> Lines;
    for (const FString& Error : Errors) Lines.Add(FString::Printf(TEXT("error: %s"), *Error));
    for (const FString& Warning : Warnings) Lines.Add(FString::Printf(TEXT("warning: %s"), *Warning));
    for (const FString& Edit : SupportedEdits) Lines.Add(FString::Printf(TEXT("supported edit: %s"), *Edit));
    return FString::Join(Lines, TEXT("\n"));
}

bool KyaBankSupportsExport(const UKyaLevelBank* Bank, FString& OutReason)
{
    OutReason.Reset();
    if (!Bank)
    {
        OutReason = TEXT("The bank asset does not exist.");
        return false;
    }
    if (!Bank->HasCurrentImportSchema())
    {
        OutReason = TEXT("The bank asset predates the current import schema; reimport it before exporting.");
        return false;
    }
    bool bSceneConfig = false;
    for (const FKyaBankEntry& Entry : Bank->Entries)
        if (Entry.Type == 6 && Entry.Subtype == 2) bSceneConfig = true;
    if (!bSceneConfig)
    {
        return KyaSectorBankSupportsExport(*Bank, OutReason);
    }
    return true;
}

TArray<FKyaBankExportOutcome> KyaWriteValidatedExportSet(const TArray<FKyaBankExportRequest>& Requests)
{
    TArray<FKyaBankExportOutcome> Outcomes;
    Outcomes.Reserve(Requests.Num());

    struct FStagedFile
    {
        TArray<uint8> Bytes;
        FString TemporaryFilename;
    };
    TArray<FStagedFile> Staged;
    TArray<int32> StagedOutcomeIndex;
    bool bStagingFailed = false;

    // Preflight and stage every bank before publishing any of them. Destinations must
    // be unique: publishing one file would otherwise move another bank's bytes.
    TSet<FString> Destinations;
    TUniquePtr<FScopedSlowTask> SlowTask;
    if (!FApp::IsUnattended())
    {
        SlowTask = MakeUnique<FScopedSlowTask>(float(Requests.Num()),
            NSLOCTEXT("KyaLevelEditor", "KyaExport", "Exporting Kya banks"));
        SlowTask->MakeDialog(true);
    }
    for (int32 RequestIndex = 0; RequestIndex < Requests.Num(); ++RequestIndex)
    {
        if (SlowTask) SlowTask->EnterProgressFrame();
        const FKyaBankExportRequest& Request = Requests[RequestIndex];
        FKyaBankExportOutcome& Outcome = Outcomes.AddDefaulted_GetRef();
        if (SlowTask && SlowTask->ShouldCancel())
        {
            Outcome.Errors.Add(TEXT("Export was cancelled before this bank was written."));
            bStagingFailed = true;
            continue;
        }
        Outcome.Bank = Request.Bank;
        Outcome.DestinationFilename = Request.DestinationFilename;

        if (Request.DestinationFilename.IsEmpty())
        {
            Outcome.Errors.Add(TEXT("No destination filename was supplied."));
            bStagingFailed = true;
            continue;
        }
        const FString Normalized = FPaths::ConvertRelativePathToFull(Request.DestinationFilename).ToLower();
        if (Destinations.Contains(Normalized))
        {
            Outcome.Errors.Add(FString::Printf(
                TEXT("Duplicate export destination %s; this bank was not written."), *Request.DestinationFilename));
            bStagingFailed = true;
            continue;
        }
        Destinations.Add(Normalized);

        FString Reason;
        if (!KyaBankSupportsExport(Request.Bank, Reason))
        {
            Outcome.Errors.Add(Reason);
            bStagingFailed = true;
            continue;
        }

        FKyaBankExportDiagnostics Diagnostics;
        FStagedFile File;
        if (!KyaBuildBankExportBytes(Request.Bank, File.Bytes, Diagnostics))
        {
            Outcome.Errors.Append(Diagnostics.Errors.IsEmpty()
                ? TArray<FString>{TEXT("The bank writer rejected this bank.")} : Diagnostics.Errors);
            Outcome.Warnings = Diagnostics.Warnings;
            Outcome.SupportedEdits = Diagnostics.SupportedEdits;
            bStagingFailed = true;
            continue;
        }
        Outcome.bValidated = true;
        Outcome.Warnings = Diagnostics.Warnings;
        Outcome.SupportedEdits = Diagnostics.SupportedEdits;
        if (SlowTask && SlowTask->ShouldCancel())
        {
            Outcome.Errors.Add(TEXT("Export was cancelled after validating this bank; no file was written."));
            bStagingFailed = true;
            continue;
        }

        const FString DestinationDirectory = FPaths::GetPath(Request.DestinationFilename);
        if (!DestinationDirectory.IsEmpty()) IFileManager::Get().MakeDirectory(*DestinationDirectory, true);
        File.TemporaryFilename = FString::Printf(TEXT("%s.kya-staging-%d"), *Request.DestinationFilename, RequestIndex);
        if (!FFileHelper::SaveArrayToFile(File.Bytes, *File.TemporaryFilename))
        {
            Outcome.Errors.Add(FString::Printf(TEXT("Could not stage output at %s."), *File.TemporaryFilename));
            bStagingFailed = true;
            continue;
        }
        StagedOutcomeIndex.Add(Outcomes.Num() - 1);
        Staged.Add(MoveTemp(File));
    }

    if (bStagingFailed)
    {
        // A partial staging set must not replace any destination.
        for (const FStagedFile& File : Staged)
            IFileManager::Get().Delete(*File.TemporaryFilename, false, true, true);
        for (FKyaBankExportOutcome& Outcome : Outcomes)
            if (Outcome.bValidated)
                Outcome.Errors.Add(TEXT("Publishing was cancelled because another bank failed to stage."));
        return Outcomes;
    }

    if (SlowTask && SlowTask->ShouldCancel())
    {
        for (const FStagedFile& File : Staged)
            IFileManager::Get().Delete(*File.TemporaryFilename, false, true, true);
        for (FKyaBankExportOutcome& Outcome : Outcomes)
            if (Outcome.bValidated)
                Outcome.Errors.Add(TEXT("Publishing was cancelled before any destination was replaced."));
        return Outcomes;
    }

    for (int32 Index = 0; Index < Staged.Num(); ++Index)
    {
        FKyaBankExportOutcome& Outcome = Outcomes[StagedOutcomeIndex[Index]];
        const FStagedFile& File = Staged[Index];
        if (IFileManager::Get().Move(*Outcome.DestinationFilename, *File.TemporaryFilename, true, true, true, true))
        {
            Outcome.bPublished = true;
        }
        else
        {
            IFileManager::Get().Delete(*File.TemporaryFilename, false, true, true);
            Outcome.Errors.Add(FString::Printf(TEXT("Could not publish %s."), *Outcome.DestinationFilename));
        }
    }
    return Outcomes;
}
