#include "EventManager.h"
#include "Actor.h"

#if defined(PLATFORM_PS2)
#include <libgraph.h>
#endif
#include "ActorManager.h"
#include "MathOps.h"

ed_event_chunk** pedEventChunks;
uint edEventNbChunks;

CEventManagerInternal CEventManager::callbackFunctions = {};

EventQueue EventQueue_00448fbc;

void edEventSetCallbackFunctions(EventCallbackGetActor* param_1, EventGetActorPositionFunc* param_2, void* param_3, EventSendAllMsgs* param_4)
{
	CEventManager::callbackFunctions.pGetActorPointer = param_1;
	CEventManager::callbackFunctions.pGetActorPositionVector = param_2;
	CEventManager::callbackFunctions.pGetActorIdent = param_3;
	CEventManager::callbackFunctions.pSendAllMessages = param_4;
	return;
}

CActor* EventCallbackGetActorPointer(int index) {
	CActor* pActor;

	if (index == -1) {
		pActor = (CActor*)0x0;
	}
	else {
		pActor = (CScene::ptable.g_ActorManager_004516a4)->aActors[index];
	}

	return pActor;
}

edF32VECTOR4* EventCallbackGetActorPositionVector(CActor* pActor) {
	return &pActor->currentLocation;
}

CActor* EventCallbackGetActorIdent(int index) {
	return NULL;
}

void EventCallbackSendMessage(edCEventMessage* pEventMessage)
{
	bool bVar1;
	int* piVar2;
	int iVar3;
	int iVar4;
	CActor* pActor;
	int* piVar5;
	int iVar6;
	int iVar7;

	piVar2 = LOAD_SECTION_CAST(int*, pEventMessage->field_0x4->field_0x20[pEventMessage->field_0x0]);

	iVar7 = piVar2[1];
	piVar5 = piVar2 + 2;
	iVar6 = (*piVar2 + -1) - iVar7;
	iVar4 = iVar7 + 2;
	if (iVar7 == 0) {
		ed_event_actor_ref* pRef = LOAD_SECTION_CAST(ed_event_actor_ref*, pEventMessage->field_0x4->field_0x18);
		CActor* pActorRef = LOAD_SECTION_CAST(CActor*, pRef->pActor);
		pActorRef->ReceiveEvent(pEventMessage, pEventMessage->field_0x0, iVar6, (uint*)(piVar2 + iVar4));
	}
	else {
		while (bVar1 = iVar7 != 0, iVar7 = iVar7 + -1, bVar1) {
			iVar3 = *piVar5;

			if (iVar3 == -1) {
				ed_event_actor_ref* pRef = LOAD_SECTION_CAST(ed_event_actor_ref*, pEventMessage->field_0x4->field_0x18);
				CActor* pActorRef = LOAD_SECTION_CAST(CActor*, pRef->pActor);

				pActor = pActorRef;
			}
			else {
				if (iVar3 == -1) {
					pActor = (CActor*)0x0;
				}
				else {
					pActor = (CScene::ptable.g_ActorManager_004516a4)->aActors[iVar3];
				}
			}
	
			pActor->ReceiveEvent(pEventMessage, (long)pEventMessage->field_0x0, iVar6, (uint*)(piVar2 + iVar4));
			piVar5 = piVar5 + 1;
		}
	}
	return;
}

void EventCallbackSendAllMessages(edCEventMessage* pQueue, int count) {
	bool bVar1;
	int iVar2;

	iVar2 = count + -1;
	if (count != 0) {
		do {
			EventCallbackSendMessage(pQueue);
			pQueue = pQueue + 1;
			bVar1 = iVar2 != 0;
			iVar2 = iVar2 + -1;
		} while (bVar1);
	}
	return;
}

void edEventAllocateMessageQueue(int queueSize)
{
	if (queueSize != 0) {
		EventQueue_00448fbc.aEntries = (edCEventMessage*)edMemAlloc(TO_HEAP(H_MAIN), queueSize * sizeof(edCEventMessage));
		EventQueue_00448fbc.totalCount = queueSize;
	}
	return;
}

