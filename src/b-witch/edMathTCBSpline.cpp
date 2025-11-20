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
	this->caluclatedRoughLength = -1.0f;
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

static edF32VECTOR4 gSplineDefault =
{
	1.0f, 0.0f, 0.0f, 0.0f
};

void CedMathTCBSpline::GetPosition(float time, edF32VECTOR4* pOutPosition)
{
	uint activeKeyIndex;
	edF32VECTOR4 local_20;
	float local_4;

	activeKeyIndex = GetKeyFromTime(time, &local_4);

	local_20 = gSplineDefault;
	local_20.y = local_4 / (this->aPoints[activeKeyIndex + 1].time - this->aPoints[activeKeyIndex].time);
	local_20.z = local_20.y * local_20.y;
	local_20.w = local_20.y * local_20.z;
	local_4 = local_20.y;

	edF32Matrix4MulF32Vector4Hard(pOutPosition, &this->aPoints[activeKeyIndex].matrix, &local_20);
	pOutPosition->w = 1.0f;

	if (this->field_0x10 != (undefined*)0x0) {
		IMPLEMENTATION_GUARD(
		(*(code*)this->field_0x10)(local_4, this, activeKeyIndex, activeKeyIndex + 1);)
	}
	
	return;
}

uint CedMathTCBSpline::GetKeyFromTime(float time, float* param_3)
{
	CedMathTCBSplinePoint* pCVar1;
	uint uVar2;
	CedMathTCBSplinePoint* pCVar3;
	uint uVar4;
	float fVar5;

	pCVar1 = this->aPoints;
	uVar4 = this->nbPoints - 1;

	if (time <= pCVar1->time) {
		*param_3 = 0.0f;
		uVar2 = 0;
	}
	else {
		fVar5 = pCVar1[uVar4].time;
		if (fVar5 <= time) {
			uVar2 = this->nbPoints - 2;
			*param_3 = fVar5 - pCVar1[uVar4 - 1].time;
		}
		else {
			uVar2 = 0;
			pCVar3 = pCVar1;

			if (uVar4 != 0) {
				do {
					if (time < pCVar3[1].time) {
						*param_3 = time - pCVar1[uVar2].time;
						return uVar2;
					}

					uVar2 = uVar2 + 1;
					pCVar3 = pCVar3 + 1;
				} while (uVar2 < uVar4);
			}

			uVar2 = 0xffffffff;
		}
	}

	return uVar2;
}

edF32VECTOR4 edF32VECTOR4_0040ea90 = { 0.0f, 1.0f, 0.0f, 0.0f };

void CedMathTCBSpline::GetFirstDerivative(float time, edF32VECTOR4* pDerivative)
{
	uint uVar1;
	edF32VECTOR4 local_20;
	float local_4;

	uVar1 = GetKeyFromTime(time, &local_4);
	local_4 = local_4 / (this->aPoints[uVar1 + 1].time - this->aPoints[uVar1].time);
	local_20.x = edF32VECTOR4_0040ea90.x;
	local_20.y = edF32VECTOR4_0040ea90.y;
	local_20.z = local_4 * 2.0f;
	local_20.w = local_4 * 3.0f * local_4;
	edF32Matrix4MulF32Vector4Hard(pDerivative, &this->aPoints[uVar1].matrix, &local_20);
	edF32Vector4NormalizeHard(pDerivative, pDerivative);
	return;
}

float CedMathTCBSpline::GetRoughLength()
{
	uint uVar1;
	uint uVar2;
	uint curPointIndex;
	float fVar5;
	float alpha;
	float pointLength;
	float time;
	edF32VECTOR4 local_60;
	edF32VECTOR4 local_50;
	edF32VECTOR4 local_40;
	edF32VECTOR4 local_30;
	edF32VECTOR4 eStack32;
	float local_8;
	float local_4;

	if (this->caluclatedRoughLength < 0.0f) {
		this->caluclatedRoughLength = 0.0f;

		curPointIndex = 0;
		if (this->nbPoints != 1) {
			do {
				pointLength = 0.0f;
				time = this->aPoints[curPointIndex].time;
				alpha = this->aPoints[curPointIndex + 1].time;

				uVar1 = GetKeyFromTime(time, &local_4);
				local_50 = gSplineDefault;
				local_50.y = local_4 / (this->aPoints[uVar1 + 1].time - this->aPoints[uVar1].time);
				local_50.z = local_50.y * local_50.y;
				local_50.w = local_50.y * local_50.y * local_50.y;
				local_4 = local_50.y;

				edF32Matrix4MulF32Vector4Hard(&local_30, &this->aPoints[uVar1].matrix, &local_50);

				local_30.w = 1.0f;
				if (this->field_0x10 != (undefined*)0x0) {
					IMPLEMENTATION_GUARD(
						(*(code*)this->field_0x10)(local_4, this);)
				}

				uVar1 = 1;
				do {
					if ((int)uVar1 < 0) {
						fVar5 = (float)(uVar1 >> 1 | uVar1 & 1);
						fVar5 = fVar5 + fVar5;
					}
					else {
						fVar5 = (float)uVar1;
					}

					fVar5 = edFIntervalUnitSrcLERP(fVar5 / 20.0f, time, alpha);
					uVar2 = GetKeyFromTime(fVar5, &local_8);
					local_60 = gSplineDefault;
					local_60.y = local_8 / (this->aPoints[uVar2 + 1].time - this->aPoints[uVar2].time);
					local_60.z = local_60.y * local_60.y;
					local_60.w = local_60.y * local_60.y * local_60.y;
					local_8 = local_60.y;
					edF32Matrix4MulF32Vector4Hard(&local_40, &this->aPoints[uVar2].matrix, &local_60);
					local_40.w = 1.0f;

					if (this->field_0x10 != (undefined*)0x0) {
						IMPLEMENTATION_GUARD(
							(*(code*)this->field_0x10)(local_8);)
					}

					edF32Vector4SubHard(&eStack32, &local_40, &local_30);
					uVar1 = uVar1 + 1;
					pointLength = pointLength + edF32Vector4GetDistHard(&eStack32);
					local_30 = local_40;
				} while (uVar1 < 0x14);

				curPointIndex = curPointIndex + 1;
				this->caluclatedRoughLength = this->caluclatedRoughLength + pointLength;
			} while (curPointIndex < this->nbPoints - 1);
		}
	}

	return this->caluclatedRoughLength;
}