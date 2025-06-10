#include "MathOps.h"
#include <math.h>
#include <cmath>

#define absf std::abs

edF32MATRIX4 gF32Matrix4Zero = { };
edF32VECTOR3 gF32Vector3Zero = { };
edF32MATRIX4 gF32Matrix4Unit = { 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f };
edF32VECTOR4 gF32Vertex4Zero = { 0.0f, 0.0f, 0.0f, 1.0f };
edF32VECTOR4 gF32Vector4Zero = { 0.0f, 0.0f, 0.0f, 0.0f };
edF32VECTOR4 gF32Vector4UnitX = { 1.0f, 0.0f, 0.0f, 0.0f };
edF32VECTOR4 gF32Vector4UnitY = { 0.0f, 1.0f, 0.0f, 0.0f };
edF32VECTOR4 gF32Vector4UnitZ = { 0.0f, 0.0f, 1.0f, 0.0f };
edF32VECTOR4 g_xVector = { 0.0f, 1.0f, 0.0f, 0.0f };

#define M_PI_2f 1.5707963f
#define M_PIf 3.14159265f
#define M_2PIf 6.283185f

float g_DefaultNearClip_0044851c = -0.001f;

float edDistPointToPlane(edF32VECTOR4* subvector2, edF32VECTOR4* innerProductVector, edF32VECTOR4* subVector1)
{
	float fVar1;
	edF32VECTOR4 outputVector;

	edF32Vector4SubHard(&outputVector, subVector1, subvector2);
	fVar1 = edF32Vector4DotProductHard(&outputVector, innerProductVector);
	return fVar1;
}

void edQuatToMatrix4Hard(edF32VECTOR4* v0, edF32MATRIX4* m0)
{
	float fVar1;
	float fVar2;
	float fVar3;
	float fVar4;
	float fVar5;
	float fVar6;
	float fVar7;
	float fVar8;
	float fVar9;
	float x;
	float y;
	float z;
	float w;

	x = v0->x;
	y = v0->y;
	z = v0->z;
	w = v0->w;

	fVar1 = (y + 0.0f) * (y + 0.0f) + (z + 0.0f) * (z + 0.0f);
	fVar7 = (x + 0.0f) * (z + 0.0f) + (w + 0.0f) * (y + 0.0f);
	fVar3 = (x + 0.0f) * (y + 0.0f) - (w + 0.0f) * (z + 0.0f);
	fVar2 = (x + 0.0f) * (y + 0.0f) + (w + 0.0f) * (z + 0.0f);
	fVar5 = (x + 0.0f) * (x + 0.0f) + (z + 0.0f) * (z + 0.0f);
	fVar6 = (y + 0.0f) * (z + 0.0f) - (w + 0.0f) * (x + 0.0f);
	fVar4 = (y + 0.0f) * (z + 0.0f) + (w + 0.0f) * (x + 0.0f);
	fVar8 = (x + 0.0f) * (x + 0.0f) + (y + 0.0f) * (y + 0.0f);
	fVar9 = (x + 0.0f) * (z + 0.0f) - (w + 0.0f) * (y + 0.0f);

	m0->aa = (0.0f - (fVar1 + fVar1)) + 1.0f;
	m0->ab = fVar3 + fVar3;
	m0->ac = fVar7 + fVar7;
	m0->ad = 0.0f;

	m0->ba = fVar2 + fVar2;
	m0->bb = (0.0f - (fVar5 + fVar5)) + 1.0f;
	m0->bc = fVar6 + fVar6;
	m0->bd = 0.0f;

	m0->ca = fVar9 + fVar9;
	m0->cb = fVar4 + fVar4;
	m0->cc = (0.0f - (fVar8 + fVar8)) + 1.0f;
	m0->cd = 0.0f;

	m0->da = 0.0f;
	m0->db = 0.0f;
	m0->dc = 0.0f;
	m0->dd = 1.0f;
	return;
}


void edQuatInverse(edF32VECTOR4* param_1, edF32VECTOR4* param_2)
{
	param_1->x = -param_2->x;
	param_1->y = -param_2->y;
	param_1->z = -param_2->z;
	param_1->w = param_2->w;
	return;
}

void edQuatMul(edF32VECTOR4* param_1, edF32VECTOR4* param_2, edF32VECTOR4* param_3)
{
	float fVar1;
	float fVar2;
	float fVar3;
	float fVar4;
	float fVar5;
	float fVar6;
	float fVar7;
	float fVar8;

	fVar8 = param_3->z;
	fVar2 = param_2->y;
	fVar3 = param_3->x;
	fVar4 = param_2->w;
	fVar5 = param_3->w;
	fVar6 = param_2->x;
	fVar7 = param_3->y;
	fVar1 = param_2->z;
	param_1->x = (fVar2 * fVar8 + fVar6 * fVar5 + fVar4 * fVar3) - fVar1 * fVar7;
	param_1->y = fVar1 * fVar3 + fVar2 * fVar5 + (fVar4 * fVar7 - fVar6 * fVar8);
	param_1->z = fVar1 * fVar5 + ((fVar6 * fVar7 + fVar4 * fVar8) - fVar2 * fVar3);
	param_1->w = ((fVar4 * fVar5 - fVar6 * fVar3) - fVar2 * fVar7) - fVar1 * fVar8;
	return;
}

float edQuatToAngAxis(edF32VECTOR4* v0, float* f0, edF32VECTOR3* v1)
{
	float fVar1;
	float fVar2;
	float fVar3;

	fVar3 = sqrtf(v0->z * v0->z + v0->x * v0->x + v0->y * v0->y);
	fVar1 = edF32ACosHard(v0->w);
	*f0 = fVar1 * 2.0f;

	if (g_TinyFloat_00448548 < fVar3) {
		*v1 = v0->xyz / fVar3;
	}
	else {
		*v1 = v0->xyz;
	}

	return fVar3;
}

void edQuatFromEuler(edF32VECTOR4* v0, float x, float y, float z)
{
	float fVar1;
	float fVar2;
	float fVar3;
	float fVar4;
	float fVar5;
	float fVar6;

	fVar6 = x / 2.0f;
	fVar1 = cosf(fVar6);
	fVar2 = cosf(y / 2.0f);
	fVar3 = cosf(z / 2.0f);
	fVar6 = sinf(fVar6);
	fVar4 = sinf(y / 2.0f);
	fVar5 = sinf(z / 2.0f);
	v0->w = fVar6 * fVar4 * fVar5 + fVar1 * fVar2 * fVar3;
	v0->x = fVar6 * fVar2 * fVar3 - fVar1 * fVar4 * fVar5;
	v0->y = fVar5 * fVar6 * fVar2 + fVar3 * fVar1 * fVar4;
	v0->z = fVar5 * fVar1 * fVar2 - fVar3 * fVar6 * fVar4;
	return;
}

void edQuatShortestSLERPAccurate(float param_1, edF32VECTOR4* param_2, edF32VECTOR4* param_3, edF32VECTOR4* param_4)
{
	bool bVar1;
	float fVar2;
	float fVar3;
	float fVar4;
	float fVar5;
	float fVar6;
	float fVar7;
	float fVar8;
	float fVar9;

	fVar2 = param_3->x * param_4->x + param_3->y * param_4->y + param_3->z * param_4->z + param_3->w * param_4->w;
	bVar1 = fVar2 < 0.0f;

	if (bVar1) {
		fVar2 = -fVar2;
	}

	if (0.999999f < fVar2) {
		fVar3 = param_4->y;
		fVar2 = param_4->z;
		fVar4 = param_4->w;
		param_2->x = param_4->x;
		param_2->y = fVar3;
		param_2->z = fVar2;
		param_2->w = fVar4;
	}
	else {
		fVar2 = acosf(fVar2);
		fVar3 = sinf(fVar2);
		fVar3 = 1.0f / fVar3;
		fVar4 = sinf(fVar2 - param_1 * fVar2);
		fVar4 = fVar3 * fVar4;
		fVar2 = sinf(param_1 * fVar2);
		fVar3 = fVar3 * fVar2;

		if (bVar1) {
			fVar3 = -fVar3;
		}

		fVar2 = param_3->y;
		fVar5 = param_3->z;
		fVar6 = param_3->w;
		fVar7 = param_4->y;
		fVar8 = param_4->z;
		fVar9 = param_4->w;

		param_2->x = param_3->x * fVar4 + param_4->x * fVar3;
		param_2->y = fVar2 * fVar4 + fVar7 * fVar3;
		param_2->z = fVar5 * fVar4 + fVar8 * fVar3;
		param_2->w = fVar6 * fVar4 + fVar9 * fVar3;
	}
	return;
}

void edQuatNormalize(edF32VECTOR4* v0, edF32VECTOR4* v1)
{
	float fVar1;
	float fVar2;

	fVar2 = v1->x;
	fVar1 = 1.0f / sqrtf(v1->w * v1->w + v1->z * v1->z + v1->y * v1->y + fVar2 * fVar2);
	v0->x = fVar2 * fVar1;
	v0->y = v1->y * fVar1;
	v0->z = v1->z * fVar1;
	v0->w = v1->w * fVar1;
	return;
}

float edF32Vector4NormalizeHard(edF32VECTOR4* v0, edF32VECTOR4* v1)
{
	float fVar1;
	float fVar2;
	float fVar3;
	float fVar4;

	fVar1 = v1->x;
	fVar2 = v1->y;
	fVar3 = v1->z;
	const float magnitude = sqrtf(fVar1 * fVar1 + fVar2 * fVar2 + fVar3 * fVar3) + 0.0f;
	fVar4 = 1.0f / magnitude;
	v0->x = fVar1 * fVar4;
	v0->y = fVar2 * fVar4;
	v0->z = fVar3 * fVar4;
	v0->w = 0.0f;
	return magnitude;
}

void edF32Matrix4GetTransposeHard(edF32MATRIX4* m0, edF32MATRIX4* m1)
{
#if 1
	edF32MATRIX4 m1Copy = *m1;
	m0->aa = m1Copy.aa;
	m0->ab = m1Copy.ba;
	m0->ac = m1Copy.ca;
	m0->ad = m1Copy.da;
	m0->ba = m1Copy.ab;
	m0->bb = m1Copy.bb;
	m0->bc = m1Copy.cb;
	m0->bd = m1Copy.db;
	m0->ca = m1Copy.ac;
	m0->cb = m1Copy.bc;
	m0->cc = m1Copy.cc;
	m0->cd = m1Copy.dc;
	m0->da = m1Copy.ad;
	m0->db = m1Copy.bd;
	m0->dc = m1Copy.cd;
	m0->dd = m1Copy.dd;
	return;
#else
	__asm__ __volatile__("\n\
	lq $8,0x0000(%1)\n\
	lq $9,0x0010(%1)\n\
	lq $10,0x0020(%1)\n\
	lq $11,0x0030(%1)\n\
	\n\
	pextlw     $12,$9,$8\n\
	pextuw     $13,$9,$8\n\
	pextlw     $14,$11,$10\n\
	pextuw     $15,$11,$10\n\
	\n\
	pcpyld     $8,$14,$12\n\
	pcpyud     $9,$12,$14\n\
	pcpyld     $10,$15,$13\n\
	pcpyud     $11,$13,$15\n\
	\n\
	sq $8,0x0000(%0)\n\
	sq $9,0x0010(%0)\n\
	sq $10,0x0020(%0)\n\
	sq $11,0x0030(%0)\n\
	": : "r" (m0), "r" (m1) : "memory");
#endif
}

