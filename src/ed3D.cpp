#include "ed3D.h"
#include "edMem.h"
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "edDlist.h"

#if defined(PLATFORM_PS2)
#include <eekernel.h>
#include <libdma.h>
#else
#include "port.h"
#endif

#include "edVideo/VideoD.h"
#include "port/pointer_conv.h"
#include "edVideo/VideoB.h"
#include "edVideo/VideoC.h"
#include "Rendering/DisplayList.h"
#include "Camera.h"

#include "Rendering/CustomShell.h"

char* s_ed3D_Initialsation_004333a0 = "ed3D Initialsation\n";

ed3D_Params::ed3D_Params()
	: meshHeaderCountB(0x3E0)
	, initialStaticMeshMasterCount(0xF)
	, field_0x8(0x10)
	, meshHeaderCountBAlt(0x2800)
	, meshHeaderCountA(0x1F4)
	, field_0x14(0xFA)
	, field_0x18(0x7D0)
	, meshHeaderCountAA(0xA)
	, meshHeaderCountBB(0x19)
	, bEnableProfile(0x0)
	, field_0x25(0x0)
	, field_0x26(0x64)
	, field_0x27(0x0)
	, field_0x28(0xC8)
	, field_0x2c(0x2)
	, field_0x2d(0x1)
	, field_0x2e(0x0)
	, field_0x2f(0x0)
	, meshTransformDataCount(0x3E8)
	, field_0x34(0x0)
{

}

namespace ed3D
{
	TextureInfoSmall* TexturePool_004491cc;
	int LoadedTextureCount_004491c4;
	
	ulong* LoadedTextureData_004491e0;
	ulong* LoadedTextureDataBase_004491d8;

	ed3D_Params Params_00424e30;

	FrameBuffer* PTR_VidModeData_30_004491f4 = NULL;

	StaticMeshMaster* cameraPanStaticMasterArray_00449378 = NULL;
	MeshTransformParentHeader* PTR_MeshTransformParentHeader_0044937c = NULL;

	ulong* g_RenderCommand_004492c0 = NULL;
	ulong* g_RenderCommand_004492b0 = NULL;

	int g_FrameBufferPtr_004491f0 = 0;

	CameraPanMasterHeader g_ModelDoubleBuffer_0048c6d0[2] = { 0 };
	CameraPanMasterHeader g_OtherDoubleBuffer_0048cd50[2] = { 0 };

	struct MeshTransformData;
	MeshTransformData* g_RenderMeshTransform_004493f4 = NULL;
	undefined* PTR_DAT_00449428 = NULL;

	MeshTransformData* PTR_MeshTransformData_ARRAY_0044942c = NULL;
	MeshTransformParentHeader* PTR_MeshTransformParentHeader_00449430 = NULL;
	MeshTransformParent* PTR_MeshTransformParent_ARRAY_00449434 = NULL;

	byte BYTE_00448a5c = 1;
	uint UINT_0048c2c0 = 0;
	uint UINT_0048c2c4 = 0;
	uint UINT_0048c2c8 = 0;
	uint UINT_0048c2cc = 0;
	Vector Vector_0048c2d0 = { 0 };
	Vector Vector_0048c2e0 = { 0 };
	Vector Vector_0048c2f0 = { 0 };

	byte BYTE_00448a70 = 1;
	byte BYTE_00449418 = 0;
	RenderCommand* g_RenderCommandBuffer_00449400 = NULL;

	uint g_StaticVideoList_00489590[100] = { 0 };

	void FUN_0029c740(void)
	{
		int iVar1;
		ulong uVar2;
		ulong uVar3;

		//uVar2 = 0;
		//do {
		//	iVar1 = (int)uVar2;
		//	uVar3 = uVar2 & 0xffffffff;
		//	uVar2 = SEXT48(iVar1 + 1);
		//	*(ulong*)(g_StaticVideoList_00489590 + iVar1 * 4 + 0x54) = uVar3 | 0x302e400000008000;
		//	*(undefined8*)(g_StaticVideoList_00489590 + iVar1 * 4 + 0x56) = 0x512;
		//	(&DAT_00489cf0)[iVar1 * 2] = uVar3 | 0x3026400000008000;
		//	(&DAT_00489cf8)[iVar1 * 2] = 0x512;
		//	(&DAT_0048a300)[iVar1 * 2] = uVar3 | 0x3022400000008000;
		//	(&DAT_0048a308)[iVar1 * 2] = 0x512;
		//} while (uVar2 < 0x61);
		//DAT_0048a910 = 0x302ec00000008000;
		//DAT_0048a920 = 0x3026c00000008000;
		//DAT_0048a918 = 0x512;
		//uVar2 = 0;
		//DAT_0048a930 = 0x3022c00000008000;
		//DAT_0048a928 = 0x512;
		//DAT_0048a938 = 0x512;
		//do {
		//	iVar1 = (int)uVar2;
		//	uVar3 = uVar2 & 0xffffffff;
		//	uVar2 = SEXT48(iVar1 + 1);
		//	(&DAT_0048a940)[iVar1 * 2] = uVar3 | 0x302d400000008000;
		//	(&DAT_0048a948)[iVar1 * 2] = 0x512;
		//	(&DAT_0048af50)[iVar1 * 2] = uVar3 | 0x3025400000008000;
		//	(&DAT_0048af58)[iVar1 * 2] = 0x512;
		//} while (uVar2 < 0x61);
		//uVar2 = 0;
		//do {
		//	iVar1 = (int)uVar2;
		//	uVar3 = uVar2 & 0xffffffff;
		//	uVar2 = SEXT48(iVar1 + 1);
		//	(&DAT_0048b560)[iVar1 * 2] = uVar3 | 0x3020400000008000;
		//	(&DAT_0048b568)[iVar1 * 2] = 0x512;
		//	(&DAT_0048bb70)[iVar1 * 2] = uVar3 | 0x3028400000008000;
		//	(&DAT_0048bb78)[iVar1 * 2] = 0x512;
		//} while (uVar2 < 0x61);
		g_StaticVideoList_00489590[7] = 0;
		g_StaticVideoList_00489590[11] = 0;
		g_StaticVideoList_00489590[0] = 0x01000101;
		g_StaticVideoList_00489590[4] = 0x01000101;
		g_StaticVideoList_00489590[1] = 0x20000000;
		g_StaticVideoList_00489590[5] = 0x20000000;
		g_StaticVideoList_00489590[8] = 0x01000103;
		g_StaticVideoList_00489590[2] = 0xffffff00;
		g_StaticVideoList_00489590[34] = 0xffffffc0;
		g_StaticVideoList_00489590[6] = 0xffffff00;
		g_StaticVideoList_00489590[9] = 0x20000000;
		g_StaticVideoList_00489590[10] = 0xffffff00;
		g_StaticVideoList_00489590[26] = 0xfffffff0;
		g_StaticVideoList_00489590[66] = 0xffffff40;
		g_StaticVideoList_00489590[74] = 0xbf800000;
		g_StaticVideoList_00489590[75] = 0xc000;
		//DAT_004492dc = 0x76;
		g_StaticVideoList_00489590[28] = 0x01000103;
		//DAT_004492e0 = 0x198;
		g_StaticVideoList_00489590[29] = 0x20000000;
		//DAT_004492e4 = 0x2ba;
		g_StaticVideoList_00489590[30] = 0xffffff00;
		//DAT_004492d8 = 1;
		g_StaticVideoList_00489590[31] = 0;
		//DAT_0048c348 = 0x200000003000077;
		//_DAT_0048c358 = 0x200000003000199;
		//DAT_0048c368 = 0x2000000030002bb;
		g_StaticVideoList_00489590[32] = 0x01000103;
		g_StaticVideoList_00489590[33] = 0x20000000;
		g_StaticVideoList_00489590[35] = 0;
		g_StaticVideoList_00489590[12] = 0x01000103;
		g_StaticVideoList_00489590[13] = 0x20000000;
		g_StaticVideoList_00489590[14] = 0xffffff00;
		g_StaticVideoList_00489590[15] = 0;
		g_StaticVideoList_00489590[16] = 0x01000103;
		g_StaticVideoList_00489590[17] = 0x20000000;
		g_StaticVideoList_00489590[18] = 0xffffff50;
		g_StaticVideoList_00489590[19] = 0x30000000;
		g_StaticVideoList_00489590[20] = 0;
		g_StaticVideoList_00489590[21] = 0;
		g_StaticVideoList_00489590[22] = 0x3f800000;
		g_StaticVideoList_00489590[23] = 0;
		g_StaticVideoList_00489590[24] = 0x01000103;
		g_StaticVideoList_00489590[25] = 0x20000000;
		g_StaticVideoList_00489590[27] = 0;
		g_StaticVideoList_00489590[36] = 0x01000101;
		g_StaticVideoList_00489590[37] = 0x20000000;
		g_StaticVideoList_00489590[38] = 0xffffff00;
		g_StaticVideoList_00489590[39] = 0;
		g_StaticVideoList_00489590[64] = 0x01000103;
		g_StaticVideoList_00489590[65] = 0x20000000;
		g_StaticVideoList_00489590[67] = 0;
		g_StaticVideoList_00489590[60] = 0x01000103;
		g_StaticVideoList_00489590[61] = 0x20000000;
		g_StaticVideoList_00489590[62] = 0xffffff00;
		g_StaticVideoList_00489590[63] = 0;
		g_StaticVideoList_00489590[68] = 0x01000101;
		g_StaticVideoList_00489590[69] = 0x20000000;
		g_StaticVideoList_00489590[70] = 0xffffff50;
		g_StaticVideoList_00489590[71] = 0x30000000;
		g_StaticVideoList_00489590[72] = 0;
		g_StaticVideoList_00489590[73] = 0;
		g_StaticVideoList_00489590[76] = 0x01000103;
		g_StaticVideoList_00489590[56] = 0x01000103;
		g_StaticVideoList_00489590[77] = 0x20000000;
		g_StaticVideoList_00489590[78] = 0xffffff50;
		g_StaticVideoList_00489590[50] = 0xffffff50;
		g_StaticVideoList_00489590[79] = 0x30000000;
		g_StaticVideoList_00489590[80] = 0;
		g_StaticVideoList_00489590[81] = 0;
		g_StaticVideoList_00489590[82] = 0x3f800000;
		g_StaticVideoList_00489590[54] = 0x3f800000;
		g_StaticVideoList_00489590[83] = 0;
		g_StaticVideoList_00489590[57] = 0x20000000;
		g_StaticVideoList_00489590[58] = 0xffffff00;
		g_StaticVideoList_00489590[59] = 0;
		g_StaticVideoList_00489590[44] = 0x01000101;
		g_StaticVideoList_00489590[45] = 0x20000000;
		g_StaticVideoList_00489590[46] = 0xffffff00;
		g_StaticVideoList_00489590[42] = 0xffffff00;
		g_StaticVideoList_00489590[47] = 0;
		g_StaticVideoList_00489590[40] = 0x01000101;
		g_StaticVideoList_00489590[48] = 0x01000101;
		g_StaticVideoList_00489590[41] = 0x20000000;
		g_StaticVideoList_00489590[49] = 0x20000000;
		g_StaticVideoList_00489590[43] = 0;
		g_StaticVideoList_00489590[51] = 0x30000000;
		g_StaticVideoList_00489590[52] = 0;
		g_StaticVideoList_00489590[53] = 0;
		g_StaticVideoList_00489590[55] = 0;
		//DAT_0048c340 = 0x30000000;
		//_DAT_0048c350 = 0x30000000;
		//DAT_0048c360 = 0x30000000;
		//FUN_002ab6f0();
		return;
	}

	void InitVidMode_002936b0(void)
	{
		int iVar1;
		uint uVar2;
		int iVar3;
		VidModeData local_10;

		local_10.screenWidth = 0x200;
		local_10.screenHeight = 0x200;
		local_10.field_0x6 = 2;
		local_10.frameBufferCount = 1;
		local_10.flags_0x8 = 2;
		local_10.pixelStoreMode = SCE_GS_PSMCT32;
		local_10.bUseGlobalFrameBuffer = false;
		PTR_VidModeData_30_004491f4 = AllocateFrameBuffers_002bbb90(&local_10);
		uVar2 = PTR_VidModeData_30_004491f4->frameBasePtr;
		iVar3 = 0;
		//DAT_00489430 = uVar2;
		//do {
		//	(&DAT_00449310)[iVar3] = uVar2;
		//	(&DAT_00449318)[iVar3] = uVar2 + 0x380;
		//	iVar1 = FUN_0029e110(0);
		//	iVar1 = FUN_0029e1b0(iVar1);
		//	(&DAT_00449320)[iVar3] = iVar1 << 0x10;
		//	iVar1 = FUN_0029e110(0);
		//	iVar1 = FUN_0029e1b0(iVar1);
		//	(&DAT_00449328)[iVar3] = iVar1 << 0xe;
		//	iVar1 = iVar3 + 1;
		//	uVar2 = uVar2 + 0x400;
		//	(&DAT_00489434)[iVar3] = (&DAT_00449310)[iVar3];
		//	(&DAT_00489444)[iVar3] = (&DAT_00449318)[iVar3];
		//	(&DAT_0048943c)[iVar3] = (&DAT_00449320)[iVar3];
		//	(&DAT_0048944c)[iVar3] = (&DAT_00449328)[iVar3];
		//	iVar3 = iVar1;
		//} while (iVar1 < 2);
		g_FrameBufferPtr_004491f0 = uVar2;
		return;
	}

	int g_CameraFlipIndex_004494a4 = 0;
	RenderCommand* g_VideoMemoryPtr_00449280 = NULL;
	RenderCommand* g_VideoMemoryBasePtr_00449260 = NULL;
	RenderCommand* g_FlipDMA_00449270 = NULL;
	uint g_VideoMemorySize_00449280 = 0;
	uint g_Model_PCR_00449240 = 0;
	byte g_WaitAfterVIF_00449244 = 0;
	int g_ModelDoubleBufferIndex_004494a0 = 0;

#ifdef PLATFORM_PS2
	int g_Model_DMAC_Ctrl_mode_00448984 = D_CTRL_RELE_O;
#endif

	uint g_VIF1_FIFO_00424fe0[4] = { 0x6008000, 0x0, 0x0, 0x0 };

	struct RenderFrame_30 {
		char* pFileData;
		struct MeshTransformSpecial* field_0x4;
		struct MeshTransformSpecial internalMeshTransformSpecial;
		undefined4 field_0x18;
		int index_0x1c;
		struct Matrix* pTransformB;
		struct MeshTransformData* pTransformA;
		int flags_0x28;
		char* field_0x2c;
	};

	byte BYTE_00449340 = 0;
	int INT_00449334 = 0;
	MeshTransformParent* g_TransformParent_00449304 = NULL;
	char* PTR_DAT_00449338 = NULL;
	CameraPanMasterHeader* g_ActiveCameraPanMasterHeader_00449308 = NULL;

	void SendModelGIFCommands_0029d010(CameraPanMasterHeader* param_1)
	{
		char* pcVar1;
		int iVar2;
		ulong* puVar3;
		int iVar4;
		RenderFrame_30* pMVar1;

		if (BYTE_00449340 == 0x3c) {
			BYTE_00449340 = 0;
		}
		INT_00449334 = 0;
		BYTE_00449340 = BYTE_00449340 + 1;
		g_TransformParent_00449304 = (MeshTransformParent*)param_1->pLoadedData;
		do {
			pMVar1 = (RenderFrame_30*)g_TransformParent_00449304->pMeshTransformData;
			if (pMVar1 != (RenderFrame_30*)0x0) {
				PTR_DAT_00449338 = pMVar1->pFileData;
			}
			g_TransformParent_00449304 = g_TransformParent_00449304->pNext;
		} while ((&pMVar1->pFileData)[0xb] == (char*)0xdfdfdfdf);
		g_ActiveCameraPanMasterHeader_00449308 = param_1;
		if (((uint)(&pMVar1->pFileData)[10] & 2) != 0) {
			assert(false);
			//FUN_002ab4b0();
			//(&pMVar1->pFileData)[10] = (char*)((uint)(&pMVar1->pFileData)[10] & 0xfffffffd);
		}
		if ((byte)*pMVar1->pFileData < 2) {
			pcVar1 = (&pMVar1->pFileData)[0xb];
			if ((pcVar1 != (char*)0x0) && (*(short*)(pcVar1 + 4) != 0)) {
				iVar2 = *(int*)(pcVar1 + 8);
				iVar4 = INT_00449334 * 8;
				WaitDMA();
				shellDmaStartChainB(SHELLDMA_CHANNEL_GIF, *(ulonglong**)(iVar2 + iVar4));
			}
		}
		else {
			assert(false);
			//puVar3 = FUN_0029d150(pMVar1, g_ed3D_CommandBuffer_00449300, 1, INT_00449334);
			//if (puVar3 != (ulong*)0x0) {
			//	WaitDMA();
			//	shellDmaStartChainB(SHELLDMA_CHANNEL_GIF, (sceDmaTag*)g_ed3D_CommandBuffer_00449300);
			//}
		}
		INT_00449334 = INT_00449334 + 1;
		return;
	}

	void SendModelGIFCommands_0029cf90(CameraPanMasterHeader* pCommandInfo)
	{
		ulong* puVar1;

		SendModelGIFCommands_0029d010(pCommandInfo);
		puVar1 = g_RenderCommand_004492c0;
		DMA_Registers[2].QWC = 0xc0;
		if (g_RenderCommand_004492b0 != g_RenderCommand_004492c0) {
			*g_RenderCommand_004492c0 = 0x70000000;
			puVar1[1] = 0;
			WaitForDraw_00258230();
			WaitForDraw_00258230();
#ifdef PLATFORM_IS2
			DPUT_VIF1_FIFO(*(u_long128*)&g_VIF1_FIFO_00424fe0[0]);
#endif
			WaitDMA();
			shellDmaStartChainB(SHELLDMA_CHANNEL_GIF, (ulonglong*)g_RenderCommand_004492b0);
		}
		//FUN_002ab4b0();
		return;
	}

