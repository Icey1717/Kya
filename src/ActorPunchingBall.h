#ifndef ACTOR_PUNCHING_BALL_H
#define ACTOR_PUNCHING_BALL_H

#include "Types.h"
#include "ActorFighter.h"
#include "ActorBasicBox.h"
#include "CameraViewManager.h"

class CActorPunchingBall;

struct S_TRAP_STREAM_REF;

#define PUNCHING_BALL_BEHAVIOUR_STAND 7

class CBehaviourPunchingBall : public CBehaviour
{
public:
	virtual void Create(ByteCode* pByteCode);
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);
	virtual int InterpretEvent(edCEventMessage* pEventMessage, undefined8 param_3, int param_4, uint* param_5);

	CActorPunchingBall* pOwner;
};

class CBehaviourPunchingBallStand : public CBehaviourPunchingBall
{
public:
	virtual void Create(ByteCode* pByteCode);
	virtual void Manage();
	virtual void Begin(CActor* pOwner, int newState, int newAnimationType);
	virtual void InitState(int newState);
	virtual void TermState(int oldState, int newState);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);
};

class CBehaviourPunchingBallSlave : public CBehaviourFighterSlave
{

};

class CBehaviourPunchingBallRidden : public CBehaviourFighterRidden
{
};

class CBehaviourPunchingBallDefault : public CBehaviourFighter
{
};

class CActorPunchingBall : public CActorFighter {
public:
	static StateConfig _gStateCfg_PBA[6];

	CActorPunchingBall(){
		IMPLEMENTATION_GUARD_LOG()
	}

	// CActor
	virtual void Create(ByteCode* pByteCode);
	virtual void Init();
	virtual void Term();
	virtual void Manage();
	virtual void Reset();
	virtual CBehaviour* BuildBehaviour(int behaviourType);
	virtual StateConfig* GetStateCfg(int state);
	virtual uint GetBehaviourFlags(int state);
	virtual void UpdatePostAnimEffects();
	virtual void SetState(int newState, int animType);
	virtual void ChangeManageState(int state);
	virtual void AnimEvaluate(uint param_2, edAnmMacroAnimator* pAnimator, uint newAnim);
	virtual bool CarriedByActor(CActor* pActor, edF32MATRIX4* m0);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);

	// CActorFighter
	virtual void ProcessDeath();

	void ClearLocalData();

	float field_0xa80;
	float field_0xa84;
	float field_0xa88;
	float field_0xa8c;

	S_TRAP_STREAM_REF* field_0xa90;

	CVibrationDyn vibrationDyn;

	CCamFigData camFigData;

	float field_0xee0;
	int field_0xee4;

	S_STREAM_REF<CActor> field_0xa94;
	S_STREAM_REF<CActor> field_0xa98;

	CBehaviourPunchingBallStand behaviourStand;
};

#endif //ACTOR_PUNCHING_BALL_H