#include "MathOps.h"
#include <math.h>
#include <cmath>

#define absf std::abs

edF32MATRIX4 gF32Matrix4Zero = { };
edF32MATRIX4 gF32Matrix4Unit = { 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f };

#define M_PI_2f 1.5707963f
#define M_PIf 3.14159265f
#define M_2PIf 6.283185f

void edF32Vector4NormalizeHard(float* v0, float* v1)
{
	float fVar1;
	float fVar2;
	float fVar3;
	float fVar4;

	fVar1 = *v1;
	fVar2 = v1[1];
	fVar3 = v1[2];
	fVar4 = 1.0 / (sqrtf(fVar1 * fVar1 + fVar2 * fVar2 + fVar3 * fVar3) + 0.0);
	*v0 = fVar1 * fVar4;
	v0[1] = fVar2 * fVar4;
	v0[2] = fVar3 * fVar4;
	v0[3] = 0.0;
	return;
}

void sceVu0ApplyMatrix(edF32VECTOR4* v0, edF32MATRIX4* m0, edF32VECTOR4* v1)
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

void edF32Matrix4GetTransposeHard(edF32MATRIX4* m0, edF32MATRIX4* m1)
{
#if 1
	m1->aa = m0->aa;
	m1->ab = m0->ba;
	m1->ac = m0->ca;
	m1->ad = m0->da;
	m1->ba = m0->ab;
	m1->bb = m0->bb;
	m1->bc = m0->cb;
	m1->bd = m0->db;
	m1->ca = m0->ac;
	m1->cb = m0->bc;
	m1->cc = m0->cc;
	m1->cd = m0->dc;
	m1->da = m0->ad;
	m1->db = m0->bd;
	m1->dc = m0->cd;
	m1->dd = m0->dd;
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

void sceVu0InverseMatrix(edF32MATRIX4* m0, edF32MATRIX4* m1)
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
	m0->bd = 0.0f;
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
	float in_vf0x;
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

void ps2_vu0_sqr_vector(edF32VECTOR4* v0, edF32VECTOR4* v1)
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

void GetAnglesFromVector(edF32VECTOR4* pitchAngles, edF32VECTOR4* velocity)
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
	float fVar1;
	float fVar2;
	float fVar3;

	fVar1 = v1->y;
	fVar2 = v1->z;
	fVar3 = v1->w;
	v0->x = v1->x * t;
	v0->y = fVar1 * t;
	v0->z = fVar2 * t;
	v0->w = fVar3 * t;
	return;
}

void edF32Matrix4MulF32Matrix4Hard(edF32MATRIX4* m0, edF32MATRIX4* m1, edF32MATRIX4* m2)
{
	m0->aa = m1->aa * m2->aa + m1->ab * m2->ba + m1->ac * m2->ca + m1->ad * m2->da;
	m0->ab = m1->aa * m2->ab + m1->ab * m2->bb + m1->ac * m2->cb + m1->ad * m2->db;
	m0->ac = m1->aa * m2->ac + m1->ab * m2->bc + m1->ac * m2->cc + m1->ad * m2->dc;
	m0->ad = m1->aa * m2->ad + m1->ab * m2->bd + m1->ac * m2->cd + m1->ad * m2->dd;

	m0->ba = m1->ba * m2->aa + m1->bb * m2->ba + m1->bc * m2->ca + m1->bd * m2->da;
	m0->bb = m1->ba * m2->ab + m1->bb * m2->bb + m1->bc * m2->cb + m1->bd * m2->db;
	m0->bc = m1->ba * m2->ac + m1->bb * m2->bc + m1->bc * m2->cc + m1->bd * m2->dc;
	m0->bd = m1->ba * m2->ad + m1->bb * m2->bd + m1->bc * m2->cd + m1->bd * m2->dd;

	m0->ca = m1->ca * m2->aa + m1->cb * m2->ba + m1->cc * m2->ca + m1->cd * m2->da;
	m0->cb = m1->ca * m2->ab + m1->cb * m2->bb + m1->cc * m2->cb + m1->cd * m2->db;
	m0->cc = m1->ca * m2->ac + m1->cb * m2->bc + m1->cc * m2->cc + m1->cd * m2->dc;
	m0->cd = m1->ca * m2->ad + m1->cb * m2->bd + m1->cc * m2->cd + m1->cd * m2->dd;

	m0->da = m1->da * m2->aa + m1->db * m2->ba + m1->dc * m2->ca + m1->dd * m2->da;
	m0->db = m1->da * m2->ab + m1->db * m2->bb + m1->dc * m2->cb + m1->dd * m2->db;
	m0->dc = m1->da * m2->ac + m1->db * m2->bc + m1->dc * m2->cc + m1->dd * m2->dc;
	m0->dd = m1->da * m2->ad + m1->db * m2->bd + m1->dc * m2->cd + m1->dd * m2->dd;
}

void edF32Matrix4SetIdentityHard(edF32MATRIX4* m0)
{
	m0->da = 0.0;
	m0->db = 0.0;
	m0->dc = 0.0;
	m0->dd = 1.0;
	m0->ca = 0.0;
	m0->cb = 0.0;
	m0->cc = 1.0;
	m0->cd = 0.0;
	m0->ba = 0.0;
	m0->bb = 1.0;
	m0->bc = 0.0;
	m0->bd = 0.0;
	m0->aa = 1.0;
	m0->ab = 0.0;
	m0->ac = 0.0;
	m0->ad = 0.0;
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

