#include "ActorBasicBox.h"
#include "MemoryStream.h"
#include "CollisionManager.h"
#include "MathOps.h"
#include "TimeController.h"

#define BASIC_BOX_BEHAVIOUR_STAND 2

#define BASIC_BOX_STAND_STATE_STAND 5
#define BASIC_BOX_STAND_STATE_EXPLODE 7
#define BASIC_BOX_STAND_STATE_DESTROYED 8

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
	this->lifeBase.maxValue = (float)(int)(float)pByteCode->GetU32();
	
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

	targetSwitch.Create(pByteCode);

	this->addOnGenerator.Create(this, pByteCode);
	return;
}

void CActorBasicBox::Init()
{
	CCollision* pCVar1;
	edF32VECTOR4* pBottomPosition;
	S_NTF_TARGET_STREAM_REF* pStream;
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

	targetSwitch.Init();

	lifeBase.SetValue(this->lifeBase.GetValueMax());

	vibrationParam.field_0x0 = this->field_0x17c;
	vibrationParam.field_0x4 = this->field_0x170;
	vibrationParam.field_0x8 = this->field_0x174;
	vibrationParam.field_0xc = (this->field_0x178 * 3.141593f) / 180.0f;
	pBottomPosition = GetBottomPosition();
	vibrationParam.field_0x10 = pBottomPosition->y - this->currentLocation.y;
	vibrationParam.field_0x14 = 0.005f;
	vibrationParam.pActor = this;
	this->vibrationDyn.Init(&vibrationParam);

	this->field_0x2e0 = 0.0f;

	targetSwitch.Reset(this);

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
	S_NTF_TARGET_STREAM_REF* pSVar3;
	int iVar4;
	int iVar5;
	VibrationParam local_20;

	CActor::Reset();

	lifeBase.SetValue(this->lifeBase.GetValueMax());
	local_20.field_0x0 = this->field_0x17c;
	local_20.field_0x4 = this->field_0x170;
	local_20.field_0x8 = this->field_0x174;
	local_20.field_0xc = (this->field_0x178 * 3.141593f) / 180.0f;

	peVar2 = GetBottomPosition();
	local_20.field_0x10 = peVar2->y - this->currentLocation.y;
	local_20.field_0x14 = 0.005f;
	local_20.pActor = this;
	this->vibrationDyn.Init(&local_20);

	this->field_0x2e0 = 0.0f;

	targetSwitch.Reset(this);

	pCVar1 = this->pCollisionData;
	pCVar1->flags_0x0 = pCVar1->flags_0x0 | 0x1000;
	pCVar1 = this->pCollisionData;
	pCVar1->flags_0x0 = pCVar1->flags_0x0 | 2;
	pCVar1 = this->pCollisionData;
	pCVar1->flags_0x0 = pCVar1->flags_0x0 | 0x80000;

	return;
}

struct S_SAVE_CLASS_BASIC_BOX
{
	uint field_0x0;
};

void CActorBasicBox::SaveContext(void* pData, uint mode, uint maxSize)
{
	S_SAVE_CLASS_BASIC_BOX* pSaveData = reinterpret_cast<S_SAVE_CLASS_BASIC_BOX*>(pData);

	if (mode == 1) {
		pSaveData->field_0x0 = (uint)(0.0f < lifeBase.GetValue());
	}

	return;
}

