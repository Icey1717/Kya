#include "ActorBox.h"
#include "MemoryStream.h"
#include "MathOps.h"
#include "FileManager3D.h"

void CActorBox::Create(ByteCode* pByteCode)
{
	char* pcVar2;
	int iVar3;
	uint uVar4;
	float fVar5;

	CActorAutonomous::Create(pByteCode);

	this->field_0x350 = pByteCode->GetS32();

	S_ACTOR_STREAM_REF* pAVar1 = (S_ACTOR_STREAM_REF*)pByteCode->currentSeekPos;
	pByteCode->currentSeekPos = pByteCode->currentSeekPos + 4;
	if (pAVar1->entryCount != 0x0) {
		pByteCode->currentSeekPos = pByteCode->currentSeekPos + pAVar1->entryCount * sizeof(int);
	}
	this->field_0x380 = pAVar1;

	this->field_0x354 = pByteCode->GetF32();
	this->field_0x358 = pByteCode->GetF32();
	this->field_0x35c = pByteCode->GetF32();
	this->field_0x360 = pByteCode->GetF32();

	this->field_0x364 = pByteCode->GetU32();
	this->field_0x370 = pByteCode->GetS32();
	this->field_0x374 = pByteCode->GetS32();
	this->field_0x378 = pByteCode->GetS32();
	this->field_0x37c = pByteCode->GetS32();

	this->ntfSwitch.Create(pByteCode);
	this->addOnGenerator.Create(this, pByteCode);

	this->pCollisionData->flags_0x0 = this->pCollisionData->flags_0x0 | 0x180;

	return;
}

void CActorBox::Init()
{
	int iVar3;
	C3DFileManager* pFileManager;

	S_STREAM_REF<CActor>* pStreamRefActor = this->field_0x380->aEntries;
	for (iVar3 = this->field_0x380->entryCount; iVar3 != 0; iVar3 = iVar3 + -1) {
		pStreamRefActor->Init();
		pStreamRefActor = pStreamRefActor + 1;
	}

	this->addOnGenerator.Init(1);
	CActorAutonomous::Init();

	pFileManager = CScene::ptable.g_C3DFileManager_00451664;

	this->ntfSwitch.Init();

	int texIndex = this->pCinData->textureIndex;
	if (texIndex != -1) {
		SV_PatchG2D(pFileManager->GetActorsCommonMaterial(texIndex));
	}

	this->field_0x3e0 = GetBottomPosition()->y - this->currentLocation.y;

	this->field_0x3d4 = (CActor*)0x0;
	this->field_0x3d8 = (CActor*)0x0;

	VibrationParam vibParams;
	vibParams.field_0x0 = this->field_0x360;
	vibParams.field_0x4 = this->field_0x354;
	vibParams.field_0x8 = this->field_0x358;
	vibParams.field_0xc = this->field_0x35c * 0.01745329f;
	vibParams.field_0x10 = this->field_0x3e0;
	vibParams.field_0x14 = 0.005f;
	vibParams.pActor = (CActor*)this;
	this->vibrationDyn.Init(&vibParams);

	this->ntfSwitch.Reset(this);

	CCollision* pCVar1;
	pCVar1 = this->pCollisionData;
	pCVar1->flags_0x0 = pCVar1->flags_0x0 | 0x1000;
	pCVar1 = this->pCollisionData;
	pCVar1->flags_0x0 = pCVar1->flags_0x0 | 2;
	pCVar1 = this->pCollisionData;
	pCVar1->flags_0x0 = pCVar1->flags_0x0 | 0x80000;

	return;
}

void CActorBox::Term()
{
	CActorAutonomous::Term();
	this->addOnGenerator.Term();

	return;
}

