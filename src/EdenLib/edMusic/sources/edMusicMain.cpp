#include "edMusic/edMusic.h"

void edMusicSetOutputMode(AUDIO_MODE newMode)
{
	if (newMode == SURROUND) {
		newMode = STEREO;
	}

	edMusicStatus.outputMode = newMode;
	_edMusicSetOutputMode(newMode);

	return;
}