	void SendModelVIFCommands_0029bb10(void)
	{
#ifdef PLATFORM_PS2
		DPUT_GIF_MODE(4);
		DPUT_D_CTRL(g_Model_DMAC_Ctrl_mode_00448984);
		DPUT_D_STAT(D_STAT_CIS0_O);
#endif
		edSysHandlersCall(edSysHandlerVideo_0048cee0.mainIdentifier, edSysHandlerVideo_0048cee0.entries,
			edSysHandlerVideo_0048cee0.maxEventID, 1, (void*)0x0);
		g_ModelDoubleBufferIndex_004494a0 = g_CameraFlipIndex_004494a4;
		g_CameraFlipIndex_004494a4 = g_CameraFlipIndex_004494a4 != 0 ^ 1;
		edSysHandlersCall(edDlist::sysHandler_0048cb90.mainIdentifier, edDlist::sysHandler_0048cb90.entries,
			edDlist::sysHandler_0048cb90.maxEventID, 1, (void*)0x0);
		if ((CameraPanMasterHeader*)g_ModelDoubleBuffer_0048c6d0[g_ModelDoubleBufferIndex_004494a0].pLoadedData !=
			g_ModelDoubleBuffer_0048c6d0 + g_ModelDoubleBufferIndex_004494a0) {
			SendModelGIFCommands_0029cf90(g_ModelDoubleBuffer_0048c6d0 + g_ModelDoubleBufferIndex_004494a0);
		}
#ifdef PLATFORM_PS2
		DPUT_D_PCR(g_Model_PCR_00449240);
#endif
		if ((g_FlipDMA_00449270 != (RenderCommand*)0x0) && (g_FlipDMA_00449270 != g_VideoMemoryPtr_00449280)) {
			WaitDMA();
			shellDmaStartChainB(SHELLDMA_CHANNEL_VIF1, (ulonglong*)g_FlipDMA_00449270);
			if (g_WaitAfterVIF_00449244 != '\0') {
				WaitForDraw_00258230();
				WaitForDraw_00258230();
			}
		}
		return;
	}

	struct ed3D_Allocator {
		char* base;
		char* end;
		char* current;
	};

	ed3D_Allocator ed3D_Allocator_00449248 = { 0 };

	void FUN_0029c220(uint param_1, int param_2)
	{
		char* pcVar1;
		char* pcVar2;
		ulong lVar3;

		UINT_0048c2c0 = 0x22;
		UINT_0048c2cc = 0xc000;
		lVar3 = (ulong)ed3D_Allocator_00449248.end;
		UINT_0048c2c4 = 0x3e;
		UINT_0048c2c8 = 0x5a;
		pcVar2 = ed3D_Allocator_00449248.end + param_1 * 0x10;
		if (ed3D_Allocator_00449248.current < ed3D_Allocator_00449248.end + param_1 * 0x10) {
			lVar3 = 0;
			pcVar2 = ed3D_Allocator_00449248.end;
		}
		ed3D_Allocator_00449248.end = pcVar2;
#if PLATFORM_PS2
		g_VideoMemoryBasePtr_00449260 = (RenderCommand*)((ulong)((ulong)(lVar3 << 0x24) >> 0x24) | 0x30000000);
#else
		g_VideoMemoryBasePtr_00449260 = (RenderCommand*)lVar3;
#endif
		//DAT_00449290 = g_VideoMemoryBasePtr_00449260 + (param_1 >> 1);
		g_VideoMemorySize_00449280 = param_1;
		g_VideoMemoryPtr_00449280 = g_VideoMemoryBasePtr_00449260;
		FUN_0029c740();
		//DAT_0048c2b0 = 0;
		//DAT_0048c2b4 = 0;
		//DAT_0048c2b8 = 0;
		//FUN_0029c4f0();
		//FUN_0029c410();
		//DAT_0048c278 = 0;
		//DAT_0048c288 = 0;
		//DAT_0048c298 = 0;
		//DAT_0048c270 = 0x48964030000001;
		//DAT_0048c280 = 0x48965030000002;
		//DAT_0048c290 = 0x48963030000001;
		pcVar2 = ed3D_Allocator_00449248.end;
		pcVar1 = ed3D_Allocator_00449248.end + 0x140;
		if (ed3D_Allocator_00449248.current < ed3D_Allocator_00449248.end + 0x140) {
			pcVar2 = (char*)0x0;
			pcVar1 = ed3D_Allocator_00449248.end;
		}
		ed3D_Allocator_00449248.end = pcVar1;
		//PTR_DAT_00449300 = (undefined*)((uint)pcVar2 & 0xdfffffff);
		//FUN_002a93b0();
		lVar3 = (long)(int)ed3D_Allocator_00449248.end;
		pcVar2 = ed3D_Allocator_00449248.end + param_2 * 0x20;
		if (ed3D_Allocator_00449248.current < ed3D_Allocator_00449248.end + param_2 * 0x20) {
			lVar3 = 0;
			pcVar2 = ed3D_Allocator_00449248.end;
		}
		ed3D_Allocator_00449248.end = pcVar2;
		//DAT_004492a0 = (uint)((ulong)(lVar3 << 0x24) >> 0x24) | 0x30000000;
		//DAT_004492c0 = DAT_004492a0;
		//DAT_004492d0 = DAT_004492a0 + param_2 * 0x10;
		return;
	}

	void ResetMemoryPtr_0029c060(void)
	{
		uint uVar1;
		int iVar2;

		//DAT_00449330 = 0;
		//PTR_DAT_0044933c = (undefined*)0x0;
		//g_RenderMeshTransform_004493f4 = (MeshTransformData*)0x0;
		//DAT_004493f8 = 0;
		if (g_CameraFlipIndex_004494a4 == 0) {
			iVar2 = Params_00424e30.field_0x28 * 0x10;
			//DAT_004492c0 = DAT_004492a0;
			g_VideoMemoryPtr_00449280 = g_VideoMemoryBasePtr_00449260;
			uVar1 = g_VideoMemorySize_00449280 >> 1;
		}
		else {
			g_VideoMemoryPtr_00449280 = g_VideoMemoryBasePtr_00449260 + (g_VideoMemorySize_00449280 >> 1);
			iVar2 = Params_00424e30.field_0x28 * 0x20;
			//DAT_004492c0 = DAT_004492a0 + Params_00424e30.field_0x28 * 0x10;
			uVar1 = g_VideoMemorySize_00449280;
		}
		//DAT_004492d0 = DAT_004492a0 + iVar2;
		//DAT_00449290 = g_VideoMemoryBasePtr_00449260 + uVar1;
		g_RenderCommandBuffer_00449400 = (RenderCommand*)0x0;
		//DAT_004492d4 = 1;
		//DAT_004492b0 = DAT_004492c0;
		//FUN_002a9350();
		//PTR_DAT_004491bc = PTR_DAT_004491b8;
		g_FlipDMA_00449270 = g_VideoMemoryPtr_00449280;
		return;
	}

	int g_RenderPassCount_0044939c = 0;

	void EdSoundHandler_002b2730(int param_1, int param_2, char* param_3)
	{
		int iVar1;
		uint uVar2;
		//ProfileObject* pPVar3;
		//
		//iVar1 = g_3DRendProfileObjIndex_004491e8;
		g_RenderPassCount_0044939c = 0;
		//if (ed3D::Params_00424e30.bEnableProfile != 0) {
		//	if ((g_3DRendProfileObjIndex_004491e8 & 1U) == 0) {
		//		uVar2 = (uint)g_3DRendProfileObjIndex_004491e8 >> 4;
		//		if (g_ProfileObjB_0041ed44[uVar2].field_0x34 != 0) {
		//			g_ProfileObjB_0041ed44[uVar2].field_0x8 = 0;
		//			g_ProfileObjB_0041ed44[uVar2].field_0x20 = 0;
		//			g_ProfileObjB_0041ed44[uVar2].field_0x34 = 0;
		//		}
		//		g_ProfileObjB_0041ed44[(uint)iVar1 >> 4].field_0x0 = (long)PCR1 & 0xffffffff;
		//		g_ProfileObjB_0041ed44[(uint)iVar1 >> 4].field_0x20 = g_ProfileObjB_0041ed44[(uint)iVar1 >> 4].field_0x20 + 1;
		//	}
		//	else {
		//		uVar2 = (uint)g_3DRendProfileObjIndex_004491e8 >> 4;
		//		if (g_ProfileObjA_0041ed40[uVar2].field_0x34 != 0) {
		//			g_ProfileObjA_0041ed40[uVar2].field_0x8 = 0;
		//			g_ProfileObjA_0041ed40[uVar2].field_0x20 = 0;
		//			g_ProfileObjA_0041ed40[uVar2].field_0x34 = 0;
		//		}
		//		g_ProfileObjA_0041ed40[(uint)iVar1 >> 4].field_0x0 = (long)PCR1 & 0xffffffff;
		//		g_ProfileObjA_0041ed40[(uint)iVar1 >> 4].field_0x20 = g_ProfileObjA_0041ed40[(uint)iVar1 >> 4].field_0x20 + 1;
		//	}
		//}
		SendModelVIFCommands_0029bb10();
		//DAT_00449398 = 0;
		//FUN_002ad840();
		//if ((ed3D::Params_00424e30.field_0x2c & 2) != 0) {
		//	UpdateMeshTransforms_002b2a40();
		//}
		//g_RenderActiveCameraObj_006f7d80 = (CameraObj_28*)0x0;
		//FUN_0029c6f0();
		//FUN_0029ced0();
		ResetMemoryPtr_0029c060();
		//FUN_002b1140();
		//RenderSetupFunc_002b1220();
		//FUN_002b6b20();
		//if (ed3D::Params_00424e30.bEnableProfile != 0) {
		//	uVar2 = (uint)g_3DRendProfileObjIndex_004491e8 >> 4;
		//	if ((g_3DRendProfileObjIndex_004491e8 & 1U) == 0) {
		//		g_ProfileObjB_0041ed44[uVar2].field_0x0 = (long)PCR1 - g_ProfileObjB_0041ed44[uVar2].field_0x0;
		//		g_ProfileObjB_0041ed44[uVar2].field_0x8 =
		//			g_ProfileObjB_0041ed44[uVar2].field_0x8 + g_ProfileObjB_0041ed44[uVar2].field_0x0;
		//		pPVar3 = g_ProfileObjB_0041ed44 + uVar2;
		//		if (pPVar3->field_0x10 < g_ProfileObjB_0041ed44[uVar2].field_0x0) {
		//			pPVar3->field_0x10 = pPVar3->field_0x0;
		//		}
		//	}
		//	else {
		//		g_ProfileObjA_0041ed40[uVar2].field_0x0 = (long)PCR1 - g_ProfileObjA_0041ed40[uVar2].field_0x0;
		//		g_ProfileObjA_0041ed40[uVar2].field_0x8 =
		//			g_ProfileObjA_0041ed40[uVar2].field_0x8 + g_ProfileObjA_0041ed40[uVar2].field_0x0;
		//		pPVar3 = g_ProfileObjA_0041ed40 + uVar2;
		//		if (pPVar3->field_0x10 < g_ProfileObjA_0041ed40[uVar2].field_0x0) {
		//			pPVar3->field_0x10 = pPVar3->field_0x0;
		//		}
		//	}
		//}
		return;
	}

	CameraObj_28* g_CameraObj_28_00449384 = NULL;
	void* g_StaticMeshMasterHeap_00449438 = NULL;

	CameraPanMasterHeader* AllocateCameraPanStatic_002abc80(void)
	{
		CameraPanMasterHeader* pCameraPanStatic;

		pCameraPanStatic = AllocateCameraAndMesh_00290a10(1, 0, (EHeap)g_StaticMeshMasterHeap_00449438);
		Setup_00290bf0(pCameraPanStatic, PTR_MeshTransformParentHeader_00449430, 0, 0);
		return pCameraPanStatic;
	}

	StaticMeshMaster* SetupCameraPanStaticMaster_002b4600(CameraObj_130* param_1, CameraObj_28* param_2, long mode)
	{
		ushort uVar1;
		FrameBuffer* pVVar2;
		StaticMeshMaster* pSVar3;
		CameraPanMasterHeader* pCVar4;
		StaticMeshMaster* pSVar5;
		uint staticMeshMasterIndex;

		staticMeshMasterIndex = 0;
		/* Find inactive camera master. */
		while ((staticMeshMasterIndex < (uint)ed3D::Params_00424e30.initialStaticMeshMasterCount &&
			((ed3D::cameraPanStaticMasterArray_00449378[staticMeshMasterIndex].flags_0x4 & 1) != 0))) {
			staticMeshMasterIndex = staticMeshMasterIndex + 1;
		}
		pSVar3 = (StaticMeshMaster*)0xffffffff;
		if (staticMeshMasterIndex < (uint)ed3D::Params_00424e30.initialStaticMeshMasterCount) {
			pSVar3 = ed3D::cameraPanStaticMasterArray_00449378 + staticMeshMasterIndex;
			pSVar3->flags_0x4 = pSVar3->flags_0x4 | 1;
			ed3D::cameraPanStaticMasterArray_00449378[staticMeshMasterIndex].field_0x0 = 0;
			ed3D::cameraPanStaticMasterArray_00449378[staticMeshMasterIndex].pCameraObj130_0x8 = param_1;
			ed3D::cameraPanStaticMasterArray_00449378[staticMeshMasterIndex].pCameraObj28_0xc = param_2;
			if (g_CameraObj_28_00449384 == (CameraObj_28*)0x0) {
				g_CameraObj_28_00449384 = param_2;
			}
			//FUN_0029e500(&ed3D::cameraPanStaticMasterArray_00449378[staticMeshMasterIndex].field_0x50);
			pSVar5 = ed3D::cameraPanStaticMasterArray_00449378 + staticMeshMasterIndex;
			pVVar2 = pSVar5->pCameraObj28_0xc->pZBuffer;
			if (pVVar2 == (FrameBuffer*)0x0) {
				assert(false);
				//(pSVar5->field_0x50).field_0x14 = &DAT_00ffffef;
			}
			else {
				uVar1 = pVVar2->pVidModeData_0x0->pixelStoreMode;
				if (uVar1 == 2) {
					(pSVar5->field_0x50).field_0x14 = 0xfffe;
				}
				else {
					if (uVar1 == 1) {
						assert(false);
						//(pSVar5->field_0x50).field_0x14 = &DAT_00ffffef;
					}
					else {
						if (uVar1 == 0) {
							(pSVar5->field_0x50).field_0x14 = 0x00ffffef;
						}
					}
				}
			}
			if (mode != 0) {
				pCVar4 = AllocateCameraPanStatic_002abc80();
				ed3D::cameraPanStaticMasterArray_00449378[staticMeshMasterIndex].pFinalLink_0x1a4 = pCVar4;
				pCVar4 = AllocateCameraPanStatic_002abc80();
				ed3D::cameraPanStaticMasterArray_00449378[staticMeshMasterIndex].field_0x1a8 = pCVar4;
			}
		}
		return pSVar3;
	}

	int g_3DRendProfileObjIndex_004491e8 = 0;
	int g_3DFlushProfileObjIndex_004491ec = 0;
	byte BYTE_00448a64 = 0;
	byte BYTE_00449424 = 0;
	CameraObj_28* g_RenderActiveCameraObj_006f7d80 = NULL;
	int INT_004493a0 = 0;


	void FUN_002a8aa0(void)
	{
		g_RenderMeshTransform_004493f4 = (MeshTransformData*)0xefefefef;
		return;
	}

	void FUN_002ab830(void)
	{
		PTR_DAT_00449428 = (undefined*)0x0;
		return;
	}

	void FUN_0029c030(void)
	{
		FUN_002a8aa0();
		FUN_002ab830();
		return;
	}

	void sceVu0InverseMatrix(Matrix* m0, Matrix* m1)
	{
		float fVar1;
		float fVar2;
		float fVar3;
		float fVar4;
		float fVar5;
		float fVar6;
		float fVar7;
		float fVar8;
		float fVar9;
		float fVar10;
		float fVar11;
		float fVar12;
		float fVar13;

		fVar3 = m1->ac;
		fVar4 = m1->ba;
		fVar5 = m1->bb;
		fVar6 = m1->bc;
		fVar7 = m1->ca;
		fVar8 = m1->cb;
		fVar9 = m1->cc;
		fVar10 = m1->da;
		fVar11 = m1->db;
		fVar12 = m1->dc;
		fVar13 = m1->dd;
		fVar2 = m1->ab;
		fVar1 = m1->aa;
		m0->aa = fVar1;
		m0->ab = fVar4;
		m0->ac = fVar7;
		m0->ad = 0.0;
		m0->ba = fVar2;
		m0->bb = fVar5;
		m0->bc = fVar8;
		m0->bd = 0.0;
		m0->ca = fVar3;
		m0->cb = fVar6;
		m0->cc = fVar9;
		m0->cd = 0.0;
		m0->da = 0.0 - (fVar1 * fVar10 + fVar2 * fVar11 + fVar3 * fVar12);
		m0->db = 0.0 - (fVar4 * fVar10 + fVar5 * fVar11 + fVar6 * fVar12);
		m0->dc = 0.0 - (fVar7 * fVar10 + fVar8 * fVar11 + fVar9 * fVar12);
		m0->dd = fVar13;
		return;
	}

	struct ScratchpadAnimManager {
		struct Matrix* pMatrix_0x0;
		struct Matrix* pMatrix_0x4;
		struct Matrix* pMatrix_0x8;
		struct Matrix* pMatrix_0xc;
		struct Matrix* pMatrix_0x10;
		struct Vector* pVector_0x14;
		struct Vector* pVector_0x18;
		char* field_0x1c;
		struct Matrix** field_0x20;
		char* field_0x24;
		undefined4* field_0x28;
		struct CameraObj_130* pData_130;
		struct Matrix* pMatrix_0x30;
		char* field_0x34;
		char* field_0x38;
		char* field_0x3c;
		char* field_0x40;
		char* field_0x44;
		char* field_0x48;
		char* field_0x4c;
		char* field_0x50;
		char* field_0x54;
		char* field_0x58;
		struct ScratchPadAnimData_150* pData_150;
		struct RenderAnimationStatic* pRenderAnimationStatic_0x60;
		struct Vector* vector_0x64;
		struct AnimScratchpad* pAnimScratchpad;
	};

	ScratchpadAnimManager ScratchpadAnimManager_004494ec = { 0 };
	Matrix* g_ScratchpadAddr_00448a58 = (Matrix*)0x70000000;
	Matrix Matrix_004894e0 = { 0 };

#ifdef PLATFORM_WIN
	void sceVu0Normalize(Vector* v0, Vector* v1)
	{
		float fVar1;
		float fVar2;
		float fVar3;
		float fVar4;

		fVar1 = v1->x;
		fVar2 = v1->y;
		fVar3 = v1->z;
		fVar4 = 1.0 / (sqrtf(fVar1 * fVar1 + fVar2 * fVar2 + fVar3 * fVar3) + 0.0);
		v0->x = fVar1 * fVar4;
		v0->y = fVar2 * fVar4;
		v0->z = fVar3 * fVar4;
		v0->w = 0.0;
		return;
	}
#endif

