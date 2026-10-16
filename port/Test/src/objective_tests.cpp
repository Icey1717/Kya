#include <gtest/gtest.h>

#include <cstring>
#include <memory>
#include "ActorHero_Private.h"
#include "CollisionManager.h"
#include "EventManager.h"
#include "LevelScheduler.h"
#include "MathOps.h"
#include "TimeController.h"
#include "edMem.h"
#include "EdenLib/edSys/sources/EdSystem.h"

extern ed_event_chunk** pedEventChunks;

namespace
{
	class ObjectiveTestHero : public CActorHeroPrivate
	{
	public:
		int ReceiveMessage(CActor* sender, ACTOR_MESSAGE message, MSG_PARAM param) override
		{
			if (message == MESSAGE_KICKED) {
				++hitCount;
				hit = *static_cast<_msg_hit_param*>(param);
				return 1;
			}
			return 0;
		}

		int hitCount = 0;
		_msg_hit_param hit{};
	};

	class ObjectiveTest : public testing::Test
	{
	public:
		static void SetUpTestSuite()
		{
			if (!edSystem.system.startAddress) {
				ASSERT_TRUE(_edSystemInit(0, nullptr));
				edMemInit(0x1000);
			}
		}

	protected:
		void SetUp() override
		{
			hero = std::make_unique<ObjectiveTestHero>();
			CLevelScheduler::gThis = &scheduler;
			CActorHero::_gThis = hero.get();
			CScene::ptable.g_EventManager_006f5080 = &eventManager;
			CScene::ptable.g_SectorManager_00451670 = &sectorManager;
			chunks[0] = &chunk;
			pedEventChunks = chunks;
			chunk.field_0x8 = &primitiveType;
			chunk.aMatrices = &matrix;
			zone.field_0x0[0] = STORE_POINTER(primitiveList);
			eventManager.activeChunkId = 0;
			sectorManager.baseSector.desiredSectorID = 4;
			hero->currentLocation = { 0.0f, 0.0f, 0.0f, 1.0f };
			hero->pCollisionData = nullptr;
			hero->flags = 0;
			scheduler.currentLevelID = 1;
			scheduler.currentElevatorID = -1;
			scheduler.autoSaveTriggerTime = 0.0f;
			scheduler.gameTime = 0.0f;
			scheduler.pObjectiveStreamBegin = nullptr;
			scheduler.pObjectiveStreamEnd = nullptr;
			scheduler.objCount_0x4218 = 1;
			scheduler.field_0x4220[0] = {};
			scheduler.field_0x4220[0].field_0x0 = 0x20;
			CLevelScheduler::ScenVar_Set(0x20, 1);
			CLevelScheduler::ScenVar_Set(0x21, 1);
		}

		void TearDown() override
		{
			RELEASE_POINTER(zone.field_0x0[0]);
			hero.reset();
			pedEventChunks = previousChunks;
			CScene::ptable = previousTable;
			CLevelScheduler::gThis = previousScheduler;
			CActorHero::_gThis = previousHero;
			CLevelScheduler::ScenVar_Set(0x20, previousScenVar);
			CLevelScheduler::ScenVar_Set(0x21, previousOtherScenVar);
		}

		ObjectiveStreamEntry* InstallObjective(int nbTargets)
		{
			std::memset(streamStorage, 0, sizeof(streamStorage));
			auto* stream = reinterpret_cast<ObjectiveStream*>(streamStorage);
			stream->nbEntries = 1;
			auto* entry = stream->aEntries;
			entry->scenVarId = 0x20;
			entry->nbTargets = nbTargets;
			scheduler.pObjectiveStreamBegin = stream;
			return entry;
		}

