#include "ActorStiller.h"
#include "MemoryStream.h"
#include "CinematicManager.h"
#include "MathOps.h"

void CActorStiller::Create(ByteCode* pByteCode)
{
	int* piVar1;
	char* pcVar2;
	uint uVar3;
	int iVar4;
	float fVar5;

	CActor::Create(pByteCode);

	this->vision.Create(this, pByteCode);

	this->field_0x1d0 = pByteCode->GetU32();
	this->field_0x1d4 = pByteCode->GetU32();
	this->field_0x1d8 = pByteCode->GetU32();
	this->field_0x1dc = pByteCode->GetU32();

	this->pathFollowReader.Create(pByteCode);

	this->field_0x1e0 = pByteCode->GetF32();
	this->lifeBase.Create(pByteCode);

	this->field_0x1ec = pByteCode->GetF32();
	this->field_0x1f0 = pByteCode->GetF32();
	this->field_0x1f4 = pByteCode->GetF32();

	this->field_0x1f8 = pByteCode->GetU32();

	S_ACTOR_STREAM_REF* pActorStreamRef = reinterpret_cast<S_ACTOR_STREAM_REF*>(pByteCode->currentSeekPos);
	pByteCode->currentSeekPos = pByteCode->currentSeekPos + 4;
	if (pActorStreamRef->entryCount != 0) {
		pByteCode->currentSeekPos = pByteCode->currentSeekPos + pActorStreamRef->entryCount * 4;
	}
	this->field_0x1fc = pActorStreamRef;

	this->field_0x200 = pByteCode->GetS32();

	S_NTF_TARGET_STREAM_REF::Create(&this->field_0x204, pByteCode);

	S_STREAM_EVENT_CAMERA* pCameraEvent = reinterpret_cast<S_STREAM_EVENT_CAMERA*>(pByteCode->currentSeekPos);
	pByteCode->currentSeekPos = pByteCode->currentSeekPos + sizeof(S_STREAM_EVENT_CAMERA);
	this->field_0x208 = pCameraEvent;

	S_NTF_TARGET_STREAM_REF::Create(&this->field_0x20c, pByteCode);

	pCameraEvent = reinterpret_cast<S_STREAM_EVENT_CAMERA*>(pByteCode->currentSeekPos);
	pByteCode->currentSeekPos = pByteCode->currentSeekPos + sizeof(S_STREAM_EVENT_CAMERA);
	this->field_0x210 = pCameraEvent;

	this->field_0x214 = pByteCode->GetS32();


	this->addOnGenerator.Create(this, pByteCode);

	return;
}

void CActorStiller::Init()
{
	int* piVar1;
	S_STREAM_REF<CActor>* pRef;
	int iVar2;
	CActor* pCVar3;
	int iVar4;

	CActor::Init();

	this->pathFollowReader.Init();

	for (int i = 0; i < this->field_0x1fc->entryCount; i++) {
		this->field_0x1fc->aEntries[i].Init();
	}

	this->field_0x204->Init();
	this->field_0x208->Init();

	this->field_0x20c->Init();
	this->field_0x210->Init();

	for (int i = 0; i < this->field_0x1fc->entryCount; i++) {
		this->field_0x1fc->aEntries[i].Reset();
	}

	this->vision.Reset();

	if (this->field_0x214 == 0) {
		this->pathFollowReader.Reset();
	}

	this->lifeBase.Reset();

	this->field_0x25c = 0;

	this->field_0x204->Reset();
	this->field_0x208->Reset(this);

	this->field_0x20c->Reset();
	this->field_0x210->Reset(this);

	this->pAnimationController->RegisterBone(this->field_0x1d0);
	this->pAnimationController->RegisterBone(this->field_0x1d4);

	this->addOnGenerator.Init(1);

	return;
}

void CActorStiller::Term()
{
	CActor::Term();
	this->addOnGenerator.Term();

	return;
}

void CActorStiller::Draw()
{
	CActor::Draw();
	this->vision.Draw();

	return;
}

