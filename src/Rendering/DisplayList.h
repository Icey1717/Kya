#ifndef _DISPLAYLIST_H
#define _DISPLAYLIST_H

#include "Types.h"

#ifdef PLATFORM_PS2
#include <libvu0.h>
#endif

struct StaticMeshMaster {
	undefined4 field_0x0;
	uint flags_0x4;
	//struct CameraObj_130* pCameraObj130_0x8;
	//struct CameraObj_28* pCameraObj28_0xc;
	//struct CameraPanMasterHeader field_0x10;
	//struct CameraPanMasterHeader field_0x30;
	//struct CameraPanStatic_50 field_0x50;
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
	undefined field_0x1a0;
	undefined field_0x1a1;
	undefined field_0x1a2;
	undefined field_0x1a3;
	//struct CameraPanMasterHeader* pFinalLink_0x1a4;
	//struct CameraPanMasterHeader* field_0x1a8;
	undefined field_0x1ac;
	undefined field_0x1ad;
	undefined field_0x1ae;
	undefined field_0x1af;
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

struct RenderCommand {
	ulong cmdA;
	ulong cmdB;
};

struct DisplayListInternalSubObj_60 {
	struct RenderCommandUint aCommandArray[4];
	undefined field_0x40;
	undefined field_0x41;
	ushort type_0x42;
	struct RenderCommand* pRenderInput;
	uint field_0x48;
	undefined field_0x4c;
	undefined field_0x4d;
	undefined field_0x4e;
	undefined field_0x4f;
	undefined field_0x50;
	undefined field_0x51;
	undefined field_0x52;
	undefined field_0x53;
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
	char* field_0x10;
	struct RenderCommand* field_0x14;
	struct RenderCommand* pRenderCommands;
	struct DisplayListInternalSubObj_60* pDisplayListInternalSubObj;
	struct StaticMeshMaster* pStaticMeshMaster_0x20;
	float field_0x24;
	char* field_0x28;
	char* field_0x2c;
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

#endif // _DISPLAYLIST_H
