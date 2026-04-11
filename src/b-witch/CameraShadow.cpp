#include "CameraShadow.h"
#include "MathOps.h"
#include "DlistManager.h"
#include "ActorManager.h"
#include "ActorFactory.h"
#include "CameraViewManager.h"
#include "TimeController.h"

CCameraShadow::CCameraShadow(ByteCode* pByteCode)
{
	int iVar1;

	//this->field_0x16cc = 0;
	//this->field_0x16c4 = 0;
	this->patchRegister = GameDListPatch_Register(this, 0x2ee, 0);

	return;
}

bool CCameraShadow::InitDlistPatchable(int)
{
	CGlobalDListManager* pDlistmanager;
	int curVtxIndex;

	edDListLoadIdentity();
	edDListUseMaterial((edDList_material*)0x0);
	edDListSetProperty(4, this->sceneFlags);
	edDListBegin(0.0f, 0.0f, 0.0f, 3, 0x2ee);
	edDListColor4u8(0, 0, 0, 0x7f);
	curVtxIndex = 0;
	do {
		edDListVertex4f(0.0f, 0.0f, 0.0f, 0.0f);
		edDListVertex4f(0.0f, 0.0f, 0.0f, 0.0f);
		edDListVertex4f(0.0f, 0.0f, 0.0f, 0.0f);
		curVtxIndex = curVtxIndex + 1;
	} while (curVtxIndex < 0xfa);

	edDListEnd();
	edDListSetProperty(4, 0);

	pDlistmanager = reinterpret_cast<CGlobalDListManager*>(CScene::GetManager(MO_GlobalDListManager));
	pDlistmanager->SetActive(this->patchRegister, 1);

	return true;
}

// Should be in: D:/Projects/b-witch/CameraShadow.cpp
void CCameraShadow::Init()
{
	CCameraExt::Init();

	SetTarget((CActor*)0x0);
	this->field_0xd0 = 0;
	SetDistance(40.0f);
	SetAngleGamma(1.5708f);
	this->field_0x16e4 = 0.5735765f;
	this->field_0x16e0 = 0.8191521f;
	//this->field_0x1700 = 0;

	return;
}

void CCameraShadow::Reset()
{
	return;
}

ECameraType CCameraShadow::GetMode()
{
	EDFCAMERA_LOG(LogLevel::Info, "CCamera::GetMode Found CT_ShadowSun: 0x{:x}", (int)CT_ShadowSun);
	return CT_ShadowSun;
}

bool gCamShdDetectActorCallback(CActor* pActor, void* pParams)
{
	CShadow* pCVar1;
	CActor* pCVar2;
	float fVar3;
	edF32VECTOR4 eStack16;

	CCameraShadow* pCameraShadow = (CCameraShadow*)pParams;

	if ((((CActorFactory::gClassProperties[pActor->typeID].flags & 0x1000) != 0) &&
		(pCVar2 = pCameraShadow->GetTarget(), pActor != pCVar2)) && ((pActor->flags & 0x4000) != 0)) {
		pCVar1 = pActor->pShadow;
		pCVar2 = pCameraShadow->GetTarget();
		edF32Vector4SubHard(&eStack16, &pCVar2->currentLocation, &pActor->currentLocation);
		fVar3 = edF32Vector4GetDistHard(&eStack16);
		if (fVar3 < 4.0f) {
			if (pCVar1 != (CShadow*)0x0) {
				pCVar1->SetDisplayableAlt(0);
				pCVar1->SetGlobalIntensity(0.0f);
			}
		}
		else {
			if (pCVar1 != (CShadow*)0x0) {
				if (fVar3 <= 6.0f) {
					pCVar1->SetGlobalIntensity((fVar3 - 4.0f) / 2.0f);
				}
				else {
					pCVar1->SetGlobalIntensity(1.0f);
				}

				pCVar1->SetDisplayableAlt(1);
			}
		}

		if (fVar3 < 8.0f) {
			return true;
		}
	}

	return false;
}

