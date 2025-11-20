#include "ed3D/ed3DG3D.h"
#include "ed3D.h"

int gCurTime = 0;
int gCurFrame = 0;
int gStepTime = 0;

void ed3DResetTime(void)
{
	gCurTime = 0;
	return;
}

void ed3DSetDeltaTime(int newTime)
{
	gStepTime = newTime;
	gCurTime = gCurTime + newTime;
	return;
}

int ed3DG2DGetG2DNbMaterials(ed_hash_code* pHashCode)
{
	ed_Chunck* pChunck = reinterpret_cast<ed_Chunck*>(pHashCode - 1);
	return ed3DG2DGetG2DNbMaterials(pChunck);
}

int ed3DG2DGetG2DNbMaterials(ed_g2d_manager* pManager)
{
	// Get the hash code out
	ed_hash_code* pHashCode = reinterpret_cast<ed_hash_code*>(pManager->pMATA_HASH + 1);
	return ed3DG2DGetG2DNbMaterials(pHashCode);
}

ed_hash_code* ed3DHierarchyGetMaterialBank(ed_3d_hierarchy* pHier)
{
	ed_hash_code* peVar1;

	if (pHier->pTextureInfo == (ed_Chunck*)0x0) {
		peVar1 = (ed_hash_code*)0x0;
	}
	else {
		peVar1 = (ed_hash_code*)(pHier->pTextureInfo + 1);
	}

	return peVar1;
}