void CActorStiller::Reset()
{
	CActor::Reset();

	for (int i = 0; i < this->field_0x1fc->entryCount; i++) {
		this->field_0x1fc->aEntries[i].Reset();
	}

	vision.Reset();

	if (this->field_0x214 == 0) {
		this->pathFollowReader.Reset();
	}

	lifeBase.Reset();

	this->field_0x25c = 0;

	this->field_0x204->Reset();
	this->field_0x208->Reset(this);

	this->field_0x20c->Reset();
	this->field_0x210->Reset(this);

	return;
}

CBehaviour* CActorStiller::BuildBehaviour(int behaviourType)
{
	CBehaviour* pBehaviour;

	if (behaviourType == STILLER_BEHAVIOUR_STAND) {
		pBehaviour = &this->behaviourStand;
	}
	else {
		pBehaviour = CActor::BuildBehaviour(behaviourType);
	}

	return pBehaviour;
}

StateConfig CActorStiller::_gStateCfg_STI[5] =
{
	StateConfig(0x0, 0x300),
	StateConfig(0x8, 0x4),
	StateConfig(0x9, 0x4),
	StateConfig(0x6, 0x4),
	StateConfig(0x7, 0x4),
};

StateConfig* CActorStiller::GetStateCfg(int state)
{
	StateConfig* pStateConfig;

	if (state < 5) {
		pStateConfig = CActor::GetStateCfg(state);
	}
	else {
		assert((state - 5) < 5);
		pStateConfig = _gStateCfg_STI + state + -5;
	}

	return pStateConfig;
}

void CActorStiller::ChangeManageState(int state)
{
	StateConfig* pSVar1;
	int iVar2;
	uint uVar3;

	CActor::ChangeManageState(state);

	if (state == 0) {
		if ((GetStateFlags(this->actorState) & 0x100) == 0) {
			this->pAnimationController->UnRegisterBone(this->field_0x1d4);
		}
	}
	else {
		if ((GetStateFlags(this->actorState) & 0x100) == 0) {
			this->pAnimationController->RegisterBone(this->field_0x1d4);
		}
	}

	return;
}

CVision* CActorStiller::GetVision()
{
	return &this->vision;
}

int CActorStiller::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	bool bVar1;
	undefined8 uVar2;
	float fVar3;
	float fVar4;
	uint uVar5;
	StateConfig* pSVar6;
	int iVar7;
	float fVar8;
	edF32VECTOR4 local_20;
	edF32VECTOR4 eStack16;

	if (msg == 3) {
		IMPLEMENTATION_GUARD(
		if (this->field_0x214 != 0) {
			uVar2 = *(undefined8*)&this->currentLocation;
			local_20.z = this->currentLocation.z;
			local_20.w = this->currentLocation.w;
			local_20.x = (float)uVar2;
			local_20.y = (float)((ulong)uVar2 >> 0x20) + 0.5;
			ActorComponent34Func_00397180(&this->addOnGenerator, &local_20);
			(*(this->pVTable)->SetState)((CActor*)this, 0xe, -1);
			return 1;
		}

		(*(this->pVTable)->SetState)((CActor*)this, 0xd, -1);
			)
		return 1;
	}

	if (msg == MESSAGE_GET_VISUAL_DETECTION_POINT) {
		uVar5 = GetStateFlags(this->actorState) & 1;

		bVar1 = uVar5 != 0;
		if (bVar1) {
			bVar1 = this->field_0x214 != 0;
		}

		if (!bVar1) {
			bVar1 = (GetStateFlags(this->actorState) & 100) == 0;
		}

		if (bVar1) {
			GetPositionMsgParams* pGetPosMsgParams = reinterpret_cast<GetPositionMsgParams*>(pMsgParam);
			if (this->field_0x1d4 != 0) {
				SV_GetBoneWorldPosition(this->field_0x1d4, &eStack16);
				edF32Vector4SubHard(&pGetPosMsgParams->vectorFieldB, &eStack16, &this->currentLocation);
				return 1;
			}

			pGetPosMsgParams->vectorFieldB = gF32Vector4Zero;
		}
	}
	else {
		if (msg != 2) {
			iVar7 = CActor::InterpretMessage(pSender, msg, (GetPositionMsgParams*)pMsgParam);
			return iVar7;
		}

		IMPLEMENTATION_GUARD(
		iVar7 = this->actorState;
		if (iVar7 == -1) {
			uVar5 = 0;
		}
		else {
			pSVar6 = (*(this->pVTable)->GetStateCfg)((CActor*)this, iVar7);
			uVar5 = pSVar6->flags_0x4 & 1;
		}
		bVar1 = uVar5 != 0;
		if (bVar1) {
			bVar1 = this->field_0x214 != 0;
		}
		if (!bVar1) {
			iVar7 = this->actorState;
			if (iVar7 == -1) {
				uVar5 = 0;
			}
			else {
				pSVar6 = (*(this->pVTable)->GetStateCfg)((CActor*)this, iVar7);
				uVar5 = pSVar6->flags_0x4 & 0x100;
			}
			bVar1 = uVar5 == 0;
		}

		/* WARNING: Load size is inaccurate */
		if ((bVar1) && (*pMsgParam == 4)) {
			if ((this->field_0x1f8 & 1) == 0) {
				CLifeBase::LifeDecrease(*(undefined4*)((int)pMsgParam + 0xc), (CLifeBase*)&this->lifeBase);
			}
			fVar8 = CLifeBase::GetValue((CLifeBase*)&this->lifeBase);
			if (0.0 < fVar8) {
				(*(this->pVTable)->SetState)((CActor*)this, 9, -1);
				fVar8 = CLifeBase::GetValueMax((CLifeBase*)&this->lifeBase);
				CLifeBase::SetValue(fVar8, (CLifeBase*)&this->lifeBase);
				return 1;
			}
			(*(this->pVTable)->SetState)((CActor*)this, 0xd, -1);
			return 1;
		})
	}

	return 0;
}

