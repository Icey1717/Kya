#include "KyaDiscFactory.h"

#include "KyaDisc.h"
#include "KyaIso.h"
#include "Misc/FeedbackContext.h"
#include "Misc/Paths.h"

UKyaDiscFactory::UKyaDiscFactory()
{
    SupportedClass = UKyaDisc::StaticClass();
    bCreateNew = false;
    bEditorImport = true;
    bText = false;
    Formats.Add(TEXT("iso;Kya game disc"));
}

UObject* UKyaDiscFactory::FactoryCreateFile(UClass* InClass, UObject* InParent, FName InName,
    EObjectFlags Flags, const FString& Filename, const TCHAR* Parms,
    FFeedbackContext* Warn, bool& bOutOperationCanceled)
{
    bOutOperationCanceled = false;
    TArray<FKyaDiscFile> Files;
    FString Error;
    if (!KyaIso::ReadDirectory(Filename, Files, Error))
    {
        if (Warn) Warn->Logf(ELogVerbosity::Error, TEXT("%s"), *Error);
        return nullptr;
    }
    if (!Files.ContainsByPredicate([](const FKyaDiscFile& File)
        { return File.Path.EndsWith(TEXT("/LEVEL.BNK"), ESearchCase::IgnoreCase); }))
    {
        if (Warn) Warn->Logf(ELogVerbosity::Error, TEXT("The image contains no Kya LEVEL.BNK files."));
        return nullptr;
    }
    UKyaDisc* Disc = NewObject<UKyaDisc>(InParent, InClass, InName, Flags);
    Disc->SourceFilename = FPaths::ConvertRelativePathToFull(Filename);
    Disc->Files = MoveTemp(Files);
    for (const FKyaDiscFile& File : Disc->Files)
    {
        if (!File.Path.EndsWith(TEXT("/LEVEL.BNK"), ESearchCase::IgnoreCase)) continue;
        FKyaDiscLevel Level;
        Level.SourceDirectory = FPaths::GetPath(File.Path);
        Disc->Levels.Add(MoveTemp(Level));
    }
    return Disc;
}
