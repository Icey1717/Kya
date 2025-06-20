#ifndef FX_MANAGER_H
#define FX_MANAGER_H

#include "Types.h"
#include "LargeObject.h"

#define FX_TYPE_COMPOSITE 0
#define FX_TYPE_PATH 1
#define FX_TYPE_GROUP 2
#define FX_TYPE_PARTICLE 3
#define FX_TYPE_SOUND 4
#define FX_TYPE_RANDOM 5
#define FX_TYPE_LOD 6

struct ByteCode;

struct EffectsManagerObj_60;

class CCameraManager;
class CNewFx;

enum FX_MATERIAL_SELECTOR
{
	FX_MATERIAL_SELECTOR_DEFAULT = 0,
	FX_MATERIAL_SELECTOR_WATER = 1,
	FX_MATERIAL_SELECTOR_LAVA = 2,
	FX_MATERIAL_SELECTOR_SAND = 3,
	FX_MATERIAL_SELECTOR_ICE = 4,
	FX_MATERIAL_SELECTOR_GRASS = 5,
	FX_MATERIAL_SELECTOR_MAX = 6
};

class CFxHandle
{
public:
	CFxHandle();

	undefined4 id;
	CNewFx* pFx;
};

template <typename FxType>
class CDoubleLinkedNode
{
public:
	CDoubleLinkedNode()
	{
		this->pNext = (CDoubleLinkedNode<FxType>*)0x0;
		this->pPrev = (CDoubleLinkedNode<FxType>*)0x0;
	}

	FxType* aFx;
	CDoubleLinkedNode<FxType>* pNext;
	CDoubleLinkedNode<FxType>* pPrev;
};

class CFx
{
public:
	CFx();
	virtual void AddAll(ByteCode* pByteCode, CFx* aFx, int count) = 0;
	virtual void InitAll(CFx* aFx, int count) = 0;
	virtual void ManageAll(uint count, CFx* aFx) = 0;
	virtual void Draw() = 0;
	virtual void CheckpointResetAll(uint count, CFx* aFx) = 0;

	uint flags;

	edF32VECTOR4 vector_0x10;
	edF32VECTOR4 vector_0x20;

	undefined4 field_0x30;
	undefined4 field_0x34;
	undefined4 field_0x38;
	undefined4 field_0x3c;

	int field_0x40;

	undefined4 field_0x48;

	int count_0x50;

	EffectsManagerObj_60* field_0x58;
	int field_0x5c;
};

class CNewFx
{
public:
	CNewFx();
	virtual void Func_0x30(float param_1);

	CNewFx* field_0x4;
	int field_0x8;
	undefined4* field_0x1c;

	uint flags;
	int field_0x18;
	uint field_0x20;

	edF32VECTOR4 field_0x30;

	float field_0x40;
	float field_0x44;
	float field_0x48;
	float field_0x4c;

	edF32VECTOR4 field_0x50;
	edF32VECTOR4 field_0x60;

	float field_0x70;
};

struct s_fx_sort_data
{
	void* pFx;
	float field_0x4;
	int field_0x8;
};

// Fx Pool Interface
class CFxPoolManagerFather
{
public:
	CFxPoolManagerFather()
		: pData((char*)0x0)
	{
	}

	virtual void Init() = 0;
	virtual void CheckpointReset() = 0;
	virtual void Manage() = 0;
	virtual void Play(uint* pCount, s_fx_sort_data* pSortData, CCameraManager* pCameraManager) = 0;
	virtual void SetupPool(ByteCode* pByteCode, uint param_3) = 0;
	virtual uint InstallFxScenaricData(ByteCode* pByteCode) = 0;
	virtual void* InstanciateFx(uint param_2, FX_MATERIAL_SELECTOR selector) = 0;

	virtual int GetNbPool() = 0;

	char* pData;
};

template<typename FxType>
class CDoubleLinkedList
{
public:
	CDoubleLinkedList()
		: pHead((CDoubleLinkedNode<FxType>*)0x0)
		, pTail((CDoubleLinkedNode<FxType>*)0x0)
	{

	}

