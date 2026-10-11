#include <gtest/gtest.h>
#include <cmath>
#include "ActorWind.h"
#include "MathOps.h"

extern void ComputeDimensionsAndBase(edF32MATRIX4* pMatrix, edF32VECTOR4* pDimensions, edF32VECTOR4* pBase);

TEST(WindGeometry, DimensionsAndBaseMatchTransformedUnitPrimitive)
{
	edF32MATRIX4 matrix{};
	// Rotate the wind axis into world Z, with distinct dimensions and a translation.
	matrix.rowX = { 4.0f, 0.0f, 0.0f, 0.0f };
	matrix.rowY = { 0.0f, 0.0f, 20.0f, 0.0f };
	matrix.rowZ = { 0.0f, -6.0f, 0.0f, 0.0f };
	matrix.rowT = { 10.0f, 30.0f, 50.0f, 1.0f };
	edF32VECTOR4 dimensions{}, base{};
	ComputeDimensionsAndBase(&matrix, &dimensions, &base);
	EXPECT_FLOAT_EQ(dimensions.x, 4.0f);
	EXPECT_FLOAT_EQ(dimensions.y, 20.0f);
	EXPECT_FLOAT_EQ(dimensions.z, 6.0f);
	EXPECT_FLOAT_EQ(dimensions.w, 0.0f);
	EXPECT_FLOAT_EQ(base.x, 10.0f);
	EXPECT_FLOAT_EQ(base.y, 30.0f);
	EXPECT_FLOAT_EQ(base.z, 40.0f);
	EXPECT_FLOAT_EQ(base.w, 1.0f);
}

TEST(WindGeometry, OuterShellFormsTwoCircularHalvesWithinWindBounds)
{
	CFxWind wind{};
	wind.field_0x130 = { 4.0f, 20.0f, 4.0f, 0.0f };
	CFxWindSolid solid{};
	solid.pOwner = &wind;
	solid.field_0x4 = 3;
	_wind_fx_vtx vertices[21]{};
	for (int half = 0; half < 2; ++half) {
		solid._Create_SemiCylinder(vertices, 0, 0, half);
		for (const auto& vertex : vertices) {
			EXPECT_NEAR(vertex.field_0x0 * vertex.field_0x0 + vertex.field_0x8 * vertex.field_0x8, 16.0f, 1e-4f);
			EXPECT_GE(vertex.field_0x4, -10.0f);
			EXPECT_LE(vertex.field_0x4, 10.0f);
		}
		EXPECT_NEAR(vertices[0].field_0x8, 0.0f, 1e-5f);
		EXPECT_NEAR(vertices[3].field_0x0, 0.0f, 1e-5f);
		EXPECT_NEAR(vertices[3].field_0x8, half == 0 ? -4.0f : 4.0f, 1e-5f);
		EXPECT_NEAR(vertices[6].field_0x8, 0.0f, 1e-5f);
	}
}