void ed3DManageAnim(ed_g3d_Anim_def* pAnimDef)
{
	uint* puVar1;
	uint* puVar2;
	uint uVar3;
	ulong uVar4;
	uint uVar5;
	uint uVar6;
	uint uVar7;
	uint* puVar8;
	float fVar9;
	float fVar10;

	uVar4 = pAnimDef->field_0x4;
	if (gCurFrame == pAnimDef->field_0x4 >> 0x1a) {
		return;
	}

	if (((uVar4 & 4) == 0) && ((uVar4 & 8) == 0)) {
		return;
	}

	pAnimDef->field_0x4 = (uint)((uVar4 << 0x26) >> 0x26) | gCurFrame << 0x1a;

	if ((pAnimDef->field_0x4 & 0x20) == 0) {
		if (pAnimDef->field_0x14 == 0) {
			pAnimDef->field_0x14 = gCurTime;
		}
		pAnimDef->field_0x18 = gCurTime;
	}
	else {
		pAnimDef->field_0x18 = pAnimDef->field_0x18 + pAnimDef->field_0xc;
	}

	puVar1 = &pAnimDef->field_0x20;
	uVar6 = pAnimDef->field_0x4;
	uVar3 = pAnimDef->field_0x18 - pAnimDef->field_0x14;
	uVar5 = puVar1[(pAnimDef->field_0x0 - 1) * 4];

	if ((uVar6 & 8) == 0) {
		if (uVar5 < uVar3) {
			uVar3 = (int)uVar3 % (int)uVar5;
			if (uVar5 == 0) {
				trap(7);
			}
			if ((uVar6 & 2) != 0) {
				pAnimDef->field_0x4 = pAnimDef->field_0x4 | 8;
				pAnimDef->field_0x4 = pAnimDef->field_0x4 & 0xfffffffb;
				pAnimDef->field_0x14 = gCurTime - uVar3;
				goto LAB_0029edbc;
			}
			if ((uVar6 & 1) == 0) {
				uVar3 = uVar5;
			}
		}
	LAB_0029ef30:
		uVar6 = *puVar1;
		for (uVar5 = 0; (uVar6 < uVar3 && (uVar5 != pAnimDef->field_0x0)); uVar5 = uVar5 + 1) {
			puVar1 = puVar1 + 4;
			uVar6 = *puVar1;
		}
		pAnimDef->field_0x8 = uVar5;
		if (uVar5 == pAnimDef->field_0x0) {
			puVar2 = puVar1 + -4;
			puVar1 = puVar1 + -8;
		}
		else {
			if (uVar5 == 0) {
				puVar2 = puVar1 + 4;
			}
			else {
				puVar2 = puVar1;
				puVar1 = puVar1 + -4;
			}
		}
		uVar3 = uVar3 - *puVar1;
		if ((int)uVar3 < 0) {
			fVar9 = (float)(uVar3 >> 1 | uVar3 & 1);
			fVar9 = fVar9 + fVar9;
		}
		else {
			fVar9 = (float)uVar3;
		}
		uVar3 = *puVar2 - *puVar1;
		if ((int)uVar3 < 0) {
			fVar10 = (float)(uVar3 >> 1 | uVar3 & 1);
			fVar10 = fVar10 + fVar10;
		}
		else {
			fVar10 = (float)uVar3;
		}
		pAnimDef->field_0x10 = (float)puVar1[1] + (fVar9 / fVar10) * ((float)puVar2[1] - (float)puVar1[1]);
		if ((pAnimDef->field_0x8 == pAnimDef->field_0x0) &&
			((uVar3 = pAnimDef->field_0x4, (uVar3 & 1) != 0 || ((uVar3 & 2) != 0)))) {
			if ((uVar3 & 2) == 0) {
				pAnimDef->field_0x8 = 0;
			}
			else {
				pAnimDef->field_0x4 = pAnimDef->field_0x4 | 8;
			}
			pAnimDef->field_0x14 = 0;
			pAnimDef->field_0x18 = 0;
		}
	}
	else {
		if (uVar5 < uVar3) {
			uVar3 = (int)uVar3 % (int)uVar5;
			if (uVar5 == 0) {
				trap(7);
			}
			if ((uVar6 & 2) != 0) {
				pAnimDef->field_0x4 = pAnimDef->field_0x4 & 0xfffffff7;
				pAnimDef->field_0x4 = pAnimDef->field_0x4 | 4;
				pAnimDef->field_0x14 = gCurTime - uVar3;
				goto LAB_0029ef30;
			}
			if ((uVar6 & 1) == 0) {
				uVar3 = uVar5;
			}
		}
	LAB_0029edbc:
		uVar6 = pAnimDef->field_0x0;
		puVar2 = puVar1 + (uVar6 - 1) * 4;
		for (uVar7 = 0; (uVar5 - *puVar2 < uVar3 && (uVar7 != uVar6)); uVar7 = uVar7 + 1) {
			puVar2 = puVar2 + -4;
		}
		pAnimDef->field_0x8 = uVar6 - uVar7;
		puVar8 = puVar2 + 4;
		if (pAnimDef->field_0x8 == 0) {
			puVar8 = &pAnimDef->field_0x30;
			puVar2 = puVar1;
		}
		else {
			uVar6 = pAnimDef->field_0x0 - 1;
			if (uVar6 <= pAnimDef->field_0x8) {
				puVar8 = puVar1 + uVar6 * 4;
				puVar2 = puVar8 + -4;
			}
		}
		uVar3 = uVar3 - (uVar5 - *puVar8);
		if ((int)uVar3 < 0) {
			fVar9 = (float)(uVar3 >> 1 | uVar3 & 1);
			fVar9 = fVar9 + fVar9;
		}
		else {
			fVar9 = (float)uVar3;
		}
		uVar3 = *puVar8 - *puVar2;
		if ((int)uVar3 < 0) {
			fVar10 = (float)(uVar3 >> 1 | uVar3 & 1);
			fVar10 = fVar10 + fVar10;
		}
		else {
			fVar10 = (float)uVar3;
		}
		pAnimDef->field_0x10 = (float)puVar8[1] - (fVar9 / fVar10) * ((float)puVar8[1] - (float)puVar2[1]);
	}

	if ((pAnimDef->field_0x4 & 0x10) != 0) {
		fVar9 = pAnimDef->field_0x10;
		if (fVar9 < 2.147484e+09f) {
			uVar3 = (uint)fVar9;
		}
		else {
			uVar3 = (int)(fVar9 - 2.147484e+09f) | 0x80000000;
		}
		if ((int)uVar3 < 0) {
			fVar9 = (float)(uVar3 >> 1 | uVar3 & 1);
			fVar9 = fVar9 + fVar9;
		}
		else {
			fVar9 = (float)uVar3;
		}

		pAnimDef->field_0x10 = fVar9;
	}
	return;
}