	Vector Vector_0048c300 = { 0 };
	Vector Vector_0048c310 = { 0 };
	Vector Vector_0048c320 = { 0 };
	Vector Vector_0048c330 = { 0 };

	float FLOAT_0044921c = 0.0f;
	float FLOAT_00449218 = 0.0f;
	float FLOAT_00449214 = 0.0f;

	int SetupCameraObj130_0029a960(StaticMeshMaster* pStaticMeshMaster, long mode)
	{
		//uint uVar1;
		//Vector* v0;
		//float fVar2;
		//float fVar3;
		//float fVar4;
		//Matrix local_40;
		//
		//fVar4 = (pStaticMeshMaster->pCameraObj130_0x8->matrix_0x0).dc;
		//memcpy(ScratchpadAnimManager_004494ec.pData_130, pStaticMeshMaster->pCameraObj130_0x8, 0x130);
		//sceVu0CopyMatrix((float(*)[4])g_ScratchpadAddr_00448a58, (float(*)[4])&(ScratchpadAnimManager_004494ec.pData_130)->matrix_0x80);
		//if (mode != 0) {
		//	sceVu0CopyMatrix((float(*)[4]) &Matrix_004894e0, (float(*)[4]) &(ScratchpadAnimManager_004494ec.pData_130)->matrix_0x80);
		//}
		//((ScratchpadAnimManager_004494ec.pData_130)->vector_0x110).y = 0.0;
		//((ScratchpadAnimManager_004494ec.pData_130)->vector_0x110).x = g_ScratchpadAddr_00448a58->cc;
		//((ScratchpadAnimManager_004494ec.pData_130)->vector_0x110).z = -g_ScratchpadAddr_00448a58->ac;
		//v0 = &(ScratchpadAnimManager_004494ec.pData_130)->vector_0x110;
		//if (((ScratchpadAnimManager_004494ec.pData_130)->vector_0x110).z +
		//	((ScratchpadAnimManager_004494ec.pData_130)->vector_0x110).x +
		//	((ScratchpadAnimManager_004494ec.pData_130)->vector_0x110).y == 0.0) {
		//	v0->x = 1.0;
		//}
		//else {
		//	sceVu0Normalize(v0, v0);
		//}
		//sceVu0InverseMatrix(&local_40, g_ScratchpadAddr_00448a58);
		//Vector_0048c310.z = local_40.ac;
		//Vector_0048c310.w = local_40.ad;
		//Vector_0048c310.x = local_40.aa;
		//Vector_0048c310.y = local_40.ab;
		//sceVu0Normalize(&Vector_0048c310, &Vector_0048c310);
		//Vector_0048c320.x = local_40.ba;
		//Vector_0048c320.y = local_40.bb;
		//Vector_0048c320.z = local_40.bc;
		//Vector_0048c320.w = local_40.bd;
		//sceVu0Normalize(&Vector_0048c320, &Vector_0048c320);
		//Vector_0048c330.x = local_40.ca;
		//Vector_0048c330.y = local_40.cb;
		//Vector_0048c330.z = local_40.cc;
		//Vector_0048c330.w = local_40.cd;
		//sceVu0Normalize(&Vector_0048c330, &Vector_0048c330);
		//Vector_0048c300.x = ((ScratchpadAnimManager_004494ec.pData_130)->matrix_0x0).aa;
		//Vector_0048c300.y = ((ScratchpadAnimManager_004494ec.pData_130)->matrix_0x0).ab;
		//Vector_0048c300.z = ((ScratchpadAnimManager_004494ec.pData_130)->matrix_0x0).ac;
		//Vector_0048c300.w = ((ScratchpadAnimManager_004494ec.pData_130)->matrix_0x0).ad;
		//fVar2 = ((ScratchpadAnimManager_004494ec.pData_130)->matrix_0x0).dc /
		//	((ScratchpadAnimManager_004494ec.pData_130)->matrix_0x0).dd;
		//FLOAT_0044921c = 1.0 / fVar2;
		//FLOAT_00449218 = fVar2 * fVar2;
		//if (mode != 0) {
		//	FLOAT_00449214 = FLOAT_00449218;
		//}
		//memcpy(&PTR_VidModeData_00489520, pStaticMeshMaster->pCameraObj28_0xc->pVidModeData30_0x10, 0x30);
		//g_CameraPanStatic_50_Ptr_00449220 = &pStaticMeshMaster->field_0x50;
		//(pStaticMeshMaster->field_0x50).field_0x18 = (pStaticMeshMaster->field_0x50).field_0x4;
		//memcpy(ScratchpadAnimManager_004494ec.pData_150, g_CameraPanStatic_50_Ptr_00449220, 0x150);
		//PTR_StaticMeshMaster_ARRAY_004d7d60[12].field_0x0_00449224 = pStaticMeshMaster;
		//CameraPanStatic_C::Func_00293ec0(&(pStaticMeshMaster->field_0x50).field_0x130);
		//FUN_0029b610((float)(int)pStaticMeshMaster->pCameraObj28_0xc->screenWidth,
		//	(float)(int)pStaticMeshMaster->pCameraObj28_0xc->screenHeight,
		//	((ScratchpadAnimManager_004494ec.pData_130)->matrix_0x0).db,
		//	((ScratchpadAnimManager_004494ec.pData_130)->matrix_0x0).dc,
		//	((ScratchpadAnimManager_004494ec.pData_130)->matrix_0x0).dd, 2048.0, 2048.0,
		//	(float)(ScratchpadAnimManager_004494ec.pData_150)->field_0x8, ScratchpadAnimManager_004494ec.pMatrix_0x0,
		//	ScratchpadAnimManager_004494ec.pMatrix_0x4, ScratchpadAnimManager_004494ec.pMatrix_0x8, &Matrix_00489460);
		//CameraSetupFunc_0029b120
		//(ScratchpadAnimManager_004494ec.pData_150, (ScratchPadAnimData_130*)ScratchpadAnimManager_004494ec.pData_130
		//	, (VidModeData*)&PTR_VidModeData_00489520);
		//sceVu0MulMatrix((sceVu0FMATRIX*)&DAT_004894a0, (sceVu0FMATRIX*)g_ScratchpadAddr_00448a58,
		//	(sceVu0FMATRIX*)ScratchpadAnimManager_004494ec.pMatrix_0x0);
		//if ((pStaticMeshMaster->flags_0x4 & 2) != 0) {
		//	memcpy(&Matrix_0048cbb0, ScratchpadAnimManager_004494ec.pMatrix_0x0, 0x40);
		//	memcpy(&Matrix_0048cbf0, ScratchpadAnimManager_004494ec.pMatrix_0x4, 0x40);
		//	memcpy(&Matrix_0048cc30, ScratchpadAnimManager_004494ec.pMatrix_0x8, 0x40);
		//	memcpy(&Matrix_0048cc70, &Matrix_00489460, 0x40);
		//	memcpy(&Matrix_0048ccb0, g_ScratchpadAddr_00448a58, 0x40);
		//	memcpy(&Vector_0048ccf0, &Vector_0048c2d0, 0x10);
		//	memcpy(&Vector_0048cd00, &Vector_0048c2e0, 0x10);
		//	memcpy(&Vector_0048cd10, &Vector_0048c2f0, 0x10);
		//	memcpy(&Vector_0048cd20, &Vector_0048c300, 0x10);
		//	memcpy(&Vector_0048cd30, &Vector_0048c310, 0x10);
		//	memcpy(&Vector_0048cd40, &Vector_0048c320, 0x10);
		//}
		//if (pStaticMeshMaster->field_0x0 == 1) {
		//	assert(false);
		//	//uVar1 = FUN_0029e3f0((int)pStaticMeshMaster->pCameraObj28_0xc->screenWidth);
		//	//if ((int)uVar1 < 0) {
		//	//	fVar2 = (float)(uVar1 >> 1 | uVar1 & 1);
		//	//	fVar2 = fVar2 + fVar2;
		//	//}
		//	//else {
		//	//	fVar2 = (float)uVar1;
		//	//}
		//	//uVar1 = FUN_0029e3f0((int)pStaticMeshMaster->pCameraObj28_0xc->screenHeight);
		//	//if ((int)uVar1 < 0) {
		//	//	fVar3 = (float)(uVar1 >> 1 | uVar1 & 1);
		//	//	fVar3 = fVar3 + fVar3;
		//	//}
		//	//else {
		//	//	fVar3 = (float)uVar1;
		//	//}
		//	//PTR_DAT_004492ec = &DAT_0048c3b0;
		//	//PTR_DAT_004492f0 = &DAT_0048c3c0;
		//	//Vector_0048c300.x = 2048.0 - fVar2 / 2.0;
		//	//Vector_0048c300.y = 2048.0 - fVar3 / 2.0;
		//	//Vector_0048c300.z = 1.0 / fVar2;
		//	//Vector_0048c300.w = 1.0 / fVar3;
		//	//FUN_002b6d10(*(undefined4*)&(ScratchpadAnimManager_004494ec.pData_150)->field_0x114);
		//	//*ScratchpadAnimManager_004494ec.field_0x28 = (uint)(ScratchpadAnimManager_004494ec.pData_150)->field_0x120;
		//	//if (*ScratchpadAnimManager_004494ec.field_0x28 == 0) {
		//	//	return 0;
		//	//}
		//	//memcpy(&DAT_00489550, g_ScratchpadAddr_00448a58, 0x40);
		//	//PTR_StaticMeshMaster_ARRAY_004d7d60[2].field_0x0_00449228 = pStaticMeshMaster;
		//}
		//else {
		//	*ScratchpadAnimManager_004494ec.field_0x28 = 0;
		//	PTR_DAT_004492ec = &DAT_0048c370;
		//	PTR_DAT_004492f0 = &DAT_0048c380;
		//}
		//if (pStaticMeshMaster->pCameraObj28_0xc->screenHeight != 0x200) {
		//	(pStaticMeshMaster->pCameraObj130_0x8->matrix_0x0).dc = fVar4;
		//}
		return 1;
	}

	RenderCommand* FUN_0029c640(RenderCommand* param_1)
	{
		uint uVar1;
		uint uVar2;
		uint uVar3;
		float fVar4;
		float fVar5;
		float fVar6;

		param_1->cmdA = 0xe10000008;
		param_1->cmdB = 0;
		*(undefined4*)&param_1->cmdB = 0;
		*(undefined4*)((ulong)&param_1->cmdB + 4) = 0x6c080000;
		uVar3 = UINT_0048c2cc;
		uVar2 = UINT_0048c2c8;
		uVar1 = UINT_0048c2c4;
		*(uint*)&param_1[1].cmdA = UINT_0048c2c0;
		*(uint*)((ulong)&param_1[1].cmdA + 4) = uVar1;
		*(uint*)&param_1[1].cmdB = uVar2;
		*(uint*)((ulong)&param_1[1].cmdB + 4) = uVar3;
		param_1[2].cmdA = 0x302ec00000008000;
		param_1[2].cmdB = 0x512;
		fVar6 = Vector_0048c2d0.w;
		fVar5 = Vector_0048c2d0.z;
		fVar4 = Vector_0048c2d0.y;
		*(float*)&param_1[3].cmdA = Vector_0048c2d0.x;
		*(float*)((ulong)&param_1[3].cmdA + 4) = fVar4;
		*(float*)&param_1[3].cmdB = fVar5;
		*(float*)((ulong)&param_1[3].cmdB + 4) = fVar6;
		fVar6 = Vector_0048c2e0.w;
		fVar5 = Vector_0048c2e0.z;
		fVar4 = Vector_0048c2e0.y;
		*(float*)&param_1[4].cmdA = Vector_0048c2e0.x;
		*(float*)((ulong)&param_1[4].cmdA + 4) = fVar4;
		*(float*)&param_1[4].cmdB = fVar5;
		*(float*)((ulong)&param_1[4].cmdB + 4) = fVar6;
		fVar6 = Vector_0048c2f0.w;
		fVar5 = Vector_0048c2f0.z;
		fVar4 = Vector_0048c2f0.y;
		*(float*)&param_1[5].cmdA = Vector_0048c2f0.x;
		*(float*)((ulong)&param_1[5].cmdA + 4) = fVar4;
		*(float*)&param_1[5].cmdB = fVar5;
		*(float*)((ulong)&param_1[5].cmdB + 4) = fVar6;
		fVar6 = Vector_0048c300.w;
		fVar5 = Vector_0048c300.z;
		fVar4 = Vector_0048c300.y;
		*(float*)&param_1[6].cmdA = Vector_0048c300.x;
		*(float*)((ulong)&param_1[6].cmdA + 4) = fVar4;
		*(float*)&param_1[6].cmdB = fVar5;
		*(float*)((ulong)&param_1[6].cmdB + 4) = fVar6;
		fVar6 = Vector_0048c310.w;
		fVar5 = Vector_0048c310.z;
		fVar4 = Vector_0048c310.y;
		*(float*)&param_1[7].cmdA = Vector_0048c310.x;
		*(float*)((ulong)&param_1[7].cmdA + 4) = fVar4;
		*(float*)&param_1[7].cmdB = fVar5;
		*(float*)((ulong)&param_1[7].cmdB + 4) = fVar6;
		fVar6 = Vector_0048c320.w;
		fVar5 = Vector_0048c320.z;
		fVar4 = Vector_0048c320.y;
		*(float*)&param_1[8].cmdA = Vector_0048c320.x;
		*(float*)((ulong)&param_1[8].cmdA + 4) = fVar4;
		*(float*)&param_1[8].cmdB = fVar5;
		*(float*)((ulong)&param_1[8].cmdB + 4) = fVar6;
		return param_1 + 9;
	}

	void LinkStaticList_0029c140(void)
	{
		sceDmaAddRef((sceDmaTag**)&g_VideoMemoryPtr_00449280, 1, (void*)g_StaticVideoList_00489590);
		g_VideoMemoryPtr_00449280 = FUN_0029c640(g_VideoMemoryPtr_00449280);
		return;
	}

	RenderCommand* FUN_002ab2d0(CameraObj_28* param_1, RenderCommand* param_2)
	{
		RenderCommand* pRVar1;
		uint uVar2;

		pRVar1 = param_2;
		if (param_1 != (CameraObj_28*)0x0) {
			pRVar1 = BuildCameraCommands_002bb110(param_1, param_2 + 1);
			uVar2 = ((uint)((ulong)pRVar1 - (ulong)param_2) >> 4) - 1;
			param_2->cmdA = (ulong)(ulong)uVar2 & 0xffffffffU | 0xe10000000;
			param_2->cmdB = 0;
			*(undefined4*)&param_2->cmdB = 0;
			*(uint*)((ulong)&param_2->cmdB + 4) = uVar2 | 0x51000000;
		}
		return pRVar1;
	}

	RenderCommand* FUN_002b1990(StaticMeshMaster* param_1, RenderCommand* param_2)
	{
		RenderCommand* pRVar1;

		param_2->cmdA = 0xe10000003;
		param_2->cmdB = 0;
		*(undefined4*)&param_2->cmdB = 0x40003dc;
		*(undefined4*)((ulong)&param_2->cmdB + 4) = 0x6c0203dc;

		param_2[1].cmdA = SCE_GIF_SET_TAG(
			1,					// NLOOP
			SCE_GS_TRUE,		// EOP
			SCE_GS_FALSE,		// PRE
			0,					// PRIM
			SCE_GIF_PACKED,		// FLG
			1					// NREG
		);
		param_2[1].cmdB = SCE_GIF_PACKED_AD;

		param_2[2].cmdA = (ulong)(param_1->field_0x50).fogCol_0xf0;
		param_2[2].cmdB = SCE_GS_FOGCOL;
		
		//param_2[3].cmdA = SCE_GS_SET_PRIM(
		//	SCE_GS_PRIM_TRISTRIP,	// PRIM (Primitive type)
		//	0,						// IIP  (Gouraud)
		//	SCE_GS_TRUE,			// TME  (Textured)
		//	0,						// FGE  (Fogging)
		//	0,						// ABE  (Alpha Blending)
		//	0,						// AA1  (Anti-Aliasing)
		//	0,						// FST  (Use ST for texture coords)
		//	0,						// CTXT (Context)
		//	0						// FIX  (Fragment control)
		//);

		//param_2[3].cmdB = SCE_GS_PRIM;

		*(undefined4*)&param_2[3].cmdA = 0;
		*(undefined4*)((ulong)&param_2[3].cmdA + 4) = 0x14000000;
		param_2[3].cmdB = 0x11000000;
		pRVar1 = param_2 + 4;
		if (param_1->pCameraObj28_0xc != (CameraObj_28*)0x0) {
			pRVar1 = FUN_002ab2d0(param_1->pCameraObj28_0xc, pRVar1);
		}
		return pRVar1;
	}

	struct CameraScreenData {
		short posX;
		short posY;
		short screenWidth;
		short screenHeight;
	};

	CameraScreenData CameraScreenData_00449390 = { 0 };
	CameraScreenData CameraScreenData_00449560 = { 0 };

	RenderCommand* SetScreenParams_002aabd0(RenderCommand* pRenderCommand, CameraScreenData* pScreenData)
	{
		pRenderCommand->cmdA = 0xe10000003;
		pRenderCommand->cmdB = 0;
		*(undefined4*)&pRenderCommand->cmdB = 0;
		*(undefined4*)((ulong)&pRenderCommand->cmdB + 4) = 0x50000003;

		pRenderCommand[1].cmdA = SCE_GIF_SET_TAG(
			2,					// NLOOP
			SCE_GS_TRUE,		// EOP
			SCE_GS_FALSE,		// PRE
			1,					// PRIM
			SCE_GIF_PACKED,		// FLG
			1					// NREG
		);
		pRenderCommand[1].cmdB = SCE_GIF_PACKED_AD;

		// OFFSET
		pRenderCommand[2].cmdA = SCE_GS_SET_XYOFFSET(
			(((0x1000 - ((int)pScreenData->screenWidth + (int)pScreenData->posX) >> 1) - ((int)pScreenData->posX >> 1)) * 0x10) & 0xffffffffU,	// X
			(((0x1000 - ((int)pScreenData->screenHeight + (int)pScreenData->posY) >> 1) - ((int)pScreenData->posY >> 1)) * 0x10)	// Y
		);
		pRenderCommand[2].cmdB = SCE_GS_XYOFFSET_1;

		// SCISSOR
		pRenderCommand[3].cmdA = SCE_GS_SET_SCISSOR(
			pScreenData->posX,											// SCAX0
			((int)pScreenData->screenWidth + (int)pScreenData->posX + -1),	// SCAX1
			pScreenData->posY,											// SCAY0
			((int)pScreenData->screenHeight + (int)pScreenData->posY + -1)	// SCAY1
		);
		pRenderCommand[3].cmdB = SCE_GS_SCISSOR_1;

		return pRenderCommand + 4;
	}

