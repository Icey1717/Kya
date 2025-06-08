#include "CameraRail.h"

#include <algorithm>

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
			_UpdateTargetPosWithPlane();

			fVar5 = this->field_0xe8 / (this->field_0xe8 - this->field_0xec);

			if ((this->flags_0xc & 0x400) != 0) {
				edF32Vector4LERPHard(fVar5, &this->lookAt, &this->field_0xf4[this->field_0xe4].field_0x0, &this->field_0xf4[this->field_0xe4 + 1].field_0x0);
				this->lookAt.w = 1.0f;
			}

			fVar4 = this->field_0xf4[this->field_0xe4].field_0x20;
			this->pSpline->GetPosition(fVar5 * (this->field_0xf4[this->field_0xe4 + 1].field_0x20 - fVar4) + fVar4, &local_10);
			this->transformationMatrix.rowT = local_10;

			if ((this->flags_0xc & 0x400) == 0) {
				this->lookAt = this->field_0x100;
			}

			edF32Vector4SubHard(&local_10, &this->lookAt, &this->transformationMatrix.rowT);
			SetDistance(edF32Vector4NormalizeHard(&local_10, &local_10));
			this->transformationMatrix.rowZ = local_10;

			SetAngleAlpha(CCamera::GetAngleAlpha());
			SetAngleBeta(CCamera::GetAngleBeta());
			bVar1 = true;
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
		Alert();
	}

	return bVar1;
}

void CCameraRail::Alert()
{
	CActor* pCVar1;
	uint uVar3;
	uint unaff_s2_lo;
	uint uVar4;
	float fVar5;
	float fVar6;
	edF32VECTOR4 eStack32;
	edF32VECTOR4 eStack16;

	uVar3 = 0;
	if ((this->pSpline)->nbPoints != 1) {
		fVar6 = 3.402823e+38f;
		uVar4 = unaff_s2_lo;
		do {
			pCVar1 = GetTarget();
			edF32Vector4SubHard(&eStack16, &pCVar1->currentLocation, &this->field_0xf4[uVar3].field_0x0);
			fVar5 = edF32Vector4GetDistHard(&eStack16);
			unaff_s2_lo = uVar3;
			if (fVar6 <= fVar5) {
				fVar5 = fVar6;
				unaff_s2_lo = uVar4;
			}
			uVar3 = uVar3 + 1;
			fVar6 = fVar5;
			uVar4 = unaff_s2_lo;
		} while (uVar3 < (this->pSpline)->nbPoints - 1);
	}

	this->field_0xe4 = unaff_s2_lo;

	ComputeTargetPosition(&this->field_0x100);
	ComputeTargetOffset(&eStack32);
	edF32Vector4AddHard(&this->field_0x100, &this->field_0x100, &eStack32);

	_UpdateTargetPosWithPlane();

	return;
}

