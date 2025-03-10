#include "VideoA.h"
#include "VideoB.h"
#include "EdenLib/edSys/sources/EdSystem.h"
#include "VideoC.h"

#ifdef PLATFORM_WIN
#include "port.h"
#endif
#include "Viewport.h"
#include "../kya.h"

edSurface* psurf = NULL;
edSurface* pzbuf = NULL;

edSurface* VideoGetDisplaybuffer(void)
{
	return psurf;
}

edSurface* VideoGetZbuffer(void)
{
	return pzbuf;
}

void CleanupFunc_001ba9e0(void)
{
	if (psurf != (edSurface*)0x0) {
		edSurfaceDel(psurf);
		psurf = (edSurface*)0x0;
	}
	if (pzbuf != (edSurface*)0x0) {
		edSurfaceDel(pzbuf);
		pzbuf = (edSurface*)0x0;
	}
	return;
}

void edVideoSetBackgroundColor(byte param_1, byte param_2, byte param_3)
{
	VideoManager.params18.r = param_1;
	VideoManager.params18.g = param_2;
	VideoManager.params18.b = param_3;
	if (VideoManager.pCamera != (ed_viewport*)0x0) {
		edViewportSetBackgroundColor(VideoManager.pCamera, param_1, param_2, param_3);
	}
	return;
}

void edVideoSetFadeColor(byte r, byte g, byte b)
{
	VideoManager.fadeColorR = r;
	VideoManager.fadeColorG = g;
	VideoManager.fadeColorB = b;

	return;
}

void edVideoSetFade(float alpha)
{
	float adjustedAlpha;

	if (alpha < 0.0f) {
		adjustedAlpha = 0.0f;
	}
	else {
		adjustedAlpha = 1.0f;
		if (alpha <= 1.0f) {
			adjustedAlpha = alpha;
		}
	}

	VideoManager.fadeFlags = (uint)(adjustedAlpha != 0.0f);
	VideoManager.bFadeActive = 0;
	VideoManager.fadeAlpha = (int)(adjustedAlpha * 128.0f);
	VideoManager.fadeSpeed = 0;
	VideoManager.fadeAlphaTarget = (int)(adjustedAlpha * 128.0f);

	return;
}

void edVideoSetFadeIn(uint speed)
{
	if (0x80 < speed) {
		speed = 0x80;
	}

	VideoManager.bFadeActive = 1;
	VideoManager.fadeSpeed = -(0x80 / (int)speed);
	VideoManager.fadeAlphaTarget = 0;
	VideoManager.fadeFlags = 0;

	return;
}

void edVideoSetFadeOut(uint speed, uint flags)
{
	if (0x80 < speed) {
		speed = 0x80;
	}

	VideoManager.bFadeActive = 1;
	VideoManager.fadeSpeed = 0x80 / (int)speed;
	VideoManager.fadeAlphaTarget = 0x80;
	VideoManager.fadeFlags = flags;

	return;
}

byte edVideoIsFadeActive(void)
{
	return VideoManager.bFadeActive;
}

void edVideoSetOffset(short inDX, short inDY)
{
	VideoManager.params18.DX =
		inDX * (short)(0xa00 / VideoManager.screenWidth);
	VideoManager.params18.DY = inDY;
	_SetVideoEnv();
	return;
}

void SetVideoMode(short omode, short screenWidth, short screenHeight, short vidMode1, int param_5)
{
	edVideoConfig* pVVar1;
	ed_surface_desc local_30;
	ed_video_attr local_20;

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
	edVideoSetAttribute(&local_20);
	if (psurf != (edSurface*)0x0) {
		edSurfaceDel(psurf);
	}
	local_30.pixelStoreMode = SCE_GS_PSMCT32;
	local_30.field_0x6 = 2;
	local_30.flags_0x8 = 0x24;
	local_30.bUseGlobalFrameBuffer = false;
	local_30.screenWidth = screenWidth;
	local_30.screenHeight = screenHeight;
	pVVar1 = edVideoGetConfig();
	if (pVVar1->field_0x1 == 0) {
		local_30.frameBufferCount = 1;
	}
	else {
		local_30.frameBufferCount = 2;
	}
	psurf = edSurfaceNew(&local_30);
	if (pzbuf != (edSurface*)0x0) {
		edSurfaceDel(pzbuf);
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
	pzbuf = edSurfaceNew(&local_30);
	edVideoSetBackgroundColor(0, 0, 0);
	edVideoSetOffset((short)gVideoConfig.offsetX, (short)gVideoConfig.offsetY);
	return;
}
