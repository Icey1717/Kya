#include <gtest/gtest.h>

#include "ActorShocker.h"
#include "DlistManager.h"
#include "MemoryStream.h"

namespace
{
	class ShockerActor : public CActorShocker
	{
	public:
		void SetState(int newState, int animationType) override
		{
			this->actorState = static_cast<EActorState>(newState);
			lastAnimationType = animationType;
			return;
		}

		void LifeDecrease(float amount) override
		{
			damage = amount;
			this->lifeInterface.currentValue = 0.0f;
			return;
		}

		int lastAnimationType = -2;
		float damage = 0.0f;
	};

	class ShockerReceiver : public CActor
	{
	public:
		int ReceiveMessage(CActor* pSender, ACTOR_MESSAGE msg, MSG_PARAM pMsgParam) override
		{
			lastMessage = msg;
			if (msg == MESSAGE_KICKED) {
				hit = *static_cast<_msg_hit_param*>(pMsgParam);
			}
			else {
				command = *static_cast<int*>(pMsgParam);
			}
			return 1;
		}

		int lastMessage = -1;
		int command = -1;
		_msg_hit_param hit{};
	};

	class ShockerTest : public testing::Test
	{
	protected:
		void SetUp() override
		{
			actor.addOnGenerator = {};
			actor.lifeInterface.currentValue = 5.0f;
			actor.actorState = static_cast<EActorState>(SHOCKER_STATE_CHASE);
			actor.behaviourShockerFireWave.pOwner = &actor;
			previousDlistManager = CScene::ptable.g_GlobalDListManager_004516bc;
			dlistManager.bCompletedLevelInit = 0;
			dlistManager.activeSectorPatchId = -1;
			CScene::ptable.g_GlobalDListManager_004516bc = &dlistManager;
			for (int i = 0; i < 4; i++) {
				CFxSpark* pSpark = &actor.behaviourShockerFireWave.circularWaveShoot.aFxSparks[i];
				pSpark->count_0x98 = 0;
				pSpark->pVector_0xc = nullptr;
				pSpark->dlistPatchId = -1;
			}
			return;
		}

		void TearDown() override
		{
			CScene::ptable.g_GlobalDListManager_004516bc = previousDlistManager;
			return;
		}

		ShockerActor actor;
		ShockerReceiver receiver;
		CGlobalDListManager dlistManager;
		CGlobalDListManager* previousDlistManager;
	};
}

TEST_F(ShockerTest, BeginDiscardsThePreviousWaveAndSelectsSleepOrTheRequestedState)
{
	CCircularWaveShoot& wave = actor.behaviourShockerFireWave.circularWaveShoot;
	wave.field_0x30 = 1;
	wave.field_0x2c = 12.0f;
	wave.field_0x40 = edF32VECTOR4{ 1.0f, 2.0f, 3.0f, 1.0f };
	wave.actorsTable.Add(&receiver);
	actor.behaviourShockerFireWave.Begin(&actor, -1, 123);
	EXPECT_EQ(actor.actorState, SHOCKER_STATE_SLEEP);
	EXPECT_EQ(actor.lastAnimationType, -1);
	EXPECT_EQ(wave.field_0x30, 0);
	EXPECT_FLOAT_EQ(wave.field_0x2c, 0.0f);
	EXPECT_FLOAT_EQ(wave.field_0x40.y, 0.0f);
	EXPECT_EQ(wave.actorsTable.nbEntries, 0);
	actor.behaviourShockerFireWave.Begin(&actor, SHOCKER_STATE_CHASE, 7);
	EXPECT_EQ(actor.actorState, SHOCKER_STATE_CHASE);
	EXPECT_EQ(actor.lastAnimationType, 7);
}

TEST_F(ShockerTest, FireCommandsRespectTerminalStatesAndTheOriginalFlagOverride)
{
	int command = 0;
	EXPECT_EQ(actor.InterpretMessage(&receiver, 0x69, &command), 1);
	EXPECT_EQ(actor.field_0x3a4, &receiver);
	command = 1;
	EXPECT_EQ(actor.InterpretMessage(&receiver, 0x69, &command), 1);
	EXPECT_EQ(actor.actorState, SHOCKER_STATE_FIRE_WAVE_WIND_UP);
	actor.actorState = static_cast<EActorState>(SHOCKER_STATE_DEATH);
	actor.flags = 0;
	EXPECT_EQ(actor.InterpretMessage(&receiver, 0x69, &command), 0);
	EXPECT_EQ(actor.actorState, SHOCKER_STATE_DEATH);
	actor.flags = 4;
	EXPECT_EQ(actor.InterpretMessage(&receiver, 0x69, &command), 1);
	EXPECT_EQ(actor.actorState, SHOCKER_STATE_FIRE_WAVE_WIND_UP);
}

