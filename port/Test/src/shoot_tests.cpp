#include <gtest/gtest.h>

#include "ActorShoot.h"
#include "ActorProjectile.h"
#include "ActorHero_Private.h"
#include "ActorPatternService.h"
#include "AnmManager.h"
#include "MathOps.h"
#include "TimeController.h"
#include <cmath>

namespace
{
	class ProjectileHitReceiver : public CActor
	{
	public:
		int ReceiveMessage(CActor* pSender, ACTOR_MESSAGE msg, MSG_PARAM pMsgParam) override
		{
			if (msg == MESSAGE_GET_BONE_ID) {
				return 0;
			}
			lastMessage = msg;
			if (msg == MESSAGE_KICKED) {
				hit = *static_cast<_msg_hit_param*>(pMsgParam);
			}
			return 1;
		}

		int lastMessage = -1;
		_msg_hit_param hit{};
	};

	class ShootActor : public CActorShoot
	{
	public:
		void SetState(int newState, int animationType) override
		{
			this->actorState = static_cast<EActorState>(newState);
			lastAnimationType = animationType;
			return;
		}

		void ManageDyn(float param_1, uint flags, CActorsTable* pActorsTable) override
		{
			dynamicFlags = flags;
			return;
		}

		int lastAnimationType = -2;
		uint dynamicFlags = 0;
	};

	class ShootTest : public testing::Test
	{
	protected:
		void SetUp() override
		{
			previousDelta = GetTimer()->cutsceneDeltaTime;
			GetTimer()->cutsceneDeltaTime = 0.125f;
			actor.addOnGenerator = {};
			actor.field_0x438 = nullptr;
			actor.field_0x3c8 = 0.25f;
			actor.field_0x3cc = 0.5f;
			actor.field_0x3d0 = 3;
			actor.field_0x3d4 = 1.0f;
			actor.field_0x3f4 = 0.0f;
			actor.field_0x3f8 = 0.0f;
			actor.field_0x3fc = 0.0f;
			actor.field_0x400 = 0;
			actor.field_0x3ec = 0;
			actor.currentAnimType = 0x11;
			actor.actorState = static_cast<EActorState>(0xc);
			actor.pAnimationController = &animation;
			animation.anmBinMetaAnimator.aAnimData = &layer;
			animation.currentAnimType = 0x11;
			actor.behaviourShootFire.fireshot.pActorStreamRef = nullptr;
			actor.behaviourShootFire.fireshot.field_0x298 = 0;
			actor.behaviourShootFireWave.conicalWaveShoot.Reset();
			return;
		}

		void TearDown() override
		{
			actor.staticMeshComponent.pMeshTransformParent = nullptr;
			actor.staticMeshComponent.pMeshTransformData = nullptr;
			GetTimer()->cutsceneDeltaTime = previousDelta;
			return;
		}

		void Manage(bool wave)
		{
			if (wave) {
				actor.BehaviourFireWave_Manage(&actor.behaviourShootFireWave);
			}
			else {
				actor.BehaviourShootFire_Manage(&actor.behaviourShootFire);
			}
			return;
		}

		float previousDelta;
		ShootActor actor;
		CAnimation animation;
		edAnmLayer layer{};
		ed_3d_hierarchy_node mesh{};
	};
}