void CCameraRail::_UpdateTargetPosWithPlane()
{
	bool bVar1;
	int iVar2;
	uint uVar3;
	float fVar4;
	edF32VECTOR4 eStack16;

	uVar3 = this->field_0xe4;
	iVar2 = 0;
	ComputeTargetPosition(&this->field_0x100);
	ComputeTargetOffset(&eStack16);
	edF32Vector4AddHard(&this->field_0x100, &this->field_0x100, &eStack16);

	do {
		fVar4 = edDistPointToPlane(&this->field_0xf4[uVar3].field_0x0, &this->field_0xf4[uVar3].field_0x10, &this->field_0x100);
		this->field_0xe8 = -fVar4;
		fVar4 = edDistPointToPlane(&this->field_0xf4[uVar3 + 1].field_0x0, &this->field_0xf4[uVar3 + 1].field_0x10, &this->field_0x100);
		this->field_0xec = -fVar4;
		bVar1 = false;

		if (0.0f < this->field_0xe8) {
			if ((int)uVar3 < 1) {
				this->field_0xe8 = -1e-20;
			}
			else {
				uVar3 = uVar3 - 1;
				bVar1 = true;
			}
		}

		if (this->field_0xec < 0.0f) {
			if ((int)(uVar3 + 2) < this->field_0xf0) {
				uVar3 = uVar3 + 1;
				bVar1 = true;
			}
			else {
				this->field_0xec = 1e-20f;
			}
		}

		iVar2 = iVar2 + 1;
		if (this->field_0xf0 < iVar2) {
			bVar1 = false;
		}
	} while (bVar1);

	this->field_0xe4 = uVar3;

	return;
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
	bool bHandled;
	CActorMovable* pMovable;
	Timer* pTVar4;
	edF32VECTOR4* v0;
	float fVar5;
	float fVar6;
	float fVar7;

	if (GetTarget() == (CActor*)0x0) {
		bHandled = false;
	}
	else {
		bHandled = CCamera::Manage();
		if (bHandled == false) {
			pMovable = reinterpret_cast<CActorMovable*>(GetTarget());
			ComputeTargetPosition(&this->lookAt);

			edF32VECTOR4 targetOffset;
			ComputeTargetOffset(&targetOffset);

			CAMERA_LOG(LogLevel::Verbose, "CameraRailSimple::Manage targetOffset {}", targetOffset.ToString());

			if ((this->flags_0xc & 0x400) != 0) {
				this->lookAt.y = this->lookAt.y - GetTarget()->distanceToGround;
			}

			edF32Vector4AddHard(&this->lookAt, &this->lookAt, &targetOffset);

			edF32VECTOR4 newDelta;
			edF32Vector4SubHard(&newDelta, &this->lookAt, &this->transformationMatrix.rowT);
			SetDistance(edF32Vector4NormalizeHard(&newDelta, &newDelta));
			this->transformationMatrix.rowZ = newDelta;

			edF32VECTOR4 splineDerivative;
			this->pSpline->GetFirstDerivative(this->splineAlpha, &splineDerivative);

			this->speedDyn.UpdateLerp((pMovable->dynamic).linearAcceleration);

			fVar6 = edF32Vector4DotProductHard(&this->transformationMatrix.rowZ, &splineDerivative);
			fVar6 = fVar6 * ((this->speedDyn).currentAlpha / this->pSpline->GetRoughLength());

			if (GetDistance() < (this->field_0xe0).x) {
				fVar6 = fVar6 * (this->field_0xe0).z * ((this->field_0xe0).x - GetDistance());
			}
			else {
				if ((this->field_0xe0).y < GetDistance()) {
					fVar6 = fVar6 * (this->field_0xe0).w * ((this->field_0xe0).y - GetDistance());
				}
			}

			this->splineAlpha = this->splineAlpha - fVar6 * GetTimer()->cutsceneDeltaTime;
			this->splineAlpha = std::clamp(this->splineAlpha, 0.0f, 1.0f);

			CAMERA_LOG(LogLevel::Verbose, "CameraRailSimple::Manage splineAlpha {}", this->splineAlpha);

			edF32VECTOR4 splinePointPosition;
			this->pSpline->GetPosition(this->splineAlpha, &splinePointPosition);
			this->transformationMatrix.rowT = splinePointPosition;

			CAMERA_LOG(LogLevel::Verbose, "CameraRailSimple::Manage splinePointPosition {}", splinePointPosition.ToString());

			edF32Vector4SubHard(&newDelta, &this->lookAt, &this->transformationMatrix.rowT);
			SetDistance(edF32Vector4NormalizeHard(&newDelta, &newDelta));
			this->transformationMatrix.rowZ = newDelta;

			SetAngleAlpha(CCamera::GetAngleAlpha());
			SetAngleBeta(CCamera::GetAngleBeta());

			bHandled = true;
		}
		else {
			bHandled = false;
		}
	}

	return bHandled;
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

	this->splineAlpha = fVar5 - puVar10 * 0.05f;

	this->pSpline->GetPosition(this->splineAlpha, &eStack16);
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
		this->splineAlpha = fVar5 - fVar6;
	}

	this->speedDyn.Init(0.0f, 30.0f);

	return;
}
