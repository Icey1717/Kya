#include "PathFollow.h"
#include "PathManager.h"
#include "MathOps.h"

edF32VECTOR4 CPathFollow::gPathDefQuat = { 0.0f, 0.0f, 0.0f, 1.0f };

CPathFollow::CPathFollow()
{
	this->splinePointCount = 0;
	this->field_0x14 = 0;
	this->aSplinePoints = (edF32VECTOR4*)0x0;
	this->aSplineRotationsEuler = (edF32VECTOR4*)0x0;
	this->aSplineRotationsQuat = (edF32VECTOR4*)0;
	this->field_0x28 = 0;
	this->aDelays = (float*)0x0;
	this->field_0x30 = 0;
	this->field_0x34 = 0;
	this->field_0x38 = 0;
}

void CPathFollow::Create(ByteCode* pByteCode)
{
	char* pcVar1;
	uint flags;
	int iVar3;
	int iVar4;

	pByteCode->Align(0x10);
	flags = pByteCode->GetU32();
	pByteCode->GetS32();
	pByteCode->GetS32();

	this->splinePointCount = pByteCode->GetS32();
	this->field_0x14 = 0;

	if ((flags & 1) != 0) {
		pcVar1 = pByteCode->currentSeekPos;
		pByteCode->currentSeekPos = pcVar1 + this->splinePointCount * sizeof(edF32VECTOR4);
		this->aSplinePoints = reinterpret_cast<edF32VECTOR4*>(pcVar1);
	}

	if ((flags & 2) != 0) {
		pcVar1 = pByteCode->currentSeekPos;
		pByteCode->currentSeekPos = pcVar1 + this->splinePointCount * sizeof(edF32VECTOR4);
		this->aSplineRotationsEuler = reinterpret_cast<edF32VECTOR4*>(pcVar1);
	}

	if ((flags & 4) != 0) {
		pcVar1 = pByteCode->currentSeekPos;
		pByteCode->currentSeekPos = pcVar1 + this->splinePointCount * 0x10;
		this->field_0x28 = pcVar1;
	}

	if ((flags & 8) != 0) {
		pcVar1 = pByteCode->currentSeekPos;
		pByteCode->currentSeekPos = pcVar1 + this->splinePointCount * sizeof(float);
		this->aDelays = reinterpret_cast<float*>(pcVar1);
	}

	if ((flags & 0x10) == 0) {
		this->field_0x18 = 0;
	}
	else {
		pcVar1 = pByteCode->currentSeekPos;
		pByteCode->currentSeekPos = pcVar1 + this->splinePointCount * 4;
		this->field_0x30 = reinterpret_cast<uint*>(pcVar1);
		this->field_0x18 = 0;

		iVar3 = 0;
		if (0 < (int)this->splinePointCount) {
			iVar4 = 0;
			do {
				this->field_0x18 = this->field_0x18 | *(uint*)(this->field_0x30 + iVar4);
				if ((*(uint*)(this->field_0x30 + iVar4) & 1) != 0) {
					this->field_0x14 = iVar3;
				}
				iVar3 = iVar3 + 1;
				iVar4 = iVar4 + 4;
			} while (iVar3 < (int)this->splinePointCount);
		}
	}

	if ((flags & 0x20) != 0) {
		pcVar1 = pByteCode->currentSeekPos;
		pByteCode->currentSeekPos = pcVar1 + this->splinePointCount * 4;
		this->field_0x34 = pcVar1;
	}

	if ((flags & 0x40) != 0) {
		pcVar1 = pByteCode->currentSeekPos;
		pByteCode->currentSeekPos = pcVar1 + this->splinePointCount * 4;
		this->field_0x38 = pcVar1;
	}

	this->mode = pByteCode->GetU32();
	this->type = pByteCode->GetU32();
	this->pathType = pByteCode->GetU32();

	return;
}

