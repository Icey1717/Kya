#ifndef _CINEMATICMANAGER_H
#define _CINEMATICMANAGER_H

#include "Types.h"
#include "LargeObject.h"
#include "Actor.h"
#include "edBank/edBankBuffer.h"

#ifdef PLATFORM_PS2
#include <libvu0.h>
#endif
#include "ScenaricCondition.h"
#include "TranslatedTextData.h"
#include "edSound/edSoundPlay.h"

#include "EdenLib/edCinematic/Sources/Cinematic.h"

#define IMPLEMENTATION_GUARD_LIP(x)

struct ed_g2d_manager;
struct ed_g3d_manager;
struct CActorCinematic;
struct CActor;

#define CINEMATIC_LOAD_STATE_COMPLETE 4

#define CINEMATIC_FLAG_ONE_SHOT_GATED				0x00000001 // One-shot mode enabled; replay is controlled by runtime lock state.
#define CINEMATIC_FLAG_ALLOW_SKIP_INPUT				0x00000002 // Allow input-driven skip while playing.
#define CINEMATIC_FLAG_SKIP_NEEDS_0x1400			0x00000004 // Skip path additionally requires runtime skip-context bits.
#define CINEMATIC_FLAG_LOOPING_LOADING				0x00000008 // Loop/reload mode for the cinematic stream.
#define CINEMATIC_FLAG_HERO_MSG_82_83				0x00000010 // Send hero messages 0x82 (start) / 0x83 (stop).
#define CINEMATIC_FLAG_PRERESET_ON_CHECKPOINT		0x00000020 // Set in some cinematics, and checked in Level_PreReset/Level_SetupInternalData to conditionally preserve runtime notify-triggered state.
#define CINEMATIC_FLAG_SAVE_CONTEXT					0x00000040 // Include this cinematic in Level_SaveContext (BLCI).
#define CINEMATIC_FLAG_APPLY_END_ON_STOP			0x00000080 // Apply end state via final timeslice when stopped early.
#define CINEMATIC_FLAG_KEEP_TIME_ON_START			0x00000100 // Preserve totalCutsceneDelta on Start().
#define CINEMATIC_FLAG_CUTSCENE_BANDS				0x00000200 // Marks a blocking cutscene / band state.
#define CINEMATIC_FLAG_HIDE_FRONTEND				0x00000400 // Disable frontend UI during playback.
#define CINEMATIC_FLAG_AUTO_TRY_START				0x00000800 // Auto-attempt start from manager-side checks.
#define CINEMATIC_FLAG_FORCE_INTERPOLATE			0x00001000 // Force entry through CS_Interpolate.
#define CINEMATIC_FLAG_RELATIVE_TRANSFORM			0x00002000 // Build cinematic transform relative to actor refs.
#define CINEMATIC_FLAG_ALT_ACTOR_INIT_MODE			0x00004000 // Use alternate actor initialization path.
#define CINEMATIC_FLAG_REMAP_ACTORREFB_HASH			0x00008000 // Remap actorRefB hash from current actor context.
#define CINEMATIC_FLAG_SET_HERO_0x400000			0x00010000 // Apply hero actor flag 0x400000 while active.
#define CINEMATIC_FLAG_IGNORE_BASE_SECTOR			0x00040000 // Bypass base-sector filtering.
#define CINEMATIC_FLAG_AUDIO_MODE_A					0x00080000 // Audio handling mode A during playback.
#define CINEMATIC_FLAG_AUDIO_MODE_B					0x00200000 // Audio handling mode B during playback.
#define CINEMATIC_FLAG_RECHECK_CONDITION_ON_CLEAR	0x00100000 // Re-run cond_0x248 in Level_ClearAll().
#define CINEMATIC_FLAG_MESH_TRANSFORM_OPTION		0x00400000 // Enable special cinematic mesh-transform behavior.
#define CINEMATIC_FLAG_FORCE_ACTOR_0x200000			0x00800000 // Force actor update path (legacy 0x200000 behavior).
#define CINEMATIC_FLAG_START_PAUSED_TIME			0x01000000 // Start with timeline paused.
#define CINEMATIC_FLAG_PRELOAD_BANK					0x02000000 // Preload/load bank data before triggering.
#define CINEMATIC_FLAG_IGNORE_ZONE_C_GATE			0x04000000 // Ignore zone C gate when deciding to start.
#define CINEMATIC_FLAG_LOCALIZED_CINE_PATH			0x08000000 // Use localized cinematic path selection.
#define CINEMATIC_FLAG_KEEP_0x20_ON_RESET			0x10000000 // Keep runtime notify bit during PreReset/Setup resets.
#define CINEMATIC_FLAG_GAMESTATE_80					0x20000000 // Contributes to GAME_CUTSCENE_80 mask.

