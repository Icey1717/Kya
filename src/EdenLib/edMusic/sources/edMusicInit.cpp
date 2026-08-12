#include "edMusic/edMusic.h"
#include "edMem.h"

ed_music_config edMusicConfig;

ed_music_config* edMusicGetConfig(void)
{
	return &edMusicConfig;
}

void _edMusicInitStream(ed_music_stream* pMusicStream, uint index)
{
	pMusicStream->index = index;
	pMusicStream->field_0x4 = 0;
	pMusicStream->pSong = (ed_music_song*)0x0;
	pMusicStream->field_0xc = 1;
	pMusicStream->field_0x14 = 0;
	pMusicStream->field_0x10 = 0;
	pMusicStream->tempo = 0x100;
	pMusicStream->volume = 0xffff;
	pMusicStream->field_0xbc = 0;

	for (int i = 0; i < 0x10; i++) {
		pMusicStream->aChannelVolumes[i] = 0xffff;
	}

	pMusicStream->field_0xc0 = 0;
	pMusicStream->fadeFlags = 0;

	return;
}

void _edMusicInitSong(ed_music_song* pMusicSong)
{
	pMusicSong->flags = 0;
	pMusicSong->flags = pMusicSong->flags | 0x1;

	return;
}

void _edMusicInitBank(_ed_music_bank* pMusicBank)
{
	pMusicBank->flags = 0;
	pMusicBank->flags = pMusicBank->flags | 0x1;

	return;
}

ed_music_stream* pedMusicStreams;
ed_music_song* _pedMusicSongs;
_ed_music_bank* _pedMusicBanks;

void edMusicInit(void)
{
	int iVar1;
	int iVar2;
	uint uVar3;

	edDebugPrintf("edMusicInit\n");
	edMusicStatus.outputMode = MONO;
	edMusicStatus.masterTempo = 0x100;
	edMusicStatus.endOfSongCallback = (edMusicEndOfSongCallback)0x0;
	edMusicStatus.field_0xc = 0;
	edMusicStatus.field_0x18 = edMusicConfig.nbVoices;
	edMusicStatus.nbStreams = edMusicConfig.nbStreams;
	edMusicStatus.nbBanks = edMusicConfig.nbBanks;
	edMusicStatus.nbSongs = edMusicConfig.nbSongs;

	edDebugPrintf("edMusicInit :\n%d streams, %d banks, %d songs\n", edMusicConfig.nbStreams, edMusicConfig.nbBanks, edMusicConfig.nbSongs);
	pedMusicStreams = (ed_music_stream*)edMemAlloc(TO_HEAP(H_MAIN), edMusicStatus.nbStreams * sizeof(ed_music_stream));
	uVar3 = 0;
	if (edMusicStatus.nbStreams != 0) {
		do {
			_edMusicInitStream(&pedMusicStreams[uVar3], uVar3);
			uVar3 = uVar3 + 1;
		} while (uVar3 < edMusicStatus.nbStreams);
	}

	_pedMusicSongs = (ed_music_song*)edMemAlloc(TO_HEAP(H_MAIN), edMusicStatus.nbSongs * sizeof(ed_music_song));
	uVar3 = 0;
	if (edMusicStatus.nbSongs != 0) {
		iVar2 = 0;
		do {
			_edMusicInitSong(_pedMusicSongs + uVar3);
			uVar3 = uVar3 + 1;
		} while (uVar3 < edMusicStatus.nbSongs);
	}

	_pedMusicBanks = (_ed_music_bank*)edMemAlloc(TO_HEAP(H_MAIN), edMusicStatus.nbBanks * sizeof(_ed_music_bank));
	uVar3 = 0;
	if (edMusicStatus.nbBanks != 0) {
		iVar2 = 0;
		do {
			_edMusicInitBank(_pedMusicBanks + uVar3);
			uVar3 = uVar3 + 1;
		} while (uVar3 < edMusicStatus.nbBanks);
	}

	edMusicSetMasterVolume(0xffff);
	edMusicSetMasterTempo(0x100);
	_edMusicInit(&edMusicConfig);
	_edMusicSetOutputMode(edMusicStatus.outputMode);

	return;
}

