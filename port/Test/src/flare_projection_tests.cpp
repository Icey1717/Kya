#include <gtest/gtest.h>

#include "FlareProjection.h"
#include "port/NativeProjection.h"
#include <limits>

using Renderer::Native::FlareScreenData;
using Renderer::Native::ProjectFlare;

TEST(FlareProjection, MapsGsScreenCoordinatesAndMarkerIndependently)
{
	FlareScreenData data;
	Renderer::Native::GsProjection projection;
	projection.depth = { 1000.0f, 0.0f, 0.0f, 1.0f };
	ASSERT_TRUE(ProjectFlare(2048.0f, 2048.0f, 1800.0f, 2296.0f, 120.0f,
		512.0f, 448.0f, 500.0f, projection, data));
	EXPECT_FLOAT_EQ(data.center[0], 0.5f);
	EXPECT_FLOAT_EQ(data.center[1], 0.5f);
	EXPECT_FLOAT_EQ(data.halfSize[0], 120.0f / 512.0f);
	EXPECT_FLOAT_EQ(data.halfSize[1], 120.0f / 448.0f);
	EXPECT_FLOAT_EQ(data.marker[0], 8.0f / 512.0f);
	EXPECT_FLOAT_EQ(data.marker[1], 472.0f / 448.0f);
	EXPECT_FLOAT_EQ(data.marker[2], 8.0f / 512.0f);
	EXPECT_FLOAT_EQ(data.marker[3], 8.0f / 448.0f);
}

TEST(FlareProjection, DepthConversionMatchesNativePerspectiveAtMultipleDistances)
{
	const float nearClip = -0.1f;
	const float farClip = -5000.0f;
	const float gsNear = 16777215.0f;
	const float gsFar = 1.0f;
	const auto projection = BuildNativeProjection(1.0f, 1.0f, 1.0f, nearClip, farClip);
	// GS depth has the same perspective W, with different near/far endpoints.
	const float a = (-gsFar * farClip + gsNear * nearClip) / (farClip - nearClip);
	const float b = -gsNear * nearClip - a * nearClip;
	auto gs = projection;
	gs.cc = a;
	gs.dc = b;
	const auto conversion = Renderer::Native::BuildGsProjection(projection.raw, gs.raw);
	for (float z : { nearClip, -1.0f, -100.0f, farClip }) {
		const float gsDepth = (a * z + b) / -z;
		FlareScreenData data;
		ASSERT_TRUE(ProjectFlare(2048, 2048, 2048, 2048, 32, 512, 512, gsDepth, conversion, data));
		const float nativeDepth = (projection.cc * z + projection.dc) / (projection.cd * z + projection.dd);
		EXPECT_NEAR(data.depth, nativeDepth, 1e-6f);
	}
}

TEST(FlareProjection, RejectsInvalidInputsAndOutOfRangeDepth)
{
	FlareScreenData data;
	Renderer::Native::GsProjection projection;
	projection.depth = { 1000.0f, 0.0f, 0.0f, 1.0f };
	EXPECT_FALSE(ProjectFlare(2048, 2048, 2048, 2048, 32, 0, 512, 500, projection, data));
	EXPECT_FALSE(ProjectFlare(2048, 2048, 2048, 2048, 0, 512, 512, 500, projection, data));
	EXPECT_FALSE(ProjectFlare(2048, 2048, 2048, 2048, 32, 512, 512, -1, projection, data));
	EXPECT_FALSE(ProjectFlare(2048, 2048, 2048, 2048, 32, 512, 512, 1001, projection, data));
	EXPECT_FALSE(ProjectFlare(std::numeric_limits<float>::infinity(), 2048, 2048, 2048, 32, 512, 512, 500, projection, data));
	projection.depth = { 0.0f, 1.0f, 0.0f, 1.0f }; // Noninvertible GS depth.
	EXPECT_FALSE(ProjectFlare(2048, 2048, 2048, 2048, 32, 512, 512, 500, projection, data));
}
