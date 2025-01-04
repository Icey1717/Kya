#include "ActorPunchingBall.h"
#include "MemoryStream.h"
#include "MathOps.h"
#include "CinematicManager.h"
#include "TimeController.h"
#include "ActorHero.h"

void CActorPunchingBall::Create(ByteCode* pByteCode)
{
	S_TRAP_STREAM_REF* pSVar1;
	CActor* pCVar2;
	float fVar3;
	edF32VECTOR4 local_20;
	edF32VECTOR4 local_10;

	CActorFighter::Create(pByteCode);
	this->field_0xa80 = pByteCode->GetF32();
	this->field_0xa84 = pByteCode->GetF32();
	this->field_0xa88 = pByteCode->GetF32();
	this->field_0xa8c = pByteCode->GetF32();

	pSVar1 = (S_TRAP_STREAM_REF*)pByteCode->currentSeekPos;
	pByteCode->currentSeekPos = (char*)pSVar1->aEntries;
	if (pSVar1->entryCount != 0) {
		pByteCode->currentSeekPos = pByteCode->currentSeekPos + pSVar1->entryCount * sizeof(S_TRAP_STREAM_ENTRY);
	}
	this->field_0xa90 = pSVar1;

	(this->field_0xa94).index = pByteCode->GetS32();
	(this->field_0xa98).index = pByteCode->GetS32();

	this->camFigData.Create(pByteCode);
	this->camFigData.IsValid(0);

	static edF32VECTOR4 edF32VECTOR4_00425990 = { 0.4f, 0.8f, 0.4f, 0.0f };
	static edF32VECTOR4 edF32VECTOR4_004259a0 = { 0.0f, 0.8f, 0.0f, 1.0f };

	local_10 = edF32VECTOR4_00425990;
	local_20 = edF32VECTOR4_004259a0;

	ChangeCollisionSphere(0.0f, &local_10, &local_20);
	StoreCollisionSphere();
	//(*(code*)((this->field_0xe50).pVTable)->setObjCounts)(&this->field_0xe50, 3, 2);
}

void CActorPunchingBall::Init()
{
	for (int i = 0; i < this->field_0xa90->entryCount; i++) {
		this->field_0xa90->aEntries[i].onoff.Init();
		this->field_0xa90->aEntries[i].eventCamera.Init();
	}

	this->field_0xa94.Init();
	this->field_0xa98.Init();

	CActorFighter::Init();

	//(*(code*)((this->field_0xe50).pVTable)->setupObjects)(&this->field_0xe50, this);

	ClearLocalData();

	this->field_0x504 = 0.0f;

	return;
}

void CActorPunchingBall::Term()
{
	CActorFighter::Term();

	//(*(code*)((this->field_0xe50).pVTable)->field_0x10)(&this->field_0xe50);

	this->field_0xa90 = (S_TRAP_STREAM_REF*)0x0;

	return;
}

