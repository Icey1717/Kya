#include "CameraGame.h"
#include "MathOps.h"
#include "TimeController.h"
#include "ActorHeroPrivate.h"
#include "edVideo/CameraStack.h"
#include "ActorManager.h"
#include "ActorFactory.h"

#include <math.h>
#include "CollisionRay.h"
#include "CameraFightData.h"

int CCameraGame::_b_use_fig_data = 0;
CCamFigData* CCameraGame::_pfig_data = NULL;

struct CameraVectorBase {
	bool FUN_002bf570(CCameraGame* pCamera);
	void FUN_002bfc60();

	edF32VECTOR4 aCameraLocations[0x20];
	edF32VECTOR4 field_0x400;
	edF32VECTOR4 field_0x410;
	int cameraNum;
	float field_0x424;
	float field_0x428;
	uint flags;
};

CameraVectorBase g_CameraVectorBase;

bool CameraVectorBase::FUN_002bf570(CCameraGame* pCamera)
{
	edF32VECTOR4* peVar1;
	edF32VECTOR4* peVar2;
	int iVar3;
	float in_f0;
	float fVar4;
	float fVar5;
	float fVar6;
	edF32VECTOR4 local_120;
	edF32VECTOR4 local_110;
	edF32VECTOR4 local_e0;
	edF32VECTOR4 local_d0;
	edF32VECTOR4 local_a0;
	edF32VECTOR4 local_90;
	edF32VECTOR4 local_80;
	edF32VECTOR4 local_70;
	edF32VECTOR4 local_60;
	edF32VECTOR4 eStack80;
	edF32VECTOR4 eStack64;
	edF32VECTOR4 eStack48;
	edF32VECTOR4 local_20;
	edF32VECTOR4 local_10;

	if (1 < this->cameraNum) {
		edF32Vector4SubHard(&eStack48, this->aCameraLocations + 1, this->aCameraLocations);
		edF32Vector4SubHard(&eStack64, this->aCameraLocations + 2, this->aCameraLocations + 1);
		GetAnglesFromVector(&local_10.xyz, &eStack48);
		GetAnglesFromVector(&local_20.xyz, &eStack64);
		fVar4 = edF32GetAnglesDelta(local_10.x, local_20.x);
		fVar5 = edF32GetAnglesDelta(local_10.y, local_20.y);
		in_f0 = fVar5;
		if ((this->flags & 0x80) != 0) {
			in_f0 = pCamera->GetAngleAlpha();
			if (pCamera->cameraConfig.field_0x30 <= in_f0) {
				fVar5 = fVar5 * 20.0f;
				in_f0 = 20.0f;
			}
		}

		if (((this->flags & 0x100) != 0) && ((in_f0 = 0.03490658f, 0.03490658f < fVar4 || (in_f0 = pCamera->GetAngleAlpha(), in_f0 <= pCamera->cameraConfig.field_0x2c)))) {
			this->flags = this->flags & 0xfffffeff;
		}

		if (((this->flags & 0x80) != 0) && ((in_f0 = -0.03490658f, fVar4 < -0.03490658f || (in_f0 = pCamera->GetAngleAlpha(), pCamera->cameraConfig.field_0x30 <= in_f0)))) {
			this->flags = this->flags & 0xffffff7f;
		}

		if ((this->flags & 0x20) != 0) {
			in_f0 = 0.03490658f;

			if (0.03490658f < fVar5) {
				this->flags = this->flags & 0xffffffdf;
				in_f0 = 0.03490658f;
			}
		}

		if ((this->flags & 0x40) != 0) {
			in_f0 = -0.03490658f;

			if (fVar5 < -0.03490658f) {
				this->flags = this->flags & 0xffffffbf;
				in_f0 = -0.03490658f;
			}
		}
	}

	if (1 < this->cameraNum) {
		edF32Vector4SubHard(&local_a0, this->aCameraLocations + this->cameraNum, (edF32VECTOR4*)this);
		fVar4 = edF32Vector4NormalizeHard(&local_a0, &local_a0);
		CCollisionRay CStack192 = CCollisionRay(fVar4, (edF32VECTOR4*)this, &local_a0);
		fVar4 = CStack192.Intersect(RAY_FLAG_SCENERY, (CActor*)0x0, (CActor*)0x0, 0x40000004, &local_90, (_ray_info_out*)0x0);

		if (fVar4 == 1e+30f) {
			if (g_CameraVectorBase.cameraNum != 1) {
				g_CameraVectorBase.aCameraLocations[1] = g_CameraVectorBase.aCameraLocations[g_CameraVectorBase.cameraNum];

				g_CameraVectorBase.cameraNum = 1;
			}

			g_CameraVectorBase.flags = 0;
			g_CameraVectorBase.field_0x400 = g_CameraVectorBase.aCameraLocations[1];
		}
		else {
			edF32Vector4SubHard(&local_e0, this->aCameraLocations + 2, (edF32VECTOR4*)this);
			fVar4 = edF32Vector4NormalizeHard(&local_e0, &local_e0);
			CCollisionRay CStack256 = CCollisionRay(fVar4, this->aCameraLocations, &local_e0);
			fVar4 = CStack256.Intersect(RAY_FLAG_SCENERY, (CActor*)0x0, (CActor*)0x0, 0x40000004, &local_d0, (_ray_info_out*)0x0);

			if ((fVar4 == 1e+30f) || (this->flags == 0)) {
				for (iVar3 = 1; iVar3 < this->cameraNum; iVar3 = iVar3 + 1) {
					peVar2 = this->aCameraLocations + iVar3 + 1;
					peVar1 = this->aCameraLocations + iVar3;

					*peVar1 = *peVar2;
				}

				this->cameraNum = this->cameraNum + -1;

				if (this->cameraNum == 2) {
					FUN_002bfc60();
				}
			}
		}
	}

	edF32Vector4SubHard(&eStack80, this->aCameraLocations + this->cameraNum, this->aCameraLocations + this->cameraNum + -1);
	
	fVar4 = edF32Vector4DotProductHard(&eStack80, &eStack80);
	if (0.01f < fVar4) {
		iVar3 = this->cameraNum;
		edF32Vector4SubHard(&local_120, this->aCameraLocations + iVar3 + -1, this->aCameraLocations + iVar3);
		local_10.w = edF32Vector4NormalizeHard(&local_120, &local_120);

		CCollisionRay CStack320 = CCollisionRay(local_10.w, this->aCameraLocations + iVar3, &local_120);
		fVar4 = CStack320.Intersect(RAY_FLAG_SCENERY, (CActor*)0x0, (CActor*)0x0, 0x40000004, &local_110, (_ray_info_out*)0x0);
		local_70 = local_120;

		local_80 = local_110;

		if ((fVar4 != 1e+30f) && (0.1f < fabs(fVar4 - local_10.w))) {
			edF32Vector4ScaleHard(fVar4, &local_70, &local_70);
			edF32Vector4AddHard(&local_60, this->aCameraLocations + this->cameraNum, &local_70);
			edF32Vector4ScaleHard(0.05f, &local_80, &local_80);
			edF32Vector4AddHard(&local_60, &local_60, &local_80);
			pCamera->GetTarget();
			iVar3 = this->cameraNum;
			if (iVar3 < 0x3f) {
				peVar1 = this->aCameraLocations + iVar3;
				peVar2 = this->aCameraLocations + iVar3 + 1;
				*peVar2 = *peVar1;

				iVar3 = this->cameraNum;
				this->cameraNum = iVar3 + 1;
				peVar1 = this->aCameraLocations + iVar3;
				*peVar1 = local_60;

				if (this->cameraNum == 2) {
					FUN_002bfc60();
				}
			}
		}
	}

	return 1 < this->cameraNum;
}


void CameraVectorBase::FUN_002bfc60()
{
	float fVar1;
	edF32VECTOR4 eStack64;
	edF32VECTOR4 eStack48;
	edF32VECTOR4 local_20;
	edF32VECTOR4 local_10;

	if (this->cameraNum == 2) {
		edF32Vector4SubHard(&eStack48, this->aCameraLocations + 1, this->aCameraLocations + 0);
		edF32Vector4AddHard(&eStack64, this->aCameraLocations + 2, &this->field_0x410);
		edF32Vector4SubHard(&eStack64, &eStack64, this->aCameraLocations + 1);
		GetAnglesFromVector(&local_10.xyz, &eStack48);
		GetAnglesFromVector(&local_20.xyz, &eStack64);
		fVar1 = edF32GetAnglesDelta(local_10.x, local_20.x);
		this->field_0x428 = fVar1;
		fVar1 = edF32GetAnglesDelta(local_10.y, local_20.y);
		this->field_0x424 = fVar1;
		this->flags = 0;
		if (2.356194f < fabs(this->field_0x424)) {
			this->field_0x428 = -this->field_0x428;
		}
		if (0.1745329f < fabs(this->field_0x428)) {
			if (this->field_0x428 < 0.0) {
				this->flags = this->flags | 0x100;
			}
			else {
				this->flags = this->flags | 0x80;
			}
		}
		if (0.1745329f < fabs(this->field_0x424)) {
			if (this->field_0x424 < 0.0) {
				this->flags = this->flags | 0x20;
			}
			else {
				this->flags = this->flags | 0x40;
			}
		}
	}
	return;
}

CCameraGame::CCameraGame(ECameraType type, ByteCode* pMemoryStream)
	: CCameraExt(pMemoryStream)
{
	ECameraType EVar1;
	long lVar2;
	CAMERA_CONFIG* puVar3;

	this->actorsTable.nbEntries = 0;
	cameraConfig.Create(pMemoryStream);
	lVar2 = subObj_12.Init(pMemoryStream);

	puVar3 = &this->cameraConfig;
	if (lVar2 == 0) {
		puVar3->flags = puVar3->flags & 0xfffdffff;
	}
	else {
		puVar3->flags = puVar3->flags | 0x20000;
	}

	SetMode(type);

	EVar1 = GetMode();
	if (((EVar1 == CT_Main) && ((this->cameraConfig).field_0x2c < 1.22173f)) && (1.22173f < (this->cameraConfig).field_0x30)) {
		(this->cameraConfig).field_0x30 = 1.22173f;
	}

	flags_0xc = flags_0xc | 0x20000;
	return;
}

void CCameraGame::Reset()
{
	edF32VECTOR4* peVar1;
	CActor* pCVar2;
	edF32VECTOR4* peVar3;
	edF32VECTOR4* peVar5;
	float fVar6;
	float fVar7;
	float fVar8;
	float fVar9;
	edF32VECTOR4 local_70 = {};
	edF32VECTOR4 eStack96 = {};
	edF32MATRIX4 eStack80 = {};
	edF32VECTOR4 eStack16 = {};

	CCamera::_gpcam_man->KeepSameParam(this, 0x37);

	pCVar2 = GetTarget();
	if (pCVar2 != (CActor*)0x0) {
		UpdateTarget(&this->field_0x2a0, true);
		ComputeTargetPosition(&this->lookAt);
		UpdateTarget(&eStack16, true);
		peVar5 = &this->lookAt;
		edF32Vector4AddHard(peVar5, peVar5, &eStack16);
	
		this->field_0x1c0 = this->lookAt;

		this->field_0x1d0 = 0;
		this->field_0x1d4 = 0;
		this->field_0x1e0 = 0.0f;
		this->field_0x1d8 = 0.0f;
		this->field_0x1dc = 0.0f;
		this->field_0x1e4 = -1.0f;
		pCVar2 = GetTarget();
		fVar6 = GetAngleYFromVector(&pCVar2->rotationQuat);
		this->angleBeta_0x1e8 = fVar6;
	}

	fVar6 = 0.25f;
	this->gameLookAt = this->lookAt;

	fVar7 = this->cameraConfig.baseTargetPitch;
	this->pitchDyn.currentAlpha = fVar7;
	this->targetPitch = fVar7;

	this->field_0x204 = this->cameraConfig.field_0x38.z;

	fVar7 = (this->cameraConfig).field_0xc0;
	if (0.25f <= fVar7) {
		fVar6 = fVar7;
	}
	this->field_0x208 = fVar6;

	local_70.z = -this->field_0x208;
	edF32Matrix4RotateXHard(this->targetPitch, &eStack80, &gF32Matrix4Unit);
	edF32Matrix4RotateYHard(this->field_0x204, &eStack80, &eStack80);
	edF32Matrix4MulF32Vector4Hard(&local_70, &eStack80, &local_70);
	edF32Vector4AddHard(&eStack96, &this->lookAt, &local_70);
	edF32Vector4SubHard(&local_70, &this->lookAt, &eStack96);
	fVar6 = edF32Vector4GetDistHard(&local_70);
	this->distance = fVar6;
	fVar6 = GetAngleXFromVector(&local_70);
	this->angles.x = fVar6;
	fVar6 = GetAngleYFromVector(&local_70);
	this->angles.y = fVar6;
	SetAngleGamma(0.0f);
	this->field_0x2b4 = this->cameraConfig.field_0xc;
	this->field_0x234 = 0.0f;
	this->field_0x230 = 0.0f;
	this->field_0x22c = 0.0f;
	this->field_0x228 = 0.0f;
	this->field_0x224 = 0.0f;
	this->field_0x220 = 0.0f;

	this->field_0x260 = gF32Vertex4Zero;

	this->field_0x270 = gF32Vector4Zero;

	this->field_0x280 = 0;
	this->mode_0x1bc = 0;
	this->field_0x20c = 0.0f;
	this->field_0x214 = 2.0f;

	this->pitchDyn.Init(0.0f, this->cameraConfig.field_0x34);

	this->mode_0x2e4 = 3;
	this->field_0x2e8 = 0.0f;
	this->modeFlags = 0;
	this->flags_0x2e0 = (this->cameraConfig).flags_0x70;
	this->field_0x300 = gF32Vertex4Zero;
	this->field_0x330 = gF32Vertex4Zero;
	this->field_0x444 = 0.0f;
	this->field_0x2f0 = 0.0f;
	this->actorsTable.nbEntries = 0;

	switch (GetMode()) {
	case CT_Main:
		(this->field_0x450).x = 999.0f;
		(this->field_0x450).y = 0.0f;
		g_CameraVectorBase.cameraNum = 1;
		g_CameraVectorBase.field_0x424 = 0.0f;
		g_CameraVectorBase.field_0x428 = 0.0f;
		g_CameraVectorBase.flags = 0;
		g_CameraVectorBase.field_0x400 = gF32Vertex4Zero;
		g_CameraVectorBase.field_0x410 = gF32Vector4Zero;
		break;
	case CT_Fight:
		_b_use_fig_data = 0;
		break;
	case CT_KyaJamgut:
		this->field_0x460 = 0;
		this->field_0x450 = gF32Vertex4Zero;

		this->field_0x464 = 0.05f;
		this->field_0x468 = 0.5f;
		this->field_0x46c = 0.5f;
		(this->field_0x470).x = 65.0f;
		(this->field_0x470).y = 0.25f;
		(this->field_0x470).z = 0.125f;
		(this->field_0x470).w = 0.0625f;
		this->field_0x480 = 0.0375f;
	}

	return;
}

void CCameraGame::Init()
{
	float fVar1;
	bool bVar2;
	CActor* pCVar3;
	ECameraType EVar4;
	float fVar5;
	float fVar6;

	CCameraExt::Init();

	_pfig_data = (CCamFigData*)0x0;
	_b_use_fig_data = 0;
	this->flags_0xc = this->flags_0xc | 0x20000;
	this->cameraConfig.pActorRefA.Init();
	this->cameraConfig.pActorRefB.Init();
	InitFromConfig(&this->cameraConfig);

	if ((this->cameraConfig.flags & 0x18000) == 0) {
		this->field_0x2dc = CScene::_pinstance->field_0x118;
	}
	else {
		this->field_0x2dc = 0.0f;
		if ((this->cameraConfig.flags & 0x8000) != 0) {
			this->field_0x2dc = this->field_0x2dc + 50.0;
		}
		if ((this->cameraConfig.flags & 0x10000) != 0) {
			this->field_0x2dc = this->field_0x2dc + 100.0;
		}
	}

	Reset();

	fVar5 = this->field_0x50.y;
	fVar6 = 1.0f;
	if (fVar5 != 0.0f) {
		fVar6 = fVar5;
	}

	fVar5 = fVar6 * (0.965992f / 0.25857207f);
	this->field_0x23c = sqrtf(fVar6 * fVar6 + fVar5 * fVar5);
	this->field_0x244 = 1.0472f;
	this->pitchAlpha = 0.2617994f;
	this->field_0x24c = 0;
	this->field_0x240 = 0;
	pCVar3 = GetTarget();
	if (pCVar3 == (CActor*)0x0) {
		this->field_0xd0 = 0;
	}
	else {
		pCVar3 = GetTarget();
		bVar2 = pCVar3->IsKindOfObject(2);
		if (bVar2 == false) {
			pCVar3 = (CActor*)0x0;
		}
		else {
			pCVar3 = GetTarget();
		}
		this->field_0xd0 = reinterpret_cast<CActorHeroPrivate*>(pCVar3);
	}

	this->field_0x4a0 = 0;
	this->field_0x4a4 = 0xffffffff;
	EVar4 = GetMode();
	switch (EVar4) {
	case CT_Fight:
		this->flags_0xc = this->flags_0xc | 1;
		this->cameraConfig.flags = this->cameraConfig.flags | 0x6000;
		this->cameraConfig.flags = this->cameraConfig.flags & 0xfffffbbe;
		this->switchMode = SWITCH_MODE_F;
		this->field_0x98 = 0.8f;
		break;
	case CT_KyaWindWall:
		this->cameraConfig.flags = this->cameraConfig.flags | 0x4000;
		this->cameraConfig.field_0x58.y = 0.35f;
		break;
	case 6:
	case 8:
	case 9:
		this->cameraConfig.flags = this->cameraConfig.flags | 0x4000;
		break;
	case CT_KyaJamgut:
		this->switchMode = SWITCH_MODE_F;
		this->field_0x98 = 0.8f;
		this->field_0x460 = 0;
		this->field_0x450 = gF32Vertex4Zero;
	}
	this->field_0x1b8 = 0;
	return;
}

void CCameraGame::SetMode(ECameraType type)
{
	uint uVar1;
	bool bVar2;
	bool bVar3;
	bool bVar4;
	bool bVar5;
	bool bVar6;
	bool bVar7;
	bool bVar8;

	if (type == CT_MainCamera) {
		uVar1 = flags_0xc;
		bVar2 = (uVar1 & 0x40) != 0;
		bVar3 = (uVar1 & 4) != 0;
		bVar4 = (uVar1 & 8) != 0;
		bVar5 = (uVar1 & 0x10) != 0;
		bVar6 = (uVar1 & 0x20) != 0;
		bVar7 = (uVar1 & 0x100) != 0;
		bVar8 = (uVar1 & 0x200) != 0;
		if ((byte)(bVar3 + bVar2 + bVar4 + bVar5 + bVar6 + bVar7 + bVar8) < 2) {
			if (bVar3) {
				type = CT_Main;
			}
			else {
				if (bVar4) {
					type = CT_KyaJamgut;
				}
				else {
					if (bVar5) {
						type = (ECameraType)8;
					}
					else {
						if (bVar6) {
							type = (ECameraType)9;
						}
						else {
							if (bVar2) {
								type = CT_Fight;
							}
							else {
								if (bVar7) {
									type = CT_KyaWindWall;
								}
								else {
									type = (ECameraType)6;
									if (!bVar8) {
										type = CT_Main;
									}
								}
							}
						}
					}
				}
			}
		}
		else {
			type = CT_Main;
		}
	}

	CAMERA_LOG(LogLevel::Info, "CCamera::SetMode New Mode: 0x{:x}", (int)type);
	this->cameraType_0xd4 = type;
	return;
}

ECameraType CCameraGame::GetMode()
{
	CAMERA_LOG(LogLevel::Info, "CCamera::GetMode Found: 0x{:x}", (int)this->cameraType_0xd4);
	return this->cameraType_0xd4;
}

