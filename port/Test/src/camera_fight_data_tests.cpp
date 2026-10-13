#include <gtest/gtest.h>

#include <memory>
#include "ActorHero.h"
#include "ActorManager.h"
#include "CameraFightData.h"
#include "CameraGame.h"
#include "CollisionManager.h"
#include "MemoryStream.h"
#include "MathOps.h"
#include "TimeController.h"

extern CamUpdate CamUpdate_0049c760;

namespace
{
	class FightCameraTest : public testing::Test
	{
	protected:
		void SetUp() override
		{
			previousHero = CActorHero::_gThis;
			previousActors = CScene::ptable.g_ActorManager_004516a4;
			previousCollision = CScene::ptable.g_CollisionManager_00451690;
			previousDelta = GetTimer()->cutsceneDeltaTime;
			previousFrameTime = GetTimer()->lastFrameTime;
			previousConfig = CCameraGame::_pfig_data;
			previousEnabled = CCameraGame::_b_use_fig_data;
			hero = std::make_unique<CActorHero>();
			hero->pMainCamera = &mainCamera;
			CActorHero::_gThis = hero.get();
			CScene::ptable.g_ActorManager_004516a4 = &actors;
			CScene::ptable.g_CollisionManager_00451690 = &collision;
			GetTimer()->cutsceneDeltaTime = 0.0f;
			GetTimer()->lastFrameTime = 0.0f;

			// Minimal camera bytecode: looking along Z with Y up, no optional records.
			cameraBytes[6] = 1.0f;
			cameraBytes[8] = 1.0f;
			ByteCode cameraStream;
			cameraStream.currentSeekPos = reinterpret_cast<char*>(cameraBytes);
			camera = std::make_unique<CCameraGame>(CT_Fight, &cameraStream);
			target.typeID = ACTOR;
			camera->SetTarget(&target);
			camera->transformationMatrix = gF32Matrix4Unit;
			camera->transformationMatrix.rowT.z = -5.0f;
			camera->SetDistance(5.0f);

			ByteCode fightStream;
			fightStream.currentSeekPos = reinterpret_cast<char*>(fightParams);
			data.Create(&fightStream);
			data.pCommanderRef_0x230 = &target;
			data.Cam_SetDefault(1.0f, 2.0f, camera.get());
			data.field_0x2c4 = data.field_0x4c;
			data.field_0x2e0 = target.currentLocation;
			data.SetAdversaryPos(&adversaryPos);
		}

		void TearDown() override
		{
			hero.reset();
			CActorHero::_gThis = previousHero;
			CScene::ptable.g_ActorManager_004516a4 = previousActors;
			CScene::ptable.g_CollisionManager_00451690 = previousCollision;
			CCameraGame::_pfig_data = previousConfig;
			CCameraGame::_b_use_fig_data = previousEnabled;
			GetTimer()->cutsceneDeltaTime = previousDelta;
			GetTimer()->lastFrameTime = previousFrameTime;
			CamUpdate_0049c760 = previousSamples;
			delete[] actors.aLinkedActorData;
		}

		CamUpdate previousSamples = CamUpdate_0049c760;
		CActorHero* previousHero = nullptr;
		CActorManager* previousActors = nullptr;
		CCollisionManager* previousCollision = nullptr;
		CCamFigData* previousConfig = nullptr;
		int previousEnabled = 0;
		float previousDelta = 0.0f;
		float previousFrameTime = 0.0f;
		std::unique_ptr<CActorHero> hero;
		CCamera mainCamera;
		CActor target;
		CActorManager actors;
		CCollisionManager collision;
		std::unique_ptr<CCameraGame> camera;
		CCamFigData data;
		float cameraBytes[128] = {};
		float fightParams[14] = { 30.0f, 100.0f, 30.0f, 10.0f, 1.0f, 10.0f,
			0.0f, 2.0f, 0.0f, 0.0f, 10.0f, 2.0f, 0.0f, 0.0f };
		edF32VECTOR4 adversaryPos = { 0.0f, 0.0f, 2.0f, 1.0f };
	};
}

