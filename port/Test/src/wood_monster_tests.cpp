#include <gtest/gtest.h>

#include "ActorWoodMonster.h"
#include "ActorAutonomous.h"
#include "WayPoint.h"

namespace
{
	class WoodMonsterActor : public CActorWoodMonster
	{
	public:
		void SetState(int newState, int animationType) override
		{
			actorState = static_cast<EActorState>(newState);
		}
	};

	class WoodMonsterTarget : public CActorAutonomous
	{
	public:
		CLifeInterface* GetLifeInterface() override { return &lifeInterface; }
		void LifeDecrease(float amount) override { damage = amount; }
		StateConfig* GetStateCfg(int state) override { return &stateConfig; }

		int ReceiveMessage(CActor* pSender, ACTOR_MESSAGE msg, MSG_PARAM pMsgParam) override
		{
			lastMessage = msg;
			if (msg == 0x33) {
				struct ReleaseParams
				{
					edF32VECTOR4* pPosition;
					edF32VECTOR3* pDestination;
					edF32VECTOR3* pRotation;
					float duration;
				};
				ReleaseParams* pParams = static_cast<ReleaseParams*>(pMsgParam);
				position = *pParams->pPosition;
				destination = *pParams->pDestination;
				rotation = *pParams->pRotation;
				duration = pParams->duration;
			}
			return 1;
		}

		StateConfig stateConfig = StateConfig(0, 0);
		float damage = 0.0f;
		int lastMessage = -1;
		edF32VECTOR4 position{};
		edF32VECTOR3 destination{};
		edF32VECTOR3 rotation{};
		float duration = 0.0f;
	};

	class WoodMonsterTest : public testing::Test
	{
	protected:
		void SetUp() override
		{
			actor.field_0x170 = &target;
			actor.behaviourGet.pOwner = &actor;
			actor.behaviourGet.field_0x10 = -1;
			actor.pAnimationController = &animation;
			animation.anmBinMetaAnimator.aAnimData = &layer;
			animation.currentAnimType = 6;
			layer.currentAnimDesc.animType = 6;
			layer.animPlayState = 1;
			target.flags = 0;
			target.actorState = static_cast<EActorState>(5);
		}

		WoodMonsterActor actor;
		WoodMonsterTarget target;
		CAnimation animation;
		edAnmLayer layer{};
	};
}

TEST_F(WoodMonsterTest, DamageWaitsForTheCurrentAnimationAndUsesTheConfiguredAmount)
{
	actor.actorState = static_cast<EActorState>(WOODMONSTER_STATE_GET);
	actor.behaviourGet.field_0x8 = 3.0f;
	target.lifeInterface.currentValue = 4.0f;
	layer.field_0xcc = 2;
	layer.currentAnimDesc.animType = 7;
	actor.behaviourGet.Manage();
	EXPECT_EQ(actor.actorState, WOODMONSTER_STATE_GET);
	EXPECT_FLOAT_EQ(target.damage, 0.0f);
	layer.currentAnimDesc.animType = 6;
	actor.behaviourGet.Manage();
	EXPECT_FLOAT_EQ(target.damage, 3.0f);
	EXPECT_EQ(actor.actorState, WOODMONSTER_STATE_WAIT);
	EXPECT_EQ(target.lastMessage, -1);
}

TEST_F(WoodMonsterTest, ReleaseWaitsPastTwoSecondsAndForTheTargetStateToAllowIt)
{
	actor.actorState = static_cast<EActorState>(WOODMONSTER_STATE_WAIT);
	actor.timeInAir = 2.0f;
	actor.behaviourGet.Manage();
	EXPECT_EQ(actor.actorState, WOODMONSTER_STATE_WAIT);
	actor.timeInAir = 2.01f;
	target.stateConfig.flags_0x4 = 1;
	actor.behaviourGet.Manage();
	EXPECT_EQ(actor.actorState, WOODMONSTER_STATE_WAIT);
	target.actorState = static_cast<EActorState>(-1);
	actor.behaviourGet.Manage();
	EXPECT_EQ(actor.actorState, WOODMONSTER_STATE_RELEASE);
}

TEST_F(WoodMonsterTest, ReleaseSendsTheWaypointAndDurationAfterTheOriginalDelay)
{
	CWayPoint waypoint{};
	waypoint.location = edF32VECTOR3{ 1.0f, 2.0f, 3.0f };
	waypoint.rotation = edF32VECTOR3{ 0.0f, 0.5f, 0.0f };
	actor.behaviourGet.wayPointRef.pObj = STORE_POINTER(&waypoint);
	actor.currentLocation = edF32VECTOR4{ 4.0f, 5.0f, 6.0f, 1.0f };
	actor.actorState = static_cast<EActorState>(WOODMONSTER_STATE_RELEASE);
	actor.timeInAir = 0.35f;
	actor.behaviourGet.Manage();
	EXPECT_EQ(target.lastMessage, -1);
	actor.timeInAir = 0.36f;
	actor.behaviourGet.Manage();
	EXPECT_EQ(target.lastMessage, 0x33);
	EXPECT_FLOAT_EQ(target.position.y, 5.0f);
	EXPECT_FLOAT_EQ(target.destination.x, 1.0f);
	EXPECT_FLOAT_EQ(target.destination.z, 3.0f);
	EXPECT_FLOAT_EQ(target.rotation.y, 0.5f);
	EXPECT_FLOAT_EQ(target.duration, 3.0f);
	EXPECT_EQ(actor.actorState, WOODMONSTER_STATE_RELEASE_END);
	layer.field_0xcc = 2;
	actor.behaviourGet.Manage();
	EXPECT_EQ(actor.actorState, WOODMONSTER_STATE_STAND);
}