void CActorBasicBox::LoadContext(void* pData, uint mode, uint maxSize)
{
	S_SAVE_CLASS_BASIC_BOX* pSaveData = reinterpret_cast<S_SAVE_CLASS_BASIC_BOX*>(pData);

	if ((mode == 1) && (pSaveData->field_0x0 == 0)) {
		lifeBase.SetValue(0.0f);
		this->flags = this->flags & 0xffffff7f;
		this->flags = this->flags | 0x20;

		EvaluateDisplayState();

		this->flags = this->flags & 0xfffffffd;
		this->flags = this->flags | 1;

		SetBehaviour(BASIC_BOX_BEHAVIOUR_STAND, BASIC_BOX_STAND_STATE_DESTROYED, -1);
	}
	return;
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
	int result;
	long lVar3;
	float fVar4;
	edF32VECTOR4 local_60;
	edF32VECTOR4 eStack80;
	edF32VECTOR4 local_40;
	edF32VECTOR4 eStack48;
	edF32VECTOR4 local_20;
	edF32VECTOR4 eStack16;

	_msg_hit_param* pHitParam = reinterpret_cast<_msg_hit_param*>(pMsgParam);
	if (msg == 3) {
		if (lifeBase.GetValue() <= 1.0f) {
			result = 0;
		}
		else {
			SetState(BASIC_BOX_STAND_STATE_EXPLODE, -1);
			result = 1;
		}
	}
	else {
		if (msg == MESSAGE_GET_VISUAL_DETECTION_POINT) {
			GetPositionMsgParams* pGetPosMsgParams = reinterpret_cast<GetPositionMsgParams*>(pMsgParam);
			edF32Vector4SubHard(&eStack16, GetTopPosition(), &this->currentLocation);
			fVar4 = edF32Vector4DotProductHard(&eStack16, &this->pMeshTransform->base.transformA.rowY);
			edF32Vector4ScaleHard(fVar4, &eStack16, &this->pMeshTransform->base.transformA.rowY);
			edF32Vector4SubHard(&pGetPosMsgParams->vectorFieldB, GetBottomPosition(), GetTopPosition());
			fVar4 = edF32Vector4DotProductHard(&pGetPosMsgParams->vectorFieldB, &this->pMeshTransform->base.transformA.rowY);
			edF32Vector4ScaleHard(fVar4 * 0.5f, &pGetPosMsgParams->vectorFieldB, &this->pMeshTransform->base.transformA.rowY);
			edF32Vector4AddHard(&pGetPosMsgParams->vectorFieldB, &pGetPosMsgParams->vectorFieldB, &eStack16);
			result = 1;
		}
		else {
			if (msg == 2) {
				if (lifeBase.GetValue() <= 0.0f) {
					result = 0;
				}
				else {
					this->field_0x2e0 = pHitParam->field_0x30;

					switch (pHitParam->projectileType) {
					case HIT_TYPE_BOOMY:
						if ((this->field_0x180 & 2) != 0) {
							ApplyHit(&pHitParam);

							return 1;
						}
						break;
					case 7:
						IMPLEMENTATION_GUARD(
						if ((this->field_0x180 & 0x40) != 0) {
							FUN_00114fc0(pHitParam->damage);
							if (lifeBase.GetValue() <= 1.0f) {
								peVar1 = pHitParam->field_0x40;
								if ((this->field_0x180 & 1) != 0) {
									if ((this->flags & 0x1000) != 0) {
										SetVectorFromAngles(&this->rotationQuat, &this->rotationEuler);
									}

									edF32Vector4SubHard(&eStack80, &this->currentLocation, peVar1);

									fVar4 = edF32Vector4DotProductHard(&eStack80, &this->rotationQuat);
									if (fVar4 < 1.0f) {
										edF32Vector4ScaleHard(-1.0f, &local_60, &this->rotationQuat);
										this->rotationQuat = local_60;

										if ((this->flags & 0x1000) != 0) {
											GetAnglesFromVector(&this->rotationEuler, &this->rotationQuat);
										}
									}
								}

								SetState(BASIC_BOX_STAND_STATE_EXPLODE, -1);

								return 1;
							}

							result = FUN_0038ce50((int)this, (edF32VECTOR4*)&this->field_0x290, &pHitParam->field_0x40, 0);
							if (result != 0) {
								(this->vibrationDyn).field_0x40 = this->field_0x170;
								FUN_00116a00(pHitParam->field_0x30, (float*)&this->vibrationDyn,
									(edF32VECTOR4*)&this->field_0x290, &pHitParam->field_0x40);
								SetState(6, -1);

								return 1;
							}
						})
						break;
					case 8:
						if ((this->field_0x180 & 4) != 0) {
							if ((this->field_0x180 & 1) != 0) {
								if ((this->flags & 0x1000) != 0) {
									SetVectorFromAngles(&this->rotationQuat, &this->rotationEuler.xyz);
								}

								edF32Vector4SubHard(&eStack48, &this->currentLocation, &pHitParam->field_0x40);

								fVar4 = edF32Vector4DotProductHard(&eStack48, &this->rotationQuat);
								if (fVar4 < 1.0f) {
									edF32Vector4ScaleHard(-1.0f, &local_40, &this->rotationQuat);
									this->rotationQuat = local_40;

									if ((this->flags & 0x1000) != 0) {
										GetAnglesFromVector(&this->rotationEuler.xyz, &this->rotationQuat);
									}
								}
							}

							SetState(BASIC_BOX_STAND_STATE_EXPLODE, -1);

							return 1;
						}
						break;
					case 9:
						if ((this->field_0x180 & 8) != 0) {
							ApplyHit(&pHitParam);

							return 1;
						}
						break;
					case 10:
						if (((this->field_0x180 & 0x10) != 0) && (1.0f < pHitParam->damage)) {
							if ((this->field_0x180 & 1) != 0) {
								if ((this->flags & 0x1000) != 0) {
									SetVectorFromAngles(&this->rotationQuat, &this->rotationEuler.xyz);
								}

								fVar4 = edF32Vector4DotProductHard(&pHitParam->field_0x20, &this->rotationQuat);
								if (fVar4 < 1.0f) {
									edF32Vector4ScaleHard(-1.0f, &local_20, &this->rotationQuat);
									this->rotationQuat = local_20;

									if ((this->flags & 0x1000) != 0) {
										GetAnglesFromVector(&this->rotationEuler.xyz, &this->rotationQuat);
									}
								}
							}

							SetState(BASIC_BOX_STAND_STATE_EXPLODE, -1);

							return 1;
						}
						break;
					case 0xb:
						if ((this->field_0x180 & 0x20) != 0) {
							pHitParam->damage = lifeBase.GetValueMax();
							ApplyHit(&pHitParam);

							return 1;
						}
					}

					result = 0;
				}
			}
			else {
				result = CActor::InterpretMessage(pSender, msg, pMsgParam);
			}
		}
	}

	return result;
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

void CActorBasicBox::StateBasicBoxExplosionInit()
{
	S_ACTOR_STREAM_REF* pSVar1;
	KyaUpdateObjA* pKVar2;
	CCollision* pCVar3;
	CNewFx* pCVar4;
	undefined8 uVar5;
	int iVar6;
	edF32VECTOR4* peVar7;
	S_NTF_TARGET_STREAM_REF* pSVar8;
	int iVar9;
	int iVar10;
	edF32VECTOR4 local_20;
	edF32VECTOR4 local_10;
	CActor* pActor;

	iVar9 = 0;
	while (true) {
		pSVar1 = this->pActorStream;
		iVar6 = 0;

		if (pSVar1 != (S_ACTOR_STREAM_REF*)0x0) {
			iVar6 = pSVar1->entryCount;
		}

		if (iVar6 <= iVar9) break;

		pActor = pSVar1->aEntries[iVar9].Get();
		if (pActor != (CActor*)0x0) {
			pActor->flags = pActor->flags & 0xffffff5f;
			pActor->EvaluateDisplayState();
			pActor->flags = pActor->flags & 0xfffffffc;
		}

		iVar9 = iVar9 + 1;
	}

	local_10 = this->currentLocation;

	peVar7 = GetBottomPosition();
	local_10.y = local_10.y + (peVar7->y - this->currentLocation.y) / 2.0f;

	this->addOnGenerator.Generate(&local_10);

	if (this->field_0x194 != -1) {
		pKVar2 = this->subObjA;
		local_20 = pKVar2->boundingSphere;
		if (local_20.w < 20.0f) {
			local_20.w = 20.0f;
		}

		SV_SwitchToModel(&this->actorAlternateModel, this->field_0x194, (this->pCinData)->textureIndex, &local_20);
	}

	pCVar3 = this->pCollisionData;
	pCVar3->flags_0x0 = pCVar3->flags_0x0 & 0xffffefff;
	pCVar3 = this->pCollisionData;
	pCVar3->flags_0x0 = pCVar3->flags_0x0 & 0xfffffffd;
	pCVar3 = this->pCollisionData;
	pCVar3->flags_0x0 = pCVar3->flags_0x0 & 0xfff7ffff;

	CScene::ptable.g_EffectsManager_004516b8->GetDynamicFx(&this->classObj_0x1e4, this->field_0x160, FX_MATERIAL_SELECTOR_NONE);
	pCVar4 = (this->classObj_0x1e4).pFx;
	if (((pCVar4 != (CNewFx*)0x0) && (iVar10 = (this->classObj_0x1e4).id, iVar10 != 0)) && (iVar10 == pCVar4->id)) {
		pCVar4->SpatializeOnActor(6, this, 0);
	}
	pCVar4 = (this->classObj_0x1e4).pFx;
	if (((pCVar4 != (CNewFx*)0x0) && (iVar10 = (this->classObj_0x1e4).id, iVar10 != 0)) && (iVar10 == pCVar4->id)) {
		pCVar4->Start(0, 0);
	}

	targetSwitch.Switch(this);

	return;
}

void CActorBasicBox::ApplyHit(_msg_hit_param** ppHitParam)
{
	_msg_hit_param* p_Var1;
	undefined8 uVar2;
	C3DFileManager* pFileManager;
	ed_g2d_manager* peVar3;
	edF32VECTOR4* peVar4;
	float fVar5;
	edF32VECTOR4 local_20;
	edF32VECTOR4 local_10;

	pFileManager = CScene::ptable.g_C3DFileManager_00451664;

	lifeBase.LifeDecrease((*ppHitParam)->damage);

	fVar5 = lifeBase.GetValue();
	if ((lifeBase.GetValueMax() * 2.0f) / 3.0f <= fVar5) {
		if (this->field_0x18c != -1) {
			peVar3 = pFileManager->GetActorsCommonMaterial(this->field_0x18c);
			SV_PatchG2D(peVar3);
		}
	}
	else {
		if (lifeBase.GetValueMax() / 3.0f <= fVar5) {
			if (this->field_0x190 != -1) {
				peVar3 = pFileManager->GetActorsCommonMaterial(this->field_0x190);
				SV_PatchG2D(peVar3);
			}
		}
		else {
			if (fVar5 <= 0.0f) {
				(*ppHitParam)->field_0x74 = 1;
				p_Var1 = *ppHitParam;

				if ((this->field_0x180 & 1) != 0) {
					if ((this->flags & 0x1000) != 0) {
						SetVectorFromAngles(&this->rotationQuat, &this->rotationEuler.xyz);
					}

					fVar5 = edF32Vector4DotProductHard(&p_Var1->field_0x20, &this->rotationQuat);
					if (fVar5 < 0.0) {
						edF32Vector4ScaleHard(-1.0f, &local_20, &this->rotationQuat);
						this->rotationQuat = local_20;
						if ((this->flags & 0x1000) != 0) {
							GetAnglesFromVector(&this->rotationEuler.xyz, &this->rotationQuat);
						}
					}
				}

				SetState(BASIC_BOX_STAND_STATE_EXPLODE, -1);
				return;
			}
		}
	}

	(this->vibrationDyn).field_0x40 = 5.1f;
	local_10 = this->currentLocation;
	peVar4 = GetBottomPosition();
	local_10.y = local_10.y + (peVar4->y - this->currentLocation.y);
	this->vibrationDyn.UpdateAllFactors((*ppHitParam)->damage, &(*ppHitParam)->field_0x20, &local_10);
	SetState(6, -1);

	return;
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

	pBasicBox->targetSwitch.pStreamEventCamera->Manage(pBasicBox);

	curState = pBasicBox->actorState;

	if (curState != BASIC_BOX_STAND_STATE_DESTROYED) {
		if (curState == BASIC_BOX_STAND_STATE_EXPLODE) {
			if (((pBasicBox->field_0x180 & 0x80) == 0) && (0.2f < pBasicBox->timeInAir)) {
				pCVar2 = pBasicBox->pAnimationController;

				if (pCVar2 != (CAnimation*)0x0) {
					if (!pCVar2->IsCurrentLayerAnimEndReached(0)) goto LAB_0038cdc0;
				}

				pBasicBox->SetState(BASIC_BOX_STAND_STATE_DESTROYED, -1);
			}
		}
		else {
			if (curState == 6) {
				bVar5 = pBasicBox->vibrationDyn.MakeVibrate(&local_20);

				if (bVar5 != false) {
					pBasicBox->SetState(BASIC_BOX_STAND_STATE_STAND, -1);
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
		this->pOwner->SetState(BASIC_BOX_STAND_STATE_STAND, -1);
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

	if (newState == BASIC_BOX_STAND_STATE_DESTROYED) {
		/* CActor::SetDisplayType */
		pBasicBox->flags = pBasicBox->flags & 0xffffff7f;
		pBasicBox->flags = pBasicBox->flags | 0x20;
		pBasicBox->EvaluateDisplayState();

		/* CActor::SetManagementType */
		pBasicBox->flags = pBasicBox->flags & 0xfffffffd;
		pBasicBox->flags = pBasicBox->flags | 1;
	}
	else {
		if (newState == BASIC_BOX_STAND_STATE_EXPLODE) {
			pBasicBox->StateBasicBoxExplosionInit();
		}
		else {
			if (newState == BASIC_BOX_STAND_STATE_STAND) {
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

	if ((oldState == BASIC_BOX_STAND_STATE_EXPLODE) && (pBasicBox->field_0x194 != -1)) {
		pBasicBox->SV_RestoreOrgModel(&pBasicBox->actorAlternateModel);
	}

	return;
}

int CBehaviourBasicBoxStand::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	return 0;
}

void CLifeBase::SetValue(float newValue)
{
	this->value = (float)(int)newValue;

	return;
}

float CLifeBase::GetValue()
{
	return this->value;
}

float CLifeBase::GetValueMax()
{
	return this->maxValue;
}
