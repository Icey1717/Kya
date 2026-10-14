#include <gtest/gtest.h>

#include "Actor.h"
#include "MemoryStream.h"
#include "SwitchBehaviour.h"

TEST(SwitchBehaviour, ReentryRequiresANewMessageToRestartPatrol)
{
	// One message switch: message 0xf starts guard-area behaviour 10.
	int words[] = { 1, 10 };
	ByteCode stream;
	stream.currentSeekPos = reinterpret_cast<char*>(words);
	TCouple_Condition_Execution<CondMessage, CExecuteSwitchBehaviour> couple{};
	couple.GetCondition()->Create(&stream);
	couple.GetExecution()->Create(&stream);
	CCoupleConditionExecution* entries[] = { &couple };
	CSwitchBehaviour switches;
	switches.nbTotalSwitches = 1;
	switches.apSwitches = entries;

	switches.Begin(nullptr);
	EXPECT_EQ(switches.Test(nullptr), 0);
	EXPECT_EQ(switches.Execute(nullptr), -1);
	EXPECT_EQ(switches.InterpretMessage(nullptr, nullptr, 0xf, nullptr), 1);
	EXPECT_EQ(switches.Test(nullptr), 1);
	EXPECT_EQ(switches.Execute(nullptr), 10);

	// Patrol finishes and the waiting behaviour starts again. The old message
	// must not launch another patrol while the metro is away.
	switches.Begin(nullptr);
	EXPECT_EQ(switches.Test(nullptr), 0);
	EXPECT_EQ(switches.Execute(nullptr), -1);
	EXPECT_EQ(switches.InterpretMessage(nullptr, nullptr, 0xe, nullptr), 0);
	EXPECT_EQ(switches.Test(nullptr), 0);
	EXPECT_EQ(switches.InterpretMessage(nullptr, nullptr, 0xf, nullptr), 1);
	EXPECT_EQ(switches.Test(nullptr), 1);
	EXPECT_EQ(switches.Execute(nullptr), 10);

}
