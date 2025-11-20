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
	this->pathFollowIndex = pByteCode->GetS32();
	this->flags_0xc = this->flags_0xc | pByteCode->GetU32() << 10;
	this->aRailSegments = (CameraRailSegment*)0x0;

	return;
}

ECameraType CCameraRail::GetMode()
{
	return CT_Rail;
}

void CCameraRail::Init()
{
	CameraRailSegment* pInnerSegment;
	CameraRailSegment* pSegmentIt;
	edF32VECTOR4* pTangent;
	int iVar4;
	CameraRailSegment* pCurSegment;
	int iVar7;
	CPathFollow* pPathFollow;
	float fVar9;

	CCameraRailVirtual::Init();

	if (this->pathFollowIndex == -1) {
		pPathFollow = (CPathFollow*)0x0;
	}
	else {
		pPathFollow = (CScene::ptable.g_PathManager_004516a0)->aPathFollow + this->pathFollowIndex;
	}

	this->nbSegments = pPathFollow->splinePointCount;
	this->aRailSegments = new CameraRailSegment[this->nbSegments];
	iVar4 = this->nbSegments;
	iVar7 = 0;
	if (0 < iVar4) {
		do {
			pCurSegment = this->aRailSegments + iVar7;

			if (pPathFollow->aSplinePoints == (edF32VECTOR4*)0x0) {
				pTangent = &gF32Vertex4Zero;
			}
			else {
				pTangent = pPathFollow->aSplinePoints + iVar7;
			}
			(pCurSegment->position).x = pTangent->x;

			if (pPathFollow->aSplinePoints == (edF32VECTOR4*)0x0) {
				pTangent = &gF32Vertex4Zero;
			}
			else {
				pTangent = pPathFollow->aSplinePoints + iVar7;
			}
			(pCurSegment->position).y = pTangent->y;

			if (pPathFollow->aSplinePoints == (edF32VECTOR4*)0x0) {
				pTangent = &gF32Vertex4Zero;
			}
			else {
				pTangent = pPathFollow->aSplinePoints + iVar7;
			}
			(pCurSegment->position).z = pTangent->z;

			if (pPathFollow->aSplinePoints == (edF32VECTOR4*)0x0) {
				pTangent = &gF32Vertex4Zero;
			}
			else {
				pTangent = pPathFollow->aSplinePoints + iVar7;
			}
			(pCurSegment->position).w = pTangent->w;

			fVar9 = (float)iVar7;
			pCurSegment->alpha = fVar9 / (float)(this->nbSegments + -1);
			iVar4 = this->nbSegments;
			iVar7 = iVar7 + 1;
		} while (iVar7 < iVar4);
	}

	// First segment tangent (index 0)
	pSegmentIt = this->aRailSegments;
	int iVar5 = iVar4 + -1;
	edF32Vector4SubHard(&pSegmentIt->tangent, &pSegmentIt[1].position, &pSegmentIt->position);
	edF32Vector4NormalizeHard(&pSegmentIt->tangent, &pSegmentIt->tangent);

	// Middle segments (index 1 to nbSegments-2)
	iVar7 = 1;
	if (1 < iVar5) {
		pInnerSegment = pSegmentIt;
		do {
			pTangent = &pInnerSegment[1].tangent;
			edF32Vector4SubHard(pTangent, &pSegmentIt[iVar7 + 1].position, &pSegmentIt[iVar7 - 1].position);
			edF32Vector4NormalizeHard(pTangent, pTangent);

			iVar7 = iVar7 + 1;
			pInnerSegment = pInnerSegment + 1;
		} while (iVar7 < iVar5);
	}

	// Last segment tangent (index nbSegments-1)
	pTangent = &pSegmentIt[iVar5].tangent;
	edF32Vector4SubHard(pTangent, &pSegmentIt[iVar5].position, &pSegmentIt[iVar4 + -2].position);
	edF32Vector4NormalizeHard(pTangent, pTangent);

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

			fVar5 = this->distanceToPlaneA / (this->distanceToPlaneA - this->distanceToPlaneB);

			if ((this->flags_0xc & 0x400) != 0) {
				edF32Vector4LERPHard(fVar5, &this->lookAt, &this->aRailSegments[this->currentSegmentIndex].position, &this->aRailSegments[this->currentSegmentIndex + 1].position);
				this->lookAt.w = 1.0f;
			}

			fVar4 = this->aRailSegments[this->currentSegmentIndex].alpha;
			this->pSpline->GetPosition(fVar5 * (this->aRailSegments[this->currentSegmentIndex + 1].alpha - fVar4) + fVar4, &local_10);
			this->transformationMatrix.rowT = local_10;

			if ((this->flags_0xc & 0x400) == 0) {
				this->lookAt = this->railTargetPosition;
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
			edF32Vector4SubHard(&eStack16, &pCVar1->currentLocation, &this->aRailSegments[uVar3].position);
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

	this->currentSegmentIndex = unaff_s2_lo;

	ComputeTargetPosition(&this->railTargetPosition);
	ComputeTargetOffset(&eStack32);
	edF32Vector4AddHard(&this->railTargetPosition, &this->railTargetPosition, &eStack32);

	_UpdateTargetPosWithPlane();

	return;
}

void CCameraRail::_UpdateTargetPosWithPlane()
{
	bool bVar1;
	int processedSegments;
	uint currentIndex;
	float fVar4;
	edF32VECTOR4 computedOffset;

	currentIndex = this->currentSegmentIndex;

	processedSegments = 0;
	ComputeTargetPosition(&this->railTargetPosition);
	ComputeTargetOffset(&computedOffset);
	edF32Vector4AddHard(&this->railTargetPosition, &this->railTargetPosition, &computedOffset);

	do {
		this->distanceToPlaneA = -edDistPointToPlane(&this->aRailSegments[currentIndex].position, &this->aRailSegments[currentIndex].tangent, &this->railTargetPosition);
		this->distanceToPlaneB = -edDistPointToPlane(&this->aRailSegments[currentIndex + 1].position, &this->aRailSegments[currentIndex + 1].tangent, &this->railTargetPosition);
		bVar1 = false;

		if (0.0f < this->distanceToPlaneA) {
			if (currentIndex < 1) {
				this->distanceToPlaneA = -1e-20f;
			}
			else {
				currentIndex = currentIndex - 1;
				bVar1 = true;
			}
		}

		if (this->distanceToPlaneB < 0.0f) {
			if (currentIndex + 2 < this->nbSegments) {
				currentIndex = currentIndex + 1;
				bVar1 = true;
			}
			else {
				this->distanceToPlaneB = 1e-20f;
			}
		}

		processedSegments = processedSegments + 1;
		if (this->nbSegments < processedSegments) {
			bVar1 = false;
		}
	} while (bVar1);

	this->currentSegmentIndex = currentIndex;

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