	CDoubleLinkedNode<FxType>* RemoveHead()
	{
		CDoubleLinkedNode<FxType>* pHead;

		pHead = this->pHead;

		if (pHead != (CDoubleLinkedNode<FxType>*)0x0) {
			if (pHead->pPrev == (CDoubleLinkedNode<FxType>*)0x0) {
				this->pTail = (CDoubleLinkedNode<FxType>*)0x0;
			}
			else {
				pHead->pPrev->pNext = (CDoubleLinkedNode<FxType>*)0x0;
			}

			this->pHead = this->pHead->pPrev;
		}

		return pHead;
	}

	CDoubleLinkedNode<FxType>* pHead;
	CDoubleLinkedNode<FxType>* pTail;
};

template<typename FxType, typename ScenaricDataType>
class CFxPoolManager : public CFxPoolManagerFather
{
public:
	CFxPoolManager()
	{
		this->nbPool = 0;
		this->aNodes = (CDoubleLinkedNode<FxType>*)0x0;
		this->aFx = (FxType*)0x0;
		this->nbScenaricData = 0;
		this->aScenaricData = 0;
	}

	virtual void Init()
	{
		for (uint i = 0; i < this->nbScenaricData; i++) {
			this->aScenaricData[i].Init();
		}

		return;
	}

	virtual void CheckpointReset()
	{
		return;
	}

	virtual void Manage()
	{
		uint uVar1;
		bool bVar2;
		FxType** ppCVar3;
		CDoubleLinkedNode<FxType>* pCVar1;

		pCVar1 = (this->listB).pHead;

		while (pCVar1 != (CDoubleLinkedNode<FxType>*)0x0) {
			ppCVar3 = &pCVar1->aFx;
			pCVar1 = pCVar1->pPrev;
			uVar1 = (*ppCVar3)->flags;
			bVar2 = false;

			if (((uVar1 & 1) != 0) && ((uVar1 & 4) == 0)) {
				bVar2 = true;
			}

			if (bVar2) {
				(*ppCVar3)->Manage();
			}
		}

		return;
	}

	virtual void Play(uint* pCount, s_fx_sort_data* pSortData, CCameraManager* pCameraManager)
	{
		FxType* pCVar1;
		bool bVar2;
		CDoubleLinkedNode<FxType>* pCVar3;
		float fVar4;
		edF32VECTOR4 local_60;
		edF32VECTOR4 local_50;
		edF32MATRIX4 eStack64;

		pCVar3 = (this->listB).pHead;
		if (pCVar3 != (CDoubleLinkedNode<FxType> *)0x0) {
			do {
				pCVar1 = pCVar3->aFx;
				bVar2 = false;
				if (((pCVar1->flags & 2) != 0) && ((pCVar1->flags & 8) == 0)) {
					bVar2 = true;
				}

				if (bVar2) {
					pSortData[*pCount].field_0x8 = 1;
					pSortData[*pCount].pFx = pCVar1;
					edF32Matrix4SetIdentityHard(&eStack64);
					edF32Matrix4TranslateHard(&eStack64, &eStack64, &pCVar1->field_0x30);
					edF32Matrix4MulF32Matrix4Hard(&eStack64, &eStack64, &pCameraManager->worldToCamera_0x3d0);
					local_50.x = eStack64.ac;
					local_50.y = eStack64.bc;
					local_50.z = eStack64.cc;
					local_60 = pCVar1->field_0x30;
					edF32Vector4SubHard(&local_60, &local_60, &(pCameraManager->transformationMatrix).rowT);
					fVar4 = edF32Vector4DotProductHard(&local_50, &local_60);
					pSortData[*pCount].field_0x4 = fVar4;
					*pCount = *pCount + 1;
				}

				pCVar3 = pCVar3->pPrev;
			} while (pCVar3 != (CDoubleLinkedNode<FxType>*)0x0);
		}

		return;
	}

