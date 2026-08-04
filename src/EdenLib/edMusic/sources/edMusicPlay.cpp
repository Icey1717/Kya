#include "edMusic/edMusic.h"

edCMusicStatus edMusicStatus;

void edMusicSetMasterVolume(uint newVolume)
{
	uint local_4;

	edMusicStatus.masterVolume = newVolume;

	IMPLEMENTATION_GUARD_PS2(
		local_4 = newVolume;
	_edMusicQueueCommand(4, 0, &local_4, 4);)

		return;
}

void edMusicSetMasterTempo(uint newTempo)
{
	uint local_4;

	edMusicStatus.masterTempo = newTempo;
	IMPLEMENTATION_GUARD_PS2(
	local_4 = newTempo;
	_edMusicQueueCommand(5, 0, &local_4, 4);)

	return;
}

