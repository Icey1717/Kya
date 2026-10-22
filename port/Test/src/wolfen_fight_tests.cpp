#include <gtest/gtest.h>

#include "ActorWolfen.h"
#include "EventManager.h"
#include "MathOps.h"
#include "edMem.h"
#include "EdenLib/edSys/sources/EdSystem.h"
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
