#include "ActorSwitch.h"
#include "MemoryStream.h"
#include "CinematicManager.h"
#include "CollisionManager.h"
#include "MathOps.h"

StateConfig CActorSwitch::_gStateCfg_SWT[5] = {
	StateConfig(0x0, 0x4),
	StateConfig(0x0, 0x4),
	StateConfig(0x0, 0x4),
	StateConfig(0x0, 0x4),
	StateConfig(0x0, 0x4),
};

void CActorSwitch::Create(ByteCode* pByteCode)
{
	S_TARGET_STREAM_REF* pSVar1;
	S_STREAM_EVENT_CAMERA* pSVar2;
	CBehaviour* pCVar3;
	int* piVar4;

	CActor::Create(pByteCode);

	pSVar1 = (S_TARGET_STREAM_REF*)pByteCode->currentSeekPos;
	pByteCode->currentSeekPos = (char*)pSVar1->aEntries;
	if (pSVar1->entryCount != 0) {
		pByteCode->currentSeekPos = pByteCode->currentSeekPos + pSVar1->entryCount * sizeof(S_STREAM_NTF_TARGET_SWITCH);
	}
	this->pTargetStreamRef = pSVar1;

	pSVar2 = (S_STREAM_EVENT_CAMERA*)pByteCode->currentSeekPos;
	pByteCode->currentSeekPos = (char*)(pSVar2 + 1);
	this->pStreamEventCamera = pSVar2;

	pCVar3 = CActor::GetBehaviour(2);
	if (pCVar3 == (CBehaviour*)0x0) {
		this->pActorSound = (CActorSound*)0x0;
	}
	else {
		IMPLEMENTATION_GUARD_AUDIO(
		piVar4 = CActor::CreateActorSound((CActor*)this, 1);
		this->pActorSound = piVar4;)
	}

	return;
}

void CActorSwitch::Init()
{
	for (int i = 0; i < this->pTargetStreamRef->entryCount; i++) {
		this->pTargetStreamRef->aEntries[i].Init();
	}

	this->pStreamEventCamera->Init();
	CActor::Init();

	return;
}

void CActorSwitch::Reset()
{
	for (int i = 0; i < this->pTargetStreamRef->entryCount; i++) {
		this->pTargetStreamRef->aEntries[i].Reset();
	}

	this->pStreamEventCamera->Reset(this);

	return;
}

CBehaviour* CActorSwitch::BuildBehaviour(int behaviourType)
{
	CBehaviour* pBehaviour;

	if (behaviourType < 9) {
		switch (behaviourType) {
		case 0:
		case 1:
			pBehaviour = CActor::BuildBehaviour(behaviourType);
			break;
		case 2:
			pBehaviour = new CBehaviourSwitchLever;
			break;
		case 3:
			pBehaviour = new CBehaviourSwitchMagic;
			break;
		case 4:
			pBehaviour = new CBehaviourSwitchWolfenCounter;
			break;
		case 5:
			pBehaviour = &this->behaviourSwitchMultiCondition;
			break;
		case 6:
			pBehaviour = new CBehaviourSwitchTarget;
			break;
		case 7:
			pBehaviour = &this->behaviourSwitchSequence;
			break;
		case 8:
			pBehaviour = new CBehaviourSwitchNew;
			break;
		}

		return pBehaviour;
	}

	pBehaviour = CActor::BuildBehaviour(behaviourType);
	return pBehaviour;
}

StateConfig* CActorSwitch::GetStateCfg(int state)
{
	StateConfig* pStateConfig;

	if (state < 5) {
		pStateConfig = CActor::GetStateCfg(state);
	}
	else {
		pStateConfig = _gStateCfg_SWT + state + -5;
	}

	return pStateConfig;
}

void CActorSwitch::ChangeManageState(int state)
{
	CBehaviourSwitch* pCVar1;

	CActor::ChangeManageState(state);

	switch (this->curBehaviourId) {
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
	case 7:
		pCVar1 = static_cast<CBehaviourSwitch*>(GetBehaviour(this->curBehaviourId));
		if (pCVar1 != (CBehaviourSwitch*)0x0) {
			pCVar1->ChangeManageState(state);
		}
	}

	return;
}

int CActorSwitch::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	return CActor::InterpretMessage(pSender, msg, pMsgParam);
}

CBehaviourSwitchMagic::CBehaviourSwitchMagic()
{
	//this->field_0x1c = 0;
	//this->field_0x20 = 0;
	//this->field_0x34 = 0;
	//this->field_0x38 = 0;
	(this->fxDigits).field_0x0 = (ParticleInfo*)0x0;
	//this->field_0x50 = 0;
	//this->field_0x54 = 0;
	this->pStreamEventCamera = (S_STREAM_EVENT_CAMERA*)0x0;
}

