#include "CameraShadow.h"
#include "MathOps.h"
#include "DlistManager.h"
#include "ActorManager.h"
#include "ActorFactory.h"
#include "CollisionManager.h"
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
		uVar1 = *(undefined8*)&this->lookAt;
		local_20.z = this->lookAt.z;
		local_20.w = this->lookAt.w;
		local_20.x = static_cast<float>(uVar1);
		local_20.y = static_cast<float>((ulong)uVar1 >> 0x20);

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