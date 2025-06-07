#ifndef ACTOR_AMORTOS_H
#define ACTOR_AMORTOS_H

#include "Types.h"
#include "Actor.h"

#define AMORTOS_STATE_EXPLODE 0x7
#define AMORTOS_STATE_WAIT_RESWELL 0x8

class CCylinderDetection
{
public:
	CCylinderDetection();
	bool DetectAttract(CActor* pActor);

	float field_0x0;
	float field_0x4;
	float field_0x8;
	float field_0xc;
	float field_0x10;

	edF32VECTOR4 field_0x20;
	edF32VECTOR4 field_0x30;
};

struct S_TARGET_STREAM_REF;
struct S_STREAM_EVENT_CAMERA;

class CActorMovable;
class CActorAmortos;

class CBehaviourAmortos : public CBehaviour
{
public:
	virtual void Create(ByteCode* pByteCode) {}
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam) { return 0; }
	virtual int InterpretEvent(edCEventMessage* pEventMessage, undefined8 param_3, int param_4, uint* param_5) { return 0; }

	CActorAmortos* pOwner;
};

class CBehaviourAmortosStand : public CBehaviourAmortos
{
public:
	virtual void Create(ByteCode* pByteCode) {}
	virtual void Manage();
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual void InitState(int newState);
	virtual void TermState(int oldState, int newState);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam) { return 0; }
};

class CActorAmortos : public CActor 
{
public:
	CActorAmortos();

	static StateConfig _gStateCfg_AMO[5];

	virtual void Create(ByteCode* pByteCode);
	virtual void Init();
	virtual void Term();
	virtual void Reset();
	virtual CBehaviour* BuildBehaviour(int behaviourType);
	virtual StateConfig* GetStateCfg(int state);
	virtual void ChangeManageState(int state);
	virtual CVision* GetVision();
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);
	virtual int InterpretEvent(edCEventMessage* pEventMessage, undefined8 param_3, int param_4, uint* param_5);

	void BehaviourAmortosStand_Manage();
	void BehaviourAmortosStand_TermState();

	bool DetectActorWithCylinder();

	void StateAmortosWaitReSwell();
	void StateAmortosExplode();

	CAddOnGenerator addOnGenerator;

	float field_0x160;
	float field_0x164;
	float field_0x168;
	float field_0x16c;
	float field_0x170;
	float field_0x174;
	float field_0x178;
	float field_0x17c;
	float field_0x180;

	CActorMovable* pLaunchedActor;

	S_ACTOR_STREAM_REF* pCheckpoint_0x184;
	S_ACTOR_STREAM_REF* field_0x188;
	S_ACTOR_STREAM_REF* field_0x18c;

	S_STREAM_REF<CActor> field_0x190;

	uint field_0x1c8;
	S_TARGET_STREAM_REF* field_0x1cc;

	S_STREAM_EVENT_CAMERA* field_0x1d0;
	S_TARGET_STREAM_REF* field_0x1d4;
	S_STREAM_EVENT_CAMERA* field_0x1d8;

	CBehaviourAmortosStand behaviourStand;

	CCylinderDetection cylinderDetection;

	float field_0x2a0;
	float field_0x2b4;
	float field_0x2b0;
	byte field_0x2ac;
	float field_0x2a8;

	CActorsTable field_0x408;
};

#endif //ACTOR_AMORTOS_H