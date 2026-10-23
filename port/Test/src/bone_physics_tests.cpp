#include <gtest/gtest.h>

#include "ActorWolfen.h"
#include "ActorBrazul.h"
#include "ActorPunchingBall.h"
#include <cmath>

TEST(ActorBonePhysics, RigTopologiesMatchThePS2Vtables)
{
	CPunchingBallBonePhysics punchingBall;
	punchingBall.SetObjCounts(3, 2);
	punchingBall.SetupObjects(nullptr);
	EXPECT_EQ(punchingBall.count_0x8, 3u);
	EXPECT_EQ(punchingBall.count_0xc, 2u);
	EXPECT_EQ(punchingBall.field_0x4[0]->pointA, 0u);
	EXPECT_EQ(punchingBall.field_0x4[0]->pointB, 1u);
	punchingBall.Term();

	CWolfenSharedBonePhysics sharedWolfen;
	sharedWolfen.SetObjCounts(4, 0);
	sharedWolfen.SetupObjects(nullptr);
	EXPECT_EQ(sharedWolfen.count_0xc, 3u);
	EXPECT_EQ(sharedWolfen.field_0x4[2]->pointA, 2u);
	EXPECT_EQ(sharedWolfen.field_0x4[2]->pointB, 3u);
	sharedWolfen.Term();

	CWolfenFullBodyPhysics fullBodyWolfen;
	fullBodyWolfen.SetObjCounts(26, 29);
	fullBodyWolfen.SetupObjects(nullptr);
	EXPECT_EQ(fullBodyWolfen.field_0x0[5]->boneHash, 0xcae60310u);
	EXPECT_EQ(fullBodyWolfen.field_0x4[28]->pointB, 24u);
	fullBodyWolfen.Term();

	CBrazulBonePhysics brazul;
	brazul.SetObjCounts(16, 21);
	brazul.SetupObjects(nullptr);
	EXPECT_EQ(brazul.count_0x8, 16u);
	EXPECT_EQ(brazul.field_0x4[20]->pointA, 15u);
	brazul.Term();
}

// CWolfenFullBodyPhysics::SetupObjects, PS2 0x003c1810.
TEST(ActorBonePhysics, KronosChainsKeepTheirRootsAndUnboundEndpoints)
{
	CWolfenFullBodyPhysics physics;
	physics.SetObjCounts(26, 29);
	physics.SetupObjects(nullptr);
	const uint roots[] = {0xcae60310, 0xc290efdb, 0xc28cefdb, 0xd18defe7, 0xd189efe7};
	const uint lengths[] = {5, 4, 4, 4, 4};
	for (uint chain = 0; chain < 5; ++chain) {
		SCOPED_TRACE(chain);
		const auto* anchorLink = physics.field_0x4[chain];
		const uint root = anchorLink->pointB;
		EXPECT_EQ(anchorLink->pointA, chain);
		EXPECT_EQ(physics.field_0x0[root]->boneHash, roots[chain]);
		EXPECT_FLOAT_EQ(physics.field_0x0[root]->inverseMass, 0.001f);
		for (uint offset = 1; offset < lengths[chain]; ++offset) {
			const auto* point = physics.field_0x0[root + offset];
			EXPECT_FLOAT_EQ(point->inverseMass, 1.0f + 0.25f * static_cast<float>(offset - 1));
			if (offset + 1 == lengths[chain]) {
				EXPECT_EQ(point->boneHash, 0u);
				EXPECT_NE(point->flags & 1, 0u);
			}
			else {
				EXPECT_EQ(point->boneHash, roots[chain] + offset * (chain == 0 ? 0x10000 : 0x100));
				EXPECT_EQ(point->flags & 1, 0u);
			}
		}
	}
	physics.Term();
}

TEST(ActorBonePhysics, DistanceConstraintMovesBothPointsByTheirWeight)
{
	CActorBonePhysics physics;
	physics.SetObjCounts(2, 1);
	physics.SetupObjects(nullptr);
	physics.field_0x0[0]->inverseMass = 1.0f;
	physics.field_0x0[1]->inverseMass = 1.0f;
	physics.field_0x0[0]->position = {0.0f, 0.0f, 0.0f, 1.0f};
	physics.field_0x0[1]->position = {2.0f, 0.0f, 0.0f, 1.0f};
	physics.field_0x4[0]->pointA = 0;
	physics.field_0x4[0]->pointB = 1;
	physics.field_0x4[0]->minDistance = 1.0f;
	physics.field_0x4[0]->maxDistance = 1.0f;
	physics.Func_0x34(0);
	EXPECT_NEAR(physics.field_0x0[0]->position.x, 0.5f, 1e-5f);
	EXPECT_NEAR(physics.field_0x0[1]->position.x, 1.5f, 1e-5f);
	physics.Term();
	EXPECT_EQ(physics.field_0x0, nullptr);
	EXPECT_EQ(physics.field_0x4, nullptr);
}

