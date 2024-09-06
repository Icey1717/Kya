#include "ActorBasicBox.h"
#include "MemoryStream.h"
#include "CollisionManager.h"
#include "MathOps.h"
#include "TimeController.h"

#define BASIC_BOX_BEHAVIOUR_STAND 2

CActorBasicBox::CActorBasicBox()
{

}

void CActorBasicBox::Create(ByteCode* pByteCode)
{
	int iVar3;
	uint uVar4;
	float fVar5;

	CActor::Create(pByteCode);
	this->field_0x160 = pByteCode->GetS32();
	this->field_0x16c = (float)(int)(float)pByteCode->GetU32();
	
	S_ACTOR_STREAM_REF* pSeekActorStreamRef = reinterpret_cast<S_ACTOR_STREAM_REF*>(pByteCode->currentSeekPos);
	pByteCode->currentSeekPos += sizeof(S_ACTOR_STREAM_REF);
	if (pSeekActorStreamRef->entryCount != 0) {
		pByteCode->currentSeekPos = pByteCode->currentSeekPos + (pSeekActorStreamRef->entryCount * sizeof(S_STREAM_REF<CActor>));
	}
	this->pActorStream = pSeekActorStreamRef;

	this->field_0x170 = pByteCode->GetF32();
	this->field_0x174 = pByteCode->GetF32();
	this->field_0x178 = pByteCode->GetF32();
	this->field_0x17c = pByteCode->GetF32();

	this->field_0x180 = pByteCode->GetU32();
	this->field_0x18c = pByteCode->GetS32();
	this->field_0x190 = pByteCode->GetS32();
	this->field_0x198 = pByteCode->GetS32();
	this->field_0x194 = pByteCode->GetS32();

	S_TARGET_STREAM_REF* pSeekTargetStreamRef = reinterpret_cast<S_TARGET_STREAM_REF*>(pByteCode->currentSeekPos);
	pByteCode->currentSeekPos += sizeof(S_TARGET_STREAM_REF);
	if (pSeekTargetStreamRef->entryCount != 0) {
		pByteCode->currentSeekPos = pByteCode->currentSeekPos + (pSeekTargetStreamRef->entryCount * sizeof(S_STREAM_NTF_TARGET_SWITCH));

		static_assert(sizeof(S_STREAM_NTF_TARGET_SWITCH) == 0x1c);
	}
	this->targetStreamRef = pSeekTargetStreamRef;
	
	S_STREAM_EVENT_CAMERA* pSeekStreamCamera = reinterpret_cast<S_STREAM_EVENT_CAMERA*>(pByteCode->currentSeekPos);
	pByteCode->currentSeekPos += sizeof(S_STREAM_EVENT_CAMERA);
	this->streamEventCamera = pSeekStreamCamera;

	this->addOnGenerator.Create(this, pByteCode);
	return;
}

