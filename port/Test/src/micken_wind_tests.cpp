#include <gtest/gtest.h>

#include "ActorMicken.h"
#include "MathOps.h"

namespace
{
	class MickenWindActor : public CActorMicken
	{
	public:
		void ManageDyn(float drag, uint flags, CActorsTable* pActorsTable) override
		{
			managedFlags = flags;
		}

		uint managedFlags = 0;
	};
}

TEST(MickenWind, WindMessagesUpdateStateThroughAutonomousVirtualDispatch)
{
	CActorMicken actor;
	CActorAutonomous* pAutonomous = &actor;
	ASSERT_NE(pAutonomous->GetWindState(), nullptr);
	pAutonomous->GetWindState()->Reset();

	_wind_param_in params{};
	params.field_0x0 = 5.0f;
	params.field_0x4 = 4.0f;
	params.field_0x8 = 1;
	params.field_0xc = 1;
	params.field_0x1c = 1.0f;
	params.field_0x20 = 1.0f;
	EXPECT_EQ(actor.InterpretMessage(nullptr, MESSAGE_ENTER_WIND, &params), 1);
	EXPECT_EQ(pAutonomous->GetWindState()->nbActiveWind, 1);
	EXPECT_EQ(pAutonomous->GetWindState()->nbWindWaypoint, 1);
	EXPECT_EQ(pAutonomous->GetWindState()->nbCylinder, 1);
	EXPECT_FLOAT_EQ(pAutonomous->GetWindState()->neutralAirSpeed, 5.0f);

	EXPECT_EQ(actor.InterpretMessage(nullptr, MESSAGE_LEAVE_WIND, &params), 1);
	EXPECT_EQ(pAutonomous->GetWindState()->nbActiveWind, 0);
	EXPECT_EQ(pAutonomous->GetWindState()->nbWindWaypoint, 0);
	EXPECT_EQ(pAutonomous->GetWindState()->nbCylinder, 0);
}

TEST(MickenWind, ReleasedAirborneMickenRetainsGroundProjectionInPhysicsFlags)
{
	MickenWindActor actor;
	CActorAutonomous* pAutonomous = &actor;
	ASSERT_NE(pAutonomous->GetWindState(), nullptr);
	pAutonomous->GetWindState()->Reset();
	CCollision collision{};
	actor.pCollisionData = &collision;
	actor.dynamicExt.aImpulseVelocities[2] = gF32Vector4Zero;
	actor.dynamicExt.aImpulseVelocityMagnitudes[2] = 0.0f;
	actor.dynamic.linearAcceleration = 3.0f;
	actor.timeInAir = 0.02f;
	actor.StateMickenRollInTheWind();

	// Null wind state selected 0x2920, leaving ground response disabled after release.
	EXPECT_EQ(actor.managedFlags, 0x2932u);
	EXPECT_NE(actor.managedFlags & 2, 0u);
}