		decltype(CScene::ptable) previousTable = CScene::ptable;
		CLevelScheduler* previousScheduler = CLevelScheduler::gThis;
		CActorHero* previousHero = CActorHero::_gThis;
		ed_event_chunk** previousChunks = pedEventChunks;
		int previousScenVar = CLevelScheduler::ScenVar_Get(0x20);
		int previousOtherScenVar = CLevelScheduler::ScenVar_Get(0x21);
		CLevelScheduler scheduler;
		CEventManager eventManager;
		CSectorManager sectorManager;
		std::unique_ptr<ObjectiveTestHero> hero;
		ed_event_chunk* chunks[1]{};
		ed_event_chunk chunk{};
		ed_zone_3d zone{};
		edF32MATRIX4 matrix = gF32Matrix4Unit;
		byte primitiveType = 2;
		uint primitiveList[2] = { 1, 0 };
		alignas(4) unsigned char streamStorage[4 + 2 * (0x20 + 2 * 0x10)]{};
	};
}

TEST_F(ObjectiveTest, EnteringTargetAdvancesOnlyOneStepAndLoadsNextMessage)
{
	auto* entry = InstallObjective(2);
	entry->aTargets[0].zoneRef.pObj = STORE_POINTER(&zone);
	entry->aTargets[1].messageKeyA = 0x12345678;
	entry->aTargets[1].messageKeyB = 0x90abcdef;
	scheduler.Level_Manage();
	EXPECT_EQ(scheduler.field_0x4220[0].field_0x2c, 1);
	EXPECT_EQ(scheduler.field_0x4220[0].field_0x40, 0x90abcdef12345678ULL);
	EXPECT_FLOAT_EQ(scheduler.field_0x4220[0].field_0x30.x, 0.0f);
	RELEASE_POINTER(entry->aTargets[0].zoneRef.pObj);
}

TEST_F(ObjectiveTest, CompletingLastTargetClearsMarkerAndMessage)
{
	auto* entry = InstallObjective(1);
	entry->aTargets[0].zoneRef.pObj = STORE_POINTER(&zone);
	scheduler.field_0x4220[0].field_0x40 = 123;
	scheduler.field_0x4220[0].field_0x30 = { 1, 2, 3, 1 };
	scheduler.Level_Manage();
	EXPECT_EQ(scheduler.field_0x4220[0].field_0x2c, 1);
	EXPECT_EQ(scheduler.field_0x4220[0].field_0x40, 0);
	EXPECT_FLOAT_EQ(scheduler.field_0x4220[0].field_0x30.z, 0.0f);
	RELEASE_POINTER(entry->aTargets[0].zoneRef.pObj);
}

TEST_F(ObjectiveTest, ExplicitTargetActorOverridesHeroPosition)
{
	auto* entry = InstallObjective(1);
	CActor targetActor;
	targetActor.currentLocation = { 2, 0, 0, 1 };
	entry->aTargets[0].zoneRef.pObj = STORE_POINTER(&zone);
	entry->aTargets[0].actorRef.pObj = STORE_POINTER(&targetActor);
	scheduler.Level_Manage();
	EXPECT_EQ(scheduler.field_0x4220[0].field_0x2c, 0);
	CLevelScheduler::ScenVar_Set(0x20, 0);
	targetActor.currentLocation.x = 0.0f;
	scheduler.Level_Manage();
	EXPECT_EQ(scheduler.field_0x4220[0].field_0x2c, 0);
	RELEASE_POINTER(entry->aTargets[0].zoneRef.pObj);
	RELEASE_POINTER(entry->aTargets[0].actorRef.pObj);
}

TEST_F(ObjectiveTest, VariableLengthRecordsReachTheFollowingObjective)
{
	auto* first = InstallObjective(2);
	auto* second = reinterpret_cast<ObjectiveStreamEntry*>(first->aTargets + first->nbTargets);
	scheduler.pObjectiveStreamBegin->nbEntries = 2;
	second->scenVarId = 0x21;
	second->nbTargets = 1;
	second->aTargets[0].zoneRef.pObj = STORE_POINTER(&zone);
	scheduler.objCount_0x4218 = 2;
	scheduler.field_0x4220[1] = {};
	scheduler.field_0x4220[1].field_0x0 = 0x21;
	scheduler.Level_Manage();
	EXPECT_EQ(scheduler.field_0x4220[0].field_0x2c, 0);
	EXPECT_EQ(scheduler.field_0x4220[1].field_0x2c, 1);
	RELEASE_POINTER(second->aTargets[0].zoneRef.pObj);
}