void CActorBasicBox::Init()
{
	CCollision* pCVar1;
	edF32VECTOR4* pBottomPosition;
	S_TARGET_STREAM_REF* pStream;
	S_ACTOR_STREAM_REF* pActorStreamRef;
	int iVar4;
	int iVar5;
	VibrationParam vibrationParam;

	pActorStreamRef = this->pActorStream;
	S_STREAM_REF<CActor>* pStreamEntry = pActorStreamRef->aEntries;
	for (iVar5 = pActorStreamRef->entryCount; iVar5 != 0; iVar5 = iVar5 + -1) {
		pStreamEntry->Init();
		pStreamEntry = pStreamEntry + 1;
	}

	IMPLEMENTATION_GUARD_LOG(
	this->addOnGenerator.Init(0);)

	CActor::Init();

	pStream = this->targetStreamRef;
	iVar5 = 0;
	if (0 < pStream->entryCount) {
		do {
			pStream->aEntries[iVar5].Init();
			iVar5 = iVar5 + 1;
		} while (iVar5 < pStream->entryCount);
	}

	this->streamEventCamera->Init();

	this->field_0x168 = (float)(int)this->field_0x16c;

	vibrationParam.field_0x0 = this->field_0x17c;
	vibrationParam.field_0x4 = this->field_0x170;
	vibrationParam.field_0x8 = this->field_0x174;
	vibrationParam.field_0xc = (this->field_0x178 * 3.141593f) / 180.0f;
	pBottomPosition = GetBottomPosition();
	vibrationParam.field_0x10 = pBottomPosition->y - this->currentLocation.y;
	vibrationParam.field_0x14 = 0.005f;
	vibrationParam.pActor = this;
	this->vibrationDyn.Init(&vibrationParam);

	this->field_0x2e0 = 0;
	pStream = this->targetStreamRef;
	iVar5 = 0;
	if (0 < pStream->entryCount) {
		do {
			pStream->aEntries[iVar5].Reset();
			iVar5 = iVar5 + 1;
		} while (iVar5 < pStream->entryCount);
	}

	this->streamEventCamera->Reset(this);

	pCVar1 = this->pCollisionData;
	pCVar1->flags_0x0 = pCVar1->flags_0x0 | 0x1000;
	pCVar1 = this->pCollisionData;
	pCVar1->flags_0x0 = pCVar1->flags_0x0 | 2;
	pCVar1 = this->pCollisionData;
	pCVar1->flags_0x0 = pCVar1->flags_0x0 | 0x80000;
	return;
}

void CActorBasicBox::Term()
{
	IMPLEMENTATION_GUARD();
}

void CActorBasicBox::Reset()
{
	IMPLEMENTATION_GUARD();
}

void CActorBasicBox::SaveContext(uint*, int)
{
	IMPLEMENTATION_GUARD();
}

void CActorBasicBox::LoadContext(uint*, int)
{
	IMPLEMENTATION_GUARD();
}

CBehaviour* CActorBasicBox::BuildBehaviour(int behaviourType)
{
	CBehaviour* pBehaviour;

	if (behaviourType == BASIC_BOX_BEHAVIOUR_STAND) {
		pBehaviour = &this->behaviourStand;
	}
	else {
		pBehaviour = CActor::BuildBehaviour(behaviourType);
	}

	return pBehaviour;
}

StateConfig* CActorBasicBox::GetStateCfg(int state)
{
	StateConfig* pStateConfig;

	if (state < 5) {
		pStateConfig = CActor::GetStateCfg(state);
	}
	else {
		pStateConfig = _gStateCfg_BAB + state + -5;
	}

	return pStateConfig;
}

int CActorBasicBox::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	IMPLEMENTATION_GUARD();
}

int CActorBasicBox::InterpretEvent(edCEventMessage* pEventMessage, undefined8 param_3, int param_4, uint* param_5)
{
	return CActor::InterpretEvent(pEventMessage, param_3, param_4, param_5);
}

void CActorBasicBox::StateBasicBoxStandInit()
{
	int curIndex = 0;

	while (true) {
		S_ACTOR_STREAM_REF* pSVar1 = this->pActorStream;
		int entryCount = 0;
		if (pSVar1 != (S_ACTOR_STREAM_REF*)0x0) {
			entryCount = pSVar1->entryCount;
		}

		if (entryCount <= curIndex) break;

		CActor* pActorRef = pSVar1->aEntries[curIndex].Get();

		if (pActorRef != (CActor*)0x0) {
			pActorRef->flags = pActorRef->flags & 0xffffff7f;
			pActorRef->flags = pActorRef->flags | 0x20;
			pActorRef->EvaluateDisplayState();
			pActorRef->flags = pActorRef->flags & 0xfffffffd;
			pActorRef->flags = pActorRef->flags | 1;
		}

		curIndex = curIndex + 1;
	}
}

StateConfig CActorBasicBox::_gStateCfg_BAB[4] = {
	StateConfig(0x0, 0x4),
	StateConfig(0x0, 0x4),
	StateConfig(6, 0x21),
	StateConfig(0x1, 0x2),
};

