#include "CollisionManager.h"
#include "MemoryStream.h"
#include "ActorFactory.h"
#include "ActorManager.h"
#include "port/pointer_conv.h"
#include "MathOps.h"
#include "Actor.h"
#include "SectorManager.h"
#include "ActorHero.h"

#include <math.h>
#include <string.h>
#include "CollisionRay.h"

struct ClusterCallbackParams
{
	CActorsTable* pTable;
	CActor* pActor;
};

void CCollisionManager::ClearStaticData()
{
	S_TIED_ACTOR_ENTRY* pSVar1;
	int iVar2;
	S_CARRY_ACTOR_ENTRY* pSVar3;

	iVar2 = 0x100;
	pSVar1 = CCollisionManager::_tied_actors_table;
	do {
		pSVar1->pActor = (CActor*)0x0;
		pSVar1->pNext = (S_TIED_ACTOR_ENTRY*)0x0;
		iVar2 = iVar2 + -8;
		pSVar1->pPrev = (S_TIED_ACTOR_ENTRY*)0x0;
		pSVar1[1].pActor = (CActor*)0x0;
		pSVar1[1].pNext = (S_TIED_ACTOR_ENTRY*)0x0;
		pSVar1[1].pPrev = (S_TIED_ACTOR_ENTRY*)0x0;
		pSVar1[2].pActor = (CActor*)0x0;
		pSVar1[2].pNext = (S_TIED_ACTOR_ENTRY*)0x0;
		pSVar1[2].pPrev = (S_TIED_ACTOR_ENTRY*)0x0;
		pSVar1[3].pActor = (CActor*)0x0;
		pSVar1[3].pNext = (S_TIED_ACTOR_ENTRY*)0x0;
		pSVar1[3].pPrev = (S_TIED_ACTOR_ENTRY*)0x0;
		pSVar1[4].pActor = (CActor*)0x0;
		pSVar1[4].pNext = (S_TIED_ACTOR_ENTRY*)0x0;
		pSVar1[4].pPrev = (S_TIED_ACTOR_ENTRY*)0x0;
		pSVar1[5].pActor = (CActor*)0x0;
		pSVar1[5].pNext = (S_TIED_ACTOR_ENTRY*)0x0;
		pSVar1[5].pPrev = (S_TIED_ACTOR_ENTRY*)0x0;
		pSVar1[6].pActor = (CActor*)0x0;
		pSVar1[6].pNext = (S_TIED_ACTOR_ENTRY*)0x0;
		pSVar1[6].pPrev = (S_TIED_ACTOR_ENTRY*)0x0;
		pSVar1[7].pActor = (CActor*)0x0;
		pSVar1[7].pNext = (S_TIED_ACTOR_ENTRY*)0x0;
		pSVar1[7].pPrev = (S_TIED_ACTOR_ENTRY*)0x0;
		pSVar1 = pSVar1 + 8;
	} while (0 < iVar2);

	iVar2 = 0x80;
	pSVar3 = CCollisionManager::_carry_info_table;
	do {
		pSVar3->field_0x80 = 0;
		pSVar3[1].field_0x80 = 0;
		iVar2 = iVar2 + -8;
		pSVar3[2].field_0x80 = 0;
		pSVar3[3].field_0x80 = 0;
		pSVar3[4].field_0x80 = 0;
		pSVar3[5].field_0x80 = 0;
		pSVar3[6].field_0x80 = 0;
		pSVar3[7].field_0x80 = 0;
		pSVar3 = pSVar3 + 8;
	} while (0 < iVar2);
	return;
}

MaterialTableEntry CCollisionManager::_material_table[MATERIAL_TABLE_SIZE] = {
	{0.5235988f, 0.6981317f, 1.0471976f, 1.0f, 1.0f},
	{0.5235988f, 0.6981317f, 1.0471976f, 1.0f, 1.0f},
	{0.5235988f, 0.6981317f, 1.0471976f, 1.0f, 1.0f},
	{0.5235988f, 0.6981317f, 1.0471976f, 1.0f, 1.0f},
	{0.5235988f, 0.6981317f, 1.0471976f, 1.0f, 1.0f},
	{0.0f,       0.0f,       3.1241393f, 1.0f, 0.2f},
	{0.5235988f, 0.6981317f, 1.0471976f, 1.0f, 1.0f},
	{0.5235988f, 0.6981317f, 1.0471976f, 1.0f, 1.0f},
	{0.5235988f, 0.6981317f, 1.0471976f, 1.0f, 1.0f},
	{0.5235988f, 0.6981317f, 1.0471976f, 1.0f, 1.0f},
	{0.5235988f, 0.6981317f, 1.0471976f, 1.0f, 1.0f},
	{0.5235988f, 0.6981317f, 1.0471976f, 1.0f, 1.0f},
};

CCollisionManager::CCollisionManager()
{
	this->bankCollisionCount = 0;
	this->loadedBankCount_0x8 = 0;
	this->pBankCollisionData = (BankCollision_14*)0x0;
	this->pCollisionDataArray = (CCollision*)0x0;
	this->inUseCollisionDataCount = 0;
	this->totalCollisionCount = 0;
	this->pDynColHeap = (DynColEntry*)0x0;

	for (int i = 0; i < 0x10; i++) {
		this->aStaticCollisionRefs[i] = 0;
	}

	this->staticCollisionCount = 0;
	return;
}

int __bmat_tab_initialised = 0;

void CCollisionManager::Level_Create(ByteCode* pMemoryStream)
{
	int iVar1;
	BankCollision_14* pColObj;
	int iVar3;

	iVar1 = pMemoryStream->GetS32();

	this->bankCollisionCount = iVar1;
	this->loadedBankCount_0x8 = 0;
	if (this->bankCollisionCount == 0) {
		this->pBankCollisionData = (BankCollision_14*)0x0;
	}
	else {
		this->pBankCollisionData = new BankCollision_14[this->bankCollisionCount];
	}

	for (iVar1 = 0; iVar1 < (int)this->bankCollisionCount; iVar1 = iVar1 + 1) {
		this->pBankCollisionData[iVar1].pBase = 0x0;
		this->pBankCollisionData[iVar1].dynColSize = 0;
		this->pBankCollisionData[iVar1].pNextFree = 0x0;
		this->pBankCollisionData[iVar1].useCount_0xc = 0;
		
		iVar3 = pMemoryStream->GetS32();
		this->pBankCollisionData[iVar1].field_0x8 = iVar3;
	}

	ClearStaticData();

	if (__bmat_tab_initialised == 0) {
		for (iVar1 = 0; iVar1 < MATERIAL_TABLE_SIZE; iVar1 = iVar1 + 1) {
			_material_table[iVar1].field_0x0 = cosf(_material_table[iVar1].field_0x0);
			_material_table[iVar1].field_0x4 = cosf(_material_table[iVar1].field_0x4);
			_material_table[iVar1].field_0x8 = cosf(_material_table[iVar1].field_0x8);
		}
		__bmat_tab_initialised = 1;
	}

	return;
}

void CCollisionManager::Level_PostCreate()
{
	uint count;
	int* pBase;
	CCollision* pCVar1;
	edColConfig* pDbConfig;
	int iVar2;
	ActorClassProperties* pAVar3;
	int iVar4;
	CActorManager* pCVar5;

	iVar4 = 0;
	this->totalCollisionCount = 0;
	if (0 < this->bankCollisionCount) {
		iVar2 = 0;
		do {
			this->totalCollisionCount = this->totalCollisionCount + pBankCollisionData[iVar4].field_0x8;
			iVar4 = iVar4 + 1;
		} while (iVar4 < this->bankCollisionCount);
	}

	iVar4 = 0;
	pAVar3 = CActorFactory::gClassProperties;
	pCVar5 = CScene::ptable.g_ActorManager_004516a4;
	do {
		if ((pAVar3->field_0x4 & 0x800) != 0) {
			this->totalCollisionCount = this->totalCollisionCount + pCVar5->aClassInfo[iVar4].totalCount;
		}
		iVar4 = iVar4 + 1;
		pAVar3 = pAVar3 + 1;
	} while (iVar4 < 0x57);

	count = this->totalCollisionCount;
	if (count != 0) {
		this->pCollisionDataArray = new CCollision[count];
	}

	this->inUseCollisionDataCount = 0;

	iVar4 = this->totalCollisionCount;
	if (iVar4 == 0) {
		iVar4 = 1;
	}

	pDbConfig = edColGetConfig();
	pDbConfig->bCreateProfileObj = 0;
	pDbConfig->field_0x1 = 1;
	pDbConfig->databaseCount = 1;
	pDbConfig->field_0x3 = 1;
	pDbConfig->field_0x4 = 0;
	pDbConfig->aDbTypeData[0].colObj.nbMax = iVar4;
	pDbConfig->aDbTypeData[0].dbObj.nbMax = 0x80;
	pDbConfig->aDbTypeData[0].primObj.nbMax = iVar4;
	pDbConfig->aDbTypeData[0].field_0xc.nbMax = 8;
	pDbConfig->field_0x18 = 8;
	pDbConfig->nbPrimEntries = 2;
	pDbConfig->field_0x1c = 6;
	pDbConfig->heapID_B = TO_HEAP(H_MAIN);
	pDbConfig->heapID_A = TO_HEAP(H_MAIN);
	pDbConfig->bSetMemFlags = 1;

	edColInit();
	edColSetDataBase(0);
	return;
}

bool CCollisionManager::IsASlidingGround(s_collision_contact* pContact)
{
	uint uVar1;

	uVar1 = pContact->materialFlags & 0xf;
	if (uVar1 == 0) {
		uVar1 = CScene::_pinstance->defaultMaterialIndex;
	}
	return (pContact->location).y < _material_table[uVar1].field_0x0;
}

float CCollisionManager::GetMaterialSlidingCoef(s_collision_contact* pContact)
{
	uint uVar1;

	uVar1 = pContact->materialFlags & 0xf;
	if (uVar1 == 0) {
		uVar1 = CScene::_pinstance->defaultMaterialIndex;
	}
	return _material_table[uVar1].slidingCoefficient;
}

CCollision* CCollisionManager::NewCCollision()
{
	int nextCollisionId;

	nextCollisionId = this->inUseCollisionDataCount;
	this->inUseCollisionDataCount = nextCollisionId + 1;
	return this->pCollisionDataArray + nextCollisionId;
}

void CCollisionManager::Level_AddAll(ByteCode* pByteCode)
{
	int* piVar1;
	int* piVar2;
	int dynColSize;
	int iVar4;
	BankCollision_14* pBankEntry;
	int iVar6;
	DynColEntry* pDynColEntry;
	DynColEntry* pNext;

	int dynColHeapSize = 0;
	pBankEntry = this->pBankCollisionData;
	for (iVar6 = this->bankCollisionCount; 0 < iVar6; iVar6 = iVar6 + -1) {
		dynColHeapSize = dynColHeapSize + pBankEntry->dynColSize * (pBankEntry->field_0x8 + -1);
		pBankEntry = pBankEntry + 1;
	}

	if (0 < dynColHeapSize) {

		this->pDynColHeap = (DynColEntry*)edMemAlloc(TO_HEAP(H_MAIN), dynColHeapSize);
		pDynColEntry = this->pDynColHeap;
		pBankEntry = this->pBankCollisionData;

		for (iVar6 = this->bankCollisionCount; 0 < iVar6; iVar6 = iVar6 + -1) {
			iVar4 = pBankEntry->field_0x8 + -1;

			if (0 < iVar4) {
				pBankEntry->pNextFree = pDynColEntry;
				dynColSize = pBankEntry->dynColSize;
				pNext = pBankEntry->pNextFree;
				pDynColEntry = (DynColEntry*)((char*)pDynColEntry + (dynColSize * iVar4));
				for (; iVar4 != 0; iVar4 = iVar4 + -1) {
					memcpy(pNext, pBankEntry->pBase, dynColSize);
					dynColSize = pBankEntry->dynColSize;
					pNext = (DynColEntry*)((char*)pNext + dynColSize);
				}
			}

			pBankEntry = pBankEntry + 1;
		}
	}
	return;
}

