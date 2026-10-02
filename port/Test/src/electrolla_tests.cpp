#include <gtest/gtest.h>

#include "ActorElectrolla.h"
#include "AnmManager.h"
#include "MathOps.h"
#include "TimeController.h"

namespace
{
	class ElectrollaActor : public CActorElectrolla
	{
	public:
		void SetState(int newState, int animationType) override
		{
			this->actorState = static_cast<EActorState>(newState);
			lastAnimationType = animationType;
			this->behaviourStand.InitState(newState);
		}

		int lastAnimationType = -2;
	};

	class ElectrollaTest : public testing::Test
	{
	protected:
		void SetUp() override
		{
			previousDelta = GetTimer()->cutsceneDeltaTime;
			GetTimer()->cutsceneDeltaTime = 0.125f;
			animation.pBoneData = &bone;
			animation.anmBinMetaAnimator.aAnimData = &layer;
			bone.boneId = 0x45477cb3;
			bone.matrix = gF32Matrix4Unit;
			bone.matrix.rowT = edF32VECTOR4{ 1.0f, 2.0f, 3.0f, 1.0f };
			mesh.base.transformA = gF32Matrix4Unit;
			mesh.base.transformA.rowT = edF32VECTOR4{ 10.0f, 20.0f, 30.0f, 1.0f };
			actor.pAnimationController = &animation;
			actor.pMeshTransform = &mesh;
			actor.subObjA = &bounds;
			actor.scale = edF32VECTOR4{ 1.0f, 1.0f, 1.0f, 0.0f };
			actor.field_0x1c4.pStreamEventCamera = &camera;
			actor.behaviourStand.pOwner = &actor;
		}

		void TearDown() override
		{
			GetTimer()->cutsceneDeltaTime = previousDelta;
		}

		float previousDelta;
		ElectrollaActor actor;
		CAnimation animation;
		BoneData bone{};
		edAnmLayer layer{};
		ed_3d_hierarchy_node mesh{};
		KyaUpdateObjA bounds{};
		S_STREAM_EVENT_CAMERA camera{};
	};
}

TEST_F(ElectrollaTest, HitsStartChargingAndDoNotRestartProtectedStates)
{
	actor.actorState = static_cast<EActorState>(ELECTROLLA_STATE_SCAN);
	actor.field_0x1a8.Add(&actor);
	actor.field_0x1a0 = 3.0f;
	EXPECT_EQ(actor.InterpretMessage(nullptr, 1, nullptr), 1);
	EXPECT_EQ(actor.InterpretMessage(nullptr, MESSAGE_KICKED, nullptr), 1);
	EXPECT_EQ(actor.actorState, ELECTROLLA_STATE_CHARGE);
	EXPECT_EQ(actor.field_0x1a8.nbEntries, 0);
	EXPECT_FLOAT_EQ(actor.field_0x1a0, 0.0f);

	for (int state = ELECTROLLA_STATE_CHARGE; state <= ELECTROLLA_STATE_COOLDOWN; state++) {
		actor.actorState = static_cast<EActorState>(state);
		actor.field_0x1a0 = 1.0f;
		actor.field_0x1a8.Add(&actor);
		EXPECT_EQ(actor.InterpretMessage(nullptr, MESSAGE_KICKED, nullptr), 1);
		EXPECT_EQ(actor.actorState, state);
		EXPECT_FLOAT_EQ(actor.field_0x1a0, 1.0f);
		EXPECT_EQ(actor.field_0x1a8.nbEntries, 1);
		actor.field_0x1a8.nbEntries = 0;
	}
}

TEST_F(ElectrollaTest, BehaviourBeginSelectsIdleOrTheRequestedStateAndResetsItsTimer)
{
	EXPECT_EQ(actor.BuildBehaviour(ELECTROLLA_BEHAVIOUR_STAND), &actor.behaviourStand);
	actor.field_0x1a0 = 4.0f;
	actor.behaviourStand.Begin(&actor, -1, 123);
	EXPECT_EQ(actor.actorState, ELECTROLLA_STATE_STAND);
	EXPECT_EQ(actor.lastAnimationType, -1);
	EXPECT_FLOAT_EQ(actor.field_0x1a0, 0.0f);
	actor.behaviourStand.Begin(&actor, ELECTROLLA_STATE_ALERT, 7);
	EXPECT_EQ(actor.actorState, ELECTROLLA_STATE_ALERT);
	EXPECT_EQ(actor.lastAnimationType, 7);
}

