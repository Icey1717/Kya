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
		this->field_0x30 = pcVar1;
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

int CPathFollowReaderAbsolute::ComputePosition(float param_1, edF32VECTOR4* param_3, edF32VECTOR4* param_4, S_PATHREADER_POS_INFO* param_5)
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
		if (param_5 != (S_PATHREADER_POS_INFO*)0x0) {
			param_5->field_0x0 = local_8;
			param_5->field_0x4 = local_c;
			param_5->field_0x8 = local_4;
		}
	}
	return iVar1;
}

int CPathFollowReaderAbsolute::ComputeMatrix(float param_1, edF32MATRIX4* pMatrix, edF32VECTOR4* param_4, S_PATHREADER_POS_INFO* param_5)
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
	edF32VECTOR4 eStack32;
	int local_c;
	int local_8;
	float local_4;

	if (this->pActor3C_0x0 == (CPathFollow*)0x0) {
		iVar1 = 2;
	}
	else {
		iVar1 = ComputeSegment(param_1, &local_8, &local_c, &local_4);

		peVar3 = this->pActor3C_0x0->aSplineRotationsQuat;
		if (peVar3 == (edF32VECTOR4*)0x0) {
			peVar3 = &CPathFollow::gPathDefQuat;
		}
		else {
			peVar3 = peVar3 + local_c;
		}

		peVar2 = this->pActor3C_0x0->aSplineRotationsQuat;
		if (peVar2 == (edF32VECTOR4*)0x0) {
			peVar2 = &CPathFollow::gPathDefQuat;
		}
		else {
			peVar2 = peVar2 + local_8;
		}

		edQuatShortestSLERPAccurate(local_4, &eStack32, peVar3, peVar2);
		edQuatToMatrix4Hard(&eStack32, pMatrix);

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
		pMatrix->da = peVar3->x - peVar2->x;
		pMatrix->db = fVar4 - fVar7;
		pMatrix->dc = fVar5 - fVar8;
		pMatrix->dd = fVar6 - fVar9;
		pMatrix->da = pMatrix->da * local_4;
		pMatrix->db = pMatrix->db * local_4;
		pMatrix->dc = pMatrix->dc * local_4;
		pMatrix->dd = pMatrix->dd * local_4;
		fVar4 = peVar2->y;
		fVar5 = peVar2->z;
		fVar6 = peVar2->w;
		pMatrix->da = pMatrix->da + peVar2->x;
		pMatrix->db = pMatrix->db + fVar4;
		pMatrix->dc = pMatrix->dc + fVar5;
		pMatrix->dd = pMatrix->dd + fVar6;

		if (param_4 != (edF32VECTOR4*)0x0) {
			IMPLEMENTATION_GUARD(
				ComputeTangent(local_4, param_4, local_8, local_c);)
		}

		if (param_5 != (S_PATHREADER_POS_INFO*)0x0) {
			param_5->field_0x0 = local_8;
			param_5->field_0x4 = local_c;
			param_5->field_0x8 = local_4;
		}
	}

	return iVar1;
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