edColG3D_OBB_TREE_DYN* CCollisionManager::InstanciateDynCol(int index)
{
	byte bVar1;
	edF32MATRIX4* peVar2;
	undefined* puVar4;
	DynColEntry* pDynColEntry;
	BankCollision_14* pBVar5;
	edF32TRIANGLE4* pTriangle;
	edObbTREE_DYN* pObbTree;
	int iVar8;
	float* pfVar9;
	int iVar10;

	pBVar5 = this->pBankCollisionData + index;
	if (pBVar5->useCount_0xc == 0) {
		pDynColEntry = pBVar5->pBase;
	}
	else {
		pDynColEntry = (DynColEntry*)((char*)pBVar5->pNextFree + (pBVar5->dynColSize * (pBVar5->useCount_0xc + -1)));
	}

	{
		char szTag[9];
		memcpy(szTag, pDynColEntry->szTag, 8);
		szTag[8] = 0;
		assert(strcmp(".COLCOLI", szTag) == 0);
	}

	pBVar5->useCount_0xc = pBVar5->useCount_0xc + 1;
	if ((pDynColEntry->dynCollision).bLoaded == 0) {
		(pDynColEntry->dynCollision).bLoaded = 1;

		{
			const int srcVerticesOffset = (pDynColEntry->dynCollision).aSourceVertices;
			if (srcVerticesOffset != 0x0) {
				(pDynColEntry->dynCollision).aSourceVertices = STORE_SECTION((char*)pDynColEntry + srcVerticesOffset);
			}
		}

		{
			const int verticesOffset = (pDynColEntry->dynCollision).aVertices;
			if (verticesOffset != 0x0) {
				(pDynColEntry->dynCollision).aVertices = STORE_SECTION((char*)pDynColEntry + verticesOffset);
			}
		}

		{
			const int offset = (pDynColEntry->dynCollision).aTriangles;
			if (offset != 0) {
				(pDynColEntry->dynCollision).aTriangles = STORE_SECTION((char*)pDynColEntry + offset);
			}
		}

		{
			const int offset = (pDynColEntry->dynCollision).field_0x2c;
			if (offset != 0) {
				(pDynColEntry->dynCollision).field_0x2c = STORE_SECTION((char*)pDynColEntry + offset);
			}
		}

		{
			const int offset = (pDynColEntry->dynCollision).field_0x34;
			if (offset != 0) {
				(pDynColEntry->dynCollision).field_0x34 = STORE_SECTION((char*)pDynColEntry + offset);
			}
		}

		{
			const int offset = (pDynColEntry->dynCollision).field_0x30;
			if (offset != 0) {
				(pDynColEntry->dynCollision).field_0x30 = STORE_SECTION((char*)pDynColEntry + offset);
			}
		}

		const int obbTreeOffset = (pDynColEntry->dynCollision).pObbTree;
		if (obbTreeOffset != 0x0) {
			(pDynColEntry->dynCollision).pObbTree = STORE_SECTION((char*)pDynColEntry + obbTreeOffset);
		}

	
		pTriangle = (edF32TRIANGLE4*)LOAD_SECTION((pDynColEntry->dynCollision).aTriangles);
		for (iVar8 = (pDynColEntry->dynCollision).nbTriangles; iVar8 != 0; iVar8 = iVar8 + -1) {
			pTriangle->p1 = STORE_SECTION((char*)pDynColEntry + pTriangle->p1 * 0x10);
			pTriangle->p2 = STORE_SECTION((char*)pDynColEntry + pTriangle->p2 * 0x10);
			pTriangle->p3 = STORE_SECTION((char*)pDynColEntry + pTriangle->p3 * 0x10);
			pTriangle = pTriangle + 1;
		}

		edObbTREE_DYN* pObbTreeBase = (edObbTREE_DYN*)LOAD_SECTION((pDynColEntry->dynCollision).pObbTree);

		pObbTree = pObbTreeBase;
		for (iVar8 = (pDynColEntry->dynCollision).field_0x18; iVar8 != 0; iVar8 = iVar8 + -1) {

			bVar1 = pObbTree->type;
			if (bVar1 == 0xd) {
				pObbTree->field_0x54[0] = STORE_SECTION(((char*)LOAD_SECTION((pDynColEntry->dynCollision).field_0x30) + pObbTree->field_0x54[0] * 0x150));
			}
			else {
				if (bVar1 == COL_TYPE_PRIM_OBJ) {
					pObbTree->field_0x54[0] = STORE_SECTION(((char*)LOAD_SECTION((pDynColEntry->dynCollision).field_0x34) + pObbTree->field_0x54[0] * 0x150));
				}
				else {
					if (bVar1 == 4) {
						pObbTree->field_0x54[0] = STORE_SECTION(((char*)LOAD_SECTION((pDynColEntry->dynCollision).aTriangles) + pObbTree->field_0x54[0] * 0x10));
					}
					else {
						iVar10 = 0;
						if (bVar1 == COL_TYPE_TREE) {
							bVar1 = pObbTree->count_0x52;
							for (; iVar10 < (int)(uint)bVar1; iVar10 = iVar10 + 1) {
								edObbTREE_DYN* pNext = (edObbTREE_DYN*)((char*)pObbTreeBase + (pObbTree->field_0x54[iVar10] * sizeof(edObbTREE_DYN)));
								pObbTree->field_0x54[iVar10] = STORE_SECTION(pNext);
							}
						}
					}
				}
			}

			pObbTree += 1;
		}
	}
	return &pDynColEntry->dynCollision;
}

edObbTREE_DYN* CCollisionManager::InstallColFile(char* pFile, uint size)
{
	edColG3D_OBB_TREE* peVar1;

	peVar1 = edColLoadStatic(pFile, size, 0);
	this->aStaticCollisionRefs[this->staticCollisionCount] = (edObbTREE_DYN*)LOAD_SECTION(peVar1->pObbTree);
	this->staticCollisionCount = this->staticCollisionCount + 1;
	return (edObbTREE_DYN*)LOAD_SECTION(peVar1->pObbTree);
}

void CCollisionManager::UninstallColFile(edObbTREE_DYN* pObbTree)
{
	int iVar2;

	iVar2 = 0;
	for (; (iVar2 < this->staticCollisionCount && (this->aStaticCollisionRefs[iVar2] != pObbTree)); ) {
		iVar2 = iVar2 + 1;
	}

	if (iVar2 < this->staticCollisionCount) {
		this->staticCollisionCount = this->staticCollisionCount + -1;
		this->aStaticCollisionRefs[iVar2] = this->aStaticCollisionRefs[this->staticCollisionCount];
		this->aStaticCollisionRefs[this->staticCollisionCount] = (edObbTREE_DYN*)0x0;
	}
	return;
}

void CCollision::Create(CActor* pActor, int index)
{
	edColG3D_OBB_TREE_DYN* pDVar1;

	pDVar1 = CScene::ptable.g_CollisionManager_00451690->InstanciateDynCol(index);
	this->pDynCol = pDVar1;
	this->pObbTree = (edObbTREE_DYN*)LOAD_SECTION(this->pDynCol->pObbTree);
	SetupInternalData(pActor);
	return;
}

void CCollision::ClearInternalData()
{
	this->field_0x18 = 0;
	this->field_0x1c = (CActor*)0x0;
	this->actorFieldA = (CActor*)0x0;
	this->actorField = (CActor*)0x0;

	(this->field_0x90).x = 0.0f;
	(this->field_0x90).y = 0.0f;
	(this->field_0x90).z = 0.0f;
	(this->field_0x90).w = 0.0f;

	(this->highestVertex).x = 0.0f;
	(this->highestVertex).y = 0.0f;
	(this->highestVertex).z = 0.0f;
	(this->highestVertex).w = 1.0f;

	(this->lowestVertex).x = 0.0f;
	(this->lowestVertex).y = 0.0f;
	(this->lowestVertex).z = 0.0f;
	(this->lowestVertex).w = 1.0f;

	edF32Matrix4SetIdentityHard(&this->transformMatrix);
	return;
}

void CCollision::SetupInternalData(CActor* pActor)
{
	edColOBJECT* puVar1;
	edObbTREE_DYN* pObbTree;


	ClearInternalData();
	
	this->pTiedActorEntry = (S_TIED_ACTOR_ENTRY*)0x0;
	this->pCarryActorEntry = (S_CARRY_ACTOR_ENTRY*)0x0;

	this->flags_0x4 = 0;
	this->flags_0x0 = 0x1c503b;

	edColBegin(1, 0);
	edColObjectSetCollisionsType(1, 3, 1);
	edColCollisionAddPrim(1, 1, (void*)0x0, this->pObbTree, 2, 0, (void*)0x0);
	puVar1 = edColEnd(0);
	this->pColObj = puVar1;

	this->pColObj->pActor = STORE_SECTION(pActor);

	this->pObbPrim = 0;
	this->primType = 0;

	pObbTree = GetFirstObbPrimRecurse(this->pObbTree);
	if (pObbTree != 0) {
		this->pObbPrim = (edColPRIM_OBJECT*)LOAD_SECTION(pObbTree->field_0x54[0]);
		this->primType = pObbTree->type;
	}

	return;
}

static edF32VECTOR4 edF32VECTOR4_0040f120 = { 0.01f, 0.01f, 0.01f, 0.0f };

void CCollision::InvalidatePrims()
{
	byte bVar1;
	edColG3D_OBB_TREE_DYN* peVar2;
	edColPRIM_OBJECT* peVar3;
	edObbTREE_DYN* peVar4;
	edObbTREE_DYN* peVar5;
	undefined8 uVar6;
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
	edF32MATRIX4 eStack96;
	edF32VECTOR4 local_20;
	edF32VECTOR4 local_10;

	bVar1 = this->primType;
	peVar2 = this->pDynCol;
	peVar3 = this->pObbPrim;

	if (((bVar1 == 0xe) || (bVar1 == 0xd)) &&
		(peVar2->field_0x4 + peVar2->nbTriangles + peVar2->field_0x8 + peVar2->field_0xc == 1)) {
		edF32Matrix4FromEulerSoft(&this->pObbTree->matrix_0x70, &peVar3->field_0xa0.xyz, "XYZ");
		peVar4 = this->pObbTree;
		(peVar4->matrix_0x70).rowT = peVar3->field_0xb0;

		if (bVar1 == 0xe) {
			local_10 = peVar3->field_0x90;
		}
		else {
			local_10 = peVar3->field_0x90 * 0.5f;
		}

		local_20 = edF32VECTOR4_0040f120;

		peVar4 = this->pObbTree;

		(peVar4->field_0xb0).x = local_10.x + local_20.x;
		(peVar4->field_0xb0).y = local_10.y + local_20.y;
		(peVar4->field_0xb0).z = local_10.z + edF32VECTOR4_0040f120.z;
		(peVar4->field_0xb0).w = local_10.w + edF32VECTOR4_0040f120.w;


		peVar4 = this->pObbTree;
		peVar5 = this->pObbTree;

		(peVar5->bbox).transform = peVar4->matrix_0x70;

		(peVar5->bbox).width = peVar4->field_0xb0.x;
		(peVar5->bbox).height = peVar4->field_0xb0.y;
		(peVar5->bbox).depth = peVar4->field_0xb0.z;
		(peVar5->bbox).field_0x4c = peVar4->field_0xb0.w;

		edF32Matrix4GetInverseOrthoHard(&eStack96, &this->transformMatrix);
		SetObbTreeMatrixRecurse(this->pObbTree, &this->transformMatrix, &eStack96);
	}
	else {
		this->flags_0x4 = this->flags_0x4 & 0xef;
	}
	return;
}

void CCollision::Reset()
{
	ClearInternalData();

	this->flags_0x4 = this->flags_0x4 & 0xe8;

	return;
}

edObbTREE_DYN* CCollision::GetFirstObbPrimRecurse(edObbTREE_DYN* pObbTree)
{
	byte bVar1;
	edObbTREE_DYN* pMainTree;
	edObbTREE_DYN* pResult;
	int iVar6;
	int iVar7;

	bVar1 = pObbTree->type;
	if ((bVar1 != 0xe) && (bVar1 != 0xd)) {
		if ((bVar1 == COL_TYPE_TREE) && (iVar7 = 0, pObbTree->count_0x52 != 0)) {
			do {
				pMainTree = (edObbTREE_DYN*)LOAD_SECTION(pObbTree->field_0x54[iVar7]);
				bVar1 = pMainTree->type;
				pResult = pMainTree;
				if ((bVar1 != 0xe) && (bVar1 != 0xd)) {
					if ((bVar1 == COL_TYPE_TREE) && (iVar6 = 0, pMainTree->count_0x52 != 0)) {
						do {
							pResult = GetFirstObbPrimRecurse((edObbTREE_DYN*)LOAD_SECTION(pMainTree->field_0x54[iVar6]));

							if (pResult != (edObbTREE_DYN*)0x0) goto LAB_001e1da0;

							iVar6 = iVar6 + 1;				
						} while (iVar6 < (int)(uint)pMainTree->count_0x52);
					}
					pResult = (edObbTREE_DYN*)0x0;
				}

			LAB_001e1da0:
				if (pResult != (edObbTREE_DYN*)0x0) {
					return pResult;
				}

				iVar7 = iVar7 + 1;
			} while (iVar7 < pObbTree->count_0x52);
		}

		pObbTree = (edObbTREE_DYN*)0x0;
	}

	return pObbTree;
}