void CPathFollow::ComputeMatrix(edF32MATRIX4* pMatrix, int param_3)
{
	edF32VECTOR4* peVar1;
	float fVar2;
	float fVar3;
	float fVar4;

	if (this->aSplineRotationsQuat == (edF32VECTOR4*)0x0) {
		if (this->aSplineRotationsEuler == (edF32VECTOR4*)0x0) {
			edF32Matrix4SetIdentityHard(pMatrix);
		}
		else {
			edF32Matrix4FromEulerSoft(pMatrix, (edF32VECTOR3*)(this->aSplineRotationsEuler + param_3), "XYZ");
		}
	}
	else {
		edQuatToMatrix4Hard(this->aSplineRotationsQuat + param_3, pMatrix);
	}
	if (this->aSplinePoints != (edF32VECTOR4*)0x0) {
		peVar1 = this->aSplinePoints + param_3;
		fVar4 = peVar1->y;
		fVar2 = peVar1->z;
		fVar3 = peVar1->w;
		pMatrix->da = peVar1->x;
		pMatrix->db = fVar4;
		pMatrix->dc = fVar2;
		pMatrix->dd = fVar3;
	}
	return;
}

CPathFollowReaderAbsolute::CPathFollowReaderAbsolute()
{
	this->pActor3C_0x0 = (CPathFollow*)0x0;
	this->field_0x10 = (float*)0x0;
}

void CPathFollowReaderAbsolute::Create(ByteCode* pByteCode)
{
	int iVar1;
	CPathFollow* pPathFollow;
	float fVar2;
	float fVar3;
	CPathManager* pPathManager;

	pPathManager = CScene::ptable.g_PathManager_004516a0;

	iVar1 = pByteCode->GetS32();
	if (iVar1 == -1) {
		pPathFollow = (CPathFollow*)0x0;
	}
	else {
		pPathFollow = pPathManager->aPathFollow + iVar1;
	}

	iVar1 = pByteCode->GetS32();
	fVar2 = pByteCode->GetF32();
	fVar3 = pByteCode->GetF32();

	if ((pPathFollow != (CPathFollow*)0x0) && (pPathFollow != (CPathFollow*)0x0)) {
		Create(fVar2, fVar3, pPathFollow, pPathFollow->mode, pPathFollow->type, iVar1, 0);
	}

	return;
}