TEST_F(ObjectiveTest, VisitedZoneResolvesSerializedPointerAndSetsHighBit)
{
	int visitedZones[] = { 1, 31, STORE_POINTER(&zone) };
	scheduler.pObjectiveStreamEnd = visitedZones;
	scheduler.field_0x5b30 = 2;
	scheduler.Level_Manage();
	EXPECT_EQ(scheduler.field_0x5b30, 0x80000002U);
	RELEASE_POINTER(visitedZones[2]);
}

TEST_F(ObjectiveTest, NoTutorialZonesClearsActivePrompt)
{
	hero->field_0xd30 = 2;
	hero->field_0xd2c = 0.75f;
	hero->ActorFunc_00347660();
	EXPECT_EQ(hero->field_0xd30, 0);
	EXPECT_EQ(hero->field_0xd28, nullptr);
	EXPECT_FLOAT_EQ(hero->field_0xd2c, 0.0f);
}

TEST_F(ObjectiveTest, ProjectileZonePreservesDamageAndForceBitsAndFiltersSector)
{
	hero->checkpointCount = 1;
	hero->field_0xe38 = new HeroProjectileZone[1]{};
	auto& projectile = hero->field_0xe38[0];
	projectile.zoneRef.pObj = STORE_POINTER(&zone);
	projectile.projectileType = 1;
	projectile.damage = 7.25f;
	projectile.force = 123.5f;
	projectile.waypointIndex = -1;
	projectile.nbSamples = 3;
	projectile.sectorId = 5;
	hero->ActorFunc_00347290();
	EXPECT_EQ(hero->hitCount, 0);
	projectile.sectorId = 4;
	hero->ActorFunc_00347290();
	EXPECT_EQ(hero->hitCount, 1);
	EXPECT_FLOAT_EQ(hero->hit.damage, 7.25f);
	EXPECT_FLOAT_EQ(hero->hit.field_0x30, 123.5f);
	RELEASE_POINTER(projectile.zoneRef.pObj);
}

TEST_F(ObjectiveTest, RestartMessageMarksTheCurrentTutorialRecord)
{
	hero->field_0xd20 = 2;
	hero->pEventChunk24_0xd24 = new HeroTutorialZone[2]{};
	hero->field_0xd28 = hero->pEventChunk24_0xd24 + 1;
	hero->field_0xd30 = 2;
	hero->field_0xd2c = 0.6f;
	hero->field_0x2e4 = 0.0f;
	hero->field_0x1558 = 0.0f;
	hero->actorState = AS_None;
	std::memset(hero->field_0xd34, 0, sizeof(hero->field_0xd34));
	hero->GetLifeInterface()->SetValueMax(100.0f);
	hero->GetLifeInterface()->SetValue(100.0f);
	EXPECT_EQ(hero->InterpretMessage(nullptr, 0x7e, nullptr), 1);
	EXPECT_EQ(hero->field_0xd30, 1);
	EXPECT_EQ(hero->field_0xd34[1][0], 0);
	EXPECT_EQ(hero->field_0xd34[1][1], 3);
	EXPECT_FLOAT_EQ(hero->field_0xd2c, 0.6f);
}

TEST_F(ObjectiveTest, ProjectileZoneTestsBodySamplesBelowTheActorOrigin)
{
	hero->checkpointCount = 1;
	hero->field_0xe38 = new HeroProjectileZone[1]{};
	auto& projectile = hero->field_0xe38[0];
	projectile.zoneRef.pObj = STORE_POINTER(&zone);
	projectile.projectileType = 1;
	projectile.waypointIndex = -1;
	projectile.nbSamples = 3;
	projectile.sectorId = 4;
	CCollision collision{};
	collision.lowestVertex = { 0, 0, 0, 1 };
	hero->pCollisionData = &collision;
	hero->currentLocation.y = 2.0f;
	hero->ActorFunc_00347290();
	EXPECT_EQ(hero->hitCount, 1);
	hero->pCollisionData = nullptr;
	RELEASE_POINTER(projectile.zoneRef.pObj);
}
