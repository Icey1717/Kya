#ifndef ED_SOUND_PLAY_H
#define ED_SOUND_PLAY_H

#include "Types.h"

struct edsound_listener;

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
	void* pDynamicData;
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
	char field_0x10[64];
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
	edF32VECTOR3 field_0x24;
	edF32VECTOR3 field_0x30;
	edF32VECTOR3 field_0x3c;
	edF32VECTOR3 field_0x48;
	AUDIO_MODE outputMode;
	undefined4 field_0x58;
	float field_0x5c;
	float field_0x60;
	float field_0x64;
	float field_0x68;
	void* field_0x6c;
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

struct edCSoundParam
{
	uint nbVoices;
	uint field_0x4;
};


struct _ed_sound_bit_array_handle
{
	int voiceIndex;
	int field_0x4;
	int field_0x8;
};

struct ed_sound_voice_position
{
	undefined4 field_0x0;
	int position;
};

struct ED_SOUND_3D_DATA
{
	edF32VECTOR3 field_0x0;
	edF32VECTOR3 field_0xc;
	float field_0x18;
	float field_0x1c;
	float field_0x20;
	byte field_0x24;
};

int _edSoundStreamInit(GlobalSound_FileData* pSoundData, _ed_sound_stream* pSoundStream, char* szPath, ulong param_4, undefined8 param_5, undefined8 param_6);
void _edSoundStreamTerm(_ed_sound_stream* pSoundStream);

int edSoundStreamLoadA(_ed_sound_stream* pSoundStream, GlobalSound_FileData* pSoundData, char* szPath, undefined8 param_4);
int edSoundStreamLoadB(_ed_sound_stream* pSoundStream, GlobalSound_FileData* pSoundData, ulong param_3, undefined8 param_4, undefined8 param_5);
void edSoundStreamFree(_ed_sound_stream* pSoundStream);

void edSoundFlush();

void edSoundTerminateAllInstances(void);

uint edSoundInstanceStop(uint instanceId);
void edSoundInstanceFade(float volume, float frequency, float targetVolume, float targetFrequency, float duration, uint soundId);
void edSoundInstanceFadeTypeSet(uint soundId, uint fadeType);

int _edSoundSampleLoad(SoundFileData* soundFileData, ed_sound_sample* pSample, ulong flags);
int edSoundSampleLoad(char* pSoundFile, ed_sound_sample* pSoundSample, ulong flags);

void _edSoundWaitAllSoundDataLoaded(void);
void edSoundWaitAllSoundDataLoaded(void);


extern uint _edSoundAllocatedVoices[2];
void _edSoundInstanceSetFree(struct ed_sound_instance* pInstance);

// Inline new helpers for voice count, no equivalent function in ghidra
inline uint EdSoundVoiceCountFromFlags(uint flags)
{
	return (flags & 0x28) != 0 ? 2 : 1;
}

inline void EdSoundVoiceSetAllocated(uint voiceIndex)
{
	_edSoundAllocatedVoices[voiceIndex >> 5] |= 1u << (voiceIndex & 0x1f);
}

inline void EdSoundVoiceSetFree(uint voiceIndex)
{
	_edSoundAllocatedVoices[voiceIndex >> 5] &= ~(1u << (voiceIndex & 0x1f));
}

void edSoundSampleLoadWait(char* pSoundFile, ed_sound_sample* pSoundSample, ulong flags);

AUDIO_MODE edSoundOutputModeGet(void);
void edSoundOutputModeSet(AUDIO_MODE newMode);
void _edSoundEndFlush(uint nbFlush);

void edSoundSetMasterVolume(float newVolume);

int _edSoundSetVoiceRange(uint nbVoices, uint param_2);

uint _edSoundVoiceGetFirstFree(_ed_sound_bit_array_handle* pSoundBitArrayHandle);
uint _edSoundVoiceGetFirstFreeFromPointer(uint* param_1, _ed_sound_bit_array_handle* pSoundBitArrayHandle);

void edSoundSetFinishedInstancesCallback(edSoundFinishedInstancesCallback pCallback);

void edSoundPrepareReverbTypes(uint nbTypes, ED_SOUND_REVERB_TYPE* aTypes);

int _edSoundSampleFree(ed_sound_sample* pSoundSample);
int edSoundSampleFree(ed_sound_sample* pSoundSample);

uint _edSoundMemFree(void* pMem);

bool edSoundAreAllSoundDataLoaded();

void edSoundSetListener(edsound_listener* pListener);

// SOUND_
int SOUND_SetMaxStreamLimit(uint nbStreams);

extern edCSoundGlobalParams edSoundGlobalParams;
extern edCSoundParam edSoundParam;
extern ed_sound_voice_position* pedSoundVoicePosition;
extern edsound_listener edSoundListenerDefault;
extern ED_SOUND_3D_DATA edSound3DDataDefault;

#endif // ED_SOUND_PLAY_H
