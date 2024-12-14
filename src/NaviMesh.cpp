#include "NaviMesh.h"
#include "PoolAllocators.h"
#include "MathOps.h"

bool IsInBoundingBox(edF32MATRIX4* pBox, edF32VECTOR4* pPoint)
{
	bool bIsInBoundingBox;
	edF32VECTOR4 transformedPoint;

	edF32Matrix4MulF32Vector4Hard(&transformedPoint, pBox, pPoint);

	if ((((transformedPoint.x <= -0.5f) || (0.5f < transformedPoint.x)) 
		|| (transformedPoint.y <= -0.5f)) || (((0.5f < transformedPoint.y 
		|| (transformedPoint.z <= -0.5f)) || (0.5f < transformedPoint.z)))) {
		bIsInBoundingBox = false;
	}
	else {
		bIsInBoundingBox = true;
	}

	return bIsInBoundingBox;
}

void CBasicNaviMesh::Init()
{
	CBasicNaviCell* pCVar1;
	float fVar2;
	CAstarBasicTransition* pTransition;
	uint uVar4;
	CBasicNaviCell* iVar9;
	uint uVar5;
	NaviMeshSubObj* pSubObj;
	CBasicNaviCell* pBasicNaviCell;
	edF32VECTOR4 local_20;
	edF32VECTOR2 local_8;

	uVar5 = 0;
	if (this->nbCells != 0) {
		do {
			local_20.x = 3.402823e+38f;
			local_20.y = 3.402823e+38f;
			local_20.z = -3.402823e+38f;
			local_20.w = -3.402823e+38f;
			pBasicNaviCell = this->aNaviCells + uVar5;

			uVar4 = 0;
			if (pBasicNaviCell->nbSubDataB != 0) {
				do {
					pSubObj = this->aSubObjs + pBasicNaviCell->pDataB[uVar4];
					fVar2 = pSubObj->field_0x0;
					if (local_20.x <= pSubObj->field_0x0) {
						fVar2 = local_20.x;
					}
					local_20.x = fVar2;

					fVar2 = pSubObj->field_0x4;
					if (local_20.y <= pSubObj->field_0x4) {
						fVar2 = local_20.y;
					}
					local_20.y = fVar2;

					fVar2 = pSubObj->field_0x0;
					if (pSubObj->field_0x0 <= local_20.z) {
						fVar2 = local_20.z;
					}
					local_20.z = fVar2;

					fVar2 = pSubObj->field_0x4;
					if (pSubObj->field_0x4 <= local_20.w) {
						fVar2 = local_20.w;
					}
					local_20.w = fVar2;

					uVar4 = uVar4 + 1;
					pBasicNaviCell = this->aNaviCells + uVar5;
				} while (uVar4 < pBasicNaviCell->nbSubDataB);
			}

			pBasicNaviCell->field_0x14.x = (local_20.x + local_20.z) * 0.5f;
			pBasicNaviCell->field_0x14.y = (local_20.y + local_20.w) * 0.5f;

			uVar5 = uVar5 + 1;
			pBasicNaviCell->field_0x20 = local_20;
		} while (uVar5 < this->nbCells);
	}

	uVar5 = 0;
	if (this->nbTransitions != 0) {
		do {
			pCVar1 = this->aNaviCells;
			pTransition = this->aAstarBasicTransitions + uVar5;
			edF32Vector2Sub(&local_8, &pCVar1[pTransition->field_0x0].field_0x14, &pCVar1[pTransition->field_0x2].field_0x14);
			uVar5 = uVar5 + 1 & 0xffff;
			pTransition->field_0x4 = sqrtf(local_8.y * local_8.y + local_8.x * local_8.x);
		} while (uVar5 < this->nbTransitions);
	}

	return;
}