void CBehaviourSwitchMagic::Create(ByteCode* pByteCode)
{
	S_STREAM_EVENT_CAMERA* pSVar1;

	this->field_0x8 = pByteCode->GetF32();

	this->field_0x10 = pByteCode->GetF32();
	if (0.0f < this->field_0x10) {
		this->field_0x10 = this->field_0x8 / this->field_0x10;
	}
	else {
		this->field_0x10 = this->field_0x8;
	}

	this->fxDigits.Init(pByteCode->GetS32());
	this->field_0x14 = pByteCode->GetF32();
	this->field_0x4c = pByteCode->GetS32();
	this->field_0x18 = pByteCode->GetS32();
	this->field_0x28 = pByteCode->GetS32();
	this->field_0x24 = pByteCode->GetS32();
	this->field_0x30 = pByteCode->GetS32();
	this->field_0x40 = pByteCode->GetS32();
	this->field_0x3c = pByteCode->GetS32();

	pSVar1 = (S_STREAM_EVENT_CAMERA*)pByteCode->currentSeekPos;
	pByteCode->currentSeekPos = (char*)(pSVar1 + 1);
	this->pStreamEventCamera = pSVar1;

	this->pHierarchy = (edNODE*)0x0;
	this->field_0x44 = 0;

	return;

}

void CBehaviourSwitchMagic::Init(CActor* pOwner)
{
	this->pStreamEventCamera->Init();

	return;
}

static edF32VECTOR4 gMagicSwitchBoundingSphere = { 0.0f, 0.0f, 0.0f, 1.0f };

void CBehaviourSwitchMagic::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	CActorSwitch* pCVar1;
	edF32VECTOR4 local_10;

	this->pOwner = static_cast<CActorSwitch*>(pOwner);

	this->field_0xc = 0.0f;

	local_10.x = gMagicSwitchBoundingSphere.x;
	local_10.z = gMagicSwitchBoundingSphere.z;
	local_10.w = gMagicSwitchBoundingSphere.w;
	local_10.y = this->field_0x14;
	this->pOwner->SetLocalBoundingSphere(fabs(this->field_0x14 * 2.0f), &local_10);

	this->field_0x1c = 0;
	this->field_0x20 = 0;
	this->field_0x34 = 0;
	this->field_0x38 = 0;
	this->field_0x50 = 0;
	this->field_0x54 = 0;

	if (newState == -1) {
		this->pOwner->SetState(5, -1);
	}
	else {
		this->pOwner->SetState(newState, newAnimationType);
	}

	if ((this->pOwner->flags & 4) != 0) {
		this->ChangeManageState(1);
	}

	this->pStreamEventCamera->Reset(this->pOwner);

	return;
}

void CBehaviourSwitchMagic::InitState(int newState)
{
	if (newState == 6) {
		this->pStreamEventCamera->SwitchOn(this->pOwner);
	}

	return;
}

void CBehaviourSwitchTarget::Create(ByteCode* pByteCode)
{
	this->field_0x8 = pByteCode->GetF32();
	this->field_0xc = pByteCode->GetF32();
	this->field_0x10 = pByteCode->GetF32();

	return;
}

void CBehaviourSwitchTarget::Init(CActor* pOwner)
{
	CCollision* pCVar1;
	CActorSwitch* pSwitch;

	this->pOwner = static_cast<CActorSwitch*>(pOwner);

	this->field_0xc = this->field_0xc * 100.0f;
	this->field_0x10 = this->field_0x10 * 100.0f;

	pCVar1 = this->pOwner->pCollisionData;
	if (pCVar1 != (CCollision*)0x0) {
		pCVar1->flags_0x0 = pCVar1->flags_0x0 | 0x800;
		pSwitch = this->pOwner;
		pSwitch->UpdatePosition(&pSwitch->currentLocation, true);
	}

	return;
}

void CBehaviourSwitchTarget::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	this->field_0x14 = 0.0f;
	this->field_0x18 = 0;

	if (newState == -1) {
		this->pOwner->SetState(5, -1);
	}
	else {
		this->pOwner->SetState(newState, newAnimationType);
	}

	return;
}

void CBehaviourSwitchLever::Create(ByteCode* pByteCode)
{
	this->field_0x40.index = pByteCode->GetS32();
	this->field_0x44.index = pByteCode->GetS32();
	return;
}