float edF32Vector3DotProductSoft(edF32VECTOR3* param_1, edF32VECTOR3* param_2)
{
	return param_1->z * param_2->z + param_1->x * param_2->x + param_1->y * param_2->y;
}

int _edEventZoneAgainstVertexValidatedResult[3] = { 1, 2, 1 };

int _edEventComputePrimAgainstVertex(uint param_1, edF32MATRIX4* param_2, edF32VECTOR4* param_3)
{
	float fVar1;
	edF32VECTOR4 local_10;

	edF32Matrix4MulF32Vector4Hard(&local_10, param_2, param_3);

	switch (param_1) {
	case 0:
		goto joined_r0x00258adc;
	case 1:
		if (local_10.x < -0.5f) {
			return 2;
		}

		if (0.5f < local_10.x) {
			return 2;
		}

		if (local_10.z < -0.5f) {
			return 2;
		}

		if (0.5f < local_10.z) {
			return 2;
		}
	joined_r0x00258adc:
		if (0.0f <= local_10.y) {
			return 1;
		}
		break;
	case 2:
		if ((((-0.5f <= local_10.x) && (local_10.x <= 0.5f)) &&
			((-0.5f <= local_10.y && ((local_10.y <= 0.5f && (-0.5f <= local_10.z)))))) && (local_10.z <= 0.5f)) {
			return 1;
		}
		break;
	case 3:
		if (local_10.y < -0.5f) {
			return 2;
		}

		if (0.5f < local_10.y) {
			return 2;
		}

		fVar1 = local_10.x * local_10.x + local_10.z * local_10.z;

		goto joined_r0x00258cbc;
	case 4:
		if (((-0.5f <= local_10.y) && (local_10.y <= 0.5f)) &&
			(local_10.x * local_10.x + local_10.z * local_10.z <= (0.5f - local_10.y) * (0.5f - local_10.y))) {
			return 1;
		}
		break;
	case 5:
		fVar1 = local_10.z * local_10.z + local_10.x * local_10.x + local_10.y * local_10.y;
	joined_r0x00258cbc:
		if (fVar1 <= 1.0) {
			return 1;
		}
	}
	return 2;
}

int edEventComputeZoneAgainstVertex(ed_event_chunk* pEventChunk, ed_zone_3d* pZone, edF32VECTOR4* pLocation, uint mode)
{
	byte* pbVar1;
	edF32MATRIX4* puVar2;
	uint uVar3;
	int iVar4;
	uint* puVar5;
	uint uVar6;
	int* piVar7;
	uint uVar8;
	float fVar9;
	float fVar10;
	uint local_30;
	edF32VECTOR4 eStack32;
	edF32VECTOR3 local_10;

	if (pZone->pMatrix != 0x0) {
		edF32Matrix4MulF32Vector4Hard(&eStack32, LOAD_SECTION_CAST(edF32MATRIX4*, pZone->pMatrix), pLocation);
		pLocation = &eStack32;
	}

	if ((pZone->boundSphere).w != 0.0f) {
		local_10 = pLocation->xyz - (pZone->boundSphere).xyz;
		fVar10 = edF32Vector3DotProductSoft(&local_10, &local_10);
		fVar9 = (pZone->boundSphere).w;

		if (fVar9 * fVar9 < fVar10) {
			return 2;
		}
	}

	pbVar1 = pEventChunk->field_0x8;
	puVar2 = pEventChunk->aMatrices;
	uVar6 = 7;

	if (mode != 0) {
		uVar6 = 5;
	}

	uVar8 = 0;
	piVar7 = _edEventZoneAgainstVertexValidatedResult;
	do {
		if (((uVar6 & 1) != 0) && (puVar5 = LOAD_SECTION_CAST(uint*, pZone->field_0x0[uVar8]), puVar5 != (uint*)0x0)) {
			uVar3 = *puVar5;
			local_30 = 0;
			if (uVar3 != 0) {
				do {
					puVar5 = puVar5 + 1;
					iVar4 = _edEventComputePrimAgainstVertex(pbVar1[*puVar5], puVar2 + *puVar5, pLocation);

					if (iVar4 == 1) {
						return *piVar7;
					}

					local_30 = local_30 + 1;
				} while (local_30 < uVar3);
			}
		}

		uVar8 = uVar8 + 1;
		piVar7 = piVar7 + 1;
		uVar6 = uVar6 >> 1;
	} while (uVar8 < 3);

	return 2;
}

