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

#endif // ED_SOUND_PLAY_H
