#ifndef _CINEMATICMANAGER_H
#define _CINEMATICMANAGER_H

#include "Types.h"
#include "LargeObject.h"
#include "edC/edCBank.h"

#ifdef PLATFORM_PS2
#include <libvu0.h>
#endif
#include "ScenaricCondition.h"
#include "TranslatedTextData.h"

#include "EdenLib/edCinematic/Sources/Cinematic.h"

struct ed_g2d_manager;
struct ed_g3d_manager;
struct CActorCinematic;
struct CActor;

class CBWCinActor : public edCinActorInterface {
public:
	virtual bool Initialize();
	virtual bool SetVisibility(bool bVisible);
	virtual bool OnFrameDirected();
	virtual bool SetPos(float newX, float newY, float newZ);
	virtual bool SetHeadingEuler(float x, float y, float z, bool param_5);
	virtual bool SetHeadingQuat(float x, float y, float z, float w);
	virtual bool SetScale(float x, float y, float z);
	virtual bool SetAnim(edCinActorInterface::ANIM_PARAMStag* pTag);
	virtual bool Shutdown();

	void SetupTransform(edF32VECTOR4* position, edF32VECTOR4* heading, edF32VECTOR4* scale, ed_g3d_manager* pMeshManager);

	CActor* pParent;
	int field_0x8;

	edF32VECTOR4 nextPos;
	edF32VECTOR4 position;
	edF32VECTOR4 heading;
	edF32VECTOR4 scale;
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
	undefined field_0x120;
	undefined field_0x121;
	undefined field_0x122;
	undefined field_0x123;
	ed_g3d_manager* pAltModelManager;
};

class CBWCinCam : public edCinCamInterface {
public:
	virtual bool Activate();
	virtual bool Initialize(bool param_2, uint* flags);
	virtual bool SetFov(float fov);
	virtual bool SetPos(float x, float y, float z);
	virtual bool SetHeadingQuat(float x, float y, float z, float w);
	virtual bool SetHeadingEuler(float x, float y, float z, bool param_5);
	virtual bool Shutdown() { return true; }
};

class CBWitchCin : public edCinGameInterface {
public:

	virtual char* GetResource(edResCollection::RES_TYPE type1, bool type2, const char* fileName, int* bufferLengthOut);
	virtual bool ReleaseResource(byte, bool, void*);

	virtual bool CreateActor(edCinActorInterface** ppActorInterface, edCinGameInterface::ACTORV_CREATIONtag* const pTag);
	virtual bool GetActor(edCinActorInterface** ppActorInterface, int hashCode, edCinGameInterface::ACTORV_CREATIONtag* const pTag);
	virtual bool ReleaseActor(edCinActorInterface*);

	virtual bool CreateScenery(edCinSceneryInterface** ppActorInterface, const edCinGameInterface::SCENERY_CREATIONtag* pTag);
	virtual bool ReleaseScenery(edCinSceneryInterface*);

	virtual bool GetCamera(edCinCamInterface** pCinCamInterface, const edCinCamInterface::CAMERA_CREATIONtag*);
	virtual bool ReleaseCamera(edCinCamInterface*);

	CBWCinCam BWCinCam_Obj;
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
	undefined field_0x60;
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

PACK(
struct S_STREAM_EVENT_CAMERA {
	int field_0x0;
	int field_0x4;
	float field_0x8;
	float field_0xc;
	float field_0x10;
	int pActor; // CActor*
	int pEventChunk24_0x18; // EventChunk_24*
	undefined4 field_0x1c;

	void Init();
	void Manage(CActor* pActor);
	void SwitchOn(CActor* pActor);
	void Reset(CActor* pActor);
});


PACK(
	struct S_STREAM_NTF_TARGET_BASE
{
	void Init();

	union
	{
		int pRef; // CActor*
		int index;
	};

	int cutsceneId;
	uint flags;

}); 

struct S_STREAM_NTF_TARGET_SWITCH : public S_STREAM_NTF_TARGET_BASE
{
	void Reset();
	bool Switch(CActor* pActor);

	int messageId;
	uint messageFlags;
	int field_0x14;
	undefined4 field_0x18;
};

PACK(
struct S_STREAM_NTF_TARGET_SWITCH_LIST {
	int count;
	S_STREAM_NTF_TARGET_SWITCH aSwitches[];
});

struct CCinematic {
	void InitInternalData();
	void SetupInternalData();

	CCinematic();
	void Create(struct ByteCode* pByteCode);
	void Init();

	void PreReset();