TEST(ProjectileHit, StraightImpactLiftsTheActor)
{
	CActorProjectile projectile;
	ProjectileSubObj subObj{};
	ProjectileHitReceiver receiver;
	projectile.aProjectileSubObjs = &subObj;
	projectile.currentLocation = edF32VECTOR4{ 1.0f, 2.0f, 3.0f, 1.0f };
	subObj.damage = 1.0f;
	subObj.field_0x20 = 120.0f;
	subObj.field_0x24 = 60.0f;
	receiver.flags = 0;
	receiver.pCollisionData = nullptr;
	receiver.currentLocation = edF32VECTOR4{ 2.0f, 2.0f, 3.0f, 1.0f };
	edF32VECTOR4 sphere = projectile.currentLocation;
	sphere.w = 3.402823e+38f;
	projectile.HitActor(&sphere, &receiver, 0, 0);
	ASSERT_EQ(receiver.lastMessage, MESSAGE_KICKED);
	EXPECT_EQ(receiver.hit.projectileType, 10);
	EXPECT_FLOAT_EQ(receiver.hit.field_0x30, 120.0f);
	EXPECT_NEAR(receiver.hit.field_0x20.x, 0.0f, 0.0001f);
	EXPECT_FLOAT_EQ(receiver.hit.field_0x20.y, 1.0f);
	EXPECT_FLOAT_EQ(receiver.hit.field_0x20.z, 0.0f);

	CActorHeroPrivate hero;
	hero.field_0x634 = &projectile;
	hero.currentLocation = receiver.currentLocation;
	hero.rotationEuler = edF32VECTOR4{ 0.0f, 0.0f, 0.0f, 0.0f };
	hero.dynamicExt.gravityScale = 1.0f;
	hero.fighterAnatomyZones.field_0x10 = edF32VECTOR4{ 0.0f, 1.0f, 0.0f, 1.0f };
	hero._UpdateDynForExplosion(&receiver.hit);
	CBehaviourFighterProjected projected;
	projected.pOwner = &hero;
	projected._ComputeDynamics();
	EXPECT_NEAR(hero.field_0x740.x, 0.0f, 0.0001f);
	EXPECT_FLOAT_EQ(hero.field_0x740.y, 2.0f);
	EXPECT_FLOAT_EQ(hero.field_0x740.z, 0.0f);
	EXPECT_TRUE(std::isfinite(hero.field_0x7b4));

	edF32MATRIX4 rotation;
	edF32Matrix4FromAngAxisSoft(-hero.field_0x7b4 * 0.016f, &rotation, &hero.field_0x7a0);
	edF32Matrix4MulF32Matrix4Hard(&rotation, &hero.field_0x760, &rotation);
	hero._SV_DYN_SetRotationAroundMassCenter(&rotation);
	EXPECT_TRUE(std::isfinite(hero.currentLocation.x));
	EXPECT_TRUE(std::isfinite(hero.currentLocation.y));
	EXPECT_TRUE(std::isfinite(hero.currentLocation.z));
}

TEST(ProjectileHit, RadiusControlsTheHorizontalAndVerticalImpactComponents)
{
	CActorProjectile projectile;
	ProjectileSubObj subObj{};
	ProjectileHitReceiver receiver;
	projectile.aProjectileSubObjs = &subObj;
	projectile.currentLocation = edF32VECTOR4{ 0.0f, 0.0f, 0.0f, 1.0f };
	receiver.flags = 0;
	receiver.pCollisionData = nullptr;
	receiver.currentLocation = edF32VECTOR4{ 3.0f, 0.0f, 0.0f, 1.0f };
	edF32VECTOR4 sphere = { 0.0f, 0.0f, 0.0f, 5.0f };
	projectile.HitActor(&sphere, &receiver, 1, 0);
	ASSERT_EQ(receiver.lastMessage, MESSAGE_KICKED);
	EXPECT_EQ(receiver.hit.projectileType, 0);
	EXPECT_NEAR(receiver.hit.field_0x20.x, 0.6f, 0.0001f);
	EXPECT_NEAR(receiver.hit.field_0x20.y, 0.8f, 0.0001f);
	EXPECT_FLOAT_EQ(receiver.hit.field_0x20.z, 0.0f);
}