void edF32Vector4SubHard(edF32VECTOR4* v0, edF32VECTOR4* v1, edF32VECTOR4* v2)
{
	float fVar1;
	float fVar2;
	float fVar3;
	float fVar4;
	float fVar5;
	float fVar6;

	fVar1 = v1->y;
	fVar2 = v1->z;
	fVar3 = v1->w;
	fVar4 = v2->y;
	fVar5 = v2->z;
	fVar6 = v2->w;
	v0->x = v1->x - v2->x;
	v0->y = fVar1 - fVar4;
	v0->z = fVar2 - fVar5;
	v0->w = fVar3 - fVar6;
	return;
}

void edF32Vector4CrossProductHard(edF32VECTOR4* v0, edF32VECTOR4* v1, edF32VECTOR4* v2)
{
	float fVar1;
	float fVar2;
	float fVar3;
	float fVar4;
	float fVar5;
	float fVar6;

	fVar1 = v1->x;
	fVar2 = v1->y;
	fVar3 = v1->z;
	fVar4 = v2->x;
	fVar5 = v2->y;
	fVar6 = v2->z;
	v0->x = fVar2 * fVar6 - fVar5 * fVar3;
	v0->y = fVar3 * fVar4 - fVar6 * fVar1;
	v0->z = fVar1 * fVar5 - fVar4 * fVar2;
	v0->w = in_vf0x;
	return;
}

void GetAnglesFromVector(edF32VECTOR3* pitchAngles, edF32VECTOR4* velocity)
{
	float fVar1;

	fVar1 = atan2f(-velocity->y, sqrtf(velocity->z * velocity->z + velocity->x * velocity->x));
	fVar1 = fmodf(fVar1, M_2PIf);
	pitchAngles->x = fVar1;
	fVar1 = atan2f(velocity->x, velocity->z);
	fVar1 = fmodf(fVar1, M_2PIf);
	pitchAngles->y = fVar1;
	pitchAngles->z = 0.0f;
	return;
}

void edF32Matrix4RotateXHard(float angle, edF32MATRIX4* m0, edF32MATRIX4* m1)
{
	edF32MATRIX4 local_40;

	local_40.bb = cosf(angle);
	local_40.bc = sinf(angle);
	local_40.ab = 0.0f;
	local_40.ac = 0.0f;
	local_40.ad = 0.0f;
	local_40.aa = 1.0f;
	local_40.cb = -local_40.bc;
	local_40.ba = 0.0f;
	local_40.bd = 0.0f;
	local_40.ca = 0.0f;
	local_40.cd = 0.0f;
	local_40.da = 0.0f;
	local_40.db = 0.0f;
	local_40.dc = 0.0f;
	local_40.dd = 1.0f;
	local_40.cc = local_40.bb;
	edF32Matrix4MulF32Matrix4Hard(m0, m1, &local_40);
	return;
}

void edF32Matrix4RotateYHard(float angle, edF32MATRIX4* outputMatrix, edF32MATRIX4* inputMatrix)
{
	edF32MATRIX4 local_40;

	local_40.aa = cosf(angle);
	local_40.ca = sinf(angle);
	local_40.ab = 0.0f;
	local_40.ac = -local_40.ca;
	local_40.ad = 0.0f;
	local_40.ba = 0.0f;
	local_40.bb = 1.0f;
	local_40.bc = 0.0f;
	local_40.bd = 0.0f;
	local_40.cb = 0.0f;
	local_40.cd = 0.0f;
	local_40.da = 0.0f;
	local_40.db = 0.0f;
	local_40.dc = 0.0f;
	local_40.dd = 1.0f;
	local_40.cc = local_40.aa;
	edF32Matrix4MulF32Matrix4Hard(outputMatrix, inputMatrix, &local_40);
	return;
}

void edF32Matrix4CopyHard(edF32MATRIX4* m0, edF32MATRIX4* m1)
{
	float fVar1;
	float fVar2;
	float fVar3;
	float fVar4;
	float fVar5;
	float fVar6;
	float fVar7;
	float fVar8;
	float fVar9;
	float fVar10;
	float fVar11;
	float fVar12;
	float fVar13;
	float fVar14;
	float fVar15;

	fVar1 = m1->ab;
	fVar2 = m1->ac;
	fVar3 = m1->ad;
	fVar4 = m1->ba;
	fVar5 = m1->bb;
	fVar6 = m1->bc;
	fVar7 = m1->bd;
	fVar8 = m1->ca;
	fVar9 = m1->cb;
	fVar10 = m1->cc;
	fVar11 = m1->cd;
	fVar12 = m1->da;
	fVar13 = m1->db;
	fVar14 = m1->dc;
	fVar15 = m1->dd;
	m0->aa = m1->aa;
	m0->ab = fVar1;
	m0->ac = fVar2;
	m0->ad = fVar3;
	m0->ba = fVar4;
	m0->bb = fVar5;
	m0->bc = fVar6;
	m0->bd = fVar7;
	m0->ca = fVar8;
	m0->cb = fVar9;
	m0->cc = fVar10;
	m0->cd = fVar11;
	m0->da = fVar12;
	m0->db = fVar13;
	m0->dc = fVar14;
	m0->dd = fVar15;
	return;
}

void edF32Matrix4ScaleHard(edF32MATRIX4* m0, edF32MATRIX4* m1, edF32VECTOR4* v0)
{
	undefined8 uVar1;
	float fVar2;
	float fVar3;
	int iVar4;
	edF32MATRIX4 local_40;

	edF32Matrix4CopyHard(&local_40, &gF32Matrix4Zero);

	local_40.aa = v0->x;
	local_40.bb = v0->y;
	local_40.cc = v0->z;
	local_40.dd = 1.0;
	edF32Matrix4MulF32Matrix4Hard(m0, m1, &local_40);
	return;
}

void edF32Matrix4RotateZHard(float t0, edF32MATRIX4* m0, edF32MATRIX4* m1)
{
	edF32MATRIX4 local_40;

	local_40.aa = cosf(t0);
	local_40.ab = sinf(t0);
	local_40.ac = 0.0f;
	local_40.ba = -local_40.ab;
	local_40.ad = 0.0f;
	local_40.bc = 0.0f;
	local_40.bd = 0.0f;
	local_40.ca = 0.0f;
	local_40.cb = 0.0f;
	local_40.cc = 1.0f;
	local_40.cd = 0.0f;
	local_40.da = 0.0f;
	local_40.db = 0.0f;
	local_40.dc = 0.0f;
	local_40.dd = 1.0f;
	local_40.bb = local_40.aa;
	edF32Matrix4MulF32Matrix4Hard(m0, m1, &local_40);
	return;
}

void ed3DComputeProjectionToScreenMatrix
(float startX, float endX, float startY, float endY, float startZ, float endZ, float startW, float endW, edF32MATRIX4* m0)
{
	float fVar1;
	float fVar2;

	fVar2 = 1.0f / (endX - startX);
	fVar1 = 1.0f / (endY - startY);
	m0->aa = (endZ - startZ) * fVar2;
	m0->ab = 0.0f;
	m0->ac = 0.0f;
	m0->ad = 0.0f;
	m0->ba = 0.0f;
	m0->bb = (endW - startW) * fVar1;
	m0->bc = 0.0f;
	m0->bd = 0.0f;
	m0->ca = 0.0f;
	m0->cb = 0.0f;
	m0->cc = 1.0f;
	m0->cd = 0.0f;
	m0->da = fVar2 * (startZ * (endX - startX) - startX * (endZ - startZ));
	m0->db = fVar1 * (startW * (endY - startY) - startY * (endW - startW));
	m0->dc = 0.0f;
	m0->dd = 1.0f;
	return;
}

void ed3DComputeLocalToProjectionMatrix(float x, float y, float yMin, float yMax, edF32MATRIX4* m0)
{
	float fVar1;
	float fVar2;

	m0->aa = 1.0f;
	m0->ab = 0.0f;
	m0->ac = 0.0f;
	m0->ad = 0.0f;
	m0->ba = 0.0f;
	fVar1 = 1.0f / (yMax - yMin);
	m0->bb = 1.0f;
	m0->bc = 0.0f;
	m0->bd = 0.0f;
	m0->ca = x * fVar1;
	m0->cb = y * fVar1;
	m0->cc = yMax * fVar1;
	m0->cd = fVar1;
	fVar2 = yMax * -fVar1;
	m0->da = x * fVar2;
	m0->db = y * fVar2;
	m0->dc = yMin * fVar2;
	m0->dd = yMin * -fVar1;
	return;
}

uint GetGreaterPower2Val(uint value)
{
	uint uVar1;
	uint uVar2;

	uVar2 = 0;
	uVar1 = value;
	if (value == 0) {
		value = 0;
	}
	else {
		for (; (uVar1 != 0 && (uVar1 != 1)); uVar1 = uVar1 >> 1) {
			uVar2 = uVar2 + 1;
		}
		uVar1 = 1 << (uVar2 & 0x1f);
		if (uVar1 != value) {
			value = uVar1 << 1;
		}
	}
	return value;
}

float edF32ATanSoft(float val)
{
	int iVar1;
	float fVar2;
	float fVar3;

	iVar1 = -1;
	if (val < 0.0) {
		val = -val;
	}
	else {
		iVar1 = 1;
	}
	if (2.414214f < val) {
		fVar2 = 1.570796f;
		val = -(1.0f / val);
	}
	else {
		if (0.4142136f < val) {
			fVar2 = 0.7853982f;
			val = (val - 1.0f) / (val + 1.0f);
		}
		else {
			fVar2 = 0.0f;
		}
	}
	fVar3 = val * val;
	fVar2 = fVar2 + val + val * fVar3 * (fVar3 * ((fVar3 * 0.08053745f - 0.1387769f) * fVar3 + 0.1997771f) - 0.3333295f);
	if (iVar1 < 0) {
		fVar2 = -fVar2;
	}
	return fVar2;
}

float edF32ATanHard(float val)
{
	return edF32ATanSoft(val);
}

#define M_PI 3.1415927f
#define M_NEG_PI -3.1415927f
#define M_PI_2 1.5707964f
#define M_NEG_PI_2 -1.5707964f

