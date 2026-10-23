#include <gtest/gtest.h>

#include "ActorWolfen.h"
#include "ActorHero.h"
#include "MemoryStream.h"
#include "EventManager.h"
#include "MathOps.h"
#include "edMem.h"
#include "EdenLib/edSys/sources/EdSystem.h"
#include <memory>
#include <vector>

extern ed_event_chunk** pedEventChunks;
extern EventQueue EventQueue_00448fbc;

namespace
{
	class FlipTestWolfen : public CActorWolfen
	{
	public:
		bool Func_0x19c() override { return false; }
	};

	class FlipTestBehaviour : public CBehaviourFighterWolfen
	{
	public:
		bool Execute(s_fighter_action* action, s_fighter_action_param*) override
		{
			commands.push_back(*action);
			return true;
		}

		std::vector<s_fighter_action> commands;
	};

	class CommanderTestWolfen : public CActorWolfen
	{
	public:
		int ReceiveMessage(CActor* sender, ACTOR_MESSAGE msg, MSG_PARAM param) override
		{
			EXPECT_EQ(msg, 0x1a);
			auto* command = static_cast<InitPathfindingClientMsgParams*>(param);
			EXPECT_EQ(command->msgId, 5u);
			target = command->pActor;
			++messageCount;
			if (forwardToWolfen) {
				return CActorWolfen::InterpretMessage(sender, msg, param);
			}
			return 1;
		}

		CActorFighter* target = nullptr;
		int messageCount = 0;
		bool forwardToWolfen = false;
	};
}

TEST(WolfenFight, CommanderAttackCarriesConfiguredTargetAndHeroFallback)
{
	CActorCommander commander;
	CommanderTestWolfen wolfen;
	CActorHero hero;
	CActorFighter configuredTarget;
	ACommanderComponent_10 sequence[] = { { 0, 0, 1, 1 } };
	CTeamElt team{};
	team.pEnemyActor = &wolfen;
	commander.aTeamElt = &team;
	commander.nbTeams = 1;
	commander.aComponent_0x170 = sequence;
	commander.field_0x168 = 0;
	commander.squad.eltTable.nbEntries = 0;
	wolfen.flags = 0;
	wolfen.curBehaviourId = WOLFEN_BEHAVIOUR_WATCH_DOG;
	wolfen.combatMode_0xb7c = ECM_None;

	auto* previousHero = CActorHero::_gThis;
	CActorHero::_gThis = &hero;
	commander.actorRef.pObj = STORE_POINTER(&configuredTarget);
	commander.field_0x160 = 0;
	commander.UpdateAttack();
	EXPECT_EQ(wolfen.messageCount, 1);
	EXPECT_EQ(wolfen.target, &configuredTarget);
	EXPECT_EQ(commander.field_0x160, 1);

	commander.actorRef.pObj = 0;
	commander.field_0x160 = 0;
	commander.UpdateAttack();
	EXPECT_EQ(wolfen.messageCount, 2);
	EXPECT_EQ(wolfen.target, &hero);
	EXPECT_EQ(commander.field_0x160, 1);
	CActorHero::_gThis = previousHero;
}

TEST(WolfenFight, BellSequenceLoadsTheLevelWaveAndAlertsFiveWolfen)
{
	CActorCommander commander;
	// COMMANDER_01_L1: release five enemies, then finish after five deaths.
	int words[] = { 2, 0, 5, 5, 0 };
	ByteCode stream;
	stream.currentSeekPos = reinterpret_cast<char*>(words);
	commander.CreateSequence(&stream);
	std::unique_ptr<ACommanderComponent_10[]> sequence(commander.aComponent_0x170);
	ASSERT_EQ(commander.count_0x16c, 2);
	EXPECT_EQ(stream.currentSeekPos, reinterpret_cast<char*>(words + std::size(words)));
	EXPECT_EQ(sequence[0].field_0x0, 0);
	EXPECT_EQ(sequence[0].field_0x4, 0);
	EXPECT_EQ(sequence[0].field_0x8, 5);
	EXPECT_EQ(sequence[0].field_0xc, 5);
	EXPECT_EQ(sequence[1].field_0x0, 5);
	EXPECT_EQ(sequence[1].field_0x4, 5);
	EXPECT_EQ(sequence[1].field_0x8, 0);
	EXPECT_EQ(sequence[1].field_0xc, 5);

	CommanderTestWolfen wolfen[8];
	CTeamElt team[8]{};
	CActorHero hero;
	hero.typeID = ACTOR_HERO_PRIVATE;
	hero.lifeInterface.currentValue = 1.0f;
	hero.pCollisionData = nullptr;
	commander.actorRef.pObj = STORE_POINTER(&hero);
	commander.aTeamElt = team;
	commander.nbTeams = 8;
	commander.field_0x168 = 0;
	commander.field_0x174 = nullptr;
	commander.bInCombat_0x1b0 = 1;
	commander.squad.eltTable.nbEntries = 0;
	for (int i = 0; i < 8; ++i) {
		team[i].pEnemyActor = &wolfen[i];
		wolfen[i].flags = 0;
		wolfen[i].curBehaviourId = WOLFEN_BEHAVIOUR_WATCH_DOG;
		wolfen[i].combatMode_0xb7c = ECM_None;
		wolfen[i].combatFlags_0xb78 = 0;
		wolfen[i].lifeInterface.currentValue = 1.0f;
		wolfen[i].pCommander = &commander;
		wolfen[i].forwardToWolfen = true;
	}

	// Detection alone does not release the wave. The bell sets flag 8.
	commander.flags_0x18c = 2;
	commander._UpdateSequence();
	for (auto& enemy : wolfen) EXPECT_EQ(enemy.messageCount, 0);
	commander.flags_0x18c |= 8;
	commander._UpdateSequence(); // Latch the bell activation.
	commander._UpdateSequence(); // Release the parsed wave through real Wolfen handling.
	EXPECT_EQ(commander.field_0x160, 5);
	for (int i = 0; i < 5; ++i) {
		auto& enemy = wolfen[i];
		EXPECT_EQ(enemy.messageCount, 1);
		EXPECT_EQ(enemy.pTargetActor_0xc80, &hero);
		EXPECT_EQ(enemy.combatMode_0xb7c, ECM_InCombat);
	}
	commander._UpdateSequence();
	for (int i = 0; i < 8; ++i) EXPECT_EQ(wolfen[i].messageCount, i < 5 ? 1 : 0);

	// Reaching the final threshold must not read past the two parsed entries.
	for (int i = 0; i < 5; ++i) wolfen[i].lifeInterface.currentValue = 0.0f;
	commander._UpdateSequence();
	EXPECT_EQ(commander.field_0x168, 1);
	EXPECT_EQ(commander.field_0x164, 5);
	for (int i = 5; i < 8; ++i) EXPECT_EQ(wolfen[i].messageCount, 0);
}

