#include "ActorFighter.h"
#include "MathOps.h"

void CActorFighter::Init()
{
	CAnimation* pCVar1;
	int iVar2;
	float fVar3;

	IMPLEMENTATION_GUARD_LOG(
	S_STREAM_REF<CActor>::Init((S_STREAM_REF<CActor> *) & this->field_0x910);)
	CActorAutonomous::Init();
	this->flags = this->flags | 0x100000;

	IMPLEMENTATION_GUARD_LOG(
	_InitBlowsDB(this);
	FUN_00302b20(this);
	fVar3 = (float)_InitGrabsDB(this);
	pCVar1 = this->pAnimationController;
	iVar2 = CActor::GetIdMacroAnim((CActor*)this, New_Name_(41));
	if (iVar2 < 0) {
		fVar3 = 0.0;
		this->field_0x6c0 = 0.0;
	}
	else {
		CAnimation::GetAnimLength(pCVar1, iVar2, 1);
		this->field_0x6c0 = fVar3;
	}
	pCVar1 = this->pAnimationController;
	iVar2 = CActor::GetIdMacroAnim((CActor*)this, New_Name_(42));
	if (iVar2 < 0) {
		fVar3 = 0.0;
	}
	else {
		CAnimation::GetAnimLength(pCVar1, iVar2, 1);
	}
	fVar3 = this->field_0x6c0 + fVar3;
	this->field_0x6c0 = fVar3;
	pCVar1 = this->pAnimationController;
	iVar2 = CActor::GetIdMacroAnim((CActor*)this, New_Name_(43));
	if (iVar2 < 0) {
		fVar3 = 0.0;
	}
	else {
		CAnimation::GetAnimLength(pCVar1, iVar2, 1);
	}
	this->field_0x6c0 = this->field_0x6c0 + fVar3;)
	return;
}

CBehaviour* CActorFighter::BuildBehaviour(int behaviourType)
{
	CBehaviour* pNewBehaviour;

	if (behaviourType == 6) {
		pNewBehaviour = new CBehaviourFighterSlave;
	}
	else {
		if (behaviourType == 5) {
			pNewBehaviour = new CBehaviourFighterRidden;
		}
		else {
			if (behaviourType == 4) {
				pNewBehaviour = new CBehaviourFighterProjected;
			}
			else {
				if (behaviourType == 3) {
					pNewBehaviour = new CBehaviourFighter;
				}
				else {
					pNewBehaviour = CActorAutonomous::BuildBehaviour(behaviourType);
				}
			}
		}
	}
	return pNewBehaviour;
}

bool CActorFighter::IsFightRelated(int param_2)
{
	bool bVar1;

	if ((param_2 - 3U < 3) || (param_2 == 6)) {
		bVar1 = true;
	}
	else {
		bVar1 = false;
	}
	return bVar1;
}

void CActorFighter::ChangeCollisionSphere(float param_1, edF32VECTOR4* param_3, edF32VECTOR4* param_4)
{
	edF32VECTOR4* peVar1;
	float fVar2;
	float fVar3;
	float fVar4;
	edColPRIM_OBJECT* iVar1;

	CActorAutonomous::ChangeCollisionSphere(param_1, param_3, param_4);
	iVar1 = this->pCollisionData->pObbPrim;
	this->field_0x410 = iVar1->field_0xb0;
	fVar2 = edF32Vector4GetDistHard(&this->field_0x410);
	peVar1 = &this->field_0x400;
	edF32Vector4NormalizeHard(peVar1, peVar1);
	peVar1 = &this->field_0x400;
	edF32Vector4ScaleHard(fVar2, peVar1, peVar1);
	this->field_0x400.w = 1.0f;
	return;
}

void CBehaviourFighter::Init(CActor* pOwner)
{
	this->pOwner = pOwner;
	this->field_0x8 = 3;
	return;
}