void CBehaviourBasicBox::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	this->pOwner = static_cast<CActorBasicBox*>(pOwner);
}

int CBehaviourBasicBox::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	return 0;
}

void CBehaviourBasicBoxStand::Manage()
{
	CActorBasicBox* pBasicBox;
	int curState;
	CAnimation* pCVar2;
	edAnmLayer* pAnimData;
	undefined8 uVar4;
	bool bVar5;
	edF32VECTOR3 local_20;
	edF32VECTOR4 local_10;

	pBasicBox = this->pOwner;

	pBasicBox->streamEventCamera->Manage(pBasicBox);

	curState = pBasicBox->actorState;

	if (curState != 8) {
		if (curState == 7) {
			if (((pBasicBox->field_0x180 & 0x80) == 0) && (0.2f < pBasicBox->timeInAir)) {
				pCVar2 = pBasicBox->pAnimationController;

				if (pCVar2 != (CAnimation*)0x0) {
					pAnimData = (pCVar2->anmBinMetaAnimator).aAnimData;

					if ((pAnimData->currentAnimDesc).animType == pCVar2->currentAnimType_0x30) {
						if (pAnimData->animPlayState == 0) {
							bVar5 = false;
						}
						else {
							bVar5 = (pAnimData->field_0xcc & 2) != 0;
						}
					}
					else {
						bVar5 = false;
					}

					if (!bVar5) goto LAB_0038cdc0;
				}

				pBasicBox->SetState(8, -1);
			}
		}
		else {
			if (curState == 6) {
				bVar5 = pBasicBox->vibrationDyn.MakeVibrate(&local_20);

				if (bVar5 != false) {
					pBasicBox->SetState(5, -1);
				}

				pBasicBox->rotationEuler.x = local_20.x;
				pBasicBox->rotationEuler.y = local_20.y;
				pBasicBox->rotationEuler.z = local_20.z;
				pBasicBox->UpdatePosition(&pBasicBox->currentLocation, false);
			}
		}
	}

LAB_0038cdc0:
	if (pBasicBox->pTiedActor != (CActor*)0x0) {
		local_10 = pBasicBox->baseLocation;
		pBasicBox->SV_UpdatePosition_Rel(&local_10, 0, 0, (CActorsTable*)0x0, (edF32VECTOR4*)0x0);
	}

	return;
}

void CBehaviourBasicBoxStand::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	this->pOwner = static_cast<CActorBasicBox*>(pOwner);

	if (newState == -1) {
		this->pOwner->SetState(5, -1);
	}
	else {
		this->pOwner->SetState(newState, newAnimationType);
	}

	return;
}

void CBehaviourBasicBoxStand::InitState(int newState)
{
	CActor* this_00;
	S_ACTOR_STREAM_REF* pSVar1;
	int iVar2;
	int iVar3;
	int iVar4;
	CActorBasicBox* pBasicBox;

	pBasicBox = this->pOwner;

	if (newState == 8) {
		/* CActor::SetDisplayType */
		pBasicBox->flags = pBasicBox->flags & 0xffffff7f;
		pBasicBox->flags = pBasicBox->flags | 0x20;
		pBasicBox->EvaluateDisplayState();

		/* CActor::SetManagementType */
		pBasicBox->flags = pBasicBox->flags & 0xfffffffd;
		pBasicBox->flags = pBasicBox->flags | 1;
	}
	else {
		if (newState == 7) {
			IMPLEMENTATION_GUARD(
			pBasicBox->StateBasicBoxExplosionInit();)
		}
		else {
			if (newState == 5) {
				pBasicBox->StateBasicBoxStandInit();
			}
		}
	}
	return;
}

void CBehaviourBasicBoxStand::TermState(int oldState, int newState)
{
	CActorBasicBox* pBasicBox;

	pBasicBox = this->pOwner;
	IMPLEMENTATION_GUARD(
	if ((oldState == 7) && ((float)pBasicBox->field_0x194 != -NAN)) {
		CActor::SV_RestoreOrgModel((CActor*)pBasicBox, &pBasicBox->actorAlternateModel);
	})
	return;
}