#define CINEMATIC_FLAG_AUDIO_MODE_MASK				(CINEMATIC_FLAG_AUDIO_MODE_A | CINEMATIC_FLAG_AUDIO_MODE_B) // Composite audio mode bits.
#define CINEMATIC_FLAG_GAME_CUTSCENE_80_MASK		(CINEMATIC_FLAG_CUTSCENE_BANDS | CINEMATIC_FLAG_GAMESTATE_80) // Bits that set GAME_CUTSCENE_80.
#define CINEMATIC_RESET_SECTOR_FLAGS				(CINEMATIC_FLAG_CUTSCENE_BANDS | CINEMATIC_FLAG_SET_HERO_0x400000 | CINEMATIC_FLAG_IGNORE_BASE_SECTOR | CINEMATIC_FLAG_AUDIO_MODE_A) // Flags restored on sector/reset path.

#define CINEMATIC_RUNTIME_FLAG_HAS_AUDIO_TRACK			0x00000001 // Runtime: cinematic has an audio track handle.
#define CINEMATIC_RUNTIME_FLAG_HAS_CAMERA_INTERFACE		0x00000002 // Runtime: cinematic requested camera interface.
#define CINEMATIC_RUNTIME_FLAG_STOP_CALLED_THIS_FRAME	0x00000004 // Runtime: Stop() executed this frame.
#define CINEMATIC_RUNTIME_FLAG_CONDITION_BLOCKED		0x00000008 // Runtime: condition gate currently blocks trigger.
#define CINEMATIC_RUNTIME_FLAG_CIN_DATA_INITIALIZED		0x00000010 // Runtime: cinFileData.Initialize() has run.
#define CINEMATIC_RUNTIME_FLAG_NOTIFY_TRIGGERED			0x00000020 // Runtime: set by NotifyCinematic trigger message.
#define CINEMATIC_RUNTIME_FLAG_TIME_PAUSED				0x00000040 // Runtime: timeline pause toggle.
#define CINEMATIC_RUNTIME_FLAG_ACTIVE					0x00000080 // Runtime: active/pending trigger latch.
#define CINEMATIC_RUNTIME_FLAG_PENDING_STOP				0x00000100 // Runtime: stop requested, to be processed.
#define CINEMATIC_RUNTIME_FLAG_ZONE_TRIGGER_LATCH		0x00000200 // Runtime: zone trigger latch while actor remains inside.
#define CINEMATIC_RUNTIME_FLAG_ONE_SHOT_LOCKED			0x00000400 // Runtime: one-shot lock bit (persisted in save context).
#define CINEMATIC_RUNTIME_FLAG_FORCED_TRIGGER			0x00000800 // Runtime: forced trigger path.
#define CINEMATIC_RUNTIME_FLAG_SAVE_CONTEXT_0x10		0x00001000 // Runtime: save-context extra bit 0x10.
#define CINEMATIC_RUNTIME_FLAG_CLEARALL_DEFER			0x00002000 // Runtime: defer restart during clear-all pass.

#define CINEMATIC_RUNTIME_FLAGS_START_BLOCKERS		(CINEMATIC_RUNTIME_FLAG_NOTIFY_TRIGGERED | CINEMATIC_RUNTIME_FLAG_CONDITION_BLOCKED) // Runtime blockers for normal one-shot starts.
#define CINEMATIC_RUNTIME_FLAGS_SKIP_CONTEXT_MASK	(CINEMATIC_RUNTIME_FLAG_ONE_SHOT_LOCKED | CINEMATIC_RUNTIME_FLAG_SAVE_CONTEXT_0x10) // Runtime skip-context gate bits.
#define CINEMATIC_RUNTIME_RESET_FLAGS				(CINEMATIC_RUNTIME_FLAG_STOP_CALLED_THIS_FRAME | CINEMATIC_RUNTIME_FLAG_CIN_DATA_INITIALIZED | CINEMATIC_RUNTIME_FLAG_NOTIFY_TRIGGERED | CINEMATIC_RUNTIME_FLAG_TIME_PAUSED | CINEMATIC_RUNTIME_FLAG_PENDING_STOP | CINEMATIC_RUNTIME_FLAG_ZONE_TRIGGER_LATCH | CINEMATIC_RUNTIME_FLAG_ONE_SHOT_LOCKED | CINEMATIC_RUNTIME_FLAG_CLEARALL_DEFER) // Runtime bits cleared by reset/pre-reset.
#define CINEMATIC_RUNTIME_RESET_FLAGS_KEEP_NOTIFY	(CINEMATIC_RUNTIME_RESET_FLAGS & ~CINEMATIC_RUNTIME_FLAG_NOTIFY_TRIGGERED) // Same as above, but preserve notify-triggered.

