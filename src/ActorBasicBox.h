#ifndef ACTOR_BASIC_BOX_H
#define ACTOR_BASIC_BOX_H

#include "Types.h"
#include "Actor.h"
#include "ActorWind.h"

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

struct VibrationParam {
	float field_0x0;
	float field_0x4;
	float field_0x8;
	float field_0xc;
	float field_0x10;
	float field_0x14;
	CActor* pActor;
};

class CVibrationDyn 
{
public:
	void Init(VibrationParam* pParams);
	void UpdateAllFactors(float param_1, edF32VECTOR4* param_3, edF32VECTOR4* pPosition);
	int UpdateKineticToPotentialUpDown();
	int ComputeCurFactor();
	bool MakeVibrate(edF32VECTOR3* param_2);

	float field_0x0;
	float field_0x4;
	undefined4 field_0x8;
	undefined4 field_0xc;
	float field_0x10;
	float field_0x14;
	float field_0x18;
	float field_0x1c;
	float field_0x20;
	float field_0x24;
	undefined4 field_0x28;
	undefined4 field_0x2c;
	byte field_0x30;
	byte field_0x31;
	undefined field_0x32;
	undefined field_0x33;
	struct CActor* pActor;
	float field_0x38;
	float field_0x3c;
	float field_0x40;
	float field_0x44;
	float field_0x48;
	float field_0x4c;
	undefined field_0x50;
	undefined field_0x51;
	undefined field_0x52;
	undefined field_0x53;
	float field_0x54;
	undefined field_0x58;
	undefined field_0x59;
	undefined field_0x5a;
	undefined field_0x5b;
	undefined field_0x5c;
	undefined field_0x5d;
	undefined field_0x5e;
	undefined field_0x5f;
	edF32MATRIX4 field_0x60;
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
	S_TARGET_STREAM_REF* targetStreamRef;
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