#include "ActorHeroPrivate.h"
#include "MemoryStream.h"
#include "MathOps.h"
#include "TimeController.h"
#include "LevelScheduleManager.h"

void CActorHeroPrivate::Create(ByteCode* pByteCode)
{
	SkipToNextActor(pByteCode);

	if (CActorHero::_gThis == (CActorHero*)0x0) {
		CActorHero::_gThis = this;
	}
	this->field_0x1610 = 1;
	this->field_0x18dc = 1;
	this->field_0xee0 = 0;
}

void CActorHeroPrivate::Init()
{
	this->field_0xee0 = 1;
}

CBehaviour* CActorHeroPrivate::BuildBehaviour(int behaviourType)
{
	CBehaviour* pNewBehaviour;

	if (behaviourType == 0xb) {
		pNewBehaviour = &this->behaviour_0x1fd0;
	}
	else {
		if (behaviourType == 10) {
			pNewBehaviour = &this->behaviour_0x1e10;
		}
		else {
			if (behaviourType == 9) {
				pNewBehaviour = &this->behaviour_0x1c50;
			}
			else {
				if (behaviourType == 8) {
					pNewBehaviour = new CBehaviourHeroRideJamGut;
				}
				else {
					if (behaviourType == 7) {
						pNewBehaviour = &this->behaviourHeroDefault;
					}
					else {
						pNewBehaviour = CActorFighter::BuildBehaviour(behaviourType);
					}
				}
			}
		}
	}
	return pNewBehaviour;
}

void CActorHeroPrivate::SetState(int newState, int animType)
{
	uint uVar1;
	int iVar2;
	AnimResult* piVar3;
	Timer* pTVar4;
	undefined8 uVar5;
	ulong uVar6;
	bool bFightRelated;
	float fVar8;
	EActorState currentState;

	/* For stand to jump the type or mode is 0x78
	   For begin jump the type or mode is 0x79
	   Initial anim is -1 */
	currentState = this->actorState;
	uVar1 = TestState_IsInHit(0xffffffff);
	if (uVar1 != 0) {
		IMPLEMENTATION_GUARD(
		uVar5 = (**(code**)((int)(this->base.base.pVTable + 1) + 0x40))(this);
		fVar8 = (float)(**(code**)(*(int*)uVar5 + 0x24))(uVar5);
		if ((fVar8 <= 0.0) || (0.0 < *(float*)&this->field_0x2e4)) {
			newState = ChooseStateDead(this, 0xc, 4, 1);
			animType = AT_None;
		})
	}

	if ((currentState != newState) &&
		(uVar6 = GetBehaviourFlags(this->curBehaviourId), (uVar6 & 0x200) != 0)) {
		GetStateHeroFlags(newState);
		//::EmptyFunction();
	}

	bFightRelated = this->IsFightRelated(this->curBehaviourId);

	if ((bFightRelated != false) && (newState != AS_None)) {
		IMPLEMENTATION_GUARD(
		animType = ActorStateFunc_00327470((Actor*)this, newState, animType);)
	}

	uVar1 = GetStateHeroFlags(newState);
	uVar1 = TestState_IsOnAToboggan(uVar1);
	if (((uVar1 != 0) && (iVar2 = LevelScheduleManager::ScenVar_Get(0x10), 0 < iVar2)) &&
		(newState != 0xef)) {
		piVar3 = GetStateCfg(newState);
		animType = piVar3->field_0x0 + 0xa;
	}

	CActorMovable::SetState(newState, animType);

	if (currentState != newState) {
		pTVar4 = GetTimer();
		this->time_0x1538 = pTVar4->scaledTotalTime;
		pTVar4 = GetTimer();
		this->time_0x153c = pTVar4->scaledTotalTime;

		uVar6 =GetBehaviourFlags(this->curBehaviourId);
		if ((uVar6 & 0x200) != 0) {
			//::EmptyFunction();
		}
	}

	uVar1 = GetStateHeroFlags(this->actorState);
	this->heroFlags = uVar1;
	return;
}

