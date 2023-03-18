#include "VideoC.h"
#include "VideoB.h"
#include "edMem.h"
#include <string.h>
#include "VideoD.h"

#ifdef PLATFORM_PS2
#include <libgraph.h>
#else
#include "port.h"
#endif
#include "../Camera.h"

// Not here

void FUN_002b98f0(FrameBuffer* pFrameBuffer)
{
	int iVar1;
	int iVar2;

	iVar2 = (0x800 - ((int)(uint)pFrameBuffer->pVidModeData_0x0->screenWidth >> 1)) * 0x10;
	iVar1 = (0x800 - ((int)(uint)pFrameBuffer->pVidModeData_0x0->screenHeight >> 1)) * 0x10;
	*g_ActiveVidParams_0048cd90.listDataAC.field_0xa4 = (ulong)iVar2 & 0xffffffffU | ((ulong)iVar1 & 0xffffffffU) << 0x10;
	*g_ActiveVidParams_0048cd90.listDataAC.field_0xa8 =
		(ulong)(int)(iVar2 + (uint)pFrameBuffer->pVidModeData_0x0->screenWidth * 0x10) & 0xffffffffU |
		((ulong)(int)(iVar1 + (uint)pFrameBuffer->pVidModeData_0x0->screenHeight * 0x10) & 0xffffffffU) << 0x10;
	return;
}



void AllocateVidModeData20_002b8a30(FrameBuffer* pFrameBuffer, byte interlace)
{
	VidModeData_20* pVVar1;

	pVVar1 = (VidModeData_20*)edMemAlloc(H_MAIN, sizeof(VidModeData_20));
	pFrameBuffer->pVidModeData20 = pVVar1;
	ulong dispfb2 =
		(ulong)g_VidParams8_00449588.field_0x1 << 0x2b |
		(ulong)pFrameBuffer->pVidModeData_0x0->pixelStoreMode << 0xf |
		(ulong)pFrameBuffer->frameBasePtr | (ulong)((int)(uint)pFrameBuffer->pVidModeData_0x0->screenWidth >> 6) << 9;
	pFrameBuffer->pVidModeData20->dispfb2 = *(tGS_DISPFB2*)&dispfb2;
	ulong dispfb1 =
		(ulong)pFrameBuffer->pVidModeData_0x0->pixelStoreMode << 0xf |
		(ulong)pFrameBuffer->frameBasePtr | (ulong)((int)(uint)pFrameBuffer->pVidModeData_0x0->screenWidth >> 6) << 9;
	pFrameBuffer->pVidModeData20->dispfb1 = *(tGS_DISPFB1*)&dispfb1;
	pFrameBuffer->pVidModeData20->csrValue_0x10 = interlace;
	FUN_002b98f0(pFrameBuffer);
	return;
}

void CreateZBufferTags(FrameBuffer* pFrameBuffer)
{
	ZBufferTags* pTags;
	int zBufferFormat;

	pTags = (ZBufferTags*)edMemAlloc(H_MAIN, sizeof(ZBufferTags));
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
	zBufferFormat = GetZBufferTextureFormat(pFrameBuffer->pVidModeData_0x0->pixelStoreMode);
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

int GetCanInterlace_002b8b90(void)
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

void SetupDisplayGlobals_002b8c00(void)
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
			ulong dispfb2 = (uVar1 | (ulong)(int)((uint)(ushort)g_ActiveVidParams_0048cd90.params18.DH - (uint)g_VidParams8_00449588.field_0x1) <<
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
			ulong dispfb2 = (uVar1 | (ulong)(int)((uint)(ushort)g_ActiveVidParams_0048cd90.params18.DH - (uint)g_VidParams8_00449588.field_0x1) <<
				GS_DISPLAY2_DH_O);
			g_ActiveVidParams_0048cd90.disp2 = *(tGS_DISPLAY2*)&dispfb2;
			ulong dispfb1 = ((ulong)(ushort)g_ActiveVidParams_0048cd90.params18.DH << GS_DISPLAY2_DH_O | uVar1);
			g_ActiveVidParams_0048cd90.disp1 = *(tGS_DISPLAY1*)&dispfb1;
		}
	}
	return;
}

