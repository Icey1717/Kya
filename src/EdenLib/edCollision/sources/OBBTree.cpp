#include "edCollision/OBBTree.h"
#include "edCollision/edCollisions.h"
#include "profile.h"
#include "MathOps.h"

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
	edColARRAY_PRIM_PRIM local_fc0;
	edColARRAY_TRI4_PRIM local_f50;
	//edColARRAY_VERT_IND_PRIM local_f90;
	//edColARRAY_PRIM_TRI4_FAST local_f30;
	//edColARRAY_PRIM_TRI4_FAST local_f00;
	edColARRAY_PRIM_TRI4 local_ed0;
	edColARRAY_PRIM_TRI4 local_eb0;
	//edColARRAY_VERT_IND_QUAD4 local_e70;
	undefined local_e50[16];
	uint local_e40;
	edF32TRIANGLE4* local_e3c;
	IntersectResTestA aIntersectResultsA[100];
	edColPrimEntry aIntersectResultsB[100];
	edObbTREE_DYN* local_340[2][100];
	edColRAY_TRIANGLE4_IN local_20;
	int local_10[3];
	float local_4;

	uVar13 = 0;
	uVar10 = 0;
	local_340[0][0] = pObbTreeB;
	local_10[0] = 1;
	intersectCountA = 0;
	local_10[1] = 0;
	intersectCountB = 0;
	if (gColConfig.bCreateProfileObj != 0) {
		edProfileBegin(prof_obb_col);
	}

	// Initial BB tests.
	COLLISION_LOG_VERBOSE(LogLevel::VeryVerbose, "\nedObbTreeIntersectObbTree");

	iVar8 = 0;
	do {
		uVar5 = uVar10 != 0 ^ 1;
		ppeVar16 = local_340[0] + (iVar8 + uVar10) * 0x14;
		local_10[uVar5] = 0;
		ppeVar20 = local_340[uVar5];

		COLLISION_LOG_VERBOSE(LogLevel::VeryVerbose, "\nedObbTreeIntersectObbTree flip: {} count: {}", uVar5, local_10[uVar10]);

		for (iVar8 = 0; iVar8 < local_10[uVar10]; iVar8 = iVar8 + 1) {
			peVar2 = *ppeVar16;
			iVar18 = 0;

			COLLISION_LOG_VERBOSE(LogLevel::VeryVerbose, "edObbTreeIntersectObbTree type: {} (0x{:x})", peVar2->type, peVar2->type);

			if (peVar2->type == COL_TYPE_TREE) {

				COLLISION_LOG_VERBOSE(LogLevel::VeryVerbose, "edObbTreeIntersectObbTree count: {}", peVar2->count_0x52);
				bVar1 = peVar2->count_0x52;
				for (; iVar18 < bVar1; iVar18 = iVar18 + 1) {

					COLLISION_LOG_VERBOSE(LogLevel::VeryVerbose, "edObbTreeIntersectObbTree checking: {}", iVar18);

					edObbTREE_DYN* pLinkedTree = (edObbTREE_DYN*)LOAD_POINTER(peVar2->field_0x54[iVar18]);
					lVar6 = edObbIntersect(&pObbTreeA->bbox, &pLinkedTree->bbox);

					COLLISION_LOG_VERBOSE(LogLevel::VeryVerbose, "edObbTreeIntersectObbTree edObbIntersect result: {}", lVar6);

					if (lVar6 != 0) {
						uVar5 = uVar10 != 0 ^ 1;
						if (local_10[uVar5] < 100) {
							*ppeVar20 = pLinkedTree;
							ppeVar20 = ppeVar20 + 1;
							local_10[uVar5] = local_10[uVar5] + 1;

							COLLISION_LOG_VERBOSE(LogLevel::VeryVerbose, "edObbTreeIntersectObbTree flip index: {} new count: {}", uVar5, local_10[uVar5]);
						}
					}
				}
			}
			else {
				lVar6 = edObbIntersect(&pObbTreeA->bbox, &peVar2->bbox);

				COLLISION_LOG_VERBOSE(LogLevel::VeryVerbose, "edObbTreeIntersectObbTree edObbIntersect {}\n{}", pObbTreeA->bbox.transform.ToString(), peVar2->bbox.transform.ToString());
				COLLISION_LOG_VERBOSE(LogLevel::VeryVerbose, "edObbTreeIntersectObbTree edObbIntersect result: {}", lVar6);

				if ((lVar6 != 0) && (intersectCountA < 100)) {
					aIntersectResultsA[intersectCountA].pObbTree = peVar2;
					aIntersectResultsA[intersectCountA].sub.type = peVar2->type;
					aIntersectResultsA[intersectCountA].sub.count = peVar2->count_0x52;
					aIntersectResultsA[intersectCountA].sub.pData = LOAD_POINTER_CAST(void*, peVar2->field_0x54[0]);
					intersectCountA = intersectCountA + 1;

					COLLISION_LOG_VERBOSE(LogLevel::VeryVerbose, "edObbTreeIntersectObbTree new intersect count: {} (type: 0x{:x} - count: 0x{:x})", intersectCountA, peVar2->type, peVar2->count_0x52);
				}
			}

			ppeVar16 = ppeVar16 + 1;
		}

		uVar10 = uVar10 != 0 ^ 1;
		iVar8 = uVar10 << 2;
	} while (local_10[uVar10] != 0);

	COLLISION_LOG_VERBOSE(LogLevel::VeryVerbose, "\nedObbTreeIntersectObbTree Process A Results");

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
				if (peVar3->type == COL_TYPE_TREE) {
					bVar1 = peVar3->count_0x52;
					for (; iVar17 < bVar1; iVar17 = iVar17 + 1) {
						edObbTREE_DYN* pLinkedTree = (edObbTREE_DYN*)LOAD_POINTER(peVar3->field_0x54[iVar17]);

						lVar6 = edObbIntersect(&peVar2->bbox, &pLinkedTree->bbox);
						if (lVar6 != 0) {
							uVar5 = uVar10 != 0 ^ 1;
							if (local_10[uVar5] < 100) {
								*ppeVar20 = pLinkedTree;
								ppeVar20 = ppeVar20 + 1;
								local_10[uVar5] = local_10[uVar5] + 1;
							}
						}
						bVar1 = peVar3->count_0x52;
					}
				}
				else {
					lVar6 = edObbIntersect(&peVar2->bbox, &peVar3->bbox);
					if ((lVar6 != 0) && (intersectCountB < 100)) {
						aIntersectResultsB[intersectCountB].a.type = peVar3->type;
						aIntersectResultsB[intersectCountB].a.count = peVar3->count_0x52;
						aIntersectResultsB[intersectCountB].a.pData = LOAD_POINTER_CAST(void*, peVar3->field_0x54[0]);

						aIntersectResultsB[intersectCountB].b = aIntersectResultsA[iVar8].sub;

						intersectCountB = intersectCountB + 1;

						COLLISION_LOG_VERBOSE(LogLevel::VeryVerbose, "edObbTreeIntersectObbTree new intersect B count: {} (type: 0x{:x} - count: 0x{:x})", intersectCountB, peVar3->type, peVar3->count_0x52);

					}
				}

				ppeVar16 = ppeVar16 + 1;
			}
			uVar10 = uVar10 != 0 ^ 1;
		} while (local_10[uVar10] != 0);
	}

	edProfileEnd(prof_obb_col);

	edProfileBegin(prof_prim_col);

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
						local_20.pRayOrigin = (float*)(uVar5 + 0x10);
						local_20.pRayDirection = (float*)(uVar5 + 0x30);
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
						local_20.pRayOrigin = (float*)(uVar5 + 0x10);
						local_20.pRayDirection = (float*)(uVar5 + 0x30);
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
							local_20.pRayDirection = (float*)(uVar5 + 0x30);
							local_20.pRayOrigin = (float*)(uVar5 + 0x10);
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
						COLLISION_LOG_VERBOSE(LogLevel::VeryVerbose, "edObbTreeIntersectObbTree edColArrayObjectPrimsPenatratingArrayPrims 0x{:x}", uVar5);
						local_fc0.pColObj = pColInfoObbTree->pColObj;
						local_fc0.aData = (puVar12->a).pData;
						local_fc0.aCount = (puVar12->a).count;
						local_fc0.primSize = _gColSizeOfPrims[(puVar12->a).type];
						local_fc0.aType = (puVar12->a).type;
						local_fc0.pOtherColObj = pColInfoObbTree->pOtherColObj;
						local_fc0.bData = (puVar12->b).pData;
						local_fc0.bCount = (puVar12->b).count;
						local_fc0.bPrimSize = _gColSizeOfPrims[(puVar12->b).type];
						local_fc0.bType = (puVar12->b).type;

						uVar5 = edColArrayObjectPrimsPenatratingArrayPrims(&local_fc0);
						uVar13 = uVar13 | uVar5;
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
								COLLISION_LOG_VERBOSE(LogLevel::VeryVerbose, "edObbTreeIntersectObbTree edColArrayObjectPrimPenatratingArrayQuads4 0x{:x}", uVar5);
								local_f70.pColObj = pColInfoObbTree->pColObj;
								local_f70.aData = puVar12->a.pData;
								local_f70.aCount = puVar12->a.count;
								local_f70.primSize = _gColSizeOfPrims[(puVar12->a).type];
								local_f70.aType = puVar12->a.type;
								local_f70.pOtherColObj = pColInfoObbTree->pOtherColObj;
								local_f70.bData = puVar12->b.pData;
								local_f70.bCount = puVar12->b.count;

								COLLISION_LOG_VERBOSE(LogLevel::VeryVerbose, "edObbTreeIntersectObbTree aCount: 0x{:x} aType: 0x{:x} bCount: 0x{:x} primSize: 0x{:x}",
									local_f70.aCount, local_f70.aType, local_f70.bCount, local_f70.primSize);

								uVar5 = edColArrayObjectPrimPenatratingArrayQuads4(&local_f70);
								COLLISION_LOG_VERBOSE(LogLevel::VeryVerbose, "edObbTreeIntersectObbTree edColArrayObjectPrimPenatratingArrayQuads4 result: 0x{:x}", uVar5);
								uVar13 = uVar13 | uVar5;
							}
							else {
								if ((((uVar5 == 0x40d) || (uVar5 == 0x40e)) || (uVar5 == 0x40a)) || (uVar5 == 0x40b)) {
									COLLISION_LOG_VERBOSE(LogLevel::VeryVerbose, "edObbTreeIntersectObbTree edColArrayObjectTriangles4PenatratingPrims 0x{:x}", uVar5);
									local_f50.pOtherColObj = pColInfoObbTree->pOtherColObj;
									local_f50.bData = (puVar12->b).pData;
									local_f50.bCount = (puVar12->b).count;
									local_f50.bSize = _gColSizeOfPrims[(puVar12->b).type];
									local_f50.bType = (puVar12->b).type;
									local_f50.pColObj = pColInfoObbTree->pColObj;
									local_f50.aData = (puVar12->a).pData;
									local_f50.bCount2 = (ushort)(puVar12->b).count;
									uVar5 = edColArrayObjectTriangles4PenatratingPrims(&local_f50);
									uVar13 = uVar13 | uVar5;
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

	edProfileEnd(prof_prim_col);

	return uVar13;
}

float edObbIntersectObbRay(edObbBOX* pBox, edF32VECTOR4* pRayStart, edF32VECTOR4* pRayEnd)
{
	float fVar1;
	float fVar2;
	float fVar3;
	float fVar4;
	float fVar5;
	float fVar6;
	bool bVar7;
	bool bVar8;
	bool bVar9;
	bool bVar10;
	bool bVar11;
	float fVar12;
	bool bVar13;
	bool bVar14;
	bool bVar15;
	bool bVar16;
	bool bVar17;
	bool bVar18;
	float distance;
	float fVar19;
	float local_8;
	float local_4;

	COLLISION_LOG_VERBOSE(LogLevel::Verbose, "edObbIntersectObbRay pRayStart: {} pRayEnd: {}", pRayStart->ToString(), pRayEnd->ToString());

	fVar12 = FLOAT_004485a4;
	fVar1 = pRayStart->x - (pBox->transform).da;
	fVar5 = pRayStart->y - (pBox->transform).db;
	fVar6 = pRayStart->z - (pBox->transform).dc;
	fVar2 = fVar1 * (pBox->transform).aa + fVar5 * (pBox->transform).ab + fVar6 * (pBox->transform).ac;
	fVar3 = fVar1 * (pBox->transform).ba + fVar5 * (pBox->transform).bb + fVar6 * (pBox->transform).bc;
	fVar1 = fVar1 * (pBox->transform).ca + fVar5 * (pBox->transform).cb + fVar6 * (pBox->transform).cc;
	fVar5 = pRayEnd->x * (pBox->transform).aa + pRayEnd->y * (pBox->transform).ab + pRayEnd->z * (pBox->transform).ac;
	fVar6 = pRayEnd->x * (pBox->transform).ba + pRayEnd->y * (pBox->transform).bb + pRayEnd->z * (pBox->transform).bc;
	fVar4 = pRayEnd->x * (pBox->transform).ca + pRayEnd->y * (pBox->transform).cb + pRayEnd->z * (pBox->transform).cc;
	local_4 = 0.0f;
	local_8 = FLOAT_004485a4;
	fVar19 = 0.0f;
	bVar13 = edObbRayPlaneClip(fVar5, -fVar2 - pBox->width, &local_4, &local_8);
	bVar14 = edObbRayPlaneClip(-fVar5, fVar2 - pBox->width, &local_4, &local_8);
	bVar15 = edObbRayPlaneClip(fVar6, -fVar3 - pBox->height, &local_4, &local_8);
	bVar16 = edObbRayPlaneClip(-fVar6, fVar3 - pBox->height, &local_4, &local_8);
	bVar17 = edObbRayPlaneClip(fVar4, -fVar1 - pBox->depth, &local_4, &local_8);
	bVar18 = edObbRayPlaneClip(-fVar4, fVar1 - pBox->depth, &local_4, &local_8);

	bVar11 = false;
	bVar10 = false;
	bVar9 = false;
	bVar8 = false;
	bVar7 = false;

	if ((bVar13 != false) && (bVar14 != false)) {
		bVar7 = true;
	}

	if ((bVar7) && (bVar15 != false)) {
		bVar8 = true;
	}

	if ((bVar8) && (bVar16 != false)) {
		bVar9 = true;
	}

	if ((bVar9) && (bVar17 != false)) {
		bVar10 = true;
	}

	if ((bVar10) && (bVar18 != false)) {
		bVar11 = true;
	}

	bVar7 = false;

	if (bVar11) {
		bVar8 = true;
		if ((local_4 == fVar19) && (local_8 == fVar12)) {
			bVar8 = false;
		}
		if (bVar8) {
			bVar7 = true;
		}
	}

	if (bVar7) {
		distance = 0.0f;

		if (0.0f < local_4) {
			if (local_8 < FLOAT_004485a4) {
				distance = std::min(local_4, local_8); //(float)((int)local_4 * (uint)(local_4 < local_8) | (int)local_8 * (uint)(local_4 >= local_8));
			}
		}
		else {
			distance = -1.0f;

			if (local_8 < FLOAT_004485a4) {
				return 0.0f;
			}
		}
	}
	else {
		distance = -1.0f;
	}

	COLLISION_LOG_VERBOSE(LogLevel::Verbose, "edObbIntersectObbRay distance: {}", distance);

	return distance;
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

bool edObbRayPlaneClip(float param_1, float param_2, float* param_3, float* param_4)
{
	bool bVar1;

	if (0.0f < param_1) {
		bVar1 = false;
		if (param_2 <= param_1 * *param_4) {
			if (param_1 * *param_3 < param_2) {
				*param_3 = param_2 / param_1;
			}
			bVar1 = true;
		}
	}
	else {
		if (param_1 < 0.0f) {
			bVar1 = false;
			if (param_2 <= param_1 * *param_3) {
				if (param_1 * *param_4 < param_2) {
					*param_4 = param_2 / param_1;
				}
				bVar1 = true;
			}
		}
		else {
			bVar1 = param_2 <= 0.0f;
		}
	}

	COLLISION_LOG_VERBOSE(LogLevel::Verbose, "edObbRayPlaneClip bVar1: {} f1: {} f2: {}", bVar1, *param_3, *param_4);

	return bVar1;
}


uint CheckRayObbTreeIntersection(edObbTREE_DYN* pObbTree, edF32VECTOR4* pStart, edF32VECTOR4* pEnd)
{
	float projectedTranslationX;
	float projectedTranslationY;
	float projectedTranslationZ;
	float startProjX;
	float startProjY;
	float startProjZ;
	float endProjX;
	float endProjY;
	float endProjZ;

	edF32VECTOR3 rowX;
	edF32VECTOR3 rowY;
	edF32VECTOR3 rowZ;
	float height;
	float width;
	float depth;
	edF32VECTOR3 rowT;
	edF32VECTOR4 start;
	edF32VECTOR4 end;

	rowX = (pObbTree->bbox).transform.rowX.xyz;
	rowY = (pObbTree->bbox).transform.rowY.xyz;
	rowZ = (pObbTree->bbox).transform.rowZ.xyz;
	rowT = (pObbTree->bbox).transform.rowT.xyz;

	start = *pStart;
	end = *pEnd;

	// Compute projected translations along each axis
	projectedTranslationX = 0.0f - (rowX.x * rowT.x + rowX.y * rowT.y + rowX.z * rowT.z);
	projectedTranslationY = 0.0f - (rowY.x * rowT.x + rowY.y * rowT.y + rowY.z * rowT.z);
	projectedTranslationZ = 0.0f - (rowZ.x * rowT.x + rowZ.y * rowT.y + rowZ.z * rowT.z);

	// Project the start point onto each axis
	startProjX = rowX.x * start.x + rowX.y * start.y + rowX.z * start.z + projectedTranslationX * start.w;
	startProjY = rowY.x * start.x + rowY.y * start.y + rowY.z * start.z + projectedTranslationY * start.w;
	startProjZ = rowZ.x * start.x + rowZ.y * start.y + rowZ.z * start.z + projectedTranslationZ * start.w;

	// Project the end point onto each axis
	endProjX = rowX.x * end.x + rowX.y * end.y + rowX.z * end.z + projectedTranslationX * end.w;
	endProjY = rowY.x * end.x + rowY.y * end.y + rowY.z * end.z + projectedTranslationY * end.w;
	endProjZ = rowZ.x * end.x + rowZ.y * end.y + rowZ.z * end.z + projectedTranslationZ * end.w;

	// Get the box dimensions
	width = (pObbTree->bbox).width;
	height = (pObbTree->bbox).height;
	depth = (pObbTree->bbox).depth;

	// Perform boundary checks for intersection
	return (uint)(width < startProjX) + (((uint)(startProjX < -width) << 0x17) >> 0x16) +
		(((uint)(height < startProjY) << 0x17) >> 0x15) + (((uint)(startProjY < -height) << 0x17) >> 0x14) +
		(((uint)(depth < startProjZ) << 0x17) >> 0x13) + (((uint)(startProjZ < -depth) << 0x17) >> 0x12) &
		(uint)(width < endProjX) + (((uint)(endProjX < -width) << 0x17) >> 0x16) + (((uint)(height < endProjY) << 0x17) >> 0x15)
		+ (((uint)(endProjY < -height) << 0x17) >> 0x14) + (((uint)(depth < endProjZ) << 0x17) >> 0x13) +
		(((uint)(endProjZ < -depth) << 0x17) >> 0x12);
}

int INT_ARRAY_00448910[2] = { 1, 0 };

float edObbIntersectObbTreeRayPrim(void** pOutHit, uint* pOutType, edObbTREE_DYN* pObbTree, edColRAY_OBB_IN* pRay)
{
	byte bVar1;
	edF32VECTOR4* pDirection;
	edF32VECTOR4* pLocation;
	edObbTREE_DYN* peVar4;
	bool bVar5;
	uint uVar6;
	int iVar7;
	uint uVar8;
	edObbTREE_DYN* peVar9;
	int iVar10;
	int iVar11;
	float fVar12;
	float distance;
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
	float fVar25;
	float fVar26;
	float fVar27;
	edF32VECTOR4 local_610;
	edF32VECTOR4 local_600;
	float local_5f0;
	float fStack1516;
	float fStack1512;
	float fStack1508;
	float local_5e0;
	float fStack1500;
	float fStack1496;
	float fStack1492;
	undefined auStack1488[32];
	float local_5b0;
	float fStack1452;
	float fStack1448;
	float fStack1444;
	int local_590;
	edF32VECTOR4 local_570;
	edF32VECTOR4 local_560;
	edF32VECTOR4 local_550;
	edF32VECTOR4 local_540;
	undefined auStack1328[32];
	edF32VECTOR4 local_510;
	int local_4f0;
	edF32VECTOR4 local_4d0;
	edF32VECTOR4 local_4c0;
	edF32VECTOR4 local_4b0;
	edF32VECTOR4 local_4a0;
	//TraceData48 TStack1168;
	edF32VECTOR4 local_430;
	edF32VECTOR4 local_420;
	edF32VECTOR4 local_410;
	edF32VECTOR4 local_400;
	//TraceData48 TStack1008;
	undefined local_390[20];
	float local_37c;
	float local_378;
	edF32VECTOR4 endLocation;
	edObbTREE_DYN* local_360[202];
	float* local_38;
	float* local_34;
	float* local_30;
	float* local_2c;
	edF32VECTOR4* local_28;
	float* local_24;
	edF32VECTOR4* local_20[2];
	int local_18[2];
	edColRAY_TRIANGLE4_IN local_10;
	float outDistance;

	distance = -1.0f;
	uVar8 = 0;
	outDistance = -1.0f;

	local_18[0] = INT_ARRAY_00448910[0];
	local_18[1] = INT_ARRAY_00448910[1];
	fVar12 = pRay->lengthA;
	pDirection = pRay->pLeadVector;
	pLocation = pRay->pLocation;
	endLocation.xyz = (pDirection->xyz * fVar12) + pLocation->xyz;
	endLocation.w = 1.0f;
	local_360[0] = pObbTree;
	uVar6 = CheckRayObbTreeIntersection(pObbTree, pRay->pLocation, &endLocation);

	COLLISION_LOG_VERBOSE(LogLevel::Verbose, "edObbIntersectObbTreeRayPrim CheckRayObbTreeIntersection {}", uVar6);

	if (uVar6 == 0) {
		bVar5 = false;
		do {
			iVar11 = 0;
			local_18[bVar5 ^ 1] = 0;

			while (iVar11 < local_18[uVar8]) {
				peVar4 = local_360[uVar8 * 100 + iVar11];
				bVar1 = peVar4->type;
				COLLISION_LOG_VERBOSE(LogLevel::Verbose, "edObbIntersectObbTreeRayPrim type 0x{:x}", bVar1);

				if (bVar1 == COL_TYPE_TREE) {
					COLLISION_LOG_VERBOSE(LogLevel::Verbose, "edObbIntersectObbTreeRayPrim count {}", peVar4->count_0x52);

					for (iVar7 = 0; iVar7 < peVar4->count_0x52; iVar7 = iVar7 + 1) {
						uVar6 = CheckRayObbTreeIntersection(LOAD_POINTER_CAST(edObbTREE_DYN*, peVar4->field_0x54[iVar7]), pRay->pLocation, &endLocation);
						COLLISION_LOG_VERBOSE(LogLevel::Verbose, "edObbIntersectObbTreeRayPrim CheckRayObbTreeIntersection {}", uVar6);

						if (uVar6 == 0) {
							uVar6 = uVar8 != 0 ^ 1;
							if (local_18[uVar6] < 100) {
								local_360[uVar6 * 100 + local_18[uVar6]] = LOAD_POINTER_CAST(edObbTREE_DYN*, peVar4->field_0x54[iVar7]);
								local_18[uVar6] = local_18[uVar6] + 1;
							}
						}
					}

					iVar11 = iVar11 + 1;
				}
				else {
					if (bVar1 == COL_TYPE_BOX_DYN) {
						edColPRIM_OBJECT* pPrim = LOAD_POINTER_CAST(edColPRIM_OBJECT*, peVar4->field_0x54[0]);
						COLLISION_LOG_VERBOSE(LogLevel::Verbose, "edObbIntersectObbTreeRayPrim PRIM count {}", peVar4->count_0x52);

						for (iVar7 = 0; iVar7 < peVar4->count_0x52; iVar7 = iVar7 + 1) {
							pDirection = pRay->pLeadVector;
							pLocation = pRay->pLocation;

							fVar22 = pLocation->x;
							fVar24 = pLocation->y;
							fVar26 = pLocation->z;
							float fVar28 = pLocation->w;
							float fVar30 = pDirection->x;
							float fVar32 = pDirection->y;
							float fVar34 = pDirection->z;
							float fVar36 = pDirection->w;

							fVar13 = (pPrim->worldTransform).aa;
							fVar14 = (pPrim->worldTransform).ab;
							fVar15 = (pPrim->worldTransform).ac;
							fVar16 = (pPrim->worldTransform).ad;
							fVar17 = (pPrim->worldTransform).ba;
							fVar18 = (pPrim->worldTransform).bb;
							fVar19 = (pPrim->worldTransform).bc;
							fVar20 = (pPrim->worldTransform).bd;
							fVar21 = (pPrim->worldTransform).ca;
							fVar23 = (pPrim->worldTransform).cb;
							fVar25 = (pPrim->worldTransform).cc;
							fVar27 = (pPrim->worldTransform).cd;
							float fVar29 = (pPrim->worldTransform).da;
							float fVar31 = (pPrim->worldTransform).db;
							float fVar33 = (pPrim->worldTransform).dc;
							float fVar35 = (pPrim->worldTransform).dd;

							edF32VECTOR4 local_5e0;
							edF32VECTOR4 local_5f0;

							local_5e0.x = fVar13 * fVar22 + fVar17 * fVar24 + fVar21 * fVar26 + fVar29 * fVar28;
							local_5e0.y = fVar14 * fVar22 + fVar18 * fVar24 + fVar23 * fVar26 + fVar31 * fVar28;
							local_5e0.z = fVar15 * fVar22 + fVar19 * fVar24 + fVar25 * fVar26 + fVar33 * fVar28;
							local_5e0.w = fVar16 * fVar22 + fVar20 * fVar24 + fVar27 * fVar26 + fVar35 * fVar28;

							local_5f0.x = fVar13 * fVar30 + fVar17 * fVar32 + fVar21 * fVar34 + fVar29 * fVar36;
							local_5f0.y = fVar14 * fVar30 + fVar18 * fVar32 + fVar23 * fVar34 + fVar31 * fVar36;
							local_5f0.z = fVar15 * fVar30 + fVar19 * fVar32 + fVar25 * fVar34 + fVar33 * fVar36;
							local_5f0.w = fVar16 * fVar30 + fVar20 * fVar32 + fVar27 * fVar34 + fVar35 * fVar36;

							edColPRIM_RAY_UNIT_BOX_UNIT_IN primRayUnitBoxUnitIn;
							primRayUnitBoxUnitIn.field_0x4 = &local_5f0;
							primRayUnitBoxUnitIn.field_0x0 = &local_5e0;

							edColINFO_OUT colOut;

							edColIntersectRayUnitBoxUnit(&colOut, &primRayUnitBoxUnitIn);

							COLLISION_LOG_VERBOSE(LogLevel::Verbose, "edObbIntersectObbTreeRayPrim edColIntersectRayUnitBoxUnit result {}", colOut.result);

							if (colOut.result != 0) {
								COLLISION_LOG_VERBOSE(LogLevel::Verbose, "edObbIntersectObbTreeRayPrim edColIntersectRayUnitBoxUnit intersectionPoint {}", colOut.intersectionPoint.ToString());
								COLLISION_LOG_VERBOSE(LogLevel::Verbose, "edObbIntersectObbTreeRayPrim edColIntersectRayUnitBoxUnit vertices {}", pPrim->localToWorld.ToString());

								local_600.x = (pPrim->localToWorld).aa * colOut.intersectionPoint.x +
									(pPrim->localToWorld).ba * colOut.intersectionPoint.y +
									(pPrim->localToWorld).ca * colOut.intersectionPoint.z +
									(pPrim->localToWorld).da * colOut.intersectionPoint.w;

								local_600.y = (pPrim->localToWorld).ab * colOut.intersectionPoint.x +
									(pPrim->localToWorld).bb * colOut.intersectionPoint.y +
									(pPrim->localToWorld).cb * colOut.intersectionPoint.z +
									(pPrim->localToWorld).db * colOut.intersectionPoint.w;

								local_600.z = (pPrim->localToWorld).ac * colOut.intersectionPoint.x +
									(pPrim->localToWorld).bc * colOut.intersectionPoint.y +
									(pPrim->localToWorld).cc * colOut.intersectionPoint.z +
									(pPrim->localToWorld).dc * colOut.intersectionPoint.w;

								local_600.w = (pPrim->localToWorld).ad * colOut.intersectionPoint.x +
									(pPrim->localToWorld).bd * colOut.intersectionPoint.y +
									(pPrim->localToWorld).cd * colOut.intersectionPoint.z +
									(pPrim->localToWorld).dd * colOut.intersectionPoint.w;

								pLocation = pRay->pLocation;

								local_610 = local_600 - *pLocation;

								COLLISION_LOG_VERBOSE(LogLevel::Verbose, "edObbIntersectObbTreeRayPrim local_600: {}", local_600.ToString());
								COLLISION_LOG_VERBOSE(LogLevel::Verbose, "edObbIntersectObbTreeRayPrim pLocation {}", pLocation->ToString());
								COLLISION_LOG_VERBOSE(LogLevel::Verbose, "edObbIntersectObbTreeRayPrim local_610 {}", local_610.ToString());

								outDistance = edF32Vector4GetDistHard(&local_610);

								COLLISION_LOG_VERBOSE(LogLevel::Verbose, "edObbIntersectObbTreeRayPrim outDistance {} distance: {} ray length: {}", outDistance, distance, pRay->lengthA);

								if ((outDistance < distance) || ((distance < 0.0f && (outDistance <= pRay->lengthA)))) {
									*pOutHit = pPrim;
									*pOutType = 10;
									distance = outDistance;
								}
							}

							pPrim = pPrim + 1;
						}
					}
					else {
						if (bVar1 == COL_TYPE_BOX) {
							edColPRIM_BOX* pBox = LOAD_POINTER_CAST(edColPRIM_BOX*, peVar4->field_0x54[0]);
							for (iVar7 = 0; iVar7 < peVar4->count_0x52; iVar7 = iVar7 + 1) {
								pDirection = pRay->pLeadVector;
								pLocation = pRay->pLocation;

								edF32VECTOR4 local_5e0;
								edF32VECTOR4 local_5f0;

								local_5e0.x = (pBox->worldTransform).aa * pLocation->x + (pBox->worldTransform).ba * pLocation->y + (pBox->worldTransform).ca * pLocation->z + (pBox->worldTransform).da * pLocation->w;
								local_5e0.y = (pBox->worldTransform).ab * pLocation->x + (pBox->worldTransform).bb * pLocation->y + (pBox->worldTransform).cb * pLocation->z + (pBox->worldTransform).db * pLocation->w;
								local_5e0.z = (pBox->worldTransform).ac * pLocation->x + (pBox->worldTransform).bc * pLocation->y + (pBox->worldTransform).cc * pLocation->z + (pBox->worldTransform).dc * pLocation->w;
								local_5e0.w = (pBox->worldTransform).ad * pLocation->x + (pBox->worldTransform).bd * pLocation->y + (pBox->worldTransform).cd * pLocation->z + (pBox->worldTransform).dd * pLocation->w;

								local_5f0.x = (pBox->worldTransform).aa * pDirection->x + (pBox->worldTransform).ba * pDirection->y + (pBox->worldTransform).ca * pDirection->z + (pBox->worldTransform).da * pDirection->w;
								local_5f0.y = (pBox->worldTransform).ab * pDirection->x + (pBox->worldTransform).bb * pDirection->y + (pBox->worldTransform).cb * pDirection->z + (pBox->worldTransform).db * pDirection->w;
								local_5f0.z = (pBox->worldTransform).ac * pDirection->x + (pBox->worldTransform).bc * pDirection->y + (pBox->worldTransform).cc * pDirection->z + (pBox->worldTransform).dc * pDirection->w;
								local_5f0.w = (pBox->worldTransform).ad * pDirection->x + (pBox->worldTransform).bd * pDirection->y + (pBox->worldTransform).cd * pDirection->z + (pBox->worldTransform).dd * pDirection->w;

								edColPRIM_RAY_UNIT_BOX_UNIT_IN local_38;
								local_38.field_0x4 = &local_5f0;
								local_38.field_0x0 = &local_5e0;

								edColINFO_OUT eStack1488;

								edColIntersectRayUnitBoxUnit(&eStack1488, &local_38);

								if (eStack1488.result != 0) {
									local_600.x = (pBox->localToWorld).aa * eStack1488.intersectionPoint.x +
										(pBox->localToWorld).ba * eStack1488.intersectionPoint.y +
										(pBox->localToWorld).ca * eStack1488.intersectionPoint.z +
										(pBox->localToWorld).da * eStack1488.intersectionPoint.w;
									local_600.y = (pBox->localToWorld).ab * eStack1488.intersectionPoint.x +
										(pBox->localToWorld).bb * eStack1488.intersectionPoint.y +
										(pBox->localToWorld).cb * eStack1488.intersectionPoint.z +
										(pBox->localToWorld).db * eStack1488.intersectionPoint.w;
									local_600.z = (pBox->localToWorld).ac * eStack1488.intersectionPoint.x +
										(pBox->localToWorld).bc * eStack1488.intersectionPoint.y +
										(pBox->localToWorld).cc * eStack1488.intersectionPoint.z +
										(pBox->localToWorld).dc * eStack1488.intersectionPoint.w;
									local_600.w = (pBox->localToWorld).ad * eStack1488.intersectionPoint.x +
										(pBox->localToWorld).bd * eStack1488.intersectionPoint.y +
										(pBox->localToWorld).cd * eStack1488.intersectionPoint.z +
										(pBox->localToWorld).dd * eStack1488.intersectionPoint.w;

									pLocation = pRay->pLocation;

									local_610 = local_600 - *pLocation;

									outDistance = edF32Vector4GetDistHard(&local_610);
									if ((outDistance < distance) || ((distance < 0.0f && (outDistance <= pRay->lengthA)))) {
										*pOutHit = pBox;
										*pOutType = 10;
										distance = outDistance;
									}
								}

								pBox = pBox + 1;
							}
						}
						else {
							if (bVar1 == 0xe) {
								edColPRIM_OBJECT* pPrim = LOAD_POINTER_CAST(edColPRIM_OBJECT*, peVar4->field_0x54[0]);
								for (iVar7 = 0; iVar7 < peVar4->count_0x52; iVar7 = iVar7 + 1) {
									pDirection = pRay->pLeadVector;
									pLocation = pRay->pLocation;
									float fVar31 = pLocation->x;
									float fVar32 = pLocation->y;
									float fVar33 = pLocation->z;
									float fVar34 = pLocation->w;
									float fVar35 = pDirection->x;
									float fVar36 = pDirection->y;
									float fVar37 = pDirection->z;
									float fVar38 = pDirection->w;
									fVar15 = (pPrim->worldTransform).aa;
									fVar16 = (pPrim->worldTransform).ab;
									fVar17 = (pPrim->worldTransform).ac;
									fVar18 = (pPrim->worldTransform).ad;
									fVar19 = (pPrim->worldTransform).ba;
									fVar20 = (pPrim->worldTransform).bb;
									fVar21 = (pPrim->worldTransform).bc;
									fVar22 = (pPrim->worldTransform).bd;
									fVar23 = (pPrim->worldTransform).ca;
									fVar24 = (pPrim->worldTransform).cb;
									fVar25 = (pPrim->worldTransform).cc;
									fVar26 = (pPrim->worldTransform).cd;
									fVar27 = (pPrim->worldTransform).da;
									float fVar28 = (pPrim->worldTransform).db;
									float fVar29 = (pPrim->worldTransform).dc;
									float fVar30 = (pPrim->worldTransform).dd;

									local_4a0.x = fVar15 * fVar31 + fVar19 * fVar32 + fVar23 * fVar33 + fVar27 * fVar34;
									local_4a0.y = fVar16 * fVar31 + fVar20 * fVar32 + fVar24 * fVar33 + fVar28 * fVar34;
									local_4a0.z = fVar17 * fVar31 + fVar21 * fVar32 + fVar25 * fVar33 + fVar29 * fVar34;
									local_4a0.w = fVar18 * fVar31 + fVar22 * fVar32 + fVar26 * fVar33 + fVar30 * fVar34;

									edF32VECTOR4 local_4b0;

									local_4b0.x = fVar15 * fVar35 + fVar19 * fVar36 + fVar23 * fVar37 + fVar27 * fVar38;
									local_4b0.y = fVar16 * fVar35 + fVar20 * fVar36 + fVar24 * fVar37 + fVar28 * fVar38;
									local_4b0.z = fVar17 * fVar35 + fVar21 * fVar36 + fVar25 * fVar37 + fVar29 * fVar38;
									local_4b0.w = fVar18 * fVar35 + fVar22 * fVar36 + fVar26 * fVar37 + fVar30 * fVar38;

									edColPRIM_RAY_UNIT_SPHERE_UNIT_IN local_28;
									local_28.field_0x4 = &local_4b0;
									local_28.field_0x0 = &local_4a0;

									edColINFO_OUT eStack1168;
									edColIntersectRayUnitSphereUnit(&eStack1168, &local_28);
									if (eStack1168.result != 0) {
										local_4c0.x = (pPrim->localToWorld).aa * eStack1168.intersectionPoint.x +
											(pPrim->localToWorld).ba * eStack1168.intersectionPoint.y +
											(pPrim->localToWorld).ca * eStack1168.intersectionPoint.z +
											(pPrim->localToWorld).da * eStack1168.intersectionPoint.w;
										local_4c0.y = (pPrim->localToWorld).ab * eStack1168.intersectionPoint.x +
											(pPrim->localToWorld).bb * eStack1168.intersectionPoint.y +
											(pPrim->localToWorld).cb * eStack1168.intersectionPoint.z +
											(pPrim->localToWorld).db * eStack1168.intersectionPoint.w;
										local_4c0.z = (pPrim->localToWorld).ac * eStack1168.intersectionPoint.x +
											(pPrim->localToWorld).bc * eStack1168.intersectionPoint.y +
											(pPrim->localToWorld).cc * eStack1168.intersectionPoint.z +
											(pPrim->localToWorld).dc * eStack1168.intersectionPoint.w;
										local_4c0.w = (pPrim->localToWorld).ad * eStack1168.intersectionPoint.x +
											(pPrim->localToWorld).bd * eStack1168.intersectionPoint.y +
											(pPrim->localToWorld).cd * eStack1168.intersectionPoint.z +
											(pPrim->localToWorld).dd * eStack1168.intersectionPoint.w;

										pDirection = pRay->pLocation;
										local_4d0 = local_4c0 - *pDirection;

										outDistance = edF32Vector4GetDistHard(&local_4d0);
										if ((outDistance < distance) || ((distance < 0.0f && (outDistance <= pRay->lengthA)))) {
											*pOutHit = pPrim;
											*pOutType = COL_TYPE_SPHERE;
											distance = outDistance;
										}
									}

									pPrim = pPrim + 1;
								}
							}
							else {
								if (bVar1 == COL_TYPE_SPHERE) {
									edColPRIM_SPHERE* pSphere = LOAD_POINTER_CAST(edColPRIM_SPHERE*, peVar4->field_0x54[0]);
									for (iVar7 = 0; iVar7 < peVar4->count_0x52; iVar7 = iVar7 + 1) {
										pDirection = pRay->pLeadVector;
										pLocation = pRay->pLocation;

										float fVar29 = pLocation->x;
										float fVar30 = pLocation->y;
										float fVar31 = pLocation->z;
										float fVar32 = pLocation->w;
										float fVar33 = pDirection->x;
										float fVar34 = pDirection->y;
										float fVar35 = pDirection->z;
										float fVar36 = pDirection->w;

										fVar13 = (pSphere->worldTransform).aa;
										fVar14 = (pSphere->worldTransform).ab;
										fVar15 = (pSphere->worldTransform).ac;
										fVar16 = (pSphere->worldTransform).ad;
										fVar17 = (pSphere->worldTransform).ba;
										fVar18 = (pSphere->worldTransform).bb;
										fVar19 = (pSphere->worldTransform).bc;
										fVar20 = (pSphere->worldTransform).bd;
										fVar21 = (pSphere->worldTransform).ca;
										fVar22 = (pSphere->worldTransform).cb;
										fVar23 = (pSphere->worldTransform).cc;
										fVar24 = (pSphere->worldTransform).cd;
										fVar25 = (pSphere->worldTransform).da;
										fVar26 = (pSphere->worldTransform).db;
										fVar27 = (pSphere->worldTransform).dc;
										float fVar28 = (pSphere->worldTransform).dd;

										local_400.x = fVar13 * fVar29 + fVar17 * fVar30 + fVar21 * fVar31 + fVar25 * fVar32;
										local_400.y = fVar14 * fVar29 + fVar18 * fVar30 + fVar22 * fVar31 + fVar26 * fVar32;
										local_400.z = fVar15 * fVar29 + fVar19 * fVar30 + fVar23 * fVar31 + fVar27 * fVar32;
										local_400.w = fVar16 * fVar29 + fVar20 * fVar30 + fVar24 * fVar31 + fVar28 * fVar32;
										local_410.x = fVar13 * fVar33 + fVar17 * fVar34 + fVar21 * fVar35 + fVar25 * fVar36;
										local_410.y = fVar14 * fVar33 + fVar18 * fVar34 + fVar22 * fVar35 + fVar26 * fVar36;
										local_410.z = fVar15 * fVar33 + fVar19 * fVar34 + fVar23 * fVar35 + fVar27 * fVar36;
										local_410.w = fVar16 * fVar33 + fVar20 * fVar34 + fVar24 * fVar35 + fVar28 * fVar36;

										edColPRIM_RAY_UNIT_SPHERE_UNIT_IN local_20;
										edColINFO_OUT eStack1008;

										local_20.field_0x4 = &local_410;
										local_20.field_0x0 = &local_400;
										edColIntersectRayUnitSphereUnit(&eStack1008, &local_20);

										if (eStack1008.result != 0) {
											local_420.x = (pSphere->localToWorld).aa * eStack1008.intersectionPoint.x +
												(pSphere->localToWorld).ba * eStack1008.intersectionPoint.y +
												(pSphere->localToWorld).ca * eStack1008.intersectionPoint.z +
												(pSphere->localToWorld).da * eStack1008.intersectionPoint.w;
											local_420.y = (pSphere->localToWorld).ab * eStack1008.intersectionPoint.x +
												(pSphere->localToWorld).bb * eStack1008.intersectionPoint.y +
												(pSphere->localToWorld).cb * eStack1008.intersectionPoint.z +
												(pSphere->localToWorld).db * eStack1008.intersectionPoint.w;
											local_420.z = (pSphere->localToWorld).ac * eStack1008.intersectionPoint.x +
												(pSphere->localToWorld).bc * eStack1008.intersectionPoint.y +
												(pSphere->localToWorld).cc * eStack1008.intersectionPoint.z +
												(pSphere->localToWorld).dc * eStack1008.intersectionPoint.w;
											local_420.w = (pSphere->localToWorld).ad * eStack1008.intersectionPoint.x +
												(pSphere->localToWorld).bd * eStack1008.intersectionPoint.y +
												(pSphere->localToWorld).cd * eStack1008.intersectionPoint.z +
												(pSphere->localToWorld).dd * eStack1008.intersectionPoint.w;

											pDirection = pRay->pLocation;
											local_430 = local_420 - *pDirection;
											outDistance = edF32Vector4GetDistHard(&local_430);

											if ((outDistance < distance) || ((distance < 0.0f && (outDistance <= pRay->lengthA)))) {
												*pOutHit = (edObbTREE_DYN*)pSphere;
												*pOutType = COL_TYPE_SPHERE;
												distance = outDistance;
											}
										}

										pSphere = pSphere + 1;
									}
								}
								else {
									if (bVar1 == COL_TYPE_QUAD) {
										edF32QUAD4* pQuad = LOAD_POINTER_CAST(edF32QUAD4*, peVar4->field_0x54[0]);

										for (iVar7 = 0; iVar7 < peVar4->count_0x52; iVar7 = iVar7 + 1) {
											edF32TRIANGLE4_Stack triangle;

											for (iVar10 = 0; iVar10 < 2; iVar10 = iVar10 + 1) {
												if (iVar10 == 0) {
													triangle.p1 = LOAD_POINTER_CAST(edF32VECTOR4*, pQuad->p1);
													triangle.p2 = LOAD_POINTER_CAST(edF32VECTOR4*, pQuad->p2);
													triangle.p3 = LOAD_POINTER_CAST(edF32VECTOR4*, pQuad->p3);
												}
												else {
													triangle.p1 = LOAD_POINTER_CAST(edF32VECTOR4*, pQuad->p1);
													triangle.p2 = LOAD_POINTER_CAST(edF32VECTOR4*, pQuad->p3);
													triangle.p3 = LOAD_POINTER_CAST(edF32VECTOR4*, pQuad->p4);
												}

												triangle.flags = pQuad->flags;
												local_10.pTriangle = &triangle;
												local_10.pRayOrigin = pRay->pLocation;
												local_10.pRayDirection = pRay->pLeadVector;

												edColIntersectRayTriangle4(&outDistance, &local_10);

												if ((((outDistance != -8888.0f) && (outDistance != -9999.0f)) && (0.0f <= outDistance)) &&
													((outDistance < distance || ((distance < 0.0f && (outDistance <= pRay->lengthA)))))) {
													*pOutHit = pQuad;
													*pOutType = COL_TYPE_QUAD;
													distance = outDistance;
												}
											}

											pQuad = pQuad + 1;
										}
									}
									else {
										if (bVar1 == 5) {
											IMPLEMENTATION_GUARD(
												peVar9 = peVar4->field_0x54;
											for (iVar7 = 0; iVar7 < (int)(uint)peVar4->count_0x52; iVar7 = iVar7 + 1) {
												local_10.pTriangle = (edF32TRIANGLE4*)(local_390 + 0x10);
												local_390._16_4_ =
													(float)(*(int*)(pRay->field_0xc + 0x28) +
														(uint) * (ushort*)&(peVar9->bbox).transform.aa * 0x10);
												local_37c = (float)(*(int*)(pRay->field_0xc + 0x28) +
													(uint) * (ushort*)((int)&(peVar9->bbox).transform.aa + 2) * 0x10);
												local_378 = (float)(*(int*)(pRay->field_0xc + 0x28) +
													(uint) * (ushort*)&(peVar9->bbox).transform.ab * 0x10);
												local_10.pRayOrigin = pRay->pLocation;
												local_10.pRayDirection = pRay->pLeadVector;
												edColIntersectRayTriangle4((float*)&outDistance, &local_10);
												if ((((outDistance != -8888.0) && (outDistance != -9999.0)) && (0.0 <= outDistance)) &&
													((outDistance < distance ||
														((distance < 0.0 && (outDistance <= pRay->lengthA)))))) {
													*pOutHit = peVar9;
													*pOutType = 5;
													distance = outDistance;
												}
												peVar9 = (edObbTREE_DYN*)&(peVar9->bbox).transform.bc;
											})
										}
										else {
											if (bVar1 == 4) {
												edF32TRIANGLE4* pTriangle = LOAD_POINTER_CAST(edF32TRIANGLE4*, peVar4->field_0x54[0]);
												for (iVar7 = 0; iVar7 < peVar4->count_0x52; iVar7 = iVar7 + 1) {
													local_10.pRayOrigin = pRay->pLocation;
													local_10.pRayDirection = pRay->pLeadVector;

#ifdef PLATFORM_WIN
													edF32TRIANGLE4_Stack stackTriangle = pTriangle;
													local_10.pTriangle = &stackTriangle;
#else
													local_10.pTriangle = pTriangle;
#endif

													edColIntersectRayTriangle4(&outDistance, &local_10);

													if ((((outDistance != -8888.0f) && (outDistance != -9999.0f)) && (0.0f <= outDistance))
														&& ((outDistance < distance || ((distance < 0.0f && (outDistance <= pRay->lengthA)))))) {
														*pOutHit = pTriangle;
														*pOutType = 4;
														distance = outDistance;
													}

													pTriangle = pTriangle + 1;
												}
											}
										}
									}
								}
							}
						}
					}
					iVar11 = iVar11 + 1;
				}
			}

			uVar8 = uVar8 != 0 ^ 1;
			bVar5 = uVar8 != 0;
		} while (local_18[uVar8] != 0);
	}

	return distance;
}

ulong FUN_00253340(edF32TRIANGLE4* param_1, edObbBOX* param_2)
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
	float fVar16;
	float fVar17;
	float fVar18;
	float fVar19;
	float fVar20;
	float fVar21;
	float fVar22;
	float fVar23;
	float fVar24;
	float fVar25;
	float fVar26;
	float fVar27;
	float fVar28;
	float fVar29;
	float fVar30;

	peVar1 = LOAD_POINTER_CAST(edF32VECTOR4*, param_1->p1);
	peVar2 = LOAD_POINTER_CAST(edF32VECTOR4*, param_1->p2);
	peVar3 = LOAD_POINTER_CAST(edF32VECTOR4*, param_1->p3);
	fVar7 = (param_2->transform).ac;
	fVar8 = (param_2->transform).ba;
	fVar9 = (param_2->transform).bb;
	fVar10 = (param_2->transform).bc;
	fVar11 = (param_2->transform).ca;
	fVar12 = (param_2->transform).cb;
	fVar13 = (param_2->transform).cc;
	fVar16 = (param_2->transform).da;
	fVar17 = (param_2->transform).db;
	fVar18 = (param_2->transform).dc;
	fVar19 = peVar1->x;
	fVar20 = peVar1->y;
	fVar21 = peVar1->z;
	fVar22 = peVar1->w;
	fVar23 = peVar2->x;
	fVar24 = peVar2->y;
	fVar25 = peVar2->z;
	fVar26 = peVar2->w;
	fVar27 = peVar3->x;
	fVar28 = peVar3->y;
	fVar29 = peVar3->z;
	fVar30 = peVar3->w;
	fVar15 = param_2->transform.ab;
	fVar14 = param_2->transform.aa;
	fVar4 = 0.0f - (fVar14 * fVar16 + fVar15 * fVar17 + fVar7 * fVar18);
	fVar5 = 0.0f - (fVar8 * fVar16 + fVar9 * fVar17 + fVar10 * fVar18);
	fVar6 = 0.0f - (fVar11 * fVar16 + fVar12 * fVar17 + fVar13 * fVar18);
	fVar16 = fVar14 * fVar19 + fVar15 * fVar20 + fVar7 * fVar21 + fVar4 * fVar22;
	fVar18 = fVar8 * fVar19 + fVar9 * fVar20 + fVar10 * fVar21 + fVar5 * fVar22;
	fVar20 = fVar11 * fVar19 + fVar12 * fVar20 + fVar13 * fVar21 + fVar6 * fVar22;
	fVar17 = fVar14 * fVar23 + fVar15 * fVar24 + fVar7 * fVar25 + fVar4 * fVar26;
	fVar19 = fVar8 * fVar23 + fVar9 * fVar24 + fVar10 * fVar25 + fVar5 * fVar26;
	fVar21 = fVar11 * fVar23 + fVar12 * fVar24 + fVar13 * fVar25 + fVar6 * fVar26;
	fVar4 = fVar14 * fVar27 + fVar15 * fVar28 + fVar7 * fVar29 + fVar4 * fVar30;
	fVar5 = fVar8 * fVar27 + fVar9 * fVar28 + fVar10 * fVar29 + fVar5 * fVar30;
	fVar6 = fVar11 * fVar27 + fVar12 * fVar28 + fVar13 * fVar29 + fVar6 * fVar30;
	fVar14 = param_2->width;
	fVar7 = param_2->height;
	fVar15 = param_2->depth;
	fVar9 = -fVar14;
	fVar8 = -fVar7;
	fVar10 = -fVar15;
	return (uint)(((uint)(fVar14 < fVar16) + (((uint)(fVar16 < fVar9) << 0x17) >> 0x16) +
		(((uint)(fVar7 < fVar18) << 0x17) >> 0x15) +
		(((uint)(fVar18 < fVar8) << 0x17) >> 0x14) +
		(((uint)(fVar15 < fVar20) << 0x17) >> 0x13) +
		(((uint)(fVar20 < fVar10) << 0x17) >> 0x12) &
		(uint)(fVar14 < fVar17) + (((uint)(fVar17 < fVar9) << 0x17) >> 0x16) +
		(((uint)(fVar7 < fVar19) << 0x17) >> 0x15) +
		(((uint)(fVar19 < fVar8) << 0x17) >> 0x14) +
		(((uint)(fVar15 < fVar21) << 0x17) >> 0x13) +
		(((uint)(fVar21 < fVar10) << 0x17) >> 0x12) &
		(uint)(fVar14 < fVar4) + (((uint)(fVar4 < fVar9) << 0x17) >> 0x16) +
		(((uint)(fVar7 < fVar5) << 0x17) >> 0x15) +
		(((uint)(fVar5 < fVar8) << 0x17) >> 0x14) +
		(((uint)(fVar15 < fVar6) << 0x17) >> 0x13) +
		(((uint)(fVar6 < fVar10) << 0x17) >> 0x12)) == 0);
}

int INT_ARRAY_00448918[2] = { 1, 0 };
int gObbCurrentIndex = 0;

edF32VECTOR4 __cube_corners[4] = {
	{ 0.5f, 0.5f, 0.5f, 1.0f },
	{ 0.5f, 0.5f, -0.5f, 1.0f },
	{ 0.5f, -0.5f, 0.5f, 1.0f },
	{ 0.5f, -0.5f, -0.5f, 1.0f },
};

int INT_ARRAY_0041d910[24] = {
	0, 2, 3, 1,
	4, 6, 2, 0,
	5, 7, 6, 4,
	1, 3, 7, 5,
	4, 0, 1, 5,
	3, 2, 6, 7 };

int edObbIntersectObbtreeTrisBox(edCol_INFO_OBB_TRIS_BOX* pColInfoObbTrisBox, edObbTREE_DYN* pObbTree, edObbBOX* pObbBox, int* param_4, uint param_5)
{
	byte bVar1;
	int iVar2;
	int iVar3;
	edObbTREE_DYN* peVar4;
	edF32VECTOR4* peVar5;
	bool bVar6;
	byte bVar7;
	int* piVar8;
	float* pfVar9;
	uint uVar10;
	ulong uVar11;
	edF32TRIANGLE4* pSVar12;
	int iVar13;
	edObbTREE_DYN* peVar14;
	edF32TRIANGLE4* pSVar15;
	int iVar16;
	int iVar17;
	int local_3e0;
	edF32VECTOR4* local_360;
	edObbTREE_DYN* local_330[202];
	int local_8[2];

	uVar10 = 0;
	local_8[0] = INT_ARRAY_00448918[0];
	local_8[1] = INT_ARRAY_00448918[1];
	iVar17 = pColInfoObbTrisBox->field_0x8;
	pSVar15 = pColInfoObbTrisBox->pTris;
	iVar2 = pColInfoObbTrisBox->field_0xc;
	iVar3 = pColInfoObbTrisBox->field_0x10;
	local_360 = pColInfoObbTrisBox->pVector;
	*param_4 = 0;
	pSVar15 = pSVar15 + iVar17;
	gObbCurrentIndex = 0;
	bVar6 = edObbIntersect(&pObbTree->bbox, pObbBox);
	if (bVar6 != false) {
		bVar6 = false;
		local_330[0] = pObbTree;
		do {
			bVar7 = bVar6 ^ 1;
			iVar17 = 0;
			piVar8 = local_8 + bVar7;
			*piVar8 = 0;
			while (iVar17 < local_8[uVar10]) {
				peVar4 = local_330[uVar10 * 100 + iVar17];
				bVar1 = peVar4->type;
				if (bVar1 == COL_TYPE_TREE) {
					for (iVar13 = 0; iVar13 < peVar4->count_0x52; iVar13 = iVar13 + 1) {
						edObbTREE_DYN* pLinkedTree = (edObbTREE_DYN*)LOAD_POINTER(peVar4->field_0x54[iVar13]);
						bVar6 = edObbIntersect(&pLinkedTree->bbox, pObbBox);
						if ((bVar6 != false) && (*piVar8 < 100)) {
							local_330[(static_cast<uint>(bVar7) * 4 + static_cast<uint>(bVar7)) * 0x14 + *piVar8] = pLinkedTree;
							*piVar8 = *piVar8 + 1;
						}
					}

					iVar17 = iVar17 + 1;
				}
				else {
					if (bVar1 == COL_TYPE_BOX) {
						edColPRIM_BOX* pBox = LOAD_POINTER_CAST(edColPRIM_BOX*, peVar4->field_0x54[0]);
						if ((pBox->flags_0x80 & param_5 & 0xffff) == 0) {
							for (local_3e0 = 0; local_3e0 < peVar4->count_0x52; local_3e0 = local_3e0 + 1) {
								for (iVar13 = 0; peVar5 = local_360, iVar13 < 6; iVar13 = iVar13 + 1) {
									for (iVar16 = 0; iVar16 < 4; iVar16 = iVar16 + 1) {
										if (iVar3 <= *param_4) {
											return gObbCurrentIndex;
										}

										edF32Matrix4MulF32Vector4Hard(local_360, &pBox->localToWorld, __cube_corners + INT_ARRAY_0041d910[iVar13 * 4 + iVar16]);
										local_360 = local_360 + 1;
										*param_4 = *param_4 + 1;
									}

									pSVar15[gObbCurrentIndex].p1 = STORE_POINTER(peVar5 + 0);
									pSVar15[gObbCurrentIndex].p2 = STORE_POINTER(peVar5 + 1);
									pSVar15[gObbCurrentIndex].p3 = STORE_POINTER(peVar5 + 2);
									pSVar15[gObbCurrentIndex].flags = pBox->flags_0x80;
									uVar11 = FUN_00253340(pSVar15 + gObbCurrentIndex, pObbBox);
									if ((uVar11 != 0) &&
										(gObbCurrentIndex = gObbCurrentIndex + 1, iVar2 <= gObbCurrentIndex)) {
										return gObbCurrentIndex;
									}

									pSVar15[gObbCurrentIndex].p1 = STORE_POINTER(peVar5 + 0);
									pSVar15[gObbCurrentIndex].p2 = STORE_POINTER(peVar5 + 2);
									pSVar15[gObbCurrentIndex].p3 = STORE_POINTER(peVar5 + 3);
									pSVar15[gObbCurrentIndex].flags = pBox->flags_0x80;
									uVar11 = FUN_00253340(pSVar15 + gObbCurrentIndex, pObbBox);
									if ((uVar11 != 0) &&
										(gObbCurrentIndex = gObbCurrentIndex + 1, iVar2 <= gObbCurrentIndex)) {
										return gObbCurrentIndex;
									}
								}

								pBox = pBox + 1;
							}
						}
					}
					else {
						if (bVar1 == COL_TYPE_QUAD) {
							edF32QUAD4* pQuad = LOAD_POINTER_CAST(edF32QUAD4*, peVar4->field_0x54[0]);
							for (iVar13 = 0; iVar13 < peVar4->count_0x52; iVar13 = iVar13 + 1) {
								pSVar15[gObbCurrentIndex].p1 = pQuad->p1;
								pSVar15[gObbCurrentIndex].p2 = pQuad->p2;
								pSVar15[gObbCurrentIndex].p3 = pQuad->p3;
								pSVar15[gObbCurrentIndex].flags = pQuad->flags;

								uVar11 = FUN_00253340(pSVar15 + gObbCurrentIndex, pObbBox);
								if ((uVar11 != 0) &&
									(gObbCurrentIndex = gObbCurrentIndex + 1, iVar2 <= gObbCurrentIndex)) {
									return gObbCurrentIndex;
								}
		
								pSVar15[gObbCurrentIndex].p1 = pQuad->p1;
								pSVar15[gObbCurrentIndex].p2 = pQuad->p3;
								pSVar15[gObbCurrentIndex].p3 = pQuad->p4;
								pSVar15[gObbCurrentIndex].flags = pQuad->flags;

								uVar11 = FUN_00253340(pSVar15 + gObbCurrentIndex, pObbBox);
								if ((uVar11 != 0) &&
									(gObbCurrentIndex = gObbCurrentIndex + 1, iVar2 <= gObbCurrentIndex)) {
									return gObbCurrentIndex;
								}

								pQuad = pQuad + 1;
							}
						}
						else {
							if (bVar1 == COL_TYPE_TRIANGLE) {
								edF32TRIANGLE4* pTri = LOAD_POINTER_CAST(edF32TRIANGLE4*, peVar4->field_0x54[0]);
								for (iVar13 = 0; iVar13 < peVar4->count_0x52; iVar13 = iVar13 + 1) {
									pSVar15[gObbCurrentIndex] = *pTri;
									uVar11 = FUN_00253340(pSVar15 + gObbCurrentIndex, pObbBox);
									if ((uVar11 != 0) &&
										(gObbCurrentIndex = gObbCurrentIndex + 1, iVar2 <= gObbCurrentIndex)) {
										return gObbCurrentIndex;
									}

									pTri = pTri + 1;
								}
							}
						}
					}

					iVar17 = iVar17 + 1;
				}
			}

			uVar10 = static_cast<uint>(bVar7);
			bVar6 = uVar10 != 0;
		} while (local_8[bVar7] != 0);
	}

	return gObbCurrentIndex;
}