void CActorHeroPrivate::CinematicMode_Leave(int behaviourId)
{
	CBehaviour* pCVar1;
	undefined8 uVar2;
	CActor* pOtherActor;
	edF32VECTOR4 local_30;
	undefined4 local_18;
	CActorHeroPrivate* local_14;
	CActor* local_10;
	undefined4 local_8;
	undefined4 local_4;

	pOtherActor = (CActor*)0x0;
	if (behaviourId == 8) {
		IMPLEMENTATION_GUARD(
		CActor::GetBehaviour((CActor*)this, 8);
		LevelScheduleManager::ScenVar_Get(0xd);
		local_18 = 0x42c80000;
		local_10 = (CActor*)0x0;
		local_30.x = (float)this->pCollisionData;
		local_30.y = *(float*)&this->currentLocation;
		local_30.z = *(float*)((int)&this->currentLocation + 4);
		local_30.w = 10.0;
		local_14 = this;
		KyaVectorFunc(&(CScene::ptable.g_ActorManager_004516a4)->cluster, &local_30, &LAB_0033d920, &local_18);
		pOtherActor = local_10;
		local_4 = 0;
		uVar2 = CActor::DoMessage((CActor*)this, local_10, 0x4d, (ActorCompareStruct*)0x0);
		*(int*)&this->field_0x15a0 = (int)uVar2;
		local_8 = 0;
		CActor::DoMessage((CActor*)this, pOtherActor, 0x14, (ActorCompareStruct*)0x0);)
	}
	CActor::CinematicMode_Leave(behaviourId);
	if (behaviourId == 8) {
		IMPLEMENTATION_GUARD(
		pCVar1 = CActor::GetBehaviour((CActor*)this, (int)this->aComponents);
		(*(code*)pCVar1->pVTable[1].Init)(pCVar1, pOtherActor, *(undefined4*)&this->field_0x15a0, 7, 0x11d);
		(*((this->pVTable)->actorBase).SetState)((CActor*)this, 0x11e, -1);)
	}
	IMPLEMENTATION_GUARD_LOG(
	this->field_0x19b1 = 1;
	this->field_0xffc = 0.0f;)
	return;
}

int CActorHeroPrivate::StateEvaluate()
{
	//InputManager* pIVar1;
	int iVar2;
	bool bVar3;
	int iVar4;
	uint uVar5;
	//AnimResult* pAVar6;
	long lVar7;
	undefined8 uVar8;
	edF32VECTOR4* peVar9;
	float fVar10;
	edF32VECTOR4 local_10;

	if ((this->field_0xee0 == 0) && (this->prevBehaviourId != 1)) {
		IMPLEMENTATION_GUARD(
		fVar10 = this->dynamicExt.base.field_0x10.z;
		lVar7 = (**(code**)((int)(this->base.base.pVTable + 1) + 8))();
		peVar9 = (edF32VECTOR4*)0x0;
		if (lVar7 != 0) {
			iVar4 = (**(code**)((int)(this->base.base.pVTable + 1) + 8))(this);
			peVar9 = *(edF32VECTOR4**)(iVar4 + 0x2c);
		}
		iVar4 = GetPossibleWind(fVar10, this, (edF32VECTOR4*)&this->dynamicExt.base.field_0x40
			, peVar9);
		if (iVar4 == -1) {
			if ((*(byte*)((int)this->pCollisionData + 4) & 2) == 0) {
				bVar3 = InClimbZone((CActorHero*)this, &this->currentLocation);
				*(bool*)&this->field_0x1454 = bVar3;
				iVar4 = 0xa9;
				if (*(char*)&this->field_0x1454 == '\0') {
					iVar4 = ChooseStateFall((CActorHero*)this, 0);
				}
			}
			else {
				iVar4 = this->prevActorState;
				if (iVar4 == -1) {
					uVar5 = 0;
				}
				else {
					pAVar6 = (*(this->base.base.pVTable)->GetStateCfg)((CActor*)this, iVar4);
					uVar5 = pAVar6->flags_0x4 & 0x100;
				}
				if (uVar5 == 0) {
					iVar4 = ActorFunc_0014bdf0((float)this->base.dynamic.field_0x44 *
						this->base.dynamic.field_0x20.y, (CActorHero*)this);
					if (iVar4 == -1) {
						iVar4 = 0x73;
					}
				}
				else {
					pIVar1 = this->field_0x18d8;
					if ((pIVar1 == (InputManager*)0x0) || (*(int*)&this->field_0x18dc != 0)) {
						fVar10 = 0.0;
					}
					else {
						fVar10 = pIVar1->buttonArray[5].floatFieldB;
					}
					iVar4 = 0x85;
					if (fVar10 == 0.0) {
						if ((pIVar1 == (InputManager*)0x0) || (*(int*)&this->field_0x18dc != 0)) {
							fVar10 = 0.0;
						}
						else {
							fVar10 = pIVar1->field_0x5fc;
						}
						if (0.3 < fVar10) {
							if ((pIVar1 == (InputManager*)0x0) || (*(int*)&this->field_0x18dc != 0)) {
								fVar10 = 0.0;
							}
							else {
								fVar10 = pIVar1->buttonArray[5].floatFieldB;
							}
							iVar4 = 0x86;
							if (fVar10 == 0.0) {
								if ((pIVar1 == (InputManager*)0x0) || (*(int*)&this->field_0x18dc != 0)) {
									fVar10 = 0.0;
								}
								else {
									fVar10 = pIVar1->field_0x5fc;
								}
								iVar4 = 0x77;
								if (0.9 <= fVar10) {
									iVar4 = 0x76;
								}
							}
						}
						else {
							iVar4 = 0x73;
						}
					}
				}
			}
		}
		(*(this->base.base.pVTable)->SetState)((CActor*)this, iVar4, -1);
		uVar8 = (**(code**)((int)(this->base.base.pVTable + 1) + 0x40))(this);
		fVar10 = (float)(**(code**)(*(int*)uVar8 + 0x24))(uVar8);
		bVar3 = fVar10 - *(float*)&this->field_0x2e4 <= 0.0;
		if (!bVar3) {
			iVar2 = this->actorState;
			if (iVar2 == -1) {
				uVar5 = 0;
			}
			else {
				pAVar6 = (*(this->base.base.pVTable)->GetStateCfg)((CActor*)this, iVar2);
				uVar5 = pAVar6->flags_0x4 & 1;
			}
			bVar3 = uVar5 != 0;
		}
		if (bVar3) {
			iVar4 = ActorStateFunc_0013f0c0((Actor*)this, 0xc, 4, 1);
			(*(this->base.base.pVTable)->SetState)((CActor*)this, iVar4, 0xffffffff);
		})
	}
	else {
		iVar4 = 0x73;
		SetState(0x73, -1);
		if ((this->prevBehaviourId == 1) &&
			((fVar10 = this->distanceToGround, 0.07f < fVar10 && (fVar10 < 0.3f))
				)) {

			local_10.x = this->currentLocation.x;
			local_10.z = this->currentLocation.z;
			local_10.w = this->currentLocation.w;
			local_10.y = this->currentLocation.y - this->distanceToGround;

			UpdatePosition(&local_10, true);
		}
	}
	return iVar4;
}

