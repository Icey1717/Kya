#include "edSound/edSoundPlay.h"
#include "edSound/edSoundInit.h"
#include "edSound/edSoundInstance.h"
#include "edSound/ps2/_edSoundAcoustic3D.h"
#include "edSound/ps2/_edSoundPlay.h"
#include "MathOps.h"
#ifdef PLATFORM_WIN
#include "edSoundStreamService.h"
#include "edSoundSampleService.h"
#endif

static int gNbSoundStreams = 0;

int edSoundStreamLoadA(_ed_sound_stream* pSoundStream, GlobalSound_FileData* pSoundData, char* szPath, undefined8 param_4)
{
	int iVar1;

	iVar1 = _edSoundStreamInit(pSoundData, pSoundStream, szPath, 0, param_4, 0);
	gNbSoundStreams = gNbSoundStreams + 1;
	return iVar1;
}

int edSoundStreamLoadB(_ed_sound_stream* pSoundStream, GlobalSound_FileData* pSoundData, uint lsn, undefined8 param_4, uint filesize)
{
	int iVar1;

	iVar1 = _edSoundStreamInit(pSoundData, pSoundStream, (char*)0x0, lsn, param_4, filesize);
	gNbSoundStreams = gNbSoundStreams + 1;
	return iVar1;
}

void edSoundStreamFree(_ed_sound_stream* pSoundStream)
{
	_edSoundStreamTerm(pSoundStream);
	gNbSoundStreams = gNbSoundStreams + -1;

	return;
}

/*
 * Port-only flush seam. On PS2 this waits for the previous EE-to-IOP RPC; on
 * Windows it will drain finished voice IDs from the host audio backend.
 */
static void EdSoundPrepareFlush()
{
#ifdef PLATFORM_PS2
	ed_sound_instance* currentSoundInstance;

	_edSoundPrepareFlush();

	ed_sound_instance* highestPrioritySound;
	while (currentSoundInstance = highestPrioritySound, currentSoundInstance != (ed_sound_instance*)0x0) {
		highestPrioritySound = currentSoundInstance->lowerPrioritySoundInstance;
		if (((currentSoundInstance->flags & 0x400) != 0) && ((currentSoundInstance->flags & 0x80) == 0)) {
			_edSoundInstanceCheckFinished(currentSoundInstance);
		}
	}
#elif defined(PLATFORM_WIN)
	for (uint instanceId : Audio::PollFinishedSamples()) {
		if ((instanceId & 0xffff) < edSoundMaxInstances && edSoundInstanceIsAlive(instanceId)) {
			edSoundInstanceFinish(pedSoundInstances + (instanceId & 0xffff), 1);
		}
	}
#endif
}

uint* _pedSoundInstanceCommandsCount;
void* gSoundFlushCurrent_0044915c;
uint _edSoundInstanceCommandsCount;

