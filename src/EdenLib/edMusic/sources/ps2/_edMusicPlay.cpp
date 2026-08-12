#include "edMusic/edMusic.h"
#include "edSound/edSoundPlay.h"
#include "edSys/ps2/edSysDataTransfer.h"

extern uint _edMusicLastTransferIndex;

extern uint _edMusicLoadDataNoWait(void* pSource, uint size, void* pCallbackParams, edSysTransferFunc* pCallback);
extern int _edSoundLoadToSoundRamNoWait(void* pAdpcm, uint dataSize, int param_3, void* pSample, edSysTransferFunc* pFunc);

void _edMusicBankInstallTransferCallback(void* pData)
{
	int local_10;
	undefined4 local_c;
	undefined4 local_8;

	_ed_music_bank* pMusicBank = (_ed_music_bank*)pData;

	pMusicBank->flags = pMusicBank->flags | 4;
	pMusicBank->flags = pMusicBank->flags & 0xfffffffd;

	IMPLEMENTATION_GUARD_PS2(
	local_10 = (int)(_ed_music_bank*)((int)pMusicBank - (int)_pedMusicBanks) >> 0x1f;
	local_10 = ((int)(_ed_music_bank*)((int)pMusicBank - (int)_pedMusicBanks) / 6 + local_10 >> 1) - local_10;
	local_c = *(undefined4*)pMusicBank;
	local_8 = *(undefined4*)&pMusicBank->field_0x4;
	_edMusicQueueCommand(2, 0, &local_10, 0xc);)

	return;
}

void _edMusicBankInstallNoWait(_ed_music_bank* pBank, void* param_2, void* pAdpcm, uint size, uint dataSize)
{
	_edMusicLastTransferIndex = _edMusicLoadDataNoWait(param_2, size, (void*)&pBank->field_0x4, (edSysTransferFunc*)0x0);
	_edMusicLastTransferIndex = _edSoundLoadToSoundRamNoWait(pAdpcm, dataSize, 0, pBank, _edMusicBankInstallTransferCallback);

	pBank->flags = pBank->flags & 0xfffffffb;
	pBank->flags = pBank->flags | 2;

	return;
}

void _edMusicDataRemove(uint flags, void* pSong)
{
	if ((flags & 1) == 1) {
		_edSoundMemFree(pSong);
	}
	else {
		if ((flags & 1) == 0) {
			IMPLEMENTATION_GUARD_PS2(
			sceSifFreeSysMemory(pSong);)
		}
	}

	return;
}

void _edMusicSongRemove(ed_music_song* pSong)
{
	IMPLEMENTATION_GUARD_PS2(
	int peVar1;
	int local_4;

	peVar1 = (int)pSong - (int)_pedMusicSongs;
	if (peVar1 < 0) {
		peVar1 = peVar1 + 7;
	}
	local_4 = peVar1 >> 3;)
	_edMusicDataRemove(0, pSong->field_0x0);
	//_edMusicQueueCommand(1, 0, &local_4, 4);

	pSong->flags = pSong->flags & 0xfffffffb;

	return;
}


void _edMusicBankRemove(_ed_music_bank* pMusicBank)
{
	int local_4;

	_edMusicDataRemove(1, pMusicBank->field_0x0);
	_edMusicDataRemove(0, pMusicBank->field_0x4);

	IMPLEMENTATION_GUARD_PS2(
	local_4 = (int)(_ed_music_bank*)((int)pMusicBank - (int)_pedMusicBanks) >> 0x1f;
	local_4 = ((int)(_ed_music_bank*)((int)pMusicBank - (int)_pedMusicBanks) / 6 + local_4 >> 1) - local_4;
	_edMusicQueueCommand(3, 0, &local_4, 4);)

	pMusicBank->flags = pMusicBank->flags & 0xfffffffb;

	return;
}

bool _edMusicAreAllMusicDataLoaded(void)
{
	bool bVar1;

	bVar1 = true;
	if ((_edSysTransferIndex != 0) && ((_edMusicLastTransferIndex == 0 || (bVar1 = false, _edMusicLastTransferIndex <= _edSysCompletedTransferIndex)))) {
		bVar1 = true;
	}
	return bVar1;
}