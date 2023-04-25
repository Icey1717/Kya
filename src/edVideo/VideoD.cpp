#include "VideoD.h"
#include "VideoB.h"
#include "edSystem.h"
#include "Camera.h"
#include "edDlist.h"

#ifdef PLATFORM_PS2
#include <libvu0.h>
#include <libdma.h>
#else
#include "port.h"
#endif
#include <assert.h>

#include "Rendering/CustomShell.h"

edVideo_Globals edVideo_Globals_00449590 = { 0 };

void FUN_002b8f30(void)
{
	if (g_ActiveVidParams_0048cd90.params26.omode == SCE_GS_PAL) {
		edVideo_Globals_00449590.field_0x4 = (undefined*)0x0059f740;
	}
	else {
		edVideo_Globals_00449590.field_0x4 = (undefined*)&GetHeap()[0x59b].pStartAddr;
	}
	edVideo_Globals_00449590.pActiveVidParams = &g_ActiveVidParams_0048cd90;
	return;
}

void SetVideoDefaults_002ba440(edVideoData* param_1)
{
	(param_1->params26).screenWidth = 0x200;
	(param_1->params26).screenHeight = 0x200;
	(param_1->params26).vidMode1 = 0;
	(param_1->params26).omode = SCE_GS_PAL;
	(param_1->params26).isNTSC = 1;
	(param_1->params26).inter = SCE_GS_INTERLACE;
	(param_1->params26).field_0xb = 1;
	(param_1->params26).field_0xc = 1;
	(param_1->params26).bVSyncForever = 1;
	(param_1->params26).maxVblank_0xe = 1;
	(param_1->params26).field_0xf = 1;
	(param_1->params26).field_0x10[0] = '\x01';
	return;
}

edSysHandlerVideo edSysHandlerVideo_0048cee0;

edSysHandlerVideo::edSysHandlerVideo()
{
	SetVideoDefaults_002ba440(&g_ActiveVidParams_0048cd90);
	nodeParent = &g_SysHandlersNodeTable_00489170;
	maxEventID = 0xb;
	mainIdentifier = 2;
	//SetupEd10_00217720((undefined*)&edSysHandlerVideo_0048cee0, Destructor, &EdFileGlobal_10_0048ced0);
	return;
}

void CallRenderFunc_002ba0f0(void)
{
	FrameBuffer* pVVar1;

	pVVar1 = (g_ActiveVidParams_0048cd90.pFrameBuffer)->pVidModeData_0x0->pLink_0xc;
	edSysHandlersCall(edSysHandlerVideo_0048cee0.mainIdentifier, edSysHandlerVideo_0048cee0.entries,
		edSysHandlerVideo_0048cee0.maxEventID, 0, pVVar1);
	if (g_ActiveVidParams_0048cd90.params26.field_0xf != 0) {
		(g_ActiveVidParams_0048cd90.pFrameBuffer)->pVidModeData_0x0->pLink_0xc = pVVar1->pNext;
	}
	return;
}

void SetGSRegistersFromVidModeData_002b8e20(FrameBuffer* param_1)
{
#ifdef PLATFORM_PS2
	DPUT_GS_PMODE((ulong)(ushort)g_ActiveVidParams_0048cd90.params18.gs_pmode);
	DPUT_GS_SMODE2((ulong)(ushort)g_ActiveVidParams_0048cd90.params18.gs_smode2);
	DPUT_GS_DISPFB2(*(u_long*)&param_1->pVidModeData20->dispfb2);
	DPUT_GS_DISPLAY2(*(u_long*)&g_ActiveVidParams_0048cd90.disp2);
	DPUT_GS_BGCOLOR(0);
	DPUT_GS_DISPLAY1(*(u_long*)&g_ActiveVidParams_0048cd90.disp1);
	DPUT_GS_DISPFB1(*(u_long*)&param_1->pVidModeData20->dispfb1);
#endif
	return;
}

