#include "edSound/edSoundInstance.h"
#include "edSound/edSoundPlay.h"
#include "edMem.h"

ed_sound_instance* pedSoundInstances;
ed_sound_instance_finished* pedSoundFinishedInstances;
uint edSoundNbFinishedInstances;
uint edSoundMaxInstances;
uint* pedSoundInstancesToDelete;
int edSoundInstancesToDeleteNb;
edSoundInstanceComType* edSoundInstanceCom;
uint edSoundCurrentInstancesNumber;
ushort edSoundInstanceNewGlobalIndex = 1;
uint edSoundNextFreeInstanceIndex;

ed_sound_instance* pedSoundInstanceListHead;
ed_sound_instance* pedSoundInstanceListTail;

void _edSoundInitInstances() {}

void edSoundInitInstances(int nbInstances)
{
	uint uVar1;
	ed_sound_instance* peVar2;

	pedSoundInstances = (ed_sound_instance*)edMemAlloc(TO_HEAP(H_MAIN), nbInstances * sizeof(ed_sound_instance));
	pedSoundFinishedInstances = (ed_sound_instance_finished*)edMemAlloc(TO_HEAP(H_MAIN), nbInstances * sizeof(ed_sound_instance_finished));
	_edSoundInitInstances();
	edSoundNbFinishedInstances = 0;
	uVar1 = 0;
	peVar2 = pedSoundInstances;
	edSoundMaxInstances = nbInstances;
	if (nbInstances != 0) {
		do {
			peVar2->soundInstanceIndex = 0xffff;
			peVar2->lowerPrioritySoundInstance = (ed_sound_instance*)0x0;
			uVar1 = uVar1 + 1;
			peVar2->higherPrioritySoundInstance = (ed_sound_instance*)0x0;
			peVar2 = peVar2 + 1;
		} while (uVar1 < edSoundMaxInstances);
	}

	return;
}

void _edSoundInitInstance(ed_sound_instance* pSoundInstance)
{
	pSoundInstance->field_0x88 = 1.0;
	pSoundInstance->field_0x8c = 1.0;
	pSoundInstance->field_0x90 = 0;
	pSoundInstance->field_0x92 = 0;
	pSoundInstance->field_0x94 = 1.0;
	pSoundInstance->field_0x98 = 1.0;
	pSoundInstance->field_0x9c = 0;
	pSoundInstance->field_0x9e = 0;
	return;
}

void edSoundInitInstance(ed_sound_instance* pSoundInstance)
{
	pSoundInstance->volume = 1.0f;
	pSoundInstance->field_0x4c = 1.0f;
	pSoundInstance->targetVolume = 1.0f;
	pSoundInstance->frequency = 1.0f;
	pSoundInstance->field_0x50 = 1.0f;
	pSoundInstance->targetFrequency = 1.0f;
	pSoundInstance->field_0x74 = 0;
	pSoundInstance->field_0x75 = 0;
	pSoundInstance->fadeType = 0;
	pSoundInstance->duration = 0.0f;
	pSoundInstance->field_0x64 = 0;
	pSoundInstance->field_0x68 = 0.0f;
	pSoundInstance->field_0x6c = 0;
	pSoundInstance->field_0x70 = 0;

	_edSoundInitInstance(pSoundInstance);

	pSoundInstance->p3dData = (edsound_3d_data*)0x0;
	pSoundInstance->field_0xa0 = 0;

	return;
}

