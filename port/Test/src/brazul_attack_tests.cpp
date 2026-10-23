#include <gtest/gtest.h>
#include "ActorBrazul.h"
#include "ActorManager.h"
#include "CollisionRay.h"
#include "CollisionManager.h"
#include "MathOps.h"
#include "TimeController.h"

namespace
{
	class BrazulHitReceiver : public CActor
	{
	public:
		int ReceiveMessage(CActor*, ACTOR_MESSAGE msg, MSG_PARAM param) override
		{
			if (msg == MESSAGE_KICKED) {
				++hitCount;
				damage = static_cast<_msg_hit_param*>(param)->damage;
			}
			return 1;
		}

		int hitCount = 0;
		float damage = 0.0f;
	};

	class BrazulAttackTest : public testing::Test
	{
	protected:
		void SetUp() override
		{
			previousManager = CScene::ptable.g_ActorManager_004516a4;
			CScene::ptable.g_ActorManager_004516a4 = &manager;
			previousDelta = GetTimer()->cutsceneDeltaTime;
			GetTimer()->cutsceneDeltaTime = 0.05f;
			ed_Bound_Box bounds{ { 100.0f, 0.0f, 100.0f, 1.0f }, { 320.0f, 80.0f, 320.0f, 1.0f } };
			manager.cluster.Init(1, &bounds, 1);
			target.actorState = static_cast<EActorState>(-1);
			target.flags = 0;
			actor.currentLocation = { 100.0f, 10.0f, 100.0f, 1.0f };
			actor.rotationQuat = gF32Vector4UnitZ;
			actor.flags = 0;
			actor.timeInAir = 0.0f;
			actor.hitMultiplier = 1.0f;
			actor.actorsExcludeTable.nbEntries = 0;
			blow.damage = 12.0f;
			actor.pBlow = &blow;
			mesh.base.transformA = gF32Matrix4Unit;
			mesh.base.transformA.rowT = actor.currentLocation;
			actor.field_0x31a0 = mesh.base.transformA;
			actor.field_0x3100.pMeshTransformData = &mesh;
			actor.field_0x3100.field_0x60 = 1;
			actor.field_0x3100.field_0x61 = 0;
			actor.field_0x3100.field_0x64 = 0;
			actor.field_0x3100.field_0x68 = 0.0f;
		}

		void TearDown() override
		{
			manager.cluster.Term();
			GetTimer()->cutsceneDeltaTime = previousDelta;
			CScene::ptable.g_ActorManager_004516a4 = previousManager;
		}

		void PlaceTarget(float distance, float height = 0.0f)
		{
			target.currentLocation = actor.currentLocation;
			target.currentLocation.x += distance;
			target.currentLocation.y += height;
			target.sphereCentre = target.currentLocation;
			target.sphereCentre.w = 0.5f;
			manager.cluster.NewNode(&target);
		}

		void UpdateRing()
		{
			actor.field_0x3180 = 1.0f;
			actor.field_0x3184 = 2.5f;
			actor.field_0x3188 = 5.0f;
			actor.field_0x318c = 0.3f;
			actor.field_0x3190 = 1.0f;
			actor.FUN_003e60c0(); // Radius 2.75, inner radius 1.75.
		}

		CActorManager manager;
		CActorManager* previousManager;
		float previousDelta;
		CActorBrazul actor;
		BrazulHitReceiver target;
		s_fighter_blow blow{};
		ed_3d_hierarchy_node mesh{};
	};
}

TEST_F(BrazulAttackTest, RingHitsAtSavedWorldPosition)
{
	PlaceTarget(2.1f);
	UpdateRing();
	EXPECT_EQ(target.hitCount, 1);
	EXPECT_FLOAT_EQ(target.damage, 12.0f);
}

TEST_F(BrazulAttackTest, RingExcludesInteriorWithFractionalInnerRadius)
{
	PlaceTarget(1.6f);
	UpdateRing();
	EXPECT_EQ(target.hitCount, 0);
}

TEST_F(BrazulAttackTest, RingPassesBelowAirborneTarget)
{
	PlaceTarget(2.1f, 0.4f);
	UpdateRing();
	EXPECT_EQ(target.hitCount, 0);
}

TEST_F(BrazulAttackTest, ConeRayHitsPlayerCollisionBetweenFrames)
{
	PlaceTarget(2.1f);
	// Exercise the same actor query as FUN_003e59e0, crossing the surface of a
	// dynamic player sphere in one 60 Hz step at the cone's speed of 8 units/s.
	target.typeID = ACTOR_HERO_PRIVATE;
	edColPRIM_OBJECT sphere{};
	sphere.localToWorld = gF32Matrix4Unit;
	sphere.localToWorld.aa = sphere.localToWorld.bb = sphere.localToWorld.cc = 0.5f;
	sphere.localToWorld.rowT = target.currentLocation;
	sphere.worldTransform = gF32Matrix4Unit;
	sphere.worldTransform.aa = sphere.worldTransform.bb = sphere.worldTransform.cc = 2.0f;
	sphere.worldTransform.rowT = { -2.0f * target.currentLocation.x, -2.0f * target.currentLocation.y,
		-2.0f * target.currentLocation.z, 1.0f };
	edObbTREE_DYN tree{};
	tree.bbox.transform = gF32Matrix4Unit;
	tree.bbox.transform.rowT = target.currentLocation;
	tree.bbox.width = tree.bbox.height = tree.bbox.depth = 0.5f;
	tree.type = 0xe;
	tree.count_0x52 = 1;
	tree.field_0x54[0] = STORE_POINTER(&sphere);
	CCollision collision{};
	collision.flags_0x0 = 0x80000;
	collision.pObbTree = &tree;
	target.pCollisionData = &collision;
	edF32VECTOR4 origin = target.currentLocation;
	origin.x -= 0.55f;
	edF32VECTOR4 direction{ 1.0f, 0.0f, 0.0f, 0.0f };
	CCollisionRay ray(8.0f / 60.0f, &origin, &direction);
	_ray_info_out hit{};
	EXPECT_NEAR(ray.IntersectActors(&actor, nullptr, 0x7fffffff, nullptr, &hit), 0.05f, 0.0001f);
	EXPECT_EQ(hit.pActor_0x0, &target);
}
