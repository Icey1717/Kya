#include "VideoA.h"
#include "VideoB.h"
#include "EdenLib/edSys/sources/EdSystem.h"
#include "VideoC.h"

#ifdef PLATFORM_WIN
#include "port.h"
#endif
#include "Viewport.h"
#include "kya.h"

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

void ClearVideoMode(void)
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

void edVideoSetBackgroundColor(byte r, byte g, byte b)
{
	VideoManager.params18.r = r;
	VideoManager.params18.g = g;
	VideoManager.params18.b = b;

	if (VideoManager.pCamera != (ed_viewport*)0x0) {
		edViewportSetBackgroundColor(VideoManager.pCamera, r, g, b);
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
	VideoManager.params18.DX = inDX * (short)(0xa00 / VideoManager.screenWidth);
	VideoManager.params18.DY = inDY;
	_SetVideoEnv();

	return;
}

void SetVideoMode(short omode, short screenWidth, short screenHeight, short vidMode1, int param_5)
{
	edVideoConfig* pVVar1;
	ed_surface_desc surfaceDesc;
	ed_video_attr videoAttr;

	videoAttr.field_0xf = 1;
	videoAttr.isNTSC = (ushort)g_isNTSC;
	videoAttr.inter = SCE_GS_INTERLACE;
	videoAttr.field_0xb = 0;
	videoAttr.field_0xc = 0;
	videoAttr.bVSyncForever = 0;
	videoAttr.maxVblank_0xe = 1;
	videoAttr.field_0x10[0] = '\0';
	videoAttr.screenWidth = screenWidth;
	videoAttr.screenHeight = screenHeight;
	videoAttr.vidMode1 = vidMode1;
	videoAttr.omode = omode;
	edVideoSetAttribute(&videoAttr);

	if (psurf != (edSurface*)0x0) {
		edSurfaceDel(psurf);
	}

	surfaceDesc.pixelStoreMode = SCE_GS_PSMCT32;
	surfaceDesc.field_0x6 = 2;
	surfaceDesc.flags_0x8 = 0x24;
	surfaceDesc.bUseGlobalFrameBuffer = false;
	surfaceDesc.screenWidth = screenWidth;
	surfaceDesc.screenHeight = screenHeight;
	pVVar1 = edVideoGetConfig();
	if (pVVar1->field_0x1 == 0) {
		surfaceDesc.frameBufferCount = 1;
	}
	else {
		surfaceDesc.frameBufferCount = 2;
	}
	psurf = edSurfaceNew(&surfaceDesc);

	if (pzbuf != (edSurface*)0x0) {
		edSurfaceDel(pzbuf);
	}

	if (param_5 == 2) {
		surfaceDesc.pixelStoreMode = SCE_GS_PSMCT16;
	}
	else {
		surfaceDesc.pixelStoreMode = SCE_GS_PSMCT32;
	}

	surfaceDesc.field_0x6 = 2;
	surfaceDesc.flags_0x8 = 8;
	surfaceDesc.bUseGlobalFrameBuffer = false;
	surfaceDesc.frameBufferCount = 1;
	surfaceDesc.screenWidth = screenWidth;
	surfaceDesc.screenHeight = screenHeight;
	pzbuf = edSurfaceNew(&surfaceDesc);

	edVideoSetBackgroundColor(0, 0, 0);
	edVideoSetOffset(gVideoConfig.offsetX, gVideoConfig.offsetY);

	return;
}
