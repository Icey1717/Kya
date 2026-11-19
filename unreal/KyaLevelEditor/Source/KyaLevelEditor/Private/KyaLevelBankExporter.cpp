#include "KyaLevelBankExporter.h"
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
#include "KyaLevelBank.h"
#include "KyaActorCatalog.h"
#include "KyaBankExport.h"
#include "KyaSectorBankExport.h"
#include "KyaGeometryEncoding.h"
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
#include "KyaBankStructuralWriter.h"
#include "UObject/StrongObjectPtr.h"
#include "Misc/FeedbackContext.h"

UKyaLevelBankExporter::UKyaLevelBankExporter()
{
    SupportedClass = UKyaLevelBank::StaticClass();
    FormatExtension.Add(TEXT("bnk"));
    FormatDescription.Add(TEXT("Kya level bank"));
    bText = false;
}

bool KyaBuildBankExportBytes(const UKyaLevelBank* Bank, TArray<uint8>& OutBytes,
    FKyaBankExportDiagnostics& Diagnostics)
{
    OutBytes.Reset();
    if (!Bank)
    {
        Diagnostics.Errors.Add(TEXT("No bank was supplied."));
        return false;
    }
    if (!Bank->HasCurrentImportSchema())
    {
        Diagnostics.Errors.Add(TEXT("This Kya bank asset uses an older import schema. Reimport the source bank before exporting."));
        return false;
    }
    const FKyaBankEntry* Scene = nullptr;
    const FKyaBankEntry* SceneData = nullptr;
    for (const FKyaBankEntry& Entry : Bank->Entries)
    {
        if (Entry.Type == 6 && Entry.Subtype == 1)
        {
            if (SceneData) return false;
            SceneData = &Entry;
        }
        if (Entry.Type == 6 && Entry.Subtype == 2)
        {
            if (Scene)
            {
                Diagnostics.Errors.Add(TEXT("Multiple scene configuration entries cannot be exported."));
                return false;
            }
            Scene = &Entry;
        }
    }
    if (!Scene && !SceneData) return KyaBuildSectorBankExportBytes(*Bank, OutBytes, Diagnostics);
    if (!Scene || Scene->Size < 28 || Scene->Offset < 0 ||
        Scene->Offset > Bank->BankBytes.Num() || Scene->Size > Bank->BankBytes.Num() - Scene->Offset ||
        Bank->SceneConfiguration.Field20 < 0 || Bank->SceneConfiguration.Field20 > MAX_uint32 ||
        Bank->BankBytes.Num() < 16 || (Bank->BankBytes[12] & 1))
    {
        Diagnostics.Errors.Add(TEXT("Cannot export this bank's scene configuration."));
        return false;
    }
    TArray<uint8> Bytes = Bank->BankBytes;
    auto ReadU32 = [&Bytes](int64 Offset) -> uint32 {
        const uint8* P = Bytes.GetData() + Offset;
        return uint32(P[0]) | (uint32(P[1]) << 8) | (uint32(P[2]) << 16) | (uint32(P[3]) << 24);
    };
    if (!SceneData || SceneData->Offset < 0 || SceneData->Size < 24 ||
        SceneData->Offset > Bytes.Num() || SceneData->Size > Bytes.Num() - SceneData->Offset ||
        Bank->SceneEnvironment.BackgroundFlags < 0 || Bank->SceneEnvironment.BackgroundFlags > MAX_uint32 ||
        Bank->SceneEnvironment.FogFlags < 0 || Bank->SceneEnvironment.FogFlags > MAX_uint32 ||
        Bank->SceneEnvironment.CameraFlags < 0 || Bank->SceneEnvironment.CameraFlags > MAX_uint32 ||
        Bank->SceneEnvironment.CameraConfigFlags < 0 || Bank->SceneEnvironment.CameraConfigFlags > MAX_uint32 ||
        Bank->SceneEnvironment.Field11C < 0 || Bank->SceneEnvironment.Field11C > MAX_uint32 ||
        Bank->SceneEnvironment.Count120 < 0 || Bank->SceneEnvironment.Count120 > MAX_uint32 ||
        Bank->SceneEnvironment.MapField08 < 0 || Bank->SceneEnvironment.MapField08 > MAX_uint32) return false;
    int64 Setup = SceneData->Offset + 16;
    const int64 SceneEnd = SceneData->Offset + SceneData->Size;
    const uint32 HasCluster = ReadU32(Setup);
    Setup += 4;
    if (HasCluster) Setup += 12;
    if (Setup + 4 > SceneEnd) return false;
    const int32 Background = int32(ReadU32(Setup));
    Setup += 4;
    if (Background != -1) Setup += 4;
    if (Setup + 112 > SceneEnd) return false;
    // Flag bit 1 controls the presence of additional camera records. Changing
    // it without rebuilding the variable-length stream would corrupt the bank.
    if (((ReadU32(Setup + 96) ^ uint32(Bank->SceneEnvironment.CameraFlags)) & 2) != 0)
    {
        Diagnostics.Errors.Add(TEXT("Changing camera flag 0x2 requires rebuilding its optional records."));
        return false;
    }
    int64 Config = 0;
    if (!KyaFindCameraConfig(MakeArrayView(Bytes.GetData(), Bytes.Num()), Setup, Config)) return false;
    int64 CameraConditions = 0;
    if (!KyaFindCameraConditions(MakeArrayView(Bytes.GetData(), int32(SceneEnd)), Setup, CameraConditions) ||
        ReadU32(CameraConditions) != uint32(Bank->SceneEnvironment.CameraConditions.Num())) return false;
    int64 CameraSwitch = Setup + 112;
    if (ReadU32(Setup + 96) & 2)
    {
        CameraSwitch += 4;
        const TArray<int32>* Lists[] = { &Bank->SceneEnvironment.CameraSwitchActorsA, &Bank->SceneEnvironment.CameraSwitchActorsB };
        for (const TArray<int32>* List : Lists)
        {
            const uint32 Count = ReadU32(CameraSwitch); CameraSwitch += 4;
            if (Count != uint32(List->Num())) return false;
            CameraSwitch += int64(Count) * 4;
        }
    }
    const int64 ConfigValues = Config + 20 + (ReadU32(Config + 16) == 1 ? 20 : 0);
    int64 SceneValues = 0;
    if (!KyaFindSceneAfterCamera(MakeArrayView(Bytes.GetData(), Bytes.Num()), Config, SceneValues)) return false;
    const int32 OptionalCount = ReadU32(Config + 16) == 1 ? 5 : 0;
    const int32 ExtraCount = ReadU32(ConfigValues + 36) == 1 ? 12 : 0;
    if (Bank->SceneEnvironment.CameraOptionalConfigValues.Num() != OptionalCount ||
        Bank->SceneEnvironment.CameraExtraValues.Num() != ExtraCount) return false;
    const int64 Map = SceneValues + 12;
    int64 MapEnd = 0;
    if (!KyaFindMapEnd(MakeArrayView(Bytes.GetData(), Bytes.Num()), Map, MapEnd) ||
        MapEnd + 48 > SceneEnd || Bank->SceneEnvironment.EffectPoolCounts.Num() != 7 ||
        ReadU32(Map + 12) != uint32(Bank->SceneEnvironment.MapEntries.Num())) return false;
    for (int64 PoolCount : Bank->SceneEnvironment.EffectPoolCounts)
        if (PoolCount < 0 || PoolCount > MAX_uint32) return false;
    TArray<FKyaConditionedOffsets> ScenarioBlocks;
    int64 ScenarioEnd = 0;
    if (!KyaReadConditionedOffsets(MakeArrayView(Bytes.GetData(), int32(SceneEnd)), MapEnd + 48, ScenarioBlocks, ScenarioEnd) ||
        ScenarioBlocks.Num() != Bank->SceneEnvironment.InitialScenarioOperations.Num()) return false;
    for (int32 Block = 0; Block < ScenarioBlocks.Num(); ++Block)
        if (ScenarioBlocks[Block].Operations.Num() != Bank->SceneEnvironment.InitialScenarioOperations[Block].Operations.Num() ||
            ScenarioBlocks[Block].Conditions.Num() != Bank->SceneEnvironment.InitialScenarioOperations[Block].Conditions.Num()) return false;
    TArray<FKyaObjectiveOffsets> Objectives;
    int64 ObjectiveEnd = 0, ObjectiveTrailing = 0;
    if (!KyaReadObjectiveOffsets(MakeArrayView(Bytes.GetData(), int32(SceneEnd)), ScenarioEnd, Objectives, ObjectiveEnd, &ObjectiveTrailing) ||
        Objectives.Num() != Bank->SceneEnvironment.Objectives.Num()) return false;
    if (ReadU32(ObjectiveTrailing) != uint32(Bank->SceneEnvironment.ObjectiveZones.Num()) || ObjectiveEnd + 12 > SceneEnd ||
        ReadU32(ObjectiveEnd + 8) != uint32(Bank->SceneEnvironment.ElevatorCutscenes.Num()) ||
        uint64(Bank->SceneEnvironment.ElevatorCutscenes.Num()) * 4 > uint64(SceneEnd - ObjectiveEnd - 12)) return false;
    int64 WaypointRecords = 0;
    int32 WaypointCount = 0;
    if (!KyaFindWaypoints(MakeArrayView(Bytes.GetData(), int32(SceneEnd)), ObjectiveEnd, WaypointRecords, WaypointCount) ||
        WaypointCount != Bank->SceneEnvironment.Waypoints.Num()) return false;
    TArray<FKyaPathOffsets> Paths;
    if (!KyaReadPathOffsets(MakeArrayView(Bytes.GetData(), int32(SceneEnd)), WaypointRecords, WaypointCount, SceneData->Offset, Paths) ||
        Paths.Num() != Bank->SceneEnvironment.Paths.Num()) return false;
    TArray<FKyaActorOffsets> Actors;
    if (!KyaReadActorOffsets(MakeArrayView(Bytes.GetData(), int32(SceneEnd)), WaypointRecords, SceneData->Offset, Actors) ||
        Actors.Num() != Bank->SceneEnvironment.Actors.Num())
    {
        Diagnostics.Errors.Add(TEXT("The serialized actor count differs from the asset. Apply structural actor "
            "commands (create, duplicate, delete) so the bank's actor stream is rebuilt before exporting."));
        return false;
    }
    for (int32 Index = 0; Index < Actors.Num(); ++Index)
    {
        const FKyaActorOffsets& ActorOffsets = Actors[Index];
        if (ActorOffsets.ClassData < 0 || ActorOffsets.ClassDataSize < 0 ||
            ActorOffsets.ClassData + ActorOffsets.ClassDataSize > Bytes.Num())
        {
            Diagnostics.FailedActorIndex = Index;
            Diagnostics.Errors.Add(TEXT("The actor class data lies outside the bank bytes."));
            return false;
        }
        const TConstArrayView<uint8> ClassData(Bytes.GetData() + ActorOffsets.ClassData,
            ActorOffsets.ClassDataSize);
        if (!KyaActorChunkNeedsRebuild(Bank->SceneEnvironment.Actors[Index], ActorOffsets, ClassData))
            continue;
        // Relocate a temporary bank so exporting does not mutate the asset or its undo history.
        TStrongObjectPtr<UKyaLevelBank> Resized(DuplicateObject<UKyaLevelBank>(Bank, GetTransientPackage()));
        bool bChanged = false;
        FString Error;
        if (!KyaRebuildActorChunks(*Resized.Get(), bChanged, Error))
        {
            Diagnostics.FailedActorIndex = Index;
            Diagnostics.Errors.Add(Error);
            return false;
        }
        if (!bChanged)
        {
            Diagnostics.FailedActorIndex = Index;
            Diagnostics.Errors.Add(TEXT("The actor array sizes still differ after rebuilding the actor chunk."));
            return false;
        }
        static int32 RebuildDepth = 0;
        if (RebuildDepth >= 8)
        {
            Diagnostics.FailedActorIndex = Index;
            Diagnostics.Errors.Add(TEXT("Actor chunk rebuilding exceeded its recursion limit."));
            return false;
        }
        TGuardValue<int32> RebuildGuard(RebuildDepth, RebuildDepth + 1);
        return KyaBuildBankExportBytes(Resized.Get(), OutBytes, Diagnostics);
    }
    for (int32 Index = 0; Index < Actors.Num(); ++Index)
    {
        const FKyaActorPlacement& Actor = Bank->SceneEnvironment.Actors[Index];
        const FKyaActorOffsets& Offsets = Actors[Index];
        if (Actor.Index != int32(ReadU32(Offsets.Chunk + 8)) || Actor.ClassId != ReadU32(Offsets.Chunk + 12) ||
            Actor.MacroAnimations.Num() != Offsets.MacroCount || Actor.HashCode < 0 || Actor.HashCode > MAX_uint32 ||
            Actor.LightingFlags < 0 || Actor.LightingFlags > MAX_uint32 || Actor.Behaviours.Num() != Offsets.Behaviours.Num() ||
            Actor.Conditions.Num() != Offsets.ConditionCount)
        {
            Diagnostics.FailedActorIndex = Index;
            Diagnostics.FailedProperty = TEXT("Actor");
            Diagnostics.Errors.Add(FString::Printf(
                TEXT("Actor %d record layout changed (identity, class, lighting, behaviours or conditions); "
                    "structural record changes are not supported."), Index));
            return false;
        }
        FKyaPlatformPrefixOffsets Platform;
        const bool HasPlatform = Actor.ClassId == 0x0d && KyaReadPlatformPrefix(MakeArrayView(Bytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, Platform);
        FKyaPlatformDetailsOffsets Details;
        const bool HasDetails = HasPlatform && KyaReadPlatformDetails(MakeArrayView(Bytes.GetData(), int32(SceneEnd)), Platform.Suffix, Offsets.ClassData + Offsets.ClassDataSize - Platform.Suffix, Details);
        FKyaEventGeneratorOffsets EventGenerator;
        const bool HasEventGenerator = Actor.ClassId == 0x3b && KyaReadEventGeneratorOffsets(
            MakeArrayView(Bytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, EventGenerator);
        FKyaExplosiveDistributorOffsets ExplosiveDistributor;
        const bool HasExplosiveDistributor = Actor.ClassId == 0x1d && KyaReadExplosiveDistributorOffsets(
            MakeArrayView(Bytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, ExplosiveDistributor);
        FKyaShockerOffsets Shocker;
        const bool HasShocker = Actor.ClassId == 0x4b && KyaReadShockerOffsets(
            MakeArrayView(Bytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, Shocker);
        FKyaNativShopOffsets NativShop;
        const bool HasNativShop = Actor.ClassId == 0x34 && KyaReadNativShopOffsets(
            MakeArrayView(Bytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, NativShop);
        FKyaPunchingBallOffsets PunchingBall;
        const bool HasPunchingBall = Actor.ClassId == 0x30 && KyaReadPunchingBallOffsets(
            MakeArrayView(Bytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, PunchingBall);
        FKyaNoseMonsterOffsets NoseMonster;
        const bool HasNoseMonster = Actor.ClassId == 0x26 && KyaReadNoseMonsterOffsets(
            MakeArrayView(Bytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, NoseMonster);
        FKyaPatternOffsets Pattern;
        const bool HasPattern = Actor.ClassId == 0x3f && KyaReadPatternOffsets(
            MakeArrayView(Bytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, Pattern);
        FKyaShipOffsets Ship;
        const bool HasShip = Actor.ClassId == 0x36 && KyaReadShipOffsets(
            MakeArrayView(Bytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, Ship);
        FKyaBunchOffsets Bunch;
        const bool HasBunch = Actor.ClassId == 0x4e && KyaReadBunchOffsets(
            MakeArrayView(Bytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, Bunch);
        FKyaBrazulOffsets Brazul;
        const bool HasBrazul = Actor.ClassId == 0x4f && KyaReadBrazulOffsets(
            MakeArrayView(Bytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, Brazul);
        const bool bMiniGameClass = Actor.ClassId == 0x44 || Actor.ClassId == 0x45 ||
            Actor.ClassId == 0x46 || Actor.ClassId == 0x47 || Actor.ClassId == 0x55;
        FKyaMiniGameOffsets MiniGame;
        const bool HasMiniGame = bMiniGameClass && KyaReadMiniGameOffsets(Actor.ClassId,
            MakeArrayView(Bytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, MiniGame);
        FKyaBasicBoxOffsets Box;
        const bool HasBox = Actor.ClassId == 0x39 && KyaReadBasicBoxOffsets(
            MakeArrayView(Bytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, Box);
        FKyaFxOffsets Fx;
        const bool HasFx = Actor.ClassId == 0x2e && KyaReadFxOffsets(
            MakeArrayView(Bytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, Fx);
        FKyaAraignosOffsets Araignos;
        const bool HasAraignos = Actor.ClassId == 0x21 && KyaReadAraignosOffsets(
            MakeArrayView(Bytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, Araignos);
        FKyaProjectileOffsets Projectile;
        const bool HasProjectile = Actor.ClassId == 0x1c && KyaReadProjectileOffsets(
            MakeArrayView(Bytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, Projectile);
        FKyaHedgehogOffsets Hedgehog;
        const bool HasHedgehog = Actor.ClassId == 0x22 && KyaReadHedgehogOffsets(
            MakeArrayView(Bytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, Hedgehog);
        FKyaWindOffsets Wind;
        const bool HasWind = Actor.ClassId == 0x12 && KyaReadWindOffsets(
            MakeArrayView(Bytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, Wind);
        FKyaTrapOffsets Trap;
        const bool HasTrap = Actor.ClassId == 0x2c && KyaReadTrapOffsets(
            MakeArrayView(Bytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, Trap);
        FKyaWeaponOffsets Weapon;
        const bool HasWeapon = Actor.ClassId == 0x20 && KyaReadWeaponOffsets(
            MakeArrayView(Bytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, Weapon);
        FKyaSwitchActorOffsets SwitchData;
        const bool HasSwitch = Actor.ClassId == 0x11 && KyaReadSwitchActorOffsets(
            MakeArrayView(Bytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, SwitchData);
        FKyaBonusOffsets Bonus;
        const bool HasBonus = Actor.ClassId == 0x0e && KyaReadBonusOffsets(
            MakeArrayView(Bytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, Bonus);
        FKyaMickenOffsets Micken;
        const bool HasMicken = Actor.ClassId == 0x07 && KyaReadMickenOffsets(
            MakeArrayView(Bytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, Micken);
        FKyaWolfenOffsets Wolfen;
        const bool HasWolfen = Actor.ClassId == 0x09 && KyaReadWolfenOffsets(
            MakeArrayView(Bytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, Wolfen);
        FKyaFruitOffsets Fruit;
        const bool HasFruit = Actor.ClassId == 0x16 && KyaReadFruitOffsets(
            MakeArrayView(Bytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, Fruit);
        FKyaCommanderOffsets Commander;
        const bool HasCommander = Actor.ClassId == 0x13 && KyaReadCommanderOffsets(
            MakeArrayView(Bytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, Commander);
        FKyaNativOffsets Nativ;
        const bool HasNativ = Actor.ClassId == 0x0a && KyaReadNativOffsets(
            MakeArrayView(Bytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, Nativ);
        FKyaAmortosOffsets Amortos;
        const bool HasAmortos = Actor.ClassId == 0x29 && KyaReadAmortosOffsets(
            MakeArrayView(Bytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, Amortos);
        FKyaTeleporterOffsets Teleporter;
        const bool HasTeleporter = Actor.ClassId == 0x24 && KyaReadTeleporterOffsets(
            MakeArrayView(Bytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, Teleporter);
        FKyaDcaOffsets Dca;
        const bool HasDca = Actor.ClassId == 0x1b && KyaReadDcaOffsets(
            MakeArrayView(Bytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, Dca);
        FKyaBoxOffsets BoxClass;
        const bool HasBoxClass = Actor.ClassId == 0x35 && KyaReadBoxOffsets(
            MakeArrayView(Bytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, BoxClass);
        FKyaMoneyOffsets Money;
        const bool HasMoney = Actor.ClassId == 0x32 && KyaReadMoneyOffsets(
            MakeArrayView(Bytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, Money);
        FKyaFogOffsets Fog;
        const bool HasFog = Actor.ClassId == 0x3d && KyaReadFogOffsets(
            MakeArrayView(Bytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, Fog);
        FKyaShootOffsets Shoot;
        const bool HasShoot = Actor.ClassId == 0x3e && KyaReadShootOffsets(
            MakeArrayView(Bytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, Shoot);
        FKyaAmbreOffsets Ambre;
        const bool HasAmbre = Actor.ClassId == 0x14 && KyaReadAmbreOffsets(
            MakeArrayView(Bytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, Ambre);
        FKyaStillerOffsets Stiller;
        const bool HasStiller = Actor.ClassId == 0x4c && KyaReadStillerOffsets(
            MakeArrayView(Bytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, Stiller);
        FKyaRuneOffsets Rune;
        const bool HasRune = Actor.ClassId == 0x51 && KyaReadRuneOffsets(
            MakeArrayView(Bytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, Rune);
        FKyaFountainOffsets Fountain;
        const bool HasFountain = Actor.ClassId == 0x52 && KyaReadFountainOffsets(
            MakeArrayView(Bytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, Fountain);
        FKyaRopeOffsets Rope;
        const bool HasRope = Actor.ClassId == 0x0f && KyaReadRopeOffsets(
            MakeArrayView(Bytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, Rope);
        FKyaGravityOffsets Gravity;
        const bool HasGravity = Actor.ClassId == 0x10 && KyaReadGravityOffsets(
            MakeArrayView(Bytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, Gravity);
        FKyaNativCmdOffsets Command;
        const bool HasNativCmd = Actor.ClassId == 0x3c && KyaReadNativCmdOffsets(
            MakeArrayView(Bytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, Command);
        FKyaCheckpointsOffsets CheckpointData;
        const bool HasCheckpoints = Actor.ClassId == 0x1e && KyaReadCheckpointsOffsets(
            MakeArrayView(Bytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, CheckpointData);
        FKyaCompanionOffsets Companion;
        const bool HasCompanion = Actor.ClassId == 0x19 && KyaReadCompanionOffsets(
            MakeArrayView(Bytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, Companion);
        FKyaBridgeOffsets Bridge;
        const bool HasBridge = Actor.ClassId == 0x17 && KyaReadBridgeOffsets(
            MakeArrayView(Bytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, Bridge);
        FKyaAtonOffsets Aton;
        const bool HasAton = Actor.ClassId == 0x38 && KyaReadAtonOffsets(
            MakeArrayView(Bytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, Aton);
        FKyaBoomyOffsets Boomy;
        const bool HasBoomy = Actor.ClassId == 0x05 && KyaReadBoomyOffsets(
            MakeArrayView(Bytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, Boomy);
        FKyaKimOffsets Kim;
        const bool HasKim = Actor.ClassId == 0x06 && KyaReadKimOffsets(
            MakeArrayView(Bytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, Kim);
        if (Actor.bHasHunterData != (Actor.ClassId == 0x48 && Offsets.ClassDataSize >= 40)) return false;
        if (Actor.bHasWoodMonsterData != (Actor.ClassId == 0x27 && Offsets.ClassDataSize >= 16)) return false;
        FKyaShockerCmdOffsets ShockerCmd;
        const bool HasShockerCmd = Actor.ClassId == 0x50 && KyaReadShockerCmdOffsets(
            MakeArrayView(Bytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, ShockerCmd);
        if (HasShockerCmd != Actor.bHasShockerCmdData) return false;
        if (HasShockerCmd)
        {
            if (Actor.ShockerCmdField168 < 0 || Actor.ShockerCmdField168 > MAX_uint32 ||
                Actor.ShockerCmdSequences.Num() != ShockerCmd.Sequences.Num()) return false;
            for (int32 Sequence = 0; Sequence < ShockerCmd.Counts.Num(); ++Sequence)
                if (Actor.ShockerCmdSequences[Sequence].Values.Num() != ShockerCmd.Counts[Sequence]) return false;
        }
        if (Actor.bHasAcceleratosData != (Actor.ClassId == 0x3a && Offsets.ClassDataSize >= 20)) return false;
        FKyaAddOnPurchaseOffsets AddOnPurchase;
        const bool HasAddOnPurchase = Actor.ClassId == 0x49 && KyaReadAddOnPurchaseOffsets(
            MakeArrayView(Bytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, AddOnPurchase);
        if (HasAddOnPurchase != Actor.bHasAddOnPurchaseData) return false;
        FKyaAmortosCmdOffsets AmortosCmd;
        const bool HasAmortosCmd = Actor.ClassId == 0x31 && KyaReadAmortosCmdOffsets(
            MakeArrayView(Bytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, AmortosCmd);
        if (HasAmortosCmd != Actor.bHasAmortosCmdData ||
            (HasAmortosCmd && Actor.AmortosCmdActorRefs.Num() != AmortosCmd.Count)) return false;
        FKyaPatternCmdOffsets PatternCmd;
        const bool HasPatternCmd = Actor.ClassId == 0x40 && KyaReadPatternCmdOffsets(
            MakeArrayView(Bytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, PatternCmd);
        if (HasPatternCmd != Actor.bHasPatternCmdData || (HasPatternCmd &&
            (Actor.PatternCmdActorRefs.Num() != PatternCmd.Count || Actor.PatternCmdPattern.bHasField24 != PatternCmd.bHasField24 ||
                Actor.PatternCmdField164 < 0 || Actor.PatternCmdField164 > MAX_uint32 ||
                Actor.PatternCmdField160 < 0 || Actor.PatternCmdField160 > MAX_uint32))) return false;
        FKyaEggOffsets Egg;
        const bool HasEgg = Actor.ClassId == 0x2f && KyaReadEggOffsets(
            MakeArrayView(Bytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, Egg);
        if (HasEgg != Actor.bHasEggData || (HasEgg &&
            (Actor.EggPathIndices.Num() != Egg.Count || Actor.EggField1DC < 0 || Actor.EggField1DC > MAX_uint32 ||
                Actor.EggField1E0 < 0 || Actor.EggField1E0 > MAX_uint32))) return false;
        if (Actor.bHasCareBoyData != (Actor.ClassId == 0x37 && Offsets.ClassDataSize >= 12)) return false;
        FKyaElectrollaOffsets Electrolla;
        const bool HasElectrolla = Actor.ClassId == 0x0b && KyaReadElectrollaOffsets(
            MakeArrayView(Bytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, Electrolla);
        if (HasElectrolla != Actor.bHasElectrollaData || (HasElectrolla &&
            (Actor.ElectrollaSwitch.Targets.Num() != Electrolla.Switch.Count || Actor.ElectrollaSparkCount < 0))) return false;
        FKyaBombLauncherOffsets BombLauncher;
        const bool HasBombLauncher = Actor.ClassId == 0x53 && KyaReadBombLauncherOffsets(
            MakeArrayView(Bytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, BombLauncher);
        if (HasBombLauncher != Actor.bHasBombLauncherData ||
            (HasBombLauncher && Actor.BombLauncherActorRefs.Num() != BombLauncher.Count)) return false;
        FKyaBlazerOffsets Blazer;
        const bool HasBlazer = Actor.ClassId == 0x4d && KyaReadBlazerOffsets(
            MakeArrayView(Bytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, Blazer);
        if (HasBlazer != Actor.bHasBlazerData || (HasBlazer &&
            (Actor.BlazerData.ActorRefs.Num() != Blazer.Count || Actor.BlazerData.bHasWaypointRefs != Blazer.bHasWaypointRefs))) return false;
        FKyaWoofOffsets Woof;
        const bool HasWoof = Actor.ClassId == 0x2a && KyaReadWoofOffsets(
            MakeArrayView(Bytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, Woof);
        if (HasWoof != Actor.bHasWoofData || (HasWoof && Actor.WoofData.ActorRefs.Num() != Woof.Count)) return false;
        FKyaMiniGamesSupportOffsets MiniGamesSupport;
        const bool HasMiniGamesSupport = KyaReadMiniGamesSupportOffsets(Actor.ClassId,
            MakeArrayView(Bytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, MiniGamesSupport);
        if (Actor.bHasMiniGamesManagerData != (HasMiniGamesSupport && Actor.ClassId == 0x42) ||
            Actor.bHasMiniGamesOrganizerData != (HasMiniGamesSupport && Actor.ClassId == 0x43)) return false;
        if (HasMiniGamesSupport && (Actor.ClassId == 0x42 ? Actor.MiniGamesManagerOrganizerRefs.Num() :
            Actor.MiniGamesOrganizerData.MiniGameRefs.Num()) != MiniGamesSupport.Count) return false;
        FKyaCreditsOffsets Credits;
        const bool HasCredits = Actor.ClassId == 0x56 && KyaReadCreditsOffsets(
            MakeArrayView(Bytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, Credits);
        if (HasCredits != Actor.bHasCreditsData || (HasCredits &&
            (Actor.CreditsData.MessageKeys.Num() != Credits.Count || Actor.CreditsData.CompletionSwitch.Targets.Num() != Credits.Switch.Count ||
                Actor.CreditsData.MusicIndex < 0 || Actor.CreditsData.MusicIndex > MAX_uint32))) return false;
        FKyaWantedZooOffsets WantedZoo;
        const bool HasWantedZoo = Actor.ClassId == 0x54 && KyaReadWantedZooOffsets(
            MakeArrayView(Bytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, WantedZoo);
        if (HasWantedZoo != Actor.bHasWantedZooData) return false;
        if (HasWantedZoo)
        {
            const FKyaWantedZooData& Zoo = Actor.WantedZooData;
            if (Zoo.TextPositions.Num() != WantedZoo.TextCount || Zoo.NumberPositions.Num() != WantedZoo.NumberCount ||
                Zoo.Entries.Num() != WantedZoo.Entries.Num()) return false;
            for (int32 Entry = 0; Entry < Zoo.Entries.Num(); ++Entry)
                if (Zoo.Entries[Entry].Conditions.Num() != WantedZoo.Entries[Entry].ConditionCount ||
                    Zoo.Entries[Entry].MessageKeys.Num() != WantedZoo.Entries[Entry].KeyCount ||
                    Zoo.Entries[Entry].Numbers.Num() != WantedZoo.Entries[Entry].NumberCount) return false;
        }
        FKyaJamGutOffsets JamGut;
        const bool HasJamGut = Actor.ClassId == 0x33 && KyaReadJamGutOffsets(
            MakeArrayView(Bytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, JamGut);
        if (HasJamGut != Actor.bHasJamGutData || (HasJamGut &&
            (Actor.JamGutData.bHasField41C != JamGut.bHasField41C || Actor.JamGutData.Checkpoints.Num() != JamGut.CheckpointCount ||
                Actor.JamGutData.Paths.Num() != JamGut.PathCount))) return false;
        const int64 RawOffset = HasMiniGame ? MiniGame.Tail : HasBrazul ? Brazul.Tail : HasBunch ? Bunch.Tail : HasShip ? Ship.Tail : HasPattern ? Pattern.Tail : HasNoseMonster ? NoseMonster.Tail : HasPunchingBall ? PunchingBall.Tail : HasNativShop ? NativShop.Tail : HasShocker ? Shocker.Tail : HasExplosiveDistributor ? ExplosiveDistributor.Tail : HasEventGenerator ? EventGenerator.Tail : HasJamGut ? JamGut.Tail : HasWantedZoo ? WantedZoo.Tail : HasCredits ? Credits.Tail : HasMiniGamesSupport ? MiniGamesSupport.Tail : HasWoof ? Woof.Tail : HasBlazer ? Blazer.Tail : HasBombLauncher ? BombLauncher.Tail : HasElectrolla ? Electrolla.Tail : (Actor.ClassId == 0x37 && Actor.bHasCareBoyData) ? Offsets.ClassData + 12 : HasEgg ? Egg.Tail : HasPatternCmd ? PatternCmd.Tail : HasAmortosCmd ? AmortosCmd.Tail : HasAddOnPurchase ? AddOnPurchase.Tail : (Actor.ClassId == 0x3a && Actor.bHasAcceleratosData) ? Offsets.ClassData + 20 : HasShockerCmd ? ShockerCmd.Tail : (Actor.ClassId == 0x27 && Actor.bHasWoodMonsterData) ? Offsets.ClassData + 16 : (Actor.ClassId == 0x2b && Actor.bHasHelperSignData) ? Offsets.ClassData + 12 : (Actor.ClassId == 0x48 && Actor.bHasHunterData) ? Offsets.ClassData + 40 : HasAton ? Aton.Tail : (HasBridge ? Bridge.Tail : (HasCompanion ? Companion.Tail : (HasGravity ? Gravity.Tail : (HasNativCmd ? Command.Tail : (HasCheckpoints ? CheckpointData.Tail : (HasRope ? Rope.Tail : (HasFountain ? Fountain.Tail : (HasRune ? Rune.Tail : (HasStiller ? Stiller.Tail : (HasAmbre ? Ambre.Tail : (HasShoot ? Shoot.Tail : (HasFog ? Fog.Tail : (HasMoney ? Money.Tail : (HasBoxClass ? BoxClass.Tail : (HasDca ? Dca.Tail : (HasTeleporter ? Teleporter.Tail : (HasAmortos ? Amortos.Tail : (HasNativ ? Nativ.Tail : (HasCommander ? Commander.Tail : (HasFruit ? Fruit.Tail : (HasWolfen ? Wolfen.WeaponIndex : (HasMicken ? Micken.Tail : (HasBonus ? Bonus.Tail : (HasSwitch ? SwitchData.Tail : (HasWeapon ? Weapon.Tail : (HasTrap ? Trap.Tail : (HasWind ? Wind.Tail : (HasProjectile ? Projectile.Tail : (HasHedgehog ? Hedgehog.Tail : (HasAraignos ? Araignos.Tail : (HasFx ? Fx.Tail : (HasBox ? Box.Tail : (HasDetails ? Details.Tail : (HasPlatform ? Platform.Suffix : Offsets.ClassData))))))))))))))))))))))))))))))))));
        const int64 BoomyRawOffset = HasBoomy ? Boomy.Tail : RawOffset;
        if (Actor.bHasHelperSignData != (Actor.ClassId == 0x2b && Offsets.ClassDataSize >= 12)) return false;
        const int64 KimRawOffset = HasKim ? Kim.Tail : RawOffset;
        if (HasPlatform != Actor.bHasPlatformPrefix || HasDetails != Actor.bHasPlatformDetails || HasEventGenerator != Actor.bHasEventGeneratorData ||
            HasExplosiveDistributor != Actor.bHasExplosiveDistributorData ||
            HasShocker != Actor.bHasShockerData ||
            HasNativShop != Actor.bHasNativShopData ||
            HasPunchingBall != Actor.bHasPunchingBallData ||
            HasNoseMonster != Actor.bHasNoseMonsterData ||
            HasPattern != Actor.bHasPatternData ||
            HasShip != Actor.bHasShipData ||
            HasBunch != Actor.bHasBunchData ||
            HasBrazul != Actor.bHasBrazulData ||
            HasMiniGame != Actor.bHasMiniGameData ||
            HasBox != Actor.bHasBasicBoxData ||
            HasFx != Actor.bHasFxData || HasAraignos != Actor.bHasAraignosData || HasProjectile != Actor.bHasProjectileData ||
            HasHedgehog != Actor.bHasHedgehogData ||
            HasWind != Actor.bHasWindData || HasTrap != Actor.bHasTrapData || HasWeapon != Actor.bHasWeaponData ||
            HasSwitch != Actor.bHasSwitchData || HasBonus != Actor.bHasBonusData || HasMicken != Actor.bHasMickenData ||
            HasWolfen != Actor.bHasWolfenData || HasFruit != Actor.bHasFruitData || HasCommander != Actor.bHasCommanderData ||
            HasNativ != Actor.bHasNativData || HasAmortos != Actor.bHasAmortosData ||
            HasTeleporter != Actor.bHasTeleporterData || HasDca != Actor.bHasDcaData ||
            HasBoxClass != Actor.bHasBoxData || HasMoney != Actor.bHasMoneyData ||
            HasFog != Actor.bHasFogData || HasShoot != Actor.bHasShootData ||
            HasAmbre != Actor.bHasAmbreData || HasStiller != Actor.bHasStillerData ||
            HasRune != Actor.bHasRuneData || HasFountain != Actor.bHasBonusFountainData ||
            HasRope != Actor.bHasRopeData || HasGravity != Actor.bHasGravityAwareData ||
            HasNativCmd != Actor.bHasNativCmdData || HasCheckpoints != Actor.bHasTypedCheckpoints ||
            HasCompanion != Actor.bHasCompanionData || HasBridge != Actor.bHasBridgeData ||
            HasAton != Actor.bHasAtonData || HasBoomy != Actor.bHasBoomyData || HasKim != Actor.bHasKimData) return false;
        if (HasCommander && (Actor.CommanderTeamEnemies.Num() != Commander.TeamCount ||
            Actor.CommanderSequence.Num() != Commander.ComponentCount || Actor.CommanderCameraValues.Num() != 14)) return false;
        if (HasAmortos && (Actor.AmortosCheckpointActors.Num() != Amortos.Streams[0].CountValue ||
            Actor.AmortosActors188.Num() != Amortos.Streams[1].CountValue ||
            Actor.AmortosActors18C.Num() != Amortos.Streams[2].CountValue ||
            Actor.AmortosAddOn.SubObjMode != Amortos.SubObjModeValue ||
            Actor.AmortosSwitch1CC.Targets.Num() != Amortos.Switches[0].Count ||
            Actor.AmortosSwitch1D4.Targets.Num() != Amortos.Switches[1].Count)) return false;
        if (HasTeleporter && (Actor.TeleporterDestinations.Num() != Teleporter.DestCountValue ||
            Actor.TeleporterWaypoints.Num() != Teleporter.WaypointCountValue ||
            Actor.TeleporterConditionedOperations.Num() != Teleporter.BlockOps.Num() ||
            Actor.TeleporterSwitch170.Targets.Num() != Teleporter.Switches[0].Count ||
            Actor.TeleporterSwitch178.Targets.Num() != Teleporter.Switches[1].Count ||
            Actor.TeleporterSwitch180.Targets.Num() != Teleporter.Switches[2].Count)) return false;
        if (HasTeleporter)
            for (int32 Block = 0; Block < Actor.TeleporterConditionedOperations.Num(); ++Block)
                if (Actor.TeleporterConditionedOperations[Block].Operations.Num() != Teleporter.BlockOpCountValues[Block] ||
                    Actor.TeleporterConditionedOperations[Block].Conditions.Num() != Teleporter.BlockCondCountValues[Block]) return false;
        if (HasDca && Actor.DcaShotActorRefs.Num() != Dca.ShotCountValue) return false;
        if (HasFog && Actor.FogHierarchies.Num() != Fog.CountValue) return false;
        if (HasShoot && (Actor.ShootAddOn.SubObjMode != Shoot.SubObjModeValue ||
            Actor.ShootLifeMax < 0 || Actor.ShootLifeMax > MAX_uint32)) return false;
        if (HasAmbre && Actor.AmbreSwitch.Targets.Num() != Ambre.Switch.Count) return false;
        if (HasStiller && (Actor.StillerActorRefs.Num() != Stiller.ActorCountValue ||
            Actor.StillerSwitch208.Targets.Num() != Stiller.Switches[0].Count ||
            Actor.StillerSwitch210.Targets.Num() != Stiller.Switches[1].Count ||
            Actor.StillerAddOn.SubObjMode != Stiller.SubObjModeValue)) return false;
        if (HasFountain && Actor.BonusFountainAddOn.SubObjMode != Fountain.SubObjModeValue) return false;
        if (HasRope && Actor.RopeSwitch.Targets.Num() != Rope.Switch.Count) return false;
        if (HasCheckpoints && Actor.Checkpoints.Num() != CheckpointData.CountValue) return false;
        if (HasBridge && Actor.BridgeActorRefs.Num() != Bridge.ActorCountValue) return false;
        if (HasAton && (Actor.AtonLifeMax < 0 || Actor.AtonLifeMax > MAX_uint32)) return false;
        if (HasAton && (Actor.AtonPathIndices.Num() != Aton.PathCountValue ||
            Actor.AtonOtherSubObjs.Num() != Aton.OtherCountValue ||
            Actor.AtonSubObjs.Num() != Aton.AtonCountValue)) return false;
        if (HasAton)
        {
            for (int32 SubIndex = 0; SubIndex < Actor.AtonOtherSubObjs.Num(); ++SubIndex)
                if (Actor.AtonOtherSubObjs[SubIndex].CinematicIds.Num() != int32(ReadU32(Aton.OtherOffsets[SubIndex] + 4))) return false;
            for (int32 SubIndex = 0; SubIndex < Actor.AtonSubObjs.Num(); ++SubIndex)
                if (Actor.AtonSubObjs[SubIndex].CinematicIds.Num() != int32(ReadU32(Aton.AtonOffsets[SubIndex] + 8))) return false;
        }
        if (HasBoomy && (Actor.BoomyTypes.Num() != Boomy.TypeCountValue ||
            Actor.BoomyCamera.Conditions.Num() != Boomy.Camera.ConditionCount ||
            Actor.BoomyCamera.bHasSwitch != Boomy.Camera.bHasSwitch ||
            (Boomy.Camera.bHasSwitch && (Actor.BoomyCamera.SwitchActorsA.Num() != Boomy.Camera.SwitchCountA ||
                Actor.BoomyCamera.SwitchActorsB.Num() != Boomy.Camera.SwitchCountB)) ||
            Actor.BoomyCamera.ConfigOptionalValues.Num() != (Boomy.Camera.ConfigMode == 1 ? 5 : 0) ||
            Actor.BoomyCamera.ExtraValues.Num() != (Boomy.Camera.ExtraCount == 1 ? 12 : 0))) return false;
        if (HasKim)
        {
            if (ReadU32(Kim.Fighter.Blows) != uint32(Actor.FighterBlows.Num()) ||
                ReadU32(Kim.Fighter.Grabs) != uint32(Actor.FighterGrabs.Num()) ||
                ReadU32(Kim.Fighter.GrabReacts) != uint32(Actor.FighterGrabReacts.Num()) ||
                ReadU32(Kim.Fighter.Combos) != uint32(Actor.FighterCombos.Num()) ||
                Actor.FighterField51C.Num() != 8 || Actor.FighterField53C.Num() != 5) return false;
            if (Actor.KimBoneKeys.Num() != 10 ||
                Actor.KimClimbZones.Num() != Kim.ClimbCountValue ||
                Actor.KimTutorialZones.Num() != Kim.TutorialCountValue ||
                Actor.KimProjectileZones.Num() != Kim.ProjectileCountValue ||
                Actor.KimBoomyZonePairs.Num() != Kim.PairCountValue ||
                Actor.KimZoneStream.Num() != Kim.ZoneStreamCountValue ||
                Actor.KimCameras.Num() != Kim.Cameras.Num() ||
                Actor.KimFxHandleTypes.Num() != 3) return false;
            for (int32 TutorialIndex = 0; TutorialIndex < Actor.KimTutorialZones.Num(); ++TutorialIndex)
                if (Actor.KimTutorialZones[TutorialIndex].Targets.Num() != Kim.Tutorials[TutorialIndex].TargetCountValue) return false;
        }
        if (HasNativCmd && (Actor.NativCmdActorRefs.Num() != Command.ActorCountValue || Actor.NativCmdAddOns.Num() != Command.AddOnCountValues.Num()))
            return false;
        if (HasNativCmd)
        {
            int32 SubObjIndex = 0;
            for (int32 AddOnIndex = 0; AddOnIndex < Actor.NativCmdAddOns.Num(); ++AddOnIndex)
            {
                const FKyaNativCmdAddOn& AddOn = Actor.NativCmdAddOns[AddOnIndex];
                if (AddOn.SubObjs.Num() != Command.AddOnCountValues[AddOnIndex]) return false;
                for (const FKyaAddOnSubObj& Sub : AddOn.SubObjs)
                {
                    if (SubObjIndex >= Command.SubObjOffsets.Num()) return false;
                    const int64 SubOffset = Command.SubObjOffsets[SubObjIndex++];
                    if (Sub.CinematicIds.Num() != int32(ReadU32(SubOffset + 4))) return false;
                }
            }
        }
        if (HasBoxClass && (Actor.BoxLifeMax < 0 || Actor.BoxLifeMax > MAX_uint32 ||
            Actor.BoxActorRefs.Num() != BoxClass.ActorCountValue ||
            Actor.BoxSwitch.Targets.Num() != BoxClass.Switch.Count ||
            Actor.BoxAddOn.SubObjMode != BoxClass.SubObjModeValue)) return false;
        if (HasWolfen)
        {
            FKyaWolfenFightParamOffsets Params;
            if (!Actor.ClassData.IsEmpty() ||
                !KyaReadWolfenFightParamOffsets(MakeArrayView(Bytes.GetData(), int32(SceneEnd)), Wolfen.FightParam, Wolfen.Tail, Params) ||
                Params.Tail != Wolfen.Tail ||
                ReadU32(Wolfen.Blows) != uint32(Actor.FighterBlows.Num()) ||
                ReadU32(Wolfen.Grabs) != uint32(Actor.FighterGrabs.Num()) ||
                ReadU32(Wolfen.GrabReacts) != uint32(Actor.FighterGrabReacts.Num()) ||
                ReadU32(Wolfen.Combos) != uint32(Actor.FighterCombos.Num()) ||
                Actor.WolfenFightParamData.Capabilities.Num() != 3 ||
                Actor.WolfenFightParamData.Capability0Fields.Num() != 5 ||
                Actor.WolfenFightParamData.Capability2Fields.Num() != 5 ||
                Actor.WolfenFightParamData.ConfigurationValues.Num() != 4 ||
                Actor.WolfenFightParamData.TuningValues.Num() != 6 ||
                Actor.WolfenFightParamData.Knowledge.Num() != 4)
            return false;
            for (int32 Cap = 0; Cap < 3; ++Cap)
                if (Actor.WolfenFightParamData.Capabilities[Cap].Items.Num() != Params.Capabilities[Cap].Items.Num())
                return false;
            if (Actor.WolfenFightParamData.ComboList1.Num() != int32(ReadU32(Params.ComboList1Count)) ||
                Actor.WolfenFightParamData.ComboList2.Num() != int32(ReadU32(Params.ComboList2Count)))
            return false;
        }
        else if (Actor.ClassData.Num() != Offsets.ClassData + Offsets.ClassDataSize - (HasKim ? Kim.Tail : BoomyRawOffset)) return false;
        if (HasPlatform)
        {
            if (Actor.PlatformActors.Num() != Platform.ActorCount || Actor.PlatformZones.Num() != Platform.ZoneCount ||
                Actor.PlatformNoFrictionZones.Num() != Platform.NoFrictionZones.Num()) return false;
            for (int32 Zone = 0; Zone < Platform.NoFrictionZones.Num(); ++Zone)
                if (uint32(Actor.PlatformNoFrictionZones[Zone].Actors.Num()) != ReadU32(Platform.NoFrictionZones[Zone] + 4)) return false;
        }
        if (HasDetails)
        {
            const FKyaPlatformProperties& P = Actor.PlatformProperties;
            if (Actor.PlatformCameras.Num() != Details.CameraCount || Actor.PlatformSoundsA.Num() != Details.SoundCounts[0] || Actor.PlatformSoundsB.Num() != Details.SoundCounts[1] ||
                P.EffectRef1C < 0 || P.EffectRef1C > MAX_uint32 || P.EffectRef20 < 0 || P.EffectRef20 > MAX_uint32 || P.PlatformFlags < 0 || P.PlatformFlags > MAX_uint32) return false;
        }
        if (HasBox)
        {
            if (Actor.BasicBoxActorRefs.Num() != Box.ActorCount || Actor.BasicBoxSwitch.Targets.Num() != Box.SwitchTargetCount ||
                Actor.BasicBoxAddOn.SubObjMode != Box.SubObjModeValue || Actor.BasicBoxLifeMax < 0 || Actor.BasicBoxLifeMax > MAX_uint32) return false;
        }
        if (HasAraignos && Actor.AraignosAddOn.SubObjMode != Araignos.SubObjModeValue) return false;
        if (HasHedgehog && (Actor.HedgehogAddOn.SubObjMode != Hedgehog.SubObjModeValue ||
            Actor.HedgehogLifeMax < 0 || Actor.HedgehogLifeMax > MAX_uint32)) return false;
        if (HasProjectile && (Actor.ProjectileLifeMax < 0 || Actor.ProjectileLifeMax > MAX_uint32 ||
            Actor.ProjectilePattern.bHasField24 != Projectile.bHasPatternField24)) return false;
        if (HasFruit && (Actor.FruitLifeMax < 0 || Actor.FruitLifeMax > MAX_uint32)) return false;
        if (HasNativ && (Actor.NativLifeMax < 0 || Actor.NativLifeMax > MAX_uint32)) return false;
        if (HasWind)
        {
            if (Actor.WindActorRefs.Num() != Wind.ActorCount) return false;
            const FKyaSwitch* Switches[] = { &Actor.WindSwitch1D0, &Actor.WindSwitch1D8, &Actor.WindSwitch1E0 };
            for (int32 Switch = 0; Switch < 3; ++Switch)
                if (Switches[Switch]->Targets.Num() != (Wind.bHasSwitches ? Wind.Switches[Switch].Count : 0)) return false;
        }
        if (HasTrap)
        {
            if (Actor.TrapActorRefs.Num() != Trap.ActorCount || Actor.TrapStreamEntries.Num() != Trap.TrapCount ||
                Actor.bHasTrapField24 != Trap.bHasField24) return false;
        }
        if (HasSwitch && Actor.SwitchTarget.Targets.Num() != SwitchData.Count) return false;
        for (int32 Behaviour = 0; Behaviour < Actor.Behaviours.Num(); ++Behaviour)
        {
            const FKyaActorBehaviour& Source = Actor.Behaviours[Behaviour];
            const FKyaBehaviourOffsets& Location = Offsets.Behaviours[Behaviour];
            if (Source.Data.Num() != Location.Size) return false;
            if (Source.bHasTypedJamGutBehaviour && (Actor.ClassId != 0x33 || Source.Id != 4 || Location.Size != 16 ||
                Source.JamGutMusicIndex < 0 || Source.JamGutMusicIndex > MAX_uint32)) return false;
            if (Source.bHasTypedEmptyBehaviour &&
                (Location.Size != 0 || !KyaActorFamilies::HasEmptyPayload(Actor.ClassId, Source.Id))) return false;
            if (Source.bHasTypedWoodMonsterBehaviour &&
                (Actor.ClassId != 0x27 || Source.Id != 2 || Location.Size != 12)) return false;
            if (Source.bHasTypedHelperSignBehaviour &&
                (Actor.ClassId != 0x2b || (Source.Id != 2 && Source.Id != 3) ||
                    Location.Size != (Source.Id == 3 ? 4 : 0))) return false;
            if (Source.bHasTypedFxBehaviour)
            {
                FKyaFxBehaviourValues Values;
                if (Actor.ClassId != 0x2e ||
                    !KyaReadFxBehaviour(Source.Id, MakeArrayView(Bytes.GetData(), int32(SceneEnd)), Location.Payload, Location.Size, Values) ||
                    Values.UnknownValues.Num() != Source.FxUnknownValues.Num()) return false;
            }
            if (Source.bHasTypedTrapBehaviour &&
                (Actor.ClassId != 0x2c || (Source.Id != 2 && Source.Id != 3) || Location.Size != 12)) return false;
            if (Source.bHasTypedHedgehogBehaviour)
            {
                FKyaHedgehogBehaviourValues Values;
                if (Actor.ClassId != 0x22 ||
                    !KyaReadHedgehogBehaviour(Source.Id, MakeArrayView(Bytes.GetData(), int32(SceneEnd)),
                        Location.Payload, Location.Size, Values)) return false;
            }
            if (Source.bHasTypedWeaponBehaviour)
            {
                FKyaWeaponBehaviourValues Values;
                if (Actor.ClassId != 0x20 ||
                    !KyaReadWeaponBehaviour(Source.Id, MakeArrayView(Bytes.GetData(), int32(SceneEnd)), Location.Payload, Location.Size, Values) ||
                    Values.Burst.Num() != Source.WeaponBurst.Num() || Values.RpgActorRefs.Num() != Source.WeaponRpgActorRefs.Num()) return false;
            }
            if (Source.bHasTypedBonusBehaviour)
            {
                FKyaBonusBehaviourValues Values;
                if (Actor.ClassId != 0x0e ||
                    !KyaReadBonusBehaviour(Source.Id, MakeArrayView(Bytes.GetData(), int32(SceneEnd)), Location.Payload, Location.Size, Values)) return false;
            }
            if (Source.bHasTypedMickenBehaviour)
            {
                FKyaMickenBehaviourValues Values;
                if (Actor.ClassId != 0x07 ||
                    !KyaReadMickenBehaviour(Source.Id, MakeArrayView(Bytes.GetData(), int32(SceneEnd)), Location.Payload, Location.Size, Values)) return false;
            }
            if (Source.bHasTypedFruitBehaviour)
            {
                if (Actor.ClassId != 0x16) return false;
                FKyaFruitInventoryOffsets Off;
                if (Source.Id == 5)
                {
                    if (!KyaReadFruitInventoryOffsets(MakeArrayView(Bytes.GetData(), int32(SceneEnd)),
                        Location.Payload, Location.Size, Off)) return false;
                }
                else if ((Source.Id != 4 || Location.Size != 4) && (Source.Id != 6 || Location.Size != 8)) return false;
            }
            if (Source.bHasTypedCommanderBehaviour)
            {
                FKyaCommanderBehaviourOffsets Off;
                if (Actor.ClassId != 0x13 || Source.Id != 2 || Source.CommanderSquadConfig.Num() != 2 ||
                    !KyaReadCommanderBehaviourOffsets(MakeArrayView(Bytes.GetData(), int32(SceneEnd)), Location.Payload, Location.Payload + Location.Size, Off)) return false;
            }
            if (Source.bHasTypedNativBehaviour)
            {
                FKyaActorBehaviour Parsed;
                if (Actor.ClassId != 0x0a ||
                    !KyaReadNativBehaviour(Parsed, Source.Id, MakeArrayView(Bytes.GetData(), int32(SceneEnd)), Location.Payload, Location.Size) ||
                    Parsed.NativTrajectories.Num() != Source.NativTrajectories.Num() ||
                    Parsed.NativSpeakValues.Num() != Source.NativSpeakValues.Num() ||
                    Parsed.NativSellerSubObjs.Num() != Source.NativSellerSubObjs.Num()) return false;
                for (int32 Sub = 0; Sub < Parsed.NativSellerSubObjs.Num(); ++Sub)
                    if (Parsed.NativSellerSubObjs[Sub].CinematicIds.Num() != Source.NativSellerSubObjs[Sub].CinematicIds.Num()) return false;
            }
            if (Source.bHasTypedCinematicBehaviour &&
                (Source.Id != 1 || Location.Size != 4)) return false;
            if (Source.bHasTypedDcaControlledBehaviour)
            {
                FKyaDcaBehaviourOffsets Off;
                if (Actor.ClassId != 0x1b || Source.Id != 3 ||
                    !KyaReadDcaBehaviourOffsets(3, MakeArrayView(Bytes.GetData(), int32(SceneEnd)), Location.Payload, Location.Size, Off) || !Off.bControlled) return false;
            }
            if (Source.bHasTypedDcaTrajectoryBehaviour)
            {
                FKyaDcaBehaviourOffsets Off;
                if (Actor.ClassId != 0x1b || Source.Id != 5 ||
                    !KyaReadDcaBehaviourOffsets(5, MakeArrayView(Bytes.GetData(), int32(SceneEnd)), Location.Payload, Location.Size, Off) || !Off.bTrajectory) return false;
            }
            if (Source.bHasTypedFogBehaviour)
            {
                FKyaFogZoneOffsets Off;
                if (Actor.ClassId != 0x3d || Source.Id != 2 ||
                    !KyaReadFogZoneOffsets(MakeArrayView(Bytes.GetData(), int32(SceneEnd)), Location.Payload, Location.Payload + Location.Size, Off) ||
                    Off.CountValue != Source.FogZones.Num()) return false;
            }
            if (Source.bHasTypedMoneyFlockBehaviour)
            {
                FKyaMoneyFlockOffsets Off;
                if (Actor.ClassId != 0x32 || Source.Id != 2 ||
                    !KyaReadMoneyFlockOffsets(MakeArrayView(Bytes.GetData(), int32(SceneEnd)), Location.Payload, Location.Size, Off)) return false;
            }
            if (Source.bHasTypedShootFireBehaviour)
            {
                FKyaShootFireOffsets Off;
                if (Actor.ClassId != 0x3e || Source.Id != 3 ||
                    !KyaReadShootFireOffsets(MakeArrayView(Bytes.GetData(), int32(SceneEnd)), Location.Payload, Location.Payload + Location.Size, Off) ||
                    Off.CountValue != Source.ShootFireActorRefs.Num()) return false;
            }
            if (Source.bHasTypedShootWaveBehaviour)
            {
                FKyaShootWaveOffsets Off;
                if (Actor.ClassId != 0x3e || Source.Id != 4 || Source.ShootWaveValues.Num() != 7 ||
                    !KyaReadShootWaveOffsets(MakeArrayView(Bytes.GetData(), int32(SceneEnd)), Location.Payload, Location.Size, Off)) return false;
            }
            if (Source.bHasTypedGravityFallBehaviour || Source.bHasTypedGravityBellBehaviour)
            {
                FKyaActorBehaviour Parsed;
                if (Actor.ClassId != 0x10 ||
                    !KyaReadGravityBehaviour(Parsed, Source.Id, MakeArrayView(Bytes.GetData(), int32(SceneEnd)), Location.Payload, Location.Size) ||
                    Parsed.GravityBellSwitch.Targets.Num() != Source.GravityBellSwitch.Targets.Num()) return false;
            }
            if (Source.bHasTypedCatchByTrapBehaviour)
            {
                FKyaCatchByTrapOffsets Off;
                if (!KyaActorFamilies::UsesCatchByTrap(Actor.ClassId, Source.Id) ||
                    !KyaReadCatchByTrapOffsets(MakeArrayView(Bytes.GetData(), int32(SceneEnd)),
                        Location.Payload, Location.Size, Off) ||
                    Off.Blocks.Num() != Source.CatchOperations.Num()) return false;
                for (int32 Block = 0; Block < Source.CatchOperations.Num(); ++Block)
                    if (Source.CatchOperations[Block].Operations.Num() != Off.Blocks[Block].Operations.Num() ||
                        Source.CatchOperations[Block].Conditions.Num() != Off.Blocks[Block].Conditions.Num()) return false;
            }
            if (Source.bHasTypedHeroUnknownBehaviour)
            {
                FKyaHeroUnknownOffsets Off;
                if (Actor.ClassId != 0x06 || (Source.Id != 9 && Source.Id != 10 && Source.Id != 11) ||
                    !KyaReadHeroUnknownOffsets(MakeArrayView(Bytes.GetData(), int32(SceneEnd)),
                        Location.Payload, Location.Size, Off)) return false;
            }
            if (Source.bHasTypedInventoryBehaviour)
            {
                FKyaProjectileInventoryOffsets Off;
                if (Actor.ClassId != 0x1c || Source.Id != 9 ||
                    !KyaReadProjectileInventoryOffsets(MakeArrayView(Bytes.GetData(), int32(SceneEnd)),
                        Location.Payload, Location.Size, Off)) return false;
            }
            if (Source.bHasTypedWolfenBehaviour)
            {
                FKyaWolfenBehaviourOffsets Off;
                if ((Actor.ClassId != 0x09 && Actor.ClassId != 0x4e && Actor.ClassId != 0x4f) ||
                    !KyaReadWolfenBehaviourOffsets(Source.Id, MakeArrayView(Bytes.GetData(), int32(SceneEnd)),
                        Location.Payload, Location.Size, Off)) return false;
            }
            if (Source.bHasTypedProjectileBehaviour)
            {
                FKyaProjectileBehaviourOffsets Off;
                if (Actor.ClassId != 0x1c ||
                    !KyaReadProjectileBehaviourOffsets(Source.Id, MakeArrayView(Bytes.GetData(), int32(SceneEnd)),
                        Location.Payload, Location.Size, Off)) return false;
            }
            if (Source.bHasTypedSwitchBehaviour)
            {
                FKyaSwitchBehaviourOffsets Off;
                if (Actor.ClassId != 0x11 ||
                    !KyaReadSwitchBehaviourOffsets(Source.Id, MakeArrayView(Bytes.GetData(), int32(SceneEnd)),
                        Location.Payload, Location.Size, Off)) return false;
            }
            if (Source.bHasTypedPlatformBehaviour)
            {
                FKyaPlatformBehaviourOffsets Off;
                if (Actor.ClassId != 0x0d ||
                    !KyaReadPlatformBehaviourOffsets(Source.Id, MakeArrayView(Bytes.GetData(), int32(SceneEnd)),
                        Location.Payload, Location.Size, Off)) return false;
            }
            if (Source.bHasTypedCompanionBehaviour)
            {
                FKyaCompanionBehaviourOffsets Off;
                if (Actor.ClassId != 0x19 || Source.Id != 3 ||
                    !KyaReadCompanionBehaviourOffsets(MakeArrayView(Bytes.GetData(), int32(SceneEnd)), Location.Payload,
                        Location.Size, Off) ||
                    Off.Alerts.Num() != Source.CompanionAlerts.Num()) return false;
                for (int32 AlertIndex = 0; AlertIndex < Source.CompanionAlerts.Num(); ++AlertIndex)
                {
                    const FKyaCompanionAlert& Alert = Source.CompanionAlerts[AlertIndex];
                    const FKyaCompanionAlertOffsets& AlertOffsets = Off.Alerts[AlertIndex];
                    if (Alert.Conditions.Num() != AlertOffsets.Conditions.CountValue ||
                        Alert.Field34.Num() != AlertOffsets.ListCountValue ||
                        Alert.TargetSwitch.Targets.Num() != AlertOffsets.Switch.Count ||
                        Alert.Field70.Num() != AlertOffsets.Blocks.Num()) return false;
                    for (int32 Block = 0; Block < Alert.Field70.Num(); ++Block)
                        if (Alert.Field70[Block].Operations.Num() != AlertOffsets.Blocks[Block].Operations.Num() ||
                            Alert.Field70[Block].Conditions.Num() != AlertOffsets.Blocks[Block].Conditions.Num()) return false;
                }
            }
            if (Source.bHasTypedBridgeBasicBehaviour)
            {
                FKyaBridgeBasicOffsets Off;
                if (Actor.ClassId != 0x17 || Source.Id != 2 ||
                    !KyaReadBridgeBasicOffsets(MakeArrayView(Bytes.GetData(), int32(SceneEnd)), Location.Payload, Location.Size, Off)) return false;
            }
            if (Source.bHasTypedBridgeWindBehaviour)
            {
                FKyaBridgeWindOffsets Off;
                if (Actor.ClassId != 0x17 || Source.Id != 3 || Source.BridgeWindValues.Num() != 5 ||
                    !KyaReadBridgeWindOffsets(MakeArrayView(Bytes.GetData(), int32(SceneEnd)), Location.Payload, Location.Size, Off)) return false;
            }
            if (Source.bHasTypedClusterZones)
            {
                if (Actor.ClassId != 0x18 || Source.Id != 2) return false;
                TArray<FKyaClusterZoneOffsets> Zones;
                if (!KyaReadClusterZoneOffsets(MakeArrayView(Bytes.GetData(), int32(SceneEnd)), Location.Payload, Location.Size, Zones) || Zones.Num() != Source.ClusterZones.Num()) return false;
                for (int32 ZoneIndex = 0; ZoneIndex < Zones.Num(); ++ZoneIndex)
                {
                    const FKyaClusterZone& Zone = Source.ClusterZones[ZoneIndex];
                    const int32 Counts[] = { Zone.Actors.Num(), Zone.OtherActors.Num(), Zone.Cameras.Num(), Zone.Lights.Num(), Zone.Hashes.Num() };
                    if (Zone.Flags < 0 || Zone.Flags > MAX_uint32) return false;
                    for (int32 List = 0; List < 5; ++List) if (Counts[List] != Zones[ZoneIndex].Counts[List]) return false;
                }
            }
        }
        if (Actor.bHasTypedCheckpoints)
        {
            if (Actor.ClassId != 0x1e) return false;
            FKyaCheckpointsOffsets CheckpointValidation;
            if (!KyaReadCheckpointsOffsets(MakeArrayView(Bytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, CheckpointValidation) ||
                CheckpointValidation.CountValue != Actor.Checkpoints.Num()) return false;
            for (int32 Checkpoint = 0; Checkpoint < CheckpointValidation.CountValue; ++Checkpoint)
                if (CheckpointValidation.Records[Checkpoint].ActorWaypointCount != Actor.Checkpoints[Checkpoint].ActorWaypoints.Num() ||
                    Actor.Checkpoints[Checkpoint].Flags < 0 || Actor.Checkpoints[Checkpoint].Flags > MAX_uint32) return false;
        }
        const FString OriginalName = ANSI_TO_TCHAR(reinterpret_cast<const char*>(Bytes.GetData() + Offsets.Name));
        // Original names can contain extended ANSI characters. The writer keeps
        // their original bytes; restrict only newly authored names to ASCII.
        for (TCHAR Char : Actor.Name) if (Actor.Name != OriginalName && (Char < 1 || Char > 127))
        {
            Diagnostics.FailedActorIndex = Index;
            Diagnostics.FailedProperty = TEXT("Name");
            Diagnostics.Errors.Add(FString::Printf(TEXT("Actor %d: edited names must use ASCII characters."), Index));
            return false;
        }
        if (Align(Actor.Name.Len() + 1, 4) != Offsets.NameCapacity)
        {
            Diagnostics.FailedActorIndex = Index;
            Diagnostics.FailedProperty = TEXT("Name");
            Diagnostics.Errors.Add(FString::Printf(
                TEXT("Actor %d name needs %d serialized bytes but the record slot holds %d; names cannot be resized."),
                Index, Align(Actor.Name.Len() + 1, 4), int32(Offsets.NameCapacity)));
            return false;
        }
        if (Actor.Models.Num() * 2 != Offsets.ModelStrings.Num())
        {
            Diagnostics.FailedActorIndex = Index;
            Diagnostics.FailedProperty = TEXT("Models");
            Diagnostics.Errors.Add(FString::Printf(
                TEXT("Actor %d model reference count changed (%d models, %d serialized strings); "
                    "model lists cannot be resized."), Index, Actor.Models.Num(),
                int32(Offsets.ModelStrings.Num())));
            return false;
        }
        for (int32 Model = 0; Model < Actor.Models.Num(); ++Model)
            for (int32 Part = 0; Part < 2; ++Part)
            {
                const FString& Name = Part == 0 ? Actor.Models[Model].MeshName : Actor.Models[Model].TextureName;
                for (TCHAR Char : Name) if (Char < 1 || Char > 127)
                {
                    Diagnostics.FailedActorIndex = Index;
                    Diagnostics.FailedProperty = TEXT("Models");
                    Diagnostics.Errors.Add(FString::Printf(
                        TEXT("Actor %d model name uses characters that cannot be serialized."), Index));
                    return false;
                }
                if (Align(Name.Len() + 1, 4) != Offsets.ModelStrings[Model * 2 + Part].Capacity)
                {
                    Diagnostics.FailedActorIndex = Index;
                    Diagnostics.FailedProperty = TEXT("Models");
                    Diagnostics.Errors.Add(FString::Printf(
                        TEXT("Actor %d model name needs %d serialized bytes but its slot holds %d; "
                            "model names cannot be resized."), Index, Align(Name.Len() + 1, 4),
                        int32(Offsets.ModelStrings[Model * 2 + Part].Capacity)));
                    return false;
                }
            }
    }
    for (int32 Index = 0; Index < Paths.Num(); ++Index)
    {
        const FKyaPath& Path = Bank->SceneEnvironment.Paths[Index];
        const FKyaPathOffsets& Offsets = Paths[Index];
        const int32 Lengths[7] = { Path.Points.Num(), Path.Rotations.Num(), Path.ExtraVectors.Num(), Path.Delays.Num(), Path.PointFlags.Num(), Path.ExtraValues.Num(), Path.ExtraWords.Num() };
        if (Path.DataFlags != Offsets.Flags || Path.Mode < 0 || Path.Mode > MAX_uint32 || Path.Type < 0 || Path.Type > MAX_uint32 || Path.PathType < 0 || Path.PathType > MAX_uint32) return false;
        for (int32 Array = 0; Array < 7; ++Array)
            if (Lengths[Array] != ((Offsets.Flags & (1u << Array)) ? int64(Offsets.Count) : 0)) return false;
        for (int64 Value : Path.PointFlags) if (Value < 0 || Value > MAX_uint32) return false;
        for (int64 Value : Path.ExtraWords) if (Value < 0 || Value > MAX_uint32) return false;
    }
    for (int32 Index = 0; Index < Objectives.Num(); ++Index)
    {
        const FKyaObjective& Objective = Bank->SceneEnvironment.Objectives[Index];
        if (Objective.Fields.Num() != 6 || Objective.Targets.Num() != Objectives[Index].Targets.Num()) return false;
        for (int64 Field : Objective.Fields) if (Field < 0 || Field > MAX_uint32) return false;
        for (const FKyaObjectiveTarget& Target : Objective.Targets)
            if (Target.MessageKeyA < 0 || Target.MessageKeyA > MAX_uint32 || Target.MessageKeyB < 0 || Target.MessageKeyB > MAX_uint32) return false;
    }
    int64 Pos = Scene->Offset + 36;
    const int64 End = Scene->Offset + Scene->Size;
    if (Pos + 4 > End) return false;
    const uint32 MeshCount = ReadU32(Pos);
    Pos += 4;
    if (MeshCount > uint64(End - Pos) / 4 || MeshCount != uint32(Bank->SceneConfiguration.CommonMeshValues.Num()) ||
        Bank->SceneConfiguration.HeaderWords.Num() != 2 || Bank->SceneConfiguration.PostAnimationWords.Num() != 4 ||
        Bank->SceneConfiguration.ParticleWord < 0 || Bank->SceneConfiguration.ParticleWord > MAX_uint32) return false;
    for (int64 Word : Bank->SceneConfiguration.HeaderWords) if (Word < 0 || Word > MAX_uint32) return false;
    for (int64 Word : Bank->SceneConfiguration.PostAnimationWords) if (Word < 0 || Word > MAX_uint32) return false;
    const int64 MeshDataPos = Pos;
    Pos += int64(MeshCount) * 4;
    if (Pos + 12 > End || Bank->SceneConfiguration.CommonTextureCount < 0) return false;
    const int64 TextureCountPos = Pos;
    Pos += 8;
    const uint32 CollisionCount = ReadU32(Pos);
    Pos += 4;
    if (CollisionCount > uint64(End - Pos) / 4 || CollisionCount != uint32(Bank->SceneConfiguration.CollisionBankValues.Num()))
    {
        Diagnostics.Errors.Add(TEXT("Collision bank count cannot be changed."));
        return false;
    }
    const int64 AnimationCountPos = Pos + int64(CollisionCount) * 4;
    if (AnimationCountPos + 4 > End || Bank->SceneConfiguration.LevelAnimationCount < 0)
    {
        Diagnostics.Errors.Add(TEXT("Invalid animation count or truncated scene configuration."));
        return false;
    }
    const int64 ClassCountPos = AnimationCountPos + 20;
    if (ClassCountPos + 4 > End) return false;
    const uint32 ClassCount = ReadU32(ClassCountPos);
    const int64 ClassDataPos = ClassCountPos + 4;
    if (ClassCount > uint64(End - ClassDataPos) / 8 || ClassCount != uint32(Bank->SceneConfiguration.ActorClassAllocations.Num()) ||
        ClassDataPos + int64(ClassCount) * 8 + 4 > End)
    {
        Diagnostics.Errors.Add(TEXT("Actor class allocation count cannot be changed."));
        return false;
    }
    for (const FKyaActorClassAllocation& Allocation : Bank->SceneConfiguration.ActorClassAllocations)
        if (Allocation.Count < 0) return false;
    auto WriteU32 = [&Bytes](int64 Offset, uint32 Value) {
        for (int32 Byte = 0; Byte != 4; ++Byte)
            Bytes[int32(Offset) + Byte] = uint8(Value >> (Byte * 8));
    };
    uint32 Bits;
    FMemory::Memcpy(&Bits, &Bank->SceneConfiguration.Field1C, sizeof(Bits));
    WriteU32(Scene->Offset + 16, Bits);
    WriteU32(Scene->Offset + 20, uint32(Bank->SceneConfiguration.Field20));
    FMemory::Memcpy(&Bits, &Bank->SceneConfiguration.Field24, sizeof(Bits));
    WriteU32(Scene->Offset + 24, Bits);
    const FKyaSceneEnvironment& Environment = Bank->SceneEnvironment;
    for (int32 Index = 0; Index < Environment.ObjectiveZones.Num(); ++Index)
    {
        WriteU32(ObjectiveTrailing + 4 + Index * 8, uint32(Environment.ObjectiveZones[Index].Field));
        WriteU32(ObjectiveTrailing + 8 + Index * 8, uint32(Environment.ObjectiveZones[Index].ZoneRef));
    }
    WriteU32(ObjectiveEnd, uint32(Environment.Field10C));
    WriteU32(ObjectiveEnd + 4, uint32(Environment.Field110));
    for (int32 Index = 0; Index < Environment.ElevatorCutscenes.Num(); ++Index)
        WriteU32(ObjectiveEnd + 12 + Index * 4, uint32(Environment.ElevatorCutscenes[Index]));
    WriteU32(Setup + 20, uint32(Environment.BackgroundFlags));
    FMemory::Memcpy(&Bits, &Environment.ClipDistance, sizeof(Bits));
    WriteU32(Setup + 24, Bits);
    FMemory::Memcpy(&Bits, &Environment.FogParameter, sizeof(Bits));
    WriteU32(Setup + 28, Bits);
    Bytes[Setup + 32] = Environment.FogColor.R;
    Bytes[Setup + 33] = Environment.FogColor.G;
    Bytes[Setup + 34] = Environment.FogColor.B;
    Bytes[Setup + 35] = Environment.FogColor.A;
    WriteU32(Setup + 36, uint32(Environment.FogFlags));
    WriteU32(Setup + 40, uint32(Environment.MipmapL));
    WriteU32(Setup + 44, uint32(Environment.MipmapK));

    auto WriteFloat = [&](int64 Offset, float Value) {
        uint32 FloatBits;
        FMemory::Memcpy(&FloatBits, &Value, sizeof(FloatBits));
        WriteU32(Offset, FloatBits);
    };
    // Shared CActorFighter prefix writer used by Wolfen (0x09) and Kim (0x06).
    auto WriteFighterPrefix = [&](const FKyaActorPlacement& Actor, const FKyaFighterPrefixOffsets& Offsets) -> bool
    {
        if (Actor.FighterLifeMax < 0 || Actor.FighterLifeMax > MAX_uint32) return false;
        const int64 F = Offsets.Fixed;
        WriteFloat(Offsets.Weight, Actor.FighterWeightB);
        WriteU32(Offsets.Autonomous, uint32(uint64(Actor.FighterLifeMax)));
        WriteFloat(Offsets.Autonomous + 4, Actor.FighterGravityScale);
        WriteFloat(Offsets.Autonomous + 8, Actor.FighterDynamicExtField4);
        WriteFloat(Offsets.Autonomous + 12, Actor.FighterDynamicExtField8);
    WriteU32(F, uint32(Actor.FighterInputAnalyserFlag));
    WriteU32(F + 4, uint32(Actor.FighterCommandMask));
    WriteFloat(F + 8, Actor.FighterField7C0); WriteFloat(F + 12, Actor.FighterField7C4);
    WriteFloat(F + 16, Actor.FighterField7D4); WriteFloat(F + 20, Actor.FighterField7CC);
    WriteFloat(F + 24, Actor.FighterAnatomy20); WriteFloat(F + 28, Actor.FighterAnatomy28);
    WriteFloat(F + 32, Actor.FighterAnatomy24); WriteFloat(F + 36, Actor.FighterAnatomy2C);
    for (int32 Axis = 0; Axis < 3; ++Axis) WriteFloat(F + 40 + Axis * 4, float(Actor.FighterField810A[Axis]));
    for (int32 Axis = 0; Axis < 3; ++Axis) WriteFloat(F + 52 + Axis * 4, float(Actor.FighterField810B[Axis]));
    WriteU32(F + 64, uint32(Actor.FighterField800)); WriteU32(F + 68, uint32(Actor.FighterFightFlags));
    WriteFloat(F + 72, Actor.FighterField50C); WriteFloat(F + 76, Actor.FighterField510);
    WriteFloat(F + 80, Actor.FighterField514); WriteU32(F + 84, uint32(Actor.FighterField518));
    WriteU32(F + 88, uint32(Actor.FighterField58));
    for (int32 Item = 0; Item < Actor.FighterField51C.Num(); ++Item)
        WriteU32(F + 92 + Item * 4, uint32(Actor.FighterField51C[Item]));
    for (int32 Item = 0; Item < Actor.FighterField53C.Num(); ++Item)
        WriteU32(F + 124 + Item * 4, uint32(Actor.FighterField53C[Item]));
    WriteU32(F + 144, uint32(Actor.FighterStaticMeshTextureIndex));
    WriteU32(F + 148, uint32(Actor.FighterStaticMeshMeshIndex));
    WriteU32(F + 152, uint32(Actor.FighterField5F0));
    WriteU32(Offsets.WeaponIndex, uint32(Actor.FighterWeaponActorIndex));
    int64 BlowPos = Offsets.Blows + 4;
    for (int32 BlowIndex = 0; BlowIndex < Actor.FighterBlows.Num(); ++BlowIndex)
    {
        FKyaFighterBlowOffsets Off;
        if (!KyaReadFighterBlowOffsets(MakeArrayView(Bytes.GetData(), int32(SceneEnd)), BlowPos, SceneEnd, Off)) return false;
        const FKyaFighterBlow& Blow = Actor.FighterBlows[BlowIndex];
        if (Align(Blow.Name.Len() + 1, 4) != Off.NameCapacity || Blow.SubObjs.Num() != Off.SubObjCount || Blow.Stages.Num() != 3) return false;
        FMemory::Memzero(Bytes.GetData() + Off.Name, Off.NameCapacity);
        for (int32 Char = 0; Char < Blow.Name.Len(); ++Char) Bytes[Off.Name + Char] = uint8(Blow.Name[Char]);
        WriteU32(Off.Field4, uint32(Blow.Field4A)); WriteU32(Off.Field4 + 4, uint32(Blow.Field4B)); WriteU32(Off.Field4 + 8, uint32(Blow.Field4C));
        WriteFloat(Off.Field8, Blow.Field8); WriteFloat(Off.Field8 + 4, Blow.FieldC); WriteFloat(Off.Field8 + 8, Blow.Field10);
        WriteFloat(Off.Field8 + 12, Blow.Damage); WriteFloat(Off.Field8 + 16, Blow.Field18);
        for (int32 Axis = 0; Axis < 3; ++Axis) WriteFloat(Off.Field20 + Axis * 4, float(Blow.Field20[Axis]));
        for (int32 Axis = 0; Axis < 3; ++Axis) WriteFloat(Off.Field30 + Axis * 4, float(Blow.Field30[Axis]));
        WriteU32(Off.Field40, uint32(Blow.Field40));
        for (int32 Sub = 0; Sub < Blow.SubObjs.Num(); ++Sub)
        {
            const FKyaFighterBlowSubObj& Item = Blow.SubObjs[Sub];
            const int64 S = Off.SubObjs + int64(Sub) * 28;
            WriteU32(S, uint32(Item.BoneA)); WriteU32(S + 4, uint32(Item.BoneB));
            WriteU32(S + 8, uint32(Item.MaterialId)); WriteU32(S + 12, uint32(Item.Field14));
            WriteU32(S + 16, uint32(Item.Flags)); WriteFloat(S + 20, Item.Field1C); WriteU32(S + 24, uint32(Item.Field20));
        }
        WriteU32(Off.BoneRefs, uint32(Blow.BoneRefA)); WriteU32(Off.BoneRefs + 4, uint32(Blow.BoneRefB));
        WriteFloat(Off.CanActivateRange, Blow.CanActivateRange);
        for (int32 Stage = 0; Stage < Blow.Stages.Num(); ++Stage)
        {
            const FKyaFighterStage& Item = Blow.Stages[Stage];
            const int64 S = Off.Stages + int64(Stage) * 28;
            WriteU32(S, uint32(Item.AnimId)); WriteFloat(S + 4, Item.Field4); WriteFloat(S + 8, Item.Field8);
            for (int32 Axis = 0; Axis < 3; ++Axis) WriteFloat(S + 12 + Axis * 4, float(Item.Field10[Axis]));
            WriteU32(S + 24, uint32(Item.Flags));
        }
    }
    int64 GrabPos = Offsets.Grabs + 4;
    for (int32 GrabIndex = 0; GrabIndex < Actor.FighterGrabs.Num(); ++GrabIndex)
    {
        FKyaFighterGrabOffsets Off;
        if (!KyaReadFighterGrabOffsets(MakeArrayView(Bytes.GetData(), int32(SceneEnd)), GrabPos, SceneEnd, Off)) return false;
        const FKyaFighterGrab& Grab = Actor.FighterGrabs[GrabIndex];
        if (Align(Grab.Name.Len() + 1, 4) != Off.NameCapacity || Grab.Stages.Num() != 5) return false;
        FMemory::Memzero(Bytes.GetData() + Off.Name, Off.NameCapacity);
        for (int32 Char = 0; Char < Grab.Name.Len(); ++Char) Bytes[Off.Name + Char] = uint8(Grab.Name[Char]);
        WriteU32(Off.Field4, uint32(Grab.Field4A)); WriteU32(Off.Field4 + 4, uint32(Grab.Field4B)); WriteU32(Off.Field4 + 8, uint32(Grab.Field4C));
        WriteFloat(Off.Field8, Grab.Field8); WriteFloat(Off.Field8 + 4, Grab.FieldC); WriteFloat(Off.Field8 + 8, Grab.Field10);
        WriteU32(Off.Field14, uint32(Grab.Field14));
        for (int32 Axis = 0; Axis < 3; ++Axis) WriteFloat(Off.Field20 + Axis * 4, float(Grab.Field20[Axis]));
        WriteFloat(Off.Field30, Grab.Field30); WriteFloat(Off.Field34, Grab.Field34); WriteU32(Off.Field38, uint32(Grab.Field38));
        WriteFloat(Off.Field3C, Grab.Field3C); WriteFloat(Off.Field60, Grab.Field60); WriteFloat(Off.Field40, Grab.Field40);
        WriteFloat(Off.Field44, Grab.Field44); WriteFloat(Off.Field48, Grab.Field48);
        for (int32 Axis = 0; Axis < 3; ++Axis) WriteFloat(Off.Field50 + Axis * 4, float(Grab.Field50[Axis]));
        WriteU32(Off.Field68, uint32(Grab.Field68)); WriteU32(Off.Field68 + 4, uint32(Grab.Skip));
        for (int32 Stage = 0; Stage < Grab.Stages.Num(); ++Stage)
        {
            const FKyaFighterStage& Item = Grab.Stages[Stage];
            const int64 S = Off.Stages + int64(Stage) * 28;
            WriteU32(S, uint32(Item.AnimId)); WriteFloat(S + 4, Item.Field4); WriteFloat(S + 8, Item.Field8);
            for (int32 Axis = 0; Axis < 3; ++Axis) WriteFloat(S + 12 + Axis * 4, float(Item.Field10[Axis]));
            WriteU32(S + 24, uint32(Item.Flags));
        }
    }
    int64 ReactPos = Offsets.GrabReacts + 4;
    for (int32 ReactIndex = 0; ReactIndex < Actor.FighterGrabReacts.Num(); ++ReactIndex)
    {
        const FKyaFighterGrabReact& React = Actor.FighterGrabReacts[ReactIndex];
        if (Align(React.Name.Len() + 1, 4) != React.NameCapacity) return false;
        FMemory::Memzero(Bytes.GetData() + ReactPos, React.NameCapacity);
        for (int32 Char = 0; Char < React.Name.Len(); ++Char) Bytes[ReactPos + Char] = uint8(React.Name[Char]);
        ReactPos += React.NameCapacity;
        WriteU32(ReactPos, uint32(React.Field4)); WriteU32(ReactPos + 4, uint32(React.Field8));
        ReactPos += 8;
    }
    int64 ComboPos = Offsets.Combos + 4;
    for (int32 ComboIndex = 0; ComboIndex < Actor.FighterCombos.Num(); ++ComboIndex)
    {
        FKyaFighterComboOffsets Off;
        if (!KyaReadFighterComboOffsets(MakeArrayView(Bytes.GetData(), int32(SceneEnd)), ComboPos, SceneEnd, Off)) return false;
        const FKyaFighterCombo& Combo = Actor.FighterCombos[ComboIndex];
        if (Align(Combo.Name.Len() + 1, 4) != Off.NameCapacity || Combo.Inputs.Num() != Off.InputCount ||
            Align(Combo.Action.Len() + 1, 4) != Off.ActionCapacity || Combo.Branches.Num() != Off.BranchOffsets.Num()) return false;
        FMemory::Memzero(Bytes.GetData() + Off.Name, Off.NameCapacity);
        for (int32 Char = 0; Char < Combo.Name.Len(); ++Char) Bytes[Off.Name + Char] = uint8(Combo.Name[Char]);
        WriteU32(Off.Field4, uint32(Combo.Field4A)); WriteU32(Off.Field4 + 4, uint32(Combo.Field4B));
        WriteFloat(Off.Field8, Combo.Field8);
        for (int32 Input = 0; Input < Combo.Inputs.Num(); ++Input) WriteU32(Off.Inputs + Input * 4, uint32(Combo.Inputs[Input]));
        WriteU32(Off.Field2, uint32(Combo.Field2)); WriteU32(Off.Field3, uint32(Combo.Field3)); WriteFloat(Off.FieldC, Combo.FieldC);
        FMemory::Memzero(Bytes.GetData() + Off.Action, Off.ActionCapacity);
        for (int32 Char = 0; Char < Combo.Action.Len(); ++Char) Bytes[Off.Action + Char] = uint8(Combo.Action[Char]);
        for (int32 Branch = 0; Branch < Combo.Branches.Num(); ++Branch)
        {
            if (Align(Combo.Branches[Branch].Len() + 1, 4) != Off.BranchCapacities[Branch]) return false;
            FMemory::Memzero(Bytes.GetData() + Off.BranchOffsets[Branch], Off.BranchCapacities[Branch]);
            for (int32 Char = 0; Char < Combo.Branches[Branch].Len(); ++Char)
                Bytes[Off.BranchOffsets[Branch] + Char] = uint8(Combo.Branches[Branch][Char]);
        }
    }
        return true;
    };
    for (int32 Index = 0; Index < Actors.Num(); ++Index)
    {
        const FKyaActorPlacement& Actor = Environment.Actors[Index];
        const FKyaActorOffsets& Offsets = Actors[Index];
        const FString OriginalName = ANSI_TO_TCHAR(reinterpret_cast<const char*>(Bytes.GetData() + Offsets.Name));
        if (Actor.Name != OriginalName)
        {
            FMemory::Memzero(Bytes.GetData() + Offsets.Name, Offsets.NameCapacity);
            for (int32 Char = 0; Char < Actor.Name.Len(); ++Char) Bytes[Offsets.Name + Char] = uint8(Actor.Name[Char]);
        }
        for (int32 Model = 0; Model < Actor.Models.Num(); ++Model)
            for (int32 Part = 0; Part < 2; ++Part)
            {
                const FString& Name = Part == 0 ? Actor.Models[Model].MeshName : Actor.Models[Model].TextureName;
                const FKyaActorStringOffset& String = Offsets.ModelStrings[Model * 2 + Part];
                if (Name != ANSI_TO_TCHAR(reinterpret_cast<const char*>(Bytes.GetData() + String.Offset)))
                {
                    FMemory::Memzero(Bytes.GetData() + String.Offset, String.Capacity);
                    for (int32 Char = 0; Char < Name.Len(); ++Char) Bytes[String.Offset + Char] = uint8(Name[Char]);
                }
            }
        WriteU32(Offsets.Placement, uint32(Actor.MeshIndex));
        WriteU32(Offsets.Placement + 4, uint32(Actor.TextureIndex));
        WriteU32(Offsets.Placement + 8, uint32(Actor.CollisionIndex));
        for (int32 Behaviour = 0; Behaviour < Actor.Behaviours.Num(); ++Behaviour)
        {
            const FKyaBehaviourOffsets& BehaviourOffset = Offsets.Behaviours[Behaviour];
            WriteU32(BehaviourOffset.Id, uint32(Actor.Behaviours[Behaviour].Id));
            if (BehaviourOffset.Size)
                FMemory::Memcpy(Bytes.GetData() + BehaviourOffset.Payload, Actor.Behaviours[Behaviour].Data.GetData(), BehaviourOffset.Size);
            if (Actor.Behaviours[Behaviour].bHasTypedHedgehogBehaviour)
            {
                const FKyaActorBehaviour& Source = Actor.Behaviours[Behaviour];
                if (!KyaWriteHedgehogBehaviour(Source, Bytes, BehaviourOffset.Payload,
                    BehaviourOffset.Size)) return false;
            }
            if (Actor.Behaviours[Behaviour].bHasTypedHelperSignBehaviour)
            {
                const FKyaActorBehaviour& Source = Actor.Behaviours[Behaviour];
                if (!KyaWriteHelperSignBehaviour(Source, Bytes, BehaviourOffset.Payload,
                    BehaviourOffset.Size)) return false;
            }
            if (Actor.Behaviours[Behaviour].bHasTypedWoodMonsterBehaviour)
            {
                if (!KyaWriteWoodMonsterBehaviour(Actor.Behaviours[Behaviour], Bytes,
                    BehaviourOffset.Payload, BehaviourOffset.Size)) return false;
            }
            if (Actor.Behaviours[Behaviour].bHasTypedFxBehaviour)
            {
                const FKyaActorBehaviour& Source = Actor.Behaviours[Behaviour];
                if (!KyaWriteFxBehaviour(Source, Bytes, BehaviourOffset.Payload,
                    BehaviourOffset.Size)) return false;
            }
            if (Actor.Behaviours[Behaviour].bHasTypedTrapBehaviour)
            {
                const FKyaActorBehaviour& Source = Actor.Behaviours[Behaviour];
                WriteU32(BehaviourOffset.Payload, uint32(Source.TrapBehaviourField0));
                WriteU32(BehaviourOffset.Payload + 4, uint32(Source.TrapBehaviourField4));
                WriteU32(BehaviourOffset.Payload + 8, uint32(Source.TrapBehaviourField8));
            }
            if (Actor.Behaviours[Behaviour].bHasTypedWeaponBehaviour)
            {
                const FKyaActorBehaviour& Source = Actor.Behaviours[Behaviour];
                if (!KyaWriteWeaponBehaviour(Source, Bytes, BehaviourOffset.Payload,
                    BehaviourOffset.Size)) return false;
            }
            if (Actor.Behaviours[Behaviour].bHasTypedBonusBehaviour)
            {
                const FKyaActorBehaviour& Source = Actor.Behaviours[Behaviour];
                if (!KyaWriteBonusBehaviour(Source, Bytes, BehaviourOffset.Payload,
                    BehaviourOffset.Size)) return false;
            }
            if (Actor.Behaviours[Behaviour].bHasTypedNativShopBehaviour)
            {
                const FKyaActorBehaviour& Source = Actor.Behaviours[Behaviour];
                if (!KyaWriteNativShopBehaviour(Source, Bytes, BehaviourOffset.Payload,
                    BehaviourOffset.Size)) return false;
            }
                        if (Actor.Behaviours[Behaviour].bHasTypedMiniGameBetting)
            {
                const FKyaActorBehaviour& Source = Actor.Behaviours[Behaviour];
                if (!KyaWriteMiniGameBehaviour(Source, Bytes, BehaviourOffset.Payload,
                    BehaviourOffset.Size)) return false;
            }
                        if (Actor.Behaviours[Behaviour].bHasTypedBrazulA ||
                Actor.Behaviours[Behaviour].bHasTypedBrazulB)
            {
                const FKyaActorBehaviour& Source = Actor.Behaviours[Behaviour];
                if (!KyaWriteBrazulBehaviour(Source, Bytes, BehaviourOffset.Payload,
                    BehaviourOffset.Size)) return false;
            }
                        if (Actor.Behaviours[Behaviour].bHasTypedBunchRepel ||
                Actor.Behaviours[Behaviour].bHasTypedBunchFence)
            {
                const FKyaActorBehaviour& Source = Actor.Behaviours[Behaviour];
                if (!KyaWriteBunchBehaviour(Source, Bytes, BehaviourOffset.Payload,
                    BehaviourOffset.Size)) return false;
            }
                        if (Actor.Behaviours[Behaviour].bHasTypedPatternBehaviour)
            {
                const FKyaActorBehaviour& Source = Actor.Behaviours[Behaviour];
                if (!KyaWritePatternBehaviour(Source, Bytes, BehaviourOffset.Payload,
                    BehaviourOffset.Size)) return false;
            }
                        if (Actor.Behaviours[Behaviour].bHasTypedShockerBehaviour)
            {
                const FKyaActorBehaviour& Source = Actor.Behaviours[Behaviour];
                if (!KyaWriteShockerBehaviour(Source, Bytes, BehaviourOffset.Payload,
                    BehaviourOffset.Size)) return false;
            }
                        if (Actor.Behaviours[Behaviour].bHasTypedEventGeneratorBehaviour)
            {
                const FKyaActorBehaviour& Source = Actor.Behaviours[Behaviour];
                if (!KyaWriteEventGeneratorBehaviour(Source, Bytes, BehaviourOffset.Payload,
                    BehaviourOffset.Size)) return false;
            }
                        if (Actor.Behaviours[Behaviour].bHasTypedMickenBehaviour)
            {
                const FKyaActorBehaviour& Source = Actor.Behaviours[Behaviour];
                if (!KyaWriteMickenBehaviour(Source, Bytes, BehaviourOffset.Payload,
                    BehaviourOffset.Size)) return false;
            }
                        if (Actor.Behaviours[Behaviour].bHasTypedCommanderBehaviour)
            {
                const FKyaActorBehaviour& Source = Actor.Behaviours[Behaviour];
                if (!KyaWriteCommanderBehaviour(Source, Bytes, BehaviourOffset.Payload,
                    BehaviourOffset.Size)) return false;
            }
            if (Actor.Behaviours[Behaviour].bHasTypedNativBehaviour)
            {
                const FKyaActorBehaviour& Source = Actor.Behaviours[Behaviour];
                if (!KyaWriteNativBehaviour(Source, Bytes, BehaviourOffset.Payload,
                    BehaviourOffset.Size)) return false;
            }
            if (Actor.Behaviours[Behaviour].bHasTypedJamGutBehaviour)
            {
                if (!KyaWriteJamGutBehaviour(Actor.Behaviours[Behaviour], Bytes, BehaviourOffset.Payload, BehaviourOffset.Size)) return false;
            }
            if (Actor.Behaviours[Behaviour].bHasTypedCinematicBehaviour)
            {
                const FKyaActorBehaviour& Source = Actor.Behaviours[Behaviour];
                WriteU32(BehaviourOffset.Payload, uint32(Source.CinematicLeaveBehaviourId));
            }
            if (Actor.Behaviours[Behaviour].bHasTypedDcaControlledBehaviour ||
                Actor.Behaviours[Behaviour].bHasTypedDcaTrajectoryBehaviour)
            {
                const FKyaActorBehaviour& Source = Actor.Behaviours[Behaviour];
                if (!KyaWriteDcaBehaviour(Source, Bytes, BehaviourOffset.Payload,
                    BehaviourOffset.Size)) return false;
            }
            if (Actor.Behaviours[Behaviour].bHasTypedFogBehaviour)
            {
                const FKyaActorBehaviour& Source = Actor.Behaviours[Behaviour];
                if (!KyaWriteFogBehaviour(Source, Bytes, BehaviourOffset.Payload,
                    BehaviourOffset.Size)) return false;
            }
            if (Actor.Behaviours[Behaviour].bHasTypedMoneyFlockBehaviour)
            {
                const FKyaActorBehaviour& Source = Actor.Behaviours[Behaviour];
                if (!KyaWriteMoneyBehaviour(Source, Bytes, BehaviourOffset.Payload,
                    BehaviourOffset.Size)) return false;
            }
            if (Actor.Behaviours[Behaviour].bHasTypedShootFireBehaviour ||
                Actor.Behaviours[Behaviour].bHasTypedShootWaveBehaviour)
            {
                const FKyaActorBehaviour& Source = Actor.Behaviours[Behaviour];
                if (!KyaWriteShootBehaviour(Source, Bytes, BehaviourOffset.Payload,
                    BehaviourOffset.Size)) return false;
            }
            if (Actor.Behaviours[Behaviour].bHasTypedFruitBehaviour)
            {
                const FKyaActorBehaviour& Source = Actor.Behaviours[Behaviour];
                if (!KyaWriteFruitBehaviour(Source, Bytes, BehaviourOffset.Payload,
                    BehaviourOffset.Size)) return false;
            }
            if (Actor.Behaviours[Behaviour].bHasTypedGravityFallBehaviour ||
                Actor.Behaviours[Behaviour].bHasTypedGravityBellBehaviour)
            {
                const FKyaActorBehaviour& Source = Actor.Behaviours[Behaviour];
                if (!KyaWriteGravityBehaviour(Source, Bytes, BehaviourOffset.Payload,
                    BehaviourOffset.Size)) return false;
            }
            if (Actor.Behaviours[Behaviour].bHasTypedCatchByTrapBehaviour)
            {
                const FKyaActorBehaviour& Source = Actor.Behaviours[Behaviour];
                FKyaCatchByTrapOffsets Off;
                if (!KyaReadCatchByTrapOffsets(MakeArrayView(Bank->BankBytes.GetData(), int32(SceneEnd)),
                    BehaviourOffset.Payload, BehaviourOffset.Size, Off)) return false;
                for (int32 Block = 0; Block < Source.CatchOperations.Num(); ++Block)
                {
                    const FKyaConditionedOperation& Result = Source.CatchOperations[Block];
                    const FKyaConditionedOffsets& BlockOffsets = Off.Blocks[Block];
                    for (int32 Item = 0; Item < Result.Operations.Num(); ++Item)
                    {
                        const FKyaScenarioOperation& Op = Result.Operations[Item];
                        const int64 O = BlockOffsets.Operations[Item];
                        WriteU32(O, uint32(Op.Variable));
                        WriteU32(O + 4, uint32(Op.Type));
                        WriteU32(O + 8, uint32(Op.Value));
                    }
                    for (int32 Item = 0; Item < Result.Conditions.Num(); ++Item)
                    {
                        const FKyaScenarioCondition& Condition = Result.Conditions[Item];
                        const int64 C = BlockOffsets.Conditions[Item];
                        WriteU32(C, uint32(Condition.Combiner));
                        WriteU32(C + 4, uint32(Condition.Variable));
                        WriteU32(C + 8, uint32(Condition.Comparison));
                        WriteU32(C + 12, uint32(Condition.Value));
                    }
                }
            }
            if (Actor.Behaviours[Behaviour].bHasTypedHeroUnknownBehaviour)
            {
                const FKyaActorBehaviour& Source = Actor.Behaviours[Behaviour];
                FKyaHeroUnknownOffsets Off;
                if (!KyaReadHeroUnknownOffsets(MakeArrayView(Bank->BankBytes.GetData(), int32(SceneEnd)),
                    BehaviourOffset.Payload, BehaviourOffset.Size, Off)) return false;
                const FKyaHeroUnknownBehaviour& Values = Source.HeroUnknownBehaviour;
                WriteU32(Off.Values, uint32(Values.FxTypeA));
                WriteU32(Off.Values + 4, uint32(Values.TextureA));
                WriteU32(Off.Values + 8, uint32(Values.MeshA));
                WriteU32(Off.Values + 12, uint32(Values.FxTypeB));
                WriteU32(Off.Values + 16, uint32(Values.FxTypeC));
                WriteU32(Off.Values + 20, uint32(Values.TextureB));
                WriteU32(Off.Values + 24, uint32(Values.MeshB));
            }
            if (Actor.Behaviours[Behaviour].bHasTypedInventoryBehaviour)
            {
                const FKyaActorBehaviour& Source = Actor.Behaviours[Behaviour];
                if (!KyaWriteProjectileInventory(Source.InventoryInfo, Bytes, BehaviourOffset.Payload,
                    BehaviourOffset.Size)) return false;
            }
            if (Actor.Behaviours[Behaviour].bHasTypedWolfenBehaviour)
            {
                const FKyaActorBehaviour& Source = Actor.Behaviours[Behaviour];
                if (!KyaWriteWolfenBehaviour(Source, Bytes, BehaviourOffset.Payload,
                    BehaviourOffset.Size)) return false;
            }
            if (Actor.Behaviours[Behaviour].bHasTypedProjectileBehaviour)
            {
                const FKyaActorBehaviour& Source = Actor.Behaviours[Behaviour];
                if (!KyaWriteProjectileBehaviour(Source, Bytes, BehaviourOffset.Payload,
                    BehaviourOffset.Size)) return false;
            }
            if (Actor.Behaviours[Behaviour].bHasTypedSwitchBehaviour)
            {
                const FKyaActorBehaviour& Source = Actor.Behaviours[Behaviour];
                if (!KyaWriteSwitchBehaviour(Source, Bytes, BehaviourOffset.Payload,
                    BehaviourOffset.Size)) return false;
            }
            if (Actor.Behaviours[Behaviour].bHasTypedPlatformBehaviour)
            {
                const FKyaActorBehaviour& Source = Actor.Behaviours[Behaviour];
                if (!KyaWriteMovingPlatformBehaviour(Source, Bytes, BehaviourOffset.Payload,
                    BehaviourOffset.Size)) return false;
            }
            if (Actor.Behaviours[Behaviour].bHasTypedCompanionBehaviour)
            {
                const FKyaActorBehaviour& Source = Actor.Behaviours[Behaviour];
                if (!KyaWriteCompanionBehaviour(Source, Bytes, BehaviourOffset.Payload,
                    BehaviourOffset.Size)) return false;
            }
            if (Actor.Behaviours[Behaviour].bHasTypedBridgeBasicBehaviour ||
                Actor.Behaviours[Behaviour].bHasTypedBridgeWindBehaviour)
            {
                const FKyaActorBehaviour& Source = Actor.Behaviours[Behaviour];
                if (!KyaWriteBridgeBehaviour(Source, Bytes, BehaviourOffset.Payload,
                    BehaviourOffset.Size)) return false;
            }
            if (Actor.Behaviours[Behaviour].bHasTypedClusterZones)
            {
                const FKyaActorBehaviour& Source = Actor.Behaviours[Behaviour];
                if (!KyaWriteClusteriserBehaviour(Source, Bytes, BehaviourOffset.Payload,
                    BehaviourOffset.Size)) return false;
            }
        }
        const int64 Common = Offsets.Common;
        WriteU32(Common, uint32(Actor.DefaultBehaviourId));
        for (int32 Axis = 0; Axis < 4; ++Axis) WriteFloat(Common + 4 + Axis * 4, float(Actor.BoundingSphere[Axis]));
        WriteFloat(Common + 20, Actor.Clipping);
        WriteFloat(Common + 24, Actor.VisibilityDistance);
        WriteFloat(Common + 28, Actor.CullingDistance);
        WriteFloat(Common + 32, Actor.Field20);
        WriteU32(Common + 36, uint32(Actor.SectorId));
        WriteU32(Common + 40, uint32(Actor.HashCode));
        WriteFloat(Common + 44, float(Actor.LodBiases.X));
        WriteFloat(Common + 48, float(Actor.LodBiases.Y));
        WriteU32(Common + 56, uint32(Actor.ActorFlags));
        WriteU32(Common + 60, uint32(Actor.AnimationLayers));
        WriteU32(Common + 64, uint32(Actor.Field40));
        WriteU32(Common + 72, uint32(Actor.LightingFlags));
        WriteFloat(Common + 76, Actor.LightingValue);
        for (int32 ConditionIndex = 0; ConditionIndex < Actor.Conditions.Num(); ++ConditionIndex)
        {
            const FKyaScenarioCondition& Condition = Actor.Conditions[ConditionIndex];
            const int64 Offset = Offsets.Conditions + int64(ConditionIndex) * 16;
            WriteU32(Offset, uint32(Condition.Combiner));
            WriteU32(Offset + 4, uint32(Condition.Variable));
            WriteU32(Offset + 8, uint32(Condition.Comparison));
            WriteU32(Offset + 12, uint32(Condition.Value));
        }
        FKyaPlatformPrefixOffsets Platform;
        const bool HasPlatform = Actor.bHasPlatformPrefix && KyaReadPlatformPrefix(MakeArrayView(Bank->BankBytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, Platform);
        FKyaPlatformDetailsOffsets Details;
        const bool HasDetails = HasPlatform && Actor.bHasPlatformDetails && KyaReadPlatformDetails(MakeArrayView(Bank->BankBytes.GetData(), int32(SceneEnd)), Platform.Suffix, Offsets.ClassData + Offsets.ClassDataSize - Platform.Suffix, Details);
        FKyaEventGeneratorOffsets EventGenerator;
        const bool HasEventGenerator = Actor.ClassId == 0x3b && KyaReadEventGeneratorOffsets(
            MakeArrayView(Bytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, EventGenerator);
        FKyaExplosiveDistributorOffsets ExplosiveDistributor;
        const bool HasExplosiveDistributor = Actor.ClassId == 0x1d && KyaReadExplosiveDistributorOffsets(
            MakeArrayView(Bytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, ExplosiveDistributor);
        FKyaShockerOffsets Shocker;
        const bool HasShocker = Actor.ClassId == 0x4b && KyaReadShockerOffsets(
            MakeArrayView(Bytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, Shocker);
        FKyaNativShopOffsets NativShop;
        const bool HasNativShop = Actor.ClassId == 0x34 && KyaReadNativShopOffsets(
            MakeArrayView(Bytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, NativShop);
        FKyaPunchingBallOffsets PunchingBall;
        const bool HasPunchingBall = Actor.ClassId == 0x30 && KyaReadPunchingBallOffsets(
            MakeArrayView(Bytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, PunchingBall);
        FKyaNoseMonsterOffsets NoseMonster;
        const bool HasNoseMonster = Actor.ClassId == 0x26 && KyaReadNoseMonsterOffsets(
            MakeArrayView(Bytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, NoseMonster);
        FKyaPatternOffsets Pattern;
        const bool HasPattern = Actor.ClassId == 0x3f && KyaReadPatternOffsets(
            MakeArrayView(Bytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, Pattern);
        FKyaShipOffsets Ship;
        const bool HasShip = Actor.ClassId == 0x36 && KyaReadShipOffsets(
            MakeArrayView(Bytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, Ship);
        FKyaBunchOffsets Bunch;
        const bool HasBunch = Actor.ClassId == 0x4e && KyaReadBunchOffsets(
            MakeArrayView(Bytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, Bunch);
        FKyaBrazulOffsets Brazul;
        const bool HasBrazul = Actor.ClassId == 0x4f && KyaReadBrazulOffsets(
            MakeArrayView(Bytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, Brazul);
        const bool bMiniGameClass = Actor.ClassId == 0x44 || Actor.ClassId == 0x45 ||
            Actor.ClassId == 0x46 || Actor.ClassId == 0x47 || Actor.ClassId == 0x55;
        FKyaMiniGameOffsets MiniGame;
        const bool HasMiniGame = bMiniGameClass && KyaReadMiniGameOffsets(Actor.ClassId,
            MakeArrayView(Bytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, MiniGame);
        FKyaBasicBoxOffsets Box;
        const bool HasBox = Actor.ClassId == 0x39 && KyaReadBasicBoxOffsets(
            MakeArrayView(Bank->BankBytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, Box);
        FKyaFxOffsets Fx;
        const bool HasFx = Actor.ClassId == 0x2e && KyaReadFxOffsets(
            MakeArrayView(Bank->BankBytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, Fx);
        FKyaAraignosOffsets Araignos;
        const bool HasAraignos = Actor.ClassId == 0x21 && KyaReadAraignosOffsets(
            MakeArrayView(Bank->BankBytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, Araignos);
        FKyaProjectileOffsets Projectile;
        const bool HasProjectile = Actor.ClassId == 0x1c && KyaReadProjectileOffsets(
            MakeArrayView(Bank->BankBytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, Projectile);
        FKyaHedgehogOffsets Hedgehog;
        const bool HasHedgehog = Actor.ClassId == 0x22 && KyaReadHedgehogOffsets(
            MakeArrayView(Bank->BankBytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, Hedgehog);
        FKyaWindOffsets Wind;
        const bool HasWind = Actor.ClassId == 0x12 && KyaReadWindOffsets(
            MakeArrayView(Bank->BankBytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, Wind);
        FKyaTrapOffsets Trap;
        const bool HasTrap = Actor.ClassId == 0x2c && KyaReadTrapOffsets(
            MakeArrayView(Bank->BankBytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, Trap);
        FKyaWeaponOffsets Weapon;
        const bool HasWeapon = Actor.ClassId == 0x20 && KyaReadWeaponOffsets(
            MakeArrayView(Bank->BankBytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, Weapon);
        FKyaSwitchActorOffsets SwitchData;
        const bool HasSwitch = Actor.ClassId == 0x11 && KyaReadSwitchActorOffsets(
            MakeArrayView(Bank->BankBytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, SwitchData);
        FKyaBonusOffsets Bonus;
        const bool HasBonus = Actor.ClassId == 0x0e && KyaReadBonusOffsets(
            MakeArrayView(Bank->BankBytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, Bonus);
        FKyaMickenOffsets Micken;
        const bool HasMicken = Actor.ClassId == 0x07 && KyaReadMickenOffsets(
            MakeArrayView(Bank->BankBytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, Micken);
        FKyaWolfenOffsets Wolfen;
        const bool HasWolfen = Actor.ClassId == 0x09 && KyaReadWolfenOffsets(
            MakeArrayView(Bank->BankBytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, Wolfen);
        FKyaFruitOffsets Fruit;
        const bool HasFruit = Actor.ClassId == 0x16 && KyaReadFruitOffsets(
            MakeArrayView(Bank->BankBytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, Fruit);
        FKyaCommanderOffsets Commander;
        const bool HasCommander = Actor.ClassId == 0x13 && KyaReadCommanderOffsets(
            MakeArrayView(Bank->BankBytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, Commander);
        FKyaNativOffsets Nativ;
        const bool HasNativ = Actor.ClassId == 0x0a && KyaReadNativOffsets(
            MakeArrayView(Bank->BankBytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, Nativ);
        FKyaAmortosOffsets Amortos;
        const bool HasAmortos = Actor.ClassId == 0x29 && KyaReadAmortosOffsets(
            MakeArrayView(Bank->BankBytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, Amortos);
        FKyaTeleporterOffsets Teleporter;
        const bool HasTeleporter = Actor.ClassId == 0x24 && KyaReadTeleporterOffsets(
            MakeArrayView(Bank->BankBytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, Teleporter);
        FKyaDcaOffsets Dca;
        const bool HasDca = Actor.ClassId == 0x1b && KyaReadDcaOffsets(
            MakeArrayView(Bank->BankBytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, Dca);
        FKyaBoxOffsets BoxClass;
        const bool HasBoxClass = Actor.ClassId == 0x35 && KyaReadBoxOffsets(
            MakeArrayView(Bank->BankBytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, BoxClass);
        FKyaMoneyOffsets Money;
        const bool HasMoney = Actor.ClassId == 0x32 && KyaReadMoneyOffsets(
            MakeArrayView(Bank->BankBytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, Money);
        FKyaFogOffsets Fog;
        const bool HasFog = Actor.ClassId == 0x3d && KyaReadFogOffsets(
            MakeArrayView(Bank->BankBytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, Fog);
        FKyaShootOffsets Shoot;
        const bool HasShoot = Actor.ClassId == 0x3e && KyaReadShootOffsets(
            MakeArrayView(Bank->BankBytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, Shoot);
        FKyaAmbreOffsets Ambre;
        const bool HasAmbre = Actor.ClassId == 0x14 && KyaReadAmbreOffsets(
            MakeArrayView(Bank->BankBytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, Ambre);
        FKyaStillerOffsets Stiller;
        const bool HasStiller = Actor.ClassId == 0x4c && KyaReadStillerOffsets(
            MakeArrayView(Bank->BankBytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, Stiller);
        FKyaRuneOffsets Rune;
        const bool HasRune = Actor.ClassId == 0x51 && KyaReadRuneOffsets(
            MakeArrayView(Bank->BankBytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, Rune);
        FKyaFountainOffsets Fountain;
        const bool HasFountain = Actor.ClassId == 0x52 && KyaReadFountainOffsets(
            MakeArrayView(Bank->BankBytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, Fountain);
        FKyaRopeOffsets Rope;
        const bool HasRope = Actor.ClassId == 0x0f && KyaReadRopeOffsets(
            MakeArrayView(Bank->BankBytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, Rope);
        FKyaGravityOffsets Gravity;
        const bool HasGravity = Actor.ClassId == 0x10 && KyaReadGravityOffsets(
            MakeArrayView(Bank->BankBytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, Gravity);
        FKyaNativCmdOffsets Command;
        const bool HasNativCmd = Actor.ClassId == 0x3c && KyaReadNativCmdOffsets(
            MakeArrayView(Bank->BankBytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, Command);
        FKyaCheckpointsOffsets CheckpointData;
        const bool HasCheckpoints = Actor.ClassId == 0x1e && KyaReadCheckpointsOffsets(
            MakeArrayView(Bank->BankBytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, CheckpointData);
        FKyaCompanionOffsets Companion;
        const bool HasCompanion = Actor.ClassId == 0x19 && KyaReadCompanionOffsets(
            MakeArrayView(Bank->BankBytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, Companion);
        FKyaBridgeOffsets Bridge;
        const bool HasBridge = Actor.ClassId == 0x17 && KyaReadBridgeOffsets(
            MakeArrayView(Bank->BankBytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, Bridge);
        FKyaAtonOffsets Aton;
        const bool HasAton = Actor.ClassId == 0x38 && KyaReadAtonOffsets(
            MakeArrayView(Bank->BankBytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, Aton);
        FKyaBoomyOffsets Boomy;
        const bool HasBoomy = Actor.ClassId == 0x05 && KyaReadBoomyOffsets(
            MakeArrayView(Bank->BankBytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, Boomy);
        FKyaKimOffsets Kim;
        const bool HasKim = Actor.ClassId == 0x06 && KyaReadKimOffsets(
            MakeArrayView(Bank->BankBytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, Kim);
        FKyaWolfenFightParamOffsets FightParam;
        if (HasWolfen && !KyaReadWolfenFightParamOffsets(MakeArrayView(Bank->BankBytes.GetData(), int32(SceneEnd)), Wolfen.FightParam, Wolfen.Tail, FightParam)) return false;
        if (Actor.bHasWoodMonsterData != (Actor.ClassId == 0x27 && Offsets.ClassDataSize >= 16)) return false;
        FKyaShockerCmdOffsets ShockerCmd;
        const bool HasShockerCmd = Actor.ClassId == 0x50 && KyaReadShockerCmdOffsets(
            MakeArrayView(Bytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, ShockerCmd);
        if (HasShockerCmd != Actor.bHasShockerCmdData) return false;
        if (HasShockerCmd)
        {
            if (Actor.ShockerCmdField168 < 0 || Actor.ShockerCmdField168 > MAX_uint32 ||
                Actor.ShockerCmdSequences.Num() != ShockerCmd.Sequences.Num()) return false;
            for (int32 Sequence = 0; Sequence < ShockerCmd.Counts.Num(); ++Sequence)
                if (Actor.ShockerCmdSequences[Sequence].Values.Num() != ShockerCmd.Counts[Sequence]) return false;
        }
        if (Actor.bHasAcceleratosData != (Actor.ClassId == 0x3a && Offsets.ClassDataSize >= 20)) return false;
        FKyaAddOnPurchaseOffsets AddOnPurchase;
        const bool HasAddOnPurchase = Actor.ClassId == 0x49 && KyaReadAddOnPurchaseOffsets(
            MakeArrayView(Bytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, AddOnPurchase);
        if (HasAddOnPurchase != Actor.bHasAddOnPurchaseData) return false;
        FKyaAmortosCmdOffsets AmortosCmd;
        const bool HasAmortosCmd = Actor.ClassId == 0x31 && KyaReadAmortosCmdOffsets(
            MakeArrayView(Bytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, AmortosCmd);
        if (HasAmortosCmd != Actor.bHasAmortosCmdData ||
            (HasAmortosCmd && Actor.AmortosCmdActorRefs.Num() != AmortosCmd.Count)) return false;
        FKyaPatternCmdOffsets PatternCmd;
        const bool HasPatternCmd = Actor.ClassId == 0x40 && KyaReadPatternCmdOffsets(
            MakeArrayView(Bytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, PatternCmd);
        if (HasPatternCmd != Actor.bHasPatternCmdData || (HasPatternCmd &&
            (Actor.PatternCmdActorRefs.Num() != PatternCmd.Count || Actor.PatternCmdPattern.bHasField24 != PatternCmd.bHasField24 ||
                Actor.PatternCmdField164 < 0 || Actor.PatternCmdField164 > MAX_uint32 ||
                Actor.PatternCmdField160 < 0 || Actor.PatternCmdField160 > MAX_uint32))) return false;
        FKyaEggOffsets Egg;
        const bool HasEgg = Actor.ClassId == 0x2f && KyaReadEggOffsets(
            MakeArrayView(Bytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, Egg);
        if (HasEgg != Actor.bHasEggData || (HasEgg &&
            (Actor.EggPathIndices.Num() != Egg.Count || Actor.EggField1DC < 0 || Actor.EggField1DC > MAX_uint32 ||
                Actor.EggField1E0 < 0 || Actor.EggField1E0 > MAX_uint32))) return false;
        if (Actor.bHasCareBoyData != (Actor.ClassId == 0x37 && Offsets.ClassDataSize >= 12)) return false;
        FKyaElectrollaOffsets Electrolla;
        const bool HasElectrolla = Actor.ClassId == 0x0b && KyaReadElectrollaOffsets(
            MakeArrayView(Bytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, Electrolla);
        if (HasElectrolla != Actor.bHasElectrollaData || (HasElectrolla &&
            (Actor.ElectrollaSwitch.Targets.Num() != Electrolla.Switch.Count || Actor.ElectrollaSparkCount < 0))) return false;
        FKyaBombLauncherOffsets BombLauncher;
        const bool HasBombLauncher = Actor.ClassId == 0x53 && KyaReadBombLauncherOffsets(
            MakeArrayView(Bytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, BombLauncher);
        if (HasBombLauncher != Actor.bHasBombLauncherData ||
            (HasBombLauncher && Actor.BombLauncherActorRefs.Num() != BombLauncher.Count)) return false;
        FKyaBlazerOffsets Blazer;
        const bool HasBlazer = Actor.ClassId == 0x4d && KyaReadBlazerOffsets(
            MakeArrayView(Bytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, Blazer);
        if (HasBlazer != Actor.bHasBlazerData || (HasBlazer &&
            (Actor.BlazerData.ActorRefs.Num() != Blazer.Count || Actor.BlazerData.bHasWaypointRefs != Blazer.bHasWaypointRefs))) return false;
        FKyaWoofOffsets Woof;
        const bool HasWoof = Actor.ClassId == 0x2a && KyaReadWoofOffsets(
            MakeArrayView(Bytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, Woof);
        if (HasWoof != Actor.bHasWoofData || (HasWoof && Actor.WoofData.ActorRefs.Num() != Woof.Count)) return false;
        FKyaMiniGamesSupportOffsets MiniGamesSupport;
        const bool HasMiniGamesSupport = KyaReadMiniGamesSupportOffsets(Actor.ClassId,
            MakeArrayView(Bytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, MiniGamesSupport);
        if (Actor.bHasMiniGamesManagerData != (HasMiniGamesSupport && Actor.ClassId == 0x42) ||
            Actor.bHasMiniGamesOrganizerData != (HasMiniGamesSupport && Actor.ClassId == 0x43)) return false;
        if (HasMiniGamesSupport && (Actor.ClassId == 0x42 ? Actor.MiniGamesManagerOrganizerRefs.Num() :
            Actor.MiniGamesOrganizerData.MiniGameRefs.Num()) != MiniGamesSupport.Count) return false;
        FKyaCreditsOffsets Credits;
        const bool HasCredits = Actor.ClassId == 0x56 && KyaReadCreditsOffsets(
            MakeArrayView(Bytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, Credits);
        if (HasCredits != Actor.bHasCreditsData || (HasCredits &&
            (Actor.CreditsData.MessageKeys.Num() != Credits.Count || Actor.CreditsData.CompletionSwitch.Targets.Num() != Credits.Switch.Count ||
                Actor.CreditsData.MusicIndex < 0 || Actor.CreditsData.MusicIndex > MAX_uint32))) return false;
        FKyaWantedZooOffsets WantedZoo;
        const bool HasWantedZoo = Actor.ClassId == 0x54 && KyaReadWantedZooOffsets(
            MakeArrayView(Bytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, WantedZoo);
        if (HasWantedZoo != Actor.bHasWantedZooData) return false;
        if (HasWantedZoo)
        {
            const FKyaWantedZooData& Zoo = Actor.WantedZooData;
            if (Zoo.TextPositions.Num() != WantedZoo.TextCount || Zoo.NumberPositions.Num() != WantedZoo.NumberCount ||
                Zoo.Entries.Num() != WantedZoo.Entries.Num()) return false;
            for (int32 Entry = 0; Entry < Zoo.Entries.Num(); ++Entry)
                if (Zoo.Entries[Entry].Conditions.Num() != WantedZoo.Entries[Entry].ConditionCount ||
                    Zoo.Entries[Entry].MessageKeys.Num() != WantedZoo.Entries[Entry].KeyCount ||
                    Zoo.Entries[Entry].Numbers.Num() != WantedZoo.Entries[Entry].NumberCount) return false;
        }
        FKyaJamGutOffsets JamGut;
        const bool HasJamGut = Actor.ClassId == 0x33 && KyaReadJamGutOffsets(
            MakeArrayView(Bytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, JamGut);
        if (HasJamGut != Actor.bHasJamGutData || (HasJamGut &&
            (Actor.JamGutData.bHasField41C != JamGut.bHasField41C || Actor.JamGutData.Checkpoints.Num() != JamGut.CheckpointCount ||
                Actor.JamGutData.Paths.Num() != JamGut.PathCount))) return false;
        const int64 RawOffset = HasMiniGame ? MiniGame.Tail : HasBrazul ? Brazul.Tail : HasBunch ? Bunch.Tail : HasShip ? Ship.Tail : HasPattern ? Pattern.Tail : HasNoseMonster ? NoseMonster.Tail : HasPunchingBall ? PunchingBall.Tail : HasNativShop ? NativShop.Tail : HasShocker ? Shocker.Tail : HasExplosiveDistributor ? ExplosiveDistributor.Tail : HasEventGenerator ? EventGenerator.Tail : HasJamGut ? JamGut.Tail : HasWantedZoo ? WantedZoo.Tail : HasCredits ? Credits.Tail : HasMiniGamesSupport ? MiniGamesSupport.Tail : HasWoof ? Woof.Tail : HasBlazer ? Blazer.Tail : HasBombLauncher ? BombLauncher.Tail : HasElectrolla ? Electrolla.Tail : (Actor.ClassId == 0x37 && Actor.bHasCareBoyData) ? Offsets.ClassData + 12 : HasEgg ? Egg.Tail : HasPatternCmd ? PatternCmd.Tail : HasAmortosCmd ? AmortosCmd.Tail : HasAddOnPurchase ? AddOnPurchase.Tail : (Actor.ClassId == 0x3a && Actor.bHasAcceleratosData) ? Offsets.ClassData + 20 : HasShockerCmd ? ShockerCmd.Tail : (Actor.ClassId == 0x27 && Actor.bHasWoodMonsterData) ? Offsets.ClassData + 16 : (Actor.ClassId == 0x2b && Actor.bHasHelperSignData) ? Offsets.ClassData + 12 : (Actor.ClassId == 0x48 && Actor.bHasHunterData) ? Offsets.ClassData + 40 : HasAton ? Aton.Tail : (HasBridge ? Bridge.Tail : (HasCompanion ? Companion.Tail : (HasGravity ? Gravity.Tail : (HasNativCmd ? Command.Tail : (HasCheckpoints ? CheckpointData.Tail : (HasRope ? Rope.Tail : (HasFountain ? Fountain.Tail : (HasRune ? Rune.Tail : (HasStiller ? Stiller.Tail : (HasAmbre ? Ambre.Tail : (HasShoot ? Shoot.Tail : (HasFog ? Fog.Tail : (HasMoney ? Money.Tail : (HasBoxClass ? BoxClass.Tail : (HasDca ? Dca.Tail : (HasTeleporter ? Teleporter.Tail : (HasAmortos ? Amortos.Tail : (HasNativ ? Nativ.Tail : (HasCommander ? Commander.Tail : (HasFruit ? Fruit.Tail : (HasWolfen ? Wolfen.WeaponIndex : (HasMicken ? Micken.Tail : (HasBonus ? Bonus.Tail : (HasSwitch ? SwitchData.Tail : (HasWeapon ? Weapon.Tail : (HasTrap ? Trap.Tail : (HasWind ? Wind.Tail : (HasProjectile ? Projectile.Tail : (HasHedgehog ? Hedgehog.Tail : (HasAraignos ? Araignos.Tail : (HasFx ? Fx.Tail : (HasBox ? Box.Tail : (HasDetails ? Details.Tail : (HasPlatform ? Platform.Suffix : Offsets.ClassData))))))))))))))))))))))))))))))))));
        const int64 SecondPassRawOffset = HasKim ? Kim.Tail : (HasBoomy ? Boomy.Tail : RawOffset);
        if (Actor.bHasHunterData != (Actor.ClassId == 0x48 && Offsets.ClassDataSize >= 40)) return false;
        if (Actor.bHasHelperSignData != (Actor.ClassId == 0x2b && Offsets.ClassDataSize >= 12)) return false;
        if (!Actor.ClassData.IsEmpty()) FMemory::Memcpy(Bytes.GetData() + SecondPassRawOffset, Actor.ClassData.GetData(), Actor.ClassData.Num());
        if (HasPlatform)
        {
            if (!KyaWriteMovingPlatformActor(Actor, Bytes, Offsets.ClassData, Offsets.ClassDataSize)) return false;
        }
        if (HasHedgehog)
        {
            if (!KyaWriteHedgehogActor(Actor, Bytes, Offsets.ClassData, Offsets.ClassDataSize)) return false;
        }
        if (HasBox)
        {
            if (!KyaWriteBasicBoxActor(Actor, Bytes, Offsets.ClassData, Offsets.ClassDataSize)) return false;
        }
        if (HasEventGenerator)
        {
            if (!KyaWriteEventGeneratorActor(Actor, Bytes, Offsets.ClassData, Offsets.ClassDataSize)) return false;
        }
        if (HasExplosiveDistributor)
        {
            if (!KyaWriteExplosiveDistributorActor(Actor, Bytes, Offsets.ClassData, Offsets.ClassDataSize)) return false;
        }
        if (HasShocker)
        {
            if (!KyaWriteShockerActor(Actor, Bytes, Offsets.ClassData, Offsets.ClassDataSize)) return false;
        }
        if (HasNativShop)
        {
            if (!KyaWriteNativShopActor(Actor, Bytes, Offsets.ClassData, Offsets.ClassDataSize)) return false;
        }
        if (HasPunchingBall)
        {
            if (!KyaWritePunchingBallActor(Actor, Bytes, Offsets.ClassData, Offsets.ClassDataSize)) return false;
        }
        if (HasNoseMonster)
        {
            if (!KyaWriteNoseMonsterActor(Actor, Bytes, Offsets.ClassData, Offsets.ClassDataSize)) return false;
        }
        if (HasPattern)
        {
            if (!KyaWritePatternActor(Actor, Bytes, Offsets.ClassData, Offsets.ClassDataSize)) return false;
        }
        if (HasShip)
        {
            if (!KyaWriteShipActor(Actor, Bytes, Offsets.ClassData, Offsets.ClassDataSize)) return false;
        }
        if (HasBunch)
        {
            if (!KyaWriteBunchActor(Actor, Bytes, Offsets.ClassData, Offsets.ClassDataSize)) return false;
        }
        if (HasBrazul)
        {
            if (!KyaWriteBrazulActor(Actor, Bytes, Offsets.ClassData, Offsets.ClassDataSize)) return false;
        }
        if (HasMiniGame)
        {
            if (!KyaWriteMiniGameActor(Actor, Bytes, Offsets.ClassData, Offsets.ClassDataSize)) return false;
        }
        if (HasWolfen)
        {
            FKyaFighterPrefixOffsets Prefix;
            Prefix.Weight = Wolfen.FighterPrefix;
            Prefix.Autonomous = Wolfen.FighterPrefix + 4;
            Prefix.Fixed = Wolfen.FighterFixed;
            Prefix.Blows = Wolfen.Blows;
            Prefix.Grabs = Wolfen.Grabs;
            Prefix.GrabReacts = Wolfen.GrabReacts;
            Prefix.Combos = Wolfen.Combos;
            Prefix.WeaponIndex = Wolfen.WeaponIndex;
            Prefix.Tail = Wolfen.WeaponIndex + 4;
            if (!WriteFighterPrefix(Actor, Prefix)) return false;
            if (!KyaWriteWolfenActor(Actor, Bytes, Offsets.ClassData, Offsets.ClassDataSize)) return false;
        }
        if (HasNativ)
        {
            if (!KyaWriteNativActor(Actor, Bytes, Offsets.ClassData, Offsets.ClassDataSize)) return false;
        }
        if (HasAmortos)
        {
            if (!KyaWriteAmortosActor(Actor, Bytes, Offsets.ClassData, Offsets.ClassDataSize)) return false;
        }
        if (HasTeleporter)
        {
            if (!KyaWriteTeleporterActor(Actor, Bytes, Offsets.ClassData, Offsets.ClassDataSize)) return false;
        }
        if (HasDca)
        {
            if (!KyaWriteDcaActor(Actor, Bytes, Offsets.ClassData, Offsets.ClassDataSize)) return false;
        }
        if (HasBoxClass)
        {
            if (!KyaWriteBoxActor(Actor, Bytes, Offsets.ClassData, Offsets.ClassDataSize)) return false;
        }
        if (HasMoney)
        {
            if (!KyaWriteMoneyActor(Actor, Bytes, Offsets.ClassData, Offsets.ClassDataSize)) return false;
        }
        if (HasFog)
        {
            if (!KyaWriteFogActor(Actor, Bytes, Offsets.ClassData, Offsets.ClassDataSize)) return false;
        }
        if (HasShoot)
        {
            if (!KyaWriteShootActor(Actor, Bytes, Offsets.ClassData, Offsets.ClassDataSize)) return false;
        }
        if (HasAmbre)
        {
            if (!KyaWriteAmbreActor(Actor, Bytes, Offsets.ClassData, Offsets.ClassDataSize)) return false;
        }
        if (HasStiller)
        {
            if (!KyaWriteStillerActor(Actor, Bytes, Offsets.ClassData, Offsets.ClassDataSize)) return false;
        }
        if (Actor.ClassId == 0x48 && Actor.bHasHunterData)
        {
            if (!KyaWriteHunterActor(Actor, Bytes, Offsets.ClassData, Offsets.ClassDataSize)) return false;
        }
        if (Actor.ClassId == 0x2b && Actor.bHasHelperSignData)
        {
            if (!KyaWriteHelperSignActor(Actor, Bytes, Offsets.ClassData, Offsets.ClassDataSize)) return false;
        }
        if (Actor.ClassId == 0x27 && Actor.bHasWoodMonsterData)
        {
            if (!KyaWriteWoodMonsterActor(Actor, Bytes, Offsets.ClassData, Offsets.ClassDataSize)) return false;
        }
        if (Actor.ClassId == 0x50 && Actor.bHasShockerCmdData)
        {
            if (!KyaWriteShockerCmdActor(Actor, Bytes, Offsets.ClassData, Offsets.ClassDataSize)) return false;
        }
        if (Actor.ClassId == 0x3a && Actor.bHasAcceleratosData)
        {
            if (!KyaWriteAcceleratosActor(Actor, Bytes, Offsets.ClassData, Offsets.ClassDataSize)) return false;
        }
        if (Actor.ClassId == 0x49 && Actor.bHasAddOnPurchaseData)
        {
            if (!KyaWriteAddOnPurchaseActor(Actor, Bytes, Offsets.ClassData, Offsets.ClassDataSize)) return false;
        }
        if (Actor.ClassId == 0x31 && Actor.bHasAmortosCmdData)
        {
            if (!KyaWriteAmortosCmdActor(Actor, Bytes, Offsets.ClassData, Offsets.ClassDataSize)) return false;
        }
        if (Actor.ClassId == 0x40 && Actor.bHasPatternCmdData)
        {
            if (!KyaWritePatternCmdActor(Actor, Bytes, Offsets.ClassData, Offsets.ClassDataSize)) return false;
        }
        if (Actor.ClassId == 0x2f && Actor.bHasEggData)
        {
            if (!KyaWriteEggActor(Actor, Bytes, Offsets.ClassData, Offsets.ClassDataSize)) return false;
        }
        if (Actor.ClassId == 0x37 && Actor.bHasCareBoyData)
        {
            if (!KyaWriteCareBoyActor(Actor, Bytes, Offsets.ClassData, Offsets.ClassDataSize)) return false;
        }
        if (Actor.ClassId == 0x0b && Actor.bHasElectrollaData)
        {
            if (!KyaWriteElectrollaActor(Actor, Bytes, Offsets.ClassData, Offsets.ClassDataSize)) return false;
        }
        if (Actor.ClassId == 0x53 && Actor.bHasBombLauncherData)
        {
            if (!KyaWriteBombLauncherActor(Actor, Bytes, Offsets.ClassData, Offsets.ClassDataSize)) return false;
        }
        if (Actor.ClassId == 0x4d && Actor.bHasBlazerData)
        {
            if (!KyaWriteBlazerActor(Actor, Bytes, Offsets.ClassData, Offsets.ClassDataSize)) return false;
        }
        if (Actor.ClassId == 0x2a && Actor.bHasWoofData)
        {
            if (!KyaWriteWoofActor(Actor, Bytes, Offsets.ClassData, Offsets.ClassDataSize)) return false;
        }
        if (Actor.bHasMiniGamesManagerData || Actor.bHasMiniGamesOrganizerData)
        {
            if (!KyaWriteMiniGamesSupportActor(Actor, Bytes, Offsets.ClassData, Offsets.ClassDataSize)) return false;
        }
        if (Actor.bHasCreditsData)
        {
            if (!KyaWriteCreditsActor(Actor, Bytes, Offsets.ClassData, Offsets.ClassDataSize)) return false;
        }
        if (Actor.bHasWantedZooData)
        {
            if (!KyaWriteWantedZooActor(Actor, Bytes, Offsets.ClassData, Offsets.ClassDataSize)) return false;
        }
        if (Actor.bHasJamGutData)
        {
            if (!KyaWriteJamGutActor(Actor, Bytes, Offsets.ClassData, Offsets.ClassDataSize)) return false;
        }
        if (HasRope)
        {
            if (!KyaWriteRopeActor(Actor, Bytes, Offsets.ClassData, Offsets.ClassDataSize)) return false;
        }
        if (HasFountain)
        {
            if (!KyaWriteFountainActor(Actor, Bytes, Offsets.ClassData, Offsets.ClassDataSize)) return false;
        }
        if (HasRune)
        {
            if (!KyaWriteRuneActor(Actor, Bytes, Offsets.ClassData, Offsets.ClassDataSize)) return false;
        }
        if (HasGravity)
        {
            if (!KyaWriteGravityActor(Actor, Bytes, Offsets.ClassData, Offsets.ClassDataSize)) return false;
        }
        if (HasNativCmd)
        {
            if (!KyaWriteNativCmdActor(Actor, Bytes, Offsets.ClassData, Offsets.ClassDataSize)) return false;
        }
        if (HasBridge)
        {
            if (!KyaWriteBridgeActor(Actor, Bytes, Offsets.ClassData, Offsets.ClassDataSize)) return false;
        }
        if (HasCompanion)
        {
            if (!KyaWriteCompanionActor(Actor, Bytes, Offsets.ClassData, Offsets.ClassDataSize)) return false;
        }
        if (HasAton)
        {
            if (!KyaWriteAtonActor(Actor, Bytes, Offsets.ClassData, Offsets.ClassDataSize)) return false;
        }
        if (HasBoomy)
        {
            if (!KyaWriteBoomyActor(Actor, Bytes, Offsets.ClassData, Offsets.ClassDataSize)) return false;
        }
        if (HasKim)
        {
            if (!WriteFighterPrefix(Actor, Kim.Fighter)) return false;
            if (!KyaWriteKimActor(Actor, Bytes, Offsets.ClassData, Offsets.ClassDataSize)) return false;
        }
        if (HasCommander)
        {
            if (!KyaWriteCommanderActor(Actor, Bytes, Offsets.ClassData, Offsets.ClassDataSize)) return false;
        }
        if (HasFruit)
        {
            if (!KyaWriteFruitActor(Actor, Bytes, Offsets.ClassData, Offsets.ClassDataSize)) return false;
        }
        if (HasMicken)
        {
            if (!KyaWriteMickenActor(Actor, Bytes, Offsets.ClassData, Offsets.ClassDataSize)) return false;
        }
        if (HasBonus)
        {
            if (!KyaWriteBonusActor(Actor, Bytes, Offsets.ClassData, Offsets.ClassDataSize)) return false;
        }
        if (HasSwitch)
        {
            if (!KyaWriteSwitchActor(Actor, Bytes, Offsets.ClassData, Offsets.ClassDataSize)) return false;
        }
        if (HasWeapon)
        {
            if (!KyaWriteWeaponActor(Actor, Bytes, Offsets.ClassData, Offsets.ClassDataSize)) return false;
        }
        if (HasTrap)
        {
            if (!KyaWriteTrapActor(Actor, Bytes, Offsets.ClassData, Offsets.ClassDataSize)) return false;
        }
        if (HasWind)
        {
            if (!KyaWriteWindActor(Actor, Bytes, Offsets.ClassData, Offsets.ClassDataSize)) return false;
        }
        if (HasProjectile)
        {
            if (!KyaWriteProjectileActor(Actor, Bytes, Offsets.ClassData, Offsets.ClassDataSize)) return false;
        }
        if (HasAraignos)
        {
            if (!KyaWriteAraignosActor(Actor, Bytes, Offsets.ClassData, Offsets.ClassDataSize)) return false;
        }
        if (HasFx)
        {
            if (!KyaWriteFxActor(Actor, Bytes, Offsets.ClassData, Offsets.ClassDataSize)) return false;
        }
        if (Actor.bHasTypedCheckpoints)
        {
            if (!KyaWriteCheckpointsActor(Actor, Bytes, Offsets.ClassData, Offsets.ClassDataSize)) return false;
        }
        for (int32 Animation = 0; Animation < Actor.MacroAnimations.Num(); ++Animation)
        {
            WriteU32(Offsets.MacroAnimations + Animation * 8, uint32(Actor.MacroAnimations[Animation].AnimationType));
            WriteU32(Offsets.MacroAnimations + Animation * 8 + 4, uint32(Actor.MacroAnimations[Animation].MacroAnimationId));
        }
        for (int32 Axis = 0; Axis < 3; ++Axis)
        {
            WriteFloat(Offsets.Placement + 12 + Axis * 4, float(Actor.Position[Axis]));
            WriteFloat(Offsets.Placement + 24 + Axis * 4, float(Actor.Rotation[Axis]));
            WriteFloat(Offsets.Placement + 36 + Axis * 4, float(Actor.Scale[Axis]));
        }
    }
    for (int32 Index = 0; Index < Paths.Num(); ++Index)
    {
        const FKyaPath& Path = Environment.Paths[Index];
        const FKyaPathOffsets& Offsets = Paths[Index];
        for (int32 Array = 0; Array < 7; ++Array)
            if (Offsets.Arrays[Array] != INDEX_NONE)
                for (uint32 Point = 0; Point < Offsets.Count; ++Point)
                {
                    const int64 Offset = Offsets.Arrays[Array] + Point * (Array < 3 ? 16 : 4);
                    if (Array < 3)
                    {
                        const FVector4& Value = Array == 0 ? Path.Points[Point] : (Array == 1 ? Path.Rotations[Point] : Path.ExtraVectors[Point]);
                        for (int32 Axis = 0; Axis < 4; ++Axis) WriteFloat(Offset + Axis * 4, float(Value[Axis]));
                    }
                    else if (Array == 3) WriteFloat(Offset, Path.Delays[Point]);
                    else if (Array == 4) WriteU32(Offset, uint32(Path.PointFlags[Point]));
                    else if (Array == 5) WriteFloat(Offset, Path.ExtraValues[Point]);
                    else WriteU32(Offset, uint32(Path.ExtraWords[Point]));
                }
        WriteU32(Offsets.Trailer, uint32(Path.Mode));
        WriteU32(Offsets.Trailer + 4, uint32(Path.Type));
        WriteU32(Offsets.Trailer + 8, uint32(Path.PathType));
    }
    for (int32 Index = 0; Index < WaypointCount; ++Index)
    {
        const FKyaWaypoint& Waypoint = Environment.Waypoints[Index];
        const int64 Offset = WaypointRecords + Index * 36;
        for (int32 Axis = 0; Axis < 3; ++Axis)
        {
            WriteFloat(Offset + Axis * 4, float(Waypoint.Position[Axis]));
            WriteFloat(Offset + 12 + Axis * 4, float(Waypoint.Rotation[Axis]));
            WriteFloat(Offset + 24 + Axis * 4, float(Waypoint.Parameters[Axis]));
        }
    }
    WriteFloat(Setup + 48, Environment.CameraFov);
    for (int32 Axis = 0; Axis < 3; ++Axis)
    {
        WriteFloat(Setup + 52 + Axis * 4, float(Environment.CameraPosition[Axis]));
        WriteFloat(Setup + 64 + Axis * 4, float(Environment.CameraLookAt[Axis]));
        WriteFloat(Setup + 76 + Axis * 4, float(Environment.CameraUp[Axis]));
        WriteFloat(Setup + 100 + Axis * 4, float(Environment.CameraTargetOffset[Axis]));
    }
    WriteU32(Setup + 88, uint32(Environment.CameraSectorId));
    WriteU32(Setup + 92, uint32(Environment.CameraField08));
    WriteU32(Setup + 96, uint32(Environment.CameraFlags));
    if (ReadU32(Setup + 96) & 2)
    {
        int64 Offset = Setup + 112;
        WriteU32(Offset, uint32(Environment.CameraSwitchActor)); Offset += 4;
        const TArray<int32>* Lists[] = { &Environment.CameraSwitchActorsA, &Environment.CameraSwitchActorsB };
        for (const TArray<int32>* List : Lists)
        {
            Offset += 4;
            for (int32 ActorRef : *List) { WriteU32(Offset, uint32(ActorRef)); Offset += 4; }
        }
        WriteFloat(Offset, Environment.CameraSwitchValueA);
        WriteU32(Offset + 4, uint32(Environment.CameraSwitchModeA));
        WriteFloat(Offset + 8, Environment.CameraSwitchValueB);
        WriteU32(Offset + 12, uint32(Environment.CameraSwitchModeB));
        WriteFloat(Offset + 16, Environment.CameraSwitchValueC);
    }
    for (int32 Index = 0; Index < Environment.CameraConditions.Num(); ++Index)
    {
        const FKyaCameraCondition& Condition = Environment.CameraConditions[Index];
        const int64 Offset = CameraConditions + 4 + int64(Index) * 16;
        WriteU32(Offset, uint32(Condition.Combiner));
        WriteU32(Offset + 4, uint32(Condition.ActorRef));
        WriteU32(Offset + 8, uint32(Condition.Type));
        WriteFloat(Offset + 12, Condition.Value);
    }
    WriteU32(Config, uint32(Environment.CameraConfigFlags));
    WriteU32(Config + 4, uint32(Environment.CameraActorRefA));
    WriteU32(Config + 8, uint32(Environment.CameraActorRefB));
    WriteFloat(Config + 12, Environment.CameraConfigAngle);
    for (int32 Index = 0; Index < OptionalCount; ++Index)
        WriteFloat(Config + 20 + Index * 4, Environment.CameraOptionalConfigValues[Index]);
    for (int32 Index = 0; Index < ExtraCount; ++Index)
        WriteFloat(ConfigValues + 40 + Index * 4, Environment.CameraExtraValues[Index]);
    WriteFloat(SceneValues, Environment.Field118);
    WriteU32(SceneValues + 4, uint32(Environment.Field11C));
    WriteU32(SceneValues + 8, uint32(Environment.Count120));
    WriteU32(Map, uint32(Environment.MapField04));
    WriteU32(Map + 4, uint32(Environment.MapField08));
    WriteFloat(Map + 8, Environment.MapAngle);
    for (int32 Index = 0; Index < Environment.MapEntries.Num(); ++Index)
        WriteU32(Map + 16 + Index * 4, uint32(Environment.MapEntries[Index]));
    WriteU32(MapEnd, uint32(Environment.MapField14));
    WriteU32(MapEnd + 4, uint32(Environment.MapActorRef));
    WriteU32(MapEnd + 8, uint32(Environment.DefaultTextureIndex));
    WriteU32(MapEnd + 12, uint32(Environment.AlternateTextureIndex));
    WriteU32(MapEnd + 16, uint32(Environment.DefaultMaterialIndex));
    for (int32 Category = 0; Category < 7; ++Category)
        WriteU32(MapEnd + 20 + Category * 4, uint32(Environment.EffectPoolCounts[Category]));
    for (int32 Block = 0; Block < ScenarioBlocks.Num(); ++Block)
    {
        const FKyaConditionedOperation& Source = Environment.InitialScenarioOperations[Block];
        for (int32 Index = 0; Index < Source.Operations.Num(); ++Index)
        {
            const int64 Offset = ScenarioBlocks[Block].Operations[Index];
            WriteU32(Offset, uint32(Source.Operations[Index].Variable));
            WriteU32(Offset + 4, uint32(Source.Operations[Index].Type));
            WriteU32(Offset + 8, uint32(Source.Operations[Index].Value));
        }
        for (int32 Index = 0; Index < Source.Conditions.Num(); ++Index)
        {
            const int64 Offset = ScenarioBlocks[Block].Conditions[Index];
            WriteU32(Offset, uint32(Source.Conditions[Index].Combiner));
            WriteU32(Offset + 4, uint32(Source.Conditions[Index].Variable));
            WriteU32(Offset + 8, uint32(Source.Conditions[Index].Comparison));
            WriteU32(Offset + 12, uint32(Source.Conditions[Index].Value));
        }
    }
    for (int32 Index = 0; Index < Objectives.Num(); ++Index)
    {
        const FKyaObjective& Objective = Environment.Objectives[Index];
        const FKyaObjectiveOffsets& Offsets = Objectives[Index];
        WriteU32(Offsets.Header, uint32(Objective.ScenarioVariable));
        for (int32 Field = 0; Field < 6; ++Field)
            WriteU32(Offsets.Header + 4 + Field * 4, uint32(Objective.Fields[Field]));
        for (int32 TargetIndex = 0; TargetIndex < Objective.Targets.Num(); ++TargetIndex)
        {
            const FKyaObjectiveTarget& Target = Objective.Targets[TargetIndex];
            const int64 Offset = Offsets.Targets[TargetIndex];
            WriteU32(Offset, uint32(Target.MessageKeyA));
            WriteU32(Offset + 4, uint32(Target.MessageKeyB));
            WriteU32(Offset + 8, uint32(Target.ActorRef));
            WriteU32(Offset + 12, uint32(Target.ZoneRef));
        }
    }
    for (int32 Axis = 0; Axis < 3; ++Axis)
    {
        WriteFloat(ConfigValues + Axis * 4, float(Environment.CameraPitchAngles[Axis]));
        WriteFloat(ConfigValues + 12 + Axis * 4, float(Environment.CameraOtherAngles[Axis]));
        WriteFloat(ConfigValues + 24 + Axis * 4, float(Environment.CameraDistances[Axis]));
    }
    WriteU32(TextureCountPos, uint32(Bank->SceneConfiguration.CommonTextureCount));
    WriteU32(Scene->Offset + 28, uint32(Bank->SceneConfiguration.HeaderWords[0]));
    WriteU32(Scene->Offset + 32, uint32(Bank->SceneConfiguration.HeaderWords[1]));
    for (int32 Index = 0; Index < Bank->SceneConfiguration.CommonMeshValues.Num(); ++Index)
        WriteU32(MeshDataPos + Index * 4, uint32(Bank->SceneConfiguration.CommonMeshValues[Index]));
    WriteU32(TextureCountPos + 4, uint32(Bank->SceneConfiguration.TextureWord));
    for (uint32 Index = 0; Index < CollisionCount; ++Index)
        WriteU32(Pos + Index * 4, uint32(Bank->SceneConfiguration.CollisionBankValues[Index]));
    WriteU32(AnimationCountPos, uint32(Bank->SceneConfiguration.LevelAnimationCount));
    for (int32 Index = 0; Index < 4; ++Index)
        WriteU32(AnimationCountPos + 4 + Index * 4, uint32(Bank->SceneConfiguration.PostAnimationWords[Index]));
    for (uint32 Index = 0; Index < ClassCount; ++Index)
    {
        const FKyaActorClassAllocation& Allocation = Bank->SceneConfiguration.ActorClassAllocations[Index];
        WriteU32(ClassDataPos + Index * 8, uint32(Allocation.ClassId));
        WriteU32(ClassDataPos + Index * 8 + 4, uint32(Allocation.Count));
    }
    WriteU32(ClassDataPos + int64(ClassCount) * 8, uint32(Bank->SceneConfiguration.ParticleWord));

    if (!KyaPatchBankLights(*Bank, Bytes, Diagnostics)) return false;

    const bool bHasBaseline = HasKyaImportedBaseline(*Bank);
    int32 ChangedActors = 0;
    if (bHasBaseline)
        for (int32 ActorIndex = 0; ActorIndex < Bank->SceneEnvironment.Actors.Num(); ++ActorIndex)
            if (KyaActorDiffersFromImported(*Bank, ActorIndex)) ++ChangedActors;
    Diagnostics.SupportedEdits.Add(FString::Printf(
        TEXT("%d actor, %d waypoint, %d path and %d light record(s) patched in place; %s. Structural changes "
            "other than actor condition counts (counts, names, arrays, classes) are rejected before writing."),
        Bank->SceneEnvironment.Actors.Num(), Bank->SceneEnvironment.Waypoints.Num(),
        Bank->SceneEnvironment.Paths.Num(), Bank->Lights.Num(),
        bHasBaseline ? *FString::Printf(TEXT("%d actor record(s) differ from the imported baseline"), ChangedActors)
                     : TEXT("change count unavailable: no imported baseline")));

    FString GeometryError;
    if (!KyaGeometry::ValidateAppliedGeometry(*Bank, Bytes, GeometryError)) { Diagnostics.Errors.Add(GeometryError); return false; }
    OutBytes = MoveTemp(Bytes);
    return true;
}

bool KyaPatchBankLights(const UKyaLevelBank& Bank, TArray<uint8>& Bytes,
    FKyaBankExportDiagnostics& Diagnostics)
{
    // Light archive (THGL): patch the fixed records in place using the serialized
    // offset table, preserving any bytes the typed schema does not expose.
    auto WriteLightU32 = [&Bytes](int64 Offset, uint32 Value)
    {
        Bytes[Offset] = uint8(Value); Bytes[Offset + 1] = uint8(Value >> 8);
        Bytes[Offset + 2] = uint8(Value >> 16); Bytes[Offset + 3] = uint8(Value >> 24);
    };
    auto WriteLightFloat = [&Bytes](int64 Offset, float Value)
    {
        uint32 Bits; FMemory::Memcpy(&Bits, &Value, sizeof(Bits));
        Bytes[Offset] = uint8(Bits); Bytes[Offset + 1] = uint8(Bits >> 8);
        Bytes[Offset + 2] = uint8(Bits >> 16); Bytes[Offset + 3] = uint8(Bits >> 24);
    };
    auto ReadLightFloat = [&Bytes](int64 Offset)
    {
        const uint32 Bits = uint32(Bytes[Offset]) | (uint32(Bytes[Offset + 1]) << 8) |
            (uint32(Bytes[Offset + 2]) << 16) | (uint32(Bytes[Offset + 3]) << 24);
        float Value; FMemory::Memcpy(&Value, &Bits, sizeof(Value));
        return Value;
    };
    const auto ReadColorChannel = [&Bytes](int64 Offset, float Multiplier)
    {
        const float Value = FMath::Min(Bytes[Offset], uint8(127)) * 2.0f * Multiplier / 255.0f;
        return FMath::Clamp(Value, 0.0f, 1.0f);
    };
    const auto WriteColorChannel = [&Bytes](int64 Offset, float Channel, float Multiplier)
    {
        const float Raw = Multiplier > KINDA_SMALL_NUMBER
            ? Channel * 255.0f / (2.0f * Multiplier) : 0.0f;
        // One byte per channel; adjacent channels must keep their original bytes.
        Bytes[Offset] = uint8(FMath::Clamp(FMath::RoundToInt(Raw), 0, 127));
    };
    const auto ReadU32 = [&Bytes](int64 Offset) -> uint32
    {
        const uint8* P = Bytes.GetData() + Offset;
        return uint32(P[0]) | (uint32(P[1]) << 8) | (uint32(P[2]) << 16) | (uint32(P[3]) << 24);
    };
    bool bFoundArchive = false;
    for (const FKyaBankEntry& LightEntry : Bank.Entries)
    {
        if (LightEntry.Type != 16 || LightEntry.Subtype != 1) continue;
        if (LightEntry.Offset < 0 || LightEntry.Size < 32 || LightEntry.Offset > Bytes.Num() ||
            LightEntry.Size > Bytes.Num() - LightEntry.Offset)
        {
            Diagnostics.Errors.Add(TEXT("The light archive lies outside the bank bytes or is truncated."));
            return false;
        }
        if (ReadU32(LightEntry.Offset) != 0xDEADBEEF ||
            FMemory::Memcmp(Bytes.GetData() + LightEntry.Offset + 8, "THGL", 4) != 0)
        {
            Diagnostics.Errors.Add(TEXT("The light archive has an invalid THGL header."));
            return false;
        }
        if (bFoundArchive)
        {
            Diagnostics.Errors.Add(TEXT("Multiple light archives cannot be exported."));
            return false;
        }
        bFoundArchive = true;
        TArray<FKyaLightOffsets> Offsets;
        if (!KyaReadLightOffsets(MakeArrayView(Bytes.GetData(), Bytes.Num()), LightEntry.Offset + 16,
            LightEntry.Size - 16, Offsets))
        {
            Diagnostics.Errors.Add(TEXT("The light archive records are malformed or truncated."));
            return false;
        }
        if (Offsets.Num() != Bank.Lights.Num())
        {
            Diagnostics.Errors.Add(FString::Printf(
                TEXT("The light archive has %d records but the asset has %d; changing the light count is not supported."),
                Offsets.Num(), Bank.Lights.Num()));
            return false;
        }
        for (int32 Index = 0; Index < Offsets.Num(); ++Index)
        {
            const FKyaLight& Light = Bank.Lights[Index];
            const FKyaLightOffsets& Off = Offsets[Index];
            if (Light.TrailingValues.Num() != Off.TrailingCount)
            {
                Diagnostics.Errors.Add(FString::Printf(TEXT("Light %d trailing value count cannot be changed."), Index));
                return false;
            }
            // Direction/FOV presence is part of the serialized layout; a type/shape
            // change that alters it would make existing bytes parse differently.
            const auto HasDirection = [](int32 Type, int32 Shape)
            {
                return Type != 2 && (Shape == 1 || Shape == 2 || Shape == 4);
            };
            const auto HasFov = [](int32 Shape) { return Shape == 4; };
            if (HasDirection(Off.LightType, Off.ShapeType) != HasDirection(Light.LightType, Light.ShapeType) ||
                HasFov(Off.ShapeType) != HasFov(Light.ShapeType) ||
                (HasDirection(Light.LightType, Light.ShapeType) && Off.Direction == 0) ||
                (HasFov(Light.ShapeType) && Off.Fov == 0))
            {
                Diagnostics.Errors.Add(FString::Printf(
                    TEXT("Light %d changes its serialized layout (type/shape controls direction/FOV presence); "
                        "rebuilding the light archive is not supported."), Index));
                return false;
            }
            WriteLightU32(Off.Record, uint32(Light.LightType));
            WriteLightU32(Off.Record + 4, uint32(Light.ShapeType));
            const float OriginalMultiplier = ReadLightFloat(Off.Multiplier);
            const FLinearColor Colors[] = {Light.AmbientColor, Light.Color, Light.Field20Color};
            const int64 ColorOffsets[] = {Off.Ambient, Off.Color, Off.Field20};
            for (int32 ColorIndex = 0; ColorIndex < 3; ++ColorIndex)
            {
                const FLinearColor& Value = Colors[ColorIndex];
                // Only touched channels are re-encoded; untouched saturated channels
                // keep their original bytes so unrelated edits stay byte-exact.
                const float Channels[] = {Value.R, Value.G, Value.B, Value.A};
                for (int32 Channel = 0; Channel < 4; ++Channel)
                {
                    if (FMath::IsNearlyEqual(Channels[Channel],
                        ReadColorChannel(ColorOffsets[ColorIndex] + Channel, OriginalMultiplier), 1.0e-4f))
                        continue;
                    WriteColorChannel(ColorOffsets[ColorIndex] + Channel, Channels[Channel], Light.Multiplier);
                }
            }
            if (Off.Position != 0 && Off.Falloff != 0)
            {
                WriteLightFloat(Off.Position, Light.Position.X);
                WriteLightFloat(Off.Position + 4, Light.Position.Y);
                WriteLightFloat(Off.Position + 8, Light.Position.Z);
                WriteLightFloat(Off.Falloff, Light.Falloff0);
                WriteLightFloat(Off.Falloff + 4, Light.Falloff1);
            }
            if (Off.Direction != 0)
            {
                WriteLightFloat(Off.Direction, Light.Direction.X);
                WriteLightFloat(Off.Direction + 4, Light.Direction.Y);
                WriteLightFloat(Off.Direction + 8, Light.Direction.Z);
            }
            if (Off.Fov != 0)
            {
                WriteLightFloat(Off.Fov, Light.Fov0);
                WriteLightFloat(Off.Fov + 4, Light.Fov1);
            }
            for (int32 Trailing = 0; Trailing < Off.TrailingCount && Light.TrailingValues.IsValidIndex(Trailing); ++Trailing)
                WriteLightU32(Off.Trailing + Trailing * 4, uint32(Light.TrailingValues[Trailing]));
        }
        // Scalar header fields are rewritten last so an overlapping trailing/color
        // write in a malformed record cannot corrupt the multiplier or type words.
        for (int32 Index = 0; Index < Offsets.Num(); ++Index)
        {
            const FKyaLight& Light = Bank.Lights[Index];
            const FKyaLightOffsets& Off = Offsets[Index];
            WriteLightU32(Off.Record, uint32(Light.LightType));
            WriteLightU32(Off.Record + 4, uint32(Light.ShapeType));
            WriteLightFloat(Off.Multiplier, Light.Multiplier);
        }
    }

    if (!bFoundArchive && !Bank.Lights.IsEmpty())
    {
        Diagnostics.Errors.Add(TEXT("The bank has no light archive to store its light records."));
        return false;
    }
    return true;
}

bool UKyaLevelBankExporter::ExportBinary(UObject* Object, const TCHAR* Type, FArchive& Ar,
    FFeedbackContext* Warn, int32 FileIndex, uint32 PortFlags)
{
    const UKyaLevelBank* Bank = CastChecked<UKyaLevelBank>(Object);
    TArray<uint8> Bytes;
    FKyaBankExportDiagnostics Diagnostics;
    if (!KyaBuildBankExportBytes(Bank, Bytes, Diagnostics))
    {
        if (Warn)
            for (const FString& Error : Diagnostics.Errors) Warn->Logf(ELogVerbosity::Error, TEXT("%s"), *Error);
        return false;
    }
    Ar.Serialize(Bytes.GetData(), Bytes.Num());
    return !Ar.IsError();
}
