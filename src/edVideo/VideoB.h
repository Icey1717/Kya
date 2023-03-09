#ifndef VIDEO_B_H
#define VIDEO_B_H

#include "Types.h"

#ifdef PLATFORM_PS2
#include <libgraph.h>
#endif

#ifdef PLATFORM_WIN
typedef ulong tGS_DISPFB2;
typedef ulong tGS_DISPFB1;
typedef ulong tGS_DISPLAY2;
typedef ulong tGS_DISPLAY1;
#endif

struct VidParams8 {
	byte field_0x0;
	byte field_0x1;
	byte field_0x2;
	byte field_0x3;
	byte field_0x4;
	undefined field_0x5;
	undefined field_0x6;
	undefined field_0x7;
};

struct VidParams26 {
	ushort screenWidth;
	ushort screenHeight;
	ushort vidMode1;
	ushort omode;
	ushort isNTSC;
	byte inter;
	byte field_0xb;
	byte field_0xc;
	byte field_0xd;
	byte field_0xe;
	byte field_0xf;
	char field_0x10[4];
	int field_0x14;
	byte field_0x18;
	byte field_0x19;
};

struct VidParams18 {
	short gs_pmode;
	short gs_smode2;
	short field_0x4;
	short field_0x6;
	short field_0x8;
	short field_0xa;
	uint field_0xc;
	undefined field_0x10;
	undefined field_0x11;
	undefined field_0x12;
	undefined field_0x13;
	byte field_0x14;
	byte field_0x15;
	byte field_0x16;
	byte ffmode;
};

struct VidModeData_20 {
	tGS_DISPFB2 dispfb2;
	tGS_DISPFB1 dispfb1;
	byte field_0x10;
	undefined field_0x11;
	undefined field_0x12;
	undefined field_0x13;
	undefined field_0x14;
	undefined field_0x15;
	undefined field_0x16;
	undefined field_0x17;
	undefined field_0x18;
	undefined field_0x19;
	undefined field_0x1a;
	undefined field_0x1b;
	undefined field_0x1c;
	undefined field_0x1d;
	undefined field_0x1e;
	undefined field_0x1f;
};

struct VidModeData_30 {
	struct VidModeData* pVidModeData_0x0;
	undefined field_0x4;
	undefined field_0x5;
	undefined field_0x6;
	undefined field_0x7;
	uint size_0x8;
	char* data_0xc;
	undefined field_0x10;
	undefined field_0x11;
	undefined field_0x12;
	undefined field_0x13;
	struct VideModeData_30_30* pVidModeData30_30;
	struct VidModeData_20* pVidModeData20;
	struct VidModeData_30* pLink_0x1c;
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

struct VideoListData_AC {
	ulong field_0x0[20];
	ulong* field_0xa0;
	ulong* field_0xa4;
	ulong* field_0xa8;
};

struct edVideoData {
	struct VidParams26 params26;
	undefined field_0x1a;
	undefined field_0x1b;
	undefined field_0x1c;
	undefined field_0x1d;
	undefined field_0x1e;
	undefined field_0x1f;
	struct VidParams18 params18;
	undefined field_0x38;
	undefined field_0x39;
	undefined field_0x3a;
	undefined field_0x3b;
	undefined field_0x3c;
	undefined field_0x3d;
	undefined field_0x3e;
	undefined field_0x3f;
	tGS_DISPLAY2 disp2;
	tGS_DISPLAY1 disp1;
	byte field_0x50;
	byte field_0x51;
	byte field_0x52;
	undefined field_0x53;
	uint field_0x54;
	undefined field_0x58;
	undefined field_0x59;
	undefined field_0x5a;
	undefined field_0x5b;
	undefined field_0x5c;
	undefined field_0x5d;
	undefined field_0x5e;
	undefined field_0x5f;
	struct VidModeData_30* data30;
	struct CameraObj_28* pCamera;
	byte field_0x68;
	byte field_0x69;
	byte field_0x6a;
	byte field_0x6b;
	undefined4 field_0x6c;
	undefined4 field_0x70;
	undefined4 field_0x74;
	byte field_0x78;
	byte field_0x79;
	byte field_0x7a;
	byte field_0x7b;
	undefined4 field_0x7c;
	undefined4 field_0x80;
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
	struct VideoListData_AC listDataAC;
	undefined field_0x13c;
	undefined field_0x13d;
	undefined field_0x13e;
	undefined field_0x13f;
};

struct VidModeData {
	ushort screenWidth;
	ushort screenHeight;
	ushort mode_0x4;
	ushort field_0x6;
	ushort field_0x8;
	char field_0xa;
	byte count_0xb;
	struct VidModeData_30* pLink_0xc;
};

struct VideModeData_30_30 {
	undefined8 field_0x0;
	undefined8 field_0x8;
	ulong field_0x10;
	undefined8 field_0x18;
	undefined8 field_0x20;
	undefined8 field_0x28;
};

void SetActiveVidModeData_002b9b10(VidModeData_30* param_1);
void SetupPCRTC_002b9b80(VidParams26* param_1);
VidParams8* GetVidParams8_002b9e60(void);
void Init_edVideo_002b9e70(void);

extern VidParams8 g_VidParams8_00449588;
extern edVideoData g_ActiveVidParams_0048cd90;

#endif //VIDEO_B_H
