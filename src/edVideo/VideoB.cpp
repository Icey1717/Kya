#include "VideoB.h"
#include "VideoA.h"
#include "LargeObject.h"
#include <assert.h>
#include <string.h>
#include "VideoC.h"
#include "VideoD.h"
#include "edMem.h"

#ifdef PLATFORM_WIN
#include "port.h"
#include "renderer.h"
#endif

#include "Rendering/CustomShell.h"
#include "Viewport.h"

edVideoConfig edVideoConfiguration = { 0 };
ed_video_attr VideoManager;

void _VideoManagerAddFlippingSurface(edSurface* pNewFrameBuffer)
{
	pNewFrameBuffer->pSurfaceDesc->pLink_0xc = pNewFrameBuffer;
	VideoManager.pFrameBuffer = pNewFrameBuffer;
	_VideoUpdateSystemViewport(pNewFrameBuffer);
	return;
}

void CleanupFunc_002b9b30(void)
{
	if (VideoManager.pFrameBuffer != (edSurface*)0x0) {
		edSurfaceDel(VideoManager.pFrameBuffer);
		VideoManager.pFrameBuffer = (edSurface*)0x0;
	}
	edViewportDel(VideoManager.pCamera, false);
	VideoManager.pCamera = (ed_viewport*)0x0;
	return;
}

#ifdef PLATFORM_PS2
#include "edPacket.h"
#endif

void edVideoClearVram(ulong r, ulong g, ulong b, ulong a)
{
#ifdef PLATFORM_PS2
	ulonglong* pBuffer;
	edPacket eStack32;

	pBuffer = (ulonglong*)edSystemFastRamGetAddr();
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
		MY_LOG_CATEGORY("Video", LogLevel::VeryVerbose, "DMA Begin edVideoClearVram\n");
		edDmaSendN(SHELLDMA_CHANNEL_GIF, eStack32.pBuffer, eStack32.size_0x14);
	}
#endif
	return;
}

