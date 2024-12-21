#include "FileManager3D.h"
#include <string.h>
#include "MemoryStream.h"
#include "edList.h"
#include "port/pointer_conv.h"

#ifdef PLATFORM_WIN
#include "port.h"
#endif

#include "edDlist.h"
#include "CameraViewManager.h"

C3DFileManager::C3DFileManager()
{
	this->pParticleInfoArray_0x50 = new ParticleInfo[0x600];
	Level_ClearInternalData();
	return;
}

void C3DFileManager::Level_AddAll(ByteCode* pMemoryStream)
{
	int iVar1;
	int iVar2;
	edNODE* pMVar3;
	Mesh* pMVar4;
	int iStack8;
	int iStack4;

	iVar1 = pMemoryStream->GetS32();
	if (iVar1 == 0) {
		this->pMeshInfo = (ed_g3d_manager*)0x0;
		this->levelSectorTextureIndex = 0;
		this->pLastMeshTransformParent = (edNODE*)0x0;
	}
	else {
		const int commonLevelMeshIndex = pMemoryStream->GetS32();
		const int commonLevelTextureIndex = pMemoryStream->GetS32();
		pMemoryStream->GetS32();
		this->levelSectorTextureIndex = commonLevelTextureIndex;
		pMVar4 = this->aCommonLevelMeshes + commonLevelMeshIndex;

		NAME_NEXT_OBJECT("%s", pMVar4->name.c_str());

		if (pMVar4->pTextureInfo == (TextureInfo*)0x0) {
			pMVar4->pTextureInfo = this->aCommonLevelTextures + commonLevelTextureIndex;
			ed3DInstallG3D(pMVar4->pFileData, pMVar4->fileLength, 0, &iStack4, &pMVar4->pTextureInfo->manager, 0xc, &pMVar4->meshInfo);
		}

		this->pMeshInfo = &pMVar4->meshInfo;
		ed3DScenePushCluster(CScene::_scene_handleA, this->pMeshInfo);
		pMVar3 = ed3DHierarchyAddToSceneByHashcode(CScene::_scene_handleA, this->pMeshInfo, 0x43494d414e5944);
		this->pLastMeshTransformParent = pMVar3;
	}

	const int commonLevelMeshIndex = pMemoryStream->GetS32();
	if (commonLevelMeshIndex == -1) {
		this->pBackgroundNode = (edNODE*)0x0;
	}
	else {
		const int commonLevelTextureIndex = pMemoryStream->GetS32();
		pMVar4 = this->aCommonLevelMeshes + commonLevelMeshIndex;
		if (pMVar4->pTextureInfo == (TextureInfo*)0x0) {
			NAME_NEXT_OBJECT("%s", pMVar4->name.c_str());

			pMVar4->pTextureInfo = this->aCommonLevelTextures + commonLevelTextureIndex;
			ed3DInstallG3D(pMVar4->pFileData, pMVar4->fileLength, 0, &iStack8, &pMVar4->pTextureInfo->manager, 0xc, &pMVar4->meshInfo);
		}

		pMVar3 = ed3DHierarchyAddToScene(CScene::_scene_handleB, &pMVar4->meshInfo, (char*)0x0);
		this->pBackgroundNode = pMVar3;

		if (this->pBackgroundNode != (edNODE*)0x0) {
			SetupBackground(this->pBackgroundNode);
		}
	}

	return;
}

edDList_material* C3DFileManager::GetMaterialFromId(int materialId, int internalId)
{
	edDList_material* pMaterial;
	int curIndex;
	ParticleInfo* pPVar3;

	pPVar3 = this->pParticleInfoArray_0x50;
	for (curIndex = 0; (pPVar3->ID != materialId && (curIndex < 0x80)); curIndex = curIndex + 1) {
		pPVar3 = pPVar3 + 1;
	}

	if (pPVar3->materialInfoArray_0x8 == (edDList_material*)0x0) {
		pPVar3 = (ParticleInfo*)0x0;
	}

	pMaterial = (edDList_material*)0x0;
	if (pPVar3 != (ParticleInfo*)0x0) {
		if ((internalId < 0) || (pPVar3->materialCount_0x4 <= internalId)) {
			pMaterial = (edDList_material*)0x0;
		}
		else {
			pMaterial = pPVar3->materialInfoArray_0x8 + internalId;
		}
	}

	return pMaterial;
}