	uint CallMeshRenderFunc_002b2b70(StaticMeshMaster* pInStaticMeshMaster, StaticMeshMaster* pStaticMeshMaster)
	{
		float fVar1;
		CameraObj_28* pCVar2;
		int iVar3;
		bool bVar4;
		uint uVar5;
		uint uVar6;
		//ProfileObject* pPVar7;
		MeshTransformParent* pCVar10;
		MeshTransformParent* pCVar9;
		StaticMeshMaster* local_10;
		undefined4 local_c;
		undefined2 local_8;
		MeshTransformParent* pCVar8;

		iVar3 = g_3DFlushProfileObjIndex_004491ec;
		if ((pInStaticMeshMaster->field_0x0 == 1) && ((pStaticMeshMaster->flags_0x4 & 0x10) != 0)) {
			uVar6 = 0;
		}
		else {
			if (ed3D::Params_00424e30.bEnableProfile != 0) {
				assert(false);
				//if ((g_3DFlushProfileObjIndex_004491ec & 1U) == 0) {
				//	uVar6 = (uint)g_3DFlushProfileObjIndex_004491ec >> 4;
				//	if (g_ProfileObjB_0041ed44[uVar6].field_0x34 != 0) {
				//		g_ProfileObjB_0041ed44[uVar6].field_0x8 = 0;
				//		g_ProfileObjB_0041ed44[uVar6].field_0x20 = 0;
				//		g_ProfileObjB_0041ed44[uVar6].field_0x34 = 0;
				//	}
				//	g_ProfileObjB_0041ed44[(uint)iVar3 >> 4].field_0x0 = (long)PCR1 & 0xffffffff;
				//	g_ProfileObjB_0041ed44[(uint)iVar3 >> 4].field_0x20 = g_ProfileObjB_0041ed44[(uint)iVar3 >> 4].field_0x20 + 1;
				//}
				//else {
				//	uVar6 = (uint)g_3DFlushProfileObjIndex_004491ec >> 4;
				//	if (g_ProfileObjA_0041ed40[uVar6].field_0x34 != 0) {
				//		g_ProfileObjA_0041ed40[uVar6].field_0x8 = 0;
				//		g_ProfileObjA_0041ed40[uVar6].field_0x20 = 0;
				//		g_ProfileObjA_0041ed40[uVar6].field_0x34 = 0;
				//	}
				//	g_ProfileObjA_0041ed40[(uint)iVar3 >> 4].field_0x0 = (long)PCR1 & 0xffffffff;
				//	g_ProfileObjA_0041ed40[(uint)iVar3 >> 4].field_0x20 = g_ProfileObjA_0041ed40[(uint)iVar3 >> 4].field_0x20 + 1;
				//}
			}
			FUN_0029c030();
			uVar6 = SetupCameraObj130_0029a960(pInStaticMeshMaster, (ulong)((pInStaticMeshMaster->flags_0x4 & 0x20) != 0));
			uVar6 = uVar6 & 0xff;
			if (ed3D::Params_00424e30.bEnableProfile != 0) {
				assert(false);
				//uVar5 = (uint)g_3DFlushProfileObjIndex_004491ec >> 4;
				//if ((g_3DFlushProfileObjIndex_004491ec & 1U) == 0) {
				//	g_ProfileObjB_0041ed44[uVar5].field_0x0 = (long)PCR1 - g_ProfileObjB_0041ed44[uVar5].field_0x0;
				//	g_ProfileObjB_0041ed44[uVar5].field_0x8 =
				//		g_ProfileObjB_0041ed44[uVar5].field_0x8 + g_ProfileObjB_0041ed44[uVar5].field_0x0;
				//	pPVar7 = g_ProfileObjB_0041ed44 + uVar5;
				//	if (pPVar7->field_0x10 < g_ProfileObjB_0041ed44[uVar5].field_0x0) {
				//		pPVar7->field_0x10 = pPVar7->field_0x0;
				//	}
				//}
				//else {
				//	g_ProfileObjA_0041ed40[uVar5].field_0x0 = (long)PCR1 - g_ProfileObjA_0041ed40[uVar5].field_0x0;
				//	g_ProfileObjA_0041ed40[uVar5].field_0x8 =
				//		g_ProfileObjA_0041ed40[uVar5].field_0x8 + g_ProfileObjA_0041ed40[uVar5].field_0x0;
				//	pPVar7 = g_ProfileObjA_0041ed40 + uVar5;
				//	if (pPVar7->field_0x10 < g_ProfileObjA_0041ed40[uVar5].field_0x0) {
				//		pPVar7->field_0x10 = pPVar7->field_0x0;
				//	}
				//}
			}
			iVar3 = g_3DFlushProfileObjIndex_004491ec;
			if (uVar6 != 0) {
				g_RenderPassCount_0044939c = g_RenderPassCount_0044939c + 1;
				if (ed3D::Params_00424e30.bEnableProfile != 0) {
					assert(false);
				//	if ((g_3DFlushProfileObjIndex_004491ec & 1U) == 0) {
				//		uVar5 = (uint)g_3DFlushProfileObjIndex_004491ec >> 4;
				//		if (g_ProfileObjB_0041ed44[uVar5].field_0x34 != 0) {
				//			g_ProfileObjB_0041ed44[uVar5].field_0x8 = 0;
				//			g_ProfileObjB_0041ed44[uVar5].field_0x20 = 0;
				//			g_ProfileObjB_0041ed44[uVar5].field_0x34 = 0;
				//		}
				//		g_ProfileObjB_0041ed44[(uint)iVar3 >> 4].field_0x0 = (long)PCR1 & 0xffffffff;
				//		g_ProfileObjB_0041ed44[(uint)iVar3 >> 4].field_0x20 = g_ProfileObjB_0041ed44[(uint)iVar3 >> 4].field_0x20 + 1;
				//	}
				//	else {
				//		uVar5 = (uint)g_3DFlushProfileObjIndex_004491ec >> 4;
				//		if (g_ProfileObjA_0041ed40[uVar5].field_0x34 != 0) {
				//			g_ProfileObjA_0041ed40[uVar5].field_0x8 = 0;
				//			g_ProfileObjA_0041ed40[uVar5].field_0x20 = 0;
				//			g_ProfileObjA_0041ed40[uVar5].field_0x34 = 0;
				//		}
				//		g_ProfileObjA_0041ed40[(uint)iVar3 >> 4].field_0x0 = (long)PCR1 & 0xffffffff;
				//		g_ProfileObjA_0041ed40[(uint)iVar3 >> 4].field_0x20 = g_ProfileObjA_0041ed40[(uint)iVar3 >> 4].field_0x20 + 1;
				//	}
				}
				LinkStaticList_0029c140();
				g_VideoMemoryPtr_00449280 = FUN_002b1990(pInStaticMeshMaster, g_VideoMemoryPtr_00449280);
				if (ed3D::Params_00424e30.bEnableProfile != 0) {
					assert(false);
				//	uVar5 = (uint)g_3DFlushProfileObjIndex_004491ec >> 4;
				//	if ((g_3DFlushProfileObjIndex_004491ec & 1U) == 0) {
				//		g_ProfileObjB_0041ed44[uVar5].field_0x0 = (long)PCR1 - g_ProfileObjB_0041ed44[uVar5].field_0x0;
				//		g_ProfileObjB_0041ed44[uVar5].field_0x8 =
				//			g_ProfileObjB_0041ed44[uVar5].field_0x8 + g_ProfileObjB_0041ed44[uVar5].field_0x0;
				//		pPVar7 = g_ProfileObjB_0041ed44 + uVar5;
				//		if (pPVar7->field_0x10 < g_ProfileObjB_0041ed44[uVar5].field_0x0) {
				//			pPVar7->field_0x10 = pPVar7->field_0x0;
				//		}
				//	}
				//	else {
				//		g_ProfileObjA_0041ed40[uVar5].field_0x0 = (long)PCR1 - g_ProfileObjA_0041ed40[uVar5].field_0x0;
				//		g_ProfileObjA_0041ed40[uVar5].field_0x8 =
				//			g_ProfileObjA_0041ed40[uVar5].field_0x8 + g_ProfileObjA_0041ed40[uVar5].field_0x0;
				//		pPVar7 = g_ProfileObjA_0041ed40 + uVar5;
				//		if (pPVar7->field_0x10 < g_ProfileObjA_0041ed40[uVar5].field_0x0) {
				//			pPVar7->field_0x10 = pPVar7->field_0x0;
				//		}
				//	}
				//	iVar3 = g_3DRendProfileObjIndex_004491e8;
				//	if ((g_3DRendProfileObjIndex_004491e8 & 1U) == 0) {
				//		uVar5 = (uint)g_3DRendProfileObjIndex_004491e8 >> 4;
				//		if (g_ProfileObjB_0041ed44[uVar5].field_0x34 != 0) {
				//			g_ProfileObjB_0041ed44[uVar5].field_0x8 = 0;
				//			g_ProfileObjB_0041ed44[uVar5].field_0x20 = 0;
				//			g_ProfileObjB_0041ed44[uVar5].field_0x34 = 0;
				//		}
				//		g_ProfileObjB_0041ed44[(uint)iVar3 >> 4].field_0x0 = (long)PCR1 & 0xffffffff;
				//		g_ProfileObjB_0041ed44[(uint)iVar3 >> 4].field_0x20 = g_ProfileObjB_0041ed44[(uint)iVar3 >> 4].field_0x20 + 1;
				//	}
				//	else {
				//		uVar5 = (uint)g_3DRendProfileObjIndex_004491e8 >> 4;
				//		if (g_ProfileObjA_0041ed40[uVar5].field_0x34 != 0) {
				//			g_ProfileObjA_0041ed40[uVar5].field_0x8 = 0;
				//			g_ProfileObjA_0041ed40[uVar5].field_0x20 = 0;
				//			g_ProfileObjA_0041ed40[uVar5].field_0x34 = 0;
				//		}
				//		g_ProfileObjA_0041ed40[(uint)iVar3 >> 4].field_0x0 = (long)PCR1 & 0xffffffff;
				//		g_ProfileObjA_0041ed40[(uint)iVar3 >> 4].field_0x20 = g_ProfileObjA_0041ed40[(uint)iVar3 >> 4].field_0x20 + 1;
				//	}
				}
				pCVar8 = (MeshTransformParent*)pStaticMeshMaster->pFinalLink_0x1a4;
				pCVar9 = pCVar8->pNext;
				//if (*ScratchpadAnimManager_004494ec.field_0x28 == 0) {
					//assert(false);
				//	if (ed3D::Params_00424e30.field_0x2d == 0) {
				//		for (; pCVar9 != pCVar8; pCVar9 = pCVar9->pSubMeshParent) {
				//			MeshTransformData::GenerateTransform_002b48f0(pCVar9->pMeshTransformData);
				//		}
				//	}
				//	else {
				//		for (; pCVar9 != pCVar8; pCVar9 = pCVar9->pSubMeshParent) {
				//			FUN_002b4a30(pCVar9->pMeshTransformData);
				//		}
				//	}
				//}
				//else {
				//	for (; pCVar9 != pCVar8; pCVar9 = pCVar9->pSubMeshParent) {
				//		MeshTransformData::GenerateShadow_002b4b80(pCVar9->pMeshTransformData);
				//	}
				//}
				//FUN_002ad7b0((int)pInStaticMeshMaster);
				//if ((pInStaticMeshMaster->field_0x0 != 1) &&
				//	(FUN_002a52a0((int)pInStaticMeshMaster), (pInStaticMeshMaster->flags_0x4 & 8) == 0)) {
				//	for (pCVar10 = (MeshTransformParent*)(pStaticMeshMaster->field_0x30).pLoadedData;
				//		(CameraPanMasterHeader*)pCVar10 != &pStaticMeshMaster->field_0x30; pCVar10 = pCVar10->pSubMeshParent) {
				//		if ((((uint)(pCVar10->pMeshTransformData->transformA).bb & 2) == 0) &&
				//			(fVar1 = (pCVar10->pMeshTransformData->transformA).ba, *(int*)((int)fVar1 + 0x24) != 0)) {
				//			bVar4 = FUN_002948b0((int)fVar1);
				//			uVar6 = (int)bVar4 & 0xff;
				//		}
				//	}
				//}
				local_10 = pStaticMeshMaster;
				if (BYTE_00448a5c == 0) {
					if (pInStaticMeshMaster == pStaticMeshMaster) {
						local_c = pInStaticMeshMaster->field_0x0;
						local_8 = 2;
						edSysHandlersCall(edDlist::sysHandler_0048cb90.mainIdentifier, edDlist::sysHandler_0048cb90.entries,
							edDlist::sysHandler_0048cb90.maxEventID, 0, &local_10);
					}
					else {
						local_c = pInStaticMeshMaster->field_0x0;
						local_8 = 1;
						if (pInStaticMeshMaster != pStaticMeshMaster) {
							local_8 = 3;
						}
						edSysHandlersCall(edDlist::sysHandler_0048cb90.mainIdentifier, edDlist::sysHandler_0048cb90.entries,
							edDlist::sysHandler_0048cb90.maxEventID, 0, &local_10);
					}
				}
				else {
					local_c = pInStaticMeshMaster->field_0x0;
					local_8 = 1;
					if (pInStaticMeshMaster != pStaticMeshMaster) {
						local_8 = 3;
					}
					edSysHandlersCall(edDlist::sysHandler_0048cb90.mainIdentifier, edDlist::sysHandler_0048cb90.entries,
						edDlist::sysHandler_0048cb90.maxEventID, 0, &local_10);
				}
				if (ed3D::Params_00424e30.bEnableProfile != 0) {
					assert(false);
				//	uVar5 = (uint)g_3DRendProfileObjIndex_004491e8 >> 4;
				//	if ((g_3DRendProfileObjIndex_004491e8 & 1U) == 0) {
				//		g_ProfileObjB_0041ed44[uVar5].field_0x0 = (long)PCR1 - g_ProfileObjB_0041ed44[uVar5].field_0x0;
				//		g_ProfileObjB_0041ed44[uVar5].field_0x8 =
				//			g_ProfileObjB_0041ed44[uVar5].field_0x8 + g_ProfileObjB_0041ed44[uVar5].field_0x0;
				//		pPVar7 = g_ProfileObjB_0041ed44 + uVar5;
				//		if (pPVar7->field_0x10 < g_ProfileObjB_0041ed44[uVar5].field_0x0) {
				//			pPVar7->field_0x10 = pPVar7->field_0x0;
				//		}
				//	}
				//	else {
				//		g_ProfileObjA_0041ed40[uVar5].field_0x0 = (long)PCR1 - g_ProfileObjA_0041ed40[uVar5].field_0x0;
				//		g_ProfileObjA_0041ed40[uVar5].field_0x8 =
				//			g_ProfileObjA_0041ed40[uVar5].field_0x8 + g_ProfileObjA_0041ed40[uVar5].field_0x0;
				//		pPVar7 = g_ProfileObjA_0041ed40 + uVar5;
				//		if (pPVar7->field_0x10 < g_ProfileObjA_0041ed40[uVar5].field_0x0) {
				//			pPVar7->field_0x10 = pPVar7->field_0x0;
				//		}
				//	}
				//	iVar3 = g_3DFlushProfileObjIndex_004491ec;
				//	if ((g_3DFlushProfileObjIndex_004491ec & 1U) == 0) {
				//		uVar5 = (uint)g_3DFlushProfileObjIndex_004491ec >> 4;
				//		if (g_ProfileObjB_0041ed44[uVar5].field_0x34 != 0) {
				//			g_ProfileObjB_0041ed44[uVar5].field_0x8 = 0;
				//			g_ProfileObjB_0041ed44[uVar5].field_0x20 = 0;
				//			g_ProfileObjB_0041ed44[uVar5].field_0x34 = 0;
				//		}
				//		g_ProfileObjB_0041ed44[(uint)iVar3 >> 4].field_0x0 = (long)PCR1 & 0xffffffff;
				//		g_ProfileObjB_0041ed44[(uint)iVar3 >> 4].field_0x20 = g_ProfileObjB_0041ed44[(uint)iVar3 >> 4].field_0x20 + 1;
				//	}
				//	else {
				//		uVar5 = (uint)g_3DFlushProfileObjIndex_004491ec >> 4;
				//		if (g_ProfileObjA_0041ed40[uVar5].field_0x34 != 0) {
				//			g_ProfileObjA_0041ed40[uVar5].field_0x8 = 0;
				//			g_ProfileObjA_0041ed40[uVar5].field_0x20 = 0;
				//			g_ProfileObjA_0041ed40[uVar5].field_0x34 = 0;
				//		}
				//		g_ProfileObjA_0041ed40[(uint)iVar3 >> 4].field_0x0 = (long)PCR1 & 0xffffffff;
				//		g_ProfileObjA_0041ed40[(uint)iVar3 >> 4].field_0x20 = g_ProfileObjA_0041ed40[(uint)iVar3 >> 4].field_0x20 + 1;
				//	}
				}
				if (pInStaticMeshMaster->field_0x0 == 1) {
					assert(false);
				//	*(undefined8*)g_VideoMemoryPtr_00449280 = 0xe10000002;
				//	*(undefined8*)g_VideoMemoryPtr_00449280->p = 0;
				//	g_VideoMemoryPtr_00449280->p[0] = 0;
				//	g_VideoMemoryPtr_00449280->p[1] = 0x50000002;
				//	g_VideoMemoryPtr_00449280 = g_VideoMemoryPtr_00449280 + 1;
				//	*(undefined8*)g_VideoMemoryPtr_00449280 = 0x1000800000008001;
				//	*(undefined8*)g_VideoMemoryPtr_00449280->p = 0xe;
				//	g_VideoMemoryPtr_00449280 = g_VideoMemoryPtr_00449280 + 1;
				//	*(ulong*)g_VideoMemoryPtr_00449280 =
				//		(ulong)pInStaticMeshMaster->pCameraObj28_0xc->pVidModeData30_0x14->size_0x8 | 0x130000000;
				//	*(undefined8*)g_VideoMemoryPtr_00449280->p = 0x4e;
				//	g_VideoMemoryPtr_00449280 = g_VideoMemoryPtr_00449280 + 1;
				//	ProcessDrawTypes_0029bc30();
				//	FUN_002b51d0();
				}
				else {
					//assert(false);
					//ProcessDrawTypes_0029bc30();
				}
				g_VideoMemoryPtr_00449280 = SetScreenParams_002aabd0(g_VideoMemoryPtr_00449280, &CameraScreenData_00449390);
				pCVar2 = pInStaticMeshMaster->pCameraObj28_0xc;
				CameraScreenData_00449560.posX = pCVar2->posX;
				CameraScreenData_00449560.posY = pCVar2->posY;
				CameraScreenData_00449560.screenWidth = pCVar2->screenWidth;
				CameraScreenData_00449560.screenHeight = pCVar2->screenHeight;
				g_VideoMemoryPtr_00449280 = SetScreenParams_002aabd0(g_VideoMemoryPtr_00449280, &CameraScreenData_00449390);
				//RenderSetupFunc_002b1220();
				if (ed3D::Params_00424e30.bEnableProfile != 0) {
					assert(false);
				//	uVar5 = (uint)g_3DFlushProfileObjIndex_004491ec >> 4;
				//	if ((g_3DFlushProfileObjIndex_004491ec & 1U) == 0) {
				//		g_ProfileObjB_0041ed44[uVar5].field_0x0 = (long)PCR1 - g_ProfileObjB_0041ed44[uVar5].field_0x0;
				//		g_ProfileObjB_0041ed44[uVar5].field_0x8 =
				//			g_ProfileObjB_0041ed44[uVar5].field_0x8 + g_ProfileObjB_0041ed44[uVar5].field_0x0;
				//		pPVar7 = g_ProfileObjB_0041ed44 + uVar5;
				//		if (pPVar7->field_0x10 < g_ProfileObjB_0041ed44[uVar5].field_0x0) {
				//			pPVar7->field_0x10 = pPVar7->field_0x0;
				//		}
				//	}
				//	else {
				//		g_ProfileObjA_0041ed40[uVar5].field_0x0 = (long)PCR1 - g_ProfileObjA_0041ed40[uVar5].field_0x0;
				//		g_ProfileObjA_0041ed40[uVar5].field_0x8 =
				//			g_ProfileObjA_0041ed40[uVar5].field_0x8 + g_ProfileObjA_0041ed40[uVar5].field_0x0;
				//		pPVar7 = g_ProfileObjA_0041ed40 + uVar5;
				//		if (pPVar7->field_0x10 < g_ProfileObjA_0041ed40[uVar5].field_0x0) {
				//			pPVar7->field_0x10 = pPVar7->field_0x0;
				//		}
				//	}
				}
			}
		}
		return uVar6;
	}


