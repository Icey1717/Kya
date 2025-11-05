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

#define VIEWPORT_LOG(level, format, ...) MY_LOG_CATEGORY("Viewport", level, format, ##__VA_ARGS__)

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
	edSurface* pSurfA;
	edSurface* pSurfB;
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
	edpkt_data* pPktA;
	edpkt_data* pPktB;

	VIEWPORT_LOG(LogLevel::VeryVerbose, "edViewportUpdateEnv pViewport: 0x{:x} ({})", (uintptr_t)pViewport, pViewport->clearColor.a);

	pSurfA = pViewport->pColorBuffer->pSurfaceDesc->pLink_0xc;
	pCommandBuf->cmdB = SCE_GIF_PACKED_AD;

	// FRAME
	pCommandBuf[1].cmdA = SCE_GS_SET_FRAME(
		pSurfA->frameBasePtr,						// FBP
		pSurfA->pSurfaceDesc->screenWidth >> 6,		// FBW
		pSurfA->pSurfaceDesc->pixelStoreMode,		// PSM
		pViewport->fbMask							// FBMASK
	);
	pCommandBuf[1].cmdB = SCE_GS_FRAME_1;

	VIEWPORT_LOG(LogLevel::VeryVerbose, "edViewportUpdateEnv FBMASK mask: 0x{:x}", pViewport->fbMask);

	pPktA = pCommandBuf + 2;

	// SCAN MASK
	if ((pViewport->clearColor.a & 4) != 0) {
		if ((VideoManager.params18.ffmode == SCE_GS_FIELD) && (pSurfA->pNext == pSurfA)) {
			iVar3 = 3; // Drawing of pixel with odd Y coordinate is prohibited.
			if (pSurfA->pSurfaceDispEnv->csrValue_0x10 == 0) {
				iVar3 = 2; // Drawing of pixel with even Y coordinate is prohibited.
			}
			pPktA->cmdA = SCE_GS_SET_SCANMSK(iVar3);
		}
		else {
			pPktA->cmdA = SCE_GS_SET_SCANMSK(0); // Normal drawing (not masked.)
		}
		pPktA->cmdB = SCE_GS_SCANMSK;
		pPktA = pCommandBuf + 3;
	}

	// ZBUFFER
	pSurfB = pViewport->pZBuffer;
	if (pSurfB != (edSurface*)0x0) {
		if ((pViewport->clearColor.a & 2) == 0) {
			iVar3 = edSurfaceGetZBufferBpp((uint)pSurfB->pSurfaceDesc->pixelStoreMode);
			pPktA->cmdA = (ulong)pViewport->pZBuffer->frameBasePtr | ((ulong)iVar3 & 0xffffffffU) << 0x18 | 0x100000000;
			pPktA->cmdA = SCE_GS_SET_ZBUF(
				pViewport->pZBuffer->frameBasePtr,	// ZBP
				iVar3,								// PSM
				SCE_GS_TRUE							// ZMASK
			);
		}
		else {
			iVar3 = edSurfaceGetZBufferBpp((uint)pSurfB->pSurfaceDesc->pixelStoreMode);
			pPktA->cmdA = (ulong)pViewport->pZBuffer->frameBasePtr | ((ulong)iVar3 & 0xffffffffU) << 0x18;
			pPktA->cmdA = SCE_GS_SET_ZBUF(
				pViewport->pZBuffer->frameBasePtr,	// ZBP
				iVar3,								// PSM
				SCE_GS_FALSE						// ZMASK
			);
		}

		pPktA->cmdB = SCE_GS_ZBUF_1;
		pPktA = pPktA + 1;
	}

	// OFFSET
	pPktA->cmdA = SCE_GS_SET_XYOFFSET(
		((0x1000 - ((int)pViewport->screenWidth + (int)pViewport->posX) >> 1) << 4),	// X
		((0x1000 - ((int)pViewport->screenHeight + (int)pViewport->posY) >> 1) << 4)	// Y
	);
	pPktA->cmdB = SCE_GS_XYOFFSET_1;

	VIEWPORT_LOG(LogLevel::VeryVerbose, "edViewportUpdateEnv posX: {}, posY: {}, screenWidth: {}, screenHeight: {}",
		pViewport->posX, pViewport->posY, pViewport->screenWidth, pViewport->screenHeight);

	// SCISSOR
	pPktA[1].cmdA = SCE_GS_SET_SCISSOR(
		pViewport->posX,											// SCAX0
		((int)pViewport->screenWidth + (int)pViewport->posX + -1),	// SCAX1
		pViewport->posY,											// SCAY0
		((int)pViewport->screenHeight + (int)pViewport->posY + -1)	// SCAY1
		);
	pPktA[1].cmdB = SCE_GS_SCISSOR_1;

	// PRIM MODE CONTINUOUS
	pPktA[2].cmdA = SCE_GS_SET_PRMODECONT(1);
	pPktA[2].cmdB = SCE_GS_PRMODECONT;

	// COLOUR CLAMP
	pPktA[3].cmdA = SCE_GS_SET_COLCLAMP(1);
	pPktA[3].cmdB = SCE_GS_COLCLAMP;

	// DITHER
	pPktA[4].cmdA = SCE_GS_SET_DTHE(0);
	pPktA[4].cmdB = SCE_GS_DTHE;

	pPktB = pPktA + 5;

	if (((pViewport->clearColor.a & 2) != 0) || ((pViewport->clearColor.a & 1) != 0)) {
		VIEWPORT_LOG(LogLevel::VeryVerbose, "edViewportUpdateEnv CLEAR");

		Renderer::Native::EClearMode clearMode = Renderer::Native::EClearMode::ColorDepth;

		if (pViewport->clearColor.a == 3) {
			clearMode = Renderer::Native::EClearMode::ColorDepth;
			VIEWPORT_LOG(LogLevel::VeryVerbose, "edViewportUpdateEnv CLEAR DEPTH COLOR");
		}

		if (pViewport->clearColor.a == 2) {
			clearMode = Renderer::Native::EClearMode::Depth;
			VIEWPORT_LOG(LogLevel::VeryVerbose, "edViewportUpdateEnv CLEAR DEPTH");
		}

		if (pViewport->clearColor.a = 1) {
			clearMode = Renderer::Native::EClearMode::Depth;
			VIEWPORT_LOG(LogLevel::VeryVerbose, "edViewportUpdateEnv CLEAR DEPTH 2");
		}

		if (pViewport->clearMask == 0x0)
		{
			clearMode = Renderer::Native::EClearMode::None;
			VIEWPORT_LOG(LogLevel::VeryVerbose, "edViewportUpdateEnv CLEAR NONE");
		}

		Renderer::Native::UpdateRenderPassKey(clearMode);

		// FRAME
		pPktB->cmdA = SCE_GS_SET_FRAME(
			pSurfA->frameBasePtr,						// FBP
			pSurfA->pSurfaceDesc->screenWidth >> 6,		// FBW
			pSurfA->pSurfaceDesc->pixelStoreMode,		// PSM
			pViewport->clearMask						// FBMASK
		);
		pPktA[5].cmdB = SCE_GS_FRAME_1;

		VIEWPORT_LOG(LogLevel::VeryVerbose, "edViewportUpdateEnv Clear mask: 0x{:x}", pViewport->clearMask);

		// TEST
		if (pViewport->pZBuffer == (edSurface*)0x0) {
			VIEWPORT_LOG(LogLevel::VeryVerbose, "edViewportUpdateEnv Disable Z Buffering");
			pPktA[6].cmdA = SCE_GS_SET_TEST(
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
			VIEWPORT_LOG(LogLevel::VeryVerbose, "edViewportUpdateEnv Enable Z Buffering");
			pPktA[6].cmdA = 0x30000;
			pPktA[6].cmdA = SCE_GS_SET_TEST(
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
		pPktA[6].cmdB = SCE_GS_TEST_1;

		uVar7 = 0;

		// PRIM
		pPktA[7].cmdA = SCE_GS_SET_PRIM(
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
		pPktA[7].cmdB = SCE_GS_PRIM;

		VIEWPORT_LOG(LogLevel::VeryVerbose, "edViewportUpdateEnv Clear color: r: {}, g: {}, b: {}, a: {}",
			pViewport->clearColor.r, pViewport->clearColor.g, pViewport->clearColor.b, pViewport->clearColor.a);

		// RGBAQ
		pPktA[8].cmdA = SCE_GS_SET_RGBAQ(pViewport->clearColor.r, pViewport->clearColor.g, pViewport->clearColor.b, pViewport->clearColor.a, 0x3f800000);
		pPktA[8].cmdB = SCE_GS_RGBAQ;

		pPktB = pPktA + 9;
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
					pPktB->cmdA = SCE_GS_SET_XYZ2(iVar6, iVar3, 0);
					pPktB->cmdB = SCE_GS_XYZ2;
					iVar18 = iVar5 + 0x400;
					iVar19 = iVar6 + 0x600;
					iVar20 = iVar5 + 0x600;
					iVar17 = iVar6 + 0x800;
					pPktB[1].cmdA = SCE_GS_SET_XYZ2(iVar5, iVar3 + pViewport->screenHeight * 0x10, 0);
					pPktB[1].cmdB = SCE_GS_XYZ2;
					pPktB[2].cmdA = SCE_GS_SET_XYZ2(iVar6 + 0x200, iVar3, 0);
					pPktB[2].cmdB = SCE_GS_XYZ2;
					iVar10 = iVar6 + 0xa00;
					uVar11 = iVar6 + 0xc00;
					iVar15 = iVar5 + 0x800;
					uVar12 = iVar5 + 0xc00;
					pPktB[3].cmdA = SCE_GS_SET_XYZ2(iVar5 + 0x200, iVar3 + pViewport->screenHeight * 0x10, 0);
					pPktB[3].cmdB = SCE_GS_XYZ2;
					pPktB[4].cmdA = SCE_GS_SET_XYZ2(iVar6 + 0x400, iVar3, 0);
					pPktB[4].cmdB = SCE_GS_XYZ2;
					iVar13 = iVar6 + 0xe00;
					iVar16 = iVar5 + 0xa00;
					iVar14 = iVar5 + 0xe00;
					uVar7 = uVar7 + 8;
					iVar6 = iVar6 + 0x1000;
					iVar5 = iVar5 + 0x1000;

					pPktB[5].cmdA = SCE_GS_SET_XYZ2(iVar18, iVar3 + pViewport->screenHeight * 0x10, 0);
					pPktB[5].cmdB = SCE_GS_XYZ2;
					pPktB[6].cmdA = SCE_GS_SET_XYZ2(iVar19, iVar3, 0);
					pPktB[6].cmdB = SCE_GS_XYZ2;
					pPktB[7].cmdA = SCE_GS_SET_XYZ2(iVar20, iVar3 + pViewport->screenHeight * 0x10, 0);
					pPktB[7].cmdB = SCE_GS_XYZ2;
					pPktB[8].cmdA = SCE_GS_SET_XYZ2(iVar17, iVar3, 0);
					pPktB[8].cmdB = SCE_GS_XYZ2;
					pPktB[9].cmdA = SCE_GS_SET_XYZ2(iVar15, iVar3 + pViewport->screenHeight * 0x10, 0);
					pPktB[9].cmdB = SCE_GS_XYZ2;
					pPktB[10].cmdA = SCE_GS_SET_XYZ2(iVar10, iVar3, 0);
					pPktB[10].cmdB = SCE_GS_XYZ2;
					pPktB[0xb].cmdA = SCE_GS_SET_XYZ2(iVar16, iVar3 + pViewport->screenHeight * 0x10, 0);
					pPktB[0xb].cmdB = SCE_GS_XYZ2;
					pPktB[0xc].cmdA = SCE_GS_SET_XYZ2(uVar11, iVar3, 0);
					pPktB[0xc].cmdB = SCE_GS_XYZ2;
					pPktB[0xd].cmdA = SCE_GS_SET_XYZ2(uVar12, iVar3 + pViewport->screenHeight * 0x10, 0);
					pPktB[0xd].cmdB = SCE_GS_XYZ2;
					pPktB[0xe].cmdA = SCE_GS_SET_XYZ2(iVar13, iVar3, 0);
					pPktB[0xe].cmdB = SCE_GS_XYZ2;
					pPktB[0xf].cmdA = SCE_GS_SET_XYZ2(iVar14, iVar3 + pViewport->screenHeight * 0x10, 0);
					pPktB[0xf].cmdB = SCE_GS_XYZ2;
					pPktB = pPktB + 0x10;
				} while (uVar7 < uVar8 - 8);
			}
			if (uVar7 < uVar8) {
				iVar6 = iVar9 + uVar7 * 0x200;
				iVar9 = iVar9 + 0x200 + uVar7 * 0x200;
				do {
					uVar4 = iVar9;
					pPktB->cmdA = SCE_GS_SET_XYZ2(iVar6, iVar3, 0);
					pPktB->cmdB = SCE_GS_XYZ2;
					uVar7 = uVar7 + 1;
					iVar6 = iVar6 + 0x200;
					iVar9 = iVar9 + 0x200;
					pPktB[1].cmdA = SCE_GS_SET_XYZ2(uVar4, iVar3 + pViewport->screenHeight * 0x10, 0);
					pPktB[1].cmdB = SCE_GS_XYZ2;
					pPktB = pPktB + 2;
				} while (uVar7 < uVar8);
			}
		}
	}
	
	// XY OFFSET
	pPktB->cmdA = SCE_GS_SET_XYOFFSET(
		((0x1000 - ((int)pViewport->screenWidth + (int)pViewport->posX) >> 1) << 4),	// X
		((0x1000 - ((int)pViewport->screenHeight + (int)pViewport->posY) >> 1) << 4)	// Y
	);
	pPktB->cmdB = SCE_GS_XYOFFSET_1;

	// Z TEST
	if (pViewport->pZBuffer == (edSurface*)0x0) {
		pPktB[1].cmdA = SCE_GS_SET_TEST(
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
		pPktB[1].cmdA = SCE_GS_SET_TEST(
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
	pPktB[1].cmdB = SCE_GS_TEST_1;

	// FRAME
	pPktB[2].cmdA = SCE_GS_SET_FRAME(
		pSurfA->frameBasePtr,						// FBP
		pSurfA->pSurfaceDesc->screenWidth >> 6,		// FBW
		pSurfA->pSurfaceDesc->pixelStoreMode,		// PSM
		pViewport->fbMask							// FBMASK
	);
	pPktB[2].cmdB = SCE_GS_FRAME_1;

	pPktA = pPktB + 3;

	// Z BUFFER
	if (pViewport->pZBuffer != (edSurface*)0x0) {
		iVar3 = edSurfaceGetZBufferBpp((uint)pViewport->pZBuffer->pSurfaceDesc->pixelStoreMode);
		pPktA->cmdA = (ulong)pViewport->pZBuffer->frameBasePtr | ((ulong)iVar3 & 0xffffffffU) << 0x18;

		pPktA->cmdA = SCE_GS_SET_ZBUF(
			pViewport->pZBuffer->frameBasePtr,	// ZBP
			iVar3,								// PSM
			SCE_GS_FALSE						// ZMASK
		);

		pPktB[3].cmdB = SCE_GS_ZBUF_1;
		pPktA = pPktB + 4;
	}

	uint commandCount = (((ulong)pPktA - (ulong)pCommandBuf) >> 4) - 1;
	pCommandBuf->cmdA = SCE_GIF_SET_TAG(
		commandCount,		// NLOOP
		SCE_GS_TRUE,		// EOP
		SCE_GS_FALSE,		// PRE
		0,					// PRIM
		SCE_GIF_PACKED,		// FLG
		1					// NREG
	);

	return pPktA;
}

void edViewPortApplyDrawingEnv(ed_viewport* pViewport)
{
	if (pViewport->pColorBuffer != (edSurface*)0x0) {
		edViewportUpdateEnv(pViewport, pViewport->pCameraObj390_0x24->commandBuffer);
		edDmaFlushCache();
		edDmaSyncPath();
#ifdef PLATFORM_PS2
		VIEWPORT_LOG(LogLevel::VeryVerbose, "DMA Begin edViewPortApplyDrawingEnv");
		edDmaSendN_nowait(SHELLDMA_CHANNEL_GIF, pViewport->pCameraObj390_0x24, pViewport->pCameraObj390_0x24->qwc);
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
