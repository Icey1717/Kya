#include "EventManager.h"
#include "Actor.h"

#if defined(PLATFORM_PS2)
#include <libgraph.h>
#endif


CEventManagerInternal CEventManager::callbackFunctions = {};

EventQueue EventQueue_00448fbc;

struct EventManager_C {

};

void edEventSetCallbackFunctions(void* param_1, void* param_2, void* param_3, void* param_4)
{
	CEventManager::callbackFunctions.pGetActorPointer = param_1;
	CEventManager::callbackFunctions.pGetActorPositionVector = param_2;
	CEventManager::callbackFunctions.pGetActorIdent = param_3;
	CEventManager::callbackFunctions.pSendAllMessages = param_4;
	return;
}

CActor* EventCallbackGetActorPointer(int index) {
	return NULL;
}

CActor* EventCallbackGetActorPositionVector(int index) {
	return NULL;
}

CActor* EventCallbackGetActorIdent(int index) {
	return NULL;
}

CActor* EventCallbackSendAllMessages(int index) {
	return NULL;
}

void edEventAllocateMessageQueue(int queueSize)
{
	if (queueSize != 0) {
		EventQueue_00448fbc.aEntries = (EventManager_C*)edMemAlloc(TO_HEAP(H_MAIN), queueSize * sizeof(EventManager_C));
		EventQueue_00448fbc.totalCount = queueSize;
	}
	return;
}

CEventManager::CEventManager()
{
	edEventSetCallbackFunctions((void*)EventCallbackGetActorPointer, (void*)EventCallbackGetActorPositionVector, (void*)EventCallbackGetActorIdent, (void*)EventCallbackSendAllMessages);
	this->activeChunkId = -1;
	edEventAllocateMessageQueue(0x40);
}

void CEventManager::Level_Term()
{
	if (this->activeChunkId != -1) {
		IMPLEMENTATION_GUARD(
		edEventClearMessageQueue();
		edEventRemoveChunk(this->activeChunkId);
		this->activeChunkId = -1;
		this->field_0x4 = 0;)
	}
	return;
}

void CEventManager::Level_Manage()
{
	if (this->activeChunkId != -1) {
		IMPLEMENTATION_GUARD_LOG(
		edEventComputeChunk(this->activeChunkId, 1);)
	}
	return;
}

void CEventManager::Level_Reset()
{
	if (this->activeChunkId != -1) {
		IMPLEMENTATION_GUARD(
		edEventInitChunk(this->activeChunkId);
		edEventClearMessageQueue();)
	}
	return;
}

void CEventManager::Level_CheckpointReset()
{
	Level_Reset();
}


struct ed_event_chunk {
	void* pFileData;
	int field_0x4;
	undefined4* field_0x8;
	char* field_0xc;
	uint field_0x10;
	int* field_0x14; // 
	uint field_0x18;
	undefined4* field_0x1c;
	uint field_0x20;
	undefined4* field_0x24;

	uint nbZones;
	ed_zone_3d* aZones;

	uint count_0x30;
	uint** field_0x34;
	uint field_0x38;
	uint* field_0x3c;
	undefined field_0x40;
	undefined field_0x41;
	undefined field_0x42;
	undefined field_0x43;
};

ed_event_chunk** pedEventChunks;
uint edEventNbChunks;

uint edEventGetChunkNbEvents(int eventIndex)
{
	return pedEventChunks[eventIndex]->count_0x30;
}

struct EdEventConfig {
	uint nbChunks;
};

EdEventConfig edEventConfig = { 0xa };

void edEventInit(void)
{
	edEventNbChunks = edEventConfig.nbChunks;
	if (edEventConfig.nbChunks != 0) {
		pedEventChunks = (ed_event_chunk**)edMemAlloc(TO_HEAP(H_MAIN), edEventConfig.nbChunks * sizeof(ed_event_chunk*));
	}

	if (edEventNbChunks != 0) {
		for (int i = 0; i < edEventNbChunks; i++) {
			pedEventChunks[i] = (ed_event_chunk*)0x0;
		}
	}
	return;
}