void CActorBox::Reset()
{
	CActorAutonomous::Reset();

	int texIndex = this->pCinData->textureIndex;
	if (texIndex != -1) {
		SV_PatchG2D(CScene::ptable.g_C3DFileManager_00451664->GetActorsCommonMaterial(texIndex));
	}

	this->field_0x3d4 = (CActor*)0x0;
	this->field_0x3d8 = (CActor*)0x0;

	VibrationParam vibParams;
	vibParams.field_0x0 = this->field_0x360;
	vibParams.field_0x4 = this->field_0x354;
	vibParams.field_0x8 = this->field_0x358;
	vibParams.field_0xc = this->field_0x35c * 0.01745329f;
	vibParams.field_0x10 = this->field_0x3e0;
	vibParams.field_0x14 = 0.005f;
	vibParams.pActor = (CActor*)this;
	this->vibrationDyn.Init(&vibParams);

	this->ntfSwitch.Reset(this);

	CCollision* pCVar1;
	pCVar1 = this->pCollisionData;
	pCVar1->flags_0x0 = pCVar1->flags_0x0 | 0x1000;
	pCVar1 = this->pCollisionData;
	pCVar1->flags_0x0 = pCVar1->flags_0x0 | 2;
	pCVar1 = this->pCollisionData;
	pCVar1->flags_0x0 = pCVar1->flags_0x0 | 0x80000;

	return;
}

struct S_SAVE_CLASS_BOX
{
	uint field_0x0;
};

void CActorBox::SaveContext(void* pData, uint mode, uint maxSize)
{
	if (mode == 1) {
		S_SAVE_CLASS_BOX* pSaveData = reinterpret_cast<S_SAVE_CLASS_BOX*>(pData);
		pSaveData->field_0x0 = 0.0f < GetLifeInterface()->GetValue();
	}

	return;
}

void CActorBox::LoadContext(void* pData, uint mode, uint maxSize)
{
	if (mode == 1 && reinterpret_cast<S_SAVE_CLASS_BOX*>(pData)->field_0x0 == 0) {
		LifeAnnihilate();

		this->flags = this->flags & 0xffffff7f;
		this->flags = this->flags | 0x20;
		EvaluateDisplayState();

		this->flags = this->flags & 0xfffffffd;
		this->flags = this->flags | 1;

		SetBehaviour(BOX_BEHAVIOUR_STAND, BOX_STAND_STATE_DESTROYED, -1);
	}

	return;
}

CBehaviour* CActorBox::BuildBehaviour(int behaviourType)
{
	CBehaviour* pBehaviour;

	if (behaviourType == BOX_BEHAVIOUR_STAND) {
		pBehaviour = &this->behaviourBoxStand;
	}
	else {
		pBehaviour = CActorAutonomous::BuildBehaviour(behaviourType);
	}

	return pBehaviour;
}

StateConfig CActorBox::_gStateCfg_BOX[8] = {
	StateConfig(0x0, 0x4),
	StateConfig(0x0, 0x804),
	StateConfig(0x0, 0x4),
	StateConfig(0x0, 0x4),
	StateConfig(0x0, 0x804),
	StateConfig(0x0, 0x4),
	StateConfig(0xc, 0x821),
	StateConfig(0x0, 0x801),
};

StateConfig* CActorBox::GetStateCfg(int state)
{
	StateConfig* pStateConfig;

	if (state < 6) {
		pStateConfig = CActorAutonomous::GetStateCfg(state);
	}
	else {
		pStateConfig = &CActorBox::_gStateCfg_BOX[state - 6];
	}

	return pStateConfig;
}

bool CActorBox::CanPassThrough()
{
	return GetLifeInterface()->GetValue() <= 0.0f;
}