ed_hash_code* ed3DG2DGetHashCode(ed_g2d_manager* pManager, ed_g2d_material* pMaterial)
{
	ed_Chunck* pMATA_HASH;
	ed_Chunck* pcVar2;
	uint nbMaterials;

	pMATA_HASH = pManager->pMATA_HASH;

	ed_hash_code* pHashCode = reinterpret_cast<ed_hash_code*>(pManager->pMATA_HASH + 1);

	pcVar2 = LOAD_POINTER_CAST(ed_Chunck*, pHashCode->pData);
	nbMaterials = pMATA_HASH->nextChunckOffset - 0x10U >> 4;

	while (true) {
		if ((nbMaterials == 0) || (pMaterial == (ed_g2d_material*)(pcVar2 + 1))) break;
		nbMaterials = nbMaterials - 1;
		pcVar2 = LOAD_POINTER_CAST(ed_Chunck*, pHashCode->pData);
		pHashCode = pHashCode + 1;
	}

	return pHashCode;
}

int ed3DG2DGetG2DNbMaterials(ed_Chunck* pChunck)
{
	int materialCount;
	int adjustedChunkSize;

	if ((pChunck->hash == HASH_CODE_HASH) || (materialCount = -1, pChunck->hash == HASH_CODE_MBNK /*MBNK*/)) {
		adjustedChunkSize = pChunck->size + -0x10;
		materialCount = adjustedChunkSize >> 4;
		if (adjustedChunkSize < 0) {
			materialCount = pChunck->size + -1 >> 4;
		}
	}

	return materialCount;
}

ed_g3d_hierarchy* ed3DG3DHierarchyGetFromIndex(ed_g3d_manager* pMeshInfo, int count)
{
	ed_hash_code* pMVar1;

	pMVar1 = (ed_hash_code*)(pMeshInfo->HALL + 2);

	for (; count != 0; count = count + -1) {
		pMVar1 = pMVar1 + 1;
	}

	char* pLoaded = (char*)LOAD_POINTER(pMVar1->pData);

	return (ed_g3d_hierarchy*)(pLoaded + 0x10);
}

void ed3DG3DHierarchyNodeSetAndClrStripFlag(ed_g3d_hierarchy* pHier, ushort flag, byte param_3, uint param_4)
{
	short sVar1;
	ed3DLod* peVar2;
	float* pfVar3;
	uint uVar4;
	ed_hash_code* ppeVar5;
	uint uVar6;
	ed3DLod* pLod;

	if (((pHier->flags_0x9e & 2) != 0) || ((pHier->flags_0x9e & 4) != 0)) {
		if (param_3 == 0) {
			pHier->flags_0x9e = pHier->flags_0x9e & 0xfdff;
		}
		else {
			pHier->flags_0x9e = pHier->flags_0x9e | 0x200;
		}
	}
	uVar4 = (int)(char)param_3 & 0xff;
	uVar6 = 0;
	do {
		while (true) {
			if (pHier->lodCount <= uVar6) {
				return;
			}

			pLod = &pHier->aLods[uVar6];

			sVar1 = pLod->renderType;
			ppeVar5 = (ed_hash_code*)LOAD_POINTER(pLod->pObj);
			if (((sVar1 != 3) && (sVar1 != 2)) && (sVar1 != 1)) break;
		LAB_0029f5e0:
			uVar6 = uVar6 + 1;
		}
		if (sVar1 == 0) {
			if (param_4 == 4) {
				ed3DObjectSetStripShadowReceive(ppeVar5, flag, uVar4);
			}
			else {
				if (param_4 == 2) {
					if ((pHier->flags_0x9e & 0x100) == 0) {
						ed3DObjectSetStripShadowCast(ppeVar5, flag, uVar4);
					}
					else {
						peVar2 = ed3DHierarcGetLOD(pHier, pHier->lodCount - 1);
						if (peVar2 != (ed3DLod*)0x0) {
							ed3DObjectSetStripShadowCast((ed_hash_code*)LOAD_POINTER(peVar2->pObj), flag, uVar4);
						}
					}
				}
				else {
					if (param_4 == 1) {
						IMPLEMENTATION_GUARD(
							ed3DObjectSetStripProperty(ppeVar5, flag, uVar4);)
					}
				}
			}
			goto LAB_0029f5e0;
		}
		uVar6 = uVar6 + 1;
	} while (true);
}