void _FadeInit(void)
{
	int iVar1;
	int iVar2;

	iVar2 = (0x800 - ((int)(uint)VideoManager.screenWidth >> 1)) * 0x10;
	iVar1 = (0x800 - ((int)(uint)VideoManager.screenHeight >> 1)) * 0x10;
	VideoManager.fadeColorR = 0;
	VideoManager.fadeColorG = 0;
	VideoManager.fadeColorB = 0;
	VideoManager.fadeFlags = 0;

	// GIF TAG
	VideoManager.listDataAC.field_0x0[0] = SCE_GIF_SET_TAG(
		9,				// NLOOP
		SCE_GS_TRUE,	// EOP
		SCE_GS_FALSE,	// PRE
		0,				// PRIM
		SCE_GIF_PACKED, // FLG
		1				// NREG
	);
	VideoManager.listDataAC.field_0x0[1] = SCE_GIF_PACKED_AD;

	// GS TEST
	VideoManager.listDataAC.field_0x0[2] = SCE_GS_SET_TEST(
		1,	// ATE 
		1,	// ATST
		0,	// AREF
		0,	// AFAIL
		0,	// DATE
		0,	// DATM
		1,	// ZTE
		1	// ZTST
	);
	VideoManager.listDataAC.field_0x0[3] = SCE_GS_TEST_1;

	// ALPHA
	VideoManager.listDataAC.field_0x0[4] = SCE_GS_SET_ALPHA(SCE_GS_ALPHA_CS, SCE_GS_ALPHA_CD, SCE_GS_ALPHA_CS, SCE_GS_ALPHA_CD, 0);
	VideoManager.listDataAC.field_0x0[5] = SCE_GS_ALPHA_1;

	// PABE
	VideoManager.listDataAC.field_0x0[6] = SCE_GS_SET_PABE(0);
	VideoManager.listDataAC.field_0x0[7] = SCE_GS_PABE;

	// FBA
	VideoManager.listDataAC.field_0x0[8] = SCE_GS_SET_FBA(SCE_GS_FALSE);
	VideoManager.listDataAC.field_0x0[9] = SCE_GS_FBA_1;

	// PRIM
	VideoManager.listDataAC.field_0x0[10] = SCE_GS_SET_PRIM(
		SCE_GS_PRIM_SPRITE,		// PRIM (Primitive type)
		SCE_GS_TRUE,			// IIP  (Gouraud)
		0,						// TME  (Textured)
		0,						// FGE  (Fogging)
		SCE_GS_TRUE,			// ABE  (Alpha Blending)
		0,						// AA1  (Anti-Aliasing)
		0,						// FST  (Use ST for texture coords)
		0,						// CTXT (Context)
		0						// FIX  (Fragment control)
	);
	VideoManager.listDataAC.field_0x0[11] = SCE_GS_PRIM;

	// RGBAQ
	VideoManager.listDataAC.field_0x0[12] =
		(ulong)(int7)((uint7)VideoManager.fadeAlpha << 0x18) | 0x3f80000000000000;
	VideoManager.listDataAC.field_0x0[13] = SCE_GS_RGBAQ;

	// XYZF2
	VideoManager.listDataAC.field_0x0[14] = SCE_GS_SET_XYZF(iVar2 & 0xffffffffU, iVar1 & 0xffffffffU, 0, 0);
	VideoManager.listDataAC.field_0x0[15] = SCE_GS_XYZF2;

	// XYZF2
	VideoManager.listDataAC.field_0x0[16] = SCE_GS_SET_XYZF(
		(int)(iVar2 + (uint)VideoManager.screenWidth * 0x10) & 0xffffffffU,
		(int)(iVar1 + (uint)VideoManager.screenHeight * 0x10) & 0xffffffffU,
		0, 0);
	VideoManager.listDataAC.field_0x0[17] = SCE_GS_XYZF2;

	// TEST
	VideoManager.listDataAC.field_0x0[18] = SCE_GS_SET_TEST(
		0,	// ATE 
		0,	// ATST
		0,	// AREF
		0,	// AFAIL
		0,	// DATE
		0,	// DATM
		1,	// ZTE
		3	// ZTST
	);
	VideoManager.listDataAC.field_0x0[19] = SCE_GS_TEST_1;

	VideoManager.listDataAC.field_0xa0 = (ulong*)((char*)&VideoManager + 0xf0);
	VideoManager.listDataAC.field_0xa4 = (ulong*)((char*)&VideoManager + 0x100);
	VideoManager.listDataAC.field_0xa8 = (ulong*)((char*)&VideoManager + 0x110);
	return;
}

void edVideoSetAttribute(ed_video_attr* pAttr)
{
	int canInterlace;
	int iVar3;
	ulong uVar1;
	int iVar2;
	bool bShouldInterlace;

	if (pAttr == (ed_video_attr*)0x0) {
		pAttr = &VideoManager;
	}

	VideoManager.screenWidth = pAttr->screenWidth;
	VideoManager.screenHeight = pAttr->screenHeight;
	VideoManager.vidMode1 = pAttr->vidMode1;
	VideoManager.omode = pAttr->omode;
	VideoManager.isNTSC = pAttr->isNTSC;
	VideoManager.inter = pAttr->inter;
	VideoManager.field_0xb = pAttr->field_0xb;
	VideoManager.field_0xc = pAttr->field_0xc;
	VideoManager.bVSyncForever = pAttr->bVSyncForever;
	VideoManager.maxVblank_0xe = pAttr->maxVblank_0xe;
	VideoManager.field_0xf = pAttr->field_0xf;
	VideoManager.field_0x10[0] = pAttr->field_0x10[0];
	VideoManager.field_0x10[1] = pAttr->field_0x10[1];
	VideoManager.field_0x10[2] = pAttr->field_0x10[2];
	VideoManager.field_0x10[3] = pAttr->field_0x10[3];
	VideoManager.field_0x14 = pAttr->field_0x14;
	VideoManager.field_0x18 = pAttr->field_0x18;
	VideoManager.field_0x19 = pAttr->field_0x19;
#ifdef PLATFORM_PS2
	sceGsSyncV(0);
#endif
	edVideoClearVram(0, 0, 0, 0);
	canInterlace = _HighResMode();
	bShouldInterlace = canInterlace != 0;
	if (bShouldInterlace) {
		bShouldInterlace = pAttr->inter != SCE_GS_NOINTERLACE;
	}
	VideoManager.params18.ffmode = bShouldInterlace ^ 1;
	if (pAttr->field_0x10[0] != '\0') {
#ifdef PLATFORM_PS2
		sceGsResetGraph(0, (ushort)pAttr->inter, VideoManager.omode, (ushort)VideoManager.params18.ffmode);
#endif
	}
#ifdef PLATFORM_PS2
	uVar1 = sceGsGetIMR();
	sceGsPutIMR(uVar1 & 0xfffffffffffffeff);
	uVar1 = sceGsGetIMR();
	sceGsPutIMR(uVar1 & 0xfffffffffffffdff);
#endif
	VideoManager.params18.unused_0xc = 0;
	VideoManager.params18.gs_pmode = edVideoConfiguration.field_0x1 | 0x8066;
	VideoManager.params18.gs_smode2 = (ushort)pAttr->inter | (ushort)VideoManager.params18.ffmode << 1;
	VideoManager.params18.DX = 0;
	VideoManager.params18.DH = (pAttr->screenHeight << (VideoManager.params18.ffmode & 0x1f)) + -1;
	VideoManager.params18.DY = 0;
	VideoManager.params18.MAGH = (short)((pAttr->screenWidth + 0x9ff) / (uint)pAttr->screenWidth) + -1;
	_UpdateVideoInfo();
	_SetVideoEnv();
#ifdef PLATFORM_PS2
	iVar2 = 0;
	do {
		iVar3 = sceGsSyncV(0);
		iVar2 = iVar2 + 1;
	} while (iVar2 < 0x14);
#endif
	return;
}

