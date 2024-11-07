#include "CameraRail.h"
#include "PathFollow.h"
#include "PathManager.h"
#include "MathOps.h"
#include "ActorManager.h"
#include "ActorMovable.h"
#include "TimeController.h"
#include "CameraViewManager.h"

CCameraRailVirtual::CCameraRailVirtual(ByteCode* pByteCode)
	: CCameraExt(pByteCode)
{
	CActorManager* pCVar1;
	int iVar2;
	CActor* pCVar3;

	pCVar1 = CScene::ptable.g_ActorManager_004516a4;
	iVar2 = pByteCode->GetS32();
	if (iVar2 == -1) {
		pCVar3 = (CActor*)0x0;
	}
	else {
		pCVar3 = pCVar1->aActors[iVar2];
	}


	this->pActorView = pCVar3;
	this->pathFollowIndex = pByteCode->GetS32();
	this->pSpline = (CedMathTCBSpline*)0x0;
}

void CCameraRailVirtual::Init()
{
	uint pointCount;
	CActor* pCVar1;
	edF32VECTOR4* v0;
	uint uVar2;
	CPathFollow* pCVar3;
	float fVar4;
	edF32VECTOR4 local_10;

	CCameraExt::Init();
	if (this->pathFollowIndex == -1) {
		pCVar3 = (CPathFollow*)0x0;
	}
	else {
		pCVar3 = (CScene::ptable.g_PathManager_004516a0)->aPathFollow + this->pathFollowIndex;
	}

	pointCount = pCVar3->splinePointCount;
	
	this->pSpline = new CedMathTCBSpline(pointCount, 0);

	uVar2 = 0;
	if (pointCount != 0) {
		do {
			if (pCVar3->aSplinePoints == (edF32VECTOR4*)0x0) {
				v0 = &gF32Vertex4Zero;
			}
			else {
				v0 = pCVar3->aSplinePoints + uVar2;
			}

			this->pSpline->SetKeyPosition(uVar2, v0);
			this->pSpline->SetKeyTCB(0, 0, 0, uVar2);

			uVar2 = uVar2 + 1;
		} while (uVar2 < pointCount);
	}

	this->pSpline->PreComputeSpline(1);

	pCVar1 = GetTarget();
	if (pCVar1 != (CActor*)0x0) {
		ComputeTargetPosition(&local_10);

		fVar4 = this->field_0x50.y;
		this->lookAt.x = local_10.x;
		this->lookAt.y = local_10.y + fVar4;
		this->lookAt.z = local_10.z;
		this->lookAt.w = local_10.w;
	}

	return;
}

CCameraRail::CCameraRail(ByteCode* pByteCode)
	: CCameraRailVirtual(pByteCode)
{
	this->field_0xe0 = pByteCode->GetS32();
	this->flags_0xc = this->flags_0xc | pByteCode->GetU32() << 10;
	this->field_0xf4 = (CameraRailSubObj*)0x0;

	return;
}

ECameraType CCameraRail::GetMode()
{
	return CT_Rail;
}