float edF32ATan2Soft(float a, float b)
{
	uint uVar1;
	float fVar2;
	float fVar3;

	uVar1 = 0;
	if (b < 0.0f) {
		uVar1 = 2;
	}
	if (a < 0.0f) {
		uVar1 = uVar1 | 1;
	}
	if (b == 0.0f) {
		fVar3 = M_NEG_PI_2;
		if (((uVar1 & 1) == 0) && (fVar3 = 0.0f, a != 0.0f)) {
			fVar3 = M_PI_2;
		}
	}
	else {
		if (a == 0.0f) {
			fVar3 = 0.0f;
			if ((uVar1 & 2) != 0) {
				fVar3 = M_PI;
			}
		}
		else {
			fVar3 = M_NEG_PI;
			if ((uVar1 != 3) && (fVar3 = M_PI, uVar1 != 2)) {
				fVar3 = 0.0f;
			}
			fVar2 = edF32ATanHard(a / b);
			fVar3 = fVar3 + fVar2;
		}
	}
	return fVar3;
}

void SetVectorFromAngleY(float t0, edF32VECTOR4* v0)
{
	v0->x = sinf(t0);
	v0->y = 0.0f;
	v0->z = cosf(t0);
	v0->w = 0.0f;
	return;
}

void SetVectorFromAngles(edF32VECTOR4* rotQuat, edF32VECTOR3* rotEuler)
{
	float fVar1;
	edF32MATRIX4 m0;

	if ((0.0001f < fabs(rotEuler->z)) || (0.0001f < fabs(rotEuler->x))) {
		edF32Matrix4FromEulerSoft(&m0, rotEuler, "XYZ");
		rotQuat->x = m0.ca;
		rotQuat->y = m0.cb;
		rotQuat->z = m0.cc;
		rotQuat->w = m0.cd;
	}
	else {
		fVar1 = rotEuler->y;
		rotQuat->x = sinf(fVar1); //g_FloatSineCurve_00472260[(int)(fabs((fVar1 - 1.570796) * 1303.797) + 0.5) & 0x1fff];
		rotQuat->y = 0.0f;
		rotQuat->z = cosf(fVar1); //g_FloatSineCurve_00472260[(int)(fabs(fVar1 * 1303.797) + 0.5) & 0x1fff];
		rotQuat->w = 0.0f;
	}
	return;
}

void edF32Matrix4ToEulerSoft(edF32MATRIX4* m0, edF32VECTOR3* v0, char* rotationOrder)
{
	float* pfVar1;
	int iVar2;
	int iVar3;
	int iVar4;
	float fVar5;
	float fVar6;
	float fVar7;

	iVar4 = *rotationOrder + -0x58;
	iVar3 = rotationOrder[1] + -0x58;
	iVar2 = rotationOrder[2] + -0x58;
	pfVar1 = &m0[-0x16].aa + *rotationOrder * 4;
	fVar5 = pfVar1[iVar4];
	fVar7 = pfVar1[iVar3];
	fVar5 = sqrtf(fVar7 * fVar7 + fVar5 * fVar5);
	if (0.001f < fVar5) {
		fVar7 = edF32ATan2Soft(*(float*)((ulong)m0 + iVar2 * 4 + iVar3 * 0x10), ((edF32MATRIX4*)((ulong)m0 + iVar2 * 0x14))->aa
		);
		fVar5 = edF32ATan2Soft(-pfVar1[iVar2], fVar5);
		fVar6 = edF32ATan2Soft(pfVar1[iVar3], pfVar1[iVar4]);
	}
	else {
		fVar7 = edF32ATan2Soft(-*(float*)((ulong)m0 + iVar3 * 4 + iVar2 * 0x10),
			((edF32MATRIX4*)((ulong)m0 + iVar3 * 0x14))->aa);
		fVar5 = M_PI_2f;
		if (-pfVar1[iVar2] < 0.0f) {
			fVar5 = M_NEG_PI_2;
		}
		fVar6 = 0.0;
	}
	((edF32VECTOR3*)((ulong)v0 + iVar4 * 4))->x = fVar7;
	((edF32VECTOR3*)((ulong)v0 + iVar3 * 4))->x = fVar5;
	((edF32VECTOR3*)((ulong)v0 + iVar2 * 4))->x = fVar6;
	return;
}

void MatrixRotationFromVectorAndAngle(float angle, edF32MATRIX4* v0, edF32VECTOR4* v1)
{
	float fVar1;
	float fVar2;
	float fVar3;
	float fVar4;
	float fVar5;
	float fVar6;
	float fVar7;
	float fVar8;
	float fVar9;
	float fVar10;
	float fVar11;
	float fVar12;
	float fVar13;

	fVar11 = v1->y;
	fVar8 = v1->y;
	fVar9 = v1->z;
	fVar6 = v1->z;
	fVar2 = v1->x;
	fVar12 = cosf(angle);
	fVar13 = sinf(angle);
	fVar10 = 1.0f - fVar12;
	fVar4 = fVar10 * v1->x * v1->y;
	fVar5 = fVar10 * v1->y * v1->z;
	fVar1 = v1->z * fVar13;
	fVar7 = fVar10 * v1->x * v1->z;

	fVar3 = v1->y * fVar13;
	v0->aa = fVar12 + v1->x * v1->x * fVar10;
	fVar2 = fVar2 * fVar13;
	v0->ab = fVar4 - fVar1;
	v0->ac = fVar7 + fVar3;
	v0->ad = 0.0f;

	v0->ba = fVar4 + fVar1;
	v0->bb = fVar12 + fVar11 * fVar8 * fVar10;
	v0->bc = fVar5 - fVar2;
	v0->bd = 0.0f;

	v0->ca = fVar7 - fVar3;
	v0->cb = fVar5 + fVar2;
	v0->cc = fVar12 + fVar9 * fVar6 * fVar10;
	v0->cd = 0.0f;

	v0->rowT = gF32Vertex4Zero;
	return;
}

edF32MATRIX4* edF32Matrix4FromEulerSoft(edF32MATRIX4* m0, edF32VECTOR3* v0, char* order)
{
	edF32VECTOR4* puVar2;
	edF32VECTOR4* puVar3;
	float* pRow;
	float fVar5;
	float fVar6;
	float fVar7;
	float fVar8;
	float fVar9;
	float fVar10;
	float fVar11;
	edF32VECTOR4 local_20;
	int firstOrderIndex;
	int secondOrderIndex;
	int thirdOrderIndex;
	edF32VECTOR4* puVar1;
	char firstChar;
	char secondChar;
	char thirdChar;


	firstChar = order[0];
	secondChar = order[1];
	thirdChar = order[2];

	firstOrderIndex = firstChar + -0x58;
	secondOrderIndex = secondChar + -0x58;
	thirdOrderIndex = thirdChar + -0x58;

	memset(&local_20, 0, sizeof(local_20));

	fVar5 = v0->raw[firstOrderIndex];
	fVar6 = v0->raw[secondOrderIndex];
	fVar7 = v0->raw[thirdOrderIndex];

	fVar8 = cosf(fVar5);
	fVar10 = sinf(fVar5);
	fVar11 = cosf(fVar6);
	fVar9 = sinf(fVar6);
	fVar5 = cosf(fVar7);
	fVar6 = sinf(fVar7);

	pRow = m0->vector[firstOrderIndex].raw;
	pRow[firstOrderIndex] = fVar11 * fVar5;
	pRow[secondOrderIndex] = fVar11 * fVar6;
	pRow[thirdOrderIndex] = -fVar9;
	pRow[3] = 0.0f;

	pRow = m0->vector[secondOrderIndex].raw;
	pRow[firstOrderIndex] = fVar9 * fVar10 * fVar5 - fVar8 * fVar6;
	pRow[secondOrderIndex] = fVar8 * fVar5 + fVar10 * fVar6 * fVar9;
	pRow[thirdOrderIndex] = fVar11 * fVar10;
	pRow[3] = 0.0f;

	pRow = m0->vector[thirdOrderIndex].raw;
	pRow[firstOrderIndex] = fVar10 * fVar6 + fVar9 * fVar8 * fVar5;
	pRow[secondOrderIndex] = fVar9 * fVar8 * fVar6 - fVar10 * fVar5;
	pRow[thirdOrderIndex] = fVar8 * fVar11;
	pRow[3] = 0.0f;

	m0->rowT = gF32Vertex4Zero;
	return m0;
}

void edQuatFromMatrix4(edF32VECTOR4* v0, edF32MATRIX4* m0)
{
	int iVar1;
	int iVar2;
	float* pfVar3;
	float* pfVar4;
	float* pfVar5;
	int iVar6;
	float fVar7;
	float fVar8;
	float fVar9;
	float fVar10;

	fVar10 = m0->bb;
	fVar7 = m0->aa;
	fVar8 = m0->cc;
	fVar9 = fVar8 + fVar7 + fVar10;
	if (0.0f < fVar9) {
		fVar7 = sqrtf(fVar9 + 1.0f);
		fVar8 = 0.5f / fVar7;
		v0->w = fVar7 * 0.5f;
		v0->x = fVar8 * (m0->cb - m0->bc);
		v0->y = fVar8 * (m0->ac - m0->ca);
		v0->z = fVar8 * (m0->ba - m0->ab);
	}
	else {
		if (fVar10 < fVar7) {
			iVar2 = 2;
			if (fVar8 < fVar7) {
				iVar2 = 0;
				iVar1 = 1;
				iVar6 = 2;
			}
			else {
				iVar1 = 0;
				iVar6 = 1;
			}
		}
		else {
			iVar2 = 2;
			if (fVar8 < fVar10) {
				iVar2 = 1;
				iVar1 = 2;
				iVar6 = 0;
			}
			else {
				iVar1 = 0;
				iVar6 = 1;
			}
		}

		pfVar5 = &m0->aa + iVar1 * 4;
		pfVar3 = &m0->aa + iVar2 * 4;
		pfVar4 = &m0->aa + iVar6 * 4;

		// #Cleanup
		fVar7 = sqrtf(((pfVar3[iVar2] - pfVar5[iVar1]) - pfVar4[iVar6]) + 1.0f);
		fVar8 = 0.5f / fVar7;
		(&v0->x)[iVar2] = fVar7 * 0.5f;
		v0->w = fVar8 * (pfVar4[iVar1] - pfVar5[iVar6]);
		(&v0->x)[iVar1] = fVar8 * (pfVar5[iVar2] + pfVar3[iVar1]);
		(&v0->x)[iVar6] = fVar8 * (pfVar4[iVar2] + pfVar3[iVar6]);
	}

	return;
}

