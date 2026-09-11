#include "edMusic/edMusic.h"
#include "edSound/edSoundPlay.h"
#include "edSys/ps2/edSysDataTransfer.h"
#ifdef PLATFORM_WIN
#include "edSysTransferService.h"
#include "edMusicService.h"
#endif

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
#ifdef PLATFORM_WIN
	Audio::InstallMusicBank(static_cast<unsigned>(pMusicBank - _pedMusicBanks),
		static_cast<uint>(reinterpret_cast<uintptr_t>(pMusicBank->field_0x4)),
		static_cast<uint>(reinterpret_cast<uintptr_t>(pMusicBank->field_0x0)));
#endif

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
#ifdef PLATFORM_WIN
	pBank->field_0x0 = nullptr;
	pBank->field_0x4 = nullptr;
#endif
	_edMusicLastTransferIndex = _edMusicLoadDataNoWait(param_2, size, (void*)&pBank->field_0x4, (edSysTransferFunc*)0x0);
	_edMusicLastTransferIndex = _edSoundLoadToSoundRamNoWait(pAdpcm, dataSize, 0, pBank, _edMusicBankInstallTransferCallback);

	pBank->flags = pBank->flags & 0xfffffffb;
	pBank->flags = pBank->flags | 2;

	return;
}

void _edMusicDataRemove(uint flags, void* pSong)
{
#ifdef PLATFORM_WIN
	Audio::ReleaseLoadedData(static_cast<uint>(reinterpret_cast<uintptr_t>(pSong)));
	return;
#endif
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
#ifdef PLATFORM_WIN
	_edMusicAreAllMusicDataLoaded();
	Audio::RemoveMusicSong(static_cast<unsigned>(pSong - _pedMusicSongs));
#endif
	IMPLEMENTATION_GUARD_PS2(
	int peVar1;
	int local_4;

	peVar1 = (int)pSong - (int)_pedMusicSongs;
	if (peVar1 < 0) {
		peVar1 = peVar1 + 7;
	}
	local_4 = peVar1 >> 3;)
	_edMusicDataRemove(0, pSong->field_0x0);
#ifdef PLATFORM_WIN
	pSong->field_0x0 = nullptr;
#endif
	//_edMusicQueueCommand(1, 0, &local_4, 4);

	pSong->flags = pSong->flags & 0xfffffffb;

	return;
}


void _edMusicBankRemove(_ed_music_bank* pMusicBank)
{
#ifdef PLATFORM_WIN
	_edMusicAreAllMusicDataLoaded();
	Audio::RemoveMusicBank(static_cast<unsigned>(pMusicBank - _pedMusicBanks));
#endif
	int local_4;

	_edMusicDataRemove(1, pMusicBank->field_0x0);
	_edMusicDataRemove(0, pMusicBank->field_0x4);
#ifdef PLATFORM_WIN
	pMusicBank->field_0x0 = nullptr;
	pMusicBank->field_0x4 = nullptr;
#endif

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

#ifdef PLATFORM_WIN
	if ((_edSysTransferIndex != 0) && (_edMusicLastTransferIndex != 0) &&
		(_edMusicLastTransferIndex > (uint)_edSysCompletedTransferIndex)) {
		_edSysCompletedTransferIndex = static_cast<int>(Audio::PumpThrough(_edMusicLastTransferIndex));
	}
#endif

	bVar1 = true;
	if ((_edSysTransferIndex != 0) && ((_edMusicLastTransferIndex == 0 || (bVar1 = false, _edMusicLastTransferIndex <= _edSysCompletedTransferIndex)))) {
		bVar1 = true;
	}
	return bVar1;
}

void _edMusicSongInstallTransferCallback(void* pData)
{
	uint peVar1;
	int local_8;
	void* local_4;

	ed_music_song* pSong = (ed_music_song*)pData;

	pSong->flags = pSong->flags | 4;
	pSong->flags = pSong->flags & 0xfffffffd;
#ifdef PLATFORM_WIN
	Audio::InstallMusicSong(static_cast<unsigned>(pSong - _pedMusicSongs),
		static_cast<uint>(reinterpret_cast<uintptr_t>(pSong->field_0x0)));
#endif

	IMPLEMENTATION_GUARD_PS2(
	peVar1 = (int)pSong - (int)_pedMusicSongs;
	if ((int)peVar1 < 0) {
		peVar1 = peVar1 + 7;
	}
	local_8 = (int)peVar1 >> 3;
	local_4 = pSong->field_0x0;
	_edMusicQueueCommand(0, 0, &local_8, 8);)

	return;
}

void _edMusicSongInstallNoWait(ed_music_song* pSong, void* pSource, uint size)
{
#ifdef PLATFORM_WIN
	pSong->field_0x0 = nullptr;
#endif
	_edMusicLastTransferIndex = _edMusicLoadDataNoWait(pSource, size, pSong, _edMusicSongInstallTransferCallback);
	pSong->flags = pSong->flags & 0xfffffffb;
	pSong->flags = pSong->flags | 2;

	return;
}
