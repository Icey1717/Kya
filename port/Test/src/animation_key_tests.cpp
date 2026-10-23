#include <gtest/gtest.h>
#include "Animation.h"
#include <array>

namespace
{
	struct PackedVectorTrack
	{
		ushort count = 1;
		ushort flags = 4;
		float duration = 1.0f;
		float minimum;
		float range;
		ushort xyz[3];
		ushort padding = 0;
	};
	static_assert(sizeof(PackedVectorTrack) == 24);
}

// edAnmTransformCtrl::GetValue, PS2 0x0023e930; single-key vector reads at
// 0x0023f440 (translation) and 0x0023fc80 (scale) skip the range header.
TEST(AnimationKeys, SingleQuantizedVectorsDecodePayloadAfterRangeHeader)
{
	for (ushort channels : {2, 4, 6}) {
		for (ushort timeFormat : {0, 1}) {
			SCOPED_TRACE(channels);
			SCOPED_TRACE(timeFormat);
			struct {
				int header[3] = {1, 0, 0};
				ushort channels;
				ushort keyCount = 0;
				int keyOffset = 60;
				uint nodeId = 0x12345678;
				PackedVectorTrack tracks[2];
			} bank{};
			bank.channels = channels;
			PackedVectorTrack translation{1, static_cast<ushort>(4 | timeFormat), 1.0f,
				-12.5f, 25.0f, {0, 65535, 32768}};
			PackedVectorTrack scale{1, static_cast<ushort>(4 | timeFormat), 1.0f,
				0.5f, 1.5f, {65535, 0, 32768}};
			bank.tracks[0] = channels == 4 ? scale : translation;
			if (channels == 6) bank.tracks[1] = scale;

			// One-bone skeleton: tag, parent index, sorted bone ID, bone index.
			std::array<uint, 4> skeleton = {1, 0xffffffff, bank.nodeId, 0};
			edF32MATRIX3 constant{};
			constant.ad = constant.bd = constant.ca = constant.cb = constant.cc = constant.cd = 1.0f;
			edANM_WRTS destination{};
			edAnmStage stage;
			stage.anmSkeleton.pTag = reinterpret_cast<edANM_SKELETON*>(skeleton.data());
			stage.pConstantMatrixData = &constant;
			stage.pRelativeTransformMatrixBuffer = &destination;
			stage.pKeyData = reinterpret_cast<edANM_HDR*>(&bank);
			stage.field_0x24 = 0;
			for (float time : {0.0f, 0.5f, 1.0f}) {
				stage.animTime = time;
				stage.AnimToWRTS();
				const auto& result = destination.matrices[0];
				EXPECT_FLOAT_EQ(result.aa, 0.0f);
				EXPECT_FLOAT_EQ(result.ad, 1.0f);
				EXPECT_FLOAT_EQ(result.ba, (channels & 2) ? -12.5f : 0.0f);
				EXPECT_FLOAT_EQ(result.bb, (channels & 2) ? 12.5f : 0.0f);
				EXPECT_NEAR(result.bc, (channels & 2) ? -12.5f + 25.0f * (32768.0f / 65535.0f) : 0.0f, 1e-6f);
				EXPECT_FLOAT_EQ(result.bd, 1.0f);
				EXPECT_FLOAT_EQ(result.ca, (channels & 4) ? 2.0f : 1.0f);
				EXPECT_FLOAT_EQ(result.cb, (channels & 4) ? 0.5f : 1.0f);
				EXPECT_NEAR(result.cc, (channels & 4) ? 0.5f + 1.5f * (32768.0f / 65535.0f) : 1.0f, 1e-6f);
				EXPECT_FLOAT_EQ(result.cd, 1.0f);
			}
		}
	}
}
