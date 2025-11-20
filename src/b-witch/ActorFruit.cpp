#include "ActorFruit.h"
#include "MemoryStream.h"
#include "ActorProjectile.h"
#include "TimeController.h"
#include "BehaviourInventory.h"
#include "EventManager.h"
#include "MathOps.h"

void CActorFruit::Create(ByteCode* pByteCode)
{
	CActorAutonomous::Create(pByteCode);

	this->field_0x360 = pByteCode->GetU32();

	if (2.2f <= CScene::_pinstance->field_0x1c) {
		ntfSwitch.Create(pByteCode);
	}
	else {
		ntfSwitchExList.Create(pByteCode);
	}

	this->field_0x374[0] = (ed_zone_3d*)0x0;
	this->field_0x374[1] = (ed_zone_3d*)0x0;
	this->field_0x374[2] = (ed_zone_3d*)0x0;

	return;
}

void CActorFruit::Init()
{
	CActorAutonomous::Init();

	this->field_0x36c = 0.0f;
	this->field_0x380 = 0;
	this->field_0x370 = 0.0f;

	if (2.2f <= CScene::_pinstance->field_0x1c) {
		this->ntfSwitch.Init();
	}
	else {
		this->ntfSwitchExList.Init();
	}

	return;
}

void CActorFruit::Reset()
{
	CActorAutonomous::Reset();

	this->field_0x36c = 0.0f;
	this->field_0x380 = 0;
	this->field_0x370 = 0.0f;

	if (2.2f <= CScene::_pinstance->field_0x1c) {
		this->ntfSwitch.Reset(this);
	}
	else {
		this->ntfSwitchExList.Reset(this);
	}

	return;
}

CBehaviour* CActorFruit::BuildBehaviour(int behaviourType)
{
	CBehaviour* pBehaviour;

	if (behaviourType == FRUIT_BEHAVIOUR_PROJECTILE) {
		pBehaviour = new CBehaviourProjectileNew;
	}
	else {
		if (behaviourType == FRUIT_BEHAVIOUR_BONUS) {
			pBehaviour = new CBehaviourFruitBonus;
		}
		else {
			if (behaviourType == FRUIT_BEHAVIOUR_INVENTORY) {
				pBehaviour = new CBehaviourInventory;
			}
			else {
				if (behaviourType == FRUIT_BEHAVIOUR_SQUASHED) {
					pBehaviour = &this->behaviourSquashed;
				}
				else {
					if (behaviourType == FRUIT_BEHAVIOUR_SUSPENDED) {
						pBehaviour = &this->behaviourSuspended;
					}
					else {
						pBehaviour = CActorAutonomous::BuildBehaviour(behaviourType);
					}
				}
			}
		}
	}

	return pBehaviour;
}

StateConfig CActorFruit::_gStateCfg_FRT[12] = {
	StateConfig(0x0, 0x0),
	StateConfig(0xc, 0x4),
	StateConfig(0xd, 0x4),
	StateConfig(0xe, 0x0),
	StateConfig(0x0, 0x800),
	StateConfig(0x0, 0x0),
	StateConfig(0x0, 0x900),
	StateConfig(0x9, 0x100),
	StateConfig(0x9, 0x900),
	StateConfig(0xf, 0x0),
	StateConfig(0xf, 0x800),
	StateConfig(0x0, 0x0),
};

StateConfig* CActorFruit::GetStateCfg(int state)
{
	if (state < 6) {
		return CActorAutonomous::GetStateCfg(state);
	}
	else {
		return &_gStateCfg_FRT[state - 6];
	}
}