void CCameraRail::Init()
{
	CameraRailSubObj* pCVar1;
	CameraRailSubObj* pCVar2;
	edF32VECTOR4* peVar3;
	int iVar4;
	CameraRailSubObj* pfVar7;
	int iVar7;
	CPathFollow* pCVar8;
	float fVar9;

	CCameraRailVirtual::Init();

	if (this->field_0xe0 == -1) {
		pCVar8 = (CPathFollow*)0x0;
	}
	else {
		pCVar8 = (CScene::ptable.g_PathManager_004516a0)->aPathFollow + this->field_0xe0;
	}

	this->field_0xf0 = pCVar8->splinePointCount;
	this->field_0xf4 = new CameraRailSubObj[this->field_0xf0];
	iVar4 = this->field_0xf0;
	iVar7 = 0;
	if (0 < iVar4) {
		do {
			pfVar7 = this->field_0xf4 + iVar7;

			if (pCVar8->aSplinePoints == (edF32VECTOR4*)0x0) {
				peVar3 = &gF32Vertex4Zero;
			}
			else {
				peVar3 = pCVar8->aSplinePoints + iVar7;
			}
			(pfVar7->field_0x0).x = peVar3->x;

			if (pCVar8->aSplinePoints == (edF32VECTOR4*)0x0) {
				peVar3 = &gF32Vertex4Zero;
			}
			else {
				peVar3 = pCVar8->aSplinePoints + iVar7;
			}
			(pfVar7->field_0x0).y = peVar3->y;

			if (pCVar8->aSplinePoints == (edF32VECTOR4*)0x0) {
				peVar3 = &gF32Vertex4Zero;
			}
			else {
				peVar3 = pCVar8->aSplinePoints + iVar7;
			}
			(pfVar7->field_0x0).z = peVar3->z;

			if (pCVar8->aSplinePoints == (edF32VECTOR4*)0x0) {
				peVar3 = &gF32Vertex4Zero;
			}
			else {
				peVar3 = pCVar8->aSplinePoints + iVar7;
			}
			(pfVar7->field_0x0).w = peVar3->w;
			fVar9 = (float)iVar7;
			pfVar7->field_0x20 = fVar9 / (float)(this->field_0xf0 + -1);
			iVar4 = this->field_0xf0;
			iVar7 = iVar7 + 1;
		} while (iVar7 < iVar4);
	}

	pCVar2 = this->field_0xf4;
	int iVar5 = iVar4 + -1;
	edF32Vector4SubHard(&pCVar2->field_0x10, &pCVar2[1].field_0x0, &pCVar2->field_0x0);
	edF32Vector4NormalizeHard(&pCVar2->field_0x10, &pCVar2->field_0x10);

	iVar7 = 1;
	if (1 < iVar5) {
		pCVar1 = pCVar2;
		do {
			peVar3 = &pCVar1[1].field_0x10;
			edF32Vector4SubHard(peVar3, &pCVar2[iVar7 + 1].field_0x0, &pCVar2[iVar7 - 1].field_0x0);
			edF32Vector4NormalizeHard(peVar3, peVar3);
			iVar7 = iVar7 + 1;
			pCVar1 = pCVar1 + 1;
		} while (iVar7 < iVar5);
	}

	peVar3 = &pCVar2[iVar5].field_0x10;
	edF32Vector4SubHard(peVar3, &pCVar2[iVar5].field_0x0, &pCVar2[iVar4 + -2].field_0x0);
	edF32Vector4NormalizeHard(peVar3, peVar3);
	return;
}

bool CCameraRail::Manage()
{
	bool bVar1;
	CActor* pCVar2;
	void* pvVar3;
	float fVar4;
	float fVar5;
	edF32VECTOR4 local_10;

	pCVar2 = GetTarget();
	if (pCVar2 == (CActor*)0x0) {
		bVar1 = false;
	}
	else {
		bVar1 = CCamera::Manage();
		if (bVar1 == false) {
			IMPLEMENTATION_GUARD(
			_UpdateTargetPosWithPlane((long)(int)this);
			fVar5 = this->field_0xe8 / (this->field_0xe8 - this->field_0xec);
			if ((this->camera.flags_0xc & 0x400) != 0) {
				edF32Vector4LERPHard
				(fVar5, &this->camera.lookAt,
					(edF32VECTOR4*)((int)this->field_0xf4 + *(int*)&this->field_0xe4 * 0x30),
					(edF32VECTOR4*)((int)this->field_0xf4 + (*(int*)&this->field_0xe4 + 1) * 0x30));
				this->camera.lookAt.w = 1.0;
			}
			pvVar3 = (void*)(*(int*)&this->field_0xe4 * 0x30 + (int)this->field_0xf4);
			fVar4 = *(float*)((int)pvVar3 + 0x20);
			CedMathTCBSpline::GetPosition
			(fVar5 * (*(float*)((int)pvVar3 + 0x50) - fVar4) + fVar4, this->pSpline, &local_10);
			this->camera.transformationMatrix.da = local_10.x;
			this->camera.transformationMatrix.db = local_10.y;
			this->camera.transformationMatrix.dc = local_10.z;
			this->camera.transformationMatrix.dd = local_10.w;
			if ((this->camera.flags_0xc & 0x400) == 0) {
				this->camera.lookAt.x = *(float*)&this->field_0x100;
				this->camera.lookAt.y = *(float*)&this->field_0x104;
				this->camera.lookAt.z = *(float*)&this->field_0x108;
				this->camera.lookAt.w = *(float*)&this->field_0x10c;
			}
			edF32Vector4SubHard(&local_10, &this->camera.lookAt,
				(edF32VECTOR4*)&this->camera.transformationMatrix.da);
			fVar5 = edF32Vector4NormalizeHard(&local_10, &local_10);
			(*(code*)(this->camera.objBase.pVTable)->SetDistance)(fVar5, (long)(int)this);
			this->camera.transformationMatrix.ca = local_10.x;
			this->camera.transformationMatrix.cb = local_10.y;
			this->camera.transformationMatrix.cc = local_10.z;
			this->camera.transformationMatrix.cd = local_10.w;
			fVar5 = CCamera::GetAngleAlpha((CCamera*)this);
			(*(this->camera.objBase.pVTable)->SetAngleAlpha)(fVar5, (CCamera*)this);
			fVar5 = CCamera::GetAngleBeta((CCamera*)this);
			(*(this->camera.objBase.pVTable)->SetAngleBeta)(fVar5, (CCamera*)this);
			bVar1 = true;)
		}
		else {
			bVar1 = false;
		}
	}

	return bVar1;
}