// PS2 0x003c0cc0, type-2 branch at 0x003c0f60. Root links must stay in
// the reference's vertical plane even before they reach their bend limit.
TEST(ActorBonePhysics, HairRootProjectsSidewaysMotionIntoItsVerticalPlane)
{
	CWolfenFullBodyPhysics physics;
	physics.SetObjCounts(2, 1);
	physics.CActorBonePhysics::SetupObjects(nullptr);
	physics.field_0x0[0]->position = {0, 0, 0, 1};
	physics.field_0x0[1]->position = {-0.25f, 0, 0, 1};
	auto* link = physics.field_0x4[0];
	link->pointB = 1;
	link->minDistance = link->maxDistance = 0.25f;
	link->constraintType = 2;
	link->axis = {0, 0, 1, 0};
	link->field_0x30.y = 0.85f;
	physics.Func_0x34(0);
	const auto& end = physics.field_0x0[1]->position;
	const float angle = (1.5707963268f + 0.85f) * 0.5f;
	EXPECT_NEAR(end.x, 0.0f, 1e-5f);
	EXPECT_NEAR(end.y, -0.25f * std::sin(angle), 1e-4f);
	EXPECT_NEAR(end.z, -0.25f * std::cos(angle), 1e-4f);
	EXPECT_FLOAT_EQ(end.w, 1.0f);
	physics.Term();
}

TEST(ActorBonePhysics, HairRootDampsTheReturnBendBelowItsLimit)
{
	CWolfenFullBodyPhysics physics;
	physics.SetObjCounts(2, 1);
	physics.CActorBonePhysics::SetupObjects(nullptr);
	physics.field_0x0[0]->position = {0, 0, 0, 1};
	physics.field_0x0[1]->position = {0, 0.25f * std::sin(0.4f), -0.25f * std::cos(0.4f), 1};
	auto* link = physics.field_0x4[0];
	link->pointB = 1;
	link->minDistance = link->maxDistance = 0.25f;
	link->constraintType = 2;
	link->axis = {0, 0, 1, 0};
	link->field_0x30.y = 0.85f;
	physics.Func_0x34(0);
	const auto& end = physics.field_0x0[1]->position;
	EXPECT_NEAR(end.y, 0.25f * std::sin(0.34f), 1e-4f);
	EXPECT_NEAR(end.z, -0.25f * std::cos(0.34f), 1e-4f);
	physics.Term();
}

// PS2 0x003c0e58: retain half the excess bend instead of hard-clamping it.
TEST(ActorBonePhysics, HairChildSoftensExcessBendRelativeToItsParent)
{
	CWolfenFullBodyPhysics physics;
	physics.SetObjCounts(3, 2);
	physics.CActorBonePhysics::SetupObjects(nullptr);
	physics.field_0x0[0]->position = {0, 0, 0.25f, 1};
	physics.field_0x0[1]->position = {0, 0, 0, 1};
	physics.field_0x0[2]->position = {-0.25f, 0, 0, 1};
	physics.field_0x4[0]->pointB = 1;
	auto* link = physics.field_0x4[1];
	link->pointA = 1;
	link->pointB = 2;
	link->minDistance = link->maxDistance = 0.25f;
	link->constraintType = 1;
	link->field_0x18 = 1;
	link->parentLink = 0;
	link->field_0x30.y = 1.0f;
	physics.Func_0x34(1);
	const auto& end = physics.field_0x0[2]->position;
	const float angle = (1.5707963268f + 1.0f) * 0.5f;
	EXPECT_NEAR(end.x, -0.25f * std::sin(angle), 1e-4f);
	EXPECT_NEAR(end.y, 0.0f, 1e-5f);
	EXPECT_NEAR(end.z, -0.25f * std::cos(angle), 1e-4f);
	physics.Term();
}