void CActorFruit::AnimEvaluate(uint layerId, edAnmMacroAnimator* pAnimator, uint newAnim)
{
	char* pBase = (char*)pAnimator->pAnimKeyTableEntry;
	AnimKeySomething* pValue = (AnimKeySomething*)(pBase + pAnimator->pAnimKeyTableEntry->keyIndex_0x8.asKey * 4);

	if (newAnim == 0xf) {
		float fValue = this->field_0x36c;

		if (0.0f <= fValue) {
			pValue->field_0xc = fValue;
			pValue->field_0x10 = (1.0f - pValue->field_0xc);
			pValue->field_0x14 = 0.0f;
		}
		else {
			pValue->field_0x14 = -fValue;
			pValue->field_0x10 = (1.0f - pValue->field_0x14);
			pValue->field_0xc = 0.0f;
		}
	}
	else {
		CActor::AnimEvaluate(layerId, pAnimator, newAnim);
	}

	return;
}

void CActorFruit::Func_0xd4(ed_zone_3d* param_2)
{
	ed_zone_3d** pCVar1;
	int iVar2;

	iVar2 = 0;
	pCVar1 = this->field_0x374;
	do {
		if (*pCVar1 == 0) {
			this->field_0x374[iVar2] = param_2;
			return;
		}

		iVar2 = iVar2 + 1;
		pCVar1 = pCVar1 + 1;
	} while (iVar2 < 3);

	return;
}

void CActorFruit::Func_0xd8(ed_zone_3d* param_2)
{
	int iVar1;

	iVar1 = 0;
	do {
		if (this->field_0x374[iVar1] == param_2) {
			return;
		}
		iVar1 = iVar1 + 1;
	} while (iVar1 < 3);

	return;
}

CInventoryInfo* CActorFruit::GetInventoryInfo()
{
	CBehaviourInventory* pBehaviourInventory;
	CInventoryInfo* pInfo;

	pBehaviourInventory = (CBehaviourInventory*)CActor::GetBehaviour(FRUIT_BEHAVIOUR_INVENTORY);

	if (pBehaviourInventory == (CBehaviourInventory*)0x0) {
		pInfo = (CInventoryInfo*)0x0;
	}
	else {
		pInfo = &pBehaviourInventory->inventoryInfo;
	}

	return pInfo;
}

int CActorFruit::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	byte bVar1;
	StateConfig* pSVar2;
	uint uVar3;
	int iVar5;

	if (msg == 0x35) {
		if ((this->field_0x360 & 1) != 0) {
			if ((GetStateFlags(this->actorState) & 0x800) != 0) {
				MessageSoccerParamsDetailed* pSoccerParams = (MessageSoccerParamsDetailed*)pMsgParam;
				if (pSoccerParams->field_0x0 == 0) {
					this->rotationEuler.y = pSoccerParams->rotation;
					SetVectorFromAngles(&this->rotationQuat, &this->rotationEuler.xyz);
					this->dynamic.rotationQuat = this->rotationQuat;
					this->dynamic.speed = pSoccerParams->speed;

					if (((this->pCollisionData)->flags_0x4 & 2) == 0) {
						SetBehaviour(FRUIT_BEHAVIOUR_SQUASHED, 0x10, -1);
					}
					else {
						if (this->currentAnimType == 10) {
							SetBehaviour(FRUIT_BEHAVIOUR_SQUASHED, 0xe, 0);
						}
						else {
							SetBehaviour(FRUIT_BEHAVIOUR_SQUASHED, 0xe, -1);
						}
					}
				}
				else {
					bVar1 = this->state_0x10;
					if (bVar1 != 0) {
						this->field_0x380 = (int)(char)bVar1;
						this->flags = this->flags | 2;
						this->flags = this->flags & 0xfffffffe;
					}

					this->field_0x368 = static_cast<CActorMovable*>(pSender);
					SetBehaviour(FRUIT_BEHAVIOUR_SQUASHED, 0xc, -1);
				}

				return 1;
			}
		}

		if ((((this->field_0x360 & 1) == 0) || (this->actorState != 0xd)) || (*reinterpret_cast<int*>(pMsgParam) != 0)) {
			iVar5 = 0;
		}
		else {
			iVar5 = 1;
			this->field_0x370 = this->field_0x370 + GetTimer()->cutsceneDeltaTime;
		}
	}
	else {
		if (msg == 0x22) {
			if ((pSender->pAnimationController != (CAnimation*)0x0) && ((this->field_0x360 & 1) != 0)) {
				if ((GetStateFlags(this->actorState) & 0x800) != 0) {
					this->field_0x364 = static_cast<CActorMovable*>(pSender);
					SetBehaviour(FRUIT_BEHAVIOUR_SQUASHED, 0xd, -1);

					return 1;
				}
			}
			iVar5 = 0;
		}
		else {
			iVar5 = CActorAutonomous::InterpretMessage(pSender, msg, pMsgParam);
		}
	}

	return iVar5;
}

