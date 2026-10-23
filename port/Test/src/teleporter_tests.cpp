#include <gtest/gtest.h>

#include "ActorTeleporter.h"
#include "EventManager.h"
#include "LevelScheduler.h"

namespace
{
	class TeleporterReturnTest : public testing::Test
	{
	protected:
		void SetUp() override
		{
			previousScheduler = CLevelScheduler::gThis;
			CLevelScheduler::gThis = &scheduler;
			scheduler.currentLevelID = 0;
			for (auto& level : scheduler.aLevelInfo) {
				level.maxElevatorId = 0;
				for (auto& subsector : level.aSubSectorInfo) subsector = {};
			}
			scheduler.aLevelInfo[10].maxElevatorId = 4;
			// NATIV TELEPORTER_L7 destinations use cutscene 123. It is not an elevator ID.
			teleporter.pDestinationList = reinterpret_cast<S_DESTINATION_LIST*>(&destinations);
			teleporter.bOpen = 1;
			teleporter.field_0x1e0.pObj = STORE_POINTER(&zone);
		}

		void TearDown() override
		{
			RELEASE_POINTER(teleporter.field_0x1e0.pObj);
			CLevelScheduler::gThis = previousScheduler;
		}

		CLevelScheduler* previousScheduler;
		CLevelScheduler scheduler;
		CActorTeleporter teleporter;
		ed_zone_3d zone{};
		struct
		{
			int nbEntries = 3;
			S_DESTINATION_ENTRY entries[3] = { {10, 1, 123}, {10, 2, 123}, {10, 3, 123} };
		} destinations;
	};
}

TEST_F(TeleporterReturnTest, OpenSourceElevatorAllowsHubDoorToOpen)
{
	EXPECT_FALSE(teleporter.LevelHasTeleporters());
	scheduler.aLevelInfo[10].aSubSectorInfo[2].flags = 1;
	EXPECT_TRUE(teleporter.LevelHasTeleporters());
}

TEST_F(TeleporterReturnTest, DifferentLevelDoesNotUnlockHubDoor)
{
	scheduler.aLevelInfo[2].maxElevatorId = 4;
	scheduler.aLevelInfo[2].aSubSectorInfo[2].flags = 1;
	EXPECT_FALSE(teleporter.LevelHasTeleporters());
}

TEST_F(TeleporterReturnTest, ClosedHubTeleporterDoesNotUnlockDoor)
{
	scheduler.aLevelInfo[10].aSubSectorInfo[2].flags = 1;
	teleporter.bOpen = 0;
	EXPECT_FALSE(teleporter.LevelHasTeleporters());
}

TEST_F(TeleporterReturnTest, DestinationMaskUsesLevelAndElevatorRatherThanCutscene)
{
	scheduler.aLevelInfo[10].aSubSectorInfo[2].flags = 1;
	teleporter.DetectDisabledDestinations(1);
	EXPECT_EQ(teleporter.disabledDestinationMask & 7U, 5U);
	// Changing a cutscene must not change availability.
	destinations.entries[1].field_0x8 = -1;
	teleporter.DetectDisabledDestinations(1);
	EXPECT_EQ(teleporter.disabledDestinationMask & 7U, 5U);
}

TEST_F(TeleporterReturnTest, LiveDestinationMaskRefreshUsesTheDestinationElevator)
{
	scheduler.currentLevelID = 10;
	scheduler.aLevelInfo[10].aSubSectorInfo[3].flags = 1;
	teleporter.disabledDestinationMask = 0xffffffffU;
	teleporter.DetectDisabledDestinations(0);
	EXPECT_EQ(teleporter.disabledDestinationMask & 7U, 3U);
}
