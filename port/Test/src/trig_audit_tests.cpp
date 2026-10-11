#include <gtest/gtest.h>
#include <cmath>
#include "MathOps.h"

TEST(OrderedEulerTrig, ZeroAnglesProduceIdentityForEveryAxisOrder)
{
	char orders[][4] = { "XYZ", "XZY", "YXZ", "YZX", "ZXY", "ZYX" };
	float angles[3] = {};
	for (auto& order : orders) {
		SCOPED_TRACE(order);
		edF32MATRIX4 matrix{};
		edF32Matrix4FromEulerOrdSoft(&matrix, order, angles);
		for (int row = 0; row < 4; ++row) {
			for (int column = 0; column < 4; ++column) {
				EXPECT_NEAR(matrix.raw[row * 4 + column], row == column ? 1.0f : 0.0f, 1e-5f);
			}
		}
	}
}

TEST(OrderedEulerTrig, XRotationMatchesIndependentRotationMatrix)
{
	char order[] = "XYZ";
	for (float angle : { -1.1f, 0.35f, 1.5707964f }) {
		SCOPED_TRACE(angle);
		float angles[] = { angle, 0.0f, 0.0f };
		edF32MATRIX4 matrix{};
		edF32Matrix4FromEulerOrdSoft(&matrix, order, angles);
		const float sine = std::sin(angle), cosine = std::cos(angle);
		const float expected[] = {
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, cosine, sine, 0.0f,
			0.0f, -sine, cosine, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f,
		};
		// The PS2 table rounds angles to one of 8192 samples.
		for (int index = 0; index < 16; ++index) {
			EXPECT_NEAR(matrix.raw[index], expected[index], 5e-4f);
		}
	}
}