void _edEventAddMessage(ed_event_chunk* pEventChunk, uint param_2, _ed_event_collider_test* param_3)
{
	edCEventMessage* peVar1;

	peVar1 = EventQueue_00448fbc.aEntries + EventQueue_00448fbc.inUseCount;
	peVar1->field_0x0 = param_2;
	peVar1->field_0x4 = param_3;
	peVar1->pEventChunk = pEventChunk;
	EventQueue_00448fbc.inUseCount = EventQueue_00448fbc.inUseCount + 1;
	return;
}

void _edEventComputeEvent(ed_event_chunk* pEventChunk, ed_event* pEvent)
{
	byte bVar1;
	ed_zone_3d* peVar2;
	bool bVar3;
	uint uVar4;
	int iVar5;
	uint uVar6;
	_ed_event_collider_test* peVar7;
	byte* pbVar8;
	uint uVar9;
	int* ppuVar10;

	if ((pEvent->flags & 0x80) != 0) {
		uVar9 = pEvent->field_0xc;
		bVar3 = false;
		uVar6 = 0;
		peVar7 = (_ed_event_collider_test*)(pEvent + 1);

		if (uVar9 != 0) {
			do {
				peVar2 = LOAD_SECTION_CAST(ed_zone_3d*, pEvent->pZone);
				if ((peVar2->flags & 1) == 0) {
					ed_event_actor_ref* pActorRef = LOAD_SECTION_CAST(ed_event_actor_ref*, peVar7->field_0x18);
					uVar4 = edEventComputeZoneAgainstVertex(pEventChunk, peVar2, LOAD_SECTION_CAST(edF32VECTOR4*, pActorRef->pLocation), 0);
					uVar9 = peVar7->field_0x14;
				}
				else {
					IMPLEMENTATION_GUARD(
					ed_event_actor_ref* pActorRef = LOAD_SECTION_CAST(ed_event_actor_ref*, peVar7->field_0x18);
					uVar4 = _edEventComputeZeroVolumeZoneAgainstVertex(pEventchunk, peVar2, (edF32VECTOR4*)peVar7->field_0x18->pLocation, (edF32VECTOR4*)peVar7,
						(uint*)&peVar7->field_0x14);
					uVar9 = peVar7->field_0x14;)
				}

				if ((uVar4 & uVar9) == 0) {
					if ((uVar4 & 1) == 0) {
						peVar7->field_0x14 = 10;
					}
					else {
						peVar7->field_0x14 = 5;
					}
				}
				else {
					peVar7->field_0x14 = uVar9 & 0xfffffff3;
				}

				uVar9 = pEvent->field_0xc;
				uVar6 = uVar6 + 1;
				peVar7 = peVar7 + 1;
			} while (uVar6 < uVar9);
		}

		uVar6 = 0;
		peVar7 = (_ed_event_collider_test*)(pEvent + 1);

		if (uVar9 != 0) {
			do {
				uVar9 = 0;
				ppuVar10 = peVar7->field_0x20;
				do {
					iVar5 = peVar7->field_0x20[uVar9];

					byte* pLoaded = LOAD_SECTION_CAST(byte*, peVar7->field_0x20[uVar9]);

					byte* pByte = pLoaded + 0x10;

					if (((*ppuVar10 != 0x0) &&
						(pbVar8 = pByte, (*pByte & 0x80) != 0)) &&
						((peVar7->field_0x14 & 1 << (uVar9 & 0x1f)) != 0)) {
						_edEventAddMessage(pEventChunk, uVar9, peVar7);
						bVar1 = *pbVar8;
						bVar3 = true;

						if ((bVar1 & 2) != 0) {
							*pbVar8 = bVar1 & 0x7f;
						}
					}

					uVar9 = uVar9 + 1;
					ppuVar10 = ppuVar10 + 1;
				} while (uVar9 < 4);

				uVar6 = uVar6 + 1;
				peVar7 = peVar7 + 1;
			} while (uVar6 < pEvent->field_0xc);
		}

		if ((bVar3) && ((pEvent->flags & 2) != 0)) {
			pEvent->flags = pEvent->flags & 0xffffff7f;
		}
	}
	return;
}