void edF32Matrix4FromEulerOrdSoft(edF32MATRIX4* rotatedMatrix, char* rotationOrder, float* rotationAngles)
{
	char cVar1;
	char cVar2;
	int iVar3;
	int iVar4;
	int iVar5;
	float* pfVar6;
	float fVar7;
	float fVar8;
	float fVar9;
	float fVar10;
	float fVar11;
	float fVar12;

	iVar3 = *rotationOrder + -0x58;
	cVar1 = rotationOrder[1];
	iVar4 = cVar1 + -0x58;
	cVar2 = rotationOrder[2];
	iVar5 = cVar2 + -0x58;
	pfVar6 = &rotatedMatrix[-0x16].aa + *rotationOrder * 4;
	fVar7 = sinf(rotationAngles[0]); //g_FloatSineCurve_00472260[(int)(fabs(*rotationAngles * g_ScalingFactor_00448518) + 0.5) & 0x1fff];
	fVar11 = cosf(rotationAngles[0]); //g_FloatSineCurve_00472260[(int)(fabs((*rotationAngles - M_PI_2f) * g_ScalingFactor_00448518) + 0.5) & 0x1fff];
	fVar12 = sinf(rotationAngles[1]); //g_FloatSineCurve_00472260[(int)(fabs(rotationAngles[1] * g_ScalingFactor_00448518) + 0.5) & 0x1fff];
	fVar10 = cosf(rotationAngles[1]); //g_FloatSineCurve_00472260 [(int)(fabs((rotationAngles[1] - M_PI_2f) * g_ScalingFactor_00448518) + 0.5) & 0x1fff];
	fVar8 = sinf(rotationAngles[2]); //g_FloatSineCurve_00472260[(int)(fabs(rotationAngles[2] * g_ScalingFactor_00448518) + 0.5) & 0x1fff];
	fVar9 = cosf(rotationAngles[2]); //g_FloatSineCurve_00472260[(int)(fabs((rotationAngles[2] - M_PI_2f) * g_ScalingFactor_00448518) + 0.5) & 0x1fff];
	pfVar6[iVar3] = fVar12 * fVar8;
	pfVar6[iVar4] = fVar12 * fVar9;
	pfVar6[iVar5] = -fVar10;
	pfVar6[3] = 0.0;
	pfVar6 = &rotatedMatrix[-0x16].aa + cVar1 * 4;
	pfVar6[iVar3] = fVar10 * fVar11 * fVar8 - fVar7 * fVar9;
	pfVar6[iVar4] = fVar7 * fVar8 + fVar11 * fVar9 * fVar10;
	pfVar6[iVar5] = fVar12 * fVar11;
	pfVar6[3] = 0.0;
	pfVar6 = &rotatedMatrix[-0x16].aa + cVar2 * 4;
	pfVar6[iVar3] = fVar11 * fVar9 + fVar10 * fVar7 * fVar8;
	pfVar6[iVar4] = fVar10 * fVar7 * fVar9 - fVar11 * fVar8;
	pfVar6[iVar5] = fVar7 * fVar12;
	pfVar6[3] = 0.0;
	fVar9 = gF32Vertex4Zero.w;
	fVar8 = gF32Vertex4Zero.z;
	fVar7 = gF32Vertex4Zero.y;
	rotatedMatrix->da = gF32Vertex4Zero.x;
	rotatedMatrix->db = fVar7;
	rotatedMatrix->dc = fVar8;
	rotatedMatrix->dd = fVar9;
	return;
}

void edQuatShortestSLERPHard(float alpha, edF32VECTOR4* outRotation, edF32VECTOR4* currentRotation, edF32VECTOR4* targetRotation)
{
	bool bVar1;
	float fVar2;
	float fVar3;
	float fVar4;
	float fVar5;
	float fVar6;
	float fVar7;
	float fVar8;
	float fVar9;

	fVar5 = currentRotation->x * targetRotation->x + currentRotation->y * targetRotation->y +
		currentRotation->z * targetRotation->z + currentRotation->w * targetRotation->w;
	bVar1 = fVar5 < 0.0f;
	if (bVar1) {
		fVar5 = -fVar5;
	}
	if (0.99999f < fVar5) {
		fVar2 = targetRotation->y;
		fVar5 = targetRotation->z;
		fVar3 = targetRotation->w;
		outRotation->x = targetRotation->x;
		outRotation->y = fVar2;
		outRotation->z = fVar5;
		outRotation->w = fVar3;
	}
	else {
		fVar3 = (((1.570729f - fVar5 * 0.2121144f) + fVar5 * fVar5 * 0.074261f) - fVar5 * fVar5 * fVar5 * 0.0187293f) * sqrtf(1.0f - fVar5);
		fVar2 = 1.0f / sqrtf(1.0f - fVar5 * fVar5);
		fVar4 = alpha * fVar3;
		fVar3 = fVar3 - fVar4;
		fVar7 = fVar3 * fVar3;
		fVar5 = fVar7 * fVar3 * fVar7;
		fVar6 = fVar5 * fVar7;
		fVar5 = fVar2 * (fVar6 * fVar7 * 2.601887e-06f +
			fVar6 * -0.0001980741f + fVar5 * 0.008333026f + fVar7 * fVar3 * -0.1666666f + fVar3 * 1.0f);
		fVar7 = fVar4 * fVar4;
		fVar3 = fVar7 * fVar4 * fVar7;
		fVar6 = fVar3 * fVar7;
		fVar2 = fVar2 * (fVar6 * fVar7 * 2.601887e-06f +
			fVar6 * -0.0001980741f + fVar3 * 0.008333026f + fVar7 * fVar4 * -0.1666666f + fVar4 * 1.0f);
		if (bVar1) {
			fVar2 = -fVar2;
		}
		fVar3 = currentRotation->y;
		fVar4 = currentRotation->z;
		fVar6 = currentRotation->w;
		fVar7 = targetRotation->y;
		fVar8 = targetRotation->z;
		fVar9 = targetRotation->w;
		outRotation->x = currentRotation->x * fVar5 + targetRotation->x * fVar2;
		outRotation->y = fVar3 * fVar5 + fVar7 * fVar2;
		outRotation->z = fVar4 * fVar5 + fVar8 * fVar2;
		outRotation->w = fVar6 * fVar5 + fVar9 * fVar2;
	}
	return;
}

void edF32Vector3LERPSoft(float alpha, edF32VECTOR3* outWorldLocation, edF32VECTOR3* currentLocation, edF32VECTOR3* targetLocation)
{
	float remainingKeyframePlayTime;

	if (currentLocation == targetLocation) {
		/* If the keyframes are the same, just get the first keyframe and set our location from that */
		outWorldLocation->x = currentLocation->x;
		outWorldLocation->y = currentLocation->y;
		outWorldLocation->z = currentLocation->z;
	}
	else {
		/* If the keyframes are different, find somewhere midway through based on current keyframe play
		   time */
		remainingKeyframePlayTime = 1.0 - alpha;
		outWorldLocation->x = targetLocation->x * alpha + currentLocation->x * remainingKeyframePlayTime;
		outWorldLocation->y = targetLocation->y * alpha + currentLocation->y * remainingKeyframePlayTime;
		outWorldLocation->z = targetLocation->z * alpha + currentLocation->z * remainingKeyframePlayTime;
	}
	return;
}

void edF32Vector4AddHard(edF32VECTOR4* v0, edF32VECTOR4* v1, edF32VECTOR4* v2)
{
	float fVar1;
	float fVar2;
	float fVar3;
	float fVar4;
	float fVar5;
	float fVar6;

	fVar1 = v1->y;
	fVar2 = v1->z;
	fVar3 = v1->w;
	fVar4 = v2->y;
	fVar5 = v2->z;
	fVar6 = v2->w;
	v0->x = v1->x + v2->x;
	v0->y = fVar1 + fVar4;
	v0->z = fVar2 + fVar5;
	v0->w = fVar3 + fVar6;
	return;
}

void edF32Matrix4GetInverseOrthoHard(edF32MATRIX4* m0, edF32MATRIX4* m1)
{
	float fVar1;
	float fVar2;
	float fVar3;
	float fVar4;
	float fVar5;
	float fVar6;
	float fVar7;
	float fVar8;
	float fVar9;
	float fVar10;
	float fVar11;
	float fVar12;
	float fVar13;

	fVar3 = m1->ac;
	fVar4 = m1->ba;
	fVar5 = m1->bb;
	fVar6 = m1->bc;
	fVar7 = m1->ca;
	fVar8 = m1->cb;
	fVar9 = m1->cc;
	fVar10 = m1->da;
	fVar11 = m1->db;
	fVar12 = m1->dc;
	fVar13 = m1->dd;
	fVar2 = m1->ab;
	fVar1 = m1->aa;
	m0->aa = fVar1;
	m0->ab = fVar4;
	m0->ac = fVar7;
	m0->ad = 0.0f;
	m0->ba = fVar2;
	m0->bb = fVar5;
	m0->bc = fVar8;
	m0->bd = 0.0;
	m0->ca = fVar3;
	m0->cb = fVar6;
	m0->cc = fVar9;
	m0->cd = 0.0f;
	m0->da = 0.0f - (fVar1 * fVar10 + fVar2 * fVar11 + fVar3 * fVar12);
	m0->db = 0.0f - (fVar4 * fVar10 + fVar5 * fVar11 + fVar6 * fVar12);
	m0->dc = 0.0f - (fVar7 * fVar10 + fVar8 * fVar11 + fVar9 * fVar12);
	m0->dd = fVar13;
	return;
}

void edF32Vector4ScaleHard(float t, edF32VECTOR4* v0, edF32VECTOR4* v1)
{
	*v0 = *v1 * t;
	return;
}

float edFIntervalDotSrcLERP(float param_1, float param_2, float param_3)
{
	float fVar1;

	fVar1 = param_2;
	if (((-1.0f < param_1) && (fVar1 = param_3, param_1 < 1.0f)) && (param_3 != param_2)) {
		fVar1 = param_2 + (param_1 + 1.0f) * (param_3 - param_2) * 0.5f;
	}
	return fVar1;
}

float edFIntervalLERP(float alpha, float param_2, float param_3, float param_4, float param_5)
{
	if (param_2 < param_3) {
		if (alpha <= param_2) {
			return param_4;
		}
		if (param_3 <= alpha) {
			return param_5;
		}
	}
	else {
		if (param_2 <= alpha) {
			return param_4;
		}
		if (alpha <= param_3) {
			return param_5;
		}
	}
	if ((param_5 != param_4) && (param_3 != param_2)) {
		param_5 = param_4 + ((alpha - param_2) * (param_5 - param_4)) / (param_3 - param_2);
	}
	return param_5;
}

