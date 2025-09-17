#include "ActorCompanion.h"
#include "MemoryStream.h"
#include "LevelScheduleManager.h"
#include "CinematicManager.h"
#include "camera.h"
#include "TimeController.h"
#include "ActorHero.h"
#include "LocalizationManager.h"
#include "CameraViewManager.h"
#include "MathOps.h"
#include "InputManager.h"
#include "EventManager.h"
#include "WayPoint.h"
#include "Frontend.h"
#include "FrontEndDisp.h"
#include "Rendering/DisplayList.h"
#include "Pause.h"
#include "kya.h"
#include "FrontEndInventory.h"
#include "CameraGame.h"
#include "edStr.h"

CActorCompanion* CActorCompanion::_gThis = NULL;

void CActorCompanion::Create(ByteCode* pByteCode)
{
	CActorCompanion::_gThis = this;

	CActorAutonomous::Create(pByteCode);

	if (this->pCollisionData != (CCollision*)0x0) {
		this->pCollisionData->flags_0x0 = this->pCollisionData->flags_0x0 | 0x2000;
		this->pCollisionData->flags_0x0 = this->pCollisionData->flags_0x0 & 0xffffefff;
	}

	this->field_0x358 = pByteCode->GetS32();
	this->materialId_0x35c = pByteCode->GetS32();
	this->field_0x360 = pByteCode->GetS32();

	CActor::SV_InstallMaterialId(this->field_0x358);
	CActor::SV_InstallMaterialId(this->materialId_0x35c);
	CActor::SV_InstallMaterialId(this->field_0x360);

	return;
}

void CActorCompanion::Init()
{
	CActorAutonomous::Init();

	this->pAnimationController->RegisterBone(0x5579b496);

	this->field_0x354.rgba = 0x4010a010;
	this->field_0x36c = 0;

	return;
}

void CActorCompanion::Term()
{
	this->pAnimationController->UnRegisterBone(0x5579b496);

	CActorAutonomous::Term();

	_gThis = (CActorCompanion*)0x0;

	return;
}

void CActorCompanion::Draw()
{
	CShadow* pCVar1;
	CCameraManager* pCVar2;
	bool bVar3;
	CompanionAlert* pCVar4;
	edF32MATRIX4* peVar5;
	edDList_material* peVar6;
	int iVar7;
	int iVar8;
	uint uVar9;
	uint uVar10;
	uint uVar11;
	float fVar12;
	float x;
	edF32MATRIX4 local_110;
	edF32MATRIX4 local_d0;
	edF32VECTOR4 eStack144;
	edF32VECTOR4 eStack128;
	edF32VECTOR4 local_70;
	edF32MATRIX4 eStack96;
	edF32VECTOR2 eStack32;
	edF32VECTOR2 local_18;
	edF32VECTOR2 eStack16;
	float fStack8;
	float fStack4;
	C3DFileManager* pFileManager;

	pFileManager = CScene::ptable.g_C3DFileManager_00451664;
	pCVar2 = CCameraManager::_gThis;

	CActorAutonomous::Draw();

	iVar7 = this->actorState;
	if ((iVar7 == 0xe) || (iVar7 == 0xd)) {
		pCVar1 = this->pShadow;
		if (pCVar1 != (CShadow*)0x0) {
			IMPLEMENTATION_GUARD(
			(**(code**)((pCVar1->base).pVTable + 0x14))(pCVar1, 0);)
		}
	}
	else {
		if ((GameFlags & 0x20) == 0) {
			pCVar1 = this->pShadow;
			if (pCVar1 != (CShadow*)0x0) {
				IMPLEMENTATION_GUARD(
				(**(code**)((pCVar1->base).pVTable + 0x14))(pCVar1, 1);)
			}
			iVar7 = this->actorState;
			if ((7 < iVar7) && (iVar7 < 0xb)) {
				if (this->behaviourCompanion.GetActiveAlert() != (CompanionAlert*)0x0) {
					FUN_001e82a0();
				}
			}

			bVar3 = GameDList_BeginCurrent();
			if (bVar3 != false) {
				peVar5 = this->pAnimationController->GetCurBoneMatrix(0x5579b496);
				local_70 = peVar5->rowT;
				peVar5 = this->pAnimationController->GetCurBoneMatrix(0x5579b496);
				edF32Matrix4MulF32Vector4Hard(&eStack144, peVar5, &gF32Vector4Zero);
				edF32Vector4AddHard(&local_70, &local_70, &eStack144);
				edF32Matrix4MulF32Vector4Hard(&local_70, &this->pMeshTransform->base.transformA, &local_70);
				edF32Vector4SubHard(&eStack128, &(pCVar2->transformationMatrix).rowT, &local_70);
				edF32Vector4NormalizeHard(&eStack128, &eStack128);
				edF32Vector4ScaleHard(0.1f, &eStack128, &eStack128);
				edF32Vector4AddHard(&local_70, &local_70, &eStack128);
				local_70.w = 1.0f;
				edF32Matrix4CopyHard(&eStack96, &pCVar2->transMatrix_0x390);
				eStack96.rowT = local_70;
				edDListLoadMatrix(&eStack96);
				peVar6 = pFileManager->GetMaterialFromId(this->field_0x358, 0);
				edDListUseMaterial(peVar6);
				edDListBegin(0.0f, 0.0f, 0.0f, 8, 4);

				iVar7 = rand();
				uVar11 = (uint)(this->field_0x354).a;
				edDListColor4u8((this->field_0x354).r, (this->field_0x354).g, (this->field_0x354).b,
					(byte)(int)((float)uVar11 * 0.5f + (float)uVar11 * ((float)iVar7 / 2.147484e+09f) * 0.4f));

				edDListTexCoo2f(0.0f, 0.0f);
				edDListVertex4f(-0.1f, 0.1f, 0.0f, 0.0f);
				edDListTexCoo2f(1.0f, 0.0f);
				edDListVertex4f(0.1f, 0.1f, 0.0f, 0.0f);
				edDListTexCoo2f(0.0f, 1.0f);
				edDListVertex4f(-0.1f, -0.1f, 0.0f, 0.0f);
				edDListTexCoo2f(1.0f, 1.0f);
				edDListVertex4f(0.1f, -0.1f, 0.0f, 0.0f);
				edDListEnd();

				if (this->actorState == 10) {
					if (this->behaviourCompanion.GetActiveAlert() != (CompanionAlert*)0x0) {
						if (this->behaviourCompanion.GetActiveAlert()->field_0x44 != 0) {
							eStack96.rowT.x = this->currentLocation.x;
							eStack96.rowT.y = this->currentLocation.z;
							eStack96.rowT.z = this->currentLocation.w;
							eStack96.rowT.w = this->currentLocation.y + -0.4f;

							x = (float)-this->behaviourCompanion.GetActiveAlert()->field_0x44 * 0.5f;
							if (this->field_0x368 != 0) {
								edF32Vector4ScaleHard(-1.0f, &eStack96.rowX, &eStack96.rowX);
							}

							edDListLoadMatrix(&eStack96);
							uVar11 = 0;
							while (true) {
								iVar8 = (this->behaviourCompanion).activeSubObjId;
								uVar9 = 0x44; // offset from NULL?
								if (-1 < iVar8) {
									uVar9 = (this->behaviourCompanion).aSubObjs[iVar8].field_0x44;
								}

								if (uVar9 <= uVar11) break;

								peVar6 = pFileManager->GetMaterialFromId(this->materialId_0x35c, this->behaviourCompanion.GetActiveAlert()->field_0x34[uVar11]);
								edDListUseMaterial(peVar6);
								edDListBegin(0.0f, 0.0f, 0.0f, 8, 4);
								edDListColor4u8(0x80, 0x80, 0x80, 0x80);

								uVar9 = 8;
								if (this->behaviourCompanion.GetActiveAlert()->field_0x34[uVar11] == 2) {
									CFrontendDisplay* pFrontendDisplay = (CFrontendDisplay*)CScene::GetManager(MO_Frontend);
									CFrontendInventory* pInventory = pFrontendDisplay->pInventory;
									fVar12 = 0.0f;
									uVar9 = pInventory->ComputeGameScreenCoordinate(&eStack16, &this->currentLocation, &fStack4, 0);
									uVar10 = pInventory->ComputeGameScreenCoordinate(&eStack32, &(this->behaviourCompanion).field_0x40, &fStack8, 0);
									if ((uVar9 & uVar10) != 0) {
										edF32Vector2Sub(&local_18, &eStack32, &eStack16);
										fVar12 = atan2f(local_18.x, local_18.y);
									}

									if (this->field_0x368 == 0) {
										fVar12 = edF32Between_2Pi(fVar12 - 1.570796f);
									}
									else {
										fVar12 = edF32Between_2Pi(-fVar12 - 1.570796f);
									}

									local_110.aa = -1.0f;
									local_110.ab = -1.0f;
									local_110.ac = 0.0f;
									local_110.ad = 1.0f;
									local_110.ba = 0.5f;
									local_110.bb = -1.0f;
									local_110.bc = 0.0f;
									local_110.bd = 1.0f;
									local_110.ca = -1.0f;
									local_110.cb = 0.5f;
									local_110.cc = 0.0f;
									local_110.cd = 1.0f;
									local_110.da = 0.5f;
									local_110.db = 0.5f;
									local_110.dc = 0.0f;
									local_110.dd = 1.0f;

									edF32Matrix4RotateZHard(fVar12, &local_d0, &gF32Matrix4Unit);
									peVar5 = &local_d0;
									local_d0.da = 0.5f;
									local_d0.db = 0.5f;

									edF32Matrix4MulF32Matrix4Hard(peVar5, &local_110, peVar5);
									edDListTexCoo2f(0.0f, 0.0f);
									edDListVertex4f(x + local_d0.aa, local_d0.ab, 0.0f, 0.0f);
									edDListTexCoo2f(1.0f, 0.0f);
									edDListVertex4f(x + local_d0.ba, local_d0.bb, 0.0f, 0.0f);
									edDListTexCoo2f(0.0f, 1.0f);
									edDListVertex4f(x + local_d0.ca, local_d0.cb, 0.0f, 0.0f);
									edDListTexCoo2f(1.0f, 1.0f);
									edDListVertex4f(x + local_d0.da, local_d0.db, 0.0f, 0.0f);
								}
								else {
									edDListTexCoo2f(0.0f, 0.0f);
									edDListVertex4f(x, 1.0f, 0.0f, 0.0f);
									edDListTexCoo2f(1.0f, 0.0f);
									fVar12 = x + 1.0f;
									edDListVertex4f(fVar12, 1.0f, 0.0f, 0.0f);
									edDListTexCoo2f(0.0f, 1.0f);
									edDListVertex4f(x, 0.0f, 0.0f, 0.0f);
									edDListTexCoo2f(1.0f, 1.0f);
									edDListVertex4f(fVar12, 0.0f, 0.0f, 0.0f);
								}

								edDListEnd();
								uVar11 = uVar11 + 1;
								x = x + 1.0f;
							}
						}
					}
				}

				GameDList_EndCurrent();
			}
		}
	}

	return;
}