void CActorPunchingBall::Manage()
{
	CAnimation* pCVar1;
	edAnmLayer* peVar2;
	bool bVar3;
	CLifeInterface* pCVar4;
	Timer* pTVar5;
	int iVar6;
	int iVar7;
	int iVar8;
	float fVar9;
	edF32MATRIX4 auStack144;
	edF32MATRIX4 eStack80;
	edF32VECTOR3 local_10;

	if ((((this->currentAnimType != 0x81) && (iVar8 = this->curBehaviourId, 2 < iVar8)) && (iVar8 < 7)) && ((iVar8 = this->actorState, iVar8 == 0x57 || (iVar8 == 0x60)))) {
		PlayAnim(0x81);
		SetVectorFromAngles(&this->rotationQuat, &this->rotationEuler.xyz);

		if (this->rotationQuat.y < 0.0f) {
			edF32Matrix4FromEulerSoft(&auStack144, &this->rotationEuler.xyz, "XYZ");
			edF32Matrix4BuildFromVectorAndAngle(3.141593f, &eStack80, &auStack144.rowY);
			edF32Matrix4MulF32Matrix4Hard(&eStack80, &auStack144, &eStack80);
			edF32Matrix4ToEulerSoft(&eStack80, &local_10, "XYZ");
			this->rotationEuler.xyz = local_10;
			this->field_0x7dc = this->field_0x7dc * -1.0f;
		}
	}

	if (this->currentAnimType == 0x81) {
		fVar9 = GetLifeInterface()->GetValue();
		if (fVar9 <= 0.0f) {
			pCVar1 = this->pAnimationController;
			peVar2 = (pCVar1->anmBinMetaAnimator).aAnimData;
			bVar3 = false;
			if ((peVar2->currentAnimDesc).animType == pCVar1->currentAnimType_0x30) {
				if (peVar2->animPlayState == 0) {
					bVar3 = false;
				}
				else {
					bVar3 = (peVar2->field_0xcc & 2) != 0;
				}
			}

			if (bVar3) {
				ProcessDeath();
			}
		}
	}

	CActorFighter::Manage();

	iVar8 = 0;
	if (this->field_0xa90 != (S_TRAP_STREAM_REF*)0x0) {
		iVar8 = this->field_0xa90->entryCount;
	}

	for (int i = 0; i < iVar8; i++) {
		this->field_0xa90->aEntries[i].eventCamera.Manage(this);
	}

	pTVar5 = GetTimer();
	fVar9 = this->field_0xee0 - pTVar5->cutsceneDeltaTime;
	this->field_0xee0 = fVar9;
	if (fVar9 < 0.0f) {
		this->field_0xee0 = 0.0f;
	}

	if ((CActorHero::_gThis != (CActorHero*)0x0) &&
		(bVar3 = CActorHero::_gThis->IsFightRelated(CActorHero::_gThis->curBehaviourId), bVar3 != false)) {
		this->field_0xee0 = 0.5f;
	}

	return;
}

void CActorPunchingBall::Reset()
{
	IMPLEMENTATION_GUARD();
}

CBehaviour* CActorPunchingBall::BuildBehaviour(int behaviourType)
{
	CBehaviour* pBehaviour;

	if (behaviourType == FIGHTER_BEHAVIOUR_SLAVE) {
		pBehaviour = new CBehaviourPunchingBallSlave;
	}
	else {
		if (behaviourType == FIGHTER_BEHAVIOUR_RIDDEN) {
			pBehaviour = new CBehaviourPunchingBallRidden;
		}
		else {
			if (behaviourType == FIGHTER_BEHAVIOUR_DEFAULT) {
				pBehaviour = new CBehaviourPunchingBallDefault;
			}
			else {
				if (behaviourType == PUNCHING_BALL_BEHAVIOUR_STAND) {
					pBehaviour = &this->behaviourStand;
				}
				else {
					pBehaviour = CActorFighter::BuildBehaviour(behaviourType);
				}
			}
		}
	}

	return pBehaviour;
}

StateConfig CActorPunchingBall::_gStateCfg_PBA[6] = {
	StateConfig(0x8b, 0x0),
	StateConfig(0x8b, 0x100),
	StateConfig(0x8b, 0x0),
	StateConfig(0x8b, 0x100),
	StateConfig(0x81, 0x100),
	StateConfig(0x8b, 0x100),
};

StateConfig* CActorPunchingBall::GetStateCfg(int state)
{
	StateConfig* pSVar1;

	if (state < 0x72) {
		pSVar1 = CActorFighter::GetStateCfg(state);
	}
	else {
		pSVar1 = _gStateCfg_PBA + state + -0x72;
	}

	return pSVar1;
}

uint CActorPunchingBall::GetBehaviourFlags(int state)
{
	uint behaviourFlags;

	if (state < 7) {
		behaviourFlags = CActorFighter::GetBehaviourFlags(state);
	}
	else {
		behaviourFlags = 0;
	}

	return behaviourFlags;
}

void CActorPunchingBall::UpdatePostAnimEffects()
{
	IMPLEMENTATION_GUARD_FX();
}

void CActorPunchingBall::SetState(int newState, int animType)
{
	if (this->currentAnimType == 0x81) {
		if (GetLifeInterface()->GetValue() <= 0.0f) {
			animType = 0x81;
		}
	}

	CActorMovable::SetState(newState, animType);

	return;
}