int CActorBox::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	undefined4* puVar1;
	CLifeInterface* pCVar2;
	edF32VECTOR4* peVar3;
	edF32VECTOR4* v2;
	int iVar4;
	undefined4 uVar5;
	float fVar6;
	undefined4 uVar7;
	undefined4 uVar8;
	_msg_hit_param* local_64;
	edF32VECTOR4 local_60;
	edF32VECTOR4 eStack80;
	edF32VECTOR4 local_40;
	edF32VECTOR4 eStack48;
	edF32VECTOR4 local_20;
	edF32VECTOR4 eStack16;

	local_64 = reinterpret_cast<_msg_hit_param*>(pMsgParam);
	if (msg == 3) {
		fVar6 = GetLifeInterface()->GetValue();
		if (fVar6 <= 0.0f) {
			iVar4 = 0;
		}
		else {
			SetState(0xc, -1);
			iVar4 = 1;
		}
	}
	else {
		if (msg == 0xd) {
			fVar6 = GetLifeInterface()->GetValue();
			if (fVar6 <= 0.0f) {
				iVar4 = 0;
			}
			else {
				if (local_64->field_0x4 == 0) {
					this->field_0x4a0 = *reinterpret_cast<edF32VECTOR4*>(pMsgParam);
					(this->field_0x4a0).y = (this->field_0x4a0).y - this->field_0x3e0;
					UpdatePosition(&this->field_0x4a0, true);
					SetState(7, -1);
				}
				else {
					SetState(10, -1);
				}
				iVar4 = 1;
			}
		}
		else {
			if (msg == MESSAGE_GET_VISUAL_DETECTION_POINT) {
				GetPositionMsgParams* pGetPosMsgParams = reinterpret_cast<GetPositionMsgParams*>(pMsgParam);
				edF32Vector4SubHard(&eStack16, GetBottomPosition(), &this->currentLocation);
				fVar6 = edF32Vector4DotProductHard(&eStack16, &this->pMeshTransform->base.transformA.rowY);
				edF32Vector4ScaleHard(fVar6, &eStack16, &this->pMeshTransform->base.transformA.rowY);
				edF32Vector4SubHard(&pGetPosMsgParams->vectorFieldB, GetBottomPosition(), GetTopPosition());
				fVar6 = edF32Vector4DotProductHard(&pGetPosMsgParams->vectorFieldB, &this->pMeshTransform->base.transformA.rowY);
				edF32Vector4ScaleHard(fVar6 * 0.5f, &pGetPosMsgParams->vectorFieldB, &this->pMeshTransform->base.transformA.rowY);
				edF32Vector4AddHard(&pGetPosMsgParams->vectorFieldB, &pGetPosMsgParams->vectorFieldB, &eStack16);
				iVar4 = 1;
			}
			else {
				if (msg == MESSAGE_KICKED) {
					fVar6 = GetLifeInterface()->GetValue();
					if (fVar6 <= 0.0f) {
						iVar4 = 0;
					}
					else {
						switch (local_64->projectileType) {
						case HIT_TYPE_BOOMY:
							if ((this->field_0x364 & 2) != 0) {
								ApplyHit(&local_64);
								return 1;
							}
							break;
						case 7:
							IMPLEMENTATION_GUARD(
							if (((this->field_0x364 & 0x40) != 0) && (this->actorState != 10)) {
								(*(this->pVTable)->LifeDecrease)
									(local_64->damage, (CActorAutonomous*)this);
								pCVar2 = (*(this->pVTable)->GetLifeInterface)((CActor*)this);
								fVar6 = (*pCVar2->pVtable->GetValue)((CInterface*)pCVar2);
								if (fVar6 <= 0.0) {
									peVar3 = &local_64->field_0x40;
									if ((this->field_0x364 & 1) != 0) {
										if ((this->flags & 0x1000) != 0) {
											SetVectorFromAngles(&this->rotationQuat,
												(edF32VECTOR3*)&this->rotationEuler);
										}
										edF32Vector4SubHard(&eStack80, &this->currentLocation, peVar3);
										fVar6 = edF32Vector4DotProductHard
										(&eStack80, &this->rotationQuat);
										if (fVar6 < 0.0f) {
											edF32Vector4ScaleHard
											((float)&DAT_bf800000, &local_60, &this->rotationQuat
											);
											this->rotationQuat.x = local_60.x;
											this->rotationQuat.y = local_60.y;
											this->rotationQuat.z = local_60.z;
											this->rotationQuat.w = local_60.w;
											if ((this->flags & 0x1000) != 0) {
												GetAnglesFromVector((edF32VECTOR3*)&this->rotationEuler,
													&this->rotationQuat);
											}
										}
									}
									(*(this->pVTable)->SetState)((CActor*)this, 0xc, -1);
									return 1;
								}
								iVar4 = FUN_00370c50((int)this, (edF32VECTOR4*)&this->field_0x4b0,
									&local_64->field_0x40, 0);
								if (iVar4 != 0) {
									CVibrationDyn::UpdateAllFactors
									(local_64->field_0x30, &this->vibrationDyn,
										(edF32VECTOR4*)&this->field_0x4b0, &local_64->field_0x40);
									(*(this->pVTable)->SetState)((CActor*)this, 8, -1);
									return 1;
								}
							}
							break;)
						case 8:
							if (((this->field_0x364 & 4) != 0) && (this->actorState != 10)) {
								peVar3 = &local_64->field_0x40;
								if ((this->field_0x364 & 1) != 0) {
									if ((this->flags & 0x1000) != 0) {
										SetVectorFromAngles(&this->rotationQuat, &this->rotationEuler.xyz);
									}

									edF32Vector4SubHard(&eStack48, &this->currentLocation, peVar3);
									fVar6 = edF32Vector4DotProductHard(&eStack48, &this->rotationQuat);
									if (fVar6 < 0.0f) {
										edF32Vector4ScaleHard(-1.0f, &local_40, &this->rotationQuat);
										this->rotationQuat = local_40;

										if ((this->flags & 0x1000) != 0) {
											GetAnglesFromVector(&this->rotationEuler.xyz, &this->rotationQuat);
										}
									}
								}

								SetState(0xc, -1);

								return 1;
							}
							break;
						case 9:
							if ((this->field_0x364 & 8) != 0) {
								ApplyHit(&local_64);
								return 1;
							}
							break;
						case 10:
							if (((this->field_0x364 & 0x10) != 0) && (0.0f < local_64->damage)) {
								peVar3 = &local_64->field_0x20;
								if ((this->field_0x364 & 1) != 0) {
									if ((this->flags & 0x1000) != 0) {
										SetVectorFromAngles(&this->rotationQuat, &this->rotationEuler.xyz);
									}

									fVar6 = edF32Vector4DotProductHard(peVar3, &this->rotationQuat);
									if (fVar6 < 0.0f) {
										edF32Vector4ScaleHard(-1.0f, &local_20, &this->rotationQuat);
										this->rotationQuat = local_20;
										if ((this->flags & 0x1000) != 0) {
											GetAnglesFromVector(&this->rotationEuler.xyz, &this->rotationQuat);
										}
									}
								}

								SetState(0xc, -1);
								return 1;
							}
							break;
						case 0xb:
							if ((this->field_0x364 & 0x20) != 0) {
								fVar6 = GetLifeInterface()->GetValueMax();
								local_64->damage = fVar6;
								ApplyHit(&local_64);
								return 1;
							}
						}

						iVar4 = 0;
					}
				}
				else {
					iVar4 = CActorAutonomous::InterpretMessage(pSender, msg, pMsgParam);
				}
			}
		}
	}

	return iVar4;
}