TEST(ProjectileHit, NormalHeroHitCreatesFiniteKnockback)
{
	CActorProjectile projectile;
	ProjectileSubObj subObj{};
	ProjectileHitReceiver receiver;
	projectile.aProjectileSubObjs = &subObj;
	projectile.currentLocation = edF32VECTOR4{ 1.0f, 0.0f, 0.0f, 1.0f };
	subObj.damage = 1.0f;
	subObj.field_0x20 = 120.0f;
	subObj.field_0x24 = 60.0f;
	receiver.flags = 0;
	receiver.pCollisionData = nullptr;
	receiver.currentLocation = edF32VECTOR4{ 0.0f, 0.0f, 0.0f, 1.0f };
	edF32VECTOR4 sphere = projectile.currentLocation;
	sphere.w = 3.402823e+38f;
	projectile.HitActor(&sphere, &receiver, 1, 0);
	ASSERT_EQ(receiver.lastMessage, MESSAGE_KICKED);
	ASSERT_EQ(receiver.hit.projectileType, 0);

	CActorHeroPrivate hero;
	hero.actorState = static_cast<EActorState>(STATE_HERO_STAND);
	hero.currentAnimType = -1;
	hero.currentLocation = receiver.currentLocation;
	hero.dynamicExt.aImpulseVelocities[0] = gF32Vector4Zero;
	const float previousDelta = GetTimer()->cutsceneDeltaTime;
	GetTimer()->cutsceneDeltaTime = 0.02f;
	const int state = hero.ChooseStateHit(&projectile, &receiver.hit, nullptr, 1);
	GetTimer()->cutsceneDeltaTime = previousDelta;
	EXPECT_EQ(state, STATE_HERO_HURT_A);
	EXPECT_FLOAT_EQ(hero.dynamicExt.field_0x6c, 0.0f);
	EXPECT_FLOAT_EQ(hero.dynamicExt.aImpulseVelocities[0].x, -150.0f);
	EXPECT_FLOAT_EQ(hero.dynamicExt.aImpulseVelocities[0].y, 300.0f);
	EXPECT_FLOAT_EQ(hero.dynamicExt.aImpulseVelocities[0].z, 0.0f);
}

TEST(PatternHit, UnspecifiedForceUsesTheDefaultHeroKnockback)
{
	CActor owner;
	ProjectileHitReceiver receiver;
	receiver.flags = 0;
	receiver.pCollisionData = nullptr;
	receiver.currentLocation = edF32VECTOR4{ 0.0f, 0.5f, 0.0f, 1.0f };
	CPointPattern points[3]{};
	for (int i = 0; i < 3; i++) {
		points[i].field_0x0 = edF32VECTOR4{ 2.0f * i, 0.0f, 0.0f, 1.0f };
		points[i].field_0x10 = gF32Vector4UnitY;
		points[i].field_0x20 = 1.0f;
		points[i].field_0x24 = 1;
	}
	CPatternPart pattern;
	pattern.field_0x0 = 1.0f;
	pattern.field_0x8 = 2.0f;
	pattern.field_0xc = 2.0f;
	pattern.field_0x10 = 2.0f;
	pattern.field_0x20 = 1.0f;
	pattern.field_0x40 = &receiver;
	pattern.field_0x48 = 1.0f;
	pattern.field_0x4c = 0;
	pattern.field_0x50 = 3;
	pattern.field_0x5c = -1;
	pattern.field_0x70 = 1.0f;
	pattern.pOwner = &owner;
	pattern.nbPointPatterns = 3;
	pattern.aPointPatterns = points;
	const float previousDelta = GetTimer()->cutsceneDeltaTime;
	GetTimer()->cutsceneDeltaTime = 0.02f;
	EXPECT_TRUE(pattern.UpdatePatternPartLife());
	GetTimer()->cutsceneDeltaTime = previousDelta;
	ASSERT_EQ(receiver.lastMessage, MESSAGE_KICKED);
	EXPECT_EQ(receiver.hit.projectileType, 1);
	EXPECT_FLOAT_EQ(receiver.hit.damage, 2.0f);
	EXPECT_EQ(receiver.hit.flags, 0);
	EXPECT_FLOAT_EQ(receiver.hit.field_0x30, 0.0f);

	CActorHeroPrivate hero;
	hero.actorState = static_cast<EActorState>(STATE_HERO_STAND);
	hero.currentAnimType = -1;
	hero.dynamicExt.aImpulseVelocities[0] = gF32Vector4Zero;
	GetTimer()->cutsceneDeltaTime = 0.02f;
	const int state = hero.ChooseStateHit(&owner, &receiver.hit, nullptr, 1);
	GetTimer()->cutsceneDeltaTime = previousDelta;
	EXPECT_EQ(state, STATE_HERO_HURT_A);
	EXPECT_FLOAT_EQ(hero.dynamicExt.aImpulseVelocities[0].x, 0.0f);
	EXPECT_FLOAT_EQ(hero.dynamicExt.aImpulseVelocities[0].y, 300.0f);
	EXPECT_FLOAT_EQ(std::fabs(hero.dynamicExt.aImpulseVelocities[0].z), 150.0f);
	EXPECT_TRUE(std::isfinite(edF32Vector4GetDistHard(&hero.dynamicExt.aImpulseVelocities[0])));
}

