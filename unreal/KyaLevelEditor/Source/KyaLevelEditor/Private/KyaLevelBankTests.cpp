#if WITH_DEV_AUTOMATION_TESTS
// The round-trip test is intentionally large; suppress the optimizer-size warning.
#pragma warning(disable : 4883)
#include "KyaLevelBankFactory.h"
#include "KyaLevelBankExporter.h"
#include "KyaLevelBank.h"
#include "KyaG3DChunks.h"
#include "KyaG2D.h"
#include "KyaCameraStream.h"
#include "KyaLevelPreview.h"
#include "KyaWaypointComponent.h"
#include "KyaPathPointComponent.h"
#include "KyaActorMarkerComponent.h"
#include "KyaActorTransform.h"
#include "ProceduralMeshComponent.h"
#include "Components/SplineComponent.h"
#include "Engine/World.h"
#include "Materials/Material.h"
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Serialization/MemoryWriter.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKyaBankRoundTripTest, "Kya.Bank.LevelOneRoundTrip",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKyaBankRoundTripTest::RunTest(const FString& Parameters)
{
    TestEqual(TEXT("Material hash folds ASCII case"), KyaG2D::HashName(TEXT("abc")), uint64(0x434241));
    TestEqual(TEXT("Material hash packs eight lanes"), KyaG2D::HashName(TEXT("ABCDEFGH")), uint64(0x4847464544434241));
    TestEqual(TEXT("Material hash wraps lane sums"), KyaG2D::HashName(TEXT("abcdefgha")), uint64(0x4847464544434282));
    {
        TArray<uint8> Data;
        Data.SetNumZeroed(672);
        auto Write = [&Data](int32 Offset, uint32 Value) {
            for (int32 Byte = 0; Byte < 4; ++Byte) Data[Offset + Byte] = uint8(Value >> (Byte * 8));
        };
        FMemory::Memcpy(Data.GetData() + 12, ".G3D", 4);
        Write(16, 0x4c4c4148); Write(24, 80);
        Write(32, 0x48534148); Write(40, 64);
        Write(56, 96); Write(72, 288); Write(88, 480);
        Write(80, 0xb2a49e87); Write(84, 0x4c464e99);
        for (int32 Chunk : { 96, 288, 480 }) { Write(Chunk, 0x52454948); Write(Chunk + 8, 192); }
        Data[112 + 0xac] = 2;
        Write(304 + 0x90, 96); Write(496 + 0x90, 96);
        TArray<int32> CameraRelative;
        if (TestTrue(TEXT("Classify synthetic background"), KyaG3D::ReadCameraRelativeBackgroundHierarchies(Data, CameraRelative)))
        {
            TestEqual(TEXT("Synthetic background excludes fixed hash"), CameraRelative.Num(), 1);
            if (CameraRelative.Num() == 1) TestEqual(TEXT("Synthetic background follows camera"), CameraRelative[0], 304);
        }
        Write(304 + 0x90, 288);
        TestFalse(TEXT("Reject cyclic background hierarchy"), KyaG3D::ReadCameraRelativeBackgroundHierarchies(Data, CameraRelative));
    }
    {
        TArray<uint8> Data;
        Data.SetNumZeroed(640);
        auto Write = [&Data](int32 Offset, uint32 Value) {
            for (int32 Byte = 0; Byte < 4; ++Byte) Data[Offset + Byte] = uint8(Value >> (Byte * 8));
        };
        for (int32 Chunk : { 16, 208, 400 }) FMemory::Memcpy(Data.GetData() + Chunk, "HIER", 4);
        Write(32 + 0x90, 208);
        Write(224 + 0x90, 400);
        const TArray<int32> Roots = { 32, 224 };
        TArray<int32> Expanded;
        if (TestTrue(TEXT("Follow linked cluster hierarchies"), KyaG3D::ExpandClusterHierarchies(Data, Roots, Expanded)))
        {
            TestEqual(TEXT("Linked hierarchy count without duplicates"), Expanded.Num(), 3);
            TestTrue(TEXT("Linked tail included"), Expanded.Contains(416));
        }
        Write(416 + 0x90, 16);
        TestFalse(TEXT("Reject cyclic hierarchy links"), KyaG3D::ExpandClusterHierarchies(Data, Roots, Expanded));
        Write(416 + 0x90, 600);
        TestFalse(TEXT("Reject invalid hierarchy link"), KyaG3D::ExpandClusterHierarchies(Data, Roots, Expanded));
    }
    {
        // CSTA / CDQA / CDQU with a nested CDQU. Each node uses a distinct MBNK.
        TArray<uint8> Data;
        Data.SetNumZeroed(448);
        auto Write = [&Data](int32 Offset, uint32 Value) {
            for (int32 Byte = 0; Byte < 4; ++Byte) Data[Offset + Byte] = uint8(Value >> (Byte * 8));
        };
        FMemory::Memcpy(Data.GetData() + 12, ".G3D", 4);
        Write(16, 0x41545343); Write(24, 224); // CSTA
        Write(32, 0x41514443); Write(40, 208); // CDQA
        Write(80, 0x55514443); Write(88, 160); // root CDQU
        Write(104, 1); Write(144, 240); Write(152, 320);
        Write(160, 0x55514443); Write(168, 80); // nested CDQU
        Write(184, 1); Write(224, 272); Write(232, 384);
        Write(240, 0x4b4e424d); Write(248, 32);
        Write(272, 0x4b4e424d); Write(280, 32);
        Write(304, 0x41544144); Write(312, 144);
        TArray<KyaG3D::FClusterStripReference> Strips;
        if (TestTrue(TEXT("Read nested cluster fixture"), KyaG3D::ReadClusterTreeStrips(Data, Strips)) &&
            TestEqual(TEXT("Nested cluster strip count"), Strips.Num(), 2))
        {
            TestEqual(TEXT("Root strip and bank"), Strips[0].StripOffset, 320);
            TestEqual(TEXT("Root material bank"), Strips[0].MaterialBank, 240);
            TestEqual(TEXT("Child strip and bank"), Strips[1].StripOffset, 384);
            TestEqual(TEXT("Child material bank"), Strips[1].MaterialBank, 272);
        }
        Write(168, 81);
        TestFalse(TEXT("Reject nested cluster exceeding parent"), KyaG3D::ReadClusterTreeStrips(Data, Strips));
    }
    {
        TArray<uint8> PlatformData;
        PlatformData.SetNumZeroed(40);
        PlatformData[4] = 1; PlatformData[8] = 7; PlatformData[12] = 2;
        PlatformData[16] = 1; PlatformData[20] = 9;
        PlatformData[24] = 1; PlatformData[28] = 11; PlatformData[32] = 1; PlatformData[36] = 13;
        FKyaPlatformPrefixOffsets Offsets;
        if (TestTrue(TEXT("Read platform prefix with no-friction actor"), KyaReadPlatformPrefix(PlatformData, 0, PlatformData.Num(), Offsets)))
        {
            TestEqual(TEXT("Platform actor count"), Offsets.ActorCount, 1);
            TestEqual(TEXT("Platform zone count"), Offsets.ZoneCount, 1);
            TestEqual(TEXT("Platform no-friction zone count"), Offsets.NoFrictionZones.Num(), 1);
            TestEqual(TEXT("Platform no-friction zone offset"), Offsets.NoFrictionZones[0], int64(28));
            TestEqual(TEXT("Platform suffix offset"), Offsets.Suffix, int64(40));
        }
        PlatformData.SetNum(39);
        TestFalse(TEXT("Reject truncated no-friction actor list"), KyaReadPlatformPrefix(PlatformData, 0, PlatformData.Num(), Offsets));
    }
    {
        TArray<uint8> Details;
        Details.SetNumZeroed(192);
        Details[0] = 1; Details[152] = 1; Details[172] = 1;
        FKyaPlatformDetailsOffsets Offsets;
        if (TestTrue(TEXT("Read platform camera, property and sound fixture"), KyaReadPlatformDetails(Details, 0, Details.Num(), Offsets)))
        {
            TestEqual(TEXT("Platform camera fixture count"), Offsets.CameraCount, 1);
            TestEqual(TEXT("Platform properties fixture offset"), Offsets.Properties, int64(76));
            TestEqual(TEXT("Platform sound A fixture count"), Offsets.SoundCounts[0], 1);
            TestEqual(TEXT("Platform sound B fixture count"), Offsets.SoundCounts[1], 1);
            TestEqual(TEXT("Platform details fixture end"), Offsets.Tail, int64(192));
        }
        Details.SetNum(191);
        TestFalse(TEXT("Reject truncated platform sound record"), KyaReadPlatformDetails(Details, 0, Details.Num(), Offsets));
    }
    {
        TArray<uint8> CameraData;
        CameraData.SetNumZeroed(72);
        auto WriteU32 = [&CameraData](int32 Offset, uint32 Value) {
            for (int32 Byte = 0; Byte < 4; ++Byte) CameraData[Offset + Byte] = uint8(Value >> (Byte * 8));
        };
        auto WriteF32 = [&WriteU32](int32 Offset, float Value) {
            uint32 Bits;
            FMemory::Memcpy(&Bits, &Value, sizeof(Bits));
            WriteU32(Offset, Bits);
        };
        WriteU32(0, 14); WriteU32(4, 21); WriteU32(8, 23); WriteU32(12, 25);
        WriteU32(16, 27); WriteU32(20, 29); WriteU32(24, 31); WriteU32(28, 33);
        WriteU32(32, 35); WriteU32(36, 37); WriteU32(40, 39);
        WriteF32(44, 1.5f); WriteF32(48, 2.5f); WriteF32(52, 3.5f);
        WriteU32(56, 41); WriteU32(60, 43); WriteF32(64, 4.5f); WriteU32(68, 45);
        FKyaPlatformCamera Camera;
        if (TestTrue(TEXT("Read typed platform camera record"), KyaReadPlatformCamera(CameraData, 0, Camera)))
        {
            TestEqual(TEXT("Platform camera event type"), Camera.EventType, 14);
            TestEqual(TEXT("Platform camera entry flags"), Camera.Flags, 21);
            TestEqual(TEXT("Platform camera target"), Camera.TargetRef, 23);
            TestEqual(TEXT("Platform camera cutscene"), Camera.CutsceneId, 25);
            TestEqual(TEXT("Platform camera target flags"), Camera.TargetFlags, 27);
            TestEqual(TEXT("Platform camera message"), Camera.MessageId, 29);
            TestEqual(TEXT("Platform camera message flags"), Camera.MessageFlags, 31);
            TestEqual(TEXT("Platform camera target field 14"), Camera.TargetField14, 33);
            TestEqual(TEXT("Platform camera target field 18"), Camera.TargetField18, 35);
            TestEqual(TEXT("Platform camera index"), Camera.CameraIndex, 37);
            TestEqual(TEXT("Platform camera flags"), Camera.CameraFlags, 39);
            TestEqual(TEXT("Platform camera float 8"), Camera.Field8, 1.5f);
            TestEqual(TEXT("Platform camera float c"), Camera.FieldC, 2.5f);
            TestEqual(TEXT("Platform camera float 10"), Camera.Field10, 3.5f);
            TestEqual(TEXT("Platform camera actor reference"), Camera.ActorRef, 41);
            TestEqual(TEXT("Platform camera zone reference"), Camera.ZoneRef, 43);
            TestEqual(TEXT("Platform camera float 1c"), Camera.Field1C, 4.5f);
            TestEqual(TEXT("Platform camera sound reference"), Camera.SoundRef, 45);
        }
        CameraData.SetNum(71);
        TestFalse(TEXT("Reject truncated platform camera record"), KyaReadPlatformCamera(CameraData, 0, Camera));
    }
    {
        TArray<uint8> BoxData;
        BoxData.SetNumZeroed(160);
        auto WriteU32 = [&BoxData](int32 Offset, uint32 Value) {
            for (int32 Byte = 0; Byte < 4; ++Byte) BoxData[Offset + Byte] = uint8(Value >> (Byte * 8));
        };
        WriteU32(8, 1); WriteU32(12, 7);
        WriteU32(52, 1);
        for (int32 Index = 0; Index < 7; ++Index) WriteU32(56 + Index * 4, uint32(10 + Index));
        for (int32 Index = 0; Index < 8; ++Index) WriteU32(84 + Index * 4, uint32(20 + Index));
        for (int32 Index = 0; Index < 5; ++Index) WriteU32(116 + Index * 4, uint32(30 + Index));
        WriteU32(136, 1);
        FKyaBasicBoxOffsets Box;
        if (TestTrue(TEXT("Read basic box fixture with refs, switch and sub-object"), KyaReadBasicBoxOffsets(BoxData, 0, BoxData.Num(), Box)))
        {
            TestEqual(TEXT("Basic box actor count"), Box.ActorCount, 1);
            TestEqual(TEXT("Basic box actor offset"), Box.Actors, int64(12));
            TestEqual(TEXT("Basic box vibration offset"), Box.Vibration, int64(16));
            TestEqual(TEXT("Basic box switch count"), Box.SwitchTargetCount, 1);
            TestEqual(TEXT("Basic box switch target offset"), Box.SwitchTargets, int64(56));
            TestEqual(TEXT("Basic box switch camera offset"), Box.SwitchCamera, int64(84));
            TestEqual(TEXT("Basic box add-on offset"), Box.AddOn, int64(116));
            TestEqual(TEXT("Basic box sub-object mode"), Box.SubObjModeValue, 1);
            TestEqual(TEXT("Basic box sub-object floats offset"), Box.SubObjFloats, int64(140));
            TestEqual(TEXT("Basic box fixture end"), Box.Tail, int64(160));
        }
        BoxData.SetNum(159);
        TestFalse(TEXT("Reject truncated basic box sub-object"), KyaReadBasicBoxOffsets(BoxData, 0, BoxData.Num(), Box));
        BoxData.SetNum(140);
        WriteU32(136, 0);
        TestTrue(TEXT("Basic box without sub-object floats"), KyaReadBasicBoxOffsets(BoxData, 0, BoxData.Num(), Box));
        TestEqual(TEXT("Basic box mode-zero end"), Box.Tail, int64(140));
    }
    {
        TArray<uint8> FxData;
        FxData.SetNumZeroed(8);
        FKyaFxOffsets Fx;
        if (TestTrue(TEXT("Read FX fixture"), KyaReadFxOffsets(FxData, 0, FxData.Num(), Fx)))
        {
            TestEqual(TEXT("FX field 19c offset"), Fx.Field19C, int64(0));
            TestEqual(TEXT("FX field 1a0 offset"), Fx.Field1A0, int64(4));
            TestEqual(TEXT("FX fixture end"), Fx.Tail, int64(8));
        }
        FxData.SetNum(7);
        TestFalse(TEXT("Reject truncated FX payload"), KyaReadFxOffsets(FxData, 0, FxData.Num(), Fx));
    }
    {
        TArray<uint8> Behaviour;
        Behaviour.SetNumZeroed(12);
        auto WriteU32 = [&Behaviour](int32 Offset, uint32 Value) {
            for (int32 Byte = 0; Byte < 4; ++Byte) Behaviour[Offset + Byte] = uint8(Value >> (Byte * 8));
        };
        WriteU32(0, 123); WriteU32(4, 7);
        FKyaFxBehaviourValues Values;
        if (TestTrue(TEXT("Read FX single behaviour"), KyaReadFxBehaviour(2, Behaviour, 0, 4, Values)))
            TestEqual(TEXT("FX single behaviour value"), Values.Value, 123);
        if (TestTrue(TEXT("Read FX path behaviour"), KyaReadFxBehaviour(3, Behaviour, 0, 8, Values)))
        {
            TestEqual(TEXT("FX path behaviour value"), Values.Value, 123);
            TestEqual(TEXT("FX path follow index"), Values.PathFollowIndex, 7);
        }
        WriteU32(0, 2); WriteU32(4, 11); WriteU32(8, 12);
        if (TestTrue(TEXT("Read FX unknown behaviour"), KyaReadFxBehaviour(4, Behaviour, 0, Behaviour.Num(), Values)))
        {
            TestEqual(TEXT("FX unknown behaviour count"), Values.UnknownValues.Num(), 2);
            TestEqual(TEXT("FX unknown behaviour value 0"), Values.UnknownValues[0], 11);
            TestEqual(TEXT("FX unknown behaviour value 1"), Values.UnknownValues[1], 12);
        }
        TestFalse(TEXT("Reject truncated FX unknown behaviour"), KyaReadFxBehaviour(4, Behaviour, 0, Behaviour.Num() - 1, Values));
        TestFalse(TEXT("Reject malformed FX single behaviour"), KyaReadFxBehaviour(2, Behaviour, 0, 8, Values));
    }
    {
        TArray<uint8> AraignosData;
        AraignosData.SetNumZeroed(76);
        auto WriteU32 = [&AraignosData](int32 Offset, uint32 Value) {
            for (int32 Byte = 0; Byte < 4; ++Byte) AraignosData[Offset + Byte] = uint8(Value >> (Byte * 8));
        };
        WriteU32(4, 3); WriteU32(20, 31); WriteU32(24, 32); WriteU32(28, 33);
        WriteU32(52, 1);
        FKyaAraignosOffsets Araignos;
        if (TestTrue(TEXT("Read Araignos fixture with sub-object"), KyaReadAraignosOffsets(AraignosData, 0, AraignosData.Num(), Araignos)))
        {
            TestEqual(TEXT("Araignos weight offset"), Araignos.WeightB, int64(0));
            TestEqual(TEXT("Araignos path offset"), Araignos.PathFollow, int64(4));
            TestEqual(TEXT("Araignos float offset"), Araignos.Floats, int64(8));
            TestEqual(TEXT("Araignos sound offset"), Araignos.Sounds, int64(20));
            TestEqual(TEXT("Araignos add-on offset"), Araignos.AddOn, int64(32));
            TestEqual(TEXT("Araignos sub-object mode"), Araignos.SubObjModeValue, 1);
            TestEqual(TEXT("Araignos sub-object floats offset"), Araignos.SubObjFloats, int64(56));
            TestEqual(TEXT("Araignos fixture end"), Araignos.Tail, int64(76));
        }
        AraignosData.SetNum(75);
        TestFalse(TEXT("Reject truncated Araignos sub-object"), KyaReadAraignosOffsets(AraignosData, 0, AraignosData.Num(), Araignos));
        AraignosData.SetNum(56);
        WriteU32(52, 0);
        TestTrue(TEXT("Araignos without sub-object floats"), KyaReadAraignosOffsets(AraignosData, 0, AraignosData.Num(), Araignos));
        TestEqual(TEXT("Araignos mode-zero end"), Araignos.Tail, int64(56));
    }
    {
        TArray<uint8> ProjectileData;
        ProjectileData.SetNumZeroed(136);
        FKyaProjectileOffsets Projectile;
        if (TestTrue(TEXT("Read projectile fixture"), KyaReadProjectileOffsets(ProjectileData, 0, ProjectileData.Num(), Projectile)))
        {
            TestEqual(TEXT("Projectile weight offset"), Projectile.WeightB, int64(0));
            TestEqual(TEXT("Projectile autonomous offset"), Projectile.Autonomous, int64(4));
            TestEqual(TEXT("Projectile sub-object offset"), Projectile.SubObj, int64(20));
            TestEqual(TEXT("Projectile pattern offset"), Projectile.Pattern, int64(92));
            TestTrue(TEXT("Projectile pattern field 24 present"), Projectile.bHasPatternField24);
            TestEqual(TEXT("Projectile pattern field 24 offset"), Projectile.PatternField24, int64(128));
            TestEqual(TEXT("Projectile fixture end"), Projectile.Tail, int64(136));
        }
        ProjectileData.SetNum(132);
        if (TestTrue(TEXT("Read old projectile fixture"), KyaReadProjectileOffsets(ProjectileData, 0, ProjectileData.Num(), Projectile)))
        {
            TestFalse(TEXT("Old projectile omits pattern field 24"), Projectile.bHasPatternField24);
            TestEqual(TEXT("Old projectile fixture end"), Projectile.Tail, int64(132));
        }
        ProjectileData.SetNum(135);
        TestFalse(TEXT("Reject partial projectile payload"), KyaReadProjectileOffsets(ProjectileData, 0, ProjectileData.Num(), Projectile));
        ProjectileData.SetNum(131);
        TestFalse(TEXT("Reject short projectile payload"), KyaReadProjectileOffsets(ProjectileData, 0, ProjectileData.Num(), Projectile));
    }
    {
        TArray<uint8> WindData;
        WindData.SetNumZeroed(228);
        auto WriteU32 = [&WindData](int32 Offset, uint32 Value) {
            for (int32 Byte = 0; Byte < 4; ++Byte) WindData[Offset + Byte] = uint8(Value >> (Byte * 8));
        };
        WriteU32(56, 1); WriteU32(60, 7);
        FKyaWindOffsets Wind;
        if (TestTrue(TEXT("Read wind fixture without targets"), KyaReadWindOffsets(WindData, 0, WindData.Num(), Wind)))
        {
            TestEqual(TEXT("Wind actor count"), Wind.ActorCount, 1);
            TestEqual(TEXT("Wind actor offset"), Wind.Actors, int64(60));
            TestEqual(TEXT("Wind material offset"), Wind.Material, int64(64));
            TestEqual(TEXT("Wind sector hash offset"), Wind.SectorHash, int64(80));
            TestTrue(TEXT("Wind fixture has switches"), Wind.bHasSwitches);
            TestEqual(TEXT("Wind switch 0 count"), Wind.Switches[0].Count, 0);
            TestEqual(TEXT("Wind switch 0 camera offset"), Wind.Switches[0].Camera, int64(92));
            TestEqual(TEXT("Wind switch 2 camera offset"), Wind.Switches[2].Camera, int64(164));
            TestEqual(TEXT("Wind float offset"), Wind.Floats194, int64(196));
            TestEqual(TEXT("Wind fixture end"), Wind.Tail, int64(228));
        }
        WindData.SetNumZeroed(288);
        WriteU32(56, 2); WriteU32(60, 7); WriteU32(64, 8);
        WriteU32(92, 1); WriteU32(156, 0); WriteU32(192, 1);
        if (TestTrue(TEXT("Read wind fixture with targets"), KyaReadWindOffsets(WindData, 0, WindData.Num(), Wind)))
        {
            TestEqual(TEXT("Wind target actor count"), Wind.ActorCount, 2);
            TestEqual(TEXT("Wind switch 0 target count"), Wind.Switches[0].Count, 1);
            TestEqual(TEXT("Wind switch 0 target offset"), Wind.Switches[0].Targets, int64(96));
            TestEqual(TEXT("Wind switch 2 target count"), Wind.Switches[2].Count, 1);
            TestEqual(TEXT("Wind target fixture end"), Wind.Tail, int64(288));
        }
        WindData.SetNum(287);
        TestFalse(TEXT("Reject truncated wind payload"), KyaReadWindOffsets(WindData, 0, WindData.Num(), Wind));
        WindData.SetNumZeroed(116);
        WriteU32(56, 0);
        if (TestTrue(TEXT("Read old wind fixture without switches"), KyaReadWindOffsets(WindData, 0, WindData.Num(), Wind)))
        {
            TestFalse(TEXT("Old wind fixture omits switches"), Wind.bHasSwitches);
            TestEqual(TEXT("Old wind fixture end"), Wind.Tail, int64(116));
        }
    }
    {
        TArray<uint8> TrapData;
        TrapData.SetNumZeroed(56);
        auto WriteU32 = [&TrapData](int32 Offset, uint32 Value) {
            for (int32 Byte = 0; Byte < 4; ++Byte) TrapData[Offset + Byte] = uint8(Value >> (Byte * 8));
        };
        FKyaTrapOffsets Trap;
        if (TestTrue(TEXT("Read trap fixture"), KyaReadTrapOffsets(TrapData, 0, TrapData.Num(), Trap)))
        {
            TestEqual(TEXT("Trap weight offset"), Trap.WeightB, int64(0));
            TestEqual(TEXT("Trap actor offset"), Trap.Actors, int64(8));
            TestEqual(TEXT("Trap field 14 offset"), Trap.Field14, int64(8));
            TestEqual(TEXT("Trap field 24 offset"), Trap.Field24, int64(24));
            TestEqual(TEXT("Trap actor reference offset"), Trap.ActorRef, int64(28));
            TestEqual(TEXT("Trap speed offset"), Trap.Speed, int64(32));
            TestEqual(TEXT("Trap path offset"), Trap.PathFollow, int64(40));
            TestEqual(TEXT("Trap required escapes offset"), Trap.RequiredEscapes, int64(48));
            TestEqual(TEXT("Trap fixture end"), Trap.Tail, int64(56));
        }
        TrapData.SetNum(52);
        if (TestTrue(TEXT("Read old trap fixture"), KyaReadTrapOffsets(TrapData, 0, TrapData.Num(), Trap)))
        {
            TestFalse(TEXT("Old trap omits field 24"), Trap.bHasField24);
            TestEqual(TEXT("Old trap fixture end"), Trap.Tail, int64(52));
        }
        TrapData.SetNumZeroed(116);
        WriteU32(52, 1);
        if (TestTrue(TEXT("Read trap fixture with stream entry"), KyaReadTrapOffsets(TrapData, 0, TrapData.Num(), Trap)))
        {
            TestEqual(TEXT("Trap stream count"), Trap.TrapCount, 1);
            TestEqual(TEXT("Trap stream offset"), Trap.TrapEntries, int64(56));
            TestEqual(TEXT("Trap stream fixture end"), Trap.Tail, int64(116));
        }
        TrapData.SetNum(55);
        TestFalse(TEXT("Reject partial trap payload"), KyaReadTrapOffsets(TrapData, 0, TrapData.Num(), Trap));
        TrapData.SetNum(51);
        TestFalse(TEXT("Reject short trap payload"), KyaReadTrapOffsets(TrapData, 0, TrapData.Num(), Trap));
    }
    {
        TArray<uint8> WeaponData;
        WeaponData.SetNumZeroed(12);
        FKyaWeaponOffsets Weapon;
        if (TestTrue(TEXT("Read weapon fixture"), KyaReadWeaponOffsets(WeaponData, 0, WeaponData.Num(), Weapon)))
        {
            TestEqual(TEXT("Weapon weight offset"), Weapon.WeightB, int64(0));
            TestEqual(TEXT("Weapon field 1d8 offset"), Weapon.Field1D8, int64(4));
            TestEqual(TEXT("Weapon field 1dc offset"), Weapon.Field1DC, int64(8));
            TestEqual(TEXT("Weapon fixture end"), Weapon.Tail, int64(12));
        }
        WeaponData.SetNum(11);
        TestFalse(TEXT("Reject truncated weapon payload"), KyaReadWeaponOffsets(WeaponData, 0, WeaponData.Num(), Weapon));
    }
    {
        TArray<uint8> Behaviour;
        Behaviour.SetNumZeroed(56);
        FKyaWeaponBehaviourValues Values;
        if (TestTrue(TEXT("Read sniper weapon behaviour"), KyaReadWeaponBehaviour(4, Behaviour, 0, 56, Values)))
        {
            TestEqual(TEXT("Weapon burst entry count"), Values.Burst.Num(), 2);
            TestEqual(TEXT("Weapon sniper material"), Values.SniperMaterialId, 0);
        }
        TestTrue(TEXT("Read pistol weapon behaviour"), KyaReadWeaponBehaviour(3, Behaviour, 0, 52, Values));
        TestFalse(TEXT("Reject truncated pistol weapon behaviour"), KyaReadWeaponBehaviour(3, Behaviour, 0, 51, Values));
        auto WriteU32 = [&Behaviour](int32 Offset, uint32 Value) {
            for (int32 Byte = 0; Byte < 4; ++Byte) Behaviour[Offset + Byte] = uint8(Value >> (Byte * 8));
        };
        WriteU32(32, 4); WriteU32(36, 31); WriteU32(40, 32); WriteU32(44, 33); WriteU32(48, 34);
        if (TestTrue(TEXT("Read RPG weapon behaviour"), KyaReadWeaponBehaviour(6, Behaviour, 0, 52, Values)))
        {
            TestEqual(TEXT("Weapon RPG actor count"), Values.RpgActorRefs.Num(), 4);
            TestEqual(TEXT("Weapon RPG actor 0"), Values.RpgActorRefs[0], 31);
            TestEqual(TEXT("Weapon RPG actor 3"), Values.RpgActorRefs[3], 34);
        }
        TestFalse(TEXT("Reject malformed RPG weapon behaviour"), KyaReadWeaponBehaviour(6, Behaviour, 0, 51, Values));
    }
    {
        TArray<uint8> SwitchData;
        SwitchData.SetNumZeroed(120);
        auto WriteU32 = [&SwitchData](int32 Offset, uint32 Value) {
            for (int32 Byte = 0; Byte < 4; ++Byte) SwitchData[Offset + Byte] = uint8(Value >> (Byte * 8));
        };
        WriteU32(0, 3);
        FKyaSwitchOffsets Switch;
        if (TestTrue(TEXT("Read switch fixture with three targets"), KyaReadSwitchOffsets(SwitchData, 0, SwitchData.Num(), Switch)))
        {
            TestEqual(TEXT("Switch target count"), Switch.Count, 3);
            TestEqual(TEXT("Switch target offset"), Switch.Targets, int64(4));
            TestEqual(TEXT("Switch camera offset"), Switch.Camera, int64(88));
            TestEqual(TEXT("Switch fixture end"), Switch.Tail, int64(120));
        }
        WriteU32(0, 0);
        if (TestTrue(TEXT("Read switch fixture without targets"), KyaReadSwitchOffsets(SwitchData, 0, 36, Switch)))
        {
            TestEqual(TEXT("Switch empty camera offset"), Switch.Camera, int64(4));
            TestEqual(TEXT("Switch empty fixture end"), Switch.Tail, int64(36));
        }
        TestFalse(TEXT("Reject truncated switch camera"), KyaReadSwitchOffsets(SwitchData, 0, 35, Switch));
        WriteU32(0, 4);
        TestFalse(TEXT("Reject switch target count beyond payload"), KyaReadSwitchOffsets(SwitchData, 0, SwitchData.Num(), Switch));
    }
    {
        TArray<uint8> BonusData;
        BonusData.SetNumZeroed(16);
        FKyaBonusOffsets Bonus;
        if (TestTrue(TEXT("Read bonus fixture"), KyaReadBonusOffsets(BonusData, 0, BonusData.Num(), Bonus)))
        {
            TestEqual(TEXT("Bonus weight offset"), Bonus.WeightB, int64(0));
            TestEqual(TEXT("Bonus sound offset"), Bonus.Sound, int64(4));
            TestEqual(TEXT("Bonus flare offset"), Bonus.FlareMaterial, int64(8));
            TestEqual(TEXT("Bonus anim offset"), Bonus.AnimMaterial, int64(12));
            TestEqual(TEXT("Bonus fixture end"), Bonus.Tail, int64(16));
        }
        BonusData.SetNum(15);
        TestFalse(TEXT("Reject truncated bonus payload"), KyaReadBonusOffsets(BonusData, 0, BonusData.Num(), Bonus));
    }
    {
        TArray<uint8> Behaviour;
        Behaviour.SetNumZeroed(32);
        FKyaBonusBehaviourValues Values;
        if (TestTrue(TEXT("Read bonus turn behaviour"), KyaReadBonusBehaviour(3, Behaviour, 0, 32, Values)))
            TestEqual(TEXT("Bonus turn field count"), Values.Field14, 0.0f);
        TestFalse(TEXT("Reject truncated bonus turn behaviour"), KyaReadBonusBehaviour(3, Behaviour, 0, 31, Values));
        Behaviour.SetNumZeroed(4);
        if (TestTrue(TEXT("Read bonus flock behaviour"), KyaReadBonusBehaviour(5, Behaviour, 0, 4, Values)))
            TestEqual(TEXT("Bonus flock path"), Values.FlockPathIndex, 0);
        if (TestTrue(TEXT("Read bonus add-on behaviour"), KyaReadBonusBehaviour(6, Behaviour, 0, 4, Values)))
            TestEqual(TEXT("Bonus add-on field"), Values.AddOnField18, 0.0f);
        TestFalse(TEXT("Reject malformed bonus flock behaviour"), KyaReadBonusBehaviour(5, Behaviour, 0, 3, Values));
    }
    {
        TArray<uint8> MickenData;
        MickenData.SetNumZeroed(40);
        FKyaMickenOffsets Micken;
        if (TestTrue(TEXT("Read Micken fixture"), KyaReadMickenOffsets(MickenData, 0, MickenData.Num(), Micken)))
        {
            TestEqual(TEXT("Micken weight offset"), Micken.WeightB, int64(0));
            TestEqual(TEXT("Micken autonomous offset"), Micken.Autonomous, int64(4));
            TestEqual(TEXT("Micken waypoint offset"), Micken.Waypoint, int64(20));
            TestEqual(TEXT("Micken actor offset"), Micken.Actor, int64(24));
            TestEqual(TEXT("Micken field 358 offset"), Micken.Field358, int64(28));
            TestEqual(TEXT("Micken field 35c offset"), Micken.Field35C, int64(32));
            TestEqual(TEXT("Micken field 360 offset"), Micken.Field360, int64(36));
            TestEqual(TEXT("Micken fixture end"), Micken.Tail, int64(40));
        }
        MickenData.SetNum(39);
        TestFalse(TEXT("Reject truncated Micken payload"), KyaReadMickenOffsets(MickenData, 0, MickenData.Num(), Micken));
    }
    {
        TArray<uint8> Behaviour;
        Behaviour.SetNumZeroed(16);
        FKyaMickenBehaviourValues Values;
        if (TestTrue(TEXT("Read Micken eat behaviour"), KyaReadMickenBehaviour(4, Behaviour, 0, 16, Values)))
            TestEqual(TEXT("Micken eat path index"), Values.PathIndex, 0);
        TestFalse(TEXT("Reject truncated Micken eat behaviour"), KyaReadMickenBehaviour(4, Behaviour, 0, 15, Values));
    }
    {
        TArray<uint8> FruitData;
        FruitData.SetNumZeroed(60);
        FKyaFruitOffsets Fruit;
        if (TestTrue(TEXT("Read fruit fixture"), KyaReadFruitOffsets(FruitData, 0, FruitData.Num(), Fruit)))
        {
            TestEqual(TEXT("Fruit weight offset"), Fruit.WeightB, int64(0));
            TestEqual(TEXT("Fruit autonomous offset"), Fruit.Autonomous, int64(4));
            TestEqual(TEXT("Fruit field 360 offset"), Fruit.Field360, int64(20));
            TestEqual(TEXT("Fruit switch offset"), Fruit.Switch, int64(24));
            TestEqual(TEXT("Fruit fixture end"), Fruit.Tail, int64(60));
        }
        FruitData.SetNum(59);
        TestFalse(TEXT("Reject truncated fruit payload"), KyaReadFruitOffsets(FruitData, 0, FruitData.Num(), Fruit));
    }
    {
        TArray<uint8> Inventory;
        Inventory.SetNumZeroed(44);
        auto WriteU32 = [&Inventory](int32 Offset, uint32 Value) {
            for (int32 Byte = 0; Byte < 4; ++Byte) Inventory[Offset + Byte] = uint8(Value >> (Byte * 8));
        };
        WriteU32(0, 1); WriteU32(40, 7);
        FKyaFruitInventoryValues Values;
        if (TestTrue(TEXT("Read fruit inventory fixture"), KyaReadFruitInventory(Inventory, 0, Inventory.Num(), Values)))
        {
            TestTrue(TEXT("Fruit inventory has data"), Values.bHasData);
            TestFalse(TEXT("Fruit inventory has no object"), Values.bHasObject);
            TestEqual(TEXT("Fruit inventory initial behaviour"), Values.InitialBehaviour, 7);
        }
        Inventory.SetNum(43);
        TestFalse(TEXT("Reject truncated fruit inventory"), KyaReadFruitInventory(Inventory, 0, Inventory.Num(), Values));
    }
    {
        TArray<uint8> CommanderData;
        CommanderData.SetNumZeroed(100);
        auto WriteU32 = [&CommanderData](int32 Offset, uint32 Value) {
            for (int32 Byte = 0; Byte < 4; ++Byte) CommanderData[Offset + Byte] = uint8(Value >> (Byte * 8));
        };
        WriteU32(32, 1); WriteU32(40, 0);
        FKyaCommanderOffsets Commander;
        if (TestTrue(TEXT("Read commander fixture"), KyaReadCommanderOffsets(CommanderData, 0, CommanderData.Num(), Commander)))
        {
            TestEqual(TEXT("Commander team count"), Commander.TeamCount, 1);
            TestEqual(TEXT("Commander team offset"), Commander.Teams, int64(36));
            TestEqual(TEXT("Commander component offset"), Commander.Components, int64(44));
            TestEqual(TEXT("Commander camera offset"), Commander.Camera, int64(44));
            TestEqual(TEXT("Commander fixture end"), Commander.Tail, int64(100));
        }
        CommanderData.SetNum(99);
        TestFalse(TEXT("Reject truncated commander payload"), KyaReadCommanderOffsets(CommanderData, 0, CommanderData.Num(), Commander));
    }
    {
        TArray<uint8> Behaviour;
        Behaviour.SetNumZeroed(44);
        FKyaCommanderBehaviourOffsets Off;
        if (TestTrue(TEXT("Read commander behaviour fixture"), KyaReadCommanderBehaviourOffsets(Behaviour, 0, Behaviour.Num(), Off)))
        {
            TestEqual(TEXT("Commander squad config offset"), Off.SquadConfig, int64(0));
            TestEqual(TEXT("Commander switch offset"), Off.Switch, int64(8));
            TestEqual(TEXT("Commander behaviour end"), Off.Tail, int64(44));
        }
        Behaviour.SetNum(43);
        TestFalse(TEXT("Reject truncated commander behaviour"), KyaReadCommanderBehaviourOffsets(Behaviour, 0, Behaviour.Num(), Off));
    }
    {
        TArray<uint8> NativData;
        NativData.SetNumZeroed(64);
        FKyaNativOffsets Nativ;
        if (TestTrue(TEXT("Read Nativ fixture"), KyaReadNativOffsets(NativData, 0, NativData.Num(), Nativ)))
        {
            TestEqual(TEXT("Nativ weight offset"), Nativ.WeightB, int64(0));
            TestEqual(TEXT("Nativ field 350 offset"), Nativ.Field350, int64(20));
            TestEqual(TEXT("Nativ field 368 offset"), Nativ.Field368, int64(44));
            TestEqual(TEXT("Nativ field 380 offset"), Nativ.Field380, int64(60));
            TestEqual(TEXT("Nativ fixture end"), Nativ.Tail, int64(64));
        }
        NativData.SetNum(63);
        TestFalse(TEXT("Reject truncated Nativ payload"), KyaReadNativOffsets(NativData, 0, NativData.Num(), Nativ));
    }
    {
        TArray<uint8> Behaviour;
        Behaviour.SetNumZeroed(20);
        Behaviour[0] = 1;
        FKyaNativTakePutOffsets Off;
        if (TestTrue(TEXT("Read Nativ take-and-put behaviour"), KyaReadNativTakePutOffsets(Behaviour, 0, Behaviour.Num(), Off)))
        {
            TestEqual(TEXT("Nativ trajectory count"), Off.Items.Num(), 1);
            TestEqual(TEXT("Nativ trajectory offset"), Off.Items[0], int64(4));
            TestEqual(TEXT("Nativ behaviour end"), Off.Tail, int64(20));
        }
        Behaviour.SetNum(19);
        TestFalse(TEXT("Reject truncated Nativ behaviour"), KyaReadNativTakePutOffsets(Behaviour, 0, Behaviour.Num(), Off));
    }
    {
        TArray<uint8> AmortosData;
        AmortosData.SetNumZeroed(152);
        FKyaAmortosOffsets Amortos;
        if (TestTrue(TEXT("Read Amortos fixture"), KyaReadAmortosOffsets(AmortosData, 0, AmortosData.Num(), Amortos)))
        {
            TestEqual(TEXT("Amortos checkpoint stream count"), Amortos.Streams[0].CountValue, 0);
            TestEqual(TEXT("Amortos field 190 offset"), Amortos.Field190, int64(12));
            TestEqual(TEXT("Amortos float offset"), Amortos.Floats160, int64(16));
            TestEqual(TEXT("Amortos add-on offset"), Amortos.AddOn, int64(52));
            TestEqual(TEXT("Amortos sub-object mode"), Amortos.SubObjModeValue, 0);
            TestEqual(TEXT("Amortos field 1c8 offset"), Amortos.Field1C8, int64(76));
            TestEqual(TEXT("Amortos switch 0 camera"), Amortos.Switches[0].Camera, int64(84));
            TestEqual(TEXT("Amortos switch 1 camera"), Amortos.Switches[1].Camera, int64(120));
            TestEqual(TEXT("Amortos fixture end"), Amortos.Tail, int64(152));
        }
        AmortosData.SetNum(151);
        TestFalse(TEXT("Reject truncated Amortos payload"), KyaReadAmortosOffsets(AmortosData, 0, AmortosData.Num(), Amortos));
        AmortosData.SetNumZeroed(156);
        AmortosData[8] = 1;
        if (TestTrue(TEXT("Read Amortos fixture with checkpoint actor"), KyaReadAmortosOffsets(AmortosData, 0, AmortosData.Num(), Amortos)))
        {
            TestEqual(TEXT("Amortos stream 2 count"), Amortos.Streams[2].CountValue, 1);
            TestEqual(TEXT("Amortos stream 2 entry offset"), Amortos.Streams[2].Entries, int64(12));
            TestEqual(TEXT("Amortos entry fixture end"), Amortos.Tail, int64(156));
        }
    }
    {
        TArray<uint8> TeleporterData;
        TeleporterData.SetNumZeroed(172);
        FKyaTeleporterOffsets Teleporter;
        if (TestTrue(TEXT("Read teleporter fixture"), KyaReadTeleporterOffsets(TeleporterData, 0, TeleporterData.Num(), Teleporter)))
        {
            TestEqual(TEXT("Teleporter destination count offset"), Teleporter.DestCount, int64(16));
            TestEqual(TEXT("Teleporter destination offset"), Teleporter.DestEntries, int64(20));
            TestEqual(TEXT("Teleporter switch 0 camera"), Teleporter.Switches[0].Camera, int64(24));
            TestEqual(TEXT("Teleporter switch 1 camera"), Teleporter.Switches[1].Camera, int64(60));
            TestEqual(TEXT("Teleporter switch 2 camera"), Teleporter.Switches[2].Camera, int64(96));
            TestEqual(TEXT("Teleporter zone 188 offset"), Teleporter.Zone188, int64(128));
            TestEqual(TEXT("Teleporter cinematic offset"), Teleporter.CinematicId, int64(140));
            TestEqual(TEXT("Teleporter field 1e0 offset"), Teleporter.Field1E0, int64(160));
            TestEqual(TEXT("Teleporter waypoint count offset"), Teleporter.WaypointCount, int64(164));
            TestEqual(TEXT("Teleporter condition count offset"), Teleporter.CndOpCount, int64(168));
            TestEqual(TEXT("Teleporter fixture end"), Teleporter.Tail, int64(172));
        }
        TeleporterData.SetNum(171);
        TestFalse(TEXT("Reject truncated teleporter payload"), KyaReadTeleporterOffsets(TeleporterData, 0, TeleporterData.Num(), Teleporter));
        TeleporterData.SetNumZeroed(208);
        auto WriteU32 = [&TeleporterData](int32 Offset, uint32 Value) {
            for (int32 Byte = 0; Byte < 4; ++Byte) TeleporterData[Offset + Byte] = uint8(Value >> (Byte * 8));
        };
        WriteU32(168, 1); WriteU32(172, 1); WriteU32(188, 1);
        if (TestTrue(TEXT("Read teleporter conditioned-operation fixture"), KyaReadTeleporterOffsets(TeleporterData, 0, TeleporterData.Num(), Teleporter)))
        {
            TestEqual(TEXT("Teleporter condition block count"), Teleporter.BlockOps.Num(), 1);
            TestEqual(TEXT("Teleporter operation count"), Teleporter.BlockOpCountValues[0], 1);
            TestEqual(TEXT("Teleporter operation offset"), Teleporter.BlockOps[0], int64(176));
            TestEqual(TEXT("Teleporter condition block size"), Teleporter.BlockCondCountValues[0], 1);
            TestEqual(TEXT("Teleporter condition offset"), Teleporter.BlockConds[0], int64(192));
            TestEqual(TEXT("Teleporter condition fixture end"), Teleporter.Tail, int64(208));
        }
        TeleporterData.SetNum(207);
        TestFalse(TEXT("Reject truncated teleporter condition block"), KyaReadTeleporterOffsets(TeleporterData, 0, TeleporterData.Num(), Teleporter));
    }
    {
        TArray<uint8> DcaData;
        DcaData.SetNumZeroed(36);
        FKyaDcaOffsets Dca;
        if (TestTrue(TEXT("Read DCA fixture"), KyaReadDcaOffsets(DcaData, 0, DcaData.Num(), Dca)))
        {
            TestEqual(TEXT("DCA field 504 offset"), Dca.Field504, int64(0));
            TestEqual(TEXT("DCA aim adjust offset"), Dca.AimAdjust, int64(20));
            TestEqual(TEXT("DCA shot count offset"), Dca.ShotCount, int64(24));
            TestEqual(TEXT("DCA field 450 offset"), Dca.Field450, int64(28));
            TestEqual(TEXT("DCA empty fixture end"), Dca.Tail, int64(36));
        }
        DcaData.SetNumZeroed(40);
        DcaData[24] = 1;
        if (TestTrue(TEXT("Read DCA fixture with shot actor"), KyaReadDcaOffsets(DcaData, 0, DcaData.Num(), Dca)))
        {
            TestEqual(TEXT("DCA shot count"), Dca.ShotCountValue, 1);
            TestEqual(TEXT("DCA shot entry offset"), Dca.ShotEntries, int64(28));
            TestEqual(TEXT("DCA shot fixture end"), Dca.Tail, int64(40));
        }
        DcaData.SetNum(39);
        TestFalse(TEXT("Reject truncated DCA payload"), KyaReadDcaOffsets(DcaData, 0, DcaData.Num(), Dca));
    }
    {
        TArray<uint8> Controlled;
        Controlled.SetNumZeroed(40);
        FKyaDcaBehaviourOffsets Off;
        if (TestTrue(TEXT("Read DCA controlled behaviour"), KyaReadDcaBehaviour(3, Controlled, 0, Controlled.Num(), Off)))
        {
            TestTrue(TEXT("DCA controlled flag"), Off.bControlled);
            TestEqual(TEXT("DCA controlled end"), Off.Tail, int64(40));
        }
        Controlled.SetNum(39);
        TestFalse(TEXT("Reject truncated DCA controlled behaviour"), KyaReadDcaBehaviour(3, Controlled, 0, Controlled.Num(), Off));
        TArray<uint8> Trajectory;
        Trajectory.SetNumZeroed(12);
        if (TestTrue(TEXT("Read DCA trajectory behaviour"), KyaReadDcaBehaviour(5, Trajectory, 0, Trajectory.Num(), Off)))
        {
            TestTrue(TEXT("DCA trajectory flag"), Off.bTrajectory);
            TestEqual(TEXT("DCA trajectory end"), Off.Tail, int64(12));
        }
        Trajectory.SetNum(11);
        TestFalse(TEXT("Reject truncated DCA trajectory behaviour"), KyaReadDcaBehaviour(5, Trajectory, 0, Trajectory.Num(), Off));
        TestFalse(TEXT("Reject unknown DCA behaviour"), KyaReadDcaBehaviour(4, Controlled, 0, Controlled.Num(), Off));
    }
    {
        TArray<uint8> BoxData;
        BoxData.SetNumZeroed(124);
        FKyaBoxOffsets Box;
        if (TestTrue(TEXT("Read box fixture"), KyaReadBoxOffsets(BoxData, 0, BoxData.Num(), Box)))
        {
            TestEqual(TEXT("Box weight offset"), Box.WeightB, int64(0));
            TestEqual(TEXT("Box autonomous offset"), Box.Autonomous, int64(4));
            TestEqual(TEXT("Box field 350 offset"), Box.Field350, int64(20));
            TestEqual(TEXT("Box actor count offset"), Box.ActorCount, int64(24));
            TestEqual(TEXT("Box actor entries offset"), Box.ActorEntries, int64(28));
            TestEqual(TEXT("Box float offset"), Box.Floats354, int64(28));
            TestEqual(TEXT("Box field 364 offset"), Box.Field364, int64(44));
            TestEqual(TEXT("Box field 370 offset"), Box.Field370, int64(48));
            TestEqual(TEXT("Box switch camera offset"), Box.Switch.Camera, int64(68));
            TestEqual(TEXT("Box add-on offset"), Box.AddOn, int64(100));
            TestEqual(TEXT("Box sub-object mode offset"), Box.SubObjMode, int64(120));
            TestEqual(TEXT("Box fixture end"), Box.Tail, int64(124));
        }
        BoxData.SetNumZeroed(128);
        BoxData[24] = 1;
        if (TestTrue(TEXT("Read box fixture with actor"), KyaReadBoxOffsets(BoxData, 0, BoxData.Num(), Box)))
        {
            TestEqual(TEXT("Box actor count"), Box.ActorCountValue, 1);
            TestEqual(TEXT("Box actor entry"), Box.ActorEntries, int64(28));
            TestEqual(TEXT("Box actor fixture end"), Box.Tail, int64(128));
        }
        BoxData.SetNum(123);
        TestFalse(TEXT("Reject truncated box payload"), KyaReadBoxOffsets(BoxData, 0, BoxData.Num(), Box));
    }
    {
        TArray<uint8> MoneyData;
        MoneyData.SetNumZeroed(16);
        FKyaMoneyOffsets Money;
        if (TestTrue(TEXT("Read money fixture"), KyaReadMoneyOffsets(MoneyData, 0, MoneyData.Num(), Money)))
        {
            TestEqual(TEXT("Money weight offset"), Money.WeightB, int64(0));
            TestEqual(TEXT("Money value offset"), Money.MoneyValue, int64(4));
            TestEqual(TEXT("Money sound offset"), Money.Sound, int64(8));
            TestEqual(TEXT("Money material offset"), Money.Material, int64(12));
            TestEqual(TEXT("Money fixture end"), Money.Tail, int64(16));
        }
        MoneyData.SetNum(15);
        TestFalse(TEXT("Reject truncated money payload"), KyaReadMoneyOffsets(MoneyData, 0, MoneyData.Num(), Money));
    }
    {
        TArray<uint8> FogData;
        FogData.SetNumZeroed(4);
        FKyaFogOffsets Fog;
        if (TestTrue(TEXT("Read fog fixture"), KyaReadFogOffsets(FogData, 0, FogData.Num(), Fog)))
        {
            TestEqual(TEXT("Fog hierarchy count offset"), Fog.Count, int64(0));
            TestEqual(TEXT("Fog hierarchy entries offset"), Fog.Entries, int64(4));
            TestEqual(TEXT("Fog fixture end"), Fog.Tail, int64(4));
        }
        FogData.SetNumZeroed(20);
        FogData[0] = 1;
        if (TestTrue(TEXT("Read fog fixture with hierarchy"), KyaReadFogOffsets(FogData, 0, FogData.Num(), Fog)))
        {
            TestEqual(TEXT("Fog hierarchy count"), Fog.CountValue, 1);
            TestEqual(TEXT("Fog hierarchy entry offset"), Fog.Entries, int64(4));
            TestEqual(TEXT("Fog hierarchy fixture end"), Fog.Tail, int64(20));
        }
        FogData.SetNum(19);
        TestFalse(TEXT("Reject truncated fog hierarchy"), KyaReadFogOffsets(FogData, 0, FogData.Num(), Fog));
        TArray<uint8> Zones;
        Zones.SetNumZeroed(4);
        FKyaFogZoneOffsets Off;
        if (TestTrue(TEXT("Read fog zone behaviour"), KyaReadFogZoneOffsets(Zones, 0, Zones.Num(), Off)))
        {
            TestEqual(TEXT("Fog zone count"), Off.CountValue, 0);
            TestEqual(TEXT("Fog zone behaviour end"), Off.Tail, int64(4));
        }
        Zones.SetNumZeroed(32);
        Zones[0] = 1;
        if (TestTrue(TEXT("Read fog zone behaviour with zone"), KyaReadFogZoneOffsets(Zones, 0, Zones.Num(), Off)))
        {
            TestEqual(TEXT("Fog zone count with zone"), Off.CountValue, 1);
            TestEqual(TEXT("Fog zone entry offset"), Off.Entries, int64(4));
            TestEqual(TEXT("Fog zone fixture end"), Off.Tail, int64(32));
        }
        Zones.SetNum(31);
        TestFalse(TEXT("Reject truncated fog zone"), KyaReadFogZoneOffsets(Zones, 0, Zones.Num(), Off));
    }
    {
        TArray<uint8> ShootData;
        ShootData.SetNumZeroed(112);
        FKyaShootOffsets Shoot;
        if (TestTrue(TEXT("Read shooter fixture"), KyaReadShootOffsets(ShootData, 0, ShootData.Num(), Shoot)))
        {
            TestEqual(TEXT("Shooter weight offset"), Shoot.WeightB, int64(0));
            TestEqual(TEXT("Shooter field 350 offset"), Shoot.Field350, int64(20));
            TestEqual(TEXT("Shooter texture offset"), Shoot.Texture, int64(28));
            TestEqual(TEXT("Shooter float 3c0 offset"), Shoot.Floats3C0, int64(36));
            TestEqual(TEXT("Shooter field 3d0 offset"), Shoot.Field3D0, int64(52));
            TestEqual(TEXT("Shooter float 3d4 offset"), Shoot.Floats3D4, int64(56));
            TestEqual(TEXT("Shooter field 3e8 offset"), Shoot.Fields3E8, int64(72));
            TestEqual(TEXT("Shooter add-on offset"), Shoot.AddOn, int64(88));
            TestEqual(TEXT("Shooter sub-object mode offset"), Shoot.SubObjMode, int64(108));
            TestEqual(TEXT("Shooter fixture end"), Shoot.Tail, int64(112));
        }
        ShootData.SetNumZeroed(132);
        ShootData[108] = 1;
        if (TestTrue(TEXT("Read shooter fixture with sub-object"), KyaReadShootOffsets(ShootData, 0, ShootData.Num(), Shoot)))
        {
            TestEqual(TEXT("Shooter sub-object mode"), Shoot.SubObjModeValue, 1);
            TestEqual(TEXT("Shooter sub-object offset"), Shoot.SubObjFloats, int64(112));
            TestEqual(TEXT("Shooter sub-object fixture end"), Shoot.Tail, int64(132));
        }
        ShootData.SetNum(111);
        TestFalse(TEXT("Reject truncated shooter payload"), KyaReadShootOffsets(ShootData, 0, ShootData.Num(), Shoot));
    }
    {
        TArray<uint8> FireData;
        FireData.SetNumZeroed(28);
        FKyaShootFireOffsets Off;
        if (TestTrue(TEXT("Read shooter fire behaviour"), KyaReadShootFireOffsets(FireData, 0, FireData.Num(), Off)))
        {
            TestEqual(TEXT("Shooter fire count offset"), Off.Count, int64(12));
            TestEqual(TEXT("Shooter fire entry offset"), Off.Entries, int64(16));
            TestEqual(TEXT("Shooter empty fire end"), Off.Tail, int64(16));
        }
        FireData[12] = 3;
        if (TestTrue(TEXT("Read shooter fire behaviour with actors"), KyaReadShootFireOffsets(FireData, 0, FireData.Num(), Off)))
        {
            TestEqual(TEXT("Shooter fire actor count"), Off.CountValue, 3);
            TestEqual(TEXT("Shooter fire fixture end"), Off.Tail, int64(28));
        }
        FireData.SetNum(15);
        TestFalse(TEXT("Reject truncated shooter fire behaviour"), KyaReadShootFireOffsets(FireData, 0, FireData.Num(), Off));
        TArray<uint8> WaveData;
        WaveData.SetNumZeroed(28);
        FKyaShootWaveOffsets Wave;
        if (TestTrue(TEXT("Read shooter wave behaviour"), KyaReadShootWaveOffsets(WaveData, 0, WaveData.Num(), Wave)))
            TestEqual(TEXT("Shooter wave end"), Wave.Tail, int64(28));
        WaveData.SetNum(27);
        TestFalse(TEXT("Reject truncated shooter wave behaviour"), KyaReadShootWaveOffsets(WaveData, 0, WaveData.Num(), Wave));
    }
    {
        TArray<uint8> AmbreData;
        AmbreData.SetNumZeroed(56);
        FKyaAmbreOffsets Ambre;
        if (TestTrue(TEXT("Read Ambre fixture"), KyaReadAmbreOffsets(AmbreData, 0, AmbreData.Num(), Ambre)))
        {
            TestEqual(TEXT("Ambre material offset"), Ambre.Material, int64(0));
            TestEqual(TEXT("Ambre effect type offset"), Ambre.EffectType, int64(4));
            TestEqual(TEXT("Ambre texture offset"), Ambre.Texture, int64(8));
            TestEqual(TEXT("Ambre mesh offset"), Ambre.Mesh, int64(12));
            TestEqual(TEXT("Ambre field 1d0 offset"), Ambre.Field1D0, int64(16));
            TestEqual(TEXT("Ambre switch camera offset"), Ambre.Switch.Camera, int64(24));
            TestEqual(TEXT("Ambre fixture end"), Ambre.Tail, int64(56));
        }
        AmbreData.SetNum(55);
        TestFalse(TEXT("Reject truncated Ambre payload"), KyaReadAmbreOffsets(AmbreData, 0, AmbreData.Num(), Ambre));
    }
    {
        TArray<uint8> StillerData;
        StillerData.SetNumZeroed(172);
        FKyaStillerOffsets Stiller;
        if (TestTrue(TEXT("Read Stiller fixture"), KyaReadStillerOffsets(StillerData, 0, StillerData.Num(), Stiller)))
        {
            TestEqual(TEXT("Stiller vision offset"), Stiller.Vision, int64(0));
            TestEqual(TEXT("Stiller field 1d0 offset"), Stiller.Field1D0, int64(20));
            TestEqual(TEXT("Stiller path offset"), Stiller.Path, int64(36));
            TestEqual(TEXT("Stiller life offset"), Stiller.LifeMax, int64(44));
            TestEqual(TEXT("Stiller actor count offset"), Stiller.ActorCount, int64(64));
            TestEqual(TEXT("Stiller field 200 offset"), Stiller.Field200, int64(68));
            TestEqual(TEXT("Stiller switch 0 camera"), Stiller.Switches[0].Camera, int64(76));
            TestEqual(TEXT("Stiller switch 1 camera"), Stiller.Switches[1].Camera, int64(112));
            TestEqual(TEXT("Stiller field 214 offset"), Stiller.Field214, int64(144));
            TestEqual(TEXT("Stiller add-on offset"), Stiller.AddOn, int64(148));
            TestEqual(TEXT("Stiller sub-object mode offset"), Stiller.SubObjMode, int64(168));
            TestEqual(TEXT("Stiller fixture end"), Stiller.Tail, int64(172));
        }
        StillerData.SetNumZeroed(200);
        StillerData[72] = 1;
        if (TestTrue(TEXT("Read Stiller fixture with switch target"), KyaReadStillerOffsets(StillerData, 0, StillerData.Num(), Stiller)))
        {
            TestEqual(TEXT("Stiller switch 0 target count"), Stiller.Switches[0].Count, 1);
            TestEqual(TEXT("Stiller switch 0 target offset"), Stiller.Switches[0].Targets, int64(76));
            TestEqual(TEXT("Stiller switch 0 target camera"), Stiller.Switches[0].Camera, int64(104));
            TestEqual(TEXT("Stiller target fixture end"), Stiller.Tail, int64(200));
        }
        StillerData.SetNum(171);
        TestFalse(TEXT("Reject truncated Stiller payload"), KyaReadStillerOffsets(StillerData, 0, StillerData.Num(), Stiller));
    }
    {
        TArray<uint8> RuneData;
        RuneData.SetNumZeroed(24);
        FKyaRuneOffsets Rune;
        if (TestTrue(TEXT("Read rune fixture"), KyaReadRuneOffsets(RuneData, 0, RuneData.Num(), Rune)))
        {
            TestEqual(TEXT("Rune cinematic offset"), Rune.Cinematic, int64(0));
            TestEqual(TEXT("Rune trigger offset"), Rune.TriggerDistance, int64(4));
            TestEqual(TEXT("Rune scen var offset"), Rune.ScenVar, int64(8));
            TestEqual(TEXT("Rune fx offset"), Rune.Fx, int64(12));
            TestEqual(TEXT("Rune material A offset"), Rune.MaterialA, int64(16));
            TestEqual(TEXT("Rune material B offset"), Rune.MaterialB, int64(20));
            TestEqual(TEXT("Rune fixture end"), Rune.Tail, int64(24));
        }
        RuneData.SetNum(23);
        TestFalse(TEXT("Reject truncated rune payload"), KyaReadRuneOffsets(RuneData, 0, RuneData.Num(), Rune));
        TArray<uint8> FountainData;
        FountainData.SetNumZeroed(24);
        FKyaBonusFountainOffsets Fountain;
        if (TestTrue(TEXT("Read bonus fountain fixture"), KyaReadBonusFountainOffsets(FountainData, 0, FountainData.Num(), Fountain)))
        {
            TestEqual(TEXT("Bonus fountain add-on offset"), Fountain.AddOn, int64(0));
            TestEqual(TEXT("Bonus fountain mode offset"), Fountain.SubObjMode, int64(20));
            TestEqual(TEXT("Bonus fountain fixture end"), Fountain.Tail, int64(24));
        }
        FountainData.SetNumZeroed(44);
        FountainData[20] = 1;
        if (TestTrue(TEXT("Read bonus fountain sub-object fixture"), KyaReadBonusFountainOffsets(FountainData, 0, FountainData.Num(), Fountain)))
        {
            TestEqual(TEXT("Bonus fountain sub-object mode"), Fountain.SubObjModeValue, 1);
            TestEqual(TEXT("Bonus fountain sub-object offset"), Fountain.SubObjFloats, int64(24));
            TestEqual(TEXT("Bonus fountain sub-object end"), Fountain.Tail, int64(44));
        }
        TArray<uint8> RopeData;
        RopeData.SetNumZeroed(76);
        FKyaRopeOffsets Rope;
        if (TestTrue(TEXT("Read rope fixture"), KyaReadRopeOffsets(RopeData, 0, RopeData.Num(), Rope)))
        {
            TestEqual(TEXT("Rope path offset"), Rope.Path, int64(0));
            TestEqual(TEXT("Rope scale offset"), Rope.Scale, int64(4));
            TestEqual(TEXT("Rope material offset"), Rope.Material, int64(12));
            TestEqual(TEXT("Rope color offset"), Rope.Color, int64(16));
            TestEqual(TEXT("Rope actor A offset"), Rope.ActorA, int64(28));
            TestEqual(TEXT("Rope switch camera"), Rope.Switch.Camera, int64(40));
            TestEqual(TEXT("Rope flags offset"), Rope.Flags, int64(72));
            TestEqual(TEXT("Rope fixture end"), Rope.Tail, int64(76));
        }
        RopeData.SetNum(75);
        TestFalse(TEXT("Reject truncated rope payload"), KyaReadRopeOffsets(RopeData, 0, RopeData.Num(), Rope));
    }
    {
        TArray<uint8> GravityData;
        GravityData.SetNumZeroed(20);
        FKyaGravityOffsets Gravity;
        if (TestTrue(TEXT("Read gravity-aware fixture"), KyaReadGravityOffsets(GravityData, 0, GravityData.Num(), Gravity)))
        {
            TestEqual(TEXT("Gravity weight offset"), Gravity.WeightB, int64(0));
            TestEqual(TEXT("Gravity autonomous offset"), Gravity.Autonomous, int64(4));
            TestEqual(TEXT("Gravity fixture end"), Gravity.Tail, int64(20));
        }
        GravityData.SetNum(19);
        TestFalse(TEXT("Reject truncated gravity-aware payload"), KyaReadGravityOffsets(GravityData, 0, GravityData.Num(), Gravity));
        TArray<uint8> BellData;
        BellData.SetNumZeroed(44);
        FKyaGravityBellOffsets Bell;
        if (TestTrue(TEXT("Read gravity bell behaviour"), KyaReadGravityBellOffsets(BellData, 0, BellData.Num(), Bell)))
        {
            TestEqual(TEXT("Gravity bell sound offset"), Bell.Sound, int64(0));
            TestEqual(TEXT("Gravity bell field c offset"), Bell.FieldC, int64(4));
            TestEqual(TEXT("Gravity bell switch camera"), Bell.Switch.Camera, int64(12));
            TestEqual(TEXT("Gravity bell fixture end"), Bell.Tail, int64(44));
        }
        BellData.SetNum(43);
        TestFalse(TEXT("Reject truncated gravity bell behaviour"), KyaReadGravityBellOffsets(BellData, 0, BellData.Num(), Bell));
        TArray<uint8> CommandData;
        CommandData.SetNumZeroed(24);
        FKyaNativCmdOffsets Command;
        if (TestTrue(TEXT("Read nativ command fixture"), KyaReadNativCmdOffsets(CommandData, 0, CommandData.Num(), Command)))
        {
            TestEqual(TEXT("Nativ command field 164 offset"), Command.Field164, int64(4));
            TestEqual(TEXT("Nativ command add-on count"), Command.AddOnCountValues.Num(), 4);
            TestEqual(TEXT("Nativ command fixture end"), Command.Tail, int64(24));
        }
        CommandData.SetNumZeroed(36);
        CommandData[8] = 1; CommandData[16] = 1;
        if (TestTrue(TEXT("Read nativ command sub-object fixture"), KyaReadNativCmdOffsets(CommandData, 0, CommandData.Num(), Command)))
        {
            TestEqual(TEXT("Nativ command first add-on count"), Command.AddOnCountValues[0], 1);
            TestEqual(TEXT("Nativ command sub-object offset"), Command.SubObjOffsets[0], int64(12));
            TestEqual(TEXT("Nativ command sub-object fixture end"), Command.Tail, int64(36));
        }
        CommandData.SetNum(23);
        TestFalse(TEXT("Reject truncated nativ command payload"), KyaReadNativCmdOffsets(CommandData, 0, CommandData.Num(), Command));
    }
    {
        TArray<uint8> CompanionData;
        CompanionData.SetNumZeroed(32);
        FKyaCompanionOffsets Companion;
        if (TestTrue(TEXT("Read companion fixture"), KyaReadCompanionOffsets(CompanionData, 0, CompanionData.Num(), Companion)))
        {
            TestEqual(TEXT("Companion weight offset"), Companion.WeightB, int64(0));
            TestEqual(TEXT("Companion autonomous offset"), Companion.Autonomous, int64(4));
            TestEqual(TEXT("Companion field 358 offset"), Companion.Field358, int64(20));
            TestEqual(TEXT("Companion fixture end"), Companion.Tail, int64(32));
        }
        CompanionData.SetNum(31);
        TestFalse(TEXT("Reject truncated companion payload"), KyaReadCompanionOffsets(CompanionData, 0, CompanionData.Num(), Companion));
        TArray<uint8> BridgeData;
        BridgeData.SetNumZeroed(48);
        FKyaBridgeOffsets Bridge;
        if (TestTrue(TEXT("Read bridge fixture"), KyaReadBridgeOffsets(BridgeData, 0, BridgeData.Num(), Bridge)))
        {
            TestEqual(TEXT("Bridge actor count offset"), Bridge.ActorCount, int64(0));
            TestEqual(TEXT("Bridge waypoint offset"), Bridge.WaypointA, int64(4));
            TestEqual(TEXT("Bridge float offset"), Bridge.Floats330, int64(12));
            TestEqual(TEXT("Bridge field 344 offset"), Bridge.Field344, int64(32));
            TestEqual(TEXT("Bridge config offset"), Bridge.ConfigFlags, int64(36));
            TestEqual(TEXT("Bridge step scale offset"), Bridge.StepCollisionScale, int64(44));
            TestEqual(TEXT("Bridge fixture end"), Bridge.Tail, int64(48));
        }
        BridgeData.SetNumZeroed(172);
        BridgeData[0] = 31;
        if (TestTrue(TEXT("Read bridge fixture with actors"), KyaReadBridgeOffsets(BridgeData, 0, BridgeData.Num(), Bridge)))
        {
            TestEqual(TEXT("Bridge actor count"), Bridge.ActorCountValue, 31);
            TestEqual(TEXT("Bridge actor fixture end"), Bridge.Tail, int64(172));
        }
        BridgeData.SetNum(47);
        TestFalse(TEXT("Reject truncated bridge payload"), KyaReadBridgeOffsets(BridgeData, 0, BridgeData.Num(), Bridge));
        TArray<uint8> BasicData;
        BasicData.SetNumZeroed(32);
        FKyaBridgeBasicOffsets Basic;
        if (TestTrue(TEXT("Read bridge basic behaviour"), KyaReadBridgeBasicOffsets(BasicData, 0, BasicData.Num(), Basic)))
        {
            TestEqual(TEXT("Bridge basic direction offset"), Basic.Direction, int64(0));
            TestEqual(TEXT("Bridge basic field 40 offset"), Basic.Fields40, int64(12));
            TestEqual(TEXT("Bridge basic fixture end"), Basic.Tail, int64(32));
        }
        BasicData.SetNum(31);
        TestFalse(TEXT("Reject truncated bridge basic behaviour"), KyaReadBridgeBasicOffsets(BasicData, 0, BasicData.Num(), Basic));
        TArray<uint8> WindData;
        WindData.SetNumZeroed(20);
        FKyaBridgeWindOffsets Wind;
        if (TestTrue(TEXT("Read bridge wind behaviour"), KyaReadBridgeWindOffsets(WindData, 0, WindData.Num(), Wind)))
            TestEqual(TEXT("Bridge wind fixture end"), Wind.Tail, int64(20));
        WindData.SetNum(19);
        TestFalse(TEXT("Reject truncated bridge wind behaviour"), KyaReadBridgeWindOffsets(WindData, 0, WindData.Num(), Wind));
    }
    {
        TArray<uint8> AtonData;
        AtonData.SetNumZeroed(76);
        FKyaAtonOffsets Aton;
        if (TestTrue(TEXT("Read Aton fixture"), KyaReadAtonOffsets(AtonData, 0, AtonData.Num(), Aton)))
        {
            TestEqual(TEXT("Aton weight offset"), Aton.WeightB, int64(0));
            TestEqual(TEXT("Aton field 354 offset"), Aton.Fields354, int64(20));
            TestEqual(TEXT("Aton effect offset"), Aton.EffectType, int64(36));
            TestEqual(TEXT("Aton trail offset"), Aton.Trail, int64(48));
            TestEqual(TEXT("Aton path count offset"), Aton.PathCount, int64(56));
            TestEqual(TEXT("Aton add-on offset"), Aton.AddOnField18, int64(60));
            TestEqual(TEXT("Aton other count offset"), Aton.OtherCount, int64(64));
            TestEqual(TEXT("Aton sub-object count offset"), Aton.AtonCount, int64(68));
            TestEqual(TEXT("Aton add-on field 24 offset"), Aton.AddOnField24, int64(72));
            TestEqual(TEXT("Aton fixture end"), Aton.Tail, int64(76));
        }
        AtonData.SetNumZeroed(92);
        AtonData[56] = 1; AtonData[68] = 1; AtonData[76] = 1;
        if (TestTrue(TEXT("Read Aton fixture with path and sub-object"), KyaReadAtonOffsets(AtonData, 0, AtonData.Num(), Aton)))
        {
            TestEqual(TEXT("Aton path count"), Aton.PathCountValue, 1);
            TestEqual(TEXT("Aton other sub-object count"), Aton.OtherCountValue, 1);
            TestEqual(TEXT("Aton other sub-object offset"), Aton.OtherOffsets[0], int64(72));
            TestEqual(TEXT("Aton sub-object fixture end"), Aton.Tail, int64(92));
        }
        AtonData.SetNum(75);
        TestFalse(TEXT("Reject truncated Aton payload"), KyaReadAtonOffsets(AtonData, 0, AtonData.Num(), Aton));
    }
    {
        TArray<uint8> BoomyData;
        BoomyData.SetNumZeroed(204);
        FKyaBoomyOffsets Boomy;
        if (TestTrue(TEXT("Read Boomy fixture"), KyaReadBoomyOffsets(BoomyData, 0, BoomyData.Num(), Boomy)))
        {
            TestEqual(TEXT("Boomy weight offset"), Boomy.Weight, int64(0));
            TestEqual(TEXT("Boomy vision offset"), Boomy.Vision, int64(4));
            TestEqual(TEXT("Boomy sound 2b8 offset"), Boomy.Sound2B8, int64(24));
            TestEqual(TEXT("Boomy field 744 offset"), Boomy.Field744, int64(32));
            TestEqual(TEXT("Boomy tail width offset"), Boomy.TailWidthAxis, int64(36));
            TestEqual(TEXT("Boomy tail segments offset"), Boomy.TailSegments, int64(44));
            TestEqual(TEXT("Boomy particle 3d0 offset"), Boomy.Particle3D0, int64(48));
            TestEqual(TEXT("Boomy texture offset"), Boomy.Texture, int64(64));
            TestEqual(TEXT("Boomy mesh offset"), Boomy.Mesh, int64(68));
            TestEqual(TEXT("Boomy camera offset"), Boomy.Camera, int64(72));
            TestEqual(TEXT("Boomy camera fov offset"), Boomy.CameraFov, int64(72));
            TestEqual(TEXT("Boomy camera position offset"), Boomy.CameraPosition, int64(76));
            TestEqual(TEXT("Boomy camera look-at offset"), Boomy.CameraLookAt, int64(88));
            TestEqual(TEXT("Boomy camera up offset"), Boomy.CameraUp, int64(100));
            TestEqual(TEXT("Boomy camera sector offset"), Boomy.CameraSector, int64(112));
            TestEqual(TEXT("Boomy camera flags offset"), Boomy.CameraFlags, int64(120));
            TestEqual(TEXT("Boomy camera target offset"), Boomy.CameraTargetOffset, int64(124));
            TestEqual(TEXT("Boomy camera conditions offset"), Boomy.Conditions, int64(136));
            TestEqual(TEXT("Boomy camera config offset"), Boomy.Config, int64(140));
            TestEqual(TEXT("Boomy camera config angles offset"), Boomy.ConfigAngles, int64(160));
            TestEqual(TEXT("Boomy camera extra offset"), Boomy.Extra, int64(196));
            TestEqual(TEXT("Boomy type count offset"), Boomy.TypeCount, int64(200));
            TestEqual(TEXT("Boomy fixture end"), Boomy.Tail, int64(204));
        }
        BoomyData.SetNum(203);
        TestFalse(TEXT("Reject truncated Boomy payload"), KyaReadBoomyOffsets(BoomyData, 0, BoomyData.Num(), Boomy));
        BoomyData.SetNumZeroed(324);
        auto WriteU32 = [&BoomyData](int32 Offset, uint32 Value) {
            for (int32 Byte = 0; Byte < 4; ++Byte) BoomyData[Offset + Byte] = uint8(Value >> (Byte * 8));
        };
        WriteU32(120, 2); WriteU32(136, 300); WriteU32(140, 1); WriteU32(144, 301); WriteU32(148, 0);
        WriteU32(192, 1); WriteU32(252, 1); WriteU32(304, 1);
        if (TestTrue(TEXT("Read Boomy fixture with camera switch"), KyaReadBoomyOffsets(BoomyData, 0, BoomyData.Num(), Boomy)))
        {
            TestTrue(TEXT("Boomy camera has switch"), Boomy.bHasSwitch);
            TestEqual(TEXT("Boomy switch actor offset"), Boomy.SwitchActor, int64(136));
            TestEqual(TEXT("Boomy switch list A offset"), Boomy.SwitchListA, int64(144));
            TestEqual(TEXT("Boomy switch list A count"), Boomy.SwitchCountA, 1);
            TestEqual(TEXT("Boomy switch list B offset"), Boomy.SwitchListB, int64(152));
            TestEqual(TEXT("Boomy switch list B count"), Boomy.SwitchCountB, 0);
            TestEqual(TEXT("Boomy switch values offset"), Boomy.SwitchValues, int64(152));
            TestEqual(TEXT("Boomy switch conditions offset"), Boomy.Conditions, int64(172));
            TestEqual(TEXT("Boomy switch config offset"), Boomy.Config, int64(176));
            TestEqual(TEXT("Boomy config mode"), Boomy.ConfigMode, 1);
            TestEqual(TEXT("Boomy optional config offset"), Boomy.ConfigOptional, int64(196));
            TestEqual(TEXT("Boomy switch extra offset"), Boomy.Extra, int64(252));
            TestEqual(TEXT("Boomy extra count"), Boomy.ExtraCount, 1);
            TestEqual(TEXT("Boomy type count offset"), Boomy.TypeCount, int64(304));
            TestEqual(TEXT("Boomy type entry offset"), Boomy.TypeEntries, int64(308));
            TestEqual(TEXT("Boomy type count"), Boomy.TypeCountValue, 1);
            TestEqual(TEXT("Boomy switch fixture end"), Boomy.Tail, int64(324));
        }
        BoomyData.SetNum(323);
        TestFalse(TEXT("Reject truncated Boomy type table"), KyaReadBoomyOffsets(BoomyData, 0, BoomyData.Num(), Boomy));
    }
    {
        TArray<uint8> KimData;
        KimData.SetNumZeroed(292);
        FKyaKimOffsets Kim;
        if (TestTrue(TEXT("Read Kim fixture"), KyaReadKimOffsets(KimData, 0, KimData.Num(), Kim)))
        {
            TestEqual(TEXT("Kim fighter end"), Kim.FighterEnd, int64(196));
            TestEqual(TEXT("Kim bone offset"), Kim.Bones, int64(196));
            TestEqual(TEXT("Kim climb count offset"), Kim.ClimbCount, int64(236));
            TestEqual(TEXT("Kim tutorial count offset"), Kim.TutorialCount, int64(240));
            TestEqual(TEXT("Kim projectile count offset"), Kim.ProjectileCount, int64(244));
            TestEqual(TEXT("Kim pair count offset"), Kim.PairCount, int64(248));
            TestEqual(TEXT("Kim zone stream count offset"), Kim.ZoneStreamCount, int64(252));
            TestEqual(TEXT("Kim material offset"), Kim.Material, int64(256));
            TestEqual(TEXT("Kim camera count offset"), Kim.CameraCount, int64(268));
            TestEqual(TEXT("Kim field 15d4 offset"), Kim.Field15D4, int64(272));
            TestEqual(TEXT("Kim fx handles offset"), Kim.FxHandles, int64(280));
            TestEqual(TEXT("Kim fixture end"), Kim.Tail, int64(292));
        }
        KimData.SetNum(291);
        TestFalse(TEXT("Reject truncated Kim payload"), KyaReadKimOffsets(KimData, 0, KimData.Num(), Kim));
        KimData.SetNumZeroed(548);
        KimData[268] = 1;
        if (TestTrue(TEXT("Read Kim fixture with cameras"), KyaReadKimOffsets(KimData, 0, KimData.Num(), Kim)))
        {
            TestEqual(TEXT("Kim camera count"), Kim.CameraCountValue, 1);
            TestEqual(TEXT("Kim expands one camera count to two cameras"), Kim.Cameras.Num(), 2);
            TestEqual(TEXT("Kim first camera offset"), Kim.Cameras[0].Base, int64(272));
            TestEqual(TEXT("Kim second camera offset"), Kim.Cameras[1].Base, int64(400));
            TestEqual(TEXT("Kim camera fixture end"), Kim.Tail, int64(548));
        }
        KimData.SetNum(547);
        TestFalse(TEXT("Reject truncated Kim camera"), KyaReadKimOffsets(KimData, 0, KimData.Num(), Kim));
    }
    {
        TArray<uint8> CompanionData;
        CompanionData.SetNumZeroed(112);
        CompanionData[0] = 1;
        FKyaCompanionBehaviourOffsets Companion;
        if (TestTrue(TEXT("Read companion behaviour fixture"), KyaReadCompanionBehaviourOffsets(CompanionData, 0, CompanionData.Num(), Companion)))
        {
            TestEqual(TEXT("Companion alert count"), Companion.CountValue, 1);
            TestEqual(TEXT("Companion alert offset"), Companion.Alerts.Num(), 1);
            TestEqual(TEXT("Companion condition count offset"), Companion.Alerts[0].Conditions.Count, int64(4));
            TestEqual(TEXT("Companion flags offset"), Companion.Alerts[0].Flags, int64(8));
            TestEqual(TEXT("Companion actor offset"), Companion.Alerts[0].ActorRef, int64(28));
            TestEqual(TEXT("Companion vector offset"), Companion.Alerts[0].Vector20, int64(32));
            TestEqual(TEXT("Companion camera offset"), Companion.Alerts[0].CameraRef, int64(44));
            TestEqual(TEXT("Companion list count offset"), Companion.Alerts[0].ListCount, int64(48));
            TestEqual(TEXT("Companion string key offset"), Companion.Alerts[0].LevelStringKey, int64(52));
            TestEqual(TEXT("Companion switch offset"), Companion.Alerts[0].Switch.Targets, int64(76));
            TestEqual(TEXT("Companion conditioned offset"), Companion.Alerts[0].Conditioned, int64(108));
            TestEqual(TEXT("Companion fixture end"), Companion.Tail, int64(112));
        }
        CompanionData.SetNum(111);
        TestFalse(TEXT("Reject truncated companion behaviour"), KyaReadCompanionBehaviourOffsets(CompanionData, 0, CompanionData.Num(), Companion));
        CompanionData.SetNumZeroed(196);
        auto WriteU32 = [&CompanionData](int32 Offset, uint32 Value) {
            for (int32 Byte = 0; Byte < 4; ++Byte) CompanionData[Offset + Byte] = uint8(Value >> (Byte * 8));
        };
        WriteU32(0, 1); WriteU32(4, 1); WriteU32(64, 1); WriteU32(92, 1); WriteU32(156, 1); WriteU32(160, 1); WriteU32(176, 1);
        if (TestTrue(TEXT("Read companion behaviour fixture with switch and blocks"), KyaReadCompanionBehaviourOffsets(CompanionData, 0, CompanionData.Num(), Companion)))
        {
            TestEqual(TEXT("Companion fixture condition count"), Companion.Alerts[0].Conditions.CountValue, 1);
            TestEqual(TEXT("Companion fixture condition entry offset"), Companion.Alerts[0].Conditions.Entries, int64(8));
            TestEqual(TEXT("Companion fixture list count"), Companion.Alerts[0].ListCountValue, 1);
            TestEqual(TEXT("Companion fixture switch count"), Companion.Alerts[0].Switch.Count, 1);
            TestEqual(TEXT("Companion fixture switch camera offset"), Companion.Alerts[0].Switch.Camera, int64(124));
            TestEqual(TEXT("Companion fixture block count"), Companion.Alerts[0].Blocks.Num(), 1);
            TestEqual(TEXT("Companion fixture operation count"), Companion.Alerts[0].Blocks[0].Operations.Num(), 1);
            TestEqual(TEXT("Companion fixture operation offset"), Companion.Alerts[0].Blocks[0].Operations[0], int64(164));
            TestEqual(TEXT("Companion fixture block condition count"), Companion.Alerts[0].Blocks[0].Conditions.Num(), 1);
            TestEqual(TEXT("Companion fixture block condition offset"), Companion.Alerts[0].Blocks[0].Conditions[0], int64(180));
            TestEqual(TEXT("Companion full fixture end"), Companion.Tail, int64(196));
        }
        CompanionData.SetNum(195);
        TestFalse(TEXT("Reject truncated companion block"), KyaReadCompanionBehaviourOffsets(CompanionData, 0, CompanionData.Num(), Companion));
    }
    {
        TArray<uint8> PlatformBehaviourData;
        PlatformBehaviourData.SetNumZeroed(24);
        FKyaPlatformBehaviourOffsets Platform;
        if (TestTrue(TEXT("Read platform stand behaviour"), KyaReadPlatformBehaviourOffsets(7, PlatformBehaviourData, 0, 8, Platform)))
        {
            TestTrue(TEXT("Platform stand flag"), Platform.bStand);
            TestEqual(TEXT("Platform stand sound offset"), Platform.SoundRef, int64(0));
            TestEqual(TEXT("Platform stand field 0c offset"), Platform.Field0C, int64(4));
            TestEqual(TEXT("Platform stand end"), Platform.Tail, int64(8));
        }
        if (TestTrue(TEXT("Read platform slave behaviour"), KyaReadPlatformBehaviourOffsets(4, PlatformBehaviourData, 0, 8, Platform)))
        {
            TestTrue(TEXT("Platform slave flag"), Platform.bSlave);
            TestEqual(TEXT("Platform slave path offset"), Platform.PathIndex, int64(0));
            TestEqual(TEXT("Platform slave timing offset"), Platform.Timing, int64(4));
            TestEqual(TEXT("Platform slave end"), Platform.Tail, int64(8));
        }
        if (TestTrue(TEXT("Read platform destroyed behaviour"), KyaReadPlatformBehaviourOffsets(6, PlatformBehaviourData, 0, 24, Platform)))
        {
            TestTrue(TEXT("Platform destroyed flag"), Platform.bDestroyed);
            TestEqual(TEXT("Platform destroyed path offset"), Platform.PathIndex, int64(0));
            TestEqual(TEXT("Platform destroyed reader time"), Platform.PathField4, int64(8));
            TestEqual(TEXT("Platform destroyed field 40"), Platform.Field40, int64(16));
            TestEqual(TEXT("Platform destroyed end"), Platform.Tail, int64(24));
        }
        if (TestTrue(TEXT("Read platform selector slave behaviour"), KyaReadPlatformBehaviourOffsets(9, PlatformBehaviourData, 0, 24, Platform)))
        {
            TestTrue(TEXT("Platform selector slave flag"), Platform.bSelectorSlave);
            TestEqual(TEXT("Platform selector slave field 18"), Platform.Field18, int64(0));
            TestEqual(TEXT("Platform selector slave material"), Platform.MaterialId, int64(4));
            TestEqual(TEXT("Platform selector slave sound"), Platform.SoundRef, int64(20));
            TestEqual(TEXT("Platform selector slave end"), Platform.Tail, int64(24));
        }
        TestFalse(TEXT("Reject wrong-length platform stand"), KyaReadPlatformBehaviourOffsets(7, PlatformBehaviourData, 0, 24, Platform));
        TestFalse(TEXT("Reject unknown platform behaviour"), KyaReadPlatformBehaviourOffsets(12, PlatformBehaviourData, 0, 24, Platform));
        if (TestTrue(TEXT("Read platform trajectory behaviour"), KyaReadPlatformBehaviourOffsets(2, PlatformBehaviourData, 0, 24, Platform)))
        {
            TestTrue(TEXT("Platform trajectory flag"), Platform.bTrajectory);
            TestEqual(TEXT("Platform trajectory path offset"), Platform.PathIndex, int64(0));
            TestEqual(TEXT("Platform trajectory timing offset"), Platform.Timing, int64(4));
            TestEqual(TEXT("Platform trajectory segment time"), Platform.SegmentStartTime, int64(16));
            TestEqual(TEXT("Platform trajectory loop delay"), Platform.LoopDelay, int64(20));
            TestEqual(TEXT("Platform trajectory end"), Platform.Tail, int64(24));
        }
    }
    {
        TArray<uint8> SlabData;
        SlabData.SetNumZeroed(56);
        FKyaPlatformBehaviourOffsets Slab;
        if (TestTrue(TEXT("Read platform slab behaviour"), KyaReadPlatformBehaviourOffsets(3, SlabData, 0, 56, Slab)))
        {
            TestTrue(TEXT("Platform slab flag"), Slab.bSlab);
            TestEqual(TEXT("Platform slab switch offset"), Slab.SlabSwitch.Targets, int64(4));
            TestEqual(TEXT("Platform slab float offset"), Slab.SlabFloats, int64(36));
            TestEqual(TEXT("Platform slab field 20 offset"), Slab.SlabField20, int64(52));
            TestEqual(TEXT("Platform slab end"), Slab.Tail, int64(56));
        }
        SlabData.SetNumZeroed(84);
        SlabData[0] = 1;
        if (TestTrue(TEXT("Read platform slab with target"), KyaReadPlatformBehaviourOffsets(3, SlabData, 0, 84, Slab)))
        {
            TestEqual(TEXT("Platform slab target count"), Slab.SlabSwitch.Count, 1);
            TestEqual(TEXT("Platform slab camera offset"), Slab.SlabSwitch.Camera, int64(32));
            TestEqual(TEXT("Platform slab target end"), Slab.Tail, int64(84));
        }
        TArray<uint8> MasterData;
        MasterData.SetNumZeroed(96);
        MasterData[20] = 1;
        if (TestTrue(TEXT("Read platform master behaviour"), KyaReadPlatformBehaviourOffsets(5, MasterData, 0, 96, Slab)))
        {
            TestTrue(TEXT("Platform master flag"), Slab.bMaster);
            TestEqual(TEXT("Platform master path offset"), Slab.MasterPath, int64(0));
            TestEqual(TEXT("Platform master analog count"), Slab.AnalogCountValue, 1);
            TestEqual(TEXT("Platform master analog offset"), Slab.AnalogEntries, int64(24));
            TestEqual(TEXT("Platform master camera offset"), Slab.AnalogCamera, int64(64));
            TestEqual(TEXT("Platform master end"), Slab.Tail, int64(96));
        }
        TArray<uint8> SelectorData;
        SelectorData.SetNumZeroed(100);
        SelectorData[24] = 5;
        SelectorData[48] = 1;
        if (TestTrue(TEXT("Read platform selector-master behaviour"), KyaReadPlatformBehaviourOffsets(8, SelectorData, 0, 100, Slab)))
        {
            TestTrue(TEXT("Platform selector-master flag"), Slab.bSelectorMaster);
            TestEqual(TEXT("Platform selector-master actor count"), Slab.SelectorActorCountValue, 5);
            TestEqual(TEXT("Platform selector-master switch count"), Slab.SelectorSwitchCountValue, 1);
            TestEqual(TEXT("Platform selector-master switch offset"), Slab.SelectorSwitchEntries, int64(52));
            TestEqual(TEXT("Platform selector-master camera offset"), Slab.SelectorCamera, int64(68));
            TestEqual(TEXT("Platform selector-master end"), Slab.Tail, int64(100));
        }
    }
    {
        TArray<uint8> ProjectileBehaviourData;
        ProjectileBehaviourData.SetNumZeroed(20);
        FKyaProjectileBehaviourOffsets Projectile;
        if (TestTrue(TEXT("Read projectile stand behaviour"), KyaReadProjectileBehaviourOffsets(3, ProjectileBehaviourData, 0, 12, Projectile)))
        {
            TestTrue(TEXT("Projectile stand flag"), Projectile.bStand);
            TestEqual(TEXT("Projectile stand id offset"), Projectile.BehaviourId, int64(0));
            TestEqual(TEXT("Projectile stand material offset"), Projectile.MaterialId, int64(8));
            TestEqual(TEXT("Projectile stand end"), Projectile.Tail, int64(12));
        }
        if (TestTrue(TEXT("Read projectile straight behaviour"), KyaReadProjectileBehaviourOffsets(4, ProjectileBehaviourData, 0, 20, Projectile)))
        {
            TestTrue(TEXT("Projectile straight flag"), Projectile.bStraight);
            TestEqual(TEXT("Projectile straight speed offset"), Projectile.Speed, int64(8));
            TestEqual(TEXT("Projectile straight acceleration offset"), Projectile.Acceleration, int64(12));
            TestEqual(TEXT("Projectile straight field 18 offset"), Projectile.Field18, int64(16));
            TestEqual(TEXT("Projectile straight end"), Projectile.Tail, int64(20));
        }
        if (TestTrue(TEXT("Read projectile portable behaviour"), KyaReadProjectileBehaviourOffsets(5, ProjectileBehaviourData, 0, 12, Projectile)))
        {
            TestTrue(TEXT("Projectile portable flag"), Projectile.bPortable);
            TestEqual(TEXT("Projectile portable field 10 offset"), Projectile.Field10, int64(8));
            TestEqual(TEXT("Projectile portable end"), Projectile.Tail, int64(12));
        }
        if (TestTrue(TEXT("Read projectile excuse behaviour"), KyaReadProjectileBehaviourOffsets(11, ProjectileBehaviourData, 0, 12, Projectile)))
        {
            TestTrue(TEXT("Projectile excuse flag"), Projectile.bExcuse);
            TestEqual(TEXT("Projectile excuse actor offset"), Projectile.ActorRef, int64(8));
            TestEqual(TEXT("Projectile excuse end"), Projectile.Tail, int64(12));
        }
        TestFalse(TEXT("Reject wrong-length projectile stand"), KyaReadProjectileBehaviourOffsets(3, ProjectileBehaviourData, 0, 20, Projectile));
        TestFalse(TEXT("Reject unknown projectile behaviour"), KyaReadProjectileBehaviourOffsets(6, ProjectileBehaviourData, 0, 20, Projectile));
    }
    {
        TArray<uint8> SwitchBehaviourData;
        SwitchBehaviourData.SetNumZeroed(76);
        FKyaSwitchBehaviourOffsets Switch;
        if (TestTrue(TEXT("Read switch lever behaviour"), KyaReadSwitchBehaviourOffsets(2, SwitchBehaviourData, 0, 8, Switch)))
        {
            TestTrue(TEXT("Switch lever flag"), Switch.bLever);
            TestEqual(TEXT("Switch lever A offset"), Switch.LeverRefA, int64(0));
            TestEqual(TEXT("Switch lever B offset"), Switch.LeverRefB, int64(4));
            TestEqual(TEXT("Switch lever end"), Switch.Tail, int64(8));
        }
        if (TestTrue(TEXT("Read switch magic behaviour"), KyaReadSwitchBehaviourOffsets(3, SwitchBehaviourData, 0, 76, Switch)))
        {
            TestTrue(TEXT("Switch magic flag"), Switch.bMagic);
            TestEqual(TEXT("Switch magic field 08 offset"), Switch.MagicField08, int64(0));
            TestEqual(TEXT("Switch magic digits offset"), Switch.MagicFxDigits, int64(8));
            TestEqual(TEXT("Switch magic texture offset"), Switch.OpenTextureId, int64(36));
            TestEqual(TEXT("Switch magic camera offset"), Switch.MagicCamera, int64(44));
            TestEqual(TEXT("Switch magic end"), Switch.Tail, int64(76));
        }
        if (TestTrue(TEXT("Read switch multi-condition behaviour"), KyaReadSwitchBehaviourOffsets(5, SwitchBehaviourData, 0, 12, Switch)))
        {
            TestTrue(TEXT("Switch multi flag"), Switch.bMultiCondition);
            TestEqual(TEXT("Switch multi field 14 offset"), Switch.MultiField14, int64(0));
            TestEqual(TEXT("Switch multi field 08 offset"), Switch.MultiField08, int64(4));
            TestEqual(TEXT("Switch multi end"), Switch.Tail, int64(12));
        }
        if (TestTrue(TEXT("Read switch target behaviour"), KyaReadSwitchBehaviourOffsets(6, SwitchBehaviourData, 0, 12, Switch)))
        {
            TestTrue(TEXT("Switch target flag"), Switch.bTarget);
            TestEqual(TEXT("Switch target displacement offset"), Switch.TargetDisplacement, int64(0));
            TestEqual(TEXT("Switch target end"), Switch.Tail, int64(12));
        }
        TestFalse(TEXT("Reject wrong-length switch lever"), KyaReadSwitchBehaviourOffsets(2, SwitchBehaviourData, 0, 76, Switch));
        TestFalse(TEXT("Reject unknown switch behaviour"), KyaReadSwitchBehaviourOffsets(7, SwitchBehaviourData, 0, 76, Switch));
    }
    {
        TArray<uint8> WolfenBehaviourData;
        WolfenBehaviourData.SetNumZeroed(68);
        WolfenBehaviourData[16] = 1;
        FKyaWolfenBehaviourOffsets WolfenBehaviour;
        if (TestTrue(TEXT("Read Wolfen track behaviour"), KyaReadWolfenBehaviourOffsets(15, WolfenBehaviourData, 0, 8, WolfenBehaviour)))
        {
            TestTrue(TEXT("Wolfen track flag"), WolfenBehaviour.bTrack);
            TestEqual(TEXT("Wolfen track field offset"), WolfenBehaviour.TrackField, int64(4));
            TestEqual(TEXT("Wolfen track end"), WolfenBehaviour.Tail, int64(8));
        }
        if (TestTrue(TEXT("Read Wolfen DCA behaviour"), KyaReadWolfenBehaviourOffsets(23, WolfenBehaviourData, 0, 12, WolfenBehaviour)))
        {
            TestTrue(TEXT("Wolfen DCA flag"), WolfenBehaviour.bDca);
            TestEqual(TEXT("Wolfen DCA actor offset"), WolfenBehaviour.DcaActorRef, int64(4));
            TestEqual(TEXT("Wolfen DCA cooldown offset"), WolfenBehaviour.DcaShootCooldown, int64(8));
            TestEqual(TEXT("Wolfen DCA end"), WolfenBehaviour.Tail, int64(12));
        }
        if (TestTrue(TEXT("Read Wolfen avoid behaviour"), KyaReadWolfenBehaviourOffsets(24, WolfenBehaviourData, 0, 12, WolfenBehaviour)))
        {
            TestTrue(TEXT("Wolfen avoid flag"), WolfenBehaviour.bAvoid);
            TestEqual(TEXT("Wolfen avoid field 80 offset"), WolfenBehaviour.Avoid80, int64(4));
            TestEqual(TEXT("Wolfen avoid field 84 offset"), WolfenBehaviour.Avoid84, int64(8));
            TestEqual(TEXT("Wolfen avoid end"), WolfenBehaviour.Tail, int64(12));
        }
        if (TestTrue(TEXT("Read Wolfen track-weapon-stand behaviour"), KyaReadWolfenBehaviourOffsets(18, WolfenBehaviourData, 0, 40, WolfenBehaviour)))
        {
            TestTrue(TEXT("Wolfen track weapon stand flag"), WolfenBehaviour.bWeaponStand);
            TestEqual(TEXT("Wolfen weapon field 90 offset"), WolfenBehaviour.WeaponField90, int64(4));
            TestEqual(TEXT("Wolfen weapon count offset"), WolfenBehaviour.WeaponCount, int64(16));
            TestEqual(TEXT("Wolfen weapon sub-object count"), WolfenBehaviour.SubObjs.Num(), 1);
            TestEqual(TEXT("Wolfen weapon sub-object offset"), WolfenBehaviour.SubObjs[0].Field0, int64(20));
            TestEqual(TEXT("Wolfen track weapon stand end"), WolfenBehaviour.Tail, int64(40));
        }
        if (TestTrue(TEXT("Read Wolfen track-weapon behaviour"), KyaReadWolfenBehaviourOffsets(17, WolfenBehaviourData, 0, 48, WolfenBehaviour)))
        {
            TestTrue(TEXT("Wolfen track weapon flag"), WolfenBehaviour.bWeapon);
            TestEqual(TEXT("Wolfen weapon trailing offset"), WolfenBehaviour.WeaponFieldF0, int64(40));
            TestEqual(TEXT("Wolfen weapon target offset"), WolfenBehaviour.WeaponTargetField, int64(44));
            TestEqual(TEXT("Wolfen track weapon end"), WolfenBehaviour.Tail, int64(48));
        }
        if (TestTrue(TEXT("Read Wolfen track-weapon-snipe behaviour"), KyaReadWolfenBehaviourOffsets(19, WolfenBehaviourData, 0, 44, WolfenBehaviour)))
        {
            TestTrue(TEXT("Wolfen track weapon snipe flag"), WolfenBehaviour.bWeaponSnipe);
            TestEqual(TEXT("Wolfen weapon snipe trailing offset"), WolfenBehaviour.WeaponFieldF0, int64(40));
            TestEqual(TEXT("Wolfen track weapon snipe end"), WolfenBehaviour.Tail, int64(44));
        }
        TestFalse(TEXT("Reject wrong-length Wolfen track"), KyaReadWolfenBehaviourOffsets(15, WolfenBehaviourData, 0, 12, WolfenBehaviour));
        TestFalse(TEXT("Reject wrong-length Wolfen track weapon"), KyaReadWolfenBehaviourOffsets(17, WolfenBehaviourData, 0, 40, WolfenBehaviour));
        TestFalse(TEXT("Reject unknown Wolfen behaviour"), KyaReadWolfenBehaviourOffsets(16, WolfenBehaviourData, 0, 40, WolfenBehaviour));
        TArray<uint8> LostData;
        LostData.SetNumZeroed(60);
        LostData[8] = 1;
        LostData[32] = 1;
        if (TestTrue(TEXT("Read Wolfen lost behaviour"), KyaReadWolfenBehaviourOffsets(22, LostData, 0, 60, WolfenBehaviour)))
        {
            TestTrue(TEXT("Wolfen lost flag"), WolfenBehaviour.bLost);
            TestEqual(TEXT("Wolfen lost record count"), WolfenBehaviour.Switch.RecordOffsets.Num(), 2);
            TestEqual(TEXT("Wolfen lost first category"), WolfenBehaviour.Switch.RecordCategories[0], 0);
            TestEqual(TEXT("Wolfen lost second category"), WolfenBehaviour.Switch.RecordCategories[1], 2);
            TestEqual(TEXT("Wolfen lost message count"), WolfenBehaviour.Switch.CountValues[0], 1);
            TestEqual(TEXT("Wolfen lost area count"), WolfenBehaviour.Switch.CountValues[2], 1);
            TestEqual(TEXT("Wolfen lost end"), WolfenBehaviour.Tail, int64(60));
        }
        TArray<uint8> WatchDogData;
        WatchDogData.SetNumZeroed(80);
        if (TestTrue(TEXT("Read Wolfen watchdog behaviour"), KyaReadWolfenBehaviourOffsets(8, WatchDogData, 0, 80, WolfenBehaviour)))
        {
            TestTrue(TEXT("Wolfen watchdog flag"), WolfenBehaviour.bWatchDog);
            TestEqual(TEXT("Wolfen watchdog field 80 offset"), WolfenBehaviour.Field80, int64(4));
            TestEqual(TEXT("Wolfen watchdog track offset"), WolfenBehaviour.TrackBehaviourId, int64(8));
            TestEqual(TEXT("Wolfen watchdog switch end"), WolfenBehaviour.Switch.Tail, int64(44));
            TestEqual(TEXT("Wolfen watchdog onoff offset"), WolfenBehaviour.OnOffCount, int64(44));
            TestEqual(TEXT("Wolfen watchdog camera offset"), WolfenBehaviour.OnOffCamera, int64(48));
            TestEqual(TEXT("Wolfen watchdog end"), WolfenBehaviour.Tail, int64(80));
        }
        TArray<uint8> SnipeData;
        SnipeData.SetNumZeroed(96);
        if (TestTrue(TEXT("Read Wolfen snipe behaviour"), KyaReadWolfenBehaviourOffsets(21, SnipeData, 0, 96, WolfenBehaviour)))
        {
            TestTrue(TEXT("Wolfen snipe flag"), WolfenBehaviour.bSnipe);
            TestEqual(TEXT("Wolfen snipe field 90 offset"), WolfenBehaviour.SnipeField90, int64(4));
            TestEqual(TEXT("Wolfen snipe spot track offset"), WolfenBehaviour.SpotTrack, int64(24));
            TestEqual(TEXT("Wolfen snipe shadow material offset"), WolfenBehaviour.ShadowMaterial, int64(32));
            TestEqual(TEXT("Wolfen snipe nb spot offset"), WolfenBehaviour.NbSpot, int64(48));
            TestEqual(TEXT("Wolfen snipe switch end"), WolfenBehaviour.Switch.Tail, int64(96));
            TestEqual(TEXT("Wolfen snipe end"), WolfenBehaviour.Tail, int64(96));
        }
        TArray<uint8> GuardData;
        GuardData.SetNumZeroed(88);
        if (TestTrue(TEXT("Read Wolfen guard-area behaviour"), KyaReadWolfenBehaviourOffsets(10, GuardData, 0, 88, WolfenBehaviour)))
        {
            TestTrue(TEXT("Wolfen guard flag"), WolfenBehaviour.bGuard);
            TestEqual(TEXT("Wolfen guard path offset"), WolfenBehaviour.GuardPath, int64(4));
            TestEqual(TEXT("Wolfen guard field 98 offset"), WolfenBehaviour.GuardField98, int64(8));
            TestEqual(TEXT("Wolfen guard switch end"), WolfenBehaviour.Switch.Tail, int64(52));
            TestEqual(TEXT("Wolfen guard onoff offset"), WolfenBehaviour.OnOffCount, int64(52));
            TestEqual(TEXT("Wolfen guard camera offset"), WolfenBehaviour.OnOffCamera, int64(56));
            TestEqual(TEXT("Wolfen guard end"), WolfenBehaviour.Tail, int64(88));
        }
        TArray<uint8> EscapeData;
        EscapeData.SetNumZeroed(72);
        EscapeData[4] = 1;
        if (TestTrue(TEXT("Read Wolfen escape behaviour"), KyaReadWolfenBehaviourOffsets(13, EscapeData, 0, 72, WolfenBehaviour)))
        {
            TestTrue(TEXT("Wolfen escape flag"), WolfenBehaviour.bEscape);
            TestEqual(TEXT("Wolfen escape reader count"), WolfenBehaviour.EscapeCountValue, 1);
            TestEqual(TEXT("Wolfen escape reader offset"), WolfenBehaviour.EscapeEntries, int64(8));
            TestEqual(TEXT("Wolfen escape field 88 offset"), WolfenBehaviour.EscapeField88, int64(12));
            TestEqual(TEXT("Wolfen escape float offset"), WolfenBehaviour.EscapeFloats, int64(16));
            TestEqual(TEXT("Wolfen escape field a0 offset"), WolfenBehaviour.EscapeFieldA0, int64(36));
            TestEqual(TEXT("Wolfen escape switch end"), WolfenBehaviour.Switch.Tail, int64(72));
            TestEqual(TEXT("Wolfen escape end"), WolfenBehaviour.Tail, int64(72));
        }
        TArray<uint8> ExorcismData;
        ExorcismData.SetNumZeroed(48);
        if (TestTrue(TEXT("Read Wolfen exorcism behaviour"), KyaReadWolfenBehaviourOffsets(14, ExorcismData, 0, 48, WolfenBehaviour)))
        {
            TestTrue(TEXT("Wolfen exorcism flag"), WolfenBehaviour.bExorcism);
            TestEqual(TEXT("Wolfen exorcism material offset"), WolfenBehaviour.ExorcismMaterial, int64(0));
            TestEqual(TEXT("Wolfen exorcism field 24 offset"), WolfenBehaviour.ExorcismField24, int64(4));
            TestEqual(TEXT("Wolfen exorcism mesh pairs offset"), WolfenBehaviour.ExorcismMeshes, int64(8));
            TestEqual(TEXT("Wolfen exorcism field 18 offset"), WolfenBehaviour.ExorcismField18, int64(24));
            TestEqual(TEXT("Wolfen exorcism words offset"), WolfenBehaviour.ExorcismWords, int64(28));
            TestEqual(TEXT("Wolfen exorcism end"), WolfenBehaviour.Tail, int64(48));
        }
        TestFalse(TEXT("Reject wrong-length Wolfen exorcism"), KyaReadWolfenBehaviourOffsets(14, ExorcismData, 0, 44, WolfenBehaviour));
    }
    {
        TArray<uint8> CatchData;
        CatchData.SetNumZeroed(24);
        CatchData[0] = 1;
        CatchData[4] = 1;
        FKyaCatchByTrapOffsets Catch;
        if (TestTrue(TEXT("Read catch-by-trap fixture"), KyaReadCatchByTrapOffsets(CatchData, 0, CatchData.Num(), Catch)))
        {
            TestEqual(TEXT("Catch-by-trap block count"), Catch.Blocks.Num(), 1);
            TestEqual(TEXT("Catch-by-trap operation count"), Catch.Blocks[0].Operations.Num(), 1);
            TestEqual(TEXT("Catch-by-trap operation offset"), Catch.Blocks[0].Operations[0], int64(8));
            TestEqual(TEXT("Catch-by-trap condition offset"), Catch.Blocks[0].Conditions.Num(), 0);
            TestEqual(TEXT("Catch-by-trap end"), Catch.Tail, int64(24));
        }
        CatchData.SetNum(4);
        CatchData[0] = 0;
        if (TestTrue(TEXT("Read empty catch-by-trap fixture"), KyaReadCatchByTrapOffsets(CatchData, 0, 4, Catch)))
        {
            TestEqual(TEXT("Empty catch-by-trap block count"), Catch.Blocks.Num(), 0);
            TestEqual(TEXT("Empty catch-by-trap end"), Catch.Tail, int64(4));
        }
        CatchData.SetNum(8);
        CatchData[0] = 1;
        CatchData[4] = 1;
        TestFalse(TEXT("Reject truncated catch-by-trap"), KyaReadCatchByTrapOffsets(CatchData, 0, 8, Catch));
    }
    {
        TArray<uint8> HeroUnknownData;
        HeroUnknownData.SetNumZeroed(28);
        FKyaHeroUnknownOffsets HeroUnknown;
        if (TestTrue(TEXT("Read hero unknown fixture"), KyaReadHeroUnknownOffsets(HeroUnknownData, 0, 28, HeroUnknown)))
        {
            TestEqual(TEXT("Hero unknown values offset"), HeroUnknown.Values, int64(0));
            TestEqual(TEXT("Hero unknown end"), HeroUnknown.Tail, int64(28));
        }
        TestFalse(TEXT("Reject truncated hero unknown"), KyaReadHeroUnknownOffsets(HeroUnknownData, 0, 24, HeroUnknown));
    }
    {
        TArray<uint8> InventoryData;
        InventoryData.SetNumZeroed(44);
        InventoryData[0] = 1;
        InventoryData[36] = 0;
        FKyaInventoryOffsets Inventory;
        if (TestTrue(TEXT("Read projectile inventory fixture"), KyaReadInventoryOffsets(InventoryData, 0, 44, Inventory)))
        {
            TestEqual(TEXT("Inventory present offset"), Inventory.Field0, int64(0));
            TestEqual(TEXT("Inventory u64 offset"), Inventory.Field8, int64(4));
            TestEqual(TEXT("Inventory purchase offset"), Inventory.PurchaseId, int64(20));
            TestEqual(TEXT("Inventory object flag offset"), Inventory.ObjectFlag, int64(36));
            TestFalse(TEXT("Inventory has no object block"), Inventory.bHasObject);
            TestEqual(TEXT("Inventory initial behaviour offset"), Inventory.InitialBehaviourId, int64(40));
            TestEqual(TEXT("Inventory end"), Inventory.Tail, int64(44));
        }
        InventoryData.SetNumZeroed(56);
        InventoryData[0] = 1;
        InventoryData[36] = 1;
        if (TestTrue(TEXT("Read projectile inventory with object"), KyaReadInventoryOffsets(InventoryData, 0, 56, Inventory)))
        {
            TestTrue(TEXT("Inventory has object block"), Inventory.bHasObject);
            TestEqual(TEXT("Inventory object id offset"), Inventory.ObjectId, int64(40));
            TestEqual(TEXT("Inventory object field 18 offset"), Inventory.Field18, int64(44));
            TestEqual(TEXT("Inventory object end"), Inventory.Tail, int64(56));
        }
        InventoryData.SetNumZeroed(8);
        InventoryData[0] = 0;
        if (TestTrue(TEXT("Read empty inventory fixture"), KyaReadInventoryOffsets(InventoryData, 0, 8, Inventory)))
        {
            TestEqual(TEXT("Empty inventory initial behaviour offset"), Inventory.InitialBehaviourId, int64(4));
            TestEqual(TEXT("Empty inventory end"), Inventory.Tail, int64(8));
        }
        InventoryData.SetNum(43);
        TestFalse(TEXT("Reject truncated inventory"), KyaReadInventoryOffsets(InventoryData, 0, 43, Inventory));
    }    {
        TArray<uint8> WolfenData;
        WolfenData.SetNumZeroed(416);
        FKyaWolfenOffsets Wolfen;
        if (TestTrue(TEXT("Read Wolfen fixture"), KyaReadWolfenOffsets(WolfenData, 0, WolfenData.Num(), Wolfen)))
        {
            TestEqual(TEXT("Wolfen fighter prefix offset"), Wolfen.FighterPrefix, int64(0));
            TestEqual(TEXT("Wolfen starts after fighter prefix"), Wolfen.WolfenStart, int64(196));
            TestEqual(TEXT("Wolfen field b74 offset"), Wolfen.FieldB74, int64(196));
            TestEqual(TEXT("Wolfen vision offset"), Wolfen.Vision, int64(200));
            TestEqual(TEXT("Wolfen start sector offset"), Wolfen.StartSector, int64(220));
            TestEqual(TEXT("Wolfen hearing offset"), Wolfen.Hearing, int64(224));
            TestEqual(TEXT("Wolfen detection offset"), Wolfen.VisionDetection, int64(232));
            TestEqual(TEXT("Wolfen field cf4 offset"), Wolfen.FieldCF4, int64(236));
            TestEqual(TEXT("Wolfen walk values offset"), Wolfen.WalkValues, int64(240));
            TestEqual(TEXT("Wolfen fight param offset"), Wolfen.FightParam, int64(292));
            TestEqual(TEXT("Wolfen fixture end"), Wolfen.Tail, int64(416));
        }
        WolfenData.SetNum(415);
        TestFalse(TEXT("Reject truncated Wolfen fight parameter"), KyaReadWolfenOffsets(WolfenData, 0, WolfenData.Num(), Wolfen));
    }
    {
        TArray<uint8> CameraStream;
        CameraStream.SetNumZeroed(112 + 4 + 16 + 56);
        CameraStream[112] = 1;
        int64 Conditions = 0, Config = 0;
        TestTrue(TEXT("Locate camera condition record"), KyaFindCameraConditions(CameraStream, 0, Conditions));
        TestEqual(TEXT("Camera condition count offset"), Conditions, int64(112));
        TestTrue(TEXT("Locate configuration after camera condition"), KyaFindCameraConfig(CameraStream, 0, Config));
        TestEqual(TEXT("Camera configuration offset"), Config, int64(132));
        CameraStream.SetNum(130);
        TestFalse(TEXT("Reject truncated camera condition"), KyaFindCameraConditions(CameraStream, 0, Conditions));
    }
    TestTrue(TEXT("Actor yaw rotates forward toward game X"), KyaActorTransform::ApplyDefaultRotation(
        FVector(0, 0, 1), FVector(0, PI / 2, 0)).Equals(FVector(1, 0, 0), 0.0001));
    TestTrue(TEXT("Actor pitch rotates forward toward negative game Y"), KyaActorTransform::ApplyDefaultRotation(
        FVector(0, 0, 1), FVector(PI / 2, 0, 0)).Equals(FVector(0, -1, 0), 0.0001));
    TestTrue(TEXT("Actor roll rotates right toward game Y"), KyaActorTransform::ApplyDefaultRotation(
        FVector(1, 0, 0), FVector(0, 0, PI / 2)).Equals(FVector(0, 1, 0), 0.0001));
    TestTrue(TEXT("Yaw-only actor class removes pitch"), KyaActorTransform::ApplyClassRotation(
        FVector(0, 0, 1), FVector(PI / 4, 0, 0), 6).Equals(FVector(0, 0, 1), 0.0001));
    TestTrue(TEXT("Euler-matrix actor class retains pitch"), KyaActorTransform::ApplyClassRotation(
        FVector(0, 0, 1), FVector(PI / 4, 0, 0), 1).Equals(FVector(0, -FMath::Sqrt(0.5), FMath::Sqrt(0.5)), 0.0001));
    {
        // One block with one operation and one condition.
        TArray<uint8> Stream;
        Stream.SetNumZeroed(40);
        Stream[0] = 1; Stream[4] = 1; Stream[20] = 1;
        TArray<FKyaConditionedOffsets> Blocks;
        int64 End = 0;
        if (TestTrue(TEXT("Read conditioned operation stream"), KyaReadConditionedOffsets(Stream, 0, Blocks, End)))
        {
            TestEqual(TEXT("Conditioned block count"), Blocks.Num(), 1);
            TestEqual(TEXT("Operation position"), Blocks[0].Operations[0], int64(8));
            TestEqual(TEXT("Condition position"), Blocks[0].Conditions[0], int64(24));
            TestEqual(TEXT("Conditioned stream end"), End, int64(40));
        }
        TestFalse(TEXT("Reject truncated conditioned operation stream"), KyaReadConditionedOffsets(
            MakeArrayView(Stream.GetData(), Stream.Num() - 1), 0, Blocks, End));
    }
    const FString Filename = FPlatformMisc::GetEnvironmentVariable(TEXT("KYA_TEST_BANK"));
    if (Filename.IsEmpty())
    {
        AddWarning(TEXT("Set KYA_TEST_BANK to run the level bank round-trip test."));
        return true;
    }
    TArray<uint8> Input;
    if (!TestTrue(TEXT("Read bank"), FFileHelper::LoadFileToArray(Input, *Filename))) return false;
    const uint8* Cursor = Input.GetData();
    UKyaLevelBankFactory* Factory = NewObject<UKyaLevelBankFactory>();
    UKyaLevelBank* Bank = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
        GetTransientPackage(), NAME_None, RF_Transient, nullptr, TEXT("bnk"), Cursor,
        Input.GetData() + Input.Num(), GWarn));
    if (!TestNotNull(TEXT("Import bank"), Bank)) return false;
    TestTrue(TEXT("Imported bank has current schema"), Bank->HasCurrentImportSchema());
    Bank->ImportSchemaVersion = 0;
    TestFalse(TEXT("Old bank schema is detected"), Bank->HasCurrentImportSchema());
    Bank->ImportSchemaVersion = UKyaLevelBank::CurrentImportSchemaVersion;
    UWorld* PreviewWorld = UWorld::CreateWorld(EWorldType::EditorPreview, false);
    if (!TestNotNull(TEXT("Create preview world"), PreviewWorld)) return false;
    AKyaLevelPreview* Preview = PreviewWorld->SpawnActor<AKyaLevelPreview>();
    if (!TestNotNull(TEXT("Spawn level preview"), Preview))
    {
        PreviewWorld->DestroyWorld(false);
        return false;
    }
    Preview->Bank = Bank;
    Preview->OnConstruction(FTransform::Identity);
    Preview->PreviewPathIndex = 0;
    Preview->PreviewActorIndex = 0;
    Preview->OnConstruction(FTransform::Identity);
    auto PreviewVertexCount = [&]() {
        int32 Count = 0;
        for (int32 Section = 0; Section < Preview->Mesh->GetNumSections(); ++Section)
            if (const FProcMeshSection* MeshSection = Preview->Mesh->GetProcMeshSection(Section)) Count += MeshSection->ProcVertexBuffer.Num();
        return Count;
    };
    const int32 VerticesWithoutActor = PreviewVertexCount();
    int32 ActorWithMesh = INDEX_NONE;
    for (int32 ActorIndex = 0; ActorIndex < Bank->SceneEnvironment.Actors.Num() && ActorWithMesh == INDEX_NONE; ++ActorIndex)
    {
        int32 Ordinal = Bank->SceneEnvironment.Actors[ActorIndex].MeshIndex;
        if (Ordinal < 0) continue;
        for (const FKyaBankEntry& MeshEntry : Bank->Entries)
            if (MeshEntry.Type == 4 && MeshEntry.Subtype == 1 && Ordinal-- == 0)
            {
                TArray<FKyaG3DDecodedStrip> ModelStrips;
                if (KyaG3D::DecodeHierarchyStrips(MakeArrayView(Input.GetData() + MeshEntry.Offset, int32(MeshEntry.Size)), ModelStrips) && !ModelStrips.IsEmpty()) ActorWithMesh = ActorIndex;
                break;
            }
    }
    if (TestTrue(TEXT("Find a supported level-one actor mesh"), ActorWithMesh != INDEX_NONE))
    {
        Preview->PreviewActorIndex = ActorWithMesh;
        Preview->bShowActorMesh = true;
        Preview->OnConstruction(FTransform::Identity);
        TestTrue(TEXT("Actor mesh adds preview geometry"), PreviewVertexCount() > VerticesWithoutActor);
        Preview->bShowActorMesh = false;
        Preview->PreviewActorIndex = 0;
        Preview->OnConstruction(FTransform::Identity);
    }
    TArray<UKyaActorMarkerComponent*> ActorMarkers;
    Preview->GetComponents<UKyaActorMarkerComponent>(ActorMarkers);
    if (TestEqual(TEXT("Actor marker count"), ActorMarkers.Num(), 1))
    {
        const FVector OriginalActorPosition = Bank->SceneEnvironment.Actors[0].Position;
        ActorMarkers[0]->SetRelativeLocation(FVector(100, 200, 300));
        ActorMarkers[0]->PostEditComponentMove(true);
        TestEqual(TEXT("Moving actor marker updates bank"), Bank->SceneEnvironment.Actors[0].Position, FVector(1, 3, 2));
        Bank->SceneEnvironment.Actors[0].Position = OriginalActorPosition;
        Preview->OnConstruction(FTransform::Identity);
    }
    TArray<USplineComponent*> PathPreviews;
    Preview->GetComponents<USplineComponent>(PathPreviews);
    if (TestEqual(TEXT("Path preview component count"), PathPreviews.Num(), 1))
    {
        TestEqual(TEXT("Path preview point count"), PathPreviews[0]->GetNumberOfSplinePoints(), Bank->SceneEnvironment.Paths[0].Points.Num());
        const FVector4 P = Bank->SceneEnvironment.Paths[0].Points[0];
        TestTrue(TEXT("Path preview coordinate conversion"), PathPreviews[0]->GetLocationAtSplinePoint(0, ESplineCoordinateSpace::Local).Equals(FVector(P.X, P.Z, P.Y) * 100.0));
        if (PathPreviews[0]->GetNumberOfSplinePoints() >= 2)
        {
            const FVector A = PathPreviews[0]->GetLocationAtSplinePoint(0, ESplineCoordinateSpace::Local);
            const FVector B = PathPreviews[0]->GetLocationAtSplinePoint(1, ESplineCoordinateSpace::Local);
            TestTrue(TEXT("Path guide interpolates linearly between points"),
                PathPreviews[0]->GetLocationAtSplineInputKey(0.5f, ESplineCoordinateSpace::Local).Equals((A + B) * 0.5));
        }
        TArray<UKyaPathPointComponent*> PathMarkers;
        Preview->GetComponents<UKyaPathPointComponent>(PathMarkers);
        if (TestEqual(TEXT("Path marker count"), PathMarkers.Num(), Bank->SceneEnvironment.Paths[0].Points.Num()) && !PathMarkers.IsEmpty())
        {
            UKyaPathPointComponent** Found = PathMarkers.FindByPredicate([](const UKyaPathPointComponent* Item) { return Item->PointIndex == 0; });
            if (TestNotNull(TEXT("First path marker"), Found))
            {
                (*Found)->SetRelativeLocation(FVector(100, 200, 300));
                (*Found)->PostEditComponentMove(true);
                TestTrue(TEXT("Moving path point updates bank and preserves W"), Bank->SceneEnvironment.Paths[0].Points[0].Equals(FVector4(1, 3, 2, P.W)));
                TestTrue(TEXT("Moving path point updates guide"), PathPreviews[0]->GetLocationAtSplinePoint(0, ESplineCoordinateSpace::Local).Equals(FVector(100, 200, 300)));
                Bank->SceneEnvironment.Paths[0].Points[0] = P;
                Preview->OnConstruction(FTransform::Identity);
            }
        }
    }
    TArray<UKyaWaypointComponent*> Markers;
    Preview->GetComponents<UKyaWaypointComponent>(Markers);
    if (TestEqual(TEXT("Waypoint marker count"), Markers.Num(), Bank->SceneEnvironment.Waypoints.Num()) && !Markers.IsEmpty())
    {
        UKyaWaypointComponent* Marker = *Markers.FindByPredicate([](const UKyaWaypointComponent* Item) { return Item->WaypointIndex == 0; });
        const FVector OriginalPosition = Bank->SceneEnvironment.Waypoints[0].Position;
        Marker->SetRelativeLocation(FVector(100, 200, 300));
        Marker->PostEditComponentMove(true);
        TestEqual(TEXT("Moving waypoint marker updates game coordinates"), Bank->SceneEnvironment.Waypoints[0].Position, FVector(1, 3, 2));
        Preview->OnConstruction(FTransform::Identity);
        TArray<UKyaWaypointComponent*> ReconstructedMarkers;
        Preview->GetComponents<UKyaWaypointComponent>(ReconstructedMarkers);
        TestTrue(TEXT("Construction preserves selected marker identity"), ReconstructedMarkers.Contains(Marker));
        Bank->SceneEnvironment.Waypoints[0].Position = OriginalPosition;
        Preview->OnConstruction(FTransform::Identity);
        TestTrue(TEXT("Reconstruction restores marker position from bank"), Marker->GetRelativeLocation().Equals(
            FVector(OriginalPosition.X, OriginalPosition.Z, OriginalPosition.Y) * 100.0));
    }
    TestTrue(TEXT("Default preview creates textured sections without a material override"), Preview->Mesh->GetNumSections() > 1);
    bool HasPreviewTriangles = false;
    for (int32 Section = 0; Section < Preview->Mesh->GetNumSections(); ++Section)
        if (const FProcMeshSection* MeshSection = Preview->Mesh->GetProcMeshSection(Section))
            HasPreviewTriangles |= !MeshSection->ProcIndexBuffer.IsEmpty();
    TestTrue(TEXT("Preview has background geometry"), HasPreviewTriangles);
    TestTrue(TEXT("Untextured preview uses a generated material"), Preview->Mesh->GetMaterial(0) &&
        Preview->Mesh->GetMaterial(0) != UMaterial::GetDefaultMaterial(MD_Surface));
    auto CountPreviewVertices = [&]() {
        int64 Count = 0;
        for (int32 Section = 0; Section < Preview->Mesh->GetNumSections(); ++Section)
            if (const FProcMeshSection* MeshSection = Preview->Mesh->GetProcMeshSection(Section))
                Count += MeshSection->ProcVertexBuffer.Num();
        return Count;
    };
    const int64 BackgroundVertices = CountPreviewVertices();
    Preview->bShowAllActorMeshes = true;
    Preview->OnConstruction(FTransform::Identity);
    TestTrue(TEXT("All actor preview adds model geometry"), CountPreviewVertices() > BackgroundVertices);
    Preview->bShowAllActorMeshes = false;
    Preview->OnConstruction(FTransform::Identity);
    TestEqual(TEXT("Disabling all actor preview restores background geometry"), CountPreviewVertices(), BackgroundVertices);
    const FString SectorFilename = FPaths::Combine(FPaths::GetPath(Filename), TEXT("SECT1.BNK"));
    TArray<uint8> SectorInput;
    if (TestTrue(TEXT("Read sector bank"), FFileHelper::LoadFileToArray(SectorInput, *SectorFilename)))
    {
        const uint8* SectorCursor = SectorInput.GetData();
        UKyaLevelBank* Sector = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
            GetTransientPackage(), NAME_None, RF_Transient, nullptr, TEXT("bnk"), SectorCursor,
            SectorInput.GetData() + SectorInput.Num(), GWarn));
        if (TestNotNull(TEXT("Import sector bank"), Sector))
        {
            const FKyaBankEntry* SectorMesh = Sector->Entries.FindByPredicate([](const FKyaBankEntry& Entry) {
                return Entry.Type == 4 && Entry.Subtype == 1;
            });
            if (TestNotNull(TEXT("Sector mesh entry"), SectorMesh))
            {
                TArray<int32> ClusterStrips;
                int32 ClusterMaterialBank = INDEX_NONE;
                TestTrue(TEXT("Read sector cluster strips"), KyaG3D::ReadClusterStrips(
                    MakeArrayView(SectorInput.GetData() + SectorMesh->Offset, int32(SectorMesh->Size)), ClusterStrips, &ClusterMaterialBank));
                TestEqual(TEXT("Sector cluster strip count"), ClusterStrips.Num(), 17);
                const auto MeshData = MakeArrayView(SectorInput.GetData() + SectorMesh->Offset, int32(SectorMesh->Size));
                TArray<KyaG3D::FClusterStripReference> TreeStrips;
                TArray<int32> ClusterHierarchies;
                if (TestTrue(TEXT("Read nested sector cluster strips"), KyaG3D::ReadClusterTreeStrips(MeshData, TreeStrips, &ClusterHierarchies)))
                {
                    TestTrue(TEXT("Cluster tree includes root strips"), TreeStrips.Num() >= ClusterStrips.Num());
                    TestEqual(TEXT("Sector one cluster hierarchy count"), ClusterHierarchies.Num(), 6);
                    TArray<int32> Expanded;
                    TestTrue(TEXT("Follow sector one hierarchy links"), KyaG3D::ExpandClusterHierarchies(MeshData, ClusterHierarchies, Expanded));
                    TestTrue(TEXT("Sector one linked hierarchies include roots"), Expanded.Num() >= ClusterHierarchies.Num());
                    TArray<FKyaG3DStripReference> HierarchyStrips;
                    if (TestTrue(TEXT("Read sector hierarchy strips"), KyaG3D::ReadHierarchyStrips(MeshData, HierarchyStrips)))
                    {
                        int32 Referenced = 0;
                        for (const FKyaG3DStripReference& Strip : HierarchyStrips)
                            if (ClusterHierarchies.Contains(Strip.HierarchyOffset)) ++Referenced;
                        TestEqual(TEXT("Sector one cluster hierarchy strip count"), Referenced, 589);
                    }
                }
                int32 ResolvedMaterials = 0;
                for (int32 Strip : ClusterStrips)
                {
                    uint64 MaterialHash = 0;
                    if (KyaG3D::ReadStripMaterialHash(MeshData, Strip, ClusterMaterialBank, MaterialHash)) ++ResolvedMaterials;
                    uint32 Count = 0, Primitive = 0;
                    TArray<FKyaG3DVertex> Vertices;
                    TArray<FKyaG3DVertexAttributes> Attributes;
                    TArray<uint32> Sections, Indices;
                    if (!KyaG3D::ReadFirstStripGifTag(MeshData, Strip, Count, Primitive) ||
                        !KyaG3D::ReadStripVertices(MeshData, Strip, Vertices, Sections) ||
                        !KyaG3D::ReadStripAttributes(MeshData, Strip, Attributes) ||
                        !KyaG3D::BuildStripTriangles(Vertices, Sections, Primitive, Indices))
                        AddError(FString::Printf(TEXT("Cannot decode sector cluster strip at 0x%x"), Strip));
                }
                TestTrue(TEXT("Sector cluster material hashes resolve"), ResolvedMaterials > 0);
            }
            Preview->SectorBanks.Add(Sector);
            Preview->OnConstruction(FTransform::Identity);
            TestTrue(TEXT("Sector adds preview geometry"), CountPreviewVertices() > BackgroundVertices);
            FKyaBankEntry* SectorBackground = Sector->Entries.FindByPredicate([](const FKyaBankEntry& Item) {
                return Item.Type == 4 && Item.Subtype == 2;
            });
            if (TestNotNull(TEXT("Sector one background entry"), SectorBackground))
            {
                TArray<FKyaG3DDecodedStrip> BackgroundStrips;
                const auto BackgroundData = MakeArrayView(SectorInput.GetData() + SectorBackground->Offset, int32(SectorBackground->Size));
                TArray<int32> BackgroundHierarchies, CameraRelative;
                TestTrue(TEXT("Read sector background hierarchies"), KyaG3D::ReadHierarchyOffsets(BackgroundData, BackgroundHierarchies));
                if (TestTrue(TEXT("Classify sector background camera-follow hierarchies"), KyaG3D::ReadCameraRelativeBackgroundHierarchies(BackgroundData, CameraRelative)))
                {
                    TestEqual(TEXT("Sector one camera-follow hierarchy count"), CameraRelative.Num(), 1);
                    if (CameraRelative.Num() == 1)
                    {
                        FMatrix AtOrigin, AtCamera;
                        const FVector Camera(12, 34, 56);
                        if (TestTrue(TEXT("Read background matrix at origin"), KyaG3D::ReadBackgroundWorldMatrix(BackgroundData, CameraRelative[0], CameraRelative, FVector::ZeroVector, AtOrigin)) &&
                            TestTrue(TEXT("Read background matrix at camera"), KyaG3D::ReadBackgroundWorldMatrix(BackgroundData, CameraRelative[0], CameraRelative, Camera, AtCamera)))
                            TestTrue(TEXT("Camera affects background transform"), !AtOrigin.Equals(AtCamera));
                    }
                }
                if (TestTrue(TEXT("Decode sector background for visibility test"), KyaG3D::DecodeHierarchyStrips(
                    BackgroundData, BackgroundStrips)))
                {
                    int64 SectorBackgroundVertices = 0;
                    for (const FKyaG3DDecodedStrip& Strip : BackgroundStrips) SectorBackgroundVertices += Strip.Vertices.Num();
                    const int64 WithSectorBackground = CountPreviewVertices();
                    SectorBackground->Subtype = 0;
                    Preview->OnConstruction(FTransform::Identity);
                    TestEqual(TEXT("Sector background replaces common background"),
                        WithSectorBackground - CountPreviewVertices(), SectorBackgroundVertices - BackgroundVertices);
                    SectorBackground->Subtype = 2;
                    Preview->OnConstruction(FTransform::Identity);
                }
            }
            const int64 OriginalBackgroundFlags = Bank->SceneEnvironment.BackgroundFlags;
            auto CollectPreviewPositions = [&]() {
                TArray<FVector> Result;
                for (int32 Section = 0; Section < Preview->Mesh->GetNumSections(); ++Section)
                    if (const FProcMeshSection* MeshSection = Preview->Mesh->GetProcMeshSection(Section))
                        for (const FProcMeshVertex& Vertex : MeshSection->ProcVertexBuffer) Result.Add(Vertex.Position);
                return Result;
            };
            Bank->SceneEnvironment.BackgroundFlags |= 1;
            Preview->OnConstruction(FTransform::Identity);
            Preview->SetBackgroundCameraPosition(FVector::ZeroVector);
            const TArray<FVector> AtOrigin = CollectPreviewPositions();
            Preview->SetBackgroundCameraPosition(FVector(1200, 3400, 5600));
            const TArray<FVector> AtCamera = CollectPreviewPositions();
            int32 MovedVertices = 0;
            if (TestEqual(TEXT("Camera movement preserves vertex count"), AtCamera.Num(), AtOrigin.Num()))
                for (int32 Index = 0; Index < AtCamera.Num(); ++Index)
                    if (!AtOrigin[Index].Equals(AtCamera[Index], 0.01)) ++MovedVertices;
            TestTrue(TEXT("Camera-follow background moves"), MovedVertices > 0 && MovedVertices < AtCamera.Num());
            Bank->SceneEnvironment.BackgroundFlags &= ~int64(1);
            Preview->OnConstruction(FTransform::Identity);
            const TArray<FVector> FixedBackground = CollectPreviewPositions();
            Preview->SetBackgroundCameraPosition(FVector(1200, 3400, 5600));
            const TArray<FVector> FixedAfterCamera = CollectPreviewPositions();
            TestTrue(TEXT("Disabled background flag keeps all vertices fixed"), FixedBackground == FixedAfterCamera);
            Bank->SceneEnvironment.BackgroundFlags = OriginalBackgroundFlags;
            Preview->OnConstruction(FTransform::Identity);
            bool HasMaskedMaterial = false;
            for (int32 Section = 1; Section < Preview->Mesh->GetNumSections(); ++Section)
                if (UMaterialInterface* Material = Preview->Mesh->GetMaterial(Section))
                    HasMaskedMaterial |= Material->GetBlendMode() == BLEND_Masked;
            TestTrue(TEXT("Alpha-tested sector materials use masked preview"), HasMaskedMaterial);
            Preview->TextureMaterial = UMaterial::GetDefaultMaterial(MD_Surface);
            Preview->OnConstruction(FTransform::Identity);
            TestTrue(TEXT("Supported background textures create material sections"), Preview->Mesh->GetNumSections() > 1);
            const int32 BackgroundTextureEntry = Bank->BackgroundTextureEntry;
            Bank->BackgroundTextureEntry = INDEX_NONE;
            Preview->OnConstruction(FTransform::Identity);
            TestTrue(TEXT("Sector background creates textured sections independently"), Preview->Mesh->GetNumSections() > 1);
            Bank->BackgroundTextureEntry = BackgroundTextureEntry;
        }
    }
    {
        TArray<uint8> Sector7Input;
        const FString Sector7Filename = FPaths::Combine(FPaths::GetPath(Filename), TEXT("SECT7.BNK"));
        if (TestTrue(TEXT("Read sector seven bank"), FFileHelper::LoadFileToArray(Sector7Input, *Sector7Filename)))
        {
            const uint8* SectorCursor = Sector7Input.GetData();
            UKyaLevelBank* Sector = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
                GetTransientPackage(), NAME_None, RF_Transient, nullptr, TEXT("bnk"), SectorCursor,
                Sector7Input.GetData() + Sector7Input.Num(), GWarn));
            if (TestNotNull(TEXT("Import sector seven bank"), Sector))
            {
                Preview->SectorBanks.Reset();
                Preview->SectorBanks.Add(Sector);
                Preview->OnConstruction(FTransform::Identity);
                TestTrue(TEXT("Hierarchy-only sector adds preview geometry"), CountPreviewVertices() > BackgroundVertices);
                const FKyaBankEntry* SectorMesh = Sector->Entries.FindByPredicate([](const FKyaBankEntry& Entry) {
                    return Entry.Type == 4 && Entry.Subtype == 1;
                });
                if (TestNotNull(TEXT("Sector seven mesh entry"), SectorMesh))
                {
                    const auto MeshData = MakeArrayView(Sector7Input.GetData() + SectorMesh->Offset, int32(SectorMesh->Size));
                    TArray<KyaG3D::FClusterStripReference> StaticStrips;
                    TArray<int32> Hierarchies;
                    if (TestTrue(TEXT("Read sector seven cluster"), KyaG3D::ReadClusterTreeStrips(MeshData, StaticStrips, &Hierarchies)))
                    {
                        TestEqual(TEXT("Sector seven has no static strips"), StaticStrips.Num(), 0);
                        TestEqual(TEXT("Sector seven cluster hierarchy count"), Hierarchies.Num(), 1);
                        TArray<int32> Expanded;
                        TestTrue(TEXT("Follow sector seven hierarchy links"), KyaG3D::ExpandClusterHierarchies(MeshData, Hierarchies, Expanded));
                        TestTrue(TEXT("Sector seven linked hierarchies include root"), Expanded.Num() >= Hierarchies.Num());
                        TArray<FKyaG3DStripReference> References;
                        if (TestTrue(TEXT("Read sector seven hierarchy strips"), KyaG3D::ReadHierarchyStrips(MeshData, References)))
                        {
                            int32 Referenced = 0;
                            for (const FKyaG3DStripReference& Strip : References)
                                if (Hierarchies.Contains(Strip.HierarchyOffset)) ++Referenced;
                            TestEqual(TEXT("Sector seven referenced hierarchy strips"), Referenced, 27);
                        }
                    }
                }
            }
        }
    }
    {
        const FKyaBankEntry& CommonEntry = Bank->Entries[Bank->SceneTextureEntry];
        const auto CommonData = MakeArrayView(Input.GetData() + CommonEntry.Offset, int32(CommonEntry.Size));
        TMap<uint64, int32> CommonMaterials;
        KyaG2D::ReadMaterials(CommonData, CommonMaterials);
        for (int32 SectorNumber = 1; SectorNumber <= 13; ++SectorNumber)
        {
            TArray<uint8> Bytes;
            const FString Path = FPaths::Combine(FPaths::GetPath(Filename), FString::Printf(TEXT("SECT%d.BNK"), SectorNumber));
            if (!FFileHelper::LoadFileToArray(Bytes, *Path)) continue;
            const uint8* Position = Bytes.GetData();
            UKyaLevelBank* Sector = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
                GetTransientPackage(), NAME_None, RF_Transient, nullptr, TEXT("bnk"), Position, Bytes.GetData() + Bytes.Num(), GWarn));
            if (!Sector) continue;
            const FKyaBankEntry* MeshEntry = Sector->Entries.FindByPredicate([](const FKyaBankEntry& E) { return E.Type == 4 && E.Subtype == 1; });
            const FKyaBankEntry* TextureEntry = Sector->Entries.FindByPredicate([](const FKyaBankEntry& E) { return E.Type == 5 && E.Subtype == 1; });
            if (!MeshEntry || !TextureEntry) continue;
            const auto MeshData = MakeArrayView(Bytes.GetData() + MeshEntry->Offset, int32(MeshEntry->Size));
            const auto TextureData = MakeArrayView(Bytes.GetData() + TextureEntry->Offset, int32(TextureEntry->Size));
            TMap<uint64, int32> Materials;
            KyaG2D::ReadMaterials(TextureData, Materials);
            TArray<KyaG3D::FClusterStripReference> Strips;
            TMap<FString, int32> BackgroundBlends;
            {
                TArray<FKyaG3DStripReference> BackgroundRefs;
                if (KyaG3D::ReadHierarchyStrips(MeshData, BackgroundRefs))
                {
                    for (const FKyaG3DStripReference& Ref : BackgroundRefs)
                    {
                        uint32 VertexCount = 0, Primitive = 0;
                        if (!KyaG3D::ReadFirstStripGifTag(MeshData, Ref.StripOffset, VertexCount, Primitive)) continue;
                        ++BackgroundBlends.FindOrAdd(FString::Printf(TEXT("ABE=%d prim=%x verts=%d"),
                            int32(Primitive & 1), Primitive & ~1u, VertexCount));
                    }
                }
                for (const auto& Blend : BackgroundBlends)
                    AddInfo(FString::Printf(TEXT("SECT%d background %s: %d strips"), SectorNumber, *Blend.Key, Blend.Value));
            }
            if (!KyaG3D::ReadClusterTreeStrips(MeshData, Strips)) continue;
            int32 NoHash = 0, Missing = 0, Decoded = 0, Failed = 0;
            TMap<uint32, int32> Formats;
            TMap<FString, int32> FailureReasons;
            TMap<FString, int32> AlphaTests;
            TMap<FString, int32> BlendTests;
            for (const auto& Strip : Strips)
            {
                uint64 Hash = 0;
                if (!KyaG3D::ReadStripMaterialHash(MeshData, Strip.StripOffset, Strip.MaterialBank, Hash)) { ++NoHash; continue; }
                const int32* Offset = Materials.Find(Hash);
                const bool Local = Offset != nullptr;
                if (!Offset) Offset = CommonMaterials.Find(Hash);
                if (!Offset) { ++Missing; continue; }
                const TConstArrayView<uint8> Source = Local ? TConstArrayView<uint8>(TextureData) : TConstArrayView<uint8>(CommonData);
                uint64 Tex0 = 0;
                if (KyaG2D::ReadMaterialTex0(Source, *Offset, 0, Tex0)) ++Formats.FindOrAdd(uint32((Tex0 >> 20) & 63));
                uint64 TestRegister = 0;
                if (KyaG2D::ReadMaterialRegister(Source, *Offset, 0, 0x47, TestRegister))
                    ++AlphaTests.FindOrAdd(FString::Printf(TEXT("ATE=%d ATST=%d AREF=%d AFAIL=%d"),
                        uint32(TestRegister & 1), uint32((TestRegister >> 1) & 7),
                        uint32((TestRegister >> 4) & 255), uint32((TestRegister >> 12) & 3)));
                else ++AlphaTests.FindOrAdd(TEXT("no TEST register"));
                {
                    uint64 Alpha1 = 0, Alpha2 = 0;
                    const bool bHasAlpha1 = KyaG2D::ReadMaterialRegister(Source, *Offset, 0, 0x00, Alpha1);
                    const bool bHasAlpha2 = KyaG2D::ReadMaterialRegister(Source, *Offset, 0, 0x01, Alpha2);
                    uint32 VertexCount = 0, Primitive = 0;
                    const bool bHasPrim = KyaG3D::ReadFirstStripGifTag(MeshData, Strip.StripOffset, VertexCount, Primitive);
                    const uint32 Abe = bHasPrim ? (Primitive & 1) : 2;
                    const uint32 A1 = uint32(Alpha1 & 3), B1 = uint32((Alpha1 >> 2) & 3);
                    const uint32 C1 = uint32((Alpha1 >> 4) & 3), D1 = uint32((Alpha1 >> 6) & 3);
                    const uint32 A2 = uint32(Alpha2 & 3), B2 = uint32((Alpha2 >> 2) & 3);
                    const uint32 C2 = uint32((Alpha2 >> 4) & 3), D2 = uint32((Alpha2 >> 6) & 3);
                    const FString Key = FString::Printf(TEXT("ABE=%d A1=%d%d%d%d A2=%d%d%d%d%s"),
                        Abe, A1, B1, C1, D1, A2, B2, C2, D2,
                        (bHasAlpha1 || bHasAlpha2) ? TEXT("") : TEXT(" (no ALPHA regs)"));
                    ++BlendTests.FindOrAdd(Key);
                }
                int32 Width = 0, Height = 0;
                TArray<FColor> Pixels;
                if (KyaG2D::DecodeMaterialTexture(Source, *Offset, Width, Height, Pixels)) ++Decoded;
                else
                {
                    ++Failed;
                    KyaG2D::FBitmapReference Bitmap;
                    TArray<TPair<int32, int32>> UploadLists;
                    TArray<KyaG2D::FUploadTransfer> Uploads;
                    FString Reason;
                    if (!KyaG2D::ReadMaterialBitmap(Source, *Offset, Bitmap)) Reason = TEXT("bitmap");
                    else if (!KyaG2D::ReadMaterialTex0(Source, *Offset, 0, Tex0)) Reason = TEXT("tex0");
                    else if (!KyaG2D::ReadUploadLists(Source, Bitmap.PaletteOffset == INDEX_NONE ? Bitmap.BitmapOffset : Bitmap.PaletteOffset, UploadLists)) Reason = TEXT("upload lists");
                    else if (UploadLists.IsEmpty() || !KyaG2D::ReadUploadTransfers(Source, UploadLists[0].Key, UploadLists[0].Value, Uploads)) Reason = TEXT("upload transfers");
                    else
                    {
                        TArray<uint8> Memory;
                        Memory.SetNumZeroed(4 * 1024 * 1024);
                        bool UploadOk = true;
                        for (const auto& Upload : Uploads)
                            if (Upload.DestinationFormat() == 0 ? !KyaG2D::WriteGs32(Source, Upload, Memory) : !KyaG2D::WriteGsIndexed(Source, Upload, Memory)) UploadOk = false;
                        if (!UploadOk)
                        {
                            Reason = TEXT("upload format");
                            for (const auto& Upload : Uploads) Reason += FString::Printf(TEXT(" %x"), Upload.DestinationFormat());
                        }
                        else
                        {
                            TArray<uint8> Indices;
                            TArray<FColor> Colors;
                            const uint32 Format = uint32((Tex0 >> 20) & 63);
                            if (Format == 0 && !KyaG2D::ReadGs32(Memory, Tex0, Colors)) Reason = TEXT("direct pixels");
                            else if (Format == 0x13 && !KyaG2D::ReadGs8(Memory, Tex0, Indices)) Reason = TEXT("8-bit pixels");
                            else if (Format == 0x14 && !KyaG2D::ReadGs4(Memory, Tex0, Indices)) Reason = TEXT("4-bit pixels");
                            else Reason = FString::Printf(TEXT("palette format=%x csm=%d csa=%d psm=%x"),
                                uint32((Tex0 >> 51) & 15), uint32((Tex0 >> 55) & 1), uint32((Tex0 >> 56) & 31), Format);
                        }
                    }
                    ++FailureReasons.FindOrAdd(Reason);
                }
            }
            {
                TArray<FKyaG3DSprite> SectorSprites;
                if (KyaG3D::ReadSprites(MeshData, SectorSprites))
                {
                    FString Tags;
                    for (const FKyaG3DSprite& Sprite : SectorSprites)
                        for (uint32 Tag : Sprite.ChunkTags)
                        {
                            const TCHAR Chars[5] = { TCHAR(Tag & 0xff), TCHAR((Tag >> 8) & 0xff), TCHAR((Tag >> 16) & 0xff), TCHAR((Tag >> 24) & 0xff), 0 };
                            Tags += FString(Chars) + TEXT(",");
                        }
                    AddInfo(FString::Printf(TEXT("SECT%d sprites=%d rects=%d material0=%d tags=%s"), SectorNumber,
                        SectorSprites.Num(), SectorSprites[0].RectCount, SectorSprites[0].MaterialIndex, *Tags));
                    if (SectorNumber == 2) TestEqual(TEXT("Sector two sprite count"), SectorSprites.Num(), 3);
                    if (SectorNumber == 4) TestEqual(TEXT("Sector four sprite count"), SectorSprites.Num(), 5);
                }
            }
            AddInfo(FString::Printf(TEXT("SECT%d cluster materials: strips=%d no-hash=%d missing=%d decoded=%d decode-failed=%d"),
                SectorNumber, Strips.Num(), NoHash, Missing, Decoded, Failed));
            for (const auto& Format : Formats)
                AddInfo(FString::Printf(TEXT("SECT%d TEX0 format 0x%x: %d strips"), SectorNumber, Format.Key, Format.Value));
            for (const auto& Reason : FailureReasons)
                AddInfo(FString::Printf(TEXT("SECT%d texture failure %s: %d strips"), SectorNumber, *Reason.Key, Reason.Value));
            for (const auto& Alpha : AlphaTests)
                AddInfo(FString::Printf(TEXT("SECT%d alpha %s: %d strips"), SectorNumber, *Alpha.Key, Alpha.Value));
            for (const auto& Blend : BlendTests)
                AddInfo(FString::Printf(TEXT("SECT%d blend %s: %d strips"), SectorNumber, *Blend.Key, Blend.Value));
        }
    }
    {
        if (Bank->Entries.IsValidIndex(Bank->BackgroundMeshEntry))
        {
            const FKyaBankEntry& BackgroundEntry = Bank->Entries[Bank->BackgroundMeshEntry];
            if (BackgroundEntry.Offset >= 0 && BackgroundEntry.Size > 0 && BackgroundEntry.Offset <= Input.Num() &&
                BackgroundEntry.Size <= Input.Num() - BackgroundEntry.Offset)
            {
                const auto BackgroundData = MakeArrayView(Input.GetData() + BackgroundEntry.Offset, int32(BackgroundEntry.Size));
                TArray<FKyaG3DStripReference> BackgroundRefs;
                TArray<FKyaG3DDecodedStrip> BackgroundDecoded;
                const bool bRefs = KyaG3D::ReadHierarchyStrips(BackgroundData, BackgroundRefs);
                const bool bDecoded = KyaG3D::DecodeHierarchyStrips(BackgroundData, BackgroundDecoded);
                TMap<FString, int32> CommonBlends;
                if (bRefs)
                {
                    for (const FKyaG3DStripReference& Ref : BackgroundRefs)
                    {
                        uint32 VertexCount = 0, Primitive = 0;
                        if (!KyaG3D::ReadFirstStripGifTag(BackgroundData, Ref.StripOffset, VertexCount, Primitive)) continue;
                        ++CommonBlends.FindOrAdd(FString::Printf(TEXT("ABE=%d prim=%x"), int32(Primitive & 1), Primitive & ~1u));
                    }
                }
                AddInfo(FString::Printf(TEXT("Common background: refs_ok=%d refs=%d decoded_ok=%d decoded=%d"),
                    bRefs, BackgroundRefs.Num(), bDecoded, BackgroundDecoded.Num()));
                for (const auto& Blend : CommonBlends)
                    AddInfo(FString::Printf(TEXT("Common background %s: %d strips"), *Blend.Key, Blend.Value));
            }
        }
    }
    {
        TArray<int32> MeshEntries;
        for (int32 Index = 0; Index < Bank->Entries.Num(); ++Index)
            if (Bank->Entries[Index].Type == 4 && Bank->Entries[Index].Subtype == 1) MeshEntries.Add(Index);
        int32 Models = 0, HierarchyOk = 0, ClusterOk = 0, HierarchyStrips = 0, ClusterStrips = 0;
        int32 MatrixOk = 0, MatrixFail = 0, TexturedStrips = 0, NormalStrips = 0, NormalMismatch = 0;
        TMap<int32, int32> MeshHierarchy, MeshCluster;
        for (const FKyaActorPlacement& Placement : Bank->SceneEnvironment.Actors)
        {
            if (Placement.MeshIndex < 0 || Placement.MeshIndex >= MeshEntries.Num()) continue;
            ++Models;
            if (MeshHierarchy.Contains(Placement.MeshIndex)) continue;
            const FKyaBankEntry& Entry = Bank->Entries[MeshEntries[Placement.MeshIndex]];
            if (Entry.Offset < 0 || Entry.Size <= 0 || Entry.Offset > Input.Num() || Entry.Size > Input.Num() - Entry.Offset) continue;
            const auto Data = MakeArrayView(Input.GetData() + Entry.Offset, int32(Entry.Size));
            TArray<FKyaG3DDecodedStrip> Decoded;
            const bool bHierarchy = KyaG3D::DecodeHierarchyStrips(Data, Decoded) && !Decoded.IsEmpty();
            TArray<KyaG3D::FClusterStripReference> Cluster;
            const bool bCluster = KyaG3D::ReadClusterTreeStrips(Data, Cluster) && !Cluster.IsEmpty();
            MeshHierarchy.Add(Placement.MeshIndex, bHierarchy ? Decoded.Num() : 0);
            MeshCluster.Add(Placement.MeshIndex, bCluster ? Cluster.Num() : 0);
            if (bHierarchy)
            {
                ++HierarchyOk;
                HierarchyStrips += Decoded.Num();
                for (const FKyaG3DDecodedStrip& Strip : Decoded)
                {
                    FMatrix Matrix;
                    if (KyaG3D::ReadHierarchyWorldMatrix(Data, Strip.HierarchyOffset, Matrix)) ++MatrixOk;
                    else ++MatrixFail;
                    if (Strip.MaterialHash != 0) ++TexturedStrips;
                    if (!Strip.Normals.IsEmpty())
                    {
                        ++NormalStrips;
                        if (Strip.Normals.Num() != Strip.Vertices.Num()) ++NormalMismatch;
                    }
                }
            }
            if (bCluster) { ++ClusterOk; ClusterStrips += Cluster.Num(); }
        }
        AddInfo(FString::Printf(TEXT("Actor mesh decode: models=%d meshes=%d hierarchy_ok=%d hierarchy_strips=%d cluster_ok=%d cluster_strips=%d matrix_ok=%d matrix_fail=%d hashed=%d normals=%d normal_mismatch=%d"),
            Models, MeshHierarchy.Num(), HierarchyOk, HierarchyStrips, ClusterOk, ClusterStrips, MatrixOk, MatrixFail, TexturedStrips, NormalStrips, NormalMismatch));
        // Actor models live in the type-4 mesh entries and decode through HALL hierarchies,
        // not cluster trees; every referenced mesh must decode with a world matrix.
        TestTrue(TEXT("Actor models reference mesh entries"), Models > 0);
        TestEqual(TEXT("Every actor model mesh decodes"), HierarchyOk, MeshHierarchy.Num());
        TestEqual(TEXT("Every actor model strip has a world matrix"), MatrixFail, 0);
        TestEqual(TEXT("Every actor model strip has a material hash"), TexturedStrips, MatrixOk);
        TestEqual(TEXT("Actor models are not cluster trees"), ClusterOk, 0);
        TestTrue(TEXT("Actor model strips decode stored normals"), NormalStrips > 0);
        TestEqual(TEXT("Stored actor normals match expanded vertices"), NormalMismatch, 0);
        {
            // Classify where actor-model material hashes resolve: own texture bank,
            // the alternate/default bank, the scene bank, or nowhere.
            const FKyaBankEntry* AlternateEntry = nullptr;
            const int32 AlternateIndex = Bank->SceneEnvironment.AlternateTextureIndex;
            int32 Ordinal = 0;
            for (const FKyaBankEntry& Candidate : Bank->Entries)
                if (Candidate.Type == 5 && Candidate.Subtype == 1 && Ordinal++ == AlternateIndex) { AlternateEntry = &Candidate; break; }
            TMap<uint64, int32> AlternateMaterials, SceneMaterials;
            auto ValidEntry = [&Input](const FKyaBankEntry* Candidate) {
                return Candidate && Candidate->Offset >= 0 && Candidate->Size > 0 && Candidate->Offset <= Input.Num() &&
                    Candidate->Size <= Input.Num() - Candidate->Offset;
            };
            if (ValidEntry(AlternateEntry))
                KyaG2D::ReadMaterials(MakeArrayView(Input.GetData() + AlternateEntry->Offset, int32(AlternateEntry->Size)), AlternateMaterials);
            if (Bank->Entries.IsValidIndex(Bank->SceneTextureEntry) && ValidEntry(&Bank->Entries[Bank->SceneTextureEntry]))
            {
                const FKyaBankEntry& SceneEntry = Bank->Entries[Bank->SceneTextureEntry];
                KyaG2D::ReadMaterials(MakeArrayView(Input.GetData() + SceneEntry.Offset, int32(SceneEntry.Size)), SceneMaterials);
            }
            // Union of every texture bank: hash -> owning entry, to test whether the
            // unresolved actor-model strips can be resolved by hash alone.
            TMap<uint64, int32> AllMaterials;
            TMap<int32, TMap<uint64, int32>> UnionBankMaterials;
            TMap<int32, TConstArrayView<uint8>> UnionBankData;
            for (int32 Index = 0; Index < Bank->Entries.Num(); ++Index)
            {
                const FKyaBankEntry& Candidate = Bank->Entries[Index];
                if (Candidate.Type != 5 || Candidate.Subtype != 1 || !ValidEntry(&Candidate)) continue;
                TMap<uint64, int32> Materials;
                const auto CandidateData = MakeArrayView(Input.GetData() + Candidate.Offset, int32(Candidate.Size));
                KyaG2D::ReadMaterials(CandidateData, Materials);
                for (const auto& Pair : Materials) AllMaterials.FindOrAdd(Pair.Key, Index);
                UnionBankData.Add(Index, CandidateData);
                UnionBankMaterials.Add(Index, MoveTemp(Materials));
            }
            int32 UnionResolved = 0, UnionMissing = 0;
            int32 Own = 0, Alternate = 0, Scene = 0, None = 0;
            TArray<int32> TextureEntries;
            for (int32 Index = 0; Index < Bank->Entries.Num(); ++Index)
                if (Bank->Entries[Index].Type == 5 && Bank->Entries[Index].Subtype == 1) TextureEntries.Add(Index);
            TSet<int32> Classified;
            for (const FKyaActorPlacement& Placement : Bank->SceneEnvironment.Actors)
            {
                if (Placement.MeshIndex < 0 || Placement.MeshIndex >= MeshEntries.Num()) continue;
                if (Placement.TextureIndex < 0)
                {
                    const FKyaBankEntry& Entry = Bank->Entries[MeshEntries[Placement.MeshIndex]];
                    if (Entry.Offset < 0 || Entry.Size <= 0 || Entry.Offset > Input.Num() || Entry.Size > Input.Num() - Entry.Offset) continue;
                    TArray<FKyaG3DDecodedStrip> Decoded;
                    if (!KyaG3D::DecodeHierarchyStrips(MakeArrayView(Input.GetData() + Entry.Offset, int32(Entry.Size)), Decoded)) continue;
                    for (const FKyaG3DDecodedStrip& Strip : Decoded)
                    {
                        if (Strip.MaterialHash == 0) continue;
                        ++None;
                        if (AllMaterials.Contains(Strip.MaterialHash)) ++UnionResolved;
                        else ++UnionMissing;
                    }
                    continue;
                }
                if (Placement.TextureIndex >= TextureEntries.Num()) { ++None; continue; }
                if (Classified.Contains(Placement.TextureIndex)) continue;
                Classified.Add(Placement.TextureIndex);
                const FKyaBankEntry& Entry = Bank->Entries[MeshEntries[Placement.MeshIndex]];
                if (Entry.Offset < 0 || Entry.Size <= 0 || Entry.Offset > Input.Num() || Entry.Size > Input.Num() - Entry.Offset) continue;
                const auto Data = MakeArrayView(Input.GetData() + Entry.Offset, int32(Entry.Size));
                const FKyaBankEntry& TextureEntry = Bank->Entries[TextureEntries[Placement.TextureIndex]];
                if (!ValidEntry(&TextureEntry)) continue;
                TMap<uint64, int32> OwnMaterials;
                KyaG2D::ReadMaterials(MakeArrayView(Input.GetData() + TextureEntry.Offset, int32(TextureEntry.Size)), OwnMaterials);
                TArray<FKyaG3DDecodedStrip> Decoded;
                if (!KyaG3D::DecodeHierarchyStrips(Data, Decoded)) continue;
                for (const FKyaG3DDecodedStrip& Strip : Decoded)
                {
                    if (Strip.MaterialHash == 0) continue;
                    if (OwnMaterials.Contains(Strip.MaterialHash)) ++Own;
                    else if (AlternateMaterials.Contains(Strip.MaterialHash)) ++Alternate;
                    else if (SceneMaterials.Contains(Strip.MaterialHash)) ++Scene;
                    else
                    {
                        ++None;
                        if (AllMaterials.Contains(Strip.MaterialHash)) ++UnionResolved;
                        else ++UnionMissing;
                    }
                }
            }
            AddInfo(FString::Printf(TEXT("Actor materials: own=%d alternate=%d scene=%d none=%d alternate_index=%d"),
                Own, Alternate, Scene, None, AlternateIndex));
            AddInfo(FString::Printf(TEXT("Actor materials union: resolvable=%d missing=%d banks=%d"),
                UnionResolved, UnionMissing, AllMaterials.Num()));
            // The grunt's body material exists in both its own bank and the shared
            // actor-material bank. Native links the shared bank afterwards, so the
            // shared 128x128 replacement must win; its eye material exists only in
            // the model bank and stays untouched.
            {
                const FKyaActorPlacement* Grunt = Bank->SceneEnvironment.Actors.FindByPredicate(
                    [](const FKyaActorPlacement& Candidate) { return Candidate.Name == TEXT("GRUNTNATIV"); });
                if (TestNotNull(TEXT("Grunt actor placement"), Grunt))
                {
                    TestTrue(TEXT("Grunt names its own texture bank"), Grunt->TextureIndex >= 0 && Grunt->TextureIndex < TextureEntries.Num());
                    if (Grunt->TextureIndex >= 0 && Grunt->TextureIndex < TextureEntries.Num())
                    {
                        const FKyaBankEntry& OwnEntry = Bank->Entries[TextureEntries[Grunt->TextureIndex]];
                        TMap<uint64, int32> OwnMaterials;
                        KyaG2D::ReadMaterials(MakeArrayView(Input.GetData() + OwnEntry.Offset, int32(OwnEntry.Size)), OwnMaterials);
                        const uint64 BodyHash = 0x505f535649544180ULL;
                        const uint64 EyeHash = 0x13c7ee4133403361ULL;
                        TestTrue(TEXT("Grunt body material is in its own bank"), OwnMaterials.Contains(BodyHash));
                        TestTrue(TEXT("Grunt body material has a shared replacement"), AlternateMaterials.Contains(BodyHash));
                        TestTrue(TEXT("Grunt eye material is in its own bank"), OwnMaterials.Contains(EyeHash));
                        TestFalse(TEXT("Grunt eye material is not overridden by the shared bank"), AlternateMaterials.Contains(EyeHash));
                    }
                }
            }
            // Diagnose the ROLLINGROCK actor that renders dark: its decoded vertex colours,
            // material hashes and the actual texture bytes behind each hash.
            {
                const FKyaActorPlacement* Rock = Bank->SceneEnvironment.Actors.FindByPredicate(
                    [](const FKyaActorPlacement& Candidate) { return Candidate.Name == TEXT("ROLLINGROCK"); });
                if (Rock)
                {
                    AddInfo(FString::Printf(TEXT("Rock: mesh=%d tex=%d scale=(%.3f,%.3f,%.3f)"), Rock->MeshIndex, Rock->TextureIndex, Rock->Scale.X, Rock->Scale.Y, Rock->Scale.Z));
                    if (Rock->MeshIndex >= 0 && Rock->MeshIndex < MeshEntries.Num())
                    {
                        const FKyaBankEntry& MeshEntry = Bank->Entries[MeshEntries[Rock->MeshIndex]];
                        const auto MeshView = MakeArrayView(Input.GetData() + MeshEntry.Offset, int32(MeshEntry.Size));
                        TArray<FKyaG3DDecodedStrip> RockStrips;
                        const bool bDecoded = KyaG3D::DecodeHierarchyStrips(MeshView, RockStrips);
                        AddInfo(FString::Printf(TEXT("Rock strips: decoded=%d count=%d"), bDecoded, RockStrips.Num()));
                        {
                            TArray<FKyaG3DStripReference> RockRefs;
                            if (KyaG3D::ReadHierarchyStrips(MeshView, RockRefs) && !RockRefs.IsEmpty())
                            {
                                uint32 VertexCount = 0, Primitive = 0;
                                if (KyaG3D::ReadFirstStripGifTag(MeshView, RockRefs[0].StripOffset, VertexCount, Primitive))
                                    AddInfo(FString::Printf(TEXT("Rock primitive: prim=%x abe=%d verts=%d"),
                                        Primitive & ~1u, int32(Primitive & 1), VertexCount));
                            }
                        }
                        {
                            TArray<FKyaG3DStripReference> RockRefs;
                            if (KyaG3D::ReadHierarchyStrips(MeshView, RockRefs) && !RockRefs.IsEmpty())
                            {
                                auto Word = [&MeshView](int32 Offset) -> uint32 {
                                    const uint8* P = MeshView.GetData() + Offset;
                                    return uint32(P[0]) | (uint32(P[1]) << 8) | (uint32(P[2]) << 16) | (uint32(P[3]) << 24);
                                };
                                const int32 S = RockRefs[0].StripOffset;
                                TArray<uint32> Sections;
                                KyaG3D::ReadStripSectionCounts(MeshView, S, Sections);
                                AddInfo(FString::Printf(TEXT("Rock raw: offset=%d flags=%08x uvbase=%d colorbase=%d sections=%d first=%d"),
                                    S, Word(S), Word(S + 32), Word(S + 36), Sections.Num(), Sections.IsEmpty() ? -1 : int32(Sections[0])));
                                const int32 ColorBase = int32(Word(S + 36));
                                FString Bytes;
                                for (int32 Offset = 0; Offset < 32 && ColorBase + 16 + Offset < MeshView.Num(); ++Offset)
                                    Bytes += FString::Printf(TEXT("%02x"), MeshView[ColorBase + 16 + Offset]);
                                AddInfo(FString::Printf(TEXT("Rock raw colour bytes: %s"), *Bytes));
                                for (int32 Index = 0; Index < 4; ++Index)
                                    AddInfo(FString::Printf(TEXT("Rock raw colour %d: %d,%d,%d,%d"), Index, MeshView[ColorBase + 16 + Index * 4],
                                        MeshView[ColorBase + 17 + Index * 4], MeshView[ColorBase + 18 + Index * 4], MeshView[ColorBase + 19 + Index * 4]));
                            }
                        }
                        for (int32 StripIndex = 0; StripIndex < RockStrips.Num() && StripIndex < 3; ++StripIndex)
                        {
                            const FKyaG3DDecodedStrip& Strip = RockStrips[StripIndex];
                            int32 MinR = 255, MinG = 255, MinB = 255, MaxR = 0, MaxG = 0, MaxB = 0;
                            double SumR = 0, SumG = 0, SumB = 0;
                            for (const FKyaG3DVertexAttributes& Attribute : Strip.Attributes)
                            {
                                MinR = FMath::Min(MinR, int32(Attribute.Color.R)); MaxR = FMath::Max(MaxR, int32(Attribute.Color.R));
                                MinG = FMath::Min(MinG, int32(Attribute.Color.G)); MaxG = FMath::Max(MaxG, int32(Attribute.Color.G));
                                MinB = FMath::Min(MinB, int32(Attribute.Color.B)); MaxB = FMath::Max(MaxB, int32(Attribute.Color.B));
                                SumR += Attribute.Color.R; SumG += Attribute.Color.G; SumB += Attribute.Color.B;
                            }
                            const int32 Count = FMath::Max(1, Strip.Attributes.Num());
                            AddInfo(FString::Printf(TEXT("Rock strip %d: verts=%d hash=%08x%08x colour mean=(%.0f,%.0f,%.0f) min=(%d,%d,%d) max=(%d,%d,%d)"),
                                StripIndex, Strip.Vertices.Num(), uint32(Strip.MaterialHash >> 32), uint32(Strip.MaterialHash),
                                SumR / Count, SumG / Count, SumB / Count, MinR, MinG, MinB, MaxR, MaxG, MaxB));
                            int32 Width = 0, Height = 0;
                            TArray<FColor> Pixels;
                            for (const auto& BankPair : UnionBankMaterials)
                            {
                                const int32* MaterialOffset = BankPair.Value.Find(Strip.MaterialHash);
                                if (!MaterialOffset) continue;
                                if (!KyaG2D::DecodeMaterialTexture(UnionBankData[BankPair.Key], *MaterialOffset, Width, Height, Pixels))
                                {
                                    AddInfo(FString::Printf(TEXT("Rock strip %d: texture decode failed bank=%d"), StripIndex, BankPair.Key));
                                    break;
                                }
                                double R = 0, G = 0, B = 0;
                                for (const FColor& Pixel : Pixels) { R += Pixel.R; G += Pixel.G; B += Pixel.B; }
                                const int32 PixelCount = FMath::Max(1, Pixels.Num());
                                AddInfo(FString::Printf(TEXT("Rock strip %d: texture bank=%d %dx%d mean=(%.0f,%.0f,%.0f)"),
                                    StripIndex, BankPair.Key, Width, Height, R / PixelCount, G / PixelCount, B / PixelCount));
                                {
                                    AddInfo(FString::Printf(TEXT("Rock texture size: %dx%d pixels=%d"), Width, Height, Pixels.Num()));
                                {
                                    const auto& BankView = UnionBankData[BankPair.Key];
                                    const int32 Layers = int32(BankView[*MaterialOffset + 16]);
                                    AddInfo(FString::Printf(TEXT("Rock material: layers=%d"), Layers));
                                    for (int32 Layer = 0; Layer < Layers && Layer < 4; ++Layer)
                                    {
                                        uint64 Tex0 = 0, Alpha1 = 0, Test1 = 0, Alpha2 = 0;
                                        const bool bTex = KyaG2D::ReadMaterialRegister(BankView, *MaterialOffset, Layer, 0x06, Tex0);
                                        const bool bA1 = KyaG2D::ReadMaterialRegister(BankView, *MaterialOffset, Layer, 0x00, Alpha1);
                                        const bool bT1 = KyaG2D::ReadMaterialRegister(BankView, *MaterialOffset, Layer, 0x47, Test1);
                                        const bool bA2 = KyaG2D::ReadMaterialRegister(BankView, *MaterialOffset, Layer, 0x01, Alpha2);
                                        AddInfo(FString::Printf(TEXT("Rock layer %d: tex0=%d fmt=%x a1=%d(%08x%08x) test=%d(%08x%08x) a2=%d"),
                                            Layer, bTex, uint32((Tex0 >> 20) & 63), bA1, uint32(Alpha1 >> 32), uint32(Alpha1),
                                            bT1, uint32(Test1 >> 32), uint32(Test1), bA2));
                                    }
                                }
                                    const int32 PpmHeight = Width > 0 ? Pixels.Num() / Width : 0;
                                    if (PpmHeight > 0)
                                    {
                                        const FString Header = FString::Printf(TEXT("P6\n%d %d\n255\n"), Width, PpmHeight);
                                        FTCHARToUTF8 HeaderUtf8(*Header);
                                        TArray<uint8> Bytes;
                                        Bytes.Append(reinterpret_cast<const uint8*>(HeaderUtf8.Get()), HeaderUtf8.Length());
                                        for (int32 PixelIndex = 0; PixelIndex < Width * PpmHeight; ++PixelIndex)
                                        {
                                            Bytes.Add(Pixels[PixelIndex].R);
                                            Bytes.Add(Pixels[PixelIndex].G);
                                            Bytes.Add(Pixels[PixelIndex].B);
                                        }
                                        FFileHelper::SaveArrayToFile(Bytes, TEXT("G:/repos/Kya/out/rock_texture.ppm"));
                                    }
                                }
                                break;
                            }
                        }
                    }
                }
            }
            // Actors with TextureIndex == -1 name no texture bank; every material these
            // actors reference must still resolve by hash across the bank's texture banks.
            TestTrue(TEXT("Actors without a texture index reference materials"), UnionResolved > 0);
            TestEqual(TEXT("Every material of actors without a texture index resolves"), UnionMissing, 0);
            TestEqual(TEXT("Every material of actors with a texture bank resolves"), None - UnionResolved, 0);
        }
    }
    PreviewWorld->DestroyWorld(false);
    TestTrue(TEXT("Bank has a light archive"), Bank->bHasLights);
    TestEqual(TEXT("Light count"), Bank->Lights.Num(), 21);
    if (Bank->Lights.Num() == 21)
    {
        const FKyaLight& Sun = Bank->Lights[0];
        TestEqual(TEXT("Sun light type"), Sun.LightType, 1);
        TestEqual(TEXT("Sun shape type"), Sun.ShapeType, 1);
        TestTrue(TEXT("Sun colour is mid grey"), FMath::IsNearlyEqual(Sun.Color.R, 154.0f / 255.0f, 0.01f));
        TestTrue(TEXT("Sun direction points down and forward"),
            FMath::IsNearlyEqual(Sun.Direction.X, 0.788f, 0.02f) &&
            FMath::IsNearlyEqual(Sun.Direction.Y, 0.594f, 0.02f) &&
            FMath::IsNearlyEqual(Sun.Direction.Z, -0.160f, 0.02f));
        const int32 TorchCount = Bank->Lights.FilterByPredicate(
            [](const FKyaLight& Light) { return Light.LightType == 2; }).Num();
        TestTrue(TEXT("Bank has torch lights"), TorchCount > 0);
        AddInfo(FString::Printf(TEXT("Lights: total=%d torches=%d suns=%d"), Bank->Lights.Num(), TorchCount,
            Bank->Lights.Num() - TorchCount));
    }
    TestEqual(TEXT("Entry count"), Bank->EntryCount, 1531);
    TestEqual(TEXT("Scene mesh entry"), Bank->SceneMeshEntry, 278);
    TestEqual(TEXT("Scene texture entry"), Bank->SceneTextureEntry, 108);
    TestEqual(TEXT("Background mesh entry"), Bank->BackgroundMeshEntry, 240);
    TestEqual(TEXT("Background texture entry"), Bank->BackgroundTextureEntry, 170);
    if (Bank->Entries.IsValidIndex(Bank->BackgroundTextureEntry))
    {
        const FKyaBankEntry& Texture = Bank->Entries[Bank->BackgroundTextureEntry];
        TMap<uint64, int32> Materials;
        if (TestTrue(TEXT("Read background G2D materials"), KyaG2D::ReadMaterials(
            MakeArrayView(Input.GetData() + Texture.Offset, int32(Texture.Size)), Materials)))
        {
            const int32* Material = Materials.Find(0x197097db0edf22dbULL);
            if (TestNotNull(TEXT("Background contains first mesh material"), Material))
            {
                uint64 Tex0 = 0;
                TestTrue(TEXT("Read material TEX0"), KyaG2D::ReadMaterialTex0(
                    MakeArrayView(Input.GetData() + Texture.Offset, int32(Texture.Size)), *Material, 0, Tex0));
                TestEqual(TEXT("Material TEX0 value"), Tex0, uint64(0x60007005dd308000ULL));
                int32 DecodedWidth = 0, DecodedHeight = 0;
                TArray<FColor> DecodedPixels;
                if (TestTrue(TEXT("Decode indexed material texture"), KyaG2D::DecodeMaterialTexture(
                    MakeArrayView(Input.GetData() + Texture.Offset, int32(Texture.Size)), *Material,
                    DecodedWidth, DecodedHeight, DecodedPixels)))
                {
                    TestEqual(TEXT("Decoded material width"), DecodedWidth, 128);
                    TestEqual(TEXT("Decoded material height"), DecodedHeight, 128);
                    TestEqual(TEXT("Decoded material pixels"), DecodedPixels.Num(), 128 * 128);
                }
                KyaG2D::FBitmapReference Bitmap;
                if (TestTrue(TEXT("Resolve material bitmap"), KyaG2D::ReadMaterialBitmap(
                    MakeArrayView(Input.GetData() + Texture.Offset, int32(Texture.Size)), *Material, Bitmap)))
                {
                    TestEqual(TEXT("Bitmap width"), Bitmap.Width, uint16(128));
                    TestEqual(TEXT("Bitmap height"), Bitmap.Height, uint16(128));
                    TestEqual(TEXT("Bitmap format"), Bitmap.PixelFormat, uint16(8));
                    TestEqual(TEXT("Bitmap mip count"), Bitmap.MipCount, uint16(3));
                    TestEqual(TEXT("Bitmap chunk size"), Bitmap.BitmapChunkSize, 0x5420);
                    TestTrue(TEXT("Bitmap has palette"), Bitmap.PaletteOffset != INDEX_NONE);
                    TArray<TPair<int32, int32>> UploadLists;
                    if (TestTrue(TEXT("Resolve palette upload lists"), KyaG2D::ReadUploadLists(
                        MakeArrayView(Input.GetData() + Texture.Offset, int32(Texture.Size)), Bitmap.PaletteOffset, UploadLists)))
                    {
                        TestEqual(TEXT("First upload packet count"), UploadLists[0].Value, 37);
                        TArray<KyaG2D::FUploadTransfer> Transfers;
                        if (TestTrue(TEXT("Read upload transfers"), KyaG2D::ReadUploadTransfers(
                            MakeArrayView(Input.GetData() + Texture.Offset, int32(Texture.Size)), UploadLists[0].Key, UploadLists[0].Value, Transfers)))
                        {
                            TestEqual(TEXT("Upload transfer count"), Transfers.Num(), 4);
                            if (Transfers.Num() == 4)
                            {
                                TestEqual(TEXT("First transfer width"), Transfers[0].Width(), uint32(64));
                                TestEqual(TEXT("First transfer height"), Transfers[0].Height(), uint32(64));
                                TestEqual(TEXT("First transfer format"), Transfers[0].DestinationFormat(), uint32(0));
                                TestEqual(TEXT("First transfer bytes"), Transfers[0].DataSize, 16384);
                                TArray<uint8> GsMemory;
                                GsMemory.SetNumZeroed(4 * 1024 * 1024);
                                TestTrue(TEXT("Apply first GS transfer"), KyaG2D::WriteGs32(
                                    MakeArrayView(Input.GetData() + Texture.Offset, int32(Texture.Size)), Transfers[0], GsMemory));
                                TestEqual(TEXT("First uploaded byte"), GsMemory[Transfers[0].DestinationBase() * 256],
                                    Input[int32(Texture.Offset) + Transfers[0].DataOffset]);
                                TArray<uint8> Indices;
                                TestTrue(TEXT("Read uploaded PSMT8 texture"), KyaG2D::ReadGs8(GsMemory, Tex0, Indices));
                                TestEqual(TEXT("Decoded texture pixel count"), Indices.Num(), 128 * 128);
                            }
                        }
                    }
                    TArray<FColor> Palette;
                    if (TestTrue(TEXT("Read PA32 palette"), KyaG2D::ReadPalette32(
                        MakeArrayView(Input.GetData() + Texture.Offset, int32(Texture.Size)), Bitmap.PaletteOffset, Palette)))
                    {
                        TestEqual(TEXT("Palette entries"), Palette.Num(), 256);
                        if (!Palette.IsEmpty()) TestEqual(TEXT("First palette entry"), Palette[0], FColor(0xb2, 0x90, 0x51, 0x80));
                    }
                }
            }
        }
    }
    {
        // Independent GS column-table examples, including row and block boundaries.
        TArray<uint8> Memory;
        Memory.SetNumZeroed(4 * 1024 * 1024);
        Memory[0] = 11;
        Memory[33] = 22; // (0, 2)
        Memory[96] = 33; // (0, 4)
        Memory[256] = 44; // (16, 0), next block
        // CSM1 index 16 maps to word 64 in the palette block.
        Memory[8192 + 64 * 4] = 73;
        TArray<uint8> Pixels;
        const uint64 Tex0 = (uint64(2) << 14) | (uint64(0x13) << 20) | (uint64(7) << 26) | (uint64(6) << 30);
        if (TestTrue(TEXT("Read synthetic PSMT8 texture"), KyaG2D::ReadGs8(Memory, Tex0, Pixels)))
        {
            TestEqual(TEXT("GS pixel origin"), Pixels[0], uint8(11));
            TestEqual(TEXT("GS pixel row 2"), Pixels[2 * 128], uint8(22));
            TestEqual(TEXT("GS pixel row 4"), Pixels[4 * 128], uint8(33));
            TestEqual(TEXT("GS pixel next block"), Pixels[16], uint8(44));
        }
        TArray<FColor> Clut;
        if (TestTrue(TEXT("Read synthetic CSM1 palette"), KyaG2D::ReadGsClut32(Memory, Tex0 | (uint64(32) << 37), Clut)))
            TestEqual(TEXT("CSM1 index 16"), Clut[16].R, uint8(73));
        TArray<uint8> FourBitMemory;
        FourBitMemory.SetNumZeroed(4 * 1024 * 1024);
        FourBitMemory[0] = 0x03;
        FourBitMemory[4] = 0x07; // (1, 0), nibble 8
        FourBitMemory[32] = 0xa0; // (0, 2), nibble 65
        FourBitMemory[256] = 0x09; // (0, 16), next block
        FourBitMemory[8192 + 2 * 4] = 61; // palette index 8 maps to word 2
        const uint64 FourBitTex0 = (uint64(2) << 14) | (uint64(0x14) << 20) |
            (uint64(7) << 26) | (uint64(7) << 30) | (uint64(32) << 37);
        TArray<uint8> FourBitPixels;
        if (TestTrue(TEXT("Read synthetic PSMT4 texture"), KyaG2D::ReadGs4(FourBitMemory, FourBitTex0, FourBitPixels)))
        {
            TestEqual(TEXT("PSMT4 origin low nibble"), FourBitPixels[0], uint8(3));
            TestEqual(TEXT("PSMT4 next pixel"), FourBitPixels[1], uint8(7));
            TestEqual(TEXT("PSMT4 row two high nibble"), FourBitPixels[2 * 128], uint8(10));
            TestEqual(TEXT("PSMT4 next block"), FourBitPixels[16 * 128], uint8(9));
        }
        TestTrue(TEXT("Read PSMT4 texture with zero TEX0 buffer width"), KyaG2D::ReadGs4(
            FourBitMemory, FourBitTex0 & ~(uint64(63) << 14), FourBitPixels));
        TestTrue(TEXT("Read PSMT8 texture with zero TEX0 buffer width"), KyaG2D::ReadGs8(
            Memory, Tex0 & ~(uint64(63) << 14), Pixels));
        TArray<FColor> FourBitClut;
        if (TestTrue(TEXT("Read synthetic PSMT4 CSM1 palette"), KyaG2D::ReadGsClut4(FourBitMemory, FourBitTex0, FourBitClut)))
            TestEqual(TEXT("PSMT4 palette index 8"), FourBitClut[8].R, uint8(61));
        KyaG2D::FUploadTransfer Upload;
        Upload.DataOffset = 0; Upload.DataSize = 2;
        Upload.BitBlt = (uint64(1) << 48) | (uint64(0x14) << 56);
        Upload.Region = uint64(4) | (uint64(1) << 32);
        const uint8 Packed[] = { 0x21, 0x43 };
        TArray<uint8> UploadedMemory;
        UploadedMemory.SetNumZeroed(4 * 1024 * 1024);
        if (TestTrue(TEXT("Upload packed PSMT4 pixels"), KyaG2D::WriteGsIndexed(MakeArrayView(Packed), Upload, UploadedMemory)))
        {
            TArray<uint8> UploadedPixels;
            const uint64 NarrowTex0 = (uint64(1) << 14) | (uint64(0x14) << 20) | (uint64(2) << 26);
            if (TestTrue(TEXT("Read narrow PSMT4 upload"), KyaG2D::ReadGs4(UploadedMemory, NarrowTex0, UploadedPixels)))
                for (int32 Index = 0; Index < 4; ++Index)
                    TestEqual(TEXT("Uploaded nibble value"), UploadedPixels[Index], uint8(Index + 1));
        }
        TArray<uint8> DirectMemory;
        DirectMemory.SetNumZeroed(4 * 1024 * 1024);
        DirectMemory[0] = 17;
        DirectMemory[4] = 42; // (1, 0)
        DirectMemory[256] = 91; // (8, 0), next block
        DirectMemory[8192] = 123; // (0, 32), next page
        TArray<FColor> DirectPixels;
        const uint64 DirectTex0 = (uint64(1) << 14) | (uint64(6) << 26) | (uint64(6) << 30);
        if (TestTrue(TEXT("Read synthetic PSMCT32 texture"), KyaG2D::ReadGs32(DirectMemory, DirectTex0, DirectPixels)))
        {
            TestEqual(TEXT("Direct texture origin"), DirectPixels[0].R, uint8(17));
            TestEqual(TEXT("Direct texture adjacent pixel"), DirectPixels[1].R, uint8(42));
            TestEqual(TEXT("Direct texture next block"), DirectPixels[8].R, uint8(91));
            TestEqual(TEXT("Direct texture next page"), DirectPixels[32 * 64].R, uint8(123));
        }
    }
    for (const int32 MeshIndex : { Bank->SceneMeshEntry, Bank->BackgroundMeshEntry })
    {
        if (!TestTrue(TEXT("Mesh entry index is valid"), Bank->Entries.IsValidIndex(MeshIndex))) return false;
        const FKyaBankEntry& Mesh = Bank->Entries[MeshIndex];
        TArray<FKyaG3DChunk> Chunks;
        if (!TestTrue(TEXT("Read G3D chunks"), KyaG3D::ReadChunks(
            MakeArrayView(Input.GetData() + Mesh.Offset, int32(Mesh.Size)), Chunks))) return false;
        TestTrue(TEXT("Contains PSX2 geometry"), Chunks.ContainsByPredicate(
            [](const FKyaG3DChunk& Chunk) { return Chunk.Tag == 0x32585350; }));
        if (MeshIndex == Bank->SceneMeshEntry)
        {
            TArray<int32> ClusterStrips;
            if (TestTrue(TEXT("Read main scene cluster strips"), KyaG3D::ReadClusterStrips(
                MakeArrayView(Input.GetData() + Mesh.Offset, int32(Mesh.Size)), ClusterStrips)))
                TestEqual(TEXT("Level one main scene cluster strip count"), ClusterStrips.Num(), 0);
        }
        TMap<int32, int32> Relocations;
        TArray<int32> InvalidFields;
        TestTrue(TEXT("Read G3D relocations"), KyaG3D::ReadRelocations(
            MakeArrayView(Input.GetData() + Mesh.Offset, int32(Mesh.Size)), Relocations, InvalidFields));
        TestTrue(TEXT("G3D has relocations"), !Relocations.IsEmpty());
        TArray<int32> Hierarchies;
        TestTrue(TEXT("Read G3D hierarchies"), KyaG3D::ReadHierarchyOffsets(
            MakeArrayView(Input.GetData() + Mesh.Offset, int32(Mesh.Size)), Hierarchies));
        TestEqual(TEXT("Hierarchy count"), Hierarchies.Num(), MeshIndex == 240 ? 97 : 0);
        for (int32 Hierarchy : Hierarchies)
        {
            FMatrix World;
            if (!KyaG3D::ReadHierarchyWorldMatrix(MakeArrayView(Input.GetData() + Mesh.Offset, int32(Mesh.Size)), Hierarchy, World))
                AddError(FString::Printf(TEXT("Cannot resolve hierarchy transform at 0x%x"), Hierarchy));
        }
        TArray<FKyaG3DStripReference> Strips;
        TestTrue(TEXT("Read hierarchy strips"), KyaG3D::ReadHierarchyStrips(
            MakeArrayView(Input.GetData() + Mesh.Offset, int32(Mesh.Size)), Strips));
        TestEqual(TEXT("First LOD strip count"), Strips.Num(), MeshIndex == 240 ? 119 : 0);
        for (const FKyaG3DStripReference& Strip : Strips)
        {
            TArray<uint32> Counts;
            if (!KyaG3D::ReadStripSectionCounts(MakeArrayView(Input.GetData() + Mesh.Offset, int32(Mesh.Size)), Strip.StripOffset, Counts))
                AddError(FString::Printf(TEXT("Cannot read strip sections at G3D offset 0x%x"), Strip.StripOffset));
        }
        TArray<FKyaG3DDecodedStrip> DecodedStrips;
        if (TestTrue(TEXT("Decode hierarchy strips"), KyaG3D::DecodeHierarchyStrips(
            MakeArrayView(Input.GetData() + Mesh.Offset, int32(Mesh.Size)), DecodedStrips)))
        {
            TestEqual(TEXT("Decoded strip count"), DecodedStrips.Num(), Strips.Num());
            if (MeshIndex == 240 && !DecodedStrips.IsEmpty())
            {
                TestEqual(TEXT("First strip material index"), DecodedStrips[0].MaterialIndex, 0);
                TestEqual(TEXT("First strip material hash"), DecodedStrips[0].MaterialHash, uint64(0x197097db0edf22dbULL));
            }
        }
        if (MeshIndex == 240 && !Strips.IsEmpty())
        {
            uint32 VertexCount = 0, Primitive = 0;
            TestTrue(TEXT("Read first GIF tag"), KyaG3D::ReadFirstStripGifTag(
                MakeArrayView(Input.GetData() + Mesh.Offset, int32(Mesh.Size)), Strips[0].StripOffset,
                VertexCount, Primitive));
            TestEqual(TEXT("First strip vertex count"), VertexCount, uint32(96));
            TArray<uint32> SectionCounts;
            if (TestTrue(TEXT("Read strip section counts"), KyaG3D::ReadStripSectionCounts(
                MakeArrayView(Input.GetData() + Mesh.Offset, int32(Mesh.Size)), Strips[0].StripOffset, SectionCounts)))
            {
                TestEqual(TEXT("Section count"), SectionCounts.Num(), 3);
                if (SectionCounts.Num() == 3)
                    TestEqual(TEXT("Final section vertices"), SectionCounts[2], uint32(20));
            }
            TArray<FKyaG3DVertex> Vertices;
            if (TestTrue(TEXT("Read first strip vertices"), KyaG3D::ReadFirstStripVertices(
                MakeArrayView(Input.GetData() + Mesh.Offset, int32(Mesh.Size)), Strips[0].StripOffset, Vertices)))
            {
                TestEqual(TEXT("Decoded vertex count"), Vertices.Num(), 96);
                TestTrue(TEXT("First vertex X"), FMath::IsNearlyEqual(Vertices[0].Position.X, -0.65968186f));
                TestEqual(TEXT("First vertex flags"), Vertices[0].Flags, uint32(49152));
            }
            TArray<FKyaG3DVertex> AllVertices;
            TArray<uint32> AllSections;
            TArray<FKyaG3DVertexAttributes> Attributes;
            if (TestTrue(TEXT("Read strip attributes"), KyaG3D::ReadStripAttributes(
                MakeArrayView(Input.GetData() + Mesh.Offset, int32(Mesh.Size)), Strips[0].StripOffset, Attributes)))
                TestEqual(TEXT("Section-order attribute count"), Attributes.Num(), 212);
            if (TestTrue(TEXT("Read full strip vertices"), KyaG3D::ReadStripVertices(
                MakeArrayView(Input.GetData() + Mesh.Offset, int32(Mesh.Size)), Strips[0].StripOffset, AllVertices, AllSections)))
            {
                TestEqual(TEXT("Full strip unique vertices"), AllVertices.Num(), 208);
                TArray<uint32> Indices;
                TestTrue(TEXT("Build strip triangles"), KyaG3D::BuildStripTriangles(AllVertices, AllSections, Primitive, Indices));
                TestTrue(TEXT("Strip contains triangles"), !Indices.IsEmpty() && Indices.Num() % 3 == 0);
            }
        }
        TestEqual(TEXT("Out-of-range relocation count"), InvalidFields.Num(), MeshIndex == 240 ? 1 : 0);
        if (MeshIndex == 240 && InvalidFields.Num() == 1)
            TestEqual(TEXT("Out-of-range relocation field"), InvalidFields[0], 0x235cc);
        TestFalse(TEXT("Truncated mesh rejected"), KyaG3D::ReadChunks(
            MakeArrayView(Input.GetData() + Mesh.Offset, int32(Mesh.Size) - 1), Chunks));
    }
    if (!TestEqual(TEXT("Collision bank count"), Bank->SceneConfiguration.CollisionBankValues.Num(), 91)) return false;
    TestEqual(TEXT("First collision bank value"), Bank->SceneConfiguration.CollisionBankValues[0], 1);
    TestEqual(TEXT("Animation count"), Bank->SceneConfiguration.LevelAnimationCount, 1052);
    if (!TestEqual(TEXT("Actor class count"), Bank->SceneConfiguration.ActorClassAllocations.Num(), 36)) return false;
    TestEqual(TEXT("First actor class"), Bank->SceneConfiguration.ActorClassAllocations[0].ClassId, 5);
    TestEqual(TEXT("First actor allocation"), Bank->SceneConfiguration.ActorClassAllocations[0].Count, 1);
    TArray<uint8> Output;
    FMemoryWriter Writer(Output);
    UKyaLevelBankExporter* Exporter = NewObject<UKyaLevelBankExporter>();
    if (!TestTrue(TEXT("Export bank"), Exporter->ExportBinary(Bank, TEXT("bnk"), Writer, GWarn))) return false;
    TestTrue(TEXT("Unchanged bank is byte-identical"), Input == Output);
    Bank->SceneConfiguration.Field1C = 4.5f;
    Bank->SceneConfiguration.Field20 = 123456;
    Bank->SceneConfiguration.Field24 = -0.75f;
    Bank->SceneConfiguration.CommonTextureCount = 123;
    if (!TestEqual(TEXT("Scene header word count"), Bank->SceneConfiguration.HeaderWords.Num(), 2) ||
        !TestEqual(TEXT("Scene post-animation word count"), Bank->SceneConfiguration.PostAnimationWords.Num(), 4) ||
        !TestTrue(TEXT("Scene mesh values present"), !Bank->SceneConfiguration.CommonMeshValues.IsEmpty())) return false;
    Bank->SceneConfiguration.HeaderWords[1] = 0x12345678;
    Bank->SceneConfiguration.CommonMeshValues[0] = -27;
    Bank->SceneConfiguration.TextureWord = -19;
    Bank->SceneConfiguration.PostAnimationWords[2] = 0x87654321;
    Bank->SceneConfiguration.ParticleWord = 0xabcdef12;
    Bank->SceneEnvironment.ClipDistance = 456.0f;
    Bank->SceneEnvironment.FogColor = FColor(12, 34, 56, 78);
    Bank->SceneEnvironment.MipmapK = -12;
    Bank->SceneEnvironment.CameraFov = 1.25f;
    Bank->SceneEnvironment.CameraPosition = FVector(2.0, -3.0, 4.5);
    Bank->SceneEnvironment.CameraFlags = 0x1234;
    Bank->SceneEnvironment.CameraTargetOffset = FVector(1.0, 2.0, 3.0);
    Bank->SceneEnvironment.CameraConfigAngle = 55.0f;
    if (Bank->SceneEnvironment.bHasCameraSwitchData)
    {
        Bank->SceneEnvironment.CameraSwitchActor = 47;
        Bank->SceneEnvironment.CameraSwitchValueB = 9.25f;
    }
    if (!Bank->SceneEnvironment.CameraConditions.IsEmpty())
        Bank->SceneEnvironment.CameraConditions[0].Value = 8.25f;
    if (!Bank->SceneEnvironment.CameraOptionalConfigValues.IsEmpty())
        Bank->SceneEnvironment.CameraOptionalConfigValues[0] = 6.25f;
    if (!Bank->SceneEnvironment.CameraExtraValues.IsEmpty())
        Bank->SceneEnvironment.CameraExtraValues[0] = 7.25f;
    Bank->SceneEnvironment.CameraPitchAngles = FVector(10.0, 20.0, 30.0);
    Bank->SceneEnvironment.Field118 = 25.0f;
    Bank->SceneEnvironment.Field11C = 1;
    Bank->SceneEnvironment.Count120 = 3;
    Bank->SceneEnvironment.MapAngle = 42.0f;
    Bank->SceneEnvironment.MapActorRef = 123;
    Bank->SceneEnvironment.DefaultMaterialIndex = 7;
    if (!TestEqual(TEXT("Effect pool category count"), Bank->SceneEnvironment.EffectPoolCounts.Num(), 7)) return false;
    Bank->SceneEnvironment.EffectPoolCounts[3] = 37;
    if (!TestEqual(TEXT("Objective count"), Bank->SceneEnvironment.Objectives.Num(), 3)) return false;
    Bank->SceneEnvironment.Objectives[0].ScenarioVariable = 91;
    Bank->SceneEnvironment.Objectives[0].Fields[0] = 321;
    Bank->SceneEnvironment.Field10C = 20;
    Bank->SceneEnvironment.Field110 = 21;
    if (!TestEqual(TEXT("Waypoint count"), Bank->SceneEnvironment.Waypoints.Num(), 60)) return false;
    Bank->SceneEnvironment.Waypoints[0].Position = FVector(1.0, 2.0, 3.0);
    if (!TestEqual(TEXT("Path count"), Bank->SceneEnvironment.Paths.Num(), 231)) return false;
    if (!TestTrue(TEXT("First path has points"), !Bank->SceneEnvironment.Paths[0].Points.IsEmpty())) return false;
    Bank->SceneEnvironment.Paths[0].Points[0] = FVector4(1, 2, 3, 1);
    if (!TestEqual(TEXT("Actor count"), Bank->SceneEnvironment.Actors.Num(), 616)) return false;
    {
        int32 Platforms = 0, Prefixes = 0, Details = 0, Cameras = 0, SoundA = 0, SoundB = 0, Friction = 0, FrictionActors = 0;
        for (const FKyaActorPlacement& Item : Bank->SceneEnvironment.Actors)
        {
            if (Item.ClassId != 0x0d) continue;
            ++Platforms;
            Prefixes += Item.bHasPlatformPrefix ? 1 : 0;
            Details += Item.bHasPlatformDetails ? 1 : 0;
            Cameras += Item.PlatformCameras.Num(); SoundA += Item.PlatformSoundsA.Num(); SoundB += Item.PlatformSoundsB.Num();
            Friction += Item.PlatformNoFrictionZones.Num();
            for (const FKyaPlatformNoFrictionZone& Zone : Item.PlatformNoFrictionZones) FrictionActors += Zone.Actors.Num();
        }
        AddInfo(FString::Printf(TEXT("Platform inventory: actors=%d prefixes=%d details=%d camera-records=%d sound-a=%d sound-b=%d friction-zones=%d friction-actors=%d"),
            Platforms, Prefixes, Details, Cameras, SoundA, SoundB, Friction, FrictionActors));
        UE_LOG(LogTemp, Display, TEXT("Kya platform inventory: actors=%d prefixes=%d details=%d camera-records=%d sound-a=%d sound-b=%d friction-zones=%d friction-actors=%d"),
            Platforms, Prefixes, Details, Cameras, SoundA, SoundB, Friction, FrictionActors);
    }
    Bank->SceneEnvironment.Actors[0].Position = FVector(4, 5, 6);
    Bank->SceneEnvironment.Actors[0].VisibilityDistance = 123.0f;
    Bank->SceneEnvironment.Actors[0].DefaultBehaviourId = 7;
    const int32 ConditionActor = Bank->SceneEnvironment.Actors.IndexOfByPredicate([](const FKyaActorPlacement& Actor) { return !Actor.Conditions.IsEmpty(); });
    const int32 PlatformActor = Bank->SceneEnvironment.Actors.IndexOfByPredicate([](const FKyaActorPlacement& Actor) { return Actor.bHasPlatformPrefix; });
    const int32 PlatformDetailActor = Bank->SceneEnvironment.Actors.IndexOfByPredicate([](const FKyaActorPlacement& Actor) { return Actor.bHasPlatformDetails; });
    const int32 PlatformCameraActor = Bank->SceneEnvironment.Actors.IndexOfByPredicate([](const FKyaActorPlacement& Actor) { return Actor.bHasPlatformDetails && !Actor.PlatformCameras.IsEmpty(); });
    const int32 PlatformSoundActor = Bank->SceneEnvironment.Actors.IndexOfByPredicate([](const FKyaActorPlacement& Actor) { return Actor.bHasPlatformDetails && !Actor.PlatformSoundsA.IsEmpty(); });
    const int32 BasicBoxActor = Bank->SceneEnvironment.Actors.IndexOfByPredicate([](const FKyaActorPlacement& Actor) { return Actor.bHasBasicBoxData; });
    const int32 FxActor = Bank->SceneEnvironment.Actors.IndexOfByPredicate([](const FKyaActorPlacement& Actor) { return Actor.bHasFxData; });
    const int32 FxSingleActor = Bank->SceneEnvironment.Actors.IndexOfByPredicate([](const FKyaActorPlacement& Actor) {
        return Actor.Behaviours.ContainsByPredicate([](const FKyaActorBehaviour& Behaviour) { return Behaviour.Id == 2 && Behaviour.bHasTypedFxBehaviour; });
    });
    const int32 FxPathActor = Bank->SceneEnvironment.Actors.IndexOfByPredicate([](const FKyaActorPlacement& Actor) {
        return Actor.Behaviours.ContainsByPredicate([](const FKyaActorBehaviour& Behaviour) { return Behaviour.Id == 3 && Behaviour.bHasTypedFxBehaviour; });
    });
    const int32 AraignosActor = Bank->SceneEnvironment.Actors.IndexOfByPredicate([](const FKyaActorPlacement& Actor) { return Actor.bHasAraignosData; });
    const int32 ProjectileActor = Bank->SceneEnvironment.Actors.IndexOfByPredicate([](const FKyaActorPlacement& Actor) { return Actor.bHasProjectileData; });
    const int32 WindActor = Bank->SceneEnvironment.Actors.IndexOfByPredicate([](const FKyaActorPlacement& Actor) {
        return Actor.bHasWindData && !Actor.WindSwitch1D0.Targets.IsEmpty();
    });
    const int32 TrapActor = Bank->SceneEnvironment.Actors.IndexOfByPredicate([](const FKyaActorPlacement& Actor) { return Actor.bHasTrapData; });
    const int32 TrapStandActor = Bank->SceneEnvironment.Actors.IndexOfByPredicate([](const FKyaActorPlacement& Actor) {
        return Actor.Behaviours.ContainsByPredicate([](const FKyaActorBehaviour& Behaviour) { return Behaviour.Id == 2 && Behaviour.bHasTypedTrapBehaviour; });
    });
    const int32 TrapInactiveActor = Bank->SceneEnvironment.Actors.IndexOfByPredicate([](const FKyaActorPlacement& Actor) {
        return Actor.Behaviours.ContainsByPredicate([](const FKyaActorBehaviour& Behaviour) { return Behaviour.Id == 3 && Behaviour.bHasTypedTrapBehaviour; });
    });
    const int32 WeaponActor = Bank->SceneEnvironment.Actors.IndexOfByPredicate([](const FKyaActorPlacement& Actor) { return Actor.bHasWeaponData; });
    const int32 WeaponPistolActor = Bank->SceneEnvironment.Actors.IndexOfByPredicate([](const FKyaActorPlacement& Actor) {
        return Actor.Behaviours.ContainsByPredicate([](const FKyaActorBehaviour& Behaviour) { return Behaviour.Id == 3 && Behaviour.bHasTypedWeaponBehaviour; });
    });
    const int32 WeaponSniperActor = Bank->SceneEnvironment.Actors.IndexOfByPredicate([](const FKyaActorPlacement& Actor) {
        return Actor.Behaviours.ContainsByPredicate([](const FKyaActorBehaviour& Behaviour) { return Behaviour.Id == 4 && Behaviour.bHasTypedWeaponBehaviour; });
    });
    const int32 WeaponRpgActor = Bank->SceneEnvironment.Actors.IndexOfByPredicate([](const FKyaActorPlacement& Actor) {
        return Actor.Behaviours.ContainsByPredicate([](const FKyaActorBehaviour& Behaviour) { return Behaviour.Id == 6 && Behaviour.bHasTypedWeaponBehaviour; });
    });
    const int32 SwitchActor = Bank->SceneEnvironment.Actors.IndexOfByPredicate([](const FKyaActorPlacement& Actor) {
        return Actor.bHasSwitchData && !Actor.SwitchTarget.Targets.IsEmpty();
    });
    const int32 BonusActor = Bank->SceneEnvironment.Actors.IndexOfByPredicate([](const FKyaActorPlacement& Actor) { return Actor.bHasBonusData; });
    const int32 BonusTurnActor = Bank->SceneEnvironment.Actors.IndexOfByPredicate([](const FKyaActorPlacement& Actor) {
        return Actor.Behaviours.ContainsByPredicate([](const FKyaActorBehaviour& Behaviour) { return Behaviour.Id == 3 && Behaviour.bHasTypedBonusBehaviour; });
    });
    const int32 BonusFlockActor = Bank->SceneEnvironment.Actors.IndexOfByPredicate([](const FKyaActorPlacement& Actor) {
        return Actor.Behaviours.ContainsByPredicate([](const FKyaActorBehaviour& Behaviour) { return Behaviour.Id == 5 && Behaviour.bHasTypedBonusBehaviour; });
    });
    const int32 BonusAddOnActor = Bank->SceneEnvironment.Actors.IndexOfByPredicate([](const FKyaActorPlacement& Actor) {
        return Actor.Behaviours.ContainsByPredicate([](const FKyaActorBehaviour& Behaviour) { return Behaviour.Id == 6 && Behaviour.bHasTypedBonusBehaviour; });
    });
    const int32 MickenActor = Bank->SceneEnvironment.Actors.IndexOfByPredicate([](const FKyaActorPlacement& Actor) { return Actor.bHasMickenData; });
    const int32 MickenEatActor = Bank->SceneEnvironment.Actors.IndexOfByPredicate([](const FKyaActorPlacement& Actor) {
        return Actor.Behaviours.ContainsByPredicate([](const FKyaActorBehaviour& Behaviour) { return Behaviour.Id == 4 && Behaviour.bHasTypedMickenBehaviour; });
    });
    const int32 FruitActor = Bank->SceneEnvironment.Actors.IndexOfByPredicate([](const FKyaActorPlacement& Actor) { return Actor.bHasFruitData; });
    const int32 FruitSquashedActor = Bank->SceneEnvironment.Actors.IndexOfByPredicate([](const FKyaActorPlacement& Actor) {
        return Actor.Behaviours.ContainsByPredicate([](const FKyaActorBehaviour& Behaviour) { return Behaviour.Id == 4 && Behaviour.bHasTypedFruitBehaviour; });
    });
    const int32 FruitBonusActor = Bank->SceneEnvironment.Actors.IndexOfByPredicate([](const FKyaActorPlacement& Actor) {
        return Actor.Behaviours.ContainsByPredicate([](const FKyaActorBehaviour& Behaviour) { return Behaviour.Id == 6 && Behaviour.bHasTypedFruitBehaviour; });
    });
    const int32 FruitInventoryActor = Bank->SceneEnvironment.Actors.IndexOfByPredicate([](const FKyaActorPlacement& Actor) {
        return Actor.Behaviours.ContainsByPredicate([](const FKyaActorBehaviour& Behaviour) { return Behaviour.Id == 5 && Behaviour.bHasTypedFruitBehaviour; });
    });
    const int32 CommanderActor = Bank->SceneEnvironment.Actors.IndexOfByPredicate([](const FKyaActorPlacement& Actor) { return Actor.bHasCommanderData; });
    const int32 CommanderBehaviourActor = Bank->SceneEnvironment.Actors.IndexOfByPredicate([](const FKyaActorPlacement& Actor) {
        return Actor.Behaviours.ContainsByPredicate([](const FKyaActorBehaviour& Behaviour) { return Behaviour.Id == 2 && Behaviour.bHasTypedCommanderBehaviour; });
    });
    const int32 NativActor = Bank->SceneEnvironment.Actors.IndexOfByPredicate([](const FKyaActorPlacement& Actor) { return Actor.bHasNativData; });
    const int32 NativBehaviourActor = Bank->SceneEnvironment.Actors.IndexOfByPredicate([](const FKyaActorPlacement& Actor) {
        return Actor.Behaviours.ContainsByPredicate([](const FKyaActorBehaviour& Behaviour) { return Behaviour.Id == 6 && Behaviour.bHasTypedNativBehaviour; });
    });
    const int32 AmortosActor = Bank->SceneEnvironment.Actors.IndexOfByPredicate([](const FKyaActorPlacement& Actor) { return Actor.bHasAmortosData; });
    const int32 AmortosBehaviourActor = Bank->SceneEnvironment.Actors.IndexOfByPredicate([](const FKyaActorPlacement& Actor) {
        return Actor.ClassId == 0x29 && Actor.Behaviours.ContainsByPredicate([](const FKyaActorBehaviour& Behaviour) { return Behaviour.Id == 1 && Behaviour.bHasTypedCinematicBehaviour; });
    });
    const int32 TeleporterActor = Bank->SceneEnvironment.Actors.IndexOfByPredicate([](const FKyaActorPlacement& Actor) {
        return Actor.bHasTeleporterData && !Actor.TeleporterDestinations.IsEmpty();
    });
    const int32 DcaActor = Bank->SceneEnvironment.Actors.IndexOfByPredicate([](const FKyaActorPlacement& Actor) { return Actor.bHasDcaData; });
    const int32 DcaControlledActor = Bank->SceneEnvironment.Actors.IndexOfByPredicate([](const FKyaActorPlacement& Actor) {
        return Actor.Behaviours.ContainsByPredicate([](const FKyaActorBehaviour& Behaviour) { return Behaviour.Id == 3 && Behaviour.bHasTypedDcaControlledBehaviour; });
    });
    const int32 DcaTrajectoryActor = Bank->SceneEnvironment.Actors.IndexOfByPredicate([](const FKyaActorPlacement& Actor) {
        return Actor.Behaviours.ContainsByPredicate([](const FKyaActorBehaviour& Behaviour) { return Behaviour.Id == 5 && Behaviour.bHasTypedDcaTrajectoryBehaviour; });
    });
    const int32 BoxClassActor = Bank->SceneEnvironment.Actors.IndexOfByPredicate([](const FKyaActorPlacement& Actor) { return Actor.bHasBoxData; });
    const int32 MoneyActor = Bank->SceneEnvironment.Actors.IndexOfByPredicate([](const FKyaActorPlacement& Actor) { return Actor.bHasMoneyData; });
    const int32 FogActor = Bank->SceneEnvironment.Actors.IndexOfByPredicate([](const FKyaActorPlacement& Actor) {
        return Actor.bHasFogData && !Actor.FogHierarchies.IsEmpty();
    });
    const int32 FogBehaviourActor = Bank->SceneEnvironment.Actors.IndexOfByPredicate([](const FKyaActorPlacement& Actor) {
        return Actor.Behaviours.ContainsByPredicate([](const FKyaActorBehaviour& Behaviour) { return Behaviour.Id == 2 && Behaviour.bHasTypedFogBehaviour; });
    });
    const int32 ShootActor = Bank->SceneEnvironment.Actors.IndexOfByPredicate([](const FKyaActorPlacement& Actor) { return Actor.bHasShootData; });
    const int32 ShootFireActor = Bank->SceneEnvironment.Actors.IndexOfByPredicate([](const FKyaActorPlacement& Actor) {
        return Actor.Behaviours.ContainsByPredicate([](const FKyaActorBehaviour& Behaviour) { return Behaviour.Id == 3 && Behaviour.bHasTypedShootFireBehaviour; });
    });
    const int32 AmbreActor = Bank->SceneEnvironment.Actors.IndexOfByPredicate([](const FKyaActorPlacement& Actor) { return Actor.bHasAmbreData; });
    const int32 StillerActor = Bank->SceneEnvironment.Actors.IndexOfByPredicate([](const FKyaActorPlacement& Actor) {
        return Actor.bHasStillerData && !Actor.StillerSwitch208.Targets.IsEmpty();
    });
    const int32 RuneActor = Bank->SceneEnvironment.Actors.IndexOfByPredicate([](const FKyaActorPlacement& Actor) { return Actor.bHasRuneData; });
    const int32 FountainActor = Bank->SceneEnvironment.Actors.IndexOfByPredicate([](const FKyaActorPlacement& Actor) { return Actor.bHasBonusFountainData; });
    const int32 RopeActor = Bank->SceneEnvironment.Actors.IndexOfByPredicate([](const FKyaActorPlacement& Actor) { return Actor.bHasRopeData; });
    const int32 GravityActor = Bank->SceneEnvironment.Actors.IndexOfByPredicate([](const FKyaActorPlacement& Actor) { return Actor.bHasGravityAwareData; });
    const int32 NativCmdActor = Bank->SceneEnvironment.Actors.IndexOfByPredicate([](const FKyaActorPlacement& Actor) { return Actor.bHasNativCmdData; });
    const int32 CompanionActor = Bank->SceneEnvironment.Actors.IndexOfByPredicate([](const FKyaActorPlacement& Actor) { return Actor.bHasCompanionData; });
    const int32 BridgeActor = Bank->SceneEnvironment.Actors.IndexOfByPredicate([](const FKyaActorPlacement& Actor) {
        return Actor.bHasBridgeData && !Actor.BridgeActorRefs.IsEmpty();
    });
    const int32 AtonActor = Bank->SceneEnvironment.Actors.IndexOfByPredicate([](const FKyaActorPlacement& Actor) {
        return Actor.bHasAtonData && !Actor.AtonOtherSubObjs.IsEmpty();
    });
    const int32 BoomyActor = Bank->SceneEnvironment.Actors.IndexOfByPredicate([](const FKyaActorPlacement& Actor) {
        return Actor.bHasBoomyData && !Actor.BoomyTypes.IsEmpty();
    });
    const int32 KimActor = Bank->SceneEnvironment.Actors.IndexOfByPredicate([](const FKyaActorPlacement& Actor) {
        return Actor.bHasKimData && !Actor.KimTutorialZones.IsEmpty();
    });
    const int32 CompanionAlertActor = Bank->SceneEnvironment.Actors.IndexOfByPredicate([](const FKyaActorPlacement& Actor) {
        return Actor.Behaviours.ContainsByPredicate([](const FKyaActorBehaviour& Behaviour) {
            return Behaviour.Id == 3 && Behaviour.bHasTypedCompanionBehaviour && Behaviour.CompanionAlerts.Num() >= 2;
        });
    });
    const int32 PlatformStandActor = Bank->SceneEnvironment.Actors.IndexOfByPredicate([](const FKyaActorPlacement& Actor) {
        return Actor.Behaviours.ContainsByPredicate([](const FKyaActorBehaviour& Behaviour) {
            return Behaviour.Id == 7 && Behaviour.bHasTypedPlatformBehaviour;
        });
    });
    const int32 PlatformSelectorActor = Bank->SceneEnvironment.Actors.IndexOfByPredicate([](const FKyaActorPlacement& Actor) {
        return Actor.Behaviours.ContainsByPredicate([](const FKyaActorBehaviour& Behaviour) {
            return Behaviour.Id == 9 && Behaviour.bHasTypedPlatformBehaviour;
        });
    });
    const int32 PlatformTrajectoryActor = Bank->SceneEnvironment.Actors.IndexOfByPredicate([](const FKyaActorPlacement& Actor) {
        return Actor.Behaviours.ContainsByPredicate([](const FKyaActorBehaviour& Behaviour) {
            return Behaviour.Id == 2 && Behaviour.bHasTypedPlatformBehaviour;
        });
    });
    const auto FindPlatformBehaviourActor = [&Bank](int32 Id) {
        return Bank->SceneEnvironment.Actors.IndexOfByPredicate([Id](const FKyaActorPlacement& Actor) {
            return Actor.Behaviours.ContainsByPredicate([Id](const FKyaActorBehaviour& Behaviour) {
                return Behaviour.Id == Id && Behaviour.bHasTypedPlatformBehaviour;
            });
        });
    };
    const int32 PlatformSlabActor = FindPlatformBehaviourActor(3);
    const int32 PlatformMasterActor = FindPlatformBehaviourActor(5);
    const int32 PlatformSelectorMasterActor = FindPlatformBehaviourActor(8);
    const int32 ProjectileStandActor = Bank->SceneEnvironment.Actors.IndexOfByPredicate([](const FKyaActorPlacement& Actor) {
        return Actor.Behaviours.ContainsByPredicate([](const FKyaActorBehaviour& Behaviour) {
            return Behaviour.Id == 3 && Behaviour.bHasTypedProjectileBehaviour;
        });
    });
    const int32 ProjectileStraightActor = Bank->SceneEnvironment.Actors.IndexOfByPredicate([](const FKyaActorPlacement& Actor) {
        return Actor.Behaviours.ContainsByPredicate([](const FKyaActorBehaviour& Behaviour) {
            return Behaviour.Id == 4 && Behaviour.bHasTypedProjectileBehaviour;
        });
    });
    const int32 SwitchLeverActor = Bank->SceneEnvironment.Actors.IndexOfByPredicate([](const FKyaActorPlacement& Actor) {
        return Actor.Behaviours.ContainsByPredicate([](const FKyaActorBehaviour& Behaviour) {
            return Behaviour.Id == 2 && Behaviour.bHasTypedSwitchBehaviour;
        });
    });
    const int32 SwitchMagicActor = Bank->SceneEnvironment.Actors.IndexOfByPredicate([](const FKyaActorPlacement& Actor) {
        return Actor.Behaviours.ContainsByPredicate([](const FKyaActorBehaviour& Behaviour) {
            return Behaviour.Id == 3 && Behaviour.bHasTypedSwitchBehaviour;
        });
    });
    const auto FindWolfenBehaviourActor = [&Bank](int32 Id) {
        return Bank->SceneEnvironment.Actors.IndexOfByPredicate([Id](const FKyaActorPlacement& Actor) {
            return Actor.Behaviours.ContainsByPredicate([Id](const FKyaActorBehaviour& Behaviour) {
                return Behaviour.Id == Id && Behaviour.bHasTypedWolfenBehaviour;
            });
        });
    };
    const int32 WolfenTrackActor = FindWolfenBehaviourActor(15);
    const int32 WolfenTrackWeaponActor = FindWolfenBehaviourActor(17);
    const int32 WolfenTrackWeaponStandActor = FindWolfenBehaviourActor(18);
    const int32 WolfenTrackWeaponSnipeActor = FindWolfenBehaviourActor(19);
    const int32 WolfenDcaActor = FindWolfenBehaviourActor(23);
    const int32 WolfenWatchDogActor = FindWolfenBehaviourActor(8);
    const int32 WolfenSnipeActor = FindWolfenBehaviourActor(21);
    const int32 WolfenGuardActor = FindWolfenBehaviourActor(10);
    const int32 WolfenEscapeActor = FindWolfenBehaviourActor(13);
    const int32 WolfenExorcismActor = FindWolfenBehaviourActor(14);
    const int32 CatchActor = Bank->SceneEnvironment.Actors.IndexOfByPredicate([](const FKyaActorPlacement& Actor) {
        return Actor.Behaviours.ContainsByPredicate([](const FKyaActorBehaviour& Behaviour) {
            return Behaviour.bHasTypedCatchByTrapBehaviour && !Behaviour.CatchOperations.IsEmpty();
        });
    });
    const int32 HeroUnknownActor = Bank->SceneEnvironment.Actors.IndexOfByPredicate([](const FKyaActorPlacement& Actor) {
        return Actor.Behaviours.ContainsByPredicate([](const FKyaActorBehaviour& Behaviour) {
            return Behaviour.bHasTypedHeroUnknownBehaviour;
        });
    });
    const int32 InventoryActor = Bank->SceneEnvironment.Actors.IndexOfByPredicate([](const FKyaActorPlacement& Actor) {
        return Actor.Behaviours.ContainsByPredicate([](const FKyaActorBehaviour& Behaviour) {
            return Behaviour.bHasTypedInventoryBehaviour;
        });
    });
    const int32 WolfenLostActor = Bank->SceneEnvironment.Actors.IndexOfByPredicate([](const FKyaActorPlacement& Actor) {
        return Actor.Behaviours.ContainsByPredicate([](const FKyaActorBehaviour& Behaviour) {
            return Behaviour.Id == 22 && Behaviour.bHasTypedWolfenBehaviour && !Behaviour.WolfenBehaviour.SwitchRecords.IsEmpty();
        });
    });
    const int32 WolfenAvoidActor = FindWolfenBehaviourActor(24);
    const int32 WolfenActor = Bank->SceneEnvironment.Actors.IndexOfByPredicate([](const FKyaActorPlacement& Actor) {
        return Actor.bHasWolfenData && !Actor.WolfenFightParamData.Capabilities.IsEmpty() && !Actor.FighterBlows.IsEmpty();
    });
    const int32 CheckpointActor = Bank->SceneEnvironment.Actors.IndexOfByPredicate([](const FKyaActorPlacement& Actor) { return Actor.ClassId == 0x1e && !Actor.Checkpoints.IsEmpty(); });
    if (!TestTrue(TEXT("Level has actor scenario conditions"), ConditionActor != INDEX_NONE) ||
        !TestTrue(TEXT("Level has checkpoint records"), CheckpointActor != INDEX_NONE) ||
        !TestTrue(TEXT("Level has moving platform data"), PlatformActor != INDEX_NONE) ||
        !TestTrue(TEXT("Level has moving platform details"), PlatformDetailActor != INDEX_NONE) ||
        !TestTrue(TEXT("Level has moving platform camera details"), PlatformCameraActor != INDEX_NONE) ||
        !TestTrue(TEXT("Level has moving platform sound details"), PlatformSoundActor != INDEX_NONE) ||
        !TestTrue(TEXT("Level has basic box data"), BasicBoxActor != INDEX_NONE) ||
        !TestTrue(TEXT("Level has FX data"), FxActor != INDEX_NONE) ||
        !TestTrue(TEXT("Level has FX single behaviour"), FxSingleActor != INDEX_NONE) ||
        !TestTrue(TEXT("Level has FX path behaviour"), FxPathActor != INDEX_NONE) ||
        !TestTrue(TEXT("Level has Araignos data"), AraignosActor != INDEX_NONE) ||
        !TestTrue(TEXT("Level has projectile data"), ProjectileActor != INDEX_NONE) ||
        !TestTrue(TEXT("Level has wind data with switches"), WindActor != INDEX_NONE) ||
        !TestTrue(TEXT("Level has trap data"), TrapActor != INDEX_NONE) ||
        !TestTrue(TEXT("Level has trap stand behaviour"), TrapStandActor != INDEX_NONE) ||
        !TestTrue(TEXT("Level has trap inactive behaviour"), TrapInactiveActor != INDEX_NONE) ||
        !TestTrue(TEXT("Level has weapon data"), WeaponActor != INDEX_NONE) ||
        !TestTrue(TEXT("Level has pistol weapon behaviour"), WeaponPistolActor != INDEX_NONE) ||
        !TestTrue(TEXT("Level has sniper weapon behaviour"), WeaponSniperActor != INDEX_NONE) ||
        !TestTrue(TEXT("Level has RPG weapon behaviour"), WeaponRpgActor != INDEX_NONE) ||
        !TestTrue(TEXT("Level has switch data with targets"), SwitchActor != INDEX_NONE) ||
        !TestTrue(TEXT("Level has bonus data"), BonusActor != INDEX_NONE) ||
        !TestTrue(TEXT("Level has bonus turn behaviour"), BonusTurnActor != INDEX_NONE) ||
        !TestTrue(TEXT("Level has bonus flock behaviour"), BonusFlockActor != INDEX_NONE) ||
        !TestTrue(TEXT("Level has bonus add-on behaviour"), BonusAddOnActor != INDEX_NONE) ||
        !TestTrue(TEXT("Level has Micken data"), MickenActor != INDEX_NONE) ||
        !TestTrue(TEXT("Level has Micken eat behaviour"), MickenEatActor != INDEX_NONE) ||
        !TestTrue(TEXT("Level has Wolfen data"), WolfenActor != INDEX_NONE) ||
        !TestTrue(TEXT("Level has fruit data"), FruitActor != INDEX_NONE) ||
        !TestTrue(TEXT("Level has fruit squashed behaviour"), FruitSquashedActor != INDEX_NONE) ||
        !TestTrue(TEXT("Level has fruit bonus behaviour"), FruitBonusActor != INDEX_NONE) ||
        !TestTrue(TEXT("Level has fruit inventory behaviour"), FruitInventoryActor != INDEX_NONE) ||
        !TestTrue(TEXT("Level has commander data"), CommanderActor != INDEX_NONE) ||
        !TestTrue(TEXT("Level has commander behaviour"), CommanderBehaviourActor != INDEX_NONE) ||
        !TestTrue(TEXT("Level has Nativ data"), NativActor != INDEX_NONE) ||
        !TestTrue(TEXT("Level has Nativ take-and-put behaviour"), NativBehaviourActor != INDEX_NONE) ||
        !TestTrue(TEXT("Level has Amortos data"), AmortosActor != INDEX_NONE) ||
        !TestTrue(TEXT("Level has Amortos cinematic behaviour"), AmortosBehaviourActor != INDEX_NONE) ||
        !TestTrue(TEXT("Level has teleporter data"), TeleporterActor != INDEX_NONE) ||
        !TestTrue(TEXT("Level has DCA data"), DcaActor != INDEX_NONE) ||
        !TestTrue(TEXT("Level has DCA controlled behaviour"), DcaControlledActor != INDEX_NONE) ||
        !TestTrue(TEXT("Level has DCA trajectory behaviour"), DcaTrajectoryActor != INDEX_NONE) ||
        !TestTrue(TEXT("Level has box data"), BoxClassActor != INDEX_NONE) ||
        !TestTrue(TEXT("Level has money data"), MoneyActor != INDEX_NONE) ||
        !TestTrue(TEXT("Level has fog data"), FogActor != INDEX_NONE) ||
        !TestTrue(TEXT("Level has fog behaviour"), FogBehaviourActor != INDEX_NONE) ||
        !TestTrue(TEXT("Level has shooter data"), ShootActor != INDEX_NONE) ||
        !TestTrue(TEXT("Level has shooter fire behaviour"), ShootFireActor != INDEX_NONE) ||
        !TestTrue(TEXT("Level has Ambre data"), AmbreActor != INDEX_NONE) ||
        !TestTrue(TEXT("Level has Stiller data"), StillerActor != INDEX_NONE) ||
        !TestTrue(TEXT("Level has rune data"), RuneActor != INDEX_NONE) ||
        !TestTrue(TEXT("Level has bonus fountain data"), FountainActor != INDEX_NONE) ||
        !TestTrue(TEXT("Level has rope data"), RopeActor != INDEX_NONE) ||
        !TestTrue(TEXT("Level has gravity-aware data"), GravityActor != INDEX_NONE) ||
        !TestTrue(TEXT("Level has nativ command data"), NativCmdActor != INDEX_NONE) ||
        !TestTrue(TEXT("Level has companion data"), CompanionActor != INDEX_NONE) ||
        !TestTrue(TEXT("Level has bridge data"), BridgeActor != INDEX_NONE) ||
        !TestTrue(TEXT("Level has Aton data"), AtonActor != INDEX_NONE) ||
        !TestTrue(TEXT("Level has Boomy data"), BoomyActor != INDEX_NONE) ||
        !TestTrue(TEXT("Level has Kim data"), KimActor != INDEX_NONE) ||
        !TestTrue(TEXT("Level has companion alerts"), CompanionAlertActor != INDEX_NONE) ||
        !TestTrue(TEXT("Level has a platform stand behaviour"), PlatformStandActor != INDEX_NONE) ||
        !TestTrue(TEXT("Level has a platform selector-slave behaviour"), PlatformSelectorActor != INDEX_NONE) ||
        !TestTrue(TEXT("Level has a platform trajectory behaviour"), PlatformTrajectoryActor != INDEX_NONE) ||
        !TestTrue(TEXT("Level has a projectile stand behaviour"), ProjectileStandActor != INDEX_NONE) ||
        !TestTrue(TEXT("Level has a projectile straight behaviour"), ProjectileStraightActor != INDEX_NONE) ||
        !TestTrue(TEXT("Level has a switch lever behaviour"), SwitchLeverActor != INDEX_NONE) ||
        !TestTrue(TEXT("Level has a switch magic behaviour"), SwitchMagicActor != INDEX_NONE) ||
        !TestTrue(TEXT("Level has a Wolfen track behaviour"), WolfenTrackActor != INDEX_NONE) ||
        !TestTrue(TEXT("Level has a Wolfen track-weapon behaviour"), WolfenTrackWeaponActor != INDEX_NONE) ||
        !TestTrue(TEXT("Level has a Wolfen track-weapon-stand behaviour"), WolfenTrackWeaponStandActor != INDEX_NONE) ||
        !TestTrue(TEXT("Level has a Wolfen track-weapon-snipe behaviour"), WolfenTrackWeaponSnipeActor != INDEX_NONE) ||
        !TestTrue(TEXT("Level has a Wolfen DCA behaviour"), WolfenDcaActor != INDEX_NONE) ||
        !TestTrue(TEXT("Level has a Wolfen avoid behaviour"), WolfenAvoidActor != INDEX_NONE) ||
        !TestTrue(TEXT("Level has a Wolfen lost behaviour"), WolfenLostActor != INDEX_NONE) ||
        !TestTrue(TEXT("Level has a Wolfen watchdog behaviour"), WolfenWatchDogActor != INDEX_NONE) ||
        !TestTrue(TEXT("Level has a Wolfen snipe behaviour"), WolfenSnipeActor != INDEX_NONE) ||
        !TestTrue(TEXT("Level has a Wolfen guard-area behaviour"), WolfenGuardActor != INDEX_NONE) ||
        !TestTrue(TEXT("Level has a Wolfen escape behaviour"), WolfenEscapeActor != INDEX_NONE) ||
        !TestTrue(TEXT("Level has a Wolfen exorcism behaviour"), WolfenExorcismActor != INDEX_NONE) ||
        !TestTrue(TEXT("Level has a catch-by-trap behaviour"), CatchActor != INDEX_NONE) ||
        !TestTrue(TEXT("Level has a hero-unknown behaviour"), HeroUnknownActor != INDEX_NONE) ||
        !TestTrue(TEXT("Level has a projectile inventory behaviour"), InventoryActor != INDEX_NONE)) return false;
    Bank->SceneEnvironment.Actors[ConditionActor].Conditions[0].Value = 197;
    Bank->SceneEnvironment.Actors[CheckpointActor].Checkpoints[0].SectorId = 87;
    Bank->SceneEnvironment.Actors[PlatformActor].PlatformWeight = 12.5f;
    {
        FKyaPlatformProperties& Properties = Bank->SceneEnvironment.Actors[PlatformDetailActor].PlatformProperties;
        Properties.TiltValue = 6.75f;
        Properties.SoundRefA = 5;
        Properties.SampleRefA = 6;
        Properties.EffectRef1C = 0x1234;
        Properties.EffectRef20 = 0x5678;
        Properties.PlatformFlags = 0x9abc;
        Properties.CameraIndex = 23;
    }
    if (PlatformCameraActor != INDEX_NONE)
    {
        FKyaPlatformCamera& Camera = Bank->SceneEnvironment.Actors[PlatformCameraActor].PlatformCameras[0];
        Camera.EventType ^= 1;
        Camera.CameraIndex ^= 1;
        Camera.Field8 = 1.5f;
    }
    if (PlatformSoundActor != INDEX_NONE)
    {
        FKyaPlatformSound& Sound = Bank->SceneEnvironment.Actors[PlatformSoundActor].PlatformSoundsA[0];
        Sound.SegmentStart = 5;
        Sound.SegmentEnd = 9;
        Sound.SoundRef = 81;
    }
    {
        FKyaActorPlacement& BoxActor = Bank->SceneEnvironment.Actors[BasicBoxActor];
        BoxActor.BasicBoxField160 = 77;
        BoxActor.BasicBoxLifeMax = 25.0f;
        BoxActor.BasicBoxVibration0 = 1.25f;
        BoxActor.BasicBoxVibration1 = 2.25f;
        BoxActor.BasicBoxVibration2 = 3.25f;
        BoxActor.BasicBoxVibration3 = 4.25f;
        BoxActor.BasicBoxField180 = 0x1234;
        BoxActor.BasicBoxField18C = 11;
        BoxActor.BasicBoxField190 = 12;
        BoxActor.BasicBoxField198 = 13;
        BoxActor.BasicBoxField194 = 14;
        FKyaStreamEventCamera& BoxCamera = BoxActor.BasicBoxSwitch.Camera;
        BoxCamera.CameraIndex = 9;
        BoxCamera.Flags = 39;
        BoxCamera.Field8 = 1.5f;
        BoxCamera.FieldC = 2.5f;
        BoxCamera.Field10 = 3.5f;
        BoxCamera.ActorRef = 41;
        BoxCamera.ZoneRef = 43;
        BoxCamera.Field1C = 4.5f;
        FKyaAddOnGenerator& AddOn = BoxActor.BasicBoxAddOn;
        AddOn.Field0 = 1;
        AddOn.MoneyType = 2;
        AddOn.MoneyCount = 3;
        AddOn.BonusCount = 4;
        AddOn.FieldC = 5;
    }
    const int32 PlatformWithActors = Bank->SceneEnvironment.Actors.IndexOfByPredicate([](const FKyaActorPlacement& Actor) { return Actor.bHasPlatformPrefix && !Actor.PlatformActors.IsEmpty(); });
    const int32 PlatformWithZones = Bank->SceneEnvironment.Actors.IndexOfByPredicate([](const FKyaActorPlacement& Actor) { return Actor.bHasPlatformPrefix && !Actor.PlatformZones.IsEmpty(); });
    const int32 PlatformWithFriction = Bank->SceneEnvironment.Actors.IndexOfByPredicate([](const FKyaActorPlacement& Actor) { return Actor.bHasPlatformPrefix && !Actor.PlatformNoFrictionZones.IsEmpty(); });
    const int32 PlatformWithFrictionActors = Bank->SceneEnvironment.Actors.IndexOfByPredicate([](const FKyaActorPlacement& Actor) {
        return Actor.bHasPlatformPrefix && !Actor.PlatformNoFrictionZones.IsEmpty() && !Actor.PlatformNoFrictionZones[0].Actors.IsEmpty();
    });
    if (PlatformWithActors != INDEX_NONE) Bank->SceneEnvironment.Actors[PlatformWithActors].PlatformActors[0].CarryMethod = 77;
    if (PlatformWithZones != INDEX_NONE) Bank->SceneEnvironment.Actors[PlatformWithZones].PlatformZones[0] = 78;
    if (PlatformWithFriction != INDEX_NONE) Bank->SceneEnvironment.Actors[PlatformWithFriction].PlatformNoFrictionZones[0].ZoneRef = 79;
    if (PlatformWithFrictionActors != INDEX_NONE) Bank->SceneEnvironment.Actors[PlatformWithFrictionActors].PlatformNoFrictionZones[0].Actors[0] = 80;
    Bank->SceneEnvironment.Actors[FxActor].FxField19C = 0.75f;
    Bank->SceneEnvironment.Actors[FxActor].FxField1A0 = 2.5f;
    const int32 FxSingleBehaviour = Bank->SceneEnvironment.Actors[FxSingleActor].Behaviours.IndexOfByPredicate(
        [](const FKyaActorBehaviour& Behaviour) { return Behaviour.Id == 2; });
    const int32 FxPathBehaviour = Bank->SceneEnvironment.Actors[FxPathActor].Behaviours.IndexOfByPredicate(
        [](const FKyaActorBehaviour& Behaviour) { return Behaviour.Id == 3; });
    Bank->SceneEnvironment.Actors[FxSingleActor].Behaviours[FxSingleBehaviour].FxBehaviourValue = 123;
    Bank->SceneEnvironment.Actors[FxPathActor].Behaviours[FxPathBehaviour].FxBehaviourValue = 456;
    Bank->SceneEnvironment.Actors[FxPathActor].Behaviours[FxPathBehaviour].FxPathFollowIndex = 7;
    {
        FKyaActorPlacement& Araignos = Bank->SceneEnvironment.Actors[AraignosActor];
        Araignos.AraignosWeightB = 1.5f;
        Araignos.AraignosPathFollowIndex = 8;
        Araignos.AraignosField1F8 = 2.5f;
        Araignos.AraignosField1FC = 3.5f;
        Araignos.AraignosDamage = 4.5f;
        Araignos.AraignosSoundRef240 = 51;
        Araignos.AraignosSoundRef238 = 52;
        Araignos.AraignosSoundRef23C = 53;
        Araignos.AraignosAddOn.Field0 = 1;
        Araignos.AraignosAddOn.MoneyType = 2;
        Araignos.AraignosAddOn.MoneyCount = 3;
        Araignos.AraignosAddOn.BonusCount = 4;
        Araignos.AraignosAddOn.FieldC = 5;
    }
    {
        FKyaActorPlacement& Projectile = Bank->SceneEnvironment.Actors[ProjectileActor];
        Projectile.ProjectileWeightB = 1.25f;
        Projectile.ProjectileLifeMax = 30.0f;
        Projectile.ProjectileGravityScale = 2.5f;
        Projectile.ProjectileDynamicExtField4 = 3.5f;
        Projectile.ProjectileDynamicExtField8 = 4.5f;
        FKyaProjectileSubObj& Sub = Projectile.ProjectileSubObj;
        Sub.TimeToExplode = 5.5f; Sub.ExplosionRange = 6.5f; Sub.AnimDuration = 7.5f;
        Sub.Flags = 0x1234; Sub.Damage = 8.5f; Sub.HitRadius = 9.5f; Sub.WarnRadius = 10.5f;
        Sub.Field1C = 11.5f; Sub.Field20 = 12.5f; Sub.Field24 = 13.5f; Sub.Field28 = 14;
        Sub.MaterialId = 15; Sub.Field30 = 16; Sub.Field34 = 17; Sub.Field38 = 18; Sub.Field3C = 19;
        Sub.Field40 = 20.5f; Sub.Field44 = 21.5f;
        FKyaPatternPart& Pattern = Projectile.ProjectilePattern;
        Pattern.Field4 = 22.5f; Pattern.Field0 = 23.5f; Pattern.FieldC = 24.5f; Pattern.Field8 = 25.5f;
        Pattern.Field10 = 26.5f; Pattern.Field14 = 27.5f; Pattern.MaterialId = 28; Pattern.PointCount = 2.5f;
        Pattern.Field20 = 29.5f; Pattern.Field24 = 30.5f; Pattern.Field28 = 31;
    }
    {
        FKyaActorPlacement& Wind = Bank->SceneEnvironment.Actors[WindActor];
        Wind.WindField160 = 0x1234;
        Wind.WindMaxWind = 1.5f; Wind.WindField168 = 2.5f; Wind.WindField16C = 3.5f;
        Wind.WindField174 = 4.5f; Wind.WindField180 = 5.5f; Wind.WindField184 = 6.5f;
        Wind.WindField188 = 7.5f; Wind.WindField18C = 8.5f; Wind.WindField178 = 9.5f;
        Wind.WindField17C = 10.5f; Wind.WindField190 = 0x2345;
        Wind.WindSoundRef = 61; Wind.WindActivationZoneIndex = 2;
        if (!Wind.WindActorRefs.IsEmpty()) Wind.WindActorRefs[0] = 71;
        Wind.WindMaterialId = 62; Wind.WindZoneRef1C4 = 3; Wind.WindColor1C8 = 0x11223344;
        Wind.WindWaypointRef1CC = 4; Wind.WindSectorHierarchyHash = int64(0x123456789abcdef0ull);
        if (!Wind.WindSwitch1D0.Targets.IsEmpty()) Wind.WindSwitch1D0.Targets[0].TargetRef = 72;
        if (!Wind.WindSwitch1D0.Targets.IsEmpty()) Wind.WindSwitch1D0.Targets[0].MessageId = 73;
        Wind.WindSwitch1D0.Camera.CameraIndex = 5; Wind.WindSwitch1D0.Camera.Field8 = 1.25f;
        Wind.WindSwitch1D8.Camera.CameraIndex = 6;
        if (!Wind.WindSwitch1E0.Targets.IsEmpty()) Wind.WindSwitch1E0.Targets[0].TargetRef = 74;
        Wind.WindSwitch1E0.Camera.CameraIndex = 7;
        Wind.WindField194 = 11.5f; Wind.WindField198 = 12.5f; Wind.WindField19C = 13.5f;
        Wind.WindField1A0 = 14.5f; Wind.WindField1A4 = 15.5f;
        Wind.WindWaypointRef = 8; Wind.WindField1AC = 16.5f; Wind.WindField1B0 = 17.5f;
    }
    {
        FKyaActorPlacement& Trap = Bank->SceneEnvironment.Actors[TrapActor];
        Trap.TrapWeightB = 1.75f;
        Trap.TrapField14 = 21; Trap.TrapField18 = 22; Trap.TrapField1C = 23; Trap.TrapField20 = 24;
        Trap.TrapField24 = 25;
        Trap.TrapActorRef = 9;
        Trap.TrapSpeed = 2.25f; Trap.TrapAcceleration = 3.25f; Trap.TrapPathFollowIndex = 10;
        Trap.TrapField4C = 4.25f; Trap.TrapRequiredEscapeAttempts = 11;
    }
    const int32 TrapStandBehaviour = Bank->SceneEnvironment.Actors[TrapStandActor].Behaviours.IndexOfByPredicate(
        [](const FKyaActorBehaviour& Behaviour) { return Behaviour.Id == 2; });
    const int32 TrapInactiveBehaviour = Bank->SceneEnvironment.Actors[TrapInactiveActor].Behaviours.IndexOfByPredicate(
        [](const FKyaActorBehaviour& Behaviour) { return Behaviour.Id == 3; });
    Bank->SceneEnvironment.Actors[TrapStandActor].Behaviours[TrapStandBehaviour].TrapBehaviourField0 = 111;
    Bank->SceneEnvironment.Actors[TrapStandActor].Behaviours[TrapStandBehaviour].TrapBehaviourField4 = 112;
    Bank->SceneEnvironment.Actors[TrapStandActor].Behaviours[TrapStandBehaviour].TrapBehaviourField8 = 113;
    Bank->SceneEnvironment.Actors[TrapInactiveActor].Behaviours[TrapInactiveBehaviour].TrapBehaviourField0 = 121;
    Bank->SceneEnvironment.Actors[TrapInactiveActor].Behaviours[TrapInactiveBehaviour].TrapBehaviourField4 = 122;
    Bank->SceneEnvironment.Actors[TrapInactiveActor].Behaviours[TrapInactiveBehaviour].TrapBehaviourField8 = 123;
    {
        FKyaActorPlacement& Weapon = Bank->SceneEnvironment.Actors[WeaponActor];
        Weapon.WeaponWeightB = 0.75f;
        Weapon.WeaponField1D8 = 1.75f;
        Weapon.WeaponField1DC = 2.75f;
    }
    const int32 WeaponPistolBehaviour = Bank->SceneEnvironment.Actors[WeaponPistolActor].Behaviours.IndexOfByPredicate(
        [](const FKyaActorBehaviour& Behaviour) { return Behaviour.Id == 3; });
    const int32 WeaponSniperBehaviour = Bank->SceneEnvironment.Actors[WeaponSniperActor].Behaviours.IndexOfByPredicate(
        [](const FKyaActorBehaviour& Behaviour) { return Behaviour.Id == 4; });
    const int32 WeaponRpgBehaviour = Bank->SceneEnvironment.Actors[WeaponRpgActor].Behaviours.IndexOfByPredicate(
        [](const FKyaActorBehaviour& Behaviour) { return Behaviour.Id == 6; });
    {
        FKyaActorBehaviour& Pistol = Bank->SceneEnvironment.Actors[WeaponPistolActor].Behaviours[WeaponPistolBehaviour];
        Pistol.WeaponBurst[0].FieldC = 3.75f; Pistol.WeaponBurst[0].Field4 = 4;
        Pistol.WeaponBurst[0].Field0 = 5.75f; Pistol.WeaponBurst[0].Field8 = 6.75f;
        Pistol.WeaponField40 = 41; Pistol.WeaponField44 = 42; Pistol.WeaponSoundIndex = 43;
        Pistol.WeaponPistolField2E0 = 44; Pistol.WeaponPistolField2E4 = 45;
    }
    {
        FKyaActorBehaviour& Sniper = Bank->SceneEnvironment.Actors[WeaponSniperActor].Behaviours[WeaponSniperBehaviour];
        Sniper.WeaponField40 = 51; Sniper.WeaponField44 = 52; Sniper.WeaponSoundIndex = 53;
        Sniper.WeaponSniperTextureIndex = 54; Sniper.WeaponSniperMeshIndex = 55; Sniper.WeaponSniperMaterialId = 56;
    }
    {
        FKyaActorBehaviour& Rpg = Bank->SceneEnvironment.Actors[WeaponRpgActor].Behaviours[WeaponRpgBehaviour];
        if (!Rpg.WeaponRpgActorRefs.IsEmpty()) Rpg.WeaponRpgActorRefs[0] = 62;
    }
    {
        FKyaSwitch& Switch = Bank->SceneEnvironment.Actors[SwitchActor].SwitchTarget;
        Switch.Targets[0].TargetRef = 81;
        Switch.Targets[0].MessageId = 82;
        Switch.Camera.CameraIndex = 9;
        Switch.Camera.Field8 = 2.5f;
    }
    {
        FKyaActorPlacement& Bonus = Bank->SceneEnvironment.Actors[BonusActor];
        Bonus.BonusWeightB = 1.5f;
        Bonus.BonusSoundRef = 71;
        Bonus.BonusFlareMaterialId = 72;
        Bonus.BonusAnimMaterialId = 73;
    }
    const int32 BonusTurnBehaviour = Bank->SceneEnvironment.Actors[BonusTurnActor].Behaviours.IndexOfByPredicate(
        [](const FKyaActorBehaviour& Behaviour) { return Behaviour.Id == 3; });
    const int32 BonusFlockBehaviour = Bank->SceneEnvironment.Actors[BonusFlockActor].Behaviours.IndexOfByPredicate(
        [](const FKyaActorBehaviour& Behaviour) { return Behaviour.Id == 5; });
    const int32 BonusAddOnBehaviour = Bank->SceneEnvironment.Actors[BonusAddOnActor].Behaviours.IndexOfByPredicate(
        [](const FKyaActorBehaviour& Behaviour) { return Behaviour.Id == 6; });
    {
        FKyaActorBehaviour& Turn = Bank->SceneEnvironment.Actors[BonusTurnActor].Behaviours[BonusTurnBehaviour];
        Turn.BonusField14 = 1.25f; Turn.BonusField1B8 = 2.25f; Turn.BonusField1B4 = 3.25f;
        Turn.BonusMaterialId = 74; Turn.BonusField1B0 = 4.25f;
        Turn.BonusFxTailCount = 75; Turn.BonusG2DIndex = 76; Turn.BonusTurnValue = 5.25f;
        Bank->SceneEnvironment.Actors[BonusFlockActor].Behaviours[BonusFlockBehaviour].BonusFlockPathIndex = 77;
        Bank->SceneEnvironment.Actors[BonusAddOnActor].Behaviours[BonusAddOnBehaviour].BonusAddOnField18 = 6.25f;
    }
    {
        FKyaActorPlacement& Micken = Bank->SceneEnvironment.Actors[MickenActor];
        Micken.MickenWeightB = 0.5f;
        Micken.MickenLifeMax = 40.0f;
        Micken.MickenGravityScale = 2.0f;
        Micken.MickenDynamicExtField4 = 3.0f;
        Micken.MickenDynamicExtField8 = 4.0f;
        Micken.MickenWaypointIndex = 91;
        Micken.MickenActorIndex = 92;
        Micken.MickenField358 = 93;
        Micken.MickenField35C = 5.0f;
        Micken.MickenField360 = 94;
    }
    const int32 MickenEatBehaviour = Bank->SceneEnvironment.Actors[MickenEatActor].Behaviours.IndexOfByPredicate(
        [](const FKyaActorBehaviour& Behaviour) { return Behaviour.Id == 4; });
    {
        FKyaActorBehaviour& Eat = Bank->SceneEnvironment.Actors[MickenEatActor].Behaviours[MickenEatBehaviour];
        Eat.MickenEatField8 = 6.0f;
        Eat.MickenEatFieldC = 7.0f;
        Eat.MickenEatField10 = 95;
        Eat.MickenEatPathIndex = 96;
    }
    {
        FKyaActorPlacement& Wolfen = Bank->SceneEnvironment.Actors[WolfenActor];
        Wolfen.FighterInputAnalyserFlag = 1;
        Wolfen.FighterCommandMask = 0x1234;
        Wolfen.FighterField7C0 = 1.1f; Wolfen.FighterField7C4 = 1.2f;
        Wolfen.FighterField7D4 = 1.3f; Wolfen.FighterField7CC = 1.4f;
        Wolfen.FighterAnatomy20 = 2.1f; Wolfen.FighterAnatomy28 = 2.2f;
        Wolfen.FighterAnatomy24 = 2.3f; Wolfen.FighterAnatomy2C = 2.4f;
        Wolfen.FighterField810A = FVector(3.1f, 3.2f, 3.3f);
        Wolfen.FighterField810B = FVector(3.4f, 3.5f, 3.6f);
        Wolfen.FighterField800 = 1; Wolfen.FighterFightFlags = 0x55;
        Wolfen.FighterField50C = 4.1f; Wolfen.FighterField510 = 4.2f; Wolfen.FighterField514 = 4.3f;
        Wolfen.FighterField518 = 99; Wolfen.FighterField58 = 100;
        Wolfen.FighterField51C = { 10, 11, 12, 13, 14, 15, 16, 17 };
        Wolfen.FighterField53C = { 20, 21, 22, 23, 24 };
        Wolfen.FighterStaticMeshTextureIndex = 111;
        Wolfen.FighterStaticMeshMeshIndex = 112;
        Wolfen.FighterField5F0 = 113;
        Wolfen.FighterWeaponActorIndex = 114;
        Wolfen.WolfenFieldB74 = 2;
        Wolfen.WolfenVisionRange = 20.0f;
        Wolfen.WolfenVisionFov = 70.0f;
        Wolfen.WolfenVisionField38 = 3.25f;
        Wolfen.WolfenVisionField3C = 2.25f;
        Wolfen.WolfenVisionFlags = 4;
        Wolfen.WolfenStartSectorId = 5;
        Wolfen.WolfenHearingRangeSquared = 100.0f;
        Wolfen.WolfenHearingMaxHeight = 10.0f;
        Wolfen.WolfenVisionDetectionRange = 1.25f;
        Wolfen.WolfenFieldCF4 = 1.5f;
        Wolfen.WolfenWalkSpeed = 2.5f;
        Wolfen.WolfenWalkAcceleration = 3.5f;
        Wolfen.WolfenWalkRotSpeed = 4.5f;
        Wolfen.WolfenRunSpeedScale = 5.5f;
        Wolfen.WolfenDefaultRunSpeed = 6.5f;
        Wolfen.WolfenRunAcceleration = 7.5f;
        Wolfen.WolfenRotRunSpeed = 8.5f;
        Wolfen.WolfenMagicRequired = 30;
        Wolfen.WolfenFieldD30 = 31;
        Wolfen.WolfenFieldB90 = 9.5f;
        Wolfen.WolfenFieldB94 = 10.5f;
        Wolfen.WolfenFieldB98 = 11.5f;
        if (!Wolfen.FighterBlows.IsEmpty())
        {
            FKyaFighterBlow& Blow = Wolfen.FighterBlows[0];
            if (!Blow.Name.IsEmpty()) Blow.Name[0] = Blow.Name[0] == TEXT('X') ? TEXT('Y') : TEXT('X');
            Blow.Field4A = 201; Blow.Field4B = 202; Blow.Field4C = 203;
            Blow.Field8 = 1.25f; Blow.FieldC = 2.25f; Blow.Field10 = 3.25f;
            Blow.Damage = 4.25f; Blow.Field18 = 5.25f;
            Blow.Field20 = FVector(6.25f, 7.25f, 8.25f);
            Blow.Field30 = FVector(9.25f, 10.25f, 11.25f);
            Blow.Field40 = 204;
            if (!Blow.SubObjs.IsEmpty())
            {
                Blow.SubObjs[0].BoneA = 205; Blow.SubObjs[0].BoneB = 206;
                Blow.SubObjs[0].MaterialId = 207; Blow.SubObjs[0].Field14 = 208;
                Blow.SubObjs[0].Flags = 209; Blow.SubObjs[0].Field1C = 12.25f;
                Blow.SubObjs[0].Field20 = 210;
            }
            Blow.BoneRefA = 211; Blow.BoneRefB = 212;
            Blow.CanActivateRange = 13.25f;
            if (!Blow.Stages.IsEmpty())
            {
                Blow.Stages[0].AnimId = 213; Blow.Stages[0].Field4 = 14.25f;
                Blow.Stages[0].Field8 = 15.25f;
                Blow.Stages[0].Field10 = FVector(16.25f, 17.25f, 18.25f);
                Blow.Stages[0].Flags = 214;
            }
        }
        if (!Wolfen.FighterGrabs.IsEmpty())
        {
            FKyaFighterGrab& Grab = Wolfen.FighterGrabs[0];
            if (!Grab.Name.IsEmpty()) Grab.Name[0] = Grab.Name[0] == TEXT('X') ? TEXT('Y') : TEXT('X');
            Grab.Field4A = 301; Grab.Field4B = 302; Grab.Field4C = 303;
            Grab.Field8 = 21.25f; Grab.FieldC = 22.25f; Grab.Field10 = 23.25f;
            Grab.Field14 = 304; Grab.Field20 = FVector(24.25f, 25.25f, 26.25f);
            Grab.Field30 = 27.25f; Grab.Field34 = 28.25f; Grab.Field38 = 305;
            Grab.Field3C = 29.25f; Grab.Field60 = 30.25f; Grab.Field40 = 31.25f;
            Grab.Field44 = 32.25f; Grab.Field48 = 33.25f;
            Grab.Field50 = FVector(34.25f, 35.25f, 36.25f);
            Grab.Field68 = 306; Grab.Skip = 307;
            if (!Grab.Stages.IsEmpty()) Grab.Stages[0].AnimId = 308;
        }
        if (!Wolfen.FighterGrabReacts.IsEmpty())
        {
            FKyaFighterGrabReact& React = Wolfen.FighterGrabReacts[0];
            if (!React.Name.IsEmpty()) React.Name[0] = React.Name[0] == TEXT('X') ? TEXT('Y') : TEXT('X');
            React.Field4 = 311; React.Field8 = 312;
        }
        if (!Wolfen.FighterCombos.IsEmpty())
        {
            FKyaFighterCombo& Combo = Wolfen.FighterCombos[0];
            if (!Combo.Name.IsEmpty()) Combo.Name[0] = Combo.Name[0] == TEXT('X') ? TEXT('Y') : TEXT('X');
            Combo.Field4A = 321; Combo.Field4B = 322; Combo.Field8 = 37.25f;
            if (!Combo.Inputs.IsEmpty()) Combo.Inputs[0] = 323;
            Combo.Field2 = 324; Combo.Field3 = 325; Combo.FieldC = 38.25f;
            if (!Combo.Action.IsEmpty()) Combo.Action[0] = Combo.Action[0] == TEXT('X') ? TEXT('Y') : TEXT('X');
            if (!Combo.Branches.IsEmpty() && !Combo.Branches[0].IsEmpty())
                Combo.Branches[0][0] = Combo.Branches[0][0] == TEXT('X') ? TEXT('Y') : TEXT('X');
        }
        FKyaWolfenFightParam& Params = Wolfen.WolfenFightParamData;
        if (!Params.Capability0Fields.IsEmpty()) Params.Capability0Fields[0] = 401;
        if (!Params.Capability2Fields.IsEmpty()) Params.Capability2Fields[0] = 402;
        if (!Params.ConfigurationValues.IsEmpty()) Params.ConfigurationValues[0] = 41.25f;
        if (!Params.Capabilities.IsEmpty() && !Params.Capabilities[0].Items.IsEmpty())
        {
            FKyaWolfenCapabilityItem& Item = Params.Capabilities[0].Items[0];
            if (!Item.Chains.IsEmpty()) Item.Chains[0] = 403;
            Item.Field0 = 42.25f; Item.Field4 = 43.25f;
        }
        if (!Params.ComboList1.IsEmpty()) Params.ComboList1[0] = 404;
        if (!Params.ComboList2.IsEmpty()) Params.ComboList2[0] = 405;
        Params.FieldAfterLists = 406; Params.FieldB44 = 407;
        if (!Params.TuningValues.IsEmpty()) Params.TuningValues[0] = 44.25f;
        if (!Params.Knowledge.IsEmpty()) Params.Knowledge[0] = 408;
    }
    {
        FKyaActorPlacement& Fruit = Bank->SceneEnvironment.Actors[FruitActor];
        Fruit.FruitWeightB = 0.75f;
        Fruit.FruitLifeMax = 50.0f;
        Fruit.FruitGravityScale = 2.75f;
        Fruit.FruitDynamicExtField4 = 3.75f;
        Fruit.FruitDynamicExtField8 = 4.75f;
        Fruit.FruitField360 = 0x1234;
        Fruit.FruitSwitch.Camera.CameraIndex = 11;
        Fruit.FruitSwitch.Camera.Field8 = 1.75f;
    }
    const int32 FruitSquashedBehaviour = Bank->SceneEnvironment.Actors[FruitSquashedActor].Behaviours.IndexOfByPredicate(
        [](const FKyaActorBehaviour& Behaviour) { return Behaviour.Id == 4; });
    const int32 FruitBonusBehaviour = Bank->SceneEnvironment.Actors[FruitBonusActor].Behaviours.IndexOfByPredicate(
        [](const FKyaActorBehaviour& Behaviour) { return Behaviour.Id == 6; });
    const int32 FruitInventoryBehaviour = Bank->SceneEnvironment.Actors[FruitInventoryActor].Behaviours.IndexOfByPredicate(
        [](const FKyaActorBehaviour& Behaviour) { return Behaviour.Id == 5; });
    Bank->SceneEnvironment.Actors[FruitSquashedActor].Behaviours[FruitSquashedBehaviour].FruitSquashedValue = 5.75f;
    Bank->SceneEnvironment.Actors[FruitBonusActor].Behaviours[FruitBonusBehaviour].FruitBonusActorRef = 401;
    Bank->SceneEnvironment.Actors[FruitBonusActor].Behaviours[FruitBonusBehaviour].FruitBonusInitialBehaviour = 402;
    {
        FKyaActorBehaviour& Inventory = Bank->SceneEnvironment.Actors[FruitInventoryActor].Behaviours[FruitInventoryBehaviour];
        Inventory.FruitInventoryField8 = 0x123456789abcdef0ull;
        Inventory.FruitInventoryField10 = 0x0fedcba987654321ull;
        Inventory.FruitInventoryPurchaseId = 403;
        Inventory.FruitInventoryField28 = 404;
        Inventory.FruitInventoryMoneyCost = 405;
        Inventory.FruitInventoryField2C = 406;
        Inventory.FruitInventoryHasObject = false;
        Inventory.FruitInventoryInitialBehaviour = 407;
    }
    {
        FKyaActorPlacement& Commander = Bank->SceneEnvironment.Actors[CommanderActor];
        Commander.CommanderDetectAreaZoneId = 501;
        Commander.CommanderGuardAreaZoneId = 502;
        Commander.CommanderField180 = 503;
        Commander.CommanderNewPathFindingId = 504;
        Commander.CommanderActorRef = 505;
        Commander.CommanderSquadValue = 1.25f;
        Commander.CommanderSquadField4 = 506;
        Commander.CommanderSquadField8 = 507;
        if (!Commander.CommanderTeamEnemies.IsEmpty()) Commander.CommanderTeamEnemies[0] = 508;
        if (!Commander.CommanderSequence.IsEmpty())
        {
            Commander.CommanderSequence[0].Field0 = 509;
            Commander.CommanderSequence[0].Field8 = 510;
        }
        if (!Commander.CommanderCameraValues.IsEmpty())
        {
            Commander.CommanderCameraValues[0] = 2.25f;
            Commander.CommanderCameraValues[13] = 3.25f;
        }
    }
    const int32 CommanderBehaviour = Bank->SceneEnvironment.Actors[CommanderBehaviourActor].Behaviours.IndexOfByPredicate(
        [](const FKyaActorBehaviour& Behaviour) { return Behaviour.Id == 2; });
    {
        FKyaActorBehaviour& Behaviour = Bank->SceneEnvironment.Actors[CommanderBehaviourActor].Behaviours[CommanderBehaviour];
        if (Behaviour.CommanderSquadConfig.Num() >= 2)
        {
            Behaviour.CommanderSquadConfig[0] = 511;
            Behaviour.CommanderSquadConfig[1] = 512;
        }
        Behaviour.CommanderSwitch.Camera.CameraIndex = 12;
        Behaviour.CommanderSwitch.Camera.Field8 = 4.25f;
        if (!Behaviour.CommanderSwitch.Targets.IsEmpty()) Behaviour.CommanderSwitch.Targets[0].TargetRef = 513;
    }
    {
        FKyaActorPlacement& Nativ = Bank->SceneEnvironment.Actors[NativActor];
        Nativ.NativWeightB = 0.65f;
        Nativ.NativLifeMax = 60.0f;
        Nativ.NativGravityScale = 2.85f;
        Nativ.NativDynamicExtField4 = 3.85f;
        Nativ.NativDynamicExtField8 = 4.85f;
        Nativ.NativField350 = 601;
        Nativ.NativField354 = 602;
        Nativ.NativField358 = 603;
        Nativ.NativMoveSpeedNoHeld = 5.85f;
        Nativ.NativMoveSpeedHeld = 6.85f;
        Nativ.NativField370 = 604;
        Nativ.NativField368 = 605;
        Nativ.NativField364 = 606;
        Nativ.NativField378 = 607;
        Nativ.NativField37C = 608;
        Nativ.NativField380 = 609;
    }
    const int32 NativBehaviour = Bank->SceneEnvironment.Actors[NativBehaviourActor].Behaviours.IndexOfByPredicate(
        [](const FKyaActorBehaviour& Behaviour) { return Behaviour.Id == 6; });
    {
        FKyaActorBehaviour& Behaviour = Bank->SceneEnvironment.Actors[NativBehaviourActor].Behaviours[NativBehaviour];
        if (!Behaviour.NativTrajectories.IsEmpty())
        {
            Behaviour.NativTrajectories[0].Field0 = 7.85f;
            Behaviour.NativTrajectories[0].PathFollowIndex = 610;
            Behaviour.NativTrajectories[0].Field14 = 611;
            Behaviour.NativTrajectories[0].Field18 = 8.85f;
        }
    }
    {
        FKyaActorPlacement& Amortos = Bank->SceneEnvironment.Actors[AmortosActor];
        Amortos.AmortosActorRef = 701;
        Amortos.AmortosField160 = 1.15f; Amortos.AmortosField164 = 2.15f; Amortos.AmortosField168 = 3.15f;
        Amortos.AmortosField16C = 4.15f; Amortos.AmortosField170 = 5.15f; Amortos.AmortosField174 = 6.15f;
        Amortos.AmortosField178 = 7.15f; Amortos.AmortosField17C = 8.15f; Amortos.AmortosField180 = 9.15f;
        Amortos.AmortosAddOn.Field0 = 1; Amortos.AmortosAddOn.MoneyType = 2;
        Amortos.AmortosAddOn.MoneyCount = 3; Amortos.AmortosAddOn.BonusCount = 4;
        Amortos.AmortosAddOn.FieldC = 5;
        Amortos.AmortosField1C8 = 702;
        Amortos.AmortosSwitch1CC.Camera.CameraIndex = 13;
        Amortos.AmortosSwitch1CC.Camera.Field8 = 5.15f;
        Amortos.AmortosSwitch1D4.Camera.CameraIndex = 14;
        Amortos.AmortosSwitch1D4.Camera.FieldC = 6.15f;
        if (!Amortos.AmortosCheckpointActors.IsEmpty()) Amortos.AmortosCheckpointActors[0] = 703;
        if (!Amortos.AmortosActors188.IsEmpty()) Amortos.AmortosActors188[0] = 704;
        if (!Amortos.AmortosActors18C.IsEmpty()) Amortos.AmortosActors18C[0] = 705;
        if (!Amortos.AmortosSwitch1CC.Targets.IsEmpty()) Amortos.AmortosSwitch1CC.Targets[0].TargetRef = 706;
        if (!Amortos.AmortosSwitch1D4.Targets.IsEmpty()) Amortos.AmortosSwitch1D4.Targets[0].TargetRef = 707;
    }
    const int32 AmortosBehaviour = Bank->SceneEnvironment.Actors[AmortosBehaviourActor].Behaviours.IndexOfByPredicate(
        [](const FKyaActorBehaviour& Behaviour) { return Behaviour.Id == 1; });
    Bank->SceneEnvironment.Actors[AmortosBehaviourActor].Behaviours[AmortosBehaviour].CinematicLeaveBehaviourId = 708;
    {
        FKyaActorPlacement& Teleporter = Bank->SceneEnvironment.Actors[TeleporterActor];
        Teleporter.TeleporterSubsectorMaterialId = 801;
        Teleporter.TeleporterHash = int64(0x123456789abcdef0ull);
        Teleporter.TeleporterField16C = 802;
        Teleporter.TeleporterZone188 = 803; Teleporter.TeleporterZone18C = 804;
        Teleporter.TeleporterMaterialId1 = 805; Teleporter.TeleporterCinematicId = 806;
        Teleporter.TeleporterMaterialId2 = 807; Teleporter.TeleporterField1D4 = 808;
        Teleporter.TeleporterField1D8 = 809; Teleporter.TeleporterMaterialId3 = 810;
        Teleporter.TeleporterField1E0 = 811;
        Teleporter.TeleporterSwitch170.Camera.CameraIndex = 15;
        Teleporter.TeleporterSwitch170.Camera.Field8 = 7.15f;
        Teleporter.TeleporterSwitch178.Camera.CameraIndex = 16;
        Teleporter.TeleporterSwitch180.Camera.CameraIndex = 17;
        if (!Teleporter.TeleporterDestinations.IsEmpty())
        {
            Teleporter.TeleporterDestinations[0].LevelId = 812;
            Teleporter.TeleporterDestinations[0].ElevatorId = 813;
            Teleporter.TeleporterDestinations[0].Field8 = 814;
        }
        if (!Teleporter.TeleporterSwitch170.Targets.IsEmpty()) Teleporter.TeleporterSwitch170.Targets[0].TargetRef = 815;
        if (!Teleporter.TeleporterSwitch178.Targets.IsEmpty()) Teleporter.TeleporterSwitch178.Targets[0].TargetRef = 816;
        if (!Teleporter.TeleporterSwitch180.Targets.IsEmpty()) Teleporter.TeleporterSwitch180.Targets[0].TargetRef = 817;
    }
    {
        FKyaActorPlacement& Dca = Bank->SceneEnvironment.Actors[DcaActor];
        Dca.DcaField504 = 901; Dca.DcaField500 = 902; Dca.DcaField464 = 1.55f;
        Dca.DcaField508 = 903; Dca.DcaField50C = 904; Dca.DcaAimAdjustDegrees = 2.55f;
        Dca.DcaField450 = 905; Dca.DcaField454 = 906;
        if (!Dca.DcaShotActorRefs.IsEmpty()) Dca.DcaShotActorRefs[0] = 907;
    }
    const int32 DcaControlled = Bank->SceneEnvironment.Actors[DcaControlledActor].Behaviours.IndexOfByPredicate(
        [](const FKyaActorBehaviour& Behaviour) { return Behaviour.Id == 3; });
    {
        FKyaActorBehaviour& Behaviour = Bank->SceneEnvironment.Actors[DcaControlledActor].Behaviours[DcaControlled];
        Behaviour.DcaControlledField0 = 3.55f; Behaviour.DcaControlledField4 = 4.55f;
        Behaviour.DcaControlledField8 = 5.55f; Behaviour.DcaControlledFovDegrees = 6.55f;
        Behaviour.DcaControlledVector10 = FVector(7.55f, 8.55f, 9.55f);
        Behaviour.DcaControlledVector20 = FVector(10.55f, 11.55f, 12.55f);
    }
    const int32 DcaTrajectory = Bank->SceneEnvironment.Actors[DcaTrajectoryActor].Behaviours.IndexOfByPredicate(
        [](const FKyaActorBehaviour& Behaviour) { return Behaviour.Id == 5; });
    {
        FKyaActorBehaviour& Behaviour = Bank->SceneEnvironment.Actors[DcaTrajectoryActor].Behaviours[DcaTrajectory];
        Behaviour.DcaTrajectoryActorRef = 908;
        Behaviour.DcaTrajectoryField18 = 13.55f;
        Behaviour.DcaTrajectoryField8 = 909;
    }
    {
        FKyaActorPlacement& Box = Bank->SceneEnvironment.Actors[BoxClassActor];
        Box.BoxWeightB = 0.45f;
        Box.BoxLifeMax = 70.0f;
        Box.BoxGravityScale = 2.45f;
        Box.BoxDynamicExtField4 = 3.45f;
        Box.BoxDynamicExtField8 = 4.45f;
        Box.BoxField350 = 1001;
        Box.BoxField354 = 5.45f; Box.BoxField358 = 6.45f;
        Box.BoxField35C = 7.45f; Box.BoxField360 = 8.45f;
        Box.BoxField364 = 1002;
        Box.BoxField370 = 1003; Box.BoxField374 = 1004;
        Box.BoxField378 = 1005; Box.BoxField37C = 1006;
        Box.BoxSwitch.Camera.CameraIndex = 18;
        Box.BoxSwitch.Camera.Field8 = 9.45f;
        Box.BoxAddOn.Field0 = 2; Box.BoxAddOn.MoneyType = 3;
        Box.BoxAddOn.MoneyCount = 4; Box.BoxAddOn.BonusCount = 5;
        Box.BoxAddOn.FieldC = 6;
        if (!Box.BoxActorRefs.IsEmpty()) Box.BoxActorRefs[0] = 1007;
        if (!Box.BoxSwitch.Targets.IsEmpty()) Box.BoxSwitch.Targets[0].TargetRef = 1008;
    }
    {
        FKyaActorPlacement& Money = Bank->SceneEnvironment.Actors[MoneyActor];
        Money.MoneyWeightB = 0.35f;
        Money.MoneyValue = 1101;
        Money.MoneySoundRef = 1102;
        Money.MoneyMaterialId = 1103;
    }
    {
        FKyaActorPlacement& Fog = Bank->SceneEnvironment.Actors[FogActor];
        if (!Fog.FogHierarchies.IsEmpty())
        {
            Fog.FogHierarchies[0].Hash = int64(0x0fedcba987654321ull);
            Fog.FogHierarchies[0].Field20 = 7.05f;
            Fog.FogHierarchies[0].Field24 = 8.05f;
        }
    }
    const int32 FogBehaviour = Bank->SceneEnvironment.Actors[FogBehaviourActor].Behaviours.IndexOfByPredicate(
        [](const FKyaActorBehaviour& Behaviour) { return Behaviour.Id == 2; });
    {
        FKyaActorPlacement& Shoot = Bank->SceneEnvironment.Actors[ShootActor];
        Shoot.ShootWeightB = 0.25f;
        Shoot.ShootLifeMax = 80.0f;
        Shoot.ShootGravityScale = 2.35f;
        Shoot.ShootDynamicExtField4 = 3.35f;
        Shoot.ShootDynamicExtField8 = 4.35f;
        Shoot.ShootField350 = 1201; Shoot.ShootField354 = 1202;
        Shoot.ShootTextureIndex = 1203; Shoot.ShootMeshIndex = 1204;
        Shoot.ShootField3C0 = 5.35f; Shoot.ShootField3C4 = 6.35f;
        Shoot.ShootField3C8 = 7.35f; Shoot.ShootField3CC = 8.35f;
        Shoot.ShootField3D0 = 1205;
        Shoot.ShootField3D4 = 9.35f; Shoot.ShootField3D8 = 10.35f;
        Shoot.ShootField3DC = 11.35f; Shoot.ShootField3E0 = 12.35f;
        Shoot.ShootField3E8 = 1206; Shoot.ShootField3E4 = 1207;
        Shoot.ShootField3EC = 1208; Shoot.ShootField3F0 = 1209;
        Shoot.ShootAddOn.Field0 = 1; Shoot.ShootAddOn.MoneyType = 2;
        Shoot.ShootAddOn.MoneyCount = 3; Shoot.ShootAddOn.BonusCount = 4;
        Shoot.ShootAddOn.FieldC = 5;
    }
    const int32 ShootFireBehaviour = Bank->SceneEnvironment.Actors[ShootFireActor].Behaviours.IndexOfByPredicate(
        [](const FKyaActorBehaviour& Behaviour) { return Behaviour.Id == 3; });
    {
        FKyaActorBehaviour& Behaviour = Bank->SceneEnvironment.Actors[ShootFireActor].Behaviours[ShootFireBehaviour];
        Behaviour.ShootFireField2B0 = 1210;
        Behaviour.ShootFireField8 = 13.35f;
        Behaviour.ShootFireField2B4 = 14.35f;
        if (!Behaviour.ShootFireActorRefs.IsEmpty()) Behaviour.ShootFireActorRefs[0] = 1211;
    }
    {
        FKyaActorPlacement& Ambre = Bank->SceneEnvironment.Actors[AmbreActor];
        Ambre.AmbreMaterialId = 1301;
        Ambre.AmbreEffectType = 1302;
        Ambre.AmbreTextureIndex = 1303;
        Ambre.AmbreMeshIndex = 1304;
        Ambre.AmbreField1D0 = 1305;
        Ambre.AmbreSwitch.Camera.CameraIndex = 19;
        Ambre.AmbreSwitch.Camera.Field8 = 15.35f;
    }
    {
        FKyaActorPlacement& Stiller = Bank->SceneEnvironment.Actors[StillerActor];
        Stiller.StillerVisionRange = 1601.0f;
        Stiller.StillerVisionHalfAngle = 1602.0f;
        Stiller.StillerVisionField38 = 1.65f;
        Stiller.StillerVisionField3C = 2.65f;
        Stiller.StillerVisionFlags = 1603;
        Stiller.StillerField1D0 = 1604; Stiller.StillerField1D4 = 1605;
        Stiller.StillerField1D8 = 1606; Stiller.StillerField1DC = 1607;
        Stiller.StillerPathIndex = 1608;
        Stiller.StillerField1E0 = 3.65f; Stiller.StillerLifeMax = 4.65f;
        Stiller.StillerField1EC = 5.65f; Stiller.StillerField1F0 = 6.65f;
        Stiller.StillerField1F4 = 7.65f; Stiller.StillerField1F8 = 1609;
        Stiller.StillerField200 = 1610;
        Stiller.StillerField214 = 1611;
        Stiller.StillerSwitch208.Camera.CameraIndex = 20;
        Stiller.StillerSwitch208.Camera.Field8 = 8.65f;
        Stiller.StillerSwitch210.Camera.CameraIndex = 21;
        Stiller.StillerAddOn.Field0 = 1; Stiller.StillerAddOn.MoneyType = 2;
        Stiller.StillerAddOn.MoneyCount = 3; Stiller.StillerAddOn.BonusCount = 4;
        Stiller.StillerAddOn.FieldC = 5;
        if (!Stiller.StillerActorRefs.IsEmpty()) Stiller.StillerActorRefs[0] = 1612;
        if (!Stiller.StillerSwitch208.Targets.IsEmpty()) Stiller.StillerSwitch208.Targets[0].TargetRef = 1613;
    }
    {
        FKyaActorPlacement& Rune = Bank->SceneEnvironment.Actors[RuneActor];
        Rune.RuneCinematicId = 1701;
        Rune.RuneTriggerDistance = 1.75f;
        Rune.RuneScenVarId = 1702;
        Rune.RuneFxId = 1703;
        Rune.RuneMaterialIdA = 1704;
        Rune.RuneMaterialIdB = 1705;
    }
    {
        FKyaActorPlacement& Fountain = Bank->SceneEnvironment.Actors[FountainActor];
        Fountain.BonusFountainAddOn.Field0 = 1;
        Fountain.BonusFountainAddOn.MoneyType = 2;
        Fountain.BonusFountainAddOn.MoneyCount = 3;
        Fountain.BonusFountainAddOn.BonusCount = 4;
        Fountain.BonusFountainAddOn.FieldC = 5;
    }
    {
        FKyaActorPlacement& Rope = Bank->SceneEnvironment.Actors[RopeActor];
        Rope.RopePathIndex = 1801;
        Rope.RopeScale = 2.75f;
        Rope.RopeField17C = 3.75f;
        Rope.RopeMaterialId = 1802;
        Rope.RopeColor = 0x11223344;
        Rope.RopeSoundRef = 1803;
        Rope.RopeField194 = 1804;
        Rope.RopeActorRefA = 1805;
        Rope.RopeActorRefB = 1806;
        Rope.RopeFlags = 0x2345;
        Rope.RopeSwitch.Camera.CameraIndex = 22;
        Rope.RopeSwitch.Camera.Field8 = 4.75f;
        if (!Rope.RopeSwitch.Targets.IsEmpty()) Rope.RopeSwitch.Targets[0].TargetRef = 1807;
    }
    {
        FKyaActorPlacement& Gravity = Bank->SceneEnvironment.Actors[GravityActor];
        Gravity.GravityWeightB = 0.55f;
        Gravity.GravityLifeMax = 90.0f;
        Gravity.GravityScale = 2.95f;
        Gravity.GravityDynamicExtField4 = 3.95f;
        Gravity.GravityDynamicExtField8 = 4.95f;
    }
    const int32 GravityFallBehaviour = Bank->SceneEnvironment.Actors[GravityActor].Behaviours.IndexOfByPredicate(
        [](const FKyaActorBehaviour& Behaviour) { return Behaviour.Id == 3; });
    const int32 GravityBellBehaviour = Bank->SceneEnvironment.Actors[GravityActor].Behaviours.IndexOfByPredicate(
        [](const FKyaActorBehaviour& Behaviour) { return Behaviour.Id == 4; });
    {
        FKyaActorBehaviour& Fall = Bank->SceneEnvironment.Actors[GravityActor].Behaviours[GravityFallBehaviour];
        Fall.GravityFallSoundRef = 1901;
        FKyaActorBehaviour& Bell = Bank->SceneEnvironment.Actors[GravityActor].Behaviours[GravityBellBehaviour];
        Bell.GravityBellSoundRef = 1902;
        Bell.GravityBellFieldC = 5.95f;
        Bell.GravityBellSwitch.Camera.CameraIndex = 23;
        Bell.GravityBellSwitch.Camera.Field8 = 6.95f;
        if (!Bell.GravityBellSwitch.Targets.IsEmpty()) Bell.GravityBellSwitch.Targets[0].TargetRef = 1903;
    }
    {
        FKyaActorPlacement& Command = Bank->SceneEnvironment.Actors[NativCmdActor];
        Command.NativCmdField164 = 2001;
        if (!Command.NativCmdActorRefs.IsEmpty()) Command.NativCmdActorRefs[0] = 2002;
    }
    {
        FKyaActorPlacement& Companion = Bank->SceneEnvironment.Actors[CompanionActor];
        Companion.CompanionWeightB = 0.45f;
        Companion.CompanionLifeMax = 100.0f;
        Companion.CompanionGravityScale = 3.05f;
        Companion.CompanionDynamicExtField4 = 4.05f;
        Companion.CompanionDynamicExtField8 = 5.05f;
        Companion.CompanionField358 = 2101;
        Companion.CompanionMaterial35C = 2102;
        Companion.CompanionField360 = 2103;
    }
    {
        FKyaActorPlacement& Bridge = Bank->SceneEnvironment.Actors[BridgeActor];
        Bridge.BridgeWaypointA = 2201;
        Bridge.BridgeWaypointB = 2202;
        Bridge.BridgeField330 = 1.05f;
        Bridge.BridgeField334 = 2.05f;
        Bridge.BridgeField33C = 3.05f;
        Bridge.BridgeField340 = 4.05f;
        Bridge.BridgeField338 = 5.05f;
        Bridge.BridgeField344 = 2203;
        Bridge.BridgeConfigFlags = 0x1234;
        Bridge.BridgeField348 = 2204;
        Bridge.BridgeStepCollisionScale = 6.05f;
        if (!Bridge.BridgeActorRefs.IsEmpty()) Bridge.BridgeActorRefs[0] = 2205;
    }
    const int32 BridgeBasicBehaviour = Bank->SceneEnvironment.Actors[BridgeActor].Behaviours.IndexOfByPredicate(
        [](const FKyaActorBehaviour& Behaviour) { return Behaviour.Id == 2; });
    {
        FKyaActorBehaviour& Behaviour = Bank->SceneEnvironment.Actors[BridgeActor].Behaviours[BridgeBasicBehaviour];
        Behaviour.BridgeBasicDirection = FVector(7.05f, 8.05f, 9.05f);
        Behaviour.BridgeBasicField40 = 10.05f;
        Behaviour.BridgeBasicField44 = 11.05f;
        Behaviour.BridgeBasicField48 = 12.05f;
        Behaviour.BridgeBasicField4C = 13.05f;
        Behaviour.BridgeBasicField50 = 14.05f;
    }
    {
        FKyaActorPlacement& Aton = Bank->SceneEnvironment.Actors[AtonActor];
        Aton.AtonWeightB = 0.65f;
        Aton.AtonLifeMax = 110.0f;
        Aton.AtonGravityScale = 3.15f;
        Aton.AtonDynamicExtField4 = 4.15f;
        Aton.AtonDynamicExtField8 = 5.15f;
        Aton.AtonField354 = 2301;
        Aton.AtonField358 = 2302;
        Aton.AtonField35C = 2303;
        Aton.AtonField360 = 2304;
        Aton.AtonEffectType = 2305;
        Aton.AtonTextureIndex = 2306;
        Aton.AtonMeshIndex = 2307;
        Aton.AtonTrailMaterialId = 2308;
        Aton.AtonPathDefaultDelay = 6.15f;
        Aton.AtonAddOnField18 = 2309;
        Aton.AtonAddOnField24 = 7.15f;
        if (!Aton.AtonPathIndices.IsEmpty()) Aton.AtonPathIndices[0] = 2310;
        if (!Aton.AtonOtherSubObjs.IsEmpty())
        {
            Aton.AtonOtherSubObjs[0].Field0 = 2311;
            if (!Aton.AtonOtherSubObjs[0].CinematicIds.IsEmpty())
                Aton.AtonOtherSubObjs[0].CinematicIds[0] = 2312;
        }
    }
    const int32 AtonBehaviour = Bank->SceneEnvironment.Actors[AtonActor].Behaviours.IndexOfByPredicate(
        [](const FKyaActorBehaviour& Behaviour) { return Behaviour.Id == 1; });
    Bank->SceneEnvironment.Actors[AtonActor].Behaviours[AtonBehaviour].CinematicLeaveBehaviourId = 2313;
    {
        FKyaActorPlacement& Boomy = Bank->SceneEnvironment.Actors[BoomyActor];
        Boomy.BoomyWeight = 0.75f;
        Boomy.BoomyVisionRange = 2401.0f;
        Boomy.BoomyVisionAngle = 2402.0f;
        Boomy.BoomyVisionField38 = 1.25f;
        Boomy.BoomyVisionField3C = 2.25f;
        Boomy.BoomyVisionFlags = 2403;
        Boomy.BoomySound2B8 = 2404;
        Boomy.BoomySound2BC = 2405;
        Boomy.BoomyField744 = 3.25f;
        Boomy.BoomyTailWidthAxis = 2406;
        Boomy.BoomyField740 = 4.25f;
        Boomy.BoomyTailSegments = 2407;
        Boomy.BoomyParticle3D0 = 2408;
        Boomy.BoomyParticle3D4 = 2409;
        Boomy.BoomyParticle3DC = 2410;
        Boomy.BoomyField3CC = 2411;
        Boomy.BoomyTextureIndex = 2412;
        Boomy.BoomyMeshIndex = 2413;
        FKyaActorCamera& Camera = Boomy.BoomyCamera;
        Camera.Fov = 5.25f;
        Camera.Position = FVector(6.25f, 7.25f, 8.25f);
        Camera.LookAt = FVector(9.25f, 10.25f, 11.25f);
        Camera.Up = FVector(12.25f, 13.25f, 14.25f);
        Camera.SectorId = 2414;
        Camera.Field08 = 2415;
        Camera.Flags = 2416;
        Camera.TargetOffset = FVector(15.25f, 16.25f, 17.25f);
        Camera.ConfigFlags = 2417;
        Camera.ConfigActorRefA = 2418;
        Camera.ConfigActorRefB = 2419;
        Camera.ConfigAngle = 18.25f;
        Camera.PitchAngles = FVector(19.25f, 20.25f, 21.25f);
        Camera.OtherAngles = FVector(22.25f, 23.25f, 24.25f);
        Camera.Distances = FVector(25.25f, 26.25f, 27.25f);
        if (!Camera.Conditions.IsEmpty())
        {
            Camera.Conditions[0].Combiner = 2420;
            Camera.Conditions[0].ActorRef = 2421;
            Camera.Conditions[0].Type = 2422;
            Camera.Conditions[0].Value = 28.25f;
        }
        if (!Boomy.BoomyTypes.IsEmpty())
        {
            Boomy.BoomyTypes[0].Field4 = 29.25f;
            Boomy.BoomyTypes[0].VisionRange = 30.25f;
            Boomy.BoomyTypes[0].HitDamage = 2423;
            Boomy.BoomyTypes[0].FxColorA = 2424;
        }
    }
    {
        FKyaActorPlacement& Kim = Bank->SceneEnvironment.Actors[KimActor];
        Kim.FighterWeightB = 0.85f;
        Kim.FighterLifeMax = 120.0f;
        Kim.FighterGravityScale = 2.05f;
        Kim.FighterDynamicExtField4 = 3.05f;
        Kim.FighterDynamicExtField8 = 4.05f;
        Kim.FighterInputAnalyserFlag = 2600;
        Kim.FighterCommandMask = 2601;
        Kim.FighterField7C0 = 9.45f;
        if (!Kim.FighterBlows.IsEmpty()) Kim.FighterBlows[0].Field4A = 2602;
        if (!Kim.FighterGrabs.IsEmpty()) Kim.FighterGrabs[0].Field4A = 2603;
        if (!Kim.FighterCombos.IsEmpty()) Kim.FighterCombos[0].Field4A = 2604;
        if (!Kim.FighterGrabReacts.IsEmpty()) Kim.FighterGrabReacts[0].Field4 = 2605;
        for (int32 Index = 0; Index < Kim.KimBoneKeys.Num(); ++Index) Kim.KimBoneKeys[Index] = 2500 + Index;
        if (!Kim.KimClimbZones.IsEmpty()) Kim.KimClimbZones[0] = 2510;
        if (!Kim.KimTutorialZones.IsEmpty())
        {
            FKyaHeroTutorialZone& Zone = Kim.KimTutorialZones[0];
            Zone.ZoneRef = 2511;
            Zone.MessageKey = int64(0x0123456789abcdefull);
            Zone.Flags = 2512;
            Zone.Action = 2513;
            Zone.Duration = 1.45f;
            if (!Zone.Targets.IsEmpty())
            {
                Zone.Targets[0].TargetRef = 2514;
                Zone.Targets[0].CutsceneId = 2515;
                Zone.Targets[0].Field18 = 2516;
            }
            Zone.Camera.CameraIndex = 24;
            Zone.Camera.Field8 = 2.45f;
        }
        if (!Kim.KimProjectileZones.IsEmpty())
        {
            FKyaHeroProjectileZone& Zone = Kim.KimProjectileZones[0];
            Zone.ZoneRef = 2517;
            Zone.ProjectileType = 2518;
            Zone.Damage = 3.45f;
            Zone.WaypointIndex = 2519;
            Zone.Force = 4.45f;
            Zone.SampleCount = 2520;
            Zone.SectorId = 2521;
        }
        if (!Kim.KimBoomyZonePairs.IsEmpty())
        {
            Kim.KimBoomyZonePairs[0].ZoneRef = 2522;
            Kim.KimBoomyZonePairs[0].Field4 = 2523;
        }
        if (!Kim.KimZoneStream.IsEmpty()) Kim.KimZoneStream[0] = 2524;
        Kim.KimMaterialId = 2525;
        Kim.KimFxType = 2526;
        Kim.KimJokeWarnRadius = 5.45f;
        if (!Kim.KimCameras.IsEmpty())
        {
            FKyaActorCamera& Camera = Kim.KimCameras[0];
            Camera.Fov = 6.45f;
            Camera.SectorId = 2527;
            Camera.ConfigAngle = 7.45f;
        }
        Kim.KimField15D4 = 2528;
        Kim.KimFxInitType = 2529;
        for (int32 Index = 0; Index < Kim.KimFxHandleTypes.Num(); ++Index) Kim.KimFxHandleTypes[Index] = 2530 + Index;
    }
    {
        FKyaActorPlacement& Companion = Bank->SceneEnvironment.Actors[CompanionAlertActor];
        const int32 CompanionBehaviourIndex = Companion.Behaviours.IndexOfByPredicate(
            [](const FKyaActorBehaviour& Behaviour) { return Behaviour.Id == 3; });
        FKyaActorBehaviour& Behaviour = Companion.Behaviours[CompanionBehaviourIndex];
        if (!Behaviour.CompanionAlerts.IsEmpty())
        {
            FKyaCompanionAlert& Alert = Behaviour.CompanionAlerts[0];
            Alert.Flags = 2700;
            Alert.Field08 = 2701;
            Alert.Field10 = 2702;
            Alert.Field14 = 2703;
            Alert.Field18 = 1.55f;
            Alert.ActorRef = 2704;
            Alert.Field20 = FVector(2.55f, 3.55f, 4.55f);
            Alert.CameraRef = 2705;
            Alert.LevelStringKey = int64(0x0fedcba987654321ull);
            Alert.Field50 = 5.55f;
            Alert.Field58 = 2706;
            Alert.Field5C = 6.55f;
            if (!Alert.Field34.IsEmpty()) Alert.Field34[0] = 2707;
            if (!Alert.TargetSwitch.Targets.IsEmpty()) Alert.TargetSwitch.Targets[0].TargetRef = 2708;
            if (!Alert.Field70.IsEmpty() && !Alert.Field70[0].Operations.IsEmpty())
                Alert.Field70[0].Operations[0].Value = 2709;
        }
        if (Behaviour.CompanionAlerts.Num() > 1 && !Behaviour.CompanionAlerts[1].Conditions.IsEmpty())
        {
            Behaviour.CompanionAlerts[1].Conditions[0].Combiner = 2710;
            Behaviour.CompanionAlerts[1].Conditions[0].Variable = 2711;
            Behaviour.CompanionAlerts[1].Conditions[0].Comparison = 2712;
            Behaviour.CompanionAlerts[1].Conditions[0].Value = 2713;
        }
    }
    {
        FKyaActorPlacement& Actor = Bank->SceneEnvironment.Actors[PlatformStandActor];
        const int32 BehaviourIndex = Actor.Behaviours.IndexOfByPredicate(
            [](const FKyaActorBehaviour& Behaviour) { return Behaviour.Id == 7; });
        Actor.Behaviours[BehaviourIndex].PlatformBehaviour.SoundRef = 2800;
        Actor.Behaviours[BehaviourIndex].PlatformBehaviour.Field0C = 2801;
    }
    {
        FKyaActorPlacement& Actor = Bank->SceneEnvironment.Actors[PlatformSelectorActor];
        const int32 BehaviourIndex = Actor.Behaviours.IndexOfByPredicate(
            [](const FKyaActorBehaviour& Behaviour) { return Behaviour.Id == 9; });
        FKyaPlatformBehaviour& Values = Actor.Behaviours[BehaviourIndex].PlatformBehaviour;
        Values.Field18 = 2802;
        Values.MaterialId = 2803;
        Values.Field8 = 1.85f;
        Values.FieldC = 2.85f;
        Values.Field10 = 3.85f;
        Values.SoundRef = 2804;
    }
    {
        FKyaActorPlacement& Actor = Bank->SceneEnvironment.Actors[PlatformTrajectoryActor];
        const int32 BehaviourIndex = Actor.Behaviours.IndexOfByPredicate(
            [](const FKyaActorBehaviour& Behaviour) { return Behaviour.Id == 2; });
        FKyaPlatformBehaviour& Values = Actor.Behaviours[BehaviourIndex].PlatformBehaviour;
        Values.PathIndex = 2805;
        Values.Timing = 2806;
        Values.PathField4 = 4.85f;
        Values.PathTotalTime = 5.85f;
        Values.SegmentStartTime = 6.85f;
        Values.LoopDelay = 7.85f;
    }
    {
        FKyaActorPlacement& Actor = Bank->SceneEnvironment.Actors[PlatformSlabActor];
        const int32 PlatformExtraBehaviourIndex = Actor.Behaviours.IndexOfByPredicate(
            [](const FKyaActorBehaviour& Behaviour) { return Behaviour.Id == 3; });
        FKyaPlatformBehaviour& Values = Actor.Behaviours[PlatformExtraBehaviourIndex].PlatformBehaviour;
        Values.SlabField10 = 1.15f; Values.SlabField14 = 2.15f; Values.SlabField18 = 3.15f;
        Values.SlabField1C = 4.15f; Values.SlabField20 = 3100;
        Values.SlabSwitch.Camera.CameraIndex = 27; Values.SlabSwitch.Camera.Field8 = 5.15f;
        if (!Values.SlabSwitch.Targets.IsEmpty()) Values.SlabSwitch.Targets[0].TargetRef = 3101;
    }
    {
        FKyaActorPlacement& Actor = Bank->SceneEnvironment.Actors[PlatformMasterActor];
        const int32 PlatformExtraBehaviourIndex = Actor.Behaviours.IndexOfByPredicate(
            [](const FKyaActorBehaviour& Behaviour) { return Behaviour.Id == 5; });
        FKyaPlatformBehaviour& Values = Actor.Behaviours[PlatformExtraBehaviourIndex].PlatformBehaviour;
        Values.MasterPathIndex = 3102; Values.MasterTiming = 3103; Values.MasterStreamActor = 3104;
        Values.MasterField40 = 6.15f; Values.MasterField44 = 7.15f;
        Values.AnalogCamera.CameraIndex = 28; Values.AnalogCamera.Field8 = 8.15f;
        if (!Values.AnalogTargets.IsEmpty()) { Values.AnalogTargets[0].ActorRef = 3105; Values.AnalogTargets[0].Field20 = 9.15f; }
    }
    {
        FKyaActorPlacement& Actor = Bank->SceneEnvironment.Actors[PlatformSelectorMasterActor];
        const int32 PlatformExtraBehaviourIndex = Actor.Behaviours.IndexOfByPredicate(
            [](const FKyaActorBehaviour& Behaviour) { return Behaviour.Id == 8; });
        FKyaPlatformBehaviour& Values = Actor.Behaviours[PlatformExtraBehaviourIndex].PlatformBehaviour;
        Values.Field18 = 3106; Values.MaterialId = 3107; Values.Field8 = 10.15f;
        Values.FieldC = 11.15f; Values.Field10 = 12.15f; Values.SoundRef = 3108;
        Values.SelectorCamera.CameraIndex = 29; Values.SelectorCamera.Field8 = 13.15f;
        if (!Values.SelectorActorRefs.IsEmpty()) Values.SelectorActorRefs[0] = 3109;
        if (!Values.SelectorSwitchEx.IsEmpty()) Values.SelectorSwitchEx[0].ActorRef = 3110;
    }
    {
        FKyaActorPlacement& Actor = Bank->SceneEnvironment.Actors[ProjectileStandActor];
        const int32 ProjectileBehaviourIndex = Actor.Behaviours.IndexOfByPredicate(
            [](const FKyaActorBehaviour& Behaviour) { return Behaviour.Id == 3; });
        FKyaProjectileBehaviour& Values = Actor.Behaviours[ProjectileBehaviourIndex].ProjectileBehaviour;
        Values.BehaviourId = 2900;
        Values.Field0C = 2901;
        Values.MaterialId = 2902;
    }
    {
        FKyaActorPlacement& Actor = Bank->SceneEnvironment.Actors[ProjectileStraightActor];
        const int32 ProjectileBehaviourIndex = Actor.Behaviours.IndexOfByPredicate(
            [](const FKyaActorBehaviour& Behaviour) { return Behaviour.Id == 4; });
        FKyaProjectileBehaviour& Values = Actor.Behaviours[ProjectileBehaviourIndex].ProjectileBehaviour;
        Values.BehaviourId = 2903;
        Values.Field0C = 2904;
        Values.Speed = 1.95f;
        Values.Acceleration = 2.95f;
        Values.Field18 = 3.95f;
    }
    {
        FKyaActorPlacement& Actor = Bank->SceneEnvironment.Actors[SwitchLeverActor];
        const int32 SwitchBehaviourIndex = Actor.Behaviours.IndexOfByPredicate(
            [](const FKyaActorBehaviour& Behaviour) { return Behaviour.Id == 2; });
        Actor.Behaviours[SwitchBehaviourIndex].SwitchBehaviour.LeverRefA = 2905;
        Actor.Behaviours[SwitchBehaviourIndex].SwitchBehaviour.LeverRefB = 2906;
    }
    {
        FKyaActorPlacement& Actor = Bank->SceneEnvironment.Actors[SwitchMagicActor];
        const int32 SwitchBehaviourIndex = Actor.Behaviours.IndexOfByPredicate(
            [](const FKyaActorBehaviour& Behaviour) { return Behaviour.Id == 3; });
        FKyaSwitchBehaviour& Values = Actor.Behaviours[SwitchBehaviourIndex].SwitchBehaviour;
        Values.MagicField08 = 4.95f;
        Values.MagicField10 = 5.95f;
        Values.MagicFxDigits = 2907;
        Values.MagicField14 = 6.95f;
        Values.MagicField4C = 2908;
        Values.MagicField18 = 2909;
        Values.MagicField28 = 2910;
        Values.MagicField24 = 2911;
        Values.MagicField30 = 2912;
        Values.OpenTextureId = 2913;
        Values.OpenMeshId = 2914;
        Values.MagicCamera.CameraIndex = 25;
        Values.MagicCamera.Field8 = 7.95f;
    }
    {
        auto EditWolfen = [&Bank](int32 ActorIndex, int32 BehaviourId, auto&& Fn) {
            FKyaActorPlacement& Actor = Bank->SceneEnvironment.Actors[ActorIndex];
            const int32 BehaviourIndex = Actor.Behaviours.IndexOfByPredicate(
                [BehaviourId](const FKyaActorBehaviour& Behaviour) { return Behaviour.Id == BehaviourId; });
            Fn(Actor.Behaviours[BehaviourIndex].WolfenBehaviour);
        };
        EditWolfen(WolfenTrackActor, 15, [](FKyaWolfenBehaviour& V) { V.Flags = 3000; V.TrackField = 1.05f; });
        EditWolfen(WolfenTrackWeaponActor, 17, [](FKyaWolfenBehaviour& V) {
            V.Flags = 3001; V.WeaponField90 = 2.05f; V.WeaponField94 = 3002; V.WeaponField98 = 3003;
            V.WeaponFieldF0 = 3.05f; V.WeaponTargetField = 4.05f;
            if (!V.WeaponSubObjs.IsEmpty()) { V.WeaponSubObjs[0].Field0 = 3004; V.WeaponSubObjs[0].AField4 = 5.05f; }
        });
        EditWolfen(WolfenTrackWeaponStandActor, 18, [](FKyaWolfenBehaviour& V) {
            V.Flags = 3005; V.WeaponField90 = 6.05f; V.WeaponField94 = 3006; V.WeaponField98 = 3007;
            if (!V.WeaponSubObjs.IsEmpty()) V.WeaponSubObjs[0].Field10 = 3008;
        });
        EditWolfen(WolfenTrackWeaponSnipeActor, 19, [](FKyaWolfenBehaviour& V) { V.Flags = 3009; V.WeaponFieldF0 = 7.05f; });
        EditWolfen(WolfenDcaActor, 23, [](FKyaWolfenBehaviour& V) { V.Flags = 3010; V.DcaActorRef = 3011; V.DcaShootCooldown = 8.05f; });
        EditWolfen(WolfenAvoidActor, 24, [](FKyaWolfenBehaviour& V) { V.Flags = 3012; V.AvoidField80 = 9.05f; V.AvoidField84 = 10.05f; });
        EditWolfen(WolfenWatchDogActor, 8, [](FKyaWolfenBehaviour& V) {
            V.Flags = 3020; V.Field80 = 3021; V.TrackBehaviourId = 3022;
            V.OnOffCamera.CameraIndex = 26; V.OnOffCamera.Field8 = 11.05f;
            if (!V.OnOffTargets.IsEmpty()) V.OnOffTargets[0].TargetRef = 3023;
            if (!V.SwitchRecords.IsEmpty()) V.SwitchRecords[0].BehaviourId = 3024;
        });
        EditWolfen(WolfenGuardActor, 10, [](FKyaWolfenBehaviour& V) {
            V.Flags = 3100; V.GuardPathIndex = 3101; V.GuardField98 = 19.05f;
            V.GuardTrackBehaviourId = 3102; V.GuardField90 = 3103;
            V.OnOffCamera.CameraIndex = 30; V.OnOffCamera.Field8 = 20.05f;
            if (!V.OnOffTargets.IsEmpty()) V.OnOffTargets[0].TargetRef = 3104;
        });
        EditWolfen(WolfenEscapeActor, 13, [](FKyaWolfenBehaviour& V) {
            V.Flags = 3105; V.EscapeField88 = 3106; V.EscapeField9C = 21.05f; V.EscapeField90 = 22.05f;
            V.EscapeField8C = 23.05f; V.EscapeField94 = 24.05f; V.EscapeField98 = 25.05f; V.EscapeFieldA0 = 3107;
            if (!V.EscapePathIndices.IsEmpty()) V.EscapePathIndices[0] = 3108;
        });
        EditWolfen(WolfenExorcismActor, 14, [](FKyaWolfenBehaviour& V) {
            V.ExorcismDigitMaterialId = 3201; V.ExorcismField24 = 3202;
            V.ExorcismTextureA = 3203; V.ExorcismMeshA = 3204; V.ExorcismTextureB = 3205;
            V.ExorcismMeshB = 3206; V.ExorcismField18 = 3207;
            for (int32 Index = 0; Index < V.ExorcismWords.Num(); ++Index) V.ExorcismWords[Index] = 3210 + Index;
        });
        {
            FKyaActorPlacement& Actor = Bank->SceneEnvironment.Actors[CatchActor];
            const int32 NewTypedBehaviourIndex = Actor.Behaviours.IndexOfByPredicate(
                [](const FKyaActorBehaviour& Behaviour) { return Behaviour.bHasTypedCatchByTrapBehaviour; });
            FKyaActorBehaviour& Behaviour = Actor.Behaviours[NewTypedBehaviourIndex];
            if (!Behaviour.CatchOperations.IsEmpty() && !Behaviour.CatchOperations[0].Operations.IsEmpty())
            {
                Behaviour.CatchOperations[0].Operations[0].Variable = 3300;
                Behaviour.CatchOperations[0].Operations[0].Type = 3301;
                Behaviour.CatchOperations[0].Operations[0].Value = 3302;
            }
        }
        {
            FKyaActorPlacement& Actor = Bank->SceneEnvironment.Actors[HeroUnknownActor];
            const int32 NewTypedBehaviourIndex = Actor.Behaviours.IndexOfByPredicate(
                [](const FKyaActorBehaviour& Behaviour) { return Behaviour.bHasTypedHeroUnknownBehaviour; });
            FKyaHeroUnknownBehaviour& Values = Actor.Behaviours[NewTypedBehaviourIndex].HeroUnknownBehaviour;
            Values.FxTypeA = 3303; Values.TextureA = 3304; Values.MeshA = 3305;
            Values.FxTypeB = 3306; Values.FxTypeC = 3307; Values.TextureB = 3308; Values.MeshB = 3309;
        }
        {
            FKyaActorPlacement& Actor = Bank->SceneEnvironment.Actors[InventoryActor];
            const int32 NewTypedBehaviourIndex = Actor.Behaviours.IndexOfByPredicate(
                [](const FKyaActorBehaviour& Behaviour) { return Behaviour.bHasTypedInventoryBehaviour; });
            FKyaInventoryInfo& Values = Actor.Behaviours[NewTypedBehaviourIndex].InventoryInfo;
            Values.Field0 = 1; Values.PurchaseId = 3310; Values.Field28 = 3311;
            Values.MoneyCost = 3312; Values.Field2C = 3313; Values.InitialBehaviourId = 3314;
        }
        EditWolfen(WolfenSnipeActor, 21, [](FKyaWolfenBehaviour& V) {
            V.Flags = 3025; V.SnipeField90 = 3026; V.SnipeField94 = 12.05f; V.SnipeField98 = 13.05f;
            V.SnipeField9C = 14.05f; V.SnipeFieldA0 = 15.05f; V.SpotTrackBehaviourId = 3027;
            V.SnipeTrackBehaviourId = 3028; V.ShadowMaterialId = 3029; V.SnipeFieldAC = 3030;
            V.SnipeFieldB0 = 16.05f; V.SnipeFieldB4 = 17.05f; V.NbSpot = 3031; V.NbShadow = 3032;
            V.RotationSpeed = 18.05f; V.SnipeFieldB8 = 3033;
        });
        EditWolfen(WolfenLostActor, 22, [](FKyaWolfenBehaviour& V) {
            V.Flags = 3013;
            if (!V.SwitchRecords.IsEmpty())
            {
                V.SwitchRecords[0].BehaviourId = 3014;
                if (V.SwitchRecords[0].Category == 0) V.SwitchRecords[0].MessageType = 3015;
                else if (V.SwitchRecords[0].Category == 3) V.SwitchRecords[0].SeeActorRef = 3016;
                else { V.SwitchRecords[0].ActorRef = 3017; V.SwitchRecords[0].ZoneRef = 3018; V.SwitchRecords[0].Flags = 3019; }
            }
        });
    }
    const int32 AnimatedActor = Bank->SceneEnvironment.Actors.IndexOfByPredicate([](const FKyaActorPlacement& Actor) { return !Actor.MacroAnimations.IsEmpty(); });
    if (!TestTrue(TEXT("Level has an actor with macro animations"), AnimatedActor != INDEX_NONE)) return false;
    Bank->SceneEnvironment.Actors[AnimatedActor].MacroAnimations[0].MacroAnimationId = 123;
    const int32 ClusterActor = Bank->SceneEnvironment.Actors.IndexOfByPredicate([](const FKyaActorPlacement& Actor) {
        return Actor.ClassId == 0x18 && Actor.Behaviours.ContainsByPredicate([](const FKyaActorBehaviour& Behaviour) { return Behaviour.Id == 2 && !Behaviour.ClusterZones.IsEmpty(); });
    });
    if (!TestTrue(TEXT("Level has clusteriser zone data"), ClusterActor != INDEX_NONE)) return false;
    const int32 ClusterBehaviour = Bank->SceneEnvironment.Actors[ClusterActor].Behaviours.IndexOfByPredicate([](const FKyaActorBehaviour& Behaviour) { return Behaviour.Id == 2 && !Behaviour.ClusterZones.IsEmpty(); });
    Bank->SceneEnvironment.Actors[ClusterActor].Behaviours[ClusterBehaviour].ClusterZones[0].ZoneRef = 1234;
    const int32 ModelActor = Bank->SceneEnvironment.Actors.IndexOfByPredicate([](const FKyaActorPlacement& Actor) {
        return !Actor.Models.IsEmpty() && !Actor.Models[0].MeshName.IsEmpty();
    });
    if (!TestTrue(TEXT("Level has an actor model reference"), ModelActor != INDEX_NONE)) return false;
    FString& ModelName = Bank->SceneEnvironment.Actors[ModelActor].Models[0].MeshName;
    ModelName[0] = ModelName[0] == TEXT('X') ? TEXT('Y') : TEXT('X');
    Bank->SceneConfiguration.CollisionBankValues[0] = 42;
    Bank->SceneConfiguration.LevelAnimationCount = 1053;
    Bank->SceneConfiguration.ActorClassAllocations[0].Count = 2;
    TArray<uint8> Edited;
    FMemoryWriter EditedWriter(Edited);
    if (!TestTrue(TEXT("Export edited bank"), Exporter->ExportBinary(Bank, TEXT("bnk"), EditedWriter, GWarn))) return false;
    const uint8* EditedCursor = Edited.GetData();
    UKyaLevelBank* Reimported = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
        GetTransientPackage(), NAME_None, RF_Transient, nullptr, TEXT("bnk"), EditedCursor,
        Edited.GetData() + Edited.Num(), GWarn));
    if (!TestNotNull(TEXT("Reimport edited bank"), Reimported)) return false;
    TestEqual(TEXT("Edited field 1c"), Reimported->SceneConfiguration.Field1C, 4.5f);
    TestEqual(TEXT("Edited field 20"), Reimported->SceneConfiguration.Field20, int64(123456));
    TestEqual(TEXT("Edited field 24"), Reimported->SceneConfiguration.Field24, -0.75f);
    TestEqual(TEXT("Edited common texture count"), Reimported->SceneConfiguration.CommonTextureCount, 123);
    TestEqual(TEXT("Edited scene header word"), Reimported->SceneConfiguration.HeaderWords[1], int64(0x12345678));
    TestEqual(TEXT("Edited common mesh value"), Reimported->SceneConfiguration.CommonMeshValues[0], -27);
    TestEqual(TEXT("Edited texture word"), Reimported->SceneConfiguration.TextureWord, -19);
    TestEqual(TEXT("Edited post-animation word"), Reimported->SceneConfiguration.PostAnimationWords[2], int64(0x87654321));
    TestEqual(TEXT("Edited particle word"), Reimported->SceneConfiguration.ParticleWord, int64(0xabcdef12));
    TestEqual(TEXT("Edited clip distance"), Reimported->SceneEnvironment.ClipDistance, 456.0f);
    TestEqual(TEXT("Edited fog color"), Reimported->SceneEnvironment.FogColor, FColor(12, 34, 56, 78));
    TestEqual(TEXT("Edited mipmap K"), Reimported->SceneEnvironment.MipmapK, -12);
    TestEqual(TEXT("Edited camera FOV"), Reimported->SceneEnvironment.CameraFov, 1.25f);
    TestEqual(TEXT("Edited camera position"), Reimported->SceneEnvironment.CameraPosition, FVector(2.0, -3.0, 4.5));
    TestEqual(TEXT("Edited camera flags"), Reimported->SceneEnvironment.CameraFlags, int64(0x1234));
    TestEqual(TEXT("Edited camera target offset"), Reimported->SceneEnvironment.CameraTargetOffset, FVector(1.0, 2.0, 3.0));
    TestEqual(TEXT("Edited camera configuration angle"), Reimported->SceneEnvironment.CameraConfigAngle, 55.0f);
    if (Bank->SceneEnvironment.bHasCameraSwitchData)
    {
        TestEqual(TEXT("Camera switch actor round trips"), Reimported->SceneEnvironment.CameraSwitchActor, 47);
        TestEqual(TEXT("Camera switch value round trips"), Reimported->SceneEnvironment.CameraSwitchValueB, 9.25f);
    }
    if (!Bank->SceneEnvironment.CameraConditions.IsEmpty() && TestEqual(TEXT("Camera condition count round trips"), Reimported->SceneEnvironment.CameraConditions.Num(), Bank->SceneEnvironment.CameraConditions.Num()))
        TestEqual(TEXT("Edited camera condition round trips"), Reimported->SceneEnvironment.CameraConditions[0].Value, 8.25f);
    TestTrue(TEXT("Optional camera configuration round trips"), Reimported->SceneEnvironment.CameraOptionalConfigValues == Bank->SceneEnvironment.CameraOptionalConfigValues);
    TestTrue(TEXT("Extra camera values round trip"), Reimported->SceneEnvironment.CameraExtraValues == Bank->SceneEnvironment.CameraExtraValues);
    TestEqual(TEXT("Edited camera pitch angles"), Reimported->SceneEnvironment.CameraPitchAngles, FVector(10.0, 20.0, 30.0));
    TestEqual(TEXT("Edited scene field 118"), Reimported->SceneEnvironment.Field118, 25.0f);
    TestEqual(TEXT("Edited scene field 11c"), Reimported->SceneEnvironment.Field11C, int64(1));
    TestEqual(TEXT("Edited scene count 120"), Reimported->SceneEnvironment.Count120, int64(3));
    TestEqual(TEXT("Edited map angle"), Reimported->SceneEnvironment.MapAngle, 42.0f);
    TestEqual(TEXT("Edited map actor"), Reimported->SceneEnvironment.MapActorRef, 123);
    TestEqual(TEXT("Edited default material"), Reimported->SceneEnvironment.DefaultMaterialIndex, 7);
    TestEqual(TEXT("Edited particle pool count"), Reimported->SceneEnvironment.EffectPoolCounts[3], int64(37));
    TestEqual(TEXT("Edited objective variable"), Reimported->SceneEnvironment.Objectives[0].ScenarioVariable, 91);
    TestEqual(TEXT("Edited objective field"), Reimported->SceneEnvironment.Objectives[0].Fields[0], int64(321));
    TestEqual(TEXT("Edited field 10c"), Reimported->SceneEnvironment.Field10C, 20);
    TestEqual(TEXT("Edited field 110"), Reimported->SceneEnvironment.Field110, 21);
    TestEqual(TEXT("Edited waypoint position"), Reimported->SceneEnvironment.Waypoints[0].Position, FVector(1.0, 2.0, 3.0));
    TestTrue(TEXT("Edited path point"), Reimported->SceneEnvironment.Paths[0].Points[0].Equals(FVector4(1, 2, 3, 1)));
    TestEqual(TEXT("Edited actor position"), Reimported->SceneEnvironment.Actors[0].Position, FVector(4, 5, 6));
    TestEqual(TEXT("Edited actor visibility"), Reimported->SceneEnvironment.Actors[0].VisibilityDistance, 123.0f);
    TestEqual(TEXT("Edited actor default behavior"), Reimported->SceneEnvironment.Actors[0].DefaultBehaviourId, 7);
    TestEqual(TEXT("Edited actor scenario condition"), Reimported->SceneEnvironment.Actors[ConditionActor].Conditions[0].Value, 197);
    TestEqual(TEXT("Edited checkpoint sector"), Reimported->SceneEnvironment.Actors[CheckpointActor].Checkpoints[0].SectorId, 87);
    TestEqual(TEXT("Edited moving platform weight"), Reimported->SceneEnvironment.Actors[PlatformActor].PlatformWeight, 12.5f);
    {
        const FKyaPlatformProperties& Properties = Reimported->SceneEnvironment.Actors[PlatformDetailActor].PlatformProperties;
        TestEqual(TEXT("Edited platform tilt value"), Properties.TiltValue, 6.75f);
        TestEqual(TEXT("Edited platform sound ref A"), Properties.SoundRefA, 5);
        TestEqual(TEXT("Edited platform sample ref A"), Properties.SampleRefA, 6);
        TestEqual(TEXT("Edited platform effect 1c"), Properties.EffectRef1C, int64(0x1234));
        TestEqual(TEXT("Edited platform effect 20"), Properties.EffectRef20, int64(0x5678));
        TestEqual(TEXT("Edited platform flags"), Properties.PlatformFlags, int64(0x9abc));
        TestEqual(TEXT("Edited platform camera index"), Properties.CameraIndex, 23);
    }
    if (PlatformCameraActor != INDEX_NONE)
    {
        const FKyaPlatformCamera& EditedCamera = Reimported->SceneEnvironment.Actors[PlatformCameraActor].PlatformCameras[0];
        const FKyaPlatformCamera& SourceCamera = Bank->SceneEnvironment.Actors[PlatformCameraActor].PlatformCameras[0];
        TestEqual(TEXT("Edited platform camera event type"), EditedCamera.EventType, SourceCamera.EventType);
        TestEqual(TEXT("Edited platform camera index"), EditedCamera.CameraIndex, SourceCamera.CameraIndex);
        TestEqual(TEXT("Edited platform camera field 8"), EditedCamera.Field8, 1.5f);
    }
    if (PlatformSoundActor != INDEX_NONE)
    {
        const FKyaPlatformSound& EditedSound = Reimported->SceneEnvironment.Actors[PlatformSoundActor].PlatformSoundsA[0];
        TestEqual(TEXT("Edited platform sound segment start"), EditedSound.SegmentStart, 5);
        TestEqual(TEXT("Edited platform sound segment end"), EditedSound.SegmentEnd, 9);
        TestEqual(TEXT("Edited platform sound reference"), EditedSound.SoundRef, 81);
    }
    {
        const FKyaActorPlacement& Box = Reimported->SceneEnvironment.Actors[BasicBoxActor];
        TestEqual(TEXT("Edited basic box field 160"), Box.BasicBoxField160, 77);
        TestEqual(TEXT("Edited basic box life"), Box.BasicBoxLifeMax, 25.0f);
        TestEqual(TEXT("Edited basic box vibration 0"), Box.BasicBoxVibration0, 1.25f);
        TestEqual(TEXT("Edited basic box vibration 1"), Box.BasicBoxVibration1, 2.25f);
        TestEqual(TEXT("Edited basic box vibration 2"), Box.BasicBoxVibration2, 3.25f);
        TestEqual(TEXT("Edited basic box vibration 3"), Box.BasicBoxVibration3, 4.25f);
        TestEqual(TEXT("Edited basic box field 180"), Box.BasicBoxField180, 0x1234);
        TestEqual(TEXT("Edited basic box field 18c"), Box.BasicBoxField18C, 11);
        TestEqual(TEXT("Edited basic box field 190"), Box.BasicBoxField190, 12);
        TestEqual(TEXT("Edited basic box field 198"), Box.BasicBoxField198, 13);
        TestEqual(TEXT("Edited basic box field 194"), Box.BasicBoxField194, 14);
        TestEqual(TEXT("Edited basic box camera index"), Box.BasicBoxSwitch.Camera.CameraIndex, 9);
        TestEqual(TEXT("Edited basic box camera flags"), Box.BasicBoxSwitch.Camera.Flags, 39);
        TestEqual(TEXT("Edited basic box camera field 8"), Box.BasicBoxSwitch.Camera.Field8, 1.5f);
        TestEqual(TEXT("Edited basic box camera field c"), Box.BasicBoxSwitch.Camera.FieldC, 2.5f);
        TestEqual(TEXT("Edited basic box camera field 10"), Box.BasicBoxSwitch.Camera.Field10, 3.5f);
        TestEqual(TEXT("Edited basic box camera actor"), Box.BasicBoxSwitch.Camera.ActorRef, 41);
        TestEqual(TEXT("Edited basic box camera zone"), Box.BasicBoxSwitch.Camera.ZoneRef, 43);
        TestEqual(TEXT("Edited basic box camera field 1c"), Box.BasicBoxSwitch.Camera.Field1C, 4.5f);
        TestEqual(TEXT("Edited basic box add-on field 0"), Box.BasicBoxAddOn.Field0, 1);
        TestEqual(TEXT("Edited basic box add-on money type"), Box.BasicBoxAddOn.MoneyType, 2);
        TestEqual(TEXT("Edited basic box add-on money count"), Box.BasicBoxAddOn.MoneyCount, 3);
        TestEqual(TEXT("Edited basic box add-on bonus count"), Box.BasicBoxAddOn.BonusCount, 4);
        TestEqual(TEXT("Edited basic box add-on field c"), Box.BasicBoxAddOn.FieldC, 5);
    }
    TestEqual(TEXT("Edited FX field 19c"), Reimported->SceneEnvironment.Actors[FxActor].FxField19C, 0.75f);
    TestEqual(TEXT("Edited FX field 1a0"), Reimported->SceneEnvironment.Actors[FxActor].FxField1A0, 2.5f);
    TestEqual(TEXT("Edited FX single behaviour"), Reimported->SceneEnvironment.Actors[FxSingleActor].Behaviours[FxSingleBehaviour].FxBehaviourValue, 123);
    TestEqual(TEXT("Edited FX path behaviour"), Reimported->SceneEnvironment.Actors[FxPathActor].Behaviours[FxPathBehaviour].FxBehaviourValue, 456);
    TestEqual(TEXT("Edited FX path follow index"), Reimported->SceneEnvironment.Actors[FxPathActor].Behaviours[FxPathBehaviour].FxPathFollowIndex, 7);
    {
        const FKyaActorPlacement& Araignos = Reimported->SceneEnvironment.Actors[AraignosActor];
        TestEqual(TEXT("Edited Araignos weight"), Araignos.AraignosWeightB, 1.5f);
        TestEqual(TEXT("Edited Araignos path index"), Araignos.AraignosPathFollowIndex, 8);
        TestEqual(TEXT("Edited Araignos field 1f8"), Araignos.AraignosField1F8, 2.5f);
        TestEqual(TEXT("Edited Araignos field 1fc"), Araignos.AraignosField1FC, 3.5f);
        TestEqual(TEXT("Edited Araignos damage"), Araignos.AraignosDamage, 4.5f);
        TestEqual(TEXT("Edited Araignos sound 240"), Araignos.AraignosSoundRef240, 51);
        TestEqual(TEXT("Edited Araignos sound 238"), Araignos.AraignosSoundRef238, 52);
        TestEqual(TEXT("Edited Araignos sound 23c"), Araignos.AraignosSoundRef23C, 53);
        TestEqual(TEXT("Edited Araignos add-on field 0"), Araignos.AraignosAddOn.Field0, 1);
        TestEqual(TEXT("Edited Araignos add-on money type"), Araignos.AraignosAddOn.MoneyType, 2);
        TestEqual(TEXT("Edited Araignos add-on money count"), Araignos.AraignosAddOn.MoneyCount, 3);
        TestEqual(TEXT("Edited Araignos add-on bonus count"), Araignos.AraignosAddOn.BonusCount, 4);
        TestEqual(TEXT("Edited Araignos add-on field c"), Araignos.AraignosAddOn.FieldC, 5);
    }
    {
        const FKyaActorPlacement& Projectile = Reimported->SceneEnvironment.Actors[ProjectileActor];
        TestEqual(TEXT("Edited projectile weight"), Projectile.ProjectileWeightB, 1.25f);
        TestEqual(TEXT("Edited projectile life"), Projectile.ProjectileLifeMax, 30.0f);
        TestEqual(TEXT("Edited projectile gravity"), Projectile.ProjectileGravityScale, 2.5f);
        TestEqual(TEXT("Edited projectile dynamic ext 4"), Projectile.ProjectileDynamicExtField4, 3.5f);
        TestEqual(TEXT("Edited projectile dynamic ext 8"), Projectile.ProjectileDynamicExtField8, 4.5f);
        const FKyaProjectileSubObj& Sub = Projectile.ProjectileSubObj;
        TestEqual(TEXT("Edited projectile time"), Sub.TimeToExplode, 5.5f);
        TestEqual(TEXT("Edited projectile range"), Sub.ExplosionRange, 6.5f);
        TestEqual(TEXT("Edited projectile duration"), Sub.AnimDuration, 7.5f);
        TestEqual(TEXT("Edited projectile flags"), Sub.Flags, 0x1234);
        TestEqual(TEXT("Edited projectile damage"), Sub.Damage, 8.5f);
        TestEqual(TEXT("Edited projectile hit radius"), Sub.HitRadius, 9.5f);
        TestEqual(TEXT("Edited projectile warn radius"), Sub.WarnRadius, 10.5f);
        TestEqual(TEXT("Edited projectile field 1c"), Sub.Field1C, 11.5f);
        TestEqual(TEXT("Edited projectile field 20"), Sub.Field20, 12.5f);
        TestEqual(TEXT("Edited projectile field 24"), Sub.Field24, 13.5f);
        TestEqual(TEXT("Edited projectile field 28"), Sub.Field28, 14);
        TestEqual(TEXT("Edited projectile material"), Sub.MaterialId, 15);
        TestEqual(TEXT("Edited projectile field 30"), Sub.Field30, 16);
        TestEqual(TEXT("Edited projectile field 34"), Sub.Field34, 17);
        TestEqual(TEXT("Edited projectile field 38"), Sub.Field38, 18);
        TestEqual(TEXT("Edited projectile field 3c"), Sub.Field3C, 19);
        TestEqual(TEXT("Edited projectile field 40"), Sub.Field40, 20.5f);
        TestEqual(TEXT("Edited projectile field 44"), Sub.Field44, 21.5f);
        const FKyaPatternPart& Pattern = Projectile.ProjectilePattern;
        TestEqual(TEXT("Edited pattern field 4"), Pattern.Field4, 22.5f);
        TestEqual(TEXT("Edited pattern field 0"), Pattern.Field0, 23.5f);
        TestEqual(TEXT("Edited pattern field c"), Pattern.FieldC, 24.5f);
        TestEqual(TEXT("Edited pattern field 8"), Pattern.Field8, 25.5f);
        TestEqual(TEXT("Edited pattern field 10"), Pattern.Field10, 26.5f);
        TestEqual(TEXT("Edited pattern field 14"), Pattern.Field14, 27.5f);
        TestEqual(TEXT("Edited pattern material"), Pattern.MaterialId, 28);
        TestEqual(TEXT("Edited pattern point count"), Pattern.PointCount, 2.5f);
        TestEqual(TEXT("Edited pattern field 20"), Pattern.Field20, 29.5f);
        TestEqual(TEXT("Edited pattern field 24"), Pattern.Field24, 30.5f);
        TestEqual(TEXT("Edited pattern field 28"), Pattern.Field28, 31);
    }
    {
        const FKyaActorPlacement& Wind = Reimported->SceneEnvironment.Actors[WindActor];
        TestEqual(TEXT("Edited wind field 160"), Wind.WindField160, 0x1234);
        TestEqual(TEXT("Edited wind max"), Wind.WindMaxWind, 1.5f);
        TestEqual(TEXT("Edited wind field 168"), Wind.WindField168, 2.5f);
        TestEqual(TEXT("Edited wind field 16c"), Wind.WindField16C, 3.5f);
        TestEqual(TEXT("Edited wind field 174"), Wind.WindField174, 4.5f);
        TestEqual(TEXT("Edited wind field 180"), Wind.WindField180, 5.5f);
        TestEqual(TEXT("Edited wind field 184"), Wind.WindField184, 6.5f);
        TestEqual(TEXT("Edited wind field 188"), Wind.WindField188, 7.5f);
        TestEqual(TEXT("Edited wind field 18c"), Wind.WindField18C, 8.5f);
        TestEqual(TEXT("Edited wind field 178"), Wind.WindField178, 9.5f);
        TestEqual(TEXT("Edited wind field 17c"), Wind.WindField17C, 10.5f);
        TestEqual(TEXT("Edited wind field 190"), Wind.WindField190, 0x2345);
        TestEqual(TEXT("Edited wind sound"), Wind.WindSoundRef, 61);
        TestEqual(TEXT("Edited wind activation zone"), Wind.WindActivationZoneIndex, 2);
        if (!Wind.WindActorRefs.IsEmpty()) TestEqual(TEXT("Edited wind actor ref"), Wind.WindActorRefs[0], 71);
        TestEqual(TEXT("Edited wind material"), Wind.WindMaterialId, 62);
        TestEqual(TEXT("Edited wind zone 1c4"), Wind.WindZoneRef1C4, 3);
        TestEqual(TEXT("Edited wind color"), Wind.WindColor1C8, 0x11223344);
        TestEqual(TEXT("Edited wind waypoint 1cc"), Wind.WindWaypointRef1CC, 4);
        TestEqual(TEXT("Edited wind sector hash"), Wind.WindSectorHierarchyHash, int64(0x123456789abcdef0ull));
        if (!Wind.WindSwitch1D0.Targets.IsEmpty())
        {
            TestEqual(TEXT("Edited wind switch target"), Wind.WindSwitch1D0.Targets[0].TargetRef, 72);
            TestEqual(TEXT("Edited wind switch message"), Wind.WindSwitch1D0.Targets[0].MessageId, 73);
        }
        TestEqual(TEXT("Edited wind switch 1d0 camera"), Wind.WindSwitch1D0.Camera.CameraIndex, 5);
        TestEqual(TEXT("Edited wind switch 1d0 float"), Wind.WindSwitch1D0.Camera.Field8, 1.25f);
        TestEqual(TEXT("Edited wind switch 1d8 camera"), Wind.WindSwitch1D8.Camera.CameraIndex, 6);
        if (!Wind.WindSwitch1E0.Targets.IsEmpty()) TestEqual(TEXT("Edited wind switch 1e0 target"), Wind.WindSwitch1E0.Targets[0].TargetRef, 74);
        TestEqual(TEXT("Edited wind switch 1e0 camera"), Wind.WindSwitch1E0.Camera.CameraIndex, 7);
        TestEqual(TEXT("Edited wind field 194"), Wind.WindField194, 11.5f);
        TestEqual(TEXT("Edited wind field 198"), Wind.WindField198, 12.5f);
        TestEqual(TEXT("Edited wind field 19c"), Wind.WindField19C, 13.5f);
        TestEqual(TEXT("Edited wind field 1a0"), Wind.WindField1A0, 14.5f);
        TestEqual(TEXT("Edited wind field 1a4"), Wind.WindField1A4, 15.5f);
        TestEqual(TEXT("Edited wind waypoint"), Wind.WindWaypointRef, 8);
        TestEqual(TEXT("Edited wind field 1ac"), Wind.WindField1AC, 16.5f);
        TestEqual(TEXT("Edited wind field 1b0"), Wind.WindField1B0, 17.5f);
    }
    {
        const FKyaActorPlacement& Trap = Reimported->SceneEnvironment.Actors[TrapActor];
        TestEqual(TEXT("Edited trap weight"), Trap.TrapWeightB, 1.75f);
        TestEqual(TEXT("Edited trap field 14"), Trap.TrapField14, 21);
        TestEqual(TEXT("Edited trap field 18"), Trap.TrapField18, 22);
        TestEqual(TEXT("Edited trap field 1c"), Trap.TrapField1C, 23);
        TestEqual(TEXT("Edited trap field 20"), Trap.TrapField20, 24);
        TestEqual(TEXT("Edited trap field 24"), Trap.TrapField24, 25);
        TestEqual(TEXT("Edited trap actor ref"), Trap.TrapActorRef, 9);
        TestEqual(TEXT("Edited trap speed"), Trap.TrapSpeed, 2.25f);
        TestEqual(TEXT("Edited trap acceleration"), Trap.TrapAcceleration, 3.25f);
        TestEqual(TEXT("Edited trap path"), Trap.TrapPathFollowIndex, 10);
        TestEqual(TEXT("Edited trap field 4c"), Trap.TrapField4C, 4.25f);
        TestEqual(TEXT("Edited trap escape attempts"), Trap.TrapRequiredEscapeAttempts, 11);
    }
    {
        const FKyaActorBehaviour& Stand = Reimported->SceneEnvironment.Actors[TrapStandActor].Behaviours[TrapStandBehaviour];
        TestEqual(TEXT("Edited trap stand field 0"), Stand.TrapBehaviourField0, 111);
        TestEqual(TEXT("Edited trap stand field 4"), Stand.TrapBehaviourField4, 112);
        TestEqual(TEXT("Edited trap stand field 8"), Stand.TrapBehaviourField8, 113);
        const FKyaActorBehaviour& Inactive = Reimported->SceneEnvironment.Actors[TrapInactiveActor].Behaviours[TrapInactiveBehaviour];
        TestEqual(TEXT("Edited trap inactive field 0"), Inactive.TrapBehaviourField0, 121);
        TestEqual(TEXT("Edited trap inactive field 4"), Inactive.TrapBehaviourField4, 122);
        TestEqual(TEXT("Edited trap inactive field 8"), Inactive.TrapBehaviourField8, 123);
    }
    {
        const FKyaActorPlacement& Weapon = Reimported->SceneEnvironment.Actors[WeaponActor];
        TestEqual(TEXT("Edited weapon weight"), Weapon.WeaponWeightB, 0.75f);
        TestEqual(TEXT("Edited weapon field 1d8"), Weapon.WeaponField1D8, 1.75f);
        TestEqual(TEXT("Edited weapon field 1dc"), Weapon.WeaponField1DC, 2.75f);
        const FKyaActorBehaviour& Pistol = Reimported->SceneEnvironment.Actors[WeaponPistolActor].Behaviours[WeaponPistolBehaviour];
        TestEqual(TEXT("Edited weapon burst count"), Pistol.WeaponBurst.Num(), 2);
        TestEqual(TEXT("Edited weapon burst field c"), Pistol.WeaponBurst[0].FieldC, 3.75f);
        TestEqual(TEXT("Edited weapon burst field 4"), Pistol.WeaponBurst[0].Field4, 4);
        TestEqual(TEXT("Edited weapon burst field 0"), Pistol.WeaponBurst[0].Field0, 5.75f);
        TestEqual(TEXT("Edited weapon burst field 8"), Pistol.WeaponBurst[0].Field8, 6.75f);
        TestEqual(TEXT("Edited weapon field 40"), Pistol.WeaponField40, 41);
        TestEqual(TEXT("Edited weapon field 44"), Pistol.WeaponField44, 42);
        TestEqual(TEXT("Edited weapon sound"), Pistol.WeaponSoundIndex, 43);
        TestEqual(TEXT("Edited weapon pistol field 2e0"), Pistol.WeaponPistolField2E0, 44);
        TestEqual(TEXT("Edited weapon pistol field 2e4"), Pistol.WeaponPistolField2E4, 45);
        const FKyaActorBehaviour& Sniper = Reimported->SceneEnvironment.Actors[WeaponSniperActor].Behaviours[WeaponSniperBehaviour];
        TestEqual(TEXT("Edited weapon sniper field 40"), Sniper.WeaponField40, 51);
        TestEqual(TEXT("Edited weapon sniper field 44"), Sniper.WeaponField44, 52);
        TestEqual(TEXT("Edited weapon sniper sound"), Sniper.WeaponSoundIndex, 53);
        TestEqual(TEXT("Edited weapon sniper texture"), Sniper.WeaponSniperTextureIndex, 54);
        TestEqual(TEXT("Edited weapon sniper mesh"), Sniper.WeaponSniperMeshIndex, 55);
        TestEqual(TEXT("Edited weapon sniper material"), Sniper.WeaponSniperMaterialId, 56);
        const FKyaActorBehaviour& Rpg = Reimported->SceneEnvironment.Actors[WeaponRpgActor].Behaviours[WeaponRpgBehaviour];
        if (!Rpg.WeaponRpgActorRefs.IsEmpty()) TestEqual(TEXT("Edited weapon RPG actor"), Rpg.WeaponRpgActorRefs[0], 62);
    }
    {
        const FKyaSwitch& Switch = Reimported->SceneEnvironment.Actors[SwitchActor].SwitchTarget;
        if (!Switch.Targets.IsEmpty())
        {
            TestEqual(TEXT("Edited switch target"), Switch.Targets[0].TargetRef, 81);
            TestEqual(TEXT("Edited switch message"), Switch.Targets[0].MessageId, 82);
        }
        TestEqual(TEXT("Edited switch camera"), Switch.Camera.CameraIndex, 9);
        TestEqual(TEXT("Edited switch camera float"), Switch.Camera.Field8, 2.5f);
    }
    {
        const FKyaActorPlacement& Bonus = Reimported->SceneEnvironment.Actors[BonusActor];
        TestEqual(TEXT("Edited bonus weight"), Bonus.BonusWeightB, 1.5f);
        TestEqual(TEXT("Edited bonus sound"), Bonus.BonusSoundRef, 71);
        TestEqual(TEXT("Edited bonus flare material"), Bonus.BonusFlareMaterialId, 72);
        TestEqual(TEXT("Edited bonus anim material"), Bonus.BonusAnimMaterialId, 73);
        const FKyaActorBehaviour& Turn = Reimported->SceneEnvironment.Actors[BonusTurnActor].Behaviours[BonusTurnBehaviour];
        TestEqual(TEXT("Edited bonus turn field 14"), Turn.BonusField14, 1.25f);
        TestEqual(TEXT("Edited bonus turn field 1b8"), Turn.BonusField1B8, 2.25f);
        TestEqual(TEXT("Edited bonus turn field 1b4"), Turn.BonusField1B4, 3.25f);
        TestEqual(TEXT("Edited bonus turn material"), Turn.BonusMaterialId, 74);
        TestEqual(TEXT("Edited bonus turn field 1b0"), Turn.BonusField1B0, 4.25f);
        TestEqual(TEXT("Edited bonus turn fx count"), Turn.BonusFxTailCount, 75);
        TestEqual(TEXT("Edited bonus turn g2d"), Turn.BonusG2DIndex, 76);
        TestEqual(TEXT("Edited bonus turn value"), Turn.BonusTurnValue, 5.25f);
        TestEqual(TEXT("Edited bonus flock path"), Reimported->SceneEnvironment.Actors[BonusFlockActor].Behaviours[BonusFlockBehaviour].BonusFlockPathIndex, 77);
        TestEqual(TEXT("Edited bonus add-on field"), Reimported->SceneEnvironment.Actors[BonusAddOnActor].Behaviours[BonusAddOnBehaviour].BonusAddOnField18, 6.25f);
    }
    {
        const FKyaActorPlacement& Micken = Reimported->SceneEnvironment.Actors[MickenActor];
        TestEqual(TEXT("Edited Micken weight"), Micken.MickenWeightB, 0.5f);
        TestEqual(TEXT("Edited Micken life"), Micken.MickenLifeMax, 40.0f);
        TestEqual(TEXT("Edited Micken gravity"), Micken.MickenGravityScale, 2.0f);
        TestEqual(TEXT("Edited Micken dynamic ext 4"), Micken.MickenDynamicExtField4, 3.0f);
        TestEqual(TEXT("Edited Micken dynamic ext 8"), Micken.MickenDynamicExtField8, 4.0f);
        TestEqual(TEXT("Edited Micken waypoint"), Micken.MickenWaypointIndex, 91);
        TestEqual(TEXT("Edited Micken actor"), Micken.MickenActorIndex, 92);
        TestEqual(TEXT("Edited Micken field 358"), Micken.MickenField358, 93);
        TestEqual(TEXT("Edited Micken field 35c"), Micken.MickenField35C, 5.0f);
        TestEqual(TEXT("Edited Micken field 360"), Micken.MickenField360, 94);
        const FKyaActorBehaviour& Eat = Reimported->SceneEnvironment.Actors[MickenEatActor].Behaviours[MickenEatBehaviour];
        TestEqual(TEXT("Edited Micken eat field 8"), Eat.MickenEatField8, 6.0f);
        TestEqual(TEXT("Edited Micken eat field c"), Eat.MickenEatFieldC, 7.0f);
        TestEqual(TEXT("Edited Micken eat field 10"), Eat.MickenEatField10, 95);
        TestEqual(TEXT("Edited Micken eat path"), Eat.MickenEatPathIndex, 96);
    }
    {
        const FKyaActorPlacement& Wolfen = Reimported->SceneEnvironment.Actors[WolfenActor];
        TestEqual(TEXT("Edited fighter input analyser"), Wolfen.FighterInputAnalyserFlag, 1);
        TestEqual(TEXT("Edited fighter command mask"), Wolfen.FighterCommandMask, 0x1234);
        TestEqual(TEXT("Edited fighter field 7c0"), Wolfen.FighterField7C0, 1.1f);
        TestEqual(TEXT("Edited fighter field 7c4"), Wolfen.FighterField7C4, 1.2f);
        TestEqual(TEXT("Edited fighter field 7d4"), Wolfen.FighterField7D4, 1.3f);
        TestEqual(TEXT("Edited fighter field 7cc"), Wolfen.FighterField7CC, 1.4f);
        TestEqual(TEXT("Edited fighter anatomy 20"), Wolfen.FighterAnatomy20, 2.1f);
        TestEqual(TEXT("Edited fighter anatomy 28"), Wolfen.FighterAnatomy28, 2.2f);
        TestEqual(TEXT("Edited fighter anatomy 24"), Wolfen.FighterAnatomy24, 2.3f);
        TestEqual(TEXT("Edited fighter anatomy 2c"), Wolfen.FighterAnatomy2C, 2.4f);
        TestEqual(TEXT("Edited fighter vector A"), Wolfen.FighterField810A, FVector(3.1f, 3.2f, 3.3f));
        TestEqual(TEXT("Edited fighter vector B"), Wolfen.FighterField810B, FVector(3.4f, 3.5f, 3.6f));
        TestEqual(TEXT("Edited fighter field 800"), Wolfen.FighterField800, 1);
        TestEqual(TEXT("Edited fighter flags"), Wolfen.FighterFightFlags, 0x55);
        TestEqual(TEXT("Edited fighter field 50c"), Wolfen.FighterField50C, 4.1f);
        TestEqual(TEXT("Edited fighter field 510"), Wolfen.FighterField510, 4.2f);
        TestEqual(TEXT("Edited fighter field 514"), Wolfen.FighterField514, 4.3f);
        TestEqual(TEXT("Edited fighter field 518"), Wolfen.FighterField518, 99);
        TestEqual(TEXT("Edited fighter field 58"), Wolfen.FighterField58, 100);
        TestEqual(TEXT("Edited fighter field 51c count"), Wolfen.FighterField51C.Num(), 8);
        TestEqual(TEXT("Edited fighter field 51c first"), Wolfen.FighterField51C[0], 10);
        TestEqual(TEXT("Edited fighter field 53c count"), Wolfen.FighterField53C.Num(), 5);
        TestEqual(TEXT("Edited fighter field 53c last"), Wolfen.FighterField53C[4], 24);
        TestEqual(TEXT("Edited fighter static texture"), Wolfen.FighterStaticMeshTextureIndex, 111);
        TestEqual(TEXT("Edited fighter static mesh"), Wolfen.FighterStaticMeshMeshIndex, 112);
        TestEqual(TEXT("Edited fighter field 5f0"), Wolfen.FighterField5F0, 113);
        TestEqual(TEXT("Edited fighter weapon index"), Wolfen.FighterWeaponActorIndex, 114);
        TestEqual(TEXT("Edited Wolfen field b74"), Wolfen.WolfenFieldB74, 2);
        TestEqual(TEXT("Edited Wolfen start sector"), Wolfen.WolfenStartSectorId, 5);
        TestEqual(TEXT("Edited Wolfen hearing range"), Wolfen.WolfenHearingRangeSquared, 100.0f);
        TestEqual(TEXT("Edited Wolfen hearing height"), Wolfen.WolfenHearingMaxHeight, 10.0f);
        TestEqual(TEXT("Edited Wolfen vision"), Wolfen.WolfenVisionRange, 20.0f);
        TestEqual(TEXT("Edited Wolfen vision fov"), Wolfen.WolfenVisionFov, 70.0f);
        TestEqual(TEXT("Edited Wolfen vision field 38"), Wolfen.WolfenVisionField38, 3.25f);
        TestEqual(TEXT("Edited Wolfen vision field 3c"), Wolfen.WolfenVisionField3C, 2.25f);
        TestEqual(TEXT("Edited Wolfen vision flags"), Wolfen.WolfenVisionFlags, 4);
        TestEqual(TEXT("Edited Wolfen detection"), Wolfen.WolfenVisionDetectionRange, 1.25f);
        TestEqual(TEXT("Edited Wolfen field cf4"), Wolfen.WolfenFieldCF4, 1.5f);
        TestEqual(TEXT("Edited Wolfen walk speed"), Wolfen.WolfenWalkSpeed, 2.5f);
        TestEqual(TEXT("Edited Wolfen walk acceleration"), Wolfen.WolfenWalkAcceleration, 3.5f);
        TestEqual(TEXT("Edited Wolfen walk rotation"), Wolfen.WolfenWalkRotSpeed, 4.5f);
        TestEqual(TEXT("Edited Wolfen run scale"), Wolfen.WolfenRunSpeedScale, 5.5f);
        TestEqual(TEXT("Edited Wolfen default run"), Wolfen.WolfenDefaultRunSpeed, 6.5f);
        TestEqual(TEXT("Edited Wolfen run acceleration"), Wolfen.WolfenRunAcceleration, 7.5f);
        TestEqual(TEXT("Edited Wolfen run rotation"), Wolfen.WolfenRotRunSpeed, 8.5f);
        TestEqual(TEXT("Edited Wolfen magic"), Wolfen.WolfenMagicRequired, 30);
        TestEqual(TEXT("Edited Wolfen field d30"), Wolfen.WolfenFieldD30, 31);
        TestEqual(TEXT("Edited Wolfen field b90"), Wolfen.WolfenFieldB90, 9.5f);
        TestEqual(TEXT("Edited Wolfen field b94"), Wolfen.WolfenFieldB94, 10.5f);
        TestEqual(TEXT("Edited Wolfen field b98"), Wolfen.WolfenFieldB98, 11.5f);
        if (!Wolfen.FighterBlows.IsEmpty())
        {
            const FKyaFighterBlow& Blow = Wolfen.FighterBlows[0];
            TestEqual(TEXT("Edited Wolfen blow name"), Blow.Name, Bank->SceneEnvironment.Actors[WolfenActor].FighterBlows[0].Name);
            TestEqual(TEXT("Edited Wolfen blow field 4a"), Blow.Field4A, 201);
            TestEqual(TEXT("Edited Wolfen blow damage"), Blow.Damage, 4.25f);
            TestEqual(TEXT("Edited Wolfen blow vector"), Blow.Field20, FVector(6.25f, 7.25f, 8.25f));
            TestEqual(TEXT("Edited Wolfen blow field 40"), Blow.Field40, 204);
            if (!Blow.SubObjs.IsEmpty()) TestEqual(TEXT("Edited Wolfen blow sub-object"), Blow.SubObjs[0].MaterialId, 207);
            TestEqual(TEXT("Edited Wolfen blow bone A"), Blow.BoneRefA, 211);
            TestEqual(TEXT("Edited Wolfen blow range"), Blow.CanActivateRange, 13.25f);
            if (!Blow.Stages.IsEmpty()) TestEqual(TEXT("Edited Wolfen blow stage"), Blow.Stages[0].AnimId, 213);
        }
        if (!Wolfen.FighterGrabs.IsEmpty())
        {
            const FKyaFighterGrab& Grab = Wolfen.FighterGrabs[0];
            TestEqual(TEXT("Edited Wolfen grab name"), Grab.Name, Bank->SceneEnvironment.Actors[WolfenActor].FighterGrabs[0].Name);
            TestEqual(TEXT("Edited Wolfen grab field 4a"), Grab.Field4A, 301);
            TestEqual(TEXT("Edited Wolfen grab field 8"), Grab.Field8, 21.25f);
            TestEqual(TEXT("Edited Wolfen grab field 20"), Grab.Field20, FVector(24.25f, 25.25f, 26.25f));
            TestEqual(TEXT("Edited Wolfen grab field 68"), Grab.Field68, 306);
            if (!Grab.Stages.IsEmpty()) TestEqual(TEXT("Edited Wolfen grab stage"), Grab.Stages[0].AnimId, 308);
        }
        if (!Wolfen.FighterGrabReacts.IsEmpty())
        {
            TestEqual(TEXT("Edited Wolfen grab react name"), Wolfen.FighterGrabReacts[0].Name,
                Bank->SceneEnvironment.Actors[WolfenActor].FighterGrabReacts[0].Name);
            TestEqual(TEXT("Edited Wolfen grab react field 4"), Wolfen.FighterGrabReacts[0].Field4, 311);
        }
        if (!Wolfen.FighterCombos.IsEmpty())
        {
            const FKyaFighterCombo& Combo = Wolfen.FighterCombos[0];
            TestEqual(TEXT("Edited Wolfen combo name"), Combo.Name, Bank->SceneEnvironment.Actors[WolfenActor].FighterCombos[0].Name);
            TestEqual(TEXT("Edited Wolfen combo field 4a"), Combo.Field4A, 321);
            TestEqual(TEXT("Edited Wolfen combo field 8"), Combo.Field8, 37.25f);
            if (!Combo.Inputs.IsEmpty()) TestEqual(TEXT("Edited Wolfen combo input"), Combo.Inputs[0], 323);
            TestEqual(TEXT("Edited Wolfen combo field c"), Combo.FieldC, 38.25f);
            TestEqual(TEXT("Edited Wolfen combo action"), Combo.Action, Bank->SceneEnvironment.Actors[WolfenActor].FighterCombos[0].Action);
        }
        const FKyaWolfenFightParam& Params = Wolfen.WolfenFightParamData;
        if (!Params.Capability0Fields.IsEmpty()) TestEqual(TEXT("Edited Wolfen capability 0 field"), Params.Capability0Fields[0], 401);
        if (!Params.Capability2Fields.IsEmpty()) TestEqual(TEXT("Edited Wolfen capability 2 field"), Params.Capability2Fields[0], 402);
        if (!Params.ConfigurationValues.IsEmpty()) TestEqual(TEXT("Edited Wolfen configuration"), Params.ConfigurationValues[0], 41.25f);
        if (!Params.Capabilities.IsEmpty() && !Params.Capabilities[0].Items.IsEmpty())
        {
            const FKyaWolfenCapabilityItem& Item = Params.Capabilities[0].Items[0];
            if (!Item.Chains.IsEmpty()) TestEqual(TEXT("Edited Wolfen chain"), Item.Chains[0], 403);
            TestEqual(TEXT("Edited Wolfen chain float 0"), Item.Field0, 42.25f);
            TestEqual(TEXT("Edited Wolfen chain float 4"), Item.Field4, 43.25f);
        }
        if (!Params.ComboList1.IsEmpty()) TestEqual(TEXT("Edited Wolfen combo list 1"), Params.ComboList1[0], 404);
        if (!Params.ComboList2.IsEmpty()) TestEqual(TEXT("Edited Wolfen combo list 2"), Params.ComboList2[0], 405);
        TestEqual(TEXT("Edited Wolfen field after lists"), Params.FieldAfterLists, 406);
        TestEqual(TEXT("Edited Wolfen field b44"), Params.FieldB44, 407);
        if (!Params.TuningValues.IsEmpty()) TestEqual(TEXT("Edited Wolfen tuning"), Params.TuningValues[0], 44.25f);
        if (!Params.Knowledge.IsEmpty()) TestEqual(TEXT("Edited Wolfen knowledge"), Params.Knowledge[0], 408);
    }
    {
        const FKyaActorPlacement& Fruit = Reimported->SceneEnvironment.Actors[FruitActor];
        TestEqual(TEXT("Edited fruit weight"), Fruit.FruitWeightB, 0.75f);
        TestEqual(TEXT("Edited fruit life"), Fruit.FruitLifeMax, 50.0f);
        TestEqual(TEXT("Edited fruit gravity"), Fruit.FruitGravityScale, 2.75f);
        TestEqual(TEXT("Edited fruit dynamic ext 4"), Fruit.FruitDynamicExtField4, 3.75f);
        TestEqual(TEXT("Edited fruit dynamic ext 8"), Fruit.FruitDynamicExtField8, 4.75f);
        TestEqual(TEXT("Edited fruit field 360"), Fruit.FruitField360, 0x1234);
        TestEqual(TEXT("Edited fruit camera"), Fruit.FruitSwitch.Camera.CameraIndex, 11);
        TestEqual(TEXT("Edited fruit camera float"), Fruit.FruitSwitch.Camera.Field8, 1.75f);
        TestEqual(TEXT("Edited fruit squashed"), Reimported->SceneEnvironment.Actors[FruitSquashedActor].Behaviours[FruitSquashedBehaviour].FruitSquashedValue, 5.75f);
        TestEqual(TEXT("Edited fruit bonus actor"), Reimported->SceneEnvironment.Actors[FruitBonusActor].Behaviours[FruitBonusBehaviour].FruitBonusActorRef, 401);
        TestEqual(TEXT("Edited fruit bonus behaviour"), Reimported->SceneEnvironment.Actors[FruitBonusActor].Behaviours[FruitBonusBehaviour].FruitBonusInitialBehaviour, 402);
        const FKyaActorBehaviour& Inventory = Reimported->SceneEnvironment.Actors[FruitInventoryActor].Behaviours[FruitInventoryBehaviour];
        TestEqual(TEXT("Edited fruit inventory field 8"), Inventory.FruitInventoryField8, int64(0x123456789abcdef0ull));
        TestEqual(TEXT("Edited fruit inventory field 10"), Inventory.FruitInventoryField10, int64(0x0fedcba987654321ull));
        TestEqual(TEXT("Edited fruit inventory purchase"), Inventory.FruitInventoryPurchaseId, 403);
        TestEqual(TEXT("Edited fruit inventory field 28"), Inventory.FruitInventoryField28, 404);
        TestEqual(TEXT("Edited fruit inventory money"), Inventory.FruitInventoryMoneyCost, 405);
        TestEqual(TEXT("Edited fruit inventory field 2c"), Inventory.FruitInventoryField2C, 406);
        TestEqual(TEXT("Edited fruit inventory initial"), Inventory.FruitInventoryInitialBehaviour, 407);
    }
    {
        const FKyaActorPlacement& Commander = Reimported->SceneEnvironment.Actors[CommanderActor];
        TestEqual(TEXT("Edited commander detect zone"), Commander.CommanderDetectAreaZoneId, 501);
        TestEqual(TEXT("Edited commander guard zone"), Commander.CommanderGuardAreaZoneId, 502);
        TestEqual(TEXT("Edited commander field 180"), Commander.CommanderField180, 503);
        TestEqual(TEXT("Edited commander path"), Commander.CommanderNewPathFindingId, 504);
        TestEqual(TEXT("Edited commander actor"), Commander.CommanderActorRef, 505);
        TestEqual(TEXT("Edited commander squad value"), Commander.CommanderSquadValue, 1.25f);
        TestEqual(TEXT("Edited commander squad field 4"), Commander.CommanderSquadField4, 506);
        TestEqual(TEXT("Edited commander squad field 8"), Commander.CommanderSquadField8, 507);
        if (!Commander.CommanderTeamEnemies.IsEmpty()) TestEqual(TEXT("Edited commander team"), Commander.CommanderTeamEnemies[0], 508);
        if (!Commander.CommanderSequence.IsEmpty())
        {
            TestEqual(TEXT("Edited commander sequence 0"), Commander.CommanderSequence[0].Field0, 509);
            TestEqual(TEXT("Edited commander sequence 8"), Commander.CommanderSequence[0].Field8, 510);
        }
        if (!Commander.CommanderCameraValues.IsEmpty())
        {
            TestEqual(TEXT("Edited commander camera 0"), Commander.CommanderCameraValues[0], 2.25f);
            TestEqual(TEXT("Edited commander camera 13"), Commander.CommanderCameraValues[13], 3.25f);
        }
        const FKyaActorBehaviour& Behaviour = Reimported->SceneEnvironment.Actors[CommanderBehaviourActor].Behaviours[CommanderBehaviour];
        if (Behaviour.CommanderSquadConfig.Num() >= 2)
        {
            TestEqual(TEXT("Edited commander squad config 0"), Behaviour.CommanderSquadConfig[0], 511);
            TestEqual(TEXT("Edited commander squad config 1"), Behaviour.CommanderSquadConfig[1], 512);
        }
        TestEqual(TEXT("Edited commander camera index"), Behaviour.CommanderSwitch.Camera.CameraIndex, 12);
        TestEqual(TEXT("Edited commander camera float"), Behaviour.CommanderSwitch.Camera.Field8, 4.25f);
        if (!Behaviour.CommanderSwitch.Targets.IsEmpty()) TestEqual(TEXT("Edited commander target"), Behaviour.CommanderSwitch.Targets[0].TargetRef, 513);
    }
    {
        const FKyaActorPlacement& Nativ = Reimported->SceneEnvironment.Actors[NativActor];
        TestEqual(TEXT("Edited Nativ weight"), Nativ.NativWeightB, 0.65f);
        TestEqual(TEXT("Edited Nativ life"), Nativ.NativLifeMax, 60.0f);
        TestEqual(TEXT("Edited Nativ gravity"), Nativ.NativGravityScale, 2.85f);
        TestEqual(TEXT("Edited Nativ dynamic ext 4"), Nativ.NativDynamicExtField4, 3.85f);
        TestEqual(TEXT("Edited Nativ dynamic ext 8"), Nativ.NativDynamicExtField8, 4.85f);
        TestEqual(TEXT("Edited Nativ field 350"), Nativ.NativField350, 601);
        TestEqual(TEXT("Edited Nativ field 354"), Nativ.NativField354, 602);
        TestEqual(TEXT("Edited Nativ field 358"), Nativ.NativField358, 603);
        TestEqual(TEXT("Edited Nativ no-held speed"), Nativ.NativMoveSpeedNoHeld, 5.85f);
        TestEqual(TEXT("Edited Nativ held speed"), Nativ.NativMoveSpeedHeld, 6.85f);
        TestEqual(TEXT("Edited Nativ field 370"), Nativ.NativField370, 604);
        TestEqual(TEXT("Edited Nativ field 368"), Nativ.NativField368, 605);
        TestEqual(TEXT("Edited Nativ field 364"), Nativ.NativField364, 606);
        TestEqual(TEXT("Edited Nativ field 378"), Nativ.NativField378, 607);
        TestEqual(TEXT("Edited Nativ field 37c"), Nativ.NativField37C, 608);
        TestEqual(TEXT("Edited Nativ field 380"), Nativ.NativField380, 609);
        const FKyaActorBehaviour& Behaviour = Reimported->SceneEnvironment.Actors[NativBehaviourActor].Behaviours[NativBehaviour];
        if (!Behaviour.NativTrajectories.IsEmpty())
        {
            TestEqual(TEXT("Edited Nativ trajectory field 0"), Behaviour.NativTrajectories[0].Field0, 7.85f);
            TestEqual(TEXT("Edited Nativ trajectory path"), Behaviour.NativTrajectories[0].PathFollowIndex, 610);
            TestEqual(TEXT("Edited Nativ trajectory field 14"), Behaviour.NativTrajectories[0].Field14, 611);
            TestEqual(TEXT("Edited Nativ trajectory field 18"), Behaviour.NativTrajectories[0].Field18, 8.85f);
        }
    }
    {
        const FKyaActorPlacement& Amortos = Reimported->SceneEnvironment.Actors[AmortosActor];
        TestEqual(TEXT("Edited Amortos actor ref"), Amortos.AmortosActorRef, 701);
        TestEqual(TEXT("Edited Amortos field 160"), Amortos.AmortosField160, 1.15f);
        TestEqual(TEXT("Edited Amortos field 164"), Amortos.AmortosField164, 2.15f);
        TestEqual(TEXT("Edited Amortos field 168"), Amortos.AmortosField168, 3.15f);
        TestEqual(TEXT("Edited Amortos field 16c"), Amortos.AmortosField16C, 4.15f);
        TestEqual(TEXT("Edited Amortos field 170"), Amortos.AmortosField170, 5.15f);
        TestEqual(TEXT("Edited Amortos field 174"), Amortos.AmortosField174, 6.15f);
        TestEqual(TEXT("Edited Amortos field 178"), Amortos.AmortosField178, 7.15f);
        TestEqual(TEXT("Edited Amortos field 17c"), Amortos.AmortosField17C, 8.15f);
        TestEqual(TEXT("Edited Amortos field 180"), Amortos.AmortosField180, 9.15f);
        TestEqual(TEXT("Edited Amortos add-on money type"), Amortos.AmortosAddOn.MoneyType, 2);
        TestEqual(TEXT("Edited Amortos add-on bonus count"), Amortos.AmortosAddOn.BonusCount, 4);
        TestEqual(TEXT("Edited Amortos field 1c8"), Amortos.AmortosField1C8, 702);
        TestEqual(TEXT("Edited Amortos switch 1cc camera"), Amortos.AmortosSwitch1CC.Camera.CameraIndex, 13);
        TestEqual(TEXT("Edited Amortos switch 1cc float"), Amortos.AmortosSwitch1CC.Camera.Field8, 5.15f);
        TestEqual(TEXT("Edited Amortos switch 1d4 camera"), Amortos.AmortosSwitch1D4.Camera.CameraIndex, 14);
        TestEqual(TEXT("Edited Amortos switch 1d4 float"), Amortos.AmortosSwitch1D4.Camera.FieldC, 6.15f);
        if (!Amortos.AmortosCheckpointActors.IsEmpty()) TestEqual(TEXT("Edited Amortos checkpoint actor"), Amortos.AmortosCheckpointActors[0], 703);
        if (!Amortos.AmortosActors188.IsEmpty()) TestEqual(TEXT("Edited Amortos actor 188"), Amortos.AmortosActors188[0], 704);
        if (!Amortos.AmortosActors18C.IsEmpty()) TestEqual(TEXT("Edited Amortos actor 18c"), Amortos.AmortosActors18C[0], 705);
        if (!Amortos.AmortosSwitch1CC.Targets.IsEmpty()) TestEqual(TEXT("Edited Amortos switch 1cc target"), Amortos.AmortosSwitch1CC.Targets[0].TargetRef, 706);
        if (!Amortos.AmortosSwitch1D4.Targets.IsEmpty()) TestEqual(TEXT("Edited Amortos switch 1d4 target"), Amortos.AmortosSwitch1D4.Targets[0].TargetRef, 707);
        TestEqual(TEXT("Edited Amortos cinematic behaviour"),
            Reimported->SceneEnvironment.Actors[AmortosBehaviourActor].Behaviours[AmortosBehaviour].CinematicLeaveBehaviourId, 708);
    }
    {
        const FKyaActorPlacement& Teleporter = Reimported->SceneEnvironment.Actors[TeleporterActor];
        TestEqual(TEXT("Edited teleporter subsector material"), Teleporter.TeleporterSubsectorMaterialId, 801);
        TestEqual(TEXT("Edited teleporter hash"), Teleporter.TeleporterHash, int64(0x123456789abcdef0ull));
        TestEqual(TEXT("Edited teleporter field 16c"), Teleporter.TeleporterField16C, 802);
        TestEqual(TEXT("Edited teleporter zone 188"), Teleporter.TeleporterZone188, 803);
        TestEqual(TEXT("Edited teleporter zone 18c"), Teleporter.TeleporterZone18C, 804);
        TestEqual(TEXT("Edited teleporter material 1"), Teleporter.TeleporterMaterialId1, 805);
        TestEqual(TEXT("Edited teleporter cinematic"), Teleporter.TeleporterCinematicId, 806);
        TestEqual(TEXT("Edited teleporter material 2"), Teleporter.TeleporterMaterialId2, 807);
        TestEqual(TEXT("Edited teleporter field 1d4"), Teleporter.TeleporterField1D4, 808);
        TestEqual(TEXT("Edited teleporter field 1d8"), Teleporter.TeleporterField1D8, 809);
        TestEqual(TEXT("Edited teleporter material 3"), Teleporter.TeleporterMaterialId3, 810);
        TestEqual(TEXT("Edited teleporter field 1e0"), Teleporter.TeleporterField1E0, 811);
        TestEqual(TEXT("Edited teleporter switch 170 camera"), Teleporter.TeleporterSwitch170.Camera.CameraIndex, 15);
        TestEqual(TEXT("Edited teleporter switch 170 float"), Teleporter.TeleporterSwitch170.Camera.Field8, 7.15f);
        TestEqual(TEXT("Edited teleporter switch 178 camera"), Teleporter.TeleporterSwitch178.Camera.CameraIndex, 16);
        TestEqual(TEXT("Edited teleporter switch 180 camera"), Teleporter.TeleporterSwitch180.Camera.CameraIndex, 17);
        if (!Teleporter.TeleporterDestinations.IsEmpty())
        {
            TestEqual(TEXT("Edited teleporter destination level"), Teleporter.TeleporterDestinations[0].LevelId, 812);
            TestEqual(TEXT("Edited teleporter destination elevator"), Teleporter.TeleporterDestinations[0].ElevatorId, 813);
            TestEqual(TEXT("Edited teleporter destination field"), Teleporter.TeleporterDestinations[0].Field8, 814);
        }
        if (!Teleporter.TeleporterSwitch170.Targets.IsEmpty()) TestEqual(TEXT("Edited teleporter switch 170 target"), Teleporter.TeleporterSwitch170.Targets[0].TargetRef, 815);
        if (!Teleporter.TeleporterSwitch178.Targets.IsEmpty()) TestEqual(TEXT("Edited teleporter switch 178 target"), Teleporter.TeleporterSwitch178.Targets[0].TargetRef, 816);
        if (!Teleporter.TeleporterSwitch180.Targets.IsEmpty()) TestEqual(TEXT("Edited teleporter switch 180 target"), Teleporter.TeleporterSwitch180.Targets[0].TargetRef, 817);
    }
    {
        const FKyaActorPlacement& Dca = Reimported->SceneEnvironment.Actors[DcaActor];
        TestEqual(TEXT("Edited DCA field 504"), Dca.DcaField504, 901);
        TestEqual(TEXT("Edited DCA field 500"), Dca.DcaField500, 902);
        TestEqual(TEXT("Edited DCA field 464"), Dca.DcaField464, 1.55f);
        TestEqual(TEXT("Edited DCA field 508"), Dca.DcaField508, 903);
        TestEqual(TEXT("Edited DCA field 50c"), Dca.DcaField50C, 904);
        TestEqual(TEXT("Edited DCA aim adjust"), Dca.DcaAimAdjustDegrees, 2.55f);
        TestEqual(TEXT("Edited DCA field 450"), Dca.DcaField450, 905);
        TestEqual(TEXT("Edited DCA field 454"), Dca.DcaField454, 906);
        if (!Dca.DcaShotActorRefs.IsEmpty()) TestEqual(TEXT("Edited DCA shot actor"), Dca.DcaShotActorRefs[0], 907);
        const FKyaActorBehaviour& Controlled = Reimported->SceneEnvironment.Actors[DcaControlledActor].Behaviours[DcaControlled];
        TestEqual(TEXT("Edited DCA controlled field 0"), Controlled.DcaControlledField0, 3.55f);
        TestEqual(TEXT("Edited DCA controlled field 4"), Controlled.DcaControlledField4, 4.55f);
        TestEqual(TEXT("Edited DCA controlled field 8"), Controlled.DcaControlledField8, 5.55f);
        TestEqual(TEXT("Edited DCA controlled fov"), Controlled.DcaControlledFovDegrees, 6.55f);
        TestEqual(TEXT("Edited DCA controlled vector 10"), Controlled.DcaControlledVector10, FVector(7.55f, 8.55f, 9.55f));
        TestEqual(TEXT("Edited DCA controlled vector 20"), Controlled.DcaControlledVector20, FVector(10.55f, 11.55f, 12.55f));
        const FKyaActorBehaviour& Trajectory = Reimported->SceneEnvironment.Actors[DcaTrajectoryActor].Behaviours[DcaTrajectory];
        TestEqual(TEXT("Edited DCA trajectory actor"), Trajectory.DcaTrajectoryActorRef, 908);
        TestEqual(TEXT("Edited DCA trajectory field 18"), Trajectory.DcaTrajectoryField18, 13.55f);
        TestEqual(TEXT("Edited DCA trajectory field 8"), Trajectory.DcaTrajectoryField8, 909);
    }
    {
        const FKyaActorPlacement& Box = Reimported->SceneEnvironment.Actors[BoxClassActor];
        TestEqual(TEXT("Edited box weight"), Box.BoxWeightB, 0.45f);
        TestEqual(TEXT("Edited box life"), Box.BoxLifeMax, 70.0f);
        TestEqual(TEXT("Edited box gravity"), Box.BoxGravityScale, 2.45f);
        TestEqual(TEXT("Edited box dynamic ext 4"), Box.BoxDynamicExtField4, 3.45f);
        TestEqual(TEXT("Edited box dynamic ext 8"), Box.BoxDynamicExtField8, 4.45f);
        TestEqual(TEXT("Edited box field 350"), Box.BoxField350, 1001);
        TestEqual(TEXT("Edited box field 354"), Box.BoxField354, 5.45f);
        TestEqual(TEXT("Edited box field 358"), Box.BoxField358, 6.45f);
        TestEqual(TEXT("Edited box field 35c"), Box.BoxField35C, 7.45f);
        TestEqual(TEXT("Edited box field 360"), Box.BoxField360, 8.45f);
        TestEqual(TEXT("Edited box field 364"), Box.BoxField364, 1002);
        TestEqual(TEXT("Edited box field 370"), Box.BoxField370, 1003);
        TestEqual(TEXT("Edited box field 374"), Box.BoxField374, 1004);
        TestEqual(TEXT("Edited box field 378"), Box.BoxField378, 1005);
        TestEqual(TEXT("Edited box field 37c"), Box.BoxField37C, 1006);
        TestEqual(TEXT("Edited box switch camera"), Box.BoxSwitch.Camera.CameraIndex, 18);
        TestEqual(TEXT("Edited box switch float"), Box.BoxSwitch.Camera.Field8, 9.45f);
        TestEqual(TEXT("Edited box add-on money type"), Box.BoxAddOn.MoneyType, 3);
        TestEqual(TEXT("Edited box add-on bonus count"), Box.BoxAddOn.BonusCount, 5);
        if (!Box.BoxActorRefs.IsEmpty()) TestEqual(TEXT("Edited box actor"), Box.BoxActorRefs[0], 1007);
        if (!Box.BoxSwitch.Targets.IsEmpty()) TestEqual(TEXT("Edited box target"), Box.BoxSwitch.Targets[0].TargetRef, 1008);
    }
    {
        const FKyaActorPlacement& Money = Reimported->SceneEnvironment.Actors[MoneyActor];
        TestEqual(TEXT("Edited money weight"), Money.MoneyWeightB, 0.35f);
        TestEqual(TEXT("Edited money value"), Money.MoneyValue, 1101);
        TestEqual(TEXT("Edited money sound"), Money.MoneySoundRef, 1102);
        TestEqual(TEXT("Edited money material"), Money.MoneyMaterialId, 1103);
    }
    {
        const FKyaActorPlacement& Fog = Reimported->SceneEnvironment.Actors[FogActor];
        if (!Fog.FogHierarchies.IsEmpty())
        {
            TestEqual(TEXT("Edited fog hash"), Fog.FogHierarchies[0].Hash, int64(0x0fedcba987654321ull));
            TestEqual(TEXT("Edited fog field 20"), Fog.FogHierarchies[0].Field20, 7.05f);
            TestEqual(TEXT("Edited fog field 24"), Fog.FogHierarchies[0].Field24, 8.05f);
        }
    }
    {
        const FKyaActorPlacement& Shoot = Reimported->SceneEnvironment.Actors[ShootActor];
        TestEqual(TEXT("Edited shooter weight"), Shoot.ShootWeightB, 0.25f);
        TestEqual(TEXT("Edited shooter life"), Shoot.ShootLifeMax, 80.0f);
        TestEqual(TEXT("Edited shooter gravity"), Shoot.ShootGravityScale, 2.35f);
        TestEqual(TEXT("Edited shooter dynamic ext 4"), Shoot.ShootDynamicExtField4, 3.35f);
        TestEqual(TEXT("Edited shooter dynamic ext 8"), Shoot.ShootDynamicExtField8, 4.35f);
        TestEqual(TEXT("Edited shooter field 350"), Shoot.ShootField350, 1201);
        TestEqual(TEXT("Edited shooter field 354"), Shoot.ShootField354, 1202);
        TestEqual(TEXT("Edited shooter texture"), Shoot.ShootTextureIndex, 1203);
        TestEqual(TEXT("Edited shooter mesh"), Shoot.ShootMeshIndex, 1204);
        TestEqual(TEXT("Edited shooter field 3c0"), Shoot.ShootField3C0, 5.35f);
        TestEqual(TEXT("Edited shooter field 3cc"), Shoot.ShootField3CC, 8.35f);
        TestEqual(TEXT("Edited shooter field 3d0"), Shoot.ShootField3D0, 1205);
        TestEqual(TEXT("Edited shooter field 3d4"), Shoot.ShootField3D4, 9.35f);
        TestEqual(TEXT("Edited shooter field 3e0"), Shoot.ShootField3E0, 12.35f);
        TestEqual(TEXT("Edited shooter field 3e8"), Shoot.ShootField3E8, 1206);
        TestEqual(TEXT("Edited shooter field 3e4"), Shoot.ShootField3E4, 1207);
        TestEqual(TEXT("Edited shooter field 3ec"), Shoot.ShootField3EC, 1208);
        TestEqual(TEXT("Edited shooter field 3f0"), Shoot.ShootField3F0, 1209);
        TestEqual(TEXT("Edited shooter add-on money type"), Shoot.ShootAddOn.MoneyType, 2);
        TestEqual(TEXT("Edited shooter add-on bonus count"), Shoot.ShootAddOn.BonusCount, 4);
        const FKyaActorBehaviour& Behaviour = Reimported->SceneEnvironment.Actors[ShootFireActor].Behaviours[ShootFireBehaviour];
        TestEqual(TEXT("Edited shooter fire field 2b0"), Behaviour.ShootFireField2B0, 1210);
        TestEqual(TEXT("Edited shooter fire field 8"), Behaviour.ShootFireField8, 13.35f);
        TestEqual(TEXT("Edited shooter fire field 2b4"), Behaviour.ShootFireField2B4, 14.35f);
        if (!Behaviour.ShootFireActorRefs.IsEmpty()) TestEqual(TEXT("Edited shooter fire actor"), Behaviour.ShootFireActorRefs[0], 1211);
    }
    {
        const FKyaActorPlacement& Ambre = Reimported->SceneEnvironment.Actors[AmbreActor];
        TestEqual(TEXT("Edited Ambre material"), Ambre.AmbreMaterialId, 1301);
        TestEqual(TEXT("Edited Ambre effect type"), Ambre.AmbreEffectType, 1302);
        TestEqual(TEXT("Edited Ambre texture"), Ambre.AmbreTextureIndex, 1303);
        TestEqual(TEXT("Edited Ambre mesh"), Ambre.AmbreMeshIndex, 1304);
        TestEqual(TEXT("Edited Ambre field 1d0"), Ambre.AmbreField1D0, 1305);
        TestEqual(TEXT("Edited Ambre switch camera"), Ambre.AmbreSwitch.Camera.CameraIndex, 19);
        TestEqual(TEXT("Edited Ambre switch float"), Ambre.AmbreSwitch.Camera.Field8, 15.35f);
    }
    {
        const FKyaActorPlacement& Stiller = Reimported->SceneEnvironment.Actors[StillerActor];
        TestEqual(TEXT("Edited Stiller vision range"), Stiller.StillerVisionRange, 1601.0f);
        TestEqual(TEXT("Edited Stiller vision half angle"), Stiller.StillerVisionHalfAngle, 1602.0f);
        TestEqual(TEXT("Edited Stiller vision field 38"), Stiller.StillerVisionField38, 1.65f);
        TestEqual(TEXT("Edited Stiller vision field 3c"), Stiller.StillerVisionField3C, 2.65f);
        TestEqual(TEXT("Edited Stiller vision flags"), Stiller.StillerVisionFlags, 1603);
        TestEqual(TEXT("Edited Stiller field 1d0"), Stiller.StillerField1D0, 1604);
        TestEqual(TEXT("Edited Stiller field 1d4"), Stiller.StillerField1D4, 1605);
        TestEqual(TEXT("Edited Stiller field 1d8"), Stiller.StillerField1D8, 1606);
        TestEqual(TEXT("Edited Stiller field 1dc"), Stiller.StillerField1DC, 1607);
        TestEqual(TEXT("Edited Stiller path"), Stiller.StillerPathIndex, 1608);
        TestEqual(TEXT("Edited Stiller field 1e0"), Stiller.StillerField1E0, 3.65f);
        TestEqual(TEXT("Edited Stiller life"), Stiller.StillerLifeMax, 4.65f);
        TestEqual(TEXT("Edited Stiller field 1ec"), Stiller.StillerField1EC, 5.65f);
        TestEqual(TEXT("Edited Stiller field 1f0"), Stiller.StillerField1F0, 6.65f);
        TestEqual(TEXT("Edited Stiller field 1f4"), Stiller.StillerField1F4, 7.65f);
        TestEqual(TEXT("Edited Stiller field 1f8"), Stiller.StillerField1F8, 1609);
        TestEqual(TEXT("Edited Stiller field 200"), Stiller.StillerField200, 1610);
        TestEqual(TEXT("Edited Stiller field 214"), Stiller.StillerField214, 1611);
        TestEqual(TEXT("Edited Stiller switch 208 camera"), Stiller.StillerSwitch208.Camera.CameraIndex, 20);
        TestEqual(TEXT("Edited Stiller switch 208 float"), Stiller.StillerSwitch208.Camera.Field8, 8.65f);
        TestEqual(TEXT("Edited Stiller switch 210 camera"), Stiller.StillerSwitch210.Camera.CameraIndex, 21);
        TestEqual(TEXT("Edited Stiller add-on money type"), Stiller.StillerAddOn.MoneyType, 2);
        TestEqual(TEXT("Edited Stiller add-on bonus count"), Stiller.StillerAddOn.BonusCount, 4);
        if (!Stiller.StillerActorRefs.IsEmpty()) TestEqual(TEXT("Edited Stiller actor"), Stiller.StillerActorRefs[0], 1612);
        if (!Stiller.StillerSwitch208.Targets.IsEmpty()) TestEqual(TEXT("Edited Stiller target"), Stiller.StillerSwitch208.Targets[0].TargetRef, 1613);
    }
    {
        const FKyaActorPlacement& Rune = Reimported->SceneEnvironment.Actors[RuneActor];
        TestEqual(TEXT("Edited rune cinematic"), Rune.RuneCinematicId, 1701);
        TestEqual(TEXT("Edited rune trigger"), Rune.RuneTriggerDistance, 1.75f);
        TestEqual(TEXT("Edited rune scen var"), Rune.RuneScenVarId, 1702);
        TestEqual(TEXT("Edited rune fx"), Rune.RuneFxId, 1703);
        TestEqual(TEXT("Edited rune material A"), Rune.RuneMaterialIdA, 1704);
        TestEqual(TEXT("Edited rune material B"), Rune.RuneMaterialIdB, 1705);
        const FKyaActorPlacement& Fountain = Reimported->SceneEnvironment.Actors[FountainActor];
        TestEqual(TEXT("Edited fountain money type"), Fountain.BonusFountainAddOn.MoneyType, 2);
        TestEqual(TEXT("Edited fountain bonus count"), Fountain.BonusFountainAddOn.BonusCount, 4);
        const FKyaActorPlacement& Rope = Reimported->SceneEnvironment.Actors[RopeActor];
        TestEqual(TEXT("Edited rope path"), Rope.RopePathIndex, 1801);
        TestEqual(TEXT("Edited rope scale"), Rope.RopeScale, 2.75f);
        TestEqual(TEXT("Edited rope field 17c"), Rope.RopeField17C, 3.75f);
        TestEqual(TEXT("Edited rope material"), Rope.RopeMaterialId, 1802);
        TestEqual(TEXT("Edited rope color"), Rope.RopeColor, 0x11223344);
        TestEqual(TEXT("Edited rope sound"), Rope.RopeSoundRef, 1803);
        TestEqual(TEXT("Edited rope field 194"), Rope.RopeField194, 1804);
        TestEqual(TEXT("Edited rope actor A"), Rope.RopeActorRefA, 1805);
        TestEqual(TEXT("Edited rope actor B"), Rope.RopeActorRefB, 1806);
        TestEqual(TEXT("Edited rope flags"), Rope.RopeFlags, 0x2345);
        TestEqual(TEXT("Edited rope switch camera"), Rope.RopeSwitch.Camera.CameraIndex, 22);
        TestEqual(TEXT("Edited rope switch float"), Rope.RopeSwitch.Camera.Field8, 4.75f);
        if (!Rope.RopeSwitch.Targets.IsEmpty()) TestEqual(TEXT("Edited rope target"), Rope.RopeSwitch.Targets[0].TargetRef, 1807);
    }
    {
        const FKyaActorPlacement& Gravity = Reimported->SceneEnvironment.Actors[GravityActor];
        TestEqual(TEXT("Edited gravity weight"), Gravity.GravityWeightB, 0.55f);
        TestEqual(TEXT("Edited gravity life"), Gravity.GravityLifeMax, 90.0f);
        TestEqual(TEXT("Edited gravity scale"), Gravity.GravityScale, 2.95f);
        TestEqual(TEXT("Edited gravity dynamic ext 4"), Gravity.GravityDynamicExtField4, 3.95f);
        TestEqual(TEXT("Edited gravity dynamic ext 8"), Gravity.GravityDynamicExtField8, 4.95f);
        const FKyaActorBehaviour& Fall = Reimported->SceneEnvironment.Actors[GravityActor].Behaviours[GravityFallBehaviour];
        TestEqual(TEXT("Edited gravity fall sound"), Fall.GravityFallSoundRef, 1901);
        const FKyaActorBehaviour& Bell = Reimported->SceneEnvironment.Actors[GravityActor].Behaviours[GravityBellBehaviour];
        TestEqual(TEXT("Edited gravity bell sound"), Bell.GravityBellSoundRef, 1902);
        TestEqual(TEXT("Edited gravity bell field c"), Bell.GravityBellFieldC, 5.95f);
        TestEqual(TEXT("Edited gravity bell camera"), Bell.GravityBellSwitch.Camera.CameraIndex, 23);
        TestEqual(TEXT("Edited gravity bell camera float"), Bell.GravityBellSwitch.Camera.Field8, 6.95f);
        if (!Bell.GravityBellSwitch.Targets.IsEmpty()) TestEqual(TEXT("Edited gravity bell target"), Bell.GravityBellSwitch.Targets[0].TargetRef, 1903);
        const FKyaActorPlacement& Command = Reimported->SceneEnvironment.Actors[NativCmdActor];
        TestEqual(TEXT("Edited nativ command field 164"), Command.NativCmdField164, 2001);
        if (!Command.NativCmdActorRefs.IsEmpty()) TestEqual(TEXT("Edited nativ command actor"), Command.NativCmdActorRefs[0], 2002);
    }
    {
        const FKyaActorPlacement& Companion = Reimported->SceneEnvironment.Actors[CompanionActor];
        TestEqual(TEXT("Edited companion weight"), Companion.CompanionWeightB, 0.45f);
        TestEqual(TEXT("Edited companion life"), Companion.CompanionLifeMax, 100.0f);
        TestEqual(TEXT("Edited companion gravity"), Companion.CompanionGravityScale, 3.05f);
        TestEqual(TEXT("Edited companion dynamic ext 4"), Companion.CompanionDynamicExtField4, 4.05f);
        TestEqual(TEXT("Edited companion dynamic ext 8"), Companion.CompanionDynamicExtField8, 5.05f);
        TestEqual(TEXT("Edited companion field 358"), Companion.CompanionField358, 2101);
        TestEqual(TEXT("Edited companion material"), Companion.CompanionMaterial35C, 2102);
        TestEqual(TEXT("Edited companion field 360"), Companion.CompanionField360, 2103);
        const FKyaActorPlacement& Bridge = Reimported->SceneEnvironment.Actors[BridgeActor];
        TestEqual(TEXT("Edited bridge waypoint A"), Bridge.BridgeWaypointA, 2201);
        TestEqual(TEXT("Edited bridge waypoint B"), Bridge.BridgeWaypointB, 2202);
        TestEqual(TEXT("Edited bridge field 330"), Bridge.BridgeField330, 1.05f);
        TestEqual(TEXT("Edited bridge field 334"), Bridge.BridgeField334, 2.05f);
        TestEqual(TEXT("Edited bridge field 33c"), Bridge.BridgeField33C, 3.05f);
        TestEqual(TEXT("Edited bridge field 340"), Bridge.BridgeField340, 4.05f);
        TestEqual(TEXT("Edited bridge field 338"), Bridge.BridgeField338, 5.05f);
        TestEqual(TEXT("Edited bridge field 344"), Bridge.BridgeField344, 2203);
        TestEqual(TEXT("Edited bridge config"), Bridge.BridgeConfigFlags, 0x1234);
        TestEqual(TEXT("Edited bridge field 348"), Bridge.BridgeField348, 2204);
        TestEqual(TEXT("Edited bridge step scale"), Bridge.BridgeStepCollisionScale, 6.05f);
        if (!Bridge.BridgeActorRefs.IsEmpty()) TestEqual(TEXT("Edited bridge actor"), Bridge.BridgeActorRefs[0], 2205);
        const FKyaActorBehaviour& Basic = Reimported->SceneEnvironment.Actors[BridgeActor].Behaviours[BridgeBasicBehaviour];
        TestEqual(TEXT("Edited bridge basic direction"), Basic.BridgeBasicDirection, FVector(7.05f, 8.05f, 9.05f));
        TestEqual(TEXT("Edited bridge basic field 40"), Basic.BridgeBasicField40, 10.05f);
        TestEqual(TEXT("Edited bridge basic field 50"), Basic.BridgeBasicField50, 14.05f);
    }
    {
        const FKyaActorPlacement& Aton = Reimported->SceneEnvironment.Actors[AtonActor];
        TestEqual(TEXT("Edited Aton weight"), Aton.AtonWeightB, 0.65f);
        TestEqual(TEXT("Edited Aton life"), Aton.AtonLifeMax, 110.0f);
        TestEqual(TEXT("Edited Aton gravity"), Aton.AtonGravityScale, 3.15f);
        TestEqual(TEXT("Edited Aton dynamic ext 4"), Aton.AtonDynamicExtField4, 4.15f);
        TestEqual(TEXT("Edited Aton dynamic ext 8"), Aton.AtonDynamicExtField8, 5.15f);
        TestEqual(TEXT("Edited Aton field 354"), Aton.AtonField354, 2301);
        TestEqual(TEXT("Edited Aton field 358"), Aton.AtonField358, 2302);
        TestEqual(TEXT("Edited Aton field 35c"), Aton.AtonField35C, 2303);
        TestEqual(TEXT("Edited Aton field 360"), Aton.AtonField360, 2304);
        TestEqual(TEXT("Edited Aton effect type"), Aton.AtonEffectType, 2305);
        TestEqual(TEXT("Edited Aton texture"), Aton.AtonTextureIndex, 2306);
        TestEqual(TEXT("Edited Aton mesh"), Aton.AtonMeshIndex, 2307);
        TestEqual(TEXT("Edited Aton trail material"), Aton.AtonTrailMaterialId, 2308);
        TestEqual(TEXT("Edited Aton path delay"), Aton.AtonPathDefaultDelay, 6.15f);
        TestEqual(TEXT("Edited Aton add-on field 18"), Aton.AtonAddOnField18, 2309);
        TestEqual(TEXT("Edited Aton add-on field 24"), Aton.AtonAddOnField24, 7.15f);
        if (!Aton.AtonPathIndices.IsEmpty()) TestEqual(TEXT("Edited Aton path index"), Aton.AtonPathIndices[0], 2310);
        if (!Aton.AtonOtherSubObjs.IsEmpty())
        {
            TestEqual(TEXT("Edited Aton other sub-object field 0"), Aton.AtonOtherSubObjs[0].Field0, 2311);
            if (!Aton.AtonOtherSubObjs[0].CinematicIds.IsEmpty())
                TestEqual(TEXT("Edited Aton other sub-object cinematic"), Aton.AtonOtherSubObjs[0].CinematicIds[0], 2312);
        }
        TestEqual(TEXT("Edited Aton cinematic behaviour"),
            Reimported->SceneEnvironment.Actors[AtonActor].Behaviours[AtonBehaviour].CinematicLeaveBehaviourId, 2313);
    }
    {
        const FKyaActorPlacement& Boomy = Reimported->SceneEnvironment.Actors[BoomyActor];
        TestEqual(TEXT("Edited Boomy weight"), Boomy.BoomyWeight, 0.75f);
        TestEqual(TEXT("Edited Boomy vision range"), Boomy.BoomyVisionRange, 2401.0f);
        TestEqual(TEXT("Edited Boomy vision angle"), Boomy.BoomyVisionAngle, 2402.0f);
        TestEqual(TEXT("Edited Boomy vision field 38"), Boomy.BoomyVisionField38, 1.25f);
        TestEqual(TEXT("Edited Boomy vision field 3c"), Boomy.BoomyVisionField3C, 2.25f);
        TestEqual(TEXT("Edited Boomy vision flags"), Boomy.BoomyVisionFlags, 2403);
        TestEqual(TEXT("Edited Boomy sound 2b8"), Boomy.BoomySound2B8, 2404);
        TestEqual(TEXT("Edited Boomy sound 2bc"), Boomy.BoomySound2BC, 2405);
        TestEqual(TEXT("Edited Boomy field 744"), Boomy.BoomyField744, 3.25f);
        TestEqual(TEXT("Edited Boomy tail width axis"), Boomy.BoomyTailWidthAxis, 2406);
        TestEqual(TEXT("Edited Boomy field 740"), Boomy.BoomyField740, 4.25f);
        TestEqual(TEXT("Edited Boomy tail segments"), Boomy.BoomyTailSegments, 2407);
        TestEqual(TEXT("Edited Boomy particle 3d0"), Boomy.BoomyParticle3D0, 2408);
        TestEqual(TEXT("Edited Boomy particle 3d4"), Boomy.BoomyParticle3D4, 2409);
        TestEqual(TEXT("Edited Boomy particle 3dc"), Boomy.BoomyParticle3DC, 2410);
        TestEqual(TEXT("Edited Boomy field 3cc"), Boomy.BoomyField3CC, 2411);
        TestEqual(TEXT("Edited Boomy texture"), Boomy.BoomyTextureIndex, 2412);
        TestEqual(TEXT("Edited Boomy mesh"), Boomy.BoomyMeshIndex, 2413);
        const FKyaActorCamera& Camera = Boomy.BoomyCamera;
        TestEqual(TEXT("Edited Boomy camera fov"), Camera.Fov, 5.25f);
        TestEqual(TEXT("Edited Boomy camera position"), Camera.Position, FVector(6.25f, 7.25f, 8.25f));
        TestEqual(TEXT("Edited Boomy camera look-at"), Camera.LookAt, FVector(9.25f, 10.25f, 11.25f));
        TestEqual(TEXT("Edited Boomy camera up"), Camera.Up, FVector(12.25f, 13.25f, 14.25f));
        TestEqual(TEXT("Edited Boomy camera sector"), Camera.SectorId, 2414);
        TestEqual(TEXT("Edited Boomy camera field 08"), Camera.Field08, 2415);
        TestEqual(TEXT("Edited Boomy camera flags"), Camera.Flags, 2416);
        TestEqual(TEXT("Edited Boomy camera target offset"), Camera.TargetOffset, FVector(15.25f, 16.25f, 17.25f));
        TestEqual(TEXT("Edited Boomy config flags"), Camera.ConfigFlags, 2417);
        TestEqual(TEXT("Edited Boomy config actor A"), Camera.ConfigActorRefA, 2418);
        TestEqual(TEXT("Edited Boomy config actor B"), Camera.ConfigActorRefB, 2419);
        TestEqual(TEXT("Edited Boomy config angle"), Camera.ConfigAngle, 18.25f);
        TestEqual(TEXT("Edited Boomy pitch angles"), Camera.PitchAngles, FVector(19.25f, 20.25f, 21.25f));
        TestEqual(TEXT("Edited Boomy other angles"), Camera.OtherAngles, FVector(22.25f, 23.25f, 24.25f));
        TestEqual(TEXT("Edited Boomy distances"), Camera.Distances, FVector(25.25f, 26.25f, 27.25f));
        if (!Camera.Conditions.IsEmpty())
        {
            TestEqual(TEXT("Edited Boomy camera condition combiner"), Camera.Conditions[0].Combiner, 2420);
            TestEqual(TEXT("Edited Boomy camera condition actor"), Camera.Conditions[0].ActorRef, 2421);
            TestEqual(TEXT("Edited Boomy camera condition type"), Camera.Conditions[0].Type, 2422);
            TestEqual(TEXT("Edited Boomy camera condition value"), Camera.Conditions[0].Value, 28.25f);
        }
        if (!Boomy.BoomyTypes.IsEmpty())
        {
            TestEqual(TEXT("Edited Boomy type field 4"), Boomy.BoomyTypes[0].Field4, 29.25f);
            TestEqual(TEXT("Edited Boomy type vision range"), Boomy.BoomyTypes[0].VisionRange, 30.25f);
            TestEqual(TEXT("Edited Boomy type hit damage"), Boomy.BoomyTypes[0].HitDamage, 2423);
            TestEqual(TEXT("Edited Boomy type fx colour"), Boomy.BoomyTypes[0].FxColorA, 2424);
        }
    }
    {
        const FKyaActorPlacement& Kim = Reimported->SceneEnvironment.Actors[KimActor];
        TestEqual(TEXT("Edited Kim fighter weight"), Kim.FighterWeightB, 0.85f);
        TestEqual(TEXT("Edited Kim fighter life"), Kim.FighterLifeMax, 120.0f);
        TestEqual(TEXT("Edited Kim fighter gravity"), Kim.FighterGravityScale, 2.05f);
        TestEqual(TEXT("Edited Kim fighter dynamic ext 4"), Kim.FighterDynamicExtField4, 3.05f);
        TestEqual(TEXT("Edited Kim fighter dynamic ext 8"), Kim.FighterDynamicExtField8, 4.05f);
        TestEqual(TEXT("Edited Kim fighter input flag"), Kim.FighterInputAnalyserFlag, 2600);
        TestEqual(TEXT("Edited Kim fighter command mask"), Kim.FighterCommandMask, 2601);
        TestEqual(TEXT("Edited Kim fighter field 7c0"), Kim.FighterField7C0, 9.45f);
        if (!Kim.FighterBlows.IsEmpty()) TestEqual(TEXT("Edited Kim blow field"), Kim.FighterBlows[0].Field4A, 2602);
        if (!Kim.FighterGrabs.IsEmpty()) TestEqual(TEXT("Edited Kim grab field"), Kim.FighterGrabs[0].Field4A, 2603);
        if (!Kim.FighterCombos.IsEmpty()) TestEqual(TEXT("Edited Kim combo field"), Kim.FighterCombos[0].Field4A, 2604);
        if (!Kim.FighterGrabReacts.IsEmpty()) TestEqual(TEXT("Edited Kim grab reaction field"), Kim.FighterGrabReacts[0].Field4, 2605);
        for (int32 Index = 0; Index < Kim.KimBoneKeys.Num(); ++Index)
            TestEqual(TEXT("Edited Kim bone key"), Kim.KimBoneKeys[Index], 2500 + Index);
        if (!Kim.KimClimbZones.IsEmpty()) TestEqual(TEXT("Edited Kim climb zone"), Kim.KimClimbZones[0], 2510);
        if (!Kim.KimTutorialZones.IsEmpty())
        {
            const FKyaHeroTutorialZone& Zone = Kim.KimTutorialZones[0];
            TestEqual(TEXT("Edited Kim tutorial zone"), Zone.ZoneRef, 2511);
            TestEqual(TEXT("Edited Kim tutorial message key"), Zone.MessageKey, int64(0x0123456789abcdefull));
            TestEqual(TEXT("Edited Kim tutorial flags"), Zone.Flags, 2512);
            TestEqual(TEXT("Edited Kim tutorial action"), Zone.Action, 2513);
            TestEqual(TEXT("Edited Kim tutorial duration"), Zone.Duration, 1.45f);
            if (!Zone.Targets.IsEmpty())
            {
                TestEqual(TEXT("Edited Kim tutorial target"), Zone.Targets[0].TargetRef, 2514);
                TestEqual(TEXT("Edited Kim tutorial cutscene"), Zone.Targets[0].CutsceneId, 2515);
                TestEqual(TEXT("Edited Kim tutorial target field 18"), Zone.Targets[0].Field18, 2516);
            }
            TestEqual(TEXT("Edited Kim tutorial camera"), Zone.Camera.CameraIndex, 24);
            TestEqual(TEXT("Edited Kim tutorial camera float"), Zone.Camera.Field8, 2.45f);
        }
        if (!Kim.KimProjectileZones.IsEmpty())
        {
            const FKyaHeroProjectileZone& Zone = Kim.KimProjectileZones[0];
            TestEqual(TEXT("Edited Kim projectile zone"), Zone.ZoneRef, 2517);
            TestEqual(TEXT("Edited Kim projectile type"), Zone.ProjectileType, 2518);
            TestEqual(TEXT("Edited Kim projectile damage"), Zone.Damage, 3.45f);
            TestEqual(TEXT("Edited Kim projectile waypoint"), Zone.WaypointIndex, 2519);
            TestEqual(TEXT("Edited Kim projectile force"), Zone.Force, 4.45f);
            TestEqual(TEXT("Edited Kim projectile samples"), Zone.SampleCount, 2520);
            TestEqual(TEXT("Edited Kim projectile sector"), Zone.SectorId, 2521);
        }
        if (!Kim.KimBoomyZonePairs.IsEmpty())
        {
            TestEqual(TEXT("Edited Kim boomy zone pair"), Kim.KimBoomyZonePairs[0].ZoneRef, 2522);
            TestEqual(TEXT("Edited Kim boomy zone pair field 4"), Kim.KimBoomyZonePairs[0].Field4, 2523);
        }
        if (!Kim.KimZoneStream.IsEmpty()) TestEqual(TEXT("Edited Kim zone stream"), Kim.KimZoneStream[0], 2524);
        TestEqual(TEXT("Edited Kim material"), Kim.KimMaterialId, 2525);
        TestEqual(TEXT("Edited Kim fx type"), Kim.KimFxType, 2526);
        TestEqual(TEXT("Edited Kim joke radius"), Kim.KimJokeWarnRadius, 5.45f);
        if (!Kim.KimCameras.IsEmpty())
        {
            TestEqual(TEXT("Edited Kim camera fov"), Kim.KimCameras[0].Fov, 6.45f);
            TestEqual(TEXT("Edited Kim camera sector"), Kim.KimCameras[0].SectorId, 2527);
            TestEqual(TEXT("Edited Kim camera config angle"), Kim.KimCameras[0].ConfigAngle, 7.45f);
        }
        TestEqual(TEXT("Edited Kim field 15d4"), Kim.KimField15D4, 2528);
        TestEqual(TEXT("Edited Kim fx init"), Kim.KimFxInitType, 2529);
        for (int32 Index = 0; Index < Kim.KimFxHandleTypes.Num(); ++Index)
            TestEqual(TEXT("Edited Kim fx handle"), Kim.KimFxHandleTypes[Index], 2530 + Index);
    }
    {
        const FKyaActorPlacement& Companion = Reimported->SceneEnvironment.Actors[CompanionAlertActor];
        const int32 CompanionBehaviourIndex = Companion.Behaviours.IndexOfByPredicate(
            [](const FKyaActorBehaviour& Behaviour) { return Behaviour.Id == 3; });
        const FKyaActorBehaviour& Behaviour = Companion.Behaviours[CompanionBehaviourIndex];
        if (!Behaviour.CompanionAlerts.IsEmpty())
        {
            const FKyaCompanionAlert& Alert = Behaviour.CompanionAlerts[0];
            TestEqual(TEXT("Edited companion alert flags"), Alert.Flags, 2700);
            TestEqual(TEXT("Edited companion alert field 08"), Alert.Field08, 2701);
            TestEqual(TEXT("Edited companion alert field 10"), Alert.Field10, 2702);
            TestEqual(TEXT("Edited companion alert field 14"), Alert.Field14, 2703);
            TestEqual(TEXT("Edited companion alert field 18"), Alert.Field18, 1.55f);
            TestEqual(TEXT("Edited companion alert actor"), Alert.ActorRef, 2704);
            TestEqual(TEXT("Edited companion alert vector"), Alert.Field20, FVector(2.55f, 3.55f, 4.55f));
            TestEqual(TEXT("Edited companion alert camera"), Alert.CameraRef, 2705);
            TestEqual(TEXT("Edited companion alert string key"), Alert.LevelStringKey, int64(0x0fedcba987654321ull));
            TestEqual(TEXT("Edited companion alert field 50"), Alert.Field50, 5.55f);
            TestEqual(TEXT("Edited companion alert field 58"), Alert.Field58, 2706);
            TestEqual(TEXT("Edited companion alert field 5c"), Alert.Field5C, 6.55f);
            if (!Alert.Field34.IsEmpty()) TestEqual(TEXT("Edited companion alert list"), Alert.Field34[0], 2707);
            if (!Alert.TargetSwitch.Targets.IsEmpty()) TestEqual(TEXT("Edited companion alert target"), Alert.TargetSwitch.Targets[0].TargetRef, 2708);
            if (!Alert.Field70.IsEmpty() && !Alert.Field70[0].Operations.IsEmpty())
                TestEqual(TEXT("Edited companion alert operation"), Alert.Field70[0].Operations[0].Value, 2709);
        }
        if (Behaviour.CompanionAlerts.Num() > 1 && !Behaviour.CompanionAlerts[1].Conditions.IsEmpty())
        {
            TestEqual(TEXT("Edited companion condition combiner"), Behaviour.CompanionAlerts[1].Conditions[0].Combiner, 2710);
            TestEqual(TEXT("Edited companion condition variable"), Behaviour.CompanionAlerts[1].Conditions[0].Variable, 2711);
            TestEqual(TEXT("Edited companion condition comparison"), Behaviour.CompanionAlerts[1].Conditions[0].Comparison, 2712);
            TestEqual(TEXT("Edited companion condition value"), Behaviour.CompanionAlerts[1].Conditions[0].Value, 2713);
        }
    }
    {
        const FKyaActorPlacement& Actor = Reimported->SceneEnvironment.Actors[PlatformStandActor];
        const int32 PlatformBehaviourIndex = Actor.Behaviours.IndexOfByPredicate(
            [](const FKyaActorBehaviour& Behaviour) { return Behaviour.Id == 7; });
        TestEqual(TEXT("Edited platform stand sound"), Actor.Behaviours[PlatformBehaviourIndex].PlatformBehaviour.SoundRef, 2800);
        TestEqual(TEXT("Edited platform stand field 0c"), Actor.Behaviours[PlatformBehaviourIndex].PlatformBehaviour.Field0C, 2801);
    }
    {
        const FKyaActorPlacement& Actor = Reimported->SceneEnvironment.Actors[PlatformSelectorActor];
        const int32 PlatformTrajectoryBehaviourIndex = Actor.Behaviours.IndexOfByPredicate(
            [](const FKyaActorBehaviour& Behaviour) { return Behaviour.Id == 9; });
        const FKyaPlatformBehaviour& Values = Actor.Behaviours[PlatformTrajectoryBehaviourIndex].PlatformBehaviour;
        TestEqual(TEXT("Edited platform selector field 18"), Values.Field18, 2802);
        TestEqual(TEXT("Edited platform selector material"), Values.MaterialId, 2803);
        TestEqual(TEXT("Edited platform selector field 8"), Values.Field8, 1.85f);
        TestEqual(TEXT("Edited platform selector field c"), Values.FieldC, 2.85f);
        TestEqual(TEXT("Edited platform selector field 10"), Values.Field10, 3.85f);
        TestEqual(TEXT("Edited platform selector sound"), Values.SoundRef, 2804);
    }
    {
        const FKyaActorPlacement& Actor = Reimported->SceneEnvironment.Actors[PlatformTrajectoryActor];
        const int32 TrajectoryBehaviourIndex = Actor.Behaviours.IndexOfByPredicate(
            [](const FKyaActorBehaviour& Behaviour) { return Behaviour.Id == 2; });
        const FKyaPlatformBehaviour& Values = Actor.Behaviours[TrajectoryBehaviourIndex].PlatformBehaviour;
        TestEqual(TEXT("Edited platform trajectory path"), Values.PathIndex, 2805);
        TestEqual(TEXT("Edited platform trajectory timing"), Values.Timing, 2806);
        TestEqual(TEXT("Edited platform trajectory field 4"), Values.PathField4, 4.85f);
        TestEqual(TEXT("Edited platform trajectory total time"), Values.PathTotalTime, 5.85f);
        TestEqual(TEXT("Edited platform trajectory segment"), Values.SegmentStartTime, 6.85f);
        TestEqual(TEXT("Edited platform trajectory delay"), Values.LoopDelay, 7.85f);
    }
    {
        const FKyaActorPlacement& Actor = Reimported->SceneEnvironment.Actors[PlatformSlabActor];
        const int32 PlatformExtraBehaviourIndex = Actor.Behaviours.IndexOfByPredicate(
            [](const FKyaActorBehaviour& Behaviour) { return Behaviour.Id == 3; });
        const FKyaPlatformBehaviour& Values = Actor.Behaviours[PlatformExtraBehaviourIndex].PlatformBehaviour;
        TestEqual(TEXT("Edited platform slab field 10"), Values.SlabField10, 1.15f);
        TestEqual(TEXT("Edited platform slab field 14"), Values.SlabField14, 2.15f);
        TestEqual(TEXT("Edited platform slab field 18"), Values.SlabField18, 3.15f);
        TestEqual(TEXT("Edited platform slab field 1c"), Values.SlabField1C, 4.15f);
        TestEqual(TEXT("Edited platform slab field 20"), Values.SlabField20, 3100);
        TestEqual(TEXT("Edited platform slab camera"), Values.SlabSwitch.Camera.CameraIndex, 27);
        TestEqual(TEXT("Edited platform slab camera float"), Values.SlabSwitch.Camera.Field8, 5.15f);
        if (!Values.SlabSwitch.Targets.IsEmpty()) TestEqual(TEXT("Edited platform slab target"), Values.SlabSwitch.Targets[0].TargetRef, 3101);
    }
    {
        const FKyaActorPlacement& Actor = Reimported->SceneEnvironment.Actors[PlatformMasterActor];
        const int32 PlatformExtraBehaviourIndex = Actor.Behaviours.IndexOfByPredicate(
            [](const FKyaActorBehaviour& Behaviour) { return Behaviour.Id == 5; });
        const FKyaPlatformBehaviour& Values = Actor.Behaviours[PlatformExtraBehaviourIndex].PlatformBehaviour;
        TestEqual(TEXT("Edited platform master path"), Values.MasterPathIndex, 3102);
        TestEqual(TEXT("Edited platform master timing"), Values.MasterTiming, 3103);
        TestEqual(TEXT("Edited platform master stream actor"), Values.MasterStreamActor, 3104);
        TestEqual(TEXT("Edited platform master field 40"), Values.MasterField40, 6.15f);
        TestEqual(TEXT("Edited platform master field 44"), Values.MasterField44, 7.15f);
        TestEqual(TEXT("Edited platform master camera"), Values.AnalogCamera.CameraIndex, 28);
        TestEqual(TEXT("Edited platform master camera float"), Values.AnalogCamera.Field8, 8.15f);
        if (!Values.AnalogTargets.IsEmpty())
        {
            TestEqual(TEXT("Edited platform master analog actor"), Values.AnalogTargets[0].ActorRef, 3105);
            TestEqual(TEXT("Edited platform master analog float"), Values.AnalogTargets[0].Field20, 9.15f);
        }
    }
    {
        const FKyaActorPlacement& Actor = Reimported->SceneEnvironment.Actors[PlatformSelectorMasterActor];
        const int32 PlatformExtraBehaviourIndex = Actor.Behaviours.IndexOfByPredicate(
            [](const FKyaActorBehaviour& Behaviour) { return Behaviour.Id == 8; });
        const FKyaPlatformBehaviour& Values = Actor.Behaviours[PlatformExtraBehaviourIndex].PlatformBehaviour;
        TestEqual(TEXT("Edited platform selector-master field 18"), Values.Field18, 3106);
        TestEqual(TEXT("Edited platform selector-master material"), Values.MaterialId, 3107);
        TestEqual(TEXT("Edited platform selector-master field 8"), Values.Field8, 10.15f);
        TestEqual(TEXT("Edited platform selector-master field c"), Values.FieldC, 11.15f);
        TestEqual(TEXT("Edited platform selector-master field 10"), Values.Field10, 12.15f);
        TestEqual(TEXT("Edited platform selector-master sound"), Values.SoundRef, 3108);
        TestEqual(TEXT("Edited platform selector-master camera"), Values.SelectorCamera.CameraIndex, 29);
        TestEqual(TEXT("Edited platform selector-master camera float"), Values.SelectorCamera.Field8, 13.15f);
        if (!Values.SelectorActorRefs.IsEmpty()) TestEqual(TEXT("Edited platform selector-master actor"), Values.SelectorActorRefs[0], 3109);
        if (!Values.SelectorSwitchEx.IsEmpty()) TestEqual(TEXT("Edited platform selector-master target"), Values.SelectorSwitchEx[0].ActorRef, 3110);
    }
    {
        const FKyaActorPlacement& Actor = Reimported->SceneEnvironment.Actors[ProjectileStandActor];
        const int32 ProjectileBehaviourIndex = Actor.Behaviours.IndexOfByPredicate(
            [](const FKyaActorBehaviour& Behaviour) { return Behaviour.Id == 3; });
        const FKyaProjectileBehaviour& Values = Actor.Behaviours[ProjectileBehaviourIndex].ProjectileBehaviour;
        TestEqual(TEXT("Edited projectile stand behaviour id"), Values.BehaviourId, 2900);
        TestEqual(TEXT("Edited projectile stand field 0c"), Values.Field0C, 2901);
        TestEqual(TEXT("Edited projectile stand material"), Values.MaterialId, 2902);
    }
    {
        const FKyaActorPlacement& Actor = Reimported->SceneEnvironment.Actors[ProjectileStraightActor];
        const int32 ProjectileBehaviourIndex = Actor.Behaviours.IndexOfByPredicate(
            [](const FKyaActorBehaviour& Behaviour) { return Behaviour.Id == 4; });
        const FKyaProjectileBehaviour& Values = Actor.Behaviours[ProjectileBehaviourIndex].ProjectileBehaviour;
        TestEqual(TEXT("Edited projectile straight behaviour id"), Values.BehaviourId, 2903);
        TestEqual(TEXT("Edited projectile straight field 0c"), Values.Field0C, 2904);
        TestEqual(TEXT("Edited projectile straight speed"), Values.Speed, 1.95f);
        TestEqual(TEXT("Edited projectile straight acceleration"), Values.Acceleration, 2.95f);
        TestEqual(TEXT("Edited projectile straight field 18"), Values.Field18, 3.95f);
    }
    {
        const FKyaActorPlacement& Actor = Reimported->SceneEnvironment.Actors[SwitchLeverActor];
        const int32 SwitchBehaviourIndex = Actor.Behaviours.IndexOfByPredicate(
            [](const FKyaActorBehaviour& Behaviour) { return Behaviour.Id == 2; });
        TestEqual(TEXT("Edited switch lever A"), Actor.Behaviours[SwitchBehaviourIndex].SwitchBehaviour.LeverRefA, 2905);
        TestEqual(TEXT("Edited switch lever B"), Actor.Behaviours[SwitchBehaviourIndex].SwitchBehaviour.LeverRefB, 2906);
    }
    {
        const FKyaActorPlacement& Actor = Reimported->SceneEnvironment.Actors[SwitchMagicActor];
        const int32 SwitchBehaviourIndex = Actor.Behaviours.IndexOfByPredicate(
            [](const FKyaActorBehaviour& Behaviour) { return Behaviour.Id == 3; });
        const FKyaSwitchBehaviour& Values = Actor.Behaviours[SwitchBehaviourIndex].SwitchBehaviour;
        TestEqual(TEXT("Edited switch magic field 08"), Values.MagicField08, 4.95f);
        TestEqual(TEXT("Edited switch magic field 10"), Values.MagicField10, 5.95f);
        TestEqual(TEXT("Edited switch magic digits"), Values.MagicFxDigits, 2907);
        TestEqual(TEXT("Edited switch magic field 14"), Values.MagicField14, 6.95f);
        TestEqual(TEXT("Edited switch magic field 4c"), Values.MagicField4C, 2908);
        TestEqual(TEXT("Edited switch magic field 18"), Values.MagicField18, 2909);
        TestEqual(TEXT("Edited switch magic field 28"), Values.MagicField28, 2910);
        TestEqual(TEXT("Edited switch magic field 24"), Values.MagicField24, 2911);
        TestEqual(TEXT("Edited switch magic field 30"), Values.MagicField30, 2912);
        TestEqual(TEXT("Edited switch magic texture"), Values.OpenTextureId, 2913);
        TestEqual(TEXT("Edited switch magic mesh"), Values.OpenMeshId, 2914);
        TestEqual(TEXT("Edited switch magic camera"), Values.MagicCamera.CameraIndex, 25);
        TestEqual(TEXT("Edited switch magic camera float"), Values.MagicCamera.Field8, 7.95f);
    }
    {
        auto CheckWolfen = [this, &Reimported](int32 ActorIndex, int32 BehaviourId, auto&& Fn) {
            const FKyaActorPlacement& Actor = Reimported->SceneEnvironment.Actors[ActorIndex];
            const int32 BehaviourIndex = Actor.Behaviours.IndexOfByPredicate(
                [BehaviourId](const FKyaActorBehaviour& Behaviour) { return Behaviour.Id == BehaviourId; });
            Fn(Actor.Behaviours[BehaviourIndex].WolfenBehaviour);
        };
        CheckWolfen(WolfenTrackActor, 15, [this](const FKyaWolfenBehaviour& V) {
            TestEqual(TEXT("Edited Wolfen track flags"), V.Flags, 3000);
            TestEqual(TEXT("Edited Wolfen track field"), V.TrackField, 1.05f);
        });
        CheckWolfen(WolfenTrackWeaponActor, 17, [this](const FKyaWolfenBehaviour& V) {
            TestEqual(TEXT("Edited Wolfen track weapon flags"), V.Flags, 3001);
            TestEqual(TEXT("Edited Wolfen weapon field 90"), V.WeaponField90, 2.05f);
            TestEqual(TEXT("Edited Wolfen weapon field 94"), V.WeaponField94, 3002);
            TestEqual(TEXT("Edited Wolfen weapon field 98"), V.WeaponField98, 3003);
            TestEqual(TEXT("Edited Wolfen weapon trailing"), V.WeaponFieldF0, 3.05f);
            TestEqual(TEXT("Edited Wolfen weapon target field"), V.WeaponTargetField, 4.05f);
            if (!V.WeaponSubObjs.IsEmpty())
            {
                TestEqual(TEXT("Edited Wolfen weapon sub-object"), V.WeaponSubObjs[0].Field0, 3004);
                TestEqual(TEXT("Edited Wolfen weapon sub-object float"), V.WeaponSubObjs[0].AField4, 5.05f);
            }
        });
        CheckWolfen(WolfenTrackWeaponStandActor, 18, [this](const FKyaWolfenBehaviour& V) {
            TestEqual(TEXT("Edited Wolfen track weapon stand flags"), V.Flags, 3005);
            TestEqual(TEXT("Edited Wolfen weapon stand field 90"), V.WeaponField90, 6.05f);
            TestEqual(TEXT("Edited Wolfen weapon stand field 94"), V.WeaponField94, 3006);
            TestEqual(TEXT("Edited Wolfen weapon stand field 98"), V.WeaponField98, 3007);
            if (!V.WeaponSubObjs.IsEmpty()) TestEqual(TEXT("Edited Wolfen weapon stand sub-object"), V.WeaponSubObjs[0].Field10, 3008);
        });
        CheckWolfen(WolfenTrackWeaponSnipeActor, 19, [this](const FKyaWolfenBehaviour& V) {
            TestEqual(TEXT("Edited Wolfen track weapon snipe flags"), V.Flags, 3009);
            TestEqual(TEXT("Edited Wolfen weapon snipe trailing"), V.WeaponFieldF0, 7.05f);
        });
        CheckWolfen(WolfenDcaActor, 23, [this](const FKyaWolfenBehaviour& V) {
            TestEqual(TEXT("Edited Wolfen DCA flags"), V.Flags, 3010);
            TestEqual(TEXT("Edited Wolfen DCA actor"), V.DcaActorRef, 3011);
            TestEqual(TEXT("Edited Wolfen DCA cooldown"), V.DcaShootCooldown, 8.05f);
        });
        CheckWolfen(WolfenAvoidActor, 24, [this](const FKyaWolfenBehaviour& V) {
            TestEqual(TEXT("Edited Wolfen avoid flags"), V.Flags, 3012);
            TestEqual(TEXT("Edited Wolfen avoid field 80"), V.AvoidField80, 9.05f);
            TestEqual(TEXT("Edited Wolfen avoid field 84"), V.AvoidField84, 10.05f);
        });
        CheckWolfen(WolfenWatchDogActor, 8, [this](const FKyaWolfenBehaviour& V) {
            TestEqual(TEXT("Edited Wolfen watchdog flags"), V.Flags, 3020);
            TestEqual(TEXT("Edited Wolfen watchdog field 80"), V.Field80, 3021);
            TestEqual(TEXT("Edited Wolfen watchdog track"), V.TrackBehaviourId, 3022);
            TestEqual(TEXT("Edited Wolfen watchdog camera"), V.OnOffCamera.CameraIndex, 26);
            TestEqual(TEXT("Edited Wolfen watchdog camera float"), V.OnOffCamera.Field8, 11.05f);
            if (!V.OnOffTargets.IsEmpty()) TestEqual(TEXT("Edited Wolfen watchdog target"), V.OnOffTargets[0].TargetRef, 3023);
            if (!V.SwitchRecords.IsEmpty()) TestEqual(TEXT("Edited Wolfen watchdog switch"), V.SwitchRecords[0].BehaviourId, 3024);
        });
        CheckWolfen(WolfenGuardActor, 10, [this](const FKyaWolfenBehaviour& V) {
            TestEqual(TEXT("Edited Wolfen guard flags"), V.Flags, 3100);
            TestEqual(TEXT("Edited Wolfen guard path"), V.GuardPathIndex, 3101);
            TestEqual(TEXT("Edited Wolfen guard field 98"), V.GuardField98, 19.05f);
            TestEqual(TEXT("Edited Wolfen guard track"), V.GuardTrackBehaviourId, 3102);
            TestEqual(TEXT("Edited Wolfen guard field 90"), V.GuardField90, 3103);
            TestEqual(TEXT("Edited Wolfen guard camera"), V.OnOffCamera.CameraIndex, 30);
            TestEqual(TEXT("Edited Wolfen guard camera float"), V.OnOffCamera.Field8, 20.05f);
            if (!V.OnOffTargets.IsEmpty()) TestEqual(TEXT("Edited Wolfen guard target"), V.OnOffTargets[0].TargetRef, 3104);
        });
        CheckWolfen(WolfenEscapeActor, 13, [this](const FKyaWolfenBehaviour& V) {
            TestEqual(TEXT("Edited Wolfen escape flags"), V.Flags, 3105);
            TestEqual(TEXT("Edited Wolfen escape field 88"), V.EscapeField88, 3106);
            TestEqual(TEXT("Edited Wolfen escape field 9c"), V.EscapeField9C, 21.05f);
            TestEqual(TEXT("Edited Wolfen escape field 90"), V.EscapeField90, 22.05f);
            TestEqual(TEXT("Edited Wolfen escape field 8c"), V.EscapeField8C, 23.05f);
            TestEqual(TEXT("Edited Wolfen escape field 94"), V.EscapeField94, 24.05f);
            TestEqual(TEXT("Edited Wolfen escape field 98"), V.EscapeField98, 25.05f);
            TestEqual(TEXT("Edited Wolfen escape field a0"), V.EscapeFieldA0, 3107);
            if (!V.EscapePathIndices.IsEmpty()) TestEqual(TEXT("Edited Wolfen escape path"), V.EscapePathIndices[0], 3108);
        });
        CheckWolfen(WolfenExorcismActor, 14, [this](const FKyaWolfenBehaviour& V) {
            TestEqual(TEXT("Edited Wolfen exorcism material"), V.ExorcismDigitMaterialId, 3201);
            TestEqual(TEXT("Edited Wolfen exorcism field 24"), V.ExorcismField24, 3202);
            TestEqual(TEXT("Edited Wolfen exorcism texture A"), V.ExorcismTextureA, 3203);
            TestEqual(TEXT("Edited Wolfen exorcism mesh A"), V.ExorcismMeshA, 3204);
            TestEqual(TEXT("Edited Wolfen exorcism texture B"), V.ExorcismTextureB, 3205);
            TestEqual(TEXT("Edited Wolfen exorcism mesh B"), V.ExorcismMeshB, 3206);
            TestEqual(TEXT("Edited Wolfen exorcism field 18"), V.ExorcismField18, 3207);
            for (int32 Index = 0; Index < V.ExorcismWords.Num(); ++Index)
                TestEqual(TEXT("Edited Wolfen exorcism word"), V.ExorcismWords[Index], 3210 + Index);
        });
        {
            const FKyaActorPlacement& Actor = Reimported->SceneEnvironment.Actors[CatchActor];
            const int32 NewTypedBehaviourIndex = Actor.Behaviours.IndexOfByPredicate(
                [](const FKyaActorBehaviour& Behaviour) { return Behaviour.bHasTypedCatchByTrapBehaviour; });
            const FKyaActorBehaviour& Behaviour = Actor.Behaviours[NewTypedBehaviourIndex];
            if (!Behaviour.CatchOperations.IsEmpty() && !Behaviour.CatchOperations[0].Operations.IsEmpty())
            {
                TestEqual(TEXT("Edited catch operation variable"), Behaviour.CatchOperations[0].Operations[0].Variable, 3300);
                TestEqual(TEXT("Edited catch operation type"), Behaviour.CatchOperations[0].Operations[0].Type, 3301);
                TestEqual(TEXT("Edited catch operation value"), Behaviour.CatchOperations[0].Operations[0].Value, 3302);
            }
        }
        {
            const FKyaActorPlacement& Actor = Reimported->SceneEnvironment.Actors[HeroUnknownActor];
            const int32 NewTypedBehaviourIndex = Actor.Behaviours.IndexOfByPredicate(
                [](const FKyaActorBehaviour& Behaviour) { return Behaviour.bHasTypedHeroUnknownBehaviour; });
            const FKyaHeroUnknownBehaviour& Values = Actor.Behaviours[NewTypedBehaviourIndex].HeroUnknownBehaviour;
            TestEqual(TEXT("Edited hero-unknown fx A"), Values.FxTypeA, 3303);
            TestEqual(TEXT("Edited hero-unknown texture A"), Values.TextureA, 3304);
            TestEqual(TEXT("Edited hero-unknown mesh A"), Values.MeshA, 3305);
            TestEqual(TEXT("Edited hero-unknown fx B"), Values.FxTypeB, 3306);
            TestEqual(TEXT("Edited hero-unknown fx C"), Values.FxTypeC, 3307);
            TestEqual(TEXT("Edited hero-unknown texture B"), Values.TextureB, 3308);
            TestEqual(TEXT("Edited hero-unknown mesh B"), Values.MeshB, 3309);
        }
        {
            const FKyaActorPlacement& Actor = Reimported->SceneEnvironment.Actors[InventoryActor];
            const int32 NewTypedBehaviourIndex = Actor.Behaviours.IndexOfByPredicate(
                [](const FKyaActorBehaviour& Behaviour) { return Behaviour.bHasTypedInventoryBehaviour; });
            const FKyaInventoryInfo& Values = Actor.Behaviours[NewTypedBehaviourIndex].InventoryInfo;
            TestEqual(TEXT("Edited inventory field 0"), Values.Field0, 1);
            TestEqual(TEXT("Edited inventory purchase"), Values.PurchaseId, 3310);
            TestEqual(TEXT("Edited inventory field 28"), Values.Field28, 3311);
            TestEqual(TEXT("Edited inventory money"), Values.MoneyCost, 3312);
            TestEqual(TEXT("Edited inventory field 2c"), Values.Field2C, 3313);
            TestEqual(TEXT("Edited inventory initial behaviour"), Values.InitialBehaviourId, 3314);
        }
        CheckWolfen(WolfenSnipeActor, 21, [this](const FKyaWolfenBehaviour& V) {
            TestEqual(TEXT("Edited Wolfen snipe flags"), V.Flags, 3025);
            TestEqual(TEXT("Edited Wolfen snipe field 90"), V.SnipeField90, 3026);
            TestEqual(TEXT("Edited Wolfen snipe field 94"), V.SnipeField94, 12.05f);
            TestEqual(TEXT("Edited Wolfen snipe field 98"), V.SnipeField98, 13.05f);
            TestEqual(TEXT("Edited Wolfen snipe field 9c"), V.SnipeField9C, 14.05f);
            TestEqual(TEXT("Edited Wolfen snipe field a0"), V.SnipeFieldA0, 15.05f);
            TestEqual(TEXT("Edited Wolfen snipe spot track"), V.SpotTrackBehaviourId, 3027);
            TestEqual(TEXT("Edited Wolfen snipe track"), V.SnipeTrackBehaviourId, 3028);
            TestEqual(TEXT("Edited Wolfen snipe shadow material"), V.ShadowMaterialId, 3029);
            TestEqual(TEXT("Edited Wolfen snipe field ac"), V.SnipeFieldAC, 3030);
            TestEqual(TEXT("Edited Wolfen snipe field b0"), V.SnipeFieldB0, 16.05f);
            TestEqual(TEXT("Edited Wolfen snipe field b4"), V.SnipeFieldB4, 17.05f);
            TestEqual(TEXT("Edited Wolfen snipe nb spot"), V.NbSpot, 3031);
            TestEqual(TEXT("Edited Wolfen snipe nb shadow"), V.NbShadow, 3032);
            TestEqual(TEXT("Edited Wolfen snipe rotation"), V.RotationSpeed, 18.05f);
            TestEqual(TEXT("Edited Wolfen snipe field b8"), V.SnipeFieldB8, 3033);
        });
        CheckWolfen(WolfenLostActor, 22, [this](const FKyaWolfenBehaviour& V) {
            TestEqual(TEXT("Edited Wolfen lost flags"), V.Flags, 3013);
            if (!V.SwitchRecords.IsEmpty())
            {
                TestEqual(TEXT("Edited Wolfen lost behaviour id"), V.SwitchRecords[0].BehaviourId, 3014);
                if (V.SwitchRecords[0].Category == 0) TestEqual(TEXT("Edited Wolfen lost message type"), V.SwitchRecords[0].MessageType, 3015);
                else if (V.SwitchRecords[0].Category == 3) TestEqual(TEXT("Edited Wolfen lost see actor"), V.SwitchRecords[0].SeeActorRef, 3016);
                else
                {
                    TestEqual(TEXT("Edited Wolfen lost actor"), V.SwitchRecords[0].ActorRef, 3017);
                    TestEqual(TEXT("Edited Wolfen lost zone"), V.SwitchRecords[0].ZoneRef, 3018);
                    TestEqual(TEXT("Edited Wolfen lost flags"), V.SwitchRecords[0].Flags, 3019);
                }
            }
        });
    }
    AddInfo(FString::Printf(TEXT("Platform detail coverage: camera=%d sound=%d"), PlatformCameraActor != INDEX_NONE, PlatformSoundActor != INDEX_NONE));
    AddInfo(FString::Printf(TEXT("Basic box coverage: actor=%d"), BasicBoxActor != INDEX_NONE));
    AddInfo(FString::Printf(TEXT("FX coverage: actor=%d single=%d path=%d"), FxActor != INDEX_NONE, FxSingleActor != INDEX_NONE, FxPathActor != INDEX_NONE));
    AddInfo(FString::Printf(TEXT("Araignos coverage: actor=%d"), AraignosActor != INDEX_NONE));
    AddInfo(FString::Printf(TEXT("Projectile coverage: actor=%d field24=%d"), ProjectileActor != INDEX_NONE,
        ProjectileActor != INDEX_NONE && Bank->SceneEnvironment.Actors[ProjectileActor].ProjectilePattern.bHasField24));
    AddInfo(FString::Printf(TEXT("Wind coverage: actor=%d switches=%d"), WindActor != INDEX_NONE,
        WindActor != INDEX_NONE && Bank->SceneEnvironment.Actors[WindActor].bHasWindData));
    AddInfo(FString::Printf(TEXT("Trap coverage: actor=%d stand=%d inactive=%d"), TrapActor != INDEX_NONE,
        TrapStandActor != INDEX_NONE, TrapInactiveActor != INDEX_NONE));
    AddInfo(FString::Printf(TEXT("Weapon coverage: actor=%d pistol=%d sniper=%d rpg=%d"), WeaponActor != INDEX_NONE,
        WeaponPistolActor != INDEX_NONE, WeaponSniperActor != INDEX_NONE, WeaponRpgActor != INDEX_NONE));
    AddInfo(FString::Printf(TEXT("Switch coverage: actor=%d targets=%d"), SwitchActor != INDEX_NONE,
        SwitchActor != INDEX_NONE && !Bank->SceneEnvironment.Actors[SwitchActor].SwitchTarget.Targets.IsEmpty()));
    AddInfo(FString::Printf(TEXT("Bonus coverage: actor=%d turn=%d flock=%d addon=%d"), BonusActor != INDEX_NONE,
        BonusTurnActor != INDEX_NONE, BonusFlockActor != INDEX_NONE, BonusAddOnActor != INDEX_NONE));
    AddInfo(FString::Printf(TEXT("Micken coverage: actor=%d eat=%d"), MickenActor != INDEX_NONE, MickenEatActor != INDEX_NONE));
    AddInfo(FString::Printf(TEXT("Wolfen coverage: actor=%d prefix=%d fightParam=%d"), WolfenActor != INDEX_NONE,
        WolfenActor != INDEX_NONE && !Bank->SceneEnvironment.Actors[WolfenActor].ClassData.IsEmpty(),
        WolfenActor != INDEX_NONE && !Bank->SceneEnvironment.Actors[WolfenActor].WolfenFightParamData.Capabilities.IsEmpty()));
    AddInfo(FString::Printf(TEXT("Fruit coverage: actor=%d squashed=%d bonus=%d inventory=%d"), FruitActor != INDEX_NONE,
        FruitSquashedActor != INDEX_NONE, FruitBonusActor != INDEX_NONE, FruitInventoryActor != INDEX_NONE));
    AddInfo(FString::Printf(TEXT("Commander coverage: actor=%d behaviour=%d"), CommanderActor != INDEX_NONE, CommanderBehaviourActor != INDEX_NONE));
    AddInfo(FString::Printf(TEXT("Nativ coverage: actor=%d behaviour=%d"), NativActor != INDEX_NONE, NativBehaviourActor != INDEX_NONE));
    AddInfo(FString::Printf(TEXT("Amortos coverage: actor=%d behaviour=%d"), AmortosActor != INDEX_NONE, AmortosBehaviourActor != INDEX_NONE));
    AddInfo(FString::Printf(TEXT("Teleporter coverage: actor=%d destinations=%d switch170-targets=%d"), TeleporterActor != INDEX_NONE,
        TeleporterActor != INDEX_NONE && !Bank->SceneEnvironment.Actors[TeleporterActor].TeleporterDestinations.IsEmpty(),
        TeleporterActor != INDEX_NONE && !Bank->SceneEnvironment.Actors[TeleporterActor].TeleporterSwitch170.Targets.IsEmpty()));
    AddInfo(FString::Printf(TEXT("DCA coverage: actor=%d controlled=%d trajectory=%d"), DcaActor != INDEX_NONE,
        DcaControlledActor != INDEX_NONE, DcaTrajectoryActor != INDEX_NONE));
    AddInfo(FString::Printf(TEXT("Box coverage: actor=%d"), BoxClassActor != INDEX_NONE));
    AddInfo(FString::Printf(TEXT("Money coverage: actor=%d"), MoneyActor != INDEX_NONE));
    AddInfo(FString::Printf(TEXT("Fog coverage: actor=%d hierarchies=%d behaviour=%d"), FogActor != INDEX_NONE,
        FogActor != INDEX_NONE && !Bank->SceneEnvironment.Actors[FogActor].FogHierarchies.IsEmpty(), FogBehaviourActor != INDEX_NONE));
    AddInfo(FString::Printf(TEXT("Shooter coverage: actor=%d fire=%d"), ShootActor != INDEX_NONE, ShootFireActor != INDEX_NONE));
    AddInfo(FString::Printf(TEXT("Ambre coverage: actor=%d"), AmbreActor != INDEX_NONE));
    AddInfo(FString::Printf(TEXT("Stiller coverage: actor=%d targets=%d"), StillerActor != INDEX_NONE,
        StillerActor != INDEX_NONE && !Bank->SceneEnvironment.Actors[StillerActor].StillerSwitch208.Targets.IsEmpty()));
    AddInfo(FString::Printf(TEXT("Rune coverage: actor=%d"), RuneActor != INDEX_NONE));
    AddInfo(FString::Printf(TEXT("Bonus fountain coverage: actor=%d"), FountainActor != INDEX_NONE));
    AddInfo(FString::Printf(TEXT("Rope coverage: actor=%d targets=%d"), RopeActor != INDEX_NONE,
        RopeActor != INDEX_NONE && !Bank->SceneEnvironment.Actors[RopeActor].RopeSwitch.Targets.IsEmpty()));
    AddInfo(FString::Printf(TEXT("Gravity-aware coverage: actor=%d fall=%d bell=%d"), GravityActor != INDEX_NONE,
        GravityFallBehaviour != INDEX_NONE, GravityBellBehaviour != INDEX_NONE));
    AddInfo(FString::Printf(TEXT("Nativ command coverage: actor=%d"), NativCmdActor != INDEX_NONE));
    AddInfo(FString::Printf(TEXT("Companion coverage: actor=%d"), CompanionActor != INDEX_NONE));
    AddInfo(FString::Printf(TEXT("Bridge coverage: actor=%d basic=%d"), BridgeActor != INDEX_NONE, BridgeBasicBehaviour != INDEX_NONE));
    AddInfo(FString::Printf(TEXT("Aton coverage: actor=%d paths=%d other=%d behaviour=%d"), AtonActor != INDEX_NONE,
        AtonActor != INDEX_NONE && !Bank->SceneEnvironment.Actors[AtonActor].AtonPathIndices.IsEmpty(),
        AtonActor != INDEX_NONE && !Bank->SceneEnvironment.Actors[AtonActor].AtonOtherSubObjs.IsEmpty(),
        AtonBehaviour != INDEX_NONE));
    AddInfo(FString::Printf(TEXT("Boomy coverage: actor=%d types=%d conditions=%d"), BoomyActor != INDEX_NONE,
        BoomyActor != INDEX_NONE && !Bank->SceneEnvironment.Actors[BoomyActor].BoomyTypes.IsEmpty(),
        BoomyActor != INDEX_NONE && !Bank->SceneEnvironment.Actors[BoomyActor].BoomyCamera.Conditions.IsEmpty()));
    AddInfo(FString::Printf(TEXT("Kim coverage: actor=%d tutorials=%d projectiles=%d cameras=%d blows=%d grabs=%d reacts=%d combos=%d"),
        KimActor != INDEX_NONE,
        KimActor != INDEX_NONE && !Bank->SceneEnvironment.Actors[KimActor].KimTutorialZones.IsEmpty(),
        KimActor != INDEX_NONE && !Bank->SceneEnvironment.Actors[KimActor].KimProjectileZones.IsEmpty(),
        KimActor != INDEX_NONE && !Bank->SceneEnvironment.Actors[KimActor].KimCameras.IsEmpty(),
        KimActor != INDEX_NONE ? Bank->SceneEnvironment.Actors[KimActor].FighterBlows.Num() : 0,
        KimActor != INDEX_NONE ? Bank->SceneEnvironment.Actors[KimActor].FighterGrabs.Num() : 0,
        KimActor != INDEX_NONE ? Bank->SceneEnvironment.Actors[KimActor].FighterGrabReacts.Num() : 0,
        KimActor != INDEX_NONE ? Bank->SceneEnvironment.Actors[KimActor].FighterCombos.Num() : 0));
    AddInfo(FString::Printf(TEXT("Catch/hero coverage: catch=%d hero-unknown=%d"), CatchActor != INDEX_NONE, HeroUnknownActor != INDEX_NONE));
    AddInfo(FString::Printf(TEXT("Companion alert coverage: actor=%d alerts=%d conditions=%d"),
        CompanionAlertActor != INDEX_NONE,
        CompanionAlertActor != INDEX_NONE ? Bank->SceneEnvironment.Actors[CompanionAlertActor].Behaviours.FindByPredicate(
            [](const FKyaActorBehaviour& Behaviour) { return Behaviour.Id == 3; })->CompanionAlerts.Num() : 0,
        CompanionAlertActor != INDEX_NONE && Bank->SceneEnvironment.Actors[CompanionAlertActor].Behaviours.FindByPredicate(
            [](const FKyaActorBehaviour& Behaviour) { return Behaviour.Id == 3; })->CompanionAlerts.Num() > 1 &&
            !Bank->SceneEnvironment.Actors[CompanionAlertActor].Behaviours.FindByPredicate(
                [](const FKyaActorBehaviour& Behaviour) { return Behaviour.Id == 3; })->CompanionAlerts[1].Conditions.IsEmpty()));
    AddInfo(FString::Printf(TEXT("Platform behaviour coverage: stand=%d selector-slave=%d trajectory=%d"),
        PlatformStandActor != INDEX_NONE, PlatformSelectorActor != INDEX_NONE, PlatformTrajectoryActor != INDEX_NONE));
    AddInfo(FString::Printf(TEXT("Platform behaviour extra coverage: slab=%d master=%d selector-master=%d"),
        PlatformSlabActor != INDEX_NONE, PlatformMasterActor != INDEX_NONE, PlatformSelectorMasterActor != INDEX_NONE));
    AddInfo(FString::Printf(TEXT("Projectile behaviour coverage: stand=%d straight=%d inventory=%d"),
        ProjectileStandActor != INDEX_NONE, ProjectileStraightActor != INDEX_NONE, InventoryActor != INDEX_NONE));
    AddInfo(FString::Printf(TEXT("Switch behaviour coverage: lever=%d magic=%d"), SwitchLeverActor != INDEX_NONE, SwitchMagicActor != INDEX_NONE));
    AddInfo(FString::Printf(TEXT("Wolfen behaviour coverage: track=%d weapon=%d weapon-stand=%d weapon-snipe=%d dca=%d avoid=%d lost=%d watchdog=%d snipe=%d guard=%d escape=%d exorcism=%d"),
        WolfenTrackActor != INDEX_NONE, WolfenTrackWeaponActor != INDEX_NONE, WolfenTrackWeaponStandActor != INDEX_NONE,
        WolfenTrackWeaponSnipeActor != INDEX_NONE, WolfenDcaActor != INDEX_NONE, WolfenAvoidActor != INDEX_NONE, WolfenLostActor != INDEX_NONE,
        WolfenWatchDogActor != INDEX_NONE, WolfenSnipeActor != INDEX_NONE, WolfenGuardActor != INDEX_NONE, WolfenEscapeActor != INDEX_NONE, WolfenExorcismActor != INDEX_NONE));
    if (PlatformWithActors != INDEX_NONE) TestEqual(TEXT("Edited platform carry method"), Reimported->SceneEnvironment.Actors[PlatformWithActors].PlatformActors[0].CarryMethod, 77);
    if (PlatformWithZones != INDEX_NONE) TestEqual(TEXT("Edited platform zone"), Reimported->SceneEnvironment.Actors[PlatformWithZones].PlatformZones[0], 78);
    if (PlatformWithFriction != INDEX_NONE) TestEqual(TEXT("Edited platform no-friction zone"), Reimported->SceneEnvironment.Actors[PlatformWithFriction].PlatformNoFrictionZones[0].ZoneRef, 79);
    if (PlatformWithFrictionActors != INDEX_NONE) TestEqual(TEXT("Edited platform no-friction actor"), Reimported->SceneEnvironment.Actors[PlatformWithFrictionActors].PlatformNoFrictionZones[0].Actors[0], 80);
    AddInfo(FString::Printf(TEXT("Platform list coverage: actors=%d zones=%d friction-zones=%d friction-actors=%d"),
        PlatformWithActors != INDEX_NONE, PlatformWithZones != INDEX_NONE, PlatformWithFriction != INDEX_NONE, PlatformWithFrictionActors != INDEX_NONE));
    TestEqual(TEXT("Edited actor macro animation"), Reimported->SceneEnvironment.Actors[AnimatedActor].MacroAnimations[0].MacroAnimationId, 123);
    TestEqual(TEXT("Edited cluster zone reference"), Reimported->SceneEnvironment.Actors[ClusterActor].Behaviours[ClusterBehaviour].ClusterZones[0].ZoneRef, 1234);
    TestEqual(TEXT("Edited actor model name"), Reimported->SceneEnvironment.Actors[ModelActor].Models[0].MeshName, ModelName);
    TestEqual(TEXT("Edited collision bank value"), Reimported->SceneConfiguration.CollisionBankValues[0], 42);
    TestEqual(TEXT("Edited animation count"), Reimported->SceneConfiguration.LevelAnimationCount, 1053);
    TestEqual(TEXT("Edited actor allocation"), Reimported->SceneConfiguration.ActorClassAllocations[0].Count, 2);
    const int64 SceneOffset = Bank->Entries[0].Offset;
    const FKyaBankEntry* SceneData = Bank->Entries.FindByPredicate([](const FKyaBankEntry& Entry) { return Entry.Type == 6 && Entry.Subtype == 1; });
    if (!TestNotNull(TEXT("Scene data entry"), SceneData)) return false;
    auto ReadWord = [&Input](int64 Offset) -> uint32 {
        return uint32(Input[Offset]) | (uint32(Input[Offset + 1]) << 8) | (uint32(Input[Offset + 2]) << 16) | (uint32(Input[Offset + 3]) << 24);
    };
    const int64 MeshValuesOffset = SceneOffset + 40;
    const int64 TextureWordsOffset = MeshValuesOffset + int64(ReadWord(SceneOffset + 36)) * 4;
    const int64 AnimationOffset = TextureWordsOffset + 12 + int64(ReadWord(TextureWordsOffset + 8)) * 4;
    const int64 ClassDataOffset = AnimationOffset + 24;
    const int64 ParticleOffset = ClassDataOffset + int64(ReadWord(AnimationOffset + 20)) * 8;
    int64 SetupOffset = SceneData->Offset + 20;
    if (ReadWord(SceneData->Offset + 16)) SetupOffset += 12;
    const int32 BackgroundId = int32(ReadWord(SetupOffset));
    SetupOffset += 4 + (BackgroundId != -1 ? 4 : 0);
    int64 CameraConfigOffset = 0;
    if (!TestTrue(TEXT("Locate camera configuration"), KyaFindCameraConfig(Input, SetupOffset, CameraConfigOffset))) return false;
    const int64 CameraValuesOffset = CameraConfigOffset + 20 + (ReadWord(CameraConfigOffset + 16) == 1 ? 20 : 0);
    int64 SceneValuesOffset = 0;
    if (!TestTrue(TEXT("Locate scene values after camera"), KyaFindSceneAfterCamera(Input, CameraConfigOffset, SceneValuesOffset))) return false;
    int64 MapEndOffset = 0;
    if (!TestTrue(TEXT("Locate map end"), KyaFindMapEnd(Input, SceneValuesOffset + 12, MapEndOffset))) return false;
    TArray<FKyaConditionedOffsets> ScenarioOffsets;
    int64 ScenarioEndOffset = 0;
    if (!TestTrue(TEXT("Locate scenario end"), KyaReadConditionedOffsets(Input, MapEndOffset + 48, ScenarioOffsets, ScenarioEndOffset))) return false;
    TArray<FKyaObjectiveOffsets> ObjectiveOffsets;
    int64 ObjectiveEndOffset = 0;
    if (!TestTrue(TEXT("Locate objectives"), KyaReadObjectiveOffsets(Input, ScenarioEndOffset, ObjectiveOffsets, ObjectiveEndOffset)) || ObjectiveOffsets.IsEmpty()) return false;
    int64 WaypointOffset = 0;
    int32 WaypointCount = 0;
    if (!TestTrue(TEXT("Locate waypoints"), KyaFindWaypoints(Input, ObjectiveEndOffset, WaypointOffset, WaypointCount))) return false;
    TArray<FKyaPathOffsets> PathOffsets;
    if (!TestTrue(TEXT("Locate paths"), KyaReadPathOffsets(Input, WaypointOffset, WaypointCount, SceneData->Offset, PathOffsets)) || PathOffsets.IsEmpty()) return false;
    TArray<FKyaActorOffsets> ActorOffsets;
    if (!TestTrue(TEXT("Locate actors"), KyaReadActorOffsets(Input, WaypointOffset, SceneData->Offset, ActorOffsets)) || ActorOffsets.IsEmpty()) return false;
    TArray<int64> ChangedFieldOffsets;
    auto PlatformOffsetsFor = [&](int32 ActorIndex, FKyaPlatformPrefixOffsets& Offsets) {
        return ActorIndex != INDEX_NONE && KyaReadPlatformPrefix(Input, ActorOffsets[ActorIndex].ClassData, ActorOffsets[ActorIndex].ClassDataSize, Offsets);
    };
    FKyaPlatformPrefixOffsets PlatformOffsets;
    if (PlatformOffsetsFor(PlatformWithActors, PlatformOffsets)) ChangedFieldOffsets.Add(PlatformOffsets.Actors + 4);
    if (PlatformOffsetsFor(PlatformWithZones, PlatformOffsets)) ChangedFieldOffsets.Add(PlatformOffsets.Zones);
    if (PlatformOffsetsFor(PlatformWithFriction, PlatformOffsets)) ChangedFieldOffsets.Add(PlatformOffsets.NoFrictionZones[0]);
    if (PlatformOffsetsFor(PlatformWithFrictionActors, PlatformOffsets)) ChangedFieldOffsets.Add(PlatformOffsets.NoFrictionZones[0] + 8);
    if (PlatformOffsetsFor(PlatformDetailActor, PlatformOffsets))
    {
        FKyaPlatformDetailsOffsets Details;
        if (TestTrue(TEXT("Locate moving platform details"), KyaReadPlatformDetails(Input, PlatformOffsets.Suffix,
            ActorOffsets[PlatformDetailActor].ClassData + ActorOffsets[PlatformDetailActor].ClassDataSize - PlatformOffsets.Suffix, Details)))
        {
            for (int32 Field : { 0, 4, 28, 32, 36, 40, 44 })
                ChangedFieldOffsets.Add(Details.Properties + Field);
        }
    }
    if (PlatformOffsetsFor(PlatformCameraActor, PlatformOffsets))
    {
        FKyaPlatformDetailsOffsets Details;
        if (KyaReadPlatformDetails(Input, PlatformOffsets.Suffix, ActorOffsets[PlatformCameraActor].ClassData + ActorOffsets[PlatformCameraActor].ClassDataSize - PlatformOffsets.Suffix, Details))
        {
            ChangedFieldOffsets.Add(Details.Cameras);
            ChangedFieldOffsets.Add(Details.Cameras + 36);
            ChangedFieldOffsets.Add(Details.Cameras + 44);
        }
    }
    if (PlatformOffsetsFor(PlatformSoundActor, PlatformOffsets))
    {
        FKyaPlatformDetailsOffsets Details;
        if (KyaReadPlatformDetails(Input, PlatformOffsets.Suffix, ActorOffsets[PlatformSoundActor].ClassData + ActorOffsets[PlatformSoundActor].ClassDataSize - PlatformOffsets.Suffix, Details))
        {
            ChangedFieldOffsets.Add(Details.Sounds[0]);
            ChangedFieldOffsets.Add(Details.Sounds[0] + 4);
            ChangedFieldOffsets.Add(Details.Sounds[0] + 8);
        }
    }
    if (FxActor != INDEX_NONE)
    {
        FKyaFxOffsets Fx;
        if (KyaReadFxOffsets(Input, ActorOffsets[FxActor].ClassData, ActorOffsets[FxActor].ClassDataSize, Fx))
        {
            ChangedFieldOffsets.Add(Fx.Field19C);
            ChangedFieldOffsets.Add(Fx.Field1A0);
        }
        ChangedFieldOffsets.Add(ActorOffsets[FxSingleActor].Behaviours[FxSingleBehaviour].Payload);
        ChangedFieldOffsets.Add(ActorOffsets[FxPathActor].Behaviours[FxPathBehaviour].Payload);
        ChangedFieldOffsets.Add(ActorOffsets[FxPathActor].Behaviours[FxPathBehaviour].Payload + 4);
    }
    if (AraignosActor != INDEX_NONE)
    {
        FKyaAraignosOffsets Araignos;
        if (KyaReadAraignosOffsets(Input, ActorOffsets[AraignosActor].ClassData, ActorOffsets[AraignosActor].ClassDataSize, Araignos))
        {
            ChangedFieldOffsets.Add(Araignos.WeightB);
            ChangedFieldOffsets.Add(Araignos.PathFollow);
            for (int32 Field : { 0, 4, 8 }) ChangedFieldOffsets.Add(Araignos.Floats + Field);
            for (int32 Field : { 0, 4, 8 }) ChangedFieldOffsets.Add(Araignos.Sounds + Field);
            for (int32 Field : { 0, 4, 8, 12, 16 }) ChangedFieldOffsets.Add(Araignos.AddOn + Field);
        }
    }
    if (ProjectileActor != INDEX_NONE)
    {
        FKyaProjectileOffsets Projectile;
        if (KyaReadProjectileOffsets(Input, ActorOffsets[ProjectileActor].ClassData, ActorOffsets[ProjectileActor].ClassDataSize, Projectile))
        {
            ChangedFieldOffsets.Add(Projectile.WeightB);
            for (int32 Field : { 0, 4, 8, 12 }) ChangedFieldOffsets.Add(Projectile.Autonomous + Field);
            for (int32 Field = 0; Field <= 68; Field += 4) ChangedFieldOffsets.Add(Projectile.SubObj + Field);
            for (int32 Field = 0; Field <= 32; Field += 4) ChangedFieldOffsets.Add(Projectile.Pattern + Field);
            if (Projectile.bHasPatternField24) ChangedFieldOffsets.Add(Projectile.PatternField24);
            ChangedFieldOffsets.Add(Projectile.Pattern + (Projectile.bHasPatternField24 ? 40 : 36));
        }
    }
    if (WindActor != INDEX_NONE)
    {
        FKyaWindOffsets Wind;
        if (KyaReadWindOffsets(Input, ActorOffsets[WindActor].ClassData, ActorOffsets[WindActor].ClassDataSize, Wind))
        {
            ChangedFieldOffsets.Add(Wind.Field160);
            for (int32 Field : { 0, 4, 8, 12, 16, 20, 24, 28, 32, 36, 40, 44 }) ChangedFieldOffsets.Add(Wind.MaxWind + Field);
            ChangedFieldOffsets.Add(Wind.Sound); ChangedFieldOffsets.Add(Wind.ActivationZone);
            ChangedFieldOffsets.Add(Wind.Actors);
            for (int32 Field : { 0, 4, 8, 12 }) ChangedFieldOffsets.Add(Wind.Material + Field);
            ChangedFieldOffsets.Add(Wind.SectorHash); ChangedFieldOffsets.Add(Wind.SectorHash + 4);
            ChangedFieldOffsets.Add(Wind.Switches[0].Targets);
            ChangedFieldOffsets.Add(Wind.Switches[0].Targets + 12);
            ChangedFieldOffsets.Add(Wind.Switches[0].Camera);
            ChangedFieldOffsets.Add(Wind.Switches[0].Camera + 8);
            ChangedFieldOffsets.Add(Wind.Switches[1].Camera);
            ChangedFieldOffsets.Add(Wind.Switches[2].Targets);
            ChangedFieldOffsets.Add(Wind.Switches[2].Camera);
            for (int32 Field : { 0, 4, 8, 12, 16 }) ChangedFieldOffsets.Add(Wind.Floats194 + Field);
            ChangedFieldOffsets.Add(Wind.Waypoint);
            ChangedFieldOffsets.Add(Wind.Waypoint + 4);
            ChangedFieldOffsets.Add(Wind.Waypoint + 8);
        }
    }
    if (TrapActor != INDEX_NONE)
    {
        FKyaTrapOffsets Trap;
        if (KyaReadTrapOffsets(Input, ActorOffsets[TrapActor].ClassData, ActorOffsets[TrapActor].ClassDataSize, Trap))
        {
            ChangedFieldOffsets.Add(Trap.WeightB);
            for (int32 Field : { 0, 4, 8, 12 }) ChangedFieldOffsets.Add(Trap.Field14 + Field);
            if (Trap.bHasField24) ChangedFieldOffsets.Add(Trap.Field24);
            for (int32 Field : { 0, 4, 8, 12, 16, 20 }) ChangedFieldOffsets.Add(Trap.ActorRef + Field);
        }
        for (int32 Field : { 0, 4, 8 })
        {
            ChangedFieldOffsets.Add(ActorOffsets[TrapStandActor].Behaviours[TrapStandBehaviour].Payload + Field);
            ChangedFieldOffsets.Add(ActorOffsets[TrapInactiveActor].Behaviours[TrapInactiveBehaviour].Payload + Field);
        }
    }
    if (WeaponActor != INDEX_NONE)
    {
        FKyaWeaponOffsets Weapon;
        if (KyaReadWeaponOffsets(Input, ActorOffsets[WeaponActor].ClassData, ActorOffsets[WeaponActor].ClassDataSize, Weapon))
            for (int32 Field : { 0, 4, 8 }) ChangedFieldOffsets.Add(Weapon.WeightB + Field);
        const int64 Pistol = ActorOffsets[WeaponPistolActor].Behaviours[WeaponPistolBehaviour].Payload;
        for (int32 Field : { 0, 4, 8, 12, 32, 36, 40, 44, 48 }) ChangedFieldOffsets.Add(Pistol + Field);
        const int64 Sniper = ActorOffsets[WeaponSniperActor].Behaviours[WeaponSniperBehaviour].Payload;
        for (int32 Field : { 32, 36, 40, 44, 48, 52 }) ChangedFieldOffsets.Add(Sniper + Field);
        const int64 Rpg = ActorOffsets[WeaponRpgActor].Behaviours[WeaponRpgBehaviour].Payload;
        ChangedFieldOffsets.Add(Rpg + 36);
    }
    if (SwitchActor != INDEX_NONE)
    {
        FKyaSwitchOffsets Switch;
        if (KyaReadSwitchOffsets(Input, ActorOffsets[SwitchActor].ClassData, ActorOffsets[SwitchActor].ClassDataSize, Switch))
        {
            ChangedFieldOffsets.Add(Switch.Targets);
            ChangedFieldOffsets.Add(Switch.Targets + 12);
            ChangedFieldOffsets.Add(Switch.Camera);
            ChangedFieldOffsets.Add(Switch.Camera + 8);
        }
    }
    if (BonusActor != INDEX_NONE)
    {
        FKyaBonusOffsets Bonus;
        if (KyaReadBonusOffsets(Input, ActorOffsets[BonusActor].ClassData, ActorOffsets[BonusActor].ClassDataSize, Bonus))
            for (int32 Field : { 0, 4, 8, 12 }) ChangedFieldOffsets.Add(Bonus.WeightB + Field);
        for (int32 Field = 0; Field <= 28; Field += 4)
            ChangedFieldOffsets.Add(ActorOffsets[BonusTurnActor].Behaviours[BonusTurnBehaviour].Payload + Field);
        ChangedFieldOffsets.Add(ActorOffsets[BonusFlockActor].Behaviours[BonusFlockBehaviour].Payload);
        ChangedFieldOffsets.Add(ActorOffsets[BonusAddOnActor].Behaviours[BonusAddOnBehaviour].Payload);
    }
    if (MickenActor != INDEX_NONE)
    {
        FKyaMickenOffsets Micken;
        if (KyaReadMickenOffsets(Input, ActorOffsets[MickenActor].ClassData, ActorOffsets[MickenActor].ClassDataSize, Micken))
            for (int32 Field = 0; Field <= 36; Field += 4) ChangedFieldOffsets.Add(Micken.WeightB + Field);
        const int64 Eat = ActorOffsets[MickenEatActor].Behaviours[MickenEatBehaviour].Payload;
        for (int32 Field : { 0, 4, 8, 12 }) ChangedFieldOffsets.Add(Eat + Field);
    }
    if (WolfenActor != INDEX_NONE)
    {
        FKyaWolfenOffsets Wolfen;
        if (KyaReadWolfenOffsets(Input, ActorOffsets[WolfenActor].ClassData, ActorOffsets[WolfenActor].ClassDataSize, Wolfen))
        {
            for (int32 Field = 0; Field <= 152; Field += 4) ChangedFieldOffsets.Add(Wolfen.FighterFixed + Field);
            ChangedFieldOffsets.Add(Wolfen.WeaponIndex);
            {
                int64 BlowPos = Wolfen.Blows + 4;
                FKyaFighterBlowOffsets Blow;
                if (KyaReadFighterBlowOffsets(Input, BlowPos, Input.Num(), Blow))
                    for (int64 Field = Blow.Name; Field < Blow.Tail; Field += 4) ChangedFieldOffsets.Add(Field);
                int64 GrabPos = Wolfen.Grabs + 4;
                FKyaFighterGrabOffsets Grab;
                if (KyaReadFighterGrabOffsets(Input, GrabPos, Input.Num(), Grab))
                    for (int64 Field = Grab.Name; Field < Grab.Tail; Field += 4) ChangedFieldOffsets.Add(Field);
                int64 ReactPos = Wolfen.GrabReacts + 4;
                const int64 ReactStart = ReactPos;
                if (KyaSkipString(Input, ReactPos, Input.Num()))
                {
                    ReactPos += 8;
                    for (int64 Field = ReactStart; Field < ReactPos; Field += 4) ChangedFieldOffsets.Add(Field);
                }
                int64 ComboPos = Wolfen.Combos + 4;
                FKyaFighterComboOffsets Combo;
                if (KyaReadFighterComboOffsets(Input, ComboPos, Input.Num(), Combo))
                    for (int64 Field = Combo.Name; Field < Combo.Tail; Field += 4) ChangedFieldOffsets.Add(Field);
            }
            ChangedFieldOffsets.Add(Wolfen.FieldB74);
            for (int32 Field = 0; Field <= 16; Field += 4) ChangedFieldOffsets.Add(Wolfen.Vision + Field);
            ChangedFieldOffsets.Add(Wolfen.StartSector);
            ChangedFieldOffsets.Add(Wolfen.Hearing);
            ChangedFieldOffsets.Add(Wolfen.Hearing + 4);
            ChangedFieldOffsets.Add(Wolfen.VisionDetection);
            ChangedFieldOffsets.Add(Wolfen.FieldCF4);
            for (int32 Field = 0; Field <= 28; Field += 4) ChangedFieldOffsets.Add(Wolfen.WalkValues + Field);
            ChangedFieldOffsets.Add(Wolfen.MagicRequired);
            ChangedFieldOffsets.Add(Wolfen.FieldD30);
            for (int32 Field : { 0, 4, 8 }) ChangedFieldOffsets.Add(Wolfen.FieldB90 + Field);
            for (int64 Field = Wolfen.FightParam; Field < Wolfen.Tail; Field += 4) ChangedFieldOffsets.Add(Field);
        }
    }
    if (FruitActor != INDEX_NONE)
    {
        for (int32 Field = 0; Field <= 56; Field += 4) ChangedFieldOffsets.Add(ActorOffsets[FruitActor].ClassData + Field);
        for (int32 Field = 0; Field < 4; Field += 4) ChangedFieldOffsets.Add(ActorOffsets[FruitSquashedActor].Behaviours[FruitSquashedBehaviour].Payload + Field);
        for (int32 Field = 0; Field < 8; Field += 4) ChangedFieldOffsets.Add(ActorOffsets[FruitBonusActor].Behaviours[FruitBonusBehaviour].Payload + Field);
        for (int32 Field = 0; Field < 44; Field += 4) ChangedFieldOffsets.Add(ActorOffsets[FruitInventoryActor].Behaviours[FruitInventoryBehaviour].Payload + Field);
    }
    if (CommanderActor != INDEX_NONE)
    {
        for (int32 Field = 0; Field < ActorOffsets[CommanderActor].ClassDataSize; Field += 4)
            ChangedFieldOffsets.Add(ActorOffsets[CommanderActor].ClassData + Field);
        const FKyaBehaviourOffsets& Behaviour = ActorOffsets[CommanderBehaviourActor].Behaviours[CommanderBehaviour];
        for (int32 Field = 0; Field < Behaviour.Size; Field += 4) ChangedFieldOffsets.Add(Behaviour.Payload + Field);
    }
    if (NativActor != INDEX_NONE)
    {
        for (int32 Field = 0; Field < 64; Field += 4) ChangedFieldOffsets.Add(ActorOffsets[NativActor].ClassData + Field);
        const FKyaBehaviourOffsets& Behaviour = ActorOffsets[NativBehaviourActor].Behaviours[NativBehaviour];
        for (int32 Field = 0; Field < Behaviour.Size; Field += 4) ChangedFieldOffsets.Add(Behaviour.Payload + Field);
    }
    if (AmortosActor != INDEX_NONE)
    {
        for (int32 Field = 0; Field < ActorOffsets[AmortosActor].ClassDataSize; Field += 4)
            ChangedFieldOffsets.Add(ActorOffsets[AmortosActor].ClassData + Field);
        const FKyaBehaviourOffsets& Behaviour = ActorOffsets[AmortosBehaviourActor].Behaviours[AmortosBehaviour];
        for (int32 Field = 0; Field < Behaviour.Size; Field += 4) ChangedFieldOffsets.Add(Behaviour.Payload + Field);
    }
    if (TeleporterActor != INDEX_NONE)
        for (int32 Field = 0; Field < ActorOffsets[TeleporterActor].ClassDataSize; Field += 4)
            ChangedFieldOffsets.Add(ActorOffsets[TeleporterActor].ClassData + Field);
    if (DcaActor != INDEX_NONE)
    {
        for (int32 Field = 0; Field < ActorOffsets[DcaActor].ClassDataSize; Field += 4)
            ChangedFieldOffsets.Add(ActorOffsets[DcaActor].ClassData + Field);
        const FKyaBehaviourOffsets& Controlled = ActorOffsets[DcaControlledActor].Behaviours[DcaControlled];
        for (int32 Field = 0; Field < Controlled.Size; Field += 4) ChangedFieldOffsets.Add(Controlled.Payload + Field);
        const FKyaBehaviourOffsets& Trajectory = ActorOffsets[DcaTrajectoryActor].Behaviours[DcaTrajectory];
        for (int32 Field = 0; Field < Trajectory.Size; Field += 4) ChangedFieldOffsets.Add(Trajectory.Payload + Field);
    }
    if (BoxClassActor != INDEX_NONE)
        for (int32 Field = 0; Field < ActorOffsets[BoxClassActor].ClassDataSize; Field += 4)
            ChangedFieldOffsets.Add(ActorOffsets[BoxClassActor].ClassData + Field);
    if (MoneyActor != INDEX_NONE)
        for (int32 Field = 0; Field < 16; Field += 4)
            ChangedFieldOffsets.Add(ActorOffsets[MoneyActor].ClassData + Field);
    if (FogActor != INDEX_NONE)
    {
        for (int32 Field = 0; Field < ActorOffsets[FogActor].ClassDataSize; Field += 4)
            ChangedFieldOffsets.Add(ActorOffsets[FogActor].ClassData + Field);
        const FKyaBehaviourOffsets& Behaviour = ActorOffsets[FogBehaviourActor].Behaviours[FogBehaviour];
        for (int32 Field = 0; Field < Behaviour.Size; Field += 4) ChangedFieldOffsets.Add(Behaviour.Payload + Field);
    }
    if (ShootActor != INDEX_NONE)
    {
        for (int32 Field = 0; Field < ActorOffsets[ShootActor].ClassDataSize; Field += 4)
            ChangedFieldOffsets.Add(ActorOffsets[ShootActor].ClassData + Field);
        const FKyaBehaviourOffsets& Behaviour = ActorOffsets[ShootFireActor].Behaviours[ShootFireBehaviour];
        for (int32 Field = 0; Field < Behaviour.Size; Field += 4) ChangedFieldOffsets.Add(Behaviour.Payload + Field);
    }
    if (AmbreActor != INDEX_NONE)
        for (int32 Field = 0; Field < ActorOffsets[AmbreActor].ClassDataSize; Field += 4)
            ChangedFieldOffsets.Add(ActorOffsets[AmbreActor].ClassData + Field);
    if (StillerActor != INDEX_NONE)
        for (int32 Field = 0; Field < ActorOffsets[StillerActor].ClassDataSize; Field += 4)
            ChangedFieldOffsets.Add(ActorOffsets[StillerActor].ClassData + Field);
    for (int32 SingleActor : { RuneActor, FountainActor, RopeActor, GravityActor, NativCmdActor, CompanionActor, BridgeActor, AtonActor, KimActor })
        if (SingleActor != INDEX_NONE)
            for (int32 Field = 0; Field < ActorOffsets[SingleActor].ClassDataSize; Field += 4)
                ChangedFieldOffsets.Add(ActorOffsets[SingleActor].ClassData + Field);
    for (int32 TypedActor : { CatchActor, HeroUnknownActor, InventoryActor })
        if (TypedActor != INDEX_NONE)
        {
            const FKyaActorPlacement& TypedPlacement = Bank->SceneEnvironment.Actors[TypedActor];
            const int32 TypedIndex = TypedPlacement.Behaviours.IndexOfByPredicate(
                [](const FKyaActorBehaviour& Behaviour) {
                    return Behaviour.bHasTypedCatchByTrapBehaviour || Behaviour.bHasTypedHeroUnknownBehaviour ||
                        Behaviour.bHasTypedInventoryBehaviour;
                });
            const FKyaBehaviourOffsets& Behaviour = ActorOffsets[TypedActor].Behaviours[TypedIndex];
            for (int32 Field = 0; Field < Behaviour.Size; Field += 4) ChangedFieldOffsets.Add(Behaviour.Payload + Field);
        }
    if (AtonActor != INDEX_NONE)
    {
        const FKyaBehaviourOffsets& Behaviour = ActorOffsets[AtonActor].Behaviours[AtonBehaviour];
        for (int32 Field = 0; Field < Behaviour.Size; Field += 4) ChangedFieldOffsets.Add(Behaviour.Payload + Field);
    }
    if (BoomyActor != INDEX_NONE)
        for (int32 Field = 0; Field < ActorOffsets[BoomyActor].ClassDataSize; Field += 4)
            ChangedFieldOffsets.Add(ActorOffsets[BoomyActor].ClassData + Field);
    if (CompanionAlertActor != INDEX_NONE)
    {
        const FKyaActorPlacement& Companion = Bank->SceneEnvironment.Actors[CompanionAlertActor];
        const int32 CompanionBehaviourIndex = Companion.Behaviours.IndexOfByPredicate(
            [](const FKyaActorBehaviour& Behaviour) { return Behaviour.Id == 3; });
        const FKyaBehaviourOffsets& Behaviour = ActorOffsets[CompanionAlertActor].Behaviours[CompanionBehaviourIndex];
        for (int32 Field = 0; Field < Behaviour.Size; Field += 4) ChangedFieldOffsets.Add(Behaviour.Payload + Field);
    }
    const TPair<int32, int32> PlatformBehaviourEntries[] = {
        { PlatformStandActor, 7 }, { PlatformSelectorActor, 9 }, { PlatformTrajectoryActor, 2 },
        { PlatformSlabActor, 3 }, { PlatformMasterActor, 5 }, { PlatformSelectorMasterActor, 8 }
    };
    for (const TPair<int32, int32>& Entry : PlatformBehaviourEntries)
        if (Entry.Key != INDEX_NONE)
        {
            const FKyaActorPlacement& PlatformActorPlacement = Bank->SceneEnvironment.Actors[Entry.Key];
            const int32 PlatformBehaviourIndex = PlatformActorPlacement.Behaviours.IndexOfByPredicate(
                [Entry](const FKyaActorBehaviour& Behaviour) { return Behaviour.Id == Entry.Value; });
            const FKyaBehaviourOffsets& Behaviour = ActorOffsets[Entry.Key].Behaviours[PlatformBehaviourIndex];
            for (int32 Field = 0; Field < Behaviour.Size; Field += 4) ChangedFieldOffsets.Add(Behaviour.Payload + Field);
        }
    const TPair<int32, int32> TypedBehaviourEntries[] = {
        { ProjectileStandActor, 3 }, { ProjectileStraightActor, 4 }, { SwitchLeverActor, 2 }, { SwitchMagicActor, 3 },
        { WolfenTrackActor, 15 }, { WolfenTrackWeaponActor, 17 }, { WolfenTrackWeaponStandActor, 18 },
        { WolfenTrackWeaponSnipeActor, 19 }, { WolfenDcaActor, 23 }, { WolfenAvoidActor, 24 }, { WolfenLostActor, 22 },
        { WolfenWatchDogActor, 8 }, { WolfenSnipeActor, 21 }, { WolfenGuardActor, 10 }, { WolfenEscapeActor, 13 },
        { WolfenExorcismActor, 14 }
    };
    for (const TPair<int32, int32>& Entry : TypedBehaviourEntries)
        if (Entry.Key != INDEX_NONE)
        {
            const FKyaActorPlacement& TypedPlacement = Bank->SceneEnvironment.Actors[Entry.Key];
            const int32 TypedBehaviourIndex = TypedPlacement.Behaviours.IndexOfByPredicate(
                [Entry](const FKyaActorBehaviour& Behaviour) { return Behaviour.Id == Entry.Value; });
            const FKyaBehaviourOffsets& Behaviour = ActorOffsets[Entry.Key].Behaviours[TypedBehaviourIndex];
            for (int32 Field = 0; Field < Behaviour.Size; Field += 4) ChangedFieldOffsets.Add(Behaviour.Payload + Field);
        }
    if (BridgeActor != INDEX_NONE)
    {
        const FKyaBehaviourOffsets& Basic = ActorOffsets[BridgeActor].Behaviours[BridgeBasicBehaviour];
        for (int32 Field = 0; Field < Basic.Size; Field += 4) ChangedFieldOffsets.Add(Basic.Payload + Field);
    }
    if (GravityActor != INDEX_NONE)
    {
        const FKyaBehaviourOffsets& Fall = ActorOffsets[GravityActor].Behaviours[GravityFallBehaviour];
        for (int32 Field = 0; Field < Fall.Size; Field += 4) ChangedFieldOffsets.Add(Fall.Payload + Field);
        const FKyaBehaviourOffsets& Bell = ActorOffsets[GravityActor].Behaviours[GravityBellBehaviour];
        for (int32 Field = 0; Field < Bell.Size; Field += 4) ChangedFieldOffsets.Add(Bell.Payload + Field);
    }
    if (BasicBoxActor != INDEX_NONE)
    {
        FKyaBasicBoxOffsets Box;
        if (KyaReadBasicBoxOffsets(Input, ActorOffsets[BasicBoxActor].ClassData, ActorOffsets[BasicBoxActor].ClassDataSize, Box))
        {
            ChangedFieldOffsets.Add(Box.Field160);
            ChangedFieldOffsets.Add(Box.LifeMax);
            for (int32 Field : { 0, 4, 8, 12 }) ChangedFieldOffsets.Add(Box.Vibration + Field);
            ChangedFieldOffsets.Add(Box.Field180);
            for (int32 Field : { 0, 4, 8, 12 }) ChangedFieldOffsets.Add(Box.Fields + Field);
            for (int32 Field : { 0, 4, 8, 12, 16, 20, 24, 28 }) ChangedFieldOffsets.Add(Box.SwitchCamera + Field);
            for (int32 Field : { 0, 4, 8, 12, 16 }) ChangedFieldOffsets.Add(Box.AddOn + Field);
        }
    }
    bool OtherBytesUnchanged = Input.Num() == Edited.Num();
    for (int64 Index = 0; OtherBytesUnchanged && Index < Input.Num(); ++Index)
        if ((Index < SceneOffset + 16 || Index >= SceneOffset + 28) &&
            (Index < SceneOffset + 32 || Index >= SceneOffset + 36) &&
            (Index < MeshValuesOffset || Index >= MeshValuesOffset + 4) &&
            (Index < TextureWordsOffset + 4 || Index >= TextureWordsOffset + 8) &&
            (Index < AnimationOffset + 12 || Index >= AnimationOffset + 16) &&
            (Index < ParticleOffset || Index >= ParticleOffset + 4) &&
            (Index < SceneOffset + 508 || Index >= SceneOffset + 512) &&
            (Index < SetupOffset + 24 || Index >= SetupOffset + 28) &&
            (Index < SetupOffset + 32 || Index >= SetupOffset + 36) &&
            (Index < SetupOffset + 44 || Index >= SetupOffset + 48) &&
            (Index < SetupOffset + 48 || Index >= SetupOffset + 64) &&
            (Index < SetupOffset + 96 || Index >= SetupOffset + 100) &&
            (Index < SetupOffset + 100 || Index >= SetupOffset + 112) &&
            (Index < CameraConfigOffset + 12 || Index >= CameraConfigOffset + 16) &&
            (Index < CameraValuesOffset || Index >= CameraValuesOffset + 12) &&
            (Index < SceneValuesOffset || Index >= SceneValuesOffset + 12) &&
            (Index < SceneValuesOffset + 20 || Index >= SceneValuesOffset + 24) &&
            (Index < MapEndOffset + 4 || Index >= MapEndOffset + 8) &&
            (Index < MapEndOffset + 16 || Index >= MapEndOffset + 20) &&
            (Index < MapEndOffset + 32 || Index >= MapEndOffset + 36) &&
            (Index < ObjectiveOffsets[0].Header || Index >= ObjectiveOffsets[0].Header + 8) &&
            (Index < ObjectiveEndOffset || Index >= ObjectiveEndOffset + 8) &&
            (Index < WaypointOffset || Index >= WaypointOffset + 12) &&
            (Index < PathOffsets[0].Arrays[0] || Index >= PathOffsets[0].Arrays[0] + 16) &&
            (Index < ActorOffsets[0].Placement + 12 || Index >= ActorOffsets[0].Placement + 24) &&
            (Index < ActorOffsets[0].Common || Index >= ActorOffsets[0].Common + 4) &&
            (Index < ActorOffsets[0].Common + 24 || Index >= ActorOffsets[0].Common + 28) &&
            (Index < ActorOffsets[ConditionActor].Conditions + 12 || Index >= ActorOffsets[ConditionActor].Conditions + 16) &&
            (Index < ActorOffsets[CheckpointActor].ClassData + 12 || Index >= ActorOffsets[CheckpointActor].ClassData + 16) &&
            (Index < ActorOffsets[PlatformActor].ClassData || Index >= ActorOffsets[PlatformActor].ClassData + 4) &&
            !ChangedFieldOffsets.ContainsByPredicate([Index](int64 Offset) { return Index >= Offset && Index < Offset + 4; }) &&
            (Index < ActorOffsets[AnimatedActor].MacroAnimations + 4 || Index >= ActorOffsets[AnimatedActor].MacroAnimations + 8) &&
            (Index < ActorOffsets[ClusterActor].Behaviours[ClusterBehaviour].Payload + 8 || Index >= ActorOffsets[ClusterActor].Behaviours[ClusterBehaviour].Payload + 12) &&
            (Index < ActorOffsets[ModelActor].ModelStrings[0].Offset || Index >= ActorOffsets[ModelActor].ModelStrings[0].Offset + ActorOffsets[ModelActor].ModelStrings[0].Capacity) &&
            (Index < SceneOffset + 520 || Index >= SceneOffset + 524) &&
            (Index < SceneOffset + 884 || Index >= SceneOffset + 888) &&
            (Index < SceneOffset + 912 || Index >= SceneOffset + 916) && Input[Index] != Edited[Index])
            OtherBytesUnchanged = false;
    TestTrue(TEXT("Edited export preserves other bytes"), OtherBytesUnchanged);
    return true;
}
#endif