void CActorBox::BehaviourBoxStand_Manage()
{
	int iVar1;
	CActor* pCVar2;
	bool bVar5;
	StateConfig* pSVar6;
	uint uVar7;
	edF32VECTOR3 local_10;

	this->ntfSwitch.pStreamEventCamera->Manage(this);

	if (((this->pCollisionData)->flags_0x4 & 2) == 0) {

		if ((GetStateFlags(this->actorState) & 0x800) == 0) {
			SetState(10, -1);
		}
	}

	switch (this->actorState) {
	case 6:
		pCVar2 = this->pTiedActor;
		if (pCVar2 != this->field_0x3d4) {
			this->field_0x3d4 = pCVar2;
			SetState(10, -1);
		}
		break;
	case 7:
		this->dynamic.speed = 0.0f;
		this->dynamic.linearAcceleration = 0.0f;
		break;
	case 8:
		bVar5 = this->vibrationDyn.MakeVibrate(&local_10);
		this->rotationEuler.x = local_10.x;
		this->rotationEuler.y = local_10.y;
		this->rotationEuler.z = local_10.z;

		ManageDyn(4.0f, 0x1002023b, (CActorsTable*)0x0);

		if (bVar5 != false) {
			SetState(6, -1);
		}
		break;
	case 10:
		FUN_0036fb80();
		break;
	case 0xb:
		IMPLEMENTATION_GUARD(
		StateBoxFallToShake();)
		break;
	case BOX_STAND_STATE_EXPLODE:
		if (((this->field_0x364 & 0x80) == 0) && (0.2f < this->timeInAir)) {
			if (this->pAnimationController != (CAnimation*)0x0) {
				if (!this->pAnimationController->IsCurrentLayerAnimEndReached(0)) {
					return;
				}
			}

			SetState(BOX_STAND_STATE_DESTROYED, -1);
		}
	}

	return;
}