	uint LaunchRenderTask_002b39d0(StaticMeshMaster* pStaticMeshMaster)
	{
		bool bVar1;
		bool bVar2;
		byte bVar3;
		int iVar4;
		RenderCommand* pRVar5;
		uint uVar6;
		RenderCommand* pRVar7;
		uint uVar8;
		//ProfileObject* pPVar9;
		//RenderTaskData renderTaskData;

		iVar4 = g_3DFlushProfileObjIndex_004491ec;
		if (ed3D::Params_00424e30.bEnableProfile != 0) {
			assert(false);
			//if ((g_3DFlushProfileObjIndex_004491ec & 1U) == 0) {
			//	uVar8 = (uint)g_3DFlushProfileObjIndex_004491ec >> 4;
			//	if (g_ProfileObjB_0041ed44[uVar8].field_0x34 != 0) {
			//		g_ProfileObjB_0041ed44[uVar8].field_0x8 = 0;
			//		g_ProfileObjB_0041ed44[uVar8].field_0x20 = 0;
			//		g_ProfileObjB_0041ed44[uVar8].field_0x34 = 0;
			//	}
			//	g_ProfileObjB_0041ed44[(uint)iVar4 >> 4].field_0x0 = (long)PCR1 & 0xffffffff;
			//	g_ProfileObjB_0041ed44[(uint)iVar4 >> 4].field_0x20 = g_ProfileObjB_0041ed44[(uint)iVar4 >> 4].field_0x20 + 1;
			//}
			//else {
			//	uVar8 = (uint)g_3DFlushProfileObjIndex_004491ec >> 4;
			//	if (g_ProfileObjA_0041ed40[uVar8].field_0x34 != 0) {
			//		g_ProfileObjA_0041ed40[uVar8].field_0x8 = 0;
			//		g_ProfileObjA_0041ed40[uVar8].field_0x20 = 0;
			//		g_ProfileObjA_0041ed40[uVar8].field_0x34 = 0;
			//	}
			//	g_ProfileObjA_0041ed40[(uint)iVar4 >> 4].field_0x0 = (long)PCR1 & 0xffffffff;
			//	g_ProfileObjA_0041ed40[(uint)iVar4 >> 4].field_0x20 = g_ProfileObjA_0041ed40[(uint)iVar4 >> 4].field_0x20 + 1;
			//}
		}
		pRVar5 = SetScreenParams_002aabd0(g_VideoMemoryPtr_00449280, &CameraScreenData_00449390);
		g_VideoMemoryPtr_00449280 = pRVar5;
		FUN_0029c030();
		uVar8 = SetupCameraObj130_0029a960(pStaticMeshMaster, (ulong)((pStaticMeshMaster->flags_0x4 & 0x20) != 0));
		if (ed3D::Params_00424e30.bEnableProfile != 0) {
			assert(false);
			//uVar6 = (uint)g_3DFlushProfileObjIndex_004491ec >> 4;
			//if ((g_3DFlushProfileObjIndex_004491ec & 1U) == 0) {
			//	g_ProfileObjB_0041ed44[uVar6].field_0x0 = (long)PCR1 - g_ProfileObjB_0041ed44[uVar6].field_0x0;
			//	g_ProfileObjB_0041ed44[uVar6].field_0x8 =
			//		g_ProfileObjB_0041ed44[uVar6].field_0x8 + g_ProfileObjB_0041ed44[uVar6].field_0x0;
			//	pPVar9 = g_ProfileObjB_0041ed44 + uVar6;
			//	if (pPVar9->field_0x10 < g_ProfileObjB_0041ed44[uVar6].field_0x0) {
			//		pPVar9->field_0x10 = pPVar9->field_0x0;
			//	}
			//}
			//else {
			//	g_ProfileObjA_0041ed40[uVar6].field_0x0 = (long)PCR1 - g_ProfileObjA_0041ed40[uVar6].field_0x0;
			//	g_ProfileObjA_0041ed40[uVar6].field_0x8 =
			//		g_ProfileObjA_0041ed40[uVar6].field_0x8 + g_ProfileObjA_0041ed40[uVar6].field_0x0;
			//	pPVar9 = g_ProfileObjA_0041ed40 + uVar6;
			//	if (pPVar9->field_0x10 < g_ProfileObjA_0041ed40[uVar6].field_0x0) {
			//		pPVar9->field_0x10 = pPVar9->field_0x0;
			//	}
			//}
		}
		iVar4 = g_3DFlushProfileObjIndex_004491ec;
		if ((uVar8 & 0xff) != 0) {
			if (ed3D::Params_00424e30.bEnableProfile != 0) {
				assert(false);
				//if ((g_3DFlushProfileObjIndex_004491ec & 1U) == 0) {
				//	uVar6 = (uint)g_3DFlushProfileObjIndex_004491ec >> 4;
				//	if (g_ProfileObjB_0041ed44[uVar6].field_0x34 != 0) {
				//		g_ProfileObjB_0041ed44[uVar6].field_0x8 = 0;
				//		g_ProfileObjB_0041ed44[uVar6].field_0x20 = 0;
				//		g_ProfileObjB_0041ed44[uVar6].field_0x34 = 0;
				//	}
				//	g_ProfileObjB_0041ed44[(uint)iVar4 >> 4].field_0x0 = (long)PCR1 & 0xffffffff;
				//	g_ProfileObjB_0041ed44[(uint)iVar4 >> 4].field_0x20 = g_ProfileObjB_0041ed44[(uint)iVar4 >> 4].field_0x20 + 1;
				//}
				//else {
				//	uVar6 = (uint)g_3DFlushProfileObjIndex_004491ec >> 4;
				//	if (g_ProfileObjA_0041ed40[uVar6].field_0x34 != 0) {
				//		g_ProfileObjA_0041ed40[uVar6].field_0x8 = 0;
				//		g_ProfileObjA_0041ed40[uVar6].field_0x20 = 0;
				//		g_ProfileObjA_0041ed40[uVar6].field_0x34 = 0;
				//	}
				//	g_ProfileObjA_0041ed40[(uint)iVar4 >> 4].field_0x0 = (long)PCR1 & 0xffffffff;
				//	g_ProfileObjA_0041ed40[(uint)iVar4 >> 4].field_0x20 = g_ProfileObjA_0041ed40[(uint)iVar4 >> 4].field_0x20 + 1;
				//}
			}
			LinkStaticList_0029c140();
			bVar3 = pStaticMeshMaster->pCameraObj28_0xc->field_0x18[3];
			pStaticMeshMaster->pCameraObj28_0xc->field_0x18[3] = bVar3 & 0xfc;
			g_VideoMemoryPtr_00449280 = FUN_002b1990(pStaticMeshMaster, g_VideoMemoryPtr_00449280);
			pStaticMeshMaster->pCameraObj28_0xc->field_0x18[3] = bVar3;
			if (ed3D::Params_00424e30.bEnableProfile != 0) {
				assert(false);
				//uVar6 = (uint)g_3DFlushProfileObjIndex_004491ec >> 4;
				//if ((g_3DFlushProfileObjIndex_004491ec & 1U) == 0) {
				//	g_ProfileObjB_0041ed44[uVar6].field_0x0 = (long)PCR1 - g_ProfileObjB_0041ed44[uVar6].field_0x0;
				//	g_ProfileObjB_0041ed44[uVar6].field_0x8 =
				//		g_ProfileObjB_0041ed44[uVar6].field_0x8 + g_ProfileObjB_0041ed44[uVar6].field_0x0;
				//	pPVar9 = g_ProfileObjB_0041ed44 + uVar6;
				//	if (pPVar9->field_0x10 < g_ProfileObjB_0041ed44[uVar6].field_0x0) {
				//		pPVar9->field_0x10 = pPVar9->field_0x0;
				//	}
				//}
				//else {
				//	g_ProfileObjA_0041ed40[uVar6].field_0x0 = (long)PCR1 - g_ProfileObjA_0041ed40[uVar6].field_0x0;
				//	g_ProfileObjA_0041ed40[uVar6].field_0x8 =
				//		g_ProfileObjA_0041ed40[uVar6].field_0x8 + g_ProfileObjA_0041ed40[uVar6].field_0x0;
				//	pPVar9 = g_ProfileObjA_0041ed40 + uVar6;
				//	if (pPVar9->field_0x10 < g_ProfileObjA_0041ed40[uVar6].field_0x0) {
				//		pPVar9->field_0x10 = pPVar9->field_0x0;
				//	}
				//}
				//iVar4 = g_3DRendProfileObjIndex_004491e8;
				//if ((g_3DRendProfileObjIndex_004491e8 & 1U) == 0) {
				//	uVar6 = (uint)g_3DRendProfileObjIndex_004491e8 >> 4;
				//	if (g_ProfileObjB_0041ed44[uVar6].field_0x34 != 0) {
				//		g_ProfileObjB_0041ed44[uVar6].field_0x8 = 0;
				//		g_ProfileObjB_0041ed44[uVar6].field_0x20 = 0;
				//		g_ProfileObjB_0041ed44[uVar6].field_0x34 = 0;
				//	}
				//	g_ProfileObjB_0041ed44[(uint)iVar4 >> 4].field_0x0 = (long)PCR1 & 0xffffffff;
				//	g_ProfileObjB_0041ed44[(uint)iVar4 >> 4].field_0x20 = g_ProfileObjB_0041ed44[(uint)iVar4 >> 4].field_0x20 + 1;
				//}
				//else {
				//	uVar6 = (uint)g_3DRendProfileObjIndex_004491e8 >> 4;
				//	if (g_ProfileObjA_0041ed40[uVar6].field_0x34 != 0) {
				//		g_ProfileObjA_0041ed40[uVar6].field_0x8 = 0;
				//		g_ProfileObjA_0041ed40[uVar6].field_0x20 = 0;
				//		g_ProfileObjA_0041ed40[uVar6].field_0x34 = 0;
				//	}
				//	g_ProfileObjA_0041ed40[(uint)iVar4 >> 4].field_0x0 = (long)PCR1 & 0xffffffff;
				//	g_ProfileObjA_0041ed40[(uint)iVar4 >> 4].field_0x20 = g_ProfileObjA_0041ed40[(uint)iVar4 >> 4].field_0x20 + 1;
				//}
			}
			//renderTaskData.field_0x4 = pStaticMeshMaster->field_0x0;
			//renderTaskData.field_0x8 = 2;
			//renderTaskData.field_0x0 = pStaticMeshMaster;
			//edSysHandlersCall(edDlist::sysHandler_0048cb90.mainIdentifier, edDlist::sysHandler_0048cb90.entries,
			//	edDlist::sysHandler_0048cb90.maxEventID, 0, &renderTaskData);
			if (ed3D::Params_00424e30.bEnableProfile != 0) {
				assert(false);
				//uVar6 = (uint)g_3DRendProfileObjIndex_004491e8 >> 4;
				//if ((g_3DRendProfileObjIndex_004491e8 & 1U) == 0) {
				//	g_ProfileObjB_0041ed44[uVar6].field_0x0 = (long)PCR1 - g_ProfileObjB_0041ed44[uVar6].field_0x0;
				//	g_ProfileObjB_0041ed44[uVar6].field_0x8 =
				//		g_ProfileObjB_0041ed44[uVar6].field_0x8 + g_ProfileObjB_0041ed44[uVar6].field_0x0;
				//	pPVar9 = g_ProfileObjB_0041ed44 + uVar6;
				//	if (pPVar9->field_0x10 < g_ProfileObjB_0041ed44[uVar6].field_0x0) {
				//		pPVar9->field_0x10 = pPVar9->field_0x0;
				//	}
				//}
				//else {
				//	g_ProfileObjA_0041ed40[uVar6].field_0x0 = (long)PCR1 - g_ProfileObjA_0041ed40[uVar6].field_0x0;
				//	g_ProfileObjA_0041ed40[uVar6].field_0x8 =
				//		g_ProfileObjA_0041ed40[uVar6].field_0x8 + g_ProfileObjA_0041ed40[uVar6].field_0x0;
				//	pPVar9 = g_ProfileObjA_0041ed40 + uVar6;
				//	if (pPVar9->field_0x10 < g_ProfileObjA_0041ed40[uVar6].field_0x0) {
				//		pPVar9->field_0x10 = pPVar9->field_0x0;
				//	}
				//}
				//iVar4 = g_3DFlushProfileObjIndex_004491ec;
				//if ((g_3DFlushProfileObjIndex_004491ec & 1U) == 0) {
				//	uVar6 = (uint)g_3DFlushProfileObjIndex_004491ec >> 4;
				//	if (g_ProfileObjB_0041ed44[uVar6].field_0x34 != 0) {
				//		g_ProfileObjB_0041ed44[uVar6].field_0x8 = 0;
				//		g_ProfileObjB_0041ed44[uVar6].field_0x20 = 0;
				//		g_ProfileObjB_0041ed44[uVar6].field_0x34 = 0;
				//	}
				//	g_ProfileObjB_0041ed44[(uint)iVar4 >> 4].field_0x0 = (long)PCR1 & 0xffffffff;
				//	g_ProfileObjB_0041ed44[(uint)iVar4 >> 4].field_0x20 = g_ProfileObjB_0041ed44[(uint)iVar4 >> 4].field_0x20 + 1;
				//}
				//else {
				//	uVar6 = (uint)g_3DFlushProfileObjIndex_004491ec >> 4;
				//	if (g_ProfileObjA_0041ed40[uVar6].field_0x34 != 0) {
				//		g_ProfileObjA_0041ed40[uVar6].field_0x8 = 0;
				//		g_ProfileObjA_0041ed40[uVar6].field_0x20 = 0;
				//		g_ProfileObjA_0041ed40[uVar6].field_0x34 = 0;
				//	}
				//	g_ProfileObjA_0041ed40[(uint)iVar4 >> 4].field_0x0 = (long)PCR1 & 0xffffffff;
				//	g_ProfileObjA_0041ed40[(uint)iVar4 >> 4].field_0x20 = g_ProfileObjA_0041ed40[(uint)iVar4 >> 4].field_0x20 + 1;
				//}
			}
			pRVar7 = SetScreenParams_002aabd0(g_VideoMemoryPtr_00449280, &CameraScreenData_00449390);
			g_VideoMemoryPtr_00449280 = pRVar7;
			//ProcessDrawTypes_0029bc30();
			bVar1 = g_VideoMemoryPtr_00449280 == pRVar7;
			pRVar7 = SetScreenParams_002aabd0(g_VideoMemoryPtr_00449280, &CameraScreenData_00449390);
			g_VideoMemoryPtr_00449280 = pRVar7;
			//FUN_002b51d0();
			bVar2 = g_VideoMemoryPtr_00449280 == pRVar7;
			//RenderSetupFunc_002b1220();
			if (bVar2 && bVar1) {
				g_VideoMemoryPtr_00449280 = pRVar5;
			}
			pRVar5 = g_VideoMemoryPtr_00449280;
			if (ed3D::Params_00424e30.bEnableProfile != 0) {
				assert(false);
				//uVar6 = (uint)g_3DFlushProfileObjIndex_004491ec >> 4;
				//if ((g_3DFlushProfileObjIndex_004491ec & 1U) == 0) {
				//	g_ProfileObjB_0041ed44[uVar6].field_0x0 = (long)PCR1 - g_ProfileObjB_0041ed44[uVar6].field_0x0;
				//	g_ProfileObjB_0041ed44[uVar6].field_0x8 =
				//		g_ProfileObjB_0041ed44[uVar6].field_0x8 + g_ProfileObjB_0041ed44[uVar6].field_0x0;
				//	pPVar9 = g_ProfileObjB_0041ed44 + uVar6;
				//	pRVar5 = g_VideoMemoryPtr_00449280;
				//	if (pPVar9->field_0x10 < g_ProfileObjB_0041ed44[uVar6].field_0x0) {
				//		pPVar9->field_0x10 = pPVar9->field_0x0;
				//		pRVar5 = g_VideoMemoryPtr_00449280;
				//	}
				//}
				//else {
				//	g_ProfileObjA_0041ed40[uVar6].field_0x0 = (long)PCR1 - g_ProfileObjA_0041ed40[uVar6].field_0x0;
				//	g_ProfileObjA_0041ed40[uVar6].field_0x8 =
				//		g_ProfileObjA_0041ed40[uVar6].field_0x8 + g_ProfileObjA_0041ed40[uVar6].field_0x0;
				//	pPVar9 = g_ProfileObjA_0041ed40 + uVar6;
				//	pRVar5 = g_VideoMemoryPtr_00449280;
				//	if (pPVar9->field_0x10 < g_ProfileObjA_0041ed40[uVar6].field_0x0) {
				//		pPVar9->field_0x10 = pPVar9->field_0x0;
				//		pRVar5 = g_VideoMemoryPtr_00449280;
				//	}
				//}
			}
		}
		g_VideoMemoryPtr_00449280 = pRVar5;
		return uVar8 & 0xff;
	}