void _edSoundCheckForInstancesToCreate(void)
{
	uint uVar1;
	ed_sound_instance* pCurListEntry;
	long lVar3;
	uint uVar4;
	ed_sound_instance* pCurInstance;
	uint uVar6;
	_ed_sound_bit_array_handle soundBitArrayHandle;
	uint local_c;
	uint soundInstanceId;
	uint local_4;

	pCurInstance = (ed_sound_instance*)0x0;
	uVar4 = (edSoundParam.field_0x4 - edSoundParam.nbVoices) + 1;
	pCurListEntry = pedSoundInstanceListHead;
	if (pedSoundInstanceListHead != (ed_sound_instance*)0x0) {
		do {
			uVar6 = 2;
			if ((pCurListEntry->flags & 0x28) == 0) {
				uVar6 = 1;
			}
			if (uVar4 < uVar6) {
				pCurListEntry->flags = pCurListEntry->flags | 0x800;
				if (pCurInstance == (ed_sound_instance*)0x0) {
					pCurInstance = pCurListEntry;
				}
			}
			else {
				uVar4 = uVar4 - uVar6;
			}

			pCurListEntry = pCurListEntry->lowerPrioritySoundInstance;
		} while ((uVar4 != 0) && (pCurListEntry != (ed_sound_instance*)0x0));
	}

	if ((pCurListEntry != (ed_sound_instance*)0x0) || (pCurInstance != (ed_sound_instance*)0x0)) {
		if (pCurListEntry != (ed_sound_instance*)0x0) {
			edSoundInstanceDeleteAllFromCurrentLessPrioritary(pCurListEntry);
		}

		for (; pCurInstance != (ed_sound_instance*)0x0; pCurInstance = pCurInstance->lowerPrioritySoundInstance) {
			if ((pCurInstance->flags & 0x800) != 0) {
				edSoundInstanceDelete(pCurInstance);
			}
		}
	}

	soundBitArrayHandle.field_0x8 = 0;
	uVar4 = _edSoundVoiceGetFirstFree(&soundBitArrayHandle);
	lVar3 = (long)(int)uVar4;
	pCurListEntry = pedSoundInstanceListHead;
	do {
		if (pCurListEntry == (ed_sound_instance*)0x0) {
			return;
		}
		if ((pCurListEntry->flags & 0x400) == 0) {
			local_4 = pCurListEntry->fullSoundInstanceId;
			uVar4 = 2;
			edSoundInstanceCom[local_4 & 0xffff].flags = edSoundInstanceCom[local_4 & 0xffff].flags | 0x800;
			edSoundInstanceCom[local_4 & 0xffff].soundInstanceId = local_4;
			pCurListEntry->flags = pCurListEntry->flags | 0x400;
			if ((pCurListEntry->flags & 0x28) == 0) {
				uVar4 = 1;
			}
			uVar6 = 0;
			uint* pVoiceIndices = pCurListEntry->voiceIndices;
			uint* pOther = pCurListEntry->field_0x80;
			if (uVar4 != 0) {
				do {
					if (soundBitArrayHandle.field_0x8 == 0) {
						soundBitArrayHandle.voiceIndex = (uint)lVar3 >> 5;
						soundBitArrayHandle.field_0x8 = 1;
						soundBitArrayHandle.field_0x4 = (uint)lVar3 + soundBitArrayHandle.voiceIndex * -0x20;
					}
					_edSoundAllocatedVoices[soundBitArrayHandle.voiceIndex] = _edSoundAllocatedVoices[soundBitArrayHandle.voiceIndex] | 1 << (soundBitArrayHandle.field_0x4 & 0x1fU);
					*pVoiceIndices = (uint)lVar3;
					*pOther = 0xfffffffe;
					soundBitArrayHandle.field_0x4 = soundBitArrayHandle.field_0x4 + 1;
					uVar1 = ~- (1 << (soundBitArrayHandle.field_0x4 & 0x1fU)) | _edSoundAllocatedVoices[soundBitArrayHandle.voiceIndex];
					if (uVar1 == 0xffffffff) {
						soundBitArrayHandle.voiceIndex = soundBitArrayHandle.voiceIndex + 1;
						uVar1 = _edSoundVoiceGetFirstFreeFromPointer(_edSoundAllocatedVoices + soundBitArrayHandle.voiceIndex, &soundBitArrayHandle);
						lVar3 = (long)(int)uVar1;
					}
					else {
						lVar3 = (long)(soundBitArrayHandle.field_0x4 + soundBitArrayHandle.voiceIndex * 0x20);
						uVar1 = uVar1 >> (soundBitArrayHandle.field_0x4 & 0x1fU);
						do {
							if ((uVar1 & 1) == 0) goto LAB_00288160;
							lVar3 = (long)((int)lVar3 + 1);
							uVar1 = uVar1 >> 1;
							soundBitArrayHandle.field_0x4 = soundBitArrayHandle.field_0x4 + 1;
						} while (lVar3 < 0x30);
						lVar3 = -1;
					}
				LAB_00288160:
					uVar6 = uVar6 + 1;
					pVoiceIndices = pVoiceIndices + 1;
					pOther = pOther + 1;
				} while (uVar6 < uVar4);
			}
		}

		if ((pCurListEntry->flags & 0x100) != 0) {
			pCurListEntry->flags = pCurListEntry->flags & 0xfffffeff;
			pCurListEntry->flags = pCurListEntry->flags & 0xffffff7f;
			soundInstanceId = pCurListEntry->fullSoundInstanceId;
			edSoundInstanceCom[soundInstanceId & 0xffff].flags = edSoundInstanceCom[soundInstanceId & 0xffff].flags & 0xfffffffb;
			edSoundInstanceCom[soundInstanceId & 0xffff].soundInstanceId = soundInstanceId;
			local_c = pCurListEntry->fullSoundInstanceId;
			edSoundInstanceCom[local_c & 0xffff].flags = edSoundInstanceCom[local_c & 0xffff].flags | 2;
			edSoundInstanceCom[local_c & 0xffff].soundInstanceId = local_c;
		}

		pCurListEntry = pCurListEntry->lowerPrioritySoundInstance;
	} while (true);
}

void _edSoundPrepareReverbTypes(uint nbTypes, ED_SOUND_REVERB_TYPE* aTypes)
{
	int iVar9;
	uint uVar11;

	uVar11 = 0x8000;
	if (nbTypes == 0) {
		uVar11 = 0x8001;
	}
	else {
		for (uint i = 0; i < nbTypes; i = i + 1) {
			uVar11 = uVar11 | 1 << (static_cast<uint>(aTypes[i]) & 0x1f);
		}
	}
	IMPLEMENTATION_GUARD_PS2(
	iVar9 = static_cast<int>(edComBuffer) + (int)*edComBuffer;
	*static_cast<undefined4*>(iVar9 + 4) = 6;
	gSoundFlushCurrent_0044915c = static_cast<SoundFlush_0x8*>(iVar9 + 8);
	gSoundFlushCurrent_0044915c->pToDelete = uVar11;
	*edComBuffer = static_cast<SoundFlush_0x8*>((int)gSoundFlushCurrent_0044915c - (int)edComBuffer);)

	return;
}

void edSoundPrepareReverbTypes(uint nbTypes, ED_SOUND_REVERB_TYPE* aTypes)
{
	_edSoundPrepareReverbTypes(nbTypes, aTypes);

	return;
};

