#ifndef ACTOR_BOX_H
#define ACTOR_BOX_H

#include "Types.h"
#include "ActorAutonomous.h"
#include "ActorBonusServices.h"
#include "CinematicManager.h"
#include "Dynamic.h"
#include "Fx.h"

#define BOX_BEHAVIOUR_STAND 3

#define BOX_STAND_STATE_EXPLODE 0xc
#define BOX_STAND_STATE_DESTROYED 0xd

class CActorBox;

class CBehaviourBox : public CBehaviour
{
public:
	virtual void Create(ByteCode* pByteCode);
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);
	virtual int InterpretEvent(edCEventMessage* pEventMessage, undefined8 param_3, int param_4, uint* param_5);

	CActorBox* pOwner;
};

class CBehaviourBoxStand : public CBehaviourBox
{
public:
	virtual void Create(ByteCode* pByteCode);
	virtual void Manage();
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual void InitState(int newState);
	virtual void TermState(int oldState, int newState);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);
};

class CActorBox : public CActorAutonomous
{
public:
	static StateConfig _gStateCfg_BOX[8];

	virtual void Create(ByteCode* pByteCode);
	virtual void Init();
	virtual void Term();
	virtual void Reset();
	virtual void SaveContext(void* pData, uint mode, uint maxSize);
	virtual void LoadContext(void* pData, uint mode, uint maxSize);
	virtual CBehaviour* BuildBehaviour(int behaviourType);
	virtual StateConfig* GetStateCfg(int state);
	virtual bool CanPassThrough();
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);

	void BehaviourBoxStand_Manage();

	void StateBoxExplosionInit(CBehaviourBoxStand* pBehaviour);
	void StateBoxExplosion();

	void ApplyHit(_msg_hit_param** ppHitParam);
	void FUN_0036fb80();

	int field_0x350;
	float field_0x354;
	float field_0x358;
	float field_0x35c;
	float field_0x360;
	uint field_0x364;

	S_NTF_SWITCH ntfSwitch;

	int field_0x370;
	int field_0x374;
	int field_0x378;
	int field_0x37c;

	S_ACTOR_STREAM_REF* field_0x380;
	CAddOnGenerator addOnGenerator;
	CActorAlternateModel actorAlternateModel;

	CFxHandle field_0x3cc;

	CActor* field_0x3d4;
	CActor* field_0x3d8;

	float field_0x3dc;
	float field_0x3e0;

	edF32VECTOR4 field_0x4a0;
	edF32VECTOR4 field_0x490;

	CVibrationDyn vibrationDyn;

	CBehaviourBoxStand behaviourBoxStand;
};

#endif //ACTOR_BOX_H