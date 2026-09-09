#ifndef AUDIO_H
#define AUDIO_H

#include "Types.h"
#include "List.h"
#include "edSound/edSoundPlay.h"
#include "edSound/edSoundInstance.h"
#include "LargeObject.h"

class CActor;
class CAudioManager;
class edCBankBufferEntry;
class CSoundWind;

struct edCEventMessage;

struct SoundEntry
{
	int selector;
	char* fileName;
	void* pNode;
};

class CSoundBase
{
public:
	CSoundBase();

	virtual uint Play(uint soundInstanceId, uint otherId, edsound_3d_data* p3dData, void* param_5, uint* param_6) = 0;
	virtual uint Stop(uint instanceId);
	virtual bool IsLooping() = 0;

	uint PlayAlt(float param_1, float param_2, float param_3, uint soundId, uint param_6);

	int setupIntFieldA;
	SoundEntry aSoundEntries[6];
	ed_sound_sample edSoundSample;
	float field_0x6c;
	float field_0x70;
	float field_0x74;
	float field_0x78;
	float priority;
	uint field_0x80;
};

class CSound : public CSoundBase
{
public:
	virtual uint Play(uint soundInstanceId, uint otherId, edsound_3d_data* p3dData, void* pUserData, uint* param_6);
	virtual bool IsLooping();

	void Create(ByteCode* pByteCode);
	void FadeTo(float param_1, float param_2, float param_3, uint instanceId);
	void SetPause(uint soundInstanceId, int bPaused);
	void InitializeFromSample(float param_1, float param_2, float param_3, float param_4, float param_5, float priority, ed_sound_sample* soundInfoObj, uint param_9);

	void SetVolume(float param_1, uint soundInstanceId);

	uint field_0x84;
	uint field_0x88;
};

class CSoundSample : public CSound
{
public:
	virtual uint Play(uint soundInstanceId, uint otherId, edsound_3d_data* p3dData, void* pUserData, uint* pOutId);
	virtual bool IsLooping() { return IsLooping(-1); }
	virtual bool IsLooping(int soundInstanceId);
};

struct CSoundStream : public CSoundSample
{
public:
	virtual uint Play(uint soundInstanceId, uint otherId, edsound_3d_data* p3dData, void* pUserData, uint* param_6) { IMPLEMENTATION_GUARD(); }
	virtual uint Stop(uint instanceId);

	undefined4 field_0x8c;
};

struct SOUND_SAMPLE_REF
{
	union {
		CSound* pSample;
		int index;
	};

	void Init();
};

struct SOUND_STREAM_REF
{
	union {
		strd_ptr(CSoundStream) pStream;
		int index;
	};

	void Init();
};

class CWayPoint;

class CSoundAmbiance
{
public:
	CSoundAmbiance();
	void Init();

	SOUND_SAMPLE_REF field_0x0;
	SOUND_SAMPLE_REF field_0x4;
	SOUND_STREAM_REF field_0x8;
	float field_0xc;
	float field_0x10;
	S_STREAM_REF<CWayPoint> wayPointRef;
	edsound_3d_data soundPosData;

	undefined field_0x40;

	CSound* field_0x4c;
	uint field_0x50;
	edsound_3d_data* field_0x54;

	uint field_0x60;
	float field_0x68;
	float field_0x6c;
};

class CAmbiance
{
public:
	virtual void Play(float param_1);

	CSoundAmbiance* aSoundAmbiance;
	uint nbSoundAmbiance;
	float field_0x8;
	float field_0xc;
	undefined4 field_0x10;
	int field_0x14;
	float field_0x18;
	float field_0x1c;
};

class CMusicAmbiance : public CAmbiance
{
public:
	CMusicAmbiance();

	void Add(ByteCode* pByteCode);
};

class CMusic
{
public:
	int bankIndex;
	int songIndex;
	uint flags;
	float field_0xc;
};

struct s_sound_3d_data
{
	edsound_3d_data edSoundData;
	edF32VECTOR3 field_0x28;
	int field_0x34;
	CActor* pActor;
	uint boneId;
};

class CMusicManager
{
public:
	struct s_music_cell
	{
		CMusic* pMusic;
		float priority;
		uint flags;
		uint stateFlags;
		float volume;
		float gain;
		float targetVolume;
		float targetGain;
		float initialVolume;
		float initialGain;
		float transitionDuration;
		float transitionRemaining;
		float stopDuration;
		float stopRemaining;
		int streamIndex;
		float field_0x3c;
		void (*pCallback)();
		CMusic* pQueuedMusic;
	};

