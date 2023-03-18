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

	int g_FrameBufferPtr_004491f0 = 0;

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
	sceDmaTag* g_VideoMemoryPtr_00449280 = NULL;
	sceDmaTag* g_VideoMemoryBasePtr_00449260 = NULL;
	sceDmaTag* g_FlipDMA_00449270 = NULL;
	uint g_VideoMemorySize_00449280 = 0;

#ifdef PLATFORM_PS2
	int DMAC_Ctrl_mode_00448984 = D_CTRL_RELE_O;
#endif

	void FUN_0029bb10(void)
	{
#ifdef PLATFORM_PS2
		DPUT_GIF_MODE(4);
		DPUT_D_CTRL(DMAC_Ctrl_mode_00448984);
		DPUT_D_STAT(D_STAT_CIS0_O);
#endif
		edSysHandlersCall(edSysHandlerVideo_0048cee0.mainIdentifier, edSysHandlerVideo_0048cee0.entries,
			edSysHandlerVideo_0048cee0.maxEventID, 1, (void*)0x0);
		//DAT_004494a0 = g_CameraFlipIndex_004494a4;
		g_CameraFlipIndex_004494a4 = g_CameraFlipIndex_004494a4 != 0 ^ 1;
		edSysHandlersCall(edDlist::sysHandler_0048cb90.mainIdentifier, edDlist::sysHandler_0048cb90.entries,
			edDlist::sysHandler_0048cb90.maxEventID, 1, (void*)0x0);
		//if ((undefined*)(&DAT_0048c6d4)[DAT_004494a0 * 8] != &DAT_0048c6d0 + DAT_004494a0 * 0x20) {
		//	FUN_0029cf90((int)(&DAT_0048c6d0 + DAT_004494a0 * 0x20));
		//}
		//write_volatile_4(REG_DMAC_PCR, UINT_00449240);
		if ((g_FlipDMA_00449270 != (sceDmaTag*)0x0) && (g_FlipDMA_00449270 != g_VideoMemoryPtr_00449280)) {
#ifdef PLATFORM_PS2
			//WaitDMA();
			//shellDmaStartChainB(SHELLDMA_CHANNEL_VIF1, (ShellDmaTag*)g_FlipDMA_00449270);
			//if (DAT_00449244 != '\0') {
			//	WaitForDraw_00258230();
			//	WaitForDraw_00258230();
			//}
#endif
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

		//DAT_0048c2c0 = 0x22;
		//DAT_0048c2cc = 0xc000;
		lVar3 = (ulong)ed3D_Allocator_00449248.end;
		//DAT_0048c2c4 = 0x3e;
		//DAT_0048c2c8 = 0x5a;
		pcVar2 = ed3D_Allocator_00449248.end + param_1 * 0x10;
		if (ed3D_Allocator_00449248.current < ed3D_Allocator_00449248.end + param_1 * 0x10) {
			lVar3 = 0;
			pcVar2 = ed3D_Allocator_00449248.end;
		}
		ed3D_Allocator_00449248.end = pcVar2;
		g_VideoMemoryBasePtr_00449260 = (sceDmaTag*)((ulong)((ulong)(lVar3 << 0x24) >> 0x24) | 0x30000000);
		//DAT_00449290 = g_VideoMemoryBasePtr_00449260 + (param_1 >> 1);
		g_VideoMemorySize_00449280 = param_1;
		g_VideoMemoryPtr_00449280 = g_VideoMemoryBasePtr_00449260;
		//FUN_0029c740();
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
		//DAT_00449400 = 0;
		//DAT_004492d4 = 1;
		//DAT_004492b0 = DAT_004492c0;
		//FUN_002a9350();
		//PTR_DAT_004491bc = PTR_DAT_004491b8;
		g_FlipDMA_00449270 = g_VideoMemoryPtr_00449280;
		return;
	}

	void EdSoundHandler_002b2730(int param_1, int param_2, char* param_3)
	{
		int iVar1;
		uint uVar2;
		//ProfileObject* pPVar3;
		//
		//iVar1 = g_3DRendProfileObjIndex_004491e8;
		//DAT_0044939c = 0;
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
		FUN_0029bb10();
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
				//assert(false);
				//pCVar4 = AllocateCameraPanStatic_002abc80();
				//ed3D::cameraPanStaticMasterArray_00449378[staticMeshMasterIndex].pFinalLink_0x1a4 = pCVar4;
				//pCVar4 = AllocateCameraPanStatic_002abc80();
				//ed3D::cameraPanStaticMasterArray_00449378[staticMeshMasterIndex].field_0x1a8 = pCVar4;
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
			//uVar6 = SetupCameraObj130_0029a960(pInStaticMeshMaster, (ulong)((pInStaticMeshMaster->flags_0x4 & 0x20) != 0));
			//uVar6 = uVar6 & 0xff;
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
				assert(false);
				//DAT_0044939c = DAT_0044939c + 1;
				//if (ed3D::Params_00424e30.bEnableProfile != 0) {
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
				//}
				assert(false);
				//FUN_0029c140();
				//g_VideoMemoryPtr_00449280 = (sceDmaTag*)FUN_002b1990(pInStaticMeshMaster, (undefined8*)g_VideoMemoryPtr_00449280);
				//if (ed3D::Params_00424e30.bEnableProfile != 0) {
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
				//}
				//pCVar8 = (MeshTransformParent*)pStaticMeshMaster->pFinalLink_0x1a4;
				//pCVar9 = pCVar8->pSubMeshParent;
				//if (*ScratchpadAnimManager_004494ec.field_0x28 == 0) {
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
				//local_10 = pStaticMeshMaster;
				//if (DAT_00448a5c == '\0') {
				//	if (pInStaticMeshMaster == pStaticMeshMaster) {
				//		local_c = pInStaticMeshMaster->field_0x0;
				//		local_8 = 2;
				//		edSysHandlersCall(edDlist::sysHandler_0048cb90.mainIdentifier, edDlist::sysHandler_0048cb90.entries,
				//			edDlist::sysHandler_0048cb90.maxEventID, 0, &local_10);
				//	}
				//	else {
				//		local_c = pInStaticMeshMaster->field_0x0;
				//		local_8 = 1;
				//		if (pInStaticMeshMaster != pStaticMeshMaster) {
				//			local_8 = 3;
				//		}
				//		edSysHandlersCall(edDlist::sysHandler_0048cb90.mainIdentifier, edDlist::sysHandler_0048cb90.entries,
				//			edDlist::sysHandler_0048cb90.maxEventID, 0, &local_10);
				//	}
				//}
				//else {
				//	local_c = pInStaticMeshMaster->field_0x0;
				//	local_8 = 1;
				//	if (pInStaticMeshMaster != pStaticMeshMaster) {
				//		local_8 = 3;
				//	}
				//	edSysHandlersCall(edDlist::sysHandler_0048cb90.mainIdentifier, edDlist::sysHandler_0048cb90.entries,
				//		edDlist::sysHandler_0048cb90.maxEventID, 0, &local_10);
				//}
				//if (ed3D::Params_00424e30.bEnableProfile != 0) {
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
				//}
				//if (pInStaticMeshMaster->field_0x0 == 1) {
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
				//}
				//else {
				//	ProcessDrawTypes_0029bc30();
				//}
				//g_VideoMemoryPtr_00449280 =
				//	(sceDmaTag*)FUN_002aabd0((undefined8*)g_VideoMemoryPtr_00449280, (short*)&CameraScreenData_00449390);
				//pCVar2 = pInStaticMeshMaster->pCameraObj28_0xc;
				//CameraScreenData_00449560.posX = pCVar2->posX;
				//CameraScreenData_00449560.posY = pCVar2->posY;
				//CameraScreenData_00449560.screenWidth = pCVar2->screenWidth;
				//CameraScreenData_00449560.screenHeight = pCVar2->screenHeight;
				//g_VideoMemoryPtr_00449280 =
				//	(sceDmaTag*)FUN_002aabd0((undefined8*)g_VideoMemoryPtr_00449280, (short*)&CameraScreenData_00449390);
				//RenderSetupFunc_002b1220();
				//if (ed3D::Params_00424e30.bEnableProfile != 0) {
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
				//}
			}
		}
		return uVar6;
	}

	void RenderScene(int, int, char*)
	{
		CameraObj_28* pCVar1;
		ulong uVar2;
		bool bVar3;
		sceDmaTag* psVar4;
		undefined4 in_zero_lo;
		_sceDmaTag* in_zero_hi;
		uint in_zero_udw;
		uint in_register_0000000c;
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
			//DAT_004492c0 = DAT_004492b0;
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
						assert(false);
						//bVar3 = false;
						//iVar5 = FUN_002ba3b0();
						//if ((iVar5 != 0) && (DAT_00449400 != (ulong*)0x0)) {
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
						//}
					}
					psVar4 = g_VideoMemoryPtr_00449280;
					pInStaticMeshMaster = ed3D::cameraPanStaticMasterArray_00449378 + staticMeshMasterIndex;
					if (g_RenderActiveCameraObj_006f7d80 == (CameraObj_28*)0x0) {
						g_RenderActiveCameraObj_006f7d80 = pInStaticMeshMaster->pCameraObj28_0xc;
					}
					pCVar1 = pInStaticMeshMaster->pCameraObj28_0xc;
					//CameraScreenData_00449390.posX = pCVar1->posX;
					//CameraScreenData_00449390.posY = pCVar1->posY;
					//CameraScreenData_00449390.screenWidth = pCVar1->screenWidth;
					//CameraScreenData_00449390.screenHeight = pCVar1->screenHeight;
					//g_FrontendStaticMeshMaster_00449558 = pInStaticMeshMaster;
					CallMeshRenderFunc_002b2b70(pInStaticMeshMaster, pInStaticMeshMaster);
					for (pCVar10 = (CameraPanMasterHeader*)(pInStaticMeshMaster->field_0x10).pLoadedData;
						pCVar10 != &pInStaticMeshMaster->field_0x10; pCVar10 = (CameraPanMasterHeader*)pCVar10->pLoadedData) {
						if ((((StaticMeshMaster*)pCVar10->pCameraPanHeader_0xc)->flags_0x4 & 4) == 0) {
							assert(false);
							//CallMeshRenderFunc_002b2b70((StaticMeshMaster*)pCVar10->pCameraPanHeader_0xc, pInStaticMeshMaster);
						}
					}
					//if (DAT_00448a5c != '\0') {
					//	StaticMeshMaster::LaunchRenderTask_002b39d0(pInStaticMeshMaster);
					//}
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
						assert(false);
						//if ((pInStaticMeshMaster->flags_0x4 & 0x40) != 0) {
						//	bVar3 = true;
						//	FUN_002aeb40();
						//}
						//if ((pInStaticMeshMaster->flags_0x4 & 0x800) == 0) {
						//	if ((pInStaticMeshMaster->flags_0x4 & 0x80) != 0) {
						//		bVar3 = true;
						//		g_VideoMemoryPtr_00449280 = (sceDmaTag*)FUN_002b8640((ulong*)g_VideoMemoryPtr_00449280);
						//	}
						//}
						//else {
						//	bVar3 = true;
						//	g_VideoMemoryPtr_00449280 = (sceDmaTag*)FUN_002b8030((ulong*)g_VideoMemoryPtr_00449280);
						//}
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
			if ((g_FlipDMA_00449270 != (sceDmaTag*)0x0) && (g_FlipDMA_00449270 != g_VideoMemoryPtr_00449280)) {
				assert(false);
				//g_VideoMemoryPtr_00449280 = (sceDmaTag*)FUN_002ab350((ulong*)g_VideoMemoryPtr_00449280);
				//if (DAT_00449414 != '\0') {
				//	FUN_002aa800();
				//}
				//*(undefined8*)g_VideoMemoryPtr_00449280 = 0x30000000;
				//*(undefined8*)g_VideoMemoryPtr_00449280->p = 0;
				//g_VideoMemoryPtr_00449280->p[0] = 0x6000000;
				//g_VideoMemoryPtr_00449280 = g_VideoMemoryPtr_00449280 + 1;
				//*(undefined8*)g_VideoMemoryPtr_00449280 = 0xe10000002;
				//*(undefined8*)g_VideoMemoryPtr_00449280->p = 0;
				//g_VideoMemoryPtr_00449280->p[0] = 0;
				//g_VideoMemoryPtr_00449280->p[1] = 0x50000002;
				//g_VideoMemoryPtr_00449280 = g_VideoMemoryPtr_00449280 + 1;
				//*(undefined8*)g_VideoMemoryPtr_00449280 = 0x100a400000008001;
				//*(undefined8*)g_VideoMemoryPtr_00449280->p = 0xe;
				//g_VideoMemoryPtr_00449280 = g_VideoMemoryPtr_00449280 + 1;
				//*(undefined8*)g_VideoMemoryPtr_00449280 = 0x2a;
				//*(undefined8*)g_VideoMemoryPtr_00449280->p = 0x42;
				//g_VideoMemoryPtr_00449280 = g_VideoMemoryPtr_00449280 + 1;
				//g_VideoMemoryPtr_00449280 = (sceDmaTag*)FUN_0029c1b0((undefined8*)g_VideoMemoryPtr_00449280);
				//*(undefined8*)g_VideoMemoryPtr_00449280 = 0x70000000;
				//*(undefined8*)g_VideoMemoryPtr_00449280->p = 0;
				//g_VideoMemoryPtr_00449280 = g_VideoMemoryPtr_00449280 + 1;
				//if (DAT_00449400 != (ulong*)0x0) {
				//	*DAT_00449400 = ((long)(int)(DAT_00449400 + 8) & 0xfffffffU) << 0x20 | 0x20000000;
				//	DAT_00449400[1] = 0;
				//}
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

	void Init(void)
	{
		TextureInfoSmall* pTVar1;
		uint uVar2;
		int size;

		PrintString(s_ed3D_Initialsation_004333a0);
		InitVidMode_002936b0();
		//DAT_004491b0 = 1;
		//CameraPanMasterHeaderA_004491d0 = AllocateCameraAndMesh_00290a10(H_MAIN, Params_00424e30.meshHeaderCountAA, 1);
		//CameraPanMasterHeaderB_004491d4 = AllocateCameraAndMesh_00290a10(H_MAIN, Params_00424e30.meshHeaderCountBB, 1);
		//PTR_DAT_004491c8 = (undefined*)edMemAlloc(H_MAIN, Params_00424e30.meshHeaderCountAA << 6);
		//memset(PTR_DAT_004491c8, 0, Params_00424e30.meshHeaderCountAA << 6);
		TexturePool_004491cc = (TextureInfoSmall*)edMemAlloc(H_MAIN, Params_00424e30.meshHeaderCountBB * sizeof(TextureInfoSmall));
		memset(TexturePool_004491cc, 0, Params_00424e30.meshHeaderCountBB * 0x30);
		//pTVar1 = TexturePool_004491cc;
		//INT_004491b4 = 0;
		LoadedTextureCount_004491c4 = 0;
		//*(undefined8*)TexturePool_004491cc = 0x3fff00000000;
		//*(undefined8*)&pTVar1->textureHeaderStart = 0x50;
		//*(undefined8*)&pTVar1->textureBufferStart = 0x7ffe000000000;
		//memset(&DAT_0048c3e0, 0, 0x200);
		cameraPanStaticMasterArray_00449378 = (StaticMeshMaster*)edMemAlloc(H_MAIN, Params_00424e30.initialStaticMeshMasterCount * 0x1c0 + 1);
		memset(cameraPanStaticMasterArray_00449378, 0, Params_00424e30.initialStaticMeshMasterCount * sizeof(StaticMeshMaster));
		PTR_MeshTransformParentHeader_0044937c = AllocateCameraPanHeader_00290af0(Params_00424e30.field_0x8 + Params_00424e30.initialStaticMeshMasterCount * 3, H_MAIN);
		//PTR_DAT_00449380 = (undefined*)edMemAlloc(H_MAIN, Params_00424e30.field_0x8 * 0x30);
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
		//if (Params_00424e30.meshHeaderCountBAlt < 0x180) {
		//	Params_00424e30.meshHeaderCountBAlt = Params_00424e30.meshHeaderCountBAlt + 0x180;
		//}
		LoadedTextureDataBase_004491d8 = (ulong*)edMemAlloc(H_MAIN, Params_00424e30.meshHeaderCountBAlt);
		//PTR_DAT_004491e4 = PTR_PTR_DAT_004491d8 + Params_00424e30.meshHeaderCountBAlt;
		size = (Params_00424e30.field_0x28 * 2 + Params_00424e30.field_0x18 + 0x8c0 + Params_00424e30.meshHeaderCountA * 0x38 + Params_00424e30.field_0x34 * 8) * 0x10;
		LoadedTextureData_004491e0 = LoadedTextureDataBase_004491d8;
		ed3D_Allocator_00449248.base = (char*)edMemAlloc(H_MAIN, size);
		memset(ed3D_Allocator_00449248.base, 0, size);
		ed3D_Allocator_00449248.current = ed3D_Allocator_00449248.base + size;
		ed3D_Allocator_00449248.end = ed3D_Allocator_00449248.base;
		//PTR_DAT_004491b8 = (undefined*)FUN_0029ce60(16000);
		//PTR_DAT_004491c0 = PTR_DAT_004491b8 + 16000;
		//PTR_DAT_004491bc = PTR_DAT_004491b8;
		//FUN_002b12a0(Params_00424e30.field_0x14, Params_00424e30.meshHeaderCountA);
		//FUN_002ad860();
		//InitFunc_002b13b0(Params_00424e30.field_0x14, Params_00424e30.meshHeaderCountB);
		FUN_0029c220(Params_00424e30.field_0x18 + Params_00424e30.field_0x34 * 8, Params_00424e30.field_0x28);
		//thunk_FUN_002ab6d0();
		//FUN_0029cf60();
		//if (DAT_004491f8 == '\0') {
		//	DAT_004491f8 = '\x01';
		//}
		//g_RenderActiveCameraObj_006f7d80 = (CameraObj_28*)0x0;
		//FUN_0029ced0();
		ResetMemoryPtr_0029c060();
		//FUN_002b1140();
		//RenderSetupFunc_002b1220();
		//FUN_002b6b20();
		//InitFunc_002abd00();
		//FUN_0029ffd0(&DAT_0048c380, 1);
		//FUN_0029ffd0(&DAT_0048c370, 1);
		//FUN_0029ffd0(&DAT_0048c3a0, 1);
		//FUN_0029ffd0(&DAT_0048c390, 1);
		//FUN_0029ffd0(&DAT_0048c3c0, 1);
		//FUN_0029ffd0(&DAT_0048c3b0, 1);
		//DAT_0044897c = 0xffffffff;
		//PTR_DAT_004491dc = PTR_DAT_004491e0;
		//if (Params_00424e30.bEnableProfile == 1) {
		//	g_3DRendProfileObjIndex_004491e8 = CreateNamedProfileObj_00281340(1, 0, 0, 0x80, s_3DRend_004333b8);
		//	g_3DFlushProfileObjIndex_004491ec = CreateNamedProfileObj_00281340(1, 0x80, 0, 0, s_3DFlush_004333c0);
		//}
		//PrintString(s_ed3D_Init_total_allocation_ammou_004333d0,
		//	Params_00424e30.meshHeaderCountA * 0x220 +
		//	Params_00424e30.field_0x18 * 0x10 + Params_00424e30.meshHeaderCountBAlt + Params_00424e30.meshHeaderCountAA * 0x40 + Params_00424e30.meshHeaderCountBB * 0x30 +
		//	Params_00424e30.field_0x14 * 0x60);
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
