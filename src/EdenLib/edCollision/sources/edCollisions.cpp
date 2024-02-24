#include "edCollisions.h"
#include "MathOps.h"
#include "port/pointer_conv.h"

GlobalCollisionData gColData;
CollisionTD gColTD;

edColConfig gColConfig;

void edColComputeMatrices(edColPRIM_OBJECT* pPrimObj)
{
	float puVar4;
	float puVar5;
	float puVar6;
	float puVar1;
	float puVar2;
	float puVar3;
	edF32VECTOR4* pfVar1;
	float fVar2;
	float fVar3;
	float fVar4;
	float fVar5;
	float fVar6;
	float fVar7;
	float fVar8;
	float fVar9;
	edF32VECTOR4 local_10;

	pfVar1 = &pPrimObj->field_0x90;

	fVar2 = cosf(pPrimObj->field_0xa0.x);
	fVar4 = sinf(pPrimObj->field_0xa0.x);

	fVar3 = cosf(pPrimObj->field_0xa0.y);
	fVar6 = sinf(pPrimObj->field_0xa0.y);

	fVar7 = cosf(pPrimObj->field_0xa0.z);
	fVar8 = sinf(pPrimObj->field_0xa0.z);

	(pPrimObj->matrix_0x0).aa = pfVar1->x * fVar7 * fVar3;
	(pPrimObj->matrix_0x0).ab = pfVar1->x * fVar3 * fVar8;
	(pPrimObj->matrix_0x0).ac = pfVar1->x * -fVar6;
	(pPrimObj->matrix_0x0).ad = 0.0f;
	(pPrimObj->matrix_0x0).ba = pPrimObj->field_0x90.y * (fVar7 * fVar4 * fVar6 - fVar2 * fVar8);
	fVar9 = fVar7 * fVar2;
	(pPrimObj->matrix_0x0).bb = pPrimObj->field_0x90.y * (fVar6 * fVar8 * fVar4 + fVar9);
	(pPrimObj->matrix_0x0).bc = pPrimObj->field_0x90.y * fVar4 * fVar3;
	(pPrimObj->matrix_0x0).bd = 0.0f;
	fVar5 = fVar6 * fVar9 + fVar8 * fVar4;
	(pPrimObj->matrix_0x0).ca = pPrimObj->field_0x90.z * fVar5;
	(pPrimObj->matrix_0x0).cb = pPrimObj->field_0x90.z * (fVar6 * fVar2 * fVar8 - fVar4 * fVar7);
	(pPrimObj->matrix_0x0).cc = pPrimObj->field_0x90.z * fVar2 * fVar3;
	(pPrimObj->matrix_0x0).cd = 0.0f;
	(pPrimObj->matrix_0x0).da = (pPrimObj->field_0xb0).x;
	(pPrimObj->matrix_0x0).db = (pPrimObj->field_0xb0).y;
	(pPrimObj->matrix_0x0).dc = (pPrimObj->field_0xb0).z;
	(pPrimObj->matrix_0x0).dd = 1.0;

	(pPrimObj->matrix_0x40).aa = (fVar7 * fVar3) / pfVar1->x;
	(pPrimObj->matrix_0x40).ab = (fVar6 * fVar4 * fVar7 + -fVar2 * fVar8) / pPrimObj->field_0x90.y;
	(pPrimObj->matrix_0x40).ac = fVar5 / pPrimObj->field_0x90.z;
	(pPrimObj->matrix_0x40).ad = 0.0f;
	(pPrimObj->matrix_0x40).ba = (fVar3 * fVar8) / pfVar1->x;
	(pPrimObj->matrix_0x40).bb = (fVar9 + fVar4 * fVar6 * fVar8) / pPrimObj->field_0x90.y;
	(pPrimObj->matrix_0x40).bc = (fVar8 * fVar2 * fVar6 + -fVar4 * fVar7) / pPrimObj->field_0x90.z;
	(pPrimObj->matrix_0x40).bd = 0.0f;
	(pPrimObj->matrix_0x40).ca = -fVar6 / pfVar1->x;
	(pPrimObj->matrix_0x40).cb = (fVar4 * fVar3) / pPrimObj->field_0x90.y;
	(pPrimObj->matrix_0x40).cc = (fVar2 * fVar3) / pPrimObj->field_0x90.z;
	(pPrimObj->matrix_0x40).cd = 0.0f;
	(pPrimObj->matrix_0x40).da = 0.0f;
	(pPrimObj->matrix_0x40).db = 0.0f;
	(pPrimObj->matrix_0x40).dc = 0.0f;
	(pPrimObj->matrix_0x40).dd = 1.0;

	local_10.x = -(pPrimObj->field_0xb0).x;
	local_10.y = -(pPrimObj->field_0xb0).y;
	local_10.z = -(pPrimObj->field_0xb0).z;
	local_10.w = 1.0;
	edF32Matrix4MulF32Vector4Hard(&local_10, &pPrimObj->matrix_0x40, &local_10);
	(pPrimObj->matrix_0x40).rowT = local_10;
	return;
}

edColG3D_OBB_TREE* edColLoadStatic(char** pOutData, char* pFileBuffer, uint* param_3, uint bConvertTriangles)
{
	byte bVar1;
	void* pvVar2;
	StaticCollisionEntry* pStaticColEntry;
	edF32TRIANGLE4* peVar3;
	int* piVar4;
	edObbTREE* pObbTree;
	int iVar6;
	float* pfVar7;
	int iVar8;

	pStaticColEntry = (StaticCollisionEntry*)*pOutData;

	if (*param_3 == 0) {
		pStaticColEntry = (StaticCollisionEntry*)&pStaticColEntry->obbTree;
	}

	*pOutData = (char*)(&pStaticColEntry->field_0x0 + pStaticColEntry->field_0x8);
	if (*pOutData == pFileBuffer + *(int*)(pFileBuffer + 8)) {
		*pOutData = (char*)0x0;
	}

	if ((pStaticColEntry->obbTree).bLoaded == 0) {
		(pStaticColEntry->obbTree).bLoaded = 1;

		{
			const int offset = (pStaticColEntry->obbTree).field_0x28;
			if (offset != 0) {
				(pStaticColEntry->obbTree).field_0x28 = STORE_SECTION(pFileBuffer + offset);
			}
		}

		{
			const int offset = (pStaticColEntry->obbTree).aTriangles;
			if (offset != 0) {
				(pStaticColEntry->obbTree).aTriangles = STORE_SECTION(pFileBuffer + offset);
			}
		}

		{
			const int offset = (pStaticColEntry->obbTree).field_0x30;
			if (offset != 0) {
				(pStaticColEntry->obbTree).field_0x30 = STORE_SECTION(pFileBuffer + offset);
			}
		}

		{
			const int offset = (pStaticColEntry->obbTree).aQuads;
			if (offset != 0) {
				(pStaticColEntry->obbTree).aQuads = STORE_SECTION(pFileBuffer + offset);
			}
		}

		{
			const int offset = (pStaticColEntry->obbTree).field_0x3c;
			if (offset != 0) {
				(pStaticColEntry->obbTree).field_0x3c = STORE_SECTION(pFileBuffer + offset);
			}
		}

		{
			const int offset = (pStaticColEntry->obbTree).field_0x38;
			if (offset != 0) {
				(pStaticColEntry->obbTree).field_0x38 = STORE_SECTION(pFileBuffer + offset);
			}
		}

		{
			const int offset = (pStaticColEntry->obbTree).pObbTree;
			if (offset != 0) {
				(pStaticColEntry->obbTree).pObbTree = STORE_SECTION(pFileBuffer + offset);
			}
		}

		edF32TRIANGLE4* pTriangle = LOAD_SECTION_CAST(edF32TRIANGLE4*, (pStaticColEntry->obbTree).aTriangles);
		char* pBase = LOAD_SECTION_CAST(char*, (pStaticColEntry->obbTree).field_0x28);
		for (iVar8 = (pStaticColEntry->obbTree).nbTriangles; iVar8 != 0; iVar8 = iVar8 + -1) {
			pTriangle->p1 = STORE_SECTION(pBase + pTriangle->p1 * 0x10);
			pTriangle->p2 = STORE_SECTION(pBase + pTriangle->p2 * 0x10);
			pTriangle->p3 = STORE_SECTION(pBase + pTriangle->p3 * 0x10);
			pTriangle = pTriangle + 1;
		}

		edF32QUAD4* pQuad = LOAD_SECTION_CAST(edF32QUAD4*, (pStaticColEntry->obbTree).aQuads);

		for (iVar6 = (pStaticColEntry->obbTree).nbQuads; iVar6 != 0; iVar6 = iVar6 + -1) {
			pQuad->p1 = STORE_SECTION(pBase + pQuad->p1 * 0x10);
			pQuad->p2 = STORE_SECTION(pBase + pQuad->p2 * 0x10);
			pQuad->p3 = STORE_SECTION(pBase + pQuad->p3 * 0x10);
			pQuad->p4 = STORE_SECTION(pBase + pQuad->p4 * 0x10);
			pQuad = pQuad + 1;
		}

		edObbTREE* pObbTreeBase = (edObbTREE*)LOAD_SECTION((pStaticColEntry->obbTree).pObbTree);

		pObbTree = pObbTreeBase;
		for (iVar6 = (pStaticColEntry->obbTree).field_0x20; iVar6 != 0; iVar6 = iVar6 + -1) {
			bVar1 = pObbTree->type;
			if (bVar1 == 10) {
				pObbTree->field_0x54[0] = STORE_SECTION(((char*)LOAD_SECTION((pStaticColEntry->obbTree).field_0x38) + pObbTree->field_0x54[0] * 0x90));
			}
			else {
				if (bVar1 == 0xb) {
					pObbTree->field_0x54[0] = STORE_SECTION(((char*)LOAD_SECTION((pStaticColEntry->obbTree).field_0x3c) + pObbTree->field_0x54[0] * 0x90));
				}
				else {
					if (bVar1 == 8) {
						pObbTree->field_0x54[0] = STORE_SECTION(((char*)LOAD_SECTION((pStaticColEntry->obbTree).aQuads) + pObbTree->field_0x54[0] * sizeof(edF32QUAD4)));
					}
					else {
						if (bVar1 == 5) {
							pObbTree->field_0x54[0] = STORE_SECTION(((char*)LOAD_SECTION((pStaticColEntry->obbTree).field_0x30) + pObbTree->field_0x54[0] * 0x18));
						}
						else {
							if (bVar1 == 4) {
								pObbTree->field_0x54[0] = STORE_SECTION(((char*)LOAD_SECTION((pStaticColEntry->obbTree).aTriangles) + pObbTree->field_0x54[0] * 0x10));
							}
							else {
								iVar8 = 0;
								if (bVar1 == 1) {
									bVar1 = pObbTree->count_0x52;
									for (; iVar8 < (int)(uint)bVar1; iVar8 = iVar8 + 1) {
										edObbTREE* pNext = (edObbTREE*)((char*)pObbTreeBase + (pObbTree->field_0x54[iVar8] * sizeof(edObbTREE)));
										pObbTree->field_0x54[iVar8] = STORE_SECTION(pNext);
									}
								}
							}
						}
					}
				}
			}
			
			pObbTree += 1;
		}
		if (bConvertTriangles != 0) {
			IMPLEMENTATION_GUARD(
			edColConvertToTriangle4Fast(&pStaticColEntry->obbTree);)
		}
	}
	return &pStaticColEntry->obbTree;
}

edColG3D_OBB_TREE* edColLoadStatic(char* pFileBuffer, uint length, uint bConvertTriangles)
{
	edColG3D_OBB_TREE* peVar1;
	uint local_8;
	char* local_4;

	local_8 = 0;
	local_4 = pFileBuffer;
	peVar1 = edColLoadStatic(&local_4, pFileBuffer, &local_8, bConvertTriangles);
	return peVar1;
}