void CActorCompanion::Reset()
{
	int bHasCompanion;
	int iVar4;

	CActorAutonomous::Reset();

	SetState(0xe, -1);

	bHasCompanion = CLevelScheduler::ScenVar_Get(SCN_GAME_COMPANION);

	iVar4 = 2;
	if (bHasCompanion == 0) {
		iVar4 = 0;
	}

	if (iVar4 == 2) {
		(this->behaviourCompanion).pOwner->flags = (this->behaviourCompanion).pOwner->flags | 2;
		(this->behaviourCompanion).pOwner->flags = (this->behaviourCompanion).pOwner->flags & 0xfffffffe;
	}
	else {
		if (iVar4 == 1) {
			(this->behaviourCompanion).pOwner->flags = (this->behaviourCompanion).pOwner->flags & 0xfffffffc;
		}
		else {
			if (iVar4 == 0) {
				(this->behaviourCompanion).pOwner->flags = (this->behaviourCompanion).pOwner->flags & 0xfffffffd;
				(this->behaviourCompanion).pOwner->flags = (this->behaviourCompanion).pOwner->flags | 1;
			}
		}
	}

	(this->behaviourCompanion).pOwner->flags = (this->behaviourCompanion).pOwner->flags & 0xffffff7f;
	(this->behaviourCompanion).pOwner->flags = (this->behaviourCompanion).pOwner->flags | 0x20;

	(this->behaviourCompanion).pOwner->EvaluateDisplayState();

	if (bHasCompanion == 0) {
		(this->behaviourCompanion).pOwner->pCollisionData->flags_0x0 = (this->behaviourCompanion).pOwner->pCollisionData->flags_0x0 & 0xfff7efff;
		(this->behaviourCompanion).pOwner->flags = (this->behaviourCompanion).pOwner->flags & 0xfffffbff;
	}
	else {
		(this->behaviourCompanion).pOwner->pCollisionData->flags_0x0 = (this->behaviourCompanion).pOwner->pCollisionData->flags_0x0 | 0x81000;
		(this->behaviourCompanion).pOwner->flags = (this->behaviourCompanion).pOwner->flags | 0x400;
	}

	this->behaviourCompanion.field_0x68 = 0;
	this->field_0x354.rgba = 0x4010a010;

	return;
}

struct S_SAVE_CLASS_COMPANION
{
	uint bits[8]; // supports up to 256 companion instances
};

void CActorCompanion::SaveContext(void* pData, uint mode, uint maxSize)
{
	S_SAVE_CLASS_COMPANION* pSaveData = reinterpret_cast<S_SAVE_CLASS_COMPANION*>(pData);

	// Clear out all bitfields
	std::memset(pSaveData->bits, 0, sizeof(pSaveData->bits));

	CBehaviourCompanion* pBehaviour = static_cast<CBehaviourCompanion*>(GetBehaviour(this->curBehaviourId));

	for (uint32_t i = 0; i < pBehaviour->nbSubObjs; ++i)
	{
		const CompanionAlert& inst = pBehaviour->aSubObjs[i];

		if (inst.flags_0x4 & 4) // active?
		{
			uint32_t wordIndex = i / 32;
			uint32_t bitIndex = i % 32;

			pSaveData->bits[wordIndex] |= (1u << bitIndex);
		}
	}

	return;
}

void CActorCompanion::LoadContext(uint*, int)
{
	IMPLEMENTATION_GUARD();
}

CBehaviour* CActorCompanion::BuildBehaviour(int behaviourType)
{
	CBehaviour* pBehaviour;

	if (behaviourType == COMPANION_BEHAVIOUR_DEFAULT) {
		pBehaviour = &this->behaviourCompanion;
	}
	else {
		pBehaviour = CActor::BuildBehaviour(behaviourType);
	}

	return pBehaviour;
}

StateConfig CActorCompanion::_gStateCfg_CMP[9] =
{
	StateConfig(0xc, 0x0),
	StateConfig(0xe, 0x0),
	StateConfig(0xe, 0x0),
	StateConfig(0xf, 0x2),
	StateConfig(0xf, 0x2),
	StateConfig(0xe, 0x0),
	StateConfig(0xe, 0x2),
	StateConfig(-1, 0x0),
	StateConfig(-1, 0x0),
};

StateConfig* CActorCompanion::GetStateCfg(int state)
{
	StateConfig* pStateConfig;

	if (state < 6) {
		pStateConfig = CActorAutonomous::GetStateCfg(state);
	}
	else {
		assert((state + -6) < 9);
		pStateConfig = _gStateCfg_CMP + state + -6;
	}

	return pStateConfig;
}

float CMP_DIST_LIMIT = 0.05f;
float CMP_RUN_SPEED = 6.0f;
float CMP_RUN_ACCELERATION = 4.0f;
float CMP_WALK_DECELERATION = 10.0f;
float CMP_STAND_TRIG_LOW = 2.0f;

void CActorCompanion::BehaviourCompanion_Manage()
{
	int iVar1;
	CCollision* pCVar2;
	CActorHero* pHero;
	float fVar4;
	float fVar5;
	bool bVar6;
	CompanionAlert* pCVar7;
	float fVar8;
	edF32VECTOR4 local_40;
	edF32VECTOR4 eStack48;
	edF32VECTOR4 local_20;
	edF32VECTOR4 local_10;
	CActorCompanion* pActor;

	static edF32VECTOR4 g_vectorHalf = { 0.5f, 0.5f, 0.5f, 1.0f };
	local_10 = g_vectorHalf;
	SetScaleVector(&local_10);

	this->field_0x354.rgba = 0x4010a010;

	switch (this->actorState) {
	case 6:
		IMPLEMENTATION_GUARD(
		StateStand();)
		break;
	case 7:
		pHero = this->pActorHero;
		local_20.x = pHero->currentLocation.x;
		local_20.z = pHero->currentLocation.z;
		local_20.w = pHero->currentLocation.w;
		local_20.y = pHero->currentLocation.y + 2.5f;
		fVar8 = SV_CMP_MoveToSpecial(CMP_RUN_SPEED, CMP_RUN_ACCELERATION, CMP_WALK_DECELERATION, 8.0f, this->pActorHero, &local_20, 0x14);
		if (fVar8 <= CMP_DIST_LIMIT) {
			SetState(6, -1);
		}
		break;
	case 8:
		if (this->behaviourCompanion.GetActiveAlert() == (CompanionAlert*)0x0) {
			SetState(7, -1);
		}
		else {
			this->behaviourCompanion.FUN_001e9660(&eStack48);

			if (SV_CMP_MoveToSpecial(40.0f, 80.0f, 80.0f, 0.0f, (CActor*)0x0, &eStack48, 0x14) <= CMP_DIST_LIMIT) {
				this->dynamic.speed = 0.0f;
				SetState(COMPANION_DEFAULT_STATE_TIPS_ALERT, -1);
			}
			else {
				pHero = this->pActorHero;
				fVar8 = (this->behaviourCompanion).field_0x30.x - pHero->currentLocation.x;
				fVar4 = (this->behaviourCompanion).field_0x30.y - pHero->currentLocation.y;
				fVar5 = (this->behaviourCompanion).field_0x30.z - pHero->currentLocation.z;

				if (this->behaviourCompanion.GetActiveAlert()->field_0x50 < fVar8 * fVar8 + fVar4 * fVar4 + fVar5 * fVar5) {
					(this->behaviourCompanion).field_0x54 = 1;
					SetState(7, -1);
				}
			}
		}
		break;
	case COMPANION_DEFAULT_STATE_TIPS_ALERT:
		StateTipsAlert();
		break;
	case COMPANION_DEFAULT_STATE_TIPS_SHOW:
		StateTipsShow();
		break;
	case 0xb:
		pHero = this->pActorHero;
		local_40.x = pHero->currentLocation.x;
		local_40.z = pHero->currentLocation.z;
		local_40.w = pHero->currentLocation.w;
		local_40.y = pHero->currentLocation.y + 15.0f;

		fVar8 = SV_CMP_MoveToSpecial(3.0f, CMP_RUN_ACCELERATION, CMP_WALK_DECELERATION, CMP_STAND_TRIG_LOW, this->pActorHero, &local_40, 0x14);

		if (fVar8 <= CMP_DIST_LIMIT) {
			SetState(0xe, -1);
			pActor = (this->behaviourCompanion).pOwner;
			pActor->flags = pActor->flags | 2;
			pActor->flags = pActor->flags & 0xfffffffe;
			pActor = (this->behaviourCompanion).pOwner;
			pActor->flags = pActor->flags & 0xffffff7f;
			pActor->flags = pActor->flags | 0x20;
			pActor->EvaluateDisplayState();
			pCVar2 = this->behaviourCompanion.pOwner->pCollisionData;
			pCVar2->flags_0x0 = pCVar2->flags_0x0 | 0x81000;
			pActor = (this->behaviourCompanion).pOwner;
			pActor->flags = pActor->flags | 0x400;
			(this->behaviourCompanion).field_0x68 = 0;
		}
	}

	pHero = this->pActorHero;
	bVar6 = pHero->IsFightRelated(pHero->curBehaviourId);
	if ((bVar6 != false) && (this->actorState != 0xb)) {
		SetState(0xb, -1);
	}

	return;
}

void CActorCompanion::StateTipsAlert()
{
	int iVar1;
	CompanionAlert* pSubObj;
	CActorMovParamsIn movParamsIn;
	CActorMovParamsOut movParamsOut;
	CCameraManager* pCameraManager;

	movParamsOut.flags = 0;
	movParamsIn.pRotation = (edF32VECTOR4*)0x0;
	movParamsIn.rotSpeed = 12.56637f;
	movParamsIn.speed = 0.0f;
	movParamsIn.acceleration = 10.0f;
	movParamsIn.flags = 0x416;
	SV_MOV_MoveTo(&movParamsOut, &movParamsIn, &(CCameraManager::_gThis->transformationMatrix).rowT);

	ManageDyn(4.0f, 0, (CActorsTable*)0x0);

	pCameraManager = CCameraManager::_gThis;

	if (this->behaviourCompanion.GetActiveAlert() == (CompanionAlert*)0x0) {
		SetState(0xb, -1);
	}
	else {
		pSubObj = (this->behaviourCompanion).aSubObjs + (this->behaviourCompanion).activeSubObjId;
		if ((((pSubObj->flags_0x2 & 0x100) != 0) && ((pSubObj->flags_0x0 & 2) != 0)) && ((pSubObj->flags_0x4 & 1) == 0)) {
			this->behaviourCompanion.FUN_001e56b0(1);
		}

		if (((this->behaviourCompanion).aSubObjs[(this->behaviourCompanion).activeSubObjId].flags_0x2 & 4) != 0) {
			pCameraManager->PushCamera((this->behaviourCompanion.GetActiveAlert()->pCamera).Get(), 0);
		}

		SetState(COMPANION_DEFAULT_STATE_TIPS_SHOW, -1);
	}

	return;
}

