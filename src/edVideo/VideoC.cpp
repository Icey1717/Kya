#include "VideoC.h"
#include "VideoB.h"
#include "edMem.h"
#include <string.h>
#include "VideoD.h"

#ifdef PLATFORM_PS2
#include <libgraph.h>
#else
#include "port.h"
#include "renderer.h"
#endif
#include "Viewport.h"

// Not here

void _FadeUpdateScreenSize(edSurface* pFrameBuffer)
{
	int iVar1;
	int iVar2;

	iVar2 = (0x800 - ((int)(uint)pFrameBuffer->pSurfaceDesc->screenWidth >> 1)) * 0x10;
	iVar1 = (0x800 - ((int)(uint)pFrameBuffer->pSurfaceDesc->screenHeight >> 1)) * 0x10;
	*g_ActiveVidParams_0048cd90.listDataAC.field_0xa4 = (ulong)iVar2 & 0xffffffffU | ((ulong)iVar1 & 0xffffffffU) << 0x10;
	*g_ActiveVidParams_0048cd90.listDataAC.field_0xa8 =
		(ulong)(int)(iVar2 + (uint)pFrameBuffer->pSurfaceDesc->screenWidth * 0x10) & 0xffffffffU |
		((ulong)(int)(iVar1 + (uint)pFrameBuffer->pSurfaceDesc->screenHeight * 0x10) & 0xffffffffU) << 0x10;
	return;
}



void _SetSurfaceDispEnv(edSurface* pFrameBuffer, byte interlace)
{
	SurfaceDispEnv* pVVar1;

	pVVar1 = (SurfaceDispEnv*)edMemAlloc(TO_HEAP(H_MAIN), sizeof(SurfaceDispEnv));
	pFrameBuffer->pSurfaceDispEnv = pVVar1;
	ulong dispfb2 =
		(ulong)edVideoConfiguration.field_0x1 << 0x2b |
		(ulong)pFrameBuffer->pSurfaceDesc->pixelStoreMode << 0xf |
		(ulong)pFrameBuffer->frameBasePtr | (ulong)((int)(uint)pFrameBuffer->pSurfaceDesc->screenWidth >> 6) << 9;
	pFrameBuffer->pSurfaceDispEnv->dispfb2 = *(tGS_DISPFB2*)&dispfb2;
	ulong dispfb1 =
		(ulong)pFrameBuffer->pSurfaceDesc->pixelStoreMode << 0xf |
		(ulong)pFrameBuffer->frameBasePtr | (ulong)((int)(uint)pFrameBuffer->pSurfaceDesc->screenWidth >> 6) << 9;
	pFrameBuffer->pSurfaceDispEnv->dispfb1 = *(tGS_DISPFB1*)&dispfb1;
	pFrameBuffer->pSurfaceDispEnv->csrValue_0x10 = interlace;
	_FadeUpdateScreenSize(pFrameBuffer);
	return;
}

void _SetSurfaceZEnv(edSurface* pFrameBuffer)
{
	ZBufferTags* pTags;
	int zBufferFormat;

	pTags = (ZBufferTags*)edMemAlloc(TO_HEAP(H_MAIN), sizeof(ZBufferTags));
	pFrameBuffer->pZTags = pTags;

	// TAG
	pTags->commandBuffer[0].cmdA = SCE_GIF_SET_TAG(
		2,					// NLOOP
		SCE_GS_TRUE,		// EOP
		SCE_GS_FALSE,		// PRE
		0,					// PRIM
		SCE_GIF_PACKED,		// FLG
		1					// NREG
	);
	pTags->commandBuffer[0].cmdB = SCE_GIF_PACKED_AD;

	// Z BUFFER
	zBufferFormat = edSurfaceGetZBufferBpp(pFrameBuffer->pSurfaceDesc->pixelStoreMode);
	pTags->commandBuffer[1].cmdA = SCE_GS_SET_ZBUF(
		pFrameBuffer->frameBasePtr,	// ZBP
		zBufferFormat,				// PSM
		0							// ZMASK
	);
	pTags->commandBuffer[1].cmdB = SCE_GS_ZBUF_1;

	// Z TEST
	pTags->commandBuffer[2].cmdA = SCE_GS_SET_TEST(
		0,	// ATE 
		0,	// ATST
		0,	// AREF
		0,	// AFAIL
		0,	// DATE
		0,	// DATM
		1,	// ZTE
		3	// ZTST
	);
	pTags->commandBuffer[2].cmdB = SCE_GS_TEST_1;
	return;
}