void edF32Matrix4MulF32Matrix4Hard(edF32MATRIX4* dst, edF32MATRIX4* m1, edF32MATRIX4* m2)
{
	edF32MATRIX4 m1_copy = *m1;
	edF32MATRIX4 m2_copy = *m2;
	dst->aa = m1_copy.aa * m2_copy.aa + m1_copy.ab * m2_copy.ba + m1_copy.ac * m2_copy.ca + m1_copy.ad * m2_copy.da;
	dst->ab = m1_copy.aa * m2_copy.ab + m1_copy.ab * m2_copy.bb + m1_copy.ac * m2_copy.cb + m1_copy.ad * m2_copy.db;
	dst->ac = m1_copy.aa * m2_copy.ac + m1_copy.ab * m2_copy.bc + m1_copy.ac * m2_copy.cc + m1_copy.ad * m2_copy.dc;
	dst->ad = m1_copy.aa * m2_copy.ad + m1_copy.ab * m2_copy.bd + m1_copy.ac * m2_copy.cd + m1_copy.ad * m2_copy.dd;

	dst->ba = m1_copy.ba * m2_copy.aa + m1_copy.bb * m2_copy.ba + m1_copy.bc * m2_copy.ca + m1_copy.bd * m2_copy.da;
	dst->bb = m1_copy.ba * m2_copy.ab + m1_copy.bb * m2_copy.bb + m1_copy.bc * m2_copy.cb + m1_copy.bd * m2_copy.db;
	dst->bc = m1_copy.ba * m2_copy.ac + m1_copy.bb * m2_copy.bc + m1_copy.bc * m2_copy.cc + m1_copy.bd * m2_copy.dc;
	dst->bd = m1_copy.ba * m2_copy.ad + m1_copy.bb * m2_copy.bd + m1_copy.bc * m2_copy.cd + m1_copy.bd * m2_copy.dd;

	dst->ca = m1_copy.ca * m2_copy.aa + m1_copy.cb * m2_copy.ba + m1_copy.cc * m2_copy.ca + m1_copy.cd * m2_copy.da;
	dst->cb = m1_copy.ca * m2_copy.ab + m1_copy.cb * m2_copy.bb + m1_copy.cc * m2_copy.cb + m1_copy.cd * m2_copy.db;
	dst->cc = m1_copy.ca * m2_copy.ac + m1_copy.cb * m2_copy.bc + m1_copy.cc * m2_copy.cc + m1_copy.cd * m2_copy.dc;
	dst->cd = m1_copy.ca * m2_copy.ad + m1_copy.cb * m2_copy.bd + m1_copy.cc * m2_copy.cd + m1_copy.cd * m2_copy.dd;

	dst->da = m1_copy.da * m2_copy.aa + m1_copy.db * m2_copy.ba + m1_copy.dc * m2_copy.ca + m1_copy.dd * m2_copy.da;
	dst->db = m1_copy.da * m2_copy.ab + m1_copy.db * m2_copy.bb + m1_copy.dc * m2_copy.cb + m1_copy.dd * m2_copy.db;
	dst->dc = m1_copy.da * m2_copy.ac + m1_copy.db * m2_copy.bc + m1_copy.dc * m2_copy.cc + m1_copy.dd * m2_copy.dc;
	dst->dd = m1_copy.da * m2_copy.ad + m1_copy.db * m2_copy.bd + m1_copy.dc * m2_copy.cd + m1_copy.dd * m2_copy.dd;
}

void edF32Matrix4SetIdentityHard(edF32MATRIX4* m0)
{
	m0->da = 0.0f;
	m0->db = 0.0f;
	m0->dc = 0.0f;
	m0->dd = 1.0f;
	m0->ca = 0.0f;
	m0->cb = 0.0f;
	m0->cc = 1.0f;
	m0->cd = 0.0f;
	m0->ba = 0.0f;
	m0->bb = 1.0f;
	m0->bc = 0.0f;
	m0->bd = 0.0f;
	m0->aa = 1.0f;
	m0->ab = 0.0f;
	m0->ac = 0.0f;
	m0->ad = 0.0f;
	return;
}

void edF32Matrix4TranslateHard(edF32MATRIX4* m0, edF32MATRIX4* m1, edF32VECTOR4* v0)
{
	float fVar1;
	float fVar2;
	float fVar3;
	float fVar4;
	float fVar5;
	float fVar6;
	float fVar7;
	float fVar8;
	float fVar9;
	float fVar10;
	float fVar11;
	float fVar12;
	float fVar13;
	float fVar14;
	float fVar15;
	float fVar16;
	float fVar17;
	float fVar18;

	fVar12 = v0->x;
	fVar13 = v0->y;
	fVar14 = v0->z;
	fVar15 = m1->da;
	fVar16 = m1->db;
	fVar17 = m1->dc;
	fVar18 = m1->dd;
	fVar1 = m1->ab;
	fVar2 = m1->ac;
	fVar3 = m1->ad;
	fVar4 = m1->ba;
	fVar5 = m1->bb;
	fVar6 = m1->bc;
	fVar7 = m1->bd;
	fVar8 = m1->ca;
	fVar9 = m1->cb;
	fVar10 = m1->cc;
	fVar11 = m1->cd;
	m0->aa = m1->aa;
	m0->ab = fVar1;
	m0->ac = fVar2;
	m0->ad = fVar3;
	m0->ba = fVar4;
	m0->bb = fVar5;
	m0->bc = fVar6;
	m0->bd = fVar7;
	m0->ca = fVar8;
	m0->cb = fVar9;
	m0->cc = fVar10;
	m0->cd = fVar11;
	m0->da = fVar15 + fVar12;
	m0->db = fVar16 + fVar13;
	m0->dc = fVar17 + fVar14;
	m0->dd = fVar18;
	return;
}

void edF32Matrix4BuildFromVectorUnitAndAngle(float t0, edF32MATRIX4* m0, edF32VECTOR4* v0)
{
	float fVar1;
	float fVar2;
	float fVar3;
	float fVar4;
	float fVar5;
	float fVar6;
	float fVar7;
	float fVar8;
	float fVar9;
	float fVar10;
	float fVar11;

	fVar7 = v0->x * v0->x;
	fVar3 = v0->x * v0->z;
	fVar1 = v0->y * v0->z;
	fVar4 = v0->y * v0->y;
	fVar5 = v0->z * v0->z;
	fVar10 = v0->x;
	fVar11 = v0->y;
	fVar8 = v0->z;
	fVar2 = v0->x * v0->y;
	fVar6 = 1.0f - cosf(t0);
	fVar9 = sinf(t0);
	m0->aa = 1.0f - fVar6 * (fVar4 + fVar5);
	fVar10 = fVar9 * fVar10;
	fVar11 = fVar9 * fVar11;
	fVar9 = fVar9 * fVar8;
	m0->ab = fVar9 + fVar6 * fVar2;
	m0->ac = -fVar11 + fVar6 * fVar3;
	m0->ad = 0.0f;
	m0->ba = -fVar9 + fVar6 * fVar2;
	m0->bb = 1.0f - fVar6 * (fVar7 + fVar5);
	m0->bc = fVar10 + fVar6 * fVar1;
	m0->bd = 0.0f;
	m0->ca = fVar11 + fVar6 * fVar3;
	m0->cb = -fVar10 + fVar6 * fVar1;
	m0->cc = 1.0f - fVar6 * (fVar7 + fVar4);
	m0->cd = 0.0f;
	m0->da = 0.0f;
	m0->db = 0.0f;
	m0->dc = 0.0f;
	m0->dd = 1.0f;
	return;
}

void BuildMatrixFromNormalAndSpeed(edF32MATRIX4* m0, edF32VECTOR4* v0, edF32VECTOR4* v1)
{
	m0->rowY = *v0;
	m0->rowZ = *v1;
	edF32Vector4CrossProductHard(&m0->rowX, &m0->rowY, &m0->rowZ);
	edF32Vector4NormalizeHard(&m0->rowX, &m0->rowX);
	edF32Vector4CrossProductHard(&m0->rowZ, &m0->rowX, &m0->rowY);
	edF32Vector4NormalizeHard(&m0->rowZ, &m0->rowZ);
	return;
}

void edF32Matrix4BuildFromVectorAndAngle(float t0, edF32MATRIX4* m0, edF32VECTOR4* v0)
{
	float fVar1;
	float fVar2;
	float fVar3;
	float fVar4;
	edF32VECTOR4 localVector;

	fVar1 = v0->x;
	fVar2 = v0->y;
	fVar3 = v0->z;
	fVar4 = 1.0f / (sqrtf(fVar1 * fVar1 + fVar2 * fVar2 + fVar3 * fVar3) + 0.0f);
	localVector.x = fVar1 * fVar4;
	localVector.y = fVar2 * fVar4;
	localVector.z = fVar3 * fVar4;
	localVector.w = 0.0f;
	edF32Matrix4BuildFromVectorUnitAndAngle(t0, m0, &localVector);
	return;
}

void F32MatrixBuildFromF32VectorAndF32Matrix(edF32MATRIX4* m0, edF32VECTOR4* v1, edF32MATRIX4* m1)
{
	edF32VECTOR4* peVar1;
	float in_vf0x = 0.0f;
	float fVar1;
	float fVar2;
	float fVar3;
	float fVar4;
	float fVar5;
	float fVar6;

	peVar1 = &m1->rowX;
	if (fabs(v1->x * m1->ba + v1->y * m1->bb + v1->z * m1->bc) <= 0.999f) {
		peVar1 = &m1->rowY;
	}

	fVar4 = peVar1->y * v1->z - v1->y * peVar1->z;
	fVar5 = peVar1->z * v1->x - v1->z * peVar1->x;
	fVar6 = peVar1->x * v1->y - v1->x * peVar1->y;
	fVar1 = v1->x;
	fVar2 = v1->y;
	fVar3 = v1->z;
	m0->aa = fVar4;
	m0->ab = fVar5;
	m0->ac = fVar6;
	m0->ad = in_vf0x;
	m0->ba = fVar2 * fVar6 - fVar5 * fVar3;
	m0->bb = fVar3 * fVar4 - fVar6 * fVar1;
	m0->bc = fVar1 * fVar5 - fVar4 * fVar2;
	m0->bd = in_vf0x;

	m0->rowZ = *v1;

	if (m0 != m1) {
		m0->rowT = m1->rowT;
	}

	return;
}

void edF32Matrix4TransposeHard(edF32MATRIX4* m0)
{
	edF32MATRIX4 m1 = *m0;
	m0->aa = m1.aa;
	m0->ab = m1.ba;
	m0->ac = m1.ca;
	m0->ad = m1.da;
	m0->ba = m1.ab;
	m0->bb = m1.bb;
	m0->bc = m1.cb;
	m0->bd = m1.db;
	m0->ca = m1.ac;
	m0->cb = m1.bc;
	m0->cc = m1.cc;
	m0->cd = m1.dc;
	m0->da = m1.ad;
	m0->db = m1.bd;
	m0->dc = m1.cd;
	m0->dd = m1.dd;
	return;
}