void CActorCompanion::StateTipsShow()
{
	int iVar1;
	float fVar2;
	float fVar3;
	float fVar4;
	CompanionAlert* pSubObj;
	CActor* pTieActor;
	CActorMovParamsIn movParamsIn;
	CActorMovParamsOut movParamsOut;
	CActorHero* pHero;
	CActor* pSubObjActor;

	movParamsOut.flags = 0;
	movParamsIn.pRotation = (edF32VECTOR4*)0x0;
	movParamsIn.acceleration = 8.0f;
	movParamsIn.rotSpeed = 12.56637f;
	movParamsIn.speed = 0.0f;
	movParamsIn.flags = 0x402;
	SV_MOV_MoveTo(&movParamsOut, &movParamsIn, &(CCameraManager::_gThis->transformationMatrix).rowT);

	if (this->behaviourCompanion.GetActiveAlert() != (CompanionAlert*)0x0) {
		if ((this->behaviourCompanion.GetActiveAlert()->flags_0x2 & 0x40) != 0) {
			pSubObjActor = (this->behaviourCompanion.GetActiveAlert()->pActor).Get();
			pTieActor = pSubObjActor->pTiedActor;
			if (pTieActor == (CActor*)0x0) {
				pTieActor = pSubObjActor->GetCollidingActor();
			}

			if ((pTieActor != (CActor*)0x0) && (this->pTiedActor == (CActor*)0x0)) {
				TieToActor(pTieActor, 0, 1, (edF32MATRIX4*)0x0);
			}
		}
	}

	ManageDyn(4.0f, 0x100, (CActorsTable*)0x0);

	this->field_0x354.rgba = 0x40a0a0a0;

	if (this->behaviourCompanion.GetActiveAlert() != (CompanionAlert*)0x0) {
		pHero = this->pActorHero;
		fVar2 = (this->behaviourCompanion).field_0x30.x - pHero->currentLocation.x;
		fVar3 = (this->behaviourCompanion).field_0x30.y - pHero->currentLocation.y;
		fVar4 = (this->behaviourCompanion).field_0x30.z - pHero->currentLocation.z;
		if (fVar2 * fVar2 + fVar3 * fVar3 + fVar4 * fVar4 <= this->behaviourCompanion.GetActiveAlert()->field_0x50) {
			return;
		}
	}

	if (this->behaviourCompanion.GetActiveAlert() != (CompanionAlert*)0x0) {
		this->behaviourCompanion.FUN_001e5790();
	}

	if (this->pTiedActor != (CActor*)0x0) {
		TieToActor((CActor*)0x0, 0, 1, (edF32MATRIX4*)0x0);
	}

	SetState(0xb, -1);

	return;
}

_rgba _rgba_0042ca48 = { 0x80, 0x80, 0x80, 0x80 };

void CActorCompanion::FUN_001e82a0()
{
	bool bVar1;
	CFrontendDisplay* pFrontend;
	edDList_material* pMaterialInfo;
	ulong flags;
	float fVar4;
	float zRotation;
	edF32VECTOR2 translation;
	float local_4;
	C3DFileManager* pFileManager;

	pFileManager = CScene::ptable.g_C3DFileManager_00451664;
	pFrontend = (CFrontendDisplay*)CScene::GetManager(MO_Frontend);
	zRotation = 0.0f;
	flags = 0;
	pFrontend->pInventory->ComputeGameScreenCoordinate(&translation, &(this->behaviourCompanion).field_0x40, &local_4, 0);
	if (local_4 < 0.0f) {
		translation.x = -translation.x * 500.0f;
	}

	if (translation.x < 0.1f) {
		flags = 1;
	}

	else {
		if (0.9f < translation.x) {
			flags = 2;
		}
	}

	if ((flags != 0) && (bVar1 = GuiDList_BeginCurrent(), bVar1 != false)) {
		if (translation.y < 0.1f) {
			translation.y = 0.1f;
		}

		if (0.9f < translation.y) {
			translation.y = 0.9f;
		}

		if ((flags & 1) != 0) {
			translation.x = 0.1f;
			zRotation = 3.141593f;
		}

		if ((flags & 2) != 0) {
			translation.x = 0.9f;
		}

		pMaterialInfo = pFileManager->GetMaterialFromId(this->materialId_0x35c, 2);
		edDListUseMaterial(pMaterialInfo);
		edDListLoadIdentity();

		fVar4 = fmodf(GetTimer()->scaledTotalTime, 3.0f);
		if (fVar4 < 1.5f) {
			fVar4 = cosf(((1.0f - fVar4 / 1.5f) * 6.283185f - 1.570796f));
		}
		else {
			fVar4 = cosf((((fVar4 - 1.5f) / 1.5f) * 6.283185f - 1.570796f));
		}

		edDListColor4u8(_rgba_0042ca48.r, _rgba_0042ca48.g, _rgba_0042ca48.b, static_cast<byte>(static_cast<float>(_rgba_0042ca48.a) * fabs(fVar4)));
		edDListBegin(0.0f, 0.0f, 0.0f, 8, 4);
		edDListRotateZ(zRotation);
		edDListTranslate(translation.x * (float)gVideoConfig.screenWidth, translation.y * (float)gVideoConfig.screenHeight, 0.0f);
		edDListTexCoo2f(0.0f, 0.0f);
		edDListVertex4f((float)gVideoConfig.screenWidth * -0.04f, (float)gVideoConfig.screenHeight * -0.04f, 0.0f, 0.0f);
		edDListTexCoo2f(1.0f, 0.0f);
		edDListVertex4f((float)gVideoConfig.screenWidth * 0.04f, (float)gVideoConfig.screenHeight * -0.04f, 0.0f, 0.0f);
		edDListTexCoo2f(0.0f, 1.0f);
		edDListVertex4f((float)gVideoConfig.screenWidth * -0.04f, (float)gVideoConfig.screenHeight * 0.04f, 0.0f, 0.0f);
		edDListTexCoo2f(1.0f, 1.0f);
		edDListVertex4f((float)gVideoConfig.screenWidth * 0.04f, (float)gVideoConfig.screenHeight * 0.04f, 0.0f, 0.0f);
		edDListEnd();
		GuiDList_EndCurrent();
	}

	return;
}

float ComputeAccelDistance(float param_1, float param_2, float param_3)
{
	return ((param_2 + param_1) / 2.0f) * ((param_2 - param_1) / param_3);
}

float CMP_STAND_ROTSPEED = 6.283185f; // 2 * PI

float CActorCompanion::SV_CMP_MoveToSpecial(float speed, float acceleration, float deceleration, float followTrigLow, CActor* pActor, edF32VECTOR4* pPosition, uint flags)
{
	float fVar1;
	float fVar2;
	float fVar3;
	edF32VECTOR4 local_50;
	CActorMovParamsIn movParamsIn;
	CActorMovParamsOut movParamsOut;

	movParamsOut.flags = 0;
	movParamsIn.flags = 0;
	movParamsIn.pRotation = (edF32VECTOR4*)0x0;
	movParamsIn.speed = 0.0f;

	if (followTrigLow == 0.0f) {
		fVar3 = pPosition->x - this->currentLocation.x;
		fVar2 = pPosition->y - this->currentLocation.y;
		fVar1 = pPosition->z - this->currentLocation.z;
		local_50.x = pPosition->x;
		fVar3 = sqrtf(fVar3 * fVar3 + fVar2 * fVar2 + fVar1 * fVar1);
		local_50.y = pPosition->y;
		local_50.z = pPosition->z;
		local_50.w = pPosition->w;
	}
	else {
		edF32Vector4SubHard(&local_50, pPosition, &this->currentLocation);
		fVar3 = edF32Vector4NormalizeHard(&local_50, &local_50);
		edF32Vector4ScaleHard(fVar3 - followTrigLow, &local_50, &local_50);
		edF32Vector4AddHard(&local_50, &local_50, &this->currentLocation);
		fVar3 = fabs(fVar3 - followTrigLow);
	}

	fVar2 = ComputeAccelDistance(this->dynamic.speed, 0.0f, -deceleration);
	movParamsIn.speed = speed;
	movParamsIn.acceleration = acceleration;
	if (fVar3 < fVar2) {
		movParamsIn.speed = 0.0f;
		movParamsIn.acceleration = deceleration;
	}

	movParamsIn.flags = movParamsIn.flags | 0x400;
	movParamsIn.rotSpeed = CMP_STAND_ROTSPEED;
	movParamsIn.flags = movParamsIn.flags | flags | 2;
	SV_MOV_MoveTo_AvoidActor(&movParamsOut, &movParamsIn, &local_50, pActor);
	ManageDyn(4.0f, 0x100, (CActorsTable*)0x0);

	return movParamsOut.moveVelocity;
}

void CBehaviourCompanion::Create(ByteCode* pByteCode)
{
	S_NTF_TARGET_STREAM_REF* pSVar1;
	S_STREAM_EVENT_CAMERA* pSVar2;
	int iVar3;
	int* pBase;
	CompanionAlert* pCVar4;
	int uVar4;
	int iVar5;
	CCamera* pCVar6;
	int iVar7;
	ulong uVar8;
	CompanionAlert* pCVar9;
	float fVar10;

	this->nbSubObjs = pByteCode->GetS32();
	this->aSubObjs = (CompanionAlert*)0x0;
	uVar4 = this->nbSubObjs;
	if (0 < uVar4) {
		this->aSubObjs = new CompanionAlert[uVar4];

		pCVar9 = this->aSubObjs;
		for (iVar3 = 0; iVar3 < this->nbSubObjs; iVar3 = iVar3 + 1) {
			pCVar9->field_0x64.Create(pByteCode);

			pCVar9->flags_0x0 = (ushort)pByteCode->GetU32() & 0xf;
			pCVar9->flags_0x2 = 0;
			pCVar9->flags_0x4 = 4;
			pCVar9->field_0xc = 0;

			pCVar9->field_0x8 = pByteCode->GetS32();
			if (pCVar9->field_0x8 != -1) {
				pCVar9->flags_0x2 = pCVar9->flags_0x2 | 1;
			}

			pCVar9->field_0x10 = pByteCode->GetS32();
			pCVar9->field_0x14 = pByteCode->GetS32();
			pCVar9->field_0x18 = pByteCode->GetF32();

			if (pCVar9->field_0x14 == -1) {
				pCVar9->field_0x14 = pCVar9->field_0x10;
			}

			if (pCVar9->field_0x10 != -1) {
				pCVar9->flags_0x2 = pCVar9->flags_0x2 | 0x20;
			}

			pCVar9->pActor.index = pByteCode->GetS32();
			pCVar9->field_0x20.x = pByteCode->GetF32();
			pCVar9->field_0x20.y = pByteCode->GetF32();
			pCVar9->field_0x20.z = pByteCode->GetF32();
			pCVar9->field_0x20.w = 0.0f;

			if (pCVar9->pActor.index != -1) {
				pCVar9->flags_0x2 = pCVar9->flags_0x2 | COMPANION_SUB_OBJ_FLAG_HAS_ACTOR;
			}

			if (fabs(pCVar9->field_0x20.z) + fabs(pCVar9->field_0x20.x) + fabs(pCVar9->field_0x20.y) != 0.0f) {
				pCVar9->flags_0x2 = pCVar9->flags_0x2 | 0x80;
			}

			(pCVar9->pCamera).index = pByteCode->GetS32();
			if ((pCVar9->pCamera).index != -1) {
				pCVar9->flags_0x2 = pCVar9->flags_0x2 | COMPANION_SUB_OBJ_FLAG_HAS_CAMERA;
			}

			pCVar9->field_0x44 = pByteCode->GetS32();
			for (iVar5 = 0; iVar5 < pCVar9->field_0x44; iVar5 = iVar5 + 1) {
				pCVar9->field_0x34[iVar5] = pByteCode->GetS32();
			}

			if (0 < pCVar9->field_0x44) {
				pCVar9->flags_0x2 = pCVar9->flags_0x2 | 8;
			}

			pCVar9->levelStringKey = pByteCode->GetU64();

			pCVar9->field_0x50 = pByteCode->GetF32();
			pCVar9->field_0x50 = pCVar9->field_0x50 * pCVar9->field_0x50;

			if (pCVar9->levelStringKey != 0) {
				pCVar9->flags_0x2 = pCVar9->flags_0x2 | 2;
			}

			pCVar9->field_0x58 = pByteCode->GetS32();
			if (pCVar9->field_0x58 != -1) {
				pCVar9->flags_0x2 = pCVar9->flags_0x2 | 0x10;
			}

			pCVar9->field_0x5c = pByteCode->GetF32();
			if (0.0f < fVar10) {
				pCVar9->flags_0x2 = pCVar9->flags_0x2 | 0x100;
			}

			pCVar9->targetSwitch.Create(pByteCode);
			pCVar9->field_0x70.Create(pByteCode);

			pCVar9 = pCVar9 + 1;
		}
	}

	return;
}

