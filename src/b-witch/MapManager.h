#ifndef _MAP_MANAGER_H
#define _MAP_MANAGER_H

#include "Types.h"
#include "LargeObject.h"
#include "ed3D.h"
#include "edBank/edBankBuffer.h"

#define MARKER_TYPE_RUNE 0x10


struct MapPosition
{
	edF32VECTOR3 position;
	int field_0xc;
};

struct MapDataSizes
{
	int field_0x0;
	int nbTeleporters;
	int nbWolfen;
	int nbRunes;
};

struct MapSaveChunk
{
	edF32VECTOR3 field_0x0;
	edF32VECTOR3 field_0xc;
	edF32VECTOR3 field_0x18;
	float field_0x24;
	uint field_0x28;
};

struct SaveDataChunk_BLMP
{
	MapSaveChunk saveChunk;
	float field_0x2c;
	int field_0x30;
	MapDataSizes mapDataSizes;
	MapPosition aPositions[];
};

struct SubAstruct
{
	union {
		struct {
			float x1, y1, s1, t1;
		};
		float raw[4];
	};
};

struct astruct_21
{
	SubAstruct field_0x0[4];
};

struct LoadLoopObject_50;
struct NativShopLevelSubObj;

class CLevelMap
{
public:
	void Level_PostInit();
	void Draw();
	bool LoadMapBank(int levelId);
	void InstallMapBank();
	void SetupMatrices();
	void Update();
	void ManagePad();
	void FUN_003bafa0();
	MapPosition* FUN_003bd1f0(int inLevelId, MapDataSizes* pMapSizes, MapPosition* pMarkerBuffer, long param_5);
	void FUN_003bd3b0(LoadLoopObject_50* param_2, int param_3);
	void FUN_003bb210(LoadLoopObject_50* pLoadLoopObj, int param_3);
	void FUN_003bde70(float param_1, float param_2, int param_4, edF32VECTOR4* param_5, uint param_6);
	MapPosition* FUN_003bb990(LoadLoopObject_50* pLoadLoopObj, int param_3, MapDataSizes* param_4, MapPosition* param_5);
	void FUN_003bd960(float param_1, float param_2, edF32VECTOR4* param_4, undefined3 param_5);
	void FUN_003be250(float param_1, float param_2, int param_4, edF32VECTOR4* param_5, edF32VECTOR4* param_6, uint param_7);
	void FUN_003be570(float param_1, float param_2, edF32VECTOR4* param_4, uint param_5);
	NativShopLevelSubObj* FUN_003bc1e0();
	void FUN_003bcdf0(MapPosition* pMapPosition, int param_3);
	void FUN_003bbe50(LoadLoopObject_50* pLoadLoopObj, int param_3, MapDataSizes* pMapSize, MapPosition* pMapPosition);

	int field_0x0;
	int levelId;
	edCBankBuffer bankBuffer;
	edCBankBufferEntry* pBankBufferEntry;

	edDList_material* pMaterials;

	ed_g2d_manager field_0x44;

	int field_0x78;
	char* field_0x7c;

	edF32VECTOR4 field_0x80;
	edF32VECTOR4 field_0x90;
	edF32VECTOR4 field_0xa0;

	edF32VECTOR4 field_0xb0;

	float field_0xc0;
	float field_0xc4;
	float field_0xc8;
	float field_0xcc;

	undefined4 field_0xd0;
	undefined4 field_0xd4;
	float field_0xd8;
	undefined4 field_0xdc;

	undefined4 field_0xe0;
	undefined4 field_0xe4;

	edF32MATRIX4 field_0xf0;
	edF32MATRIX4 field_0x130;
	edF32MATRIX4 field_0x170;
	edF32MATRIX4 field_0x1b0;
};

class CWorldMap
{
public:
	void Level_PostInit();
	void Draw();

	int currentLevelId;
};

class edCTextFormat;

struct astruct_20
{
	float FUN_003f69b0();
	void FUN_003f67c0(float param_2, float param_3, float param_4, edCTextFormat* pTextFormat, uint color);
	void FUN_003f6480(float param_1, float param_2, char* pText, int param_5, uint color, uint param_7, int param_8);
	void FUN_003f5da0(float param_1, float param_2, uint param_4, int param_5);

	int field_0x0;
	int field_0x4;
	ulong hash;
	int field_0x10;
	int field_0x14;
	ulong hash_2;

};

struct CMapManager : public CObjectManager
{
	virtual void Game_Init();
	virtual void Game_Term();

	virtual void Level_Init();
	virtual void Level_Term();
	virtual void Level_ClearAll();

	virtual void Level_Manage();
	virtual void Level_ManagePaused();
	virtual void Level_Draw();

	virtual void Level_SaveContext();
	virtual void Level_LoadContext();

	void OnLoadLevelBnk_003f9a60(ByteCode* pMemoryStream);

	void Level_PostInit();

	void OnFileLoaded();

	void Func_003f8d80();
	void FUN_003f8a00();
	float FUN_003f6ce0(float param_1, int param_3);
	void FUN_003f6e10(float param_1);
	bool FUN_003f7a40(int param_2, MapDataSizes* param_3, MapPosition* param_4, int param_5);
	void GetMarkerPositions(MapDataSizes* param_2, MapPosition* param_3);
	bool FUN_003f6ae0(int levelId, int markerType, int param_4);
	int FUN_003f87b0(SubAstruct* param_2, SubAstruct* param_3, int nbCount, uint index);
	void DrawSprite(edDList_material* pMaterial, astruct_21* param_3, uint color);
	void FUN_003f85c0(edDList_material* pMaterial, astruct_21* param_3, uint color);
	int FUN_003f9040();
	edDList_material* GetDlistMaterial(int index);

#ifdef PLATFORM_WIN
	virtual char* ProfileGetName() { return "Map"; }
#endif

	int field_0x4;
	undefined4 field_0x8;
	float field_0xc;
	int* field_0x10;
	undefined4 field_0x14;
	S_STREAM_REF<ed_zone_3d> field_0x18;
	CLevelMap* pLevelMap;
	CWorldMap* pWorldMap;
	undefined4 field_0x24;
	undefined4 field_0x28;
	MapSaveChunk field_0x2c;
	MapSaveChunk field_0x58;
	edCBankBuffer mapBank;
	edCBankBufferEntry* pBankBufferEntry;
	ed_g2d_manager field_0xc0;
	edDList_material field_0xf0[15];
	ed_g2d_manager field_0x1e0;
	edDList_material field_0x210[15];
	undefined4 field_0x300;
	undefined field_0x304;
	undefined field_0x305;
	undefined field_0x306;
	undefined field_0x307;
	undefined field_0x308;
	undefined field_0x309;
	undefined field_0x30a;
	undefined field_0x30b;
	undefined field_0x30c;
	undefined field_0x30d;
	undefined field_0x30e;
	undefined field_0x30f;
	edF32VECTOR4 field_0x310;
	edF32VECTOR4 field_0x320;
	float field_0x330;
	int field_0x334;

	astruct_20 field_0x338;
	astruct_20 field_0x358;
	float field_0x378;

	float field_0x37c;
	int field_0x380;
	float field_0x384;
	float field_0x388;
	undefined4 field_0x38c;
	undefined4 field_0x390;
	undefined4 field_0x394;
	undefined field_0x398;
	undefined field_0x399;
	undefined field_0x39a;
	undefined field_0x39b;
	undefined field_0x39c;
	undefined field_0x39d;
	undefined field_0x39e;
	undefined field_0x39f;
};

uint FUN_003f6910(astruct_20* param_1);

#endif // _MAP_MANAGER_H
