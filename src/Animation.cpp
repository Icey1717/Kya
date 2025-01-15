#include "Animation.h"
#include "edList.h"
#include "ed3D.h"
#include "Actor.h"
#include "MathOps.h"
#include "Rendering/CustomShell.h"
#include <math.h>
#include <assert.h>
#include "EventTrack.h"

edAnmStage TheAnimStage;

edAnmStage::edAnmStage()
{
	this->animMode = -5;
	this->field_0x4 = 0;
	this->field_0x8 = -1.0f;
	this->pFrameMatrixData = (edF32MATRIX4*)0x0;
	this->pConstantMatrixData = (edF32MATRIX3*)0x0;
	this->pAnimMatrix = (AnimMatrix*)0x0;
	this->pRelativeTransformMatrixBuffer = (edANM_WRTS*)0x0;
	this->anmSkeleton = (edANM_SKELETON*)0x0;
	this->pKeyData = (edANM_HDR*)0x0;
	this->field_0x24 = 0;
	this->bLoop = 1;
	this->field_0x4c = false;
}

CAnimation::CAnimation()
{
	(this->anmBinMetaAnimator).layerCount = 0;
	(this->anmBinMetaAnimator).aAnimData = (edAnmLayer*)0x0;
	(this->anmBinMetaAnimator).pAnimKeyEntryData = (int*)0x0;
	(this->anmSkeleton).pTag = (edANM_SKELETON*)0x0;
	this->pAnimMatrix = (edF32MATRIX3*)0x0;
	this->field_0x28 = 0;
	return;
}

bool CAnimation::UpdateCurSkeleton(CActor* pActor)
{
	edNODE* pMeshNode;
	BoneData* pAVar2;
	ed_Chunck* pSkeletonChunck;
	int iVar4;
	int iVar5;
	BoneData* pAVar1;

	pMeshNode = pActor->pMeshNode;
	pSkeletonChunck = (ed_Chunck*)0x0;

	if (pMeshNode != (edNODE*)0x0) {
		pSkeletonChunck = ed3DHierarchyNodeGetSkeletonChunck(pMeshNode, false);
	}

	if (pSkeletonChunck == (ed_Chunck*)0x0) {
		if ((this->anmSkeleton).pTag != (edANM_SKELETON*)0x0) {
			(this->anmSkeleton).pTag = (edANM_SKELETON*)0x0;

			for (pAVar1 = this->pBoneData; pAVar1 != (BoneData*)0x0; pAVar1 = pAVar1->pPrev) {
				pAVar1->boneNodeIndex = 0;
			}

			iVar4 = 0;
			if (0 < this->nbDisabledBones) {
				do {
					this->aDisabledBoneData[iVar4].nodeId = 0;
					iVar4 = iVar4 + 1;
				} while (iVar4 < this->nbDisabledBones);
			}
		}
	}
	else {
		if (pSkeletonChunck + 1 != (ed_Chunck*)(this->anmSkeleton).pTag) {
			(this->anmSkeleton).pTag = (edANM_SKELETON*)(pSkeletonChunck + 1);
			for (pAVar2 = this->pBoneData; pAVar2 != (BoneData*)0x0; pAVar2 = pAVar2->pPrev) {
				iVar4 = -1;
				if ((this->anmSkeleton).pTag != (edANM_SKELETON*)0x0) {
					iVar4 = this->anmSkeleton.NodeIndexFromID(pAVar2->boneId);
				}

				if (iVar4 == -1) {
					iVar4 = 0;
				}

				pAVar2->boneNodeIndex = iVar4;
			}

			iVar4 = 0;
			if (0 < this->nbDisabledBones) {
				do {
					iVar5 = -1;
					if ((this->anmSkeleton).pTag != (edANM_SKELETON*)0x0) {
						iVar5 = this->anmSkeleton.NodeIndexFromID(this->aDisabledBoneData[iVar4].boneId);
					}

					if (iVar5 == -1) {
						iVar5 = 0;
					}

					this->aDisabledBoneData[iVar4].nodeId = iVar5;
					iVar4 = iVar4 + 1;
				} while (iVar4 < this->nbDisabledBones);
			}
		}
	}

	return (this->anmSkeleton).pTag != (edANM_SKELETON*)0x0;
}

void CAnimation::Create(CActor* pActor, uint count, edAnmLayer* aAnimLayers, int layerCount)
{
	bool bVar1;
	CAnimationManager* pCVar2;
	int iVar3;
	//CEventTrack* pCVar4;
	edAnmSkeleton* peVar5;
	//AnimationCallback puVar6;
	int layerIndex;
	uint uVar8;

	this->pBoneData = (BoneData*)0x0;
	//peVar5 = &this->pShortData_0xc;
	//iVar7 = 3;
	//do {
	//	peVar5[5].pTag = (edAnmSkeletonTag*)0x0;
	//	peVar5 = peVar5 + -1;
	//	bVar1 = iVar7 != 0;
	//	iVar7 = iVar7 + -1;
	//} while (bVar1);
	this->nbDisabledBones = 0;
	this->count_0x2c = 0;
	this->currentAnimType_0x30 = -1;
	UpdateCurSkeleton(pActor);
	pCVar2 = CScene::ptable.g_AnimManager_00451668;
	this->count_0x2c = count;
	(this->anmBinMetaAnimator).aAnimData = aAnimLayers;
	this->anmBinMetaAnimator.SetLayerProperties(layerCount);
	this->anmBinMetaAnimator.SetLayerResourceArray(-1, pCVar2->pAnimKeyTable);
	this->anmBinMetaAnimator.SetAnimStatesBinary(pCVar2->pAnimKeyEntryData);
	layerIndex = (this->anmBinMetaAnimator).layerCount;
	if (layerIndex != 0) {
		do {
			layerIndex = layerIndex + -1;
			this->anmBinMetaAnimator.aAnimData[layerIndex].Reset();
			this->anmBinMetaAnimator.SetLayerMacroAnimCallback(layerIndex, CAnimationManager::_gLayersCallbacks[layerIndex]);
			this->anmBinMetaAnimator.SetLayerMacroAnimUserParams(layerIndex, pActor);
		} while (layerIndex != 0);
	}

	if (((this->anmBinMetaAnimator).aAnimData)->animPlayState == STATE_ANIM_PLAYING) {
		layerIndex = 0;
		for (uVar8 = this->count_0x2c; uVar8 != 0; uVar8 = uVar8 >> 1) {
			if ((uVar8 & 1) != 0) {
				iVar3 = this->anmBinMetaAnimator.GetAnimEventTrackID(layerIndex);
				if (iVar3 != -1) {
					CEventTrack* pTrack = CScene::ptable.g_TrackManager_004516b4->GetTrack(iVar3);
					pTrack->Reset();
				}
				layerIndex = layerIndex + 1;
			}
		}
	}
	layerIndex = (this->anmBinMetaAnimator).layerCount;
	if (layerIndex != 0) {
		do {
			layerIndex = layerIndex + -1;
			this->anmBinMetaAnimator.aAnimData[layerIndex].Reset();
			this->anmBinMetaAnimator.SetLayerMacroAnimCallback(layerIndex, CAnimationManager::_gLayersCallbacks[layerIndex]);
			this->anmBinMetaAnimator.SetLayerMacroAnimUserParams(layerIndex, pActor);
		} while (layerIndex != 0);
	}
	return;
}

void CAnimation::StopEventTrack(int state)
{
	int trackId;
	int index;
	uint uVar2;

	if ((((this->anmBinMetaAnimator).aAnimData)->animPlayState == STATE_ANIM_PLAYING) && (state == 0)) {
		index = 0;
		for (uVar2 = this->count_0x2c; uVar2 != 0; uVar2 = uVar2 >> 1) {
			if ((uVar2 & 1) != 0) {
				trackId = this->anmBinMetaAnimator.GetAnimEventTrackID(index);
				if (trackId != -1) {
					CEventTrack* pTrack = CScene::ptable.g_TrackManager_004516b4->GetTrack(trackId);
					pTrack->Reset();
				}
				index = index + 1;
			}
		}
	}
	return;
}

void edAnmStage::SetActor(edANM_SKELETON* pSkeleton)
{
	edANM_SKELETON* pTag;

	(this->anmSkeleton).pTag = pSkeleton;
	pTag = (this->anmSkeleton).pTag;
	const uint offset = pTag->boneCount * 0xc + 0x13 & 0xfffffff0;
	this->pConstantMatrixData = reinterpret_cast<edF32MATRIX3*>(reinterpret_cast<char*>(pTag) + offset);
	return;
}

void edAnmStage::SetDestinationWRTS(edANM_WRTS* pMatrixBuffer, int count)
{
	edF32MATRIX4* peVar1;
	edF32MATRIX4* peVar2;

	this->pRelativeTransformMatrixBuffer = pMatrixBuffer;

	if (count != 0) {

		if (count == -1) {
			count = ((this->anmSkeleton).pTag)->boneCount;
		}

		peVar2 = this->pRelativeTransformMatrixBuffer->matrices;
		peVar1 = peVar2 + count;
		if (peVar2 < peVar1) {
			if (8 < (ulong)peVar1 + (0x3f - (ulong)peVar2) >> 6) {
				do {
					peVar2[0].da = -1.0f;
					peVar2[1].da = -1.0f;
					peVar2[2].da = -1.0f;
					peVar2[3].da = -1.0f;
					peVar2[4].da = -1.0f;
					peVar2[5].da = -1.0f;
					peVar2[6].da = -1.0f;
					peVar2[7].da = -1.0f;
					peVar2 = peVar2 + 8;
				} while (peVar2 < peVar1 + -8);
			}

			for (; peVar2 < peVar1; peVar2 = peVar2 + 1) {
				peVar2->da = -1.0f;
			}
		}
	}
	return;
}

void edAnmStage::SetSingleToDestWRTS(edF32MATRIX4* pMatrix)
{
	ushort boneCount;
	undefined8 uVar2;
	undefined8 uVar3;
	edF32MATRIX4* peVar4;
	float fVar5;
	float fVar6;
	float fVar7;
	float fVar8;
	float fVar9;
	float fVar10;
	float fVar11;
	edF32MATRIX4* peVar12;

	peVar12 = this->pRelativeTransformMatrixBuffer->matrices;
	boneCount = ((this->anmSkeleton).pTag)->boneCount;
	peVar4 = this->pRelativeTransformMatrixBuffer->matrices;

	for (; peVar12 < (peVar4 + boneCount); peVar12 = peVar12 + 1) {
		if (pMatrix->da != -1.0f) {
			*pMatrix = *peVar12;
		}
		pMatrix = pMatrix + 1;
	}

	return;
}

void edAnmStage::BlendWithDestWRTS(float alpha, edF32MATRIX4* pMatrix)
{
	ushort uVar1;
	float fVar2;
	edF32MATRIX3* peVar4;
	edF32MATRIX4* pCurMatrix;
	edF32MATRIX4* pRelativeMatrixDataEnd;
	edF32MATRIX3* pSource;
	float fVar5;
	float fVar6;
	float fVar7;
	float fVar8;
	float fVar9;
	float fVar10;

	pCurMatrix = this->pRelativeTransformMatrixBuffer->matrices;
	peVar4 = this->pConstantMatrixData;
	pRelativeMatrixDataEnd = this->pRelativeTransformMatrixBuffer->matrices + ((this->anmSkeleton).pTag)->boneCount;
	for (; pCurMatrix < pRelativeMatrixDataEnd; pCurMatrix = pCurMatrix + 1) {
		if (pMatrix->da != -1.0f) {
			pSource = reinterpret_cast<edF32MATRIX3*>(pCurMatrix);

			if (pCurMatrix->da == -1.0f) {
				pSource = peVar4;
			}

			edQuatShortestSLERPHard(alpha, &pCurMatrix->rowX, &pSource->rowX, &pMatrix->rowX);

			fVar5 = pSource->bb;
			fVar6 = pSource->bc;
			fVar7 = pSource->bd;
			fVar8 = pMatrix->bb;
			fVar9 = pMatrix->bc;
			fVar10 = pMatrix->bd;
			fVar2 = 1.0f - alpha;

			pCurMatrix->ba = pMatrix->ba * alpha + pSource->ba * fVar2;
			pCurMatrix->bb = fVar8 * alpha + fVar5 * fVar2;
			pCurMatrix->bc = fVar9 * alpha + fVar6 * fVar2;
			pCurMatrix->bd = fVar10 * alpha + fVar7 * fVar2;

			fVar5 = pSource->cb;
			fVar6 = pSource->cc;
			fVar7 = pSource->cd;
			fVar8 = pMatrix->cb;
			fVar9 = pMatrix->cc;
			fVar10 = pMatrix->cd;
			fVar2 = 1.0f - alpha;

			pCurMatrix->ca = pMatrix->ca * alpha + pSource->ca * fVar2;
			pCurMatrix->cb = fVar8 * alpha + fVar5 * fVar2;
			pCurMatrix->cc = fVar9 * alpha + fVar6 * fVar2;
			pCurMatrix->cd = fVar10 * alpha + fVar7 * fVar2;

			pCurMatrix->da = 1.0f;
		}

		pMatrix = pMatrix + 1;
		peVar4 = peVar4 + 1;
	}

	return;
}

void edAnmStage::BlendToDestWRTS(float alpha, edF32MATRIX4* pSrc, edF32MATRIX4* pDst)
{
	undefined8 uVar1;
	float fVar2;
	float fVar3;
	edF32MATRIX3* pConstantMatrixDataEnd;
	edF32MATRIX3* pConstantMatrixData;
	edF32MATRIX4* pMatrixBuffer;
	edF32MATRIX3* pCurrentDstMatrix;
	edF32MATRIX3* pCurrentSrcMatrix;
	float fVar5;
	float fVar6;
	float fVar7;
	float fVar8;
	float fVar9;

	pMatrixBuffer = this->pRelativeTransformMatrixBuffer->matrices;
	pConstantMatrixData = this->pConstantMatrixData;
	pConstantMatrixDataEnd = pConstantMatrixData + ((this->anmSkeleton).pTag)->boneCount;
	for (; pConstantMatrixData < pConstantMatrixDataEnd; pConstantMatrixData = pConstantMatrixData + 1) {

		pCurrentSrcMatrix = pConstantMatrixData;
		if (pSrc->da != -1.0f) {
			pCurrentSrcMatrix = (edF32MATRIX3*)pSrc;
		}

		pCurrentDstMatrix = pConstantMatrixData;
		if (pDst->da != -1.0f) {
			pCurrentDstMatrix = (edF32MATRIX3*)pDst;
		}

		if (pCurrentSrcMatrix == pCurrentDstMatrix) {
			if (pCurrentSrcMatrix == pConstantMatrixData) {
				// Inherited neither src or dst.
				pMatrixBuffer->da = -1.0f;
			}
			else {
				// Just src.
				*pMatrixBuffer = *pCurrentSrcMatrix;
				pMatrixBuffer->da = 1.0f;
			}
		}
		else {
			// Blend.
			edQuatShortestSLERPHard(alpha, &pMatrixBuffer->rowX, &pCurrentSrcMatrix->rowX, &pCurrentDstMatrix->rowX);
			fVar3 = pCurrentSrcMatrix->bb;
			fVar5 = pCurrentSrcMatrix->bc;
			fVar6 = pCurrentSrcMatrix->bd;
			fVar7 = pCurrentDstMatrix->bb;
			fVar8 = pCurrentDstMatrix->bc;
			fVar9 = pCurrentDstMatrix->bd;
			fVar2 = 1.0f - alpha;
			pMatrixBuffer->ba = pCurrentDstMatrix->ba * alpha + pCurrentSrcMatrix->ba * fVar2;
			pMatrixBuffer->bb = fVar7 * alpha + fVar3 * fVar2;
			pMatrixBuffer->bc = fVar8 * alpha + fVar5 * fVar2;
			pMatrixBuffer->bd = fVar9 * alpha + fVar6 * fVar2;
			fVar3 = pCurrentSrcMatrix->cb;
			fVar5 = pCurrentSrcMatrix->cc;
			fVar6 = pCurrentSrcMatrix->cd;
			fVar7 = pCurrentDstMatrix->cb;
			fVar8 = pCurrentDstMatrix->cc;
			fVar9 = pCurrentDstMatrix->cd;
			fVar2 = 1.0f - alpha;
			pMatrixBuffer->ca = pCurrentDstMatrix->ca * alpha + pCurrentSrcMatrix->ca * fVar2;
			pMatrixBuffer->cb = fVar7 * alpha + fVar3 * fVar2;
			pMatrixBuffer->cc = fVar8 * alpha + fVar5 * fVar2;
			pMatrixBuffer->cd = fVar9 * alpha + fVar6 * fVar2;
			pMatrixBuffer->da = 1.0f;
		}
		pSrc = pSrc + 1;
		pDst = pDst + 1;
		pMatrixBuffer = pMatrixBuffer + 1;
	}
	return;
}