void CActorPunchingBall::ChangeManageState(int state)
{
	if (state == 0) {
		if ((GetStateFlags(this->actorState) & 0x10000) != 0) {
			return;
		}
	}

	CActorAutonomous::ChangeManageState(state);

	return;
}

void CActorPunchingBall::AnimEvaluate(uint layerId, edAnmMacroAnimator* pAnimator, uint newAnim)
{
	edANM_HDR* peVar1;
	uint uVar2;
	float r2;
	float r1;
	edAnmMacroBlendN local_4;

	local_4 = edAnmMacroBlendN(pAnimator->pAnimKeyTableEntry);
	if (newAnim == 0x8b) {	
		r2 = (this->vibrationDyn).field_0x4;
		uVar2 = 0;
		r1 = (this->vibrationDyn).field_0x0;
		
		char* pBase = (char*)pAnimator->pAnimKeyTableEntry;
		AnimKeySomething* pValue = (AnimKeySomething*)(pBase + pAnimator->pAnimKeyTableEntry->keyIndex_0x8.asKey * 4);
		if (local_4.pHdr->keyIndex_0x8.asKey != 0) {
			do {
				pValue->field_0xc_array[uVar2] = 0.0f;
				uVar2 = uVar2 + 1;
			} while (uVar2 < local_4.pHdr->keyIndex_0x8.asKey);
		}

		if ((r2 < 0.0f) || (r1 < 0.0f)) {
			if ((r2 <= 0.0f) && (0.0f <= r1)) {
				CActor::SV_Blend3AnimationsWith2Ratios(r1, -r2, &local_4, 0, 4, 1);
			}
			if ((r2 < 0.0f) || (0.0f < r1)) {
				if ((r2 <= 0.0f) && (r1 <= 0.0f)) {
					CActor::SV_Blend3AnimationsWith2Ratios(-r1, -r2, &local_4, 0, 3, 1);
				}
			}
			else {
				CActor::SV_Blend3AnimationsWith2Ratios(-r1, r2, &local_4, 0, 3, 2);
			}
		}
		else {
			CActor::SV_Blend3AnimationsWith2Ratios(r1, r2, &local_4, 0, 4, 2);
		}
	}
	else {
		CActorFighter::AnimEvaluate(layerId, pAnimator, newAnim);
	}

	return;
}

bool CActorPunchingBall::CarriedByActor(CActor* pActor, edF32MATRIX4* m0)
{
	IMPLEMENTATION_GUARD();
}