int _HighResMode(void)
{
	int iVar1;

	iVar1 = 0;
	if (g_ActiveVidParams_0048cd90.params26.inter != SCE_GS_NOINTERLACE) {
		if ((g_ActiveVidParams_0048cd90.params26.omode == SCE_GS_NTSC) &&
			(0xe0 < g_ActiveVidParams_0048cd90.params26.screenHeight)) {
			iVar1 = 1;
		}
		else {
			if ((g_ActiveVidParams_0048cd90.params26.omode == SCE_GS_PAL) &&
				(0x100 < g_ActiveVidParams_0048cd90.params26.screenHeight)) {
				iVar1 = 1;
			}
		}
	}
	return iVar1;
}

void _SetVideoEnv(void)
{
	ulong uVar1;

	if (g_ActiveVidParams_0048cd90.params26.omode == SCE_GS_NTSC) {
		if (g_ActiveVidParams_0048cd90.params26.inter == SCE_GS_NOINTERLACE) {

			ulong dispfb2 = ((ulong)(ushort)g_ActiveVidParams_0048cd90.params18.DH << GS_DISPLAY2_DH_O |
					(ulong)(ushort)g_ActiveVidParams_0048cd90.params18.MAGH << GS_DISPLAY2_MAGH_O |
					(ulong)(g_ActiveVidParams_0048cd90.params18.DX + 0x27c << GS_DISPLAY2_DX_O) |
					(ulong)(g_ActiveVidParams_0048cd90.params18.DY + 0x19) << GS_DISPLAY2_DY_O |
					0x9ff00000000);
			g_ActiveVidParams_0048cd90.disp2 = *(tGS_DISPLAY2*)&dispfb2;
		}
		else {
			uVar1 = (ulong)(ushort)g_ActiveVidParams_0048cd90.params18.MAGH << GS_DISPLAY2_MAGH_O |
				(ulong)(g_ActiveVidParams_0048cd90.params18.DX + 0x27c) | (ulong)(g_ActiveVidParams_0048cd90.params18.DY + 0x32) << GS_DISPLAY2_DY_O |
				0x9ff00000000;
			ulong dispfb2 = (uVar1 | (ulong)(int)((uint)(ushort)g_ActiveVidParams_0048cd90.params18.DH - (uint)edVideoConfiguration.field_0x1) <<
				GS_DISPLAY2_DH_O);
			g_ActiveVidParams_0048cd90.disp2 = *(tGS_DISPLAY2*)&dispfb2;
			ulong dispfb1 = ((ulong)(ushort)g_ActiveVidParams_0048cd90.params18.DH << GS_DISPLAY2_DH_O | uVar1);
			g_ActiveVidParams_0048cd90.disp1 = *(tGS_DISPLAY1*)&dispfb1;
		}
	}
	else {
		if (g_ActiveVidParams_0048cd90.params26.inter == SCE_GS_NOINTERLACE) {
			ulong dispfb2 = ((ulong)(ushort)g_ActiveVidParams_0048cd90.params18.DH << GS_DISPLAY2_DH_O |
					(ulong)(ushort)g_ActiveVidParams_0048cd90.params18.MAGH << GS_DISPLAY2_MAGH_O |
					(ulong)(g_ActiveVidParams_0048cd90.params18.DX + 0x290) | (ulong)(g_ActiveVidParams_0048cd90.params18.DY + 0x24) << GS_DISPLAY2_DY_O |
					0x9ff00000000);
			g_ActiveVidParams_0048cd90.disp2 = *(tGS_DISPLAY2*)&dispfb2;
		}
		else {
			uVar1 = (ulong)(ushort)g_ActiveVidParams_0048cd90.params18.MAGH << GS_DISPLAY2_MAGH_O |
				(ulong)(g_ActiveVidParams_0048cd90.params18.DX + 0x290) | (ulong)(g_ActiveVidParams_0048cd90.params18.DY + 0x48) << GS_DISPLAY2_DY_O |
				0x9ff00000000;
			ulong dispfb2 = (uVar1 | (ulong)(int)((uint)(ushort)g_ActiveVidParams_0048cd90.params18.DH - (uint)edVideoConfiguration.field_0x1) <<
				GS_DISPLAY2_DH_O);
			g_ActiveVidParams_0048cd90.disp2 = *(tGS_DISPLAY2*)&dispfb2;
			ulong dispfb1 = ((ulong)(ushort)g_ActiveVidParams_0048cd90.params18.DH << GS_DISPLAY2_DH_O | uVar1);
			g_ActiveVidParams_0048cd90.disp1 = *(tGS_DISPLAY1*)&dispfb1;
		}
	}
	return;
}