TEST_F(FightCameraTest, ResetRestoresFightStateAndCollisionCache)
{
	data.field_0x2a0 = 1;
	data.field_0x2a4 = data.field_0x2a8 = data.field_0x2ac = 1;
	data.field_0x2b0 = data.field_0x2b4 = data.field_0x2b8 = data.field_0x2bc = 3;
	data.field_0x2c0 = 5.0f;
	data.field_0x2d0 = 1;
	data.field_0x304 = 2;
	data.field_0x280 = data.field_0x284 = data.field_0x288 = data.field_0x28c = 0.5f;
	CamUpdate_0049c760.activeIndex = 34;
	CamUpdate_0049c760.field_0x20 = 4;
	CamUpdate_0049c760.aSubObjA[0] = { 1, 1, 5, 10 };
	for (auto& sample : CamUpdate_0049c760.aSubObjB) {
		sample = { 0.5f, 2.0f };
	}

	data.Reset();

	EXPECT_EQ(data.field_0x2a0, 0u);
	EXPECT_EQ(data.field_0x2a4, 0);
	EXPECT_EQ(data.field_0x2a8, 0u);
	EXPECT_EQ(data.field_0x2ac, 0);
	EXPECT_EQ(data.field_0x2b0, 0);
	EXPECT_EQ(data.field_0x2b4, 0);
	EXPECT_EQ(data.field_0x2b8, 1);
	EXPECT_EQ(data.field_0x2bc, 1);
	EXPECT_EQ(data.field_0x2c0, 0.0f);
	EXPECT_EQ(data.field_0x2d0, 0);
	EXPECT_EQ(data.field_0x304, 6);
	EXPECT_EQ(data.field_0x280, 1e+30f);
	EXPECT_EQ(data.field_0x284, 1e+30f);
	EXPECT_EQ(data.field_0x288, 1e+30f);
	EXPECT_EQ(data.field_0x28c, 1e+30f);
	EXPECT_EQ(data.pCamera, camera.get());
	EXPECT_EQ(data.pCommanderRef_0x230, &target);
	EXPECT_EQ(CamUpdate_0049c760.activeIndex, 0);
	EXPECT_EQ(CamUpdate_0049c760.aSubObjA[0].field_0x0, 0);
	EXPECT_EQ(CamUpdate_0049c760.aSubObjA[0].field_0x2, 0);
	EXPECT_EQ(CamUpdate_0049c760.aSubObjA[0].field_0x4, 0);
	EXPECT_EQ(CamUpdate_0049c760.aSubObjA[0].field_0x6, 35);
	// PS2 Reset leaves the run count and sample ages unchanged.
	EXPECT_EQ(CamUpdate_0049c760.field_0x20, 4u);
	for (const auto& sample : CamUpdate_0049c760.aSubObjB) {
		EXPECT_EQ(sample.intersectionDistance, 1e+30f);
		EXPECT_EQ(sample.field_0x4, 2.0f);
	}
}

TEST_F(FightCameraTest, CameraAlertsPreserveValidityAndRestoreMainCameraFlag)
{
	CCameraGame::_pfig_data = &data;
	for (uint originalFlags : { 0x120u, 0x121u }) {
		data.IsValid(0);
		mainCamera.flags_0xc = originalFlags;
		data.UpdateMainCameraFlags(2);
		EXPECT_EQ(data.field_0x2a0, 0u);
		EXPECT_EQ(mainCamera.flags_0xc, originalFlags | 1u);

		data.IsValid(1);
		// Exercise the actual departure call site, not just the restored handler.
		camera->AlertCamera(1, nullptr, nullptr);
		EXPECT_EQ(data.field_0x2a0, 1u);
		EXPECT_EQ(mainCamera.flags_0xc, originalFlags);
		EXPECT_EQ(CCameraGame::_b_use_fig_data, 0);
	}
}