void _edEventInstallChunk(ed_event_chunk* pEventChunk, void* pFileData, uint mode)
{
	bool bVar1;
	ed_zone_3d* peVar2;
	uint* puVar3;
	int* piVar4;
	ed_zone_3d* pZoneStream;
	int iVar6;
	uint uVar7;
	undefined4* puVar8;
	char* puVar9;
	uint uVar10;
	int iVar11;
	int iVar12;
	int iVar13;
	uint uVar14;
	int* pSubZone;

	int* pStreamData = reinterpret_cast<int*>(pFileData);

	puVar3 = (uint*)(pStreamData + 1);
	pEventChunk->pFileData = pFileData;
	pEventChunk->field_0x4 = *pStreamData;

	if (pEventChunk->field_0x4 == 0) {
		pEventChunk->field_0x8 = (undefined4*)0x0;
		pEventChunk->field_0xc = (char*)0x0;
	}
	else {
		pEventChunk->field_0x8 = puVar3;
		puVar9 = ((char*)puVar3) + pEventChunk->field_0x4;
		if (((ulong)puVar9 & 0xf) != 0) {
			// Not aligned.
			puVar9 = (char*)((((ulong)puVar9 >> 4) + 1) * 0x10);
		}
		pEventChunk->field_0xc = puVar9;
		puVar3 = (uint*)(puVar9 + pEventChunk->field_0x4 * 0x40);
	}
	pEventChunk->field_0x10 = *puVar3;
	puVar3 = puVar3 + 1;
	if (pEventChunk->field_0x10 == 0) {
		pEventChunk->field_0x14 = (int*)0x0;
	}
	else {
		pEventChunk->field_0x14 = (int*)puVar3;
	}
	uVar14 = 0;
	uVar7 = puVar3[pEventChunk->field_0x10];
	if (pEventChunk->field_0x10 != 0) {
		do {
			pEventChunk->field_0x14[uVar14] = STORE_SECTION((char*)pEventChunk->field_0x14 + pEventChunk->field_0x14[uVar14]);
			uVar14 = uVar14 + 1;
		} while (uVar14 < pEventChunk->field_0x10);
	}
	pEventChunk->nbZones = *(uint*)((ulong)puVar3 + uVar7);
	peVar2 = (ed_zone_3d*)((uint*)((ulong)puVar3 + uVar7) + 1);
	if (pEventChunk->nbZones == 0) {
		pEventChunk->aZones = (ed_zone_3d*)0x0;
	}
	else {
		pEventChunk->aZones = peVar2;
		peVar2 = peVar2 + pEventChunk->nbZones;
	}
	pZoneStream = pEventChunk->aZones;
	uVar7 = 0;
	if (pEventChunk->nbZones != 0) {
		do {
			iVar11 = 0;
			bVar1 = false;
			uVar14 = 0;
			pSubZone = pZoneStream->field_0x0;
			do {
				if (*pSubZone  == -1) {
					*pSubZone = 0x0;
				}
				else {
					*pSubZone = pEventChunk->field_0x14[*pSubZone];

					int* pLoaded = (int*)LOAD_SECTION(*pSubZone);

					iVar11 = iVar11 + *pLoaded;
					if ((iVar11 == 1) &&
						(*(char*)((ulong)pEventChunk->field_0x8 + pLoaded[1]) == '\x01')) {
						bVar1 = true;
					}
				}
				uVar14 = uVar14 + 1;
				pSubZone = pSubZone + 1;
			} while (uVar14 < 3);

			pZoneStream->flags = 0;

			if (bVar1) {
				pZoneStream->flags = pZoneStream->flags | 1;
			}

			pZoneStream->field_0x20 = 0x0;
			uVar7 = uVar7 + 1;
			pZoneStream = pZoneStream + 1;
		} while (uVar7 < pEventChunk->nbZones);
	}
	IMPLEMENTATION_GUARD_LOG(
	puVar8 = (undefined4*)&peVar2->field_0x4;
	pEventChunk->field_0x18 = (uint)peVar2->field_0x0;
	if (pEventChunk->field_0x18 == 0) {
		pEventChunk->field_0x1c = (undefined4*)0x0;
	}
	else {
		pEventChunk->field_0x1c = puVar8;
	}
	uVar7 = 0;
	iVar11 = puVar8[pEventChunk->field_0x18];
	if (pEventChunk->field_0x18 != 0) {
		iVar12 = 0;
		do {
			uVar7 = uVar7 + 1;
			piVar4 = (int*)((int)pEventChunk->field_0x1c + iVar12);
			*piVar4 = (int)pEventChunk->field_0x1c + *piVar4;
			iVar12 = iVar12 + 4;
		} while (uVar7 < pEventChunk->field_0x18);
	}
	pEventChunk->field_0x20 = *(uint*)((int)puVar8 + iVar11);
	puVar3 = (uint*)((int)puVar8 + iVar11) + 1;
	if (pEventChunk->field_0x20 == 0) {
		pEventChunk->field_0x24 = (undefined4*)0x0;
	}
	else {
		pEventChunk->field_0x24 = puVar3;
		puVar3 = puVar3 + pEventChunk->field_0x20 * 2;
	}
	pEventChunk->count_0x30 = *puVar3;
	if (pEventChunk->count_0x30 == 0) {
		pEventChunk->field_0x34 = (uint**)0x0;
	}
	else {
		pEventChunk->field_0x34 = (uint**)(puVar3 + 1);
	}
	uVar7 = 0;
	if (pEventChunk->count_0x30 != 0) {
		iVar11 = 0;
		do {
			piVar4 = (int*)((int)pEventChunk->field_0x34 + iVar11);
			iVar12 = (int)pEventChunk->field_0x34 + *piVar4;
			*piVar4 = iVar12;
			if (*(int*)(iVar12 + 4) == -1) {
				*(undefined4*)(iVar12 + 4) = 0;
			}
			else {
				*(ed_zone_3d**)(iVar12 + 4) = pEventChunk->aZones + *(int*)(iVar12 + 4);
			}
			uVar14 = 0;
			iVar13 = iVar12 + 0x10;
			if (*(int*)(iVar12 + 0xc) != 0) {
				do {
					uVar10 = 0;
					*(undefined4**)(iVar13 + 0x18) = pEventChunk->field_0x24 + *(int*)(iVar13 + 0x18) * 2;
					*(uint**)(iVar13 + 0x1c) = pEventChunk->field_0x34[*(int*)(iVar13 + 0x1c)];
					iVar6 = iVar13;
					do {
						if (*(int*)(iVar6 + 0x20) == -1) {
							*(undefined4*)(iVar6 + 0x20) = 0;
						}
						else {
							*(undefined4*)(iVar6 + 0x20) = pEventChunk->field_0x1c[*(int*)(iVar6 + 0x20)];
						}
						uVar10 = uVar10 + 1;
						iVar6 = iVar6 + 4;
					} while (uVar10 < 4);
					uVar14 = uVar14 + 1;
					iVar13 = iVar13 + 0x30;
				} while (uVar14 < *(uint*)(iVar12 + 0xc));
			}
			uVar7 = uVar7 + 1;
			iVar11 = iVar11 + 4;
		} while (uVar7 < pEventChunk->count_0x30);
	}
	_edEventInstallGameData(pEventChunk, (long)(int)mode);
	_edEventInitChunk(pEventChunk);)
	return;
}

