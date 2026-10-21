#include <gtest/gtest.h>

#include "InputManager.h"
#include "CameraViewManager.h"
#include "Scene.h"

namespace
{
	class PlayerInputTest : public testing::Test
	{
	protected:
		void SetUp() override
		{
			previousGameFlags = GameFlags;
			previousCameraManager = CCameraManager::_gThis;
			// Freeze device polling so UpdateOne consumes the supplied route samples.
			GameFlags |= 0x20;
			CCameraManager::_gThis = 0;
			for (auto& route : input.aRoutes) {
				route.portId = -1;
			}
			input.field_0x1c.field_0x18 = 4;
			input.field_0x40.field_0x18 = 4;
			for (int stickIndex = 0; stickIndex < 2; ++stickIndex) {
				auto& stick = input.aAnalogSticks[stickIndex];
				stick.leftRouteId = stickIndex * 4;
				stick.rightRouteId = stickIndex * 4 + 1;
				stick.upRouteId = stickIndex * 4 + 2;
				stick.downRouteId = stickIndex * 4 + 3;
			}
		}

		void TearDown() override
		{
			GameFlags = previousGameFlags;
			CCameraManager::_gThis = previousCameraManager;
		}

		CPlayerInput input{};
		uint previousGameFlags;
		CCameraManager* previousCameraManager;
	};
}

TEST_F(PlayerInputTest, LeftOnlyClearsVerticalInputAfterADiagonal)
{
	for (int stickIndex = 0; stickIndex < 2; ++stickIndex) {
		for (int verticalDirection = 2; verticalDirection <= 3; ++verticalDirection) {
			const int leftRoute = stickIndex * 4;
			const int verticalRoute = leftRoute + verticalDirection;
			input.aButtons[leftRoute].analogValue = 0.8f;
			input.aButtons[verticalRoute].analogValue = 0.5f;
			input.UpdateOne(0.02f);
			ASSERT_NE(input.aAnalogSticks[stickIndex].y, 0.0f);

			input.aButtons[verticalRoute].analogValue = 0.0f;
			input.UpdateOne(0.02f);
			EXPECT_LT(input.aAnalogSticks[stickIndex].x, 0.0f);
			EXPECT_FLOAT_EQ(input.aAnalogSticks[stickIndex].y, 0.0f);
		}
	}
}

TEST_F(PlayerInputTest, LeftDriftInsideDeadzoneReturnsBothAxesToNeutral)
{
	input.aButtons[0].analogValue = 0.8f;
	input.aButtons[2].analogValue = 0.5f;
	input.UpdateOne(0.02f);
	ASSERT_NE(input.aAnalogSticks[0].y, 0.0f);

	input.aButtons[0].analogValue = 0.1f;
	input.aButtons[2].analogValue = 0.0f;
	input.UpdateOne(0.02f);
	EXPECT_FLOAT_EQ(input.aAnalogSticks[0].x, 0.0f);
	EXPECT_FLOAT_EQ(input.aAnalogSticks[0].y, 0.0f);
	EXPECT_FLOAT_EQ(input.aAnalogSticks[0].magnitude, 0.0f);
}
