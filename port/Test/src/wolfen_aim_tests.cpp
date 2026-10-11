#include <gtest/gtest.h>
#include "ActorWolfen.h"
#include "Animation.h"

TEST(WolfenSnipeAim, ScanTargetSelectsNormalizedAdjacentPoses)
{
	CActorWolfen actor;
	CBehaviourSnipe behaviour;
	BehaviourList<1> components{};
	components.count = 1;
	components.aComponents[0].id = WOLFEN_BEHAVIOUR_SNIPE;
	components.aComponents[0].SetBehaviour(&behaviour);
	actor.aComponents = reinterpret_cast<BehaviourEntry*>(&components);
	actor.curBehaviourId = WOLFEN_BEHAVIOUR_SNIPE;
	actor.pCollisionData = nullptr;
	actor.currentLocation = { 10.0f, 20.0f, 30.0f, 1.0f };
	// Base rotation must not be interpreted as the scan's world target.
	behaviour.rotationEuler = { 0.0f, 1.0f, 0.0f, 0.0f };

	alignas(edANM_HDR) unsigned char storage[sizeof(edANM_HDR) + 7 * sizeof(float)]{};
	auto* header = reinterpret_cast<edANM_HDR*>(storage);
	header->keyIndex_0x8.asKey = 3;
	edAnmMacroAnimator animator{};
	animator.pAnimKeyTableEntry = header;
	float* weights = header->pData + 3;
	header->pData[6] = 123.0f;

	const edF32VECTOR4 targets[] = {
		{ 10.0f, 30.0f, 30.0f, 1.0f },
		{ 10.0f, 20.0f, 40.0f, 1.0f },
		{ 10.0f, 10.0f, 30.0f, 1.0f },
		{ 10.0f, 17.0f, 34.0f, 1.0f },
	};
	const float expected[][3] = {
		{ 1.0f, 0.0f, 0.0f },
		{ 0.0f, 1.0f, 0.0f },
		{ 0.0f, 0.0f, 1.0f },
		{ 0.0f, 0.4f, 0.6f },
	};
	for (int sample = 0; sample < 4; ++sample) {
		SCOPED_TRACE(sample);
		behaviour.aimTarget = targets[sample];
		for (int pose = 0; pose < 3; ++pose) weights[pose] = 0.75f;
		actor.AnimEvaluate_0017c930(0, &animator);
		for (int pose = 0; pose < 3; ++pose) {
			EXPECT_NEAR(weights[pose], expected[sample][pose], 1e-5f);
		}
		EXPECT_NEAR(weights[0] + weights[1] + weights[2], 1.0f, 1e-5f);
		EXPECT_FLOAT_EQ(header->pData[6], 123.0f);
	}
}
