#include <gtest/gtest.h>

#include "ActorFighter.h"
#include "TimeController.h"

namespace
{
	class ManageTestFighter : public CActorFighter
	{
	public:
		int UpdateFightCommand() override { return 0; }
		void AcquireAdversary() override {}
		void SetLookingAtOn() override { ++lookingAtCount; }
		void ManageDyn(float, uint, CActorsTable*) override { ++dynCount; }

		int lookingAtCount = 0;
		int dynCount = 0;
	};

	class ManageTestBehaviour : public CBehaviourFighter
	{
	public:
		void _ManageExit() override {}
	};

	class FighterManageTest : public testing::Test
	{
	protected:
		void SetUp() override
		{
			previousDelta = GetTimer()->cutsceneDeltaTime;
			GetTimer()->cutsceneDeltaTime = 0.05f;
			behaviour.pOwner = &fighter;
			fighter.pFighterCombo = nullptr;
			fighter.pAdversary = nullptr;
			fighter.pAnimationController = &animation;
			fighter.currentLocation = { 0.0f, 0.0f, 0.0f, 1.0f };
			fighter.actorState = static_cast<EActorState>(FIGHTER_DEFAULT_STATE_IDLE);
			fighter.field_0x474 = 0.0f;
			fighter.timeInAir = 1.0f;
			fighter.standAnim = -1;
			fighter.fightFlags = 0;
			animation.anmBinMetaAnimator.aAnimData = &layer;
			animation.currentAnimType = 0;
			layer.currentAnimDesc.animType = 0;
			layer.playRate = 0.25f;
		}

		void TearDown() override
		{
			GetTimer()->cutsceneDeltaTime = previousDelta;
		}

		ManageTestFighter fighter;
		ManageTestBehaviour behaviour;
		CAnimation animation;
		edAnmLayer layer{};
		float previousDelta;
	};
}

TEST_F(FighterManageTest, IdleUnlocksWhenOnlyActionLockIsSet)
{
	fighter.fightFlags = FIGHT_FLAG_ACTION_LOCKED;
	behaviour.Manage();
	EXPECT_EQ(fighter.fightFlags & FIGHT_FLAG_ACTION_LOCKED, 0u);
	EXPECT_EQ(fighter.lookingAtCount, 1);
	EXPECT_FLOAT_EQ(layer.playRate, 1.0f);
}

TEST_F(FighterManageTest, IdleKeepsAnimationRateWhenOnlyUnrelatedFlagsAreSet)
{
	fighter.fightFlags = FIGHT_FLAG_ALLOW_PROJECTED_FALLBACK;
	behaviour.Manage();
	EXPECT_EQ(fighter.fightFlags, FIGHT_FLAG_ALLOW_PROJECTED_FALLBACK);
	EXPECT_EQ(fighter.lookingAtCount, 0);
	EXPECT_FLOAT_EQ(layer.playRate, 0.25f);
}

TEST_F(FighterManageTest, PendingGrabContinuationUpdatesDynamicsOnce)
{
	s_fighter_grab grab{};
	fighter.actorState = static_cast<EActorState>(0x6e);
	fighter.pExecutingGrab = &grab;
	layer.animPlayState = 0;
	behaviour.Manage();
	EXPECT_EQ(fighter.actorState, 0x6e);
	EXPECT_EQ(fighter.dynCount, 1);
}
