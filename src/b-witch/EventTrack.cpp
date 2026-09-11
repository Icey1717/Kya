#include "EventTrack.h"
#include "MemoryStream.h"
#include "CollisionManager.h"
#include "port/pointer_conv.h"

bool BnkInstallTrack(char* pFileData, int length)
{
	MY_LOG("BnkInstallAnimMacro\n");

	CTrackManager* pTrackManager;
	uint trackCount;
	CEventTrack* pTrack;
	int trackIndex;
	ByteCode byteCode;

	byteCode.Init(pFileData);
	byteCode.GetChunk();
	pTrackManager = CScene::ptable.g_TrackManager_004516b4;
	trackCount = byteCode.GetU32();
	pTrackManager->trackCount = trackCount;
	trackCount = pTrackManager->trackCount;

	if (trackCount != 0) {
		pTrackManager->aTracks = new CEventTrack[trackCount];
	}

	pTrack = pTrackManager->aTracks;
	for (trackIndex = pTrackManager->trackCount; trackIndex != 0; trackIndex = trackIndex + -1) {
		pTrack->Add(&byteCode);
		pTrack = pTrack + 1;
	}
	return false;
}

void CEventTrack::Add(ByteCode* pByteCode)
{
	bool bIsLooping;
	uint uTempValue;
	uint uValue;
	ushort eventIndex;
	int iVar5;
	s_track_event* pTrackEvent;
	float eventValue;

	uTempValue = pByteCode->GetU32();
	this->eventCount = (ushort)uTempValue;
	pTrackEvent = (s_track_event*)pByteCode->GetPosition();
	this->pTrackEvent = pTrackEvent;
	pTrackEvent = this->pTrackEvent;

	eventIndex = 0;
	if (this->eventCount != 0) {
		do {
			eventValue = pByteCode->GetF32();
			pTrackEvent->field_0x0 = eventValue;
			uTempValue = pByteCode->GetU32();
			uValue = pByteCode->GetU32();
			pTrackEvent->field_0x8 = uValue;
			uValue = pByteCode->GetU32();
			pTrackEvent->type = uValue;
			if (pTrackEvent->type == 0x39) {
				uValue = pByteCode->GetU32();
				pTrackEvent->field_0x10 = uValue;
				if (uTempValue == 4) {
					uTempValue = pByteCode->GetU32();
					pTrackEvent->field_0x14 = uTempValue;
				}
				else {
					pTrackEvent->field_0x14 = 0;
				}
			}
			else {
				iVar5 = uTempValue - 3;
				if (uTempValue != 2) {
					do {
						pByteCode->GetU32();
						bIsLooping = iVar5 != 0;
						iVar5 = iVar5 + -1;
					} while (bIsLooping);
				}
			}

			iVar5 = 2;
			do {
				pByteCode->GetU32();
				bIsLooping = iVar5 != 0;
				iVar5 = iVar5 + -1;
			} while (bIsLooping);

			eventIndex = eventIndex + 1;
			pTrackEvent = pTrackEvent + 1;
		} while (eventIndex < this->eventCount);
	}

	return;
}

CTrackManager::CTrackManager()
{
	this->trackCount = 0;
	this->aTracks = (CEventTrack*)0x0;
}

void CTrackManager::Level_Init()
{
	bool bVar1;
	int* piVar2;
	int iVar3;
	s_track_event* psVar4;
	uint uVar5;
	CEventTrack* piVar5;
	int iVar7;

	iVar7 = this->trackCount + -1;
	if (this->trackCount != 0) {
		do {
			piVar5 = &this->aTracks[iVar7];
			psVar4 = piVar5->pTrackEvent;
			uVar5 = (uint)piVar5->eventCount;
			if (piVar5->eventCount != 0) {
				do {
					uVar5 = uVar5 - 1;
					psVar4->field_0x20 = 0;
					psVar4 = psVar4 + 1;
				} while (uVar5 != 0);
			}
			psVar4 = piVar5->pTrackEvent;
			uVar5 = (uint)piVar5->eventCount;
			if (piVar5->eventCount != 0) {
				do {
					uVar5 = uVar5 - 1;
					if (UNPACK_HANDLE(psVar4).IsValid()) {
						UNPACK_HANDLE(psVar4).Kill();
						UNPACK_HANDLE(psVar4).Reset();
					}

					psVar4->fxHandle.id = 0;
					psVar4->fxHandle.pFx = 0x0;
					psVar4->field_0x20 = 0;
					psVar4 = psVar4 + 1;
				} while (uVar5 != 0);
			}

			bVar1 = iVar7 != 0;
			iVar7 = iVar7 + -1;
		} while (bVar1);
	}

	return;
}

void CEventTrack::Resume()
{
	int* piVar1;
	s_track_event* psVar2;
	uint uVar3;

	psVar2 = this->pTrackEvent;
	uVar3 = (uint)this->eventCount;
	if (this->eventCount != 0) {
		do {
			uVar3 = uVar3 - 1;
			if ((((psVar2->type == 0x39) && UNPACK_HANDLE(psVar2).IsValid()))) {
				UNPACK_HANDLE(psVar2).Resume();
			}

			psVar2 = psVar2 + 1;
		} while (uVar3 != 0);
	}

	return;
}

void CEventTrack::Pause()
{
	int* piVar1;
	s_track_event* psVar2;
	uint uVar3;

	psVar2 = this->pTrackEvent;
	uVar3 = (uint)this->eventCount;
	if (this->eventCount != 0) {
		do {
			uVar3 = uVar3 - 1;
			if ((((psVar2->type == 0x39) && UNPACK_HANDLE(psVar2).IsValid()))) {
				UNPACK_HANDLE(psVar2).Pause();
			}
			psVar2 = psVar2 + 1;
		} while (uVar3 != 0);
	}
	return;
}