int CActorFruit::InterpretEvent(edCEventMessage* pEventMessage, undefined8 param_3, int param_4, uint* param_5)
{
	int iVar1;

	if (*param_5 == 1) {
		if (param_3 == 2) {
			SetBehaviour(FRUIT_BEHAVIOUR_SQUASHED, 6, -1);
		}
		iVar1 = 1;
	}
	else {
		iVar1 = CActor::InterpretEvent(pEventMessage, param_3, param_4, param_5);
	}

	return iVar1;
}

void CActorFruit::BehaviourFruitSquashed_InitState(int newState)
{
	CCollision* pCVar2;

	if (newState == 0xd) {
		byte bVar1 = this->state_0x10;
		if (bVar1 != 0) {
			this->field_0x380 = bVar1;
			this->flags = this->flags | 2;
			this->flags = this->flags & 0xfffffffe;
		}
	}
	else {
		if (newState == 6) {
			pCVar2 = this->pCollisionData;
			pCVar2->flags_0x0 = pCVar2->flags_0x0 | 0x2000;
			pCVar2 = this->pCollisionData;
			pCVar2->flags_0x0 = pCVar2->flags_0x0 & 0xfffffffd;
		}
		else {
			if (newState == 0xb) {
				if (2.2f <= CScene::_pinstance->field_0x1c) {
					this->ntfSwitch.Switch(this);
				}
				else {
					this->ntfSwitchExList.Switch(this, 0);
				}

				pCVar2 = this->pCollisionData;
				pCVar2->flags_0x0 = pCVar2->flags_0x0 | 0x2000;
				pCVar2 = this->pCollisionData;
				pCVar2->flags_0x0 = pCVar2->flags_0x0 & 0xfffffffd;
			}
			else {
				if ((newState == 0xf) || (newState == 10)) {
					this->dynamic.speed = 0.0f;
				}
				else {
					if (newState == 9) {
						pCVar2 = this->pCollisionData;
						pCVar2->flags_0x0 = pCVar2->flags_0x0 | 0x2000;
						pCVar2 = this->pCollisionData;
						pCVar2->flags_0x0 = pCVar2->flags_0x0 & 0xfffffffd;
						this->dynamicExt.normalizedTranslation.x = 0.0f;
						this->dynamicExt.normalizedTranslation.y = 0.0f;
						this->dynamicExt.normalizedTranslation.z = 0.0f;
						this->dynamicExt.normalizedTranslation.w = 0.0f;
						this->dynamicExt.field_0x6c = 0.0f;
						this->dynamic.speed = 0.0f;
					}
				}
			}
		}
	}
	return;
}

