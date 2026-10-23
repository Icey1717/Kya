#include <gtest/gtest.h>

#include "Frontend.h"
#include "FrontEndEnemy.h"
#include "TimeController.h"

namespace
{
	class EnemyLifeInterface : public CLifeInterface
	{
	public:
		bool Manage() override { return true; }
	};

	class FrontendEnemyListTest : public testing::Test
	{
	protected:
		void SetUp() override
		{
			previousTime = GetTimer()->totalTime;
			GetTimer()->totalTime = 0.0f;
			list.Reset();
		}

		void TearDown() override { GetTimer()->totalTime = previousTime; }

		void Advance()
		{
			GetTimer()->totalTime += 0.25f;
			list.Update(GetTimer()->totalTime);
		}

		float previousTime;
		CFrontendEnemyList list;
		EnemyLifeInterface interfaces[5];
	};
}

TEST_F(FrontendEnemyListTest, DisplaysFiveDistinctEnemies)
{
	for (auto& life : interfaces) list.AddFighterInterface(&life);
	for (int frame = 0; frame < 20; ++frame) Advance();

	EXPECT_EQ(list.enemyInterfaceTable.nbEntries, 0);
	for (int slot = 0; slot < 5; ++slot) {
		ASSERT_NE(list.aWidgets[slot], nullptr);
		EXPECT_TRUE(list.aWidgets[slot]->bVisible);
		EXPECT_EQ(list.aWidgets[slot]->state_0x37c, FE_ENEMY_STATE_ACTIVE);
		EXPECT_FLOAT_EQ(list.aWidgets[slot]->widgetSlotA.position.y, list.aSlots[slot].position.y);
		for (int other = 0; other < slot; ++other) {
			EXPECT_NE(list.aWidgets[slot], list.aWidgets[other]);
			EXPECT_NE(list.aWidgets[slot]->GetInterface(), list.aWidgets[other]->GetInterface());
		}
	}
}

TEST_F(FrontendEnemyListTest, PromotesNewAttackerWithoutLosingPreviousGauge)
{
	list.AddFighterInterface(&interfaces[0]);
	for (int frame = 0; frame < 4; ++frame) Advance();
	list.AddFighterInterface(&interfaces[1]);
	for (int frame = 0; frame < 4; ++frame) Advance();
	ASSERT_NE(list.aWidgets[0], nullptr);
	ASSERT_NE(list.aWidgets[1], nullptr);
	ASSERT_EQ(list.aWidgets[0]->GetInterface(), &interfaces[0]);

	interfaces[1].SetPriority(5);
	for (int frame = 0; frame < 4; ++frame) Advance();
	ASSERT_NE(list.aWidgets[0], nullptr);
	ASSERT_NE(list.aWidgets[1], nullptr);
	EXPECT_EQ(list.aWidgets[0]->GetInterface(), &interfaces[1]);
	EXPECT_EQ(list.aWidgets[1]->GetInterface(), &interfaces[0]);
	EXPECT_EQ(list.aWidgets[0]->field_0x380, 1);
	EXPECT_EQ(list.aWidgets[1]->field_0x380, 0);
}

TEST(EnemyInterfaceTable, HoldsSixteenPendingInterfacesAndIgnoresOverflow)
{
	// PS2 Add: 0x001d9d10, Contains: 0x001d9d40, Pop: 0x001d9d90.
	OtherInterface interfaces[17];
	EnemyInterfaceTable table{};

	EXPECT_EQ(table.Pop(), nullptr);
	for (int i = 0; i < 16; i++) {
		EXPECT_FALSE(table.Contains(&interfaces[i]));
		table.Add(&interfaces[i]);
		ASSERT_EQ(table.nbEntries, i + 1);
		EXPECT_TRUE(table.Contains(&interfaces[i]));
	}

	table.Add(&interfaces[16]);
	EXPECT_EQ(table.nbEntries, 16);
	EXPECT_FALSE(table.Contains(&interfaces[16]));

	for (int i = 15; i >= 0; i--) {
		EXPECT_EQ(table.Pop(), &interfaces[i]);
		EXPECT_EQ(table.nbEntries, i);
		EXPECT_FALSE(table.Contains(&interfaces[i]));
	}
	EXPECT_EQ(table.Pop(), nullptr);
}