void edF32Matrix4MulF32Vector4Hard(edF32VECTOR4* v0, edF32MATRIX4* m0, edF32VECTOR4* v1)
{
	float fVar1;
	float fVar2;
	float fVar3;
	float fVar4;
	float fVar5;
	float fVar6;
	float fVar7;
	float fVar8;
	float fVar9;
	float fVar10;
	float fVar11;
	float fVar12;
	float fVar13;
	float fVar14;
	float fVar15;
	float fVar16;

	fVar13 = v1->x;
	fVar14 = v1->y;
	fVar15 = v1->z;
	fVar16 = v1->w;
	fVar1 = m0->ab;
	fVar2 = m0->ac;
	fVar3 = m0->ad;
	fVar4 = m0->bb;
	fVar5 = m0->bc;
	fVar6 = m0->bd;
	fVar7 = m0->cb;
	fVar8 = m0->cc;
	fVar9 = m0->cd;
	fVar10 = m0->db;
	fVar11 = m0->dc;
	fVar12 = m0->dd;
	v0->x = m0->aa * fVar13 + m0->ba * fVar14 + m0->ca * fVar15 + m0->da * fVar16;
	v0->y = fVar1 * fVar13 + fVar4 * fVar14 + fVar7 * fVar15 + fVar10 * fVar16;
	v0->z = fVar2 * fVar13 + fVar5 * fVar14 + fVar8 * fVar15 + fVar11 * fVar16;
	v0->w = fVar3 * fVar13 + fVar6 * fVar14 + fVar9 * fVar15 + fVar12 * fVar16;
	return;
}

void edF32Matrix4GetInverseSoft(edF32MATRIX4* m0, edF32MATRIX4* m1)
{
	undefined8 uVar1;
	int iVar2;
	edF32MATRIX4* peVar3;
	float fVar4;
	float fVar5;
	float fVar6;
	float fVar7;
	float fVar8;
	float fVar9;
	float fVar10;
	float fVar11;
	float fVar12;
	float fVar13;
	float fVar14;
	float fVar15;
	float fVar16;
	float fVar17;
	float fVar18;
	float fVar19;
	edF32MATRIX4 local_40;

	fVar18 = m1->aa;
	peVar3 = &local_40;
	fVar8 = m1->bb;
	fVar6 = m1->ab;
	fVar19 = m1->ba;
	fVar7 = m1->cc;
	fVar11 = m1->dd;
	fVar16 = m1->cd;
	fVar4 = m1->dc;
	fVar17 = m1->bc;
	fVar5 = m1->ac;
	fVar14 = m1->cb;
	fVar15 = m1->db;
	fVar9 = m1->ca;
	fVar10 = m1->da;
	fVar12 = m1->bd;
	fVar13 = m1->ad;

	fVar4 = (fVar5 * fVar12 - fVar13 * fVar17) * (fVar9 * fVar15 - fVar14 * fVar10) +
		(((fVar6 * fVar17 - fVar5 * fVar8) * (fVar9 * fVar11 - fVar16 * fVar10) +
			(fVar18 * fVar12 - fVar13 * fVar19) * (fVar14 * fVar4 - fVar7 * fVar15) +
			((fVar18 * fVar8 - fVar6 * fVar19) * (fVar7 * fVar11 - fVar16 * fVar4) -
				(fVar18 * fVar17 - fVar5 * fVar19) * (fVar14 * fVar11 - fVar16 * fVar15))) -
			(fVar6 * fVar12 - fVar13 * fVar8) * (fVar9 * fVar4 - fVar7 * fVar10));
	if (1.0E-10f <= fabs(fVar4)) {
		iVar2 = 4;
		fVar4 = 1.0f / fVar4;

		*peVar3 = *m1;

		m0->aa = fVar4 * (local_40.bd * (local_40.cb * local_40.dc - local_40.cc * local_40.db) +
			local_40.bb * (local_40.cc * local_40.dd - local_40.cd * local_40.dc) +
			local_40.bc * (local_40.cd * local_40.db - local_40.cb * local_40.dd));
		m0->ab = fVar4 * (local_40.cd * (local_40.ab * local_40.dc - local_40.ac * local_40.db) +
			local_40.cb * (local_40.ac * local_40.dd - local_40.ad * local_40.dc) +
			local_40.cc * (local_40.ad * local_40.db - local_40.ab * local_40.dd));
		m0->ac = fVar4 * (local_40.dd * (local_40.ab * local_40.bc - local_40.ac * local_40.bb) +
			local_40.db * (local_40.ac * local_40.bd - local_40.ad * local_40.bc) +
			local_40.dc * (local_40.ad * local_40.bb - local_40.ab * local_40.bd));
		m0->ad = fVar4 * (local_40.ad * (local_40.bc * local_40.cb - local_40.bb * local_40.cc) +
			local_40.ab * (local_40.bd * local_40.cc - local_40.bc * local_40.cd) +
			local_40.ac * (local_40.bb * local_40.cd - local_40.bd * local_40.cb));
		m0->ba = fVar4 * (local_40.ba * (local_40.cd * local_40.dc - local_40.cc * local_40.dd) +
			local_40.bc * (local_40.ca * local_40.dd - local_40.cd * local_40.da) +
			local_40.bd * (local_40.cc * local_40.da - local_40.ca * local_40.dc));
		m0->bb = fVar4 * (local_40.ca * (local_40.ad * local_40.dc - local_40.ac * local_40.dd) +
			local_40.cc * (local_40.aa * local_40.dd - local_40.ad * local_40.da) +
			local_40.cd * (local_40.ac * local_40.da - local_40.aa * local_40.dc));
		m0->bc = fVar4 * (local_40.da * (local_40.ad * local_40.bc - local_40.ac * local_40.bd) +
			local_40.dc * (local_40.aa * local_40.bd - local_40.ad * local_40.ba) +
			local_40.dd * (local_40.ac * local_40.ba - local_40.aa * local_40.bc));
		m0->bd = fVar4 * (local_40.aa * (local_40.bc * local_40.cd - local_40.bd * local_40.cc) +
			local_40.ac * (local_40.bd * local_40.ca - local_40.ba * local_40.cd) +
			local_40.ad * (local_40.ba * local_40.cc - local_40.bc * local_40.ca));
		m0->ca = fVar4 * (local_40.bb * (local_40.cd * local_40.da - local_40.ca * local_40.dd) +
			local_40.bd * (local_40.ca * local_40.db - local_40.cb * local_40.da) +
			local_40.ba * (local_40.cb * local_40.dd - local_40.cd * local_40.db));
		m0->cb = fVar4 * (local_40.cb * (local_40.ad * local_40.da - local_40.aa * local_40.dd) +
			local_40.cd * (local_40.aa * local_40.db - local_40.ab * local_40.da) +
			local_40.ca * (local_40.ab * local_40.dd - local_40.ad * local_40.db));
		m0->cc = fVar4 * (local_40.db * (local_40.ad * local_40.ba - local_40.aa * local_40.bd) +
			local_40.dd * (local_40.aa * local_40.bb - local_40.ab * local_40.ba) +
			local_40.da * (local_40.ab * local_40.bd - local_40.ad * local_40.bb));
		m0->cd = fVar4 * (local_40.ab * (local_40.ba * local_40.cd - local_40.bd * local_40.ca) +
			local_40.ad * (local_40.bb * local_40.ca - local_40.ba * local_40.cb) +
			local_40.aa * (local_40.bd * local_40.cb - local_40.bb * local_40.cd));
		m0->da = fVar4 * (local_40.bc * (local_40.cb * local_40.da - local_40.ca * local_40.db) +
			local_40.ba * (local_40.cc * local_40.db - local_40.cb * local_40.dc) +
			local_40.bb * (local_40.ca * local_40.dc - local_40.cc * local_40.da));
		m0->db = fVar4 * (local_40.cc * (local_40.ab * local_40.da - local_40.aa * local_40.db) +
			local_40.ca * (local_40.ac * local_40.db - local_40.ab * local_40.dc) +
			local_40.cb * (local_40.aa * local_40.dc - local_40.ac * local_40.da));
		m0->dc = fVar4 * (local_40.dc * (local_40.ab * local_40.ba - local_40.aa * local_40.bb) +
			local_40.da * (local_40.ac * local_40.bb - local_40.ab * local_40.bc) +
			local_40.db * (local_40.aa * local_40.bc - local_40.ac * local_40.ba));
		m0->dd = fVar4 * (local_40.ac * (local_40.ba * local_40.cb - local_40.bb * local_40.ca) +
			local_40.aa * (local_40.bb * local_40.cc - local_40.bc * local_40.cb) +
			local_40.ab * (local_40.bc * local_40.ca - local_40.ba * local_40.cc));
	}
	return;
}

void edF32Matrix4InverseOrthoSoft(edF32MATRIX4* m0)
{
	float fVar1;
	float fVar2;
	float fVar3;

	fVar1 = m0->ba;
	m0->ba = m0->ab;
	m0->ab = fVar1;
	fVar1 = m0->ca;
	m0->ca = m0->ac;
	m0->ac = fVar1;
	fVar1 = m0->cb;
	m0->cb = m0->bc;
	m0->bc = fVar1;
	fVar2 = m0->da;
	fVar3 = m0->db;
	fVar1 = m0->dc;
	m0->da = -(fVar1 * m0->ca + fVar2 * m0->aa + fVar3 * m0->ba);
	m0->db = -(fVar1 * m0->cb + fVar2 * m0->ab + fVar3 * m0->bb);
	m0->dc = -(fVar1 * m0->cc + fVar2 * m0->ac + fVar3 * m0->bc);
	return;
}

void edF32Matrix4InverseSoft(edF32MATRIX4* m0)
{
	edF32Matrix4GetInverseSoft(m0, m0);
	return;
}