void CActorFruit::BehaviourFruitSquashed_Manage(CBehaviourFruitSquashed* pBehaviour)
{
	ed_zone_3d* iVar1;
	CCollision* pCVar2;
	bool bVar3;
	ed_zone_3d** pCVar4;
	CEventManager* pEventManager;
	StateConfig* pSVar6;
	uint uVar7;
	CBehaviour* pCVar8;
	float fVar10;

	if ((this->flags & 0x2000001) == 0) {
		uVar7 = GetStateFlags(this->actorState) & 1;
		pEventManager = CScene::ptable.g_EventManager_006f5080;
		if (((uVar7 == 0) && (this->field_0x380 != 0)) && (this->state_0x10 == 0)) {
			iVar1 = this->field_0x374[0];
			pCVar4 = this->field_0x374;

			while (iVar1 != 0) {
				uVar7 = edEventComputeZoneAgainstVertex(pEventManager->activeChunkId, *pCVar4, &this->currentLocation, 0);
				if ((uVar7 & 1) != 0) {
					bVar3 = true;
					goto LAB_001de5d8;
				}

				iVar1 = pCVar4[1];
				pCVar4 = pCVar4 + 1;
			}

			bVar3 = false;
		LAB_001de5d8:
			if (bVar3) {
				this->flags = this->flags & 0xfffffffc;
			}
			else {
				SetBehaviour(FRUIT_BEHAVIOUR_SQUASHED, 6, -1);
			}
		}
	}

	switch (this->actorState) {
	case 6:
		this->flags = this->flags | 0x8000;
		this->flags = this->flags & 0xffffff7f;
		this->flags = this->flags | 0x20;

		EvaluateDisplayState();

		if (pBehaviour->field_0x8 < this->timeInAir) {
			this->flags = this->flags & 0xfffffffc;
			this->field_0x380 = 0;
			this->flags = this->flags & 0xffffff5f;
			EvaluateDisplayState();
			SetBehaviour(-1, -1, -1);
			UpdatePosition(&this->baseLocation, true);
			pCVar8 = GetBehaviour(FRUIT_BEHAVIOUR_SUSPENDED);
			if (pCVar8 == (CBehaviour*)0x0) {
				SetBehaviour(FRUIT_BEHAVIOUR_SQUASHED, -1, -1);
			}
			else {
				SetBehaviour(FRUIT_BEHAVIOUR_SUSPENDED, -1, -1);
			}
		}
		break;
	case 9:
		ManageDyn(4.0f, 0x400, (CActorsTable*)0x0);
		break;
	case 10:
		pCVar2 = this->pCollisionData;
		this->dynamicExt.normalizedTranslation.x = 0.0f;
		this->dynamicExt.normalizedTranslation.y = 0.0f;
		this->dynamicExt.normalizedTranslation.z = 0.0f;
		this->dynamicExt.normalizedTranslation.w = 0.0f;
		this->dynamicExt.field_0x6c = 0.0f;

		ManageDyn(4.0f, 0x100a023b, (CActorsTable*)0x0);

		if ((pCVar2->flags_0x4 & 2) == 0) {
			if (0.2f < this->timeInAir) {
				SetState(0x10, -1);
			}
		}
		else {
			this->timeInAir = 0.0f;
		}
		break;
	case 0xb:
		this->flags = this->flags & 0xffffff7f;
		this->flags = this->flags | 0x20;
		EvaluateDisplayState();
		this->dynamicExt.normalizedTranslation.x = 0.0f;
		this->dynamicExt.normalizedTranslation.y = 0.0f;
		this->dynamicExt.normalizedTranslation.z = 0.0f;
		this->dynamicExt.normalizedTranslation.w = 0.0f;
		this->dynamicExt.field_0x6c = 0.0f;
		this->dynamic.speed = 0.0f;
		ManageDyn(4.0f, 0x400, (CActorsTable*)0x0);
		break;
	case 0xc:
		StateAutSoccer(4.0f, 1, 9, this->field_0x368);
		break;
	case 0xd:
		StateAutSoccer(0.0f, 0, 0, this->field_0x364);

		if (0.0f < this->field_0x370) {
			fVar10 = this->field_0x370 + GetTimer()->cutsceneDeltaTime;
			this->field_0x370 = fVar10;

			if (0.06f < fVar10) {
				SetState(0xf, -1);
			}
		}
		break;
	case 0xe:
		StateAutRollOnGround(8.0f, 2.0f, 0.3f, 10, 0x10);
		break;
	case 0xf:
		FUN_001de2f0(0.2f, 0x10);
		break;
	case 0x10:
		FUN_001de2f0(0.0f, -1);
	}

	if (2.2f <= CScene::_pinstance->field_0x1c) {
		this->ntfSwitch.pStreamEventCamera->Manage(this);
	}
	else {
		this->ntfSwitchExList.pStreamEventCamera->Manage(this);
	}

	return;
}

