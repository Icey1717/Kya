#include "FxParticle.h"
#include "MemoryStream.h"
#include "EdenLib/edBank/include/edBankBuffer.h"
#include "LevelScheduleManager.h"
#include "MathOps.h"
#include "TimeController.h"
#include "Rendering/DisplayList.h"

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
					this->aMaterials[iVar8] = pPVar3->materialInfoArray_0x8 + index;
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

CFxParticleManager::CFxParticleManager()
{
	this->pPoolHeap = edMemAlloc(TO_HEAP(H_MAIN), 0x3e800);

	return;
}

CFxParticleManager::~CFxParticleManager()
{
	_prt_bank_first_index = 0;

	return;
}

void* CFxParticleManager::InstanciateFx(uint param_2, FX_MATERIAL_SELECTOR selector)
{
	CFxNewParticle* pNewParticle;

	pNewParticle = _InstanciateFx();
	if ((pNewParticle != 0) && (param_2 != 0xffffffff)) {
		pNewParticle->Instanciate(this->aScenaricData + param_2, selector);
	}

	return pNewParticle;
}

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

void CFxParticleScenaricData::Term()
{
	return;
}

void CFxParticleScenaricData::Create(ByteCode* pByteCode)
{
	this->field_0x0 = pByteCode->GetU32();
	this->nbData = pByteCode->GetU32();

	if (this->nbData != 0) {
		for (uint i = 0; i < this->nbData; i++) {
			this->aSubObjs[i].selectorType = pByteCode->GetU32();
			this->aSubObjs[i].field_0x4 = pByteCode->GetS32();
		}
	}

	this->aSubObjs[0].field_0x4 = pByteCode->GetS32();
	this->nbData = this->nbData + 1;

	return;
}

void CFxNewParticle::Draw()
{
	bool bVar1;
	float fVar2;
	edF32MATRIX4 local_40;

	if (this->pManager != (_ed_particle_manager*)0x0) {
		edParticlesSetSystem(this->pManager);

		local_40 = gF32Matrix4Unit;

		if ((this->field_0x84 & 1) == 0) {
			edF32Matrix4FromEulerSoft(&local_40, &this->rotationEuler.xyz, "XYZ");

			local_40.rowX = local_40.rowX * this->scale.x;
			local_40.rowY = local_40.rowY * this->scale.y;
			local_40.rowZ = local_40.rowZ * this->scale.z;
			local_40.rowT = this->position;
		}

		edPartSetDisplayMatrix(this->pManager, &local_40);

		bVar1 = GameDList_BeginCurrent();
		if (bVar1 != false) {
			edParticlesDraw(this->pManager, this->field_0x70);
			GameDList_EndCurrent();
		}
	}
	return;
}

void CFxNewParticle::Kill()
{
	if (this->pManager != (_ed_particle_manager*)0x0) {
		edParticlesUnInstall(this->pManager, CScene::_scene_handleA);
		edMemFree(this->pFileData);
	}

	CNewFx::Kill();

	return;
}

void CFxNewParticle::Stop(float param_1)
{
	this->flags = this->flags | 0x20000;

	if (this->pManager != (_ed_particle_manager*)0x0) {
		edPartSetRespawning(this->pManager, 0);
	}

	return;
}

int CFxNewParticle::GetType()
{
	return FX_TYPE_PARTICLE;
}