ed_sound_instance* edSoundInstanceCreate(float priority, int bForce)
{
	ed_sound_instance* pScanInstance;
	ed_sound_instance* newSoundInstance;
	uint soundInstanceIndex;

	if (edSoundCurrentInstancesNumber < edSoundMaxInstances) {
		if (edSoundNextFreeInstanceIndex == 0xffffffff) {
			edSoundNextFreeInstanceIndex = 0;
			pScanInstance = pedSoundInstances;
			if (edSoundMaxInstances != 0) {
				do {
					if (pScanInstance->soundInstanceIndex == 0xffff) {
						break;
					}
					edSoundNextFreeInstanceIndex++;
					pScanInstance++;
				} while (edSoundNextFreeInstanceIndex < edSoundMaxInstances);
			}
		}

		soundInstanceIndex = edSoundNextFreeInstanceIndex;
		newSoundInstance = pedSoundInstances + edSoundNextFreeInstanceIndex;
	}
	else {
		edDebugPrintf("edSoundInstanceCreate : no free instances (trying to delete one), -> please increase the number of sound instances handled\\n");
		newSoundInstance = pedSoundInstanceListTail;
		if ((pedSoundInstanceListTail->flags & 1) == 0) {
			if (bForce == 0 && priority < pedSoundInstanceListTail->priority) {
				edDebugPrintf("edSoundInstanceCreate : new instance creation refused (not prioritary enough) -> you might increase the number of instances at edSoundInit()\\n");
				return (ed_sound_instance*)0x0;
			}
		}
		else if (bForce == 0 || priority < pedSoundInstanceListTail->priority) {
			edDebugPrintf("edSoundInstanceCreate : new instance creation refused (not prioritary enough) -> you might increase the number of instances at edSoundInit()\\n");
			return (ed_sound_instance*)0x0;
		}

		soundInstanceIndex = (uint)(pedSoundInstanceListTail - pedSoundInstances);
		edSoundInstanceDeleteLessPrioritary();
	}

	newSoundInstance->soundInstanceIndex = (ushort)soundInstanceIndex;
	newSoundInstance->generation = edSoundInstanceNewGlobalIndex;
	newSoundInstance->flags = 0x82;
	if (bForce) {
		newSoundInstance->flags |= 1;
	}
	newSoundInstance->priority = priority;
	if (++edSoundInstanceNewGlobalIndex == 0) {
		edSoundInstanceNewGlobalIndex = 1;
	}

	edSoundNextFreeInstanceIndex = 0xffffffff;
	edSoundCurrentInstancesNumber++;
	if (soundInstanceIndex + 1 < edSoundMaxInstances &&
		newSoundInstance[1].soundInstanceIndex == 0xffff) {
		edSoundNextFreeInstanceIndex = soundInstanceIndex + 1;
	}

	_edSoundInstanceListInstanceInsert(priority, newSoundInstance, bForce);
	edSoundInitInstance(newSoundInstance);

	return newSoundInstance;
}

void _edSoundInstanceListRemoveTail(void)
{
	ed_sound_instance* peVar1;

	peVar1 = pedSoundInstanceListTail;
	if (pedSoundInstanceListTail == pedSoundInstanceListHead) {
		pedSoundInstanceListTail = (ed_sound_instance*)0x0;
		pedSoundInstanceListHead = (ed_sound_instance*)0x0;
	}
	else {
		pedSoundInstanceListTail = pedSoundInstanceListTail->higherPrioritySoundInstance;
		pedSoundInstanceListTail->lowerPrioritySoundInstance = (ed_sound_instance*)0x0;
	}

	peVar1->higherPrioritySoundInstance = (ed_sound_instance*)0x0;
	peVar1->lowerPrioritySoundInstance = (ed_sound_instance*)0x0;

	return;
}

void edSoundInstanceDeleteLessPrioritary()
{
	_edSoundInstanceSetFree(pedSoundInstanceListTail);
	_edSoundInstanceListRemoveTail();
	edSoundCurrentInstancesNumber = edSoundCurrentInstancesNumber - 1;

	return;
}