int C3DFileManager::InstanciateG2D(int index)
{
	ParticleInfo* pPVar1;
	int iVar2;

	pPVar1 = this->pParticleInfoArray_0x50;
	for (iVar2 = 0; (pPVar1->ID != index && (iVar2 < 0x80)); iVar2 = iVar2 + 1) {
		pPVar1 = pPVar1 + 1;
	}
	if (pPVar1->materialInfoArray_0x8 == (edDList_material*)0x0) {
		pPVar1 = (ParticleInfo*)0x0;
	}
	if (pPVar1 == (ParticleInfo*)0x0) {
		IMPLEMENTATION_GUARD(
		pPVar1 = _CreateG2DInfo(index);)
	}
	if (pPVar1 == (ParticleInfo*)0x0) {
		iVar2 = -1;
	}
	else {
		iVar2 = pPVar1->materialCount_0x4;
	}
	return iVar2;
}

TextureInfo* C3DFileManager::GetCommonSectorG2D()
{
	return this->aCommonLevelTextures + this->levelSectorTextureIndex;
}

void C3DFileManager::Level_ClearInternalData()
{
	void* pvVar1;
	ParticleInfo* pPVar2;
	int iVar3;

	iVar3 = 0x80;
	this->meshCount = 0;
	this->meshLoadedCount = 0;
	this->aCommonLevelMeshes = (Mesh*)0x0;
	this->textureCount = 0;
	this->textureLoadedCount = 0;
	this->aCommonLevelTextures = (TextureInfo*)0x0;
	this->pMeshInfo = (ed_g3d_manager*)0x0;
	this->pBackgroundNode = (edNODE*)0x0;
	pPVar2 = this->pParticleInfoArray_0x50;
	do {
		pPVar2->ID = -1;
		pPVar2->materialCount_0x4 = 0;
		iVar3 = iVar3 + -8;
		pPVar2->materialInfoArray_0x8 = (edDList_material*)0x0;
		pPVar2[1].ID = -1;
		pPVar2[1].materialCount_0x4 = 0;
		pPVar2[1].materialInfoArray_0x8 = (edDList_material*)0x0;
		pPVar2[2].ID = -1;
		pPVar2[2].materialCount_0x4 = 0;
		pPVar2[2].materialInfoArray_0x8 = (edDList_material*)0x0;
		pPVar2[3].ID = -1;
		pPVar2[3].materialCount_0x4 = 0;
		pPVar2[3].materialInfoArray_0x8 = (edDList_material*)0x0;
		pPVar2[4].ID = -1;
		pPVar2[4].materialCount_0x4 = 0;
		pPVar2[4].materialInfoArray_0x8 = (edDList_material*)0x0;
		pPVar2[5].ID = -1;
		pPVar2[5].materialCount_0x4 = 0;
		pPVar2[5].materialInfoArray_0x8 = (edDList_material*)0x0;
		pPVar2[6].ID = -1;
		pPVar2[6].materialCount_0x4 = 0;
		pPVar2[6].materialInfoArray_0x8 = (edDList_material*)0x0;
		pPVar2[7].ID = -1;
		pPVar2[7].materialCount_0x4 = 0;
		pPVar2[7].materialInfoArray_0x8 = (edDList_material*)0x0;
		pPVar2 = pPVar2 + 8;
	} while (0 < iVar3);
	pvVar1 = memset(&this->backgroundHierarchySetup, 0, sizeof(ed_3d_hierarchy_setup));
	this->field_0x30 = 10000.0f;
	this->backgroundHierarchySetup.clipping_0x0 = (char*)&this->field_0x30;
	(this->backgroundHierarchySetup).pBoundingSphere = (edF32VECTOR4*)0x0;
	return;
}