void _VideoUpdateSystemViewport(edSurface* pColorBuffer)
{
	byte alpha;
	CameraObjParams params;

	params.posY = 0;
	params.posX = 0;
	params.screenHeight = 0x200;
	params.screenWidth = 0x200;
	if (pColorBuffer != (edSurface*)0x0) {
		params.screenWidth = pColorBuffer->pSurfaceDesc->screenWidth;
		params.screenHeight = pColorBuffer->pSurfaceDesc->screenHeight;
	}
	alpha = 4;
	if (g_ActiveVidParams_0048cd90.params26.field_0xc != 0) {
		alpha = 6;
	}
	if (g_ActiveVidParams_0048cd90.params26.field_0xb != 0) {
		alpha = alpha | 1;
	}
	if (g_ActiveVidParams_0048cd90.pCamera == (ed_viewport*)0x0) {
		edViewportNew(&params, pColorBuffer, (edSurface*)0x0, alpha);
		edViewportSetBackgroundColor(g_ActiveVidParams_0048cd90.pCamera, 0x20, 0x20, 0x40);
	}
	else {
		edViewportUpdate(g_ActiveVidParams_0048cd90.pCamera, &params, pColorBuffer, 0, alpha);
		edViewportSetBackgroundColor
		(g_ActiveVidParams_0048cd90.pCamera, g_ActiveVidParams_0048cd90.params18.r,
			g_ActiveVidParams_0048cd90.params18.g, g_ActiveVidParams_0048cd90.params18.b);
	}
	return;
}


// End Not here

int edSurfaceGetAlignments(int param_1, int psm, int* pPageWidth, int* pPageHeight)
{
	int iVar1;

	if (psm == (SCE_GS_PSMCT24 | SCE_GS_PSMCT16)) {
		*pPageWidth = 0x80;
		*pPageHeight = 0x40;
	}
	else {
		if (psm == SCE_GS_PSMCT16) {
			*pPageWidth = 0x40;
			*pPageHeight = 0x40;
		}
		else {
			if ((psm != SCE_GS_PSMCT24) && (psm != SCE_GS_PSMCT32)) {
				return 0;
			}
			*pPageWidth = 0x40;
			*pPageHeight = 0x20;
		}
	}
	if (param_1 == 2) {
		iVar1 = 0x100;
	}
	else {
		if ((param_1 == 8) || (param_1 == 4)) {
			iVar1 = 0x2000;
		}
		else {
			iVar1 = 0;
		}
	}
	return iVar1;
}

uint edSurfaceGetGreaterPower2Val(uint pixels)
{
	uint uVar1;
	uint uVar2;

	uVar2 = 0;
	uVar1 = pixels;
	if (pixels == 0) {
		pixels = 0;
	}
	else {
		for (; (uVar1 != 0 && (uVar1 != 1)); uVar1 = uVar1 >> 1) {
			uVar2 = uVar2 + 1;
		}
		uVar1 = 1 << (uVar2 & 0x1f);
		if (uVar1 != pixels) {
			pixels = uVar1 << 1;
		}
	}
	return pixels;
}