void CActorFruit::BehaviourFruitSquashed_TermState(int oldState)
{
	if (oldState == 0xf) {
		this->field_0x370 = 0;
	}
	else {
		if ((oldState == 6) || (oldState == 9)) {
			CCollision* pCol = this->pCollisionData;
			pCol->flags_0x0 = pCol->flags_0x0 & 0xffffdfff;
			pCol = this->pCollisionData;
			pCol->flags_0x0 = pCol->flags_0x0 | 2;
		}
	}

	return;
}

void CActorFruit::FUN_001de2f0(float param_1, int nextState)
{
	CCollision* pCol;
	float value;

	pCol = this->pCollisionData;
	value = fabs(this->dynamic.linearAcceleration * this->dynamic.velocityDirectionEuler.y);
	this->field_0x36c = edFIntervalUnitDstLERP(value, 10.0f, 0.0f);
	ManageDyn(4.0f, 0x41920, (CActorsTable*)0x0);

	if ((pCol->flags_0x4 & 2) != 0) {
		if ((this->field_0x360 & 1) == 0) {
			SetState(9, -1);
			return;
		}

		if (6.0f < value) {
			this->field_0x36c = -edFIntervalUnitDstLERP(value, 0.0f, 10.0f) * 0.5f;
		}
		else {
			PlayAnim(0);
		}

		if (value < 1.0f) {
			this->field_0x36c = 0.0f;
			SetState(10, -1);
			return;
		}
	}

	if ((nextState != -1) && (param_1 < this->timeInAir)) {
		SetState(nextState, -1);
	}

	return;
}

void CBehaviourFruit::Create(ByteCode* pByteCode)
{
	return;
}

void CBehaviourFruit::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	this->pOwner = (CActorFruit*)pOwner;

	return;
}

void CBehaviourFruit::End(int newBehaviourId)
{
	return;
}

void CBehaviourFruitSuspended::Create(ByteCode* pByteCode)
{
	return;
}

void CBehaviourFruitSuspended::Manage()
{
	CActorFruit* pFruit;

	pFruit = this->pOwner;
	if (pFruit->actorState == 8) {
		pFruit->ManageDyn(4.0f, 0x400, (CActorsTable*)0x0);
	}
	else {
		if (pFruit->actorState == 7) {
			pFruit->ManageDyn(4.0f, 0x400, (CActorsTable*)0x0);

			if (pFruit->pAnimationController->IsCurrentLayerAnimEndReached(0)) {
				pFruit->SetState(8, -1);
			}
		}
	}

	return;
}

void CBehaviourFruitSuspended::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	CActorFruit* pFruit;

	this->pOwner = static_cast<CActorFruit*>(pOwner);
	if (newState == -1) {
		this->pOwner->SetState(7, -1);
	}
	else {
		this->pOwner->SetState(newState, newAnimationType);
	}

	this->pOwner->flags = this->pOwner->flags | 0x8000;
	this->pOwner->dynamic.speed = 0.0f;
	pFruit = this->pOwner;
	pFruit->dynamicExt.normalizedTranslation.x = 0.0f;
	pFruit->dynamicExt.normalizedTranslation.y = 0.0f;
	pFruit->dynamicExt.normalizedTranslation.z = 0.0f;
	pFruit->dynamicExt.normalizedTranslation.w = 0.0f;
	pFruit->dynamicExt.field_0x6c = 0.0f;

	return;
}

