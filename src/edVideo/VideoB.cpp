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
edVideoData g_ActiveVidParams_0048cd90 = { 0 };

void _VideoManagerAddFlippingSurface(edSurface* pNewFrameBuffer)
{
	pNewFrameBuffer->pSurfaceDesc->pLink_0xc = pNewFrameBuffer;
	g_ActiveVidParams_0048cd90.pFrameBuffer = pNewFrameBuffer;
	_VideoUpdateSystemViewport(pNewFrameBuffer);
	return;
}

void CleanupFunc_002b9b30(void)
{
	if (g_ActiveVidParams_0048cd90.pFrameBuffer != (edSurface*)0x0) {
		edSurfaceDel(g_ActiveVidParams_0048cd90.pFrameBuffer);
		g_ActiveVidParams_0048cd90.pFrameBuffer = (edSurface*)0x0;
	}
	edViewportDel(g_ActiveVidParams_0048cd90.pCamera, false);
	g_ActiveVidParams_0048cd90.pCamera = (ed_viewport*)0x0;
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

	iVar2 = (0x800 - ((int)(uint)g_ActiveVidParams_0048cd90.params26.screenWidth >> 1)) * 0x10;
	iVar1 = (0x800 - ((int)(uint)g_ActiveVidParams_0048cd90.params26.screenHeight >> 1)) * 0x10;
	g_ActiveVidParams_0048cd90.field_0x69 = 0;
	g_ActiveVidParams_0048cd90.field_0x6a = 0;
	g_ActiveVidParams_0048cd90.field_0x6b = 0;
	g_ActiveVidParams_0048cd90.field_0x80 = 0;

	// GIF TAG
	g_ActiveVidParams_0048cd90.listDataAC.field_0x0[0] = SCE_GIF_SET_TAG(
		9,				// NLOOP
		SCE_GS_TRUE,	// EOP
		SCE_GS_FALSE,	// PRE
		0,				// PRIM
		SCE_GIF_PACKED, // FLG
		1				// NREG
	);
	g_ActiveVidParams_0048cd90.listDataAC.field_0x0[1] = SCE_GIF_PACKED_AD;

	// GS TEST
	g_ActiveVidParams_0048cd90.listDataAC.field_0x0[2] = SCE_GS_SET_TEST(
		1,	// ATE 
		1,	// ATST
		0,	// AREF
		0,	// AFAIL
		0,	// DATE
		0,	// DATM
		1,	// ZTE
		1	// ZTST
	);
	g_ActiveVidParams_0048cd90.listDataAC.field_0x0[3] = SCE_GS_TEST_1;

	// ALPHA
	g_ActiveVidParams_0048cd90.listDataAC.field_0x0[4] = SCE_GS_SET_ALPHA(SCE_GS_ALPHA_CS, SCE_GS_ALPHA_CD, SCE_GS_ALPHA_CS, SCE_GS_ALPHA_CD, 0);
	g_ActiveVidParams_0048cd90.listDataAC.field_0x0[5] = SCE_GS_ALPHA_1;

	// PABE
	g_ActiveVidParams_0048cd90.listDataAC.field_0x0[6] = SCE_GS_SET_PABE(0);
	g_ActiveVidParams_0048cd90.listDataAC.field_0x0[7] = SCE_GS_PABE;

	// FBA
	g_ActiveVidParams_0048cd90.listDataAC.field_0x0[8] = SCE_GS_SET_FBA(SCE_GS_FALSE);
	g_ActiveVidParams_0048cd90.listDataAC.field_0x0[9] = SCE_GS_FBA_1;

	// PRIM
	g_ActiveVidParams_0048cd90.listDataAC.field_0x0[10] = SCE_GS_SET_PRIM(
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
	g_ActiveVidParams_0048cd90.listDataAC.field_0x0[11] = SCE_GS_PRIM;

	// RGBAQ
	g_ActiveVidParams_0048cd90.listDataAC.field_0x0[12] =
		(ulong)(int7)((uint7)g_ActiveVidParams_0048cd90.field_0x6c << 0x18) | 0x3f80000000000000;
	g_ActiveVidParams_0048cd90.listDataAC.field_0x0[13] = SCE_GS_RGBAQ;

	// XYZF2
	g_ActiveVidParams_0048cd90.listDataAC.field_0x0[14] = SCE_GS_SET_XYZF(iVar2 & 0xffffffffU, iVar1 & 0xffffffffU, 0, 0);
	g_ActiveVidParams_0048cd90.listDataAC.field_0x0[15] = SCE_GS_XYZF2;

	// XYZF2
	g_ActiveVidParams_0048cd90.listDataAC.field_0x0[16] = SCE_GS_SET_XYZF(
		(int)(iVar2 + (uint)g_ActiveVidParams_0048cd90.params26.screenWidth * 0x10) & 0xffffffffU,
		(int)(iVar1 + (uint)g_ActiveVidParams_0048cd90.params26.screenHeight * 0x10) & 0xffffffffU,
		0, 0);
	g_ActiveVidParams_0048cd90.listDataAC.field_0x0[17] = SCE_GS_XYZF2;

	// TEST
	g_ActiveVidParams_0048cd90.listDataAC.field_0x0[18] = SCE_GS_SET_TEST(
		0,	// ATE 
		0,	// ATST
		0,	// AREF
		0,	// AFAIL
		0,	// DATE
		0,	// DATM
		1,	// ZTE
		3	// ZTST
	);
	g_ActiveVidParams_0048cd90.listDataAC.field_0x0[19] = SCE_GS_TEST_1;

	g_ActiveVidParams_0048cd90.listDataAC.field_0xa0 = (ulong*)((char*)&g_ActiveVidParams_0048cd90 + 0xf0);
	g_ActiveVidParams_0048cd90.listDataAC.field_0xa4 = (ulong*)((char*)&g_ActiveVidParams_0048cd90 + 0x100);
	g_ActiveVidParams_0048cd90.listDataAC.field_0xa8 = (ulong*)((char*)&g_ActiveVidParams_0048cd90 + 0x110);
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
		pAttr = &g_ActiveVidParams_0048cd90.params26;
	}
	g_ActiveVidParams_0048cd90.params26 = *pAttr;
#ifdef PLATFORM_PS2
	sceGsSyncV(0);
#endif
	edVideoClearVram(0, 0, 0, 0);
	canInterlace = _HighResMode();
	bShouldInterlace = canInterlace != 0;
	if (bShouldInterlace) {
		bShouldInterlace = pAttr->inter != SCE_GS_NOINTERLACE;
	}
	g_ActiveVidParams_0048cd90.params18.ffmode = bShouldInterlace ^ 1;
	if (pAttr->field_0x10[0] != '\0') {
#ifdef PLATFORM_PS2
		sceGsResetGraph(0, (ushort)pAttr->inter, g_ActiveVidParams_0048cd90.params26.omode, (ushort)g_ActiveVidParams_0048cd90.params18.ffmode);
#endif
	}
#ifdef PLATFORM_PS2
	uVar1 = sceGsGetIMR();
	sceGsPutIMR(uVar1 & 0xfffffffffffffeff);
	uVar1 = sceGsGetIMR();
	sceGsPutIMR(uVar1 & 0xfffffffffffffdff);
#endif
	g_ActiveVidParams_0048cd90.params18.unused_0xc = 0;
	g_ActiveVidParams_0048cd90.params18.gs_pmode = edVideoConfiguration.field_0x1 | 0x8066;
	g_ActiveVidParams_0048cd90.params18.gs_smode2 = (ushort)pAttr->inter | (ushort)g_ActiveVidParams_0048cd90.params18.ffmode << 1;
	g_ActiveVidParams_0048cd90.params18.DX = 0;
	g_ActiveVidParams_0048cd90.params18.DH = (pAttr->screenHeight << (g_ActiveVidParams_0048cd90.params18.ffmode & 0x1f)) + -1;
	g_ActiveVidParams_0048cd90.params18.DY = 0;
	g_ActiveVidParams_0048cd90.params18.MAGH = (short)((pAttr->screenWidth + 0x9ff) / (uint)pAttr->screenWidth) + -1;
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
	DPUT_GS_PMODE((ulong)(ushort)g_ActiveVidParams_0048cd90.params18.gs_pmode);
	/* CRTC Video Settings */
	DPUT_GS_SMODE2((ulong)(ushort)g_ActiveVidParams_0048cd90.params18.gs_smode2);
	/* RC2 data source settings */
	DPUT_GS_DISPFB2(0x1400);
	/* RC2 display output settings */
	DPUT_GS_DISPLAY2(*(u_long*)&g_ActiveVidParams_0048cd90.disp2);
	/* Set CRTC background color */
	DPUT_GS_BGCOLOR(0);
	return;
}

