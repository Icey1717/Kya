#include "edSound/edSoundPlay.h"

int _edSoundStreamInit(GlobalSound_FileData* pSoundData, _ed_sound_stream* pSoundStream, char* szPath,
	ulong param_4, undefined8 param_5, undefined8 param_6)
{
	IMPLEMENTATION_GUARD_AUDIO();
	return 1;
}

void _edSoundStreamTerm(_ed_sound_stream* pSoundStream)
{
	if (pSoundStream->pMem != (void*)0x0) {
		IMPLEMENTATION_GUARD_AUDIO(
		SOUND_CloseStreamBuffer(pSoundStream->streamBufferId);
		SOUND_FreeFileID(pSoundStream->streamFileId);
		FlushIOPCommand(1, 0);
		_edSoundMemFree(pSoundStream->pMem);)
	}

	return;
}