	virtual void SetupPool(ByteCode* pByteCode, uint count)
	{
		this->nbPool = count;

		if (this->nbPool != 0) {
			this->aFx = new FxType[this->nbPool];
			this->aNodes = new CDoubleLinkedNode<FxType>[this->nbPool];

			CDoubleLinkedNode<FxType>* pCurNode = this->aNodes;
			FxType* pCurFx = this->aFx;

			uint i = this->nbPool;

			while (i != 0) {
				i = i - 1;
				pCurNode->aFx = pCurFx;
				pCurNode->pNext = this->listA.pTail;
				pCurNode->pPrev = (CDoubleLinkedNode<FxType>*)0x0;

				if (this->listA.pTail == (CDoubleLinkedNode<FxType>*)0x0) {
					this->listA.pHead = pCurNode;
				}
				else {
					this->listA.pTail->pPrev = pCurNode;
				}
				this->listA.pTail = pCurNode;

				pCurNode = pCurNode + 1;
				pCurFx = pCurFx + 1;
			}
		}
	}

	virtual uint InstallFxScenaricData(ByteCode* pByteCode)
	{
		this->nbScenaricData = pByteCode->GetU32();

		if (this->nbScenaricData != 0) {
			this->aScenaricData = new ScenaricDataType[this->nbScenaricData];

			for (uint i = 0; i < this->nbScenaricData; i++) {
				this->aScenaricData[i].Create(pByteCode);
			}
		}

		return this->nbScenaricData;
	}

	virtual int GetNbPool()
	{
		return this->nbPool;
	}

	virtual FxType* _InstanciateFx()
	{
		CDoubleLinkedNode<FxType>* pTail;
		CDoubleLinkedNode<FxType>* pPrevHead;
		FxType* pNewFx;

		pPrevHead = this->listA.RemoveHead();

		if (pPrevHead == (CDoubleLinkedNode<FxType> *)0x0) {
			pNewFx = (FxType*)0x0;
		}
		else {
			pPrevHead->pNext = (this->listB).pTail;
			pPrevHead->pPrev = (CDoubleLinkedNode<FxType> *)0x0;
			pTail = (this->listB).pTail;
			if (pTail == (CDoubleLinkedNode<FxType> *)0x0) {
				(this->listB).pHead = pPrevHead;
			}
			else {
				pTail->pPrev = pPrevHead;
			}

			(this->listB).pTail = pPrevHead;

			pNewFx = pPrevHead->aFx;
			pNewFx->flags = 0;
		}

		return pNewFx;
	}

	CDoubleLinkedList<FxType> listA;
	CDoubleLinkedList<FxType> listB;

	uint nbPool;
	CDoubleLinkedNode<FxType>* aNodes;
	FxType* aFx;

	uint nbScenaricData;
	ScenaricDataType* aScenaricData;

};

struct CFxManager : public CObjectManager {

	CFxManager();

	virtual void Game_Init();
	virtual void Game_Term();

	virtual void Level_Init();
	virtual void Level_Term();
	virtual void Level_AddAll(ByteCode* pByteCode);
	virtual void Level_ClearAll();

	virtual void Level_Manage();
	virtual void Level_ManagePaused();
	virtual void Level_Draw();

	virtual void Level_Reset();

	virtual void Level_CheckpointReset();

	virtual void Level_PauseChange(bool bPaused);

	void Level_PreInit();

	void AddPool(ByteCode* pMemoryStream);

	uint AddFxClass(ByteCode* pByteCode, CFx** pOutEffectObj, int* outClass);

	void GetDynamicFx(CFxHandle* pHandle, uint param_3, FX_MATERIAL_SELECTOR selector);

	uint count_0x4;
	CFx** aFx;
	uint* orderedCountArray;
	uint effectCountByType[7];
	int totalEffectCount_0x2c;
	int aParticleClassSizes[7];
	CFxPoolManagerFather* aEffectCategory[7];
	s_fx_sort_data* pSortData;
	int field_0x6c;
};

struct ParticleInfo;

class CFxDigits
{
public:
	void Init(int param_2) {}
	void Draw(float param_1, float param_2, float param_3, float param_4, edF32VECTOR4* param_6, int param_7) {}

	ParticleInfo* field_0x0;
};

#endif // FX_MANAGER_H