bool CCameraShadow::Manage()
{
	CCollision* pCVar1;
	CActorManager* pCVar2;
	CLightManager* pCVar3;
	bool bVar4;
	CCameraManager* iVar5;
	CActor* pCVar6;
	BoneData* pBVar7;
	CActor* pCVar8;
	edF32MATRIX4* m0;
	CActorsTable* pCVar9;
	float fVar10;
	edF32MATRIX4 eStack400;
	edF32VECTOR4 local_150;
	edF32VECTOR4 local_140;
	edF32VECTOR4 eStack304;
	edF32VECTOR4 local_120;
	CActorsTable local_110;

	bVar4 = CCamera::Manage();

	pCVar3 = CScene::ptable.g_LightManager_004516b0;
	if (bVar4 == false) {
		local_110.nbEntries = 0;
		CScene::GetManager(MO_GlobalDListManager);
		iVar5 = (CCameraManager*)CScene::GetManager(MO_Camera);
		this->field_0x16dc = 0;
		if ((GetTarget() == (CActor*)0x0) || (this->field_0xd0 == 0)) {
			bVar4 = false;
		}
		else {
			pCVar2 = CScene::ptable.g_ActorManager_004516a4;
			if ((GetTarget() != (CActor*)0x0) && (iVar5->field_0xa00 == 0)) {
				local_140.w = 10.0f;
				local_140.xyz = (GetTarget()->currentLocation).xyz;
				pCVar2->cluster.GetActorsIntersectingSphereWithCriterion(&local_110, &local_140, gCamShdDetectActorCallback, this);
			}

			FUN_00190720();

			CCameraExt::ComputeTargetPosition(&local_120);

			if (GetTarget() != (CActor*)0x0) {
				if (GetTarget()->pCollisionData == (CCollision*)0x0) {
					if (GetTarget()->pAnimationController == (CAnimation*)0x0) {
						local_120.y = local_120.y + 1.0f;
					}
					else {
						pBVar7 = GetTarget()->pAnimationController->FindReggedBone(0x53538f8b);
						if (pBVar7 == (BoneData*)0x0) {
							local_120.y = local_120.y + 1.0f;
						}
						else {
							GetTarget()->SV_GetBoneWorldPosition(0x53538f8b, &local_120);
						}
					}
				}
				else {
					pCVar1 = GetTarget()->pCollisionData;
					local_120.x = (pCVar1->highestVertex).x;
					local_120.y = (pCVar1->highestVertex).y;
					local_120.z = (pCVar1->highestVertex).z;
					local_120.w = (pCVar1->highestVertex).w;
					local_120.y = local_120.y + (GetTarget()->pCollisionData->pObbPrim->position).y;
				}
			}
			if (0 < local_110.nbEntries) {
				int curActorIndex = 0;
				local_120.x = local_120.x * static_cast<float>(local_110.nbEntries);
				local_120.z = local_120.z * static_cast<float>(local_110.nbEntries);

				if (0 < local_110.nbEntries) {
					pCVar9 = &local_110;
					do {
						pCVar6 = pCVar9->aEntries[curActorIndex];
						if (pCVar6 != GetTarget()) {
							local_120.x = local_120.x + (pCVar6->currentLocation).x;
							local_120.z = local_120.z + (pCVar6->currentLocation).z;
						}

						curActorIndex = curActorIndex + 1;
					} while (curActorIndex < local_110.nbEntries);
				}

				local_120.x = local_120.x / (static_cast<float>(local_110.nbEntries) * 2.0f);
				local_120.z = local_120.z / (static_cast<float>(local_110.nbEntries) * 2.0f);
			}

			ComputeTargetPosition(&local_120);

			if (local_110.nbEntries == 0) {
				this->lookAt = local_120;
			}
			else {
				FUN_001912c0(&local_120);
			}

			local_150 = pCVar3->vector_0xf0;

			if (local_150.w != 0.0f) {
				local_150.xyz = gF32Vector4UnitY.xyz;
				local_150.z = -1.0f;
			}
			if (local_150.y < 0.0f) {
				if (-this->field_0x16e0 < local_150.y) {
					fVar10 = this->field_0x16e4 / sqrtf(local_150.z * local_150.z + local_150.x * local_150.x);
					local_150.x = local_150.x * fVar10;
					local_150.z = local_150.z * fVar10;
					local_150.y = -this->field_0x16e0;
				}
			}
			else {
				if (local_150.y < this->field_0x16e0) {
					fVar10 = this->field_0x16e4 / sqrtf(local_150.z * local_150.z + local_150.x * local_150.x);
					local_150.x = local_150.x * fVar10;
					local_150.z = local_150.z * fVar10;
					local_150.y = this->field_0x16e0;
				}
			}

			edF32Matrix4BuildFromVectorUnitSoft(&this->transformationMatrix, &local_150);
			fVar10 = GetAngleGamma();
			edF32Matrix4RotateZHard(fVar10, &eStack400, &gF32Matrix4Unit);
			m0 = &this->transformationMatrix;
			edF32Matrix4MulF32Matrix4Hard(m0, &eStack400, m0);
			fVar10 = GetDistance();
			edF32Vector4ScaleHard(fVar10, &eStack304, &local_150);
			edF32Vector4SubHard(&local_120, &this->lookAt, &eStack304);
			this->transformationMatrix.rowT = local_120;
			fVar10 = GetAngleXFromVector(&local_150);
			SetAngleAlpha(fVar10);
			fVar10 = GetAngleYFromVector(&local_150);
			SetAngleBeta(fVar10);
			bVar4 = true;
		}
	}
	else {
		bVar4 = false;
	}
	return bVar4;
}