	byte BYTE_00449414 = 0;
	CameraObj_28* PTR_00449388 = NULL;

	RenderCommand* FUN_002ab350(RenderCommand* param_1)
	{
		byte bVar1;
		RenderCommand* pRVar2;
		RenderCommand* pRVar3;
		uint uVar4;

		pRVar3 = param_1;
		if (PTR_00449388 == (CameraObj_28*)0x0) {
			if ((g_CameraObj_28_00449384 != (CameraObj_28*)0x0) && (g_CameraObj_28_00449384 != (CameraObj_28*)0x0)) {
				bVar1 = g_CameraObj_28_00449384->field_0x18[3];
				g_CameraObj_28_00449384->field_0x18[3] = 0;
				pRVar2 = g_VideoMemoryPtr_00449280;
				if (g_CameraObj_28_00449384 != (CameraObj_28*)0x0) {
					g_VideoMemoryPtr_00449280 = BuildCameraCommands_002bb110(g_CameraObj_28_00449384, g_VideoMemoryPtr_00449280 + 1);
					uVar4 = ((uint)((ulong)g_VideoMemoryPtr_00449280 - (ulong)pRVar2) >> 4) - 1;
					pRVar2->cmdA = (ulong)(int)uVar4 & 0xffffffffU | 0xe10000000;
					pRVar2->cmdB = 0;
					*(undefined4*)&pRVar2->cmdB = 0;
					*(uint*)((ulong)&pRVar2->cmdB + 4) = uVar4 | 0x51000000;
				}
				g_CameraObj_28_00449384->field_0x18[3] = bVar1;
			}
		}
		else {
			if (PTR_00449388 != (CameraObj_28*)0x0) {
				pRVar3 = BuildCameraCommands_002bb110(PTR_00449388, param_1 + 1);
				uVar4 = ((uint)((ulong)pRVar3 - (ulong)param_1) >> 4) - 1;
				param_1->cmdA = (ulong)(int)uVar4 & 0xffffffffU | 0xe10000000;
				param_1->cmdB = 0;
				*(undefined4*)&param_1->cmdB = 0;
				*(uint*)((ulong)&param_1->cmdB + 4) = uVar4 | 0x51000000;
			}
		}
		return pRVar3;
	}

	uint UINT_00425000 = 0;

	RenderCommand* FUN_0029c1b0(RenderCommand* param_1)
	{
		param_1->cmdA = 0xe10000003;
		param_1->cmdB = 0;
		*(undefined4*)&param_1->cmdB = 0x40003dc;
		*(undefined4*)((ulong)&param_1->cmdB + 4) = 0x6c0203dc;

		param_1[1].cmdA = SCE_GIF_SET_TAG(
			1,					// NLOOP
			SCE_GS_TRUE,		// EOP
			SCE_GS_FALSE,		// PRE
			0,					// PRIM
			SCE_GIF_PACKED,		// FLG
			1					// NREG
		);
		param_1[1].cmdB = SCE_GIF_PACKED_AD;

		param_1[2].cmdA = SCE_GS_FINISH;
		param_1[2].cmdB = SCE_GS_FINISH;
		*(undefined4*)&param_1[3].cmdA = 0;
		*(undefined4*)((ulong)&param_1[3].cmdA + 4) = 0x14000000;
		param_1[3].cmdB = 0;
		return param_1 + 4;
	}

	void FUN_002aeb40(void)
	{
		ushort** ppuVar1;
		ulong* puVar2;
		int iVar3;
		RenderCommand* pRVar4;

		pRVar4 = g_VideoMemoryPtr_00449280;
		if (((UINT_00425000 & 1) != 0) && (g_RenderActiveCameraObj_006f7d80 != (CameraObj_28*)0x0)) {
			assert(false);
		}
		return;
	}

	RenderCommand* FUN_002b8640(RenderCommand* pRenderCommand)
	{
		ulong* puVar1;
		ushort** ppuVar2;
		int iVar3;

		if (((((BYTE_00448a70 != 0) && (BYTE_00449418 != 0)) && (g_RenderActiveCameraObj_006f7d80 != (CameraObj_28*)0x0)) &&
			(CameraScreenData_00449390.screenWidth == 0x200)) &&
			((CameraScreenData_00449390.screenHeight == 0x200 || (CameraScreenData_00449390.screenHeight == 0x1c0)))) {
			assert(false);
		}
		return pRenderCommand;
	}

	FrameBuffer* GetFramebuffer_002ba3b0(void)
	{
		FrameBuffer* pFVar1;

		if (g_ActiveVidParams_0048cd90.pFrameBuffer == (FrameBuffer*)0x0) {
			pFVar1 = (FrameBuffer*)0x0;
		}
		else {
			pFVar1 = (g_ActiveVidParams_0048cd90.pFrameBuffer)->pVidModeData_0x0->pLink_0xc;
		}
		return pFVar1;
	}

	void RenderScene(int, int, char*)
	{
		CameraObj_28* pCVar1;
		ulong uVar2;
		bool bVar3;
		RenderCommand* psVar4;
		undefined4 in_zero_lo;
		_sceDmaTag* in_zero_hi;
		uint in_zero_udw;
		uint in_register_0000000c;
		FrameBuffer* pFVar6;
		int iVar5;
		uint uVar6;
		//ProfileObject* pPVar7;
		_sceDmaTag* p_Var8;
		uint uVar9;
		CameraPanMasterHeader* pCVar10;
		StaticMeshMaster* pInStaticMeshMaster;
		uint staticMeshMasterIndex;
		uint staticMeshMasterFlags;

		iVar5 = g_3DRendProfileObjIndex_004491e8;
		if (BYTE_00448a64 == 0) {
			BYTE_00448a64 = 1;
			g_VideoMemoryPtr_00449280 = g_FlipDMA_00449270;
			g_RenderCommand_004492c0 = g_RenderCommand_004492b0;
		}
		else {
			g_RenderActiveCameraObj_006f7d80 = (CameraObj_28*)0x0;
			INT_004493a0 = INT_004493a0 != 0 ^ 1;
			if (ed3D::Params_00424e30.bEnableProfile != 0) {
				assert(false);
				//if ((g_3DRendProfileObjIndex_004491e8 & 1U) == 0) {
				//	uVar6 = (uint)g_3DRendProfileObjIndex_004491e8 >> 4;
				//	if (g_ProfileObjB_0041ed44[uVar6].field_0x34 != 0) {
				//		g_ProfileObjB_0041ed44[uVar6].field_0x8 = 0;
				//		g_ProfileObjB_0041ed44[uVar6].field_0x20 = 0;
				//		g_ProfileObjB_0041ed44[uVar6].field_0x34 = 0;
				//	}
				//	g_ProfileObjB_0041ed44[(uint)iVar5 >> 4].field_0x0 = (long)PCR1 & 0xffffffff;
				//	g_ProfileObjB_0041ed44[(uint)iVar5 >> 4].field_0x20 = g_ProfileObjB_0041ed44[(uint)iVar5 >> 4].field_0x20 + 1;
				//}
				//else {
				//	uVar6 = (uint)g_3DRendProfileObjIndex_004491e8 >> 4;
				//	if (g_ProfileObjA_0041ed40[uVar6].field_0x34 != 0) {
				//		g_ProfileObjA_0041ed40[uVar6].field_0x8 = 0;
				//		g_ProfileObjA_0041ed40[uVar6].field_0x20 = 0;
				//		g_ProfileObjA_0041ed40[uVar6].field_0x34 = 0;
				//	}
				//	g_ProfileObjA_0041ed40[(uint)iVar5 >> 4].field_0x0 = (long)PCR1 & 0xffffffff;
				//	g_ProfileObjA_0041ed40[(uint)iVar5 >> 4].field_0x20 = g_ProfileObjA_0041ed40[(uint)iVar5 >> 4].field_0x20 + 1;
				//}
			}
			//FUN_002b14a0();
			if ((ed3D::Params_00424e30.field_0x2c & 1) != 0) {
				assert(false);
				//UpdateMeshTransforms_002b2a40();
			}
			if (ed3D::Params_00424e30.bEnableProfile != 0) {
				assert(false);
				//uVar6 = (uint)g_3DRendProfileObjIndex_004491e8 >> 4;
				//if ((g_3DRendProfileObjIndex_004491e8 & 1U) == 0) {
				//	g_ProfileObjB_0041ed44[uVar6].field_0x0 = (long)PCR1 - g_ProfileObjB_0041ed44[uVar6].field_0x0;
				//	g_ProfileObjB_0041ed44[uVar6].field_0x8 =
				//		g_ProfileObjB_0041ed44[uVar6].field_0x8 + g_ProfileObjB_0041ed44[uVar6].field_0x0;
				//	pPVar7 = g_ProfileObjB_0041ed44 + uVar6;
				//	if (pPVar7->field_0x10 < g_ProfileObjB_0041ed44[uVar6].field_0x0) {
				//		pPVar7->field_0x10 = pPVar7->field_0x0;
				//	}
				//}
				//else {
				//	g_ProfileObjA_0041ed40[uVar6].field_0x0 = (long)PCR1 - g_ProfileObjA_0041ed40[uVar6].field_0x0;
				//	g_ProfileObjA_0041ed40[uVar6].field_0x8 =
				//		g_ProfileObjA_0041ed40[uVar6].field_0x8 + g_ProfileObjA_0041ed40[uVar6].field_0x0;
				//	pPVar7 = g_ProfileObjA_0041ed40 + uVar6;
				//	if (pPVar7->field_0x10 < g_ProfileObjA_0041ed40[uVar6].field_0x0) {
				//		pPVar7->field_0x10 = pPVar7->field_0x0;
				//	}
				//}
			}
			bVar3 = false;
			for (staticMeshMasterIndex = 0; staticMeshMasterIndex < (uint)ed3D::Params_00424e30.initialStaticMeshMasterCount;
				staticMeshMasterIndex = staticMeshMasterIndex + 1) {
				staticMeshMasterFlags = ed3D::cameraPanStaticMasterArray_00449378[staticMeshMasterIndex].flags_0x4;
				if ((((staticMeshMasterFlags & 1) != 0) && ((staticMeshMasterFlags & 4) == 0)) &&
					(ed3D::cameraPanStaticMasterArray_00449378[staticMeshMasterIndex].field_0x0 == 0)) {
					if (bVar3) {
						bVar3 = false;
						pFVar6 = GetFramebuffer_002ba3b0();
						if ((pFVar6 != (FrameBuffer*)0x0) && (g_RenderCommandBuffer_00449400 != (RenderCommand*)0x0)) {
							assert(false);
						//	*(undefined8*)g_VideoMemoryPtr_00449280 = 0x30000000;
						//	*(undefined8*)g_VideoMemoryPtr_00449280->p = 0x1300000000000000;
						//	psVar4 = g_VideoMemoryPtr_00449280;
						//	g_VideoMemoryPtr_00449280 = g_VideoMemoryPtr_00449280 + 1;
						//	uVar2 = *DAT_00449400;
						//	uVar6 = *(uint*)(DAT_00449400 + 1);
						//	uVar9 = *(uint*)((int)DAT_00449400 + 0xc);
						//	*(int*)g_VideoMemoryPtr_00449280 = (int)uVar2;
						//	psVar4[1].next = (_sceDmaTag*)(uVar2 >> 0x20);
						//	psVar4[1].p[0] = uVar6;
						//	psVar4[1].p[1] = uVar9;
						//	psVar4 = g_VideoMemoryPtr_00449280;
						//	g_VideoMemoryPtr_00449280 = g_VideoMemoryPtr_00449280 + 1;
						//	p_Var8 = *(_sceDmaTag**)((int)DAT_00449400 + 0x14);
						//	uVar6 = *(uint*)(DAT_00449400 + 3);
						//	uVar9 = *(uint*)((int)DAT_00449400 + 0x1c);
						//	*(undefined4*)g_VideoMemoryPtr_00449280 = *(undefined4*)(DAT_00449400 + 2);
						//	psVar4[1].next = p_Var8;
						//	psVar4[1].p[0] = uVar6;
						//	psVar4[1].p[1] = uVar9;
						//	psVar4 = g_VideoMemoryPtr_00449280;
						//	g_VideoMemoryPtr_00449280 = g_VideoMemoryPtr_00449280 + 1;
						//	p_Var8 = *(_sceDmaTag**)((int)DAT_00449400 + 0x24);
						//	uVar6 = *(uint*)(DAT_00449400 + 5);
						//	uVar9 = *(uint*)((int)DAT_00449400 + 0x2c);
						//	*(undefined4*)g_VideoMemoryPtr_00449280 = *(undefined4*)(DAT_00449400 + 4);
						//	psVar4[1].next = p_Var8;
						//	psVar4[1].p[0] = uVar6;
						//	psVar4[1].p[1] = uVar9;
						//	psVar4 = g_VideoMemoryPtr_00449280;
						//	g_VideoMemoryPtr_00449280 = g_VideoMemoryPtr_00449280 + 1;
						//	p_Var8 = *(_sceDmaTag**)((int)DAT_00449400 + 0x34);
						//	uVar6 = *(uint*)(DAT_00449400 + 7);
						//	uVar9 = *(uint*)((int)DAT_00449400 + 0x3c);
						//	*(undefined4*)g_VideoMemoryPtr_00449280 = *(undefined4*)(DAT_00449400 + 6);
						//	psVar4[1].next = p_Var8;
						//	psVar4[1].p[0] = uVar6;
						//	psVar4[1].p[1] = uVar9;
						//	g_VideoMemoryPtr_00449280 = g_VideoMemoryPtr_00449280 + 1;
						//	*DAT_00449400 = ((long)(int)(DAT_00449400 + 8) & 0xfffffffU) << 0x20 | 0x20000000;
						//	DAT_00449400[1] = 0;
						//	*(undefined8*)g_VideoMemoryPtr_00449280 = 0x30000000;
						//	*(undefined8*)g_VideoMemoryPtr_00449280->p = 0x1300000000000000;
						//	*(undefined8*)(g_VideoMemoryPtr_00449280 + 1) = 0x30000000;
						//	*(undefined8*)g_VideoMemoryPtr_00449280[1].p = 0x1100000011000000;
						//	*(undefined8*)(g_VideoMemoryPtr_00449280 + 2) = 0x30000000;
						//	*(undefined8*)g_VideoMemoryPtr_00449280[2].p = 0x600000000000000;
						//	*(undefined8*)(g_VideoMemoryPtr_00449280 + 3) = 0xe10000001;
						//	*(undefined8*)g_VideoMemoryPtr_00449280[3].p = 0;
						//	psVar4 = g_VideoMemoryPtr_00449280;
						//	*(undefined4*)(g_VideoMemoryPtr_00449280 + 4) = in_zero_lo;
						//	psVar4[4].next = in_zero_hi;
						//	psVar4[4].p[0] = in_zero_udw;
						//	psVar4[4].p[1] = in_register_0000000c;
						//	*(undefined8*)(g_VideoMemoryPtr_00449280 + 5) = 0x30000000;
						//	*(undefined8*)g_VideoMemoryPtr_00449280[5].p = 0x600800000000000;
						//	g_VideoMemoryPtr_00449280 = g_VideoMemoryPtr_00449280 + 6;
						}
					}
					psVar4 = g_VideoMemoryPtr_00449280;
					pInStaticMeshMaster = ed3D::cameraPanStaticMasterArray_00449378 + staticMeshMasterIndex;
					if (g_RenderActiveCameraObj_006f7d80 == (CameraObj_28*)0x0) {
						g_RenderActiveCameraObj_006f7d80 = pInStaticMeshMaster->pCameraObj28_0xc;
					}
					pCVar1 = pInStaticMeshMaster->pCameraObj28_0xc;
					CameraScreenData_00449390.posX = pCVar1->posX;
					CameraScreenData_00449390.posY = pCVar1->posY;
					CameraScreenData_00449390.screenWidth = pCVar1->screenWidth;
					CameraScreenData_00449390.screenHeight = pCVar1->screenHeight;
					//g_FrontendStaticMeshMaster_00449558 = pInStaticMeshMaster;
					CallMeshRenderFunc_002b2b70(pInStaticMeshMaster, pInStaticMeshMaster);
					for (pCVar10 = (CameraPanMasterHeader*)(pInStaticMeshMaster->field_0x10).pLoadedData;
						pCVar10 != &pInStaticMeshMaster->field_0x10; pCVar10 = (CameraPanMasterHeader*)pCVar10->pLoadedData) {
						if ((((StaticMeshMaster*)pCVar10->pCameraPanHeader_0xc)->flags_0x4 & 4) == 0) {
							CallMeshRenderFunc_002b2b70((StaticMeshMaster*)pCVar10->pCameraPanHeader_0xc, pInStaticMeshMaster);
						}
					}
					if (BYTE_00448a5c != 0) {
						LaunchRenderTask_002b39d0(pInStaticMeshMaster);
					}
					iVar5 = g_3DFlushProfileObjIndex_004491ec;
					if (ed3D::Params_00424e30.bEnableProfile != 0) {
						assert(false);
						//if ((g_3DFlushProfileObjIndex_004491ec & 1U) == 0) {
						//	uVar6 = (uint)g_3DFlushProfileObjIndex_004491ec >> 4;
						//	if (g_ProfileObjB_0041ed44[uVar6].field_0x34 != 0) {
						//		g_ProfileObjB_0041ed44[uVar6].field_0x8 = 0;
						//		g_ProfileObjB_0041ed44[uVar6].field_0x20 = 0;
						//		g_ProfileObjB_0041ed44[uVar6].field_0x34 = 0;
						//	}
						//	g_ProfileObjB_0041ed44[(uint)iVar5 >> 4].field_0x0 = (long)PCR1 & 0xffffffff;
						//	g_ProfileObjB_0041ed44[(uint)iVar5 >> 4].field_0x20 =
						//		g_ProfileObjB_0041ed44[(uint)iVar5 >> 4].field_0x20 + 1;
						//}
						//else {
						//	uVar6 = (uint)g_3DFlushProfileObjIndex_004491ec >> 4;
						//	if (g_ProfileObjA_0041ed40[uVar6].field_0x34 != 0) {
						//		g_ProfileObjA_0041ed40[uVar6].field_0x8 = 0;
						//		g_ProfileObjA_0041ed40[uVar6].field_0x20 = 0;
						//		g_ProfileObjA_0041ed40[uVar6].field_0x34 = 0;
						//	}
						//	g_ProfileObjA_0041ed40[(uint)iVar5 >> 4].field_0x0 = (long)PCR1 & 0xffffffff;
						//	g_ProfileObjA_0041ed40[(uint)iVar5 >> 4].field_0x20 =
						//		g_ProfileObjA_0041ed40[(uint)iVar5 >> 4].field_0x20 + 1;
						//}
					}
					if (psVar4 != g_VideoMemoryPtr_00449280) {
						if ((pInStaticMeshMaster->flags_0x4 & 0x40) != 0) {
							bVar3 = true;
							FUN_002aeb40();
						}
						if ((pInStaticMeshMaster->flags_0x4 & 0x800) == 0) {
							if ((pInStaticMeshMaster->flags_0x4 & 0x80) != 0) {
								bVar3 = true;
								g_VideoMemoryPtr_00449280 = FUN_002b8640(g_VideoMemoryPtr_00449280);
							}
						}
						else {
							bVar3 = true;
							assert(false);
							//g_VideoMemoryPtr_00449280 = (sceDmaTag*)FUN_002b8030((ulong*)g_VideoMemoryPtr_00449280);
						}
					}
					if (ed3D::Params_00424e30.bEnableProfile != 0) {
						assert(false);
						//uVar6 = (uint)g_3DFlushProfileObjIndex_004491ec >> 4;
						//if ((g_3DFlushProfileObjIndex_004491ec & 1U) == 0) {
						//	g_ProfileObjB_0041ed44[uVar6].field_0x0 = (long)PCR1 - g_ProfileObjB_0041ed44[uVar6].field_0x0;
						//	g_ProfileObjB_0041ed44[uVar6].field_0x8 =
						//		g_ProfileObjB_0041ed44[uVar6].field_0x8 + g_ProfileObjB_0041ed44[uVar6].field_0x0;
						//	pPVar7 = g_ProfileObjB_0041ed44 + uVar6;
						//	if (pPVar7->field_0x10 < g_ProfileObjB_0041ed44[uVar6].field_0x0) {
						//		pPVar7->field_0x10 = pPVar7->field_0x0;
						//	}
						//}
						//else {
						//	g_ProfileObjA_0041ed40[uVar6].field_0x0 = (long)PCR1 - g_ProfileObjA_0041ed40[uVar6].field_0x0;
						//	g_ProfileObjA_0041ed40[uVar6].field_0x8 =
						//		g_ProfileObjA_0041ed40[uVar6].field_0x8 + g_ProfileObjA_0041ed40[uVar6].field_0x0;
						//	pPVar7 = g_ProfileObjA_0041ed40 + uVar6;
						//	if (pPVar7->field_0x10 < g_ProfileObjA_0041ed40[uVar6].field_0x0) {
						//		pPVar7->field_0x10 = pPVar7->field_0x0;
						//	}
						//}
					}
				}
			}
			if (BYTE_00449424 != 0) {
				assert(false);
				//g_VideoMemoryPtr_00449280 = (sceDmaTag*)FUN_002aa4f0((ulong*)g_VideoMemoryPtr_00449280);
			}
			iVar5 = g_3DFlushProfileObjIndex_004491ec;
			if (ed3D::Params_00424e30.bEnableProfile != 0) {
				if ((g_3DFlushProfileObjIndex_004491ec & 1U) == 0) {
					assert(false);
					//uVar6 = (uint)g_3DFlushProfileObjIndex_004491ec >> 4;
					//if (g_ProfileObjB_0041ed44[uVar6].field_0x34 != 0) {
					//	g_ProfileObjB_0041ed44[uVar6].field_0x8 = 0;
					//	g_ProfileObjB_0041ed44[uVar6].field_0x20 = 0;
					//	g_ProfileObjB_0041ed44[uVar6].field_0x34 = 0;
					//}
					//g_ProfileObjB_0041ed44[(uint)iVar5 >> 4].field_0x0 = (long)PCR1 & 0xffffffff;
					//g_ProfileObjB_0041ed44[(uint)iVar5 >> 4].field_0x20 = g_ProfileObjB_0041ed44[(uint)iVar5 >> 4].field_0x20 + 1;
				}
				else {
					assert(false);
					//uVar6 = (uint)g_3DFlushProfileObjIndex_004491ec >> 4;
					//if (g_ProfileObjA_0041ed40[uVar6].field_0x34 != 0) {
					//	g_ProfileObjA_0041ed40[uVar6].field_0x8 = 0;
					//	g_ProfileObjA_0041ed40[uVar6].field_0x20 = 0;
					//	g_ProfileObjA_0041ed40[uVar6].field_0x34 = 0;
					//}
					//g_ProfileObjA_0041ed40[(uint)iVar5 >> 4].field_0x0 = (long)PCR1 & 0xffffffff;
					//g_ProfileObjA_0041ed40[(uint)iVar5 >> 4].field_0x20 = g_ProfileObjA_0041ed40[(uint)iVar5 >> 4].field_0x20 + 1;
				}
			}
			//FUN_002b1110();
			if ((g_FlipDMA_00449270 != (RenderCommand*)0x0) && (g_FlipDMA_00449270 != g_VideoMemoryPtr_00449280)) {
				g_VideoMemoryPtr_00449280 = FUN_002ab350(g_VideoMemoryPtr_00449280);
				if (BYTE_00449414 != 0) {
					assert(false);
					//FUN_002aa800();
				}
				g_VideoMemoryPtr_00449280->cmdA = 0x30000000;
				g_VideoMemoryPtr_00449280->cmdB = 0;
				*(undefined4*)&g_VideoMemoryPtr_00449280->cmdB = 0x6000000;
				g_VideoMemoryPtr_00449280 = g_VideoMemoryPtr_00449280 + 1;
				g_VideoMemoryPtr_00449280->cmdA = 0xe10000002;
				g_VideoMemoryPtr_00449280->cmdB = 0;
				*(undefined4*)&g_VideoMemoryPtr_00449280->cmdB = 0;
				*(undefined4*)((ulong)&g_VideoMemoryPtr_00449280->cmdB + 4) = 0x50000002;
				g_VideoMemoryPtr_00449280 = g_VideoMemoryPtr_00449280 + 1;

				g_VideoMemoryPtr_00449280->cmdA = SCE_GIF_SET_TAG(
					1,					// NLOOP
					SCE_GS_TRUE,		// EOP
					SCE_GS_TRUE,		// PRE
					20,					// PRIM
					SCE_GIF_PACKED,		// FLG
					1					// NREG
				);
				g_VideoMemoryPtr_00449280->cmdB = SCE_GIF_PACKED_AD;

				g_VideoMemoryPtr_00449280 = g_VideoMemoryPtr_00449280 + 1;
				g_VideoMemoryPtr_00449280->cmdA = SCE_GS_SET_ALPHA_1(SCE_GS_ALPHA_ZERO, SCE_GS_ALPHA_ZERO, SCE_GS_ALPHA_ZERO, SCE_GS_ALPHA_CS, SCE_GS_ALPHA_AS);
				g_VideoMemoryPtr_00449280->cmdB = SCE_GS_ALPHA_1;

				g_VideoMemoryPtr_00449280 = g_VideoMemoryPtr_00449280 + 1;
				g_VideoMemoryPtr_00449280 = FUN_0029c1b0(g_VideoMemoryPtr_00449280);

				sceDmaAddEnd((sceDmaTag**)&g_VideoMemoryPtr_00449280, 0, 0);
				if (g_RenderCommandBuffer_00449400 != (RenderCommand*)0x0) {
					assert(false);
				//	*DAT_00449400 = ((long)(int)(DAT_00449400 + 8) & 0xfffffffU) << 0x20 | 0x20000000;
				//	DAT_00449400[1] = 0;
				}
			}
			if (ed3D::Params_00424e30.bEnableProfile != 0) {
				assert(false);
				uVar6 = (uint)g_3DFlushProfileObjIndex_004491ec >> 4;
				//if ((g_3DFlushProfileObjIndex_004491ec & 1U) == 0) {
				//	g_ProfileObjB_0041ed44[uVar6].field_0x0 = (long)PCR1 - g_ProfileObjB_0041ed44[uVar6].field_0x0;
				//	g_ProfileObjB_0041ed44[uVar6].field_0x8 =
				//		g_ProfileObjB_0041ed44[uVar6].field_0x8 + g_ProfileObjB_0041ed44[uVar6].field_0x0;
				//	pPVar7 = g_ProfileObjB_0041ed44 + uVar6;
				//	if (pPVar7->field_0x10 < g_ProfileObjB_0041ed44[uVar6].field_0x0) {
				//		pPVar7->field_0x10 = pPVar7->field_0x0;
				//	}
				//}
				//else {
				//	g_ProfileObjA_0041ed40[uVar6].field_0x0 = (long)PCR1 - g_ProfileObjA_0041ed40[uVar6].field_0x0;
				//	g_ProfileObjA_0041ed40[uVar6].field_0x8 =
				//		g_ProfileObjA_0041ed40[uVar6].field_0x8 + g_ProfileObjA_0041ed40[uVar6].field_0x0;
				//	pPVar7 = g_ProfileObjA_0041ed40 + uVar6;
				//	if (pPVar7->field_0x10 < g_ProfileObjA_0041ed40[uVar6].field_0x0) {
				//		pPVar7->field_0x10 = pPVar7->field_0x0;
				//	}
				//}
			}
		}
		return;
	}
	

