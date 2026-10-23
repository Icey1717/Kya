#include <gtest/gtest.h>

#include "Frontend.h"
#include "FrontEndEnemy.h"

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