CSP_Manager gSP_Manager;

struct TargetCalc
{
	edF32VECTOR4 field_0x0;
	edF32VECTOR4 field_0x10;
	edF32VECTOR4 field_0x20;
	edF32VECTOR4 field_0x30;
	CActor* pActor;
	uint field_0x44;
	int field_0x48;
	float field_0x4c;
	float field_0x50;
	float field_0x54;
	float field_0x58;
};

float CCameraGame::_Manage_TargetPos(edF32VECTOR4* v0) 
{
	bool bVar1;
	TargetCalc* v1;
	CActorHeroPrivate* pCVar2;
	CActor* pCVar3;
	ECameraType EVar4;
	int iVar5;
	uint uVar6;
	long lVar7;
	float fVar8;
	float fVar9;
	float fVar10;

	v1 = (TargetCalc*)gSP_Manager.GetFreeBuffer(0x60);

	pCVar2 = static_cast<CActorHeroPrivate*>(GetTarget());

	if (GetTarget()->typeID == ACTOR_HERO_PRIVATE) {
		pCVar3 = GetTarget();
	}
	else {
		pCVar3 = (CActor*)0x0;
	}

	v1->pActor = pCVar3;
	v1->field_0x48 = this->field_0x1d0;
	v1->field_0x44 = 1;
	if (((this->cameraConfig).flags_0x70 & 0x2000000) != 0) {
		pCVar2 = this->field_0x1ec;
	}

	if (this->field_0x1e4 == -1.0f) {
		this->field_0x1e4 = pCVar2->distanceToGround;
	}
	
	v1->field_0x0 = this->lookAt;
	v1->field_0x10 = *v0;
	this->field_0x1c0 = v1->field_0x10;

	ComputeTargetPosition(&v1->field_0x30);

	if ((GetTarget()->pCollisionData != (CCollision*)0x0) && (GetTarget()->pCollisionData->pObbPrim != (edColPRIM_OBJECT*)0x0)) {
		(v1->field_0x30).y = (v1->field_0x30).y + (GetTarget()->pCollisionData->pObbPrim->field_0xb0).y;
	}

	pCVar3 = v1->pActor;
	if (pCVar3 == (CActor*)0x0) {
		v1->field_0x20 = v1->field_0x30;

		if ((GetTarget()->pCollisionData != (CCollision*)0x0) && (GetTarget()->pCollisionData->pObbPrim != (edColPRIM_OBJECT*)0x0)) {
			(v1->field_0x20).y = (v1->field_0x20).y + (GetTarget()->pCollisionData->pObbPrim->field_0x90).y;
		}
	}
	else {
		uint boneId = pCVar3->DoMessage(pCVar3, MESSAGE_GET_BONE_ID, (MSG_PARAM)5);
		BoneData* pBone = v1->pActor->pAnimationController->FindReggedBone(boneId);
		if (pBone == (BoneData*)0x0) {
			pCVar3 = v1->pActor;
			v1->field_0x20 = pCVar3->currentLocation;
			(v1->field_0x20).y = (v1->field_0x20).y + 1.39f;
		}
		else {
			v1->pActor->SV_GetBoneWorldPosition(boneId, &v1->field_0x20);
		}
	}

	v1->field_0x54 = CCamera::_gpcam_man->FUN_00197970(&v1->field_0x30);
	v1->field_0x58 = CCamera::_gpcam_man->FUN_00197970(&v1->field_0x20);

	if ((1.0f < v1->field_0x54) || (v1->field_0x58 < -1.0f)) {
		v1->field_0x48 = 1;
	}

	if ((this->cameraConfig.flags & 0x2000) != 0) {
		if (GetMode() == CT_Fight) {
			v1->field_0x44 = (uint)(pCVar2->distanceToGround < 0.1f);
		}
		else {
			v1->field_0x44 = (uint)((pCVar2->GetStateFlags(pCVar2->actorState) & 0x100) != 0);
		}

		if (v1->pActor != (CActor*)0x0) {
			CActorHero* pHero = reinterpret_cast<CActorHero*>(v1->pActor);
			uVar6 = pHero->TestState_IsGrippedOrClimbing(0xffffffff);
			v1->field_0x44 = v1->field_0x44 | (uint)(uVar6 != 0);
		}
	}

	if ((this->field_0x1d0 == 0) && (v1->field_0x44 == 0)) {
		fVar8 = pCVar2->distanceToGround;
		if ((0.4f <= fabs(this->field_0x1e4 - fVar8)) || (3.0f <= fVar8)) {
			v1->field_0x48 = 1;
		}
		else {
			fVar9 = (v1->field_0x0).y;
			(v1->field_0x0).y = fVar9 + (((this->field_0x1c0).y - fVar8) - fVar9) * 0.1;
			this->field_0x1e4 = pCVar2->distanceToGround;
		}
	}

	(v1->field_0x10).y = (v1->field_0x0).y;
	if ((this->cameraConfig.flags & 0x2000) != 0) {
		if ((((this->field_0x1d0 == 0) && (v1->field_0x44 != 0)) && (this->field_0x1d4 == 0)) &&
			(0.4f < fabs((v1->field_0x10).y - (this->field_0x1c0).y))) {
			v1->field_0x48 = 1;
		}
		this->field_0x1d4 = v1->field_0x44;
	}

	fVar8 = (this->cameraConfig).field_0x74;
	if (fVar8 < this->field_0x2b4) {
		fVar8 = fVar8 + CCamera::_gpcam_man->time_0x4 * 2.0f;
		(this->cameraConfig).field_0x74 = fVar8;
		if (this->field_0x2b4 < fVar8) {
			(this->cameraConfig).field_0x74 = this->field_0x2b4;
		}
	}
	else {
		fVar8 = fVar8 - CCamera::_gpcam_man->time_0x4 * 2.0f;
		(this->cameraConfig).field_0x74 = fVar8;
		if (fVar8 < this->field_0x2b4) {
			(this->cameraConfig).field_0x74 = this->field_0x2b4;
		}
	}

	fVar8 = this->field_0x2b0;
	if (fVar8 < 1.0f) {
		fVar8 = fVar8 + CCamera::_gpcam_man->time_0x4 * 2.0f;
		this->field_0x2b0 = fVar8;
		if (1.0f < fVar8) {
			this->field_0x2b0 = 1.0f;
		}
	}
	else {
		fVar8 = fVar8 - CCamera::_gpcam_man->time_0x4 * 2.0f;
		this->field_0x2b0 = fVar8;
		if (fVar8 < 1.0f) {
			this->field_0x2b0 = 1.0f;
		}
	}

	v1->field_0x50 = (this->cameraConfig).field_0x74;
	if (((this->cameraConfig).flags_0x70 & 0x400000) != 0) {
		v1->field_0x50 = 1.0f;
	}

	if (this->field_0x280 == 0) {
		this->field_0x2b0 = 0.0f;
		edF32Vector4LERPHard(v1->field_0x50, &v1->field_0x10, &v1->field_0x0, &v1->field_0x10);
		(v1->field_0x10).w = 1.0f;
	}
	else {
		(this->field_0x260).y = (v1->field_0x10).y;
		edF32Vector4LERPHard(v1->field_0x50, &v1->field_0x10, &v1->field_0x0, &v1->field_0x10);
		edF32Vector4LERPHard(this->field_0x2b0, &v1->field_0x10, &v1->field_0x10, &this->field_0x260);
		(v1->field_0x10).w = 1.0f;
	}

	v1->field_0x4c = (this->field_0x1c0).y - (v1->field_0x10).y;
	if (v1->field_0x48 == 0) {
		if (v1->field_0x44 != 0) {
			(v1->field_0x10).y = (v1->field_0x10).y + v1->field_0x4c * 0.1f;
		}
	}
	else {
		fVar8 = edFIntervalLERP(this->field_0x1e0, 0.0f, 2.0f, 0.0f, v1->field_0x4c);
		(v1->field_0x10).y = (v1->field_0x10).y + fVar8;
		this->field_0x1e0 = this->field_0x1e0 + CCamera::_gpcam_man->time_0x4;
		if ((v1->field_0x44 != 0) && ((2.0f < this->field_0x1e0 || (fabs(v1->field_0x4c) <= 0.01f)))) {
			this->field_0x1e0 = 0;
			v1->field_0x48 = 0;
			this->field_0x1e4 = pCVar2->distanceToGround;
		}
	}

	this->lookAt = v1->field_0x10;
	this->field_0x1d0 = v1->field_0x48;

	// HACK?
	float preventUseAfterFree = v1->field_0x4c;
	gSP_Manager.ReleaseBuffer(v1);

	return preventUseAfterFree;
}

void CCameraGame::_UpdateCameraData(edF32VECTOR4* translation)
{
	edF32VECTOR4* peVar1;
	ECameraType EVar2;
	edF32VECTOR4* peVar3;
	edF32VECTOR3* v0;
	edF32VECTOR4* peVar4;
	edF32MATRIX4* m0;
	float fVar5;
	edF32VECTOR4 local_130;
	edF32VECTOR4 eStack288;
	edF32MATRIX4 eStack272;
	edF32VECTOR4 local_d0;
	edF32VECTOR4 eStack192;
	edF32MATRIX4 eStack176;
	edF32VECTOR4 local_70;
	edF32VECTOR4 local_60;
	edF32MATRIX4 eStack80;
	edF32VECTOR4 local_10;

	EVar2 = GetMode();
	if (EVar2 == CT_KyaJamgut) {
		memset(&local_130, 0, sizeof(edF32VECTOR4));

		local_130.z = -this->field_0x208;

		edF32Matrix4RotateXHard(this->targetPitch, &eStack272, &gF32Matrix4Unit);
		edF32Matrix4RotateYHard(this->field_0x204, &eStack272, &eStack272);
		edF32Matrix4MulF32Vector4Hard(&local_130, &eStack272, &local_130);
		edF32Vector4AddHard(&eStack288, &this->gameLookAt, &local_130);
		edF32Vector4SubHard(&local_130, translation, &eStack288);
		fVar5 = edF32Vector4GetDistHard(&local_130);

		this->distance = fVar5;
		fVar5 = GetAngleXFromVector(&local_130);
		this->angles.x = fVar5;
		fVar5 = GetAngleYFromVector(&local_130);
		this->angles.y = fVar5;
		_Toboggan_UpdateCameraData();
	}
	else {
		if (((((EVar2 == CT_Fight) || (EVar2 == 9)) || (EVar2 == 8)) || ((EVar2 == CT_Camera_6 || (EVar2 == CT_KyaWindWall)))) || (EVar2 == CT_Main)) {
			memset(&local_d0, 0, sizeof(edF32VECTOR4));

			local_d0.z = -this->field_0x208;
			edF32Matrix4RotateXHard(this->targetPitch, &eStack176, &gF32Matrix4Unit);
			edF32Matrix4RotateYHard(this->field_0x204, &eStack176, &eStack176);

			CAMERA_LOG(LogLevel::Verbose, "CCameraGame::_UpdateCameraData: x: {} y: {} z:{}",
				this->targetPitch, this->field_0x204, this->field_0x208);

			edF32Matrix4MulF32Vector4Hard(&local_d0, &eStack176, &local_d0);
			edF32Vector4AddHard(&eStack192, &this->gameLookAt, &local_d0);
			edF32Vector4SubHard(&local_d0, translation, &eStack192);
			fVar5 = edF32Vector4GetDistHard(&local_d0);
			this->distance = fVar5;
			fVar5 = GetAngleXFromVector(&local_d0);
			this->angles.x = fVar5;
			fVar5 = GetAngleYFromVector(&local_d0);
			this->angles.y = fVar5;
			local_10.x = 0.0f;
			local_10.y = 0.0f;
			local_10.z = GetDistance();
			local_10.z = -local_10.z;
			local_10.w = 0.0f;
			v0 = GetAngles();
			edF32Matrix4FromEulerSoft(&this->transformationMatrix, v0, "ZXY");
			edF32Matrix4MulF32Vector4Hard(&local_10, &this->transformationMatrix, &local_10);
			edF32Vector4AddHard(&this->transformationMatrix.rowT, translation, &local_10);

			CAMERA_LOG(LogLevel::Verbose, "CCameraGame::_UpdateCameraData: Final location: {}, translation: {} other: {}",
				this->transformationMatrix.rowT.ToString(), translation->ToString(), local_10.ToString());
		}
	}

	if ((((this->flags_0xc & 0x37c) != 0x37c) && (this->pOtherTarget != (CActor*)0x0)) && (((this->cameraConfig).flags_0x70 & 0x2000000) == 0)) {
		FillThisFrameExpectedDifferentialMatrix(&eStack80);

		m0 = &this->transformationMatrix;

		if (((this->cameraConfig).flags & 0x4000) == 0) {
			peVar4 = &this->transformationMatrix.rowT;
			edF32Matrix4MulF32Vector4Hard(peVar4, &eStack80, peVar4);
			peVar4 = &this->lookAt;
			edF32Matrix4MulF32Vector4Hard(peVar4, &eStack80, peVar4);
		}
		else {
			edF32Matrix4MulF32Matrix4Hard(m0, m0, &eStack80);
			peVar4 = &this->lookAt;
			edF32Matrix4MulF32Vector4Hard(peVar4, &eStack80, peVar4);
		}

		local_60 = this->transformationMatrix.rowT;
		local_70 = this->transformationMatrix.rowZ;
		edF32Matrix4BuildFromVectorUnitSoft(&this->transformationMatrix, &local_70);
		this->transformationMatrix.rowT = local_60;
	}

	return;
}

void CCameraGame::_Toboggan_UpdateCameraData()
{
	CActor* pTarget;
	edF32MATRIX4* m0;
	float fVar2;
	edF32VECTOR4 eStack288;
	edF32VECTOR4 eStack272;
	edF32MATRIX4 eStack256;
	edF32MATRIX4 eStack192;
	edF32VECTOR4 eStack128;
	edF32VECTOR4 eStack112;
	edF32VECTOR4 eStack96;
	edF32VECTOR4 eStack80;
	edF32VECTOR4 eStack64;
	edF32VECTOR4 eStack48;
	edF32VECTOR4 local_20;
	edF32VECTOR3 local_10;

	pTarget = GetTarget();
	edF32Matrix4CopyHard(&eStack192, &pTarget->pMeshTransform->base.transformA);

	local_20.x = 0.0f;
	local_20.y = 0.0f;
	local_20.z = GetDistance();
	local_20.z = -local_20.z;
	local_20.w = 0.0f;

	edQuatFromMatrix4(&eStack48, &eStack192);
	edQuatNormalize(&eStack64, &eStack48);
	edQuatShortestSLERPAccurate((this->field_0x470).y, &eStack128, &this->field_0x450, &eStack64);
	edQuatShortestSLERPAccurate((this->field_0x470).z, &eStack112, &this->field_0x450, &eStack64);
	edQuatShortestSLERPAccurate((this->field_0x470).w, &eStack96, &this->field_0x450, &eStack64);
	edQuatShortestSLERPAccurate(this->field_0x480, &eStack80, &this->field_0x450, &eStack64);

	pTarget = GetTarget();
	if (pTarget->typeID == ACTOR_HERO_PRIVATE) {
		CActorHeroPrivate* pHero = reinterpret_cast<CActorHeroPrivate*>(GetTarget());
		fVar2 = pHero->field_0xa80;
	}
	else {
		fVar2 = this->field_0xd0->dynamic.linearAcceleration;
	}

	fVar2 = edFIntervalLERP(fVar2, this->field_0x46c, (this->field_0x470).x, this->field_0x464, this->field_0x468);
	this->field_0x460 = (uint)fVar2;

	edQuatShortestSLERPAccurate(fVar2, &eStack272, &eStack80, &eStack112);
	edQuatShortestSLERPAccurate(fVar2, &eStack288, &eStack96, &eStack128);
	edQuatShortestSLERPAccurate(fVar2 * 2.0f * (1.0f - fVar2), &this->field_0x450, &eStack272, &eStack288);
	edQuatNormalize(&this->field_0x450, &this->field_0x450);
	edQuatToMatrix4Hard(&this->field_0x450, &this->transformationMatrix);

	local_10.x = (this->cameraConfig).field_0x88;
	local_10.y = (this->cameraConfig).field_0x38.z;
	local_10.z = 0.0f;

	fVar2 = (this->cameraConfig).field_0x88;
	(this->cameraConfig).field_0x88 = fVar2 + ((this->cameraConfig).baseTargetPitch - fVar2) * 0.05f;

	edF32Matrix4FromEulerSoft(&eStack256, &local_10, "ZXY");
	m0 = &this->transformationMatrix;
	edF32Matrix4MulF32Matrix4Hard(m0, &eStack256, m0);
	edF32Matrix4MulF32Vector4Hard(&local_20, &this->transformationMatrix, &local_20);
	edF32Vector4AddHard(&this->transformationMatrix.rowT, &this->lookAt, &local_20);
	edF32Matrix4ToEulerSoft(&this->transformationMatrix, &this->angles, "ZXY");

	return;
}

void CCameraGame::_ResetDataForCut(int param_2)
{
	edF32VECTOR4* peVar1;
	edF32VECTOR4* peVar2;
	edF32VECTOR4* peVar3;
	float fVar4;
	float fVar5;
	edF32VECTOR4 local_80 = {};
	edF32VECTOR4 eStack112 = {};
	edF32VECTOR4 local_60 = {};
	edF32MATRIX4 eStack80 = {};
	edF32VECTOR4 eStack16 = {};

	(this->cameraConfig).field_0x74 = this->cameraConfig.field_0xc;
	this->field_0x2b4 = this->cameraConfig.field_0xc;
	(this->cameraConfig).field_0x78 = this->cameraConfig.field_0x10;
	(this->cameraConfig).field_0x7c = this->cameraConfig.field_0x14;
	this->field_0x1b0 = this->field_0x1b0 & 0xfffff9ff;
	(this->cameraConfig).field_0xc4 = 0;
	(this->cameraConfig).field_0xc8 = 0;
	(this->cameraConfig).field_0xcc = this->cameraConfig.field_0x6c;
	this->field_0x1b0 = this->field_0x1b0 & 0xfffffe7f;
	(this->cameraConfig).field_0x8c = 0.0f;
	(this->cameraConfig).field_0x90 = 0.0f;
	(this->cameraConfig).field_0x94 = this->cameraConfig.field_0x38.y;
	this->field_0x1b0 = this->field_0x1b0 & 0xffffff9f;
	(this->cameraConfig).field_0xa8 = 0.0f;
	(this->cameraConfig).field_0xac = 0.0f;
	(this->cameraConfig).field_0xb0 = this->cameraConfig.field_0x48.w;

	if (param_2 != 0) {
		this->field_0x1d0 = 0;
		this->field_0x1d4 = 1;
		this->field_0x1e0 = 0;
		this->field_0x1e4 = -1.0f;
		ComputeTargetPosition(&this->lookAt);
		UpdateTarget(&eStack16, true);
		peVar3 = &this->lookAt;
		edF32Vector4AddHard(peVar3, peVar3, &eStack16);
	}

	local_80.z = -this->distance;
	ComputeTargetPosition(&local_60);
	UpdateTarget(&eStack112, true);
	local_60.y = local_60.y + eStack112.y;
	this->gameLookAt = local_60;

	edF32Matrix4RotateXHard(this->angles.x, &eStack80, &gF32Matrix4Unit);
	edF32Matrix4RotateYHard(this->angles.y, &eStack80, &eStack80);
	edF32Matrix4MulF32Vector4Hard(&local_80, &eStack80, &local_80);
	edF32Vector4AddHard(&local_60, &this->lookAt, &local_80);
	edF32Vector4SubHard(&local_80, &this->gameLookAt, &local_60);

	fVar4 = edF32Vector4GetDistHard(&local_80);
	fVar5 = 0.25f;
	if (0.25f <= fVar4) {
		fVar5 = fVar4;
	}
	this->field_0x208 = fVar5;

	fVar5 = GetAngleXFromVector(&local_80);
	this->pitchDyn.currentAlpha = fVar5;
	this->targetPitch = fVar5;
	fVar5 = GetAngleYFromVector(&local_80);
	this->field_0x204 = fVar5;

	_Manage_TargetPos(&this->lookAt);
	_UpdateCameraData(&this->lookAt);

	return;
}

