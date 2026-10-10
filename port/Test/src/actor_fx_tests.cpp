#include <gtest/gtest.h>
#include <vector>
#include "MathOps.h"
#include "ActorFx.h"
#include "PathFollow.h"
#include "ActorManager.h"
#include "Animation.h"
#include "AnmManager.h"
#include "FxComposite.h"

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

#ifdef PLATFORM_WIN
	class TeardownActor : public CActor {
	public:
		CFxHandle fxHandle;
		int termCount = 0, destroyCount = 0;
		void Term() override {
			EXPECT_TRUE(fxHandle.IsValid());
			++termCount;
		}
		void Destroy() override {
			EXPECT_TRUE(fxHandle.IsValid());
			++destroyCount;
		}
	};
#endif
}

#ifdef PLATFORM_WIN
TEST(ActorFxTeardown, RetainsAnimationUntilCompositeUnregistersBone)
{
	CActorManager actors;
	// No class arrays are owned by this fixture; the test actor lives on the stack.
	for (auto& classInfo : actors.aClassInfo) classInfo = {};
	TeardownActor actor;
	actors.nbActors = 1;
	actors.aActors = new CActor*[1]{ &actor };
	actors.aAnimation = new CAnimation[1];
	actor.pAnimationController = actors.aAnimation;
	BoneData bone{};
	bone.boneId = 42;
	bone.usedByCount = 1;
	actors.aAnimation[0].pBoneData = &bone;

	CFxManager effects;
	auto* previousEffects = CScene::ptable.g_EffectsManager_004516b8;
	CScene::ptable.g_EffectsManager_004516b8 = &effects;
	auto* pool = new CFxCompositeManager;
	effects.aEffectCategory[FX_TYPE_COMPOSITE] = pool;
	pool->nbPool = 1;
	pool->aFx = new CFxNewComposite[1];
	pool->aNodes = new CDoubleLinkedNode<CFxNewComposite*>[1];
	auto& fx = pool->aFx[0];
	fx.nbComponentParticles = 0;
	fx.id = 2;
	fx.flags = 0x100;
	fx.pActor = &actor;
	fx.boneId = bone.boneId;
	pool->aNodes[0].node = &fx;
	pool->activeList.InsertFront(&pool->aNodes[0]);
	actor.fxHandle = CFxHandle(fx.id, &fx);

	actors.Level_TermActors();
	EXPECT_EQ(actor.termCount, 1);
	EXPECT_EQ(actor.destroyCount, 1);
	EXPECT_EQ(actors.aActors[0], &actor);
	EXPECT_EQ(actors.aAnimation, actor.pAnimationController);
	EXPECT_EQ(bone.usedByCount, 1);

	// Exercise the reported ASan path through pool Term, composite Kill and
	// CNewFx::Kill, while the animation allocation is still alive.
	effects.Level_Term();
	EXPECT_EQ(bone.usedByCount, 0);
	EXPECT_EQ(actors.aAnimation[0].pBoneData, nullptr);
	actors.Level_FreeActors();
	EXPECT_EQ(actor.termCount, 1);
	EXPECT_EQ(actor.destroyCount, 1);
	EXPECT_EQ(actors.aActors, nullptr);
	EXPECT_EQ(actors.aAnimation, nullptr);
	CScene::ptable.g_EffectsManager_004516b8 = previousEffects;
	delete[] actors.aLinkedActorData;
}
#endif

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
	path.field_0x28 = scales;
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
