#include "Camera.h"
#include "edVideo/VideoB.h"
#include "edMem.h"
#include "edVideo/VideoC.h"
#include <string.h>
#include <assert.h>

#ifdef PLATFORM_WIN
#include "port.h"
#endif

RenderCommand* BuildCameraCommands_002bb110(CameraObj_28* pCamera, RenderCommand* pCommandBuf)
{
	FrameBuffer* pVVar1;
	FrameBuffer* pVVar2;
	int iVar3;
	ulong uVar4;
	int iVar5;
	int iVar6;
	uint uVar7;
	uint uVar8;
	int iVar9;
	int iVar10;
	uint uVar11;
	uint uVar12;
	int iVar13;
	int iVar14;
	int iVar15;
	int iVar16;
	int iVar17;
	int iVar18;
	int iVar19;
	int iVar20;
	RenderCommand* pRVar21;
	RenderCommand* pRVar22;

	pVVar1 = pCamera->pVidModeData30_0x10->pVidModeData_0x0->pLink_0xc;
	pCommandBuf->cmdB = SCE_GIF_PACKED_AD;

	// FRAME
	pCommandBuf[1].cmdA = SCE_GS_SET_FRAME(
		pVVar1->frameBasePtr,							// FBP
		pVVar1->pVidModeData_0x0->screenWidth >> 6,		// FBW
		pVVar1->pVidModeData_0x0->pixelStoreMode,		// PSM
		pCamera->fbMask									// FBMASK
	);
	pCommandBuf[1].cmdB = SCE_GS_FRAME_1;

	pRVar21 = pCommandBuf + 2;

	// SCAN MASK
	if ((pCamera->field_0x18[3] & 4) != 0) {
		if ((g_ActiveVidParams_0048cd90.params18.ffmode == SCE_GS_FIELD) && (pVVar1->pNext == pVVar1)) {
			iVar3 = 3; // Drawing of pixel with odd Y coordinate is prohibited.
			if (pVVar1->pVidModeData20->csrValue_0x10 == 0) {
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
	pVVar2 = pCamera->pZBuffer;
	if (pVVar2 != (FrameBuffer*)0x0) {
		if ((pCamera->field_0x18[3] & 2) == 0) {
			iVar3 = GetZBufferTextureFormat((uint)pVVar2->pVidModeData_0x0->pixelStoreMode);
			pRVar21->cmdA = (ulong)pCamera->pZBuffer->frameBasePtr | ((ulong)iVar3 & 0xffffffffU) << 0x18 | 0x100000000;
			pRVar21->cmdA = SCE_GS_SET_ZBUF(
				pCamera->pZBuffer->frameBasePtr,	// ZBP
				iVar3,								// PSM
				1									// ZMASK
			);
		}
		else {
			iVar3 = GetZBufferTextureFormat((uint)pVVar2->pVidModeData_0x0->pixelStoreMode);
			pRVar21->cmdA = (ulong)pCamera->pZBuffer->frameBasePtr | ((ulong)iVar3 & 0xffffffffU) << 0x18;
			pRVar21->cmdA = SCE_GS_SET_ZBUF(
				pCamera->pZBuffer->frameBasePtr,	// ZBP
				iVar3,								// PSM
				0									// ZMASK
			);
		}
		pRVar21->cmdB = SCE_GS_ZBUF_1;
		pRVar21 = pRVar21 + 1;
	}

	// OFFSET
	pRVar21->cmdA = SCE_GS_SET_XYOFFSET(
		((0x1000 - ((int)pCamera->screenWidth + (int)pCamera->posX) >> 1) << 4),	// X
		((0x1000 - ((int)pCamera->screenHeight + (int)pCamera->posY) >> 1) << 4)	// Y
	);
	pRVar21->cmdB = SCE_GS_XYOFFSET_1;

	// SCISSOR
	pRVar21[1].cmdA = SCE_GS_SET_SCISSOR(
		pCamera->posX,											// SCAX0
		((int)pCamera->screenWidth + (int)pCamera->posX + -1),	// SCAX1
		pCamera->posY,											// SCAY0
		((int)pCamera->screenHeight + (int)pCamera->posY + -1)	// SCAY1
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

	if (((pCamera->field_0x18[3] & 2) != 0) || ((pCamera->field_0x18[3] & 1) != 0)) {
		pRVar22->cmdA =
			(ulong)pCamera->field_0x20 << 0x20 |
			(ulong)pVVar1->pVidModeData_0x0->pixelStoreMode << 0x18 |
			(ulong)pVVar1->frameBasePtr | (ulong)((int)(uint)pVVar1->pVidModeData_0x0->screenWidth >> 6) << 0x10;
		pRVar21[5].cmdB = 0x4c;
		if (pCamera->pZBuffer == (FrameBuffer*)0x0) {
			pRVar21[6].cmdA = 0;
		}
		else {
			pRVar21[6].cmdA = 0x30000;
		}
		pRVar21[6].cmdB = 0x47;
		uVar7 = 0;
		pRVar21[7].cmdA = 6;
		pRVar21[7].cmdB = 0;
		pRVar21[8].cmdA = (ulong) * (uint3*)pCamera->field_0x18 | 0x3f80000000000000;
		pRVar21[8].cmdB = 1;
		pRVar22 = pRVar21 + 9;
		uVar8 = (int)pCamera->screenWidth >> 5;
		iVar9 = ((int)pCamera->posX + (0x1000 - ((int)pCamera->screenWidth + (int)pCamera->posX) >> 1)) * 0x10;
		iVar3 = ((int)pCamera->posY + (0x1000 - ((int)pCamera->screenHeight + (int)pCamera->posY) >> 1)) * 0x10;
		if (uVar8 != 0) {
			if (8 < uVar8) {
				iVar5 = iVar9 + 0x200;
				uVar4 = ((ulong)iVar3 & 0xffffffffU) << 0x10;
				iVar6 = iVar9;
				do {
					pRVar22->cmdA = uVar4 | (ulong)iVar6 & 0xffffffffU;
					pRVar22->cmdB = 5;
					iVar18 = iVar5 + 0x400;
					iVar19 = iVar6 + 0x600;
					iVar20 = iVar5 + 0x600;
					iVar17 = iVar6 + 0x800;
					pRVar22[1].cmdA =
						(ulong)iVar5 & 0xffffffffU | ((ulong)(iVar3 + pCamera->screenHeight * 0x10) & 0xffffffffU) << 0x10;
					pRVar22[1].cmdB = 5;
					pRVar22[2].cmdA = uVar4 | (ulong)(iVar6 + 0x200) & 0xffffffffU;
					iVar10 = iVar6 + 0xa00;
					pRVar22[2].cmdB = 5;
					uVar11 = iVar6 + 0xc00;
					iVar15 = iVar5 + 0x800;
					uVar12 = iVar5 + 0xc00;
					pRVar22[3].cmdA =
						(ulong)(iVar5 + 0x200) & 0xffffffffU |
						((ulong)(iVar3 + pCamera->screenHeight * 0x10) & 0xffffffffU) << 0x10;
					iVar16 = iVar5 + 0xa00;
					pRVar22[3].cmdB = 5;
					pRVar22[4].cmdA = uVar4 | (ulong)(iVar6 + 0x400) & 0xffffffffU;
					iVar13 = iVar6 + 0xe00;
					pRVar22[4].cmdB = 5;
					iVar14 = iVar5 + 0xe00;
					uVar7 = uVar7 + 8;
					iVar6 = iVar6 + 0x1000;
					iVar5 = iVar5 + 0x1000;
					pRVar22[5].cmdA =
						(ulong)iVar18 & 0xffffffffU | ((ulong)(iVar3 + pCamera->screenHeight * 0x10) & 0xffffffffU) << 0x10;
					pRVar22[5].cmdB = 5;
					pRVar22[6].cmdA = uVar4 | (ulong)iVar19 & 0xffffffffU;
					pRVar22[6].cmdB = 5;
					pRVar22[7].cmdA =
						(ulong)iVar20 & 0xffffffffU | ((ulong)(iVar3 + pCamera->screenHeight * 0x10) & 0xffffffffU) << 0x10;
					pRVar22[7].cmdB = 5;
					pRVar22[8].cmdA = uVar4 | (ulong)iVar17 & 0xffffffffU;
					pRVar22[8].cmdB = 5;
					pRVar22[9].cmdA =
						(ulong)iVar15 & 0xffffffffU | ((ulong)(iVar3 + pCamera->screenHeight * 0x10) & 0xffffffffU) << 0x10;
					pRVar22[9].cmdB = 5;
					pRVar22[10].cmdA = uVar4 | (ulong)iVar10 & 0xffffffffU;
					pRVar22[10].cmdB = 5;
					pRVar22[0xb].cmdA =
						(ulong)iVar16 & 0xffffffffU | ((ulong)(iVar3 + pCamera->screenHeight * 0x10) & 0xffffffffU) << 0x10;
					pRVar22[0xb].cmdB = 5;
					pRVar22[0xc].cmdA = uVar4 & 0xffffffff00000000 | (ulong)((uint)uVar4 | uVar11);
					pRVar22[0xc].cmdB = 5;
					pRVar22[0xd].cmdA = (ulong)uVar12 | ((ulong)(iVar3 + pCamera->screenHeight * 0x10) & 0xffffffffU) << 0x10;
					pRVar22[0xd].cmdB = 5;
					pRVar22[0xe].cmdA = uVar4 | (ulong)iVar13 & 0xffffffffU;
					pRVar22[0xe].cmdB = 5;
					pRVar22[0xf].cmdA =
						(ulong)iVar14 & 0xffffffffU | ((ulong)(iVar3 + pCamera->screenHeight * 0x10) & 0xffffffffU) << 0x10;
					pRVar22[0xf].cmdB = 5;
					pRVar22 = pRVar22 + 0x10;
				} while (uVar7 < uVar8 - 8);
			}
			if (uVar7 < uVar8) {
				iVar6 = iVar9 + uVar7 * 0x200;
				iVar9 = iVar9 + 0x200 + uVar7 * 0x200;
				do {
					uVar4 = iVar9;
					pRVar22->cmdA = ((ulong)iVar3 & 0xffffffffU) << 0x10 | (ulong)iVar6 & 0xffffffffU;
					uVar7 = uVar7 + 1;
					pRVar22->cmdB = 5;
					iVar6 = iVar6 + 0x200;
					iVar9 = iVar9 + 0x200;
					pRVar22[1].cmdA = uVar4 & 0xffffffff | ((ulong)(iVar3 + pCamera->screenHeight * 0x10) & 0xffffffffU) << 0x10;
					pRVar22[1].cmdB = 5;
					pRVar22 = pRVar22 + 2;
				} while (uVar7 < uVar8);
			}
		}
	}
	
	// XY OFFSET
	pRVar22->cmdA = SCE_GS_SET_XYOFFSET(
		((0x1000 - ((int)pCamera->screenWidth + (int)pCamera->posX) >> 1) << 4),	// X
		((0x1000 - ((int)pCamera->screenHeight + (int)pCamera->posY) >> 1) << 4)	// Y
	);
	pRVar22->cmdB = SCE_GS_XYOFFSET_1;

	// Z TEST
	if (pCamera->pZBuffer == (FrameBuffer*)0x0) {
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
		pRVar22[1].cmdA = 0x70000;
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
		pVVar1->pVidModeData_0x0->screenWidth >> 6,		// FBW
		pVVar1->pVidModeData_0x0->pixelStoreMode,		// PSM
		pCamera->fbMask									// FBMASK
	);
	pRVar22[2].cmdB = SCE_GS_FRAME_1;

	pRVar21 = pRVar22 + 3;

	// Z BUFFER
	if (pCamera->pZBuffer != (FrameBuffer*)0x0) {
		iVar3 = GetZBufferTextureFormat((uint)pCamera->pZBuffer->pVidModeData_0x0->pixelStoreMode);
		pRVar21->cmdA = (ulong)pCamera->pZBuffer->frameBasePtr | ((ulong)iVar3 & 0xffffffffU) << 0x18;

		pRVar21->cmdA = SCE_GS_SET_ZBUF(
			pCamera->pZBuffer->frameBasePtr,	// ZBP
			iVar3,								// PSM
			0									// ZMASK
		);

		pRVar22[3].cmdB = SCE_GS_ZBUF_1;
		pRVar21 = pRVar22 + 4;
	}

	pCommandBuf->cmdA = SCE_GIF_SET_TAG(
		(int)(((uint)((ulong)pRVar21 - (ulong)pCommandBuf) >> 4) - 1), // NLOOP
		SCE_GS_TRUE,		// EOP
		SCE_GS_FALSE,		// PRE
		0,					// PRIM
		SCE_GIF_PACKED,		// FLG
		1					// NREG
	);

	return pRVar21;
}

CameraObj_28*
AllocateCameraObj28_002bae70
(CameraObjParams* pParams, FrameBuffer* pVidModeDataA, FrameBuffer* pVidModeDataB, byte param_4)
{
	CameraObj_28* __s;
	CameraObj_390* pCVar1;
	RenderCommand* pRVar2;
	CameraObj_28* pReturnValue;
	CameraObjParams local_8;

	if (pParams == (CameraObjParams*)0x0) {
		local_8.field_0x0 = 0;
		local_8.field_0x2 = 0;
		if (pVidModeDataA == (FrameBuffer*)0x0) {
			local_8.screenWidth = 0x200;
			local_8.screenHeight = 0x200;
		}
		else {
			local_8.screenWidth = pVidModeDataA->pVidModeData_0x0->screenWidth;
			local_8.screenHeight = pVidModeDataA->pVidModeData_0x0->screenHeight;
		}
		pParams = &local_8;
	}
	__s = (CameraObj_28*)edMemAlloc(H_MAIN, sizeof(CameraObj_28));
	memset(__s, 0, sizeof(CameraObj_28));
	__s->field_0x18[3] = param_4;
	__s->fbMask = 0;
	if ((param_4 & 1) == 0) {
		__s->field_0x20 = 0xffffffff;
	}
	else {
		__s->field_0x20 = 0;
	}
	__s->posX = pParams->field_0x0;
	__s->posY = pParams->field_0x2;
	__s->screenWidth = pParams->screenWidth;
	__s->screenHeight = pParams->screenHeight;
	__s->pVidModeData30_0x10 = pVidModeDataA;
	__s->pZBuffer = pVidModeDataB;
	__s->pCameraObj390_0x24 = (CameraObj_390*)0x0;
	__s->field_0x18[2] = 0x40;
	__s->field_0x18[1] = 0x40;
	__s->field_0x18[0] = 0x40;
	if ((param_4 & 4) != 0) {
		g_ActiveVidParams_0048cd90.pCamera = __s;
		pCVar1 = (CameraObj_390*)edMemAlloc(H_MAIN, sizeof(CameraObj_390));
		__s->pCameraObj390_0x24 = pCVar1;
		if (__s->pVidModeData30_0x10 == (FrameBuffer*)0x0) {
			__s->pCameraObj390_0x24->qwc = 0;
		}
		else {
			pRVar2 = BuildCameraCommands_002bb110(__s, __s->pCameraObj390_0x24->commandBuffer);
			__s->pCameraObj390_0x24->qwc = (int)((ulong)pRVar2 - (ulong)__s->pCameraObj390_0x24);
		}
	}
	return __s;
}

void SetCameraObjBytes_002bb960(CameraObj_28* param_1, byte param_2, byte param_3, byte param_4)
{
	param_1->field_0x18[0] = param_2;
	param_1->field_0x18[1] = param_3;
	param_1->field_0x18[2] = param_4;
	return;
}

void BuildCameraCommands_002bafe0(CameraObj_28* pCamera, CameraObjParams* param_2, FrameBuffer* param_3, FrameBuffer* param_4, byte param_5)
{
	RenderCommand* pRVar1;

	if (param_2 != (CameraObjParams*)0x0) {
		pCamera->posX = param_2->field_0x0;
		pCamera->posY = param_2->field_0x2;
		pCamera->screenWidth = param_2->screenWidth;
		pCamera->screenHeight = param_2->screenHeight;
	}
	pCamera->pVidModeData30_0x10 = param_3;
	pCamera->pZBuffer = param_4;
	pCamera->field_0x18[3] = param_5;
	if (pCamera->pVidModeData30_0x10 == (FrameBuffer*)0x0) {
		pCamera->pCameraObj390_0x24->qwc = 0;
	}
	else {
		pRVar1 = BuildCameraCommands_002bb110(pCamera, (RenderCommand*)pCamera->pCameraObj390_0x24);
		pCamera->pCameraObj390_0x24->qwc = (int)((ulong)pRVar1 - (ulong)pCamera->pCameraObj390_0x24);
	}
	return;
}