edVideoConfig* edVideoGetConfig(void)
{
	return &edVideoConfiguration;
}

#ifdef PLATFORM_PS2
void _PutFakeDisplayEnv(void)
{
	edVideoWaitVsync(0);
	/* Setup CRT Controller */
	DPUT_GS_PMODE((ulong)(ushort)VideoManager.params18.gs_pmode);
	/* CRTC Video Settings */
	DPUT_GS_SMODE2((ulong)(ushort)VideoManager.params18.gs_smode2);
	/* RC2 data source settings */
	DPUT_GS_DISPFB2(0x1400);
	/* RC2 display output settings */
	DPUT_GS_DISPLAY2(*(u_long*)&VideoManager.disp2);
	/* Set CRTC background color */
	DPUT_GS_BGCOLOR(0);
	return;
}

void _edVideoSyncReset(void)
{
	VideoManager.vblankCount = 0;
	VideoManager.bWaitingForVSync = 0;
	return;
}

int _edVideoGs(int cause)
{
	void* pvVar1;
	int eventID;
	uint uVar2;
	ulong flag;
	ulong uVar4;
	undefined8 uVar3;

	flag = isceGsPutIMR(0xff00);
	uVar2 = DGET_GS_CSR();
	uVar3 = DGET_GS_SIGLBLID();
	uVar4 = DGET_GS_SIGLBLID();
	pvVar1 = (void*)((ulong)uVar3 >> 0x20);
	eventID = (int)uVar4;
	if ((uVar2 & 1) != 0) {
		DPUT_GS_CSR((ulong)(int)(uVar2 & 0xfffffcf0) & 0xffffffffU | 1);
		__asm__ volatile(" sync.l ");
		if ((((eventID == 8) || (eventID == 5)) || (eventID == 4)) || (eventID == 1)) {
			edSysHandlersCall(edVideoHandlers.mainIdentifier, edVideoHandlers.entries, edVideoHandlers.maxEventID, eventID, pvVar1);
		}
	}

	if ((uVar2 & 2) != 0) {
		DPUT_GS_CSR((ulong)(int)(uVar2 & 0xfffffcf0) & 0xffffffffU | 2);
		__asm__ volatile(" sync.l ");
		edSysHandlersCall(edVideoHandlers.mainIdentifier, edVideoHandlers.entries, edVideoHandlers.maxEventID, 2, pvVar1);
	}

	isceGsPutIMR(flag);
	ExitHandler();
	return 0;
}