void CBehaviourCompanion::Init(CActor* pOwner)
{
	S_NTF_TARGET_STREAM_REF* pSVar1;
	int iVar2;
	CompanionAlert* pCVar3;
	int iVar4;
	int iVar5;

	pCVar3 = this->aSubObjs;
	iVar2 = 0;
	if (0 < this->nbSubObjs) {
		do {
			if ((pCVar3->flags_0x2 & COMPANION_SUB_OBJ_FLAG_HAS_ACTOR) != 0) {
				pCVar3->pActor.Init();
			}

			if ((pCVar3->flags_0x2 & COMPANION_SUB_OBJ_FLAG_HAS_CAMERA) != 0) {
				pCVar3->pCamera.Init();
			}

			pCVar3->targetSwitch.Init();
			pCVar3->targetSwitch.Reset(pOwner);

			iVar2 = iVar2 + 1;
			pCVar3 = pCVar3 + 1;
		} while (iVar2 < this->nbSubObjs);
	}
	return;
}

void CBehaviourCompanion::Manage()
{
	CActorCompanion* pCVar1;
	CCollision* pCVar2;
	CActorHero* pCVar3;
	CActor* pCVar4;
	bool bVar5;
	CCameraManager* pCVar6;
	CEventManager* pCVar7;
	int iVar8;
	ed_zone_3d* peVar9;
	uint uVar10;
	uint uVar11;
	Timer* pTVar12;
	ulong uVar13;
	CompanionAlert* pCVar14;
	int iVar15;
	CompanionAlert* iVar16;
	int iVar17;
	float fVar18;
	float fVar19;
	float fVar20;
	edF32VECTOR4 eStack48;
	edF32VECTOR4 eStack32;
	edF32VECTOR4 local_10;

	pCVar7 = CScene::ptable.g_EventManager_006f5080;

	if (this->pOwner->actorState == 0xd) {
		if (CLevelScheduler::ScenVar_Get(SCN_GAME_COMPANION) == 0) {
			return;
		}

		this->pOwner->flags = this->pOwner->flags | 2;
		this->pOwner->flags = this->pOwner->flags & 0xfffffffe;

		this->pOwner->flags = this->pOwner->flags & 0xffffff5f;
		this->pOwner->EvaluateDisplayState();

		pCVar2 = this->pOwner->pCollisionData;
		pCVar2->flags_0x0 = pCVar2->flags_0x0 | 0x81000;

		this->pOwner->flags = this->pOwner->flags | 0x400;
		this->field_0x68 = 1;
		this->pOwner->SetState(0xb, -1);

		return;
	}

	if (this->field_0x68 != 0) {
		this->pOwner->BehaviourCompanion_Manage();
	}

	if (this->field_0x5c != 0) {
		FUN_001e56b0(1);
	}

	FUN_001e50a0();

	if (GetActiveAlert() == (CompanionAlert*)0x0) {
		iVar8 = this->field_0x10;

		if (GetAlert(iVar8) == (CompanionAlert*)0x0) {
			if (this->field_0x54 != 0) {
				this->activeSubObjId = -1;

				iVar8 = 0;
				if (0 < this->nbSubObjs) {
					do {
						iVar16 = this->aSubObjs + iVar8;
						bVar5 = false;
						if (((iVar16->flags_0x4 & 4) != 0) &&
							(((iVar16->flags_0x2 & 0x40) == 0 || (bVar5 = false, (((iVar16->pActor).Get())->flags & 0x2000001) == 0)))) {
							uVar13 = iVar16->field_0x64.IsVerified();
							bVar5 = false;
							if (uVar13 != 0) {
								bVar5 = true;
							}
						}

						if ((bVar5) && ((iVar16->flags_0x2 & 1) != 0)) {
							peVar9 = (ed_zone_3d*)0x0;
							if (iVar16->field_0x8 != 0xffffffff) {
								peVar9 = edEventGetChunkZone(pCVar7->activeChunkId, iVar16->field_0x8);
							}

							uVar10 = edEventComputeZoneAgainstVertex(pCVar7->activeChunkId, peVar9, &this->pOwner->pActorHero->currentLocation, 0);
							if ((uVar10 & 1 | (uint)iVar16->field_0xc << 1) == 1) {
								CompanionAlert* pSubObj = this->aSubObjs + iVar8;
								bVar5 = false;
								if (((pSubObj->flags_0x4 & 4) != 0) &&
									(((pSubObj->flags_0x2 & 0x40) == 0 ||
										(bVar5 = false, pSubObj->pActor.Get()->flags & 0x2000001) == 0))) {
									uVar13 = pSubObj->field_0x64.IsVerified();
									bVar5 = false;
									if (uVar13 != 0) {
										bVar5 = true;
									}
								}

								if (bVar5) {
									this->field_0x1c = 0;
									this->displayTime = 0.0;
									this->field_0x10 = iVar8;
									this->aSubObjs[this->field_0x10].field_0x60 = this->aSubObjs[this->field_0x10].field_0x5c;
									pCVar14 = this->aSubObjs + this->field_0x10;
									if ((((pCVar14->flags_0x2 & 0x100) == 0) && ((pCVar14->flags_0x0 & 2) != 0)) &&
										((pCVar14->flags_0x4 & 1) == 0)) {
										FUN_001e56b0(1);
									}

									iVar15 = this->pOwner->actorState;
									if ((iVar15 == 0xb) || (iVar15 == 0xe)) {
										pCVar3 = this->pOwner->pActorHero;
										local_10.x = pCVar3->currentLocation.x;
										local_10.z = pCVar3->currentLocation.z;
										local_10.w = pCVar3->currentLocation.w;
										local_10.y = pCVar3->currentLocation.y + 15.0f;
										this->pOwner->UpdatePosition(&local_10, true);
									}

									this->field_0x54 = 0;
								}
							}
							if ((iVar16->flags_0x2 & 0x10) != 0) {
								peVar9 = (ed_zone_3d*)0x0;
								if (iVar16->field_0x58 != 0xffffffff) {
									peVar9 = edEventGetChunkZone(pCVar7->activeChunkId, iVar16->field_0x58);
								}

								uVar11 = edEventComputeZoneAgainstVertex(pCVar7->activeChunkId, peVar9, &this->pOwner->pActorHero->currentLocation, 0);
								if ((uVar11 & 1) != 0) {
									iVar16->flags_0x4 = iVar16->flags_0x4 & 0xfffb;
								}
							}

							iVar16->field_0xc = (uVar10 & 1);
						}

						iVar8 = iVar8 + 1;
					} while (iVar8 < this->nbSubObjs);
				}

				pCVar6 = CCameraManager::_gThis;

				if (GetActiveAlert() != (CompanionAlert*)0x0) {
					pCVar1 = this->pOwner;
					if ((this->pOwner->flags & 0x4000) == 0) {
						edF32Vector4ScaleHard(-2.0f, &eStack48, &(CCameraManager::_gThis->transformationMatrix).rowZ);
						edF32Vector4AddHard(&eStack32, &pCVar6->transformationMatrix.rowT, &eStack48);
						eStack32.y = pCVar1->pActorHero->currentLocation.y + 2.5f;
						pCVar1->UpdatePosition(&eStack32, true);
					}

					this->pOwner->flags = this->pOwner->flags | 2;
					this->pOwner->flags = this->pOwner->flags & 0xfffffffe;
					this->pOwner->flags = this->pOwner->flags & 0xffffff5f;
					this->pOwner->EvaluateDisplayState();
					pCVar2 = this->pOwner->pCollisionData;
					pCVar2->flags_0x0 = pCVar2->flags_0x0 | 0x81000;
					this->pOwner->flags = this->pOwner->flags | 0x400;
					this->field_0x68 = 1;
					this->pOwner->SetState(8, -1);
				}
			}
		}
		else {
			pCVar14 = GetAlert(iVar8);

			if ((pCVar14->flags_0x2 & 1) != 0) {
				peVar9 = (ed_zone_3d*)0x0;
				if (pCVar14->field_0x8 != 0xffffffff) {
					peVar9 = edEventGetChunkZone(pCVar7->activeChunkId, pCVar14->field_0x8);
				}

				uVar10 = edEventComputeZoneAgainstVertex(pCVar7->activeChunkId, peVar9, &this->pOwner->pActorHero->currentLocation, 0);

				if ((uVar10 & 1 | (uint)pCVar14->field_0xc << 1) == 2) {
					this->field_0x10 = -1;
					this->field_0x54 = 1;
					this->pOwner->SetState(0xb, -1);
				}

				pCVar14->field_0xc = uVar10 & 1;
			}
		}
	}
	else {
		pCVar14 = GetActiveAlert();

		if ((pCVar14->flags_0x2 & 1) != 0) {
			peVar9 = (ed_zone_3d*)0x0;
			if (pCVar14->field_0x8 != 0xffffffff) {
				peVar9 = edEventGetChunkZone(pCVar7->activeChunkId, pCVar14->field_0x8);
			}

			uVar10 = edEventComputeZoneAgainstVertex(pCVar7->activeChunkId, peVar9, &this->pOwner->pActorHero->currentLocation, 0);
			if ((uVar10 & 1 | (uint)pCVar14->field_0xc << 1) == 2) {
				FUN_001e5790();
				this->pOwner->SetState(0xb, -1);
			}

			pCVar14->field_0xc = uVar10 & 1;
		}
	}

	if (this->field_0x10 != -1) {
		if (this->field_0x5c != 0) {
			FUN_001e56b0(1);
		}

		if ((this->aSubObjs[this->field_0x10].flags_0x2 & 0x100) != 0) {
			this->aSubObjs[this->field_0x10].field_0x60 = this->aSubObjs[this->field_0x10].field_0x60 - GetTimer()->cutsceneDeltaTime;

			if (0.0f < this->aSubObjs[this->field_0x10].field_0x60) goto LAB_001e7598;
		}

		FUN_001e58e0();
	}

LAB_001e7598:
	iVar8 = this->activeSubObjId;
	if (iVar8 != -1) {
		if ((GetAlert(iVar8)->flags_0x4 & 8) != 0) {
			GetAlert(iVar8)->targetSwitch.pStreamEventCamera->Manage(this->pOwner);

			iVar8 = this->activeSubObjId;
			if ((GetAlert(iVar8)->flags_0x2 & 0x40) != 0) {
				pCVar4 = (GetAlert(iVar8)->pActor).Get();
				this->field_0x40 = pCVar4->currentLocation;
			}
		}
	}

	return;
}

