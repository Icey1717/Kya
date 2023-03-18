#include "VideoB.h"
#include "VideoA.h"
#include "LargeObject.h"
#include <assert.h>
#include <string.h>
#include "VideoC.h"
#include "VideoD.h"
#include "edMem.h"

#ifdef PLATFORM_WIN
#include "port.h"
#endif

#include "Rendering/CustomShell.h"

VidParams8 g_VidParams8_00449588 = { 0 };
edVideoData g_ActiveVidParams_0048cd90 = { 0 };

void SetActiveFrameBuffer_002b9b10(FrameBuffer* pNewFrameBuffer)
{
	pNewFrameBuffer->pVidModeData_0x0->pLink_0xc = pNewFrameBuffer;
	g_ActiveVidParams_0048cd90.pFrameBuffer = pNewFrameBuffer;
	UpdateCameraCommands_002b8f70(pNewFrameBuffer);
	return;
}

void CleanupFunc_002b9b30(void)
{
	if (g_ActiveVidParams_0048cd90.pFrameBuffer != (FrameBuffer*)0x0) {
		DestroyFrameBuffer_002bbd40(g_ActiveVidParams_0048cd90.pFrameBuffer);
		g_ActiveVidParams_0048cd90.pFrameBuffer = (FrameBuffer*)0x0;
	}
	//FreeObject_002bb070((long)(int)g_CameraObj_28_0048cdf4, 0);
	g_ActiveVidParams_0048cd90.pCamera = (CameraObj_28*)0x0;
	return;
}

#ifdef PLATFORM_PS2
#include "edPacket.h"
#endif

void ColorPacketFunc_002b9160(ulong r, ulong g, ulong b, ulong a)
{
#ifdef PLATFORM_PS2
	ulonglong* pBuffer;
	edPacket eStack32;

	pBuffer = (ulonglong*)GetScratchPadPtr_00424e10();
	if (pBuffer != (ulonglong*)0x0) {
		edPacketInit(&eStack32, 0xa0, pBuffer);
		edPacketWriteLong_0026a000(&eStack32, 0x1000000000008009);
		edPacketWriteLong_0026a000(&eStack32, 0xe);
		edPacketWriteLong_0026a000(&eStack32, 1);
		edPacketWriteLong_0026a000(&eStack32, 0x1a);
		edPacketWriteLong_0026a000(&eStack32, 0x100000);
		edPacketWriteLong_0026a000(&eStack32, 0x4c);
		edPacketWriteLong_0026a000(&eStack32, 0);
		edPacketWriteLong_0026a000(&eStack32, 0x18);
		edPacketWriteLong_0026a000(&eStack32, 0x400000004000000);
		edPacketWriteLong_0026a000(&eStack32, 0x40);
		edPacketWriteLong_0026a000(&eStack32, 0);
		edPacketWriteLong_0026a000(&eStack32, 0x47);
		edPacketWriteLong_0026a000(&eStack32, 6);
		edPacketWriteLong_0026a000(&eStack32, 0);
		edPacketWriteLong_0026a000
		(&eStack32, (a & 0xff) << 0x18 | (b & 0xff) << 0x10 | r & 0xff | (g & 0xff) << 8 | 0x3f80000000000000);
		edPacketWriteLong_0026a000(&eStack32, 1);
		edPacketWriteLong_0026a000(&eStack32, 0);
		edPacketWriteLong_0026a000(&eStack32, 4);
		edPacketWriteLong_0026a000(&eStack32, 0x40004000);
		edPacketWriteLong_0026a000(&eStack32, 4);
		edPacketClose_00269e70(&eStack32);
		shellDmaStart(SHELLDMA_CHANNEL_GIF, eStack32.pBuffer, eStack32.size_0x14);
	}
#endif
	return;
}

