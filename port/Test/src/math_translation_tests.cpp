#include <gtest/gtest.h>
#include <cmath>
#include "ActorPatternService.h"
#include "edParticles/edParticles.h"
#include "MathOps.h"

void edPartGenNewPosAndSpeed(_ed_particle_generator_param* pGeneratorParam, edF32VECTOR4* pPosition, edF32VECTOR4* pSpeed);

TEST(MathTranslation, UnsignedFloatConversionsPreserveHighBitBoundary)
{
	// Exact equality matters: EXPECT_FLOAT_EQ tolerates the one-ULP errors here.
	EXPECT_EQ(DecodeFloat(0u), 0.0f);
	EXPECT_EQ(DecodeFloat(0x80000000u), 2147483648.0f);
	EXPECT_EQ(DecodeFloat(0x80000100u), 2147483904.0f);
	EXPECT_EQ(DecodeFloat(0xc0000081u), 3221225728.0f);
	EXPECT_EQ(DecodeFloat(0xffffffffu), 4294967296.0f);
	EXPECT_EQ(EncodeFloat(0.0f), 0u);
	EXPECT_EQ(EncodeFloat(2147483648.0f), 0x80000000u);
	EXPECT_EQ(EncodeFloat(2147483904.0f), 0x80000100u);
	EXPECT_EQ(EncodeFloat(4294967040.0f), 0xffffff00u);
}

TEST(MathTranslation, TrailRenderingRetainsFractionalLifeAndFade)
{
	CPointPattern points[2]{};
	CPatternPart pattern;
	pattern.field_0x50 = 2;
	pattern.nbPointPatterns = 2;
	pattern.aPointPatterns = points;
	pattern.field_0x48 = 1.0f;
	pattern.field_0x28 = 0;

	points[0].field_0x20 = 0.2f;
	EXPECT_NEAR(pattern.FUN_003a6320(0, 1), 0.6f, 0.00001f);
	points[0].field_0x20 = 0.75f;
	EXPECT_FLOAT_EQ(pattern.FUN_003a6320(0, 0), 0.75f);
	points[0].field_0x20 = 0.0f;
	EXPECT_FLOAT_EQ(pattern.FUN_003a6320(0, 1), 0.0f);
}

TEST(MathTranslation, ParticlePerimeterPositionsRemainWithinShapeAndKeepRandomSequence)
{
	for (uint seed : { 0u, 1u, 12345u, 0x7fffffffu, 0xffffffffu }) {
		SCOPED_TRACE(seed);
		_ed_particle_generator_param generator{};
		generator.shape = 7;
		generator.field_0xf = 0x12; // Perimeter positions, fixed upward direction.
		generator.field_0x40 = gF32Matrix4Unit;
		generator.field_0x21c = seed;
		edF32VECTOR4 position{};
		edF32VECTOR4 speed{};
		edPartGenNewPosAndSpeed(&generator, &position, &speed);
		EXPECT_TRUE(std::isfinite(position.x));
		EXPECT_TRUE(std::isfinite(position.z));
		EXPECT_LE(std::fabs(position.x), 0.5f);
		EXPECT_LE(std::fabs(position.z), 0.5f);
		EXPECT_FLOAT_EQ(std::max(std::fabs(position.x), std::fabs(position.z)), 0.5f);
		EXPECT_FLOAT_EQ(position.y, 0.0f);
		EXPECT_FLOAT_EQ(position.w, 1.0f);
		uint expectedSeed = seed >> 1;
		for (int i = 0; i < 2; ++i) {
			expectedSeed = expectedSeed * 0x19660d + 0x3c6ef35f;
		}
		EXPECT_EQ(generator.field_0x21c, expectedSeed);
	}
}