void C3DFileManager::Level_Create(ByteCode* pMemoryStream)
{
	int iVar1;
	Mesh* pMVar2;
	TextureInfo* pTVar3;
	int iVar4;

	this->levelSectorTextureIndex = 0;
	iVar1 = pMemoryStream->GetS32();
	this->meshCount = iVar1;
	pMVar2 = new Mesh[this->meshCount];
	this->aCommonLevelMeshes = pMVar2;
	for (iVar1 = this->meshCount; 0 < iVar1; iVar1 = iVar1 + -1) {
		pMemoryStream->GetS32();
	}
	iVar1 = pMemoryStream->GetS32();
	this->textureCount = iVar1;
	pTVar3 = new TextureInfo[this->textureCount];
	this->aCommonLevelTextures = pTVar3;
	pMemoryStream->GetS32();
	pMVar2 = this->aCommonLevelMeshes;
	for (iVar1 = this->meshCount; 0 < iVar1; iVar1 = iVar1 + -1) {
		pMVar2->fileLength = 0;
		pMVar2->pFileData = (char*)0x0;
		pMVar2->pTextureInfo = (TextureInfo*)0x0;
		pMVar2 = pMVar2 + 1;
	}
	iVar1 = 0;
	if (0 < this->textureCount) {
		do {
			this->aCommonLevelTextures[iVar1].pFileBuffer = NULL;
			iVar1 = iVar1 + 1;
		} while (iVar1 < this->textureCount);
	}
	return;
}

void C3DFileManager::ShowCommonBackground()
{
	if (this->pBackgroundNode != (edNODE*)0x0) {
		ed3DHierarchyNodeSetRenderOff(CScene::_scene_handleA, this->pBackgroundNode);
	}
	return;
}

void C3DFileManager::HideCommonBackground()
{
	if (this->pBackgroundNode != (edNODE*)0x0) {
		ed3DHierarchyNodeSetRenderOn(CScene::_scene_handleA, this->pBackgroundNode);
	}
	return;
}

void C3DFileManager::SetupBackground(edNODE* pNode)
{
	edNODE* pPrevNode;
	ed_g3d_object* piVar2;
	uint uVar3;

	ed_3d_hierarchy* pHier = (ed_3d_hierarchy*)pNode->pData;

	if ((pNode->pData != (void*)0x0) && (uVar3 = pHier->linkedHierCount, uVar3 != 0)) {
		pPrevNode = pNode->pPrev;
		for (; uVar3 != 0; uVar3 = uVar3 - 1) {

			ed_3d_hierarchy* pPrevHier = (ed_3d_hierarchy*)pPrevNode->pData;
			if (pPrevHier->hash.number == 0x4c464e99b2a49e87) {
				piVar2 = ed3DHierarchyGetObject((ed_3d_hierarchy*)pPrevNode->pData);

				ed_3d_strip* pStrip = (ed_3d_strip*)LOAD_SECTION(piVar2->p3DStrip);

				pStrip->pDMA_Matrix.flagsA = 0x200;
			}

			pPrevNode = pPrevNode->pPrev;
		}
	}

	ed3DHierarchyNodeSetSetup(pNode, &this->backgroundHierarchySetup);

	return;
}