void CCollision::PatchObbTreeFlagsRecurse(edObbTREE_DYN* pObbTree, int param_2, int param_3, int param_4)
{
	byte obbType;
	int iVar2;
	int iVar3;
	int iVar4;
	edObbTREE_DYN* peVar5;

	obbType = pObbTree->type;
	if (obbType == COL_TYPE_TREE) {
		iVar4 = 0;
		if (pObbTree->count_0x52 != 0) {
			do {
				PatchObbTreeFlagsRecurse((edObbTREE_DYN*)LOAD_SECTION(pObbTree->field_0x54[iVar4]), param_2, param_3, param_4);
				iVar4 = iVar4 + 1;
			} while (iVar4 < pObbTree->count_0x52);
		}
	}
	else {
		if (obbType == 4) {
			IMPLEMENTATION_GUARD(
			iVar4 = 0;
			if (pObbTree->count_0x52 != 0) {
				iVar2 = 0;
				do {
					iVar4 = iVar4 + 1;
					iVar3 = (int)&(pObbTree->field_0x54->vertices).aa + iVar2;
					*(uint*)(iVar3 + 0xc) = *(uint*)(iVar3 + 0xc) & param_3;
					*(uint*)(iVar3 + 0xc) = *(uint*)(iVar3 + 0xc) | param_2;
					*(uint*)(iVar3 + 0xc) = *(uint*)(iVar3 + 0xc) ^ param_4;
					iVar2 = iVar2 + 0x10;
				} while (iVar4 < pObbTree->count_0x52);
			})
		}
		else {
			if (((obbType == COL_TYPE_PRIM_OBJ) || (obbType == 0xd)) && (iVar4 = 0, pObbTree->count_0x52 != 0)) {
				iVar2 = 0;
				edColPRIM_OBJECT* pPrimObj = (edColPRIM_OBJECT*)LOAD_SECTION(pObbTree->field_0x54[0]);
				do {					
					pPrimObj[iVar4].flags_0x80 = pPrimObj[iVar4].flags_0x80 & param_3;
					pPrimObj[iVar4].flags_0x80 = pPrimObj[iVar4].flags_0x80 | param_2;
					pPrimObj[iVar4].flags_0x80 = pPrimObj[iVar4].flags_0x80 ^ param_4;

					iVar4 = iVar4 + 1;
				} while (iVar4 < pObbTree->count_0x52);
			}
		}
	}
	return;
}

void CCollision::SetObbTreePositionRecurse(edObbTREE_DYN* pObbTree, edF32MATRIX4* pMatrix)
{
	byte bVar1;
	int iVar2;
	edColPRIM_OBJECT* peVar3;
	edF32VECTOR4 local_10;

	edF32Matrix4MulF32Vector4Hard(&(pObbTree->bbox).transform.rowT, pMatrix, &pObbTree->matrix_0x70.rowT);

	bVar1 = pObbTree->type;
	iVar2 = 0;

	if (bVar1 == COL_TYPE_TREE) {
		for (; iVar2 < pObbTree->count_0x52; iVar2 = iVar2 + 1) {
			SetObbTreePositionRecurse(LOAD_SECTION_CAST(edObbTREE_DYN*, pObbTree->field_0x54[iVar2]), pMatrix);
		}
	}
	else {
		if (((bVar1 != 8) && (bVar1 != 4)) && ((bVar1 == 0xe || (bVar1 == 0xd)))) {
			edColPRIM_OBJECT* pPrim = LOAD_SECTION_CAST(edColPRIM_OBJECT*, pObbTree->field_0x54[0]);
			for (iVar2 = 0; iVar2 < pObbTree->count_0x52; iVar2 = iVar2 + 1) {
				peVar3 = pPrim + iVar2;

				edF32Matrix4MulF32Vector4Hard(&peVar3->vertices.rowT, pMatrix, &peVar3->field_0xb0);

				local_10.w = peVar3->vertices.rowT.w;
				local_10.x = 0.0f - peVar3->vertices.rowT.x;
				local_10.y = 0.0f - peVar3->vertices.rowT.y;
				local_10.z = 0.0f - peVar3->vertices.rowT.z;

				(peVar3->worldTransform).rowT.x = 0.0f;
				(peVar3->worldTransform).rowT.y = 0.0f;
				(peVar3->worldTransform).rowT.z = 0.0f;
				(peVar3->worldTransform).rowT.w = 1.0f;

				edF32Matrix4MulF32Vector4Hard(&local_10, &peVar3->worldTransform, &local_10);

				(peVar3->worldTransform).rowT = local_10;

				COLLISION_LOG(LogLevel::Verbose, "SetObbTreePositionRecurse new transform: {}", peVar3->worldTransform.ToString());
			}
		}
	}
	return;
}

void CCollision::SetObbTreeMatrixNoRotationRecurse(edObbTREE_DYN* pObbTree, edF32MATRIX4* param_2, edF32MATRIX4* param_3)
{
	byte obbType;
	edObbTREE_DYN* peVar2;
	int iVar3;
	int iVar4;
	edF32MATRIX4* m0;
	edF32VECTOR4 local_10;

	edF32Matrix4TranslateHard(&pObbTree->bbox.transform, &pObbTree->matrix_0x70, &param_2->rowT);

	obbType = pObbTree->type;

	if (obbType == COL_TYPE_TREE) {
		iVar4 = 0;
		if (pObbTree->count_0x52 != 0) {
			do {
				SetObbTreeMatrixNoRotationRecurse((edObbTREE_DYN*)LOAD_SECTION(pObbTree->field_0x54[iVar4]), param_2, param_3);
				iVar4 = iVar4 + 1;
			} while (iVar4 < pObbTree->count_0x52);
		}
	}
	else {
		if ((((obbType != 8) && (obbType != 4)) && ((obbType == COL_TYPE_PRIM_OBJ || (obbType == 0xd)))) && (iVar4 = 0, pObbTree->count_0x52 != 0))
		{
			edColPRIM_OBJECT* pPrimObj = (edColPRIM_OBJECT*)LOAD_SECTION(pObbTree->field_0x54[0]);

			do {
				edColComputeMatrices(&pPrimObj[iVar4]);
				edF32Matrix4TranslateHard(&pPrimObj[iVar4].vertices, &pPrimObj[iVar4].vertices, &param_2->rowT);
				edF32Matrix4MulF32Vector4Hard(&local_10, &pPrimObj[iVar4].worldTransform, &param_3->rowT);
				(pPrimObj[iVar4].worldTransform).rowT = local_10;
				COLLISION_LOG(LogLevel::Verbose, "SetObbTreeMatrixNoRotationRecurse new transform: {}", (pPrimObj[iVar4].worldTransform).ToString());
				iVar4 = iVar4 + 1;
			} while (iVar4 < pObbTree->count_0x52);
		}
	}
	return;
}

void CCollision::ComputePrimLowestAndHighestVertices(edF32VECTOR4* pHighestVertex, edF32VECTOR4* pLowestVertex, int param_3, edColPRIM_OBJECT* pPrimObj, byte type)
{
	edF32VECTOR4* v1;
	float fVar1;
	float fVar2;
	float fVar3;

	if ((param_3 == 0) && (pPrimObj->field_0xa0.z + pPrimObj->field_0xa0.x + pPrimObj->field_0xa0.y == 0.0f)) {
	
		*pHighestVertex = (pPrimObj->vertices).rowT;
		*pLowestVertex = (pPrimObj->vertices).rowT;

		if (type == 0xd) {
			pHighestVertex->y = pHighestVertex->y - pPrimObj->field_0x90.y * 0.5f;
			pLowestVertex->y = pLowestVertex->y + pPrimObj->field_0x90.y * 0.5f;
		}
		else {
			pHighestVertex->y = pHighestVertex->y - pPrimObj->field_0x90.y;
			pLowestVertex->y = pLowestVertex->y + pPrimObj->field_0x90.y;
		}
	}
	else {
		v1 = &pPrimObj->worldTransform.rowY;
		if (type == 0xd) {
			if (v1->x <= 0.0f) {
				pLowestVertex->x = -1.0f;
			}
			else {
				pLowestVertex->x = 0.5f;
			}
			if ((pPrimObj->worldTransform).bb <= 0.0f) {
				pLowestVertex->y = -1.0f;
			}
			else {
				pLowestVertex->y = 0.5f;
			}
			if ((pPrimObj->worldTransform).bc <= 0.0f) {
				pLowestVertex->z = -1.0f;
			}
			else {
				pLowestVertex->z = 0.5f;
			}
		}
		else {
			edF32Vector4NormalizeHard(pLowestVertex, v1);
		}

		pLowestVertex->w = 1.0f;
	
		pHighestVertex->x = 0.0f - pLowestVertex->x;
		pHighestVertex->y = 0.0f - pLowestVertex->y;
		pHighestVertex->z = 0.0f - pLowestVertex->z;
		pHighestVertex->w = pLowestVertex->w;

		edF32Matrix4MulF32Vector4Hard(pHighestVertex, &pPrimObj->vertices, pHighestVertex);
		edF32Matrix4MulF32Vector4Hard(pLowestVertex, &pPrimObj->vertices, pLowestVertex);
	}
	return;
}

void CCollision::SetObbTreeMatrixRecurse(edObbTREE_DYN* pObbTree, edF32MATRIX4* param_2, edF32MATRIX4* param_3)
{
	byte bVar1;
	edObbTREE_DYN* peVar2;
	int iVar3;
	int iVar4;
	edColPRIM_OBJECT* pPrimObj;

	edF32Matrix4MulF32Matrix4Hard(&pObbTree->bbox.transform, &pObbTree->matrix_0x70, param_2);
	bVar1 = pObbTree->type;
	if (bVar1 == COL_TYPE_TREE) {
		iVar3 = 0;
		if (pObbTree->count_0x52 != 0) {
			do {
				SetObbTreeMatrixRecurse((edObbTREE_DYN*)LOAD_SECTION(pObbTree->field_0x54[iVar3]), param_2, param_3);
				iVar3 = iVar3 + 1;
			} while (iVar3 < pObbTree->count_0x52);
		}
	}
	else {
		if ((((bVar1 != 8) && (bVar1 != 4)) && ((bVar1 == 0xe || (bVar1 == 0xd)))) && (iVar3 = 0, pObbTree->count_0x52 != 0)) {
			pPrimObj = (edColPRIM_OBJECT*)LOAD_SECTION(pObbTree->field_0x54[0]);
			do {
				edColComputeMatrices(&pPrimObj[iVar3]);
				edF32Matrix4MulF32Matrix4Hard(&pPrimObj[iVar3].vertices, &pPrimObj[iVar3].vertices, param_2);
				edF32Matrix4MulF32Matrix4Hard(&pPrimObj[iVar3].worldTransform, param_3, &pPrimObj[iVar3].worldTransform);
				COLLISION_LOG(LogLevel::Verbose, "SetObbTreeMatrixRecurse new transform: {}", (pPrimObj[iVar3].worldTransform).ToString());
				iVar3 = iVar3 + 1;
			} while (iVar3 < pObbTree->count_0x52);
		}
	}
	return;
}

void CCollision::ComputeObbTreeLowestAndHighestVerticesRecurse(edF32VECTOR4* pHighestVertex, edF32VECTOR4* pLowestVertex, int param_3, edObbTREE_DYN* pObbTree)
{
	byte colType;
	int index;
	edF32VECTOR4 outLowestVertex;
	edF32VECTOR4 outHighestVertex;

	colType = pObbTree->type;

	if (colType == COL_TYPE_TREE) {
		index = 0;
		if (pObbTree->count_0x52 != 0) {
			do {
				ComputeObbTreeLowestAndHighestVerticesRecurse(pHighestVertex, pLowestVertex, param_3, (edObbTREE_DYN*)LOAD_SECTION(pObbTree->field_0x54[index]));
				index = index + 1;
			} while (index < pObbTree->count_0x52);
		}
	}
	else {
		if ((((colType != 8) && (colType != 4)) && ((colType == COL_TYPE_PRIM_OBJ || (colType == 0xd)))) && (index = 0, pObbTree->count_0x52 != 0)) {

			edColPRIM_OBJECT* pPrimObj = (edColPRIM_OBJECT*)LOAD_SECTION(pObbTree->field_0x54[0]);

			do {
				ComputePrimLowestAndHighestVertices(&outHighestVertex, &outLowestVertex, param_3, &pPrimObj[index], pObbTree->type);
				if (outHighestVertex.y < pHighestVertex->y) {
					*pHighestVertex = outHighestVertex;
				}
				if (pLowestVertex->y < outLowestVertex.y) {
					*pLowestVertex = outLowestVertex;
				}
				index = index + 1;
			} while (index < pObbTree->count_0x52);
		}
	}
	return;
}

void CCollision::ComputeG3DObbTreeLowestAndHighestVertices(edF32VECTOR4* pHighestVertex, edF32VECTOR4* pLowestVertex, int param_3, edColG3D_OBB_TREE_DYN* pDynCol)
{
	edF32VECTOR4* pVertex;
	int iVar2;

	pHighestVertex->y = 1e+30f;
	pLowestVertex->y = -1e+30f;

	ComputeObbTreeLowestAndHighestVerticesRecurse(pHighestVertex, pLowestVertex, param_3, (edObbTREE_DYN*)LOAD_SECTION(pDynCol->pObbTree));

	pVertex = (edF32VECTOR4*)LOAD_SECTION(pDynCol->aVertices);
	for (iVar2 = pDynCol->vertexCount; iVar2 != 0; iVar2 = iVar2 + -1) {
		if (pVertex->y < pHighestVertex->y) {
			*pHighestVertex = *pVertex;
		}

		if (pLowestVertex->y < pVertex->y) {
			*pLowestVertex = *pVertex;
		}

		pVertex = pVertex + 1;
	}

	return;
}