int CActorPunchingBall::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	CLifeInterface* pCVar1;
	StateConfig* pSVar2;
	int iVar3;
	uint uVar4;
	edF32VECTOR4* v0;
	float fVar5;
	float fVar6;
	float fVar7;
	VibrationParam local_20;

	if (msg != 3) {
		if (msg == 0x5d) {
			if ((GetStateFlags(this->actorState) & 0x10000) != 0) {
				return 0;
			}

			iVar3 = CActorFighter::InterpretMessage(pSender, 0x5d, pMsgParam);
			return iVar3;
		}

		if (msg == 0x5c) {
			if ((GetStateFlags(this->actorState) & 0x10000) != 0) {
				return 0;
			}

			UpdatePosition(&this->baseLocation, true);

			this->field_0xee4 = 1;
			SetBehaviour(PUNCHING_BALL_BEHAVIOUR_STAND, 0x75, -1);
			iVar3 = CActorFighter::InterpretMessage(pSender, 0x5c, pMsgParam);
			return iVar3;
		}

		if (msg == 0x10) {
			this->field_0xee4 = this->field_0xee4 + 1;
		}
		else {
			if (msg == 0xf) {
				this->field_0xee4 = this->field_0xee4 + -1;
			}
			else {
				if (msg != 2) {
					iVar3 = CActorFighter::InterpretMessage(pSender, msg, pMsgParam);
					return iVar3;
				}
				IMPLEMENTATION_GUARD(
				v0 = &(this->base).field_0x6a0;
				fVar7 = *(float*)((int)pMsgParam + 0x24);
				fVar5 = *(float*)((int)pMsgParam + 0x28);
				fVar6 = *(float*)((int)pMsgParam + 0x2c);
				(this->base).field_0x6a0.x = *(float*)((int)pMsgParam + 0x20);
				(this->base).field_0x6a0.y = fVar7;
				(this->base).field_0x6a0.z = fVar5;
				(this->base).field_0x6a0.w = fVar6;
				(this->base).field_0x6a0.x = *(float*)((int)pMsgParam + 0x20);
				(this->base).field_0x6a0.y = 0.0;
				(this->base).field_0x6a0.z = *(float*)((int)pMsgParam + 0x28);
				(this->base).field_0x6a0.w = 0.0;
				edF32Vector4NormalizeHard(v0, v0);
				(this->base).field_0x6b4 = *(undefined4*)((int)pMsgParam + 0xc);
				/* WARNING: Load size is inaccurate */
				if ((*pMsgParam == 8) || (*pMsgParam == 7)) {
					pCVar1 = (*(this->pVTable)->GetLifeInterface)((CActor*)this);
					fVar5 = (float)(**(code**)((int)pCVar1->pVtable + 0x24))();
					if (0.0 < fVar5) {
						if ((*(uint*)((int)pMsgParam + 8) & 1) != 0) {
							local_20.field_0x0 = this->field_0xa88;
							local_20.field_0x4 = this->field_0xa8c;
							local_20.field_0x8 = this->field_0xa80;
							local_20.field_0x10 = 1.355;
							local_20.field_0x14 = 0.05;
							local_20.field_0xc = (this->field_0xa84 * 3.141593) / 180.0;
							local_20.pActor = (CActor*)this;
							CVibrationDyn::Init(&this->vibrationDyn, &local_20);
							iVar3 = CActorFighter::InterpretMessage((CActorFighter*)this, pSender, 2, pMsgParam);
							return iVar3;
						}
						CActorFighter::PlayImpactFx
						((CActorFighter*)this, (long)((int)pMsgParam + 0x40), &(this->base).field_0x6a0, 0, '\0');
						if (this->curBehaviourId == 7) {
							FUN_00116a00((float)(this->base).field_0x6b4, (float*)&this->vibrationDyn, &(this->base).field_0x6a0,
								(edF32VECTOR4*)((int)pMsgParam + 0x40));
							(*(this->pVTable)->SetBehaviour)((CActor*)this, 7, 0x77, -1);
						}
					}
				})
			}
		}
	}

	return 0;
}

void CActorPunchingBall::ProcessDeath()
{
	IMPLEMENTATION_GUARD();
}

void CActorPunchingBall::ClearLocalData()
{
	int iVar1;
	CLifeInterface* pCVar2;
	int iVar3;
	int iVar4;
	int iVar5;
	float fVar6;
	VibrationParam vibrationParam;

	vibrationParam.field_0x0 = this->field_0xa88;
	vibrationParam.field_0x4 = this->field_0xa8c;
	vibrationParam.field_0x8 = this->field_0xa80;
	vibrationParam.field_0x10 = 1.355f;
	vibrationParam.field_0x14 = 0.05f;
	vibrationParam.field_0xc = (this->field_0xa84 * 3.141593f) / 180.0f;
	vibrationParam.pActor = this;
	this->vibrationDyn.Init(&vibrationParam);

	iVar4 = 0;
	if (this->field_0xa90 != (S_TRAP_STREAM_REF*)0x0) {
		iVar4 = this->field_0xa90->entryCount;
	}

	for (int i = 0; i < iVar4; i++) {
		this->field_0xa90->aEntries[i].onoff.Reset();
		this->field_0xa90->aEntries[i].eventCamera.Reset(this);
	}

	GetLifeInterface()->SetValue(GetLifeInterface()->GetValueMax());

	(this->camFigData).pCommanderRef_0x230 = this;

	this->camFigData.Reset();
	this->camFigData.IsValid(0);

	// (*(code*)((this->field_0xe50).pVTable)->field_0x14)(&this->field_0xe50, 7);

	this->field_0xee0 = 0.0f;
	this->field_0xee4 = 1;

	return;
}