uint edEventAddChunk(void* pFileData, uint mode)
{
	ed_event_chunk* pEventChunk;
	ed_event_chunk** pNextFreeChunk;
	uint chunkIndex;

	int* pStreamData = reinterpret_cast<int*>(pFileData);

	pEventChunk = (ed_event_chunk*)edMemAlloc(TO_HEAP(H_MAIN), sizeof(ed_event_chunk));

	chunkIndex = 0;

	pNextFreeChunk = pedEventChunks;
	if (edEventNbChunks != 0) {
		do {
			if (*pNextFreeChunk == (ed_event_chunk*)0x0) break;
			chunkIndex = chunkIndex + 1;
			pNextFreeChunk = pNextFreeChunk + 1;
		} while (chunkIndex < edEventNbChunks);
	}

	pedEventChunks[chunkIndex] = pEventChunk;

	if (*pStreamData < 0x36) {
		edDebugPrintf("\nedEventAddChunk : incompatible .bin version number,\nplease recompile with at least compiler v%d.0%d\n", 0, 0x36);
	}
	_edEventInstallChunk(pEventChunk, pStreamData + 1, mode);
	return chunkIndex;
}

ed_zone_3d* edEventGetChunkZone(uint chunkId, uint zoneId)
{
	return pedEventChunks[chunkId]->aZones + zoneId;
}