#define CINEMATIC_FLAG_ACTIVE CINEMATIC_RUNTIME_FLAG_ACTIVE // Backward-compatible alias for runtime active bit.

#define CINEMATIC_MESSAGE_RECEPTACLE_CHANGED 0x11

class CBWCinActor : public edCinActorInterface
{
public:
	virtual bool Initialize();
	virtual bool SetVisibility(bool bVisible);
	virtual bool OnFrameDirected();
	virtual bool SetPos(float newX, float newY, float newZ);
	virtual bool SetHeadingEuler(float x, float y, float z, bool param_5);
	virtual bool SetHeadingQuat(float x, float y, float z, float w);
	virtual bool SetScale(float x, float y, float z);
	virtual bool SetAnim(edCinActorInterface::ANIM_PARAMStag* pTag);
	virtual bool SetParticles(float param_1, edCinActorInterface::PARTICLE_PARAMStag* pTag);
	virtual bool SetSubtitle(float param_1, edCinSourceSubtitleI::SUBTITLE_PARAMStag* pParams);
	virtual bool SetMessage(float param_1, edCinActorInterface::MESSAGE_PARAMStag* pTag);
	virtual bool Shutdown();

	void SetupTransform(edF32VECTOR4* position, edF32VECTOR4* heading, edF32VECTOR4* scale, ed_g3d_manager* pMeshManager);

	CActor* pParent;
	int field_0x8;

	edF32VECTOR4 nextPos;
	edF32VECTOR4 position;
	edF32VECTOR4 heading;
	edF32VECTOR4 scale;
	CActorAlternateModel alternateModel;
	ed_g3d_manager* pAltModelManager;
};

class CBWCinCam : public edCinCamInterface
{
public:
	virtual bool Activate();
	virtual bool Initialize(bool param_2, uint* flags);
	virtual bool SetFov(float fov);
	virtual bool SetPos(float x, float y, float z);
	virtual bool SetHeadingQuat(float x, float y, float z, float w);
	virtual bool SetHeadingEuler(float x, float y, float z, bool param_5);
	virtual bool Shutdown() { return true; }
};

class CBWCinSourceSubtitle : public edCinSourceSubtitleI
{
public:
	virtual bool Create(char* pFileName, long param_3);
	virtual bool Init() { return true; }
	virtual bool SetPos(float x, float y, float z);
	virtual bool SetSubtitle(float keyTime, SUBTITLE_PARAMStag* pTag);
	virtual bool Shutdown() { return true; }
	virtual bool Destroy() { return true; }

	edF32VECTOR4 position;
};

class CBWCinSourceAudio : public edCinSourceAudioI
{
public:
	CBWCinSourceAudio();

	virtual bool Create(char* pFileName);
	virtual bool Play();
	virtual bool Stop();
	virtual bool GetTime() { IMPLEMENTATION_GUARD(); }
	virtual bool Destroy();
	virtual float Func_0x1c(int audioTrackId);

	void SetAudioTrack(int audioTrackId);

	uint soundInstanceId;
	float field_0x8;
	float floatFieldA;
	GlobalSound_FileData* pGlobalSoundFileData;
	_ed_sound_stream soundStream;
	_ed_sound_stream* pSoundStream;
	byte field_0x38;
	byte field_0x39;
	undefined field_0x3a;
	undefined field_0x3b;
};

class CBWitchCin : public edCinGameInterface {
public:

	virtual char* GetResource(edResCollection::RES_TYPE type1, bool type2, const char* fileName, int* bufferLengthOut);
	virtual bool ReleaseResource(byte, bool, void*);

	virtual bool CreateActor(edCinActorInterface** ppActorInterface, edCinGameInterface::ACTORV_CREATIONtag* const pTag);
	virtual bool GetActor(edCinActorInterface** ppActorInterface, uint hashCode, edCinGameInterface::ACTORV_CREATIONtag* const pTag);
	virtual bool ReleaseActor(edCinActorInterface*);

