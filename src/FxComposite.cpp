#include "FxComposite.h"
#include "MemoryStream.h"
#include "MathOps.h"

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

void* CFxCompositeManager::InstanciateFx(uint param_2, FX_MATERIAL_SELECTOR selector)
{
	CFxNewComposite* pNewComposite = _InstanciateFx();

	if ((pNewComposite != 0) && (param_2 != 0xffffffff)) {
		pNewComposite->Instanciate(this->aScenaricData + param_2, selector);
	}

	return pNewComposite;
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

	this->field_0x30 = gF32Vertex4Zero;
	this->field_0x50 = gF32Vector4Zero;
	this->field_0x48 = 1.0f;
	this->field_0x44 = 1.0f;
	this->field_0x40 = 1.0f;
	this->field_0x4c = 0.0f;
	this->field_0x4 = 0;

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
				(bVar2 = true, pHandle->id != pNewFx->field_0x18)) {
				bVar2 = false;
			}

			if (((bVar2) && (pNewFx != (CNewFx*)0x0)) &&
				((pHandle->id != 0 && (pHandle->id == pNewFx->field_0x18)))) {
				pNewFx->field_0x4 = this;
				pNewFx->field_0x8 = i;
			}

			pComponentIdIt = pComponentIdIt + 1;
			pHandle = pHandle + 1;
			i = i + 1;
		}
	}

	return;
}

