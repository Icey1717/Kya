#ifndef ACTOR_BASIC_BOX_H
#define ACTOR_BASIC_BOX_H

#include "Types.h"
#include "Actor.h"
#include "ActorWind.h"
#include "Dynamic.h"

class CActorBasicBox;

class CBehaviourBasicBox : public CBehaviour
{
public:
	virtual void Create(ByteCode* pByteCode) {}
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);
	virtual int InterpretEvent(edCEventMessage* pEventMessage, undefined8 param_3, int param_4, uint* param_5) { return 0; }

	CActorBasicBox* pOwner;
};

class CBehaviourBasicBoxStand : public CBehaviourBasicBox
{
public:
	virtual void Create(ByteCode* pByteCode) {}
	virtual void Manage();
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual void InitState(int newState);
	virtual void TermState(int oldState, int newState);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);
};

#define BASIC_BOX_STATE_COUNT 4

class CActorBasicBox : public CActor {
public:
	CActorBasicBox();

	virtual void Create(ByteCode* pByteCode);
	virtual void Init();
	virtual void Term();
	virtual void Reset();
	virtual void SaveContext(uint*, int);
	virtual void LoadContext(uint*, int);
	virtual CBehaviour* BuildBehaviour(int behaviourType);
	virtual StateConfig* GetStateCfg(int state);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);
	virtual int InterpretEvent(edCEventMessage* pEventMessage, undefined8 param_3, int param_4, uint* param_5);

	void StateBasicBoxStandInit();

	static StateConfig _gStateCfg_BAB[BASIC_BOX_STATE_COUNT];

	CActorAlternateModel actorAlternateModel;
	CAddOnGenerator addOnGenerator;
	undefined4 classObj_0x1e4;
	undefined4 classObj_0x1e8;
	CVibrationDyn vibrationDyn;
	CBehaviourBasicBoxStand behaviourStand;

	S_ACTOR_STREAM_REF* pActorStream;
	S_NTF_TARGET_STREAM_REF* targetStreamRef;
	S_STREAM_EVENT_CAMERA* streamEventCamera;

	int field_0x160;
	float field_0x168;
	float field_0x16c;

	float field_0x170;
	float field_0x174;
	float field_0x178;
	float field_0x17c;

	uint field_0x180;

	int field_0x184;
	int field_0x188;
	int field_0x18c;
	int field_0x190;
	int field_0x194;
	int field_0x198;

	undefined4 field_0x2e0;
};

#endif //ACTOR_BASIC_BOX_H