// PS2 0x0035b0e0: callback outputs belong to the rendered middle bone.
TEST(ActorBonePhysics, PunchingBallAimsAndScalesTheMiddleBoneWithoutMutatingEndpoints)
{
	CPunchingBallBonePhysics physics;
	physics.SetObjCounts(3, 2);
	physics.SetupObjects(nullptr);
	physics.field_0x0[1]->position = {0, 0, 0, 1};
	physics.field_0x0[2]->position = {0.36f, 0, -0.48f, 1};
	physics.field_0x0[2]->scale = {7, 8, 9, 1};
	edF32VECTOR4 rotation = {0, 0, 0, 1}, position = {1, 2, 3, 1};
	edF32VECTOR4 offset = {4, 5, 6, 0}, scale = {3, 4, 5, 1};
	physics.Func_0x3c(1, &rotation, &position, &offset, &scale);
	// edQuatFromAngAxis uses the PS2 sine/cosine lookup table.
	EXPECT_NEAR(rotation.y, std::sqrt(0.1f), 5e-4f);
	EXPECT_NEAR(rotation.w, std::sqrt(0.9f), 5e-4f);
	EXPECT_FLOAT_EQ(scale.x, 1.0f);
	EXPECT_FLOAT_EQ(scale.y, 1.0f);
	EXPECT_NEAR(scale.z, 2.0f, 1e-5f);
	EXPECT_FLOAT_EQ(physics.field_0x0[2]->scale.z, 9.0f);
	EXPECT_FLOAT_EQ(position.x, 1.0f);
	EXPECT_FLOAT_EQ(offset.x, 4.0f);
	physics.Term();
}

TEST(ActorBonePhysics, SharedWolfenInitializesGravityOnBothMiddlePoints)
{
	CWolfenSharedBonePhysics physics;
	physics.SetObjCounts(4, 3);
	physics.SetupObjects(nullptr);
	physics.field_0x70 = 2;
	EXPECT_EQ(physics.UpdateLinkedPostAnimEffects(), 1);
	EXPECT_FLOAT_EQ(physics.field_0x0[1]->gravity.y, -10.0f);
	EXPECT_FLOAT_EQ(physics.field_0x0[2]->gravity.y, -10.0f);
	EXPECT_FLOAT_EQ(physics.field_0x0[3]->gravity.y, 0.0f);
	physics.field_0x70 = 3;
	EXPECT_EQ(physics.UpdateLinkedPostAnimEffects(), 0);
	physics.Term();
}

// PS2 0x003e4b80 starts at this link's source, then skips unbound points.
TEST(ActorBonePhysics, BrazulReferenceAxisStartsAtTheNearestBoundSource)
{
	CBrazulBonePhysics physics;
	physics.SetObjCounts(16, 21);
	physics.SetupObjects(nullptr);
	physics.field_0x0[2]->position = {0, 0, 0, 1};
	physics.field_0x0[3]->position = {1, 0, 0, 1};
	physics.field_0x0[4]->position = {1, 0, 1, 1};
	physics.Func_0x14(1);
	EXPECT_NEAR(physics.field_0x4[3]->axis.x, 0.0f, 1e-5f);
	EXPECT_NEAR(physics.field_0x4[3]->axis.z, 1.0f, 1e-5f);
	physics.Term();
}

// PS2 0x003e3ad0 receives local pivot offset and scale as distinct arguments.
TEST(ActorBonePhysics, BrazulBoneCorrectionRotatesItsPivotAndPreservesScale)
{
	CBrazulBonePhysics physics;
	physics.SetObjCounts(16, 21);
	physics.SetupObjects(nullptr);
	physics.field_0x0[0]->position = {1, 0, 0, 1};
	physics.field_0x0[1]->position = {0, 0, 0, 1};
	physics.field_0x4[0]->field_0x30 = {0, 0, 1, 0};
	edF32VECTOR4 rotation = {0, 0, 0, 1}, position = {1, 2, 3, 1};
	edF32VECTOR4 offset = {0, 0, 2, 0}, scale = {3, 4, 5, 1};
	physics.Func_0x3c(1, &rotation, &position, &offset, &scale);
	EXPECT_NEAR(rotation.y, -std::sqrt(0.5f), 1e-4f);
	EXPECT_NEAR(offset.x, 2.0f, 1e-4f);
	EXPECT_NEAR(offset.z, 0.0f, 1e-4f);
	EXPECT_FLOAT_EQ(scale.x, 3.0f);
	EXPECT_FLOAT_EQ(scale.y, 4.0f);
	EXPECT_FLOAT_EQ(scale.z, 5.0f);
	EXPECT_FLOAT_EQ(position.y, 2.0f);
	physics.Term();
}

