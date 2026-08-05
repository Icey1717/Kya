#include "edSound/edSoundPlay.h"
#include "edSound/edSoundInit.h"
#include "edSound/edSoundInstance.h"
#include "edSound/ps2/_edSoundPlay.h"

// Recovered globals and functions whose source implementations are still pending.

void edSoundInstancesComputeFade(ed_sound_instance* pSoundInstance);
void _edSoundCheckForInstancesToCreate();
void edSoundAcousticCompute();
uint _edSoundInstanceCheckFinished(ed_sound_instance* pSoundInstance);

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
	// HINT: consume completion IDs here, never directly from an audio callback.
#endif
}

/*
 * Port-only instance-command phase. PS2 calls _edSoundExecuteCommands; Windows
 * will translate each pending mask into typed create/start/gain/pitch commands.
 */
static void EdSoundAppendPendingInstanceCommands()
{
#ifdef PLATFORM_PS2

#elif defined(PLATFORM_WIN)
	// Windows will start a typed command list for this flush.
	// The Windows backend begins an empty typed command list here.

	// Windows will emit typed destroy commands that contain only a copied soundInstanceID.
	// The Windows backend appends one typed destroy command per pending delete.
	 
	// Windows it will drain finished voice IDs from the host audio backend.
	// The Windows backend converts masks and copied payloads to typed commands.
	
	// Windows will translate each pending mask into typed create/start/gain/pitch commands.
	// HINT: only copied command data may cross this thread boundary.
#endif
}


/*
 * Port-only submission phase. PS2 terminates and sends edComBuffer, whereas
 * Windows submits the typed list to its audio thread and swaps frame buffers.
 */
static void EdSoundEndFlush()
{
#ifdef PLATFORM_PS2
	IMPLEMENTATION_GUARD_AUDIO(0);
#elif defined(PLATFORM_WIN)
	// HINT: only copied command data may cross this thread boundary.
#endif
}

uint* _pedSoundInstanceCommandsCount;
void* gSoundFlushCurrent_0044915c;
uint _edSoundInstanceCommandsCount;

void edSoundFlush()
{
	//SoundFlushCommand* iVar1;
	ed_sound_instance** pToDelete;
	uint nbFlush;
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
	edSoundAcousticCompute();

#ifdef PLATFORM_PS2
	iVar1 = (SoundFlushCommand*)((int)edComBuffer + *edComBuffer);
	iVar1->field_0x4 = 0;
	_pedSoundInstanceCommandsCount = &iVar1->nbCommands;
	_edSoundInstanceCommandsCount = 0;
	gSoundFlushCurrent_0044915c = iVar1->aSoundFlushes;
#else 
	// Windows will start a typed command list for this flush.
	// The Windows backend begins an empty typed command list here.
#endif

#ifdef PLATFORM_PS2
	pToDelete = pedSoundInstancesToDelete;
	while (edSoundInstancesToDeleteNb != 0) {
		nbFlush = nbFlush + 1;
		edSoundInstancesToDeleteNb = edSoundInstancesToDeleteNb + -1;
		((SoundFlushCommand*)gSoundFlushCurrent_0044915c)->pToDelete = *pToDelete;
		pToDelete = pToDelete + 1;
		((SoundFlushCommand*)gSoundFlushCurrent_0044915c)->field_0x4 = 0x1000;
		_edSoundInstanceCommandsCount = _edSoundInstanceCommandsCount + 1;
		gSoundFlushCurrent_0044915c = gSoundFlushCurrent_0044915c + 1;
	}
#else
	// Windows will emit typed destroy commands that contain only a copied soundInstanceID.
	// The Windows backend appends one typed destroy command per pending delete.
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
	// Windows it will drain finished voice IDs from the host audio backend.
	// The Windows backend converts masks and copied payloads to typed commands.
#endif

	if (edSoundGlobalParams.finishedInstancesCallback != (edSoundFinishedInstancesCallback)0x0) {
		edSoundGlobalParams.finishedInstancesCallback(pedSoundFinishedInstances, edSoundNbFinishedInstances);
	}

	// Windows will translate each pending mask into typed create/start/gain/pitch commands.
	// HINT: only copied command data may cross this thread boundary.

	_edSoundEndFlush(nbFlush);
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