void edAnmStage::ToonWRTSToGlobalMatrices(uchar mode)
{
	edANM_SKELETON* peVar1;
	float fVar2;
	float fVar3;
	float fVar4;
	float fVar5;
	undefined8 uVar6;
	edF32VECTOR4* pVectorArray;
	edF32MATRIX4* pMatrixArray;
	int iVar7;
	edF32MATRIX3* peVar8;
	edANM_SKELETON* pMeshSectionDataMap;
	edF32MATRIX4* pRelativeTransformMatrixBuffer;
	edF32MATRIX3* pCurrentRelativeTransform;
	edF32MATRIX4* pFrameMatrixEnd;
	edF32MATRIX4* pFrameMatrixData;
	edF32MATRIX4* peVar9;
	edF32MATRIX4* peVar10;
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
	float fVar31;
	float fVar32;
	float fVar33;
	float fVar34;
	float fVar35;
	float fVar36;
	float fVar37;
	float fVar38;
	float fVar39;
	float fVar40;
	float fVar41;
	float fVar42;
	edF32MATRIX4 local_40;
	ushort meshSectionCount;

	pMeshSectionDataMap = (this->anmSkeleton).pTag;
	pFrameMatrixData = this->pFrameMatrixData;
	pRelativeTransformMatrixBuffer = this->pRelativeTransformMatrixBuffer->matrices;
	peVar8 = this->pConstantMatrixData;
	meshSectionCount = pMeshSectionDataMap->boneCount;
	pFrameMatrixEnd = pFrameMatrixData + meshSectionCount;
	for (; pMeshSectionDataMap = pMeshSectionDataMap + 1, pFrameMatrixData < pFrameMatrixEnd;
		pFrameMatrixData = pFrameMatrixData + 1) {
		pCurrentRelativeTransform = (edF32MATRIX3*)pRelativeTransformMatrixBuffer;
		if (pRelativeTransformMatrixBuffer->da == -1.0f) {
			pCurrentRelativeTransform = peVar8;
		}

		int value = *(int*)pMeshSectionDataMap;

		if (value == -1) {
			fVar31 = pCurrentRelativeTransform->aa;
			fVar19 = pCurrentRelativeTransform->ab;
			fVar23 = pCurrentRelativeTransform->ac;
			fVar28 = pCurrentRelativeTransform->ad;
			fVar22 = (fVar19 + 0.0f) * (fVar19 + 0.0f) + (fVar23 + 0.0f) * (fVar23 + 0.0f);
			fVar40 = (fVar31 + 0.0f) * (fVar23 + 0.0f) + (fVar28 + 0.0f) * (fVar19 + 0.0f);
			fVar38 = (fVar31 + 0.0f) * (fVar19 + 0.0f) - (fVar28 + 0.0f) * (fVar23 + 0.0f);
			fVar26 = (fVar31 + 0.0f) * (fVar19 + 0.0f) + (fVar28 + 0.0f) * (fVar23 + 0.0f);
			fVar32 = (fVar31 + 0.0f) * (fVar31 + 0.0f) + (fVar23 + 0.0f) * (fVar23 + 0.0f);
			fVar35 = (fVar19 + 0.0f) * (fVar23 + 0.0f) - (fVar28 + 0.0f) * (fVar31 + 0.0f);
			fVar27 = (fVar19 + 0.0f) * (fVar23 + 0.0f) + (fVar28 + 0.0f) * (fVar31 + 0.0f);
			fVar36 = (fVar31 + 0.0f) * (fVar31 + 0.0f) + (fVar19 + 0.0f) * (fVar19 + 0.0f);
			fVar31 = (fVar31 + 0.0f) * (fVar23 + 0.0f) - (fVar28 + 0.0f) * (fVar19 + 0.0f);
			pFrameMatrixData->aa = (0.0f - (fVar22 + fVar22)) + 1.0f;
			pFrameMatrixData->ab = fVar38 + fVar38;
			pFrameMatrixData->ac = fVar40 + fVar40;
			pFrameMatrixData->ad = 0.0f;
			pFrameMatrixData->ba = fVar26 + fVar26;
			pFrameMatrixData->bb = (0.0f - (fVar32 + fVar32)) + 1.0f;
			pFrameMatrixData->bc = fVar35 + fVar35;
			pFrameMatrixData->bd = 0.0f;
			pFrameMatrixData->ca = fVar31 + fVar31;
			pFrameMatrixData->cb = fVar27 + fVar27;
			pFrameMatrixData->cc = (0.0f - (fVar36 + fVar36)) + 1.0f;
			pFrameMatrixData->cd = 0.0f;
			pFrameMatrixData->da = 1.0f;
			pFrameMatrixData->db = 0.0f;
			pFrameMatrixData->dc = 0.0f;
			pFrameMatrixData->dd = 0.0f;

			pFrameMatrixData->rowT = pCurrentRelativeTransform->rowY;
			pFrameMatrixData->dd = 1.0f;
		}
		else {
			peVar10 = this->pRelativeTransformMatrixBuffer->matrices + value;
			fVar31 = pCurrentRelativeTransform->aa;
			fVar19 = pCurrentRelativeTransform->ab;
			fVar23 = pCurrentRelativeTransform->ac;
			fVar28 = pCurrentRelativeTransform->ad;
			fVar22 = (fVar19 + 0.0f) * (fVar19 + 0.0f) + (fVar23 + 0.0f) * (fVar23 + 0.0f);
			fVar40 = (fVar31 + 0.0f) * (fVar23 + 0.0f) + (fVar28 + 0.0f) * (fVar19 + 0.0f);
			fVar38 = (fVar31 + 0.0f) * (fVar19 + 0.0f) - (fVar28 + 0.0f) * (fVar23 + 0.0f);
			fVar26 = (fVar31 + 0.0f) * (fVar19 + 0.0f) + (fVar28 + 0.0f) * (fVar23 + 0.0f);
			fVar32 = (fVar31 + 0.0f) * (fVar31 + 0.0f) + (fVar23 + 0.0f) * (fVar23 + 0.0f);
			fVar35 = (fVar19 + 0.0f) * (fVar23 + 0.0f) - (fVar28 + 0.0f) * (fVar31 + 0.0f);
			fVar27 = (fVar19 + 0.0f) * (fVar23 + 0.0f) + (fVar28 + 0.0f) * (fVar31 + 0.0f);
			fVar36 = (fVar31 + 0.0f) * (fVar31 + 0.0f) + (fVar19 + 0.0f) * (fVar19 + 0.0f);
			fVar31 = (fVar31 + 0.0f) * (fVar23 + 0.0f) - (fVar28 + 0.0f) * (fVar19 + 0.0f);
			fVar38 = fVar38 + fVar38;
			fVar40 = fVar40 + fVar40;
			fVar22 = (0.0f - (fVar22 + fVar22)) + 1.0f;
			local_40.ba = fVar26 + fVar26;
			local_40.bb = (0.0f - (fVar32 + fVar32)) + 1.0f;
			local_40.bc = fVar35 + fVar35;
			local_40.bd = 0.0f;
			local_40.ca = fVar31 + fVar31;
			local_40.cb = fVar27 + fVar27;
			local_40.cc = (0.0f - (fVar36 + fVar36)) + 1.0f;
			local_40.cd = 0.0f;
			local_40.da = peVar10->ca * peVar8->ba + (pCurrentRelativeTransform->ba - peVar8->ba);
			local_40.db = peVar10->cb * peVar8->bb + (pCurrentRelativeTransform->bb - peVar8->bb);
			local_40.dc = peVar10->cc * peVar8->bc + (pCurrentRelativeTransform->bc - peVar8->bc);
			local_40.dd = 1.0f;
			peVar10 = this->pFrameMatrixData + value;
			fVar14 = 0.0f;
			fVar26 = peVar10->aa;
			fVar31 = peVar10->ab;
			fVar27 = peVar10->ac;
			fVar32 = peVar10->ad;
			fVar35 = peVar10->ba;
			fVar36 = peVar10->bb;
			fVar19 = peVar10->bc;
			fVar23 = peVar10->bd;
			fVar28 = peVar10->ca;
			fVar20 = peVar10->cb;
			fVar24 = peVar10->cc;
			fVar29 = peVar10->cd;
			fVar33 = peVar10->da;
			fVar11 = peVar10->db;
			fVar12 = peVar10->dc;
			fVar13 = peVar10->dd;
			iVar7 = 4;

			edF32VECTOR4* pVecA = (edF32VECTOR4*)pFrameMatrixData;
			edF32VECTOR4* pVecB = (edF32VECTOR4*)&local_40;
			do {
				fVar15 = fVar26 * fVar22;
				fVar21 = fVar31 * fVar22;
				fVar37 = fVar27 * fVar22;
				fVar2 = fVar32 * fVar22;
				fVar16 = fVar35 * fVar38;
				fVar25 = fVar36 * fVar38;
				fVar39 = fVar19 * fVar38;
				fVar3 = fVar23 * fVar38;
				fVar17 = fVar28 * fVar40;
				fVar30 = fVar20 * fVar40;
				fVar41 = fVar24 * fVar40;
				fVar4 = fVar29 * fVar40;
				fVar18 = fVar33 * fVar14;
				fVar34 = fVar11 * fVar14;
				fVar42 = fVar12 * fVar14;
				fVar5 = fVar13 * fVar14;
				iVar7 = iVar7 + -1;
				pVecA->x = fVar15 + fVar16 + fVar17 + fVar18;
				pVecA->y = fVar21 + fVar25 + fVar30 + fVar34;
				pVecA->z = fVar37 + fVar39 + fVar41 + fVar42;
				pVecA->w = fVar2 + fVar3 + fVar4 + fVar5;
				pVecA = pVecA + 1;
				pVecB = pVecB + 1;
				fVar22 = pVecB->x;
				fVar38 = pVecB->y;
				fVar40 = pVecB->z;
				fVar14 = pVecB->w;
			} while (iVar7 != 0);
		}

		// If we are using constant data.
		if (pCurrentRelativeTransform == peVar8) {
			pRelativeTransformMatrixBuffer->rowZ = pCurrentRelativeTransform->rowZ;
		}
		peVar8 = peVar8 + 1;
		pRelativeTransformMatrixBuffer = pRelativeTransformMatrixBuffer + 1;
	}
	pFrameMatrixData = this->pFrameMatrixData;
	pRelativeTransformMatrixBuffer = this->pRelativeTransformMatrixBuffer->matrices;
	pFrameMatrixEnd = pFrameMatrixData + meshSectionCount;
	if (mode == 0) {
		for (; pFrameMatrixData < pFrameMatrixEnd; pFrameMatrixData = pFrameMatrixData + 1) {
			fVar22 = pRelativeTransformMatrixBuffer->ca;
			fVar26 = pRelativeTransformMatrixBuffer->cb;
			fVar31 = pRelativeTransformMatrixBuffer->cc;
			pFrameMatrixData->aa = pFrameMatrixData->aa * fVar22;
			pFrameMatrixData->ab = pFrameMatrixData->ab * fVar22;
			pFrameMatrixData->ac = pFrameMatrixData->ac * fVar22;
			pFrameMatrixData->ad = pFrameMatrixData->ad * fVar22;
			pFrameMatrixData->ba = pFrameMatrixData->ba * fVar26;
			pFrameMatrixData->bb = pFrameMatrixData->bb * fVar26;
			pFrameMatrixData->bc = pFrameMatrixData->bc * fVar26;
			pFrameMatrixData->bd = pFrameMatrixData->bd * fVar26;
			pFrameMatrixData->ca = pFrameMatrixData->ca * fVar31;
			pFrameMatrixData->cb = pFrameMatrixData->cb * fVar31;
			pFrameMatrixData->cc = pFrameMatrixData->cc * fVar31;
			pFrameMatrixData->cd = pFrameMatrixData->cd * fVar31;
			pRelativeTransformMatrixBuffer = pRelativeTransformMatrixBuffer + 1;
		}
	}
	else {
		peVar1 = (this->anmSkeleton).pTag;
		if ((peVar1->flags & 2) == 0) {
			meshSectionCount = peVar1->boneCount;

			uint a = ((uint)meshSectionCount * 2 + (uint)meshSectionCount) * 8;
			uint b = (meshSectionCount * 0xc + 0x13) & 0xfffffff0;
			uint c = (a * 2) + (b);
			pMatrixArray = (edF32MATRIX4*)((char*)peVar1 + c);

			for (; pFrameMatrixData < pFrameMatrixEnd; pFrameMatrixData = pFrameMatrixData + 1) {
				fVar19 = pRelativeTransformMatrixBuffer->ca;
				fVar23 = pRelativeTransformMatrixBuffer->cb;
				fVar28 = pRelativeTransformMatrixBuffer->cc;
				fVar22 = pFrameMatrixData->aa * fVar19;
				fVar38 = pFrameMatrixData->ab * fVar19;
				fVar35 = pFrameMatrixData->ac * fVar19;
				fVar19 = pFrameMatrixData->ad * fVar19;
				fVar26 = pFrameMatrixData->ba * fVar23;
				fVar27 = pFrameMatrixData->bb * fVar23;
				fVar40 = pFrameMatrixData->bc * fVar23;
				fVar23 = pFrameMatrixData->bd * fVar23;
				fVar31 = pFrameMatrixData->ca * fVar28;
				fVar32 = pFrameMatrixData->cb * fVar28;
				fVar36 = pFrameMatrixData->cc * fVar28;
				fVar28 = pFrameMatrixData->cd * fVar28;
				fVar37 = pFrameMatrixData->da;
				fVar39 = pFrameMatrixData->db;
				fVar41 = pFrameMatrixData->dc;
				fVar42 = pFrameMatrixData->dd;
				fVar20 = pMatrixArray->aa;
				fVar24 = pMatrixArray->ab;
				fVar29 = pMatrixArray->ac;
				fVar33 = pMatrixArray->ad;
				fVar11 = pMatrixArray->ba;
				fVar12 = pMatrixArray->bb;
				fVar13 = pMatrixArray->bc;
				fVar14 = pMatrixArray->bd;
				fVar15 = pMatrixArray->ca;
				fVar16 = pMatrixArray->cb;
				fVar17 = pMatrixArray->cc;
				fVar18 = pMatrixArray->cd;
				fVar21 = pMatrixArray->da;
				fVar25 = pMatrixArray->db;
				fVar30 = pMatrixArray->dc;
				fVar34 = pMatrixArray->dd;
				pFrameMatrixData->aa = fVar20 * fVar22 + fVar11 * fVar38 + fVar15 * fVar35 + fVar21 * fVar19;
				pFrameMatrixData->ab = fVar24 * fVar22 + fVar12 * fVar38 + fVar16 * fVar35 + fVar25 * fVar19;
				pFrameMatrixData->ac = fVar29 * fVar22 + fVar13 * fVar38 + fVar17 * fVar35 + fVar30 * fVar19;
				pFrameMatrixData->ad = fVar33 * fVar22 + fVar14 * fVar38 + fVar18 * fVar35 + fVar34 * fVar19;
				pFrameMatrixData->ba = fVar20 * fVar26 + fVar11 * fVar27 + fVar15 * fVar40 + fVar21 * fVar23;
				pFrameMatrixData->bb = fVar24 * fVar26 + fVar12 * fVar27 + fVar16 * fVar40 + fVar25 * fVar23;
				pFrameMatrixData->bc = fVar29 * fVar26 + fVar13 * fVar27 + fVar17 * fVar40 + fVar30 * fVar23;
				pFrameMatrixData->bd = fVar33 * fVar26 + fVar14 * fVar27 + fVar18 * fVar40 + fVar34 * fVar23;
				pFrameMatrixData->ca = fVar20 * fVar31 + fVar11 * fVar32 + fVar15 * fVar36 + fVar21 * fVar28;
				pFrameMatrixData->cb = fVar24 * fVar31 + fVar12 * fVar32 + fVar16 * fVar36 + fVar25 * fVar28;
				pFrameMatrixData->cc = fVar29 * fVar31 + fVar13 * fVar32 + fVar17 * fVar36 + fVar30 * fVar28;
				pFrameMatrixData->cd = fVar33 * fVar31 + fVar14 * fVar32 + fVar18 * fVar36 + fVar34 * fVar28;
				pFrameMatrixData->da = fVar20 * fVar37 + fVar11 * fVar39 + fVar15 * fVar41 + fVar21 * fVar42;
				pFrameMatrixData->db = fVar24 * fVar37 + fVar12 * fVar39 + fVar16 * fVar41 + fVar25 * fVar42;
				pFrameMatrixData->dc = fVar29 * fVar37 + fVar13 * fVar39 + fVar17 * fVar41 + fVar30 * fVar42;
				pFrameMatrixData->dd = fVar33 * fVar37 + fVar14 * fVar39 + fVar18 * fVar41 + fVar34 * fVar42;
				pRelativeTransformMatrixBuffer = pRelativeTransformMatrixBuffer + 1;
				pMatrixArray = pMatrixArray + 1;
			}
		}
		else {
			meshSectionCount = peVar1->boneCount;
			uint a = ((uint)meshSectionCount * 2 + (uint)meshSectionCount) * 8;
			uint b = (meshSectionCount * 0xc + 0x13) & 0xfffffff0;
			uint c = (a * 2) + (b);
			pVectorArray = (edF32VECTOR4*)((char*)peVar1 + c);
			for (; pFrameMatrixData < pFrameMatrixEnd; pFrameMatrixData = pFrameMatrixData + 1) {

				float xScale = pRelativeTransformMatrixBuffer->ca;
				float yScale = pRelativeTransformMatrixBuffer->cb;
				float zScale = pRelativeTransformMatrixBuffer->cc;

				edF32VECTOR4 scaledRowX = pFrameMatrixData->rowX * xScale;
				edF32VECTOR4 scaledRowY = pFrameMatrixData->rowY * yScale;
				edF32VECTOR4 scaledRowZ = pFrameMatrixData->rowZ * zScale;

				fVar20 = pVectorArray->x;
				fVar24 = pVectorArray->y;
				fVar29 = pVectorArray->z;
				fVar33 = pVectorArray->w;

				pFrameMatrixData->rowX = scaledRowX;
				pFrameMatrixData->rowY = scaledRowY;
				pFrameMatrixData->rowZ = scaledRowZ;

				pFrameMatrixData->da = scaledRowX.x * fVar20 + scaledRowY.x * fVar24 + scaledRowZ.x * fVar29 + pFrameMatrixData->da * fVar33;
				pFrameMatrixData->db = scaledRowX.y * fVar20 + scaledRowY.y * fVar24 + scaledRowZ.y * fVar29 + pFrameMatrixData->db * fVar33;
				pFrameMatrixData->dc = scaledRowX.z * fVar20 + scaledRowY.z * fVar24 + scaledRowZ.z * fVar29 + pFrameMatrixData->dc * fVar33;
				pFrameMatrixData->dd = scaledRowX.w * fVar20 + scaledRowY.w * fVar24 + scaledRowZ.w * fVar29 + pFrameMatrixData->dd * fVar33;

				pRelativeTransformMatrixBuffer = pRelativeTransformMatrixBuffer + 1;
				pVectorArray = pVectorArray + 1;
			}
		}
	}
	return;
}

