#ifndef _CINEMATIC_H
#define _CINEMATIC_H

#include "Types.h"
#include "CinResCollection.h"

struct CutsceneHoldsDurations {
	float durationA;
	float durationB;
};

struct edCinCamInterface {
	struct CAMERA_CREATIONtag {
		int a;
	};

	virtual bool Activate() = 0;
	virtual bool Initialize(bool param_2, uint* flags) = 0;
	virtual bool SetFov(float fov) = 0;
	virtual bool SetPos(float x, float y, float z) = 0;
	virtual bool SetHeadingQuat(float x, float y, float z, float w) = 0;
	virtual bool SetHeadingEuler(float x, float y, float z, bool param_5) = 0;
};

struct edCinActorInterface {
	struct ANIM_PARAMStag {
		uint field_0x0;
		float field_0x4;
		char field_0x8;
		undefined field_0x9;
		undefined field_0xa;
		undefined field_0xb;
		uint field_0xc;
		float field_0x10;
		char field_0x14;
		undefined field_0x15;
		undefined field_0x16;
		undefined field_0x17;
		float field_0x18;
	};

	virtual bool Initialize() = 0;
	virtual bool SetVisibility(bool bVisible) = 0;
	virtual bool OnFrameDirected() = 0;
	virtual bool SetPos(float newX, float newY, float newZ) = 0;
	virtual bool SetHeadingEuler(float x, float y, float z, bool param_5) = 0;
	virtual bool SetHeadingQuat(float x, float y, float z, float w) = 0;
	virtual bool SetScale(float x, float y, float z) = 0;
	virtual bool SetAnim(edCinActorInterface::ANIM_PARAMStag* pTag) = 0;
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
		float field_0x40;
		float field_0x44;
		float field_0x48;
		float field_0x4c;
		edF32VECTOR4 vectorFieldA;
		edF32VECTOR4 vectorFieldB;
		edF32VECTOR4 vectorFieldC;
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

	virtual bool GetCamera(edCinCamInterface** pCinCam, const edCinCamInterface::CAMERA_CREATIONtag*) = 0;
	virtual char* GetResource(edResCollection::RES_TYPE type1, long type2, const char* fileName, int* bufferLengthOut) = 0;
	virtual bool CreateActor(edCinActorInterface** ppActorInterface, edCinGameInterface::ACTORV_CREATIONtag* const pTag) = 0;
	virtual bool CreateScenery(edCinSceneryInterface** ppActorInterface, const edCinGameInterface::SCENERY_CREATIONtag* pTag) = 0;
	virtual bool ReleaseResource(void*, bool) = 0;
};

PACK(
	struct edCinematicSourceInternal {
	int type;
	int offset;
	int pTag; // edSCENEtag *
});

struct edCinematicSource {
	bool Create(edCinGameInterface& loadObj, edResCollection& resPtr);
	bool Initialize();
	bool Timeslice(float currentPlayTime, uint param_3);
	edCinematicSourceInternal* pInternal;
};

struct FrameInfo {
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
	edResCollection* pRes;
	bool ExtractVersions(int size, int* cinematicLibraryVersion, int* cinematicCompilerVersion);
	bool Create(edCinGameInterface& pInterface, const char* fileName);
	bool Create(edCinGameInterface& pInterface, void* cinFileBuffer, int bufferLength);
	bool Initialize();
	bool Timeslice(float deltaTime, FrameInfo* pFrameInfo);
};


#endif //_CINEMATIC_H