void edSoundFlush()
{
	//SoundFlushCommand* iVar1;
	uint* pToDelete;
	uint nbFlush = 0;
	uint uVar4;

	edSoundNbFinishedInstances = 0;

	EdSoundPrepareFlush();

	for (ed_sound_instance* pSoundInstance = pedSoundInstanceListHead; pSoundInstance != (ed_sound_instance*)0x0;) {
		ed_sound_instance* pNextSoundInstance = pSoundInstance->lowerPrioritySoundInstance;
		if (((pSoundInstance->flags & 0x400) != 0) && ((pSoundInstance->flags & 0x80) == 0)) {
			_edSoundInstanceCheckFinished(pSoundInstance);
		}
		pSoundInstance = pNextSoundInstance;
	}

	edSoundInstancesComputeFade(pedSoundInstanceListHead);
	_edSoundCheckForInstancesToCreate();
	edSoundAcousticCompute(pedSoundInstanceListHead);

#ifdef PLATFORM_PS2
	iVar1 = (SoundFlushCommand*)((int)edComBuffer + *edComBuffer);
	iVar1->field_0x4 = 0;
	_pedSoundInstanceCommandsCount = &iVar1->nbCommands;
	_edSoundInstanceCommandsCount = 0;
	gSoundFlushCurrent_0044915c = iVar1->aSoundFlushes;
#endif

#ifdef PLATFORM_PS2
	pToDelete = pedSoundInstancesToDelete;
	while (edSoundInstancesToDeleteNb != 0) {
		nbFlush = nbFlush + 1;
		edSoundInstancesToDeleteNb = edSoundInstancesToDeleteNb + -1;
		gSoundFlushCurrent_0044915c->pToDelete = *pToDelete;
		pToDelete = pToDelete + 1;
		gSoundFlushCurrent_0044915c->flags = 0x1000;
		_edSoundInstanceCommandsCount = _edSoundInstanceCommandsCount + 1;
		gSoundFlushCurrent_0044915c = gSoundFlushCurrent_0044915c + 1;
	}
#else
	// Windows destroys voices synchronously in _edSoundInstanceSetFree, before
	// their instance slots can be reused. No bounded PS2 deletion queue is needed.
	edSoundInstancesToDeleteNb = 0;
#endif

#ifdef PLATFORM_PS2
	uVar4 = 0;
	local_4 = gSoundFlushCurrent_0044915c;
	if (edSoundMaxInstances != 0) {
		iVar3 = 0;
		iVar2 = 0;
		do {
			uVar1 = *(uint*)((int)&edSoundInstanceCom->flags + iVar3);
			if (uVar1 != 0) {
				nbFlush = nbFlush + 1;
				_edSoundExecuteCommands((ed_sound_instance*)((int)&pedSoundInstances->soundInstanceId + iVar2), uVar1, &local_4);
				*(undefined4*)((int)&edSoundInstanceCom->flags + iVar3) = 0;
				*(undefined4*)((int)&edSoundInstanceCom->soundInstanceId + iVar3) = 0;
			}
			uVar4 = uVar4 + 1;
			iVar3 = iVar3 + 8;
			iVar2 = iVar2 + 0xa4;
		} while (uVar4 < edSoundMaxInstances);
	}
#else
	for (uint index = 0; index < edSoundMaxInstances; ++index) {
		ed_sound_instance* pInstance = pedSoundInstances + index;
		const uint mask = edSoundInstanceCom[index].flags;
		const uint instanceId = edSoundInstanceCom[index].soundInstanceId;
		edSoundInstanceCom[index].flags = 0;
		edSoundInstanceCom[index].soundInstanceId = 0;
		if (mask == 0 || instanceId == 0 || pInstance->fullSoundInstanceId != instanceId) continue;
		// Streams already start through edSoundStream_00283650. Do not create a
		// sample voice for their differently laid-out pSoundStream union member.
		if ((pInstance->flags & 0x10) != 0) {
			if ((mask & (4 | 8)) != 0) Audio::StopStream(pInstance->pSoundStream->streamBufferId[0]);
			else if ((mask & 0x10) != 0) Audio::StartStream(pInstance->pSoundStream->streamBufferId[0]);
			continue;
		}
		Audio::SampleDescription sample;
		if (pInstance->pSample != (ed_sound_sample*)0x0) {
			sample = {pInstance->pSample->soundRamAddress, pInstance->pSample->sampleRate,
				pInstance->pSample->dataSize, pInstance->pSample->loopStartOffset,
				pInstance->pSample->loopEndOffset, (pInstance->pSample->flags & 1) != 0};
		}
		const Audio::SampleControls controls = {
			(pInstance->field_0x88 + ((pInstance->flags & 0x28) != 0 ? pInstance->field_0x94 : 0.0f)) * edSoundGlobalParams.volume,
			(pInstance->field_0x8c + ((pInstance->flags & 0x28) != 0 ? pInstance->field_0x98 : 0.0f)) * edSoundGlobalParams.volume,
			pInstance->field_0x50};
		if ((mask & 4) != 0) {
			Audio::QueueSampleCommand({Audio::SampleCommandType::Stop, instanceId});
			continue;
		}
		// Recreate on start as well: retained (flags & 1) instances may have
		// finished naturally and released their previous host voice.
		Audio::QueueSampleCommand({(mask & (0x800 | 2)) != 0 ? Audio::SampleCommandType::Create : Audio::SampleCommandType::Update,
			instanceId, sample, controls});
		if ((mask & 2) != 0 && (mask & 8) == 0) Audio::QueueSampleCommand({Audio::SampleCommandType::Start, instanceId});
		if ((mask & 8) != 0) {
			pInstance->flags |= 0x200;
			Audio::QueueSampleCommand({Audio::SampleCommandType::Pause, instanceId});
		}
		else if ((mask & 0x10) != 0) {
			pInstance->flags &= ~0x200u;
			Audio::QueueSampleCommand({Audio::SampleCommandType::Resume, instanceId});
		}
		++nbFlush;
	}
#endif

	if (edSoundGlobalParams.finishedInstancesCallback != (edSoundFinishedInstancesCallback)0x0) {
		edSoundGlobalParams.finishedInstancesCallback(pedSoundFinishedInstances, edSoundNbFinishedInstances);
	}

	_edSoundEndFlush(nbFlush);
}