bool edAnmStage::ComputeAnimParams(float param_1, float param_2, float param_3, float* pOutParams, bool param_5, int loopType)
{
	bool bVar1;
	float fVar2;
	float fVar3;

	if (param_5 != false) {
		param_1 = param_1 * param_2;
	}
	if (param_2 <= 0.0f) {
		pOutParams[3] = 0.0f;
		bVar1 = true;
		pOutParams[2] = 0.0f;
		pOutParams[1] = 0.0f;
		*pOutParams = 0.0f;
	}
	else {
		if ((param_1 < 0.0) || (param_2 <= param_1)) {
			if (loopType != 2) {
				if (loopType == 1) {
					fVar3 = param_3;
					if (param_1 < 0.0f) {
						fVar2 = fmodf(param_1, param_2 - param_3);
						pOutParams[2] = fVar2;
					}
					else {
						fVar2 = fmodf(param_1 - param_2, param_2 - param_3);
						pOutParams[2] = fVar2;
					}
					*pOutParams = pOutParams[2] + fVar3;
					pOutParams[3] = pOutParams[2] / (param_2 - param_3);
					pOutParams[1] = *pOutParams / param_2;
				}
				else {
					if (loopType == 0) {
						*pOutParams = param_2;
						pOutParams[1] = 1.0f;
						pOutParams[2] = param_2 - param_3;
						pOutParams[3] = 1.0f;
					}
				}
			}
			bVar1 = true;
		}
		else {
			*pOutParams = param_1;
			bVar1 = false;
			pOutParams[1] = param_1 / param_2;
			pOutParams[2] = param_1 - param_3;
			pOutParams[3] = (param_1 - param_3) / param_2 - param_3;
		}
	}
	return bVar1;
}

void edAnmStage::SetAnim(edANM_HDR* pKeyData)
{
	ANIMATION_LOG(LogLevel::Verbose, "edAnmStage::SetAnim start: {} end: {}", pKeyData->keyIndex_0x8.asTime, pKeyData->field_0x4.asTime);

	this->pKeyData = pKeyData;
	this->bLoop = 0;
	this->field_0x2c = pKeyData->keyIndex_0x8.asTime;
	this->field_0x38 = 0.0f;
	this->field_0x34 = pKeyData->field_0x4.asTime - pKeyData->keyIndex_0x8.asTime;
	this->field_0x30 = this->field_0x2c;
	return;
}

void edAnmStage::SetAnimLoop(edANM_HDR* pKeyData)
{
	ANIMATION_LOG(LogLevel::Verbose, "edAnmStage::SetAnimLoop start: {} end: {}", pKeyData->keyIndex_0x8.asTime, pKeyData->field_0x4.asTime);

	this->pKeyData = pKeyData;
	this->bLoop = 1;
	this->field_0x2c = pKeyData->field_0x4.asTime;
	this->field_0x38 = 0.0f;
	this->field_0x34 = pKeyData->field_0x4.asTime - pKeyData->keyIndex_0x8.asTime;
	this->field_0x30 = this->field_0x2c;
	return;
}

void edAnmStage::SetTimeAsRatio(float ratio)
{
	bool bVar1;
	float aOutParams[4];

	ANIMATION_LOG(LogLevel::Verbose, "edAnmStage::SetTimeAsRatio ratio: {}", ratio);

	bVar1 = ComputeAnimParams(ratio * this->field_0x2c, this->field_0x2c, this->field_0x38, aOutParams, false, this->bLoop);
	this->field_0x4c = bVar1;
	this->animTime = aOutParams[0];
	this->field_0x40 = aOutParams[1];
	this->field_0x44 = aOutParams[2];
	this->field_0x48 = aOutParams[3];

	bVar1 = this->bLoop != 0;
	if ((bVar1) && (bVar1 = true, this->animTime < this->field_0x2c - this->field_0x34)) {
		bVar1 = false;
	}

	this->field_0x4d = bVar1;
	return;
}

void edAnmStage::SetTime(float time)
{
	bool bVar1;
	float aOutParams[4];

	ANIMATION_LOG(LogLevel::Verbose, "edAnmStage::SetTime time: {}", time);

	bVar1 = ComputeAnimParams(time, this->field_0x2c, this->field_0x38, aOutParams, false, this->bLoop);
	this->field_0x4c = bVar1;
	this->animTime = aOutParams[0];
	this->field_0x40 = aOutParams[1];
	this->field_0x44 = aOutParams[2];
	this->field_0x48 = aOutParams[3];
	bVar1 = this->bLoop != 0;
	if ((bVar1) && (bVar1 = true, this->animTime < this->field_0x2c - this->field_0x34)) {
		bVar1 = false;
	}
	this->field_0x4d = bVar1;
	return;
}

struct edANM_RTS_Key
{
	ushort flags;
	ushort field_0x2;
	float field_0x4;
	ushort field_0x8;
	ushort field_0x10;
};

struct edANM_RTS_Key_Hdr
{
	ushort flags;
	ushort keyCount;
	int keyOffset;
	uint nodeId;
};

struct edANM_RTS 
{
	edANM_RTS() {

	}

	edANM_RTS(edANM_RTS_Key_Hdr* pInAnm) : pAnm(pInAnm) {

	}

	edANM_RTS_Key_Hdr* pAnm;
};

struct edAnmTransformCtrl
{
	static void GetValue(float time, edANM_RTS* ppKeyData, edF32MATRIX3* pAnimMatrix);
};