static uint gBorderColorA = 0x555B6287;
static uint gBorderColorB = 0x5B6287;

void CBehaviourCompanion::Draw()
{
	int iVar1;
	bool bVar2;
	Timer* pTVar3;
	CompanionAlert* pCVar4;
	CompanionAlert* pCVar5;
	float x;
	float bgX;
	float bgY;

	if (((GameFlags & 0x20) == 0) && (this->field_0x64 != 0)) {
		iVar1 = this->field_0x18;

		if (GetAlert(iVar1) != (CompanionAlert*)0x0) {
			if ((GetAlert(iVar1)->flags_0x2 & 2) != 0) {
				if (this->field_0x60 == 0) {
					if ((GetAlert(iVar1)->flags_0x4 & 2) != 0) {
						pCVar4 = GetAlert(iVar1);

						if (this->activeSubObjId < 0) {
							pCVar5 = (CompanionAlert*)0x0;
						}
						else {
							pCVar5 = this->aSubObjs + this->activeSubObjId;
						}

						if (pCVar5 == pCVar4) {
							this->displayTime = 0.0f;
							this->field_0x1c = 0.0f;
							x = fmodf(GetTimer()->scaledTotalTime, 3.0f);
							bVar2 = GuiDList_BeginCurrent();
							if (bVar2 != false) {
								_Display_Text(0.0f, 0.7f, x);
								GuiDList_EndCurrent();
							}
						}
					}
				}
				else {
					if (this->field_0x60 == 1) {
						x = this->field_0x1c + GetTimer()->cutsceneDeltaTime;
						this->field_0x1c = x;

						if (0.5f < x) {
							this->field_0x1c = 0.5f;
							this->displayTime = this->displayTime + GetTimer()->cutsceneDeltaTime;
						}

						if ((GetAlert(this->field_0x18)->flags_0x4 & 1) == 0) {
							x = -0.15f;
						}
						else {
							x = 0.0f;
							this->displayTime = 10000.0f;
						}
					}
					else {
						x = this->field_0x1c - GetTimer()->cutsceneDeltaTime;
						this->field_0x1c = x;
						if (x < 0.0f) {
							this->displayTime = 0.0f;
							this->field_0x1c = 0.0f;
							this->field_0x60 = 0;
							GetAlert(this->field_0x18)->flags_0x4 = GetAlert(this->field_0x18)->flags_0x4 & 0xffdf;
						}
						x = -0.15f;
					}

					bgX = this->field_0x1c / 0.5f;
					if (bgX < 0.5f) {
						bgY = 1.1f;
						x = x + bgX * 2.0f * -x;
					}
					else {
						bgY = (bgX - 0.5f) * 2.0f * -0.4f + 1.1f;
						x = 0.0f;
					}

					bgX = 0.54f;
					bVar2 = GuiDList_BeginCurrent();
					if (bVar2 != false) {
						CPauseManager::DrawRectangleBorder(bgX * (float)gVideoConfig.screenWidth, (bgY + 0.125f) * (float)gVideoConfig.screenHeight,
							(float)gVideoConfig.screenWidth * 0.91f, (float)gVideoConfig.screenHeight * 0.3f, 12.0f, 12.0f,
							gBorderColorA, gBorderColorB, 0);
						_Display_BGText(bgX, bgY, this->displayTime);
						_Display_Text(x, 0.7f, 3.0f);
						GuiDList_EndCurrent();
					}
				}
			}
		}
	}

	return;
}

void CBehaviourCompanion::Begin(CActor* pOwner, int newState, int newAnimationType)
{
	char cVar1;
	CCollision* pCVar3;
	int iVar4;
	char* pcVar5;
	CompanionAlert* iVar9;
	int iVar10;
	int iVar12;
	int iVar11;

	this->pOwner = reinterpret_cast<CActorCompanion*>(pOwner);
	this->pOwner->pActorHero = CActorHero::_gThis;

	if (newState == -1) {
		this->pOwner->SetState(0xb, -1);
	}
	else {
		this->pOwner->SetState(newState, newAnimationType);
	}

	this->pOwner->SetState(0xe, -1);

	if (CLevelScheduler::ScenVar_Get(SCN_GAME_COMPANION) == 0) {
		this->pOwner->SetState(0xd, -1);
	}

	this->pOwner->flags = this->pOwner->flags | 2;
	this->pOwner->flags = this->pOwner->flags & 0xfffffffe;

	this->pOwner->flags = this->pOwner->flags & 0xffffff7f;
	this->pOwner->flags = this->pOwner->flags | 0x20;

	this->pOwner->EvaluateDisplayState();

	pCVar3 = this->pOwner->pCollisionData;
	pCVar3->flags_0x0 = pCVar3->flags_0x0 | 0x81000;
	this->pOwner->flags = this->pOwner->flags | 0x400;
	this->field_0x68 = 0;
	iVar4 = 0;
	if (0 < this->nbSubObjs) {
		do {
			this->aSubObjs[iVar4].flags_0x4 = this->aSubObjs[iVar4].flags_0x4 & 0xfff4;
			this->aSubObjs[iVar4].flags_0x4 = this->aSubObjs[iVar4].flags_0x4 | 4;
			this->aSubObjs[iVar4].field_0x60 = this->aSubObjs[iVar4].field_0x5c;

			iVar9 = this->aSubObjs + iVar4;
			if ((iVar9->flags_0x2 & 2) != 0) {
				pcVar5 = CScene::ptable.g_LocalizationManager_00451678->GetLevelString(iVar9->levelStringKey);
				iVar12 = 0;
				iVar10 = 0;
				do {
					if (pcVar5[iVar10] == '%') {
						do {
							iVar11 = iVar10;
							iVar10 = iVar11 + 1;
							cVar1 = pcVar5[iVar10];
							if (cVar1 == ']') break;
						} while (cVar1 != '%');
						if (cVar1 == ']') {
							iVar10 = iVar11 + 2;
						}
					}
					else {
						if (' ' < pcVar5[iVar10]) {
							iVar12 = iVar12 + 1;
						}
					}
					iVar10 = iVar10 + 1;
				} while (pcVar5[iVar10] != '\0');
				this->aSubObjs[iVar4].levelStringLen = iVar12;
			}

			iVar4 = iVar4 + 1;
		} while (iVar4 < this->nbSubObjs);
	}

	this->field_0x60 = 0;
	this->field_0x64 = 0;
	this->field_0x5c = 0;
	this->field_0x58 = 0;
	this->field_0x1c = 0;
	this->activeSubObjId = -1;
	this->field_0x18 = -1;
	this->field_0x10 = -1;
	this->field_0x54 = 1;

	return;
}

int CBehaviourCompanion::InterpretMessage(CActor* pSender, int msg, void* pMsgParam)
{
	CActorCompanion* pCVar1;
	CCollision* pCVar2;
	CActorHero* pCVar3;
	bool bVar4;
	int iVar5;
	ulong uVar6;
	CompanionAlert* pCVar8;
	CompanionAlert* pCVar7;
	edF32VECTOR4 local_10;

	if (msg == 0x87) {
		const int index = static_cast<int>(reinterpret_cast<uintptr_t>(pMsgParam));
		this->aSubObjs[index].flags_0x4 = this->aSubObjs[index].flags_0x4 & 0xfffb;
		if (index == this->activeSubObjId) {
			FUN_001e5790();
		}

		iVar5 = 1;
	}
	else {
		if (msg == 0x7b) {
			IMPLEMENTATION_GUARD(
			pCVar7 = this->aSubObjs + pMsgParam;
			bVar4 = false;
			if (((pCVar7->flags_0x4 & 4) != 0) &&
				(((pCVar7->flags_0x2 & 0x40) == 0 || (bVar4 = false, (((pCVar7->pActor).pActor)->flags & 0x2000001) == 0)))) {
				uVar6 = CScenaricCondition::IsVerified(&pCVar7->field_0x64);
				if (uVar6 == 0) {
					bVar4 = false;
				}
				else {
					bVar4 = true;
				}
			}
			if (bVar4) {
				this->field_0x1c = 0;
				this->displayTime = 0.0;
				this->field_0x10 = pMsgParam;
				this->aSubObjs[this->field_0x10].field_0x60 = this->aSubObjs[this->field_0x10].field_0x5c;
				pCVar8 = this->aSubObjs + this->field_0x10;
				if ((((pCVar8->flags_0x2 & 0x100) == 0) && ((pCVar8->flags_0x0 & 2) != 0)) && ((pCVar8->flags_0x4 & 1) == 0)) {
					FUN_001e56b0(1);
				}
				iVar5 = this->pOwner->actorState;
				if ((iVar5 == 0xb) || (iVar5 == 0xe)) {
					pCVar3 = this->pOwner->pActorHero;
					local_10.x = (pCVar3->character).characterBase.base.base.currentLocation.x;
					local_10.z = (pCVar3->character).characterBase.base.base.currentLocation.z;
					local_10.w = (pCVar3->character).characterBase.base.base.currentLocation.w;
					local_10.y = (pCVar3->character).characterBase.base.base.currentLocation.y + 15.0;
					CActor::UpdatePosition((CActor*)this->pOwner, &local_10, true);
				}
				this->field_0x54 = 0;
				iVar5 = 1;
				this->aSubObjs[this->field_0x10].flags_0x4 = this->aSubObjs[this->field_0x10].flags_0x4 | 0x10;
			}
			else {
				iVar5 = 0;
			})
		}
		else {
			if (msg == 0x5c) {
				CLevelScheduler::ScenVar_Set(SCN_GAME_COMPANION, 1);

				this->pOwner->flags = this->pOwner->flags | 2;
				this->pOwner->flags = this->pOwner->flags & 0xfffffffe;

				this->pOwner->flags = this->pOwner->flags & 0xffffff5f;
				this->pOwner->EvaluateDisplayState();

				iVar5 = 1;

				pCVar2 = this->pOwner->pCollisionData;
				pCVar2->flags_0x0 = pCVar2->flags_0x0 | 0x81000;
				this->pOwner->flags = this->pOwner->flags | 0x400;
				this->field_0x68 = 1;
			}
			else {
				if (msg == 0x5d) {
					CLevelScheduler::ScenVar_Set(SCN_GAME_COMPANION, 0);

					this->pOwner->SetState(0xe, -1);

					this->pOwner->flags = this->pOwner->flags & 0xfffffffd;
					this->pOwner->flags = this->pOwner->flags | 1;

					this->pOwner->flags = this->pOwner->flags & 0xffffff7f;
					this->pOwner->flags = this->pOwner->flags | 0x20;

					this->pOwner->EvaluateDisplayState();

					iVar5 = 1;
					pCVar2 = this->pOwner->pCollisionData;
					pCVar2->flags_0x0 = pCVar2->flags_0x0 & 0xfff7efff;
					this->pOwner->flags = this->pOwner->flags & 0xfffffbff;
					this->field_0x68 = 0;
				}
				else {
					if (msg == 3) {
						this->pOwner->SetState(0xe, -1);

						this->pOwner->flags = this->pOwner->flags & 0xfffffffd;
						this->pOwner->flags = this->pOwner->flags | 1;

						this->pOwner->flags = this->pOwner->flags & 0xffffff7f;
						this->pOwner->flags = this->pOwner->flags | 0x20;

						this->pOwner->EvaluateDisplayState();

						iVar5 = 1;
						pCVar2 = this->pOwner->pCollisionData;
						pCVar2->flags_0x0 = pCVar2->flags_0x0 & 0xfff7efff;
						this->pOwner->flags = this->pOwner->flags & 0xfffffbff;
						this->field_0x68 = 0;
					}
					else {
						iVar5 = 0;
					}
				}
			}
		}
	}

	return iVar5;
}