void CCollision::TransformG3DObbTreeVertices(edColG3D_OBB_TREE_DYN* pDynCol, int matrixType, edF32MATRIX4* pTransformMatrix)
{
	edF32VECTOR4* pResultVertex;
	edF32VECTOR4* pSrcVertex;
	int index;

	index = pDynCol->vertexCount;
	pSrcVertex = (edF32VECTOR4*)LOAD_SECTION(pDynCol->aSourceVertices);
	pResultVertex = (edF32VECTOR4*)LOAD_SECTION(pDynCol->aVertices);
	if (matrixType == 0) {
		const edF32VECTOR4 rowT = pTransformMatrix->rowT;
		for (; index != 0; index = index + -1) {
			pResultVertex->x = rowT.x + pSrcVertex->x;
			pResultVertex->y = rowT.y + pSrcVertex->y;
			pResultVertex->z = rowT.z + pSrcVertex->z;
			pResultVertex->w = pSrcVertex->w; + 0.0f;

			pSrcVertex = pSrcVertex + 1;
			pResultVertex = pResultVertex + 1;
		}
	}
	else {
		for (; index != 0; index = index + -1) {
			*pResultVertex = (*pSrcVertex) * (*pTransformMatrix);

			pSrcVertex = pSrcVertex + 1;
			pResultVertex = pResultVertex + 1;
		}
	}
	return;
}

float CCollisionManager::GetWallNormalYLimit(s_collision_contact* pContact)
{
	uint uVar1;

	uVar1 = pContact->materialFlags & 0xf;

	if (uVar1 == 0) {
		uVar1 = CScene::_pinstance->defaultMaterialIndex;
	}

	return _material_table[uVar1].field_0x8;
}

float CCollisionManager::GetGroundSpeedDecreaseNormalYLimit(s_collision_contact* pContact)

{
	uint uVar1;

	uVar1 = pContact->materialFlags & 0xf;

	if (uVar1 == 0) {
		uVar1 = CScene::_pinstance->defaultMaterialIndex;
	}

	return _material_table[uVar1].field_0x4;
}

int CCollision::GetPrimNumDeltaSubdivisions(edColPRIM_OBJECT* pPrimObj, byte colType, edF32VECTOR4* param_3)
{
	float fVar1;
	int iVar2;
	float fVar3;
	edF32VECTOR4 local_10;

	iVar2 = 1;

	edF32Matrix4MulF32Vector4Hard(&local_10, &pPrimObj->worldTransform, param_3);
	if (colType == 0xd) {
		fVar1 = 0.0f;

		for (iVar2 = 0; iVar2 < 3; iVar2 = iVar2 + 1) {
			local_10.raw[iVar2] = fabs(local_10.raw[iVar2]);

			fVar3 = local_10.raw[iVar2];

			if (fVar3 <= fVar1) {
				fVar3 = fVar1;
			}

			fVar1 = fVar3;
		}

		iVar2 = (int)(fVar1 * 4.0f + 1.0f);
	}
	else {
		if (colType == 0xe) {
			fVar1 = edF32Vector4GetDistHard(&local_10);

			iVar2 = (int)(fVar1 * 2.0f + 1.0f);
		}
	}

	return iVar2;
}

bool CCollision::IsVertexAboveAndAgainstObbTree(edF32VECTOR4* v0, edObbTREE_DYN* pObbTree)
{
	bool bVar1;
	float fVar2;
	CCollisionRay local_30;
	void* peStack8;
	uint uStack4;

	static edF32VECTOR4 ray_dir = { 0.0f, -1.0f, 0.0f, 0.0f };

	local_30.lengthB.z = v0->z;
	local_30.lengthB.w = v0->w;
	local_30.pLocation = &local_30.lengthB;
	local_30.lengthB.x = v0->x;
	local_30.pLeadVector = &ray_dir;
	local_30.lengthB.y = v0->y + 0.1f;
	local_30.lengthA = 0.25f;
	fVar2 = edObbIntersectObbTreeRayPrim(&peStack8, &uStack4, pObbTree, &local_30);
	bVar1 = 0.0f <= fVar2;
	if ((bVar1) && (bVar1 = true, local_30.lengthA <= fVar2)) {
		bVar1 = false;
	}
	return bVar1;
}

float CCollision::GetPrimDistanceFromPosition(edColPRIM_OBJECT* param_1, byte param_2, edF32VECTOR4* param_3)
{
	int iVar1;
	float fVar2;
	edF32VECTOR4 local_20;
	edF32VECTOR4 local_10;

	edF32Matrix4MulF32Vector4Hard(&local_10, &param_1->worldTransform, param_3);

	if (param_2 == 0xe) {
		local_10.w = 0.0f;
		edF32Vector4SafeNormalize0Hard(&local_20, &local_10);
		local_20.w = 1.0f;
	}
	else {
		if (param_2 == 0xd) {
			local_20 = local_10;

			if (0.5f <= fabs(local_20.x)) {
				iVar1 = 1;

				if (local_20.x <= 0.0f) {
					iVar1 = -1;
				}

				local_20.x = (float)iVar1 * 0.5f;
			}

			if (0.5f <= fabs(local_20.y)) {
				iVar1 = 1;

				if (local_20.y <= 0.0f) {
					iVar1 = -1;
				}

				local_20.y = (float)iVar1 * 0.5f;
			}

			if (0.5f <= fabs(local_10.z)) {
				iVar1 = 1;
				if (local_10.z <= 0.0f) {
					iVar1 = -1;
				}

				local_20.z = (float)iVar1 * 0.5f;
			}
		}
	}

	edF32Matrix4MulF32Vector4Hard(&local_20, &param_1->vertices, &local_20);
	local_20 = local_20 - *param_3;
	fVar2 = edF32Vector4GetDistHard(&local_20);

	return fVar2;
}

float CCollision::GetWallNormalYLimit(s_collision_contact* pContact)
{
	return CCollisionManager::GetWallNormalYLimit(pContact);
}

float CCollision::GetObbTreeDistanceFromPosition(edObbTREE_DYN* pObbTree, edF32VECTOR4* v0)
{
	edObbTREE* peVar1;
	int iVar2;
	int iVar3;
	float fVar4;
	float fVar5;

	fVar5 = 1e+30f;
	fVar4 = 1e+30f;

	if (pObbTree->type == 1) {
		iVar3 = 0;
		fVar4 = fVar5;

		if (pObbTree->count_0x52 != 0) {
			do {
				fVar4 = GetObbTreeDistanceFromPosition(LOAD_SECTION_CAST(edObbTREE_DYN*, pObbTree->field_0x54[iVar3]), v0);

				if (fVar5 <= fVar4) {
					fVar4 = fVar5;
				}

				iVar3 = iVar3 + 1;
				fVar5 = fVar4;
			} while (iVar3 < pObbTree->count_0x52);
		}
	}
	else {
		iVar3 = 0;
		if (pObbTree->count_0x52 != 0) {
			edColPRIM_OBJECT* pPrimObj = LOAD_SECTION_CAST(edColPRIM_OBJECT*, pObbTree->field_0x54[0]);
			do {
				fVar4 = GetPrimDistanceFromPosition(pPrimObj + iVar3, pObbTree->type, v0);

				if (fVar5 <= fVar4) {
					fVar4 = fVar5;
				}

				iVar3 = iVar3 + 1;
				fVar5 = fVar4;
			} while (iVar3 < pObbTree->count_0x52);
		}
	}

	return fVar4;
}

void CCollision::SetObbTreePositionNoRotationRecurse(edObbTREE_DYN* pObbTree, edF32MATRIX4* param_2)
{
	byte colType;
	int iVar2;
	edColPRIM_OBJECT* pPrimObj;
	float fVar3;
	float fVar4;
	float fVar5;
	float fVar6;
	float fVar7;
	float fVar8;
	edF32VECTOR4 local_10;

	(pObbTree->bbox.transform).rowT = param_2->rowT + pObbTree->matrix_0x70.rowT;
	(pObbTree->bbox.transform).rowT.w = 1.0f;

	colType = pObbTree->type;
	iVar2 = 0;

	if (colType == COL_TYPE_TREE) {
		for (; iVar2 < pObbTree->count_0x52; iVar2 = iVar2 + 1) {
			SetObbTreePositionNoRotationRecurse((edObbTREE_DYN*)LOAD_SECTION(pObbTree->field_0x54[iVar2]), param_2);
		}
	}
	else {
		if (((colType != 8) && (colType != 4)) && ((colType == 0xe || (colType == 0xd)))) {
			edColPRIM_OBJECT* pPrimObj = (edColPRIM_OBJECT*)LOAD_SECTION(pObbTree->field_0x54[0]);
			for (iVar2 = 0; iVar2 < pObbTree->count_0x52; iVar2 = iVar2 + 1) {

				pPrimObj[iVar2].vertices.rowT = param_2->rowT + pPrimObj[iVar2].field_0xb0;
				pPrimObj[iVar2].vertices.rowT.w = 1.0f;

				local_10.w = pPrimObj[iVar2].vertices.rowT.w;
				local_10.x = 0.0f - pPrimObj[iVar2].vertices.rowT.x;
				local_10.y = 0.0f - pPrimObj[iVar2].vertices.rowT.y;
				local_10.z = 0.0f - pPrimObj[iVar2].vertices.rowT.z;

				(pPrimObj[iVar2].worldTransform).rowT.x = 0.0f;
				(pPrimObj[iVar2].worldTransform).rowT.y = 0.0f;
				(pPrimObj[iVar2].worldTransform).rowT.z = 0.0f;
				(pPrimObj[iVar2].worldTransform).rowT.w = 1.0f;

				edF32Matrix4MulF32Vector4Hard(&local_10, &pPrimObj[iVar2].worldTransform, &local_10);

				(pPrimObj[iVar2].worldTransform).rowT = local_10;
				COLLISION_LOG(LogLevel::Verbose, "SetObbTreePositionNoRotationRecurse new transform: {}", (pPrimObj[iVar2].worldTransform).ToString());
			}
		}
	}
	return;
}

void CCollision::UpdateMatrix(edF32MATRIX4* param_2)
{
	int iVar1;
	S_CARRY_ACTOR_ENTRY* peVar2;
	int lVar3;
	edF32MATRIX4* peVar4;
	float fVar5;
	float fVar6;
	float fVar7;
	edF32MATRIX4 local_40;

	if ((this->flags_0x0 & 0x800) == 0) {
		this->transformMatrix = gF32Matrix4Unit;

		local_40 = gF32Matrix4Unit;
		
		this->transformMatrix.rowT = param_2->rowT;

		local_40.rowT = param_2->rowT;
		
		if ((this->flags_0x4 & 0x10) == 0) {
			SetObbTreeMatrixNoRotationRecurse(this->pObbTree, &this->transformMatrix, &local_40);
			this->flags_0x4 = this->flags_0x4 | 0x10;
		}
		else {
			SetObbTreePositionNoRotationRecurse(this->pObbTree, &this->transformMatrix);
		}

		if (0 < this->pDynCol->vertexCount) {
			TransformG3DObbTreeVertices(this->pDynCol, 0, &this->transformMatrix);
		}

		lVar3 = 0;
	}
	else {
		this->transformMatrix = *param_2;

		
		edF32Matrix4GetInverseOrthoHard(&local_40, &this->transformMatrix);
		SetObbTreeMatrixRecurse(this->pObbTree, &this->transformMatrix, &local_40);

		this->flags_0x4 = this->flags_0x4 & 0xef;

		if (0 < this->pDynCol->vertexCount) {
			TransformG3DObbTreeVertices(this->pDynCol, 1, &this->transformMatrix);
		}
		lVar3 = 1;
	}
	if ((this->flags_0x0 & 0x40000) == 0) {
		this->lowestVertex = this->transformMatrix.rowT;
		this->highestVertex = this->lowestVertex;
	}
	else {
		ComputeG3DObbTreeLowestAndHighestVertices(&this->highestVertex, &this->lowestVertex, lVar3, this->pDynCol);
	}
	
	peVar2 = this->pCarryActorEntry;
	if (peVar2 != (S_CARRY_ACTOR_ENTRY*)0x0) {
		edF32Matrix4MulF32Matrix4Hard(&peVar2->m1, &peVar2->m0, &this->transformMatrix);
	}
	return;
}