void ed3DG3DHierarchySetStripShadowCastFlag(ed_g3d_hierarchy* pHier, ushort flag)
{
	ed_g3d_hierarchy* peVar1;
	ed3DLod* peVar2;
	ed_hash_code* peVar3;

	pHier->flags_0x9e = pHier->flags_0x9e | 0x200;
	if (pHier != (ed_g3d_hierarchy*)0x0) {
		if (((pHier->flags_0x9e & 2) != 0) || ((pHier->flags_0x9e & 4) != 0)) {
			pHier->flags_0x9e = pHier->flags_0x9e | 0x200;
		}

		// Walk back to chunk.
		ed_Chunck* pChunck = (ed_Chunck*)(((char*)pHier) - sizeof(ed_Chunck));

		if (pChunck->hash == 0x52454948) {
			while (pChunck->hash == 0x52454948) {
				ed_g3d_hierarchy* pNewHier = (ed_g3d_hierarchy*)(pChunck + 1);

				ed_3d_hierarchy_node* pNode = (ed_3d_hierarchy_node*)LOAD_POINTER(pHier->pLinkTransformData);

				if (pNode != (ed_3d_hierarchy_node*)0x0) {
					IMPLEMENTATION_GUARD(
						peVar1 = (ed_g3d_hierarchy*)(peVar2[0x14].pObj + 1);
					while (peVar1 != (ed_g3d_hierarchy*)0x0) {
						if (peVar1 == pHier) {
							ed3DG3DHierarchyNodeSetAndClrStripFlag((ed_g3d_hierarchy*)(peVar2 + 2), flag, 1, 2);
							break;
						}
						if (peVar1->pLinkTransformData == (ed_3d_hierarchy_node*)0x0) {
							peVar1 = (ed_g3d_hierarchy*)0x0;
						}
						else {
							peVar1 = (ed_g3d_hierarchy*)&(peVar1->pLinkTransformData->base).transformA.ba;
						}
					})
				}
				pChunck = edChunckGetNext(pChunck, (char*)0x0);
			}
			ed3DG3DHierarchyNodeSetAndClrStripFlag(pHier, flag, 1, 2);
		}
	}
	return;
}

void ed3DG3DHierarchySetStripShadowReceiveFlag(ed_g3d_hierarchy* pHier, ushort flag)
{
	ed_g3d_hierarchy* peVar1;
	ed_Chunck* pChunk;
	uint curChunkHash;

	pHier->flags_0x9e = pHier->flags_0x9e | 0x200;

	if (pHier != (ed_g3d_hierarchy*)0x0) {
		if (((pHier->flags_0x9e & 2) != 0) || ((pHier->flags_0x9e & 4) != 0)) {
			pHier->flags_0x9e = pHier->flags_0x9e | 0x200;
		}

		// Walk back to the chunk, which is right before the hierarchy.
		pChunk = reinterpret_cast<ed_Chunck*>(reinterpret_cast<char*>(pHier) - sizeof(ed_Chunck));

		if (pChunk->hash == HASH_CODE_HIER) {
			curChunkHash = pChunk->hash;

			while (curChunkHash == HASH_CODE_HIER) {
				if (pHier->pHierarchySetup != 0) {
					IMPLEMENTATION_GUARD(); // This needs fixing, it doesn't look correct.
					ed_Chunck* pNextChunk = (ed_Chunck*)(pHier->aLods);
					peVar1 = (ed_g3d_hierarchy*)(pNextChunk + 1);
					while (peVar1 != (ed_g3d_hierarchy*)0x0) {
						if (peVar1 == pHier) {
							ed3DG3DHierarchyNodeSetAndClrStripFlag((ed_g3d_hierarchy*)(&pChunk->hash + 4), flag, 1, 4);
							break;
						}
						if (peVar1->pLinkTransformData == 0x0) {
							peVar1 = (ed_g3d_hierarchy*)0x0;
						}
						else {
							peVar1 = (ed_g3d_hierarchy*)((ed_Chunck*)LOAD_POINTER(peVar1->pLinkTransformData) + 1);
						}
					}
				}

				pChunk = edChunckGetNext(pChunk, (char*)0x0);
				curChunkHash = pChunk->hash;
			}

			ed3DG3DHierarchyNodeSetAndClrStripFlag(pHier, flag, 1, 4);
		}
	}

	return;
}