void CActorStiller::BehaviourStillerStand_InitState(int newState)
{
	int iVar1;
	int iVar2;
	CAnimation* pAnimation;

	if (newState == 8) {
		pAnimation = this->pAnimationController;
		if (pAnimation != (CAnimation*)0x0) {
			pAnimation->RegisterBone(this->field_0x1d0);
		}
	}
	else {
		if ((newState == 9) || (newState == 0xd)) {
			this->field_0x204->Switch(this);
			this->field_0x208->SwitchOn(this);
		}
	}

	return;
}

void CActorStiller::BehaviourStillerStand_Manage()
{
	CAnimation* pCVar1;
	edAnmLayer* peVar2;
	bool bVar3;
	float newValue;

	this->field_0x208->Manage(this);
	this->field_0x210->Manage(this);

	switch (this->actorState) {
	case 5:
		IMPLEMENTATION_GUARD(
		bVar3 = CheckDetectArea(this);
		if (bVar3 != false) {
			(*(this->pVTable)->SetState)((CActor*)this, 6, -1);
		})
		break;
	case 6:
		IMPLEMENTATION_GUARD(
		bVar3 = CheckDetectArea(this);
		if (bVar3 == false) {
			(*(this->pVTable)->SetState)((CActor*)this, 10, -1);
		}
		else {
			bVar3 = CheckAttackArea(this);
			if (bVar3 == false) {
				if (this->pAnimationController->IsCurrentLayerAnimEndReached(0)) {
					(*(this->pVTable)->SetState)((CActor*)this, 7, -1);
				}
			}
			else {
				(*(this->pVTable)->SetState)((CActor*)this, 8, -1);
			}
		})
		break;
	case 7:
		IMPLEMENTATION_GUARD(
		bVar3 = CheckAttackArea(this);
		if (bVar3 == false) {
			bVar3 = CheckDetectArea(this);
			if (bVar3 == false) {
				(*(this->pVTable)->SetState)((CActor*)this, 10, -1);
			}
		}
		else {
			(*(this->pVTable)->SetState)((CActor*)this, 8, -1);
		})
		break;
	case 8:
		IMPLEMENTATION_GUARD(
		ActorFunc_003d52a0((Actor*)this);)
		break;
	case 9:
		IMPLEMENTATION_GUARD(
		if (((this->field_0x1f8 & 1) == 0) || (bVar3 = CheckAttackArea(this), bVar3 == false)) {
			if (this->pAnimationController->IsCurrentLayerAnimEndReached(0)) {
				if ((this->field_0x1f8 & 1) == 0) {
					(*(this->pVTable)->SetState)((CActor*)this, 0xc, -1);
				}
				else {
					(*(this->pVTable)->SetState)((CActor*)this, 7, -1);
				}
			}
		}
		else {
			(*(this->pVTable)->SetState)((CActor*)this, 8, -1);
		})
		break;
	case 10:
		IMPLEMENTATION_GUARD(
		if (this->pAnimationController->IsCurrentLayerAnimEndReached(0)) {
			(*(this->pVTable)->SetState)((CActor*)this, 5, -1);
		}
		bVar3 = CheckDetectArea(this);
		if (bVar3 == false) {
			bVar3 = CheckAttackArea(this);
			if (bVar3 != false) {
				(*(this->pVTable)->SetState)((CActor*)this, 8, -1);
			}
		}
		else {
			(*(this->pVTable)->SetState)((CActor*)this, 6, -1);
		})
		break;
	case 0xb:
		IMPLEMENTATION_GUARD(
		if (this->pAnimationController->IsCurrentLayerAnimEndReached(0)) {
			newValue = CLifeBase::GetValueMax((CLifeBase*)&this->lifeBase);
			CLifeBase::SetValue(newValue, (CLifeBase*)&this->lifeBase);
			(*(this->pVTable)->SetState)((CActor*)this, 0xc, -1);
		})
		break;
	case 0xc:
		IMPLEMENTATION_GUARD(
		bVar3 = CheckDetectArea(this);
		if (bVar3 == false) {
			(*(this->pVTable)->SetState)((CActor*)this, 5, -1);
		}
		else {
			if (this->field_0x1f4 < this->timeInAir) {
				(*(this->pVTable)->SetState)((CActor*)this, 6, -1);
			}
		})
	}
	
	return;
}