void CActorBox::StateBoxExplosionInit(CBehaviourBoxStand* pBehaviour)
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

	if (this->pTiedActor != (CActor*)0x0) {
		TieToActor((CActor*)0x0, 0, 1, (edF32MATRIX4*)0x0);
	}

	CCollision* pCVar1 = this->pCollisionData;
	S_TIED_ACTOR_ENTRY* pSVar2 = pCVar1->pTiedActorEntry;
	while (pSVar2 != (S_TIED_ACTOR_ENTRY*)0x0) {
		pSVar2->pActor->TieToActor((CActor*)0x0, 0, 1, (edF32MATRIX4*)0x0);
		pSVar2 = pCVar1->pTiedActorEntry;
	}

	iVar9 = 0;
	while (true) {
		pSVar1 = this->field_0x380;
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
			pActor->UpdatePosition(&this->currentLocation, true);
		}

		iVar9 = iVar9 + 1;
	}

	local_10.z = this->currentLocation.z;
	local_10.w = this->currentLocation.w;
	local_10.x = this->currentLocation.x;
	local_10.y = this->currentLocation.y + this->field_0x3e0 / 2.0f;

	this->addOnGenerator.Generate(&local_10);

	if (this->field_0x37c != -1) {
		pKVar2 = this->subObjA;
		local_20 = pKVar2->boundingSphere;
		if (local_20.w < 20.0f) {
			local_20.w = 20.0f;
		}

		SV_SwitchToModel(&this->actorAlternateModel, this->field_0x37c, (this->pCinData)->textureIndex, &local_20);
	}

	pCVar3 = this->pCollisionData;
	pCVar3->flags_0x0 = pCVar3->flags_0x0 & 0xffffefff;
	pCVar3 = this->pCollisionData;
	pCVar3->flags_0x0 = pCVar3->flags_0x0 & 0xfffffffd;
	pCVar3 = this->pCollisionData;
	pCVar3->flags_0x0 = pCVar3->flags_0x0 & 0xfff7ffff;

	CScene::ptable.g_EffectsManager_004516b8->GetDynamicFx(&this->field_0x3cc, this->field_0x350, FX_MATERIAL_SELECTOR_NONE);
	pCVar4 = (this->field_0x3cc).pFx;
	if (((pCVar4 != (CNewFx*)0x0) && (iVar10 = (this->field_0x3cc).id, iVar10 != 0)) && (iVar10 == pCVar4->id)) {
		pCVar4->SpatializeOnActor(2, this, 0);
	}
	pCVar4 = (this->field_0x3cc).pFx;
	if (((pCVar4 != (CNewFx*)0x0) && (iVar10 = (this->field_0x3cc).id, iVar10 != 0)) && (iVar10 == pCVar4->id)) {
		pCVar4->Start(0, 0);
	}

	ntfSwitch.Switch(this);
}