	virtual bool CreateScenery(edCinSceneryInterface** ppActorInterface, const edCinGameInterface::SCENERY_CREATIONtag* pTag);
	virtual bool ReleaseScenery(edCinSceneryInterface*);

	virtual bool GetCamera(edCinCamInterface** pCinCamInterface, const edCinCamInterface::CAMERA_CREATIONtag*);
	virtual bool ReleaseCamera(edCinCamInterface*);

	virtual bool GetSourceAudioInterface(edCinSourceAudioI** ppSourceAudioInterface);
	virtual bool ReleaseSourceAudioInterface(edCinSourceAudioI*);

	virtual bool GetSourceSubtitleInterface(edCinSourceSubtitleI** ppSourceSubtitleInterface);
	virtual bool ReleaseSourceSubtitleInterface(edCinSourceSubtitleI*);

	CBWCinSourceAudio BWCinSourceAudio_Obj;
	CBWCinCam BWCinCam_Obj;
	CBWCinSourceSubtitle BWCinSourceSubtitle_Obj;
};



struct CinematicFileDataHeader {
	CutsceneHoldsDurations durationA;
	undefined field_0x8;
	undefined field_0x9;
	undefined field_0xa;
	undefined field_0xb;
	undefined1 offsetSomeObj; /* Created by retype action */
	undefined field_0xd;
	undefined field_0xe;
	undefined field_0xf;
	undefined1 numSomeObjAA; /* Created by retype action */
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
	undefined1 sceFileBufferStart; /* Created by retype action */
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
	undefined field_0x54;
	undefined field_0x55;
	undefined field_0x56;
	undefined field_0x57;
	undefined field_0x58;
	undefined field_0x59;
	undefined field_0x5a;
	undefined field_0x5b;
	undefined field_0x5c;
	undefined field_0x5d;
	undefined field_0x5e;
	undefined field_0x5f;
	undefined instanceIndex;
	undefined field_0x61;
	undefined field_0x62;
	undefined field_0x63;
	undefined field_0x64;
	undefined field_0x65;
	undefined field_0x66;
	undefined field_0x67;
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
};

struct CinFileContainer {
	edResCollection::RES_TYPE type;
	char* pData;
};

enum ECinematicState {
	CS_Stopped,
	CS_Interpolate,
	CS_Playing
};

struct ed_sound_sample {
	undefined field_0x0;
	undefined field_0x1;
	undefined field_0x2;
	undefined field_0x3;
	uint field_0x4;
	uint flags;
	uint field_0xc;
	undefined* field_0x10;
	undefined* field_0x14;
};

struct CCineActorConfig {
	CCineActorConfig();
	S_STREAM_REF<CActor> pActor;
	uint flags;
	float field_0x8;
	float field_0xc;
	edF32VECTOR4 postCinematicLocation;
	edF32VECTOR4 postCinematicRotationEuler;
};

struct S_STREAM_EVENT_CAMERA
{
	int cameraIndex;
	uint flags;
	float field_0x8;
	float field_0xc;
	float field_0x10;
	strd_ptr(CActor*) pActor;
	strd_ptr(ed_zone_3d*) pZone;
	float field_0x1c;

	void Init();
	void Manage(CActor* pActor);
	void SwitchOn(CActor* pActor);
	void SwitchOff(CActor* pActor);
	void Reset(CActor* pActor);
	void SaveContext(uint* pData);
	void LoadContext(uint* pData);
};

static_assert(sizeof(S_STREAM_EVENT_CAMERA) == 0x20);

struct S_STREAM_EVENT_CAMERA_CONTAINER
{
	S_STREAM_EVENT_CAMERA* pCameraStreamRef;

	void Create(ByteCode* pByteCode);

	// overload arrow operator
	S_STREAM_EVENT_CAMERA* operator->() const { return pCameraStreamRef; }
};

static_assert(sizeof(S_STREAM_EVENT_CAMERA) == 0x20);

PACK(
	struct S_STREAM_NTF_TARGET_BASE
{
	void Init();

	union
	{
		strd_ptr(CActor*) pRef;
		int index;
	};

	int cutsceneId;
	uint flags;
});

static_assert(sizeof(S_STREAM_NTF_TARGET_BASE) == 0xc);

struct S_SAVE_CLASS_NTF_SWITCH
{
	uint switchBits;
};

struct S_SAVE_CLASS_SWITCH_CAMERA
{
	int mode;
	S_SAVE_CLASS_NTF_SWITCH switchData;
	uint cameraData;
};