void CActorStiller::BehaviourStillerStand_TermState(int oldState)
{
	int iVar1;
	int iVar2;
	CAnimation* pAnimation;

	if (oldState == 8) {
		pAnimation = this->pAnimationController;
		if (pAnimation != (CAnimation*)0x0) {
			pAnimation->UnRegisterBone(this->field_0x1d0);
		}
	}
	else {
		if (oldState == 0xc) {
			for (int i = 0; i < this->field_0x20c->entryCount; i++) {
				this->field_0x20c->aEntries[i].Switch(this);
			}

			this->field_0x210->SwitchOn(this);
		}
	}
	return;
}

void CBehaviourStiller::Create(ByteCode* pByteCode)
{

}

void CBehaviourStiller::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	this->pOwner = static_cast<CActorStiller*>(pOwner);

	return;
}

int CBehaviourStiller::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	return 0;
}

int CBehaviourStiller::InterpretEvent(edCEventMessage* pEventMessage, undefined8 param_3, int param_4, uint* param_5)
{
	return 0;
}

void CBehaviourStillerStand::Create(ByteCode* pByteCode)
{

}

void CBehaviourStillerStand::Manage()
{
	this->pOwner->BehaviourStillerStand_Manage();

	return;
}

void CBehaviourStillerStand::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	this->pOwner = static_cast<CActorStiller*>(pOwner);

	if (newState == -1) {
		this->pOwner->SetState(5, -1);
	}
	else {
		this->pOwner->SetState(newState, newAnimationType);
	}

	return;
}

void CBehaviourStillerStand::InitState(int newState)
{
	this->pOwner->BehaviourStillerStand_InitState(newState);

	return;
}

void CBehaviourStillerStand::TermState(int oldState, int newState)
{
	this->pOwner->BehaviourStillerStand_TermState(oldState);

	return;
}

int CBehaviourStillerStand::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	return 0;
}