void CActorBox::ApplyHit(_msg_hit_param** ppHitParam)
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
	if ((this->field_0x364 & 2) != 0) {
		LifeDecrease((*ppHitParam)->damage);

		if ((GetLifeInterface()->GetValueMax() * 2.0f) / 3.0f <= GetLifeInterface()->GetValue()) {
			if (this->field_0x370 != -1) {
				peVar3 = pFileManager->GetActorsCommonMaterial(this->field_0x370);
				SV_PatchG2D(peVar3);
			}
		}
		else {
			if (GetLifeInterface()->GetValueMax() / 3.0f <= GetLifeInterface()->GetValue()) {
				if (this->field_0x374 != -1) {
					peVar3 = pFileManager->GetActorsCommonMaterial(this->field_0x374);
					SV_PatchG2D(peVar3);
				}
			}
			else {
				if (GetLifeInterface()->GetValue() <= 0.0f) {
					(*ppHitParam)->field_0x74 = 1;
					p_Var1 = *ppHitParam;

					if ((this->field_0x364 & 1) != 0) {
						if ((this->flags & 0x1000) != 0) {
							SetVectorFromAngles(&this->rotationQuat, &this->rotationEuler.xyz);
						}

						fVar5 = edF32Vector4DotProductHard(&p_Var1->field_0x20, &this->rotationQuat);
						if (fVar5 < 0.0f) {
							edF32Vector4ScaleHard(-1.0f, &local_20, &this->rotationQuat);
							this->rotationQuat = local_20;
							if ((this->flags & 0x1000) != 0) {
								GetAnglesFromVector(&this->rotationEuler.xyz, &this->rotationQuat);
							}
						}
					}

					SetState(BOX_STAND_STATE_EXPLODE, -1);
					return;
				}
			}
		}

		local_10.z = this->currentLocation.z;
		local_10.w = this->currentLocation.w;
		local_10.x = this->currentLocation.x;
		local_10.y = this->currentLocation.y + this->field_0x3e0;
		this->vibrationDyn.UpdateAllFactors((*ppHitParam)->damage, &(*ppHitParam)->field_0x20, &local_10);
		SetState(8, -1);
	}

	return;
}

void CActorBox::FUN_0036fb80()
{
	S_TIED_ACTOR_ENTRY* pSVar1;
	float fVar2;
	edF32VECTOR4 local_10;

	fVar2 = (this->field_0x490).y - this->currentLocation.y;

	if (((this->pCollisionData)->flags_0x4 & 2) == 0) {
		if (2.0f < fVar2) {
			local_10.z = this->rotationQuat.z;
			local_10.w = this->rotationQuat.w;
			local_10.x = this->rotationQuat.x;
			local_10.y = 0.2f;
			edF32Vector4NormalizeHard(&local_10, &local_10);
			SV_UpdateOrientation(6.283185f, &local_10);
		}

		ManageDyn(4.0f, 0x1002023b, (CActorsTable*)0x0);
	}
	else {
		ManageDyn(4.0f, 0x1002023b, (CActorsTable*)0x0);

		if (10.0f < fVar2) {
			SetState(BOX_STAND_STATE_EXPLODE, -1);
		}
		else {
			pSVar1 = (this->pCollisionData)->pTiedActorEntry;
			this->field_0x3d4 = this->pTiedActor;

			if ((fVar2 <= 2.0f) || (pSVar1 == (S_TIED_ACTOR_ENTRY*)0x0)) {
				SetState(6, -1);
			}
			else {
				SetState(0xb, -1);
			}
		}
	}

	return;
}



void CBehaviourBoxStand::Create(ByteCode* pByteCode)
{
	return;
}

void CBehaviourBoxStand::Manage()
{
	this->pOwner->BehaviourBoxStand_Manage();

	return;
}