float shadow_y_bias$12886 = 0.03f;

void CCameraShadow::Draw()
{
	int iVar1;
	edF32VECTOR4* peVar2;
	edF32VECTOR4* peVar3;
	float fVar4;
	float fVar5;
	float fVar6;
	float fVar7;
	float fVar8;
	float fVar9;
	bool bVar10;
	CGlobalDListPatch* pCVar11;
	uint uVar12;
	uint uVar13;
	int nbVerticesPatched;
	edVertex* peVar15;
	uint uVar16;
	edF32TRIANGLE4* pSVar17;
	edF32VECTOR4 local_70;
	edF32VECTOR4 local_60;
	edF32VECTOR3 local_50;
	edF32VECTOR4 local_40;
	edF32VECTOR4 local_30;

	if (this->field_0xd0 != 0) {
		CScene::GetManager(MO_GlobalDListManager);
		bVar10 = ComputeShadowTriangles();
		if (bVar10 == false) {
			pCVar11 = GameDListPatch_BeginCurrent(this->patchRegister);
			if (pCVar11 != (CGlobalDListPatch*)0x0) {
				GameDListPatch_EndCurrent(0, 0);
			}
		}
		else {
			edF32Vector4SubHard(&local_60, &this->lookAt, &this->transformationMatrix.rowT);
			edF32Vector4ScaleHard(shadow_y_bias$12886, &local_70, &gSceneCameras[this->sceneIndex].transformMatrix.rowZ);
			pCVar11 = GameDListPatch_BeginCurrent(this->patchRegister);
			if (pCVar11 != (CGlobalDListPatch*)0x0) {
				iVar1 = this->field_0x16c0;
				uVar13 = 0;
				nbVerticesPatched = 0;
				for (pSVar17 = this->field_0xd4; pSVar17 < this->field_0xd4 + iVar1; pSVar17 = pSVar17 + 1)
				{
					peVar2 = LOAD_POINTER_CAST(edF32VECTOR4*, pSVar17->p1);
					peVar3 = LOAD_POINTER_CAST(edF32VECTOR4*, pSVar17->p2);
					fVar4 = peVar3->x - peVar2->x;
					fVar6 = peVar3->y - peVar2->y;
					fVar8 = peVar3->z - peVar2->z;

					peVar2 = LOAD_POINTER_CAST(edF32VECTOR4*, pSVar17->p1);
					peVar3 = LOAD_POINTER_CAST(edF32VECTOR4*, pSVar17->p3);
					fVar5 = peVar3->x - peVar2->x;
					fVar7 = peVar3->y - peVar2->y;
					fVar9 = peVar3->z - peVar2->z;

					local_50.x = fVar6 * fVar9 - fVar7 * fVar8;
					local_50.y = fVar8 * fVar5 - fVar9 * fVar4;
					local_50.z = fVar4 * fVar7 - fVar5 * fVar6;

					if (local_50.x * local_60.x + local_50.y * local_60.y + local_50.z * local_60.z < 0.0f) {
						// 1
						peVar2 = LOAD_POINTER_CAST(edF32VECTOR4*, pSVar17->p1);
						fVar4 = local_70.x + peVar2->x;
						fVar5 = local_70.y + peVar2->y;
						fVar6 = local_70.z + peVar2->z;
						fVar7 = local_70.w + peVar2->w;

						edF32VECTOR3 xyz;
						xyz.x = fVar4;
						xyz.y = fVar5;
						xyz.z = fVar6;

						edDListPatchVertex_Inline(pCVar11->pCurrentPatch->pVertex, &xyz, &fVar7, uVar13 + 0);

						// 2
						peVar2 = LOAD_POINTER_CAST(edF32VECTOR4*, pSVar17->p2);
						fVar4 = local_70.x + peVar2->x;
						fVar5 = local_70.y + peVar2->y;
						fVar6 = local_70.z + peVar2->z;
						fVar7 = local_70.w + peVar2->w;

						xyz.x = fVar4;
						xyz.y = fVar5;
						xyz.z = fVar6;

						edDListPatchVertex_Inline(pCVar11->pCurrentPatch->pVertex, &xyz, &fVar7, uVar13 + 1);
						
						// 3
						peVar2 = LOAD_POINTER_CAST(edF32VECTOR4*, pSVar17->p3);

						fVar4 = local_70.x + peVar2->x;
						fVar5 = local_70.y + peVar2->y;
						fVar6 = local_70.z + peVar2->z;
						fVar7 = local_70.w + peVar2->w;

						xyz.x = fVar4;
						xyz.y = fVar5;
						xyz.z = fVar6;

						edDListPatchVertex_Inline(pCVar11->pCurrentPatch->pVertex, &xyz, &fVar7, uVar13 + 2);
						uVar13 = uVar13 + 3;
						
						nbVerticesPatched = nbVerticesPatched + 1;
					}
				}

				GameDListPatch_EndCurrent(nbVerticesPatched * 3, 0);
			}
		}
	}

	return;
}