void edSoundInstancesComputeFade(ed_sound_instance* soundInstance)
{
	float fVar1;
	ed_sound_instance* pInstance;
	uint soundInstanceId;

	if (soundInstance != (ed_sound_instance*)0x0) {
		do {
			pInstance = soundInstance->lowerPrioritySoundInstance;
			if ((soundInstance->flags & 0x2000) != 0) {
				if (edSoundGlobalParams.g_DesiredFrameTime_00483824 < soundInstance->duration) {
					fVar1 = edSoundGlobalParams.g_DesiredFrameTime_00483824 / soundInstance->duration;
					soundInstance->volume = soundInstance->volume + fVar1 * (soundInstance->targetVolume - soundInstance->volume);
					soundInstance->frequency = soundInstance->frequency + fVar1 * (soundInstance->targetFrequency - soundInstance->frequency);
					soundInstance->duration = soundInstance->duration - edSoundGlobalParams.g_DesiredFrameTime_00483824;
				}
				else {
					soundInstance->flags = soundInstance->flags & 0xffffdfff;
					soundInstance->duration = 0.0;
					soundInstance->volume = soundInstance->targetVolume;
					soundInstance->frequency = soundInstance->targetFrequency;
					if ((soundInstance->targetVolume == 0.0) || (soundInstance->targetFrequency == 0.0)) {
						soundInstanceId = soundInstance->fullSoundInstanceId;
						edSoundInstanceCom[soundInstanceId & 0xffff].flags = edSoundInstanceCom[soundInstanceId & 0xffff].flags & 0xfffffffd;
						edSoundInstanceCom[soundInstanceId & 0xffff].soundInstanceId = soundInstanceId;
						soundInstanceId = soundInstance->fullSoundInstanceId;
						edSoundInstanceCom[soundInstanceId & 0xffff].flags = edSoundInstanceCom[soundInstanceId & 0xffff].flags | 4;
						edSoundInstanceCom[soundInstanceId & 0xffff].soundInstanceId = soundInstanceId;
						if ((soundInstance->flags & 1) == 0) {
							_edSoundInstanceSetFree(soundInstance);

							edSoundCurrentInstancesNumber = edSoundCurrentInstancesNumber + -1;
							if (soundInstance->higherPrioritySoundInstance == (ed_sound_instance*)0x0) {
								pedSoundInstanceListHead = soundInstance->lowerPrioritySoundInstance;
							}
							else {
								soundInstance->higherPrioritySoundInstance->lowerPrioritySoundInstance = soundInstance->lowerPrioritySoundInstance;
							}
							if (soundInstance->lowerPrioritySoundInstance == (ed_sound_instance*)0x0) {
								pedSoundInstanceListTail = soundInstance->higherPrioritySoundInstance;
							}
							else {
								soundInstance->lowerPrioritySoundInstance->higherPrioritySoundInstance = soundInstance->higherPrioritySoundInstance;
							}
						}
						else {
							soundInstance->flags = soundInstance->flags & 0xfffffeff;
							soundInstance->flags = soundInstance->flags | 0x80;
						}
					}
				}
			}

			soundInstance = pInstance;
		} while (pInstance != (ed_sound_instance*)0x0);
	}

	return;
}

int _edSoundInstanceListRemoveFromCurrentToTail(ed_sound_instance* pInstance)
{
	int nbRemoved;
	ed_sound_instance* pCurInstance;

	pedSoundInstanceListTail = pInstance->higherPrioritySoundInstance;
	if (pedSoundInstanceListTail == (ed_sound_instance*)0x0) {
		pedSoundInstanceListHead = (ed_sound_instance*)0x0;
	}
	else {
		pedSoundInstanceListTail->lowerPrioritySoundInstance = (ed_sound_instance*)0x0;
	}
	nbRemoved = 0;
	if (pInstance != (ed_sound_instance*)0x0) {
		do {
			_edSoundInstanceSetFree(pInstance);

			pCurInstance = pInstance->lowerPrioritySoundInstance;
			nbRemoved = nbRemoved + 1;
			pInstance->higherPrioritySoundInstance = (ed_sound_instance*)0x0;
			pInstance->lowerPrioritySoundInstance = (ed_sound_instance*)0x0;
			pInstance = pCurInstance;
		} while (pCurInstance != (ed_sound_instance*)0x0);
	}
	return nbRemoved;
}