void FUN_002b9740(void)
{
	int iVar1;
	int iVar2;

	iVar2 = (0x800 - ((int)(uint)g_ActiveVidParams_0048cd90.params26.screenWidth >> 1)) * 0x10;
	iVar1 = (0x800 - ((int)(uint)g_ActiveVidParams_0048cd90.params26.screenHeight >> 1)) * 0x10;
	g_ActiveVidParams_0048cd90.field_0x69 = 0;
	g_ActiveVidParams_0048cd90.field_0x6a = 0;
	g_ActiveVidParams_0048cd90.field_0x6b = 0;
	g_ActiveVidParams_0048cd90.field_0x80 = 0;
	g_ActiveVidParams_0048cd90.listDataAC.field_0x0[0] = 0x1000000000008009;
	g_ActiveVidParams_0048cd90.listDataAC.field_0x0[1] = 0xe;
	g_ActiveVidParams_0048cd90.listDataAC.field_0x0[2] = 0x30003;
	g_ActiveVidParams_0048cd90.listDataAC.field_0x0[3] = 0x47;
	g_ActiveVidParams_0048cd90.listDataAC.field_0x0[4] = 0x44;
	g_ActiveVidParams_0048cd90.listDataAC.field_0x0[5] = 0x42;
	g_ActiveVidParams_0048cd90.listDataAC.field_0x0[6] = 0;
	g_ActiveVidParams_0048cd90.listDataAC.field_0x0[7] = 0x49;
	g_ActiveVidParams_0048cd90.listDataAC.field_0x0[8] = 0;
	g_ActiveVidParams_0048cd90.listDataAC.field_0x0[9] = 0x4a;
	g_ActiveVidParams_0048cd90.listDataAC.field_0x0[10] = 0x4e;
	g_ActiveVidParams_0048cd90.listDataAC.field_0x0[11] = 0;
	g_ActiveVidParams_0048cd90.listDataAC.field_0x0[12] =
		(long)(int7)((uint7)g_ActiveVidParams_0048cd90.field_0x6c << 0x18) | 0x3f80000000000000;
	g_ActiveVidParams_0048cd90.listDataAC.field_0x0[13] = 1;
	g_ActiveVidParams_0048cd90.listDataAC.field_0x0[14] = (long)iVar2 & 0xffffffffU | ((long)iVar1 & 0xffffffffU) << 0x10;
	g_ActiveVidParams_0048cd90.listDataAC.field_0x0[15] = 4;
	g_ActiveVidParams_0048cd90.listDataAC.field_0x0[16] =
		(long)(int)(iVar2 + (uint)g_ActiveVidParams_0048cd90.params26.screenWidth * 0x10) & 0xffffffffU |
		((long)(int)(iVar1 + (uint)g_ActiveVidParams_0048cd90.params26.screenHeight * 0x10) & 0xffffffffU) << 0x10;
	g_ActiveVidParams_0048cd90.listDataAC.field_0x0[17] = 4;
	g_ActiveVidParams_0048cd90.listDataAC.field_0x0[18] = 0x70000;
	g_ActiveVidParams_0048cd90.listDataAC.field_0x0[19] = 0x47;
	g_ActiveVidParams_0048cd90.listDataAC.field_0xa0 = (ulong*)((char*)&g_ActiveVidParams_0048cd90 + 0xf0);
	g_ActiveVidParams_0048cd90.listDataAC.field_0xa4 = (ulong*)((char*)&g_ActiveVidParams_0048cd90 + 0x100);
	g_ActiveVidParams_0048cd90.listDataAC.field_0xa8 = (ulong*)((char*)&g_ActiveVidParams_0048cd90 + 0x110);
	return;
}


void SetupPCRTC_002b9b80(VidParams26* pInVidParams)
{
	int canInterlace;
	int iVar3;
	ulong uVar1;
	int iVar2;
	bool bShouldInterlace;

	if (pInVidParams == (VidParams26*)0x0) {
		pInVidParams = &g_ActiveVidParams_0048cd90.params26;
	}
	g_ActiveVidParams_0048cd90.params26 = *pInVidParams;
#ifdef PLATFORM_PS2
	sceGsSyncV(0);
#endif
	ColorPacketFunc_002b9160(0, 0, 0, 0);
	canInterlace = GetCanInterlace_002b8b90();
	bShouldInterlace = canInterlace != 0;
	if (bShouldInterlace) {
		bShouldInterlace = pInVidParams->inter != SCE_GS_NOINTERLACE;
	}
	g_ActiveVidParams_0048cd90.params18.ffmode = bShouldInterlace ^ 1;
	if (pInVidParams->field_0x10[0] != '\0') {
#ifdef PLATFORM_PS2
		sceGsResetGraph(0, (ushort)pInVidParams->inter, g_ActiveVidParams_0048cd90.params26.omode, (ushort)g_ActiveVidParams_0048cd90.params18.ffmode);
#endif
	}
#ifdef PLATFORM_PS2
	uVar1 = sceGsGetIMR();
	sceGsPutIMR(uVar1 & 0xfffffffffffffeff);
	uVar1 = sceGsGetIMR();
	sceGsPutIMR(uVar1 & 0xfffffffffffffdff);
#endif
	g_ActiveVidParams_0048cd90.params18.unused_0xc = 0;
	g_ActiveVidParams_0048cd90.params18.gs_pmode = g_VidParams8_00449588.field_0x1 | 0x8066;
	g_ActiveVidParams_0048cd90.params18.gs_smode2 = (ushort)pInVidParams->inter | (ushort)g_ActiveVidParams_0048cd90.params18.ffmode << 1;
	g_ActiveVidParams_0048cd90.params18.DX = 0;
	g_ActiveVidParams_0048cd90.params18.DH = (pInVidParams->screenHeight << (g_ActiveVidParams_0048cd90.params18.ffmode & 0x1f)) + -1;
	g_ActiveVidParams_0048cd90.params18.DY = 0;
	g_ActiveVidParams_0048cd90.params18.MAGH = (short)((pInVidParams->screenWidth + 0x9ff) / (uint)pInVidParams->screenWidth) + -1;
	FUN_002b8f30();
	SetupDisplayGlobals_002b8c00();
#ifdef PLATFORM_PS2
	iVar2 = 0;
	do {
		iVar3 = sceGsSyncV(0);
		iVar2 = iVar2 + 1;
	} while (iVar2 < 0x14);
#endif
	return;
}