void CCameraShadow::SetTarget(CActor* pActor)
{
	bool bVar1;
	CActor* pPrevTarget;
	ed_g3d_hierarchy* pHierarchy;
	CShadow* pShadow;

	if (pActor != GetTarget()) {
		if (GetTarget() != (CActor*)0x0) {
			GetTarget()->FUN_00115ea0(1);
			if (GetTarget()->pShadow != (CShadow*)0x0) {
				pPrevTarget = GetTarget();
				pPrevTarget->pShadow->SetDisplayableAlt(1);
			}

			if ((GetTarget()->pAnimationController != (CAnimation*)0x0) && (GetTarget()->pHier != (ed_g3d_hierarchy*)0x0)) {
				bVar1 = GetTarget()->pAnimationController->IsBonePresent(0x746f6f52);
				if (bVar1 != false) {
					GetTarget()->pAnimationController->UnRegisterBone(0x746f6f52);
				}

				bVar1 = GetTarget()->pAnimationController->IsBonePresent(0x53538f8b);
				if (bVar1 != false) {
					GetTarget()->pAnimationController->UnRegisterBone(0x53538f8b);
				}
			}
		}
		if (pActor != (CActor*)0x0) {
			pShadow = pActor->pShadow;
			if (pShadow != (CShadow*)0x0) {
				pShadow->SetDisplayableAlt(0);
			}

			if (pActor->pHier != (ed_g3d_hierarchy*)0x0) {
				pActor->FUN_00115ea0(this->sceneFlags);
				pHierarchy = pActor->pHier;
				if (1 < pHierarchy->lodCount) {
					pHierarchy->flags_0x9e = pHierarchy->flags_0x9e | 0x100;
				}

				if (pActor->pAnimationController != (CAnimation*)0x0) {
					bVar1 = pActor->pAnimationController->IsBonePresent(0x746f6f52);
					if (bVar1 != false) {
						pActor->pAnimationController->RegisterBone(0x746f6f52);
					}

					bVar1 = pActor->pAnimationController->IsBonePresent(0x53538f8b);
					if (bVar1 != false) {
						pActor->pAnimationController->RegisterBone(0x53538f8b);
					}
				}
			}
		}

		if (pActor != (CActor*)0x0) {
			this->lookAt = pActor->currentLocation;
		}

		this->pActorView = pActor;
	}

	return;
}

