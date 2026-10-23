#include <gtest/gtest.h>

#include <limits>
#include <memory>
#include "ActorNativ.h"
#include "ActorNativCmd.h"
#include "ActorMovingPlatform.h"

namespace
{
	class MessageTestNativ : public CActorNativ
	{
	public:
		int ReceiveMessage(CActor*, ACTOR_MESSAGE message, MSG_PARAM param) override
		{
			EXPECT_EQ(message, MESSAGE_NATIV_CMD);
			lastCommand = static_cast<CActorNativMsgParam_0xe*>(param)->type;
			++messageCount;
			return 1;
		}

		int lastCommand = -1;
		int messageCount = 0;
	};

	class MessageTestNativCmd : public CActorNativCmd
	{
	public:
		int ReceiveMessage(CActor* sender, ACTOR_MESSAGE message, MSG_PARAM param) override
		{
			return InterpretMessage(sender, message, param);
		}
	};

	class AnalogMessageReceiver : public CActor
	{
	public:
		int ReceiveMessage(CActor*, ACTOR_MESSAGE message, MSG_PARAM param) override
		{
			EXPECT_EQ(message, 0x11);
			payload = *static_cast<_msg_analog_params*>(param);
			++messageCount;
			return slave.InterpretMessage(nullptr, message, param);
		}

		_msg_analog_params payload{};
		CBehaviourWeighingMachineSlave slave;
		int messageCount = 0;
	};
}

TEST(MessageParameters, CommanderReplacementCancelsTheNativesActualConversation)
{
	auto native = std::make_unique<MessageTestNativ>();
	auto peer = std::make_unique<MessageTestNativ>();
	MessageTestNativCmd oldCommander;
	CActor newCommander;
	CTalkParamSubObj participants[] = { { native.get(), 6 }, { peer.get(), 6 } };
	CTalkParam conversations[2]{};
	conversations[0].field_0x10 = 11; // Must not cancel conversation zero.
	conversations[0].totalTime = 42.0f;
	conversations[1].field_0x8 = participants;
	conversations[1].nbSubObj = 2;
	conversations[1].field_0x10 = 2;
	conversations[1].field_0x1c = 1;
	oldCommander.nbTalkParams = 2;
	oldCommander.aTalkParams = conversations;
	oldCommander.flags = 0;
	native->typeID = NATIV;
	native->flags = 0;
	peer->flags = 0;
	native->curBehaviourId = NATIVE_BEHAVIOUR_SPEAK;
	native->behaviourSpeak.field_0x24 = 1;
	native->field_0x3f0 = &oldCommander;
	CActorNativMsgParam_0xe command{};
	command.type = 1;
	command.field_0x4 = -1; // No navigation database is needed.

	EXPECT_EQ(native->InterpretMessage(&newCommander, MESSAGE_NATIV_CMD, &command), 1);
	EXPECT_EQ(native->field_0x3f0, &newCommander);
	EXPECT_EQ(native->messageCount, 1);
	EXPECT_EQ(peer->messageCount, 1);
	EXPECT_EQ(native->lastCommand, 2);
	EXPECT_EQ(peer->lastCommand, 2);
	EXPECT_EQ(conversations[1].field_0x10, 0);
	EXPECT_EQ(conversations[1].field_0x1c, 0);
	EXPECT_EQ(participants[0].pNativ, nullptr);
	EXPECT_EQ(participants[1].pNativ, nullptr);
	EXPECT_EQ(conversations[0].field_0x10, 11);
	EXPECT_FLOAT_EQ(conversations[0].totalTime, 42.0f);
}

TEST(MessageParameters, CancellationRejectsInvalidConversationIndices)
{
	auto native = std::make_unique<MessageTestNativ>();
	MessageTestNativCmd commander;
	CTalkParam conversation{};
	conversation.field_0x10 = 11;
	commander.nbTalkParams = 1;
	commander.aTalkParams = &conversation;
	native->typeID = NATIV;
	CActorNativMsgParam_0xe command{};
	command.type = 7;
	for (int index : { -1, 1, std::numeric_limits<int>::max() }) {
		command.field_0x8 = index;
		EXPECT_EQ(commander.InterpretMessage(native.get(), MESSAGE_NATIV_CMD, &command), 0);
		EXPECT_EQ(conversation.field_0x10, 11);
		EXPECT_EQ(native->messageCount, 0);
	}
	command.field_0x8 = 0;
	commander.aTalkParams = nullptr;
	EXPECT_EQ(commander.InterpretMessage(native.get(), MESSAGE_NATIV_CMD, &command), 0);
}

TEST(MessageParameters, AnalogNotificationCarriesBothWordsAndUpdatesTheSlave)
{
	AnalogMessageReceiver receiver;
	CActor sender;
	receiver.flags = 0;
	S_STREAM_NTF_TARGET_ANALOG notification{};
	notification.pRef = STORE_POINTER(&receiver);
	notification.field_0x1c = 0;
	notification.field_0x10 = 0x12345678;

	EXPECT_EQ(notification.NotifyAnalog(0.25f, 0.75f, &sender, nullptr), 1);
	EXPECT_EQ(receiver.messageCount, 1);
	EXPECT_FLOAT_EQ(receiver.payload.value, 0.75f);
	EXPECT_EQ(receiver.payload.field_0x4, 0x12345678u);
	EXPECT_FLOAT_EQ(receiver.slave.field_0x34, 0.75f);

	// A missing sender falls back to the notification's target actor.
	notification.field_0x10 = 0;
	EXPECT_EQ(notification.NotifyAnalog(0.75f, -0.5f, nullptr, nullptr), 1);
	EXPECT_EQ(receiver.messageCount, 2);
	EXPECT_FLOAT_EQ(receiver.payload.value, -0.5f);
	EXPECT_EQ(receiver.payload.field_0x4, 0u);
	EXPECT_FLOAT_EQ(receiver.slave.field_0x34, -0.5f);
}
