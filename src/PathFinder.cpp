#include "PathFinder.h"
#include "MathOps.h"

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
		pCVar2->Reset();
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

void CAstarMemBank::RecycleNodeFromNodeBank(CPathNode* pNewPathNode)
{
	pNewPathNode->Reset();

	pNewPathNode->pNext = this->pCurPathNode;
	this->pCurPathNode = pNewPathNode;
	this->nbPathNodesUsed = this->nbPathNodesUsed + -1;

	return;
}

CPathNode* CAstarMemBank::GetFreeNodeFromNodeBank()
{
	CPathNode* pFreeNode;

	pFreeNode = this->pCurPathNode;
	if (pFreeNode != (CPathNode*)0x0) {
		this->pCurPathNode = pFreeNode->pNext;
		pFreeNode->Reset();
		this->nbPathNodesUsed = this->nbPathNodesUsed + 1;
	}

	return pFreeNode;
}

CBasicPathFinder::CBasicPathFinder()
{
	this->nodeListCount = 0;
	this->pNodeList = (CPathNode*)0x0;

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

int CBasicPathFinder::FindPath(CPathDynamic* pPath, int id)
{
	ushort uVar1;
	bool bVar2;
	edF32VECTOR4* pStart;
	edF32VECTOR4* pGoal;
	int resultStatus;
	uint uVar3;
	uint goalCellId;
	CPathNode* pCVar5;
	uint uVar6;
	CPathNode* pNode;
	long lVar7;
	uint uVar8;
	CAstarBasicTransition* pCVar9;
	CBasicNaviCell* pCVar10;
	CBasicNaviCell* pCVar11;
	CBasicNaviCell* pCVar12;
	CBasicNaviMesh* pNavMesh;
	uint uVar13;
	float fVar14;
	float fVar15;
	edF32VECTOR4 local_20;
	edF32VECTOR4 local_10;

	if (pPath == (CPathDynamic*)0x0) {
		resultStatus = 5;
	}
	else {
		pStart = pPath->GetStart();
		pGoal = pPath->GetGoalB();
		if (id == -1) {
			pNavMesh = (CBasicNaviMesh*)0x0;

			edF32Matrix4MulF32Vector4Hard(&local_10, &this->aNaviMeshes[this->curNaviMeshIndex].field_0x30, pStart);
			if (((((local_10.x < -0.5005f) || (0.5005f < local_10.x)) || (local_10.y < -0.5005f)) ||
				((0.5005f < local_10.y || (local_10.z < -0.5005f)))) || (bVar2 = true, 0.5005f < local_10.z)) {
				bVar2 = false;
			}

			if (bVar2) {
				pNavMesh = this->aNaviMeshes + this->curNaviMeshIndex;
			}
			else {
				uVar3 = 0;
				if (this->nbNaviMeshes != 0) {
					do {
						edF32Matrix4MulF32Vector4Hard(&local_20, &this->aNaviMeshes[uVar3].field_0x30, pStart);
						if (((local_20.x < -0.5005f) || (0.5005f < local_20.x)) ||
							((local_20.y < -0.5005f || (((0.5005f < local_20.y || (local_20.z < -0.5005f)) || (bVar2 = true, 0.5005f < local_20.z)))))) {
							bVar2 = false;
						}

						if (bVar2) {
							this->curNaviMeshIndex = uVar3;
							this->curNaviMeshIndex = uVar3;
							pNavMesh = this->aNaviMeshes + uVar3;
							break;
						}

						uVar3 = uVar3 + 1;
					} while (uVar3 < this->nbNaviMeshes);
				}
			}
		}
		else {
			this->curNaviMeshIndex = id;
			pNavMesh = this->aNaviMeshes + id;
		}

		if (pNavMesh == (CBasicNaviMesh*)0x0) {
			pPath->SetStatus(3);
			resultStatus = pPath->GetStatus();
		}
		else {
			uint startCellId = pNavMesh->SearchCellId(pStart);

			if (((startCellId) == 0xffff) ||
				(goalCellId = pNavMesh->SearchCellId(pGoal), (goalCellId) == 0xffff)) {
				pPath->SetStatus(3);
				resultStatus = pPath->GetStatus();
			}
			else {
				if ((this->priorityQueue).field_0x400 == -1) {
					pCVar5 = this->aNaviMeshes[this->curNaviMeshIndex].aNaviCells[startCellId].pPathNode;
					if ((pCVar5 == (CPathNode*)0x0) &&
						(pCVar5 = this->astarMemBank.GetFreeNodeFromNodeBank(), pCVar5 != (CPathNode*)0x0)) {
						pCVar5->field_0x4 = startCellId;
						if (this->pNodeList != (CPathNode*)0x0) {
							this->pNodeList->pPrev = pCVar5;
						}

						pCVar5->pNext = this->pNodeList;
						pCVar5->pPrev = (CPathNode*)0x0;
						this->pNodeList = pCVar5;
						this->nodeListCount = this->nodeListCount + 1;
						this->aNaviMeshes[this->curNaviMeshIndex].aNaviCells[pCVar5->field_0x4].pPathNode = pCVar5;
					}

					if (pCVar5 == (CPathNode*)0x0) {
						return 2;
					}

					pCVar11 = pNavMesh->aNaviCells;
					uVar1 = pCVar5->field_0x4;
					pCVar10 = pCVar11 + goalCellId;
					pCVar11 = pCVar11 + uVar1;
					fVar15 = pCVar10->field_0x14.x - pCVar11->field_0x14.x;
					fVar14 = pCVar10->field_0x14.y - pCVar11->field_0x14.y;
					pCVar5->field_0x10 = sqrtf(fVar14 * fVar14 + fVar15 * fVar15);
					pCVar5->field_0x0 = 1;
					this->priorityQueue.PushPriorityQueue(pCVar5);
				}

				goalCellId = goalCellId;

				do {
					if ((this->priorityQueue).field_0x400 == -1) {
						FreeAllNodes();
						pPath->SetStatus(3);
						resultStatus = pPath->GetStatus();
						return resultStatus;
					}

					pCVar5 = this->priorityQueue.PopPriorityQueue();
					if (goalCellId == pCVar5->field_0x4) {
						BuildPath(pPath, pCVar5);
						FreeAllNodes();
						resultStatus = pPath->GetStatus();
						return resultStatus;
					}

					uVar6 = pCVar5->field_0x4;
					uVar3 = pNavMesh->aNaviCells[uVar6].nbSubDataA;
					uVar13 = 0;
					if (uVar3 != 0) {
						resultStatus = 0;
						do {
							pCVar9 = pNavMesh->aAstarBasicTransitions + pNavMesh->aNaviCells[uVar6].pDataA[uVar13];
							uVar8 = pCVar9->field_0x2;
							if (uVar6 == uVar8) {
								uVar8 = pCVar9->field_0x0;
							}

							pNode = this->aNaviMeshes[this->curNaviMeshIndex].aNaviCells[uVar8].pPathNode;
							if ((pNode == (CPathNode*)0x0) &&
								(pNode = this->astarMemBank.GetFreeNodeFromNodeBank(), pNode != (CPathNode*)0x0)) {
								pNode->field_0x4 = uVar8;
								if (this->pNodeList != (CPathNode*)0x0) {
									this->pNodeList->pPrev = pNode;
								}

								pNode->pNext = this->pNodeList;
								pNode->pPrev = (CPathNode*)0x0;
								this->pNodeList = pNode;
								this->nodeListCount = this->nodeListCount + 1;
								this->aNaviMeshes[this->curNaviMeshIndex].aNaviCells[pNode->field_0x4].pPathNode = pNode;
							}

							if (pNode == (CPathNode*)0x0) {
								FreeAllNodes();
								return 2;
							}

							if ((pCVar5->field_0x8 == (CPathNode*)0x0) || (pCVar5->field_0x8->field_0x4 != pNode->field_0x4)) {
								fVar14 = pCVar5->field_0xc + pNavMesh->aAstarBasicTransitions[pNavMesh->aNaviCells[uVar6].pDataA[uVar13]].field_0x4;

								if (pNode->field_0x0 == 1) {
									if (fVar14 < pNode->field_0xc) {
										pNode->field_0xc = fVar14;
										pCVar11 = pNavMesh->aNaviCells;
										pCVar10 = pCVar11 + uVar8;
										pCVar11 = pCVar11 + goalCellId;
										fVar15 = pCVar10->field_0x14.x - pCVar11->field_0x14.x;
										fVar14 = pCVar10->field_0x14.y - pCVar11->field_0x14.y;
										pNode->field_0x10 = sqrtf(fVar14 * fVar14 + fVar15 * fVar15);
										pNode->field_0x8 = pCVar5;
										this->priorityQueue.UpdateNodeOnPriorityQueue(pNode);
									}
								}
								else {
									if (pNode->field_0x0 == 2) {
										if (fVar14 < pNode->field_0xc) {
											pNode->field_0xc = fVar14;
											pCVar11 = pNavMesh->aNaviCells;
											pCVar12 = pCVar11 + uVar8;
											pCVar10 = pCVar11 + goalCellId;
											fVar15 = pCVar10->field_0x14.x - pCVar11->field_0x14.x;
											fVar14 = pCVar10->field_0x14.y - pCVar11->field_0x14.y;
											pNode->field_0x10 = pNode->field_0xc + sqrtf(fVar14 * fVar14 + fVar15 * fVar15);
											pNode->field_0x8 = pCVar5;
											pNode->field_0x0 = 1;
											this->priorityQueue.PushPriorityQueue(pNode);
										}
									}
									else {
										pNode->field_0xc = fVar14;
										pCVar11 = pNavMesh->aNaviCells;
										pCVar12 = pCVar11 + uVar8;
										pCVar10 = pCVar11 + goalCellId;
										fVar15 = pCVar10->field_0x14.x - pCVar11->field_0x14.x;
										fVar14 = pCVar10->field_0x14.y - pCVar11->field_0x14.y;
										pNode->field_0x10 = pNode->field_0xc + sqrtf(fVar14 * fVar14 + fVar15 * fVar15);
										pNode->field_0x8 = pCVar5;
										pNode->field_0x0 = 1;
										this->priorityQueue.PushPriorityQueue(pNode);
									}
								}
							}

							uVar13 = uVar13 + 1;
							resultStatus = resultStatus + 2;
						} while (uVar13 < uVar3);
					}

					pCVar5->field_0x0 = 2;
				} while (ShouldAbortSearch() == false);

				FreeAllNodes();
				pPath->SetStatus(2);
				resultStatus = pPath->GetStatus();
			}
		}
	}

	return resultStatus;
}

void CBasicPathFinder::FreeAllNodes()
{
	CPathNode* pNewPathNode;

	pNewPathNode = this->pNodeList;
	while (pNewPathNode != (CPathNode*)0x0) {
		// Possibly inline FreeNodeFromExploredNodeList
		this->aNaviMeshes[this->curNaviMeshIndex].aNaviCells[pNewPathNode->field_0x4].pPathNode = (CPathNode*)0x0;

		if (((this->pNodeList == pNewPathNode) || (pNewPathNode->pPrev != (CPathNode*)0x0)) ||
			(pNewPathNode->pNext != (CPathNode*)0x0)) {
			if (this->pNodeList == pNewPathNode) {
				this->pNodeList = pNewPathNode->pNext;
			}
			if (pNewPathNode->pPrev != (CPathNode*)0x0) {
				pNewPathNode->pPrev->pNext = pNewPathNode->pNext;
			}
			if (pNewPathNode->pNext != (CPathNode*)0x0) {
				pNewPathNode->pNext->pPrev = pNewPathNode->pPrev;
			}

			this->nodeListCount = this->nodeListCount + -1;
			pNewPathNode->pPrev = (CPathNode*)0x0;
			pNewPathNode->pNext = (CPathNode*)0x0;
		}

		this->astarMemBank.RecycleNodeFromNodeBank(pNewPathNode);
		pNewPathNode = this->pNodeList;
	}

	this->priorityQueue.Clear();

	return;
}

bool CBasicPathFinder::ShouldAbortSearch()
{
	return false;
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

void CBasicPathFinder::BuildPath(CPathDynamic* pPathDynamic, CPathNode* pNode)
{
	CPathNode* pCVar1;
	CPathNode* pCVar2;

	pCVar1 = (CPathNode*)0x0;
	if (pNode != (CPathNode*)0x0) {
		do {
			pCVar2 = pNode;
			this->aNaviMeshes[this->curNaviMeshIndex].aNaviCells[pCVar2->field_0x4].pPathNode = (CPathNode*)0x0;
			if (((this->pNodeList == pCVar2) || (pCVar2->pPrev != (CPathNode*)0x0)) || (pCVar2->pNext != (CPathNode*)0x0)) {
				if (this->pNodeList == pCVar2) {
					this->pNodeList = pCVar2->pNext;
				}

				if (pCVar2->pPrev != (CPathNode*)0x0) {
					pCVar2->pPrev->pNext = pCVar2->pNext;
				}

				if (pCVar2->pNext != (CPathNode*)0x0) {
					pCVar2->pNext->pPrev = pCVar2->pPrev;
				}

				this->nodeListCount = this->nodeListCount + -1;
				pCVar2->pPrev = (CPathNode*)0x0;
				pCVar2->pNext = (CPathNode*)0x0;
			}

			pCVar2->field_0x0 = 3;
			pCVar2->pNext = pCVar1;
			pPathDynamic->pPathNode = pCVar2;
			pNode = pCVar2->field_0x8;
			pCVar1 = pCVar2;
		} while (pCVar2->field_0x8 != (CPathNode*)0x0);
	}

	pPathDynamic->naviMeshIndex = this->curNaviMeshIndex;

	pPathDynamic->Reset();

	if ((pPathDynamic->pPathNode == (CPathNode*)0x0) || (pPathDynamic->pPathNode->pNext != (CPathNode*)0x0)) {
		pPathDynamic->SetStatus(0);
	}
	else {
		pPathDynamic->SetStatus(1);
	}

	return;
}


int CBasicPathFinder::CheckAndUpdatePath(CPathDynamic** ppPathDynamic, edF32VECTOR4* param_3, edF32VECTOR4* param_4)
{
	CBasicNaviMesh* pNaviMeshes;
	uint cellId;
	CPathNode* pCVar3;
	CPathDynamic* pCVar4;
	int naviMeshIndex;
	float* pfVar6;
	int iVar7;
	long lVar8;
	float fVar9;

	pNaviMeshes = this->aNaviMeshes;
	naviMeshIndex = (*ppPathDynamic)->naviMeshIndex;
	cellId = pNaviMeshes[naviMeshIndex].SearchCellId(param_4);

	pCVar3 = (*ppPathDynamic)->GetLastData();
	if ((cellId & 0xffff) == (uint)(ushort)pCVar3->field_0x4) {

		*(*ppPathDynamic)->GetGoalB() = *param_4;

		cellId = pNaviMeshes[naviMeshIndex].SearchCellId(param_3);
		while (true) {
			if (((cellId & 0xffff) == (uint)(ushort)(*ppPathDynamic)->field_0x54->field_0x4) || ((*ppPathDynamic)->AtGoal() != 0)) break;
			(*ppPathDynamic)->NextWayPoint();
		}

		pCVar4 = *ppPathDynamic;
		if ((cellId & 0xffff) == (uint) * (ushort*)(pCVar4->field_0x54 + 4)) {
			if (pCVar4->AtGoal() == 0) {
				(*ppPathDynamic)->SetStatus(0);
			}
			else {
				(*ppPathDynamic)->SetStatus(1);
			}
		}
		else {
			DeletePathDynamic(*ppPathDynamic);
			*ppPathDynamic = NewPathDynamic(param_3, param_4);
			FindPath(*ppPathDynamic, (*ppPathDynamic)->naviMeshIndex);

			if (((*ppPathDynamic)->GetStatus() != 0) && ((*ppPathDynamic)->GetStatus() != 1)) {
				(*ppPathDynamic)->SetStatus(4);
			}
		}
	}
	else {
		if ((cellId & 0xffff) == 0xffff) {
			(*ppPathDynamic)->SetStatus(4);
		}
		else {
			DeletePathDynamic(*ppPathDynamic);
			*ppPathDynamic = NewPathDynamic(param_3, param_4);
			FindPath(*ppPathDynamic, (*ppPathDynamic)->naviMeshIndex);

			if (((*ppPathDynamic)->GetStatus() != 0) && ((*ppPathDynamic)->GetStatus() != 1)) {
				(*ppPathDynamic)->SetStatus(4);
			}
		}
	}

	return (*ppPathDynamic)->GetStatus();
}


bool ClipWayThroughPortal(float param_1, edF32VECTOR4* param_2, edF32VECTOR4* param_3, edF32VECTOR4* param_4, CNaviMeshPoint* param_5, CNaviMeshPoint* param_6)
{
	edF32VECTOR4* peVar1;
	bool bVar2;
	float fVar3;
	float fVar4;
	float fVar5;
	edF32VECTOR4 local_80;
	edF32VECTOR4 local_70;
	edF32VECTOR4 local_60;
	edF32VECTOR4 local_50;
	edF32VECTOR4 eStack64;
	edF32VECTOR4 eStack48;
	edF32VECTOR4 eStack32;
	edF32VECTOR4 eStack16;

	bVar2 = false;

	local_50.x = param_5->field_0x0;
	local_50.y = 0.0f;
	local_50.z = param_5->field_0x4;
	local_50.w = 1.0f;

	local_60.x = param_6->field_0x0;
	local_60.y = 0.0f;
	local_60.z = param_6->field_0x4;
	local_60.w = 1.0f;

	edF32Vector4SubHard(&eStack64, &local_60, &local_50);

	fVar3 = edF32Vector4GetDistHard(&eStack64);
	if (fVar3 < param_1 * 2.0f) {
		edF32Vector4AddHard(&eStack64, &local_50, &local_60);
		edF32Vector4ScaleHard(0.5f, param_2, &eStack64);
	}
	else {
		edF32Vector4NormalizeHard(&eStack64, &eStack64);
		edF32Vector4ScaleHard(param_1, &eStack64, &eStack64);
		if ((param_5->field_0x8 & 1) == 0) {
			edF32Vector4AddHard(&local_70, &local_50, &eStack64);
		}
		else {
			local_70 = local_50;
		}

		if ((param_6->field_0x8 & 1) == 0) {
			edF32Vector4SubHard(&local_80, &local_60, &eStack64);
		}
		else {
			local_80 = local_60;
		}

		edF32Vector4SubHard(&eStack16, param_3, param_4);
		edF32Vector4SubHard(&eStack32, &local_70, param_4);
		edF32Vector4SubHard(&eStack48, &local_80, param_4);
		eStack48.y = 0.0f;
		eStack32.y = 0.0f;
		eStack16.y = 0.0f;
		fVar3 = edF32Vector4GetDistHard(&eStack16);
		if (fVar3 <= 0.001f) {
			bVar2 = false;
			*param_2 = *param_3;
		}
		else {
			edF32Vector4ScaleHard(1.0f / fVar3, &eStack16, &eStack16);
			edF32Vector4NormalizeHard(&eStack32, &eStack32);
			edF32Vector4NormalizeHard(&eStack48, &eStack48);
			edF32Vector4CrossProductHard(&eStack64, &eStack16, &eStack32);
			fVar3 = eStack64.y;
			edF32Vector4CrossProductHard(&eStack64, &eStack16, &eStack48);
			if ((fVar3 <= 0.0f) || (eStack64.y <= 0.0f)) {
				if ((0.0f <= fVar3) || (0.0f <= eStack64.y)) {
					*param_2 = *param_3;

				}
				else {
					peVar1 = &local_80;
					if (eStack64.y < fVar3) {
						peVar1 = &local_70;
					}
					bVar2 = true;
					*param_2 = *peVar1;
				}
			}
			else {
				peVar1 = &local_80;
				if (fVar3 < eStack64.y) {
					peVar1 = &local_70;
				}

				bVar2 = true;
				*param_2 = *peVar1;
			}
		}
	}

	return bVar2;
}



void CBasicPathFinder::ComputeSubTarget(float param_1, edF32VECTOR4* param_3, edF32VECTOR4* param_4, CPathDynamic* pPathDynamic)
{
	ushort uVar1;
	ushort uVar2;
	CPathNode* pCVar3;
	CNaviMeshPoint* pNVar4;
	bool bVar5;
	float* pfVar6;
	int iVar7;
	CPathNode* pCVar8;
	uint uVar9;
	long lVar10;
	CBasicNaviCell* pCVar11;
	CAstarBasicTransition* pCVar12;
	ulong uVar13;
	ulong uVar14;
	ulong uVar15;
	CPathNode* pPathNode;
	CBasicNaviMesh* pNaviMesh;
	float fVar17;
	float fVar18;
	float fVar19;
	edF32VECTOR4 local_20;
	edF32VECTOR2 local_8;

	local_8.x = param_4->x;
	local_8.y = param_4->z;

	pPathNode = pPathDynamic->field_0x54;
	uVar1 = pPathNode->field_0x4;
	pNaviMesh = this->aNaviMeshes + pPathDynamic->naviMeshIndex;

	if (pPathNode->pNext == (CPathNode*)0x0) {
		*param_3 = *pPathDynamic->GetGoalB();
		pPathDynamic->SetStatus(1);
	}
	else {
		uVar2 = pPathNode->pNext->field_0x4;
		uVar14 = (ulong)uVar2;
		pCVar11 = pNaviMesh->aNaviCells + uVar2;

		if ((((local_8.x < (pCVar11->field_0x20).x) || ((pCVar11->field_0x20).z < local_8.x)) ||
			(local_8.y < (pCVar11->field_0x20).y)) || ((pCVar11->field_0x20).w < local_8.y)) {
			bVar5 = false;
		}
		else {
			bVar5 = true;
		}

		uVar15 = (ulong)uVar1;
		if ((bVar5) && (iVar7 = pNaviMesh->IsInCell(uVar2, &local_8), iVar7 != 0)) {
			pPathDynamic->NextWayPoint();

			if (pPathDynamic->AtGoal() != false) {
				*param_3 = *pPathDynamic->GetGoalB();
				pPathDynamic->SetStatus(1);
				return;
			}

			pPathNode = pPathDynamic->field_0x54;
			uVar14 = (ulong)(ushort)pPathNode->field_0x4;
			uVar15 = uVar14;
		}

		uVar13 = 0xffffffffffffffff;
		if ((pPathDynamic->field_0x30 & 1) != 0) {
			uVar13 = (ulong)((long)*(int*)&pPathDynamic->field_0x30 << 0x2f) >> 0x30;
		}

		if (((uVar13 == 0xffffffffffffffff) || (uVar15 == uVar13)) || (uVar14 == uVar13)) {
			pCVar8 = pPathDynamic->GetLastData();
			local_20 = *pPathDynamic->GetGoalB();

			uVar1 = pCVar8->field_0x4;
			while (pCVar8 != pPathNode) {
				pCVar3 = pCVar8->field_0x8;
				uVar2 = pCVar8->field_0x4;
				uVar9 = pNaviMesh->GetEdgeId(pCVar3->field_0x4, uVar2);
				pNVar4 = pNaviMesh->aMeshPoints;
				pCVar12 = pNaviMesh->aAstarBasicTransitions + (uVar9 & 0xffff);
				lVar10 = ClipWayThroughPortal(param_1, &local_20, &local_20, param_4, pNVar4 + pCVar12->field_0x8, pNVar4 + pCVar12->field_0xa);
				pCVar8 = pCVar3;
				if (lVar10 != 0) {
					uVar1 = uVar2;
				}
			}

			*param_3 = local_20;
			pPathDynamic->field_0x40 = local_20;

			pPathDynamic->field_0x30 = pPathDynamic->field_0x30 & 0xfe | 1;
			pPathDynamic->field_0x30_uint = pPathDynamic->field_0x30_uint & 0xfffe0001 | (uint)uVar1 << 1;
		}
		else {
			*param_3 = pPathDynamic->field_0x40;
		}
	}
	return;
}

CPriorityQueue::CPriorityQueue()
{
	this->field_0x400 = -1;

	for (int i = 0; i < 0x100; i++) {
		this->aEntries[i] = (CPathNode*)0x0;
	}

	return;
}

void CPriorityQueue::Clear()
{
	for (int i = 0; i < 0x100; i++) {
		this->aEntries[i] = (CPathNode*)0x0;
	}

	this->field_0x400 = -1;

	return;
}

void CPriorityQueue::PushPriorityQueue(CPathNode* pPathNode)
{
	CPathNode* pCVar1;
	CPathNode** ppCVar2;
	int iVar3;
	CPathNode** ppCVar4;
	int iVar5;

	iVar3 = this->field_0x400;
	iVar5 = iVar3 + 1;
	this->field_0x400 = iVar5;
	this->aEntries[iVar3 + 1] = pPathNode;
	this->aEntries[iVar3 + 1]->field_0x1c = iVar5;

	while (true) {
		if (iVar5 < 1) {
			return;
		}

		iVar3 = iVar5 + -1 >> 1;
		ppCVar2 = this->aEntries + iVar5;
		pCVar1 = *ppCVar2;

		if (this->aEntries[iVar3]->field_0x10 <= pCVar1->field_0x10) break;

		ppCVar4 = this->aEntries + iVar3;
		*ppCVar2 = *ppCVar4;
		*ppCVar4 = pCVar1;
		(*ppCVar2)->field_0x1c = (short)iVar3;
		(*ppCVar4)->field_0x1c = (short)iVar5;
		iVar5 = iVar3;
	}

	return;
}

CPathNode* CPriorityQueue::PopPriorityQueue()
{
	int iVar1;
	CPathNode* pCVar2;

	iVar1 = this->field_0x400;
	this->field_0x400 = iVar1 + -1;
	pCVar2 = this->aEntries[0];
	this->aEntries[0] = this->aEntries[iVar1];
	this->aEntries[0]->field_0x1c = 0;

	if (0 < this->field_0x400) {
		this->aEntries[this->field_0x400 + 1] = (CPathNode*)0x0;
		UpdateNodeOnPriorityQueue(this->aEntries[0]);
	}

	pCVar2->field_0x1c = -1;

	return pCVar2;
}

void CPriorityQueue::UpdateNodeOnPriorityQueue(CPathNode* pNode)
{
	int iVar1;
	CPathNode* pCVar2;
	bool bVar3;
	int iVar4;
	long lVar5;
	CPathNode** ppCVar6;
	CPathNode** ppCVar7;
	int iVar8;
	int iVar9;
	long lVar10;

	lVar5 = (long)pNode->field_0x1c;
	do {
		iVar1 = this->field_0x400;
		iVar4 = (int)lVar5;
		bVar3 = true;
		iVar8 = iVar4 * 2 + 2;
		iVar9 = iVar4 * 2 + 1;
		lVar10 = (long)iVar9;

		if ((long)iVar1 < (long)iVar8) {
			if (lVar10 == iVar1) {
				ppCVar7 = this->aEntries + iVar4;
				ppCVar6 = this->aEntries + iVar1;
				pCVar2 = *ppCVar7;
				if ((*ppCVar6)->field_0x10 < pCVar2->field_0x10) {
					*ppCVar7 = *ppCVar6;
					*ppCVar6 = pCVar2;
					(*ppCVar7)->field_0x1c = (short)iVar1;
					(*ppCVar6)->field_0x1c = (short)lVar5;
				}
			}
		}
		else {
			if (this->aEntries[iVar4 * 2 + 2]->field_0x10 <= this->aEntries[iVar4 * 2 + 1]->field_0x10) {
				lVar10 = (long)iVar8;
				iVar9 = iVar8;
			}

			ppCVar7 = this->aEntries + iVar9;
			ppCVar6 = this->aEntries + iVar4;
			pCVar2 = *ppCVar6;
			if ((*ppCVar7)->field_0x10 < pCVar2->field_0x10) {
				*ppCVar6 = *ppCVar7;
				bVar3 = false;
				*ppCVar7 = pCVar2;
				(*ppCVar6)->field_0x1c = (short)lVar10;
				(*ppCVar7)->field_0x1c = (short)lVar5;
				lVar5 = lVar10;
			}
		}
	} while (!bVar3);

	return;
}