bool edSurfaceAlloc(edSurface* pFrameBuffer)
{
	ushort psm;
	ed_surface_desc* pVVar2;
	int align;
	void* pvVar3;
	int size;
	uint width;
	uint height;
	int iStack8;
	int iStack4;

	pVVar2 = pFrameBuffer->pSurfaceDesc;
	width = (uint)pVVar2->screenWidth;
	height = (uint)pVVar2->screenHeight;
	align = edSurfaceGetAlignments(pVVar2->flags_0x8 & 0xf, (uint)pVVar2->pixelStoreMode, &iStack4, &iStack8);

	if ((pFrameBuffer->pSurfaceDesc->flags_0x8 & 0x20) != 0) {
		width = edSurfaceGetGreaterPower2Val(width);
		height = edSurfaceGetGreaterPower2Val(height);
	}
	size = width * height * 2;
	psm = pFrameBuffer->pSurfaceDesc->pixelStoreMode;
	if ((psm == SCE_GS_PSMCT32) || (psm == SCE_GS_PSMCT24)) {
		size = width * height * 4;
	}

	MY_LOG("edSurfaceAlloc Allocating surface for %p width: %x height: %x align: %x psm: %x size: %x", (void*)pFrameBuffer, width, height, align, psm, size);

	pvVar3 = edMemAllocAlign(TO_HEAP(H_VIDEO), size, align);
	pFrameBuffer->data_0xc = (char*)pvVar3;
	pvVar3 = edMemGetLocalAddress((void*)pFrameBuffer->data_0xc);
	pFrameBuffer->frameBasePtr = (int)pvVar3 / align;
	return pFrameBuffer->data_0xc != 0;
}

edSurface* edSurfaceNew(ed_surface_desc* pVidModeData)
{
	bool bVar2;
	byte interlaceFormat;
	ed_surface_desc* pVidModeCopy;
	ed_surface_desc* puVar2;
	edSurface* bufferStart;
	uint uVar4;
	edSurface* lVar5;
	edSurface* pPrevFrameBuffer;
	edSurface* pNewFrameBuffer;

	lVar5 = (edSurface*)0x0;
	pVidModeCopy = (ed_surface_desc*)edMemAlloc(TO_HEAP(H_MAIN), sizeof(ed_surface_desc));
	*pVidModeCopy = *pVidModeData;

	uVar4 = 0;
	pPrevFrameBuffer = (edSurface*)0x0;
	if (pVidModeData->frameBufferCount != 0) {
		do {
			pNewFrameBuffer = (edSurface*)edMemAlloc(TO_HEAP(H_MAIN), sizeof(edSurface));
			memset(pNewFrameBuffer, 0, sizeof(edSurface));
			pNewFrameBuffer->pSurfaceDesc = pVidModeCopy;
			if (lVar5 == (edSurface*)0x0) {
				lVar5 = pNewFrameBuffer;
			}
			if ((pVidModeData->bUseGlobalFrameBuffer == '\0') && (bVar2 = edSurfaceAlloc(pNewFrameBuffer), bVar2 == false)) {
				edMemFree(pNewFrameBuffer);
				return (edSurface*)0x0;
			}
			if ((pVidModeData->flags_0x8 & 0x20) != 0) {
				interlaceFormat = 0;
				if (g_ActiveVidParams_0048cd90.params18.ffmode != SCE_GS_FIELD) {
					interlaceFormat = (byte)uVar4 & 1;
				}
				_SetSurfaceDispEnv(pNewFrameBuffer, interlaceFormat);
			}
			if ((pVidModeData->flags_0x8 & 8) != 0) {
				_SetSurfaceZEnv(pNewFrameBuffer);
			}
			if (pPrevFrameBuffer != (edSurface*)0x0) {
				pNewFrameBuffer->pNext = pPrevFrameBuffer;
			}
			uVar4 = uVar4 + 1;
			pPrevFrameBuffer = pNewFrameBuffer;
		} while (uVar4 < pVidModeData->frameBufferCount);
	}
	lVar5->pNext = pNewFrameBuffer;
	if ((pVidModeData->flags_0x8 & 0x20) != 0) {
		_VideoManagerAddFlippingSurface(pNewFrameBuffer);
	}
	return pNewFrameBuffer;
}