void CBehaviourCompanion::FUN_001e50a0()
{
	float fVar1;
	float fVar2;
	float fVar3;
	bool bVar4;
	bool bVar5;
	bool bVar6;
	CActorCompanion* pCompanion;
	CPlayerInput* pPlayerInput;
	CompanionAlert* pCVar7;
	int iVar8;
	CActorHero* pHero;

	pHero = this->pOwner->pActorHero;
	iVar8 = 0;
	bVar6 = pHero->IsFightRelated(pHero->curBehaviourId);
	if ((bVar6 == false) && (bVar6 = g_CinematicManager_0048efc->FUN_001c5c60(), bVar6 == false)) {
		iVar8 = 1;
	}

	this->field_0x64 = iVar8;
	pCompanion = static_cast<CActorCompanion*>(this->pOwner->pActorHero->GetTalkingToActor());
	if ((pCompanion != (CActorCompanion*)0x0) && (pCompanion != this->pOwner)) {
		this->field_0x64 = 0;
	}

	if (this->field_0x64 == 0) {
		this->field_0x1c = 0;
		this->displayTime = 0.0f;
		this->field_0x60 = 0;

		if (this->field_0x18 < 0) {
			pCVar7 = (CompanionAlert*)0x0;
		}
		else {
			pCVar7 = this->aSubObjs + this->field_0x18;
		}

		if (pCVar7 == (CompanionAlert*)0x0) {
			return;
		}

		if (this->field_0x18 < 0) {
			pCVar7 = (CompanionAlert*)0x0;
		}
		else {
			pCVar7 = this->aSubObjs + this->field_0x18;
		}

		pCVar7->flags_0x4 = pCVar7->flags_0x4 & 0xffdf;
		return;
	}

	if (this->field_0x18 < 0) {
		pCVar7 = (CompanionAlert*)0x0;
	}
	else {
		pCVar7 = this->aSubObjs + this->field_0x18;
	}

	if (pCVar7 == (CompanionAlert*)0x0) {
		return;
	}

	pHero = this->pOwner->pActorHero;
	pPlayerInput = pHero->GetInputManager(0, 0);
	bVar6 = false;
	if (this->field_0x18 < 0) {
		pCVar7 = (CompanionAlert*)0x0;
	}
	else {
		pCVar7 = this->aSubObjs + this->field_0x18;
	}

	if ((pCVar7->flags_0x2 & 2) != 0) {
		if (this->field_0x18 < 0) {
			pCVar7 = (CompanionAlert*)0x0;
		}
		else {
			pCVar7 = this->aSubObjs + this->field_0x18;
		}
		if ((pCVar7->flags_0x4 & 0x20) == 0) {
			bVar6 = true;
		}
	}

	bVar5 = false;
	bVar4 = false;
	if (bVar6) {
		if (this->field_0x18 < 0) {
			pCVar7 = (CompanionAlert*)0x0;
		}
		else {
			pCVar7 = this->aSubObjs + this->field_0x18;
		}
		if ((pCVar7->flags_0x0 & 4) != 0) {
			bVar4 = true;
		}
	}

	if (bVar4) {
		if (this->field_0x18 < 0) {
			pCVar7 = (CompanionAlert*)0x0;
		}
		else {
			pCVar7 = this->aSubObjs + this->field_0x18;
		}
		if ((pCVar7->flags_0x4 & 1) == 0) {
			bVar5 = true;
		}
	}

	bVar4 = false;
	if ((pPlayerInput != (CPlayerInput*)0x0) && ((pPlayerInput->pressedBitfield & 0x100) != 0)) {
		bVar4 = true;
	}
	if (((bVar5) || (bVar4)) && (bVar6)) {
		if (this->field_0x18 < 0) {
			pCVar7 = (CompanionAlert*)0x0;
		}
		else {
			pCVar7 = this->aSubObjs + this->field_0x18;
		}

		pCVar7->flags_0x4 = pCVar7->flags_0x4 | 2;
		if (this->field_0x60 == 0) {
			this->field_0x60 = 1;
			if (this->field_0x18 < 0) {
				pCVar7 = (CompanionAlert*)0x0;
			}
			else {
				pCVar7 = this->aSubObjs + this->field_0x18;
			}
			pCVar7->flags_0x4 = pCVar7->flags_0x4 | 0x20;
		}

		goto LAB_001e5488;
	}

	if (this->field_0x18 < 0) {
		pCVar7 = (CompanionAlert*)0x0;
	}
	else {
		pCVar7 = this->aSubObjs + this->field_0x18;
	}

	if ((pCVar7->flags_0x2 & 2) == 0) {
	LAB_001e53a8:
		if (this->field_0x18 < 0) {
			pCVar7 = (CompanionAlert*)0x0;
		}
		else {
			pCVar7 = this->aSubObjs + this->field_0x18;
		}
		if ((pCVar7->flags_0x2 & 2) != 0) goto LAB_001e5488;
	}
	else {
		if (this->field_0x18 < 0) {
			pCVar7 = (CompanionAlert*)0x0;
		}
		else {
			pCVar7 = this->aSubObjs + this->field_0x18;
		}
		if ((pCVar7->flags_0x4 & 0x20) == 0) goto LAB_001e53a8;
	}

	if (bVar4) {
		this->field_0x60 = 2;
		if (this->field_0x18 < 0) {
			pCVar7 = (CompanionAlert*)0x0;
		}
		else {
			pCVar7 = this->aSubObjs + this->field_0x18;
		}

		pCVar7->flags_0x4 = pCVar7->flags_0x4 | 1;
		if (this->activeSubObjId < 0) {
			pCVar7 = (CompanionAlert*)0x0;
		}
		else {
			pCVar7 = this->aSubObjs + this->activeSubObjId;
		}

		if ((pCVar7 != (CompanionAlert*)0x0) &&
			(((this->aSubObjs[this->activeSubObjId].flags_0x2 & 4) != 0 ||
				((this->aSubObjs[this->activeSubObjId].flags_0x0 & 2) != 0)))) {
			FUN_001e5790();
		}
	}
LAB_001e5488:
	if (this->field_0x18 < 0) {
		pCVar7 = (CompanionAlert*)0x0;
	}
	else {
		pCVar7 = this->aSubObjs + this->field_0x18;
	}

	if (pCVar7 != (CompanionAlert*)0x0) {
		pCompanion = this->pOwner;
		pHero = pCompanion->pActorHero;
		fVar1 = (pCompanion->behaviourCompanion).field_0x30.x - pHero->currentLocation.x;
		fVar2 = (pCompanion->behaviourCompanion).field_0x30.y - pHero->currentLocation.y;
		fVar3 = (pCompanion->behaviourCompanion).field_0x30.z - pHero->currentLocation.z;

		if (this->field_0x18 < 0) {
			pCVar7 = (CompanionAlert*)0x0;
		}
		else {
			pCVar7 = this->aSubObjs + this->field_0x18;
		}

		if (pCVar7->field_0x50 < fVar1 * fVar1 + fVar2 * fVar2 + fVar3 * fVar3) {
			this->field_0x60 = 2;
			if (this->field_0x18 < 0) {
				pCVar7 = (CompanionAlert*)0x0;
			}
			else {
				pCVar7 = this->aSubObjs + this->field_0x18;
			}
			pCVar7->flags_0x4 = pCVar7->flags_0x4 | 3;
		}
	}

	return;
}

void CBehaviourCompanion::FUN_001e5790()
{
	float fVar1;
	float fVar2;
	float fVar3;
	CompanionAlert* pSubObj;
	CCameraManager* pCameraManager;

	pCameraManager = CCameraManager::_gThis;
	this->aSubObjs[this->activeSubObjId].flags_0x4 = this->aSubObjs[this->activeSubObjId].flags_0x4 & 0xffe7;
	if (this->field_0x58 != 0) {
		FUN_001e56b0(0);
	}

	this->field_0x5c = 0;
	if ((this->aSubObjs[this->activeSubObjId].flags_0x2 & 4) != 0) {
		if (this->activeSubObjId < 0) {
			pSubObj = (CompanionAlert*)0x0;
		}
		else {
			pSubObj = this->aSubObjs + this->activeSubObjId;
		}

		pCameraManager->PopCamera((pSubObj->pCamera).Get());
	}

	pSubObj = this->aSubObjs + this->activeSubObjId;
	if ((pSubObj->flags_0x0 & 1) != 0) {
		pSubObj->flags_0x4 = pSubObj->flags_0x4 & 0xfffb;
	}

	pSubObj = this->aSubObjs + this->activeSubObjId;
	if ((pSubObj->flags_0x2 & 2) == 0) {
		pSubObj->flags_0x4 = pSubObj->flags_0x4 & 0xffdf;
	}

	this->field_0x18 = this->activeSubObjId;
	this->activeSubObjId = -1;
	this->field_0x40 = gF32Vector4Zero;

	this->field_0x54 = 1;

	return;
}

