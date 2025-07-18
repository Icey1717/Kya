#include "FxComposite.h"
#include "MemoryStream.h"
#include "MathOps.h"
#include "TimeController.h"

void CFxCompositeScenaricData::Init()
{
	return;
}

CFxCompositeScenaricData::CFxCompositeScenaricData()
{
	this->nbData = 0;
	this->aComponentParticles = (uint*)0x0;

	return;
}

void CFxCompositeScenaricData::Create(ByteCode* pByteCode)
{
	this->nbData = pByteCode->GetU32();

	if (this->nbData != 0) {
		void* pAlloc = edMemAlloc(TO_HEAP(H_MAIN), this->nbData * (sizeof(uint) + sizeof(float)));

		this->aComponentParticles = reinterpret_cast<uint*>(pAlloc);
		this->field_0x8 = reinterpret_cast<float*>(this->aComponentParticles + this->nbData);

		for (uint i = 0; i < this->nbData; i++) {
			this->aComponentParticles[i] = pByteCode->GetU32();
			this->field_0x8[i] = pByteCode->GetF32();
		}
	}

	return;
}

void* CFxCompositeManager::InstanciateFx(uint scenaricDataIndex, FX_MATERIAL_SELECTOR selector)
{
	CFxNewComposite* pNewComposite = _InstanciateFx();

	if ((pNewComposite != 0) && (scenaricDataIndex != 0xffffffff)) {
		pNewComposite->Instanciate(this->aScenaricData + scenaricDataIndex, selector);
	}

	return pNewComposite;
}


void CFxNewComposite::Draw()
{
	return;
}

void CFxNewComposite::Kill()
{
	uint curIndex;
	CFxHandle* pHandle;
	bool bValidFx;
	CNewFx* pChildFx;

	pHandle = this->aFxHandles;
	curIndex = this->nbComponentParticles;

	while (curIndex != 0) {
		curIndex = curIndex - 1;
		pChildFx = pHandle->pFx;
		if (((pChildFx == (CNewFx*)0x0) || (pHandle->id == 0)) || (bValidFx = true, pHandle->id != pChildFx->id)) {
			bValidFx = false;
		}

		if (bValidFx) {
			if (((pChildFx != (CNewFx*)0x0) && (pHandle->id != 0)) && (pHandle->id == pChildFx->id)) {
				pChildFx->Kill();
			}

			pHandle->pFx = (CNewFx*)0x0;
			pHandle->id = 0;
		}

		pHandle = pHandle + 1;
	}

	CNewFx::Kill();

	return;
}

void CFxNewComposite::Start(float param_1, float param_2)
{
	CNewFx::Start(param_1, param_2);

	this->field_0x80 = 0.0f;
	this->flags = this->flags & 0xfffffffd;

	return;
}

void CFxNewComposite::Stop(float param_1)
{
	CNewFx* pFxChild;
	bool bValidFx;
	uint curIndex;
	CFxHandle* pHandle;

	pHandle = this->aFxHandles;
	this->flags = this->flags | 0x20000;
	curIndex = this->nbComponentParticles;
	while (curIndex != 0) {
		curIndex = curIndex - 1;
		pFxChild = pHandle->pFx;

		if (((pFxChild == (CNewFx*)0x0) || (pHandle->id == 0)) || (bValidFx = true, pHandle->id != pFxChild->id)) {
			bValidFx = false;
		}

		if (((bValidFx) && (pFxChild != (CNewFx*)0x0)) && ((pHandle->id != 0 && (pHandle->id == pFxChild->id)))) {
			pFxChild->Stop(param_1);
		}

		pHandle = pHandle + 1;
	}

	return;
}

int CFxNewComposite::GetType()
{
	return FX_TYPE_COMPOSITE;
}

void CFxNewComposite::Func_0x30(float param_1)
{
	uint curIndex;
	CFxHandle* pHandle;
	CNewFx* pFxChild;

	pHandle = this->aFxHandles;
	this->field_0x70 = param_1;

	curIndex = this->nbComponentParticles;
	while (curIndex != 0) {
		curIndex = curIndex - 1;
		pFxChild = pHandle->pFx;

		if (((pFxChild != (CNewFx*)0x0) && (pHandle->id != 0)) && (pHandle->id == pFxChild->id)) {
			pFxChild->Func_0x30(param_1);
		}

		pHandle = pHandle + 1;
	}

	return;
}

void CFxNewComposite::NotifySonIsDead(CNewFx* pSon, int index)
{
	this->aFxHandles[index].id = 0;
	this->aFxHandles[index].pFx = (CNewFx*)0x0;

	return;
}

void CFxNewComposite::SpatializeOnActor(uint flags, CActor* pActor, uint boneId)
{
	CNewFx* pChildFx;
	bool bValid;
	uint handleIndex;
	CFxHandle* pHandle;

	CNewFx::SpatializeOnActor(flags, pActor, boneId);

	handleIndex = this->nbComponentParticles;
	pHandle = this->aFxHandles;
	if (handleIndex != 0) {
		do {
			handleIndex = handleIndex - 1;
			pChildFx = pHandle->pFx;

			if (((pChildFx == (CNewFx*)0x0) || (pHandle->id == 0)) || (bValid = true, pHandle->id != pChildFx->id)) {
				bValid = false;
			}

			if (((bValid) && (pChildFx != (CNewFx*)0x0)) && ((pHandle->id != 0 && (pHandle->id == pChildFx->id)))) {
				pChildFx->SpatializeOnActor(flags | 0x40, pActor, boneId);
			}

			pHandle = pHandle + 1;
		} while (handleIndex != 0);
	}

	return;
}

