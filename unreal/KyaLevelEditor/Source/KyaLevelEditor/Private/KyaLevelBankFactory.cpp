#include "KyaLevelBankFactory.h"
#include "Actors/KyaHunter.h"
#include "Actors/KyaHelperSign.h"
#include "Actors/KyaWoodMonster.h"
#include "Actors/KyaShockerCmd.h"
#include "Actors/KyaAcceleratos.h"
#include "Actors/KyaAddOnPurchase.h"
#include "Actors/KyaAmortosCmd.h"
#include "Actors/KyaPatternCmd.h"
#include "Actors/KyaEgg.h"
#include "Actors/KyaCareBoy.h"
#include "Actors/KyaElectrolla.h"
#include "Actors/KyaBombLauncher.h"
#include "Actors/KyaBlazer.h"
#include "Actors/KyaWoof.h"
#include "Actors/KyaMiniGamesSupport.h"
#include "Actors/KyaCredits.h"
#include "Actors/KyaWantedZoo.h"
#include "Actors/KyaJamGut.h"
#include "KyaActorCatalog.h"
#include "KyaLevelBank.h"
#include "Misc/ScopedSlowTask.h"
#include "Misc/App.h"
#include "KyaCameraStream.h"
#include "Actors/KyaEventGenerator.h"
#include "Actors/KyaExplosiveDistributor.h"
#include "Actors/KyaShocker.h"
#include "Actors/KyaNativShop.h"
#include "Actors/KyaPunchingBall.h"
#include "Actors/KyaNoseMonster.h"
#include "Actors/KyaPattern.h"
#include "Actors/KyaShip.h"
#include "Actors/KyaBunch.h"
#include "Actors/KyaBrazul.h"
#include "Actors/KyaMiniGame.h"
#include "Actors/KyaHedgehog.h"
#include "Actors/KyaProjectile.h"
#include "Actors/KyaMovingPlatform.h"
#include "Actors/KyaSwitch.h"
#include "Actors/KyaWind.h"
#include "Actors/KyaTrap.h"
#include "Actors/KyaWeapon.h"
#include "Actors/KyaAraignos.h"
#include "Actors/KyaBonus.h"
#include "Actors/KyaTeleporter.h"
#include "Actors/KyaAmortos.h"
#include "Actors/KyaDca.h"
#include "Actors/KyaFruit.h"
#include "Actors/KyaNativ.h"
#include "EditorFramework/AssetImportData.h"
#include "Misc/FeedbackContext.h"

static bool KyaPopulateFighterPrefix(FKyaActorPlacement& Actor, const uint8* Data, int32 DataSize, const FKyaFighterPrefixOffsets& Offsets)
{
    const int64 F = Offsets.Fixed;
    auto ReadU32 = [](const uint8* P) -> uint32 {
        return uint32(P[0]) | (uint32(P[1]) << 8) | (uint32(P[2]) << 16) | (uint32(P[3]) << 24);
    };
    auto ReadFloat = [&ReadU32, Data](int64 Offset) -> float {
        const uint32 Bits = ReadU32(Data + Offset);
        float Value;
        FMemory::Memcpy(&Value, &Bits, sizeof(Value));
        return Value;
    };
    Actor.FighterWeightB = ReadFloat(Offsets.Weight);
    Actor.FighterLifeMax = int64(ReadU32(Data + Offsets.Autonomous));
    Actor.FighterGravityScale = ReadFloat(Offsets.Autonomous + 4);
    Actor.FighterDynamicExtField4 = ReadFloat(Offsets.Autonomous + 8);
    Actor.FighterDynamicExtField8 = ReadFloat(Offsets.Autonomous + 12);

                                                    Actor.FighterInputAnalyserFlag = int32(ReadU32(Data + F));
                                                    Actor.FighterCommandMask = int32(ReadU32(Data + F + 4));
                                                    Actor.FighterField7C0 = ReadFloat(F + 8);
                                                    Actor.FighterField7C4 = ReadFloat(F + 12);
                                                    Actor.FighterField7D4 = ReadFloat(F + 16);
                                                    Actor.FighterField7CC = ReadFloat(F + 20);
                                                    Actor.FighterAnatomy20 = ReadFloat(F + 24);
                                                    Actor.FighterAnatomy28 = ReadFloat(F + 28);
                                                    Actor.FighterAnatomy24 = ReadFloat(F + 32);
                                                    Actor.FighterAnatomy2C = ReadFloat(F + 36);
                                                    Actor.FighterField810A = FVector(ReadFloat(F + 40), ReadFloat(F + 44), ReadFloat(F + 48));
                                                    Actor.FighterField810B = FVector(ReadFloat(F + 52), ReadFloat(F + 56), ReadFloat(F + 60));
                                                    Actor.FighterField800 = int32(ReadU32(Data + F + 64));
                                                    Actor.FighterFightFlags = int32(ReadU32(Data + F + 68));
                                                    Actor.FighterField50C = ReadFloat(F + 72);
                                                    Actor.FighterField510 = ReadFloat(F + 76);
                                                    Actor.FighterField514 = ReadFloat(F + 80);
                                                    Actor.FighterField518 = int32(ReadU32(Data + F + 84));
                                                    Actor.FighterField58 = int32(ReadU32(Data + F + 88));
                                                    for (int32 Index = 0; Index < 8; ++Index)
                                                        Actor.FighterField51C.Add(int32(ReadU32(Data + F + 92 + Index * 4)));
                                                    for (int32 Index = 0; Index < 5; ++Index)
                                                        Actor.FighterField53C.Add(int32(ReadU32(Data + F + 124 + Index * 4)));
                                                    Actor.FighterStaticMeshTextureIndex = int32(ReadU32(Data + F + 144));
                                                    Actor.FighterStaticMeshMeshIndex = int32(ReadU32(Data + F + 148));
                                                    Actor.FighterField5F0 = int32(ReadU32(Data + F + 152));
                                                    Actor.FighterWeaponActorIndex = int32(ReadU32(Data + Offsets.WeaponIndex));
                                                    int64 BlowPos = Offsets.Blows;
                                                    const uint32 BlowCount = ReadU32(Data + BlowPos); BlowPos += 4;
                                                    for (uint32 Index = 0; Index < BlowCount; ++Index)
                                                    {
                                                        FKyaFighterBlowOffsets Off;
                                                        if (!KyaReadFighterBlowOffsets(MakeArrayView(Data, DataSize), BlowPos, int64(DataSize), Off)) return false;
                                                        FKyaFighterBlow& Blow = Actor.FighterBlows.AddDefaulted_GetRef();
                                                        Blow.Name = ANSI_TO_TCHAR(reinterpret_cast<const char*>(Data + Off.Name));
                                                        Blow.NameCapacity = Off.NameCapacity;
                                                        Blow.Field4A = int32(ReadU32(Data + Off.Field4));
                                                        Blow.Field4B = int32(ReadU32(Data + Off.Field4 + 4));
                                                        Blow.Field4C = int32(ReadU32(Data + Off.Field4 + 8));
                                                        Blow.Field8 = ReadFloat(Off.Field8);
                                                        Blow.FieldC = ReadFloat(Off.Field8 + 4);
                                                        Blow.Field10 = ReadFloat(Off.Field8 + 8);
                                                        Blow.Damage = ReadFloat(Off.Field8 + 12);
                                                        Blow.Field18 = ReadFloat(Off.Field8 + 16);
                                                        Blow.Field20 = FVector(ReadFloat(Off.Field20), ReadFloat(Off.Field20 + 4), ReadFloat(Off.Field20 + 8));
                                                        Blow.Field30 = FVector(ReadFloat(Off.Field30), ReadFloat(Off.Field30 + 4), ReadFloat(Off.Field30 + 8));
                                                        Blow.Field40 = int32(ReadU32(Data + Off.Field40));
                                                        for (int32 Sub = 0; Sub < Off.SubObjCount; ++Sub)
                                                        {
                                                            FKyaFighterBlowSubObj& Item = Blow.SubObjs.AddDefaulted_GetRef();
                                                            const int64 S = Off.SubObjs + int64(Sub) * 28;
                                                            Item.BoneA = int32(ReadU32(Data + S));
                                                            Item.BoneB = int32(ReadU32(Data + S + 4));
                                                            Item.MaterialId = int32(ReadU32(Data + S + 8));
                                                            Item.Field14 = int32(ReadU32(Data + S + 12));
                                                            Item.Flags = int32(ReadU32(Data + S + 16));
                                                            Item.Field1C = ReadFloat(S + 20);
                                                            Item.Field20 = int32(ReadU32(Data + S + 24));
                                                        }
                                                        Blow.BoneRefA = int32(ReadU32(Data + Off.BoneRefs));
                                                        Blow.BoneRefB = int32(ReadU32(Data + Off.BoneRefs + 4));
                                                        Blow.CanActivateRange = ReadFloat(Off.CanActivateRange);
                                                        for (int32 Stage = 0; Stage < 3; ++Stage)
                                                        {
                                                            FKyaFighterStage& Item = Blow.Stages.AddDefaulted_GetRef();
                                                            const int64 S = Off.Stages + int64(Stage) * 28;
                                                            Item.AnimId = int32(ReadU32(Data + S));
                                                            Item.Field4 = ReadFloat(S + 4);
                                                            Item.Field8 = ReadFloat(S + 8);
                                                            Item.Field10 = FVector(ReadFloat(S + 12), ReadFloat(S + 16), ReadFloat(S + 20));
                                                            Item.Flags = int32(ReadU32(Data + S + 24));
                                                        }
                                                    }
                                                    int64 GrabPos = Offsets.Grabs;
                                                    const uint32 GrabCount = ReadU32(Data + GrabPos); GrabPos += 4;
                                                    for (uint32 Index = 0; Index < GrabCount; ++Index)
                                                    {
                                                        FKyaFighterGrabOffsets Off;
                                                        if (!KyaReadFighterGrabOffsets(MakeArrayView(Data, DataSize), GrabPos, int64(DataSize), Off)) return false;
                                                        FKyaFighterGrab& Grab = Actor.FighterGrabs.AddDefaulted_GetRef();
                                                        Grab.Name = ANSI_TO_TCHAR(reinterpret_cast<const char*>(Data + Off.Name));
                                                        Grab.NameCapacity = Off.NameCapacity;
                                                        Grab.Field4A = int32(ReadU32(Data + Off.Field4));
                                                        Grab.Field4B = int32(ReadU32(Data + Off.Field4 + 4));
                                                        Grab.Field4C = int32(ReadU32(Data + Off.Field4 + 8));
                                                        Grab.Field8 = ReadFloat(Off.Field8); Grab.FieldC = ReadFloat(Off.Field8 + 4); Grab.Field10 = ReadFloat(Off.Field8 + 8);
                                                        Grab.Field14 = int32(ReadU32(Data + Off.Field14));
                                                        Grab.Field20 = FVector(ReadFloat(Off.Field20), ReadFloat(Off.Field20 + 4), ReadFloat(Off.Field20 + 8));
                                                        Grab.Field30 = ReadFloat(Off.Field30); Grab.Field34 = ReadFloat(Off.Field34);
                                                        Grab.Field38 = int32(ReadU32(Data + Off.Field38)); Grab.Field3C = ReadFloat(Off.Field3C);
                                                        Grab.Field60 = ReadFloat(Off.Field60); Grab.Field40 = ReadFloat(Off.Field40);
                                                        Grab.Field44 = ReadFloat(Off.Field44); Grab.Field48 = ReadFloat(Off.Field48);
                                                        Grab.Field50 = FVector(ReadFloat(Off.Field50), ReadFloat(Off.Field50 + 4), ReadFloat(Off.Field50 + 8));
                                                        Grab.Field68 = int32(ReadU32(Data + Off.Field68));
                                                        Grab.Skip = int32(ReadU32(Data + Off.Field68 + 4));
                                                        for (int32 Stage = 0; Stage < 5; ++Stage)
                                                        {
                                                            FKyaFighterStage& Item = Grab.Stages.AddDefaulted_GetRef();
                                                            const int64 S = Off.Stages + int64(Stage) * 28;
                                                            Item.AnimId = int32(ReadU32(Data + S)); Item.Field4 = ReadFloat(S + 4); Item.Field8 = ReadFloat(S + 8);
                                                            Item.Field10 = FVector(ReadFloat(S + 12), ReadFloat(S + 16), ReadFloat(S + 20));
                                                            Item.Flags = int32(ReadU32(Data + S + 24));
                                                        }
                                                    }
                                                    int64 ReactPos = Offsets.GrabReacts;
                                                    const uint32 ReactCount = ReadU32(Data + ReactPos); ReactPos += 4;
                                                    for (uint32 Index = 0; Index < ReactCount; ++Index)
                                                    {
                                                        FKyaFighterGrabReact& React = Actor.FighterGrabReacts.AddDefaulted_GetRef();
                                                        const int64 NameStart = ReactPos;
                                                        if (!KyaSkipString(MakeArrayView(Data, DataSize), ReactPos, int64(DataSize))) return false;
                                                        React.Name = ANSI_TO_TCHAR(reinterpret_cast<const char*>(Data + NameStart));
                                                        React.NameCapacity = int32(ReactPos - NameStart);
                                                        React.Field4 = int32(ReadU32(Data + ReactPos));
                                                        React.Field8 = int32(ReadU32(Data + ReactPos + 4));
                                                        ReactPos += 8;
                                                    }
                                                    int64 ComboPos = Offsets.Combos;
                                                    const uint32 ComboCount = ReadU32(Data + ComboPos); ComboPos += 4;
                                                    for (uint32 Index = 0; Index < ComboCount; ++Index)
                                                    {
                                                        FKyaFighterComboOffsets Off;
                                                        if (!KyaReadFighterComboOffsets(MakeArrayView(Data, DataSize), ComboPos, int64(DataSize), Off)) return false;
                                                        FKyaFighterCombo& Combo = Actor.FighterCombos.AddDefaulted_GetRef();
                                                        Combo.Name = ANSI_TO_TCHAR(reinterpret_cast<const char*>(Data + Off.Name));
                                                        Combo.NameCapacity = Off.NameCapacity;
                                                        Combo.Field4A = int32(ReadU32(Data + Off.Field4));
                                                        Combo.Field4B = int32(ReadU32(Data + Off.Field4 + 4));
                                                        Combo.Field8 = ReadFloat(Off.Field8);
                                                        for (int32 Input = 0; Input < Off.InputCount; ++Input)
                                                            Combo.Inputs.Add(int32(ReadU32(Data + Off.Inputs + Input * 4)));
                                                        Combo.Field2 = int32(ReadU32(Data + Off.Field2));
                                                        Combo.Field3 = int32(ReadU32(Data + Off.Field3));
                                                        Combo.FieldC = ReadFloat(Off.FieldC);
                                                        Combo.Action = ANSI_TO_TCHAR(reinterpret_cast<const char*>(Data + Off.Action));
                                                        Combo.ActionCapacity = Off.ActionCapacity;
                                                        for (int32 Branch = 0; Branch < Off.BranchOffsets.Num(); ++Branch)
                                                        {
                                                            Combo.Branches.Add(ANSI_TO_TCHAR(reinterpret_cast<const char*>(Data + Off.BranchOffsets[Branch])));
                                                            Combo.BranchCapacities.Add(Off.BranchCapacities[Branch]);
                                                        }
                                                    }
    return true;
}