TEST_F(FightCameraTest, CameraFlagHandlerHonorsValidityAndMissingHero)
{
	data.IsValid(1);
	data.field_0x2a8 = 0;
	mainCamera.flags_0xc = 0x120;
	data.UpdateMainCameraFlags(2);
	EXPECT_EQ(mainCamera.flags_0xc, 0x120u);
	data.IsValid(0);
	mainCamera.flags_0xc = 0x121;
	data.UpdateMainCameraFlags(1);
	EXPECT_EQ(mainCamera.flags_0xc, 0x121u);
	hero->pMainCamera = nullptr;
	data.UpdateMainCameraFlags(2);
	CActorHero::_gThis = nullptr;
	data.UpdateMainCameraFlags(1);
	EXPECT_EQ(data.field_0x2a0, 0u);
}

TEST_F(FightCameraTest, UpdateIncludesNextBlockedRunAtEndOfInterval)
{
	// The +/-18-degree interval crosses from the clear run into the last blocked run.
	data.angleBetaDyn.Init(7.5f * 6.283185f / 35.0f, 0.0f);
	data.field_0x28c = 0.5f; // Keep an existing obstruction while the other ray refreshes.
	CamUpdate_0049c760.field_0x140 = 0;
	for (int i = 0; i < 35; ++i) {
		CamUpdate_0049c760.aSubObjB[i] = { i < 9 ? 10.0f : 0.0f, 0.0f };
		CamUpdate_0049c760.aSubObjA[i] = {};
	}

	data.Cam_Update(&target.currentLocation, 1);

	ASSERT_EQ(CamUpdate_0049c760.field_0x20, 2u);
	EXPECT_EQ(data.field_0x2b4, 2); // Blocked arc avoidance, rather than clear-arc state 3.
}

TEST(CameraFightData, UniformCollisionSamplesFormOneRun)
{
	for (float distance : {0.0f, 2.0f}) {
		CamUpdate update = CamUpdate_0049c760;
		update.field_0x140 = 0; // Keep the supplied samples instead of casting rays.
		update.field_0x18 = 1.0f;
		for (auto& sample : update.aSubObjB) {
			sample.intersectionDistance = distance;
			sample.field_0x4 = 0.0f;
		}

		edF32VECTOR4 position = {};
		update.FUN_003c4a40(&position);

		ASSERT_EQ(update.field_0x20, 1u);
		EXPECT_EQ(update.aSubObjA[0].field_0x0, distance < 1.0f);
		EXPECT_EQ(update.aSubObjA[0].field_0x2, 0);
		EXPECT_EQ(update.aSubObjA[0].field_0x4, 0);
		EXPECT_EQ(update.aSubObjA[0].field_0x6, 35);
	}
}

TEST(CameraFightData, CollisionChangeAtLastSampleFormsFinalRun)
{
	CamUpdate update = CamUpdate_0049c760;
	update.field_0x140 = 0;
	update.field_0x18 = 1.0f;
	for (auto& sample : update.aSubObjB) {
		sample.intersectionDistance = 2.0f;
		sample.field_0x4 = 0.0f;
	}
	update.aSubObjB[34].intersectionDistance = 0.0f;

	edF32VECTOR4 position = {};
	update.FUN_003c4a40(&position);

	ASSERT_EQ(update.field_0x20, 2u);
	EXPECT_EQ(update.aSubObjA[0].field_0x0, 0);
	EXPECT_EQ(update.aSubObjA[0].field_0x4, 0);
	EXPECT_EQ(update.aSubObjA[0].field_0x6, 34);
	EXPECT_EQ(update.aSubObjA[1].field_0x0, 1);
	EXPECT_EQ(update.aSubObjA[1].field_0x4, 34);
	EXPECT_EQ(update.aSubObjA[1].field_0x6, 35);
}
