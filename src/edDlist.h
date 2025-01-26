#ifndef _EDDLIST_H
#define _EDDLIST_H

#include "Types.h"
#include "Rendering/DisplayListData.h"

#ifdef PLATFORM_WIN
#include "port.h"
#include "renderer.h"
#endif

#define PRIM_TYPE_TRIANGLE_LIST 4
#define PRIM_TYPE_SPRITE 6

struct edSysHandlerDisplayList
{
	edSysHandlerDisplayList(struct edSysHandlersNodeTable* inNodeParent, int inMaxEventID, int inMainIdentifier)
		: nodeParent(inNodeParent)
		, maxEventID(inMaxEventID)
		, mainIdentifier(inMainIdentifier)
	{

	}

	struct edSysHandlersNodeTable* nodeParent;
	struct edSysHandlersPoolEntry* entries[5];
	int maxEventID;
	int mainIdentifier;
};

struct DisplayList_0x10
{
	undefined field_0x0;
	undefined field_0x1;
	undefined field_0x2;
	undefined field_0x3;
	undefined field_0x4;
	undefined field_0x5;
	undefined field_0x6;
	undefined field_0x7;
	struct TextureData_HASH_Internal_MAT* pHASH_MAT;
	undefined field_0xc;
	undefined field_0xd;
	undefined field_0xe;
	undefined field_0xf;
};

struct DisplayListInternal;
union edpkt_data;
struct edDList_material;
struct ed_viewport;
struct ed_3D_Scene;
struct ed_g2d_manager;
struct ed_g2d_material;
struct TextureData_HASH_Internal_MAT;
struct ed_hash_code;
struct ed_dma_material;

extern int gNbUsedMaterial;
extern int gCurRenderState;
extern DisplayListInternal* gCurDList;
extern edDList_material* gCurMaterial;
extern int gNbStateAdded;
extern int gbDispList;
extern int gCurStatePKTSize;
extern ed_hash_code* gBankMaterial;

typedef enum DL_LINK_MODE
{
	LM_CALL = 2,
	LM_FUNC = 4,
	LM_REF_0 = 0,
	LM_REF_1 = 5,
	LM_RELINK = 1,
	LM_SKIP = 3
} DL_LINK_MODE;

struct edDlistConfiguration
{
	int field_0x0;
	int field_0x4;
	int matrixCount;
	int commandCount;
	int bankMaterialCount;
};

struct edDList_material
{
	ed_g2d_manager* pManager;
	ed_g2d_material* pMaterial;
	int mode;
	int index;
};

extern edDlistConfiguration edDlistConfig;

extern edSysHandlerDisplayList sysHandler_0048cb90;

#ifdef PLATFORM_WIN
Renderer::TextureData MakeTextureDataFromPacket(ed_g2d_material* pMaterial, struct ed_g2d_bitmap* pTextureBitmap, struct ed_g2d_bitmap* pPaletteBitmap, int index);
#endif

void edDListInit(void);

void edDListLoadMatrix(edF32MATRIX4* m0);

void edDListUseMaterial(struct edDList_material* pMaterialInfo);

void edDListAlphaTestAndZTest(ulong param_1, ulong param_2, ulong param_3, ulong param_4, ulong param_5, ulong param_6, ulong param_7,
	ulong param_8);

struct ed_g2d_bitmap* edDListGetG2DBitmap(struct ed_g2d_material* pMAT, int offset, bool* bHasPalette, struct ed_g2d_bitmap** pOutAddr);

void edDListSetState(ulong cmdA, ulong cmdB);

void edDListBlendFuncNormal(void);
void edDListBlendSet(uint mode);
void edDListBlendFunc50(void);

void edDListSetActiveViewPort(ed_viewport* pCamera);

void edDlistAddtoView(DisplayListInternal* param_1);

void edDListBegin(float x, float y, float z, uint mode, int count);

void edDListColor4u8(byte r, byte g, byte b, byte a);

void edDListTexCoo2f(float param_1, float param_2);
void edDListVertex4f(float x, float y, float param_3, uint param_4);
void edDListSetProperty(uint type, uint value);

void edDListEnd(void);

void edDListLoadIdentity(void);

DisplayListInternal* edDListSetCurrent(DisplayListInternal* pNewDisplayList);
DisplayListInternal* edDListNew(EHeap heapID, uint inFlags, int param_3, int param_4, int param_5, undefined8 param_6, DisplayListInternal* pInBuffer);
void edDListSetSceneUsed(DisplayListInternal* pDisplayListInternalArray, ed_3D_Scene* pStaticMeshMaster);
uint edDListGetBufSizeNeeded(uint param_1, int param_2, int param_3, int param_4, uint* param_5, uint* param_6);

void edDListInitMaterial(edDList_material* outObj, ed_hash_code* pHASH_MAT, ed_g2d_manager* textureInfoObj, uint mode);
bool edDListTermMaterial(edDList_material* pMaterial);

void edDListDeleteFrameBufferMaterial(edDList_material* pMaterial);
edDList_material* edDListCreatMaterialFromIndex(edDList_material* pMaterialInfo, int index, ed_g2d_manager* pTextureInfo, uint mode);
edDList_material* edDListCreatMaterialFromHashCode(edDList_material* pMaterial, ulong hashCode, ed_g2d_manager* pManager, uint mode);

#ifdef PLATFORM_WIN
Multidelegate<edDList_material*>& edDListGetMaterialLoadedDelegate();
Multidelegate<edDList_material*>& edDListGetMaterialUnloadedDelegate();
#endif

#endif //_EDDLIST_H
