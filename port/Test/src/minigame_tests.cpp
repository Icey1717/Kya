#include <gtest/gtest.h>

#include "ActorMiniGameBoomy.h"
#include "ActorMiniGameBoxCounter.h"
#include "ActorMiniGameDistance.h"
#include "MemoryStream.h"
#include "TimeController.h"

namespace
{
	class MiniGameTest : public testing::Test
	{
	protected:
		void SetUp() override
		{
			previousDelta = GetTimer()->cutsceneDeltaTime;
			GetTimer()->cutsceneDeltaTime = 0.125f;
		}

		void TearDown() override
		{
			GetTimer()->cutsceneDeltaTime = previousDelta;
		}

		float previousDelta;
	};
}

TEST_F(MiniGameTest, BehaviourIdsSelectTheEmbeddedSharedBaseBehaviours)
{
	CActorMiniGameBoxCounter boxCounter;
	CActorMiniGameDistance distance;
	CActorMiniGameBoomy boomy;
	CActorMiniGame* games[] = { &boxCounter, &distance, &boomy };
	for (CActorMiniGame* pGame : games) {
		EXPECT_EQ(pGame->BuildBehaviour(2), pGame->GetBhvTraining());
		EXPECT_EQ(pGame->BuildBehaviour(3), pGame->GetBhvBetting());
		EXPECT_EQ(pGame->BuildBehaviour(4), pGame->GetBhvMulti());
		EXPECT_EQ(pGame->GetStateCfg(8)->flags_0x4, 0u);
	}
	EXPECT_EQ(boomy.GetStateCfg(9)->flags_0x4, 0u);
	EXPECT_EQ(boxCounter.GetUnity(), 3);
	EXPECT_EQ(distance.GetUnity(), 2);
	EXPECT_EQ(boomy.GetUnity(), 3);
}

TEST_F(MiniGameTest, BoxMessagesClampTheScoreAndTheCountdownUsesTheOriginalBoundary)
{
	CActorMiniGameBoxCounter game;
	game.field_0x1d0 = 0.0f;
	EXPECT_EQ(game.InterpretMessage(nullptr, 0x5a, nullptr), 1);
	EXPECT_FLOAT_EQ(game.field_0x1d0, 0.0f);
	EXPECT_EQ(game.InterpretMessage(nullptr, 0x59, nullptr), 1);
	EXPECT_FLOAT_EQ(game.field_0x1d0, 1.0f);

	game.bMustStop = false;
	game.timeLimit = 1.0f;
	game.timeRemaining = 0.25f;
	EXPECT_FALSE(game.MustStop());
	EXPECT_FLOAT_EQ(game.timeRemaining, 0.125f);
	EXPECT_TRUE(game.MustStop());
	game.timeLimit = -1.0f;
	game.timeRemaining = 2.0f;
	EXPECT_FALSE(game.MustStop());
	EXPECT_FLOAT_EQ(game.timeRemaining, 2.0f);
	game.bMustStop = true;
	EXPECT_TRUE(game.MustStop());
}

TEST_F(MiniGameTest, BoomyCountdownWaitsForTheSharedStartFlagAndPenaltiesCanStopTheGame)
{
	CActorMiniGameBoomy game;
	game.bMustStop = false;
	game.field_0x1b9 = 0;
	game.timeLimit = 1.0f;
	game.timeRemaining = 0.25f;
	game.maxPenalties = 2;
	game.penaltyCount = 0;
	game.hitCount = 0;
	EXPECT_FALSE(game.MustStop());
	EXPECT_FLOAT_EQ(game.timeRemaining, 0.25f);
	game.field_0x1b9 = 1;
	EXPECT_FALSE(game.MustStop());
	EXPECT_FLOAT_EQ(game.timeRemaining, 0.125f);
	EXPECT_TRUE(game.MustStop());

	game.timeLimit = -1.0f;
	EXPECT_EQ(game.InterpretMessage(nullptr, 0x59, nullptr), 1);
	EXPECT_EQ(game.hitCount, 1);
	EXPECT_EQ(game.InterpretMessage(nullptr, 0x5a, nullptr), 1);
	EXPECT_FALSE(game.MustStop());
	EXPECT_EQ(game.InterpretMessage(nullptr, 0x5a, nullptr), 1);
	EXPECT_TRUE(game.MustStop());
}

TEST_F(MiniGameTest, BoomySequenceReadsOnlyItsPayloadAndSkipsEmptySequences)
{
	int payload[] = { 3, 0, 4, 5, 0x12345678 };
	ByteCode byteCode;
	byteCode.SetPosition(reinterpret_cast<char*>(payload));
	S_MINI_GAME_BOOMY_SEQUENCE sequence;
	sequence.Create(&byteCode);
	EXPECT_EQ(byteCode.GetPosition(), reinterpret_cast<char*>(payload + 4));
	EXPECT_EQ(sequence.nbTargets, 3);
	EXPECT_EQ(sequence.GetTargetIndex(3), -1);
	_rgba color;
	sequence.GetTargetColor(1, &color);
	EXPECT_EQ(color.rgba, 0x800080ffu);
	delete[] sequence.aTargetIndices;

	CActorMiniGameBoomy game;
	game.nbSequences = 3;
	game.aSequences = new S_MINI_GAME_BOOMY_SEQUENCE[3]{};
	game.aSequences[1].nbTargets = 1;
	game.aSequences[1].aTargetIndices = new int[1]{ 5 };
	game.curSequence = 0;
	EXPECT_EQ(game.GetCurSequence(), &game.aSequences[1]);
	EXPECT_EQ(game.curSequence, 1);
	EXPECT_EQ(game.aSequences[1].curTarget, 0);
}

TEST_F(MiniGameTest, DistancePathLengthIncludesTheClosingSegmentOnlyForLoopPaths)
{
	edF32VECTOR4 points[] = {
		{ 0.0f, 0.0f, 0.0f, 1.0f },
		{ 3.0f, 0.0f, 0.0f, 1.0f },
		{ 3.0f, 4.0f, 0.0f, 1.0f }
	};
	CPathFollow path;
	path.splinePointCount = 3;
	path.aSplinePoints = points;
	path.type = 0;
	EXPECT_FLOAT_EQ(path.GetLength(), 7.0f);
	path.type = 1;
	EXPECT_FLOAT_EQ(path.GetLength(), 12.0f);
}
