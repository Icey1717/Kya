#include <gtest/gtest.h>

#include "ActorHero_Private.h"
#include "ActorWolfen.h"

namespace
{
	class FightLockWolfen : public CActorWolfen
	{
	public:
		bool IsInHitState() override { return false; }
	};

	class HeroFightLockTest : public testing::Test
	{
	protected:
		void SetUp() override
		{
			lock.pOwner = &owner;
			lock.nbPotentialAdversaries = 2;
			lock.pAdversary = &enemies[0];
			lock.adversaryIndex = 0;
			commander.aTeamElt = teams;
			commander.nbTeams = 2;
			owner.currentLocation = { 0.0f, 0.0f, 0.0f, 1.0f };
			for (int i = 0; i < 2; ++i) {
				enemies[i].pCommander = &commander;
				enemies[i].curBehaviourId = 3;
				enemies[i].field_0x6c8 = 1.0f;
				enemies[i].fightFlags = 0;
				enemies[i].currentLocation = { 0.0f, 0.0f, 1.0f + i, 1.0f };
				teams[i].pEnemyActor = &enemies[i];
				lock.aAdversaries[i].pActorFighter = &enemies[i];
			}
			const InferenceRuleFunc rules[] = {
				CFightLock_SE::_Rule_Flee, CFightLock_SE::_Rule_DangerCritical,
				CFightLock_SE::_Rule_Attack, CFightLock_SE::_Rule_DangerMax,
				CFightLock_SE::_Rule_KeepFocus, CFightLock_SE::_Rule_DangerMin,
				CFightLock_SE::_Rule_DistMin
			};
			for (int i = 0; i < 7; ++i) lock.aInferenceRules[i].pFunc = rules[i];
		}

		CFightLock_SE lock{};
		CActorFighter owner;
		FightLockWolfen enemies[2];
		CActorCommander commander;
		CTeamElt teams[2]{};
	};
}

TEST_F(HeroFightLockTest, SelectsNewCommanderAttackerWhenPreviousTargetBecomesSupporter)
{
	// PS2 danger heuristic: 0x00324d90; inference rules: 0x003253d0.
	teams[0].field_0x14 = 1;
	teams[1].field_0x14 = 0;
	lock._Heuristic_Danger();
	EXPECT_FLOAT_EQ(lock.aAdversaries[0].field_0x14, 0.5f);
	EXPECT_FLOAT_EQ(lock.aAdversaries[1].field_0x14, 2.0f);
	EXPECT_EQ(lock.ApplyInferenceRules(), &enemies[1]);
	EXPECT_EQ(lock.adversaryIndex, 1);
}

TEST_F(HeroFightLockTest, KeepsFocusWhenBothEnemiesRemainActiveThreats)
{
	teams[0].field_0x14 = 0;
	teams[1].field_0x14 = 0;
	lock._Heuristic_Danger();
	EXPECT_FLOAT_EQ(lock.aAdversaries[0].field_0x14, 2.0f);
	EXPECT_FLOAT_EQ(lock.aAdversaries[1].field_0x14, 2.0f);
	EXPECT_EQ(lock.ApplyInferenceRules(), &enemies[0]);
}