void edSoundTerminateAllInstances(void)
{
	bool bRemainingInstances;
	uint soundInstanceId;
	uint soundInstanceIndex;
	ed_sound_instance* peVar3;
	ed_sound_instance* pInstance;

	bRemainingInstances = pedSoundInstanceListHead != (ed_sound_instance*)0x0;
	peVar3 = pedSoundInstanceListHead;
	while (bRemainingInstances) {
		soundInstanceId = peVar3->fullSoundInstanceId;
		peVar3 = peVar3->lowerPrioritySoundInstance;
		if (soundInstanceId != 0) {
			soundInstanceIndex = soundInstanceId & 0xffff;
			pInstance = &pedSoundInstances[soundInstanceIndex];
			if (pInstance->fullSoundInstanceId == soundInstanceId) {
				edSoundInstanceCom[soundInstanceIndex].flags = edSoundInstanceCom[soundInstanceIndex].flags & 0xfffffffd;
				edSoundInstanceCom[soundInstanceIndex].soundInstanceId = soundInstanceId;
				edSoundInstanceCom[soundInstanceIndex].flags = edSoundInstanceCom[soundInstanceIndex].flags | 4;
				edSoundInstanceCom[soundInstanceIndex].soundInstanceId = soundInstanceId;
				edSoundInstanceFinish(pInstance, 0);
			}

			pInstance = &pedSoundInstances[soundInstanceIndex];
			if (pInstance->fullSoundInstanceId == soundInstanceId) {
				pInstance->flags = pInstance->flags & 0xfffffffe;
				if ((pInstance->flags & 0x80) == 0) {
					_edSoundInstanceListInstanceRemove(pInstance);
					_edSoundInstanceListInstanceInsert(pInstance->priority, pInstance, 0);
				}
				else {
					edSoundInstanceDelete(pInstance);
				}
			}
		}

		bRemainingInstances = peVar3 != (ed_sound_instance*)0x0;
	}

	return;
}

uint edSoundInstanceStop(uint instanceId)
{
	bool bVar1;
	ed_sound_instance* pInstance;

	if ((instanceId != 0) && (pInstance = pedSoundInstances + (instanceId & 0xffff), pInstance->fullSoundInstanceId == instanceId)) {
		edSoundInstanceCom[instanceId & 0xffff].flags = edSoundInstanceCom[instanceId & 0xffff].flags & 0xfffffffd;
		edSoundInstanceCom[instanceId & 0xffff].soundInstanceId = instanceId;
		edSoundInstanceCom[instanceId & 0xffff].flags = edSoundInstanceCom[instanceId & 0xffff].flags | 4;
		edSoundInstanceCom[instanceId & 0xffff].soundInstanceId = instanceId;
		bVar1 = edSoundInstanceFinish(pInstance, 0);
		if (bVar1 == false) {
			return instanceId;
		}
	}

	return 0;
}

void edSoundInstanceFade(float volume, float frequency, float targetVolume, float targetFrequency, float duration, uint soundId)
{
	ed_sound_instance* pInstance;

	if (soundId == 0) {
		return;
	}

	pInstance = &pedSoundInstances[soundId & 0xffff];
	if (pInstance->fullSoundInstanceId != soundId) {
		return;
	}

	pInstance->flags |= 0x2000;

	if (volume != -1.0f) {
		pInstance->volume = volume;
	}

	if (frequency != -1.0f) {
		pInstance->frequency = frequency;
	}

	if (targetVolume == -1.0f) {
		targetVolume = pInstance->volume;
	}

	if (targetFrequency == -1.0f) {
		targetFrequency = pInstance->frequency;
	}

	pInstance->targetVolume = targetVolume;
	pInstance->targetFrequency = targetFrequency;
	pInstance->duration = duration;
}

void edSoundInstanceFadeTypeSet(uint soundId, uint fadeType)
{
	if (soundId != 0 &&
		pedSoundInstances[soundId & 0xffff].fullSoundInstanceId == soundId) {
		pedSoundInstances[soundId & 0xffff].fadeType = fadeType;
	}
}

int _NbLoadedSamples = 0;

int edSoundSampleLoad(char* pSoundFile, ed_sound_sample* pSoundSample, ulong flags)
{
	int transferIndex;

	_NbLoadedSamples = _NbLoadedSamples + 1;
	transferIndex = _edSoundSampleLoad((SoundFileData*)pSoundFile, pSoundSample, flags);
	return transferIndex;
}

void edSoundWaitAllSoundDataLoaded()
{
	_edSoundWaitAllSoundDataLoaded();

	return;
}

void edSoundSampleLoadWait(char* pSoundFile, ed_sound_sample* pSoundSample, ulong flags)
{
	edSoundSampleLoad(pSoundFile, pSoundSample, flags);
	edSoundWaitAllSoundDataLoaded();

	return;
}

edCSoundGlobalParams edSoundGlobalParams;

void edSoundSetMasterVolume(float newVolume)
{
	//int iVar1;

	//iVar1 = (int)edComBuffer + (int)*edComBuffer;
	edSoundGlobalParams.volume = newVolume;

	IMPLEMENTATION_GUARD_PS2(
	*(undefined4*)(iVar1 + 4) = 2;
	DAT_0044915c = (float*)(iVar1 + 8);
	*DAT_0044915c = newVolume;
	*edComBuffer = (float*)((int)DAT_0044915c - (int)edComBuffer);)

	return;
}

AUDIO_MODE edSoundOutputModeGet(void)
{
	return edSoundGlobalParams.outputMode;
}

void edSoundOutputModeSet(AUDIO_MODE newMode)
{
	edSoundGlobalParams.outputMode = newMode;
	return;
}

edCSoundParam edSoundParam;