void edAnmTransformCtrl::GetValue(float time, edANM_RTS* ppKeyData, edF32MATRIX3* pAnimMatrix)
{
	ushort uVar1;
	float* pfVar2;
	int iVar3;
	uint uVar4;
	uint uVar5;
	ushort* puVar6;
	float* pfVar7;
	short* rowCTransformC;
	ushort* rowCTransformD;
	short* rowBTransformC;
	ushort* rowBTransformD;
	edANM_RTS_Key* pDataStream;
	short* rowCTransformA;
	ushort* rowCTransformB;
	uint dataCount;
	short* transShortBuffer;
	short* transShortBufferB;
	short* rowBTransformA;
	short* rowBTransformB;
	int iVar8;
	float* pfVar9;
	uint uVar10;
	float puVar14;
	float fVar11;
	float fVar12;
	float fVar13;
	float fVar14;
	edF32VECTOR4 local_60;
	edF32VECTOR4 local_50;
	edF32VECTOR3 local_40;
	edF32VECTOR3 local_30;
	edANM_RTS_Key_Hdr* pKeyData;

	if (time < 0.0f) {
		time = 0.0f;
	}

	pKeyData = ppKeyData->pAnm;
	pDataStream = (edANM_RTS_Key*)(pKeyData + 1);

	if ((pKeyData->flags & 1) != 0) {
		ANIMATION_LOG(LogLevel::Verbose, "edAnmTransformCtrl::GetValue contains row X data");

		dataCount = (uint)pDataStream->flags;
		if ((pDataStream->field_0x2 & 1) == 0) {
			pfVar2 = &pDataStream->field_0x4 + dataCount;
		}
		else {
			if ((pDataStream->flags & 1) != 0) {
				dataCount = dataCount - 1;
			}
			/* Find the offset for the float data. */
			pfVar2 = (float*)(&pDataStream->field_0x8 + dataCount);
		}
	
		pfVar7 = &pDataStream->field_0x4;
		if (pDataStream->field_0x4 <= time) {
			time = pDataStream->field_0x4 - 1e-06f;
		}
		dataCount = (uint)pDataStream->flags;
		if (dataCount == 1) {
			if ((pDataStream->field_0x2 & 6) == 0) {
				pAnimMatrix->aa = pfVar2[0];
				pAnimMatrix->ab = pfVar2[1];
				pAnimMatrix->ac = pfVar2[2];
				pAnimMatrix->ad = pfVar2[3];
			}
			else {
				short* pTrack = (short*)pfVar2;

				pAnimMatrix->aa = (float)(int)pTrack[0] * 3.051851e-05f;
				pAnimMatrix->ab = (float)(int)pTrack[1] * 3.051851e-05f;

				uint thirdAxis = pTrack[2];
				
				pAnimMatrix->ac = (float)((int)(thirdAxis & 0xfffffffe)) * 3.051851e-05f;

				float sum = (pAnimMatrix->ac * pAnimMatrix->ac +
					pAnimMatrix->aa * pAnimMatrix->aa + pAnimMatrix->ab * pAnimMatrix->ab);

				assert(sum < 1.0f);

				pAnimMatrix->ad = sqrtf(1.0f - sum);
				if ((thirdAxis & 1) != 0) {
					pAnimMatrix->ad = -pAnimMatrix->ad;
				}
			}
		}
		else {
			if ((pDataStream->field_0x2 & 1) == 0) {
				fVar11 = pfVar7[dataCount - 1];
				if (fVar11 <= time) {
					iVar8 = dataCount - 1;
					fVar12 = 0.0f;
					iVar3 = 0;
					if (0.0f < *pfVar7 - fVar11) {
						fVar12 = (time - fVar11) / (*pfVar7 - fVar11);
					}
				}
				else {
					do {
						dataCount = (int)dataCount >> 1;
						if (pfVar7[dataCount] <= time) break;
					} while (1 < dataCount);

					if (dataCount == 0) {
						dataCount = 1;
					}

					for (pfVar9 = pfVar7 + dataCount; *pfVar9 <= time; pfVar9 = pfVar9 + 1) {
					}

					int dataLen = (char*)pfVar9 - (char*)pfVar7;
					if (dataLen < 0) {
						dataLen = dataLen + 3;
					}
					iVar3 = dataLen >> 2;
					iVar8 = iVar3 + -1;
					if (iVar8 == 0) {
						fVar12 = 0.0f;
						if (0.0f < pfVar9[0]) {
							fVar12 = time / pfVar9[0];
						}
					}
					else {
						fVar12 = 0.0f;
						fVar11 = pfVar9[0] - pfVar9[-1];
						if (0.0f < fVar11) {
							fVar12 = (time - pfVar9[-1]) / fVar11;
						}
					}
				}
			}
			else {
				fVar11 = (time / *pfVar7) * 65535.0f;
				if (fVar11 < 2.147484e+09f) {
					uVar1 = pDataStream->field_0x8;
				}
				else {
					fVar11 = fVar11 - 2.147484e+09f;
					uVar1 = pDataStream->field_0x8;
				}
				uVar4 = (uint)fVar11;
				uVar10 = uVar4 & 0xffff;
				ushort* pUshortBuffer = &pDataStream->field_0x8;
				if (uVar10 < uVar1) {
					iVar8 = 0;
					iVar3 = 1;
					fVar12 = (float)uVar10 / (float)(uint)uVar1;
				}
				else {
					uVar5 = pUshortBuffer[dataCount - 2];
					if (uVar10 < uVar5) {
						iVar8 = dataCount - 1;
						do {
							iVar8 = iVar8 >> 1;
							if (pUshortBuffer[iVar8] <= (uVar4 & 0xffff)) break;
						} while (0 < iVar8);
						for (pUshortBuffer = pUshortBuffer + iVar8; *pUshortBuffer <= (uVar4 & 0xffff); pUshortBuffer = pUshortBuffer + 1) {
							iVar8 = iVar8 + 1;
						}
						fVar12 = 0.0f;
						fVar11 = (float)(uint)*pUshortBuffer - (float)(uint)pUshortBuffer[-1];
						iVar3 = iVar8 + 1;
						if (0.0f < fVar11) {
							float len = ((float)(uVar4 & 0xffff) - (float)(uint)pUshortBuffer[-1]);
							fVar12 = len / fVar11;
						}
					}
					else {
						iVar8 = dataCount - 1;
						fVar13 = (float)uVar5;
						fVar12 = 0.0;
						fVar11 = 65535.0 - fVar13;
						iVar3 = 0;
						if (0.0 < fVar11) {
							fVar12 = ((float)uVar10 - fVar13) / fVar11;
						}
					}
				}
			}
			if ((pDataStream->field_0x2 & 6) == 0) {
				IMPLEMENTATION_GUARD(
				pfVar7 = pfVar2 + iVar8 * 4;
				local_50.x = *pfVar7;
				local_50.y = pfVar7[1];
				local_50.z = pfVar7[2];
				local_50.w = pfVar7[3];
				pfVar7 = pfVar2 + iVar3 * 4;
				local_60.x = *pfVar7;
				local_60.y = pfVar7[1];
				local_60.z = pfVar7[2];
				local_60.w = pfVar7[3];)
			}
			else {
				transShortBuffer = (short*)((char*)pfVar2 + iVar8 * 6);
				local_50.x = (float)(int)transShortBuffer[0] * 3.051851e-05f;
				local_50.y = (float)(int)transShortBuffer[1] * 3.051851e-05f;
				local_50.z = (float)((int)((uint)transShortBuffer[2] & 0xfffffffe)) * 3.051851e-05f;
				local_50.w = sqrtf(1.0f - (local_50.z * local_50.z + local_50.x * local_50.x + local_50.y * local_50.y));
				if ((transShortBuffer[2] & 1U) != 0) {
					local_50.w = -local_50.w;
				}
				transShortBufferB = (short*)((char*)pfVar2 + iVar3 * 6);
				local_60.x = (float)(int)transShortBufferB[0] * 3.051851e-05f;
				local_60.y = (float)(int)transShortBufferB[1] * 3.051851e-05f;
				local_60.z = (float)((int)((uint)transShortBufferB[2] & 0xfffffffe)) * 3.051851e-05f;
				local_60.w = sqrtf(1.0 - (local_60.z * local_60.z + local_60.x * local_60.x + local_60.y * local_60.y));
				if ((transShortBufferB[2] & 1U) != 0) {
					local_60.w = -local_60.w;
				}
			}
			if ((iVar3 == 0) && ((pDataStream->field_0x2 & 8) != 0)) {
				local_60.x = 0.0f - local_60.x;
				local_60.y = 0.0f - local_60.y;
				local_60.z = 0.0f - local_60.z;
				local_60.w = 0.0f - local_60.w;
			}
			puVar14 = local_50.x * local_60.x + local_50.y * local_60.y + local_50.z * local_60.z + local_50.w * local_60.w;
			if (puVar14 < -1.0f) {
				puVar14 = -1.0f;
			}
			else {
				if (1.0f < puVar14) {
					puVar14 = 1.0f;
				}
			}
			fVar11 = (((1.570729f - puVar14 * 0.2121144f) + puVar14 * puVar14 * 0.074261f) -
				puVar14 * puVar14 * puVar14 * 0.0187293f) * sqrtf(1.0f - puVar14);

			fVar14 = fVar11 - fVar12 * fVar11;
			fVar12 = fVar12 * fVar11;

			fVar11 = sinf(fVar11);

			if (fVar11 < 1e-06f) {
				pAnimMatrix->rowX = local_60;
			}
			else {
				fVar11 = 1.0f / fVar11;
				fVar13 = sinf(fVar14) * fVar11;
				fVar11 = sinf(fVar12) * fVar11;

				// 0.777459
				// 0.223343
				pAnimMatrix->aa = local_50.x * fVar13 + local_60.x * fVar11;
				pAnimMatrix->ab = local_50.y * fVar13 + local_60.y * fVar11;
				pAnimMatrix->ac = local_50.z * fVar13 + local_60.z * fVar11;
				pAnimMatrix->ad = local_50.w * fVar13 + local_60.w * fVar11;
			}
		}

		if ((pDataStream->field_0x2 & 6) == 0) {
			pDataStream = (edANM_RTS_Key*)(pfVar2 + (uint)pDataStream->flags * 4);
		}
		else {
			puVar6 = &pDataStream->flags;
			pDataStream = (edANM_RTS_Key*)((char*)pfVar2 + (uint)*puVar6 * 6);
			if (((uint)*puVar6 * 3 & 1) != 0) {
				pDataStream = (edANM_RTS_Key*)&pDataStream->field_0x2;
			}
		}
	}

	if ((ppKeyData->pAnm->flags & 2) != 0) {
		ANIMATION_LOG(LogLevel::Verbose, "edAnmTransformCtrl::GetValue contains row Y data");

		dataCount = (uint)pDataStream->flags;
		if ((pDataStream->field_0x2 & 1) == 0) {
			pfVar2 = &pDataStream->field_0x4 + dataCount;
		}
		else {
			if ((pDataStream->flags & 1) != 0) {
				dataCount = dataCount - 1;
			}
			pfVar2 = (float*)(&pDataStream->field_0x8 + dataCount);
		}
		pfVar7 = &pDataStream->field_0x4;
		if (pDataStream->field_0x4 <= time) {
			time = pDataStream->field_0x4 - 1e-06;
		}
		dataCount = (uint)pDataStream->flags;
		if (dataCount == 1) {
			if ((pDataStream->field_0x2 & 6) == 0) {
				pAnimMatrix->ba = pfVar2[0];
				pAnimMatrix->bb = pfVar2[1];
				pAnimMatrix->bc = pfVar2[2];
			}
			else {
				if ((pDataStream->field_0x2 & 6) == 2) {
					short* pTrack = (short*)pfVar2;
					pAnimMatrix->ba = (float)(int)pTrack[0] * 6.103888e-05f;
					pAnimMatrix->bb = (float)(int)pTrack[1] * 6.103888e-05f;
					pAnimMatrix->bc = (float)(int)pTrack[2] * 6.103888e-05f;
				}
				else {
					ushort* pTrack = (ushort*)pfVar2;
					fVar12 = pfVar2[0];
					fVar11 = pfVar2[1] / 65535.0f;
					pAnimMatrix->ba = fVar12 + fVar11 * (float)(uint)pTrack[0];
					pAnimMatrix->bb = fVar12 + fVar11 * (float)(uint)pTrack[1];
					pAnimMatrix->bc = fVar12 + fVar11 * (float)(uint)pTrack[2];
				}
			}
			pAnimMatrix->bd = 1.0f;
		}
		else {

			if ((pDataStream->field_0x2 & 1) == 0) {
				fVar11 = pfVar7[dataCount - 1];
				if (fVar11 <= time) {
					iVar8 = dataCount - 1;
					fVar12 = 0.0f;
					iVar3 = 0;
					if (0.0f < *pfVar7 - fVar11) {
						fVar12 = (time - fVar11) / (*pfVar7 - fVar11);
					}
				}
				else {
					do {
						dataCount = (int)dataCount >> 1;
						if (pfVar7[dataCount] <= time) break;
					} while (1 < dataCount);
					if (dataCount == 0) {
						dataCount = 1;
					}
					for (pfVar9 = pfVar7 + dataCount; *pfVar9 <= time; pfVar9 = pfVar9 + 1) {
					}
					int dataSize = ((char*)pfVar9 - (char*)pfVar7);
					if (dataSize < 0) {
						dataSize = dataSize + 3;
					}
					iVar3 = dataSize >> 2;
					iVar8 = iVar3 + -1;
					if (iVar8 == 0) {
						fVar12 = 0.0f;
						if (0.0 < *pfVar9) {
							fVar12 = time / *pfVar9;
						}
					}
					else {
						fVar12 = 0.0f;
						fVar11 = *pfVar9 - pfVar9[-1];
						if (0.0f < fVar11) {
							fVar12 = (time - pfVar9[-1]) / fVar11;
						}
					}
				}
			}
			else {
				fVar11 = (time / *pfVar7) * 65535.0f;
				if (2.147484e+09f <= fVar11) {
					fVar11 = fVar11 - 2.147484e+09f;
				}
				uVar4 = (uint)fVar11;
				ushort* pUshortBuffer = &pDataStream->field_0x8;
				uVar10 = uVar4 & 0xffff;
				if (uVar10 < pDataStream->field_0x8) {
					iVar8 = 0;
					iVar3 = 1;
					fVar12 = (float)uVar10 / (float)pDataStream->field_0x8;
				}
				else {
					uVar5 = pUshortBuffer[dataCount - 2];
					if (uVar10 < uVar5) {
						iVar8 = dataCount - 1;
						do {
							iVar8 = iVar8 >> 1;
							if (pUshortBuffer[iVar8] <= (uVar4 & 0xffff)) break;
						} while (0 < iVar8);
						for (puVar6 = pUshortBuffer + iVar8; (uint)*puVar6 <= (uVar4 & 0xffff); puVar6 = puVar6 + 1) {
							iVar8 = iVar8 + 1;
						}
						fVar12 = 0.0f;
						fVar11 = (float)(uint)*puVar6 - (float)(uint)puVar6[-1];
						iVar3 = iVar8 + 1;
						if (0.0f < fVar11) {
							fVar12 = ((float)(uVar4 & 0xffff) - (float)(uint)puVar6[-1]) / fVar11;
						}
					}
					else {
						iVar8 = dataCount - 1;
						fVar13 = (float)uVar5;
						fVar12 = 0.0;
						fVar11 = 65535.0f - fVar13;
						iVar3 = 0;
						if (0.0f < fVar11) {
							fVar12 = ((float)uVar10 - fVar13) / fVar11;
						}
					}
				}
			}
			if ((pDataStream->field_0x2 & 6) == 0) {
				pfVar7 = pfVar2 + iVar8 * 3;
				local_30.x = *pfVar7;
				local_30.y = pfVar7[1];
				local_30.z = pfVar7[2];
			}
			else {
				if ((pDataStream->field_0x2 & 6) == 2) {
					rowBTransformA = (short*)((char*)pfVar2 + iVar8 * 6);
					local_30.x = (float)(int)rowBTransformA[0] * 6.103888e-05f;
					local_30.y = (float)(int)rowBTransformA[1] * 6.103888e-05f;
					local_30.z = (float)(int)rowBTransformA[2] * 6.103888e-05f;
				}
				else {
					ushort* pTransform = (ushort*)(iVar8 * 6 + (char*)pfVar2);
					fVar11 = pfVar2[0];
					fVar13 = pfVar2[1] / 65535.0f;

					const float a = pTransform[4];
					const float b = pTransform[5];
					const float c = pTransform[6];

					local_30.x = fVar11 + fVar13 * a;
					local_30.y = fVar11 + fVar13 * b;
					local_30.z = fVar11 + fVar13 * c;
				}
			}

			if ((pDataStream->field_0x2 & 6) == 0) {
				pfVar7 = pfVar2 + iVar3 * 3;
				local_40.x = pfVar7[0];
				local_40.y = pfVar7[1];
				local_40.z = pfVar7[2];
			}
			else {
				if ((pDataStream->field_0x2 & 6) == 2) {
					rowBTransformC = (short*)((char*)pfVar2 + iVar3 * 6);
					local_40.x = (float)(int)rowBTransformC[0] * 6.103888e-05f;
					local_40.y = (float)(int)rowBTransformC[1] * 6.103888e-05f;
					local_40.z = (float)(int)rowBTransformC[2] * 6.103888e-05f;
				}
				else {
					rowBTransformD = (ushort*)(iVar3 * 6 + (char*)pfVar2);
					fVar11 = pfVar2[0];
					fVar13 = pfVar2[1] / 65535.0f;
					local_40.x = fVar11 + fVar13 * (float)(uint)rowBTransformD[4];
					local_40.y = fVar11 + fVar13 * (float)(uint)rowBTransformD[5];
					local_40.z = fVar11 + fVar13 * (float)(uint)rowBTransformD[6];
				}
			}
			fVar11 = 1.0f - fVar12;
			pAnimMatrix->ba = local_40.x * fVar12 + local_30.x * fVar11;
			pAnimMatrix->bb = local_40.y * fVar12 + local_30.y * fVar11;
			pAnimMatrix->bc = local_40.z * fVar12 + local_30.z * fVar11;
			pAnimMatrix->bd = fVar12 * 1.0f + fVar11 * 1.0f;
		}

		if ((pDataStream->field_0x2 & 6) == 0) {
			pDataStream = (edANM_RTS_Key*)(pfVar2 + (uint)pDataStream->flags * 3);
		}
		else {
			if ((pDataStream->field_0x2 & 6) == 2) {
				dataCount = (uint)pDataStream->flags * 3;
				if ((dataCount & 1) != 0) {
					dataCount = dataCount + 1;
				}
				pDataStream = (edANM_RTS_Key*)((char*)pfVar2 + dataCount * 2);
			}
			else {
				dataCount = (uint)pDataStream->flags * 3;
				if ((dataCount & 1) != 0) {
					dataCount = dataCount + 1;
				}
				pDataStream = (edANM_RTS_Key*)((char*)pfVar2 + dataCount * 2 + 8);
			}
		}
	}
	
	if ((ppKeyData->pAnm->flags & 4) != 0) {
		ANIMATION_LOG(LogLevel::Verbose, "edAnmTransformCtrl::GetValue contains row Z data");

		dataCount = (uint)pDataStream->flags;
		if ((pDataStream->field_0x2 & 1) == 0) {
			pfVar2 = &pDataStream->field_0x4 + dataCount;
		}
		else {
			if ((pDataStream->flags & 1) != 0) {
				dataCount = dataCount - 1;
			}
			pfVar2 = (float*)(&pDataStream->field_0x8 + dataCount);
		}
		pfVar7 = &pDataStream->field_0x4;
		if (pDataStream->field_0x4 <= time) {
			time = pDataStream->field_0x4 - 1e-06f;
		}
		dataCount = (uint)pDataStream->flags;
		if (dataCount == 1) {
			if ((pDataStream->field_0x2 & 6) == 0) {
				pAnimMatrix->ca = pfVar2[0];
				pAnimMatrix->cb = pfVar2[1];
				pAnimMatrix->cc = pfVar2[2];
			}
			else {
				if ((pDataStream->field_0x2 & 6) == 2) {
					short* pTrack = (short*)pfVar2;
					pAnimMatrix->ca = (float)(int)pTrack[0] * 6.103888e-05f;
					pAnimMatrix->cb = (float)(int)pTrack[1] * 6.103888e-05f;
					pAnimMatrix->cc = (float)(int)pTrack[2] * 6.103888e-05f;
				}
				else {
					ushort* pTrack = (ushort*)pfVar2;
					fVar12 = *pfVar2;
					fVar11 = pfVar2[1] / 65535.0;
					pAnimMatrix->ca = fVar12 + fVar11 * (float)(uint)pTrack[0];
					pAnimMatrix->cb = fVar12 + fVar11 * (float)(uint)pTrack[1];
					pAnimMatrix->cc = fVar12 + fVar11 * (float)(uint)pTrack[2];
				}
			}
			pAnimMatrix->cd = 1.0f;
		}
		else {
			if ((pDataStream->field_0x2 & 1) == 0) {
				fVar11 = pfVar7[dataCount - 1];
				if (fVar11 <= time) {
					iVar8 = dataCount - 1;
					fVar12 = 0.0f;
					iVar3 = 0;
					if (0.0f < *pfVar7 - fVar11) {
						fVar12 = (time - fVar11) / (*pfVar7 - fVar11);
					}
				}
				else {
					do {
						dataCount = (int)dataCount >> 1;
						if (pfVar7[dataCount] <= time) break;
					} while (1 < dataCount);
					if (dataCount == 0) {
						dataCount = 1;
					}
					for (pfVar9 = pfVar7 + dataCount; *pfVar9 <= time; pfVar9 = pfVar9 + 1) {
					}
					pfVar7 = (float*)((int)pfVar9 - (int)pfVar7);
					if ((int)pfVar7 < 0) {
						pfVar7 = (float*)((int)pfVar7 + 3);
					}
					iVar3 = (int)pfVar7 >> 2;
					iVar8 = iVar3 + -1;
					if (iVar8 == 0) {
						fVar12 = 0.0f;
						if (0.0f < *pfVar9) {
							fVar12 = time / *pfVar9;
						}
					}
					else {
						fVar12 = 0.0f;
						fVar11 = *pfVar9 - pfVar9[-1];
						if (0.0f < fVar11) {
							fVar12 = (time - pfVar9[-1]) / fVar11;
						}
					}
				}
			}
			else {
				fVar11 = (time / *pfVar7) * 65535.0f;
				if (fVar11 < 2.147484e+09f) {
					uVar1 = pDataStream->field_0x8;
				}
				else {
					fVar11 = fVar11 - 2.147484e+09f;
					uVar1 = pDataStream->field_0x8;
				}
				uVar4 = (uint)fVar11;
				uVar10 = uVar4 & 0xffff;
				ushort* pUshortBuffer = &pDataStream->field_0x8;
				if (uVar10 < uVar1) {
					iVar8 = 0;
					iVar3 = 1;
					fVar12 = (float)uVar10 / (float)(uint)uVar1;
				}
				else {
					uVar5 = pUshortBuffer[dataCount - 2];
					if (uVar10 < uVar5) {
						iVar8 = dataCount - 1;
						do {
							iVar8 = iVar8 >> 1;
							if (pUshortBuffer[iVar8] <= (uVar4 & 0xffff)) break;
						} while (0 < iVar8);
						for (puVar6 = pUshortBuffer + iVar8; (uint)*puVar6 <= (uVar4 & 0xffff); puVar6 = puVar6 + 1) {
							iVar8 = iVar8 + 1;
						}
						fVar12 = 0.0f;
						fVar11 = (float)(uint)*puVar6 - (float)(uint)puVar6[-1];
						iVar3 = iVar8 + 1;
						if (0.0f < fVar11) {
							fVar12 = ((float)(uVar4 & 0xffff) - (float)(uint)puVar6[-1]) / fVar11;
						}
					}
					else {
						iVar8 = dataCount - 1;
						fVar13 = (float)uVar5;
						fVar12 = 0.0f;
						fVar11 = 65535.0f - fVar13;
						iVar3 = 0;
						if (0.0f < fVar11) {
							fVar12 = ((float)uVar10 - fVar13) / fVar11;
						}
					}
				}
			}

			if ((pDataStream->field_0x2 & 6) == 0) {
				pfVar7 = pfVar2 + iVar8 * 3;
				local_30.x = pfVar7[0];
				local_30.y = pfVar7[1];
				local_30.z = pfVar7[2];
			}
			else {
				if ((pDataStream->field_0x2 & 6) == 2) {
					rowCTransformA = (short*)((char*)pfVar2 + iVar8 * 6);
					local_30.x = (float)(int)rowCTransformA[0] * 6.103888e-05;
					local_30.y = (float)(int)rowCTransformA[1] * 6.103888e-05;
					local_30.z = (float)(int)rowCTransformA[2] * 6.103888e-05;
				}
				else {
					rowCTransformB = (ushort*)(iVar8 * 6 + (char*)pfVar2);
					fVar11 = pfVar2[0];
					fVar13 = pfVar2[1] / 65535.0f;
					local_30.x = fVar11 + fVar13 * (float)(uint)rowCTransformB[4];
					local_30.y = fVar11 + fVar13 * (float)(uint)rowCTransformB[5];
					local_30.z = fVar11 + fVar13 * (float)(uint)rowCTransformB[6];
				}
			}
			if ((pDataStream->field_0x2 & 6) == 0) {
				pfVar2 = pfVar2 + iVar3 * 3;
				local_40.x = pfVar2[0];
				local_40.y = pfVar2[1];
				local_40.z = pfVar2[2];
			}
			else {
				if ((pDataStream->field_0x2 & 6) == 2) {
					rowCTransformC = (short*)((char*)pfVar2 + iVar3 * 6);
					local_40.x = (float)(int)rowCTransformC[0] * 6.103888e-05f;
					local_40.y = (float)(int)rowCTransformC[1] * 6.103888e-05f;
					local_40.z = (float)(int)rowCTransformC[2] * 6.103888e-05f;
				}
				else {
					rowCTransformD = (ushort*)(iVar3 * 6 + (char*)pfVar2);
					fVar11 = pfVar2[0];
					fVar13 = pfVar2[1] / 65535.0f;
					local_40.x = fVar11 + fVar13 * (float)(uint)rowCTransformD[4];
					local_40.y = fVar11 + fVar13 * (float)(uint)rowCTransformD[5];
					local_40.z = fVar11 + fVar13 * (float)(uint)rowCTransformD[6];
				}
			}

			fVar11 = 1.0f - fVar12;

			pAnimMatrix->ca = local_40.x * fVar12 + local_30.x * fVar11;
			pAnimMatrix->cb = local_40.y * fVar12 + local_30.y * fVar11;
			pAnimMatrix->cc = local_40.z * fVar12 + local_30.z * fVar11;
			pAnimMatrix->cd = fVar12 * 1.0f + fVar11 * 1.0f;
		}
	}

	return;
}