void ConditonallyUpdateRegisters_002ba050(void)
{
	FrameBuffer* pVVar1;
	ulong uVar2;

	pVVar1 = (g_ActiveVidParams_0048cd90.pFrameBuffer)->pVidModeData_0x0->pLink_0xc;
	SetGSRegistersFromVidModeData_002b8e20(pVVar1->pNext);
	if ((g_ActiveVidParams_0048cd90.params18.ffmode == SCE_GS_FIELD) && (pVVar1->pNext == pVVar1)) {
#ifdef PLATFORM_PS2
		uVar2 = DGET_GS_CSR();
		pVVar1->pVidModeData20->csrValue_0x10 = (byte)((uVar2 & 0x2000) >> 0xd);
#endif
	}
	return;
}

void SendCameraCommands_002bb910(CameraObj_28* pCamera)
{
	if (pCamera->pColorBuffer != (FrameBuffer*)0x0) {
		BuildCameraCommands_002bb110(pCamera, pCamera->pCameraObj390_0x24->commandBuffer);
		WaitDMA();
		WaitForDraw_00258230();
#ifdef PLATFORM_PS2
		shellDmaStartB(SHELLDMA_CHANNEL_GIF, pCamera->pCameraObj390_0x24, pCamera->pCameraObj390_0x24->qwc);
#endif
	}
	return;
}

void SendGifPacket_002ba0c0(void)
{
	if (g_ActiveVidParams_0048cd90.pCamera != (CameraObj_28*)0x0) {
		SendCameraCommands_002bb910(g_ActiveVidParams_0048cd90.pCamera);
	}
	return;
}

void FUN_002b9550(void)
{
	if (g_ActiveVidParams_0048cd90.field_0x78 != 0) {
		assert(false);
		//FUN_002b96c0();
	}
	if (g_ActiveVidParams_0048cd90.field_0x68 != 0) {
		g_ActiveVidParams_0048cd90.field_0x6c =
			g_ActiveVidParams_0048cd90.field_0x6c + g_ActiveVidParams_0048cd90.field_0x70;
		if (0x80 < g_ActiveVidParams_0048cd90.field_0x6c) {
			g_ActiveVidParams_0048cd90.field_0x70 = -1;
		}
		if (g_ActiveVidParams_0048cd90.field_0x6c == 0) {
			g_ActiveVidParams_0048cd90.field_0x70 = 1;
		}
		if (((0 < g_ActiveVidParams_0048cd90.field_0x70) &&
			(g_ActiveVidParams_0048cd90.field_0x74 < g_ActiveVidParams_0048cd90.field_0x6c)) ||
			((g_ActiveVidParams_0048cd90.field_0x70 < 0 &&
				(g_ActiveVidParams_0048cd90.field_0x6c < g_ActiveVidParams_0048cd90.field_0x74)))) {
			g_ActiveVidParams_0048cd90.field_0x68 = 0;
			g_ActiveVidParams_0048cd90.field_0x6c = g_ActiveVidParams_0048cd90.field_0x74;
		}
	}
	if (g_ActiveVidParams_0048cd90.field_0x68 != 0 || g_ActiveVidParams_0048cd90.field_0x80 == 1) {
		g_ActiveVidParams_0048cd90.field_0x7c = g_ActiveVidParams_0048cd90.field_0x6c;
		g_ActiveVidParams_0048cd90.field_0x79 = g_ActiveVidParams_0048cd90.field_0x69;
		g_ActiveVidParams_0048cd90.field_0x7a = g_ActiveVidParams_0048cd90.field_0x6a;
		g_ActiveVidParams_0048cd90.field_0x7b = g_ActiveVidParams_0048cd90.field_0x6b;
	}
	g_ActiveVidParams_0048cd90.field_0x78 =
		g_ActiveVidParams_0048cd90.field_0x68 != 0 || g_ActiveVidParams_0048cd90.field_0x80 == 1;
	return;
}

uint UINT_00449584 = 0;

unsigned int GetCountValue(void)
{
#ifdef PLATFORM_PS2
	unsigned int i;

	__asm__ volatile (" mfc0 %0, $09" : "=r"(i));

	return i;
#else
	return 0;
#endif
}

void SetiGpffff9094ToCountValue(void)

{
	UINT_00449584 = GetCountValue();
	return;
}