void CBehaviourHeroDefault::Manage()
{
	IMPLEMENTATION_GUARD();
}

void CBehaviourHeroDefault::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	CActorHeroPrivate* pCVar1;
	float fVar2;
	float in_f0;
	float fVar3;
	edF32VECTOR4 local_10;

	this->field_0xc = reinterpret_cast<CActorHeroPrivate*>(pOwner);
	pCVar1 = this->field_0xc;
	if ((pCVar1->flags & 0x1000) == 0) {
		pCVar1->rotationEuler.z = 0.0f;
		pCVar1->rotationEuler.x = 0.0f;

		local_10 = pCVar1->rotationQuat;
		local_10.y = 0.0f;
		edF32Vector4SafeNormalize1Hard(&local_10, &local_10);

		pCVar1->rotationQuat = local_10;
		in_f0 = local_10.w;
	}
	if (newState == -1) {
		this->field_0xc->StateEvaluate();
	}
	else {
		if (((newState - 0x78U < 2) || (newState - 0x7bU < 2)) || (newState == 0x11d)) {
			IMPLEMENTATION_GUARD(
			if (newState == 0x11d) {
				pCVar1 = this->field_0xc;
				fVar2 = *(float*)&(pCVar1->base).field_0xf54;
				fVar3 = (pCVar1->base).field_0x1154;
				if (fVar2 != 0.0) {
					in_f0 = 4.0;
					fVar3 = 4.0 - (*(float*)((int)&pCVar1->currentLocation + 4) -
						*(float*)((int)fVar2 + 0x34));
				}
				(**(GetAnimForState**)((int)((pCVar1->base).character.characterBase.base.base.pVTable + 1) + 0x54))
					((CActor*)pCVar1, (int)pOwner);
				ActorFloatFunc_00147a70
				(0.1, in_f0, *(float*)&(pCVar1->base).field_0x1158, (pCVar1->base).field_0x1150, fVar3, (Actor*)pCVar1, 1,
					(edF32VECTOR4*)0x0);
			}
			else {
				pCVar1 = this->field_0xc;
				(**(GetAnimForState**)((int)((pCVar1->base).character.characterBase.base.base.pVTable + 1) + 0x54))
					((CActor*)pCVar1, (int)pOwner);
				ActorFunc_001479b0(in_f0, pCVar1);
			})
		}
		this->field_0xc->SetState(newState, newAnimationType);
	}
	return;
}

void CBehaviourHeroDefault::End(int newBehaviourId)
{
	IMPLEMENTATION_GUARD();
}
