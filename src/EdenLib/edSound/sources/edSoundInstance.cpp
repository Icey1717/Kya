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
			peVar2->soundInstanceId = 0xffff;
			peVar2->lowerPrioritySoundInstance = (ed_sound_instance*)0x0;
			uVar1 = uVar1 + 1;
			peVar2->higherPrioritySoundInstance = (ed_sound_instance*)0x0;
			peVar2 = peVar2 + 1;
		} while (uVar1 < edSoundMaxInstances);
	}

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
				if (edSoundGlobalParams.g_DesiredFrameTime_00483824 < soundInstance->field_0x60) {
					fVar1 = edSoundGlobalParams.g_DesiredFrameTime_00483824 / soundInstance->field_0x60;
					soundInstance->field_0x44 = soundInstance->field_0x44 + fVar1 * (soundInstance->field_0x54 - soundInstance->field_0x44);
					soundInstance->field_0x48 = soundInstance->field_0x48 + fVar1 * (soundInstance->field_0x58 - soundInstance->field_0x48);
					soundInstance->field_0x60 = soundInstance->field_0x60 - edSoundGlobalParams.g_DesiredFrameTime_00483824;
				}
				else {
					soundInstance->flags = soundInstance->flags & 0xffffdfff;
					soundInstance->field_0x60 = 0.0;
					soundInstance->field_0x44 = soundInstance->field_0x54;
					soundInstance->field_0x48 = soundInstance->field_0x58;
					if ((soundInstance->field_0x54 == 0.0) || (soundInstance->field_0x58 == 0.0)) {
						soundInstanceId = soundInstance->soundInstanceId;
						edSoundInstanceCom[soundInstanceId & 0xffff].flags = edSoundInstanceCom[soundInstanceId & 0xffff].flags & 0xfffffffd;
						edSoundInstanceCom[soundInstanceId & 0xffff].soundInstanceId = soundInstanceId;
						soundInstanceId = soundInstance->soundInstanceId;
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
			pedSoundFinishedInstances[edSoundNbFinishedInstances].pSoundInstance = (ed_sound_instance*)pInstance->soundInstanceId;
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

