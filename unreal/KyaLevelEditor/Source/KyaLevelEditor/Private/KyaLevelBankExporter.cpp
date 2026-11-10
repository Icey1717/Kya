#include "KyaLevelBankExporter.h"
#include "KyaLevelBank.h"
#include "KyaCameraStream.h"
#include "Misc/FeedbackContext.h"

UKyaLevelBankExporter::UKyaLevelBankExporter()
{
    SupportedClass = UKyaLevelBank::StaticClass();
    FormatExtension.Add(TEXT("bnk"));
    FormatDescription.Add(TEXT("Kya level bank"));
    bText = false;
}

bool UKyaLevelBankExporter::ExportBinary(UObject* Object, const TCHAR* Type, FArchive& Ar,
    FFeedbackContext* Warn, int32 FileIndex, uint32 PortFlags)
{
    const UKyaLevelBank* Bank = CastChecked<UKyaLevelBank>(Object);
    if (!Bank->HasCurrentImportSchema())
    {
        Warn->Logf(ELogVerbosity::Error, TEXT("This Kya bank asset uses an older import schema. Reimport the source bank before exporting."));
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
                Warn->Logf(ELogVerbosity::Error, TEXT("Multiple scene configuration entries cannot be exported."));
                return false;
            }
            Scene = &Entry;
        }
    }
    if (!Scene || Scene->Size < 28 || Scene->Offset < 0 ||
        Scene->Offset > Bank->BankBytes.Num() || Scene->Size > Bank->BankBytes.Num() - Scene->Offset ||
        Bank->SceneConfiguration.Field20 < 0 || Bank->SceneConfiguration.Field20 > MAX_uint32 ||
        Bank->BankBytes.Num() < 16 || (Bank->BankBytes[12] & 1))
    {
        Warn->Logf(ELogVerbosity::Error, TEXT("Cannot export this bank's scene configuration."));
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
        Warn->Logf(ELogVerbosity::Error, TEXT("Changing camera flag 0x2 requires rebuilding its optional records."));
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
        Actors.Num() != Bank->SceneEnvironment.Actors.Num()) return false;
    for (int32 Index = 0; Index < Actors.Num(); ++Index)
    {
        const FKyaActorPlacement& Actor = Bank->SceneEnvironment.Actors[Index];
        const FKyaActorOffsets& Offsets = Actors[Index];
        if (Actor.Index != int32(ReadU32(Offsets.Chunk + 8)) || Actor.ClassId != ReadU32(Offsets.Chunk + 12) ||
            Actor.MacroAnimations.Num() != Offsets.MacroCount || Actor.HashCode < 0 || Actor.HashCode > MAX_uint32 ||
            Actor.LightingFlags < 0 || Actor.LightingFlags > MAX_uint32 || Actor.Behaviours.Num() != Offsets.Behaviours.Num() ||
            Actor.Conditions.Num() != Offsets.ConditionCount) return false;
        FKyaPlatformPrefixOffsets Platform;
        const bool HasPlatform = Actor.ClassId == 0x0d && KyaReadPlatformPrefix(MakeArrayView(Bytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, Platform);
        FKyaPlatformDetailsOffsets Details;
        const bool HasDetails = HasPlatform && KyaReadPlatformDetails(MakeArrayView(Bytes.GetData(), int32(SceneEnd)), Platform.Suffix, Offsets.ClassData + Offsets.ClassDataSize - Platform.Suffix, Details);
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
        FKyaWindOffsets Wind;
        const bool HasWind = Actor.ClassId == 0x12 && KyaReadWindOffsets(
            MakeArrayView(Bytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, Wind);
        FKyaTrapOffsets Trap;
        const bool HasTrap = Actor.ClassId == 0x2c && KyaReadTrapOffsets(
            MakeArrayView(Bytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, Trap);
        FKyaWeaponOffsets Weapon;
        const bool HasWeapon = Actor.ClassId == 0x20 && KyaReadWeaponOffsets(
            MakeArrayView(Bytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, Weapon);
        FKyaSwitchOffsets SwitchData;
        const bool HasSwitch = Actor.ClassId == 0x11 && KyaReadSwitchOffsets(
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
        FKyaBonusFountainOffsets Fountain;
        const bool HasFountain = Actor.ClassId == 0x52 && KyaReadBonusFountainOffsets(
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
        TArray<int64> CheckpointOffsets;
        int64 CheckpointTail = 0;
        const bool HasCheckpoints = Actor.ClassId == 0x1e && KyaReadCheckpointOffsets(
            MakeArrayView(Bytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, CheckpointOffsets, &CheckpointTail);
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
        const int64 RawOffset = HasAton ? Aton.Tail : (HasBridge ? Bridge.Tail : (HasCompanion ? Companion.Tail : (HasGravity ? Gravity.Tail : (HasNativCmd ? Command.Tail : (HasCheckpoints ? CheckpointTail : (HasRope ? Rope.Tail : (HasFountain ? Fountain.Tail : (HasRune ? Rune.Tail : (HasStiller ? Stiller.Tail : (HasAmbre ? Ambre.Tail : (HasShoot ? Shoot.Tail : (HasFog ? Fog.Tail : (HasMoney ? Money.Tail : (HasBoxClass ? BoxClass.Tail : (HasDca ? Dca.Tail : (HasTeleporter ? Teleporter.Tail : (HasAmortos ? Amortos.Tail : (HasNativ ? Nativ.Tail : (HasCommander ? Commander.Tail : (HasFruit ? Fruit.Tail : (HasWolfen ? Wolfen.WeaponIndex : (HasMicken ? Micken.Tail : (HasBonus ? Bonus.Tail : (HasSwitch ? SwitchData.Tail : (HasWeapon ? Weapon.Tail : (HasTrap ? Trap.Tail : (HasWind ? Wind.Tail : (HasProjectile ? Projectile.Tail : (HasAraignos ? Araignos.Tail : (HasFx ? Fx.Tail : (HasBox ? Box.Tail : (HasDetails ? Details.Tail : (HasPlatform ? Platform.Suffix : Offsets.ClassData)))))))))))))))))))))))))))))))));
        const int64 BoomyRawOffset = HasBoomy ? Boomy.Tail : RawOffset;
        const int64 KimRawOffset = HasKim ? Kim.Tail : RawOffset;
        if (HasPlatform != Actor.bHasPlatformPrefix || HasDetails != Actor.bHasPlatformDetails || HasBox != Actor.bHasBasicBoxData ||
            HasFx != Actor.bHasFxData || HasAraignos != Actor.bHasAraignosData || HasProjectile != Actor.bHasProjectileData ||
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
        if (HasShoot && Actor.ShootAddOn.SubObjMode != Shoot.SubObjModeValue) return false;
        if (HasAmbre && Actor.AmbreSwitch.Targets.Num() != Ambre.Switch.Count) return false;
        if (HasStiller && (Actor.StillerActorRefs.Num() != Stiller.ActorCountValue ||
            Actor.StillerSwitch208.Targets.Num() != Stiller.Switches[0].Count ||
            Actor.StillerSwitch210.Targets.Num() != Stiller.Switches[1].Count ||
            Actor.StillerAddOn.SubObjMode != Stiller.SubObjModeValue)) return false;
        if (HasFountain && Actor.BonusFountainAddOn.SubObjMode != Fountain.SubObjModeValue) return false;
        if (HasRope && Actor.RopeSwitch.Targets.Num() != Rope.Switch.Count) return false;
        if (HasCheckpoints && Actor.Checkpoints.Num() != CheckpointOffsets.Num()) return false;
        if (HasBridge && Actor.BridgeActorRefs.Num() != Bridge.ActorCountValue) return false;
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
            Actor.BoomyCamera.Conditions.Num() != Boomy.ConditionCount ||
            Actor.BoomyCamera.bHasSwitch != Boomy.bHasSwitch ||
            (Boomy.bHasSwitch && (Actor.BoomyCamera.SwitchActorsA.Num() != Boomy.SwitchCountA ||
                Actor.BoomyCamera.SwitchActorsB.Num() != Boomy.SwitchCountB)) ||
            Actor.BoomyCamera.ConfigOptionalValues.Num() != (Boomy.ConfigMode == 1 ? 5 : 0) ||
            Actor.BoomyCamera.ExtraValues.Num() != (Boomy.ExtraCount == 1 ? 12 : 0))) return false;
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
        if (HasBoxClass && (Actor.BoxActorRefs.Num() != BoxClass.ActorCountValue ||
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
                Actor.WolfenFightParamData.Knowledge.Num() != 4) return false;
            for (int32 Cap = 0; Cap < 3; ++Cap)
                if (Actor.WolfenFightParamData.Capabilities[Cap].Items.Num() != Params.Capabilities[Cap].Items.Num()) return false;
            if (Actor.WolfenFightParamData.ComboList1.Num() != int32(ReadU32(Params.ComboList1Count)) ||
                Actor.WolfenFightParamData.ComboList2.Num() != int32(ReadU32(Params.ComboList2Count))) return false;
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
                Actor.BasicBoxAddOn.SubObjMode != Box.SubObjModeValue || Actor.BasicBoxLifeMax < 0) return false;
        }
        if (HasAraignos && Actor.AraignosAddOn.SubObjMode != Araignos.SubObjModeValue) return false;
        if (HasProjectile && (Actor.ProjectileLifeMax < 0 || Actor.ProjectilePattern.bHasField24 != Projectile.bHasPatternField24)) return false;
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
            if (Source.bHasTypedFxBehaviour)
            {
                FKyaFxBehaviourValues Values;
                if (Actor.ClassId != 0x2e ||
                    !KyaReadFxBehaviour(Source.Id, MakeArrayView(Bytes.GetData(), int32(SceneEnd)), Location.Payload, Location.Size, Values) ||
                    Values.UnknownValues.Num() != Source.FxUnknownValues.Num()) return false;
            }
            if (Source.bHasTypedTrapBehaviour &&
                (Actor.ClassId != 0x2c || (Source.Id != 2 && Source.Id != 3) || Location.Size != 12)) return false;
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
                if (Source.Id == 5)
                {
                    FKyaFruitInventoryValues Values;
                    if (!KyaReadFruitInventory(MakeArrayView(Bytes.GetData(), int32(SceneEnd)), Location.Payload, Location.Size, Values)) return false;
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
                FKyaNativTakePutOffsets Off;
                if (Actor.ClassId != 0x0a || Source.Id != 6 ||
                    !KyaReadNativTakePutOffsets(MakeArrayView(Bytes.GetData(), int32(SceneEnd)), Location.Payload, Location.Payload + Location.Size, Off) ||
                    Off.Items.Num() != Source.NativTrajectories.Num()) return false;
            }
            if (Source.bHasTypedCinematicBehaviour &&
                (Source.Id != 1 || Location.Size != 4)) return false;
            if (Source.bHasTypedDcaControlledBehaviour)
            {
                FKyaDcaBehaviourOffsets Off;
                if (Actor.ClassId != 0x1b || Source.Id != 3 ||
                    !KyaReadDcaBehaviour(3, MakeArrayView(Bytes.GetData(), int32(SceneEnd)), Location.Payload, Location.Size, Off) || !Off.bControlled) return false;
            }
            if (Source.bHasTypedDcaTrajectoryBehaviour)
            {
                FKyaDcaBehaviourOffsets Off;
                if (Actor.ClassId != 0x1b || Source.Id != 5 ||
                    !KyaReadDcaBehaviour(5, MakeArrayView(Bytes.GetData(), int32(SceneEnd)), Location.Payload, Location.Size, Off) || !Off.bTrajectory) return false;
            }
            if (Source.bHasTypedFogBehaviour)
            {
                FKyaFogZoneOffsets Off;
                if (Actor.ClassId != 0x3d || Source.Id != 2 ||
                    !KyaReadFogZoneOffsets(MakeArrayView(Bytes.GetData(), int32(SceneEnd)), Location.Payload, Location.Payload + Location.Size, Off) ||
                    Off.CountValue != Source.FogZones.Num()) return false;
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
            if (Source.bHasTypedGravityFallBehaviour &&
                (Actor.ClassId != 0x10 || Source.Id != 3 || Location.Size != 4)) return false;
            if (Source.bHasTypedGravityBellBehaviour)
            {
                FKyaGravityBellOffsets Off;
                if (Actor.ClassId != 0x10 || Source.Id != 4 ||
                    !KyaReadGravityBellOffsets(MakeArrayView(Bytes.GetData(), int32(SceneEnd)), Location.Payload, Location.Payload + Location.Size, Off) ||
                    Source.GravityBellSwitch.Targets.Num() != Off.Switch.Count) return false;
            }
            if (Source.bHasTypedCatchByTrapBehaviour)
            {
                FKyaCatchByTrapOffsets Off;
                if ((Actor.ClassId != 0x07 && Actor.ClassId != 0x0a) || Source.Id != 2 ||
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
                FKyaInventoryOffsets Off;
                if (Actor.ClassId != 0x1c || Source.Id != 9 ||
                    !KyaReadInventoryOffsets(MakeArrayView(Bytes.GetData(), int32(SceneEnd)),
                        Location.Payload, Location.Size, Off)) return false;
            }
            if (Source.bHasTypedWolfenBehaviour)
            {
                FKyaWolfenBehaviourOffsets Off;
                if (Actor.ClassId != 0x09 ||
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
            TArray<int64> Checkpoints;
            if (!KyaReadCheckpointOffsets(MakeArrayView(Bytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, Checkpoints) ||
                Checkpoints.Num() != Actor.Checkpoints.Num()) return false;
            for (int32 Checkpoint = 0; Checkpoint < Checkpoints.Num(); ++Checkpoint)
                if (ReadU32(Checkpoints[Checkpoint] + 20) != uint32(Actor.Checkpoints[Checkpoint].ActorWaypoints.Num()) ||
                    Actor.Checkpoints[Checkpoint].Flags < 0 || Actor.Checkpoints[Checkpoint].Flags > MAX_uint32) return false;
        }
        for (TCHAR Char : Actor.Name) if (Char < 1 || Char > 127) return false;
        if (Align(Actor.Name.Len() + 1, 4) != Offsets.NameCapacity || Actor.Models.Num() * 2 != Offsets.ModelStrings.Num()) return false;
        for (int32 Model = 0; Model < Actor.Models.Num(); ++Model)
            for (int32 Part = 0; Part < 2; ++Part)
            {
                const FString& Name = Part == 0 ? Actor.Models[Model].MeshName : Actor.Models[Model].TextureName;
                for (TCHAR Char : Name) if (Char < 1 || Char > 127) return false;
                if (Align(Name.Len() + 1, 4) != Offsets.ModelStrings[Model * 2 + Part].Capacity) return false;
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
        Warn->Logf(ELogVerbosity::Error, TEXT("Collision bank count cannot be changed."));
        return false;
    }
    const int64 AnimationCountPos = Pos + int64(CollisionCount) * 4;
    if (AnimationCountPos + 4 > End || Bank->SceneConfiguration.LevelAnimationCount < 0)
    {
        Warn->Logf(ELogVerbosity::Error, TEXT("Invalid animation count or truncated scene configuration."));
        return false;
    }
    const int64 ClassCountPos = AnimationCountPos + 20;
    if (ClassCountPos + 4 > End) return false;
    const uint32 ClassCount = ReadU32(ClassCountPos);
    const int64 ClassDataPos = ClassCountPos + 4;
    if (ClassCount > uint64(End - ClassDataPos) / 8 || ClassCount != uint32(Bank->SceneConfiguration.ActorClassAllocations.Num()) ||
        ClassDataPos + int64(ClassCount) * 8 + 4 > End)
    {
        Warn->Logf(ELogVerbosity::Error, TEXT("Actor class allocation count cannot be changed."));
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
        const int64 F = Offsets.Fixed;
        WriteFloat(Offsets.Weight, Actor.FighterWeightB);
        WriteU32(Offsets.Autonomous, uint32(Actor.FighterLifeMax));
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
            if (Actor.Behaviours[Behaviour].bHasTypedFxBehaviour)
            {
                const FKyaActorBehaviour& Source = Actor.Behaviours[Behaviour];
                WriteU32(BehaviourOffset.Payload, uint32(Source.FxBehaviourValue));
                if (Source.Id == 3) WriteU32(BehaviourOffset.Payload + 4, uint32(Source.FxPathFollowIndex));
                if (Source.Id == 4)
                    for (int32 Item = 0; Item < Source.FxUnknownValues.Num(); ++Item)
                        WriteU32(BehaviourOffset.Payload + 4 + Item * 4, uint32(Source.FxUnknownValues[Item]));
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
                for (int32 Item = 0; Item < Source.WeaponBurst.Num(); ++Item)
                {
                    const FKyaWeaponAdvancedEntry& Entry = Source.WeaponBurst[Item];
                    const int64 E = BehaviourOffset.Payload + int64(Item) * 16;
                    WriteFloat(E, Entry.FieldC); WriteU32(E + 4, uint32(Entry.Field4));
                    WriteFloat(E + 8, Entry.Field0); WriteFloat(E + 12, Entry.Field8);
                }
                if (Source.Id == 3 || Source.Id == 4)
                {
                    const int64 B = BehaviourOffset.Payload + 32;
                    WriteU32(B, uint32(Source.WeaponField40)); WriteU32(B + 4, uint32(Source.WeaponField44));
                    WriteU32(B + 8, uint32(Source.WeaponSoundIndex));
                    if (Source.Id == 3)
                    {
                        WriteU32(B + 12, uint32(Source.WeaponPistolField2E0));
                        WriteU32(B + 16, uint32(Source.WeaponPistolField2E4));
                    }
                    else
                    {
                        WriteU32(B + 12, uint32(Source.WeaponSniperTextureIndex));
                        WriteU32(B + 16, uint32(Source.WeaponSniperMeshIndex));
                        WriteU32(B + 20, uint32(Source.WeaponSniperMaterialId));
                    }
                }
                else if (Source.Id == 6)
                    for (int32 Item = 0; Item < Source.WeaponRpgActorRefs.Num(); ++Item)
                        WriteU32(BehaviourOffset.Payload + 36 + Item * 4, uint32(Source.WeaponRpgActorRefs[Item]));
            }
            if (Actor.Behaviours[Behaviour].bHasTypedBonusBehaviour)
            {
                const FKyaActorBehaviour& Source = Actor.Behaviours[Behaviour];
                if (Source.Id == 3)
                {
                    WriteFloat(BehaviourOffset.Payload, Source.BonusField14);
                    WriteFloat(BehaviourOffset.Payload + 4, Source.BonusField1B8);
                    WriteFloat(BehaviourOffset.Payload + 8, Source.BonusField1B4);
                    WriteU32(BehaviourOffset.Payload + 12, uint32(Source.BonusMaterialId));
                    WriteFloat(BehaviourOffset.Payload + 16, Source.BonusField1B0);
                    WriteU32(BehaviourOffset.Payload + 20, uint32(Source.BonusFxTailCount));
                    WriteU32(BehaviourOffset.Payload + 24, uint32(Source.BonusG2DIndex));
                    WriteFloat(BehaviourOffset.Payload + 28, Source.BonusTurnValue);
                }
                else if (Source.Id == 5) WriteU32(BehaviourOffset.Payload, uint32(Source.BonusFlockPathIndex));
                else if (Source.Id == 6) WriteFloat(BehaviourOffset.Payload, Source.BonusAddOnField18);
            }
            if (Actor.Behaviours[Behaviour].bHasTypedMickenBehaviour)
            {
                const FKyaActorBehaviour& Source = Actor.Behaviours[Behaviour];
                WriteFloat(BehaviourOffset.Payload, Source.MickenEatField8);
                WriteFloat(BehaviourOffset.Payload + 4, Source.MickenEatFieldC);
                WriteU32(BehaviourOffset.Payload + 8, uint32(Source.MickenEatField10));
                WriteU32(BehaviourOffset.Payload + 12, uint32(Source.MickenEatPathIndex));
            }
            if (Actor.Behaviours[Behaviour].bHasTypedCommanderBehaviour)
            {
                const FKyaActorBehaviour& Source = Actor.Behaviours[Behaviour];
                FKyaCommanderBehaviourOffsets Off;
                if (!KyaReadCommanderBehaviourOffsets(MakeArrayView(Bank->BankBytes.GetData(), int32(SceneEnd)),
                    BehaviourOffset.Payload, BehaviourOffset.Payload + BehaviourOffset.Size, Off)) return false;
                WriteU32(Off.SquadConfig, uint32(Source.CommanderSquadConfig[0]));
                WriteU32(Off.SquadConfig + 4, uint32(Source.CommanderSquadConfig[1]));
                for (int32 Item = 0; Item < Source.CommanderSwitch.Targets.Num(); ++Item)
                {
                    const FKyaSwitchTarget& Target = Source.CommanderSwitch.Targets[Item];
                    const int64 T = Off.Switch + 4 + int64(Item) * 28;
                    WriteU32(T, uint32(Target.TargetRef)); WriteU32(T + 4, uint32(Target.CutsceneId));
                    WriteU32(T + 8, uint32(Target.Flags)); WriteU32(T + 12, uint32(Target.MessageId));
                    WriteU32(T + 16, uint32(Target.MessageFlags)); WriteU32(T + 20, uint32(Target.Field14));
                    WriteU32(T + 24, uint32(Target.Field18));
                }
                const int64 Camera = Off.Switch + 4 + int64(Source.CommanderSwitch.Targets.Num()) * 28;
                const FKyaStreamEventCamera& Cam = Source.CommanderSwitch.Camera;
                WriteU32(Camera, uint32(Cam.CameraIndex)); WriteU32(Camera + 4, uint32(Cam.Flags));
                WriteFloat(Camera + 8, Cam.Field8); WriteFloat(Camera + 12, Cam.FieldC); WriteFloat(Camera + 16, Cam.Field10);
                WriteU32(Camera + 20, uint32(Cam.ActorRef)); WriteU32(Camera + 24, uint32(Cam.ZoneRef));
                WriteFloat(Camera + 28, Cam.Field1C);
            }
            if (Actor.Behaviours[Behaviour].bHasTypedNativBehaviour)
            {
                const FKyaActorBehaviour& Source = Actor.Behaviours[Behaviour];
                FKyaNativTakePutOffsets Off;
                if (!KyaReadNativTakePutOffsets(MakeArrayView(Bank->BankBytes.GetData(), int32(SceneEnd)),
                    BehaviourOffset.Payload, BehaviourOffset.Payload + BehaviourOffset.Size, Off) ||
                    Off.Items.Num() != Source.NativTrajectories.Num()) return false;
                for (int32 Item = 0; Item < Source.NativTrajectories.Num(); ++Item)
                {
                    const FKyaNativTrajectory& Trajectory = Source.NativTrajectories[Item];
                    const int64 P = Off.Items[Item];
                    WriteFloat(P, Trajectory.Field0);
                    WriteU32(P + 4, uint32(Trajectory.PathFollowIndex));
                    WriteU32(P + 8, uint32(Trajectory.Field14));
                    WriteFloat(P + 12, Trajectory.Field18);
                }
            }
            if (Actor.Behaviours[Behaviour].bHasTypedCinematicBehaviour)
            {
                const FKyaActorBehaviour& Source = Actor.Behaviours[Behaviour];
                WriteU32(BehaviourOffset.Payload, uint32(Source.CinematicLeaveBehaviourId));
            }
            if (Actor.Behaviours[Behaviour].bHasTypedDcaControlledBehaviour)
            {
                const FKyaActorBehaviour& Source = Actor.Behaviours[Behaviour];
                const int64 P = BehaviourOffset.Payload;
                WriteFloat(P, Source.DcaControlledField0);
                WriteFloat(P + 4, Source.DcaControlledField4);
                WriteFloat(P + 8, Source.DcaControlledField8);
                WriteFloat(P + 12, Source.DcaControlledFovDegrees);
                for (int32 Axis = 0; Axis < 3; ++Axis) WriteFloat(P + 16 + Axis * 4, float(Source.DcaControlledVector10[Axis]));
                for (int32 Axis = 0; Axis < 3; ++Axis) WriteFloat(P + 28 + Axis * 4, float(Source.DcaControlledVector20[Axis]));
            }
            if (Actor.Behaviours[Behaviour].bHasTypedDcaTrajectoryBehaviour)
            {
                const FKyaActorBehaviour& Source = Actor.Behaviours[Behaviour];
                const int64 P = BehaviourOffset.Payload;
                WriteU32(P, uint32(Source.DcaTrajectoryActorRef));
                WriteFloat(P + 4, Source.DcaTrajectoryField18);
                WriteU32(P + 8, uint32(Source.DcaTrajectoryField8));
            }
            if (Actor.Behaviours[Behaviour].bHasTypedFogBehaviour)
            {
                const FKyaActorBehaviour& Source = Actor.Behaviours[Behaviour];
                for (int32 Item = 0; Item < Source.FogZones.Num(); ++Item)
                {
                    const FKyaFogZone& Zone = Source.FogZones[Item];
                    const int64 Z = BehaviourOffset.Payload + 4 + int64(Item) * 28;
                    WriteU32(Z, uint32(Zone.Flags));
                    WriteU32(Z + 4, uint32(Zone.ZoneRef));
                    WriteFloat(Z + 8, Zone.ClipValue);
                    WriteFloat(Z + 12, Zone.Field4);
                    WriteU32(Z + 16, uint32(Zone.FogRGBA));
                    WriteU32(Z + 20, uint32(Zone.FogFlags));
                    WriteFloat(Z + 24, Zone.Field18);
                }
            }
            if (Actor.Behaviours[Behaviour].bHasTypedShootFireBehaviour)
            {
                const FKyaActorBehaviour& Source = Actor.Behaviours[Behaviour];
                const int64 P = BehaviourOffset.Payload;
                WriteU32(P, uint32(Source.ShootFireField2B0));
                WriteFloat(P + 4, Source.ShootFireField8);
                WriteFloat(P + 8, Source.ShootFireField2B4);
                for (int32 Item = 0; Item < Source.ShootFireActorRefs.Num(); ++Item)
                    WriteU32(P + 16 + Item * 4, uint32(Source.ShootFireActorRefs[Item]));
            }
            if (Actor.Behaviours[Behaviour].bHasTypedShootWaveBehaviour)
            {
                const FKyaActorBehaviour& Source = Actor.Behaviours[Behaviour];
                for (int32 Item = 0; Item < Source.ShootWaveValues.Num(); ++Item)
                    WriteFloat(BehaviourOffset.Payload + Item * 4, Source.ShootWaveValues[Item]);
            }
            if (Actor.Behaviours[Behaviour].bHasTypedFruitBehaviour)
            {
                const FKyaActorBehaviour& Source = Actor.Behaviours[Behaviour];
                if (Source.Id == 4) WriteFloat(BehaviourOffset.Payload, Source.FruitSquashedValue);
                else if (Source.Id == 6)
                {
                    WriteU32(BehaviourOffset.Payload, uint32(Source.FruitBonusActorRef));
                    WriteU32(BehaviourOffset.Payload + 4, uint32(Source.FruitBonusInitialBehaviour));
                }
                else if (Source.Id == 5)
                {
                    const int64 P = BehaviourOffset.Payload;
                    if (BehaviourOffset.Size == 8) WriteU32(P + 4, uint32(Source.FruitInventoryInitialBehaviour));
                    else
                    {
                        WriteU32(P + 4, uint32(uint64(Source.FruitInventoryField8)));
                        WriteU32(P + 8, uint32(uint64(Source.FruitInventoryField8) >> 32));
                        WriteU32(P + 12, uint32(uint64(Source.FruitInventoryField10)));
                        WriteU32(P + 16, uint32(uint64(Source.FruitInventoryField10) >> 32));
                        WriteU32(P + 20, uint32(Source.FruitInventoryPurchaseId));
                        WriteU32(P + 24, uint32(Source.FruitInventoryField28));
                        WriteU32(P + 28, uint32(Source.FruitInventoryMoneyCost));
                        WriteU32(P + 32, uint32(Source.FruitInventoryField2C));
                        if (BehaviourOffset.Size == 40) WriteU32(P + 36, uint32(Source.FruitInventoryInitialBehaviour));
                        else
                        {
                            WriteU32(P + 36, Source.FruitInventoryHasObject ? 1 : 0);
                            if (Source.FruitInventoryHasObject)
                            {
                                WriteU32(P + 40, uint32(Source.FruitInventoryObjectId));
                                WriteU32(P + 44, uint32(uint64(Source.FruitInventoryField18)));
                                WriteU32(P + 48, uint32(uint64(Source.FruitInventoryField18) >> 32));
                                WriteU32(P + 52, uint32(Source.FruitInventoryInitialBehaviour));
                            }
                            else WriteU32(P + 40, uint32(Source.FruitInventoryInitialBehaviour));
                        }
                    }
                }
            }
            if (Actor.Behaviours[Behaviour].bHasTypedGravityFallBehaviour)
            {
                const FKyaActorBehaviour& Source = Actor.Behaviours[Behaviour];
                WriteU32(BehaviourOffset.Payload, uint32(Source.GravityFallSoundRef));
            }
            if (Actor.Behaviours[Behaviour].bHasTypedGravityBellBehaviour)
            {
                const FKyaActorBehaviour& Source = Actor.Behaviours[Behaviour];
                const int64 P = BehaviourOffset.Payload;
                WriteU32(P, uint32(Source.GravityBellSoundRef));
                WriteFloat(P + 4, Source.GravityBellFieldC);
                for (int32 Item = 0; Item < Source.GravityBellSwitch.Targets.Num(); ++Item)
                {
                    const FKyaSwitchTarget& Target = Source.GravityBellSwitch.Targets[Item];
                    const int64 T = P + 12 + int64(Item) * 28;
                    WriteU32(T, uint32(Target.TargetRef)); WriteU32(T + 4, uint32(Target.CutsceneId));
                    WriteU32(T + 8, uint32(Target.Flags)); WriteU32(T + 12, uint32(Target.MessageId));
                    WriteU32(T + 16, uint32(Target.MessageFlags)); WriteU32(T + 20, uint32(Target.Field14));
                    WriteU32(T + 24, uint32(Target.Field18));
                }
                const FKyaStreamEventCamera& Camera = Source.GravityBellSwitch.Camera;
                const int64 C = P + 12 + int64(Source.GravityBellSwitch.Targets.Num()) * 28;
                WriteU32(C, uint32(Camera.CameraIndex)); WriteU32(C + 4, uint32(Camera.Flags));
                WriteFloat(C + 8, Camera.Field8); WriteFloat(C + 12, Camera.FieldC); WriteFloat(C + 16, Camera.Field10);
                WriteU32(C + 20, uint32(Camera.ActorRef)); WriteU32(C + 24, uint32(Camera.ZoneRef));
                WriteFloat(C + 28, Camera.Field1C);
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
                FKyaInventoryOffsets Off;
                if (!KyaReadInventoryOffsets(MakeArrayView(Bank->BankBytes.GetData(), int32(SceneEnd)),
                    BehaviourOffset.Payload, BehaviourOffset.Size, Off)) return false;
                const FKyaInventoryInfo& Values = Source.InventoryInfo;
                WriteU32(Off.Field0, uint32(Values.Field0));
                if (Off.Field8 != 0)
                {
                    WriteU32(Off.Field8, uint32(uint64(Values.Field8)));
                    WriteU32(Off.Field8 + 4, uint32(uint64(Values.Field8) >> 32));
                    WriteU32(Off.Field10, uint32(uint64(Values.Field10)));
                    WriteU32(Off.Field10 + 4, uint32(uint64(Values.Field10) >> 32));
                    WriteU32(Off.PurchaseId, uint32(Values.PurchaseId));
                    WriteU32(Off.Field28, uint32(Values.Field28));
                    WriteU32(Off.MoneyCost, uint32(Values.MoneyCost));
                    WriteU32(Off.Field2C, uint32(Values.Field2C));
                }
                if (Off.bHasObject)
                {
                    WriteU32(Off.ObjectId, uint32(Values.ObjectId));
                    WriteU32(Off.Field18, uint32(uint64(Values.Field18)));
                    WriteU32(Off.Field18 + 4, uint32(uint64(Values.Field18) >> 32));
                }
                WriteU32(Off.InitialBehaviourId, uint32(Values.InitialBehaviourId));
            }
            if (Actor.Behaviours[Behaviour].bHasTypedWolfenBehaviour)
            {
                const FKyaActorBehaviour& Source = Actor.Behaviours[Behaviour];
                FKyaWolfenBehaviourOffsets Off;
                if (!KyaReadWolfenBehaviourOffsets(Source.Id, MakeArrayView(Bank->BankBytes.GetData(), int32(SceneEnd)),
                    BehaviourOffset.Payload, BehaviourOffset.Size, Off)) return false;
                const FKyaWolfenBehaviour& Values = Source.WolfenBehaviour;
                if (!Off.bExorcism) WriteU32(Off.Flags, uint32(Values.Flags));
                if (Off.bTrack) WriteFloat(Off.TrackField, Values.TrackField);
                if (Off.bDca)
                {
                    WriteU32(Off.DcaActorRef, uint32(Values.DcaActorRef));
                    WriteFloat(Off.DcaShootCooldown, Values.DcaShootCooldown);
                }
                if (Off.bAvoid)
                {
                    WriteFloat(Off.Avoid80, Values.AvoidField80);
                    WriteFloat(Off.Avoid84, Values.AvoidField84);
                }
                if (Off.bWeaponStand || Off.bWeapon || Off.bWeaponSnipe)
                {
                    WriteFloat(Off.WeaponField90, Values.WeaponField90);
                    WriteU32(Off.WeaponField94, uint32(Values.WeaponField94));
                    WriteU32(Off.WeaponField98, uint32(Values.WeaponField98));
                    for (int32 SubObjIndex = 0; SubObjIndex < Values.WeaponSubObjs.Num() && SubObjIndex < Off.SubObjs.Num(); ++SubObjIndex)
                    {
                        const FKyaTrackSubObj& SubObj = Values.WeaponSubObjs[SubObjIndex];
                        const FKyaTrackSubObjOffsets& SubObjOffsets = Off.SubObjs[SubObjIndex];
                        WriteU32(SubObjOffsets.Field0, uint32(SubObj.Field0));
                        WriteU32(SubObjOffsets.AField0, uint32(SubObj.AField0));
                        WriteFloat(SubObjOffsets.AField4, SubObj.AField4);
                        WriteFloat(SubObjOffsets.AField8, SubObj.AField8);
                        WriteU32(SubObjOffsets.Field10, uint32(SubObj.Field10));
                    }
                    if (Off.bWeapon || Off.bWeaponSnipe) WriteFloat(Off.WeaponFieldF0, Values.WeaponFieldF0);
                    if (Off.bWeapon) WriteFloat(Off.WeaponTargetField, Values.WeaponTargetField);
                }
                auto WriteSwitchScript = [&](const FKyaSwitchScriptOffsets& Script) -> bool {
                    if (Values.SwitchRecords.Num() != Script.RecordOffsets.Num()) return false;
                    int32 CategoryCounts[4] = { 0, 0, 0, 0 };
                    for (const FKyaSwitchBehaviourRecord& Record : Values.SwitchRecords)
                    {
                        if (Record.Category < 0 || Record.Category > 3) return false;
                        ++CategoryCounts[Record.Category];
                    }
                    for (int32 Category = 0; Category < 4; ++Category)
                    {
                        if (CategoryCounts[Category] != Script.CountValues[Category]) return false;
                        WriteU32(Script.Counts[Category], uint32(Script.CountValues[Category]));
                    }
                    for (int32 ScriptIndex = 0; ScriptIndex < Values.SwitchRecords.Num(); ++ScriptIndex)
                    {
                        const FKyaSwitchBehaviourRecord& Record = Values.SwitchRecords[ScriptIndex];
                        const int64 P = Script.RecordOffsets[ScriptIndex];
                        if (Record.Category == 0)
                        {
                            WriteU32(P, uint32(Record.MessageType));
                            WriteU32(P + 4, uint32(Record.BehaviourId));
                        }
                        else if (Record.Category == 3)
                        {
                            WriteU32(P, uint32(Record.SeeActorRef));
                            WriteU32(P + 4, uint32(Record.BehaviourId));
                        }
                        else
                        {
                            WriteU32(P, uint32(Record.ActorRef));
                            WriteU32(P + 4, uint32(Record.ZoneRef));
                            WriteU32(P + 8, uint32(Record.Flags));
                            WriteU32(P + 12, uint32(Record.BehaviourId));
                        }
                    }
                    return true;
                };
                if (Off.bLost && !WriteSwitchScript(Off.Switch)) return false;
                if (Off.bWatchDog)
                {
                    WriteU32(Off.Field80, uint32(Values.Field80));
                    WriteU32(Off.TrackBehaviourId, uint32(Values.TrackBehaviourId));
                    if (!WriteSwitchScript(Off.Switch)) return false;
                    if (Values.OnOffTargets.Num() != Off.OnOffCountValue) return false;
                    for (int32 TargetIndex = 0; TargetIndex < Values.OnOffTargets.Num(); ++TargetIndex)
                    {
                        const FKyaSwitchTarget& Target = Values.OnOffTargets[TargetIndex];
                        const int64 T = Off.OnOffEntries + int64(TargetIndex) * 28;
                        WriteU32(T, uint32(Target.TargetRef)); WriteU32(T + 4, uint32(Target.CutsceneId));
                        WriteU32(T + 8, uint32(Target.Flags)); WriteU32(T + 12, uint32(Target.MessageId));
                        WriteU32(T + 16, uint32(Target.MessageFlags)); WriteU32(T + 20, uint32(Target.Field14));
                        WriteU32(T + 24, uint32(Target.Field18));
                    }
                    const int64 C = Off.OnOffCamera;
                    WriteU32(C, uint32(Values.OnOffCamera.CameraIndex)); WriteU32(C + 4, uint32(Values.OnOffCamera.Flags));
                    WriteFloat(C + 8, Values.OnOffCamera.Field8); WriteFloat(C + 12, Values.OnOffCamera.FieldC);
                    WriteFloat(C + 16, Values.OnOffCamera.Field10); WriteU32(C + 20, uint32(Values.OnOffCamera.ActorRef));
                    WriteU32(C + 24, uint32(Values.OnOffCamera.ZoneRef)); WriteFloat(C + 28, Values.OnOffCamera.Field1C);
                }
                if (Off.bExorcism)
                {
                    if (Values.ExorcismWords.Num() != 5) return false;
                    WriteU32(Off.ExorcismMaterial, uint32(Values.ExorcismDigitMaterialId));
                    WriteU32(Off.ExorcismField24, uint32(Values.ExorcismField24));
                    WriteU32(Off.ExorcismMeshes, uint32(Values.ExorcismTextureA));
                    WriteU32(Off.ExorcismMeshes + 4, uint32(Values.ExorcismMeshA));
                    WriteU32(Off.ExorcismMeshes + 8, uint32(Values.ExorcismTextureB));
                    WriteU32(Off.ExorcismMeshes + 12, uint32(Values.ExorcismMeshB));
                    WriteU32(Off.ExorcismField18, uint32(Values.ExorcismField18));
                    for (int32 WordIndex = 0; WordIndex < Values.ExorcismWords.Num(); ++WordIndex)
                        WriteU32(Off.ExorcismWords + WordIndex * 4, uint32(Values.ExorcismWords[WordIndex]));
                }
                if (Off.bGuard)
                {
                    WriteU32(Off.GuardPath, uint32(Values.GuardPathIndex));
                    WriteFloat(Off.GuardField98, Values.GuardField98);
                    WriteU32(Off.GuardTrack, uint32(Values.GuardTrackBehaviourId));
                    WriteU32(Off.GuardField90, uint32(Values.GuardField90));
                    if (!WriteSwitchScript(Off.Switch)) return false;
                    if (Values.OnOffTargets.Num() != Off.OnOffCountValue) return false;
                    for (int32 TargetIndex = 0; TargetIndex < Values.OnOffTargets.Num(); ++TargetIndex)
                    {
                        const FKyaSwitchTarget& Target = Values.OnOffTargets[TargetIndex];
                        const int64 T = Off.OnOffEntries + int64(TargetIndex) * 28;
                        WriteU32(T, uint32(Target.TargetRef)); WriteU32(T + 4, uint32(Target.CutsceneId));
                        WriteU32(T + 8, uint32(Target.Flags)); WriteU32(T + 12, uint32(Target.MessageId));
                        WriteU32(T + 16, uint32(Target.MessageFlags)); WriteU32(T + 20, uint32(Target.Field14));
                        WriteU32(T + 24, uint32(Target.Field18));
                    }
                    const int64 C = Off.OnOffCamera;
                    WriteU32(C, uint32(Values.OnOffCamera.CameraIndex)); WriteU32(C + 4, uint32(Values.OnOffCamera.Flags));
                    WriteFloat(C + 8, Values.OnOffCamera.Field8); WriteFloat(C + 12, Values.OnOffCamera.FieldC);
                    WriteFloat(C + 16, Values.OnOffCamera.Field10); WriteU32(C + 20, uint32(Values.OnOffCamera.ActorRef));
                    WriteU32(C + 24, uint32(Values.OnOffCamera.ZoneRef)); WriteFloat(C + 28, Values.OnOffCamera.Field1C);
                }
                if (Off.bEscape)
                {
                    if (Values.EscapePathIndices.Num() != Off.EscapeCountValue) return false;
                    for (int32 PathIndex = 0; PathIndex < Values.EscapePathIndices.Num(); ++PathIndex)
                        WriteU32(Off.EscapeEntries + PathIndex * 4, uint32(Values.EscapePathIndices[PathIndex]));
                    WriteU32(Off.EscapeField88, uint32(Values.EscapeField88));
                    WriteFloat(Off.EscapeFloats, Values.EscapeField9C);
                    WriteFloat(Off.EscapeFloats + 4, Values.EscapeField90);
                    WriteFloat(Off.EscapeFloats + 8, Values.EscapeField8C);
                    WriteFloat(Off.EscapeFloats + 12, Values.EscapeField94);
                    WriteFloat(Off.EscapeFloats + 16, Values.EscapeField98);
                    WriteU32(Off.EscapeFieldA0, uint32(Values.EscapeFieldA0));
                    if (!WriteSwitchScript(Off.Switch)) return false;
                }
                if (Off.bSnipe)
                {
                    WriteU32(Off.SnipeField90, uint32(Values.SnipeField90));
                    WriteFloat(Off.SnipeField94, Values.SnipeField94);
                    WriteFloat(Off.SnipeField94 + 4, Values.SnipeField98);
                    WriteFloat(Off.SnipeField94 + 8, Values.SnipeField9C);
                    WriteFloat(Off.SnipeField94 + 12, Values.SnipeFieldA0);
                    WriteU32(Off.SpotTrack, uint32(Values.SpotTrackBehaviourId));
                    WriteU32(Off.SnipeTrack, uint32(Values.SnipeTrackBehaviourId));
                    WriteU32(Off.ShadowMaterial, uint32(Values.ShadowMaterialId));
                    WriteU32(Off.SnipeFieldAC, uint32(Values.SnipeFieldAC));
                    WriteFloat(Off.SnipeFieldB0, Values.SnipeFieldB0);
                    WriteFloat(Off.SnipeFieldB4, Values.SnipeFieldB4);
                    WriteU32(Off.NbSpot, uint32(Values.NbSpot));
                    WriteU32(Off.NbShadow, uint32(Values.NbShadow));
                    WriteFloat(Off.RotationSpeed, Values.RotationSpeed);
                    WriteU32(Off.SnipeFieldB8, uint32(Values.SnipeFieldB8));
                    if (!WriteSwitchScript(Off.Switch)) return false;
                }
            }
            if (Actor.Behaviours[Behaviour].bHasTypedProjectileBehaviour)
            {
                const FKyaActorBehaviour& Source = Actor.Behaviours[Behaviour];
                FKyaProjectileBehaviourOffsets Off;
                if (!KyaReadProjectileBehaviourOffsets(Source.Id, MakeArrayView(Bank->BankBytes.GetData(), int32(SceneEnd)),
                    BehaviourOffset.Payload, BehaviourOffset.Size, Off)) return false;
                const FKyaProjectileBehaviour& Values = Source.ProjectileBehaviour;
                WriteU32(Off.BehaviourId, uint32(Values.BehaviourId));
                WriteU32(Off.Field0C, uint32(Values.Field0C));
                if (Off.bStand) WriteU32(Off.MaterialId, uint32(Values.MaterialId));
                if (Off.bStraight)
                {
                    WriteFloat(Off.Speed, Values.Speed);
                    WriteFloat(Off.Acceleration, Values.Acceleration);
                    WriteFloat(Off.Field18, Values.Field18);
                }
                if (Off.bPortable) WriteFloat(Off.Field10, Values.Field10);
                if (Off.bExcuse) WriteU32(Off.ActorRef, uint32(Values.ActorRef));
            }
            if (Actor.Behaviours[Behaviour].bHasTypedSwitchBehaviour)
            {
                const FKyaActorBehaviour& Source = Actor.Behaviours[Behaviour];
                FKyaSwitchBehaviourOffsets Off;
                if (!KyaReadSwitchBehaviourOffsets(Source.Id, MakeArrayView(Bank->BankBytes.GetData(), int32(SceneEnd)),
                    BehaviourOffset.Payload, BehaviourOffset.Size, Off)) return false;
                const FKyaSwitchBehaviour& Values = Source.SwitchBehaviour;
                if (Off.bLever)
                {
                    WriteU32(Off.LeverRefA, uint32(Values.LeverRefA));
                    WriteU32(Off.LeverRefB, uint32(Values.LeverRefB));
                }
                if (Off.bMagic)
                {
                    WriteFloat(Off.MagicField08, Values.MagicField08);
                    WriteFloat(Off.MagicField10, Values.MagicField10);
                    WriteU32(Off.MagicFxDigits, uint32(Values.MagicFxDigits));
                    WriteFloat(Off.MagicField14, Values.MagicField14);
                    WriteU32(Off.MagicField4C, uint32(Values.MagicField4C));
                    WriteU32(Off.MagicField18, uint32(Values.MagicField18));
                    WriteU32(Off.MagicField28, uint32(Values.MagicField28));
                    WriteU32(Off.MagicField24, uint32(Values.MagicField24));
                    WriteU32(Off.MagicField30, uint32(Values.MagicField30));
                    WriteU32(Off.OpenTextureId, uint32(Values.OpenTextureId));
                    WriteU32(Off.OpenMeshId, uint32(Values.OpenMeshId));
                    const int64 C = Off.MagicCamera;
                    WriteU32(C, uint32(Values.MagicCamera.CameraIndex)); WriteU32(C + 4, uint32(Values.MagicCamera.Flags));
                    WriteFloat(C + 8, Values.MagicCamera.Field8); WriteFloat(C + 12, Values.MagicCamera.FieldC);
                    WriteFloat(C + 16, Values.MagicCamera.Field10); WriteU32(C + 20, uint32(Values.MagicCamera.ActorRef));
                    WriteU32(C + 24, uint32(Values.MagicCamera.ZoneRef)); WriteFloat(C + 28, Values.MagicCamera.Field1C);
                }
                if (Off.bMultiCondition)
                {
                    WriteU32(Off.MultiField14, uint32(Values.MultiField14));
                    WriteU32(Off.MultiField08, uint32(Values.MultiField08));
                    WriteU32(Off.MultiField0C, uint32(Values.MultiField0C));
                }
                if (Off.bTarget)
                {
                    WriteFloat(Off.TargetDisplacement, Values.TargetDisplacement);
                    WriteFloat(Off.TargetSpringStrength, Values.TargetSpringStrength);
                    WriteFloat(Off.TargetDamping, Values.TargetDamping);
                }
            }
            if (Actor.Behaviours[Behaviour].bHasTypedPlatformBehaviour)
            {
                const FKyaActorBehaviour& Source = Actor.Behaviours[Behaviour];
                FKyaPlatformBehaviourOffsets Off;
                if (!KyaReadPlatformBehaviourOffsets(Source.Id, MakeArrayView(Bank->BankBytes.GetData(), int32(SceneEnd)),
                    BehaviourOffset.Payload, BehaviourOffset.Size, Off)) return false;
                const FKyaPlatformBehaviour& Values = Source.PlatformBehaviour;
                if (Off.bStand)
                {
                    WriteU32(Off.SoundRef, uint32(Values.SoundRef));
                    WriteU32(Off.Field0C, uint32(Values.Field0C));
                }
                if (Off.bTrajectory)
                {
                    WriteU32(Off.PathIndex, uint32(Values.PathIndex));
                    WriteU32(Off.Timing, uint32(Values.Timing));
                    WriteFloat(Off.PathField4, Values.PathField4);
                    WriteFloat(Off.PathTotalTime, Values.PathTotalTime);
                    WriteFloat(Off.SegmentStartTime, Values.SegmentStartTime);
                    WriteFloat(Off.LoopDelay, Values.LoopDelay);
                }
                if (Off.bSlab)
                {
                    if (Values.SlabSwitch.Targets.Num() != Off.SlabSwitch.Count) return false;
                    for (int32 TargetIndex = 0; TargetIndex < Values.SlabSwitch.Targets.Num(); ++TargetIndex)
                    {
                        const FKyaSwitchTarget& Target = Values.SlabSwitch.Targets[TargetIndex];
                        const int64 T = Off.SlabSwitch.Targets + int64(TargetIndex) * 28;
                        WriteU32(T, uint32(Target.TargetRef)); WriteU32(T + 4, uint32(Target.CutsceneId));
                        WriteU32(T + 8, uint32(Target.Flags)); WriteU32(T + 12, uint32(Target.MessageId));
                        WriteU32(T + 16, uint32(Target.MessageFlags)); WriteU32(T + 20, uint32(Target.Field14));
                        WriteU32(T + 24, uint32(Target.Field18));
                    }
                    {
                        const FKyaStreamEventCamera& Camera = Values.SlabSwitch.Camera;
                        const int64 C = Off.SlabSwitch.Camera;
                        WriteU32(C, uint32(Camera.CameraIndex)); WriteU32(C + 4, uint32(Camera.Flags));
                        WriteFloat(C + 8, Camera.Field8); WriteFloat(C + 12, Camera.FieldC); WriteFloat(C + 16, Camera.Field10);
                        WriteU32(C + 20, uint32(Camera.ActorRef)); WriteU32(C + 24, uint32(Camera.ZoneRef));
                        WriteFloat(C + 28, Camera.Field1C);
                    }
                    WriteFloat(Off.SlabFloats, Values.SlabField10);
                    WriteFloat(Off.SlabFloats + 4, Values.SlabField14);
                    WriteFloat(Off.SlabFloats + 8, Values.SlabField18);
                    WriteFloat(Off.SlabFloats + 12, Values.SlabField1C);
                    WriteU32(Off.SlabField20, uint32(Values.SlabField20));
                }
                if (Off.bMaster)
                {
                    WriteU32(Off.MasterPath, uint32(Values.MasterPathIndex));
                    WriteU32(Off.MasterPath + 4, uint32(Values.MasterTiming));
                    WriteU32(Off.MasterStream, uint32(Values.MasterStreamActor));
                    WriteFloat(Off.MasterFloats, Values.MasterField40);
                    WriteFloat(Off.MasterFloats + 4, Values.MasterField44);
                    if (Values.AnalogTargets.Num() != Off.AnalogCountValue) return false;
                    for (int32 TargetIndex = 0; TargetIndex < Values.AnalogTargets.Num(); ++TargetIndex)
                    {
                        const FKyaAnalogTarget& Target = Values.AnalogTargets[TargetIndex];
                        const int64 T = Off.AnalogEntries + int64(TargetIndex) * 40;
                        WriteU32(T, uint32(Target.ActorRef)); WriteU32(T + 4, uint32(Target.CutsceneId));
                        WriteU32(T + 8, uint32(Target.Flags)); WriteU32(T + 12, uint32(Target.Field0C));
                        WriteU32(T + 16, uint32(Target.Field10)); WriteU32(T + 20, uint32(Target.Field14));
                        WriteU32(T + 24, uint32(Target.Field18)); WriteU32(T + 28, uint32(Target.Field1C));
                        WriteFloat(T + 32, Target.Field20); WriteFloat(T + 36, Target.Field24);
                    }
                    {
                        const FKyaStreamEventCamera& Camera = Values.AnalogCamera;
                        const int64 C = Off.AnalogCamera;
                        WriteU32(C, uint32(Camera.CameraIndex)); WriteU32(C + 4, uint32(Camera.Flags));
                        WriteFloat(C + 8, Camera.Field8); WriteFloat(C + 12, Camera.FieldC); WriteFloat(C + 16, Camera.Field10);
                        WriteU32(C + 20, uint32(Camera.ActorRef)); WriteU32(C + 24, uint32(Camera.ZoneRef));
                        WriteFloat(C + 28, Camera.Field1C);
                    }
                }
                if (Off.bSelectorMaster)
                {
                    WriteU32(Off.Field18, uint32(Values.Field18));
                    WriteU32(Off.MaterialId, uint32(Values.MaterialId));
                    WriteFloat(Off.Field8, Values.Field8);
                    WriteFloat(Off.FieldC, Values.FieldC);
                    WriteFloat(Off.Field10, Values.Field10);
                    WriteU32(Off.SoundRef, uint32(Values.SoundRef));
                    if (Values.SelectorActorRefs.Num() != Off.SelectorActorCountValue) return false;
                    for (int32 TargetIndex = 0; TargetIndex < Values.SelectorActorRefs.Num(); ++TargetIndex)
                        WriteU32(Off.SelectorActorEntries + TargetIndex * 4, uint32(Values.SelectorActorRefs[TargetIndex]));
                    if (Values.SelectorSwitchEx.Num() != Off.SelectorSwitchCountValue) return false;
                    for (int32 TargetIndex = 0; TargetIndex < Values.SelectorSwitchEx.Num(); ++TargetIndex)
                    {
                        const FKyaSwitchExTarget& Target = Values.SelectorSwitchEx[TargetIndex];
                        const int64 T = Off.SelectorSwitchEntries + int64(TargetIndex) * 16;
                        WriteU32(T, uint32(Target.ActorRef)); WriteU32(T + 4, uint32(Target.CutsceneId));
                        WriteU32(T + 8, uint32(Target.Flags)); WriteU32(T + 12, uint32(Target.MessageId));
                    }
                    {
                        const FKyaStreamEventCamera& Camera = Values.SelectorCamera;
                        const int64 C = Off.SelectorCamera;
                        WriteU32(C, uint32(Camera.CameraIndex)); WriteU32(C + 4, uint32(Camera.Flags));
                        WriteFloat(C + 8, Camera.Field8); WriteFloat(C + 12, Camera.FieldC); WriteFloat(C + 16, Camera.Field10);
                        WriteU32(C + 20, uint32(Camera.ActorRef)); WriteU32(C + 24, uint32(Camera.ZoneRef));
                        WriteFloat(C + 28, Camera.Field1C);
                    }
                }
                if (Off.bSlave)
                {
                    WriteU32(Off.PathIndex, uint32(Values.PathIndex));
                    WriteU32(Off.Timing, uint32(Values.Timing));
                }
                if (Off.bDestroyed)
                {
                    WriteU32(Off.PathIndex, uint32(Values.PathIndex));
                    WriteU32(Off.Timing, uint32(Values.Timing));
                    WriteFloat(Off.PathField4, Values.PathField4);
                    WriteFloat(Off.PathTotalTime, Values.PathTotalTime);
                    WriteU32(Off.Field40, uint32(Values.Field40));
                    WriteU32(Off.AltModelId, uint32(Values.AltModelId));
                }
                if (Off.bSelectorSlave)
                {
                    WriteU32(Off.Field18, uint32(Values.Field18));
                    WriteU32(Off.MaterialId, uint32(Values.MaterialId));
                    WriteFloat(Off.Field8, Values.Field8);
                    WriteFloat(Off.FieldC, Values.FieldC);
                    WriteFloat(Off.Field10, Values.Field10);
                    WriteU32(Off.SoundRef, uint32(Values.SoundRef));
                }
            }
            if (Actor.Behaviours[Behaviour].bHasTypedCompanionBehaviour)
            {
                const FKyaActorBehaviour& Source = Actor.Behaviours[Behaviour];
                FKyaCompanionBehaviourOffsets Off;
                const bool ReadOk = KyaReadCompanionBehaviourOffsets(MakeArrayView(Bank->BankBytes.GetData(), int32(SceneEnd)),
                    BehaviourOffset.Payload, BehaviourOffset.Size, Off);
                if (!ReadOk) return false;
                for (int32 AlertIndex = 0; AlertIndex < Source.CompanionAlerts.Num(); ++AlertIndex)
                {
                    const FKyaCompanionAlert& Alert = Source.CompanionAlerts[AlertIndex];
                    const FKyaCompanionAlertOffsets& AlertOffsets = Off.Alerts[AlertIndex];
                    for (int32 Item = 0; Item < Alert.Conditions.Num(); ++Item)
                    {
                        const FKyaScenarioCondition& Condition = Alert.Conditions[Item];
                        const int64 C = AlertOffsets.Conditions.Entries + int64(Item) * 16;
                        WriteU32(C, uint32(Condition.Combiner));
                        WriteU32(C + 4, uint32(Condition.Variable));
                        WriteU32(C + 8, uint32(Condition.Comparison));
                        WriteU32(C + 12, uint32(Condition.Value));
                    }
                    WriteU32(AlertOffsets.Flags, uint32(Alert.Flags));
                    WriteU32(AlertOffsets.Field08, uint32(Alert.Field08));
                    WriteU32(AlertOffsets.Field10, uint32(Alert.Field10));
                    WriteU32(AlertOffsets.Field14, uint32(Alert.Field14));
                    WriteFloat(AlertOffsets.Field18, Alert.Field18);
                    WriteU32(AlertOffsets.ActorRef, uint32(Alert.ActorRef));
                    for (int32 Axis = 0; Axis < 3; ++Axis)
                        WriteFloat(AlertOffsets.Vector20 + Axis * 4, float(Alert.Field20[Axis]));
                    WriteU32(AlertOffsets.CameraRef, uint32(Alert.CameraRef));
                    for (int32 Item = 0; Item < Alert.Field34.Num(); ++Item)
                        WriteU32(AlertOffsets.ListEntries + Item * 4, uint32(Alert.Field34[Item]));
                    WriteU32(AlertOffsets.LevelStringKey, uint32(uint64(Alert.LevelStringKey)));
                    WriteU32(AlertOffsets.LevelStringKey + 4, uint32(uint64(Alert.LevelStringKey) >> 32));
                    WriteFloat(AlertOffsets.Field50, Alert.Field50);
                    WriteU32(AlertOffsets.Field58, uint32(Alert.Field58));
                    WriteFloat(AlertOffsets.Field5C, Alert.Field5C);
                    for (int32 Item = 0; Item < Alert.TargetSwitch.Targets.Num(); ++Item)
                    {
                        const FKyaSwitchTarget& Target = Alert.TargetSwitch.Targets[Item];
                        const int64 T = AlertOffsets.Switch.Targets + int64(Item) * 28;
                        WriteU32(T, uint32(Target.TargetRef)); WriteU32(T + 4, uint32(Target.CutsceneId));
                        WriteU32(T + 8, uint32(Target.Flags)); WriteU32(T + 12, uint32(Target.MessageId));
                        WriteU32(T + 16, uint32(Target.MessageFlags)); WriteU32(T + 20, uint32(Target.Field14));
                        WriteU32(T + 24, uint32(Target.Field18));
                    }
                    {
                        const FKyaStreamEventCamera& Camera = Alert.TargetSwitch.Camera;
                        const int64 C = AlertOffsets.Switch.Camera;
                        WriteU32(C, uint32(Camera.CameraIndex)); WriteU32(C + 4, uint32(Camera.Flags));
                        WriteFloat(C + 8, Camera.Field8); WriteFloat(C + 12, Camera.FieldC); WriteFloat(C + 16, Camera.Field10);
                        WriteU32(C + 20, uint32(Camera.ActorRef)); WriteU32(C + 24, uint32(Camera.ZoneRef));
                        WriteFloat(C + 28, Camera.Field1C);
                    }
                    for (int32 Block = 0; Block < Alert.Field70.Num(); ++Block)
                    {
                        const FKyaConditionedOperation& Result = Alert.Field70[Block];
                        const FKyaConditionedOffsets& BlockOffsets = AlertOffsets.Blocks[Block];
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
            }
            if (Actor.Behaviours[Behaviour].bHasTypedBridgeBasicBehaviour)
            {
                const FKyaActorBehaviour& Source = Actor.Behaviours[Behaviour];
                const int64 P = BehaviourOffset.Payload;
                for (int32 Axis = 0; Axis < 3; ++Axis) WriteFloat(P + Axis * 4, float(Source.BridgeBasicDirection[Axis]));
                WriteFloat(P + 12, Source.BridgeBasicField40);
                WriteFloat(P + 16, Source.BridgeBasicField44);
                WriteFloat(P + 20, Source.BridgeBasicField48);
                WriteFloat(P + 24, Source.BridgeBasicField4C);
                WriteFloat(P + 28, Source.BridgeBasicField50);
            }
            if (Actor.Behaviours[Behaviour].bHasTypedBridgeWindBehaviour)
            {
                const FKyaActorBehaviour& Source = Actor.Behaviours[Behaviour];
                for (int32 Item = 0; Item < Source.BridgeWindValues.Num(); ++Item)
                    WriteFloat(BehaviourOffset.Payload + Item * 4, Source.BridgeWindValues[Item]);
            }
            if (Actor.Behaviours[Behaviour].bHasTypedClusterZones)
            {
                TArray<FKyaClusterZoneOffsets> Zones;
                if (!KyaReadClusterZoneOffsets(MakeArrayView(Bank->BankBytes.GetData(), int32(SceneEnd)), BehaviourOffset.Payload, BehaviourOffset.Size, Zones)) return false;
                for (int32 ZoneIndex = 0; ZoneIndex < Zones.Num(); ++ZoneIndex)
                {
                    const FKyaClusterZone& Zone = Actor.Behaviours[Behaviour].ClusterZones[ZoneIndex];
                    const FKyaClusterZoneOffsets& Location = Zones[ZoneIndex];
                    WriteU32(Location.Header, uint32(Zone.Flags));
                    WriteU32(Location.Header + 4, uint32(Zone.ZoneRef));
                    const TArray<int32>* Lists[] = { &Zone.Actors, &Zone.OtherActors, &Zone.Cameras, &Zone.Lights };
                    for (int32 List = 0; List < 4; ++List)
                        for (int32 Item = 0; Item < Lists[List]->Num(); ++Item)
                            WriteU32(Location.Lists[List] + Item * 4, uint32((*Lists[List])[Item]));
                    for (int32 Item = 0; Item < Zone.Hashes.Num(); ++Item)
                    {
                        const uint64 Hash = uint64(Zone.Hashes[Item]);
                        WriteU32(Location.Lists[4] + int64(Item) * 8, uint32(Hash));
                        WriteU32(Location.Lists[4] + int64(Item) * 8 + 4, uint32(Hash >> 32));
                    }
                    WriteU32(Location.Tail, uint32(Zone.OtherZoneRef));
                }
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
        FKyaWindOffsets Wind;
        const bool HasWind = Actor.ClassId == 0x12 && KyaReadWindOffsets(
            MakeArrayView(Bank->BankBytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, Wind);
        FKyaTrapOffsets Trap;
        const bool HasTrap = Actor.ClassId == 0x2c && KyaReadTrapOffsets(
            MakeArrayView(Bank->BankBytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, Trap);
        FKyaWeaponOffsets Weapon;
        const bool HasWeapon = Actor.ClassId == 0x20 && KyaReadWeaponOffsets(
            MakeArrayView(Bank->BankBytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, Weapon);
        FKyaSwitchOffsets SwitchData;
        const bool HasSwitch = Actor.ClassId == 0x11 && KyaReadSwitchOffsets(
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
        FKyaBonusFountainOffsets Fountain;
        const bool HasFountain = Actor.ClassId == 0x52 && KyaReadBonusFountainOffsets(
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
        TArray<int64> CheckpointOffsets;
        int64 CheckpointTail = 0;
        const bool HasCheckpoints = Actor.ClassId == 0x1e && KyaReadCheckpointOffsets(
            MakeArrayView(Bank->BankBytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, CheckpointOffsets, &CheckpointTail);
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
        const int64 RawOffset = HasAton ? Aton.Tail : (HasBridge ? Bridge.Tail : (HasCompanion ? Companion.Tail : (HasGravity ? Gravity.Tail : (HasNativCmd ? Command.Tail : (HasCheckpoints ? CheckpointTail : (HasRope ? Rope.Tail : (HasFountain ? Fountain.Tail : (HasRune ? Rune.Tail : (HasStiller ? Stiller.Tail : (HasAmbre ? Ambre.Tail : (HasShoot ? Shoot.Tail : (HasFog ? Fog.Tail : (HasMoney ? Money.Tail : (HasBoxClass ? BoxClass.Tail : (HasDca ? Dca.Tail : (HasTeleporter ? Teleporter.Tail : (HasAmortos ? Amortos.Tail : (HasNativ ? Nativ.Tail : (HasCommander ? Commander.Tail : (HasFruit ? Fruit.Tail : (HasWolfen ? Wolfen.WeaponIndex : (HasMicken ? Micken.Tail : (HasBonus ? Bonus.Tail : (HasSwitch ? SwitchData.Tail : (HasWeapon ? Weapon.Tail : (HasTrap ? Trap.Tail : (HasWind ? Wind.Tail : (HasProjectile ? Projectile.Tail : (HasAraignos ? Araignos.Tail : (HasFx ? Fx.Tail : (HasBox ? Box.Tail : (HasDetails ? Details.Tail : (HasPlatform ? Platform.Suffix : Offsets.ClassData)))))))))))))))))))))))))))))))));
        const int64 SecondPassRawOffset = HasKim ? Kim.Tail : (HasBoomy ? Boomy.Tail : RawOffset);
        if (!Actor.ClassData.IsEmpty()) FMemory::Memcpy(Bytes.GetData() + SecondPassRawOffset, Actor.ClassData.GetData(), Actor.ClassData.Num());
        if (HasPlatform)
        {
            WriteFloat(Platform.Weight, Actor.PlatformWeight);
            for (int32 Item = 0; Item < Actor.PlatformActors.Num(); ++Item)
            {
                WriteU32(Platform.Actors + Item * 8, uint32(Actor.PlatformActors[Item].ActorRef));
                WriteU32(Platform.Actors + Item * 8 + 4, uint32(Actor.PlatformActors[Item].CarryMethod));
            }
            for (int32 Item = 0; Item < Actor.PlatformZones.Num(); ++Item)
                WriteU32(Platform.Zones + Item * 4, uint32(Actor.PlatformZones[Item]));
            for (int32 Zone = 0; Zone < Actor.PlatformNoFrictionZones.Num(); ++Zone)
            {
                const int64 Offset = Platform.NoFrictionZones[Zone];
                WriteU32(Offset, uint32(Actor.PlatformNoFrictionZones[Zone].ZoneRef));
                for (int32 Item = 0; Item < Actor.PlatformNoFrictionZones[Zone].Actors.Num(); ++Item)
                    WriteU32(Offset + 8 + Item * 4, uint32(Actor.PlatformNoFrictionZones[Zone].Actors[Item]));
            }
        }
        if (HasDetails)
        {
            for (int32 Camera = 0; Camera < Actor.PlatformCameras.Num(); ++Camera)
            {
                const FKyaPlatformCamera& C = Actor.PlatformCameras[Camera];
                const int64 Offset = Details.Cameras + int64(Camera) * 72;
                WriteU32(Offset, uint32(C.EventType)); WriteU32(Offset + 4, uint32(C.Flags));
                WriteU32(Offset + 8, uint32(C.TargetRef)); WriteU32(Offset + 12, uint32(C.CutsceneId));
                WriteU32(Offset + 16, uint32(C.TargetFlags)); WriteU32(Offset + 20, uint32(C.MessageId));
                WriteU32(Offset + 24, uint32(C.MessageFlags)); WriteU32(Offset + 28, uint32(C.TargetField14));
                WriteU32(Offset + 32, uint32(C.TargetField18)); WriteU32(Offset + 36, uint32(C.CameraIndex));
                WriteU32(Offset + 40, uint32(C.CameraFlags));
                WriteFloat(Offset + 44, C.Field8); WriteFloat(Offset + 48, C.FieldC); WriteFloat(Offset + 52, C.Field10);
                WriteU32(Offset + 56, uint32(C.ActorRef)); WriteU32(Offset + 60, uint32(C.ZoneRef));
                WriteFloat(Offset + 64, C.Field1C); WriteU32(Offset + 68, uint32(C.SoundRef));
            }
            const FKyaPlatformProperties& P = Actor.PlatformProperties;
            const int64 Offset = Details.Properties;
            WriteU32(Offset, uint32(P.SoundRefA)); WriteU32(Offset + 4, uint32(P.SampleRefA));
            WriteU32(Offset + 8, uint32(P.SoundRefB)); WriteU32(Offset + 12, uint32(P.SampleRefB));
            for (int32 Axis = 0; Axis < 3; ++Axis) WriteFloat(Offset + 16 + Axis * 4, float(P.Offset[Axis]));
            WriteU32(Offset + 28, uint32(P.EffectRef1C)); WriteU32(Offset + 32, uint32(P.EffectRef20));
            WriteU32(Offset + 36, uint32(P.PlatformFlags)); WriteU32(Offset + 40, uint32(P.CameraIndex));
            WriteFloat(Offset + 44, P.TiltValue); WriteU32(Offset + 48, uint32(P.TiltMode));
            for (int32 Axis = 0; Axis < 4; ++Axis) WriteFloat(Offset + 52 + Axis * 4, float(P.TiltAndPush[Axis]));
            WriteFloat(Offset + 68, float(P.PushOscillation.X)); WriteFloat(Offset + 72, float(P.PushOscillation.Y));
            const TArray<FKyaPlatformSound>* Lists[] = { &Actor.PlatformSoundsA, &Actor.PlatformSoundsB };
            for (int32 List = 0; List < 2; ++List)
                for (int32 Item = 0; Item < Lists[List]->Num(); ++Item)
                {
                    const FKyaPlatformSound& Sound = (*Lists[List])[Item];
                    const int64 SoundOffset = Details.Sounds[List] + int64(Item) * 16;
                    WriteU32(SoundOffset, uint32(Sound.SegmentStart)); WriteU32(SoundOffset + 4, uint32(Sound.SegmentEnd));
                    WriteU32(SoundOffset + 8, uint32(Sound.SoundRef)); WriteU32(SoundOffset + 12, uint32(Sound.SampleRef));
                }
        }
        if (HasBox)
        {
            WriteU32(Box.Field160, uint32(Actor.BasicBoxField160));
            WriteU32(Box.LifeMax, uint32(Actor.BasicBoxLifeMax));
            for (int32 Item = 0; Item < Actor.BasicBoxActorRefs.Num(); ++Item)
                WriteU32(Box.Actors + Item * 4, uint32(Actor.BasicBoxActorRefs[Item]));
            WriteFloat(Box.Vibration, Actor.BasicBoxVibration0);
            WriteFloat(Box.Vibration + 4, Actor.BasicBoxVibration1);
            WriteFloat(Box.Vibration + 8, Actor.BasicBoxVibration2);
            WriteFloat(Box.Vibration + 12, Actor.BasicBoxVibration3);
            WriteU32(Box.Field180, uint32(Actor.BasicBoxField180));
            WriteU32(Box.Fields, uint32(Actor.BasicBoxField18C));
            WriteU32(Box.Fields + 4, uint32(Actor.BasicBoxField190));
            WriteU32(Box.Fields + 8, uint32(Actor.BasicBoxField198));
            WriteU32(Box.Fields + 12, uint32(Actor.BasicBoxField194));
            for (int32 Item = 0; Item < Actor.BasicBoxSwitch.Targets.Num(); ++Item)
            {
                const FKyaSwitchTarget& Target = Actor.BasicBoxSwitch.Targets[Item];
                const int64 T = Box.SwitchTargets + int64(Item) * 28;
                WriteU32(T, uint32(Target.TargetRef)); WriteU32(T + 4, uint32(Target.CutsceneId));
                WriteU32(T + 8, uint32(Target.Flags)); WriteU32(T + 12, uint32(Target.MessageId));
                WriteU32(T + 16, uint32(Target.MessageFlags)); WriteU32(T + 20, uint32(Target.Field14));
                WriteU32(T + 24, uint32(Target.Field18));
            }
            const FKyaStreamEventCamera& EventCamera = Actor.BasicBoxSwitch.Camera;
            const int64 C = Box.SwitchCamera;
            WriteU32(C, uint32(EventCamera.CameraIndex)); WriteU32(C + 4, uint32(EventCamera.Flags));
            WriteFloat(C + 8, EventCamera.Field8); WriteFloat(C + 12, EventCamera.FieldC); WriteFloat(C + 16, EventCamera.Field10);
            WriteU32(C + 20, uint32(EventCamera.ActorRef)); WriteU32(C + 24, uint32(EventCamera.ZoneRef));
            WriteFloat(C + 28, EventCamera.Field1C);
            const FKyaAddOnGenerator& AddOn = Actor.BasicBoxAddOn;
            const int64 A = Box.AddOn;
            WriteU32(A, uint32(AddOn.Field0)); WriteU32(A + 4, uint32(AddOn.MoneyType));
            WriteU32(A + 8, uint32(AddOn.MoneyCount)); WriteU32(A + 12, uint32(AddOn.BonusCount));
            WriteU32(A + 16, uint32(AddOn.FieldC)); WriteU32(Box.SubObjMode, uint32(AddOn.SubObjMode));
            if (Box.SubObjFloats)
            {
                WriteFloat(Box.SubObjFloats, AddOn.SubObjField0);
                WriteFloat(Box.SubObjFloats + 4, AddOn.SubObjField4);
                WriteFloat(Box.SubObjFloats + 8, AddOn.SubObjFieldC);
                WriteFloat(Box.SubObjFloats + 12, AddOn.SubObjField10);
                WriteFloat(Box.SubObjFloats + 16, AddOn.SubObjField14);
            }
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
            WriteU32(Wolfen.FieldB74, uint32(Actor.WolfenFieldB74));
            WriteFloat(Wolfen.Vision, Actor.WolfenVisionRange);
            WriteFloat(Wolfen.Vision + 4, Actor.WolfenVisionFov);
            WriteFloat(Wolfen.Vision + 8, Actor.WolfenVisionField38);
            WriteFloat(Wolfen.Vision + 12, Actor.WolfenVisionField3C);
            WriteU32(Wolfen.Vision + 16, uint32(Actor.WolfenVisionFlags));
            WriteU32(Wolfen.StartSector, uint32(Actor.WolfenStartSectorId));
            WriteFloat(Wolfen.Hearing, Actor.WolfenHearingRangeSquared);
            WriteFloat(Wolfen.Hearing + 4, Actor.WolfenHearingMaxHeight);
            WriteFloat(Wolfen.VisionDetection, Actor.WolfenVisionDetectionRange);
            WriteFloat(Wolfen.FieldCF4, Actor.WolfenFieldCF4);
            WriteFloat(Wolfen.WalkValues, Actor.WolfenWalkSpeed);
            WriteFloat(Wolfen.WalkValues + 4, Actor.WolfenWalkAcceleration);
            WriteFloat(Wolfen.WalkValues + 8, Actor.WolfenWalkRotSpeed);
            WriteFloat(Wolfen.WalkValues + 12, Actor.WolfenRunSpeedScale);
            WriteFloat(Wolfen.WalkValues + 16, Actor.WolfenDefaultRunSpeed);
            WriteFloat(Wolfen.WalkValues + 20, Actor.WolfenRunAcceleration);
            WriteFloat(Wolfen.WalkValues + 24, Actor.WolfenRotRunSpeed);
            WriteU32(Wolfen.MagicRequired, uint32(Actor.WolfenMagicRequired));
            WriteU32(Wolfen.FieldD30, uint32(Actor.WolfenFieldD30));
            WriteFloat(Wolfen.FieldB90, Actor.WolfenFieldB90);
            WriteFloat(Wolfen.FieldB90 + 4, Actor.WolfenFieldB94);
            WriteFloat(Wolfen.FieldB90 + 8, Actor.WolfenFieldB98);
            const FKyaWolfenFightParam& Params = Actor.WolfenFightParamData;
            for (int32 FieldIndex = 0; FieldIndex < Params.Capability0Fields.Num(); ++FieldIndex)
                WriteU32(FightParam.Capability0Fields + FieldIndex * 4, uint32(Params.Capability0Fields[FieldIndex]));
            for (int32 FieldIndex = 0; FieldIndex < Params.Capability2Fields.Num(); ++FieldIndex)
                WriteU32(FightParam.Capability2Fields + FieldIndex * 4, uint32(Params.Capability2Fields[FieldIndex]));
            for (int32 FieldIndex = 0; FieldIndex < Params.ConfigurationValues.Num(); ++FieldIndex)
                WriteFloat(FightParam.Floats + FieldIndex * 4, Params.ConfigurationValues[FieldIndex]);
            for (int32 Cap = 0; Cap < Params.Capabilities.Num(); ++Cap)
            {
                const FKyaWolfenCapability& Capability = Params.Capabilities[Cap];
                const FKyaWolfenCapabilityOffsets& CapOffsets = FightParam.Capabilities[Cap];
                for (int32 ItemIndex = 0; ItemIndex < Capability.Items.Num(); ++ItemIndex)
                {
                    const FKyaWolfenCapabilityItem& Item = Capability.Items[ItemIndex];
                    const FKyaWolfenCapabilityItemOffsets& ItemOffsets = CapOffsets.Items[ItemIndex];
                    if (Item.Chains.Num() != ItemOffsets.ChainCount) return false;
                    for (int32 Chain = 0; Chain < Item.Chains.Num(); ++Chain)
                        WriteU32(ItemOffsets.Chains + Chain * 4, uint32(Item.Chains[Chain]));
                    WriteFloat(ItemOffsets.Floats, Item.Field0);
                    WriteFloat(ItemOffsets.Floats + 4, Item.Field4);
                }
            }
            for (int32 FieldIndex = 0; FieldIndex < Params.ComboList1.Num(); ++FieldIndex)
                WriteU32(FightParam.ComboList1Data + FieldIndex * 4, uint32(Params.ComboList1[FieldIndex]));
            for (int32 FieldIndex = 0; FieldIndex < Params.ComboList2.Num(); ++FieldIndex)
                WriteU32(FightParam.ComboList2Data + FieldIndex * 4, uint32(Params.ComboList2[FieldIndex]));
            WriteU32(FightParam.Trailing, uint32(Params.FieldAfterLists));
            WriteU32(FightParam.Trailing + 4, uint32(Params.FieldB44));
            for (int32 FieldIndex = 0; FieldIndex < Params.TuningValues.Num(); ++FieldIndex)
                WriteFloat(FightParam.Trailing + 8 + FieldIndex * 4, Params.TuningValues[FieldIndex]);
            for (int32 FieldIndex = 0; FieldIndex < Params.Knowledge.Num(); ++FieldIndex)
                WriteU32(FightParam.Trailing + 32 + FieldIndex * 4, uint32(Params.Knowledge[FieldIndex]));
        }
        if (HasNativ)
        {
            WriteFloat(Nativ.WeightB, Actor.NativWeightB);
            WriteU32(Nativ.Autonomous, uint32(Actor.NativLifeMax));
            WriteFloat(Nativ.Autonomous + 4, Actor.NativGravityScale);
            WriteFloat(Nativ.Autonomous + 8, Actor.NativDynamicExtField4);
            WriteFloat(Nativ.Autonomous + 12, Actor.NativDynamicExtField8);
            WriteU32(Nativ.Field350, uint32(Actor.NativField350));
            WriteU32(Nativ.Field354, uint32(Actor.NativField354));
            WriteU32(Nativ.Field358, uint32(Actor.NativField358));
            WriteFloat(Nativ.MoveSpeedNoHeld, Actor.NativMoveSpeedNoHeld);
            WriteFloat(Nativ.MoveSpeedHeld, Actor.NativMoveSpeedHeld);
            WriteU32(Nativ.Field370, uint32(Actor.NativField370));
            WriteU32(Nativ.Field368, uint32(Actor.NativField368));
            WriteU32(Nativ.Field364, uint32(Actor.NativField364));
            WriteU32(Nativ.Field378, uint32(Actor.NativField378));
            WriteU32(Nativ.Field37C, uint32(Actor.NativField37C));
            WriteU32(Nativ.Field380, uint32(Actor.NativField380));
        }
        if (HasAmortos)
        {
            for (int32 Stream = 0; Stream < 3; ++Stream)
            {
                const TArray<int32>& List = Stream == 0 ? Actor.AmortosCheckpointActors :
                    (Stream == 1 ? Actor.AmortosActors188 : Actor.AmortosActors18C);
                for (int32 Item = 0; Item < List.Num(); ++Item)
                    WriteU32(Amortos.Streams[Stream].Entries + Item * 4, uint32(List[Item]));
            }
            WriteU32(Amortos.Field190, uint32(Actor.AmortosActorRef));
            WriteFloat(Amortos.Floats160, Actor.AmortosField160);
            WriteFloat(Amortos.Floats160 + 4, Actor.AmortosField164);
            WriteFloat(Amortos.Floats160 + 8, Actor.AmortosField168);
            WriteFloat(Amortos.Floats160 + 12, Actor.AmortosField16C);
            WriteFloat(Amortos.Floats160 + 16, Actor.AmortosField170);
            WriteFloat(Amortos.Floats160 + 20, Actor.AmortosField174);
            WriteFloat(Amortos.Floats160 + 24, Actor.AmortosField178);
            WriteFloat(Amortos.Floats160 + 28, Actor.AmortosField17C);
            WriteFloat(Amortos.Floats160 + 32, Actor.AmortosField180);
            const FKyaAddOnGenerator& AddOn = Actor.AmortosAddOn;
            const int64 A = Amortos.AddOn;
            WriteU32(A, uint32(AddOn.Field0)); WriteU32(A + 4, uint32(AddOn.MoneyType));
            WriteU32(A + 8, uint32(AddOn.MoneyCount)); WriteU32(A + 12, uint32(AddOn.BonusCount));
            WriteU32(A + 16, uint32(AddOn.FieldC)); WriteU32(Amortos.SubObjMode, uint32(AddOn.SubObjMode));
            if (Amortos.SubObjFloats)
            {
                WriteFloat(Amortos.SubObjFloats, AddOn.SubObjField0);
                WriteFloat(Amortos.SubObjFloats + 4, AddOn.SubObjField4);
                WriteFloat(Amortos.SubObjFloats + 8, AddOn.SubObjFieldC);
                WriteFloat(Amortos.SubObjFloats + 12, AddOn.SubObjField10);
                WriteFloat(Amortos.SubObjFloats + 16, AddOn.SubObjField14);
            }
            WriteU32(Amortos.Field1C8, uint32(Actor.AmortosField1C8));
            for (int32 SwitchIndex = 0; SwitchIndex < 2; ++SwitchIndex)
            {
                const FKyaSwitch& Switch = SwitchIndex == 0 ? Actor.AmortosSwitch1CC : Actor.AmortosSwitch1D4;
                const FKyaWindSwitchOffsets& SwitchOffsets = Amortos.Switches[SwitchIndex];
                for (int32 Item = 0; Item < Switch.Targets.Num(); ++Item)
                {
                    const FKyaSwitchTarget& Target = Switch.Targets[Item];
                    const int64 T = SwitchOffsets.Targets + int64(Item) * 28;
                    WriteU32(T, uint32(Target.TargetRef)); WriteU32(T + 4, uint32(Target.CutsceneId));
                    WriteU32(T + 8, uint32(Target.Flags)); WriteU32(T + 12, uint32(Target.MessageId));
                    WriteU32(T + 16, uint32(Target.MessageFlags)); WriteU32(T + 20, uint32(Target.Field14));
                    WriteU32(T + 24, uint32(Target.Field18));
                }
                const FKyaStreamEventCamera& Camera = Switch.Camera;
                const int64 C = SwitchOffsets.Camera;
                WriteU32(C, uint32(Camera.CameraIndex)); WriteU32(C + 4, uint32(Camera.Flags));
                WriteFloat(C + 8, Camera.Field8); WriteFloat(C + 12, Camera.FieldC); WriteFloat(C + 16, Camera.Field10);
                WriteU32(C + 20, uint32(Camera.ActorRef)); WriteU32(C + 24, uint32(Camera.ZoneRef));
                WriteFloat(C + 28, Camera.Field1C);
            }
        }
        if (HasTeleporter)
        {
            WriteU32(Teleporter.SubsectorMaterialId, uint32(Actor.TeleporterSubsectorMaterialId));
            WriteU32(Teleporter.Hash, uint32(uint64(Actor.TeleporterHash)));
            WriteU32(Teleporter.Hash + 4, uint32(uint64(Actor.TeleporterHash) >> 32));
            WriteU32(Teleporter.Field16C, uint32(Actor.TeleporterField16C));
            for (int32 Item = 0; Item < Actor.TeleporterDestinations.Num(); ++Item)
            {
                const FKyaTeleporterDestination& Destination = Actor.TeleporterDestinations[Item];
                const int64 D = Teleporter.DestEntries + int64(Item) * 12;
                WriteU32(D, uint32(Destination.LevelId));
                WriteU32(D + 4, uint32(Destination.ElevatorId));
                WriteU32(D + 8, uint32(Destination.Field8));
            }
            const FKyaSwitch* Switches[] = { &Actor.TeleporterSwitch170, &Actor.TeleporterSwitch178, &Actor.TeleporterSwitch180 };
            for (int32 SwitchIndex = 0; SwitchIndex < 3; ++SwitchIndex)
            {
                const FKyaSwitch& Switch = *Switches[SwitchIndex];
                const FKyaWindSwitchOffsets& SwitchOffsets = Teleporter.Switches[SwitchIndex];
                for (int32 Item = 0; Item < Switch.Targets.Num(); ++Item)
                {
                    const FKyaSwitchTarget& Target = Switch.Targets[Item];
                    const int64 T = SwitchOffsets.Targets + int64(Item) * 28;
                    WriteU32(T, uint32(Target.TargetRef)); WriteU32(T + 4, uint32(Target.CutsceneId));
                    WriteU32(T + 8, uint32(Target.Flags)); WriteU32(T + 12, uint32(Target.MessageId));
                    WriteU32(T + 16, uint32(Target.MessageFlags)); WriteU32(T + 20, uint32(Target.Field14));
                    WriteU32(T + 24, uint32(Target.Field18));
                }
                const FKyaStreamEventCamera& Camera = Switch.Camera;
                const int64 C = SwitchOffsets.Camera;
                WriteU32(C, uint32(Camera.CameraIndex)); WriteU32(C + 4, uint32(Camera.Flags));
                WriteFloat(C + 8, Camera.Field8); WriteFloat(C + 12, Camera.FieldC); WriteFloat(C + 16, Camera.Field10);
                WriteU32(C + 20, uint32(Camera.ActorRef)); WriteU32(C + 24, uint32(Camera.ZoneRef));
                WriteFloat(C + 28, Camera.Field1C);
            }
            WriteU32(Teleporter.Zone188, uint32(Actor.TeleporterZone188));
            WriteU32(Teleporter.Zone18C, uint32(Actor.TeleporterZone18C));
            WriteU32(Teleporter.Material1, uint32(Actor.TeleporterMaterialId1));
            WriteU32(Teleporter.CinematicId, uint32(Actor.TeleporterCinematicId));
            WriteU32(Teleporter.Material2, uint32(Actor.TeleporterMaterialId2));
            WriteU32(Teleporter.Field1D4, uint32(Actor.TeleporterField1D4));
            WriteU32(Teleporter.Field1D8, uint32(Actor.TeleporterField1D8));
            WriteU32(Teleporter.Material3, uint32(Actor.TeleporterMaterialId3));
            WriteU32(Teleporter.Field1E0, uint32(Actor.TeleporterField1E0));
            for (int32 Item = 0; Item < Actor.TeleporterWaypoints.Num(); ++Item)
            {
                WriteU32(Teleporter.WaypointEntries + Item * 8, uint32(Actor.TeleporterWaypoints[Item].ActorRef));
                WriteU32(Teleporter.WaypointEntries + Item * 8 + 4, uint32(Actor.TeleporterWaypoints[Item].WaypointRef));
            }
            for (int32 BlockIndex = 0; BlockIndex < Actor.TeleporterConditionedOperations.Num(); ++BlockIndex)
            {
                const FKyaTeleporterBlock& Block = Actor.TeleporterConditionedOperations[BlockIndex];
                for (int32 Op = 0; Op < Block.Operations.Num(); ++Op)
                {
                    const FKyaTeleporterOperation& Operation = Block.Operations[Op];
                    const int64 O = Teleporter.BlockOps[BlockIndex] + int64(Op) * 12;
                    WriteU32(O, uint32(Operation.ScenaricVariable));
                    WriteU32(O + 4, uint32(Operation.Operation));
                    WriteU32(O + 8, uint32(Operation.Value));
                }
                for (int32 Cond = 0; Cond < Block.Conditions.Num(); ++Cond)
                {
                    const FKyaTeleporterCondition& Condition = Block.Conditions[Cond];
                    const int64 C = Teleporter.BlockConds[BlockIndex] + int64(Cond) * 16;
                    WriteU32(C, uint32(Condition.Combine));
                    WriteU32(C + 4, uint32(Condition.Variable));
                    WriteU32(C + 8, uint32(Condition.Comparison));
                    WriteU32(C + 12, uint32(Condition.Value));
                }
            }
        }
        if (HasDca)
        {
            WriteU32(Dca.Field504, uint32(Actor.DcaField504));
            WriteU32(Dca.Field500, uint32(Actor.DcaField500));
            WriteFloat(Dca.Field464, Actor.DcaField464);
            WriteU32(Dca.Field508, uint32(Actor.DcaField508));
            WriteU32(Dca.Field50C, uint32(Actor.DcaField50C));
            WriteFloat(Dca.AimAdjust, Actor.DcaAimAdjustDegrees);
            for (int32 Item = 0; Item < Actor.DcaShotActorRefs.Num(); ++Item)
                WriteU32(Dca.ShotEntries + Item * 4, uint32(Actor.DcaShotActorRefs[Item]));
            WriteU32(Dca.Field450, uint32(Actor.DcaField450));
            WriteU32(Dca.Field454, uint32(Actor.DcaField454));
        }
        if (HasBoxClass)
        {
            WriteFloat(BoxClass.WeightB, Actor.BoxWeightB);
            WriteU32(BoxClass.Autonomous, uint32(Actor.BoxLifeMax));
            WriteFloat(BoxClass.Autonomous + 4, Actor.BoxGravityScale);
            WriteFloat(BoxClass.Autonomous + 8, Actor.BoxDynamicExtField4);
            WriteFloat(BoxClass.Autonomous + 12, Actor.BoxDynamicExtField8);
            WriteU32(BoxClass.Field350, uint32(Actor.BoxField350));
            for (int32 Item = 0; Item < Actor.BoxActorRefs.Num(); ++Item)
                WriteU32(BoxClass.ActorEntries + Item * 4, uint32(Actor.BoxActorRefs[Item]));
            WriteFloat(BoxClass.Floats354, Actor.BoxField354);
            WriteFloat(BoxClass.Floats354 + 4, Actor.BoxField358);
            WriteFloat(BoxClass.Floats354 + 8, Actor.BoxField35C);
            WriteFloat(BoxClass.Floats354 + 12, Actor.BoxField360);
            WriteU32(BoxClass.Field364, uint32(Actor.BoxField364));
            WriteU32(BoxClass.Field370, uint32(Actor.BoxField370));
            WriteU32(BoxClass.Field370 + 4, uint32(Actor.BoxField374));
            WriteU32(BoxClass.Field370 + 8, uint32(Actor.BoxField378));
            WriteU32(BoxClass.Field370 + 12, uint32(Actor.BoxField37C));
            for (int32 Item = 0; Item < Actor.BoxSwitch.Targets.Num(); ++Item)
            {
                const FKyaSwitchTarget& Target = Actor.BoxSwitch.Targets[Item];
                const int64 T = BoxClass.Switch.Targets + int64(Item) * 28;
                WriteU32(T, uint32(Target.TargetRef)); WriteU32(T + 4, uint32(Target.CutsceneId));
                WriteU32(T + 8, uint32(Target.Flags)); WriteU32(T + 12, uint32(Target.MessageId));
                WriteU32(T + 16, uint32(Target.MessageFlags)); WriteU32(T + 20, uint32(Target.Field14));
                WriteU32(T + 24, uint32(Target.Field18));
            }
            {
                const FKyaStreamEventCamera& Camera = Actor.BoxSwitch.Camera;
                const int64 C = BoxClass.Switch.Camera;
                WriteU32(C, uint32(Camera.CameraIndex)); WriteU32(C + 4, uint32(Camera.Flags));
                WriteFloat(C + 8, Camera.Field8); WriteFloat(C + 12, Camera.FieldC); WriteFloat(C + 16, Camera.Field10);
                WriteU32(C + 20, uint32(Camera.ActorRef)); WriteU32(C + 24, uint32(Camera.ZoneRef));
                WriteFloat(C + 28, Camera.Field1C);
            }
            const FKyaAddOnGenerator& AddOn = Actor.BoxAddOn;
            const int64 A = BoxClass.AddOn;
            WriteU32(A, uint32(AddOn.Field0)); WriteU32(A + 4, uint32(AddOn.MoneyType));
            WriteU32(A + 8, uint32(AddOn.MoneyCount)); WriteU32(A + 12, uint32(AddOn.BonusCount));
            WriteU32(A + 16, uint32(AddOn.FieldC)); WriteU32(BoxClass.SubObjMode, uint32(AddOn.SubObjMode));
            if (BoxClass.SubObjFloats)
            {
                WriteFloat(BoxClass.SubObjFloats, AddOn.SubObjField0);
                WriteFloat(BoxClass.SubObjFloats + 4, AddOn.SubObjField4);
                WriteFloat(BoxClass.SubObjFloats + 8, AddOn.SubObjFieldC);
                WriteFloat(BoxClass.SubObjFloats + 12, AddOn.SubObjField10);
                WriteFloat(BoxClass.SubObjFloats + 16, AddOn.SubObjField14);
            }
        }
        if (HasMoney)
        {
            WriteFloat(Money.WeightB, Actor.MoneyWeightB);
            WriteU32(Money.MoneyValue, uint32(Actor.MoneyValue));
            WriteU32(Money.Sound, uint32(Actor.MoneySoundRef));
            WriteU32(Money.Material, uint32(Actor.MoneyMaterialId));
        }
        if (HasFog)
        {
            for (int32 Item = 0; Item < Actor.FogHierarchies.Num(); ++Item)
            {
                const FKyaFogHierarchy& Hierarchy = Actor.FogHierarchies[Item];
                const int64 H = Fog.Entries + int64(Item) * 16;
                WriteU32(H, uint32(uint64(Hierarchy.Hash)));
                WriteU32(H + 4, uint32(uint64(Hierarchy.Hash) >> 32));
                WriteFloat(H + 8, Hierarchy.Field20);
                WriteFloat(H + 12, Hierarchy.Field24);
            }
        }
        if (HasShoot)
        {
            WriteFloat(Shoot.WeightB, Actor.ShootWeightB);
            WriteU32(Shoot.Autonomous, uint32(Actor.ShootLifeMax));
            WriteFloat(Shoot.Autonomous + 4, Actor.ShootGravityScale);
            WriteFloat(Shoot.Autonomous + 8, Actor.ShootDynamicExtField4);
            WriteFloat(Shoot.Autonomous + 12, Actor.ShootDynamicExtField8);
            WriteU32(Shoot.Field350, uint32(Actor.ShootField350));
            WriteU32(Shoot.Field354, uint32(Actor.ShootField354));
            WriteU32(Shoot.Texture, uint32(Actor.ShootTextureIndex));
            WriteU32(Shoot.Mesh, uint32(Actor.ShootMeshIndex));
            WriteFloat(Shoot.Floats3C0, Actor.ShootField3C0);
            WriteFloat(Shoot.Floats3C0 + 4, Actor.ShootField3C4);
            WriteFloat(Shoot.Floats3C0 + 8, Actor.ShootField3C8);
            WriteFloat(Shoot.Floats3C0 + 12, Actor.ShootField3CC);
            WriteU32(Shoot.Field3D0, uint32(Actor.ShootField3D0));
            WriteFloat(Shoot.Floats3D4, Actor.ShootField3D4);
            WriteFloat(Shoot.Floats3D4 + 4, Actor.ShootField3D8);
            WriteFloat(Shoot.Floats3D4 + 8, Actor.ShootField3DC);
            WriteFloat(Shoot.Floats3D4 + 12, Actor.ShootField3E0);
            WriteU32(Shoot.Fields3E8, uint32(Actor.ShootField3E8));
            WriteU32(Shoot.Fields3E8 + 4, uint32(Actor.ShootField3E4));
            WriteU32(Shoot.Fields3E8 + 8, uint32(Actor.ShootField3EC));
            WriteU32(Shoot.Fields3E8 + 12, uint32(Actor.ShootField3F0));
            const FKyaAddOnGenerator& AddOn = Actor.ShootAddOn;
            const int64 A = Shoot.AddOn;
            WriteU32(A, uint32(AddOn.Field0)); WriteU32(A + 4, uint32(AddOn.MoneyType));
            WriteU32(A + 8, uint32(AddOn.MoneyCount)); WriteU32(A + 12, uint32(AddOn.BonusCount));
            WriteU32(A + 16, uint32(AddOn.FieldC)); WriteU32(Shoot.SubObjMode, uint32(AddOn.SubObjMode));
            if (Shoot.SubObjFloats)
            {
                WriteFloat(Shoot.SubObjFloats, AddOn.SubObjField0);
                WriteFloat(Shoot.SubObjFloats + 4, AddOn.SubObjField4);
                WriteFloat(Shoot.SubObjFloats + 8, AddOn.SubObjFieldC);
                WriteFloat(Shoot.SubObjFloats + 12, AddOn.SubObjField10);
                WriteFloat(Shoot.SubObjFloats + 16, AddOn.SubObjField14);
            }
        }
        if (HasAmbre)
        {
            WriteU32(Ambre.Material, uint32(Actor.AmbreMaterialId));
            WriteU32(Ambre.EffectType, uint32(Actor.AmbreEffectType));
            WriteU32(Ambre.Texture, uint32(Actor.AmbreTextureIndex));
            WriteU32(Ambre.Mesh, uint32(Actor.AmbreMeshIndex));
            WriteU32(Ambre.Field1D0, uint32(Actor.AmbreField1D0));
            for (int32 Item = 0; Item < Actor.AmbreSwitch.Targets.Num(); ++Item)
            {
                const FKyaSwitchTarget& Target = Actor.AmbreSwitch.Targets[Item];
                const int64 T = Ambre.Switch.Targets + int64(Item) * 28;
                WriteU32(T, uint32(Target.TargetRef)); WriteU32(T + 4, uint32(Target.CutsceneId));
                WriteU32(T + 8, uint32(Target.Flags)); WriteU32(T + 12, uint32(Target.MessageId));
                WriteU32(T + 16, uint32(Target.MessageFlags)); WriteU32(T + 20, uint32(Target.Field14));
                WriteU32(T + 24, uint32(Target.Field18));
            }
            {
                const FKyaStreamEventCamera& Camera = Actor.AmbreSwitch.Camera;
                const int64 C = Ambre.Switch.Camera;
                WriteU32(C, uint32(Camera.CameraIndex)); WriteU32(C + 4, uint32(Camera.Flags));
                WriteFloat(C + 8, Camera.Field8); WriteFloat(C + 12, Camera.FieldC); WriteFloat(C + 16, Camera.Field10);
                WriteU32(C + 20, uint32(Camera.ActorRef)); WriteU32(C + 24, uint32(Camera.ZoneRef));
                WriteFloat(C + 28, Camera.Field1C);
            }
        }
        if (HasStiller)
        {
            WriteFloat(Stiller.Vision, Actor.StillerVisionRange);
            WriteFloat(Stiller.Vision + 4, Actor.StillerVisionHalfAngle);
            WriteFloat(Stiller.Vision + 8, Actor.StillerVisionField38);
            WriteFloat(Stiller.Vision + 12, Actor.StillerVisionField3C);
            WriteU32(Stiller.Vision + 16, uint32(Actor.StillerVisionFlags));
            WriteU32(Stiller.Field1D0, uint32(Actor.StillerField1D0));
            WriteU32(Stiller.Field1D0 + 4, uint32(Actor.StillerField1D4));
            WriteU32(Stiller.Field1D0 + 8, uint32(Actor.StillerField1D8));
            WriteU32(Stiller.Field1D0 + 12, uint32(Actor.StillerField1DC));
            WriteU32(Stiller.Path, uint32(Actor.StillerPathIndex));
            WriteFloat(Stiller.Field1E0, Actor.StillerField1E0);
            WriteFloat(Stiller.LifeMax, Actor.StillerLifeMax);
            WriteFloat(Stiller.Floats1EC, Actor.StillerField1EC);
            WriteFloat(Stiller.Floats1EC + 4, Actor.StillerField1F0);
            WriteFloat(Stiller.Floats1EC + 8, Actor.StillerField1F4);
            WriteU32(Stiller.Field1F8, uint32(Actor.StillerField1F8));
            for (int32 Item = 0; Item < Actor.StillerActorRefs.Num(); ++Item)
                WriteU32(Stiller.ActorEntries + Item * 4, uint32(Actor.StillerActorRefs[Item]));
            WriteU32(Stiller.Field200, uint32(Actor.StillerField200));
            const FKyaSwitch* Switches[] = { &Actor.StillerSwitch208, &Actor.StillerSwitch210 };
            for (int32 SwitchIndex = 0; SwitchIndex < 2; ++SwitchIndex)
            {
                const FKyaSwitch& Switch = *Switches[SwitchIndex];
                const FKyaWindSwitchOffsets& SwitchOffsets = Stiller.Switches[SwitchIndex];
                for (int32 Item = 0; Item < Switch.Targets.Num(); ++Item)
                {
                    const FKyaSwitchTarget& Target = Switch.Targets[Item];
                    const int64 T = SwitchOffsets.Targets + int64(Item) * 28;
                    WriteU32(T, uint32(Target.TargetRef)); WriteU32(T + 4, uint32(Target.CutsceneId));
                    WriteU32(T + 8, uint32(Target.Flags)); WriteU32(T + 12, uint32(Target.MessageId));
                    WriteU32(T + 16, uint32(Target.MessageFlags)); WriteU32(T + 20, uint32(Target.Field14));
                    WriteU32(T + 24, uint32(Target.Field18));
                }
                const FKyaStreamEventCamera& Camera = Switch.Camera;
                const int64 C = SwitchOffsets.Camera;
                WriteU32(C, uint32(Camera.CameraIndex)); WriteU32(C + 4, uint32(Camera.Flags));
                WriteFloat(C + 8, Camera.Field8); WriteFloat(C + 12, Camera.FieldC); WriteFloat(C + 16, Camera.Field10);
                WriteU32(C + 20, uint32(Camera.ActorRef)); WriteU32(C + 24, uint32(Camera.ZoneRef));
                WriteFloat(C + 28, Camera.Field1C);
            }
            WriteU32(Stiller.Field214, uint32(Actor.StillerField214));
            const FKyaAddOnGenerator& AddOn = Actor.StillerAddOn;
            const int64 A = Stiller.AddOn;
            WriteU32(A, uint32(AddOn.Field0)); WriteU32(A + 4, uint32(AddOn.MoneyType));
            WriteU32(A + 8, uint32(AddOn.MoneyCount)); WriteU32(A + 12, uint32(AddOn.BonusCount));
            WriteU32(A + 16, uint32(AddOn.FieldC)); WriteU32(Stiller.SubObjMode, uint32(AddOn.SubObjMode));
            if (Stiller.SubObjFloats)
            {
                WriteFloat(Stiller.SubObjFloats, AddOn.SubObjField0);
                WriteFloat(Stiller.SubObjFloats + 4, AddOn.SubObjField4);
                WriteFloat(Stiller.SubObjFloats + 8, AddOn.SubObjFieldC);
                WriteFloat(Stiller.SubObjFloats + 12, AddOn.SubObjField10);
                WriteFloat(Stiller.SubObjFloats + 16, AddOn.SubObjField14);
            }
        }
        if (HasRune)
        {
            WriteU32(Rune.Cinematic, uint32(Actor.RuneCinematicId));
            WriteFloat(Rune.TriggerDistance, Actor.RuneTriggerDistance);
            WriteU32(Rune.ScenVar, uint32(Actor.RuneScenVarId));
            WriteU32(Rune.Fx, uint32(Actor.RuneFxId));
            WriteU32(Rune.MaterialA, uint32(Actor.RuneMaterialIdA));
            WriteU32(Rune.MaterialB, uint32(Actor.RuneMaterialIdB));
        }
        if (HasFountain)
        {
            const FKyaAddOnGenerator& AddOn = Actor.BonusFountainAddOn;
            const int64 A = Fountain.AddOn;
            WriteU32(A, uint32(AddOn.Field0)); WriteU32(A + 4, uint32(AddOn.MoneyType));
            WriteU32(A + 8, uint32(AddOn.MoneyCount)); WriteU32(A + 12, uint32(AddOn.BonusCount));
            WriteU32(A + 16, uint32(AddOn.FieldC)); WriteU32(Fountain.SubObjMode, uint32(AddOn.SubObjMode));
            if (Fountain.SubObjFloats)
            {
                WriteFloat(Fountain.SubObjFloats, AddOn.SubObjField0);
                WriteFloat(Fountain.SubObjFloats + 4, AddOn.SubObjField4);
                WriteFloat(Fountain.SubObjFloats + 8, AddOn.SubObjFieldC);
                WriteFloat(Fountain.SubObjFloats + 12, AddOn.SubObjField10);
                WriteFloat(Fountain.SubObjFloats + 16, AddOn.SubObjField14);
            }
        }
        if (HasRope)
        {
            WriteU32(Rope.Path, uint32(Actor.RopePathIndex));
            WriteFloat(Rope.Scale, Actor.RopeScale);
            WriteFloat(Rope.Field17C, Actor.RopeField17C);
            WriteU32(Rope.Material, uint32(Actor.RopeMaterialId));
            WriteU32(Rope.Color, uint32(Actor.RopeColor));
            WriteU32(Rope.Sound, uint32(Actor.RopeSoundRef));
            WriteU32(Rope.Field194, uint32(Actor.RopeField194));
            WriteU32(Rope.ActorA, uint32(Actor.RopeActorRefA));
            WriteU32(Rope.ActorB, uint32(Actor.RopeActorRefB));
            for (int32 Item = 0; Item < Actor.RopeSwitch.Targets.Num(); ++Item)
            {
                const FKyaSwitchTarget& Target = Actor.RopeSwitch.Targets[Item];
                const int64 T = Rope.Switch.Targets + int64(Item) * 28;
                WriteU32(T, uint32(Target.TargetRef)); WriteU32(T + 4, uint32(Target.CutsceneId));
                WriteU32(T + 8, uint32(Target.Flags)); WriteU32(T + 12, uint32(Target.MessageId));
                WriteU32(T + 16, uint32(Target.MessageFlags)); WriteU32(T + 20, uint32(Target.Field14));
                WriteU32(T + 24, uint32(Target.Field18));
            }
            {
                const FKyaStreamEventCamera& Camera = Actor.RopeSwitch.Camera;
                const int64 C = Rope.Switch.Camera;
                WriteU32(C, uint32(Camera.CameraIndex)); WriteU32(C + 4, uint32(Camera.Flags));
                WriteFloat(C + 8, Camera.Field8); WriteFloat(C + 12, Camera.FieldC); WriteFloat(C + 16, Camera.Field10);
                WriteU32(C + 20, uint32(Camera.ActorRef)); WriteU32(C + 24, uint32(Camera.ZoneRef));
                WriteFloat(C + 28, Camera.Field1C);
            }
            WriteU32(Rope.Flags, uint32(Actor.RopeFlags));
        }
        if (HasGravity)
        {
            WriteFloat(Gravity.WeightB, Actor.GravityWeightB);
            WriteU32(Gravity.Autonomous, uint32(Actor.GravityLifeMax));
            WriteFloat(Gravity.Autonomous + 4, Actor.GravityScale);
            WriteFloat(Gravity.Autonomous + 8, Actor.GravityDynamicExtField4);
            WriteFloat(Gravity.Autonomous + 12, Actor.GravityDynamicExtField8);
        }
        if (HasNativCmd)
        {
            for (int32 Item = 0; Item < Actor.NativCmdActorRefs.Num(); ++Item)
                WriteU32(Command.ActorEntries + Item * 4, uint32(Actor.NativCmdActorRefs[Item]));
            WriteU32(Command.Field164, uint32(Actor.NativCmdField164));
            int32 SubObjIndex = 0;
            for (int32 AddOnIndex = 0; AddOnIndex < Actor.NativCmdAddOns.Num(); ++AddOnIndex)
            {
                const FKyaNativCmdAddOn& AddOn = Actor.NativCmdAddOns[AddOnIndex];
                for (int32 Sub = 0; Sub < AddOn.SubObjs.Num(); ++Sub)
                {
                    const FKyaAddOnSubObj& Item = AddOn.SubObjs[Sub];
                    const int64 SubOffset = Command.SubObjOffsets[SubObjIndex++];
                    WriteU32(SubOffset, uint32(Item.Field0));
                    for (int32 Cine = 0; Cine < Item.CinematicIds.Num(); ++Cine)
                        WriteU32(SubOffset + 8 + Cine * 4, uint32(Item.CinematicIds[Cine]));
                }
            }
        }
        if (HasBridge)
        {
            for (int32 Item = 0; Item < Actor.BridgeActorRefs.Num(); ++Item)
                WriteU32(Bridge.ActorEntries + Item * 4, uint32(Actor.BridgeActorRefs[Item]));
            WriteU32(Bridge.WaypointA, uint32(Actor.BridgeWaypointA));
            WriteU32(Bridge.WaypointB, uint32(Actor.BridgeWaypointB));
            WriteFloat(Bridge.Floats330, Actor.BridgeField330);
            WriteFloat(Bridge.Floats330 + 4, Actor.BridgeField334);
            WriteFloat(Bridge.Floats330 + 8, Actor.BridgeField33C);
            WriteFloat(Bridge.Floats330 + 12, Actor.BridgeField340);
            WriteFloat(Bridge.Floats330 + 16, Actor.BridgeField338);
            WriteU32(Bridge.Field344, uint32(Actor.BridgeField344));
            WriteU32(Bridge.ConfigFlags, uint32(Actor.BridgeConfigFlags));
            WriteU32(Bridge.Field348, uint32(Actor.BridgeField348));
            WriteFloat(Bridge.StepCollisionScale, Actor.BridgeStepCollisionScale);
        }
        if (HasCompanion)
        {
            WriteFloat(Companion.WeightB, Actor.CompanionWeightB);
            WriteU32(Companion.Autonomous, uint32(Actor.CompanionLifeMax));
            WriteFloat(Companion.Autonomous + 4, Actor.CompanionGravityScale);
            WriteFloat(Companion.Autonomous + 8, Actor.CompanionDynamicExtField4);
            WriteFloat(Companion.Autonomous + 12, Actor.CompanionDynamicExtField8);
            WriteU32(Companion.Field358, uint32(Actor.CompanionField358));
            WriteU32(Companion.Field358 + 4, uint32(Actor.CompanionMaterial35C));
            WriteU32(Companion.Field358 + 8, uint32(Actor.CompanionField360));
        }
        if (HasAton)
        {
            WriteFloat(Aton.WeightB, Actor.AtonWeightB);
            WriteU32(Aton.Autonomous, uint32(Actor.AtonLifeMax));
            WriteFloat(Aton.Autonomous + 4, Actor.AtonGravityScale);
            WriteFloat(Aton.Autonomous + 8, Actor.AtonDynamicExtField4);
            WriteFloat(Aton.Autonomous + 12, Actor.AtonDynamicExtField8);
            WriteU32(Aton.Fields354, uint32(Actor.AtonField354));
            WriteU32(Aton.Fields354 + 4, uint32(Actor.AtonField358));
            WriteU32(Aton.Fields354 + 8, uint32(Actor.AtonField35C));
            WriteU32(Aton.Fields354 + 12, uint32(Actor.AtonField360));
            WriteU32(Aton.EffectType, uint32(Actor.AtonEffectType));
            WriteU32(Aton.Texture, uint32(Actor.AtonTextureIndex));
            WriteU32(Aton.Mesh, uint32(Actor.AtonMeshIndex));
            WriteU32(Aton.Trail, uint32(Actor.AtonTrailMaterialId));
            WriteFloat(Aton.PathDelay, Actor.AtonPathDefaultDelay);
            for (int32 Item = 0; Item < Actor.AtonPathIndices.Num(); ++Item)
                WriteU32(Aton.PathEntries + Item * 4, uint32(Actor.AtonPathIndices[Item]));
            WriteU32(Aton.AddOnField18, uint32(Actor.AtonAddOnField18));
            for (int32 SubIndex = 0; SubIndex < Actor.AtonOtherSubObjs.Num(); ++SubIndex)
            {
                const FKyaAddOnSubObj& Item = Actor.AtonOtherSubObjs[SubIndex];
                const int64 Offset = Aton.OtherOffsets[SubIndex];
                WriteU32(Offset, uint32(Item.Field0));
                for (int32 Cine = 0; Cine < Item.CinematicIds.Num(); ++Cine)
                    WriteU32(Offset + 8 + Cine * 4, uint32(Item.CinematicIds[Cine]));
            }
            for (int32 SubIndex = 0; SubIndex < Actor.AtonSubObjs.Num(); ++SubIndex)
            {
                const FKyaAtonSubObj& Item = Actor.AtonSubObjs[SubIndex];
                const int64 Offset = Aton.AtonOffsets[Index];
                WriteU32(Offset, uint32(Item.Field18));
                WriteU32(Offset + 4, uint32(Item.Field0));
                for (int32 Cine = 0; Cine < Item.CinematicIds.Num(); ++Cine)
                    WriteU32(Offset + 12 + Cine * 4, uint32(Item.CinematicIds[Cine]));
            }
            WriteFloat(Aton.AddOnField24, Actor.AtonAddOnField24);
        }
        if (HasBoomy)
        {
            WriteFloat(Boomy.Weight, Actor.BoomyWeight);
            WriteFloat(Boomy.Vision, Actor.BoomyVisionRange);
            WriteFloat(Boomy.Vision + 4, Actor.BoomyVisionAngle);
            WriteFloat(Boomy.Vision + 8, Actor.BoomyVisionField38);
            WriteFloat(Boomy.Vision + 12, Actor.BoomyVisionField3C);
            WriteU32(Boomy.Vision + 16, uint32(Actor.BoomyVisionFlags));
            WriteU32(Boomy.Sound2B8, uint32(Actor.BoomySound2B8));
            WriteU32(Boomy.Sound2BC, uint32(Actor.BoomySound2BC));
            WriteFloat(Boomy.Field744, Actor.BoomyField744);
            WriteU32(Boomy.TailWidthAxis, uint32(Actor.BoomyTailWidthAxis));
            WriteFloat(Boomy.Field740, Actor.BoomyField740);
            WriteU32(Boomy.TailSegments, uint32(Actor.BoomyTailSegments));
            WriteU32(Boomy.Particle3D0, uint32(Actor.BoomyParticle3D0));
            WriteU32(Boomy.Particle3D4, uint32(Actor.BoomyParticle3D4));
            WriteU32(Boomy.Particle3DC, uint32(Actor.BoomyParticle3DC));
            WriteU32(Boomy.Field3CC, uint32(Actor.BoomyField3CC));
            WriteU32(Boomy.Texture, uint32(Actor.BoomyTextureIndex));
            WriteU32(Boomy.Mesh, uint32(Actor.BoomyMeshIndex));
            const FKyaActorCamera& Camera = Actor.BoomyCamera;
            WriteFloat(Boomy.CameraFov, Camera.Fov);
            for (int32 Axis = 0; Axis < 3; ++Axis) WriteFloat(Boomy.CameraPosition + Axis * 4, float(Camera.Position[Axis]));
            for (int32 Axis = 0; Axis < 3; ++Axis) WriteFloat(Boomy.CameraLookAt + Axis * 4, float(Camera.LookAt[Axis]));
            for (int32 Axis = 0; Axis < 3; ++Axis) WriteFloat(Boomy.CameraUp + Axis * 4, float(Camera.Up[Axis]));
            WriteU32(Boomy.CameraSector, uint32(Camera.SectorId));
            WriteU32(Boomy.CameraField08, uint32(Camera.Field08));
            WriteU32(Boomy.CameraFlags, uint32(Camera.Flags));
            for (int32 Axis = 0; Axis < 3; ++Axis) WriteFloat(Boomy.CameraTargetOffset + Axis * 4, float(Camera.TargetOffset[Axis]));
            if (Boomy.bHasSwitch)
            {
                WriteU32(Boomy.SwitchActor, uint32(Camera.SwitchActor));
                for (int32 SubIndex = 0; SubIndex < Camera.SwitchActorsA.Num(); ++SubIndex)
                    WriteU32(Boomy.SwitchListA + SubIndex * 4, uint32(Camera.SwitchActorsA[SubIndex]));
                for (int32 SubIndex = 0; SubIndex < Camera.SwitchActorsB.Num(); ++SubIndex)
                    WriteU32(Boomy.SwitchListB + SubIndex * 4, uint32(Camera.SwitchActorsB[SubIndex]));
                WriteFloat(Boomy.SwitchValues, Camera.SwitchValueA);
                WriteU32(Boomy.SwitchValues + 4, uint32(Camera.SwitchModeA));
                WriteFloat(Boomy.SwitchValues + 8, Camera.SwitchValueB);
                WriteU32(Boomy.SwitchValues + 12, uint32(Camera.SwitchModeB));
                WriteFloat(Boomy.SwitchValues + 16, Camera.SwitchValueC);
            }
            for (int32 ConditionIndex = 0; ConditionIndex < Camera.Conditions.Num(); ++ConditionIndex)
            {
                const FKyaCameraCondition& Condition = Camera.Conditions[ConditionIndex];
                const int64 C = Boomy.Conditions + 4 + int64(ConditionIndex) * 16;
                WriteU32(C, uint32(Condition.Combiner));
                WriteU32(C + 4, uint32(Condition.ActorRef));
                WriteU32(C + 8, uint32(Condition.Type));
                WriteFloat(C + 12, Condition.Value);
            }
            WriteU32(Boomy.Config, uint32(Camera.ConfigFlags));
            WriteU32(Boomy.Config + 4, uint32(Camera.ConfigActorRefA));
            WriteU32(Boomy.Config + 8, uint32(Camera.ConfigActorRefB));
            WriteFloat(Boomy.Config + 12, Camera.ConfigAngle);
            for (int32 ValueIndex = 0; ValueIndex < Camera.ConfigOptionalValues.Num(); ++ValueIndex)
                WriteFloat(Boomy.ConfigOptional + ValueIndex * 4, Camera.ConfigOptionalValues[ValueIndex]);
            for (int32 Axis = 0; Axis < 3; ++Axis) WriteFloat(Boomy.ConfigAngles + Axis * 4, float(Camera.PitchAngles[Axis]));
            for (int32 Axis = 0; Axis < 3; ++Axis) WriteFloat(Boomy.ConfigAngles + 12 + Axis * 4, float(Camera.OtherAngles[Axis]));
            for (int32 Axis = 0; Axis < 3; ++Axis) WriteFloat(Boomy.ConfigAngles + 24 + Axis * 4, float(Camera.Distances[Axis]));
            for (int32 ExtraIndex = 0; ExtraIndex < Camera.ExtraValues.Num(); ++ExtraIndex)
                WriteFloat(Boomy.Extra + 4 + ExtraIndex * 4, Camera.ExtraValues[ExtraIndex]);
            for (int32 TypeIndex = 0; TypeIndex < Actor.BoomyTypes.Num(); ++TypeIndex)
            {
                const FKyaBoomyType& Type = Actor.BoomyTypes[TypeIndex];
                const int64 T = Boomy.TypeEntries + int64(TypeIndex) * 16;
                WriteFloat(T, Type.Field4);
                WriteFloat(T + 4, Type.VisionRange);
                WriteU32(T + 8, uint32(Type.HitDamage));
                WriteU32(T + 12, uint32(Type.FxColorA));
            }
        }
        if (HasKim)
        {
            if (!WriteFighterPrefix(Actor, Kim.Fighter)) return false;
            for (int32 KimIndex = 0; KimIndex < Actor.KimBoneKeys.Num(); ++KimIndex)
                WriteU32(Kim.Bones + KimIndex * 4, uint32(Actor.KimBoneKeys[KimIndex]));
            for (int32 KimIndex = 0; KimIndex < Actor.KimClimbZones.Num(); ++KimIndex)
                WriteU32(Kim.ClimbEntries + KimIndex * 4, uint32(Actor.KimClimbZones[KimIndex]));
            for (int32 KimIndex = 0; KimIndex < Actor.KimTutorialZones.Num(); ++KimIndex)
            {
                const FKyaHeroTutorialZone& Zone = Actor.KimTutorialZones[KimIndex];
                const int64 H = Kim.Tutorials[KimIndex].Header;
                WriteU32(H, uint32(Zone.ZoneRef));
                WriteU32(H + 4, uint32(uint64(Zone.MessageKey)));
                WriteU32(H + 8, uint32(uint64(Zone.MessageKey) >> 32));
                WriteU32(H + 12, uint32(Zone.Flags));
                WriteU32(H + 16, uint32(Zone.Action));
                WriteFloat(H + 20, Zone.Duration);
                for (int32 Target = 0; Target < Zone.Targets.Num(); ++Target)
                {
                    const FKyaSwitchTarget& Item = Zone.Targets[Target];
                    const int64 S = Kim.Tutorials[KimIndex].Targets + int64(Target) * 28;
                    WriteU32(S, uint32(Item.TargetRef)); WriteU32(S + 4, uint32(Item.CutsceneId));
                    WriteU32(S + 8, uint32(Item.Flags)); WriteU32(S + 12, uint32(Item.MessageId));
                    WriteU32(S + 16, uint32(Item.MessageFlags)); WriteU32(S + 20, uint32(Item.Field14));
                    WriteU32(S + 24, uint32(Item.Field18));
                }
                const int64 C = Kim.Tutorials[KimIndex].Camera;
                WriteU32(C, uint32(Zone.Camera.CameraIndex)); WriteU32(C + 4, uint32(Zone.Camera.Flags));
                WriteFloat(C + 8, Zone.Camera.Field8); WriteFloat(C + 12, Zone.Camera.FieldC);
                WriteFloat(C + 16, Zone.Camera.Field10); WriteU32(C + 20, uint32(Zone.Camera.ActorRef));
                WriteU32(C + 24, uint32(Zone.Camera.ZoneRef)); WriteFloat(C + 28, Zone.Camera.Field1C);
            }
            for (int32 KimIndex = 0; KimIndex < Actor.KimProjectileZones.Num(); ++KimIndex)
            {
                const FKyaHeroProjectileZone& Zone = Actor.KimProjectileZones[KimIndex];
                const int64 P = Kim.ProjectileEntries + int64(KimIndex) * 28;
                WriteU32(P, uint32(Zone.ZoneRef));
                WriteU32(P + 4, uint32(Zone.ProjectileType));
                WriteFloat(P + 8, Zone.Damage);
                WriteU32(P + 12, uint32(Zone.WaypointIndex));
                WriteFloat(P + 16, Zone.Force);
                WriteU32(P + 20, uint32(Zone.SampleCount));
                WriteU32(P + 24, uint32(Zone.SectorId));
            }
            for (int32 KimIndex = 0; KimIndex < Actor.KimBoomyZonePairs.Num(); ++KimIndex)
            {
                WriteU32(Kim.PairEntries + KimIndex * 8, uint32(Actor.KimBoomyZonePairs[KimIndex].ZoneRef));
                WriteU32(Kim.PairEntries + KimIndex * 8 + 4, uint32(Actor.KimBoomyZonePairs[KimIndex].Field4));
            }
            for (int32 KimIndex = 0; KimIndex < Actor.KimZoneStream.Num(); ++KimIndex)
                WriteU32(Kim.ZoneStreamEntries + KimIndex * 4, uint32(Actor.KimZoneStream[KimIndex]));
            WriteU32(Kim.Material, uint32(Actor.KimMaterialId));
            WriteU32(Kim.FxType, uint32(Actor.KimFxType));
            WriteFloat(Kim.JokeRadius, Actor.KimJokeWarnRadius);
            for (int32 KimIndex = 0; KimIndex < Actor.KimCameras.Num(); ++KimIndex)
            {
                const FKyaActorCamera& Camera = Actor.KimCameras[KimIndex];
                const FKyaActorCameraOffsets& O = Kim.Cameras[KimIndex];
                WriteFloat(O.Fov, Camera.Fov);
                for (int32 Axis = 0; Axis < 3; ++Axis) WriteFloat(O.Position + Axis * 4, float(Camera.Position[Axis]));
                for (int32 Axis = 0; Axis < 3; ++Axis) WriteFloat(O.LookAt + Axis * 4, float(Camera.LookAt[Axis]));
                for (int32 Axis = 0; Axis < 3; ++Axis) WriteFloat(O.Up + Axis * 4, float(Camera.Up[Axis]));
                WriteU32(O.Sector, uint32(Camera.SectorId));
                WriteU32(O.Field08, uint32(Camera.Field08));
                WriteU32(O.Flags, uint32(Camera.Flags));
                for (int32 Axis = 0; Axis < 3; ++Axis) WriteFloat(O.TargetOffset + Axis * 4, float(Camera.TargetOffset[Axis]));
                if (O.bHasSwitch)
                {
                    WriteU32(O.SwitchActor, uint32(Camera.SwitchActor));
                    for (int32 Item = 0; Item < Camera.SwitchActorsA.Num(); ++Item)
                        WriteU32(O.SwitchListA + Item * 4, uint32(Camera.SwitchActorsA[Item]));
                    for (int32 Item = 0; Item < Camera.SwitchActorsB.Num(); ++Item)
                        WriteU32(O.SwitchListB + Item * 4, uint32(Camera.SwitchActorsB[Item]));
                    WriteFloat(O.SwitchValues, Camera.SwitchValueA);
                    WriteU32(O.SwitchValues + 4, uint32(Camera.SwitchModeA));
                    WriteFloat(O.SwitchValues + 8, Camera.SwitchValueB);
                    WriteU32(O.SwitchValues + 12, uint32(Camera.SwitchModeB));
                    WriteFloat(O.SwitchValues + 16, Camera.SwitchValueC);
                }
                for (int32 Item = 0; Item < Camera.Conditions.Num(); ++Item)
                {
                    const FKyaCameraCondition& Condition = Camera.Conditions[Item];
                    const int64 C = O.Conditions + 4 + int64(Item) * 16;
                    WriteU32(C, uint32(Condition.Combiner));
                    WriteU32(C + 4, uint32(Condition.ActorRef));
                    WriteU32(C + 8, uint32(Condition.Type));
                    WriteFloat(C + 12, Condition.Value);
                }
                WriteU32(O.Config, uint32(Camera.ConfigFlags));
                WriteU32(O.Config + 4, uint32(Camera.ConfigActorRefA));
                WriteU32(O.Config + 8, uint32(Camera.ConfigActorRefB));
                WriteFloat(O.Config + 12, Camera.ConfigAngle);
                for (int32 Item = 0; Item < Camera.ConfigOptionalValues.Num(); ++Item)
                    WriteFloat(O.ConfigOptional + Item * 4, Camera.ConfigOptionalValues[Item]);
                for (int32 Axis = 0; Axis < 3; ++Axis) WriteFloat(O.ConfigAngles + Axis * 4, float(Camera.PitchAngles[Axis]));
                for (int32 Axis = 0; Axis < 3; ++Axis) WriteFloat(O.ConfigAngles + 12 + Axis * 4, float(Camera.OtherAngles[Axis]));
                for (int32 Axis = 0; Axis < 3; ++Axis) WriteFloat(O.ConfigAngles + 24 + Axis * 4, float(Camera.Distances[Axis]));
                for (int32 Item = 0; Item < Camera.ExtraValues.Num(); ++Item)
                    WriteFloat(O.Extra + 4 + Item * 4, Camera.ExtraValues[Item]);
            }
            WriteU32(Kim.Field15D4, uint32(Actor.KimField15D4));
            WriteU32(Kim.FxInit, uint32(Actor.KimFxInitType));
            for (int32 KimIndex = 0; KimIndex < Actor.KimFxHandleTypes.Num(); ++KimIndex)
                WriteU32(Kim.FxHandles + KimIndex * 4, uint32(Actor.KimFxHandleTypes[KimIndex]));
        }
        if (HasCommander)
        {
            WriteU32(Commander.Headers, uint32(Actor.CommanderDetectAreaZoneId));
            WriteU32(Commander.Headers + 4, uint32(Actor.CommanderGuardAreaZoneId));
            WriteU32(Commander.Headers + 8, uint32(Actor.CommanderField180));
            WriteU32(Commander.Headers + 12, uint32(Actor.CommanderNewPathFindingId));
            WriteU32(Commander.Headers + 16, uint32(Actor.CommanderActorRef));
            WriteFloat(Commander.Squad, Actor.CommanderSquadValue);
            WriteU32(Commander.Squad + 4, uint32(Actor.CommanderSquadField4));
            WriteU32(Commander.Squad + 8, uint32(Actor.CommanderSquadField8));
            for (int32 Item = 0; Item < Actor.CommanderTeamEnemies.Num(); ++Item)
                WriteU32(Commander.Teams + Item * 4, uint32(Actor.CommanderTeamEnemies[Item]));
            for (int32 Item = 0; Item < Actor.CommanderSequence.Num(); ++Item)
            {
                WriteU32(Commander.Components + Item * 8, uint32(Actor.CommanderSequence[Item].Field0));
                WriteU32(Commander.Components + Item * 8 + 4, uint32(Actor.CommanderSequence[Item].Field8));
            }
            for (int32 Item = 0; Item < Actor.CommanderCameraValues.Num(); ++Item)
                WriteFloat(Commander.Camera + Item * 4, Actor.CommanderCameraValues[Item]);
        }
        if (HasFruit)
        {
            WriteFloat(Fruit.WeightB, Actor.FruitWeightB);
            WriteU32(Fruit.Autonomous, uint32(Actor.FruitLifeMax));
            WriteFloat(Fruit.Autonomous + 4, Actor.FruitGravityScale);
            WriteFloat(Fruit.Autonomous + 8, Actor.FruitDynamicExtField4);
            WriteFloat(Fruit.Autonomous + 12, Actor.FruitDynamicExtField8);
            WriteU32(Fruit.Field360, uint32(Actor.FruitField360));
            FKyaSwitchOffsets Switch;
            if (KyaReadSwitchOffsets(MakeArrayView(Bank->BankBytes.GetData(), int32(SceneEnd)), Fruit.Switch, Offsets.ClassDataSize - 24, Switch))
            {
                for (int32 Item = 0; Item < Actor.FruitSwitch.Targets.Num(); ++Item)
                {
                    const FKyaSwitchTarget& Target = Actor.FruitSwitch.Targets[Item];
                    const int64 T = Switch.Targets + int64(Item) * 28;
                    WriteU32(T, uint32(Target.TargetRef)); WriteU32(T + 4, uint32(Target.CutsceneId));
                    WriteU32(T + 8, uint32(Target.Flags)); WriteU32(T + 12, uint32(Target.MessageId));
                    WriteU32(T + 16, uint32(Target.MessageFlags)); WriteU32(T + 20, uint32(Target.Field14));
                    WriteU32(T + 24, uint32(Target.Field18));
                }
                const FKyaStreamEventCamera& Camera = Actor.FruitSwitch.Camera;
                const int64 C = Switch.Camera;
                WriteU32(C, uint32(Camera.CameraIndex)); WriteU32(C + 4, uint32(Camera.Flags));
                WriteFloat(C + 8, Camera.Field8); WriteFloat(C + 12, Camera.FieldC); WriteFloat(C + 16, Camera.Field10);
                WriteU32(C + 20, uint32(Camera.ActorRef)); WriteU32(C + 24, uint32(Camera.ZoneRef));
                WriteFloat(C + 28, Camera.Field1C);
            }
        }
        if (HasMicken)
        {
            WriteFloat(Micken.WeightB, Actor.MickenWeightB);
            WriteU32(Micken.Autonomous, uint32(Actor.MickenLifeMax));
            WriteFloat(Micken.Autonomous + 4, Actor.MickenGravityScale);
            WriteFloat(Micken.Autonomous + 8, Actor.MickenDynamicExtField4);
            WriteFloat(Micken.Autonomous + 12, Actor.MickenDynamicExtField8);
            WriteU32(Micken.Waypoint, uint32(Actor.MickenWaypointIndex));
            WriteU32(Micken.Actor, uint32(Actor.MickenActorIndex));
            WriteU32(Micken.Field358, uint32(Actor.MickenField358));
            WriteFloat(Micken.Field35C, Actor.MickenField35C);
            WriteU32(Micken.Field360, uint32(Actor.MickenField360));
        }
        if (HasBonus)
        {
            WriteFloat(Bonus.WeightB, Actor.BonusWeightB);
            WriteU32(Bonus.Sound, uint32(Actor.BonusSoundRef));
            WriteU32(Bonus.FlareMaterial, uint32(Actor.BonusFlareMaterialId));
            WriteU32(Bonus.AnimMaterial, uint32(Actor.BonusAnimMaterialId));
        }
        if (HasSwitch)
        {
            for (int32 Item = 0; Item < Actor.SwitchTarget.Targets.Num(); ++Item)
            {
                const FKyaSwitchTarget& Target = Actor.SwitchTarget.Targets[Item];
                const int64 T = SwitchData.Targets + int64(Item) * 28;
                WriteU32(T, uint32(Target.TargetRef)); WriteU32(T + 4, uint32(Target.CutsceneId));
                WriteU32(T + 8, uint32(Target.Flags)); WriteU32(T + 12, uint32(Target.MessageId));
                WriteU32(T + 16, uint32(Target.MessageFlags)); WriteU32(T + 20, uint32(Target.Field14));
                WriteU32(T + 24, uint32(Target.Field18));
            }
            const FKyaStreamEventCamera& Camera = Actor.SwitchTarget.Camera;
            const int64 C = SwitchData.Camera;
            WriteU32(C, uint32(Camera.CameraIndex)); WriteU32(C + 4, uint32(Camera.Flags));
            WriteFloat(C + 8, Camera.Field8); WriteFloat(C + 12, Camera.FieldC); WriteFloat(C + 16, Camera.Field10);
            WriteU32(C + 20, uint32(Camera.ActorRef)); WriteU32(C + 24, uint32(Camera.ZoneRef));
            WriteFloat(C + 28, Camera.Field1C);
        }
        if (HasWeapon)
        {
            WriteFloat(Weapon.WeightB, Actor.WeaponWeightB);
            WriteFloat(Weapon.Field1D8, Actor.WeaponField1D8);
            WriteFloat(Weapon.Field1DC, Actor.WeaponField1DC);
        }
        if (HasTrap)
        {
            WriteFloat(Trap.WeightB, Actor.TrapWeightB);
            for (int32 Item = 0; Item < Actor.TrapActorRefs.Num(); ++Item)
                WriteU32(Trap.Actors + Item * 4, uint32(Actor.TrapActorRefs[Item]));
            WriteU32(Trap.Field14, uint32(Actor.TrapField14)); WriteU32(Trap.Field18, uint32(Actor.TrapField18));
            WriteU32(Trap.Field1C, uint32(Actor.TrapField1C)); WriteU32(Trap.Field20, uint32(Actor.TrapField20));
            if (Trap.bHasField24) WriteU32(Trap.Field24, uint32(Actor.TrapField24));
            WriteU32(Trap.ActorRef, uint32(Actor.TrapActorRef));
            WriteFloat(Trap.Speed, Actor.TrapSpeed); WriteFloat(Trap.Acceleration, Actor.TrapAcceleration);
            WriteU32(Trap.PathFollow, uint32(Actor.TrapPathFollowIndex));
            WriteFloat(Trap.Field4C, Actor.TrapField4C);
            WriteU32(Trap.RequiredEscapes, uint32(Actor.TrapRequiredEscapeAttempts));
            for (int32 Item = 0; Item < Actor.TrapStreamEntries.Num(); ++Item)
            {
                const FKyaTrapStreamEntry& TrapEntry = Actor.TrapStreamEntries[Item];
                const int64 Offset = Trap.TrapEntries + int64(Item) * 60;
                WriteU32(Offset, uint32(TrapEntry.Target.TargetRef)); WriteU32(Offset + 4, uint32(TrapEntry.Target.CutsceneId));
                WriteU32(Offset + 8, uint32(TrapEntry.Target.Flags)); WriteU32(Offset + 12, uint32(TrapEntry.Target.MessageId));
                WriteU32(Offset + 16, uint32(TrapEntry.Target.MessageFlags)); WriteU32(Offset + 20, uint32(TrapEntry.Target.Field14));
                WriteU32(Offset + 24, uint32(TrapEntry.Target.Field18));
                WriteU32(Offset + 28, uint32(TrapEntry.Camera.CameraIndex)); WriteU32(Offset + 32, uint32(TrapEntry.Camera.Flags));
                WriteFloat(Offset + 36, TrapEntry.Camera.Field8); WriteFloat(Offset + 40, TrapEntry.Camera.FieldC);
                WriteFloat(Offset + 44, TrapEntry.Camera.Field10); WriteU32(Offset + 48, uint32(TrapEntry.Camera.ActorRef));
                WriteU32(Offset + 52, uint32(TrapEntry.Camera.ZoneRef)); WriteFloat(Offset + 56, TrapEntry.Camera.Field1C);
            }
        }
        if (HasWind)
        {
            WriteU32(Wind.Field160, uint32(Actor.WindField160));
            WriteFloat(Wind.MaxWind, Actor.WindMaxWind); WriteFloat(Wind.Field168, Actor.WindField168);
            WriteFloat(Wind.Field16C, Actor.WindField16C); WriteFloat(Wind.Field174, Actor.WindField174);
            WriteFloat(Wind.Field180, Actor.WindField180); WriteFloat(Wind.Field184, Actor.WindField184);
            WriteFloat(Wind.Field188, Actor.WindField188); WriteFloat(Wind.Field18C, Actor.WindField18C);
            WriteFloat(Wind.Field178, Actor.WindField178); WriteFloat(Wind.Field17C, Actor.WindField17C);
            WriteU32(Wind.Field190, uint32(Actor.WindField190));
            WriteU32(Wind.Sound, uint32(Actor.WindSoundRef)); WriteU32(Wind.ActivationZone, uint32(Actor.WindActivationZoneIndex));
            for (int32 Item = 0; Item < Actor.WindActorRefs.Num(); ++Item)
                WriteU32(Wind.Actors + Item * 4, uint32(Actor.WindActorRefs[Item]));
            WriteU32(Wind.Material, uint32(Actor.WindMaterialId)); WriteU32(Wind.Zone1C4, uint32(Actor.WindZoneRef1C4));
            WriteU32(Wind.Color1C8, uint32(Actor.WindColor1C8)); WriteU32(Wind.Waypoint1CC, uint32(Actor.WindWaypointRef1CC));
            WriteU32(Wind.SectorHash, uint32(uint64(Actor.WindSectorHierarchyHash)));
            WriteU32(Wind.SectorHash + 4, uint32(uint64(Actor.WindSectorHierarchyHash) >> 32));
            if (Wind.bHasSwitches)
            {
                const FKyaSwitch* Switches[] = { &Actor.WindSwitch1D0, &Actor.WindSwitch1D8, &Actor.WindSwitch1E0 };
                for (int32 SwitchIndex = 0; SwitchIndex < 3; ++SwitchIndex)
                {
                    const FKyaSwitch& Switch = *Switches[SwitchIndex];
                    const FKyaWindSwitchOffsets& SwitchOffsets = Wind.Switches[SwitchIndex];
                    for (int32 Item = 0; Item < Switch.Targets.Num(); ++Item)
                    {
                        const FKyaSwitchTarget& Target = Switch.Targets[Item];
                        const int64 T = SwitchOffsets.Targets + int64(Item) * 28;
                        WriteU32(T, uint32(Target.TargetRef)); WriteU32(T + 4, uint32(Target.CutsceneId));
                        WriteU32(T + 8, uint32(Target.Flags)); WriteU32(T + 12, uint32(Target.MessageId));
                        WriteU32(T + 16, uint32(Target.MessageFlags)); WriteU32(T + 20, uint32(Target.Field14));
                        WriteU32(T + 24, uint32(Target.Field18));
                    }
                    const FKyaStreamEventCamera& Camera = Switch.Camera;
                    const int64 C = SwitchOffsets.Camera;
                    WriteU32(C, uint32(Camera.CameraIndex)); WriteU32(C + 4, uint32(Camera.Flags));
                    WriteFloat(C + 8, Camera.Field8); WriteFloat(C + 12, Camera.FieldC); WriteFloat(C + 16, Camera.Field10);
                    WriteU32(C + 20, uint32(Camera.ActorRef)); WriteU32(C + 24, uint32(Camera.ZoneRef));
                    WriteFloat(C + 28, Camera.Field1C);
                }
            }
            WriteFloat(Wind.Floats194, Actor.WindField194); WriteFloat(Wind.Floats194 + 4, Actor.WindField198);
            WriteFloat(Wind.Floats194 + 8, Actor.WindField19C); WriteFloat(Wind.Floats194 + 12, Actor.WindField1A0);
            WriteFloat(Wind.Floats194 + 16, Actor.WindField1A4);
            WriteU32(Wind.Waypoint, uint32(Actor.WindWaypointRef));
            WriteFloat(Wind.Waypoint + 4, Actor.WindField1AC); WriteFloat(Wind.Waypoint + 8, Actor.WindField1B0);
        }
        if (HasProjectile)
        {
            WriteFloat(Projectile.WeightB, Actor.ProjectileWeightB);
            WriteU32(Projectile.Autonomous, uint32(Actor.ProjectileLifeMax));
            WriteFloat(Projectile.Autonomous + 4, Actor.ProjectileGravityScale);
            WriteFloat(Projectile.Autonomous + 8, Actor.ProjectileDynamicExtField4);
            WriteFloat(Projectile.Autonomous + 12, Actor.ProjectileDynamicExtField8);
            const FKyaProjectileSubObj& Sub = Actor.ProjectileSubObj;
            const int64 S = Projectile.SubObj;
            WriteFloat(S, Sub.TimeToExplode); WriteFloat(S + 4, Sub.ExplosionRange); WriteFloat(S + 8, Sub.AnimDuration);
            WriteU32(S + 12, uint32(Sub.Flags));
            WriteFloat(S + 16, Sub.Damage); WriteFloat(S + 20, Sub.HitRadius); WriteFloat(S + 24, Sub.WarnRadius);
            WriteFloat(S + 28, Sub.Field1C); WriteFloat(S + 32, Sub.Field20); WriteFloat(S + 36, Sub.Field24);
            WriteU32(S + 40, uint32(Sub.Field28)); WriteU32(S + 44, uint32(Sub.MaterialId));
            WriteU32(S + 48, uint32(Sub.Field30)); WriteU32(S + 52, uint32(Sub.Field34));
            WriteU32(S + 56, uint32(Sub.Field38)); WriteU32(S + 60, uint32(Sub.Field3C));
            WriteFloat(S + 64, Sub.Field40); WriteFloat(S + 68, Sub.Field44);
            const FKyaPatternPart& Pattern = Actor.ProjectilePattern;
            const int64 P = Projectile.Pattern;
            WriteFloat(P, Pattern.Field4); WriteFloat(P + 4, Pattern.Field0); WriteFloat(P + 8, Pattern.FieldC);
            WriteFloat(P + 12, Pattern.Field8); WriteFloat(P + 16, Pattern.Field10); WriteFloat(P + 20, Pattern.Field14);
            WriteU32(P + 24, uint32(Pattern.MaterialId));
            WriteFloat(P + 28, Pattern.PointCount); WriteFloat(P + 32, Pattern.Field20);
            if (Projectile.bHasPatternField24) WriteFloat(P + 36, Pattern.Field24);
            WriteU32(P + (Projectile.bHasPatternField24 ? 40 : 36), uint32(Pattern.Field28));
        }
        if (HasAraignos)
        {
            WriteFloat(Araignos.WeightB, Actor.AraignosWeightB);
            WriteU32(Araignos.PathFollow, uint32(Actor.AraignosPathFollowIndex));
            WriteFloat(Araignos.Floats, Actor.AraignosField1F8);
            WriteFloat(Araignos.Floats + 4, Actor.AraignosField1FC);
            WriteFloat(Araignos.Floats + 8, Actor.AraignosDamage);
            WriteU32(Araignos.Sounds, uint32(Actor.AraignosSoundRef240));
            WriteU32(Araignos.Sounds + 4, uint32(Actor.AraignosSoundRef238));
            WriteU32(Araignos.Sounds + 8, uint32(Actor.AraignosSoundRef23C));
            const FKyaAddOnGenerator& AddOn = Actor.AraignosAddOn;
            const int64 A = Araignos.AddOn;
            WriteU32(A, uint32(AddOn.Field0)); WriteU32(A + 4, uint32(AddOn.MoneyType));
            WriteU32(A + 8, uint32(AddOn.MoneyCount)); WriteU32(A + 12, uint32(AddOn.BonusCount));
            WriteU32(A + 16, uint32(AddOn.FieldC)); WriteU32(Araignos.SubObjMode, uint32(AddOn.SubObjMode));
            if (Araignos.SubObjFloats)
            {
                WriteFloat(Araignos.SubObjFloats, AddOn.SubObjField0);
                WriteFloat(Araignos.SubObjFloats + 4, AddOn.SubObjField4);
                WriteFloat(Araignos.SubObjFloats + 8, AddOn.SubObjFieldC);
                WriteFloat(Araignos.SubObjFloats + 12, AddOn.SubObjField10);
                WriteFloat(Araignos.SubObjFloats + 16, AddOn.SubObjField14);
            }
        }
        if (HasFx)
        {
            WriteFloat(Fx.Field19C, Actor.FxField19C);
            WriteFloat(Fx.Field1A0, Actor.FxField1A0);
        }
        if (Actor.bHasTypedCheckpoints)
        {
            TArray<int64> Checkpoints;
            if (!KyaReadCheckpointOffsets(MakeArrayView(Bank->BankBytes.GetData(), int32(SceneEnd)), Offsets.ClassData, Offsets.ClassDataSize, Checkpoints)) return false;
            for (int32 CheckpointIndex = 0; CheckpointIndex < Checkpoints.Num(); ++CheckpointIndex)
            {
                const FKyaCheckpoint& Checkpoint = Actor.Checkpoints[CheckpointIndex];
                const int64 Offset = Checkpoints[CheckpointIndex];
                WriteU32(Offset, uint32(Checkpoint.ZoneRef));
                WriteU32(Offset + 4, uint32(Checkpoint.WaypointA));
                WriteU32(Offset + 8, uint32(Checkpoint.SectorId));
                WriteU32(Offset + 12, uint32(Checkpoint.WaypointB));
                WriteU32(Offset + 16, uint32(Checkpoint.Flags));
                for (int32 Pair = 0; Pair < Checkpoint.ActorWaypoints.Num(); ++Pair)
                {
                    WriteU32(Offset + 24 + Pair * 8, uint32(Checkpoint.ActorWaypoints[Pair].ActorRef));
                    WriteU32(Offset + 28 + Pair * 8, uint32(Checkpoint.ActorWaypoints[Pair].WaypointRef));
                }
            }
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
    Ar.Serialize(Bytes.GetData(), Bytes.Num());
    return !Ar.IsError();
}
