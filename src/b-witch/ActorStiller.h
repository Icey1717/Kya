#ifndef ACTOR_STILLER_H
#define ACTOR_STILLER_H

#include "Types.h"
#include "Actor.h"
#include "ActorServices.h"
#include "PathFollow.h"
#include "Vision.h"
#include "ActorBonusServices.h"

/*
* Stillers are the snake like enemies that usually guard the bulbous entities that provide magic when hit.
*/

#define STILLER_BEHAVIOUR_STAND 2

struct S_NTF_TARGET_STREAM_REF;
struct S_STREAM_EVENT_CAMERA;
class CActorStiller;

class CBehaviourStiller : public CBehaviour
{
public:
	virtual void Create(ByteCode* pByteCode);
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);
	virtual int InterpretEvent(edCEventMessage* pEventMessage, undefined8 param_3, int param_4, uint* param_5);

	CActorStiller* pOwner;
};

class CBehaviourStillerStand : public CBehaviourStiller
{
public:
	virtual void Create(ByteCode* pByteCode);
	virtual void Manage();
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual void InitState(int newState);
	virtual void TermState(int oldState, int newState);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);
};

class CActorStiller : public CActor
{
public:

	static StateConfig _gStateCfg_STI[5];

	virtual void Create(ByteCode* pByteCode);
	virtual void Init();
	virtual void Term();
	virtual void Draw();
	virtual void Reset();
	virtual CBehaviour* BuildBehaviour(int behaviourType);
	virtual StateConfig* GetStateCfg(int state);
	virtual void ChangeManageState(int state);
	virtual CVision* GetVision();
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);

	void BehaviourStillerStand_InitState(int newState);
	void BehaviourStillerStand_Manage();
	void BehaviourStillerStand_TermState(int oldState);	

	CVision vision;

	CBehaviourStillerStand behaviourStand;

	CPathFollowReader pathFollowReader;

	CAddOnGenerator addOnGenerator;

	uint field_0x1d0;
	uint field_0x1d4;
	uint field_0x1d8;
	uint field_0x1dc;

	float field_0x1e0;

	CLifeBase lifeBase;

	float field_0x1ec;
	float field_0x1f0;
	float field_0x1f4;

	int field_0x1f8;

	S_ACTOR_STREAM_REF* field_0x1fc;

	int field_0x200;

	S_NTF_TARGET_STREAM_REF* field_0x204;
	S_STREAM_EVENT_CAMERA* field_0x208;
	S_NTF_TARGET_STREAM_REF* field_0x20c;
	S_STREAM_EVENT_CAMERA* field_0x210;

	int field_0x214;

	undefined4 field_0x25c;
};

#endif //ACTOR_STILLER_H