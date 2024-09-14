#include "Path.h"
#include "MathOps.h"
#include "PathManager.h"

void CPathPlaneArray::Init()
{
	PlaneData* pPVar1;
	CPathFollow* pPathFollow;
	int iVar4;
	int iVar5;
	int iVar6;
	CPathPlane* pCurPathPlane;
	uint uVar8;

	uVar8 = 0;

	if (this->nbPathPlanes != 0) {
		do {
			pCurPathPlane = this->aPathPlanes + uVar8;

			if (pCurPathPlane->pathFollowReader.index == -1) {
				pPathFollow = (CPathFollow*)0x0;
			}
			else {
				pPathFollow = (CScene::ptable.g_PathManager_004516a0)->aPathFollow + pCurPathPlane->pathFollowReader.index;
			}

			pCurPathPlane->pathFollowReader.pPathFollow = pPathFollow;

			if (pCurPathPlane->pathFollowReader.pPathFollow != (CPathFollow*)0x0) {
				pPVar1 = new PlaneData[pCurPathPlane->pathFollowReader.pPathFollow->splinePointCount];
				pCurPathPlane->outData.aPlaneData = pPVar1;

				iVar6 = pCurPathPlane->pathFollowReader.pPathFollow->splinePointCount;
				if (0 < iVar6) {
					iVar4 = 0;
					do {
						iVar5 = iVar4 + 1;
						pCurPathPlane->outData.aPlaneData[iVar4].field_0x10 = (float)iVar4 / (float)(iVar6 + -1);
						iVar6 = pCurPathPlane->pathFollowReader.pPathFollow->splinePointCount;
						iVar4 = iVar5;
					} while (iVar5 < iVar6);
				}

				pCurPathPlane->computePlanesFromKeys(pCurPathPlane->outData.aPlaneData, iVar6);
			}

			uVar8 = uVar8 + 1;
		} while (uVar8 < this->nbPathPlanes);
	}

	return;
}

void CPathPlaneArray::Reset()
{
	CPathPlane* pPathPlane;
	uint uVar3;

	this->curIndex = 0;

	uVar3 = 0;
	if (this->nbPathPlanes != 0) {
		do {
			pPathPlane = this->aPathPlanes + uVar3;
			uVar3 = uVar3 + 1;
			(pPathPlane->pathFollowReader).splinePointIndex = 0;
			(pPathPlane->pathFollowReader).field_0xc = 1;
			pPathPlane->outData.field_0x0 = -1;
			pPathPlane->outData.field_0x4 = 0;
			pPathPlane->outData.field_0x8 = 0;
		} while (uVar3 < this->nbPathPlanes);
	}

	return;
}

int CPathPlaneArray::GetNbPathPlane()
{
	return this->nbPathPlanes;
}

CPathPlane* CPathPlaneArray::GetCurPathPlane()
{
	return this->aPathPlanes + this->curIndex;
}

