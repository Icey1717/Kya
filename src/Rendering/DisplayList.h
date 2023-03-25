#ifndef _DISPLAYLIST_H
#define _DISPLAYLIST_H

#include "Types.h"

#ifdef PLATFORM_PS2
#include <libvu0.h>
#endif
#include "CameraPanMasterHeader.h"

struct CameraPanStatic_50 {
	undefined field_0x0;
	undefined field_0x1;
	undefined field_0x2;
	undefined field_0x3;
	undefined4 field_0x4;
	float field_0x8;
	float field_0xc;
	undefined4 field_0x10;
	undefined4 field_0x14;
	undefined4 field_0x18;
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
	undefined field_0x30;
	undefined field_0x31;
	undefined field_0x32;
	undefined field_0x33;
	undefined field_0x34;
	undefined field_0x35;
	undefined field_0x36;
	undefined field_0x37;
	undefined field_0x38;
	undefined field_0x39;
	undefined field_0x3a;
	undefined field_0x3b;
	undefined field_0x3c;
	undefined field_0x3d;
	undefined field_0x3e;
	undefined field_0x3f;
	undefined field_0x40;
	undefined field_0x41;
	undefined field_0x42;
	undefined field_0x43;
	undefined field_0x44;
	undefined field_0x45;
	undefined field_0x46;
	undefined field_0x47;
	undefined field_0x48;
	undefined field_0x49;
	undefined field_0x4a;
	undefined field_0x4b;
	undefined field_0x4c;
	undefined field_0x4d;
	undefined field_0x4e;
	undefined field_0x4f;
	undefined field_0x50;
	undefined field_0x51;
	undefined field_0x52;
	undefined field_0x53;
	undefined field_0x54;
	undefined field_0x55;
	undefined field_0x56;
	undefined field_0x57;
	undefined field_0x58;
	undefined field_0x59;
	undefined field_0x5a;
	undefined field_0x5b;
	undefined field_0x5c;
	undefined field_0x5d;
	undefined field_0x5e;
	undefined field_0x5f;
	undefined field_0x60;
	undefined field_0x61;
	undefined field_0x62;
	undefined field_0x63;
	undefined field_0x64;
	undefined field_0x65;
	undefined field_0x66;
	undefined field_0x67;
	undefined field_0x68;
	undefined field_0x69;
	undefined field_0x6a;
	undefined field_0x6b;
	undefined field_0x6c;
	undefined field_0x6d;
	undefined field_0x6e;
	undefined field_0x6f;
	undefined field_0x70;
	undefined field_0x71;
	undefined field_0x72;
	undefined field_0x73;
	undefined field_0x74;
	undefined field_0x75;
	undefined field_0x76;
	undefined field_0x77;
	undefined field_0x78;
	undefined field_0x79;
	undefined field_0x7a;
	undefined field_0x7b;
	undefined field_0x7c;
	undefined field_0x7d;
	undefined field_0x7e;
	undefined field_0x7f;
	undefined field_0x80;
	undefined field_0x81;
	undefined field_0x82;
	undefined field_0x83;
	undefined field_0x84;
	undefined field_0x85;
	undefined field_0x86;
	undefined field_0x87;
	undefined field_0x88;
	undefined field_0x89;
	undefined field_0x8a;
	undefined field_0x8b;
	undefined field_0x8c;
	undefined field_0x8d;
	undefined field_0x8e;
	undefined field_0x8f;
	undefined field_0x90;
	undefined field_0x91;
	undefined field_0x92;
	undefined field_0x93;
	undefined field_0x94;
	undefined field_0x95;
	undefined field_0x96;
	undefined field_0x97;
	undefined field_0x98;
	undefined field_0x99;
	undefined field_0x9a;
	undefined field_0x9b;
	undefined field_0x9c;
	undefined field_0x9d;
	undefined field_0x9e;
	undefined field_0x9f;
	undefined field_0xa0;
	undefined field_0xa1;
	undefined field_0xa2;
	undefined field_0xa3;
	undefined field_0xa4;
	undefined field_0xa5;
	undefined field_0xa6;
	undefined field_0xa7;
	undefined field_0xa8;
	undefined field_0xa9;
	undefined field_0xaa;
	undefined field_0xab;
	undefined field_0xac;
	undefined field_0xad;
	undefined field_0xae;
	undefined field_0xaf;
	undefined field_0xb0;
	undefined field_0xb1;
	undefined field_0xb2;
	undefined field_0xb3;
	undefined field_0xb4;
	undefined field_0xb5;
	undefined field_0xb6;
	undefined field_0xb7;
	undefined field_0xb8;
	undefined field_0xb9;
	undefined field_0xba;
	undefined field_0xbb;
	undefined field_0xbc;
	undefined field_0xbd;
	undefined field_0xbe;
	undefined field_0xbf;
	undefined field_0xc0;
	undefined field_0xc1;
	undefined field_0xc2;
	undefined field_0xc3;
	undefined field_0xc4;
	undefined field_0xc5;
	undefined field_0xc6;
	undefined field_0xc7;
	undefined field_0xc8;
	undefined field_0xc9;
	undefined field_0xca;
	undefined field_0xcb;
	undefined field_0xcc;
	undefined field_0xcd;
	undefined field_0xce;
	undefined field_0xcf;
	undefined field_0xd0;
	undefined field_0xd1;
	undefined field_0xd2;
	undefined field_0xd3;
	undefined field_0xd4;
	undefined field_0xd5;
	undefined field_0xd6;
	undefined field_0xd7;
	undefined field_0xd8;
	undefined field_0xd9;
	undefined field_0xda;
	undefined field_0xdb;
	undefined field_0xdc;
	undefined field_0xdd;
	undefined field_0xde;
	undefined field_0xdf;
	undefined4 field_0xe0;
	undefined4 field_0xe4;
	undefined4 field_0xe8;
	undefined4 field_0xec;
	uint3 fogCol_0xf0;
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
	//struct CameraPanStatic_100 field_0x100;
	undefined field_0x125;
	undefined field_0x126;
	undefined field_0x127;
	int field_0x128;
	int field_0x12c;
	//struct CameraPanStatic_C field_0x130;
};

struct StaticMeshMaster {
	void SetFlag_002a5400(uint flag);
	void SetFlag_002a5410(bool bValue);
	void SetFlag_002a5440(bool bValue);

	undefined4 field_0x0;
	uint flags_0x4;
	struct CameraObj_130* pCameraObj130_0x8;
	struct CameraObj_28* pCameraObj28_0xc;
	CameraPanMasterHeader field_0x10;
	CameraPanMasterHeader field_0x30;
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
	undefined field_0x1a0;
	undefined field_0x1a1;
	undefined field_0x1a2;
	undefined field_0x1a3;
	struct CameraPanMasterHeader* pFinalLink_0x1a4;
	struct CameraPanMasterHeader* field_0x1a8;
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
