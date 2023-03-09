#include "VideoB.h"
#include "VideoA.h"
#include "LargeObject.h"
#include <assert.h>
#include <string.h>
#include "VideoC.h"
#include "VideoD.h"

VidParams8 g_VidParams8_00449588 = { 0 };
edVideoData g_ActiveVidParams_0048cd90 = { 0 };

void SetActiveVidModeData_002b9b10(VidModeData_30* param_1)
{
	param_1->pVidModeData_0x0->pLink_0xc = param_1;
	g_ActiveVidParams_0048cd90.data30 = param_1;
	FUN_002b8f70(param_1);
	return;
}

void CleanupFunc_002b9b30(void)
{
	if (g_ActiveVidParams_0048cd90.data30 != (VidModeData_30*)0x0) {
		//FreeObject_002bbd40(PTR_VidModeData_30_0048cdf0);
		g_ActiveVidParams_0048cd90.data30 = (VidModeData_30*)0x0;
	}
	//FreeObject_002bb070((long)(int)g_CameraObj_28_0048cdf4, 0);
	g_ActiveVidParams_0048cd90.pCamera = (CameraObj_28*)0x0;
	return;
}

#ifdef PLATFORM_PS2
#include "edPacket.h"
#include <shellCore.h>
#include <shellDma.h>
#endif

void ColorPacketFunc_002b9160(ulong r, ulong g, ulong b, ulong a)
{
#ifdef PLATFORM_PS2
	ulonglong* pBuffer;
	edPacket eStack32;

	pBuffer = (ulonglong*)GetScratchPadPtr_00424e10();
	if (pBuffer != (ulonglong*)0x0) {
		edPacketInit(&eStack32, 0xa0, pBuffer);
		edPacketWriteLong_0026a000(&eStack32, 0x1000000000008009);
		edPacketWriteLong_0026a000(&eStack32, 0xe);
		edPacketWriteLong_0026a000(&eStack32, 1);
		edPacketWriteLong_0026a000(&eStack32, 0x1a);
		edPacketWriteLong_0026a000(&eStack32, 0x100000);
		edPacketWriteLong_0026a000(&eStack32, 0x4c);
		edPacketWriteLong_0026a000(&eStack32, 0);
		edPacketWriteLong_0026a000(&eStack32, 0x18);
		edPacketWriteLong_0026a000(&eStack32, 0x400000004000000);
		edPacketWriteLong_0026a000(&eStack32, 0x40);
		edPacketWriteLong_0026a000(&eStack32, 0);
		edPacketWriteLong_0026a000(&eStack32, 0x47);
		edPacketWriteLong_0026a000(&eStack32, 6);
		edPacketWriteLong_0026a000(&eStack32, 0);
		edPacketWriteLong_0026a000
		(&eStack32, (a & 0xff) << 0x18 | (b & 0xff) << 0x10 | r & 0xff | (g & 0xff) << 8 | 0x3f80000000000000);
		edPacketWriteLong_0026a000(&eStack32, 1);
		edPacketWriteLong_0026a000(&eStack32, 0);
		edPacketWriteLong_0026a000(&eStack32, 4);
		edPacketWriteLong_0026a000(&eStack32, 0x40004000);
		edPacketWriteLong_0026a000(&eStack32, 4);
		edPacketClose_00269e70(&eStack32);
		shellDmaStart(SHELLDMA_CHANNEL_GIF, eStack32.pBuffer, eStack32.size_0x14);
	}
#endif
	return;
}

