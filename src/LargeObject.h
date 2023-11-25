#ifndef _LARGEOBJECT_H
#define _LARGEOBJECT_H

#include "Types.h"
#include "edMem.h"

class CObjectManager
{
public:
	virtual void Game_Init() {};
	virtual void Game_Term() {};
	virtual void LevelLoading_Begin() {};
	virtual void LevelLoading_End() {};
	virtual bool LevelLoading_Manage() { return false; }
	virtual void Level_ClearAll() {}
	virtual void LevelLoading_Draw() {};
	virtual void Level_Install() {};
	virtual void Level_Init() {};
	virtual void Level_Term() {};
	virtual void Level_AddAll(struct ByteCode* pMemoryStream) {};
	virtual void Level_Manage() {};
	virtual void Level_ManagePaused() {};
	virtual void Level_Draw() {};
	virtual void Level_CheckpointReset() {};
	virtual void Level_Reset() {};

	virtual void Level_SectorChange(int oldSectorId, int newSectorId) {}

	void* operator new(size_t size)
	{
		void* p = edMemAlloc(TO_HEAP(H_MAIN), size);
		return p;
	}
};

enum MANAGER_TYPE {
	MO_GlobalDListManager = 0x17
};

PACK(
struct ManagerContainer {
	struct LevelScheduleManager* g_LevelScheduleManager_00451660;
	struct FileManager3D * g_FileManager3D_00451664;
	struct CAnimationManager* g_AnimManager_00451668;
	char* g_CinematicManager_0045166c;
	struct CSectorManager* g_SectorManager_00451670;
	struct Manager_170* g_Manager170_00451674;
	struct LocalizationManager* g_LocalizationManager_00451678;
	struct CCameraManager* g_CameraManager_0045167c;
	struct FrontendManager* g_FrontendManager_00451680;
	struct HelpManager* g_HelpManager_00451684;
	struct PauseManager* g_PauseManager_00451688;
	struct MapManager* g_MapManager_0045168c;
	struct CCollisionManager* g_CollisionManager_00451690;
	struct Manager_208* g_Manager208_00451694;
	struct GlobalSound_00451698* g_GlobalSoundPtr_00451698;
	struct CWayPointManager* g_WayPointManager_0045169c;
	struct CPathManager* g_PathManager_004516a0;
	struct CActorManager* g_ActorManager_004516a4;
	struct EventManager* g_EventManager_006f5080;
	struct CCinematicManager* g_CinematicManagerPtr_004516ac;
	struct LightManager* g_LightManager_004516b0;
	struct Manager_C_Alt* g_ManagerC_Alt_004516b4;
	struct CFxManager* g_EffectsManager_004516b8;
	struct CGlobalDListManager* g_GlobalDListManager_004516bc;
});

PACK(
struct S_STREAM_FOG_DEF {
	float clipValue_0x0;
	float field_0x4;
	_rgba fogRGBA;
	uint flags;
});

struct FogClipEntry {
	S_STREAM_FOG_DEF* pStreamDef;
	float field_0x4;
};

class CScene 
{
public:
	CScene();

	void* operator new(size_t size)
	{
		void* p = edMemAlloc(TO_HEAP(H_MAIN), size);
		return p;
	}

public:
	int clipValue_0x0;
	struct ed_viewport* pViewportA;
	struct ed_viewport* pViewportB;
	S_STREAM_FOG_DEF* pFogClipStream;
	undefined4 mipmapK;
	undefined4 mipmapL;
	undefined4 field_0x18;
	float field_0x1c;
	undefined4 field_0x20;
	float field_0x24;
	int field_0x28;
	int defaultTextureIndex_0x2c;
	undefined4 field_0x30;
	undefined field_0x34;
	undefined field_0x35;
	undefined field_0x36;
	undefined field_0x37;
	undefined8 field_0x38;
	int curState;
	undefined4 field_0x44;
	undefined4 field_0x48;
	FogClipEntry aFogClipStack[2];
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
	undefined field_0xa8;
	undefined field_0xa9;
	undefined field_0xaa;
	undefined field_0xab;
	undefined field_0xac;
	undefined field_0xad;
	undefined field_0xae;
	undefined field_0xaf;
	undefined field_0xb0;
	undefined field_0xb1;
	undefined field_0xb2;
	undefined field_0xb3;
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
	undefined field_0xc0;
	undefined field_0xc1;
	undefined field_0xc2;
	undefined field_0xc3;
	undefined field_0xc4;
	undefined field_0xc5;
	undefined field_0xc6;
	undefined field_0xc7;
	undefined field_0xc8;
	undefined field_0xc9;
	undefined field_0xca;
	undefined field_0xcb;
	int fogClipSettingStackSize;
	float field_0xd0;
	float field_0xd4;
	float field_0xd8;
	float field_0xdc;
	_rgba prevFogRGBA;
	uint prevFogFlags;
	float clipValue_0xe8;
	float field_0xec;
	_rgba fogRGBA;
	undefined4 fogFlags;
	short mode_0xf8;
	undefined field_0xfa;
	undefined field_0xfb;
	undefined field_0xfc;
	undefined field_0xfd;
	undefined field_0xfe;
	undefined field_0xff;
	undefined field_0x100;
	undefined field_0x101;
	undefined field_0x102;
	undefined field_0x103;
	undefined field_0x104;
	undefined field_0x105;
	undefined field_0x106;
	undefined field_0x107;
	undefined field_0x108;
	undefined field_0x109;
	undefined field_0x10a;
	undefined field_0x10b;
	int field_0x10c;
	int field_0x110;
	undefined4 field_0x114;
	float field_0x118;
	undefined4 field_0x11c;
	uint count_0x120;
	undefined field_0x124;
	undefined field_0x125;
	undefined field_0x126;
	undefined field_0x127;

public:
	void Level_Setup(struct ByteCode* pMemoryStream);
	bool CheckFunc_001b9300(void);

	void LoadFunc_001b87b0(void);

	void HandleFogAndClippingSettings();
	void PushFogAndClippingSettings(float other, S_STREAM_FOG_DEF* pFogStream);

	void Level_Install(void);
	void Level_Init(void);
	void Level_Manage(void);
	void LevelLoading_Draw(void);
	void LevelLoading_End(void);
	bool LevelLoading_Manage(void);
	void LevelLoading_Begin(void);

	void Level_SectorChange(int oldSectorId, int newSectorId);

	void HandleCurState();

	static void CreateScene(void);
	static void* GetManager(MANAGER_TYPE type);

	static CScene* _pinstance;
	static struct ed_3D_Scene* _scene_handleA;
	static struct ed_3D_Scene* _scene_handleB;

	static ManagerContainer ptable;
};

void Game_Init(void);
void UpdateObjectsMain(void);
void ed3DSetMipmapProp(bool bDoMipmap, uint mipMapL, uint mipMapK);

ed_3D_Scene* GetStaticMeshMasterA_001031b0(void);

extern ed_3D_Scene* gShadowScene;

extern byte gbDoMipmap;
extern uint gMipmapK;
extern uint gMipmapL;

#define GAME_STATE_PAUSED 0x20

extern uint GameFlags;

#endif //_LARGEOBJECT_H