void UpdateCameraCommands_002b8f70(FrameBuffer* pVidModeData)
{
	byte bVar1;
	CameraObjParams local_8;

	local_8.field_0x2 = 0;
	local_8.field_0x0 = 0;
	local_8.screenHeight = 0x200;
	local_8.screenWidth = 0x200;
	if (pVidModeData != (FrameBuffer*)0x0) {
		local_8.screenWidth = pVidModeData->pVidModeData_0x0->screenWidth;
		local_8.screenHeight = pVidModeData->pVidModeData_0x0->screenHeight;
	}
	bVar1 = 4;
	if (g_ActiveVidParams_0048cd90.params26.field_0xc != 0) {
		bVar1 = 6;
	}
	if (g_ActiveVidParams_0048cd90.params26.field_0xb != 0) {
		bVar1 = bVar1 | 1;
	}
	if (g_ActiveVidParams_0048cd90.pCamera == (CameraObj_28*)0x0) {
		AllocateCameraObj28_002bae70(&local_8, pVidModeData, (FrameBuffer*)0x0, bVar1);
		SetCameraObjBytes_002bb960(g_ActiveVidParams_0048cd90.pCamera, 0x20, 0x20, 0x40);
	}
	else {
		BuildCameraCommands_002bafe0(g_ActiveVidParams_0048cd90.pCamera, &local_8, pVidModeData, 0, bVar1);
		SetCameraObjBytes_002bb960
		(g_ActiveVidParams_0048cd90.pCamera, g_ActiveVidParams_0048cd90.params18.field_0x14,
			g_ActiveVidParams_0048cd90.params18.field_0x15, g_ActiveVidParams_0048cd90.params18.field_0x16);
	}
	return;
}


// End Not here

int GetVideoMemoryAlign_002bb980(int param_1, int psm, int* pPageWidth, int* pPageHeight)
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

uint GetTextureMemorySize_002bba40(uint pixels)
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

bool CreateVideoMemoryForFrameBuffer_002bbaa0(FrameBuffer* pFrameBuffer)
{
	ushort uVar1;
	VidModeData* pVVar2;
	int align;
	void* pvVar3;
	int size;
	uint width;
	uint height;
	int iStack8;
	int iStack4;

	pVVar2 = pFrameBuffer->pVidModeData_0x0;
	width = (uint)pVVar2->screenWidth;
	height = (uint)pVVar2->screenHeight;
	align = GetVideoMemoryAlign_002bb980(pVVar2->flags_0x8 & 0xf, (uint)pVVar2->pixelStoreMode, &iStack4, &iStack8);
	if ((pFrameBuffer->pVidModeData_0x0->flags_0x8 & 0x20) != 0) {
		width = GetTextureMemorySize_002bba40(width);
		height = GetTextureMemorySize_002bba40(height);
	}
	size = width * height * 2;
	uVar1 = pFrameBuffer->pVidModeData_0x0->pixelStoreMode;
	if ((uVar1 == SCE_GS_PSMCT32) || (uVar1 == SCE_GS_PSMCT24)) {
		size = width * height * 4;
	}
	pvVar3 = edMemAllocAlign(H_VIDEO, size, align);
	pFrameBuffer->data_0xc = (char*)pvVar3;
	pvVar3 = edMemGetBlockAddress((void*)pFrameBuffer->data_0xc);
	pFrameBuffer->frameBasePtr = (int)pvVar3 / align;
	return pFrameBuffer->data_0xc != 0;
}

FrameBuffer* AllocateFrameBuffers_002bbb90(VidModeData* pVidModeData)
{
	bool bVar2;
	byte interlaceFormat;
	VidModeData* pVidModeCopy;
	VidModeData* puVar2;
	FrameBuffer* bufferStart;
	uint uVar4;
	FrameBuffer* lVar5;
	FrameBuffer* pPrevFrameBuffer;
	FrameBuffer* pNewFrameBuffer;

	lVar5 = (FrameBuffer*)0x0;
	pVidModeCopy = (VidModeData*)edMemAlloc(H_MAIN, sizeof(VidModeData));
	*pVidModeCopy = *pVidModeData;

	uVar4 = 0;
	pPrevFrameBuffer = (FrameBuffer*)0x0;
	if (pVidModeData->frameBufferCount != 0) {
		do {
			pNewFrameBuffer = (FrameBuffer*)edMemAlloc(H_MAIN, sizeof(FrameBuffer));
			memset(pNewFrameBuffer, 0, sizeof(FrameBuffer));
			pNewFrameBuffer->pVidModeData_0x0 = pVidModeCopy;
			if (lVar5 == (FrameBuffer*)0x0) {
				lVar5 = pNewFrameBuffer;
			}
			if ((pVidModeData->bUseGlobalFrameBuffer == '\0') && (bVar2 = CreateVideoMemoryForFrameBuffer_002bbaa0(pNewFrameBuffer), bVar2 == false)) {
				edMemFree(pNewFrameBuffer);
				return (FrameBuffer*)0x0;
			}
			if ((pVidModeData->flags_0x8 & 0x20) != 0) {
				interlaceFormat = 0;
				if (g_ActiveVidParams_0048cd90.params18.ffmode != SCE_GS_FIELD) {
					interlaceFormat = (byte)uVar4 & 1;
				}
				AllocateVidModeData20_002b8a30(pNewFrameBuffer, interlaceFormat);
			}
			if ((pVidModeData->flags_0x8 & 8) != 0) {
				CreateZBufferTags(pNewFrameBuffer);
			}
			if (pPrevFrameBuffer != (FrameBuffer*)0x0) {
				pNewFrameBuffer->pNext = pPrevFrameBuffer;
			}
			uVar4 = uVar4 + 1;
			pPrevFrameBuffer = pNewFrameBuffer;
		} while (uVar4 < pVidModeData->frameBufferCount);
	}
	lVar5->pNext = pNewFrameBuffer;
	if ((pVidModeData->flags_0x8 & 0x20) != 0) {
		SetActiveFrameBuffer_002b9b10(pNewFrameBuffer);
	}
	return pNewFrameBuffer;
}

