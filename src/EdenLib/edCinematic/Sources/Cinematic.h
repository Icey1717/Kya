#ifndef _CINEMATIC_H
#define _CINEMATIC_H

#include "Types.h"
#include "CinResCollection.h"

struct CutsceneHoldsDurations
{
	float durationA;
	float durationB;
};

struct edCinSourceAudioI
{
	virtual bool Create(char* pFileName) = 0;
	virtual bool Play() = 0;
	virtual bool Stop() = 0;
	virtual bool GetTime() = 0;
	virtual bool Destroy() = 0;
	virtual float Func_0x1c(int audioTrackId) = 0;
};

struct edCinCamInterface
{
	struct CAMERA_CREATIONtag
	{
		int a;
	};

	virtual bool Activate() = 0;
	virtual bool Initialize(bool param_2, uint* flags) = 0;
	virtual bool SetFov(float fov) = 0;
	virtual bool SetPos(float x, float y, float z) = 0;
	virtual bool SetHeadingQuat(float x, float y, float z, float w) = 0;
	virtual bool SetHeadingEuler(float x, float y, float z, bool param_5) = 0;
	virtual bool Shutdown() = 0;
};

struct edCinSourceSubtitleI
{
	struct SUBTITLE_PARAMStag
	{
		int field_0x0;

		uint keyA;
		uint keyB;

		float time;

		uint flags;
	};

	static_assert(sizeof(SUBTITLE_PARAMStag) == 0x14, "SUBTITLE_PARAMStag size is incorrect");

	virtual bool Create(char* pFileName, long param_3) = 0;
	virtual bool Init() = 0;
	virtual bool SetPos(float x, float y, float z) = 0;
	virtual bool SetSubtitle(float keyTime, SUBTITLE_PARAMStag* pTag) = 0;
	virtual bool Shutdown() = 0;
	virtual bool Destroy() = 0;
};

struct edCinActorInterface
{
	struct ANIM_PARAMStag_Anim
	{
		int pHdr; // edANM_HDR*
		float field_0x4;
		char field_0x8;
		undefined field_0x9;
		undefined field_0xa;
		undefined field_0xb;
	};

	struct ANIM_PARAMStag
	{
		ANIM_PARAMStag_Anim srcAnim;
		ANIM_PARAMStag_Anim dstAnim;
		float field_0x18;
	};

	struct PARTICLE_PARAMStag
	{
		int particleId; // void*
		int field_0x4;
		float field_0x8;
	};

	virtual bool Initialize() = 0;
	virtual bool SetVisibility(bool bVisible) = 0;
	virtual bool OnFrameDirected() = 0;
	virtual bool SetPos(float newX, float newY, float newZ) = 0;
	virtual bool SetHeadingEuler(float x, float y, float z, bool param_5) = 0;
	virtual bool SetHeadingQuat(float x, float y, float z, float w) = 0;
	virtual bool SetScale(float x, float y, float z) = 0;
	virtual bool SetAnim(edCinActorInterface::ANIM_PARAMStag* pTag) = 0;
	virtual bool SetParticles(float param_1, edCinActorInterface::PARTICLE_PARAMStag* pTag) = 0;
	virtual bool SetSubtitle(float param_1, edCinSourceSubtitleI::SUBTITLE_PARAMStag* pParams) = 0;
	virtual bool Shutdown() = 0;
};

struct edCinSceneryInterface {

};

struct edCinGameInterface 
{
	struct ACTORV_CREATIONtag 
	{
		char name[32];
		char* meshName;
		bool bHasMesh;
		undefined field_0x25;
		undefined field_0x26;
		undefined field_0x27;
		char* textureName;
		bool bHasTexture;
		undefined field_0x2d;
		undefined field_0x2e;
		undefined field_0x2f;
		float field_0x30;
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
		edF32VECTOR4 boundingSphere;
		edF32VECTOR4 position;
		edF32VECTOR4 heading;
		edF32VECTOR4 scale;
	};

	struct SCENERY_CREATIONtag {
		char name[32];
		char* szMeshPath;
		byte meshType;
		undefined field_0x25;
		undefined field_0x26;
		undefined field_0x27;
		char* szTexturePath;
		byte textureType;
	};

	virtual char* GetResource(edResCollection::RES_TYPE type1, bool type2, const char* fileName, int* bufferLengthOut) = 0;
	virtual bool ReleaseResource(byte, bool, void*) = 0;

	virtual bool CreateActor(edCinActorInterface** ppActorInterface, edCinGameInterface::ACTORV_CREATIONtag* const pTag) = 0;
	virtual bool GetActor(edCinActorInterface** ppActorInterface, uint hashCode, edCinGameInterface::ACTORV_CREATIONtag* const pTag) = 0;
	virtual bool ReleaseActor(edCinActorInterface*) = 0;

	virtual bool CreateScenery(edCinSceneryInterface** ppActorInterface, const edCinGameInterface::SCENERY_CREATIONtag* pTag) = 0;
	virtual bool ReleaseScenery(edCinSceneryInterface*) = 0;

	virtual bool GetCamera(edCinCamInterface** pCinCam, const edCinCamInterface::CAMERA_CREATIONtag*) = 0;
	virtual bool ReleaseCamera(edCinCamInterface*) = 0;

	virtual bool GetSourceAudioInterface(edCinSourceAudioI** ppSourceAudioInterface) = 0;
	virtual bool ReleaseSourceAudioInterface(edCinSourceAudioI*) = 0;

	virtual bool GetSourceSubtitleInterface(edCinSourceSubtitleI** ppSourceSubtitleInterface) = 0;
	virtual bool ReleaseSourceSubtitleInterface(edCinSourceSubtitleI*) = 0;
};

struct edCinematicSourceInternal
{
	int type;
	int offset;
	int pTag; // edSCENEtag *
};

static_assert(sizeof(edCinematicSourceInternal) == 0xc, "edCinematicSourceInternal size is incorrect");

struct edCinematicSource
{
	bool Create(edCinGameInterface& loadObj, edResCollection& resPtr);
	bool Initialize();
	bool Timeslice(float currentPlayTime, uint param_3);
	bool Shutdown();
	bool Destroy(edCinGameInterface& pCinGameInterface);
	edCinematicSourceInternal* pInternal;
};

struct FrameInfo
{
	float* field_0x0;
	float field_0x4;
	float field_0x8;
	float field_0xc;
	float field_0x10;
	undefined4 field_0x14;
	float field_0x18;
};

PACK(
struct edCinematicTag {
	float field_0x0;
	float totalPlayTime;
	undefined field_0x8;
	undefined field_0x9;
	undefined field_0xa;
	undefined field_0xb;
	int srcCount;
	int field_0x10;
	int field_0x14;
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
});

struct edCinematic {
	edCinematicTag* pCinTag;
	int* fileStart;
	edResCollectionHeader* pRes;
	bool ExtractVersions(int size, int* cinematicLibraryVersion, int* cinematicCompilerVersion);
	bool Create(edCinGameInterface& pInterface, const char* fileName);
	bool Create(edCinGameInterface& pInterface, void* cinFileBuffer, int bufferLength);
	bool Initialize();
	bool Timeslice(float deltaTime, FrameInfo* pFrameInfo);
	bool Shutdown();
	bool Destroy(edCinGameInterface& pCinGameInterface);
};


#endif //_CINEMATIC_H