bool CCameraRail::AlertCamera(int alertType, int param_3, CCamera* param_4)
{
	bool bVar1;

	bVar1 = CCamera::AlertCamera(alertType, param_3, param_4);
	if (alertType == 2) {
		IMPLEMENTATION_GUARD(
		Alert();)
	}

	return bVar1;
}

CCameraRailSimple::CCameraRailSimple(ByteCode* pByteCode)
	: CCameraRailVirtual(pByteCode)
{
	uint uVar4;

	uVar4 = pByteCode->GetU32();

	this->field_0xe0.x = pByteCode->GetF32();
	this->field_0xe0.y = pByteCode->GetF32();
	this->field_0xe0.z = pByteCode->GetF32();
	this->field_0xe0.w = pByteCode->GetF32();

	if ((uVar4 & 1) == 0) {
		this->flags_0xc = this->flags_0xc & 0xfffffbff;
	}
	else {
		this->flags_0xc = this->flags_0xc | 0x400;
	}

	return;
}

ECameraType CCameraRailSimple::GetMode()
{
	return CT_RailSimple;
}

bool CCameraRailSimple::Manage()
{
	bool bVar1;
	CActor* pCVar3;
	CActorMovable* pCVar2;
	Timer* pTVar4;
	edF32VECTOR4* v0;
	float fVar5;
	float fVar6;
	float fVar7;
	edF32VECTOR4 local_20;
	edF32VECTOR4 eStack16;

	pCVar3 = GetTarget();
	if (pCVar3 == (CActor*)0x0) {
		bVar1 = false;
	}
	else {
		bVar1 = CCamera::Manage();
		if (bVar1 == false) {
			pCVar2 = reinterpret_cast<CActorMovable*>(GetTarget());
			ComputeTargetPosition(&this->lookAt);
			ComputeTargetOffset(&local_20);

			if ((this->flags_0xc & 0x400) != 0) {
				pCVar3 = GetTarget();
				this->lookAt.y = this->lookAt.y - pCVar3->distanceToGround;
			}

			v0 = &this->lookAt;
			edF32Vector4AddHard(v0, v0, &local_20);
			edF32Vector4SubHard(&local_20, &this->lookAt, &this->transformationMatrix.rowT);
			fVar5 = edF32Vector4NormalizeHard(&local_20, &local_20);
			SetDistance(fVar5);
			this->transformationMatrix.rowZ = local_20;

			this->pSpline->GetFirstDerivative(this->field_0xf8, &eStack16);

			this->speedDyn.UpdateLerp((pCVar2->dynamic).linearAcceleration);

			fVar6 = edF32Vector4DotProductHard(&this->transformationMatrix.rowZ, &eStack16);
			fVar7 = (this->speedDyn).field_0x0;
			fVar5 = this->pSpline->GetRoughLength();
			fVar6 = fVar6 * (fVar7 / fVar5);
			fVar5 = GetDistance();
			if (fVar5 < (this->field_0xe0).x) {
				fVar5 = GetDistance();
				fVar6 = fVar6 * (this->field_0xe0).z * ((this->field_0xe0).x - fVar5);
			}
			else {
				fVar5 = GetDistance();
				if ((this->field_0xe0).y < fVar5) {
					fVar5 = GetDistance();
					fVar6 = fVar6 * (this->field_0xe0).w * ((this->field_0xe0).y - fVar5);
				}
			}

			pTVar4 = GetTimer();
			fVar5 = this->field_0xf8 - fVar6 * pTVar4->cutsceneDeltaTime;
			this->field_0xf8 = fVar5;
			if (1.0f < fVar5) {
				this->field_0xf8 = 1.0f;
			}
			else {
				if (fVar5 < 0.0f) {
					this->field_0xf8 = 0.0f;
				}
			}

			this->pSpline->GetPosition(this->field_0xf8, &local_20);

			this->transformationMatrix.rowT = local_20;

			edF32Vector4SubHard(&local_20, &this->lookAt, &this->transformationMatrix.rowT);
			fVar5 = edF32Vector4NormalizeHard(&local_20, &local_20);
			SetDistance(fVar5);
			this->transformationMatrix.rowZ = local_20;

			SetAngleAlpha(GetAngleAlpha());
			SetAngleBeta(GetAngleBeta());

			bVar1 = true;
		}
		else {
			bVar1 = false;
		}
	}

	return bVar1;
}

