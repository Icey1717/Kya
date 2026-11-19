#include "KyaSectorBankExport.h"

#include "KyaBankExport.h"
#include "KyaLevelBank.h"
#include "KyaGeometryEncoding.h"
#include "UObject/Class.h"

bool KyaSectorBankSupportsExport(const UKyaLevelBank& Bank, FString& OutReason)
{
    OutReason.Reset();
    const TArray<uint8>& Bytes = Bank.BankBytes;
    if (Bytes.Num() < 0x48 || FMemory::Memcmp(Bytes.GetData() + 8, "KNAB", 4) != 0)
    {
        OutReason = TEXT("The sector bank has no valid retained bank header.");
        return false;
    }
    const auto ReadU32 = [&Bytes](int64 Offset) -> uint32
    {
        const uint8* P = Bytes.GetData() + Offset;
        return uint32(P[0]) | (uint32(P[1]) << 8) | (uint32(P[2]) << 16) | (uint32(P[3]) << 24);
    };
    if (ReadU32(0x0c) & 1)
    {
        OutReason = TEXT("Packed sector banks must be unpacked before export.");
        return false;
    }
    const uint32 Count = ReadU32(0x28);
    const uint64 Table = uint64(ReadU32(0x30)) + 8;
    const uint32 CommonType = ReadU32(0x14);
    const uint64 TypeTable = uint64(ReadU32(0x34)) + 8;
    if (Count != uint32(Bank.Entries.Num()) || Bank.EntryCount != Bank.Entries.Num() ||
        Table > uint64(Bytes.Num()) || uint64(Count) * 16 > uint64(Bytes.Num()) - Table ||
        (!CommonType && (TypeTable > uint64(Bytes.Num()) || uint64(Count) * 4 > uint64(Bytes.Num()) - TypeTable)))
    {
        OutReason = TEXT("The sector bank entry tables are invalid or differ from the imported entries.");
        return false;
    }
    bool bMesh = false;
    bool bTexture = false;
    for (int32 Index = 0; Index < Bank.Entries.Num(); ++Index)
    {
        const FKyaBankEntry& Entry = Bank.Entries[Index];
        const uint32 Pair = CommonType ? CommonType : ReadU32(TypeTable + uint64(Index) * 4);
        if (Entry.Offset < 0 || Entry.Size < 0 || Entry.Offset > Bytes.Num() || Entry.Size > Bytes.Num() - Entry.Offset ||
            Entry.Offset != ReadU32(Table + uint64(Index) * 16) ||
            Entry.Size != ReadU32(Table + uint64(Index) * 16 + 4) ||
            Entry.Type != int32(Pair >> 16) || Entry.Subtype != int32(Pair & 0xffff))
        {
            OutReason = FString::Printf(TEXT("Sector bank entry %d is invalid or differs from its retained table."), Index);
            return false;
        }
        if (Entry.Type == 6)
        {
            OutReason = TEXT("A bank with scene records requires the main scene writer and a scene configuration entry.");
            return false;
        }
        // CSector::InstallCallback installs these entries directly. Collision,
        // background and ANHR entries are optional; retain every entry verbatim.
        bMesh |= Entry.Type == 4 && Entry.Subtype == 1 && Entry.Size > 0;
        bTexture |= Entry.Type == 5 && Entry.Subtype == 1 && Entry.Size > 0;
    }
    if (!bMesh || !bTexture)
    {
        OutReason = TEXT("The bank has neither a scene configuration nor the mesh/texture entries required by a sector.");
        return false;
    }
    return true;
}

bool KyaBuildSectorBankExportBytes(const UKyaLevelBank& Bank, TArray<uint8>& OutBytes,
    FKyaBankExportDiagnostics& Diagnostics)
{
    FString Reason;
    if (!KyaSectorBankSupportsExport(Bank, Reason))
    {
        Diagnostics.Errors.Add(Reason);
        return false;
    }
    // The factory leaves these at their defaults when there is no scene entry.
    // Reject edits instead of reporting success while silently dropping them.
    const FKyaSceneEnvironment DefaultEnvironment;
    const FKyaSceneConfiguration DefaultConfiguration;
    if (!FKyaSceneEnvironment::StaticStruct()->CompareScriptStruct(&Bank.SceneEnvironment, &DefaultEnvironment, 0) ||
        !FKyaSceneConfiguration::StaticStruct()->CompareScriptStruct(&Bank.SceneConfiguration, &DefaultConfiguration, 0))
    {
        Diagnostics.Errors.Add(TEXT("Sector banks have no scene environment or configuration record; edit those values on the main level bank."));
        return false;
    }
    TArray<uint8> Bytes = Bank.BankBytes;
    if (!KyaPatchBankLights(Bank, Bytes, Diagnostics)) return false;
    if (!KyaGeometry::ValidateAppliedGeometry(Bank, Bytes, Reason)) { Diagnostics.Errors.Add(Reason); return false; }
    Diagnostics.SupportedEdits.Add(FString::Printf(
        TEXT("Sector bank: %d entries preserved, including meshes, textures, collision and animation; %d light record(s) patched in place. "
            "Scene and actor records belong to the main level bank. Hierarchy transforms are preserved."),
        Bank.Entries.Num(), Bank.Lights.Num()));
    OutBytes = MoveTemp(Bytes);
    return true;
}