bool CCameraGame::AlertCamera(int alertType, int pParams, CCamera* pOtherCamera)
{
	bool bVar1;
	undefined uVar2;
	CBehaviour* pCVar6;
	int lVar8;
	uint* puVar9;
	float* peVar10;
	edF32VECTOR4* peVar11;
	float fVar12;
	float fVar13;
	float fVar14;
	edF32VECTOR4 eStack16;

	if (alertType == 5) {
		IMPLEMENTATION_GUARD(
			if (*(int*)pParams == 0) {
				this->field_0x2b4 = this->cameraConfig.field_0xc;
				(this->cameraConfig).field_0x78 = this->cameraConfig.field_0x10;
				(this->cameraConfig).field_0x7c = this->cameraConfig.field_0x14;
				(this->field_0x260).x = 0.0;
				(this->field_0x260).y = 0.0;
				(this->field_0x260).z = 0.0;
				(this->field_0x260).w = 1.0;
				(this->field_0x270).x = 0.0;
				(this->field_0x270).y = 0.0;
				(this->field_0x270).z = 0.0;
				(this->field_0x270).w = 0.0;
				puVar9 = &(this->cameraConfig).flags_0x70;
				if ((this->cameraConfig.flags & 2) == 0) {
					*puVar9 = *puVar9 & 0xffffffdf;
				}
				else {
					*puVar9 = *puVar9 | 0x20;
				}
				if ((this->cameraConfig.flags & 0x80) == 0) {
					*puVar9 = *puVar9 & 0xffffffbf;
				}
				else {
					*puVar9 = *puVar9 | 0x40;
				}
				*(undefined4*)&this->field_0x280 = 0;
			}
			else {
				this->field_0x2b4 = 0.0;
				(this->cameraConfig).field_0x78 = 0x3f800000;
				(this->cameraConfig).field_0x7c = 0;
				if (*(int*)&this->field_0x280 == 0) {
					fVar14 = *(float*)(pParams + 0x14);
					fVar12 = *(float*)(pParams + 0x18);
					fVar13 = *(float*)(pParams + 0x1c);
					(this->field_0x260).x = *(float*)(pParams + 0x10);
					(this->field_0x260).y = fVar14;
					(this->field_0x260).z = fVar12;
					(this->field_0x260).w = fVar13;
				}
				edF32Vector4CrossProductHard(&this->field_0x270, (edF32VECTOR4*)(pParams + 0x20), &g_xVector);
				edF32Vector4NormalizeHard(&this->field_0x270, &this->field_0x270);
				fVar12 = edF32Vector4DotProductHard
				(&this->field_0x270, (edF32VECTOR4*)&this->transformationMatrix.ca);
				if (fVar12 < 0.0) {
					edF32Vector4ScaleHard(-1.0f, &this->field_0x270, &this->field_0x270);
				}
				this->field_0x1b0 = this->field_0x1b0 & 0xfffff9ff;
				(this->cameraConfig).field_0xc4 = 0;
				(this->cameraConfig).field_0xc8 = 0;
				(this->cameraConfig).field_0xcc = this->cameraConfig.field_0x6c;
				this->field_0x1b0 = this->field_0x1b0 & 0xfffffe7f;
				(this->cameraConfig).field_0x8c = 0.0f;
				(this->cameraConfig).field_0x90 = 0.0f;
				(this->cameraConfig).field_0x94 = this->cameraConfig.field_0x38.y;
				this->field_0x1b0 = this->field_0x1b0 & 0xffffff9f;
				(this->cameraConfig).field_0xa8 = 0.0f;
				(this->cameraConfig).field_0xac = 0.0f;
				(this->cameraConfig).field_0xb0 = this->cameraConfig.field_0x48.w;
				*(undefined4*)&this->field_0x280 = 1;
			})
			goto LAB_002c1448;
	}

	if (alertType != 2) {
		if (alertType == 3) {
			if ((GetTarget() != (CActor*)0x0) && (GetTarget()->pMeshNode != (edNODE*)0x0)) {
				GetTarget()->ToggleMeshAlpha();
				GetTarget()->SetBFCulling(0);
			}
		}
		else {
			if (alertType == 1) {
				bVar1 = true;
				if (GetMode() == CT_Fight) {
					CCameraGame* pGameCamera = reinterpret_cast<CCameraGame*>(pOtherCamera);
					if ((((pOtherCamera != 0) && (lVar8 = pOtherCamera->IsKindOfObject(0x80), lVar8 != 0)) && ((pGameCamera->flags_0xc & 1) == 0)) &&
						((((pGameCamera->cameraConfig).flags & 0x2000) == 0 && ((pGameCamera->cameraConfig).baseTargetPitch == 0.25f)
							))) {
						bVar1 = false;
					}

					if ((bVar1) && ((pOtherCamera == 0 || (pOtherCamera->GetMode() != CT_Fight)))) {
						if (_pfig_data != (CCamFigData*)0x0) {
							_pfig_data->IsValid(1);
						}

						CCameraGame::_b_use_fig_data = 0;
					}
				}
			}
		}
		goto LAB_002c1448;
	}

	if ((GetTarget() != (CActor*)0x0) && (GetTarget()->typeID == ACTOR_HERO_PRIVATE)) {
		CActor* pHero = GetTarget();
		if (pHero->curBehaviourId == 8) {
			pCVar6 = pHero->GetBehaviour(pHero->curBehaviourId);
			IMPLEMENTATION_GUARD(
				*(CBehaviourVtable**)&this->field_0x1ec = pCVar6[0x23].pVTable;)
		}
		else {
			this->field_0x1ec = 0;
		}

		puVar9 = &(this->cameraConfig).flags_0x70;
		if (pHero->curBehaviourId == 8) {
			*puVar9 = *puVar9 | 0x2000000;
		}
		else {
			*puVar9 = *puVar9 & 0xfdffffff;
		}
	}

	peVar10 = &this->cameraConfig.baseTargetPitch;
	puVar9 = &(this->cameraConfig).flags_0x70;
	peVar11 = &this->cameraConfig.field_0x58;
	(this->cameraConfig).targetPitch = this->cameraConfig.baseTargetPitch;
	(this->cameraConfig).field_0xbc = this->cameraConfig.field_0x58.x;
	UpdateTarget(&this->field_0x2a0, false);

	this->mode_0x2e4 = 3;
	this->field_0x2e8 = 0;
	this->modeFlags = 0;

	this->flags_0x2e0 = *puVar9;

	this->field_0x300 = gF32Vertex4Zero;
	this->field_0x330 = gF32Vertex4Zero;
	this->field_0x444 = 0.0f;
	this->field_0x2f0 = 0;
	this->actorsTable.nbEntries = 0;

	if ((this->cameraConfig.flags & 0x20000) != 0) {
		this->cameraConfig.field_0x58.z = (this->subObj_12).field_0x20 + peVar11->x + 1.0;
	}

	this->angleBeta_0x1e8 = GetAngleYFromVector(&GetTarget()->rotationQuat);

	if (GetMode() == CT_Fight) {
		if (_pfig_data != (CCamFigData*)0x0) {
			_pfig_data->IsValid(2);
		}

		CCamera::_gpcam_man->KeepSameParam(this, 0x77);
		_ResetDataForCut(0);
	}
	else {
		ECameraType EVar4;

		if ((pOtherCamera == (CCamera*)0x0) ||
			((((EVar4 = pOtherCamera->GetMode(), EVar4 != CT_IntView &&
				(EVar4 = pOtherCamera->GetMode(), EVar4 != CT_SilverBoomy)) &&
				(EVar4 = pOtherCamera->GetMode(), EVar4 != 0x17)) &&
				((EVar4 = pOtherCamera->GetMode(), EVar4 != CT_KyaJamgut ||
					((*puVar9 & 0x2000000) != 0)))))) {
			UpdateTarget(&this->field_0x2a0, false);

			if (pParams == 1) {
				if (GetTarget() != (CActor*)0x0) {
					CCamera::_gpcam_man->KeepSameParam(this, 0x73);
					SetAngleAlpha(*peVar10);

					if (GetMode() != 9) {
						SetAngleBeta(this->angleBeta_0x1e8);
					}

					_ResetDataForCut(1);
				}
			}
			else {
				CCamera::_gpcam_man->KeepSameParam(this, 0x37);
				if (pOtherCamera != (CCamera*)0x0) {
					if (pOtherCamera->GetTarget() != (CActor*)0x0) {
						if (pOtherCamera->GetTarget() == GetTarget()) goto LAB_002c10a0;
					}

					ComputeTargetPosition(&this->lookAt);
					UpdateTarget(&eStack16, true);
					peVar11 = &this->lookAt;
					edF32Vector4AddHard(peVar11, peVar11, &eStack16);
					edF32Vector4SubHard(&eStack16, &this->lookAt, &this->transformationMatrix.rowT);
					fVar12 = edF32Vector4GetDistHard(&eStack16);
					SetDistance(fVar12);
					fVar12 = GetAngleXFromVector(&eStack16);
					SetAngleAlpha(fVar12);
					fVar12 = GetAngleYFromVector(&eStack16);
					SetAngleBeta(fVar12);
				}
			LAB_002c10a0:
				if (GetMode() == CT_KyaJamgut) {
					fVar12 = pOtherCamera->GetAngleAlpha();
					if (fVar12 < 0.1963495) {
						(this->cameraConfig).field_0x88 = 0.7853982f;
					}
					else {
						(this->cameraConfig).field_0x88 = *peVar10;
					}
				}

				if (GetTarget()->typeID == BOOMY) {
					SetAngleBeta((GetTarget()->rotationEuler).y);
				}
				_ResetDataForCut(0);
				if (((pOtherCamera != 0) && (fVar12 = pOtherCamera->GetAngleAlpha(), fVar12 < 0.1963495f)) &&
					((*puVar9 & 0x2000000) != 0)) {
					this->pitchDyn.currentAlpha = 0.1963495f;
					this->targetPitch = 0.1963495f;
				}
			}
		}
		else {
			CCamera::_gpcam_man->KeepSameParam(this, 0x27);
			SetAngleAlpha(*peVar10);
			SetDistance(peVar11->x);
			_ResetDataForCut(1);
		}
	}

	SetAngleGamma(0.0f);
	uVar2 = GetTarget()->IsKindOfObject(OBJ_TYPE_AUTONOMOUS);
	if (uVar2 == 0) {
		this->cameraConfig.flags = this->cameraConfig.flags & 0xffffdfff;
	}

	_TestRay_Sphere();

	LAB_002c1448:
	(this->cameraConfig).flags_0x70 = (this->cameraConfig).flags_0x70 & 0xfffffffe;
	CCamera::AlertCamera(alertType, pParams, pOtherCamera);
	return true;
}

static edF32VECTOR4 edF32VECTOR4_004250a0 = { 0.0f, 3.1415927f, 1.5707964f, -1.5707964f };

void CCameraGame::_TestRay_Sphere()
{
	bool bVar1;
	CActor* pActor;
	float* pfVar2;
	edF32VECTOR4* peVar3;
	int iVar4;
	float fVar5;
	float in_f21;
	float puVar6;
	float unaff_f20;
	edF32VECTOR4 local_50;
	edF32VECTOR4 local_20;
	edF32VECTOR4 eStack16;

	iVar4 = 0;
	bVar1 = false;
	peVar3 = &local_20;
	pfVar2 = &(this->cameraConfig).field_0xbc;
	local_20 = edF32VECTOR4_004250a0;

	while ((!bVar1 && (iVar4 < 4))) {
		unaff_f20 = edF32Between_2Pi(peVar3->x + this->field_0x204);
		CCollisionRay CStack64 = CCollisionRay(this->targetPitch, unaff_f20, *pfVar2 + 0.8f, &this->gameLookAt, &eStack16);
		edF32Vector4ScaleHard(-1.0f, &eStack16, &eStack16);
		pActor = this->GetTarget();
		fVar5 = CStack64.Intersect(RAY_FLAG_ACTOR_SCENERY, pActor, (CActor*)0x0, 0x40000004, (edF32VECTOR4*)0x0, (_ray_info_out*)0x0);
		if (fVar5 == 1e+30f) {
			bVar1 = true;
			in_f21 = *pfVar2 + 0.8f;
		}
		else {
			in_f21 = 0.8f;
			if (0.8f <= (fVar5 - 0.8f)) {
				in_f21 = (fVar5 - 0.8f);
			}

			if (1.0f < in_f21) {
				bVar1 = true;
			}
			else {
				peVar3 = (edF32VECTOR4*)&peVar3->y;
				iVar4 = iVar4 + 1;
			}
		}
	}

	if (iVar4 == 4) {
		unaff_f20 = this->field_0x204;
		in_f21 = 0.8f;
	}

	edF32Vector4ScaleHard(in_f21, &eStack16, &eStack16);
	edF32Vector4AddHard(&local_50, &this->gameLookAt, &eStack16);

	puVar6 = 0.25f;

	this->transformationMatrix.rowT = local_50;

	if (0.25f <= in_f21) {
		puVar6 = in_f21;
	}

	this->field_0x208 = puVar6;
	this->field_0x204 = unaff_f20;

	return;
}

void CCameraGame::UpdateMode()
{
	ECameraType typeMode;
	CActor* pCameraActorA;
	CActor* pCameraActorB;
	uint uVar2;
	float fVar3;
	float t0;
	edF32VECTOR4 localVector;

	typeMode = GetMode();
	if (typeMode == CT_Fight) {
		this->mode_0x1bc = 3;
	}
	else {
		if (((typeMode == 9) || (typeMode == 8)) || (typeMode == CT_KyaJamgut)) {
			this->mode_0x1bc = 2;
		}
		else {
			if (typeMode == CT_Main) {
				if (((((this->cameraConfig).flags_0x70 & 0x2000000) == 0) && ((this->modeFlags & 1) != 0)) &&
					(pCameraActorA = GetTarget(), pCameraActorA->typeID == ACTOR_HERO_PRIVATE)) {
					this->flags_0x2e0 = (this->cameraConfig).flags_0x70 & 0x60;
					(this->cameraConfig).flags_0x70 = (this->cameraConfig).flags_0x70 & 0xffffff9f;

					pCameraActorB = GetTarget();
					edF32Vector4SubHard(&localVector, &this->field_0x330, &pCameraActorB->currentLocation);
					t0 = this->angleBeta_0x1e8;
					fVar3 = GetAngleYFromVector(&localVector);
					fVar3 = edF32GetAnglesDelta(t0, fVar3);

					if ((this->actorsTable.nbEntries == 0) || (uVar2 = this->modeFlags & 0x60, uVar2 == 0x40)) {
						this->mode_0x2e4 = 4;
					}
					else {
						if (uVar2 == 0x20) {
							this->mode_0x2e4 = 3;
						}
						else {
							if (((this->cameraConfig).flags_0x70 & 0x800000) == 0) {
								this->field_0x2e8 = 0;
							}
							else {
								if (2.356194f <= fabs(fVar3)) {
									fVar3 = this->field_0x2e8 + CCamera::_gpcam_man->time_0x4;
									this->field_0x2e8 = fVar3;
									if ((this->subObj_12).field_0x0 < fVar3) {
										this->field_0x2e8 = 0;
										this->mode_0x2e4 = 4;
									}
								}
								else {
									if ((fabs(fVar3) <= 0.7853982f) &&
										(fVar3 = this->field_0x2e8 + CCamera::_gpcam_man->time_0x4, this->field_0x2e8 = fVar3,
											(this->subObj_12).field_0x0 < fVar3)) {
										this->field_0x2e8 = 0;
										this->mode_0x2e4 = 3;
									}
								}
							}
						}
					}

					this->mode_0x1bc = this->mode_0x2e4;
				}
				else {
					this->mode_0x1bc = 1;
				}
			}
			else {
				this->mode_0x1bc = 1;
			}
		}
	}

	return;
}

struct GameCheckStruct {
	CActorHeroPrivate* gCameraGameHero;
	float FLOAT_004495a4;
};

GameCheckStruct gGameCheckStruct;

void CCameraGame::FUN_00199c20(edF32VECTOR4* param_2, edF32VECTOR4* param_3)
{
	CActor* pCVar1;
	float fVar2;
	float fVar3;
	float fVar4;
	edF32MATRIX4 local_40;

	pCVar1 = this->pOtherTarget;

	if (pCVar1 == (CActor*)0x0) {
		*param_2 = *param_3;
	}
	else {
		if (pCVar1 == (CActor*)0x0) {
			local_40 = gF32Matrix4Unit;
		}
		else {
			pCVar1->FillThisFrameExpectedDifferentialMatrix(&local_40);
		}

		edF32Matrix4MulF32Vector4Hard(param_2, &local_40, param_3);
	}
	return;
}

bool FUN_002bf4b0(CActor* pActor, void* pData)
{
	GameCheckStruct* pGameCheckStruct = reinterpret_cast<GameCheckStruct*>(pData);

	CActor* pCVar1;
	float fVar2;
	float fVar3;
	float fVar4;
	bool bPass;

	if ((pActor->Can_0x9c() == 0) ||
		(pCVar1 = pGameCheckStruct->gCameraGameHero, fVar2 = (pCVar1->currentLocation).x - (pActor->currentLocation).x,
			fVar3 = (pCVar1->currentLocation).y - (pActor->currentLocation).y,
			fVar4 = (pCVar1->currentLocation).z - (pActor->currentLocation).z,
			pGameCheckStruct->FLOAT_004495a4 * pGameCheckStruct->FLOAT_004495a4 < fVar2 * fVar2 + fVar3 * fVar3 + fVar4 * fVar4)) {
		bPass = false;
	}
	else {
		bPass = true;
	}

	return bPass;
}