void CFxNewParticle::Manage()
{
	uint uVar1;
	bool bVar2;
	_ed_particle_group* p_Var4;
	int iVar5;
	int iVar6;
	_ed_particle_generator_param* p_Var7;
	edF32MATRIX4 eStack128;
	edF32MATRIX4 eStack64;

	CNewFx::Manage();

	if ((this->field_0x84 & 0x20) == 0) {
		uVar1 = this->flags;
		bVar2 = false;
		if (((uVar1 & 1) != 0) && ((uVar1 & 4) == 0)) {
			bVar2 = true;
		}

		if (bVar2) {
			if (this->pManager == (_ed_particle_manager*)0x0) {
				Kill();
			}
			else {
				edParticlesSetSystem(this->pManager);

				if (((this->field_0x84 & 1) != 0) && ((this->field_0x84 & 2) == 0)) {
					p_Var7 = this->pManager->aGeneratorParams.pData;
					iVar6 = this->pManager->nbGeneratorParams;
					edF32Matrix4ScaleHard(&eStack128, &gF32Matrix4Unit, &this->scale);
					edF32Matrix4FromEulerSoft(&eStack64, &this->rotationEuler.xyz, "XYZ");
					edF32Matrix4TranslateHard(&eStack64, &eStack64, &this->position);
					edF32Matrix4MulF32Matrix4Hard(&eStack64, &eStack128, &eStack64);
					for (; iVar6 != 0; iVar6 = iVar6 + -1) {
						edF32Matrix4MulF32Matrix4Hard(&p_Var7->field_0x40, &p_Var7->field_0x80, &eStack64);
						p_Var7 = p_Var7 + 1;
					}
				}

				edParticlesUpdate(this->field_0x80 * Timer::GetTimer()->cutsceneDeltaTime);

				iVar6 = this->pManager->nbTotalGroups;
				iVar5 = 0;
				if (0 < iVar6) {
					p_Var4 = this->pManager->aGroups.pData;
					do {
						bVar2 = p_Var4->field_0x14 != 0;
						if (!bVar2) {
							bVar2 = p_Var4->field_0x5c != 0;
						}

						if (!(bool)(bVar2 ^ 1)) break;

						iVar5 = iVar5 + 1;
						p_Var4 = p_Var4 + 1;
					} while (iVar5 < iVar6);
				}

				if (iVar5 == iVar6) {
					Kill();
				}
			}
		}
	}
	else {
		Kill();
	}

	return;
}

void CFxNewParticle::Instanciate(CFxParticleScenaricData* pData, FX_MATERIAL_SELECTOR selector)
{
	uint size;
	int iVar1;
	ParticleFileData* pPVar2;
	CFxParticleScenaricData::CFxParticleScenaricDataSubObj* piVar4;
	int iVar6;
	CParticleRes* pCVar7;
	CFxParticleManager* pFxParticleManager;

	iVar6 = 1;
	this->position = gF32Vertex4Zero;
	this->rotationEuler = gF32Vector4Zero;
	this->scale.z = 1.0f;
	this->scale.y = 1.0f;
	this->scale.x = 1.0f;
	this->scale.w = 0.0f;
	this->pSon = (CNewFx*)0x0;
	this->pFileData = (ParticleFileData*)0x0;
	this->pManager = (_ed_particle_manager*)0x0;
	this->field_0x80 = 1.0f;

	pFxParticleManager = static_cast<CFxParticleManager*>(CScene::ptable.g_EffectsManager_004516b8->aEffectCategory[3]);
	iVar1 = 0;
	if (1 < pData->nbData) {
		piVar4 = pData->aSubObjs + 1;
		do {
			iVar1 = iVar6;

			if (piVar4->selectorType == selector) break;

			iVar6 = iVar6 + 1;
			piVar4 = piVar4 + 1;
			iVar1 = 0;
		} while (iVar6 < pData->nbData);
	}

	if (((selector != 8) && (selector != FX_MATERIAL_SELECTOR_ICE)) && (iVar1 = pData->aSubObjs[iVar1].field_0x4, iVar1 != -1)) {
		pCVar7 = pFxParticleManager->aParticleRes + iVar1;
		size = pCVar7->fileSize;

		if (size != 0) {
			iVar1 = edMemGetMemoryAvailable(pFxParticleManager->pPoolHeap);
			if ((int)size < iVar1) {
				pPVar2 = (ParticleFileData*)edMemAlloc(pFxParticleManager->pPoolHeap, size);
				if (pPVar2 == (ParticleFileData*)0x0) {
					IMPLEMENTATION_GUARD(
					edMemDump(pFxParticleManager->pPoolHeap, 2);)
				}
			}
			else {
				pPVar2 = (ParticleFileData*)0x0;
			}

			this->pFileData = pPVar2;
			if (this->pFileData != (ParticleFileData*)0x0) {
				memcpy(this->pFileData, pCVar7->pFileStart, size);
				this->pManager = edParticlesInstall(this->pFileData, CScene::_scene_handleA, 0, pCVar7->aMaterials, pCVar7->aHashes, pCVar7->nbLoadedRes, (ed_g3d_manager*)0x0, true);
				this->field_0x84 = pData->field_0x0;
			}
		}
	}
	return;
}
