#ifndef _DISPLAYLIST_H
#define _DISPLAYLIST_H

#include "Types.h"

#ifdef PLATFORM_PS2
#include <libvu0.h>
#endif
#include "CameraPanMasterHeader.h"
#include "../edList.h"

struct FrustumData {
	struct edF32MATRIX4 frustumMatrix;
	struct edF32VECTOR4 field_0x40;
	struct edF32VECTOR4 field_0x50;
};

struct ed_3D_Light_Config {
	struct edF32VECTOR4* pLightAmbient;
	struct edF32VECTOR4* pLightDirections;
	struct edF32VECTOR4* pLightColorMatrix;
};

struct ed_3D_Shadow_Config {
	undefined4 field_0x0;
	float field_0x4;
	float field_0x8;
	float field_0xc;
	struct ed_viewport* pCamera_0x10;
	struct ed_viewport* pViewport;
	undefined4 field_0x18;
	undefined4 field_0x1c;
	undefined2 renderMask;
	byte field_0x22;
	byte field_0x23;
	byte field_0x24;
};

struct SceneConfig {
	undefined* field_0x0;
	float clipValue_0x4;
	float nearClip;
	float farClip;
	uint field_0x10;
	uint field_0x14;
	float clipValue_0x18;
	undefined field_0x1c;
	undefined field_0x1d;
	undefined field_0x1e;
	undefined field_0x1f;
	struct FrustumData frustumA;
	struct FrustumData frustumB;
	struct edF32VECTOR4 field_0xe0;
	ByteColor3 fogCol_0xf0;
	undefined field_0xf3;
	undefined field_0xf4;
	undefined field_0xf5;
	undefined field_0xf6;
	undefined field_0xf7;
	undefined field_0xf8;
	undefined field_0xf9;
	undefined field_0xfa;
	undefined field_0xfb;
	undefined field_0xfc;
	undefined field_0xfd;
	undefined field_0xfe;
	undefined field_0xff;
	struct ed_3D_Shadow_Config pShadowConfig;
	undefined field_0x125;
	undefined field_0x126;
	undefined field_0x127;
	int field_0x128;
	int field_0x12c;
	struct ed_3D_Light_Config pLightConfig;
	undefined field_0x13c;
	undefined field_0x13d;
	undefined field_0x13e;
	undefined field_0x13f;
	undefined field_0x140;
	undefined field_0x141;
	undefined field_0x142;
	undefined field_0x143;
	undefined field_0x144;
	undefined field_0x145;
	undefined field_0x146;
	undefined field_0x147;
	undefined field_0x148;
	undefined field_0x149;
	undefined field_0x14a;
	undefined field_0x14b;
	undefined field_0x14c;
	undefined field_0x14d;
	undefined field_0x14e;
	undefined field_0x14f;
};

struct ed_3D_Scene {
	void RemoveFlag_002a53e0(uint flag);
	void ed3DSceneSetFlag(uint flag);
	void ed3DSceneSetFogProperty(bool bValue);
	void SetFlag_002a5440(bool bValue);

	int bShadowScene;
	uint flags_0x4;
	struct edFCamera* pCamera;
	struct ed_viewport* pViewport;
	edLIST headerA;
	edLIST headerB;
	struct SceneConfig sceneConfig;
	undefined field_0x18c;
	undefined field_0x18d;
	undefined field_0x18e;
	undefined field_0x18f;
	undefined field_0x190;
	undefined field_0x191;
	undefined field_0x192;
	undefined field_0x193;
	undefined field_0x194;
	undefined field_0x195;
	undefined field_0x196;
	undefined field_0x197;
	undefined field_0x198;
	undefined field_0x199;
	undefined field_0x19a;
	undefined field_0x19b;
	undefined field_0x19c;
	undefined field_0x19d;
	undefined field_0x19e;
	undefined field_0x19f;
	struct edLIST* field_0x1a0;
	struct edLIST* pHeirListA;
	struct edLIST* pHeirListB;
	int* field_0x1ac;
	uint heirarchyListCount;
	undefined field_0x1b4;
	undefined field_0x1b5;
	undefined field_0x1b6;
	undefined field_0x1b7;
	undefined field_0x1b8;
	undefined field_0x1b9;
	undefined field_0x1ba;
	undefined field_0x1bb;
	undefined field_0x1bc;
	undefined field_0x1bd;
	undefined field_0x1be;
	undefined field_0x1bf;
};


struct RenderCommandUint {
	uint type;
	union edpkt_data* pCommandBuffer;
	uint size;
	struct DisplayListInternal* pDisplayList;
};

struct DisplayListInternalSubObj_60 {
	struct RenderCommandUint aCommandArray[4];
	byte field_0x40;
	byte field_0x41;
	ushort type_0x42;
	union edpkt_data* pRenderInput;
	uint pCurDListBuf;
	int field_0x4c;
	uint field_0x50;
	int nbMatrix;
	struct edF32MATRIX4* pCurMatrixArray;
	undefined field_0x5c;
	undefined field_0x5d;
	undefined field_0x5e;
	undefined field_0x5f;
};


struct DisplayListInternal {
	ushort flags_0x0;
	undefined field_0x2;
	byte field_0x3;
	ushort subCommandBufferCount;
	undefined2 field_0x6;
	undefined4 field_0x8;
	char* pCommandBuffer;
	struct MeshDrawRenderCommand* field_0x10;
	union edpkt_data* field_0x14;
	union edpkt_data* pRenderCommands;
	struct DisplayListInternalSubObj_60* pDisplayListInternalSubObj;
	struct ed_3D_Scene* pStaticMeshMaster_0x20;
	float field_0x24;
	char* field_0x28;
	char* field_0x2c;
};

struct edCluster {
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
	struct ed_g3d_manager* pMeshInfo;
	uint flags;
	float field_0x18;
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

class GlobalDList {
public:
	GlobalDList();
	GlobalDList(int inField_0x8, int inField_0xc, int inField_0x10, int inField_0x1c, ed_3D_Scene* pInStaticMeshMaster);

	void Init();

	DisplayListInternal* pDisplayListInternal;
	int field_0x8;
	int field_0xc;
	int field_0x10;
	int bEnabled;
	byte field_0x18;
	byte field_0x19;
	byte field_0x1a;
	byte field_0x1b;
	int field_0x1c;
};

void GlobalDList_Init(void);
bool GuiDList_BeginCurrent(void);
void GlobalDList_AddToView(void);
void GuiDList_EndCurrent(void);

extern DisplayListInternal* gCurDListHandle;
extern struct MeshDrawRenderCommand* gCurDListBuf;

#endif // _DISPLAYLIST_H
