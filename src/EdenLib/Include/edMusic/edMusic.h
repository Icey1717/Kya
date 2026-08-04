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

typedef int ED_SOUND_REVERB_TYPE;

struct edCMusicStatus
{
	AUDIO_MODE outputMode;
	uint masterVolume;
	uint masterTempo;
	undefined4 field_0xc;
	void* endOfSongCallback;
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
	undefined field_0x0;
	undefined field_0x1;
	undefined field_0x2;
	undefined field_0x3;
	uint flags;
};

struct _ed_music_bank
{
	undefined field_0x0;
	undefined field_0x1;
	undefined field_0x2;
	undefined field_0x3;
	undefined field_0x4;
	undefined field_0x5;
	undefined field_0x6;
	undefined field_0x7;
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
	undefined field_0x20;
	undefined field_0x21;
	undefined field_0x22;
	undefined field_0x23;
	undefined field_0x24;
	undefined field_0x25;
	undefined field_0x26;
	undefined field_0x27;
	int aChannelVolumes[16];
	undefined field_0x68;
	undefined field_0x69;
	undefined field_0x6a;
	undefined field_0x6b;
	undefined field_0x6c;
	undefined field_0x6d;
	undefined field_0x6e;
	undefined field_0x6f;
	undefined field_0x70;
	undefined field_0x71;
	undefined field_0x72;
	undefined field_0x73;
	undefined field_0x74;
	undefined field_0x75;
	undefined field_0x76;
	undefined field_0x77;
	undefined field_0x78;
	undefined field_0x79;
	undefined field_0x7a;
	undefined field_0x7b;
	undefined field_0x7c;
	undefined field_0x7d;
	undefined field_0x7e;
	undefined field_0x7f;
	undefined field_0x80;
	undefined field_0x81;
	undefined field_0x82;
	undefined field_0x83;
	undefined field_0x84;
	undefined field_0x85;
	undefined field_0x86;
	undefined field_0x87;
	undefined field_0x88;
	undefined field_0x89;
	undefined field_0x8a;
	undefined field_0x8b;
	undefined field_0x8c;
	undefined field_0x8d;
	undefined field_0x8e;
	undefined field_0x8f;
	undefined field_0x90;
	undefined field_0x91;
	undefined field_0x92;
	undefined field_0x93;
	undefined field_0x94;
	undefined field_0x95;
	undefined field_0x96;
	undefined field_0x97;
	undefined field_0x98;
	undefined field_0x99;
	undefined field_0x9a;
	undefined field_0x9b;
	undefined field_0x9c;
	undefined field_0x9d;
	undefined field_0x9e;
	undefined field_0x9f;
	undefined field_0xa0;
	undefined field_0xa1;
	undefined field_0xa2;
	undefined field_0xa3;
	undefined field_0xa4;
	undefined field_0xa5;
	undefined field_0xa6;
	undefined field_0xa7;
	uint field_0xa8;
	undefined field_0xac;
	undefined field_0xad;
	undefined field_0xae;
	undefined field_0xaf;
	float field_0xb0;
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

extern edCMusicStatus edMusicStatus;

#endif // ED_MUSIC_H