bool edObbIntersect(edObbBOX* pBoxA, edObbBOX* pBoxB)
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
	float fVar19;
	float fVar20;
	float fVar21;
	float fVar23;
	float fVar24;
	float fVar25;
	edF32VECTOR3 fVar22;
	float fVar26;
	float fVar27;
	float fVar28;
	float fVar29;
	float fVar30;
	float fVar31;
	edF32VECTOR3 fVar32;
	edF32VECTOR3 fVar33;
	edF32VECTOR3 fVar34;

	fVar25 = pBoxA->width;
	fVar23 = pBoxA->height;
	fVar24 = pBoxA->depth;
	fVar20 = pBoxB->width;
	fVar21 = pBoxB->height;
	fVar19 = pBoxB->depth;
	fVar22.x = (pBoxA->transform).aa;
	fVar22.y = (pBoxA->transform).ab;
	fVar22.z = (pBoxA->transform).ac;
	fVar32.x = (pBoxB->transform).aa;
	fVar32.y = (pBoxB->transform).ab;
	fVar32.z = (pBoxB->transform).ac;
	fVar33.x = (pBoxB->transform).ba;
	fVar33.y = (pBoxB->transform).bb;
	fVar33.z = (pBoxB->transform).bc;
	fVar34.x = (pBoxB->transform).ca;
	fVar34.y = (pBoxB->transform).cb;
	fVar34.z = (pBoxB->transform).cc;
	fVar1 = (pBoxB->transform).da - (pBoxA->transform).da;
	fVar14 = (pBoxB->transform).db - (pBoxA->transform).db;
	fVar18 = (pBoxB->transform).dc - (pBoxA->transform).dc;
	fVar2 = fVar22.x * fVar32.x + fVar22.y * fVar32.y + fVar22.z * fVar32.z;
	fVar11 = fVar22.y * fVar33.y + fVar22.x * fVar33.x + fVar22.z * fVar33.z;
	fVar15 = fVar22.z * fVar34.z + fVar22.x * fVar34.x + fVar22.y * fVar34.y;
	fVar3 = fVar22.x * fVar1 + fVar22.y * fVar14 + fVar22.z * fVar18;
	fVar4 = fabs(fVar2);
	fVar7 = fabs(fVar11);
	fVar8 = fabs(fVar15);
	if (fabs(fVar3) <= fVar19 * fVar8 + fVar21 * fVar7 + fVar20 * fVar4 + fVar25 + 0.0001f) {
		fVar26 = (pBoxA->transform).ba;
		fVar27 = (pBoxA->transform).bb;
		fVar28 = (pBoxA->transform).bc;
		fVar5 = fVar26 * fVar32.x + fVar27 * fVar32.y + fVar28 * fVar32.z;
		fVar12 = fVar27 * fVar33.y + fVar26 * fVar33.x + fVar28 * fVar33.z;
		fVar16 = fVar28 * fVar34.z + fVar26 * fVar34.x + fVar27 * fVar34.y;
		fVar26 = fVar26 * fVar1 + fVar27 * fVar14 + fVar28 * fVar18;
		fVar27 = fabs(fVar5);
		fVar28 = fabs(fVar12);
		fVar9 = fabs(fVar16);
		if (fabs(fVar26) <= fVar19 * fVar9 + fVar21 * fVar28 + fVar20 * fVar27 + fVar23 + 0.0001f) {
			fVar29 = (pBoxA->transform).ca;
			fVar30 = (pBoxA->transform).cb;
			fVar31 = (pBoxA->transform).cc;
			fVar6 = fVar29 * fVar32.x + fVar30 * fVar32.y + fVar31 * fVar32.z;
			fVar13 = fVar30 * fVar33.y + fVar29 * fVar33.x + fVar31 * fVar33.z;
			fVar17 = fVar31 * fVar34.z + fVar29 * fVar34.x + fVar30 * fVar34.y;
			fVar29 = fVar29 * fVar1 + fVar30 * fVar14 + fVar31 * fVar18;
			fVar30 = fabs(fVar6);
			fVar31 = fabs(fVar13);
			fVar10 = fabs(fVar17);
			if (((((fabs(fVar29) <= fVar19 * fVar10 + fVar21 * fVar31 + fVar20 * fVar30 + fVar24 + 0.0001f) &&
				(fabs(fVar32.x * fVar1 + fVar32.y * fVar14 + fVar32.z * fVar18) <=
					fVar24 * fVar30 + fVar23 * fVar27 + fVar25 * fVar4 + fVar20 + 0.0001f)) &&
				(fabs(fVar33.x * fVar1 + fVar33.y * fVar14 + fVar33.z * fVar18) <=
					fVar24 * fVar31 + fVar23 * fVar28 + fVar25 * fVar7 + fVar21 + 0.0001f)) &&
				(((fabs(fVar34.x * fVar1 + fVar34.y * fVar14 + fVar34.z * fVar18) <=
					fVar24 * fVar10 + fVar23 * fVar9 + fVar25 * fVar8 + fVar19 + 0.0001f &&
					(fabs(fVar29 * fVar5 - fVar26 * fVar6) <=
						fVar19 * fVar7 + fVar21 * fVar8 + fVar24 * fVar27 + fVar23 * fVar30 + 0.0001f + 0.0f)) &&
					((fabs(fVar29 * fVar12 - fVar26 * fVar13) <=
						fVar19 * fVar4 + fVar20 * fVar8 + fVar24 * fVar28 + fVar23 * fVar31 + 0.0001f + 0.0f &&
						((fabs(fVar29 * fVar16 - fVar26 * fVar17) <=
							fVar21 * fVar4 + fVar20 * fVar7 + fVar24 * fVar9 + fVar23 * fVar10 + 0.0001f + 0.0f &&
							(fabs(fVar3 * fVar6 - fVar29 * fVar2) <=
								fVar19 * fVar28 + fVar21 * fVar9 + fVar24 * fVar4 + fVar25 * fVar30 + 0.0001f + 0.0f)))))))) &&
				((fabs(fVar3 * fVar13 - fVar29 * fVar11) <=
					fVar19 * fVar27 + fVar20 * fVar9 + fVar24 * fVar7 + fVar25 * fVar31 + 0.0001f + 0.0f &&
					((((fabs(fVar3 * fVar17 - fVar29 * fVar15) <=
						fVar21 * fVar27 + fVar20 * fVar28 + fVar24 * fVar8 + fVar25 * fVar10 + 0.0001f + 0.0f &&
						(fabs(fVar26 * fVar2 - fVar3 * fVar5) <=
							fVar19 * fVar31 + fVar21 * fVar10 + fVar23 * fVar4 + fVar25 * fVar27 + 0.0001f + 0.0f)) &&
						(fabs(fVar26 * fVar11 - fVar3 * fVar12) <=
							fVar19 * fVar30 + fVar20 * fVar10 + fVar23 * fVar7 + fVar25 * fVar28 + 0.0001f + 0.0f)) &&
						(fabs(fVar26 * fVar15 - fVar3 * fVar16) <=
							fVar21 * fVar30 + fVar20 * fVar31 + fVar23 * fVar8 + fVar25 * fVar9 + 0.0001f + 0.0f)))))) {
				return true;
			}
		}
	}
	return false;
}

static int _gColSizeOfPrims[21] = { 
	0x0, 0x0, 0x0, 0x0,
	0x0, 0x0, 0x0, 0x0,
	0x0, 0x0, 0x90, 0x90,
	0x90, 0x150, 0x150, 0x150,
	0x0, 0x0, 0x0, 0x1D0,
	0x0 
};

float edColSqrDistancePointTriangle(edF32VECTOR4* v0, edF32TRIANGLE4* t0)
{
	edF32VECTOR4* peVar1;
	edF32VECTOR4* peVar2;
	edF32VECTOR4* peVar3;
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

	peVar1 = (edF32VECTOR4*)LOAD_SECTION(t0->p1);
	peVar2 = (edF32VECTOR4*)LOAD_SECTION(t0->p2);
	peVar3 = (edF32VECTOR4*)LOAD_SECTION(t0->p3);

	fVar13 = peVar1->x;
	fVar14 = peVar1->y;
	fVar15 = peVar1->z;
	fVar4 = peVar2->x - fVar13;
	fVar11 = peVar2->y - fVar14;
	fVar9 = peVar2->z - fVar15;
	fVar5 = peVar3->x - fVar13;
	fVar12 = peVar3->y - fVar14;
	fVar10 = peVar3->z - fVar15;
	fVar13 = fVar13 - v0->x;
	fVar14 = fVar14 - v0->y;
	fVar15 = fVar15 - v0->z;
	fVar6 = fVar4 * fVar4 + fVar11 * fVar11 + fVar9 * fVar9;
	fVar7 = fVar4 * fVar5 + fVar11 * fVar12 + fVar9 * fVar10;
	fVar8 = fVar5 * fVar5 + fVar12 * fVar12 + fVar10 * fVar10;
	fVar4 = fVar13 * fVar4 + fVar14 * fVar11 + fVar15 * fVar9;
	fVar5 = fVar13 * fVar5 + fVar14 * fVar12 + fVar15 * fVar10;
	fVar13 = fVar13 * fVar13 + fVar14 * fVar14 + fVar15 * fVar15;
	fVar14 = fabs(fVar6 * fVar8 - fVar7 * fVar7);
	fVar11 = fVar7 * fVar5 - fVar8 * fVar4;
	fVar12 = fVar7 * fVar4 - fVar6 * fVar5;
	if (fVar11 + fVar12 <= fVar14) {
		if (fVar11 < 0.0f) {
			if (fVar12 < 0.0f) {
				if (fVar4 < 0.0f) {
					if (fVar6 <= -fVar4) {
						fVar13 = fVar13 + fVar6 + fVar4 * 2.0f;
					}
					else {
						fVar13 = fVar13 + fVar4 * (-fVar4 / fVar6);
					}
				}
				else {
					if (fVar5 < 0.0f) {
						if (fVar8 <= -fVar5) {
							fVar13 = fVar13 + fVar8 + fVar5 * 2.0f;
						}
						else {
							fVar13 = fVar13 + fVar5 * (-fVar5 / fVar8);
						}
					}
				}
			}
			else {
				if (fVar5 < 0.0f) {
					if (fVar8 <= -fVar5) {
						fVar13 = fVar13 + fVar8 + fVar5 * 2.0f;
					}
					else {
						fVar13 = fVar13 + fVar5 * (-fVar5 / fVar8);
					}
				}
			}
		}
		else {
			if (fVar12 < 0.0f) {
				if (fVar4 < 0.0f) {
					if (fVar6 <= -fVar4) {
						fVar13 = fVar13 + fVar6 + fVar4 * 2.0f;
					}
					else {
						fVar13 = fVar13 + fVar4 * (-fVar4 / fVar6);
					}
				}
			}
			else {
				fVar14 = 1.0f / fVar14;
				fVar12 = fVar12 * fVar14;
				fVar11 = fVar11 * fVar14;
				fVar13 = fVar13 + fVar12 * (fVar5 * 2.0f + fVar7 * fVar11 + fVar8 * fVar12) +
					fVar11 * (fVar4 * 2.0f + fVar6 * fVar11 + fVar7 * fVar12);
			}
		}
	}
	else {
		if (fVar11 < 0.0f) {
			fVar11 = fVar8 + fVar5;
			if (fVar7 + fVar4 < fVar11) {
				fVar11 = fVar11 - (fVar7 + fVar4);
				fVar12 = fVar8 + (fVar6 - fVar7 * 2.0f);
				if (fVar12 <= fVar11) {
					fVar13 = fVar13 + fVar6 + fVar4 * 2.0f;
				}
				else {
					fVar11 = fVar11 / fVar12;
					fVar12 = 1.0f - fVar11;
					fVar13 = fVar13 + fVar12 * (fVar5 * 2.0f + fVar7 * fVar11 + fVar8 * fVar12) +
						fVar11 * (fVar4 * 2.0f + fVar6 * fVar11 + fVar7 * fVar12);
				}
			}
			else {
				if (fVar11 <= 0.0f) {
					fVar13 = fVar13 + fVar8 + fVar5 * 2.0f;
				}
				else {
					if (fVar5 < 0.0f) {
						fVar13 = fVar13 + fVar5 * (-fVar5 / fVar8);
					}
				}
			}
		}
		else {
			if (fVar12 < 0.0f) {
				fVar11 = fVar6 + fVar4;
				if (fVar7 + fVar5 < fVar11) {
					fVar11 = fVar11 - (fVar7 + fVar5);
					fVar12 = fVar8 + (fVar6 - fVar7 * 2.0f);
					if (fVar12 <= fVar11) {
						fVar13 = fVar13 + fVar8 + fVar5 * 2.0f;
					}
					else {
						fVar11 = fVar11 / fVar12;
						fVar12 = 1.0f - fVar11;
						fVar13 = fVar13 + fVar11 * (fVar5 * 2.0f + fVar7 * fVar12 + fVar8 * fVar11) +
							fVar12 * (fVar4 * 2.0f + fVar6 * fVar12 + fVar7 * fVar11);
					}
				}
				else {
					if (fVar11 <= 0.0f) {
						fVar13 = fVar13 + fVar6 + fVar4 * 2.0f;
					}
					else {
						if (fVar4 < 0.0f) {
							fVar13 = fVar13 + fVar4 * (-fVar4 / fVar6);
						}
					}
				}
			}
			else {
				fVar11 = ((fVar8 + fVar5) - fVar7) - fVar4;
				if (fVar11 <= 0.0f) {
					fVar13 = fVar13 + fVar8 + fVar5 * 2.0f;
				}
				else {
					fVar12 = fVar8 + (fVar6 - fVar7 * 2.0f);
					if (fVar12 <= fVar11) {
						fVar13 = fVar13 + fVar6 + fVar4 * 2.0f;
					}
					else {
						fVar11 = fVar11 / fVar12;
						fVar12 = 1.0f - fVar11;
						fVar13 = fVar13 + fVar12 * (fVar5 * 2.0f + fVar7 * fVar11 + fVar8 * fVar12) +
							fVar11 * (fVar4 * 2.0f + fVar6 * fVar11 + fVar7 * fVar12);
					}
				}
			}
		}
	}
	return fabs(fVar13);
}

struct edColPRIM_IN {
	edColOBJECT* pColObj;
	edColOBJECT* pOtherColObj;
	edColPRIM_OBJECT* field_0x8;
	uint aType;
	edF32VECTOR4* field_0x10;
	edF32VECTOR4* field_0x14;
	edF32VECTOR4* field_0x18;
};

struct edColPRIM_SPHERE_QUAD4_IN : public edColPRIM_IN {
	struct edF32QUAD4* pQuad;
};

struct edColINFO_OUT
{
	edF32VECTOR4 location;
	uint result;
	edF32VECTOR4 field_0x10;
	edF32VECTOR4 field_0x20;
	edF32VECTOR4 field_0x30;
	float field_0x44;
	int field_0x48;
	int field_0x4c;
	int field_0x50;
};

struct edF32TRIANGLE4_INFOS {
	edF32VECTOR4 field_0x0;
	float field_0x10;
};

void edColTriangle4GetInfo(edF32TRIANGLE4_INFOS* pInfo, edF32TRIANGLE4* pTriangle)
{
	edF32VECTOR4* peVar1;
	edF32VECTOR4* peVar2;
	float fVar3;
	float fVar4;
	float fVar5;
	float fVar6;
	float fVar7;
	float fVar8;
	float in_vf0x;

	peVar1 = LOAD_SECTION_CAST(edF32VECTOR4*, pTriangle->p1);
	peVar2 = LOAD_SECTION_CAST(edF32VECTOR4*, pTriangle->p2);
	fVar3 = peVar2->x - peVar1->x;
	fVar5 = peVar2->y - peVar1->y;
	fVar7 = peVar2->z - peVar1->z;
	peVar2 = LOAD_SECTION_CAST(edF32VECTOR4*, pTriangle->p3);
	fVar4 = peVar2->x - peVar1->x;
	fVar6 = peVar2->y - peVar1->y;
	fVar8 = peVar2->z - peVar1->z;
	(pInfo->field_0x0).x = fVar5 * fVar8 - fVar6 * fVar7;
	(pInfo->field_0x0).y = fVar7 * fVar4 - fVar8 * fVar3;
	(pInfo->field_0x0).z = fVar3 * fVar6 - fVar4 * fVar5;
	(pInfo->field_0x0).w = in_vf0x;
	edF32Vector4NormalizeHard(&pInfo->field_0x0, &pInfo->field_0x0);
	pInfo->field_0x10 = -((pInfo->field_0x0).x * peVar1->x + (pInfo->field_0x0).y * peVar1->y + (pInfo->field_0x0).z * peVar1->z);
	return;
}

struct edColRAY_TRIANGLE4_IN {
	edF32VECTOR4* field_0x0;
	edF32VECTOR4* field_0x4;
	edF32TRIANGLE4* pTriangle;
};

const float FLOAT_0044854c = 0.001f;