void FUN_002b9740(void)
{
	int iVar1;
	int iVar2;

	iVar2 = (0x800 - ((int)(uint)g_ActiveVidParams_0048cd90.params26.screenWidth >> 1)) * 0x10;
	iVar1 = (0x800 - ((int)(uint)g_ActiveVidParams_0048cd90.params26.screenHeight >> 1)) * 0x10;
	g_ActiveVidParams_0048cd90.field_0x69 = 0;
	g_ActiveVidParams_0048cd90.field_0x6a = 0;
	g_ActiveVidParams_0048cd90.field_0x6b = 0;
	g_ActiveVidParams_0048cd90.field_0x80 = 0;
	g_ActiveVidParams_0048cd90.listDataAC.field_0x0[0] = 0x1000000000008009;
	g_ActiveVidParams_0048cd90.listDataAC.field_0x0[1] = 0xe;
	g_ActiveVidParams_0048cd90.listDataAC.field_0x0[2] = 0x30003;
	g_ActiveVidParams_0048cd90.listDataAC.field_0x0[3] = 0x47;
	g_ActiveVidParams_0048cd90.listDataAC.field_0x0[4] = 0x44;
	g_ActiveVidParams_0048cd90.listDataAC.field_0x0[5] = 0x42;
	g_ActiveVidParams_0048cd90.listDataAC.field_0x0[6] = 0;
	g_ActiveVidParams_0048cd90.listDataAC.field_0x0[7] = 0x49;
	g_ActiveVidParams_0048cd90.listDataAC.field_0x0[8] = 0;
	g_ActiveVidParams_0048cd90.listDataAC.field_0x0[9] = 0x4a;
	g_ActiveVidParams_0048cd90.listDataAC.field_0x0[10] = 0x4e;
	g_ActiveVidParams_0048cd90.listDataAC.field_0x0[11] = 0;
	g_ActiveVidParams_0048cd90.listDataAC.field_0x0[12] =
		(long)(int7)((uint7)g_ActiveVidParams_0048cd90.field_0x6c << 0x18) | 0x3f80000000000000;
	g_ActiveVidParams_0048cd90.listDataAC.field_0x0[13] = 1;
	g_ActiveVidParams_0048cd90.listDataAC.field_0x0[14] = (long)iVar2 & 0xffffffffU | ((long)iVar1 & 0xffffffffU) << 0x10;
	g_ActiveVidParams_0048cd90.listDataAC.field_0x0[15] = 4;
	g_ActiveVidParams_0048cd90.listDataAC.field_0x0[16] =
		(long)(int)(iVar2 + (uint)g_ActiveVidParams_0048cd90.params26.screenWidth * 0x10) & 0xffffffffU |
		((long)(int)(iVar1 + (uint)g_ActiveVidParams_0048cd90.params26.screenHeight * 0x10) & 0xffffffffU) << 0x10;
	g_ActiveVidParams_0048cd90.listDataAC.field_0x0[17] = 4;
	g_ActiveVidParams_0048cd90.listDataAC.field_0x0[18] = 0x70000;
	g_ActiveVidParams_0048cd90.listDataAC.field_0x0[19] = 0x47;
	g_ActiveVidParams_0048cd90.listDataAC.field_0xa0 = (ulong*)((char*)&g_ActiveVidParams_0048cd90 + 0xf0);
	g_ActiveVidParams_0048cd90.listDataAC.field_0xa4 = (ulong*)((char*)&g_ActiveVidParams_0048cd90 + 0x100);
	g_ActiveVidParams_0048cd90.listDataAC.field_0xa8 = (ulong*)((char*)&g_ActiveVidParams_0048cd90 + 0x110);
	return;
}