void edSurfaceDel(edSurface* pFrameBuffer)
{
	byte bVar1;
	ed_surface_desc* heapID;
	edSurface* pVVar2;
	uint uVar3;

	heapID = pFrameBuffer->pSurfaceDesc;
	bVar1 = heapID->frameBufferCount;
	if (heapID != (ed_surface_desc*)0x0) {
		edMemFree(heapID);
	}
	uVar3 = 0;
	if (bVar1 != 0) {
		do {
			if (g_ActiveVidParams_0048cd90.pFrameBuffer == pFrameBuffer) {
				g_ActiveVidParams_0048cd90.pFrameBuffer = (edSurface*)0x0;
			}
			if (pFrameBuffer->pSurfaceDispEnv != (SurfaceDispEnv*)0x0) {
				edMemFree(pFrameBuffer->pSurfaceDispEnv);
			}
			if (pFrameBuffer->pZTags != (ZBufferTags*)0x0) {
				edMemFree(pFrameBuffer->pZTags);
			}
			if ((void*)pFrameBuffer->data_0xc != (void*)0x0) {
				edMemFree((void*)pFrameBuffer->data_0xc);
			}
			pVVar2 = pFrameBuffer->pNext;
			edMemFree(pFrameBuffer);
			uVar3 = uVar3 + 1;
			pFrameBuffer = pVVar2;
		} while (uVar3 < bVar1);
	}
	return;
}

void edSurfaceUpdateEnv(edSurface* pSurface, ushort pixelStoreMode)
{
	uint uVar1;
	uint uVar2;

	uVar2 = (uint)pSurface->pSurfaceDesc->frameBufferCount;
	uVar1 = 0;
	if (uVar2 != 0) {
		do {
			pSurface->pSurfaceDesc->pixelStoreMode = pixelStoreMode;
			if (pSurface->pSurfaceDispEnv != (SurfaceDispEnv*)0x0) {
				ulong dispfb2 = (ulong)edVideoConfiguration.field_0x1 << 0x2b |
					(ulong)pSurface->pSurfaceDesc->pixelStoreMode << 0xf |
					(ulong)pSurface->frameBasePtr | (long)((int)(uint)pSurface->pSurfaceDesc->screenWidth >> 6) << 9;

				pSurface->pSurfaceDispEnv->dispfb2 = *(tGS_DISPFB2*)&dispfb2;
				ulong dispfb1 = (ulong)pSurface->pSurfaceDesc->pixelStoreMode << 0xf |
					(ulong)pSurface->frameBasePtr | (long)((int)(uint)pSurface->pSurfaceDesc->screenWidth >> 6) << 9;
				pSurface->pSurfaceDispEnv->dispfb1 = *(tGS_DISPFB1*)&dispfb1;
			}
			uVar1 = uVar1 + 1;
			pSurface = pSurface->pNext;
		} while (uVar1 < uVar2);
	}
	return;
}

char* g_szInvalidZBuffer_00433480 = "You don't have a valid zbuffer !!!!";

int edSurfaceGetZBufferBpp(int pixelStoreMode)
{
	edSurface* pVVar1;
	int outFormat;

	if (pixelStoreMode == SCE_GS_PSMCT16) {
		pVVar1 = edVideoGetDisplaySurface();
		outFormat = 0x32;
		if (pVVar1->pSurfaceDesc->pixelStoreMode != SCE_GS_PSMCT16) {
			outFormat = SCE_GS_PSMZ16S;
		}
	}
	else {
		if (pixelStoreMode == SCE_GS_PSMCT24) {
			outFormat = SCE_GS_PSMZ24;
		}
		else {
			if (pixelStoreMode == SCE_GS_PSMCT32) {
				pVVar1 = edVideoGetDisplaySurface();
				if (pVVar1->pSurfaceDesc->pixelStoreMode == SCE_GS_PSMCT16) {
					edSurfaceUpdateEnv(pVVar1, SCE_GS_PSMCT16S);
				}
				outFormat = SCE_GS_PSMZ32;
			}
			else {
				outFormat = SCE_GS_PSMZ32;
				edDebugPrintf(g_szInvalidZBuffer_00433480);
			}
		}
	}
	return outFormat;
}