TEST(WolfenFight, CommanderSequenceAccumulatesThresholdsAndReleases)
{
	CActorCommander commander;
	int words[] = { 3, 0, 2, 2, 3, 3, 0 };
	ByteCode stream;
	stream.currentSeekPos = reinterpret_cast<char*>(words);
	commander.CreateSequence(&stream);
	std::unique_ptr<ACommanderComponent_10[]> sequence(commander.aComponent_0x170);
	EXPECT_EQ(sequence[0].field_0x4, 0);
	EXPECT_EQ(sequence[0].field_0xc, 2);
	EXPECT_EQ(sequence[1].field_0x4, 2);
	EXPECT_EQ(sequence[1].field_0xc, 5);
	EXPECT_EQ(sequence[2].field_0x4, 5);
	EXPECT_EQ(sequence[2].field_0xc, 5);

	int emptyWords[] = { 0 };
	stream.currentSeekPos = reinterpret_cast<char*>(emptyWords);
	commander.CreateSequence(&stream);
	EXPECT_EQ(commander.count_0x16c, 0);
	EXPECT_EQ(commander.aComponent_0x170, nullptr);
}

TEST(WolfenFight, RejectedHeadFlipIssuesDodgeWithoutCancellingIt)
{
	if (!edSystem.system.startAddress) {
		ASSERT_TRUE(_edSystemInit(0, nullptr));
		edMemInit(0x1000);
	}
	const auto previousQueue = EventQueue_00448fbc;
	const auto previousCallbacks = CEventManager::callbackFunctions;
	CEventManager eventManager;
	ed_zone_3d zone{};
	edF32MATRIX4 matrix = gF32Matrix4Unit;
	byte primitiveType = 2;
	uint primitiveList[] = { 1, 0 };
	zone.field_0x0[0] = STORE_POINTER(primitiveList);
	ed_event_chunk chunk{};
	chunk.aZones = &zone;
	chunk.field_0x8 = &primitiveType;
	chunk.aMatrices = &matrix;
	ed_event_chunk* chunks[] = { &chunk };
	auto* previousChunks = pedEventChunks;
	auto* previousEventManager = CScene::ptable.g_EventManager_006f5080;
	pedEventChunks = chunks;
	CScene::ptable.g_EventManager_006f5080 = &eventManager;
	eventManager.activeChunkId = 0;

	CActorCommander commander;
	commander.detectAreaZoneId = 0;
	FlipTestWolfen wolfen;
	CActorFighter adversary;
	adversary.pBlow = nullptr;
	FlipTestBehaviour behaviour;
	WolfenComboData commands[7]{};
	commands[6].field_0x0 = 0x40000;
	wolfen.field_0xb64 = commands;
	wolfen.field_0xb74 = 2;
	wolfen.pCommander = &commander;
	wolfen.pAdversary = &adversary;
	wolfen.validCommandMask.all = 0xffffffff;
	wolfen.fightFlags = 0;
	wolfen.field_0x450.all = 0;
	wolfen.field_0x44c = 0;
	wolfen.field_0x44d = 0;
	wolfen.curBehaviourId = FIGHTER_BEHAVIOUR_DEFAULT;
	wolfen.currentLocation = { 0.0f, 0.0f, 0.0f, 1.0f };
	wolfen.rotationQuat = { 0.0f, 0.0f, 1.0f, 0.0f };
	wolfen.field_0x4c4 = 0.1f;
	wolfen.pathFinderClient.id = -1;
	behaviour.pOwner = &wolfen;
	behaviour.currentCommandId = -1;

	EXPECT_EQ(behaviour.InterpretMessage(nullptr, MESSAGE_JUMP_ON, nullptr), 0);
	EXPECT_EQ(behaviour.currentCommandId, 6);
	EXPECT_EQ(behaviour.field_0x68, 1);
	EXPECT_EQ(behaviour.commands.size(), 2u);
	if (behaviour.commands.size() == 2) {
		EXPECT_EQ(behaviour.commands[0].actionByte & 0xf, 8);
		EXPECT_EQ(behaviour.commands[1].moveByte & 0xf0, 0x20);
		EXPECT_EQ(behaviour.commands[1].moveByte & 0xf, 6);
		EXPECT_EQ(behaviour.commands[1].actionByte & 0xf, 0);
	}

	wolfen.field_0xb64 = nullptr;
	pedEventChunks = previousChunks;
	CScene::ptable.g_EventManager_006f5080 = previousEventManager;
	edMemFree(EventQueue_00448fbc.aEntries);
	EventQueue_00448fbc = previousQueue;
	CEventManager::callbackFunctions = previousCallbacks;
}
