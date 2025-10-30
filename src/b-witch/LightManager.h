#ifndef LIGHT_MANAGER
#define LIGHT_MANAGER

#include "Types.h"
#include "LargeObject.h"
#include "ed3D/ed3DSceneManager.h"

class CActor;
struct ed_3d_hierarchy;
struct S_CHECKPOINT;
struct ByteCode;
struct BaseShape;

struct CLightConfig {
	edF32VECTOR4 lightAmbient;
	edF32MATRIX4 lightDirections;
	edF32MATRIX4 lightColorMatrix;
	ed_3D_Light_Config config;
	float field_0x9c;

	static void Validate(ed_3D_Light_Config* pConfig, bool bDoSomething);
	static float ComputeShadow(ed_3D_Light_Config* pConfig, edF32VECTOR4* param_2);
};

struct LightingContext {
	edF32VECTOR4 position;
	edF32VECTOR4* pLightDirection;
	edF32VECTOR4* pLightAmbient;
	edF32VECTOR4* pLightColor;
	undefined4 field_0x1c;
	float colorMultiplier;
};

class CLight {
public:
	CLight();

	virtual void Init();
	virtual void Manage() { return; }
	virtual void Term() { return; }
	virtual void Activate();
	virtual bool DoLighting(LightingContext* pContext) { return false; }
	virtual int GetBaseShape(BaseShape** ppBaseShape) { return 0; }
	virtual edF32VECTOR4* GetPosition() { return NULL; }
	virtual void Create(ByteCode* pByteCode) {}

	bool TestIlluminationZones(edF32VECTOR4* pLocation, int id);
	bool IsInCluster(ed_zone_3d* pZone);

	_rgba colour_0x4;
	ushort field_0x8;
	short referencedLightIndex;
	S_ZONE_STREAM_REF* pZoneHolderA;
	S_ZONE_STREAM_REF* pZoneHolderB;
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
};

struct S_LIGHT_STREAM_REF {
	int entryCount;
	S_STREAM_REF<CLight> aEntries[];
};

struct LightZoneEntry
{
	ed_zone_3d* pZone;
	float field_0x4;
	int field_0x8;
};

class CLightManager : public CObjectManager {

public:
	CLightManager();

	// Begin Manager
	virtual bool LevelLoading_Manage();
	
	virtual void Level_Init();
	virtual void Level_Term();
	virtual void Level_AddAll(struct ByteCode* pByteCode);
	virtual void Level_ClearAll();

	virtual void Level_Manage();
	virtual void Level_ManagePaused();
	virtual void Level_Draw();

	virtual void Level_Reset();

	virtual void Level_CheckpointReset();

	virtual void Level_SectorChange(int oldSectorId, int newSectorId);

	virtual char* ProfileGetName();
	// End Manager

	void ComputeLighting(float multiplier, CActor* pActor, uint flags, ed_3D_Light_Config* pConfig);
	void ComputeLighting(float multiplier, ed_3d_hierarchy* pHier, edF32VECTOR4* pLocation, uint flags, ed_3D_Light_Config* pConfig, uint param_7);
	void ComputeLighting(edF32VECTOR4* pLocation, ed_3D_Light_Config* pConfig);

	void AddLight(ByteCode* pByteCode, CLight** ppOutLight);

	CLight* CreateSimpleLight(ByteCode* pByteCode);
	void Activate(CLight* pLight, int param_3);
	void Reference(CLight* pLight, int param_3, bool param_4, bool param_5, int param_6);
	int ReferenceZone(ed_zone_3d* pZone);

	void BuildSectorList();
	void BuildActiveList();

	float ComputeCollision(CLight* pLightA, CLight* pLightB);

	void SetManagedByCluster(CLight* pLight);
	void EnterManagedCluster(CLight* pLight);
	void LeaveManagedCluster(CLight* pLight);

	int referencedLightsCount;
	int lightCount;
	int sectorLightCount;
	int sectorId;
	int activeLightCount;
	int bSectorListDirty;
	int bActiveListDirty;
	int nextFreeLightConfig;
	ed_3D_Light_Config lightConfig;

	CLight** aLights;
	CLight** aSectorLights;
	CLight** aActiveLights;

	CLightConfig* aLightConfigs;

	int totalLightCount;
	int totalLightConfigs;

	edF32VECTOR4 vector_0xf0;

	edF32VECTOR4 lightAmbient;
	edF32MATRIX4 lightDirections;
	edF32MATRIX4 lightColorMatrix;

	LightZoneEntry* field_0xe0;
	int field_0xe4;
	int field_0xe8;

	float shadowValue;

	int field_0x104;
	int field_0x108;
	int field_0x10c;
	int field_0x110;
};

#endif //LIGHT_MANAGER
