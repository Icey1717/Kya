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

#define EVENT_LOG(level, format, ...) MY_LOG_CATEGORY("event", level, format, ##__VA_ARGS__)
#define EVENT_LOG_SLOW(level, format, ...)

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

	EVENT_LOG(LogLevel::Verbose, "EventCallbackGetActorPointer index: 0x{:x} -> actor: {}", index, pActor->name);

	return pActor;
}

edF32VECTOR4* EventCallbackGetActorPositionVector(CActor* pActor) {
	EVENT_LOG(LogLevel::Verbose, "EventCallbackGetActorPositionVector actor: {}", pActor->name);
	return &pActor->currentLocation;
}

CActor* EventCallbackGetActorIdent(int index) {
	return NULL;
}

void EventCallbackSendMessage(edCEventMessage* pEventMessage)
{
	bool bVar1;
	EventSendInfo* pSendInfo;
	int iVar3;
	int iVar4;
	CActor* pActor;
	int* pCurrentActorIndex;
	int iVar6;
	int nbActorIndexes;

	pSendInfo = LOAD_SECTION_CAST(EventSendInfo*, pEventMessage->pEventCollider->aSendInfo[pEventMessage->colliderId]);

	nbActorIndexes = pSendInfo->nbActorIndexes;
	pCurrentActorIndex = reinterpret_cast<int*>(pSendInfo + 1);
	iVar6 = (pSendInfo->field_0x0 + -1) - nbActorIndexes;

	int* pReceiveData = pCurrentActorIndex + nbActorIndexes;

	if (nbActorIndexes == 0) {
		ed_event_actor_ref* pRef = LOAD_SECTION_CAST(ed_event_actor_ref*, pEventMessage->pEventCollider->pActorRef);
		CActor* pActorRef = LOAD_SECTION_CAST(CActor*, pRef->pActor);
		pActorRef->ReceiveEvent(pEventMessage, pEventMessage->colliderId, iVar6, reinterpret_cast<uint*>(pReceiveData));
	}
	else {
		while (bVar1 = nbActorIndexes != 0, nbActorIndexes = nbActorIndexes + -1, bVar1) {
			iVar3 = *pCurrentActorIndex;

			if (iVar3 == -1) {
				ed_event_actor_ref* pRef = LOAD_SECTION_CAST(ed_event_actor_ref*, pEventMessage->pEventCollider->pActorRef);
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
	
			pActor->ReceiveEvent(pEventMessage, (long)pEventMessage->colliderId, iVar6, reinterpret_cast<uint*>(pReceiveData));
			pCurrentActorIndex = pCurrentActorIndex + 1;
		}
	}
	return;
}

void EventCallbackSendAllMessages(edCEventMessage* pQueue, int count) 
{
	bool bProcessedAllEvents;
	int curIndex = count + -1;
	if (count != 0) {
		do {
			EventCallbackSendMessage(pQueue);
			pQueue = pQueue + 1;
			bProcessedAllEvents = curIndex != 0;
			curIndex = curIndex + -1;
		} while (bProcessedAllEvents);
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

int _edEventComputePrimAgainstVertex(uint prim, edF32MATRIX4* pMatrix, edF32VECTOR4* pVertex)
{
	float fVar1;
	edF32VECTOR4 local_10;

	EVENT_LOG_SLOW(LogLevel::Verbose, "_edEventComputePrimAgainstVertex prim: {} matrix: {} vertex: {}", prim, pMatrix->ToString(), pVertex->ToString());

	edF32Matrix4MulF32Vector4Hard(&local_10, pMatrix, pVertex);

	switch (prim) {
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

	EVENT_LOG_SLOW(LogLevel::Verbose, "edEventComputeZoneAgainstVertex zone sphere: {} location: {} mode: {} zone matrix: {}", pZone->boundSphere.ToString(), pLocation->ToString(), mode, pZone->pMatrix ? true : false);

	if (pZone->pMatrix != 0x0) {
		edF32Matrix4MulF32Vector4Hard(&eStack32, LOAD_SECTION_CAST(edF32MATRIX4*, pZone->pMatrix), pLocation);
		pLocation = &eStack32;
		EVENT_LOG_SLOW(LogLevel::Verbose, "edEventComputeZoneAgainstVertex updated location: {}", pLocation->ToString());
	}

	if ((pZone->boundSphere).w != 0.0f) {
		local_10 = pLocation->xyz - (pZone->boundSphere).xyz;
		fVar10 = edF32Vector3DotProductSoft(&local_10, &local_10);
		fVar9 = (pZone->boundSphere).w;

		if (fVar9 * fVar9 < fVar10) {
			EVENT_LOG(LogLevel::Verbose, "edEventComputeZoneAgainstVertex outside bound sphere");
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
						EVENT_LOG_SLOW(LogLevel::Verbose, "edEventComputeZoneAgainstVertex in event {}", *piVar7);
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

	EVENT_LOG_SLOW(LogLevel::Verbose, "edEventComputeZoneAgainstVertex not in event");
	return 2;
}

int _edEventComputeZeroVolumeZoneAgainstVertex(ed_event_chunk* pEventChunk, ed_zone_3d* pZone, edF32VECTOR4* param_3, edF32VECTOR4* param_4, uint* param_5)
{
	uint* puVar1;
	int uVar2;
	float t;
	edF32VECTOR4 local_30;
	edF32VECTOR4 local_20;
	edF32VECTOR4 local_10;

	puVar1 = LOAD_SECTION_CAST(uint*, pZone->field_0x0[0]);
	if (puVar1 == (uint*)0x0) {
		puVar1 = LOAD_SECTION_CAST(uint*, pZone->field_0x0[2]);
		local_10.x = param_4->x;
	}
	else {
		local_10.x = param_4->x;
	}

	local_10.y = param_4->y;
	local_10.z = param_4->z;
	local_10.w = param_4->w;

	edF32Matrix4MulF32Vector4Hard(&local_20, pEventChunk->aMatrices + puVar1[1], param_3);

	uVar2 = 2;
	if ((((-0.5f <= local_20.x) && (local_20.x <= 0.5f)) && (-0.5f <= local_20.z)) &&
		((local_20.z <= 0.5f && (0.0f <= local_20.y)))) {
		uVar2 = 1;
	}

	*param_4 = local_20;

	if ((uVar2 & *param_5) == 0) {
		if (((local_20.y < 0.0f) && (0.0f <= local_10.y)) || ((0.0f <= local_20.y && (local_10.y < 0.0f)))) {
			edF32Vector4SubHard(&local_30, &local_20, &local_10);
			t = 0.5f;
			if (local_30.y != 0.0f) {
				t = -local_10.y / local_30.y;
			}

			edF32Vector4ScaleHard(t, &local_30, &local_30);
			edF32Vector4AddHard(&local_30, &local_30, &local_10);

			if ((((-0.5f <= local_30.x) && (local_30.x <= 0.5f)) && (-0.5f <= local_30.z)) && (local_30.z <= 0.5f)) {
				return uVar2;
			}
		}

		*param_5 = uVar2;
	}

	return uVar2;
}

int edEventComputeZoneAgainstVertex(int index, ed_zone_3d* pZone, edF32VECTOR4* param_3, long mode)
{
	int iVar1;

	iVar1 = edEventComputeZoneAgainstVertex(pedEventChunks[index], pZone, param_3, (uint)mode);
	return iVar1;
}

int edEventGetChunkZoneNbInclusivePrimitives(int param_1, ed_zone_3d* param_2)
{
	int iVar1;

	iVar1 = 0;
	if (param_2->field_0x0[0] != 0x0) {
		iVar1 = *LOAD_SECTION_CAST(int*, param_2->field_0x0[0]);
	}
	if (param_2->field_0x0[2] != 0x0) {
		iVar1 = iVar1 + *LOAD_SECTION_CAST(int*, param_2->field_0x0[2]);
	}
	return iVar1;
}

edF32MATRIX4* edEventGetChunkZonePrimitive(uint param_1, ed_zone_3d* param_2, uint param_3, e_ed_event_prim3d_type* param_4)
{
	ed_event_chunk* peVar1;
	uint uVar2;
	uint* puVar3;

	puVar3 = LOAD_SECTION_CAST(uint*, param_2->field_0x0[0]);
	peVar1 = pedEventChunks[param_1];

	if (puVar3 == (uint*)0x0) {
		puVar3 = LOAD_SECTION_CAST(uint*, param_2->field_0x0[2]);
	}
	else {
		uVar2 = *puVar3;
		if (uVar2 <= param_3) {
			puVar3 = LOAD_SECTION_CAST(uint*, param_2->field_0x0[2]);
			param_3 = param_3 - uVar2;
		}
	}

	uVar2 = puVar3[param_3 + 1];
	*param_4 = (uint)peVar1->field_0x8[uVar2];

	return peVar1->aMatrices + uVar2;
}

void _edEventAddMessage(ed_event_chunk* pEventChunk, uint colliderId, _ed_event_collider_test* pEventCollider)
{
	edCEventMessage* pMsg;

	EVENT_LOG(LogLevel::Verbose, "_edEventAddMessage collider: {}", colliderId);

	pMsg = EventQueue_00448fbc.aEntries + EventQueue_00448fbc.inUseCount;
	pMsg->colliderId = colliderId;
	pMsg->pEventCollider = pEventCollider;
	pMsg->pEventChunk = pEventChunk;
	EventQueue_00448fbc.inUseCount = EventQueue_00448fbc.inUseCount + 1;
	return;
}

void _edEventComputeEvent(ed_event_chunk* pEventChunk, ed_event* pEvent)
{
	byte bVar1;
	ed_zone_3d* pZone;
	bool bVar3;
	int iVar5;
	uint curColliderIndex;
	_ed_event_collider_test* pCollider;
	byte* pbVar8;
	int* pSendInfo;

	EVENT_LOG_SLOW(LogLevel::Verbose, "_edEventComputeEvent flags 0x{:x} colliders: {}", pEvent->flags, pEvent->nbColliders);

	if ((pEvent->flags & 0x80) != 0) {
		bVar3 = false;
		curColliderIndex = 0;
		pCollider = reinterpret_cast<_ed_event_collider_test*>(pEvent + 1);

		if (pEvent->nbColliders != 0) {
			do {
				EVENT_LOG_SLOW(LogLevel::Verbose, "_edEventComputeEvent processing collider: {}", curColliderIndex);

				pZone = LOAD_SECTION_CAST(ed_zone_3d*, pEvent->pZone);

				EVENT_LOG_SLOW(LogLevel::Verbose, "_edEventComputeEvent processing zone flags: 0x{:x}", pZone->flags);

				uint coliderFlags;
				uint result;

				if ((pZone->flags & 1) == 0) {
					ed_event_actor_ref* pActorRef = LOAD_SECTION_CAST(ed_event_actor_ref*, pCollider->pActorRef);
					EVENT_LOG_SLOW(LogLevel::Verbose, "_edEventComputeEvent test actor: {}", LOAD_SECTION_CAST(CActor*, pActorRef->pActor)->name);
					result = edEventComputeZoneAgainstVertex(pEventChunk, pZone, LOAD_SECTION_CAST(edF32VECTOR4*, pActorRef->pLocation), 0);
					coliderFlags = pCollider->flags;
				}
				else {
					ed_event_actor_ref* pActorRef = LOAD_SECTION_CAST(ed_event_actor_ref*, pCollider->pActorRef);
					result = _edEventComputeZeroVolumeZoneAgainstVertex(pEventChunk, pZone, LOAD_SECTION_CAST(edF32VECTOR4*, pActorRef->pLocation), &pCollider->worldLocation,
						&pCollider->flags);
					coliderFlags = pCollider->flags;
				}

				if ((result & coliderFlags) == 0) {
					if ((result & 1) == 0) {
						pCollider->flags = 0xa;
					}
					else {
						pCollider->flags = 0x5;
					}
				}
				else {
					pCollider->flags = coliderFlags & 0xfffffff3;
				}

				curColliderIndex = curColliderIndex + 1;
				pCollider = pCollider + 1;
			} while (curColliderIndex < pEvent->nbColliders);
		}

		curColliderIndex = 0;
		pCollider = reinterpret_cast<_ed_event_collider_test*>(pEvent + 1);

		if (pEvent->nbColliders != 0) {
			do {
				EVENT_LOG_SLOW(LogLevel::Verbose, "_edEventComputeEvent processing collider: {}", curColliderIndex);

				uint messageIndex = 0;
				pSendInfo = pCollider->aSendInfo;

				do {
					EVENT_LOG_SLOW(LogLevel::Verbose, "_edEventComputeEvent processing zone?: {}", messageIndex);

					byte* pByte = pCollider->messageFlags + messageIndex;

					EVENT_LOG_SLOW(LogLevel::Verbose, "_edEventComputeEvent byte flags 0x{:x} data: {} collider flags: {} byte pass: {}", *pByte, *pSendInfo, pCollider->flags, (*pByte & 0x80) != 0);

					if (((*pSendInfo != 0x0) && ((*pByte & 0x80) != 0)) && ((pCollider->flags & 1 << (messageIndex & 0x1f)) != 0)) {
						_edEventAddMessage(pEventChunk, messageIndex, pCollider);
						bVar1 = *pByte;
						bVar3 = true;

						if ((bVar1 & 2) != 0) {
							*pByte = bVar1 & 0x7f;
						}
					}

					messageIndex = messageIndex + 1;
					pSendInfo = pSendInfo + 1;
				} while (messageIndex < 4);

				curColliderIndex = curColliderIndex + 1;
				pCollider = pCollider + 1;
			} while (curColliderIndex < pEvent->nbColliders);
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
	int* pEvent;
	uint uVar2;

	EVENT_LOG(LogLevel::Verbose, "edEventComputeChunk active chunk ID: {} param_2: {}", activeChunkID, param_2);

	pEventchunk = pedEventChunks[activeChunkID];

	EVENT_LOG(LogLevel::Verbose, "edEventComputeChunk count: {} events: {}", pEventchunk->count_0x38, pEventchunk->nbEvents);

	if ((pEventchunk->count_0x38 == 0) || (param_2 == false)) {
		pEvent = pEventchunk->aEvents;
		uVar2 = 0;
		if (pEventchunk->nbEvents != 0) {
			do {
				EVENT_LOG(LogLevel::Verbose, "\nedEventComputeChunk compute {}", uVar2);
				_edEventComputeEvent(pEventchunk, LOAD_SECTION_CAST(ed_event*, *pEvent));
				uVar2 = uVar2 + 1;
				pEvent = pEvent + 1;
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
		edEventClearMessageQueue();
		edEventRemoveChunk(this->activeChunkId);
		this->activeChunkId = -1;
		this->nbEvents = 0;
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
		edEventInitChunk(this->activeChunkId);
		edEventClearMessageQueue();
	}
	return;
}

void CEventManager::Level_CheckpointReset()
{
	Level_Reset();
}

char* CEventManager::ProfileGetName()
{
	return "Events ";
}

uint edEventGetChunkNbEvents(int chunkIndex)
{
	return pedEventChunks[chunkIndex]->nbEvents;
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
	ed_event* pEvent;
	ed_zone_3d* peVar2;
	ed_event_alloc_10* puVar3;
	_edCluster* pvVar4;
	float* pfVar5;
	undefined4 uVar6;
	uint uVar7;
	int iVar8;
	uint uVar9;
	_ed_event_collider_test* pCollider;
	ed_event_actor_ref* pActorRef;
	uint uVar12;
	int* ppeVar13;
	uint uVar14;
	_edCluster* ppfVar15;
	float fVar16;
	float fVar17;
	ed_zone_3d* peVar18;

	uVar9 = 0;
	pActorRef = pEventChunk->aActorRefs;
	if (pEventChunk->nbActorRefs != 0) {
		do {
			pActorRef->pLocation = 0;
			uVar9 = uVar9 + 1;
			pActorRef = pActorRef + 1;
		} while (uVar9 < pEventChunk->nbActorRefs);
	}

	ppeVar13 = pEventChunk->aEvents;
	uVar9 = 0;
	if (pEventChunk->nbEvents != 0) {
		do {
			pEvent = LOAD_SECTION_CAST(ed_event*, *ppeVar13);
			uVar12 = 0;
			pCollider = (_ed_event_collider_test*)(pEvent + 1);

			if (pEvent->nbColliders != 0) {
				do {
					uVar12 = uVar12 + 1;
					ed_event_actor_ref* pLoaded = LOAD_SECTION_CAST(ed_event_actor_ref*, pCollider->pActorRef);
					pLoaded->pLocation = pLoaded->pLocation + 1;
					pCollider = pCollider + 1;
				} while (uVar12 < pEvent->nbColliders);
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
				pActorRef = pEventChunk->aActorRefs;
				if (pEventChunk->nbActorRefs != 0) {
					do {
						if (uVar14 <= pActorRef->pLocation) {
							uVar9 = uVar7;
							uVar14 = pActorRef->pLocation;
						}
						uVar7 = uVar7 + 1;
						pActorRef = pActorRef + 1;
					} while (uVar7 < pEventChunk->nbActorRefs);
				}

				pActorRef = pEventChunk->aActorRefs + uVar9;
				pActorRef->pLocation = 0;
				puVar3->clusterCount = uVar14;
				puVar3->field_0x4 = pActorRef;
				pvVar4 = (_edCluster*)edMemAlloc(TO_HEAP(H_MAIN), puVar3->clusterCount * sizeof(_edCluster));
				puVar3->aClusters = pvVar4;

				ppfVar15 = puVar3->aClusters;
				ppeVar13 = pEventChunk->aEvents;
				uVar14 = 0;
				if (pEventChunk->nbEvents != 0) {
					do {
						pEvent = LOAD_SECTION_CAST(ed_event*, *ppeVar13);
						uVar7 = 0;
						pCollider = (_ed_event_collider_test*)(pEvent + 1);
						if (pEvent->nbColliders != 0) {
							do {
								if (LOAD_SECTION_CAST(ed_event_actor_ref*, pCollider->pActorRef) == pActorRef) {
									IMPLEMENTATION_GUARD(
									pfVar5 = edClusterInsertNode(ppfVar15, 4, (long)(int)&pEvent->field_0x4->field_0xc, pEvent);
									ppfVar15->field_0x30 = pfVar5;
									ppfVar15 = ppfVar15 + 1;)
									break;
								}

								uVar7 = uVar7 + 1;
								pCollider = pCollider + 1;
							} while (uVar7 < pEvent->nbColliders);
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

	pActorRef = pEventChunk->aActorRefs;
	uVar9 = 0;
	if (pEventChunk->nbActorRefs != 0) {
		do {
			CActor* pActor = CEventManager::callbackFunctions.pGetActorPointer(pActorRef->pActor);
			pActorRef->pActor = STORE_SECTION(pActor);
			pActorRef->pLocation = STORE_SECTION(CEventManager::callbackFunctions.pGetActorPositionVector(pActor));
			uVar9 = uVar9 + 1;
			pActorRef = pActorRef + 1;
		} while (uVar9 < pEventChunk->nbActorRefs);
	}

	ppeVar13 = pEventChunk->aEvents;
	uVar9 = 0;
	if (pEventChunk->nbEvents != 0) {
		do {
			pEvent = LOAD_SECTION_CAST(ed_event*, *ppeVar13);
			peVar2 = LOAD_SECTION_CAST(ed_zone_3d*, pEvent->pZone);
			uVar12 = 0;
			pCollider = (_ed_event_collider_test*)(pEvent + 1);

			if (pEvent->nbColliders != 0) {
				do {
					if ((peVar2->flags & 1) == 0) {
						ed_event_actor_ref* pActorRef = LOAD_SECTION_CAST(ed_event_actor_ref*, pCollider->pActorRef);
						pCollider->worldLocation = *LOAD_SECTION_CAST(edF32VECTOR4*, pActorRef->pLocation);
					}
					else {
						if (peVar2->field_0x0[0] == 0x0) {
							IMPLEMENTATION_GUARD(
							iVar8 = *(int*)(peVar2->field_0x0[2] + 4);)
						}
						else {
							iVar8 = *(int*)(LOAD_SECTION_CAST(char*, peVar2->field_0x0[0]) + 4);
						}

						edF32Matrix4MulF32Vector4Hard(&pCollider->worldLocation, pEventChunk->aMatrices + iVar8, LOAD_SECTION_CAST(edF32VECTOR4*, LOAD_SECTION_CAST(ed_event_actor_ref*, pCollider->pActorRef)->pLocation));
					}

					uVar12 = uVar12 + 1;
					pCollider = pCollider + 1;
				} while (uVar12 < pEvent->nbColliders);
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
			if (peVar2->nbColliders != 0) {
				do {
					peVar9->flags = 2;
					uVar8 = 0;
					ppuVar7 = peVar9->aSendInfo;
					do {
						if (*ppuVar7 != 0x0) {
							bVar1 = peVar9->messageFlags[uVar8];
							if ((bVar1 & 1) == 0) {
								peVar9->messageFlags[uVar8] = bVar1 & 0x7f;
							}
							else {
								peVar9->messageFlags[uVar8] = bVar1 | 0x80;
							}
						}

						uVar8 = uVar8 + 1;
						ppuVar7 = ppuVar7 + 1;
					} while (uVar8 < 4);

					uVar10 = uVar10 + 1;
					peVar9 = peVar9 + 1;
				} while (uVar10 < peVar2->nbColliders);
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

static void _edEventUninstallGameData(ed_event_chunk* pEventChunk)
{
	_edCluster* p_Var1;
	uint uVar2;
	ed_event_alloc_10* peVar3;
	uint uVar4;

	if (pEventChunk->count_0x38 != 0) {
		peVar3 = pEventChunk->field_0x3c;

		uVar4 = 0;
		if (pEventChunk->count_0x38 != 0) {
			do {
				p_Var1 = peVar3->aClusters;
				uVar2 = 0;
				if (peVar3->clusterCount != 0) {
					do {
						IMPLEMENTATION_GUARD(
						edClusterRemoveNode(p_Var1, peVar3->field_0x8, p_Var1->field_0x30);)
						uVar2 = uVar2 + 1;
						p_Var1 = p_Var1 + 1;
					} while (uVar2 < peVar3->clusterCount);
				}

				edMemFree(peVar3->aClusters);
				uVar4 = uVar4 + 1;
				peVar3 = peVar3 + 1;
			} while (uVar4 < pEventChunk->count_0x38);
		}

		edMemFree(pEventChunk->field_0x3c);
	}

	return;
}

static void _edEventUninstallChunk(ed_event_chunk* pEventChunk)
{
	_edEventUninstallGameData(pEventChunk);
	return;
}

void edEventInitChunk(int eventChunkID)
{
	_edEventInitChunk(pedEventChunks[eventChunkID]);
	return;
}

void edEventRemoveChunk(int eventChunkID)
{
	ed_event_chunk* pEventChunk = pedEventChunks[eventChunkID];
	_edEventUninstallChunk(pEventChunk);
	pedEventChunks[eventChunkID] = (ed_event_chunk*)0x0;
	edMemFree(pEventChunk);
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
	pEventChunk->nbSendInfo = *pNewObj;
	if (pEventChunk->nbSendInfo == 0) {
		pEventChunk->aSendInfo = (int*)0x0;
	}
	else {
		pEventChunk->aSendInfo = puVar8;
	}
	
	uVar7 = 0;
	iVar11 = puVar8[pEventChunk->nbSendInfo];
	if (pEventChunk->nbSendInfo != 0) {
		do {
			const int offset = pEventChunk->aSendInfo[uVar7];
			pEventChunk->aSendInfo[uVar7] = STORE_SECTION(reinterpret_cast<char*>(pEventChunk->aSendInfo) + offset);
			uVar7 = uVar7 + 1;
		} while (uVar7 < pEventChunk->nbSendInfo);
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
			_ed_event_collider_test* pCollider = reinterpret_cast<_ed_event_collider_test*>(pEvent + 1);
			if (pEvent->nbColliders != 0) {
				do {
					uVar10 = 0;
					pCollider->pActorRef = STORE_SECTION(reinterpret_cast<char*>(pEventChunk->aActorRefs) + (pCollider->pActorRef * sizeof(ed_event_actor_ref)));
					pCollider->field_0x1c = pEventChunk->aEvents[pCollider->field_0x1c];
					do {
						if (pCollider->aSendInfo[uVar10] == -1) {
							pCollider->aSendInfo[uVar10] = 0;
						}
						else {
							// Transfer this to the collider from the event chunk.
							pCollider->aSendInfo[uVar10] = pEventChunk->aSendInfo[pCollider->aSendInfo[uVar10]];
						}

						uVar10 = uVar10 + 1;
					} while (uVar10 < 4);

					pCollider = pCollider + 1;
					uVar14 = uVar14 + 1;
				} while (uVar14 < pEvent->nbColliders);
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

	pEventChunk = reinterpret_cast<ed_event_chunk*>(edMemAlloc(TO_HEAP(H_MAIN), sizeof(ed_event_chunk)));

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
