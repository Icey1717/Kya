#include "CameraFixePerso.h"
#include "MathOps.h"
#include "CameraViewManager.h"
#include "ActorManager.h"
#include "MemoryStream.h"

CCameraFixePerso::CCameraFixePerso()
{
	CCameraExt();
	this->flags_0xc = this->flags_0xc | 0xb7c;
	this->switchMode = SWITCH_MODE_B;
	this->field_0x98 = 0.0f;
}

CCameraFixePerso::CCameraFixePerso(ByteCode* pByteCode)
	: CCameraExt(pByteCode)
{
	CActorManager* pCVar1;
	int iVar2;
	uint uVar3;
	CActor* pCVar4;
	uint* puVar5;

	pCVar1 = CScene::ptable.g_ActorManager_004516a4;
	iVar2 = pByteCode->GetS32();
	pCVar4 = (CActor*)0x0;
	if (iVar2 != -1) {
		pCVar4 = pCVar1->aActors[iVar2];
	}

	SetTarget(pCVar4);

	uVar3 = pByteCode->GetU32();
	puVar5 = &this->flags_0xc;
	if ((uVar3 & 1) == 0) {
		*puVar5 = *puVar5 & 0xfffffbff;
	}
	else {
		*puVar5 = *puVar5 | 0x400;
	}

	puVar5 = &this->flags_0xc;
	if ((uVar3 & 2) == 0) {
		*puVar5 = *puVar5 & 0xfffff7ff;
	}
	else {
		*puVar5 = *puVar5 | 0x800;
	}

	return;
}

ECameraType CCameraFixePerso::GetMode()
{
	return (ECameraType)0x13;
}

void CCameraFixePerso::Init()
{
	float fVar1;
	int iVar2;

	CCameraExt::Init();
	this->field_0xe0 = this->transformationMatrix;
	
	this->field_0xd4 = 0;
	this->field_0xd0 = 0;
	this->field_0xd8 = 0.0f;
	return;
}

static float FLOAT_00448614 = 0.01f;
static float FLOAT_00448618 = 0.05;
static float FLOAT_0044861c = 3.0f;
static float FLOAT_00448620 = 0.4f;

bool CCameraFixePerso::Manage()
{
	bool bVar1;
	CActor* pCVar2;
	StateConfig* pSVar3;
	uint uVar4;
	edF32VECTOR4* v0;
	float fVar5;
	float fVar6;
	edF32MATRIX4 eStack96;
	edF32VECTOR4 local_20;
	edF32VECTOR4 eStack16;

	if ((this->flags_0xc & 0x800000) == 0) {
		this->flags_0xc = this->flags_0xc | 0x800000;
		ComputeTargetPosition(&this->lookAt);
		ComputeTargetOffset(&eStack16);

		if ((this->flags_0xc & 0x400) != 0) {
			uVar4 = 0;
			pCVar2 = GetTarget();
			fVar6 = pCVar2->distanceToGround;
			fVar5 = fabs(fVar6 - this->field_0xd8);

			pCVar2 = GetTarget();
			bVar1 = pCVar2->IsKindOfObject(4);
			if (bVar1 != false) {
				uVar4 = GetTarget()->GetStateFlags(GetTarget()->actorState) & 0x100;
			}

			if ((uVar4 != 0) ||
				(pCVar2 = GetTarget(), pCVar2->field_0xf0 <= fVar6)) {
				fVar6 = 0.0f;
			}

			if (uVar4 == 0) {
				if (FLOAT_0044861c < fVar6) {
					fVar6 = this->field_0xd8;
					this->field_0xd0 = 1;
				}
				else {
					if (FLOAT_00448620 < fVar5) {
						fVar6 = 0.0;
						this->field_0xd0 = 1;
					}
				}
			}

			if (this->field_0xd0 == 0) {
				this->field_0xd8 = fVar6;
			}
			else {
				this->field_0xd8 = this->field_0xd8 + FLOAT_00448618 * (fVar6 - this->field_0xd8);
				if (fabs(this->field_0xd8 - fVar6) <= FLOAT_00448614) {
					this->field_0xd0 = 0;
				}
			}
			this->field_0xd4 = uVar4;

			this->lookAt.y = this->lookAt.y - this->field_0xd8;
		}

		v0 = &this->lookAt;
		edF32Vector4AddHard(v0, v0, &eStack16);
		edF32Vector4SubHard(&eStack16, &this->lookAt, &this->transformationMatrix.rowT);
		fVar5 = edF32Vector4NormalizeHard(&eStack16, &eStack16);
		SetDistance(fVar5);
		GetAnglesFromVector(&local_20.xyz, &eStack16);
		SetAngleAlpha(local_20.x);
		SetAngleBeta(local_20.y);

		fVar5 = GetDistance();
		if (15.0f < fVar5) {
			edF32Vector4ScaleHard(15.0f, &eStack16, &eStack16);
			edF32Vector4AddHard(&this->lookAt, &this->transformationMatrix.rowT, &eStack16);
			SetDistance(15.0f);
		}

		pCVar2 = this->pOtherTarget;
		if (pCVar2 != (CActor*)0x0) {
			pCVar2->SV_ComputeDiffMatrixFromInit(&eStack96);
			edF32Matrix4MulF32Matrix4Hard(&this->transformationMatrix, &this->field_0xe0, &eStack96);
			FUN_00199b80();
		}
	}

	return true;
}

bool CCameraFixePerso::AlertCamera(int alertType, int param_3, CCamera* param_4)
{
	bool bVar1;

	if (alertType == 2) {
		this->field_0xd4 = 0;
		this->field_0xd0 = 0;
		this->field_0xd8 = 0.0f;

		if ((this->flags_0xc & 0x800) != 0) {
			CCamera::_gpcam_man->KeepSameParam(this, 0x7f);
			return true;
		}
	}

	bVar1 = CCamera::AlertCamera(alertType, param_3, param_4);

	return bVar1;
}
