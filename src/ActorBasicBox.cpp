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

	S_TARGET_STREAM_REF::Create(&this->targetStreamRef, pByteCode);
	
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

	this->addOnGenerator.Init(0);

	CActor::Init();

	this->targetStreamRef->Init();
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

	this->targetStreamRef->Reset();
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
	CActor::Term();
	this->addOnGenerator.Term();

	return;
}

void CActorBasicBox::Reset()
{
	CCollision* pCVar1;
	edF32VECTOR4* peVar2;
	S_TARGET_STREAM_REF* pSVar3;
	int iVar4;
	int iVar5;
	VibrationParam local_20;

	CActor::Reset();

	this->field_0x168 = (float)(int)this->field_0x16c;
	local_20.field_0x0 = this->field_0x17c;
	local_20.field_0x4 = this->field_0x170;
	local_20.field_0x8 = this->field_0x174;
	local_20.field_0xc = (this->field_0x178 * 3.141593f) / 180.0f;

	peVar2 = GetBottomPosition();
	local_20.field_0x10 = peVar2->y - this->currentLocation.y;
	local_20.field_0x14 = 0.005f;
	local_20.pActor = this;
	this->vibrationDyn.Init(&local_20);

	this->field_0x2e0 = 0;

	this->targetStreamRef->Reset();
	this->streamEventCamera->Reset(this);

	pCVar1 = this->pCollisionData;
	pCVar1->flags_0x0 = pCVar1->flags_0x0 | 0x1000;
	pCVar1 = this->pCollisionData;
	pCVar1->flags_0x0 = pCVar1->flags_0x0 | 2;
	pCVar1 = this->pCollisionData;
	pCVar1->flags_0x0 = pCVar1->flags_0x0 | 0x80000;

	return;
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
	edF32VECTOR4* peVar1;
	edF32VECTOR4* v2;
	int iVar2;
	long lVar3;
	float fVar4;
	void* local_64;
	edF32VECTOR4 local_60;
	edF32VECTOR4 eStack80;
	edF32VECTOR4 local_40;
	edF32VECTOR4 eStack48;
	edF32VECTOR4 local_20;
	edF32VECTOR4 eStack16;

	lVar3 = (long)(int)this;
	local_64 = pMsgParam;
	if (msg == 3) {
		IMPLEMENTATION_GUARD(
		if (this->field_0x168 <= 1.0f) {
			iVar2 = 0;
		}
		else {
			(*((this->base).pVTable)->SetState)((CActor*)this, 7, -1);
			iVar2 = 1;
		})
	}
	else {
		if (msg == MESSAGE_GET_VISUAL_DETECTION_POINT) {
			GetPositionMsgParams* pGetPosMsgParams = reinterpret_cast<GetPositionMsgParams*>(pMsgParam);
			peVar1 = GetTopPosition();
			edF32Vector4SubHard(&eStack16, peVar1, &this->currentLocation);
			fVar4 = edF32Vector4DotProductHard(&eStack16, &this->pMeshTransform->base.transformA.rowY);
			edF32Vector4ScaleHard(fVar4, &eStack16, &this->pMeshTransform->base.transformA.rowY);
			peVar1 = GetBottomPosition();
			v2 = GetTopPosition();
			edF32Vector4SubHard(&pGetPosMsgParams->vectorFieldB, peVar1, v2);
			fVar4 = edF32Vector4DotProductHard(&pGetPosMsgParams->vectorFieldB, &this->pMeshTransform->base.transformA.rowY);
			edF32Vector4ScaleHard(fVar4 * 0.5f, &pGetPosMsgParams->vectorFieldB, &this->pMeshTransform->base.transformA.rowY);
			edF32Vector4AddHard(&pGetPosMsgParams->vectorFieldB, &pGetPosMsgParams->vectorFieldB, &eStack16);
			iVar2 = 1;
		}
		else {
			if (msg == 2) {
				IMPLEMENTATION_GUARD(
				if (this->field_0x168 <= 1.0f) {
					iVar2 = 0;
				}
				else {
					this->field_0x2e0 = *(undefined4*)((int)pMsgParam + 0x30);
					/* WARNING: Load size is inaccurate */
					switch (*pMsgParam) {
					case 4:
						if ((this->field_0x180 & 2) != 0) {
							FUN_0038d5d0(lVar3, (int*)&local_64);
							return 1;
						}
						break;
					case 7:
						if ((this->field_0x180 & 0x40) != 0) {
							FUN_00114fc0(*(undefined4*)((int)pMsgParam + 0xc));
							if (this->field_0x168 <= 1.0f) {
								peVar1 = (edF32VECTOR4*)((int)local_64 + 0x40);
								if ((this->field_0x180 & 1) != 0) {
									if (((this->base).flags & 0x1000) != 0) {
										SetVectorFromAngles(&(this->base).rotationQuat, (edF32VECTOR3*)&(this->base).rotationEuler);
									}
									edF32Vector4SubHard(&eStack80, &(this->base).currentLocation, peVar1);
									fVar4 = edF32Vector4DotProductHard(&eStack80, &(this->base).rotationQuat);
									if (fVar4 < 1.0f) {
										edF32Vector4ScaleHard((float)&DAT_bf800000, &local_60, &(this->base).rotationQuat);
										(this->base).rotationQuat.x = local_60.x;
										(this->base).rotationQuat.y = local_60.y;
										(this->base).rotationQuat.z = local_60.z;
										(this->base).rotationQuat.w = local_60.w;
										if (((this->base).flags & 0x1000) != 0) {
											GetAnglesFromVector((edF32VECTOR3*)&(this->base).rotationEuler, &(this->base).rotationQuat);
										}
									}
								}
								(*((this->base).pVTable)->SetState)((CActor*)this, 7, -1);
								return 1;
							}
							iVar2 = FUN_0038ce50((int)this, (edF32VECTOR4*)&this->field_0x290, (edF32VECTOR4*)((int)local_64 + 0x40), 0
							);
							if (iVar2 != 0) {
								(this->vibrationDyn).field_0x40 = this->field_0x170;
								FUN_00116a00(*(float*)((int)local_64 + 0x30), (float*)&this->vibrationDyn,
									(edF32VECTOR4*)&this->field_0x290, (edF32VECTOR4*)((int)local_64 + 0x40));
								(*((this->base).pVTable)->SetState)((CActor*)this, 6, -1);
								return 1;
							}
						}
						break;
					case 8:
						if ((this->field_0x180 & 4) != 0) {
							if ((this->field_0x180 & 1) != 0) {
								if (((this->base).flags & 0x1000) != 0) {
									SetVectorFromAngles(&(this->base).rotationQuat, (edF32VECTOR3*)&(this->base).rotationEuler);
								}
								edF32Vector4SubHard(&eStack48, &(this->base).currentLocation, (edF32VECTOR4*)((int)pMsgParam + 0x40));
								fVar4 = edF32Vector4DotProductHard(&eStack48, &(this->base).rotationQuat);
								if (fVar4 < 1.0f) {
									edF32Vector4ScaleHard((float)&DAT_bf800000, &local_40, &(this->base).rotationQuat);
									(this->base).rotationQuat.x = local_40.x;
									(this->base).rotationQuat.y = local_40.y;
									(this->base).rotationQuat.z = local_40.z;
									(this->base).rotationQuat.w = local_40.w;
									if (((this->base).flags & 0x1000) != 0) {
										GetAnglesFromVector((edF32VECTOR3*)&(this->base).rotationEuler, &(this->base).rotationQuat);
									}
								}
							}
							(*((this->base).pVTable)->SetState)((CActor*)this, 7, -1);
							return 1;
						}
						break;
					case 9:
						if ((this->field_0x180 & 8) != 0) {
							FUN_0038d5d0(lVar3, (int*)&local_64);
							return 1;
						}
						break;
					case 10:
						if (((this->field_0x180 & 0x10) != 0) && (1.0f < *(float*)((int)pMsgParam + 0xc))) {
							if ((this->field_0x180 & 1) != 0) {
								if (((this->base).flags & 0x1000) != 0) {
									SetVectorFromAngles(&(this->base).rotationQuat, (edF32VECTOR3*)&(this->base).rotationEuler);
								}
								fVar4 = edF32Vector4DotProductHard((edF32VECTOR4*)((int)pMsgParam + 0x20), &(this->base).rotationQuat);
								if (fVar4 < 1.0f) {
									edF32Vector4ScaleHard((float)&DAT_bf800000, &local_20, &(this->base).rotationQuat);
									(this->base).rotationQuat.x = local_20.x;
									(this->base).rotationQuat.y = local_20.y;
									(this->base).rotationQuat.z = local_20.z;
									(this->base).rotationQuat.w = local_20.w;
									if (((this->base).flags & 0x1000) != 0) {
										GetAnglesFromVector((edF32VECTOR3*)&(this->base).rotationEuler, &(this->base).rotationQuat);
									}
								}
							}
							(*((this->base).pVTable)->SetState)((CActor*)this, 7, -1);
							return 1;
						}
						break;
					case 0xb:
						if ((this->field_0x180 & 0x20) != 0) {
							*(float*)((int)pMsgParam + 0xc) = this->field_0x16c;
							FUN_0038d5d0(lVar3, (int*)&local_64);
							return 1;
						}
					}
					iVar2 = 0;
				})
			}
			else {
				iVar2 = CActor::InterpretMessage(pSender, msg, (GetPositionMsgParams*)pMsgParam);
			}
		}
	}

	return iVar2;
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
					if (!pCVar2->IsCurrentLayerAnimEndReached(0)) goto LAB_0038cdc0;
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

	if ((oldState == 7) && (pBasicBox->field_0x194 != -1)) {
		IMPLEMENTATION_GUARD(
		CActor::SV_RestoreOrgModel((CActor*)pBasicBox, &pBasicBox->actorAlternateModel);)
	}

	return;
}

int CBehaviourBasicBoxStand::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	return 0;
}
