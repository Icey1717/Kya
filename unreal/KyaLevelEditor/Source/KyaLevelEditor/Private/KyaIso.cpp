#include "KyaIso.h"

#include "HAL/PlatformFileManager.h"

namespace
{
constexpr int64 SectorSize = 2048;
uint32 ReadLE(const uint8* P) { return uint32(P[0]) | uint32(P[1]) << 8 | uint32(P[2]) << 16 | uint32(P[3]) << 24; }
uint32 ReadBE(const uint8* P) { return uint32(P[3]) | uint32(P[2]) << 8 | uint32(P[1]) << 16 | uint32(P[0]) << 24; }

struct FReader
{
    IFileHandle& Handle;
    FString& Error;
    TSet<int64> Directories;

    bool Fail(const TCHAR* Message) { Error = Message; return false; }
    bool Read(int64 Offset, uint8* Data, int64 Size)
    {
        if (Offset < 0 || Size < 0 || Offset > Handle.Size() || Size > Handle.Size() - Offset)
            return Fail(TEXT("ISO extent lies outside the image."));
        return (Handle.Seek(Offset) && Handle.Read(Data, Size)) || Fail(TEXT("Cannot read ISO image."));
    }
    bool Extent(const uint8* Record, int64& Offset, int64& Size)
    {
        if (ReadLE(Record + 2) != ReadBE(Record + 6) || ReadLE(Record + 10) != ReadBE(Record + 14))
            return Fail(TEXT("Inconsistent ISO9660 extent metadata."));
        if (Record[1] || Record[26] || Record[27] || (Record[25] & 128))
            return Fail(TEXT("Extended, interleaved and multi-extent ISO files are unsupported."));
        Offset = int64(ReadLE(Record + 2)) * SectorSize;
        Size = ReadLE(Record + 10);
        return (Offset <= Handle.Size() && Size <= Handle.Size() - Offset)
            || Fail(TEXT("ISO extent lies outside the image."));
    }
    bool Walk(const uint8* Record, const FString& Parent, TArray<FKyaDiscFile>& Files, int32 Depth)
    {
        int64 Offset, Size;
        if (!Extent(Record, Offset, Size)) return false;
        if (Depth > 64 || Directories.Contains(Offset) || Size > 64 * 1024 * 1024)
            return Fail(TEXT("Invalid or excessively large ISO directory tree."));
        Directories.Add(Offset);
        TArray<uint8> Data;
        Data.SetNumUninitialized(int32(Size));
        if (!Read(Offset, Data.GetData(), Size)) return false;
        for (int64 Position = 0; Position < Size;)
        {
            const uint8 Length = Data[Position];
            if (!Length) { Position = (Position / SectorSize + 1) * SectorSize; continue; }
            if (Length < 34 || Length > Size - Position || Position % SectorSize + Length > SectorSize)
                return Fail(TEXT("Invalid ISO directory record."));
            const uint8* Entry = Data.GetData() + Position;
            Position += Length;
            const int32 NameLength = Entry[32];
            if (!NameLength || 33 + NameLength > Length) return Fail(TEXT("Invalid ISO filename."));
            if (NameLength == 1 && Entry[33] <= 1) continue;
            FString Name;
            for (int32 I = 0; I < NameLength && Entry[33 + I] != ';'; ++I)
            {
                const uint8 C = Entry[33 + I];
                if (C < 32 || C > 126 || C == '/' || C == '\\') return Fail(TEXT("Unsupported ISO filename."));
                Name.AppendChar(TCHAR(C));
            }
            if (Name.IsEmpty() || Name == TEXT(".") || Name == TEXT("..")) return Fail(TEXT("Invalid ISO filename."));
            const FString Path = Parent + TEXT("/") + Name;
            if (Entry[25] & 2)
            {
                if (!Walk(Entry, Path, Files, Depth + 1)) return false;
            }
            else
            {
                FKyaDiscFile File;
                File.Path = Path;
                File.DirectoryRecordOffset = Offset + Position - Length;
                if (!Extent(Entry, File.ImportOffset, File.Size)) return false;
                Files.Add(MoveTemp(File));
            }
        }
        return true;
    }
};
}

bool KyaIso::ReadDirectory(const FString& Filename, TArray<FKyaDiscFile>& Files, FString& Error)
{
    Files.Reset();
    Error.Reset();
    TUniquePtr<IFileHandle> Handle(FPlatformFileManager::Get().GetPlatformFile().OpenRead(*Filename));
    if (!Handle) { Error = TEXT("Cannot open ISO image."); return false; }
    FReader Reader{*Handle, Error, {}};
    uint8 Descriptor[2048];
    if (!Reader.Read(16 * SectorSize, Descriptor, sizeof(Descriptor))) return false;
    const uint8 Signature[] = {1, 'C', 'D', '0', '0', '1', 1};
    if (FMemory::Memcmp(Descriptor, Signature, sizeof(Signature)) || Descriptor[128] != 0 || Descriptor[129] != 8
        || Descriptor[130] != 8 || Descriptor[131] != 0 || Descriptor[156] < 34)
        return Reader.Fail(TEXT("Requires a 2048-byte-sector ISO9660 image."));
    TArray<FKyaDiscFile> Result;
    if (!Reader.Walk(Descriptor + 156, TEXT(""), Result, 0)) return false;
    TSet<FString> Paths;
    for (const FKyaDiscFile& File : Result)
    {
        if (Paths.Contains(File.Path)) return Reader.Fail(TEXT("Duplicate ISO file path."));
        Paths.Add(File.Path);
    }
    Files = MoveTemp(Result);
    return true;
}

bool KyaIso::Extract(const FString& Filename, const FString& Path, TArray<uint8>& Bytes, FString& Error)
{
    Bytes.Reset();
    TArray<FKyaDiscFile> Files;
    if (!ReadDirectory(Filename, Files, Error)) return false;
    const FKyaDiscFile* File = Files.FindByPredicate([&](const FKyaDiscFile& F) { return F.Path == Path; });
    if (!File || File->Size > MAX_int32) { Error = TEXT("ISO file is missing or too large to import."); return false; }
    TUniquePtr<IFileHandle> Handle(FPlatformFileManager::Get().GetPlatformFile().OpenRead(*Filename));
    if (!Handle) { Error = TEXT("Cannot open ISO image."); return false; }
    FReader Reader{*Handle, Error, {}};
    Bytes.SetNumUninitialized(int32(File->Size));
    if (!Reader.Read(File->ImportOffset, Bytes.GetData(), File->Size)) { Bytes.Reset(); return false; }
    return true;
}