void CCameraShadow::ComputeTargetPosition(edF32VECTOR4* pOutPosition)
{
	CCameraManager* iVar1;
	CActor* pCVar2;
	float fVar3;
	float fVar4;
	float fVar5;
	edF32VECTOR4 eStack32;
	edF32VECTOR4 local_10;

	iVar1 = (CCameraManager*)CScene::GetManager(MO_Camera);
	if (iVar1->field_0xa00 == 1) {
		this->fov = 0.025f;
	}
	else {
		local_10 = CCameraManager::_gThis->transformationMatrix.rowT;
		fVar5 = CCameraManager::_gThis->fov_0xa34;
		edF32Vector4SubHard(&eStack32, &local_10, &GetTarget()->currentLocation);
		fVar3 = edF32Vector4GetDistHard(&eStack32);
		fVar3 = fVar3 - 1.0f;
		edF32Vector4SubHard(&eStack32, &GetTarget()->currentLocation, pOutPosition);
		fVar5 = (fVar3 / 5.0f) * (fVar5 / 0.8f);
		fVar4 = 1.0f - fVar5;
		eStack32.y = 0.0f;
		fVar3 = 1.0f;
		if ((fVar4 <= 1.0f) && (fVar3 = fVar4, fVar4 < 0.0f)) {
			fVar3 = 0.0f;
		}

		fVar4 = 1.0f;
		if (fVar3 <= 0.9f) {
			fVar4 = fVar3;
		}

		edF32Vector4ScaleHard(fVar4, &eStack32, &eStack32);
		edF32Vector4AddHard(pOutPosition, &eStack32, pOutPosition);
		fVar5 = fVar5 * 0.135f;
		if (0.135f < fVar5) {
			fVar3 = 0.135f;
		}
		else {
			fVar3 = 0.025f;
			if (0.025f <= fVar5) {
				fVar3 = fVar5;
			}
		}

		this->fov = fVar3;
	}

	return;
}

