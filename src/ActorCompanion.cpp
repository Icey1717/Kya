#include "ActorCompanion.h"
#include "MemoryStream.h"
#include "LevelScheduleManager.h"
#include "CinematicManager.h"
#include "camera.h"
#include "TimeController.h"
#include "ActorHero.h"
#include "LocalizationManager.h"
#include "CameraViewManager.h"

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
	this->field_0x35c = pByteCode->GetS32();
	this->field_0x360 = pByteCode->GetS32();

	CActor::SV_InstallMaterialId(this->field_0x358);
	CActor::SV_InstallMaterialId(this->field_0x35c);
	CActor::SV_InstallMaterialId(this->field_0x360);

	return;
}

void CActorCompanion::Init()
{
	CActorAutonomous::Init();

	this->pAnimationController->RegisterBone(0x5579b496);

	this->field_0x354 = 2.259769f;
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
	CBehaviourCompanionSubObj_80* pCVar4;
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
			IMPLEMENTATION_GUARD(
			pCVar1 = this->pShadow;
			if (pCVar1 != (CShadow*)0x0) {
				(**(code**)((pCVar1->base).pVTable + 0x14))(pCVar1, 1);
			}
			iVar7 = this->actorState;
			if ((7 < iVar7) && (iVar7 < 0xb)) {
				iVar7 = (this->behaviourCompanion).activeSubObjId;
				if (iVar7 < 0) {
					pCVar4 = (CBehaviourCompanionSubObj_80*)0x0;
				}
				else {
					pCVar4 = (this->behaviourCompanion).aSubObjs + iVar7;
				}
				if (pCVar4 != (CBehaviourCompanionSubObj_80*)0x0) {
					FUN_001e82a0((int)this);
				}
			}
			bVar3 = GameDList_BeginCurrent();
			if (bVar3 != false) {
				peVar5 = CAnimation::GetCurBoneMatrix(this->pAnimationController, 0x5579b496);
				local_70.x = peVar5->da;
				local_70.y = peVar5->db;
				local_70.z = peVar5->dc;
				local_70.w = peVar5->dd;
				peVar5 = CAnimation::GetCurBoneMatrix(this->pAnimationController, 0x5579b496);
				edF32Matrix4MulF32Vector4Hard(&eStack144, peVar5, &gF32Vector4Zero);
				edF32Vector4AddHard(&local_70, &local_70, &eStack144);
				edF32Matrix4MulF32Vector4Hard(&local_70, (edF32MATRIX4*)this->pMeshTransform, &local_70);
				edF32Vector4SubHard(&eStack128, (edF32VECTOR4*)&(pCVar2->transformationMatrix).da, &local_70);
				edF32Vector4NormalizeHard(&eStack128, &eStack128);
				edF32Vector4ScaleHard(0.1, &eStack128, &eStack128);
				edF32Vector4AddHard(&local_70, &local_70, &eStack128);
				local_70.w = 1.0;
				edF32Matrix4CopyHard(&eStack96, &pCVar2->transMatrix_0x390);
				eStack96.da = local_70.x;
				eStack96.db = local_70.y;
				eStack96.dc = local_70.z;
				eStack96.dd = local_70.w;
				edDListLoadMatrix(&eStack96);
				peVar6 = C3DFileManager::GetMaterialFromId(pFileManager, this->field_0x358, 0);
				edDListUseMaterial(peVar6);
				edDListBegin(0.0, 0.0, 0.0, 8, 4);
				iVar7 = rand();
				uVar11 = (uint) * (byte*)((int)&this->field_0x354 + 3);
				uVar9 = (uint) * (byte*)&this->field_0x354;
				edDListColor4u8(*(byte*)&this->field_0x354, *(byte*)((int)&this->field_0x354 + 1),
					*(byte*)((int)&this->field_0x354 + 2),
					(byte)(int)((float)uVar11 * 0.5 + (float)uVar11 * ((float)iVar7 / 2.147484e+09) * 0.4));
				edDListTexCoo2f(0.0, 0.0);
				edDListVertex4f(-0.1, 0.1, 0.0, 0.0f);
				edDListTexCoo2f(1.0, 0.0);
				edDListVertex4f(0.1, 0.1, 0.0, 0.0f);
				edDListTexCoo2f(0.0, 1.0);
				edDListVertex4f(-0.1, -0.1, 0.0, 0.0f);
				edDListTexCoo2f(1.0, 1.0);
				edDListVertex4f(0.1, -0.1, 0.0, 0.0f);
				edDListEnd();
				if (this->actorState == 10) {
					iVar7 = (this->behaviourCompanion).activeSubObjId;
					pCVar4 = (CBehaviourCompanionSubObj_80*)0x0;
					if (-1 < iVar7) {
						pCVar4 = (this->behaviourCompanion).aSubObjs + iVar7;
					}
					if (pCVar4 != (CBehaviourCompanionSubObj_80*)0x0) {
						pCVar4 = (CBehaviourCompanionSubObj_80*)0x0;
						if (-1 < iVar7) {
							pCVar4 = (this->behaviourCompanion).aSubObjs + iVar7;
						}
						if (pCVar4->field_0x44 != 0) {
							pCVar4 = (CBehaviourCompanionSubObj_80*)0x0;
							if (-1 < iVar7) {
								pCVar4 = (this->behaviourCompanion).aSubObjs + iVar7;
							}
							eStack96.da = this->currentLocation.x;
							eStack96.dc = this->currentLocation.z;
							eStack96.dd = this->currentLocation.w;
							eStack96.db = this->currentLocation.y + -0.4;
							x = (float)-pCVar4->field_0x44 * 0.5;
							if (*(int*)&this->field_0x368 != 0) {
								edF32Vector4ScaleHard((float)&DAT_bf800000, (edF32VECTOR4*)&eStack96, (edF32VECTOR4*)&eStack96);
							}
							edDListLoadMatrix(&eStack96);
							uVar11 = 0;
							iVar7 = 0;
							while (true) {
								iVar8 = (this->behaviourCompanion).activeSubObjId;
								uVar9 = uRam00000044;
								if (-1 < iVar8) {
									uVar9 = (this->behaviourCompanion).aSubObjs[iVar8].field_0x44;
								}
								if (uVar9 <= uVar11) break;
								pCVar4 = (CBehaviourCompanionSubObj_80*)0x0;
								if (-1 < iVar8) {
									pCVar4 = (this->behaviourCompanion).aSubObjs + iVar8;
								}
								peVar6 = C3DFileManager::GetMaterialFromId
								(pFileManager, this->field_0x35c, (int)*(short*)((int)pCVar4->field_0x34 + iVar7));
								edDListUseMaterial(peVar6);
								edDListBegin(0.0, 0.0, 0.0, 8, 4);
								edDListColor4u8(0x80, 0x80, 0x80, 0x80);
								iVar8 = (this->behaviourCompanion).activeSubObjId;
								if (iVar8 < 0) {
									pCVar4 = (CBehaviourCompanionSubObj_80*)0x0;
								}
								else {
									pCVar4 = (this->behaviourCompanion).aSubObjs + iVar8;
								}
								uVar9 = 8;
								if (*(short*)((int)pCVar4->field_0x34 + iVar7) == 2) {
									iVar8 = CScene::GetManager(MO_Frontend);
									iVar8 = *(int*)(iVar8 + 0x6c);
									fVar12 = 0.0;
									uVar9 = CFrontEndInventory::ComputeGameScreenCoordinate
									((long)iVar8, (float*)&eStack16, &this->currentLocation, &fStack4, 0);
									uVar10 = CFrontEndInventory::ComputeGameScreenCoordinate
									((long)iVar8, (float*)&eStack32, &(this->behaviourCompanion).field_0x40, &fStack8, 0);
									if ((uVar9 & uVar10) != 0) {
										edF32Vector2Sub(&local_18, &eStack32, &eStack16);
										fVar12 = atan2f(local_18.x, local_18.y);
									}
									if (*(int*)&this->field_0x368 == 0) {
										fVar12 = edF32Between_2Pi(fVar12 - 1.570796);
									}
									else {
										fVar12 = edF32Between_2Pi(-fVar12 - 1.570796);
									}
									local_110.aa = (float)&DAT_bf000000;
									local_110.ab = (float)&DAT_bf000000;
									local_110.ac = 0.0;
									local_110.ad = 1.0;
									local_110.ba = 0.5;
									local_110.bb = (float)&DAT_bf000000;
									local_110.bc = 0.0;
									local_110.bd = 1.0;
									local_110.ca = (float)&DAT_bf000000;
									local_110.cb = 0.5;
									local_110.cc = 0.0;
									local_110.cd = 1.0;
									local_110.da = 0.5;
									local_110.db = 0.5;
									local_110.dc = 0.0;
									local_110.dd = 1.0;
									edF32Matrix4RotateZHard(fVar12, &local_d0, &gF32Matrix4Unit);
									peVar5 = &local_d0;
									local_d0.da = 0.5;
									local_d0.db = 0.5;
									edF32Matrix4MulF32Matrix4Hard(peVar5, &local_110, peVar5);
									edDListTexCoo2f(0.0, 0.0);
									edDListVertex4f(x + local_d0.aa, local_d0.ab, 0.0, 0.0f);
									edDListTexCoo2f(1.0, 0.0);
									edDListVertex4f(x + local_d0.ba, local_d0.bb, 0.0, 0.0f);
									edDListTexCoo2f(0.0, 1.0);
									edDListVertex4f(x + local_d0.ca, local_d0.cb, 0.0, 0.0f);
									edDListTexCoo2f(1.0, 1.0);
									edDListVertex4f(x + local_d0.da, local_d0.db, 0.0, 0.0f);
								}
								else {
									edDListTexCoo2f(0.0, 0.0);
									edDListVertex4f(x, 1.0, 0.0, 0.0f);
									edDListTexCoo2f(1.0, 0.0);
									fVar12 = x + 1.0;
									edDListVertex4f(fVar12, 1.0, 0.0, 0.0f);
									edDListTexCoo2f(0.0, 1.0);
									edDListVertex4f(x, 0.0, 0.0, 0.0f);
									edDListTexCoo2f(1.0, 1.0);
									edDListVertex4f(fVar12, 0.0, 0.0, 0.0f);
								}
								edDListEnd();
								iVar7 = iVar7 + 2;
								uVar11 = uVar11 + 1;
								x = x + 1.0;
							}
						}
					}
				}

				GameDList_EndCurrent();
			})
		}
	}

	return;
}