void edEventClearMessageQueue(void)
{
	EventQueue_00448fbc.inUseCount = 0;
	return;
}

void _edEventSendMessages(bool param_1)
{
	uint uVar1;
	edCEventMessage* pEVar2;

	if (CEventManager::callbackFunctions.pSendAllMessages != (void*)0x0) {
		if (param_1 == false) {
			CEventManager::callbackFunctions.pSendAllMessages(EventQueue_00448fbc.aEntries, EventQueue_00448fbc.inUseCount);
		}
		else {
			uVar1 = 0;
			pEVar2 = EventQueue_00448fbc.aEntries;
			if (EventQueue_00448fbc.totalCount != 0) {
				do {
					CEventManager::callbackFunctions.pSendAllMessages(pEVar2, 1);
					uVar1 = uVar1 + 1;
					pEVar2 = pEVar2 + 1;
				} while (uVar1 < (uint)EventQueue_00448fbc.totalCount);
			}
		}
	}

	edEventClearMessageQueue();
	return;
}

void edEventComputeChunk(int activeChunkID, bool param_2)
{
	ed_event_chunk* pEventchunk;
	int* ppuVar1;
	uint uVar2;

	pEventchunk = pedEventChunks[activeChunkID];

	if ((pEventchunk->count_0x38 == 0) || (param_2 == false)) {
		ppuVar1 = pEventchunk->aEvents;
		uVar2 = 0;
		if (pEventchunk->nbEvents != 0) {
			do {
				_edEventComputeEvent(pEventchunk, LOAD_SECTION_CAST(ed_event*, *ppuVar1));
				uVar2 = uVar2 + 1;
				ppuVar1 = ppuVar1 + 1;
			} while (uVar2 < pEventchunk->nbEvents);
		}
	}

	_edEventSendMessages(false);
	return;
}