bool edF32Matrix4GetInverseGaussSoft(edF32MATRIX4* param_1, edF32MATRIX4* param_2)
{
	float* pfVar1;
	bool bVar2;
	int iVar3;
	int iVar4;
	int iVar5;
	float fVar6;
	float fVar7;
	float local_a0[4];
	float* local_90[4];

	float auStack128[8];
	float auStack96[8];
	float auStack64[8];
	float afStack32[8];

	local_90[0] = afStack32;
	local_90[1] = auStack64;
	local_90[2] = auStack96;
	local_90[3] = auStack128;

	for (iVar5 = 0; iVar5 < 4; iVar5 = iVar5 + 1) {
		for (iVar4 = 0; iVar4 < 4; iVar4 = iVar4 + 1) {
			local_90[iVar5][iVar4] = (&param_2->aa)[iVar5 * 4 + iVar4];

			if (iVar5 == iVar4) {
				local_90[iVar5][iVar4 + 4] = 1.0f;
			}
			else {
				local_90[iVar5][iVar4 + 4] = 0.0f;
			}
		}
	}

	iVar5 = 0;
	while (true) {
		if (3 < iVar5) {
			for (iVar5 = 0; iVar5 < 4; iVar5 = iVar5 + 1) {
				fVar6 = fabs(local_90[iVar5][iVar5] / local_a0[iVar5]);
				iVar3 = iVar5;
				iVar4 = iVar5;

				while (iVar4 = iVar4 + 1, iVar4 < 4) {
					if (fVar6 < fabs(local_90[iVar4][iVar5] / local_a0[iVar4])) {
						fVar6 = fabs(local_90[iVar4][iVar5] / local_a0[iVar4]);
						iVar3 = iVar4;
					}
				}

				iVar4 = iVar5;
				if (iVar3 != iVar5) {
					pfVar1 = local_90[iVar5];
					local_90[iVar5] = local_90[iVar3];
					local_90[iVar3] = pfVar1;
					fVar6 = local_a0[iVar5];
					local_a0[iVar5] = local_a0[iVar3];
					local_a0[iVar3] = fVar6;
				}

				while (iVar4 = iVar4 + 1, iVar4 < 4) {
					fVar7 = local_90[iVar5][iVar5];
					fVar6 = local_90[iVar4][iVar5];
					local_90[iVar4][iVar5] = 0.0f;
					iVar3 = iVar5;

					while (iVar3 = iVar3 + 1, iVar3 < 8) {
						local_90[iVar4][iVar3] = local_90[iVar4][iVar3] - (fVar6 / fVar7) * local_90[iVar5][iVar3];
					}
				}
			}

			iVar5 = 3;

			if (fabs(*(float*)((ulong)local_90[3] + 0xc)) < g_TinyFloat_00448548) {
				bVar2 = false;
			}
			else {
				for (; iVar4 = iVar5, 0 < iVar5; iVar5 = iVar5 + -1) {
					while (iVar4 = iVar4 + -1, -1 < iVar4) {
						fVar7 = local_90[iVar5][iVar5];
						fVar6 = local_90[iVar4][iVar5];
						iVar3 = iVar4;
						while (iVar3 = iVar3 + 1, iVar3 < 8) {
							local_90[iVar4][iVar3] = local_90[iVar4][iVar3] - (fVar6 / fVar7) * local_90[iVar5][iVar3];
						}
					}
				}

				for (iVar5 = 0; iVar5 < 4; iVar5 = iVar5 + 1) {
					for (iVar4 = 0; iVar4 < 4; iVar4 = iVar4 + 1) {
						(&param_1->aa)[iVar5 * 4 + iVar4] = local_90[iVar5][iVar4 + 4] / local_90[iVar5][iVar5];
					}
				}

				bVar2 = true;
			}
			return bVar2;
		}

		local_a0[iVar5] = fabs(*local_90[iVar5]);

		for (iVar4 = 1; iVar4 < 4; iVar4 = iVar4 + 1) {
			if (local_a0[iVar5] < fabs(local_90[iVar5][iVar4])) {
				local_a0[iVar5] = fabs(local_90[iVar5][iVar4]);
			}
		}

		if (local_a0[iVar5] < g_TinyFloat_00448548) break;

		iVar5 = iVar5 + 1;
	}
	return false;
}

void edF32Vector4ScaleV4Hard(edF32VECTOR4* v0, edF32VECTOR4* v1, edF32VECTOR4* v2)
{
	float fVar1;
	float fVar2;
	float fVar3;
	float fVar4;
	float fVar5;
	float fVar6;

	fVar1 = v1->y;
	fVar2 = v1->z;
	fVar3 = v1->w;
	fVar4 = v2->y;
	fVar5 = v2->z;
	fVar6 = v2->w;
	v0->x = v2->x * v1->x;
	v0->y = fVar4 * fVar1;
	v0->z = fVar5 * fVar2;
	v0->w = fVar6 * fVar3;
	return;
}

void edF32Vector4SquareHard(edF32VECTOR4* v0, edF32VECTOR4* v1)
{
	float fVar1;
	float fVar2;
	float fVar3;

	fVar1 = v1->y;
	fVar2 = v1->z;
	fVar3 = v1->w;
	v0->x = v1->x * v1->x;
	v0->y = fVar1 * fVar1;
	v0->z = fVar2 * fVar2;
	v0->w = fVar3 * fVar3;
	return;
}

float edF32Vector4DotProductSoft(edF32VECTOR4* v0, edF32VECTOR4* v1)
{
	return v0->z * v1->z + v0->x * v1->x + v0->y * v1->y;
}

void edF32Vector4CrossProductSoft(edF32VECTOR4* resultVector, edF32VECTOR4* vector1, edF32VECTOR4* vector2)
{
	float fVar1;
	float fVar2;
	float fVar3;
	float fVar4;
	float fVar5;
	float fVar6;

	fVar6 = vector2->z;
	fVar2 = vector1->y;
	fVar3 = vector2->y;
	fVar4 = vector1->z;
	fVar5 = vector2->x;
	fVar1 = vector1->x;
	resultVector->x = fVar2 * fVar6 - fVar4 * fVar3;
	resultVector->y = fVar4 * fVar5 - fVar1 * fVar6;
	resultVector->z = fVar1 * fVar3 - fVar2 * fVar5;
	resultVector->w = 0.0f;
	return;
}

void edF32Vector4NormalizeSoft(edF32VECTOR4* v0, edF32VECTOR4* v1)
{
	float v1Magnitude;
	float fVar1;

	fVar1 = v1->x;
	v1Magnitude = sqrtf(fVar1 * fVar1 + v1->y * v1->y + v1->z * v1->z);
	v0->x = fVar1 / v1Magnitude;
	v0->y = v1->y / v1Magnitude;
	v0->z = v1->z / v1Magnitude;
	v0->w = v1->w;
	return;
}

void edF32Matrix4BuildFromVectorUnitSoft(edF32MATRIX4* m0, edF32VECTOR4* v0)
{
	float fVar1;
	edF32VECTOR4 local_30;
	edF32VECTOR4 local_20;
	edF32VECTOR4 local_10;

	local_10.y = 1.0f;
	local_10.x = 0.0f;
	local_10.z = 0.0f;
	local_10.w = 0.0f;
	fVar1 = edF32Vector4DotProductSoft(v0, &local_10);
	if (0.999f < fabs(fVar1)) {
		local_10.x = 0.0f;
		local_10.z = 1.0f;
		local_10.y = 0.0f;
		local_10.w = 0.0f;
	}
	edF32Vector4CrossProductSoft(&local_20, &local_10, v0);
	edF32Vector4NormalizeSoft(&local_20, &local_20);
	edF32Vector4CrossProductSoft(&local_30, v0, &local_20);
	edF32Vector4NormalizeSoft(&local_30, &local_30);
	m0->aa = local_20.x;
	m0->ab = local_20.y;
	m0->ac = local_20.z;
	m0->ad = local_20.w;
	m0->ba = local_30.x;
	m0->bb = local_30.y;
	m0->bc = local_30.z;
	m0->bd = local_30.w;
	m0->ca = v0->x;
	m0->cb = v0->y;
	m0->cc = v0->z;
	m0->cd = v0->w;
	m0->da = 0.0f;
	m0->db = 0.0f;
	m0->dc = 0.0f;
	m0->dd = 0.0f;
	m0->dd = 1.0f;
	return;
}

float edF32Vector4SafeNormalize1Hard(edF32VECTOR4* v0, edF32VECTOR4* v1)
{
	float fVar1;
	float fVar2;
	float fVar3;
	float v1Magnitude;

	fVar3 = gF32Vector4UnitZ.w;
	fVar2 = gF32Vector4UnitZ.z;
	fVar1 = gF32Vector4UnitZ.y;

	v1Magnitude = sqrtf(v1->x * v1->x + v1->y * v1->y + v1->z * v1->z) + 0.0f;

	if (v1Magnitude < g_TinyFloat_00448548) {
		v0->x = gF32Vector4UnitZ.x;
		v0->y = fVar1;
		v0->z = fVar2;
		v0->w = fVar3;
	}
	else {
		const float v1RecipricalMagnitude = 1.0f / v1Magnitude;
		fVar1 = v1->y;
		fVar2 = v1->z;
		fVar3 = v1->w;
		v0->x = v1->x * v1RecipricalMagnitude;
		v0->y = fVar1 * v1RecipricalMagnitude;
		v0->z = fVar2 * v1RecipricalMagnitude;
		v0->w = fVar3 * v1RecipricalMagnitude;
	}

	return v1Magnitude;
}

float edF32Vector4GetDistHard(edF32VECTOR4* v0)
{
	return sqrtf(v0->x * v0->x + v0->y * v0->y + v0->z * v0->z) + 0.0f;
}

float edF32Vector4SafeNormalize0Hard(edF32VECTOR4* v0, edF32VECTOR4* v1)
{
	float v1Magnitude;
	v1Magnitude = sqrtf(v1->x * v1->x + v1->y * v1->y + v1->z * v1->z) + 0.0f;

	if (v1Magnitude < 1e-06f) {
		*v0 = gF32Vector4Zero;
	}
	else {
		const float invMagnitude = 1.0f / v1Magnitude;
		*v0 = *v1 * invMagnitude;
	}

	return v1Magnitude;
}

float edF32Vector4DotProductHard(edF32VECTOR4* v0, edF32VECTOR4* v1)
{
	return v0->x * v1->x + v0->y * v1->y + v0->z * v1->z;
}

float edFIntervalUnitSrcLERP(float start, float end, float alpha)
{
	float fVar1;

	fVar1 = end;
	if (((0.0f < start) && (fVar1 = alpha, start < 1.0f)) && (alpha != end)) {
		fVar1 = end + start * (alpha - end);
	}
	return fVar1;
}

bool edProjectVectorOnPlane(float projectionFactor, edF32VECTOR4* pResult, edF32VECTOR4* pInput, edF32VECTOR4* pPlaneNormal, int optionFlag)
{
	bool bVar1;
	float fVar2;
	float clampedProjectionFactor;

	clampedProjectionFactor = 1.0f;
	if ((projectionFactor <= 1.0f) && (clampedProjectionFactor = projectionFactor, projectionFactor < 0.0f)) {
		clampedProjectionFactor = 0.0f;
	}

	fVar2 = pPlaneNormal->x * pInput->x + pPlaneNormal->y * pInput->y + pPlaneNormal->z * pInput->z;
	if ((optionFlag == 0) || (fVar2 <= 0.0f)) {
		fVar2 = fVar2 * (clampedProjectionFactor + 1.0f);

		*pResult = *pInput - (*pPlaneNormal * fVar2);
		bVar1 = true;
	}
	else {
		bVar1 = false;
		*pResult = *pInput;
	}

	return bVar1;
}