VidParams8* GetVidParams8_002b9e60(void)
{
	return &g_VidParams8_00449588;
}

#ifdef PLATFORM_PS2
void SetupPrivilegedGSRegisters(void)
{
	WaitForVSync(0);
	/* Setup CRT Controller */
	DPUT_GS_PMODE((ulong)(ushort)g_ActiveVidParams_0048cd90.params18.gs_pmode);
	/* CRTC Video Settings */
	DPUT_GS_SMODE2((ulong)(ushort)g_ActiveVidParams_0048cd90.params18.gs_smode2);
	/* RC2 data source settings */
	DPUT_GS_DISPFB2(0x1400);
	/* RC2 display output settings */
	DPUT_GS_DISPLAY2(*(u_long*)&g_ActiveVidParams_0048cd90.disp2);
	/* Set CRTC background color */
	DPUT_GS_BGCOLOR(0);
	return;
}

void ResetVideoData_002ba560(void)
{
	g_ActiveVidParams_0048cd90.vblankCount = 0;
	g_ActiveVidParams_0048cd90.bWaitingForVSync = 0;
	return;
}

ulong SetIMR_00227ea8(ulong flag)
{
	ulong prevIMR;

	prevIMR = *GS_IMR;
	DPUT_GS_IMR(flag);
	return prevIMR;
}

int GSHandler_002ba6d0(int cause)
{
	void* pvVar1;
	int eventID;
	uint uVar2;
	ulong flag;
	ulong uVar4;
	undefined8 uVar3;

	flag = SetIMR_00227ea8(0xff00);
	uVar2 = DGET_GS_CSR();
	uVar3 = DGET_GS_SIGLBLID();
	uVar4 = DGET_GS_SIGLBLID();
	pvVar1 = (void*)((ulong)uVar3 >> 0x20);
	eventID = (int)uVar4;
	if ((uVar2 & 1) != 0) {
		DPUT_GS_CSR((ulong)(int)(uVar2 & 0xfffffcf0) & 0xffffffffU | 1);
		__asm__ volatile(" sync.l ");
		if ((((eventID == 8) || (eventID == 5)) || (eventID == 4)) || (eventID == 1)) {
			edSysHandlersCall(edSysHandlerVideo_0048cee0.mainIdentifier, edSysHandlerVideo_0048cee0.entries,
				edSysHandlerVideo_0048cee0.maxEventID, eventID, pvVar1);
		}
	}
	if ((uVar2 & 2) != 0) {
		DPUT_GS_CSR((ulong)(int)(uVar2 & 0xfffffcf0) & 0xffffffffU | 2);
		__asm__ volatile(" sync.l ");
		edSysHandlersCall(edSysHandlerVideo_0048cee0.mainIdentifier, edSysHandlerVideo_0048cee0.entries,
			edSysHandlerVideo_0048cee0.maxEventID, 2, pvVar1);
	}
	SetIMR_00227ea8(flag);
	ExitHandler();
	return 0;
}