bool edColIntersectRayTriangle4(float* pOutDistance, edColRAY_TRIANGLE4_IN* pRayTriangleIn)
{
	edF32TRIANGLE4* peVar1;
	edF32VECTOR4* peVar2;
	edF32VECTOR4* peVar3;
	edF32VECTOR4* peVar4;
	edF32VECTOR4* peVar5;
	edF32VECTOR4* peVar6;
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
	float fVar20;
	float fVar21;
	float fVar22;
	float fVar23;
	float fVar24;

	fVar23 = FLOAT_0044854c;
	fVar10 = g_DefaultNearClip_0044851c;
	*pOutDistance = -8888.0;
	peVar1 = pRayTriangleIn->pTriangle;
	peVar2 = pRayTriangleIn->field_0x4;
	peVar3 = pRayTriangleIn->field_0x0;
	peVar4 = LOAD_SECTION_CAST(edF32VECTOR4*, peVar1->p1);
	peVar5 = LOAD_SECTION_CAST(edF32VECTOR4*, peVar1->p2);
	peVar6 = LOAD_SECTION_CAST(edF32VECTOR4*, peVar1->p3);
	fVar14 = peVar4->x;
	fVar15 = peVar4->y;
	fVar16 = peVar4->z;
	fVar17 = peVar2->x;
	fVar18 = peVar2->y;
	fVar19 = peVar2->z;
	fVar7 = peVar5->x - fVar14;
	fVar11 = peVar5->y - fVar15;
	fVar24 = peVar5->z - fVar16;
	fVar8 = peVar6->x - fVar14;
	fVar12 = peVar6->y - fVar15;
	fVar13 = peVar6->z - fVar16;
	fVar20 = fVar18 * fVar13 - fVar12 * fVar19;
	fVar21 = fVar19 * fVar8 - fVar13 * fVar17;
	fVar22 = fVar17 * fVar12 - fVar8 * fVar18;
	fVar9 = fVar7 * fVar20 + fVar11 * fVar21 + fVar24 * fVar22;
	if (fVar23 <= fVar9) {
		fVar14 = peVar3->x - fVar14;
		fVar15 = peVar3->y - fVar15;
		fVar16 = peVar3->z - fVar16;
		fVar10 = fVar14 * fVar20 + fVar15 * fVar21 + fVar16 * fVar22;
		if (fVar10 < 0.0f) {
			return false;
		}
		if (fVar10 <= fVar9) {
			fVar23 = fVar15 * fVar24 - fVar11 * fVar16;
			fVar24 = fVar16 * fVar7 - fVar24 * fVar14;
			fVar14 = fVar14 * fVar11 - fVar7 * fVar15;
			fVar7 = fVar17 * fVar23 + fVar18 * fVar24 + fVar19 * fVar14;
			if ((0.0f <= fVar7) && (fVar10 + fVar7 <= fVar9)) {
			LAB_00250f60:
				*pOutDistance = (fVar8 * fVar23 + fVar12 * fVar24 + fVar13 * fVar14) / fVar9;
				return true;
			}
		}
	}
	else {
		if (fVar9 <= fVar10) {
			fVar14 = peVar3->x - fVar14;
			fVar15 = peVar3->y - fVar15;
			fVar16 = peVar3->z - fVar16;
			fVar10 = fVar14 * fVar20 + fVar15 * fVar21 + fVar16 * fVar22;
			if ((fVar10 <= 0.0f) && (fVar9 <= fVar10)) {
				fVar23 = fVar15 * fVar24 - fVar11 * fVar16;
				fVar24 = fVar16 * fVar7 - fVar24 * fVar14;
				fVar14 = fVar14 * fVar11 - fVar7 * fVar15;
				fVar7 = fVar17 * fVar23 + fVar18 * fVar24 + fVar19 * fVar14;
				if ((fVar7 <= 0.0f) && (fVar9 <= fVar10 + fVar7)) goto LAB_00250f60;
			}
		}
		else {
			*pOutDistance = -9999.0f;
		}
	}
	return false;
}

struct edColPRIM_RAY_SPHERE_UNIT_IN {
	edF32VECTOR4* field_0x0;
	edF32VECTOR4* field_0x4;
};

uint edColIntersectRaySphereUnit(edColINFO_OUT* pColInfoOut, edColPRIM_RAY_SPHERE_UNIT_IN* pPrimRaySphereIn)
{
	edF32VECTOR4* peVar1;
	edF32VECTOR4* peVar2;
	edF32VECTOR4* peVar3;
	edF32VECTOR4* peVar4;
	uint uVar5;
	float fVar6;
	float fVar7;
	float fVar8;
	float fVar9;
	float fVar10;
	float fVar11;
	float fVar12;
	float fVar13;
	float fVar14;

	pColInfoOut->result = 0;
	peVar1 = pPrimRaySphereIn->field_0x4;
	fVar8 = peVar1->x * peVar1->x + peVar1->y * peVar1->y + peVar1->z * peVar1->z;
	peVar1 = pPrimRaySphereIn->field_0x4;
	peVar2 = pPrimRaySphereIn->field_0x0;
	peVar3 = pPrimRaySphereIn->field_0x0;
	peVar4 = pPrimRaySphereIn->field_0x0;
	fVar9 = (peVar1->x * peVar2->x + peVar1->y * peVar2->y + peVar1->z * peVar2->z) * 2.0;
	fVar6 = fVar9 * fVar9 - fVar8 * 4.0 * ((peVar3->x * peVar4->x + peVar3->y * peVar4->y + peVar3->z * peVar4->z) - 1.0);
	uVar5 = 0xffffffff;
	if (0.0 <= fVar6) {
		fVar6 = sqrtf(fVar6);
		fVar7 = 1.0 / (fVar8 * 2.0);
		fVar8 = fVar7 * (-fVar9 + fVar6);
		fVar7 = fVar7 * (-fVar9 - fVar6);
		uVar5 = (uint)(0.0 <= fVar7);
		if ((fVar8 < 0.0) || (1.0 < fVar8)) {
			if ((fVar7 < 0.0) || (fVar8 = fVar7, 1.0 < fVar7)) {
				fVar8 = -8888.0;
			}
		}
		else {
			if ((0.0 <= fVar7) && (fVar7 <= 1.0)) {
				uVar5 = 0xffffffff;
				fVar8 = (fVar8 + fVar7) * 0.5;
			}
		}
		if (0.0 <= fVar8) {
			peVar1 = pPrimRaySphereIn->field_0x4;
			fVar6 = peVar1->x;
			fVar9 = peVar1->y;
			fVar7 = peVar1->z;
			fVar10 = peVar1->w;
			peVar1 = pPrimRaySphereIn->field_0x0;
			fVar11 = peVar1->x;
			fVar12 = peVar1->y;
			fVar13 = peVar1->z;
			fVar14 = peVar1->w;
			pColInfoOut->field_0x44 = fVar8 - 1.0;
			(pColInfoOut->field_0x20).x = fVar6 * fVar8 + fVar11;
			(pColInfoOut->field_0x20).y = fVar9 * fVar8 + fVar12;
			(pColInfoOut->field_0x20).z = fVar7 * fVar8 + fVar13;
			(pColInfoOut->field_0x20).w = fVar10 * fVar8 + fVar14;
			pColInfoOut->result = 1;
		}
	}
	return uVar5;
}

static int _gtri_edge[3][2] = {
	{0, 1}, {1, 2}, {2, 0}
};

float FLOAT_004485b0 = 0.1f;

void edColGetNormalInWorldFromLocal(edF32VECTOR4* param_1, edF32MATRIX4* param_2, edF32VECTOR4* param_3)
{
	edF32MATRIX4 eStack64;

	edF32Matrix4GetTransposeHard(&eStack64, param_2);
	edF32Matrix4MulF32Vector4Hard(param_1, &eStack64, param_3);
	param_1->w = 0.0f;
	edF32Vector4NormalizeHard(param_1, param_1);
	return;
}

void edColGetWorldVelocity
(edF32VECTOR4* param_1, edF32VECTOR4* param_2, edF32VECTOR4* param_3, edF32VECTOR4* param_4, edF32VECTOR4* param_5)
{
	float fVar1;
	float fVar2;
	float fVar3;
	float in_vf0x;
	float fVar4;
	float fVar5;
	float fVar6;
	float fVar7;
	float fVar8;
	float fVar9;

	fVar1 = param_2->x - param_3->x;
	fVar2 = param_2->y - param_3->y;
	fVar3 = param_2->z - param_3->z;
	fVar4 = param_5->x;
	fVar5 = param_5->y;
	fVar7 = param_5->z;
	fVar6 = param_4->y;
	fVar8 = param_4->z;
	fVar9 = param_4->w;
	param_1->x = param_4->x + (fVar5 * fVar3 - fVar2 * fVar7);
	param_1->y = fVar6 + (fVar7 * fVar1 - fVar3 * fVar4);
	param_1->z = fVar8 + (fVar4 * fVar2 - fVar1 * fVar5);
	param_1->w = fVar9 + in_vf0x;
	return;
}

bool edColIntersectSphereUnitTriangle4(edColINFO_OUT* pColInfoOut, edColPRIM_IN* pPrimIn, edF32TRIANGLE4* pTriangle)
{
	edColPRIM_OBJECT* m0;
	edF32VECTOR4* peVar1;
	float fVar2;
	float fVar3;
	float fVar4;
	bool bVar5;
	int iVar6;
	bool lVar7;
	int iVar8;
	int iVar9;
	float fVar10;
	edF32VECTOR4 local_140;
	edF32VECTOR4 local_130;
	edF32VECTOR4 eStack288;
	edF32VECTOR4 local_110;
	edColINFO_OUT eStack256;
	edF32VECTOR4 local_a0;
	edF32VECTOR4 local_90;
	edF32VECTOR4 local_80;
	edF32TRIANGLE4_INFOS local_70;
	float local_50;
	float fStack76;
	float fStack72;
	float fStack68;
	edF32VECTOR4 local_40;
	edF32VECTOR4 local_30;
	edColPRIM_RAY_SPHERE_UNIT_IN local_18;
	edColRAY_TRIANGLE4_IN local_10;
	float local_4;

	pColInfoOut->result = 0;
	m0 = pPrimIn->field_0x8;
	fVar10 = edColSqrDistancePointTriangle(&gF32Vertex4Zero, pTriangle);
	if (fVar10 < 1.0f) {
		edColTriangle4GetInfo(&local_70, pTriangle);
		local_40.x = 0.0f - local_70.field_0x0.x;
		local_40.y = 0.0f - local_70.field_0x0.y;
		local_40.z = 0.0f - local_70.field_0x0.z;
		local_40.w = local_70.field_0x0.w;
		local_10.field_0x4 = &local_40;
		local_10.field_0x0 = &gF32Vertex4Zero;
		local_10.pTriangle = pTriangle;
		lVar7 = edColIntersectRayTriangle4(&local_4, &local_10);
		local_80.x = 0.0f;
		iVar8 = 0;
		local_80.y = 0.0f;
		local_80.z = 0.0f;
		local_80.w = 0.0f;
		if (lVar7 != false) {
			local_50 = local_40.x * local_4;
			fStack76 = local_40.y * local_4;
			fStack72 = local_40.z * local_4;
			fStack68 = local_40.w * local_4;
			if (local_50 * local_50 + fStack76 * fStack76 + fStack72 * fStack72 < 1.0) {
				local_80.x = local_50 + 0.0f;
				local_80.y = fStack76 + 0.0f;
				local_80.z = fStack72 + 0.0f;
				local_80.w = fStack68 + 0.0f;
				iVar8 = 1;
				pColInfoOut->field_0x50 = 2;
			}
		}

		iVar9 = 0;
		if (iVar8 == 0) {
			bVar5 = true;
			while (bVar5) {
				local_18.field_0x0 = LOAD_SECTION_CAST(edF32VECTOR4*, pTriangle->points[_gtri_edge[iVar9][0]]);
				peVar1 = LOAD_SECTION_CAST(edF32VECTOR4*, pTriangle->points[_gtri_edge[iVar9][1]]);
				local_a0 = *peVar1 - *local_18.field_0x0;
				local_18.field_0x4 = &local_a0;
				iVar6 = edColIntersectRaySphereUnit(&eStack256, &local_18);
				if (eStack256.result != 0) {
					fVar10 = eStack256.field_0x20.x;
					fVar2 = eStack256.field_0x20.y;
					fVar3 = eStack256.field_0x20.z;
					fVar4 = eStack256.field_0x20.w;
					if (-1 < iVar6) {
						peVar1 = LOAD_SECTION_CAST(edF32VECTOR4*, pTriangle->points[_gtri_edge[iVar9][iVar6]]);
						fVar10 = peVar1->x;
						fVar2 = peVar1->y;
						fVar3 = peVar1->z;
						fVar4 = peVar1->w;
					}
					local_80.w = local_80.w + fVar4;
					local_80.z = local_80.z + fVar3;
					local_80.y = local_80.y + fVar2;
					local_80.x = local_80.x + fVar10;
					iVar8 = iVar8 + 1;
					pColInfoOut->field_0x50 = 1;
				}
				iVar9 = iVar9 + 1;
				bVar5 = iVar9 < 3;
			}
		}

		bVar5 = false;
		if (iVar8 != 0) {
			if (1 < iVar8) {
				fVar10 = 1.0f / (float)iVar8;
				local_80.x = local_80.x * fVar10;
				local_80.y = local_80.y * fVar10;
				local_80.z = local_80.z * fVar10;
			}

			local_80.w = 1.0f;
			edF32Vector4NormalizeHard(&local_90, &local_80);
			local_90.x = 0.0f - local_90.x;
			local_90.y = 0.0f - local_90.y;
			local_90.z = 0.0f - local_90.z;
			local_90.w = 0.0f;

			local_110 = local_80;

			edColGetNormalInWorldFromLocal(&local_90, &m0->matrix_0x40, &local_90);
			edF32Vector4NormalizeHard(&eStack288, &local_80);
			eStack288.w = 1.0f;
			edF32Matrix4MulF32Vector4Hard(&local_80, &m0->matrix_0x0, &local_80);
			edF32Matrix4MulF32Vector4Hard(&local_130, &m0->matrix_0x0, &eStack288);
			local_140 = local_130 - local_80;

			fVar10 = edF32Vector4GetDistHard(&local_140);
			edF32Vector4NormalizeHard(&local_110, &local_110);
			local_110.w = 1.0f;
			edF32Matrix4MulF32Vector4Hard(&local_80, &m0->matrix_0x0, &local_110);
			edColGetWorldVelocity(&local_30, &local_80, pPrimIn->field_0x10, pPrimIn->field_0x14, pPrimIn->field_0x18);
			if (local_30.x * local_90.x + local_30.y * local_90.y + local_30.z * local_90.z < FLOAT_004485b0) {
				bVar5 = true;
				pColInfoOut->field_0x20 = local_80;

				pColInfoOut->field_0x10 = local_90;

				pColInfoOut->field_0x30 = local_30;
				pColInfoOut->field_0x44 = -fVar10;
				pColInfoOut->field_0x48 = 0;
				pColInfoOut->field_0x4c = 1;
				pColInfoOut->result = 1;
			}
			else {
				bVar5 = false;
			}
		}
	}
	else {
		bVar5 = false;
	}
	return bVar5;
}

