#ifndef ED_MUSIC_H
#define ED_MUSIC_H

#include "Types.h"

struct ed_music_config
{
	uint nbVoices;
	uint nbStreams;
	uint nbSongs;
	uint nbBanks;
};

typedef void (*edMusicEndOfSongCallback)(uint);

struct edCMusicStatus
{
	AUDIO_MODE outputMode;
	uint masterVolume;
	uint masterTempo;
	undefined4 field_0xc;
	edMusicEndOfSongCallback endOfSongCallback;
	undefined field_0x14;
	undefined field_0x15;
	undefined field_0x16;
	undefined field_0x17;
	uint field_0x18;
	uint nbStreams;
	uint nbSongs;
	uint nbBanks;
	undefined field_0x28;
	undefined field_0x29;
	undefined field_0x2a;
	undefined field_0x2b;
	ED_SOUND_REVERB_TYPE reverbType;
	float field_0x30;
	float field_0x34;
	float field_0x38;
	float field_0x3c;
};

struct ed_music_song
{
	void* field_0x0;
	uint flags;
};

struct _ed_music_bank
{
	void* field_0x0;
	void* field_0x4;
	uint flags;
};

struct ed_music_stream
{
	ed_music_song* pSong;
	undefined4 field_0x4;
	uint index;
	int field_0xc;
	undefined4 field_0x10;
	undefined4 field_0x14;
	uint volume;
	uint tempo;
	uint targetVolume;
	uint targetTempo;
	int aChannelVolumes[16];
	int aTargetChannelVolumes[16];
	uint fadeFlags;
	float volumeTempoFadeTime;
	float channelFadeTime;
	undefined field_0xb4;
	undefined field_0xb5;
	undefined field_0xb6;
	undefined field_0xb7;
	undefined field_0xb8;
	undefined field_0xb9;
	undefined field_0xba;
	undefined field_0xbb;
	undefined4 field_0xbc;
	undefined4 field_0xc0;
};

ed_music_config* edMusicGetConfig(void);
void _edMusicInit(ed_music_config* pConfig);
void edMusicInit(void);

void edMusicSetMasterVolume(uint newVolume);
void _edMusicSetOutputMode(AUDIO_MODE newMode);

void edMusicSetMasterTempo(uint newTempo);
void edMusicSetOutputMode(AUDIO_MODE newMode);

void edMusicSetEndOfSongCallback(edMusicEndOfSongCallback pCallback);

uint edMusicFlush(void);

void edMusicSetReverb(float param_1, float param_2, float param_3, float param_4, ED_SOUND_REVERB_TYPE param_5);

void edMusicStopAllSongs(void);

uint edMusicBankInstallNoWait(void* pAdpcm, void* pFileData, uint dataSize, uint size);
void _edMusicBankInstallNoWait(_ed_music_bank* pBank, void* param_2, void* pAdpcm, uint size, uint dataSize);

void _edMusicSongRemove(ed_music_song* pSong);
void edMusicSongRemove(int songIndex);

void _edMusicBankRemove(_ed_music_bank* pMusicBank);
void edMusicBankRemove(int bankIndex);

bool _edMusicAreAllMusicDataLoaded(void);
bool edMusicAreAllMusicDataLoaded();

void edMusicStreamChannelFade(float fadeTime, int musicStreamIndex, int channel, uint otherVolume, int volume);

extern edCMusicStatus edMusicStatus;
extern ed_music_stream* pedMusicStreams;

extern ed_music_stream* pedMusicStreams;
extern ed_music_song* _pedMusicSongs;
extern _ed_music_bank* _pedMusicBanks;

#endif // ED_MUSIC_H