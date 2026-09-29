#include <gtest/gtest.h>

#include "ActorWolfen.h"
#include "ActorBrazul.h"
#include "ActorPunchingBall.h"

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