void CBehaviourFruitSuspended::End(int newBehaviourId)
{
	this->pOwner->flags = this->pOwner->flags & 0xffff7fff;

	return;
}

void CBehaviourFruitSuspended::InitState(int newState)
{
	return;
}

void CBehaviourFruitSuspended::TermState(int oldState, int newState)
{
	return;
}

int CBehaviourFruitSuspended::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	CBehaviour* pCVar1;
	int iVar2;

	if ((msg == 0xe) || (msg == 0xf)) {
		if (this->pOwner->GetBehaviour(FRUIT_BEHAVIOUR_SQUASHED) != (CBehaviour*)0x0) {
			this->pOwner->SetBehaviour(FRUIT_BEHAVIOUR_SQUASHED, 0x10, -1);
		}
		iVar2 = 1;
	}
	else {
		/* WARNING: Load size is inaccurate */
		if (((msg == 2) && (pCVar1 = this->pOwner->GetBehaviour(FRUIT_BEHAVIOUR_SQUASHED), pCVar1 != (CBehaviour*)0x0)) &&
			(reinterpret_cast<_msg_hit_param*>(pMsgParam)->projectileType == 4)) {
			this->pOwner->SetBehaviour(FRUIT_BEHAVIOUR_SQUASHED, 0x10, -1);
			iVar2 = 1;
		}
		else {
			iVar2 = 0;
		}
	}

	return iVar2;
}

void CBehaviourFruitBonus::Create(ByteCode* pByteCode)
{
	this->actorRef.index = pByteCode->GetS32();
	this->initialBehaviourId = pByteCode->GetS32();

	return;
}

void CBehaviourFruitBonus::Init(CActor* pOwner)
{
	this->actorRef.Init();

	return;
}

void CBehaviourFruitBonus::Manage()
{
	return;
}

void CBehaviourFruitBonus::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	int purchaseId;
	CActor* pReceiver;
	CBehaviourInventory* pBehaviourInventory;

	CBehaviourFruit::Begin(pOwner, newState, newAnimationType);

	pBehaviourInventory = (CBehaviourInventory*)this->pOwner->GetBehaviour(FRUIT_BEHAVIOUR_INVENTORY);
	purchaseId = (pBehaviourInventory->inventoryInfo).purchaseId;
	pReceiver = (this->actorRef).Get();
	if (purchaseId == 0x1f) {
		this->pOwner->DoMessage(pReceiver, (ACTOR_MESSAGE)0x84, (MSG_PARAM)0x1e);
	}
	else {
		if (purchaseId == 0x1e) {
			this->pOwner->DoMessage(pReceiver, (ACTOR_MESSAGE)10, (MSG_PARAM)8);
		}
		else {
			if (purchaseId == 0x1d) {
				CActorAutonomous* pAutonomous = static_cast<CActorAutonomous*>(pReceiver);
				pAutonomous->LifeRestore();
			}
		}
	}
	
	this->pOwner->SetBehaviour(this->initialBehaviourId, -1, -1);

	return;
}

void CBehaviourFruitSquashed::Create(ByteCode* pByteCode)
{
	this->field_0x8 = pByteCode->GetF32();

	return;
}

void CBehaviourFruitSquashed::Manage()
{
	this->pOwner->BehaviourFruitSquashed_Manage(this);

	return;
}

void CBehaviourFruitSquashed::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	CBehaviourFruit::Begin(pOwner, newState, newAnimationType);

	if (newState == -1) {
		this->pOwner->SetState(10, -1);
	}
	else {
		this->pOwner->SetState(newState, newAnimationType);
	}

	this->pOwner->flags = this->pOwner->flags & 0xffff7fff;
	this->pOwner->field_0x370 = 0;

	return;
}

void CBehaviourFruitSquashed::InitState(int newState)
{
	this->pOwner->BehaviourFruitSquashed_InitState(newState);

	return;
}

