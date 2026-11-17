#include "KyaIso.h"
#include "HAL/PlatformFileManager.h"
#include "HAL/FileManager.h"
#include "Misc/Guid.h"
#include "Misc/SecureHash.h"

namespace
{
constexpr int64 Sector = 2048;
uint64 Get(const TArray<uint8>& B, int32 At, int32 Count = 4)
{
    uint64 V = 0;
    for (int32 I = 0; I < Count; ++I) V |= uint64(B[At + I]) << (I * 8);
    return V;
}
void Put(TArray<uint8>& B, int32 At, uint64 V, int32 Count = 4)
{
    for (int32 I = 0; I < Count; ++I) B[At + I] = uint8(V >> (I * 8));
}
TArray<uint8> Both(uint32 V)
{
    TArray<uint8> B;
    B.SetNumZeroed(8);
    Put(B, 0, V);
    for (int32 I = 0; I < 4; ++I) B[4 + I] = uint8(V >> ((3 - I) * 8));
    return B;
}
uint16 Crc(const uint8* Data, int32 Size)
{
    uint16 Result = 0;
    for (int32 I = 0; I < Size; ++I)
    {
        Result ^= uint16(Data[I]) << 8;
        for (int32 Bit = 0; Bit < 8; ++Bit) Result = (Result & 0x8000) ? (Result << 1) ^ 0x1021 : Result << 1;
    }
    return Result;
}
uint8 TagSum(const TArray<uint8>& B)
{
    uint8 Sum = 0;
    for (int32 I = 0; I < 16; ++I) if (I != 4) Sum += B[I];
    return Sum;
}
bool ValidTag(const TArray<uint8>& B)
{
    const int32 N = int32(Get(B, 10, 2));
    return N <= 2032 && TagSum(B) == B[4] && Crc(B.GetData() + 16, N) == Get(B, 8, 2);
}
void Seal(TArray<uint8>& B)
{
    Put(B, 8, Crc(B.GetData() + 16, int32(Get(B, 10, 2))), 2);
    B[4] = TagSum(B);
}
struct FChange { int64 Offset; TArray<uint8> Bytes; };
struct FWriter
{
    IFileHandle& Handle;
    FString& Error;
    TArray<FChange> Changes;
    bool Fail(const TCHAR* Message) { Error = Message; return false; }
    bool Read(int64 Lba, TArray<uint8>& B, bool Check = true)
    {
        B.SetNumUninitialized(Sector);
        if (Lba < 0 || Lba >= Handle.Size() / Sector || !Handle.Seek(Lba * Sector) || !Handle.Read(B.GetData(), Sector))
            return Fail(TEXT("Cannot read ISO metadata."));
        return !Check || ValidTag(B) || Fail(TEXT("Invalid UDF descriptor checksum."));
    }
    void Add(int64 Lba, TArray<uint8> B) { Seal(B); Changes.Add({Lba * Sector, MoveTemp(B)}); }
    bool Growth(int64 Size)
    {
        const int64 Total = Handle.Size();
        if (Total % Sector) return Fail(TEXT("ISO is not sector aligned."));
        const int64 End = Total / Sector + (Size + Sector - 1) / Sector;
        if (End >= MAX_uint32) return Fail(TEXT("ISO exceeds supported size."));
        TArray<uint8> Anchor;
        if (!Read(256, Anchor)) return false;
        if (Get(Anchor, 0, 2) != 2) return Fail(TEXT("Missing UDF anchor."));
        int64 Base = -1, OldLength = -1, IntegrityLba = -1, IntegrityLength = -1;
        for (int32 Field : {16, 24})
        {
            const int64 Length = Get(Anchor, Field), Location = Get(Anchor, Field + 4);
            if (!Length || Length % Sector || Length > 1024 * 1024) return Fail(TEXT("Unsupported UDF descriptor sequence."));
            int32 Partitions = 0, Volumes = 0;
            for (int64 Lba = Location; Lba < Location + Length / Sector; ++Lba)
            {
                TArray<uint8> B;
                if (!Read(Lba, B)) return false;
                const int32 Kind = int32(Get(B, 0, 2));
                if (Kind == 8) break;
                if (Kind == 5)
                {
                    ++Partitions;
                    const int64 NewBase = Get(B, 188), NewLength = Get(B, 192);
                    for (int32 I = 56; I < 184; ++I) if (B[I]) return Fail(TEXT("UDF allocation tables are unsupported."));
                    if (Get(B, 184) != 1 || NewBase + NewLength != Total / Sector - 1)
                        return Fail(TEXT("Requires a read-only UDF partition ending before the final anchor."));
                    if (Base != -1 && (Base != NewBase || OldLength != NewLength)) return Fail(TEXT("UDF partition copies disagree."));
                    Base = NewBase; OldLength = NewLength;
                    Put(B, 192, End - Base);
                    Add(Lba, MoveTemp(B));
                }
                else if (Kind == 6)
                {
                    ++Volumes;
                    if (Get(B, 212) != Sector || Get(B, 264) != 6 || Get(B, 268) != 1 || B[440] != 1 || B[441] != 6)
                        return Fail(TEXT("Unsupported UDF partition map."));
                    const int64 N = Get(B, 432), L = Get(B, 436);
                    if (IntegrityLba != -1 && (IntegrityLba != L || IntegrityLength != N)) return Fail(TEXT("UDF volume copies disagree."));
                    IntegrityLba = L; IntegrityLength = N;
                }
                else if (Kind == 7 && Get(B, 20)) return Fail(TEXT("UDF unallocated extents are unsupported."));
            }
            if (Partitions != 1 || Volumes != 1) return Fail(TEXT("Requires one physical UDF partition."));
        }
        if (IntegrityLength != 2 * Sector) return Fail(TEXT("Unsupported UDF integrity sequence."));
        TArray<uint8> B;
        if (!Read(IntegrityLba, B)) return false;
        if (Get(B, 0, 2) != 9 || Get(B, 28) != 1 || Get(B, 32, 8) || Get(B, 72) != 1 || Get(B, 84) != uint64(OldLength))
            return Fail(TEXT("Unsupported UDF integrity descriptor."));
        Put(B, 84, End - Base);
        Add(IntegrityLba, MoveTemp(B));
        Put(Anchor, 12, End);
        Add(End, MoveTemp(Anchor));
        Changes.Add({16 * Sector + 80, Both(uint32(End + 1))});
        return true;
    }
    bool Resize(const TArray<FKyaDiscFile>& Files, const FKyaDiscFile& File, int64 Size, int64 Destination)
    {
        for (const FKyaDiscFile& F : Files)
        {
            if (F.ImportOffset != File.ImportOffset) continue;
            if (F.Size != File.Size) return Fail(TEXT("Aliased ISO extents have inconsistent sizes."));
            Changes.Add({F.DirectoryRecordOffset + 10, Both(uint32(Size))});
            Changes.Add({F.DirectoryRecordOffset + 2, Both(uint32(Destination / Sector))});
        }
        bool Terminated = false;
        for (int64 Lba = 17; Lba < 81; ++Lba)
        {
            TArray<uint8> B;
            if (!Read(Lba, B, false)) return false;
            if (FMemory::Memcmp(B.GetData() + 1, "CD001\1", 6)) return Fail(TEXT("Invalid ISO volume descriptor."));
            if (B[0] == 255) { Terminated = true; break; }
            if (B[0] != 0) return Fail(TEXT("Additional ISO volume views are unsupported for resizing."));
        }
        if (!Terminated) return Fail(TEXT("Missing ISO descriptor terminator."));
        TArray<uint8> Anchor;
        if (!Read(256, Anchor)) return false;
        if (Get(Anchor, 0, 2) != 2) return Fail(TEXT("Resizing requires a PS2 UDF bridge image."));
        const int64 Length = Get(Anchor, 16), Location = Get(Anchor, 20);
        if (!Length || Length % Sector || Length > 1024 * 1024) return Fail(TEXT("Invalid UDF sequence."));
        int64 Partition = -1;
        for (int64 Lba = Location; Lba < Location + Length / Sector; ++Lba)
        {
            TArray<uint8> B;
            if (!Read(Lba, B)) return false;
            if (Get(B, 0, 2) == 8) break;
            if (Get(B, 0, 2) != 5) continue;
            if (Partition != -1) return Fail(TEXT("Multiple UDF partitions are unsupported."));
            Partition = Get(B, 188);
        }
        if (Partition < 0) return Fail(TEXT("Missing UDF partition."));
        int64 FirstFile = Handle.Size() / Sector;
        for (const FKyaDiscFile& F : Files) FirstFile = FMath::Min(FirstFile, F.ImportOffset / Sector);
        int32 Found = 0;
        for (int64 Lba = Partition; Lba < FirstFile; ++Lba)
        {
            TArray<uint8> B;
            if (!Read(Lba, B, false)) return false;
            if (Get(B, 0, 2) != 261 || B[27] != 5) continue;
            const uint64 At = 176 + Get(B, 168);
            if (Get(B, 172) != 8 || At > 2040 || (Get(B, 34, 2) & 7))
                return Fail(TEXT("Unsupported UDF file allocation descriptor."));
            if ((Partition + int64(Get(B, int32(At + 4)))) * Sector != File.ImportOffset) continue;
            if (!ValidTag(B) || Get(B, 10, 2) < At + 8 - 16) return Fail(TEXT("Invalid UDF file entry checksum."));
            if (Get(B, int32(At)) != uint64(File.Size) || Get(B, 56, 8) != uint64(File.Size))
                return Fail(TEXT("ISO and UDF file sizes disagree."));
            Put(B, 56, Size, 8);
            Put(B, int32(At), Size);
            Put(B, int32(At + 4), Destination / Sector - Partition);
            Put(B, 64, (Size + Sector - 1) / Sector, 8);
            Add(Lba, MoveTemp(B));
            ++Found;
        }
        return Found == 1 || Fail(TEXT("Cannot uniquely locate the UDF file entry."));
    }
    bool Apply()
    {
        for (const FChange& Change : Changes)
            if (!Handle.Seek(Change.Offset) || !Handle.Write(Change.Bytes.GetData(), Change.Bytes.Num()))
                return Fail(TEXT("Cannot write staged ISO."));
        return Handle.Flush(true) || Fail(TEXT("Cannot flush staged ISO."));
    }
};

bool Patch(const FString& Filename, const KyaIso::FReplacement& Replacement, FString& Error)
{
    TArray<FKyaDiscFile> Files;
    if (!KyaIso::ReadDirectory(Filename, Files, Error)) return false;
    const FKyaDiscFile* File = Files.FindByPredicate([&](const FKyaDiscFile& F) { return F.Path == Replacement.Path; });
    if (!File) { Error = TEXT("Missing ISO replacement path: ") + Replacement.Path; return false; }
    // OpenWrite(append=true) preserves existing bytes; every write seeks explicitly.
    TUniquePtr<IFileHandle> Handle(FPlatformFileManager::Get().GetPlatformFile().OpenWrite(*Filename, true, true));
    if (!Handle) { Error = TEXT("Cannot open staged ISO for writing."); return false; }
    FWriter Writer{*Handle, Error, {}};
    const int64 Size = Replacement.Bytes.Num();
    int64 Destination = File->ImportOffset;
    if (Size != File->Size)
    {
        if ((Size + Sector - 1) / Sector != (File->Size + Sector - 1) / Sector)
        {
            Destination = Handle->Size();
            if (!Writer.Growth(Size)) return false;
        }
        if (!Writer.Resize(Files, *File, Size, Destination)) return false;
    }
    Writer.Changes.Insert({Destination, Replacement.Bytes}, 0);
    return Writer.Apply();
}
}

