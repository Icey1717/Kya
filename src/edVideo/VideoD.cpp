#include "VideoD.h"
#include "VideoB.h"
#include "EdenLib/edSys/sources/EdSystem.h"
#include "Viewport.h"
#include "edDlist.h"

#ifdef PLATFORM_PS2
#include <libvu0.h>
#include <libdma.h>
#else
#include "port.h"
#include "port/vu1_emu.h"
#include "profiling.h"
#endif
#include <assert.h>

#include "Rendering/CustomShell.h"

#ifdef PLATFORM_WIN
#include "renderer.h"

#include <chrono>
#include <thread>
#endif

#include "CameraViewManager.h"

#ifdef PLATFORM_WIN
Multidelegate<> gOnVideoFlip;

Multidelegate<>& GetOnVideoFlip()
{
	return gOnVideoFlip;
}
#endif // PLATFORM_WIN


edVideo_Globals edVideo_Globals_00449590 = { 0 };

void _UpdateVideoInfo(void)
{
	if (VideoManager.omode == SCE_GS_PAL) {
		edVideo_Globals_00449590.field_0x4 = (undefined*)0x0059f740;
	}
	else {
		edVideo_Globals_00449590.field_0x4 = (undefined*)&edmemGetMainHeader()[0x59b].pStartAddr;
	}
	edVideo_Globals_00449590.pActiveVidParams = &VideoManager;
	return;
}

ed_video_attr::ed_video_attr()
{
	this->screenWidth = 0x200;
	this->screenHeight = 0x200;
	this->vidMode1 = 0;
	this->omode = SCE_GS_PAL;
	this->isNTSC = 1;
	this->inter = SCE_GS_INTERLACE;
	this->field_0xb = 1;
	this->field_0xc = 1;
	this->bVSyncForever = 1;
	this->maxVblank_0xe = 1;
	this->field_0xf = 1;
	this->field_0x10[0] = '\x01';
	return;
}

edSysHandlerVideo edVideoHandlers;

void edVideoSwap(void)
{
	edSurface* pVVar1;

	pVVar1 = (VideoManager.pFrameBuffer)->pSurfaceDesc->pLink_0xc;
	edSysHandlersCall(edVideoHandlers.mainIdentifier, edVideoHandlers.entries,
		edVideoHandlers.maxEventID, 0, pVVar1);
	if (VideoManager.field_0xf != 0) {
		(VideoManager.pFrameBuffer)->pSurfaceDesc->pLink_0xc = pVVar1->pNext;
	}
	return;
}

void SetGSRegistersFromVidModeData_002b8e20(edSurface* param_1)
{
#ifdef PLATFORM_PS2
	DPUT_GS_PMODE((ulong)(ushort)VideoManager.params18.gs_pmode);
	DPUT_GS_SMODE2((ulong)(ushort)VideoManager.params18.gs_smode2);
	DPUT_GS_DISPFB2(*(u_long*)&param_1->pSurfaceDispEnv->dispfb2);
	DPUT_GS_DISPLAY2(*(u_long*)&VideoManager.disp2);
	DPUT_GS_BGCOLOR(0);
	DPUT_GS_DISPLAY1(*(u_long*)&VideoManager.disp1);
	DPUT_GS_DISPFB1(*(u_long*)&param_1->pSurfaceDispEnv->dispfb1);
#endif
	return;
}

void edVideoPutDisplayEnv(void)
{
	edSurface* pVVar1;
	ulong uVar2;

	pVVar1 = (VideoManager.pFrameBuffer)->pSurfaceDesc->pLink_0xc;
	SetGSRegistersFromVidModeData_002b8e20(pVVar1->pNext);
	if ((VideoManager.params18.ffmode == SCE_GS_FIELD) && (pVVar1->pNext == pVVar1)) {
#ifdef PLATFORM_PS2
		uVar2 = DGET_GS_CSR();
		pVVar1->pSurfaceDispEnv->csrValue_0x10 = (byte)((uVar2 & 0x2000) >> 0xd);
#endif
	}
	return;
}

void edVideoPutDrawEnv(void)
{
	if (VideoManager.pCamera != (ed_viewport*)0x0) {
		edViewPortApplyDrawingEnv(VideoManager.pCamera);
	}
	return;
}

void _DrawFade(void)
{
#ifdef PLATFORM_WIN
	Renderer::Native::DrawFade(VideoManager.activeFadeColorR, VideoManager.activeFadeColorG,
		VideoManager.activeFadeColorB, VideoManager.activeFadeAlpha);
#else
	IMPLEMENTATION_GUARD();
#endif
}

void _ManageFade(void)
{
	if (VideoManager.bShouldDrawFade != 0) {
		_DrawFade();
	}

	if (VideoManager.bFadeActive != 0) {
		VideoManager.fadeAlpha = VideoManager.fadeAlpha + VideoManager.fadeSpeed;
		if (0x80 < VideoManager.fadeAlpha) {
			VideoManager.fadeSpeed = -1;
		}

		if (VideoManager.fadeAlpha == 0) {
			VideoManager.fadeSpeed = 1;
		}

		if (((0 < VideoManager.fadeSpeed) && (VideoManager.fadeAlphaTarget < VideoManager.fadeAlpha)) ||
			((VideoManager.fadeSpeed < 0 && (VideoManager.fadeAlpha < VideoManager.fadeAlphaTarget)))) {
			VideoManager.bFadeActive = 0;
			VideoManager.fadeAlpha = VideoManager.fadeAlphaTarget;
		}
	}

	if (VideoManager.bFadeActive != 0 || VideoManager.fadeFlags == 1) {
		VideoManager.activeFadeAlpha = VideoManager.fadeAlpha;
		VideoManager.activeFadeColorR = VideoManager.fadeColorR;
		VideoManager.activeFadeColorG = VideoManager.fadeColorG;
		VideoManager.activeFadeColorB = VideoManager.fadeColorB;
	}

	VideoManager.bShouldDrawFade = VideoManager.bFadeActive != 0 || VideoManager.fadeFlags == 1;
	return;
}