struct S_STREAM_NTF_TARGET_SWITCH : public S_STREAM_NTF_TARGET_BASE
{
	void Reset();
	bool Switch(CActor* pActor);
	bool PostSwitch(CActor* pActor);

	int messageId;
	uint messageFlags;
	int field_0x14;
	uint field_0x18;
};

static_assert(sizeof(S_STREAM_NTF_TARGET_SWITCH) == 0x1c);


struct S_STREAM_NTF_TARGET_SWITCH_EX : public S_STREAM_NTF_TARGET_BASE
{
	void Reset();
	bool Switch(CActor* pActor, uint messageFlags);
	int messageId;
};


struct S_STREAM_NTF_TARGET_ANALOG : public S_STREAM_NTF_TARGET_BASE
{
	void Reset();
	int NotifyAnalog(float param_1, float param_2, CActor* param_4, S_STREAM_EVENT_CAMERA* param_5);
	uint field_0xc;
	uint field_0x10;
	uint field_0x14;
	uint field_0x18;
	int field_0x1c;
	float field_0x20;
	float field_0x24;
};

static_assert(sizeof(S_STREAM_NTF_TARGET_ANALOG) == 0x28);

struct S_NTF_TARGET_STREAM_REF
{
	int entryCount;
	S_STREAM_NTF_TARGET_SWITCH aEntries[];

	static void Create(S_NTF_TARGET_STREAM_REF** pThis, ByteCode* pByteCode);
	void Init();
	void Reset();
	void Switch(CActor* pActor);
	void PostSwitch(CActor* pActor);
	void SaveContext(S_SAVE_CLASS_NTF_SWITCH* pSaveData);
	void LoadContext(S_SAVE_CLASS_NTF_SWITCH* pSaveData);
};

// This is likely what the real implementation is, as S_NTF_TARGET_STREAM_REF and S_STREAM_EVENT_CAMERA alway appear together in the code.
struct S_NTF_SWITCH
{
	S_NTF_TARGET_STREAM_REF* pTargetStreamRef;
	S_STREAM_EVENT_CAMERA* pStreamEventCamera;

	void Create(ByteCode* pByteCode);
	void Init();
	void Reset(CActor* pActor);
	void Switch(CActor* pActor);
	void PostSwitch(CActor* pActor);
	void SaveContext(S_SAVE_CLASS_SWITCH_CAMERA* pSaveData);
	void LoadContext(S_SAVE_CLASS_SWITCH_CAMERA* pSaveData);
	bool IsValid();
};

struct S_STREAM_NTF_TARGET_ONOFF : public S_STREAM_NTF_TARGET_SWITCH
{
	void Reset();
	bool SwitchOn(CActor* pActor);
	int SwitchOff(CActor* pActor);
};

static_assert(sizeof(S_STREAM_NTF_TARGET_ONOFF) == 0x1c);

struct S_TARGET_ON_OFF_STREAM_REF
{
	int entryCount;
	S_STREAM_NTF_TARGET_ONOFF aEntries[];

	static void Create(S_TARGET_ON_OFF_STREAM_REF** pThis, ByteCode* pByteCode);
	void Init();
	void Reset();
	void SwitchOn(CActor* pActor);
	void SwitchOff(CActor* pActor);
	void SaveContext(S_SAVE_CLASS_NTF_SWITCH* pSaveData);
	void LoadContext(S_SAVE_CLASS_NTF_SWITCH* pSaveData);
};

struct S_NTF_SWITCH_ONOFF
{
	S_TARGET_ON_OFF_STREAM_REF* pTargetStreamRef;
	S_STREAM_EVENT_CAMERA* pStreamEventCamera;

	void Create(ByteCode* pByteCode);
	void Init();
	void Reset(CActor* pActor);
	void SwitchOn(CActor* pActor);
	void SwitchOff(CActor* pActor);
	void SaveContext(S_SAVE_CLASS_SWITCH_CAMERA* pSaveData);
	void LoadContext(S_SAVE_CLASS_SWITCH_CAMERA* pSaveData);
};

struct S_STREAM_NTF_TARGET_SWITCH_EX_LIST
{
	int entryCount;
	S_STREAM_NTF_TARGET_SWITCH_EX aEntries[];

	void Init();
	void Reset();
	void Switch(CActor* pActor, uint flags);
	void SaveContext(S_SAVE_CLASS_NTF_SWITCH* pSaveData);
	void LoadContext(S_SAVE_CLASS_NTF_SWITCH* pSaveData);
};

struct S_NTF_SWITCH_EX_LIST
{
	S_STREAM_NTF_TARGET_SWITCH_EX_LIST* pTargetStreamRef;
	S_STREAM_EVENT_CAMERA* pStreamEventCamera;

