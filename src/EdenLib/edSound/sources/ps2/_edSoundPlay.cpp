#include "edSound/edSoundPlay.h"
#include "edSys/ps2/edSysDataTransfer.h"

#define READ_BE_UINT32(pValue) \
		(((uint)((const byte*)(pValue))[0] << 24) | \
		((uint)((const byte*)(pValue))[1] << 16) | \
		((uint)((const byte*)(pValue))[2] << 8) | \
		((uint)((const byte*)(pValue))[3]))

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

void _edSoundWaitAllSoundDataLoaded(void)
{
	_edSysWaitUntilTransferFinished(_edSoundLastTransferIndex);
	_edSoundLastTransferIndex = 0;

	return;
}

int _edSoundLoadToSoundRamNoWait(void* pAdpcm, uint dataSize, int param_3, ed_sound_sample* pSample, edSysTransferFunc* pFunc)
{
	EdSysTransferFlags setupFlags;

	setupFlags = NO_FLAGS_SET;
	if (param_3 != 0) {
		setupFlags = HIGH_MEM;
	}

	_edSoundLastTransferIndex = _edSysTransferData(pAdpcm, dataSize, 0x40, setupFlags, 1, 2, 3, pSample, 4, 0, pFunc);

	return _edSoundLastTransferIndex;
}

int edSoundLoadToSoundRamNoWaitDirect(void* pAdpcm, uint size, long param_3, ed_sound_sample* pSample, edSysTransferFunc* pFunc)
{
	EdSysTransferFlags setupFlags;

	setupFlags = (EdSysTransferFlags)(ONE_BLOCK_ONLY | KEEP_IN_IOP);
	if (param_3 != 0) {
		setupFlags = (EdSysTransferFlags)(ONE_BLOCK_ONLY | KEEP_IN_IOP | HIGH_MEM);
	}

	_edSoundLastTransferIndex = _edSysTransferData(pAdpcm, size, 0x40, setupFlags, 1, 2, 3, pSample, 4, 0, pFunc);

	return _edSoundLastTransferIndex;
}

void SetFlag_00288910(void* pParam)
{
	ed_sound_sample* pSoundSample = (ed_sound_sample*)pParam;
	pSoundSample->flags = pSoundSample->flags | 4;

	return;
}

void edSoundPrepareSampleLoad(SoundFileData* soundFileData, ed_sound_sample* pSample, ulong flags)
{
	soundFileData->header.magic[0] = 'M';
	soundFileData->header.magic[1] = 'O';
	soundFileData->header.magic[2] = 'D';
	soundFileData->header.magic[3] = 'p';

	pSample->dataSize =
		READ_BE_UINT32(&soundFileData->header.dataSizeBe);

	pSample->sampleRate =
		READ_BE_UINT32(&soundFileData->header.sampleRateBe);

	pSample->flags = 0;

	if ((flags & 1) != 0) {
		pSample->flags |= 2;
	}

	// The instruction at 0x00288730 reads offset 0x41.
	// ADPCM data starts at 0x30, making this block 1's flags byte.
	if ((soundFileData->adpcm[1].flags & 2) != 0) {
		pSample->flags |= 1;
	}

	pSample->loopStartOffset = 0;
	pSample->loopEndOffset = 0;

	adpcmBlock* pFirstBlock = soundFileData->adpcm;
	adpcmBlock* pBlock = pFirstBlock;
	uint blockCount = pSample->dataSize >> 4;

	while (blockCount != 0) {
		blockCount--;

		uint blockOffset =
			(uint)((byte*)pBlock - (byte*)pFirstBlock);

		if ((pBlock->flags & 4) != 0) {
			pSample->loopStartOffset = blockOffset;
		}

		if ((pBlock->flags & 1) == 0) {
			pBlock->flags = 2;
		}
		else {
			pSample->loopEndOffset = blockOffset;

			if ((flags & 1) == 0) {
				pBlock->flags = 3;
			}
			else {
				pBlock->flags = 2;
			}
		}

		pBlock++;
	}

	return;
}


int _edSoundSampleLoad(SoundFileData* soundFileData, ed_sound_sample* pSample, ulong flags)
{
	int transferIndex;

	edSoundPrepareSampleLoad(soundFileData, pSample, flags);

	pSample->soundRamAddress = 0;
	if ((flags & 1) == 0) {
		transferIndex = _edSoundLoadToSoundRamNoWait(soundFileData->adpcm, pSample->dataSize, flags & 2, pSample, SetFlag_00288910);
	}
	else {
		transferIndex = edSoundLoadToSoundRamNoWaitDirect(soundFileData->adpcm, pSample->dataSize, flags & 2, pSample, SetFlag_00288910);
	}

	return transferIndex;
}

