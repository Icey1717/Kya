#include <gtest/gtest.h>
#include <vector>
#include "ActorFx.h"
#include "PathFollow.h"

namespace {
	struct FxDrawSample {
		edF32VECTOR4 position;
		edF32VECTOR4 rotation;
		edF32VECTOR4 scale;
		bool hidden;
	};

	class RecordingFx : public CNewFx {
	public:
		void Draw() override {
			samples.push_back({ position, rotationEuler, scale, (flags & FX_FLAG_HIDDEN) != 0 });
		}
		int GetType() override { return FX_TYPE_PARTICLE; }
		std::vector<FxDrawSample> samples;
	};
}

TEST(ActorFxPath, DrawsAtWorldSpaceKeysAndHidesFromGlobalPass)
{
	RecordingFx fx;
	CPathFollow path;
	edF32VECTOR4 positions[] = { { 300, 2, -40, 1 }, { 310, 5, -20, 1 } };
	edF32VECTOR4 rotations[] = { { 0, 1, 0, 0 }, { 0, 2, 0, 0 } };
	edF32VECTOR4 scales[] = { { 2, 3, 4, 1 }, { 5, 6, 7, 1 } };
	path.splinePointCount = 2;
	path.aSplinePoints = positions;
	path.aSplineRotationsEuler = rotations;
	path.field_0x28 = reinterpret_cast<char*>(scales);
	CActorFx::CBhvPath behaviour;
	behaviour.field_0xc = 1;
	behaviour.field_0x14 = 0;
	behaviour.pPathFollow = &path;
	behaviour.fxHandle = CFxHandle(fx.id, &fx);
	fx.Hide();
	behaviour.Draw();
	ASSERT_EQ(fx.samples.size(), 2u);
	for (int i = 0; i < 2; ++i) {
		EXPECT_FLOAT_EQ(fx.samples[i].position.x, positions[i].x);
		EXPECT_FLOAT_EQ(fx.samples[i].position.z, positions[i].z);
		EXPECT_FLOAT_EQ(fx.samples[i].rotation.y, rotations[i].y);
		EXPECT_FLOAT_EQ(fx.samples[i].scale.y, scales[i].y);
		EXPECT_FALSE(fx.samples[i].hidden);
	}
	EXPECT_NE(fx.flags & FX_FLAG_HIDDEN, 0u);

	// Missing optional key data uses the PS2 defaults, not the previous key transform.
	path.aSplineRotationsEuler = nullptr;
	path.field_0x28 = nullptr;
	behaviour.Draw();
	ASSERT_EQ(fx.samples.size(), 4u);
	EXPECT_FLOAT_EQ(fx.samples[2].rotation.y, 0.0f);
	EXPECT_FLOAT_EQ(fx.samples[2].scale.y, 1.0f);

	behaviour.field_0xc = 0;
	behaviour.Draw();
	EXPECT_EQ(fx.samples.size(), 4u);
	behaviour.field_0xc = 1;
	behaviour.fxHandle.id = fx.id + 1;
	behaviour.Draw();
	EXPECT_EQ(fx.samples.size(), 4u);
}