	CMusicManager();

	uint _ManageCell(float param_1, float param_2, CAudioManager* pAudioManager, CMusicManager::s_music_cell* pMusicCell, int* pStart, int* pStop, int* pClearCell, int* pResume, int* pForceStop, float* pGain, float* pGainSource);
	void Manage(CAudioManager* pAudioManager);
	bool IsMusic(int handle, CMusic* pMusic);
	void CancelStop(float fadeInTime, int handle);
	void Stop(float fadeOutTime, float param_2, int handle);
	int Start(float priority, float initialVolume, float fadeInTime, float fadeOutTime, CMusic* pMusic, uint flags);

	s_music_cell aMusicCells[10];
	byte aStreamUsed[10];
};

void MusicStop(float fadeOutTime, int callback, CMusicManager* pMusicManager, int handle);

struct SoundSampleEntry;

struct PendingSoundPlay
{
	CDoubleLinkedNode<SoundSampleEntry>* pNode;
	CSoundStream* pSoundStream;
	float priority;
	edsound_3d_data* p3dData;
	int* field_0x10;
	uint* field_0x14;
	uint* field_0x18;
};

struct SoundSampleEntry
{
	bool LoadStreamCh();

	int mode;
	int nbCount;
	bool(SoundSampleEntry::* loadFunc)(void);
	char fileName[12];
	undefined field_0x20;
	undefined field_0x21;
	undefined field_0x22;
	undefined field_0x23;
	undefined field_0x24;
	undefined field_0x25;
	undefined field_0x26;
	undefined field_0x27;
	undefined field_0x28;
	undefined field_0x29;
	undefined field_0x2a;
	undefined field_0x2b;
	undefined field_0x2c;
	undefined field_0x2d;
	undefined field_0x2e;
	undefined field_0x2f;
	undefined field_0x30;
	undefined field_0x31;
	undefined field_0x32;
	undefined field_0x33;
	undefined field_0x34;
	undefined field_0x35;
	undefined field_0x36;
	undefined field_0x37;
	undefined field_0x38;
	undefined field_0x39;
	undefined field_0x3a;
	undefined field_0x3b;
	undefined field_0x3c;
	undefined field_0x3d;
	undefined field_0x3e;
	undefined field_0x3f;
	undefined field_0x40;
	undefined field_0x41;
	undefined field_0x42;
	undefined field_0x43;
	undefined field_0x44;
	undefined field_0x45;
	undefined field_0x46;
	undefined field_0x47;
	undefined field_0x48;
	undefined field_0x49;
	undefined field_0x4a;
	undefined field_0x4b;
	undefined field_0x4c;
	undefined field_0x4d;
	undefined field_0x4e;
	undefined field_0x4f;
	undefined field_0x50;
	undefined field_0x51;
	undefined field_0x52;
	undefined field_0x53;
	ed_sound_sample edSoundSample;
	CDoubleLinkedList<PendingSoundPlay> field_0x6c;
	uint field_0x74;
};

struct MusicSomething_0x8
{
	void* pAdpcm;
	uint size;
};

struct SOUND_SPATIALIZATION_PARAM
{
	float* data;
};

class CAudioManager : public CObjectManager
{
public:
	CAudioManager();

	virtual void Level_Init();
	virtual void Level_Term();
	virtual void Level_AddAll(struct ByteCode* pByteCode);
	virtual void Level_ClearAll();

	virtual void Level_Manage();
	virtual void Level_ManagePaused();

	virtual void Level_Reset();
	virtual void Level_CheckpointReset();
	virtual void Level_PauseChange(bool bPaused);

	void ClearInternalData();

	CSoundSample* GetSound(uint id)
	{
		return nullptr;
	}

	void SetMusicVolume(float newVolume);
	void SetSfxVolume(float newVolume);

	void SetOutputMode(AUDIO_MODE newMode);

	void PlayCombatMusic();
	void StopCombatMusic();

	bool EnsureSoundMemoryAvailable(uint param_2, uint size);
	void WillLoadFileFromBank(edCBankBufferEntry* pBankBuffer);

	void AddMusics(ByteCode* pByteCode);
	void AddSceneData(ByteCode* pByteCode);
	void LoadGlobalSoundFunc_00184a70(ByteCode* pByteCode);
	void AddSoundStreams(ByteCode* pByteCode);

