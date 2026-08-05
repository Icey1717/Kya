#include "edSound/edSoundInstance.h"
#include "edSound/edSoundPlay.h"
#include "edMem.h"

ed_sound_instance* pedSoundInstances;
ed_sound_instance_finished* pedSoundFinishedInstances;
uint edSoundNbFinishedInstances;
uint edSoundMaxInstances;

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