void CCollision::TranslateList(CActor* pActor, CActorsTable* param_3, edF32MATRIX4* m0, CActor* param_5)
{
	CActor* pCVar1;
	float fVar2;
	float fVar3;
	float fVar4;
	CActor* pCVar5;
	CActor* pCVar6;
	bool bVar7;
	int iVar8;
	int iVar9;
	edF32MATRIX4 local_a0;
	edF32VECTOR4 local_60;
	edF32VECTOR4 local_50;
	edF32MATRIX4 local_40;

	fVar2 = m0->da - (this->transformMatrix).da;
	fVar3 = m0->db - (this->transformMatrix).db;
	fVar4 = m0->dc - (this->transformMatrix).dc;

	local_60.x = m0->aa - (this->transformMatrix).aa;
	local_60.y = m0->bb - (this->transformMatrix).bb;
	local_60.z = m0->cc - (this->transformMatrix).cc;
	local_60.w = 0.0f;

	if (fVar2 * fVar2 + fVar3 * fVar3 + fVar4 * fVar4 + local_60.x * local_60.x + local_60.y * local_60.y + local_60.z * local_60.z != 0.0f) {
		if ((this->transformMatrix).cc + (this->transformMatrix).aa + (this->transformMatrix).bb == 3.0f) {
			local_a0.rowX = gF32Matrix4Unit.rowX;
			local_a0.rowY = gF32Matrix4Unit.rowY;
			local_a0.rowZ = gF32Matrix4Unit.rowZ;
			
			local_a0.dd = (this->transformMatrix).dd;
			local_a0.da = 0.0f - (this->transformMatrix).da;
			local_a0.db = 0.0f - (this->transformMatrix).db;
			local_a0.dc = 0.0f - (this->transformMatrix).dc;
		}
		else {
			edF32Matrix4GetInverseOrthoHard(&local_a0, &this->transformMatrix);
		}

		edF32Matrix4MulF32Matrix4Hard(&local_40, &local_a0, m0);

		iVar9 = param_3->entryCount;

		while (0 < iVar9) {
			iVar8 = iVar9 + -1;
			pCVar1 = param_3->aEntries[iVar9 + -1];
			iVar9 = iVar8;

			if ((param_5 != pCVar1) && (pCVar1->flags = pCVar1->flags | 0x40000, (CActorFactory::gClassProperties[pCVar1->typeID].flags & 0x40) != 0))
			{
				bVar7 = true;
				if ((pActor != pCVar1->pTiedActor) &&
					((pCVar1->pCollisionData != (CCollision*)0x0 && ((pCVar1->pCollisionData->flags_0x0 & 0x80) != 0)))) {
					bVar7 = false;
				}

				if (((bVar7) && (pCVar6 = pCVar1->pTiedActor, pCVar6 != (CActor*)0x0)) && (pCVar6 != pActor)) {
					for (; pCVar5 = pActor, pCVar6->pTiedActor != (CActor*)0x0; pCVar6 = pCVar6->pTiedActor) {
					}
					for (; pCVar5->pTiedActor != (CActor*)0x0; pCVar5 = pCVar5->pTiedActor) {
					}
					if (pCVar6 == pCVar5) {
						bVar7 = false;
					}
				}
				if (bVar7) {
					pCVar1->CarriedByActor(pActor, &local_40);
				}
			}
		}
	}

	return;
}



void gClusterCallback_Collision(CActor* pActor, void* pInParams)
{
	CCollision* pCollisionData;
	uint overlapColFlags;
	uint actorColFlags;

	ClusterCallbackParams* pParams = (ClusterCallbackParams*)pInParams;

	if ((((pActor != pParams->pActor) && (pCollisionData = pActor->pCollisionData, pCollisionData != (CCollision*)0x0)) &&
		(overlapColFlags = pCollisionData->flags_0x0, (overlapColFlags & 0x1000) != 0)) &&
		(pCollisionData = pParams->pActor->pCollisionData, pCollisionData->actorFieldA != pActor)) {
		actorColFlags = pCollisionData->flags_0x0;
		if (((overlapColFlags & actorColFlags & 0x800000) == 0) &&
			((((overlapColFlags & 0x2000) == 0 || ((overlapColFlags & 0x8000) != 0)) || ((actorColFlags & 0x4000) == 0)))) {
			pParams->pTable->Add(pActor);
		}
	}
	return;
}

uint CCollision::CheckCollisionsWithActors(CActor* pActor, edF32MATRIX4* m0)
{
	int entryCount;
	uint lastResult;
	int currentEntryIndex;
	uint result;
	edColINFO_OBBTREE_OBBTREE colInfoObbTreeObbTree;
	edF32VECTOR4 worldBoundingSphere;
	edF32MATRIX4 worldBoundingSphereMatrix;
	CFixedTable<CActor*, 64> actorIntersectTable;
	ClusterCallbackParams clusterCallbackParams;
	CActorHero* pActorHero;

	COLLISION_LOG(LogLevel::VeryVerbose, "CCollision::CheckCollisionsWithActors {}", pActor->name);

	clusterCallbackParams.pTable = &actorIntersectTable;
	result = 0;
	actorIntersectTable.entryCount = 0;
	clusterCallbackParams.pActor = pActor;

	edF32Matrix4ScaleHard(&worldBoundingSphereMatrix, m0, &pActor->scale);
	worldBoundingSphereMatrix.rowT = m0->rowT;
	pActor->ComputeWorldBoundingSphere(&worldBoundingSphere, &worldBoundingSphereMatrix);

	pActorHero = CActorHero::_gThis;

	if ((this->flags_0x0 & 0x20000) == 0) {
		(CScene::ptable.g_ActorManager_004516a4)->cluster.ApplyCallbackToActorsIntersectingSphere(&worldBoundingSphere, gClusterCallback_Collision, &clusterCallbackParams);
	}
	else {
		if ((CActorHero::_gThis != (CActorHero*)0x0) &&
			(lastResult = CActorHero::_gThis->SV_IsWorldBoundingSphereIntersectingSphere(&worldBoundingSphere), lastResult != 0)) {
			gClusterCallback_Collision(pActorHero, &clusterCallbackParams);
		}
	}

	COLLISION_LOG(LogLevel::VeryVerbose, "CCollision::CheckCollisionsWithActors {} entry count: {}", pActor->name, actorIntersectTable.entryCount);

	entryCount = actorIntersectTable.entryCount;
	if (actorIntersectTable.entryCount != 0) {
		colInfoObbTreeObbTree.pColObj = this->pColObj;
		currentEntryIndex = 0;
		colInfoObbTreeObbTree.field_0xc = (char*)0x0;

		if (0 < actorIntersectTable.entryCount) {
			do {
				COLLISION_LOG(LogLevel::VeryVerbose, "CCollision::CheckCollisionsWithActors {} with: {}", pActor->name, actorIntersectTable.aEntries[currentEntryIndex]->name);

				CCollision* pCollision = actorIntersectTable.aEntries[currentEntryIndex]->pCollisionData;

				colInfoObbTreeObbTree.pOtherColObj = pCollision->pColObj;
				lastResult = edObbTreeIntersectObbTree(&colInfoObbTreeObbTree, this->pObbTree, pCollision->pObbTree);

				COLLISION_LOG(LogLevel::VeryVerbose, "CCollision::CheckCollisionsWithActors {} with: {} result: {}", pActor->name, actorIntersectTable.aEntries[currentEntryIndex]->name, lastResult);

				result = result | lastResult;
				currentEntryIndex = currentEntryIndex + 1;
			} while (currentEntryIndex < entryCount);
		}
	}

	return result;
}

uint CCollision::CheckCollisionsWithScenery(int param_2)
{
	edObbTREE_DYN* pObbTreeA;
	edObbTREE_DYN* pObbTreeB;
	uint uVar1;
	uint uVar2;
	edObbTREE_DYN** pStaticColEnd;
	edObbTREE_DYN** pStaticColStart;
	edColINFO_OBBTREE_OBBTREE local_10;

	COLLISION_LOG(LogLevel::VeryVerbose, "CCollision::CheckCollisionsWithScenery");

	uVar1 = 0;
	pObbTreeA = this->pObbTree;
	local_10.pColObj = this->pColObj;
	local_10.pOtherColObj = (edColOBJECT*)0x0;
	local_10.field_0xc = (char*)0x0;
	if (param_2 == 0) {
		pStaticColStart = (CScene::ptable.g_CollisionManager_00451690)->aStaticCollisionRefs;
		pStaticColEnd = pStaticColStart + (CScene::ptable.g_CollisionManager_00451690)->staticCollisionCount;
		for (; pStaticColStart < pStaticColEnd; pStaticColStart = pStaticColStart + 1) {
			COLLISION_LOG(LogLevel::VeryVerbose, "CCollision::CheckCollisionsWithScenery Begin");
			uVar2 = edObbTreeIntersectObbTree(&local_10, pObbTreeA, *pStaticColStart);
			COLLISION_LOG(LogLevel::VeryVerbose, "CCollision::CheckCollisionsWithScenery result: {}", uVar2);
			uVar1 = uVar1 | uVar2;
		}
	}
	else {
		pObbTreeB = ((CScene::ptable.g_SectorManager_00451670)->baseSector).pObbTree;
		if (pObbTreeB != (edObbTREE_DYN*)0x0) {
			uVar1 = edObbTreeIntersectObbTree(&local_10, pObbTreeA, pObbTreeB);
		}
	}
	return uVar1;
}

struct ColHeapEntry {
	int field_0x0;
	undefined field_0x4;
	undefined field_0x5;
	undefined field_0x6;
	undefined field_0x7;
	undefined field_0x8;
	undefined field_0x9;
	undefined field_0xa;
	undefined field_0xb;
	undefined field_0xc;
	undefined field_0xd;
	undefined field_0xe;
	undefined field_0xf;
	undefined4 field_0x10;
	undefined4 field_0x14;
	undefined field_0x18;
	undefined field_0x19;
	undefined field_0x1a;
	undefined field_0x1b;
	undefined4 field_0x1c;
};

struct ColHeapEntryB {
	edF32VECTOR4 field_0x0;
	int field_0x10;
	undefined4 field_0x14;
	undefined field_0x18;
	undefined field_0x19;
	undefined field_0x1a;
	undefined field_0x1b;
	undefined4 field_0x1c;
};