void CPathPlaneArray::NextWayPoint()
{
	bool bVar1;
	int iVar2;
	int iVar3;
	int iVar4;
	bool bVar5;
	CPathPlane* pPathPlane;
	CPathFollow* pPathFollow;

	bVar5 = true;
	if (this->nbPathPlanes == 1) {
		this->aPathPlanes[this->curIndex].pathFollowReader.NextWayPoint();
		return;
	}

	iVar2 = this->curIndex;
	pPathPlane = this->aPathPlanes + iVar2;
	pPathFollow = (pPathPlane->pathFollowReader).pPathFollow;
	iVar3 = (pPathPlane->pathFollowReader).splinePointIndex;
	iVar4 = (pPathPlane->pathFollowReader).field_0xc;

	if (pPathFollow->type == 0) {
		if (pPathFollow->mode == 1) {
			bVar1 = iVar3 == 0;
			iVar3 = iVar4;
			if (bVar1) {
			joined_r0x001bfde0:
				if (iVar3 == 0) goto LAB_001bfdf8;
			}
		}
		else {
			if (pPathFollow->mode == 0) {
				if (iVar4 == 0) goto joined_r0x001bfde0;
				if (iVar3 + 1 == pPathFollow->splinePointCount) goto LAB_001bfdf8;
			}
		}
		bVar5 = false;
	}
	else {
		bVar5 = false;
	}

LAB_001bfdf8:
	if (!bVar5) {
		pPathPlane->pathFollowReader.NextWayPoint();
		return;
	}

	pPathFollow = (pPathPlane->pathFollowReader).pPathFollow;

	bVar5 = true;
	if (pPathFollow->type == 1) {
		bVar5 = false;
	}
	else {
		if (iVar4 == 0) {
			if (iVar2 == 0) goto LAB_001bfe60;
		}
		else {
			if ((iVar2 == this->nbPathPlanes + -1) && (pPathFollow->mode != 1)) goto LAB_001bfe60;
		}
		bVar5 = false;
	}

LAB_001bfe60:
	if (!bVar5) {
		this->curIndex = this->curIndex + 1;
	}

	return;
}

int CPathPlaneArray::AtGoal()
{
	bool bVar1;
	int iVar2;
	CPathFollow* pCVar3;
	int iVar4;
	bool bVar5;
	CPathPlane* pCVar6;

	iVar2 = this->curIndex;
	pCVar6 = this->aPathPlanes + iVar2;

	if (pCVar6 == (CPathPlane*)0x0) {
		return 0;
	}

	pCVar3 = (pCVar6->pathFollowReader).pPathFollow;
	bVar5 = true;
	if (pCVar3->type == 1) {
		bVar5 = false;
	}
	else {
		if ((pCVar6->pathFollowReader).field_0xc == 0) {
			if (iVar2 != 0) goto LAB_001bff08;
		}
		else {
			if ((iVar2 != this->nbPathPlanes + -1) || (pCVar3->mode == 1)) {
			LAB_001bff08:
				bVar5 = false;
			}
		}
	}

	if (!bVar5) {
		return 0;
	}

	pCVar3 = (pCVar6->pathFollowReader).pPathFollow;
	iVar4 = (pCVar6->pathFollowReader).splinePointIndex;
	iVar2 = (pCVar6->pathFollowReader).field_0xc;

	if (pCVar3->type != 0) {
		bVar5 = false;
		goto LAB_001bffa8;
	}

	bVar5 = true;

	if (pCVar3->mode == 1) {
		bVar1 = iVar4 == 0;
		iVar4 = iVar2;
		if (bVar1) {
		joined_r0x001bff90:
			if (iVar4 == 0) goto LAB_001bffa8;
		}
	}
	else {
		if (pCVar3->mode == 0) {
			if (iVar2 == 0) goto joined_r0x001bff90;
			if (iVar4 + 1 == pCVar3->splinePointCount) goto LAB_001bffa8;
		}
	}

	bVar5 = false;
LAB_001bffa8:
	if (!bVar5) {
		return 0;
	}

	return 1;
}