void edSoundInstanceDeleteAllFromCurrentLessPrioritary(ed_sound_instance* pInstance)
{
	int nbRemoved;

	nbRemoved = _edSoundInstanceListRemoveFromCurrentToTail(pInstance);
	edSoundCurrentInstancesNumber = edSoundCurrentInstancesNumber - nbRemoved;
	return;
}

void _edSoundInstanceListInstanceRemove(ed_sound_instance* pInstance)
{
	if (pInstance->higherPrioritySoundInstance == (ed_sound_instance*)0x0) {
		pedSoundInstanceListHead = pInstance->lowerPrioritySoundInstance;
	}
	else {
		pInstance->higherPrioritySoundInstance->lowerPrioritySoundInstance = pInstance->lowerPrioritySoundInstance;
	}

	if (pInstance->lowerPrioritySoundInstance == (ed_sound_instance*)0x0) {
		pedSoundInstanceListTail = pInstance->higherPrioritySoundInstance;
	}
	else {
		pInstance->lowerPrioritySoundInstance->higherPrioritySoundInstance = pInstance->higherPrioritySoundInstance;
	}

	return;
}

void edSoundInstanceDelete(ed_sound_instance* pInstance)
{
	_edSoundInstanceSetFree(pInstance);
	edSoundCurrentInstancesNumber = edSoundCurrentInstancesNumber - 1;
	_edSoundInstanceListInstanceRemove(pInstance);

	return;
}

bool edSoundInstanceFinish(ed_sound_instance* pInstance, int param_2)
{
	bool bSuccess;

	if ((pInstance->flags & 1) == 0) {
		if (param_2 != 0) {
			pedSoundFinishedInstances[edSoundNbFinishedInstances].pSoundInstance = (ed_sound_instance*)pInstance->fullSoundInstanceId;
			pedSoundFinishedInstances[edSoundNbFinishedInstances].field_0x4 = pInstance->field_0xa0;
			edSoundNbFinishedInstances = edSoundNbFinishedInstances + 1;
		}

		_edSoundInstanceSetFree(pInstance);
		edSoundCurrentInstancesNumber = edSoundCurrentInstancesNumber - 1;
		_edSoundInstanceListInstanceRemove(pInstance);
		bSuccess = true;
	}
	else {
		bSuccess = false;
		pInstance->flags = pInstance->flags & 0xfffffeff;
		pInstance->flags = pInstance->flags | 0x80;
	}

	return bSuccess;
}

bool edSoundInstanceIsAlive(uint soundID)
{
	return soundID != 0 &&
		pedSoundInstances[soundID & 0xffff].fullSoundInstanceId == soundID;
}

uint _edSoundInstanceCheckFinished(ed_sound_instance* pSoundInstance)
{
	uint voiceCount = EdSoundVoiceCountFromFlags(pSoundInstance->flags);

	if ((pSoundInstance->flags & 0x10) == 0) {
		uint activeVoiceCount = voiceCount;
		for (uint voiceIndexIndex = voiceCount; voiceIndexIndex != 0; voiceIndexIndex--) {
			if (pedSoundVoicePosition[pSoundInstance->voiceIndices[voiceIndexIndex - 1]].position == -1) {
				activeVoiceCount--;
			}
		}

		voiceCount = activeVoiceCount;
	}
	else if (pedSoundVoicePosition[pSoundInstance->voiceIndices[0]].position == -1) {
		voiceCount = 0;
	}

	if (voiceCount != 0) {
		pSoundInstance->field_0x80[0] = pedSoundVoicePosition[pSoundInstance->voiceIndices[0]].position;
	}
	else {
		edSoundInstanceFinish(pSoundInstance, 1);
	}

	return voiceCount == 0;
}