UKyaLevelBankFactory::UKyaLevelBankFactory()
{
    SupportedClass = UKyaLevelBank::StaticClass();
    bCreateNew = false;
    bEditorImport = true;
    Formats.Add(TEXT("bnk;Kya level bank"));
}

UObject* UKyaLevelBankFactory::FactoryCreateBinary(UClass* InClass, UObject* InParent,
    FName InName, EObjectFlags Flags, UObject* Context, const TCHAR* Type,
    const uint8*& Buffer, const uint8* BufferEnd, FFeedbackContext* Warn)
{
    const int64 Size = BufferEnd - Buffer;
    if (Size < 0x48 || Size > MAX_int32 || FMemory::Memcmp(Buffer + 8, "KNAB", 4) != 0)
    {
        Warn->Logf(ELogVerbosity::Error, TEXT("Not a valid Kya bank header."));
        return nullptr;
    }
    auto ReadU32 = [](const uint8* P) -> uint32 {
        return uint32(P[0]) | (uint32(P[1]) << 8) | (uint32(P[2]) << 16) | (uint32(P[3]) << 24);
    };
    // Packed banks need the Eden LZ77 unpacking and relocation pass before
    // their entry offsets can be interpreted as uncompressed data.
    if ((ReadU32(Buffer + 0x0c) & 1) != 0)
    {
        Warn->Logf(ELogVerbosity::Error, TEXT("Packed Kya banks are not supported yet."));
        return nullptr;
    }
    const uint32 Count = ReadU32(Buffer + 0x28);
    const uint64 Table = uint64(ReadU32(Buffer + 0x30)) + 8;
    if (Count > MAX_int32 || Table > uint64(Size) || uint64(Count) * 16 > uint64(Size) - Table)
    {
        Warn->Logf(ELogVerbosity::Error, TEXT("Invalid Kya bank entry table."));
        return nullptr;
    }
    const uint32 CommonType = ReadU32(Buffer + 0x14);
    const uint64 TypeTable = uint64(ReadU32(Buffer + 0x34)) + 8;
    if (CommonType == 0 && (TypeTable > uint64(Size) || uint64(Count) * 4 > uint64(Size) - TypeTable))
    {
        Warn->Logf(ELogVerbosity::Error, TEXT("Invalid Kya bank type table."));
        return nullptr;
    }
    TArray<FKyaBankEntry> Entries;
    Entries.Reserve(int32(Count));
    for (uint32 Index = 0; Index < Count; ++Index)
    {
        const uint8* Record = Buffer + Table + uint64(Index) * 16;
        FKyaBankEntry Entry;
        Entry.Offset = ReadU32(Record);
        Entry.Size = ReadU32(Record + 4);
        if (Entry.Offset > Size || Entry.Size > Size - Entry.Offset)
        {
            Warn->Logf(ELogVerbosity::Error, TEXT("Kya bank entry %u lies outside the file."), Index);
            return nullptr;
        }
        const uint32 Pair = CommonType ? CommonType : ReadU32(Buffer + TypeTable + uint64(Index) * 4);
        Entry.Subtype = Pair & 0xffff;
        Entry.Type = Pair >> 16;
        Entries.Add(Entry);
    }
    UKyaLevelBank* Bank = NewObject<UKyaLevelBank>(InParent, InClass, InName, Flags);
    Bank->ImportSchemaVersion = UKyaLevelBank::CurrentImportSchemaVersion;
    Bank->SourceFilename = CurrentFilename;
    Bank->BumpEditGeneration();
    Bank->AssetImportData = NewObject<UAssetImportData>(Bank);
    if (!CurrentFilename.IsEmpty()) Bank->AssetImportData->Update(CurrentFilename);
    Bank->EntryCount = int32(Count);
    TUniquePtr<FScopedSlowTask> ImportProgress;
    if (!FApp::IsUnattended())
    {
        ImportProgress = MakeUnique<FScopedSlowTask>(100.0f,
            NSLOCTEXT("KyaLevelEditor", "KyaImport", "Importing Kya bank"));
        ImportProgress->MakeDialog(true);
    }
    const auto ImportCancelled = [&ImportProgress, Warn]()
    {
        if (!ImportProgress) return false;
        ImportProgress->EnterProgressFrame();
        if (!ImportProgress->ShouldCancel()) return false;
        if (Warn) Warn->Logf(ELogVerbosity::Warning, TEXT("Kya bank import was cancelled."));
        return true;
    };
    if (ImportCancelled()) return nullptr;
    Bank->Entries = MoveTemp(Entries);
    auto FindTypedEntry = [&Bank](int32 TypeId, int32 Ordinal) -> int32 {
        if (Ordinal < 0) return INDEX_NONE;
        for (int32 Index = 0; Index < Bank->Entries.Num(); ++Index)
            if (Bank->Entries[Index].Type == TypeId && Bank->Entries[Index].Subtype == 1 && Ordinal-- == 0)
                return Index;
        return INDEX_NONE;
    };
    for (const FKyaBankEntry& Entry : Bank->Entries)
    {
        if (Entry.Type != 6 || Entry.Subtype != 1 || Entry.Size < 24) continue;
        const uint8* Data = Buffer + Entry.Offset;
        int64 Pos = 16;
        const int32 HasCluster = int32(ReadU32(Data + Pos));
        Pos += 4;
        if (HasCluster)
        {
            if (Pos + 12 > Entry.Size) break;
            Bank->SceneMeshEntry = FindTypedEntry(4, int32(ReadU32(Data + Pos)));
            Bank->SceneTextureEntry = FindTypedEntry(5, int32(ReadU32(Data + Pos + 4)));
            Pos += 12;
        }
        if (Pos + 4 > Entry.Size) break;
        const int32 Background = int32(ReadU32(Data + Pos));
        Pos += 4;
        if (Background != -1 && Pos + 4 <= Entry.Size)
        {
            Bank->BackgroundMeshEntry = FindTypedEntry(4, Background);
            Bank->BackgroundTextureEntry = FindTypedEntry(5, int32(ReadU32(Data + Pos)));
            Pos += 4;
        }
        // CScene::Level_Setup follows the two mesh references. Its first 20
        // bytes are currently unused by the game, then come flags and fog.
        if (Pos + 48 <= Entry.Size)
        {
            FKyaSceneEnvironment& Environment = Bank->SceneEnvironment;
            Environment.BackgroundFlags = ReadU32(Data + Pos + 20);
            uint32 Bits = ReadU32(Data + Pos + 24);
            FMemory::Memcpy(&Environment.ClipDistance, &Bits, sizeof(Bits));
            Bits = ReadU32(Data + Pos + 28);
            FMemory::Memcpy(&Environment.FogParameter, &Bits, sizeof(Bits));
            const uint8* Color = Data + Pos + 32;
            Environment.FogColor = FColor(Color[0], Color[1], Color[2], Color[3]);
            Environment.FogFlags = ReadU32(Data + Pos + 36);
            Environment.MipmapL = int32(ReadU32(Data + Pos + 40));
            Environment.MipmapK = int32(ReadU32(Data + Pos + 44));
            if (Pos + 112 <= Entry.Size)
            {
                auto ReadFloat = [&](int64 Offset) {
                    uint32 FloatBits = ReadU32(Data + Offset);
                    float Value;
                    FMemory::Memcpy(&Value, &FloatBits, sizeof(Value));
                    return Value;
                };
                Environment.CameraFov = ReadFloat(Pos + 48);
                Environment.CameraPosition = FVector(ReadFloat(Pos + 52), ReadFloat(Pos + 56), ReadFloat(Pos + 60));
                Environment.CameraLookAt = FVector(ReadFloat(Pos + 64), ReadFloat(Pos + 68), ReadFloat(Pos + 72));
                Environment.CameraUp = FVector(ReadFloat(Pos + 76), ReadFloat(Pos + 80), ReadFloat(Pos + 84));
                Environment.CameraSectorId = int32(ReadU32(Data + Pos + 88));
                Environment.CameraField08 = int32(ReadU32(Data + Pos + 92));
                Environment.CameraFlags = ReadU32(Data + Pos + 96);
                Environment.CameraTargetOffset = FVector(ReadFloat(Pos + 100), ReadFloat(Pos + 104), ReadFloat(Pos + 108));
                int64 Conditions = 0;
                if (KyaFindCameraConditions(MakeArrayView(Data, int32(Entry.Size)), Pos, Conditions))
                {
                    if (Environment.CameraFlags & 2)
                    {
                        Environment.bHasCameraSwitchData = true;
                        int64 Switch = Pos + 112;
                        Environment.CameraSwitchActor = int32(ReadU32(Data + Switch)); Switch += 4;
                        TArray<int32>* Lists[] = { &Environment.CameraSwitchActorsA, &Environment.CameraSwitchActorsB };
                        for (TArray<int32>* List : Lists)
                        {
                            const uint32 ListCount = ReadU32(Data + Switch); Switch += 4;
                            for (uint32 Index = 0; Index < ListCount; ++Index, Switch += 4)
                                List->Add(int32(ReadU32(Data + Switch)));
                        }
                        Environment.CameraSwitchValueA = ReadFloat(Switch);
                        Environment.CameraSwitchModeA = int32(ReadU32(Data + Switch + 4));
                        Environment.CameraSwitchValueB = ReadFloat(Switch + 8);
                        Environment.CameraSwitchModeB = int32(ReadU32(Data + Switch + 12));
                        Environment.CameraSwitchValueC = ReadFloat(Switch + 16);
                    }
                    for (uint32 Index = 0; Index < ReadU32(Data + Conditions); ++Index)
                    {
                        const int64 Offset = Conditions + 4 + int64(Index) * 16;
                        FKyaCameraCondition& Condition = Environment.CameraConditions.AddDefaulted_GetRef();
                        Condition.Combiner = int32(ReadU32(Data + Offset));
                        Condition.ActorRef = int32(ReadU32(Data + Offset + 4));
                        Condition.Type = int32(ReadU32(Data + Offset + 8));
                        Condition.Value = ReadFloat(Offset + 12);
                    }
                }
                int64 Config = 0;
                if (KyaFindCameraConfig(MakeArrayView(Data, int32(Entry.Size)), Pos, Config))
                {
                    Environment.CameraConfigFlags = ReadU32(Data + Config);
                    Environment.CameraActorRefA = int32(ReadU32(Data + Config + 4));
                    Environment.CameraActorRefB = int32(ReadU32(Data + Config + 8));
                    Environment.CameraConfigAngle = ReadFloat(Config + 12);
                    if (ReadU32(Data + Config + 16) == 1)
                        for (int32 Index = 0; Index < 5; ++Index)
                            Environment.CameraOptionalConfigValues.Add(ReadFloat(Config + 20 + Index * 4));
                    const int64 Values = Config + 20 + (ReadU32(Data + Config + 16) == 1 ? 20 : 0);
                    Environment.CameraPitchAngles = FVector(ReadFloat(Values), ReadFloat(Values + 4), ReadFloat(Values + 8));
                    Environment.CameraOtherAngles = FVector(ReadFloat(Values + 12), ReadFloat(Values + 16), ReadFloat(Values + 20));
                    Environment.CameraDistances = FVector(ReadFloat(Values + 24), ReadFloat(Values + 28), ReadFloat(Values + 32));
                    int64 SceneValues = 0;
                    if (KyaFindSceneAfterCamera(MakeArrayView(Data, int32(Entry.Size)), Config, SceneValues))
                    {
                        if (ReadU32(Data + Values + 36) == 1)
                            for (int32 Index = 0; Index < 12; ++Index)
                                Environment.CameraExtraValues.Add(ReadFloat(Values + 40 + Index * 4));
                        Environment.Field118 = ReadFloat(SceneValues);
                        Environment.Field11C = ReadU32(Data + SceneValues + 4);
                        Environment.Count120 = ReadU32(Data + SceneValues + 8);
                        const int64 Map = SceneValues + 12;
                        int64 MapEnd = 0;
                        if (KyaFindMapEnd(MakeArrayView(Data, int32(Entry.Size)), Map, MapEnd))
                        {
                            Environment.MapField04 = int32(ReadU32(Data + Map));
                            Environment.MapField08 = ReadU32(Data + Map + 4);
                            Environment.MapAngle = ReadFloat(Map + 8);
                            const uint32 MapCount = ReadU32(Data + Map + 12);
                            for (uint32 Index = 0; Index < MapCount; ++Index)
                                Environment.MapEntries.Add(int32(ReadU32(Data + Map + 16 + Index * 4)));
                            Environment.MapField14 = int32(ReadU32(Data + MapEnd));
                            Environment.MapActorRef = int32(ReadU32(Data + MapEnd + 4));
                            Environment.DefaultTextureIndex = int32(ReadU32(Data + MapEnd + 8));
                            Environment.AlternateTextureIndex = int32(ReadU32(Data + MapEnd + 12));
                            Environment.DefaultMaterialIndex = int32(ReadU32(Data + MapEnd + 16));
                            if (MapEnd + 48 <= Entry.Size)
                            {
                                for (int32 Category = 0; Category < 7; ++Category)
                                    Environment.EffectPoolCounts.Add(ReadU32(Data + MapEnd + 20 + Category * 4));
                                TArray<FKyaConditionedOffsets> Blocks;
                                int64 ScenarioEnd = 0;
                                if (KyaReadConditionedOffsets(MakeArrayView(Data, int32(Entry.Size)), MapEnd + 48, Blocks, ScenarioEnd))
                                {
                                    for (const FKyaConditionedOffsets& Block : Blocks)
                                    {
                                        FKyaConditionedOperation& Result = Environment.InitialScenarioOperations.AddDefaulted_GetRef();
                                        for (int64 Offset : Block.Operations)
                                        {
                                            FKyaScenarioOperation& Op = Result.Operations.AddDefaulted_GetRef();
                                            Op.Variable = int32(ReadU32(Data + Offset));
                                            Op.Type = int32(ReadU32(Data + Offset + 4));
                                            Op.Value = int32(ReadU32(Data + Offset + 8));
                                        }
                                        for (int64 Offset : Block.Conditions)
                                        {
                                            FKyaScenarioCondition& Condition = Result.Conditions.AddDefaulted_GetRef();
                                            Condition.Combiner = int32(ReadU32(Data + Offset));
                                            Condition.Variable = int32(ReadU32(Data + Offset + 4));
                                            Condition.Comparison = int32(ReadU32(Data + Offset + 8));
                                            Condition.Value = int32(ReadU32(Data + Offset + 12));
                                        }
                                    }
                                    TArray<FKyaObjectiveOffsets> Objectives;
                                    int64 ObjectiveEnd = 0, Trailing = 0;
                                    if (KyaReadObjectiveOffsets(MakeArrayView(Data, int32(Entry.Size)), ScenarioEnd, Objectives, ObjectiveEnd, &Trailing))
                                    {
                                        for (const FKyaObjectiveOffsets& Offsets : Objectives)
                                        {
                                            FKyaObjective& Objective = Environment.Objectives.AddDefaulted_GetRef();
                                            Objective.ScenarioVariable = int32(ReadU32(Data + Offsets.Header));
                                            for (int32 Field = 0; Field < 6; ++Field)
                                                Objective.Fields.Add(ReadU32(Data + Offsets.Header + 4 + Field * 4));
                                            for (int64 TargetOffset : Offsets.Targets)
                                            {
                                                FKyaObjectiveTarget& Target = Objective.Targets.AddDefaulted_GetRef();
                                                Target.MessageKeyA = ReadU32(Data + TargetOffset);
                                                Target.MessageKeyB = ReadU32(Data + TargetOffset + 4);
                                                Target.ActorRef = int32(ReadU32(Data + TargetOffset + 8));
                                                Target.ZoneRef = int32(ReadU32(Data + TargetOffset + 12));
                                            }
                                        }
                                        for (uint32 Index = 0; Index < ReadU32(Data + Trailing); ++Index)
                                        {
                                            FKyaObjectiveZoneEntry& Zone = Environment.ObjectiveZones.AddDefaulted_GetRef();
                                            Zone.Field = int32(ReadU32(Data + Trailing + 4 + Index * 8));
                                            Zone.ZoneRef = int32(ReadU32(Data + Trailing + 8 + Index * 8));
                                        }
                                        if (ObjectiveEnd + 12 <= Entry.Size)
                                        {
                                            Environment.Field10C = int32(ReadU32(Data + ObjectiveEnd));
                                            Environment.Field110 = int32(ReadU32(Data + ObjectiveEnd + 4));
                                            const uint32 ElevatorCount = ReadU32(Data + ObjectiveEnd + 8);
                                            if (ElevatorCount <= uint64(Entry.Size - ObjectiveEnd - 12) / 4)
                                                for (uint32 Index = 0; Index < ElevatorCount; ++Index)
                                                    Environment.ElevatorCutscenes.Add(int32(ReadU32(Data + ObjectiveEnd + 12 + Index * 4)));
                                        }
                                        int64 WaypointRecords = 0;
                                        int32 WaypointCount = 0;
                                        if (KyaFindWaypoints(MakeArrayView(Data, int32(Entry.Size)), ObjectiveEnd, WaypointRecords, WaypointCount))
                                        {
                                            for (int32 Index = 0; Index < WaypointCount; ++Index)
                                            {
                                                const int64 Offset = WaypointRecords + Index * 36;
                                                FKyaWaypoint& Waypoint = Environment.Waypoints.AddDefaulted_GetRef();
                                                Waypoint.Position = FVector(ReadFloat(Offset), ReadFloat(Offset + 4), ReadFloat(Offset + 8));
                                                Waypoint.Rotation = FVector(ReadFloat(Offset + 12), ReadFloat(Offset + 16), ReadFloat(Offset + 20));
                                                Waypoint.Parameters = FVector(ReadFloat(Offset + 24), ReadFloat(Offset + 28), ReadFloat(Offset + 32));
                                            }
                                            TArray<FKyaPathOffsets> PathOffsets;
                                            if (KyaReadPathOffsets(MakeArrayView(Data, int32(Entry.Size)), WaypointRecords, WaypointCount, 0, PathOffsets))
                                                for (const FKyaPathOffsets& Offsets : PathOffsets)
                                                {
                                                    FKyaPath& Path = Environment.Paths.AddDefaulted_GetRef();
                                                    Path.DataFlags = Offsets.Flags;
                                                    for (int32 Array = 0; Array < 7; ++Array)
                                                        if (Offsets.Arrays[Array] != INDEX_NONE)
                                                            for (uint32 Point = 0; Point < Offsets.Count; ++Point)
                                                            {
                                                                const int64 Offset = Offsets.Arrays[Array] + Point * (Array < 3 ? 16 : 4);
                                                                if (Array < 3)
                                                                {
                                                                    const FVector4 Value(ReadFloat(Offset), ReadFloat(Offset + 4), ReadFloat(Offset + 8), ReadFloat(Offset + 12));
                                                                    if (Array == 0) Path.Points.Add(Value);
                                                                    else if (Array == 1) Path.Rotations.Add(Value);
                                                                    else Path.ExtraVectors.Add(Value);
                                                                }
                                                                else if (Array == 3) Path.Delays.Add(ReadFloat(Offset));
                                                                else if (Array == 4) Path.PointFlags.Add(ReadU32(Data + Offset));
                                                                else if (Array == 5) Path.ExtraValues.Add(ReadFloat(Offset));
                                                                else Path.ExtraWords.Add(ReadU32(Data + Offset));
                                                            }
                                                    Path.Mode = ReadU32(Data + Offsets.Trailer);
                                                    Path.Type = ReadU32(Data + Offsets.Trailer + 4);
                                                    Path.PathType = ReadU32(Data + Offsets.Trailer + 8);
                                                }
                                            TArray<FKyaActorOffsets> ActorOffsets;
                                            if (KyaReadActorOffsets(MakeArrayView(Data, int32(Entry.Size)), WaypointRecords, 0, ActorOffsets))
                                                for (const FKyaActorOffsets& Offsets : ActorOffsets)
                                                {
                                                    FKyaActorPlacement& Actor = Environment.Actors.AddDefaulted_GetRef();
                                                    Actor.Index = int32(ReadU32(Data + Offsets.Chunk + 8));
                                                    Actor.ClassId = ReadU32(Data + Offsets.Chunk + 12);
                                                    Actor.Name = ANSI_TO_TCHAR(reinterpret_cast<const char*>(Data + Offsets.Name));
                                                    Actor.MeshIndex = int32(ReadU32(Data + Offsets.Placement));
                                                    Actor.TextureIndex = int32(ReadU32(Data + Offsets.Placement + 4));
                                                    Actor.CollisionIndex = int32(ReadU32(Data + Offsets.Placement + 8));
                                                    Actor.Position = FVector(ReadFloat(Offsets.Placement + 12), ReadFloat(Offsets.Placement + 16), ReadFloat(Offsets.Placement + 20));
                                                    Actor.Rotation = FVector(ReadFloat(Offsets.Placement + 24), ReadFloat(Offsets.Placement + 28), ReadFloat(Offsets.Placement + 32));
                                                    Actor.Scale = FVector(ReadFloat(Offsets.Placement + 36), ReadFloat(Offsets.Placement + 40), ReadFloat(Offsets.Placement + 44));
                                                    const int64 Common = Offsets.Common;
                                                    Actor.DefaultBehaviourId = int32(ReadU32(Data + Common));
                                                    Actor.BoundingSphere = FVector4(ReadFloat(Common + 4), ReadFloat(Common + 8), ReadFloat(Common + 12), ReadFloat(Common + 16));
                                                    Actor.Clipping = ReadFloat(Common + 20);
                                                    Actor.VisibilityDistance = ReadFloat(Common + 24);
                                                    Actor.CullingDistance = ReadFloat(Common + 28);
                                                    Actor.Field20 = ReadFloat(Common + 32);
                                                    Actor.SectorId = int32(ReadU32(Data + Common + 36));
                                                    Actor.HashCode = ReadU32(Data + Common + 40);
                                                    Actor.LodBiases = FVector2D(ReadFloat(Common + 44), ReadFloat(Common + 48));
                                                    Actor.ActorFlags = int32(ReadU32(Data + Common + 56));
                                                    Actor.AnimationLayers = int32(ReadU32(Data + Common + 60));
                                                    Actor.Field40 = int32(ReadU32(Data + Common + 64));
                                                    Actor.LightingFlags = ReadU32(Data + Common + 72);
                                                    Actor.LightingValue = ReadFloat(Common + 76);
                                                    for (int32 Index = 0; Index < Offsets.ConditionCount; ++Index)
                                                    {
                                                        const int64 Offset = Offsets.Conditions + int64(Index) * 16;
                                                        FKyaScenarioCondition& Condition = Actor.Conditions.AddDefaulted_GetRef();
                                                        Condition.Combiner = int32(ReadU32(Data + Offset));
                                                        Condition.Variable = int32(ReadU32(Data + Offset + 4));
                                                        Condition.Comparison = int32(ReadU32(Data + Offset + 8));
                                                        Condition.Value = int32(ReadU32(Data + Offset + 12));
                                                    }
                                                    if (Actor.ClassId == 0x0d)
                                                    {
                                                        Actor.bHasPlatformPrefix = KyaReadMovingPlatformActor(Actor,
                                                            MakeArrayView(Data, int32(Entry.Size)), Offsets.ClassData, Offsets.ClassDataSize);
                                                        if (!Actor.bHasPlatformPrefix)
                                                            Actor.ClassData.Append(Data + Offsets.ClassData, Offsets.ClassDataSize);
                                                    }
                                                    else Actor.ClassData.Append(Data + Offsets.ClassData, Offsets.ClassDataSize);
                                                    if (Actor.ClassId == 0x39)
                                                    {
                                                        Actor.bHasBasicBoxData = KyaReadBasicBoxActor(Actor,
                                                            MakeArrayView(Data, int32(Entry.Size)), Offsets.ClassData, Offsets.ClassDataSize);
                                                    }
                                                    if (Actor.ClassId == 0x3b)
                                                    {
                                                        Actor.bHasEventGeneratorData = KyaReadEventGeneratorActor(Actor,
                                                            MakeArrayView(Data, int32(Entry.Size)), Offsets.ClassData, Offsets.ClassDataSize);
                                                    }
                                                    if (Actor.ClassId == 0x1d)
                                                    {
                                                        Actor.bHasExplosiveDistributorData = KyaReadExplosiveDistributorActor(Actor,
                                                            MakeArrayView(Data, int32(Entry.Size)), Offsets.ClassData, Offsets.ClassDataSize);
                                                    }
                                                    if (Actor.ClassId == 0x4b)
                                                    {
                                                        Actor.bHasShockerData = KyaReadShockerActor(Actor,
                                                            MakeArrayView(Data, int32(Entry.Size)), Offsets.ClassData, Offsets.ClassDataSize);
                                                    }
                                                    if (Actor.ClassId == 0x34)
                                                    {
                                                        Actor.bHasNativShopData = KyaReadNativShopActor(Actor,
                                                            MakeArrayView(Data, int32(Entry.Size)), Offsets.ClassData, Offsets.ClassDataSize);
                                                    }
                                                    if (Actor.ClassId == 0x30)
                                                    {
                                                        Actor.bHasPunchingBallData = KyaReadPunchingBallActor(Actor,
                                                            MakeArrayView(Data, int32(Entry.Size)), Offsets.ClassData, Offsets.ClassDataSize);
                                                    }
                                                    if (Actor.ClassId == 0x26)
                                                    {
                                                        Actor.bHasNoseMonsterData = KyaReadNoseMonsterActor(Actor,
                                                            MakeArrayView(Data, int32(Entry.Size)), Offsets.ClassData, Offsets.ClassDataSize);
                                                    }
                                                    if (Actor.ClassId == 0x3f)
                                                    {
                                                        Actor.bHasPatternData = KyaReadPatternActor(Actor,
                                                            MakeArrayView(Data, int32(Entry.Size)), Offsets.ClassData, Offsets.ClassDataSize);
                                                    }
                                                    if (Actor.ClassId == 0x36)
                                                    {
                                                        Actor.bHasShipData = KyaReadShipActor(Actor,
                                                            MakeArrayView(Data, int32(Entry.Size)), Offsets.ClassData, Offsets.ClassDataSize);
                                                    }
                                                    if (Actor.ClassId == 0x4e)
                                                    {
                                                        Actor.bHasBunchData = KyaReadBunchActor(Actor,
                                                            MakeArrayView(Data, int32(Entry.Size)), Offsets.ClassData, Offsets.ClassDataSize);
                                                    }
                                                    if (Actor.ClassId == 0x4f)
                                                    {
                                                        Actor.bHasBrazulData = KyaReadBrazulActor(Actor,
                                                            MakeArrayView(Data, int32(Entry.Size)), Offsets.ClassData, Offsets.ClassDataSize);
                                                    }
                                                    if (Actor.ClassId == 0x44 || Actor.ClassId == 0x45 ||
                                                        Actor.ClassId == 0x46 || Actor.ClassId == 0x47 ||
                                                        Actor.ClassId == 0x55)
                                                    {
                                                        Actor.bHasMiniGameData = KyaReadMiniGameActor(Actor,
                                                            MakeArrayView(Data, int32(Entry.Size)), Offsets.ClassData, Offsets.ClassDataSize);
                                                    }
                                                    if (Actor.ClassId == 0x2e)
                                                    {
                                                        Actor.bHasFxData = KyaReadFxActor(Actor,
                                                            MakeArrayView(Data, int32(Entry.Size)), Offsets.ClassData, Offsets.ClassDataSize);
                                                    }
                                                    if (Actor.ClassId == 0x21)
                                                    {
                                                        Actor.bHasAraignosData = KyaReadAraignosActor(Actor,
                                                            MakeArrayView(Data, int32(Entry.Size)), Offsets.ClassData, Offsets.ClassDataSize);
                                                    }
                                                    if (Actor.ClassId == 0x22)
                                                    {
                                                        Actor.bHasHedgehogData = KyaReadHedgehogActor(Actor,
                                                            MakeArrayView(Data, int32(Entry.Size)), Offsets.ClassData, Offsets.ClassDataSize);
                                                    }
                                                    if (Actor.ClassId == 0x1c)
                                                    {
                                                        Actor.bHasProjectileData = KyaReadProjectileActor(Actor,
                                                            MakeArrayView(Data, int32(Entry.Size)), Offsets.ClassData, Offsets.ClassDataSize);
                                                    }
                                                    if (Actor.ClassId == 0x12)
                                                    {
                                                        Actor.bHasWindData = KyaReadWindActor(Actor,
                                                            MakeArrayView(Data, int32(Entry.Size)), Offsets.ClassData, Offsets.ClassDataSize);
                                                    }
                                                    if (Actor.ClassId == 0x2c)
                                                    {
                                                        Actor.bHasTrapData = KyaReadTrapActor(Actor,
                                                            MakeArrayView(Data, int32(Entry.Size)), Offsets.ClassData, Offsets.ClassDataSize);
                                                    }
                                                    if (Actor.ClassId == 0x20)
                                                    {
                                                        Actor.bHasWeaponData = KyaReadWeaponActor(Actor,
                                                            MakeArrayView(Data, int32(Entry.Size)), Offsets.ClassData, Offsets.ClassDataSize);
                                                    }
                                                    if (Actor.ClassId == 0x11)
                                                    {
                                                        Actor.bHasSwitchData = KyaReadSwitchActor(Actor,
                                                            MakeArrayView(Data, int32(Entry.Size)), Offsets.ClassData, Offsets.ClassDataSize);
                                                    }
                                                    if (Actor.ClassId == 0x0e)
                                                    {
                                                        Actor.bHasBonusData = KyaReadBonusActor(Actor,
                                                            MakeArrayView(Data, int32(Entry.Size)), Offsets.ClassData, Offsets.ClassDataSize);
                                                    }
                                                    if (Actor.ClassId == 0x07)
                                                    {
                                                        Actor.bHasMickenData = KyaReadMickenActor(Actor,
                                                            MakeArrayView(Data, int32(Entry.Size)), Offsets.ClassData, Offsets.ClassDataSize);
                                                    }
                                                    if (Actor.ClassId == 0x09)
                                                    {
                                                        FKyaWolfenOffsets Wolfen;
                                                        if (KyaReadWolfenOffsets(
                                                            MakeArrayView(Data, int32(Entry.Size)), Offsets.ClassData, Offsets.ClassDataSize, Wolfen))
                                                        {
                                                            FKyaFighterPrefixOffsets F;
                                                            F.Weight = Wolfen.FighterPrefix;
                                                            F.Autonomous = Wolfen.FighterPrefix + 4;
                                                            F.Fixed = Wolfen.FighterFixed;
                                                            F.Blows = Wolfen.Blows;
                                                            F.Grabs = Wolfen.Grabs;
                                                            F.GrabReacts = Wolfen.GrabReacts;
                                                            F.Combos = Wolfen.Combos;
                                                            F.WeaponIndex = Wolfen.WeaponIndex;
                                                            F.Tail = Wolfen.WeaponIndex + 4;
                                                            if (!KyaPopulateFighterPrefix(Actor, Data, int32(Entry.Size), F)) return nullptr;
                                                            if (!KyaReadWolfenActor(Actor,
                                                                MakeArrayView(Data, int32(Entry.Size)), Offsets.ClassData, Offsets.ClassDataSize)) return nullptr;
                                                        }
                                                    }
                                                    if (Actor.ClassId == 0x16)
                                                    {
                                                        Actor.bHasFruitData = KyaReadFruitActor(Actor,
                                                            MakeArrayView(Data, int32(Entry.Size)), Offsets.ClassData, Offsets.ClassDataSize);
                                                    }
                                                    if (Actor.ClassId == 0x13)
                                                    {
                                                        Actor.bHasCommanderData = KyaReadCommanderActor(Actor,
                                                            MakeArrayView(Data, int32(Entry.Size)), Offsets.ClassData, Offsets.ClassDataSize);
                                                    }
                                                    if (Actor.ClassId == 0x0a)
                                                    {
                                                        Actor.bHasNativData = KyaReadNativActor(Actor,
                                                            MakeArrayView(Data, int32(Entry.Size)), Offsets.ClassData, Offsets.ClassDataSize);
                                                    }
                                                    if (Actor.ClassId == 0x29)
                                                    {
                                                        Actor.bHasAmortosData = KyaReadAmortosActor(Actor,
                                                            MakeArrayView(Data, int32(Entry.Size)), Offsets.ClassData, Offsets.ClassDataSize);
                                                    }
                                                    if (Actor.ClassId == 0x24)
                                                    {
                                                        Actor.bHasTeleporterData = KyaReadTeleporterActor(Actor,
                                                            MakeArrayView(Data, int32(Entry.Size)), Offsets.ClassData, Offsets.ClassDataSize);
                                                    }
                                                    if (Actor.ClassId == 0x1b)
                                                    {
                                                        Actor.bHasDcaData = KyaReadDcaActor(Actor,
                                                            MakeArrayView(Data, int32(Entry.Size)), Offsets.ClassData, Offsets.ClassDataSize);
                                                    }
                                                    if (Actor.ClassId == 0x35)
                                                    {
                                                        Actor.bHasBoxData = KyaReadBoxActor(Actor,
                                                            MakeArrayView(Data, int32(Entry.Size)), Offsets.ClassData, Offsets.ClassDataSize);
                                                    }
                                                    if (Actor.ClassId == 0x32)
                                                    {
                                                        Actor.bHasMoneyData = KyaReadMoneyActor(Actor,
                                                            MakeArrayView(Data, int32(Entry.Size)), Offsets.ClassData, Offsets.ClassDataSize);
                                                    }
                                                    if (Actor.ClassId == 0x3d)
                                                    {
                                                        Actor.bHasFogData = KyaReadFogActor(Actor,
                                                            MakeArrayView(Data, int32(Entry.Size)), Offsets.ClassData, Offsets.ClassDataSize);
                                                    }
                                                    if (Actor.ClassId == 0x3e)
                                                    {
                                                        Actor.bHasShootData = KyaReadShootActor(Actor,
                                                            MakeArrayView(Data, int32(Entry.Size)), Offsets.ClassData, Offsets.ClassDataSize);
                                                    }
                                                    if (Actor.ClassId == 0x14)
                                                    {
                                                        Actor.bHasAmbreData = KyaReadAmbreActor(Actor,
                                                            MakeArrayView(Data, int32(Entry.Size)), Offsets.ClassData, Offsets.ClassDataSize);
                                                    }
                                                    if (Actor.ClassId == 0x48)
                                                    {
                                                        Actor.bHasHunterData = KyaReadHunterActor(Actor,
                                                            MakeArrayView(Data, int32(Entry.Size)), Offsets.ClassData, Offsets.ClassDataSize);
                                                    }
                                                    if (Actor.ClassId == 0x2b)
                                                    {
                                                        Actor.bHasHelperSignData = KyaReadHelperSignActor(Actor,
                                                            MakeArrayView(Data, int32(Entry.Size)), Offsets.ClassData, Offsets.ClassDataSize);
                                                    }
                                                    if (Actor.ClassId == 0x27)
                                                    {
                                                        Actor.bHasWoodMonsterData = KyaReadWoodMonsterActor(Actor,
                                                            MakeArrayView(Data, int32(Entry.Size)), Offsets.ClassData, Offsets.ClassDataSize);
                                                    }
                                                    if (Actor.ClassId == 0x50)
                                                    {
                                                        Actor.bHasShockerCmdData = KyaReadShockerCmdActor(Actor,
                                                            MakeArrayView(Data, int32(Entry.Size)), Offsets.ClassData, Offsets.ClassDataSize);
                                                    }
                                                    if (Actor.ClassId == 0x3a)
                                                    {
                                                        Actor.bHasAcceleratosData = KyaReadAcceleratosActor(Actor,
                                                            MakeArrayView(Data, int32(Entry.Size)), Offsets.ClassData, Offsets.ClassDataSize);
                                                    }
                                                    if (Actor.ClassId == 0x49)
                                                    {
                                                        Actor.bHasAddOnPurchaseData = KyaReadAddOnPurchaseActor(Actor,
                                                            MakeArrayView(Data, int32(Entry.Size)), Offsets.ClassData, Offsets.ClassDataSize);
                                                    }
                                                    if (Actor.ClassId == 0x31)
                                                    {
                                                        Actor.bHasAmortosCmdData = KyaReadAmortosCmdActor(Actor,
                                                            MakeArrayView(Data, int32(Entry.Size)), Offsets.ClassData, Offsets.ClassDataSize);
                                                    }
                                                    if (Actor.ClassId == 0x40)
                                                    {
                                                        Actor.bHasPatternCmdData = KyaReadPatternCmdActor(Actor,
                                                            MakeArrayView(Data, int32(Entry.Size)), Offsets.ClassData, Offsets.ClassDataSize);
                                                    }
                                                    if (Actor.ClassId == 0x2f)
                                                    {
                                                        Actor.bHasEggData = KyaReadEggActor(Actor,
                                                            MakeArrayView(Data, int32(Entry.Size)), Offsets.ClassData, Offsets.ClassDataSize);
                                                    }
                                                    if (Actor.ClassId == 0x37)
                                                    {
                                                        Actor.bHasCareBoyData = KyaReadCareBoyActor(Actor,
                                                            MakeArrayView(Data, int32(Entry.Size)), Offsets.ClassData, Offsets.ClassDataSize);
                                                    }
                                                    if (Actor.ClassId == 0x0b)
                                                    {
                                                        Actor.bHasElectrollaData = KyaReadElectrollaActor(Actor,
                                                            MakeArrayView(Data, int32(Entry.Size)), Offsets.ClassData, Offsets.ClassDataSize);
                                                    }
                                                    if (Actor.ClassId == 0x53)
                                                    {
                                                        Actor.bHasBombLauncherData = KyaReadBombLauncherActor(Actor,
                                                            MakeArrayView(Data, int32(Entry.Size)), Offsets.ClassData, Offsets.ClassDataSize);
                                                    }
                                                    if (Actor.ClassId == 0x4d)
                                                    {
                                                        Actor.bHasBlazerData = KyaReadBlazerActor(Actor,
                                                            MakeArrayView(Data, int32(Entry.Size)), Offsets.ClassData, Offsets.ClassDataSize);
                                                    }
                                                    if (Actor.ClassId == 0x2a)
                                                    {
                                                        Actor.bHasWoofData = KyaReadWoofActor(Actor,
                                                            MakeArrayView(Data, int32(Entry.Size)), Offsets.ClassData, Offsets.ClassDataSize);
                                                    }
                                                    if (Actor.ClassId == 0x42 || Actor.ClassId == 0x43)
                                                        KyaReadMiniGamesSupportActor(Actor, MakeArrayView(Data, int32(Entry.Size)),
                                                            Offsets.ClassData, Offsets.ClassDataSize);
                                                    if (Actor.ClassId == 0x56)
                                                        Actor.bHasCreditsData = KyaReadCreditsActor(Actor, MakeArrayView(Data, int32(Entry.Size)),
                                                            Offsets.ClassData, Offsets.ClassDataSize);
                                                    if (Actor.ClassId == 0x54)
                                                        Actor.bHasWantedZooData = KyaReadWantedZooActor(Actor, MakeArrayView(Data, int32(Entry.Size)),
                                                            Offsets.ClassData, Offsets.ClassDataSize);
                                                    if (Actor.ClassId == 0x33)
                                                        Actor.bHasJamGutData = KyaReadJamGutActor(Actor, MakeArrayView(Data, int32(Entry.Size)),
                                                            Offsets.ClassData, Offsets.ClassDataSize);
                                                    if (Actor.ClassId == 0x4c)
                                                    {
                                                        Actor.bHasStillerData = KyaReadStillerActor(Actor,
                                                            MakeArrayView(Data, int32(Entry.Size)), Offsets.ClassData, Offsets.ClassDataSize);
                                                    }
                                                    if (Actor.ClassId == 0x51)
                                                    {
                                                        Actor.bHasRuneData = KyaReadRuneActor(Actor,
                                                            MakeArrayView(Data, int32(Entry.Size)), Offsets.ClassData, Offsets.ClassDataSize);
                                                    }
                                                    if (Actor.ClassId == 0x52)
                                                    {
                                                        Actor.bHasBonusFountainData = KyaReadFountainActor(Actor,
                                                            MakeArrayView(Data, int32(Entry.Size)), Offsets.ClassData, Offsets.ClassDataSize);
                                                    }
                                                    if (Actor.ClassId == 0x0f)
                                                    {
                                                        Actor.bHasRopeData = KyaReadRopeActor(Actor,
                                                            MakeArrayView(Data, int32(Entry.Size)), Offsets.ClassData, Offsets.ClassDataSize);
                                                    }
                                                    if (Actor.ClassId == 0x10)
                                                    {
                                                        Actor.bHasGravityAwareData = KyaReadGravityActor(Actor,
                                                            MakeArrayView(Data, int32(Entry.Size)), Offsets.ClassData, Offsets.ClassDataSize);
                                                    }
                                                    if (Actor.ClassId == 0x3c)
                                                    {
                                                        Actor.bHasNativCmdData = KyaReadNativCmdActor(Actor,
                                                            MakeArrayView(Data, int32(Entry.Size)), Offsets.ClassData, Offsets.ClassDataSize);
                                                    }
                                                    if (Actor.ClassId == 0x19)
                                                    {
                                                        Actor.bHasCompanionData = KyaReadCompanionActor(Actor,
                                                            MakeArrayView(Data, int32(Entry.Size)), Offsets.ClassData, Offsets.ClassDataSize);
                                                    }
                                                    if (Actor.ClassId == 0x17)
                                                    {
                                                        Actor.bHasBridgeData = KyaReadBridgeActor(Actor,
                                                            MakeArrayView(Data, int32(Entry.Size)), Offsets.ClassData, Offsets.ClassDataSize);
                                                    }
                                                    if (Actor.ClassId == 0x38)
                                                    {
                                                        Actor.bHasAtonData = KyaReadAtonActor(Actor,
                                                            MakeArrayView(Data, int32(Entry.Size)), Offsets.ClassData, Offsets.ClassDataSize);
                                                    }
                                                    if (Actor.ClassId == 0x06)
                                                    {
                                                        FKyaKimOffsets Kim;
                                                        if (KyaReadKimOffsets(
                                                            MakeArrayView(Data, int32(Entry.Size)), Offsets.ClassData, Offsets.ClassDataSize, Kim))
                                                        {
                                                            if (!KyaPopulateFighterPrefix(Actor, Data, int32(Entry.Size), Kim.Fighter)) return nullptr;
                                                            if (!KyaReadKimActor(Actor,
                                                                MakeArrayView(Data, int32(Entry.Size)), Offsets.ClassData, Offsets.ClassDataSize)) return nullptr;
                                                        }
                                                    }
                                                    if (Actor.ClassId == 0x05)
                                                    {
                                                        Actor.bHasBoomyData = KyaReadBoomyActor(Actor,
                                                            MakeArrayView(Data, int32(Entry.Size)), Offsets.ClassData, Offsets.ClassDataSize);
                                                    }
                                                    if (Actor.ClassId == 0x1e)
                                                    {
                                                        Actor.bHasTypedCheckpoints = KyaReadCheckpointsActor(Actor,
                                                            MakeArrayView(Data, int32(Entry.Size)), Offsets.ClassData, Offsets.ClassDataSize);
                                                    }
                                                    for (const FKyaBehaviourOffsets& BehaviourOffset : Offsets.Behaviours)
                                                    {
                                                        FKyaActorBehaviour& Behaviour = Actor.Behaviours.AddDefaulted_GetRef();
                                                        Behaviour.Id = int32(ReadU32(Data + BehaviourOffset.Id));
                                                        Behaviour.Data.Append(Data + BehaviourOffset.Payload, BehaviourOffset.Size);
                                                        if (Actor.ClassId == 0x2e)
                                                        {
                                                            KyaReadFxBehaviourValue(Behaviour, Behaviour.Id,
                                                                MakeArrayView(Data, int32(Entry.Size)),
                                                                BehaviourOffset.Payload, BehaviourOffset.Size);
                                                        }
                                                        if (Actor.ClassId == 0x2c && (Behaviour.Id == 2 || Behaviour.Id == 3))
                                                        {
                                                            KyaReadTrapBehaviour(Behaviour, Behaviour.Id,
                                                                MakeArrayView(Data, int32(Entry.Size)),
                                                                BehaviourOffset.Payload, BehaviourOffset.Size);
                                                        }
                                                        if (Actor.ClassId == 0x22)
                                                        {
                                                            FKyaHedgehogBehaviourValues HedgehogValues;
                                                            Behaviour.bHasTypedHedgehogBehaviour = KyaReadHedgehogBehaviour(Behaviour.Id,
                                                                MakeArrayView(Data, int32(Entry.Size)), BehaviourOffset.Payload, BehaviourOffset.Size, HedgehogValues);
                                                            if (Behaviour.bHasTypedHedgehogBehaviour)
                                                                Behaviour.HedgehogPathFollowIndex = HedgehogValues.PathFollowIndex;
                                                        }
                                                        if (Actor.ClassId == 0x20)
                                                        {
                                                            KyaReadWeaponBehaviourRecord(Behaviour, Behaviour.Id,
                                                                MakeArrayView(Data, int32(Entry.Size)),
                                                                BehaviourOffset.Payload, BehaviourOffset.Size);
                                                        }
                                                        if (Actor.ClassId == 0x0e)
                                                        {
                                                            KyaReadBonusBehaviourRecord(Behaviour, Behaviour.Id,
                                                                MakeArrayView(Data, int32(Entry.Size)),
                                                                BehaviourOffset.Payload, BehaviourOffset.Size);
                                                        }
                                                        if (Actor.ClassId == 0x3b && Behaviour.Id == 2)
                                                        {
                                                            KyaReadEventGeneratorBehaviour(Behaviour, Behaviour.Id,
                                                                MakeArrayView(Data, int32(Entry.Size)),
                                                                BehaviourOffset.Payload, BehaviourOffset.Size);
                                                        }
                                                        if (Actor.ClassId == 0x4b && Behaviour.Id == 3)
                                                        {
                                                            KyaReadShockerBehaviour(Behaviour, Behaviour.Id,
                                                                MakeArrayView(Data, int32(Entry.Size)),
                                                                BehaviourOffset.Payload, BehaviourOffset.Size);
                                                        }
                                                        if (Actor.ClassId == 0x34 && Behaviour.Id == 2)
                                                        {
                                                            KyaReadNativShopBehaviour(Behaviour, Behaviour.Id,
                                                                MakeArrayView(Data, int32(Entry.Size)),
                                                                BehaviourOffset.Payload, BehaviourOffset.Size);
                                                        }
                                                        if (Actor.ClassId == 0x4e && (Behaviour.Id == 26 || Behaviour.Id == 27))
                                                        {
                                                            KyaReadBunchBehaviour(Behaviour, Behaviour.Id,
                                                                MakeArrayView(Data, int32(Entry.Size)),
                                                                BehaviourOffset.Payload, BehaviourOffset.Size);
                                                        }
                                                        if (Actor.ClassId == 0x4f && (Behaviour.Id == 26 || Behaviour.Id == 27))
                                                        {
                                                            KyaReadBrazulBehaviour(Behaviour, Behaviour.Id,
                                                                MakeArrayView(Data, int32(Entry.Size)),
                                                                BehaviourOffset.Payload, BehaviourOffset.Size);
                                                        }
                                                        if ((Actor.ClassId == 0x44 || Actor.ClassId == 0x45 ||
                                                            Actor.ClassId == 0x46 || Actor.ClassId == 0x47 ||
                                                            Actor.ClassId == 0x55) && Behaviour.Id == 3)
                                                        {
                                                            KyaReadMiniGameBehaviour(Behaviour, Behaviour.Id,
                                                                MakeArrayView(Data, int32(Entry.Size)),
                                                                BehaviourOffset.Payload, BehaviourOffset.Size);
                                                        }
                                                        if (Actor.ClassId == 0x3f && Behaviour.Id == 3)
                                                        {
                                                            KyaReadPatternBehaviour(Behaviour, Behaviour.Id,
                                                                MakeArrayView(Data, int32(Entry.Size)),
                                                                BehaviourOffset.Payload, BehaviourOffset.Size);
                                                        }
                                                        if (Actor.ClassId == 0x07 && Behaviour.Id == 4)
                                                        {
                                                            KyaReadMickenBehaviourValue(Behaviour, Behaviour.Id,
                                                                MakeArrayView(Data, int32(Entry.Size)),
                                                                BehaviourOffset.Payload, BehaviourOffset.Size);
                                                        }
                                                        if (Actor.ClassId == 0x32 && (Behaviour.Id == 2 || Behaviour.Id == 3))
                                                        {
                                                            KyaReadMoneyBehaviour(Behaviour, Behaviour.Id,
                                                                MakeArrayView(Data, int32(Entry.Size)),
                                                                BehaviourOffset.Payload, BehaviourOffset.Size);
                                                        }
                                                        if (Actor.ClassId == 0x16 && (Behaviour.Id == 4 || Behaviour.Id == 5 || Behaviour.Id == 6))
                                                        {
                                                            KyaReadFruitBehaviour(Behaviour, Behaviour.Id,
                                                                MakeArrayView(Data, int32(Entry.Size)),
                                                                BehaviourOffset.Payload, BehaviourOffset.Size);
                                                        }
                                                        if (Actor.ClassId == 0x13 && Behaviour.Id == 2)
                                                        {
                                                            KyaReadCommanderBehaviour(Behaviour, Behaviour.Id,
                                                                MakeArrayView(Data, int32(Entry.Size)),
                                                                BehaviourOffset.Payload, BehaviourOffset.Size);
                                                        }
                                                        if (Actor.ClassId == 0x0a && (Behaviour.Id == 3 || Behaviour.Id == 5 || Behaviour.Id == 6 || Behaviour.Id == 7 || Behaviour.Id == 8))
                                                        {
                                                            KyaReadNativBehaviour(Behaviour, Behaviour.Id,
                                                                MakeArrayView(Data, int32(Entry.Size)),
                                                                BehaviourOffset.Payload, BehaviourOffset.Size);
                                                        }
                                                        if (Behaviour.Id == 1 && BehaviourOffset.Size == 4)
                                                        {
                                                            Behaviour.bHasTypedCinematicBehaviour = true;
                                                            Behaviour.CinematicLeaveBehaviourId = int32(ReadU32(Data + BehaviourOffset.Payload));
                                                        }
                                                        if (Actor.ClassId == 0x1b && (Behaviour.Id == 3 || Behaviour.Id == 5))
                                                        {
                                                            KyaReadDcaBehaviour(Behaviour, Behaviour.Id,
                                                                MakeArrayView(Data, int32(Entry.Size)),
                                                                BehaviourOffset.Payload, BehaviourOffset.Size);
                                                        }
                                                        if (Actor.ClassId == 0x3d && Behaviour.Id == 2)
                                                        {
                                                            KyaReadFogBehaviour(Behaviour, Behaviour.Id,
                                                                MakeArrayView(Data, int32(Entry.Size)),
                                                                BehaviourOffset.Payload, BehaviourOffset.Size);
                                                        }
                                                        if (Actor.ClassId == 0x3e && (Behaviour.Id == 3 || Behaviour.Id == 4))
                                                        {
                                                            KyaReadShootBehaviour(Behaviour, Behaviour.Id,
                                                                MakeArrayView(Data, int32(Entry.Size)),
                                                                BehaviourOffset.Payload, BehaviourOffset.Size);
                                                        }
                                                        if (Actor.ClassId == 0x10 && (Behaviour.Id == 3 || Behaviour.Id == 4))
                                                        {
                                                            KyaReadGravityBehaviour(Behaviour, Behaviour.Id,
                                                                MakeArrayView(Data, int32(Entry.Size)),
                                                                BehaviourOffset.Payload, BehaviourOffset.Size);
                                                        }
                                                        if (Actor.ClassId == 0x17 && (Behaviour.Id == 2 || Behaviour.Id == 3))
                                                        {
                                                            KyaReadBridgeBehaviour(Behaviour, Behaviour.Id,
                                                                MakeArrayView(Data, int32(Entry.Size)),
                                                                BehaviourOffset.Payload, BehaviourOffset.Size);
                                                        }
                                                        if ((Actor.ClassId == 0x09 || Actor.ClassId == 0x4e || Actor.ClassId == 0x4f) && (Behaviour.Id == 8 || Behaviour.Id == 10 ||
                                                            Behaviour.Id == 13 || Behaviour.Id == 14 || Behaviour.Id == 15 ||
                                                            Behaviour.Id == 17 || Behaviour.Id == 18 || Behaviour.Id == 19 ||
                                                            Behaviour.Id == 21 || Behaviour.Id == 22 || Behaviour.Id == 23 ||
                                                            Behaviour.Id == 24))
                                                        {
                                                            KyaReadWolfenBehaviour(Behaviour, Behaviour.Id,
                                                                MakeArrayView(Data, int32(Entry.Size)),
                                                                BehaviourOffset.Payload, BehaviourOffset.Size);
                                                        }
                                                        if (Actor.ClassId == 0x1c && (Behaviour.Id == 3 || Behaviour.Id == 4 ||
                                                            Behaviour.Id == 5 || Behaviour.Id == 10 || Behaviour.Id == 11 ||
                                                            Behaviour.Id == 12 || Behaviour.Id == 13))
                                                        {
                                                            KyaReadProjectileBehaviour(Behaviour, Behaviour.Id,
                                                                MakeArrayView(Data, int32(Entry.Size)),
                                                                BehaviourOffset.Payload, BehaviourOffset.Size);
                                                        }
                                                        if (Actor.ClassId == 0x2b)
                                                            KyaReadHelperSignBehaviour(Behaviour, MakeArrayView(Data, int32(Entry.Size)),
                                                                BehaviourOffset.Payload, BehaviourOffset.Size);
                                                        if (Actor.ClassId == 0x27)
                                                            KyaReadWoodMonsterBehaviour(Behaviour, MakeArrayView(Data, int32(Entry.Size)),
                                                                BehaviourOffset.Payload, BehaviourOffset.Size);
                                                        if (KyaActorFamilies::HasEmptyPayload(Actor.ClassId, Behaviour.Id) && BehaviourOffset.Size == 0)
                                                            Behaviour.bHasTypedEmptyBehaviour = true;
                                                        if (Actor.ClassId == 0x33 && Behaviour.Id == 4)
                                                            KyaReadJamGutBehaviour(Behaviour, MakeArrayView(Data, int32(Entry.Size)),
                                                                BehaviourOffset.Payload, BehaviourOffset.Size);
                                                        if (KyaActorFamilies::UsesCatchByTrap(Actor.ClassId, Behaviour.Id))
                                                        {
                                                            FKyaCatchByTrapOffsets Off;
                                                            Behaviour.bHasTypedCatchByTrapBehaviour = KyaReadCatchByTrapOffsets(
                                                                MakeArrayView(Data, int32(Entry.Size)), BehaviourOffset.Payload, BehaviourOffset.Size, Off);
                                                            if (Behaviour.bHasTypedCatchByTrapBehaviour)
                                                                for (const FKyaConditionedOffsets& BlockOffsets : Off.Blocks)
                                                                {
                                                                    FKyaConditionedOperation& Result = Behaviour.CatchOperations.AddDefaulted_GetRef();
                                                                    for (int64 Offset : BlockOffsets.Operations)
                                                                    {
                                                                        FKyaScenarioOperation& Op = Result.Operations.AddDefaulted_GetRef();
                                                                        Op.Variable = int32(ReadU32(Data + Offset));
                                                                        Op.Type = int32(ReadU32(Data + Offset + 4));
                                                                        Op.Value = int32(ReadU32(Data + Offset + 8));
                                                                    }
                                                                    for (int64 Offset : BlockOffsets.Conditions)
                                                                    {
                                                                        FKyaScenarioCondition& Condition = Result.Conditions.AddDefaulted_GetRef();
                                                                        Condition.Combiner = int32(ReadU32(Data + Offset));
                                                                        Condition.Variable = int32(ReadU32(Data + Offset + 4));
                                                                        Condition.Comparison = int32(ReadU32(Data + Offset + 8));
                                                                        Condition.Value = int32(ReadU32(Data + Offset + 12));
                                                                    }
                                                                }
                                                        }
                                                        if (Actor.ClassId == 0x06 && (Behaviour.Id == 9 || Behaviour.Id == 10 || Behaviour.Id == 11))
                                                        {
                                                            FKyaHeroUnknownOffsets Off;
                                                            Behaviour.bHasTypedHeroUnknownBehaviour = KyaReadHeroUnknownOffsets(
                                                                MakeArrayView(Data, int32(Entry.Size)), BehaviourOffset.Payload, BehaviourOffset.Size, Off);
                                                            if (Behaviour.bHasTypedHeroUnknownBehaviour)
                                                            {
                                                                FKyaHeroUnknownBehaviour& HeroValues = Behaviour.HeroUnknownBehaviour;
                                                                HeroValues.FxTypeA = int32(ReadU32(Data + Off.Values));
                                                                HeroValues.TextureA = int32(ReadU32(Data + Off.Values + 4));
                                                                HeroValues.MeshA = int32(ReadU32(Data + Off.Values + 8));
                                                                HeroValues.FxTypeB = int32(ReadU32(Data + Off.Values + 12));
                                                                HeroValues.FxTypeC = int32(ReadU32(Data + Off.Values + 16));
                                                                HeroValues.TextureB = int32(ReadU32(Data + Off.Values + 20));
                                                                HeroValues.MeshB = int32(ReadU32(Data + Off.Values + 24));
                                                            }
                                                        }
                                                        if (Actor.ClassId == 0x1c && Behaviour.Id == 9)
                                                        {
                                                            Behaviour.bHasTypedInventoryBehaviour = KyaReadProjectileInventory(
                                                                Behaviour.InventoryInfo, MakeArrayView(Data, int32(Entry.Size)),
                                                                BehaviourOffset.Payload, BehaviourOffset.Size);
                                                        }
                                                        if (Actor.ClassId == 0x11 && Behaviour.Id >= 2 && Behaviour.Id <= 8)
                                                        {
                                                            KyaReadSwitchBehaviour(Behaviour, Behaviour.Id,
                                                                MakeArrayView(Data, int32(Entry.Size)),
                                                                BehaviourOffset.Payload, BehaviourOffset.Size);
                                                        }
                                                        if (Actor.ClassId == 0x0d && Behaviour.Id >= 2 && Behaviour.Id <= 10)
                                                        {
                                                            KyaReadMovingPlatformBehaviour(Behaviour, Behaviour.Id,
                                                                MakeArrayView(Data, int32(Entry.Size)),
                                                                BehaviourOffset.Payload, BehaviourOffset.Size);
                                                        }
                                                        if (Actor.ClassId == 0x19 && Behaviour.Id == 3)
                                                        {
                                                            KyaReadCompanionBehaviour(Behaviour, Behaviour.Id,
                                                                MakeArrayView(Data, int32(Entry.Size)),
                                                                BehaviourOffset.Payload, BehaviourOffset.Size);
                                                        }
                                                        if (Actor.ClassId == 0x18 && Behaviour.Id == 2)
                                                        {
                                                            KyaReadClusteriserBehaviour(Behaviour, Behaviour.Id,
                                                                MakeArrayView(Data, int32(Entry.Size)),
                                                                BehaviourOffset.Payload, BehaviourOffset.Size);
                                                        }
                                                    }
                                                    for (int32 Model = 0; Model < Offsets.ModelStrings.Num(); Model += 2)
                                                    {
                                                        FKyaActorModelReference& Reference = Actor.Models.AddDefaulted_GetRef();
                                                        Reference.MeshName = ANSI_TO_TCHAR(reinterpret_cast<const char*>(Data + Offsets.ModelStrings[Model].Offset));
                                                        Reference.TextureName = ANSI_TO_TCHAR(reinterpret_cast<const char*>(Data + Offsets.ModelStrings[Model + 1].Offset));
                                                    }
                                                    for (int32 Index = 0; Index < Offsets.MacroCount; ++Index)
                                                    {
                                                        FKyaActorMacroAnimation& Animation = Actor.MacroAnimations.AddDefaulted_GetRef();
                                                        Animation.AnimationType = int32(ReadU32(Data + Offsets.MacroAnimations + Index * 8));
                                                        Animation.MacroAnimationId = int32(ReadU32(Data + Offsets.MacroAnimations + Index * 8 + 4));
                                                    }
                                                }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        break;
    }
    for (const FKyaBankEntry& Entry : Bank->Entries)
    {
        if (Entry.Type == 6 && Entry.Subtype == 2 && Entry.Size >= 36)
        {
            const uint8* Scene = Buffer + Entry.Offset + 16;
            uint32 Bits = ReadU32(Scene);
            FMemory::Memcpy(&Bank->SceneConfiguration.Field1C, &Bits, sizeof(Bits));
            Bank->SceneConfiguration.Field20 = ReadU32(Scene + 4);
            Bits = ReadU32(Scene + 8);
            FMemory::Memcpy(&Bank->SceneConfiguration.Field24, &Bits, sizeof(Bits));
            Bank->SceneConfiguration.HeaderWords.Add(ReadU32(Scene + 12));
            Bank->SceneConfiguration.HeaderWords.Add(ReadU32(Scene + 16));
            int64 Pos = 36;
            const uint8* Data = Buffer + Entry.Offset;
            if (Pos + 4 > Entry.Size) break;
            const uint32 MeshCount = ReadU32(Data + Pos);
            Pos += 4;
            if (MeshCount > uint64(Entry.Size - Pos) / 4) break;
            for (uint32 Index = 0; Index < MeshCount; ++Index)
                Bank->SceneConfiguration.CommonMeshValues.Add(int32(ReadU32(Data + Pos + Index * 4)));
            Pos += int64(MeshCount) * 4;
            if (Pos + 12 > Entry.Size) break;
            Bank->SceneConfiguration.CommonTextureCount = int32(ReadU32(Data + Pos));
            Bank->SceneConfiguration.TextureWord = int32(ReadU32(Data + Pos + 4));
            Pos += 8;
            const uint32 CollisionCount = ReadU32(Data + Pos);
            Pos += 4;
            if (CollisionCount > uint64(Entry.Size - Pos) / 4) break;
            for (uint32 Index = 0; Index < CollisionCount; ++Index)
                Bank->SceneConfiguration.CollisionBankValues.Add(int32(ReadU32(Data + Pos + Index * 4)));
            Pos += int64(CollisionCount) * 4;
            if (Pos + 4 <= Entry.Size)
                Bank->SceneConfiguration.LevelAnimationCount = int32(ReadU32(Data + Pos));
            if (Pos + 20 > Entry.Size) break;
            for (int32 Index = 0; Index < 4; ++Index)
                Bank->SceneConfiguration.PostAnimationWords.Add(ReadU32(Data + Pos + 4 + Index * 4));
            Pos += 20;
            if (Pos + 4 > Entry.Size) break;
            const uint32 ClassCount = ReadU32(Data + Pos);
            Pos += 4;
            if (ClassCount > uint64(Entry.Size - Pos) / 8) break;
            for (uint32 Index = 0; Index < ClassCount; ++Index)
            {
                FKyaActorClassAllocation Allocation;
                Allocation.ClassId = int32(ReadU32(Data + Pos + Index * 8));
                Allocation.Count = int32(ReadU32(Data + Pos + Index * 8 + 4));
                Bank->SceneConfiguration.ActorClassAllocations.Add(Allocation);
            }
            Pos += int64(ClassCount) * 8;
            if (Pos + 4 <= Entry.Size) Bank->SceneConfiguration.ParticleWord = ReadU32(Data + Pos);
            break;
        }
    }
    Bank->BankBytes.Append(Buffer, int32(Size));
    if (ImportCancelled()) return nullptr;
    Buffer = BufferEnd;
    const uint8* LightBuffer = Bank->BankBytes.GetData();
    // Light archive: CLightManager::Level_AddAll reads a count, a table of offsets,
    // then one record per light (CLightManager.cpp:518, LightBase.cpp:5/131/171/194).
    for (const FKyaBankEntry& Candidate : Bank->Entries)
    {
        if (Candidate.Type != 16 || Candidate.Subtype != 1 || Candidate.Offset < 0 || Candidate.Size < 32 ||
            Candidate.Offset > Size || Candidate.Size > Size - Candidate.Offset) continue;
        // The entry is an Eden chunk: 16-byte header, then the THGL record stream.
        if (ReadU32(LightBuffer + Candidate.Offset) != 0xDEADBEEF ||
            FMemory::Memcmp(LightBuffer + Candidate.Offset + 8, "THGL", 4) != 0) continue;
        const int64 LightStart = Candidate.Offset + 16;
        const int64 LightSize = Candidate.Size - 16;
        TArray<FKyaLightOffsets> LightOffsets;
        if (!KyaReadLightOffsets(MakeArrayView(LightBuffer, int32(Size)), LightStart, LightSize, LightOffsets)) continue;
        auto LightReadFloat = [LightBuffer](int64 Offset) {
            uint32 Bits = uint32(LightBuffer[Offset]) | (uint32(LightBuffer[Offset + 1]) << 8) |
                (uint32(LightBuffer[Offset + 2]) << 16) | (uint32(LightBuffer[Offset + 3]) << 24);
            float Value;
            FMemory::Memcpy(&Value, &Bits, sizeof(Value));
            return Value;
        };
        auto Channel = [LightBuffer](int64 Offset, float Multiplier) {
            const uint8 Raw = LightBuffer[Offset];
            const float Value = FMath::Min(Raw, uint8(127)) * 2.0f * Multiplier / 255.0f;
            return FMath::Clamp(Value, 0.0f, 1.0f);
        };
        for (const FKyaLightOffsets& Light : LightOffsets)
        {
            FKyaLight& Result = Bank->Lights.AddDefaulted_GetRef();
            Result.LightType = Light.LightType;
            Result.ShapeType = Light.ShapeType;
            float Multiplier = 0.0f;
            FMemory::Memcpy(&Multiplier, LightBuffer + Light.Multiplier, sizeof(Multiplier));
            Result.Multiplier = Multiplier;
            Result.AmbientColor = FLinearColor(Channel(Light.Ambient, Multiplier), Channel(Light.Ambient + 1, Multiplier),
                Channel(Light.Ambient + 2, Multiplier), Channel(Light.Ambient + 3, Multiplier));
            Result.Color = FLinearColor(Channel(Light.Color, Multiplier), Channel(Light.Color + 1, Multiplier),
                Channel(Light.Color + 2, Multiplier), Channel(Light.Color + 3, Multiplier));
            Result.Field20Color = FLinearColor(Channel(Light.Field20, Multiplier), Channel(Light.Field20 + 1, Multiplier),
                Channel(Light.Field20 + 2, Multiplier), Channel(Light.Field20 + 3, Multiplier));
            if (Light.Position != 0)
            {
                Result.Position = FVector(LightReadFloat(Light.Position), LightReadFloat(Light.Position + 4), LightReadFloat(Light.Position + 8));
                Result.Falloff0 = LightReadFloat(Light.Falloff);
                Result.Falloff1 = LightReadFloat(Light.Falloff + 4);
            }
            if (Light.Direction != 0)
                Result.Direction = FVector(LightReadFloat(Light.Direction), LightReadFloat(Light.Direction + 4), LightReadFloat(Light.Direction + 8));
            if (Light.Fov != 0)
            {
                Result.Fov0 = LightReadFloat(Light.Fov);
                Result.Fov1 = LightReadFloat(Light.Fov + 4);
            }
            for (int32 Index = 0; Index < Light.TrailingCount; ++Index)
                Result.TrailingValues.Add(int32(uint32(LightBuffer[Light.Trailing + Index * 4]) | (uint32(LightBuffer[Light.Trailing + Index * 4 + 1]) << 8) | (uint32(LightBuffer[Light.Trailing + Index * 4 + 2]) << 16) | (uint32(LightBuffer[Light.Trailing + Index * 4 + 3]) << 24)));
        }
        Bank->bHasLights = !Bank->Lights.IsEmpty();
        break;
    }
    // Typed baseline for reset-to-imported and change detection; editor metadata
    // only, never written back to game banks.
    if (ImportCancelled()) return nullptr;
    CaptureKyaImportedBaseline(*Bank);
    return Bank;
}