void CBehaviourPunchingBall::Create(ByteCode* pByteCode)
{
	return;
}

void CBehaviourPunchingBall::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	this->pOwner = reinterpret_cast<CActorPunchingBall*>(pOwner);

	return;
}

int CBehaviourPunchingBall::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	return 0;
}

int CBehaviourPunchingBall::InterpretEvent(edCEventMessage* pEventMessage, undefined8 param_3, int param_4, uint* param_5)
{
	return 0;
}

void CBehaviourPunchingBallStand::Create(ByteCode* pByteCode)
{
	return;
}

void CBehaviourPunchingBallStand::Manage()
{
	bool bVar1;
	CLifeInterface* pCVar2;
	CLifeInterface* pLife;
	float fVar3;
	edF32VECTOR3 eStack96;
	edF32VECTOR4 eStack80;
	edF32MATRIX4 eStack64;
	CActorPunchingBall* pPunchingBall;

	pPunchingBall = this->pOwner;
	switch (pPunchingBall->actorState) {
	case 0x74:
		pPunchingBall->ManageDyn(4.0f, 0x129, (CActorsTable*)0x0);
		if (((pPunchingBall->pCollisionData)->flags_0x4 & 2) != 0) {
			pPunchingBall->SetState(0x73, -1);
		}
		break;
	case 0x76:
		if (0.5f < pPunchingBall->timeInAir) {
			pPunchingBall->GetLifeInterfaceOther()->SetValue(pPunchingBall->GetLifeInterfaceOther()->GetValueMax());

			pPunchingBall->SetState(0x75, -1);
		}
		break;
	case 0x77:
		bVar1 = pPunchingBall->vibrationDyn.MakeVibrate(&eStack96);
		if (bVar1 != false) {
			pPunchingBall->SetState(0x73, -1);
		}
		pPunchingBall->UpdatePosition(&pPunchingBall->currentLocation, true);
	}

	if (pPunchingBall->field_0xee4 == 0) {
		pPunchingBall->UpdatePosition(&pPunchingBall->currentLocation, true);
	}
	else {
		if (pPunchingBall->field_0xa94.Get() == (CActor*)0x0) {
			pPunchingBall->UpdatePosition(&pPunchingBall->baseLocation, true);
		}
		else {
			pPunchingBall->field_0xa94.Get()->SV_ComputeDiffMatrixFromInit(&eStack64);
			edF32Matrix4MulF32Vector4Hard(&eStack80, &eStack64, &pPunchingBall->baseLocation);
			pPunchingBall->UpdatePosition(&eStack80, true);
		}
	}

	return;
}

void CBehaviourPunchingBallStand::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	this->pOwner = reinterpret_cast<CActorPunchingBall*>(pOwner);

	if (newState == -1) {
		this->pOwner->SetState(0x72, -1);
	}
	else {
		this->pOwner->SetState(newState, newAnimationType);
	}

	return;
}

void CBehaviourPunchingBallStand::InitState(int newState)
{
	CActorPunchingBall* pPunchingBall = this->pOwner;

	if (newState == 0x72) {
		pPunchingBall->flags = pPunchingBall->flags & 0xfffffffd;
		pPunchingBall->flags = pPunchingBall->flags | 1;
		pPunchingBall->flags = pPunchingBall->flags & 0xffffff7f;
		pPunchingBall->flags = pPunchingBall->flags | 0x20;
		pPunchingBall->EvaluateDisplayState();
	}
	else {
		if (newState == 0x76) {
			pPunchingBall->GetLifeInterfaceOther()->SetValue(0.0f);
		}
		else {
			if (newState == 0x75) {
				edF32VECTOR4 newRotQuat;
				SetVectorFromAngles(&newRotQuat, &(pPunchingBall->pCinData)->rotationEuler);
				pPunchingBall->rotationQuat = newRotQuat;
			}
		}
	}

	return;
}

void CBehaviourPunchingBallStand::TermState(int oldState, int newState)
{
	return;
}

int CBehaviourPunchingBallStand::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	return 0;
}
