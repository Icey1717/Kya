#include "FileManager3D.h"
#include <string.h>
#include "MemoryStream.h"
#include "ed3D.h"

FileManager3D::FileManager3D()
{
	//ParticleInfo* pPVar1;
	//pPVar1 = (ParticleInfo*)AllocateFunc_001002a0(0x600);
	//this->pParticleInfoArray_0x50 = pPVar1;
	Setup_001a7110();
	return;
}

void FileManager3D::Deserialize(MemoryStream* pMemoryStream)
{
	int iVar1;
	int iVar2;
	MeshTransformParent* pMVar3;
	Mesh* pMVar4;
	int iStack8;
	int iStack4;

	iVar1 = pMemoryStream->ReadInt_00189b70();
	if (iVar1 == 0) {
		this->pMeshInfo = (MeshInfo*)0x0;
		this->levelSectorTextureIndex = 0;
		this->pLastMeshTransformParent = (MeshTransformParent*)0x0;
	}
	else {
		iVar1 = pMemoryStream->ReadInt_00189b70();
		iVar2 = pMemoryStream->ReadInt_00189b70();
		pMemoryStream->ReadInt_00189b70();
		this->levelSectorTextureIndex = iVar2;
		pMVar4 = this->pMeshDataArray + iVar1;
		if (pMVar4->pTextureInfo == (TextureInfo*)0x0) {
			pMVar4->pTextureInfo = this->pTextureInfoArray + iVar2;
			ed3D::LoadMeshFromBuffer(pMVar4->pFileData, pMVar4->fileLength, 0, &iStack4, pMVar4->pTextureInfo, 0xc, &pMVar4->meshInfo);
		}
		this->pMeshInfo = &pMVar4->meshInfo;
		ed3D::Func_002a5540(g_StaticMeshMasterA_00448808, this->pMeshInfo);
		pMVar3 = ed3D::SetupMeshTransform_001fffe0
		(g_StaticMeshMasterA_00448808, this->pMeshInfo, 0x43494d414e5944);
		this->pLastMeshTransformParent = pMVar3;
	}
	iVar1 = pMemoryStream->ReadInt_00189b70();
	if (iVar1 == -1) {
		this->pMeshTransformParent = (MeshTransformParent*)0x0;
	}
	else {
		iVar2 = pMemoryStream->ReadInt_00189b70();
		pMVar4 = this->pMeshDataArray + iVar1;
		if (pMVar4->pTextureInfo == (TextureInfo*)0x0) {
			pMVar4->pTextureInfo = this->pTextureInfoArray + iVar2;
			ed3D::LoadMeshFromBuffer(pMVar4->pFileData, pMVar4->fileLength, 0, &iStack8, pMVar4->pTextureInfo, 0xc, &pMVar4->meshInfo);
		}
		IMPLEMENTATION_GUARD(pMVar3 = CreateMeshTransform_002ad750(g_StaticMeshMasterA_00448808, &pMVar4->meshInfo, (char*)0x0);
		this->pMeshTransformParent = pMVar3;
		if (this->pMeshTransformParent != (MeshTransformParent*)0x0) {
			3DFileManager::Func_001a6510(this, this->pMeshTransformParent);
		})
	}
	return;
}

TextureInfo* FileManager3D::GetLevelSectorTextureInfo_001a6670()
{
	return this->pTextureInfoArray + this->levelSectorTextureIndex;
}

void FileManager3D::Setup_001a7110()
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
	this->pMeshInfo = (MeshInfo*)0x0;
	this->pMeshTransformParent = (MeshTransformParent*)0x0;
	//pPVar2 = this->pParticleInfoArray_0x50;
	//do {
	//	pPVar2->ID = -1;
	//	pPVar2->materialCount_0x4 = 0;
	//	iVar3 = iVar3 + -8;
	//	pPVar2->materialInfoArray_0x8 = (MaterialInfo*)0x0;
	//	pPVar2[1].ID = -1;
	//	pPVar2[1].materialCount_0x4 = 0;
	//	pPVar2[1].materialInfoArray_0x8 = (MaterialInfo*)0x0;
	//	pPVar2[2].ID = -1;
	//	pPVar2[2].materialCount_0x4 = 0;
	//	pPVar2[2].materialInfoArray_0x8 = (MaterialInfo*)0x0;
	//	pPVar2[3].ID = -1;
	//	pPVar2[3].materialCount_0x4 = 0;
	//	pPVar2[3].materialInfoArray_0x8 = (MaterialInfo*)0x0;
	//	pPVar2[4].ID = -1;
	//	pPVar2[4].materialCount_0x4 = 0;
	//	pPVar2[4].materialInfoArray_0x8 = (MaterialInfo*)0x0;
	//	pPVar2[5].ID = -1;
	//	pPVar2[5].materialCount_0x4 = 0;
	//	pPVar2[5].materialInfoArray_0x8 = (MaterialInfo*)0x0;
	//	pPVar2[6].ID = -1;
	//	pPVar2[6].materialCount_0x4 = 0;
	//	pPVar2[6].materialInfoArray_0x8 = (MaterialInfo*)0x0;
	//	pPVar2[7].ID = -1;
	//	pPVar2[7].materialCount_0x4 = 0;
	//	pPVar2[7].materialInfoArray_0x8 = (MaterialInfo*)0x0;
	//	pPVar2 = pPVar2 + 8;
	//} while (0 < iVar3);
	pvVar1 = memset(&this->field_0x10, 0, 0x20);
	*(undefined4*)&this->field_0x30 = 0x461c4000;
	this->field_0x10 = (char*)&this->field_0x30;
	*(undefined4*)&this->field_0x14 = 0;
	return;
}

void FileManager3D::AllocateMeshTextureMemory_001a6f10(MemoryStream* pMemoryStream)
{
	int iVar1;
	Mesh* pMVar2;
	TextureInfo* pTVar3;
	int iVar4;

	this->levelSectorTextureIndex = 0;
	iVar1 = pMemoryStream->ReadInt_00189b70();
	this->meshCount = iVar1;
	pMVar2 = (Mesh*)AllocateFunc_001002a0((long)(this->meshCount * sizeof(Mesh)));
	this->pMeshDataArray = pMVar2;
	for (iVar1 = this->meshCount; 0 < iVar1; iVar1 = iVar1 + -1) {
		pMemoryStream->ReadInt_00189b70();
	}
	iVar1 = pMemoryStream->ReadInt_00189b70();
	this->textureCount = iVar1;
	pTVar3 = (TextureInfo*)AllocateFunc_001002a0((long)(this->textureCount * sizeof(TextureInfo)));
	this->pTextureInfoArray = pTVar3;
	pMemoryStream->ReadInt_00189b70();
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