CEventManager::CEventManager()
{
	edEventSetCallbackFunctions(EventCallbackGetActorPointer, EventCallbackGetActorPositionVector, (void*)EventCallbackGetActorIdent, EventCallbackSendAllMessages);
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
		edEventComputeChunk(this->activeChunkId, 1);
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

uint edEventGetChunkNbEvents(int eventIndex)
{
	return pedEventChunks[eventIndex]->nbEvents;
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

void _edEventInstallGameData(ed_event_chunk* pEventChunk, int param_2)
{
	ed_event* peVar1;
	ed_zone_3d* peVar2;
	ed_event_alloc_10* puVar3;
	_edCluster* pvVar4;
	float* pfVar5;
	undefined4 uVar6;
	uint uVar7;
	int iVar8;
	uint uVar9;
	_ed_event_collider_test* pAfter;
	ed_event_actor_ref* puVar11;
	uint uVar12;
	int* ppeVar13;
	uint uVar14;
	_edCluster* ppfVar15;
	float fVar16;
	float fVar17;
	ed_zone_3d* peVar18;

	uVar9 = 0;
	puVar11 = pEventChunk->aActorRefs;
	if (pEventChunk->nbActorRefs != 0) {
		do {
			puVar11->pLocation = 0;
			uVar9 = uVar9 + 1;
			puVar11 = puVar11 + 1;
		} while (uVar9 < pEventChunk->nbActorRefs);
	}

	ppeVar13 = pEventChunk->aEvents;
	uVar9 = 0;
	if (pEventChunk->nbEvents != 0) {
		do {
			peVar1 = LOAD_SECTION_CAST(ed_event*, *ppeVar13);
			uVar12 = 0;
			pAfter = (_ed_event_collider_test*)(peVar1 + 1);

			if (peVar1->field_0xc != 0) {
				do {
					uVar12 = uVar12 + 1;
					int* pLoaded = LOAD_SECTION_CAST(int*, pAfter->field_0x18);
					*pLoaded = *pLoaded + 1;
					pAfter = pAfter + 1;
				} while (uVar12 < peVar1->field_0xc);
			}

			uVar9 = uVar9 + 1;
			ppeVar13 = ppeVar13 + 1;
		} while (uVar9 < pEventChunk->nbEvents);
	}

	pEventChunk->count_0x38 = 1;

	if (param_2 == 0) {
		pEventChunk->count_0x38 = 0;
	}

	if (pEventChunk->count_0x38 != 0) {
		puVar3 = (ed_event_alloc_10*)edMemAlloc(TO_HEAP(H_MAIN), pEventChunk->count_0x38 * sizeof(ed_event_alloc_10));
		pEventChunk->field_0x3c = puVar3;
		uVar9 = 0xffffffff;
		puVar3 = pEventChunk->field_0x3c;
		uVar12 = 0;

	
		if (pEventChunk->count_0x38 != 0) {
			do {
				uVar14 = 0;
				uVar7 = 0;
				puVar11 = pEventChunk->aActorRefs;
				if (pEventChunk->nbActorRefs != 0) {
					do {
						if (uVar14 <= puVar11->pLocation) {
							uVar9 = uVar7;
							uVar14 = puVar11->pLocation;
						}
						uVar7 = uVar7 + 1;
						puVar11 = puVar11 + 1;
					} while (uVar7 < pEventChunk->nbActorRefs);
				}

				puVar11 = pEventChunk->aActorRefs + uVar9;
				puVar11->pLocation = 0;
				puVar3->clusterCount = uVar14;
				puVar3->field_0x4 = puVar11;
				pvVar4 = (_edCluster*)edMemAlloc(TO_HEAP(H_MAIN), puVar3->clusterCount * sizeof(_edCluster));
				puVar3->aClusters = pvVar4;

				ppfVar15 = puVar3->aClusters;
				ppeVar13 = pEventChunk->aEvents;
				uVar14 = 0;
				if (pEventChunk->nbEvents != 0) {
					do {
						peVar1 = LOAD_SECTION_CAST(ed_event*, *ppeVar13);
						uVar7 = 0;
						pAfter = (_ed_event_collider_test*)(peVar1 + 1);
						if (peVar1->field_0xc != 0) {
							do {
								if (LOAD_SECTION_CAST(ed_event_actor_ref*, pAfter->field_0x18) == puVar11) {
									IMPLEMENTATION_GUARD(
									pfVar5 = edClusterInsertNode(ppfVar15, 4, (long)(int)&peVar1->field_0x4->field_0xc, peVar1);
									ppfVar15->field_0x30 = pfVar5;
									ppfVar15 = ppfVar15 + 1;)
									break;
								}

								uVar7 = uVar7 + 1;
								pAfter = pAfter + 1;
							} while (uVar7 < peVar1->field_0xc);
						}

						uVar14 = uVar14 + 1;
						ppeVar13 = ppeVar13 + 1;
					} while (uVar14 < pEventChunk->nbEvents);
				}

				uVar12 = uVar12 + 1;
				puVar3 = puVar3 + 1;
			} while (uVar12 < pEventChunk->count_0x38);
		}
	}

	puVar11 = pEventChunk->aActorRefs;
	uVar9 = 0;
	if (pEventChunk->nbActorRefs != 0) {
		do {
			CActor* pActor = CEventManager::callbackFunctions.pGetActorPointer(puVar11->pActor);
			puVar11->pActor = STORE_SECTION(pActor);
			puVar11->pLocation = STORE_SECTION(CEventManager::callbackFunctions.pGetActorPositionVector(pActor));			
			uVar9 = uVar9 + 1;
			puVar11 = puVar11 + 1;
		} while (uVar9 < pEventChunk->nbActorRefs);
	}

	ppeVar13 = pEventChunk->aEvents;
	uVar9 = 0;
	if (pEventChunk->nbEvents != 0) {
		do {
			peVar1 = LOAD_SECTION_CAST(ed_event*, *ppeVar13);
			peVar2 = LOAD_SECTION_CAST(ed_zone_3d*, peVar1->pZone);
			uVar12 = 0;
			pAfter = (_ed_event_collider_test*)(peVar1 + 1);

			if (peVar1->field_0xc != 0) {
				do {
					if ((peVar2->flags & 1) == 0) {
						ed_event_actor_ref* pActorRef = LOAD_SECTION_CAST(ed_event_actor_ref*, pAfter->field_0x18);
						pAfter->field_0x0 = *LOAD_SECTION_CAST(edF32VECTOR4*, pActorRef->pLocation);
					}
					else {
						IMPLEMENTATION_GUARD(
						if (peVar2->field_0x0[0] == 0x0) {
							iVar8 = *(int*)(peVar2->field_0x0[2] + 4);
						}
						else {
							iVar8 = *(int*)(peVar2->field_0x0[0] + 4);
						}

						edF32Matrix4MulF32Vector4Hard
						((edF32VECTOR4*)(peVar10 + 1), (edF32MATRIX4*)(pEventChunk->aMatrices + iVar8 * 0x40),
							*(edF32VECTOR4**)(*(int*)&peVar10[2].field_0x8 + 4));)
					}

					uVar12 = uVar12 + 1;
					pAfter = pAfter + 1;
				} while (uVar12 < peVar1->field_0xc);
			}

			uVar9 = uVar9 + 1;
			ppeVar13 = ppeVar13 + 1;
		} while (uVar9 < pEventChunk->nbEvents);
	}

	return;
}

void _edEventInitChunk(ed_event_chunk* pEventChunk)

{
	byte bVar1;
	ed_event* peVar2;
	int* ppeVar3;
	uint uVar4;
	char* iVar5;
	byte* pbVar6;
	int* ppuVar7;
	uint uVar8;
	_ed_event_collider_test* peVar9;
	uint uVar10;

	ppeVar3 = pEventChunk->aEvents;
	uVar4 = 0;
	if (pEventChunk->nbEvents != 0) {
		do {
			peVar2 = LOAD_SECTION_CAST(ed_event*, *ppeVar3);
			uVar10 = 0;
			peVar9 = (_ed_event_collider_test*)(peVar2 + 1);
			if (peVar2->field_0xc != 0) {
				do {
					peVar9->field_0x14 = 2;
					uVar8 = 0;
					ppuVar7 = peVar9->field_0x20;
					do {
						iVar5 = LOAD_SECTION_CAST(char*, peVar9->field_0x20[uVar8]);
						if (*ppuVar7 != 0x0) {
							bVar1 = *(byte*)(iVar5 + 0x10);
							pbVar6 = (byte*)(iVar5 + 0x10);
							if ((bVar1 & 1) == 0) {
								*pbVar6 = bVar1 & 0x7f;
							}
							else {
								*pbVar6 = bVar1 | 0x80;
							}
						}

						uVar8 = uVar8 + 1;
						ppuVar7 = ppuVar7 + 1;
					} while (uVar8 < 4);

					uVar10 = uVar10 + 1;
					peVar9 = peVar9 + 1;
				} while (uVar10 < peVar2->field_0xc);
			}

			uVar10 = peVar2->flags;
			if ((uVar10 & 1) == 0) {
				peVar2->flags = uVar10 & 0xffffff7f;
			}
			else {
				peVar2->flags = uVar10 | 0x80;
			}

			uVar4 = uVar4 + 1;
			ppeVar3 = ppeVar3 + 1;
		} while (uVar4 < pEventChunk->nbEvents);
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
	int* puVar8;
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
	pEventChunk->nbMatrices = *pStreamData;

	if (pEventChunk->nbMatrices == 0) {
		pEventChunk->field_0x8 = (byte*)0x0;
		pEventChunk->aMatrices = (edF32MATRIX4*)0x0;
	}
	else {
		pEventChunk->field_0x8 = reinterpret_cast<byte*>(puVar3);
		puVar9 = ((char*)puVar3) + pEventChunk->nbMatrices;
		if (((ulong)puVar9 & 0xf) != 0) {
			// Not aligned.
			puVar9 = (char*)((((ulong)puVar9 >> 4) + 1) * 0x10);
		}

		pEventChunk->aMatrices = reinterpret_cast<edF32MATRIX4*>(puVar9);
		puVar3 = (uint*)(puVar9 + pEventChunk->nbMatrices * sizeof(edF32MATRIX4));
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

			pZoneStream->pMatrix = 0x0;
			uVar7 = uVar7 + 1;
			pZoneStream = pZoneStream + 1;
		} while (uVar7 < pEventChunk->nbZones);
	}

	int* pNewObj = reinterpret_cast<int*>(peVar2);

	puVar8 = pNewObj + 1;
	pEventChunk->count_0x18 = *pNewObj;
	if (pEventChunk->count_0x18 == 0) {
		pEventChunk->field_0x1c = (int*)0x0;
	}
	else {
		pEventChunk->field_0x1c = puVar8;
	}
	
	uVar7 = 0;
	iVar11 = puVar8[pEventChunk->count_0x18];
	if (pEventChunk->count_0x18 != 0) {
		do {
			const int offset = pEventChunk->field_0x1c[uVar7];
			pEventChunk->field_0x1c[uVar7] = STORE_SECTION(reinterpret_cast<char*>(pEventChunk->field_0x1c) + offset);
			uVar7 = uVar7 + 1;
		} while (uVar7 < pEventChunk->count_0x18);
	}

	char* pNextObj = reinterpret_cast<char*>(puVar8) + iVar11;
	pEventChunk->nbActorRefs = *reinterpret_cast<int*>(pNextObj);
	pNextObj = reinterpret_cast<char*>(puVar8) + iVar11 + sizeof(int);
	if (pEventChunk->nbActorRefs == 0) {
		pEventChunk->aActorRefs = (ed_event_actor_ref*)0x0;
	}
	else {
		pEventChunk->aActorRefs = reinterpret_cast<ed_event_actor_ref*>(pNextObj);
		pNextObj = pNextObj + pEventChunk->nbActorRefs * sizeof(ed_event_actor_ref);
	}

	pEventChunk->nbEvents = *reinterpret_cast<int*>(pNextObj);
	if (pEventChunk->nbEvents == 0) {
		pEventChunk->aEvents = (int*)0x0;
	}
	else {
		pEventChunk->aEvents = reinterpret_cast<int*>(pNextObj + sizeof(int));
	}

	uVar7 = 0;
	if (pEventChunk->nbEvents != 0) {
		iVar11 = 0;

		do {
			const int offset = pEventChunk->aEvents[uVar7];
			ed_event* pEvent = reinterpret_cast<ed_event*>(reinterpret_cast<char*>(pEventChunk->aEvents) + offset);
			pEventChunk->aEvents[uVar7] = STORE_SECTION(pEvent);

			if (pEvent->pZone == -1) {
				pEvent->pZone = 0;
			}
			else {
				pEvent->pZone = STORE_SECTION(pEventChunk->aZones + pEvent->pZone);
			}

			uVar14 = 0;
			_ed_event_collider_test* pEventAfter = reinterpret_cast<_ed_event_collider_test*>(pEvent + 1);
			if (pEvent->field_0xc != 0) {
				do {
					uVar10 = 0;
					pEventAfter->field_0x18 = STORE_SECTION(reinterpret_cast<char*>(pEventChunk->aActorRefs) + (pEventAfter->field_0x18 * 8));
					pEventAfter->field_0x1c = pEventChunk->aEvents[pEventAfter->field_0x1c];
					do {
						if (pEventAfter->field_0x20[uVar10] == -1) {
							pEventAfter->field_0x20[uVar10] = 0;
						}
						else {
							pEventAfter->field_0x20[uVar10] = pEventChunk->field_0x1c[pEventAfter->field_0x20[uVar10]];
						}

						uVar10 = uVar10 + 1;
					} while (uVar10 < 4);

					pEventAfter = pEventAfter + 1;
					uVar14 = uVar14 + 1;
				} while (uVar14 < pEvent->field_0xc);
			}

			uVar7 = uVar7 + 1;
			iVar11 = iVar11 + 4;
		} while (uVar7 < pEventChunk->nbEvents);
	}

	_edEventInstallGameData(pEventChunk, mode);
	_edEventInitChunk(pEventChunk);
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