int CBehaviourBasicBoxStand::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	return 0;
}

void CVibrationDyn::Init(VibrationParam* pParams)
{
	edF32VECTOR3 local_10;

	this->field_0x0 = 0;
	this->field_0x4 = 0;
	this->field_0x8 = 0;
	this->field_0xc = 0;
	this->field_0x10 = 0;
	this->field_0x14 = 0;
	this->field_0x18 = 0;
	this->field_0x1c = 0;
	this->field_0x20 = 0;
	this->field_0x24 = 0;
	this->field_0x28 = 1;
	this->field_0x2c = 1;
	this->field_0x30 = 1;
	this->field_0x31 = 1;
	this->field_0x38 = 1.0f;

	this->field_0x3c = pParams->field_0x0;
	this->field_0x40 = pParams->field_0x4;
	this->field_0x44 = pParams->field_0x8;

	this->field_0x34 = pParams->pActor;

	this->field_0x48 = pParams->field_0xc;
	this->field_0x4c = pParams->field_0x10;
	this->field_0x54 = pParams->field_0x14;

	local_10.x = (this->field_0x34->rotationEuler).x;
	local_10.y = (this->field_0x34->rotationEuler).y;
	local_10.z = (this->field_0x34->rotationEuler).z;
	edF32Matrix4FromEulerSoft(&this->field_0x60, &local_10, "XYZ");
}

int CVibrationDyn::UpdateKineticToPotentialUpDown()
{
	Timer* pTVar1;
	int iVar2;

	if (fabs(this->field_0x24) < fabs(this->field_0x1c)) {
		this->field_0x4 = this->field_0x4 + this->field_0x24;
		this->field_0x1c = this->field_0x1c - this->field_0x24;
		this->field_0x14 = this->field_0x14 + this->field_0x24;
		iVar2 = 1;
	}
	else {
		this->field_0x14 = this->field_0x14 + this->field_0x1c;

		if (this->field_0x31 != 0) {
			iVar2 = 1;
			if (this->field_0x24 <= 0.0f) {
				iVar2 = -1;
			}

			pTVar1 = GetTimer();
			this->field_0x24 = ((float)iVar2 * fabs(this->field_0x14) * pTVar1->cutsceneDeltaTime) / this->field_0x44;
			this->field_0x31 = 0;
		}

		iVar2 = 0;
		this->field_0x14 = this->field_0x14 * this->field_0x3c;
		this->field_0x1c = 0.0f;
		this->field_0x4 = this->field_0x14;
		this->field_0x24 = this->field_0x24 * -1.0f;
	}
	return iVar2;
}