int _edSoundSetVoiceRange(uint nbVoices, uint param_2)
{
	ed_sound_instance* pInstance;
	int iVar1;
	ed_sound_instance* peVar2;
	uint uVar3;
	uint uVar4;
	uint uVar5;
	uint uVar6;
	uint uVar7;
	uint uVar8;

	edSoundFlush();

	pInstance = pedSoundInstanceListHead;
	do {
		if (pInstance == (ed_sound_instance*)0x0) {
			uVar5 = 0;
			if (nbVoices != 0) {
				if (8 < nbVoices) {
					do {
						uVar7 = uVar5 + 4;
						EdSoundVoiceSetAllocated(uVar5);
						uVar3 = uVar5 + 3;
						uVar8 = uVar5 + 6;
						EdSoundVoiceSetAllocated(uVar5 + 1);
						uVar6 = uVar5 + 5;
						EdSoundVoiceSetAllocated(uVar5 + 2);
						uVar4 = uVar5 + 7;
						uVar5 = uVar5 + 8;
						EdSoundVoiceSetAllocated(uVar3);
						EdSoundVoiceSetAllocated(uVar7);
						EdSoundVoiceSetAllocated(uVar6);
						EdSoundVoiceSetAllocated(uVar8);
						EdSoundVoiceSetAllocated(uVar4);
					} while (uVar5 < nbVoices - 8);
				}

				for (; uVar5 < nbVoices; uVar5 = uVar5 + 1) {
					EdSoundVoiceSetAllocated(uVar5);
				}
			}

			for (uVar5 = param_2 + 1; (int)uVar5 < 0x30; uVar5 = uVar5 + 1) {
				EdSoundVoiceSetAllocated(uVar5);
			}

			uVar5 = edSoundParam.nbVoices - 1;
			if ((int)param_2 <= (int)(edSoundParam.nbVoices - 1)) {
				uVar5 = param_2;
			}

			uVar3 = nbVoices;
			if ((int)nbVoices <= (int)uVar5) {
				do {
					uVar4 = uVar3 + 1;
					EdSoundVoiceSetFree(uVar3);
					uVar3 = uVar4;
				} while ((int)uVar4 <= (int)uVar5);
			}

			uVar5 = edSoundParam.field_0x4 + 1;
			if ((int)(edSoundParam.field_0x4 + 1) <= (int)nbVoices) {
				uVar5 = nbVoices;
			}

			uVar3 = param_2;
			if ((int)uVar5 <= (int)param_2) {
				do {
					uVar4 = uVar3 - 1;
					EdSoundVoiceSetFree(uVar3);
					uVar3 = uVar4;
				} while ((int)uVar5 <= (int)uVar4);
			}

			// SETS A NEW NUMBER OF VOICE COMMANDS WE PROBABLY NEED THIS
			//iVar1 = (int)edComBuffer + (int)*edComBuffer;
			//*(undefined4*)(iVar1 + 4) = 1;
			//*(uint*)(iVar1 + 8) = nbVoices;
			//gSoundFlushCurrent_0044915c = (SoundFlush_0x8*)(iVar1 + 0xc);
			//gSoundFlushCurrent_0044915c->pToDelete = param_2;
			//*edComBuffer = (SoundFlush_0x8*)((int)gSoundFlushCurrent_0044915c - (int)edComBuffer);
			iVar1 = SOUND_SetMaxStreamLimit(param_2 - nbVoices);
			edSoundFlush();
			edSoundParam.nbVoices = nbVoices;
			edSoundParam.field_0x4 = param_2;
			return iVar1;
		}

		uVar5 = EdSoundVoiceCountFromFlags(pInstance->flags);

		uVar3 = 0;
		uint* pVoiceIndices = pInstance->voiceIndices;
		if (uVar5 != 0) {
			do {
				if ((*pVoiceIndices < nbVoices) || (param_2 < *pVoiceIndices)) break;
				uVar3 = uVar3 + 1;
				pVoiceIndices = pVoiceIndices + 1;
			} while (uVar3 < uVar5);
		}

		if (uVar3 != uVar5) {
			edSoundInstanceDelete(pInstance);
		}

		pInstance = pInstance->lowerPrioritySoundInstance;
	} while (true);
}

void edSoundSetFinishedInstancesCallback(edSoundFinishedInstancesCallback pCallback)
{
	edSoundGlobalParams.finishedInstancesCallback = pCallback;
	return;
}

struct _edCSoundTrackMemory
{
	ed_sound_sample* pSoundSample;
	uint soundRamAddress;
};

_edCSoundTrackMemory _SoundMemTrack[512];

int edSoundSampleFree(ed_sound_sample* pSoundSample)
{
	bool bVar1;
	_edCSoundTrackMemory* p_Var2;
	int iVar3;

	iVar3 = 0x1ff;
	bVar1 = false;
	p_Var2 = _SoundMemTrack + 0x1ff;
	while (((p_Var2->pSoundSample == (ed_sound_sample*)0x0 || (p_Var2->pSoundSample != pSoundSample)) || (p_Var2->soundRamAddress != pSoundSample->soundRamAddress))) {
		iVar3 = iVar3 + -1;
		p_Var2 = p_Var2 + -1;
		if (iVar3 < 0) {
		LAB_002834b0:
			if (bVar1) {
				_SoundMemTrack[iVar3].pSoundSample = (ed_sound_sample*)0x0;
				_SoundMemTrack[iVar3].soundRamAddress = 0;
			}

			_NbLoadedSamples = _NbLoadedSamples + -1;
			iVar3 = _edSoundSampleFree(pSoundSample);
			pSoundSample->flags = 0;
			return iVar3;
		}
	}

	bVar1 = true;

	goto LAB_002834b0;
}

uint _edSoundLastTransferIndex;