	void Start();
	void Stop();
	void Load(long mode);
	bool LoadInternal(long mode);
	void Install();
	bool LoadCineBnk(bool mode);
	int* InstallResource(edResCollection::RES_TYPE objectType, bool type2, const char* fileName, ed_g2d_manager* textureObj, int* bufferLengthOut);
	CActorCinematic* NewCinematicActor(const edCinGameInterface::ACTORV_CREATIONtag* pTag, ed_g3d_manager* pG3D, ed_g2d_manager* pG2D);
	CCineActorConfig* GetActorConfig(CActor* pActor);

	void Draw();

	void UninstallResources();

	void Level_ClearAll();

	void FUN_001c7390(bool param_2);

	void Manage();
	void ManageState_Playing();

	bool TimeSlice(float currentPlayTime);
	void IncrementCutsceneDelta();

	void InstallSounds();

	void Flush(bool param_2);

	CActor* GetActorByHashcode(int hashCode);

	int prtBuffer;
	uint flags_0x4;
	uint flags_0x8;
	int baseB;
	byte field_0x10;
	byte field_0x11;
	byte field_0x12;
	byte field_0x13;
	char* fileName;
	uint actorCinematicCount;
	int nonCinematicOnlyActorCount;
	uint count_0x20;
	uint field_0x24;
	int field_0x28;
	int field_0x2c;
	float field_0x30;
	S_STREAM_FOG_DEF streamFogDef;

	S_STREAM_REF<CActor> actorRefB;

	char* pBankName_0x48;
	int field_0x4c;
	char* pBankName_0x50;
	undefined4 field_0x54;
	float field_0x58;
	int field_0x5c;
	float field_0x60;
	int field_0x64;
	float field_0x68;
	int field_0x6c;
	int field_0x70;
	int field_0x74;
	undefined4 field_0x78;
	undefined4 field_0x7c;
	float field_0x80;
	ECinematicState state;
	float time_0x88;

	S_STREAM_REF<ed_zone_3d> zoneRefA;
	S_STREAM_REF<CActor> actorHeroRef;
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
	CActorCinematic** ppActorCinematics;
	int loadedActorCinematicListCount;
	CActorCinematic* aActorCinematic;
	int loadedActorCinematicCount;
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
	S_STREAM_NTF_TARGET_SWITCH_LIST* pSwitchListA;
	S_STREAM_EVENT_CAMERA* pStreamEventCameraA;
	S_STREAM_NTF_TARGET_SWITCH_LIST* pSwitchListB;
	S_STREAM_EVENT_CAMERA* pStreamEventCameraB;
	int* field_0x25c;
	int intFieldE;
	undefined field_0x264;
	undefined field_0x265;
	undefined field_0x266;
	undefined field_0x267;
	undefined field_0x268;
	undefined field_0x269;
	undefined field_0x26a;
	undefined field_0x26b;
	undefined field_0x26c;
	undefined field_0x26d;
	undefined field_0x26e;
	undefined field_0x26f;
	undefined field_0x270;
	undefined field_0x271;
	undefined field_0x272;
	undefined field_0x273;
	int intFieldD;
	edCBankBuffer cineBank;
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
	int particleSectionStart;
	int numberOfParticles;
	int buffer_0x2e4;
	int count_0x2e8;
	int field_0x2ec;
};

class CCinematicManagerB : public CObjectManager {
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
};

class CCinematicManager : public CObjectManager {
public:
	CCinematicManager::CCinematicManager();

	virtual void Game_Init();
	virtual void Game_Term() {}

	virtual void LevelLoading_Begin();
	virtual void LevelLoading_End();
	virtual bool LevelLoading_Manage();

	virtual void Level_Init();

	virtual void Level_AddAll(struct ByteCode* pByteCode);
	virtual void Level_Manage();
	virtual void Level_Draw();

	virtual void Level_SectorChange(int oldSectorId, int newSectorId);

	void WillLoadCinematic();

	CCinematic* GetCurCinematic();

	void NotifyCinematic(int cinematicIndex, CActor* pActor, int messageId, uint flags);

	int GetNumCutscenes_001c50b0();
	CCinematic* GetCinematic(int index);

	struct CCinematic** ppCinematicObjB_A;
	int numCutscenes_0x8;
	struct CCameraCinematic* pCameraLocationObj;
	struct CCinematic** ppCinematicObjB_B;
	int activeCinematicCount;
	struct CCinematic* pCurCinematic;
	struct CCinematic* pCinematic;
	int field_0x20;
	int field_0x24;
	CCinematic* field_0x28;
	undefined4 field_0x2c;
	uint bInitialized;
	undefined4 field_0x34;
	undefined4 field_0x38;
	undefined field_0x3c;
	undefined field_0x3d;
	undefined field_0x3e;
	undefined field_0x3f;
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
	float x_0x60;
	float y_0x64;
	float z_0x68;
	undefined4 field_0x6c;
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
