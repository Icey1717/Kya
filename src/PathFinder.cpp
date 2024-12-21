#include "PathFinder.h"

CAstarMemBank::CAstarMemBank()
{
	int* piVar1;
	CPathDynamic* aDynamicPaths;
	int iVar2;
	CPathNode* pCVar3;
	int iVar4;
	int iVar5;

	this->nbPathDynamicUsed = 0;
	this->pCurDynamicPath = (CPathDynamic*)0x0;
	this->nbPathNodesUsed = 0;
	this->pCurPathNode = (CPathNode*)0x0;

	this->aDynamicPaths = new CPathDynamic[0x14];

	for (int i = 0; i < 0x13; i++) {
		this->aDynamicPaths[i].pNext = this->aDynamicPaths + i + 1;
	}

	this->aDynamicPaths[0x13].pNext = (CPathDynamic*)0x0;

	this->pCurDynamicPath = this->aDynamicPaths;

	this->aPathNodes = new CPathNode[0x100];

	for (int i = 0; i < 0xfe; i++) {
		this->aPathNodes[i].pNext = this->aPathNodes + i + 1;
	}

	this->aPathNodes[0xff].pNext = (CPathNode*)0x0;
	
	this->pCurPathNode = this->aPathNodes;
	return;
}

void CAstarMemBank::DeletePathDynamic(CPathDynamic* pPathDynamic)
{
	CPathNode* pCVar1;
	CPathNode* pCVar2;

	pCVar2 = pPathDynamic->pPathNode;
	while (pCVar2 != (CPathNode*)0x0) {
		pCVar1 = pCVar2->pNext;
		pCVar2->field_0x0 = 0;
		pCVar2->field_0xc = 0;
		pCVar2->field_0x10 = 0;
		pCVar2->field_0x8 = 0;
		pCVar2->field_0x4 = -1;
		pCVar2->field_0x14 = 0;

		pCVar2->pNext = (CPathNode*)0x0;
		pCVar2->pNext = this->pCurPathNode;
		this->pCurPathNode = pCVar2;
		this->nbPathNodesUsed = this->nbPathNodesUsed + -1;
		pCVar2 = pCVar1;
	}

	pPathDynamic->Clear();

	pPathDynamic->pNext = this->pCurDynamicPath;
	this->pCurDynamicPath = pPathDynamic;
	this->nbPathDynamicUsed = this->nbPathDynamicUsed + -1;
	return;
}

CPathDynamic* CAstarMemBank::NewPathDynamic(edF32VECTOR4* pStart, edF32VECTOR4* pDestination)
{
	CPathDynamic* pDynamicPath = this->pCurDynamicPath;

	if (pDynamicPath != (CPathDynamic*)0x0) {
		this->pCurDynamicPath = pDynamicPath->pNext;

		pDynamicPath->Clear();
		pDynamicPath->Init(pStart, pDestination);

		this->nbPathDynamicUsed = this->nbPathDynamicUsed + 1;
	}

	return pDynamicPath;
}

CBasicPathFinder::CBasicPathFinder()
{
	//this->field_0x408 = 0;
	//this->field_0x40c = 0;
	this->nbNaviMeshes = 0;
	this->aNaviMeshes = (CBasicNaviMesh*)0x0;
	this->curNaviMeshIndex = 0;

	return;
}

bool CBasicPathFinder::IsValidPosition(edF32VECTOR4* pPosition, int id)
{
	bool bIsValidPosition;
	short cellId;
	CBasicNaviMesh* pBasicNaviMesh;

	if (id == -1) {
		pBasicNaviMesh = GetArea(pPosition);
	}
	else {
		this->curNaviMeshIndex = id;
		pBasicNaviMesh = this->aNaviMeshes + id;
	}

	if ((pBasicNaviMesh == (CBasicNaviMesh*)0x0) || (cellId = pBasicNaviMesh->SearchCellId(pPosition), cellId == -1)) {
		bIsValidPosition = false;
	}
	else {
		bIsValidPosition = true;
	}

	return bIsValidPosition;
}

void CBasicPathFinder::DeletePathDynamic(CPathDynamic* pPathDynamic)
{
	astarMemBank.DeletePathDynamic(pPathDynamic);
}

void CBasicPathFinder::Init()
{
	for (int i = 0; i < this->nbNaviMeshes; i++) {
		this->aNaviMeshes[i].Init();
	}

	return;
}

void CBasicPathFinder::Create(ByteCode* pByteCode)
{
	int count;

	this->nbNaviMeshes = pByteCode->GetS32();

	count = this->nbNaviMeshes;
	if (count < 1) {
		this->aNaviMeshes = (CBasicNaviMesh*)0x0;
	}
	else {
		this->aNaviMeshes = new CBasicNaviMesh[count];
	}

	for (int i = 0; i < this->nbNaviMeshes; i++) {
		this->aNaviMeshes[i].Create(pByteCode);
	}

	return;
}

CBasicNaviMesh* CBasicPathFinder::GetArea(edF32VECTOR4* pLocation)
{
	bool bInBoundingBox;
	CBasicNaviMesh* pArea;
	uint curIndex;

	pArea = (CBasicNaviMesh*)0x0;

	bInBoundingBox = IsInBoundingBox(&this->aNaviMeshes[this->curNaviMeshIndex].field_0x30, pLocation);
	if (bInBoundingBox == false) {
		curIndex = 0;
		if (this->aNaviMeshes != (CBasicNaviMesh*)0x0) {
			do {
				bInBoundingBox = IsInBoundingBox(&this->aNaviMeshes[curIndex].field_0x30, pLocation);
				if (bInBoundingBox != false) {
					this->curNaviMeshIndex = curIndex;
					this->curNaviMeshIndex = curIndex;
					return this->aNaviMeshes + curIndex;
				}

				curIndex = curIndex + 1;
			} while (curIndex < this->nbNaviMeshes);
		}
	}
	else {
		pArea = this->aNaviMeshes + this->curNaviMeshIndex;
	}

	return pArea;
}

CPathDynamic* CBasicPathFinder::NewPathDynamic(edF32VECTOR4* pStart, edF32VECTOR4* pDestination)
{
	return this->astarMemBank.NewPathDynamic(pStart, pDestination);
}
