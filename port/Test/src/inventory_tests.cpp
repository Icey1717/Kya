#include <gtest/gtest.h>
#include "ActorHero_Inventory.h"
#include "LevelScheduler.h"

namespace {
class InventoryRestore : public testing::Test
{
protected:
	void SetUp() override
	{
		savedGameInfo = CLevelScheduler::_gGameNfo;
	}

	void TearDown() override
	{
		CLevelScheduler::_gGameNfo = savedGameInfo;
	}

	GameInfo savedGameInfo;
	CLevelScheduler scheduler;
};
}

TEST_F(InventoryRestore, RepeatedSaveAndLoadPreservesCounts)
{
	CInventoryInterface inventory;
	ASSERT_TRUE(inventory.Cmd_AddItem(0x15, 3, 0));
	ASSERT_TRUE(inventory.Cmd_AddItem(0x1d, 5, 0));

	for (int i = 0; i < 3; i++) {
		scheduler.Game_SaveInventory(&inventory);
		scheduler.Game_LoadInventory(&inventory);
		scheduler.Game_LoadInventory(&inventory);
		ASSERT_NE(inventory.GetExistingInventorySlot(0, 0x15), nullptr);
		ASSERT_NE(inventory.GetExistingInventorySlot(1, 0x1d), nullptr);
		EXPECT_EQ(inventory.GetExistingInventorySlot(0, 0x15)->nbItems, 3);
		EXPECT_EQ(inventory.GetExistingInventorySlot(1, 0x1d)->nbItems, 5);
		EXPECT_EQ(inventory.aHeaderInfo[0].nbUsedSlots, 2);
		EXPECT_EQ(inventory.aHeaderInfo[1].nbUsedSlots, 2);
	}
}

TEST_F(InventoryRestore, SavedInventoryReplacesCurrentItemsAndSelection)
{
	CInventoryInterface savedInventory;
	ASSERT_TRUE(savedInventory.Cmd_AddItem(0x15, 3, 0));
	scheduler.Game_SaveInventory(&savedInventory);

	CInventoryInterface inventory;
	ASSERT_TRUE(inventory.Cmd_AddItem(0x16, 7, 0));
	ASSERT_TRUE(inventory.Cmd_AddItem(0x1d, 5, 0));
	inventory.aHeaderInfo[0].activeItemIndex = 1;
	scheduler.Game_LoadInventory(&inventory);

	ASSERT_NE(inventory.GetExistingInventorySlot(0, 0x15), nullptr);
	EXPECT_EQ(inventory.GetExistingInventorySlot(0, 0x15)->nbItems, 3);
	EXPECT_EQ(inventory.GetExistingInventorySlot(0, 0x16), nullptr);
	EXPECT_EQ(inventory.GetExistingInventorySlot(1, 0x1d), nullptr);
	EXPECT_EQ(inventory.aHeaderInfo[0].activeItemIndex, 0);
	EXPECT_EQ(inventory.aHeaderInfo[0].nbUsedSlots, 2);
	EXPECT_EQ(inventory.aHeaderInfo[0].pSlot, &inventory.aSlots[0][2]);
	EXPECT_EQ(inventory.aHeaderInfo[1].nbUsedSlots, 1);
}

TEST_F(InventoryRestore, EmptySaveClearsCurrentInventory)
{
	CInventoryInterface savedInventory;
	scheduler.Game_SaveInventory(&savedInventory);

	CInventoryInterface inventory;
	ASSERT_TRUE(inventory.Cmd_AddItem(0x15, 3, 0));
	ASSERT_TRUE(inventory.Cmd_AddItem(0x1d, 5, 0));
	scheduler.Game_LoadInventory(&inventory);

	EXPECT_EQ(inventory.GetExistingInventorySlot(0, 0x15), nullptr);
	EXPECT_EQ(inventory.GetExistingInventorySlot(1, 0x1d), nullptr);
	for (int i = 0; i < 2; i++) {
		EXPECT_EQ(inventory.aHeaderInfo[i].nbUsedSlots, 1);
		EXPECT_EQ(inventory.aHeaderInfo[i].pSlot, &inventory.aSlots[i][1]);
		EXPECT_EQ(inventory.aSlots[i][0].bInUse, -1);
	}
}
