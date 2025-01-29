#include "Viewport.h"
#include "edVideo/VideoB.h"
#include "edMem.h"
#include "edVideo/VideoC.h"
#include <string.h>
#include <assert.h>

#ifdef PLATFORM_WIN
#include "port.h"
#include "renderer.h"
#include <vector>
#endif
#include "Rendering/CustomShell.h"

#define CAMERA_LOG(level, format, ...) MY_LOG_CATEGORY("Camera", level, format, ##__VA_ARGS__)

ed_viewport* edViewportNew(CameraObjParams* pParams, edSurface* pVidModeDataA, edSurface* pVidModeDataB, byte alpha)
{
	ed_viewport* __s;
	CameraObj_390* pCVar1;
	edpkt_data* pRVar2;
	ed_viewport* pReturnValue;
	CameraObjParams local_8;

	if (pParams == (CameraObjParams*)0x0) {
		local_8.posX = 0;
		local_8.posY = 0;
		if (pVidModeDataA == (edSurface*)0x0) {
			local_8.screenWidth = 0x200;
			local_8.screenHeight = 0x200;
		}
		else {
			local_8.screenWidth = pVidModeDataA->pSurfaceDesc->screenWidth;
			local_8.screenHeight = pVidModeDataA->pSurfaceDesc->screenHeight;
		}
		pParams = &local_8;
	}
	__s = (ed_viewport*)edMemAlloc(TO_HEAP(H_MAIN), sizeof(ed_viewport));
	memset(__s, 0, sizeof(ed_viewport));
	__s->clearColor.a = alpha;
	__s->fbMask = 0;
	if ((alpha & 1) == 0) {
		__s->clearMask = 0xffffffff;
	}
	else {
		__s->clearMask = 0;
	}
	__s->posX = pParams->posX;
	__s->posY = pParams->posY;
	__s->screenWidth = pParams->screenWidth;
	__s->screenHeight = pParams->screenHeight;
	__s->pColorBuffer = pVidModeDataA;
	__s->pZBuffer = pVidModeDataB;
	__s->pCameraObj390_0x24 = (CameraObj_390*)0x0;
	__s->clearColor.b = 0x40;
	__s->clearColor.g = 0x40;
	__s->clearColor.r = 0x40;
	if ((alpha & 4) != 0) {
		VideoManager.pCamera = __s;
		pCVar1 = (CameraObj_390*)edMemAlloc(TO_HEAP(H_MAIN), sizeof(CameraObj_390));
		__s->pCameraObj390_0x24 = pCVar1;
		if (__s->pColorBuffer == (edSurface*)0x0) {
			__s->pCameraObj390_0x24->qwc = 0;
		}
		else {
			pRVar2 = edViewportUpdateEnv(__s, __s->pCameraObj390_0x24->commandBuffer);
			__s->pCameraObj390_0x24->qwc = (int)((ulong)pRVar2 - (ulong)__s->pCameraObj390_0x24);
		}
	}
	return __s;
}

void edViewportUpdate(ed_viewport* pCamera, CameraObjParams* pParams, edSurface* pColorBuffer, edSurface* pZBuffer, byte alpha)
{
	edpkt_data* pRVar1;

	if (pParams != (CameraObjParams*)0x0) {
		pCamera->posX = pParams->posX;
		pCamera->posY = pParams->posY;
		pCamera->screenWidth = pParams->screenWidth;
		pCamera->screenHeight = pParams->screenHeight;
	}
	pCamera->pColorBuffer = pColorBuffer;
	pCamera->pZBuffer = pZBuffer;
	pCamera->clearColor.a = alpha;
	if (pCamera->pColorBuffer == (edSurface*)0x0) {
		pCamera->pCameraObj390_0x24->qwc = 0;
	}
	else {
		pRVar1 = edViewportUpdateEnv(pCamera, (edpkt_data*)pCamera->pCameraObj390_0x24);
		pCamera->pCameraObj390_0x24->qwc = (int)((ulong)pRVar1 - (ulong)pCamera->pCameraObj390_0x24);
	}
	return;
}