void edColComputeContactQuad4(edColOBJECT* pColObj, edColOBJECT* pOtherColObj, edColINFO_OUT* pColInfoOut, uint param_4, void* param_5, edColINFO* pColInfo, edF32QUAD4* pQuad, edF32TRIANGLE4* pTriangle)
{
	edF32VECTOR4* peVar1;
	int iVar2;
	edColINFO* peVar3;
	edF32VECTOR4* peVar4;
	edColDbObj_80* pColDbObj;
	float fVar5;
	float fVar6;
	float fVar7;
	float in_vf0x;
	edF32TRIANGLE4_INFOS local_90;
	edF32VECTOR4 local_70;
	edF32VECTOR4 local_60;
	edF32VECTOR4 local_40[2];

	if ((gColData.pActiveDatabase)->field_0x4 < (int)(uint)(ushort)gColConfig.field_0x8[gColData.activeDatabaseId + 2]) {
		pColDbObj = (gColData.pActiveDatabase)->field_0x10 + (gColData.pActiveDatabase)->field_0x4;
		pColDbObj->field_0x72 = 0;
		(gColData.pActiveDatabase)->field_0x4 = (gColData.pActiveDatabase)->field_0x4 + 1;
	}
	else {
		pColDbObj = (edColDbObj_80*)0x0;
	}

	if (pColDbObj != (edColDbObj_80*)0x0) {
		pColDbObj->pColObj = pColObj;
		pColDbObj->pOtherColObj = pOtherColObj;

		pColDbObj->field_0x30 = pColInfoOut->field_0x20;
		pColDbObj->field_0x40 = pColInfoOut->field_0x10;
		pColDbObj->location = pColInfoOut->location;

		pColDbObj->field_0x78 = pColInfoOut->field_0x44;
		pColDbObj->quadFlags = pQuad->flags;
		pColDbObj->field_0x73 = (char)pColInfoOut->field_0x50;
		pColDbObj->field_0x60 = (short)param_4;
		pColDbObj->field_0x64 = param_5;
		pColDbObj->field_0x62 = 8;
		pColDbObj->pQuad = pQuad;

		if (gColConfig.field_0x4 != 0) {
			edColTriangle4GetInfo(&local_90, pTriangle);
			pColDbObj->field_0x10 = pColInfoOut->field_0x30;

			if (pColInfo == (edColINFO*)0x0) {
				pColDbObj->field_0x70 = 1;
				pColDbObj->field_0x71 = 1;
			}
			else {
				pColDbObj->field_0x70 = pColInfo->field_0x4a;
				pColDbObj->field_0x71 = pColInfo->field_0x4b;
			}

			pColDbObj->field_0x6c = pColInfo;
			pColObj->field_0x6 = pColObj->field_0x6 + 1;

			if (pOtherColObj != (edColOBJECT*)0x0) {
				pOtherColObj->field_0x6 = pOtherColObj->field_0x6 + 1;
			}

			if (pColInfo != (edColINFO*)0x0) {
				pColInfo->field_0x49 = 1;
			}

			local_60 = local_90.field_0x0;

			peVar4 = LOAD_SECTION_CAST(edF32VECTOR4*, pTriangle->p1);
			peVar1 = LOAD_SECTION_CAST(edF32VECTOR4*, pTriangle->p3);

			local_70.xyz = peVar1->xyz - peVar4->xyz;
			local_70.w = 0.0f;
			edF32Vector4NormalizeHard(&local_70, &local_70);
			peVar4 = local_40;
			local_40[0].x = local_60.y * local_70.z - local_70.y * local_60.z;
			local_40[0].y = local_60.z * local_70.x - local_70.z * local_60.x;
			local_40[0].z = local_60.x * local_70.y - local_70.x * local_60.y;
			local_40[0].w = in_vf0x;
			local_40[1].x = local_60.x;
			local_40[1].y = local_60.y;
			local_40[1].z = local_60.z;
			local_40[1].w = local_60.w;

			if ((pColInfo != (edColINFO*)0x0) && ((pQuad->flags & 0x80000000) == 0)) {
				pColInfo->field_0x44 = 0x0;

				pColInfo->field_0x0.rowX = peVar4[0];
				pColInfo->field_0x0.rowY = peVar4[1];

				pColInfo->field_0x48 = 4;
				pColInfo->field_0x40 = STORE_SECTION(pTriangle);
				pColInfo->field_0x44 = pTriangle->flags;
			}
		}
	}
	return;
}

void edColIntersectSphereQuad4(edColINFO_OUT* pColInfoOut, edColPRIM_SPHERE_QUAD4_IN* pPrimSphereQuadIn)
{
	edF32QUAD4* peVar1;
	edColPRIM_OBJECT* pUVar2;
	bool bVar3;
	uint uVar4;
	edF32MATRIX4* m0;
	int iVar5;
	edF32TRIANGLE4_INFOS local_80;
	edF32TRIANGLE4 local_60;
	uint local_54;
	edF32TRIANGLE4 local_50;
	uint local_44;
	edF32VECTOR4 eStack64;
	edF32VECTOR4 eStack48;
	edF32VECTOR4 eStack32;
	edF32VECTOR4 eStack16;

	uVar4 = 0;
	peVar1 = pPrimSphereQuadIn->pQuad;
	m0 = &(pPrimSphereQuadIn->field_0x8)->matrix_0x40;
	edF32Matrix4MulF32Vector4Hard(&eStack64, m0, (edF32VECTOR4*)LOAD_SECTION(peVar1->p1));
	edF32Matrix4MulF32Vector4Hard(&eStack48, m0, (edF32VECTOR4*)LOAD_SECTION(peVar1->p2));
	edF32Matrix4MulF32Vector4Hard(&eStack32, m0, (edF32VECTOR4*)LOAD_SECTION(peVar1->p3));
	edF32Matrix4MulF32Vector4Hard(&eStack16, m0, (edF32VECTOR4*)LOAD_SECTION(peVar1->p4));
	local_44 = peVar1->flags;
	
	local_60.p1 = STORE_SECTION(&eStack64);
	iVar5 = 0;
	local_54 = peVar1->flags;
	local_50.p1 = peVar1->p1;
	do {
		if (iVar5 == 0) {
			local_50.p2 = peVar1->p2;
			local_50.p3 = peVar1->p3;
			local_60.p2 = STORE_SECTION(&eStack48);
			local_60.p3 = STORE_SECTION(&eStack32);
		}
		else {
			local_50.p2 = peVar1->p3;
			local_60.p3 = STORE_SECTION(&eStack16);
			local_50.p3 = peVar1->p4;
			local_60.p2 = STORE_SECTION(&eStack32);
		}
		bVar3 = edColIntersectSphereUnitTriangle4(pColInfoOut, pPrimSphereQuadIn, &local_60);
		if (bVar3 != false) {
			edColTriangle4GetInfo(&local_80, &local_50);
			pColInfoOut->location = local_80.field_0x0;
			pUVar2 = pPrimSphereQuadIn->field_0x8;
			edColComputeContactQuad4(pPrimSphereQuadIn->pColObj, pPrimSphereQuadIn->pOtherColObj, pColInfoOut, pPrimSphereQuadIn->aType, pUVar2, &pUVar2->colInfo, peVar1, &local_50);
			uVar4 = uVar4 | pColInfoOut->result;
		}
		iVar5 = iVar5 + 1;
	} while (iVar5 < 2);
	pColInfoOut->result = uVar4;
	return;
}


struct edColARRAY_PRIM_TRI4 {
	edColOBJECT* pColObj;
	int aCount;
	int aType;
	int primSize;
	void* aData;
	edColOBJECT* pOtherColObj;
	int bCount;
	void* bData;
};

struct edColARRAY_PRIM_QUAD4 {
	edColOBJECT* pColObj;
	int aCount;
	int aType;
	int primSize;
	void* aData;
	edColOBJECT* pOtherColObj;
	int bCount;
	void* bData;
};

struct edColPRIM_BOX_TRI4_IN {
	edColOBJECT* pColObj;
	edColOBJECT* pOtherColObj;
	edColPRIM_OBJECT* pData;
	int aType;
	edF32VECTOR4* field_0x10;
	edF32VECTOR4* field_0x14;
	edF32VECTOR4* field_0x18;
	edF32TRIANGLE4* pTriangle;
};

struct edColPRIM_SPHERE_TRI4_IN {
	edColOBJECT* pColObj;
	edColOBJECT* pOtherColObj;
	edColPRIM_OBJECT* pData;
	int aType;
	edF32VECTOR4* field_0x10;
	edF32VECTOR4* field_0x14;
	edF32VECTOR4* field_0x18;
	edF32TRIANGLE4* pTriangle;
};

void edColComputeContactTriangle4(edColOBJECT* pColObj, edColOBJECT* pOtherColObj, edColINFO_OUT* pColInfoOut, uint param_4, void* param_5,
	edColINFO* pColInfo, edF32TRIANGLE4* pTriangle, int param_8)
{
	edF32VECTOR4* peVar1;
	edF32VECTOR4* peVar2;
	int iVar3;
	edColINFO* peVar4;
	float* pfVar5;
	edColDbObj_80* peVar6;
	float fVar7;
	float fVar8;
	float fVar9;
	undefined4 in_vf0x;
	edF32TRIANGLE4_INFOS local_90;
	edF32VECTOR4 local_70;
	edF32VECTOR4 local_60;
	edF32MATRIX4 local_40;

	if ((gColData.pActiveDatabase)->field_0x4 < gColConfig.field_0x8[gColData.activeDatabaseId + 2]) {
		peVar6 = (gColData.pActiveDatabase)->field_0x10 + (gColData.pActiveDatabase)->field_0x4;
		peVar6->field_0x72 = 0;
		(gColData.pActiveDatabase)->field_0x4 = (gColData.pActiveDatabase)->field_0x4 + 1;
	}
	else {
		peVar6 = (edColDbObj_80*)0x0;
	}

	if (peVar6 != (edColDbObj_80*)0x0) {
		peVar6->pColObj = pColObj;
		peVar6->pOtherColObj = pOtherColObj;

		peVar6->field_0x30 = pColInfoOut->field_0x20;

		peVar6->location = pColInfoOut->location;

		peVar6->field_0x40 = pColInfoOut->field_0x10;
		
		peVar6->field_0x78 = pColInfoOut->field_0x44;
		peVar6->quadFlags = pTriangle->flags;
		peVar6->field_0x73 = (char)pColInfoOut->field_0x50;

		if (param_8 == 0) {
			peVar6->field_0x60 = (short)param_4;
			peVar6->field_0x64 = param_5;
			peVar6->field_0x62 = 4;
			peVar6->pQuad = (edF32QUAD4*)pTriangle;
		}
		else {
			peVar6->field_0x62 = (short)param_4;
			peVar6->pQuad = (edF32QUAD4*)param_5;
			peVar6->field_0x60 = 4;
			peVar6->field_0x64 = pTriangle;
		}

		if (gColConfig.field_0x4 != 0) {
			edColTriangle4GetInfo(&local_90, pTriangle);
			peVar6->field_0x6c = pColInfo;
			if (pColInfo == (edColINFO*)0x0) {
				peVar6->field_0x70 = 1;
				peVar6->field_0x71 = 1;
			}
			else {
				peVar6->field_0x70 = pColInfo->field_0x4a;
				peVar6->field_0x71 = pColInfo->field_0x4b;
			}

			peVar6->field_0x10 = pColInfoOut->field_0x30;

			pColObj->field_0x6 = pColObj->field_0x6 + 1;
			if (pOtherColObj != (edColOBJECT*)0x0) {
				pOtherColObj->field_0x6 = pOtherColObj->field_0x6 + 1;
			}
			if (pColInfo != (edColINFO*)0x0) {
				pColInfo->field_0x49 = 1;
			}

			local_60 = local_90.field_0x0;

			peVar1 = LOAD_SECTION_CAST(edF32VECTOR4*, pTriangle->p1);
			peVar2 = LOAD_SECTION_CAST(edF32VECTOR4*, pTriangle->p3);
			local_70.x = peVar2->x - peVar1->x;
			local_70.y = peVar2->y - peVar1->y;
			local_70.z = peVar2->z - peVar1->z;
			local_70.w = 0.0f;

			edF32Vector4NormalizeHard(&local_70, &local_70);

			local_40.aa = local_60.y * local_70.z - local_70.y * local_60.z;
			local_40.ab = local_60.z * local_70.x - local_70.z * local_60.x;
			local_40.ac = local_60.x * local_70.y - local_70.x * local_60.y;
			local_40.ad = in_vf0x;
			local_40.rowY = local_60;
			local_40.rowZ = local_70;
			local_40.rowT = *LOAD_SECTION_CAST(edF32VECTOR4*, pTriangle->p1);

			if ((pColInfo != (edColINFO*)0x0) && ((pTriangle->flags & 0x80000000) == 0)) {
				pColInfo->field_0x44 = 0x0;

				pColInfo->field_0x0 = local_40;

				pColInfo->field_0x48 = 4;
				pColInfo->field_0x40 = STORE_SECTION(pTriangle);
				pColInfo->field_0x44 = pTriangle->flags;
			}
		}
	}
	return;
}

