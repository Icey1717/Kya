#ifndef _ACTOR_FIGHTER_H
#define _ACTOR_FIGHTER_H

#include "Types.h"
#include "Actor.h"
#include "ActorAutonomous.h"


class CActorFighter : public CActorAutonomous
{
public:
	CActorFighter() {
		IMPLEMENTATION_GUARD_LOG()
	}

	virtual bool IsKindOfObject(ulong kind);
	virtual void Init();
	virtual CBehaviour* BuildBehaviour(int behaviourType);
	virtual void Create(ByteCode* pByteCode);
	virtual bool CarriedByActor(CActor* pActor, edF32MATRIX4* m0);
	virtual int InterpretMessage(CActor* pSender, int msg, void* pMsgParam);
	virtual int ReceiveMessage(CActor* pSender, ACTOR_MESSAGE msg, MSG_PARAM pMsgParam);
	virtual void TieToActor(CActor* pTieActor, int carryMethod, int param_4, edF32MATRIX4* param_5);

	// CActorAutonomous
	virtual void ChangeCollisionSphere(float param_1, edF32VECTOR4* param_3, edF32VECTOR4* param_4);
	virtual void UpdateCollisionSphere();

	virtual bool IsFightRelated(int param_2);

	virtual void AnimEvaluate(uint param_2, edAnmMacroAnimator* pAnimator, uint newAnim);
	void ClearLocalData();

	CActor* field_0x350;
	CActor* field_0x354;

	edF32VECTOR4 field_0x400;
	edF32VECTOR4 field_0x410;

	CScalarDyn field_0x37c;
	CScalarDyn field_0x3a4;
	CScalarDyn scalarDynJump;
};

class CBehaviourFighter : public CBehaviour
{
	virtual void Init(CActor* pOwner);

	CActor* pOwner;
	int field_0x8;
};

class CBehaviourFighterSlave : public CBehaviourFighter
{

};

class CBehaviourFighterRidden : public CBehaviourFighter
{

};

class CBehaviourFighterProjected : public CBehaviourFighter
{

};

#endif // _ACTOR_FIGHTER_H