void edAnmStage::AnimToWRTS()
{
	int uVar1;
	int nodeIndex;
	edF32MATRIX3* puVar3;
	edF32MATRIX4* pMatrixBuffer;
	edANM_RTS_Key_Hdr* pKeyData;
	edF32MATRIX3 animMatrix;
	edANM_RTS pCurrentKeyData;

	ANIMATION_LOG(LogLevel::Verbose, "edAnmStage::AnimToWRTS total keys: 0x{:x}", this->pKeyData->count_0x0);

	uVar1 = this->pKeyData->count_0x0;

	for (pKeyData = (edANM_RTS_Key_Hdr*)(this->pKeyData + 1); (uVar1 = uVar1 - 1, -1 < uVar1 && ((uint)this->field_0x24 <= (uint)pKeyData->keyCount));
		pKeyData = (edANM_RTS_Key_Hdr*)((char*)pKeyData + pKeyData->keyOffset)) {
		if (((pKeyData->flags & 7) != 0) && (nodeIndex = this->anmSkeleton.edAnmSkeleton::NodeIndexFromID(pKeyData->nodeId), nodeIndex != -1)) {
			ANIMATION_LOG(LogLevel::Verbose, "edAnmStage::AnimToWRTS processing: 0x{:x} node index: {}", uVar1, nodeIndex);

			puVar3 = this->pConstantMatrixData + nodeIndex;

			animMatrix = *puVar3;

			pCurrentKeyData = edANM_RTS(pKeyData);
			edAnmTransformCtrl::GetValue(this->animTime, &pCurrentKeyData, &animMatrix);

			ANIMATION_LOG(LogLevel::Verbose, "edAnmStage::AnimToWRTS node index: {} result: {} {} {}", nodeIndex, animMatrix.rowX.ToString(), animMatrix.rowY.ToString(), animMatrix.rowZ.ToString());

			pMatrixBuffer = this->pRelativeTransformMatrixBuffer->matrices + nodeIndex;
			*pMatrixBuffer = animMatrix;
			pMatrixBuffer->da = 1.0f;
		}
	}
	return;
}

void edAnmStage::AnimBlendToWRTS(float param_1)
{
	int iVar1;
	edF32MATRIX3* puVar2;
	edF32MATRIX4* outRotation;
	int uVar2;
	float fVar3;
	float alpha;
	edF32MATRIX3 local_40;
	edANM_RTS local_8;
	edANM_RTS_Key_Hdr* local_4;

	if (0.0001f <= param_1) {	
		uVar2 = this->pKeyData->count_0x0;

		for (local_4 = (edANM_RTS_Key_Hdr*)(this->pKeyData + 1); (uVar2 = uVar2 - 1, -1 < uVar2 && ((uint)this->field_0x24 <= local_4->keyCount));
			local_4 = (edANM_RTS_Key_Hdr*)((char*)local_4 + local_4->keyOffset)) {
			if (((local_4->flags & 7) != 0) && (iVar1 = this->anmSkeleton.NodeIndexFromID(local_4->nodeId), iVar1 != -1)) {
				local_8 = edANM_RTS(local_4);
				local_40 = this->pConstantMatrixData[iVar1];

				edAnmTransformCtrl::GetValue(this->animTime, &local_8, &local_40);

				outRotation = this->pRelativeTransformMatrixBuffer->matrices + iVar1;

				if (outRotation->da == -1.0f) {
					*outRotation = local_40;

					outRotation->da = param_1;
				}
				else {
					fVar3 = outRotation->da + param_1;
					alpha = param_1 / fVar3;
					outRotation->da = fVar3;
					if (0.01f < alpha) {
						edQuatShortestSLERPHard(alpha, &outRotation->rowX, &outRotation->rowX, &local_40.rowX);
						fVar3 = 1.0f - alpha;
						outRotation->rowY = local_40.rowY * alpha + outRotation->rowY * fVar3;

						fVar3 = 1.0f - alpha;
						outRotation->rowZ = local_40.rowZ * alpha + outRotation->rowZ * fVar3;
					}
				}
			}
		}
	}
	return;
}

void CAnimation::Manage(float deltaTime, CActor* pActor, int bHasFlag, int bPlayingAnimation)
{
	ushort uVar1;
	ed_3d_hierarchy_node* p3DHierNode;
	BoneData* pBoneData;
	undefined8 uVar3;
	bool bVar4;
	ulong iVar5;
	edF32MATRIX4* pFrameMatrixData;
	edANM_WRTS* pMatrixBuffer;
	int index;
	//CEventTrack* this_00;
	ed_Chunck* peVar6;
	edANM_WRTS* pWRTS;
	long lVar7;
	float fVar8;
	float fVar9;
	float fVar10;
	float fVar11;
	float fVar12;
	float fVar13;
	float fVar14;
	edANM_SKELETON* peVar15;
	float fVar16;
	float fVar17;
	edF32MATRIX4* peVar18;
	edANM_SKELETON* pSkeleton;
	int* pCurrent;
	int* pBaseStart;
	float* pBaseEnd;
	ushort prevLodBoneCount;
	int* pOtherStart;
	uint uVar25;
	edF32MATRIX4* peVar26;
	float* pOtherEnd;
	float fVar28;
	edAnmSkeleton local_4;

	// HACK (Safety)
	bool bSetSkeleton = false;

	ANIMATION_LOG(LogLevel::Verbose, "CAnimation::Manage {}", pActor->name);

	p3DHierNode = pActor->p3DHierNode;

	if (p3DHierNode != (ed_3d_hierarchy_node*)0x0) {
		peVar15 = (this->anmSkeleton).pTag;
		prevLodBoneCount = 0;

		if (peVar15 != (edANM_SKELETON*)0x0) {
			prevLodBoneCount = peVar15->boneCount;
		}

		if ((bHasFlag == 0) || (bVar4 = UpdateCurSkeleton(pActor), bVar4 == false)) {
#ifdef PLATFORM_WIN
			if ((this->anmSkeleton).pTag == 0x0 && ((bHasFlag != 0) && (((pActor->p3DHierNode->base).flags_0x9e & 0x100) != 0))) {
				Log::GetInstance().ForceFlush();
				IMPLEMENTATION_GUARD_LOG();
				assert(false);
			}
#endif

			TheAnimStage.SetActor(edAnmSkeleton::TheNullOne.pTag);
			pMatrixBuffer = TheAnimManager.AllocWRTSBuffer();
			TheAnimStage.SetDestinationWRTS(pMatrixBuffer, -1);
			TheAnimStage.pFrameMatrixData = (edF32MATRIX4*)0x0;
			this->anmBinMetaAnimator.AnimateDT(deltaTime);
			TheAnimStage.ToonWRTSToGlobalMatrices(1);
			(p3DHierNode->base).pAnimMatrix = (edF32MATRIX4*)0x0;
			(p3DHierNode->base).pShadowAnimMatrix = (edF32MATRIX4*)0x0;
		}
		else {
			pSkeleton = (this->anmSkeleton).pTag;

			assert(pSkeleton);

			ANIMATION_LOG(LogLevel::Verbose, "CAnimation::Manage Using skeleton bone count: (0x{:x} | 0x{:x}) flags: 0x{:x}", prevLodBoneCount, pSkeleton->boneCount, pSkeleton->flags);

			bSetSkeleton = true;

			if ((pSkeleton->flags & 2) == 0) {
				uVar1 = pSkeleton->boneCount;
				iVar5 = (ulong)pSkeleton + ((uint)uVar1 * 0xc + 0x13 & 0xfffffff0) + ((uint)uVar1 * 2 + (uint)uVar1) * 0x10 + (uint)uVar1 * 0x40;
			}
			else {
				uVar1 = pSkeleton->boneCount;
				iVar5 = (ulong)pSkeleton + ((uint)uVar1 * 0xc + 0x13 & 0xfffffff0) + ((uint)uVar1 * 2 + (uint)uVar1) * 0x10 + (uint)uVar1 * 0x10;
			}

			uVar25 = iVar5 - (ulong)pSkeleton;
			if (uVar25 < 0x4001) {
				pSkeleton = (edANM_SKELETON*)edSystemFastRamGetAddr();
				edDmaLoadToFastRam((this->anmSkeleton).pTag, uVar25, pSkeleton);
			}

			TheAnimStage.SetActor(pSkeleton);
			pFrameMatrixData = TheAnimManager.GetMatrixBuffer(pSkeleton->boneCount);
			pMatrixBuffer = TheAnimManager.AllocWRTSBuffer();
			TheAnimStage.SetDestinationWRTS(pMatrixBuffer, -1);

			if ((prevLodBoneCount == ((this->anmSkeleton).pTag)->boneCount) && (this->field_0x28 == 0)) {
				TheAnimStage.pAnimMatrix = (AnimMatrix*)this->pAnimMatrix;
			}
			else {
				TheAnimStage.pAnimMatrix = (AnimMatrix*)0x0;
			}

			TheAnimStage.pFrameMatrixData = pFrameMatrixData;

			if (bPlayingAnimation == 0) {
				this->anmBinMetaAnimator.AnimateDT(deltaTime);

				uint isLooking = pActor->IsLookingAt();
				if (isLooking != 0) {
					IMPLEMENTATION_GUARD(
					pActor->UpdateLookingAt();
					)
				}

				pActor->UpdateAnimEffects();
			}
			else {
				ushort boneCount = pSkeleton->boneCount;
				for (peVar26 = pFrameMatrixData; peVar26 < pFrameMatrixData + boneCount; peVar26 = peVar26 + 1) {
					edF32Matrix4SetIdentityHard(peVar26);
				}
			}

			TheAnimStage.ToonWRTSToGlobalMatrices(1);

			if ((AnimMatrix*)this->pAnimMatrix != (AnimMatrix*)0x0) {
				IMPLEMENTATION_GUARD(
				TheAnimStage.pAnimMatrix = (AnimMatrix*)this->pAnimMatrix;
				edAnmStage::WRTSToPreviousPosture(&TheAnimStage);
				this->field_0x28 = 0;)
			}

			pActor->UpdatePostAnimEffects();

			(p3DHierNode->base).pAnimMatrix = pFrameMatrixData;
			(p3DHierNode->base).pShadowAnimMatrix = (edF32MATRIX4*)0x0;

			local_4 = edAnmSkeleton(pSkeleton);
			for (pBoneData = this->pBoneData; pBoneData != (BoneData*)0x0; pBoneData = pBoneData->pPrev) {
				local_4.UnskinNMatrices(&pBoneData->matrix, pFrameMatrixData, pBoneData->boneNodeIndex, 1);
			}

			iVar5 = 0;
			if (0 < this->nbDisabledBones) {
				do {
					peVar26 = pFrameMatrixData + this->aDisabledBoneData[iVar5].nodeId;
					edF32Matrix4SetIdentityHard(peVar26);
					peVar26->cc = 0.0f;
					iVar5 = iVar5 + 1;
					peVar26->bb = 0.0f;
					peVar26->aa = 0.0f;
				} while (iVar5 < this->nbDisabledBones);
			}
		}

		fVar28 = 0.0f;
		if (((deltaTime != 0.0f) && (((this->anmBinMetaAnimator).aAnimData)->animPlayState == STATE_ANIM_PLAYING)) &&
			((pActor->flags & 0x10000000) == 0)) {
			iVar5 = 0;

			for (uVar25 = this->count_0x2c; uVar25 != 0; uVar25 = uVar25 >> 1) {
				if ((uVar25 & 1) != 0) {
					fVar8 = this->anmBinMetaAnimator.GetLayerAnimTime(iVar5, 0);
					index = this->anmBinMetaAnimator.GetAnimEventTrackID(iVar5);
					if (index != -1) {
						CEventTrack* pTrack = CScene::ptable.g_TrackManager_004516b4->GetTrack(index);
						pTrack->Play(fVar8 * 1000.0f, this->aTrackData[iVar5], 0, pActor);
					}
					this->aTrackData[iVar5] = fVar8 * 1000.0f;
					iVar5 = iVar5 + 1;
				}
			}
		}

		if ((bHasFlag != 0) && (((pActor->p3DHierNode->base).flags_0x9e & 0x100) != 0)) {
			peVar6 = ed3DHierarchyNodeGetSkeletonChunck(pActor->pMeshNode, true);
			peVar6 = peVar6 + 1;

			assert(bSetSkeleton);

			edANM_SKELETON* pOtherSkeleton = (edANM_SKELETON*)peVar6;
			ushort boneCount = pOtherSkeleton->boneCount;

			ANIMATION_LOG(LogLevel::Verbose, "CAnimation::Manage Using skeleton bone count: 0x{:x} flags: 0x{:x}", pOtherSkeleton->boneCount, pOtherSkeleton->flags);

			pWRTS = TheAnimManager.AllocWRTSBuffer();
			TheAnimStage.SetActor(pOtherSkeleton);
			TheAnimStage.SetDestinationWRTS(pWRTS, 0);

			pOtherEnd = (float*)(pOtherSkeleton + pOtherSkeleton->boneCount + 1);
			pBaseEnd = (float*)(pSkeleton + pSkeleton->boneCount + 1);

			pOtherStart = (int*)(pOtherEnd + pOtherSkeleton->boneCount);
			pBaseStart = (int*)(pBaseEnd + pSkeleton->boneCount);

			pFrameMatrixData = TheAnimManager.GetMatrixBuffer((uint)boneCount);
			uVar25 = (uint)boneCount;
			pCurrent = pBaseStart;

			while (uVar25 = uVar25 - 1, -1 < (int)uVar25) {
				int matrixIndex = -1;
				for (; pBaseEnd < (float*)pBaseStart; pBaseEnd = pBaseEnd + 1) {
					if (*pOtherEnd == *pBaseEnd) {
						matrixIndex = *pCurrent;
						pBaseEnd = pBaseEnd + 1;
						pCurrent = pCurrent + 1;
						break;
					}
					if (*pOtherEnd < *pBaseEnd) break;
					pCurrent = pCurrent + 1;
				}
				if (matrixIndex == -1) {
					pWRTS->matrices[*pOtherStart].da = -1.0f;
				}
				else {
					peVar18 = pMatrixBuffer->matrices + matrixIndex;
					peVar26 = pWRTS->matrices + *pOtherStart;

					// Copy matrix.
					*peVar26 = *peVar18;
				}
				pOtherStart = pOtherStart + 1;
				pOtherEnd = pOtherEnd + 1;
			}

			TheAnimStage.pFrameMatrixData = pFrameMatrixData;
			TheAnimStage.ToonWRTSToGlobalMatrices(1);
			TheAnimManager.FreeWRTSBuffer(pWRTS);
			(p3DHierNode->base).pShadowAnimMatrix = pFrameMatrixData;
		}

		TheAnimManager.FreeWRTSBuffer(pMatrixBuffer);
	}
	return;
}