	MeshTransformParentHeader* AllocateCameraPanHeader_00290af0(int count, EHeap heapID)
	{
		MeshTransformParentHeader* pAllocatedBuffer;

		pAllocatedBuffer = (MeshTransformParentHeader*)edMemAlloc(heapID, count * 0x10 + 0x10);
		if (pAllocatedBuffer == (MeshTransformParentHeader*)0x0) {
			pAllocatedBuffer = (MeshTransformParentHeader*)0x0;
		}
		else {
			Setup_00290b70(pAllocatedBuffer, count);
		}
		return pAllocatedBuffer;
	}

	MeshTransformParentHeader* PTR_MeshTransformParentHeader_004494ac = NULL;
	MeshTransformParentHeader* PTR_MeshTransformParentHeader_004494b0 = NULL;
	MeshTransformParentHeader* PTR_MeshTransformParentHeader_004494b4 = NULL;

	CameraPanMasterHeader g_CameraPanDrawTypeArray_0048c710[16][2] = { 0 };

	void FUN_002b67d0(MeshTransformParentHeader** param_1)
	{
		int iVar1;

		iVar1 = 0;
		do {
			Setup_00290bf0(g_OtherDoubleBuffer_0048cd50 + iVar1, param_1[iVar1], 0, 0);
			Link_00290c10(g_OtherDoubleBuffer_0048cd50 + iVar1);
			iVar1 = iVar1 + 1;
		} while (iVar1 < 2);
		return;
	}

	void InitFunc_002b13b0(int countA, int countB)
	{
		int iVar1;
		int iVar2;

		PTR_MeshTransformParentHeader_004494ac = AllocateCameraPanHeader_00290af0(countB, TO_HEAP(H_MAIN));
		PTR_MeshTransformParentHeader_004494b0 = AllocateCameraPanHeader_00290af0(countA, TO_HEAP(H_MAIN));
		PTR_MeshTransformParentHeader_004494b4 = AllocateCameraPanHeader_00290af0(countA, TO_HEAP(H_MAIN));
		iVar2 = 0;
		do {
			Setup_00290bf0
			((CameraPanMasterHeader*)g_CameraPanDrawTypeArray_0048c710[iVar2], PTR_MeshTransformParentHeader_004494b0,
				0, 0);
			Link_00290c10((CameraPanMasterHeader*)g_CameraPanDrawTypeArray_0048c710[iVar2]);
			Setup_00290bf0
			(g_CameraPanDrawTypeArray_0048c710[iVar2] + 1, PTR_MeshTransformParentHeader_004494b4, 0, 0);
			Link_00290c10(g_CameraPanDrawTypeArray_0048c710[iVar2] + 1);
			iVar2 = iVar2 + 1;
		} while (iVar2 < 0xf);
		FUN_002b67d0(&PTR_MeshTransformParentHeader_004494b0);
		g_CameraFlipIndex_004494a4 = 0;
		g_ModelDoubleBufferIndex_004494a0 = 0;
		iVar2 = 0;
		do {
			iVar1 = iVar2 + 1;
			g_ModelDoubleBuffer_0048c6d0[iVar2].pLoadedData = (MeshTransformSpecial*)(g_ModelDoubleBuffer_0048c6d0 + iVar2);
			g_ModelDoubleBuffer_0048c6d0[iVar2].count_0x14 = 0;
			iVar2 = iVar1;
		} while (iVar1 < 2);
		return;
	}

	CameraPanMasterHeader* CameraPanMasterHeaderA_004491d0 = NULL;
	CameraPanMasterHeader* CameraPanMasterHeaderB_004491d4 = NULL;

	char* g_szed3DAllocInfo_004333d0 = "ed3D Init total allocation ammount:<< %d >>\n";

	byte BYTE_00449344 = 0;

	void SetGIFMode_0029cf60(void)
	{
		if (BYTE_00449344 == 0) {
#ifdef PLATFORM_PS2
			DPUT_GIF_MODE(4);
#endif
			SYNC(0);
			BYTE_00449344 = 1;
		}
		return;
	}


	void InitFunc_002abd00(void)
	{
		PTR_MeshTransformData_ARRAY_0044942c =
			(MeshTransformData*)edMemAlloc(TO_HEAP(H_MAIN), Params_00424e30.meshTransformDataCount * 0xd0);
		PTR_MeshTransformParentHeader_00449430 =
			AllocateCameraPanHeader_00290af0(Params_00424e30.meshTransformDataCount, TO_HEAP(H_MAIN));
		PTR_MeshTransformParent_ARRAY_00449434 = (MeshTransformParent*)PTR_MeshTransformParentHeader_00449430->field_0x4;
		g_StaticMeshMasterHeap_00449438 = edMemAlloc(TO_HEAP(H_MAIN), Params_00424e30.initialStaticMeshMasterCount * 0x60);
		return;
	}

	void Init(void)
	{
		TextureInfoSmall* pTVar1;
		uint uVar2;
		int size;

		PrintString(s_ed3D_Initialsation_004333a0);
		InitVidMode_002936b0();
		//DAT_004491b0 = 1;
		CameraPanMasterHeaderA_004491d0 = AllocateCameraAndMesh_00290a10(1, Params_00424e30.meshHeaderCountAA, TO_HEAP(H_MAIN));
		CameraPanMasterHeaderB_004491d4 = AllocateCameraAndMesh_00290a10(1, Params_00424e30.meshHeaderCountBB, TO_HEAP(H_MAIN));
		//PTR_DAT_004491c8 = (undefined*)edMemAlloc(TO_HEAP(H_MAIN), Params_00424e30.meshHeaderCountAA << 6);
		//memset(PTR_DAT_004491c8, 0, Params_00424e30.meshHeaderCountAA << 6);
		TexturePool_004491cc = (TextureInfoSmall*)edMemAlloc(TO_HEAP(H_MAIN), Params_00424e30.meshHeaderCountBB * sizeof(TextureInfoSmall));
		memset(TexturePool_004491cc, 0, Params_00424e30.meshHeaderCountBB * 0x30);
		//pTVar1 = TexturePool_004491cc;
		//INT_004491b4 = 0;
		LoadedTextureCount_004491c4 = 0;
		//*(undefined8*)TexturePool_004491cc = 0x3fff00000000;
		//*(undefined8*)&pTVar1->textureHeaderStart = 0x50;
		//*(undefined8*)&pTVar1->textureBufferStart = 0x7ffe000000000;
		//memset(&DAT_0048c3e0, 0, 0x200);
		cameraPanStaticMasterArray_00449378 = (StaticMeshMaster*)edMemAlloc(TO_HEAP(H_MAIN), Params_00424e30.initialStaticMeshMasterCount * 0x1c0 + 1);
		memset(cameraPanStaticMasterArray_00449378, 0, Params_00424e30.initialStaticMeshMasterCount * sizeof(StaticMeshMaster));
		PTR_MeshTransformParentHeader_0044937c = AllocateCameraPanHeader_00290af0(Params_00424e30.field_0x8 + Params_00424e30.initialStaticMeshMasterCount * 3, TO_HEAP(H_MAIN));
		//PTR_DAT_00449380 = (undefined*)edMemAlloc(TO_HEAP(H_MAIN), Params_00424e30.field_0x8 * 0x30);
		//memset(PTR_DAT_00449380, 0, Params_00424e30.field_0x8 * 0x30);
		for (uVar2 = 0; uVar2 < Params_00424e30.initialStaticMeshMasterCount; uVar2 = uVar2 + 1) {
			Setup_00290bf0(&cameraPanStaticMasterArray_00449378[uVar2].field_0x30, PTR_MeshTransformParentHeader_0044937c, 0, 0);
			Setup_00290bf0(&cameraPanStaticMasterArray_00449378[uVar2].field_0x10, PTR_MeshTransformParentHeader_0044937c, 0, 0);
		}
		/* Add render handler */
		edSysHandlersAdd
		(edSysHandlerVideo_0048cee0.nodeParent, edSysHandlerVideo_0048cee0.entries,
			edSysHandlerVideo_0048cee0.maxEventID, ESHT_RenderScene, RenderScene, 1, 1);
		edSysHandlersAdd
		(edSysHandlerVideo_0048cee0.nodeParent, edSysHandlerVideo_0048cee0.entries,
			edSysHandlerVideo_0048cee0.maxEventID, ESHT_Sound, EdSoundHandler_002b2730, 1, 0);
		if (Params_00424e30.meshHeaderCountBAlt < 0x180) {
			Params_00424e30.meshHeaderCountBAlt = Params_00424e30.meshHeaderCountBAlt + 0x180;
		}
		LoadedTextureDataBase_004491d8 = (ulong*)edMemAlloc(TO_HEAP(H_MAIN), Params_00424e30.meshHeaderCountBAlt);
		//PTR_DAT_004491e4 = PTR_PTR_DAT_004491d8 + Params_00424e30.meshHeaderCountBAlt;
		size = (Params_00424e30.field_0x28 * 2 + Params_00424e30.field_0x18 + 0x8c0 + Params_00424e30.meshHeaderCountA * 0x38 + Params_00424e30.field_0x34 * 8) * 0x10;
		LoadedTextureData_004491e0 = LoadedTextureDataBase_004491d8;
		ed3D_Allocator_00449248.base = (char*)edMemAlloc(TO_HEAP(H_MAIN), size);
		memset(ed3D_Allocator_00449248.base, 0, size);
		ed3D_Allocator_00449248.current = ed3D_Allocator_00449248.base + size;
		ed3D_Allocator_00449248.end = ed3D_Allocator_00449248.base;
		//PTR_DAT_004491b8 = (undefined*)FUN_0029ce60(16000);
		//PTR_DAT_004491c0 = PTR_DAT_004491b8 + 16000;
		//PTR_DAT_004491bc = PTR_DAT_004491b8;
		//FUN_002b12a0(Params_00424e30.field_0x14, Params_00424e30.meshHeaderCountA);
		//FUN_002ad860();
		InitFunc_002b13b0(Params_00424e30.field_0x14, Params_00424e30.meshHeaderCountB);
		FUN_0029c220(Params_00424e30.field_0x18 + Params_00424e30.field_0x34 * 8, Params_00424e30.field_0x28);
		//thunk_FUN_002ab6d0();
		SetGIFMode_0029cf60();
		//if (DAT_004491f8 == '\0') {
		//	DAT_004491f8 = '\x01';
		//}
		g_RenderActiveCameraObj_006f7d80 = (CameraObj_28*)0x0;
		//FUN_0029ced0();
		ResetMemoryPtr_0029c060();
		//FUN_002b1140();
		//RenderSetupFunc_002b1220();
		//FUN_002b6b20();
		InitFunc_002abd00();
		//FUN_0029ffd0(&DAT_0048c380, 1);
		//FUN_0029ffd0(&DAT_0048c370, 1);
		//FUN_0029ffd0(&DAT_0048c3a0, 1);
		//FUN_0029ffd0(&DAT_0048c390, 1);
		//FUN_0029ffd0(&DAT_0048c3c0, 1);
		//FUN_0029ffd0(&DAT_0048c3b0, 1);
		//DAT_0044897c = 0xffffffff;
		//PTR_DAT_004491dc = PTR_DAT_004491e0;
		if (Params_00424e30.bEnableProfile == 1) {
			assert(false);
		//	g_3DRendProfileObjIndex_004491e8 = CreateNamedProfileObj_00281340(1, 0, 0, 0x80, s_3DRend_004333b8);
		//	g_3DFlushProfileObjIndex_004491ec = CreateNamedProfileObj_00281340(1, 0x80, 0, 0, s_3DFlush_004333c0);
		}
		PrintString(g_szed3DAllocInfo_004333d0,
			Params_00424e30.meshHeaderCountA * 0x220 +
			Params_00424e30.field_0x18 * 0x10 +
			Params_00424e30.meshHeaderCountBAlt +
			Params_00424e30.meshHeaderCountAA * 0x40 + Params_00424e30.meshHeaderCountBB * 0x30 +
			Params_00424e30.field_0x14 * 0x60);
		//return;
	}