void _edVideoSyncReset(void)
{
	g_ActiveVidParams_0048cd90.vblankCount = 0;
	g_ActiveVidParams_0048cd90.bWaitingForVSync = 0;
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
			edSysHandlersCall(edSysHandlerVideo_0048cee0.mainIdentifier, edSysHandlerVideo_0048cee0.entries,
				edSysHandlerVideo_0048cee0.maxEventID, eventID, pvVar1);
		}
	}
	if ((uVar2 & 2) != 0) {
		DPUT_GS_CSR((ulong)(int)(uVar2 & 0xfffffcf0) & 0xffffffffU | 2);
		__asm__ volatile(" sync.l ");
		edSysHandlersCall(edSysHandlerVideo_0048cee0.mainIdentifier, edSysHandlerVideo_0048cee0.entries,
			edSysHandlerVideo_0048cee0.maxEventID, 2, pvVar1);
	}
	isceGsPutIMR(flag);
	ExitHandler();
	return 0;
}


int _edVideoSync(int cause)
{
	ulong uVar1;

	edSysHandlersCall(edSysHandlerVideo_0048cee0.mainIdentifier, edSysHandlerVideo_0048cee0.entries,
		edSysHandlerVideo_0048cee0.maxEventID, 3, (void*)0x0);
	if (g_ActiveVidParams_0048cd90.params26.bVSyncForever == 0) {
		if (g_ActiveVidParams_0048cd90.vblankCount == 0) {
			g_ActiveVidParams_0048cd90.lastCount = _VideoTimerGet();
		}
		g_ActiveVidParams_0048cd90.vblankCount = g_ActiveVidParams_0048cd90.vblankCount + 1;
	}
	else {
		g_ActiveVidParams_0048cd90.vblankCount = g_ActiveVidParams_0048cd90.vblankCount + 1;
		if (g_ActiveVidParams_0048cd90.params26.maxVblank_0xe <= g_ActiveVidParams_0048cd90.vblankCount) {
			if ((((g_ActiveVidParams_0048cd90.field_0x52 == 0) || (g_ActiveVidParams_0048cd90.params26.inter == SCE_GS_NOINTERLACE)) ||
				(g_ActiveVidParams_0048cd90.params18.ffmode == SCE_GS_FIELD)) ||
				(g_ActiveVidParams_0048cd90.pFrameBuffer == (edSurface*)0x0)) {
				_edVideoSyncReset();
			}
			else {
				uVar1 = DGET_GS_CSR();
				if ((uint)((uVar1 & 0x2000) >> 0xd) ==
					(uint)(g_ActiveVidParams_0048cd90.pFrameBuffer)->pSurfaceDesc->pLink_0xc->pSurfaceDispEnv->csrValue_0x10)
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
	memset(&g_ActiveVidParams_0048cd90, 0, sizeof(edVideoData));
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
	g_ActiveVidParams_0048cd90.vblankCount = 0;
	g_ActiveVidParams_0048cd90.bWaitingForVSync = 0;
	g_ActiveVidParams_0048cd90.field_0x68 = 0;
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