bool CCameraShadow::ComputeShadowTriangles()
{
	edColPRIM_OBJECT* peVar1;
	bool bVar2;
	BoneData* pBVar3;
	int iVar4;
	int iVar5;
	edObbBOX* peVar6;
	edF32MATRIX4* peVar7;
	edObbTREE_DYN** pStaticCollisionRefs;
	edF32VECTOR4* pVectorIt;
	edF32TRIANGLE4* pTriIt;
	int iVar11;
	edObbTREE_DYN** pStaticCollisionRefsEnd;
	float fVar13;
	float fVar14;
	float fVar15;
	float local_1cc;
	float fVar16;
	edCol_INFO_OBB_TRIS_BOX colInfoObbTrisBox;
	edF32VECTOR4 eStack192;
	edF32MATRIX4 local_b0;
	edF32VECTOR4 eStack112;
	edObbBOX bbox;
	int outCount;

	if (GetTarget() == (CActor*)0x0) {
		bVar2 = false;
	}
	else {
		bbox.transform = this->transformationMatrix;
		
		local_1cc = GetAngleBeta();
		edF32Matrix4RotateYHard(local_1cc, &local_b0, &gF32Matrix4Unit);
		bbox.transform = local_b0;

		if (GetTarget()->typeID == 1) {
			local_1cc = 0.0f;
		}
		else {
			local_1cc = GetTarget()->distanceToGround;
		}
		if (GetTarget()->pCollisionData == (CCollision*)0x0) {
			if (GetTarget()->pAnimationController != (CAnimation*)0x0) {
				pBVar3 = GetTarget()->pAnimationController->FindReggedBone(0x746f6f52);
				if (pBVar3 != (BoneData*)0x0) {
					GetTarget()->SV_GetBoneWorldPosition(0x746f6f52, &eStack192);
					local_1cc = local_1cc + (eStack192.y - (GetTarget()->currentLocation).y);
				}
			}

			local_1cc = local_1cc + 1.5f;
		}
		else {
			peVar1 = GetTarget()->pCollisionData->pObbPrim;
			local_1cc = local_1cc + (GetTarget()->pCollisionData->pObbPrim->position).y + (peVar1->scale).y;
		}

		fVar16 = GetTarget()->distanceToGround;
		if (GetTarget()->pCollisionData == (CCollision*)0x0) {
			fVar13 = 0.75f;
		}
		else {
			fVar13 = (GetTarget()->pCollisionData->pObbPrim->position).y;
		}

		fVar14 = GetAngleAlpha();
		fVar15 = cosf((1.570796f - fVar14) - 1.570796f);
		fVar14 = cosf(1.570796f - fVar14);
		edF32Vector4ScaleHard(-local_1cc * 0.5f, &eStack112, &bbox.transform.rowY);
		edF32Vector4AddHard(&bbox.transform.rowT, &this->lookAt, &eStack112);
		edF32Vector4ScaleHard((fVar16 + fVar13) * (fVar15 / fVar14), &eStack112, &bbox.transform.rowZ);
		pVectorIt = &bbox.transform.rowT;
		edF32Vector4AddHard(pVectorIt, pVectorIt, &eStack112);
		edF32Vector4AddHard(&eStack112, &bbox.transform.rowT, &eStack112);
		edF32Vector4SubHard(&eStack112, &eStack112, &this->transformationMatrix.rowT);
		fVar16 = edF32Vector4GetDistHard(&eStack112);
		pTriIt = this->field_0xd4;
		bbox.height = local_1cc * 0.5f;
		pVectorIt = this->field_0x1080;
		iVar11 = 0xfa;
		iVar5 = 100;
		bbox.depth = fVar16 * this->fov;
		pStaticCollisionRefs = (CScene::ptable.g_CollisionManager_00451690)->aStaticCollisionRefs;
		bbox.field_0x4c = 0.0f;
		bbox.width = bbox.depth / CCamera::_gpcam_man->aspectRatio;
		pStaticCollisionRefsEnd = pStaticCollisionRefs + (CScene::ptable.g_CollisionManager_00451690)->staticCollisionCount;
		for (; pStaticCollisionRefs < pStaticCollisionRefsEnd; pStaticCollisionRefs = pStaticCollisionRefs + 1) {
			if ((iVar11 != 0) && (iVar5 != 0)) {
				colInfoObbTrisBox.field_0x8 = 0;
				colInfoObbTrisBox.pTris = pTriIt;
				colInfoObbTrisBox.pVector = pVectorIt;
				colInfoObbTrisBox.field_0xc = iVar11;
				colInfoObbTrisBox.field_0x10 = iVar5;
				iVar4 = edObbIntersectObbtreeTrisBox(&colInfoObbTrisBox, *pStaticCollisionRefs, &bbox, &outCount, 0x80);
				iVar11 = iVar11 - iVar4;
				pTriIt = pTriIt + iVar4;
				iVar5 = iVar5 - outCount;
				pVectorIt = pVectorIt + outCount;
			}
		}

		this->field_0x16c0 = 0xfa - iVar11;
		this->field_0x16c8 = 100 - iVar5;

		if (this->field_0x16c4 < this->field_0x16c0) {
			this->field_0x16c4 = this->field_0x16c0;
		}

		if (this->field_0x16cc < this->field_0x16c8) {
			this->field_0x16cc = this->field_0x16c8;
		}

		bVar2 = 0 < this->field_0x16c0;
	}

	return bVar2;
}