uint CCollision::ResolveContacts(CActor* pActor, edF32VECTOR4* pTranslation, int param_4)
{
	short sVar1;
	CCollision* pCVar2;
	edF32QUAD4* peVar3;
	bool bVar4;
	undefined* puVar5;
	uint* puVar6;
	edColDbObj_80* pColDbObjEnd;
	CActor* pReceiver;
	uint materialIndex;
	uint uVar10;
	uint primFlags;
	int iVar12;
	edColDbObj_80* pColDbObj;
	int iVar14;
	int iVar15;
	int iVar16;
	uint uVar17;
	float fVar18;
	float fVar19;
	float fVar20;
	float fVar21;
	float fVar22;
	float fVar23;
	uint local_1a0;
	edF32VECTOR4 local_180;
	edF32VECTOR4 local_170;
	edF32VECTOR4 local_150;
	//float local_140;
	//float fStack316;
	//float fStack312;
	//float fStack308;
	edF32VECTOR4 local_120[2];
	//float local_110;
	//float fStack268;
	//float fStack264;
	//float fStack260;
	//float local_100;
	//float fStack252;
	//float fStack248;
	//float fStack244;
	edF32VECTOR4 local_f0;
	edF32VECTOR4 local_e0;
	//float local_d0[4];
	//int local_c0[48];

	COLLISION_LOG(LogLevel::VeryVerbose, "CCollision::ResolveContacts {} pTranslation: {}", pActor->name, pTranslation->ToString());

	ColHeapEntryB stack0xfffffdb0[2][3];

	local_1a0 = 0;
	for (iVar14 = 0; iVar14 < 2; iVar14 = iVar14 + 1) {
		for (iVar16 = 0; iVar16 < 3; iVar16 = iVar16 + 1) {
			ColHeapEntryB* pEntry = &stack0xfffffdb0[iVar14][iVar16];

			pEntry->field_0x0.x = 0.0f;
			pEntry->field_0x0.y = 0.0f;
			pEntry->field_0x0.z = 0.0f;
			pEntry->field_0x0.w = 0.0f;
			pEntry->field_0x10 = 0;
		}
	}

	iVar14 = 0;
	if (param_4 != 0) {
		for (; iVar14 < 3; iVar14 = iVar14 + 1) {
			s_collision_contact* pCollisionContact = this->aCollisionContact + iVar14;
			pCollisionContact->location.x = 0.0f;
			pCollisionContact->location.y = 0.0f;
			pCollisionContact->location.z = 0.0f;
			pCollisionContact->location.w = 0.0f;
			pCollisionContact->field_0x10.x = 0.0f;
			pCollisionContact->field_0x10.y = 0.0f;
			pCollisionContact->field_0x10.z = 0.0f;
			pCollisionContact->field_0x10.w = 0.0f;
			pCollisionContact->nbCollisionsA = 0;
			pCollisionContact->nbCollisionsB = 0;
			pCollisionContact->materialFlags = 0;
		}
	}

	local_e0.x = 0.0f;
	local_e0.y = 0.0f;
	local_e0.z = 0.0f;
	local_e0.w = 0.0f;

	bVar4 = (this->flags_0x4 & 2) == 0;
	if (bVar4) {
		bVar4 = 0.001f <= fabs(pTranslation->y);
	}
	if (!bVar4) {
		bVar4 = 0.001f <= fabs(pTranslation->x);
	}
	if (!bVar4) {
		bVar4 = 0.001f <= fabs(pTranslation->z);
	}

	pColDbObj = (gColData.pActiveDatabase)->aDbEntries;
	pColDbObjEnd = pColDbObj + (gColData.pActiveDatabase)->curDbEntryCount;

	COLLISION_LOG(LogLevel::VeryVerbose, "CCollision::ResolveContacts processing {} entries", (gColData.pActiveDatabase)->curDbEntryCount);

	for (; pColDbObj < pColDbObjEnd; pColDbObj = pColDbObj + 1) {

		if (pColDbObj->pColObj && pColDbObj->pColObj->pActor) {
			COLLISION_LOG(LogLevel::VeryVerbose, "CCollision::ResolveContacts pColDbObj {}", LOAD_SECTION_CAST(CActor*, pColDbObj->pColObj->pActor)->name);
		}

		if (pColDbObj->pOtherColObj && pColDbObj->pOtherColObj->pActor) {
			COLLISION_LOG(LogLevel::VeryVerbose, "CCollision::ResolveContacts pOtherColObj {}", LOAD_SECTION_CAST(CActor*, pColDbObj->pOtherColObj->pActor)->name);
		}

		primFlags = pColDbObj->flags;

		if (pColDbObj->depth < 0.0f) {
			pReceiver = (CActor*)0x0;

			if (pColDbObj->pOtherColObj == (edColOBJECT*)0x0) {
				iVar14 = 0;
			}
			else {
				pReceiver = LOAD_SECTION_CAST(CActor*, pColDbObj->pOtherColObj->pActor);

				if (pReceiver != (CActor*)0x0) {
					pCVar2 = pReceiver->pCollisionData;
					materialIndex = pCVar2->flags_0x0;

					if ((materialIndex & 0x2000) != 0) {
						pColDbObj->depth = 0.0;
					}

					if ((materialIndex & 0x8000) != 0) {
						// This could be incorrect?
						stack0xfffffdb0[1][2].field_0x1c = 0;
						pActor->DoMessage(pReceiver, (ACTOR_MESSAGE)0x1c, 0);
					}

					pCVar2->field_0x90 = pColDbObj->location;
				}

				iVar14 = 1;
			}

			COLLISION_LOG(LogLevel::Verbose, "CCollision::ResolveContacts flags: 0x{:x} - {} Obb prim flags: 0x{:x} - {}", primFlags, (primFlags & 0x200) != 0, this->pObbPrim->flags_0x80, (this->pObbPrim->flags_0x80 & 0x200) != 0);

			if ((((primFlags & 0x200) != 0) && (this->pObbPrim != (edColPRIM_OBJECT*)0x0)) && ((this->pObbPrim->flags_0x80 & 0x200) != 0)) {
				if (bVar4) {
					if ((((this->flags_0x0 & 0x400000) == 0) || (pReceiver == (CActor*)0x0)) || (pActor->pTiedActor != pReceiver->pTiedActor)) {
						sVar1 = pColDbObj->bType;

						if (((sVar1 == COL_TYPE_PRIM_OBJ) || (sVar1 == 0xb)) || ((sVar1 == 0xd || (sVar1 == 10)))) {
							edColPRIM_OBJECT* pPrim = reinterpret_cast<edColPRIM_OBJECT*>(pColDbObj->pPrimitiveB);

							edColPRIM_OBJECT* pOtherPrim = reinterpret_cast<edColPRIM_OBJECT*>(pColDbObj->pPrimitiveA);

							edF32VECTOR4* pQuad = &pOtherPrim->vertices.rowT;

							edF32Matrix4MulF32Vector4Hard(&local_170, &pPrim->worldTransform, pQuad);
							edColGetNormalInWorldFromLocal(&local_170, &pPrim->worldTransform, &local_170);
							local_180.xyz = local_170.xyz;
							local_180.y = 0.0f;
							edF32Vector4SafeNormalize1Hard(&local_180, &local_180);
							fVar18 = -pColDbObj->depth * 0.9f;
							local_180 = local_180 * fVar18;

							local_e0 = local_e0 + local_180;

							COLLISION_LOG(LogLevel::Verbose, "CCollision::ResolveContacts A local_e0: {} local_180: {}", local_e0.ToString(), local_180.ToString());

							pColDbObj->depth = 0.0f;
						}
					}
					else {
						pColDbObj->depth = 0.0f;
					}
				}
				else {
					pColDbObj->depth = 0.0f;
				}
			}

			materialIndex = primFlags & 0xf;

			if (pColDbObj->depth < 0.0f) {
				if (materialIndex == 0) {
					materialIndex = CScene::_pinstance->defaultMaterialIndex;
				}

				if ((pColDbObj->field_0x40).y < CCollisionManager::_material_table[materialIndex].field_0x8) {
					uVar17 = 2;
					if (-CCollisionManager::_material_table[materialIndex].field_0x8 < (pColDbObj->field_0x40).y) {
						uVar17 = 0;
					}
				}
				else {
					uVar17 = 1;
					if (((this->flags_0x0 & 0x10) != 0) && (fabs((pColDbObj->field_0x50).y) < CCollisionManager::_material_table[materialIndex].field_0x8)) {
						fVar18 = (pColDbObj->field_0x50).x * (pColDbObj->field_0x40).x + (pColDbObj->field_0x50).y * (pColDbObj->field_0x40).y + (pColDbObj->field_0x50).z * (pColDbObj->field_0x40).z;
						local_f0 = pColDbObj->field_0x40 - (pColDbObj->field_0x50 * fVar18);

						edF32Vector4NormalizeHard(&local_f0, &local_f0);

						if (CCollisionManager::_material_table[materialIndex].field_0x8 <= local_f0.y) {
							pColDbObj->field_0x50 = local_f0;
						}
					}
				}

				if ((pColDbObj->field_0x50).y < CCollisionManager::_material_table[materialIndex].field_0x8) {
					if (0.0f <= (pColDbObj->field_0x50).x * (pColDbObj->field_0x40).x + (pColDbObj->field_0x50).y * (pColDbObj->field_0x40).y + (pColDbObj->field_0x50).z * (pColDbObj->field_0x40).z) {
						uVar10 = 2;
						if (-CCollisionManager::_material_table[materialIndex].field_0x8 < (pColDbObj->field_0x40).y) {
							uVar10 = 0;
						}
					}
					else {
						uVar10 = 3;
						uVar17 = 3;
					}
				}
				else {
					uVar10 = 1;
					if ((pColDbObj->field_0x40).x * (pColDbObj->field_0x50).x + (pColDbObj->field_0x40).y * (pColDbObj->field_0x50).y + (pColDbObj->field_0x40).z * (pColDbObj->field_0x50).z < 0.99f) {
						fVar18 = (pColDbObj->location).y - (this->highestVertex).y;
						if ((((this->flags_0x0 & 0x10) != 0) && (fVar18 <= 0.205f)) || (fVar18 <= 0.05f)) {
							uVar10 = 1;
						}
						else {
							uVar10 = 3;
							uVar17 = 3;
						}
					}
				}

				if ((uVar10 < 3) && (param_4 != 0)) {
					s_collision_contact* pCollisionContact = this->aCollisionContact + uVar10;

					if (pColDbObj->pOtherColObj == (edColOBJECT*)0x0) {
						pCollisionContact->nbCollisionsB = pCollisionContact->nbCollisionsB + 1;
					}
					else {
						pCollisionContact->nbCollisionsA = pCollisionContact->nbCollisionsA + 1;
					}

					pCollisionContact->field_0x10 = pCollisionContact->field_0x10 + pColDbObj->location;
					pCollisionContact->location = pCollisionContact->location + pColDbObj->field_0x50;
					pCollisionContact->materialFlags = primFlags;
				}

				if (uVar17 < 3) {
					ColHeapEntryB* pEntry = &stack0xfffffdb0[iVar14][uVar17];

					pEntry->field_0x10 = pEntry->field_0x10 + 1;

					const float multiplier = -pColDbObj->depth;

					pEntry->field_0x0 = pEntry->field_0x0 + (pColDbObj->field_0x40 * multiplier);
				}
			}
		}
	}

	uint colIndex = 0;
	if (param_4 != 0) {
		for (; (int)colIndex < 3; colIndex = colIndex + 1) {
			s_collision_contact* pCollisionContact = this->aCollisionContact + colIndex;

			iVar14 = pCollisionContact->nbCollisionsA + pCollisionContact->nbCollisionsB;

			if (iVar14 == 0) {
				this->flags_0x4 = this->flags_0x4 & ~(byte)(1 << (colIndex & 0x1f));
			}
			else {
				this->flags_0x4 = this->flags_0x4 | (byte)(1 << (colIndex & 0x1f));

				fVar18 = 1.0f / (float)iVar14;
				
				pCollisionContact->field_0x10 = pCollisionContact->field_0x10 * fVar18;
				pCollisionContact->field_0x10.w = 1.0f;

				edF32Vector4SafeNormalize1Hard(&pCollisionContact->location, &pCollisionContact->location);
			}
		}
	}

	iVar14 = 0;
	if ((this->flags_0x0 & 8) != 0) {
		iVar16 = 0;
		for (iVar15 = 0; iVar15 < 2; iVar15 = iVar15 + 1) {
		
			for (iVar12 = 0; iVar12 < 3; iVar12 = iVar12 + 1) {
				int newValue = stack0xfffffdb0[iVar15][iVar12].field_0x10;
				iVar14 = iVar14 + newValue;
				if (iVar15 == 0) {
					iVar16 = iVar16 + newValue;
				}
			}
		}

		if (0 < iVar14) {
			if (((this->flags_0x0 & 0x20) != 0) && (iVar15 = 0, (this->flags_0x4 & 2) != 0)) {
				for (; iVar15 < 2; iVar15 = iVar15 + 1) {
					//COLLISION_LOG(LogLevel::Verbose, "0: {}", stack0xfffffdb0[iVar15][0].field_0x0.ToString());
					//COLLISION_LOG(LogLevel::Verbose, "1: {}", stack0xfffffdb0[iVar15][1].field_0x0.ToString());
					//COLLISION_LOG(LogLevel::Verbose, "2: {}", stack0xfffffdb0[iVar15][2].field_0x0.ToString());
					stack0xfffffdb0[iVar15][0].field_0x0.y = 0.0f;
					stack0xfffffdb0[iVar15][2].field_0x0.y = 0.0f;
				}
			}

			for (iVar15 = 0; iVar15 < 2; iVar15 = iVar15 + 1) {
				local_120[iVar15] = gF32Vector4Zero;

				for (iVar12 = 0; iVar12 < 3; iVar12 = iVar12 + 1) {

					ColHeapEntryB* pEntry = &stack0xfffffdb0[iVar15][iVar12];
					COLLISION_LOG(LogLevel::Verbose, "CCollision::ResolveContacts A pEntry->field_0x0: {} local_120[iVar15]: {}", pEntry->field_0x0.ToString(), local_120[iVar15].ToString());
					local_120[iVar15] = pEntry->field_0x0 + local_120[iVar15];
				}
			}

			if (iVar16 != 0) {
				COLLISION_LOG(LogLevel::Verbose, "CCollision::ResolveContacts A local_120[iVar15]: {}", local_120[0].ToString());

				edF32VECTOR4 local_130;
				edF32Vector4SafeNormalize0Hard(&local_130, &local_120[0]);

				COLLISION_LOG(LogLevel::Verbose, "CCollision::ResolveContacts A local_130: {}", local_130.ToString());
				COLLISION_LOG(LogLevel::Verbose, "CCollision::ResolveContacts A local_120[1]: {}", local_120[1].ToString());

				fVar18 = local_130.x * local_120[1].x + local_130.y * local_120[1].y + local_130.z * local_120[1].z;

				if (fVar18 < 0.0f) {
					local_120[1] = local_120[1] - (local_130 * fVar18);
				}

				fVar18 = local_130.x * local_e0.x + local_130.y * local_e0.y + local_130.z * local_e0.z;
				if (fVar18 < 0.0f) {
					local_e0 = local_e0 - (local_130 * fVar18);
					COLLISION_LOG(LogLevel::Verbose, "CCollision::ResolveContacts B local_e0: {}", local_e0.ToString());

				}
			}

			for (iVar16 = 0; iVar16 < 2; iVar16 = iVar16 + 1) {
				local_e0 = local_e0 + local_120[iVar16];
				COLLISION_LOG(LogLevel::Verbose, "CCollision::ResolveContacts C local_e0: {}", local_e0.ToString());

			}

			fVar18 = edF32Vector4GetDistHard(&local_e0);
			if (1e-06f <= fVar18) {
				fVar18 = 1.0f / (float)iVar14 + -0.002f / fVar18;
				local_e0 = local_e0 * fVar18;
				COLLISION_LOG(LogLevel::Verbose, "CCollision::ResolveContacts D local_e0: {}", local_e0.ToString());

			}
		}

		COLLISION_LOG(LogLevel::Verbose, "CCollision::ResolveContacts local_e0: {}", local_e0.ToString());

		if (1e-12f <= local_e0.x * local_e0.x + local_e0.y * local_e0.y + local_e0.z * local_e0.z) {
			if ((this->flags_0x0 & 0x40) == 0) {
				*pTranslation = *pTranslation + local_e0;
			}
			else {
				edF32Vector4SafeNormalize0Hard(&local_150, pTranslation);
				fVar18 = (pTranslation->x + local_e0.x) * local_150.x + (pTranslation->y + local_e0.y) * local_150.y + (pTranslation->z + local_e0.z) * local_150.z;
				*pTranslation = local_150 * fVar18;
			}

			local_1a0 = 1;
		}
	}

	return local_1a0;
}