int CVibrationDyn::ComputeCurFactor()
{
	bool bVar1;
	bool bVar2;
	bool bVar3;
	Timer* pTVar4;
	undefined4 uVar5;
	uint uVar6;
	int iVar7;

	bVar2 = false;
	bVar3 = false;

	if (this->field_0x54 < fabs(this->field_0x10) + fabs(this->field_0x18)) {
		if (this->field_0x28 == 0) {
			bVar1 = fabs(this->field_0x20) < fabs(this->field_0x10);

			if (bVar1) {
				this->field_0x0 = this->field_0x0 + this->field_0x20;
				this->field_0x18 = this->field_0x18 + this->field_0x20;
				this->field_0x10 = this->field_0x10 + this->field_0x20;
			}
			else {
				this->field_0x18 = this->field_0x18 + this->field_0x10;
				this->field_0x10 = 0.0;
				this->field_0x0 = 0.0;
			}

			this->field_0x28 = (uint)!bVar1;
		}
		else {
			if (fabs(this->field_0x20) < fabs(this->field_0x18)) {
				this->field_0x0 = this->field_0x0 + this->field_0x20;
				this->field_0x18 = this->field_0x18 - this->field_0x20;
				this->field_0x10 = this->field_0x10 + this->field_0x20;
				uVar5 = 1;
			}
			else {
				this->field_0x10 = this->field_0x10 + this->field_0x18;
				if (this->field_0x30 != 0) {
					iVar7 = 1;

					if (this->field_0x20 <= 0.0f) {
						iVar7 = -1;
					}

					pTVar4 = GetTimer();
					this->field_0x20 = ((float)iVar7 * fabs(this->field_0x10) * pTVar4->cutsceneDeltaTime) / this->field_0x44;
					this->field_0x30 = 0;
				}

				uVar5 = 0;
				this->field_0x10 = this->field_0x10 * this->field_0x3c;
				this->field_0x18 = 0.0f;
				this->field_0x0 = this->field_0x10;
				this->field_0x20 = this->field_0x20 * -1.0;
			}

			this->field_0x28 = uVar5;
		}
	}
	else {
		bVar2 = true;
	}

	if (this->field_0x54 < fabs(this->field_0x14) + fabs(this->field_0x1c)) {
		if (this->field_0x2c == 0) {
			bVar1 = fabs(this->field_0x24) < fabs(this->field_0x14);
			if (bVar1) {
				this->field_0x4 = this->field_0x4 + this->field_0x24;
				this->field_0x1c = this->field_0x1c + this->field_0x24;
				this->field_0x14 = this->field_0x14 + this->field_0x24;
			}
			else {
				this->field_0x1c = this->field_0x1c + this->field_0x14;
				this->field_0x14 = 0.0f;
				this->field_0x4 = 0.0f;
			}
			this->field_0x2c = (uint)!bVar1;
		}
		else {
			uVar6 = UpdateKineticToPotentialUpDown();
			this->field_0x2c = uVar6 & 0xff;
		}
	}
	else {
		bVar3 = true;
	}

	iVar7 = 0;

	if ((bVar2) && (bVar3)) {
		iVar7 = 1;
	}

	return iVar7;
}

bool CVibrationDyn::MakeVibrate(edF32VECTOR3* param_2)
{
	int iVar1;
	edF32VECTOR4 local_e0;
	edF32VECTOR4 local_d0;
	edF32MATRIX4 eStack192;
	edF32MATRIX4 eStack128;
	edF32MATRIX4 eStack64;

	iVar1 = ComputeCurFactor();

	if (iVar1 == 0) {
		edF32Matrix4SetIdentityHard(&eStack64);
		local_d0.y = 0.0f;
		local_d0.x = 1.0f;
		local_e0.z = 1.0f;
		local_d0.z = 0.0f;
		local_d0.w = 0.0f;
		local_e0.x = 0.0f;
		local_e0.y = 0.0f;
		local_e0.w = 0.0f;

		edF32Matrix4BuildFromVectorAndAngle(this->field_0x48 * this->field_0x4, &eStack128, &local_d0);
		edF32Matrix4BuildFromVectorAndAngle(this->field_0x48 * this->field_0x0, &eStack192, &local_e0);
		edF32Matrix4MulF32Matrix4Hard(&eStack64, &this->field_0x60, &eStack64);
		edF32Matrix4MulF32Matrix4Hard(&eStack128, &eStack192, &eStack128);
		edF32Matrix4MulF32Matrix4Hard(&eStack64, &eStack128, &eStack64);
		edF32Matrix4ToEulerSoft(&eStack64, param_2, "XYZ");
	}
	else {
		this->field_0x0 = 0.0;
		this->field_0x4 = 0.0;
		this->field_0x18 = 0.0;
		this->field_0x1c = 0.0;
		this->field_0x10 = 0.0;
		this->field_0x14 = 0.0;
		this->field_0x20 = 0.0;
		this->field_0x24 = 0.0;
		this->field_0x38 = 1.0;
		this->field_0x28 = 1;
		this->field_0x2c = 1;

		edF32Matrix4ToEulerSoft(&this->field_0x60, param_2, "XYZ");
	}

	return iVar1 != 0;
}