void SetupPCRTC_002b9b80(VidParams26* pInVidParams)
{
	int canInterlace;
	int iVar3;
	ulong uVar1;
	int iVar2;
	bool bShouldInterlace;

	if (pInVidParams == (VidParams26*)0x0) {
		pInVidParams = &g_ActiveVidParams_0048cd90.params26;
	}
	g_ActiveVidParams_0048cd90.params26 = *pInVidParams;
#ifdef PLATFORM_PS2
	sceGsSyncV(0);
#endif
	ColorPacketFunc_002b9160(0, 0, 0, 0);
	canInterlace = GetCanInterlace_002b8b90();
	bShouldInterlace = canInterlace != 0;
	if (bShouldInterlace) {
		bShouldInterlace = pInVidParams->inter != 0;
	}
	g_ActiveVidParams_0048cd90.params18.ffmode = bShouldInterlace ^ 1;
	if (pInVidParams->field_0x10[0] != '\0') {
#ifdef PLATFORM_PS2
		sceGsResetGraph(0, (ushort)pInVidParams->inter, g_ActiveVidParams_0048cd90.params26.omode, (ushort)g_ActiveVidParams_0048cd90.params18.ffmode);
#endif
	}
#ifdef PLATFORM_PS2
	uVar1 = sceGsGetIMR();
	sceGsPutIMR(uVar1 & 0xfffffffffffffeff);
	uVar1 = sceGsGetIMR();
	sceGsPutIMR(uVar1 & 0xfffffffffffffdff);
#endif
	g_ActiveVidParams_0048cd90.params18.field_0xc = 0;
	g_ActiveVidParams_0048cd90.params18.gs_pmode = g_VidParams8_00449588.field_0x1 | 0x8066;
	g_ActiveVidParams_0048cd90.params18.gs_smode2 = (ushort)pInVidParams->inter | (ushort)g_ActiveVidParams_0048cd90.params18.ffmode << 1;
	g_ActiveVidParams_0048cd90.params18.field_0x4 = 0;
	g_ActiveVidParams_0048cd90.params18.field_0x8 = (pInVidParams->screenHeight << (g_ActiveVidParams_0048cd90.params18.ffmode & 0x1f)) + -1;
	g_ActiveVidParams_0048cd90.params18.field_0x6 = 0;
	g_ActiveVidParams_0048cd90.params18.field_0xa = (short)((pInVidParams->screenWidth + 0x9ff) / (uint)pInVidParams->screenWidth) + -1;
	FUN_002b8f30();
	SetupDisplayGlobals_002b8c00();
#ifdef PLATFORM_PS2
	iVar2 = 0;
	do {
		iVar3 = sceGsSyncV(0);
		iVar2 = iVar2 + 1;
	} while (iVar2 < 0x14);
#endif
	return;
	}

void FUN_002b9dd0(short param_1, undefined2 param_2)
{
	assert(false);
}

void FUN_002b9e00(undefined param_1, undefined param_2, undefined param_3)
{
	assert(false);
}

VidParams8* GetVidParams8_002b9e60(void)
{
	return &g_VidParams8_00449588;
}

void Init_edVideo_002b9e70(void)
{
	int iVar1;
	VidParams26 local_20;

	local_20.screenWidth = 0x200;
	local_20.screenHeight = 0x200;
	local_20.vidMode1 = 0;
	local_20.omode = 3;
	local_20.isNTSC = 1;
	local_20.inter = 1;
	local_20.field_0xb = 1;
	local_20.field_0xc = 1;
	local_20.field_0xd = 1;
	local_20.field_0xe = 1;
	local_20.field_0xf = 1;
	local_20.field_0x10[0] = '\x01';
	memset(&g_ActiveVidParams_0048cd90, 0, sizeof(edVideoData));
	local_20.screenWidth = 0x280;
	local_20.screenHeight = 0x100;
	local_20.vidMode1 = 0;
	local_20.omode = 3;
	local_20.isNTSC = 1;
	local_20.inter = 1;
	local_20.field_0xb = 1;
	local_20.field_0xc = 1;
	local_20.field_0xd = 1;
	local_20.field_0xe = 1;
	local_20.field_0x10[0] = g_VidParams8_00449588.field_0x4;
	SetupPCRTC_002b9b80(&local_20);
	g_ActiveVidParams_0048cd90.field_0x50 = 0;
	g_ActiveVidParams_0048cd90.field_0x51 = 0;
	g_ActiveVidParams_0048cd90.field_0x68 = 0;
#ifdef PLATFORM_PS2
	//AddInterruptHandler_002ba490();
	//AddInterruptHandler_002ba4e0();
	sceGsSyncV(0);
#endif
	ColorPacketFunc_002b9160(0, 0, 0, 0x80);
#ifdef PLATFORM_PS2
	//SetupPrivilegedGSRegisters();
	iVar1 = 0;
	do {
		sceGsSyncV(0);
		iVar1 = iVar1 + 1;
	} while (iVar1 < 0x14);
#endif
	FUN_002b8f70((VidModeData_30*)0x0);
	FUN_002b9740();
	return;
}