void CCollision::PreprocessActorContacts(float param_1, CActor* pActor, CActorsTable* pTable, CActor** pOutActor)
{
	uint colFlags;
	CActor* pNewEntry;
	CCollision* pCollidingWithData;
	bool bVar3;
	bool bVar4;
	edColDbObj_80* pColDbEnd;
	edColDbObj_80* pColDb;
	edF32VECTOR4 eStack48;
	edF32VECTOR4 eStack32;
	undefined4 local_4;

	bVar3 = false;
	colFlags = this->flags_0x0;
	pColDb = (gColData.pActiveDatabase)->aDbEntries;
	pColDbEnd = pColDb + (gColData.pActiveDatabase)->curDbEntryCount;

	for (; pColDb < pColDbEnd; pColDb = pColDb + 1) {
		pNewEntry = LOAD_SECTION_CAST(CActor*, pColDb->pOtherColObj->pActor);

		COLLISION_LOG(LogLevel::Verbose, "CCollision::PreprocessActorContacts {} colliding with {}", pActor->name, pNewEntry->name);

		pCollidingWithData = pNewEntry->pCollisionData;

		COLLISION_LOG(LogLevel::Verbose, "CCollision::PreprocessActorContacts flags: 0x{:x} - 0x{:x}", colFlags, pCollidingWithData->flags_0x0);
		COLLISION_LOG(LogLevel::Verbose, "CCollision::PreprocessActorContacts field_0x30: {} field_0x40: {}", pColDb->location.ToString(), pColDb->field_0x40.ToString());


		if (((pTable != (CActorsTable*)0x0) && (pColDb->depth <= -param_1)) &&
			(bVar4 = pTable->IsInList(pNewEntry), bVar4 == false)) {
			pTable->Add(pNewEntry);
		}

		if ((pCollidingWithData->pObbPrim != (edColPRIM_OBJECT*)0x0) && ((pCollidingWithData->pObbPrim->flags_0x80 & 0x200) != 0)) {
			if (((colFlags & 0x200) != 0) && ((pColDb->depth < 0.0f && (0.5f < (pColDb->field_0x40).y)))) {
				local_4 = 0;
				pActor->DoMessage(pNewEntry, (ACTOR_MESSAGE)0x6a, 0);
			}

			if ((colFlags & 0x100) != 0) {
				if (pColDb->depth <= -0.3f) {
					pCollidingWithData->field_0x18 = pCollidingWithData->field_0x18 + 2;
					pCollidingWithData->field_0x1c = pActor;
				}

				pColDb->depth = 0.0f;
			}
		}

		if ((((((colFlags & 0x400) == 0) && ((pCollidingWithData->flags_0x0 & 0x80) != 0)) && ((pCollidingWithData->flags_0x0 & 0x200000) == 0)) &&
			(pNewEntry->pTiedActor != pActor)) && ((*pOutActor == (CActor*)0x0 || (pNewEntry == pActor->pTiedActor)))) {
			if (!bVar3) {
				ComputeG3DObbTreeLowestAndHighestVertices(&eStack32, &eStack48, 1, this->pDynCol);
				bVar3 = true;
			}

			if (fabs(eStack32.y - (pColDb->location).y) <= 0.2f) {
				*pOutActor = pNewEntry;
			}
		}
	}
	return;
}

uint CCollision::CheckCollisions_OBBTree(CActor* pActor, edF32MATRIX4* m0, edF32MATRIX4* param_4, edF32VECTOR4* pTranslation, CActorsTable* param_6, CActor** param_7, int param_8)
{
	int iVar1;
	uint bCollisionDetected;
	uint uVar3;
	ulong uVar4;

	gColData.field_0x14 = gColData.field_0x10;
	(gColData.pActiveDatabase)->curDbEntryCount = 0;

	bCollisionDetected = 0;
	if (((this->flags_0x0 & 0x502) != 0) && (bCollisionDetected = CheckCollisionsWithActors(pActor, m0), bCollisionDetected != 0)) {
		if ((param_6 != (CActorsTable*)0x0) || (((this->flags_0x0 & 0x100) != 0 || ((this->flags_0x0 & 0x400) == 0)))) {
			if ((this->flags_0x0 & 0x400) == 0) {
				PreprocessActorContacts(0.0f, pActor, param_6, param_7);
			}
			else {
				PreprocessActorContacts(0.005f, pActor, param_6, param_7);
			}
		}

		if (((this->flags_0x0 & 0x400) != 0) || ((this->flags_0x0 & 2) == 0)) {
			// Reset actor collision data
			bCollisionDetected = 0;
			gColData.field_0x14 = gColData.field_0x10;
			(gColData.pActiveDatabase)->curDbEntryCount = 0;
		}
	}

	if ((this->flags_0x0 & 1) != 0) {
		iVar1 = ((CScene::ptable.g_SectorManager_00451670)->baseSector).desiredSectorID;

		uVar4 = (ulong)(iVar1 == pActor->objectId);
		if (uVar4 != 0) {
			uVar4 = (ulong)(iVar1 != -1);
		}
	
		uVar3 = CheckCollisionsWithScenery(uVar4);
		bCollisionDetected = bCollisionDetected | uVar3;
	}

	if (bCollisionDetected == 0) {
		bCollisionDetected = 0;
		this->flags_0x4 = this->flags_0x4 & 0xf8;
	}
	else {
		bCollisionDetected = ResolveContacts(pActor, pTranslation, param_8);
	}

	return bCollisionDetected;
}

void CCollision::CheckCollisions_UpdateCollisionMatrix(CActor* pActor, edF32MATRIX4* pMatrix, CActorsTable* pActorsTable, CActor* param_5,	int param_6)
{
	int* piVar1;
	bool bVar2;
	uint uVar3;
	int stepCount;
	uint uVar5;
	long lVar6;
	uint matrixType;
	S_CARRY_ACTOR_ENTRY* peVar7;
	int** ppiVar8;
	uint uVar9;
	float fVar10;
	float fVar11;
	float fVar12;
	edF32VECTOR4 currentTranslation;
	edF32VECTOR4 resultPosition;
	edF32VECTOR4 subdividedTranslation;
	edF32VECTOR4 baseTranslation;
	edF32MATRIX4 local_150;
	CActorsTable local_110;
	undefined4 local_8;
	CActor* pCollideWithActor;

	COLLISION_LOG(LogLevel::VeryVerbose, "CCollision::CheckCollisions_UpdateCollisionMatrix {}", pActor->name);

	local_110.entryCount = 0;
	uVar3 = this->flags_0x0 & 0x480;
	bVar2 = pMatrix->cc + pMatrix->aa + pMatrix->bb != 3.0f;
	matrixType = (uint)bVar2;

	if (uVar3 != 0) {
		if (pActorsTable == (CActorsTable*)0x0) {
			pActorsTable = &local_110;
		}

		if ((this->flags_0x0 & 0x80) != 0) {
			S_TIED_ACTOR_ENTRY* pSVar7;
			for (pSVar7 = this->pTiedActorEntry; pSVar7 != (S_TIED_ACTOR_ENTRY*)0x0; pSVar7 = pSVar7->pPrev) {
				pActorsTable->Add(pSVar7->pActor);
				bVar2 = pSVar7->pActor->IsKindOfObject(2);
				if (bVar2 != false) {
					CActorMovable* pCVar1 = (CActorMovable*)pSVar7->pActor;
					(pCVar1->dynamic).field_0x10.x = 0.0f;
					(pCVar1->dynamic).field_0x10.y = 0.0f;
					(pCVar1->dynamic).field_0x10.z = 0.0f;
					(pCVar1->dynamic).field_0x10.w = 0.0f;
				}
			}
		}
	}

	edF32Vector4SubHard(&baseTranslation, &pMatrix->rowT, &(this->transformMatrix).rowT);

	COLLISION_LOG(LogLevel::VeryVerbose, "CCollision::CheckCollisions_UpdateCollisionMatrix Base translation: {}", baseTranslation.ToString());

	local_150 = gF32Matrix4Unit;

	pCollideWithActor = (CActor*)0x0;

	if ((this->flags_0x0 & 4) == 0) {
		uVar9 = 0;
		if ((this->pObbPrim == (edColPRIM_OBJECT*)0x0) || ((this->flags_0x0 & 0x10000) == 0)) {
			stepCount = 1;
		}
		else {
			stepCount = GetPrimNumDeltaSubdivisions(this->pObbPrim, this->primType, &baseTranslation);
		}
		if (0x14 < stepCount) {
			stepCount = 0x14;
		}
		float stepMultiplier = 1.0f / (float)stepCount;
		subdividedTranslation = baseTranslation * stepMultiplier;

		resultPosition = (this->transformMatrix).rowT;

		COLLISION_LOG(LogLevel::VeryVerbose, "CCollision::CheckCollisions_UpdateCollisionMatrix Base translation: {} Original position: {} Step count: {}", 
			baseTranslation.ToString(), resultPosition.ToString(), stepCount);

		for (; stepCount != 0; stepCount = stepCount + -1) {
			currentTranslation = subdividedTranslation;
			pMatrix->rowT = resultPosition + subdividedTranslation;

			COLLISION_LOG(LogLevel::VeryVerbose, "CCollision::CheckCollisions_UpdateCollisionMatrix Current translation: {} New position: {} Step count: {}",
				currentTranslation.ToString(), pMatrix->rowT.ToString(), stepCount);

			if (bVar2) {
				edF32Matrix4GetInverseOrthoHard(&local_150, pMatrix);
				SetObbTreeMatrixRecurse(this->pObbTree, pMatrix, &local_150);
				this->flags_0x4 = this->flags_0x4 & 0xef;
			}
			else {
				local_150.dd = pMatrix->dd;
				local_150.da = 0.0f - pMatrix->da;
				local_150.db = 0.0f - pMatrix->db;
				local_150.dc = 0.0f - pMatrix->dc;

				if ((this->flags_0x4 & 0x10) == 0) {
					SetObbTreeMatrixNoRotationRecurse(this->pObbTree, pMatrix, &local_150);
					this->flags_0x4 = this->flags_0x4 | 0x10;
				}
				else {
					SetObbTreePositionNoRotationRecurse(this->pObbTree, pMatrix);
				}
			}

			if (0 < this->pDynCol->vertexCount) {
				TransformG3DObbTreeVertices(this->pDynCol, matrixType, pMatrix);
			}

			COLLISION_LOG(LogLevel::VeryVerbose, "CCollision::CheckCollisions_UpdateCollisionMatrix pMatrix: {}", pMatrix->ToString());
			COLLISION_LOG(LogLevel::VeryVerbose, "CCollision::CheckCollisions_UpdateCollisionMatrix local_150: {}", local_150.ToString());

			uVar5 = CheckCollisions_OBBTree(pActor, pMatrix, &local_150, &currentTranslation, pActorsTable, &pCollideWithActor, param_6);
			uVar9 = uVar9 | uVar5;

			COLLISION_LOG(LogLevel::VeryVerbose, "CCollision::CheckCollisions_UpdateCollisionMatrix Out translation: {} - flag: {}", currentTranslation.ToString(), uVar9);

			resultPosition = resultPosition + currentTranslation;
		}

		pMatrix->rowT = resultPosition;

		COLLISION_LOG(LogLevel::VeryVerbose, "CCollision::CheckCollisions_UpdateCollisionMatrix Final position: {} result: {} name: {}", pMatrix->rowT.ToString(), uVar9, 
			pCollideWithActor ? pCollideWithActor->name : "None");

		if (uVar9 != 0) {
			if (bVar2) {
				edF32Matrix4GetInverseOrthoHard(&local_150, pMatrix);
				SetObbTreePositionRecurse(this->pObbTree, pMatrix);
				this->flags_0x4 = this->flags_0x4 & 0xef;
			}
			else {
				local_150.rowT.w = pMatrix->rowT.w;
				local_150.rowT.x = 0.0f - pMatrix->rowT.x;
				local_150.rowT.y = 0.0f - pMatrix->rowT.y;
				local_150.rowT.z = 0.0f - pMatrix->rowT.z;

				if ((this->flags_0x4 & 0x10) == 0) {
					SetObbTreeMatrixNoRotationRecurse(this->pObbTree, pMatrix, &local_150);
					this->flags_0x4 = this->flags_0x4 | 0x10;
				}
				else {
					SetObbTreePositionNoRotationRecurse(this->pObbTree, pMatrix);
				}
			}

			if (0 < this->pDynCol->vertexCount) {
				TransformG3DObbTreeVertices(this->pDynCol, matrixType, pMatrix);
			}
		}
	}
	else {
		IMPLEMENTATION_GUARD(
		CheckCollisions_RayIntersect(pActor, &(this->transformMatrix).rowT, &baseTranslation, pActorsTable, param_6);)
		pMatrix->rowT = (this->transformMatrix).rowT + baseTranslation;

		if (bVar2) {
			edF32Matrix4GetInverseOrthoHard(&local_150, pMatrix);
			SetObbTreeMatrixRecurse(this->pObbTree, pMatrix, &local_150);
			this->flags_0x4 = this->flags_0x4 & 0xef;
		}
		else {
			local_150.rowT.w = pMatrix->rowT.w;
			local_150.rowT.x = 0.0f - pMatrix->rowT.x;
			local_150.rowT.y = 0.0f - pMatrix->rowT.y;
			local_150.rowT.z = 0.0f - pMatrix->rowT.z;
			if ((this->flags_0x4 & 0x10) == 0) {
				SetObbTreeMatrixNoRotationRecurse(this->pObbTree, pMatrix, &local_150);
				this->flags_0x4 = this->flags_0x4 | 0x10;
			}
			else {
				SetObbTreePositionNoRotationRecurse(this->pObbTree, pMatrix);
			}
		}
		if (0 < this->pDynCol->vertexCount) {
			TransformG3DObbTreeVertices(this->pDynCol, matrixType, pMatrix);
		}
	}

	if ((uVar3 != 0) && ((pActorsTable->entryCount != 0 || (this->pTiedActorEntry != (S_TIED_ACTOR_ENTRY*)0x0)))) {
		TranslateList(pActor, pActorsTable, pMatrix, param_5);
	}

	this->transformMatrix = *pMatrix;
	
	peVar7 = this->pCarryActorEntry;
	if (peVar7 != (S_CARRY_ACTOR_ENTRY*)0x0) {
		edF32Matrix4MulF32Matrix4Hard(&peVar7->m1, &peVar7->m0, &this->transformMatrix);
	}

	if ((this->flags_0x0 & 0x40000) == 0) {
		this->lowestVertex = this->transformMatrix.rowT;
		this->highestVertex = this->lowestVertex;
	}
	else {
		ComputeG3DObbTreeLowestAndHighestVertices(&this->highestVertex, &this->lowestVertex, matrixType, this->pDynCol);
	}

	if (param_5 == (CActor*)0x0) {
		pActor->TieToActor(pCollideWithActor, 0, 0, (edF32MATRIX4*)0x0);
	}

	if (0 < this->field_0x18) {
		if (3 < this->field_0x18) {
			local_8 = 1;
			this->field_0x1c->DoMessage(pActor, (ACTOR_MESSAGE)3, (MSG_PARAM)1);
		}

		this->field_0x18 = this->field_0x18 + -1;
	}

	return;
}

