#ifndef _DISPLAYLIST_H
#define _DISPLAYLIST_H

#include "Types.h"

#ifdef PLATFORM_PS2
#include <libvu0.h>
#endif
#include "CameraPanMasterHeader.h"
#include "../edList.h"

union edF32VECTOR4;
union edpkt_data;

struct ed_3d_strip;
struct ed_3d_sprite;
struct ed_3D_Scene;
struct ed_g3d_manager;
struct DisplayList;

class CGlobalDListPatch;

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

class CObject;
union edVertex;

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

void GlobalDList_Init(void);
bool GuiDList_BeginCurrent(void);
void GlobalDList_AddToView(void);
void GuiDList_EndCurrent(void);

bool Frontend2DDList_BeginCurrent(void);
void FrontendDList_EndCurrent(void);

bool GameDList_BeginCurrent(void);
void GameDList_EndCurrent(void);

uint GameDListPatch_Register(CObject* pObject, int param_2, int nbInstances);
CGlobalDListPatch* GameDListPatch_BeginCurrent(int patchId);
void GameDListPatch_EndCurrent(int nbVertex, int param_2);

extern DisplayList* gCurDListHandle;
extern void* gCurDListBuf;
extern void* gCurDListBufEnd;

#endif // _DISPLAYLIST_H
