#ifndef _EDDLIST_H
#define _EDDLIST_H

#include "Types.h"
#include "Rendering/DisplayListData.h"

#ifdef PLATFORM_WIN
#include "port.h"
#include "renderer.h"
#endif

struct edSysHandlerDisplayList {

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

struct DisplayList_0x10 {
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

struct MeshDrawRenderCommand {
	uint field_0x0;
	short field_0x4;
	short field_0x6;
	union edF32VECTOR4* field_0x8;
	uint field_0xc;
	edF32VECTOR4 field_0x10;
	char* field_0x20;
	char* field_0x24;
	char* field_0x28;
	char* field_0x2c;
	short field_0x30;
	short field_0x32;
	short field_0x34;
	short field_0x36;
	byte field_0x38;
	byte field_0x39;
	short field_0x3a;
	MeshDrawRenderCommand* field_0x3c;
	uint field_0x40;
	char* field_0x44;
	undefined field_0x48;
	undefined field_0x49;
	char* field_0x4a;
	undefined field_0x4e;
	undefined field_0x4f;
};

struct DisplayListInternal;
union edpkt_data;
struct edDList_material;
struct ed_viewport;

extern int gNbUsedMaterial;
extern int gCurRenderState;
extern DisplayListInternal* gCurDList;
extern edDList_material* gCurMaterial;
extern int gNbStateAdded;
extern edpkt_data* gCurStatePKT;
extern int gbDispList;
extern int gCurStatePKTSize;
extern DisplayList_0x10* gBankMaterial;

typedef enum DL_LINK_MODE {
	LM_CALL = 2,
	LM_FUNC = 4,
	LM_REF_0 = 0,
	LM_REF_1 = 5,
	LM_RELINK = 1,
	LM_SKIP = 3
} DL_LINK_MODE;

struct edDlistConfiguration {
	int field_0x0;
	int field_0x4;
	int matrixCount;
	int commandCount;
	int field_0x10;
};

extern edDlistConfiguration edDlistConfig;

extern edSysHandlerDisplayList sysHandler_0048cb90;

#ifdef PLATFORM_WIN
Renderer::TextureData MakeTextureDataFromPacket(struct ed_g2d_bitmap* pTextureBitmap, struct ed_g2d_bitmap* pPaletteBitmap, int index);
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

void edDListBegin(float x, float y, float z, ulong mode, int count);

void edDListColor4u8(byte r, byte g, byte b, byte a);

void edDListTexCoo2f(float param_1, float param_2);
void edDListVertex4f(float x, float y, float param_3, int param_4);

void edDListEnd(void);

void edDListLoadIdentity(void);

#endif //_EDDLIST_H