extern bool _edSoundAreAllSoundDataLoaded(uint lastIndex);

bool edSoundAreAllSoundDataLoaded()
{
	return _edSoundAreAllSoundDataLoaded(_edSoundLastTransferIndex);
}

edsound_listener edSoundListenerDefault;

void edSoundSetListener(edsound_listener* pListener)
{
	if (pListener == (edsound_listener*)0x0) {
		edSoundGlobalParams.field_0x24 = edSoundListenerDefault.field_0x0;
		edSoundGlobalParams.field_0x30 = edSoundListenerDefault.field_0xc;
		edSoundGlobalParams.field_0x3c = edSoundListenerDefault.field_0x18;
		edSoundGlobalParams.field_0x48 = edSoundListenerDefault.field_0x24;
	}
	else {
		edSoundGlobalParams.field_0x24 = (pListener->field_0x0);
		edSoundGlobalParams.field_0x30 = (pListener->field_0xc);
		edSoundGlobalParams.field_0x3c = (pListener->field_0x18);
		edSoundGlobalParams.field_0x48 = (pListener->field_0x24);

		if ((((pListener->field_0xc).x == 0.0f) && ((pListener->field_0xc).y == 0.0f)) && ((pListener->field_0xc).z == 0.0f)) {
			edSoundGlobalParams.field_0x30.y = gF32Vector3Zero.y;
			edSoundGlobalParams.field_0x30.z = gF32Vector3Zero.z;
			edSoundGlobalParams.field_0x30.x = 1.0f;
		}

		if ((((pListener->field_0x18).x == 0.0f) && ((pListener->field_0x18).y == 0.0f)) && ((pListener->field_0x18).z == 0.0f)) {
			edSoundGlobalParams.field_0x3c.x = gF32Vector3Zero.x;
			edSoundGlobalParams.field_0x3c.z = gF32Vector3Zero.z;
			edSoundGlobalParams.field_0x3c.y = 1.0f;
		}
	}

	return;
}

bool FUN_00289cf0(ed_sound_instance* param_1, _ed_sound_stream* param_2)
{
	if ((param_1->flags & 0x20) != 0) {
		param_1->field_0x88 = 1.0f;
		param_1->field_0x8c = 0.0f;
		param_1->field_0x94 = 0.0f;
		param_1->field_0x98 = 1.0f;
	}

	return param_2->pMem != (void*)0x0;
}

uint edSoundStreamCreate_00284500(float priority, _ed_sound_stream* pSoundStream)
{
	bool bVar1;
	ed_sound_instance* pInstance;
	uint uVar2;

	if (pSoundStream->field_0x14 == 0) {
		uVar2 = 0;
	}
	else {
		pInstance = edSoundInstanceCreate(priority, 0);
		if (pInstance == (ed_sound_instance*)0x0) {
			uVar2 = 0;
		}
		else {
			pInstance->pSoundStream = pSoundStream;
		#ifdef PLATFORM_WIN
			Audio::PrepareStream(static_cast<uint>(pSoundStream->streamBufferId[0]));
		#endif
			if (pInstance->pSoundStream->field_0x14 == 2) {
				pInstance->flags = pInstance->flags | 0x20;
			}

			bVar1 = FUN_00289cf0(pInstance, pSoundStream);
			if (bVar1 == false) {
				edSoundInstanceDelete(pInstance);
				uVar2 = 0;
			}
			else {
				pInstance->flags = pInstance->flags | 0x10;
				uVar2 = pInstance->fullSoundInstanceId;
			}
		}
	}

	if (uVar2 != 0 && pedSoundInstances[uVar2 & 0xffff].fullSoundInstanceId == uVar2) {
		edSoundInstanceCom[uVar2 & 0xffff].flags |= 1;
		edSoundInstanceCom[uVar2 & 0xffff].soundInstanceId = uVar2;
	}

	return uVar2;
}

bool Getb18FromStreamInfo(ed_sound_instance* pSoundInstance)
{
	SOUND_StreamInfo streamInfo;

	SOUND_GetStreamInfo(pSoundInstance->pSoundStream->streamBufferId[0], &streamInfo);
	return streamInfo.b18 != 0;
}

bool edSoundStream_00284650(uint index)
{
	bool bVar1;

	if (index != 0 && pedSoundInstances[index & 0xffff].fullSoundInstanceId == index) {
		bVar1 = Getb18FromStreamInfo(pedSoundInstances + (index & 0xffff));
		return bVar1;
	}
	return true;
}

void edSoundStream_00283650(uint index)
{
	ed_sound_instance* puVar1;

	if (index != 0) {
		puVar1 = pedSoundInstances + (index & 0xffff);
		if (puVar1->fullSoundInstanceId == index) {
			puVar1->flags = puVar1->flags | 0x100;
		#ifdef PLATFORM_WIN
			Audio::StartStream(static_cast<uint>(puVar1->pSoundStream->streamBufferId[0]));
		#endif
		}
	}

	return;
}

uint edSoundStream_00283f70(uint index)
{
	uint uVar1;
	uint uVar2;

	uVar2 = 0;

	if ((index != 0) && (pedSoundInstances[index & 0xffff].fullSoundInstanceId == index)) {
		uVar1 = pedSoundInstances[index & 0xffff].flags;
		uVar2 = 2;
		if (((uVar1 & 0x200) == 0) && (uVar2 = 3, (uVar1 & 0x80) == 0)) {
			uVar2 = 1;
		}
	}

	return uVar2;
}