void CPathPlaneArray::InitPosition(edF32VECTOR4* pPosition)
{
	CPathFollow* pCurPathFollow;
	edF32VECTOR4* pSplinePoint;
	CPathPlane* pPathPlane;
	uint pathPlaneIndex;
	int splinePointIndex;
	uint uVar6;
	uint lastPathPLaneIndex;
	int lastSplinePointIndex;
	int iVar8;
	float minDistance;
	float fVar10;
	edF32VECTOR4 eStack16;

	minDistance = 3.402823e+38f;
	lastPathPLaneIndex = 0;
	pathPlaneIndex = 0;

	if (this->nbPathPlanes != 0) {
		do {
			pCurPathFollow = this->aPathPlanes[pathPlaneIndex].pathFollowReader.pPathFollow;
			splinePointIndex = 0;

			if (0 < pCurPathFollow->splinePointCount) {
				fVar10 = minDistance;
				uVar6 = lastPathPLaneIndex;
				iVar8 = lastSplinePointIndex;

				do {
					if (pCurPathFollow->aSplinePoints == (edF32VECTOR4*)0x0) {
						pSplinePoint = &gF32Vertex4Zero;
					}
					else {
						pSplinePoint = pCurPathFollow->aSplinePoints + splinePointIndex;
					}

					edF32Vector4SubHard(&eStack16, pPosition, pSplinePoint);
					minDistance = edF32Vector4GetDistHard(&eStack16);
					lastPathPLaneIndex = pathPlaneIndex;
					lastSplinePointIndex = splinePointIndex;

					if (fVar10 <= minDistance) {
						minDistance = fVar10;
						lastPathPLaneIndex = uVar6;
						lastSplinePointIndex = iVar8;
					}

					splinePointIndex = splinePointIndex + 1;
					pCurPathFollow = this->aPathPlanes[pathPlaneIndex].pathFollowReader.pPathFollow;
					fVar10 = minDistance;
					uVar6 = lastPathPLaneIndex;
					iVar8 = lastSplinePointIndex;
				} while (splinePointIndex < pCurPathFollow->splinePointCount);
			}

			pathPlaneIndex = pathPlaneIndex + 1;
		} while (pathPlaneIndex < this->nbPathPlanes);
	}

	this->curIndex = lastPathPLaneIndex;
	this->aPathPlanes[this->curIndex].pathFollowReader.field_0xc = 1;
	this->aPathPlanes[this->curIndex].pathFollowReader.splinePointIndex = lastSplinePointIndex;
	pPathPlane = this->aPathPlanes + this->curIndex;

	(pPathPlane->pathFollowReader).field_0x8 = pPathPlane->pathFollowReader.GetPrevPlace((pPathPlane->pathFollowReader).splinePointIndex, (pPathPlane->pathFollowReader).field_0xc);

	return;
}

void CPathPlane::InitTargetPos(edF32VECTOR4* pTargetPos, CPathPlaneOutData* pOutData)
{
	CPathFollow* pPathFollow;
	edF32VECTOR4* v2;
	int iVar3;
	int unaff_s2_lo;
	int iVar4;
	float fVar5;
	float fVar6;
	edF32VECTOR4 eStack16;

	pOutData->field_0x0 = -1;
	pOutData->field_0x4 = 0.0f;
	pOutData->field_0x8 = 0.0f;

	pPathFollow = (this->pathFollowReader).pPathFollow;
	if (pPathFollow != (CPathFollow*)0x0) {
		iVar3 = 0;
		if (0 < pPathFollow->splinePointCount + -1) {
			fVar6 = 3.402823e+38f;
			iVar4 = unaff_s2_lo;
			do {
				if (pPathFollow->aSplinePoints == (edF32VECTOR4*)0x0) {
					v2 = &gF32Vertex4Zero;
				}
				else {
					v2 = pPathFollow->aSplinePoints + iVar3;
				}

				edF32Vector4SubHard(&eStack16, pTargetPos, v2);
				fVar5 = edF32Vector4GetDistHard(&eStack16);
				unaff_s2_lo = iVar3;

				if (fVar6 <= fVar5) {
					fVar5 = fVar6;
					unaff_s2_lo = iVar4;
				}

				pPathFollow = (this->pathFollowReader).pPathFollow;
				iVar3 = iVar3 + 1;
				fVar6 = fVar5;
				iVar4 = unaff_s2_lo;
			} while (iVar3 < pPathFollow->splinePointCount + -1);
		}

		pOutData->field_0x0 = unaff_s2_lo;
		ExternComputeTargetPosWithPlane(pTargetPos, pOutData);
	}

	return;
}