void edColIntersectSphereTriangle4(edColINFO_OUT* pColInfoOut, edColPRIM_SPHERE_TRI4_IN* pPrimSphereTriIn, int param_3)
{
	edF32TRIANGLE4* pTriangle;
	bool bVar1;
	edF32MATRIX4* m0;
	edF32TRIANGLE4_INFOS local_60;
	edF32TRIANGLE4 local_40;
	edF32VECTOR4 eStack48;
	edF32VECTOR4 eStack32;
	edF32VECTOR4 eStack16;

	pTriangle = pPrimSphereTriIn->pTriangle;
	m0 = &pPrimSphereTriIn->pData->matrix_0x40;
	edF32Matrix4MulF32Vector4Hard(&eStack48, m0, LOAD_SECTION_CAST(edF32VECTOR4*, pTriangle->p1));
	local_40.p2 = STORE_SECTION(&eStack32);
	edF32Matrix4MulF32Vector4Hard(LOAD_SECTION_CAST(edF32VECTOR4*, local_40.p2), m0, LOAD_SECTION_CAST(edF32VECTOR4*, pTriangle->p2));
	local_40.p3 = STORE_SECTION(&eStack16);
	edF32Matrix4MulF32Vector4Hard(LOAD_SECTION_CAST(edF32VECTOR4*, local_40.p3), m0, LOAD_SECTION_CAST(edF32VECTOR4*, pTriangle->p3));
	local_40.p1 = STORE_SECTION(&eStack48);
	local_40.flags = pTriangle->flags;

	bVar1 = edColIntersectSphereUnitTriangle4(pColInfoOut, (edColPRIM_IN*)pPrimSphereTriIn, &local_40);
	if (bVar1 != false) {
		edColTriangle4GetInfo(&local_60, pTriangle);
		pColInfoOut->location = local_60.field_0x0;
		edColComputeContactTriangle4(pPrimSphereTriIn->pColObj, pPrimSphereTriIn->pOtherColObj, pColInfoOut, pPrimSphereTriIn->aType,
			pPrimSphereTriIn->pData, &pPrimSphereTriIn->pData->colInfo, pTriangle, param_3);
	}
	return;
}

uint edColArrayObjectPrimPenatratingArrayTriangles4(edColARRAY_PRIM_TRI4* pParams)
{
	int iVar1;
	int iVar2;
	edF32VECTOR4* peVar3;
	int iVar4;
	edColOBJECT* peVar5;
	uint uVar6;
	edF32VECTOR4* peVar7;
	edColPRIM_OBJECT* pUVar8;
	edColPRIM_OBJECT* pUVar9;
	edColINFO_OUT eStack160;
	edColPRIM_SPHERE_TRI4_IN local_40;
	edColPRIM_BOX_TRI4_IN local_20;

	iVar1 = pParams->primSize;
	uVar6 = 0;
	iVar2 = pParams->bCount;
	peVar3 = (edF32VECTOR4*)pParams->bData;
	pUVar8 = (edColPRIM_OBJECT*)pParams->aData;
	iVar4 = pParams->aType;
	peVar5 = pParams->pColObj;
	pUVar9 = (edColPRIM_OBJECT*)((char*)pUVar8 + pParams->aCount * iVar1);
	if ((iVar4 == 10) || (iVar4 == 0xd)) {
		local_20.pOtherColObj = pParams->pOtherColObj;
		local_20.aType = pParams->aType;
		local_20.pColObj = peVar5;
		IMPLEMENTATION_GUARD(
		for (; pUVar8 < pUVar9; pUVar8 = (edColPRIM_OBJECT*)((int)&(pUVar8->field_0x0).aa + iVar1)) {
			local_20.field_0x10 = &pUVar8->matrix_0x0.rowT;
			local_20.field_0x14 = &pUVar8->field_0xc0;
			local_20.field_0x18 = &pUVar8->field_0xd0;
			local_20.field_0x8 = (edF32MATRIX4*)pUVar8;
			for (peVar7 = peVar3; peVar7 < peVar3 + iVar2; peVar7 = peVar7 + 1) {
				local_20.field_0x1c = peVar7;
				edColIntersectBoxTriangle4(&eStack160, &local_20, 0);
				uVar6 = uVar6 | eStack160.result;
				peVar5->field_0x10 = uVar6;
			}
		})
	}
	else {
		if ((iVar4 == 0xb) || (iVar4 == 0xe)) {
			local_40.pOtherColObj = pParams->pOtherColObj;
			local_40.aType = pParams->aType;
			local_40.pColObj = peVar5;
			for (; pUVar8 < pUVar9; pUVar8 = (edColPRIM_OBJECT*)((char*)pUVar8 + iVar1)) {
				local_40.field_0x10 = &pUVar8->matrix_0x0.rowT;
				local_40.field_0x14 = &pUVar8->field_0xc0;
				local_40.field_0x18 = &pUVar8->field_0xd0;
				local_40.pData = pUVar8;
				for (peVar7 = peVar3; peVar7 < peVar3 + iVar2; peVar7 = peVar7 + 1) {
					local_40.pTriangle = (edF32TRIANGLE4*)peVar7;
					edColIntersectSphereTriangle4(&eStack160, &local_40, 0);
					uVar6 = uVar6 | eStack160.result;
					peVar5->colResult = uVar6;
				}
			}
		}
		else {
			uVar6 = 0;
		}
	}
	return uVar6;
}

uint edColArrayObjectPrimPenatratingArrayQuads4(edColARRAY_PRIM_QUAD4* pParams)
{
	int primSize;
	int nbQuads;
	edF32QUAD4* aQuads;
	edColOBJECT* pColObj;
	edColPRIM_OBJECT* pCurrentPrim;
	uint result;
	edF32QUAD4* pCurrentQuad;
	edColPRIM_OBJECT* pPrimEnd;
	//edColPRIM_BOX_QUAD4_IN local_a0;
	edColPRIM_SPHERE_QUAD4_IN primSphereQuadIn;
	edColINFO_OUT colInfoOut;

	result = 0;
	primSize = pParams->primSize;
	nbQuads = pParams->bCount;
	aQuads = (edF32QUAD4*)pParams->bData;
	pCurrentPrim = (edColPRIM_OBJECT*)pParams->aData;
	pColObj = pParams->pColObj;
	pPrimEnd = (edColPRIM_OBJECT*)((char*)pCurrentPrim + pParams->aCount * primSize);

	if (pParams->aType == 0xd) {
		IMPLEMENTATION_GUARD(
		local_a0.field_0x4 = pParams->field_0x14;
		local_a0.field_0xc = pParams->aType;
		local_a0.field_0x0 = pColObj;
		for (; pCurrentPrim < pPrimEnd; pCurrentPrim = (edColPRIM_OBJECT*)((int)&(pCurrentPrim->field_0x0).aa + primSize)) {
			local_a0._16_4_ = &(pCurrentPrim->field_0x0).da;
			local_a0._20_4_ = &pCurrentPrim->field_0xc0;
			local_a0._24_4_ = &pCurrentPrim->field_0xd0;
			local_a0.field_0x8 = (edF32MATRIX4*)pCurrentPrim;
			for (pCurrentQuad = aQuads; pCurrentQuad < aQuads + nbQuads; pCurrentQuad = pCurrentQuad + 1) {
				local_a0.field_0x1c = (edF32VECTOR4*)pCurrentQuad;
				edColIntersectBoxQuad4(&colInfoOut, &local_a0);
				result = result | colInfoOut.result;
				pColObj->field_0x10 = result;
			}
		})
	}
	else {
		if (pParams->aType == 0xe) {
			primSphereQuadIn.pOtherColObj = pParams->pOtherColObj;
			primSphereQuadIn.aType = pParams->aType;
			primSphereQuadIn.pColObj = pColObj;

			COLLISION_LOG(LogLevel::VeryVerbose, "edColArrayObjectPrimPenatratingArrayQuads4 sphere count: 0x{:x} quad count: 0x{:x}", pParams->aCount, nbQuads);

			for (; pCurrentPrim < pPrimEnd; pCurrentPrim = (edColPRIM_OBJECT*)((char*)pCurrentPrim + primSize)) {
				primSphereQuadIn.field_0x10 = &(pCurrentPrim->matrix_0x0).rowT;
				primSphereQuadIn.field_0x14 = &pCurrentPrim->field_0xc0;
				primSphereQuadIn.field_0x18 = &pCurrentPrim->field_0xd0;
				primSphereQuadIn.field_0x8 = pCurrentPrim;

				COLLISION_LOG(LogLevel::VeryVerbose, "edColArrayObjectPrimPenatratingArrayQuads4 {} {} {}", 
					primSphereQuadIn.field_0x10->ToString(), primSphereQuadIn.field_0x14->ToString(), primSphereQuadIn.field_0x18->ToString());

				for (pCurrentQuad = aQuads; pCurrentQuad < aQuads + nbQuads; pCurrentQuad = pCurrentQuad + 1) {
					primSphereQuadIn.pQuad = pCurrentQuad;
					edColIntersectSphereQuad4(&colInfoOut, &primSphereQuadIn);
					result = result | colInfoOut.result;

					COLLISION_LOG(LogLevel::VeryVerbose, "edColArrayObjectPrimPenatratingArrayQuads4 intersect result: 0x{:x} result: 0x{:x}", colInfoOut.result, result);
					pColObj->colResult = result;
				}
			}
		}
		else {
			result = 0;
		}
	}
	return result;
}

