#include "FileManager3D.h"
#include <string.h>
#include "MemoryStream.h"
#include "ed3D.h"

FileManager3D::FileManager3D()
{
	this->pParticleInfoArray_0x50 = new ParticleInfo[0x600];
	Level_ClearInternalData();
	return;
}

void FileManager3D::Level_AddAll(ByteCode* pMemoryStream)
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
		iVar1 = pMemoryStream->GetS32();
		iVar2 = pMemoryStream->GetS32();
		pMemoryStream->GetS32();
		this->levelSectorTextureIndex = iVar2;
		pMVar4 = this->pMeshDataArray + iVar1;
		if (pMVar4->pTextureInfo == (TextureInfo*)0x0) {
			pMVar4->pTextureInfo = this->pTextureInfoArray + iVar2;
			ed3DInstallG3D(pMVar4->pFileData, pMVar4->fileLength, 0, &iStack4, &pMVar4->pTextureInfo->pManager, 0xc, &pMVar4->meshInfo);
		}
		this->pMeshInfo = &pMVar4->meshInfo;
		ed3DScenePushCluster(CScene::_scene_handleA, this->pMeshInfo);
		pMVar3 = ed3DHierarchyAddToSceneByHashcode(CScene::_scene_handleA, this->pMeshInfo, 0x43494d414e5944);
		this->pLastMeshTransformParent = pMVar3;
	}
	iVar1 = pMemoryStream->GetS32();
	if (iVar1 == -1) {
		this->pMeshTransformParent = (edNODE*)0x0;
	}
	else {
		iVar2 = pMemoryStream->GetS32();
		pMVar4 = this->pMeshDataArray + iVar1;
		if (pMVar4->pTextureInfo == (TextureInfo*)0x0) {
			pMVar4->pTextureInfo = this->pTextureInfoArray + iVar2;
			ed3DInstallG3D(pMVar4->pFileData, pMVar4->fileLength, 0, &iStack8, &pMVar4->pTextureInfo->pManager, 0xc, &pMVar4->meshInfo);
		}
		IMPLEMENTATION_GUARD(pMVar3 = ed3DHierarchyAddToScene(CScene::_scene_handleB, &pMVar4->meshInfo, (char*)0x0);
		this->pMeshTransformParent = pMVar3;
		if (this->pMeshTransformParent != (edNODE*)0x0) {
			3DFileManager::Func_001a6510(this, this->pMeshTransformParent);
		})
	}
	return;
}

int FileManager3D::InstanciateG2D(int index)
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

TextureInfo* FileManager3D::GetCommonSectorG2D()
{
	return this->pTextureInfoArray + this->levelSectorTextureIndex;
}

void FileManager3D::Level_ClearInternalData()
{
	void* pvVar1;
	ParticleInfo* pPVar2;
	int iVar3;

	iVar3 = 0x80;
	this->meshCount = 0;
	this->meshLoadedCount = 0;
	this->pMeshDataArray = (Mesh*)0x0;
	this->textureCount = 0;
	this->textureLoadedCount = 0;
	this->pTextureInfoArray = (TextureInfo*)0x0;
	this->pMeshInfo = (ed_g3d_manager*)0x0;
	this->pMeshTransformParent = (edNODE*)0x0;
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
	pvVar1 = memset(&this->field_0x10, 0, 0x20);
	this->field_0x30 = 10000.0f;
	this->field_0x10 = (char*)&this->field_0x30;
	*(undefined4*)&this->field_0x14 = 0;
	return;
}

void FileManager3D::Level_Create(ByteCode* pMemoryStream)
{
	int iVar1;
	Mesh* pMVar2;
	TextureInfo* pTVar3;
	int iVar4;

	this->levelSectorTextureIndex = 0;
	iVar1 = pMemoryStream->GetS32();
	this->meshCount = iVar1;
	pMVar2 = new Mesh[this->meshCount];
	this->pMeshDataArray = pMVar2;
	for (iVar1 = this->meshCount; 0 < iVar1; iVar1 = iVar1 + -1) {
		pMemoryStream->GetS32();
	}
	iVar1 = pMemoryStream->GetS32();
	this->textureCount = iVar1;
	pTVar3 = new TextureInfo[this->textureCount];
	this->pTextureInfoArray = pTVar3;
	pMemoryStream->GetS32();
	pMVar2 = this->pMeshDataArray;
	for (iVar1 = this->meshCount; 0 < iVar1; iVar1 = iVar1 + -1) {
		pMVar2->fileLength = 0;
		pMVar2->pFileData = (char*)0x0;
		pMVar2->pTextureInfo = (TextureInfo*)0x0;
		pMVar2 = pMVar2 + 1;
	}
	iVar1 = 0;
	if (0 < this->textureCount) {
		do {
			iVar1 = iVar1 + 1;
			this->pTextureInfoArray[iVar1].pFileBuffer = NULL;
		} while (iVar1 < this->textureCount);
	}
	return;
}