// PS2 0x003e4280: even the unparented outer-ring links collide with the floor.
TEST(ActorBonePhysics, BrazulFloorContactPreservesLinkLengthAndInverseMassWeights)
{
	CBrazulBonePhysics physics;
	physics.SetObjCounts(16, 21);
	physics.SetupObjects(nullptr);
	physics.field_0x74 = 0.0f;
	auto* link = physics.field_0x4[15];
	link->minDistance = 0.0f;
	link->maxDistance = 10.0f;
	physics.field_0x0[10]->position = {0, -1, 0, 1};
	physics.field_0x0[11]->position = {1, 1, 0, 1};
	physics.field_0x0[10]->inverseMass = 1.0f;
	physics.field_0x0[11]->inverseMass = 3.0f;
	physics.Func_0x34(15);
	const auto& a = physics.field_0x0[10]->position;
	const auto& b = physics.field_0x0[11]->position;
	EXPECT_FLOAT_EQ(a.y, 0.0f);
	EXPECT_FLOAT_EQ(b.y, 1.0f);
	EXPECT_NEAR(a.x, -0.25f, 1e-5f);
	EXPECT_NEAR(b.x, 1.75f, 1e-5f);
	EXPECT_NEAR((b.x - a.x) * (b.x - a.x) + (b.y - a.y) * (b.y - a.y), 5.0f, 1e-5f);
	physics.Term();
}

TEST(ActorBonePhysics, BrazulYawConstraintSoftensBendInTheParentFrame)
{
	CBrazulBonePhysics physics;
	physics.SetObjCounts(16, 21);
	physics.SetupObjects(nullptr);
	physics.field_0x74 = -10.0f;
	physics.field_0x0[0]->position = {0, 1, 0, 1};
	physics.field_0x0[1]->position = {0, 0, 0, 1};
	physics.field_0x0[2]->position = {-1, 0, 0, 1};
	auto* link = physics.field_0x4[1];
	link->minDistance = 0.0f;
	link->maxDistance = 10.0f;
	link->field_0x40 = link->field_0x44 = 0.0f;
	link->field_0x48 = -1.5707964f;
	link->field_0x4c = 1.5707964f;
	physics.Func_0x34(1);
	const auto& end = physics.field_0x0[2]->position;
	EXPECT_NEAR(end.x, -std::sqrt(0.5f), 2e-4f);
	EXPECT_NEAR(end.y, 0.0f, 1e-5f);
	EXPECT_NEAR(end.z, -std::sqrt(0.5f), 2e-4f);
	physics.Term();
}

TEST(ActorBonePhysics, BrazulWrappedYawRangeAcceptsBothSidesOfZero)
{
	CBrazulBonePhysics physics;
	physics.SetObjCounts(16, 21);
	physics.SetupObjects(nullptr);
	physics.field_0x74 = -10.0f;
	physics.field_0x0[0]->position = {0, 1, 0, 1};
	physics.field_0x0[1]->position = {0, 0, 0, 1};
	auto* link = physics.field_0x4[1];
	link->minDistance = 0.0f;
	link->maxDistance = 10.0f;
	link->field_0x40 = 5.5f;
	link->field_0x44 = 0.5f;
	link->field_0x48 = -1.5707964f;
	link->field_0x4c = 1.5707964f;
	for (float yaw : {-0.2f, 0.2f}) {
		SCOPED_TRACE(yaw);
		const edF32VECTOR4 position = {-std::sin(yaw), 0, -std::cos(yaw), 1};
		physics.field_0x0[2]->position = position;
		physics.Func_0x34(1);
		EXPECT_NEAR(physics.field_0x0[2]->position.x, position.x, 1e-5f);
		EXPECT_NEAR(physics.field_0x0[2]->position.z, position.z, 1e-5f);
	}
	physics.Term();
}

TEST(ActorBonePhysics, BrazulResetClearsTheCachedState)
{
	CBrazulBonePhysics physics;
	physics.SetObjCounts(16, 21);
	physics.SetupObjects(nullptr);
	BrazulStateStruct state{};
	physics.pCurrentState = &state;
	physics.Func_0x18();
	EXPECT_EQ(physics.pCurrentState, nullptr);
	physics.Term();
}