int VBlankHandler_002ba580(int cause)
{
	ulong uVar1;

	edSysHandlersCall(edSysHandlerVideo_0048cee0.mainIdentifier, edSysHandlerVideo_0048cee0.entries,
		edSysHandlerVideo_0048cee0.maxEventID, 3, (void*)0x0);
	if (g_ActiveVidParams_0048cd90.params26.bVSyncForever == 0) {
		if (g_ActiveVidParams_0048cd90.vblankCount == 0) {
			g_ActiveVidParams_0048cd90.lastCount = GetCountValue();
		}
		g_ActiveVidParams_0048cd90.vblankCount = g_ActiveVidParams_0048cd90.vblankCount + 1;
	}
	else {
		g_ActiveVidParams_0048cd90.vblankCount = g_ActiveVidParams_0048cd90.vblankCount + 1;
		if (g_ActiveVidParams_0048cd90.params26.maxVblank_0xe <= g_ActiveVidParams_0048cd90.vblankCount) {
			if ((((g_ActiveVidParams_0048cd90.field_0x52 == 0) || (g_ActiveVidParams_0048cd90.params26.inter == SCE_GS_NOINTERLACE)) ||
				(g_ActiveVidParams_0048cd90.params18.ffmode == SCE_GS_FIELD)) ||
				(g_ActiveVidParams_0048cd90.pFrameBuffer == (FrameBuffer*)0x0)) {
				ResetVideoData_002ba560();
			}
			else {
				uVar1 = DGET_GS_CSR();
				if ((uint)((uVar1 & 0x2000) >> 0xd) ==
					(uint)(g_ActiveVidParams_0048cd90.pFrameBuffer)->pVidModeData_0x0->pLink_0xc->pVidModeData20->csrValue_0x10)
				{
					ResetVideoData_002ba560();
				}
			}
		}
	}
	ExitHandler();
	return 1;
}

char* g_szInterruptError_00433440 = "ERROR: Adding Interrupt Handler! ";

#define HANDLER_NEXT 0
#define HANDLER_LAST -1

void AddVBlankInterruptHandler_002ba490(int (*handler)(int))
{
	int iVar1;

	iVar1 = AddIntcHandler(INTC_VBLANK_S, handler, HANDLER_LAST);
	if (iVar1 < 0) {
		PrintString(g_szInterruptError_00433440);
	}
	else {
		EnableIntc(INTC_VBLANK_S);
	}
	return;
}

void AddGSInterruptHandler_002ba4e0(int (*handler)(int))
{
	int iVar1;
	ulong uVar2;

	iVar1 = AddIntcHandler(INTC_GS, handler, HANDLER_NEXT);
	if (iVar1 < 0) {
		PrintString(g_szInterruptError_00433440);
	}
	else {
		DPUT_GS_IMR(*GS_IMR & 0xfffffffffffffeff);
		DPUT_GS_IMR(*GS_IMR & 0xfffffffffffffdff);
		EnableIntc(INTC_GS);
	}
	return;
}
#endif

void Init_edVideo_002b9e70(void)
{
	int iVar1;
	VidParams26 local_20;

	local_20.screenWidth = 0x200;
	local_20.screenHeight = 0x200;
	local_20.vidMode1 = 0;
	local_20.omode = SCE_GS_PAL;
	local_20.isNTSC = 1;
	local_20.inter = SCE_GS_INTERLACE;
	local_20.field_0xb = 1;
	local_20.field_0xc = 1;
	local_20.bVSyncForever = 1;
	local_20.maxVblank_0xe = 1;
	local_20.field_0xf = 1;
	local_20.field_0x10[0] = '\x01';
	memset(&g_ActiveVidParams_0048cd90, 0, sizeof(edVideoData));
	local_20.screenWidth = 0x280;
	local_20.screenHeight = 0x100;
	local_20.vidMode1 = 0;
	local_20.omode = SCE_GS_PAL;
	local_20.isNTSC = 1;
	local_20.inter = SCE_GS_INTERLACE;
	local_20.field_0xb = 1;
	local_20.field_0xc = 1;
	local_20.bVSyncForever = 1;
	local_20.maxVblank_0xe = 1;
	local_20.field_0x10[0] = g_VidParams8_00449588.field_0x4;
	SetupPCRTC_002b9b80(&local_20);
	g_ActiveVidParams_0048cd90.vblankCount = 0;
	g_ActiveVidParams_0048cd90.bWaitingForVSync = 0;
	g_ActiveVidParams_0048cd90.field_0x68 = 0;
#ifdef PLATFORM_PS2
	AddVBlankInterruptHandler_002ba490(VBlankHandler_002ba580);
	AddGSInterruptHandler_002ba4e0(GSHandler_002ba6d0);
	sceGsSyncV(0);
#endif
	ColorPacketFunc_002b9160(0, 0, 0, 0x80);
#ifdef PLATFORM_PS2
	SetupPrivilegedGSRegisters();
	iVar1 = 0;
	do {
		sceGsSyncV(0);
		iVar1 = iVar1 + 1;
	} while (iVar1 < 0x14);
#endif
	UpdateCameraCommands_002b8f70((FrameBuffer*)0x0);
	FUN_002b9740();
	return;
}