void CBehaviourFruitSquashed::TermState(int oldState, int newState)
{
	this->pOwner->BehaviourFruitSquashed_TermState(oldState);

	return;
}

int CBehaviourFruitSquashed::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	int iVar1;
	float fVar2;
	float fVar3;
	Timer* pTVar4;
	uint uVar5;
	StateConfig* pSVar6;
	edF32VECTOR4* v0;
	float fVar7;
	edF32VECTOR4 eStack48;
	edF32VECTOR4 local_20;
	edF32VECTOR4 eStack16;
	CActorFruit* pFruit;

	if (msg == 3) {
		this->pOwner->SetState(6, -1);
	}
	else {
		if (msg == 0x21) {
			this->pOwner->SetState(6, -1);

			return 1;
		}

		if (msg == 0x20) {
			this->pOwner->flags = this->pOwner->flags | 0x8000;
			this->pOwner->SetState(0xb, -1);

			return 1;
		}

		if (msg == 0x12) {
			pFruit = this->pOwner;

			if ((pFruit->field_0x360& 1) != 0) {
				uVar5 = pFruit->GetStateFlags(pFruit->actorState);
				if ((uVar5 & 0x800) != 0) {
					uVar5 = pFruit->GetStateFlags(pFruit->actorState);
					if (((uVar5 & 0x1000) != 0) || ((fVar7 = pSender->SV_GetCosAngle2D(&this->pOwner->currentLocation),
						0.64265704f < fVar7 &&
							(pFruit = this->pOwner,
								fVar7 = (pSender->currentLocation).x - pFruit->currentLocation.x,
								fVar2 = (pSender->currentLocation).y - pFruit->currentLocation.y,
								fVar3 = (pSender->currentLocation).z - pFruit->currentLocation.z,
								fVar7 * fVar7 + fVar2 * fVar2 + fVar3 * fVar3 < 1.21f)))) {
						return 2;
					}
				}
			}

			return 0;
		}

		if (msg == MESSAGE_KICKED) {
			_msg_hit_param* pHitParam = (_msg_hit_param*)pMsgParam;
			pFruit = this->pOwner;

			if ((pFruit->field_0x360 & 1) != 0 && (pHitParam->projectileType == 3) && (pFruit->actorState == 0xd)) {
				pFruit->SetState(0xf, -1);
				local_20.x = this->pOwner->currentLocation.x;
				local_20.z = this->pOwner->currentLocation.z;
				local_20.w = this->pOwner->currentLocation.w;
				local_20.y = (pHitParam->field_0x40).y;
				this->pOwner->UpdatePosition(&local_20, true);
				edF32Vector4ScaleHard(pHitParam->field_0x30, &eStack16, &pHitParam->field_0x20);
				pFruit = this->pOwner;
				edF32Vector4ScaleHard(0.02f / GetTimer()->cutsceneDeltaTime, &eStack48, &eStack16);
				v0 = pFruit->dynamicExt.aImpulseVelocities;
				edF32Vector4AddHard(v0, v0, &eStack48);
				pFruit->dynamicExt.aImpulseVelocityMagnitudes[0] = edF32Vector4GetDistHard(pFruit->dynamicExt.aImpulseVelocities);

				return 1;
			}

			if (pHitParam->projectileType == 10) {
				this->pOwner->SetState(6, -1);
				return 1;
			}
		}
		else {
			GetPositionMsgParams* pGetPointParams = reinterpret_cast<GetPositionMsgParams*>(pMsgParam);
			if ((msg == MESSAGE_GET_VISUAL_DETECTION_POINT) && (pGetPointParams->field_0x0 == 0)) {
				(pGetPointParams->vectorFieldB).x = 0.0f;
				(pGetPointParams->vectorFieldB).y = 0.1f;
				(pGetPointParams->vectorFieldB).z = 0.0f;
				(pGetPointParams->vectorFieldB).w = 0.0f;
				return 1;
			}
		}
	}
	return 0;
}
