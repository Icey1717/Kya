#if WITH_DEV_AUTOMATION_TESTS
#include "KyaBankExport.h"
#include "KyaActorCatalog.h"
#include "KyaActorEditService.h"
#include "KyaCameraStream.h"
#include "KyaLevelBank.h"
#include "KyaLevelBankFactory.h"
#include "KyaLevelEditorActions.h"
#include "HAL/FileManager.h"
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

namespace
{
void PutU32(TArray<uint8>& Bytes, int32 Offset, uint32 Value)
{
    for (int32 Index = 0; Index < 4; ++Index) Bytes[Offset + Index] = uint8(Value >> (Index * 8));
}

void PutFloat(TArray<uint8>& Bytes, int32 Offset, float Value)
{
    uint32 Bits;
    FMemory::Memcpy(&Bits, &Value, 4);
    PutU32(Bytes, Offset, Bits);
}

TArray<uint8> MakeSectorBytes(bool bOptionalEntries, bool bLights = false)
{
    TArray<uint32> Types = {0x40001, 0x50001};
    if (bOptionalEntries) Types.Append({0x70001, 0x40002, 0x50003, 0x170001, 0x990001});
    if (bLights) Types.Add(0x100001);
    const int32 Count = Types.Num();
    const int32 Table = 0x48;
    const int32 TypeTable = Table + Count * 16;
    TArray<uint8> Bytes;
    Bytes.AddZeroed(TypeTable + Count * 4);
    FMemory::Memcpy(Bytes.GetData() + 8, "KNAB", 4);
    PutU32(Bytes, 0x28, Count);
    PutU32(Bytes, 0x30, Table - 8);
    PutU32(Bytes, 0x34, TypeTable - 8);
    for (int32 Index = 0; Index < Count; ++Index)
    {
        const int32 Offset = Bytes.Num();
        const int32 Size = Types[Index] == 0x100001 ? 72 : 32;
        Bytes.AddZeroed(Size);
        PutU32(Bytes, Table + Index * 16, Offset);
        PutU32(Bytes, Table + Index * 16 + 4, Size);
        PutU32(Bytes, TypeTable + Index * 4, Types[Index]);
        if (Types[Index] == 0x100001)
        {
            PutU32(Bytes, Offset, 0xDEADBEEF);
            FMemory::Memcpy(Bytes.GetData() + Offset + 8, "THGL", 4);
            PutU32(Bytes, Offset + 16, 1); // count
            PutU32(Bytes, Offset + 20, 4); // record relative to table
            // Type 0, shape 0: base record, position/falloff, one trailing word.
            for (int32 Channel = 0; Channel < 12; ++Channel) Bytes[Offset + 32 + Channel] = uint8(Channel + 3);
            PutFloat(Bytes, Offset + 44, 1.0f);
            PutFloat(Bytes, Offset + 48, 4.0f);
            PutFloat(Bytes, Offset + 60, 10.0f);
            PutFloat(Bytes, Offset + 64, 20.0f);
            PutU32(Bytes, Offset + 68, 7);
        }
        else
        {
            // Opaque content: exporting sectors must never decode or rewrite it.
            for (int32 Byte = 0; Byte < Size; ++Byte) Bytes[Offset + Byte] = uint8(Index * 31 + Byte);
        }
    }
    // Include padding not owned by any entry: that also belongs to the source file.
    Bytes.Append({0x73, 0x19, 0xfa, 0x00});
    return Bytes;
}

UKyaLevelBank* ImportSectorBytes(const TArray<uint8>& Bytes)
{
    UKyaLevelBankFactory* Factory = NewObject<UKyaLevelBankFactory>();
    const uint8* Cursor = Bytes.GetData();
    return Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
        GetTransientPackage(), NAME_None, RF_Transient, nullptr, TEXT("bnk"), Cursor,
        Bytes.GetData() + Bytes.Num(), GWarn));
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKyaSectorRoundTripTest, "Kya.Bank.SectorRoundTrip",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKyaSectorRoundTripTest::RunTest(const FString& Parameters)
{
    for (bool bOptional : {false, true})
    {
        const TArray<uint8> Original = MakeSectorBytes(bOptional);
        UKyaLevelBank* Bank = ImportSectorBytes(Original);
        if (!TestNotNull(TEXT("Import sector without scene records"), Bank)) return false;
        FString Reason = TEXT("previous error");
        TestTrue(TEXT("Sector is supported regardless of asset name"), KyaBankSupportsExport(Bank, Reason));
        TestTrue(TEXT("Successful check clears old reason"), Reason.IsEmpty());
        const FKyaBankValidationReport Report = KyaValidateBank(Bank);
        TestTrue(TEXT("Validation builds a sector"), Report.bBytesBuilt);
        TestTrue(TEXT("Sector has no missing-main-background warning"), Report.Issues.IsEmpty());
        FKyaBankExportDiagnostics Diagnostics;
        TArray<uint8> Exported;
        if (!TestTrue(TEXT("Export sector"), KyaBuildBankExportBytes(Bank, Exported, Diagnostics))) return false;
        TestTrue(TEXT("All opaque entries, tables and padding survive exactly"), Original == Exported);
        UKyaLevelBank* Reimported = ImportSectorBytes(Exported);
        if (TestNotNull(TEXT("Reimport sector"), Reimported))
            TestEqual(TEXT("Reimport preserves entry count"), Reimported->Entries.Num(), Bank->Entries.Num());
    }

    const TArray<uint8> LitOriginal = MakeSectorBytes(true, true);
    UKyaLevelBank* LitBank = ImportSectorBytes(LitOriginal);
    if (!TestNotNull(TEXT("Import sector with light archive"), LitBank) ||
        !TestEqual(TEXT("One typed light"), LitBank->Lights.Num(), 1)) return false;
    {
        FKyaBankExportDiagnostics Diagnostics;
        TArray<uint8> Exported;
        TestTrue(TEXT("Export unchanged lit sector"), KyaBuildBankExportBytes(LitBank, Exported, Diagnostics));
        TestTrue(TEXT("Unchanged light archive preserves bytes"), Exported == LitOriginal);
    }
    LitBank->Lights[0].Position.X = 23.0;
    LitBank->Lights[0].Color.R = 32.0f / 255.0f;
    {
        FKyaBankExportDiagnostics Diagnostics;
        TArray<uint8> Exported;
        if (!TestTrue(TEXT("Export edited sector light"), KyaBuildBankExportBytes(LitBank, Exported, Diagnostics))) return false;
        const int64 LightOffset = LitBank->Entries.Last().Offset;
        for (int32 Byte = 0; Byte < Exported.Num(); ++Byte)
            if (Byte != LightOffset + 36 && (Byte < LightOffset + 48 || Byte >= LightOffset + 52))
                if (Exported[Byte] != LitOriginal[Byte])
                {
                    AddError(FString::Printf(TEXT("Unedited sector byte %d changed"), Byte));
                    break;
                }
        UKyaLevelBank* Reimported = ImportSectorBytes(Exported);
        if (TestNotNull(TEXT("Reimport edited sector light"), Reimported) &&
            TestEqual(TEXT("Edited sector retains light"), Reimported->Lights.Num(), 1))
        {
            TestEqual(TEXT("Light position survives"), Reimported->Lights[0].Position.X, 23.0);
            TestTrue(TEXT("Light red channel survives"), FMath::IsNearlyEqual(Reimported->Lights[0].Color.R, 32.0f / 255.0f));
        }
    }
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKyaSectorExportValidationTest, "Kya.Bank.SectorExportValidation",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKyaSectorExportValidationTest::RunTest(const FString& Parameters)
{
    const TArray<uint8> Original = MakeSectorBytes(true);
    const auto Reject = [this](UKyaLevelBank* Bank, const TCHAR* Label)
    {
        FKyaBankExportDiagnostics Diagnostics;
        TArray<uint8> Exported = {1, 2, 3};
        TestFalse(Label, KyaBuildBankExportBytes(Bank, Exported, Diagnostics));
        TestTrue(TEXT("Rejection explains failure"), Diagnostics.HasErrors());
        TestTrue(TEXT("Rejection leaves no export bytes"), Exported.IsEmpty());
    };
    UKyaLevelBank* Bank = ImportSectorBytes(Original);
    if (!TestNotNull(TEXT("Import validation fixture"), Bank)) return false;
    Bank->SceneEnvironment.ClipDistance = 42.0f;
    Reject(Bank, TEXT("Reject phantom scene environment edits"));
    Bank->SceneEnvironment = FKyaSceneEnvironment();
    Bank->SceneEnvironment.Actors.AddDefaulted();
    Reject(Bank, TEXT("Reject actor insertion into sector bank"));
    Bank->SceneEnvironment = FKyaSceneEnvironment();
    Bank->SceneConfiguration.Field20 = 12;
    Reject(Bank, TEXT("Reject phantom scene configuration edits"));
    Bank->SceneConfiguration = FKyaSceneConfiguration();
    Bank->Lights.AddDefaulted();
    Reject(Bank, TEXT("Reject lights without an archive"));
    Bank->Lights.Reset();
    --Bank->ImportSchemaVersion;
    Reject(Bank, TEXT("Reject stale sector schema"));
    ++Bank->ImportSchemaVersion;
    PutU32(Bank->BankBytes, 12, 1);
    Reject(Bank, TEXT("Reject packed sector bytes"));
    Bank->BankBytes = Original;
    PutU32(Bank->BankBytes, 0x30, MAX_uint32);
    Reject(Bank, TEXT("Reject invalid table offset"));
    Bank->BankBytes = Original;
    ++Bank->Entries[0].Offset;
    Reject(Bank, TEXT("Reject mismatched entry metadata"));
    --Bank->Entries[0].Offset;
    Bank->Entries[0].Size = MAX_int32;
    Reject(Bank, TEXT("Reject truncated entry bytes"));
    Bank = ImportSectorBytes(Original);
    Bank->Entries[0].Type = 6;
    Bank->Entries[0].Subtype = 1;
    Reject(Bank, TEXT("Scene data without configuration must not use sector writer"));
    TArray<uint8> MissingTexture = Original;
    PutU32(MissingTexture, 0x48 + Bank->Entries.Num() * 16 + 4, 0x990001);
    Bank = ImportSectorBytes(MissingTexture);
    Reject(Bank, TEXT("Reject non-sector banks without texture entry"));

    UKyaLevelBank* LitBank = ImportSectorBytes(MakeSectorBytes(true, true));
    LitBank->Lights[0].TrailingValues.Add(1);
    Reject(LitBank, TEXT("Reject resized light arrays"));
    LitBank->Lights[0].TrailingValues.Pop();
    LitBank->Lights[0].ShapeType = 4;
    Reject(LitBank, TEXT("Reject changed light layout"));

    // An invalid sector must prevent publication of the valid banks in its set.
    UKyaLevelBank* ValidBank = ImportSectorBytes(Original);
    const FString Directory = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("KyaSectorExportValidation"));
    IFileManager::Get().MakeDirectory(*Directory, true);
    const FString Destination = FPaths::Combine(Directory, TEXT("SECT1.bnk"));
    const TArray<uint8> Sentinel = {9, 8, 7};
    TestTrue(TEXT("Write existing destination"), FFileHelper::SaveArrayToFile(Sentinel, *Destination));
    FKyaBankExportRequest ValidRequest{ValidBank, Destination};
    FKyaBankExportRequest InvalidRequest{LitBank, FPaths::Combine(Directory, TEXT("SECT2.bnk"))};
    const TArray<FKyaBankExportOutcome> Outcomes = KyaWriteValidatedExportSet({ValidRequest, InvalidRequest});
    if (TestEqual(TEXT("Both outcomes reported"), Outcomes.Num(), 2))
    {
        TestFalse(TEXT("Valid sector not published from incomplete set"), Outcomes[0].bPublished);
        TestFalse(TEXT("Invalid sector not published"), Outcomes[1].bPublished);
    }
    TArray<uint8> After;
    TestTrue(TEXT("Read existing destination"), FFileHelper::LoadFileToArray(After, *Destination));
    TestTrue(TEXT("Failed set preserves existing output"), After == Sentinel);
    TestFalse(TEXT("Failed set removes staged sector"), FPaths::FileExists(Destination + TEXT(".kya-staging-0")));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKyaRealSectorExportTest, "Kya.Bank.RealSectorExport",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKyaRealSectorExportTest::RunTest(const FString& Parameters)
{
    const FString MainFilename = FPlatformMisc::GetEnvironmentVariable(TEXT("KYA_TEST_BANK"));
    if (MainFilename.IsEmpty())
    {
        AddWarning(TEXT("Set KYA_TEST_BANK to validate and export a main bank with all its sibling sectors."));
        return true;
    }
    const FString Directory = FPaths::GetPath(MainFilename);
    TArray<FString> Files;
    IFileManager::Get().FindFiles(Files, *FPaths::Combine(Directory, TEXT("SECT*.BNK")), true, false);
    if (!TestTrue(TEXT("Real level has sector files"), Files.Num() > 0)) return false;
    TArray<UKyaLevelBank*> Banks;
    Files.Insert(FPaths::GetCleanFilename(MainFilename), 0);
    const FString Destination = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("KyaRealSectorExport"));
    TArray<TArray<uint8>> Originals;
    for (const FString& File : Files)
    {
        TArray<uint8>& Original = Originals.AddDefaulted_GetRef();
        if (!TestTrue(File + TEXT(" read"), FFileHelper::LoadFileToArray(Original, *FPaths::Combine(Directory, File)))) return false;
        UKyaLevelBank* Bank = ImportSectorBytes(Original);
        if (!TestNotNull(File + TEXT(" import"), Bank)) return false;
        Bank->Rename(*FPaths::GetBaseFilename(File));
        Banks.Add(Bank);
        const FKyaBankValidationReport Report = KyaValidateBank(Bank);
        if (!TestTrue(File + TEXT(" validates: ") + Report.UnsupportedReason, Report.bBytesBuilt)) return false;
    }
    TArray<FString> Messages;
    if (!TestTrue(TEXT("Export complete level including every sector"), KyaExportLevelBanks(Banks, Destination, Messages))) return false;
    for (int32 Index = 0; Index < Banks.Num(); ++Index)
    {
        TArray<uint8> Exported;
        const FString Output = FPaths::Combine(Destination, Banks[Index]->GetName() + TEXT(".bnk"));
        if (!TestTrue(Files[Index] + TEXT(" published"), FFileHelper::LoadFileToArray(Exported, *Output))) continue;
        if (Index > 0) TestTrue(Files[Index] + TEXT(" byte-identical"), Exported == Originals[Index]);
        UKyaLevelBank* Reimported = ImportSectorBytes(Exported);
        if (TestNotNull(Files[Index] + TEXT(" reimport"), Reimported))
            TestEqual(Files[Index] + TEXT(" entry count"), Reimported->Entries.Num(), Banks[Index]->Entries.Num());
    }
    AddInfo(FString::Printf(TEXT("Exported and reimported main bank plus %d byte-identical sectors."), Banks.Num() - 1));
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKyaActorMeshLevelExportTest, "Kya.Bank.ActorMeshLevelExport",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKyaActorMeshLevelExportTest::RunTest(const FString& Parameters)
{
    const FString Filename = FPlatformMisc::GetEnvironmentVariable(TEXT("KYA_TEST_BANK"));
    if (Filename.IsEmpty())
    {
        AddWarning(TEXT("Set KYA_TEST_BANK to run the actor mesh level export regression."));
        return true;
    }
    const FString Directory = FPaths::GetPath(Filename);
    TArray<FString> Files;
    IFileManager::Get().FindFiles(Files, *FPaths::Combine(Directory, TEXT("SECT*.BNK")), true, false);
    if (!TestTrue(TEXT("Level has sibling sectors"), !Files.IsEmpty())) return false;
    Files.Sort();
    Files.Insert(FPaths::GetCleanFilename(Filename), 0);
    TArray<UKyaLevelBank*> Banks;
    TArray<TArray<uint8>> Originals;
    for (const FString& File : Files)
    {
        TArray<uint8>& Bytes = Originals.AddDefaulted_GetRef();
        if (!TestTrue(File + TEXT(" read"), FFileHelper::LoadFileToArray(Bytes, *FPaths::Combine(Directory, File)))) return false;
        UKyaLevelBank* Bank = ImportSectorBytes(Bytes);
        if (!TestNotNull(File + TEXT(" import"), Bank)) return false;
        Bank->Rename(*FPaths::GetBaseFilename(File));
        Banks.Add(Bank);
    }
    UKyaLevelBank* Main = Banks[0];
    const TArray<uint8>& Input = Originals[0];
    const auto ReadWord = [&Input](int64 Offset)
    {
        return uint32(Input[Offset]) | (uint32(Input[Offset + 1]) << 8) |
            (uint32(Input[Offset + 2]) << 16) | (uint32(Input[Offset + 3]) << 24);
    };
    const FKyaBankEntry* Scene = Main->Entries.FindByPredicate(
        [](const FKyaBankEntry& Entry) { return Entry.Type == 6 && Entry.Subtype == 1; });
    if (!TestNotNull(TEXT("Main scene entry"), Scene)) return false;
    int64 Setup = Scene->Offset + 20;
    if (ReadWord(Scene->Offset + 16)) Setup += 12;
    Setup += 4 + (int32(ReadWord(Setup)) != -1 ? 4 : 0);
    int64 Config = 0, Values = 0, MapEnd = 0, ScenarioEnd = 0, ObjectiveEnd = 0, Waypoints = 0;
    int32 WaypointCount = 0;
    TArray<FKyaConditionedOffsets> Scenarios;
    TArray<FKyaObjectiveOffsets> Objectives;
    TArray<FKyaActorOffsets> Offsets;
    if (!TestTrue(TEXT("Locate serialized mesh fields"),
        KyaFindCameraConfig(Input, Setup, Config) && KyaFindSceneAfterCamera(Input, Config, Values) &&
        KyaFindMapEnd(Input, Values + 12, MapEnd) &&
        KyaReadConditionedOffsets(Input, MapEnd + 48, Scenarios, ScenarioEnd) &&
        KyaReadObjectiveOffsets(Input, ScenarioEnd, Objectives, ObjectiveEnd) &&
        KyaFindWaypoints(Input, ObjectiveEnd, Waypoints, WaypointCount) &&
        KyaReadActorOffsets(Input, Waypoints, Scene->Offset, Offsets))) return false;
    const TArray<int32> Meshes = GetKyaModelOrdinals(*Main);
    int32 ActorIndex = INDEX_NONE, Replacement = INDEX_NONE;
    for (int32 Index = 0; Index < Main->SceneEnvironment.Actors.Num(); ++Index)
    {
        const FKyaActorPlacement& Actor = Main->SceneEnvironment.Actors[Index];
        if (Actor.SectorId < 0 || Actor.MeshIndex < 0) continue;
        const FString SectorName = FString::Printf(TEXT("SECT%d"), Actor.SectorId);
        if (!Banks.ContainsByPredicate([&](const UKyaLevelBank* Bank) { return Bank->GetName().Equals(SectorName, ESearchCase::IgnoreCase); })) continue;
        for (int32 Mesh : Meshes)
            if (Mesh != Actor.MeshIndex) { ActorIndex = Index; Replacement = Mesh; break; }
        if (ActorIndex != INDEX_NONE) break;
    }
    if (!TestTrue(TEXT("Find meshed actor with a loaded sector and alternative mesh"), ActorIndex != INDEX_NONE)) return false;
    if (!TestTrue(TEXT("Actor offset exists"), Offsets.IsValidIndex(ActorIndex))) return false;
    FKyaActorPlacement Copy = Main->SceneEnvironment.Actors[ActorIndex];
    Copy.MeshIndex = Replacement;
    FKyaActorEditPatch Patch;
    FString Error;
    if (!TestTrue(TEXT("Build mesh picker property patch"), KyaMakePlacementPropertyPatch(Copy,
        GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, MeshIndex), Patch, &Error))) return false;
    FKyaLevelEditService& Service = FKyaLevelEditService::Get();
    if (!TestTrue(TEXT("Apply mesh edit"), Service.Apply(Service.MakeKey(Main, ActorIndex), Patch, EKyaEditNotify::Silent, &Error))) return false;
    TArray<uint8> Expected = Input;
    PutU32(Expected, int32(Offsets[ActorIndex].Placement), uint32(Replacement));
    if (!TestTrue(TEXT("Expected mesh edit changes source bytes"), Expected != Input)) return false;
    const FString Destination = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("KyaActorMeshLevelExport"));
    TArray<FString> Messages;
    if (!TestTrue(TEXT("Export main bank and all sectors"), KyaExportLevelBanks(Banks, Destination, Messages))) return false;
    for (int32 Index = 0; Index < Banks.Num(); ++Index)
    {
        TArray<uint8> Exported;
        if (!TestTrue(Files[Index] + TEXT(" published"), FFileHelper::LoadFileToArray(Exported,
            *FPaths::Combine(Destination, Banks[Index]->GetName() + TEXT(".bnk"))))) return false;
        TestTrue(Files[Index] + TEXT(" exact expected bytes"), Exported == (Index == 0 ? Expected : Originals[Index]));
        if (Index == 0)
        {
            UKyaLevelBank* Reimported = ImportSectorBytes(Exported);
            if (!TestNotNull(TEXT("Reimport edited main bank"), Reimported)) return false;
            if (!TestEqual(TEXT("Actor count preserved"), Reimported->SceneEnvironment.Actors.Num(), Main->SceneEnvironment.Actors.Num())) return false;
            TestEqual(TEXT("Mesh edit survives reimport"), Reimported->SceneEnvironment.Actors[ActorIndex].MeshIndex, Replacement);
            TestEqual(TEXT("Sector assignment preserved"), Reimported->SceneEnvironment.Actors[ActorIndex].SectorId, Copy.SectorId);
        }
    }
    AddInfo(FString::Printf(TEXT("Actor %d mesh changed to %d; verified mesh word and %d unchanged sectors."), ActorIndex, Replacement, Banks.Num() - 1));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKyaSectorCorpusRoundTripTest, "Kya.Bank.SectorCorpusRoundTrip",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKyaSectorCorpusRoundTripTest::RunTest(const FString& Parameters)
{
    const FString Root = FPlatformMisc::GetEnvironmentVariable(TEXT("KYA_TEST_SECTOR_ROOT"));
    if (Root.IsEmpty())
    {
        AddWarning(TEXT("Set KYA_TEST_SECTOR_ROOT to round-trip every sector beneath a level directory tree."));
        return true;
    }
    TArray<FString> Files;
    IFileManager::Get().FindFilesRecursive(Files, *Root, TEXT("SECT*.BNK"), true, false);
    if (!TestTrue(TEXT("Sector corpus contains banks"), Files.Num() > 0)) return false;
    for (const FString& File : Files)
    {
        TArray<uint8> Original;
        if (!TestTrue(File + TEXT(" read"), FFileHelper::LoadFileToArray(Original, *File))) return false;
        UKyaLevelBank* Bank = ImportSectorBytes(Original);
        if (!TestNotNull(File + TEXT(" import"), Bank)) return false;
        FKyaBankExportDiagnostics Diagnostics;
        TArray<uint8> Exported;
        if (!TestTrue(File + TEXT(" export: ") + Diagnostics.Summary(), KyaBuildBankExportBytes(Bank, Exported, Diagnostics)))
        {
            AddError(Diagnostics.Summary());
            return false;
        }
        if (!TestTrue(File + TEXT(" byte-identical"), Original == Exported)) return false;
        UKyaLevelBank* Reimported = ImportSectorBytes(Exported);
        if (!TestNotNull(File + TEXT(" reimport"), Reimported)) return false;
        TestEqual(File + TEXT(" entry count"), Reimported->Entries.Num(), Bank->Entries.Num());
    }
    AddInfo(FString::Printf(TEXT("Round-tripped %d byte-identical sector banks across %s."), Files.Num(), *Root));
    return true;
}
#endif
