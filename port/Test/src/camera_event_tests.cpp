#include <gtest/gtest.h>

#include "camera.h"
#include "CameraViewManager.h"

namespace
{
	class CameraMessageActor : public CActor
	{
	public:
		int ReceiveMessage(CActor* pSender, ACTOR_MESSAGE message, MSG_PARAM param) override
		{
			++messageCount;
			sender = pSender;
			lastMessage = message;
			lastParam = param;
			return 1;
		}

		int messageCount = 0;
		CActor* sender = NULL;
		ACTOR_MESSAGE lastMessage = static_cast<ACTOR_MESSAGE>(0);
		MSG_PARAM lastParam = NULL;
	};

	struct CameraActorList
	{
		int entryCount;
		S_STREAM_REF<CActor> aEntries[2];
	};

	class CameraEventTest : public testing::Test
	{
	protected:
		void SetUp() override
		{
			CCamera::_gpcam_man = &manager;
			senderList.entryCount = 1;
			senderList.aEntries[0].pObj = STORE_POINTER(&sourceActor);
			// A second unrelated actor makes wrong-list indexing observable without
			// depending on an out-of-bounds access in the old implementation.
			senderList.aEntries[1].pObj = STORE_POINTER(&unrelatedActor);
			receiverList.entryCount = 2;
			receiverList.aEntries[0].pObj = STORE_POINTER(&firstReceiver);
			receiverList.aEntries[1].pObj = STORE_POINTER(&secondReceiver);
			camera.field_0x84 = reinterpret_cast<S_ACTOR_STREAM_REF*>(&senderList);
			camera.field_0x88 = reinterpret_cast<S_ACTOR_STREAM_REF*>(&receiverList);
			camera.flags_0xc |= 2;
			camera.field_0x8c = 1.0f;
		}

		void TearDown() override
		{
			CCamera::_gpcam_man = previousCameraManager;
			CCameraManager::_gThis = previousManager;
		}

		void ExpectRecipients(ACTOR_MESSAGE message)
		{
			EXPECT_EQ(sourceActor.messageCount, 0);
			EXPECT_EQ(unrelatedActor.messageCount, 0);
			for (CameraMessageActor* receiver : { &firstReceiver, &secondReceiver }) {
				EXPECT_EQ(receiver->messageCount, 1);
				EXPECT_EQ(receiver->sender, &sourceActor);
				EXPECT_EQ(receiver->lastMessage, message);
				EXPECT_EQ(receiver->lastParam, reinterpret_cast<MSG_PARAM>(1));
			}
		}

		CCameraManager* previousManager = CCameraManager::_gThis;
		CCameraManager* previousCameraManager = CCamera::_gpcam_man;
		CCameraManager manager;
		CCamera camera;
		CameraMessageActor sourceActor, unrelatedActor, firstReceiver, secondReceiver;
		CameraActorList senderList = {}, receiverList = {};
	};
}

TEST_F(CameraEventTest, ActivationAlertMessagesRecipientsFromTheFirstSourceActor)
{
	camera.AlertCamera(2, NULL, NULL);
	ExpectRecipients(static_cast<ACTOR_MESSAGE>(0x29));
}

TEST_F(CameraEventTest, DeactivationAlertMessagesRecipientsFromTheFirstSourceActor)
{
	camera.AlertCamera(1, NULL, NULL);
	ExpectRecipients(static_cast<ACTOR_MESSAGE>(0x2a));
}

TEST_F(CameraEventTest, LeavingLastClusterMessagesRecipientsBeforeRemovingCamera)
{
	camera.nbManagedByClusters = 1;
	manager.cameraStack.aCameras[0].pCamera = &camera;
	camera.LeaveManagedCluster();
	ExpectRecipients(static_cast<ACTOR_MESSAGE>(0x2a));
	EXPECT_EQ(camera.nbManagedByClusters, 0);
}

TEST_F(CameraEventTest, ZoneExitMessagesRecipientsFromTheFirstSourceActor)
{
	// No zone actors remain inside, but the previous frame was active.
	senderList.entryCount = 0;
	camera.field_0x7c = 1;
	camera.ManageEvents();
	ExpectRecipients(static_cast<ACTOR_MESSAGE>(0x2a));
	EXPECT_EQ(camera.field_0x7c, 0u);
}