TEST_F(ShockerTest, ChargedHitsRetaliateWhileVulnerableHitsDamageTheShocker)
{
	_msg_hit_param hit{};
	hit.projectileType = 4;
	hit.damage = 3.5f;
	actor.field_0x474 = true;
	actor.field_0x36c = 7.25f;
	receiver.typeID = ACTOR_HERO_PRIVATE;
	EXPECT_EQ(actor.InterpretMessage(&receiver, MESSAGE_KICKED, &hit), 1);
	EXPECT_FLOAT_EQ(actor.damage, 0.0f);
	EXPECT_EQ(receiver.lastMessage, MESSAGE_KICKED);
	EXPECT_EQ(receiver.hit.projectileType, 5);
	EXPECT_FLOAT_EQ(receiver.hit.damage, 7.25f);
	actor.field_0x474 = false;
	EXPECT_EQ(actor.InterpretMessage(&receiver, MESSAGE_KICKED, &hit), 1);
	EXPECT_FLOAT_EQ(actor.damage, 3.5f);
	EXPECT_EQ(actor.actorState, SHOCKER_STATE_DEATH);
}

TEST_F(ShockerTest, TypeTenDamageBypassesChargeAndPreservesItsOriginalReturnValue)
{
	_msg_hit_param hit{};
	hit.projectileType = 10;
	hit.damage = 2.5f;
	actor.field_0x474 = true;
	EXPECT_EQ(actor.InterpretMessage(&receiver, MESSAGE_KICKED, &hit), 0);
	EXPECT_FLOAT_EQ(actor.damage, 2.5f);
	EXPECT_EQ(actor.actorState, SHOCKER_STATE_DEATH);
	actor.damage = 0.0f;
	EXPECT_EQ(actor.InterpretMessage(&receiver, MESSAGE_KICKED, &hit), 0);
	EXPECT_FLOAT_EQ(actor.damage, 0.0f);
}

TEST_F(ShockerTest, TiredAndRechargePhasesRetainChargeUntilTheRechargeDelay)
{
	actor.field_0x380 = 4;
	actor.ComputeInvincibility();
	EXPECT_TRUE(actor.field_0x474);
	EXPECT_FALSE(actor.field_0x3a9);
	actor.actorState = static_cast<EActorState>(SHOCKER_STATE_FIRE_WAVE_LAND);
	actor.ComputeInvincibility();
	EXPECT_FALSE(actor.field_0x474);
	EXPECT_TRUE(actor.field_0x3a9);
	actor.actorState = static_cast<EActorState>(SHOCKER_STATE_STAND_UP_TIRED_1_2);
	actor.ComputeInvincibility();
	EXPECT_FALSE(actor.field_0x474);
	actor.actorState = static_cast<EActorState>(SHOCKER_STATE_RECHARGE);
	actor.ComputeInvincibility();
	EXPECT_FALSE(actor.field_0x474);
	EXPECT_FALSE(actor.field_0x3a9);
	actor.timeInAir = 1.0f;
	actor.StateShockerRecharge();
	EXPECT_FALSE(actor.field_0x474);
	actor.timeInAir = 1.01f;
	actor.StateShockerRecharge();
	EXPECT_TRUE(actor.field_0x474);
}

TEST_F(ShockerTest, CommandWaitNotifiesTheControllerAndLandingClearsItsImpulse)
{
	actor.field_0x3a4 = &receiver;
	actor.behaviourShockerFireWave.InitState(SHOCKER_STATE_WAIT_COMMAND);
	EXPECT_EQ(receiver.lastMessage, 0x69);
	EXPECT_EQ(receiver.command, 2);
	actor.behaviourShockerFireWave.TermState(SHOCKER_STATE_WAIT_COMMAND, SHOCKER_STATE_FIRE_WAVE_WIND_UP);
	EXPECT_EQ(receiver.command, 3);
	actor.dynamic.speed = 3.0f;
	actor.dynamicExt.normalizedTranslation = edF32VECTOR4{ 1.0f, 2.0f, 3.0f, 0.0f };
	actor.dynamicExt.field_0x6c = 4.0f;
	actor.behaviourShockerFireWave.TermState(SHOCKER_STATE_FIRE_WAVE_LAND, SHOCKER_STATE_STAND_UP_TIRED_1_2);
	EXPECT_FLOAT_EQ(actor.dynamic.speed, 0.0f);
	EXPECT_FLOAT_EQ(actor.dynamicExt.normalizedTranslation.y, 0.0f);
	EXPECT_FLOAT_EQ(actor.dynamicExt.field_0x6c, 0.0f);
}

TEST_F(ShockerTest, WaveBytecodeKeepsImpulseAsAFloatAndConsumesBothResourceIds)
{
	struct WaveStream
	{
		float values[6];
		int materialId;
		int effectId;
	} stream{ { 20.0f, 2.0f, 1.0f, 3.0f, 4.0f, 5.25f }, 123, -1 };
	ByteCode byteCode;
	byteCode.currentSeekPos = reinterpret_cast<char*>(&stream);
	actor.behaviourShockerFireWave.Create(&byteCode);
	CCircularWaveShoot& wave = actor.behaviourShockerFireWave.circularWaveShoot;
	EXPECT_FLOAT_EQ(wave.field_0x1c, 5.25f);
	EXPECT_EQ(wave.field_0x20, 123);
	EXPECT_EQ(wave.field_0x24, -1);
	EXPECT_EQ(byteCode.currentSeekPos, reinterpret_cast<char*>(&stream + 1));
}