void CCameraGame::FUN_002bff30(edF32VECTOR4* v0, edF32VECTOR4* v1, edF32VECTOR4* v2)
{
	cam_temp_struct* ppeVar2;
	float fVar3;
	float peVar4;
	float fVar5;
	float fVar6;
	float fVar7;
	float puVar8;
	edF32VECTOR4 eStack80;
	edF32VECTOR4 eStack64;
	edF32VECTOR4 local_30;
	cam_temp_struct local_20;

	ppeVar2 = (cam_temp_struct*)0x0;
	if (GetMode() == CT_Fight) {
		if (_pfig_data != (CCamFigData*)0x0) {
			ppeVar2 = _pfig_data->Cam_Update(v1, _b_use_fig_data);
		}
	}
	else {
		fVar3 = edFIntervalLERP(this->field_0x444, (this->subObj_12).field_0x4, (this->subObj_12).field_0x8, 0.0f, 1.0f);
		if (this->mode_0x1bc == 3) {
			this->field_0x300 = this->field_0x330;
			(this->field_0x300).y = v1->y + v2->y;
		}
		else {
			edF32Vector4LERPHard(0.1f, &this->field_0x300, &this->field_0x330, v1);
			(this->field_0x300).y = v1->y + v2->y;
			(this->field_0x300).w = 1.0f;
		}

		local_20.pVector = &this->field_0x300;
		fVar7 = 0.0f;
		fVar5 = 0.0f;
		if ((this->mode_0x1bc == 3) && ((this->modeFlags & 2) == 0)) {
			UpdateTarget(&local_30, false);
			local_30.z = 0.0f;
			local_30.x = 0.0f;
			edF32Vector4AddHard(&local_30, &local_30, &GetTarget()->currentLocation);
			edF32Vector4SubHard(&local_30, &this->field_0x320, &local_30);
			local_30.y = 0.0f;

			if (0.25f <= edF32Vector4GetDistHard(&local_30)) {
				fVar5 = (this->subObj_12).field_0x14;
				fVar6 = this->field_0x204;
				fVar5 = fVar5 + fVar3 * ((this->subObj_12).field_0x18 - fVar5);
				fVar7 = GetAngleYFromVector(&local_30);
				fVar7 = edF32Between_0_2Pi((this->subObj_12).field_0x10 + fVar7);
				fVar7 = edF32GetAnglesDelta(fVar6, fVar7);
			}
		}

		local_20.field_0xc = edF32Between_0_2Pi(this->field_0x204 + fVar7 * fVar5 * CCamera::_gpcam_man->time_0x4);
		local_20.field_0x8 = this->targetPitch;
		fVar5 = (this->subObj_12).field_0x1c;

		local_20.field_0x10 = (((fVar5 + fVar3 * ((this->subObj_12).field_0x20 - fVar5) + (this->cameraConfig).field_0xbc) - this->field_0x208) + this->field_0x208);

		CCollisionRay CStack112 = CCollisionRay(local_20.field_0x8, local_20.field_0xc, local_20.field_0x10 + 0.8f, local_20.pVector, &eStack64);
		edF32Vector4ScaleHard(-1.0f, &eStack64, &eStack64);
		fVar3 = CStack112.Intersect(this->cameraRayFlags, GetTarget(), (CActor*)0x0, 0x40000004, &eStack80, (_ray_info_out*)0x0);

		puVar8 = local_20.field_0x10;
		if ((fVar3 != 1e+30f) && (fVar5 = edF32Vector4DotProductHard(&eStack80, &eStack64), puVar8 = local_20.field_0x10, fVar5 < 0.0f)) {
			puVar8 = 0.8f;
			if (0.8f <= (fVar3 - 0.8f)) {
				puVar8 = (fVar3 - 0.8f);
			}
		}

		if (puVar8 != local_20.field_0x10) {
			local_20.field_0x10 = puVar8;
		}

		local_20.pVector = &this->field_0x300;
		ppeVar2 = &local_20;
	}

	if (ppeVar2 == (cam_temp_struct*)0x0) {
		*v0 = *v1;
	}
	else {
		peVar4 = 0.25f;
		if (0.25f <= ppeVar2->field_0x10) {
			peVar4 = ppeVar2->field_0x10;
		}
		this->field_0x208 = peVar4;
		peVar4 = ppeVar2->field_0x8;
		(this->pitchDyn).currentAlpha = peVar4;
		this->targetPitch = peVar4;
		this->field_0x204 = ppeVar2->field_0xc;
		*v0 = *ppeVar2->pVector;
	}

	this->gameLookAt = *v0;

	return;
}

void CCameraGame::FUN_002bee80()
{
	CActor* pCurEntry;
	CActor* pCVar2;
	CActor* pCVar3;
	int iVar4;
	long lVar5;
	int iVar6;
	float fVar7;
	float fVar8;
	float fVar9;
	float fVar10;
	float fVar11;
	float fVar12;
	edF32VECTOR4 local_1a0;
	edF32VECTOR4 eStack400;
	edF32VECTOR4 local_180;
	edF32VECTOR4 local_170;
	edF32VECTOR4 eStack352;
	edF32VECTOR4 eStack336;
	edF32VECTOR4 eStack320;
	edF32VECTOR4 local_130;
	CActorsTable intersectingActors;
	edF32VECTOR4 intersectionSphere;

	if ((((this->cameraConfig).flags & 0x20000) == 0) || (((this->cameraConfig).flags_0x70 & 0x2000000) != 0)) {
		this->modeFlags = 0;
	}
	else {
		intersectingActors.nbEntries = 0;
		gGameCheckStruct.gCameraGameHero = (CActorHeroPrivate*)GetTarget();
		gGameCheckStruct.FLOAT_004495a4 = this->field_0x2dc;
		local_130.xyz = (GetTarget()->sphereCentre).xyz;
		local_130.w = 1.0f;

		CScene::GetManager(MO_Camera);

		intersectionSphere.w = this->field_0x2dc;
		intersectionSphere.xyz = gGameCheckStruct.gCameraGameHero->sphereCentre.xyz;

		(CScene::ptable.g_ActorManager_004516a4)->cluster.GetActorsIntersectingSphereWithCriterion(&intersectingActors, &intersectionSphere, FUN_002bf4b0, &gGameCheckStruct);

		while (iVar6 = 0, intersectingActors.nbEntries != 0) {
			pCVar2 = intersectingActors.PopCurrent();
			if (this->actorsTable.nbEntries == 0) {
				if ((this->modeFlags & 2) == 0) {
					UpdateTarget(&this->field_0x330, false);
					edF32Vector4AddHard(&this->field_0x330, &this->field_0x330, &GetTarget()->currentLocation);
					(this->field_0x330).y = 0.0f;
				}

				this->field_0x444 = 0.0f;
				this->field_0x2f0 = 0;
				this->modeFlags = this->modeFlags | 1;
			}

			iVar6 = this->actorsTable.IsInList(pCVar2);

			if (iVar6 == 0) {
				this->actorsTable.Add(pCVar2);
			}
		}

		while (iVar6 < this->actorsTable.nbEntries) {
			if ((this->actorsTable.aEntries[iVar6]->Can_0x9c() == 0) ||
				(pCurEntry = this->actorsTable.aEntries[iVar6], fVar12 = local_130.x - pCurEntry->currentLocation.x,
					fVar7 = local_130.y - pCurEntry->currentLocation.y, fVar8 = local_130.z - pCurEntry->currentLocation.z,
					this->field_0x2dc * this->field_0x2dc < fVar12 * fVar12 + fVar7 * fVar7 + fVar8 * fVar8)) {
				pCurEntry = this->actorsTable.aEntries[iVar6];

				if (this->actorsTable.IsInList(pCurEntry) != 0) {
					this->actorsTable.Remove(pCurEntry);
				}

				if (this->actorsTable.nbEntries == 0) {
					this->field_0x2f0 = 0;
				}
			}
			else {
				iVar6 = iVar6 + 1;
			}
		}

		this->field_0x2f0 = this->field_0x2f0 + ((this->subObj_12).field_0xc - this->field_0x2f0) * 0.025f;

		if (this->actorsTable.nbEntries == 0) {
			if ((this->modeFlags & 4) != 0) {
				this->modeFlags = this->modeFlags | 2;
			}

			UpdateTarget(&eStack320, false);
			edF32Vector4AddHard(&eStack320, &eStack320, &GetTarget()->currentLocation);
			eStack320.y = 0.0f;
			edF32Vector4LERPHard(this->field_0x2f0, &this->field_0x330, &this->field_0x330, &eStack320);
			(this->field_0x330).w = 1.0f;
			edF32Vector4SubHard(&eStack352, &this->field_0x330, &eStack320);

			fVar12 = edF32Vector4GetDistHard(&eStack352);
			this->field_0x444 = fVar12;

			if (fVar12 < 0.05f) {
				this->modeFlags = 0;
			}
		}
		else {
			this->modeFlags = this->modeFlags & 0xffffff99;
			for (iVar6 = 0; iVar6 < this->actorsTable.nbEntries; iVar6 = iVar6 + 1) {
				this->modeFlags = this->modeFlags | this->actorsTable.aEntries[iVar6]->actorFieldS & 0x60;
			}

			local_170.y = 1e+08f;
			local_180.y = -1e+08f;
			fVar12 = local_180.y;
			local_180.z = local_180.y;
			local_170.z = local_170.y;
			local_170.x = local_170.y;

			for (iVar6 = 0; local_180.x = fVar12 - local_170.x, iVar6 < this->actorsTable.nbEntries; iVar6 = iVar6 + 1) {
				pCurEntry = this->actorsTable.aEntries[iVar6];
				fVar7 = pCurEntry->currentLocation.x;
				if (local_170.x <= fVar7) {
					fVar7 = local_170.x;
				}

				fVar8 = pCurEntry->currentLocation.x;
				if (fVar8 <= fVar12) {
					fVar8 = fVar12;
				}

				fVar9 = pCurEntry->currentLocation.y;
				if (local_170.y <= fVar9) {
					fVar9 = local_170.y;
				}

				fVar12 = pCurEntry->currentLocation.y;
				if (fVar12 <= local_180.y) {
					fVar12 = local_180.y;
				}

				fVar10 = pCurEntry->currentLocation.z;
				if (local_170.z <= fVar10) {
					fVar10 = local_170.z;
				}

				fVar11 = pCurEntry->currentLocation.z;
				if (fVar11 <= local_180.z) {
					fVar11 = local_180.z;
				}

				local_180.y = fVar12;
				fVar12 = fVar8;
				local_180.z = fVar11;
				local_170.y = fVar9;
				local_170.z = fVar10;
				local_170.x = fVar7;
			}

			local_170.w = 1.0f;
			local_180.w = 0.0f;
			local_180.y = local_180.y - local_170.y;
			local_180.z = local_180.z - local_170.z;
			edF32Vector4ScaleHard(0.5f, &eStack400, &local_180);
			edF32Vector4AddHard(&this->field_0x320, &local_170, &eStack400);

			intersectionSphere.xyz = this->field_0x320.xyz;
			intersectionSphere.w = edF32Vector4GetDistHard(&local_180);
			intersectionSphere.w = intersectionSphere.w * 0.5f;

			UpdateTarget(&eStack320, false);
			edF32Vector4AddHard(&eStack320, &eStack320, &GetTarget()->currentLocation);
			eStack320.y = 0.0f;

			edF32Vector4SubHard(&eStack352, &this->field_0x320, &eStack320);
			edF32Vector4ScaleHard(0.5f, &eStack352, &eStack352);
			edF32Vector4AddHard(&eStack336, &eStack320, &eStack352);
			eStack336.y = 0.0f;
			edF32Vector4LERPHard(this->field_0x2f0, &this->field_0x330, &this->field_0x330, &eStack336);
			(this->field_0x330).w = 1.0f;
			edF32Vector4SubHard(&eStack352, &this->field_0x330, &eStack320);
			fVar12 = edF32Vector4GetDistHard(&eStack352);
			this->field_0x444 = fVar12;
		}

		FUN_00199c20(&local_1a0, &this->field_0x330);
		this->field_0x330 = local_1a0;
	}

	return;
}

void CCameraGame::_Normal_ManageIdle()
{
	int iVar1;
	uint* puVar2;
	float fVar3;
	float fVar4;
	float fVar5;
	float fVar6;

	fVar3 = 0.0f;
	if (((CCamera::_gpcam_man->cheatIdleCameraActive == 0) || (0.05f <= this->field_0xd0->dynamic.speed)) ||
		(iVar1 = CCameraStack::GetCurHeroState(), iVar1 != 4)) {
		(this->field_0x450).x = 0.0f;
	}
	else {
		fVar4 = (this->field_0x450).x + CCamera::_gpcam_man->time_0x4;
		(this->field_0x450).x = fVar4;
		if (30.0f < fVar4) {
			fVar3 = 1.401298e-45f;
		}
	}

	if (((this->field_0x450).y == 0.0f) && (fVar3 != 0.0f)) {
		this->field_0x460 = (uint)((this->flags_0xc & 1) != 0);
		this->field_0x464 = (this->cameraConfig).field_0xbc;
		this->field_0x468 = this->targetPitch;
		this->field_0x470 = this->field_0x50;
		(this->field_0x450).z = 0.0f;
		(this->field_0x450).w = 0.0f;
	}
	if (((this->field_0x450).y != 0.0f) && (fVar3 == 0.0)) {
		this->field_0x1b0 = this->field_0x1b0 & 0xfffffff9;
		puVar2 = &this->flags_0xc;
		if (this->field_0x460 == 0) {
			*puVar2 = *puVar2 & 0xfffffffe;
		}
		else {
			*puVar2 = *puVar2 | 1;
		}

		this->field_0x50 = this->field_0x470;
		(this->cameraConfig).targetPitch = this->field_0x468;
		(this->cameraConfig).field_0xbc = this->field_0x464;
	}
	if (fVar3 != 0.0f) {
		this->flags_0xc = this->flags_0xc & 0xfffffffe;
		this->field_0x1b0 = this->field_0x1b0 | 0x46;
		this->field_0x1b0 = this->field_0x1b0 | 0x40;
		this->field_0x1b0 = this->field_0x1b0 & 0xffffffdf;
		(this->cameraConfig).field_0xac = 0.3926991f;
		(this->cameraConfig).field_0xb0 = (this->cameraConfig).field_0x48.z;
		(this->cameraConfig).field_0xbc = 1.0f;
		this->field_0x50.z = 0.0f;
		this->field_0x50.x = 0.0f;
		// Might not be cosf
		this->field_0x50.y = cosf(this->field_0x450.w) * 0.3f + 1.2f;
		fVar4 = edF32Between_2Pi((this->field_0x450).w + CCamera::_gpcam_man->time_0x4 * 0.25f);
		(this->field_0x450).w = fVar4;
		(this->cameraConfig).targetPitch = cosf((this->field_0x450).z) * 0.3926991f;
		fVar4 = edF32Between_2Pi((this->field_0x450).z + CCamera::_gpcam_man->time_0x4 * 0.25f);
		(this->field_0x450).z = fVar4;
	}
	(this->field_0x450).y = fVar3;
	return;
}

void CCameraGame::CameraFunc_002c5b50()
{
	uint uVar1;
	bool bVar2;
	CActor* pCVar3;
	uint* puVar4;
	float fVar5;
	float fVar6;
	float fVar7;
	edF32VECTOR4 eStack64;
	edF32VECTOR4 eStack16;

	puVar4 = &(this->cameraConfig).flags_0x70;
	if (((this->cameraConfig).flags_0x70 & 0x10000) == 0) {
		*puVar4 = *puVar4 | 0x10000;
		fVar7 = (this->cameraConfig).field_0xbc;
		fVar5 = edF32Between_Pi(this->targetPitch + 0.1963495f);
		fVar6 = -1.570796;

		if (-1.570796 <= fVar5) {
			fVar6 = fVar5;
		}

		fVar5 = 1.570796;
		if (fVar6 <= 1.570796) {
			fVar5 = fVar6;
		}

		CCollisionRay CStack48 = CCollisionRay(fVar5, this->field_0x204, fVar7 + 0.8f, &this->gameLookAt, &eStack16);
		edF32Vector4ScaleHard(-1.0f, &eStack16, &eStack16);
		pCVar3 = GetTarget();
		fVar6 = CStack48.Intersect(this->cameraRayFlags, pCVar3, (CActor*)0x0, 0x40000004, (edF32VECTOR4*)0x0,	(_ray_info_out*)0x0);
		this->field_0x228 = fVar6;
	}

	bVar2 = this->field_0x228 < this->field_0x208 + 0.8f;

	if (!bVar2) {
		bVar2 = (this->cameraConfig).field_0x30 - (this->cameraConfig).field_0x34 * CCamera::_gpcam_man->time_0x4 <= this->targetPitch;
	}

	if (bVar2) {
		*puVar4 = *puVar4 | 0x800;
	}
	else {
		*puVar4 = *puVar4 & 0xfffff7ff;
	}

	if ((*puVar4 & 0x20000) == 0) {
		*puVar4 = *puVar4 | 0x20000;
		fVar7 = (this->cameraConfig).field_0xbc;
		fVar5 = edF32Between_Pi(this->targetPitch + -0.1963495f);

		fVar6 = -1.570796f;
		if (-1.570796f <= fVar5) {
			fVar6 = fVar5;
		}

		fVar5 = 1.570796f;
		if (fVar6 <= 1.570796f) {
			fVar5 = fVar6;
		}

		CCollisionRay CStack96 = CCollisionRay(fVar5, this->field_0x204, fVar7 + 0.8f, &this->gameLookAt, &eStack64);
		edF32Vector4ScaleHard(-1.0f, &eStack64, &eStack64);
		pCVar3 = GetTarget();
		fVar6 = CStack96.Intersect(this->cameraRayFlags, pCVar3, (CActor*)0x0, 0x40000004, (edF32VECTOR4*)0x0, (_ray_info_out*)0x0);
		this->field_0x22c = fVar6;
	}

	bVar2 = this->field_0x22c < this->field_0x208 + 0.8f;

	if (!bVar2) {
		bVar2 = this->targetPitch <=
			(this->cameraConfig).field_0x34 * CCamera::_gpcam_man->time_0x4 + (this->cameraConfig).field_0x2c;
	}

	if (bVar2) {
		*puVar4 = *puVar4 | 0x1000;
	}
	else {
		*puVar4 = *puVar4 & 0xffffefff;
	}

	*puVar4 = *puVar4 & 0xfffffffd;
	if ((*puVar4 & 0x20) != 0) {
		fVar5 = (this->cameraConfig).field_0x34;
		fVar6 = fVar5 * CCamera::_gpcam_man->time_0x4;
		if ((this->targetPitch < (this->cameraConfig).field_0x30 - fVar6) ||
			(fVar6 + (this->cameraConfig).field_0x2c < this->targetPitch)) {
			uVar1 = *puVar4;
			if (((uVar1 & 0x10) == 0) && ((this->field_0x1b0 & 1) == 0)) {
				if (((uVar1 & 0x1000) != 0) && ((uVar1 & 0x800) == 0)) {
					this->field_0x1b0 = this->field_0x1b0 | 0x80;
					this->field_0x1b0 = this->field_0x1b0 & 0xfffffeff;
					(this->cameraConfig).field_0x90 = fVar5 * 1.0;
					(this->cameraConfig).field_0x94 = (this->cameraConfig).field_0x38.x;
				}

				if (((*puVar4 & 0x800) != 0) && ((*puVar4 & 0x1000) == 0)) {
					fVar6 = (this->cameraConfig).field_0x34;
					this->field_0x1b0 = this->field_0x1b0 | 0x100;
					this->field_0x1b0 = this->field_0x1b0 & 0xffffff7f;
					(this->cameraConfig).field_0x90 = fVar6 * -1.0;
					(this->cameraConfig).field_0x94 = (this->cameraConfig).field_0x38.x;
				}
			}

			if ((this->field_0x1b0 & 0x180) != 0) {
				this->field_0x1b8 = 1;
			}
		}
		else {
			this->field_0x1b0 = this->field_0x1b0 & 0xfffffe7f;
			(this->cameraConfig).field_0x8c = 0;
			(this->cameraConfig).field_0x90 = 0;
			(this->cameraConfig).field_0x94 = (this->cameraConfig).field_0x38.y;
		}
	}
	return;
}

void CCameraGame::FUN_002c8160()
{
	CActorHeroPrivate* pCVar1;
	uint* puVar2;

	pCVar1 = this->field_0xd0;
	if (((this->cameraConfig).flags_0x70 & 0x2000000) != 0) {
		pCVar1 = this->field_0x1ec;
	}

	if ((this->field_0x280 == 0) && (pCVar1 != (CActorHeroPrivate*)0x0)) {
		this->field_0x1d8 = pCVar1->dynamic.horizontalLinearAcceleration;
	}
	else {
		this->field_0x1d8 = 7.0f;
	}

	this->field_0x1dc = fabs(pCVar1->dynamic.linearAcceleration * pCVar1->dynamic.velocityDirectionEuler.y);
	if (this->field_0x1d8 < fabs(this->field_0x1dc)) {
		this->field_0x1b8 = 1;
	}
	else {
		this->field_0x1b8 = 0;
	}

	puVar2 = &(this->cameraConfig).flags_0x70;
	if (this->field_0x1d8 <= (this->cameraConfig).field_0x20) {
		*puVar2 = *puVar2 & 0xff7fffff;
	}
	else {
		*puVar2 = *puVar2 | 0x800000;
	}

	puVar2 = &(this->cameraConfig).flags_0x70;
	if (this->pOtherTarget == (CActor*)0x0) {
		*puVar2 = *puVar2 & 0xffbfffff;
	}
	else {
		*puVar2 = *puVar2 | 0x400000;
	}

	return;
}

