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

template <typename FxType>
class CDoubleLinkedNode
{
public:
	CDoubleLinkedNode()
	{
		this->pNext = (CDoubleLinkedNode<FxType>*)0x0;
		this->pPrev = (CDoubleLinkedNode<FxType>*)0x0;
	}

	FxType* pFx;
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

	int field_0x4;
	undefined4 field_0x8;
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
	CFx* pFx;
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

	virtual int GetNbPool() = 0;

	char* pData;
};

template<typename FxType, typename ScenaricDataType>
class CFxPoolManager : public CFxPoolManagerFather
{
public:
	CFxPoolManager()
	{
		this->pNodeHead = (CDoubleLinkedNode<FxType>*)0x0;
		this->pNextNode = (CDoubleLinkedNode<FxType>*)0x0;
		this->field_0x10 = 0;
		this->field_0x14 = 0;
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
		CFxNewParticle* pCVar1;
		uint uVar2;
		bool bVar3;
		undefined4 uVar4;

		pCVar1 = (CFxNewParticle*)this->field_0x10;
		while (pCVar1 != (CFxNewParticle*)0x0) {
			IMPLEMENTATION_GUARD(
			uVar4 = &pCVar1->base;
			pCVar1 = *(CFxNewParticle**)((int)&pCVar1->base + 8);
			uVar2 = ((*(CFxNewParticle**)uVar4)->base).flags;
			bVar3 = false;
			if (((uVar2 & 1) != 0) && ((uVar2 & 4) == 0)) {
				bVar3 = true;
			}
			if (bVar3) {
				CFxNewParticle::Manage(*(CFxNewParticle**)uVar4);
			})
		}
		return;
	}

	virtual void Play(uint* pCount, s_fx_sort_data* pSortData, CCameraManager* pCameraManager)
	{
		int iVar1;
		bool bVar2;
		int* piVar3;
		float fVar4;
		edF32VECTOR4 local_60;
		edF32VECTOR4 local_50;
		edF32MATRIX4 eStack64;

		piVar3 = (int*)this->field_0x10;
		if (piVar3 != (int*)0x0) {
			do {
				IMPLEMENTATION_GUARD(
				iVar1 = *piVar3;
				bVar2 = false;
				if (((*(uint*)(iVar1 + 0x14) & 2) != 0) && ((*(uint*)(iVar1 + 0x14) & 8) == 0)) {
					bVar2 = true;
				}
				if (bVar2) {
					pSortData[*pCount].field_0x8 = 1;
					pSortData[*pCount].field_0x0 = iVar1;
					edF32Matrix4SetIdentityHard(&eStack64);
					edF32Matrix4TranslateHard(&eStack64, &eStack64, (edF32VECTOR4*)(iVar1 + 0x30));
					edF32Matrix4MulF32Matrix4Hard(&eStack64, &eStack64, &pCameraManager->worldToCamera_0x3d0);
					local_50.x = eStack64.ac;
					local_50.y = eStack64.bc;
					local_50.z = eStack64.cc;
					local_60.x = *(float*)(iVar1 + 0x30);
					local_60.y = *(float*)(iVar1 + 0x34);
					local_60.z = *(float*)(iVar1 + 0x38);
					local_60.w = *(float*)(iVar1 + 0x3c);
					edF32Vector4SubHard(&local_60, &local_60, (edF32VECTOR4*)&(pCameraManager->transformationMatrix).da);
					fVar4 = edF32Vector4DotProductHard(&local_50, &local_60);
					pSortData[*pCount].field_0x4 = fVar4;
					*pCount = *pCount + 1;
				}
				piVar3 = (int*)piVar3[2];)
			} while (piVar3 != (int*)0x0);
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
				pCurNode->pFx = pCurFx;
				pCurNode->pNext = this->pNextNode;
				pCurNode->pPrev = (CDoubleLinkedNode<FxType>*)0x0;

				if (this->pNextNode == (CDoubleLinkedNode<FxType>*)0x0) {
					this->pNodeHead = pCurNode;
				}
				else {
					this->pNextNode->pPrev = pCurNode;
				}
				this->pNextNode = pCurNode;

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

	CDoubleLinkedNode<FxType>* pNodeHead;
	CDoubleLinkedNode<FxType>* pNextNode;

	undefined4 field_0x10;
	undefined4 field_0x14;

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

#endif // FX_MANAGER_H