void DestroyFrameBuffer_002bbd40(FrameBuffer* pFrameBuffer)
{
	byte bVar1;
	VidModeData* heapID;
	FrameBuffer* pVVar2;
	uint uVar3;

	heapID = pFrameBuffer->pVidModeData_0x0;
	bVar1 = heapID->frameBufferCount;
	if (heapID != (VidModeData*)0x0) {
		edMemFree(heapID);
	}
	uVar3 = 0;
	if (bVar1 != 0) {
		do {
			if (g_ActiveVidParams_0048cd90.pFrameBuffer == pFrameBuffer) {
				g_ActiveVidParams_0048cd90.pFrameBuffer = (FrameBuffer*)0x0;
			}
			if (pFrameBuffer->pVidModeData20 != (VidModeData_20*)0x0) {
				edMemFree(pFrameBuffer->pVidModeData20);
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

void SetMode_002bbe20(FrameBuffer* param_1, ushort mode)
{
	uint uVar1;
	uint uVar2;

	uVar2 = (uint)param_1->pVidModeData_0x0->frameBufferCount;
	uVar1 = 0;
	if (uVar2 != 0) {
		do {
			param_1->pVidModeData_0x0->pixelStoreMode = mode;
			if (param_1->pVidModeData20 != (VidModeData_20*)0x0) {
				ulong dispfb2 = (ulong)g_VidParams8_00449588.field_0x1 << 0x2b |
					(ulong)param_1->pVidModeData_0x0->pixelStoreMode << 0xf |
					(ulong)param_1->frameBasePtr | (long)((int)(uint)param_1->pVidModeData_0x0->screenWidth >> 6) << 9;

				param_1->pVidModeData20->dispfb2 = *(tGS_DISPFB2*)&dispfb2;
				ulong dispfb1 = (ulong)param_1->pVidModeData_0x0->pixelStoreMode << 0xf |
					(ulong)param_1->frameBasePtr | (long)((int)(uint)param_1->pVidModeData_0x0->screenWidth >> 6) << 9;
				param_1->pVidModeData20->dispfb1 = *(tGS_DISPFB1*)&dispfb1;
			}
			uVar1 = uVar1 + 1;
			param_1 = param_1->pNext;
		} while (uVar1 < uVar2);
	}
	return;
}

char* g_szInvalidZBuffer_00433480 = "You don't have a valid zbuffer !!!!";

int GetZBufferTextureFormat(int pixelStoreMode)
{
	FrameBuffer* pVVar1;
	int outFormat;

	if (pixelStoreMode == SCE_GS_PSMCT16) {
		pVVar1 = GetVidModeData_002ba360();
		outFormat = 0x32;
		if (pVVar1->pVidModeData_0x0->pixelStoreMode != SCE_GS_PSMCT16) {
			outFormat = SCE_GS_PSMZ16S;
		}
	}
	else {
		if (pixelStoreMode == SCE_GS_PSMCT24) {
			outFormat = SCE_GS_PSMZ24;
		}
		else {
			if (pixelStoreMode == SCE_GS_PSMCT32) {
				pVVar1 = GetVidModeData_002ba360();
				if (pVVar1->pVidModeData_0x0->pixelStoreMode == SCE_GS_PSMCT16) {
					SetMode_002bbe20(pVVar1, 10);
				}
				outFormat = SCE_GS_PSMZ32;
			}
			else {
				outFormat = SCE_GS_PSMZ32;
				PrintString(g_szInvalidZBuffer_00433480);
			}
		}
	}
	return outFormat;
}
