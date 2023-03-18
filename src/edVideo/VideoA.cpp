#include "VideoA.h"
#include "VideoB.h"
#include "edSystem.h"
#include "VideoC.h"

#ifdef PLATFORM_WIN
#include "port.h"
#endif
#include "../Camera.h"

FrameBuffer* g_FrameBuffer_00448ee8 = NULL;
FrameBuffer* g_FrameBuffer_00448eec = NULL;

FrameBuffer* GetFrameBuffer_001ba9c0(void)
{
	return g_FrameBuffer_00448ee8;
}

FrameBuffer* GetFrameBuffer_001ba9d0(void)
{
	return g_FrameBuffer_00448eec;
}

void CleanupFunc_001ba9e0(void)
{
	if (g_FrameBuffer_00448ee8 != (FrameBuffer*)0x0) {
		DestroyFrameBuffer_002bbd40(g_FrameBuffer_00448ee8);
		g_FrameBuffer_00448ee8 = (FrameBuffer*)0x0;
	}
	if (g_FrameBuffer_00448eec != (FrameBuffer*)0x0) {
		DestroyFrameBuffer_002bbd40(g_FrameBuffer_00448eec);
		g_FrameBuffer_00448eec = (FrameBuffer*)0x0;
	}
	return;
}

void FUN_002b9e00(byte param_1, byte param_2, byte param_3)
{
	g_ActiveVidParams_0048cd90.params18.field_0x14 = param_1;
	g_ActiveVidParams_0048cd90.params18.field_0x15 = param_2;
	g_ActiveVidParams_0048cd90.params18.field_0x16 = param_3;
	if (g_ActiveVidParams_0048cd90.pCamera != (CameraObj_28*)0x0) {
		SetCameraObjBytes_002bb960(g_ActiveVidParams_0048cd90.pCamera, param_1, param_2, param_3);
	}
	return;
}

void SetVideoOffsets_002b9dd0(short inDX, short inDY)
{
	g_ActiveVidParams_0048cd90.params18.DX =
		inDX * (short)(0xa00 / g_ActiveVidParams_0048cd90.params26.screenWidth);
	g_ActiveVidParams_0048cd90.params18.DY = inDY;
	SetupDisplayGlobals_002b8c00();
	return;
}

void SetupVidParams_001baa30(short omode, short screenWidth, short screenHeight, short vidMode1, int param_5)
{
	VidParams8* pVVar1;
	VidModeData local_30;
	VidParams26 local_20;

	local_20.field_0xf = 1;
	local_20.isNTSC = (ushort)g_isNTSC;
	local_20.inter = SCE_GS_INTERLACE;
	local_20.field_0xb = 0;
	local_20.field_0xc = 0;
	local_20.bVSyncForever = 0;
	local_20.maxVblank_0xe = 1;
	local_20.field_0x10[0] = '\0';
	local_20.screenWidth = screenWidth;
	local_20.screenHeight = screenHeight;
	local_20.vidMode1 = vidMode1;
	local_20.omode = omode;
	SetupPCRTC_002b9b80(&local_20);
	if (g_FrameBuffer_00448ee8 != (FrameBuffer*)0x0) {
		DestroyFrameBuffer_002bbd40(g_FrameBuffer_00448ee8);
	}
	local_30.pixelStoreMode = SCE_GS_PSMCT32;
	local_30.field_0x6 = 2;
	local_30.flags_0x8 = 0x24;
	local_30.bUseGlobalFrameBuffer = false;
	local_30.screenWidth = screenWidth;
	local_30.screenHeight = screenHeight;
	pVVar1 = GetVidParams8_002b9e60();
	if (pVVar1->field_0x1 == 0) {
		local_30.frameBufferCount = 1;
	}
	else {
		local_30.frameBufferCount = 2;
	}
	g_FrameBuffer_00448ee8 = AllocateFrameBuffers_002bbb90(&local_30);
	if (g_FrameBuffer_00448eec != (FrameBuffer*)0x0) {
		DestroyFrameBuffer_002bbd40(g_FrameBuffer_00448eec);
	}
	if (param_5 == 2) {
		local_30.pixelStoreMode = SCE_GS_PSMCT16;
	}
	else {
		local_30.pixelStoreMode = SCE_GS_PSMCT32;
	}
	local_30.field_0x6 = 2;
	local_30.flags_0x8 = 8;
	local_30.bUseGlobalFrameBuffer = false;
	local_30.frameBufferCount = 1;
	local_30.screenWidth = screenWidth;
	local_30.screenHeight = screenHeight;
	g_FrameBuffer_00448eec = AllocateFrameBuffers_002bbb90(&local_30);
	FUN_002b9e00(0, 0, 0);
	SetVideoOffsets_002b9dd0((short)g_SetOffsetX, (short)g_SetOffsetY);
	return;
}
