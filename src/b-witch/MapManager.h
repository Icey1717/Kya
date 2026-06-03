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

struct S_2DVERTEX
{
	union {
		struct {
			float x1, y1, s1, t1;
		};
		float raw[4];
	};
};

struct S_2DRECT
{
	S_2DVERTEX aVertices[4];
};

struct ObjectiveEntry;
struct NativShopLevelSubObj;

PACK(struct LevelMapFileData
{
	char _padding0[0x8];
	int field_0x8;
	int field_0xc;
	int field_0x10;
	int field_0x14;
	int aTileIndices[];
};)

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
	void UpdateViewportBounds();
	MapPosition* FindMarkerForLevel(int inLevelId, MapDataSizes* pMapSizes, MapPosition* pMarkerBuffer, long param_5);
	void DrawMapTilesAndMarkers(ObjectiveEntry* pObjectiveEntry, int param_3);
	void DrawMapOverlays(ObjectiveEntry* pObjectiveEntry, int param_3);
	void DrawWorldMarkerIcon(float param_1, float param_2, int param_4, edF32VECTOR4* param_5, uint param_6);
	MapPosition* DrawCollectedMarkers(ObjectiveEntry* pObjectiveEntry, int param_3, MapDataSizes* param_4, MapPosition* param_5);
	void DrawPulsingMarkerHighlight(float param_1, float param_2, edF32VECTOR4* param_4, undefined3 param_5);
	void DrawOrientedMarkerIcon(float param_1, float param_2, int param_4, edF32VECTOR4* param_5, edF32VECTOR4* param_6, uint param_7);
	void DrawOffscreenMarkerArrow(float param_1, float param_2, edF32VECTOR4* param_4, uint param_5);
	NativShopLevelSubObj* DrawAndSelectNearestShopMarker();
	void DrawMarkerLabels(MapPosition* pMapPosition, int param_3);
	void DrawObjectiveTargetMarkers(ObjectiveEntry* pObjectiveEntry, int param_3, MapDataSizes* pMapSize, MapPosition* pMapPosition);
	void DrawLegend(float param_1, float param_2);

	int field_0x0;
	int levelId;
	edCBankBuffer bankBuffer;
	edCBankBufferEntry* pBankBufferEntry;

	edDList_material* pMaterials;

	ed_g2d_manager field_0x44;

	int field_0x78;
	LevelMapFileData* field_0x7c;

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
	float field_0xe4;

	edF32MATRIX4 field_0xf0;
	edF32MATRIX4 field_0x130;
	edF32MATRIX4 scrollMatrix;
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

class CObjectivesPanel
{
public:
	float GetPanelHeight();
	void DrawTextRevealUnderline(float param_2, float param_3, float param_4, edCTextFormat* pTextFormat, uint color);
	void DrawObjectiveTextLine(float param_1, float param_2, char* pText, int param_5, uint color, uint param_7, int param_8);
	void DrawObjectiveDetails(float param_1, float param_2, uint param_4, int param_5);
	uint GetObjectiveLineMask();

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
	void DrawMapPromptHint();
	float ComputeMapSlideFactor(float param_1, int param_3);
	void DrawObjectivePanel(float param_1);
	bool LoadMarkerPositionsForLevel(int param_2, MapDataSizes* param_3, MapPosition* param_4, int param_5);
	void GetMarkerPositions(MapDataSizes* param_2, MapPosition* param_3);
	bool HasRouteToLevel(int levelId, int markerType, int param_4);
	int ClipPoly(S_2DVERTEX* param_2, S_2DVERTEX* param_3, int nbCount, uint index);
	void DrawSprite(edDList_material* pMaterial, S_2DRECT* pRect, uint color);
	void DrawClippedSprite(edDList_material* pMaterial, S_2DRECT* pRect, uint color);
	int FUN_003f9040();
	edDList_material* GetDlistMaterial(int index);
	void DrawButtons(char* pText);

#ifdef PLATFORM_WIN
	virtual char* ProfileGetName() { return "Map"; }
#endif

	int field_0x4;
	uint field_0x8;
	float field_0xc;
	int* field_0x10;
	int field_0x14;
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
	int field_0x300;

	edF32VECTOR4 field_0x310;
	edF32VECTOR4 field_0x320;
	float field_0x330;
	int field_0x334;

	CObjectivesPanel field_0x338;
	CObjectivesPanel field_0x358;
	float field_0x378;

	float field_0x37c;
	int field_0x380;
	float field_0x384;
	float field_0x388;
	int field_0x38c;
	int field_0x390;
	int field_0x394;
};

#endif // _MAP_MANAGER_H