TEST_F(ElectrollaTest, BlendWeightsAreWrittenAfterTheAnimationKeyTable)
{
	struct AnimationData
	{
		int count;
		edANM_HDR_Internal duration;
		edANM_HDR_Internal keyCount;
		float data[4];
	} data{};
	data.keyCount.asKey = 2;
	data.data[0] = 12.0f;
	data.data[1] = 34.0f;
	edAnmMacroAnimator animator{};
	animator.pAnimKeyTableEntry = reinterpret_cast<edANM_HDR*>(&data);
	actor.currentAnimType = 8;
	actor.field_0x188 = 0.75f;
	actor.AnimEvaluate(0, &animator, 8);
	EXPECT_FLOAT_EQ(data.data[0], 12.0f);
	EXPECT_FLOAT_EQ(data.data[1], 34.0f);
	EXPECT_FLOAT_EQ(data.data[2], 0.25f);
	EXPECT_FLOAT_EQ(data.data[3], 0.75f);
}

TEST_F(ElectrollaTest, ChargingUpdatesTheBonePositionAndWaitsForItsDelay)
{
	actor.actorState = static_cast<EActorState>(ELECTROLLA_STATE_CHARGE);
	actor.field_0x194 = 0.5f;
	actor.field_0x1a0 = 0.25f;
	actor.behaviourStand.Manage();
	EXPECT_EQ(actor.actorState, ELECTROLLA_STATE_CHARGE);
	EXPECT_FLOAT_EQ(actor.field_0x1a0, 0.375f);
	EXPECT_FLOAT_EQ(actor.field_0x160.x, 11.0f);
	EXPECT_FLOAT_EQ(actor.field_0x160.y, 22.15f);
	EXPECT_FLOAT_EQ(actor.field_0x160.z, 33.0f);
	EXPECT_FLOAT_EQ(actor.field_0x160.w, 1.0f);
	EXPECT_GE(actor.field_0x18c, 0.36f);
	EXPECT_LE(actor.field_0x18c, 0.6f);
}

TEST_F(ElectrollaTest, CooldownUsesTheOriginalBoundaryAndRestoresTheBoundingSphere)
{
	actor.actorState = static_cast<EActorState>(ELECTROLLA_STATE_COOLDOWN);
	actor.field_0x178 = 2.0f;
	actor.field_0x19c = 0.5f;
	actor.field_0x1a0 = 0.375f;
	actor.behaviourStand.Manage();
	EXPECT_EQ(actor.actorState, ELECTROLLA_STATE_COOLDOWN);
	EXPECT_FLOAT_EQ(actor.field_0x1a0, 0.5f);
	actor.behaviourStand.Manage();
	EXPECT_EQ(actor.actorState, ELECTROLLA_STATE_STAND);
	EXPECT_FLOAT_EQ(actor.field_0x1a0, 0.125f);
	EXPECT_FLOAT_EQ(bounds.boundingSphere.w, 2.0f);
	EXPECT_FLOAT_EQ(actor.sphereCentre.w, 2.0f);
}

TEST_F(ElectrollaTest, DischargeEndWaitsForTheCurrentAnimationToFinish)
{
	actor.actorState = static_cast<EActorState>(ELECTROLLA_STATE_DISCHARGE_END);
	animation.currentAnimType = 10;
	layer.currentAnimDesc.animType = 10;
	layer.animPlayState = 1;
	layer.field_0xcc = 0;
	actor.behaviourStand.Manage();
	EXPECT_EQ(actor.actorState, ELECTROLLA_STATE_DISCHARGE_END);
	layer.field_0xcc = 2;
	actor.behaviourStand.Manage();
	EXPECT_EQ(actor.actorState, ELECTROLLA_STATE_COOLDOWN);
}