void CBehaviourBoxStand::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	CBehaviourBox::Begin(pOwner, newState, newAnimationType);

	if (newState == -1) {
		this->pOwner->SetState(10, -1);
	}
	else {
		this->pOwner->SetState(newState, newAnimationType);
	}

	return;
}

void CBehaviourBoxStand::InitState(int newState)
{
	CActor* this_00;
	int* piVar1;
	CCollision* pCVar2;
	int iVar3;
	int iVar4;
	int iVar5;
	float fVar6;
	float fVar7;
	float fVar8;
	CActorBox* pBox;

	pBox = this->pOwner;

	if (newState == 0xd) {
		pBox->flags = pBox->flags & 0xffffff7f;
		pBox->flags = pBox->flags | 0x20;

		pBox->EvaluateDisplayState();

		pBox->flags = pBox->flags & 0xfffffffd;
		pBox->flags = pBox->flags | 1;
	}
	else {
		if (newState == BOX_STAND_STATE_EXPLODE) {
			pBox->StateBoxExplosionInit(this);
		}
		else {
			if (newState == 10) {
				pBox->field_0x490 = pBox->currentLocation;

				pCVar2 = pBox->pCollisionData;
				pCVar2->flags_0x0 = pCVar2->flags_0x0 | 0x1000;
				pCVar2 = pBox->pCollisionData;
				pCVar2->flags_0x0 = pCVar2->flags_0x0 | 2;
				pCVar2 = pBox->pCollisionData;
				pCVar2->flags_0x0 = pCVar2->flags_0x0 | 0x80000;
			}
			else {
				if (newState == 8) {
					pBox->field_0x3dc = pBox->currentLocation.y;
					SetVectorFromAngles(&pBox->rotationQuat, &pBox->rotationEuler.xyz);
				}
				else {
					if (newState == 6) {
						iVar5 = 0;
						while (true) {
							S_ACTOR_STREAM_REF* pStream = pBox->field_0x380;

							iVar3 = 0;
							if (pStream != (S_ACTOR_STREAM_REF*)0x0) {
								iVar3 = pStream->entryCount;
							}

							if (iVar3 <= iVar5) break;

							this_00 = pStream->aEntries[iVar5].Get();
							if (this_00 != (CActor*)0x0) {
								this_00->flags = this_00->flags & 0xffffff7f;
								this_00->flags = this_00->flags | 0x20;
								this_00->EvaluateDisplayState();
								this_00->flags = this_00->flags & 0xfffffffd;
								this_00->flags = this_00->flags | 1;
							}

							iVar5 = iVar5 + 1;
						}

						pBox->dynamicExt.normalizedTranslation.x = 0.0f;
						pBox->dynamicExt.normalizedTranslation.y = 0.0f;
						pBox->dynamicExt.normalizedTranslation.z = 0.0f;
						pBox->dynamicExt.normalizedTranslation.w = 0.0f;
						pBox->dynamicExt.field_0x6c = 0.0f;
						pBox->dynamic.speed = 0.0f;
					}
				}
			}
		}
	}

	return;
}

void CBehaviourBoxStand::TermState(int oldState, int newState)
{
	CActorBox* pBox;

	pBox = this->pOwner;

	if (((oldState == 0xc) && (pBox->field_0x37c != -1)) && (pBox->field_0x378 != -1)) {
		pBox->SV_RestoreOrgModel(&pBox->actorAlternateModel);
	}

	return;
}

int CBehaviourBoxStand::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	return 0;
}

void CBehaviourBox::Create(ByteCode* pByteCode)
{
	return;
}

void CBehaviourBox::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	this->pOwner = static_cast<CActorBox*>(pOwner);

	return;
}

int CBehaviourBox::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	return 0;
}

int CBehaviourBox::InterpretEvent(edCEventMessage* pEventMessage, undefined8 param_3, int param_4, uint* param_5)
{
	return 0;
}
