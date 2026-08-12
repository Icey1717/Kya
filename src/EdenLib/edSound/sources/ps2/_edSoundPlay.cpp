#include "edSound/edSoundPlay.h"
#include "edSound/edSoundInstance.h"
#include "edSys/ps2/edSysDataTransfer.h"

#define READ_BE_UINT32(pValue) \
		(((uint)((const byte*)(pValue))[0] << 24) | \
		((uint)((const byte*)(pValue))[1] << 16) | \
		((uint)((const byte*)(pValue))[2] << 8) | \
		((uint)((const byte*)(pValue))[3]))

void _edSoundStreamFreeDynamicData(ed_sound_instance* pInstance);

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

int _edSoundLoadToSoundRamNoWait(void* pAdpcm, uint dataSize, int param_3, void* pSample, edSysTransferFunc* pFunc)
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

void _edSoundEndFlush(uint nbFlush)
{
#ifdef PLATFORM_PS2
	SoundFlush_0x8* pSVar1;

	*_pedSoundInstanceCommandsCount = _edSoundInstanceCommandsCount;
	gSoundFlushCurrent_0044915c->pToDelete = (ed_sound_instance*)0x0;
	gSoundFlushCurrent_0044915c = (SoundFlush_0x8*)&gSoundFlushCurrent_0044915c->field_0x4;
	pSVar1 = (SoundFlush_0x8*)((int)gSoundFlushCurrent_0044915c - (int)edComBuffer);
	if (((uint)pSVar1 & 0xfffffff0) != 0) {
		pSVar1 = (SoundFlush_0x8*)((uint)(pSVar1 + 2) & 0xfffffff0);
	}
	*edComBuffer = pSVar1;
	CallRpcAndApplyCommands_00289c10(nbFlush, (long)(int)pSVar1);
	edComCurrentBufferIndex = 1 - edComCurrentBufferIndex;
	edComBuffer = (SoundFlush_0x8**)(&edComDoubleBuffers)[edComCurrentBufferIndex];
	*edComBuffer = (SoundFlush_0x8*)0x0;
#else
	// Windows submits the typed list to its audio thread and swaps frame buffers.
	// HINT: only copied command data may cross this thread boundary.
#endif

	return;
}

uint _edSoundAllocatedVoices[2];

void _edSoundStreamFreeDynamicData(ed_sound_instance* pInstance)
{
	pInstance->pSoundStream->pDynamicData = (void*)0x0;

	return;
}

void _edSoundInstanceSetFree(ed_sound_instance* pInstance)
{
	if ((pInstance->flags & 0x400) != 0) {
		uint voiceCount = EdSoundVoiceCountFromFlags(pInstance->flags);

		assert(voiceCount <= 2); // Matches size of voiceIndices array.
		for (uint voiceIndexIndex = 0; voiceIndexIndex < voiceCount; voiceIndexIndex++) {
			EdSoundVoiceSetFree(pInstance->voiceIndices[voiceIndexIndex]);
		}

		pedSoundInstancesToDelete[edSoundInstancesToDeleteNb] = pInstance->fullSoundInstanceId;
		edSoundInstancesToDeleteNb = edSoundInstancesToDeleteNb + 1;
	}

	if ((pInstance->flags & 0x10) != 0) {
		_edSoundStreamFreeDynamicData(pInstance);
	}

	uint soundInstanceId = pInstance->fullSoundInstanceId;
	edSoundInstanceCom[soundInstanceId & 0xffff].flags = 0;
	edSoundInstanceCom[soundInstanceId & 0xffff].soundInstanceId = 0;
	pInstance->soundInstanceId = 0xffff;

	return;
}

uint _edSoundVoiceGetFirstFreeFromPointer(uint* param_1, _ed_sound_bit_array_handle* pSoundBitArrayHandle)
{
	uint uVar1;
	uint uVar2;

	uVar2 = *param_1;
	while (uVar2 == 0xffffffff) {
		pSoundBitArrayHandle->voiceIndex = pSoundBitArrayHandle->voiceIndex + 1;
		param_1 = param_1 + 1;
		if (pSoundBitArrayHandle->voiceIndex == 2) {
			return 0xffffffff;
		}
		uVar2 = *param_1;
	}

	pSoundBitArrayHandle->field_0x4 = 0;
	uVar2 = *param_1;
	uVar1 = pSoundBitArrayHandle->voiceIndex << 5;
	do {
		if ((uVar2 & 1) == 0) {
			pSoundBitArrayHandle->field_0x8 = 1;
			return uVar1;
		}

		uVar1 = uVar1 + 1;
		uVar2 = uVar2 >> 1;
		pSoundBitArrayHandle->field_0x4 = pSoundBitArrayHandle->field_0x4 + 1;
	} while (uVar1 < 0x30);
	return 0xffffffff;
}

uint _edSoundVoiceGetFirstFree(_ed_sound_bit_array_handle* pSoundBitArrayHandle)
{
	uint firstFreeId;

	pSoundBitArrayHandle->voiceIndex = 0;
	firstFreeId = _edSoundVoiceGetFirstFreeFromPointer(_edSoundAllocatedVoices, pSoundBitArrayHandle);
	return firstFreeId;
}

uint _edSoundMemFree(void* pMem)
{
	uint uVar1;

	IMPLEMENTATION_GUARD_PS2(
	*_pedSoundRPCSendBufferUncached = pMem;
	uVar1 = _edSysCallRPC(3, 0, 1, (long)(int)_pedSoundRPCSendBufferUncached, 4, 0, 0, 0, (uint*)0x0, _pedSoundRPCClient->field_0x4, &_pedSoundRPCClient->field_0xc, &_pedSoundRPCClient->field_0x8);
	)
	return uVar1;
}

int _edSoundSampleFree(ed_sound_sample* pSoundSample)
{
	int iVar1;

	if ((pSoundSample->flags & 2) == 0) {
		iVar1 = _edSoundMemFree((void*)pSoundSample->soundRamAddress);
	}
	else {
		IMPLEMENTATION_GUARD_PS2(
		iVar1 = sceSifFreeSysMemory((void*)pSoundSample->soundRamAddress);)
	}
	return iVar1;
}

bool _edSoundAreAllSoundDataLoaded(uint lastIndex)
{
	bool bVar1;

	bVar1 = true;
	if ((_edSysTransferIndex != 0) && ((lastIndex == 0 || (bVar1 = false, lastIndex <= _edSysCompletedTransferIndex)))) {
		bVar1 = true;
	}
	return bVar1;
}