void CAnimation::PlayAnim(CActor* pActor, int animType, int origAnimType)
{
	bool bVar1;
	int index;
	char* peVar3;
	int iVar3;
	uint uVar5;

	ANIMATION_LOG(LogLevel::Verbose, "CAnimation::PlayAnim {} type: 0x{:x} originalType: 0x{:x}", pActor->name, animType, origAnimType);

	if (((((this->anmBinMetaAnimator).aAnimData)->animPlayState == STATE_ANIM_PLAYING) && (animType != this->currentAnimType_0x30)) &&
		(this->currentAnimType_0x30 != -1)) {
		iVar3 = 0;
		for (uVar5 = this->count_0x2c; uVar5 != 0; uVar5 = uVar5 >> 1) {
			if ((uVar5 & 1) != 0) {
				index = this->anmBinMetaAnimator.GetAnimEventTrackID(iVar3);
				if (index != -1) {
					CEventTrack* pTrack = CScene::ptable.g_TrackManager_004516b4->GetTrack(index);
					pTrack->Stop();
				}
				iVar3 = iVar3 + 1;
			}
		}
	}

	this->anmBinMetaAnimator.SetAnim(animType, origAnimType);
	this->currentAnimType_0x30 = animType;

	for (int i = 0; i < 4; i++) {
		this->aTrackData[i] = 0.0f;
	}

	return;
}

void edAnmMetaAnimator::SetLayerProperties(int count)
{
	this->layerCount = count;
	if (0 < count) {
		do {
			count = count + -1;
			this->aAnimData[count].Reset();
		} while (0 < count);
	}
	return;
}

void edAnmMetaAnimator::SetLayerResourceArray(int index, edANM_HDR** pAnimManagerKeyData)
{
	int iVar2;

	/* Set all. */
	if (index == -1) {
		iVar2 = 0;
		if (0 < this->layerCount) {
			do {
				this->aAnimData[iVar2].pAnimManagerKeyData = pAnimManagerKeyData;
				iVar2 = iVar2 + 1;
			} while (iVar2 < this->layerCount);
		}
	}
	else {
		this->aAnimData[index].pAnimManagerKeyData = pAnimManagerKeyData;
	}
	return;
}

void edAnmBinMetaAnimator::SetAnimStatesBinary(void* pAnimKeyEntryData)
{
	if (pAnimKeyEntryData != (void*)0x0) {
		this->pAnimKeyEntryData = (int*)pAnimKeyEntryData;
	}
	return;
}

void edAnmBinMetaAnimator::SetLayerMacroAnimCallback(int index, AnimationCallback callback)
{
	int iVar1;
	edAnmLayer* peVar2;

	if (index == -1) {
		peVar2 = this->aAnimData;
		iVar1 = 0;
		if (0 < this->layerCount) {
			do {
				peVar2->pFunction_0xc0 = callback;
				iVar1 = iVar1 + 1;
				peVar2 = peVar2 + 1;
			} while (iVar1 < this->layerCount);
		}
	}
	else {
		this->aAnimData[index].pFunction_0xc0 = callback;
	}
	return;
}

void edAnmBinMetaAnimator::SetLayerMacroAnimUserParams(int index, CActor* pActor)
{
	int iVar1;
	edAnmLayer* peVar2;

	if (index == -1) {
		peVar2 = this->aAnimData;
		iVar1 = 0;
		if (0 < this->layerCount) {
			do {
				peVar2->pActor = pActor;
				iVar1 = iVar1 + 1;
				peVar2 = peVar2 + 1;
			} while (iVar1 < this->layerCount);
		}
	}
	else {
		this->aAnimData[index].pActor = pActor;
	}
	return;
}

void edAnmBinMetaAnimator::SetLayerTimeWarper(float value, int index)
{
	int iVar1;
	edAnmLayer* peVar2;

	if (index == -1) {
		peVar2 = this->aAnimData;
		iVar1 = 0;
		if (0 < this->layerCount) {
			do {
				iVar1 = iVar1 + 1;
				peVar2->playRate = value;
				peVar2 = peVar2 + 1;
			} while (iVar1 < this->layerCount);
		}
	}
	else {
		this->aAnimData[index].playRate = value;
	}
	return;
}

struct edAnmStateParser
{
	edAnmStateParser() {}

	edAnmStateParser(int* pInData) : pData(pInData) {}

	int* pData;
	edAnmStateDesc* BuildDesc(edAnmStateDesc* pAnimation, int animType, int origAnimType);
};

edAnmStateDesc* edAnmStateParser::BuildDesc(edAnmStateDesc* pAnimation, int animType, int origAnimType)
{
	int* pCurData;

	pAnimation->animType = animType;
	pAnimation->flags = 0;
	pAnimation->field_0x0 = 0;
	pAnimation->maxKey_0x10 = (*this->pData >> 2) + -1;
	pAnimation->pHdrB = (edANM_HDR*)(reinterpret_cast<char*>(this->pData) + *this->pData);

	pCurData = this->pData;
	pAnimation->pHdrA = (edANM_HDR*)(reinterpret_cast<char*>(pCurData) + pCurData[(*pCurData >> 2) + -1]);

	pAnimation->origAnimType = origAnimType;
	return pAnimation;
}

void edAnmBinMetaAnimator::SetAnim(int animType, int origAnimType)
{
	edAnmLayer* pLayer;
	edAnmStateDesc NewAnimation;
	edAnmStateParser anmStateParser;

	anmStateParser = edAnmStateParser(reinterpret_cast<int*>(reinterpret_cast<char*>(this->pAnimKeyEntryData) + this->pAnimKeyEntryData[animType] + 4));
	anmStateParser.BuildDesc(&NewAnimation, animType, origAnimType);

	pLayer = this->aAnimData;
	pLayer->SetAnim(&NewAnimation);
	return;
}

int edAnmBinMetaAnimator::GetAnimType_00242330(int animIndex)
{
	return this->aAnimData[animIndex].animPlayState;
}

float edAnmMetaAnimator::GetLayerAnimTime(int animIndex, int param_3)
{
	edAnmLayer* pLayer;
	float fVar2;

	pLayer = this->aAnimData + animIndex;
	if (pLayer->animPlayState == STATE_ANIM_NONE) {
		fVar2 = 0.0f;
	}
	else {
		if (param_3 == 0) {
			fVar2 = (pLayer->currentAnimDesc).state.time_0x10;
		}
		else {
			fVar2 = (pLayer->currentAnimDesc).state.time_0xc;
		}
	}

	return fVar2;
}

int edAnmBinMetaAnimator::GetAnimEventTrackID(int index)
{
	int trackId;
	int* pTrackData;
	edAnmLayer* pLayer;
	int animType;

	pLayer = this->aAnimData + index;
	trackId = -1;
	if (pLayer->animPlayState != STATE_ANIM_NONE) {
		animType = (pLayer->currentAnimDesc).animType;
		trackId = -1;
		if ((animType & 0x80000000) == 0) {
			pTrackData = (int*)((char*)this->pAnimKeyEntryData + this->pAnimKeyEntryData[animType]);
			if (pTrackData == (int*)0x0) {
				trackId = -1;
			}
			else {
				trackId = *pTrackData;
			}
		}
	}
	return trackId;
}

void edAnmBinMetaAnimator::SetLayerAnimTime(float time, int index, byte param_4)
{
	int iVar1;
	edAnmLayer* peVar2;
	float local_20[4];
	float local_10[4];

	if (index == -1) {
		peVar2 = this->aAnimData;

		iVar1 = 0;
		if (0 < this->layerCount) {
			do {
				if (peVar2->animPlayState == 1) {
					edAnmStage::ComputeAnimParams(time, (peVar2->currentAnimDesc).state.keyStartTime_0x14, 0.0f, local_10, param_4 != 0,
						(uint)(((peVar2->currentAnimDesc).state.currentAnimDataFlags & 1) != 0));

					(peVar2->currentAnimDesc).state.time_0x10 = local_10[0];
					(peVar2->currentAnimDesc).state.time_0xc = local_10[1];
				}

				iVar1 = iVar1 + 1;
				peVar2 = peVar2 + 1;
			} while (iVar1 < this->layerCount);
		}
	}
	else {
		peVar2 = this->aAnimData + index;
		if (peVar2->animPlayState == 1) {
			edAnmStage::ComputeAnimParams(time, (peVar2->currentAnimDesc).state.keyStartTime_0x14, 0.0f, local_20, param_4 != 0,
				(uint)(((peVar2->currentAnimDesc).state.currentAnimDataFlags & 1) != 0));

			(peVar2->currentAnimDesc).state.time_0x10 = local_20[0];
			(peVar2->currentAnimDesc).state.time_0xc = local_20[1];
		}
	}

	return;
}

void edAnmBinMetaAnimator::SetLayerBlendingOp(int layerIndex, int op)
{
	int iVar1;
	edAnmLayer* pLayer;

	if (layerIndex == -1) {
		pLayer = this->aAnimData;

		iVar1 = 0;
		if (0 < this->layerCount) {
			do {
				pLayer->blendOp = op;
				iVar1 = iVar1 + 1;
				pLayer = pLayer + 1;
			} while (iVar1 < this->layerCount);
		}
	}
	else {
		this->aAnimData[layerIndex].blendOp = op;
	}

	return;
}

void edAnmBinMetaAnimator::SetAnimOnLayer(int mode, int layerIndex, int param_4)
{
	edAnmLayer* pLayer;
	edAnmStateDesc anmStateDesc;
	edAnmStateParser anmStateParser;

	pLayer = this->aAnimData + layerIndex;
	if (mode == -1) {
		pLayer->animPlayState = 0;
		(pLayer->nextAnimDesc).animType = -1;
		(pLayer->currentAnimDesc).animType = -1;
	}
	else {
		int* piVar1 = this->pAnimKeyEntryData;
		anmStateParser = (int*)(reinterpret_cast<char*>(piVar1) + piVar1[mode] + 4);
		anmStateParser.BuildDesc(&anmStateDesc, mode, param_4);
		pLayer->SetAnim(&anmStateDesc);
	}

	return;
}

void edAnmMetaAnimator::AnimateDT(float deltaTime)
{
	int index;

	ANIMATION_LOG(LogLevel::Verbose, "edAnmMetaAnimator::AnimateDT layerCount: {}", this->layerCount);

	index = 0;
	if (0 < this->layerCount) {
		do {
			this->aAnimData[index].AnimateDT(deltaTime);
			index = index + 1;
		} while (index < this->layerCount);
	}
	return;
}

uint flags;
uint currentAnimDataFlags;
undefined4 field_0x8;
float time_0xc;
float time_0x10;
float keyStartTime_0x14;

float edAnmMetaAnimator::GetAnimStateDescLength(edAnmStateDesc* pOutDesc, int lengthMode)
{
	edAnmLayer* peVar1;
	uint flags;
	edAnmMacroAnimator eStack48;

	eStack48.animationType = pOutDesc->animType;
	peVar1 = this->aAnimData + pOutDesc->field_0x0;
	eStack48.pActor = peVar1->pActor;
	eStack48.pFunction = peVar1->pFunction_0xc0;
	eStack48.pKeyDataArray = peVar1->pAnimManagerKeyData;
	flags = pOutDesc->pHdrA->count_0x0;

	if (lengthMode != 0) {
		if (lengthMode == 1) {
			flags = flags | 1;
		}
		else {
			if (lengthMode == 2) {
				flags = flags & 0xfffffffe;
			}
		}
		flags = flags | 0x40000000;
	}

	eStack48.Initialize(0.0f, pOutDesc->pHdrA, false, flags);
	return eStack48.keyStartTime_0x14;
}

void edAnmLayer::Reset()
{
	this->blendOp = 0;
	this->animPlayState = STATE_ANIM_NONE;
	this->pFunction_0xc0 = 0x0;
	this->nextAnimDesc.animType = -1;
	this->currentAnimDesc.animType = -1;
	this->field_0xd4 = 0.0f;
	this->playRate = 1.0f;
	return;
}

void edAnmLayer::AnimateDT(float deltaTime)
{
	bool bComplete;
	float playTime;

	this->field_0xcc = 0;
	playTime = deltaTime * this->playRate;

	do {
		ANIMATION_LOG(LogLevel::Verbose, "edAnmMetaAnimator::AnimateDT state: {} play rate: {}", this->animPlayState, this->playRate);
		bComplete = true;
		if (this->animPlayState == STATE_ANIM_MORPHING) {
			bComplete = MorphingDT(playTime);
		}
		else {
			if (this->animPlayState == STATE_ANIM_PLAYING) {
				bComplete = PlayingDT(playTime);
			}
		}
	} while (bComplete == false);

	return;
}

