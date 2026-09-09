#ifndef ED_SOUND_PLAY_H
#define ED_SOUND_PLAY_H

#include "Types.h"

struct edsound_listener;
struct edsound_3d_data;

struct ed_sound_sample
{
	uint soundRamAddress;
	uint sampleRate;
	uint flags;
	uint dataSize;
	uint loopStartOffset;
	float loopEndOffset;
};

struct _ed_sound_stream
{
	int streamBufferId[2];
	int streamFileId;
	void* pMem;
	void* pDynamicData;

	int field_0x10;
	int field_0x14;
	float field_0x18;
};

PACK(struct VAGp {
	char magic[4]; /* VAGp magic. */
	uint versionBe; /* Big-endian VAG version. */
	uint reserved0;
	uint dataSizeBe; /* Big-endian size of ADPCM data following the header. */
	uint sampleRateBe; /* Big-endian sample rate in Hz. */
	char reserved1[12];
	char name[16];
});

PACK(struct adpcmBlock {
	byte predictorFilter;
	byte flags;
	byte data[14];
});

PACK(struct SoundFileData {
	VAGp header;
	adpcmBlock adpcm[];
});

PACK(
struct GlobalSound_FileData
{
	int sampleRate;
	int nbChannels;
	int interleaveBlockSize;
	int interleaveBlockCount;
	char field_0x10[64];
});

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

int _edSoundStreamInit(GlobalSound_FileData* pSoundData, _ed_sound_stream* pSoundStream, char* szPath, uint lsn, undefined8 param_5, uint fileSize);
void _edSoundStreamTerm(_ed_sound_stream* pSoundStream);

int edSoundStreamLoadA(_ed_sound_stream* pSoundStream, GlobalSound_FileData* pSoundData, char* szPath, undefined8 param_4);
int edSoundStreamLoadB(_ed_sound_stream* pSoundStream, GlobalSound_FileData* pSoundData, uint lsn, undefined8 param_4, uint filesize);
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

uint edSoundStreamCreate_00284500(float priority, _ed_sound_stream* pSoundStream);
bool edSoundStream_00284650(uint index);
void edSoundStream_00283650(uint index);
uint edSoundStream_00283f70(uint index);
float edSoundStreamGetPlaybackTime(uint index);
uint edSoundSamplePlay(float priority, ed_sound_sample* pSample);
void edSoundInstanceSetVolume(float param_1, uint soundInstanceId);
void edSoundInstanceSetPause(uint soundInstanceId, int bPaused);
void edSoundInstanceSet3DData(uint soundInstanceId, edsound_3d_data* pData, uint* existingSoundIDPtr, uint param_4);
void edSoundInstanceSetFrequency(float frequency, uint soundInstanceId);
void edSoundInstanceSetUserData(uint soundInstanceId, void* pUserData);
void edSoundSamplePlayAlt(float priority, ed_sound_sample* pSoundSample);

void* edSound_0x002840e0(uint soundInstanceId);

// SOUND_

struct SOUND_StreamInfo
{
	undefined4 field_0x0;
	byte b5;
	byte b6;
	byte b7;
	byte b8;
	byte b9;
	byte b10;
	byte b11;
	byte b12;
	int int1;
	byte b17;
	byte playing;
	byte b18;
	byte b19;
	byte b20;
	byte b21;
	ushort field_0x16;
	undefined4 field_0x18;
	undefined4 field_0x1c;
	int field_0x20;
	int int4;
};

int SOUND_InitIOP(void);
int SOUND_SetMaxStreamLimit(uint nbStreams);
int SOUND_FindFreeStream(void);
void SOUND_AllocateStreamBuffer(int streamBufferId, void* pMem, uint size);
void SOUND_CreateFileInfoFromLsn(ushort streamFileId, uint lsn, uint fileSize, undefined8 param_4, undefined8 param_5);
int SOUND_GetStreamInfo(uint streamID, SOUND_StreamInfo* outStreamInfo);
void SOUND_CloseStreamBuffer(int streamBufferId);
void SOUND_FreeFileID(int streamFileId);

extern byte SOUND_STREAM_STATUS[48];

// End SOUND_

extern edCSoundGlobalParams edSoundGlobalParams;
extern edCSoundParam edSoundParam;
extern ed_sound_voice_position* pedSoundVoicePosition;
extern edsound_listener edSoundListenerDefault;
extern ED_SOUND_3D_DATA edSound3DDataDefault;

#endif // ED_SOUND_PLAY_H
