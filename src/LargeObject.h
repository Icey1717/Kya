#ifndef _LARGEOBJECT_H
#define _LARGEOBJECT_H

#include "Types.h"
#include "edMem.h"

class Manager
{
public:
	virtual void OnBeginGame() {};
	virtual void LoadStageOne() {};
	virtual void EndLoadStageOne() {};
	virtual bool AsyncLoad() { return false; }
	virtual void LoadLevelUpdate() {};
	virtual void LoadA() {};
	virtual void LoadB() {};
	virtual void Unload() {};
	virtual void Deserialize(struct MemoryStream* pMemoryStream) {};
	virtual void PreUpdateA() {};
	virtual void PreUpdateB() {};
	virtual void Update() {};

	void* operator new(size_t size)
	{
		void* p = edMemAlloc(TO_HEAP(H_MAIN), size);
		return p;
	}
};

PACK(
struct ManagerContainer {
	struct LevelScheduleManager* g_LevelScheduleManager_00451660;
	struct FileManager3D * g_FileManager3D_00451664;
	struct AnimManager* g_AnimManager_00451668;
	char* g_CinematicManager_0045166c;
	struct SectorManager* g_SectorManager_00451670;
	struct Manager_170* g_Manager170_00451674;
	struct LocalizationManager* g_LocalizationManager_00451678;
	struct CameraViewManager* g_CameraManager_0045167c;
	struct FrontendManager* g_FrontendManager_00451680;
	struct HelpManager* g_HelpManager_00451684;
	struct PauseManager* g_PauseManager_00451688;
	struct MapManager* g_MapManager_0045168c;
	struct CollisionManager* g_CollisionManager_00451690;
	struct Manager_208* g_Manager208_00451694;
	struct GlobalSound_00451698* g_GlobalSoundPtr_00451698;
	struct Manager_C* g_ManagerC_0045169c;
	struct Manager_10* g_Manager10_004516a0;
	struct ActorManager* g_ActorManager_004516a4;
	struct EventManager* g_EventManager_006f5080;
	struct CinematicManager* g_CinematicManagerPtr_004516ac;
	struct LightManager* g_LightManager_004516b0;
	struct Manager_C_Alt* g_ManagerC_Alt_004516b4;
	struct EffectsManager* g_EffectsManager_004516b8;
	struct Manager_29b4* g_Manager29B4_004516bc;
});

class LargeObject 
{
public:
	LargeObject();

	void* operator new(size_t size)
	{
		void* p = edMemAlloc(TO_HEAP(H_MAIN), size);
		return p;
	}

public:
	int field_0x0;
	struct CameraObj_28* pCameraObj28_0x4;
	struct CameraObj_28* pCameraObj28_0x8;
	undefined4* field_0xc;
	undefined4 field_0x10;
	undefined4 field_0x14;
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
	int field_0x40;
	undefined4 field_0x44;
	undefined4 field_0x48;
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
	int field_0xcc;
	float field_0xd0;
	float field_0xd4;
	undefined4 field_0xd8;
	undefined4 field_0xdc;
	undefined4 field_0xe0;
	undefined4 field_0xe4;
	undefined4 field_0xe8;
	undefined4 field_0xec;
	undefined4 field_0xf0;
	undefined4 field_0xf4;
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
	void EndLoadStageOne();
	bool AsyncLoad_001b9cd0();
	void OnLoadLevelBnk_001b8920(struct MemoryStream* pMemoryStream);
	bool CheckFunc_001b9300();

	void LoadFunc_001b87b0();
};

void SetupGameCreateObject(void);
void WillSetupDisplayListAndRunConstructors(void);
void LoadLevelUpdate_001b9c60(void);
void LoadStageOne_001b9dc0(void);
void LoadA_001b9bf0(void);
void LoadB_001b95c0(LargeObject* param_1);
void PreUpdateObjects(LargeObject* param_1);
void UpdateObjectsMain(void);
void FUN_002ab4a0(byte param_1, uint param_2, uint param_3);

extern LargeObject* g_LargeObject_006db450;
extern uint g_DebugCameraFlag_00448ea4;
extern ManagerContainer g_ManagerSingletonArray_00451660;
extern struct StaticMeshMaster* g_StaticMeshMasterA_00448808;

#endif //_LARGEOBJECT_H