void CCollision::CheckCollisions_TranslateActor(CActor* pActor, edF32VECTOR4* pTranslation, CActorsTable* pActorsTable, CActor* param_5, int param_6)
{
	float y;
	float z;
	float w;
	edF32MATRIX4 actorTransform;

	if ((this->flags_0x0 & 0x800) == 0) {
		actorTransform.rowX = gF32Matrix4Unit.rowX;
		actorTransform.rowY = gF32Matrix4Unit.rowY;
		actorTransform.rowZ = gF32Matrix4Unit.rowZ;
	}
	else {
		edF32Matrix4FromEulerSoft(&actorTransform, &pActor->rotationEuler.xyz, "XYZ");
	}

	actorTransform.rowT = pActor->currentLocation + *pTranslation;

	CheckCollisions_UpdateCollisionMatrix(pActor, &actorTransform, pActorsTable, param_5, param_6);

	*pTranslation = actorTransform.rowT - pActor->currentLocation;

	pActor->UpdatePosition(&actorTransform.rowT, false);
	return;
}

CCollision::CCollision()
{
	this->flags_0x4 = 0;
	this->flags_0x0 = 0;
	this->pColObj = (edColOBJECT*)0x0;
	this->pDynCol = (edColG3D_OBB_TREE_DYN*)0x0;
	this->pObbTree = (edObbTREE_DYN*)0x0;
	this->pObbPrim = 0;
	this->field_0x1c = 0;
	this->pTiedActorEntry = (S_TIED_ACTOR_ENTRY*)0x0;
	this->pCarryActorEntry = (S_CARRY_ACTOR_ENTRY*)0x0;
	this->actorFieldA = (CActor*)0x0;
	this->actorField = (CActor*)0x0;
}

S_TIED_ACTOR_ENTRY CCollisionManager::_tied_actors_table[0x100] = {};
S_CARRY_ACTOR_ENTRY CCollisionManager::_carry_info_table[0x80] = {};

void CCollision::RegisterTiedActor(CActor* pActorTo, CActor* pActor, int carryMethod)
{
	int iVar1;
	S_TIED_ACTOR_ENTRY* ppuVar2;
	S_CARRY_ACTOR_ENTRY* peVar2;
	float fVar3;
	float fVar4;
	float fVar5;

	iVar1 = 0x100;
	ppuVar2 = CCollisionManager::_tied_actors_table;
	do {
		if (ppuVar2->pActor == (CActor*)0x0) goto LAB_00212060;
		iVar1 = iVar1 + -1;
		ppuVar2 = ppuVar2 + 1;
	} while (iVar1 != 0);
	ppuVar2 = (S_TIED_ACTOR_ENTRY*)0x0;

LAB_00212060:
	if (ppuVar2 != (S_TIED_ACTOR_ENTRY*)0x0) {
		ppuVar2->pActor = pActor;
		ppuVar2->carryMethod = carryMethod;
		ppuVar2->pNext = (S_TIED_ACTOR_ENTRY*)0x0;
		ppuVar2->pPrev = this->pTiedActorEntry;
		this->pTiedActorEntry = ppuVar2;
		if (ppuVar2->pPrev != (S_TIED_ACTOR_ENTRY*)0x0) {
			ppuVar2->pPrev->pNext = ppuVar2;
		}

		if (pCarryActorEntry == (S_CARRY_ACTOR_ENTRY*)0x0) {
			iVar1 = 0x80;
			peVar2 = CCollisionManager::_carry_info_table;
			do {
				if (peVar2->field_0x80 == 0) {
					peVar2->field_0x80 = 1;
					goto LAB_002120d0;
				}
				iVar1 = iVar1 + -1;
				peVar2 = peVar2 + 1;
			} while (iVar1 != 0);
			peVar2 = (S_CARRY_ACTOR_ENTRY*)0x0;
		LAB_002120d0:
			pCarryActorEntry = peVar2;
		}

		if (pCarryActorEntry != (S_CARRY_ACTOR_ENTRY*)0x0) {
			edF32Matrix4FromEulerSoft(&pCarryActorEntry->m0, &pActorTo->pCinData->rotationEuler, "XYZ");
			pCarryActorEntry->m0.rowT = pActorTo->baseLocation;

			edF32Matrix4GetInverseOrthoHard(&pCarryActorEntry->m0, &pCarryActorEntry->m0);
			edF32Matrix4MulF32Matrix4Hard(&pCarryActorEntry->m1, &pCarryActorEntry->m0, &this->transformMatrix);
		}
	}
	return;
}

void CCollision::UnregisterTiedActor(CActor* pActor)
{
	S_TIED_ACTOR_ENTRY* pSVar1;

	for (pSVar1 = this->pTiedActorEntry; (pSVar1 != (S_TIED_ACTOR_ENTRY*)0x0 && (pActor != pSVar1->pActor));
		pSVar1 = pSVar1->pPrev) {
	}

	if (pSVar1 != (S_TIED_ACTOR_ENTRY*)0x0) {
		if (pSVar1->pNext == (S_TIED_ACTOR_ENTRY*)0x0) {
			this->pTiedActorEntry = pSVar1->pPrev;
		}
		else {
			pSVar1->pNext->pPrev = pSVar1->pPrev;
		}

		if (pSVar1->pPrev != (S_TIED_ACTOR_ENTRY*)0x0) {
			pSVar1->pPrev->pNext = pSVar1->pNext;
		}

		pSVar1->pActor = (CActor*)0x0;
		if ((this->pTiedActorEntry == (S_TIED_ACTOR_ENTRY*)0x0) && (this->pCarryActorEntry != (S_CARRY_ACTOR_ENTRY*)0x0))
		{
			this->pCarryActorEntry->field_0x80 = 0;
			this->pCarryActorEntry = (S_CARRY_ACTOR_ENTRY*)0x0;
		}
	}

	return;
}

CActor* CCollision::FindTiedActor(CActor* pActor)
{
	S_TIED_ACTOR_ENTRY* pEntry;

	for (pEntry = this->pTiedActorEntry;
		(pEntry != (S_TIED_ACTOR_ENTRY*)0x0 && (pActor != pEntry->pActor)); pEntry = pEntry->pPrev)
	{
	}
	return pEntry->pActor;
}

float CCollision::GetSubjectiveCarriedWeight()
{
	S_TIED_ACTOR_ENTRY* pEntry;
	CActor* pCVar1;
	bool bVar2;
	float fVar3;
	float subjectiveCarriedWeight;
	CActorMovable* pMov;

	subjectiveCarriedWeight = 0.0f;

	for (pEntry = this->pTiedActorEntry; pEntry != (S_TIED_ACTOR_ENTRY*)0x0; pEntry = pEntry->pPrev) {
		pMov = reinterpret_cast<CActorMovable*>(pEntry->pActor);
		if (((pMov->flags & 4) != 0) &&
			(bVar2 = pMov->IsKindOfObject(2), bVar2 != false)) {
			fVar3 = pMov->GetSubjectiveCumulatedWeight();
			subjectiveCarriedWeight = subjectiveCarriedWeight + fVar3;
		}
	}

	return subjectiveCarriedWeight;
}

float CCollision::GetCarriedWeight()
{
	S_TIED_ACTOR_ENTRY* pEntry;
	CActor* pCVar1;
	bool bVar2;
	float fVar3;
	float carriedWeight;
	CActorMovable* pMov;

	carriedWeight = 0.0f;

	for (pEntry = this->pTiedActorEntry; pEntry != (S_TIED_ACTOR_ENTRY*)0x0; pEntry = pEntry->pPrev) {
		pMov = reinterpret_cast<CActorMovable*>(pEntry->pActor);
		if (((pMov->flags & 4) != 0) &&
			(bVar2 = pMov->IsKindOfObject(2), bVar2 != false)) {
			fVar3 = pMov->GetCumulatedWeight();
			carriedWeight = carriedWeight + fVar3;
		}
	}

	return carriedWeight;
}

void CCollision::CheckCollisions_MoveActor(CActor* pActor, edF32MATRIX4* m0, CActorsTable* pActorsTable, CActor* pOtherActor, int param_6)
{
	edF32MATRIX4 localMatrix;

	if ((this->flags_0x0 & 0x800) == 0) {
		localMatrix.rowX = gF32Matrix4Unit.rowX;
		localMatrix.rowY = gF32Matrix4Unit.rowY;
		localMatrix.rowZ = gF32Matrix4Unit.rowZ;
		localMatrix.rowT = m0->rowT;

		CheckCollisions_UpdateCollisionMatrix(pActor, &localMatrix, pActorsTable, pOtherActor, param_6);

		m0->rowT = localMatrix.rowT;

		if ((this->flags_0x0 & 0x800) == 0) {
			pActor->UpdatePosition(&localMatrix.rowT, false);
		}
		else {
			pActor->UpdatePosition(&localMatrix, 0);
		}
	}
	else {
		CheckCollisions_UpdateCollisionMatrix(pActor, m0, pActorsTable, pOtherActor, param_6);
		pActor->UpdatePosition(m0, 0);
	}
	return;
}

void CCollision::CheckCollisions_MoveActor(CActor* pActor, edF32VECTOR4* pVector, CActorsTable* pActorsTable, CActor* pOtherActor, int param_6)
{
	edF32MATRIX4 local_40;

	if ((this->flags_0x0 & 0x800) == 0) {
		local_40.rowX = gF32Matrix4Unit.rowX;
		local_40.rowY = gF32Matrix4Unit.rowY;
		local_40.rowZ = gF32Matrix4Unit.rowZ;
	}
	else {
		edF32Matrix4FromEulerSoft(&local_40, (edF32VECTOR3*)&pActor->rotationEuler, "XYZ");
	}

	local_40.rowT = *pVector;

	CheckCollisions_UpdateCollisionMatrix(pActor, &local_40, pActorsTable, pOtherActor, param_6);

	*pVector = local_40.rowT;

	if ((this->flags_0x0 & 0x800) == 0) {
		pActor->UpdatePosition(&local_40.rowT, false);
	}
	else {
		pActor->UpdatePosition(&local_40, 0);
	}
	return;
}