void CActorCompanion::Reset()
{
	int iVar3;
	int iVar4;

	CActorAutonomous::Reset();

	SetState(0xe, -1);

	iVar3 = CLevelScheduler::ScenVar_Get(3);
	iVar4 = 2;
	if (iVar3 == 0) {
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

	if (iVar3 == 0) {
		(this->behaviourCompanion).pOwner->pCollisionData->flags_0x0 = (this->behaviourCompanion).pOwner->pCollisionData->flags_0x0 & 0xfff7efff;
		(this->behaviourCompanion).pOwner->flags = (this->behaviourCompanion).pOwner->flags & 0xfffffbff;
	}
	else {
		(this->behaviourCompanion).pOwner->pCollisionData->flags_0x0 = (this->behaviourCompanion).pOwner->pCollisionData->flags_0x0 | 0x81000;
		(this->behaviourCompanion).pOwner->flags = (this->behaviourCompanion).pOwner->flags | 0x400;
	}

	this->behaviourCompanion.field_0x68 = 0;
	this->field_0x354 = 2.259769f;
	return;
}

void CActorCompanion::SaveContext(uint*, int)
{
	IMPLEMENTATION_GUARD();
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

void CBehaviourCompanion::Create(ByteCode* pByteCode)
{
	S_NTF_TARGET_STREAM_REF* pSVar1;
	S_STREAM_EVENT_CAMERA* pSVar2;
	int iVar3;
	int* pBase;
	CBehaviourCompanionSubObj_80* pCVar4;
	int uVar4;
	int iVar5;
	CCamera* pCVar6;
	int iVar7;
	ulong uVar8;
	CBehaviourCompanionSubObj_80* pCVar9;
	float fVar10;

	this->nbSubObjs = pByteCode->GetS32();
	this->aSubObjs = (CBehaviourCompanionSubObj_80*)0x0;
	uVar4 = this->nbSubObjs;
	if (0 < uVar4) {
		this->aSubObjs = new CBehaviourCompanionSubObj_80[uVar4];

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
			pCVar9->field_0x20 = pByteCode->GetF32();
			pCVar9->field_0x24 = pByteCode->GetF32();
			pCVar9->field_0x28 = pByteCode->GetF32();
			pCVar9->field_0x2c = 0;

			if (pCVar9->pActor.index != -1) {
				pCVar9->flags_0x2 = pCVar9->flags_0x2 | COMPANION_SUB_OBJ_FLAG_HAS_ACTOR;
			}

			if (fabs(pCVar9->field_0x28) + fabs(pCVar9->field_0x20) + fabs(pCVar9->field_0x24) != 0.0f) {
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

			pCVar9->field_0x48 = pByteCode->GetU64();

			pCVar9->field_0x50 = pByteCode->GetF32();
			pCVar9->field_0x50 = pCVar9->field_0x50 * pCVar9->field_0x50;

			if (pCVar9->field_0x48 != 0) {
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

			pSVar1 = (S_NTF_TARGET_STREAM_REF*)pByteCode->currentSeekPos;
			pByteCode->currentSeekPos = (char*)pSVar1->aEntries;
			if (pSVar1->entryCount != 0) {
				pByteCode->currentSeekPos = pByteCode->currentSeekPos + pSVar1->entryCount * 0x1c;
			}
			pCVar9->field_0x68 = pSVar1;

			pSVar2 = (S_STREAM_EVENT_CAMERA*)pByteCode->currentSeekPos;
			pByteCode->currentSeekPos = (char*)(pSVar2 + 1);
			pCVar9->pStreamEventCamera = pSVar2;

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
	CBehaviourCompanionSubObj_80* pCVar3;
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

			for (int i = 0; i < pCVar3->field_0x68->entryCount; i++) {
				pCVar3->field_0x68->aEntries[i].Init();
			}

			pCVar3->pStreamEventCamera->Init();

			for (int i = 0; i < pCVar3->field_0x68->entryCount; i++) {
				pCVar3->field_0x68->aEntries[i].Reset();
			}

			pCVar3->pStreamEventCamera->Reset(pOwner);

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
	CBehaviourCompanionSubObj_80* pCVar14;
	int iVar15;
	CBehaviourCompanionSubObj_80* iVar16;
	int iVar17;
	float fVar18;
	float fVar19;
	float fVar20;
	edF32VECTOR4 eStack48;
	edF32VECTOR4 eStack32;
	edF32VECTOR4 local_10;

	pCVar7 = CScene::ptable.g_EventManager_006f5080;

	if (this->pOwner->actorState == 0xd) {
		iVar8 = CLevelScheduler::ScenVar_Get(3);
		if (iVar8 == 0) {
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

	IMPLEMENTATION_GUARD(
	if (this->field_0x68 != 0) {
		CActorCompanion::BehaviourCompanion_Manage(this->pOwner);
	}
	if (this->field_0x5c != 0) {
		FUN_001e56b0(this, 1);
	}
	FUN_001e50a0(this);
	iVar8 = this->activeSubObjId;
	pCVar14 = (CBehaviourCompanionSubObj_80*)0x0;
	if (-1 < iVar8) {
		pCVar14 = this->aSubObjs + iVar8;
	}
	if (pCVar14 == (CBehaviourCompanionSubObj_80*)0x0) {
		iVar8 = this->field_0x10;
		pCVar14 = (CBehaviourCompanionSubObj_80*)0x0;
		if (-1 < iVar8) {
			pCVar14 = this->aSubObjs + iVar8;
		}
		if (pCVar14 == (CBehaviourCompanionSubObj_80*)0x0) {
			if (this->field_0x54 != 0) {
				this->activeSubObjId = -1;
				iVar8 = 0;
				if (0 < this->nbSubObjs) {
					iVar17 = 0;
					do {
						iVar16 = (CBehaviourCompanionSubObj_80*)((int)this->aSubObjs->field_0x34 + iVar17 + -0x34);
						bVar5 = false;
						if (((iVar16->flags_0x4 & 4) != 0) &&
							(((iVar16->flags_0x2 & 0x40) == 0 || (bVar5 = false, (((iVar16->pActor).pActor)->flags & 0x2000001) == 0)
								))) {
							uVar13 = CScenaricCondition::IsVerified(&iVar16->field_0x64);
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
							uVar10 = edEventComputeZoneAgainstVertex
							(pCVar7->activeChunkId, peVar9,
								&(this->pOwner->pActorHero->character).characterBase.base.base.currentLocation, 0);
							if ((uVar10 & 1 | (uint)(ushort)iVar16->field_0xc << 1) == 1) {
								iVar15 = (int)this->aSubObjs->field_0x34 + iVar17 + -0x34;
								bVar5 = false;
								if (((*(ushort*)(iVar15 + 4) & 4) != 0) &&
									(((*(ushort*)(iVar15 + 2) & 0x40) == 0 ||
										(bVar5 = false, (*(uint*)(*(int*)(iVar15 + 0x1c) + 8) & 0x2000001) == 0)))) {
									uVar13 = CScenaricCondition::IsVerified((CScenaricCondition*)(iVar15 + 100));
									bVar5 = false;
									if (uVar13 != 0) {
										bVar5 = true;
									}
								}
								if (bVar5) {
									this->field_0x1c = 0;
									this->field_0x20 = 0.0;
									this->field_0x10 = iVar8;
									this->aSubObjs[this->field_0x10].field_0x60 = this->aSubObjs[this->field_0x10].field_0x5c;
									pCVar14 = this->aSubObjs + this->field_0x10;
									if ((((pCVar14->flags_0x2 & 0x100) == 0) && ((pCVar14->flags_0x0 & 2) != 0)) &&
										((pCVar14->flags_0x4 & 1) == 0)) {
										FUN_001e56b0(this, 1);
									}
									iVar15 = this->pOwner->actorState;
									if ((iVar15 == 0xb) || (iVar15 == 0xe)) {
										pCVar3 = this->pOwner->pActorHero;
										local_10.x = (pCVar3->character).characterBase.base.base.currentLocation.x;
										local_10.z = (pCVar3->character).characterBase.base.base.currentLocation.z;
										local_10.w = (pCVar3->character).characterBase.base.base.currentLocation.w;
										local_10.y = (pCVar3->character).characterBase.base.base.currentLocation.y + 15.0;
										CActor::UpdatePosition((CActor*)this->pOwner, &local_10, true);
									}
									this->field_0x54 = 0;
								}
							}
							if ((iVar16->flags_0x2 & 0x10) != 0) {
								peVar9 = (ed_zone_3d*)0x0;
								if (iVar16->field_0x58 != 0xffffffff) {
									peVar9 = edEventGetChunkZone(pCVar7->activeChunkId, iVar16->field_0x58);
								}
								uVar11 = edEventComputeZoneAgainstVertex
								(pCVar7->activeChunkId, peVar9,
									&(this->pOwner->pActorHero->character).characterBase.base.base.currentLocation, 0);
								if ((uVar11 & 1) != 0) {
									iVar16->flags_0x4 = iVar16->flags_0x4 & 0xfffb;
								}
							}
							iVar16->field_0xc = (short)(uVar10 & 1);
						}
						iVar8 = iVar8 + 1;
						iVar17 = iVar17 + 0x80;
					} while (iVar8 < this->nbSubObjs);
				}
				pCVar6 = CCameraManager::_gThis;
				if (this->activeSubObjId < 0) {
					pCVar14 = (CBehaviourCompanionSubObj_80*)0x0;
				}
				else {
					pCVar14 = this->aSubObjs + this->activeSubObjId;
				}
				if (pCVar14 != (CBehaviourCompanionSubObj_80*)0x0) {
					pCVar1 = this->pOwner;
					if ((this->pOwner->flags & 0x4000) == 0) {
						edF32Vector4ScaleHard(-2.0, &eStack48, (edF32VECTOR4*)&(CCameraManager::_gThis->transformationMatrix).ca);
						edF32Vector4AddHard(&eStack32, (edF32VECTOR4*)&(pCVar6->transformationMatrix).da, &eStack48);
						eStack32.y = (pCVar1->pActorHero->character).characterBase.base.base.currentLocation.y + 2.5;
						CActor::UpdatePosition((CActor*)pCVar1, &eStack32, true);
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
			pCVar14 = (CBehaviourCompanionSubObj_80*)0x0;
			if (-1 < iVar8) {
				pCVar14 = this->aSubObjs + iVar8;
			}
			if ((pCVar14->flags_0x2 & 1) != 0) {
				peVar9 = (ed_zone_3d*)0x0;
				if (pCVar14->field_0x8 != 0xffffffff) {
					peVar9 = edEventGetChunkZone(pCVar7->activeChunkId, pCVar14->field_0x8);
				}
				uVar10 = edEventComputeZoneAgainstVertex
				(pCVar7->activeChunkId, peVar9,
					&(this->pOwner->pActorHero->character).characterBase.base.base.currentLocation, 0);
				if ((uVar10 & 1 | (uint)(ushort)pCVar14->field_0xc << 1) == 2) {
					this->field_0x10 = -1;
					this->field_0x54 = 1;
					this->pOwner->SetState(0xb, -1);
				}
				pCVar14->field_0xc = (short)(uVar10 & 1);
			}
		}
	}
	else {
		pCVar14 = (CBehaviourCompanionSubObj_80*)0x0;
		if (-1 < iVar8) {
			pCVar14 = this->aSubObjs + iVar8;
		}
		if ((pCVar14->flags_0x2 & 1) != 0) {
			peVar9 = (ed_zone_3d*)0x0;
			if (pCVar14->field_0x8 != 0xffffffff) {
				peVar9 = edEventGetChunkZone(pCVar7->activeChunkId, pCVar14->field_0x8);
			}
			uVar10 = edEventComputeZoneAgainstVertex
			(pCVar7->activeChunkId, peVar9,
				&(this->pOwner->pActorHero->character).characterBase.base.base.currentLocation, 0);
			if ((uVar10 & 1 | (uint)(ushort)pCVar14->field_0xc << 1) == 2) {
				FUN_001e5790(this);
				this->pOwner->SetState(0xb, -1);
			}
			pCVar14->field_0xc = (short)(uVar10 & 1);
		}
	}
	if (this->field_0x10 != -1) {
		if (this->field_0x5c != 0) {
			FUN_001e56b0(this, 1);
		}
		if ((this->aSubObjs[this->field_0x10].flags_0x2 & 0x100) != 0) {
			pTVar12 = GetTimer();
			this->aSubObjs[this->field_0x10].field_0x60 =
				this->aSubObjs[this->field_0x10].field_0x60 - pTVar12->cutsceneDeltaTime;
			if (0.0 < this->aSubObjs[this->field_0x10].field_0x60) goto LAB_001e7598;
		}
		FUN_001e58e0(this);
	}
LAB_001e7598:
	iVar8 = this->activeSubObjId;
	if (iVar8 != -1) {
		pCVar14 = (CBehaviourCompanionSubObj_80*)0x0;
		if (-1 < iVar8) {
			pCVar14 = this->aSubObjs + iVar8;
		}
		if ((pCVar14->flags_0x4 & 8) != 0) {
			if (iVar8 < 0) {
				pCVar14 = (CBehaviourCompanionSubObj_80*)0x0;
			}
			else {
				pCVar14 = this->aSubObjs + iVar8;
			}
			S_STREAM_EVENT_CAMERA::Manage(pCVar14->pStreamEventCamera, (CActor*)this->pOwner);
			iVar8 = this->activeSubObjId;
			pCVar14 = (CBehaviourCompanionSubObj_80*)0x0;
			if (-1 < iVar8) {
				pCVar14 = this->aSubObjs + iVar8;
			}
			if ((pCVar14->flags_0x2 & 0x40) != 0) {
				pCVar14 = (CBehaviourCompanionSubObj_80*)0x0;
				if (-1 < iVar8) {
					pCVar14 = this->aSubObjs + iVar8;
				}
				pCVar4 = (pCVar14->pActor).pActor;
				fVar20 = (pCVar4->currentLocation).y;
				fVar18 = (pCVar4->currentLocation).z;
				fVar19 = (pCVar4->currentLocation).w;
				(this->field_0x40).x = (pCVar4->currentLocation).x;
				(this->field_0x40).y = fVar20;
				(this->field_0x40).z = fVar18;
				(this->field_0x40).w = fVar19;
			}
		}
	})

	return;
}

void CBehaviourCompanion::Draw()
{
	int iVar1;
	bool bVar2;
	Timer* pTVar3;
	CBehaviourCompanionSubObj_80* pCVar4;
	CBehaviourCompanionSubObj_80* pCVar5;
	float fVar6;
	float fVar7;
	float fVar8;

	if (((GameFlags & 0x20) == 0) && (this->field_0x64 != 0)) {
		iVar1 = this->field_0x18;

		pCVar4 = (CBehaviourCompanionSubObj_80*)0x0;
		if (-1 < iVar1) {
			pCVar4 = this->aSubObjs + iVar1;
		}

		if (pCVar4 != (CBehaviourCompanionSubObj_80*)0x0) {
			pCVar4 = (CBehaviourCompanionSubObj_80*)0x0;
			if (-1 < iVar1) {
				pCVar4 = this->aSubObjs + iVar1;
			}

			if ((pCVar4->flags_0x2 & 2) != 0) {
				if (this->field_0x60 == 0) {
					pCVar4 = (CBehaviourCompanionSubObj_80*)0x0;
					if (-1 < iVar1) {
						pCVar4 = this->aSubObjs + iVar1;
					}

					if ((pCVar4->flags_0x4 & 2) != 0) {
						if (iVar1 < 0) {
							pCVar4 = (CBehaviourCompanionSubObj_80*)0x0;
						}
						else {
							pCVar4 = this->aSubObjs + iVar1;
						}

						if (this->activeSubObjId < 0) {
							pCVar5 = (CBehaviourCompanionSubObj_80*)0x0;
						}
						else {
							pCVar5 = this->aSubObjs + this->activeSubObjId;
						}

						if (pCVar5 == pCVar4) {
							IMPLEMENTATION_GUARD(
							this->field_0x20 = 0.0f;
							this->field_0x1c = 0.0f;
							pTVar3 = GetTimer();
							fVar6 = fmodf(pTVar3->scaledTotalTime, 3.0f);
							bVar2 = GuiDList_BeginCurrent();
							if (bVar2 != false) {
								_Display_Text(0.0f, (float)&DAT_3f333333, fVar6, this);
								GuiDList_EndCurrent();
							})
						}
					}
				}
				else {
					if (this->field_0x60 == 1) {
						pTVar3 = GetTimer();
						fVar6 = this->field_0x1c + pTVar3->cutsceneDeltaTime;
						this->field_0x1c = fVar6;

						if (0.5f < fVar6) {
							this->field_0x1c = 1.0f;
							pTVar3 = GetTimer();
							this->field_0x20 = this->field_0x20 + pTVar3->cutsceneDeltaTime;
						}

						if (this->field_0x18 < 0) {
							pCVar4 = (CBehaviourCompanionSubObj_80*)0x0;
						}
						else {
							pCVar4 = this->aSubObjs + this->field_0x18;
						}

						if ((pCVar4->flags_0x4 & 1) == 0) {
							fVar6 = -0.15f;
						}
						else {
							fVar6 = 0.0f;
							this->field_0x20 = 10000.0f;
						}
					}
					else {
						pTVar3 = GetTimer();
						fVar6 = this->field_0x1c - pTVar3->cutsceneDeltaTime;
						this->field_0x1c = fVar6;
						if (fVar6 < 0.0f) {
							this->field_0x20 = 0.0f;
							this->field_0x1c = 0.0f;
							this->field_0x60 = 0;
							pCVar4 = (CBehaviourCompanionSubObj_80*)0x0;
							if (-1 < this->field_0x18) {
								pCVar4 = this->aSubObjs + this->field_0x18;
							}
							pCVar4->flags_0x4 = pCVar4->flags_0x4 & 0xffdf;
						}
						fVar6 = -0.15f;
					}

					fVar7 = this->field_0x1c / 0.5f;
					if (fVar7 < 0.5f) {
						fVar8 = 1.1f;
						fVar6 = fVar6 + fVar7 * 2.0f * -fVar6;
					}
					else {
						fVar8 = (fVar7 - 0.5f) * 2.0f * -0.4f + 1.1f;
						fVar6 = 0.0f;
					}

					IMPLEMENTATION_GUARD(
					fVar7 = 0.54f;
					bVar2 = GuiDList_BeginCurrent();
					if (bVar2 != false) {
						DrawRectangle_001afe20
						(fVar7 * (float)gVideoConfig.screenWidth, (fVar8 + 0.125f) * (float)gVideoConfig.screenHeight,
							(float)gVideoConfig.screenWidth * 0.91f, (float)gVideoConfig.screenHeight * 0.3f, 12.0f, 12.0f,
							DAT_0042ca58, (uint)PTR_DAT_0042ca60, 0);
						FUN_001e5b90(fVar7, fVar8, this->field_0x20, this);
						_Display_Text(fVar6, (float)&DAT_3f333333, 3.0f, this);
						GuiDList_EndCurrent();
					})
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
	CBehaviourCompanionSubObj_80* iVar9;
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

	iVar4 = CLevelScheduler::ScenVar_Get(3);
	if (iVar4 == 0) {
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
				pcVar5 = CScene::ptable.g_LocalizationManager_00451678->GetLevelString(iVar9->field_0x48);
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
				this->aSubObjs[iVar4].field_0x54 = iVar12;
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
	CBehaviourCompanionSubObj_80* pCVar8;
	CBehaviourCompanionSubObj_80* pCVar7;
	edF32VECTOR4 local_10;

	if (msg == 0x87) {
		IMPLEMENTATION_GUARD(
		this->aSubObjs[pMsgParam].flags_0x4 = this->aSubObjs[pMsgParam].flags_0x4 & 0xfffb;
		if (pMsgParam == this->activeSubObjId) {
			FUN_001e5790(this);
		})
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
				this->field_0x20 = 0.0;
				this->field_0x10 = pMsgParam;
				this->aSubObjs[this->field_0x10].field_0x60 = this->aSubObjs[this->field_0x10].field_0x5c;
				pCVar8 = this->aSubObjs + this->field_0x10;
				if ((((pCVar8->flags_0x2 & 0x100) == 0) && ((pCVar8->flags_0x0 & 2) != 0)) && ((pCVar8->flags_0x4 & 1) == 0)) {
					FUN_001e56b0(this, 1);
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
				CLevelScheduler::ScenVar_Set(3, 1);

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
					CLevelScheduler::ScenVar_Set(3, 0);

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