bool CCameraRailSimple::AlertCamera(int alertType, int param_3, CCamera* param_4)
{
	bool bVar1;

	if (alertType == 2) {
		CCamera::_gpcam_man->KeepSameParam(this, 0x77);

		OnAlertCamera();
	}

	bVar1 = CCamera::AlertCamera(alertType, param_3, param_4);
	return bVar1;
}

void CCameraRailSimple::OnAlertCamera()
{
	int iVar1;
	CActor* pCVar2;
	edF32VECTOR4* peVar3;
	CPathFollow* pCVar4;
	float fVar5;
	float fVar6;
	float fVar7;
	float fVar8;
	float fVar9;
	float puVar10;
	edF32VECTOR4 eStack48;
	edF32VECTOR4 eStack32;
	edF32VECTOR4 eStack16;

	iVar1 = this->pathFollowIndex;
	if (iVar1 == -1) {
		pCVar4 = (CPathFollow*)0x0;
	}
	else {
		pCVar4 = (CScene::ptable.g_PathManager_004516a0)->aPathFollow + iVar1;
	}

	peVar3 = pCVar4->aSplinePoints;
	puVar10 = 1.0f;
	fVar9 = 0.0f;
	if (peVar3 == (edF32VECTOR4*)0x0) {
		peVar3 = &gF32Vertex4Zero;
	}

	edF32Vector4SubHard(&eStack16, &this->lookAt, peVar3);

	fVar5 = edF32Vector4GetDistHard(&eStack16);
	if (pCVar4->aSplinePoints == (edF32VECTOR4*)0x0) {
		peVar3 = &gF32Vertex4Zero;
	}
	else {
		peVar3 = pCVar4->aSplinePoints + pCVar4->splinePointCount + -1;
	}

	edF32Vector4SubHard(&eStack16, &this->lookAt, peVar3);
	fVar6 = edF32Vector4GetDistHard(&eStack16);
	if (fVar6 < fVar5) {
		puVar10 = -1.0f;
		fVar9 = 1.0f;
	}

	fVar5 = fVar9;
	fVar6 = 99999.0f;
	do {
		this->pSpline->GetPosition(fVar5, &eStack32);

		edF32Vector4SubHard(&eStack32, &this->lookAt, &eStack32);
		fVar7 = edF32Vector4GetDistHard(&eStack32);
		fVar5 = fVar5 + puVar10 * 0.05f;

		if (((fVar7 <= (this->field_0xe0).y) || (fVar5 < 0.0f)) || (fVar8 = fVar7 - fVar6, 1.0f < fVar5)) break;

		fVar6 = fVar7;
	} while (fVar8 < 0.0f);

	this->field_0xf8 = fVar5 - puVar10 * 0.05f;

	this->pSpline->GetPosition(this->field_0xf8, &eStack16);
	edF32Vector4SubHard(&eStack16, &this->lookAt, &eStack16);

	pCVar2 = GetTarget();
	fVar6 = edF32Vector4DotProductHard(&eStack16, &pCVar2->rotationQuat);
	fVar5 = 0.0f;
	if (fVar6 < 0.0f) {
		if (fVar9 < 0.5f) {
			fVar5 = 1.0f;
		}

		fVar6 = -puVar10 * 0.05f;
		fVar9 = 99999.0f;
		while (true) {
			this->pSpline->GetPosition(fVar5, &eStack48);
			edF32Vector4SubHard(&eStack48, &this->lookAt, &eStack48);
			fVar7 = edF32Vector4GetDistHard(&eStack48);
			fVar5 = fVar5 + fVar6;

			if ((fVar7 <= (this->field_0xe0).y) || (fVar5 < 0.0f)) break;
			fVar8 = fVar7 - fVar9;

			if ((1.0f < fVar5) || (fVar9 = fVar7, 0.0f <= fVar8)) break;
		}
		this->field_0xf8 = fVar5 - fVar6;
	}

	this->speedDyn.Init(0.0f, 30.0f);

	return;
}