TEST_F(ShootTest, ShotDelayStartsAfterBothCooldownsAndUsesAStrictBoundary)
{
	actor.field_0x3f8 = 0.375f;
	actor.field_0x3fc = 0.75f;
	actor.ComputeTimeAndParamToShoot();
	EXPECT_FALSE(actor.field_0x43e);
	EXPECT_FALSE(actor.field_0x43d);
	EXPECT_FLOAT_EQ(actor.field_0x3f4, 0.0f);
	actor.ComputeTimeAndParamToShoot();
	EXPECT_TRUE(actor.field_0x43e);
	EXPECT_FALSE(actor.field_0x43d);
	EXPECT_FLOAT_EQ(actor.field_0x3f4, 0.125f);
	actor.ComputeTimeAndParamToShoot();
	EXPECT_FALSE(actor.field_0x43d);
	actor.ComputeTimeAndParamToShoot();
	EXPECT_TRUE(actor.field_0x43d);
	actor.currentAnimType = 0x10;
	actor.ComputeTimeAndParamToShoot();
	EXPECT_FLOAT_EQ(actor.field_0x3f4, 0.375f);
}

TEST_F(ShootTest, LastShotResetsTheBurstCooldownAfterPublishingReadiness)
{
	actor.field_0x3f8 = 0.5f;
	actor.field_0x3fc = 1.0f;
	actor.field_0x3f4 = 0.25f;
	actor.field_0x400 = 3;
	actor.ComputeTimeAndParamToShoot();
	EXPECT_TRUE(actor.field_0x43e);
	EXPECT_TRUE(actor.field_0x43d);
	EXPECT_EQ(actor.field_0x400, 0);
	EXPECT_FLOAT_EQ(actor.field_0x3fc, 0.0f);
	actor.ComputeTimeAndParamToShoot();
	EXPECT_FALSE(actor.field_0x43e);
	EXPECT_FALSE(actor.field_0x43d);
}

TEST_F(ShootTest, BothBehavioursWaitPastTheEmergenceBoundary)
{
	for (bool wave : { false, true }) {
		actor.actorState = static_cast<EActorState>(7);
		actor.currentAnimType = 0xc;
		actor.timeInAir = 0.4f;
		actor.staticMeshComponent.pMeshTransformParent = reinterpret_cast<edNODE*>(&mesh);
		actor.staticMeshComponent.pMeshTransformData = &mesh;
		actor.currentLocation = edF32VECTOR4{ 1.0f, 2.0f, 3.0f, 1.0f };
		Manage(wave);
		EXPECT_EQ(actor.actorState, 7);
		EXPECT_EQ(actor.dynamicFlags, 0x1002023b);
		EXPECT_FLOAT_EQ(mesh.base.transformA.rowT.y, 2.08f);
		actor.timeInAir = 0.401f;
		Manage(wave);
		EXPECT_EQ(actor.actorState, 8);
		EXPECT_EQ(actor.lastAnimationType, -1);
	}
}

TEST_F(ShootTest, HitAnimationRestoresTheCurrentStateOnlyWhenItsOwnLayerEnds)
{
	for (bool wave : { false, true }) {
		actor.actorState = static_cast<EActorState>(7);
		actor.currentAnimType = 0x13;
		animation.currentAnimType = 0x13;
		layer.currentAnimDesc.animType = 0x10;
		layer.animPlayState = 1;
		layer.field_0xcc = 2;
		actor.timeInAir = 0.0f;
		actor.lastAnimationType = -2;
		Manage(wave);
		EXPECT_EQ(actor.lastAnimationType, -2);
		layer.currentAnimDesc.animType = 0x13;
		Manage(wave);
		EXPECT_EQ(actor.actorState, 7);
		EXPECT_EQ(actor.lastAnimationType, -1);
	}
}
