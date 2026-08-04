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