	void Create(ByteCode* pByteCode);
	void Init();
	void Reset(CActor* pActor);
	void Switch(CActor* pActor, uint flags);
	void SaveContext(S_SAVE_CLASS_SWITCH_CAMERA* pSaveData);
	void LoadContext(S_SAVE_CLASS_SWITCH_CAMERA* pSaveData);
};

struct S_STREAM_NTF_TARGET_ANALOG_LIST
{
	int entryCount;
	S_STREAM_NTF_TARGET_ANALOG aEntries[];

	static void Create(S_STREAM_NTF_TARGET_ANALOG_LIST** pThis, ByteCode* pByteCode);
	void Init();
	void Reset();
	void NotifyAnalog(float param_1, float param_2, CActor* param_4, S_STREAM_EVENT_CAMERA* param_5);
	void SaveContext(S_SAVE_CLASS_NTF_SWITCH* pSaveData);
	void LoadContext(S_SAVE_CLASS_NTF_SWITCH* pSaveData);
};

struct S_NTF_SWITCH_ANALOG
{
	S_STREAM_NTF_TARGET_ANALOG_LIST* pTargetStreamRef;
	S_STREAM_EVENT_CAMERA* pStreamEventCamera;

	void Create(ByteCode* pByteCode);
	void Init();
	void Reset(CActor* pActor);
	void NotifyAnalog(float param_1, float param_2, CActor* param_4);
	void SaveContext(S_SAVE_CLASS_SWITCH_CAMERA* pSaveData);
	void LoadContext(S_SAVE_CLASS_SWITCH_CAMERA* pSaveData);
};

struct S_TRAP_STREAM_ENTRY
{
	S_STREAM_NTF_TARGET_ONOFF onoff;
	S_STREAM_EVENT_CAMERA eventCamera;
};

static_assert(sizeof(S_TRAP_STREAM_ENTRY) == 0x3c, "Invalid S_TRAP_STREAM_REF size");

struct S_TRAP_STREAM_REF
{
	uint entryCount;
	S_TRAP_STREAM_ENTRY aEntries[];
};

struct _ed_particle_manager;
struct ParticleFileData;

struct ParticleInstance
{
	int id;
	float field_0x4;
	float field_0x8;
	byte field_0xc;
	byte field_0xd;
	byte field_0xe;
	byte field_0xf;
	_ed_particle_manager* pParticle;
	ParticleFileData* pParticleFileData;
	CActor* pActor;
};

class CCinematic
{
public:
	struct ParticleEntry
	{
		bool InstallFromDSC(char* pFileBuffer, int fileLength, edCBankBufferEntry* pBankBufferEntry);

		char* pFileBuffer;
		uint fileSize;

		int nbMaterials;
		edDList_material* aMaterialPtrs[16];

		ulong aHashes[16];

		edDList_material aMaterials[16];

		uint nbManagers;
		ed_g2d_manager* aManagers[16];
	};

	void InitInternalData();
	void SetupInternalData();

	CCinematic();
	~CCinematic();

	void FUN_001cbe40();

	void Create(struct ByteCode* pByteCode);
	void Init();

	void PreReset();

	void Start();
	void Stop();
	void Load(int mode);
	bool LoadInternal(long mode);
	void Install();
	bool LoadCineBnk(bool mode);
	int* InstallResource(edResCollection::RES_TYPE objectType, bool type2, const char* fileName, ed_g2d_manager* textureObj, int* bufferLengthOut);
	CActorCinematic* NewCinematicActor(const edCinGameInterface::ACTORV_CREATIONtag* pTag, ed_g3d_manager* pG3D, ed_g2d_manager* pG2D);
	CCineActorConfig* GetActorConfig(CActor* pActor);

	void TryTriggerCutscene(CActor* pActor, int param_3);

	void Draw();

	void UninstallResources();

	void Level_ClearAll();

	void ConditionallyStartCinematic(bool bShouldStartCinematic);
	void FUN_001caeb0();

	void FUN_001c92b0();

	bool Has_0x2d8();
	void Remove_0x2d8();
	void Add_0x2d8();

	bool LoadEntryByFile(edBANK_ENTRY_INFO* param_2, char* pFileName, long bLevelBnk);

	void Manage();
	void ManageState_Playing();

	bool TimeSlice(float currentPlayTime);
	void IncrementCutsceneDelta();

	void InstallSounds();

	void Flush(bool param_2);