	char* VerifyFileBufferRead(char* pBuffStart, char* pBuffEnd)
	{
		/* Checks that the end of the file is greater than the start of the file */
		if ((pBuffEnd != (char*)0x0) && (pBuffEnd <= pBuffStart)) {
			pBuffStart = (char*)0x0;
		}
		return pBuffStart;
	}

	int* GetNextFileSection(char* param_1, int* param_2)
	{
		int* piVar1;

		if ((param_2 == (int*)0x0) || (piVar1 = (int*)0x0, (int*)(param_1 + *(int*)(param_1 + 8)) < param_2)) {
			piVar1 = (int*)(param_1 + *(int*)(param_1 + 8));
		}
		return piVar1;
	}

	void LoadTextureMajorSections(TextureInfoSmall* textureInfoObj, char* fileBuffer, int length)
	{
		int* piVar1;
		int* nextSection;
		int readValue;

		/* Seek through the buffer */
		for (piVar1 = (int*)VerifyFileBufferRead(fileBuffer, fileBuffer + length); piVar1 != (int*)0x0; piVar1 = GetNextFileSection((char*)piVar1, (int*)(fileBuffer + length))) {
			readValue = *piVar1;
			/* Check the first value in the buffer is *2D* */
			if (readValue == 0x2a44322a) {
				/* Set the readValue to be the last section of the texture header */
				readValue = piVar1[2];
				textureInfoObj->textureHeaderStart = (char*)piVar1;
				for (nextSection = (int*)VerifyFileBufferRead((char*)(piVar1 + 4), (char*)(int*)((char*)piVar1 + readValue)); nextSection != (int*)0x0;
					nextSection = GetNextFileSection((char*)nextSection, (int*)((char*)piVar1 + readValue))) {
					/* Check if the value in the buffer is 'MATA' */
					if (*nextSection == 0x4154414d) {
						textureInfoObj->materialBufferStart = (char*)(nextSection + 4);
					}
				}
			}
			else {
				/* Check if the value in the buffer is 'ANMA' */
				if (readValue == 0x414d4e41) {
					textureInfoObj->animationBufferStart = (char*)piVar1;
				}
				else {
					/* Check if the value in the buffer is 'PALL' */
					if (readValue == 0x4c4c4150) {
						textureInfoObj->palleteBufferStart = (char*)piVar1;
					}
					else {
						/* Check if the value in the buffer is 'T2DA' */
						if (readValue == 0x41443254) {
							textureInfoObj->textureBufferStart = (char*)piVar1;
						}
					}
				}
			}
		}
		return;
	}

	int* SetTextureSubsectionPointers(char* fileBuffer, int length, char* fileBufferCopy, int lengthCopy)
	{
		int iVar1;
		int* piVar2;
		int* nextSectionBuffer;
		int* nextSubSection;
		uint uVar3;
		uint uVar4;
		char* actualFileStart;
		char* pcVar5;
		int subSectionValue;

		pcVar5 = fileBuffer + (-lengthCopy - (long long)fileBufferCopy);
		actualFileStart = fileBufferCopy + -0x10;
		if (fileBuffer == fileBufferCopy) {
			pcVar5 = (char*)0x0;
		}
		nextSectionBuffer = (int*)VerifyFileBufferRead(fileBuffer, fileBuffer + length);
		while (true) {
			if (nextSectionBuffer == (int*)0x0) {
				return (int*)0x0;
			}
			/* Check if the value in the buffer is 'REAA'
			   If it is, and the next value isn't 1974, exit the loop (we reached the end of the file) */
			if ((*nextSectionBuffer == 0x41414552) && (*(short*)(nextSectionBuffer + 1) != 0x7b6)) break;
			nextSectionBuffer = GetNextFileSection((char*)nextSectionBuffer, (int*)(fileBuffer + length));
		}
		/* Jump ahead in the buffer */
		iVar1 = nextSectionBuffer[2];
		/* Mark the REAA buffer with 1974 */
		*(undefined2*)(nextSectionBuffer + 1) = 0x7b6;
		piVar2 = (int*)VerifyFileBufferRead((char*)(nextSectionBuffer + 4), (char*)(int*)((char*)nextSectionBuffer + iVar1));
		pcVar5 = actualFileStart + (int)pcVar5;
		for (; piVar2 != (int*)0x0; piVar2 = GetNextFileSection((char*)piVar2, (int*)((char*)nextSectionBuffer + iVar1))) {
			subSectionValue = *piVar2;
			nextSubSection = piVar2 + 4;
			uVar3 = piVar2[2] - 0x10U >> 2;
			/* Check if the value in the buffer is 'REAL' */
			if (subSectionValue == 0x4c414552) {
				for (uVar4 = 0; uVar4 < (uVar3 & 0xffff); uVar4 = uVar4 + 1 & 0xffff) {
					/* Jump forward one section */
					if (*nextSubSection != 0) {
						/* Store a pointer to the next sub section */
						int offset = (int)*(char**)(actualFileStart + *nextSubSection);
						*(int*)(actualFileStart + *nextSubSection) = (int)STORE_SECTION(actualFileStart + offset);
					}
					nextSubSection = nextSubSection + 1;
				}
			}
			else {
				/* Is current value RSTR */
				if (subSectionValue == 0x52545352) {
					for (uVar4 = 0; uVar4 < (uVar3 & 0xffff); uVar4 = uVar4 + 1 & 0xffff) {
						if (*nextSubSection != 0) {
							/* Store a pointer to the next sub section */
							*(int*)(pcVar5 + *nextSubSection) = (int)STORE_SECTION(actualFileStart + (int)*(char**)(pcVar5 + *nextSubSection));
						}
						nextSubSection = nextSubSection + 1;
					}
				}
				else {
					/* Is current value ROBJ */
					if (subSectionValue == 0x4a424f52) {
						for (uVar4 = 0; uVar4 < (uVar3 & 0xffff); uVar4 = uVar4 + 1 & 0xffff) {
							if (*nextSubSection != 0) {
								/* Store a pointer to the next sub section */
								*(int*)(actualFileStart + *nextSubSection) = (int)STORE_SECTION(pcVar5 + (int)*(char**)(actualFileStart + *nextSubSection));
							}
							nextSubSection = nextSubSection + 1;
						}
					}
					else {
						/* Is current value RGEO */
						if (subSectionValue == 0x4f454752) {
							for (uVar4 = 0; uVar4 < (uVar3 & 0xffff); uVar4 = uVar4 + 1 & 0xffff) {
								if (*nextSubSection != 0) {
									/* Store a pointer to the next sub section */
									*(int*)(pcVar5 + *nextSubSection) = (int)STORE_SECTION(pcVar5 + (int)*(char**)(pcVar5 + *nextSubSection));
								}
								nextSubSection = nextSubSection + 1;
							}
						}
					}
				}
			}
		}
		return nextSectionBuffer;
	}

	void TextureSetup(TextureInfoSmall* textureInfoObj, ulong mode)
	{
		char* pcVar1;
		int iVar2;
		int iVar3;
		int* piVar4;
		int* piVar5;

		pcVar1 = textureInfoObj->textureHeaderStart;
		iVar2 = *(int*)(pcVar1 + 8);
		for (piVar4 = (int*)VerifyFileBufferRead(pcVar1 + 0x10, pcVar1 + iVar2); piVar4 != (int*)0x0; piVar4 = GetNextFileSection((char*)piVar4, (int*)(pcVar1 + iVar2))) {
			if (*piVar4 == 0x4154414d) {
				iVar3 = piVar4[2];
				for (piVar5 = (int*)VerifyFileBufferRead((char*)(piVar4 + 4), (char*)(int*)((int)piVar4 + iVar3)); piVar5 != (int*)0x0;
					piVar5 = GetNextFileSection((char*)piVar5, (int*)((int)piVar4 + iVar3))) {
					/* .TAM */
					if (*piVar5 == 0x2e54414d) {
#ifdef PLATFORM_WIN
						assert(false); // untested
#endif
						//GenerateRenderingCommands_0029ffd0((TamPacked*)(piVar5 + 4), mode);
					}
				}
			}
		}
		return;
	}

	void LoadTextureLayer(TextureInfoSmall* textureInfoObj, ulong mode)
	{
		int iVar1;
		ulong* puVar2;
		LayerHeaderPacked* pBuffer;
		char* fileBufferStart;
		char* texHeaderStart;

		/* Can point at 004DA040 */
		puVar2 = ed3D::LoadedTextureData_004491e0;
		texHeaderStart = textureInfoObj->textureHeaderStart;
		iVar1 = *(int*)(texHeaderStart + 8);
		if ((textureInfoObj != (TextureInfoSmall*)0x0) && (fileBufferStart = textureInfoObj->textureFileBufferStart, (*(uint*)(fileBufferStart + 4) & 1) == 0)) {
#ifdef PLATFORM_WIN
			assert(false); // untested
#endif
			for (pBuffer = (LayerHeaderPacked*)VerifyFileBufferRead(texHeaderStart + 0x10, texHeaderStart + iVar1); pBuffer != (LayerHeaderPacked*)0x0;
				pBuffer = (LayerHeaderPacked*)GetNextFileSection((char*)pBuffer, (int*)(texHeaderStart + iVar1))) {
				/* Check if the value in the buffer is 'LAYA' */
				if (*(int*)pBuffer == 0x4159414c) {
					//TextureSetup(pBuffer, textureInfoObj);
				}
			}
			TextureSetup(textureInfoObj, mode);
			textureInfoObj->textureFileLengthB = (int)ed3D::LoadedTextureData_004491e0 + (textureInfoObj->textureFileLengthB - (int)puVar2);
			*(uint*)(fileBufferStart + 4) = *(uint*)(fileBufferStart + 4) | 1;
		}
		return;
	}

	TextureInfoSmall* LoadTextureFromBuffer(char* fileBufferStart, int fileLength, int* outInt, TextureInfoSmall* pTextureInfo, ulong param_5)
	{
		bool bVar1;
		TextureInfoSmall* pOutTexture;
		char* fileBuffer;
		TextureInfoSmall* pTexturePool;

		/* Find some free area in the buffer? */
		pOutTexture = pTextureInfo;
		pTexturePool = TexturePool_004491cc;
		if (pTextureInfo == (TextureInfoSmall*)0x0) {
			while (pOutTexture = pTexturePool, pOutTexture->textureFileBufferStart != (char*)0x0) {
				pTexturePool = pOutTexture + 1;
			}
		}
		if ((pOutTexture == (TextureInfoSmall*)0x0) || (memset(pOutTexture, 0, 0x30), fileBufferStart == (char*)0x0)) {
			bVar1 = pOutTexture != pTextureInfo;
			pOutTexture = (TextureInfoSmall*)0x0;
			if (bVar1) {
				LoadedTextureCount_004491c4 = LoadedTextureCount_004491c4 + -1;
				pOutTexture = (TextureInfoSmall*)0x0;
			}
		}
		else {
			/* Offset the file pointer by 0x10 */
			fileBuffer = fileBufferStart + 0x10;
			LoadTextureMajorSections(pOutTexture, fileBuffer, fileLength + -0x10);
			SetTextureSubsectionPointers(fileBuffer, fileLength + -0x10, fileBuffer, fileLength + -0x10);
			pOutTexture->textureFileBufferStart = fileBufferStart;
			pOutTexture->textureFileLengthA = fileLength;
			pOutTexture->textureFileLengthB = fileLength;
			*outInt = fileLength - pOutTexture->textureFileLengthB;
			LoadTextureLayer(pOutTexture, param_5);
		}
		return pOutTexture;
	}

	char* GetMaterialBufferStart(TextureInfoSmall* pTextureInfo, int count)
	{
		int* pBuffStart;
		int* nextSection;
		uint uVar1;
		int* piVar2;
		int* sectionEnd;

		pBuffStart = (int*)pTextureInfo->materialBufferStart;
		/* Ensure we are trying to read something from the 'MATA' section */
		if (pBuffStart[-4] == 0x4154414d) {
			/* Work out the section end address */
			sectionEnd = (int*)((char*)(pBuffStart + -4) + pBuffStart[-2]);
			for (nextSection = (int*)VerifyFileBufferRead((char*)pBuffStart, (char*)sectionEnd); nextSection != (int*)0x0; nextSection = GetNextFileSection((char*)nextSection, sectionEnd)) {
				/* Check the first value in the buffer is *MAT.* */
				if (*nextSection == 0x2e54414d) {
					if ((count == 0) && (*pBuffStart == 0x48534148)) {
						/* Check the first value in the buffer is *HASH* */
						piVar2 = pBuffStart + 4;
						uVar1 = (uint)((unsigned long long)(pBuffStart + -4) + (pBuffStart[-1] - (unsigned long long)piVar2)) >> 4;
						for (; uVar1 != 0; uVar1 = uVar1 - 1) {
#ifdef PLATFORM_PS2
							if ((int*)piVar2[2] == nextSection) {
#else
							std::optional<void*> value = PointerConv::ResolveTextureSectionKey(piVar2[2]);

							if (value.has_value() && *value == nextSection)
							{
								MY_LOG("Searching key %d = %p\n", piVar2[2], nextSection);
#endif
								return (char*)piVar2;
							}
							piVar2 = piVar2 + 4;
						}
					}
					count = count + -1;
				}
				/* Jump to the next section */
			}
		}
		return (char*)0x0;
	}

	void* GetMaterialInfo(MaterialInfo* outObj, char* materialBuffer, TextureInfoSmall* textureInfoObj, int mode)
	{
		void* pvVar1;
		DisplayList_0x10* someGlobalBuffer;
		uint counter;

		counter = 0;
		pvVar1 = memset(outObj, 0, sizeof(MaterialInfo));
		outObj->matSubsectionStart = (char*)(LOAD_SECTION(*(int*)(materialBuffer + 8))) + 0x10;
		while (true) {
			if ((*(long*)(edDlist::g_DisplayListPtr_0044965c + counter) == 0) || (edDlist::g_DisplayListObjCount_004250e0 - 1U <= counter)) break;
			counter = counter + 1;
		}
		someGlobalBuffer = edDlist::g_DisplayListPtr_0044965c + counter;
		*(undefined8*)someGlobalBuffer = *(undefined8*)materialBuffer;
		someGlobalBuffer->field_0x8 = materialBuffer;
		outObj->Length = counter;
		edDlist::MaterialDisplayListCount_00449648 = edDlist::MaterialDisplayListCount_00449648 + 1;
		outObj->mode = mode;
		outObj->textureInfo = textureInfoObj;
		return pvVar1;
	}

	MaterialInfo* GetMaterialInfoFromTexture(MaterialInfo* outObj, int count, TextureInfoSmall* textureInfoObj, int mode)
	{
		char* materialBuffer;

		materialBuffer = GetMaterialBufferStart(textureInfoObj, count);
		if (materialBuffer == (char*)0x0) {
			outObj = (MaterialInfo*)0x0;
		}
		else {
			GetMaterialInfo(outObj, materialBuffer, textureInfoObj, mode);
		}
		return outObj;
	}
}
