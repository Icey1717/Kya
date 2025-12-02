#include "edSound/edSoundPlay.h"
#include "edSound/ps2/_edSoundPlay.h"

static int gNbSoundStreams = 0;

int edSoundStreamLoadA(_ed_sound_stream* pSoundStream, GlobalSound_FileData* pSoundData, char* szPath, undefined8 param_4)
{
	int iVar1;

	iVar1 = _edSoundStreamInit(pSoundData, pSoundStream, szPath, 0, param_4, 0);
	gNbSoundStreams = gNbSoundStreams + 1;
	return iVar1;
}

int edSoundStreamLoadB(_ed_sound_stream* pSoundStream, GlobalSound_FileData* pSoundData, ulong param_3, undefined8 param_4, undefined8 param_5)
{
	int iVar1;

	iVar1 = _edSoundStreamInit(pSoundData, pSoundStream, (char*)0x0, param_3, param_4, param_5);
	gNbSoundStreams = gNbSoundStreams + 1;
	return iVar1;
}

void edSoundStreamFree(_ed_sound_stream* pSoundStream)
{
	_edSoundStreamTerm(pSoundStream);
	gNbSoundStreams = gNbSoundStreams + -1;

	return;
}

void edSoundFlush()
{
	IMPLEMENTATION_GUARD_AUDIO();
}

uint edSoundInstanceStop(uint instanceId)
{
	IMPLEMENTATION_GUARD_AUDIO();
	return 0;
}
