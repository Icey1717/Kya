#if WITH_DEV_AUTOMATION_TESTS
#include "KyaIso.h"
#include "HAL/FileManager.h"
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Misc/SecureHash.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKyaIsoDirectoryTest, "Kya.Disc.DirectoryAndExtraction",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKyaIsoDirectoryTest::RunTest(const FString& Parameters)
{
    TArray<uint8> Image;
    Image.SetNumZeroed(24 * 2048);
    auto Both32 = [&](int32 At, uint32 Value)
    {
        for (int32 I = 0; I < 4; ++I)
        {
            Image[At + I] = uint8(Value >> (I * 8));
            Image[At + 4 + I] = uint8(Value >> ((3 - I) * 8));
        }
    };
    auto Record = [&](int32 At, uint32 Lba, uint32 Size, bool Directory, const ANSICHAR* Name, int32 NameSize)
    {
        const int32 Length = (33 + NameSize + 1) & ~1;
        Image[At] = uint8(Length);
        Both32(At + 2, Lba);
        Both32(At + 10, Size);
        Image[At + 25] = Directory ? 2 : 0;
        Image[At + 28] = 1;
        Image[At + 31] = 1;
        Image[At + 32] = uint8(NameSize);
        FMemory::Memcpy(Image.GetData() + At + 33, Name, NameSize);
    };
    const int32 Pvd = 16 * 2048;
    const uint8 Signature[] = {1, 'C', 'D', '0', '0', '1', 1};
    FMemory::Memcpy(Image.GetData() + Pvd, Signature, sizeof(Signature));
    Image[Pvd + 129] = 8;
    Image[Pvd + 130] = 8;
    Record(Pvd + 156, 20, 2048, true, "\0", 1);
    Record(20 * 2048, 21, 2048, true, "LEVEL_1", 7);
    Record(21 * 2048, 22, 4, false, "LEVEL.BNK;1", 11);
    Image[22 * 2048] = 42;
    const FString Filename = FPaths::CreateTempFilename(*FPaths::ProjectSavedDir(), TEXT("KyaIso"), TEXT(".iso"));
    if (!TestTrue(TEXT("Write fixture"), FFileHelper::SaveArrayToFile(Image, *Filename))) return false;
    TArray<FKyaDiscFile> Files;
    FString Error;
    TestTrue(TEXT("Read directory"), KyaIso::ReadDirectory(Filename, Files, Error));
    TestEqual(TEXT("One file"), Files.Num(), 1);
    if (Files.Num() == 1)
    {
        TestEqual(TEXT("Full source path"), Files[0].Path, FString(TEXT("/LEVEL_1/LEVEL.BNK")));
        TestEqual(TEXT("Extent size"), Files[0].Size, int64(4));
    }
    TArray<uint8> Bytes;
    TestTrue(TEXT("Extract nested file"), KyaIso::Extract(Filename, TEXT("/LEVEL_1/LEVEL.BNK"), Bytes, Error));
    TestEqual(TEXT("Extracted size"), Bytes.Num(), 4);
    if (Bytes.Num() == 4) TestEqual(TEXT("Extracted content"), Bytes[0], uint8(42));
    TestFalse(TEXT("Missing file rejected"), KyaIso::Extract(Filename, TEXT("/MISSING"), Bytes, Error));
    KyaIso::FReplacement Replacement;
    Replacement.Path = TEXT("/LEVEL_1/LEVEL.BNK");
    Replacement.Bytes = {42, 0, 0, 0};
    Replacement.ExpectedHash = FMD5::HashBytes(Replacement.Bytes.GetData(), Replacement.Bytes.Num());
    Replacement.Bytes[0] = 43;
    FString Backup;
    TArray<FString> Output;
    TestTrue(TEXT("Replace and publish"), KyaIso::ReplaceFiles(Filename, {Replacement}, Backup, Error,
        [&](const FString& Message) { Output.Add(Message); }));
    const int32 SourceStep = Output.IndexOfByKey(TEXT("Verifying source bank: ") + Replacement.Path);
    const int32 WriteStep = Output.IndexOfByKey(TEXT("Writing staged bank: ") + Replacement.Path);
    const int32 VerifyStep = Output.IndexOfByKey(TEXT("Verifying staged bank: ") + Replacement.Path);
    const int32 PublishStep = Output.IndexOfByKey(FString(TEXT("Publishing the verified ISO...")));
    TestTrue(TEXT("Output reports validation, writing, verification and publishing in order"),
        SourceStep >= 0 && WriteStep > SourceStep && VerifyStep > WriteStep && PublishStep > VerifyStep);
    TestTrue(TEXT("Original backup exists"), IFileManager::Get().FileExists(*Backup));
    KyaIso::Extract(Backup, Replacement.Path, Bytes, Error);
    if (Bytes.Num() == 4) TestEqual(TEXT("Backup retains original bytes"), Bytes[0], uint8(42));
    KyaIso::Extract(Filename, Replacement.Path, Bytes, Error);
    TestTrue(TEXT("Published bytes match"), Bytes == Replacement.Bytes);
    IFileManager::Get().Delete(*Backup);
    TestFalse(TEXT("Stale source hash rejected"), KyaIso::ReplaceFiles(Filename, {Replacement}, Backup, Error));
    Image[21 * 2048 + 6] = 1;
    FFileHelper::SaveArrayToFile(Image, *Filename);
    TestFalse(TEXT("Mismatched endian extents rejected"), KyaIso::ReadDirectory(Filename, Files, Error));
    TestTrue(TEXT("No partial inventory returned"), Files.IsEmpty());
    IFileManager::Get().Delete(*Filename);
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKyaIsoRealImageTest, "Kya.Disc.RealImageResize",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FKyaIsoRealImageTest::RunTest(const FString& Parameters)
{
    const FString Source = FPlatformMisc::GetEnvironmentVariable(TEXT("KYA_TEST_ISO"));
    if (Source.IsEmpty()) { AddInfo(TEXT("Set KYA_TEST_ISO to test resizing on a disposable copy.")); return true; }
    const FString Copy = FPaths::CreateTempFilename(*FPaths::ProjectSavedDir(), TEXT("KyaDiscResize"), TEXT(".iso"));
    if (!TestTrue(TEXT("Copy test ISO"), IFileManager::Get().Copy(*Copy, *Source, false, false) == COPY_OK)) return false;
    FString Error;
    TArray<FKyaDiscFile> Before;
    TestTrue(TEXT("Read real inventory"), KyaIso::ReadDirectory(Copy, Before, Error));
    KyaIso::FReplacement R;
    R.Path = TEXT("/BWITCH.INI");
    if (!KyaIso::Extract(Copy, R.Path, R.Bytes, Error)) { AddError(Error); IFileManager::Get().Delete(*Copy); return false; }
    const int32 OriginalSize = R.Bytes.Num();
    for (int32 Size : {OriginalSize + 1, OriginalSize + 8192, OriginalSize})
    {
        R.ExpectedHash = FMD5::HashBytes(R.Bytes.GetData(), R.Bytes.Num());
        R.Bytes.SetNumZeroed(Size);
        FString Backup;
        if (!KyaIso::ReplaceFiles(Copy, {R}, Backup, Error)) { AddError(Error); break; }
        TestTrue(TEXT("Backup retained"), IFileManager::Get().FileExists(*Backup));
        IFileManager::Get().Delete(*Backup);
        TArray<uint8> Actual;
        TestTrue(TEXT("Extract resized bytes"), KyaIso::Extract(Copy, R.Path, Actual, Error));
        TestTrue(TEXT("Resized content matches"), Actual == R.Bytes);
        TArray<FKyaDiscFile> After;
        TestTrue(TEXT("Read resized inventory"), KyaIso::ReadDirectory(Copy, After, Error));
        TestEqual(TEXT("File count preserved"), After.Num(), Before.Num());
        for (int32 I = 0; I < FMath::Min(Before.Num(), After.Num()); ++I)
        {
            TestEqual(TEXT("Path preserved"), After[I].Path, Before[I].Path);
            if (Before[I].Path == R.Path) continue;
            TestEqual(TEXT("Other file offset preserved"), After[I].ImportOffset, Before[I].ImportOffset);
            TestEqual(TEXT("Other file size preserved"), After[I].Size, Before[I].Size);
        }
    }
    IFileManager::Get().Delete(*Copy);
    return true;
}
#endif