void CBehaviourSwitchLever::Init(CActor* pOwner)
{
	this->pOwner = static_cast<CActorSwitch*>(pOwner);

	this->field_0x40.Init();
	this->field_0x44.Init();

	return;
}

void CBehaviourSwitchLever::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	edF32MATRIX4 rotation;

	if (newState == -1) {
		this->pOwner->SetState(5, -1);
	}
	else {
		this->pOwner->SetState(newState, newAnimationType);
	}

	edF32Matrix4FromEulerSoft(&rotation, &this->pOwner->rotationEuler.xyz, "XYZ");
	edQuatFromMatrix4(&this->field_0x20, &rotation);

	this->field_0x20 = gF32Vertex4Zero;
	this->field_0x30 = gF32Vertex4Zero;
	this->field_0x10 = gF32Vertex4Zero;

	this->pActor = (CActor*)0x0;
	return;
}

void CBehaviourSwitchLever::InitState(int newState)
{
	CSound* pSound;
	CActorSwitch* pActor;

	if (newState == 9) {
		IMPLEMENTATION_GUARD_AUDIO(
		pSound = (this->field_0x44).pSound;
		if (pSound != (CSound*)0x0) {
			pActor = this->pOwner;
			CActorSound::SoundStart
			((CActorSound*)pActor->pActorSound, (CActor*)pActor, 0, pSound, 1, 0, (SOUND_SPATIALIZATION_PARAM*)0x0);
		})
	}
	else {
		if (newState == 6) {
			this->pActor->pAnimationController->RegisterBone(0x414dad8d);
		}
	}

	return;
}

void CBehaviourSwitchMultiCondition::Create(ByteCode* pByteCode)
{
	this->field_0x14 = pByteCode->GetU32();
	this->field_0x8 = pByteCode->GetU32();
	this->field_0xc = pByteCode->GetU32();
	this->field_0x10 = this->field_0x8;

	return;
}

void CBehaviourSwitchMultiCondition::Init(CActor* pOwner)
{
	this->pOwner = static_cast<CActorSwitch*>(pOwner);

	return;
}

void CBehaviourSwitchMultiCondition::Manage()
{
	CActorSwitch* pActor;
	edF32VECTOR4 local_10;

	pActor = this->pOwner;
	pActor->pStreamEventCamera->Manage(pActor);

	if (pActor->pTiedActor != (CActor*)0x0) {
		local_10 = pActor->baseLocation;
		pActor->SV_UpdatePosition_Rel(&local_10, 0, 0, (CActorsTable*)0x0, (edF32VECTOR4*)0x0);
	}

	return;
}

void CBehaviourSwitchMultiCondition::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	this->field_0x10 = this->field_0x8;

	if (newState == -1) {
		if (this->field_0x10 == this->field_0xc) {
			this->pOwner->SetState(8, -1);

			for (int i = 0; i < this->pOwner->pTargetStreamRef->entryCount; i++) {
				this->pOwner->pTargetStreamRef->aEntries[i].Switch(this->pOwner);
			}

			this->pOwner->pStreamEventCamera->SwitchOn(this->pOwner);
		}
		else {
			this->pOwner->SetState(5, -1);
		}
	}
	else {
		this->pOwner->SetState(newState, newAnimationType);
	}

	return;
}