void CPathPlane::ExternComputeTargetPosWithPlane(edF32VECTOR4* pTargetPos, CPathPlaneOutData* pOutData)
{
	CPathFollow* pCVar1;
	int iVar2;
	edF32VECTOR4* peVar3;
	PlaneData* pPVar4;
	int iVar5;
	float fVar6;

	iVar5 = pOutData->field_0x0;

	if ((iVar5 == -1) || (pCVar1 = (this->pathFollowReader).pPathFollow, iVar5 == pCVar1->splinePointCount + -1)) {
		pCVar1 = (this->pathFollowReader).pPathFollow;
		peVar3 = pCVar1->aSplinePoints;
		iVar2 = pCVar1->splinePointCount + -1;
		if (peVar3 == (edF32VECTOR4*)0x0) {
			peVar3 = &gF32Vertex4Zero;
		}
		else {
			peVar3 = peVar3 + iVar2;
		}

		fVar6 = edDistPointToPlane(peVar3, &(this->outData).aPlaneData[iVar2].field_0x0, pTargetPos);
		pOutData->field_0x4 = fVar6;
	}
	else {
		peVar3 = pCVar1->aSplinePoints;

		if (peVar3 == (edF32VECTOR4*)0x0) {
			peVar3 = &gF32Vertex4Zero;
		}
		else {
			peVar3 = peVar3 + iVar5;
		}

		fVar6 = edDistPointToPlane(peVar3, &(this->outData).aPlaneData[iVar5].field_0x0, pTargetPos);
		pOutData->field_0x4 = fVar6;
	}

	if (pOutData->field_0x4 < 0.0f) {
		pCVar1 = (this->pathFollowReader).pPathFollow;
		iVar2 = pCVar1->splinePointCount;
		iVar5 = (iVar5 + -1 + iVar2) % iVar2;

		if (iVar2 == 0) {
			trap(7);
		}

		if ((iVar5 == -1) || (iVar5 == iVar2 + -1)) {
			if (pCVar1->aSplinePoints == (edF32VECTOR4*)0x0) {
				peVar3 = &gF32Vertex4Zero;
			}
			else {
				peVar3 = pCVar1->aSplinePoints + pCVar1->splinePointCount + -1;
			}

			fVar6 = edDistPointToPlane(peVar3, &(this->outData).aPlaneData[iVar2 - 1].field_0x0, pTargetPos);
			pOutData->field_0x4 = fVar6;
		}
		else {
			if (pCVar1->aSplinePoints == (edF32VECTOR4*)0x0) {
				peVar3 = &gF32Vertex4Zero;
			}
			else {
				peVar3 = pCVar1->aSplinePoints + iVar5;
			}

			fVar6 = edDistPointToPlane(peVar3, &(this->outData).aPlaneData[iVar5].field_0x0, pTargetPos);
			pOutData->field_0x4 = fVar6;
		}
	}

	pCVar1 = (this->pathFollowReader).pPathFollow;
	if (iVar5 < pCVar1->splinePointCount + -1) {
		if (pCVar1->aSplinePoints == (edF32VECTOR4*)0x0) {
			peVar3 = &gF32Vertex4Zero;
		}
		else {
			peVar3 = pCVar1->aSplinePoints + iVar5 + 1;
		}

		fVar6 = edDistPointToPlane(peVar3, &(this->outData).aPlaneData[iVar5 + 1].field_0x0, pTargetPos);
		pOutData->field_0x8 = fVar6;
	}
	else {
		peVar3 = pCVar1->aSplinePoints;

		if (peVar3 == (edF32VECTOR4*)0x0) {
			peVar3 = &gF32Vertex4Zero;
			pPVar4 = (this->outData).aPlaneData;
		}
		else {
			pPVar4 = (this->outData).aPlaneData;
		}

		fVar6 = edDistPointToPlane(peVar3, &pPVar4->field_0x0, pTargetPos);
		pOutData->field_0x8 = fVar6;
	}
	if (0.0 < pOutData->field_0x8) {
		pCVar1 = (this->pathFollowReader).pPathFollow;
		iVar2 = pCVar1->splinePointCount;
		iVar5 = (iVar5 + 1) % iVar2;

		if (iVar2 == 0) {
			trap(7);
		}

		if ((iVar5 == -1) || (iVar5 == iVar2 + -1)) {
			if (pCVar1->aSplinePoints == (edF32VECTOR4*)0x0) {
				peVar3 = &gF32Vertex4Zero;
			}
			else {
				peVar3 = pCVar1->aSplinePoints + pCVar1->splinePointCount + -1;
			}

			fVar6 = edDistPointToPlane(peVar3, &(this->outData).aPlaneData[iVar2 - 1].field_0x0, pTargetPos);
			pOutData->field_0x4 = fVar6;

			peVar3 = ((this->pathFollowReader).pPathFollow)->aSplinePoints;
			if (peVar3 == (edF32VECTOR4*)0x0) {
				peVar3 = &gF32Vertex4Zero;
				pPVar4 = (this->outData).aPlaneData;
			}
			else {
				pPVar4 = (this->outData).aPlaneData;
			}

			fVar6 = edDistPointToPlane(peVar3, &pPVar4->field_0x0, pTargetPos);
			pOutData->field_0x8 = fVar6;
		}
		else {
			if (pCVar1->aSplinePoints == (edF32VECTOR4*)0x0) {
				peVar3 = &gF32Vertex4Zero;
			}
			else {
				peVar3 = pCVar1->aSplinePoints + iVar5;
			}

			fVar6 = edDistPointToPlane(peVar3, &(this->outData).aPlaneData[iVar5].field_0x0, pTargetPos);
			pOutData->field_0x4 = fVar6;

			peVar3 = ((this->pathFollowReader).pPathFollow)->aSplinePoints;
			if (peVar3 == (edF32VECTOR4*)0x0) {
				peVar3 = &gF32Vertex4Zero;
			}
			else {
				peVar3 = peVar3 + iVar5 + 1;
			}

			fVar6 = edDistPointToPlane(peVar3, &(this->outData).aPlaneData[iVar5 + 1].field_0x0, pTargetPos);
			pOutData->field_0x8 = fVar6;
		}
	}

	pOutData->field_0x0 = iVar5;

	return;
}