uint UINT_00449584 = 0;

unsigned int _VideoTimerGet(void)
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
	UINT_00449584 = _VideoTimerGet();
	return;
}

void _VideoTimerSyncVbl(void)
{
	undefined* puVar1;
	int currentCount;
	uint countDifference;
	uint waitTime;
	int iVar2;

	waitTime = (uint)VideoManager.maxVblank_0xe * (int)edVideo_Globals_00449590.field_0x4;
	iVar2 = UINT_00449584 - VideoManager.lastCount;
	puVar1 = (undefined*)abs(iVar2);
	if (edVideo_Globals_00449590.field_0x4 < puVar1) {
		iVar2 = 0;
	}
	currentCount = _VideoTimerGet();
	countDifference = currentCount - UINT_00449584;
	if (countDifference < waitTime) {
		waitTime = waitTime - iVar2;
	}
	while (countDifference < waitTime) {
		iVar2 = _VideoTimerGet();
		countDifference = iVar2 - UINT_00449584;
	}
	return;
}

void edVideoWaitVsync(byte param_1)
{
#ifdef PLATFORM_PS2
	ulong uVar1;

	VideoManager.bWaitingForVSync = 1;
	if (VideoManager.bVSyncForever == 0) {
		VideoManager.field_0x52 = param_1;
		_VideoTimerSyncVbl();
		if ((((VideoManager.field_0x52 == 0) ||
			(VideoManager.inter == SCE_GS_NOINTERLACE)) ||
			(VideoManager.params18.ffmode == SCE_GS_FIELD)) ||
			(VideoManager.pFrameBuffer == (edSurface*)0x0)) {
			_edVideoSyncReset();
		}
		else {
			uVar1 = DGET_GS_CSR();
			if ((uint)((uVar1 & 0x2000) >> 0xd) ==
				(uint)(VideoManager.pFrameBuffer)->pSurfaceDesc->pLink_0xc->pSurfaceDispEnv->csrValue_0x10) {
				_edVideoSyncReset();
			}
		}
		_VideoTimerReset();
		return;
	}
	do {
	} while (VideoManager.bWaitingForVSync != 0);
#else
	static const double targetFrameTime = 1.0 / 60.0;  // Target frame time for 60 fps (in seconds)

	// Calculate frame time
	static auto frameStart = std::chrono::steady_clock::now();

	while (true)
	{
		auto frameEnd = std::chrono::steady_clock::now();
		auto frameTime = std::chrono::duration_cast<std::chrono::duration<double>>(frameEnd - frameStart).count();

		if (frameTime >= targetFrameTime)
		{
			frameStart = frameEnd;
			break;
		}
	}

	frameStart = std::chrono::steady_clock::now();
	return;
#endif
}

void edVideoFlip(void)
{
	MY_LOG_CATEGORY("Video", LogLevel::VeryVerbose, "edVideoFlip\n");
	ZONE_SCOPED;

#ifdef PLATFORM_WIN
	{
		ZONE_SCOPED_NAME("Win Begin");
		VU1Emu::BeginFrame();
		Renderer::Present();
		FRAME_MARK;
		Renderer::WaitUntilReady();
	}
#endif
	/* Render scene */
	edSysHandlersCall(edVideoHandlers.mainIdentifier, edVideoHandlers.entries, edVideoHandlers.maxEventID, ED_VIDEO_HANDLER_RENDER, (void*)0x0);
#ifdef PLATFORM_WIN
	VU1Emu::Wait();
#endif
	edDmaSyncPath();
	_ManageFade();
	/* Render UI */
	edSysHandlersCall(edVideoHandlers.mainIdentifier, edVideoHandlers.entries, edVideoHandlers.maxEventID, ED_HANDLER_VIDEO_SEND_2D, (void*)0x0);
	edDmaSyncPath();
	edSysHandlersCall(edVideoHandlers.mainIdentifier, edVideoHandlers.entries, edVideoHandlers.maxEventID, 9, (void*)0x0);

#ifdef PLATFORM_WIN
	Renderer::Native::OnVideoFlip();
#endif

	edVideoWaitVsync(1);

	if (VideoManager.field_0xf != 0) {
		edVideoPutDisplayEnv();
		edVideoPutDrawEnv();
	}

	edVideoSwap();
	edSysHandlersCall(edVideoHandlers.mainIdentifier, edVideoHandlers.entries, edVideoHandlers.maxEventID, ED_VIDEO_HANDLER_COMPUTE, (void*)0x0);

#ifdef PLATFORM_WIN
	VU1Emu::EndFrame();
	gOnVideoFlip();
#endif // PLATFORM_WIN

	return;
}

edSurface* edVideoGetDisplaySurface(void)
{
	edSurface* pVVar1;

	pVVar1 = (edSurface*)0x0;
	if (VideoManager.pFrameBuffer != (edSurface*)0x0) {
		pVVar1 = VideoManager.pFrameBuffer->pSurfaceDesc->pLink_0xc;
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