int CPathFollowReaderAbsolute::ComputeSegment(float param_1, int* param_3, int* param_4, float* param_5)
{
	int iVar1;
	float* pfVar2;
	int iVar3;
	int iVar4;
	int iVar5;
	int iVar6;
	float fVar7;
	float fVar8;
	float fVar9;
	float fVar10;

	if (param_1 < 0.0f) {
		param_1 = 0.0f;
	}

	fVar8 = this->field_0x8;
	fVar10 = 0.0f;
	iVar5 = this->pActor3C_0x0->splinePointCount;
	iVar1 = this->pActor3C_0x0->field_0x14;
	iVar6 = 0;

	if ((fVar8 == 0.0f) || (fVar8 <= param_1)) {
		param_1 = param_1 - fVar8;
		fVar10 = this->barFullAmount_0x4;
		if (fVar10 == 0.0f) {
			param_1 = -0.0001f;
		}
		else {
			iVar4 = this->mode;

			if ((iVar4 == 0) || ((iVar4 != 2 && (iVar4 != 1)))) {
				fVar10 = fVar10 - 0.0001f;

				if ((fVar10 < param_1) && (iVar6 = 2, param_1 = fVar10, fVar10 < 0.0f)) {
					param_1 = 0.0f;
				}
			}
			else {
				param_1 = fmodf(param_1, fVar10);
			}
		}

		if ((this->type != 1) || (fVar8 = this->field_0xc, param_1 < fVar8)) {
			fVar10 = 0.0f;
			iVar4 = iVar1;

			if (0.0f <= param_1) {
				pfVar2 = this->field_0x10 + iVar1;

				do {
					iVar4 = iVar4 + 1;
					fVar10 = fVar10 + *pfVar2;
					pfVar2 = pfVar2 + 1;
				} while (fVar10 <= param_1);
			}

			iVar3 = iVar4 + -1;
			fVar8 = fVar10 - this->field_0x10[iVar3];

			if (iVar5 == iVar4) {
				iVar4 = iVar1;
			}
		}
		else {
			if (param_1 == fVar8) {
				iVar3 = iVar5 + -1;
				iVar4 = iVar5 + -2;
				fVar10 = fVar8 + this->field_0x10[iVar4];
			}
			else {
				iVar4 = iVar5 + -1;
				fVar10 = fVar8;

				if (this->field_0x1c == 0) {
					if (fVar8 < param_1) {
						iVar5 = iVar4 * 4;
						for (; fVar10 = fVar8, fVar8 < param_1; fVar8 = fVar8 + fVar10 + (this->field_0x10[iVar4] - fVar7)) {
							pfVar2 = this->pActor3C_0x0->aDelays;
							if (pfVar2 == (float*)0x0) {
								fVar10 = 0.0f;
							}
							else {
								fVar10 = pfVar2[iVar4];
							}

							pfVar2 = this->pActor3C_0x0->aDelays;
							iVar5 = iVar5 + -4;
							iVar4 = iVar4 + -1;

							if (pfVar2 == (float*)0x0) {
								fVar7 = 0.0f;
							}
							else {
								fVar7 = pfVar2[iVar4];
							}
						}
					}

					pfVar2 = this->pActor3C_0x0->aDelays;
					iVar3 = iVar4 + 1;

					if (pfVar2 == (float*)0x0) {
						fVar7 = 0.0f;
					}
					else {
						fVar7 = pfVar2[iVar4];
					}

					if (pfVar2 == (float*)0x0) {
						fVar8 = 0.0f;
					}
					else {
						fVar8 = pfVar2[iVar3];
					}

					fVar8 = ((fVar10 - this->field_0x10[iVar4]) + fVar7) - fVar8;
				}
				else {
					if (fVar8 < param_1) {
						pfVar2 = this->field_0x10 + iVar4;

						do {
							pfVar2 = pfVar2 + -1;
							iVar4 = iVar4 + -1;
							fVar8 = fVar8 + *pfVar2;
							fVar10 = fVar8;
						} while (fVar8 < param_1);
					}

					iVar3 = iVar4 + 1;
					fVar8 = fVar10 - this->field_0x10[iVar4];
				}
			}
		}
	}
	else {
		iVar4 = 0;
		if (0.0f <= param_1) {
			pfVar2 = this->field_0x10;

			do {
				iVar4 = iVar4 + 1;
				fVar10 = fVar10 + *pfVar2;
				pfVar2 = pfVar2 + 1;
			} while (fVar10 <= param_1);
		}

		iVar3 = iVar4 + -1;
		fVar8 = fVar10 - this->field_0x10[iVar3];

		if (iVar5 == iVar4) {
			iVar4 = 0;
		}
	}

	if (this->field_0x1c == 0) {
		pfVar2 = this->pActor3C_0x0->aDelays;

		if (pfVar2 == (float*)0x0) {
			fVar7 = 0.0f;
		}
		else {
			fVar7 = pfVar2[iVar3];
		}
	}
	else {
		fVar7 = 0.0f;
	}

	fVar9 = (param_1 - fVar8) - fVar7;

	if (fVar9 < 0.0f) {
		fVar9 = 0.0f;
		iVar6 = 1;
	}

	fVar7 = (fVar10 - fVar8) - fVar7;

	if (0.0f < fVar7) {
		fVar9 = fVar9 / fVar7;
	}
	else {
		fVar9 = 0.0f;
	}

	*param_3 = iVar4;
	*param_4 = iVar3;
	*param_5 = fVar9;
	return iVar6;
}

