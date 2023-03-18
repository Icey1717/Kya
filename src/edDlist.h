#ifndef _EDDLIST_H
#define _EDDLIST_H

#include "Types.h"
#include "Rendering/DisplayListData.h"

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
	char* field_0x8;
	undefined field_0xc;
	undefined field_0xd;
	undefined field_0xe;
	undefined field_0xf;
};

struct MeshDrawRenderCommand {
	uint field_0x0;
	short field_0x4;
	short field_0x6;
	struct Vector* field_0x8;
	uint field_0xc;
	struct Vector field_0x10;
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
struct RenderCommand;
struct MaterialInfo;
struct CameraObj_28;

namespace edDlist
{
	typedef enum DL_LINK_MODE {
		LM_CALL = 2,
		LM_FUNC = 4,
		LM_REF_0 = 0,
		LM_REF_1 = 5,
		LM_RELINK = 1,
		LM_SKIP = 3
	} DL_LINK_MODE;

	extern DisplayList_0x10* g_DisplayListPtr_0044965c;
	extern int g_DisplayListObjCount_004250e0;
	extern int MaterialDisplayListCount_00449648;
	extern int g_DisplayListGlobal_00449664;
	extern DisplayListInternal* g_DisplayListEnd_004495d8;
	extern MaterialInfo* g_pMaterialInfo_00449644;
	extern int g_DisplayListCommandCount_004495d4;
	extern RenderCommand* g_CachedRenderCommandPtr_00449650;
	extern DisplayListData DisplayListData_004496a0;
	extern int g_HasUIData_00448a88;
	extern int INT_00449654;
	extern edSysHandlerDisplayList sysHandler_0048cb90;

	void Init(void);

	void CopyMatrixToDisplayList_002d07d0(Matrix* m0);

	void LoadMaterialResource_002cb850(struct MaterialInfo* pMaterialInfo);

	void RenderCommand_002ca6a0(ulong param_1, ulong param_2, ulong param_3, ulong param_4, ulong param_5, ulong param_6, ulong param_7,
		ulong param_8);

	void RenderCommand_002ca800(void);
	void RenderCommand_002ca8c0(uint mode);
	void FUN_002ca8c0(uint mode);

	void RenderCommand_002ca930(CameraObj_28* pCamera);

	void WillSetActiveDisplayList_002cac70(DisplayListInternal* param_1);
	bool WillSetActiveDisplayList_00290cb0(CameraPanMasterHeader* pCameraPanHeader, DisplayListInternal* pDisplayList);

	void MeshDrawCommands_002ca170(float x, float y, float z, ulong mode, int count);

	void SetDropShadowColour_002ce1a0(byte r, byte g, byte b, byte a);

	void CallpcGpffff9200(float param_1, float param_2);
	void CallpcGpffff91f8(float x, float y, float param_3, int param_4);

	void EndDraw_002cfe40(void);
}
#endif //_EDDLIST_H