void CBasicNaviMesh::Create(ByteCode* pByteCode)
{
	ushort uVar1;
	uint uVar2;
	void* pvVar3;
	uint uVar4;
	int* piVar5;
	CBasicNaviCell* pCVar6;
	CAstarBasicTransition* pCVar8;
	CAstarSubGraph* pCVar9;
	int iVar10;
	int iVar11;
	int iVar12;
	int iVar13;
	uint count;
	float fVar14;

	this->nbSubObjs = pByteCode->GetU32();

	this->aSubObjs = new NaviMeshSubObj[this->nbSubObjs];

	for (uint i = 0; i < this->nbSubObjs; i++) {
		this->aSubObjs[i].field_0x0 = pByteCode->GetF32();
		this->aSubObjs[i].field_0x4 = pByteCode->GetF32();
		this->aSubObjs[i].field_0x8 = pByteCode->GetU32();
	}

	count = 0;
	this->nbCells = pByteCode->GetU16();
	uVar2 = this->nbCells;
	uVar4 = 0;
	if (uVar2 == 0) {
		this->aNaviCells = (CBasicNaviCell*)0x0;
	}
	else {
		this->aNaviCells = new CBasicNaviCell[uVar2];
	}

	for (uint i = 0; i < this->nbCells; i++) {
		this->aNaviCells[i].Create(pByteCode);
		count = count + this->aNaviCells[i].nbSubDataB;
		uVar4 = uVar4 + this->aNaviCells[i].nbSubDataA;
	}

	this->pDataStreamB = NewPool_edU16(count);
	this->pDataStreamA = NewPool_edU16(uVar4);

	uVar2 = 0;
	if (count != 0) {
		do {
			this->pDataStreamB[uVar2] = pByteCode->GetU16();
			uVar2 = uVar2 + 1;
		} while (uVar2 < count);
	}

	uVar2 = 0;
	if (uVar4 != 0) {
		iVar13 = 0;
		do {
			this->pDataStreamA[uVar2] = pByteCode->GetU16();
			uVar2 = uVar2 + 1;
		} while (uVar2 < uVar4);
	}

	int dataOffsetA = 0;
	int dataOffsetB = 0;
	uVar2 = 0;
	if (this->nbCells != 0) {
		do {
			this->aNaviCells[uVar2].pDataB = this->pDataStreamB + dataOffsetB;
			this->aNaviCells[uVar2].pDataA = this->pDataStreamA + dataOffsetA;
			
			dataOffsetB = dataOffsetB + this->aNaviCells[uVar2].nbSubDataB;
			dataOffsetA = dataOffsetA + this->aNaviCells[uVar2].nbSubDataA;
			uVar2 = uVar2 + 1;
		} while (uVar2 < this->nbCells);
	}

	this->nbTransitions = pByteCode->GetU16();
	uVar2 = this->nbTransitions;
	if (uVar2 == 0) {
		this->aAstarBasicTransitions = (CAstarBasicTransition*)0x0;
	}
	else {
		this->aAstarBasicTransitions = new CAstarBasicTransition[uVar2];
	}

	uVar2 = 0;
	if (this->nbTransitions != 0) {
		do {
			this->aAstarBasicTransitions[uVar2].Create(pByteCode);
			uVar2 = uVar2 + 1;
		} while (uVar2 < this->nbTransitions);
	}

	this->nbSubGraphs = pByteCode->GetU16();
	uVar2 = this->nbSubGraphs;
	if (uVar2 == 0) {
		this->aSubGraphs = (CAstarSubGraph*)0x0;
	}
	else {
		this->aSubGraphs = new CAstarSubGraph[uVar2];
	}

	uVar2 = 0;
	if (this->nbSubGraphs != 0) {
		do {
			this->aSubGraphs[uVar2].Create(pByteCode);
			uVar2 = uVar2 + 1;
		} while (uVar2 < this->nbSubGraphs);
	}

	pByteCode->Align(4);

	uVar2 = 0;
	do {
		this->field_0x30.raw[uVar2] = pByteCode->GetF32();
		uVar2 = uVar2 + 1;
	} while (uVar2 < 0x10);

	return;
}

uint CBasicNaviMesh::SearchCellId(edF32VECTOR4* pPosition)
{
	float* pfVar1;
	NaviMeshSubObj* pNavMeshSubObj;
	uint uVar4;
	bool bVar5;
	float* pfVar6;
	CAstarSubGraph* pSubGraph;
	CBasicNaviCell* pCurCell;
	ushort* puVar9;
	CBasicNaviCell* pNaviCell;
	uint uVar12;
	uint curCell;
	uint maxCells;
	NaviMeshSubObj* pNVar15;
	uint uVar17;
	float fVar18;
	float fVar19;
	edF32VECTOR4 local_10;

	fVar18 = pPosition->x;
	fVar19 = pPosition->z;
	uVar17 = 0;
	if (this->nbSubGraphs != 0) {
		do {
			bVar5 = IsInBoundingBox(&this->aSubGraphs->boundingBox, pPosition);
			if (bVar5) {
				pSubGraph = this->aSubGraphs + uVar17;
				curCell = pSubGraph->cellStart;
				maxCells = curCell + pSubGraph->nbCells;

				if (curCell < maxCells) {
					pNaviCell = this->aNaviCells + curCell;

					do {
						if (((fVar18 < pNaviCell->field_0x20.x) || (pNaviCell->field_0x20.z < fVar18)) ||
							((fVar19 < pNaviCell->field_0x20.y || (pNaviCell->field_0x20.w < fVar19)))) {
							bVar5 = false;
						}
						else {
							bVar5 = true;
						}

						pCurCell = this->aNaviCells + curCell;
						if (bVar5) {
							pNavMeshSubObj = this->aSubObjs;
							uVar4 = pCurCell->nbSubDataB;
							uVar12 = 0;
							puVar9 = pCurCell->pDataB;
							pNVar15 = pNavMeshSubObj + puVar9[uVar4 - 1];
							if (uVar4 != 0) {
								do {
									pfVar1 = &pNVar15->field_0x4;
									pfVar6 = &pNVar15->field_0x0;
									pNVar15 = pNavMeshSubObj + *puVar9;
									if ((fVar18 - *pfVar6) * (pNVar15->field_0x4 - *pfVar1) -
										(pNVar15->field_0x0 - *pfVar6) * (fVar19 - *pfVar1) < 0.0f) {
										bVar5 = false;
										goto LAB_001bc478;
									}

									uVar12 = uVar12 + 1;
									puVar9 = puVar9 + 1;
								} while (uVar12 < uVar4);
							}

							bVar5 = true;
						LAB_001bc478:
							if (bVar5) {
								return curCell & 0xffff;
							}
						}

						curCell = curCell + 1;
						pNaviCell = pNaviCell + 1;
					} while (curCell < maxCells);
				}
			}

			uVar17 = uVar17 + 1;
		} while (uVar17 < this->nbSubGraphs);
	}

	return 0xffff;
}

void CAstarBasicTransition::Create(ByteCode* pByteCode)
{

	this->field_0x2 = pByteCode->GetU16();
	this->field_0x0 = pByteCode->GetU16();
	this->field_0x8 = pByteCode->GetU16();
	this->field_0xa = pByteCode->GetU16();

	return;
}

void CAstarSubGraph::Create(ByteCode* pByteCode)
{
	uint uVar2;
	float fVar3;

	this->nbCells = pByteCode->GetU16();
	this->cellStart = pByteCode->GetU16();

	pByteCode->Align(4);

	uVar2 = 0;
	do {
		this->boundingBox.raw[uVar2] = pByteCode->GetF32();
		uVar2 = uVar2 + 1;
	} while (uVar2 < 0x10);

	return;
}