float edSoundInstanceGetPlaybackTime(ed_sound_instance* pSoundInstance)
{
	_ed_sound_stream* p_Var1;
	bool bVar2;
	uint uVar3;
	float fVar4;
	float puVar5;
	SOUND_StreamInfo streamInfo;

	if ((pSoundInstance->flags & 0x10) == 0) {
		if (((pSoundInstance->pSample->flags & 2U) == 0) && (uVar3 = pSoundInstance->field_0x80[0], uVar3 != 0xffffffff)) {
			fVar4 = DecodeFloat(uVar3);
			puVar5 = (fVar4 * 3.5f) / pSoundInstance->frequency;
		}
		else {
			puVar5 = -1.0f;
		}
	}
	else {
		p_Var1 = pSoundInstance->pSoundStream;
		SOUND_GetStreamInfo(p_Var1->streamBufferId[0], &streamInfo);
		bVar2 = true;
		if (streamInfo.field_0x20 == -1) {
			pSoundInstance->field_0x92 = pSoundInstance->field_0x92 + 1;
			bVar2 = false;
			uVar3 = p_Var1->field_0x10;
			fVar4 = DecodeFloat(uVar3);
			fVar4 = fVar4 * (float)(uint)pSoundInstance->field_0x92 * edSoundGlobalParams.g_DesiredFrameTime_00483824 * 0.9f;
			uVar3 = EncodeFloat(fVar4);
		}
		else {
			uVar3 = streamInfo.int4 + streamInfo.field_0x20 * p_Var1->field_0x10;
		}

		fVar4 = DecodeFloat(uVar3);
		puVar5 = (fVar4 * 3.5f) / (p_Var1->field_0x18 * 2.0f);
		if (bVar2) {
			puVar5 = puVar5 + 0.04f;
		}
	}

	return puVar5;
}

float edSoundStreamGetPlaybackTime(uint index)
{
	float puVar1;

	if ((index != 0) && (pedSoundInstances[index & 0xffff].fullSoundInstanceId == index)) {
		puVar1 = edSoundInstanceGetPlaybackTime(pedSoundInstances + (index & 0xffff));
		return puVar1;
	}

	return -1.0f;
}

uint edSoundSamplePlay(float priority, ed_sound_sample* pSample)
{
	ed_sound_instance* newSoundInstance;
	uint newSoundId;
	ed_sound_instance* newSoundInstanceB;
	ed_sound_instance* newSoundInstanceA;

	newSoundInstance = edSoundInstanceCreate(priority, 0);
	if (newSoundInstance == (ed_sound_instance*)0x0) {
		newSoundId = 0;
	}
	else {
		newSoundInstance->pSample = pSample;
		newSoundId = newSoundInstance->fullSoundInstanceId;
	}

	if (newSoundId != 0) {
		if (newSoundId != 0) {
			newSoundInstanceA = pedSoundInstances + (newSoundId & 0xffff);
			if (newSoundInstanceA->fullSoundInstanceId == newSoundId) {
				newSoundInstanceA->flags = newSoundInstanceA->flags | 0x100;
			}
		}

		if (newSoundId != 0) {
			newSoundInstanceB = pedSoundInstances + (newSoundId & 0xffff);
			if (newSoundInstanceB->fullSoundInstanceId == newSoundId) {
				if (newSoundInstanceB->field_0x74 == 0) {
					edSoundInstanceCom[newSoundId & 0xffff].flags = edSoundInstanceCom[newSoundId & 0xffff].flags & 0xffffbfff;
					edSoundInstanceCom[newSoundId & 0xffff].soundInstanceId = newSoundId;
					edSoundInstanceCom[newSoundId & 0xffff].flags = edSoundInstanceCom[newSoundId & 0xffff].flags | 0x2000;
					edSoundInstanceCom[newSoundId & 0xffff].soundInstanceId = newSoundId;
				}

				newSoundInstanceB->field_0x74 = 1;
			}
		}
	}

	return newSoundId;
}

void edSoundInstanceSetVolume(float param_1, uint soundInstanceId)
{
	ed_sound_instance* pSoundInstance;
	float fVar1;

	if ((soundInstanceId != 0) && (pSoundInstance = pedSoundInstances + (soundInstanceId & 0xffff), pSoundInstance->fullSoundInstanceId == soundInstanceId)) {
		if (param_1 < 0.0f) {
			param_1 = 0.0f;
		}

		fVar1 = 1.0f;
		if (param_1 <= 1.0f) {
			fVar1 = param_1;
		}

		pSoundInstance->targetVolume = fVar1;
		pSoundInstance->volume = fVar1;
	}

	return;
}

void edSoundInstanceSetPause(uint soundInstanceId, int bPaused)
{
	if ((soundInstanceId != 0) && (pedSoundInstances[soundInstanceId & 0xffff].fullSoundInstanceId == soundInstanceId)) {
		if (bPaused == 0) {
			edSoundInstanceCom[soundInstanceId & 0xffff].flags = edSoundInstanceCom[soundInstanceId & 0xffff].flags & 0xfffffff7;
			edSoundInstanceCom[soundInstanceId & 0xffff].soundInstanceId = soundInstanceId;
			edSoundInstanceCom[soundInstanceId & 0xffff].flags = edSoundInstanceCom[soundInstanceId & 0xffff].flags | 0x10;
			edSoundInstanceCom[soundInstanceId & 0xffff].soundInstanceId = soundInstanceId;
		}
		else {
			edSoundInstanceCom[soundInstanceId & 0xffff].flags = edSoundInstanceCom[soundInstanceId & 0xffff].flags & 0xffffffef;
			edSoundInstanceCom[soundInstanceId & 0xffff].soundInstanceId = soundInstanceId;
			edSoundInstanceCom[soundInstanceId & 0xffff].flags = edSoundInstanceCom[soundInstanceId & 0xffff].flags | 8;
			edSoundInstanceCom[soundInstanceId & 0xffff].soundInstanceId = soundInstanceId;
		}
	}

	return;
}