void CPathPlane::computePlanesFromKeys(PlaneData* aPlaneData, int nbPoints)
{
	CPathFollow* pCVar1;
	edF32VECTOR4* peVar2;
	edF32VECTOR4* peVar3;
	PlaneData* pPVar4;
	int iVar5;
	int iVar6;

	pCVar1 = (this->pathFollowReader).pPathFollow;
	iVar6 = nbPoints + -1;

	if ((pCVar1->type == 0) && (pCVar1->mode == 0)) {
		peVar2 = pCVar1->aSplinePoints;
		if (peVar2 == (edF32VECTOR4*)0x0) {
			peVar2 = &gF32Vertex4Zero;
			peVar3 = pCVar1->aSplinePoints;
		}
		else {
			peVar3 = pCVar1->aSplinePoints;
		}

		if (peVar3 == (edF32VECTOR4*)0x0) {
			peVar3 = &gF32Vertex4Zero;
		}
		else {
			peVar3 = peVar3 + 1;
		}

		edF32Vector4SubHard(&aPlaneData->field_0x0, peVar3, peVar2);
		edF32Vector4NormalizeHard(&aPlaneData->field_0x0, &aPlaneData->field_0x0);
	}
	else {
		if (nbPoints < 3) {
			peVar2 = pCVar1->aSplinePoints;
			if (peVar2 == (edF32VECTOR4*)0x0) {
				peVar2 = &gF32Vertex4Zero;
				peVar3 = pCVar1->aSplinePoints;
			}
			else {
				peVar3 = pCVar1->aSplinePoints;
			}

			if (peVar3 == (edF32VECTOR4*)0x0) {
				peVar3 = &gF32Vertex4Zero;
			}
			else {
				peVar3 = peVar3 + 1;
			}

			edF32Vector4SubHard(&aPlaneData->field_0x0, peVar3, peVar2);
			edF32Vector4NormalizeHard(&aPlaneData->field_0x0, &aPlaneData->field_0x0);
		}
		else {
			peVar2 = pCVar1->aSplinePoints;
			if (peVar2 == (edF32VECTOR4*)0x0) {
				peVar3 = &gF32Vertex4Zero;
				peVar2 = &gF32Vertex4Zero;
			}
			else {
				peVar3 = peVar2 + nbPoints + -2;
				peVar2 = peVar2 + 1;
			}

			edF32Vector4SubHard(&aPlaneData->field_0x0, peVar2, peVar3);
			edF32Vector4NormalizeHard(&aPlaneData->field_0x0, &aPlaneData->field_0x0);
		}
	}

	iVar5 = 1;
	if (1 < iVar6) {
		pPVar4 = aPlaneData + 1;
		do {
			peVar2 = ((this->pathFollowReader).pPathFollow)->aSplinePoints;
			if (peVar2 == (edF32VECTOR4*)0x0) {
				peVar3 = &gF32Vertex4Zero;
				peVar2 = &gF32Vertex4Zero;
			}
			else {
				peVar3 = peVar2 + iVar5 + -1;
				peVar2 = peVar2 + iVar5 + 1;
			}

			edF32Vector4SubHard(&pPVar4->field_0x0, peVar2, peVar3);
			edF32Vector4NormalizeHard(&pPVar4->field_0x0, &pPVar4->field_0x0);

			iVar5 = iVar5 + 1;
			pPVar4 = pPVar4 + 1;
		} while (iVar5 < iVar6);
	}

	pCVar1 = (this->pathFollowReader).pPathFollow;
	if ((pCVar1->type == 0) && (pCVar1->mode == 0)) {
		peVar2 = pCVar1->aSplinePoints;
		if (peVar2 == (edF32VECTOR4*)0x0) {
			peVar3 = &gF32Vertex4Zero;
			peVar2 = &gF32Vertex4Zero;
		}
		else {
			peVar3 = peVar2 + nbPoints + -2;
			peVar2 = peVar2 + iVar6;
		}

		pPVar4 = aPlaneData + iVar6;
		edF32Vector4SubHard(&pPVar4->field_0x0, peVar2, peVar3);
		edF32Vector4NormalizeHard(&pPVar4->field_0x0, &pPVar4->field_0x0);
	}
	else {
		if (nbPoints < 3) {
			peVar2 = pCVar1->aSplinePoints;
			if (peVar2 == (edF32VECTOR4*)0x0) {
				peVar3 = &gF32Vertex4Zero;
				peVar2 = &gF32Vertex4Zero;
			}
			else {
				peVar3 = peVar2 + nbPoints + -2;
				peVar2 = peVar2 + iVar6;
			}

			pPVar4 = aPlaneData + iVar6;
			edF32Vector4SubHard(&pPVar4->field_0x0, peVar2, peVar3);
			edF32Vector4NormalizeHard(&pPVar4->field_0x0, &pPVar4->field_0x0);
		}
		else {
			if (pCVar1->aSplinePoints == (edF32VECTOR4*)0x0) {
				peVar2 = &gF32Vertex4Zero;
			}
			else {
				peVar2 = pCVar1->aSplinePoints + nbPoints + -3;
			}

			peVar3 = pCVar1->aSplinePoints;
			if (peVar3 == (edF32VECTOR4*)0x0) {
				peVar3 = &gF32Vertex4Zero;
			}

			pPVar4 = aPlaneData + iVar6;
			edF32Vector4SubHard(&pPVar4->field_0x0, peVar3, peVar2);
			edF32Vector4NormalizeHard(&pPVar4->field_0x0, &pPVar4->field_0x0);
		}
	}

	return;
}