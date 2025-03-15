#include "FxParticle.h"
#include "MemoryStream.h"
#include "EdenLib/edBank/include/edBankBuffer.h"
#include "LevelScheduleManager.h"

bool CParticleRes::Init()
{
	int iVar2;
	ParticleInfo* pPVar3;
	ed_g2d_manager* pManager;
	TextureData_HASH_Internal_MAT* pTVar4;
	uint* puVar5;
	uint* puVar7;
	int index;
	int iVar8;
	int curEntryIndex;
	uint* pEntry;
	edBANK_ENTRY_INFO bankEntryInfo;
	C3DFileManager* pFileManager;

	CLevelScheduler::gThis->pLevelBankBufferEntry->get_info(CFxParticleManager::_prt_bank_first_index + this->fileIndex, &bankEntryInfo, (char*)0x0);

	this->pFileStart = bankEntryInfo.fileBufferStart;
	this->fileSize = bankEntryInfo.size;

	pFileManager = CScene::ptable.g_C3DFileManager_00451664;

	this->nbLoadedRes = 0;

	curEntryIndex = 0;
	pEntry = this->aEntries;
	if (0 < this->nbEntries) {
		do {
			iVar8 = *pEntry;
			iVar2 = pFileManager->InstanciateG2D(iVar8);
			pPVar3 = pFileManager->GetG2DInfo(iVar8);
			pManager = pFileManager->GetActorsCommonMaterial(iVar8);
			iVar8 = this->nbLoadedRes;
			index = 0;
			if (0 < iVar2) {
				do {
					this->aHashes[iVar8] = ed3DG2DGetMaterialFromIndex(pManager, index)->hash.number;
					this->aManagers[iVar8] = pPVar3->materialInfoArray_0x8->pManager + index;
					index = index + 1;
					iVar8 = iVar8 + 1;
					puVar5 = puVar5 + 1;
				} while (index < iVar2);
			}

			this->nbLoadedRes = iVar8;
			curEntryIndex = curEntryIndex + 1;
			pEntry = pEntry + 1;
		} while (curEntryIndex < this->nbEntries);
	}

	return true;
}

uint CFxParticleManager::_prt_bank_first_index = 0;

void CFxParticleManager::Level_Create(ByteCode* pByteCode)
{
	edCBankBufferEntry* pBankBufferEntry;
	uint uVar1;
	edBANK_ENTRY_INFO local_20;

	pBankBufferEntry = CLevelScheduler::gThis->pLevelBankBufferEntry;

	uVar1 = pBankBufferEntry->get_element_count();

	_prt_bank_first_index = 0;

	pByteCode->GetU32();

	while ((_prt_bank_first_index < uVar1 && (pBankBufferEntry->get_info(_prt_bank_first_index, &local_20, (char*)0x0), (local_20.type << 0x10 | local_20.stype) != 0x90002))) {
		_prt_bank_first_index = _prt_bank_first_index + 1;
	}

	return;
}

bool CFxParticleManager::ReadPrtResChunk(ByteCode* pByteCode)
{
	bool bSuccess;
	uint nbEntries;
	uint uVar5;
	int curEntryIndex;
	CParticleRes* pParticleRes;
	int curResIndex;

	this->nbParticleRes = pByteCode->GetU32();

	if (this->nbParticleRes == 0) {
		this->aParticleRes = (CParticleRes*)0x0;
	}
	else {
		this->aParticleRes = new CParticleRes[this->nbParticleRes];

		curResIndex = 0;
		if (0 < this->nbParticleRes) {
			do {
				pParticleRes = this->aParticleRes + curResIndex;
				pParticleRes->nbEntries = 0;

				nbEntries = pByteCode->GetU32();
				curEntryIndex = 0;
				if (0 < nbEntries) {
					do {
						uint entryType = pByteCode->GetU32();

						uVar5 = pByteCode->GetU32();

						if (entryType == 0) {
							pParticleRes->fileIndex = uVar5;
						}
						else {
							if (entryType == 1) {
								pParticleRes->aEntries[pParticleRes->nbEntries] = uVar5;
								pParticleRes->nbEntries = pParticleRes->nbEntries + 1;
							}
						}

						curEntryIndex = curEntryIndex + 1;
					} while (curEntryIndex < nbEntries);
				}

				bSuccess = this->aParticleRes[curResIndex].Init();
				curResIndex = curResIndex + 1;
			} while (curResIndex < this->nbParticleRes);
		}
	}

	return bSuccess;
}

void CFxParticleScenaricData::Init()
{
	return;
}

void CFxParticleScenaricData::Create(ByteCode* pByteCode)
{
	this->field_0x0 = pByteCode->GetU32();
	this->nbData = pByteCode->GetU32();

	if (this->nbData != 0) {
		for (uint i = 0; i < this->nbData; i++) {
			this->aSubObjs[i].field_0x0 = pByteCode->GetU32();
			this->aSubObjs[i].field_0x4 = pByteCode->GetS32();
		}
	}

	this->field_0x8 = pByteCode->GetS32();
	this->nbData = this->nbData + 1;

	return;
}