	CActor* GetActorByHashcode(uint hashCode);

	int CreateParticleInstance(float param_1, int index, int particleId, CActor* pActor);
	int GetParticleInstance(int id);

	bool CanBePlayed();
	void Level_PauseChange(bool bPaused);

	int prtBuffer;
	uint flags_0x4;
	uint flags_0x8;
	int baseB;
	uint uniqueIdentifier;

	char* fileName;

	// Number of cinematic actors spawned for the cutscene.
	int nbTotalCinematicActors;

	// Number of actors included both cinematic actors spawned for the level and actors existing in the level.
	int nbTotalLevelActors;

	uint count_0x20;
	uint field_0x24;
	int field_0x28;
	int field_0x2c;
	float field_0x30;
	S_STREAM_FOG_DEF streamFogDef;

	S_STREAM_REF<CActor> actorRefB;

	char* pBankName_0x48;
	uint field_0x4c;
	char* pBankName_0x50;
	uint field_0x54;
	float field_0x58;
	SWITCH_MODE field_0x5c;
	float field_0x60;
	SWITCH_MODE field_0x64;
	float field_0x68;

	int endLevelId;
	int endElevatorId;
	int endCutsceneId;

	undefined4 field_0x78;
	undefined4 field_0x7c;
	float field_0x80;
	ECinematicState state;
	float time_0x88;

	S_STREAM_REF<ed_zone_3d> zoneRefA;
	S_STREAM_REF<CActor> triggerActorRef;
	S_STREAM_REF<ed_zone_3d> zoneRefB;
	S_STREAM_REF<ed_zone_3d> zoneRefC;
	
	int cineActorConfigCount;
	CCineActorConfig* aCineActorConfig;
	undefined field_0xa4;
	undefined field_0xa5;
	undefined field_0xa6;
	undefined field_0xa7;
	undefined field_0xa8;
	undefined field_0xa9;
	undefined field_0xaa;
	undefined field_0xab;
	undefined field_0xac;
	undefined field_0xad;
	undefined field_0xae;
	undefined field_0xaf;
	CBWitchCin cinematicLoadObject;
	undefined* field_0x100;
	undefined field_0x104;
	undefined field_0x105;
	undefined field_0x106;
	undefined field_0x107;
	undefined field_0x108;
	undefined field_0x109;
	undefined field_0x10a;
	undefined field_0x10b;
	undefined field_0x10c;
	undefined field_0x10d;
	undefined field_0x10e;
	undefined field_0x10f;
	undefined field_0x110;
	undefined field_0x111;
	undefined field_0x112;
	undefined field_0x113;
	undefined field_0x114;
	undefined field_0x115;
	undefined field_0x116;
	undefined field_0x117;
	undefined field_0x118;
	undefined field_0x119;
	undefined field_0x11a;
	undefined field_0x11b;
	undefined field_0x11c;
	undefined field_0x11d;
	undefined field_0x11e;
	undefined field_0x11f;
	edF32MATRIX4 matrix_0x120;
	struct edCinematic cinFileData;
	CMessageFile textData;
	struct ed_g3d_manager* pMeshInfo;
	struct edNODE* pMeshTransform;

	// List of actors both spawned and existing for use in the cutscene.
	CActorCinematic** ppActorCinematics;
	int nbActorRefs;

	// List of cinematic actors spawned solely for the cutscene.
	CActorCinematic* aActorCinematic;
	int nbCinematicActors;

	int count_0x224;
	//struct CineSunHolder* pCineSunHolderArray;
	int count_0x22c;
	//struct CineSpotHolder* pCineSpotHolderArray;
	CinFileContainer* fileInfoStart;
	int cinFileCount;
	CActor* pActor;
	float totalCutsceneDelta;
	ConditionedOperationArray condArray_0x244;
	ScenaricCondition cond_0x248;

	S_NTF_SWITCH switchListA;
	S_NTF_SWITCH switchListB;
	int* field_0x25c;
	uint aAudioTrackIds[5];
	uint defaultAudioTrackId;
	edCBankBuffer cineBank;
	edCBankBufferEntry* pCineBankEntry;
	int cineBankLoadStage_0x2b4;
	int soundCount_0x2b8;
	ed_sound_sample* sound_0x2bc;
	char** playingSounds_0x2c0;
	char* field_0x2c4;
	float field_0x2c8;
	float field_0x2cc;
	float field_0x2d0;
	float field_0x2d4;
	int count_0x2d8;
	ParticleEntry* particleSectionStart;
	int numberOfParticles;
	ParticleInstance* buffer_0x2e4;
	int nbInstalledParticles;
	void* prtFileBufferPool;
};