	void FUN_00182c10();
	void ManageSoundSamples();

	char* GetStreamFileNameFromIndex_00184a40(int index);
	GlobalSound_FileData* GetSoundFileDataFromIndex_00184a10(int index);
	int GetAmbianceIndex_001819b0();
	int GetMusicId();
	void ReleaseSound3DData(edsound_3d_data* pData);

	CSoundWind* GetWindSound(int index);

	void SetAmbiance(uint ambianceId);
	void SetMusic(uint index);

	void FUN_00182db0(float param_1, undefined4 param_3, undefined4 param_4);
	void FUN_00184470();
	void FUN_00182da0(float param_1);
	edsound_3d_data* ObtainSound3DData(int param_2, CActor* pActor, uint boneId);

	void ReceiveEvent(edCEventMessage* pEventMessage, uint param_3, undefined8 param_4, uint param_5, uint* param_6);

	void ActivateCheckpoint();

	float field_0x68;
	int field_0x6c;

	int field_0x4;
	char* pString_0x1c;
	char** aLoadedSamplePaths;
	int field_0x20;
	int field_0x2c;
	int field_0x30;
	char* field_0x34;
	int nbMaxSamples;
	int field_0x44;
	int nbBanks;
	ed_sound_sample* aSamples;
	CDoubleLinkedNode<SoundSampleEntry>* aSoundStreamNodes;
	int* field_0x54;
	int* field_0x58;
	int* field_0x5c;
	int* aSongIndexes;
	int* aBankIndexes;
	MusicSomething_0x8* field_0x60;
	float field_0x64;
	CSoundSample* aSoundSamplesA;
	CSoundStream* aSoundStreams;
	uint field_0x78;
	int field_0x7c;
	CSound* aSoundSamplesB;
	_ed_sound_stream* aEdSoundStreams;
	CMusic* aMusic;
	CMusicAmbiance* aMusicAmbiance;
	int nbLoadedSamples;
	float musicVolume;
	float sfxVolume;
	float field_0xc4;
	float field_0xc8;
	float field_0xcc;

	GlobalSound_FileData* pGlobalSoundFileData;

	CDoubleLinkedList<CMusic*> musicActiveList;
	CDoubleLinkedList<CMusic*> musicFreeList;
	CDoubleLinkedNode<CMusic*> aMusicNodes[16];

	CMusicManager* field_0x38;
	edsound_listener* pSoundListener;

	byte field_0xd0;
	byte field_0xd1;
	byte bActive_0xd2;
	byte field_0xd3;
	byte field_0xd4;
	byte field_0xd5;

	int field_0xdc;
	int field_0xe0;
	float field_0xe4;
	int field_0xe8;
	int field_0xf0;
	int field_0xf4;
	float field_0xf8;
	int field_0xfc;
	CMusic* field_0x100;
	int field_0x104;
	int field_0x108;
	float field_0x10c;
	int field_0x110;
	CMusic* field_0x114;
	CMusic* pMusic;
	CMusic* pPrevMusic;

	float field_0x1e8[16];
	uint field_0x228[16];
	undefined4 field_0x268;
	CMusic* field_0x26c;
	int cellIndex_0x270;
	int count_0x274;
	int field_0x278;
	float field_0x27c;
	float field_0x280;
	float field_0x284;
	int field_0x288;
	int field_0x294;
	int field_0x298;
	float field_0x29c;
	float field_0x2a0;
	float field_0x2a4;
	int nbSoundWind;
	CSoundWind* aSoundWind;
	CMusic* field_0x2a8[3];

	ED_SOUND_REVERB_TYPE field_0x2b4;
	float field_0x2b8;
	float field_0x2bc;
	float field_0x2c0;

	int nbMusic;
	int nbMusicAmbiance;

	uint nbSoundDataNodes;
	CDoubleLinkedNode<s_sound_3d_data>* aSoundDataNodes;
	CDoubleLinkedList<s_sound_3d_data>* field_0xac;
	CDoubleLinkedList<s_sound_3d_data>* field_0xb0;

	int field_0xb4;
	int field_0xb8;
	int musicIndex;
	int activeAmbianceIndex;

	uint field_0x28c;
	CMusic* field_0x290;

	undefined4 field_0x2dc;
	undefined4 field_0x2e0;

	edF32VECTOR3 cameraPosition;
};

extern int NoAudio;

#endif // AUDIO_H
