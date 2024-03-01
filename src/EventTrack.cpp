#include "EventTrack.h"
#include "MemoryStream.h"
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
	bool bVar1;
	uint uVar2;
	uint uVar3;
	ushort uVar4;
	int iVar5;
	s_track_event* pEvent;
	float fVar6;

	uVar2 = pByteCode->GetU32();
	this->eventCount = (ushort)uVar2;
	pEvent = (s_track_event*)pByteCode->GetPosition();
	this->pTrackEvent = pEvent;
	pEvent = this->pTrackEvent;
	uVar4 = 0;
	if (this->eventCount != 0) {
		do {
			fVar6 = pByteCode->GetF32();
			pEvent->field_0x0 = fVar6;
			uVar2 = pByteCode->GetU32();
			uVar3 = pByteCode->GetU32();
			pEvent->field_0x8 = uVar3;
			uVar3 = pByteCode->GetU32();
			pEvent->type = uVar3;
			if (pEvent->type == 0x39) {
				uVar3 = pByteCode->GetU32();
				pEvent->field_0x10 = uVar3;
				if (uVar2 == 4) {
					uVar2 = pByteCode->GetU32();
					pEvent->field_0x14 = uVar2;
				}
				else {
					pEvent->field_0x14 = 0;
				}
			}
			else {
				iVar5 = uVar2 - 3;
				if (uVar2 != 2) {
					do {
						pByteCode->GetU32();
						bVar1 = iVar5 != 0;
						iVar5 = iVar5 + -1;
					} while (bVar1);
				}
			}
			iVar5 = 2;
			do {
				pByteCode->GetU32();
				bVar1 = iVar5 != 0;
				iVar5 = iVar5 + -1;
			} while (bVar1);
			uVar4 = uVar4 + 1;
			pEvent = pEvent + 1;
		} while (uVar4 < this->eventCount);
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
					if (((psVar4->field_0x1c == 0x0) || (psVar4->field_0x18 == 0)) || (psVar4->field_0x18 != LOAD_SECTION_CAST(int*, psVar4->field_0x1c)[6])) {
						bVar1 = false;
					}
					else {
						bVar1 = true;
					}
					if (bVar1) {
						piVar2 = LOAD_SECTION_CAST(int*, psVar4->field_0x1c);
						if (((piVar2 != (int*)0x0) && (iVar3 = psVar4->field_0x18, iVar3 != 0)) && (iVar3 == piVar2[6])) {
							IMPLEMENTATION_GUARD(
							(**(code**)(*piVar2 + 0xc))();)
						}
						psVar4->field_0x1c = 0x0;
						psVar4->field_0x18 = 0;
					}
					psVar4->field_0x18 = 0;
					psVar4->field_0x1c = 0x0;
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

void CTrackManager::Level_ClearAll()
{
	if (this->aTracks != (CEventTrack*)0x0) {
		delete[] this->aTracks;
	}
	this->aTracks = (CEventTrack*)0x0;
	this->trackCount = 0;
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
				piVar1 = LOAD_SECTION_CAST(int*, pEvent->field_0x1c);
				if (((piVar1 == (int*)0x0) || (pEvent->field_0x18 == 0)) || (pEvent->field_0x18 != piVar1[6])) {
					lVar2 = 7;
				}
				else {
					IMPLEMENTATION_GUARD(
					lVar2 = (**(code**)(*piVar1 + 0x28))();)
				}
				if (((lVar2 != 0) && (piVar1 = LOAD_SECTION_CAST(int*, pEvent->field_0x1c), piVar1 != (int*)0x0)) &&
					((pEvent->field_0x18 != 0 && (pEvent->field_0x18 == piVar1[6])))) {
					IMPLEMENTATION_GUARD(
					(**(code**)(*piVar1 + 0x24))(&DAT_bf800000);)
				}
				pEvent->field_0x18 = 0;
				pEvent->field_0x1c = 0x0;
			}
			pEvent->field_0x20 = 0;
			pEvent = pEvent + 1;
		} while (uVar3 != 0);
	}
	return;
}

CEventTrack* CTrackManager::GetTrack(int index)
{
	CEventTrack* pTrack;

	pTrack = (CEventTrack*)0x0;
	if (index != -1) {
		pTrack = (CEventTrack*)(this->aTracks + index);
	}
	return pTrack;
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
				IMPLEMENTATION_GUARD_LOG(
				_PlayEvent(this, pEvent, pActor);)
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
			piVar1 = LOAD_SECTION_CAST(int*, pEvent->field_0x1c);
			if (((piVar1 == (int*)0x0) || (pEvent->field_0x18 == 0)) || (bVar2 = true, pEvent->field_0x18 != piVar1[6])) {
				bVar2 = false;
			}
			if (bVar2) {
				if (((piVar1 != (int*)0x0) && (pEvent->field_0x18 != 0)) && (pEvent->field_0x18 == piVar1[6])) {
					IMPLEMENTATION_GUARD(
					(**(code**)(*piVar1 + 0xc))();)
				}
				pEvent->field_0x1c = 0x0;
				pEvent->field_0x18 = 0;
			}
			pEvent->field_0x18 = 0;
			pEvent->field_0x1c = 0x0;
			pEvent->field_0x20 = 0;
			pEvent = pEvent + 1;
		} while (uVar3 != 0);
	}
	return;
}