bool edAnmLayer::MorphingDT(float playTime)
{
	int iVar1;
	edANM_WRTS* peVar2;
	edANM_WRTS* pMatrixBuffer;
	edANM_WRTS* pMatrixBuffer_00;
	edANM_WRTS* unaff_s3_lo;
	edANM_WRTS* unaff_s4_lo;
	float fVar3;
	float fVar4;
	float fVar5;
	float fVar6;

	ANIMATION_LOG(LogLevel::Verbose, "edAnmLayer::MorphingDT playTime: {}", playTime);

	if ((this->nextAnimDesc.animMode == -6) && (TheAnimStage.pAnimMatrix == (AnimMatrix*)0x0)) {
		fVar6 = this->nextAnimDesc.state.time_0x10;
		this->currentAnimDesc.state.playingDuration = 0.0f;
		this->field_0xc = this->currentAnimDesc.animMode;
		this->currentAnimDesc.animType = this->nextAnimDesc.animType;
		this->currentAnimDesc.pHdrA = this->nextAnimDesc.pHdrA;
		this->currentAnimDesc.flags = this->nextAnimDesc.flags;
		this->currentAnimDesc.maxKey_0xc = this->nextAnimDesc.maxKey_0xc;
		this->currentAnimDesc.pHdrB = this->nextAnimDesc.pHdrB;
		this->currentAnimDesc.origAnimType = this->nextAnimDesc.origAnimType;
		this->currentAnimDesc.state.pFunction = this->pFunction_0xc0;
		this->currentAnimDesc.state.pActor = this->pActor;
		this->currentAnimDesc.state.animationType = this->nextAnimDesc.origAnimType;
		this->currentAnimDesc.state.pKeyDataArray = this->pAnimManagerKeyData;
		this->currentAnimDesc.state.Initialize(fVar6, this->nextAnimDesc.pHdrA, false, this->nextAnimDesc.flags);
		this->nextAnimDesc.animType = -1;
		this->animPlayState = STATE_ANIM_PLAYING;
		return false;
	}

	if ((this->currentAnimDesc.state.currentAnimDataFlags & 2) == 0) {
		this->currentAnimDesc.state.UpdateAnimParams();
	}

	if ((this->nextAnimDesc.state.currentAnimDataFlags & 2) == 0) {
		this->nextAnimDesc.state.UpdateAnimParams();
	}

	if (this->field_0xbc == -1.0f) {
		this->currentAnimDesc.morphDuration = this->nextAnimDesc.state.keyDuration_0x18;
	}
	else {
		this->currentAnimDesc.morphDuration = this->field_0xbc;
	}

	fVar6 = this->nextAnimDesc.morphDuration + playTime;
	this->nextAnimDesc.morphDuration = fVar6;

	peVar2 = TheAnimStage.pRelativeTransformMatrixBuffer;

	if (this->currentAnimDesc.morphDuration <= fVar6) {
		fVar6 = this->nextAnimDesc.state.time_0x10;
		this->currentAnimDesc.state.playingDuration = 0.0f;
		this->field_0xc = this->currentAnimDesc.animMode;
		this->currentAnimDesc.animType = this->nextAnimDesc.animType;
		this->currentAnimDesc.pHdrA = this->nextAnimDesc.pHdrA;
		this->currentAnimDesc.flags = this->nextAnimDesc.flags;
		this->currentAnimDesc.maxKey_0xc = this->nextAnimDesc.maxKey_0xc;
		this->currentAnimDesc.pHdrB = this->nextAnimDesc.pHdrB;
		this->currentAnimDesc.origAnimType = this->nextAnimDesc.origAnimType;
		this->currentAnimDesc.state.pFunction = this->pFunction_0xc0;
		this->currentAnimDesc.state.pActor = this->pActor;
		this->currentAnimDesc.state.animationType = this->nextAnimDesc.origAnimType;
		this->currentAnimDesc.state.pKeyDataArray = this->pAnimManagerKeyData;
		this->currentAnimDesc.state.Initialize(fVar6, this->nextAnimDesc.pHdrA, false, this->nextAnimDesc.flags);
		this->nextAnimDesc.animType = -1;
		this->animPlayState = STATE_ANIM_PLAYING;
		return false;
	}

	fVar6 = this->field_0xd4 * 65535.0f;
	if (fVar6 < 2.147484e+09f) {
		TheAnimStage.field_0x24 = (int)fVar6;
	}
	else {
		TheAnimStage.field_0x24 = (int)(fVar6 - 2.147484e+09f) | 0x80000000;
	}

	fVar6 = this->field_0xbc;
	if (fVar6 == -1.0f) {
		fVar6 = this->nextAnimDesc.state.keyDuration_0x18;
		if (0.0f < fVar6) {
			fVar6 = this->nextAnimDesc.morphDuration / fVar6;
		}
		else {
			fVar6 = 0.0f;
		}
	}
	else {
		if (0.0f < fVar6) {
			fVar6 = this->nextAnimDesc.morphDuration / fVar6;
		}
		else {
			fVar6 = 0.0f;
		}
	}

	if (this->blendOp != 0) {
		unaff_s3_lo = TheAnimManager.AllocWRTSBuffer();
		TheAnimStage.SetDestinationWRTS(unaff_s3_lo, -1);
		unaff_s4_lo = peVar2;
	}

	peVar2 = TheAnimStage.pRelativeTransformMatrixBuffer;
	pMatrixBuffer = TheAnimManager.AllocWRTSBuffer();
	pMatrixBuffer_00 = TheAnimManager.AllocWRTSBuffer();
	TheAnimStage.SetDestinationWRTS(pMatrixBuffer, -1);

	switch (this->nextAnimDesc.animMode) {
	default:
		IMPLEMENTATION_GUARD(this->currentAnimDesc.state.AnimateIT(0.0f));
		break;
	case -6:
		IMPLEMENTATION_GUARD(
		TheAnimStage.PreviousPostureToWRTS(1.0f));
		break;
	case -5:
		if (TheAnimStage.pAnimMatrix != (AnimMatrix*)0x0) {
			IMPLEMENTATION_GUARD(
			TheAnimStage.PreviousPostureToWRTS(1.0f);
			if (0.0f < fVar6) {
				fVar4 = 0.0f;
				fVar5 = this->nextAnimDesc.morphDuration - playTime;
				if (fVar5 < 0.0f) {
					fVar5 = 0.0f;
				}
				fVar3 = this->field_0xbc;
				if (fVar3 == -1.0f) {
					fVar3 = this->nextAnimDesc.state.keyDuration_0x18;
					if (0.0f < fVar3) {
						fVar4 = fVar5 / fVar3;
					}
				}
				else {
					if (0.0f < fVar3) {
						fVar4 = fVar5 / fVar3;
					}
				}
				if (fVar4 < 1.0f) {
					fVar6 = (fVar6 - fVar4) / (1.0f - fVar4);
				}
			})
			break;
		}
	case -4:
	case -2:
		this->currentAnimDesc.state.AnimateDT(0.0f);
		break;
	case -3:
		fVar5 = this->currentAnimDesc.state.keyStartTime_0x14;
		fVar4 = this->nextAnimDesc.state.keyStartTime_0x14 * fVar6 + fVar5 * (1.0f - fVar6);
		if (fVar4 <= 0.0f) {
			fVar4 = 0.0f;
		}
		else {
			fVar4 = playTime / fVar4;
		}
		this->currentAnimDesc.state.AnimateDT(fVar4 * fVar5);
		break;
	case -1:
		this->currentAnimDesc.state.AnimateDT(playTime);
	}

	TheAnimStage.SetDestinationWRTS(pMatrixBuffer_00, -1);

	iVar1 = this->field_0xb8;
	if (iVar1 == -1) {
		this->nextAnimDesc.state.AnimateDT(playTime);
	}
	else {
		if ((iVar1 == -3) || (iVar1 == -10)) {
			IMPLEMENTATION_GUARD(
			edAnmMacroAnimator::AnimateIR(this->currentAnimDesc.state.time_0xc);)
		}
		else {
			this->nextAnimDesc.state.AnimateDT(0.0f);
		}
	}

	TheAnimStage.SetDestinationWRTS(peVar2, 0);
	TheAnimStage.BlendToDestWRTS(fVar6, pMatrixBuffer->matrices, pMatrixBuffer_00->matrices);

	TheAnimManager.FreeWRTSBuffer(pMatrixBuffer);
	TheAnimManager.FreeWRTSBuffer(pMatrixBuffer_00);

	if (this->blendOp != 0) {
		IMPLEMENTATION_GUARD(
		TheAnimStage.SetDestinationWRTS(unaff_s4_lo, 0);
		iVar1 = this->field_0x0;
		if (iVar1 == 1) {
			TheAnimStage.SetSingleToDestWRTS((edF32MATRIX4*)unaff_s3_lo);
		}
		else {
			if (iVar1 == 2) {
				TheAnimStage.BlendWithDestWRTS(0.5, (edF32MATRIX4*)unaff_s3_lo);
			}
			else {
				if (iVar1 == 3) {
					TheAnimStage.BlendWithDestWRTS(this->field_0x4, (edF32MATRIX4*)unaff_s3_lo);
				}
			}
		}
		TheAnimManager.FreeWRTSBuffer(unaff_s3_lo);)
	}

	return true;
}

bool edAnmLayer::PlayingDT(float playTime)
{
	int iVar1;
	uint stateFlags;
	edANM_WRTS* pWRTS;
	bool bComplete;
	edANM_WRTS* pMatrixBuffer;
	edANM_WRTS* unaff_s1_lo;
	float fVar4;

	ANIMATION_LOG(LogLevel::Verbose, "edAnmLayer::PlayingDT playTime: {}", playTime);

	this->currentAnimDesc.state.playingDuration = this->currentAnimDesc.state.playingDuration + playTime;

	if ((this->nextAnimDesc.animType == -1) || (iVar1 = this->nextAnimDesc.animMode, iVar1 == -4)) {
		if ((this->currentAnimDesc.state.currentAnimDataFlags & 2) == 0) {
			this->currentAnimDesc.state.UpdateAnimParams();
		}

		pWRTS = TheAnimStage.pRelativeTransformMatrixBuffer;
		if (this->blendOp != 0) {
			pMatrixBuffer = TheAnimManager.AllocWRTSBuffer();
			TheAnimStage.SetDestinationWRTS(pMatrixBuffer, -1);
			unaff_s1_lo = pWRTS;
		}

		fVar4 = this->field_0xd4 * 65535.0f;
		if (fVar4 < 2.147484e+09f) {
			TheAnimStage.field_0x24 = (int)fVar4;
		}
		else {
			TheAnimStage.field_0x24 = (int)(fVar4 - 2.147484e+09f) | 0x80000000;
		}

		this->currentAnimDesc.state.AnimateDT(playTime);

		stateFlags = this->currentAnimDesc.state.flags;
		if (stateFlags != 0) {
			if ((((stateFlags & 0x80000000) != 0) && (this->field_0xcc = this->field_0xcc | 2, this->nextAnimDesc.animType != -1)) && (this->nextAnimDesc.animMode == -4)) {
				this->currentAnimDesc.state.Initialize(this->currentAnimDesc.state.keyStartTime_0x14, this->currentAnimDesc.pHdrA, false, this->currentAnimDesc.flags);

				fVar4 = 0.0f;
				if (-1 < this->field_0xb8) {
					fVar4 = (float)this->field_0xb8 / 1000.0f;
				}

				this->nextAnimDesc.state.Initialize(fVar4, this->nextAnimDesc.pHdrA, true, this->nextAnimDesc.flags);
				this->animPlayState = STATE_ANIM_MORPHING;
				this->nextAnimDesc.morphDuration = 0.0f;
			}

			if ((this->currentAnimDesc.state.flags & 0x40000000) != 0) {
				this->field_0xcc = this->field_0xcc | 4;
			}

			if ((this->currentAnimDesc.state.flags & 0x20000000) != 0) {
				this->field_0xcc = this->field_0xcc | 8;
			}
		}

		if (this->blendOp != 0) {
			TheAnimStage.SetDestinationWRTS(unaff_s1_lo, 0);
			iVar1 = this->blendOp;
			if (iVar1 == 1) {
				TheAnimStage.SetSingleToDestWRTS(pMatrixBuffer->matrices);
			}
			else {
				if (iVar1 == 2) {
					IMPLEMENTATION_GUARD(
					TheAnimStage.BlendWithDestWRTS(0.5, (edF32MATRIX4*)pMatrixBuffer);)
				}
				else {
					if (iVar1 == 3) {
						TheAnimStage.BlendWithDestWRTS(this->field_0x4, pMatrixBuffer->matrices);
					}
				}
			}

			TheAnimManager.FreeWRTSBuffer(pMatrixBuffer);
		}

		bComplete = true;
	}
	else {
		if (iVar1 < 0) {
			if (iVar1 == -4) {
				this->currentAnimDesc.state.Initialize(this->currentAnimDesc.state.keyStartTime_0x14,this->currentAnimDesc.pHdrA, false, this->currentAnimDesc.flags);
			}
		}
		else {
			this->currentAnimDesc.state.Initialize((float)iVar1 / 1000.0f, this->currentAnimDesc.pHdrA, true, this->currentAnimDesc.flags);
		}

		fVar4 = 0.0f;
		if (-1 < this->field_0xb8) {
			fVar4 = (float)this->field_0xb8 / 1000.0f;
		}

		this->nextAnimDesc.state.Initialize(fVar4, this->nextAnimDesc.pHdrA, true, this->nextAnimDesc.flags);

		bComplete = false;
		this->animPlayState = STATE_ANIM_MORPHING;
		this->nextAnimDesc.morphDuration = 0.0f;
	}

	return bComplete;
}

void edAnmLayer::SetRawAnim(edANM_HDR* pHdr, uint flags, uint type)
{
	int curAnimPlayState;
	edAnmStateDesc anmStateDesc;

	char headerBuff[(sizeof(edANM_HDR) + (sizeof(float) * 2))];
	edANM_HDR* hdr = reinterpret_cast<edANM_HDR*>(headerBuff);

	hdr->keyIndex_0x8.asKey = -1;
	hdr->pData[1] = 0.0f;
	hdr->count_0x0 = -2;
	hdr->pData[0] = -1.0f;
	hdr->field_0x4.asKey = -2;

	assert(reinterpret_cast<char*>(&hdr->pData[1]) < headerBuff + sizeof(headerBuff));

	if (type == 0xfffffffe) {
		type = (uint)pHdr | 0x80000000;
	}

	anmStateDesc.flags = flags | 0x80000000;
	anmStateDesc.pHdrB = hdr;
	anmStateDesc.field_0x0 = -1;
	anmStateDesc.origAnimType = -1;
	anmStateDesc.maxKey_0x10 = 1;
	anmStateDesc.animType = type;
	anmStateDesc.pHdrA = pHdr;

	curAnimPlayState = this->animPlayState;

	if (curAnimPlayState == STATE_ANIM_PLAYING) {
		if (type == this->currentAnimDesc.animType) {
			if (this->nextAnimDesc.animType != -1) {
				this->nextAnimDesc.animType = -1;
			}
		}
		else {
			MorphingInitDT(&anmStateDesc);
		}
	}
	else {
		if (curAnimPlayState == STATE_ANIM_NONE) {
			this->currentAnimDesc.state.playingDuration = 0.0f;
			this->field_0xc = 0xffffffff;
			this->currentAnimDesc.animType = type;
			this->currentAnimDesc.pHdrA = pHdr;
			this->currentAnimDesc.flags = anmStateDesc.flags;
			this->currentAnimDesc.maxKey_0xc = 1;
			this->currentAnimDesc.pHdrB = anmStateDesc.pHdrB;
			this->currentAnimDesc.origAnimType = -1;
			this->currentAnimDesc.state.pFunction = this->pFunction_0xc0;
			this->currentAnimDesc.state.pActor = this->pActor;
			this->currentAnimDesc.state.animationType = -1;
			this->currentAnimDesc.state.pKeyDataArray = this->pAnimManagerKeyData;
			this->currentAnimDesc.state.Initialize(0.0f, pHdr, 0, anmStateDesc.flags);
			this->nextAnimDesc.animType = -1;
			this->animPlayState = STATE_ANIM_PLAYING;
		}
		else {
			if (curAnimPlayState == STATE_ANIM_MORPHING) {
				MorphingInitDT(&anmStateDesc);
			}
		}
	}
	return;
}

bool edAnmLayer::MorphingInitDT(edAnmStateDesc* pNewAnimation)
{
	bool bShouldInitialize;
	edANM_HDR* peVar2;
	int iVar3;
	edAnmStage* peVar4;
	float fVar5;
	int AVar1;

	bShouldInitialize = false;
	if (this->animPlayState == STATE_ANIM_MORPHING) {
		if (pNewAnimation->animType == this->nextAnimDesc.animType) {
			return true;
		}
		fVar5 = this->nextAnimDesc.state.time_0x10;
		this->currentAnimDesc.state.playingDuration = 0.0f;
		this->field_0xc = this->currentAnimDesc.animMode;
		this->currentAnimDesc.animType = this->nextAnimDesc.animType;
		this->currentAnimDesc.pHdrA = this->nextAnimDesc.pHdrA;
		this->currentAnimDesc.flags = this->nextAnimDesc.flags;
		this->currentAnimDesc.maxKey_0xc = this->nextAnimDesc.maxKey_0xc;
		this->currentAnimDesc.pHdrB = this->nextAnimDesc.pHdrB;
		this->currentAnimDesc.origAnimType = this->nextAnimDesc.origAnimType;
		this->currentAnimDesc.state.pFunction = this->pFunction_0xc0;
		this->currentAnimDesc.state.pActor = this->pActor;
		this->currentAnimDesc.state.animationType = this->nextAnimDesc.origAnimType;
		this->currentAnimDesc.state.pKeyDataArray = this->pAnimManagerKeyData;
		this->currentAnimDesc.state.Initialize(fVar5, this->nextAnimDesc.pHdrA, false, this->nextAnimDesc.flags);
		this->nextAnimDesc.animType = -1;
		bShouldInitialize = true;
		this->animPlayState = STATE_ANIM_PLAYING;
	}

	iVar3 = this->currentAnimDesc.maxKey_0xc;

	peVar4 = &TheAnimStage;
	if ((iVar3 != 0) && (peVar2 = this->currentAnimDesc.pHdrB, iVar3 != 0)) {
		do {
			AVar1 = peVar2->count_0x0;
			if (AVar1 == pNewAnimation->animType) {
				peVar4 = (edAnmStage*)&peVar2->field_0x4;
				break;
			}
			if ((AVar1 == -2) || (AVar1 == -1)) {
				peVar4 = (edAnmStage*)&peVar2->field_0x4;
			}
			iVar3 = iVar3 + -1;
			peVar2 = (edANM_HDR*)&peVar2[1].keyIndex_0x8;
		} while (iVar3 != 0);
	}

	this->nextAnimDesc.animMode = peVar4->animMode;
	this->field_0xb8 = peVar4->field_0x4;
	this->field_0xbc = peVar4->field_0x8;
	this->currentAnimDesc.animMode = pNewAnimation->field_0x0;

	// Fill out next anim description
	this->nextAnimDesc.animType = pNewAnimation->animType;
	this->nextAnimDesc.pHdrA = pNewAnimation->pHdrA;
	this->nextAnimDesc.flags = pNewAnimation->flags;
	this->nextAnimDesc.maxKey_0xc = pNewAnimation->maxKey_0x10;
	this->nextAnimDesc.pHdrB = pNewAnimation->pHdrB;
	this->nextAnimDesc.origAnimType = pNewAnimation->origAnimType;
	this->nextAnimDesc.state.pFunction = this->pFunction_0xc0;
	this->nextAnimDesc.state.pActor = this->pActor;
	this->nextAnimDesc.state.animationType = pNewAnimation->origAnimType;
	this->nextAnimDesc.state.pKeyDataArray = this->pAnimManagerKeyData;
	this->nextAnimDesc.state.playingDuration = 0.0f;

	if (bShouldInitialize) {
		this->nextAnimDesc.animMode = -6;
		iVar3 = this->nextAnimDesc.animMode;
		if (iVar3 < 0) {
			if (iVar3 == -4) {
				this->currentAnimDesc.state.Initialize(this->currentAnimDesc.state.keyStartTime_0x14, this->currentAnimDesc.pHdrA, false, this->currentAnimDesc.flags);
			}
		}
		else {
			this->currentAnimDesc.state.Initialize((float)iVar3 / 1000.0f, this->currentAnimDesc.pHdrA, true, this->currentAnimDesc.flags);
		}

		fVar5 = 0.0f;
		if (-1 < this->field_0xb8) {
			fVar5 = (float)this->field_0xb8 / 1000.0f;
		}

		this->currentAnimDesc.state.Initialize(fVar5, this->nextAnimDesc.pHdrA, true, this->nextAnimDesc.flags);
		this->animPlayState = STATE_ANIM_MORPHING;
		this->nextAnimDesc.morphDuration = 0.0f;
	}

	return true;
}