void CTrackManager::Level_ClearAll()
{
	if (this->aTracks != (CEventTrack*)0x0) {
		delete[] this->aTracks;
	}
	this->aTracks = (CEventTrack*)0x0;
	this->trackCount = 0;
	return;
}

bool CEventTrack::FUN_0019f140()
{
	bool bIsValid;
	s_track_event* pCurrentEvent;
	uint eventIndex;

	pCurrentEvent = this->pTrackEvent;
	eventIndex = (uint)this->eventCount;

	if (this->eventCount != 0) {
		do {
			eventIndex = eventIndex - 1;

			if (UNPACK_HANDLE(pCurrentEvent).IsValid()) {
				return true;
			}

			pCurrentEvent = pCurrentEvent + 1;
		} while (eventIndex != 0);
	}

	return false;
}

void CEventTrack::_PlayEvent(s_track_event* pEvent, CActor* pActor)
{
	CNewFx* pCVar1;
	int iVar2;
	bool bVar3;
	long lVar4;
	uint uVar5;
	FX_MATERIAL_SELECTOR selector;
	CCollision* pCol;
	CFxManager* pFxManager;

	pFxManager = CScene::ptable.g_EffectsManager_004516b8;
	if (pEvent->type == 0x39) {
		lVar4 = CScene::ptable.g_EffectsManager_004516b8->IsLooped(pEvent->field_0x10);
		if (((lVar4 != 0) && (pEvent->field_0x20 == 0)) || ((lVar4 == 0 && ((uVar5 = pEvent->field_0x8 & 1, uVar5 == 0 || ((uVar5 != 0 && (pEvent->field_0x20 == 0)))))))) {
			pCol = pActor->pCollisionData;
			selector = FX_MATERIAL_SELECTOR_NONE;
			if ((pCol != (CCollision*)0x0) && (((pCol->flags_0x4 & 2) != 0 && (selector = (FX_MATERIAL_SELECTOR)(pCol->aCollisionContact[1].materialFlags & 0xf), selector == FX_MATERIAL_SELECTOR_DEFAULT)))) {
				selector = (FX_MATERIAL_SELECTOR)CScene::_pinstance->defaultMaterialIndex;
			}
#ifdef PLATFORM_PS2
			pFxManager->GetDynamicFx(&pEvent->fxHandle, pEvent->field_0x10, selector);
#else
			CFxHandle tempHandle;
			pFxManager->GetDynamicFx(&tempHandle, pEvent->field_0x10, selector);
			pEvent->fxHandle = CFxHandlePackedHack{ tempHandle.id, STORE_POINTER(tempHandle.pFx)};
#endif

			if (UNPACK_HANDLE(pEvent).IsValid()) {
				UNPACK_HANDLE(pEvent).SpatializeOnActor(0xe, pActor, pEvent->field_0x14);
				UNPACK_HANDLE(pEvent).Start();
			}

			pEvent->field_0x20 = 1;
		}
	}
	else {
		pActor->DoMessage(pActor, (ACTOR_MESSAGE)pEvent->type, 0);
	}

	return;
}

void CEventTrack::Stop()
{
	int* piVar1;
	long lVar2;
	s_track_event* pEvent;
	uint uVar3;

	pEvent = this->pTrackEvent;
	uVar3 = (uint)this->eventCount;
	if (this->eventCount != 0) {
		do {
			uVar3 = uVar3 - 1;
			if (pEvent->type == 0x39) {
				lVar2 = UNPACK_HANDLE(pEvent).GetType();

				if (((lVar2 != 0) && UNPACK_HANDLE(pEvent).IsValid())) {
					UNPACK_HANDLE(pEvent).Stop();
				}

				UNPACK_HANDLE(pEvent).Reset();
			}

			pEvent->field_0x20 = 0;
			pEvent = pEvent + 1;
		} while (uVar3 != 0);
	}

	return;
}

CEventTrack* CTrackManager::GetTrack(int index)
{
	CEventTrack* pEventTrack;

	pEventTrack = (CEventTrack*)0x0;
	if (index != -1) {
		pEventTrack = (CEventTrack*)(this->aTracks + index);
	}
	return pEventTrack;
}

void CEventTrack::Play(float param_1, float param_2, undefined8 param_4, CActor* pActor)
{
	uint uVar1;
	s_track_event* pEvent;
	float fVar2;

	uVar1 = 0;
	pEvent = this->pTrackEvent;
	if (this->eventCount != 0) {
		do {
			fVar2 = pEvent->field_0x0;
			if ((((param_2 <= fVar2) && (fVar2 < param_1)) ||
				((param_1 < param_2 && ((param_2 <= fVar2 || (fVar2 < param_1)))))) ||
				((param_2 == param_1 && (fVar2 == param_1)))) {
				_PlayEvent(pEvent, pActor);
			}
			uVar1 = uVar1 + 1;
			pEvent = pEvent + 1;
		} while (uVar1 < this->eventCount);
	}

	return;
}

void CEventTrack::Reset()
{
	int* piVar1;
	bool bVar2;
	s_track_event* pEvent;
	uint uVar3;

	pEvent = this->pTrackEvent;
	uVar3 = (uint)this->eventCount;
	if (this->eventCount != 0) {
		do {
			uVar3 = uVar3 - 1;

			if (UNPACK_HANDLE(pEvent).IsValid()) {
				UNPACK_HANDLE(pEvent).Kill();
				UNPACK_HANDLE(pEvent).Reset();
			}
			pEvent->fxHandle.id = 0;
			pEvent->fxHandle.pFx = 0x0;
			pEvent->field_0x20 = 0;
			pEvent = pEvent + 1;
		} while (uVar3 != 0);
	}

	return;
}