uint edObbTreeIntersectObbTree(edColINFO_OBBTREE_OBBTREE* pColInfoObbTree, edObbTREE_DYN* pObbTreeA, edObbTREE_DYN* pObbTreeB)
{
	byte bVar1;
	edObbTREE_DYN* peVar2;
	edObbTREE_DYN* peVar3;
	undefined4 uVar4;
	uint uVar5;
	bool lVar6;
	uint uVar7;
	int iVar8;
#if 0
	ProfileObject* pPVar9;
#endif
	uint uVar10;
	undefined4* puVar11;
	edColPrimEntry* puVar12;
	uint uVar13;
	float** ppfVar14;
	ushort* puVar15;
	edObbTREE_DYN** ppeVar16;
	int iVar17;
	int iVar18;
	ushort* puVar19;
	edObbTREE_DYN** ppeVar20;
	float** ppfVar21;
	int iVar22;
	float fVar23;
	int local_1054;
	int intersectCountB;
	int intersectCountA;
	undefined4 local_fec;
	undefined4 local_fe8;
	float* local_fe0;
	float* local_fdc;
	float* local_fd8;
	float* local_fd4;
	float* local_fd0;
	int local_fcc;
	int local_fc8;
	undefined4 local_fc4;
	edColARRAY_PRIM_QUAD4 local_f70;
	edColARRAY_PRIM_TRI4 local_e90;
	IMPLEMENTATION_GUARD_LOG(
	edColARRAY_PRIM_PRIM local_fc0;
	edColARRAY_VERT_IND_PRIM local_f90;
	edColARRAY_TRI4_PRIM local_f50;
	edColARRAY_PRIM_TRI4_FAST local_f30;
	edColARRAY_PRIM_TRI4_FAST local_f00;
	edColARRAY_PRIM_TRI4 local_ed0;
	edColARRAY_PRIM_TRI4 local_eb0;
	edColARRAY_VERT_IND_QUAD4 local_e70;)
	undefined local_e50[16];
	uint local_e40;
	edF32TRIANGLE4* local_e3c;
	IntersectResTestA aIntersectResultsA[100];
	edColPrimEntry aIntersectResultsB[100];
	edObbTREE_DYN* local_340[2][100];
	IMPLEMENTATION_GUARD_LOG(
	edColRAY_TRIANGLE4_IN local_20;)
	int local_10[3];
	float local_4;

	//uVar5 = prof_obb_col;
	uVar13 = 0;
	uVar10 = 0;
	local_340[0][0] = pObbTreeB;
	local_10[0] = 1;
	intersectCountA = 0;
	local_10[1] = 0;
	intersectCountB = 0;
#if 0
	if (gColConfig.bCreateProfileObj != 0) {
		if ((prof_obb_col & 1) == 0) {
			uVar7 = prof_obb_col >> 4;
			if (ProfileManager.ProfileManager[uVar7].field_0x34 != 0) {
				ProfileManager.ProfileManager[uVar7].field_0x8 = 0;
				ProfileManager.ProfileManager[uVar7].field_0x20 = 0;
				ProfileManager.ProfileManager[uVar7].field_0x34 = 0;
			}
			ProfileManager.ProfileManager[uVar5 >> 4].field_0x0 = (long)PCR1 & 0xffffffff;
			ProfileManager.ProfileManager[uVar5 >> 4].field_0x20 = ProfileManager.ProfileManager[uVar5 >> 4].field_0x20 + 1;
		}
		else {
			uVar7 = prof_obb_col >> 4;
			if (ProfileManager.g_ProfileObjA_0041ed40[uVar7].field_0x34 != 0) {
				ProfileManager.g_ProfileObjA_0041ed40[uVar7].field_0x8 = 0;
				ProfileManager.g_ProfileObjA_0041ed40[uVar7].field_0x20 = 0;
				ProfileManager.g_ProfileObjA_0041ed40[uVar7].field_0x34 = 0;
			}
			ProfileManager.g_ProfileObjA_0041ed40[uVar5 >> 4].field_0x0 = (long)PCR1 & 0xffffffff;
			ProfileManager.g_ProfileObjA_0041ed40[uVar5 >> 4].field_0x20 =
				ProfileManager.g_ProfileObjA_0041ed40[uVar5 >> 4].field_0x20 + 1;
		}
	}
#endif

	// Initial BB tests.
	COLLISION_LOG(LogLevel::VeryVerbose, "\nedObbTreeIntersectObbTree");

	iVar8 = 0;
	do {
		uVar5 = uVar10 != 0 ^ 1;
		ppeVar16 = local_340[0] + (iVar8 + uVar10) * 0x14;
		local_10[uVar5] = 0;
		ppeVar20 = local_340[uVar5];

		COLLISION_LOG(LogLevel::VeryVerbose, "\nedObbTreeIntersectObbTree flip: {} count: {}", uVar5, local_10[uVar10]);

		for (iVar8 = 0; iVar8 < local_10[uVar10]; iVar8 = iVar8 + 1) {
			peVar2 = *ppeVar16;
			iVar18 = 0;

			COLLISION_LOG(LogLevel::VeryVerbose, "edObbTreeIntersectObbTree type: {} (0x{:x})", peVar2->type, *(uint*)&peVar2->bbox.transform.aa);

			if (peVar2->type == 1) {

				COLLISION_LOG(LogLevel::VeryVerbose, "edObbTreeIntersectObbTree count: {}", peVar2->count_0x52);
				bVar1 = peVar2->count_0x52;
				for (; iVar18 < (int)(uint)bVar1; iVar18 = iVar18 + 1) {

					COLLISION_LOG(LogLevel::VeryVerbose, "edObbTreeIntersectObbTree checking: {}", iVar18);

					edObbTREE_DYN* pLinkedTree = (edObbTREE_DYN*)LOAD_SECTION(peVar2->field_0x54[iVar18]);
					lVar6 = edObbIntersect(&pObbTreeA->bbox, &pLinkedTree->bbox);

					COLLISION_LOG(LogLevel::VeryVerbose, "edObbTreeIntersectObbTree edObbIntersect result: {}", lVar6);

					if (lVar6 != 0) {
						uVar5 = uVar10 != 0 ^ 1;
						if (local_10[uVar5] < 100) {
							*ppeVar20 = pLinkedTree;
							ppeVar20 = ppeVar20 + 1;
							local_10[uVar5] = local_10[uVar5] + 1;

							COLLISION_LOG(LogLevel::VeryVerbose, "edObbTreeIntersectObbTree flip index: {} new count: {}", uVar5, local_10[uVar5]);
						}
					}
				}
			}
			else {
				lVar6 = edObbIntersect(&pObbTreeA->bbox, &peVar2->bbox);

				COLLISION_LOG(LogLevel::VeryVerbose, "edObbTreeIntersectObbTree edObbIntersect result: {}", lVar6);

				if ((lVar6 != 0) && (intersectCountA < 100)) {
					aIntersectResultsA[intersectCountA].pObbTree = peVar2;
					aIntersectResultsA[intersectCountA].sub.type = peVar2->type;
					aIntersectResultsA[intersectCountA].sub.count = peVar2->count_0x52;
					aIntersectResultsA[intersectCountA].sub.pData = LOAD_SECTION(peVar2->field_0x54[0]);
					intersectCountA = intersectCountA + 1;

					COLLISION_LOG(LogLevel::VeryVerbose, "edObbTreeIntersectObbTree new intersect count: {} (type: 0x{:x} - count: 0x{:x})", intersectCountA, peVar2->type, peVar2->count_0x52);
				}
			}
			ppeVar16 = ppeVar16 + 1;
		}

		uVar10 = uVar10 != 0 ^ 1;
		iVar8 = uVar10 << 2;
	} while (local_10[uVar10] != 0);

	COLLISION_LOG(LogLevel::VeryVerbose, "\nedObbTreeIntersectObbTree Process A Results");

	// Go through first set of results.
	for (iVar8 = 0; iVar8 < intersectCountA; iVar8 = iVar8 + 1) {
		local_340[uVar10][0] = pObbTreeA;
		local_10[uVar10] = 1;
		local_10[uVar10 != 0 ^ 1] = 0;
		peVar2 = aIntersectResultsA[iVar8].pObbTree;
		do {
			uVar5 = uVar10 != 0 ^ 1;
			ppeVar16 = local_340[uVar10];
			local_10[uVar5] = 0;
			ppeVar20 = local_340[uVar5];
			for (iVar18 = 0; iVar18 < local_10[uVar10]; iVar18 = iVar18 + 1) {
				peVar3 = *ppeVar16;
				iVar17 = 0;
				if (peVar3->type == 1) {
					IMPLEMENTATION_GUARD(
					bVar1 = *(byte*)((int)&peVar3->field_0x4c + 6);
					for (; iVar17 < (int)(uint)bVar1; iVar17 = iVar17 + 1) {
						lVar6 = edObbIntersect((edObbBOX*)peVar2, (edObbBOX*)((float*)&peVar3->field_0x4c)[iVar17 + 2]);
						if (lVar6 != 0) {
							uVar5 = uVar10 != 0 ^ 1;
							if (local_10[uVar5] < 100) {
								*ppeVar20 = (edObbTREE_DYN*)((float*)&peVar3->field_0x4c)[iVar17 + 2];
								ppeVar20 = ppeVar20 + 1;
								local_10[uVar5] = local_10[uVar5] + 1;
							}
						}
						bVar1 = *(byte*)((int)&peVar3->field_0x4c + 6);
					})
				}
				else {
					lVar6 = edObbIntersect(&peVar2->bbox, &peVar3->bbox);
					if ((lVar6 != 0) && (intersectCountB < 100)) {
						aIntersectResultsB[intersectCountB].a.type = peVar3->type;
						aIntersectResultsB[intersectCountB].a.count = peVar3->count_0x52;
						aIntersectResultsB[intersectCountB].a.pData = LOAD_SECTION(peVar3->field_0x54[0]);

						aIntersectResultsB[intersectCountB].b = aIntersectResultsA[iVar8].sub;

						intersectCountB = intersectCountB + 1;

						COLLISION_LOG(LogLevel::VeryVerbose, "edObbTreeIntersectObbTree new intersect B count: {} (type: 0x{:x} - count: 0x{:x})", intersectCountB, peVar3->type, peVar3->count_0x52);

					}
				}
				ppeVar16 = ppeVar16 + 1;
			}
			uVar10 = uVar10 != 0 ^ 1;
		} while (local_10[uVar10] != 0);
	}

#if 0
	if (gColConfig.bCreateProfileObj != 0) {
		uVar5 = prof_obb_col >> 4;
		if ((prof_obb_col & 1) == 0) {
			ProfileManager.ProfileManager[uVar5].field_0x0 = (long)PCR1 - ProfileManager.ProfileManager[uVar5].field_0x0;
			ProfileManager.ProfileManager[uVar5].field_0x8 =
				ProfileManager.ProfileManager[uVar5].field_0x8 + ProfileManager.ProfileManager[uVar5].field_0x0;
			pPVar9 = ProfileManager.ProfileManager + uVar5;
			if (pPVar9->field_0x10 < ProfileManager.ProfileManager[uVar5].field_0x0) {
				pPVar9->field_0x10 = pPVar9->field_0x0;
			}
		}
		else {
			ProfileManager.g_ProfileObjA_0041ed40[uVar5].field_0x0 =
				(long)PCR1 - ProfileManager.g_ProfileObjA_0041ed40[uVar5].field_0x0;
			ProfileManager.g_ProfileObjA_0041ed40[uVar5].field_0x8 =
				ProfileManager.g_ProfileObjA_0041ed40[uVar5].field_0x8 +
				ProfileManager.g_ProfileObjA_0041ed40[uVar5].field_0x0;
			pPVar9 = ProfileManager.g_ProfileObjA_0041ed40 + uVar5;
			if (pPVar9->field_0x10 < ProfileManager.g_ProfileObjA_0041ed40[uVar5].field_0x0) {
				pPVar9->field_0x10 = pPVar9->field_0x0;
			}
		}
	}
	uVar5 = prof_prim_col;
	if (gColConfig.bCreateProfileObj != 0) {
		if ((prof_prim_col & 1) == 0) {
			uVar10 = prof_prim_col >> 4;
			if (ProfileManager.ProfileManager[uVar10].field_0x34 != 0) {
				ProfileManager.ProfileManager[uVar10].field_0x8 = 0;
				ProfileManager.ProfileManager[uVar10].field_0x20 = 0;
				ProfileManager.ProfileManager[uVar10].field_0x34 = 0;
			}
			ProfileManager.ProfileManager[uVar5 >> 4].field_0x0 = (long)PCR1 & 0xffffffff;
			ProfileManager.ProfileManager[uVar5 >> 4].field_0x20 = ProfileManager.ProfileManager[uVar5 >> 4].field_0x20 + 1;
		}
		else {
			uVar10 = prof_prim_col >> 4;
			if (ProfileManager.g_ProfileObjA_0041ed40[uVar10].field_0x34 != 0) {
				ProfileManager.g_ProfileObjA_0041ed40[uVar10].field_0x8 = 0;
				ProfileManager.g_ProfileObjA_0041ed40[uVar10].field_0x20 = 0;
				ProfileManager.g_ProfileObjA_0041ed40[uVar10].field_0x34 = 0;
			}
			ProfileManager.g_ProfileObjA_0041ed40[uVar5 >> 4].field_0x0 = (long)PCR1 & 0xffffffff;
			ProfileManager.g_ProfileObjA_0041ed40[uVar5 >> 4].field_0x20 =
				ProfileManager.g_ProfileObjA_0041ed40[uVar5 >> 4].field_0x20 + 1;
		}
	}
#endif

	// Go through results.
	puVar12 = aIntersectResultsB;
	for (iVar8 = 0; iVar8 < intersectCountB; iVar8 = iVar8 + 1) {
		uVar5 = (uint)(puVar12->a).type << 8 | (uint)(puVar12->b).type;
		if (uVar5 == 0x1008) {
			IMPLEMENTATION_GUARD(
			uVar5 = puVar12->a->pData;
			uVar10 = uVar5 + (uint)puVar12[1] * 0x60;
			for (; uVar5 < uVar10; uVar5 = uVar5 + 0x60) {
				fVar23 = *(float*)(uVar5 + 0x44);
				ppfVar14 = *(float***)(puVar12 + 6);
				iVar18 = -1;
				iVar17 = 0;
				ppfVar21 = ppfVar14 + (uint)puVar12[5] * 5;
				for (; ppfVar14 < ppfVar21; ppfVar14 = ppfVar14 + 5) {
					local_fe0 = *ppfVar14;
					for (iVar22 = 0; iVar22 < 2; iVar22 = iVar22 + 1) {
						if (iVar22 == 0) {
							local_fdc = ppfVar14[1];
							local_fd8 = ppfVar14[2];
						}
						else {
							local_fdc = ppfVar14[2];
							local_fd8 = ppfVar14[3];
						}
						local_fd4 = ppfVar14[4];
						local_20.field_0x0 = (float*)(uVar5 + 0x10);
						local_20.field_0x4 = (float*)(uVar5 + 0x30);
						local_20.field_0x8 = &local_fe0;
						edColIntersectRayTriangle4(&local_4, &local_20);
						if ((((local_4 != -8888.0) && (local_4 != -9999.0)) && (0.0 <= local_4)) &&
							((local_4 < fVar23 || ((fVar23 < 0.0 && (local_4 <= *(float*)(uVar5 + 0x40))))))) {
							fVar23 = local_4;
							iVar18 = iVar17;
							local_1054 = iVar22;
						}
					}
					iVar17 = iVar17 + 1;
				}
				*(float*)(uVar5 + 0x44) = fVar23;
				if (((0.0 <= fVar23) && (-1 < iVar18)) && (*(int*)(puVar12 + 6) != 0)) {
					puVar11 = (undefined4*)(*(int*)(puVar12 + 6) + iVar18 * 0x14);
					if (local_1054 == 0) {
						local_fec = puVar11[1];
						local_fe8 = puVar11[2];
					}
					else {
						local_fec = puVar11[2];
						local_fe8 = puVar11[3];
					}
					uVar4 = puVar11[4];
					*(undefined4*)(uVar5 + 0x50) = *puVar11;
					*(undefined4*)(uVar5 + 0x54) = local_fec;
					*(undefined4*)(uVar5 + 0x58) = local_fe8;
					*(undefined4*)(uVar5 + 0x5c) = uVar4;
				}
			})
		}
		else {
			if (uVar5 == 0x1005) {
				IMPLEMENTATION_GUARD(
				uVar5 = *(uint*)(puVar12 + 2);
				uVar10 = uVar5 + (uint)puVar12[1] * 0x60;
				for (; uVar5 < uVar10; uVar5 = uVar5 + 0x60) {
					fVar23 = *(float*)(uVar5 + 0x44);
					puVar15 = *(ushort**)(puVar12 + 6);
					puVar19 = puVar15 + (uint)puVar12[5] * 0xc;
					for (; puVar15 < puVar19; puVar15 = puVar15 + 0xc) {
						local_20.field_0x0 = (float*)(uVar5 + 0x10);
						local_20.field_0x4 = (float*)(uVar5 + 0x30);
						local_20.field_0x8 = &local_fd0;
						local_fd0 = (float*)(*(int*)(pColInfoObbTree->field_0x8 + 0x28) + (uint)*puVar15 * 0x10);
						local_fcc = *(int*)(pColInfoObbTree->field_0x8 + 0x28) + (uint)puVar15[1] * 0x10;
						local_fc8 = *(int*)(pColInfoObbTree->field_0x8 + 0x28) + (uint)puVar15[2] * 0x10;
						local_fc4 = *(undefined4*)(puVar15 + 8);
						edColIntersectRayTriangle4(&local_4, &local_20);
						if ((((local_4 != -8888.0) && (local_4 != -9999.0)) && (0.0 <= local_4)) &&
							((local_4 < fVar23 || ((fVar23 < 0.0 && (local_4 <= *(float*)(uVar5 + 0x40))))))) {
							*(float**)(uVar5 + 0x50) = local_fd0;
							*(int*)(uVar5 + 0x54) = local_fcc;
							*(int*)(uVar5 + 0x58) = local_fc8;
							*(undefined4*)(uVar5 + 0x5c) = local_fc4;
							fVar23 = local_4;
						}
					}
					*(float*)(uVar5 + 0x44) = fVar23;
				})
			}
			else {
				if (uVar5 == 0x1004) {
					IMPLEMENTATION_GUARD(
					uVar5 = *(uint*)(puVar12 + 2);
					uVar10 = uVar5 + (uint)puVar12[1] * 0x60;
					for (; uVar5 < uVar10; uVar5 = uVar5 + 0x60) {
						fVar23 = *(float*)(uVar5 + 0x44);
						ppfVar14 = *(float***)(puVar12 + 6);
						iVar18 = -1;
						iVar17 = 0;
						ppfVar21 = ppfVar14 + (uint)puVar12[5] * 4;
						for (; ppfVar14 < ppfVar21; ppfVar14 = ppfVar14 + 4) {
							local_20.field_0x4 = (float*)(uVar5 + 0x30);
							local_20.field_0x0 = (float*)(uVar5 + 0x10);
							local_20.field_0x8 = ppfVar14;
							edColIntersectRayTriangle4(&local_4, &local_20);
							if (((local_4 != -8888.0) && (local_4 != -9999.0)) &&
								((0.0 <= local_4 && ((local_4 < fVar23 || ((fVar23 < 0.0 && (local_4 <= *(float*)(uVar5 + 0x40))))))))
								) {
								fVar23 = local_4;
								iVar18 = iVar17;
							}
							iVar17 = iVar17 + 1;
						}
						*(float*)(uVar5 + 0x44) = fVar23;
						if ((0.0 <= fVar23) && (-1 < iVar18)) {
							if (*(int*)(puVar12 + 6) != 0) {
								puVar11 = (undefined4*)(*(int*)(puVar12 + 6) + iVar18 * 0x10);
								*(undefined4*)(uVar5 + 0x50) = *puVar11;
								*(undefined4*)(uVar5 + 0x54) = puVar11[1];
								*(undefined4*)(uVar5 + 0x58) = puVar11[2];
								*(undefined4*)(uVar5 + 0x5c) = puVar11[3];
							}
						}
					})
				}
				else {
					if ((((((uVar5 == 0x130a) || (uVar5 == 0x130d)) || (uVar5 == 0x1313)) ||
						((uVar5 == 0x130b || (uVar5 == 0x130e)))) || (uVar5 == 0xd0a)) ||
						(((((uVar5 == 0xd0d || (uVar5 == 0xd13)) ||
							((uVar5 == 0xd0b || (((uVar5 == 0xd0e || (uVar5 == 0xe0a)) || (uVar5 == 0xe0d)))))) ||
							((uVar5 == 0xe13 || (uVar5 == 0xe0b)))) || (uVar5 == 0xe0e)))) {
						IMPLEMENTATION_GUARD(
						local_fc0.field_0x0 = (int)pColInfoObbTree->field_0x0;
						local_fc0.field_0x10 = *(edF32MATRIX4**)(puVar12 + 2);
						local_fc0.field_0x4 = ZEXT24(puVar12[1]);
						local_fc0.field_0xc = *(int*)(&_gColSizeOfPrims + (uint)*puVar12 * 4);
						local_fc0.field_0x8 = ZEXT24(*puVar12);
						local_fc0.field_0x14 = (int)pColInfoObbTree->pOtherColObj;
						local_fc0.field_0x24 = *(edF32MATRIX4**)(puVar12 + 6);
						local_fc0.field_0x18 = ZEXT24(puVar12[5]);
						local_fc0.field_0x20 = *(int*)(&_gColSizeOfPrims + (uint)puVar12[4] * 4);
						local_fc0.field_0x1c = ZEXT24(puVar12[4]);
						uVar5 = edColArrayObjectPrimsPenatratingArrayPrims(&local_fc0);
						uVar13 = uVar13 | uVar5;)
					}
					else {
						if (((uVar5 == 0x30d) || (uVar5 == 0x30e)) || ((uVar5 == 0x30a || (uVar5 == 0x30b)))) {
							IMPLEMENTATION_GUARD(
							local_f90.field_0x0 = pColInfoObbTree->field_0x0;
							local_f90.field_0x4 = (char*)(uint)puVar12[1];
							local_f90.field_0x8 = *(char**)(puVar12 + 2);
							local_f90.field_0xc = pColInfoObbTree->pOtherColObj;
							local_f90.field_0x1c = *(char**)(puVar12 + 6);
							local_f90.field_0x10 = (char*)(uint)puVar12[5];
							local_f90.field_0x18 = *(char**)(&_gColSizeOfPrims + (uint)puVar12[4] * 4);
							local_f90.field_0x14 = (char*)(uint)puVar12[4];
							uVar5 = edColArrayVerticesIndexPenatratingArrayPrims(&local_f90, (int)pColInfoObbTree->field_0xc);
							uVar13 = uVar13 | uVar5;)
						}
						else {
							if ((uVar5 == 0xd08) || (uVar5 == 0xe08)) {
								COLLISION_LOG(LogLevel::VeryVerbose, "edObbTreeIntersectObbTree edColArrayObjectPrimPenatratingArrayQuads4 0x{:x}", uVar5);
								local_f70.pColObj = pColInfoObbTree->pColObj;
								local_f70.aData = puVar12->a.pData;
								local_f70.aCount = puVar12->a.count;
								local_f70.primSize = _gColSizeOfPrims[(puVar12->a).type];
								local_f70.aType = puVar12->a.type;
								local_f70.pOtherColObj = pColInfoObbTree->pOtherColObj;
								local_f70.bData = puVar12->b.pData;
								local_f70.bCount = puVar12->b.count;

								COLLISION_LOG(LogLevel::VeryVerbose, "edObbTreeIntersectObbTree aCount: 0x{:x} aType: 0x{:x} bCount: 0x{:x} primSize: 0x{:x}", 
									local_f70.aCount, local_f70.aType, local_f70.bCount, local_f70.primSize);

								uVar5 = edColArrayObjectPrimPenatratingArrayQuads4(&local_f70);
								COLLISION_LOG(LogLevel::VeryVerbose, "edObbTreeIntersectObbTree edColArrayObjectPrimPenatratingArrayQuads4 result: 0x{:x}", uVar5);
								uVar13 = uVar13 | uVar5;
							}
							else {
								if ((((uVar5 == 0x40d) || (uVar5 == 0x40e)) || (uVar5 == 0x40a)) || (uVar5 == 0x40b)) {
									IMPLEMENTATION_GUARD(
									local_f50.field_0x0 = pColInfoObbTree->pOtherColObj;
									local_f50.field_0x10 = *(uint*)(puVar12 + 6);
									local_f50.field_0x4 = (uint)puVar12[5];
									local_f50.field_0xc = *(int*)(&_gColSizeOfPrims + (uint)puVar12[4] * 4);
									local_f50.field_0x8 = ZEXT24(puVar12[4]);
									local_f50.field_0x14 = pColInfoObbTree->field_0x0;
									local_f50.field_0x1c = *(uint*)(puVar12 + 2);
									local_f50.field_0x18 = ZEXT24(puVar12[5]);
									uVar5 = edColArrayObjectTriangles4PenatratingPrims(&local_f50);
									uVar13 = uVar13 | uVar5;)
								}
								else {
									if (uVar5 == 0xe05) {
										IMPLEMENTATION_GUARD(
										local_f30.field_0x0 = (int)pColInfoObbTree->field_0x0;
										local_f30.field_0x10 = *(uint*)(puVar12 + 2);
										local_f30.field_0x4 = ZEXT24(puVar12[1]);
										local_f30.field_0xc = *(int*)(&_gColSizeOfPrims + (uint)*puVar12 * 4);
										local_f30.field_0x8 = ZEXT24(*puVar12);
										local_f30.field_0x18 = (int)pColInfoObbTree->pOtherColObj;
										local_f30.field_0x20 = *(uint*)(puVar12 + 6);
										local_f30.field_0x1c = ZEXT24(puVar12[5]);
										local_f30.field_0x14 = (int)pColInfoObbTree->field_0x8;
										uVar5 = edColArrayObjectFastPrimPenatratingArrayTriangles4Fast(&local_f30);
										uVar13 = uVar13 | uVar5;)
									}
									else {
										if (uVar5 == 0x1305) {
											IMPLEMENTATION_GUARD(
											local_f00.field_0x0 = (int)pColInfoObbTree->field_0x0;
											local_f00.field_0x10 = *(uint*)(puVar12 + 2);
											local_f00.field_0x4 = ZEXT24(puVar12[1]);
											local_f00.field_0xc = *(int*)(&_gColSizeOfPrims + (uint)*puVar12 * 4);
											local_f00.field_0x8 = ZEXT24(*puVar12);
											local_f00.field_0x18 = (int)pColInfoObbTree->pOtherColObj;
											local_f00.field_0x20 = *(uint*)(puVar12 + 6);
											local_f00.field_0x1c = ZEXT24(puVar12[5]);
											local_f00.field_0x14 = (int)pColInfoObbTree->field_0x8;
											uVar5 = edColArrayObjectFastPrimPenatratingArrayTriangles4(&local_f00);
											uVar13 = uVar13 | uVar5;)
										}
										else {
											if (uVar5 == 0x1308) {
												IMPLEMENTATION_GUARD(
												local_ed0.field_0x0 = (int)pColInfoObbTree->field_0x0;
												local_ed0.field_0x10 = *(edF32MATRIX4**)(puVar12 + 2);
												local_ed0.field_0x4 = ZEXT24(puVar12[1]);
												local_ed0.field_0xc = *(int*)(&_gColSizeOfPrims + (uint)*puVar12 * 4);
												local_ed0.field_0x8 = ZEXT24(*puVar12);
												local_ed0.field_0x14 = (int)pColInfoObbTree->pOtherColObj;
												local_ed0.field_0x1c = *(edF32VECTOR4**)(puVar12 + 6);
												local_ed0.field_0x18 = ZEXT24(puVar12[5]);
												uVar5 = edColArrayObjectFastPrimPenatratingArrayTriangles4(&local_ed0);
												uVar13 = uVar13 | uVar5;)
											}
											else {
												if (uVar5 == 0x1304) {
													IMPLEMENTATION_GUARD(
													local_eb0.field_0x0 = (int)pColInfoObbTree->field_0x0;
													local_eb0.field_0x10 = *(edF32MATRIX4**)(puVar12 + 2);
													local_eb0.field_0x4 = ZEXT24(puVar12[1]);
													local_eb0.field_0xc = *(int*)(&_gColSizeOfPrims + (uint)*puVar12 * 4);
													local_eb0.field_0x8 = ZEXT24(*puVar12);
													local_eb0.field_0x14 = (int)pColInfoObbTree->pOtherColObj;
													local_eb0.field_0x1c = *(edF32VECTOR4**)(puVar12 + 6);
													local_eb0.field_0x18 = ZEXT24(puVar12[5]);
													uVar5 = edColArrayObjectFastPrimPenatratingArrayTriangles4(&local_eb0);
													uVar13 = uVar13 | uVar5;)
												}
												else {
													if ((uVar5 == 0xd04) || (uVar5 == 0xe04)) {
														local_e90.pColObj = pColInfoObbTree->pColObj;
														local_e90.aData = puVar12->a.pData;
														local_e90.aCount = puVar12->a.count;
														local_e90.primSize = _gColSizeOfPrims[(puVar12->a).type];
														local_e90.aType = puVar12->a.type;
														local_e90.pOtherColObj = pColInfoObbTree->pOtherColObj;
														local_e90.bData = puVar12->b.pData;
														local_e90.bCount = puVar12->b.count;
														uVar5 = edColArrayObjectPrimPenatratingArrayTriangles4(&local_e90);
														uVar13 = uVar13 | uVar5;
													}
													else {
														if (uVar5 == 0x308) {
															IMPLEMENTATION_GUARD(
															local_e70.field_0x0 = (int)pColInfoObbTree->field_0x0;
															local_e70.field_0xc = (int)pColInfoObbTree->pOtherColObj;
															local_e70.field_0x8 = *(int**)(puVar12 + 2);
															local_e70.field_0x14 = *(int*)(puVar12 + 6);
															local_e70.field_0x4 = ZEXT24(puVar12[1]);
															local_e70.field_0x10 = ZEXT24(puVar12[5]);
															uVar5 = edColArrayVerticesIndexPenatratingArrayQuads4
															(&local_e70, (int)pColInfoObbTree->field_0xc);
															uVar13 = uVar13 | uVar5;)
														}
														else {
															if (uVar5 == 0x304) {
																IMPLEMENTATION_GUARD(
																local_e50._0_4_ = pColInfoObbTree->field_0x0;
																local_e50._8_4_ = *(int**)(puVar12 + 2);
																local_e3c = *(edF32TRIANGLE4**)(puVar12 + 6);
																local_e50._12_4_ = pColInfoObbTree->pOtherColObj;
																local_e50._4_4_ = (uint)puVar12[1];
																local_e40 = (uint)puVar12[5];
																uVar5 = edColArrayVerticesIndexPenatratingArrayTriangles4
																((edColARRAY_VERT_IND_TRI4*)local_e50, (int)pColInfoObbTree->field_0xc
																);
																uVar13 = uVar13 | uVar5;)
															}
														}
													}
												}
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
		puVar12 = puVar12 + 1;
	}

#if 0
	if (gColConfig.bCreateProfileObj != 0) {
		uVar5 = prof_prim_col >> 4;
		if ((prof_prim_col & 1) == 0) {
			ProfileManager.ProfileManager[uVar5].field_0x0 = (long)PCR1 - ProfileManager.ProfileManager[uVar5].field_0x0;
			ProfileManager.ProfileManager[uVar5].field_0x8 =
				ProfileManager.ProfileManager[uVar5].field_0x8 + ProfileManager.ProfileManager[uVar5].field_0x0;
			pPVar9 = ProfileManager.ProfileManager + uVar5;
			if (pPVar9->field_0x10 < ProfileManager.ProfileManager[uVar5].field_0x0) {
				pPVar9->field_0x10 = pPVar9->field_0x0;
			}
		}
		else {
			ProfileManager.g_ProfileObjA_0041ed40[uVar5].field_0x0 =
				(long)PCR1 - ProfileManager.g_ProfileObjA_0041ed40[uVar5].field_0x0;
			ProfileManager.g_ProfileObjA_0041ed40[uVar5].field_0x8 =
				ProfileManager.g_ProfileObjA_0041ed40[uVar5].field_0x8 +
				ProfileManager.g_ProfileObjA_0041ed40[uVar5].field_0x0;
			pPVar9 = ProfileManager.g_ProfileObjA_0041ed40 + uVar5;
			if (pPVar9->field_0x10 < ProfileManager.g_ProfileObjA_0041ed40[uVar5].field_0x0) {
				pPVar9->field_0x10 = pPVar9->field_0x0;
			}
		}
	}
#endif
	return uVar13;
}

edColConfig* edColGetConfig(void)
{
	return &gColConfig;
}

void edColInit(void)
{
	edColOBJECT* peVar1;
	edColDbObj_80* peVar2;
	undefined* puVar3;
	byte* pbVar4;
	uint uVar5;
	void* pvVar6;
	int iVar7;
	edColDatabase* peVar8;
	int iVar9;

	if (gColConfig.bSetMemFlags != 0) {
		edMemSetFlags(gColConfig.heapID_A, 0x100);
	}

	gColTD.field_0x4 = (undefined*)edMemAlloc(gColConfig.heapID_A, gColConfig.field_0x18 * 0xc0);
	memset(gColTD.field_0x4, 0, gColConfig.field_0x18 * 0xc0);

	gColTD.aPrim = (edColPrimEntry*)edMemAlloc(gColConfig.heapID_A, gColConfig.nbPrimEntries * sizeof(edColPrimEntry));
	memset(gColTD.aPrim, 0, gColConfig.nbPrimEntries * sizeof(edColPrimEntry));

	gColTD.field_0x14 = (undefined*)edMemAlloc(gColConfig.heapID_A, gColConfig.field_0x1c << 5);
	memset(gColTD.field_0x14, 0, gColConfig.field_0x1c << 5);

	if (gColConfig.bSetMemFlags != 0) {
		edMemClearFlags(gColConfig.heapID_A, 0x100);
	}

	gColData.aDatabases = (edColDatabase*)edMemAlloc(gColConfig.heapID_B, gColConfig.databaseCount * sizeof(edColDatabase));
	memset(gColData.aDatabases, 0, gColConfig.databaseCount * sizeof(edColDatabase));

	gColData.pActiveDatabase = gColData.aDatabases;
	for (iVar9 = 0; iVar9 < (int)(uint)gColConfig.databaseCount; iVar9 = iVar9 + 1) {
		peVar8 = gColData.aDatabases + iVar9;

		peVar8->aColObj = (edColOBJECT*)edMemAlloc(gColConfig.heapID_B, gColConfig.field_0x8[iVar9] * sizeof(edColOBJECT));
		memset(peVar8->aColObj, 0, gColConfig.field_0x8[iVar9] * sizeof(edColOBJECT));

		peVar8->field_0x10 = (edColDbObj_80*)edMemAlloc(gColConfig.heapID_B, gColConfig.field_0x8[iVar9 + 2] * sizeof(edColDbObj_80));
		memset(peVar8->field_0x10, 0, gColConfig.field_0x8[iVar9 + 2] * sizeof(edColDbObj_80));

		peVar8->aPrim = (edColPrimEntry*)edMemAlloc(gColConfig.heapID_B, gColConfig.field_0x8[iVar9 + 4] * sizeof(edColPrimEntry));
		memset(peVar8->aPrim, 0, gColConfig.field_0x8[iVar9 + 4] * sizeof(edColPrimEntry));

		peVar8->field_0x1c = (undefined*)edMemAlloc(gColConfig.heapID_B, gColConfig.field_0x8[iVar9 + 6] * 0xc0);
		memset(peVar8->field_0x1c, 0, gColConfig.field_0x8[iVar9 + 6] * 0xc0);

		if (gColConfig.field_0x1 == 0) {
			peVar8->field_0x20 = (byte*)edMemAlloc(gColConfig.heapID_B, gColConfig.field_0x8[iVar9] * gColConfig.field_0x8[iVar9]);
			uVar5 = gColConfig.field_0x8[iVar9];
			memset(peVar8->field_0x20, 0, uVar5 * uVar5);

			for (iVar7 = 0; uVar5 = gColConfig.field_0x8[iVar9], iVar7 < (int)(uVar5 * uVar5);
				iVar7 = iVar7 + 1) {
				peVar8->field_0x20[iVar7] = 1;
			}

			peVar8->curObjId = 0;
		}
		else {
			peVar8->field_0x20 = (byte*)0x0;
			peVar8->curObjId = 0;
		}

		peVar8->field_0x4 = 0;
	}

	edMemSetFlags(TO_HEAP(H_MAIN), 0x8000);
	gColData.field_0x1c = (uint)gColConfig.field_0x3 << 4;
	pvVar6 = edMemAlloc(TO_HEAP(H_MAIN), gColData.field_0x1c);
	gColData.field_0x18 = (undefined*)((uint)pvVar6 | 0x20000000);
	edMemClearFlags(TO_HEAP(H_MAIN), 0x8000);

	gColTD.field_0x0 = 0;
	gColData.bInitialized = 1;
	gColTD.pCurDatabase = gColData.aDatabases;

#if 0
	if (gColConfig.bCreateProfileObj != 0) {
		/* OBB_TEST */
		prof_obb_col = edProfileNew(1, 0x80, 0x80, 0x80, s_OBB_TEST_00430060);
		/* PRIM_TEST */
		prof_prim_col = edProfileNew(1, 0x80, 0x40, 0x40, s_PRIM_TEST_00430070);
		/* FAST_TEST */
		prof_fast_col = edProfileNew(1, 0x40, 0x40, 0x80, s_FAST_TEST_00430080);
	}
#endif
	return;
}

void edColSetDataBase(int databaseId)
{
	gColData.activeDatabaseId = (byte)databaseId;
	gColData.pActiveDatabase = gColData.aDatabases + databaseId;
	return;
}

void edColBegin(int param_1, int databaseId)
{
	gColData.pActiveDatabase = gColData.aDatabases + databaseId;
	gColData.activeDatabaseId = (byte)databaseId;
	gColTD.field_0x0 = 1;
	gColTD.field_0x1 = (byte)param_1;
	gColTD.field_0x8 = gColTD.field_0x4;
	gColTD.pCurPrim = gColTD.aPrim;
	gColTD.field_0x18 = gColTD.field_0x14;
	gColTD.pCurColObj = (gColData.pActiveDatabase)->aColObj + (gColData.pActiveDatabase)->curObjId;
	gColTD.field_0x38 = 0;
	gColTD.curObjId = (gColData.pActiveDatabase)->curObjId;
	gColTD.nbPrim = 0;
	gColTD.field_0x44 = 0;
	return;
}

void edColObjectSetCollisionsType(byte param_1, byte param_2, byte param_3)
{
	(gColTD.pCurColObj)->colType_0x0 = param_1;
	(gColTD.pCurColObj)->colType_0x1 = param_2;
	(gColTD.pCurColObj)->colType_0x2 = param_3;
	return;
}

void edColCollisionAddPrim(int aType, int aCount, void* param_3, void* aData, int bType, int bCount, void* bData)
{
	((gColTD.pCurPrim)->a).pData = aData;
	((gColTD.pCurPrim)->a).type = (ushort)aType;
	((gColTD.pCurPrim)->a).count = (short)aCount;
	((gColTD.pCurPrim)->b).pData = bData;
	((gColTD.pCurPrim)->b).type = (ushort)bType;
	((gColTD.pCurPrim)->b).count = (short)bCount;
	gColTD.pCurPrim = gColTD.pCurPrim + 1;
	gColTD.nbPrim = gColTD.nbPrim + 1;
	return;
}

edColOBJECT* edColEnd(edDynOBJECT* pDynObj)
{
	undefined8 uVar1;
	int iVar2;
	ushort* puVar4;
	undefined4 uVar5;
	undefined4 uVar6;
	undefined4 uVar7;
	undefined4 uVar8;
	int iVar9;
	edColPrimEntry* pIVar10;
	undefined8* puVar11;
	undefined4 uVar12;
	undefined4 uVar13;
	undefined4 uVar14;
	int iVar15;
	undefined8* puVar16;
	undefined4* puVar17;
	undefined4* puVar18;

#if 0
	undefined local_70[112];
	undefined* puVar3;

	puVar3 = local_70;
	iVar2 = 0x70;
	if ((undefined*)register0x000001d0 != (undefined*)0x70) {
		do {
			*puVar3 = 0;
			iVar2 = iVar2 + -1;
			puVar3 = puVar3 + 1;
		} while (iVar2 != 0);
	}
#endif

	(gColTD.pCurColObj)->pDynObj = STORE_SECTION(pDynObj);

	if (gColTD.field_0x38 != 0) {
		IMPLEMENTATION_GUARD(
		(gColTD.pCurColObj)->nbPrim = (short)gColTD.field_0x38;
		(gColTD.pCurColObj)->field_0x18 =
			(gColData.pActiveDatabase)->field_0x1c + (gColData.pActiveDatabase)->field_0xc * 0xc0;
		(gColData.pActiveDatabase)->field_0xc = (gColData.pActiveDatabase)->field_0xc + gColTD.field_0x38;
		if (gColTD.field_0x1 == 2) {
			(gColTD.pCurColObj)->field_0x1c =
				(gColData.pActiveDatabase)->field_0x1c + (gColData.pActiveDatabase)->field_0xc * 0xc0;
			(gColData.pActiveDatabase)->field_0xc = (gColData.pActiveDatabase)->field_0xc + gColTD.field_0x38;
		})
	}

	if (gColTD.nbPrim != 0) {
		(gColTD.pCurColObj)->nbPrimUsed = (short)gColTD.nbPrim;

		(gColTD.pCurColObj)->pPrim = STORE_SECTION((gColData.pActiveDatabase)->aPrim + (gColData.pActiveDatabase)->curPrimId);

		(gColData.pActiveDatabase)->curPrimId = (gColData.pActiveDatabase)->curPrimId + gColTD.nbPrim;
	}

	(gColTD.pCurColObj)->field_0x4 = 1;
	(gColTD.pCurColObj)->field_0x5 = 1;

	for (iVar2 = 0; iVar2 < gColTD.field_0x38; iVar2 = iVar2 + 1) {
		IMPLEMENTATION_GUARD(
		iVar15 = iVar2 * 0xc0;
		iVar9 = 3;
		puVar16 = (undefined8*)(gColTD.field_0x4 + iVar15);
		puVar18 = (undefined4*)((gColTD.pCurColObj)->field_0x18 + iVar15);
		puVar11 = puVar16;
		puVar17 = puVar18;
		do {
			uVar1 = *puVar11;
			uVar5 = *(undefined4*)(puVar11 + 1);
			uVar7 = *(undefined4*)((int)puVar11 + 0xc);
			iVar9 = iVar9 + -1;
			*puVar17 = (int)uVar1;
			puVar17[1] = (int)((ulong)uVar1 >> 0x20);
			puVar17[2] = uVar5;
			puVar17[3] = uVar7;
			puVar11 = puVar11 + 2;
			puVar17 = puVar17 + 4;
		} while (0 < iVar9);
		uVar12 = *(undefined4*)((int)puVar16 + 0x34);
		uVar13 = *(undefined4*)(puVar16 + 7);
		uVar14 = *(undefined4*)((int)puVar16 + 0x3c);
		puVar11 = puVar16 + 10;
		uVar5 = *(undefined4*)(puVar16 + 8);
		uVar7 = *(undefined4*)((int)puVar16 + 0x44);
		uVar6 = *(undefined4*)(puVar16 + 9);
		uVar8 = *(undefined4*)((int)puVar16 + 0x4c);
		puVar17 = puVar18 + 0x14;
		iVar9 = 7;
		puVar18[0xc] = *(undefined4*)(puVar16 + 6);
		puVar18[0xd] = uVar12;
		puVar18[0xe] = uVar13;
		puVar18[0xf] = uVar14;
		puVar18[0x10] = uVar5;
		puVar18[0x11] = uVar7;
		puVar18[0x12] = uVar6;
		puVar18[0x13] = uVar8;
		do {
			uVar1 = *puVar11;
			uVar5 = *(undefined4*)(puVar11 + 1);
			uVar7 = *(undefined4*)((int)puVar11 + 0xc);
			iVar9 = iVar9 + -1;
			*puVar17 = (int)uVar1;
			puVar17[1] = (int)((ulong)uVar1 >> 0x20);
			puVar17[2] = uVar5;
			puVar17[3] = uVar7;
			puVar11 = puVar11 + 2;
			puVar17 = puVar17 + 4;
		} while (0 < iVar9);
		puVar17 = (undefined4*)((gColTD.pCurColObj)->field_0x1c + iVar15);
		if ((gColTD.pCurColObj)->field_0x1c != 0) {
			iVar9 = 3;
			puVar16 = (undefined8*)(gColTD.field_0x4 + iVar15);
			puVar18 = puVar17;
			puVar11 = puVar16;
			do {
				uVar1 = *puVar11;
				uVar5 = *(undefined4*)(puVar11 + 1);
				uVar7 = *(undefined4*)((int)puVar11 + 0xc);
				iVar9 = iVar9 + -1;
				*puVar18 = (int)uVar1;
				puVar18[1] = (int)((ulong)uVar1 >> 0x20);
				puVar18[2] = uVar5;
				puVar18[3] = uVar7;
				puVar11 = puVar11 + 2;
				puVar18 = puVar18 + 4;
			} while (0 < iVar9);
			uVar12 = *(undefined4*)((int)puVar16 + 0x34);
			uVar13 = *(undefined4*)(puVar16 + 7);
			uVar14 = *(undefined4*)((int)puVar16 + 0x3c);
			puVar11 = puVar16 + 10;
			uVar5 = *(undefined4*)(puVar16 + 8);
			uVar7 = *(undefined4*)((int)puVar16 + 0x44);
			uVar6 = *(undefined4*)(puVar16 + 9);
			uVar8 = *(undefined4*)((int)puVar16 + 0x4c);
			puVar18 = puVar17 + 0x14;
			iVar9 = 7;
			puVar17[0xc] = *(undefined4*)(puVar16 + 6);
			puVar17[0xd] = uVar12;
			puVar17[0xe] = uVar13;
			puVar17[0xf] = uVar14;
			puVar17[0x10] = uVar5;
			puVar17[0x11] = uVar7;
			puVar17[0x12] = uVar6;
			puVar17[0x13] = uVar8;
			do {
				uVar1 = *puVar11;
				uVar5 = *(undefined4*)(puVar11 + 1);
				uVar7 = *(undefined4*)((int)puVar11 + 0xc);
				iVar9 = iVar9 + -1;
				*puVar18 = (int)uVar1;
				puVar18[1] = (int)((ulong)uVar1 >> 0x20);
				puVar18[2] = uVar5;
				puVar18[3] = uVar7;
				puVar11 = puVar11 + 2;
				puVar18 = puVar18 + 4;
			} while (0 < iVar9);
		})
	}

	// Copy prims.
	for (iVar2 = 0; iVar2 < gColTD.nbPrim; iVar2 = iVar2 + 1) {
		pIVar10 = gColTD.aPrim + iVar2;
		edColPrimEntry* pPrimStart = (edColPrimEntry*)LOAD_SECTION((gColTD.pCurColObj)->pPrim);
		edColPrimEntry* peVar4 = pPrimStart + iVar2;
		*peVar4 = *pIVar10;
	}

	(gColTD.pCurColObj)->field_0x4 = 1;
	(gColTD.pCurColObj)->field_0x5 = 1;
	(gColTD.pCurColObj)->field_0xc = (gColData.pActiveDatabase)->curObjId;
	gColTD.pCurColObj = gColTD.pCurColObj + 1;
	(gColData.pActiveDatabase)->curObjId = (gColData.pActiveDatabase)->curObjId + 1;
	gColTD.field_0x0 = 0;
	return gColTD.pCurColObj + -1;
}