bool edAnmLayer::MorphingStartDT()
{
	int iVar1;
	float fVar2;

	iVar1 = this->nextAnimDesc.animMode;
	if (iVar1 < 0) {
		if (iVar1 == -4) {
			this->currentAnimDesc.state.Initialize
			(this->currentAnimDesc.state.keyStartTime_0x14,
				this->currentAnimDesc.pHdrA, false, this->currentAnimDesc.flags);
		}
	}
	else {
		this->currentAnimDesc.state.Initialize
		((float)iVar1 / 1000.0f, this->currentAnimDesc.pHdrA, true,
			this->currentAnimDesc.flags);
	}

	fVar2 = 0.0f;
	if (-1 < this->field_0xb8) {
		fVar2 = (float)this->field_0xb8 / 1000.0f;
	}

	this->nextAnimDesc.state.Initialize(fVar2, this->nextAnimDesc.pHdrA, true, this->nextAnimDesc.flags);
	this->animPlayState = STATE_ANIM_MORPHING;
	this->nextAnimDesc.morphDuration = 0.0f;
	return true;
}

void edAnmLayer::SetAnim(edAnmStateDesc* pDesc)
{
	int curAnimPlayState = this->animPlayState;

	if (curAnimPlayState == STATE_ANIM_PLAYING) {
		if (pDesc->animType == this->currentAnimDesc.animType) {
			if (this->nextAnimDesc.animType != -1) {
				this->nextAnimDesc.animType = -1;
			}
		}
		else {
			/*  Queue up anim to play next. */
			MorphingInitDT(pDesc);
		}
	}
	else {
		if (curAnimPlayState == STATE_ANIM_NONE) {
			/* Not already playing an anim, play straight away. */
			this->currentAnimDesc.state.playingDuration = 0.0f;
			this->field_0xc = pDesc->field_0x0;
			this->currentAnimDesc.animType = pDesc->animType;
			this->currentAnimDesc.pHdrA = pDesc->pHdrA;
			this->currentAnimDesc.flags = pDesc->flags;
			this->currentAnimDesc.maxKey_0xc = pDesc->maxKey_0x10;
			this->currentAnimDesc.pHdrB = pDesc->pHdrB;
			this->currentAnimDesc.origAnimType = pDesc->origAnimType;
			this->currentAnimDesc.state.pFunction = this->pFunction_0xc0;
			this->currentAnimDesc.state.pActor = this->pActor;
			this->currentAnimDesc.state.animationType = pDesc->origAnimType;
			this->currentAnimDesc.state.pKeyDataArray = this->pAnimManagerKeyData;
			this->currentAnimDesc.state.Initialize(0, pDesc->pHdrA, false, pDesc->flags);
			this->nextAnimDesc.animType = -1;
			this->animPlayState = STATE_ANIM_PLAYING;
		}
		else {
			if (curAnimPlayState == STATE_ANIM_MORPHING) {
				/* Queue up anim to play next. */
				MorphingInitDT(pDesc);
			}
		}
	}
	return;
}

void CAnimation::Reset(CActor* pActor)
{
	int iVar1;
	//CEventTrack* pCVar2;
	int iVar3;
	uint uVar5;

	if (((this->anmBinMetaAnimator).aAnimData)->animPlayState == STATE_ANIM_PLAYING) {
		iVar3 = 0;
		for (uVar5 = this->count_0x2c; uVar5 != 0; uVar5 = uVar5 >> 1) {
			if ((uVar5 & 1) != 0) {
				iVar1 = this->anmBinMetaAnimator.GetAnimEventTrackID(iVar3);
				if (iVar1 != -1) {
					CEventTrack* pTrack = CScene::ptable.g_TrackManager_004516b4->GetTrack(iVar1);
					pTrack->Reset();
				}
				iVar3 = iVar3 + 1;
			}
		}
	}
	iVar3 = (this->anmBinMetaAnimator).layerCount;
	if (iVar3 != 0) {
		do {
			iVar3 = iVar3 + -1;

			this->anmBinMetaAnimator.aAnimData[iVar3].Reset();

			this->anmBinMetaAnimator.SetLayerMacroAnimCallback(iVar3, CAnimationManager::_gLayersCallbacks[iVar3]);
			this->anmBinMetaAnimator.SetLayerMacroAnimUserParams(iVar3, pActor);
		} while (iVar3 != 0);
	}
	return;
}

BoneData* CAnimation::FindReggedBone(int bone)
{
	BoneData* pAVar1;

	for (pAVar1 = this->pBoneData; (pAVar1 != (BoneData*)0x0 && (bone != pAVar1->boneId));
		pAVar1 = pAVar1->pPrev) {
	}
	return pAVar1;
}

void CAnimation::RegisterBone(uint boneId)
{
	ushort boneCount;
	edANM_SKELETON* pSkeleton;
	float fVar3;
	int iVar4;
	float* pfVar5;
	BoneData* pBoneData;
	BoneData* pAVar7;
	edF32MATRIX4* peVar8;
	edF32VECTOR4 local_10;

	for (pBoneData = this->pBoneData; (pBoneData != (BoneData*)0x0 && (boneId != pBoneData->boneId)); pBoneData = pBoneData->pPrev) {
	}

	if (pBoneData == (BoneData*)0x0) {
		iVar4 = 0xc0;
		pBoneData = (CScene::ptable.g_AnimManager_00451668)->aAnimMatrixData;
		do {
			if (pBoneData->usedByCount == 0) goto LAB_0017fb98;
			iVar4 = iVar4 + -1;
			pBoneData = pBoneData + 1;
		} while (iVar4 != 0);

		pBoneData = (BoneData*)0x0;

	LAB_0017fb98:
		if (pBoneData != (BoneData*)0x0) {
			pBoneData->boneId = boneId;
			int boneNodeIndex = -1;
			if ((this->anmSkeleton).pTag != (edANM_SKELETON*)0x0) {
				boneNodeIndex = this->anmSkeleton.NodeIndexFromID(boneId);
			}

			if (boneNodeIndex == -1) {
				boneNodeIndex = 0;
			}

			pBoneData->boneNodeIndex = boneNodeIndex;
			pBoneData->usedByCount = 1;
			boneNodeIndex = -1;

			if ((this->anmSkeleton).pTag != (edANM_SKELETON*)0x0) {
				boneNodeIndex = this->anmSkeleton.NodeIndexFromID(pBoneData->boneId);
			}

			if (boneNodeIndex == -1) {
				boneNodeIndex = 0;
			}

			pSkeleton = (this->anmSkeleton).pTag;

			if ((pSkeleton->flags & 2) == 0) {
				boneCount = pSkeleton->boneCount;

				edF32MATRIX4* pDst = reinterpret_cast<edF32MATRIX4*>(reinterpret_cast<char*>(pSkeleton) + ((boneCount * 0xc + 0x13) & 0xfffffff0) + ((boneCount * 2 + boneCount) * 4 + boneNodeIndex * 0x10) * 4);

				edF32Matrix4CopyHard(&pBoneData->matrix, pDst);
				edF32Matrix4InverseSoft(&pBoneData->matrix);
			}
			else {
				boneCount = pSkeleton->boneCount;
				
				edF32VECTOR4* pDst = reinterpret_cast<edF32VECTOR4*>(reinterpret_cast<char*>(pSkeleton) + ((boneCount * 0xc + 0x13) & 0xfffffff0) + ((boneCount * 2 + boneCount) * 4 + boneNodeIndex * 0x4) * 4);

				local_10 = *pDst;

				edF32Vector4GetNegHard(&local_10, &local_10);

				pBoneData->matrix = gF32Matrix4Unit;

				pBoneData->matrix.rowT.xyz = local_10.xyz;
				pBoneData->matrix.rowT.w = 1.0f;
			}

			pBoneData->pNext = (BoneData*)0x0;
			pBoneData->pPrev = this->pBoneData;

			this->pBoneData = pBoneData;

			if (pBoneData->pPrev != (BoneData*)0x0) {
				pBoneData->pPrev->pNext = pBoneData;
			}
		}
	}
	else {
		pBoneData->usedByCount = pBoneData->usedByCount + 1;
	}
	return;
}

void CAnimation::UnRegisterBone(uint boneId)
{
	BoneData* pBoneData;

	for (pBoneData = this->pBoneData; (pBoneData != (BoneData*)0x0 && (boneId != pBoneData->boneId));
		pBoneData = pBoneData->pPrev) {
	}
	if ((pBoneData != (BoneData*)0x0) && (pBoneData->usedByCount = pBoneData->usedByCount + -1, pBoneData->usedByCount == 0)) {
		if (pBoneData->pNext == (BoneData*)0x0) {
			this->pBoneData = pBoneData->pPrev;
		}
		else {
			pBoneData->pNext->pPrev = pBoneData->pPrev;
		}

		if (pBoneData->pPrev != (BoneData*)0x0) {
			pBoneData->pPrev->pNext = pBoneData->pNext;
		}

		pBoneData->usedByCount = 0;
	}
	return;
}

void CAnimation::AddDisabledBone(uint boneId)
{
	int freeIndex;
	int nodeId;
	int curIndex;

	freeIndex = this->nbDisabledBones;
	curIndex = 0;
	if (0 < freeIndex) {
		do {
			if (boneId == this->aDisabledBoneData[curIndex].boneId) {
				return;
			}

			curIndex = curIndex + 1;
		} while (curIndex < freeIndex);
	}

	this->aDisabledBoneData[freeIndex].boneId = boneId;

	nodeId = -1;
	if ((this->anmSkeleton).pTag != (edANM_SKELETON*)0x0) {
		nodeId = this->anmSkeleton.NodeIndexFromID(boneId);
	}

	if (nodeId == -1) {
		nodeId = 0;
	}

	this->aDisabledBoneData[freeIndex].nodeId = nodeId;

	this->nbDisabledBones = this->nbDisabledBones + 1;
	return;
}

void CAnimation::RemoveDisabledBone(uint boneId)
{
	int indexToRemove;
	int curIndex;

	indexToRemove = this->nbDisabledBones;
	curIndex = 0;
	if (0 < indexToRemove) {
		do {
			if (boneId == this->aDisabledBoneData[curIndex].boneId) {
				this->aDisabledBoneData[curIndex].boneId = this->aDisabledBoneData[indexToRemove + -1].boneId;
				this->aDisabledBoneData[curIndex].nodeId = this->aDisabledBoneData[indexToRemove + -1].nodeId;
				this->nbDisabledBones = this->nbDisabledBones + -1;
				return;
			}

			curIndex = curIndex + 1;
		} while (curIndex < indexToRemove);
	}

	return;
}

float CAnimation::GetAnimLength(int param_2, int lengthMode)
{
	int* piVar1;
	float fVar2;
	edAnmStateDesc eStack32;
	edAnmStateParser local_4;

	piVar1 = (this->anmBinMetaAnimator).pAnimKeyEntryData;
	local_4 = (int*)(reinterpret_cast<char*>(piVar1) + piVar1[param_2] + 4);
	local_4.BuildDesc(&eStack32, -1, -1);
	fVar2 = this->anmBinMetaAnimator.GetAnimStateDescLength(&eStack32, lengthMode);
	return fVar2;
}

bool CAnimation::IsLayerActive(uint layer)
{
	return layer == (this->count_0x2c & layer);
}

int CAnimation::PhysicalLayerFromLayerId(uint layer)
{
	int iVar1;
	uint uVar2;

	iVar1 = 0;
	if ((layer == (this->count_0x2c & layer)) && (uVar2 = 1, layer != 1)) {
		do {
			if ((this->count_0x2c & uVar2) != 0) {
				iVar1 = iVar1 + 1;
			}
			uVar2 = uVar2 << 1;
		} while (uVar2 != layer);
	}

	return iVar1;
}

edF32MATRIX4* CAnimation::GetCurBoneMatrix(uint boneId)
{
	BoneData* pAnimMatrixData;

	for (pAnimMatrixData = this->pBoneData; (pAnimMatrixData != (BoneData*)0x0 && (boneId != pAnimMatrixData->boneId)); pAnimMatrixData = pAnimMatrixData->pPrev) {
	}

	if (pAnimMatrixData == (BoneData*)0x0) {
		return &gF32Matrix4Unit;
	}

	return &pAnimMatrixData->matrix;
}

void CAnimation::GetDefaultBoneMatrix(uint boneId, edF32MATRIX4* pDefaultMatrix)
{
	ushort boneCount;
	edANM_SKELETON* pSkeleton;
	float fVar3;
	int nodeId;
	float* pfVar5;
	edF32MATRIX4* peVar6;
	edF32MATRIX4* peVar7;
	edF32VECTOR4 local_10;

	nodeId = -1;
	if ((this->anmSkeleton).pTag != (edANM_SKELETON*)0x0) {
		nodeId = this->anmSkeleton.NodeIndexFromID(boneId);
	}

	if (nodeId == -1) {
		nodeId = 0;
	}

	pSkeleton = (this->anmSkeleton).pTag;
	if ((pSkeleton->flags & 2) == 0) {
		boneCount = pSkeleton->boneCount;

		edF32MATRIX4* pDst = reinterpret_cast<edF32MATRIX4*>(reinterpret_cast<char*>(pSkeleton) + ((boneCount * 0xc + 0x13) & 0xfffffff0) + ((boneCount * 2 + boneCount) * 4 + nodeId * 0x10) * 4);

		edF32Matrix4CopyHard(pDefaultMatrix, pDst);
		edF32Matrix4InverseSoft(pDefaultMatrix);
	}
	else {
		boneCount = pSkeleton->boneCount;
		
		edF32VECTOR4* pDst = reinterpret_cast<edF32VECTOR4*>(reinterpret_cast<char*>(pSkeleton) + ((boneCount * 0xc + 0x13) & 0xfffffff0) + ((boneCount * 2 + boneCount) * 4 + nodeId * 0x4) * 4);

		local_10 = *pDst;

		edF32Vector4GetNegHard(&local_10, &local_10);

		*pDefaultMatrix = gF32Matrix4Unit;
		pDefaultMatrix->rowT.xyz = local_10.xyz;
		pDefaultMatrix->rowT.w = 1.0f;
	}

	return;
}

bool CAnimation::FUN_0017f730()
{
	int iVar1;
	CEventTrack* pEventTrack;
	int index;
	uint uVar2;

	if (((this->anmBinMetaAnimator).aAnimData)->animPlayState == 1) {
		index = 0;

		for (uVar2 = this->count_0x2c; uVar2 != 0; uVar2 = uVar2 >> 1) {
			if ((uVar2 & 1) != 0) {
				iVar1 = this->anmBinMetaAnimator.GetAnimEventTrackID(index);

				if (iVar1 != -1) {
					pEventTrack = CScene::ptable.g_TrackManager_004516b4->GetTrack(iVar1);
					if (pEventTrack->FUN_0019f140() != false) {
						return true;
					}
				}

				index = index + 1;
			}
		}
	}

	return false;
}
