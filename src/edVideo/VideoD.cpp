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

#ifdef PLATFORM_WIN
#include "renderer.h"

#include <chrono>
#include <thread>
#endif
#include "../CameraViewManager.h"

edVideo_Globals edVideo_Globals_00449590 = { 0 };

void _UpdateVideoInfo(void)
{
	if (g_ActiveVidParams_0048cd90.params26.omode == SCE_GS_PAL) {
		edVideo_Globals_00449590.field_0x4 = (undefined*)0x0059f740;
	}
	else {
		edVideo_Globals_00449590.field_0x4 = (undefined*)&edmemGetMainHeader()[0x59b].pStartAddr;
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
	//__register_global_object((undefined*)&edSysHandlerVideo_0048cee0, Destructor, &EdFileGlobal_10_0048ced0);
	return;
}

void edVideoSwap(void)
{
	edSurface* pVVar1;

	pVVar1 = (g_ActiveVidParams_0048cd90.pFrameBuffer)->pSurfaceDesc->pLink_0xc;
	edSysHandlersCall(edSysHandlerVideo_0048cee0.mainIdentifier, edSysHandlerVideo_0048cee0.entries,
		edSysHandlerVideo_0048cee0.maxEventID, 0, pVVar1);
	if (g_ActiveVidParams_0048cd90.params26.field_0xf != 0) {
		(g_ActiveVidParams_0048cd90.pFrameBuffer)->pSurfaceDesc->pLink_0xc = pVVar1->pNext;
	}
	return;
}

void SetGSRegistersFromVidModeData_002b8e20(edSurface* param_1)
{
#ifdef PLATFORM_PS2
	DPUT_GS_PMODE((ulong)(ushort)g_ActiveVidParams_0048cd90.params18.gs_pmode);
	DPUT_GS_SMODE2((ulong)(ushort)g_ActiveVidParams_0048cd90.params18.gs_smode2);
	DPUT_GS_DISPFB2(*(u_long*)&param_1->pSurfaceDispEnv->dispfb2);
	DPUT_GS_DISPLAY2(*(u_long*)&g_ActiveVidParams_0048cd90.disp2);
	DPUT_GS_BGCOLOR(0);
	DPUT_GS_DISPLAY1(*(u_long*)&g_ActiveVidParams_0048cd90.disp1);
	DPUT_GS_DISPFB1(*(u_long*)&param_1->pSurfaceDispEnv->dispfb1);
#endif
	return;
}

void edVideoPutDisplayEnv(void)
{
	edSurface* pVVar1;
	ulong uVar2;

	pVVar1 = (g_ActiveVidParams_0048cd90.pFrameBuffer)->pSurfaceDesc->pLink_0xc;
	SetGSRegistersFromVidModeData_002b8e20(pVVar1->pNext);
	if ((g_ActiveVidParams_0048cd90.params18.ffmode == SCE_GS_FIELD) && (pVVar1->pNext == pVVar1)) {
#ifdef PLATFORM_PS2
		uVar2 = DGET_GS_CSR();
		pVVar1->pSurfaceDispEnv->csrValue_0x10 = (byte)((uVar2 & 0x2000) >> 0xd);
#endif
	}
	return;
}

void edVideoPutDrawEnv(void)
{
	if (g_ActiveVidParams_0048cd90.pCamera != (ed_viewport*)0x0) {
		edViewPortApplyDrawingEnv(g_ActiveVidParams_0048cd90.pCamera);
	}
	return;
}

void _ManageFade(void)
{
	if (g_ActiveVidParams_0048cd90.field_0x78 != 0) {
		IMPLEMENTATION_GUARD();
		//_DrawFade();
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

void _VideoTimerReset(void)

{
	UINT_00449584 = GetCountValue();
	return;
}

void _VideoTimerSyncVbl(void)
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

void edVideoWaitVsync(byte param_1)
{
#ifdef PLATFORM_PS2
	ulong uVar1;

	g_ActiveVidParams_0048cd90.bWaitingForVSync = 1;
	if (g_ActiveVidParams_0048cd90.params26.bVSyncForever == 0) {
		g_ActiveVidParams_0048cd90.field_0x52 = param_1;
		_VideoTimerSyncVbl();
		if ((((g_ActiveVidParams_0048cd90.field_0x52 == 0) ||
			(g_ActiveVidParams_0048cd90.params26.inter == SCE_GS_NOINTERLACE)) ||
			(g_ActiveVidParams_0048cd90.params18.ffmode == SCE_GS_FIELD)) ||
			(g_ActiveVidParams_0048cd90.pFrameBuffer == (edSurface*)0x0)) {
			_edVideoSyncReset();
		}
		else {
			uVar1 = DGET_GS_CSR();
			if ((uint)((uVar1 & 0x2000) >> 0xd) ==
				(uint)(g_ActiveVidParams_0048cd90.pFrameBuffer)->pSurfaceDesc->pLink_0xc->pSurfaceDispEnv->csrValue_0x10) {
				_edVideoSyncReset();
			}
		}
		_VideoTimerReset();
		return;
	}
	do {
	} while (g_ActiveVidParams_0048cd90.bWaitingForVSync != 0);
#else
	static const double targetFrameTime = 1.0 / 60.0;  // Target frame time for 60 fps (in seconds)

	// Calculate frame time
	static auto frameStart = std::chrono::steady_clock::now();

	// Update game logic and render the frame

	// Calculate elapsed frame time
	auto frameEnd = std::chrono::steady_clock::now();
	auto elapsedSeconds = std::chrono::duration<double>(frameEnd - frameStart).count();

	// Sleep to maintain the desired frame rate
	if (elapsedSeconds < targetFrameTime)
	{
		std::this_thread::sleep_for(std::chrono::duration<double>(targetFrameTime - elapsedSeconds));
	}

	frameStart = std::chrono::steady_clock::now();
	return;
#endif
}

void edVideoFlip(void)
{
	RENDER_LOG("RefreshScreenRender\n");
	// #Hack
	edF32MATRIX4 matrix = {
	0.534046, 0.453021, -0.713844, 0,
	-0.0929226, 0.870664, 0.483025, 0,
	0.840335, -0.191626, 0.50707, 0,
	1199.81, -949.292, 1278.1, 1,
	};
	//CameraManager::_gThis->pShadowSunView_0x4b8->transformationMatrix = matrix;
#ifdef PLATFORM_WIN
	Renderer::Present();
#endif

#ifdef PLATFORM_WIN
	Renderer::WaitUntilReady();
#endif
	/* Render scene */
	edSysHandlersCall(edSysHandlerVideo_0048cee0.mainIdentifier, edSysHandlerVideo_0048cee0.entries,
		edSysHandlerVideo_0048cee0.maxEventID, 6, (void*)0x0);
	edDmaSyncPath();
	_ManageFade();
	/* Render UI */
	edSysHandlersCall(edSysHandlerVideo_0048cee0.mainIdentifier, edSysHandlerVideo_0048cee0.entries,
		edSysHandlerVideo_0048cee0.maxEventID, 10, (void*)0x0);
	edDmaSyncPath();
	edSysHandlersCall(edSysHandlerVideo_0048cee0.mainIdentifier, edSysHandlerVideo_0048cee0.entries,
		edSysHandlerVideo_0048cee0.maxEventID, 9, (void*)0x0);

	edVideoWaitVsync(1);

	if (g_ActiveVidParams_0048cd90.params26.field_0xf != 0) {
		edVideoPutDisplayEnv();
		edVideoPutDrawEnv();
	}
	edVideoSwap();
	edSysHandlersCall(edSysHandlerVideo_0048cee0.mainIdentifier, edSysHandlerVideo_0048cee0.entries,
		edSysHandlerVideo_0048cee0.maxEventID, 7, (void*)0x0);
	return;
}

edSurface* edVideoGetDisplaySurface(void)
{
	edSurface* pVVar1;

	pVVar1 = (edSurface*)0x0;
	if (g_ActiveVidParams_0048cd90.pFrameBuffer != (edSurface*)0x0) {
		pVVar1 = g_ActiveVidParams_0048cd90.pFrameBuffer->pSurfaceDesc->pLink_0xc;
	}
	if ((pVVar1 != (edSurface*)0x0) && (pVVar1->pNext != (edSurface*)0x0)) {
		pVVar1 = pVVar1->pNext;
	}
	return pVVar1;
}

edVideo_Globals* edVideoGetInfo(void)
{
	return &edVideo_Globals_00449590;
}