void CBehaviourCompanion::FUN_001e58e0()
{
	ushort uVar1;
	CActorHero* pHero;
	CActor* pCVar3;
	CActorCompanion* pCVar4;
	CCollision* pCVar5;
	CompanionAlert* pCVar6;
	CWayPoint* pCVar7;
	S_NTF_TARGET_STREAM_REF* pSVar8;
	int iVar9;
	int iVar10;
	float fVar11;
	float fVar12;
	float fVar13;

	this->activeSubObjId = this->field_0x10;
	this->field_0x18 = this->field_0x10;
	this->field_0x10 = -1;
	this->field_0x1c = 0;
	this->displayTime = 0.0f;
	this->field_0x60 = 0;
	this->aSubObjs[this->activeSubObjId].flags_0x4 = this->aSubObjs[this->activeSubObjId].flags_0x4 & 0xffdf;
	pHero = this->pOwner->pActorHero;
	this->field_0x30 = pHero->currentLocation;
	this->aSubObjs[this->activeSubObjId].flags_0x4 = this->aSubObjs[this->activeSubObjId].flags_0x4 | 8;

	uVar1 = this->aSubObjs[this->activeSubObjId].flags_0x2;
	if ((uVar1 & 0x40) == 0) {
		if ((uVar1 & 0x20) != 0) {
			if (this->activeSubObjId < 0) {
				pCVar6 = (CompanionAlert*)0x0;
			}
			else {
				pCVar6 = this->aSubObjs + this->activeSubObjId;
			}

			pCVar7 = (CWayPoint*)0x0;
			fVar11 = 0.0f;
			if (pCVar6->field_0x10 != -1) {
				pCVar7 = (CScene::ptable.g_WayPointManager_0045169c)->aWaypoints + pCVar6->field_0x10;
			}
			(this->field_0x40).xyz = pCVar7->location;
			(this->field_0x40).w = 1.0f;
		}
	}
	else {
		if (this->activeSubObjId < 0) {
			pCVar6 = (CompanionAlert*)0x0;
		}
		else {
			pCVar6 = this->aSubObjs + this->activeSubObjId;
		}

		pCVar3 = (pCVar6->pActor).Get();
		this->field_0x40 = pCVar3->currentLocation;
	}

	pCVar4 = this->pOwner;
	pCVar6 = this->aSubObjs + this->activeSubObjId;
	pCVar6->targetSwitch.Switch(pCVar4);
	this->aSubObjs[this->activeSubObjId].field_0x70.Perform();

	this->pOwner->SetState(8, -1);

	pCVar4 = this->pOwner;
	pCVar4->flags = pCVar4->flags | 2;
	pCVar4->flags = pCVar4->flags & 0xfffffffe;
	pCVar4 = this->pOwner;
	pCVar4->flags = pCVar4->flags & 0xffffff5f;
	pCVar4->EvaluateDisplayState();
	pCVar5 = this->pOwner->pCollisionData;
	pCVar5->flags_0x0 = pCVar5->flags_0x0 | 0x81000;
	this->pOwner->flags = this->pOwner->flags | 0x400;
	this->field_0x68 = 1;

	return;
}

void CBehaviourCompanion::FUN_001e9660(edF32VECTOR4* param_2)
{
	bool bVar1;
	ushort uVar2;
	CActorHero* pCVar3;
	CActorCompanion* pCompanion;
	float fVar5;
	float fVar6;
	float fVar7;
	float fVar8;
	float fVar9;
	float fVar10;
	CCameraManager* pCameraManager;
	CCameraManager* iVar12;
	int iVar13;
	CompanionAlert* pAlert;
	CWayPoint* pCVar15;
	CCamera* pCVar16;
	edF32VECTOR4 local_b0;
	edF32VECTOR4 local_a0;
	edF32VECTOR4 local_90;
	edF32MATRIX4 eStack128;
	edF32VECTOR4 local_40;
	edF32VECTOR4 eStack48;
	edF32VECTOR4 local_20;
	edF32VECTOR2 eStack16;
	float local_4;

	pCameraManager = CCameraManager::_gThis;
	uVar2 = this->aSubObjs[this->activeSubObjId].flags_0x2;
	if ((uVar2 & 0x40) == 0) {
		if ((uVar2 & 4) == 0) {
			if (this->activeSubObjId < 0) {
				pAlert = (CompanionAlert*)0x0;
			}
			else {
				pAlert = this->aSubObjs + this->activeSubObjId;
			}

			edF32Vector4ScaleHard(pAlert->field_0x18, &local_20, &CCameraManager::_gThis->transformationMatrix.rowX);
		}
		else {
			local_20.x = 0.0f;
			local_20.z = 0.0f;
			local_20.y = 1.0f;
			local_20.w = 0.0f;

			if (this->activeSubObjId < 0) {
				pAlert = (CompanionAlert*)0x0;
			}
			else {
				pAlert = this->aSubObjs + this->activeSubObjId;
			}

			pCVar16 = (pAlert->pCamera).Get();
			local_90 = (pCVar16->transformationMatrix).rowZ;
			edF32Vector4CrossProductHard(&local_20, &local_90, &local_20);
			if (this->activeSubObjId < 0) {
				pAlert = (CompanionAlert*)0x0;
			}
			else {
				pAlert = this->aSubObjs + this->activeSubObjId;
			}

			edF32Vector4ScaleHard(pAlert->field_0x18, &local_20, &local_20);
		}

		if (this->activeSubObjId < 0) {
			pAlert = (CompanionAlert*)0x0;
		}
		else {
			pAlert = this->aSubObjs + this->activeSubObjId;
		}

		pCVar15 = (CWayPoint*)0x0;
		if (pAlert->field_0x14 != -1) {
			pCVar15 = (CScene::ptable.g_WayPointManager_0045169c)->aWaypoints + pAlert->field_0x14;
		}

		local_b0.xyz = pCVar15->location;
		local_b0.w = 1.0f;
		edF32Vector4SubHard(&local_a0, &local_b0, &local_20);
		edF32Vector4AddHard(&local_20, &local_b0, &local_20);
		pCompanion = this->pOwner;
		fVar5 = local_a0.x - pCompanion->currentLocation.x;
		fVar7 = local_a0.y - pCompanion->currentLocation.y;
		fVar9 = local_a0.z - pCompanion->currentLocation.z;
		pCompanion = this->pOwner;
		fVar6 = local_20.x - pCompanion->currentLocation.x;
		fVar8 = local_20.y - pCompanion->currentLocation.y;
		fVar10 = local_20.z - pCompanion->currentLocation.z;
		this->pOwner->field_0x368 = 0;

		if (sqrtf(fVar6 * fVar6 + fVar8 * fVar8 + fVar10 * fVar10) < sqrtf(fVar5 * fVar5 + fVar7 * fVar7 + fVar9 * fVar9)) {
			local_a0 = local_20;
			this->pOwner->field_0x368 = 1;
		}

		*param_2 = local_a0;
	}
	else {
		CFrontendDisplay* pFrontend = reinterpret_cast<CFrontendDisplay*>(CScene::GetManager(MO_Frontend));
		CFrontendInventory* pInventory = pFrontend->pInventory;
		pCVar3 = this->pOwner->pActorHero;
		bVar1 = false;

		if (pCVar3 == this->aSubObjs[this->activeSubObjId].pActor.Get()) {
			iVar13 = pInventory->ComputeGameScreenCoordinate(&eStack16, &pCVar3->currentLocation, &local_4, 0);
			bVar1 = iVar13 == 0;

			if ((this->aSubObjs[this->activeSubObjId].flags_0x2 & 4) != 0) {
				pAlert = GetActiveAlert();
				const int computeResult = pInventory->ComputeGameScreenCoordinate(&eStack16, &this->pOwner->pActorHero->currentLocation, &local_4, (pAlert->pCamera).Get());
				if (computeResult == 0) {
					bVar1 = true;
				}
			}

			if (local_4 <= 0.0f) {
				bVar1 = true;
			}
		}

		if (bVar1) {
			static edF32VECTOR4 edF32VECTOR4_0040eda0 = { 1.0f, 0.5f, 5.0f, 1.0f };
			local_40 = edF32VECTOR4_0040eda0;
			if ((this->aSubObjs[this->activeSubObjId].flags_0x2 & 4) == 0) {
				pCVar16 = (CCamera*)0x0;
			}
			else {
				pAlert = GetActiveAlert();
				pCVar16 = (pAlert->pCamera).Get();
			}

			iVar12 = (CCameraManager*)CScene::GetManager(MO_Camera);
			if (pCVar16 == (CCamera*)0x0) {
				edF32Matrix4MulF32Vector4Hard(param_2, &iVar12->transformationMatrix, &local_40);
			}
			else {
				edF32Matrix4MulF32Vector4Hard(param_2, &pCVar16->transformationMatrix, &local_40);
			}
		}
		else {
			if ((this->aSubObjs[this->activeSubObjId].flags_0x2 & 0x80) == 0) {
				edF32Vector4ScaleHard(this->aSubObjs[this->activeSubObjId].pActor.Get()->subObjA->boundingSphere.w, &eStack48, &pCameraManager->transformationMatrix.rowX);
				eStack48.y = eStack48.y + ((this->aSubObjs[this->activeSubObjId].pActor.Get())->subObjA->boundingSphere).w;
			}
			else {
				edF32Matrix4RotateYHard((pCameraManager->angle_0xa08).y, &eStack128, &gF32Matrix4Unit);
				edF32Matrix4MulF32Vector4Hard(&eStack48, &eStack128, &this->aSubObjs[this->activeSubObjId].field_0x20);
			}

			edF32Vector4AddHard(param_2, &(this->aSubObjs[this->activeSubObjId].pActor.Get())->currentLocation, &eStack48);
		}
	}

	return;
}

void CBehaviourCompanion::FUN_001e56b0(int param_2)
{
	CActorHero* pHero;
	_msg_enter_shop msgEnterShop;
	CActorCompanion* pCompanion;

	pCompanion = this->pOwner;
	pHero = pCompanion->pActorHero;
	if (pHero != (CActorHero*)0x0) {
		if (param_2 == 0) {
			pCompanion->DoMessage(pCompanion->pActorHero, MESSAGE_LEAVE_SHOP, 0);
			this->field_0x5c = 0;
			this->field_0x58 = 0;
		}
		else {
			if ((pHero->GetStateFlags(pHero->actorState) & 0x100) == 0) {
				this->field_0x5c = 1;
			}
			else {
				msgEnterShop.field_0x0 = 0;
				msgEnterShop.field_0x8 = 1;
				msgEnterShop.field_0x4 = 0;
				msgEnterShop.field_0xc = 0;
				this->pOwner->DoMessage(this->pOwner->pActorHero, MESSAGE_ENTER_SHOP, &msgEnterShop);
				this->field_0x5c = 0;
				this->field_0x58 = 1;
			}
		}
	}

	return;
}

_rgba _rgba_0042ca50 = { 0x55, 0x55, 0x00, 0x80 };