bool edReflectVectorOnPlane(float reflectionFactor, edF32VECTOR4* pResult, edF32VECTOR4* pInput, edF32VECTOR4* pPlaneNormal, int mode)
{
	bool bVar1;
	float fVar2;
	float fVar3;

	fVar3 = 1.0f;
	if ((reflectionFactor <= 1.0f) && (fVar3 = reflectionFactor, reflectionFactor < 0.0f)) {
		fVar3 = 0.0f;
	}
	fVar2 = pPlaneNormal->x * pInput->x + pPlaneNormal->y * pInput->y + pPlaneNormal->z * pInput->z;
	if ((mode == 0) || (fVar2 <= 0.0f)) {
		fVar2 = fVar2 * 2.0f;
		*pResult = (*pInput - (*pPlaneNormal * fVar2)) * fVar3;
		bVar1 = true;
	}
	else {
		bVar1 = false;
		*pResult = *pInput;
	}
	return bVar1;
}

void FUN_00193060(edF32MATRIX4* param_1, edF32VECTOR4* param_2, edF32VECTOR4* pLookAt)
{
	float fVar1;
	float fVar2;
	float fVar3;
	float fVar4;
	float fVar5;
	float fVar6;

	fVar1 = pLookAt->y;
	fVar2 = pLookAt->z;
	fVar3 = pLookAt->w;
	fVar4 = param_2->y;
	fVar5 = param_2->z;
	fVar6 = param_2->w;
	param_1->ca = pLookAt->x - param_2->x;
	param_1->cb = fVar1 - fVar4;
	param_1->cc = fVar2 - fVar5;
	param_1->cd = fVar3 - fVar6;
	fVar1 = param_1->ca;
	fVar2 = param_1->cb;
	fVar3 = param_1->cc;
	fVar4 = 1.0f / (sqrtf(fVar1 * fVar1 + fVar2 * fVar2 + fVar3 * fVar3) + 0.0f);
	param_1->ca = fVar1 * fVar4;
	param_1->cb = fVar2 * fVar4;
	param_1->cc = fVar3 * fVar4;
	param_1->cd = 0.0f;
	fVar1 = g_xVector.z * param_1->ca;
	fVar2 = g_xVector.x * param_1->cb;
	fVar3 = param_1->cc * g_xVector.x;
	fVar4 = param_1->ca * g_xVector.y;
	param_1->aa = g_xVector.y * param_1->cc - param_1->cb * g_xVector.z;
	param_1->ab = fVar1 - fVar3;
	param_1->ac = fVar2 - fVar4;
	param_1->ad = 0.0f;
	fVar1 = param_1->aa;
	fVar2 = param_1->ab;
	fVar3 = param_1->ac;
	fVar4 = 1.0f / (sqrtf(fVar1 * fVar1 + fVar2 * fVar2 + fVar3 * fVar3) + 0.0f);
	param_1->aa = fVar1 * fVar4;
	param_1->ab = fVar2 * fVar4;
	param_1->ac = fVar3 * fVar4;
	param_1->ad = 0.0f;
	param_1->ba = param_1->cb * param_1->ac - param_1->ab * param_1->cc;
	param_1->bb = param_1->cc * param_1->aa - param_1->ac * param_1->ca;
	param_1->bc = param_1->ca * param_1->ab - param_1->aa * param_1->cb;
	param_1->bd = 0.0f;
	fVar3 = param_2->y;
	fVar1 = param_2->z;
	fVar2 = param_2->w;
	param_1->da = param_2->x;
	param_1->db = fVar3;
	param_1->dc = fVar1;
	param_1->dd = fVar2;
	return;
}

float edFIntervalUnitDstLERP(float param_1, float param_2, float param_3)
{
	float fVar1;

	if (param_2 < param_3) {
		if (param_1 <= param_2) {
			return 0.0f;
		}
		if (param_3 <= param_1) {
			return 1.0f;
		}
	}
	else {
		if (param_2 <= param_1) {
			return 0.0f;
		}
		if (param_1 <= param_3) {
			return 1.0f;
		}
	}
	if (param_3 == param_2) {
		fVar1 = 1.0f;
	}
	else {
		fVar1 = (param_1 - param_2) / (param_3 - param_2);
	}
	return fVar1;
}

void edF32Vector4LERPHard(float t, edF32VECTOR4* v0, edF32VECTOR4* v1, edF32VECTOR4* v2)
{
	float fVar1;
	float fVar2;
	float fVar3;
	float fVar4;
	float fVar5;
	float fVar6;
	float fVar7;

	fVar2 = v1->y;
	fVar3 = v1->z;
	fVar4 = v1->w;
	fVar5 = v2->y;
	fVar6 = v2->z;
	fVar7 = v2->w;
	fVar1 = 1.0f - t;
	v0->x = v2->x * t + v1->x * fVar1;
	v0->y = fVar5 * t + fVar2 * fVar1;
	v0->z = fVar6 * t + fVar3 * fVar1;
	v0->w = fVar7 * t + fVar4 * fVar1;
	return;
}

float GetAngleXFromVector(edF32VECTOR4* v0)
{
	float pitch;
	float fVar1;

	pitch = atan2f(-v0->y, sqrt(v0->z * v0->z + v0->x * v0->x));
	/* Second Param: 6.283185 (2 Pi) */
	fVar1 = fmodf(pitch, 6.283185f);
	return fVar1;
}

float GetAngleYFromVector(edF32VECTOR4* v0)
{
	float fVar1;

	fVar1 = atan2f(v0->x, v0->z);
	fVar1 = fmodf(fVar1, 6.283185f);
	return fVar1;
}

float edF32GetAnglesDelta(float t0, float t1)
{
	float fVar1;

	fVar1 = fmodf(t1 - t0, 6.283185f);
	if (fVar1 < -3.141593f) {
		fVar1 = fVar1 + 6.283185f;
	}
	else {
		if (3.141593f <= fVar1) {
			fVar1 = fVar1 - 6.283185f;
		}
	}
	return fVar1;
}

float edF32Between_2Pi(float param_1)
{
	float fVar1;

	fVar1 = fmodf(param_1, 6.283185);
	return fVar1;
}

float edF32Between_0_2Pi(float param_1)
{
	float fVar1;

	fVar1 = fmodf(param_1, 6.283185f);
	if (fVar1 < 0.0f) {
		fVar1 = fVar1 + 6.283185f;
	}
	return fVar1;
}

float edF32Between_0_2Pi_Incr(float param_1)
{
	if (param_1 < 0.0f) {
		do {
			param_1 = param_1 + 6.283185f;
		} while (param_1 < 0.0f);
	}

	for (; 6.283185f < param_1; param_1 = param_1 - 6.283185f) {
	}

	return param_1;
}

float edF32Between_Pi(float param_1)
{
	float fVar1;

	fVar1 = fmodf(param_1, 6.283185f);
	if (fVar1 < -3.141593f) {
		fVar1 = fVar1 + 6.283185f;
	}
	else {
		if (3.141593f <= fVar1) {
			fVar1 = fVar1 - 6.283185f;
		}
	}
	return fVar1;
}

void edF32Vector4GetNegHard(edF32VECTOR4* v0, edF32VECTOR4* v1)
{
	v0->x = 0.0f - v1->x;
	v0->y = 0.0f - v1->y;
	v0->z = 0.0f - v1->z;
	v0->w = v1->w;
	return;
}

float edF32Vector4GetLengthSoft(edF32VECTOR4* v0)
{
	return sqrtf(v0->w * v0->w + v0->z * v0->z + v0->y * v0->y + v0->x * v0->x);
}

void edF32Vector2Sub(edF32VECTOR2* v0, edF32VECTOR2* v1, edF32VECTOR2* v2)
{
	v0->x = v1->x - v2->x;
	v0->y = v1->y - v2->y;
	return;
}

void edF32Vector2LERP(edF32VECTOR2* pDst, edF32VECTOR2* pA, edF32VECTOR2* pB, float alpha)
{
	pDst->x = pA->x * (1.0f - alpha) + pB->x * alpha;
	pDst->y = pB->y * alpha + pA->y * (1.0f - alpha);


	return;
}

void edF32Vector2Mul(edF32VECTOR2* v0, edF32VECTOR2* v1, edF32VECTOR2* v2)
{
	v0->x = v1->x * v2->x;
	v0->y = v1->y * v2->y;

	return;
}

void edF32Vector2Add(edF32VECTOR2* v0, edF32VECTOR2* v1, edF32VECTOR2* v2)
{
	v0->x = v1->x + v2->x;
	v0->y = v1->y + v2->y;

	return;
}

float edF32Vector4DotProductHard_I(edF32VECTOR4* v0, edF32VECTOR4* v1)
{
	return v0->x * v1->x + v0->y * v1->y + v0->z * v1->z;
}

void edF32Vector4SubHard_I(edF32VECTOR4* v0, edF32VECTOR4* v1, edF32VECTOR4* v2)
{
	v0->x = v1->x - v2->x;
	v0->y = v1->y - v2->y;
	v0->z = v1->z - v2->z;
	v0->w = v1->w - v2->w;

	return;
}

float edF32ASinSoft(float param_1)
{
	bool bVar1;
	int iVar2;
	float fVar3;

	iVar2 = -1;
	if (0.0f < param_1) {
		iVar2 = 1;
	}
	else {
		param_1 = -param_1;
	}

	static float FLOAT_004485d0 = 1.0E-4f;
	static float FLOAT_004485e4 = 0.16666752f;
	static float FLOAT_004485e0 = 0.074953005f;
	static float FLOAT_004485dc = 0.045470025f;
	static float FLOAT_004485d8 = 0.024181312f;
	static float FLOAT_004485d4 = 0.0421632f;

	if (FLOAT_004485d0 <= param_1) {
		bVar1 = 0.5f < param_1;
		if (bVar1) {
			fVar3 = (1.0f - param_1) * 0.5f;
			param_1 = sqrtf(fVar3);
		}
		else {
			fVar3 = param_1 * param_1;
		}
		param_1 = param_1 + param_1 * fVar3 * (FLOAT_004485e4 + fVar3 * (FLOAT_004485e0 + fVar3 * (FLOAT_004485dc + (FLOAT_004485d8 + FLOAT_004485d4 * fVar3) * fVar3)));

		if (bVar1) {
			param_1 = M_PI_2f - (param_1 + param_1);
		}
	}
	if (iVar2 < 0) {
		param_1 = -param_1;
	}

	return param_1;
}

float edF32ACosSoft(float param_1)
{
	float fVar1;

	if (param_1 < -0.5f) {
		fVar1 = edF32ASinSoft(sqrtf((param_1 + 1.0f) * 0.5f));
		fVar1 = M_PI - fVar1 * 2.0f;
	}
	else {
		if (0.5f < param_1) {
			fVar1 = edF32ASinSoft(sqrtf((1.0f - param_1) * 0.5f));
			fVar1 = fVar1 * 2.0f;
		}
		else {
			fVar1 = edF32ASinSoft(param_1);
			fVar1 = M_PI_2f - fVar1;
		}
	}

	return fVar1;
}

float edF32ACosHard(float param_1)
{
	return edF32ACosSoft(param_1);
}