void WaitForSomething2(void)
{
	undefined* puVar1;
	int currentCount;
	uint countDifference;
	uint waitTime;
	int iVar2;

	waitTime = (uint)g_ActiveVidParams_0048cd90.params26.maxVblank_0xe * (int)edVideo_Globals_00449590.field_0x4;
	iVar2 = UINT_00449584 - g_ActiveVidParams_0048cd90.lastCount;
	puVar1 = (undefined*)abs(iVar2);
	if (edVideo_Globals_00449590.field_0x4 < puVar1) {
		iVar2 = 0;
	}
	currentCount = GetCountValue();
	countDifference = currentCount - UINT_00449584;
	if (countDifference < waitTime) {
		waitTime = waitTime - iVar2;
	}
	while (countDifference < waitTime) {
		iVar2 = GetCountValue();
		countDifference = iVar2 - UINT_00449584;
	}
	return;
}

void WaitForVSync(byte param_1)
{
#ifdef PLATFORM_PS2
	ulong uVar1;

	g_ActiveVidParams_0048cd90.bWaitingForVSync = 1;
	if (g_ActiveVidParams_0048cd90.params26.bVSyncForever == 0) {
		g_ActiveVidParams_0048cd90.field_0x52 = param_1;
		WaitForSomething2();
		if ((((g_ActiveVidParams_0048cd90.field_0x52 == 0) ||
			(g_ActiveVidParams_0048cd90.params26.inter == SCE_GS_NOINTERLACE)) ||
			(g_ActiveVidParams_0048cd90.params18.ffmode == SCE_GS_FIELD)) ||
			(g_ActiveVidParams_0048cd90.pFrameBuffer == (FrameBuffer*)0x0)) {
			ResetVideoData_002ba560();
		}
		else {
			uVar1 = DGET_GS_CSR();
			if ((uint)((uVar1 & 0x2000) >> 0xd) ==
				(uint)(g_ActiveVidParams_0048cd90.pFrameBuffer)->pVidModeData_0x0->pLink_0xc->pVidModeData20->csrValue_0x10) {
				ResetVideoData_002ba560();
			}
		}
		SetiGpffff9094ToCountValue();
		return;
	}
	do {
	} while (g_ActiveVidParams_0048cd90.bWaitingForVSync != 0);
#else
	return;
#endif
}

void RefreshScreenRender(void)
{
	RENDER_LOG("RefreshScreenRender\n");

	/* Render scene */
	edSysHandlersCall(edSysHandlerVideo_0048cee0.mainIdentifier, edSysHandlerVideo_0048cee0.entries,
		edSysHandlerVideo_0048cee0.maxEventID, 6, (void*)0x0);
	WaitForDraw_00258230();
	FUN_002b9550();
	/* Render UI */
	edSysHandlersCall(edSysHandlerVideo_0048cee0.mainIdentifier, edSysHandlerVideo_0048cee0.entries,
		edSysHandlerVideo_0048cee0.maxEventID, 10, (void*)0x0);
	WaitForDraw_00258230();
	edSysHandlersCall(edSysHandlerVideo_0048cee0.mainIdentifier, edSysHandlerVideo_0048cee0.entries,
		edSysHandlerVideo_0048cee0.maxEventID, 9, (void*)0x0);
	WaitForVSync(1);
	if (g_ActiveVidParams_0048cd90.params26.field_0xf != 0) {
		ConditonallyUpdateRegisters_002ba050();
		SendGifPacket_002ba0c0();
	}
	CallRenderFunc_002ba0f0();
	edSysHandlersCall(edSysHandlerVideo_0048cee0.mainIdentifier, edSysHandlerVideo_0048cee0.entries,
		edSysHandlerVideo_0048cee0.maxEventID, 7, (void*)0x0);
	return;
}

FrameBuffer* GetVidModeData_002ba360(void)
{
	FrameBuffer* pVVar1;

	pVVar1 = (FrameBuffer*)0x0;
	if (g_ActiveVidParams_0048cd90.pFrameBuffer != (FrameBuffer*)0x0) {
		pVVar1 = g_ActiveVidParams_0048cd90.pFrameBuffer->pVidModeData_0x0->pLink_0xc;
	}
	if ((pVVar1 != (FrameBuffer*)0x0) && (pVVar1->pNext != (FrameBuffer*)0x0)) {
		pVVar1 = pVVar1->pNext;
	}
	return pVVar1;
}

edVideo_Globals* GetVideoParams_002ba3e0(void)
{
	return &edVideo_Globals_00449590;
}