int CPathFollowReaderAbsolute::ComputePosition(float param_1, edF32VECTOR4* param_3, edF32VECTOR4* param_4, S_PATHREADER_POS_INFO* pPathReaderPosInfo)
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
	int local_c;
	int local_8;
	float local_4;

	if (this->pActor3C_0x0 == (CPathFollow*)0x0) {
		iVar1 = 2;
	}
	else {
		iVar1 = ComputeSegment(param_1, &local_8, &local_c, &local_4);

		peVar3 = this->pActor3C_0x0->aSplinePoints;
		if (peVar3 == (edF32VECTOR4*)0x0) {
			peVar3 = &gF32Vertex4Zero;
		}
		else {
			peVar3 = peVar3 + local_8;
		}

		peVar2 = this->pActor3C_0x0->aSplinePoints;
		if (peVar2 == (edF32VECTOR4*)0x0) {
			peVar2 = &gF32Vertex4Zero;
		}
		else {
			peVar2 = peVar2 + local_c;
		}

		fVar4 = peVar3->y;
		fVar5 = peVar3->z;
		fVar6 = peVar3->w;
		fVar7 = peVar2->y;
		fVar8 = peVar2->z;
		fVar9 = peVar2->w;

		param_3->x = peVar3->x - peVar2->x;
		param_3->y = fVar4 - fVar7;
		param_3->z = fVar5 - fVar8;
		param_3->w = fVar6 - fVar9;
		param_3->x = param_3->x * local_4;
		param_3->y = param_3->y * local_4;
		param_3->z = param_3->z * local_4;
		param_3->w = param_3->w * local_4;
		fVar4 = peVar2->y;
		fVar5 = peVar2->z;
		fVar6 = peVar2->w;
		param_3->x = param_3->x + peVar2->x;
		param_3->y = param_3->y + fVar4;
		param_3->z = param_3->z + fVar5;
		param_3->w = param_3->w + fVar6;

		if (param_4 != (edF32VECTOR4*)0x0) {
			IMPLEMENTATION_GUARD(
				ComputeTangent(local_4, param_4, local_8, local_c);)
		}
		if (pPathReaderPosInfo != (S_PATHREADER_POS_INFO*)0x0) {
			pPathReaderPosInfo->field_0x0 = local_8;
			pPathReaderPosInfo->field_0x4 = local_c;
			pPathReaderPosInfo->field_0x8 = local_4;
		}
	}
	return iVar1;
}

int CPathFollowReaderAbsolute::ComputeMatrix(float param_1, edF32MATRIX4* pMatrix, edF32VECTOR4* param_4, S_PATHREADER_POS_INFO* param_5)
{
	int iVar1;
	edF32VECTOR4* pPointB;
	edF32VECTOR4* pPointA;
	float fVar4;
	float fVar5;
	float fVar6;
	float fVar7;
	float fVar8;
	float fVar9;
	edF32VECTOR4 eStack32;
	int pointB;
	int pointA;
	float local_4;

	if (this->pActor3C_0x0 == (CPathFollow*)0x0) {
		iVar1 = 2;
	}
	else {
		iVar1 = ComputeSegment(param_1, &pointA, &pointB, &local_4);

		pPointA = this->pActor3C_0x0->aSplineRotationsQuat;
		if (pPointA == (edF32VECTOR4*)0x0) {
			pPointA = &CPathFollow::gPathDefQuat;
		}
		else {
			pPointA = pPointA + pointB;
		}

		pPointB = this->pActor3C_0x0->aSplineRotationsQuat;
		if (pPointB == (edF32VECTOR4*)0x0) {
			pPointB = &CPathFollow::gPathDefQuat;
		}
		else {
			pPointB = pPointB + pointA;
		}

		edQuatShortestSLERPAccurate(local_4, &eStack32, pPointA, pPointB);
		edQuatToMatrix4Hard(&eStack32, pMatrix);

		pPointA = this->pActor3C_0x0->aSplinePoints;
		if (pPointA == (edF32VECTOR4*)0x0) {
			pPointA = &gF32Vertex4Zero;
		}
		else {
			pPointA = pPointA + pointA;
		}

		pPointB = this->pActor3C_0x0->aSplinePoints;
		if (pPointB == (edF32VECTOR4*)0x0) {
			pPointB = &gF32Vertex4Zero;
		}
		else {
			pPointB = pPointB + pointB;
		}

		pMatrix->rowT = *pPointA - *pPointB;
		pMatrix->rowT = pMatrix->rowT * local_4;
		pMatrix->rowT = pMatrix->rowT + *pPointB;


		if (param_4 != (edF32VECTOR4*)0x0) {
			ComputeTangent(local_4, param_4, pointA, pointB);
		}

		if (param_5 != (S_PATHREADER_POS_INFO*)0x0) {
			param_5->field_0x0 = pointA;
			param_5->field_0x4 = pointB;
			param_5->field_0x8 = local_4;
		}
	}

	return iVar1;
}