void CBehaviourCompanion::_Display_BGText(float x, float y, float displayTime)
{
	char cVar1;
	void* pvVar2;
	CompanionAlert* pSubObj;
	char* dst;
	edCTextStyle* pNewFont;
	char* pText_00;
	undefined uVar3;
	int iVar4;
	int iVar5;
	int nbCharacters;
	Rectangle rect;
	edCTextStyle textStyle;
	char* local_8;
	char* szLevelString;
	int iVar6;

	textStyle.Reset();

	// As displayTime increases, the number of characters displayed increases.
	nbCharacters = (int)(displayTime * 30.0f);

	pvVar2 = (void*)gSP_Manager.GetFreeBuffer(0x400);
#ifdef PLATFORM_WIN
	// We're not using a real scratchpad on here, so we can't run over our buffer like the ps2 did....
	dst = (char*)gSP_Manager.GetFreeBuffer(GetAlert(this->field_0x18)->levelStringLen + 0x100); // #Hack
#else
	dst = (char*)gSP_Manager.GetFreeBuffer(GetAlert(this->field_0x18)->levelStringLen + 0xb);
#endif
	iVar5 = this->field_0x18;

	if (GetAlert(iVar5) == (CompanionAlert*)0x0) {
		szLevelString = (char*)0x0;
	}
	else {
		if ((GetAlert(iVar5)->flags_0x2 & 2) == 0) {
			szLevelString = (char*)0x0;
		}
		else {
			szLevelString = CScene::ptable.g_LocalizationManager_00451678->GetLevelString(GetAlert(iVar5)->levelStringKey);
		}
	}

	iVar5 = this->field_0x18;

	// Clamp to the maximum length of the level string.
	if (GetAlert(iVar5)->levelStringLen < nbCharacters) {
		nbCharacters = GetAlert(iVar5)->levelStringLen;
	}

	if (nbCharacters < 1) {
		*dst = '\0';
	}
	else {
		iVar4 = 0;
		iVar5 = 0;
		do {
			if (szLevelString[iVar5] == '%') {
				do {
					iVar6 = iVar5;
					iVar5 = iVar6 + 1;
					cVar1 = szLevelString[iVar5];
					if (cVar1 == ']') break;
				} while (cVar1 != '%');

				if (cVar1 == ']') {
					iVar5 = iVar6 + 2;
				}
			}
			else {
				if (' ' < szLevelString[iVar5]) {
					iVar4 = iVar4 + 1;
				}
			}

			iVar5 = iVar5 + 1;
		} while ((iVar4 < nbCharacters) && (szLevelString[iVar5] != '\0'));

		edStrnCopy(dst, szLevelString, iVar5);

		local_8 = dst + iVar5;
		*local_8 = '\0';

		szLevelString = szLevelString + iVar5;
		edStrCat(local_8, "%[ALPHA75]k");
		local_8 = edStrReturnEndPtr(local_8);
		CopyNextFormattedChar(&local_8, &szLevelString);
		*local_8 = '\0';
		edStrCat(local_8, "%[ALPHA50]k");
		local_8 = edStrReturnEndPtr(local_8);
		CopyNextFormattedChar(&local_8, &szLevelString);
		*local_8 = '\0';
		edStrCat(local_8, "%[ALPHA25]k");
		local_8 = edStrReturnEndPtr(local_8);
		CopyNextFormattedChar(&local_8, &szLevelString);
		*local_8 = '\0';
		edStrCat(local_8, "%[ALPHA0]k");
		local_8 = edStrReturnEndPtr(local_8);
		edStrCat(local_8, szLevelString);

#ifdef PLATFORM_WIN
		const int maxLength = GetAlert(this->field_0x18)->levelStringLen + 0x100;
		const int dstLength = strlen(dst);
		assert(dstLength < maxLength);
#endif
	}

	textStyle.SetShadow(0);
	textStyle.SetHorizontalAlignment(2);
	textStyle.SetVerticalAlignment(0);
	uVar3 = 0;

	textStyle.SetFont(BootDataFont, false);
	textStyle.SetHorizontalSize((float)gVideoConfig.screenWidth * 0.83f);
	textStyle.SetVerticalSize((float)gVideoConfig.screenHeight * 0.8f);
	textStyle.SetEolAutomatic(0x80);
	textStyle.SetScale(0.95f, 0.95f);
	textStyle.field_0x8c = 0.0f;
	pNewFont = edTextStyleSetCurrent(&textStyle);

	edCTextFormat textFormat = edCTextFormat(&textStyle);
	textFormat.FormatString(dst, uVar3);
	rect.x = 0.0f;
	rect.y = 0.0f;
	rect.width = (float)gVideoConfig.screenWidth * 0.9f;
	rect.height = (float)gVideoConfig.screenHeight * 0.25f;
	textFormat.DisplayWindow(x * (float)gVideoConfig.screenWidth, y * (float)gVideoConfig.screenHeight, &rect);

	uVar3 = 0;
	CPauseManager::DrawRectangleBorder((float)gVideoConfig.screenWidth * 0.86f, (float)gVideoConfig.screenHeight * (((y + 0.02f) - 0.025f) + -0.04f),
		(float)gVideoConfig.screenWidth * 0.25f, (float)gVideoConfig.screenHeight * 0.1f, 12.0f, 12.0f, gBorderColorA, gBorderColorB, 0);

	textStyle.SetEolAutomatic(0);
	textStyle.SetHorizontalSize((float)gVideoConfig.screenWidth * 0.25f);
	textStyle.SetVerticalSize((float)gVideoConfig.screenHeight * 0.1f);
	textStyle.SetHorizontalAlignment(1);
	textStyle.SetVerticalAlignment(4);

	pText_00 = CScene::ptable.g_LocalizationManager_00451678->GetHelpString(0x5a594058414a0c10);
	textFormat.FormatString(pText_00, uVar3);
	textFormat.Display((float)gVideoConfig.screenWidth * 0.95f, (float)gVideoConfig.screenHeight * ((y - 0.025f) + 0.02f));
	edTextStyleSetCurrent(pNewFont);

	gSP_Manager.ReleaseBuffer(dst);
	gSP_Manager.ReleaseBuffer(pvVar2);

	return;
}

void CBehaviourCompanion::_Display_Text(float param_1, float param_2, float param_3)
{
	uint uVar1;
	edCTextStyle* pNewFont;
	CompanionAlert* pSubObj;
	undefined uVar2;
	int internalId;
	float fVar3;
	edCTextStyle textStyle;
	C3DFileManager* pManager;

	if (param_3 < 1.5f) {
		fVar3 = cosf(((1.0f - param_3 / 1.5f) * 6.283185f - 1.570796f));
	}
	else {
		fVar3 = cosf((((param_3 - 1.5f) / 1.5f) * 6.283185f - 1.570796f));
	}
	fVar3 = fabs(fVar3);

	uVar1 = (uint)_rgba_0042ca50.a;
	textStyle.Reset();
	textStyle.SetShadow(0);
	textStyle.SetHorizontalAlignment(2);
	textStyle.SetVerticalAlignment(8);
	uVar2 = 0;
	textStyle.SetFont(BootDataFont, false);
	textStyle.rgbaColour = (int)(fVar3 * 255.0f) & 0xffU | 0x80808000;
	textStyle.altColour = textStyle.rgbaColour;
	pNewFont = edTextStyleSetCurrent(&textStyle);
	edCTextFormat textFormat = edCTextFormat(&textStyle);
	textFormat.FormatString("%[CATCH]b", uVar2);
	textFormat.Display((param_1 + 0.075f) * (float)gVideoConfig.screenWidth, (param_2 + 0.1f) * (float)gVideoConfig.screenHeight);
	edTextStyleSetCurrent(pNewFont);
	pManager = CScene::ptable.g_C3DFileManager_00451664;
	edDListLoadIdentity();

	if ((GetAlert(this->field_0x18)->flags_0x0 & 8) == 0) {
		internalId = 0;
	}
	else {
		internalId = 2;
	}

	edDListUseMaterial(pManager->GetMaterialFromId(this->pOwner->field_0x360, internalId));

	edDListBegin(0.0f, 0.0f, 0.0f, 4, 4);
	edDListColor4u8(0x80, 0x80, 0x80, 0x80 - (char)(int)((float)uVar1 * fVar3));
	edDListTexCoo2f(0.0f, 0.0f);
	edDListVertex4f(param_1 * (float)gVideoConfig.screenWidth, param_2 * (float)gVideoConfig.screenHeight, 0.0f, 0.0f);
	edDListTexCoo2f(1.0f, 0.0f);
	edDListVertex4f((float)gVideoConfig.screenWidth * (param_1 + 0.15f), param_2 * (float)gVideoConfig.screenHeight, 0.0f, 0.0f);
	edDListTexCoo2f(0.0f, 1.0f);
	edDListVertex4f(param_1 * (float)gVideoConfig.screenWidth, (float)gVideoConfig.screenHeight * (param_2 + 0.2f), 0.0f, 0.0f);
	edDListTexCoo2f(1.0f, 1.0f);
	edDListVertex4f((float)gVideoConfig.screenWidth * (param_1 + 0.15f), (float)gVideoConfig.screenHeight * (param_2 + 0.2f), 0.0f, 0.0f);
	edDListEnd();

	if ((GetAlert(this->field_0x18)->flags_0x0 & 8) == 0) {
		edDListUseMaterial(pManager->GetMaterialFromId(this->pOwner->field_0x358, 0));
		edDListBegin(0.0f, 0.0f, 0.0f, 4, 4);
		edDListColor4u8(_rgba_0042ca50.r, _rgba_0042ca50.g, _rgba_0042ca50.b, _rgba_0042ca50.a - (char)(int)(fVar3 * (float)(uint)_rgba_0042ca50.a));
		edDListTexCoo2f(0.0f, 0.0f);
		edDListVertex4f((float)gVideoConfig.screenWidth * ((param_1 + 0.088f) - 0.02f), (float)gVideoConfig.screenHeight * ((param_2 + 0.192f) - 0.025f), 0.0f, 0.0f);
		edDListTexCoo2f(1.0f, 0.0f);
		edDListVertex4f((float)gVideoConfig.screenWidth * (param_1 + 0.088f + 0.02f), (float)gVideoConfig.screenHeight * ((param_2 + 0.192f) - 0.025f), 0.0f, 0.0f);
		edDListTexCoo2f(0.0f, 1.0f);
		edDListVertex4f((float)gVideoConfig.screenWidth * ((param_1 + 0.088f) - 0.02f), (float)gVideoConfig.screenHeight * (param_2 + 0.192f + 0.025f), 0.0f, 0.0f);
		edDListTexCoo2f(1.0f, 1.0f);
		edDListVertex4f((float)gVideoConfig.screenWidth * (param_1 + 0.088f + 0.02f), (float)gVideoConfig.screenHeight * (param_2 + 0.192f + 0.025f), 0.0f, 0.0f);
		edDListEnd();
	}

	return;
}


void CBehaviourCompanion::CopyNextFormattedChar(char** param_2, char** param_3)
{
	bool bVar1;
	char cVar2;
	char* pcVar3;
	char* pcVar4;
	char* pcVar5;
	char* pcVar6;

	pcVar6 = *param_3;
	bVar1 = false;
	pcVar4 = *param_2;
	do {
		cVar2 = *pcVar6;
		if (cVar2 == '%') {
			do {
				pcVar3 = pcVar4;
				pcVar5 = pcVar6;
				*pcVar3 = cVar2;
				pcVar6 = pcVar5 + 1;
				cVar2 = *pcVar6;
				pcVar4 = pcVar3 + 1;

				if (cVar2 == ']') break;
			} while (cVar2 != '%');

			if (cVar2 == ']') {
				*pcVar4 = ']';
				pcVar3[2] = pcVar5[2];
				pcVar6 = pcVar5 + 3;
				pcVar4 = pcVar3 + 3;
			}
		}
		else {
			if (cVar2 == ' ') {
				*pcVar4 = ' ';
				pcVar6 = pcVar6 + 1;
				pcVar4 = pcVar4 + 1;
			}
			else {
				bVar1 = true;
			}
		}

		if (bVar1) {
			*pcVar4 = *pcVar6;
			*param_3 = pcVar6 + 1;
			*param_2 = pcVar4 + 1;
			return;
		}
	} while (true);
}

CompanionAlert* CBehaviourCompanion::GetActiveAlert()
{
	CompanionAlert* pSubObj;

	if (this->activeSubObjId < 0) {
		pSubObj = (CompanionAlert*)0x0;
	}
	else {
		pSubObj = this->aSubObjs + this->activeSubObjId;
	}

	return pSubObj;
}

CompanionAlert* CBehaviourCompanion::GetAlert(int index)
{
	CompanionAlert* pSubObj = (CompanionAlert*)0x0;
	if (-1 < index) {
		pSubObj = this->aSubObjs + index;
	}

	return pSubObj;
}
