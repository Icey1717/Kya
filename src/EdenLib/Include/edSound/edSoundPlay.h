#ifndef ED_SOUND_PLAY_H
#define ED_SOUND_PLAY_H

#include "Types.h"

struct ed_sound_sample
{
	uint soundRamAddress;
	uint sampleRate;
	uint flags;
	uint dataSize;
	uint loopStartOffset;
	uint loopEndOffset;
};

struct _ed_sound_stream
{
	int streamBufferId;
	int streamFileId;
	void* pMem;
};

struct VAGp {
	char magic[4]; /* VAGp magic. */
	uint versionBe; /* Big-endian VAG version. */
	uint reserved0;
	uint dataSizeBe; /* Big-endian size of ADPCM data following the header. */
	uint sampleRateBe; /* Big-endian sample rate in Hz. */
	char reserved1[12];
	char name[16];
};

struct adpcmBlock {
	byte predictorFilter;
	byte flags;
	byte data[14];
};

struct SoundFileData {
	VAGp header;
	adpcmBlock adpcm[];
};

struct GlobalSound_FileData
{

};

typedef void (*edSoundFinishedInstancesCallback)(struct ed_sound_instance_finished*, uint);

struct edCSoundGlobalParams
{
	float volume;
	undefined field_0x4;
	undefined field_0x5;
	undefined field_0x6;
	undefined field_0x7;
	undefined field_0x8;
	undefined field_0x9;
	undefined field_0xa;
	undefined field_0xb;
	undefined field_0xc;
	undefined field_0xd;
	undefined field_0xe;
	undefined field_0xf;
	undefined field_0x10;
	undefined field_0x11;
	undefined field_0x12;
	undefined field_0x13;
	undefined field_0x14;
	undefined field_0x15;
	undefined field_0x16;
	undefined field_0x17;
	undefined field_0x18;
	undefined field_0x19;
	undefined field_0x1a;
	undefined field_0x1b;
	undefined field_0x1c;
	undefined field_0x1d;
	undefined field_0x1e;
	undefined field_0x1f;
	undefined4 field_0x20;
	float field_0x24;
	float field_0x28;
	float field_0x2c;
	float field_0x30;
	float field_0x34;
	float field_0x38;
	float field_0x3c;
	float field_0x40;
	float field_0x44;
	undefined4 field_0x48;
	undefined4 field_0x4c;
	undefined4 field_0x50;
	AUDIO_MODE outputMode;
	undefined4 field_0x58;
	float field_0x5c;
	float field_0x60;
	float field_0x64;
	float field_0x68;
	undefined4 field_0x6c;
	edSoundFinishedInstancesCallback finishedInstancesCallback;
	float g_DesiredFrameTime_00483824;
	undefined field_0x78;
	undefined field_0x79;
	undefined field_0x7a;
	undefined field_0x7b;
	undefined field_0x7c;
	undefined field_0x7d;
	undefined field_0x7e;
	undefined field_0x7f;
};

int _edSoundStreamInit(GlobalSound_FileData* pSoundData, _ed_sound_stream* pSoundStream, char* szPath, ulong param_4, undefined8 param_5, undefined8 param_6);
void _edSoundStreamTerm(_ed_sound_stream* pSoundStream);

int edSoundStreamLoadA(_ed_sound_stream* pSoundStream, GlobalSound_FileData* pSoundData, char* szPath, undefined8 param_4);
int edSoundStreamLoadB(_ed_sound_stream* pSoundStream, GlobalSound_FileData* pSoundData, ulong param_3, undefined8 param_4, undefined8 param_5);
void edSoundStreamFree(_ed_sound_stream* pSoundStream);

void edSoundFlush();

uint edSoundInstanceStop(uint instanceId);

int _edSoundSampleLoad(SoundFileData* soundFileData, ed_sound_sample* pSample, ulong flags);

void _edSoundWaitAllSoundDataLoaded(void);
void edSoundWaitAllSoundDataLoaded(void);

void edSoundSampleLoadWait(char* pSoundFile, ed_sound_sample* pSoundSample, ulong flags);

void edSoundSetMasterVolume(float newVolume);
void edMusicSetMasterVolume(uint newVolume);
void edMusicSetMasterTempo(uint newTempo);

AUDIO_MODE edSoundOutputModeGet(void);
void edSoundOutputModeSet(AUDIO_MODE newMode);

void edMusicSetOutputMode(AUDIO_MODE newMode);

extern edCSoundGlobalParams edSoundGlobalParams;

#endif // ED_SOUND_PLAY_H