void CCameraGame::_UpdateAngleAlphaData()
{
	uint uVar1;
	bool bVar2;
	bool bVar3;
	bool bVar4;
	bool bVar5;
	bool actorBool;
	ECameraType cameraMode;
	CCameraGame* pCVar6;
	CActor* pActorA;
	CActorHero* pActorB;
	CActor* pActorC;
	CActor* pCVar7;
	Timer* pTimeController;
	uint* puVar8;
	CActorHeroPrivate* pCVar9;
	float pitch;
	float fVar10;
	float fVar11;
	float puVar12;
	float calculatedPitch;
	edF32VECTOR4 local_a0;
	edF32VECTOR4 local_90;
	edF32VECTOR4 eStack64;
	edF32VECTOR4 local_30;
	edF32VECTOR4 local_20;
	_ray_info_out _Stack16;

	cameraMode = GetMode();

	if (cameraMode != CT_KyaJamgut) {
		cameraMode = GetMode();

		if (cameraMode == CT_KyaWindWall) {
			/* Camera mode is (5) */
			if (((this->cameraConfig).flags_0x70 & 0x2000) != 0) {
				this->field_0x1b0 = this->field_0x1b0 | 0x18;
			}

			pitch = edF32GetAnglesDelta(this->targetPitch, this->cameraConfig.baseTargetPitch);

			if (1.570796f < pitch) {
				pitch = 3.141593f - pitch;
			}
			if (pitch < -1.570796f) {
				pitch = -3.141593f - pitch;
			}

			if (((CCamera::_gpcam_man->flags & 0x4000000) != 0) &&
				(pCVar6 = (CCameraGame*)CCamera::_gpcam_man->activeCamManager.FUN_003feab0(0), this == pCVar6)) {
				calculatedPitch = this->cameraConfig.baseTargetPitch;
				this->pitchDyn.currentAlpha = calculatedPitch;
				this->targetPitch = calculatedPitch;
			}

			puVar12 = sinf(pitch) / cosf(pitch) / 2.414f; //((edFCosinus[(int)(fabs((pitch - 1.570796) * 1303.797) + 0.5) & 0x1fff] /
					//edFCosinus[(int)(fabs(pitch * 1303.797) + 0.5) & 0x1fff]) / 2.414);

			if (1.0f < fabs(puVar12)) {
				if (0.0f <= puVar12) {
					puVar12 = 1.0f;
				}
				else {
					puVar12 = -1.0f;
				}
			}

			pitch = edF32Between_2Pi(this->targetPitch + puVar12 * (this->cameraConfig).field_0x7c * CCamera::_gpcam_man->time_0x4);
			this->pitchDyn.currentAlpha = pitch;
			this->targetPitch = pitch;
			calculatedPitch = this->cameraConfig.field_0x2c;

			if (pitch < calculatedPitch) {
				this->pitchDyn.currentAlpha = calculatedPitch;
				this->targetPitch = calculatedPitch;
			}
			else {
				calculatedPitch = this->cameraConfig.field_0x30;

				if (calculatedPitch < pitch) {
					this->pitchDyn.currentAlpha = calculatedPitch;
					this->targetPitch = calculatedPitch;
				}
			}
		}
		else {
			/* Camera mode is not (5) */
			if ((this->mode_0x1bc == 4) || (this->mode_0x1bc == 3)) {
				pitch = this->targetPitch + (this->cameraConfig.baseTargetPitch - this->targetPitch) * 0.1f;
				this->pitchDyn.currentAlpha = pitch;
				this->targetPitch = pitch;
			}
			else {
				puVar8 = &(this->cameraConfig).flags_0x70;
				if (((this->cameraConfig).flags_0x70 & 0x800000) != 0) {
					*puVar8 = *puVar8 & 0xfffffffe;
				}

				/* Auto camera goes in here */
				cameraMode = GetMode();
				if (cameraMode == CT_Main) {
					actorBool = false;
					pActorA = GetTarget();
					if (pActorA->typeID == ACTOR_HERO_PRIVATE) {
						pActorB = (CActorHero*)GetTarget();
						actorBool = pActorB->TestState_IsInCheatMode();
					}

					pCVar9 = this->field_0xd0;
					if (((this->cameraConfig).flags_0x70 & 0x2000000) != 0) {
						pCVar9 = this->field_0x1ec;
					}

					bVar5 = false;
					bVar4 = false;
					bVar3 = false;
					bVar2 = false;
					pActorC = GetTarget();
					if ((pActorC->typeID == ACTOR_HERO_PRIVATE) && (this->field_0x1d4 == 0)) {
						bVar2 = true;
					}

					if ((bVar2) && (this->field_0x1d0 != 0)) {
						bVar3 = true;
					}

					if ((bVar3) && (pCVar9->dynamic.linearAcceleration * pCVar9->dynamic.velocityDirectionEuler.y < 0.5f)) {
						bVar4 = true;
					}

					if ((bVar4) && (actorBool == false)) {
						bVar5 = true;
					}

					puVar8 = &(this->cameraConfig).flags_0x70;
					if (bVar5) {
						*puVar8 = *puVar8 | 0x1000000;
					}
					else {
						*puVar8 = *puVar8 & 0xfeffffff;
					}
				}

				uVar1 = (this->cameraConfig).flags_0x70;
				if ((uVar1 & 0x1000000) == 0) {
					if ((uVar1 & 8) == 0) {
						uVar1 = this->cameraConfig.flags;

						if ((uVar1 & 0x1000) == 0) {
							if ((uVar1 & 8) == 0) {
								pitch = (this->cameraConfig).targetPitch;
								this->pitchDyn.currentAlpha = pitch;
								this->targetPitch = pitch;
								this->pitchDyn.currentAlpha = pitch;
							}
							else {
								/* Auto camera goes in here */
								pTimeController = GetTimer();
								if (pTimeController->timeScale != 0.0f) {
									if ((this->field_0x1b0 & 0x180) == 0) {
										if ((~(this->cameraConfig).flags_0x70 & 3) == 3) {											
											pitch = this->pitchDyn.UpdateLerp((this->cameraConfig).targetPitch);
											this->pitchDyn.currentAlpha = pitch;
											this->targetPitch = pitch;

											CAMERA_LOG(LogLevel::Verbose, "CameraGame::_UpdateAngleAlphaData new pitch: {} target: {}", pitch, (this->cameraConfig).targetPitch);
										}
									}
									else {
										pitch = (this->cameraConfig).field_0x8c;

										if (pitch < (this->cameraConfig).field_0x90) {
											pitch = pitch + (this->cameraConfig).field_0x94 * CCamera::_gpcam_man->time_0x4;
											(this->cameraConfig).field_0x8c = pitch;
											calculatedPitch = (this->cameraConfig).field_0x90;

											if (calculatedPitch < pitch) {
												(this->cameraConfig).field_0x8c = calculatedPitch;
											}
										}
										else {
											pitch = pitch - (this->cameraConfig).field_0x94 * CCamera::_gpcam_man->time_0x4;
											(this->cameraConfig).field_0x8c = pitch;
											calculatedPitch = (this->cameraConfig).field_0x90;

											if (pitch < calculatedPitch) {
												(this->cameraConfig).field_0x8c = calculatedPitch;
											}
										}

										pitch = this->targetPitch + (this->cameraConfig).field_0x8c * CCamera::_gpcam_man->time_0x4;
										this->pitchDyn.currentAlpha = pitch;
										this->targetPitch = pitch;
										this->pitchDyn.currentAlpha = pitch;
										(this->cameraConfig).flags_0x70 = (this->cameraConfig).flags_0x70 | 1;
									}
								}
							}
						}
						else {
							pCVar7 = GetTarget();
							IMPLEMENTATION_GUARD(
							if (fabs((float)pCVar7[1].objectId) <= 0.1) {
								(this->cameraConfig).targetPitch = this->cameraConfig.baseTargetPitch;
							}
							else {
								if (0.0 < (float)pCVar7[1].objectId) {
									(this->cameraConfig).targetPitch = this->cameraConfig.field_0x2c;
								}
								else {
									(this->cameraConfig).targetPitch = this->cameraConfig.field_0x30;
								}
							}
							pitch = this->pitchDyn.UpdateLerp((this->cameraConfig).targetPitch);
							this->pitchDyn.currentAlpha = pitch;
							this->targetPitch = pitch;)
						}
					}
					else {
						/* In air camera */
						pitch = GetAngleXFromVector(&this->transformationMatrix.rowZ);
						calculatedPitch = this->field_0x1dc;

						if ((calculatedPitch < 0.0f) || (100.0f < calculatedPitch)) {
							calculatedPitch = 0.0f;
						}

						calculatedPitch = edFIntervalLERP(calculatedPitch, 0.0f, 10.0f, 3.0f, 0.0f);
						fVar11 = edF32GetAnglesDelta(pitch, this->cameraConfig.baseTargetPitch);
						/* This will just return our input angle in most cases. */
						calculatedPitch =
							edF32Between_Pi(pitch + calculatedPitch * (sinf(fVar11) / cosf(fVar11) / 2.414f) * CCamera::_gpcam_man->time_0x4);
								//((edFCosinus[(int)(fabs((fVar11 - 1.570796) * 1303.797) + 0.5) & 0x1fff] /
								//	edFCosinus[(int)(fabs(fVar11 * 1303.797) + 0.5) & 0x1fff]) / 2.414) *
								//CCamera::_gpcam_man->time_0x4);
						local_20 = g_xVector;
						local_30.x = this->transformationMatrix.da;
						local_30.z = this->transformationMatrix.dc;
						local_30.w = this->transformationMatrix.dd;
						local_30.y = this->transformationMatrix.db - 0.8;
						/* The below functions usually return the large float, and so the code never runs in the if
						   check below
						   Suspect that these are here for cinematic shots (getting shot from a cannon) */
						CCollisionRay CStack96 = CCollisionRay(1.6f, &local_30, &local_20);
						pCVar7 = GetTarget();
						fVar11 = CStack96.Intersect(this->cameraRayFlags, pCVar7, (CActor*)0x0, 0x40000004, &eStack64, &_Stack16);
						fVar10 = edF32Vector4DotProductHard(&eStack64, &local_20);
						if ((fVar11 != 1e+30f) && (fVar10 <= 0.0f)) {
							local_30.y = local_30.y + (fVar11 - 0.8f);
							calculatedPitch = pitch;
							if (0.01f < fabs(local_30.y - this->transformationMatrix.db)) {
								this->transformationMatrix.rowT = local_30;

								edF32Vector4SubHard(&local_90, &this->lookAt, &this->transformationMatrix.rowT);

								if ((~(this->cameraConfig).flags_0x70 & 0x1000008) == 0x1000008) {
									local_90.y = 0.0f;
								}

								edF32Vector4NormalizeHard(&local_90, &local_90);
								this->transformationMatrix.rowZ = local_90;

								calculatedPitch = GetAngleXFromVector(&this->transformationMatrix.rowZ);
							}
						}
						local_20.x = g_xVector.x;
						local_20.z = g_xVector.z;
						local_20.w = g_xVector.w;
						local_20.y = -1.0f;
						local_30.x = this->transformationMatrix.da;
						local_30.z = this->transformationMatrix.dc;
						local_30.w = this->transformationMatrix.dd;
						local_30.y = this->transformationMatrix.db + 0.8f;
						/* The below functions usually return the large float, and so the code never runs in the if
						   check below
						   Suspect that these are here for cinematic shots (getting shot from a cannon) */
						CCollisionRay CStack128 = CCollisionRay(1.6f, &local_30, &local_20);
						pCVar7 = GetTarget();
						fVar11 = CStack128.Intersect(this->cameraRayFlags, pCVar7, (CActor*)0x0, 0x40000004, &eStack64, &_Stack16);

						fVar10 = edF32Vector4DotProductHard(&eStack64, &local_20);

						if ((fVar11 != 1e+30f) && (fVar10 <= 0.0f)) {
							local_30.y = local_30.y - (fVar11 - 0.8f);
							calculatedPitch = pitch;
							if (0.01f < fabs(local_30.y - this->transformationMatrix.db)) {
								this->transformationMatrix.rowT = local_30;

								edF32Vector4SubHard(&local_a0, &this->lookAt, &this->transformationMatrix.rowT);

								if ((~(this->cameraConfig).flags_0x70 & 0x1000008) == 0x1000008) {
									local_a0.y = 0.0f;
								}

								edF32Vector4NormalizeHard(&local_a0, &local_a0);
								this->transformationMatrix.rowZ = local_a0;

								calculatedPitch = GetAngleXFromVector(&this->transformationMatrix.rowZ);
							}
						}

						this->pitchDyn.currentAlpha = calculatedPitch;
						this->targetPitch = calculatedPitch;
					}
				}
				else {
					pitch = this->field_0x1dc;

					if ((pitch < 0.0f) || (100.0f < pitch)) {
						pitch = 0.0f;
					}

					pitch = edFIntervalLERP(pitch, 0.0f, 15.0f, 0.0f, 7.0f);
					fVar11 = this->targetPitch;
					calculatedPitch = GetAngleXFromVector(&field_0xd0->dynamic.velocityDirectionEuler);
					calculatedPitch = edF32GetAnglesDelta(fVar11, calculatedPitch);

					pitch = edF32Between_Pi(this->targetPitch + pitch * (sinf(calculatedPitch) / cosf(calculatedPitch) / 2.414f) * CCamera::_gpcam_man->time_0x4);
					this->pitchDyn.currentAlpha = pitch;
					this->targetPitch = pitch;
				}

				/* Make sure the camera doesn't go outside the allowed range */
				pitch = this->cameraConfig.field_0x2c;

				if (this->targetPitch < pitch) {
					this->pitchDyn.currentAlpha = pitch;
					this->targetPitch = pitch;
				}
				else {
					pitch = this->cameraConfig.field_0x30;

					if (pitch < this->targetPitch) {
						this->pitchDyn.currentAlpha = pitch;
						this->targetPitch = pitch;
					}
				}
			}
		}
	}
	return;
}

void CCameraGame::_Behind_UpdateAngleBetaData()
{
	CCameraGame* pCVar1;
	CActor* pCVar2;
	CActorHeroPrivate* this_00;
	float fVar3;
	float fVar4;
	float puVar5;

	if (((this->cameraConfig).flags_0x70 & 0x2000) != 0) {
		this->field_0x1b0 = this->field_0x1b0 | 6;
	}

	fVar4 = this->angleBeta_0x1e8;
	fVar3 = GetAngleYFromVector(&this->transformationMatrix.rowZ);
	fVar3 = edF32GetAnglesDelta(fVar3, fVar4);
	if (1.570796f < fVar3) {
		fVar3 = 3.141593f - fVar3;
	}
	if (fVar3 < -1.570796f) {
		fVar3 = -3.141593f - fVar3;
	}

	if (((CCamera::_gpcam_man->flags & 0x4000000) != 0) &&
		(pCVar1 = (CCameraGame*)CCamera::_gpcam_man->activeCamManager.FUN_003feab0(0), this == pCVar1))
	{
		pCVar2 = this->GetTarget();
		if (pCVar2->typeID == ACTOR_HERO_PRIVATE) {
			this_00 = reinterpret_cast<CActorHeroPrivate*>(this->GetTarget());
			fVar4 = this_00->GetTargetBeta();
			this->field_0x204 = fVar4;
		}
		else {
			this->field_0x204 = this->angleBeta_0x1e8;
		}
	}
	puVar5 = sinf(fVar3) / cosf(fVar3) / 2.414f;

	if (1.0f < fabs(puVar5)) {
		if (0.0f <= puVar5) {
			puVar5 = 1.0f;
		}
		else {
			puVar5 = -1.0f;
		}
	}

	fVar3 = edF32Between_2Pi(this->field_0x204 + puVar5 * (this->cameraConfig).field_0x7c * CCamera::_gpcam_man->time_0x4);
	this->field_0x204 = fVar3;

	if (((this->cameraConfig).flags & 0x100) != 0) {
		fVar3 = edF32GetAnglesDelta((this->cameraConfig).field_0x38.w, this->field_0x204);
		fVar4 = edF32GetAnglesDelta((this->cameraConfig).field_0x48.x, this->field_0x204);
		if (fVar3 < 0.0f) {
			this->field_0x204 = (this->cameraConfig).field_0x38.w;
		}

		if (0.0f < fVar4) {
			this->field_0x204 = (this->cameraConfig).field_0x48.x;
		}
	}

	return;
}

void CCameraGame::_UpdateAngleBetaData()
{
	ECameraType EVar1;
	uint* puVar2;
	float fVar3;
	float t1;
	float puVar4;
	float fVar5;
	float fVar6;

	EVar1 = GetMode();
	if ((EVar1 != CT_KyaJamgut) && (EVar1 != CT_Fight)) {
		if (EVar1 == CT_KyaWindWall) {
			_Behind_UpdateAngleBetaData();
		}
		else {
			if ((((EVar1 == 9) || (EVar1 == 8)) || (EVar1 == 6)) || (EVar1 == CT_Main)) {
				fVar5 = 0.0f;
				fVar6 = (this->cameraConfig).field_0x78;

				if (this->mode_0x1bc != 3) {
					fVar3 = GetAngleYFromVector(&this->transformationMatrix.rowZ);

					if ((((((this->cameraConfig).flags_0x70 & 0x800000) == 0) && (fabs(fVar3 - this->field_0x204) < 0.001f)) ||
						((((this->cameraConfig).flags_0x70 & 0x400000) != 0 && ((this->cameraConfig.flags & 0x4000) == 0)))
						) || ((EVar1 = GetMode(), EVar1 == 4 ||
							(EVar1 = GetMode(), EVar1 == 9)))) {
						fVar3 = this->field_0x204;
					}

					EVar1 = GetMode();
					t1 = fVar3;
					if (EVar1 != 4) {
						if (this->field_0x280 == 0) {
							t1 = this->angleBeta_0x1e8;
						}
						else {
							t1 = GetAngleYFromVector(&this->field_0x270);
						}
					}

					if (this->mode_0x1bc == 2) {
						this->field_0x204 = fVar3;
					}

					if (((this->cameraConfig).flags_0x70 & 0x200000) == 0) {
						fVar3 = edF32GetAnglesDelta(fVar3, t1);

						if (1.570796f < fVar3) {
							fVar3 = 3.141593f - fVar3;
						}

						if (fVar3 < -1.570796f) {
							fVar3 = -3.141593f - fVar3;
						}

						if (((this->cameraConfig).flags_0x70 & 0x800000) != 0) {
							fVar5 = this->field_0x1d8;
							if ((fVar5 < 0.0f) || (100.0f < fVar5)) {
								fVar5 = 0.0f;
							}

							if (((this->cameraConfig).flags_0x70 & 0x2000000) != 0) {
								fVar6 = 0.05f;
							}

							fVar5 = fVar6 * fVar5;
						}

						puVar4 = sinf(fVar3) / cosf(fVar3) / 2.414f;
							//((edFCosinus[(int)(fabs((fVar3 - 1.570796) * 1303.797) + 0.5) & 0x1fff] /
							//	edFCosinus[(int)(fabs(fVar3 * 1303.797) + 0.5) & 0x1fff]) / 2.414);
						if (1.0f < fabs(puVar4)) {
							if (0.0f <= puVar4) {
								puVar4 = 1.0f;
							}
							else {
								puVar4 = -1.0f;
							}
						}
					}
					else {
						fVar6 = edF32GetAnglesDelta(fVar3, this->field_0x46c);
						fVar5 = 10.0f;

						if ((fabs(fVar6) <= 0.03490658f) || ((this->field_0x1b0 & 6) != 0)) {
							(this->cameraConfig).flags_0x70 = (this->cameraConfig).flags_0x70 & 0xffdfffff;
						}

						puVar2 = &(this->cameraConfig).flags_0x70;
						if (((this->cameraConfig).flags_0x70 & 0x600) != 0) {
							*puVar2 = *puVar2 & 0xffdfffff;
						}

						puVar4 = edFIntervalLERP(fVar6, -1.570796f, 1.570796f, -1.0f, 1.0f);
					}

					fVar6 = edF32Between_2Pi(this->field_0x204 + puVar4 * (fVar5 + (this->cameraConfig).field_0x7c) * CCamera::_gpcam_man->time_0x4);
					this->field_0x204 = fVar6;

					if ((this->cameraConfig.flags & 0x100) != 0) {
						fVar6 = edF32GetAnglesDelta(this->cameraConfig.field_0x38.w, this->field_0x204);
						fVar5 = edF32GetAnglesDelta(this->cameraConfig.field_0x48.x, this->field_0x204);

						if (fVar6 < 0.0f) {
							this->field_0x204 = this->cameraConfig.field_0x38.w;
						}

						if (0.0f < fVar5) {
							this->field_0x204 = this->cameraConfig.field_0x48.x;
						}
					}
				}
			}
		}
	}

	return;
}


