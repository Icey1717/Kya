#include "MathOps.h"
#include <math.h>
#include <cmath>

#define absf std::abs

sceVu0FMATRIX g_ZeroMatrix_00431690 = { 0 };
edF32MATRIX4 g_IdentityMatrix = { 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f };

#define M_PI_2f 1.5707963f
#define M_PIf 3.14159265f
#define M_2PIf 6.283185f

#ifdef PLATFORM_WIN
void sceVu0Normalize(float* v0, float* v1)
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
#endif

void sceVu0ApplyMatrix(Vector* v0, edF32MATRIX4* m0, Vector* v1)
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

void sceVu0TransposeMatrixFixed(sceVu0FMATRIX m0, sceVu0FMATRIX m1)
{
#if 1
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			m0[i][j] = m1[j][i];
		}
	}
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

void ps2_vu0_sub_vector(Vector* v0, Vector* v1, Vector* v2)
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

void ps2_vu0_outer_product(Vector* v0, Vector* v1, Vector* v2)
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

void ps2_vu0_sqr_vector(Vector* v0, Vector* v1)
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

void CalculatePitchAngles2D_00193c70(Vector* pitchAngles, Vector* velocity)
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

void RotateMatrixByAngle(float angle, edF32MATRIX4* m0, edF32MATRIX4* m1)
{
	edF32MATRIX4 local_40;

	local_40.bb = cosf(angle);
	local_40.bc = sinf(angle);
	local_40.ab = 0.0;
	local_40.ac = 0.0;
	local_40.ad = 0.0;
	local_40.aa = 1.0;
	local_40.cb = -local_40.bc;
	local_40.ba = 0.0;
	local_40.bd = 0.0;
	local_40.ca = 0.0;
	local_40.cd = 0.0;
	local_40.da = 0.0;
	local_40.db = 0.0;
	local_40.dc = 0.0;
	local_40.dd = 1.0;
	local_40.cc = local_40.bb;
	sceVu0MulMatrix((TO_SCE_MTX)m0, (TO_SCE_MTX)m1, (TO_SCE_MTX)&local_40);
	return;
}

void ApplyRotationToMatrix(float angle, edF32MATRIX4* outputMatrix, edF32MATRIX4* inputMatrix)
{
	edF32MATRIX4 local_40;

	local_40.aa = cosf(angle);
	local_40.ca = sinf(angle);
	local_40.ab = 0.0;
	local_40.ac = -local_40.ca;
	local_40.ad = 0.0;
	local_40.ba = 0.0;
	local_40.bb = 1.0;
	local_40.bc = 0.0;
	local_40.bd = 0.0;
	local_40.cb = 0.0;
	local_40.cd = 0.0;
	local_40.da = 0.0;
	local_40.db = 0.0;
	local_40.dc = 0.0;
	local_40.dd = 1.0;
	local_40.cc = local_40.aa;
	sceVu0MulMatrix((TO_SCE_MTX)outputMatrix, (TO_SCE_MTX)inputMatrix, (TO_SCE_MTX)&local_40);
	return;
}

void MatrixSetPosition_00267750(sceVu0FMATRIX m0, sceVu0FMATRIX m1, sceVu0FVECTOR v0)
{
	undefined8 uVar1;
	float fVar2;
	float fVar3;
	int iVar4;
	sceVu0FMATRIX local_40;

	sceVu0CopyMatrix(local_40, g_ZeroMatrix_00431690);

	local_40[0][0] = (v0)[0];
	local_40[1][1] = (v0)[1];
	local_40[2][2] = (v0)[2];
	local_40[3][3] = 1.0;
	sceVu0MulMatrix(m0, m1, local_40);
	return;
}

void CalculateRotationMatrix_002673f0(float t0, edF32MATRIX4* m0, edF32MATRIX4* m1)
{
	sceVu0FMATRIX local_40;

	local_40[0][0] = cosf(t0);
	local_40[0][1] = sinf(t0);
	local_40[0][2] = 0.0;
	local_40[1][0] = -local_40[0][1];
	local_40[0][3] = 0.0;
	local_40[1][2] = 0.0;
	local_40[1][3] = 0.0;
	local_40[2][0] = 0.0;
	local_40[2][1] = 0.0;
	local_40[2][2] = 1.0;
	local_40[2][3] = 0.0;
	local_40[3][0] = 0.0;
	local_40[3][1] = 0.0;
	local_40[3][2] = 0.0;
	local_40[3][3] = 1.0;
	local_40[1][1] = local_40[0][0];
	sceVu0MulMatrix((TO_SCE_MTX)m0, (TO_SCE_MTX)m1, local_40);
	return;
}

void ComputeMapRectangleMatrix_0029b9e0
(float startX, float endX, float startY, float endY, float startZ, float endZ, float startW, float endW, edF32MATRIX4* m0)
{
	float fVar1;
	float fVar2;

	fVar2 = 1.0 / (endX - startX);
	fVar1 = 1.0 / (endY - startY);
	m0->aa = (endZ - startZ) * fVar2;
	m0->ab = 0.0;
	m0->ac = 0.0;
	m0->ad = 0.0;
	m0->ba = 0.0;
	m0->bb = (endW - startW) * fVar1;
	m0->bc = 0.0;
	m0->bd = 0.0;
	m0->ca = 0.0;
	m0->cb = 0.0;
	m0->cc = 1.0;
	m0->cd = 0.0;
	m0->da = fVar2 * (startZ * (endX - startX) - startX * (endZ - startZ));
	m0->db = fVar1 * (startW * (endY - startY) - startY * (endW - startW));
	m0->dc = 0.0;
	m0->dd = 1.0;
	return;
}

void CalculateYAxisTransformMatrix_0029ba70(float x, float y, float yMin, float yMax, edF32MATRIX4* m0)
{
	float fVar1;
	float fVar2;

	m0->aa = 1.0;
	m0->ab = 0.0;
	m0->ac = 0.0;
	m0->ad = 0.0;
	m0->ba = 0.0;
	fVar1 = 1.0 / (yMax - yMin);
	m0->bb = 1.0;
	m0->bc = 0.0;
	m0->bd = 0.0;
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


