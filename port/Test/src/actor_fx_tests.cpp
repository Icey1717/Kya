#include <gtest/gtest.h>
#include <vector>
#include "MathOps.h"
#include "ActorFx.h"
#include "PathFollow.h"
#include "ActorManager.h"
#include "Animation.h"
#include "AnmManager.h"
#include "FxComposite.h"
#include "ActorBrazul.h"
#include "DlistManager.h"
#include "FxLightEmitter.h"
#include "TimeController.h"

TEST(BoomyTrail, CreationRestoresDefaultOpacityForBothControlledFlightEmitters)
{
	// PS2 CFxLightEmitter::Create (003a9a70) initializes the same factor
	// that ChangeAlphaFactor (003a97e0) controls and Draw (003a80c0) reads.
	for (uint emitterFlags : { 0x10u, 0x18u }) {
		CFxLightEmitter emitter;
		for (float previousAlpha : { 0.0f, 0.25f }) {
			emitter.alphaFactor = previousAlpha;
			emitter.Create(0.4f, 0x80, emitterFlags);
			EXPECT_FLOAT_EQ(emitter.alphaFactor, 1.0f);
			emitter.ChangeAlphaFactor(0.5f);
			EXPECT_FLOAT_EQ(emitter.alphaFactor, 0.5f);
			delete[] emitter.pRayDef;
			emitter.pRayDef = nullptr;
		}
	}
}

TEST(BoomyTrail, AutomaticEmissionStaggersParticlesAndRemainsVisibleAcrossLifetimes)
{
	const float previousFrameTime = GetTimer()->frameTime;
	const float previousDelta = GetTimer()->cutsceneDeltaTime;
	GetTimer()->frameTime = 1.0f / 60.0f;
	GetTimer()->cutsceneDeltaTime = 1.0f / 60.0f;
	CFxLightEmitter emitter;
	emitter.Create(0.4f, 0x80, 0x10);
	emitter.ChangeStallTime(emitter.field_0x60);
	emitter.ChangeRadius(0.1f, 0.2f);
	emitter.ChangeLengthAndWidth(1.0f, 0.2f, 0.2f, 0.2f);
	emitter.ChangeSpeedAndAccel(3.0f, 4.0f, 5.0f, -8.0f);
	emitter.field_0x40 = 0.0f;
	emitter.field_0x44 = 0.0f;
	edF32VECTOR4 position = { 0.0f, 0.0f, 0.0f, 1.0f };
	emitter.Manage(&position, 0xffffffff);
	int earliest = emitter.stallTime;
	int latest = -1;
	for (uint i = 0; i < emitter.nbRayDefs; ++i) {
		const int delay = emitter.pRayDef[i].field_0x2c;
		EXPECT_GE(delay, 0);
		EXPECT_LT(delay, emitter.stallTime);
		earliest = std::min(earliest, delay);
		latest = std::max(latest, delay);
	}
	EXPECT_LT(earliest, latest);
	for (int frame = 0; frame < 240; ++frame) {
		emitter.Manage(&position, 0xffffffff);
		if (frame < 30) continue; // Let the initial delayed particles enter the trail.
		int visibleParticles = 0;
		for (uint i = 0; i < emitter.nbRayDefs; ++i) {
			const auto& ray = emitter.pRayDef[i];
			if ((ray.field_0x30 & 1) && ray.field_0x28 >= 0.1f && ray.field_0x28 <= 0.3f) ++visibleParticles;
		}
		EXPECT_GT(visibleParticles, 0) << "frame " << frame;
	}
	delete[] emitter.pRayDef;
	emitter.pRayDef = nullptr;
	GetTimer()->frameTime = previousFrameTime;
	GetTimer()->cutsceneDeltaTime = previousDelta;
}

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

TEST(BrazulSpark, InitializesAllEightSparksWithoutOverwritingEffectHandles)
{
	CActorBrazul actor;
	actor.sectorId = 0;
	CGlobalDListPatch patch{ 0 };
	GlobalDlistEntry entry{};
	entry.pDlistPatch = &patch;
	CGlobalDListManager manager;
	manager.ppGlobalDlist = &entry;
	auto* previousManager = CScene::ptable.g_GlobalDListManager_004516bc;
	CScene::ptable.g_GlobalDListManager_004516bc = &manager;

	for (auto& spark : actor.field_0x1070) {
		spark.Create(13, 42, 43);
		EXPECT_EQ(spark.field_0x40.field_0x90, 0x00a6a500u);
		EXPECT_EQ(spark.field_0x40.field_0x94, 0x00a6a500u);
		spark.Init(&actor);
		EXPECT_EQ(spark.pOwner, &actor);
		EXPECT_EQ(spark.field_0x3d0.type, 42u);
		EXPECT_EQ(spark.field_0x3dc.type, 43u);
		for (int i = 0; i < 12; ++i) {
			EXPECT_FLOAT_EQ(spark.field_0x40.pFloat_0x10[i], 0.025f);
		}
		for (int i = 0; i < 3; ++i) {
			const auto& first = spark.field_0x40.field_0xe4[i * 12];
			const auto& last = spark.field_0x40.field_0xe4[i * 12 + 11];
			EXPECT_FLOAT_EQ(first.x, 0.0f);
			EXPECT_FLOAT_EQ(first.z, 0.0f);
			EXPECT_FLOAT_EQ(first.w, 1.0f);
			EXPECT_FLOAT_EQ(last.x, 0.0f);
			EXPECT_FLOAT_EQ(last.z, 1.0f);
			EXPECT_FLOAT_EQ(last.w, 1.0f);
		}
	}
	EXPECT_EQ(patch.nbTotalPatches, 8);
	EXPECT_EQ(entry.nbMatrices, 8 * 3 * 12 * 2);
	CScene::ptable.g_GlobalDListManager_004516bc = previousManager;
}

TEST(BrazulSpark, DrawPreservesRgbAndFadesAlphaDuringReset)
{
	astruct_19 spark{};
	// Exercise the actor's draw states without requiring renderer packets.
	spark.Create(-1, -1, -1);
	spark.field_0x4 = 2;
	spark.Draw();
	EXPECT_EQ(spark.field_0x40.field_0x90, 0x7fa6a500u);
	EXPECT_EQ(spark.field_0x40.field_0x94, 0x7fa6a500u);

	spark.Reset(2.0f);
	spark.Draw();
	EXPECT_EQ(spark.field_0x40.field_0x90, 0x7fa6a500u);
	spark.field_0x34 = 1.0f;
	spark.Draw();
	EXPECT_EQ(spark.field_0x40.field_0x90, 0x3fa6a500u);
	EXPECT_EQ(spark.field_0x40.field_0x94, 0x3fa6a500u);
	for (float remaining : { 0.0f, -1.0f }) {
		spark.field_0x34 = remaining;
		spark.Draw();
		EXPECT_EQ(spark.field_0x40.field_0x90, 0x00a6a500u);
		EXPECT_EQ(spark.field_0x40.field_0x94, 0x00a6a500u);
	}
	for (int state : { 0, 1, 4 }) {
		spark.field_0x4 = state;
		spark.Draw();
		EXPECT_EQ(spark.field_0x40.field_0x90, 0x00a6a500u);
		EXPECT_EQ(spark.field_0x40.field_0x94, 0x00a6a500u);
	}
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
