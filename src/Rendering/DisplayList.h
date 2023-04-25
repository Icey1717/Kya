#ifndef _DISPLAYLIST_H
#define _DISPLAYLIST_H

#include "Types.h"

#ifdef PLATFORM_PS2
#include <libvu0.h>
#endif
#include "CameraPanMasterHeader.h"

struct ScratchpadSubObj_60 {
	struct Matrix field_0x0;
	struct Vector field_0x40;
	struct Vector field_0x50;
};

struct CameraPanStatic_C {
	struct Vector* field_0x0;
	struct Vector* field_0x4;
	struct Vector* field_0x8;
};

struct CameraPanStatic_100 {
	undefined4 field_0x0;
	float field_0x4;
	float field_0x8;
	float field_0xc;
	struct CameraObj_28* pCamera_0x10;
	struct CameraObj_28* pCamera_0x14;
	undefined4 field_0x18;
	undefined4 field_0x1c;
	undefined2 field_0x20;
	byte field_0x22;
	byte field_0x23;
	byte field_0x24;
};

struct CameraPanStatic_50 {
	undefined* field_0x0;
	float field_0x4;
	float nearClip;
	float farClip;
	uint field_0x10;
	uint field_0x14;
	float field_0x18;
	undefined field_0x1c;
	undefined field_0x1d;
	undefined field_0x1e;
	undefined field_0x1f;
	struct ScratchpadSubObj_60 field_0x20;
	struct ScratchpadSubObj_60 field_0x80;
	struct Vector field_0xe0;
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
	struct CameraPanStatic_100 field_0x100;
	undefined field_0x125;
	undefined field_0x126;
	undefined field_0x127;
	int field_0x128;
	int field_0x12c;
	struct CameraPanStatic_C field_0x130;
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

struct StaticMeshMaster {
	void RemoveFlag_002a53e0(uint flag);
	void SetFlag_002a5400(uint flag);
	void SetFlag_002a5410(bool bValue);
	void SetFlag_002a5440(bool bValue);

	int isChild;
	uint flags_0x4;
	struct CameraObj_130* pCameraObj130_0x8;
	struct CameraObj_28* pCameraObj28_0xc;
	CameraPanMasterHeader headerA;
	CameraPanMasterHeader headerB;
	struct CameraPanStatic_50 field_0x50;
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
	struct CameraPanMasterHeader* field_0x1a0;
	struct CameraPanMasterHeader* pFinalLink_0x1a4;
	struct CameraPanMasterHeader* field_0x1a8;
	int* field_0x1ac;
	uint count_0x1b0;
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
	struct RenderCommand* pCommandBuffer;
	uint size;
	struct DisplayListInternal* pDisplayList;
};

struct DisplayListInternalSubObj_60 {
	struct RenderCommandUint aCommandArray[4];
	byte field_0x40;
	byte field_0x41;
	ushort type_0x42;
	struct RenderCommand* pRenderInput;
	uint field_0x48;
	int field_0x4c;
	uint field_0x50;
	int field_0x54;
	struct Matrix* field_0x58;
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
	char* field_0xc;
	struct MeshDrawRenderCommand* field_0x10;
	struct RenderCommand* field_0x14;
	struct RenderCommand* pRenderCommands;
	struct DisplayListInternalSubObj_60* pDisplayListInternalSubObj;
	struct StaticMeshMaster* pStaticMeshMaster_0x20;
	float field_0x24;
	char* field_0x28;
	char* field_0x2c;
};

struct DisplayListInternalMesh {
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
	struct MeshInfo* pMeshInfo;
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

class DisplayList {
public:
	DisplayList();
	DisplayList(int inField_0x8, int inField_0xc, int inField_0x10, int inField_0x1c, StaticMeshMaster* pInStaticMeshMaster);

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

void SetupDisplayLists(void);
bool GuiDisplayListFunc_002d6360(void);
void ActivateDisplayLists_002d6490(void);
void DisplayListFunc_002d6340(void);

extern DisplayListInternal* g_CurrentDisplayListBase_004495dc;
extern struct MeshDrawRenderCommand* PTR_MeshDrawRenderCommand_004495fc;

#endif // _DISPLAYLIST_H
