#ifndef DLIST_MANAGER_H
#define DLIST_MANAGER_H

#include "Types.h"
#include "LargeObject.h"
#include "DlistManager.h"

#define IMPLEMENTATION_GUARD_DLIST_PATCH(x) assert(false);

#define CALL_ELEMENT_SET_ACTIVE 1
#define CALL_ELEMENT_HIDE_VERTEX 2
#define CALL_ELEMENT_HIDE_SPRITE 3

union edF32VECTOR4;
union edpkt_data;
union edVertex;

struct ed_3d_strip;
struct ed_3d_sprite;
struct ed_3D_Scene;
struct ed_g3d_manager;

class CObject;

struct RenderCommandUint
{
	uint type;
	edpkt_data* pCommandBuffer;
	uint size;
	DisplayList* pDisplayList;
};

union RenderInput
{
	edpkt_data* pPkt;
	ed_3d_strip* pStrip;
	ed_3d_sprite* pSprite;
};

struct DisplayListCommand
{
	union
	{
		RenderCommandUint aCommandArray[4];
		edF32MATRIX4 matrix;
	};

	byte bActive;
	byte field_0x41;
	ushort dataType;
	RenderInput pRenderInput;
	edpkt_data* pCurDListBuf;
	int primType;
	uint nbAddedVertex;
	int nbMatrix;
	edF32MATRIX4* pCurMatrixArray;
	undefined field_0x5c;
	undefined field_0x5d;
	undefined field_0x5e;
	undefined field_0x5f;
};

struct DisplayList
{
	ushort flags_0x0;
	undefined field_0x2;
	byte field_0x3;
	ushort nbCommands;
	ushort nbSavedCommands;
	int nbMaxCommands;
	char* pCommandBuffer;
	edpkt_data* field_0x10;
	edpkt_data* field_0x14;
	edpkt_data* pRenderCommands;
	DisplayListCommand* aCommands;
	ed_3D_Scene* pScene;
	float field_0x24;
	char* field_0x28;
	DisplayListCommand** aSavedCommands;
};

class CGlobalDList
{
public:
	CGlobalDList();
	CGlobalDList(int inField_0x8, int inField_0xc, int inField_0x10, int inField_0x1c, ed_3D_Scene* pInStaticMeshMaster);
	~CGlobalDList();

	void Init();
	void Term() {}

	DisplayList* pDisplayListInternal; // Double buffered display list data.
	int nbCommands;
	int nbMatrices;
	int field_0x10;
	int bEnabled;
	byte field_0x18;
	byte field_0x19;
	byte field_0x1a;
	byte field_0x1b;
	int flags;
};

struct S_GLOBAL_DLIST_PATCH
{
	undefined4 field_0x0;
	CObject* pOwner;
	int nbMatrices;
	int nbInstances;
	edVertex* pVertex;
	_rgba* pRgba;
	uint* pSt;
	uint* pUv;
	DisplayListCommand* field_0x20;
};

class CGlobalDListPatch : public CGlobalDList
{
public:
	S_GLOBAL_DLIST_PATCH* aPatches[296];
	undefined field_0x4c0;
	undefined field_0x4c1;
	undefined field_0x4c2;
	undefined field_0x4c3;
	undefined field_0x4c4;
	undefined field_0x4c5;
	undefined field_0x4c6;
	undefined field_0x4c7;
	undefined field_0x4c8;
	undefined field_0x4c9;
	undefined field_0x4ca;
	undefined field_0x4cb;
	undefined field_0x4cc;
	undefined field_0x4cd;
	undefined field_0x4ce;
	undefined field_0x4cf;
	S_GLOBAL_DLIST_PATCH* pCurrentPatch;
	int nbTotalPatches;
	byte field_0x4d8;
	byte field_0x4d9;
	undefined field_0x4da;
	undefined field_0x4db;
	uint field_0x4dc;