void CPathFollowReaderAbsolute::ComputeTangent(float param_1, edF32VECTOR4* param_3, int pointA, int pointB)
{
	int iVar1;
	edF32VECTOR4* pPointB;
	edF32VECTOR4* pPointA;
	float fVar4;
	float fVar5;
	float fVar6;
	float fVar7;
	float fVar8;
	float fVar9;
	edF32VECTOR4 local_10;

	pPointA = this->pActor3C_0x0->aSplinePoints;
	if (pPointA == (edF32VECTOR4*)0x0) {
		pPointA = &gF32Vertex4Zero;
	}
	else {
		pPointA = pPointA + pointB;
	}

	pPointB = this->pActor3C_0x0->aSplinePoints;
	if (pPointB == (edF32VECTOR4*)0x0) {
		pPointB = &gF32Vertex4Zero;
	}
	else {
		pPointB = pPointB + pointA;
	}

	*param_3 = *pPointB - *pPointA;
	edF32Vector4SafeNormalize1Hard(param_3, param_3);

	if (param_1 < 0.1f) {
		iVar1 = this->pActor3C_0x0->splinePointCount;
		if (iVar1 == 0) {
			trap(7);
		}

		pPointA = this->pActor3C_0x0->aSplinePoints;
		if (pPointA == (edF32VECTOR4*)0x0) {
			pPointA = &gF32Vertex4Zero;
		}
		else {
			pPointA = pPointA + ((pointB * 2 - pointA) + iVar1) % iVar1;
		}

		pPointB = this->pActor3C_0x0->aSplinePoints;
		if (pPointB == (edF32VECTOR4*)0x0) {
			pPointB = &gF32Vertex4Zero;
		}
		else {
			pPointB = pPointB + pointB;
		}

		local_10 = *pPointB - *pPointA;
		edF32Vector4SafeNormalize1Hard(&local_10, &local_10);

		edF32Vector4LERPHard((param_1 + 0.1f) * 5.0f, param_3, &local_10, param_3);
		edF32Vector4SafeNormalize1Hard(param_3, param_3);
	}
	else {
		if (0.9f < param_1) {
			iVar1 = this->pActor3C_0x0->splinePointCount;
			if (iVar1 == 0) {
				trap(7);
			}

			pPointA = this->pActor3C_0x0->aSplinePoints;
			if (pPointA == (edF32VECTOR4*)0x0) {
				pPointA = &gF32Vertex4Zero;
			}
			else {
				pPointA = pPointA + pointA;
			}

			pPointB = this->pActor3C_0x0->aSplinePoints;
			if (pPointB == (edF32VECTOR4*)0x0) {
				pPointB = &gF32Vertex4Zero;
			}
			else {
				pPointB = pPointB + ((pointA * 2 - pointB) + iVar1) % iVar1;
			}

			local_10 = *pPointB - *pPointA;
			edF32Vector4SafeNormalize1Hard(&local_10, &local_10);

			edF32Vector4LERPHard(((1.0f - param_1) + 0.1f) * 5.0f, param_3, &local_10, param_3);
			edF32Vector4SafeNormalize1Hard(param_3, param_3);
		}
	}

	return;
}