void edViewportDel(ed_viewport* pCamera, bool bDestroyFrameBuffers)
{
	if (pCamera->pCameraObj390_0x24 != (CameraObj_390*)0x0) {
		edMemFree(pCamera->pCameraObj390_0x24);
	}
	if (bDestroyFrameBuffers != false) {
		if (pCamera->pColorBuffer != (edSurface*)0x0) {
			edSurfaceDel(pCamera->pColorBuffer);
		}
		if (pCamera->pZBuffer != (edSurface*)0x0) {
			edSurfaceDel(pCamera->pZBuffer);
		}
	}
	if (VideoManager.pCamera == pCamera) {
		VideoManager.pCamera = (ed_viewport*)0x0;
	}
	edMemFree(pCamera);
	return;
}

edpkt_data* edViewportUpdateEnv(ed_viewport* pViewport, edpkt_data* pCommandBuf)
{
	edSurface* pVVar1;
	edSurface* pVVar2;
	int iVar3;
	int uVar4;
	int iVar5;
	int iVar6;
	uint uVar7;
	uint uVar8;
	int iVar9;
	int iVar10;
	int uVar11;
	int uVar12;
	int iVar13;
	int iVar14;
	int iVar15;
	int iVar16;
	int iVar17;
	int iVar18;
	int iVar19;
	int iVar20;
	edpkt_data* pRVar21;
	edpkt_data* pRVar22;

	CAMERA_LOG(LogLevel::VeryVerbose, "edViewportUpdateEnv camera: 0x{:x} ({})", (uintptr_t)pViewport, pViewport->clearColor.a);

	pVVar1 = pViewport->pColorBuffer->pSurfaceDesc->pLink_0xc;
	pCommandBuf->cmdB = SCE_GIF_PACKED_AD;

	// FRAME
	pCommandBuf[1].cmdA = SCE_GS_SET_FRAME(
		pVVar1->frameBasePtr,							// FBP
		pVVar1->pSurfaceDesc->screenWidth >> 6,		// FBW
		pVVar1->pSurfaceDesc->pixelStoreMode,		// PSM
		pViewport->fbMask									// FBMASK
	);
	pCommandBuf[1].cmdB = SCE_GS_FRAME_1;

	pRVar21 = pCommandBuf + 2;

	// SCAN MASK
	if ((pViewport->clearColor.a & 4) != 0) {
		if ((VideoManager.params18.ffmode == SCE_GS_FIELD) && (pVVar1->pNext == pVVar1)) {
			iVar3 = 3; // Drawing of pixel with odd Y coordinate is prohibited.
			if (pVVar1->pSurfaceDispEnv->csrValue_0x10 == 0) {
				iVar3 = 2; // Drawing of pixel with even Y coordinate is prohibited.
			}
			pRVar21->cmdA = SCE_GS_SET_SCANMSK(iVar3);
		}
		else {
			pRVar21->cmdA = SCE_GS_SET_SCANMSK(0); // Normal drawing (not masked.)
		}
		pRVar21->cmdB = SCE_GS_SCANMSK;
		pRVar21 = pCommandBuf + 3;
	}

	// ZBUFFER
	pVVar2 = pViewport->pZBuffer;
	if (pVVar2 != (edSurface*)0x0) {
		if ((pViewport->clearColor.a & 2) == 0) {
			iVar3 = edSurfaceGetZBufferBpp((uint)pVVar2->pSurfaceDesc->pixelStoreMode);
			pRVar21->cmdA = (ulong)pViewport->pZBuffer->frameBasePtr | ((ulong)iVar3 & 0xffffffffU) << 0x18 | 0x100000000;
			pRVar21->cmdA = SCE_GS_SET_ZBUF(
				pViewport->pZBuffer->frameBasePtr,	// ZBP
				iVar3,								// PSM
				1									// ZMASK
			);
		}
		else {
			iVar3 = edSurfaceGetZBufferBpp((uint)pVVar2->pSurfaceDesc->pixelStoreMode);
			pRVar21->cmdA = (ulong)pViewport->pZBuffer->frameBasePtr | ((ulong)iVar3 & 0xffffffffU) << 0x18;
			pRVar21->cmdA = SCE_GS_SET_ZBUF(
				pViewport->pZBuffer->frameBasePtr,	// ZBP
				iVar3,								// PSM
				0									// ZMASK
			);
		}
		pRVar21->cmdB = SCE_GS_ZBUF_1;
		pRVar21 = pRVar21 + 1;
	}

	// OFFSET
	pRVar21->cmdA = SCE_GS_SET_XYOFFSET(
		((0x1000 - ((int)pViewport->screenWidth + (int)pViewport->posX) >> 1) << 4),	// X
		((0x1000 - ((int)pViewport->screenHeight + (int)pViewport->posY) >> 1) << 4)	// Y
	);
	pRVar21->cmdB = SCE_GS_XYOFFSET_1;

	// SCISSOR
	pRVar21[1].cmdA = SCE_GS_SET_SCISSOR(
		pViewport->posX,											// SCAX0
		((int)pViewport->screenWidth + (int)pViewport->posX + -1),	// SCAX1
		pViewport->posY,											// SCAY0
		((int)pViewport->screenHeight + (int)pViewport->posY + -1)	// SCAY1
		);
	pRVar21[1].cmdB = SCE_GS_SCISSOR_1;

	// PRIM MODE CONTINUOUS
	pRVar21[2].cmdA = SCE_GS_SET_PRMODECONT(1);
	pRVar21[2].cmdB = SCE_GS_PRMODECONT;

	// COLOUR CLAMP
	pRVar21[3].cmdA = SCE_GS_SET_COLCLAMP(1);
	pRVar21[3].cmdB = SCE_GS_COLCLAMP;

	// DITHER
	pRVar21[4].cmdA = SCE_GS_SET_DTHE(0);
	pRVar21[4].cmdB = SCE_GS_DTHE;

	pRVar22 = pRVar21 + 5;

	if (((pViewport->clearColor.a & 2) != 0) || ((pViewport->clearColor.a & 1) != 0)) {

		CAMERA_LOG(LogLevel::VeryVerbose, "edViewportUpdateEnv CLEAR");

		// FRAME
		pRVar22->cmdA = SCE_GS_SET_FRAME(
			pVVar1->frameBasePtr,							// FBP
			pVVar1->pSurfaceDesc->screenWidth >> 6,		// FBW
			pVVar1->pSurfaceDesc->pixelStoreMode,		// PSM
			pViewport->clearMask								// FBMASK
		);
		pRVar21[5].cmdB = SCE_GS_FRAME_1;

		// TEST
		if (pViewport->pZBuffer == (edSurface*)0x0) {
			pRVar21[6].cmdA = SCE_GS_SET_TEST(
				0,	// ATE 
				0,	// ATST
				0,	// AREF
				0,	// AFAIL
				0,	// DATE
				0,	// DATM
				0,	// ZTE
				0	// ZTST
			);
		}
		else {
			pRVar21[6].cmdA = 0x30000;
			pRVar21[6].cmdA = SCE_GS_SET_TEST(
				0,	// ATE 
				0,	// ATST
				0,	// AREF
				0,	// AFAIL
				0,	// DATE
				0,	// DATM
				1,	// ZTE
				1	// ZTST
			);
		}
		pRVar21[6].cmdB = SCE_GS_TEST_1;

		uVar7 = 0;

		// PRIM
		pRVar21[7].cmdA = SCE_GS_SET_PRIM(
			SCE_GS_PRIM_SPRITE,		// PRIM (Primitive type)
			SCE_GS_FALSE,			// IIP  (Gouraud)
			0,						// TME  (Textured)
			0,						// FGE  (Fogging)
			SCE_GS_FALSE,			// ABE  (Alpha Blending)
			0,						// AA1  (Anti-Aliasing)
			0,						// FST  (Use ST for texture coords)
			0,						// CTXT (Context)
			0						// FIX  (Fragment control)
		);
		pRVar21[7].cmdB = SCE_GS_PRIM;

		// RGBAQ
		pRVar21[8].cmdA = SCE_GS_SET_RGBAQ(pViewport->clearColor.r, pViewport->clearColor.g, pViewport->clearColor.b, pViewport->clearColor.a, 0x3f800000);
		pRVar21[8].cmdB = SCE_GS_RGBAQ;

		pRVar22 = pRVar21 + 9;
		uVar8 = (int)pViewport->screenWidth >> 5;
		iVar9 = ((int)pViewport->posX + (0x1000 - ((int)pViewport->screenWidth + (int)pViewport->posX) >> 1)) * 0x10;
		iVar3 = ((int)pViewport->posY + (0x1000 - ((int)pViewport->screenHeight + (int)pViewport->posY) >> 1)) * 0x10;
#ifdef PLATFORM_WIN
		Renderer::SetVertexSkip(0);
#endif
		if (uVar8 != 0) {
			if (8 < uVar8) {
				iVar5 = iVar9 + 0x200;
				iVar6 = iVar9;
				do {
					pRVar22->cmdA = SCE_GS_SET_XYZ2(iVar6, iVar3, 0);
					pRVar22->cmdB = SCE_GS_XYZ2;
					iVar18 = iVar5 + 0x400;
					iVar19 = iVar6 + 0x600;
					iVar20 = iVar5 + 0x600;
					iVar17 = iVar6 + 0x800;
					pRVar22[1].cmdA = SCE_GS_SET_XYZ2(iVar5, iVar3 + pViewport->screenHeight * 0x10, 0);
					pRVar22[1].cmdB = SCE_GS_XYZ2;
					pRVar22[2].cmdA = SCE_GS_SET_XYZ2(iVar6 + 0x200, iVar3, 0);
					pRVar22[2].cmdB = SCE_GS_XYZ2;
					iVar10 = iVar6 + 0xa00;
					uVar11 = iVar6 + 0xc00;
					iVar15 = iVar5 + 0x800;
					uVar12 = iVar5 + 0xc00;
					pRVar22[3].cmdA = SCE_GS_SET_XYZ2(iVar5 + 0x200, iVar3 + pViewport->screenHeight * 0x10, 0);
					pRVar22[3].cmdB = SCE_GS_XYZ2;
					pRVar22[4].cmdA = SCE_GS_SET_XYZ2(iVar6 + 0x400, iVar3, 0);
					pRVar22[4].cmdB = SCE_GS_XYZ2;
					iVar13 = iVar6 + 0xe00;
					iVar16 = iVar5 + 0xa00;
					iVar14 = iVar5 + 0xe00;
					uVar7 = uVar7 + 8;
					iVar6 = iVar6 + 0x1000;
					iVar5 = iVar5 + 0x1000;

					pRVar22[5].cmdA = SCE_GS_SET_XYZ2(iVar18, iVar3 + pViewport->screenHeight * 0x10, 0);
					pRVar22[5].cmdB = SCE_GS_XYZ2;
					pRVar22[6].cmdA = SCE_GS_SET_XYZ2(iVar19, iVar3, 0);
					pRVar22[6].cmdB = SCE_GS_XYZ2;
					pRVar22[7].cmdA = SCE_GS_SET_XYZ2(iVar20, iVar3 + pViewport->screenHeight * 0x10, 0);
					pRVar22[7].cmdB = SCE_GS_XYZ2;
					pRVar22[8].cmdA = SCE_GS_SET_XYZ2(iVar17, iVar3, 0);
					pRVar22[8].cmdB = SCE_GS_XYZ2;
					pRVar22[9].cmdA = SCE_GS_SET_XYZ2(iVar15, iVar3 + pViewport->screenHeight * 0x10, 0);
					pRVar22[9].cmdB = SCE_GS_XYZ2;
					pRVar22[10].cmdA = SCE_GS_SET_XYZ2(iVar10, iVar3, 0);
					pRVar22[10].cmdB = SCE_GS_XYZ2;
					pRVar22[0xb].cmdA = SCE_GS_SET_XYZ2(iVar16, iVar3 + pViewport->screenHeight * 0x10, 0);
					pRVar22[0xb].cmdB = SCE_GS_XYZ2;
					pRVar22[0xc].cmdA = SCE_GS_SET_XYZ2(uVar11, iVar3, 0);
					pRVar22[0xc].cmdB = SCE_GS_XYZ2;
					pRVar22[0xd].cmdA = SCE_GS_SET_XYZ2(uVar12, iVar3 + pViewport->screenHeight * 0x10, 0);
					pRVar22[0xd].cmdB = SCE_GS_XYZ2;
					pRVar22[0xe].cmdA = SCE_GS_SET_XYZ2(iVar13, iVar3, 0);
					pRVar22[0xe].cmdB = SCE_GS_XYZ2;
					pRVar22[0xf].cmdA = SCE_GS_SET_XYZ2(iVar14, iVar3 + pViewport->screenHeight * 0x10, 0);
					pRVar22[0xf].cmdB = SCE_GS_XYZ2;
					pRVar22 = pRVar22 + 0x10;
				} while (uVar7 < uVar8 - 8);
			}
			if (uVar7 < uVar8) {
				iVar6 = iVar9 + uVar7 * 0x200;
				iVar9 = iVar9 + 0x200 + uVar7 * 0x200;
				do {
					uVar4 = iVar9;
					pRVar22->cmdA = SCE_GS_SET_XYZ2(iVar6, iVar3, 0);
					pRVar22->cmdB = SCE_GS_XYZ2;
					uVar7 = uVar7 + 1;
					iVar6 = iVar6 + 0x200;
					iVar9 = iVar9 + 0x200;
					pRVar22[1].cmdA = SCE_GS_SET_XYZ2(uVar4, iVar3 + pViewport->screenHeight * 0x10, 0);
					pRVar22[1].cmdB = SCE_GS_XYZ2;
					pRVar22 = pRVar22 + 2;
				} while (uVar7 < uVar8);
			}
		}
	}
	
	// XY OFFSET
	pRVar22->cmdA = SCE_GS_SET_XYOFFSET(
		((0x1000 - ((int)pViewport->screenWidth + (int)pViewport->posX) >> 1) << 4),	// X
		((0x1000 - ((int)pViewport->screenHeight + (int)pViewport->posY) >> 1) << 4)	// Y
	);
	pRVar22->cmdB = SCE_GS_XYOFFSET_1;

	// Z TEST
	if (pViewport->pZBuffer == (edSurface*)0x0) {
		pRVar22[1].cmdA = SCE_GS_SET_TEST(
			0,	// ATE 
			0,	// ATST
			0,	// AREF
			0,	// AFAIL
			0,	// DATE
			0,	// DATM
			0,	// ZTE
			0	// ZTST
		);
	}
	else {
		pRVar22[1].cmdA = SCE_GS_SET_TEST(
			0,	// ATE 
			0,	// ATST
			0,	// AREF
			0,	// AFAIL
			0,	// DATE
			0,	// DATM
			1,	// ZTE
			3	// ZTST
		);
	}
	pRVar22[1].cmdB = SCE_GS_TEST_1;

	// FRAME
	pRVar22[2].cmdA = SCE_GS_SET_FRAME(
		pVVar1->frameBasePtr,							// FBP
		pVVar1->pSurfaceDesc->screenWidth >> 6,		// FBW
		pVVar1->pSurfaceDesc->pixelStoreMode,		// PSM
		pViewport->fbMask									// FBMASK
	);
	pRVar22[2].cmdB = SCE_GS_FRAME_1;

	pRVar21 = pRVar22 + 3;

	// Z BUFFER
	if (pViewport->pZBuffer != (edSurface*)0x0) {
		iVar3 = edSurfaceGetZBufferBpp((uint)pViewport->pZBuffer->pSurfaceDesc->pixelStoreMode);
		pRVar21->cmdA = (ulong)pViewport->pZBuffer->frameBasePtr | ((ulong)iVar3 & 0xffffffffU) << 0x18;

		pRVar21->cmdA = SCE_GS_SET_ZBUF(
			pViewport->pZBuffer->frameBasePtr,	// ZBP
			iVar3,								// PSM
			0									// ZMASK
		);

		pRVar22[3].cmdB = SCE_GS_ZBUF_1;
		pRVar21 = pRVar22 + 4;
	}

	uint commandCount = (((ulong)pRVar21 - (ulong)pCommandBuf) >> 4) - 1;
	pCommandBuf->cmdA = SCE_GIF_SET_TAG(
		commandCount,		// NLOOP
		SCE_GS_TRUE,		// EOP
		SCE_GS_FALSE,		// PRE
		0,					// PRIM
		SCE_GIF_PACKED,		// FLG
		1					// NREG
	);

	return pRVar21;
}

void edViewPortApplyDrawingEnv(ed_viewport* pCamera)
{
	if (pCamera->pColorBuffer != (edSurface*)0x0) {
		edViewportUpdateEnv(pCamera, pCamera->pCameraObj390_0x24->commandBuffer);
		edDmaFlushCache();
		edDmaSyncPath();
#ifdef PLATFORM_PS2
		CAMERA_LOG(LogLevel::VeryVerbose, "DMA Begin edViewPortApplyDrawingEnv");
		edDmaSendN_nowait(SHELLDMA_CHANNEL_GIF, pCamera->pCameraObj390_0x24, pCamera->pCameraObj390_0x24->qwc);
#endif
	}
	return;
}

void edViewportSetBackgroundColor(ed_viewport* pCamera, byte r, byte g, byte b)
{
	(pCamera->clearColor).r = r;
	(pCamera->clearColor).g = g;
	(pCamera->clearColor).b = b;
	return;
}
