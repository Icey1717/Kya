#include <gtest/gtest.h>

#include "ActorShoot.h"
#include "AnmManager.h"
#include "MathOps.h"
#include "TimeController.h"

namespace
{
	class ShootActor : public CActorShoot
	{
	public:
		void SetState(int newState, int animationType) override
		{
			this->actorState = static_cast<EActorState>(newState);
			lastAnimationType = animationType;
			return;
		}

		void ManageDyn(float param_1, uint flags, CActorsTable* pActorsTable) override
		{
			dynamicFlags = flags;
			return;
		}

		int lastAnimationType = -2;
		uint dynamicFlags = 0;
	};

	class ShootTest : public testing::Test
	{
	protected:
		void SetUp() override
		{
			previousDelta = GetTimer()->cutsceneDeltaTime;
			GetTimer()->cutsceneDeltaTime = 0.125f;
			actor.addOnGenerator = {};
			actor.field_0x438 = nullptr;
			actor.field_0x3c8 = 0.25f;
			actor.field_0x3cc = 0.5f;
			actor.field_0x3d0 = 3;
			actor.field_0x3d4 = 1.0f;
			actor.field_0x3f4 = 0.0f;
			actor.field_0x3f8 = 0.0f;
			actor.field_0x3fc = 0.0f;
			actor.field_0x400 = 0;
			actor.field_0x3ec = 0;
			actor.currentAnimType = 0x11;
			actor.actorState = static_cast<EActorState>(0xc);
			actor.pAnimationController = &animation;
			animation.anmBinMetaAnimator.aAnimData = &layer;
			animation.currentAnimType = 0x11;
			actor.behaviourShootFire.fireshot.pActorStreamRef = nullptr;
			actor.behaviourShootFire.fireshot.field_0x298 = 0;
			actor.behaviourShootFireWave.conicalWaveShoot.Reset();
			return;
		}

		void TearDown() override
		{
			actor.staticMeshComponent.pMeshTransformParent = nullptr;
			actor.staticMeshComponent.pMeshTransformData = nullptr;
			GetTimer()->cutsceneDeltaTime = previousDelta;
			return;
		}

		void Manage(bool wave)
		{
			if (wave) {
				actor.BehaviourFireWave_Manage(&actor.behaviourShootFireWave);
			}
			else {
				actor.BehaviourShootFire_Manage(&actor.behaviourShootFire);
			}
			return;
		}

		float previousDelta;
		ShootActor actor;
		CAnimation animation;
		edAnmLayer layer{};
		ed_3d_hierarchy_node mesh{};
	};
}

TEST_F(ShootTest, ShotDelayStartsAfterBothCooldownsAndUsesAStrictBoundary)
{
	actor.field_0x3f8 = 0.375f;
	actor.field_0x3fc = 0.75f;
	actor.ComputeTimeAndParamToShoot();
	EXPECT_FALSE(actor.field_0x43e);
	EXPECT_FALSE(actor.field_0x43d);
	EXPECT_FLOAT_EQ(actor.field_0x3f4, 0.0f);
	actor.ComputeTimeAndParamToShoot();
	EXPECT_TRUE(actor.field_0x43e);
	EXPECT_FALSE(actor.field_0x43d);
	EXPECT_FLOAT_EQ(actor.field_0x3f4, 0.125f);
	actor.ComputeTimeAndParamToShoot();
	EXPECT_FALSE(actor.field_0x43d);
	actor.ComputeTimeAndParamToShoot();
	EXPECT_TRUE(actor.field_0x43d);
	actor.currentAnimType = 0x10;
	actor.ComputeTimeAndParamToShoot();
	EXPECT_FLOAT_EQ(actor.field_0x3f4, 0.375f);
}

TEST_F(ShootTest, LastShotResetsTheBurstCooldownAfterPublishingReadiness)
{
	actor.field_0x3f8 = 0.5f;
	actor.field_0x3fc = 1.0f;
	actor.field_0x3f4 = 0.25f;
	actor.field_0x400 = 3;
	actor.ComputeTimeAndParamToShoot();
	EXPECT_TRUE(actor.field_0x43e);
	EXPECT_TRUE(actor.field_0x43d);
	EXPECT_EQ(actor.field_0x400, 0);
	EXPECT_FLOAT_EQ(actor.field_0x3fc, 0.0f);
	actor.ComputeTimeAndParamToShoot();
	EXPECT_FALSE(actor.field_0x43e);
	EXPECT_FALSE(actor.field_0x43d);
}

TEST_F(ShootTest, BothBehavioursWaitPastTheEmergenceBoundary)
{
	for (bool wave : { false, true }) {
		actor.actorState = static_cast<EActorState>(7);
		actor.currentAnimType = 0xc;
		actor.timeInAir = 0.4f;
		actor.staticMeshComponent.pMeshTransformParent = reinterpret_cast<edNODE*>(&mesh);
		actor.staticMeshComponent.pMeshTransformData = &mesh;
		actor.currentLocation = edF32VECTOR4{ 1.0f, 2.0f, 3.0f, 1.0f };
		Manage(wave);
		EXPECT_EQ(actor.actorState, 7);
		EXPECT_EQ(actor.dynamicFlags, 0x1002023b);
		EXPECT_FLOAT_EQ(mesh.base.transformA.rowT.y, 2.08f);
		actor.timeInAir = 0.401f;
		Manage(wave);
		EXPECT_EQ(actor.actorState, 8);
		EXPECT_EQ(actor.lastAnimationType, -1);
	}
}

TEST_F(ShootTest, HitAnimationRestoresTheCurrentStateOnlyWhenItsOwnLayerEnds)
{
	for (bool wave : { false, true }) {
		actor.actorState = static_cast<EActorState>(7);
		actor.currentAnimType = 0x13;
		animation.currentAnimType = 0x13;
		layer.currentAnimDesc.animType = 0x10;
		layer.animPlayState = 1;
		layer.field_0xcc = 2;
		actor.timeInAir = 0.0f;
		actor.lastAnimationType = -2;
		Manage(wave);
		EXPECT_EQ(actor.lastAnimationType, -2);
		layer.currentAnimDesc.animType = 0x13;
		Manage(wave);
		EXPECT_EQ(actor.actorState, 7);
		EXPECT_EQ(actor.lastAnimationType, -1);
	}
}