bool KyaIso::ReplaceFiles(const FString& Filename, const TArray<FReplacement>& Replacements, FString& Backup, FString& Error,
    const TFunction<void(const FString&)>& Output)
{
    auto Log = [&](const FString& Message) { if (Output) Output(Message); };
    Backup.Reset();
    Error.Reset();
    if (Replacements.IsEmpty()) return true;
    // Keep the source read-locked while validating and staging (Windows disallows writers).
    TUniquePtr<IFileHandle> SourceGuard(FPlatformFileManager::Get().GetPlatformFile().OpenRead(*Filename));
    if (!SourceGuard) { Error = TEXT("Cannot open source ISO; close other image writers."); return false; }
    TSet<FString> Paths;
    for (const FReplacement& Replacement : Replacements)
    {
        Log(TEXT("Verifying source bank: ") + Replacement.Path);
        if (Paths.Contains(Replacement.Path) || Replacement.Bytes.IsEmpty() || Replacement.Bytes.Num() >= (1 << 30)
            || Replacement.ExpectedHash.IsEmpty())
        { Error = TEXT("Invalid or duplicate ISO replacement request."); return false; }
        Paths.Add(Replacement.Path);
        TArray<uint8> Original;
        if (!Extract(Filename, Replacement.Path, Original, Error)) return false;
        if (FMD5::HashBytes(Original.GetData(), Original.Num()) != Replacement.ExpectedHash)
        { Error = TEXT("Disc file changed since import/export: ") + Replacement.Path; return false; }
    }
    const FString Suffix = FGuid::NewGuid().ToString(EGuidFormats::Digits);
    const FString Staged = Filename + TEXT(".kya-stage-") + Suffix;
    IFileManager& Manager = IFileManager::Get();
    Log(TEXT("Copying the source ISO to a staging image. This may take a while..."));
    if (Manager.Copy(*Staged, *Filename, false, false) != COPY_OK)
    { Manager.Delete(*Staged); Error = TEXT("Cannot stage ISO; check free disk space."); return false; }
    bool Success = true;
    for (const FReplacement& Replacement : Replacements)
    {
        Log(TEXT("Writing staged bank: ") + Replacement.Path);
        TArray<uint8> Original;
        if (!Extract(Staged, Replacement.Path, Original, Error)
            || FMD5::HashBytes(Original.GetData(), Original.Num()) != Replacement.ExpectedHash)
        { Error = TEXT("Staged disc source changed or contains aliased edits: ") + Replacement.Path; Success = false; break; }
        if (!Patch(Staged, Replacement, Error)) { Success = false; break; }
    }
    // Verify every file after all patches, catching conflicting aliases as well.
    if (Success) for (const FReplacement& Replacement : Replacements)
    {
        Log(TEXT("Verifying staged bank: ") + Replacement.Path);
        TArray<uint8> Actual;
        if (!Extract(Staged, Replacement.Path, Actual, Error) || Actual != Replacement.Bytes)
        { Error = TEXT("ISO replacement verification failed: ") + Replacement.Path; Success = false; break; }
    }
    if (!Success) { Manager.Delete(*Staged); return false; }
    SourceGuard.Reset();
    Backup = Filename + TEXT(".kya-backup-") + Suffix;
    Log(TEXT("Preserving original ISO as: ") + Backup);
    if (!Manager.Move(*Backup, *Filename, false, false))
    { Manager.Delete(*Staged); Backup.Reset(); Error = TEXT("Cannot preserve original ISO as backup."); return false; }
    Log(TEXT("Publishing the verified ISO..."));
    if (!Manager.Move(*Filename, *Staged, false, false))
    {
        const bool Restored = Manager.Move(*Filename, *Backup, false, false);
        Error = Restored ? TEXT("Cannot publish staged ISO; original restored.")
            : TEXT("Cannot publish staged ISO. Restore original from: ") + Backup;
        Manager.Delete(*Staged);
        if (Restored) Backup.Reset();
        return false;
    }
    return true;
}
