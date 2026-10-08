#include <gtest/gtest.h>

#include "CameraFightData.h"

extern CamUpdate CamUpdate_0049c760;

TEST(CameraFightData, UniformCollisionSamplesFormOneRun)
{
	for (float distance : {0.0f, 2.0f}) {
		CamUpdate update = CamUpdate_0049c760;
		update.field_0x140 = 0; // Keep the supplied samples instead of casting rays.
		update.field_0x18 = 1.0f;
		for (auto& sample : update.aSubObjB) {
			sample.intersectionDistance = distance;
			sample.field_0x4 = 0.0f;
		}

		edF32VECTOR4 position = {};
		update.FUN_003c4a40(&position);

		ASSERT_EQ(update.field_0x20, 1u);
		EXPECT_EQ(update.aSubObjA[0].field_0x0, distance < 1.0f);
		EXPECT_EQ(update.aSubObjA[0].field_0x2, 0);
		EXPECT_EQ(update.aSubObjA[0].field_0x4, 0);
		EXPECT_EQ(update.aSubObjA[0].field_0x6, 35);
	}
}

TEST(CameraFightData, CollisionChangeAtLastSampleFormsFinalRun)
{
	CamUpdate update = CamUpdate_0049c760;
	update.field_0x140 = 0;
	update.field_0x18 = 1.0f;
	for (auto& sample : update.aSubObjB) {
		sample.intersectionDistance = 2.0f;
		sample.field_0x4 = 0.0f;
	}
	update.aSubObjB[34].intersectionDistance = 0.0f;

	edF32VECTOR4 position = {};
	update.FUN_003c4a40(&position);

	ASSERT_EQ(update.field_0x20, 2u);
	EXPECT_EQ(update.aSubObjA[0].field_0x0, 0);
	EXPECT_EQ(update.aSubObjA[0].field_0x4, 0);
	EXPECT_EQ(update.aSubObjA[0].field_0x6, 34);
	EXPECT_EQ(update.aSubObjA[1].field_0x0, 1);
	EXPECT_EQ(update.aSubObjA[1].field_0x4, 34);
	EXPECT_EQ(update.aSubObjA[1].field_0x6, 35);
}