void CFxNewComposite::Manage()
{
	CNewFx* pCVar1;
	bool bVar2;
	Timer* pTVar3;
	CFxHandle* pCVar4;
	float* pfVar5;
	int iVar6;
	uint uVar7;

	CNewFx::Manage();

	if ((this->flags & 0x20) == 0) {
		bVar2 = (this->flags & 1) != 0;
		if (bVar2) {
			bVar2 = (this->flags & FX_FLAG_PAUSED) == 0;
		}

		if (bVar2) {
			pfVar5 = this->field_0xc8;
			iVar6 = 0;
			uVar7 = this->nbComponentParticles;
			pCVar4 = this->aFxHandles;
			pTVar3 = Timer::GetTimer();
			this->field_0x80 = this->field_0x80 + pTVar3->cutsceneDeltaTime;
			while (uVar7 != 0) {
				uVar7 = uVar7 - 1;
				pCVar1 = pCVar4->pFx;
				if (((pCVar1 == (CNewFx*)0x0) || (pCVar4->id == 0)) || (bVar2 = true, pCVar4->id != pCVar1->id)) {
					bVar2 = false;
				}
				if (bVar2) {
					if (*pfVar5 < this->field_0x80) {
						if (((pCVar1 == (CNewFx*)0x0) || (pCVar4->id == 0)) || (pCVar4->id != pCVar1->id)) {
							bVar2 = false;
						}
						else {
							bVar2 = false;
							if (((pCVar1->flags & 1) != 0) && ((pCVar1->flags & FX_FLAG_PAUSED) == 0)) {
								bVar2 = true;
							}
						}

						if ((((!bVar2) && (pCVar1 != (CNewFx*)0x0)) && (pCVar4->id != 0)) && (pCVar4->id == pCVar1->id)) {
							pCVar1->Start(0.0f, 0.0f);
						}
					}

					pCVar1 = pCVar4->pFx;
					if (((pCVar1 != (CNewFx*)0x0) && (pCVar4->id != 0)) && (pCVar4->id == pCVar1->id)) {
						pCVar1->position = this->position;
					}

					pCVar1 = pCVar4->pFx;
					if (((pCVar1 != (CNewFx*)0x0) && (pCVar4->id != 0)) && (pCVar4->id == pCVar1->id)) {
						pCVar1->rotationEuler = this->rotationEuler;
					}

					pCVar1 = pCVar4->pFx;
					if (((pCVar1 != (CNewFx*)0x0) && (pCVar4->id != 0)) && (pCVar4->id == pCVar1->id)) {
						pCVar1->scale = this->scale;
					}

					iVar6 = iVar6 + 1;
				}

				pCVar4 = pCVar4 + 1;
				pfVar5 = pfVar5 + 1;
			}

			if (iVar6 == 0) {
				Kill();
			}
		}
	}
	else {
		Kill();
	}

	return;
}

void CFxNewComposite::Instanciate(CFxCompositeScenaricData* pData, FX_MATERIAL_SELECTOR selector)
{
	CNewFx* pNewFx;
	bool bVar2;
	CFxHandle* pHandle;
	int i;
	uint* pComponentIdIt;
	uint nbComponents;
	CFxManager* pEffectsManager;

	this->position = gF32Vertex4Zero;
	this->rotationEuler = gF32Vector4Zero;
	this->scale.z = 1.0f;
	this->scale.y = 1.0f;
	this->scale.x = 1.0f;
	this->scale.w = 0.0f;
	this->pSon = (CNewFx*)0x0;

	this->nbComponentParticles = pData->nbData;
	this->field_0xc8 = pData->field_0x8;
	pEffectsManager = CScene::ptable.g_EffectsManager_004516b8;

	nbComponents = this->nbComponentParticles;
	if (nbComponents != 0) {
		pComponentIdIt = pData->aComponentParticles;
		pHandle = this->aFxHandles;
		i = 0;

		while (nbComponents != 0) {
			nbComponents = nbComponents - 1;
			pEffectsManager->GetDynamicFx(pHandle, *pComponentIdIt, selector);
			pNewFx = pHandle->pFx;

			if (((pNewFx == (CNewFx*)0x0) || (pHandle->id == 0)) ||
				(bVar2 = true, pHandle->id != pNewFx->id)) {
				bVar2 = false;
			}

			if (((bVar2) && (pNewFx != (CNewFx*)0x0)) &&
				((pHandle->id != 0 && (pHandle->id == pNewFx->id)))) {
				pNewFx->pSon = this;
				pNewFx->field_0x8 = i;
			}

			pComponentIdIt = pComponentIdIt + 1;
			pHandle = pHandle + 1;
			i = i + 1;
		}
	}

	return;
}