void CCameraGame::ClampFunc(uint* puVar5)
{
	if ((*puVar5 & 0x80000) == 0) {
		float fVar6 = -1.570796f;
		*puVar5 = *puVar5 | 0x80000;

		float fVar7 = (this->cameraConfig).field_0xbc + 0.8f;
		if (-1.570796f <= this->targetPitch) {
			fVar6 = this->targetPitch;
		}

		float fVar8 = 1.570796f;
		if (fVar6 <= 1.570796f) {
			fVar8 = fVar6;
		}

		edF32VECTOR4 eStack16;

		Log::GetInstance().ForceFlush();

		assert(!std::isnan(field_0x204));

		fVar6 = edF32Between_0_2Pi(this->field_0x204 + 0.0f);
		CCollisionRay CStack48 = CCollisionRay(fVar8, fVar6, fVar7, &this->gameLookAt, &eStack16);
		edF32Vector4ScaleHard(-1.0f, &eStack16, &eStack16);
		CActor* pCVar3 = GetTarget();
		fVar6 = CStack48.Intersect(this->cameraRayFlags, pCVar3, (CActor*)0x0, 0x40000004, (edF32VECTOR4*)0x0, (_ray_info_out*)0x0);
		this->field_0x238 = fVar6;
	}
}

void CCameraGame::ClampFuncA(uint* puVar5)
{
	*puVar5 = *puVar5 | 0x40000;

	ClampFunc(puVar5);

	float fVar6 = (this->cameraConfig).field_0x58.w;
	Timer* pTVar4 = GetTimer();
	if (this->field_0x238 <= fVar6 * pTVar4->cutsceneDeltaTime + this->field_0x208 + 0.8f) {
		this->field_0x230 = this->field_0x238;
	}
	else {
		this->field_0x230 = 1e+30f;
	}
}

void CCameraGame::SlowHorizontalCameraPan()
{
	uint uVar1;
	bool bVar2;
	CActor* pCVar3;
	Timer* pTVar4;
	uint* puVar5;
	float fVar6;
	float fVar7;
	float fVar8;

	if ((((this->field_0x1b0 & 0x20) != 0) && (((this->cameraConfig).flags_0x70 & 0x400) == 0)) &&
		((~this->field_0x1b0 & 6) == 6)) {
		this->field_0x1b0 = this->field_0x1b0 & 0xffffff9f;
		(this->cameraConfig).field_0xac = 0.0f;
		(this->cameraConfig).field_0xb0 = (this->cameraConfig).field_0x48.w;
	}

	uVar1 = (this->cameraConfig).flags_0x70;
	puVar5 = &(this->cameraConfig).flags_0x70;
	if ((((uVar1 & 0x10) == 0) && ((this->field_0x1b0 & 1) == 0)) && ((uVar1 & 0x200) != 0)) {
		bVar2 = (uVar1 & 0x40000) == 0;

		if ((bVar2) && (bVar2)) {
			ClampFuncA(puVar5);
		}

		if ((this->field_0x208 - 0.8f < this->field_0x230) &&
			(((this->field_0x1b0 & 0x20) != 0 || ((this->cameraConfig).field_0xa8 < -0.001f)))) {
			this->field_0x1b0 = this->field_0x1b0 & 0xffffff9f;
			(this->cameraConfig).field_0xa8 = 0.0f;
			(this->cameraConfig).field_0xac = 0.0f;
			(this->cameraConfig).field_0xb0 = (this->cameraConfig).field_0x48.w;
		}
	}

	if ((((this->field_0x1b0 & 0x40) != 0) && ((*puVar5 & 0x200) == 0)) && ((~this->field_0x1b0 & 6) == 6)) {
		this->field_0x1b0 = this->field_0x1b0 & 0xffffff9f;
		(this->cameraConfig).field_0xac = 0.0f;
		(this->cameraConfig).field_0xb0 = (this->cameraConfig).field_0x48.w;
	}

	uVar1 = *puVar5;
	if ((((uVar1 & 0x10) == 0) && ((this->field_0x1b0 & 1) == 0)) && ((uVar1 & 0x400) != 0)) {
		bVar2 = (uVar1 & 0x40000) == 0;

		if ((bVar2) && (bVar2)) {			
			ClampFuncA(puVar5);
		}

		if ((this->field_0x208 - 0.8f < this->field_0x230) &&
			(((this->field_0x1b0 & 0x40) != 0 || (0.001f < (this->cameraConfig).field_0xa8)))) {
			this->field_0x1b0 = this->field_0x1b0 & 0xffffff9f;
			(this->cameraConfig).field_0xa8 = 0.0f;
			(this->cameraConfig).field_0xac = 0.0f;
			(this->cameraConfig).field_0xb0 = (this->cameraConfig).field_0x48.w;
		}
	}
	return;
}

void CCameraGame::SlowVerticalCameraPan()
{
	uint uVar1;
	bool bVar2;
	CActor* pCVar3;
	Timer* pTVar4;
	uint* puVar5;
	float fVar6;
	float fVar7;
	float fVar8;

	if ((((this->field_0x1b0 & 0x80) != 0) && (((this->cameraConfig).flags_0x70 & 0x1000) == 0)) &&
		((~this->field_0x1b0 & 0x18) == 0x18)) {
		this->field_0x1b0 = this->field_0x1b0 & 0xfffffe7f;
		(this->cameraConfig).field_0x90 = 0.0f;
		(this->cameraConfig).field_0x94 = (this->cameraConfig).field_0x38.y;
	}

	uVar1 = (this->cameraConfig).flags_0x70;
	puVar5 = &(this->cameraConfig).flags_0x70;
	if ((((uVar1 & 0x10) == 0) && ((this->field_0x1b0 & 1) == 0)) && ((uVar1 & 0x800) != 0)) {
		bVar2 = (uVar1 & 0x40000) == 0;

		if ((bVar2) && (bVar2)) {
			ClampFuncA(puVar5);
		}

		if ((this->field_0x208 - 0.8f < this->field_0x230) &&
			(((this->field_0x1b0 & 0x80) != 0 || (0.001f < (this->cameraConfig).field_0x8c)))) {
			this->field_0x1b0 = this->field_0x1b0 & 0xfffffe7f;
			(this->cameraConfig).field_0x8c = 0.0f;
			(this->cameraConfig).field_0x90 = 0.0f;
			(this->cameraConfig).field_0x94 = (this->cameraConfig).field_0x38.y;
		}
	}

	if ((((this->field_0x1b0 & 0x100) != 0) && ((*puVar5 & 0x800) == 0)) && ((~this->field_0x1b0 & 0x18) == 0x18)) {
		this->field_0x1b0 = this->field_0x1b0 & 0xfffffe7f;
		(this->cameraConfig).field_0x90 = 0.0f;
		(this->cameraConfig).field_0x94 = (this->cameraConfig).field_0x38.y;
	}

	uVar1 = *puVar5;
	if ((((uVar1 & 0x10) == 0) && ((this->field_0x1b0 & 1) == 0)) && ((uVar1 & 0x1000) != 0)) {
		bVar2 = (uVar1 & 0x40000) == 0;
		if ((bVar2) && (bVar2)) {
			ClampFuncA(puVar5);
		}

		if ((this->field_0x208 - 0.8f < this->field_0x230) &&
			(((this->field_0x1b0 & 0x100) != 0 || ((this->cameraConfig).field_0x8c < -0.001f)))) {
			this->field_0x1b0 = this->field_0x1b0 & 0xfffffe7f;
			(this->cameraConfig).field_0x8c = 0.0f;
			(this->cameraConfig).field_0x90 = 0.0f;
			(this->cameraConfig).field_0x94 = (this->cameraConfig).field_0x38.y;
		}
	}
	return;
}

void CCameraGame::_Manage_Leash()
{
	uint uVar1;
	edF32VECTOR4* peVar2;
	bool bVar3;
	CActor* pCVar4;
	edF32VECTOR4* peVar5;
	ECameraType EVar6;
	CActorHero* pHero;
	edF32VECTOR4* peVar7;
	uint* puVar8;
	float fVar9;
	float fVar10;
	float fVar11;
	float fVar12;
	float fVar13;
	CCollisionRay CStack208;
	edF32VECTOR4 eStack176;
	CCollisionRay CStack160;
	edF32VECTOR4 local_80 = {};
	edF32VECTOR4 eStack112 = {};
	edF32VECTOR4 eStack96 = {};
	edF32MATRIX4 eStack80 = {};
	edF32VECTOR4 eStack16 = {};

	puVar8 = &(this->cameraConfig).flags_0x70;
	fVar9 = (this->cameraConfig).field_0xbc;
	fVar11 = this->field_0x230 - 0.8f;
	this->field_0x1b0 = this->field_0x1b0 & 0xfffff9ff;
	uVar1 = (this->cameraConfig).flags_0x70;
	fVar9 = fVar9 - this->field_0x208;

	if ((uVar1 & 0x100) != 0) {
		ClampFunc(puVar8);

		if (this->field_0x230 == 1e+30f) {
			fVar11 = this->field_0x208;
		}

		puVar8 = &(this->cameraConfig).flags_0x70;
		if (((this->cameraConfig).flags_0x70 & 0x100000) == 0) {
			//peVar5 = (edF32VECTOR4*)&DAT_00000010;
			*puVar8 = *puVar8 | 0x100000;
			//peVar7 = &local_80;
			//peVar2 = peVar7;
			//while (peVar2 != (edF32VECTOR4*)0x0) {
			//	*(undefined*)&peVar7->x = 0;
			//	peVar7 = (edF32VECTOR4*)((int)&peVar7->x + 1);
			//	peVar5 = (edF32VECTOR4*)((int)&peVar5[-1].w + 3);
			//	peVar2 = peVar5;
			//}
			local_80.z = -1.0f;
			edF32Matrix4RotateXHard(this->targetPitch, &eStack80, &gF32Matrix4Unit);
			edF32Matrix4RotateYHard(this->field_0x204, &eStack80, &eStack80);
			edF32Matrix4MulF32Vector4Hard(&local_80, &eStack80, &local_80);
			edF32Vector4ScaleHard(0.8f, &eStack96, &local_80);
			edF32Vector4SubHard(&eStack112, &this->transformationMatrix.rowT, &eStack96);
			CCollisionRay CStack160 = CCollisionRay(1.6f, &eStack112, &local_80);
			pCVar4 = GetTarget();
			fVar10 = CStack160.Intersect(this->cameraRayFlags, pCVar4, (CActor*)0x0, 0x40000004, (edF32VECTOR4*)0x0, (_ray_info_out*)0x0);
			this->field_0x234 = fVar10;
		}

		if ((this->field_0x234 != 1e+30f) && (this->field_0x4a4 != 0)) {
			fVar10 = 0.25f;
			fVar13 = this->field_0x208 - (1.6f - this->field_0x234);

			if (0.25f <= fVar13) {
				fVar10 = fVar13;
			}

			this->field_0x208 = fVar10;
		}

		if (((((this->cameraConfig).flags_0x70 & 0x80) != 0) && (this->mode_0x1bc != 4)) && (this->mode_0x1bc != 3)) {
			EVar6 = GetMode();
			if ((EVar6 == 9) || (EVar6 == 8)) {
				fVar13 = 30.0f;
				fVar10 = 1.0f;
			}
			else {
				if (EVar6 == CT_KyaJamgut) {
					fVar13 = 25.0f;
					fVar10 = 0.1f;
				}
				else {
					fVar13 = 8.1f;
					fVar12 = 1.0f;
					pCVar4 = GetTarget();
					fVar10 = fVar12;
					if (pCVar4->typeID == ACTOR_HERO_PRIVATE) {
						pHero = (CActorHero*)GetTarget();
						if ((((this->cameraConfig).flags_0x70 & 0x2000000) != 0) ||
							(bVar3 = pHero->TestState_IsInCheatMode(), fVar10 = 1.0f, bVar3 != false)) {
							fVar13 = 36.0f;
							fVar10 = fVar12;
						}
					}
					else {
						pCVar4 = GetTarget();
						if (pCVar4->typeID == BOOMY) {
							fVar13 = 18.0f;
						}
					}
				}
			}

			fVar10 = edFIntervalLERP(fabs(fVar9), 0.0f, fVar10, 0.0f, fVar13);
			fVar10 = fVar10 * CCamera::_gpcam_man->time_0x4;
			EVar6 = GetMode();
			if (EVar6 == CT_KyaJamgut) {
				if ((fVar11 < this->field_0x208) || (((this->cameraConfig).flags_0x70 & 0x2000) != 0)) {
					fVar9 = 0.25f;
					this->field_0x1b0 = this->field_0x1b0 | 0x400;
					if (0.25f <= fVar11) {
						fVar9 = fVar11;
					}

					this->field_0x208 = fVar9;
				}
				else {
					fVar9 = (this->cameraConfig).field_0xbc;

					if (fVar10 + this->field_0x208 <= fVar9) {
						fVar9 = 0.25f;
						this->field_0x1b0 = this->field_0x1b0 | 0x400;
						fVar11 = fVar10 * 0.2f + this->field_0x208;
						if (0.25f <= fVar11) {
							fVar9 = fVar11;
						}
						this->field_0x208 = fVar9;
					}
					else {
						fVar11 = 0.25f;
						if (0.25f <= fVar9) {
							fVar11 = fVar9;
						}
						this->field_0x208 = fVar11;
					}
				}
			}
			else {
				if ((((this->cameraConfig).flags & 0x4000) == 0) && (((this->cameraConfig).flags_0x70 & 0x400000) != 0)) {
					fVar12 = (this->cameraConfig).field_0xbc;
					fVar13 = 0.25f;
					if (0.25f <= fVar12) {
						fVar13 = fVar12;
					}
					this->field_0x208 = fVar13;
				}

				puVar8 = &this->field_0x1b0;
				if ((this->field_0x1b0 & 1) == 0) {
					if (((this->field_0x1b0 & 0x1e) == 0) || (((this->cameraConfig).flags_0x70 & 0x2000) == 0)) {
						uVar1 = (this->cameraConfig).flags_0x70;
						if ((uVar1 & 0x2000) == 0) {
							if (fVar10 < fabs(fVar9)) {
								if (this->field_0x238 == 1e+30f) {
									fVar11 = 0.0f;
									if ((uVar1 & 0x800000) != 0) {
										pCVar4 = GetTarget();
										fVar11 = edF32Vector4DotProductHard(&pCVar4->rotationQuat, &this->transformationMatrix.rowZ);
									}
									if ((fVar9 <= 0.0f) || (fVar11 < 0.0f)) {
										this->field_0x1b0 = this->field_0x1b0 | 0x200;

										if (fVar9 < 0.0f) {
											fVar10 = -fVar10;
										}

										fVar9 = 0.25f;
										fVar10 = fVar10 + this->field_0x208;

										if (0.25f <= fVar10) {
											fVar9 = fVar10;
										}

										this->field_0x208 = fVar9;
									}
									else {
										this->field_0x1b0 = this->field_0x1b0 | 0x400;

										if (fVar9 < 0.0f) {
											fVar10 = -fVar10;
										}

										fVar9 = 0.25f;
										fVar10 = fVar10 + this->field_0x208;

										if (0.25f <= fVar10) {
											fVar9 = fVar10;
										}

										this->field_0x208 = fVar9;
									}
								}
								else {
									edF32Vector4SubHard(&eStack16, &this->gameLookAt, &this->transformationMatrix.rowT);
									fVar11 = edF32Vector4GetDistHard(&eStack16);
									if ((0.0f < fVar9) && (fVar9 = 0.25f, fVar11 <= (this->cameraConfig).field_0xbc)) {
										if (0.25f <= fVar11) {
											fVar9 = fVar11;
										}
										this->field_0x208 = fVar9;
									}
								}
							}
							else {
								fVar11 = 0.25f;
								fVar9 = fVar9 + this->field_0x208;
								if (0.25f <= fVar9) {
									fVar11 = fVar9;
								}
								this->field_0x208 = fVar11;
							}
						}
						else {
							if (fVar9 < 0.0f) {
								fVar10 = -fVar10;
							}
							fVar9 = 0.25f;
							fVar10 = fVar10 + this->field_0x208;
							if (0.25f <= fVar10) {
								fVar9 = fVar10;
							}
							this->field_0x208 = fVar9;
						}
					}
					else {
						if (fVar11 - this->field_0x208 < 0.1) {
							fVar9 = 0.25f;
							*puVar8 = *puVar8 | 0x200;
							if (0.25f <= fVar11) {
								fVar9 = fVar11;
							}
							this->field_0x208 = fVar9;
						}
						else {
							if (fVar10 < fabs(fVar11 - (this->cameraConfig).field_0xbc)) {
								if (fVar10 + this->field_0x208 < fVar11) {
									fVar9 = 0.25f;
									*puVar8 = *puVar8 | 0x400;
									fVar10 = fVar10 + this->field_0x208;
									if (0.25f <= fVar10) {
										fVar9 = fVar10;
									}

									this->field_0x208 = fVar9;
								}
								else {
									fVar9 = 0.25f;
									if (0.25f <= fVar11) {
										fVar9 = fVar11;
									}

									this->field_0x208 = fVar9;
								}
							}
						}
					}
				}
			}
		}
	}
	return;
}

void CCameraGame::_After_Manage_Beta()
{
	CActor* pCVar1;
	float fVar3;
	float fVar4;
	edF32VECTOR4 local_10;

	if ((this->cameraConfig).pActorRefB.Get() == (CActor*)0x0) {
		if (GetMode() != CT_Fight) {
			fVar3 = (this->cameraConfig).field_0xa8;
			if (fVar3 < (this->cameraConfig).field_0xac) {
				fVar3 = fVar3 + (this->cameraConfig).field_0xb0 * CCamera::_gpcam_man->time_0x4;
				(this->cameraConfig).field_0xa8 = fVar3;
				fVar4 = (this->cameraConfig).field_0xac;
				if (fVar4 < fVar3) {
					(this->cameraConfig).field_0xa8 = fVar4;
				}
			}
			else {
				fVar3 = fVar3 - (this->cameraConfig).field_0xb0 * CCamera::_gpcam_man->time_0x4;
				(this->cameraConfig).field_0xa8 = fVar3;
				fVar4 = (this->cameraConfig).field_0xac;
				if (fVar3 < fVar4) {
					(this->cameraConfig).field_0xa8 = fVar4;
				}
			}
			fVar3 = (this->cameraConfig).field_0xa8;
			if (0.001f < fabs(fVar3)) {
				fVar3 = edF32Between_0_2Pi(this->field_0x204 + fVar3 * CCamera::_gpcam_man->time_0x4);
				this->field_0x204 = fVar3;
			}
		}
	}
	else {
		pCVar1 = GetTarget();
		edF32Vector4SubHard(&local_10, &((this->cameraConfig).pActorRefB.Get())->currentLocation, &pCVar1->currentLocation);
		fVar3 = GetAngleYFromVector(&local_10);
		fVar3 = edF32GetAnglesDelta(this->field_0x204, fVar3);
		fVar3 = fVar3 * CCamera::_gpcam_man->time_0x4 * 2.0f;

		if (0.05f < fabs(fVar3)) {
			if (0.0f <= fVar3) {
				fVar3 = 0.05f;
			}
			else {
				fVar3 = -0.05f;
			}
		}

		fVar3 = edF32Between_0_2Pi(fVar3 + this->field_0x204);
		this->field_0x204 = fVar3;
	}

	if (((this->cameraConfig).flags & 0x100) != 0) {
		fVar3 = edF32GetAnglesDelta((this->cameraConfig).field_0x38.w, this->field_0x204);
		fVar4 = edF32GetAnglesDelta((this->cameraConfig).field_0x48.x, this->field_0x204);
		if (fVar3 < 0.0f) {
			this->field_0x204 = (this->cameraConfig).field_0x38.w;
		}
		if (0.0f < fVar4) {
			this->field_0x204 = (this->cameraConfig).field_0x48.x;
		}
	}
	return;
}