class CCinematicManagerB : public CObjectManager
{
public:
	virtual void Game_Init() {};
	virtual void Game_Term() {};

	virtual void LevelLoading_Begin() {};
	virtual void LevelLoading_End() {};
	virtual bool LevelLoading_Manage();
	virtual void LevelLoading_Draw() {};

	virtual void Level_Init() {};
	virtual void Level_Term() {};

	virtual void Level_ClearAll() {}
	virtual void Level_Manage();
	virtual void Level_ManagePaused();
	virtual void Level_Draw() {};
	virtual void Level_PreReset();
	virtual void Level_Reset() {};

	virtual void Level_PreCheckpointReset();
	virtual void Level_CheckpointReset() {};

	virtual void Level_SectorChange(int oldSectorId, int newSectorId) {}

#ifdef PLATFORM_WIN
	virtual char* ProfileGetName() { return "CinematicB"; }
#endif
};

class CCinematic;
class CCameraCinematic;

class CCinematicManager : public CObjectManager
{
public:
	CCinematicManager();

	virtual void Game_Init();
	virtual void Game_Term() { IMPLEMENTATION_GUARD(); }

	virtual void LevelLoading_Begin();
	virtual void LevelLoading_End();
	virtual bool LevelLoading_Manage();
	virtual void LevelLoading_Draw();

	virtual void Level_Init();
	virtual void Level_Term();
	virtual void Level_AddAll(struct ByteCode* pByteCode);
	virtual void Level_ClearAll();
	virtual void Level_Manage();
	virtual void Level_ManagePaused();
	virtual void Level_Draw();

	virtual void Level_PreReset() { IMPLEMENTATION_GUARD(); }
	virtual void Level_Reset() { IMPLEMENTATION_GUARD(); }

	virtual void Level_PreCheckpointReset();
	virtual void Level_CheckpointReset();

	virtual void Level_SectorChange(int oldSectorId, int newSectorId);

	virtual void Level_PauseChange(bool bPaused);

	virtual void Level_SaveContext();
	virtual void Level_LoadContext();

#ifdef PLATFORM_WIN
	virtual char* ProfileGetName() { return "Cinematic"; }
#endif

	void WillLoadCinematic();

	CCinematic* GetCurCinematic();

	void NotifyCinematic(int cinematicIndex, CActor* pActor, int messageId, uint flags);

	bool IsCutsceneActive();
	int GetNumCutscenes_001c50b0();
	CCinematic* GetCinematic(int index);

	void StopAllCutscenes();

	bool PlayOutroCinematic(int index, CActor* param_3);
	bool RunSectorLoadingCinematic(int cutsceneID, CActor* pActor, int elevatorId, int param_5);

	bool ResetSector(int index, CActor* pHero);

	bool FUN_001c5c60();

	void DrawBandsAndSubtitle(int param_2);
	void SetSubtitle(float param_1, char* pText, edF32VECTOR4* param_4, int param_5);

	CCinematic** ppCinematicObjB_A;
	int numCutscenes_0x8;
	CCameraCinematic* pCinematicCamera;
	CCinematic** ppCinematicObjB_B;
	int activeCinematicCount;
	CCinematic* pCurCinematic;
	CCinematic* pCinematic;
	int field_0x20;
	int field_0x24;
	CCinematic* field_0x28;
	float startTime;
	uint bInitialized;
	float field_0x34;
	char* pSubtitleText;
	int subtitleHorizontalAlignment;
	undefined4 field_0x40;
	undefined4 field_0x44;
	undefined field_0x48;
	undefined field_0x49;
	undefined field_0x4a;
	undefined field_0x4b;
	undefined field_0x4c;
	undefined field_0x4d;
	undefined field_0x4e;
	undefined field_0x4f;
	edF32VECTOR4 vector_0x50;
	edF32VECTOR3 fadeColor;
	float fadeDuration;
	edF32MATRIX4 matrix_0x70;
	float field_0xb0;
	undefined field_0xb4;
	undefined field_0xb5;
	undefined field_0xb6;
	undefined field_0xb7;
	undefined field_0xb8;
	undefined field_0xb9;
	undefined field_0xba;
	undefined field_0xbb;
	undefined field_0xbc;
	undefined field_0xbd;
	undefined field_0xbe;
	undefined field_0xbf;
};

extern CCinematicManager* g_CinematicManager_0048efc;

#endif // _CINEMATICMANAGER_H