int CBehaviourSwitchMultiCondition::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	uint uVar1;
	CActorSwitch* pCVar2;
	uint uVar4;
	int iVar5;
	int iVar6;

	const uint msgParam = reinterpret_cast<uint>(pMsgParam);

	if (msg != 0xe) {
		if (msg != 0x10) {
			if (msg != 0xf) {
				return 0;
			}

			uVar1 = this->field_0xc;
			uVar4 = this->field_0x10 | 1 << (msgParam & 0x1f);

			if (uVar1 == this->field_0x10) {
				IMPLEMENTATION_GUARD(
				if (uVar1 != uVar4) {
					pCVar2 = (this->base).pOwner;
					(*((pCVar2->base).pVTable)->SetState)((CActor*)pCVar2, 5, -1);
					pCVar2 = (this->base).pOwner;
					pCVar3 = (CActorFighterVTable*)pCVar2->pTargetStreamRef;
					iVar6 = 0;
					if (0 < (int)pCVar3->actorBase) {
						iVar5 = 0;
						do {
							S_STREAM_NTF_TARGET_SWITCH::PostSwitch
							((S_STREAM_NTF_TARGET_SWITCH*)((int)&pCVar3->field_0x4 + iVar5), (CActor*)pCVar2);
							pCVar3 = (CActorFighterVTable*)pCVar2->pTargetStreamRef;
							iVar6 = iVar6 + 1;
							iVar5 = iVar5 + 0x1c;
						} while (iVar6 < (int)pCVar3->actorBase);
					}
				})
			}
			else {
				if (uVar1 == uVar4) {
					IMPLEMENTATION_GUARD(
					pCVar2 = (this->base).pOwner;
					(*((pCVar2->base).pVTable)->SetState)((CActor*)pCVar2, 8, -1);
					pCVar2 = (this->base).pOwner;
					pCVar3 = (CActorFighterVTable*)pCVar2->pTargetStreamRef;
					iVar6 = 0;
					if (0 < (int)pCVar3->actorBase) {
						iVar5 = 0;
						do {
							S_STREAM_NTF_TARGET_SWITCH::Switch
							((S_STREAM_NTF_TARGET_SWITCH*)((int)&pCVar3->field_0x4 + iVar5), (CActor*)pCVar2);
							pCVar3 = (CActorFighterVTable*)pCVar2->pTargetStreamRef;
							iVar6 = iVar6 + 1;
							iVar5 = iVar5 + 0x1c;
						} while (iVar6 < (int)pCVar3->actorBase);
					}
					S_STREAM_EVENT_CAMERA::SwitchOn(pCVar2->pStreamEventCamera, (CActor*)pCVar2);
					if ((this->field_0x14 & 1) != 0) {
						pCVar2 = (this->base).pOwner;
						(*((pCVar2->base).pVTable)->SetState)((CActor*)pCVar2, 5, -1);
						pCVar2 = (this->base).pOwner;
						pCVar3 = (CActorFighterVTable*)pCVar2->pTargetStreamRef;
						iVar6 = 0;
						if (0 < (int)pCVar3->actorBase) {
							iVar5 = 0;
							do {
								S_STREAM_NTF_TARGET_SWITCH::PostSwitch
								((S_STREAM_NTF_TARGET_SWITCH*)((int)&pCVar3->field_0x4 + iVar5), (CActor*)pCVar2);
								pCVar3 = (CActorFighterVTable*)pCVar2->pTargetStreamRef;
								iVar6 = iVar6 + 1;
								iVar5 = iVar5 + 0x1c;
							} while (iVar6 < (int)pCVar3->actorBase);
						}
						this->field_0x10 = this->field_0x8;
						return 1;
					})
				}
			}

			this->field_0x10 = uVar4;
			return 1;
		}

		IMPLEMENTATION_GUARD(
		uVar1 = this->field_0xc;
		uVar4 = this->field_0x10 & ~(1 << (msgParam & 0x1f));
		if (uVar1 == this->field_0x10) {
			if (uVar1 != uVar4) {
				pCVar2 = (this->base).pOwner;
				(*((pCVar2->base).pVTable)->SetState)((CActor*)pCVar2, 5, -1);
				pCVar2 = (this->base).pOwner;
				pCVar3 = (CActorFighterVTable*)pCVar2->pTargetStreamRef;
				iVar6 = 0;
				if (0 < (int)pCVar3->actorBase) {
					iVar5 = 0;
					do {
						S_STREAM_NTF_TARGET_SWITCH::PostSwitch
						((S_STREAM_NTF_TARGET_SWITCH*)((int)&pCVar3->field_0x4 + iVar5), (CActor*)pCVar2);
						pCVar3 = (CActorFighterVTable*)pCVar2->pTargetStreamRef;
						iVar6 = iVar6 + 1;
						iVar5 = iVar5 + 0x1c;
					} while (iVar6 < (int)pCVar3->actorBase);
				}
			}
		}
		else {
			if (uVar1 == uVar4) {
				pCVar2 = (this->base).pOwner;
				(*((pCVar2->base).pVTable)->SetState)((CActor*)pCVar2, 8, -1);
				pCVar2 = (this->base).pOwner;
				pCVar3 = (CActorFighterVTable*)pCVar2->pTargetStreamRef;
				iVar6 = 0;
				if (0 < (int)pCVar3->actorBase) {
					iVar5 = 0;
					do {
						S_STREAM_NTF_TARGET_SWITCH::Switch
						((S_STREAM_NTF_TARGET_SWITCH*)((int)&pCVar3->field_0x4 + iVar5), (CActor*)pCVar2);
						pCVar3 = (CActorFighterVTable*)pCVar2->pTargetStreamRef;
						iVar6 = iVar6 + 1;
						iVar5 = iVar5 + 0x1c;
					} while (iVar6 < (int)pCVar3->actorBase);
				}
				S_STREAM_EVENT_CAMERA::SwitchOn(pCVar2->pStreamEventCamera, (CActor*)pCVar2);
				if ((this->field_0x14 & 1) != 0) {
					pCVar2 = (this->base).pOwner;
					(*((pCVar2->base).pVTable)->SetState)((CActor*)pCVar2, 5, -1);
					pCVar2 = (this->base).pOwner;
					pCVar3 = (CActorFighterVTable*)pCVar2->pTargetStreamRef;
					iVar6 = 0;
					if (0 < (int)pCVar3->actorBase) {
						iVar5 = 0;
						do {
							S_STREAM_NTF_TARGET_SWITCH::PostSwitch
							((S_STREAM_NTF_TARGET_SWITCH*)((int)&pCVar3->field_0x4 + iVar5), (CActor*)pCVar2);
							pCVar3 = (CActorFighterVTable*)pCVar2->pTargetStreamRef;
							iVar6 = iVar6 + 1;
							iVar5 = iVar5 + 0x1c;
						} while (iVar6 < (int)pCVar3->actorBase);
					}
					this->field_0x10 = this->field_0x8;
					return 1;
				}
			}
		})

		this->field_0x10 = uVar4;
		return 1;
	}

	uVar1 = this->field_0xc;
	uVar4 = this->field_0x10 ^ 1 << (msgParam & 0x1f);
	if (uVar1 == this->field_0x10) {
		IMPLEMENTATION_GUARD(
		if (uVar1 != uVar4) {
			pCVar2 = (this->base).pOwner;
			(*((pCVar2->base).pVTable)->SetState)((CActor*)pCVar2, 5, -1);
			pCVar2 = (this->base).pOwner;
			pCVar3 = (CActorFighterVTable*)pCVar2->pTargetStreamRef;
			iVar6 = 0;
			if (0 < (int)pCVar3->actorBase) {
				iVar5 = 0;
				do {
					S_STREAM_NTF_TARGET_SWITCH::PostSwitch
					((S_STREAM_NTF_TARGET_SWITCH*)((int)&pCVar3->field_0x4 + iVar5), (CActor*)pCVar2);
					pCVar3 = (CActorFighterVTable*)pCVar2->pTargetStreamRef;
					iVar6 = iVar6 + 1;
					iVar5 = iVar5 + 0x1c;
				} while (iVar6 < (int)pCVar3->actorBase);
			}
		})
	}
	else {
		if (uVar1 == uVar4) {
			IMPLEMENTATION_GUARD(
			pCVar2 = (this->base).pOwner;
			(*((pCVar2->base).pVTable)->SetState)((CActor*)pCVar2, 8, -1);
			pCVar2 = (this->base).pOwner;
			pCVar3 = (CActorFighterVTable*)pCVar2->pTargetStreamRef;
			iVar6 = 0;
			if (0 < (int)pCVar3->actorBase) {
				iVar5 = 0;
				do {
					S_STREAM_NTF_TARGET_SWITCH::Switch
					((S_STREAM_NTF_TARGET_SWITCH*)((int)&pCVar3->field_0x4 + iVar5), (CActor*)pCVar2);
					pCVar3 = (CActorFighterVTable*)pCVar2->pTargetStreamRef;
					iVar6 = iVar6 + 1;
					iVar5 = iVar5 + 0x1c;
				} while (iVar6 < (int)pCVar3->actorBase);
			}
			S_STREAM_EVENT_CAMERA::SwitchOn(pCVar2->pStreamEventCamera, (CActor*)pCVar2);
			if ((this->field_0x14 & 1) != 0) {
				pCVar2 = (this->base).pOwner;
				(*((pCVar2->base).pVTable)->SetState)((CActor*)pCVar2, 5, -1);
				pCVar2 = (this->base).pOwner;
				pCVar3 = (CActorFighterVTable*)pCVar2->pTargetStreamRef;
				iVar6 = 0;
				if (0 < (int)pCVar3->actorBase) {
					iVar5 = 0;
					do {
						S_STREAM_NTF_TARGET_SWITCH::PostSwitch
						((S_STREAM_NTF_TARGET_SWITCH*)((int)&pCVar3->field_0x4 + iVar5), (CActor*)pCVar2);
						pCVar3 = (CActorFighterVTable*)pCVar2->pTargetStreamRef;
						iVar6 = iVar6 + 1;
						iVar5 = iVar5 + 0x1c;
					} while (iVar6 < (int)pCVar3->actorBase);
				}
				this->field_0x10 = this->field_0x8;
				return 1;
			})
		}
	}

	this->field_0x10 = uVar4;

	return 1;
}