void CCameraShadow::FUN_001912c0(edF32VECTOR4* param_2)
{
	undefined8 uVar1;
	undefined uVar2;
	CCameraManager* iVar1;
	CActor* pCVar3;
	float fVar5;
	float fVar6;
	float fVar7;
	edF32VECTOR4 local_40;
	edF32VECTOR4 eStack48;
	edF32VECTOR4 local_20;
	edF32VECTOR4 eStack16;

	iVar1 = static_cast<CCameraManager*>(CScene::GetManager(MO_Camera));
	if (iVar1->field_0xa00 == 1) {
		this->lookAt = *param_2;
	}
	else {
		local_20 = this->lookAt;

		local_40.x = 0.0f;
		local_40.y = 0.0f;
		local_40.z = 0.0f;
		local_40.w = 0.0f;

		pCVar3 = GetTarget();
		if ((pCVar3 == (CActor*)0x0) ||
			(pCVar3 = GetTarget(),
				pCVar3->pTiedActor == (CActor*)0x0)) {
			this->field_0x1700 = 0;
		}
		else {
			if (this->field_0x1700 == 0) {
				this->field_0x1700 = 1;
			}
			else {
				pCVar3 = GetTarget();
				edF32Vector4SubHard(&local_40, &pCVar3->pTiedActor->currentLocation, &this->field_0x16f0);
				edF32Vector4AddHard(&local_20, &local_20, &local_40);
			}

			pCVar3 = GetTarget();
			pCVar3 = pCVar3->pTiedActor;
			this->field_0x16f0 = pCVar3->currentLocation;
		}

		edF32Vector4SubHard(&eStack16, param_2, &local_20);
		fVar5 = edF32Vector4SafeNormalize0Hard(&eStack48, &eStack16);
		fVar6 = fVar5 * fVar5 + 3.0f;
		pCVar3 = GetTarget();
		if (pCVar3 != (CActor*)0x0) {
			pCVar3 = GetTarget();
			uVar2 = pCVar3->IsKindOfObject(2);
			if (uVar2 != 0) {
				pCVar3 = GetTarget();
				fVar6 = fVar6 + pCVar3[1].rotationQuat.y;
			}
		}

		if (fVar6 * GetTimer()->cutsceneDeltaTime < fVar5) {
			edF32Vector4ScaleHard(fVar6 * GetTimer()->cutsceneDeltaTime, &eStack48, &eStack48);
			edF32Vector4AddHard(&local_20, &local_20, &eStack48);
		}
		else {
			local_20.x = param_2->x;
			local_20.z = param_2->z;
			local_20.w = param_2->w;
		}

		fVar5 = param_2->y;
		this->lookAt.x = local_20.x;
		this->lookAt.y = fVar5;
		this->lookAt.z = local_20.z;
		this->lookAt.w = local_20.w;
	}

	return;
}

void CCameraShadow::FUN_00190720()
{
	ed_3d_hierarchy_node* peVar1;
	CCollision* pCVar2;
	CActorManager* pCVar3;
	edF32VECTOR4* v2;
	int iVar5;
	float fVar6;
	edF32VECTOR4 local_50;
	edF32VECTOR4 eStack64;
	edF32VECTOR4 eStack48;
	edF32VECTOR4 eStack32;
	edF32VECTOR4 local_10;
	CActor* pActor;

	pCVar3 = CScene::ptable.g_ActorManager_004516a4;
	if (((CScene::ptable.g_CameraManager_0045167c)->field_0xa00 == 0) &&
		(iVar5 = 0, 0 < (CScene::ptable.g_ActorManager_004516a4)->nbActiveActors)) {
		do {
			pActor = pCVar3->aActiveActors[iVar5];
			if (((pActor->pHier != (ed_g3d_hierarchy*)0x0) && ((pActor->actorFieldS & 0x80) == 0)) &&
				((CActorFactory::gClassProperties[pActor->typeID].flags & 0x2000) != 0)) {
				peVar1 = pActor->pMeshTransform;
				local_10 = peVar1->base.transformA.rowY;
				pCVar2 = pActor->pCollisionData;
				if (pCVar2 == (CCollision*)0x0) {
					local_50 = pActor->currentLocation;
				}
				else {
					local_50 = pCVar2->lowestVertex;
				}

				edF32Vector4SubHard(&eStack32, &this->lookAt, &this->transformationMatrix.rowT);
				v2 = pActor->GetBottomPosition();
				edF32Vector4SubHard(&eStack48, &this->lookAt, v2);
				edF32Vector4SubHard(&eStack64, &this->lookAt, &local_50);
				fVar6 = edF32Vector4DotProductHard(&eStack32, &local_10);
				if (0.0f < fVar6) {
					edF32Vector4GetNegHard(&local_10, &local_10);
				}

				fVar6 = edF32Vector4DotProductHard(&eStack48, &local_10);
				if ((0.0f <= fVar6) ||
					(fVar6 = edF32Vector4DotProductHard(&eStack64, &local_10), 0.0f <= fVar6)) {
					pActor->FUN_00115ea0(0xffff);
				}
				else {
					pActor->FUN_00115ea0(0);
				}
			}

			iVar5 = iVar5 + 1;
		} while (iVar5 < pCVar3->nbActiveActors);
	}

	return;
}