void CCameraGame::_After_Manage_Alpha()
{
	bool bVar1;
	CActor* pCVar3;
	CActorHero* pHero;
	uint uVar4;
	float fVar5;
	float fVar6;
	float fVar7;
	CCollisionRay CStack112;
	edF32VECTOR4 local_50;
	edF32MATRIX4 eStack64;

	if (GetMode() != CT_Fight) {
		if (GetMode() == CT_KyaWindWall) {
			fVar5 = (this->cameraConfig).field_0x8c;
			if (fVar5 < (this->cameraConfig).field_0x90) {
				fVar5 = fVar5 + (this->cameraConfig).field_0x94 * CCamera::_gpcam_man->time_0x4;
				(this->cameraConfig).field_0x8c = fVar5;
				fVar6 = (this->cameraConfig).field_0x90;

				if (fVar6 < fVar5) {
					(this->cameraConfig).field_0x8c = fVar6;
				}
			}
			else {
				fVar5 = fVar5 - (this->cameraConfig).field_0x94 * CCamera::_gpcam_man->time_0x4;
				(this->cameraConfig).field_0x8c = fVar5;
				fVar6 = (this->cameraConfig).field_0x90;

				if (fVar5 < fVar6) {
					(this->cameraConfig).field_0x8c = fVar6;
				}
			}

			fVar5 = (this->cameraConfig).field_0x8c;
			if (0.001f < fabs(fVar5)) {
				fVar5 = edF32Between_Pi(this->targetPitch + fVar5 * CCamera::_gpcam_man->time_0x4);
				(this->pitchDyn).currentAlpha = fVar5;
				this->targetPitch = fVar5;
			}
		}
		else {
			if (((this->field_0x1b0 & 0x180) == 0) && (((this->cameraConfig).flags_0x70 & 0x1000008) == 0)) {
				if (((this->cameraConfig).flags & 8) == 0) {
					(this->cameraConfig).targetPitch = (this->cameraConfig).baseTargetPitch;
				}
				else {
					pHero = (CActorHero*)0x0;
					pCVar3 = GetTarget();

					if (pCVar3->typeID == ACTOR_HERO_PRIVATE) {
						pHero = (CActorHero*)GetTarget();
					}

					pCVar3 = GetTarget();
					fVar5 = pCVar3->distanceToGround;

					if (fVar5 == 1e+30f) {
						pCVar3 = GetTarget();
						fVar5 = pCVar3->field_0xf0;
					}

					if (((2.0f < fVar5) &&
						((pHero == (CActorHero*)0x0 ||
							(uVar4 = pHero->TestState_IsGrippedOrClimbing(0xffffffff), uVar4 == 0)))) ||
						((pHero != (CActorHero*)0x0 && (bVar1 = pHero->TestState_IsInCheatMode(), bVar1 != false)))) {
						if ((pHero != (CActorHero*)0x0) && (uVar4 = pHero->TestState_IsInTheWind(0xffffffff), uVar4 != 0)) {
							fVar6 = edFIntervalLERP(this->field_0xd0->dynamic.speed, 3.0f, 6.0f, 0.01f, 0.8f);
							fVar7 = (this->cameraConfig).targetPitch;
							(this->cameraConfig).targetPitch = fVar7 + fVar6 * (1.047198f - fVar7);
							this->field_0x240 = fVar5;
							this->field_0x250 = this->lookAt.y;
						}
					}
					else {
						if ((((this->cameraConfig).flags & 0x20) == 0) &&
							((uVar4 = (this->cameraConfig).flags_0x70, (uVar4 & 4) == 0 || ((uVar4 & 0x800000) != 0)))) {
							this->field_0x244 = 1.0472f;
							this->pitchAlpha = 0.2617994f;
							this->field_0x24c = 0.0f;
							this->field_0x240 = fVar5;
							this->field_0x250 = this->lookAt.y;
							fVar5 = (this->cameraConfig).targetPitch;
							(this->cameraConfig).targetPitch = fVar5 + ((this->cameraConfig).baseTargetPitch - fVar5) * 0.025f;
						}
						else {
							this->field_0x240 = fVar5;
							this->field_0x250 = this->lookAt.y;
							fVar5 = this->field_0x23c;

							static edF32VECTOR4 edF32VECTOR4_00425090 = {0.0f, 0.0f, 1.0f, 0.0f};

							local_50 = edF32VECTOR4_00425090;

							edF32Matrix4RotateXHard(this->field_0x244, &eStack64, &gF32Matrix4Unit);
							edF32Matrix4RotateYHard(this->angleBeta_0x1e8, &eStack64, &eStack64);
							edF32Matrix4MulF32Vector4Hard(&local_50, &eStack64, &local_50);
							CCollisionRay CStack112 = CCollisionRay(fVar5 * 4.0f, &this->gameLookAt, &local_50);
							pCVar3 = GetTarget();
							fVar5 = CStack112.Intersect(RAY_FLAG_SCENERY | RAY_FLAG_ACTOR, pCVar3, (CActor*)0x0, 0x40000004, (edF32VECTOR4*)0x0, (_ray_info_out*)0x0);
							if (fVar5 == 1e+30f) {
								fVar5 = this->field_0x23c * 2.0f;
							}

							if (((this->field_0x23c <= fVar5) && (this->pitchAlpha < this->field_0x244)) && (this->field_0x24c < fVar5)) {
								this->pitchAlpha = this->field_0x244;
								this->field_0x24c = fVar5;
							}

							fVar5 = this->field_0x244 - 0.08726646f;
							this->field_0x244 = fVar5;
							fVar6 = (this->cameraConfig).baseTargetPitch;

							if (fVar5 <= fVar6 - 0.08726646f) {
								fVar5 = edFIntervalLERP(this->pitchAlpha, 0.2617994f, 1.0472f, fVar6, 1.0472f);
								(this->cameraConfig).targetPitch = fVar5;
								this->field_0x244 = 1.0472f;
								this->pitchAlpha = 0.2617994f;
								this->field_0x24c = 0.0f;
							}
						}
					}
				}
			}
		}
	}

	return;
}

void CCameraGame::CameraGetWorldTranslation(edF32VECTOR4* outTranslation)
{
	ECameraType EVar1;
	CActor* pAVar2;
	uint* puVar3;
	float fVar4;
	float fVar5;
	float fVar6;
	edF32VECTOR4 aeStack96[6];

	*outTranslation = this->lookAt;

	if (((this->mode_0x1bc == 3) || (this->mode_0x1bc == 4)) ||
		(EVar1 = GetMode(), EVar1 != CT_Main)) {
		fVar5 = (this->cameraConfig).field_0x58.y;
		fVar4 = 0.25f;

		if (this->field_0x208 < fVar5) {
			if (0.25f <= fVar5) {
				fVar4 = fVar5;
			}

			this->field_0x208 = fVar4;
		}
		else {
			fVar5 = (this->cameraConfig).field_0x58.z;
			fVar4 = 0.25f;

			if (fVar5 < this->field_0x208) {
				if (0.25f <= fVar5) {
					fVar4 = fVar5;
				}
				this->field_0x208 = fVar4;
			}
		}
	}
	else {
		fVar5 = (this->cameraConfig).field_0x58.z;
		fVar4 = 0.25f;
		if (fVar5 < this->field_0x208) {
			if (0.25f <= fVar5) {
				fVar4 = fVar5;
			}
			this->field_0x208 = fVar4;
		}
		else {
			fVar5 = (this->cameraConfig).field_0x58.y;
			fVar4 = 0.25f;
			if (this->field_0x208 < fVar5) {
				if (0.25f <= fVar5) {
					fVar4 = fVar5;
				}
				this->field_0x208 = fVar4;
			}
			else {
				puVar3 = &(this->cameraConfig).flags_0x70;
				if (((this->cameraConfig).flags & 0x10) == 0) {
					*puVar3 = *puVar3 & 0xfffffff7;
				}
				else {
					*puVar3 = *puVar3 | 8;
				}
				if (((this->field_0x1b0 & 1) != 0) &&
					(this->field_0x1b0 = this->field_0x1b0 & 0xffffffe1, (~this->field_0x1b0 & 0x1e0) == 0x1e0)) {
					this->field_0x1b0 = this->field_0x1b0 & 0xfffffffe;
				}

				IMPLEMENTATION_GUARD_LOG(
				pAVar2 = (Actor*)GetTarget();
				ActorFunc_00115680(pAVar2);
				pAVar2 = (Actor*)GetTarget();
				ActorFunc_00115650(pAVar2, 0);)

				if (((this->cameraConfig).flags & 4) == 0) {
					*puVar3 = *puVar3 & 0xfffffffb;
				}
				else {
					*puVar3 = *puVar3 | 4;
				}

				this->field_0x4a0 = 0;
				this->field_0x4a4 = 0xffffffff;
			}
		}
	}

	fVar5 = (this->cameraConfig).field_0x58.z;
	fVar4 = this->field_0x208;
	if (fVar5 < fVar4) {
		fVar4 = 0.25f;
		if (0.25f <= fVar5) {
			fVar4 = fVar5;
		}
		this->field_0x208 = fVar4;
		fVar4 = 0.25f;
	}

	CAMERA_LOG(LogLevel::Verbose, "CCameraGame::CameraGetWorldTranslation Current location: {}, LookAt: {}, GameLookAt: {}", this->transformationMatrix.rowT.ToString(), 
		this->lookAt.ToString(), this->gameLookAt.ToString());


	edF32Vector4SubHard(aeStack96, &this->transformationMatrix.rowT, &this->gameLookAt);
	edF32Vector4NormalizeHard(aeStack96, aeStack96);
	edF32Vector4ScaleHard(this->field_0x208, aeStack96, aeStack96);
	edF32Vector4AddHard(aeStack96, aeStack96, &this->gameLookAt);
	edF32Vector4SubHard(aeStack96, &this->lookAt, aeStack96);
	fVar4 = edF32Vector4NormalizeHard(aeStack96, aeStack96);

	this->distance = fVar4;

	if (this->field_0x4a4 == 0) {
		fVar4 = GetAngleXFromVector(aeStack96);
		(this->pitchDyn).currentAlpha = fVar4;
		this->targetPitch = fVar4;
		fVar4 = GetAngleYFromVector(aeStack96);
		this->field_0x204 = fVar4;
	}
	return;
}

void CCameraGame::FillThisFrameExpectedDifferentialMatrix(edF32MATRIX4* pMatrix)
{
	CActor* pCVar1;

	pCVar1 = this->pOtherTarget;
	if (pCVar1 == (CActor*)0x0) {
		*pMatrix = gF32Matrix4Unit;
	}
	else {
		pCVar1->FillThisFrameExpectedDifferentialMatrix(pMatrix);
	}
	return;
}

void CCameraGame::Fight_SetConfig(CCamFigData* pConfig)
{
	if (pConfig == (CCamFigData*)0x0) {
		_b_use_fig_data = 0;
	}
	else {
		if (CCamera::_gpcam_man->pActiveCamera != this) {
			CCamera::_gpcam_man->KeepSameParam(this, 0x37);
		}

		_pfig_data = pConfig;
		pConfig->Cam_SetDefault(1.0f, 2.0f, this);
		_b_use_fig_data = 1;
	}
	return;
}

void CCameraGame::InitFromConfig(CAMERA_CONFIG* pConfig)
{
	bool bVar1;
	ECameraType EVar2;
	CCamConfig* pCVar3;
	uint* puVar4;
	float fVar5;
	float fVar6;
	float fVar7;

	*static_cast<CAMERA_CONFIG*>(&this->cameraConfig) = *pConfig;

	this->cameraConfig.ResetWithConfig();

	this->fov = this->cameraConfig.field_0x18;
	SetTarget(this->cameraConfig.pActorRefA.Get());
	(this->cameraConfig).flags_0x70 = 0;
	this->field_0x1ac = 0;
	this->field_0x1b0 = 0;
	this->field_0x1b4 = 0;
	puVar4 = &(this->cameraConfig).flags_0x70;

	if ((this->cameraConfig.flags & 0x100) == 0) {
		bVar1 = this->cameraConfig.field_0x38.w != 0.0f;
		if ((!bVar1) && (bVar1 = true, this->cameraConfig.field_0x48.x == 0.0f)) {
			bVar1 = false;
		}
		pCVar3 = &this->cameraConfig;
		if (bVar1) {
			pCVar3->flags = pCVar3->flags | 0x100;
		}
		else {
			pCVar3->flags = pCVar3->flags & 0xfffffeff;
		}
	}

	pCVar3 = &this->cameraConfig;
	if (this->cameraConfig.field_0x1c == 0.0) {
		pCVar3->flags = pCVar3->flags & 0xfffffdff;
	}
	else {
		pCVar3->flags = pCVar3->flags | 0x200;
	}

	EVar2 = GetMode();
	if ((EVar2 == CT_Main) && (this->field_0x8 == -100)) {
		this->cameraConfig.flags = this->cameraConfig.flags | 2;
		this->cameraConfig.flags = this->cameraConfig.flags | 0x80;
		this->cameraConfig.flags = this->cameraConfig.flags | 0x800;
	}

	*puVar4 = *puVar4 | 0x100;
	if ((this->cameraConfig.flags & 2) == 0) {
		*puVar4 = *puVar4 & 0xffffffdf;
	}
	else {
		*puVar4 = *puVar4 | 0x20;
	}
	if ((this->cameraConfig.flags & 0x80) == 0) {
		*puVar4 = *puVar4 & 0xffffffbf;
	}
	else {
		*puVar4 = *puVar4 | 0x40;
	}
	if ((this->cameraConfig.flags & 0x800) == 0) {
		*puVar4 = *puVar4 & 0xffffff7f;
	}
	else {
		*puVar4 = *puVar4 | 0x80;
	}
	if ((this->cameraConfig.flags & 4) == 0) {
		*puVar4 = *puVar4 & 0xfffffffb;
	}
	else {
		*puVar4 = *puVar4 | 4;
	}
	if ((this->cameraConfig.flags & 0x10) == 0) {
		*puVar4 = *puVar4 & 0xfffffff7;
	}
	else {
		*puVar4 = *puVar4 | 8;
	}
	return;
}

void CCameraGame::UpdateTarget(edF32VECTOR4* v0, bool doSomething)
{
	ECameraType EVar1;
	CActor* pActorA;
	CActor* pCVar2;
	CActor* pActorB;
	CActorHero* pHero;
	uint uVar3;
	long lVar4;
	float fVar5;
	float fVar6;
	float fVar7;

	fVar5 = 1.0;
	ComputeTargetOffset(v0);
	EVar1 = GetMode();
	if (((EVar1 == CT_Main) &&
		(pActorA = GetTarget(), pActorA->typeID == ACTOR_HERO_PRIVATE)) && ((this->modeFlags & 1) == 0)) {
		pHero = (CActorHero*)GetTarget();
		lVar4 = pHero->IsFightRelated(pHero->curBehaviourId);
		if (lVar4 != 0) {
			v0->x = 0.0f;
			v0->y = 1.4f;
			v0->z = 0.0f;
		}
	}
	EVar1 = GetMode();
	if (((EVar1 == 9) && ((this->flags_0xc & 1) != 0)) && (pActorB = GetTarget(), pActorB->typeID == ACTOR_HERO_PRIVATE)) {
		v0->x = 0.0f;
		v0->y = 0.72f;
		v0->z = 0.0f;
	}

	if (((this->flags_0xc & 1) == 0) && (pCVar2 = GetTarget(), pCVar2->typeID == ACTOR_HERO_PRIVATE)) {
		pHero = (CActorHero*)GetTarget();
		if (pHero->TestState_IsCrouched(0xffffffff) != 0) {
			v0->y = 0.7f;
		}
	}

	fVar7 = (this->cameraConfig).field_0x58.x;
	if (this->field_0x208 < fVar7 - 0.001f) {
		fVar5 = edFIntervalUnitDstLERP(this->field_0x208, (this->cameraConfig).field_0x58.y, fVar7);
		v0->x = v0->x * fVar5;
		v0->z = v0->z * fVar5;
	}
	EVar1 = GetMode();
	if (EVar1 == CT_KyaJamgut) {
		fVar5 = 1.0f;
	}
	else {
		if (this->field_0x208 < (this->cameraConfig).field_0x58.x - 0.001f) {
			fVar5 = edFIntervalUnitSrcLERP(fVar5, 0.5f, 0.05f);
		}
		else {
			fVar5 = 0.05f;
		}

		EVar1 = GetMode();
		if ((((EVar1 == CT_Main) && (v0->y = v0->y - 0.15f, ((this->cameraConfig).flags & 0x20000) != 0)) &&
			((this->modeFlags & 1) != 0)) && ((this->modeFlags & 2) == 0)) {
			v0->x = 0.0f;
			v0->z = 0.0f;
		}
	}

	if (doSomething != false) {
		edF32Vector4LERPHard(fVar5, v0, &this->field_0x2a0, v0);
		this->field_0x2a0 = *v0;
	}
	return;
}