void edSoundInstanceSet3DData(uint soundInstanceId, edsound_3d_data* pData, uint* existingSoundIDPtr, uint param_4)
{
	undefined uVar1;
	undefined uVar2;
	bool bVar3;
	ed_sound_instance* soundInstance;
	edsound_3d_data* peVar4;
	float fVar5;
	float fVar6;

	if (soundInstanceId != 0) {
		soundInstance = pedSoundInstances + (soundInstanceId & 0xffff);
		bVar3 = false;
		if (soundInstance->fullSoundInstanceId == soundInstanceId) {
			if (pData == (edsound_3d_data*)0x0) {
				soundInstance->flags = soundInstance->flags & 0xffffefff;
				if (soundInstance->p3dData != (edsound_3d_data*)0x0) {
					bVar3 = true;
				}
			}
			else {
				soundInstance->flags = soundInstance->flags | 0x1000;
				soundInstance->flags = soundInstance->flags & 0xfffffffd;
				if ((param_4 == 0) || (edSoundGlobalParams.outputMode != SURROUND)) {
					soundInstance->flags = soundInstance->flags | 4;
				}
				else {
					soundInstance->flags = soundInstance->flags | 8;
				}
			}

			if (bVar3) {
				soundInstance->flags = soundInstance->flags & 0xfffffbff;
			}

			peVar4 = pData;
			if ((existingSoundIDPtr != (uint*)0x0) && (pData != (edsound_3d_data*)0x0)) {
				peVar4 = &soundInstance->data3d;
				*peVar4 = *pData;
			}

			soundInstance->p3dData = peVar4;
		}
	}

	return;
}

void edSoundInstanceSetFrequency(float frequency, uint soundInstanceId)
{
	ed_sound_instance* soundInstance;
	uint loadedSoundID;

	if (soundInstanceId != 0) {
		soundInstance = pedSoundInstances + (soundInstanceId & 0xffff);
		loadedSoundID = soundInstance->fullSoundInstanceId;
		if (loadedSoundID == soundInstanceId) {
			edSoundInstanceCom[loadedSoundID & 0xffff].flags = edSoundInstanceCom[loadedSoundID & 0xffff].flags | 0x80;
			edSoundInstanceCom[loadedSoundID & 0xffff].soundInstanceId = loadedSoundID;
			soundInstance->targetFrequency = frequency;
			soundInstance->frequency = frequency;
		}
	}

	return;
}

void edSoundInstanceSetUserData(uint soundInstanceId, void* pUserData)
{
	if (soundInstanceId != 0) {
		pedSoundInstances[soundInstanceId & 0xffff].field_0xa0 = pUserData;
	}

	return;
}

void edSoundSamplePlayAlt(float priority, ed_sound_sample* pSoundSample)
{
	bool bVar1;
	ed_sound_instance* pInstance;
	uint newSoundId;
	uint* puVar3;

	if (pSoundSample->loopEndOffset == 0.0) {
		newSoundId = 0;
	}
	else {
		pInstance = edSoundInstanceCreate(priority, 0);
		if (pInstance == (ed_sound_instance*)0x0) {
			newSoundId = 0;
		}
		else {
			pInstance->pSample = pSoundSample;
			if (((_ed_sound_stream*)pInstance->pSample)->field_0x14 == 2) {
				pInstance->flags = pInstance->flags | 0x20;
			}
			bVar1 = FUN_00289cf0(pInstance, (_ed_sound_stream*)pSoundSample);
			if (bVar1 == false) {
				edSoundInstanceDelete(pInstance);
				newSoundId = 0;
			}
			else {
				pInstance->flags = pInstance->flags | 0x10;
				newSoundId = pInstance->fullSoundInstanceId;
			}
		}
	}

	if (newSoundId != 0) {
		if (newSoundId != 0) {
			ed_sound_instance* newSoundInstanceA = pedSoundInstances + (newSoundId & 0xffff);
			if (newSoundInstanceA->fullSoundInstanceId == newSoundId) {
				newSoundInstanceA->flags = newSoundInstanceA->flags | 0x100;
			}
		}

		if (newSoundId != 0) {
			ed_sound_instance* newSoundInstanceB = pedSoundInstances + (newSoundId & 0xffff);
			if (newSoundInstanceB->fullSoundInstanceId == newSoundId) {
				if (newSoundInstanceB->field_0x74 == 0) {
					edSoundInstanceCom[newSoundId & 0xffff].flags = edSoundInstanceCom[newSoundId & 0xffff].flags & 0xffffbfff;
					edSoundInstanceCom[newSoundId & 0xffff].soundInstanceId = newSoundId;
					edSoundInstanceCom[newSoundId & 0xffff].flags = edSoundInstanceCom[newSoundId & 0xffff].flags | 0x2000;
					edSoundInstanceCom[newSoundId & 0xffff].soundInstanceId = newSoundId;
				}

				newSoundInstanceB->field_0x74 = 1;
			}
		}
	}

	return;
}

void* edSound_0x002840e0(uint soundInstanceId)
{
	void* pvVar1;

	if (soundInstanceId == 0) {
		pvVar1 = (void*)0x0;
	}
	else {
		ed_sound_instance* newSoundInstanceB = pedSoundInstances + (soundInstanceId & 0xffff);

		pvVar1 = newSoundInstanceB->field_0xa0;
	}

	return pvVar1;
}