void C3DFileManager::ManageBackground(edNODE* pNode, uint flags)
{
	ed_3d_hierarchy* peVar1;
	edF32MATRIX4* peVar2;
	CCameraManager* pCVar3;
	edNODE* peVar4;
	uint uVar5;
	float fVar6;
	float fVar7;
	float fVar8;

	pCVar3 = CScene::ptable.g_CameraManager_0045167c;
	if (((pNode != (edNODE*)0x0) && ((flags & 1) != 0)) && (peVar1 = (ed_3d_hierarchy*)pNode->pData, peVar1 != (ed_3d_hierarchy*)0x0)) {
		uVar5 = (uint)(ushort)peVar1->linkedHierCount;
		if (uVar5 == 0) {
			(peVar1->transformA).rowT = ((CScene::ptable.g_CameraManager_0045167c)->transformationMatrix).rowT;
		}
		else {
			peVar4 = pNode->pPrev;
			if (uVar5 != 0) {
				peVar2 = &(CScene::ptable.g_CameraManager_0045167c)->transformationMatrix;
				do {
					peVar1 = (ed_3d_hierarchy*)peVar4->pData;
					if (peVar1->hash.number != 0x4c464e99b2a49e87) {
						(peVar1->transformA).rowT = (pCVar3->transformationMatrix).rowT;
					}
					uVar5 = uVar5 - 1;
					peVar4 = peVar4->pPrev;
				} while (uVar5 != 0);
			}
		}
	}
	return;
}

ed_g3d_manager* C3DFileManager::GetG3DManager(int meshIndex, int textureIndex)
{
	Mesh* pCommonLevelMesh;
	int iStack4;

	pCommonLevelMesh = this->aCommonLevelMeshes + meshIndex;
	if (pCommonLevelMesh->pTextureInfo == (TextureInfo*)0x0) {
		NAME_NEXT_OBJECT("%s", pCommonLevelMesh->name.c_str());

		pCommonLevelMesh->pTextureInfo = this->aCommonLevelTextures + textureIndex;
		ed3DInstallG3D(pCommonLevelMesh->pFileData, pCommonLevelMesh->fileLength, 0, &iStack4, &pCommonLevelMesh->pTextureInfo->manager, 0xc,
			&pCommonLevelMesh->meshInfo);
	}
	return &pCommonLevelMesh->meshInfo;
}

ParticleInfo* C3DFileManager::GetG2DInfo(int index)
{
	ParticleInfo* pParticleInfo;
	int iVar1;

	pParticleInfo = this->pParticleInfoArray_0x50;

	for (iVar1 = 0; (pParticleInfo->ID != index && (iVar1 < 0x80)); iVar1 = iVar1 + 1) {
		pParticleInfo = pParticleInfo + 1;
	}

	if (pParticleInfo->materialInfoArray_0x8 == (edDList_material*)0x0) {
		pParticleInfo = (ParticleInfo*)0x0;
	}

	return pParticleInfo;
}

ed_g2d_manager* C3DFileManager::GetActorsCommonMaterial(int index)
{
	return &this->aCommonLevelTextures[index].manager;
}

ed_g2d_manager* C3DFileManager::GetActorsCommonMeshMaterial(int index)
{
	return &this->aCommonLevelMeshes[index].pTextureInfo->manager;
}

ed_g2d_manager* C3DFileManager::LoadDefaultTexture_001a65d0()
{
	TextureInfo* pTVar1;

	pTVar1 = (TextureInfo*)0x0;
	if (CScene::_pinstance->defaultTextureIndex_0x2c != -1) {
		pTVar1 = this->aCommonLevelTextures + CScene::_pinstance->defaultTextureIndex_0x2c;
	}
	return &pTVar1->manager;
}

ed_g3d_manager* C3DFileManager::GetInfoForCommonLevelMesh(char* pFileData)
{
	int curMeshIndex;
	Mesh* pMesh;

	curMeshIndex = this->meshCount;
	pMesh = this->aCommonLevelMeshes;

	while (true) {
		if (curMeshIndex == 0) {
			return (ed_g3d_manager*)0x0;
		}

		if (pFileData == pMesh->pFileData) break;

		curMeshIndex = curMeshIndex + -1;
		pMesh = pMesh + 1;
	}

	return &pMesh->meshInfo;
}