bool CCameraGame::Manage()
{
	bool bValidValue;
	CActor* local_v0_lo_96;
	CActor* pActorA;
	CActor* pActorB;
	CBehaviour* pCVar1;
	CActor* pActorC;
	CActor* pActorD;
	CActor* pActorG;
	CActor* pActorF;
	CActor* pActorE;
	float floatC;
	Timer* pTVar3;
	CActor* pActorH;
	CActorHero* this_00;
	uint uVar4;
	CActor* pActorI;
	long lVar5;
	uint* pFlag;
	float floatA;
	float floatB;
	CCollisionRay CStack656;
	edF32VECTOR4 eStack624;
	CCollisionRay CStack608;
	edF32VECTOR4 eStack576;
	CCollisionRay CStack560;
	edF32VECTOR4 eStack528;
	CCollisionRay CStack512;
	edF32VECTOR4 eStack480;
	CCollisionRay CStack464;
	edF32VECTOR4 eStack432;
	CCollisionRay CStack416;
	edF32VECTOR4 eStack384;
	edF32VECTOR4 eStack336;
	CCollisionRay CStack320;
	edF32VECTOR4 eStack288;
	CCollisionRay CStack272;
	edF32VECTOR4 eStack240;
	edF32VECTOR4 vectorI;
	edF32VECTOR4 vectorG;
	edF32VECTOR4 vectorF;
	edF32VECTOR4 outTranslation;
	edF32VECTOR4 eStack128;
	edF32VECTOR4 eStack112;
	edF32VECTOR4 vectorE;
	edF32VECTOR4 vectorH;
	edF32VECTOR4 vectorC;
	edF32VECTOR4 vectorB;
	edF32VECTOR4 vectorA;
	edF32VECTOR4 vectorD;
	int camMode;

	pActorB = GetTarget();
	if (pActorB == (CActor*)0x0) {
		bValidValue = false;
	}
	else {
		bValidValue = CCamera::Manage();
		if (bValidValue == false) {
			this->cameraRayFlags = RAY_FLAG_SCENERY | RAY_FLAG_ACTOR;

			local_v0_lo_96 = GetTarget();
			if ((local_v0_lo_96 != (CActor*)0x0) &&
				(pActorA = GetTarget(), pActorA->typeID == ACTOR_HERO_PRIVATE)) {
				pActorB = GetTarget();

				if (pActorB->curBehaviourId == 8) {
					/* Generic camera does not go in here (Val is 7) */
					pCVar1 = pActorB->GetBehaviour(pActorB->curBehaviourId);
					IMPLEMENTATION_GUARD(
					*(CBehaviourVtable**)&this->field_0x1ec = pCVar1[0x23].pVTable;)
				}
				else {
					this->field_0x1ec = (CActorHeroPrivate*)0x0;
				}

				pFlag = &(this->cameraConfig).flags_0x70;
				if (pActorB->curBehaviourId == 8) {
					*pFlag = *pFlag | 0x2000000;
				}
				else {
					*pFlag = *pFlag & 0xfdffffff;
				}
			}

			GetMode();
			UpdateTarget(&vectorA, true);
			pActorC = GetTarget();
			floatA = GetAngleYFromVector(&pActorC->rotationQuat);
			this->angleBeta_0x1e8 = floatA;
			UpdateMode();
			FUN_002bee80();

			camMode = this->mode_0x1bc;
			if ((camMode == 4) || (camMode == 3)) {
				/* Generic camera does not go in here (Val is 1) */
				ComputeTargetPosition(&vectorB);
				_Manage_TargetPos(&vectorB);
				FUN_002bff30(&vectorC, &vectorB, &vectorA);
				this->lookAt = vectorC;
			}
			else {
				if ((camMode == 2) || (camMode == 1)) {
					ComputeTargetPosition(&vectorB);

					if (GetMode() == CT_KyaJamgut) {
						/* Generic camera does not go in here (Val is 3) */
						edF32Vector4AddHard(&this->gameLookAt, &vectorB, &vectorA);
					}
					else {
						this->gameLookAt = vectorB;
						(this->gameLookAt).y = (this->gameLookAt).y + vectorA.y;
					}

					edF32Vector4AddHard(&vectorC, &vectorB, &vectorA);

					if (this->mode_0x1bc == 2) {
						/* Generic camera does not go in here (Val is 1) */
						this->lookAt = vectorC;
					}
					else {
						floatA = _Manage_TargetPos(&vectorC);
						(this->gameLookAt).y = (this->gameLookAt).y - floatA;
					}
					edF32Vector4SubHard(&vectorD, &this->gameLookAt, &this->transformationMatrix.rowT);
					floatB = edF32Vector4GetDistHard(&vectorD);
					floatA = 0.25f;
					if (0.25f <= floatB) {
						floatA = floatB;
					}
					this->field_0x208 = floatA;
					if (floatA < 0.025f) {
						/* Generic camera does not go in here (Val is 4.000009) */
						pActorD = GetTarget();
						vectorD = pActorD->rotationQuat;
						this->field_0x208 = 0.25f;
						pActorG = GetTarget();
						pActorF = GetTarget();
						edF32Vector4SubHard(&vectorH, &pActorF->currentLocation, &pActorG->rotationQuat);
						this->transformationMatrix.rowT = vectorH;
					}

					if (GetMode() != 9) {
						/* Generic camera does go in here (Val is 3) */
						floatA = GetAngleYFromVector(&vectorD);
						this->field_0x204 = floatA;
					}
				}
			}

			FUN_002c8160();

			if (GetMode() == CT_Main) {
				/* Generic camera does go in here (Val is 3) */
				UpdateTarget(&vectorE, true);
				vectorE.z = 0.0f;
				vectorE.x = 0.0f;
				pActorE = GetTarget();
				/* Adjust camera Y vector */
				edF32Vector4AddHard(&vectorE, &vectorE, &pActorE->currentLocation);
				camMode = g_CameraVectorBase.cameraNum;

				g_CameraVectorBase.aCameraLocations[0] = this->transformationMatrix.rowT;

				g_CameraVectorBase.field_0x400 = g_CameraVectorBase.aCameraLocations[g_CameraVectorBase.cameraNum];
				g_CameraVectorBase.aCameraLocations[g_CameraVectorBase.cameraNum] = vectorE;

				edF32Vector4SubHard(&vectorF, g_CameraVectorBase.aCameraLocations + g_CameraVectorBase.cameraNum, &g_CameraVectorBase.field_0x400);
				edF32Vector4ScaleHard(30.0f, &vectorF, &vectorF);
				floatA = edF32Vector4DotProductHard(&vectorF, &vectorF);
				if ((1.0f < floatA) && (g_CameraVectorBase.cameraNum != 2)) {
					g_CameraVectorBase.field_0x410 = vectorF;
				}
			}

			edF32Vector4SubHard(&vectorG, &lookAt, &transformationMatrix.rowT);

			pFlag = &(this->cameraConfig).flags_0x70;
			if ((~(this->cameraConfig).flags_0x70 & 0x1000008) == 0x1000008) {
				vectorG.y = 0.0f;
			}

			edF32Vector4NormalizeHard(&vectorG, &vectorG);

			this->transformationMatrix.rowZ = vectorG;
			this->field_0x1b0 = this->field_0x1b0 & 0xffffffe1;

			IMPLEMENTATION_GUARD_LOG(
			_Manage_Pad(this);
			)

			if (GetMode() == CT_Main) {
				_Normal_ManageIdle();
			}

			_UpdateAngleAlphaData();
			_UpdateAngleBetaData();

			if (GetMode() == CT_Main) {
				/* Generic camera does go in here (Val is 3) */
				bValidValue = (*pFlag & 0x40000) == 0;
				if ((bValidValue) && (bValidValue)) {
					ClampFuncA(pFlag);
				}

				lVar5 = g_CameraVectorBase.FUN_002bf570(this);
				if (lVar5 == 0) {
					*pFlag = *pFlag & 0xffffffef;
				}
				else {
					*pFlag = *pFlag | 0x10;
				}

				if ((*pFlag & 0x10) != 0) {
					const static float FLOAT_00448a74 = 5.0f;

					edF32Vector4SubHard(&eStack112, &g_CameraVectorBase.aCameraLocations[1], &this->transformationMatrix.rowT);
					edF32Vector4NormalizeHard(&eStack112, &eStack112);
					edF32Vector4ScaleHard(FLOAT_00448a74 * CCamera::_gpcam_man->time_0x4, &eStack112, &eStack112);
					edF32Vector4AddHard(&eStack128, (edF32VECTOR4*)&this->transformationMatrix.da, &eStack112);
					edF32Vector4SubHard(&eStack112, &this->gameLookAt, &eStack128);
					floatB = edF32Vector4GetDistHard(&eStack112);
					floatA = 0.25f;
					if (0.25f <= floatB) {
						floatA = floatB;
					}

					this->field_0x208 = floatA;
					floatA = GetAngleXFromVector(&eStack112);
					this->pitchDyn.currentAlpha = floatA;
					this->targetPitch = floatA;
					floatA = GetAngleYFromVector(&eStack112);
					this->field_0x204 = floatA;
				}
			}

			camMode = this->field_0x1b8;
			if (camMode == 2) {
				IMPLEMENTATION_GUARD(
				if ((*pFlag & 0x4000) == 0) {
					floatA = -1.570796;
					*pFlag = *pFlag | 0x4000;
					floatB = (this->cameraConfig).field_0xbc + 0.8;
					if (-1.570796 <= this->targetPitch) {
						floatA = this->targetPitch;
					}
					floatC = 1.570796;
					if (floatA <= 1.570796) {
						floatC = floatA;
					}
					floatA = edF32Between_0_2Pi(this->field_0x204 + -0.1963495f);
					CCollisionRay::CCollisionRay(floatC, floatA, floatB, &CStack464, &this->gameLookAt, &eStack432);
					edF32Vector4ScaleHard(-1.0f, &eStack432, &eStack432);
					pActorB = GetTarget();
					floatA = CCollisionRay::Intersect(&CStack464, this->cameraRayFlags, pActorB, (CActor*)0x0, 0x40000004, (edF32VECTOR4*)0x0,
						(_ray_info_out*)0x0);
					this->field_0x220 = floatA;
				}
				if (this->field_0x208 + 0.8 <= this->field_0x220) {
					*pFlag = *pFlag & 0xfffffdff;
				}
				else {
					*pFlag = *pFlag | 0x200;
				}
				if ((*pFlag & 0x8000) == 0) {
					floatA = -1.570796;
					*pFlag = *pFlag | 0x8000;
					floatB = (this->cameraConfig).field_0xbc + 0.8;
					if (-1.570796 <= this->targetPitch) {
						floatA = this->targetPitch;
					}
					floatC = 1.570796;
					if (floatA <= 1.570796) {
						floatC = floatA;
					}
					floatA = edF32Between_0_2Pi(this->field_0x204 + 0.1963495f);
					CCollisionRay::CCollisionRay(floatC, floatA, floatB, &CStack512, &this->gameLookAt, &eStack480);
					edF32Vector4ScaleHard(-1.0f, &eStack480, &eStack480);
					pActorB = GetTarget();
					floatA = CCollisionRay::Intersect(&CStack512, this->cameraRayFlags, pActorB, (CActor*)0x0, 0x40000004, (edF32VECTOR4*)0x0,
						(_ray_info_out*)0x0);
					this->field_0x224 = floatA;
				}
				if (this->field_0x208 + 0.8 <= this->field_0x224) {
					*pFlag = *pFlag & 0xfffffbff;
				}
				else {
					*pFlag = *pFlag | 0x400;
				}
				uVar4 = *pFlag;
				if ((uVar4 & 0x40) != 0) {
					if (((uVar4 & 0x10) == 0) && ((this->field_0x1b0 & 1) == 0)) {
						if (((uVar4 & 0x400) != 0) && ((uVar4 & 0x200) == 0)) {
							floatA = this->cameraConfig.field_0x48.y;
							this->field_0x1b0 = this->field_0x1b0 | 0x20;
							this->field_0x1b0 = this->field_0x1b0 & 0xffffffbf;
							(this->cameraConfig).field_0xac = floatA * -1.0;
							(this->cameraConfig).field_0xb0 = this->cameraConfig.field_0x48.z;
						}
						if (((*pFlag & 0x200) != 0) && ((*pFlag & 0x400) == 0)) {
							floatA = this->cameraConfig.field_0x48.y;
							this->field_0x1b0 = this->field_0x1b0 | 0x40;
							this->field_0x1b0 = this->field_0x1b0 & 0xffffffdf;
							(this->cameraConfig).field_0xac = floatA * 1.0;
							(this->cameraConfig).field_0xb0 = this->cameraConfig.field_0x48.z;
						}
					}
					if ((this->field_0x1b0 & 0x60) != 0) {
						this->field_0x1b8 = 0;
					}
				}
				CameraFunc_002c5b50();
				if ((*pFlag & 0x40000) == 0) {
					ClampFuncA(pFlag);
				}
				if (this->field_0x230 == 1e+30f) {
					*pFlag = *pFlag & 0xffffdfff;
				}
				else {
					*pFlag = *pFlag | 0x2000;
				})
			}
			else {
				if (camMode == 0) {
					/* Generic camera goes in here (camera type B is 0) */
					if ((*pFlag & 0x4000) == 0) {
						floatA = -1.570796f;
						*pFlag = *pFlag | 0x4000;
						floatB = (this->cameraConfig).field_0xbc + 0.8f;
						if (-1.570796f <= this->targetPitch) {
							floatA = this->targetPitch;
						}
						floatC = 1.570796f;
						if (floatA <= 1.570796f) {
							floatC = floatA;
						}
						floatA = edF32Between_0_2Pi(this->field_0x204 + -0.1963495f);
						CCollisionRay CStack368 = CCollisionRay(floatC, floatA, floatB, &this->gameLookAt, &eStack336);
						edF32Vector4ScaleHard(-1.0f, &eStack336, &eStack336);
						pActorB = GetTarget();
						floatA = CStack368.Intersect(this->cameraRayFlags, pActorB, (CActor*)0x0, 0x40000004, (edF32VECTOR4*)0x0, (_ray_info_out*)0x0);
						this->field_0x220 = floatA;
					}

					if (this->field_0x208 + 0.8f <= this->field_0x220) {
						*pFlag = *pFlag & 0xfffffdff;
					}
					else {
						*pFlag = *pFlag | 0x200;
					}

					if ((*pFlag & 0x8000) == 0) {
						floatA = -1.570796f;
						*pFlag = *pFlag | 0x8000;
						floatB = (this->cameraConfig).field_0xbc + 0.8f;
						if (-1.570796f <= this->targetPitch) {
							floatA = this->targetPitch;
						}
						floatC = 1.570796f;
						if (floatA <= 1.570796f) {
							floatC = floatA;
						}
						floatA = edF32Between_0_2Pi(this->field_0x204 + 0.1963495f);
						CCollisionRay CStack416 = CCollisionRay(floatC, floatA, floatB, &this->gameLookAt, &eStack384);
						edF32Vector4ScaleHard(-1.0f, &eStack384, &eStack384);
						pActorB = GetTarget();
						floatA = CStack416.Intersect(this->cameraRayFlags, pActorB, (CActor*)0x0, 0x40000004, (edF32VECTOR4*)0x0, (_ray_info_out*)0x0);
						this->field_0x224 = floatA;
					}

					if (this->field_0x208 + 0.8f <= this->field_0x224) {
						*pFlag = *pFlag & 0xfffffbff;
					}
					else {
						*pFlag = *pFlag | 0x400;
					}

					uVar4 = *pFlag;
					if ((uVar4 & 0x40) != 0) {
						if (((uVar4 & 0x10) == 0) && ((this->field_0x1b0 & 1) == 0)) {
							if (((uVar4 & 0x400) != 0) && ((uVar4 & 0x200) == 0)) {
								floatA = (this->cameraConfig).field_0x48.y;
								this->field_0x1b0 = this->field_0x1b0 | 0x20;
								this->field_0x1b0 = this->field_0x1b0 & 0xffffffbf;
								(this->cameraConfig).field_0xac = floatA * -1.0f;
								(this->cameraConfig).field_0xb0 = (this->cameraConfig).field_0x48.z;
							}

							if (((*pFlag & 0x200) != 0) && ((*pFlag & 0x400) == 0)) {
								floatA = (this->cameraConfig).field_0x48.y;
								this->field_0x1b0 = this->field_0x1b0 | 0x40;
								this->field_0x1b0 = this->field_0x1b0 & 0xffffffdf;
								(this->cameraConfig).field_0xac = floatA * 1.0f;
								(this->cameraConfig).field_0xb0 = (this->cameraConfig).field_0x48.z;
							}
						}

						if ((this->field_0x1b0 & 0x60) != 0) {
							this->field_0x1b8 = 0;
						}
					}

					if ((*pFlag & 0x40000) == 0) {
						ClampFuncA(pFlag);
					}
					if (this->field_0x230 == 1e+30f) {
						*pFlag = *pFlag & 0xffffdfff;
					}
					else {
						*pFlag = *pFlag | 0x2000;
					}

					/* Does not do anything for generic camera */
					CameraFunc_002c5b50();
				}
				else {
					if (camMode == 1) {
						CameraFunc_002c5b50();

						if ((*pFlag & 0x4000) == 0) {
							floatA = -1.570796f;
							*pFlag = *pFlag | 0x4000;
							floatB = (this->cameraConfig).field_0xbc + 0.8f;

							if (-1.570796f <= this->targetPitch) {
								floatA = this->targetPitch;
							}

							floatC = 1.570796f;
							if (floatA <= 1.570796f) {
								floatC = floatA;
							}

							floatA = edF32Between_0_2Pi(this->field_0x204 + -0.1963495f);
							CCollisionRay CStack272 = CCollisionRay(floatC, floatA, floatB, &this->gameLookAt, &eStack240);
							edF32Vector4ScaleHard(-1.0f, &eStack240, &eStack240);
							pActorB = GetTarget();
							floatA = CStack272.Intersect(this->cameraRayFlags, pActorB, (CActor*)0x0, 0x40000004, (edF32VECTOR4*)0x0, (_ray_info_out*)0x0);
							this->field_0x220 = floatA;
						}

						if (this->field_0x208 + 0.8f <= this->field_0x220) {
							*pFlag = *pFlag & 0xfffffdff;
						}
						else {
							*pFlag = *pFlag | 0x200;
						}

						if ((*pFlag & 0x8000) == 0) {
							floatA = -1.570796f;
							*pFlag = *pFlag | 0x8000;
							floatB = (this->cameraConfig).field_0xbc + 0.8f;

							if (-1.570796f <= this->targetPitch) {
								floatA = this->targetPitch;
							}

							floatC = 1.570796f;
							if (floatA <= 1.570796f) {
								floatC = floatA;
							}

							floatA = edF32Between_0_2Pi(this->field_0x204 + 0.1963495f);
							CCollisionRay CStack320 = CCollisionRay(floatC, floatA, floatB, &this->gameLookAt, &eStack288);
							edF32Vector4ScaleHard(-1.0f, &eStack288, &eStack288);
							pActorB = GetTarget();
							floatA = CStack320.Intersect(this->cameraRayFlags, pActorB, (CActor*)0x0, 0x40000004, (edF32VECTOR4*)0x0, (_ray_info_out*)0x0);
							this->field_0x224 = floatA;
						}

						if (this->field_0x208 + 0.8f <= this->field_0x224) {
							*pFlag = *pFlag & 0xfffffbff;
						}
						else {
							*pFlag = *pFlag | 0x400;
						}

						uVar4 = *pFlag;

						if ((uVar4 & 0x40) != 0) {
							if (((uVar4 & 0x10) == 0) && ((this->field_0x1b0 & 1) == 0)) {
								if (((uVar4 & 0x400) != 0) && ((uVar4 & 0x200) == 0)) {
									floatA = this->cameraConfig.field_0x48.y;
									this->field_0x1b0 = this->field_0x1b0 | 0x20;
									this->field_0x1b0 = this->field_0x1b0 & 0xffffffbf;
									(this->cameraConfig).field_0xac = floatA * -1.0f;
									(this->cameraConfig).field_0xb0 = this->cameraConfig.field_0x48.z;
								}

								if (((*pFlag & 0x200) != 0) && ((*pFlag & 0x400) == 0)) {
									floatA = this->cameraConfig.field_0x48.y;
									this->field_0x1b0 = this->field_0x1b0 | 0x40;
									this->field_0x1b0 = this->field_0x1b0 & 0xffffffdf;
									(this->cameraConfig).field_0xac = floatA * 1.0f;
									(this->cameraConfig).field_0xb0 = this->cameraConfig.field_0x48.z;
								}
							}

							if ((this->field_0x1b0 & 0x60) != 0) {
								this->field_0x1b8 = 0;
							}
						}
						if ((*pFlag & 0x40000) == 0) {
							ClampFuncA(pFlag);
						}

						if (this->field_0x230 == 1e+30f) {
							*pFlag = *pFlag & 0xffffdfff;
						}

						else {
							*pFlag = *pFlag | 0x2000;
						}
					}
				}
			}

			SlowHorizontalCameraPan();
			SlowVerticalCameraPan();

			if ((this->flags_0xc & 0x30) == 0) {
				/* Generic camera goes in here */
				pActorI = GetTarget();
				if (pActorI->typeID != 5) {
					floatA = GetAngleGamma();
					floatB = GetAngleGamma();
					SetAngleGamma(floatB + -floatA * 0.1f);
				}
			}
			else {
				pActorH = GetTarget();
				if ((pActorH->typeID == ACTOR_HERO_PRIVATE) && (floatA = GetAngleGamma(), floatA != 0.0f)) {
					this_00 = (CActorHero*)GetTarget();
					uVar4 = this_00->TestState_IsFlying(0xffffffff);
					if (uVar4 == 0) {
						floatA = GetAngleGamma();
						floatB = GetAngleGamma();
						SetAngleGamma(floatB + -floatA * 0.1f);
					}
				}
			}

			_Manage_Leash();
			_After_Manage_Beta();
			_After_Manage_Alpha();
			CameraGetWorldTranslation(&outTranslation);
			_UpdateCameraData(&outTranslation);

			this->field_0x1ac = *pFlag;
			this->field_0x1b4 = this->field_0x1b0;
			if ((this->mode_0x1bc == 3) || (this->mode_0x1bc == 4)) {
				*pFlag = this->flags_0x2e0 | *pFlag & 0xffffff9f;
			}

			if (GetMode() != CT_Fight) {
				*pFlag = *pFlag & 0xffe03fff;
			}

			bValidValue = true;
		}
		else {
			bValidValue = false;
		}
	}

	return bValidValue;
}