void _edSoundInstanceListInstanceInsert(float priority, ed_sound_instance* newSoundInstance, long param_3)
{
	ed_sound_instance* peVar1;
	ed_sound_instance* peVar2;
	ed_sound_instance* prevSoundInstance;
	ed_sound_instance* soundInstance;

	peVar2 = pedSoundInstanceListHead;
	/* Check if the there is currently no highest priority sound */
	if (pedSoundInstanceListHead == (ed_sound_instance*)0x0) {
		pedSoundInstanceListHead = newSoundInstance;
		pedSoundInstanceListTail = newSoundInstance;
		/* No highest proirity sound, we can take this spot */
		newSoundInstance->lowerPrioritySoundInstance = (ed_sound_instance*)0x0;
		/* Set us to be both the highest and lowest priority sounds */
		newSoundInstance->higherPrioritySoundInstance = (ed_sound_instance*)0x0;
		peVar2 = pedSoundInstanceListTail;
	}
	else {
		/* Start checking the current highest priority sound
		   Check if the new sound has lower priority */
		soundInstance = pedSoundInstanceListHead;
		if ((newSoundInstance->priority < pedSoundInstanceListHead->priority) || ((param_3 == 0 && (soundInstance = pedSoundInstanceListHead, (pedSoundInstanceListHead->flags & 1) != 0)))) {
			do {
				while (true) {
					prevSoundInstance = soundInstance;
					/* The new sound has a lower priority that the current highest, so loop through until we find a sound that is lower priority */
					soundInstance = prevSoundInstance->lowerPrioritySoundInstance;
					if (soundInstance == (ed_sound_instance*)0x0) goto LAB_FoundSoundSlot;
					if (param_3 == 0) break;
					if (((soundInstance->flags & 1) == 0) || (soundInstance->priority <= priority)) goto LAB_FoundSoundSlot;
				}
			} while (((soundInstance->flags & 1) != 0) || (priority < soundInstance->priority));
		LAB_FoundSoundSlot:
			/* Found a lower priority sound, set our lower priority sound to be that sound, and then set new sound to point to the higher priority sound */
			newSoundInstance->lowerPrioritySoundInstance = soundInstance;
			newSoundInstance->higherPrioritySoundInstance = prevSoundInstance;
			/* Set the higher priority sound to point to the new sound */
			prevSoundInstance->lowerPrioritySoundInstance = newSoundInstance;
			/* Restore the sound instance to being the new sound, we are done with the lower priority sound */
			peVar2 = newSoundInstance;
			if (newSoundInstance->lowerPrioritySoundInstance != (ed_sound_instance*)0x0) {
				/* If we have a lower priority sound, set their higher priority sound to be the new sound */
				newSoundInstance->lowerPrioritySoundInstance->higherPrioritySoundInstance = newSoundInstance;
				peVar2 = pedSoundInstanceListTail;
			}
		}
		else {
			peVar1 = newSoundInstance;
			/* Set the lower priority sound for our new sound to the previous highest priority sound */
			newSoundInstance->lowerPrioritySoundInstance = pedSoundInstanceListHead;
			pedSoundInstanceListHead = peVar1;
			/* We don't have any higher priority sounds */
			newSoundInstance->higherPrioritySoundInstance = (ed_sound_instance*)0x0;
			/* Link the previous highest priority sounds highest priority sound to the new sound, and the previous highest priority sound is now the new sound */
			peVar2->higherPrioritySoundInstance = newSoundInstance;
			peVar2 = pedSoundInstanceListTail;
		}
	}

	pedSoundInstanceListTail = peVar2;
	/* Set the lowest priority sound back to global memory */
	return;
}

edsound_3d_data::edsound_3d_data()
{
	this->field_0x18 = -1.0f;
	this->field_0x20 = 0.0f;
	this->field_0x24 = 0;

	return;
}