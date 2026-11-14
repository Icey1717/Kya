#if WITH_DEV_AUTOMATION_TESTS
// The round-trip test is intentionally large; suppress the optimizer-size warning.
#pragma warning(disable : 4883)
#include "KyaLevelBankFactory.h"
#include "KyaLevelBankExporter.h"
#include "KyaLevelBank.h"
#include "KyaG3DChunks.h"
#include "KyaG2D.h"
#include "KyaCameraStream.h"
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
#include "Actors/KyaWolfen.h"
#include "Actors/KyaCommander.h"
#include "Actors/KyaKim.h"
#include "Actors/KyaBoomy.h"
#include "Actors/KyaAton.h"
#include "Actors/KyaRope.h"
#include "Actors/KyaStiller.h"
#include "Actors/KyaAmbre.h"
#include "Actors/KyaFountain.h"
#include "Actors/KyaCheckpoints.h"
#include "Actors/KyaBridge.h"
#include "Actors/KyaFog.h"
#include "Actors/KyaShoot.h"
#include "Actors/KyaMoney.h"
#include "Actors/KyaBox.h"
#include "Actors/KyaFx.h"
#include "Actors/KyaClusteriser.h"
#include "Actors/KyaCompanion.h"
#include "Actors/KyaGravity.h"
#include "Actors/KyaRune.h"
#include "Actors/KyaMicken.h"
#include "Actors/KyaBasicBox.h"
#include "Actors/KyaNativCmd.h"
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
#include "Actors/KyaCatchByTrap.h"
#include "Actors/KyaHeroUnknown.h"
#include "KyaActorEditService.h"
#include "KyaLevelPreview.h"
#include "KyaWaypointComponent.h"
#include "KyaPathPointComponent.h"
#include "KyaActorMarkerComponent.h"
#include "KyaActorTransform.h"
#include "ProceduralMeshComponent.h"
#include "Components/SplineComponent.h"
#include "Engine/World.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/MemoryWriter.h"

namespace
{
    /**
     * Builds a small but complete Kya bank in memory: header, entry/type tables,
     * one scene entry with a single INST actor chunk, and empty mesh/texture
     * entries. The actor is class 0x0d with one two-actor no-friction zone and a
     * full platform details block, so optional layouts can be round-tripped
     * without LEVEL_1.
     */
    struct FKyaSmallBankBuilder
    {
        TArray<uint8> Scene;

        static void U32(TArray<uint8>& Out, uint32 Value)
        {
            for (int32 Shift = 0; Shift < 32; Shift += 8) Out.Add(uint8(Value >> Shift));
        }
        static void I32(TArray<uint8>& Out, int32 Value) { U32(Out, uint32(Value)); }
        static void F32(TArray<uint8>& Out, float Value)
        {
            uint32 Bits = 0;
            FMemory::Memcpy(&Bits, &Value, sizeof(Bits));
            U32(Out, Bits);
        }
        static void Tag(TArray<uint8>& Out, const char* Text)
        {
            for (int32 Index = 0; Text[Index]; ++Index) Out.Add(uint8(Text[Index]));
        }

        TArray<uint8> Build(int64 ClassId = 0x0d, bool bTeleportBehaviour = false,
            bool bSwitchBehaviours = false, bool bWindSwitches = false, bool bTrapBehaviours = false,
            bool bWeaponBehaviours = false, bool bAraignosBehaviour = false,
            bool bBonusBehaviours = false)
        {
            const bool bHedgehog = ClassId == 0x22;
            const bool bProjectile = ClassId == 0x1c;
            const bool bPlatform = ClassId == 0x0d;
            const bool bSwitch = ClassId == 0x11;
            const bool bWind = ClassId == 0x12;
            const bool bTrap = ClassId == 0x2c;
            const bool bWeapon = ClassId == 0x20;
            const bool bAraignos = ClassId == 0x21;
            const bool bBonus = ClassId == 0x0e;
            const bool bTeleporter = ClassId == 0x24;
            const bool bAmortos = ClassId == 0x29;
            const bool bDca = ClassId == 0x1b;
            const bool bFruit = ClassId == 0x16;
            const bool bNativ = ClassId == 0x0a;
            const bool bWolfen = ClassId == 0x09;
            const bool bCommander = ClassId == 0x13;
            const bool bKim = ClassId == 0x06;
            const bool bBoomy = ClassId == 0x05;
            const bool bAton = ClassId == 0x38;
            const bool bRope = ClassId == 0x0f;
            const bool bStiller = ClassId == 0x4c;
            const bool bAmbre = ClassId == 0x14;
            const bool bFountain = ClassId == 0x52;
            const bool bCheckpoints = ClassId == 0x1e;
            const bool bBridge = ClassId == 0x17;
            const bool bFog = ClassId == 0x3d;
            const bool bShoot = ClassId == 0x3e;
            const bool bMoney = ClassId == 0x32;
            const bool bBoxClass = ClassId == 0x35;
            const bool bFx = ClassId == 0x2e;
            const bool bCluster = ClassId == 0x18;
            const bool bCompanion = ClassId == 0x19;
            const bool bGravity = ClassId == 0x10;
            const bool bRune = ClassId == 0x51;
            const bool bMicken = ClassId == 0x07;
            const bool bBasicBox = ClassId == 0x39;
            const bool bNativCmd = ClassId == 0x3c;
            const bool bEventGenerator = ClassId == 0x3b;
            const bool bExplosiveDistributor = ClassId == 0x1d;
            const bool bShocker = ClassId == 0x4b;
            const bool bNativShop = ClassId == 0x34;
            const bool bPunchingBall = ClassId == 0x30;
            const bool bNoseMonster = ClassId == 0x26;
            const bool bPattern = ClassId == 0x3f;
            const bool bShip = ClassId == 0x36;
            const bool bBunch = ClassId == 0x4e;
            const bool bBrazul = ClassId == 0x4f;
            const bool bMiniGameBase = ClassId == 0x44;
            const bool bMiniGameTA = ClassId == 0x45;
            const bool bMiniGameBox = ClassId == 0x46;
            const bool bMiniGameBoomy = ClassId == 0x47;
            Scene.Reset();
            Scene.AddZeroed(16);
            U32(Scene, 1);        // has cluster
            U32(Scene, 0);        // scene mesh ordinal
            U32(Scene, 0);        // scene texture ordinal
            U32(Scene, 0);
            I32(Scene, -1);       // no sector background
            // CScene::Level_Setup: 20 unused bytes then flags/fog (112 bytes).
            Scene.AddZeroed(20);
            U32(Scene, 0);        // background flags
            F32(Scene, 500.0f);   // clip distance
            F32(Scene, 0.0f);     // fog parameter
            Scene.AddZeroed(4);   // fog colour
            U32(Scene, 0);        // fog flags
            I32(Scene, 0);        // mipmap L
            I32(Scene, 0);        // mipmap K
            Scene.AddZeroed(64);  // camera fields
            U32(Scene, 0);        // camera condition count
            U32(Scene, 0);        // camera config flags
            I32(Scene, -1);       // camera actor ref A
            I32(Scene, -1);       // camera actor ref B
            F32(Scene, 0.0f);     // camera config angle
            U32(Scene, 0);        // camera config mode
            Scene.AddZeroed(36);
            U32(Scene, 0);        // astruct_12 extra flag
            F32(Scene, 1.5f);     // field 118
            U32(Scene, 0);        // field 11C
            U32(Scene, 0);        // count 120
            I32(Scene, 0);        // map field 04
            U32(Scene, 0xABCD);   // map field 08
            F32(Scene, 0.0f);     // map angle
            U32(Scene, 0);        // map entry count
            I32(Scene, 0);        // map field 14
            I32(Scene, 0);        // map actor ref
            I32(Scene, 0);        // default texture index
            I32(Scene, 0);        // alternate texture index
            I32(Scene, 0);        // default material index
            for (int32 Pool = 0; Pool < 7; ++Pool) U32(Scene, 0);
            U32(Scene, 0);        // conditioned operation blocks
            U32(Scene, 0);        // objectives
            U32(Scene, 0);        // objective zone entries
            I32(Scene, 77);       // field 10C
            I32(Scene, 0);        // field 110
            U32(Scene, 0);        // elevator cutscenes
            Tag(Scene, "ISOP");
            U32(Scene, 12);       // waypoint chunk size
            U32(Scene, 0);        // waypoint count
            Tag(Scene, "HTAP");
            U32(Scene, 12);       // path chunk size
            U32(Scene, 0);        // path count

            // Keep the INST chunk four-byte aligned relative to the scene entry.
            const int32 ActorStart = Scene.Num() + 4;
            if (ActorStart % 4) Scene.AddZeroed(4 - ActorStart % 4);
            U32(Scene, 1);        // actor count

            TArray<uint8> Actor;
            Tag(Actor, "TSNI");
            U32(Actor, 0);        // size placeholder
            U32(Actor, 0);        // actor index
            U32(Actor, uint32(ClassId));     // class id
            Tag(Actor, bHedgehog ? "HEDGEHOG" : (bProjectile ? "PROJECTILE" : (bSwitch ? "SWITCH" : (bWind ? "WIND" : (bTrap ? "TRAP" : (bWeapon ? "WEAPON" : (bAraignos ? "ARAIGNOS" : (bBonus ? "BONUS" : (bTeleporter ? "TELEPORTER" : (bAmortos ? "AMORTOS" : (bDca ? "DCA" : (bFruit ? "FRUIT" : (bNativ ? "NATIV" : (bWolfen ? "WOLFEN" : (bCommander ? "COMMANDER" : (bKim ? "KIM" : (bBoomy ? "BOOMY" : (bAton ? "ATON" : (bRope ? "ROPE" : (bStiller ? "STILLER" : (bAmbre ? "AMBRE" : (bFountain ? "FOUNTAIN" : (bCheckpoints ? "CHECKPOINTS" : (bBridge ? "BRIDGE" : (bFog ? "FOGMANAGER" : (bShoot ? "SHOOT" : (bMoney ? "MONEY" : (bBoxClass ? "BOX" : (bFx ? "ACTORFX" : (bCluster ? "CLUSTERISER" : (bCompanion ? "COMPANION" : (bGravity ? "BELL" : (bRune ? "RUNE" : (bMicken ? "MICKEN" : (bBasicBox ? "BASICBOX" : (bNativCmd ? "NATIVCMD" : (bEventGenerator ? "EVENTGENERATOR" : (bExplosiveDistributor ? "EXPLOSIVEDIST" : (bShocker ? "SHOCKER" : (bNativShop ? "NATIV_SHOP" : (bPunchingBall ? "PUNCHINGBALL" : (bNoseMonster ? "NOZEMONSTER" : (bPattern ? "LINER" : (bShip ? "HUNTERSHIP" : (bBunch ? "KRONOS_BUNCH" : (bBrazul ? "BRAZUL" : (bMiniGameBase ? "MINIGAME" : (bMiniGameTA ? "MINIGAMESTIMEATTACK" : (bMiniGameBox ? "MINIGAMEBOXCOUNTER" : (bMiniGameBoomy ? "MINIGAMEBOOMY" : "PLATFORM"))))))))))))))))))))))))))))))))))))))))))))))))));
            Actor.Add(0);
            const int32 AbsoluteNameEnd = Scene.Num() + Actor.Num();
            if (AbsoluteNameEnd % 4) Actor.AddZeroed(4 - AbsoluteNameEnd % 4);
            U32(Actor, 0);        // mesh index
            U32(Actor, 0);        // texture index
            U32(Actor, 0);        // collision index
            for (const float Value : { 1.0f, 2.0f, 3.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f })
                F32(Actor, Value);
            U32(Actor, 0);        // macro animations
            if (bHedgehog)
            {
                // Behaviour 3 carries a path-follow index; behaviour 4 is empty.
                U32(Actor, 2);
                U32(Actor, 3); U32(Actor, 4);
                U32(Actor, 4); U32(Actor, 0);
                I32(Actor, 12);
            }
            else if (bProjectile)
            {
                // Every projectile-owned payload shape: stand (12), straight (20),
                // portable (12), inventory (44), lava ball with field 18 (44),
                // excuse (12), inactive (12), new (4).
                U32(Actor, 8);
                U32(Actor, 3); U32(Actor, 12);
                U32(Actor, 4); U32(Actor, 20);
                U32(Actor, 5); U32(Actor, 12);
                U32(Actor, 9); U32(Actor, 44);
                U32(Actor, 10); U32(Actor, 44);
                U32(Actor, 11); U32(Actor, 12);
                U32(Actor, 12); U32(Actor, 12);
                U32(Actor, 13); U32(Actor, 4);
                // Stand: behaviourId, field C, material.
                I32(Actor, 3); I32(Actor, 1); I32(Actor, 7);
                // Straight: behaviourId, field C, speed, acceleration, field 18.
                I32(Actor, 4); I32(Actor, 2); F32(Actor, 3.5f); F32(Actor, 4.5f); F32(Actor, 5.5f);
                // Portable: behaviourId, field C, field 10.
                I32(Actor, 5); I32(Actor, 3); F32(Actor, 6.5f);
                // Inventory: present, two u64, purchase/fields, no object, initial.
                U32(Actor, 1); U32(Actor, 11); U32(Actor, 0); U32(Actor, 12); U32(Actor, 0);
                I32(Actor, 21); I32(Actor, 22); I32(Actor, 23); I32(Actor, 24);
                U32(Actor, 0); I32(Actor, 3);
                // Lava ball: id, field C, field 10/14, field 30, field 18, path,
                // field 2c/34/38/3c.
                I32(Actor, 10); I32(Actor, 4); F32(Actor, 1.0f); F32(Actor, 2.0f);
                I32(Actor, 3); F32(Actor, 4.0f); I32(Actor, 5); I32(Actor, 6);
                I32(Actor, 7); I32(Actor, 8); I32(Actor, 9);
                // Excuse: behaviourId, field C, actor reference.
                I32(Actor, 11); I32(Actor, 5); I32(Actor, 0);
                // Inactive: activate message, flags, active behaviour.
                I32(Actor, 6); U32(Actor, 7); I32(Actor, 3);
                // New: CBehaviourProjectile::Create only.
                I32(Actor, 13);
            }
            else if (bPlatform && bTeleportBehaviour)
            {
                // CBehaviourTeleportRandom payload: one path-follow index.
                U32(Actor, 1);
                U32(Actor, 10); U32(Actor, 4);
                I32(Actor, 33);
            }
            else if (bSwitch && bSwitchBehaviours)
            {
                // WolfenCounter (20), Sequence (24) and Timed (124).
                U32(Actor, 3);
                U32(Actor, 4); U32(Actor, 20);
                U32(Actor, 7); U32(Actor, 24);
                U32(Actor, 8); U32(Actor, 124);
                // WolfenCounter.
                I32(Actor, 2); F32(Actor, 3.5f); I32(Actor, 4); I32(Actor, 5); I32(Actor, 6);
                // Sequence: field 08, field 0C, count 3, three values.
                I32(Actor, 7); I32(Actor, 8); U32(Actor, 3); I32(Actor, 11); I32(Actor, 12); I32(Actor, 13);
                // Timed: base delay, two entries.
                F32(Actor, 0.5f); U32(Actor, 2);
                // Entry 0: zero targets + camera + delay.
                U32(Actor, 0);
                for (int32 Item = 0; Item < 8; ++Item) U32(Actor, uint32(21 + Item));
                F32(Actor, 1.5f);
                // Entry 1: one target + camera + delay.
                U32(Actor, 1);
                for (int32 Item = 0; Item < 7; ++Item) U32(Actor, uint32(41 + Item));
                for (int32 Item = 0; Item < 8; ++Item) U32(Actor, uint32(51 + Item));
                F32(Actor, 2.5f);
                // Playback mode and flags.
                U32(Actor, 3); U32(Actor, 4);
            }
            else if (bWind)
            {
                // CBehaviourWind::Create is empty; the record still exists.
                U32(Actor, 1);
                U32(Actor, 2); U32(Actor, 0);
            }
            else if (bTrap && bTrapBehaviours)
            {
                U32(Actor, 2);
                U32(Actor, 2); U32(Actor, 12);
                U32(Actor, 3); U32(Actor, 12);
                U32(Actor, 1); U32(Actor, 2); U32(Actor, 3);
                U32(Actor, 4); U32(Actor, 5); U32(Actor, 6);
            }
            else if (bWeapon && bWeaponBehaviours)
            {
                U32(Actor, 3);
                U32(Actor, 3); U32(Actor, 52);
                U32(Actor, 4); U32(Actor, 56);
                U32(Actor, 6); U32(Actor, 52);                // Shared burst block: two 16-byte entries.
                for (int32 Entry = 0; Entry < 2; ++Entry)
                {
                    F32(Actor, float(1 + Entry * 4));
                    I32(Actor, 2 + Entry * 4);
                    F32(Actor, 3 + Entry * 4);
                    F32(Actor, 4 + Entry * 4);
                }
                // Pistol fields.
                U32(Actor, 21); U32(Actor, 22); I32(Actor, 23); I32(Actor, 24); I32(Actor, 25);
                // Sniper: burst + field40/44/sound + texture/mesh/material.
                for (int32 Entry = 0; Entry < 2; ++Entry)
                {
                    F32(Actor, float(31 + Entry * 4));
                    I32(Actor, 32 + Entry * 4);
                    F32(Actor, 33 + Entry * 4);
                    F32(Actor, 34 + Entry * 4);
                }
                U32(Actor, 41); U32(Actor, 42); I32(Actor, 43); I32(Actor, 44); I32(Actor, 45); I32(Actor, 46);
                // RPG: burst + actor stream (two refs).
                for (int32 Entry = 0; Entry < 2; ++Entry)
                {
                    F32(Actor, float(51 + Entry * 4));
                    I32(Actor, 52 + Entry * 4);
                    F32(Actor, 53 + Entry * 4);
                    F32(Actor, 54 + Entry * 4);
                }
                U32(Actor, 4); I32(Actor, 61); I32(Actor, 62); I32(Actor, 63); I32(Actor, 64);
            }
            else if (bAraignos && bAraignosBehaviour)
            {
                // CBhvDefault::Create is empty; the record still exists.
                U32(Actor, 1);
                U32(Actor, 2); U32(Actor, 0);
            }
            else if (bBonus && bBonusBehaviours)
            {
                U32(Actor, 4);
                U32(Actor, 3); U32(Actor, 32);
                U32(Actor, 4); U32(Actor, 52);
                U32(Actor, 5); U32(Actor, 4);
                U32(Actor, 6); U32(Actor, 4);
                // Turn: alone prefix + turn value.
                F32(Actor, 1.0f); F32(Actor, 2.0f); F32(Actor, 3.0f); U32(Actor, 4);
                F32(Actor, 5.0f); U32(Actor, 6); I32(Actor, 7); F32(Actor, 8.0f);
                // Path: alone prefix + path index/floats/word.
                F32(Actor, 11.0f); F32(Actor, 12.0f); F32(Actor, 13.0f); U32(Actor, 14);
                F32(Actor, 15.0f); U32(Actor, 16); I32(Actor, 17); I32(Actor, 18);
                F32(Actor, 19.0f); F32(Actor, 20.0f); F32(Actor, 21.0f); F32(Actor, 22.0f);
                U32(Actor, 23);
                // Flock path + add-on field.
                I32(Actor, 31);
                F32(Actor, 32.0f);
            }
            else if (bTeleporter)
            {
                // CBehaviourTeleporterDefault::Create is empty; the record still exists.
                U32(Actor, 1);
                U32(Actor, 2); U32(Actor, 0);
            }
            else if (bAmortos)
            {
                // CBehaviourAmortosStand::Create is empty; the record still exists.
                U32(Actor, 1);
                U32(Actor, 2); U32(Actor, 0);
            }
            else if (bDca)
            {
                // Default (0), controlled (40) and trajectory (12).
                U32(Actor, 3);
                U32(Actor, 2); U32(Actor, 0);
                U32(Actor, 3); U32(Actor, 40);
                U32(Actor, 5); U32(Actor, 12);
                // Controlled (40 bytes).
                F32(Actor, 1.0f); F32(Actor, 2.0f); F32(Actor, 3.0f); F32(Actor, 4.0f);
                F32(Actor, 5.0f); F32(Actor, 6.0f); F32(Actor, 7.0f);
                F32(Actor, 8.0f); F32(Actor, 9.0f); F32(Actor, 10.0f);
                // Trajectory (12 bytes).
                I32(Actor, 11); F32(Actor, 12.5f); U32(Actor, 13);
            }
            else if (bFruit)
            {
                // Suspended (0), squashed (4), inventory (44), bonus (8) and
                // projectile-new (4, left raw).
                U32(Actor, 5);
                U32(Actor, 3); U32(Actor, 0);
                U32(Actor, 4); U32(Actor, 4);
                U32(Actor, 5); U32(Actor, 44);
                U32(Actor, 6); U32(Actor, 8);
                U32(Actor, 7); U32(Actor, 4);
                F32(Actor, 1.5f);   // squashed value
                // Inventory: present, two u64, purchase/fields, no object, initial.
                U32(Actor, 1);
                U32(Actor, 11); U32(Actor, 0); U32(Actor, 12); U32(Actor, 0);
                I32(Actor, 13); I32(Actor, 14); I32(Actor, 15); I32(Actor, 16);
                U32(Actor, 0); I32(Actor, 3);
                I32(Actor, 21); I32(Actor, 22);   // bonus
                I32(Actor, 7);                     // projectile-new raw
            }
            else if (bNativ)
            {
                // Exorcism (empty) and take-and-put (one 16-byte trajectory).
                U32(Actor, 2);
                U32(Actor, 4); U32(Actor, 0);
                U32(Actor, 6); U32(Actor, 20);
                U32(Actor, 1);   // one trajectory
                F32(Actor, 3.5f); I32(Actor, 21); I32(Actor, 22); F32(Actor, 23.5f);
            }
            else if (bWolfen)
            {
                // Track (8), DCA (12) and Avoid (12).
                U32(Actor, 3);
                U32(Actor, 15); U32(Actor, 8);
                U32(Actor, 23); U32(Actor, 12);
                U32(Actor, 24); U32(Actor, 12);
                U32(Actor, 1); F32(Actor, 2.5f);
                U32(Actor, 2); I32(Actor, 21); F32(Actor, 3.5f);
                U32(Actor, 3); F32(Actor, 4.5f); F32(Actor, 5.5f);
            }
            else if (bCommander)
            {
                // Commander (72 bytes: squad config + one switch target).
                U32(Actor, 1);
                U32(Actor, 2); U32(Actor, 72);
                U32(Actor, 7); U32(Actor, 8);
                U32(Actor, 1);
                for (int32 Item = 0; Item < 7; ++Item) U32(Actor, uint32(31 + Item));
                for (int32 Item = 0; Item < 8; ++Item) U32(Actor, uint32(41 + Item));
            }
            else if (bKim)
            {
                // Empty hero-default behaviour.
                U32(Actor, 1);
                U32(Actor, 3); U32(Actor, 0);
            }
            else if (bBoomy)
            {
                // CBehaviourBoomyLaunch::Create is empty; the record exists.
                U32(Actor, 1);
                U32(Actor, 2); U32(Actor, 0);
            }
            else if (bAton)
            {
                // CBehaviourAtonEscape has no Create override; empty record.
                U32(Actor, 1);
                U32(Actor, 3); U32(Actor, 0);
            }
            else if (bRope)
            {
                // CBehaviourRopeStand::Create is empty; the record exists.
                U32(Actor, 1);
                U32(Actor, 2); U32(Actor, 0);
            }
            else if (bStiller)
            {
                // CBehaviourStillerStand::Create is empty; the record exists.
                U32(Actor, 1);
                U32(Actor, 2); U32(Actor, 0);
            }
            else if (bAmbre)
            {
                // CBehaviourAmbre has no Create override; empty record.
                U32(Actor, 1);
                U32(Actor, 2); U32(Actor, 0);
            }
            else if (bFountain)
            {
                // CBehaviourBonusFountainStand has no Create override.
                U32(Actor, 1);
                U32(Actor, 2); U32(Actor, 0);
            }
            else if (bCheckpoints)
            {
                // CBehaviourCheckpoint::Create is empty; the record exists.
                U32(Actor, 1);
                U32(Actor, 2); U32(Actor, 0);
            }
            else if (bBridge)
            {
                // Basic (32 bytes) and wind-aware (20 bytes).
                U32(Actor, 2);
                U32(Actor, 2); U32(Actor, 32);
                U32(Actor, 3); U32(Actor, 20);
                F32(Actor, 1.0f); F32(Actor, 2.0f); F32(Actor, 3.0f);
                F32(Actor, 4.0f); F32(Actor, 5.0f); F32(Actor, 6.0f);
                F32(Actor, 7.0f); F32(Actor, 8.0f);
                for (int32 Item = 9; Item <= 13; ++Item) F32(Actor, float(Item));
            }
            else if (bFog)
            {
                // Fog zones: count + count x 28 bytes (total 32 for one zone).
                U32(Actor, 1);
                U32(Actor, 2); U32(Actor, 32);
                U32(Actor, 1); U32(Actor, 1); I32(Actor, 2);
                F32(Actor, 3.5f); F32(Actor, 4.5f);
                U32(Actor, 0x11223344); U32(Actor, 5); F32(Actor, 6.5f);
            }
            else if (bShoot)
            {
                // Fire (28 bytes: 3 words + 3 refs) and wave (28 bytes: 7 floats).
                U32(Actor, 2);
                U32(Actor, 3); U32(Actor, 28);
                U32(Actor, 4); U32(Actor, 28);
                U32(Actor, 7); F32(Actor, 1.5f); F32(Actor, 2.5f);
                U32(Actor, 3); I32(Actor, 21); I32(Actor, 22); I32(Actor, 23);
                for (int32 Item = 1; Item <= 7; ++Item) F32(Actor, float(Item));
            }
            else if (bMoney)
            {
                // Flock (4-byte path index) and add-on (empty Create).
                U32(Actor, 2);
                U32(Actor, 2); U32(Actor, 4);
                U32(Actor, 3); U32(Actor, 0);
                I32(Actor, 21);
            }
            else if (bBoxClass)
            {
                // CBehaviourBoxStand::Create is empty; the record exists.
                U32(Actor, 1);
                U32(Actor, 3); U32(Actor, 0);
            }
            else if (bFx)
            {
                // Single (4), path (8) and unknown (count + ids).
                U32(Actor, 3);
                U32(Actor, 2); U32(Actor, 4);
                U32(Actor, 3); U32(Actor, 8);
                U32(Actor, 4); U32(Actor, 12);
                I32(Actor, 7);
                I32(Actor, 8); I32(Actor, 9);
                U32(Actor, 2); I32(Actor, 10); I32(Actor, 11);
            }
            else if (bCluster)
            {
                // One cluster zone: header, 4 id lists, 1 hash list and tail.
                U32(Actor, 1);
                U32(Actor, 2); U32(Actor, 52);
                U32(Actor, 1);                         // zone count
                U32(Actor, 1); U32(Actor, 2);          // flags, zone ref
                U32(Actor, 1); I32(Actor, 3);          // actors
                U32(Actor, 0);                         // other actors
                U32(Actor, 1); I32(Actor, 4);          // cameras
                U32(Actor, 0);                         // lights
                U32(Actor, 1); U32(Actor, 0x11223344); U32(Actor, 0x55667788);
                I32(Actor, 5);                         // other zone ref
            }
            else if (bCompanion)
            {
                // One alert: empty conditions, empty switch, empty conditioned list.
                U32(Actor, 1);
                U32(Actor, 3); U32(Actor, 112);
                U32(Actor, 1);                         // one alert
                U32(Actor, 0);                         // conditions
                U32(Actor, 1); U32(Actor, 2); U32(Actor, 3); U32(Actor, 4);
                F32(Actor, 5.5f);
                I32(Actor, 6);
                F32(Actor, 7.0f); F32(Actor, 8.0f); F32(Actor, 9.0f);
                I32(Actor, 10);
                U32(Actor, 0);                         // field 34 list
                U32(Actor, 0x11223344); U32(Actor, 0x55667788);
                F32(Actor, 11.5f); I32(Actor, 12); F32(Actor, 13.5f);
                U32(Actor, 0);                         // switch targets
                for (int32 Item = 0; Item < 8; ++Item) U32(Actor, uint32(21 + Item));
                U32(Actor, 0);                         // conditioned blocks
            }
            else if (bGravity)
            {
                // Fall (4-byte sound) and bell (sound, field C, one-target switch).
                U32(Actor, 2);
                U32(Actor, 3); U32(Actor, 4);
                U32(Actor, 4); U32(Actor, 72);
                I32(Actor, 31);                    // fall sound
                I32(Actor, 32);                    // bell sound
                F32(Actor, 1.5f);                  // bell field C
                U32(Actor, 1);                     // one switch target
                for (int32 Item = 0; Item < 7; ++Item) U32(Actor, uint32(41 + Item));
                for (int32 Item = 0; Item < 8; ++Item) U32(Actor, uint32(51 + Item));
            }
            else if (bRune)
            {
                // CBehaviourRune::Create is empty; the record exists.
                U32(Actor, 1);
                U32(Actor, 2); U32(Actor, 0);
            }
            else if (bMicken)
            {
                // Eat (16 bytes) and the empty squashed record.
                U32(Actor, 2);
                U32(Actor, 4); U32(Actor, 16);
                U32(Actor, 5); U32(Actor, 0);
                F32(Actor, 1.5f); F32(Actor, 2.5f);
                I32(Actor, 3); I32(Actor, 21);
            }
            else if (bBasicBox)
            {
                // CBehaviourBasicBoxStand::Create is empty; the record exists.
                U32(Actor, 1);
                U32(Actor, 2); U32(Actor, 0);
            }
            else if (bNativCmd)
            {
                // CBehaviourNativCmdStand::Create is empty; the record exists.
                U32(Actor, 1);
                U32(Actor, 2); U32(Actor, 0);
            }
            else if (bEventGenerator)
            {
                // CBehaviourEventGen: s32 field, f32 field, then raw managers.
                U32(Actor, 1);
                U32(Actor, 2); U32(Actor, 20);
                I32(Actor, 7); F32(Actor, 1.5f);
                for (int32 Item = 0; Item < 3; ++Item) U32(Actor, 0xAABBCCDD);
            }
            else if (bExplosiveDistributor)
            {
                // CBehaviourExplosiveDistributor::Create is empty.
                U32(Actor, 1);
                U32(Actor, 2); U32(Actor, 0);
            }
            else if (bShocker)
            {
                // CBehaviourShockerFireWave: CCircularWaveShoot (6 floats + 2 words).
                U32(Actor, 1);
                U32(Actor, 3); U32(Actor, 32);
                for (int32 Item = 1; Item <= 6; ++Item) F32(Actor, float(Item));
                I32(Actor, 61); I32(Actor, 62);
            }
            else if (bNativShop)
            {
                // CBehaviourNativShopSell: camera, five sub-object refs, path,
                // material, music, zone, three floats (52 bytes).
                U32(Actor, 1);
                U32(Actor, 2); U32(Actor, 52);
                I32(Actor, 71);
                for (int32 Item = 0; Item < 5; ++Item) I32(Actor, 72 + Item);
                I32(Actor, 77); I32(Actor, 78); I32(Actor, 79); I32(Actor, 80);
                F32(Actor, 1.5f); F32(Actor, 2.5f); F32(Actor, 3.5f);
            }
            else if (bPunchingBall)
            {
                // Both fighter default (3) and stand (7) Create bodies are empty.
                U32(Actor, 2);
                U32(Actor, 3); U32(Actor, 0);
                U32(Actor, 7); U32(Actor, 0);
            }
            else if (bNoseMonster)
            {
                // Sleep (2), attack (3) and death (4) Create bodies are empty.
                U32(Actor, 3);
                U32(Actor, 2); U32(Actor, 0);
                U32(Actor, 3); U32(Actor, 0);
                U32(Actor, 4); U32(Actor, 0);
            }
            else if (bShip)
            {
                // CBehaviourShipFLY::Create is empty.
                U32(Actor, 1);
                U32(Actor, 2); U32(Actor, 0);
            }
            else if (bMiniGameBase || bMiniGameTA || bMiniGameBox || bMiniGameBoomy)
            {
                // Training (empty), betting (three bets) and multi (empty).
                U32(Actor, 3);
                U32(Actor, 2); U32(Actor, 0);
                U32(Actor, 3); U32(Actor, 28);
                U32(Actor, 4); U32(Actor, 0);
                U32(Actor, 3); I32(Actor, 11); I32(Actor, 12);
                I32(Actor, 13); I32(Actor, 14); I32(Actor, 15); I32(Actor, 16);
            }
            else if (bBrazul)
            {
                // Empty fighter default/projected, Wolfen track, Brazul A and B.
                U32(Actor, 5);
                U32(Actor, 3); U32(Actor, 0);
                U32(Actor, 4); U32(Actor, 0);
                U32(Actor, 15); U32(Actor, 8);
                U32(Actor, 26); U32(Actor, 36);
                U32(Actor, 27); U32(Actor, 68);
                U32(Actor, 1); F32(Actor, 2.5f);       // track
                U32(Actor, 1);                          // one BrazulA sub-object
                I32(Actor, 61); U32(Actor, 62); F32(Actor, 3.5f);
                U32(Actor, 1); U32(Actor, 63); U32(Actor, 2);
                I32(Actor, 64); I32(Actor, 65);
                I32(Actor, 71);                         // BrazulB waypoint
                U32(Actor, 1);                          // one switch target
                U32(Actor, 72); U32(Actor, 73); U32(Actor, 74); U32(Actor, 75);
                U32(Actor, 76); U32(Actor, 77); U32(Actor, 78);
                U32(Actor, 81); U32(Actor, 82);
                F32(Actor, 1.5f); F32(Actor, 2.5f); F32(Actor, 3.5f);
                U32(Actor, 83); U32(Actor, 84); F32(Actor, 4.5f);
            }
            else if (bBunch)
            {
                // Empty fighter default/projected, Wolfen track, repel and fence.
                U32(Actor, 5);
                U32(Actor, 3); U32(Actor, 0);
                U32(Actor, 4); U32(Actor, 0);
                U32(Actor, 15); U32(Actor, 8);
                U32(Actor, 26); U32(Actor, 84);
                U32(Actor, 27); U32(Actor, 84);
                // Track: flags + track field.
                U32(Actor, 1); F32(Actor, 2.5f);
                // Repel: waypoint, shockwave count/floats, index list, msg hit,
                // two vectors, mesh ids and three final words.
                I32(Actor, 31); U32(Actor, 1);
                F32(Actor, 1.5f); F32(Actor, 2.5f); F32(Actor, 3.5f);
                U32(Actor, 1); I32(Actor, 41);
                F32(Actor, 1.1f); F32(Actor, 2.2f); F32(Actor, 3.3f);
                F32(Actor, 1.0f); F32(Actor, 0.0f); F32(Actor, 0.0f);
                F32(Actor, 0.0f); F32(Actor, 1.0f); F32(Actor, 0.0f);
                I32(Actor, 51); I32(Actor, 52); U32(Actor, 53); U32(Actor, 54); U32(Actor, 55);
                // Fence: refs, two waypoints, one group with two entries, two
                // mesh records and four fx types.
                I32(Actor, 61); I32(Actor, 62);
                U32(Actor, 2); I32(Actor, 63); I32(Actor, 64);
                U32(Actor, 1); U32(Actor, 1); U32(Actor, 71); U32(Actor, 2);
                I32(Actor, 72); I32(Actor, 73); I32(Actor, 74); I32(Actor, 75);
                I32(Actor, 81); I32(Actor, 82); I32(Actor, 83); I32(Actor, 84);
                U32(Actor, 91); U32(Actor, 92); U32(Actor, 93); U32(Actor, 94);
            }
            else if (bPattern)
            {
                // Pattern alone (3) carries CPatternPart; multiple (4) is empty.
                // The table lists every (id, size) pair before the payloads.
                U32(Actor, 2);
                U32(Actor, 3); U32(Actor, 56);
                U32(Actor, 4); U32(Actor, 0);
                F32(Actor, 5.0f); F32(Actor, 1.0f); F32(Actor, 4.0f);
                F32(Actor, 0.3f); F32(Actor, 0.5f); F32(Actor, 0.12f);
                F32(Actor, 0.25f); F32(Actor, 3.0f); F32(Actor, -1.0f);
                I32(Actor, 162);
                F32(Actor, 25.0f); F32(Actor, 2.0f); F32(Actor, 1.0f);
                U32(Actor, 0);
            }
            else U32(Actor, 0);   // behaviours
            U32(Actor, 0);        // models
            const int32 Common = Actor.Num();
            U32(Actor, 0);        // default behaviour
            for (int32 Index = 0; Index < 4; ++Index) F32(Actor, 0.0f);
            F32(Actor, 10.0f);    // clipping
            F32(Actor, 75.0f);    // visibility distance
            F32(Actor, 100.0f);   // culling distance
            F32(Actor, 0.0f);     // field 20
            U32(Actor, 1);        // sector id
            U32(Actor, 0x1234);   // hash code
            F32(Actor, 0.0f);     // lod bias min
            F32(Actor, 0.0f);     // lod bias max
            U32(Actor, 0);
            U32(Actor, 0);        // actor flags
            U32(Actor, 1);        // animation layers
            U32(Actor, 0);        // field 40
            U32(Actor, 0);
            U32(Actor, 0);        // lighting flags
            F32(Actor, 1.0f);     // lighting value
            U32(Actor, 0);        // scenario conditions
            check(Actor.Num() - Common == 84);
            if (bHedgehog)
            {
                F32(Actor, 1.5f);     // weightB
                U32(Actor, 100);      // life maximum
                F32(Actor, 1.0f);     // gravity scale
                F32(Actor, 0.0f);     // dynamic ext 4
                F32(Actor, 0.0f);     // dynamic ext 8
                F32(Actor, 3.0f);     // walk speed
                F32(Actor, 4.0f);     // walk acceleration
                F32(Actor, 5.0f);     // walk rotation speed
                F32(Actor, 6.0f);     // run speed
                U32(Actor, 7);        // field 374
                U32(Actor, 8);        // field 378
                F32(Actor, 9.0f);     // field 368
                F32(Actor, 10.0f);    // field 36C
                F32(Actor, 11.0f);    // field 370
                U32(Actor, 1);        // add-on flags
                U32(Actor, 2);        // add-on money type
                I32(Actor, 3);        // add-on money count
                I32(Actor, 4);        // add-on bonus count
                I32(Actor, 5);        // add-on field C
                U32(Actor, 1);        // add-on sub-object mode
                F32(Actor, 0.25f); F32(Actor, 0.5f); F32(Actor, 0.75f);
                F32(Actor, 1.25f); F32(Actor, 1.5f);
                U32(Actor, 13);       // field 380
            }
            else if (bProjectile)
            {
                F32(Actor, 1.25f);    // weightB
                U32(Actor, 16777217); // life maximum (above float integer precision)
                F32(Actor, 0.5f);     // gravity scale
                F32(Actor, 0.25f);    // dynamic ext 4
                F32(Actor, 0.75f);    // dynamic ext 8
                F32(Actor, 1.0f); F32(Actor, 2.0f); F32(Actor, 3.0f);
                U32(Actor, 4);        // sub-object flags
                F32(Actor, 5.0f); F32(Actor, 6.0f); F32(Actor, 7.0f);
                F32(Actor, 8.0f); F32(Actor, 9.0f); F32(Actor, 10.0f);
                U32(Actor, 11);       // field 28
                U32(Actor, 12);       // material id
                U32(Actor, 13);       // field 30
                U32(Actor, 14);       // field 34
                U32(Actor, 15);       // field 38
                U32(Actor, 16);       // field 3c
                F32(Actor, 17.0f); F32(Actor, 18.0f);
                // Pattern part without the optional field 0x24 (132-byte payload).
                F32(Actor, 1.0f); F32(Actor, 2.0f); F32(Actor, 3.0f);
                F32(Actor, 4.0f); F32(Actor, 5.0f); F32(Actor, 6.0f);
                U32(Actor, 7);        // pattern material id
                F32(Actor, 8.0f);     // point count
                F32(Actor, 9.0f);     // field 20
                U32(Actor, 10);       // field 28
            }
            else if (bSwitch)
            {
                // S_NTF_TARGET_STREAM_REF: one target plus camera.
                U32(Actor, 1);
                for (int32 Item = 0; Item < 7; ++Item) U32(Actor, uint32(31 + Item));
                for (int32 Item = 0; Item < 8; ++Item) U32(Actor, uint32(41 + Item));
            }
            else if (bWind)
            {
                U32(Actor, 0x1234);   // field 160
                for (int32 Item = 0; Item < 10; ++Item) F32(Actor, float(1 + Item));
                U32(Actor, 0x5678);   // field 190
                U32(Actor, 9);        // sound id
                I32(Actor, 3);        // activation zone
                U32(Actor, 0);        // actor stream
                U32(Actor, 5);        // material
                U32(Actor, 8);        // zone 1c4
                U32(Actor, 0x80a0b0c0); // colour 1c8
                U32(Actor, 7);        // waypoint 1cc
                U32(Actor, 0x11223344); U32(Actor, 0x55667788); // sector hierarchy hash
                if (bWindSwitches)
                {
                    // Switch 0: one target + camera. Switches 1/2: empty.
                    U32(Actor, 1);
                    for (int32 Item = 0; Item < 7; ++Item) U32(Actor, uint32(61 + Item));
                    for (int32 Item = 0; Item < 8; ++Item) U32(Actor, uint32(71 + Item));
                    U32(Actor, 0);
                    for (int32 Item = 0; Item < 8; ++Item) U32(Actor, uint32(81 + Item));
                    U32(Actor, 0);
                    for (int32 Item = 0; Item < 8; ++Item) U32(Actor, uint32(91 + Item));
                }
                for (int32 Item = 0; Item < 5; ++Item) F32(Actor, float(11 + Item));
                I32(Actor, 17);       // waypoint ref
                F32(Actor, 18.0f); F32(Actor, 19.0f);
            }
            else if (bTrap)
            {
                F32(Actor, 1.5f);     // weightB
                U32(Actor, 0);        // actor stream
                U32(Actor, 11); U32(Actor, 12); U32(Actor, 13); U32(Actor, 14);
                I32(Actor, 15);       // field 24
                I32(Actor, 16);       // actor ref
                F32(Actor, 2.5f);     // speed
                F32(Actor, 3.5f);     // acceleration
                I32(Actor, 17);       // path follow
                F32(Actor, 4.5f);     // field 4c
                U32(Actor, 5);        // required escapes
                U32(Actor, 1);        // trap stream count
                for (int32 Item = 0; Item < 7; ++Item) U32(Actor, uint32(21 + Item));
                for (int32 Item = 0; Item < 8; ++Item) U32(Actor, uint32(31 + Item));
            }
            else if (bWeapon)
            {
                F32(Actor, 1.5f);   // weightB
                F32(Actor, 2.5f);   // field 1d8
                F32(Actor, 3.5f);   // field 1dc
            }
            else if (bAraignos)
            {
                F32(Actor, 1.25f);   // weightB
                I32(Actor, 3);       // path follow
                F32(Actor, 2.0f); F32(Actor, 3.0f); F32(Actor, 4.0f);
                U32(Actor, 5); U32(Actor, 6); U32(Actor, 7);
                U32(Actor, 1); U32(Actor, 2); I32(Actor, 3); I32(Actor, 4); I32(Actor, 5);
                U32(Actor, 1);       // add-on sub-object mode
                F32(Actor, 0.25f); F32(Actor, 0.5f); F32(Actor, 0.75f);
                F32(Actor, 1.25f); F32(Actor, 1.5f);
            }
            else if (bBonus)
            {
                F32(Actor, 1.5f);   // weightB
                I32(Actor, 9);      // sound ref
                I32(Actor, 10);     // flare material
                I32(Actor, 11);     // animation material
            }
            else if (bTeleporter)
            {
                I32(Actor, 3);       // subsector material
                U32(Actor, 0x11223344); U32(Actor, 0x55667788); // hash
                I32(Actor, 4);       // field 16c
                U32(Actor, 1);       // destination count
                I32(Actor, 5); I32(Actor, 6); I32(Actor, 7); // level, elevator, field 8
                // Switch 0: one target + camera. Switches 1/2: empty.
                U32(Actor, 1);
                for (int32 Item = 0; Item < 7; ++Item) U32(Actor, uint32(21 + Item));
                for (int32 Item = 0; Item < 8; ++Item) U32(Actor, uint32(31 + Item));
                U32(Actor, 0);
                for (int32 Item = 0; Item < 8; ++Item) U32(Actor, uint32(41 + Item));
                U32(Actor, 0);
                for (int32 Item = 0; Item < 8; ++Item) U32(Actor, uint32(51 + Item));
                for (int32 Item = 0; Item < 9; ++Item) I32(Actor, 11 + Item);
                U32(Actor, 2);       // waypoint count
                I32(Actor, 61); I32(Actor, 62);
                I32(Actor, 63); I32(Actor, 64);
                U32(Actor, 1);       // conditioned block count
                U32(Actor, 2);       // operations
                I32(Actor, 71); I32(Actor, 72); I32(Actor, 73);
                I32(Actor, 74); I32(Actor, 75); I32(Actor, 76);
                U32(Actor, 2);       // conditions
                I32(Actor, 81); I32(Actor, 82); I32(Actor, 83); I32(Actor, 84);
                I32(Actor, 85); I32(Actor, 86); I32(Actor, 87); I32(Actor, 88);
            }
            else if (bAmortos)
            {
                U32(Actor, 1); I32(Actor, 11);   // checkpoint stream
                U32(Actor, 1); I32(Actor, 12);   // stream 188
                U32(Actor, 0);                    // stream 18C
                I32(Actor, 13);                   // actor ref 190
                // Serialized order 164,168,160,16C,170,174,178,17C,180.
                for (int32 Item = 1; Item <= 9; ++Item) F32(Actor, float(Item));
                U32(Actor, 1); I32(Actor, 2); I32(Actor, 3); I32(Actor, 4); I32(Actor, 5);
                U32(Actor, 1);                    // add-on sub-object mode
                F32(Actor, 0.25f); F32(Actor, 0.5f); F32(Actor, 0.75f);
                F32(Actor, 1.25f); F32(Actor, 1.5f);
                I32(Actor, 14);                   // field 1c8
                U32(Actor, 1);                    // switch 0: one target
                for (int32 Item = 0; Item < 7; ++Item) U32(Actor, uint32(21 + Item));
                for (int32 Item = 0; Item < 8; ++Item) U32(Actor, uint32(31 + Item));
                U32(Actor, 0);                    // switch 1: empty target list
                for (int32 Item = 0; Item < 8; ++Item) U32(Actor, uint32(41 + Item));
            }
            else if (bDca)
            {
                I32(Actor, 1);        // field 504
                I32(Actor, 2);        // field 500
                F32(Actor, 3.5f);     // field 464
                I32(Actor, 4);        // field 508
                I32(Actor, 5);        // field 50c
                F32(Actor, 6.5f);     // aim adjust
                U32(Actor, 1); I32(Actor, 7);   // fire-shot actor stream
                U32(Actor, 8);        // field 450
                U32(Actor, 9);        // field 454
            }
            else if (bFruit)
            {
                F32(Actor, 1.25f);    // weightB
                U32(Actor, 16777217); // life maximum (above float integer precision)
                F32(Actor, 2.5f);     // gravity scale
                F32(Actor, 0.5f);     // dynamic ext 4
                F32(Actor, 0.75f);    // dynamic ext 8
                U32(Actor, 55);       // field 360
                U32(Actor, 1);        // switch: one target
                for (int32 Item = 0; Item < 7; ++Item) U32(Actor, uint32(31 + Item));
                for (int32 Item = 0; Item < 8; ++Item) U32(Actor, uint32(41 + Item));
            }
            else if (bNativ)
            {
                F32(Actor, 1.25f);    // weightB
                U32(Actor, 16777217); // life maximum (above float integer precision)
                F32(Actor, 2.0f);     // gravity scale
                F32(Actor, 0.5f);     // dynamic ext 4
                F32(Actor, 0.75f);    // dynamic ext 8
                U32(Actor, 11); U32(Actor, 12); U32(Actor, 13);
                F32(Actor, 1.5f); F32(Actor, 2.5f);
                U32(Actor, 14); I32(Actor, 15); I32(Actor, 16);
                U32(Actor, 17); U32(Actor, 18); U32(Actor, 19);
            }
            else if (bWolfen)
            {
                // CActorFighter prefix: autonomous + 156-byte fixed block and
                // empty blow/grab/react/combo databases.
                F32(Actor, 1.25f);    // weightB
                U32(Actor, 16777217); // life maximum (above float integer precision)
                F32(Actor, 2.0f);     // gravity scale
                F32(Actor, 0.5f);     // dynamic ext 4
                F32(Actor, 0.75f);    // dynamic ext 8
                for (int32 Index = 0; Index < 39; ++Index) U32(Actor, 0);
                U32(Actor, 0); U32(Actor, 0); U32(Actor, 0); U32(Actor, 0);
                U32(Actor, 0);        // weapon actor index
                // Wolfen fields.
                U32(Actor, 0);        // field b74
                F32(Actor, 30.0f); F32(Actor, 90.0f); F32(Actor, 10.0f); F32(Actor, 5.0f);
                U32(Actor, 0x1234);   // vision flags
                I32(Actor, 1);        // start sector
                F32(Actor, 100.0f); F32(Actor, 50.0f);   // hearing
                F32(Actor, 10.0f);    // vision detection
                F32(Actor, 3.0f);     // field cf4
                F32(Actor, 4.0f); F32(Actor, 5.0f); F32(Actor, 6.0f); F32(Actor, 1.5f);
                F32(Actor, 7.0f); F32(Actor, 8.0f); F32(Actor, 9.0f);
                U32(Actor, 0);        // reserved word matched by the reader
                U32(Actor, 3);        // magic required
                U32(Actor, 4);        // field d30
                F32(Actor, 1.1f); F32(Actor, 2.2f); F32(Actor, 3.3f);
                // Fight parameters.
                for (int32 Index = 1; Index <= 5; ++Index) U32(Actor, uint32(Index));
                for (int32 Index = 6; Index <= 10; ++Index) U32(Actor, uint32(Index));
                F32(Actor, 0.1f); F32(Actor, 0.2f); F32(Actor, 0.3f); F32(Actor, 0.4f);
                U32(Actor, 0); U32(Actor, 0); U32(Actor, 0);   // capabilities
                U32(Actor, 1); U32(Actor, 11);                 // combo list 1
                U32(Actor, 1); U32(Actor, 12);                 // combo list 2
                U32(Actor, 13); U32(Actor, 14);
                for (int32 Index = 1; Index <= 6; ++Index) F32(Actor, float(Index));
                for (int32 Index = 15; Index <= 18; ++Index) U32(Actor, uint32(Index));
            }
            else if (bCommander)
            {
                U32(Actor, 1); U32(Actor, 2); U32(Actor, 3);
                I32(Actor, 4); I32(Actor, 5);      // new path id, actor ref
                F32(Actor, 1.5f); U32(Actor, 6); U32(Actor, 7);   // squad
                U32(Actor, 1); I32(Actor, 8);      // one team enemy
                U32(Actor, 1); I32(Actor, 9); I32(Actor, 10);     // one component
                for (int32 Index = 1; Index <= 14; ++Index) F32(Actor, float(Index));
            }
            else if (bKim)
            {
                // CActorFighter prefix.
                F32(Actor, 1.25f); U32(Actor, 100); F32(Actor, 2.0f);
                F32(Actor, 0.5f); F32(Actor, 0.75f);
                for (int32 Index = 0; Index < 39; ++Index) U32(Actor, 0);
                U32(Actor, 0); U32(Actor, 0); U32(Actor, 0); U32(Actor, 0);
                U32(Actor, 0);
                // Ten bone keys.
                for (int32 Index = 1; Index <= 10; ++Index) U32(Actor, uint32(Index));
                U32(Actor, 0);        // climb zones
                U32(Actor, 0);        // tutorials
                U32(Actor, 0);        // projectile zones
                U32(Actor, 0);        // boomy pairs
                U32(Actor, 1); I32(Actor, 11);   // one zone stream entry
                U32(Actor, 12); U32(Actor, 13); F32(Actor, 1.5f);  // material/fx/joke
                U32(Actor, 0);        // actor cameras
                U32(Actor, 14); U32(Actor, 15);   // field 15d4, fx init
                U32(Actor, 16); U32(Actor, 17); U32(Actor, 18);   // fx handles
            }
            else if (bBoomy)
            {
                F32(Actor, 1.25f);               // weight
                F32(Actor, 30.0f); F32(Actor, 90.0f); F32(Actor, 10.0f); F32(Actor, 5.0f);
                U32(Actor, 1);                   // vision flags
                U32(Actor, 11); U32(Actor, 12);  // sounds
                F32(Actor, 2.5f);                // field 744
                U32(Actor, 1);                   // tail width axis
                F32(Actor, 3.5f);                // field 740
                U32(Actor, 4);                   // tail segments
                U32(Actor, 13); U32(Actor, 14); U32(Actor, 15); U32(Actor, 16);
                U32(Actor, 17); U32(Actor, 18);  // texture, mesh
                // Camera: switch present (flags & 2), mode 1, extra values.
                F32(Actor, 60.0f);
                F32(Actor, 1.0f); F32(Actor, 2.0f); F32(Actor, 3.0f);
                F32(Actor, 4.0f); F32(Actor, 5.0f); F32(Actor, 6.0f);
                F32(Actor, 0.0f); F32(Actor, 1.0f); F32(Actor, 0.0f);
                U32(Actor, 1); U32(Actor, 2); U32(Actor, 3);
                F32(Actor, 0.0f); F32(Actor, 0.5f); F32(Actor, 0.0f);
                U32(Actor, 21);                  // switch actor
                U32(Actor, 1); I32(Actor, 22);   // list A
                U32(Actor, 0);                   // list B
                F32(Actor, 1.5f); U32(Actor, 2); F32(Actor, 3.5f); U32(Actor, 4);
                F32(Actor, 5.5f);                // switch values
                U32(Actor, 1);                   // camera condition count
                I32(Actor, 1); I32(Actor, 2); I32(Actor, 3); F32(Actor, 4.5f);
                U32(Actor, 5); I32(Actor, 6); I32(Actor, 7); F32(Actor, 8.5f);
                U32(Actor, 1);                   // config mode
                F32(Actor, 0.25f); F32(Actor, 0.5f); F32(Actor, 0.75f);
                F32(Actor, 1.25f); F32(Actor, 1.5f);
                for (int32 Item = 1; Item <= 9; ++Item) F32(Actor, float(Item));
                U32(Actor, 1);                   // extra count
                for (int32 Item = 1; Item <= 12; ++Item) F32(Actor, float(Item));
                U32(Actor, 1);                   // one type
                F32(Actor, 2.5f); F32(Actor, 3.5f); I32(Actor, 19); U32(Actor, 20);
            }
            else if (bAton)
            {
                F32(Actor, 1.25f);               // weightB
                U32(Actor, 16777217);            // life maximum (above float precision)
                F32(Actor, 2.0f);                // gravity scale
                F32(Actor, 0.5f); F32(Actor, 0.75f);
                U32(Actor, 11); U32(Actor, 12); U32(Actor, 13); U32(Actor, 14);
                U32(Actor, 15);                  // effect type
                U32(Actor, 16); U32(Actor, 17);  // texture, mesh
                U32(Actor, 18);                  // trail material
                F32(Actor, 1.5f);                // path default delay
                U32(Actor, 1); I32(Actor, 21);   // one path plane
                U32(Actor, 7);                   // add-on field 18
                U32(Actor, 1);                   // one "other" sub-object
                U32(Actor, 22); U32(Actor, 1); I32(Actor, 23);
                U32(Actor, 1);                   // one Aton sub-object
                U32(Actor, 24); U32(Actor, 25); U32(Actor, 1); I32(Actor, 26);
                F32(Actor, 3.5f);                // add-on field 24
            }
            else if (bRope)
            {
                I32(Actor, 1);                   // path index
                F32(Actor, 1.5f); F32(Actor, 2.5f);   // scale, field 17c
                I32(Actor, 4);                   // material
                U32(Actor, 0x11223344);          // colour
                I32(Actor, 5); I32(Actor, 6);    // sound, field 194
                I32(Actor, 7); I32(Actor, 8);    // actor refs A/B
                U32(Actor, 1);                   // one switch target
                for (int32 Item = 0; Item < 7; ++Item) U32(Actor, uint32(21 + Item));
                for (int32 Item = 0; Item < 8; ++Item) U32(Actor, uint32(31 + Item));
                U32(Actor, 2);                   // rope flags
            }
            else if (bStiller)
            {
                F32(Actor, 30.0f); F32(Actor, 45.0f); F32(Actor, 10.0f); F32(Actor, 5.0f);
                U32(Actor, 1);                   // vision flags
                U32(Actor, 11); U32(Actor, 12); U32(Actor, 13); U32(Actor, 14);
                I32(Actor, 1);                   // path index
                F32(Actor, 2.5f);                // field 1e0
                F32(Actor, 3.5f);                // life base
                F32(Actor, 4.5f); F32(Actor, 5.5f); F32(Actor, 6.5f);
                U32(Actor, 15);                  // field 1f8
                U32(Actor, 1); I32(Actor, 16);   // one actor ref
                I32(Actor, 17);                  // field 200
                U32(Actor, 1);                   // switch 208: one target
                for (int32 Item = 0; Item < 7; ++Item) U32(Actor, uint32(21 + Item));
                for (int32 Item = 0; Item < 8; ++Item) U32(Actor, uint32(31 + Item));
                U32(Actor, 0);                   // switch 210: empty target list
                for (int32 Item = 0; Item < 8; ++Item) U32(Actor, uint32(41 + Item));
                I32(Actor, 18);                  // field 214
                U32(Actor, 1); I32(Actor, 2); I32(Actor, 3); I32(Actor, 4); I32(Actor, 5);
                U32(Actor, 1);                   // add-on sub-object mode
                F32(Actor, 0.25f); F32(Actor, 0.5f); F32(Actor, 0.75f);
                F32(Actor, 1.25f); F32(Actor, 1.5f);
            }
            else if (bAmbre)
            {
                I32(Actor, 1);                   // material
                I32(Actor, 2);                   // effect type
                I32(Actor, 3); I32(Actor, 4);    // texture, mesh
                I32(Actor, 5);                   // field 1d0
                U32(Actor, 1);                   // one switch target
                for (int32 Item = 0; Item < 7; ++Item) U32(Actor, uint32(21 + Item));
                for (int32 Item = 0; Item < 8; ++Item) U32(Actor, uint32(31 + Item));
            }
            else if (bFountain)
            {
                U32(Actor, 1);                   // add-on field 0
                I32(Actor, 2);                   // money type
                I32(Actor, 3);                   // money count
                I32(Actor, 4);                   // bonus count
                I32(Actor, 5);                   // field c
                U32(Actor, 1);                   // sub-object mode
                F32(Actor, 0.25f); F32(Actor, 0.5f); F32(Actor, 0.75f);
                F32(Actor, 1.25f); F32(Actor, 1.5f);
            }
            else if (bCheckpoints)
            {
                U32(Actor, 1);                   // one checkpoint
                I32(Actor, 1);                   // zone ref
                I32(Actor, 2);                   // waypoint A
                I32(Actor, 3);                   // sector id
                I32(Actor, 4);                   // waypoint B
                U32(Actor, 5);                   // flags
                U32(Actor, 1); I32(Actor, 6); I32(Actor, 7);   // one actor/waypoint pair
            }
            else if (bBridge)
            {
                U32(Actor, 1); I32(Actor, 1);    // one actor ref
                I32(Actor, 2); I32(Actor, 3);    // waypoints A/B
                F32(Actor, 1.5f); F32(Actor, 2.5f); F32(Actor, 3.5f);
                F32(Actor, 4.5f); F32(Actor, 5.5f);
                U32(Actor, 6);                   // field 344
                U32(Actor, 7);                   // config flags
                I32(Actor, 8);                   // field 348
                F32(Actor, 9.5f);                // step collision scale
            }
            else if (bFog)
            {
                U32(Actor, 1);                   // one fog hierarchy
                U32(Actor, 0x11223344); U32(Actor, 0x55667788);   // hash
                F32(Actor, 1.5f); F32(Actor, 2.5f);
            }
            else if (bShoot)
            {
                F32(Actor, 1.25f);               // weightB
                U32(Actor, 16777217);            // life maximum (above float precision)
                F32(Actor, 2.0f);                // gravity scale
                F32(Actor, 0.5f); F32(Actor, 0.75f);
                U32(Actor, 11); U32(Actor, 12);  // fields 350/354
                I32(Actor, 13); I32(Actor, 14);  // texture, mesh
                F32(Actor, 1.0f); F32(Actor, 2.0f); F32(Actor, 3.0f); F32(Actor, 4.0f);
                U32(Actor, 15);                  // field 3d0
                F32(Actor, 5.0f); F32(Actor, 6.0f); F32(Actor, 7.0f); F32(Actor, 8.0f);
                U32(Actor, 16); U32(Actor, 17); U32(Actor, 18); U32(Actor, 19);
                U32(Actor, 1); I32(Actor, 2); I32(Actor, 3); I32(Actor, 4); I32(Actor, 5);
                U32(Actor, 1);                   // add-on sub-object mode
                F32(Actor, 0.25f); F32(Actor, 0.5f); F32(Actor, 0.75f);
                F32(Actor, 1.25f); F32(Actor, 1.5f);
            }
            else if (bMoney)
            {
                F32(Actor, 1.25f);               // weightB
                U32(Actor, 100);                 // money value
                I32(Actor, 5);                   // sound ref
                I32(Actor, 6);                   // material
            }
            else if (bBoxClass)
            {
                F32(Actor, 1.25f);               // weightB
                U32(Actor, 16777217);            // life maximum
                F32(Actor, 2.0f); F32(Actor, 0.5f); F32(Actor, 0.75f);
                I32(Actor, 11);                  // field 350
                U32(Actor, 1); I32(Actor, 21);   // one actor ref
                F32(Actor, 1.0f); F32(Actor, 2.0f); F32(Actor, 3.0f); F32(Actor, 4.0f);
                U32(Actor, 12);                  // field 364
                I32(Actor, 13); I32(Actor, 14); I32(Actor, 15); I32(Actor, 16);
                U32(Actor, 1);                   // switch: one target
                for (int32 Item = 0; Item < 7; ++Item) U32(Actor, uint32(31 + Item));
                for (int32 Item = 0; Item < 8; ++Item) U32(Actor, uint32(41 + Item));
                U32(Actor, 1); I32(Actor, 2); I32(Actor, 3); I32(Actor, 4); I32(Actor, 5);
                U32(Actor, 1);                   // add-on sub-object mode
                F32(Actor, 0.25f); F32(Actor, 0.5f); F32(Actor, 0.75f);
                F32(Actor, 1.25f); F32(Actor, 1.5f);
            }
            else if (bFx)
            {
                F32(Actor, 0.5f);                // field 19c
                F32(Actor, 1.5f);                // field 1a0
            }
            else if (bCluster)
            {
                // CActorClusteriser has no class-specific payload.
            }
            else if (bCompanion)
            {
                F32(Actor, 1.25f);               // weightB
                U32(Actor, 16777217);            // life maximum
                F32(Actor, 2.0f); F32(Actor, 0.5f); F32(Actor, 0.75f);
                I32(Actor, 11); I32(Actor, 12); I32(Actor, 13);
            }
            else if (bGravity)
            {
                F32(Actor, 1.25f);               // weightB
                U32(Actor, 16777217);            // life maximum
                F32(Actor, 2.0f);                // gravity scale
                F32(Actor, 0.5f); F32(Actor, 0.75f);
            }
            else if (bRune)
            {
                I32(Actor, 1);                   // cinematic
                F32(Actor, 2.5f);                // trigger distance
                I32(Actor, 3);                   // scenario variable
                I32(Actor, 4);                   // fx
                I32(Actor, 5); I32(Actor, 6);    // materials A/B
            }
            else if (bMicken)
            {
                F32(Actor, 1.25f);               // weightB
                U32(Actor, 16777217);            // life maximum
                F32(Actor, 2.0f);                // gravity scale
                F32(Actor, 0.5f); F32(Actor, 0.75f);
                I32(Actor, 11); I32(Actor, 12);  // waypoint, actor
                I32(Actor, 13);                  // field 358
                F32(Actor, 3.5f);                // field 35c
                I32(Actor, 14);                  // field 360
            }
            else if (bBasicBox)
            {
                I32(Actor, 1);                   // field 160
                U32(Actor, 16777217);            // life maximum
                U32(Actor, 1); I32(Actor, 21);   // one actor ref
                F32(Actor, 1.0f); F32(Actor, 2.0f); F32(Actor, 3.0f); F32(Actor, 4.0f);
                U32(Actor, 11);                  // field 180
                I32(Actor, 12); I32(Actor, 13); I32(Actor, 14); I32(Actor, 15);
                U32(Actor, 1);                   // switch: one target
                for (int32 Item = 0; Item < 7; ++Item) U32(Actor, uint32(31 + Item));
                for (int32 Item = 0; Item < 8; ++Item) U32(Actor, uint32(41 + Item));
                U32(Actor, 1); I32(Actor, 2); I32(Actor, 3); I32(Actor, 4); I32(Actor, 5);
                U32(Actor, 1);                   // add-on sub-object mode
                F32(Actor, 0.25f); F32(Actor, 0.5f); F32(Actor, 0.75f);
                F32(Actor, 1.25f); F32(Actor, 1.5f);
            }
            else if (bNativCmd)
            {
                U32(Actor, 1); I32(Actor, 21);   // one nativ actor ref
                I32(Actor, 11);                  // field 164
                U32(Actor, 1); U32(Actor, 22); U32(Actor, 1); I32(Actor, 23);   // A
                U32(Actor, 0);                   // B
                U32(Actor, 1); U32(Actor, 24); U32(Actor, 1); I32(Actor, 25);   // C
                U32(Actor, 0);                   // D
            }
            else if (bEventGenerator)
            {
                U32(Actor, 1);                   // global flag
                I32(Actor, 2);                   // zone reference
                U32(Actor, 0xAABBCCDD); U32(Actor, 0x11223344);   // raw manager tail
            }
            else if (bExplosiveDistributor)
            {
                F32(Actor, 5.0f); F32(Actor, 1.25f);   // radius, angle
                U32(Actor, 2);                   // two projectiles
                I32(Actor, 31); F32(Actor, 1.0f); F32(Actor, 2.0f); F32(Actor, 3.0f); F32(Actor, 4.5f);
                I32(Actor, 32); F32(Actor, 0.0f); F32(Actor, 1.0f); F32(Actor, 0.0f); F32(Actor, 6.5f);
                I32(Actor, 41); U32(Actor, 2);   // camera index, flags
                F32(Actor, 1.5f); F32(Actor, 2.5f); F32(Actor, 3.5f);
                I32(Actor, 33); I32(Actor, 34); F32(Actor, 4.5f);
            }
            else if (bMiniGameBase || bMiniGameTA || bMiniGameBox || bMiniGameBoomy)
            {
                U32(Actor, 41); U32(Actor, 42);         // field 160
                U32(Actor, 43); U32(Actor, 44);         // fields 168/16c
                U32(Actor, 1); I32(Actor, 51);          // actor list
                U32(Actor, 2); F32(Actor, 1.5f); F32(Actor, 2.5f);   // float list
                U32(Actor, 2);                          // two scores
                F32(Actor, 100.0f);
                const char* ScoreA = "ABC";
                for (int32 Char = 0; Char <= 3; ++Char) Actor.Add(uint8(ScoreA[Char]));
                F32(Actor, 200.0f);
                const char* ScoreB = "DE";
                for (int32 Char = 0; Char <= 2; ++Char) Actor.Add(uint8(ScoreB[Char]));
                Actor.Add(0);
                F32(Actor, 50.0f);                      // default score
                const char* DefaultName = "NAT";
                for (int32 Char = 0; Char <= 3; ++Char) Actor.Add(uint8(DefaultName[Char]));
                I32(Actor, 61);                         // waypoint ref
                I32(Actor, 62);                         // field 190
                U32(Actor, 63); U32(Actor, 64);         // field 1a8
                U32(Actor, 1);                          // one switch target
                U32(Actor, 71); U32(Actor, 72); U32(Actor, 73); U32(Actor, 74);
                U32(Actor, 75); U32(Actor, 76); U32(Actor, 77);
                U32(Actor, 81); U32(Actor, 82);
                F32(Actor, 1.5f); F32(Actor, 2.5f); F32(Actor, 3.5f);
                U32(Actor, 83); U32(Actor, 84); F32(Actor, 4.5f);
                if (bMiniGameBase)
                {
                    // No variant tail.
                }
                else if (bMiniGameBox)
                {
                    F32(Actor, 20.0f);                  // box-counter time limit
                }
                else if (bMiniGameTA)
                {
                    I32(Actor, 91);                     // time-attack field 0x1e0
                }
                else
                {
                    U32(Actor, 1);                      // one Boomy target
                    I32(Actor, 101);
                    U32(Actor, 1);
                    U32(Actor, 111); U32(Actor, 112); U32(Actor, 113); U32(Actor, 114);
                    U32(Actor, 115); U32(Actor, 116); U32(Actor, 117);
                    U32(Actor, 121); U32(Actor, 122);
                    F32(Actor, 5.5f); F32(Actor, 6.5f); F32(Actor, 7.5f);
                    U32(Actor, 123); U32(Actor, 124); F32(Actor, 8.5f);
                    U32(Actor, 1);                      // one penalty target
                    I32(Actor, 141);
                    U32(Actor, 1);
                    U32(Actor, 151); U32(Actor, 152); U32(Actor, 153); U32(Actor, 154);
                    U32(Actor, 155); U32(Actor, 156); U32(Actor, 157);
                    U32(Actor, 161); U32(Actor, 162);
                    F32(Actor, 9.5f); F32(Actor, 10.5f); F32(Actor, 11.5f);
                    U32(Actor, 163); U32(Actor, 164); F32(Actor, 12.5f);
                    F32(Actor, 20.0f);                  // time limit
                    I32(Actor, 3);                      // max penalties
                    U32(Actor, 5);                      // game flags
                    U32(Actor, 1);                      // one sequence
                    U32(Actor, 2); I32(Actor, 131); I32(Actor, 132);
                }
            }
            else if (bBrazul)
            {
                // CActorFighter prefix: autonomous + 156-byte fixed block and
                // empty blow/grab/react/combo databases.
                F32(Actor, 1.25f);    // weightB
                U32(Actor, 16777217); // life maximum (above float integer precision)
                F32(Actor, 2.0f);     // gravity scale
                F32(Actor, 0.5f);     // dynamic ext 4
                F32(Actor, 0.75f);    // dynamic ext 8
                for (int32 Index = 0; Index < 39; ++Index) U32(Actor, 0);
                U32(Actor, 0); U32(Actor, 0); U32(Actor, 0); U32(Actor, 0);
                U32(Actor, 0);        // weapon actor index
                // Wolfen fields.
                U32(Actor, 0);        // field b74
                F32(Actor, 30.0f); F32(Actor, 90.0f); F32(Actor, 10.0f); F32(Actor, 5.0f);
                U32(Actor, 0x1234);   // vision flags
                I32(Actor, 1);        // start sector
                F32(Actor, 100.0f); F32(Actor, 50.0f);   // hearing
                F32(Actor, 10.0f);    // vision detection
                F32(Actor, 3.0f);     // field cf4
                F32(Actor, 4.0f); F32(Actor, 5.0f); F32(Actor, 6.0f); F32(Actor, 1.5f);
                F32(Actor, 7.0f); F32(Actor, 8.0f); F32(Actor, 9.0f);
                U32(Actor, 0);        // reserved word matched by the reader
                U32(Actor, 3);        // magic required
                U32(Actor, 4);        // field d30
                F32(Actor, 1.1f); F32(Actor, 2.2f); F32(Actor, 3.3f);
                // Fight parameters.
                for (int32 Index = 1; Index <= 5; ++Index) U32(Actor, uint32(Index));
                for (int32 Index = 6; Index <= 10; ++Index) U32(Actor, uint32(Index));
                F32(Actor, 0.1f); F32(Actor, 0.2f); F32(Actor, 0.3f); F32(Actor, 0.4f);
                U32(Actor, 0); U32(Actor, 0); U32(Actor, 0);   // capabilities
                U32(Actor, 1); U32(Actor, 11);                 // combo list 1
                U32(Actor, 1); U32(Actor, 12);                 // combo list 2
                U32(Actor, 13); U32(Actor, 14);
                for (int32 Index = 1; Index <= 6; ++Index) F32(Actor, float(Index));
                for (int32 Index = 15; Index <= 18; ++Index) U32(Actor, uint32(Index));                F32(Actor, 1.5f); F32(Actor, 2.5f);   // fields 0xfc0/0xfc4
                I32(Actor, 31);                        // path reader index
                U32(Actor, 32);                        // field 0x1060
                U32(Actor, 33);                        // reserved word
                I32(Actor, 34);                        // material id
                U32(Actor, 35); U32(Actor, 36);        // spark params
                U32(Actor, 41); U32(Actor, 42); U32(Actor, 43);   // fx handles
                U32(Actor, 44);                        // fx init
                U32(Actor, 45); U32(Actor, 46);        // two more fx handles
                I32(Actor, 51); I32(Actor, 52);        // mesh A
                I32(Actor, 53); I32(Actor, 54);        // mesh B
            }
            else if (bBunch)
            {
                // CActorFighter prefix: autonomous + 156-byte fixed block and
                // empty blow/grab/react/combo databases.
                F32(Actor, 1.25f);    // weightB
                U32(Actor, 16777217); // life maximum (above float integer precision)
                F32(Actor, 2.0f);     // gravity scale
                F32(Actor, 0.5f);     // dynamic ext 4
                F32(Actor, 0.75f);    // dynamic ext 8
                for (int32 Index = 0; Index < 39; ++Index) U32(Actor, 0);
                U32(Actor, 0); U32(Actor, 0); U32(Actor, 0); U32(Actor, 0);
                U32(Actor, 0);        // weapon actor index
                // Wolfen fields.
                U32(Actor, 0);        // field b74
                F32(Actor, 30.0f); F32(Actor, 90.0f); F32(Actor, 10.0f); F32(Actor, 5.0f);
                U32(Actor, 0x1234);   // vision flags
                I32(Actor, 1);        // start sector
                F32(Actor, 100.0f); F32(Actor, 50.0f);   // hearing
                F32(Actor, 10.0f);    // vision detection
                F32(Actor, 3.0f);     // field cf4
                F32(Actor, 4.0f); F32(Actor, 5.0f); F32(Actor, 6.0f); F32(Actor, 1.5f);
                F32(Actor, 7.0f); F32(Actor, 8.0f); F32(Actor, 9.0f);
                U32(Actor, 0);        // reserved word matched by the reader
                U32(Actor, 3);        // magic required
                U32(Actor, 4);        // field d30
                F32(Actor, 1.1f); F32(Actor, 2.2f); F32(Actor, 3.3f);
                // Fight parameters.
                for (int32 Index = 1; Index <= 5; ++Index) U32(Actor, uint32(Index));
                for (int32 Index = 6; Index <= 10; ++Index) U32(Actor, uint32(Index));
                F32(Actor, 0.1f); F32(Actor, 0.2f); F32(Actor, 0.3f); F32(Actor, 0.4f);
                U32(Actor, 0); U32(Actor, 0); U32(Actor, 0);   // capabilities
                U32(Actor, 1); U32(Actor, 11);                 // combo list 1
                U32(Actor, 1); U32(Actor, 12);                 // combo list 2
                U32(Actor, 13); U32(Actor, 14);
                for (int32 Index = 1; Index <= 6; ++Index) F32(Actor, float(Index));
                for (int32 Index = 15; Index <= 18; ++Index) U32(Actor, uint32(Index));                F32(Actor, 1.5f);                      // field 0xfc8
                U32(Actor, 2);                          // two stream entries
                U32(Actor, 11); U32(Actor, 0); U32(Actor, 12); U32(Actor, 13);
                U32(Actor, 14); U32(Actor, 1); U32(Actor, 15); U32(Actor, 16); U32(Actor, 17);
                I32(Actor, 21); I32(Actor, 22);         // mesh component, mesh index
            }
            else if (bShip)
            {
                U32(Actor, 2); I32(Actor, 21); I32(Actor, 22);   // care-boy actors
                U32(Actor, 2); I32(Actor, 23); I32(Actor, 24);   // fx ints
                I32(Actor, 31); I32(Actor, 32); I32(Actor, 33);  // jaw/air cannon/hunter
                for (int32 Index = 1; Index <= 9; ++Index) F32(Actor, float(Index) + 0.1f);
                I32(Actor, 41); I32(Actor, 42); I32(Actor, 43); I32(Actor, 44);
                I32(Actor, 51); I32(Actor, 52); I32(Actor, 53); I32(Actor, 54);
                I32(Actor, 61); I32(Actor, 62); I32(Actor, 63); I32(Actor, 64);
                F32(Actor, 2.1f); F32(Actor, 2.2f); F32(Actor, 2.3f); F32(Actor, 2.4f);
                U32(Actor, 65);                                  // vision flags
                U32(Actor, 1);                                   // one switch target
                U32(Actor, 71); U32(Actor, 72); U32(Actor, 73); U32(Actor, 74);
                U32(Actor, 75); U32(Actor, 76); U32(Actor, 77);
                U32(Actor, 81); U32(Actor, 82);
                F32(Actor, 1.5f); F32(Actor, 2.5f); F32(Actor, 3.5f);
                U32(Actor, 83); U32(Actor, 84); F32(Actor, 4.5f);
            }
            else if (bPattern)
            {
                F32(Actor, 1.25f); U32(Actor, 100); F32(Actor, 2.0f);
                F32(Actor, 0.5f); F32(Actor, 0.75f);
                F32(Actor, 10.0f); F32(Actor, 10.0f); F32(Actor, 6.0f); F32(Actor, 12.0f);
                F32(Actor, 0.6f); F32(Actor, 1.6f); F32(Actor, 3.0f);
                I32(Actor, 46); I32(Actor, 140);
                U32(Actor, 0x8079674D);
                // Add-on generator: five header words, mode 0 (no sub-object floats).
                U32(Actor, 1); U32(Actor, 2); I32(Actor, 3); I32(Actor, 4); I32(Actor, 5);
                U32(Actor, 0);
            }
            else if (bNoseMonster)
            {
                // Autonomous prefix.
                F32(Actor, 1.25f); U32(Actor, 100); F32(Actor, 2.0f);
                F32(Actor, 0.5f); F32(Actor, 0.75f);
                I32(Actor, 11);                        // zone ref
                F32(Actor, 1.1f); F32(Actor, 1.2f); F32(Actor, 1.3f); F32(Actor, 1.4f); F32(Actor, 1.5f);
                F32(Actor, 1.6f); F32(Actor, 1.7f);    // scene >= 2.21 floats
                U32(Actor, 12);                        // field 364
                F32(Actor, 1.8f); F32(Actor, 1.9f);    // fields 370/374
                U32(Actor, 1); I32(Actor, 13); I32(Actor, 14);   // one zone path
                U32(Actor, 15);                        // field 36c
                U32(Actor, 2); I32(Actor, 16); I32(Actor, 17);   // fire-shot actors
                I32(Actor, 18);                        // field 630
                U32(Actor, 19);                        // field 380
                F32(Actor, 2.1f); F32(Actor, 2.2f); F32(Actor, 2.3f); F32(Actor, 2.4f);
                U32(Actor, 20); U32(Actor, 21); U32(Actor, 22);  // fields 6a0/6a4/6a8
                U32(Actor, 1);                         // one switch target
                U32(Actor, 31); U32(Actor, 32); U32(Actor, 33); U32(Actor, 34);
                U32(Actor, 35); U32(Actor, 36); U32(Actor, 37);
                U32(Actor, 41); U32(Actor, 42);
                F32(Actor, 1.5f); F32(Actor, 2.5f); F32(Actor, 3.5f);
                U32(Actor, 43); U32(Actor, 44); F32(Actor, 4.5f);
            }
            else if (bPunchingBall)
            {
                // CActorFighter prefix (same shape as the Wolfen/Kim fixtures).
                F32(Actor, 1.25f); U32(Actor, 100); F32(Actor, 2.0f);
                F32(Actor, 0.5f); F32(Actor, 0.75f);
                for (int32 Index = 0; Index < 39; ++Index) U32(Actor, 0);
                U32(Actor, 0); U32(Actor, 0); U32(Actor, 0); U32(Actor, 0);
                U32(Actor, 0);
                // Four tuning floats.
                F32(Actor, 10.0f); F32(Actor, 20.0f); F32(Actor, 30.0f); F32(Actor, 40.0f);
                // One S_TRAP_STREAM_ENTRY: seven on/off words + camera.
                U32(Actor, 1);
                U32(Actor, 51); U32(Actor, 52); U32(Actor, 53); U32(Actor, 54);
                U32(Actor, 55); U32(Actor, 56); U32(Actor, 57);
                U32(Actor, 61); U32(Actor, 62);
                F32(Actor, 1.5f); F32(Actor, 2.5f); F32(Actor, 3.5f);
                U32(Actor, 63); U32(Actor, 64); F32(Actor, 4.5f);
                // Two actor references, then fourteen camera-fight floats.
                I32(Actor, 71); I32(Actor, 72);
                for (int32 Index = 1; Index <= 14; ++Index) F32(Actor, float(Index));
            }
            else if (bNativShop)
            {
                F32(Actor, 2.5f);                // radius
                const char* Name = "NATIV_SHOP"; // 10 chars + NUL, aligned to 12
                for (int32 Char = 0; Char <= 10; ++Char) Actor.Add(uint8(Name[Char]));
                Actor.Add(0);
                U32(Actor, 2);                   // two actor refs
                I32(Actor, 31); I32(Actor, 32);
                U32(Actor, 41);                  // episode threshold link
                U32(Actor, 42);                  // field 174
                I32(Actor, 43);                  // actor ref
            }
            else if (bShocker)
            {
                F32(Actor, 1.25f);               // weightB
                U32(Actor, 16777217);            // life maximum
                F32(Actor, 2.0f); F32(Actor, 0.5f); F32(Actor, 0.75f);
                for (int32 Item = 1; Item <= 9; ++Item) F32(Actor, float(Item));
                I32(Actor, 11); I32(Actor, 12);  // zone inner/outer
                U32(Actor, 1);                   // switch: one target
                for (int32 Item = 0; Item < 7; ++Item) U32(Actor, uint32(21 + Item));
                for (int32 Item = 0; Item < 8; ++Item) U32(Actor, uint32(31 + Item));
                U32(Actor, 41); I32(Actor, 42); I32(Actor, 43); I32(Actor, 44);
                I32(Actor, 45); I32(Actor, 46); I32(Actor, 47); I32(Actor, 48);
                U32(Actor, 1); I32(Actor, 2); I32(Actor, 3); I32(Actor, 4); I32(Actor, 5);
                U32(Actor, 1);                   // add-on sub-object mode
                F32(Actor, 0.25f); F32(Actor, 0.5f); F32(Actor, 0.75f);
                F32(Actor, 1.25f); F32(Actor, 1.5f);
            }
            else
            {
            // Class 0x0d payload: weight, empty carried/zone lists, one no-friction
            // zone with two carried actors, then full platform details.
            F32(Actor, 1.0f);
            U32(Actor, 0);
            U32(Actor, 0);
            U32(Actor, 1);
            U32(Actor, 42);
            U32(Actor, 2);
            U32(Actor, 7);
            U32(Actor, 8);
            U32(Actor, 0);        // camera records
            I32(Actor, -1);       // sound ref A
            I32(Actor, -1);       // sample ref A
            I32(Actor, -1);       // sound ref B
            I32(Actor, -1);       // sample ref B
            for (int32 Index = 0; Index < 3; ++Index) F32(Actor, 0.0f);
            U32(Actor, 0);        // effect ref 1C
            U32(Actor, 0);        // effect ref 20
            U32(Actor, 14);       // platform flags
            I32(Actor, -1);       // camera index
            F32(Actor, 50.0f);    // tilt value
            I32(Actor, -1);       // tilt mode
            for (int32 Index = 0; Index < 4; ++Index) F32(Actor, 0.0f);
            F32(Actor, 0.0f);     // push oscillation X
            F32(Actor, 0.0f);     // push oscillation Y
            U32(Actor, 0);        // sound list A
            U32(Actor, 0);        // sound list B
            }
            check(Actor.Num() >= 4);
            const uint32 ActorSize = uint32(Actor.Num());
            for (int32 Byte = 0; Byte < 4; ++Byte) Actor[4 + Byte] = uint8(ActorSize >> (Byte * 8));
            Scene.Append(Actor);

            // Scene configuration (6/2): the exporter requires exactly one and
            // patches it in place, so include a complete minimal block.
            TArray<uint8> Config;
            Config.AddZeroed(16);
            F32(Config, 0.0f);    // field 1C
            U32(Config, 1);       // field 20 must fit a u32
            F32(Config, 0.0f);    // field 24
            U32(Config, 0);       // header word 0
            U32(Config, 0);       // header word 1
            U32(Config, 0);       // common mesh count
            I32(Config, 0);       // common texture count
            I32(Config, 0);       // texture word
            U32(Config, 0);       // collision bank count
            I32(Config, 0);       // level animation count
            for (int32 Index = 0; Index < 4; ++Index) U32(Config, 0);  // post-animation words
            U32(Config, 0);       // actor class allocation count
            U32(Config, 0);       // particle word

            // Assemble header, entry table, type table and the two payloads.
            const int32 HeaderSize = 0x48;
            const int32 TableOffset = HeaderSize;
            const int32 EntryCount = 4;
            const int32 TypeOffset = TableOffset + EntryCount * 16;
            const int32 SceneOffset = TypeOffset + EntryCount * 4;
            const int32 ConfigOffset = SceneOffset + Scene.Num();
            const int32 EmptyOffset = ConfigOffset + Config.Num();
            TArray<uint8> Bytes;
            Bytes.SetNumZeroed(HeaderSize);
            Bytes[8] = 'K'; Bytes[9] = 'N'; Bytes[10] = 'A'; Bytes[11] = 'B';
            auto WriteHeader = [&Bytes](int32 Offset, uint32 Value) {
                for (int32 Byte = 0; Byte < 4; ++Byte) Bytes[Offset + Byte] = uint8(Value >> (Byte * 8));
            };
            WriteHeader(0x0c, 0);
            WriteHeader(0x14, 0);
            WriteHeader(0x28, EntryCount);
            WriteHeader(0x30, uint32(TableOffset - 8));
            WriteHeader(0x34, uint32(TypeOffset - 8));
            Bytes.SetNum(SceneOffset);
            auto WriteEntry = [&Bytes](int32 Entry, uint32 Offset, uint32 Size) {
                for (int32 Byte = 0; Byte < 4; ++Byte) Bytes[Entry + Byte] = uint8(Offset >> (Byte * 8));
                for (int32 Byte = 0; Byte < 4; ++Byte) Bytes[Entry + 4 + Byte] = uint8(Size >> (Byte * 8));
            };
            WriteEntry(TableOffset, uint32(SceneOffset), uint32(Scene.Num()));
            WriteEntry(TableOffset + 16, uint32(ConfigOffset), uint32(Config.Num()));
            WriteEntry(TableOffset + 32, uint32(EmptyOffset), 0);
            WriteEntry(TableOffset + 48, uint32(EmptyOffset), 0);
            const uint32 TypeWords[4] = {
                (6u << 16) | 1u, (6u << 16) | 2u, (4u << 16) | 1u, (5u << 16) | 1u };
            for (int32 Index = 0; Index < EntryCount; ++Index)
                for (int32 Byte = 0; Byte < 4; ++Byte)
                    Bytes[TypeOffset + Index * 4 + Byte] = uint8(TypeWords[Index] >> (Byte * 8));
            Bytes.Append(Scene);
            Bytes.Append(Config);
            return Bytes;
        }
    };
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKyaSmallBankRoundTripTest, "Kya.Bank.SmallBankRoundTrip",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKyaSmallBankRoundTripTest::RunTest(const FString& Parameters)
{
    FKyaSmallBankBuilder Builder;
    TArray<uint8> Input = Builder.Build();
    AddInfo(TEXT("Synthetic fixture: bytes are built by FKyaSmallBankBuilder, not read from a real game bank. "
        "Real-bank coverage is Kya.Bank.LevelOneRoundTrip and Kya.Bank.SelectedActorRoundTrip."));
    // Emit the constructed asset so it can be imported for manual inspection.
    const FString FixturePath = FPaths::ProjectSavedDir() / TEXT("KyaSmallBank.BNK");
    if (TestTrue(TEXT("Write small bank asset"), FFileHelper::SaveArrayToFile(Input, *FixturePath)))
        AddInfo(FString::Printf(TEXT("Small bank asset: %s (%d bytes)"), *FixturePath, Input.Num()));
    UKyaLevelBankFactory* Factory = NewObject<UKyaLevelBankFactory>();
    const uint8* Cursor = Input.GetData();
    UKyaLevelBank* Bank = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
        GetTransientPackage(), NAME_None, RF_Transient, nullptr, TEXT("bnk"), Cursor,
        Input.GetData() + Input.Num(), GWarn));
    if (!TestNotNull(TEXT("Import small bank"), Bank)) return false;
    TestEqual(TEXT("Small bank entry count"), Bank->Entries.Num(), 4);
    TestEqual(TEXT("Small bank scene configuration"), Bank->SceneConfiguration.Field20, int64(1));
    if (!TestEqual(TEXT("Small bank actor count"), Bank->SceneEnvironment.Actors.Num(), 1)) return false;
    FKyaActorPlacement& Actor = Bank->SceneEnvironment.Actors[0];
    TestEqual(TEXT("Small bank actor name"), Actor.Name, FString(TEXT("PLATFORM")));
    TestEqual(TEXT("Small bank actor class"), Actor.ClassId, int64(0x0d));
    TestEqual(TEXT("Small bank actor position"), Actor.Position, FVector(1.0, 2.0, 3.0));
    TestEqual(TEXT("Small bank actor sector"), Actor.SectorId, 1);
    TestTrue(TEXT("Small bank platform prefix parses"), Actor.bHasPlatformPrefix);
    TestTrue(TEXT("Small bank platform details parse"), Actor.bHasPlatformDetails);
    TestEqual(TEXT("Small bank platform weight"), Actor.PlatformWeight, 1.0f);
    TestEqual(TEXT("Small bank leaves no raw class data"), Actor.ClassData.Num(), 0);
    TestEqual(TEXT("Small bank platform flags"), Actor.PlatformProperties.PlatformFlags, int64(14));
    if (!TestEqual(TEXT("Small bank no-friction zones"), Actor.PlatformNoFrictionZones.Num(), 1)) return false;
    TestEqual(TEXT("Small bank no-friction ref"), Actor.PlatformNoFrictionZones[0].ZoneRef, 42);
    if (!TestEqual(TEXT("Small bank no-friction actors"), Actor.PlatformNoFrictionZones[0].Actors.Num(), 2)) return false;
    TestEqual(TEXT("Small bank no-friction actor 0"), Actor.PlatformNoFrictionZones[0].Actors[0], 7);
    TestEqual(TEXT("Small bank no-friction actor 1"), Actor.PlatformNoFrictionZones[0].Actors[1], 8);

    UKyaLevelBankExporter* Exporter = NewObject<UKyaLevelBankExporter>();
    TArray<uint8> Plain;
    FMemoryWriter PlainWriter(Plain);
    if (!TestTrue(TEXT("Export unedited small bank"), Exporter->ExportBinary(Bank, TEXT("bnk"), PlainWriter, GWarn))) return false;
    TestEqual(TEXT("Unedited export keeps the file size"), Plain.Num(), Input.Num());
    TestTrue(TEXT("Unedited export preserves every byte"), Plain == Input);

    Actor.PlatformNoFrictionZones[0].ZoneRef = 99;
    Actor.PlatformNoFrictionZones[0].Actors[0] = 1234;
    Actor.PlatformWeight = 2.5f;
    Actor.PlatformProperties.PlatformFlags = 99;
    TArray<uint8> Edited;
    FMemoryWriter EditedWriter(Edited);
    if (!TestTrue(TEXT("Export edited small bank"), Exporter->ExportBinary(Bank, TEXT("bnk"), EditedWriter, GWarn))) return false;
    TestEqual(TEXT("Edited export keeps the file size"), Edited.Num(), Input.Num());
    int32 ChangedBytes = 0;
    for (int32 Index = 0; Index < Input.Num(); ++Index)
        if (Input[Index] != Edited[Index]) ++ChangedBytes;
    // 2 bytes of weight, 1 of no-friction ref, 2 of the first carried actor and
    // 1 of the platform flags differ; every other byte is untouched.
    TestEqual(TEXT("Edited export touches only the edited fields"), ChangedBytes, 6);

    const uint8* EditedCursor = Edited.GetData();
    UKyaLevelBank* Reimported = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
        GetTransientPackage(), NAME_None, RF_Transient, nullptr, TEXT("bnk"), EditedCursor,
        Edited.GetData() + Edited.Num(), GWarn));
    if (!TestNotNull(TEXT("Reimport edited small bank"), Reimported)) return false;
    const FKyaActorPlacement& EditedActor = Reimported->SceneEnvironment.Actors[0];
    TestEqual(TEXT("Edited no-friction ref"), EditedActor.PlatformNoFrictionZones[0].ZoneRef, 99);
    TestEqual(TEXT("Edited no-friction actor"), EditedActor.PlatformNoFrictionZones[0].Actors[0], 1234);
    TestEqual(TEXT("Edited platform weight"), EditedActor.PlatformWeight, 2.5f);
    TestEqual(TEXT("Edited platform flags"), EditedActor.PlatformProperties.PlatformFlags, int64(99));

    const TArray<FKyaScenarioCondition> OriginalConditions = Actor.Conditions;
    FKyaScenarioCondition& AddedCondition = Actor.Conditions.AddDefaulted_GetRef();
    AddedCondition.Variable = 17;
    AddedCondition.Comparison = 4;
    AddedCondition.Value = 1;
    for (int32 Pass = 0; Pass < 2; ++Pass)
    {
        TArray<uint8> Resized;
        FMemoryWriter ResizedWriter(Resized);
        if (!TestTrue(TEXT("Export resized actor conditions"),
            Exporter->ExportBinary(Bank, TEXT("bnk"), ResizedWriter, GWarn))) return false;
        const uint8* ResizedCursor = Resized.GetData();
        UKyaLevelBank* RoundTrip = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(
            UKyaLevelBank::StaticClass(), GetTransientPackage(), NAME_None, RF_Transient,
            nullptr, TEXT("bnk"), ResizedCursor, Resized.GetData() + Resized.Num(), GWarn));
        if (!TestNotNull(TEXT("Reimport resized conditions"), RoundTrip)) return false;
        const FKyaActorPlacement& Result = RoundTrip->SceneEnvironment.Actors[0];
        TestEqual(TEXT("Condition count round trips"), Result.Conditions.Num(), Actor.Conditions.Num());
        if (Pass == 0 && !Result.Conditions.IsEmpty())
            TestEqual(TEXT("Added condition value round trips"), Result.Conditions.Last().Variable, 17);
        TestEqual(TEXT("Class payload survives relocation"), Result.PlatformWeight, Actor.PlatformWeight);
        TestTrue(TEXT("Export preserves asset source bytes"), Bank->BankBytes == Input);
        if (Pass == 0)
        {
            RoundTrip->SceneEnvironment.Actors[0].Conditions.Reset();
            TArray<uint8> Removed;
            FMemoryWriter RemovedWriter(Removed);
            if (!TestTrue(TEXT("Export removal of serialized conditions"),
                Exporter->ExportBinary(RoundTrip, TEXT("bnk"), RemovedWriter, GWarn))) return false;
            const uint8* RemovedCursor = Removed.GetData();
            UKyaLevelBank* RemovedBank = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(
                UKyaLevelBank::StaticClass(), GetTransientPackage(), NAME_None, RF_Transient,
                nullptr, TEXT("bnk"), RemovedCursor, Removed.GetData() + Removed.Num(), GWarn));
            if (!TestNotNull(TEXT("Reimport removed conditions"), RemovedBank)) return false;
            TestTrue(TEXT("All conditions removed"), RemovedBank->SceneEnvironment.Actors[0].Conditions.IsEmpty());
            TestEqual(TEXT("Class data survives removal"),
                RemovedBank->SceneEnvironment.Actors[0].PlatformWeight, Actor.PlatformWeight);
        }
        Actor.Conditions.Reset();
    }
    Actor.Conditions = OriginalConditions;

    FKyaPlatformNoFrictionZone ExtraZone;
    ExtraZone.ZoneRef = 5;
    Actor.PlatformNoFrictionZones.Add(ExtraZone);
    TArray<uint8> Rejected;
    FMemoryWriter RejectedWriter(Rejected);
    TestFalse(TEXT("Exporter rejects a changed no-friction zone count"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), RejectedWriter, GWarn));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKyaHedgehogFixtureRoundTripTest, "Kya.Bank.HedgehogFixtureRoundTrip",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKyaHedgehogFixtureRoundTripTest::RunTest(const FString& Parameters)
{
    // Source-derived fixture: a 104-byte Hedgehog payload (sub-object mode 1),
    // a GuardArea behaviour carrying a path index, and an empty WatchDog
    // behaviour. This is not a game bank; LEVEL_1 has no class 0x22 record.
    FKyaSmallBankBuilder Builder;
    const TArray<uint8> Input = Builder.Build(0x22);
    UKyaLevelBankFactory* Factory = NewObject<UKyaLevelBankFactory>();
    const uint8* Cursor = Input.GetData();
    UKyaLevelBank* Bank = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
        GetTransientPackage(), NAME_None, RF_Transient, nullptr, TEXT("bnk"), Cursor,
        Input.GetData() + Input.Num(), GWarn));
    if (!TestNotNull(TEXT("Import Hedgehog fixture"), Bank)) return false;
    if (!TestEqual(TEXT("Hedgehog fixture actor count"), Bank->SceneEnvironment.Actors.Num(), 1)) return false;
    FKyaActorPlacement& Actor = Bank->SceneEnvironment.Actors[0];
    TestEqual(TEXT("Hedgehog class id"), Actor.ClassId, int64(0x22));
    TestEqual(TEXT("Hedgehog actor name"), Actor.Name, FString(TEXT("HEDGEHOG")));
    TestTrue(TEXT("Hedgehog class payload parses"), Actor.bHasHedgehogData);
    TestEqual(TEXT("Hedgehog weightB"), Actor.HedgehogWeightB, 1.5f);
    TestEqual(TEXT("Hedgehog life maximum"), Actor.HedgehogLifeMax, int64(100));
    TestEqual(TEXT("Hedgehog walk speed"), Actor.HedgehogWalkSpeed, 3.0f);
    TestEqual(TEXT("Hedgehog run speed"), Actor.HedgehogRunSpeed, 6.0f);
    TestEqual(TEXT("Hedgehog field 374"), Actor.HedgehogField374, 7);
    TestEqual(TEXT("Hedgehog field 368"), Actor.HedgehogField368, 9.0f);
    TestEqual(TEXT("Hedgehog add-on money type"), Actor.HedgehogAddOn.MoneyType, 2);
    TestEqual(TEXT("Hedgehog add-on sub-object mode"), Actor.HedgehogAddOn.SubObjMode, 1);
    TestEqual(TEXT("Hedgehog add-on sub-object float 0"), Actor.HedgehogAddOn.SubObjField0, 0.25f);
    TestEqual(TEXT("Hedgehog add-on sub-object float 14"), Actor.HedgehogAddOn.SubObjField14, 1.5f);
    TestEqual(TEXT("Hedgehog field 380"), Actor.HedgehogField380, 13);
    TestEqual(TEXT("Hedgehog fixture leaves no raw class data"), Actor.ClassData.Num(), 0);
    if (!TestEqual(TEXT("Hedgehog behaviour count"), Actor.Behaviours.Num(), 2)) return false;
    TestEqual(TEXT("Hedgehog guard-area id"), Actor.Behaviours[0].Id, 3);
    TestTrue(TEXT("Hedgehog guard-area is typed"), Actor.Behaviours[0].bHasTypedHedgehogBehaviour);
    TestEqual(TEXT("Hedgehog guard-area path index"), Actor.Behaviours[0].HedgehogPathFollowIndex, 12);
    TestEqual(TEXT("Hedgehog watchdog id"), Actor.Behaviours[1].Id, 4);
    TestTrue(TEXT("Hedgehog watchdog is typed"), Actor.Behaviours[1].bHasTypedHedgehogBehaviour);
    TestEqual(TEXT("Hedgehog watchdog payload is empty"), Actor.Behaviours[1].Data.Num(), 0);
    TestEqual(TEXT("Hedgehog watchdog has no path index"), Actor.Behaviours[1].HedgehogPathFollowIndex, -1);
    TestEqual(TEXT("Hedgehog typed field is exportable"),
        GetKyaActorFieldCapability(GET_MEMBER_NAME_CHECKED(FKyaActorPlacement, HedgehogWalkSpeed)).Kind,
        EKyaFieldCapability::EditableExportable);
    TestEqual(TEXT("Hedgehog behaviour field is exportable"),
        GetKyaNestedFieldCapability(TEXT("Behaviours.HedgehogPathFollowIndex")).Kind,
        EKyaFieldCapability::EditableExportable);

    UKyaLevelBankExporter* Exporter = NewObject<UKyaLevelBankExporter>();
    TArray<uint8> Plain;
    FMemoryWriter PlainWriter(Plain);
    if (!TestTrue(TEXT("Export unedited Hedgehog fixture"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), PlainWriter, GWarn))) return false;
    TestEqual(TEXT("Unedited Hedgehog export keeps the file size"), Plain.Num(), Input.Num());
    TestTrue(TEXT("Unedited Hedgehog export preserves every byte"), Plain == Input);

    Actor.HedgehogWalkSpeed = 7.5f;
    Actor.HedgehogField380 = 222;
    Actor.HedgehogAddOn.MoneyCount = 9;
    Actor.Behaviours[0].HedgehogPathFollowIndex = 34;
    TArray<uint8> Edited;
    FMemoryWriter EditedWriter(Edited);
    if (!TestTrue(TEXT("Export edited Hedgehog fixture"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), EditedWriter, GWarn))) return false;
    TestEqual(TEXT("Edited Hedgehog export keeps the file size"), Edited.Num(), Input.Num());
    const uint8* EditedCursor = Edited.GetData();
    UKyaLevelBank* Reimported = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
        GetTransientPackage(), NAME_None, RF_Transient, nullptr, TEXT("bnk"), EditedCursor,
        Edited.GetData() + Edited.Num(), GWarn));
    if (!TestNotNull(TEXT("Reimport edited Hedgehog fixture"), Reimported)) return false;
    const FKyaActorPlacement& EditedActor = Reimported->SceneEnvironment.Actors[0];
    TestEqual(TEXT("Edited Hedgehog walk speed round trips"), EditedActor.HedgehogWalkSpeed, 7.5f);
    TestEqual(TEXT("Edited Hedgehog field 380 round trips"), EditedActor.HedgehogField380, 222);
    TestEqual(TEXT("Edited Hedgehog add-on money count round trips"), EditedActor.HedgehogAddOn.MoneyCount, 9);
    TestEqual(TEXT("Edited Hedgehog path index round trips"),
        EditedActor.Behaviours[0].HedgehogPathFollowIndex, 34);
    TestTrue(TEXT("Reimported Hedgehog watchdog payload stays empty"), EditedActor.Behaviours[1].Data.IsEmpty());

    // The sub-object mode controls the payload size, so a mode edit must be rejected
    // instead of silently changing the class-data layout.
    Actor.HedgehogAddOn.SubObjMode = 0;
    TArray<uint8> Rejected;
    FMemoryWriter RejectedWriter(Rejected);
    TestFalse(TEXT("Exporter rejects a Hedgehog sub-object mode change"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), RejectedWriter, GWarn));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKyaProjectileFixtureRoundTripTest, "Kya.Bank.ProjectileFixtureRoundTrip",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKyaProjectileFixtureRoundTripTest::RunTest(const FString& Parameters)
{
    // Source-derived fixture: 132-byte class payload plus every projectile-owned
    // behaviour shape (3, 4, 5, 9, 10 with the version-gated float, 11, 12, 13).
    FKyaSmallBankBuilder Builder;
    const TArray<uint8> Input = Builder.Build(0x1c);
    UKyaLevelBankFactory* Factory = NewObject<UKyaLevelBankFactory>();
    const uint8* Cursor = Input.GetData();
    UKyaLevelBank* Bank = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
        GetTransientPackage(), NAME_None, RF_Transient, nullptr, TEXT("bnk"), Cursor,
        Input.GetData() + Input.Num(), GWarn));
    if (!TestNotNull(TEXT("Import Projectile fixture"), Bank)) return false;
    if (!TestEqual(TEXT("Projectile fixture actor count"), Bank->SceneEnvironment.Actors.Num(), 1)) return false;
    FKyaActorPlacement& Actor = Bank->SceneEnvironment.Actors[0];
    TestEqual(TEXT("Projectile class id"), Actor.ClassId, int64(0x1c));
    TestEqual(TEXT("Projectile actor name"), Actor.Name, FString(TEXT("PROJECTILE")));
    TestTrue(TEXT("Projectile class payload parses"), Actor.bHasProjectileData);
    TestEqual(TEXT("Projectile weightB"), Actor.ProjectileWeightB, 1.25f);
    TestEqual(TEXT("Projectile life maximum is exact"), Actor.ProjectileLifeMax, int64(16777217));
    TestEqual(TEXT("Projectile sub-object damage"), Actor.ProjectileSubObj.Damage, 5.0f);
    TestEqual(TEXT("Projectile pattern material"), Actor.ProjectilePattern.MaterialId, 7);
    TestEqual(TEXT("Projectile pattern field 28"), Actor.ProjectilePattern.Field28, 10);
    TestFalse(TEXT("Projectile fixture omits pattern field 24"), Actor.ProjectilePattern.bHasField24);
    TestEqual(TEXT("Projectile fixture leaves no raw class data"), Actor.ClassData.Num(), 0);
    if (!TestEqual(TEXT("Projectile behaviour count"), Actor.Behaviours.Num(), 8)) return false;

    const FKyaActorBehaviour& Stand = Actor.Behaviours[0];
    const FKyaActorBehaviour& Straight = Actor.Behaviours[1];
    const FKyaActorBehaviour& Portable = Actor.Behaviours[2];
    const FKyaActorBehaviour& Inventory = Actor.Behaviours[3];
    const FKyaActorBehaviour& LavaBall = Actor.Behaviours[4];
    const FKyaActorBehaviour& Excuse = Actor.Behaviours[5];
    const FKyaActorBehaviour& Inactive = Actor.Behaviours[6];
    const FKyaActorBehaviour& NewBehaviour = Actor.Behaviours[7];
    TestEqual(TEXT("Projectile stand id"), Stand.Id, 3);
    TestTrue(TEXT("Projectile stand is typed"), Stand.bHasTypedProjectileBehaviour);
    TestEqual(TEXT("Projectile stand material"), Stand.ProjectileBehaviour.MaterialId, 7);
    TestEqual(TEXT("Projectile straight id"), Straight.Id, 4);
    TestTrue(TEXT("Projectile straight is typed"), Straight.bHasTypedProjectileBehaviour);
    TestEqual(TEXT("Projectile straight speed"), Straight.ProjectileBehaviour.Speed, 3.5f);
    TestEqual(TEXT("Projectile straight acceleration"), Straight.ProjectileBehaviour.Acceleration, 4.5f);
    TestEqual(TEXT("Projectile straight field 18"), Straight.ProjectileBehaviour.Field18, 5.5f);
    TestEqual(TEXT("Projectile portable id"), Portable.Id, 5);
    TestTrue(TEXT("Projectile portable is typed"), Portable.bHasTypedProjectileBehaviour);
    TestEqual(TEXT("Projectile portable field 10"), Portable.ProjectileBehaviour.Field10, 6.5f);
    TestEqual(TEXT("Projectile inventory id"), Inventory.Id, 9);
    TestTrue(TEXT("Projectile inventory is typed"), Inventory.bHasTypedInventoryBehaviour);
    TestEqual(TEXT("Projectile inventory purchase id"), Inventory.InventoryInfo.PurchaseId, 21);
    TestEqual(TEXT("Projectile inventory money cost"), Inventory.InventoryInfo.MoneyCost, 23);
    TestEqual(TEXT("Projectile inventory initial behaviour"), Inventory.InventoryInfo.InitialBehaviourId, 3);
    TestEqual(TEXT("Projectile lava ball id"), LavaBall.Id, 10);
    TestTrue(TEXT("Projectile lava ball is typed"), LavaBall.bHasTypedProjectileBehaviour);
    TestTrue(TEXT("Projectile lava ball has field 18"), LavaBall.ProjectileBehaviour.bHasLavaBallField18);
    TestEqual(TEXT("Projectile lava ball field 18"), LavaBall.ProjectileBehaviour.LavaBallField18, 4.0f);
    TestEqual(TEXT("Projectile lava ball path"), LavaBall.ProjectileBehaviour.LavaBallPathIndex, 5);
    TestEqual(TEXT("Projectile lava ball field 3c"), LavaBall.ProjectileBehaviour.LavaBallField3C, 9);
    TestEqual(TEXT("Projectile excuse id"), Excuse.Id, 11);
    TestTrue(TEXT("Projectile excuse is typed"), Excuse.bHasTypedProjectileBehaviour);
    TestEqual(TEXT("Projectile excuse actor"), Excuse.ProjectileBehaviour.ActorRef, 0);
    TestEqual(TEXT("Projectile inactive id"), Inactive.Id, 12);
    TestTrue(TEXT("Projectile inactive is typed"), Inactive.bHasTypedProjectileBehaviour);
    TestEqual(TEXT("Projectile inactive message"), Inactive.ProjectileBehaviour.InactiveActivateMessageId, 6);
    TestEqual(TEXT("Projectile inactive flags"), Inactive.ProjectileBehaviour.InactiveFlags, 7);
    TestEqual(TEXT("Projectile inactive active behaviour"), Inactive.ProjectileBehaviour.InactiveActiveBehaviourId, 3);
    TestEqual(TEXT("Projectile new id"), NewBehaviour.Id, 13);
    TestTrue(TEXT("Projectile new is typed"), NewBehaviour.bHasTypedProjectileBehaviour);
    TestEqual(TEXT("Projectile new behaviour id"), NewBehaviour.ProjectileBehaviour.BehaviourId, 13);

    UKyaLevelBankExporter* Exporter = NewObject<UKyaLevelBankExporter>();
    TArray<uint8> Plain;
    FMemoryWriter PlainWriter(Plain);
    if (!TestTrue(TEXT("Export unedited Projectile fixture"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), PlainWriter, GWarn))) return false;
    TestEqual(TEXT("Unedited Projectile export keeps the file size"), Plain.Num(), Input.Num());
    TestTrue(TEXT("Unedited Projectile export preserves every byte"), Plain == Input);

    Actor.ProjectileWeightB = 2.5f;
    Actor.ProjectileLifeMax = 4000000000ll;
    Actor.ProjectileSubObj.Damage = 55.0f;
    Actor.ProjectilePattern.Field28 = 77;
    Actor.Behaviours[1].ProjectileBehaviour.Speed = 8.5f;
    Actor.Behaviours[3].InventoryInfo.MoneyCost = 123;
    Actor.Behaviours[4].ProjectileBehaviour.LavaBallPathIndex = 21;
    Actor.Behaviours[6].ProjectileBehaviour.InactiveFlags = 99;
    TArray<uint8> Edited;
    FMemoryWriter EditedWriter(Edited);
    if (!TestTrue(TEXT("Export edited Projectile fixture"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), EditedWriter, GWarn))) return false;
    TestEqual(TEXT("Edited Projectile export keeps the file size"), Edited.Num(), Input.Num());
    const uint8* EditedCursor = Edited.GetData();
    UKyaLevelBank* Reimported = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
        GetTransientPackage(), NAME_None, RF_Transient, nullptr, TEXT("bnk"), EditedCursor,
        Edited.GetData() + Edited.Num(), GWarn));
    if (!TestNotNull(TEXT("Reimport edited Projectile fixture"), Reimported)) return false;
    const FKyaActorPlacement& EditedActor = Reimported->SceneEnvironment.Actors[0];
    TestEqual(TEXT("Edited Projectile weight round trips"), EditedActor.ProjectileWeightB, 2.5f);
    TestEqual(TEXT("Edited Projectile life maximum round trips"), EditedActor.ProjectileLifeMax,
        int64(4000000000ll));
    TestEqual(TEXT("Edited Projectile damage round trips"), EditedActor.ProjectileSubObj.Damage, 55.0f);
    TestEqual(TEXT("Edited Projectile pattern field round trips"), EditedActor.ProjectilePattern.Field28, 77);
    TestEqual(TEXT("Edited straight speed round trips"),
        EditedActor.Behaviours[1].ProjectileBehaviour.Speed, 8.5f);
    TestEqual(TEXT("Edited inventory cost round trips"),
        EditedActor.Behaviours[3].InventoryInfo.MoneyCost, 123);
    TestEqual(TEXT("Edited lava ball path round trips"),
        EditedActor.Behaviours[4].ProjectileBehaviour.LavaBallPathIndex, 21);
    TestEqual(TEXT("Edited inactive flags round trips"),
        EditedActor.Behaviours[6].ProjectileBehaviour.InactiveFlags, 99);

    // Layout-changing edits must be rejected, not silently written.
    Actor.ProjectilePattern.bHasField24 = true;
    TArray<uint8> Rejected;
    FMemoryWriter RejectedWriter(Rejected);
    TestFalse(TEXT("Exporter rejects a Projectile pattern field 24 change"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), RejectedWriter, GWarn));
    Actor.ProjectilePattern.bHasField24 = false;
    Actor.ProjectileLifeMax = -1;
    TArray<uint8> RejectedLife;
    FMemoryWriter RejectedLifeWriter(RejectedLife);
    TestFalse(TEXT("Exporter rejects an out-of-range Projectile life maximum"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), RejectedLifeWriter, GWarn));
    Actor.ProjectileLifeMax = 4000000000ll;

    // The inventory presence word selects the payload shape; changing its
    // zero/non-zero state would corrupt the following fields.
    Actor.Behaviours[3].InventoryInfo.Field0 = 0;
    TArray<uint8> RejectedInventory;
    FMemoryWriter RejectedInventoryWriter(RejectedInventory);
    TestFalse(TEXT("Exporter rejects a Projectile inventory presence change"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), RejectedInventoryWriter, GWarn));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKyaMovingPlatformFixtureRoundTripTest, "Kya.Bank.MovingPlatformFixtureRoundTrip",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKyaMovingPlatformFixtureRoundTripTest::RunTest(const FString& Parameters)
{
    // The existing small-bank fixture covers the class payload; this variant adds
    // the one behaviour shape absent from LEVEL_1 (id 10 TeleportRandom).
    FKyaSmallBankBuilder Builder;
    const TArray<uint8> Input = Builder.Build(0x0d, true);
    UKyaLevelBankFactory* Factory = NewObject<UKyaLevelBankFactory>();
    const uint8* Cursor = Input.GetData();
    UKyaLevelBank* Bank = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
        GetTransientPackage(), NAME_None, RF_Transient, nullptr, TEXT("bnk"), Cursor,
        Input.GetData() + Input.Num(), GWarn));
    if (!TestNotNull(TEXT("Import moving-platform fixture"), Bank)) return false;
    if (!TestEqual(TEXT("Moving-platform fixture actor count"), Bank->SceneEnvironment.Actors.Num(), 1)) return false;
    FKyaActorPlacement& Actor = Bank->SceneEnvironment.Actors[0];
    TestEqual(TEXT("Moving-platform class id"), Actor.ClassId, int64(0x0d));
    TestEqual(TEXT("Moving-platform actor name"), Actor.Name, FString(TEXT("PLATFORM")));
    TestTrue(TEXT("Moving-platform prefix parses"), Actor.bHasPlatformPrefix);
    TestTrue(TEXT("Moving-platform details parse"), Actor.bHasPlatformDetails);
    TestEqual(TEXT("Moving-platform weight"), Actor.PlatformWeight, 1.0f);
    TestEqual(TEXT("Moving-platform actor list"), Actor.PlatformActors.Num(), 0);
    TestEqual(TEXT("Moving-platform zone list"), Actor.PlatformZones.Num(), 0);
    if (!TestEqual(TEXT("Moving-platform no-friction zones"), Actor.PlatformNoFrictionZones.Num(), 1)) return false;
    TestEqual(TEXT("Moving-platform no-friction ref"), Actor.PlatformNoFrictionZones[0].ZoneRef, 42);
    TestEqual(TEXT("Moving-platform no-friction actor 0"), Actor.PlatformNoFrictionZones[0].Actors[0], 7);
    TestEqual(TEXT("Moving-platform no-friction actor 1"), Actor.PlatformNoFrictionZones[0].Actors[1], 8);
    TestEqual(TEXT("Moving-platform camera count"), Actor.PlatformCameras.Num(), 0);
    TestEqual(TEXT("Moving-platform sound list A"), Actor.PlatformSoundsA.Num(), 0);
    TestEqual(TEXT("Moving-platform sound list B"), Actor.PlatformSoundsB.Num(), 0);
    TestEqual(TEXT("Moving-platform flags"), Actor.PlatformProperties.PlatformFlags, int64(14));
    TestEqual(TEXT("Moving-platform tilt value"), Actor.PlatformProperties.TiltValue, 50.0f);
    TestEqual(TEXT("Moving-platform fixture leaves no raw class data"), Actor.ClassData.Num(), 0);
    if (!TestEqual(TEXT("Moving-platform behaviour count"), Actor.Behaviours.Num(), 1)) return false;
    TestEqual(TEXT("Moving-platform teleport id"), Actor.Behaviours[0].Id, 10);
    TestTrue(TEXT("Moving-platform teleport is typed"), Actor.Behaviours[0].bHasTypedPlatformBehaviour);
    TestEqual(TEXT("Moving-platform teleport path"), Actor.Behaviours[0].PlatformBehaviour.PathIndex, 33);

    UKyaLevelBankExporter* Exporter = NewObject<UKyaLevelBankExporter>();
    TArray<uint8> Plain;
    FMemoryWriter PlainWriter(Plain);
    if (!TestTrue(TEXT("Export unedited moving-platform fixture"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), PlainWriter, GWarn))) return false;
    TestEqual(TEXT("Unedited moving-platform export keeps the file size"), Plain.Num(), Input.Num());
    TestTrue(TEXT("Unedited moving-platform export preserves every byte"), Plain == Input);

    Actor.PlatformWeight = 2.5f;
    Actor.PlatformNoFrictionZones[0].ZoneRef = 99;
    Actor.PlatformNoFrictionZones[0].Actors[0] = 1234;
    Actor.PlatformProperties.PlatformFlags = 99;
    Actor.PlatformProperties.TiltValue = 12.5f;
    Actor.Behaviours[0].PlatformBehaviour.PathIndex = 44;
    TArray<uint8> Edited;
    FMemoryWriter EditedWriter(Edited);
    if (!TestTrue(TEXT("Export edited moving-platform fixture"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), EditedWriter, GWarn))) return false;
    TestEqual(TEXT("Edited moving-platform export keeps the file size"), Edited.Num(), Input.Num());
    const uint8* EditedCursor = Edited.GetData();
    UKyaLevelBank* Reimported = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
        GetTransientPackage(), NAME_None, RF_Transient, nullptr, TEXT("bnk"), EditedCursor,
        Edited.GetData() + Edited.Num(), GWarn));
    if (!TestNotNull(TEXT("Reimport edited moving-platform fixture"), Reimported)) return false;
    const FKyaActorPlacement& EditedActor = Reimported->SceneEnvironment.Actors[0];
    TestEqual(TEXT("Edited moving-platform weight round trips"), EditedActor.PlatformWeight, 2.5f);
    TestEqual(TEXT("Edited moving-platform no-friction ref round trips"),
        EditedActor.PlatformNoFrictionZones[0].ZoneRef, 99);
    TestEqual(TEXT("Edited moving-platform no-friction actor round trips"),
        EditedActor.PlatformNoFrictionZones[0].Actors[0], 1234);
    TestEqual(TEXT("Edited moving-platform flags round trips"), EditedActor.PlatformProperties.PlatformFlags,
        int64(99));
    TestEqual(TEXT("Edited moving-platform tilt round trips"), EditedActor.PlatformProperties.TiltValue, 12.5f);
    TestEqual(TEXT("Edited moving-platform teleport path round trips"),
        EditedActor.Behaviours[0].PlatformBehaviour.PathIndex, 44);

    Actor.PlatformNoFrictionZones.AddDefaulted();
    TArray<uint8> Rejected;
    FMemoryWriter RejectedWriter(Rejected);
    TestFalse(TEXT("Exporter rejects a changed no-friction zone count"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), RejectedWriter, GWarn));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKyaSwitchFixtureRoundTripTest, "Kya.Bank.SwitchFixtureRoundTrip",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKyaSwitchFixtureRoundTripTest::RunTest(const FString& Parameters)
{
    // Source-derived fixture: one target-switch class payload plus the three
    // behaviour shapes absent from LEVEL_1 (4 WolfenCounter, 7 Sequence, 8 Timed).
    FKyaSmallBankBuilder Builder;
    const TArray<uint8> Input = Builder.Build(0x11, false, true);
    UKyaLevelBankFactory* Factory = NewObject<UKyaLevelBankFactory>();
    const uint8* Cursor = Input.GetData();
    UKyaLevelBank* Bank = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
        GetTransientPackage(), NAME_None, RF_Transient, nullptr, TEXT("bnk"), Cursor,
        Input.GetData() + Input.Num(), GWarn));
    if (!TestNotNull(TEXT("Import switch fixture"), Bank)) return false;
    if (!TestEqual(TEXT("Switch fixture actor count"), Bank->SceneEnvironment.Actors.Num(), 1)) return false;
    FKyaActorPlacement& Actor = Bank->SceneEnvironment.Actors[0];
    TestEqual(TEXT("Switch class id"), Actor.ClassId, int64(0x11));
    TestEqual(TEXT("Switch actor name"), Actor.Name, FString(TEXT("SWITCH")));
    TestTrue(TEXT("Switch class payload parses"), Actor.bHasSwitchData);
    if (!TestEqual(TEXT("Switch target count"), Actor.SwitchTarget.Targets.Num(), 1)) return false;
    TestEqual(TEXT("Switch target ref"), Actor.SwitchTarget.Targets[0].TargetRef, 31);
    TestEqual(TEXT("Switch camera index"), Actor.SwitchTarget.Camera.CameraIndex, 41);
    TestEqual(TEXT("Switch fixture leaves no raw class data"), Actor.ClassData.Num(), 0);
    if (!TestEqual(TEXT("Switch behaviour count"), Actor.Behaviours.Num(), 3)) return false;
    const FKyaActorBehaviour& Wolfen = Actor.Behaviours[0];
    const FKyaActorBehaviour& Sequence = Actor.Behaviours[1];
    const FKyaActorBehaviour& Timed = Actor.Behaviours[2];
    TestEqual(TEXT("Switch Wolfen id"), Wolfen.Id, 4);
    TestTrue(TEXT("Switch Wolfen is typed"), Wolfen.bHasTypedSwitchBehaviour);
    TestEqual(TEXT("Switch Wolfen required"), Wolfen.SwitchBehaviour.WolfenRequired, 2);
    TestEqual(TEXT("Switch Wolfen radius"), Wolfen.SwitchBehaviour.WolfenTriggerRadius, 3.5f);
    TestEqual(TEXT("Switch Wolfen material"), Wolfen.SwitchBehaviour.WolfenMaterialId, 6);
    TestEqual(TEXT("Switch sequence id"), Sequence.Id, 7);
    TestTrue(TEXT("Switch sequence is typed"), Sequence.bHasTypedSwitchBehaviour);
    TestEqual(TEXT("Switch sequence values"), Sequence.SwitchBehaviour.SequenceValues.Num(), 3);
    TestEqual(TEXT("Switch sequence value 0"), Sequence.SwitchBehaviour.SequenceValues[0], 11);
    TestEqual(TEXT("Switch sequence value 2"), Sequence.SwitchBehaviour.SequenceValues[2], 13);
    TestEqual(TEXT("Switch timed id"), Timed.Id, 8);
    TestTrue(TEXT("Switch timed is typed"), Timed.bHasTypedSwitchBehaviour);
    TestEqual(TEXT("Switch timed base delay"), Timed.SwitchBehaviour.TimedBaseDelay, 0.5f);
    if (!TestEqual(TEXT("Switch timed entry count"), Timed.SwitchBehaviour.TimedSwitches.Num(), 2)) return false;
    TestEqual(TEXT("Switch timed entry 0 targets"), Timed.SwitchBehaviour.TimedSwitches[0].Targets.Num(), 0);
    TestEqual(TEXT("Switch timed entry 1 target"), Timed.SwitchBehaviour.TimedSwitches[1].Targets[0].TargetRef, 41);
    TestEqual(TEXT("Switch timed delay 0"), Timed.SwitchBehaviour.TimedDelays[0], 1.5f);
    TestEqual(TEXT("Switch timed delay 1"), Timed.SwitchBehaviour.TimedDelays[1], 2.5f);
    TestEqual(TEXT("Switch timed playback"), Timed.SwitchBehaviour.TimedPlaybackMode, 3);
    TestEqual(TEXT("Switch timed flags"), Timed.SwitchBehaviour.TimedFlags, 4);

    UKyaLevelBankExporter* Exporter = NewObject<UKyaLevelBankExporter>();
    TArray<uint8> Plain;
    FMemoryWriter PlainWriter(Plain);
    if (!TestTrue(TEXT("Export unedited switch fixture"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), PlainWriter, GWarn))) return false;
    TestEqual(TEXT("Unedited switch export keeps the file size"), Plain.Num(), Input.Num());
    TestTrue(TEXT("Unedited switch export preserves every byte"), Plain == Input);

    Actor.SwitchTarget.Targets[0].TargetRef = 99;
    Actor.SwitchTarget.Camera.CameraIndex = 77;
    Actor.Behaviours[0].SwitchBehaviour.WolfenRequired = 9;
    Actor.Behaviours[1].SwitchBehaviour.SequenceValues[0] = 21;
    Actor.Behaviours[2].SwitchBehaviour.TimedDelays[1] = 9.5f;
    Actor.Behaviours[2].SwitchBehaviour.TimedFlags = 5;
    TArray<uint8> Edited;
    FMemoryWriter EditedWriter(Edited);
    if (!TestTrue(TEXT("Export edited switch fixture"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), EditedWriter, GWarn))) return false;
    TestEqual(TEXT("Edited switch export keeps the file size"), Edited.Num(), Input.Num());
    const uint8* EditedCursor = Edited.GetData();
    UKyaLevelBank* Reimported = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
        GetTransientPackage(), NAME_None, RF_Transient, nullptr, TEXT("bnk"), EditedCursor,
        Edited.GetData() + Edited.Num(), GWarn));
    if (!TestNotNull(TEXT("Reimport edited switch fixture"), Reimported)) return false;
    const FKyaActorPlacement& EditedActor = Reimported->SceneEnvironment.Actors[0];
    TestEqual(TEXT("Edited switch target round trips"), EditedActor.SwitchTarget.Targets[0].TargetRef, 99);
    TestEqual(TEXT("Edited switch camera round trips"), EditedActor.SwitchTarget.Camera.CameraIndex, 77);
    TestEqual(TEXT("Edited switch Wolfen round trips"),
        EditedActor.Behaviours[0].SwitchBehaviour.WolfenRequired, 9);
    TestEqual(TEXT("Edited switch sequence round trips"),
        EditedActor.Behaviours[1].SwitchBehaviour.SequenceValues[0], 21);
    TestEqual(TEXT("Edited switch timed delay round trips"),
        EditedActor.Behaviours[2].SwitchBehaviour.TimedDelays[1], 9.5f);
    TestEqual(TEXT("Edited switch timed flags round trips"),
        EditedActor.Behaviours[2].SwitchBehaviour.TimedFlags, 5);

    Actor.Behaviours[1].SwitchBehaviour.SequenceValues.Add(99);
    TArray<uint8> Rejected;
    FMemoryWriter RejectedWriter(Rejected);
    TestFalse(TEXT("Exporter rejects a changed switch sequence count"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), RejectedWriter, GWarn));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKyaWindFixtureRoundTripTest, "Kya.Bank.WindFixtureRoundTrip",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKyaWindFixtureRoundTripTest::RunTest(const FString& Parameters)
{
    // Modern 2.2+ Wind payload with three switch records and the empty id 2
    // behaviour. Real LEVEL_1 Winds use the same class layout.
    FKyaSmallBankBuilder Builder;
    const TArray<uint8> Input = Builder.Build(0x12, false, false, true);
    UKyaLevelBankFactory* Factory = NewObject<UKyaLevelBankFactory>();
    const uint8* Cursor = Input.GetData();
    UKyaLevelBank* Bank = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
        GetTransientPackage(), NAME_None, RF_Transient, nullptr, TEXT("bnk"), Cursor,
        Input.GetData() + Input.Num(), GWarn));
    if (!TestNotNull(TEXT("Import wind fixture"), Bank)) return false;
    if (!TestEqual(TEXT("Wind fixture actor count"), Bank->SceneEnvironment.Actors.Num(), 1)) return false;
    FKyaActorPlacement& Actor = Bank->SceneEnvironment.Actors[0];
    TestEqual(TEXT("Wind class id"), Actor.ClassId, int64(0x12));
    TestEqual(TEXT("Wind actor name"), Actor.Name, FString(TEXT("WIND")));
    TestTrue(TEXT("Wind class payload parses"), Actor.bHasWindData);
    TestEqual(TEXT("Wind field 160"), Actor.WindField160, 0x1234);
    TestEqual(TEXT("Wind max wind"), Actor.WindMaxWind, 1.0f);
    TestEqual(TEXT("Wind field 190"), Actor.WindField190, 0x5678);
    TestEqual(TEXT("Wind sound"), Actor.WindSoundRef, 9);
    TestEqual(TEXT("Wind activation zone"), Actor.WindActivationZoneIndex, 3);
    TestEqual(TEXT("Wind actor list"), Actor.WindActorRefs.Num(), 0);
    TestEqual(TEXT("Wind material"), Actor.WindMaterialId, 5);
    TestEqual(TEXT("Wind zone 1c4"), Actor.WindZoneRef1C4, 8);
    TestEqual(TEXT("Wind waypoint 1cc"), Actor.WindWaypointRef1CC, 7);
    TestEqual(TEXT("Wind sector hash"), Actor.WindSectorHierarchyHash, int64(0x5566778811223344ll));
    if (!TestEqual(TEXT("Wind switch 0 targets"), Actor.WindSwitch1D0.Targets.Num(), 1)) return false;
    TestEqual(TEXT("Wind switch 0 target ref"), Actor.WindSwitch1D0.Targets[0].TargetRef, 61);
    TestEqual(TEXT("Wind switch 0 camera"), Actor.WindSwitch1D0.Camera.CameraIndex, 71);
    TestEqual(TEXT("Wind switch 1 targets"), Actor.WindSwitch1D8.Targets.Num(), 0);
    TestEqual(TEXT("Wind switch 2 targets"), Actor.WindSwitch1E0.Targets.Num(), 0);
    TestEqual(TEXT("Wind field 194"), Actor.WindField194, 11.0f);
    TestEqual(TEXT("Wind field 1a4"), Actor.WindField1A4, 15.0f);
    TestEqual(TEXT("Wind waypoint ref"), Actor.WindWaypointRef, 17);
    TestEqual(TEXT("Wind field 1ac"), Actor.WindField1AC, 18.0f);
    TestEqual(TEXT("Wind field 1b0"), Actor.WindField1B0, 19.0f);
    TestEqual(TEXT("Wind fixture leaves no raw class data"), Actor.ClassData.Num(), 0);
    if (!TestEqual(TEXT("Wind behaviour count"), Actor.Behaviours.Num(), 1)) return false;
    TestEqual(TEXT("Wind behaviour id"), Actor.Behaviours[0].Id, 2);
    TestEqual(TEXT("Wind behaviour is empty"), Actor.Behaviours[0].Data.Num(), 0);

    UKyaLevelBankExporter* Exporter = NewObject<UKyaLevelBankExporter>();
    TArray<uint8> Plain;
    FMemoryWriter PlainWriter(Plain);
    if (!TestTrue(TEXT("Export unedited wind fixture"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), PlainWriter, GWarn))) return false;
    TestEqual(TEXT("Unedited wind export keeps the file size"), Plain.Num(), Input.Num());
    TestTrue(TEXT("Unedited wind export preserves every byte"), Plain == Input);

    Actor.WindField160 = 99;
    Actor.WindMaxWind = 7.5f;
    Actor.WindMaterialId = 12;
    Actor.WindSwitch1D0.Targets[0].TargetRef = 123;
    Actor.WindField1A0 = 42.0f;
    Actor.WindField1AC = 43.0f;
    TArray<uint8> Edited;
    FMemoryWriter EditedWriter(Edited);
    if (!TestTrue(TEXT("Export edited wind fixture"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), EditedWriter, GWarn))) return false;
    TestEqual(TEXT("Edited wind export keeps the file size"), Edited.Num(), Input.Num());
    const uint8* EditedCursor = Edited.GetData();
    UKyaLevelBank* Reimported = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
        GetTransientPackage(), NAME_None, RF_Transient, nullptr, TEXT("bnk"), EditedCursor,
        Edited.GetData() + Edited.Num(), GWarn));
    if (!TestNotNull(TEXT("Reimport edited wind fixture"), Reimported)) return false;
    const FKyaActorPlacement& EditedActor = Reimported->SceneEnvironment.Actors[0];
    TestEqual(TEXT("Edited wind field 160 round trips"), EditedActor.WindField160, 99);
    TestEqual(TEXT("Edited wind max wind round trips"), EditedActor.WindMaxWind, 7.5f);
    TestEqual(TEXT("Edited wind material round trips"), EditedActor.WindMaterialId, 12);
    TestEqual(TEXT("Edited wind switch target round trips"),
        EditedActor.WindSwitch1D0.Targets[0].TargetRef, 123);
    TestEqual(TEXT("Edited wind field 1a0 round trips"), EditedActor.WindField1A0, 42.0f);
    TestEqual(TEXT("Edited wind field 1ac round trips"), EditedActor.WindField1AC, 43.0f);

    Actor.WindActorRefs.Add(9);
    TArray<uint8> Rejected;
    FMemoryWriter RejectedWriter(Rejected);
    TestFalse(TEXT("Exporter rejects a changed wind actor count"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), RejectedWriter, GWarn));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKyaTrapFixtureRoundTripTest, "Kya.Bank.TrapFixtureRoundTrip",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKyaTrapFixtureRoundTripTest::RunTest(const FString& Parameters)
{
    FKyaSmallBankBuilder Builder;
    const TArray<uint8> Input = Builder.Build(0x2c, false, false, false, true);
    UKyaLevelBankFactory* Factory = NewObject<UKyaLevelBankFactory>();
    const uint8* Cursor = Input.GetData();
    UKyaLevelBank* Bank = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
        GetTransientPackage(), NAME_None, RF_Transient, nullptr, TEXT("bnk"), Cursor,
        Input.GetData() + Input.Num(), GWarn));
    if (!TestNotNull(TEXT("Import trap fixture"), Bank)) return false;
    if (!TestEqual(TEXT("Trap fixture actor count"), Bank->SceneEnvironment.Actors.Num(), 1)) return false;
    FKyaActorPlacement& Actor = Bank->SceneEnvironment.Actors[0];
    TestEqual(TEXT("Trap class id"), Actor.ClassId, int64(0x2c));
    TestEqual(TEXT("Trap actor name"), Actor.Name, FString(TEXT("TRAP")));
    TestTrue(TEXT("Trap class payload parses"), Actor.bHasTrapData);
    TestEqual(TEXT("Trap weight"), Actor.TrapWeightB, 1.5f);
    TestEqual(TEXT("Trap actor refs"), Actor.TrapActorRefs.Num(), 0);
    TestEqual(TEXT("Trap field 14"), Actor.TrapField14, 11);
    TestEqual(TEXT("Trap field 20"), Actor.TrapField20, 14);
    TestTrue(TEXT("Trap has field 24"), Actor.bHasTrapField24);
    TestEqual(TEXT("Trap field 24"), Actor.TrapField24, 15);
    TestEqual(TEXT("Trap actor ref"), Actor.TrapActorRef, 16);
    TestEqual(TEXT("Trap speed"), Actor.TrapSpeed, 2.5f);
    TestEqual(TEXT("Trap acceleration"), Actor.TrapAcceleration, 3.5f);
    TestEqual(TEXT("Trap path index"), Actor.TrapPathFollowIndex, 17);
    TestEqual(TEXT("Trap field 4c"), Actor.TrapField4C, 4.5f);
    TestEqual(TEXT("Trap required escapes"), Actor.TrapRequiredEscapeAttempts, 5);
    if (!TestEqual(TEXT("Trap stream count"), Actor.TrapStreamEntries.Num(), 1)) return false;
    TestEqual(TEXT("Trap stream target"), Actor.TrapStreamEntries[0].Target.TargetRef, 21);
    TestEqual(TEXT("Trap stream camera"), Actor.TrapStreamEntries[0].Camera.CameraIndex, 31);
    TestEqual(TEXT("Trap fixture leaves no raw class data"), Actor.ClassData.Num(), 0);
    if (!TestEqual(TEXT("Trap behaviour count"), Actor.Behaviours.Num(), 2)) return false;
    TestEqual(TEXT("Trap stand id"), Actor.Behaviours[0].Id, 2);
    TestTrue(TEXT("Trap stand is typed"), Actor.Behaviours[0].bHasTypedTrapBehaviour);
    TestEqual(TEXT("Trap stand field 0"), Actor.Behaviours[0].TrapBehaviourField0, 1);
    TestEqual(TEXT("Trap stand field 8"), Actor.Behaviours[0].TrapBehaviourField8, 3);
    TestEqual(TEXT("Trap inactive id"), Actor.Behaviours[1].Id, 3);
    TestTrue(TEXT("Trap inactive is typed"), Actor.Behaviours[1].bHasTypedTrapBehaviour);
    TestEqual(TEXT("Trap inactive field 0"), Actor.Behaviours[1].TrapBehaviourField0, 4);
    TestEqual(TEXT("Trap inactive field 8"), Actor.Behaviours[1].TrapBehaviourField8, 6);

    UKyaLevelBankExporter* Exporter = NewObject<UKyaLevelBankExporter>();
    TArray<uint8> Plain;
    FMemoryWriter PlainWriter(Plain);
    if (!TestTrue(TEXT("Export unedited trap fixture"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), PlainWriter, GWarn))) return false;
    TestEqual(TEXT("Unedited trap export keeps the file size"), Plain.Num(), Input.Num());
    TestTrue(TEXT("Unedited trap export preserves every byte"), Plain == Input);

    Actor.TrapWeightB = 7.5f;
    Actor.TrapField14 = 91;
    Actor.TrapActorRef = 92;
    Actor.TrapSpeed = 8.5f;
    Actor.TrapStreamEntries[0].Target.TargetRef = 93;
    Actor.Behaviours[0].TrapBehaviourField4 = 94;
    Actor.Behaviours[1].TrapBehaviourField8 = 95;
    TArray<uint8> Edited;
    FMemoryWriter EditedWriter(Edited);
    if (!TestTrue(TEXT("Export edited trap fixture"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), EditedWriter, GWarn))) return false;
    TestEqual(TEXT("Edited trap export keeps the file size"), Edited.Num(), Input.Num());
    const uint8* EditedCursor = Edited.GetData();
    UKyaLevelBank* Reimported = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
        GetTransientPackage(), NAME_None, RF_Transient, nullptr, TEXT("bnk"), EditedCursor,
        Edited.GetData() + Edited.Num(), GWarn));
    if (!TestNotNull(TEXT("Reimport edited trap fixture"), Reimported)) return false;
    const FKyaActorPlacement& EditedActor = Reimported->SceneEnvironment.Actors[0];
    TestEqual(TEXT("Edited trap weight round trips"), EditedActor.TrapWeightB, 7.5f);
    TestEqual(TEXT("Edited trap field 14 round trips"), EditedActor.TrapField14, 91);
    TestEqual(TEXT("Edited trap actor ref round trips"), EditedActor.TrapActorRef, 92);
    TestEqual(TEXT("Edited trap speed round trips"), EditedActor.TrapSpeed, 8.5f);
    TestEqual(TEXT("Edited trap stream target round trips"),
        EditedActor.TrapStreamEntries[0].Target.TargetRef, 93);
    TestEqual(TEXT("Edited trap stand field round trips"),
        EditedActor.Behaviours[0].TrapBehaviourField4, 94);
    TestEqual(TEXT("Edited trap inactive field round trips"),
        EditedActor.Behaviours[1].TrapBehaviourField8, 95);

    Actor.TrapStreamEntries.AddDefaulted();
    TArray<uint8> Rejected;
    FMemoryWriter RejectedWriter(Rejected);
    TestFalse(TEXT("Exporter rejects a changed trap stream count"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), RejectedWriter, GWarn));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKyaWeaponFixtureRoundTripTest, "Kya.Bank.WeaponFixtureRoundTrip",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKyaWeaponFixtureRoundTripTest::RunTest(const FString& Parameters)
{
    FKyaSmallBankBuilder Builder;
    const TArray<uint8> Input = Builder.Build(0x20, false, false, false, false, true);
    UKyaLevelBankFactory* Factory = NewObject<UKyaLevelBankFactory>();
    const uint8* Cursor = Input.GetData();
    UKyaLevelBank* Bank = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
        GetTransientPackage(), NAME_None, RF_Transient, nullptr, TEXT("bnk"), Cursor,
        Input.GetData() + Input.Num(), GWarn));
    if (!TestNotNull(TEXT("Import weapon fixture"), Bank)) return false;
    if (!TestEqual(TEXT("Weapon fixture actor count"), Bank->SceneEnvironment.Actors.Num(), 1)) return false;
    FKyaActorPlacement& Actor = Bank->SceneEnvironment.Actors[0];
    TestEqual(TEXT("Weapon class id"), Actor.ClassId, int64(0x20));
    TestEqual(TEXT("Weapon actor name"), Actor.Name, FString(TEXT("WEAPON")));
    TestTrue(TEXT("Weapon class payload parses"), Actor.bHasWeaponData);
    TestEqual(TEXT("Weapon weight"), Actor.WeaponWeightB, 1.5f);
    TestEqual(TEXT("Weapon field 1d8"), Actor.WeaponField1D8, 2.5f);
    TestEqual(TEXT("Weapon field 1dc"), Actor.WeaponField1DC, 3.5f);
    TestEqual(TEXT("Weapon fixture leaves no raw class data"), Actor.ClassData.Num(), 0);
    if (!TestEqual(TEXT("Weapon behaviour count"), Actor.Behaviours.Num(), 3)) return false;
    const FKyaActorBehaviour& Pistol = Actor.Behaviours[0];
    const FKyaActorBehaviour& Sniper = Actor.Behaviours[1];
    const FKyaActorBehaviour& Rpg = Actor.Behaviours[2];
    TestEqual(TEXT("Weapon pistol id"), Pistol.Id, 3);
    TestTrue(TEXT("Weapon pistol is typed"), Pistol.bHasTypedWeaponBehaviour);
    TestEqual(TEXT("Weapon pistol burst count"), Pistol.WeaponBurst.Num(), 2);
    TestEqual(TEXT("Weapon pistol field 40"), Pistol.WeaponField40, 21);
    TestEqual(TEXT("Weapon pistol sound"), Pistol.WeaponSoundIndex, 23);
    TestEqual(TEXT("Weapon pistol material"), Pistol.WeaponPistolField2E0, 24);
    TestEqual(TEXT("Weapon sniper id"), Sniper.Id, 4);
    TestTrue(TEXT("Weapon sniper is typed"), Sniper.bHasTypedWeaponBehaviour);
    TestEqual(TEXT("Weapon sniper field 40"), Sniper.WeaponField40, 41);
    TestEqual(TEXT("Weapon sniper texture"), Sniper.WeaponSniperTextureIndex, 44);
    TestEqual(TEXT("Weapon sniper mesh"), Sniper.WeaponSniperMeshIndex, 45);
    TestEqual(TEXT("Weapon sniper material"), Sniper.WeaponSniperMaterialId, 46);
    TestEqual(TEXT("Weapon RPG id"), Rpg.Id, 6);
    TestTrue(TEXT("Weapon RPG is typed"), Rpg.bHasTypedWeaponBehaviour);
    TestEqual(TEXT("Weapon RPG refs"), Rpg.WeaponRpgActorRefs.Num(), 4);
    TestEqual(TEXT("Weapon RPG ref 0"), Rpg.WeaponRpgActorRefs[0], 61);
    TestEqual(TEXT("Weapon RPG ref 3"), Rpg.WeaponRpgActorRefs[3], 64);

    UKyaLevelBankExporter* Exporter = NewObject<UKyaLevelBankExporter>();
    TArray<uint8> Plain;
    FMemoryWriter PlainWriter(Plain);
    if (!TestTrue(TEXT("Export unedited weapon fixture"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), PlainWriter, GWarn))) return false;
    TestEqual(TEXT("Unedited weapon export keeps the file size"), Plain.Num(), Input.Num());
    TestTrue(TEXT("Unedited weapon export preserves every byte"), Plain == Input);

    Actor.WeaponWeightB = 7.5f;
    Actor.Behaviours[0].WeaponField40 = 91;
    Actor.Behaviours[1].WeaponSniperMeshIndex = 92;
    Actor.Behaviours[2].WeaponRpgActorRefs[0] = 93;
    Actor.Behaviours[0].WeaponBurst[0].Field4 = 94;
    TArray<uint8> Edited;
    FMemoryWriter EditedWriter(Edited);
    if (!TestTrue(TEXT("Export edited weapon fixture"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), EditedWriter, GWarn))) return false;
    TestEqual(TEXT("Edited weapon export keeps the file size"), Edited.Num(), Input.Num());
    const uint8* EditedCursor = Edited.GetData();
    UKyaLevelBank* Reimported = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
        GetTransientPackage(), NAME_None, RF_Transient, nullptr, TEXT("bnk"), EditedCursor,
        Edited.GetData() + Edited.Num(), GWarn));
    if (!TestNotNull(TEXT("Reimport edited weapon fixture"), Reimported)) return false;
    const FKyaActorPlacement& EditedActor = Reimported->SceneEnvironment.Actors[0];
    TestEqual(TEXT("Edited weapon weight round trips"), EditedActor.WeaponWeightB, 7.5f);
    TestEqual(TEXT("Edited pistol field round trips"), EditedActor.Behaviours[0].WeaponField40, 91);
    TestEqual(TEXT("Edited sniper mesh round trips"),
        EditedActor.Behaviours[1].WeaponSniperMeshIndex, 92);
    TestEqual(TEXT("Edited RPG ref round trips"),
        EditedActor.Behaviours[2].WeaponRpgActorRefs[0], 93);
    TestEqual(TEXT("Edited pistol burst field round trips"),
        EditedActor.Behaviours[0].WeaponBurst[0].Field4, 94);

    Actor.Behaviours[2].WeaponRpgActorRefs.Add(99);
    TArray<uint8> Rejected;
    FMemoryWriter RejectedWriter(Rejected);
    TestFalse(TEXT("Exporter rejects a changed weapon RPG count"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), RejectedWriter, GWarn));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKyaAraignosFixtureRoundTripTest, "Kya.Bank.AraignosFixtureRoundTrip",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKyaAraignosFixtureRoundTripTest::RunTest(const FString& Parameters)
{
    FKyaSmallBankBuilder Builder;
    const TArray<uint8> Input = Builder.Build(0x21, false, false, false, false, false, true);
    UKyaLevelBankFactory* Factory = NewObject<UKyaLevelBankFactory>();
    const uint8* Cursor = Input.GetData();
    UKyaLevelBank* Bank = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
        GetTransientPackage(), NAME_None, RF_Transient, nullptr, TEXT("bnk"), Cursor,
        Input.GetData() + Input.Num(), GWarn));
    if (!TestNotNull(TEXT("Import Araignos fixture"), Bank)) return false;
    if (!TestEqual(TEXT("Araignos fixture actor count"), Bank->SceneEnvironment.Actors.Num(), 1)) return false;
    FKyaActorPlacement& Actor = Bank->SceneEnvironment.Actors[0];
    TestEqual(TEXT("Araignos class id"), Actor.ClassId, int64(0x21));
    TestEqual(TEXT("Araignos actor name"), Actor.Name, FString(TEXT("ARAIGNOS")));
    TestTrue(TEXT("Araignos class payload parses"), Actor.bHasAraignosData);
    TestEqual(TEXT("Araignos weight"), Actor.AraignosWeightB, 1.25f);
    TestEqual(TEXT("Araignos path"), Actor.AraignosPathFollowIndex, 3);
    TestEqual(TEXT("Araignos field 1f8"), Actor.AraignosField1F8, 2.0f);
    TestEqual(TEXT("Araignos damage"), Actor.AraignosDamage, 4.0f);
    TestEqual(TEXT("Araignos sound 240"), Actor.AraignosSoundRef240, 5);
    TestEqual(TEXT("Araignos add-on money type"), Actor.AraignosAddOn.MoneyType, 2);
    TestEqual(TEXT("Araignos add-on sub-object mode"), Actor.AraignosAddOn.SubObjMode, 1);
    TestEqual(TEXT("Araignos add-on float 0"), Actor.AraignosAddOn.SubObjField0, 0.25f);
    TestEqual(TEXT("Araignos add-on float 14"), Actor.AraignosAddOn.SubObjField14, 1.5f);
    TestEqual(TEXT("Araignos fixture leaves no raw class data"), Actor.ClassData.Num(), 0);
    if (!TestEqual(TEXT("Araignos behaviour count"), Actor.Behaviours.Num(), 1)) return false;
    TestEqual(TEXT("Araignos behaviour id"), Actor.Behaviours[0].Id, 2);
    TestEqual(TEXT("Araignos behaviour is empty"), Actor.Behaviours[0].Data.Num(), 0);

    UKyaLevelBankExporter* Exporter = NewObject<UKyaLevelBankExporter>();
    TArray<uint8> Plain;
    FMemoryWriter PlainWriter(Plain);
    if (!TestTrue(TEXT("Export unedited Araignos fixture"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), PlainWriter, GWarn))) return false;
    TestEqual(TEXT("Unedited Araignos export keeps the file size"), Plain.Num(), Input.Num());
    TestTrue(TEXT("Unedited Araignos export preserves every byte"), Plain == Input);

    Actor.AraignosWeightB = 9.5f;
    Actor.AraignosPathFollowIndex = 12;
    Actor.AraignosDamage = 44.0f;
    Actor.AraignosSoundRef23C = 77;
    Actor.AraignosAddOn.MoneyCount = 88;
    Actor.AraignosAddOn.SubObjFieldC = 1.75f;
    TArray<uint8> Edited;
    FMemoryWriter EditedWriter(Edited);
    if (!TestTrue(TEXT("Export edited Araignos fixture"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), EditedWriter, GWarn))) return false;
    TestEqual(TEXT("Edited Araignos export keeps the file size"), Edited.Num(), Input.Num());
    const uint8* EditedCursor = Edited.GetData();
    UKyaLevelBank* Reimported = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
        GetTransientPackage(), NAME_None, RF_Transient, nullptr, TEXT("bnk"), EditedCursor,
        Edited.GetData() + Edited.Num(), GWarn));
    if (!TestNotNull(TEXT("Reimport edited Araignos fixture"), Reimported)) return false;
    const FKyaActorPlacement& EditedActor = Reimported->SceneEnvironment.Actors[0];
    TestEqual(TEXT("Edited Araignos weight round trips"), EditedActor.AraignosWeightB, 9.5f);
    TestEqual(TEXT("Edited Araignos path round trips"), EditedActor.AraignosPathFollowIndex, 12);
    TestEqual(TEXT("Edited Araignos damage round trips"), EditedActor.AraignosDamage, 44.0f);
    TestEqual(TEXT("Edited Araignos sound round trips"), EditedActor.AraignosSoundRef23C, 77);
    TestEqual(TEXT("Edited Araignos money count round trips"), EditedActor.AraignosAddOn.MoneyCount, 88);
    TestEqual(TEXT("Edited Araignos sub-object float round trips"),
        EditedActor.AraignosAddOn.SubObjFieldC, 1.75f);

    Actor.AraignosAddOn.SubObjMode = 0;
    TArray<uint8> Rejected;
    FMemoryWriter RejectedWriter(Rejected);
    TestFalse(TEXT("Exporter rejects an Araignos sub-object mode change"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), RejectedWriter, GWarn));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKyaBonusFixtureRoundTripTest, "Kya.Bank.BonusFixtureRoundTrip",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKyaBonusFixtureRoundTripTest::RunTest(const FString& Parameters)
{
    FKyaSmallBankBuilder Builder;
    const TArray<uint8> Input = Builder.Build(0x0e, false, false, false, false, false, false, true);
    UKyaLevelBankFactory* Factory = NewObject<UKyaLevelBankFactory>();
    const uint8* Cursor = Input.GetData();
    UKyaLevelBank* Bank = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
        GetTransientPackage(), NAME_None, RF_Transient, nullptr, TEXT("bnk"), Cursor,
        Input.GetData() + Input.Num(), GWarn));
    if (!TestNotNull(TEXT("Import bonus fixture"), Bank)) return false;
    if (!TestEqual(TEXT("Bonus fixture actor count"), Bank->SceneEnvironment.Actors.Num(), 1)) return false;
    FKyaActorPlacement& Actor = Bank->SceneEnvironment.Actors[0];
    TestEqual(TEXT("Bonus class id"), Actor.ClassId, int64(0x0e));
    TestEqual(TEXT("Bonus actor name"), Actor.Name, FString(TEXT("BONUS")));
    TestTrue(TEXT("Bonus class payload parses"), Actor.bHasBonusData);
    TestEqual(TEXT("Bonus weight"), Actor.BonusWeightB, 1.5f);
    TestEqual(TEXT("Bonus sound"), Actor.BonusSoundRef, 9);
    TestEqual(TEXT("Bonus flare material"), Actor.BonusFlareMaterialId, 10);
    TestEqual(TEXT("Bonus animation material"), Actor.BonusAnimMaterialId, 11);
    TestEqual(TEXT("Bonus fixture leaves no raw class data"), Actor.ClassData.Num(), 0);
    if (!TestEqual(TEXT("Bonus behaviour count"), Actor.Behaviours.Num(), 4)) return false;
    const FKyaActorBehaviour& Turn = Actor.Behaviours[0];
    const FKyaActorBehaviour& Path = Actor.Behaviours[1];
    const FKyaActorBehaviour& Flock = Actor.Behaviours[2];
    const FKyaActorBehaviour& AddOn = Actor.Behaviours[3];
    TestEqual(TEXT("Bonus turn id"), Turn.Id, 3);
    TestTrue(TEXT("Bonus turn is typed"), Turn.bHasTypedBonusBehaviour);
    TestEqual(TEXT("Bonus turn field 14"), Turn.BonusField14, 1.0f);
    TestEqual(TEXT("Bonus turn value"), Turn.BonusTurnValue, 8.0f);
    TestEqual(TEXT("Bonus path id"), Path.Id, 4);
    TestTrue(TEXT("Bonus path is typed"), Path.bHasTypedBonusBehaviour);
    TestEqual(TEXT("Bonus path field 14"), Path.BonusField14, 11.0f);
    TestEqual(TEXT("Bonus path index"), Path.BonusPathIndex, 18);
    TestEqual(TEXT("Bonus path field 1f4"), Path.BonusPathField1F4, 19.0f);
    TestEqual(TEXT("Bonus path field 204"), Path.BonusPathField204, 23);
    TestEqual(TEXT("Bonus flock id"), Flock.Id, 5);
    TestTrue(TEXT("Bonus flock is typed"), Flock.bHasTypedBonusBehaviour);
    TestEqual(TEXT("Bonus flock path"), Flock.BonusFlockPathIndex, 31);
    TestEqual(TEXT("Bonus add-on id"), AddOn.Id, 6);
    TestTrue(TEXT("Bonus add-on is typed"), AddOn.bHasTypedBonusBehaviour);
    TestEqual(TEXT("Bonus add-on field"), AddOn.BonusAddOnField18, 32.0f);

    UKyaLevelBankExporter* Exporter = NewObject<UKyaLevelBankExporter>();
    TArray<uint8> Plain;
    FMemoryWriter PlainWriter(Plain);
    if (!TestTrue(TEXT("Export unedited bonus fixture"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), PlainWriter, GWarn))) return false;
    TestEqual(TEXT("Unedited bonus export keeps the file size"), Plain.Num(), Input.Num());
    TestTrue(TEXT("Unedited bonus export preserves every byte"), Plain == Input);

    Actor.BonusWeightB = 9.5f;
    Actor.BonusFlareMaterialId = 41;
    Actor.Behaviours[0].BonusTurnValue = 42.0f;
    Actor.Behaviours[1].BonusPathIndex = 43;
    Actor.Behaviours[1].BonusPathField200 = 44.0f;
    Actor.Behaviours[2].BonusFlockPathIndex = 45;
    Actor.Behaviours[3].BonusAddOnField18 = 46.0f;
    TArray<uint8> Edited;
    FMemoryWriter EditedWriter(Edited);
    if (!TestTrue(TEXT("Export edited bonus fixture"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), EditedWriter, GWarn))) return false;
    TestEqual(TEXT("Edited bonus export keeps the file size"), Edited.Num(), Input.Num());
    const uint8* EditedCursor = Edited.GetData();
    UKyaLevelBank* Reimported = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
        GetTransientPackage(), NAME_None, RF_Transient, nullptr, TEXT("bnk"), EditedCursor,
        Edited.GetData() + Edited.Num(), GWarn));
    if (!TestNotNull(TEXT("Reimport edited bonus fixture"), Reimported)) return false;
    const FKyaActorPlacement& EditedActor = Reimported->SceneEnvironment.Actors[0];
    TestEqual(TEXT("Edited bonus weight round trips"), EditedActor.BonusWeightB, 9.5f);
    TestEqual(TEXT("Edited bonus flare round trips"), EditedActor.BonusFlareMaterialId, 41);
    TestEqual(TEXT("Edited bonus turn round trips"), EditedActor.Behaviours[0].BonusTurnValue, 42.0f);
    TestEqual(TEXT("Edited bonus path round trips"), EditedActor.Behaviours[1].BonusPathIndex, 43);
    TestEqual(TEXT("Edited bonus path field round trips"),
        EditedActor.Behaviours[1].BonusPathField200, 44.0f);
    TestEqual(TEXT("Edited bonus flock round trips"),
        EditedActor.Behaviours[2].BonusFlockPathIndex, 45);
    TestEqual(TEXT("Edited bonus add-on round trips"),
        EditedActor.Behaviours[3].BonusAddOnField18, 46.0f);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKyaTeleporterFixtureRoundTripTest, "Kya.Bank.TeleporterFixtureRoundTrip",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKyaTeleporterFixtureRoundTripTest::RunTest(const FString& Parameters)
{
    FKyaSmallBankBuilder Builder;
    const TArray<uint8> Input = Builder.Build(0x24);
    UKyaLevelBankFactory* Factory = NewObject<UKyaLevelBankFactory>();
    const uint8* Cursor = Input.GetData();
    UKyaLevelBank* Bank = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
        GetTransientPackage(), NAME_None, RF_Transient, nullptr, TEXT("bnk"), Cursor,
        Input.GetData() + Input.Num(), GWarn));
    if (!TestNotNull(TEXT("Import teleporter fixture"), Bank)) return false;
    if (!TestEqual(TEXT("Teleporter fixture actor count"), Bank->SceneEnvironment.Actors.Num(), 1)) return false;
    FKyaActorPlacement& Actor = Bank->SceneEnvironment.Actors[0];
    TestEqual(TEXT("Teleporter class id"), Actor.ClassId, int64(0x24));
    TestEqual(TEXT("Teleporter actor name"), Actor.Name, FString(TEXT("TELEPORTER")));
    TestTrue(TEXT("Teleporter class payload parses"), Actor.bHasTeleporterData);
    TestEqual(TEXT("Teleporter subsector material"), Actor.TeleporterSubsectorMaterialId, 3);
    TestEqual(TEXT("Teleporter hash"), Actor.TeleporterHash, int64(0x5566778811223344ll));
    TestEqual(TEXT("Teleporter field 16c"), Actor.TeleporterField16C, 4);
    if (!TestEqual(TEXT("Teleporter destination count"), Actor.TeleporterDestinations.Num(), 1)) return false;
    TestEqual(TEXT("Teleporter destination level"), Actor.TeleporterDestinations[0].LevelId, 5);
    TestEqual(TEXT("Teleporter destination elevator"), Actor.TeleporterDestinations[0].ElevatorId, 6);
    if (!TestEqual(TEXT("Teleporter switch 0 targets"), Actor.TeleporterSwitch170.Targets.Num(), 1)) return false;
    TestEqual(TEXT("Teleporter switch 0 target"), Actor.TeleporterSwitch170.Targets[0].TargetRef, 21);
    TestEqual(TEXT("Teleporter switch 0 camera"), Actor.TeleporterSwitch170.Camera.CameraIndex, 31);
    TestEqual(TEXT("Teleporter switch 1 targets"), Actor.TeleporterSwitch178.Targets.Num(), 0);
    TestEqual(TEXT("Teleporter switch 2 targets"), Actor.TeleporterSwitch180.Targets.Num(), 0);
    TestEqual(TEXT("Teleporter zone 188"), Actor.TeleporterZone188, 11);
    TestEqual(TEXT("Teleporter cinematic"), Actor.TeleporterCinematicId, 14);
    TestEqual(TEXT("Teleporter field 1e0"), Actor.TeleporterField1E0, 19);
    if (!TestEqual(TEXT("Teleporter waypoint count"), Actor.TeleporterWaypoints.Num(), 2)) return false;
    TestEqual(TEXT("Teleporter waypoint 0 actor"), Actor.TeleporterWaypoints[0].ActorRef, 61);
    TestEqual(TEXT("Teleporter waypoint 1 waypoint"), Actor.TeleporterWaypoints[1].WaypointRef, 64);
    if (!TestEqual(TEXT("Teleporter block count"), Actor.TeleporterConditionedOperations.Num(), 1)) return false;
    const FKyaTeleporterBlock& Block = Actor.TeleporterConditionedOperations[0];
    if (!TestEqual(TEXT("Teleporter op count"), Block.Operations.Num(), 2)) return false;
    TestEqual(TEXT("Teleporter op 0 variable"), Block.Operations[0].ScenaricVariable, 71);
    TestEqual(TEXT("Teleporter op 1 value"), Block.Operations[1].Value, 76);
    if (!TestEqual(TEXT("Teleporter condition count"), Block.Conditions.Num(), 2)) return false;
    TestEqual(TEXT("Teleporter condition 0 combine"), Block.Conditions[0].Combine, 81);
    TestEqual(TEXT("Teleporter condition 1 value"), Block.Conditions[1].Value, 88);
    TestEqual(TEXT("Teleporter fixture leaves no raw class data"), Actor.ClassData.Num(), 0);
    if (!TestEqual(TEXT("Teleporter behaviour count"), Actor.Behaviours.Num(), 1)) return false;
    TestEqual(TEXT("Teleporter behaviour id"), Actor.Behaviours[0].Id, 2);
    TestEqual(TEXT("Teleporter behaviour is empty"), Actor.Behaviours[0].Data.Num(), 0);

    UKyaLevelBankExporter* Exporter = NewObject<UKyaLevelBankExporter>();
    TArray<uint8> Plain;
    FMemoryWriter PlainWriter(Plain);
    if (!TestTrue(TEXT("Export unedited teleporter fixture"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), PlainWriter, GWarn))) return false;
    TestEqual(TEXT("Unedited teleporter export keeps the file size"), Plain.Num(), Input.Num());
    TestTrue(TEXT("Unedited teleporter export preserves every byte"), Plain == Input);

    Actor.TeleporterSubsectorMaterialId = 91;
    Actor.TeleporterHash = 0x1122334455667788ll;
    Actor.TeleporterDestinations[0].LevelId = 92;
    Actor.TeleporterSwitch170.Targets[0].TargetRef = 93;
    Actor.TeleporterWaypoints[0].ActorRef = 94;
    Actor.TeleporterConditionedOperations[0].Operations[1].Value = 95;
    Actor.TeleporterConditionedOperations[0].Conditions[0].Value = 96;
    TArray<uint8> Edited;
    FMemoryWriter EditedWriter(Edited);
    if (!TestTrue(TEXT("Export edited teleporter fixture"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), EditedWriter, GWarn))) return false;
    TestEqual(TEXT("Edited teleporter export keeps the file size"), Edited.Num(), Input.Num());
    const uint8* EditedCursor = Edited.GetData();
    UKyaLevelBank* Reimported = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
        GetTransientPackage(), NAME_None, RF_Transient, nullptr, TEXT("bnk"), EditedCursor,
        Edited.GetData() + Edited.Num(), GWarn));
    if (!TestNotNull(TEXT("Reimport edited teleporter fixture"), Reimported)) return false;
    const FKyaActorPlacement& EditedActor = Reimported->SceneEnvironment.Actors[0];
    TestEqual(TEXT("Edited teleporter material round trips"), EditedActor.TeleporterSubsectorMaterialId, 91);
    TestEqual(TEXT("Edited teleporter hash round trips"), EditedActor.TeleporterHash,
        int64(0x1122334455667788ll));
    TestEqual(TEXT("Edited teleporter destination round trips"),
        EditedActor.TeleporterDestinations[0].LevelId, 92);
    TestEqual(TEXT("Edited teleporter switch target round trips"),
        EditedActor.TeleporterSwitch170.Targets[0].TargetRef, 93);
    TestEqual(TEXT("Edited teleporter waypoint round trips"),
        EditedActor.TeleporterWaypoints[0].ActorRef, 94);
    TestEqual(TEXT("Edited teleporter operation round trips"),
        EditedActor.TeleporterConditionedOperations[0].Operations[1].Value, 95);
    TestEqual(TEXT("Edited teleporter condition round trips"),
        EditedActor.TeleporterConditionedOperations[0].Conditions[0].Value, 96);

    Actor.TeleporterDestinations.AddDefaulted();
    TArray<uint8> Rejected;
    FMemoryWriter RejectedWriter(Rejected);
    TestFalse(TEXT("Exporter rejects a changed teleporter destination count"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), RejectedWriter, GWarn));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKyaAmortosFixtureRoundTripTest, "Kya.Bank.AmortosFixtureRoundTrip",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKyaAmortosFixtureRoundTripTest::RunTest(const FString& Parameters)
{
    FKyaSmallBankBuilder Builder;
    const TArray<uint8> Input = Builder.Build(0x29);
    UKyaLevelBankFactory* Factory = NewObject<UKyaLevelBankFactory>();
    const uint8* Cursor = Input.GetData();
    UKyaLevelBank* Bank = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
        GetTransientPackage(), NAME_None, RF_Transient, nullptr, TEXT("bnk"), Cursor,
        Input.GetData() + Input.Num(), GWarn));
    if (!TestNotNull(TEXT("Import Amortos fixture"), Bank)) return false;
    if (!TestEqual(TEXT("Amortos fixture actor count"), Bank->SceneEnvironment.Actors.Num(), 1)) return false;
    FKyaActorPlacement& Actor = Bank->SceneEnvironment.Actors[0];
    TestEqual(TEXT("Amortos class id"), Actor.ClassId, int64(0x29));
    TestEqual(TEXT("Amortos actor name"), Actor.Name, FString(TEXT("AMORTOS")));
    TestTrue(TEXT("Amortos class payload parses"), Actor.bHasAmortosData);
    if (!TestEqual(TEXT("Amortos checkpoint list"), Actor.AmortosCheckpointActors.Num(), 1)) return false;
    TestEqual(TEXT("Amortos checkpoint ref"), Actor.AmortosCheckpointActors[0], 11);
    TestEqual(TEXT("Amortos stream 188"), Actor.AmortosActors188.Num(), 1);
    TestEqual(TEXT("Amortos stream 188 ref"), Actor.AmortosActors188[0], 12);
    TestEqual(TEXT("Amortos stream 18c"), Actor.AmortosActors18C.Num(), 0);
    TestEqual(TEXT("Amortos actor ref"), Actor.AmortosActorRef, 13);
    // Source serializes 164,168,160,16C,...
    TestEqual(TEXT("Amortos field 164"), Actor.AmortosField164, 1.0f);
    TestEqual(TEXT("Amortos field 168"), Actor.AmortosField168, 2.0f);
    TestEqual(TEXT("Amortos field 160"), Actor.AmortosField160, 3.0f);
    TestEqual(TEXT("Amortos field 16c"), Actor.AmortosField16C, 4.0f);
    TestEqual(TEXT("Amortos field 170"), Actor.AmortosField170, 5.0f);
    TestEqual(TEXT("Amortos field 174"), Actor.AmortosField174, 6.0f);
    TestEqual(TEXT("Amortos field 178"), Actor.AmortosField178, 7.0f);
    TestEqual(TEXT("Amortos field 17c"), Actor.AmortosField17C, 8.0f);
    TestEqual(TEXT("Amortos field 180"), Actor.AmortosField180, 9.0f);
    TestEqual(TEXT("Amortos add-on money type"), Actor.AmortosAddOn.MoneyType, 2);
    TestEqual(TEXT("Amortos add-on money count"), Actor.AmortosAddOn.MoneyCount, 3);
    TestEqual(TEXT("Amortos add-on sub-object mode"), Actor.AmortosAddOn.SubObjMode, 1);
    TestEqual(TEXT("Amortos add-on float 0"), Actor.AmortosAddOn.SubObjField0, 0.25f);
    TestEqual(TEXT("Amortos add-on float 14"), Actor.AmortosAddOn.SubObjField14, 1.5f);
    TestEqual(TEXT("Amortos field 1c8"), Actor.AmortosField1C8, 14);
    if (!TestEqual(TEXT("Amortos switch 0 targets"), Actor.AmortosSwitch1CC.Targets.Num(), 1)) return false;
    TestEqual(TEXT("Amortos switch 0 target"), Actor.AmortosSwitch1CC.Targets[0].TargetRef, 21);
    TestEqual(TEXT("Amortos switch 0 camera"), Actor.AmortosSwitch1CC.Camera.CameraIndex, 31);
    TestEqual(TEXT("Amortos switch 1 targets"), Actor.AmortosSwitch1D4.Targets.Num(), 0);
    TestEqual(TEXT("Amortos switch 1 camera"), Actor.AmortosSwitch1D4.Camera.CameraIndex, 41);
    TestEqual(TEXT("Amortos fixture leaves no raw class data"), Actor.ClassData.Num(), 0);
    if (!TestEqual(TEXT("Amortos behaviour count"), Actor.Behaviours.Num(), 1)) return false;
    TestEqual(TEXT("Amortos behaviour id"), Actor.Behaviours[0].Id, 2);
    TestEqual(TEXT("Amortos behaviour is empty"), Actor.Behaviours[0].Data.Num(), 0);

    UKyaLevelBankExporter* Exporter = NewObject<UKyaLevelBankExporter>();
    TArray<uint8> Plain;
    FMemoryWriter PlainWriter(Plain);
    if (!TestTrue(TEXT("Export unedited Amortos fixture"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), PlainWriter, GWarn))) return false;
    TestEqual(TEXT("Unedited Amortos export keeps the file size"), Plain.Num(), Input.Num());
    TestTrue(TEXT("Unedited Amortos export preserves every byte"), Plain == Input);

    Actor.AmortosField164 = 21.5f;
    Actor.AmortosActorRef = 22;
    Actor.AmortosField1C8 = 23;
    Actor.AmortosAddOn.MoneyCount = 24;
    Actor.AmortosSwitch1CC.Targets[0].TargetRef = 25;
    Actor.AmortosSwitch1D4.Camera.CameraIndex = 26;
    Actor.AmortosAddOn.SubObjFieldC = 27.5f;
    TArray<uint8> Edited;
    FMemoryWriter EditedWriter(Edited);
    if (!TestTrue(TEXT("Export edited Amortos fixture"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), EditedWriter, GWarn))) return false;
    TestEqual(TEXT("Edited Amortos export keeps the file size"), Edited.Num(), Input.Num());
    const uint8* EditedCursor = Edited.GetData();
    UKyaLevelBank* Reimported = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
        GetTransientPackage(), NAME_None, RF_Transient, nullptr, TEXT("bnk"), EditedCursor,
        Edited.GetData() + Edited.Num(), GWarn));
    if (!TestNotNull(TEXT("Reimport edited Amortos fixture"), Reimported)) return false;
    const FKyaActorPlacement& EditedActor = Reimported->SceneEnvironment.Actors[0];
    TestEqual(TEXT("Edited Amortos field 164 round trips"), EditedActor.AmortosField164, 21.5f);
    TestEqual(TEXT("Edited Amortos actor ref round trips"), EditedActor.AmortosActorRef, 22);
    TestEqual(TEXT("Edited Amortos field 1c8 round trips"), EditedActor.AmortosField1C8, 23);
    TestEqual(TEXT("Edited Amortos money count round trips"), EditedActor.AmortosAddOn.MoneyCount, 24);
    TestEqual(TEXT("Edited Amortos switch target round trips"),
        EditedActor.AmortosSwitch1CC.Targets[0].TargetRef, 25);
    TestEqual(TEXT("Edited Amortos switch camera round trips"),
        EditedActor.AmortosSwitch1D4.Camera.CameraIndex, 26);
    TestEqual(TEXT("Edited Amortos sub-object float round trips"),
        EditedActor.AmortosAddOn.SubObjFieldC, 27.5f);

    Actor.AmortosAddOn.SubObjMode = 0;
    TArray<uint8> Rejected;
    FMemoryWriter RejectedWriter(Rejected);
    TestFalse(TEXT("Exporter rejects an Amortos sub-object mode change"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), RejectedWriter, GWarn));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKyaDcaFixtureRoundTripTest, "Kya.Bank.DcaFixtureRoundTrip",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKyaDcaFixtureRoundTripTest::RunTest(const FString& Parameters)
{
    FKyaSmallBankBuilder Builder;
    const TArray<uint8> Input = Builder.Build(0x1b);
    UKyaLevelBankFactory* Factory = NewObject<UKyaLevelBankFactory>();
    const uint8* Cursor = Input.GetData();
    UKyaLevelBank* Bank = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
        GetTransientPackage(), NAME_None, RF_Transient, nullptr, TEXT("bnk"), Cursor,
        Input.GetData() + Input.Num(), GWarn));
    if (!TestNotNull(TEXT("Import DCA fixture"), Bank)) return false;
    if (!TestEqual(TEXT("DCA fixture actor count"), Bank->SceneEnvironment.Actors.Num(), 1)) return false;
    FKyaActorPlacement& Actor = Bank->SceneEnvironment.Actors[0];
    TestEqual(TEXT("DCA class id"), Actor.ClassId, int64(0x1b));
    TestEqual(TEXT("DCA actor name"), Actor.Name, FString(TEXT("DCA")));
    TestTrue(TEXT("DCA class payload parses"), Actor.bHasDcaData);
    TestEqual(TEXT("DCA field 504"), Actor.DcaField504, 1);
    TestEqual(TEXT("DCA field 500"), Actor.DcaField500, 2);
    TestEqual(TEXT("DCA field 464"), Actor.DcaField464, 3.5f);
    TestEqual(TEXT("DCA field 508"), Actor.DcaField508, 4);
    TestEqual(TEXT("DCA field 50c"), Actor.DcaField50C, 5);
    TestEqual(TEXT("DCA aim adjust"), Actor.DcaAimAdjustDegrees, 6.5f);
    if (!TestEqual(TEXT("DCA shot stream"), Actor.DcaShotActorRefs.Num(), 1)) return false;
    TestEqual(TEXT("DCA shot ref"), Actor.DcaShotActorRefs[0], 7);
    TestEqual(TEXT("DCA field 450"), Actor.DcaField450, 8);
    TestEqual(TEXT("DCA field 454"), Actor.DcaField454, 9);
    TestEqual(TEXT("DCA fixture leaves no raw class data"), Actor.ClassData.Num(), 0);
    if (!TestEqual(TEXT("DCA behaviour count"), Actor.Behaviours.Num(), 3)) return false;
    const FKyaActorBehaviour& Default = Actor.Behaviours[0];
    const FKyaActorBehaviour& Controlled = Actor.Behaviours[1];
    const FKyaActorBehaviour& Trajectory = Actor.Behaviours[2];
    TestEqual(TEXT("DCA default id"), Default.Id, 2);
    TestEqual(TEXT("DCA default payload is empty"), Default.Data.Num(), 0);
    TestEqual(TEXT("DCA controlled id"), Controlled.Id, 3);
    TestTrue(TEXT("DCA controlled is typed"), Controlled.bHasTypedDcaControlledBehaviour);
    TestEqual(TEXT("DCA controlled field 0"), Controlled.DcaControlledField0, 1.0f);
    TestEqual(TEXT("DCA controlled field 4"), Controlled.DcaControlledField4, 2.0f);
    TestEqual(TEXT("DCA controlled fov"), Controlled.DcaControlledFovDegrees, 4.0f);
    TestEqual(TEXT("DCA controlled vector 10 x"), float(Controlled.DcaControlledVector10.X), 5.0f);
    TestEqual(TEXT("DCA controlled vector 20 z"), float(Controlled.DcaControlledVector20.Z), 10.0f);
    TestEqual(TEXT("DCA trajectory id"), Trajectory.Id, 5);
    TestTrue(TEXT("DCA trajectory is typed"), Trajectory.bHasTypedDcaTrajectoryBehaviour);
    TestEqual(TEXT("DCA trajectory actor"), Trajectory.DcaTrajectoryActorRef, 11);
    TestEqual(TEXT("DCA trajectory field 18"), Trajectory.DcaTrajectoryField18, 12.5f);
    TestEqual(TEXT("DCA trajectory field 8"), Trajectory.DcaTrajectoryField8, 13);

    UKyaLevelBankExporter* Exporter = NewObject<UKyaLevelBankExporter>();
    TArray<uint8> Plain;
    FMemoryWriter PlainWriter(Plain);
    if (!TestTrue(TEXT("Export unedited DCA fixture"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), PlainWriter, GWarn))) return false;
    TestEqual(TEXT("Unedited DCA export keeps the file size"), Plain.Num(), Input.Num());
    TestTrue(TEXT("Unedited DCA export preserves every byte"), Plain == Input);

    Actor.DcaField504 = 21;
    Actor.DcaField464 = 22.5f;
    Actor.DcaAimAdjustDegrees = 23.5f;
    Actor.DcaShotActorRefs[0] = 24;
    Actor.Behaviours[1].DcaControlledFovDegrees = 25.5f;
    Actor.Behaviours[2].DcaTrajectoryField18 = 26.5f;
    TArray<uint8> Edited;
    FMemoryWriter EditedWriter(Edited);
    if (!TestTrue(TEXT("Export edited DCA fixture"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), EditedWriter, GWarn))) return false;
    TestEqual(TEXT("Edited DCA export keeps the file size"), Edited.Num(), Input.Num());
    const uint8* EditedCursor = Edited.GetData();
    UKyaLevelBank* Reimported = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
        GetTransientPackage(), NAME_None, RF_Transient, nullptr, TEXT("bnk"), EditedCursor,
        Edited.GetData() + Edited.Num(), GWarn));
    if (!TestNotNull(TEXT("Reimport edited DCA fixture"), Reimported)) return false;
    const FKyaActorPlacement& EditedActor = Reimported->SceneEnvironment.Actors[0];
    TestEqual(TEXT("Edited DCA field 504 round trips"), EditedActor.DcaField504, 21);
    TestEqual(TEXT("Edited DCA field 464 round trips"), EditedActor.DcaField464, 22.5f);
    TestEqual(TEXT("Edited DCA aim adjust round trips"), EditedActor.DcaAimAdjustDegrees, 23.5f);
    TestEqual(TEXT("Edited DCA shot ref round trips"), EditedActor.DcaShotActorRefs[0], 24);
    TestEqual(TEXT("Edited DCA controlled fov round trips"),
        EditedActor.Behaviours[1].DcaControlledFovDegrees, 25.5f);
    TestEqual(TEXT("Edited DCA trajectory field round trips"),
        EditedActor.Behaviours[2].DcaTrajectoryField18, 26.5f);

    Actor.DcaShotActorRefs.Add(99);
    TArray<uint8> Rejected;
    FMemoryWriter RejectedWriter(Rejected);
    TestFalse(TEXT("Exporter rejects a changed DCA shot count"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), RejectedWriter, GWarn));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKyaFruitFixtureRoundTripTest, "Kya.Bank.FruitFixtureRoundTrip",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKyaFruitFixtureRoundTripTest::RunTest(const FString& Parameters)
{
    FKyaSmallBankBuilder Builder;
    const TArray<uint8> Input = Builder.Build(0x16);
    UKyaLevelBankFactory* Factory = NewObject<UKyaLevelBankFactory>();
    const uint8* Cursor = Input.GetData();
    UKyaLevelBank* Bank = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
        GetTransientPackage(), NAME_None, RF_Transient, nullptr, TEXT("bnk"), Cursor,
        Input.GetData() + Input.Num(), GWarn));
    if (!TestNotNull(TEXT("Import fruit fixture"), Bank)) return false;
    if (!TestEqual(TEXT("Fruit fixture actor count"), Bank->SceneEnvironment.Actors.Num(), 1)) return false;
    FKyaActorPlacement& Actor = Bank->SceneEnvironment.Actors[0];
    TestEqual(TEXT("Fruit class id"), Actor.ClassId, int64(0x16));
    TestEqual(TEXT("Fruit actor name"), Actor.Name, FString(TEXT("FRUIT")));
    TestTrue(TEXT("Fruit class payload parses"), Actor.bHasFruitData);
    TestEqual(TEXT("Fruit weight"), Actor.FruitWeightB, 1.25f);
    TestEqual(TEXT("Fruit life maximum"), Actor.FruitLifeMax, int64(16777217));
    TestEqual(TEXT("Fruit gravity"), Actor.FruitGravityScale, 2.5f);
    TestEqual(TEXT("Fruit dynamic ext 4"), Actor.FruitDynamicExtField4, 0.5f);
    TestEqual(TEXT("Fruit dynamic ext 8"), Actor.FruitDynamicExtField8, 0.75f);
    TestEqual(TEXT("Fruit field 360"), Actor.FruitField360, 55);
    if (!TestEqual(TEXT("Fruit switch targets"), Actor.FruitSwitch.Targets.Num(), 1)) return false;
    TestEqual(TEXT("Fruit switch target"), Actor.FruitSwitch.Targets[0].TargetRef, 31);
    TestEqual(TEXT("Fruit switch camera"), Actor.FruitSwitch.Camera.CameraIndex, 41);
    TestEqual(TEXT("Fruit fixture leaves no raw class data"), Actor.ClassData.Num(), 0);
    if (!TestEqual(TEXT("Fruit behaviour count"), Actor.Behaviours.Num(), 5)) return false;
    const FKyaActorBehaviour& Suspended = Actor.Behaviours[0];
    const FKyaActorBehaviour& Squashed = Actor.Behaviours[1];
    const FKyaActorBehaviour& Inventory = Actor.Behaviours[2];
    const FKyaActorBehaviour& Bonus = Actor.Behaviours[3];
    const FKyaActorBehaviour& Projectile = Actor.Behaviours[4];
    TestEqual(TEXT("Fruit suspended id"), Suspended.Id, 3);
    TestEqual(TEXT("Fruit suspended payload is empty"), Suspended.Data.Num(), 0);
    TestEqual(TEXT("Fruit squashed id"), Squashed.Id, 4);
    TestTrue(TEXT("Fruit squashed is typed"), Squashed.bHasTypedFruitBehaviour);
    TestEqual(TEXT("Fruit squashed value"), Squashed.FruitSquashedValue, 1.5f);
    TestEqual(TEXT("Fruit inventory id"), Inventory.Id, 5);
    TestTrue(TEXT("Fruit inventory is typed"), Inventory.bHasTypedFruitBehaviour);
    TestEqual(TEXT("Fruit inventory field 0"), Inventory.FruitInventoryField0, 2);
    TestEqual(TEXT("Fruit inventory purchase"), Inventory.FruitInventoryPurchaseId, 13);
    TestEqual(TEXT("Fruit inventory money"), Inventory.FruitInventoryMoneyCost, 15);
    TestFalse(TEXT("Fruit inventory has no object"), Inventory.FruitInventoryHasObject);
    TestEqual(TEXT("Fruit inventory initial behaviour"), Inventory.FruitInventoryInitialBehaviour, 3);
    TestEqual(TEXT("Fruit bonus id"), Bonus.Id, 6);
    TestTrue(TEXT("Fruit bonus is typed"), Bonus.bHasTypedFruitBehaviour);
    TestEqual(TEXT("Fruit bonus actor"), Bonus.FruitBonusActorRef, 21);
    TestEqual(TEXT("Fruit bonus initial behaviour"), Bonus.FruitBonusInitialBehaviour, 22);
    TestEqual(TEXT("Fruit projectile id"), Projectile.Id, 7);
    TestFalse(TEXT("Fruit projectile stays raw"), Projectile.bHasTypedFruitBehaviour);
    TestEqual(TEXT("Fruit projectile raw size"), Projectile.Data.Num(), 4);

    UKyaLevelBankExporter* Exporter = NewObject<UKyaLevelBankExporter>();
    TArray<uint8> Plain;
    FMemoryWriter PlainWriter(Plain);
    if (!TestTrue(TEXT("Export unedited fruit fixture"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), PlainWriter, GWarn))) return false;
    TestEqual(TEXT("Unedited fruit export keeps the file size"), Plain.Num(), Input.Num());
    TestTrue(TEXT("Unedited fruit export preserves every byte"), Plain == Input);

    Actor.FruitWeightB = 7.5f;
    Actor.FruitLifeMax = 4000000000ll;
    Actor.FruitField360 = 91;
    Actor.FruitSwitch.Targets[0].TargetRef = 92;
    Actor.Behaviours[1].FruitSquashedValue = 93.5f;
    Actor.Behaviours[2].FruitInventoryMoneyCost = 94;
    Actor.Behaviours[2].FruitInventoryInitialBehaviour = 4;
    Actor.Behaviours[3].FruitBonusInitialBehaviour = 95;
    TArray<uint8> Edited;
    FMemoryWriter EditedWriter(Edited);
    if (!TestTrue(TEXT("Export edited fruit fixture"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), EditedWriter, GWarn))) return false;
    TestEqual(TEXT("Edited fruit export keeps the file size"), Edited.Num(), Input.Num());
    const uint8* EditedCursor = Edited.GetData();
    UKyaLevelBank* Reimported = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
        GetTransientPackage(), NAME_None, RF_Transient, nullptr, TEXT("bnk"), EditedCursor,
        Edited.GetData() + Edited.Num(), GWarn));
    if (!TestNotNull(TEXT("Reimport edited fruit fixture"), Reimported)) return false;
    const FKyaActorPlacement& EditedActor = Reimported->SceneEnvironment.Actors[0];
    TestEqual(TEXT("Edited fruit weight round trips"), EditedActor.FruitWeightB, 7.5f);
    TestEqual(TEXT("Edited fruit life round trips"), EditedActor.FruitLifeMax, int64(4000000000ll));
    TestEqual(TEXT("Edited fruit field 360 round trips"), EditedActor.FruitField360, 91);
    TestEqual(TEXT("Edited fruit switch target round trips"),
        EditedActor.FruitSwitch.Targets[0].TargetRef, 92);
    TestEqual(TEXT("Edited fruit squashed round trips"),
        EditedActor.Behaviours[1].FruitSquashedValue, 93.5f);
    TestEqual(TEXT("Edited fruit inventory money round trips"),
        EditedActor.Behaviours[2].FruitInventoryMoneyCost, 94);
    TestEqual(TEXT("Edited fruit inventory initial round trips"),
        EditedActor.Behaviours[2].FruitInventoryInitialBehaviour, 4);
    TestEqual(TEXT("Edited fruit bonus initial round trips"),
        EditedActor.Behaviours[3].FruitBonusInitialBehaviour, 95);

    Actor.FruitLifeMax = -1;
    TArray<uint8> RejectedLife;
    FMemoryWriter RejectedLifeWriter(RejectedLife);
    TestFalse(TEXT("Exporter rejects an out-of-range fruit life maximum"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), RejectedLifeWriter, GWarn));
    Actor.FruitLifeMax = 4000000000ll;

    Actor.FruitSwitch.Targets.AddDefaulted();
    TArray<uint8> Rejected;
    FMemoryWriter RejectedWriter(Rejected);
    TestFalse(TEXT("Exporter rejects a changed fruit switch count"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), RejectedWriter, GWarn));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKyaNativFixtureRoundTripTest, "Kya.Bank.NativFixtureRoundTrip",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKyaNativFixtureRoundTripTest::RunTest(const FString& Parameters)
{
    FKyaSmallBankBuilder Builder;
    const TArray<uint8> Input = Builder.Build(0x0a);
    UKyaLevelBankFactory* Factory = NewObject<UKyaLevelBankFactory>();
    const uint8* Cursor = Input.GetData();
    UKyaLevelBank* Bank = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
        GetTransientPackage(), NAME_None, RF_Transient, nullptr, TEXT("bnk"), Cursor,
        Input.GetData() + Input.Num(), GWarn));
    if (!TestNotNull(TEXT("Import Nativ fixture"), Bank)) return false;
    if (!TestEqual(TEXT("Nativ fixture actor count"), Bank->SceneEnvironment.Actors.Num(), 1)) return false;
    FKyaActorPlacement& Actor = Bank->SceneEnvironment.Actors[0];
    TestEqual(TEXT("Nativ class id"), Actor.ClassId, int64(0x0a));
    TestEqual(TEXT("Nativ actor name"), Actor.Name, FString(TEXT("NATIV")));
    TestTrue(TEXT("Nativ class payload parses"), Actor.bHasNativData);
    TestEqual(TEXT("Nativ weight"), Actor.NativWeightB, 1.25f);
    TestEqual(TEXT("Nativ life maximum is exact"), Actor.NativLifeMax, int64(16777217));
    TestEqual(TEXT("Nativ gravity"), Actor.NativGravityScale, 2.0f);
    TestEqual(TEXT("Nativ field 350"), Actor.NativField350, 11);
    TestEqual(TEXT("Nativ field 354"), Actor.NativField354, 12);
    TestEqual(TEXT("Nativ field 358"), Actor.NativField358, 13);
    TestEqual(TEXT("Nativ move speed no held"), Actor.NativMoveSpeedNoHeld, 1.5f);
    TestEqual(TEXT("Nativ move speed held"), Actor.NativMoveSpeedHeld, 2.5f);
    TestEqual(TEXT("Nativ field 370"), Actor.NativField370, 14);
    TestEqual(TEXT("Nativ field 368"), Actor.NativField368, 15);
    TestEqual(TEXT("Nativ field 364"), Actor.NativField364, 16);
    TestEqual(TEXT("Nativ field 378"), Actor.NativField378, 17);
    TestEqual(TEXT("Nativ field 37c"), Actor.NativField37C, 18);
    TestEqual(TEXT("Nativ field 380"), Actor.NativField380, 19);
    TestEqual(TEXT("Nativ fixture leaves no raw class data"), Actor.ClassData.Num(), 0);
    if (!TestEqual(TEXT("Nativ behaviour count"), Actor.Behaviours.Num(), 2)) return false;
    const FKyaActorBehaviour& Exorcism = Actor.Behaviours[0];
    const FKyaActorBehaviour& TakePut = Actor.Behaviours[1];
    TestEqual(TEXT("Nativ exorcism id"), Exorcism.Id, 4);
    TestEqual(TEXT("Nativ exorcism payload is empty"), Exorcism.Data.Num(), 0);
    TestEqual(TEXT("Nativ take-put id"), TakePut.Id, 6);
    TestTrue(TEXT("Nativ take-put is typed"), TakePut.bHasTypedNativBehaviour);
    if (!TestEqual(TEXT("Nativ trajectory count"), TakePut.NativTrajectories.Num(), 1)) return false;
    TestEqual(TEXT("Nativ trajectory field 0"), TakePut.NativTrajectories[0].Field0, 3.5f);
    TestEqual(TEXT("Nativ trajectory path"), TakePut.NativTrajectories[0].PathFollowIndex, 21);
    TestEqual(TEXT("Nativ trajectory field 14"), TakePut.NativTrajectories[0].Field14, 22);
    TestEqual(TEXT("Nativ trajectory field 18"), TakePut.NativTrajectories[0].Field18, 23.5f);

    UKyaLevelBankExporter* Exporter = NewObject<UKyaLevelBankExporter>();
    TArray<uint8> Plain;
    FMemoryWriter PlainWriter(Plain);
    if (!TestTrue(TEXT("Export unedited Nativ fixture"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), PlainWriter, GWarn))) return false;
    TestEqual(TEXT("Unedited Nativ export keeps the file size"), Plain.Num(), Input.Num());
    TestTrue(TEXT("Unedited Nativ export preserves every byte"), Plain == Input);

    Actor.NativWeightB = 7.5f;
    Actor.NativLifeMax = 4000000000ll;
    Actor.NativField350 = 91;
    Actor.NativMoveSpeedHeld = 92.5f;
    Actor.Behaviours[1].NativTrajectories[0].PathFollowIndex = 93;
    Actor.Behaviours[1].NativTrajectories[0].Field18 = 94.5f;
    TArray<uint8> Edited;
    FMemoryWriter EditedWriter(Edited);
    if (!TestTrue(TEXT("Export edited Nativ fixture"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), EditedWriter, GWarn))) return false;
    TestEqual(TEXT("Edited Nativ export keeps the file size"), Edited.Num(), Input.Num());
    const uint8* EditedCursor = Edited.GetData();
    UKyaLevelBank* Reimported = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
        GetTransientPackage(), NAME_None, RF_Transient, nullptr, TEXT("bnk"), EditedCursor,
        Edited.GetData() + Edited.Num(), GWarn));
    if (!TestNotNull(TEXT("Reimport edited Nativ fixture"), Reimported)) return false;
    const FKyaActorPlacement& EditedActor = Reimported->SceneEnvironment.Actors[0];
    TestEqual(TEXT("Edited Nativ weight round trips"), EditedActor.NativWeightB, 7.5f);
    TestEqual(TEXT("Edited Nativ life round trips"), EditedActor.NativLifeMax, int64(4000000000ll));
    TestEqual(TEXT("Edited Nativ field 350 round trips"), EditedActor.NativField350, 91);
    TestEqual(TEXT("Edited Nativ move speed round trips"), EditedActor.NativMoveSpeedHeld, 92.5f);
    TestEqual(TEXT("Edited Nativ trajectory path round trips"),
        EditedActor.Behaviours[1].NativTrajectories[0].PathFollowIndex, 93);
    TestEqual(TEXT("Edited Nativ trajectory field round trips"),
        EditedActor.Behaviours[1].NativTrajectories[0].Field18, 94.5f);

    Actor.NativLifeMax = -1;
    TArray<uint8> RejectedLife;
    FMemoryWriter RejectedLifeWriter(RejectedLife);
    TestFalse(TEXT("Exporter rejects an out-of-range Nativ life maximum"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), RejectedLifeWriter, GWarn));
    Actor.NativLifeMax = 4000000000ll;

    Actor.Behaviours[1].NativTrajectories.AddDefaulted();
    TArray<uint8> Rejected;
    FMemoryWriter RejectedWriter(Rejected);
    TestFalse(TEXT("Exporter rejects a changed Nativ trajectory count"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), RejectedWriter, GWarn));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKyaWolfenFixtureRoundTripTest, "Kya.Bank.WolfenFixtureRoundTrip",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKyaWolfenFixtureRoundTripTest::RunTest(const FString& Parameters)
{
    FKyaSmallBankBuilder Builder;
    const TArray<uint8> Input = Builder.Build(0x09);
    UKyaLevelBankFactory* Factory = NewObject<UKyaLevelBankFactory>();
    const uint8* Cursor = Input.GetData();
    UKyaLevelBank* Bank = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
        GetTransientPackage(), NAME_None, RF_Transient, nullptr, TEXT("bnk"), Cursor,
        Input.GetData() + Input.Num(), GWarn));
    if (!TestNotNull(TEXT("Import Wolfen fixture"), Bank)) return false;
    if (!TestEqual(TEXT("Wolfen fixture actor count"), Bank->SceneEnvironment.Actors.Num(), 1)) return false;
    FKyaActorPlacement& Actor = Bank->SceneEnvironment.Actors[0];
    TestEqual(TEXT("Wolfen class id"), Actor.ClassId, int64(0x09));
    TestEqual(TEXT("Wolfen actor name"), Actor.Name, FString(TEXT("WOLFEN")));
    TestTrue(TEXT("Wolfen class payload parses"), Actor.bHasWolfenData);
    TestEqual(TEXT("Wolfen fighter weight"), Actor.FighterWeightB, 1.25f);
    TestEqual(TEXT("Wolfen fighter life is exact"), Actor.FighterLifeMax, int64(16777217));
    TestEqual(TEXT("Wolfen vision range"), Actor.WolfenVisionRange, 30.0f);
    TestEqual(TEXT("Wolfen vision fov"), Actor.WolfenVisionFov, 90.0f);
    TestEqual(TEXT("Wolfen vision flags"), Actor.WolfenVisionFlags, 0x1234);
    TestEqual(TEXT("Wolfen start sector"), Actor.WolfenStartSectorId, 1);
    TestEqual(TEXT("Wolfen hearing range"), Actor.WolfenHearingRangeSquared, 100.0f);
    TestEqual(TEXT("Wolfen walk speed"), Actor.WolfenWalkSpeed, 4.0f);
    TestEqual(TEXT("Wolfen default run speed"), Actor.WolfenDefaultRunSpeed, 7.0f);
    TestEqual(TEXT("Wolfen magic required"), Actor.WolfenMagicRequired, 3);
    TestEqual(TEXT("Wolfen field d30"), Actor.WolfenFieldD30, 4);
    TestEqual(TEXT("Wolfen field b90"), Actor.WolfenFieldB90, 1.1f);
    TestEqual(TEXT("Wolfen field b98"), Actor.WolfenFieldB98, 3.3f);
    const FKyaWolfenFightParam& Fight = Actor.WolfenFightParamData;
    if (!TestEqual(TEXT("Wolfen capability 0 field count"), Fight.Capability0Fields.Num(), 5)) return false;
    TestEqual(TEXT("Wolfen capability 0 field 0"), Fight.Capability0Fields[0], 1);
    TestEqual(TEXT("Wolfen capability 2 field 4"), Fight.Capability2Fields[4], 10);
    TestEqual(TEXT("Wolfen configuration count"), Fight.ConfigurationValues.Num(), 4);
    TestEqual(TEXT("Wolfen configuration field 0"), Fight.ConfigurationValues[0], 0.1f);
    TestEqual(TEXT("Wolfen capability count"), Fight.Capabilities.Num(), 3);
    TestEqual(TEXT("Wolfen combo list 1"), Fight.ComboList1.Num(), 1);
    TestEqual(TEXT("Wolfen combo list 1 value"), Fight.ComboList1[0], 11);
    TestEqual(TEXT("Wolfen combo list 2 value"), Fight.ComboList2[0], 12);
    TestEqual(TEXT("Wolfen field after lists"), Fight.FieldAfterLists, 13);
    TestEqual(TEXT("Wolfen tuning count"), Fight.TuningValues.Num(), 6);
    TestEqual(TEXT("Wolfen knowledge count"), Fight.Knowledge.Num(), 4);
    TestEqual(TEXT("Wolfen knowledge 0"), Fight.Knowledge[0], 15);
    TestEqual(TEXT("Wolfen fixture leaves no raw class data"), Actor.ClassData.Num(), 0);
    if (!TestEqual(TEXT("Wolfen behaviour count"), Actor.Behaviours.Num(), 3)) return false;
    const FKyaActorBehaviour& Track = Actor.Behaviours[0];
    const FKyaActorBehaviour& Dca = Actor.Behaviours[1];
    const FKyaActorBehaviour& Avoid = Actor.Behaviours[2];
    TestEqual(TEXT("Wolfen track id"), Track.Id, 15);
    TestTrue(TEXT("Wolfen track is typed"), Track.bHasTypedWolfenBehaviour);
    TestEqual(TEXT("Wolfen track field"), Track.WolfenBehaviour.TrackField, 2.5f);
    TestEqual(TEXT("Wolfen DCA id"), Dca.Id, 23);
    TestTrue(TEXT("Wolfen DCA is typed"), Dca.bHasTypedWolfenBehaviour);
    TestEqual(TEXT("Wolfen DCA actor"), Dca.WolfenBehaviour.DcaActorRef, 21);
    TestEqual(TEXT("Wolfen DCA cooldown"), Dca.WolfenBehaviour.DcaShootCooldown, 3.5f);
    TestEqual(TEXT("Wolfen avoid id"), Avoid.Id, 24);
    TestTrue(TEXT("Wolfen avoid is typed"), Avoid.bHasTypedWolfenBehaviour);
    TestEqual(TEXT("Wolfen avoid field 80"), Avoid.WolfenBehaviour.AvoidField80, 4.5f);
    TestEqual(TEXT("Wolfen avoid field 84"), Avoid.WolfenBehaviour.AvoidField84, 5.5f);

    UKyaLevelBankExporter* Exporter = NewObject<UKyaLevelBankExporter>();
    TArray<uint8> Plain;
    FMemoryWriter PlainWriter(Plain);
    if (!TestTrue(TEXT("Export unedited Wolfen fixture"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), PlainWriter, GWarn))) return false;
    TestEqual(TEXT("Unedited Wolfen export keeps the file size"), Plain.Num(), Input.Num());
    TestTrue(TEXT("Unedited Wolfen export preserves every byte"), Plain == Input);

    Actor.WolfenVisionRange = 91.5f;
    Actor.FighterLifeMax = 4000000000ll;
    Actor.WolfenStartSectorId = 4;
    Actor.WolfenWalkSpeed = 92.5f;
    Actor.WolfenFightParamData.Capability0Fields[0] = 93;
    Actor.WolfenFightParamData.ComboList1[0] = 94;
    Actor.Behaviours[0].WolfenBehaviour.TrackField = 95.5f;
    Actor.Behaviours[1].WolfenBehaviour.DcaActorRef = 96;
    Actor.Behaviours[2].WolfenBehaviour.AvoidField84 = 97.5f;
    TArray<uint8> Edited;
    FMemoryWriter EditedWriter(Edited);
    if (!TestTrue(TEXT("Export edited Wolfen fixture"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), EditedWriter, GWarn))) return false;
    TestEqual(TEXT("Edited Wolfen export keeps the file size"), Edited.Num(), Input.Num());
    const uint8* EditedCursor = Edited.GetData();
    UKyaLevelBank* Reimported = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
        GetTransientPackage(), NAME_None, RF_Transient, nullptr, TEXT("bnk"), EditedCursor,
        Edited.GetData() + Edited.Num(), GWarn));
    if (!TestNotNull(TEXT("Reimport edited Wolfen fixture"), Reimported)) return false;
    const FKyaActorPlacement& EditedActor = Reimported->SceneEnvironment.Actors[0];
    TestEqual(TEXT("Edited Wolfen vision round trips"), EditedActor.WolfenVisionRange, 91.5f);
    TestEqual(TEXT("Edited Wolfen fighter life round trips"), EditedActor.FighterLifeMax,
        int64(4000000000ll));
    TestEqual(TEXT("Edited Wolfen start sector round trips"), EditedActor.WolfenStartSectorId, 4);
    TestEqual(TEXT("Edited Wolfen walk speed round trips"), EditedActor.WolfenWalkSpeed, 92.5f);
    TestEqual(TEXT("Edited Wolfen capability field round trips"),
        EditedActor.WolfenFightParamData.Capability0Fields[0], 93);
    TestEqual(TEXT("Edited Wolfen combo list round trips"),
        EditedActor.WolfenFightParamData.ComboList1[0], 94);
    TestEqual(TEXT("Edited Wolfen track round trips"),
        EditedActor.Behaviours[0].WolfenBehaviour.TrackField, 95.5f);
    TestEqual(TEXT("Edited Wolfen DCA actor round trips"),
        EditedActor.Behaviours[1].WolfenBehaviour.DcaActorRef, 96);
    TestEqual(TEXT("Edited Wolfen avoid field round trips"),
        EditedActor.Behaviours[2].WolfenBehaviour.AvoidField84, 97.5f);

    Actor.WolfenFightParamData.ComboList1.Add(99);
    TArray<uint8> Rejected;
    FMemoryWriter RejectedWriter(Rejected);
    TestFalse(TEXT("Exporter rejects a changed Wolfen combo list count"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), RejectedWriter, GWarn));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKyaCommanderFixtureRoundTripTest, "Kya.Bank.CommanderFixtureRoundTrip",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKyaCommanderFixtureRoundTripTest::RunTest(const FString& Parameters)
{
    FKyaSmallBankBuilder Builder;
    const TArray<uint8> Input = Builder.Build(0x13);
    UKyaLevelBankFactory* Factory = NewObject<UKyaLevelBankFactory>();
    const uint8* Cursor = Input.GetData();
    UKyaLevelBank* Bank = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
        GetTransientPackage(), NAME_None, RF_Transient, nullptr, TEXT("bnk"), Cursor,
        Input.GetData() + Input.Num(), GWarn));
    if (!TestNotNull(TEXT("Import Commander fixture"), Bank)) return false;
    if (!TestEqual(TEXT("Commander fixture actor count"), Bank->SceneEnvironment.Actors.Num(), 1)) return false;
    FKyaActorPlacement& Actor = Bank->SceneEnvironment.Actors[0];
    TestEqual(TEXT("Commander class id"), Actor.ClassId, int64(0x13));
    TestEqual(TEXT("Commander actor name"), Actor.Name, FString(TEXT("COMMANDER")));
    TestTrue(TEXT("Commander class payload parses"), Actor.bHasCommanderData);
    TestEqual(TEXT("Commander detect zone"), Actor.CommanderDetectAreaZoneId, 1);
    TestEqual(TEXT("Commander guard zone"), Actor.CommanderGuardAreaZoneId, 2);
    TestEqual(TEXT("Commander field 180"), Actor.CommanderField180, 3);
    TestEqual(TEXT("Commander new path id"), Actor.CommanderNewPathFindingId, 4);
    TestEqual(TEXT("Commander actor ref"), Actor.CommanderActorRef, 5);
    TestEqual(TEXT("Commander squad value"), Actor.CommanderSquadValue, 1.5f);
    TestEqual(TEXT("Commander squad field 4"), Actor.CommanderSquadField4, 6);
    TestEqual(TEXT("Commander squad field 8"), Actor.CommanderSquadField8, 7);
    if (!TestEqual(TEXT("Commander team count"), Actor.CommanderTeamEnemies.Num(), 1)) return false;
    TestEqual(TEXT("Commander team enemy"), Actor.CommanderTeamEnemies[0], 8);
    if (!TestEqual(TEXT("Commander component count"), Actor.CommanderSequence.Num(), 1)) return false;
    TestEqual(TEXT("Commander component field 0"), Actor.CommanderSequence[0].Field0, 9);
    TestEqual(TEXT("Commander component field 8"), Actor.CommanderSequence[0].Field8, 10);
    if (!TestEqual(TEXT("Commander camera value count"), Actor.CommanderCameraValues.Num(), 14)) return false;
    TestEqual(TEXT("Commander camera value 0"), Actor.CommanderCameraValues[0], 1.0f);
    TestEqual(TEXT("Commander camera value 13"), Actor.CommanderCameraValues[13], 14.0f);
    TestEqual(TEXT("Commander fixture leaves no raw class data"), Actor.ClassData.Num(), 0);
    if (!TestEqual(TEXT("Commander behaviour count"), Actor.Behaviours.Num(), 1)) return false;
    const FKyaActorBehaviour& Commander = Actor.Behaviours[0];
    TestEqual(TEXT("Commander behaviour id"), Commander.Id, 2);
    TestTrue(TEXT("Commander behaviour is typed"), Commander.bHasTypedCommanderBehaviour);
    if (!TestEqual(TEXT("Commander squad config count"), Commander.CommanderSquadConfig.Num(), 2)) return false;
    TestEqual(TEXT("Commander squad config 0"), Commander.CommanderSquadConfig[0], 7);
    TestEqual(TEXT("Commander squad config 1"), Commander.CommanderSquadConfig[1], 8);
    if (!TestEqual(TEXT("Commander switch target count"), Commander.CommanderSwitch.Targets.Num(), 1)) return false;
    TestEqual(TEXT("Commander switch target"), Commander.CommanderSwitch.Targets[0].TargetRef, 31);
    TestEqual(TEXT("Commander switch camera"), Commander.CommanderSwitch.Camera.CameraIndex, 41);

    UKyaLevelBankExporter* Exporter = NewObject<UKyaLevelBankExporter>();
    TArray<uint8> Plain;
    FMemoryWriter PlainWriter(Plain);
    if (!TestTrue(TEXT("Export unedited Commander fixture"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), PlainWriter, GWarn))) return false;
    TestEqual(TEXT("Unedited Commander export keeps the file size"), Plain.Num(), Input.Num());
    TestTrue(TEXT("Unedited Commander export preserves every byte"), Plain == Input);

    Actor.CommanderDetectAreaZoneId = 91;
    Actor.CommanderSquadValue = 92.5f;
    Actor.CommanderTeamEnemies[0] = 93;
    Actor.CommanderSequence[0].Field8 = 94;
    Actor.CommanderCameraValues[3] = 95.5f;
    Actor.Behaviours[0].CommanderSquadConfig[0] = 96;
    Actor.Behaviours[0].CommanderSwitch.Targets[0].TargetRef = 97;
    Actor.Behaviours[0].CommanderSwitch.Camera.CameraIndex = 98;
    TArray<uint8> Edited;
    FMemoryWriter EditedWriter(Edited);
    if (!TestTrue(TEXT("Export edited Commander fixture"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), EditedWriter, GWarn))) return false;
    TestEqual(TEXT("Edited Commander export keeps the file size"), Edited.Num(), Input.Num());
    const uint8* EditedCursor = Edited.GetData();
    UKyaLevelBank* Reimported = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
        GetTransientPackage(), NAME_None, RF_Transient, nullptr, TEXT("bnk"), EditedCursor,
        Edited.GetData() + Edited.Num(), GWarn));
    if (!TestNotNull(TEXT("Reimport edited Commander fixture"), Reimported)) return false;
    const FKyaActorPlacement& EditedActor = Reimported->SceneEnvironment.Actors[0];
    TestEqual(TEXT("Edited Commander detect zone round trips"), EditedActor.CommanderDetectAreaZoneId, 91);
    TestEqual(TEXT("Edited Commander squad value round trips"), EditedActor.CommanderSquadValue, 92.5f);
    TestEqual(TEXT("Edited Commander team round trips"), EditedActor.CommanderTeamEnemies[0], 93);
    TestEqual(TEXT("Edited Commander component round trips"), EditedActor.CommanderSequence[0].Field8, 94);
    TestEqual(TEXT("Edited Commander camera round trips"), EditedActor.CommanderCameraValues[3], 95.5f);
    TestEqual(TEXT("Edited Commander squad config round trips"),
        EditedActor.Behaviours[0].CommanderSquadConfig[0], 96);
    TestEqual(TEXT("Edited Commander switch target round trips"),
        EditedActor.Behaviours[0].CommanderSwitch.Targets[0].TargetRef, 97);
    TestEqual(TEXT("Edited Commander switch camera round trips"),
        EditedActor.Behaviours[0].CommanderSwitch.Camera.CameraIndex, 98);

    Actor.Behaviours[0].CommanderSwitch.Targets.AddDefaulted();
    TArray<uint8> Rejected;
    FMemoryWriter RejectedWriter(Rejected);
    TestFalse(TEXT("Exporter rejects a changed Commander switch count"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), RejectedWriter, GWarn));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKyaKimFixtureRoundTripTest, "Kya.Bank.KimFixtureRoundTrip",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKyaKimFixtureRoundTripTest::RunTest(const FString& Parameters)
{
    FKyaSmallBankBuilder Builder;
    const TArray<uint8> Input = Builder.Build(0x06);
    UKyaLevelBankFactory* Factory = NewObject<UKyaLevelBankFactory>();
    const uint8* Cursor = Input.GetData();
    UKyaLevelBank* Bank = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
        GetTransientPackage(), NAME_None, RF_Transient, nullptr, TEXT("bnk"), Cursor,
        Input.GetData() + Input.Num(), GWarn));
    if (!TestNotNull(TEXT("Import Kim fixture"), Bank)) return false;
    if (!TestEqual(TEXT("Kim fixture actor count"), Bank->SceneEnvironment.Actors.Num(), 1)) return false;
    FKyaActorPlacement& Actor = Bank->SceneEnvironment.Actors[0];
    TestEqual(TEXT("Kim class id"), Actor.ClassId, int64(0x06));
    TestEqual(TEXT("Kim actor name"), Actor.Name, FString(TEXT("KIM")));
    TestTrue(TEXT("Kim class payload parses"), Actor.bHasKimData);
    TestEqual(TEXT("Kim fighter weight"), Actor.FighterWeightB, 1.25f);
    if (!TestEqual(TEXT("Kim bone key count"), Actor.KimBoneKeys.Num(), 10)) return false;
    TestEqual(TEXT("Kim bone key 0"), Actor.KimBoneKeys[0], 1);
    TestEqual(TEXT("Kim bone key 9"), Actor.KimBoneKeys[9], 10);
    TestEqual(TEXT("Kim climb zones"), Actor.KimClimbZones.Num(), 0);
    TestEqual(TEXT("Kim tutorial zones"), Actor.KimTutorialZones.Num(), 0);
    TestEqual(TEXT("Kim projectile zones"), Actor.KimProjectileZones.Num(), 0);
    TestEqual(TEXT("Kim zone pairs"), Actor.KimBoomyZonePairs.Num(), 0);
    if (!TestEqual(TEXT("Kim zone stream"), Actor.KimZoneStream.Num(), 1)) return false;
    TestEqual(TEXT("Kim zone stream 0"), Actor.KimZoneStream[0], 11);
    TestEqual(TEXT("Kim material"), Actor.KimMaterialId, 12);
    TestEqual(TEXT("Kim fx type"), Actor.KimFxType, 13);
    TestEqual(TEXT("Kim joke radius"), Actor.KimJokeWarnRadius, 1.5f);
    TestEqual(TEXT("Kim cameras"), Actor.KimCameras.Num(), 0);
    TestEqual(TEXT("Kim field 15d4"), Actor.KimField15D4, 14);
    TestEqual(TEXT("Kim fx init"), Actor.KimFxInitType, 15);
    if (!TestEqual(TEXT("Kim fx handle count"), Actor.KimFxHandleTypes.Num(), 3)) return false;
    TestEqual(TEXT("Kim fx handle 2"), Actor.KimFxHandleTypes[2], 18);
    TestEqual(TEXT("Kim fixture leaves no raw class data"), Actor.ClassData.Num(), 0);
    if (!TestEqual(TEXT("Kim behaviour count"), Actor.Behaviours.Num(), 1)) return false;
    TestEqual(TEXT("Kim behaviour id"), Actor.Behaviours[0].Id, 3);
    TestEqual(TEXT("Kim behaviour is empty"), Actor.Behaviours[0].Data.Num(), 0);

    UKyaLevelBankExporter* Exporter = NewObject<UKyaLevelBankExporter>();
    TArray<uint8> Plain;
    FMemoryWriter PlainWriter(Plain);
    if (!TestTrue(TEXT("Export unedited Kim fixture"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), PlainWriter, GWarn))) return false;
    TestEqual(TEXT("Unedited Kim export keeps the file size"), Plain.Num(), Input.Num());
    TestTrue(TEXT("Unedited Kim export preserves every byte"), Plain == Input);

    Actor.FighterWeightB = 91.5f;
    Actor.KimBoneKeys[0] = 92;
    Actor.KimZoneStream[0] = 93;
    Actor.KimMaterialId = 94;
    Actor.KimFxType = 95;
    Actor.KimJokeWarnRadius = 96.5f;
    Actor.KimField15D4 = 97;
    Actor.KimFxHandleTypes[1] = 98;
    TArray<uint8> Edited;
    FMemoryWriter EditedWriter(Edited);
    if (!TestTrue(TEXT("Export edited Kim fixture"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), EditedWriter, GWarn))) return false;
    TestEqual(TEXT("Edited Kim export keeps the file size"), Edited.Num(), Input.Num());
    const uint8* EditedCursor = Edited.GetData();
    UKyaLevelBank* Reimported = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
        GetTransientPackage(), NAME_None, RF_Transient, nullptr, TEXT("bnk"), EditedCursor,
        Edited.GetData() + Edited.Num(), GWarn));
    if (!TestNotNull(TEXT("Reimport edited Kim fixture"), Reimported)) return false;
    const FKyaActorPlacement& EditedActor = Reimported->SceneEnvironment.Actors[0];
    TestEqual(TEXT("Edited Kim weight round trips"), EditedActor.FighterWeightB, 91.5f);
    TestEqual(TEXT("Edited Kim bone round trips"), EditedActor.KimBoneKeys[0], 92);
    TestEqual(TEXT("Edited Kim zone stream round trips"), EditedActor.KimZoneStream[0], 93);
    TestEqual(TEXT("Edited Kim material round trips"), EditedActor.KimMaterialId, 94);
    TestEqual(TEXT("Edited Kim fx type round trips"), EditedActor.KimFxType, 95);
    TestEqual(TEXT("Edited Kim joke radius round trips"), EditedActor.KimJokeWarnRadius, 96.5f);
    TestEqual(TEXT("Edited Kim field round trips"), EditedActor.KimField15D4, 97);
    TestEqual(TEXT("Edited Kim fx handle round trips"), EditedActor.KimFxHandleTypes[1], 98);

    Actor.KimZoneStream.Add(99);
    TArray<uint8> Rejected;
    FMemoryWriter RejectedWriter(Rejected);
    TestFalse(TEXT("Exporter rejects a changed Kim zone-stream count"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), RejectedWriter, GWarn));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKyaBoomyFixtureRoundTripTest, "Kya.Bank.BoomyFixtureRoundTrip",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKyaBoomyFixtureRoundTripTest::RunTest(const FString& Parameters)
{
    FKyaSmallBankBuilder Builder;
    const TArray<uint8> Input = Builder.Build(0x05);
    UKyaLevelBankFactory* Factory = NewObject<UKyaLevelBankFactory>();
    const uint8* Cursor = Input.GetData();
    UKyaLevelBank* Bank = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
        GetTransientPackage(), NAME_None, RF_Transient, nullptr, TEXT("bnk"), Cursor,
        Input.GetData() + Input.Num(), GWarn));
    if (!TestNotNull(TEXT("Import Boomy fixture"), Bank)) return false;
    if (!TestEqual(TEXT("Boomy fixture actor count"), Bank->SceneEnvironment.Actors.Num(), 1)) return false;
    FKyaActorPlacement& Actor = Bank->SceneEnvironment.Actors[0];
    TestEqual(TEXT("Boomy class id"), Actor.ClassId, int64(0x05));
    TestEqual(TEXT("Boomy actor name"), Actor.Name, FString(TEXT("BOOMY")));
    TestTrue(TEXT("Boomy class payload parses"), Actor.bHasBoomyData);
    TestEqual(TEXT("Boomy weight"), Actor.BoomyWeight, 1.25f);
    TestEqual(TEXT("Boomy vision range"), Actor.BoomyVisionRange, 30.0f);
    TestEqual(TEXT("Boomy vision angle"), Actor.BoomyVisionAngle, 90.0f);
    TestEqual(TEXT("Boomy vision field 38"), Actor.BoomyVisionField38, 10.0f);
    TestEqual(TEXT("Boomy vision field 3c"), Actor.BoomyVisionField3C, 5.0f);
    TestEqual(TEXT("Boomy vision flags"), Actor.BoomyVisionFlags, 1);
    TestEqual(TEXT("Boomy sound 2b8"), Actor.BoomySound2B8, 11);
    TestEqual(TEXT("Boomy sound 2bc"), Actor.BoomySound2BC, 12);
    TestEqual(TEXT("Boomy field 744"), Actor.BoomyField744, 2.5f);
    TestEqual(TEXT("Boomy tail width axis"), Actor.BoomyTailWidthAxis, 1);
    TestEqual(TEXT("Boomy field 740"), Actor.BoomyField740, 3.5f);
    TestEqual(TEXT("Boomy tail segments"), Actor.BoomyTailSegments, 4);
    TestEqual(TEXT("Boomy particle 3d0"), Actor.BoomyParticle3D0, 13);
    TestEqual(TEXT("Boomy particle 3d4"), Actor.BoomyParticle3D4, 14);
    TestEqual(TEXT("Boomy particle 3dc"), Actor.BoomyParticle3DC, 15);
    TestEqual(TEXT("Boomy field 3cc"), Actor.BoomyField3CC, 16);
    TestEqual(TEXT("Boomy texture"), Actor.BoomyTextureIndex, 17);
    TestEqual(TEXT("Boomy mesh"), Actor.BoomyMeshIndex, 18);
    const FKyaActorCamera& Camera = Actor.BoomyCamera;
    TestEqual(TEXT("Boomy camera fov"), Camera.Fov, 60.0f);
    TestEqual(TEXT("Boomy camera position x"), float(Camera.Position.X), 1.0f);
    TestEqual(TEXT("Boomy camera look at z"), float(Camera.LookAt.Z), 6.0f);
    TestEqual(TEXT("Boomy camera up y"), float(Camera.Up.Y), 1.0f);
    TestEqual(TEXT("Boomy camera sector"), Camera.SectorId, 1);
    TestEqual(TEXT("Boomy camera field 08"), Camera.Field08, 2);
    TestEqual(TEXT("Boomy camera flags"), Camera.Flags, 3);
    TestEqual(TEXT("Boomy camera target offset y"), float(Camera.TargetOffset.Y), 0.5f);
    TestTrue(TEXT("Boomy camera has switch"), Camera.bHasSwitch);
    TestEqual(TEXT("Boomy camera switch actor"), Camera.SwitchActor, 21);
    if (!TestEqual(TEXT("Boomy camera switch list A"), Camera.SwitchActorsA.Num(), 1)) return false;
    TestEqual(TEXT("Boomy camera switch entry"), Camera.SwitchActorsA[0], 22);
    TestEqual(TEXT("Boomy camera switch list B"), Camera.SwitchActorsB.Num(), 0);
    TestEqual(TEXT("Boomy camera switch value A"), Camera.SwitchValueA, 1.5f);
    TestEqual(TEXT("Boomy camera switch mode A"), Camera.SwitchModeA, 2);
    TestEqual(TEXT("Boomy camera switch value B"), Camera.SwitchValueB, 3.5f);
    TestEqual(TEXT("Boomy camera switch mode B"), Camera.SwitchModeB, 4);
    TestEqual(TEXT("Boomy camera switch value C"), Camera.SwitchValueC, 5.5f);
    if (!TestEqual(TEXT("Boomy camera condition count"), Camera.Conditions.Num(), 1)) return false;
    TestEqual(TEXT("Boomy camera condition combiner"), Camera.Conditions[0].Combiner, 1);
    TestEqual(TEXT("Boomy camera condition value"), Camera.Conditions[0].Value, 4.5f);
    TestEqual(TEXT("Boomy camera config flags"), Camera.ConfigFlags, 5);
    TestEqual(TEXT("Boomy camera config actor B"), Camera.ConfigActorRefB, 7);
    TestEqual(TEXT("Boomy camera config angle"), Camera.ConfigAngle, 8.5f);
    if (!TestEqual(TEXT("Boomy camera optional count"), Camera.ConfigOptionalValues.Num(), 5)) return false;
    TestEqual(TEXT("Boomy camera optional 4"), Camera.ConfigOptionalValues[4], 1.5f);
    TestEqual(TEXT("Boomy camera pitch z"), float(Camera.PitchAngles.Z), 3.0f);
    TestEqual(TEXT("Boomy camera distances z"), float(Camera.Distances.Z), 9.0f);
    if (!TestEqual(TEXT("Boomy camera extra count"), Camera.ExtraValues.Num(), 12)) return false;
    TestEqual(TEXT("Boomy camera extra 11"), Camera.ExtraValues[11], 12.0f);
    if (!TestEqual(TEXT("Boomy type count"), Actor.BoomyTypes.Num(), 1)) return false;
    TestEqual(TEXT("Boomy type field 4"), Actor.BoomyTypes[0].Field4, 2.5f);
    TestEqual(TEXT("Boomy type vision range"), Actor.BoomyTypes[0].VisionRange, 3.5f);
    TestEqual(TEXT("Boomy type hit damage"), Actor.BoomyTypes[0].HitDamage, 19);
    TestEqual(TEXT("Boomy type fx colour"), Actor.BoomyTypes[0].FxColorA, 20);
    TestEqual(TEXT("Boomy fixture leaves no raw class data"), Actor.ClassData.Num(), 0);
    if (!TestEqual(TEXT("Boomy behaviour count"), Actor.Behaviours.Num(), 1)) return false;
    TestEqual(TEXT("Boomy behaviour id"), Actor.Behaviours[0].Id, 2);
    TestEqual(TEXT("Boomy behaviour is empty"), Actor.Behaviours[0].Data.Num(), 0);

    UKyaLevelBankExporter* Exporter = NewObject<UKyaLevelBankExporter>();
    TArray<uint8> Plain;
    FMemoryWriter PlainWriter(Plain);
    if (!TestTrue(TEXT("Export unedited Boomy fixture"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), PlainWriter, GWarn))) return false;
    TestEqual(TEXT("Unedited Boomy export keeps the file size"), Plain.Num(), Input.Num());
    TestTrue(TEXT("Unedited Boomy export preserves every byte"), Plain == Input);

    Actor.BoomyWeight = 91.5f;
    Actor.BoomyVisionRange = 92.5f;
    Actor.BoomySound2B8 = 93;
    Actor.BoomyCamera.Fov = 94.5f;
    Actor.BoomyCamera.SwitchActorsA[0] = 95;
    Actor.BoomyCamera.ConfigOptionalValues[2] = 96.5f;
    Actor.BoomyTypes[0].HitDamage = 97;
    Actor.BoomyCamera.Conditions[0].Type = 98;
    TArray<uint8> Edited;
    FMemoryWriter EditedWriter(Edited);
    if (!TestTrue(TEXT("Export edited Boomy fixture"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), EditedWriter, GWarn))) return false;
    TestEqual(TEXT("Edited Boomy export keeps the file size"), Edited.Num(), Input.Num());
    const uint8* EditedCursor = Edited.GetData();
    UKyaLevelBank* Reimported = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
        GetTransientPackage(), NAME_None, RF_Transient, nullptr, TEXT("bnk"), EditedCursor,
        Edited.GetData() + Edited.Num(), GWarn));
    if (!TestNotNull(TEXT("Reimport edited Boomy fixture"), Reimported)) return false;
    const FKyaActorPlacement& EditedActor = Reimported->SceneEnvironment.Actors[0];
    TestEqual(TEXT("Edited Boomy weight round trips"), EditedActor.BoomyWeight, 91.5f);
    TestEqual(TEXT("Edited Boomy vision round trips"), EditedActor.BoomyVisionRange, 92.5f);
    TestEqual(TEXT("Edited Boomy sound round trips"), EditedActor.BoomySound2B8, 93);
    TestEqual(TEXT("Edited Boomy camera fov round trips"), EditedActor.BoomyCamera.Fov, 94.5f);
    TestEqual(TEXT("Edited Boomy switch entry round trips"),
        EditedActor.BoomyCamera.SwitchActorsA[0], 95);
    TestEqual(TEXT("Edited Boomy optional value round trips"),
        EditedActor.BoomyCamera.ConfigOptionalValues[2], 96.5f);
    TestEqual(TEXT("Edited Boomy type round trips"), EditedActor.BoomyTypes[0].HitDamage, 97);
    TestEqual(TEXT("Edited Boomy condition round trips"),
        EditedActor.BoomyCamera.Conditions[0].Type, 98);

    Actor.BoomyTypes.AddDefaulted();
    TArray<uint8> Rejected;
    FMemoryWriter RejectedWriter(Rejected);
    TestFalse(TEXT("Exporter rejects a changed Boomy type count"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), RejectedWriter, GWarn));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKyaAtonFixtureRoundTripTest, "Kya.Bank.AtonFixtureRoundTrip",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKyaAtonFixtureRoundTripTest::RunTest(const FString& Parameters)
{
    FKyaSmallBankBuilder Builder;
    const TArray<uint8> Input = Builder.Build(0x38);
    UKyaLevelBankFactory* Factory = NewObject<UKyaLevelBankFactory>();
    const uint8* Cursor = Input.GetData();
    UKyaLevelBank* Bank = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
        GetTransientPackage(), NAME_None, RF_Transient, nullptr, TEXT("bnk"), Cursor,
        Input.GetData() + Input.Num(), GWarn));
    if (!TestNotNull(TEXT("Import Aton fixture"), Bank)) return false;
    if (!TestEqual(TEXT("Aton fixture actor count"), Bank->SceneEnvironment.Actors.Num(), 1)) return false;
    FKyaActorPlacement& Actor = Bank->SceneEnvironment.Actors[0];
    TestEqual(TEXT("Aton class id"), Actor.ClassId, int64(0x38));
    TestEqual(TEXT("Aton actor name"), Actor.Name, FString(TEXT("ATON")));
    TestTrue(TEXT("Aton class payload parses"), Actor.bHasAtonData);
    TestEqual(TEXT("Aton weight"), Actor.AtonWeightB, 1.25f);
    TestEqual(TEXT("Aton life maximum is exact"), Actor.AtonLifeMax, int64(16777217));
    TestEqual(TEXT("Aton gravity"), Actor.AtonGravityScale, 2.0f);
    TestEqual(TEXT("Aton field 354"), Actor.AtonField354, 11);
    TestEqual(TEXT("Aton field 358"), Actor.AtonField358, 12);
    TestEqual(TEXT("Aton field 35c"), Actor.AtonField35C, 13);
    TestEqual(TEXT("Aton field 360"), Actor.AtonField360, 14);
    TestEqual(TEXT("Aton effect type"), Actor.AtonEffectType, 15);
    TestEqual(TEXT("Aton texture"), Actor.AtonTextureIndex, 16);
    TestEqual(TEXT("Aton mesh"), Actor.AtonMeshIndex, 17);
    TestEqual(TEXT("Aton trail material"), Actor.AtonTrailMaterialId, 18);
    TestEqual(TEXT("Aton path delay"), Actor.AtonPathDefaultDelay, 1.5f);
    if (!TestEqual(TEXT("Aton path count"), Actor.AtonPathIndices.Num(), 1)) return false;
    TestEqual(TEXT("Aton path index"), Actor.AtonPathIndices[0], 21);
    TestEqual(TEXT("Aton add-on field 18"), Actor.AtonAddOnField18, 7);
    if (!TestEqual(TEXT("Aton other sub-object count"), Actor.AtonOtherSubObjs.Num(), 1)) return false;
    TestEqual(TEXT("Aton other sub-object field 0"), Actor.AtonOtherSubObjs[0].Field0, 22);
    if (!TestEqual(TEXT("Aton other cinematic count"), Actor.AtonOtherSubObjs[0].CinematicIds.Num(), 1)) return false;
    TestEqual(TEXT("Aton other cinematic"), Actor.AtonOtherSubObjs[0].CinematicIds[0], 23);
    if (!TestEqual(TEXT("Aton sub-object count"), Actor.AtonSubObjs.Num(), 1)) return false;
    TestEqual(TEXT("Aton sub-object field 18"), Actor.AtonSubObjs[0].Field18, 24);
    TestEqual(TEXT("Aton sub-object field 0"), Actor.AtonSubObjs[0].Field0, 25);
    if (!TestEqual(TEXT("Aton sub-object cinematic count"), Actor.AtonSubObjs[0].CinematicIds.Num(), 1)) return false;
    TestEqual(TEXT("Aton sub-object cinematic"), Actor.AtonSubObjs[0].CinematicIds[0], 26);
    TestEqual(TEXT("Aton add-on field 24"), Actor.AtonAddOnField24, 3.5f);
    TestEqual(TEXT("Aton fixture leaves no raw class data"), Actor.ClassData.Num(), 0);
    if (!TestEqual(TEXT("Aton behaviour count"), Actor.Behaviours.Num(), 1)) return false;
    TestEqual(TEXT("Aton behaviour id"), Actor.Behaviours[0].Id, 3);
    TestEqual(TEXT("Aton behaviour is empty"), Actor.Behaviours[0].Data.Num(), 0);

    UKyaLevelBankExporter* Exporter = NewObject<UKyaLevelBankExporter>();
    TArray<uint8> Plain;
    FMemoryWriter PlainWriter(Plain);
    if (!TestTrue(TEXT("Export unedited Aton fixture"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), PlainWriter, GWarn))) return false;
    TestEqual(TEXT("Unedited Aton export keeps the file size"), Plain.Num(), Input.Num());
    TestTrue(TEXT("Unedited Aton export preserves every byte"), Plain == Input);

    Actor.AtonWeightB = 91.5f;
    Actor.AtonLifeMax = 4000000000ll;
    Actor.AtonEffectType = 92;
    Actor.AtonPathIndices[0] = 93;
    Actor.AtonOtherSubObjs[0].CinematicIds[0] = 94;
    Actor.AtonSubObjs[0].Field0 = 95;
    Actor.AtonSubObjs[0].CinematicIds[0] = 96;
    Actor.AtonAddOnField24 = 97.5f;
    TArray<uint8> Edited;
    FMemoryWriter EditedWriter(Edited);
    if (!TestTrue(TEXT("Export edited Aton fixture"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), EditedWriter, GWarn))) return false;
    TestEqual(TEXT("Edited Aton export keeps the file size"), Edited.Num(), Input.Num());
    const uint8* EditedCursor = Edited.GetData();
    UKyaLevelBank* Reimported = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
        GetTransientPackage(), NAME_None, RF_Transient, nullptr, TEXT("bnk"), EditedCursor,
        Edited.GetData() + Edited.Num(), GWarn));
    if (!TestNotNull(TEXT("Reimport edited Aton fixture"), Reimported)) return false;
    const FKyaActorPlacement& EditedActor = Reimported->SceneEnvironment.Actors[0];
    TestEqual(TEXT("Edited Aton weight round trips"), EditedActor.AtonWeightB, 91.5f);
    TestEqual(TEXT("Edited Aton life round trips"), EditedActor.AtonLifeMax, int64(4000000000ll));
    TestEqual(TEXT("Edited Aton effect round trips"), EditedActor.AtonEffectType, 92);
    TestEqual(TEXT("Edited Aton path round trips"), EditedActor.AtonPathIndices[0], 93);
    TestEqual(TEXT("Edited Aton other cinematic round trips"),
        EditedActor.AtonOtherSubObjs[0].CinematicIds[0], 94);
    TestEqual(TEXT("Edited Aton sub-object field round trips"), EditedActor.AtonSubObjs[0].Field0, 95);
    TestEqual(TEXT("Edited Aton sub-object cinematic round trips"),
        EditedActor.AtonSubObjs[0].CinematicIds[0], 96);
    TestEqual(TEXT("Edited Aton add-on field 24 round trips"), EditedActor.AtonAddOnField24, 97.5f);

    Actor.AtonLifeMax = -1;
    TArray<uint8> RejectedLife;
    FMemoryWriter RejectedLifeWriter(RejectedLife);
    TestFalse(TEXT("Exporter rejects an out-of-range Aton life maximum"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), RejectedLifeWriter, GWarn));
    Actor.AtonLifeMax = 4000000000ll;

    Actor.AtonSubObjs[0].CinematicIds.Add(99);
    TArray<uint8> Rejected;
    FMemoryWriter RejectedWriter(Rejected);
    TestFalse(TEXT("Exporter rejects a changed Aton cinematic count"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), RejectedWriter, GWarn));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKyaRopeFixtureRoundTripTest, "Kya.Bank.RopeFixtureRoundTrip",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKyaRopeFixtureRoundTripTest::RunTest(const FString& Parameters)
{
    FKyaSmallBankBuilder Builder;
    const TArray<uint8> Input = Builder.Build(0x0f);
    UKyaLevelBankFactory* Factory = NewObject<UKyaLevelBankFactory>();
    const uint8* Cursor = Input.GetData();
    UKyaLevelBank* Bank = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
        GetTransientPackage(), NAME_None, RF_Transient, nullptr, TEXT("bnk"), Cursor,
        Input.GetData() + Input.Num(), GWarn));
    if (!TestNotNull(TEXT("Import Rope fixture"), Bank)) return false;
    if (!TestEqual(TEXT("Rope fixture actor count"), Bank->SceneEnvironment.Actors.Num(), 1)) return false;
    FKyaActorPlacement& Actor = Bank->SceneEnvironment.Actors[0];
    TestEqual(TEXT("Rope class id"), Actor.ClassId, int64(0x0f));
    TestEqual(TEXT("Rope actor name"), Actor.Name, FString(TEXT("ROPE")));
    TestTrue(TEXT("Rope class payload parses"), Actor.bHasRopeData);
    TestEqual(TEXT("Rope path index"), Actor.RopePathIndex, 1);
    TestEqual(TEXT("Rope scale"), Actor.RopeScale, 1.5f);
    TestEqual(TEXT("Rope field 17c"), Actor.RopeField17C, 2.5f);
    TestEqual(TEXT("Rope material"), Actor.RopeMaterialId, 4);
    TestEqual(TEXT("Rope colour"), Actor.RopeColor, 0x11223344);
    TestEqual(TEXT("Rope sound"), Actor.RopeSoundRef, 5);
    TestEqual(TEXT("Rope field 194"), Actor.RopeField194, 6);
    TestEqual(TEXT("Rope actor ref A"), Actor.RopeActorRefA, 7);
    TestEqual(TEXT("Rope actor ref B"), Actor.RopeActorRefB, 8);
    if (!TestEqual(TEXT("Rope switch target count"), Actor.RopeSwitch.Targets.Num(), 1)) return false;
    TestEqual(TEXT("Rope switch target"), Actor.RopeSwitch.Targets[0].TargetRef, 21);
    TestEqual(TEXT("Rope switch camera"), Actor.RopeSwitch.Camera.CameraIndex, 31);
    TestEqual(TEXT("Rope flags"), Actor.RopeFlags, 2);
    TestEqual(TEXT("Rope fixture leaves no raw class data"), Actor.ClassData.Num(), 0);
    if (!TestEqual(TEXT("Rope behaviour count"), Actor.Behaviours.Num(), 1)) return false;
    TestEqual(TEXT("Rope behaviour id"), Actor.Behaviours[0].Id, 2);
    TestEqual(TEXT("Rope behaviour is empty"), Actor.Behaviours[0].Data.Num(), 0);

    UKyaLevelBankExporter* Exporter = NewObject<UKyaLevelBankExporter>();
    TArray<uint8> Plain;
    FMemoryWriter PlainWriter(Plain);
    if (!TestTrue(TEXT("Export unedited Rope fixture"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), PlainWriter, GWarn))) return false;
    TestEqual(TEXT("Unedited Rope export keeps the file size"), Plain.Num(), Input.Num());
    TestTrue(TEXT("Unedited Rope export preserves every byte"), Plain == Input);

    Actor.RopePathIndex = 91;
    Actor.RopeScale = 92.5f;
    Actor.RopeMaterialId = 93;
    Actor.RopeSoundRef = 94;
    Actor.RopeSwitch.Targets[0].TargetRef = 95;
    Actor.RopeSwitch.Camera.CameraIndex = 96;
    Actor.RopeFlags = 97;
    TArray<uint8> Edited;
    FMemoryWriter EditedWriter(Edited);
    if (!TestTrue(TEXT("Export edited Rope fixture"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), EditedWriter, GWarn))) return false;
    TestEqual(TEXT("Edited Rope export keeps the file size"), Edited.Num(), Input.Num());
    const uint8* EditedCursor = Edited.GetData();
    UKyaLevelBank* Reimported = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
        GetTransientPackage(), NAME_None, RF_Transient, nullptr, TEXT("bnk"), EditedCursor,
        Edited.GetData() + Edited.Num(), GWarn));
    if (!TestNotNull(TEXT("Reimport edited Rope fixture"), Reimported)) return false;
    const FKyaActorPlacement& EditedActor = Reimported->SceneEnvironment.Actors[0];
    TestEqual(TEXT("Edited Rope path round trips"), EditedActor.RopePathIndex, 91);
    TestEqual(TEXT("Edited Rope scale round trips"), EditedActor.RopeScale, 92.5f);
    TestEqual(TEXT("Edited Rope material round trips"), EditedActor.RopeMaterialId, 93);
    TestEqual(TEXT("Edited Rope sound round trips"), EditedActor.RopeSoundRef, 94);
    TestEqual(TEXT("Edited Rope switch target round trips"),
        EditedActor.RopeSwitch.Targets[0].TargetRef, 95);
    TestEqual(TEXT("Edited Rope switch camera round trips"),
        EditedActor.RopeSwitch.Camera.CameraIndex, 96);
    TestEqual(TEXT("Edited Rope flags round trips"), EditedActor.RopeFlags, 97);

    Actor.RopeSwitch.Targets.AddDefaulted();
    TArray<uint8> Rejected;
    FMemoryWriter RejectedWriter(Rejected);
    TestFalse(TEXT("Exporter rejects a changed Rope switch count"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), RejectedWriter, GWarn));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKyaStillerFixtureRoundTripTest, "Kya.Bank.StillerFixtureRoundTrip",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKyaStillerFixtureRoundTripTest::RunTest(const FString& Parameters)
{
    FKyaSmallBankBuilder Builder;
    const TArray<uint8> Input = Builder.Build(0x4c);
    UKyaLevelBankFactory* Factory = NewObject<UKyaLevelBankFactory>();
    const uint8* Cursor = Input.GetData();
    UKyaLevelBank* Bank = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
        GetTransientPackage(), NAME_None, RF_Transient, nullptr, TEXT("bnk"), Cursor,
        Input.GetData() + Input.Num(), GWarn));
    if (!TestNotNull(TEXT("Import Stiller fixture"), Bank)) return false;
    if (!TestEqual(TEXT("Stiller fixture actor count"), Bank->SceneEnvironment.Actors.Num(), 1)) return false;
    FKyaActorPlacement& Actor = Bank->SceneEnvironment.Actors[0];
    TestEqual(TEXT("Stiller class id"), Actor.ClassId, int64(0x4c));
    TestEqual(TEXT("Stiller actor name"), Actor.Name, FString(TEXT("STILLER")));
    TestTrue(TEXT("Stiller class payload parses"), Actor.bHasStillerData);
    TestEqual(TEXT("Stiller vision range"), Actor.StillerVisionRange, 30.0f);
    TestEqual(TEXT("Stiller vision half angle"), Actor.StillerVisionHalfAngle, 45.0f);
    TestEqual(TEXT("Stiller vision field 38"), Actor.StillerVisionField38, 10.0f);
    TestEqual(TEXT("Stiller vision field 3c"), Actor.StillerVisionField3C, 5.0f);
    TestEqual(TEXT("Stiller vision flags"), Actor.StillerVisionFlags, 1);
    TestEqual(TEXT("Stiller field 1d0"), Actor.StillerField1D0, 11);
    TestEqual(TEXT("Stiller field 1dc"), Actor.StillerField1DC, 14);
    TestEqual(TEXT("Stiller path index"), Actor.StillerPathIndex, 1);
    TestEqual(TEXT("Stiller field 1e0"), Actor.StillerField1E0, 2.5f);
    TestEqual(TEXT("Stiller life max"), Actor.StillerLifeMax, 3.5f);
    TestEqual(TEXT("Stiller field 1ec"), Actor.StillerField1EC, 4.5f);
    TestEqual(TEXT("Stiller field 1f4"), Actor.StillerField1F4, 6.5f);
    TestEqual(TEXT("Stiller field 1f8"), Actor.StillerField1F8, 15);
    if (!TestEqual(TEXT("Stiller actor count"), Actor.StillerActorRefs.Num(), 1)) return false;
    TestEqual(TEXT("Stiller actor ref"), Actor.StillerActorRefs[0], 16);
    TestEqual(TEXT("Stiller field 200"), Actor.StillerField200, 17);
    if (!TestEqual(TEXT("Stiller switch 208 count"), Actor.StillerSwitch208.Targets.Num(), 1)) return false;
    TestEqual(TEXT("Stiller switch 208 target"), Actor.StillerSwitch208.Targets[0].TargetRef, 21);
    TestEqual(TEXT("Stiller switch 208 camera"), Actor.StillerSwitch208.Camera.CameraIndex, 31);
    TestEqual(TEXT("Stiller switch 210 count"), Actor.StillerSwitch210.Targets.Num(), 0);
    TestEqual(TEXT("Stiller switch 210 camera"), Actor.StillerSwitch210.Camera.CameraIndex, 41);
    TestEqual(TEXT("Stiller field 214"), Actor.StillerField214, 18);
    TestEqual(TEXT("Stiller add-on money type"), Actor.StillerAddOn.MoneyType, 2);
    TestEqual(TEXT("Stiller add-on money count"), Actor.StillerAddOn.MoneyCount, 3);
    TestEqual(TEXT("Stiller add-on sub-object mode"), Actor.StillerAddOn.SubObjMode, 1);
    TestEqual(TEXT("Stiller add-on float 0"), Actor.StillerAddOn.SubObjField0, 0.25f);
    TestEqual(TEXT("Stiller add-on float 14"), Actor.StillerAddOn.SubObjField14, 1.5f);
    TestEqual(TEXT("Stiller fixture leaves no raw class data"), Actor.ClassData.Num(), 0);
    if (!TestEqual(TEXT("Stiller behaviour count"), Actor.Behaviours.Num(), 1)) return false;
    TestEqual(TEXT("Stiller behaviour id"), Actor.Behaviours[0].Id, 2);
    TestEqual(TEXT("Stiller behaviour is empty"), Actor.Behaviours[0].Data.Num(), 0);

    UKyaLevelBankExporter* Exporter = NewObject<UKyaLevelBankExporter>();
    TArray<uint8> Plain;
    FMemoryWriter PlainWriter(Plain);
    if (!TestTrue(TEXT("Export unedited Stiller fixture"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), PlainWriter, GWarn))) return false;
    TestEqual(TEXT("Unedited Stiller export keeps the file size"), Plain.Num(), Input.Num());
    TestTrue(TEXT("Unedited Stiller export preserves every byte"), Plain == Input);

    Actor.StillerVisionRange = 91.5f;
    Actor.StillerPathIndex = 92;
    Actor.StillerLifeMax = 93.5f;
    Actor.StillerField1F8 = 94;
    Actor.StillerActorRefs[0] = 95;
    Actor.StillerSwitch208.Targets[0].TargetRef = 96;
    Actor.StillerAddOn.MoneyCount = 97;
    Actor.StillerAddOn.SubObjFieldC = 98.5f;
    TArray<uint8> Edited;
    FMemoryWriter EditedWriter(Edited);
    if (!TestTrue(TEXT("Export edited Stiller fixture"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), EditedWriter, GWarn))) return false;
    TestEqual(TEXT("Edited Stiller export keeps the file size"), Edited.Num(), Input.Num());
    const uint8* EditedCursor = Edited.GetData();
    UKyaLevelBank* Reimported = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
        GetTransientPackage(), NAME_None, RF_Transient, nullptr, TEXT("bnk"), EditedCursor,
        Edited.GetData() + Edited.Num(), GWarn));
    if (!TestNotNull(TEXT("Reimport edited Stiller fixture"), Reimported)) return false;
    const FKyaActorPlacement& EditedActor = Reimported->SceneEnvironment.Actors[0];
    TestEqual(TEXT("Edited Stiller vision round trips"), EditedActor.StillerVisionRange, 91.5f);
    TestEqual(TEXT("Edited Stiller path round trips"), EditedActor.StillerPathIndex, 92);
    TestEqual(TEXT("Edited Stiller life round trips"), EditedActor.StillerLifeMax, 93.5f);
    TestEqual(TEXT("Edited Stiller field 1f8 round trips"), EditedActor.StillerField1F8, 94);
    TestEqual(TEXT("Edited Stiller actor ref round trips"), EditedActor.StillerActorRefs[0], 95);
    TestEqual(TEXT("Edited Stiller switch round trips"),
        EditedActor.StillerSwitch208.Targets[0].TargetRef, 96);
    TestEqual(TEXT("Edited Stiller money round trips"), EditedActor.StillerAddOn.MoneyCount, 97);
    TestEqual(TEXT("Edited Stiller add-on float round trips"),
        EditedActor.StillerAddOn.SubObjFieldC, 98.5f);

    Actor.StillerSwitch210.Targets.AddDefaulted();
    TArray<uint8> Rejected;
    FMemoryWriter RejectedWriter(Rejected);
    TestFalse(TEXT("Exporter rejects a changed Stiller switch count"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), RejectedWriter, GWarn));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKyaAmbreFixtureRoundTripTest, "Kya.Bank.AmbreFixtureRoundTrip",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKyaAmbreFixtureRoundTripTest::RunTest(const FString& Parameters)
{
    FKyaSmallBankBuilder Builder;
    const TArray<uint8> Input = Builder.Build(0x14);
    UKyaLevelBankFactory* Factory = NewObject<UKyaLevelBankFactory>();
    const uint8* Cursor = Input.GetData();
    UKyaLevelBank* Bank = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
        GetTransientPackage(), NAME_None, RF_Transient, nullptr, TEXT("bnk"), Cursor,
        Input.GetData() + Input.Num(), GWarn));
    if (!TestNotNull(TEXT("Import Ambre fixture"), Bank)) return false;
    if (!TestEqual(TEXT("Ambre fixture actor count"), Bank->SceneEnvironment.Actors.Num(), 1)) return false;
    FKyaActorPlacement& Actor = Bank->SceneEnvironment.Actors[0];
    TestEqual(TEXT("Ambre class id"), Actor.ClassId, int64(0x14));
    TestEqual(TEXT("Ambre actor name"), Actor.Name, FString(TEXT("AMBRE")));
    TestTrue(TEXT("Ambre class payload parses"), Actor.bHasAmbreData);
    TestEqual(TEXT("Ambre material"), Actor.AmbreMaterialId, 1);
    TestEqual(TEXT("Ambre effect type"), Actor.AmbreEffectType, 2);
    TestEqual(TEXT("Ambre texture"), Actor.AmbreTextureIndex, 3);
    TestEqual(TEXT("Ambre mesh"), Actor.AmbreMeshIndex, 4);
    TestEqual(TEXT("Ambre field 1d0"), Actor.AmbreField1D0, 5);
    if (!TestEqual(TEXT("Ambre switch target count"), Actor.AmbreSwitch.Targets.Num(), 1)) return false;
    TestEqual(TEXT("Ambre switch target"), Actor.AmbreSwitch.Targets[0].TargetRef, 21);
    TestEqual(TEXT("Ambre switch camera"), Actor.AmbreSwitch.Camera.CameraIndex, 31);
    TestEqual(TEXT("Ambre fixture leaves no raw class data"), Actor.ClassData.Num(), 0);
    if (!TestEqual(TEXT("Ambre behaviour count"), Actor.Behaviours.Num(), 1)) return false;
    TestEqual(TEXT("Ambre behaviour id"), Actor.Behaviours[0].Id, 2);
    TestEqual(TEXT("Ambre behaviour is empty"), Actor.Behaviours[0].Data.Num(), 0);

    UKyaLevelBankExporter* Exporter = NewObject<UKyaLevelBankExporter>();
    TArray<uint8> Plain;
    FMemoryWriter PlainWriter(Plain);
    if (!TestTrue(TEXT("Export unedited Ambre fixture"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), PlainWriter, GWarn))) return false;
    TestEqual(TEXT("Unedited Ambre export keeps the file size"), Plain.Num(), Input.Num());
    TestTrue(TEXT("Unedited Ambre export preserves every byte"), Plain == Input);

    Actor.AmbreMaterialId = 91;
    Actor.AmbreEffectType = 92;
    Actor.AmbreTextureIndex = 93;
    Actor.AmbreMeshIndex = 94;
    Actor.AmbreField1D0 = 95;
    Actor.AmbreSwitch.Targets[0].TargetRef = 96;
    Actor.AmbreSwitch.Camera.CameraIndex = 97;
    TArray<uint8> Edited;
    FMemoryWriter EditedWriter(Edited);
    if (!TestTrue(TEXT("Export edited Ambre fixture"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), EditedWriter, GWarn))) return false;
    TestEqual(TEXT("Edited Ambre export keeps the file size"), Edited.Num(), Input.Num());
    const uint8* EditedCursor = Edited.GetData();
    UKyaLevelBank* Reimported = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
        GetTransientPackage(), NAME_None, RF_Transient, nullptr, TEXT("bnk"), EditedCursor,
        Edited.GetData() + Edited.Num(), GWarn));
    if (!TestNotNull(TEXT("Reimport edited Ambre fixture"), Reimported)) return false;
    const FKyaActorPlacement& EditedActor = Reimported->SceneEnvironment.Actors[0];
    TestEqual(TEXT("Edited Ambre material round trips"), EditedActor.AmbreMaterialId, 91);
    TestEqual(TEXT("Edited Ambre effect round trips"), EditedActor.AmbreEffectType, 92);
    TestEqual(TEXT("Edited Ambre texture round trips"), EditedActor.AmbreTextureIndex, 93);
    TestEqual(TEXT("Edited Ambre mesh round trips"), EditedActor.AmbreMeshIndex, 94);
    TestEqual(TEXT("Edited Ambre field 1d0 round trips"), EditedActor.AmbreField1D0, 95);
    TestEqual(TEXT("Edited Ambre switch target round trips"),
        EditedActor.AmbreSwitch.Targets[0].TargetRef, 96);
    TestEqual(TEXT("Edited Ambre switch camera round trips"),
        EditedActor.AmbreSwitch.Camera.CameraIndex, 97);

    Actor.AmbreSwitch.Targets.AddDefaulted();
    TArray<uint8> Rejected;
    FMemoryWriter RejectedWriter(Rejected);
    TestFalse(TEXT("Exporter rejects a changed Ambre switch count"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), RejectedWriter, GWarn));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKyaFountainFixtureRoundTripTest, "Kya.Bank.FountainFixtureRoundTrip",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKyaFountainFixtureRoundTripTest::RunTest(const FString& Parameters)
{
    FKyaSmallBankBuilder Builder;
    const TArray<uint8> Input = Builder.Build(0x52);
    UKyaLevelBankFactory* Factory = NewObject<UKyaLevelBankFactory>();
    const uint8* Cursor = Input.GetData();
    UKyaLevelBank* Bank = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
        GetTransientPackage(), NAME_None, RF_Transient, nullptr, TEXT("bnk"), Cursor,
        Input.GetData() + Input.Num(), GWarn));
    if (!TestNotNull(TEXT("Import fountain fixture"), Bank)) return false;
    if (!TestEqual(TEXT("Fountain fixture actor count"), Bank->SceneEnvironment.Actors.Num(), 1)) return false;
    FKyaActorPlacement& Actor = Bank->SceneEnvironment.Actors[0];
    TestEqual(TEXT("Fountain class id"), Actor.ClassId, int64(0x52));
    TestEqual(TEXT("Fountain actor name"), Actor.Name, FString(TEXT("FOUNTAIN")));
    TestTrue(TEXT("Fountain class payload parses"), Actor.bHasBonusFountainData);
    TestEqual(TEXT("Fountain add-on field 0"), Actor.BonusFountainAddOn.Field0, 1);
    TestEqual(TEXT("Fountain money type"), Actor.BonusFountainAddOn.MoneyType, 2);
    TestEqual(TEXT("Fountain money count"), Actor.BonusFountainAddOn.MoneyCount, 3);
    TestEqual(TEXT("Fountain bonus count"), Actor.BonusFountainAddOn.BonusCount, 4);
    TestEqual(TEXT("Fountain field c"), Actor.BonusFountainAddOn.FieldC, 5);
    TestEqual(TEXT("Fountain sub-object mode"), Actor.BonusFountainAddOn.SubObjMode, 1);
    TestEqual(TEXT("Fountain sub-object float 0"), Actor.BonusFountainAddOn.SubObjField0, 0.25f);
    TestEqual(TEXT("Fountain sub-object float 14"), Actor.BonusFountainAddOn.SubObjField14, 1.5f);
    TestEqual(TEXT("Fountain fixture leaves no raw class data"), Actor.ClassData.Num(), 0);
    if (!TestEqual(TEXT("Fountain behaviour count"), Actor.Behaviours.Num(), 1)) return false;
    TestEqual(TEXT("Fountain behaviour id"), Actor.Behaviours[0].Id, 2);
    TestEqual(TEXT("Fountain behaviour is empty"), Actor.Behaviours[0].Data.Num(), 0);

    UKyaLevelBankExporter* Exporter = NewObject<UKyaLevelBankExporter>();
    TArray<uint8> Plain;
    FMemoryWriter PlainWriter(Plain);
    if (!TestTrue(TEXT("Export unedited fountain fixture"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), PlainWriter, GWarn))) return false;
    TestEqual(TEXT("Unedited fountain export keeps the file size"), Plain.Num(), Input.Num());
    TestTrue(TEXT("Unedited fountain export preserves every byte"), Plain == Input);

    Actor.BonusFountainAddOn.Field0 = 91;
    Actor.BonusFountainAddOn.MoneyType = 92;
    Actor.BonusFountainAddOn.MoneyCount = 93;
    Actor.BonusFountainAddOn.BonusCount = 94;
    Actor.BonusFountainAddOn.FieldC = 95;
    Actor.BonusFountainAddOn.SubObjField4 = 96.5f;
    Actor.BonusFountainAddOn.SubObjFieldC = 97.5f;
    TArray<uint8> Edited;
    FMemoryWriter EditedWriter(Edited);
    if (!TestTrue(TEXT("Export edited fountain fixture"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), EditedWriter, GWarn))) return false;
    TestEqual(TEXT("Edited fountain export keeps the file size"), Edited.Num(), Input.Num());
    const uint8* EditedCursor = Edited.GetData();
    UKyaLevelBank* Reimported = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
        GetTransientPackage(), NAME_None, RF_Transient, nullptr, TEXT("bnk"), EditedCursor,
        Edited.GetData() + Edited.Num(), GWarn));
    if (!TestNotNull(TEXT("Reimport edited fountain fixture"), Reimported)) return false;
    const FKyaActorPlacement& EditedActor = Reimported->SceneEnvironment.Actors[0];
    TestEqual(TEXT("Edited fountain field 0 round trips"), EditedActor.BonusFountainAddOn.Field0, 91);
    TestEqual(TEXT("Edited fountain money type round trips"), EditedActor.BonusFountainAddOn.MoneyType, 92);
    TestEqual(TEXT("Edited fountain money count round trips"), EditedActor.BonusFountainAddOn.MoneyCount, 93);
    TestEqual(TEXT("Edited fountain bonus count round trips"), EditedActor.BonusFountainAddOn.BonusCount, 94);
    TestEqual(TEXT("Edited fountain field c round trips"), EditedActor.BonusFountainAddOn.FieldC, 95);
    TestEqual(TEXT("Edited fountain sub-object float round trips"),
        EditedActor.BonusFountainAddOn.SubObjField4, 96.5f);
    TestEqual(TEXT("Edited fountain sub-object float c round trips"),
        EditedActor.BonusFountainAddOn.SubObjFieldC, 97.5f);

    Actor.BonusFountainAddOn.SubObjMode = 0;
    TArray<uint8> Rejected;
    FMemoryWriter RejectedWriter(Rejected);
    TestFalse(TEXT("Exporter rejects an out-of-range fountain sub-object mode"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), RejectedWriter, GWarn));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKyaCheckpointsFixtureRoundTripTest, "Kya.Bank.CheckpointsFixtureRoundTrip",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKyaCheckpointsFixtureRoundTripTest::RunTest(const FString& Parameters)
{
    FKyaSmallBankBuilder Builder;
    const TArray<uint8> Input = Builder.Build(0x1e);
    UKyaLevelBankFactory* Factory = NewObject<UKyaLevelBankFactory>();
    const uint8* Cursor = Input.GetData();
    UKyaLevelBank* Bank = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
        GetTransientPackage(), NAME_None, RF_Transient, nullptr, TEXT("bnk"), Cursor,
        Input.GetData() + Input.Num(), GWarn));
    if (!TestNotNull(TEXT("Import checkpoints fixture"), Bank)) return false;
    if (!TestEqual(TEXT("Checkpoints fixture actor count"), Bank->SceneEnvironment.Actors.Num(), 1)) return false;
    FKyaActorPlacement& Actor = Bank->SceneEnvironment.Actors[0];
    TestEqual(TEXT("Checkpoints class id"), Actor.ClassId, int64(0x1e));
    TestEqual(TEXT("Checkpoints actor name"), Actor.Name, FString(TEXT("CHECKPOINTS")));
    TestTrue(TEXT("Checkpoints class payload parses"), Actor.bHasTypedCheckpoints);
    if (!TestEqual(TEXT("Checkpoint count"), Actor.Checkpoints.Num(), 1)) return false;
    const FKyaCheckpoint& Checkpoint = Actor.Checkpoints[0];
    TestEqual(TEXT("Checkpoint zone ref"), Checkpoint.ZoneRef, 1);
    TestEqual(TEXT("Checkpoint waypoint A"), Checkpoint.WaypointA, 2);
    TestEqual(TEXT("Checkpoint sector"), Checkpoint.SectorId, 3);
    TestEqual(TEXT("Checkpoint waypoint B"), Checkpoint.WaypointB, 4);
    TestEqual(TEXT("Checkpoint flags"), Checkpoint.Flags, int64(5));
    if (!TestEqual(TEXT("Checkpoint actor-waypoint count"), Checkpoint.ActorWaypoints.Num(), 1)) return false;
    TestEqual(TEXT("Checkpoint actor ref"), Checkpoint.ActorWaypoints[0].ActorRef, 6);
    TestEqual(TEXT("Checkpoint waypoint ref"), Checkpoint.ActorWaypoints[0].WaypointRef, 7);
    TestEqual(TEXT("Checkpoints fixture leaves no raw class data"), Actor.ClassData.Num(), 0);
    if (!TestEqual(TEXT("Checkpoints behaviour count"), Actor.Behaviours.Num(), 1)) return false;
    TestEqual(TEXT("Checkpoints behaviour id"), Actor.Behaviours[0].Id, 2);
    TestEqual(TEXT("Checkpoints behaviour is empty"), Actor.Behaviours[0].Data.Num(), 0);

    UKyaLevelBankExporter* Exporter = NewObject<UKyaLevelBankExporter>();
    TArray<uint8> Plain;
    FMemoryWriter PlainWriter(Plain);
    if (!TestTrue(TEXT("Export unedited checkpoints fixture"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), PlainWriter, GWarn))) return false;
    TestEqual(TEXT("Unedited checkpoints export keeps the file size"), Plain.Num(), Input.Num());
    TestTrue(TEXT("Unedited checkpoints export preserves every byte"), Plain == Input);

    Actor.Checkpoints[0].ZoneRef = 91;
    Actor.Checkpoints[0].WaypointA = 92;
    Actor.Checkpoints[0].SectorId = 93;
    Actor.Checkpoints[0].WaypointB = 94;
    Actor.Checkpoints[0].Flags = 95;
    Actor.Checkpoints[0].ActorWaypoints[0].ActorRef = 96;
    Actor.Checkpoints[0].ActorWaypoints[0].WaypointRef = 97;
    TArray<uint8> Edited;
    FMemoryWriter EditedWriter(Edited);
    if (!TestTrue(TEXT("Export edited checkpoints fixture"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), EditedWriter, GWarn))) return false;
    TestEqual(TEXT("Edited checkpoints export keeps the file size"), Edited.Num(), Input.Num());
    const uint8* EditedCursor = Edited.GetData();
    UKyaLevelBank* Reimported = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
        GetTransientPackage(), NAME_None, RF_Transient, nullptr, TEXT("bnk"), EditedCursor,
        Edited.GetData() + Edited.Num(), GWarn));
    if (!TestNotNull(TEXT("Reimport edited checkpoints fixture"), Reimported)) return false;
    const FKyaCheckpoint& EditedCheckpoint = Reimported->SceneEnvironment.Actors[0].Checkpoints[0];
    TestEqual(TEXT("Edited checkpoint zone round trips"), EditedCheckpoint.ZoneRef, 91);
    TestEqual(TEXT("Edited checkpoint waypoint A round trips"), EditedCheckpoint.WaypointA, 92);
    TestEqual(TEXT("Edited checkpoint sector round trips"), EditedCheckpoint.SectorId, 93);
    TestEqual(TEXT("Edited checkpoint waypoint B round trips"), EditedCheckpoint.WaypointB, 94);
    TestEqual(TEXT("Edited checkpoint flags round trips"), EditedCheckpoint.Flags, int64(95));
    TestEqual(TEXT("Edited checkpoint actor ref round trips"),
        EditedCheckpoint.ActorWaypoints[0].ActorRef, 96);
    TestEqual(TEXT("Edited checkpoint waypoint ref round trips"),
        EditedCheckpoint.ActorWaypoints[0].WaypointRef, 97);

    Actor.Checkpoints[0].ActorWaypoints.AddDefaulted();
    TArray<uint8> Rejected;
    FMemoryWriter RejectedWriter(Rejected);
    TestFalse(TEXT("Exporter rejects a changed checkpoint pair count"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), RejectedWriter, GWarn));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKyaBridgeFixtureRoundTripTest, "Kya.Bank.BridgeFixtureRoundTrip",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKyaBridgeFixtureRoundTripTest::RunTest(const FString& Parameters)
{
    FKyaSmallBankBuilder Builder;
    const TArray<uint8> Input = Builder.Build(0x17);
    UKyaLevelBankFactory* Factory = NewObject<UKyaLevelBankFactory>();
    const uint8* Cursor = Input.GetData();
    UKyaLevelBank* Bank = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
        GetTransientPackage(), NAME_None, RF_Transient, nullptr, TEXT("bnk"), Cursor,
        Input.GetData() + Input.Num(), GWarn));
    if (!TestNotNull(TEXT("Import bridge fixture"), Bank)) return false;
    if (!TestEqual(TEXT("Bridge fixture actor count"), Bank->SceneEnvironment.Actors.Num(), 1)) return false;
    FKyaActorPlacement& Actor = Bank->SceneEnvironment.Actors[0];
    TestEqual(TEXT("Bridge class id"), Actor.ClassId, int64(0x17));
    TestEqual(TEXT("Bridge actor name"), Actor.Name, FString(TEXT("BRIDGE")));
    TestTrue(TEXT("Bridge class payload parses"), Actor.bHasBridgeData);
    if (!TestEqual(TEXT("Bridge actor ref count"), Actor.BridgeActorRefs.Num(), 1)) return false;
    TestEqual(TEXT("Bridge actor ref"), Actor.BridgeActorRefs[0], 1);
    TestEqual(TEXT("Bridge waypoint A"), Actor.BridgeWaypointA, 2);
    TestEqual(TEXT("Bridge waypoint B"), Actor.BridgeWaypointB, 3);
    TestEqual(TEXT("Bridge field 330"), Actor.BridgeField330, 1.5f);
    TestEqual(TEXT("Bridge field 334"), Actor.BridgeField334, 2.5f);
    TestEqual(TEXT("Bridge field 33c"), Actor.BridgeField33C, 3.5f);
    TestEqual(TEXT("Bridge field 340"), Actor.BridgeField340, 4.5f);
    TestEqual(TEXT("Bridge field 338"), Actor.BridgeField338, 5.5f);
    TestEqual(TEXT("Bridge field 344"), Actor.BridgeField344, 6);
    TestEqual(TEXT("Bridge config flags"), Actor.BridgeConfigFlags, 7);
    TestEqual(TEXT("Bridge field 348"), Actor.BridgeField348, 8);
    TestEqual(TEXT("Bridge step collision scale"), Actor.BridgeStepCollisionScale, 9.5f);
    TestEqual(TEXT("Bridge fixture leaves no raw class data"), Actor.ClassData.Num(), 0);
    if (!TestEqual(TEXT("Bridge behaviour count"), Actor.Behaviours.Num(), 2)) return false;
    const FKyaActorBehaviour& Basic = Actor.Behaviours[0];
    const FKyaActorBehaviour& Wind = Actor.Behaviours[1];
    TestEqual(TEXT("Bridge basic id"), Basic.Id, 2);
    TestTrue(TEXT("Bridge basic is typed"), Basic.bHasTypedBridgeBasicBehaviour);
    TestEqual(TEXT("Bridge basic direction x"), float(Basic.BridgeBasicDirection.X), 1.0f);
    TestEqual(TEXT("Bridge basic direction z"), float(Basic.BridgeBasicDirection.Z), 3.0f);
    TestEqual(TEXT("Bridge basic field 40"), Basic.BridgeBasicField40, 4.0f);
    TestEqual(TEXT("Bridge basic field 50"), Basic.BridgeBasicField50, 8.0f);
    TestEqual(TEXT("Bridge wind id"), Wind.Id, 3);
    TestTrue(TEXT("Bridge wind is typed"), Wind.bHasTypedBridgeWindBehaviour);
    if (!TestEqual(TEXT("Bridge wind value count"), Wind.BridgeWindValues.Num(), 5)) return false;
    TestEqual(TEXT("Bridge wind value 0"), Wind.BridgeWindValues[0], 9.0f);
    TestEqual(TEXT("Bridge wind value 4"), Wind.BridgeWindValues[4], 13.0f);

    UKyaLevelBankExporter* Exporter = NewObject<UKyaLevelBankExporter>();
    TArray<uint8> Plain;
    FMemoryWriter PlainWriter(Plain);
    if (!TestTrue(TEXT("Export unedited bridge fixture"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), PlainWriter, GWarn))) return false;
    TestEqual(TEXT("Unedited bridge export keeps the file size"), Plain.Num(), Input.Num());
    TestTrue(TEXT("Unedited bridge export preserves every byte"), Plain == Input);

    Actor.BridgeActorRefs[0] = 91;
    Actor.BridgeWaypointA = 92;
    Actor.BridgeField330 = 93.5f;
    Actor.BridgeConfigFlags = 94;
    Actor.BridgeStepCollisionScale = 95.5f;
    Actor.Behaviours[0].BridgeBasicField40 = 96.5f;
    Actor.Behaviours[1].BridgeWindValues[2] = 97.5f;
    TArray<uint8> Edited;
    FMemoryWriter EditedWriter(Edited);
    if (!TestTrue(TEXT("Export edited bridge fixture"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), EditedWriter, GWarn))) return false;
    TestEqual(TEXT("Edited bridge export keeps the file size"), Edited.Num(), Input.Num());
    const uint8* EditedCursor = Edited.GetData();
    UKyaLevelBank* Reimported = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
        GetTransientPackage(), NAME_None, RF_Transient, nullptr, TEXT("bnk"), EditedCursor,
        Edited.GetData() + Edited.Num(), GWarn));
    if (!TestNotNull(TEXT("Reimport edited bridge fixture"), Reimported)) return false;
    const FKyaActorPlacement& EditedActor = Reimported->SceneEnvironment.Actors[0];
    TestEqual(TEXT("Edited bridge actor ref round trips"), EditedActor.BridgeActorRefs[0], 91);
    TestEqual(TEXT("Edited bridge waypoint round trips"), EditedActor.BridgeWaypointA, 92);
    TestEqual(TEXT("Edited bridge field 330 round trips"), EditedActor.BridgeField330, 93.5f);
    TestEqual(TEXT("Edited bridge config round trips"), EditedActor.BridgeConfigFlags, 94);
    TestEqual(TEXT("Edited bridge step scale round trips"), EditedActor.BridgeStepCollisionScale, 95.5f);
    TestEqual(TEXT("Edited bridge basic field round trips"),
        EditedActor.Behaviours[0].BridgeBasicField40, 96.5f);
    TestEqual(TEXT("Edited bridge wind value round trips"),
        EditedActor.Behaviours[1].BridgeWindValues[2], 97.5f);

    Actor.BridgeActorRefs.Add(99);
    TArray<uint8> Rejected;
    FMemoryWriter RejectedWriter(Rejected);
    TestFalse(TEXT("Exporter rejects a changed bridge actor-ref count"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), RejectedWriter, GWarn));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKyaFogFixtureRoundTripTest, "Kya.Bank.FogFixtureRoundTrip",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKyaFogFixtureRoundTripTest::RunTest(const FString& Parameters)
{
    FKyaSmallBankBuilder Builder;
    const TArray<uint8> Input = Builder.Build(0x3d);
    UKyaLevelBankFactory* Factory = NewObject<UKyaLevelBankFactory>();
    const uint8* Cursor = Input.GetData();
    UKyaLevelBank* Bank = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
        GetTransientPackage(), NAME_None, RF_Transient, nullptr, TEXT("bnk"), Cursor,
        Input.GetData() + Input.Num(), GWarn));
    if (!TestNotNull(TEXT("Import fog fixture"), Bank)) return false;
    if (!TestEqual(TEXT("Fog fixture actor count"), Bank->SceneEnvironment.Actors.Num(), 1)) return false;
    FKyaActorPlacement& Actor = Bank->SceneEnvironment.Actors[0];
    TestEqual(TEXT("Fog class id"), Actor.ClassId, int64(0x3d));
    TestEqual(TEXT("Fog actor name"), Actor.Name, FString(TEXT("FOGMANAGER")));
    TestTrue(TEXT("Fog class payload parses"), Actor.bHasFogData);
    if (!TestEqual(TEXT("Fog hierarchy count"), Actor.FogHierarchies.Num(), 1)) return false;
    TestEqual(TEXT("Fog hierarchy hash"), Actor.FogHierarchies[0].Hash, int64(0x5566778811223344ll));
    TestEqual(TEXT("Fog hierarchy field 20"), Actor.FogHierarchies[0].Field20, 1.5f);
    TestEqual(TEXT("Fog hierarchy field 24"), Actor.FogHierarchies[0].Field24, 2.5f);
    TestEqual(TEXT("Fog fixture leaves no raw class data"), Actor.ClassData.Num(), 0);
    if (!TestEqual(TEXT("Fog behaviour count"), Actor.Behaviours.Num(), 1)) return false;
    const FKyaActorBehaviour& Fog = Actor.Behaviours[0];
    TestEqual(TEXT("Fog behaviour id"), Fog.Id, 2);
    TestTrue(TEXT("Fog behaviour is typed"), Fog.bHasTypedFogBehaviour);
    if (!TestEqual(TEXT("Fog zone count"), Fog.FogZones.Num(), 1)) return false;
    TestEqual(TEXT("Fog zone flags"), Fog.FogZones[0].Flags, 1);
    TestEqual(TEXT("Fog zone ref"), Fog.FogZones[0].ZoneRef, 2);
    TestEqual(TEXT("Fog zone clip"), Fog.FogZones[0].ClipValue, 3.5f);
    TestEqual(TEXT("Fog zone field 4"), Fog.FogZones[0].Field4, 4.5f);
    TestEqual(TEXT("Fog zone rgba"), Fog.FogZones[0].FogRGBA, 0x11223344);
    TestEqual(TEXT("Fog zone flags 2"), Fog.FogZones[0].FogFlags, 5);
    TestEqual(TEXT("Fog zone field 18"), Fog.FogZones[0].Field18, 6.5f);

    UKyaLevelBankExporter* Exporter = NewObject<UKyaLevelBankExporter>();
    TArray<uint8> Plain;
    FMemoryWriter PlainWriter(Plain);
    if (!TestTrue(TEXT("Export unedited fog fixture"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), PlainWriter, GWarn))) return false;
    TestEqual(TEXT("Unedited fog export keeps the file size"), Plain.Num(), Input.Num());
    TestTrue(TEXT("Unedited fog export preserves every byte"), Plain == Input);

    Actor.FogHierarchies[0].Hash = 0x1122334455667788ll;
    Actor.FogHierarchies[0].Field20 = 91.5f;
    Actor.FogHierarchies[0].Field24 = 92.5f;
    Actor.Behaviours[0].FogZones[0].ZoneRef = 93;
    Actor.Behaviours[0].FogZones[0].ClipValue = 94.5f;
    Actor.Behaviours[0].FogZones[0].FogRGBA = 95;
    Actor.Behaviours[0].FogZones[0].Field18 = 96.5f;
    TArray<uint8> Edited;
    FMemoryWriter EditedWriter(Edited);
    if (!TestTrue(TEXT("Export edited fog fixture"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), EditedWriter, GWarn))) return false;
    TestEqual(TEXT("Edited fog export keeps the file size"), Edited.Num(), Input.Num());
    const uint8* EditedCursor = Edited.GetData();
    UKyaLevelBank* Reimported = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
        GetTransientPackage(), NAME_None, RF_Transient, nullptr, TEXT("bnk"), EditedCursor,
        Edited.GetData() + Edited.Num(), GWarn));
    if (!TestNotNull(TEXT("Reimport edited fog fixture"), Reimported)) return false;
    const FKyaActorPlacement& EditedActor = Reimported->SceneEnvironment.Actors[0];
    TestEqual(TEXT("Edited fog hash round trips"), EditedActor.FogHierarchies[0].Hash,
        int64(0x1122334455667788ll));
    TestEqual(TEXT("Edited fog field 20 round trips"), EditedActor.FogHierarchies[0].Field20, 91.5f);
    TestEqual(TEXT("Edited fog field 24 round trips"), EditedActor.FogHierarchies[0].Field24, 92.5f);
    TestEqual(TEXT("Edited fog zone ref round trips"), EditedActor.Behaviours[0].FogZones[0].ZoneRef, 93);
    TestEqual(TEXT("Edited fog zone clip round trips"),
        EditedActor.Behaviours[0].FogZones[0].ClipValue, 94.5f);
    TestEqual(TEXT("Edited fog zone rgba round trips"),
        EditedActor.Behaviours[0].FogZones[0].FogRGBA, 95);
    TestEqual(TEXT("Edited fog zone field 18 round trips"),
        EditedActor.Behaviours[0].FogZones[0].Field18, 96.5f);

    Actor.FogHierarchies.AddDefaulted();
    TArray<uint8> Rejected;
    FMemoryWriter RejectedWriter(Rejected);
    TestFalse(TEXT("Exporter rejects a changed fog hierarchy count"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), RejectedWriter, GWarn));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKyaShootFixtureRoundTripTest, "Kya.Bank.ShootFixtureRoundTrip",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKyaShootFixtureRoundTripTest::RunTest(const FString& Parameters)
{
    FKyaSmallBankBuilder Builder;
    const TArray<uint8> Input = Builder.Build(0x3e);
    UKyaLevelBankFactory* Factory = NewObject<UKyaLevelBankFactory>();
    const uint8* Cursor = Input.GetData();
    UKyaLevelBank* Bank = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
        GetTransientPackage(), NAME_None, RF_Transient, nullptr, TEXT("bnk"), Cursor,
        Input.GetData() + Input.Num(), GWarn));
    if (!TestNotNull(TEXT("Import shoot fixture"), Bank)) return false;
    if (!TestEqual(TEXT("Shoot fixture actor count"), Bank->SceneEnvironment.Actors.Num(), 1)) return false;
    FKyaActorPlacement& Actor = Bank->SceneEnvironment.Actors[0];
    TestEqual(TEXT("Shoot class id"), Actor.ClassId, int64(0x3e));
    TestEqual(TEXT("Shoot actor name"), Actor.Name, FString(TEXT("SHOOT")));
    TestTrue(TEXT("Shoot class payload parses"), Actor.bHasShootData);
    TestEqual(TEXT("Shoot weight"), Actor.ShootWeightB, 1.25f);
    TestEqual(TEXT("Shoot life maximum is exact"), Actor.ShootLifeMax, int64(16777217));
    TestEqual(TEXT("Shoot gravity"), Actor.ShootGravityScale, 2.0f);
    TestEqual(TEXT("Shoot field 350"), Actor.ShootField350, 11);
    TestEqual(TEXT("Shoot field 354"), Actor.ShootField354, 12);
    TestEqual(TEXT("Shoot texture"), Actor.ShootTextureIndex, 13);
    TestEqual(TEXT("Shoot mesh"), Actor.ShootMeshIndex, 14);
    TestEqual(TEXT("Shoot field 3c0"), Actor.ShootField3C0, 1.0f);
    TestEqual(TEXT("Shoot field 3cc"), Actor.ShootField3CC, 4.0f);
    TestEqual(TEXT("Shoot field 3d0"), Actor.ShootField3D0, 15);
    TestEqual(TEXT("Shoot field 3d4"), Actor.ShootField3D4, 5.0f);
    TestEqual(TEXT("Shoot field 3e0"), Actor.ShootField3E0, 8.0f);
    TestEqual(TEXT("Shoot field 3e8"), Actor.ShootField3E8, 16);
    TestEqual(TEXT("Shoot field 3e4"), Actor.ShootField3E4, 17);
    TestEqual(TEXT("Shoot field 3ec"), Actor.ShootField3EC, 18);
    TestEqual(TEXT("Shoot field 3f0"), Actor.ShootField3F0, 19);
    TestEqual(TEXT("Shoot add-on money type"), Actor.ShootAddOn.MoneyType, 2);
    TestEqual(TEXT("Shoot add-on money count"), Actor.ShootAddOn.MoneyCount, 3);
    TestEqual(TEXT("Shoot add-on sub-object mode"), Actor.ShootAddOn.SubObjMode, 1);
    TestEqual(TEXT("Shoot add-on float 0"), Actor.ShootAddOn.SubObjField0, 0.25f);
    TestEqual(TEXT("Shoot add-on float 14"), Actor.ShootAddOn.SubObjField14, 1.5f);
    TestEqual(TEXT("Shoot fixture leaves no raw class data"), Actor.ClassData.Num(), 0);
    if (!TestEqual(TEXT("Shoot behaviour count"), Actor.Behaviours.Num(), 2)) return false;
    const FKyaActorBehaviour& Fire = Actor.Behaviours[0];
    const FKyaActorBehaviour& Wave = Actor.Behaviours[1];
    TestEqual(TEXT("Shoot fire id"), Fire.Id, 3);
    TestTrue(TEXT("Shoot fire is typed"), Fire.bHasTypedShootFireBehaviour);
    TestEqual(TEXT("Shoot fire field 2b0"), Fire.ShootFireField2B0, 7);
    TestEqual(TEXT("Shoot fire field 8"), Fire.ShootFireField8, 1.5f);
    TestEqual(TEXT("Shoot fire field 2b4"), Fire.ShootFireField2B4, 2.5f);
    if (!TestEqual(TEXT("Shoot fire ref count"), Fire.ShootFireActorRefs.Num(), 3)) return false;
    TestEqual(TEXT("Shoot fire ref 0"), Fire.ShootFireActorRefs[0], 21);
    TestEqual(TEXT("Shoot fire ref 2"), Fire.ShootFireActorRefs[2], 23);
    TestEqual(TEXT("Shoot wave id"), Wave.Id, 4);
    TestTrue(TEXT("Shoot wave is typed"), Wave.bHasTypedShootWaveBehaviour);
    if (!TestEqual(TEXT("Shoot wave value count"), Wave.ShootWaveValues.Num(), 7)) return false;
    TestEqual(TEXT("Shoot wave value 0"), Wave.ShootWaveValues[0], 1.0f);
    TestEqual(TEXT("Shoot wave value 6"), Wave.ShootWaveValues[6], 7.0f);

    UKyaLevelBankExporter* Exporter = NewObject<UKyaLevelBankExporter>();
    TArray<uint8> Plain;
    FMemoryWriter PlainWriter(Plain);
    if (!TestTrue(TEXT("Export unedited shoot fixture"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), PlainWriter, GWarn))) return false;
    TestEqual(TEXT("Unedited shoot export keeps the file size"), Plain.Num(), Input.Num());
    TestTrue(TEXT("Unedited shoot export preserves every byte"), Plain == Input);

    Actor.ShootWeightB = 91.5f;
    Actor.ShootLifeMax = 4000000000ll;
    Actor.ShootField350 = 92;
    Actor.ShootField3C0 = 93.5f;
    Actor.ShootAddOn.MoneyCount = 94;
    Actor.Behaviours[0].ShootFireActorRefs[1] = 95;
    Actor.Behaviours[0].ShootFireField8 = 96.5f;
    Actor.Behaviours[1].ShootWaveValues[3] = 97.5f;
    TArray<uint8> Edited;
    FMemoryWriter EditedWriter(Edited);
    if (!TestTrue(TEXT("Export edited shoot fixture"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), EditedWriter, GWarn))) return false;
    TestEqual(TEXT("Edited shoot export keeps the file size"), Edited.Num(), Input.Num());
    const uint8* EditedCursor = Edited.GetData();
    UKyaLevelBank* Reimported = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
        GetTransientPackage(), NAME_None, RF_Transient, nullptr, TEXT("bnk"), EditedCursor,
        Edited.GetData() + Edited.Num(), GWarn));
    if (!TestNotNull(TEXT("Reimport edited shoot fixture"), Reimported)) return false;
    const FKyaActorPlacement& EditedActor = Reimported->SceneEnvironment.Actors[0];
    TestEqual(TEXT("Edited shoot weight round trips"), EditedActor.ShootWeightB, 91.5f);
    TestEqual(TEXT("Edited shoot life round trips"), EditedActor.ShootLifeMax, int64(4000000000ll));
    TestEqual(TEXT("Edited shoot field 350 round trips"), EditedActor.ShootField350, 92);
    TestEqual(TEXT("Edited shoot field 3c0 round trips"), EditedActor.ShootField3C0, 93.5f);
    TestEqual(TEXT("Edited shoot money round trips"), EditedActor.ShootAddOn.MoneyCount, 94);
    TestEqual(TEXT("Edited shoot fire ref round trips"),
        EditedActor.Behaviours[0].ShootFireActorRefs[1], 95);
    TestEqual(TEXT("Edited shoot fire field round trips"),
        EditedActor.Behaviours[0].ShootFireField8, 96.5f);
    TestEqual(TEXT("Edited shoot wave round trips"),
        EditedActor.Behaviours[1].ShootWaveValues[3], 97.5f);

    Actor.Behaviours[0].ShootFireActorRefs.Add(99);
    TArray<uint8> Rejected;
    FMemoryWriter RejectedWriter(Rejected);
    TestFalse(TEXT("Exporter rejects a changed shoot fire-ref count"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), RejectedWriter, GWarn));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKyaMoneyFixtureRoundTripTest, "Kya.Bank.MoneyFixtureRoundTrip",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKyaMoneyFixtureRoundTripTest::RunTest(const FString& Parameters)
{
    FKyaSmallBankBuilder Builder;
    const TArray<uint8> Input = Builder.Build(0x32);
    UKyaLevelBankFactory* Factory = NewObject<UKyaLevelBankFactory>();
    const uint8* Cursor = Input.GetData();
    UKyaLevelBank* Bank = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
        GetTransientPackage(), NAME_None, RF_Transient, nullptr, TEXT("bnk"), Cursor,
        Input.GetData() + Input.Num(), GWarn));
    if (!TestNotNull(TEXT("Import money fixture"), Bank)) return false;
    if (!TestEqual(TEXT("Money fixture actor count"), Bank->SceneEnvironment.Actors.Num(), 1)) return false;
    FKyaActorPlacement& Actor = Bank->SceneEnvironment.Actors[0];
    TestEqual(TEXT("Money class id"), Actor.ClassId, int64(0x32));
    TestEqual(TEXT("Money actor name"), Actor.Name, FString(TEXT("MONEY")));
    TestTrue(TEXT("Money class payload parses"), Actor.bHasMoneyData);
    TestEqual(TEXT("Money weight"), Actor.MoneyWeightB, 1.25f);
    TestEqual(TEXT("Money value"), Actor.MoneyValue, 100);
    TestEqual(TEXT("Money sound"), Actor.MoneySoundRef, 5);
    TestEqual(TEXT("Money material"), Actor.MoneyMaterialId, 6);
    TestEqual(TEXT("Money fixture leaves no raw class data"), Actor.ClassData.Num(), 0);
    if (!TestEqual(TEXT("Money behaviour count"), Actor.Behaviours.Num(), 2)) return false;
    const FKyaActorBehaviour& Flock = Actor.Behaviours[0];
    const FKyaActorBehaviour& AddOn = Actor.Behaviours[1];
    TestEqual(TEXT("Money flock id"), Flock.Id, 2);
    TestTrue(TEXT("Money flock is typed"), Flock.bHasTypedMoneyFlockBehaviour);
    TestEqual(TEXT("Money flock path"), Flock.MoneyFlockPathIndex, 21);
    TestEqual(TEXT("Money add-on id"), AddOn.Id, 3);
    TestEqual(TEXT("Money add-on payload is empty"), AddOn.Data.Num(), 0);

    UKyaLevelBankExporter* Exporter = NewObject<UKyaLevelBankExporter>();
    TArray<uint8> Plain;
    FMemoryWriter PlainWriter(Plain);
    if (!TestTrue(TEXT("Export unedited money fixture"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), PlainWriter, GWarn))) return false;
    TestEqual(TEXT("Unedited money export keeps the file size"), Plain.Num(), Input.Num());
    TestTrue(TEXT("Unedited money export preserves every byte"), Plain == Input);

    Actor.MoneyWeightB = 91.5f;
    Actor.MoneyValue = 92;
    Actor.MoneySoundRef = 93;
    Actor.MoneyMaterialId = 94;
    Actor.Behaviours[0].MoneyFlockPathIndex = 95;
    TArray<uint8> Edited;
    FMemoryWriter EditedWriter(Edited);
    if (!TestTrue(TEXT("Export edited money fixture"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), EditedWriter, GWarn))) return false;
    TestEqual(TEXT("Edited money export keeps the file size"), Edited.Num(), Input.Num());
    const uint8* EditedCursor = Edited.GetData();
    UKyaLevelBank* Reimported = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
        GetTransientPackage(), NAME_None, RF_Transient, nullptr, TEXT("bnk"), EditedCursor,
        Edited.GetData() + Edited.Num(), GWarn));
    if (!TestNotNull(TEXT("Reimport edited money fixture"), Reimported)) return false;
    const FKyaActorPlacement& EditedActor = Reimported->SceneEnvironment.Actors[0];
    TestEqual(TEXT("Edited money weight round trips"), EditedActor.MoneyWeightB, 91.5f);
    TestEqual(TEXT("Edited money value round trips"), EditedActor.MoneyValue, 92);
    TestEqual(TEXT("Edited money sound round trips"), EditedActor.MoneySoundRef, 93);
    TestEqual(TEXT("Edited money material round trips"), EditedActor.MoneyMaterialId, 94);
    TestEqual(TEXT("Edited money flock path round trips"),
        EditedActor.Behaviours[0].MoneyFlockPathIndex, 95);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKyaBoxFixtureRoundTripTest, "Kya.Bank.BoxFixtureRoundTrip",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKyaBoxFixtureRoundTripTest::RunTest(const FString& Parameters)
{
    FKyaSmallBankBuilder Builder;
    const TArray<uint8> Input = Builder.Build(0x35);
    UKyaLevelBankFactory* Factory = NewObject<UKyaLevelBankFactory>();
    const uint8* Cursor = Input.GetData();
    UKyaLevelBank* Bank = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
        GetTransientPackage(), NAME_None, RF_Transient, nullptr, TEXT("bnk"), Cursor,
        Input.GetData() + Input.Num(), GWarn));
    if (!TestNotNull(TEXT("Import box fixture"), Bank)) return false;
    if (!TestEqual(TEXT("Box fixture actor count"), Bank->SceneEnvironment.Actors.Num(), 1)) return false;
    FKyaActorPlacement& Actor = Bank->SceneEnvironment.Actors[0];
    TestEqual(TEXT("Box class id"), Actor.ClassId, int64(0x35));
    TestEqual(TEXT("Box actor name"), Actor.Name, FString(TEXT("BOX")));
    TestTrue(TEXT("Box class payload parses"), Actor.bHasBoxData);
    TestEqual(TEXT("Box weight"), Actor.BoxWeightB, 1.25f);
    TestEqual(TEXT("Box life maximum is exact"), Actor.BoxLifeMax, int64(16777217));
    TestEqual(TEXT("Box gravity"), Actor.BoxGravityScale, 2.0f);
    TestEqual(TEXT("Box field 350"), Actor.BoxField350, 11);
    if (!TestEqual(TEXT("Box actor ref count"), Actor.BoxActorRefs.Num(), 1)) return false;
    TestEqual(TEXT("Box actor ref"), Actor.BoxActorRefs[0], 21);
    TestEqual(TEXT("Box field 354"), Actor.BoxField354, 1.0f);
    TestEqual(TEXT("Box field 360"), Actor.BoxField360, 4.0f);
    TestEqual(TEXT("Box field 364"), Actor.BoxField364, 12);
    TestEqual(TEXT("Box field 370"), Actor.BoxField370, 13);
    TestEqual(TEXT("Box field 374"), Actor.BoxField374, 14);
    TestEqual(TEXT("Box field 378"), Actor.BoxField378, 15);
    TestEqual(TEXT("Box field 37c"), Actor.BoxField37C, 16);
    if (!TestEqual(TEXT("Box switch target count"), Actor.BoxSwitch.Targets.Num(), 1)) return false;
    TestEqual(TEXT("Box switch target"), Actor.BoxSwitch.Targets[0].TargetRef, 31);
    TestEqual(TEXT("Box switch camera"), Actor.BoxSwitch.Camera.CameraIndex, 41);
    TestEqual(TEXT("Box add-on money type"), Actor.BoxAddOn.MoneyType, 2);
    TestEqual(TEXT("Box add-on money count"), Actor.BoxAddOn.MoneyCount, 3);
    TestEqual(TEXT("Box add-on sub-object mode"), Actor.BoxAddOn.SubObjMode, 1);
    TestEqual(TEXT("Box add-on float 0"), Actor.BoxAddOn.SubObjField0, 0.25f);
    TestEqual(TEXT("Box add-on float 14"), Actor.BoxAddOn.SubObjField14, 1.5f);
    TestEqual(TEXT("Box fixture leaves no raw class data"), Actor.ClassData.Num(), 0);
    if (!TestEqual(TEXT("Box behaviour count"), Actor.Behaviours.Num(), 1)) return false;
    TestEqual(TEXT("Box behaviour id"), Actor.Behaviours[0].Id, 3);
    TestEqual(TEXT("Box behaviour is empty"), Actor.Behaviours[0].Data.Num(), 0);

    UKyaLevelBankExporter* Exporter = NewObject<UKyaLevelBankExporter>();
    TArray<uint8> Plain;
    FMemoryWriter PlainWriter(Plain);
    if (!TestTrue(TEXT("Export unedited box fixture"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), PlainWriter, GWarn))) return false;
    TestEqual(TEXT("Unedited box export keeps the file size"), Plain.Num(), Input.Num());
    TestTrue(TEXT("Unedited box export preserves every byte"), Plain == Input);

    Actor.BoxWeightB = 91.5f;
    Actor.BoxLifeMax = 4000000000ll;
    Actor.BoxField350 = 92;
    Actor.BoxActorRefs[0] = 93;
    Actor.BoxField364 = 94;
    Actor.BoxSwitch.Targets[0].TargetRef = 95;
    Actor.BoxAddOn.MoneyCount = 96;
    Actor.BoxAddOn.SubObjFieldC = 97.5f;
    TArray<uint8> Edited;
    FMemoryWriter EditedWriter(Edited);
    if (!TestTrue(TEXT("Export edited box fixture"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), EditedWriter, GWarn))) return false;
    TestEqual(TEXT("Edited box export keeps the file size"), Edited.Num(), Input.Num());
    const uint8* EditedCursor = Edited.GetData();
    UKyaLevelBank* Reimported = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
        GetTransientPackage(), NAME_None, RF_Transient, nullptr, TEXT("bnk"), EditedCursor,
        Edited.GetData() + Edited.Num(), GWarn));
    if (!TestNotNull(TEXT("Reimport edited box fixture"), Reimported)) return false;
    const FKyaActorPlacement& EditedActor = Reimported->SceneEnvironment.Actors[0];
    TestEqual(TEXT("Edited box weight round trips"), EditedActor.BoxWeightB, 91.5f);
    TestEqual(TEXT("Edited box life round trips"), EditedActor.BoxLifeMax, int64(4000000000ll));
    TestEqual(TEXT("Edited box field 350 round trips"), EditedActor.BoxField350, 92);
    TestEqual(TEXT("Edited box actor ref round trips"), EditedActor.BoxActorRefs[0], 93);
    TestEqual(TEXT("Edited box field 364 round trips"), EditedActor.BoxField364, 94);
    TestEqual(TEXT("Edited box switch target round trips"),
        EditedActor.BoxSwitch.Targets[0].TargetRef, 95);
    TestEqual(TEXT("Edited box money round trips"), EditedActor.BoxAddOn.MoneyCount, 96);
    TestEqual(TEXT("Edited box add-on float round trips"),
        EditedActor.BoxAddOn.SubObjFieldC, 97.5f);

    Actor.BoxActorRefs.Add(99);
    TArray<uint8> Rejected;
    FMemoryWriter RejectedWriter(Rejected);
    TestFalse(TEXT("Exporter rejects a changed box actor-ref count"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), RejectedWriter, GWarn));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKyaFxFixtureRoundTripTest, "Kya.Bank.FxFixtureRoundTrip",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKyaFxFixtureRoundTripTest::RunTest(const FString& Parameters)
{
    FKyaSmallBankBuilder Builder;
    const TArray<uint8> Input = Builder.Build(0x2e);
    UKyaLevelBankFactory* Factory = NewObject<UKyaLevelBankFactory>();
    const uint8* Cursor = Input.GetData();
    UKyaLevelBank* Bank = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
        GetTransientPackage(), NAME_None, RF_Transient, nullptr, TEXT("bnk"), Cursor,
        Input.GetData() + Input.Num(), GWarn));
    if (!TestNotNull(TEXT("Import FX fixture"), Bank)) return false;
    if (!TestEqual(TEXT("FX fixture actor count"), Bank->SceneEnvironment.Actors.Num(), 1)) return false;
    FKyaActorPlacement& Actor = Bank->SceneEnvironment.Actors[0];
    TestEqual(TEXT("FX class id"), Actor.ClassId, int64(0x2e));
    TestEqual(TEXT("FX actor name"), Actor.Name, FString(TEXT("ACTORFX")));
    TestTrue(TEXT("FX class payload parses"), Actor.bHasFxData);
    TestEqual(TEXT("FX field 19c"), Actor.FxField19C, 0.5f);
    TestEqual(TEXT("FX field 1a0"), Actor.FxField1A0, 1.5f);
    TestEqual(TEXT("FX fixture leaves no raw class data"), Actor.ClassData.Num(), 0);
    if (!TestEqual(TEXT("FX behaviour count"), Actor.Behaviours.Num(), 3)) return false;
    const FKyaActorBehaviour& Single = Actor.Behaviours[0];
    const FKyaActorBehaviour& Path = Actor.Behaviours[1];
    const FKyaActorBehaviour& Unknown = Actor.Behaviours[2];
    TestEqual(TEXT("FX single id"), Single.Id, 2);
    TestTrue(TEXT("FX single is typed"), Single.bHasTypedFxBehaviour);
    TestEqual(TEXT("FX single value"), Single.FxBehaviourValue, 7);
    TestEqual(TEXT("FX path id"), Path.Id, 3);
    TestTrue(TEXT("FX path is typed"), Path.bHasTypedFxBehaviour);
    TestEqual(TEXT("FX path value"), Path.FxBehaviourValue, 8);
    TestEqual(TEXT("FX path index"), Path.FxPathFollowIndex, 9);
    TestEqual(TEXT("FX unknown id"), Unknown.Id, 4);
    TestTrue(TEXT("FX unknown is typed"), Unknown.bHasTypedFxBehaviour);
    if (!TestEqual(TEXT("FX unknown value count"), Unknown.FxUnknownValues.Num(), 2)) return false;
    TestEqual(TEXT("FX unknown value 0"), Unknown.FxUnknownValues[0], 10);
    TestEqual(TEXT("FX unknown value 1"), Unknown.FxUnknownValues[1], 11);

    UKyaLevelBankExporter* Exporter = NewObject<UKyaLevelBankExporter>();
    TArray<uint8> Plain;
    FMemoryWriter PlainWriter(Plain);
    if (!TestTrue(TEXT("Export unedited FX fixture"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), PlainWriter, GWarn))) return false;
    TestEqual(TEXT("Unedited FX export keeps the file size"), Plain.Num(), Input.Num());
    TestTrue(TEXT("Unedited FX export preserves every byte"), Plain == Input);

    Actor.FxField19C = 91.5f;
    Actor.FxField1A0 = 92.5f;
    Actor.Behaviours[0].FxBehaviourValue = 93;
    Actor.Behaviours[1].FxPathFollowIndex = 94;
    Actor.Behaviours[2].FxUnknownValues[1] = 95;
    TArray<uint8> Edited;
    FMemoryWriter EditedWriter(Edited);
    if (!TestTrue(TEXT("Export edited FX fixture"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), EditedWriter, GWarn))) return false;
    TestEqual(TEXT("Edited FX export keeps the file size"), Edited.Num(), Input.Num());
    const uint8* EditedCursor = Edited.GetData();
    UKyaLevelBank* Reimported = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
        GetTransientPackage(), NAME_None, RF_Transient, nullptr, TEXT("bnk"), EditedCursor,
        Edited.GetData() + Edited.Num(), GWarn));
    if (!TestNotNull(TEXT("Reimport edited FX fixture"), Reimported)) return false;
    const FKyaActorPlacement& EditedActor = Reimported->SceneEnvironment.Actors[0];
    if (!TestEqual(TEXT("Edited FX behaviour count"), EditedActor.Behaviours.Num(), 3)) return false;
    TestEqual(TEXT("Edited FX field 19c round trips"), EditedActor.FxField19C, 91.5f);
    TestEqual(TEXT("Edited FX field 1a0 round trips"), EditedActor.FxField1A0, 92.5f);
    TestEqual(TEXT("Edited FX single round trips"), EditedActor.Behaviours[0].FxBehaviourValue, 93);
    TestEqual(TEXT("Edited FX path round trips"), EditedActor.Behaviours[1].FxPathFollowIndex, 94);
    TestEqual(TEXT("Edited FX unknown value round trips"),
        EditedActor.Behaviours[2].FxUnknownValues[1], 95);

    Actor.Behaviours[2].FxUnknownValues.Add(99);
    TArray<uint8> Rejected;
    FMemoryWriter RejectedWriter(Rejected);
    TestFalse(TEXT("Exporter rejects a changed FX unknown-value count"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), RejectedWriter, GWarn));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKyaClusteriserFixtureRoundTripTest, "Kya.Bank.ClusteriserFixtureRoundTrip",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKyaClusteriserFixtureRoundTripTest::RunTest(const FString& Parameters)
{
    FKyaSmallBankBuilder Builder;
    const TArray<uint8> Input = Builder.Build(0x18);
    UKyaLevelBankFactory* Factory = NewObject<UKyaLevelBankFactory>();
    const uint8* Cursor = Input.GetData();
    UKyaLevelBank* Bank = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
        GetTransientPackage(), NAME_None, RF_Transient, nullptr, TEXT("bnk"), Cursor,
        Input.GetData() + Input.Num(), GWarn));
    if (!TestNotNull(TEXT("Import clusteriser fixture"), Bank)) return false;
    if (!TestEqual(TEXT("Clusteriser fixture actor count"), Bank->SceneEnvironment.Actors.Num(), 1)) return false;
    FKyaActorPlacement& Actor = Bank->SceneEnvironment.Actors[0];
    TestEqual(TEXT("Clusteriser class id"), Actor.ClassId, int64(0x18));
    TestEqual(TEXT("Clusteriser actor name"), Actor.Name, FString(TEXT("CLUSTERISER")));
    TestEqual(TEXT("Clusteriser has no raw class data"), Actor.ClassData.Num(), 0);
    if (!TestEqual(TEXT("Clusteriser behaviour count"), Actor.Behaviours.Num(), 1)) return false;
    const FKyaActorBehaviour& Zones = Actor.Behaviours[0];
    TestEqual(TEXT("Clusteriser behaviour id"), Zones.Id, 2);
    TestTrue(TEXT("Clusteriser zones are typed"), Zones.bHasTypedClusterZones);
    if (!TestEqual(TEXT("Clusteriser zone count"), Zones.ClusterZones.Num(), 1)) return false;
    const FKyaClusterZone& Zone = Zones.ClusterZones[0];
    TestEqual(TEXT("Cluster zone flags"), Zone.Flags, int64(1));
    TestEqual(TEXT("Cluster zone ref"), Zone.ZoneRef, 2);
    if (!TestEqual(TEXT("Cluster zone actor count"), Zone.Actors.Num(), 1)) return false;
    TestEqual(TEXT("Cluster zone actor"), Zone.Actors[0], 3);
    TestEqual(TEXT("Cluster zone other-actor count"), Zone.OtherActors.Num(), 0);
    if (!TestEqual(TEXT("Cluster zone camera count"), Zone.Cameras.Num(), 1)) return false;
    TestEqual(TEXT("Cluster zone camera"), Zone.Cameras[0], 4);
    TestEqual(TEXT("Cluster zone light count"), Zone.Lights.Num(), 0);
    if (!TestEqual(TEXT("Cluster zone hash count"), Zone.Hashes.Num(), 1)) return false;
    TestEqual(TEXT("Cluster zone hash"), Zone.Hashes[0], int64(0x5566778811223344ll));
    TestEqual(TEXT("Cluster zone other ref"), Zone.OtherZoneRef, 5);

    UKyaLevelBankExporter* Exporter = NewObject<UKyaLevelBankExporter>();
    TArray<uint8> Plain;
    FMemoryWriter PlainWriter(Plain);
    if (!TestTrue(TEXT("Export unedited clusteriser fixture"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), PlainWriter, GWarn))) return false;
    TestEqual(TEXT("Unedited clusteriser export keeps the file size"), Plain.Num(), Input.Num());
    TestTrue(TEXT("Unedited clusteriser export preserves every byte"), Plain == Input);

    Actor.Behaviours[0].ClusterZones[0].Flags = 91;
    Actor.Behaviours[0].ClusterZones[0].ZoneRef = 92;
    Actor.Behaviours[0].ClusterZones[0].Actors[0] = 93;
    Actor.Behaviours[0].ClusterZones[0].Cameras[0] = 94;
    Actor.Behaviours[0].ClusterZones[0].Hashes[0] = 0x1122334455667788ll;
    Actor.Behaviours[0].ClusterZones[0].OtherZoneRef = 95;
    TArray<uint8> Edited;
    FMemoryWriter EditedWriter(Edited);
    if (!TestTrue(TEXT("Export edited clusteriser fixture"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), EditedWriter, GWarn))) return false;
    TestEqual(TEXT("Edited clusteriser export keeps the file size"), Edited.Num(), Input.Num());
    const uint8* EditedCursor = Edited.GetData();
    UKyaLevelBank* Reimported = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
        GetTransientPackage(), NAME_None, RF_Transient, nullptr, TEXT("bnk"), EditedCursor,
        Edited.GetData() + Edited.Num(), GWarn));
    if (!TestNotNull(TEXT("Reimport edited clusteriser fixture"), Reimported)) return false;
    const FKyaClusterZone& EditedZone =
        Reimported->SceneEnvironment.Actors[0].Behaviours[0].ClusterZones[0];
    TestEqual(TEXT("Edited cluster flags round trips"), EditedZone.Flags, int64(91));
    TestEqual(TEXT("Edited cluster zone ref round trips"), EditedZone.ZoneRef, 92);
    TestEqual(TEXT("Edited cluster actor round trips"), EditedZone.Actors[0], 93);
    TestEqual(TEXT("Edited cluster camera round trips"), EditedZone.Cameras[0], 94);
    TestEqual(TEXT("Edited cluster hash round trips"), EditedZone.Hashes[0],
        int64(0x1122334455667788ll));
    TestEqual(TEXT("Edited cluster other ref round trips"), EditedZone.OtherZoneRef, 95);

    Actor.Behaviours[0].ClusterZones[0].Actors.Add(99);
    TArray<uint8> Rejected;
    FMemoryWriter RejectedWriter(Rejected);
    TestFalse(TEXT("Exporter rejects a changed cluster actor count"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), RejectedWriter, GWarn));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKyaCompanionFixtureRoundTripTest, "Kya.Bank.CompanionFixtureRoundTrip",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKyaCompanionFixtureRoundTripTest::RunTest(const FString& Parameters)
{
    FKyaSmallBankBuilder Builder;
    const TArray<uint8> Input = Builder.Build(0x19);
    UKyaLevelBankFactory* Factory = NewObject<UKyaLevelBankFactory>();
    const uint8* Cursor = Input.GetData();
    UKyaLevelBank* Bank = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
        GetTransientPackage(), NAME_None, RF_Transient, nullptr, TEXT("bnk"), Cursor,
        Input.GetData() + Input.Num(), GWarn));
    if (!TestNotNull(TEXT("Import companion fixture"), Bank)) return false;
    if (!TestEqual(TEXT("Companion fixture actor count"), Bank->SceneEnvironment.Actors.Num(), 1)) return false;
    FKyaActorPlacement& Actor = Bank->SceneEnvironment.Actors[0];
    TestEqual(TEXT("Companion class id"), Actor.ClassId, int64(0x19));
    TestEqual(TEXT("Companion actor name"), Actor.Name, FString(TEXT("COMPANION")));
    TestTrue(TEXT("Companion class payload parses"), Actor.bHasCompanionData);
    TestEqual(TEXT("Companion weight"), Actor.CompanionWeightB, 1.25f);
    TestEqual(TEXT("Companion life maximum is exact"), Actor.CompanionLifeMax, int64(16777217));
    TestEqual(TEXT("Companion gravity"), Actor.CompanionGravityScale, 2.0f);
    TestEqual(TEXT("Companion field 358"), Actor.CompanionField358, 11);
    TestEqual(TEXT("Companion material 35c"), Actor.CompanionMaterial35C, 12);
    TestEqual(TEXT("Companion field 360"), Actor.CompanionField360, 13);
    TestEqual(TEXT("Companion fixture leaves no raw class data"), Actor.ClassData.Num(), 0);
    if (!TestEqual(TEXT("Companion behaviour count"), Actor.Behaviours.Num(), 1)) return false;
    const FKyaActorBehaviour& AlertBehaviour = Actor.Behaviours[0];
    TestEqual(TEXT("Companion behaviour id"), AlertBehaviour.Id, 3);
    TestTrue(TEXT("Companion alert is typed"), AlertBehaviour.bHasTypedCompanionBehaviour);
    if (!TestEqual(TEXT("Companion alert count"), AlertBehaviour.CompanionAlerts.Num(), 1)) return false;
    const FKyaCompanionAlert& Alert = AlertBehaviour.CompanionAlerts[0];
    TestEqual(TEXT("Companion alert conditions"), Alert.Conditions.Num(), 0);
    TestEqual(TEXT("Companion alert flags"), Alert.Flags, 1);
    TestEqual(TEXT("Companion alert field 08"), Alert.Field08, 2);
    TestEqual(TEXT("Companion alert field 18"), Alert.Field18, 5.5f);
    TestEqual(TEXT("Companion alert actor"), Alert.ActorRef, 6);
    TestEqual(TEXT("Companion alert vector y"), float(Alert.Field20.Y), 8.0f);
    TestEqual(TEXT("Companion alert camera"), Alert.CameraRef, 10);
    TestEqual(TEXT("Companion alert list"), Alert.Field34.Num(), 0);
    TestEqual(TEXT("Companion alert string key"), Alert.LevelStringKey, int64(0x5566778811223344ll));
    TestEqual(TEXT("Companion alert field 50"), Alert.Field50, 11.5f);
    TestEqual(TEXT("Companion alert field 58"), Alert.Field58, 12);
    TestEqual(TEXT("Companion alert field 5c"), Alert.Field5C, 13.5f);
    TestEqual(TEXT("Companion alert switch targets"), Alert.TargetSwitch.Targets.Num(), 0);
    TestEqual(TEXT("Companion alert switch camera"), Alert.TargetSwitch.Camera.CameraIndex, 21);
    TestEqual(TEXT("Companion alert conditioned blocks"), Alert.Field70.Num(), 0);

    UKyaLevelBankExporter* Exporter = NewObject<UKyaLevelBankExporter>();
    TArray<uint8> Plain;
    FMemoryWriter PlainWriter(Plain);
    if (!TestTrue(TEXT("Export unedited companion fixture"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), PlainWriter, GWarn))) return false;
    TestEqual(TEXT("Unedited companion export keeps the file size"), Plain.Num(), Input.Num());
    TestTrue(TEXT("Unedited companion export preserves every byte"), Plain == Input);

    Actor.CompanionWeightB = 91.5f;
    Actor.CompanionLifeMax = 4000000000ll;
    Actor.CompanionField358 = 92;
    Actor.CompanionMaterial35C = 93;
    Actor.Behaviours[0].CompanionAlerts[0].Flags = 94;
    Actor.Behaviours[0].CompanionAlerts[0].ActorRef = 95;
    Actor.Behaviours[0].CompanionAlerts[0].LevelStringKey = 0x1122334455667788ll;
    Actor.Behaviours[0].CompanionAlerts[0].TargetSwitch.Camera.CameraIndex = 96;
    TArray<uint8> Edited;
    FMemoryWriter EditedWriter(Edited);
    if (!TestTrue(TEXT("Export edited companion fixture"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), EditedWriter, GWarn))) return false;
    TestEqual(TEXT("Edited companion export keeps the file size"), Edited.Num(), Input.Num());
    const uint8* EditedCursor = Edited.GetData();
    UKyaLevelBank* Reimported = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
        GetTransientPackage(), NAME_None, RF_Transient, nullptr, TEXT("bnk"), EditedCursor,
        Edited.GetData() + Edited.Num(), GWarn));
    if (!TestNotNull(TEXT("Reimport edited companion fixture"), Reimported)) return false;
    const FKyaActorPlacement& EditedActor = Reimported->SceneEnvironment.Actors[0];
    TestEqual(TEXT("Edited companion weight round trips"), EditedActor.CompanionWeightB, 91.5f);
    TestEqual(TEXT("Edited companion life round trips"), EditedActor.CompanionLifeMax, int64(4000000000ll));
    TestEqual(TEXT("Edited companion field 358 round trips"), EditedActor.CompanionField358, 92);
    TestEqual(TEXT("Edited companion material round trips"), EditedActor.CompanionMaterial35C, 93);
    TestEqual(TEXT("Edited companion alert flags round trips"),
        EditedActor.Behaviours[0].CompanionAlerts[0].Flags, 94);
    TestEqual(TEXT("Edited companion alert actor round trips"),
        EditedActor.Behaviours[0].CompanionAlerts[0].ActorRef, 95);
    TestEqual(TEXT("Edited companion alert string round trips"),
        EditedActor.Behaviours[0].CompanionAlerts[0].LevelStringKey, int64(0x1122334455667788ll));
    TestEqual(TEXT("Edited companion alert camera round trips"),
        EditedActor.Behaviours[0].CompanionAlerts[0].TargetSwitch.Camera.CameraIndex, 96);

    Actor.Behaviours[0].CompanionAlerts.AddDefaulted();
    TArray<uint8> Rejected;
    FMemoryWriter RejectedWriter(Rejected);
    TestFalse(TEXT("Exporter rejects a changed companion alert count"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), RejectedWriter, GWarn));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKyaGravityFixtureRoundTripTest, "Kya.Bank.GravityFixtureRoundTrip",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKyaGravityFixtureRoundTripTest::RunTest(const FString& Parameters)
{
    FKyaSmallBankBuilder Builder;
    const TArray<uint8> Input = Builder.Build(0x10);
    UKyaLevelBankFactory* Factory = NewObject<UKyaLevelBankFactory>();
    const uint8* Cursor = Input.GetData();
    UKyaLevelBank* Bank = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
        GetTransientPackage(), NAME_None, RF_Transient, nullptr, TEXT("bnk"), Cursor,
        Input.GetData() + Input.Num(), GWarn));
    if (!TestNotNull(TEXT("Import gravity fixture"), Bank)) return false;
    if (!TestEqual(TEXT("Gravity fixture actor count"), Bank->SceneEnvironment.Actors.Num(), 1)) return false;
    FKyaActorPlacement& Actor = Bank->SceneEnvironment.Actors[0];
    TestEqual(TEXT("Gravity class id"), Actor.ClassId, int64(0x10));
    TestEqual(TEXT("Gravity actor name"), Actor.Name, FString(TEXT("BELL")));
    TestTrue(TEXT("Gravity class payload parses"), Actor.bHasGravityAwareData);
    TestEqual(TEXT("Gravity weight"), Actor.GravityWeightB, 1.25f);
    TestEqual(TEXT("Gravity life maximum is exact"), Actor.GravityLifeMax, int64(16777217));
    TestEqual(TEXT("Gravity scale"), Actor.GravityScale, 2.0f);
    TestEqual(TEXT("Gravity dynamic ext 4"), Actor.GravityDynamicExtField4, 0.5f);
    TestEqual(TEXT("Gravity dynamic ext 8"), Actor.GravityDynamicExtField8, 0.75f);
    TestEqual(TEXT("Gravity fixture leaves no raw class data"), Actor.ClassData.Num(), 0);
    if (!TestEqual(TEXT("Gravity behaviour count"), Actor.Behaviours.Num(), 2)) return false;
    const FKyaActorBehaviour& Fall = Actor.Behaviours[0];
    const FKyaActorBehaviour& Bell = Actor.Behaviours[1];
    TestEqual(TEXT("Gravity fall id"), Fall.Id, 3);
    TestTrue(TEXT("Gravity fall is typed"), Fall.bHasTypedGravityFallBehaviour);
    TestEqual(TEXT("Gravity fall sound"), Fall.GravityFallSoundRef, 31);
    TestEqual(TEXT("Gravity bell id"), Bell.Id, 4);
    TestTrue(TEXT("Gravity bell is typed"), Bell.bHasTypedGravityBellBehaviour);
    TestEqual(TEXT("Gravity bell sound"), Bell.GravityBellSoundRef, 32);
    TestEqual(TEXT("Gravity bell field c"), Bell.GravityBellFieldC, 1.5f);
    if (!TestEqual(TEXT("Gravity bell switch count"), Bell.GravityBellSwitch.Targets.Num(), 1)) return false;
    TestEqual(TEXT("Gravity bell switch target"), Bell.GravityBellSwitch.Targets[0].TargetRef, 41);
    TestEqual(TEXT("Gravity bell switch camera"), Bell.GravityBellSwitch.Camera.CameraIndex, 51);

    UKyaLevelBankExporter* Exporter = NewObject<UKyaLevelBankExporter>();
    TArray<uint8> Plain;
    FMemoryWriter PlainWriter(Plain);
    if (!TestTrue(TEXT("Export unedited gravity fixture"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), PlainWriter, GWarn))) return false;
    TestEqual(TEXT("Unedited gravity export keeps the file size"), Plain.Num(), Input.Num());
    TestTrue(TEXT("Unedited gravity export preserves every byte"), Plain == Input);

    Actor.GravityWeightB = 91.5f;
    Actor.GravityLifeMax = 4000000000ll;
    Actor.GravityScale = 92.5f;
    Actor.Behaviours[0].GravityFallSoundRef = 93;
    Actor.Behaviours[1].GravityBellSoundRef = 94;
    Actor.Behaviours[1].GravityBellFieldC = 95.5f;
    Actor.Behaviours[1].GravityBellSwitch.Targets[0].TargetRef = 96;
    TArray<uint8> Edited;
    FMemoryWriter EditedWriter(Edited);
    if (!TestTrue(TEXT("Export edited gravity fixture"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), EditedWriter, GWarn))) return false;
    TestEqual(TEXT("Edited gravity export keeps the file size"), Edited.Num(), Input.Num());
    const uint8* EditedCursor = Edited.GetData();
    UKyaLevelBank* Reimported = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
        GetTransientPackage(), NAME_None, RF_Transient, nullptr, TEXT("bnk"), EditedCursor,
        Edited.GetData() + Edited.Num(), GWarn));
    if (!TestNotNull(TEXT("Reimport edited gravity fixture"), Reimported)) return false;
    const FKyaActorPlacement& EditedActor = Reimported->SceneEnvironment.Actors[0];
    TestEqual(TEXT("Edited gravity weight round trips"), EditedActor.GravityWeightB, 91.5f);
    TestEqual(TEXT("Edited gravity life round trips"), EditedActor.GravityLifeMax, int64(4000000000ll));
    TestEqual(TEXT("Edited gravity scale round trips"), EditedActor.GravityScale, 92.5f);
    TestEqual(TEXT("Edited gravity fall sound round trips"),
        EditedActor.Behaviours[0].GravityFallSoundRef, 93);
    TestEqual(TEXT("Edited gravity bell sound round trips"),
        EditedActor.Behaviours[1].GravityBellSoundRef, 94);
    TestEqual(TEXT("Edited gravity bell field c round trips"),
        EditedActor.Behaviours[1].GravityBellFieldC, 95.5f);
    TestEqual(TEXT("Edited gravity bell target round trips"),
        EditedActor.Behaviours[1].GravityBellSwitch.Targets[0].TargetRef, 96);

    Actor.GravityLifeMax = -1;
    TArray<uint8> RejectedLife;
    FMemoryWriter RejectedLifeWriter(RejectedLife);
    TestFalse(TEXT("Exporter rejects an out-of-range gravity life maximum"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), RejectedLifeWriter, GWarn));
    Actor.GravityLifeMax = 4000000000ll;

    Actor.Behaviours[1].GravityBellSwitch.Targets.AddDefaulted();
    TArray<uint8> Rejected;
    FMemoryWriter RejectedWriter(Rejected);
    TestFalse(TEXT("Exporter rejects a changed gravity bell switch count"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), RejectedWriter, GWarn));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKyaRuneFixtureRoundTripTest, "Kya.Bank.RuneFixtureRoundTrip",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKyaRuneFixtureRoundTripTest::RunTest(const FString& Parameters)
{
    FKyaSmallBankBuilder Builder;
    const TArray<uint8> Input = Builder.Build(0x51);
    UKyaLevelBankFactory* Factory = NewObject<UKyaLevelBankFactory>();
    const uint8* Cursor = Input.GetData();
    UKyaLevelBank* Bank = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
        GetTransientPackage(), NAME_None, RF_Transient, nullptr, TEXT("bnk"), Cursor,
        Input.GetData() + Input.Num(), GWarn));
    if (!TestNotNull(TEXT("Import rune fixture"), Bank)) return false;
    if (!TestEqual(TEXT("Rune fixture actor count"), Bank->SceneEnvironment.Actors.Num(), 1)) return false;
    FKyaActorPlacement& Actor = Bank->SceneEnvironment.Actors[0];
    TestEqual(TEXT("Rune class id"), Actor.ClassId, int64(0x51));
    TestEqual(TEXT("Rune actor name"), Actor.Name, FString(TEXT("RUNE")));
    TestTrue(TEXT("Rune class payload parses"), Actor.bHasRuneData);
    TestEqual(TEXT("Rune cinematic"), Actor.RuneCinematicId, 1);
    TestEqual(TEXT("Rune trigger distance"), Actor.RuneTriggerDistance, 2.5f);
    TestEqual(TEXT("Rune scenario variable"), Actor.RuneScenVarId, 3);
    TestEqual(TEXT("Rune fx"), Actor.RuneFxId, 4);
    TestEqual(TEXT("Rune material A"), Actor.RuneMaterialIdA, 5);
    TestEqual(TEXT("Rune material B"), Actor.RuneMaterialIdB, 6);
    TestEqual(TEXT("Rune fixture leaves no raw class data"), Actor.ClassData.Num(), 0);
    if (!TestEqual(TEXT("Rune behaviour count"), Actor.Behaviours.Num(), 1)) return false;
    TestEqual(TEXT("Rune behaviour id"), Actor.Behaviours[0].Id, 2);
    TestEqual(TEXT("Rune behaviour is empty"), Actor.Behaviours[0].Data.Num(), 0);

    UKyaLevelBankExporter* Exporter = NewObject<UKyaLevelBankExporter>();
    TArray<uint8> Plain;
    FMemoryWriter PlainWriter(Plain);
    if (!TestTrue(TEXT("Export unedited rune fixture"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), PlainWriter, GWarn))) return false;
    TestEqual(TEXT("Unedited rune export keeps the file size"), Plain.Num(), Input.Num());
    TestTrue(TEXT("Unedited rune export preserves every byte"), Plain == Input);

    Actor.RuneCinematicId = 91;
    Actor.RuneTriggerDistance = 92.5f;
    Actor.RuneScenVarId = 93;
    Actor.RuneFxId = 94;
    Actor.RuneMaterialIdA = 95;
    Actor.RuneMaterialIdB = 96;
    TArray<uint8> Edited;
    FMemoryWriter EditedWriter(Edited);
    if (!TestTrue(TEXT("Export edited rune fixture"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), EditedWriter, GWarn))) return false;
    TestEqual(TEXT("Edited rune export keeps the file size"), Edited.Num(), Input.Num());
    const uint8* EditedCursor = Edited.GetData();
    UKyaLevelBank* Reimported = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
        GetTransientPackage(), NAME_None, RF_Transient, nullptr, TEXT("bnk"), EditedCursor,
        Edited.GetData() + Edited.Num(), GWarn));
    if (!TestNotNull(TEXT("Reimport edited rune fixture"), Reimported)) return false;
    const FKyaActorPlacement& EditedActor = Reimported->SceneEnvironment.Actors[0];
    TestEqual(TEXT("Edited rune cinematic round trips"), EditedActor.RuneCinematicId, 91);
    TestEqual(TEXT("Edited rune trigger round trips"), EditedActor.RuneTriggerDistance, 92.5f);
    TestEqual(TEXT("Edited rune scenario variable round trips"), EditedActor.RuneScenVarId, 93);
    TestEqual(TEXT("Edited rune fx round trips"), EditedActor.RuneFxId, 94);
    TestEqual(TEXT("Edited rune material A round trips"), EditedActor.RuneMaterialIdA, 95);
    TestEqual(TEXT("Edited rune material B round trips"), EditedActor.RuneMaterialIdB, 96);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKyaMickenFixtureRoundTripTest, "Kya.Bank.MickenFixtureRoundTrip",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKyaMickenFixtureRoundTripTest::RunTest(const FString& Parameters)
{
    FKyaSmallBankBuilder Builder;
    const TArray<uint8> Input = Builder.Build(0x07);
    UKyaLevelBankFactory* Factory = NewObject<UKyaLevelBankFactory>();
    const uint8* Cursor = Input.GetData();
    UKyaLevelBank* Bank = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
        GetTransientPackage(), NAME_None, RF_Transient, nullptr, TEXT("bnk"), Cursor,
        Input.GetData() + Input.Num(), GWarn));
    if (!TestNotNull(TEXT("Import micken fixture"), Bank)) return false;
    if (!TestEqual(TEXT("Micken fixture actor count"), Bank->SceneEnvironment.Actors.Num(), 1)) return false;
    FKyaActorPlacement& Actor = Bank->SceneEnvironment.Actors[0];
    TestEqual(TEXT("Micken class id"), Actor.ClassId, int64(0x07));
    TestEqual(TEXT("Micken actor name"), Actor.Name, FString(TEXT("MICKEN")));
    TestTrue(TEXT("Micken class payload parses"), Actor.bHasMickenData);
    TestEqual(TEXT("Micken weight"), Actor.MickenWeightB, 1.25f);
    TestEqual(TEXT("Micken life maximum is exact"), Actor.MickenLifeMax, int64(16777217));
    TestEqual(TEXT("Micken gravity"), Actor.MickenGravityScale, 2.0f);
    TestEqual(TEXT("Micken waypoint"), Actor.MickenWaypointIndex, 11);
    TestEqual(TEXT("Micken actor"), Actor.MickenActorIndex, 12);
    TestEqual(TEXT("Micken field 358"), Actor.MickenField358, 13);
    TestEqual(TEXT("Micken field 35c"), Actor.MickenField35C, 3.5f);
    TestEqual(TEXT("Micken field 360"), Actor.MickenField360, 14);
    TestEqual(TEXT("Micken fixture leaves no raw class data"), Actor.ClassData.Num(), 0);
    if (!TestEqual(TEXT("Micken behaviour count"), Actor.Behaviours.Num(), 2)) return false;
    const FKyaActorBehaviour& Eat = Actor.Behaviours[0];
    TestEqual(TEXT("Micken eat id"), Eat.Id, 4);
    TestTrue(TEXT("Micken eat is typed"), Eat.bHasTypedMickenBehaviour);
    TestEqual(TEXT("Micken eat field 8"), Eat.MickenEatField8, 1.5f);
    TestEqual(TEXT("Micken eat field c"), Eat.MickenEatFieldC, 2.5f);
    TestEqual(TEXT("Micken eat field 10"), Eat.MickenEatField10, 3);
    TestEqual(TEXT("Micken eat path"), Eat.MickenEatPathIndex, 21);
    TestEqual(TEXT("Micken squashed id"), Actor.Behaviours[1].Id, 5);
    TestEqual(TEXT("Micken squashed is empty"), Actor.Behaviours[1].Data.Num(), 0);

    UKyaLevelBankExporter* Exporter = NewObject<UKyaLevelBankExporter>();
    TArray<uint8> Plain;
    FMemoryWriter PlainWriter(Plain);
    if (!TestTrue(TEXT("Export unedited micken fixture"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), PlainWriter, GWarn))) return false;
    TestEqual(TEXT("Unedited micken export keeps the file size"), Plain.Num(), Input.Num());
    TestTrue(TEXT("Unedited micken export preserves every byte"), Plain == Input);

    Actor.MickenWeightB = 91.5f;
    Actor.MickenLifeMax = 4000000000ll;
    Actor.MickenWaypointIndex = 92;
    Actor.MickenField35C = 93.5f;
    Actor.Behaviours[0].MickenEatField8 = 94.5f;
    Actor.Behaviours[0].MickenEatField10 = 95;
    Actor.Behaviours[0].MickenEatPathIndex = 96;
    TArray<uint8> Edited;
    FMemoryWriter EditedWriter(Edited);
    if (!TestTrue(TEXT("Export edited micken fixture"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), EditedWriter, GWarn))) return false;
    TestEqual(TEXT("Edited micken export keeps the file size"), Edited.Num(), Input.Num());
    const uint8* EditedCursor = Edited.GetData();
    UKyaLevelBank* Reimported = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
        GetTransientPackage(), NAME_None, RF_Transient, nullptr, TEXT("bnk"), EditedCursor,
        Edited.GetData() + Edited.Num(), GWarn));
    if (!TestNotNull(TEXT("Reimport edited micken fixture"), Reimported)) return false;
    const FKyaActorPlacement& EditedActor = Reimported->SceneEnvironment.Actors[0];
    TestEqual(TEXT("Edited micken weight round trips"), EditedActor.MickenWeightB, 91.5f);
    TestEqual(TEXT("Edited micken life round trips"), EditedActor.MickenLifeMax, int64(4000000000ll));
    TestEqual(TEXT("Edited micken waypoint round trips"), EditedActor.MickenWaypointIndex, 92);
    TestEqual(TEXT("Edited micken field 35c round trips"), EditedActor.MickenField35C, 93.5f);
    TestEqual(TEXT("Edited micken eat field 8 round trips"),
        EditedActor.Behaviours[0].MickenEatField8, 94.5f);
    TestEqual(TEXT("Edited micken eat field 10 round trips"),
        EditedActor.Behaviours[0].MickenEatField10, 95);
    TestEqual(TEXT("Edited micken eat path round trips"),
        EditedActor.Behaviours[0].MickenEatPathIndex, 96);

    Actor.MickenLifeMax = -1;
    TArray<uint8> RejectedLife;
    FMemoryWriter RejectedLifeWriter(RejectedLife);
    TestFalse(TEXT("Exporter rejects an out-of-range micken life maximum"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), RejectedLifeWriter, GWarn));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKyaBasicBoxFixtureRoundTripTest, "Kya.Bank.BasicBoxFixtureRoundTrip",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKyaBasicBoxFixtureRoundTripTest::RunTest(const FString& Parameters)
{
    FKyaSmallBankBuilder Builder;
    const TArray<uint8> Input = Builder.Build(0x39);
    UKyaLevelBankFactory* Factory = NewObject<UKyaLevelBankFactory>();
    const uint8* Cursor = Input.GetData();
    UKyaLevelBank* Bank = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
        GetTransientPackage(), NAME_None, RF_Transient, nullptr, TEXT("bnk"), Cursor,
        Input.GetData() + Input.Num(), GWarn));
    if (!TestNotNull(TEXT("Import basic box fixture"), Bank)) return false;
    if (!TestEqual(TEXT("Basic box fixture actor count"), Bank->SceneEnvironment.Actors.Num(), 1)) return false;
    FKyaActorPlacement& Actor = Bank->SceneEnvironment.Actors[0];
    TestEqual(TEXT("Basic box class id"), Actor.ClassId, int64(0x39));
    TestEqual(TEXT("Basic box actor name"), Actor.Name, FString(TEXT("BASICBOX")));
    TestTrue(TEXT("Basic box class payload parses"), Actor.bHasBasicBoxData);
    TestEqual(TEXT("Basic box field 160"), Actor.BasicBoxField160, 1);
    TestEqual(TEXT("Basic box life maximum is exact"), Actor.BasicBoxLifeMax, int64(16777217));
    if (!TestEqual(TEXT("Basic box actor ref count"), Actor.BasicBoxActorRefs.Num(), 1)) return false;
    TestEqual(TEXT("Basic box actor ref"), Actor.BasicBoxActorRefs[0], 21);
    TestEqual(TEXT("Basic box vibration 0"), Actor.BasicBoxVibration0, 1.0f);
    TestEqual(TEXT("Basic box vibration 3"), Actor.BasicBoxVibration3, 4.0f);
    TestEqual(TEXT("Basic box field 180"), Actor.BasicBoxField180, 11);
    TestEqual(TEXT("Basic box field 18c"), Actor.BasicBoxField18C, 12);
    TestEqual(TEXT("Basic box field 190"), Actor.BasicBoxField190, 13);
    TestEqual(TEXT("Basic box field 198"), Actor.BasicBoxField198, 14);
    TestEqual(TEXT("Basic box field 194"), Actor.BasicBoxField194, 15);
    if (!TestEqual(TEXT("Basic box switch target count"), Actor.BasicBoxSwitch.Targets.Num(), 1)) return false;
    TestEqual(TEXT("Basic box switch target"), Actor.BasicBoxSwitch.Targets[0].TargetRef, 31);
    TestEqual(TEXT("Basic box switch camera"), Actor.BasicBoxSwitch.Camera.CameraIndex, 41);
    TestEqual(TEXT("Basic box add-on money type"), Actor.BasicBoxAddOn.MoneyType, 2);
    TestEqual(TEXT("Basic box add-on money count"), Actor.BasicBoxAddOn.MoneyCount, 3);
    TestEqual(TEXT("Basic box add-on sub-object mode"), Actor.BasicBoxAddOn.SubObjMode, 1);
    TestEqual(TEXT("Basic box add-on float 0"), Actor.BasicBoxAddOn.SubObjField0, 0.25f);
    TestEqual(TEXT("Basic box add-on float 14"), Actor.BasicBoxAddOn.SubObjField14, 1.5f);
    TestEqual(TEXT("Basic box fixture leaves no raw class data"), Actor.ClassData.Num(), 0);
    if (!TestEqual(TEXT("Basic box behaviour count"), Actor.Behaviours.Num(), 1)) return false;
    TestEqual(TEXT("Basic box behaviour id"), Actor.Behaviours[0].Id, 2);
    TestEqual(TEXT("Basic box behaviour is empty"), Actor.Behaviours[0].Data.Num(), 0);

    UKyaLevelBankExporter* Exporter = NewObject<UKyaLevelBankExporter>();
    TArray<uint8> Plain;
    FMemoryWriter PlainWriter(Plain);
    if (!TestTrue(TEXT("Export unedited basic box fixture"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), PlainWriter, GWarn))) return false;
    TestEqual(TEXT("Unedited basic box export keeps the file size"), Plain.Num(), Input.Num());
    TestTrue(TEXT("Unedited basic box export preserves every byte"), Plain == Input);

    Actor.BasicBoxField160 = 91;
    Actor.BasicBoxLifeMax = 4000000000ll;
    Actor.BasicBoxActorRefs[0] = 92;
    Actor.BasicBoxVibration2 = 93.5f;
    Actor.BasicBoxField18C = 94;
    Actor.BasicBoxSwitch.Targets[0].TargetRef = 95;
    Actor.BasicBoxAddOn.MoneyCount = 96;
    Actor.BasicBoxAddOn.SubObjFieldC = 97.5f;
    TArray<uint8> Edited;
    FMemoryWriter EditedWriter(Edited);
    if (!TestTrue(TEXT("Export edited basic box fixture"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), EditedWriter, GWarn))) return false;
    TestEqual(TEXT("Edited basic box export keeps the file size"), Edited.Num(), Input.Num());
    const uint8* EditedCursor = Edited.GetData();
    UKyaLevelBank* Reimported = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
        GetTransientPackage(), NAME_None, RF_Transient, nullptr, TEXT("bnk"), EditedCursor,
        Edited.GetData() + Edited.Num(), GWarn));
    if (!TestNotNull(TEXT("Reimport edited basic box fixture"), Reimported)) return false;
    const FKyaActorPlacement& EditedActor = Reimported->SceneEnvironment.Actors[0];
    TestEqual(TEXT("Edited basic box field 160 round trips"), EditedActor.BasicBoxField160, 91);
    TestEqual(TEXT("Edited basic box life round trips"), EditedActor.BasicBoxLifeMax, int64(4000000000ll));
    TestEqual(TEXT("Edited basic box actor ref round trips"), EditedActor.BasicBoxActorRefs[0], 92);
    TestEqual(TEXT("Edited basic box vibration round trips"), EditedActor.BasicBoxVibration2, 93.5f);
    TestEqual(TEXT("Edited basic box field 18c round trips"), EditedActor.BasicBoxField18C, 94);
    TestEqual(TEXT("Edited basic box target round trips"),
        EditedActor.BasicBoxSwitch.Targets[0].TargetRef, 95);
    TestEqual(TEXT("Edited basic box money round trips"), EditedActor.BasicBoxAddOn.MoneyCount, 96);
    TestEqual(TEXT("Edited basic box add-on float round trips"),
        EditedActor.BasicBoxAddOn.SubObjFieldC, 97.5f);

    Actor.BasicBoxActorRefs.Add(99);
    TArray<uint8> Rejected;
    FMemoryWriter RejectedWriter(Rejected);
    TestFalse(TEXT("Exporter rejects a changed basic box actor-ref count"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), RejectedWriter, GWarn));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKyaNativCmdFixtureRoundTripTest, "Kya.Bank.NativCmdFixtureRoundTrip",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKyaNativCmdFixtureRoundTripTest::RunTest(const FString& Parameters)
{
    FKyaSmallBankBuilder Builder;
    const TArray<uint8> Input = Builder.Build(0x3c);
    UKyaLevelBankFactory* Factory = NewObject<UKyaLevelBankFactory>();
    const uint8* Cursor = Input.GetData();
    UKyaLevelBank* Bank = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
        GetTransientPackage(), NAME_None, RF_Transient, nullptr, TEXT("bnk"), Cursor,
        Input.GetData() + Input.Num(), GWarn));
    if (!TestNotNull(TEXT("Import nativ command fixture"), Bank)) return false;
    if (!TestEqual(TEXT("NativCmd fixture actor count"), Bank->SceneEnvironment.Actors.Num(), 1)) return false;
    FKyaActorPlacement& Actor = Bank->SceneEnvironment.Actors[0];
    TestEqual(TEXT("NativCmd class id"), Actor.ClassId, int64(0x3c));
    TestEqual(TEXT("NativCmd actor name"), Actor.Name, FString(TEXT("NATIVCMD")));
    TestTrue(TEXT("NativCmd class payload parses"), Actor.bHasNativCmdData);
    if (!TestEqual(TEXT("NativCmd actor ref count"), Actor.NativCmdActorRefs.Num(), 1)) return false;
    TestEqual(TEXT("NativCmd actor ref"), Actor.NativCmdActorRefs[0], 21);
    TestEqual(TEXT("NativCmd field 164"), Actor.NativCmdField164, 11);
    if (!TestEqual(TEXT("NativCmd add-on count"), Actor.NativCmdAddOns.Num(), 4)) return false;
    if (!TestEqual(TEXT("NativCmd A sub-object count"), Actor.NativCmdAddOns[0].SubObjs.Num(), 1)) return false;
    TestEqual(TEXT("NativCmd A field 0"), Actor.NativCmdAddOns[0].SubObjs[0].Field0, 22);
    if (!TestEqual(TEXT("NativCmd A cinematic count"), Actor.NativCmdAddOns[0].SubObjs[0].CinematicIds.Num(), 1)) return false;
    TestEqual(TEXT("NativCmd A cinematic"), Actor.NativCmdAddOns[0].SubObjs[0].CinematicIds[0], 23);
    TestEqual(TEXT("NativCmd B sub-object count"), Actor.NativCmdAddOns[1].SubObjs.Num(), 0);
    TestEqual(TEXT("NativCmd C field 0"), Actor.NativCmdAddOns[2].SubObjs[0].Field0, 24);
    TestEqual(TEXT("NativCmd C cinematic"), Actor.NativCmdAddOns[2].SubObjs[0].CinematicIds[0], 25);
    TestEqual(TEXT("NativCmd fixture leaves no raw class data"), Actor.ClassData.Num(), 0);
    if (!TestEqual(TEXT("NativCmd behaviour count"), Actor.Behaviours.Num(), 1)) return false;
    TestEqual(TEXT("NativCmd behaviour id"), Actor.Behaviours[0].Id, 2);
    TestEqual(TEXT("NativCmd behaviour is empty"), Actor.Behaviours[0].Data.Num(), 0);

    UKyaLevelBankExporter* Exporter = NewObject<UKyaLevelBankExporter>();
    TArray<uint8> Plain;
    FMemoryWriter PlainWriter(Plain);
    if (!TestTrue(TEXT("Export unedited nativ command fixture"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), PlainWriter, GWarn))) return false;
    TestEqual(TEXT("Unedited nativ command export keeps the file size"), Plain.Num(), Input.Num());
    TestTrue(TEXT("Unedited nativ command export preserves every byte"), Plain == Input);

    Actor.NativCmdActorRefs[0] = 91;
    Actor.NativCmdField164 = 92;
    Actor.NativCmdAddOns[0].SubObjs[0].Field0 = 93;
    Actor.NativCmdAddOns[0].SubObjs[0].CinematicIds[0] = 94;
    Actor.NativCmdAddOns[2].SubObjs[0].CinematicIds[0] = 95;
    TArray<uint8> Edited;
    FMemoryWriter EditedWriter(Edited);
    if (!TestTrue(TEXT("Export edited nativ command fixture"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), EditedWriter, GWarn))) return false;
    TestEqual(TEXT("Edited nativ command export keeps the file size"), Edited.Num(), Input.Num());
    const uint8* EditedCursor = Edited.GetData();
    UKyaLevelBank* Reimported = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
        GetTransientPackage(), NAME_None, RF_Transient, nullptr, TEXT("bnk"), EditedCursor,
        Edited.GetData() + Edited.Num(), GWarn));
    if (!TestNotNull(TEXT("Reimport edited nativ command fixture"), Reimported)) return false;
    const FKyaActorPlacement& EditedActor = Reimported->SceneEnvironment.Actors[0];
    TestEqual(TEXT("Edited nativ command ref round trips"), EditedActor.NativCmdActorRefs[0], 91);
    TestEqual(TEXT("Edited nativ command field 164 round trips"), EditedActor.NativCmdField164, 92);
    TestEqual(TEXT("Edited nativ command field 0 round trips"),
        EditedActor.NativCmdAddOns[0].SubObjs[0].Field0, 93);
    TestEqual(TEXT("Edited nativ command A cinematic round trips"),
        EditedActor.NativCmdAddOns[0].SubObjs[0].CinematicIds[0], 94);
    TestEqual(TEXT("Edited nativ command C cinematic round trips"),
        EditedActor.NativCmdAddOns[2].SubObjs[0].CinematicIds[0], 95);

    Actor.NativCmdAddOns[0].SubObjs[0].CinematicIds.Add(99);
    TArray<uint8> Rejected;
    FMemoryWriter RejectedWriter(Rejected);
    TestFalse(TEXT("Exporter rejects a changed nativ command cinematic count"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), RejectedWriter, GWarn));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKyaEventGeneratorFixtureRoundTripTest, "Kya.Bank.EventGeneratorFixtureRoundTrip",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKyaEventGeneratorFixtureRoundTripTest::RunTest(const FString& Parameters)
{
    FKyaSmallBankBuilder Builder;
    const TArray<uint8> Input = Builder.Build(0x3b);
    UKyaLevelBankFactory* Factory = NewObject<UKyaLevelBankFactory>();
    const uint8* Cursor = Input.GetData();
    UKyaLevelBank* Bank = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
        GetTransientPackage(), NAME_None, RF_Transient, nullptr, TEXT("bnk"), Cursor,
        Input.GetData() + Input.Num(), GWarn));
    if (!TestNotNull(TEXT("Import event generator fixture"), Bank)) return false;
    if (!TestEqual(TEXT("Event generator fixture actor count"), Bank->SceneEnvironment.Actors.Num(), 1)) return false;
    FKyaActorPlacement& Actor = Bank->SceneEnvironment.Actors[0];
    TestEqual(TEXT("Event generator class id"), Actor.ClassId, int64(0x3b));
    TestEqual(TEXT("Event generator actor name"), Actor.Name, FString(TEXT("EVENTGENERATOR")));
    TestTrue(TEXT("Event generator class payload parses"), Actor.bHasEventGeneratorData);
    TestEqual(TEXT("Event generator enabled flag"), Actor.EventGeneratorEnabled, 1);
    TestEqual(TEXT("Event generator zone reference"), Actor.EventGeneratorZoneRef, 2);
    if (!TestEqual(TEXT("Event generator raw manager bytes"), Actor.ClassData.Num(), 8)) return false;
    if (!TestEqual(TEXT("Event generator behaviour count"), Actor.Behaviours.Num(), 1)) return false;
    const FKyaActorBehaviour& Behaviour = Actor.Behaviours[0];
    TestEqual(TEXT("Event generator behaviour id"), Behaviour.Id, 2);
    TestTrue(TEXT("Event generator behaviour is typed"), Behaviour.bHasTypedEventGeneratorBehaviour);
    TestEqual(TEXT("Event generator behaviour field c"), Behaviour.EventGeneratorFieldC, 7);
    TestEqual(TEXT("Event generator behaviour field 10"), Behaviour.EventGeneratorField10, 1.5f);
    TestEqual(TEXT("Event generator behaviour keeps manager bytes"), Behaviour.Data.Num(), 20);

    UKyaLevelBankExporter* Exporter = NewObject<UKyaLevelBankExporter>();
    TArray<uint8> Plain;
    FMemoryWriter PlainWriter(Plain);
    if (!TestTrue(TEXT("Export unedited event generator fixture"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), PlainWriter, GWarn))) return false;
    TestEqual(TEXT("Unedited event generator export keeps the file size"), Plain.Num(), Input.Num());
    TestTrue(TEXT("Unedited event generator export preserves every byte"), Plain == Input);

    Actor.EventGeneratorEnabled = 91;
    Actor.EventGeneratorZoneRef = 92;
    Actor.Behaviours[0].EventGeneratorFieldC = 93;
    Actor.Behaviours[0].EventGeneratorField10 = 94.5f;
    TArray<uint8> Edited;
    FMemoryWriter EditedWriter(Edited);
    if (!TestTrue(TEXT("Export edited event generator fixture"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), EditedWriter, GWarn))) return false;
    TestEqual(TEXT("Edited event generator export keeps the file size"), Edited.Num(), Input.Num());
    const uint8* EditedCursor = Edited.GetData();
    UKyaLevelBank* Reimported = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
        GetTransientPackage(), NAME_None, RF_Transient, nullptr, TEXT("bnk"), EditedCursor,
        Edited.GetData() + Edited.Num(), GWarn));
    if (!TestNotNull(TEXT("Reimport edited event generator fixture"), Reimported)) return false;
    const FKyaActorPlacement& EditedActor = Reimported->SceneEnvironment.Actors[0];
    TestEqual(TEXT("Edited event generator flag round trips"), EditedActor.EventGeneratorEnabled, 91);
    TestEqual(TEXT("Edited event generator zone round trips"), EditedActor.EventGeneratorZoneRef, 92);
    TestEqual(TEXT("Edited event generator behaviour field c round trips"),
        EditedActor.Behaviours[0].EventGeneratorFieldC, 93);
    TestEqual(TEXT("Edited event generator behaviour field 10 round trips"),
        EditedActor.Behaviours[0].EventGeneratorField10, 94.5f);
    TestTrue(TEXT("Edited event generator keeps raw manager bytes"),
        EditedActor.ClassData == Actor.ClassData);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKyaExplosiveDistributorFixtureRoundTripTest, "Kya.Bank.ExplosiveDistributorFixtureRoundTrip",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKyaExplosiveDistributorFixtureRoundTripTest::RunTest(const FString& Parameters)
{
    FKyaSmallBankBuilder Builder;
    const TArray<uint8> Input = Builder.Build(0x1d);
    UKyaLevelBankFactory* Factory = NewObject<UKyaLevelBankFactory>();
    const uint8* Cursor = Input.GetData();
    UKyaLevelBank* Bank = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
        GetTransientPackage(), NAME_None, RF_Transient, nullptr, TEXT("bnk"), Cursor,
        Input.GetData() + Input.Num(), GWarn));
    if (!TestNotNull(TEXT("Import explosive distributor fixture"), Bank)) return false;
    if (!TestEqual(TEXT("Explosive distributor fixture actor count"), Bank->SceneEnvironment.Actors.Num(), 1)) return false;
    FKyaActorPlacement& Actor = Bank->SceneEnvironment.Actors[0];
    TestEqual(TEXT("Explosive distributor class id"), Actor.ClassId, int64(0x1d));
    TestEqual(TEXT("Explosive distributor actor name"), Actor.Name, FString(TEXT("EXPLOSIVEDIST")));
    TestTrue(TEXT("Explosive distributor class payload parses"), Actor.bHasExplosiveDistributorData);
    TestEqual(TEXT("Explosive distributor radius"), Actor.ExplosiveDistributorRadius, 5.0f);
    TestEqual(TEXT("Explosive distributor angle"), Actor.ExplosiveDistributorAngle, 1.25f);
    if (!TestEqual(TEXT("Explosive distributor projectile count"), Actor.ExplosiveDistributorProjectiles.Num(), 2)) return false;
    TestEqual(TEXT("Explosive distributor projectile 0 actor"), Actor.ExplosiveDistributorProjectiles[0].ActorRef, 31);
    TestEqual(TEXT("Explosive distributor projectile 0 direction y"), float(Actor.ExplosiveDistributorProjectiles[0].Direction.Y), 2.0f);
    TestEqual(TEXT("Explosive distributor projectile 0 force"), Actor.ExplosiveDistributorProjectiles[0].EjectForce, 4.5f);
    TestEqual(TEXT("Explosive distributor projectile 1 actor"), Actor.ExplosiveDistributorProjectiles[1].ActorRef, 32);
    TestEqual(TEXT("Explosive distributor projectile 1 force"), Actor.ExplosiveDistributorProjectiles[1].EjectForce, 6.5f);
    TestEqual(TEXT("Explosive distributor camera index"), Actor.ExplosiveDistributorCamera.CameraIndex, 41);
    TestEqual(TEXT("Explosive distributor camera flags"), Actor.ExplosiveDistributorCamera.Flags, 2);
    TestEqual(TEXT("Explosive distributor camera actor"), Actor.ExplosiveDistributorCamera.ActorRef, 33);
    TestEqual(TEXT("Explosive distributor camera zone"), Actor.ExplosiveDistributorCamera.ZoneRef, 34);
    TestEqual(TEXT("Explosive distributor camera field 1c"), Actor.ExplosiveDistributorCamera.Field1C, 4.5f);
    TestEqual(TEXT("Explosive distributor fixture leaves no raw class data"), Actor.ClassData.Num(), 0);
    if (!TestEqual(TEXT("Explosive distributor behaviour count"), Actor.Behaviours.Num(), 1)) return false;
    TestEqual(TEXT("Explosive distributor behaviour id"), Actor.Behaviours[0].Id, 2);
    TestEqual(TEXT("Explosive distributor behaviour is empty"), Actor.Behaviours[0].Data.Num(), 0);

    UKyaLevelBankExporter* Exporter = NewObject<UKyaLevelBankExporter>();
    TArray<uint8> Plain;
    FMemoryWriter PlainWriter(Plain);
    if (!TestTrue(TEXT("Export unedited explosive distributor fixture"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), PlainWriter, GWarn))) return false;
    TestEqual(TEXT("Unedited explosive distributor export keeps the file size"), Plain.Num(), Input.Num());
    TestTrue(TEXT("Unedited explosive distributor export preserves every byte"), Plain == Input);

    Actor.ExplosiveDistributorRadius = 91.5f;
    Actor.ExplosiveDistributorAngle = 92.5f;
    Actor.ExplosiveDistributorProjectiles[0].ActorRef = 93;
    Actor.ExplosiveDistributorProjectiles[0].Direction.X = 94.5f;
    Actor.ExplosiveDistributorProjectiles[1].EjectForce = 95.5f;
    Actor.ExplosiveDistributorCamera.CameraIndex = 96;
    Actor.ExplosiveDistributorCamera.Field1C = 97.5f;
    TArray<uint8> Edited;
    FMemoryWriter EditedWriter(Edited);
    if (!TestTrue(TEXT("Export edited explosive distributor fixture"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), EditedWriter, GWarn))) return false;
    TestEqual(TEXT("Edited explosive distributor export keeps the file size"), Edited.Num(), Input.Num());
    const uint8* EditedCursor = Edited.GetData();
    UKyaLevelBank* Reimported = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
        GetTransientPackage(), NAME_None, RF_Transient, nullptr, TEXT("bnk"), EditedCursor,
        Edited.GetData() + Edited.Num(), GWarn));
    if (!TestNotNull(TEXT("Reimport edited explosive distributor fixture"), Reimported)) return false;
    const FKyaActorPlacement& EditedActor = Reimported->SceneEnvironment.Actors[0];
    TestEqual(TEXT("Edited explosive distributor radius round trips"), EditedActor.ExplosiveDistributorRadius, 91.5f);
    TestEqual(TEXT("Edited explosive distributor angle round trips"), EditedActor.ExplosiveDistributorAngle, 92.5f);
    TestEqual(TEXT("Edited explosive distributor projectile round trips"),
        EditedActor.ExplosiveDistributorProjectiles[0].ActorRef, 93);
    TestEqual(TEXT("Edited explosive distributor direction round trips"),
        float(EditedActor.ExplosiveDistributorProjectiles[0].Direction.X), 94.5f);
    TestEqual(TEXT("Edited explosive distributor force round trips"),
        EditedActor.ExplosiveDistributorProjectiles[1].EjectForce, 95.5f);
    TestEqual(TEXT("Edited explosive distributor camera round trips"),
        EditedActor.ExplosiveDistributorCamera.CameraIndex, 96);
    TestEqual(TEXT("Edited explosive distributor camera field round trips"),
        EditedActor.ExplosiveDistributorCamera.Field1C, 97.5f);

    Actor.ExplosiveDistributorProjectiles.AddDefaulted();
    TArray<uint8> Rejected;
    FMemoryWriter RejectedWriter(Rejected);
    TestFalse(TEXT("Exporter rejects a changed explosive distributor projectile count"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), RejectedWriter, GWarn));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKyaShockerFixtureRoundTripTest, "Kya.Bank.ShockerFixtureRoundTrip",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKyaShockerFixtureRoundTripTest::RunTest(const FString& Parameters)
{
    FKyaSmallBankBuilder Builder;
    const TArray<uint8> Input = Builder.Build(0x4b);
    UKyaLevelBankFactory* Factory = NewObject<UKyaLevelBankFactory>();
    const uint8* Cursor = Input.GetData();
    UKyaLevelBank* Bank = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
        GetTransientPackage(), NAME_None, RF_Transient, nullptr, TEXT("bnk"), Cursor,
        Input.GetData() + Input.Num(), GWarn));
    if (!TestNotNull(TEXT("Import shocker fixture"), Bank)) return false;
    if (!TestEqual(TEXT("Shocker fixture actor count"), Bank->SceneEnvironment.Actors.Num(), 1)) return false;
    FKyaActorPlacement& Actor = Bank->SceneEnvironment.Actors[0];
    TestEqual(TEXT("Shocker class id"), Actor.ClassId, int64(0x4b));
    TestEqual(TEXT("Shocker actor name"), Actor.Name, FString(TEXT("SHOCKER")));
    TestTrue(TEXT("Shocker class payload parses"), Actor.bHasShockerData);
    TestEqual(TEXT("Shocker weight"), Actor.ShockerField350, 1.0f);
    TestEqual(TEXT("Shocker field 350 base"), Actor.ShockerField350, 1.0f);
    TestEqual(TEXT("Shocker zone inner"), Actor.ShockerZoneInner, 11);
    TestEqual(TEXT("Shocker zone outer"), Actor.ShockerZoneOuter, 12);
    if (!TestEqual(TEXT("Shocker switch target count"), Actor.ShockerSwitch.Targets.Num(), 1)) return false;
    TestEqual(TEXT("Shocker switch target"), Actor.ShockerSwitch.Targets[0].TargetRef, 21);
    TestEqual(TEXT("Shocker switch camera"), Actor.ShockerSwitch.Camera.CameraIndex, 31);
    TestEqual(TEXT("Shocker field 380"), Actor.ShockerField380, 41);
    TestEqual(TEXT("Shocker field 384"), Actor.ShockerField384, 42);
    TestEqual(TEXT("Shocker field 398"), Actor.ShockerField398, 47);
    TestEqual(TEXT("Shocker bone"), Actor.ShockerBoneId, 48);
    TestEqual(TEXT("Shocker add-on money type"), Actor.ShockerAddOn.MoneyType, 2);
    TestEqual(TEXT("Shocker add-on mode"), Actor.ShockerAddOn.SubObjMode, 1);
    TestEqual(TEXT("Shocker add-on float 0"), Actor.ShockerAddOn.SubObjField0, 0.25f);
    TestEqual(TEXT("Shocker fixture leaves no raw class data"), Actor.ClassData.Num(), 0);
    if (!TestEqual(TEXT("Shocker behaviour count"), Actor.Behaviours.Num(), 1)) return false;
    const FKyaActorBehaviour& Wave = Actor.Behaviours[0];
    TestEqual(TEXT("Shocker behaviour id"), Wave.Id, 3);
    TestTrue(TEXT("Shocker behaviour is typed"), Wave.bHasTypedShockerBehaviour);
    TestEqual(TEXT("Shocker wave field 8"), Wave.ShockerWave.Field8, 1.0f);
    TestEqual(TEXT("Shocker wave field 1c"), Wave.ShockerWave.Field1C, 6.0f);
    TestEqual(TEXT("Shocker wave material"), Wave.ShockerWave.MaterialId, 61);
    TestEqual(TEXT("Shocker wave field 24"), Wave.ShockerWave.Field24, 62);

    UKyaLevelBankExporter* Exporter = NewObject<UKyaLevelBankExporter>();
    TArray<uint8> Plain;
    FMemoryWriter PlainWriter(Plain);
    if (!TestTrue(TEXT("Export unedited shocker fixture"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), PlainWriter, GWarn))) return false;
    TestEqual(TEXT("Unedited shocker export keeps the file size"), Plain.Num(), Input.Num());
    TestTrue(TEXT("Unedited shocker export preserves every byte"), Plain == Input);

    Actor.ShockerField350 = 91.5f;
    Actor.ShockerZoneInner = 92;
    Actor.ShockerSwitch.Targets[0].TargetRef = 93;
    Actor.ShockerSwitch.Camera.CameraIndex = 94;
    Actor.ShockerField398 = 95;
    Actor.ShockerAddOn.MoneyCount = 96;
    Actor.Behaviours[0].ShockerWave.Field10 = 97.5f;
    Actor.Behaviours[0].ShockerWave.MaterialId = 98;
    TArray<uint8> Edited;
    FMemoryWriter EditedWriter(Edited);
    if (!TestTrue(TEXT("Export edited shocker fixture"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), EditedWriter, GWarn))) return false;
    TestEqual(TEXT("Edited shocker export keeps the file size"), Edited.Num(), Input.Num());
    const uint8* EditedCursor = Edited.GetData();
    UKyaLevelBank* Reimported = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
        GetTransientPackage(), NAME_None, RF_Transient, nullptr, TEXT("bnk"), EditedCursor,
        Edited.GetData() + Edited.Num(), GWarn));
    if (!TestNotNull(TEXT("Reimport edited shocker fixture"), Reimported)) return false;
    const FKyaActorPlacement& EditedActor = Reimported->SceneEnvironment.Actors[0];
    TestEqual(TEXT("Edited shocker field 350 round trips"), EditedActor.ShockerField350, 91.5f);
    TestEqual(TEXT("Edited shocker zone round trips"), EditedActor.ShockerZoneInner, 92);
    TestEqual(TEXT("Edited shocker switch target round trips"),
        EditedActor.ShockerSwitch.Targets[0].TargetRef, 93);
    TestEqual(TEXT("Edited shocker camera round trips"),
        EditedActor.ShockerSwitch.Camera.CameraIndex, 94);
    TestEqual(TEXT("Edited shocker field 398 round trips"), EditedActor.ShockerField398, 95);
    TestEqual(TEXT("Edited shocker money round trips"), EditedActor.ShockerAddOn.MoneyCount, 96);
    TestEqual(TEXT("Edited shocker wave float round trips"),
        EditedActor.Behaviours[0].ShockerWave.Field10, 97.5f);
    TestEqual(TEXT("Edited shocker wave material round trips"),
        EditedActor.Behaviours[0].ShockerWave.MaterialId, 98);

    Actor.ShockerSwitch.Targets.AddDefaulted();
    TArray<uint8> Rejected;
    FMemoryWriter RejectedWriter(Rejected);
    TestFalse(TEXT("Exporter rejects a changed shocker switch count"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), RejectedWriter, GWarn));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKyaNativShopFixtureRoundTripTest, "Kya.Bank.NativShopFixtureRoundTrip",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKyaNativShopFixtureRoundTripTest::RunTest(const FString& Parameters)
{
    FKyaSmallBankBuilder Builder;
    const TArray<uint8> Input = Builder.Build(0x34);
    UKyaLevelBankFactory* Factory = NewObject<UKyaLevelBankFactory>();
    const uint8* Cursor = Input.GetData();
    UKyaLevelBank* Bank = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
        GetTransientPackage(), NAME_None, RF_Transient, nullptr, TEXT("bnk"), Cursor,
        Input.GetData() + Input.Num(), GWarn));
    if (!TestNotNull(TEXT("Import Nativ shop fixture"), Bank)) return false;
    if (!TestEqual(TEXT("Nativ shop fixture actor count"), Bank->SceneEnvironment.Actors.Num(), 1)) return false;
    FKyaActorPlacement& Actor = Bank->SceneEnvironment.Actors[0];
    TestEqual(TEXT("Nativ shop class id"), Actor.ClassId, int64(0x34));
    TestEqual(TEXT("Nativ shop actor name"), Actor.Name, FString(TEXT("NATIV_SHOP")));
    TestTrue(TEXT("Nativ shop class payload parses"), Actor.bHasNativShopData);
    TestEqual(TEXT("Nativ shop radius"), Actor.NativShopRadius, 2.5f);
    TestEqual(TEXT("Nativ shop class name"), Actor.NativShopName, FString(TEXT("NATIV_SHOP")));
    if (!TestEqual(TEXT("Nativ shop actor ref count"), Actor.NativShopActorRefs.Num(), 2)) return false;
    TestEqual(TEXT("Nativ shop actor ref 0"), Actor.NativShopActorRefs[0], 31);
    TestEqual(TEXT("Nativ shop actor ref 1"), Actor.NativShopActorRefs[1], 32);
    TestEqual(TEXT("Nativ shop episode link"), Actor.NativShopEpisodeThresholdLink, 41);
    TestEqual(TEXT("Nativ shop field 174"), Actor.NativShopField174, 42);
    TestEqual(TEXT("Nativ shop actor reference"), Actor.NativShopActorRef, 43);
    TestEqual(TEXT("Nativ shop fixture leaves no raw class data"), Actor.ClassData.Num(), 0);
    if (!TestEqual(TEXT("Nativ shop behaviour count"), Actor.Behaviours.Num(), 1)) return false;
    const FKyaActorBehaviour& Sell = Actor.Behaviours[0];
    TestEqual(TEXT("Nativ shop behaviour id"), Sell.Id, 2);
    TestTrue(TEXT("Nativ shop behaviour is typed"), Sell.bHasTypedNativShopBehaviour);
    TestEqual(TEXT("Nativ shop camera ref"), Sell.NativShopCameraRef, 71);
    if (!TestEqual(TEXT("Nativ shop sub-object ref count"), Sell.NativShopSubObjActorRefs.Num(), 5)) return false;
    TestEqual(TEXT("Nativ shop sub-object ref 0"), Sell.NativShopSubObjActorRefs[0], 72);
    TestEqual(TEXT("Nativ shop sub-object ref 4"), Sell.NativShopSubObjActorRefs[4], 76);
    TestEqual(TEXT("Nativ shop path index"), Sell.NativShopPathIndex, 77);
    TestEqual(TEXT("Nativ shop material"), Sell.NativShopMaterialId, 78);
    TestEqual(TEXT("Nativ shop music"), Sell.NativShopMusicIndex, 79);
    TestEqual(TEXT("Nativ shop zone"), Sell.NativShopZoneRef, 80);
    TestEqual(TEXT("Nativ shop field c4"), Sell.NativShopFieldC4, 1.5f);
    TestEqual(TEXT("Nativ shop field c8"), Sell.NativShopFieldC8, 2.5f);
    TestEqual(TEXT("Nativ shop field cc"), Sell.NativShopFieldCC, 3.5f);

    UKyaLevelBankExporter* Exporter = NewObject<UKyaLevelBankExporter>();
    TArray<uint8> Plain;
    FMemoryWriter PlainWriter(Plain);
    if (!TestTrue(TEXT("Export unedited Nativ shop fixture"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), PlainWriter, GWarn))) return false;
    TestEqual(TEXT("Unedited Nativ shop export keeps the file size"), Plain.Num(), Input.Num());
    TestTrue(TEXT("Unedited Nativ shop export preserves every byte"), Plain == Input);

    Actor.NativShopRadius = 91.5f;
    Actor.NativShopActorRefs[0] = 92;
    Actor.NativShopEpisodeThresholdLink = 93;
    Actor.NativShopField174 = 94;
    Actor.NativShopActorRef = 95;
    Actor.Behaviours[0].NativShopCameraRef = 96;
    Actor.Behaviours[0].NativShopSubObjActorRefs[3] = 97;
    Actor.Behaviours[0].NativShopMaterialId = 98;
    Actor.Behaviours[0].NativShopFieldCC = 99.5f;
    TArray<uint8> Edited;
    FMemoryWriter EditedWriter(Edited);
    if (!TestTrue(TEXT("Export edited Nativ shop fixture"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), EditedWriter, GWarn))) return false;
    TestEqual(TEXT("Edited Nativ shop export keeps the file size"), Edited.Num(), Input.Num());
    const uint8* EditedCursor = Edited.GetData();
    UKyaLevelBank* Reimported = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
        GetTransientPackage(), NAME_None, RF_Transient, nullptr, TEXT("bnk"), EditedCursor,
        Edited.GetData() + Edited.Num(), GWarn));
    if (!TestNotNull(TEXT("Reimport edited Nativ shop fixture"), Reimported)) return false;
    const FKyaActorPlacement& EditedActor = Reimported->SceneEnvironment.Actors[0];
    TestEqual(TEXT("Edited Nativ shop radius round trips"), EditedActor.NativShopRadius, 91.5f);
    TestEqual(TEXT("Edited Nativ shop actor ref round trips"), EditedActor.NativShopActorRefs[0], 92);
    TestEqual(TEXT("Edited Nativ shop episode link round trips"), EditedActor.NativShopEpisodeThresholdLink, 93);
    TestEqual(TEXT("Edited Nativ shop field 174 round trips"), EditedActor.NativShopField174, 94);
    TestEqual(TEXT("Edited Nativ shop actor reference round trips"), EditedActor.NativShopActorRef, 95);
    TestEqual(TEXT("Edited Nativ shop camera round trips"), EditedActor.Behaviours[0].NativShopCameraRef, 96);
    TestEqual(TEXT("Edited Nativ shop sub-object ref round trips"),
        EditedActor.Behaviours[0].NativShopSubObjActorRefs[3], 97);
    TestEqual(TEXT("Edited Nativ shop material round trips"), EditedActor.Behaviours[0].NativShopMaterialId, 98);
    TestEqual(TEXT("Edited Nativ shop field cc round trips"), EditedActor.Behaviours[0].NativShopFieldCC, 99.5f);

    Actor.NativShopActorRefs.Add(99);
    TArray<uint8> Rejected;
    FMemoryWriter RejectedWriter(Rejected);
    TestFalse(TEXT("Exporter rejects a changed Nativ shop actor count"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), RejectedWriter, GWarn));
    Actor.NativShopActorRefs.RemoveAt(2);
    Actor.NativShopName = TEXT("THIS_NAME_IS_FAR_TOO_LONG_FOR_THE_SPAN");
    TArray<uint8> RejectedName;
    FMemoryWriter RejectedNameWriter(RejectedName);
    TestFalse(TEXT("Exporter rejects a Nativ shop name that outgrows its span"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), RejectedNameWriter, GWarn));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKyaPunchingBallFixtureRoundTripTest, "Kya.Bank.PunchingBallFixtureRoundTrip",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKyaPunchingBallFixtureRoundTripTest::RunTest(const FString& Parameters)
{
    FKyaSmallBankBuilder Builder;
    const TArray<uint8> Input = Builder.Build(0x30);
    UKyaLevelBankFactory* Factory = NewObject<UKyaLevelBankFactory>();
    const uint8* Cursor = Input.GetData();
    UKyaLevelBank* Bank = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
        GetTransientPackage(), NAME_None, RF_Transient, nullptr, TEXT("bnk"), Cursor,
        Input.GetData() + Input.Num(), GWarn));
    if (!TestNotNull(TEXT("Import punching ball fixture"), Bank)) return false;
    if (!TestEqual(TEXT("Punching ball fixture actor count"), Bank->SceneEnvironment.Actors.Num(), 1)) return false;
    FKyaActorPlacement& Actor = Bank->SceneEnvironment.Actors[0];
    TestEqual(TEXT("Punching ball class id"), Actor.ClassId, int64(0x30));
    TestEqual(TEXT("Punching ball actor name"), Actor.Name, FString(TEXT("PUNCHINGBALL")));
    TestTrue(TEXT("Punching ball class payload parses"), Actor.bHasPunchingBallData);
    TestEqual(TEXT("Punching ball field a80"), Actor.PunchingBallFieldA80, 10.0f);
    TestEqual(TEXT("Punching ball field a84"), Actor.PunchingBallFieldA84, 20.0f);
    TestEqual(TEXT("Punching ball field a88"), Actor.PunchingBallFieldA88, 30.0f);
    TestEqual(TEXT("Punching ball field a8c"), Actor.PunchingBallFieldA8C, 40.0f);
    if (!TestEqual(TEXT("Punching ball trap entry count"), Actor.PunchingBallTrapEntries.Num(), 1)) return false;
    const FKyaPunchingBallTrapEntry& Trap = Actor.PunchingBallTrapEntries[0];
    TestEqual(TEXT("Punching ball trap target"), Trap.TargetRef, 51);
    TestEqual(TEXT("Punching ball trap cutscene"), Trap.CutsceneId, 52);
    TestEqual(TEXT("Punching ball trap flags"), Trap.Flags, 53);
    TestEqual(TEXT("Punching ball trap message"), Trap.MessageId, 54);
    TestEqual(TEXT("Punching ball trap message flags"), Trap.MessageFlags, 55);
    TestEqual(TEXT("Punching ball trap field 14"), Trap.Field14, 56);
    TestEqual(TEXT("Punching ball trap field 18"), Trap.Field18, 57);
    TestEqual(TEXT("Punching ball trap camera index"), Trap.Camera.CameraIndex, 61);
    TestEqual(TEXT("Punching ball trap camera flags"), Trap.Camera.Flags, 62);
    TestEqual(TEXT("Punching ball trap camera field 8"), Trap.Camera.Field8, 1.5f);
    TestEqual(TEXT("Punching ball trap camera actor"), Trap.Camera.ActorRef, 63);
    TestEqual(TEXT("Punching ball trap camera zone"), Trap.Camera.ZoneRef, 64);
    TestEqual(TEXT("Punching ball trap camera field 1c"), Trap.Camera.Field1C, 4.5f);
    TestEqual(TEXT("Punching ball actor ref a94"), Actor.PunchingBallActorRefA94, 71);
    TestEqual(TEXT("Punching ball actor ref a98"), Actor.PunchingBallActorRefA98, 72);
    if (!TestEqual(TEXT("Punching ball camera-fight float count"), Actor.PunchingBallCamFigFloats.Num(), 14)) return false;
    TestEqual(TEXT("Punching ball camera-fight float 0"), Actor.PunchingBallCamFigFloats[0], 1.0f);
    TestEqual(TEXT("Punching ball camera-fight float 13"), Actor.PunchingBallCamFigFloats[13], 14.0f);
    TestEqual(TEXT("Punching ball fixture leaves no raw class data"), Actor.ClassData.Num(), 0);
    if (!TestEqual(TEXT("Punching ball behaviour count"), Actor.Behaviours.Num(), 2)) return false;
    TestEqual(TEXT("Punching ball behaviour 0 id"), Actor.Behaviours[0].Id, 3);
    TestEqual(TEXT("Punching ball behaviour 0 is empty"), Actor.Behaviours[0].Data.Num(), 0);
    TestEqual(TEXT("Punching ball behaviour 1 id"), Actor.Behaviours[1].Id, 7);
    TestEqual(TEXT("Punching ball behaviour 1 is empty"), Actor.Behaviours[1].Data.Num(), 0);

    UKyaLevelBankExporter* Exporter = NewObject<UKyaLevelBankExporter>();
    TArray<uint8> Plain;
    FMemoryWriter PlainWriter(Plain);
    if (!TestTrue(TEXT("Export unedited punching ball fixture"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), PlainWriter, GWarn))) return false;
    TestEqual(TEXT("Unedited punching ball export keeps the file size"), Plain.Num(), Input.Num());
    TestTrue(TEXT("Unedited punching ball export preserves every byte"), Plain == Input);

    Actor.PunchingBallFieldA80 = 91.5f;
    Actor.PunchingBallTrapEntries[0].TargetRef = 92;
    Actor.PunchingBallTrapEntries[0].Camera.CameraIndex = 93;
    Actor.PunchingBallActorRefA94 = 94;
    Actor.PunchingBallCamFigFloats[13] = 95.5f;
    TArray<uint8> Edited;
    FMemoryWriter EditedWriter(Edited);
    if (!TestTrue(TEXT("Export edited punching ball fixture"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), EditedWriter, GWarn))) return false;
    TestEqual(TEXT("Edited punching ball export keeps the file size"), Edited.Num(), Input.Num());
    const uint8* EditedCursor = Edited.GetData();
    UKyaLevelBank* Reimported = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
        GetTransientPackage(), NAME_None, RF_Transient, nullptr, TEXT("bnk"), EditedCursor,
        Edited.GetData() + Edited.Num(), GWarn));
    if (!TestNotNull(TEXT("Reimport edited punching ball fixture"), Reimported)) return false;
    const FKyaActorPlacement& EditedActor = Reimported->SceneEnvironment.Actors[0];
    TestEqual(TEXT("Edited punching ball field round trips"), EditedActor.PunchingBallFieldA80, 91.5f);
    TestEqual(TEXT("Edited punching ball trap target round trips"),
        EditedActor.PunchingBallTrapEntries[0].TargetRef, 92);
    TestEqual(TEXT("Edited punching ball trap camera round trips"),
        EditedActor.PunchingBallTrapEntries[0].Camera.CameraIndex, 93);
    TestEqual(TEXT("Edited punching ball actor ref round trips"), EditedActor.PunchingBallActorRefA94, 94);
    TestEqual(TEXT("Edited punching ball camera-fight float round trips"),
        EditedActor.PunchingBallCamFigFloats[13], 95.5f);

    Actor.PunchingBallTrapEntries.AddDefaulted();
    TArray<uint8> Rejected;
    FMemoryWriter RejectedWriter(Rejected);
    TestFalse(TEXT("Exporter rejects a changed punching ball trap count"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), RejectedWriter, GWarn));
    Actor.PunchingBallTrapEntries.RemoveAt(1);
    Actor.PunchingBallCamFigFloats.RemoveAt(13);
    TArray<uint8> RejectedFloats;
    FMemoryWriter RejectedFloatsWriter(RejectedFloats);
    TestFalse(TEXT("Exporter rejects a truncated camera-fight float list"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), RejectedFloatsWriter, GWarn));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKyaNoseMonsterFixtureRoundTripTest, "Kya.Bank.NoseMonsterFixtureRoundTrip",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKyaNoseMonsterFixtureRoundTripTest::RunTest(const FString& Parameters)
{
    FKyaSmallBankBuilder Builder;
    const TArray<uint8> Input = Builder.Build(0x26);
    UKyaLevelBankFactory* Factory = NewObject<UKyaLevelBankFactory>();
    const uint8* Cursor = Input.GetData();
    UKyaLevelBank* Bank = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
        GetTransientPackage(), NAME_None, RF_Transient, nullptr, TEXT("bnk"), Cursor,
        Input.GetData() + Input.Num(), GWarn));
    if (!TestNotNull(TEXT("Import nose monster fixture"), Bank)) return false;
    if (!TestEqual(TEXT("Nose monster fixture actor count"), Bank->SceneEnvironment.Actors.Num(), 1)) return false;
    FKyaActorPlacement& Actor = Bank->SceneEnvironment.Actors[0];
    TestEqual(TEXT("Nose monster class id"), Actor.ClassId, int64(0x26));
    TestEqual(TEXT("Nose monster actor name"), Actor.Name, FString(TEXT("NOZEMONSTER")));
    TestTrue(TEXT("Nose monster class payload parses"), Actor.bHasNoseMonsterData);
    TestTrue(TEXT("Nose monster uses the versioned layout"), Actor.bNoseMonsterHasVersionFloats);
    TestEqual(TEXT("Nose monster zone ref"), Actor.NoseMonsterZoneRef, 11);
    TestEqual(TEXT("Nose monster field 384"), Actor.NoseMonsterField384, 1.1f);
    TestEqual(TEXT("Nose monster field 360"), Actor.NoseMonsterField360, 1.5f);
    TestEqual(TEXT("Nose monster field 378"), Actor.NoseMonsterField378, 1.6f);
    TestEqual(TEXT("Nose monster field 37c"), Actor.NoseMonsterField37C, 1.7f);
    TestEqual(TEXT("Nose monster field 364"), Actor.NoseMonsterField364, 12);
    TestEqual(TEXT("Nose monster field 374"), Actor.NoseMonsterField374, 1.9f);
    if (!TestEqual(TEXT("Nose monster zone path count"), Actor.NoseMonsterZonePathEntries.Num(), 1)) return false;
    TestEqual(TEXT("Nose monster zone path zone"), Actor.NoseMonsterZonePathEntries[0].ZoneRef, 13);
    TestEqual(TEXT("Nose monster zone path follow"), Actor.NoseMonsterZonePathEntries[0].PathFollowRef, 14);
    TestEqual(TEXT("Nose monster field 36c"), Actor.NoseMonsterField36C, 15);
    if (!TestEqual(TEXT("Nose monster fire-shot count"), Actor.NoseMonsterFireShotActorRefs.Num(), 2)) return false;
    TestEqual(TEXT("Nose monster fire-shot ref 0"), Actor.NoseMonsterFireShotActorRefs[0], 16);
    TestEqual(TEXT("Nose monster fire-shot ref 1"), Actor.NoseMonsterFireShotActorRefs[1], 17);
    TestEqual(TEXT("Nose monster field 630"), Actor.NoseMonsterField630, 18);
    TestEqual(TEXT("Nose monster field 380"), Actor.NoseMonsterField380, 19);
    TestEqual(TEXT("Nose monster field 690"), Actor.NoseMonsterField690, 2.1f);
    TestEqual(TEXT("Nose monster field 69c"), Actor.NoseMonsterField69C, 2.4f);
    TestEqual(TEXT("Nose monster field 6a0"), Actor.NoseMonsterField6A0, 20);
    TestEqual(TEXT("Nose monster field 6a8"), Actor.NoseMonsterField6A8, 22);
    if (!TestEqual(TEXT("Nose monster switch target count"), Actor.NoseMonsterSwitch.Targets.Num(), 1)) return false;
    TestEqual(TEXT("Nose monster switch target"), Actor.NoseMonsterSwitch.Targets[0].TargetRef, 31);
    TestEqual(TEXT("Nose monster switch cutscene"), Actor.NoseMonsterSwitch.Targets[0].CutsceneId, 32);
    TestEqual(TEXT("Nose monster switch message"), Actor.NoseMonsterSwitch.Targets[0].MessageId, 34);
    TestEqual(TEXT("Nose monster switch camera"), Actor.NoseMonsterSwitch.Camera.CameraIndex, 41);
    TestEqual(TEXT("Nose monster switch camera actor"), Actor.NoseMonsterSwitch.Camera.ActorRef, 43);
    TestEqual(TEXT("Nose monster fixture leaves no raw class data"), Actor.ClassData.Num(), 0);
    if (!TestEqual(TEXT("Nose monster behaviour count"), Actor.Behaviours.Num(), 3)) return false;
    TestEqual(TEXT("Nose monster behaviour 0 id"), Actor.Behaviours[0].Id, 2);
    TestEqual(TEXT("Nose monster behaviour 1 id"), Actor.Behaviours[1].Id, 3);
    TestEqual(TEXT("Nose monster behaviour 2 id"), Actor.Behaviours[2].Id, 4);
    for (int32 Index = 0; Index < 3; ++Index)
        TestEqual(TEXT("Nose monster behaviour is empty"), Actor.Behaviours[Index].Data.Num(), 0);

    UKyaLevelBankExporter* Exporter = NewObject<UKyaLevelBankExporter>();
    TArray<uint8> Plain;
    FMemoryWriter PlainWriter(Plain);
    if (!TestTrue(TEXT("Export unedited nose monster fixture"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), PlainWriter, GWarn))) return false;
    TestEqual(TEXT("Unedited nose monster export keeps the file size"), Plain.Num(), Input.Num());
    TestTrue(TEXT("Unedited nose monster export preserves every byte"), Plain == Input);

    Actor.NoseMonsterField384 = 91.5f;
    Actor.NoseMonsterField378 = 92.5f;
    Actor.NoseMonsterZonePathEntries[0].ZoneRef = 93;
    Actor.NoseMonsterFireShotActorRefs[0] = 94;
    Actor.NoseMonsterSwitch.Targets[0].TargetRef = 95;
    Actor.NoseMonsterSwitch.Camera.CameraIndex = 96;
    Actor.NoseMonsterField6A8 = 97;
    TArray<uint8> Edited;
    FMemoryWriter EditedWriter(Edited);
    if (!TestTrue(TEXT("Export edited nose monster fixture"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), EditedWriter, GWarn))) return false;
    TestEqual(TEXT("Edited nose monster export keeps the file size"), Edited.Num(), Input.Num());
    const uint8* EditedCursor = Edited.GetData();
    UKyaLevelBank* Reimported = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
        GetTransientPackage(), NAME_None, RF_Transient, nullptr, TEXT("bnk"), EditedCursor,
        Edited.GetData() + Edited.Num(), GWarn));
    if (!TestNotNull(TEXT("Reimport edited nose monster fixture"), Reimported)) return false;
    const FKyaActorPlacement& EditedActor = Reimported->SceneEnvironment.Actors[0];
    TestEqual(TEXT("Edited nose monster field round trips"), EditedActor.NoseMonsterField384, 91.5f);
    TestEqual(TEXT("Edited nose monster version float round trips"), EditedActor.NoseMonsterField378, 92.5f);
    TestEqual(TEXT("Edited nose monster zone path round trips"),
        EditedActor.NoseMonsterZonePathEntries[0].ZoneRef, 93);
    TestEqual(TEXT("Edited nose monster fire-shot ref round trips"),
        EditedActor.NoseMonsterFireShotActorRefs[0], 94);
    TestEqual(TEXT("Edited nose monster switch target round trips"),
        EditedActor.NoseMonsterSwitch.Targets[0].TargetRef, 95);
    TestEqual(TEXT("Edited nose monster camera round trips"),
        EditedActor.NoseMonsterSwitch.Camera.CameraIndex, 96);
    TestEqual(TEXT("Edited nose monster field 6a8 round trips"), EditedActor.NoseMonsterField6A8, 97);

    Actor.bNoseMonsterHasVersionFloats = false;
    TArray<uint8> RejectedVersion;
    FMemoryWriter RejectedVersionWriter(RejectedVersion);
    TestFalse(TEXT("Exporter rejects a nose monster layout-version change"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), RejectedVersionWriter, GWarn));
    Actor.bNoseMonsterHasVersionFloats = true;
    Actor.NoseMonsterZonePathEntries.AddDefaulted();
    TArray<uint8> Rejected;
    FMemoryWriter RejectedWriter(Rejected);
    TestFalse(TEXT("Exporter rejects a changed nose monster zone-path count"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), RejectedWriter, GWarn));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKyaPatternFixtureRoundTripTest, "Kya.Bank.PatternFixtureRoundTrip",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKyaPatternFixtureRoundTripTest::RunTest(const FString& Parameters)
{
    FKyaSmallBankBuilder Builder;
    const TArray<uint8> Input = Builder.Build(0x3f);
    UKyaLevelBankFactory* Factory = NewObject<UKyaLevelBankFactory>();
    const uint8* Cursor = Input.GetData();
    UKyaLevelBank* Bank = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
        GetTransientPackage(), NAME_None, RF_Transient, nullptr, TEXT("bnk"), Cursor,
        Input.GetData() + Input.Num(), GWarn));
    if (!TestNotNull(TEXT("Import pattern fixture"), Bank)) return false;
    if (!TestEqual(TEXT("Pattern fixture actor count"), Bank->SceneEnvironment.Actors.Num(), 1)) return false;
    FKyaActorPlacement& Actor = Bank->SceneEnvironment.Actors[0];
    TestEqual(TEXT("Pattern class id"), Actor.ClassId, int64(0x3f));
    TestEqual(TEXT("Pattern actor name"), Actor.Name, FString(TEXT("LINER")));
    TestTrue(TEXT("Pattern class payload parses"), Actor.bHasPatternData);
    TestEqual(TEXT("Pattern field 350"), Actor.PatternField350, 10.0f);
    TestEqual(TEXT("Pattern field 354"), Actor.PatternField354, 10.0f);
    TestEqual(TEXT("Pattern field 358"), Actor.PatternField358, 6.0f);
    TestEqual(TEXT("Pattern field 35c"), Actor.PatternField35C, 12.0f);
    TestEqual(TEXT("Pattern field 364"), Actor.PatternField364, 0.6f);
    TestEqual(TEXT("Pattern field 360"), Actor.PatternField360, 1.6f);
    TestEqual(TEXT("Pattern field 368"), Actor.PatternField368, 3.0f);
    TestEqual(TEXT("Pattern field 36c"), Actor.PatternField36C, 46);
    TestEqual(TEXT("Pattern field 370"), Actor.PatternField370, 140);
    TestEqual(TEXT("Pattern bone id"), Actor.PatternBoneId, int32(0x8079674D));
    TestEqual(TEXT("Pattern add-on field 0"), Actor.PatternAddOn.Field0, 1);
    TestEqual(TEXT("Pattern add-on money type"), Actor.PatternAddOn.MoneyType, 2);
    TestEqual(TEXT("Pattern add-on money count"), Actor.PatternAddOn.MoneyCount, 3);
    TestEqual(TEXT("Pattern add-on bonus count"), Actor.PatternAddOn.BonusCount, 4);
    TestEqual(TEXT("Pattern add-on field c"), Actor.PatternAddOn.FieldC, 5);
    TestEqual(TEXT("Pattern add-on sub-object mode"), Actor.PatternAddOn.SubObjMode, 0);
    TestEqual(TEXT("Pattern fixture leaves no raw class data"), Actor.ClassData.Num(), 0);
    if (!TestEqual(TEXT("Pattern behaviour count"), Actor.Behaviours.Num(), 2)) return false;
    const FKyaActorBehaviour& Alone = Actor.Behaviours[0];
    TestEqual(TEXT("Pattern alone id"), Alone.Id, 3);
    TestTrue(TEXT("Pattern alone is typed"), Alone.bHasTypedPatternBehaviour);
    TestEqual(TEXT("Pattern alone field 90"), Alone.PatternField90, 5.0f);
    TestEqual(TEXT("Pattern alone field 94"), Alone.PatternField94, 1.0f);
    TestEqual(TEXT("Pattern alone field 98"), Alone.PatternField98, 4.0f);
    TestEqual(TEXT("Pattern part field 4"), Alone.PatternPartField4, 0.3f);
    TestEqual(TEXT("Pattern part field 0"), Alone.PatternPartField0, 0.5f);
    TestEqual(TEXT("Pattern part field c"), Alone.PatternPartFieldC, 0.12f);
    TestEqual(TEXT("Pattern part field 8"), Alone.PatternPartField8, 0.25f);
    TestEqual(TEXT("Pattern part field 10"), Alone.PatternPartField10, 3.0f);
    TestEqual(TEXT("Pattern part field 14"), Alone.PatternPartField14, -1.0f);
    TestEqual(TEXT("Pattern part material"), Alone.PatternPartMaterialId, 162);
    TestEqual(TEXT("Pattern part field 1c"), Alone.PatternPartField1C, 25.0f);
    TestEqual(TEXT("Pattern part field 20"), Alone.PatternPartField20, 2.0f);
    TestTrue(TEXT("Pattern part has version field"), Alone.bPatternPartHasField24);
    TestEqual(TEXT("Pattern part field 24"), Alone.PatternPartField24, 1.0f);
    TestEqual(TEXT("Pattern part field 28"), Alone.PatternPartField28, 0);
    TestEqual(TEXT("Pattern multiple id"), Actor.Behaviours[1].Id, 4);
    TestEqual(TEXT("Pattern multiple is empty"), Actor.Behaviours[1].Data.Num(), 0);

    UKyaLevelBankExporter* Exporter = NewObject<UKyaLevelBankExporter>();
    TArray<uint8> Plain;
    FMemoryWriter PlainWriter(Plain);
    if (!TestTrue(TEXT("Export unedited pattern fixture"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), PlainWriter, GWarn))) return false;
    TestEqual(TEXT("Unedited pattern export keeps the file size"), Plain.Num(), Input.Num());
    TestTrue(TEXT("Unedited pattern export preserves every byte"), Plain == Input);

    Actor.PatternField350 = 91.5f;
    Actor.PatternField36C = 92;
    Actor.PatternBoneId = 93;
    Actor.PatternAddOn.MoneyCount = 94;
    Actor.Behaviours[0].PatternField90 = 95.5f;
    Actor.Behaviours[0].PatternPartMaterialId = 96;
    Actor.Behaviours[0].PatternPartField24 = 97.5f;
    TArray<uint8> Edited;
    FMemoryWriter EditedWriter(Edited);
    if (!TestTrue(TEXT("Export edited pattern fixture"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), EditedWriter, GWarn))) return false;
    TestEqual(TEXT("Edited pattern export keeps the file size"), Edited.Num(), Input.Num());
    const uint8* EditedCursor = Edited.GetData();
    UKyaLevelBank* Reimported = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
        GetTransientPackage(), NAME_None, RF_Transient, nullptr, TEXT("bnk"), EditedCursor,
        Edited.GetData() + Edited.Num(), GWarn));
    if (!TestNotNull(TEXT("Reimport edited pattern fixture"), Reimported)) return false;
    const FKyaActorPlacement& EditedActor = Reimported->SceneEnvironment.Actors[0];
    TestEqual(TEXT("Edited pattern field round trips"), EditedActor.PatternField350, 91.5f);
    TestEqual(TEXT("Edited pattern field 36c round trips"), EditedActor.PatternField36C, 92);
    TestEqual(TEXT("Edited pattern bone round trips"), EditedActor.PatternBoneId, 93);
    TestEqual(TEXT("Edited pattern add-on round trips"), EditedActor.PatternAddOn.MoneyCount, 94);
    TestEqual(TEXT("Edited pattern behaviour float round trips"), EditedActor.Behaviours[0].PatternField90, 95.5f);
    TestEqual(TEXT("Edited pattern material round trips"), EditedActor.Behaviours[0].PatternPartMaterialId, 96);
    TestEqual(TEXT("Edited pattern version float round trips"), EditedActor.Behaviours[0].PatternPartField24, 97.5f);

    Actor.PatternAddOn.SubObjMode = 1;
    TArray<uint8> RejectedMode;
    FMemoryWriter RejectedModeWriter(RejectedMode);
    TestFalse(TEXT("Exporter rejects a pattern add-on mode change"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), RejectedModeWriter, GWarn));
    Actor.PatternAddOn.SubObjMode = 0;
    Actor.Behaviours[0].bPatternPartHasField24 = false;
    TArray<uint8> Rejected;
    FMemoryWriter RejectedWriter(Rejected);
    TestFalse(TEXT("Exporter rejects a pattern-part layout change"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), RejectedWriter, GWarn));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKyaShipFixtureRoundTripTest, "Kya.Bank.ShipFixtureRoundTrip",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKyaShipFixtureRoundTripTest::RunTest(const FString& Parameters)
{
    FKyaSmallBankBuilder Builder;
    const TArray<uint8> Input = Builder.Build(0x36);
    UKyaLevelBankFactory* Factory = NewObject<UKyaLevelBankFactory>();
    const uint8* Cursor = Input.GetData();
    UKyaLevelBank* Bank = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
        GetTransientPackage(), NAME_None, RF_Transient, nullptr, TEXT("bnk"), Cursor,
        Input.GetData() + Input.Num(), GWarn));
    if (!TestNotNull(TEXT("Import ship fixture"), Bank)) return false;
    if (!TestEqual(TEXT("Ship fixture actor count"), Bank->SceneEnvironment.Actors.Num(), 1)) return false;
    FKyaActorPlacement& Actor = Bank->SceneEnvironment.Actors[0];
    TestEqual(TEXT("Ship class id"), Actor.ClassId, int64(0x36));
    TestEqual(TEXT("Ship actor name"), Actor.Name, FString(TEXT("HUNTERSHIP")));
    TestTrue(TEXT("Ship class payload parses"), Actor.bHasShipData);
    if (!TestEqual(TEXT("Ship care-boy ref count"), Actor.ShipCareBoyActorRefs.Num(), 2)) return false;
    TestEqual(TEXT("Ship care-boy ref 0"), Actor.ShipCareBoyActorRefs[0], 21);
    TestEqual(TEXT("Ship care-boy ref 1"), Actor.ShipCareBoyActorRefs[1], 22);
    if (!TestEqual(TEXT("Ship fx int count"), Actor.ShipFxIntValues.Num(), 2)) return false;
    TestEqual(TEXT("Ship fx int 0"), Actor.ShipFxIntValues[0], 23);
    TestEqual(TEXT("Ship fx int 1"), Actor.ShipFxIntValues[1], 24);
    TestEqual(TEXT("Ship jaw ref"), Actor.ShipJawCareBoyRef, 31);
    TestEqual(TEXT("Ship air cannon ref"), Actor.ShipAirCannonRef, 32);
    TestEqual(TEXT("Ship hunter ref"), Actor.ShipHunterRef, 33);
    TestEqual(TEXT("Ship field 214"), Actor.ShipField214, 1.1f);
    TestEqual(TEXT("Ship field 210"), Actor.ShipField210, 2.1f);
    TestEqual(TEXT("Ship field 224"), Actor.ShipField224, 6.1f);
    TestEqual(TEXT("Ship kim touch range"), Actor.ShipKimTouchRange, 8.1f);
    TestEqual(TEXT("Ship field 22c"), Actor.ShipField22C, 9.1f);
    TestEqual(TEXT("Ship fx field 18"), Actor.ShipFxField18, 41);
    TestEqual(TEXT("Ship field 234"), Actor.ShipField234, 42);
    TestEqual(TEXT("Ship field 238"), Actor.ShipField238, 43);
    TestEqual(TEXT("Ship field 1ac"), Actor.ShipField1AC, 44);
    if (!TestEqual(TEXT("Ship word block size"), Actor.ShipWords18C.Num(), 4)) return false;
    TestEqual(TEXT("Ship word 0"), Actor.ShipWords18C[0], 51);
    TestEqual(TEXT("Ship word 3"), Actor.ShipWords18C[3], 54);
    TestEqual(TEXT("Ship field 19c"), Actor.ShipField19C, 61);
    TestEqual(TEXT("Ship field 1a8"), Actor.ShipField1A8, 64);
    TestEqual(TEXT("Ship vision range"), Actor.ShipVisionRange, 2.1f);
    TestEqual(TEXT("Ship vision flags"), Actor.ShipVisionFlags, 65);
    if (!TestEqual(TEXT("Ship switch target count"), Actor.ShipSwitch.Targets.Num(), 1)) return false;
    TestEqual(TEXT("Ship switch target"), Actor.ShipSwitch.Targets[0].TargetRef, 71);
    TestEqual(TEXT("Ship switch camera"), Actor.ShipSwitch.Camera.CameraIndex, 81);
    TestEqual(TEXT("Ship fixture leaves no raw class data"), Actor.ClassData.Num(), 0);
    if (!TestEqual(TEXT("Ship behaviour count"), Actor.Behaviours.Num(), 1)) return false;
    TestEqual(TEXT("Ship behaviour id"), Actor.Behaviours[0].Id, 2);
    TestEqual(TEXT("Ship behaviour is empty"), Actor.Behaviours[0].Data.Num(), 0);

    UKyaLevelBankExporter* Exporter = NewObject<UKyaLevelBankExporter>();
    TArray<uint8> Plain;
    FMemoryWriter PlainWriter(Plain);
    if (!TestTrue(TEXT("Export unedited ship fixture"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), PlainWriter, GWarn))) return false;
    TestEqual(TEXT("Unedited ship export keeps the file size"), Plain.Num(), Input.Num());
    TestTrue(TEXT("Unedited ship export preserves every byte"), Plain == Input);

    Actor.ShipCareBoyActorRefs[0] = 91;
    Actor.ShipField214 = 92.5f;
    Actor.ShipHunterRef = 93;
    Actor.ShipWords18C[2] = 94;
    Actor.ShipVisionRange = 95.5f;
    Actor.ShipSwitch.Targets[0].TargetRef = 96;
    Actor.ShipSwitch.Camera.CameraIndex = 97;
    TArray<uint8> Edited;
    FMemoryWriter EditedWriter(Edited);
    if (!TestTrue(TEXT("Export edited ship fixture"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), EditedWriter, GWarn))) return false;
    TestEqual(TEXT("Edited ship export keeps the file size"), Edited.Num(), Input.Num());
    const uint8* EditedCursor = Edited.GetData();
    UKyaLevelBank* Reimported = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
        GetTransientPackage(), NAME_None, RF_Transient, nullptr, TEXT("bnk"), EditedCursor,
        Edited.GetData() + Edited.Num(), GWarn));
    if (!TestNotNull(TEXT("Reimport edited ship fixture"), Reimported)) return false;
    const FKyaActorPlacement& EditedActor = Reimported->SceneEnvironment.Actors[0];
    TestEqual(TEXT("Edited ship care-boy round trips"), EditedActor.ShipCareBoyActorRefs[0], 91);
    TestEqual(TEXT("Edited ship field round trips"), EditedActor.ShipField214, 92.5f);
    TestEqual(TEXT("Edited ship hunter round trips"), EditedActor.ShipHunterRef, 93);
    TestEqual(TEXT("Edited ship word round trips"), EditedActor.ShipWords18C[2], 94);
    TestEqual(TEXT("Edited ship vision round trips"), EditedActor.ShipVisionRange, 95.5f);
    TestEqual(TEXT("Edited ship switch target round trips"),
        EditedActor.ShipSwitch.Targets[0].TargetRef, 96);
    TestEqual(TEXT("Edited ship camera round trips"),
        EditedActor.ShipSwitch.Camera.CameraIndex, 97);

    Actor.ShipCareBoyActorRefs.Add(98);
    TArray<uint8> Rejected;
    FMemoryWriter RejectedWriter(Rejected);
    TestFalse(TEXT("Exporter rejects a changed ship care-boy count"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), RejectedWriter, GWarn));
    Actor.ShipCareBoyActorRefs.RemoveAt(2);
    Actor.ShipWords18C.RemoveAt(3);
    TArray<uint8> RejectedWords;
    FMemoryWriter RejectedWordsWriter(RejectedWords);
    TestFalse(TEXT("Exporter rejects a truncated ship word block"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), RejectedWordsWriter, GWarn));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKyaBunchFixtureRoundTripTest, "Kya.Bank.BunchFixtureRoundTrip",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKyaBunchFixtureRoundTripTest::RunTest(const FString& Parameters)
{
    FKyaSmallBankBuilder Builder;
    const TArray<uint8> Input = Builder.Build(0x4e);
    UKyaLevelBankFactory* Factory = NewObject<UKyaLevelBankFactory>();
    const uint8* Cursor = Input.GetData();
    UKyaLevelBank* Bank = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
        GetTransientPackage(), NAME_None, RF_Transient, nullptr, TEXT("bnk"), Cursor,
        Input.GetData() + Input.Num(), GWarn));
    if (!TestNotNull(TEXT("Import bunch fixture"), Bank)) return false;
    if (!TestEqual(TEXT("Bunch fixture actor count"), Bank->SceneEnvironment.Actors.Num(), 1)) return false;
    FKyaActorPlacement& Actor = Bank->SceneEnvironment.Actors[0];
    TestEqual(TEXT("Bunch class id"), Actor.ClassId, int64(0x4e));
    TestEqual(TEXT("Bunch actor name"), Actor.Name, FString(TEXT("KRONOS_BUNCH")));
    TestTrue(TEXT("Bunch class payload parses"), Actor.bHasBunchData);
    TestEqual(TEXT("Bunch field fc8"), Actor.BunchFieldFC8, 1.5f);
    if (!TestEqual(TEXT("Bunch stream entry count"), Actor.BunchStreamEntries.Num(), 2)) return false;
    TestEqual(TEXT("Bunch entry 0 field"), Actor.BunchStreamEntries[0].Field0, 11);
    TestEqual(TEXT("Bunch entry 0 has no behaviour"), Actor.BunchStreamEntries[0].HasBehaviour, 0);
    TestEqual(TEXT("Bunch entry 0 behaviour id"), Actor.BunchStreamEntries[0].BehaviourId, -1);
    TestEqual(TEXT("Bunch entry 0 zone"), Actor.BunchStreamEntries[0].ZoneId, 12);
    TestEqual(TEXT("Bunch entry 0 field c"), Actor.BunchStreamEntries[0].FieldC, 13);
    TestEqual(TEXT("Bunch entry 1 field"), Actor.BunchStreamEntries[1].Field0, 14);
    TestEqual(TEXT("Bunch entry 1 has behaviour"), Actor.BunchStreamEntries[1].HasBehaviour, 1);
    TestEqual(TEXT("Bunch entry 1 behaviour id"), Actor.BunchStreamEntries[1].BehaviourId, 15);
    TestEqual(TEXT("Bunch entry 1 zone"), Actor.BunchStreamEntries[1].ZoneId, 16);
    TestEqual(TEXT("Bunch mesh component"), Actor.BunchMeshComponentId, 21);
    TestEqual(TEXT("Bunch mesh index"), Actor.BunchMeshIndex, 22);
    TestEqual(TEXT("Bunch fixture leaves no raw class data"), Actor.ClassData.Num(), 0);
    if (!TestEqual(TEXT("Bunch behaviour count"), Actor.Behaviours.Num(), 5)) return false;

    TestEqual(TEXT("Bunch behaviour 0 id"), Actor.Behaviours[0].Id, 3);
    TestEqual(TEXT("Bunch behaviour 0 is empty"), Actor.Behaviours[0].Data.Num(), 0);
    TestEqual(TEXT("Bunch behaviour 1 id"), Actor.Behaviours[1].Id, 4);
    TestEqual(TEXT("Bunch behaviour 1 is empty"), Actor.Behaviours[1].Data.Num(), 0);
    TestEqual(TEXT("Bunch behaviour 2 id"), Actor.Behaviours[2].Id, 15);
    TestTrue(TEXT("Bunch track behaviour is typed"), Actor.Behaviours[2].bHasTypedWolfenBehaviour);
    TestEqual(TEXT("Bunch track flag"), Actor.Behaviours[2].WolfenBehaviour.Flags, 1);
    TestEqual(TEXT("Bunch track field"), Actor.Behaviours[2].WolfenBehaviour.TrackField, 2.5f);

    const FKyaActorBehaviour& Repel = Actor.Behaviours[3];
    TestEqual(TEXT("Bunch repel id"), Repel.Id, 26);
    TestTrue(TEXT("Bunch repel is typed"), Repel.bHasTypedBunchRepel);
    TestEqual(TEXT("Bunch repel waypoint"), Repel.BunchRepelWayPointRef, 31);
    TestEqual(TEXT("Bunch repel shockwave count"), Repel.BunchRepelShockWaveCount, 1);
    TestEqual(TEXT("Bunch repel field 14"), Repel.BunchRepelField14, 1.5f);
    TestEqual(TEXT("Bunch repel shockwave field 10"), Repel.BunchRepelShockWaveField10, 2.5f);
    TestEqual(TEXT("Bunch repel shockwave field 14"), Repel.BunchRepelShockWaveField14, 3.5f);
    if (!TestEqual(TEXT("Bunch repel index count"), Repel.BunchRepelIndices.Num(), 1)) return false;
    TestEqual(TEXT("Bunch repel index"), Repel.BunchRepelIndices[0], 41);
    TestEqual(TEXT("Bunch repel damage"), Repel.BunchRepelDamage, 1.1f);
    TestEqual(TEXT("Bunch repel msg field 30"), Repel.BunchRepelMsgHitField30, 2.2f);
    TestEqual(TEXT("Bunch repel msg field 70"), Repel.BunchRepelMsgHitField70, 3.3f);
    TestEqual(TEXT("Bunch repel vector 20 y"), float(Repel.BunchRepelVector20.Y), 0.0f);
    TestEqual(TEXT("Bunch repel vector 30 y"), float(Repel.BunchRepelVector30.Y), 1.0f);
    TestEqual(TEXT("Bunch repel mesh component"), Repel.BunchRepelMeshComponentId, 51);
    TestEqual(TEXT("Bunch repel mesh index"), Repel.BunchRepelMeshIndex, 52);
    TestEqual(TEXT("Bunch repel mesh type"), Repel.BunchRepelMeshType, 53);
    TestEqual(TEXT("Bunch repel reserved"), Repel.BunchRepelReserved, 54);
    TestEqual(TEXT("Bunch repel default id"), Repel.BunchRepelDefaultId, 55);

    const FKyaActorBehaviour& Fence = Actor.Behaviours[4];
    TestEqual(TEXT("Bunch fence id"), Fence.Id, 27);
    TestTrue(TEXT("Bunch fence is typed"), Fence.bHasTypedBunchFence);
    TestEqual(TEXT("Bunch fence actor ref"), Fence.BunchFenceActorRef, 61);
    TestEqual(TEXT("Bunch fence waypoint ref"), Fence.BunchFenceWayPointRef, 62);
    if (!TestEqual(TEXT("Bunch fence waypoint count"), Fence.BunchFenceWayPoints.Num(), 2)) return false;
    TestEqual(TEXT("Bunch fence waypoint 0"), Fence.BunchFenceWayPoints[0], 63);
    TestEqual(TEXT("Bunch fence waypoint 1"), Fence.BunchFenceWayPoints[1], 64);
    if (!TestEqual(TEXT("Bunch fence group count"), Fence.BunchFenceGroups.Num(), 1)) return false;
    TestEqual(TEXT("Bunch fence group field 0"), Fence.BunchFenceGroups[0].Field0, 1);
    TestEqual(TEXT("Bunch fence group field 4"), Fence.BunchFenceGroups[0].Field4, 71);
    if (!TestEqual(TEXT("Bunch fence group entry count"), Fence.BunchFenceGroups[0].Entries.Num(), 2)) return false;
    TestEqual(TEXT("Bunch fence entry 0 ref"), Fence.BunchFenceGroups[0].Entries[0].Ref, 72);
    TestEqual(TEXT("Bunch fence entry 1 field 4"), Fence.BunchFenceGroups[0].Entries[1].Field4, 75);
    TestEqual(TEXT("Bunch fence mesh f0 texture"), Fence.BunchFenceMeshF0Texture, 81);
    TestEqual(TEXT("Bunch fence mesh f0 mesh"), Fence.BunchFenceMeshF0Mesh, 82);
    TestEqual(TEXT("Bunch fence mesh 70 texture"), Fence.BunchFenceMesh70Texture, 83);
    TestEqual(TEXT("Bunch fence mesh 70 mesh"), Fence.BunchFenceMesh70Mesh, 84);
    if (!TestEqual(TEXT("Bunch fence fx type count"), Fence.BunchFenceFxTypes.Num(), 4)) return false;
    TestEqual(TEXT("Bunch fence fx type 0"), Fence.BunchFenceFxTypes[0], 91);
    TestEqual(TEXT("Bunch fence fx type 3"), Fence.BunchFenceFxTypes[3], 94);

    UKyaLevelBankExporter* Exporter = NewObject<UKyaLevelBankExporter>();
    TArray<uint8> Plain;
    FMemoryWriter PlainWriter(Plain);
    if (!TestTrue(TEXT("Export unedited bunch fixture"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), PlainWriter, GWarn))) return false;
    TestEqual(TEXT("Unedited bunch export keeps the file size"), Plain.Num(), Input.Num());
    TestTrue(TEXT("Unedited bunch export preserves every byte"), Plain == Input);

    Actor.BunchFieldFC8 = 91.5f;
    Actor.BunchStreamEntries[1].BehaviourId = 92;
    Actor.BunchMeshIndex = 93;
    Actor.Behaviours[2].WolfenBehaviour.Flags = 94;
    Actor.Behaviours[3].BunchRepelDamage = 95.5f;
    Actor.Behaviours[3].BunchRepelIndices[0] = 96;
    Actor.Behaviours[4].BunchFenceWayPoints[0] = 97;
    Actor.Behaviours[4].BunchFenceGroups[0].Entries[1].Field4 = 98;
    Actor.Behaviours[4].BunchFenceFxTypes[2] = 99;
    TArray<uint8> Edited;
    FMemoryWriter EditedWriter(Edited);
    if (!TestTrue(TEXT("Export edited bunch fixture"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), EditedWriter, GWarn))) return false;
    TestEqual(TEXT("Edited bunch export keeps the file size"), Edited.Num(), Input.Num());
    const uint8* EditedCursor = Edited.GetData();
    UKyaLevelBank* Reimported = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
        GetTransientPackage(), NAME_None, RF_Transient, nullptr, TEXT("bnk"), EditedCursor,
        Edited.GetData() + Edited.Num(), GWarn));
    if (!TestNotNull(TEXT("Reimport edited bunch fixture"), Reimported)) return false;
    const FKyaActorPlacement& EditedActor = Reimported->SceneEnvironment.Actors[0];
    TestEqual(TEXT("Edited bunch field round trips"), EditedActor.BunchFieldFC8, 91.5f);
    TestEqual(TEXT("Edited bunch entry round trips"), EditedActor.BunchStreamEntries[1].BehaviourId, 92);
    TestEqual(TEXT("Edited bunch mesh round trips"), EditedActor.BunchMeshIndex, 93);
    TestEqual(TEXT("Edited bunch track flag round trips"), EditedActor.Behaviours[2].WolfenBehaviour.Flags, 94);
    TestEqual(TEXT("Edited bunch repel damage round trips"), EditedActor.Behaviours[3].BunchRepelDamage, 95.5f);
    TestEqual(TEXT("Edited bunch repel index round trips"), EditedActor.Behaviours[3].BunchRepelIndices[0], 96);
    TestEqual(TEXT("Edited bunch fence waypoint round trips"),
        EditedActor.Behaviours[4].BunchFenceWayPoints[0], 97);
    TestEqual(TEXT("Edited bunch fence entry round trips"),
        EditedActor.Behaviours[4].BunchFenceGroups[0].Entries[1].Field4, 98);
    TestEqual(TEXT("Edited bunch fence fx round trips"), EditedActor.Behaviours[4].BunchFenceFxTypes[2], 99);

    Actor.BunchStreamEntries.AddDefaulted();
    TArray<uint8> Rejected;
    FMemoryWriter RejectedWriter(Rejected);
    TestFalse(TEXT("Exporter rejects a changed bunch entry count"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), RejectedWriter, GWarn));
    Actor.BunchStreamEntries.RemoveAt(2);
    Actor.Behaviours[3].BunchRepelIndices.Add(100);
    TArray<uint8> RejectedIndices;
    FMemoryWriter RejectedIndicesWriter(RejectedIndices);
    TestFalse(TEXT("Exporter rejects a changed bunch repel index count"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), RejectedIndicesWriter, GWarn));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKyaBrazulFixtureRoundTripTest, "Kya.Bank.BrazulFixtureRoundTrip",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKyaBrazulFixtureRoundTripTest::RunTest(const FString& Parameters)
{
    FKyaSmallBankBuilder Builder;
    const TArray<uint8> Input = Builder.Build(0x4f);
    UKyaLevelBankFactory* Factory = NewObject<UKyaLevelBankFactory>();
    const uint8* Cursor = Input.GetData();
    UKyaLevelBank* Bank = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
        GetTransientPackage(), NAME_None, RF_Transient, nullptr, TEXT("bnk"), Cursor,
        Input.GetData() + Input.Num(), GWarn));
    if (!TestNotNull(TEXT("Import Brazul fixture"), Bank)) return false;
    if (!TestEqual(TEXT("Brazul fixture actor count"), Bank->SceneEnvironment.Actors.Num(), 1)) return false;
    FKyaActorPlacement& Actor = Bank->SceneEnvironment.Actors[0];
    TestEqual(TEXT("Brazul class id"), Actor.ClassId, int64(0x4f));
    TestEqual(TEXT("Brazul actor name"), Actor.Name, FString(TEXT("BRAZUL")));
    TestTrue(TEXT("Brazul class payload parses"), Actor.bHasBrazulData);
    TestEqual(TEXT("Brazul field fc0"), Actor.BrazulFieldFC0, 1.5f);
    TestEqual(TEXT("Brazul field fc4"), Actor.BrazulFieldFC4, 2.5f);
    TestEqual(TEXT("Brazul path index"), Actor.BrazulPathIndex, 31);
    TestEqual(TEXT("Brazul field 1060"), Actor.BrazulField1060, 32);
    TestEqual(TEXT("Brazul reserved"), Actor.BrazulReserved, 33);
    TestEqual(TEXT("Brazul material"), Actor.BrazulMaterialId, 34);
    TestEqual(TEXT("Brazul spark param A"), Actor.BrazulSparkParamA, 35);
    TestEqual(TEXT("Brazul spark param B"), Actor.BrazulSparkParamB, 36);
    if (!TestEqual(TEXT("Brazul fx handle count"), Actor.BrazulFxHandles.Num(), 5)) return false;
    TestEqual(TEXT("Brazul fx handle 0"), Actor.BrazulFxHandles[0], 41);
    TestEqual(TEXT("Brazul fx handle 4"), Actor.BrazulFxHandles[4], 46);
    TestEqual(TEXT("Brazul fx init"), Actor.BrazulFxInit, 44);
    TestEqual(TEXT("Brazul mesh A texture"), Actor.BrazulMeshATexture, 51);
    TestEqual(TEXT("Brazul mesh A mesh"), Actor.BrazulMeshAMesh, 52);
    TestEqual(TEXT("Brazul mesh B texture"), Actor.BrazulMeshBTexture, 53);
    TestEqual(TEXT("Brazul mesh B mesh"), Actor.BrazulMeshBMesh, 54);
    TestEqual(TEXT("Brazul fixture leaves no raw class data"), Actor.ClassData.Num(), 0);
    if (!TestEqual(TEXT("Brazul behaviour count"), Actor.Behaviours.Num(), 5)) return false;

    TestEqual(TEXT("Brazul behaviour 0 id"), Actor.Behaviours[0].Id, 3);
    TestEqual(TEXT("Brazul behaviour 0 is empty"), Actor.Behaviours[0].Data.Num(), 0);
    TestEqual(TEXT("Brazul behaviour 1 id"), Actor.Behaviours[1].Id, 4);
    TestEqual(TEXT("Brazul behaviour 1 is empty"), Actor.Behaviours[1].Data.Num(), 0);
    TestEqual(TEXT("Brazul behaviour 2 id"), Actor.Behaviours[2].Id, 15);
    TestTrue(TEXT("Brazul track behaviour is typed"), Actor.Behaviours[2].bHasTypedWolfenBehaviour);
    TestEqual(TEXT("Brazul track field"), Actor.Behaviours[2].WolfenBehaviour.TrackField, 2.5f);

    const FKyaActorBehaviour& A = Actor.Behaviours[3];
    TestEqual(TEXT("Brazul A id"), A.Id, 26);
    TestTrue(TEXT("Brazul A is typed"), A.bHasTypedBrazulA);
    if (!TestEqual(TEXT("Brazul A sub-object count"), A.BrazulSubObjs.Num(), 1)) return false;
    TestEqual(TEXT("Brazul A sub-object field 0"), A.BrazulSubObjs[0].Field0, 61);
    TestEqual(TEXT("Brazul A sub-object field 4"), A.BrazulSubObjs[0].Field4, 62);
    TestEqual(TEXT("Brazul A sub-object field 8"), A.BrazulSubObjs[0].Field8, 3.5f);
    TestEqual(TEXT("Brazul A sub-object field c"), A.BrazulSubObjs[0].FieldC, 1);
    TestEqual(TEXT("Brazul A sub-object field 10"), A.BrazulSubObjs[0].Field10, 63);
    if (!TestEqual(TEXT("Brazul A path count"), A.BrazulSubObjs[0].PathIndices.Num(), 2)) return false;
    TestEqual(TEXT("Brazul A path 0"), A.BrazulSubObjs[0].PathIndices[0], 64);
    TestEqual(TEXT("Brazul A path 1"), A.BrazulSubObjs[0].PathIndices[1], 65);

    const FKyaActorBehaviour& B = Actor.Behaviours[4];
    TestEqual(TEXT("Brazul B id"), B.Id, 27);
    TestTrue(TEXT("Brazul B is typed"), B.bHasTypedBrazulB);
    TestEqual(TEXT("Brazul B waypoint"), B.BrazulBWayPointRef, 71);
    if (!TestEqual(TEXT("Brazul B switch count"), B.BrazulBSwitch.Targets.Num(), 1)) return false;
    TestEqual(TEXT("Brazul B switch target"), B.BrazulBSwitch.Targets[0].TargetRef, 72);
    TestEqual(TEXT("Brazul B switch camera"), B.BrazulBSwitch.Camera.CameraIndex, 81);

    UKyaLevelBankExporter* Exporter = NewObject<UKyaLevelBankExporter>();
    TArray<uint8> Plain;
    FMemoryWriter PlainWriter(Plain);
    if (!TestTrue(TEXT("Export unedited Brazul fixture"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), PlainWriter, GWarn))) return false;
    TestEqual(TEXT("Unedited Brazul export keeps the file size"), Plain.Num(), Input.Num());
    TestTrue(TEXT("Unedited Brazul export preserves every byte"), Plain == Input);

    Actor.BrazulFieldFC0 = 91.5f;
    Actor.BrazulPathIndex = 92;
    Actor.BrazulFxHandles[4] = 93;
    Actor.BrazulMeshBMesh = 94;
    Actor.Behaviours[2].WolfenBehaviour.Flags = 95;
    Actor.Behaviours[3].BrazulSubObjs[0].Field8 = 96.5f;
    Actor.Behaviours[3].BrazulSubObjs[0].PathIndices[1] = 97;
    Actor.Behaviours[4].BrazulBSwitch.Targets[0].TargetRef = 98;
    Actor.Behaviours[4].BrazulBSwitch.Camera.CameraIndex = 99;
    TArray<uint8> Edited;
    FMemoryWriter EditedWriter(Edited);
    if (!TestTrue(TEXT("Export edited Brazul fixture"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), EditedWriter, GWarn))) return false;
    TestEqual(TEXT("Edited Brazul export keeps the file size"), Edited.Num(), Input.Num());
    const uint8* EditedCursor = Edited.GetData();
    UKyaLevelBank* Reimported = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
        GetTransientPackage(), NAME_None, RF_Transient, nullptr, TEXT("bnk"), EditedCursor,
        Edited.GetData() + Edited.Num(), GWarn));
    if (!TestNotNull(TEXT("Reimport edited Brazul fixture"), Reimported)) return false;
    const FKyaActorPlacement& EditedActor = Reimported->SceneEnvironment.Actors[0];
    TestEqual(TEXT("Edited Brazul field round trips"), EditedActor.BrazulFieldFC0, 91.5f);
    TestEqual(TEXT("Edited Brazul path round trips"), EditedActor.BrazulPathIndex, 92);
    TestEqual(TEXT("Edited Brazul fx round trips"), EditedActor.BrazulFxHandles[4], 93);
    TestEqual(TEXT("Edited Brazul mesh round trips"), EditedActor.BrazulMeshBMesh, 94);
    TestEqual(TEXT("Edited Brazul track flag round trips"), EditedActor.Behaviours[2].WolfenBehaviour.Flags, 95);
    TestEqual(TEXT("Edited Brazul A float round trips"), EditedActor.Behaviours[3].BrazulSubObjs[0].Field8, 96.5f);
    TestEqual(TEXT("Edited Brazul A path round trips"), EditedActor.Behaviours[3].BrazulSubObjs[0].PathIndices[1], 97);
    TestEqual(TEXT("Edited Brazul B target round trips"), EditedActor.Behaviours[4].BrazulBSwitch.Targets[0].TargetRef, 98);
    TestEqual(TEXT("Edited Brazul B camera round trips"), EditedActor.Behaviours[4].BrazulBSwitch.Camera.CameraIndex, 99);

    Actor.BrazulFxHandles.RemoveAt(4);
    TArray<uint8> Rejected;
    FMemoryWriter RejectedWriter(Rejected);
    TestFalse(TEXT("Exporter rejects a truncated Brazul fx list"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), RejectedWriter, GWarn));
    Actor.BrazulFxHandles.Add(100);
    Actor.Behaviours[3].BrazulSubObjs[0].PathIndices.Add(101);
    TArray<uint8> RejectedPath;
    FMemoryWriter RejectedPathWriter(RejectedPath);
    TestFalse(TEXT("Exporter rejects a changed Brazul A path count"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), RejectedPathWriter, GWarn));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKyaMiniGameTimeAttackFixtureRoundTripTest, "Kya.Bank.MiniGameTimeAttackFixtureRoundTrip",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

namespace
{
bool ImportSingleMiniGame(UKyaLevelBankFactory* Factory, const TArray<uint8>& Input,
    UKyaLevelBank*& Bank, FKyaActorPlacement*& Actor)
{
    const uint8* Cursor = Input.GetData();
    Bank = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
        GetTransientPackage(), NAME_None, RF_Transient, nullptr, TEXT("bnk"), Cursor,
        Input.GetData() + Input.Num(), GWarn));
    if (!Bank || Bank->SceneEnvironment.Actors.Num() != 1) return false;
    Actor = &Bank->SceneEnvironment.Actors[0];
    return true;
}
}

bool FKyaMiniGameTimeAttackFixtureRoundTripTest::RunTest(const FString& Parameters)
{
    FKyaSmallBankBuilder Builder;
    const TArray<uint8> Input = Builder.Build(0x45);
    UKyaLevelBankFactory* Factory = NewObject<UKyaLevelBankFactory>();
    UKyaLevelBank* Bank = nullptr;
    FKyaActorPlacement* ActorPtr = nullptr;
    if (!TestTrue(TEXT("Import minigame time-attack fixture"), ImportSingleMiniGame(Factory, Input, Bank, ActorPtr))) return false;
    FKyaActorPlacement& Actor = *ActorPtr;
    TestEqual(TEXT("Minigame class id"), Actor.ClassId, int64(0x45));
    TestEqual(TEXT("Minigame actor name"), Actor.Name, FString(TEXT("MINIGAMESTIMEATTACK")));
    TestTrue(TEXT("Minigame class payload parses"), Actor.bHasMiniGameData);
    TestEqual(TEXT("Minigame field 160"), Actor.MiniGameField160, int64((uint64(42) << 32) | 41));
    TestEqual(TEXT("Minigame field 168"), Actor.MiniGameField168, 43);
    TestEqual(TEXT("Minigame field 16c"), Actor.MiniGameField16C, 44);
    if (!TestEqual(TEXT("Minigame actor ref count"), Actor.MiniGameActorRefs.Num(), 1)) return false;
    TestEqual(TEXT("Minigame actor ref"), Actor.MiniGameActorRefs[0], 51);
    if (!TestEqual(TEXT("Minigame float count"), Actor.MiniGameFloats.Num(), 2)) return false;
    TestEqual(TEXT("Minigame float 0"), Actor.MiniGameFloats[0], 1.5f);
    if (!TestEqual(TEXT("Minigame score count"), Actor.MiniGameScores.Num(), 2)) return false;
    TestEqual(TEXT("Minigame score 0"), Actor.MiniGameScores[0].Score, 100.0f);
    TestEqual(TEXT("Minigame score 0 name"), Actor.MiniGameScores[0].Name, FString(TEXT("ABC")));
    TestEqual(TEXT("Minigame score 1 name"), Actor.MiniGameScores[1].Name, FString(TEXT("DE")));
    TestTrue(TEXT("Minigame default score present"), Actor.bMiniGameHasDefaultScore);
    TestEqual(TEXT("Minigame default score"), Actor.MiniGameDefaultScore.Score, 50.0f);
    TestEqual(TEXT("Minigame default name"), Actor.MiniGameDefaultScore.Name, FString(TEXT("NAT")));
    TestEqual(TEXT("Minigame waypoint"), Actor.MiniGameWayPointRef, 61);
    TestEqual(TEXT("Minigame field 190"), Actor.MiniGameField190, 62);
    TestEqual(TEXT("Minigame field 1a8"), Actor.MiniGameField1A8, int64((uint64(64) << 32) | 63));
    if (!TestEqual(TEXT("Minigame switch count"), Actor.MiniGameSwitch.Targets.Num(), 1)) return false;
    TestEqual(TEXT("Minigame switch target"), Actor.MiniGameSwitch.Targets[0].TargetRef, 71);
    TestEqual(TEXT("Minigame switch camera"), Actor.MiniGameSwitch.Camera.CameraIndex, 81);
    TestEqual(TEXT("Minigame time-attack field"), Actor.MiniGameTimeAttackField1E0, 91);
    TestEqual(TEXT("Minigame fixture leaves no raw class data"), Actor.ClassData.Num(), 0);
    if (!TestEqual(TEXT("Minigame behaviour count"), Actor.Behaviours.Num(), 3)) return false;
    TestEqual(TEXT("Minigame behaviour 0 id"), Actor.Behaviours[0].Id, 2);
    TestEqual(TEXT("Minigame behaviour 0 is empty"), Actor.Behaviours[0].Data.Num(), 0);
    TestEqual(TEXT("Minigame behaviour 1 id"), Actor.Behaviours[1].Id, 3);
    TestTrue(TEXT("Minigame betting is typed"), Actor.Behaviours[1].bHasTypedMiniGameBetting);
    if (!TestEqual(TEXT("Minigame bet count"), Actor.Behaviours[1].MiniGameBets.Num(), 3)) return false;
    TestEqual(TEXT("Minigame bet 0 cost"), Actor.Behaviours[1].MiniGameBets[0].Cost, 11);
    TestEqual(TEXT("Minigame bet 2 reward"), Actor.Behaviours[1].MiniGameBets[2].Reward, 16);
    TestEqual(TEXT("Minigame behaviour 2 id"), Actor.Behaviours[2].Id, 4);
    TestEqual(TEXT("Minigame behaviour 2 is empty"), Actor.Behaviours[2].Data.Num(), 0);

    UKyaLevelBankExporter* Exporter = NewObject<UKyaLevelBankExporter>();
    TArray<uint8> Plain;
    FMemoryWriter PlainWriter(Plain);
    if (!TestTrue(TEXT("Export unedited minigame fixture"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), PlainWriter, GWarn))) return false;
    TestEqual(TEXT("Unedited minigame export keeps the file size"), Plain.Num(), Input.Num());
    TestTrue(TEXT("Unedited minigame export preserves every byte"), Plain == Input);

    Actor.MiniGameScores[0].Score = 91.5f;
    Actor.MiniGameScores[0].Name = TEXT("XYZ");
    Actor.MiniGameDefaultScore.Score = 92.5f;
    Actor.MiniGameSwitch.Targets[0].TargetRef = 93;
    Actor.MiniGameTimeAttackField1E0 = 94;
    Actor.Behaviours[1].MiniGameBets[0].Cost = 95;
    TArray<uint8> Edited;
    FMemoryWriter EditedWriter(Edited);
    if (!TestTrue(TEXT("Export edited minigame fixture"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), EditedWriter, GWarn))) return false;
    TestEqual(TEXT("Edited minigame export keeps the file size"), Edited.Num(), Input.Num());
    const uint8* EditedCursor = Edited.GetData();
    UKyaLevelBank* Reimported = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
        GetTransientPackage(), NAME_None, RF_Transient, nullptr, TEXT("bnk"), EditedCursor,
        Edited.GetData() + Edited.Num(), GWarn));
    if (!TestNotNull(TEXT("Reimport edited minigame fixture"), Reimported)) return false;
    const FKyaActorPlacement& EditedActor = Reimported->SceneEnvironment.Actors[0];
    TestEqual(TEXT("Edited minigame score round trips"), EditedActor.MiniGameScores[0].Score, 91.5f);
    TestEqual(TEXT("Edited minigame score name round trips"), EditedActor.MiniGameScores[0].Name, FString(TEXT("XYZ")));
    TestEqual(TEXT("Edited minigame default score round trips"), EditedActor.MiniGameDefaultScore.Score, 92.5f);
    TestEqual(TEXT("Edited minigame switch round trips"), EditedActor.MiniGameSwitch.Targets[0].TargetRef, 93);
    TestEqual(TEXT("Edited minigame time-attack field round trips"), EditedActor.MiniGameTimeAttackField1E0, 94);
    TestEqual(TEXT("Edited minigame bet round trips"), EditedActor.Behaviours[1].MiniGameBets[0].Cost, 95);

    Actor.Behaviours[1].MiniGameBets.AddDefaulted();
    TArray<uint8> Rejected;
    FMemoryWriter RejectedWriter(Rejected);
    TestFalse(TEXT("Exporter rejects a changed minigame bet count"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), RejectedWriter, GWarn));
    Actor.Behaviours[1].MiniGameBets.RemoveAt(3);
    Actor.MiniGameScores.AddDefaulted();
    TArray<uint8> RejectedScores;
    FMemoryWriter RejectedScoresWriter(RejectedScores);
    TestFalse(TEXT("Exporter rejects a changed minigame score count"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), RejectedScoresWriter, GWarn));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKyaMiniGameBoomyFixtureRoundTripTest, "Kya.Bank.MiniGameBoomyFixtureRoundTrip",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKyaMiniGameBoomyFixtureRoundTripTest::RunTest(const FString& Parameters)
{
    FKyaSmallBankBuilder Builder;
    const TArray<uint8> Input = Builder.Build(0x47);
    UKyaLevelBankFactory* Factory = NewObject<UKyaLevelBankFactory>();
    UKyaLevelBank* Bank = nullptr;
    FKyaActorPlacement* ActorPtr = nullptr;
    if (!TestTrue(TEXT("Import minigame Boomy fixture"), ImportSingleMiniGame(Factory, Input, Bank, ActorPtr))) return false;
    FKyaActorPlacement& Actor = *ActorPtr;
    TestEqual(TEXT("Boomy class id"), Actor.ClassId, int64(0x47));
    TestEqual(TEXT("Boomy actor name"), Actor.Name, FString(TEXT("MINIGAMEBOOMY")));
    TestTrue(TEXT("Boomy class payload parses"), Actor.bHasMiniGameData);
    TestEqual(TEXT("Boomy time limit"), Actor.MiniGameTimeLimit, 20.0f);
    TestEqual(TEXT("Boomy max penalties"), Actor.MiniGameMaxPenalties, 3);
    TestEqual(TEXT("Boomy game flags"), Actor.MiniGameGameFlags, 5);
    if (!TestEqual(TEXT("Boomy target count"), Actor.MiniGameBoomyTargets.Num(), 1)) return false;
    TestEqual(TEXT("Boomy target actor"), Actor.MiniGameBoomyTargets[0].ActorRef, 101);
    if (!TestEqual(TEXT("Boomy target switch count"), Actor.MiniGameBoomyTargets[0].Switch.Targets.Num(), 1)) return false;
    TestEqual(TEXT("Boomy target switch"), Actor.MiniGameBoomyTargets[0].Switch.Targets[0].TargetRef, 111);
    TestEqual(TEXT("Boomy target camera"), Actor.MiniGameBoomyTargets[0].Switch.Camera.CameraIndex, 121);
    if (!TestEqual(TEXT("Boomy penalty count"), Actor.MiniGameBoomyPenaltyTargets.Num(), 1)) return false;
    TestEqual(TEXT("Boomy penalty actor"), Actor.MiniGameBoomyPenaltyTargets[0].ActorRef, 141);
    TestEqual(TEXT("Boomy penalty switch"), Actor.MiniGameBoomyPenaltyTargets[0].Switch.Targets[0].TargetRef, 151);
    if (!TestEqual(TEXT("Boomy sequence count"), Actor.MiniGameSequences.Num(), 1)) return false;
    if (!TestEqual(TEXT("Boomy sequence value count"), Actor.MiniGameSequences[0].TargetIndices.Num(), 2)) return false;
    TestEqual(TEXT("Boomy sequence value 0"), Actor.MiniGameSequences[0].TargetIndices[0], 131);
    TestEqual(TEXT("Boomy sequence value 1"), Actor.MiniGameSequences[0].TargetIndices[1], 132);
    TestEqual(TEXT("Boomy fixture leaves no raw class data"), Actor.ClassData.Num(), 0);
    if (!TestEqual(TEXT("Boomy behaviour count"), Actor.Behaviours.Num(), 3)) return false;
    TestTrue(TEXT("Boomy betting is typed"), Actor.Behaviours[1].bHasTypedMiniGameBetting);

    UKyaLevelBankExporter* Exporter = NewObject<UKyaLevelBankExporter>();
    TArray<uint8> Plain;
    FMemoryWriter PlainWriter(Plain);
    if (!TestTrue(TEXT("Export unedited Boomy fixture"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), PlainWriter, GWarn))) return false;
    TestEqual(TEXT("Unedited Boomy export keeps the file size"), Plain.Num(), Input.Num());
    TestTrue(TEXT("Unedited Boomy export preserves every byte"), Plain == Input);

    Actor.MiniGameTimeLimit = 91.5f;
    Actor.MiniGameBoomyTargets[0].ActorRef = 92;
    Actor.MiniGameBoomyTargets[0].Switch.Targets[0].TargetRef = 93;
    Actor.MiniGameBoomyPenaltyTargets[0].Switch.Camera.CameraIndex = 94;
    Actor.MiniGameSequences[0].TargetIndices[1] = 95;
    TArray<uint8> Edited;
    FMemoryWriter EditedWriter(Edited);
    if (!TestTrue(TEXT("Export edited Boomy fixture"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), EditedWriter, GWarn))) return false;
    TestEqual(TEXT("Edited Boomy export keeps the file size"), Edited.Num(), Input.Num());
    const uint8* EditedCursor = Edited.GetData();
    UKyaLevelBank* Reimported = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
        GetTransientPackage(), NAME_None, RF_Transient, nullptr, TEXT("bnk"), EditedCursor,
        Edited.GetData() + Edited.Num(), GWarn));
    if (!TestNotNull(TEXT("Reimport edited Boomy fixture"), Reimported)) return false;
    const FKyaActorPlacement& EditedActor = Reimported->SceneEnvironment.Actors[0];
    TestEqual(TEXT("Edited Boomy time limit round trips"), EditedActor.MiniGameTimeLimit, 91.5f);
    TestEqual(TEXT("Edited Boomy target round trips"), EditedActor.MiniGameBoomyTargets[0].ActorRef, 92);
    TestEqual(TEXT("Edited Boomy target switch round trips"),
        EditedActor.MiniGameBoomyTargets[0].Switch.Targets[0].TargetRef, 93);
    TestEqual(TEXT("Edited Boomy penalty camera round trips"),
        EditedActor.MiniGameBoomyPenaltyTargets[0].Switch.Camera.CameraIndex, 94);
    TestEqual(TEXT("Edited Boomy sequence round trips"), EditedActor.MiniGameSequences[0].TargetIndices[1], 95);

    Actor.MiniGameSequences[0].TargetIndices.Add(96);
    TArray<uint8> Rejected;
    FMemoryWriter RejectedWriter(Rejected);
    TestFalse(TEXT("Exporter rejects a changed Boomy sequence length"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), RejectedWriter, GWarn));
    Actor.MiniGameSequences[0].TargetIndices.RemoveAt(2);
    Actor.MiniGameBoomyTargets.AddDefaulted();
    TArray<uint8> RejectedTargets;
    FMemoryWriter RejectedTargetsWriter(RejectedTargets);
    TestFalse(TEXT("Exporter rejects a changed Boomy target count"),
        Exporter->ExportBinary(Bank, TEXT("bnk"), RejectedTargetsWriter, GWarn));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKyaMiniGameVariantsFixtureRoundTripTest, "Kya.Bank.MiniGameVariantsFixtureRoundTrip",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKyaMiniGameVariantsFixtureRoundTripTest::RunTest(const FString& Parameters)
{
    UKyaLevelBankFactory* Factory = NewObject<UKyaLevelBankFactory>();
    UKyaLevelBankExporter* Exporter = NewObject<UKyaLevelBankExporter>();
    FKyaSmallBankBuilder Builder;

    {
        const TArray<uint8> Input = Builder.Build(0x44);
        UKyaLevelBank* Bank = nullptr;
        FKyaActorPlacement* ActorPtr = nullptr;
        if (!TestTrue(TEXT("Import base minigame fixture"), ImportSingleMiniGame(Factory, Input, Bank, ActorPtr))) return false;
        TestTrue(TEXT("Base minigame payload parses"), ActorPtr->bHasMiniGameData);
        TestEqual(TEXT("Base minigame score count"), ActorPtr->MiniGameScores.Num(), 2);
        TestEqual(TEXT("Base minigame fixture leaves no raw class data"), ActorPtr->ClassData.Num(), 0);
        TArray<uint8> Plain;
        FMemoryWriter Writer(Plain);
        if (!TestTrue(TEXT("Export base minigame fixture"), Exporter->ExportBinary(Bank, TEXT("bnk"), Writer, GWarn))) return false;
        TestTrue(TEXT("Base minigame export preserves every byte"), Plain == Input);
    }
    {
        const TArray<uint8> Input = Builder.Build(0x46);
        UKyaLevelBank* Bank = nullptr;
        FKyaActorPlacement* ActorPtr = nullptr;
        if (!TestTrue(TEXT("Import box-counter fixture"), ImportSingleMiniGame(Factory, Input, Bank, ActorPtr))) return false;
        TestTrue(TEXT("Box-counter payload parses"), ActorPtr->bHasMiniGameData);
        TestEqual(TEXT("Box-counter time limit"), ActorPtr->MiniGameTimeLimit, 20.0f);
        TestEqual(TEXT("Box-counter fixture leaves no raw class data"), ActorPtr->ClassData.Num(), 0);
        ActorPtr->MiniGameTimeLimit = 91.5f;
        TArray<uint8> Edited;
        FMemoryWriter Writer(Edited);
        if (!TestTrue(TEXT("Export edited box-counter fixture"), Exporter->ExportBinary(Bank, TEXT("bnk"), Writer, GWarn))) return false;
        const uint8* Cursor = Edited.GetData();
        UKyaLevelBank* Reimported = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
            GetTransientPackage(), NAME_None, RF_Transient, nullptr, TEXT("bnk"), Cursor,
            Edited.GetData() + Edited.Num(), GWarn));
        if (!TestNotNull(TEXT("Reimport edited box-counter fixture"), Reimported)) return false;
        TestEqual(TEXT("Edited box-counter time limit round trips"),
            Reimported->SceneEnvironment.Actors[0].MiniGameTimeLimit, 91.5f);
    }
    return true;
}

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
        TArray<uint8> HedgehogData;
        HedgehogData.SetNumZeroed(84);
        auto WriteU32 = [&HedgehogData](int32 Offset, uint32 Value) {
            for (int32 Byte = 0; Byte < 4; ++Byte) HedgehogData[Offset + Byte] = uint8(Value >> (Byte * 8));
        };
        WriteU32(76, 0);
        FKyaHedgehogOffsets Hedgehog;
        if (TestTrue(TEXT("Read Hedgehog fixture without sub-object"), KyaReadHedgehogOffsets(HedgehogData, 0, HedgehogData.Num(), Hedgehog)))
        {
            TestEqual(TEXT("Hedgehog weight offset"), Hedgehog.WeightB, int64(0));
            TestEqual(TEXT("Hedgehog autonomous offset"), Hedgehog.Autonomous, int64(4));
            TestEqual(TEXT("Hedgehog walk speed offset"), Hedgehog.WalkSpeed, int64(20));
            TestEqual(TEXT("Hedgehog field 374 offset"), Hedgehog.Fields374, int64(36));
            TestEqual(TEXT("Hedgehog floats 368 offset"), Hedgehog.Floats368, int64(44));
            TestEqual(TEXT("Hedgehog add-on offset"), Hedgehog.AddOn, int64(56));
            TestEqual(TEXT("Hedgehog sub-object mode"), Hedgehog.SubObjModeValue, 0);
            TestEqual(TEXT("Hedgehog field 380 offset"), Hedgehog.Field380, int64(80));
            TestEqual(TEXT("Hedgehog fixture end"), Hedgehog.Tail, int64(84));
        }
        HedgehogData.SetNumZeroed(104);
        WriteU32(76, 1);
        TestTrue(TEXT("Read Hedgehog fixture with sub-object"), KyaReadHedgehogOffsets(HedgehogData, 0, HedgehogData.Num(), Hedgehog));
        TestEqual(TEXT("Hedgehog sub-object floats offset"), Hedgehog.SubObjFloats, int64(80));
        TestEqual(TEXT("Hedgehog field 380 after sub-object"), Hedgehog.Field380, int64(100));
        TestEqual(TEXT("Hedgehog fixture end with sub-object"), Hedgehog.Tail, int64(104));
        HedgehogData.SetNum(103);
        TestFalse(TEXT("Reject truncated Hedgehog sub-object"), KyaReadHedgehogOffsets(HedgehogData, 0, HedgehogData.Num(), Hedgehog));
        HedgehogData.SetNum(83);
        TestFalse(TEXT("Reject truncated Hedgehog prefix"), KyaReadHedgehogOffsets(HedgehogData, 0, HedgehogData.Num(), Hedgehog));
    }
    {
        TArray<uint8> Behaviour;
        Behaviour.SetNumZeroed(4);
        auto WriteU32 = [&Behaviour](int32 Offset, uint32 Value) {
            for (int32 Byte = 0; Byte < 4; ++Byte) Behaviour[Offset + Byte] = uint8(Value >> (Byte * 8));
        };
        WriteU32(0, 42);
        FKyaHedgehogBehaviourValues Values;
        if (TestTrue(TEXT("Read Hedgehog guard-area behaviour"), KyaReadHedgehogBehaviour(3, Behaviour, 0, 4, Values)))
            TestEqual(TEXT("Hedgehog guard-area path index"), Values.PathFollowIndex, 42);
        if (TestTrue(TEXT("Read Hedgehog guard-area-armor behaviour"), KyaReadHedgehogBehaviour(5, Behaviour, 0, 4, Values)))
            TestEqual(TEXT("Hedgehog guard-area-armor path index"), Values.PathFollowIndex, 42);
        if (TestTrue(TEXT("Read Hedgehog watchdog behaviour"), KyaReadHedgehogBehaviour(4, Behaviour, 0, 0, Values)))
            TestTrue(TEXT("Hedgehog watchdog payload is empty"), Values.bEmptyPayload);
        if (TestTrue(TEXT("Read Hedgehog watchdog-armor behaviour"), KyaReadHedgehogBehaviour(6, Behaviour, 0, 0, Values)))
            TestTrue(TEXT("Hedgehog watchdog-armor payload is empty"), Values.bEmptyPayload);
        TestFalse(TEXT("Reject empty Hedgehog guard-area payload"), KyaReadHedgehogBehaviour(3, Behaviour, 0, 0, Values));
        TestFalse(TEXT("Reject non-empty Hedgehog watchdog payload"), KyaReadHedgehogBehaviour(4, Behaviour, 0, 4, Values));
        TestFalse(TEXT("Reject inherited Hedgehog behaviour id"), KyaReadHedgehogBehaviour(2, Behaviour, 0, 4, Values));
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
        ProjectileData.SetNum(137);
        TestFalse(TEXT("Reject projectile payload with an unknown tail"), KyaReadProjectileOffsets(ProjectileData, 0, ProjectileData.Num(), Projectile));
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
        TrapData.SetNumZeroed(112);
        WriteU32(48, 1);
        if (TestTrue(TEXT("Read legacy trap fixture with stream entry"),
            KyaReadTrapOffsets(TrapData, 0, TrapData.Num(), Trap)))
        {
            TestFalse(TEXT("Legacy trap with stream omits field 24"), Trap.bHasField24);
            TestEqual(TEXT("Legacy trap stream count"), Trap.TrapCount, 1);
            TestEqual(TEXT("Legacy trap stream offset"), Trap.TrapEntries, int64(52));
            TestEqual(TEXT("Legacy trap stream fixture end"), Trap.Tail, int64(112));
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
        FKyaSwitchActorOffsets Switch;
        if (TestTrue(TEXT("Read switch fixture with three targets"), KyaReadSwitchActorOffsets(SwitchData, 0, SwitchData.Num(), Switch)))
        {
            TestEqual(TEXT("Switch target count"), Switch.Count, 3);
            TestEqual(TEXT("Switch target offset"), Switch.Targets, int64(4));
            TestEqual(TEXT("Switch camera offset"), Switch.Camera, int64(88));
            TestEqual(TEXT("Switch fixture end"), Switch.Tail, int64(120));
        }
        WriteU32(0, 0);
        if (TestTrue(TEXT("Read switch fixture without targets"), KyaReadSwitchActorOffsets(SwitchData, 0, 36, Switch)))
        {
            TestEqual(TEXT("Switch empty camera offset"), Switch.Camera, int64(4));
            TestEqual(TEXT("Switch empty fixture end"), Switch.Tail, int64(36));
        }
        TestFalse(TEXT("Reject truncated switch camera"), KyaReadSwitchActorOffsets(SwitchData, 0, 35, Switch));
        WriteU32(0, 4);
        TestFalse(TEXT("Reject switch target count beyond payload"), KyaReadSwitchActorOffsets(SwitchData, 0, SwitchData.Num(), Switch));
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
        FKyaFruitInventoryOffsets Values;
        if (TestTrue(TEXT("Read fruit inventory fixture"), KyaReadFruitInventoryOffsets(Inventory, 0, Inventory.Num(), Values)))
        {
            TestTrue(TEXT("Fruit inventory has data"), Values.bHasData);
            TestFalse(TEXT("Fruit inventory has no object"), Values.bHasObject);
            TestEqual(TEXT("Fruit inventory initial behaviour offset"), Values.InitialBehaviour, int64(40));
        }
        Inventory.SetNum(43);
        TestFalse(TEXT("Reject truncated fruit inventory"), KyaReadFruitInventoryOffsets(Inventory, 0, Inventory.Num(), Values));
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
        if (TestTrue(TEXT("Read DCA controlled behaviour"), KyaReadDcaBehaviourOffsets(3, Controlled, 0, Controlled.Num(), Off)))
        {
            TestTrue(TEXT("DCA controlled flag"), Off.bControlled);
            TestEqual(TEXT("DCA controlled end"), Off.Tail, int64(40));
        }
        Controlled.SetNum(39);
        TestFalse(TEXT("Reject truncated DCA controlled behaviour"), KyaReadDcaBehaviourOffsets(3, Controlled, 0, Controlled.Num(), Off));
        TArray<uint8> Trajectory;
        Trajectory.SetNumZeroed(12);
        if (TestTrue(TEXT("Read DCA trajectory behaviour"), KyaReadDcaBehaviourOffsets(5, Trajectory, 0, Trajectory.Num(), Off)))
        {
            TestTrue(TEXT("DCA trajectory flag"), Off.bTrajectory);
            TestEqual(TEXT("DCA trajectory end"), Off.Tail, int64(12));
        }
        Trajectory.SetNum(11);
        TestFalse(TEXT("Reject truncated DCA trajectory behaviour"), KyaReadDcaBehaviourOffsets(5, Trajectory, 0, Trajectory.Num(), Off));
        TestFalse(TEXT("Reject unknown DCA behaviour"), KyaReadDcaBehaviourOffsets(4, Controlled, 0, Controlled.Num(), Off));
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
        FKyaFountainOffsets Fountain;
        if (TestTrue(TEXT("Read bonus fountain fixture"), KyaReadFountainOffsets(FountainData, 0, FountainData.Num(), Fountain)))
        {
            TestEqual(TEXT("Bonus fountain add-on offset"), Fountain.AddOn, int64(0));
            TestEqual(TEXT("Bonus fountain mode offset"), Fountain.SubObjMode, int64(20));
            TestEqual(TEXT("Bonus fountain fixture end"), Fountain.Tail, int64(24));
        }
        FountainData.SetNumZeroed(44);
        FountainData[20] = 1;
        if (TestTrue(TEXT("Read bonus fountain sub-object fixture"), KyaReadFountainOffsets(FountainData, 0, FountainData.Num(), Fountain)))
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
            TestEqual(TEXT("Gravity bell switch camera"), Bell.SwitchCamera, int64(12));
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
            TestEqual(TEXT("Boomy camera offset"), Boomy.CameraStart, int64(72));
            TestEqual(TEXT("Boomy camera fov offset"), Boomy.Camera.Fov, int64(72));
            TestEqual(TEXT("Boomy camera position offset"), Boomy.Camera.Position, int64(76));
            TestEqual(TEXT("Boomy camera look-at offset"), Boomy.Camera.LookAt, int64(88));
            TestEqual(TEXT("Boomy camera up offset"), Boomy.Camera.Up, int64(100));
            TestEqual(TEXT("Boomy camera sector offset"), Boomy.Camera.Sector, int64(112));
            TestEqual(TEXT("Boomy camera flags offset"), Boomy.Camera.Flags, int64(120));
            TestEqual(TEXT("Boomy camera target offset"), Boomy.Camera.TargetOffset, int64(124));
            TestEqual(TEXT("Boomy camera conditions offset"), Boomy.Camera.Conditions, int64(136));
            TestEqual(TEXT("Boomy camera config offset"), Boomy.Camera.Config, int64(140));
            TestEqual(TEXT("Boomy camera config angles offset"), Boomy.Camera.ConfigAngles, int64(160));
            TestEqual(TEXT("Boomy camera extra offset"), Boomy.Camera.Extra, int64(196));
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
            TestTrue(TEXT("Boomy camera has switch"), Boomy.Camera.bHasSwitch);
            TestEqual(TEXT("Boomy switch actor offset"), Boomy.Camera.SwitchActor, int64(136));
            TestEqual(TEXT("Boomy switch list A offset"), Boomy.Camera.SwitchListA, int64(144));
            TestEqual(TEXT("Boomy switch list A count"), Boomy.Camera.SwitchCountA, 1);
            TestEqual(TEXT("Boomy switch list B offset"), Boomy.Camera.SwitchListB, int64(152));
            TestEqual(TEXT("Boomy switch list B count"), Boomy.Camera.SwitchCountB, 0);
            TestEqual(TEXT("Boomy switch values offset"), Boomy.Camera.SwitchValues, int64(152));
            TestEqual(TEXT("Boomy switch conditions offset"), Boomy.Camera.Conditions, int64(172));
            TestEqual(TEXT("Boomy switch config offset"), Boomy.Camera.Config, int64(176));
            TestEqual(TEXT("Boomy config mode"), Boomy.Camera.ConfigMode, 1);
            TestEqual(TEXT("Boomy optional config offset"), Boomy.Camera.ConfigOptional, int64(196));
            TestEqual(TEXT("Boomy switch extra offset"), Boomy.Camera.Extra, int64(252));
            TestEqual(TEXT("Boomy extra count"), Boomy.Camera.ExtraCount, 1);
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
        TArray<uint8> SelectorEmptyData;
        SelectorEmptyData.SetNumZeroed(64);
        if (TestTrue(TEXT("Read platform selector-master without lists"),
            KyaReadPlatformBehaviourOffsets(8, SelectorEmptyData, 0, 64, Slab)))
        {
            TestTrue(TEXT("Platform empty selector-master flag"), Slab.bSelectorMaster);
            TestEqual(TEXT("Platform empty selector-master actor count"), Slab.SelectorActorCountValue, 0);
            TestEqual(TEXT("Platform empty selector-master switch count"), Slab.SelectorSwitchCountValue, 0);
            TestEqual(TEXT("Platform empty selector-master camera offset"), Slab.SelectorCamera, int64(32));
            TestEqual(TEXT("Platform empty selector-master end"), Slab.Tail, int64(64));
        }
        TArray<uint8> TeleportData;
        TeleportData.SetNumZeroed(4);
        TeleportData[0] = 33;
        if (TestTrue(TEXT("Read platform teleport-random behaviour"),
            KyaReadPlatformBehaviourOffsets(10, TeleportData, 0, 4, Slab)))
        {
            TestTrue(TEXT("Platform teleport-random flag"), Slab.bTeleportRandom);
            TestEqual(TEXT("Platform teleport-random path offset"), Slab.PathIndex, int64(0));
            TestEqual(TEXT("Platform teleport-random end"), Slab.Tail, int64(4));
        }
        TestFalse(TEXT("Reject wrong-length teleport-random payload"),
            KyaReadPlatformBehaviourOffsets(10, TeleportData, 0, 8, Slab));
    }
    {
        TArray<uint8> ProjectileBehaviourData;
        ProjectileBehaviourData.SetNumZeroed(44);
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
        if (TestTrue(TEXT("Read projectile lava ball behaviour"),
            KyaReadProjectileBehaviourOffsets(10, ProjectileBehaviourData, 0, 40, Projectile)))
        {
            TestTrue(TEXT("Projectile lava ball flag"), Projectile.bLavaBall);
            TestFalse(TEXT("Old lava ball omits field 18"), Projectile.bHasLavaBallField18);
            TestEqual(TEXT("Lava ball field 14 offset"), Projectile.LavaBallField14, int64(12));
            TestEqual(TEXT("Lava ball field 30 offset"), Projectile.LavaBallField30, int64(16));
            TestEqual(TEXT("Lava ball path index offset"), Projectile.LavaBallPathIndex, int64(20));
            TestEqual(TEXT("Lava ball field 2c offset"), Projectile.LavaBallField2C, int64(24));
            TestEqual(TEXT("Lava ball field 3c offset"), Projectile.LavaBallField3C, int64(36));
            TestEqual(TEXT("Old lava ball end"), Projectile.Tail, int64(40));
        }
        if (TestTrue(TEXT("Read projectile lava ball with field 18"),
            KyaReadProjectileBehaviourOffsets(10, ProjectileBehaviourData, 0, 44, Projectile)))
        {
            TestTrue(TEXT("New lava ball has field 18"), Projectile.bHasLavaBallField18);
            TestEqual(TEXT("Lava ball field 18 offset"), Projectile.LavaBallField18, int64(20));
            TestEqual(TEXT("Shifted lava ball path index offset"), Projectile.LavaBallPathIndex, int64(24));
            TestEqual(TEXT("Shifted lava ball field 3c offset"), Projectile.LavaBallField3C, int64(40));
            TestEqual(TEXT("New lava ball end"), Projectile.Tail, int64(44));
        }
        if (TestTrue(TEXT("Read projectile inactive behaviour"),
            KyaReadProjectileBehaviourOffsets(12, ProjectileBehaviourData, 0, 12, Projectile)))
        {
            TestTrue(TEXT("Projectile inactive flag"), Projectile.bInactive);
            TestEqual(TEXT("Inactive activate message offset"), Projectile.InactiveActivateMessageId, int64(0));
            TestEqual(TEXT("Inactive flags offset"), Projectile.InactiveFlags, int64(4));
            TestEqual(TEXT("Inactive active behaviour offset"), Projectile.InactiveActiveBehaviourId, int64(8));
        }
        if (TestTrue(TEXT("Read projectile new behaviour"),
            KyaReadProjectileBehaviourOffsets(13, ProjectileBehaviourData, 0, 4, Projectile)))
        {
            TestTrue(TEXT("Projectile new flag"), Projectile.bNew);
            TestEqual(TEXT("Projectile new end"), Projectile.Tail, int64(4));
        }
        TestFalse(TEXT("Reject wrong-length projectile stand"), KyaReadProjectileBehaviourOffsets(3, ProjectileBehaviourData, 0, 20, Projectile));
        TestFalse(TEXT("Reject unknown projectile behaviour"), KyaReadProjectileBehaviourOffsets(6, ProjectileBehaviourData, 0, 20, Projectile));
        TestFalse(TEXT("Reject wrong-length lava ball"), KyaReadProjectileBehaviourOffsets(10, ProjectileBehaviourData, 0, 20, Projectile));
        TestFalse(TEXT("Reject wrong-length inactive"), KyaReadProjectileBehaviourOffsets(12, ProjectileBehaviourData, 0, 16, Projectile));
        TestFalse(TEXT("Reject wrong-length new projectile"), KyaReadProjectileBehaviourOffsets(13, ProjectileBehaviourData, 0, 8, Projectile));
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
        if (TestTrue(TEXT("Read switch Wolfen-counter behaviour"),
            KyaReadSwitchBehaviourOffsets(4, SwitchBehaviourData, 0, 20, Switch)))
        {
            TestTrue(TEXT("Switch Wolfen-counter flag"), Switch.bWolfen);
            TestEqual(TEXT("Wolfen trigger radius offset"), Switch.WolfenTriggerRadius, int64(4));
            TestEqual(TEXT("Wolfen material offset"), Switch.WolfenMaterialId, int64(16));
            TestEqual(TEXT("Switch Wolfen-counter end"), Switch.Tail, int64(20));
        }
        TArray<uint8> SequenceData;
        SequenceData.SetNumZeroed(24);
        SequenceData[8] = 3;
        if (TestTrue(TEXT("Read switch sequence behaviour"),
            KyaReadSwitchBehaviourOffsets(7, SequenceData, 0, 24, Switch)))
        {
            TestTrue(TEXT("Switch sequence flag"), Switch.bSequence);
            TestEqual(TEXT("Switch sequence value count"), Switch.SequenceCountValue, 3);
            TestEqual(TEXT("Switch sequence values offset"), Switch.SequenceValues, int64(12));
            TestEqual(TEXT("Switch sequence end"), Switch.Tail, int64(24));
        }
        TArray<uint8> TimedData;
        TimedData.SetNumZeroed(124);
        TimedData[4] = 2;   // two entries
        TimedData[48] = 1;  // second entry has one target
        if (TestTrue(TEXT("Read switch timed behaviour"),
            KyaReadSwitchBehaviourOffsets(8, TimedData, 0, 124, Switch)))
        {
            TestTrue(TEXT("Switch timed flag"), Switch.bTimed);
            TestEqual(TEXT("Switch timed entry count"), Switch.TimedEntryCountValue, 2);
            TestEqual(TEXT("Switch timed entry 0 count offset"), Switch.TimedEntries[0].Count, int64(8));
            TestEqual(TEXT("Switch timed entry 0 delay offset"), Switch.TimedEntries[0].Delay, int64(44));
            TestEqual(TEXT("Switch timed entry 1 camera offset"), Switch.TimedEntries[1].Camera, int64(80));
            TestEqual(TEXT("Switch timed playback offset"), Switch.TimedPlaybackMode, int64(116));
            TestEqual(TEXT("Switch timed flags offset"), Switch.TimedFlags, int64(120));
            TestEqual(TEXT("Switch timed end"), Switch.Tail, int64(124));
        }
        TestFalse(TEXT("Reject wrong-length switch lever"), KyaReadSwitchBehaviourOffsets(2, SwitchBehaviourData, 0, 76, Switch));
        TestFalse(TEXT("Reject unknown switch behaviour"), KyaReadSwitchBehaviourOffsets(9, SwitchBehaviourData, 0, 76, Switch));
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
        FKyaProjectileInventoryOffsets Inventory;
        if (TestTrue(TEXT("Read projectile inventory fixture"), KyaReadProjectileInventoryOffsets(InventoryData, 0, 44, Inventory)))
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
        if (TestTrue(TEXT("Read projectile inventory with object"), KyaReadProjectileInventoryOffsets(InventoryData, 0, 56, Inventory)))
        {
            TestTrue(TEXT("Inventory has object block"), Inventory.bHasObject);
            TestEqual(TEXT("Inventory object id offset"), Inventory.ObjectId, int64(40));
            TestEqual(TEXT("Inventory object field 18 offset"), Inventory.Field18, int64(44));
            TestEqual(TEXT("Inventory object end"), Inventory.Tail, int64(56));
        }
        InventoryData.SetNumZeroed(8);
        InventoryData[0] = 0;
        if (TestTrue(TEXT("Read empty inventory fixture"), KyaReadProjectileInventoryOffsets(InventoryData, 0, 8, Inventory)))
        {
            TestEqual(TEXT("Empty inventory initial behaviour offset"), Inventory.InitialBehaviourId, int64(4));
            TestEqual(TEXT("Empty inventory end"), Inventory.Tail, int64(8));
        }
        InventoryData.SetNum(43);
        TestFalse(TEXT("Reject truncated inventory"), KyaReadProjectileInventoryOffsets(InventoryData, 0, 43, Inventory));
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
            // Direct FactoryCreateBinary bypasses the import pipeline's filename;
            // sector render settings are keyed by the actual SECTn source path.
            Sector->SourceFilename = SectorFilename;
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
            TestEqual(TEXT("Sector one's own flag keeps its background fixed despite common follow flag"), MovedVertices, 0);
            Bank->SceneEnvironment.BackgroundFlags &= ~int64(1);
            Preview->OnConstruction(FTransform::Identity);
            const TArray<FVector> FixedBackground = CollectPreviewPositions();
            Preview->SetBackgroundCameraPosition(FVector(1200, 3400, 5600));
            const TArray<FVector> FixedAfterCamera = CollectPreviewPositions();
            TestTrue(TEXT("Disabled background flag keeps all vertices fixed"), FixedBackground == FixedAfterCamera);
            Bank->SceneEnvironment.BackgroundFlags = OriginalBackgroundFlags;
            Preview->OnConstruction(FTransform::Identity);
            bool HasMaskedMaterial = false;
            bool HasTranslucentMaterial = false;
            for (int32 Section = 1; Section < Preview->Mesh->GetNumSections(); ++Section)
                if (UMaterialInterface* Material = Preview->Mesh->GetMaterial(Section))
                {
                    HasMaskedMaterial |= Material->GetBlendMode() == BLEND_Masked;
                    HasTranslucentMaterial |= Material->GetBlendMode() == BLEND_AlphaComposite;
                    if (Material->GetBlendMode() == BLEND_Additive)
                        if (const auto* Instance = Cast<UMaterialInstanceDynamic>(Material))
                        {
                            float Factor = 0, SourceAlpha = 0;
                            Instance->GetScalarParameterValue(FMaterialParameterInfo(TEXT("BlendFactor")), Factor);
                            Instance->GetScalarParameterValue(FMaterialParameterInfo(TEXT("UseSourceAlpha")), SourceAlpha);
                            TestEqual(TEXT("Level-one ALPHA 0x48 uses source alpha"), SourceAlpha, 1.0f);
                            TestEqual(TEXT("Unused FIX=30 must not dim ALPHA 0x48"), Factor, 1.0f);
                        }
                }
            TestTrue(TEXT("Alpha-tested sector materials use masked preview"), HasMaskedMaterial);
            TestTrue(TEXT("Source-alpha level surfaces retain transparency"), HasTranslucentMaterial);
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
            if (SectorNumber == 2)
            {
                TArray<FKyaG3DPreparedSprite> Sprites;
                const bool bSprites = KyaG3D::ReadClusterSprites(MeshData, Sprites);
                int32 SpriteRects = 0;
                for (const FKyaG3DPreparedSprite& Sprite : Sprites) SpriteRects += Sprite.Rects.Num();
                AddInfo(FString::Printf(TEXT("SECT2 sprites: ok=%d sprites=%d rects=%d"), bSprites, Sprites.Num(), SpriteRects));
                TestTrue(TEXT("Read SECT2 prepared sprites"), bSprites);
                TestEqual(TEXT("SECT2 sprite count"), Sprites.Num(), 3);
                {
                    int32 SpriteBlended = 0, SpriteMasked = 0;
                    for (const FKyaG3DPreparedSprite& Sprite : Sprites)
                    {
                        const int32* Offset = Materials.Find(Sprite.MaterialHash);
                        if (!Offset) continue;
                        uint64 Prim = 0, Test = 0;
                        if (KyaG2D::ReadMaterialRegister(TextureData, *Offset, 0, 0x00, Prim) && ((Prim >> 6) & 1)) ++SpriteBlended;
                        if (KyaG2D::ReadMaterialRegister(TextureData, *Offset, 0, 0x47, Test) && (Test & 1)) ++SpriteMasked;
                    }
                    AddInfo(FString::Printf(TEXT("SECT2 sprite materials: blended=%d masked=%d"), SpriteBlended, SpriteMasked));
                    TestEqual(TEXT("SECT2 sprite materials are opaque"), SpriteBlended, 0);
                    TestTrue(TEXT("SECT2 sprite materials use alpha test"), SpriteMasked > 0);
                }
                if (Sprites.Num() >= 3)
                {
                    TestEqual(TEXT("SECT2 sprite 0 rects"), Sprites[0].Rects.Num(), 2);
                    TestEqual(TEXT("SECT2 sprite 1 rects"), Sprites[1].Rects.Num(), 1);
                    TestEqual(TEXT("SECT2 sprite 2 rects"), Sprites[2].Rects.Num(), 2);
                    if (Sprites[0].Rects.Num() > 0)
                    {
                        const FKyaG3DSpriteRect& Rect = Sprites[0].Rects[0];
                        AddInfo(FString::Printf(TEXT("SECT2 sprite 0 center=(%.3f,%.3f,%.3f) half=(%.4f,%.4f) hash=%016llx"),
                            Rect.Center.X, Rect.Center.Y, Rect.Center.Z, Rect.HalfSize.X, Rect.HalfSize.Y,
                            Sprites[0].MaterialHash));
                        TestTrue(TEXT("SECT2 sprite 0 center x"), FMath::Abs(Rect.Center.X + 90.836f) < 0.05f);
                        TestTrue(TEXT("SECT2 sprite 0 center y"), FMath::Abs(Rect.Center.Y + 311.572f) < 0.05f);
                        TestTrue(TEXT("SECT2 sprite 0 center z"), FMath::Abs(Rect.Center.Z + 687.901f) < 0.05f);
                        TestTrue(TEXT("SECT2 sprite 0 half width"), FMath::Abs(Rect.HalfSize.X - 0.0415f) < 0.002f);
                        TestTrue(TEXT("SECT2 sprite 0 half height"), FMath::Abs(Rect.HalfSize.Y - 0.0698f) < 0.002f);
                        TestTrue(TEXT("SECT2 sprite material resolves"), Materials.Contains(Sprites[0].MaterialHash));
                    }
                }
            }
            if (SectorNumber == 4)
            {
                TArray<FKyaG3DPreparedSprite> Sprites;
                const bool bSprites = KyaG3D::ReadClusterSprites(MeshData, Sprites);
                int32 SpriteRects = 0;
                for (const FKyaG3DPreparedSprite& Sprite : Sprites) SpriteRects += Sprite.Rects.Num();
                AddInfo(FString::Printf(TEXT("SECT4 sprites: ok=%d sprites=%d rects=%d"), bSprites, Sprites.Num(), SpriteRects));
                TestTrue(TEXT("Read SECT4 prepared sprites"), bSprites);
                TestEqual(TEXT("SECT4 sprite count"), Sprites.Num(), 5);
                TestEqual(TEXT("SECT4 sprite rect count"), SpriteRects, 7);
                {
                    int32 SpriteBlended = 0, SpriteMasked = 0;
                    for (const FKyaG3DPreparedSprite& Sprite : Sprites)
                    {
                        const int32* Offset = Materials.Find(Sprite.MaterialHash);
                        if (!Offset) continue;
                        uint64 Prim = 0, Test = 0;
                        if (KyaG2D::ReadMaterialRegister(TextureData, *Offset, 0, 0x00, Prim) && ((Prim >> 6) & 1)) ++SpriteBlended;
                        if (KyaG2D::ReadMaterialRegister(TextureData, *Offset, 0, 0x47, Test) && (Test & 1)) ++SpriteMasked;
                    }
                    AddInfo(FString::Printf(TEXT("SECT4 sprite materials: blended=%d masked=%d"), SpriteBlended, SpriteMasked));
                    TestEqual(TEXT("SECT4 sprite materials are opaque"), SpriteBlended, 0);
                    TestTrue(TEXT("SECT4 sprite materials use alpha test"), SpriteMasked > 0);
                }
            }
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
                            int32((Primitive >> 6) & 1), Primitive & ~0x40u, VertexCount));
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
                    const bool bHasAlpha1 = KyaG2D::ReadMaterialRegister(Source, *Offset, 0, 0x42, Alpha1);
                    const bool bHasAlpha2 = KyaG2D::ReadMaterialRegister(Source, *Offset, 0, 0x43, Alpha2);
                    uint32 VertexCount = 0, Primitive = 0;
                    const bool bHasPrim = KyaG3D::ReadFirstStripGifTag(MeshData, Strip.StripOffset, VertexCount, Primitive);
                    const uint32 Abe = bHasPrim ? ((Primitive >> 6) & 1) : 2;
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
                        ++CommonBlends.FindOrAdd(FString::Printf(TEXT("ABE=%d prim=%x"), int32((Primitive >> 6) & 1), Primitive & ~0x40u));
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
                                        Primitive & ~0x40u, int32((Primitive >> 6) & 1), VertexCount));
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
        int32 ActorStreams = 0, ActorRefs = 0, ZoneRefs = 0, InvalidActorRefs = 0;
        TArray<FString> ActorStreamNames;
        for (const FKyaActorPlacement& Item : Bank->SceneEnvironment.Actors)
        {
            if (Item.ClassId != 0x0d) continue;
            ++Platforms;
            Prefixes += Item.bHasPlatformPrefix ? 1 : 0;
            Details += Item.bHasPlatformDetails ? 1 : 0;
            Cameras += Item.PlatformCameras.Num(); SoundA += Item.PlatformSoundsA.Num(); SoundB += Item.PlatformSoundsB.Num();
            Friction += Item.PlatformNoFrictionZones.Num();
            for (const FKyaPlatformNoFrictionZone& Zone : Item.PlatformNoFrictionZones) FrictionActors += Zone.Actors.Num();
            ActorRefs += Item.PlatformActors.Num();
            if (!Item.PlatformActors.IsEmpty())
            {
                ++ActorStreams;
                ActorStreamNames.Add(FString::Printf(TEXT("%s:%d"), *Item.Name, Item.PlatformActors.Num()));
            }
            ZoneRefs += Item.PlatformZones.Num();
            for (const FKyaPlatformActor& Ref : Item.PlatformActors)
                if (Ref.ActorRef < 0 || Ref.ActorRef >= Bank->SceneEnvironment.Actors.Num()) ++InvalidActorRefs;
        }
        AddInfo(FString::Printf(TEXT("Platform inventory: actors=%d prefixes=%d details=%d actor-streams=%d actor-refs=%d invalid-actor-refs=%d zone-refs=%d camera-records=%d sound-a=%d sound-b=%d friction-zones=%d friction-actors=%d"),
            Platforms, Prefixes, Details, ActorStreams, ActorRefs, InvalidActorRefs, ZoneRefs, Cameras, SoundA, SoundB, Friction, FrictionActors));
        UE_LOG(LogTemp, Display, TEXT("Kya platform inventory: actors=%d prefixes=%d details=%d actor-streams=%d actor-refs=%d invalid-actor-refs=%d zone-refs=%d camera-records=%d sound-a=%d sound-b=%d friction-zones=%d friction-actors=%d"),
            Platforms, Prefixes, Details, ActorStreams, ActorRefs, InvalidActorRefs, ZoneRefs, Cameras, SoundA, SoundB, Friction, FrictionActors);
        TestEqual(TEXT("Platform actor stream refs resolve to placements"), InvalidActorRefs, 0);
        TestTrue(TEXT("Level one exercises moving-platform actor streams"), ActorStreams > 0);
        AddInfo(FString::Printf(TEXT("Platform actor streams: %s"), *FString::Join(ActorStreamNames, TEXT(","))));
        UE_LOG(LogTemp, Display, TEXT("Kya platform actor streams: %s"), *FString::Join(ActorStreamNames, TEXT(",")));
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
        BoxActor.BasicBoxLifeMax = 25;
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
        Projectile.ProjectileLifeMax = 30;
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
        Micken.MickenLifeMax = 40;
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
        Fruit.FruitLifeMax = 50;
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
        Nativ.NativLifeMax = 60;
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
        Box.BoxLifeMax = 70;
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
        Shoot.ShootLifeMax = 80;
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
        Gravity.GravityLifeMax = 90;
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
        Companion.CompanionLifeMax = 100;
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
        Aton.AtonLifeMax = 110;
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
        Kim.FighterLifeMax = 120;
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
        TestEqual(TEXT("Edited basic box life"), Box.BasicBoxLifeMax, int64(25));
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
        TestEqual(TEXT("Edited projectile life"), Projectile.ProjectileLifeMax, int64(30));
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
        TestEqual(TEXT("Edited Micken life"), Micken.MickenLifeMax, int64(40));
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
        TestEqual(TEXT("Edited fruit life"), Fruit.FruitLifeMax, int64(50));
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
        TestEqual(TEXT("Edited Nativ life"), Nativ.NativLifeMax, int64(60));
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
        TestEqual(TEXT("Edited box life"), Box.BoxLifeMax, int64(70));
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
        TestEqual(TEXT("Edited shooter life"), Shoot.ShootLifeMax, int64(80));
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
        TestEqual(TEXT("Edited gravity life"), Gravity.GravityLifeMax, int64(90));
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
        TestEqual(TEXT("Edited companion life"), Companion.CompanionLifeMax, int64(100));
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
        TestEqual(TEXT("Edited Aton life"), Aton.AtonLifeMax, int64(110));
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
        TestEqual(TEXT("Edited Kim fighter life"), Kim.FighterLifeMax, int64(120));
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
    {
        int32 PatternField24Count = 0;
        for (const FKyaActorPlacement& Actor : Bank->SceneEnvironment.Actors)
            if (Actor.bHasProjectileData && Actor.ProjectilePattern.bHasField24) ++PatternField24Count;
        AddInfo(FString::Printf(TEXT("Projectile class payloads with pattern field24: %d"), PatternField24Count));
    }
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
        FKyaSwitchActorOffsets Switch;
        if (KyaReadSwitchActorOffsets(Input, ActorOffsets[SwitchActor].ClassData, ActorOffsets[SwitchActor].ClassDataSize, Switch))
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
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FKyaSelectedActorRoundTripTest, "Kya.Bank.SelectedActorRoundTrip",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FKyaSelectedActorRoundTripTest::RunTest(const FString& Parameters)
{
    const FString Filename = FPlatformMisc::GetEnvironmentVariable(TEXT("KYA_TEST_BANK"));
    if (Filename.IsEmpty())
    {
        AddWarning(TEXT("Set KYA_TEST_BANK to run the selected-actor round-trip test."));
        return true;
    }
    TArray<uint8> Input;
    if (!TestTrue(TEXT("Read selected-actor bank"), FFileHelper::LoadFileToArray(Input, *Filename))) return false;
    const uint8* Cursor = Input.GetData();
    UKyaLevelBankFactory* Factory = NewObject<UKyaLevelBankFactory>();
    UKyaLevelBank* Bank = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
        GetTransientPackage(), NAME_None, RF_Transient, nullptr, TEXT("bnk"), Cursor,
        Input.GetData() + Input.Num(), GWarn));
    if (!TestNotNull(TEXT("Import selected-actor bank"), Bank)) return false;
    AddInfo(TEXT("Real-bank coverage: edits go through FKyaLevelEditService on one selected actor."));
    if (!TestTrue(TEXT("Real bank has actor records"), Bank->SceneEnvironment.Actors.Num() > 0)) return false;

    UKyaLevelBankExporter* Exporter = NewObject<UKyaLevelBankExporter>();
    TArray<uint8> Plain;
    FMemoryWriter PlainWriter(Plain);
    if (!TestTrue(TEXT("Export unedited real bank"), Exporter->ExportBinary(Bank, TEXT("bnk"), PlainWriter, GWarn))) return false;
    TestEqual(TEXT("Unedited real export keeps the file size"), Plain.Num(), Input.Num());
    TestTrue(TEXT("Unedited real export preserves every byte"), Plain == Input);

    // Locate actor 0's serialized record span for the unchanged-byte comparison.
    auto ReadWord = [&Input](int64 Offset) -> uint32 {
        return uint32(Input[Offset]) | (uint32(Input[Offset + 1]) << 8) |
            (uint32(Input[Offset + 2]) << 16) | (uint32(Input[Offset + 3]) << 24);
    };
    const FKyaBankEntry* SceneData = Bank->Entries.FindByPredicate(
        [](const FKyaBankEntry& Entry) { return Entry.Type == 6 && Entry.Subtype == 1; });
    if (!TestNotNull(TEXT("Selected-actor scene data entry"), SceneData)) return false;
    int64 SetupOffset = SceneData->Offset + 20;
    if (ReadWord(SceneData->Offset + 16)) SetupOffset += 12;
    const int32 BackgroundId = int32(ReadWord(SetupOffset));
    SetupOffset += 4 + (BackgroundId != -1 ? 4 : 0);
    int64 CameraConfigOffset = 0;
    if (!TestTrue(TEXT("Locate selected-actor camera configuration"),
        KyaFindCameraConfig(Input, SetupOffset, CameraConfigOffset))) return false;
    int64 SceneValuesOffset = 0;
    if (!TestTrue(TEXT("Locate selected-actor scene values"),
        KyaFindSceneAfterCamera(Input, CameraConfigOffset, SceneValuesOffset))) return false;
    int64 MapEndOffset = 0;
    if (!TestTrue(TEXT("Locate selected-actor map end"),
        KyaFindMapEnd(Input, SceneValuesOffset + 12, MapEndOffset))) return false;
    TArray<FKyaConditionedOffsets> ScenarioOffsets;
    int64 ScenarioEndOffset = 0;
    if (!TestTrue(TEXT("Locate selected-actor scenario end"),
        KyaReadConditionedOffsets(Input, MapEndOffset + 48, ScenarioOffsets, ScenarioEndOffset))) return false;
    TArray<FKyaObjectiveOffsets> ObjectiveOffsets;
    int64 ObjectiveEndOffset = 0;
    if (!TestTrue(TEXT("Locate selected-actor objectives"),
        KyaReadObjectiveOffsets(Input, ScenarioEndOffset, ObjectiveOffsets, ObjectiveEndOffset))) return false;
    int64 WaypointOffset = 0;
    int32 WaypointCount = 0;
    if (!TestTrue(TEXT("Locate selected-actor waypoints"),
        KyaFindWaypoints(Input, ObjectiveEndOffset, WaypointOffset, WaypointCount))) return false;
    TArray<FKyaPathOffsets> PathOffsets;
    if (!TestTrue(TEXT("Locate selected-actor paths"),
        KyaReadPathOffsets(Input, WaypointOffset, WaypointCount, SceneData->Offset, PathOffsets))) return false;
    TArray<FKyaActorOffsets> ActorOffsets;
    if (!TestTrue(TEXT("Locate selected-actor records"),
        KyaReadActorOffsets(Input, WaypointOffset, SceneData->Offset, ActorOffsets)) || ActorOffsets.IsEmpty()) return false;
    int64 RecordLow = ActorOffsets[0].Chunk;
    for (const int64 Candidate : {ActorOffsets[0].Name, ActorOffsets[0].Placement,
        ActorOffsets[0].MacroAnimations, ActorOffsets[0].Common, ActorOffsets[0].Conditions})
        if (Candidate > 0) RecordLow = FMath::Min(RecordLow, Candidate);
    const int64 RecordHigh = ActorOffsets[0].ClassData + ActorOffsets[0].ClassDataSize;

    const FKyaActorPlacement& Original = Bank->SceneEnvironment.Actors[0];
    const FVector OriginalPosition = Original.Position;
    const float OriginalVisibility = Original.VisibilityDistance;
    const float OriginalCulling = Original.CullingDistance;
    const FVector NeighbourPosition = Bank->SceneEnvironment.Actors.Num() > 1
        ? Bank->SceneEnvironment.Actors[1].Position : FVector::ZeroVector;

    FKyaLevelEditService& Service = FKyaLevelEditService::Get();
    Service.Reset();
    FKyaActorEditPatch Patch;
    Patch.Position = OriginalPosition + FVector(1.25, -2.5, 3.75);
    Patch.VisibilityDistance = 4321.0f;
    Patch.CullingDistance = 8765.0f;
    FString Error;
    TestTrue(TEXT("Selected-actor patch applies through the edit service"),
        Service.Apply(Service.MakeKey(Bank, 0), Patch, EKyaEditNotify::Silent, &Error));

    TArray<uint8> Edited;
    FMemoryWriter EditedWriter(Edited);
    if (!TestTrue(TEXT("Export selected-actor edit"), Exporter->ExportBinary(Bank, TEXT("bnk"), EditedWriter, GWarn))) return false;
    TestEqual(TEXT("Selected-actor export keeps the file size"), Edited.Num(), Input.Num());

    int32 ChangedBytes = 0;
    int64 FirstOutside = INDEX_NONE;
    for (int32 Index = 0; Index < Input.Num(); ++Index)
    {
        if (Input[Index] == Edited[Index]) continue;
        ++ChangedBytes;
        if ((Index < RecordLow || Index >= RecordHigh) && FirstOutside == INDEX_NONE)
            FirstOutside = Index;
    }
    TestTrue(TEXT("Selected-actor edit changes serialized bytes"), ChangedBytes > 0);
    TestTrue(TEXT("Selected-actor edit touches only its record"),
        ChangedBytes <= 40 && FirstOutside == INDEX_NONE);
    if (FirstOutside != INDEX_NONE)
        AddError(FString::Printf(TEXT("Changed byte outside actor 0 record at offset %lld (record %lld..%lld)."),
            FirstOutside, RecordLow, RecordHigh));

    const uint8* EditedCursor = Edited.GetData();
    UKyaLevelBank* Reimported = Cast<UKyaLevelBank>(Factory->FactoryCreateBinary(UKyaLevelBank::StaticClass(),
        GetTransientPackage(), NAME_None, RF_Transient, nullptr, TEXT("bnk"), EditedCursor,
        Edited.GetData() + Edited.Num(), GWarn));
    if (!TestNotNull(TEXT("Reimport selected-actor bank"), Reimported)) return false;
    const FKyaActorPlacement& EditedActor = Reimported->SceneEnvironment.Actors[0];
    TestTrue(TEXT("Selected-actor position round trips"),
        EditedActor.Position.Equals(OriginalPosition + FVector(1.25, -2.5, 3.75), 1.0e-4));
    TestTrue(TEXT("Selected-actor visibility round trips"),
        FMath::IsNearlyEqual(EditedActor.VisibilityDistance, 4321.0f, 1.0e-3f));
    TestTrue(TEXT("Selected-actor culling round trips"),
        FMath::IsNearlyEqual(EditedActor.CullingDistance, 8765.0f, 1.0e-3f));
    TestEqual(TEXT("Selected-actor name is preserved"), EditedActor.Name, Original.Name);
    if (Reimported->SceneEnvironment.Actors.Num() > 1)
        TestTrue(TEXT("Neighbour actor is untouched"),
            Reimported->SceneEnvironment.Actors[1].Position.Equals(NeighbourPosition));
    Service.Reset();
    return true;
}

#endif
