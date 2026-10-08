#include <gtest/gtest.h>

#include "ActorSwitch.h"
#include "ActorMovingPlatform.h"
#include "TimeController.h"

namespace
{
	class TimedSwitchOwner : public CActorSwitch
	{
	public:
		void SetState(int state, int animationType) override
		{
			this->actorState = static_cast<EActorState>(state);
		}
	};

	class PumpGauge : public CActorMovingPlatform
	{
	public:
		void SetState(int state, int animationType) override
		{
			this->actorState = static_cast<EActorState>(state);
		}

		int ReceiveMessage(CActor* pSender, ACTOR_MESSAGE message, MSG_PARAM param) override
		{
			++stopCount;
			trajectory.InterpretMessage(pSender, message, param);
			return 1;
		}

		CBehaviourPlatformTrajectory trajectory;
		int stopCount = 0;
	};

	struct TargetStream
	{
		int entryCount;
		S_STREAM_NTF_TARGET_SWITCH entry;
	};

	class TimedSwitchTest : public testing::Test
	{
	protected:
		void SetUp() override
		{
			previousDelta = GetTimer()->cutsceneDeltaTime;
			previousTime = GetTimer()->scaledTotalTime;
			g_CinematicManager_0048efc = &cinematicManager;
			GetTimer()->cutsceneDeltaTime = 0.02f;
			GetTimer()->scaledTotalTime = 100.0f;
			camera.cameraIndex = -1;
			entryCamera.cameraIndex = -1;
			owner.targetSwitch = { reinterpret_cast<S_NTF_TARGET_STREAM_REF*>(&emptyStream), &camera };
			gauge.pProperties = &properties;
			properties.flags_0x24 = 0x20f; // PUMP_JAUGE's level data.
			gauge.trajectory.pOwner = &gauge;
			gauge.trajectory.segmentStartTime = 0.0f;
			gauge.trajectory.trajPos.pathPosition = 0.0f;
			gauge.actorState = static_cast<EActorState>(MOVING_PLATFORM_STATE_MOVING);
			stopStream.entryCount = 1;
			stopStream.entry.pRef = STORE_POINTER(&gauge);
			stopStream.entry.cutsceneId = -1;
			stopStream.entry.messageId = 0x10;
			behaviour.entryCount = 1;
			behaviour.aEntries = new S_SWITCH_TIMED_ENTRY[1];
			behaviour.aEntries[0] = { { reinterpret_cast<S_NTF_TARGET_STREAM_REF*>(&stopStream), &entryCamera }, 0.22f };
			behaviour.pOwner = &owner;
			behaviour.playbackMode = 0;
			behaviour.flags = 0;
			behaviour.Begin(&owner, -1, -1);
		}

		void TearDown() override
		{
			GetTimer()->cutsceneDeltaTime = previousDelta;
			GetTimer()->scaledTotalTime = previousTime;
			g_CinematicManager_0048efc = previousCinematicManager;
		}

		void Tick()
		{
			GetTimer()->scaledTotalTime += GetTimer()->cutsceneDeltaTime;
			gauge.trajectory.trajPos.pathPosition = gauge.BehaviourTrajectory_ComputeTime(&gauge.trajectory);
			behaviour.Manage();
		}

		TimedSwitchOwner owner;
		PumpGauge gauge;
		CActorMovingPlatform_SubObj properties{};
		CBehaviourSwitchTimed behaviour;
		TargetStream stopStream{};
		TargetStream emptyStream{};
		S_STREAM_EVENT_CAMERA camera{};
		S_STREAM_EVENT_CAMERA entryCamera{};
		CCinematicManager* previousCinematicManager = g_CinematicManager_0048efc;
		CCinematicManager cinematicManager;
		float previousDelta;
		float previousTime;
	};
}

TEST_F(TimedSwitchTest, PumpHitsStopAfterAPartialFillAndAccumulateOverMultipleHits)
{
	for (int hit = 0; hit < 7; hit++) {
		gauge.trajectory.InterpretMessage(&owner, 0xf, nullptr);
		behaviour.InterpretMessage(&owner, 0x4f, nullptr);
		for (int frame = 0; frame < 10; frame++) {
			Tick();
		}
		EXPECT_EQ(gauge.actorState, MOVING_PLATFORM_STATE_STAND);
		for (int frame = 0; frame < 3; frame++) {
			Tick();
		}
		EXPECT_EQ(gauge.actorState, MOVING_PLATFORM_STATE_AT_ENDPOINT);
		EXPECT_EQ(gauge.stopCount, hit + 1);
		if (hit == 0) {
			EXPECT_GT(gauge.trajectory.trajPos.pathPosition, 0.2f);
			EXPECT_LT(gauge.trajectory.trajPos.pathPosition, 0.3f);
		}
	}
	// The configured path takes 1.3 seconds to fill; one hit supplies only a pulse.
	EXPECT_GT(gauge.trajectory.trajPos.pathPosition, 1.3f);
}

TEST_F(TimedSwitchTest, RestartResetsTheDelayAndStopCancelsPlayback)
{
	behaviour.InterpretMessage(&owner, 0x4f, nullptr);
	for (int frame = 0; frame < 8; frame++) Tick();
	behaviour.InterpretMessage(&owner, 0x4f, nullptr);
	for (int frame = 0; frame < 8; frame++) Tick();
	EXPECT_EQ(gauge.stopCount, 0);
	behaviour.InterpretMessage(&owner, 0x50, nullptr);
	for (int frame = 0; frame < 20; frame++) Tick();
	EXPECT_EQ(gauge.stopCount, 0);
	behaviour.InterpretMessage(&owner, 0x4f, nullptr);
	for (int frame = 0; frame < 13; frame++) Tick();
	EXPECT_EQ(gauge.stopCount, 1);
}

TEST_F(TimedSwitchTest, CreateReadsTimedEntriesAndClampsNegativeDelays)
{
	// The same entry layout as SWITCH_L3: base delay, notification list,
	// camera event, per-entry delay, playback mode and flags.
	alignas(16) uint data[] = {
		0x3e6147ae, 1, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0xbf800000, 1, 3
	};
	ByteCode byteCode;
	byteCode.currentSeekPos = reinterpret_cast<char*>(data);
	CBehaviourSwitchTimed parsed;
	parsed.Create(&byteCode);
	EXPECT_FLOAT_EQ(parsed.baseDelay, 0.22f);
	ASSERT_EQ(parsed.entryCount, 1);
	EXPECT_FLOAT_EQ(parsed.aEntries[0].delay, 0.0f);
	EXPECT_EQ(parsed.playbackMode, 1);
	EXPECT_EQ(parsed.flags, 3);
	EXPECT_EQ(byteCode.currentSeekPos, reinterpret_cast<char*>(data + std::size(data)));
}