int CPathFollowReader::GetNextPlace(int param_2, int param_3)
{
	CPathFollow* pCVar1;
	uint uVar2;
	int iVar3;
	int iVar4;
	float fVar5;

	if (param_3 == 0) {
		iVar4 = -1;
	}
	else {
		iVar4 = 1;
	}

	pCVar1 = this->pPathFollow;
	uVar2 = pCVar1->mode;

	if (((uVar2 == 0) && (pCVar1->type == 0)) &&
		(((param_3 != 0 && (pCVar1->splinePointCount + -1 <= param_2)) || ((param_3 == 0 && (param_2 < 1)))))) {
		iVar4 = -1;
	}
	else {
		if (((param_2 == pCVar1->field_0x14) && ((param_3 == 0 && (this->splinePointIndex < this->field_0x8)))) ||
			((param_2 == pCVar1->splinePointCount + -1 && (param_3 != 0)))) {
			if (uVar2 == 1) {
				if ((param_3 == 0) && (pCVar1->type != 1)) {
					return -1;
				}

				iVar4 = -iVar4;
			}

			if ((uVar2 == 0) && (pCVar1->type == 1)) {
				iVar3 = pCVar1->field_0x14;
				fVar5 = fmodf((float)((param_2 + pCVar1->splinePointCount + -1) - iVar3), (float)(((pCVar1->splinePointCount + -1) - iVar3) * 2));
				param_2 = ((int)fVar5 - iVar4) + iVar3;
			}
		}

		iVar4 = param_2 + iVar4;
	}

	return iVar4;
}

void CPathFollowReader::SetToClosestSplinePoint(edF32VECTOR4* pLocation)
{
	int iVar1;
	int iVar2;
	CPathFollow* pCVar3;
	edF32VECTOR4* v2;
	int iVar5;
	float fVar6;
	float fVar7;
	edF32VECTOR4 eStack16;

	pCVar3 = this->pPathFollow;
	iVar2 = -1;

	// Find the closest spline point to the current location
	if ((pCVar3 != (CPathFollow*)0x0) && (iVar5 = 0, 0 < pCVar3->splinePointCount)) {
		fVar7 = 3.402823e+38f;
		do {
			if (pCVar3->aSplinePoints == (edF32VECTOR4*)0x0) {
				v2 = &gF32Vertex4Zero;
			}
			else {
				v2 = pCVar3->aSplinePoints + iVar5;
			}

			edF32Vector4SubHard(&eStack16, pLocation, v2);
			fVar6 = edF32Vector4GetDistHard(&eStack16);
			iVar1 = iVar5;
			if (fVar7 <= fVar6) {
				fVar6 = fVar7;
				iVar1 = iVar2;
			}

			iVar2 = iVar1;
			pCVar3 = this->pPathFollow;
			iVar5 = iVar5 + 1;
			fVar7 = fVar6;
		} while (iVar5 < pCVar3->splinePointCount);
	}

	this->splinePointIndex = iVar2;
	iVar2 = GetPrevPlace(this->splinePointIndex, this->field_0xc);
	this->field_0x8 = iVar2;

	return;
}

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

template<>
void S_STREAM_REF<CPathFollow>::Init()
{
	CPathFollow* pPathFollow;

	if (this->index == -1) {
		pPathFollow = (CPathFollow*)0x0;
	}
	else {
		pPathFollow = (CScene::ptable.g_PathManager_004516a0)->aPathFollow + this->index;
	}

	this->pObj = STORE_SECTION(pPathFollow);
	return;
}