int _edVideoSync(int cause)
{
	ulong uVar1;

	edSysHandlersCall(edVideoHandlers.mainIdentifier, edVideoHandlers.entries, edVideoHandlers.maxEventID, ED_HANDLER_VIDEO_SYNC, (void*)0x0);

	if (VideoManager.bVSyncForever == 0) {
		if (VideoManager.vblankCount == 0) {
			VideoManager.lastCount = _VideoTimerGet();
		}
		VideoManager.vblankCount = VideoManager.vblankCount + 1;
	}
	else {
		VideoManager.vblankCount = VideoManager.vblankCount + 1;
		if (VideoManager.maxVblank_0xe <= VideoManager.vblankCount) {
			if ((((VideoManager.field_0x52 == 0) || (VideoManager.inter == SCE_GS_NOINTERLACE)) ||
				(VideoManager.params18.ffmode == SCE_GS_FIELD)) ||
				(VideoManager.pFrameBuffer == (edSurface*)0x0)) {
				_edVideoSyncReset();
			}
			else {
				uVar1 = DGET_GS_CSR();
				if ((uint)((uVar1 & 0x2000) >> 0xd) ==
					(uint)(VideoManager.pFrameBuffer)->pSurfaceDesc->pLink_0xc->pSurfaceDispEnv->csrValue_0x10)
				{
					_edVideoSyncReset();
				}
			}
		}
	}
	ExitHandler();
	return 1;
}

char* g_szInterruptError_00433440 = "ERROR: Adding Interrupt Handler! ";

#define HANDLER_NEXT 0
#define HANDLER_LAST -1

void _edVideoVSyncCallback(int (*handler)(int))
{
	int iVar1;

	iVar1 = AddIntcHandler(INTC_VBLANK_S, handler, HANDLER_LAST);
	if (iVar1 < 0) {
		edDebugPrintf(g_szInterruptError_00433440);
	}
	else {
		EnableIntc(INTC_VBLANK_S);
	}
	return;
}

void _edVideoGSCallback(int (*handler)(int))
{
	int iVar1;
	ulong uVar2;

	iVar1 = AddIntcHandler(INTC_GS, handler, HANDLER_NEXT);
	if (iVar1 < 0) {
		edDebugPrintf(g_szInterruptError_00433440);
	}
	else {
		DPUT_GS_IMR(*GS_IMR & 0xfffffffffffffeff);
		DPUT_GS_IMR(*GS_IMR & 0xfffffffffffffdff);
		EnableIntc(INTC_GS);
	}
	return;
}
#endif

void edVideoInit(void)
{
	int iVar1;
	ed_video_attr local_20;

	local_20.screenWidth = 0x200;
	local_20.screenHeight = 0x200;
	local_20.vidMode1 = 0;
	local_20.omode = SCE_GS_PAL;
	local_20.isNTSC = 1;
	local_20.inter = SCE_GS_INTERLACE;
	local_20.field_0xb = 1;
	local_20.field_0xc = 1;
	local_20.bVSyncForever = 1;
	local_20.maxVblank_0xe = 1;
	local_20.field_0xf = 1;
	local_20.field_0x10[0] = '\x01';
	memset(&VideoManager, 0, sizeof(ed_video_attr));
	local_20.screenWidth = 0x280;
	local_20.screenHeight = 0x100;
	local_20.vidMode1 = 0;
	local_20.omode = SCE_GS_PAL;
	local_20.isNTSC = 1;
	local_20.inter = SCE_GS_INTERLACE;
	local_20.field_0xb = 1;
	local_20.field_0xc = 1;
	local_20.bVSyncForever = 1;
	local_20.maxVblank_0xe = 1;
	local_20.field_0x10[0] = edVideoConfiguration.field_0x4;
	edVideoSetAttribute(&local_20);
	VideoManager.vblankCount = 0;
	VideoManager.bWaitingForVSync = 0;
	VideoManager.bFadeActive = 0;

#ifdef PLATFORM_PS2
	_edVideoVSyncCallback(_edVideoSync);
	_edVideoGSCallback(_edVideoGs);
	sceGsSyncV(0);
#endif

	edVideoClearVram(0, 0, 0, 0x80);

#ifdef PLATFORM_PS2
	_PutFakeDisplayEnv();
	iVar1 = 0;
	do {
		sceGsSyncV(0);
		iVar1 = iVar1 + 1;
	} while (iVar1 < 0x14);
#endif

	_VideoUpdateSystemViewport((edSurface*)0x0);
	_FadeInit();
	return;
}