	inline CGlobalDListPatch(uint index)
	{
		this->field_0x4dc = index;
		this->nbTotalPatches = 0;
		this->pCurrentPatch = 0;
		this->field_0x4d8 = 0;
		this->field_0x4d9 = 0;

		for (int i = 0; i < 296; i++) {
			aPatches[i] = NULL;
		}
	}

	~CGlobalDListPatch();
};

struct GlobalDlistEntry
{
	CGlobalDListPatch* pDlistPatch;
	int nbMatrices;
	int nbRegisteredPatches;
	int nbInstances;
};

struct CallFuncElement
{
	int patchId;
	int bActive;
	byte type;
	byte nbElements;
};

struct CallFuncElement2
{
	int patchId;
	int field_0x4;
	short instanceId;
	byte type;
	byte field_0xb;
};

struct edCluster
{
	undefined field_0x0;
	undefined field_0x1;
	undefined field_0x2;
	undefined field_0x3;
	undefined field_0x4;
	undefined field_0x5;
	undefined field_0x6;
	undefined field_0x7;
	undefined field_0x8;
	undefined field_0x9;
	undefined field_0xa;
	undefined field_0xb;
	undefined field_0xc;
	undefined field_0xd;
	undefined field_0xe;
	undefined field_0xf;
	ed_g3d_manager* pMeshInfo;
	uint flags;
	float distanceToCamera;
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
	undefined field_0x2c;
	undefined field_0x2d;
	undefined field_0x2e;
	undefined field_0x2f;
};

class CGlobalDListManager : public CObjectManager 
{
public:
	struct _reg_data
	{
		int patchId;
		int field_0x4;
	};

	typedef int DLIST_FUNCTION; // To be enum.

	virtual void Level_Init();
	virtual void Level_Term();

	virtual void Level_ClearAll();
	virtual void Level_Manage();
	virtual void Level_ManagePaused();
	virtual void Level_Draw();

	virtual void Level_Reset() {};

	virtual void Level_CheckpointReset() {};
	virtual void Level_SectorChange(int oldSectorId, int newSectorId);

#ifdef PLATFORM_WIN
	virtual char* ProfileGetName() { return "DList"; }
#endif

	virtual void Level_Create();

	void SectorChange(int newSectorId);
	void _ExecuteCallFunc();

	bool SetActive(int patchId, int index);
	CGlobalDListPatch* BeginCurrent(int patchId);
	void EndCurrent(int nbVertex, int param_2);

	bool _AddCallFuncElement(int patchId, DLIST_FUNCTION type, int bActive);
	bool _AddCallFuncElement2(int patchId, DLIST_FUNCTION type, short instanceId, int param_5);

	void _ExecuteCallFunc_BeginDList(_reg_data* pRegData);

	int dlistCount;
	byte bBeganLevelInit;
	byte bCompletedLevelInit;
	int field_0x1c;
	GlobalDlistEntry* ppGlobalDlist;
	DisplayList* pDisplayList;

	int field_0x14;
	int field_0x18;
	int lastPatchId;
	uint field_0x24;

	CallFuncElement aActiveCallFuncElements[400];
	int nbActiveCallFuncElements;
	CallFuncElement2 aActiveCallFuncElements2[400];
	int nbActiveCallFuncElements2;
	_reg_data aRegData[128];
	int nbRegData;
};

void GlobalDList_Init(void);
void GlobalDList_AddToView(void);

bool GameDList_BeginCurrent(void);
void GameDList_EndCurrent(void);
uint GameDListPatch_Register(CObject* pObject, int param_2, int nbInstances);
CGlobalDListPatch* GameDListPatch_BeginCurrent(int patchId);
void GameDListPatch_EndCurrent(int nbVertex, int param_2);
void GameDListPatch_HideSprite(int patchId, uint index);
void GameDListPatch_HideVertex(int patchId, int index);

bool GuiDList_BeginCurrent(void);
void GuiDList_EndCurrent(void);

bool Frontend2DDList_BeginCurrent(void);
void FrontendDList_EndCurrent(void);

extern int gpCurPatchRGBABuf;

#endif // DLIST_MANAGER_H
