#include "edMathTCBSpline.h"
#include "MathOps.h"

CedMathTCBSpline::CedMathTCBSpline(uint pointCount, int bDistributeWeights)
{
	CedMathTCBSplinePoint* pCVar1;
	uint uVar3;
	uint uVar4;
	int iVar5;
	float* pfVar6;
	float (*pafVar7)[4];
	float fVar8;
	float fVar9;

	this->field_0x0 = 0;
	this->nbPoints = pointCount;
	this->field_0x4 = -1.0f;
	this->field_0x10 = (undefined*)0x0;
	
	this->aPoints = new CedMathTCBSplinePoint[this->nbPoints];

	if (bDistributeWeights != 0) {
		this->field_0x0 = this->field_0x0 | 1;
	}

	uVar4 = this->nbPoints;
	uVar3 = 0;
	if (uVar4 != 0) {
		do {
			if ((int)uVar3 < 0) {
				fVar8 = (float)(uVar3 >> 1 | uVar3 & 1);
				fVar8 = fVar8 + fVar8;
			}
			else {
				fVar8 = (float)uVar3;
			}
			uVar4 = uVar4 - 1;
			if ((int)uVar4 < 0) {
				fVar9 = (float)(uVar4 >> 1 | uVar4 & 1);
				fVar9 = fVar9 + fVar9;
			}
			else {
				fVar9 = (float)uVar4;
			}

			this->aPoints[uVar3].time = fVar8 / fVar9;
			this->aPoints[uVar3].tension = 0.0f;
			this->aPoints[uVar3].bias = 0.0f;
			this->aPoints[uVar3].continuity = 0.0f;

			this->aPoints[uVar3].position = gF32Matrix4Zero.rowX;
			this->aPoints[uVar3].field_0x20 = gF32Matrix4Zero.rowY;
			this->aPoints[uVar3].field_0x30 = gF32Matrix4Zero.rowZ;
			this->aPoints[uVar3].field_0x40 = gF32Matrix4Zero.rowT;

			uVar4 = this->nbPoints;
			uVar3 = uVar3 + 1;
		} while (uVar3 < uVar4);
	}

	return;
}

void CedMathTCBSpline::SetKeyTCB(float tension, float bias, float continuity, int keyIndex)
{
	this->aPoints[keyIndex].tension = tension;
	this->aPoints[keyIndex].bias = bias;
	this->aPoints[keyIndex].continuity = continuity;
	return;
}

void CedMathTCBSpline::SetKeyPosition(int keyIndex, edF32VECTOR4* pPosition)
{
	this->aPoints[keyIndex].position = *pPosition;
	return;
}

void CedMathTCBSpline::PreComputeSpline(int param_2)
{
	uint uVar1;
	uint uVar3;
	uint uVar4;
	float fVar5;
	float fVar6;

	if (param_2 != 0) {
		uVar1 = this->nbPoints;
		uVar3 = 0;
		if (uVar1 != 0) {
			do {
				if ((int)uVar3 < 0) {
					fVar5 = (float)(uVar3 >> 1 | uVar3 & 1);
					fVar5 = fVar5 + fVar5;
				}
				else {
					fVar5 = (float)uVar3;
				}
				uVar1 = uVar1 - 1;
				if ((int)uVar1 < 0) {
					fVar6 = (float)(uVar1 >> 1 | uVar1 & 1);
					fVar6 = fVar6 + fVar6;
				}
				else {
					fVar6 = (float)uVar1;
				}
				this->aPoints[uVar3].time = fVar5 / fVar6;

				uVar3 = uVar3 + 1;
				uVar1 = this->nbPoints;
			} while (uVar3 < uVar1);
		}
	}

	ComputeKey(0, 0, 1, 2);

	uVar1 = this->nbPoints;
	uVar3 = uVar1 - 2;
	uVar4 = 1;
	if (1 < uVar3) {
		do {
			ComputeKey(uVar4 - 1, uVar4, uVar4 + 1, uVar4 + 2);
			uVar1 = this->nbPoints;
			uVar4 = uVar4 + 1;
			uVar3 = uVar1 - 2;
		} while (uVar4 < uVar3);
	}

	ComputeKey(uVar1 - 3, uVar3, uVar1 - 1, uVar1 - 1);

	if ((this->field_0x0 & 1) != 0) {
		IMPLEMENTATION_GUARD(
		DistributeWeights();)
	}

	return;
}

void CedMathTCBSpline::ComputeKey(uint param_2, uint param_3, uint param_4, uint param_5)
{
	CedMathTCBSplinePoint* pCVar1;
	CedMathTCBSplinePoint* pCVar2;
	edF32VECTOR4* v0;
	float fVar3;
	float fVar4;
	float fVar5;
	float fVar6;
	edF32VECTOR4 eStack112;
	edF32VECTOR4 eStack96;
	edF32VECTOR4 local_50;
	edF32MATRIX4 auStack64;

	edF32Vector4SubHard(&auStack64.rowX, &this->aPoints[param_4].position, &this->aPoints[param_3].position);
	pCVar2 = this->aPoints;
	pCVar1 = pCVar2 + param_3;
	fVar3 = pCVar1->continuity;
	fVar4 = pCVar1->bias;
	fVar6 = pCVar2[param_4].time - pCVar1->time;
	fVar5 = (fVar6 * (1.0f - pCVar1->tension)) / (pCVar2[param_4].time - pCVar2[param_2].time);
	edF32Vector4SubHard(&eStack96, &pCVar1->position, &pCVar2[param_2].position);
	edF32Vector4ScaleHard(fVar5 * (fVar4 + 1.0f) * (fVar3 + 1.0f), &eStack96, &eStack96);
	v0 = &auStack64.rowY;
	edF32Vector4ScaleHard(fVar5 * (1.0f - fVar4) * (1.0f - fVar3), v0, &auStack64.rowX);
	edF32Vector4AddHard(v0, v0, &eStack96);
	pCVar2 = this->aPoints;
	pCVar1 = pCVar2 + param_4;
	fVar3 = pCVar1->continuity;
	fVar4 = pCVar1->bias;
	fVar5 = (fVar6 * (1.0f - pCVar1->tension)) / (pCVar2[param_5].time - pCVar2[param_3].time);
	edF32Vector4SubHard(&eStack96, &pCVar2[param_5].position, &pCVar1->position);
	edF32Vector4ScaleHard(fVar5 * (fVar4 + 1.0f) * (1.0f - fVar3), &eStack96, &eStack96);
	edF32Vector4ScaleHard(fVar5 * (1.0f - fVar4) * (fVar3 + 1.0f), &eStack112, &auStack64.rowX);
	edF32Vector4AddHard(&auStack64.rowZ, &eStack96, &eStack112);
	auStack64.rowT = gF32Vector4Zero;

	pCVar2 = this->aPoints + param_3;
	pCVar2->field_0x20 = auStack64.rowY;

	local_50.x = 3.0f;
	local_50.y = -2.0f;
	local_50.z = -1.0f;
	local_50.w = 0.0f;
	edF32Matrix4MulF32Vector4Hard(&this->aPoints[param_3].field_0x30, &auStack64, &local_50);

	local_50.x = -2.0f;
	local_50.y = 1.0f;
	local_50.z = 1.0f;
	local_50.w = 0.0f;
	edF32Matrix4MulF32Vector4Hard(&this->aPoints[param_3].field_0x40, &auStack64, &local_50);

	return;
}