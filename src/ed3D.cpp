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
#include "Types.h"
#include "MathOps.h"
#include "CameraViewManager.h"

#include <math.h>
#include "FileManager3D.h"

#include "Rendering/CameraPanMasterHeader.h"

char* s_ed3D_Initialsation_004333a0 = "ed3D Initialsation\n";

ed3D_Params::ed3D_Params()
	: meshHeaderCountB(0x3E0)
	, staticMeshMasterCount(0xF)
	, meshDisplayListInternalCount(0x10)
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

Matrix SomeIdentityMatrix = {
	0.6954f, 0.0f, 0.0f, 0.0f,
	0.0f, 0.6954, 0.0f, 0.0f,
	0.0f, 0.0f, 0.6954, 0.0f,
	0.0f, 0.0f, 0.0f, 1.0f
};

namespace ed3D
{
	TextureInfoSmall* TexturePool_004491cc;
	int LoadedTextureCount_004491c4;
	
	RenderCommand* g_MeshCommandBufferCurrent_004491e0;
	RenderCommand* g_MeshCommandBufferBase_004491d8;

	ed3D_Params Params_00424e30;

	FrameBuffer* PTR_VidModeData_30_004491f4 = NULL;

	StaticMeshMaster* cameraPanStaticMasterArray_00449378 = NULL;
	MeshTransformParentHeader* PTR_MeshTransformParentHeader_0044937c = NULL;

	RenderCommand* g_RenderCommand_004492c0 = NULL;
	RenderCommand* g_RenderCommand_004492b0 = NULL;

	int g_FrameBufferPtr_004491f0 = 0;

	CameraPanMasterHeader g_ModelDoubleBuffer_0048c6d0[2] = { 0 };
	CameraPanMasterHeader g_OtherDoubleBuffer_0048cd50[2] = { 0 };

	struct AnimFuncInputData_34 {
		float field_0x0;
		float field_0x4;
		float field_0x8;
		float field_0xc;
		float field_0x10;
		float field_0x14;
		float field_0x18;
		float field_0x1c;
		char* pCDQU;
		char* pCDQU_End;
		ushort field_0x28;
		undefined field_0x2a;
		undefined field_0x2b;
		float field_0x2c;
		float field_0x30;
	};

	PACK(
		struct MeshData_CDQU {
		char field_0x0[4];
		undefined field_0x4;
		undefined field_0x5;
		undefined field_0x6;
		undefined field_0x7;
		undefined field_0x8;
		undefined field_0x9;
		undefined field_0xa;
		undefined field_0xb;
		undefined field_0xc;
		undefined field_0xd;
		undefined field_0xe;
		undefined field_0xf;
		ushort field_0x10[5];
		ushort field_0x1a;
		ushort field_0x1c;
		undefined2 field_0x1e;
		int field_0x20; // int*
		int field_0x24; // int*
		int field_0x28; // int*
		int field_0x2c; // int*
		int field_0x30; // int*
		int field_0x34; // int*
		int field_0x38; // uint*
		int field_0x3c; // uint*
		int pMBNK; // char*
		undefined field_0x44;
		undefined field_0x45;
		undefined field_0x46;
		undefined field_0x47;
		int field_0x48;
	});

	PACK(
	struct TextureData_MYSTERY {
		Vector3 field_0x0;
		ushort field_0xc;
		undefined field_0xe;
		undefined field_0xf;
	}
	);

	PACK(
	struct RenderInput_40 {
		uint flags_0x0;
		short field_0x4;
		short field_0x6;
		int offsetA;
		int pNext; // RenderInput_40*
		struct Vector vector_0x10;
		int field_0x20; // char*
		int field_0x24; // char*
		int field_0x28; // char*
		int field_0x2c; // char*
		short bUseShadowMatrix_0x30;
		undefined field_0x32;
		undefined field_0x33;
		int pRenderFrame30; // RenderFrame_30*
		byte field_0x38;
		byte cameraPanIndex;
		short meshSectionCount_0x3a;
		int pTextureDataMystery; // TextureData_MYSTERY*
	});

	MeshTransformDataBase* g_RenderMeshTransform_004493f4 = NULL;
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

	int INT_004491b4 = 0;

#ifdef PLATFORM_WIN
	uint g_StaticVideoList_00489590[0xB20] = { 0 };
	ulong* g_StaticVideoList_00489590_LONG = (ulong*)g_StaticVideoList_00489590;
#else
	volatile uint* g_StaticVideoList_00489590 = (uint*)0x00489590;
	volatile ulong* g_StaticVideoList_00489590_LONG = (ulong*)0x00489590;
#endif

	MeshTransformParentHeader* PTR_MeshTransformParentHeader_004494ac = NULL;
	MeshTransformParentHeader* PTR_MeshTransformParentHeader_004494b0 = NULL;
	MeshTransformParentHeader* PTR_MeshTransformParentHeader_004494b4 = NULL;
	MeshTransformParentHeader* PTR_MeshTransformParentHeader_004494b8 = NULL;
	MeshTransformParentHeader* PTR_MeshTransformParentHeader_004494bc = NULL;

	CameraPanMasterHeader g_CameraPanDrawTypeArray_0048c710[15][2] = { 0 };

	int g_3DRendProfileObjIndex_004491e8 = 0;
	int g_3DFlushProfileObjIndex_004491ec = 0;
	byte BYTE_00448a64 = 0;
	byte BYTE_00449424 = 0;
	CameraObj_28* g_RenderActiveCameraObj_006f7d80 = NULL;
	int INT_004493a0 = 0;

	uint g_ZBufferFrameBasePtr_0044945c = 0;

	PACK(
	struct TextureData_MAT_Internal_A {
		byte count_0x0;
		undefined field_0x1;
		ushort field_0x2;
		int pRenderFrame30; // RenderFrame_30*
		int pCommandBuffer_0x8; // RenderCommand*
		undefined4 field_0xc;
	});


	PACK(
		struct TextureData_LAY_Internal {
		uint field_0x0;
		uint field_0x4;
		undefined field_0x8;
		undefined field_0x9;
		undefined field_0xa;
		undefined field_0xb;
		undefined field_0xc;
		undefined field_0xd;
		undefined field_0xe;
		undefined field_0xf;
		undefined field_0x10;
		undefined field_0x11;
		undefined field_0x12;
		undefined field_0x13;
		undefined field_0x14;
		undefined field_0x15;
		undefined field_0x16;
		undefined field_0x17;
		undefined field_0x18;
		undefined field_0x19;
		undefined field_0x1a;
		undefined field_0x1b;
		short field_0x1c;
		ushort field_0x1e;
		int pTex; // TextureData_TEX*
	});

	PACK(
	struct TextureData_LAY {
		undefined field_0x0;
		undefined field_0x1;
		undefined field_0x2;
		undefined field_0x3;
		uint field_0x4;
		undefined field_0x8;
		undefined field_0x9;
		undefined field_0xa;
		undefined field_0xb;
		undefined field_0xc;
		undefined field_0xd;
		undefined field_0xe;
		undefined field_0xf;
		TextureData_LAY_Internal body;
	});

	PACK(
	struct TextureData_TEX_Internal {
		undefined field_0x0;
		undefined field_0x1;
		undefined field_0x2;
		undefined field_0x3;
		undefined field_0x4;
		undefined field_0x5;
		undefined field_0x6;
		undefined field_0x7;
		int field_0x8;
		undefined field_0xc;
		undefined field_0xd;
		undefined field_0xe;
		undefined field_0xf;
		int field_0x10;
		undefined field_0x14;
		undefined field_0x15;
		undefined field_0x16;
		undefined field_0x17;
		float field_0x18;
		undefined field_0x1c;
		undefined field_0x1d;
		undefined field_0x1e;
		undefined field_0x1f;
	});

	PACK(
	struct TextureData_TEX {
		undefined field_0x0;
		undefined field_0x1;
		undefined field_0x2;
		undefined field_0x3;
		undefined field_0x4;
		undefined field_0x5;
		undefined field_0x6;
		undefined field_0x7;
		undefined field_0x8;
		undefined field_0x9;
		undefined field_0xa;
		undefined field_0xb;
		undefined field_0xc;
		undefined field_0xd;
		undefined field_0xe;
		undefined field_0xf;
		TextureData_TEX_Internal body;
	});

	TextureData_MAT_Internal_A TextureData_MAT_Internal_A_0048c3b0 = { 0 };
	TextureData_MAT_Internal_A TextureData_MAT_Internal_A_0048c3c0 = { 0 };
	
	int GetStaticMeshMasterIndex(StaticMeshMaster* pStaticMeshMaster)
	{
		int staticMeshMasterIndex = 0;
		/* Find inactive camera master. */
		while (staticMeshMasterIndex < (uint)Params_00424e30.staticMeshMasterCount) {
			if (&cameraPanStaticMasterArray_00449378[staticMeshMasterIndex] == pStaticMeshMaster)
			{
				return staticMeshMasterIndex;
			}
			staticMeshMasterIndex = staticMeshMasterIndex + 1;
		}
		return -1;
	}

	struct RenderTaskData {
		struct StaticMeshMaster* pStaticMeshMaster;
		int isChild;
		ushort taskID;
	};

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

	uint g_StaticDisplayListData_0048c340[12] = { 0 };
	ulong* g_StaticDisplayListData_0048c340_LONG = (ulong*)&g_StaticDisplayListData_0048c340;

	Vector Vector_00424ff0 = { 0 };

	int INT_004489a4 = 0;
	int INT_004489a8 = 0;

	uint UINT_004492d8 = 0;
	uint UINT_ARRAY_004492dc[3] = { 0 };
	byte BYTE_004493b4 = 0;
	byte BYTE_004493b8 = 0;

	RenderCommand RenderCommand_ARRAY_0048c5f0[3] = {0};

	void FUN_002ab6f0(void)
	{
		RenderCommand_ARRAY_0048c5f0[0].cmdB = (ulong)UINT_ARRAY_004492dc[0] << 0x20 | 0x7c01000000000000;
		RenderCommand_ARRAY_0048c5f0[1].cmdB = (ulong)UINT_ARRAY_004492dc[1] << 0x20 | 0x7c01000000000000;
		RenderCommand_ARRAY_0048c5f0[2].cmdB = (ulong)UINT_ARRAY_004492dc[2] << 0x20 | 0x7c01000000000000;
		RenderCommand_ARRAY_0048c5f0[0].cmdA = 0xe10000001;
		RenderCommand_ARRAY_0048c5f0[1].cmdA = 0xe10000001;
		RenderCommand_ARRAY_0048c5f0[2].cmdA = 0xe10000001;
		return;
	}

	void InitStaticVideoList_0029c740(void)
	{
		int iVar1;
		ulong uVar2;
		ulong uVar3;

		uVar2 = 0;
		do {
			iVar1 = (int)uVar2;
			uVar3 = uVar2 & 0xffffffff;
			uVar2 = (iVar1 + 1);
			g_StaticVideoList_00489590_LONG[iVar1 * 2 + 0x2a] = uVar3 | 0x302e400000008000;
			g_StaticVideoList_00489590_LONG[iVar1 * 2 + 0x2b] = 0x512;
			g_StaticVideoList_00489590_LONG[iVar1 * 2 + 0xec] = uVar3 | 0x3026400000008000;
			g_StaticVideoList_00489590_LONG[iVar1 * 2 + 0xed] = 0x512;
			g_StaticVideoList_00489590_LONG[iVar1 * 2 + 0x1ae] = uVar3 | 0x3022400000008000;
			g_StaticVideoList_00489590_LONG[iVar1 * 2 + 0x1af] = 0x512;
		} while (uVar2 < 0x61);
		g_StaticVideoList_00489590_LONG[624] = 0x302ec00000008000;
		g_StaticVideoList_00489590_LONG[626] = 0x3026c00000008000;
		g_StaticVideoList_00489590_LONG[625] = 0x512;
		uVar2 = 0;
		g_StaticVideoList_00489590_LONG[628] = 0x3022c00000008000;
		g_StaticVideoList_00489590_LONG[627] = 0x512;
		g_StaticVideoList_00489590_LONG[629] = 0x512;
		do {
			iVar1 = (int)uVar2;
			uVar3 = uVar2 & 0xffffffff;
			uVar2 = (iVar1 + 1);
			g_StaticVideoList_00489590_LONG[iVar1 * 2 + 0x276] = uVar3 | 0x302d400000008000;
			g_StaticVideoList_00489590_LONG[iVar1 * 2 + 0x277] = 0x512;
			g_StaticVideoList_00489590_LONG[iVar1 * 2 + 0x338] = uVar3 | 0x3025400000008000;
			g_StaticVideoList_00489590_LONG[iVar1 * 2 + 0x339] = 0x512;
		} while (uVar2 < 0x61);
		uVar2 = 0;
		do {
			iVar1 = (int)uVar2;
			uVar3 = uVar2 & 0xffffffff;
			uVar2 = (iVar1 + 1);
			g_StaticVideoList_00489590_LONG[iVar1 * 2 + 0x3fa] = uVar3 | 0x3020400000008000;
			g_StaticVideoList_00489590_LONG[iVar1 * 2 + 0x3fb] = 0x512;
			g_StaticVideoList_00489590_LONG[iVar1 * 2 + 0x4bc] = uVar3 | 0x3028400000008000;
			g_StaticVideoList_00489590_LONG[iVar1 * 2 + 0x4bd] = 0x512;
		} while (uVar2 < 0x61);
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
		UINT_ARRAY_004492dc[0] = 0x76;
		g_StaticVideoList_00489590[28] = 0x01000103;
		UINT_ARRAY_004492dc[1] = 0x198;
		g_StaticVideoList_00489590[29] = 0x20000000;
		UINT_ARRAY_004492dc[2] = 0x2ba;
		g_StaticVideoList_00489590[30] = 0xffffff00;
		UINT_004492d8 = 1;
		g_StaticVideoList_00489590[31] = 0;
		g_StaticDisplayListData_0048c340[1] = 0x200000003000077;
		g_StaticDisplayListData_0048c340[3] = 0x200000003000199;
		g_StaticDisplayListData_0048c340[5] = 0x2000000030002bb;
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
		g_StaticDisplayListData_0048c340_LONG[0] = 0x30000000;
		g_StaticDisplayListData_0048c340_LONG[2] = 0x30000000;
		g_StaticDisplayListData_0048c340_LONG[4] = 0x30000000;
		FUN_002ab6f0();
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

	union RenderFrameUnion_2c
	{
		float floatValue;
		char* ptrValue;
		uint numValue;
	};

	struct RenderFrame_30 {
		char* pFileData;
		MeshTransformSpecial* field_0x4;
		MeshTransformSpecial internalMeshTransformSpecial;
		undefined4 field_0x18;
		int index_0x1c;
		struct Matrix* pTransformMatrix;
		struct MeshTransformDataBase* pMeshTransformData;
		int flags_0x28;
		RenderFrameUnion_2c field_0x2c;
	};

	struct RenderFrame_30_B_DEPRECATED {
		char* pFileData;
		struct MeshTransformSpecial* field_0x4;
		struct RenderFrame_30_B* field_0x8;
		struct MeshTransformParentHeader* field_0xc;
		undefined4 field_0x10;
		undefined4 field_0x14;
		undefined field_0x18;
		undefined field_0x19;
		undefined field_0x1a;
		undefined field_0x1b;
		int index_0x1c;
		struct Matrix* pTransformMatrix;
		struct MeshTransformData* pMeshTransformData;
		int flags_0x28;
		float field_0x2c;
	};

	struct RenderFrame_30_Header {
		struct RenderFrame_30* pMaterialArrayBegin;
		struct RenderFrame_30* pMeshArrayBegin;
		struct RenderFrame_30* pMaterialArrayEnd;
		struct RenderFrame_30* pMeshArrayEnd;
		struct RenderFrame_30* pMeshCurrent;
		struct RenderFrame_30* pMeshEnd;
		struct RenderFrame_30* pOtherStartA;
		struct RenderFrame_30* pOtherEndA;
		struct RenderFrame_30* pCurrentRenderFrame30;
		struct RenderFrame_30* pOtherEndB;
		struct RenderFrame_30* pOtherCurrent;
	};

	RenderFrame_30_Header RenderFrame_30_Header_004494c0 = { 0 };

	byte BYTE_00449340 = 0;
	int INT_00449334 = 0;
	MeshTransformParent* g_TransformParent_00449304 = NULL;
	TextureData_MAT_Internal_A* g_MAT_Internal_B_00449338 = NULL;
	CameraPanMasterHeader* g_ActiveCameraPanMasterHeader_00449308 = NULL;
	MeshTransformSpecial* g_MeshTransformSpecial_004494a8 = NULL;

	struct SomeMeshThing {
		undefined4 field_0x0;
		undefined4 field_0x4;
		undefined4 field_0x8;
		undefined4 field_0xc;
		undefined4 field_0x10;
		undefined field_0x14;
		undefined field_0x15;
		undefined field_0x16;
		undefined field_0x17;
		undefined field_0x18;
		undefined field_0x19;
		undefined field_0x1a;
		undefined field_0x1b;
		undefined field_0x1c;
		undefined field_0x1d;
		undefined field_0x1e;
		undefined field_0x1f;
	};

	SomeMeshThing SomeMeshThing_ARRAY_0048c3e0[16] = { 0 };

	void ApplyFlags_002ab4b0(void)
	{
		uint uVar1;
		uint uVar2;

		uVar1 = 0;
		do {
			uVar2 = uVar1 + 1;
			SomeMeshThing_ARRAY_0048c3e0[uVar1].field_0x8 = SomeMeshThing_ARRAY_0048c3e0[uVar1].field_0x8 & 0xfffffff7;
			uVar1 = uVar2;
		} while (uVar2 < 0x10);
		return;
	}

	MeshTransformDataBase MeshTransformDataBase_0048cad0 = { 0 };

	void RenderSetupFunc_002b1220(void)
	{
		RenderFrame_30_Header_004494c0.pOtherCurrent = RenderFrame_30_Header_004494c0.pOtherStartA;
		MeshTransformDataBase_0048cad0.field_0xa4 = (Matrix*)0x0;
		RenderFrame_30_Header_004494c0.pOtherEndB = RenderFrame_30_Header_004494c0.pOtherEndA;
		PTR_MeshTransformParentHeader_004494b8 = PTR_MeshTransformParentHeader_004494ac;
		PTR_MeshTransformParentHeader_004494bc = PTR_MeshTransformParentHeader_004494ac;
		RenderFrame_30_Header_004494c0.pCurrentRenderFrame30 = RenderFrame_30_Header_004494c0.pOtherStartA + 1;
		(RenderFrame_30_Header_004494c0.pOtherStartA)->pTransformMatrix = &g_IdentityMatrix;
		(RenderFrame_30_Header_004494c0.pOtherCurrent)->pMeshTransformData = &MeshTransformDataBase_0048cad0;
		(RenderFrame_30_Header_004494c0.pOtherCurrent)->field_0x2c.floatValue = 1.0f;
		((RenderFrame_30_Header_004494c0.pOtherCurrent)->internalMeshTransformSpecial).specUnion.randoPtr =
			RenderFrame_30_Header_004494c0.pOtherCurrent;
		(RenderFrame_30_Header_004494c0.pOtherCurrent)->field_0x4 =
			(MeshTransformSpecial*)RenderFrame_30_Header_004494c0.pOtherCurrent;
		((RenderFrame_30_Header_004494c0.pOtherCurrent)->internalMeshTransformSpecial).pNext_0x4 =
			(MeshTransformSpecial*)PTR_MeshTransformParentHeader_004494bc;
		((RenderFrame_30_Header_004494c0.pOtherCurrent)->internalMeshTransformSpecial).pPrev_0x8 = (MeshTransformSpecial*)0x0
			;
		((RenderFrame_30_Header_004494c0.pOtherCurrent)->internalMeshTransformSpecial).pRenderInput =
			(DisplayListInternal*)0x0;
		(RenderFrame_30_Header_004494c0.pOtherCurrent)->pFileData = (char*)0x0;
		(RenderFrame_30_Header_004494c0.pOtherCurrent)->pMeshTransformData = &MeshTransformDataBase_0048cad0;
		return;
	}

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
				g_MAT_Internal_B_00449338 = (TextureData_MAT_Internal_A*)pMVar1->pFileData;
			}
			g_TransformParent_00449304 = g_TransformParent_00449304->pNext;
		} while (pMVar1->field_0x2c.numValue == 0xdfdfdfdf);
		g_ActiveCameraPanMasterHeader_00449308 = param_1;
		if ((pMVar1->flags_0x28 & 2U) != 0) {
			ApplyFlags_002ab4b0();
			pMVar1->flags_0x28 = pMVar1->flags_0x28 & 0xfffffffd;
		}

		if (((TextureData_MAT_Internal_A*)pMVar1->pFileData)->count_0x0 < 2) {
			pcVar1 = pMVar1->field_0x2c.ptrValue;
			if ((pcVar1 != (char*)0x0) && (*(short*)(pcVar1 + 4) != 0)) {
				iVar2 = *(int*)(pcVar1 + 8);
				char* pBufferAddr = (char*)LOAD_SECTION(iVar2);
				iVar4 = INT_00449334 * 8;
				WaitDMA();
				RENDER_LOG("DMA Begin SendModelGIFCommands_0029d010\n");
				shellDmaStartChainB(SHELLDMA_CHANNEL_GIF, *(ulonglong**)(pBufferAddr + iVar4));
			}
		}
		else {
			IMPLEMENTATION_GUARD(
			puVar3 = FUN_0029d150(pMVar1, g_ed3D_CommandBuffer_00449300, 1, INT_00449334);
			if (puVar3 != (ulong*)0x0) {
				WaitDMA();
				RENDER_LOG("DMA Begin SendModelGIFCommands_0029d010\n");
				shellDmaStartChainB(SHELLDMA_CHANNEL_GIF, g_ed3D_CommandBuffer_00449300);
			})
		}
		INT_00449334 = INT_00449334 + 1;
		return;
	}

	void SendModelGIFCommands_0029cf90(CameraPanMasterHeader* pCommandInfo)
	{
		RenderCommand* puVar1;

		SendModelGIFCommands_0029d010(pCommandInfo);
		puVar1 = g_RenderCommand_004492c0;
		DMA_Registers[2].QWC = 0xc0;
		if (g_RenderCommand_004492b0 != g_RenderCommand_004492c0) {
			g_RenderCommand_004492c0->cmdA = 0x70000000;
			puVar1->cmdB = 0;
			WaitForDraw_00258230();
			WaitForDraw_00258230();
#ifdef PLATFORM_IS2
			DPUT_VIF1_FIFO(*(u_long128*)&g_VIF1_FIFO_00424fe0[0]);
#endif
			WaitDMA();
			RENDER_LOG("DMA Begin SendModelGIFCommands_0029cf90\n");
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
			RENDER_LOG("DMA Begin ModelVIFCommands_0029bb10\n");
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

	struct ed3D_SubAllocator {
		char* base;
		char* current;
		char* end;
	};

	ed3D_SubAllocator ed3D_SubAllocator_004491b8 = { 0 };

	void SetStaticVideoListValues_0029c4f0(void)
	{
		g_StaticVideoList_00489590[1406] = 0x4895b030000001;
		g_StaticVideoList_00489590[1407] = 0;
		g_StaticVideoList_00489590[1408] = 0x4895d030000002;
		g_StaticVideoList_00489590[1409] = 0;
		g_StaticVideoList_00489590[1410] = 0x4895c030000001;
		g_StaticVideoList_00489590[1411] = 0;
		g_StaticVideoList_00489590[1412] = 0x48960030000001;
		g_StaticVideoList_00489590[1413] = 0;
		g_StaticVideoList_00489590[1414] = 0x48962030000001;
		g_StaticVideoList_00489590[1415] = 0;
		g_StaticVideoList_00489590[1416] = 0x4895a030000001;
		g_StaticVideoList_00489590[1417] = 0;
		g_StaticVideoList_00489590[1418] = 0x4895f030000001;
		g_StaticVideoList_00489590[1419] = 0;
		g_StaticVideoList_00489590[1420] = 0x48961030000001;
		g_StaticVideoList_00489590[1421] = 0;
		return;
	}

	RenderCommand* g_RenderCommandVidMem_004492a0 = NULL;
	RenderCommand* g_RenderCommandVidMemEnd_004492d0 = NULL;
	char* g_ScratchBase_004493d0 = NULL;
	Matrix* g_GraphicsMatrixBuffer_004493e0 = NULL;

	char* SubAllocate_0029ce60(int size)
	{
		char* pcVar1;
		char* pcVar2;

		pcVar2 = ed3D_Allocator_00449248.end;
		pcVar1 = ed3D_Allocator_00449248.end + size;
		if (ed3D_Allocator_00449248.current < ed3D_Allocator_00449248.end + size) {
			pcVar2 = (char*)0x0;
			pcVar1 = ed3D_Allocator_00449248.end;
		}
		ed3D_Allocator_00449248.end = pcVar1;
		return pcVar2;
	}

	void AllocateMatrixScratch_002a93b0(void)
	{
		g_ScratchBase_004493d0 = SubAllocate_0029ce60(ed3D::Params_00424e30.meshHeaderCountA * 0x380);
		g_GraphicsMatrixBuffer_004493e0 = (Matrix*)g_ScratchBase_004493d0;
		//PTR_DAT_004493f0 = (undefined*)((int)g_ScratchBase_004493d0 + ed3D::Params_00424e30.meshHeaderCountA * 7 * 0x40);
		return;
	}

	void InitVideoMemory_0029c220(uint countA, int countB)
	{
		char* pcVar1;
		char* pcVar2;
		ulong lVar3;

		UINT_0048c2c0 = 0x22;
		UINT_0048c2cc = 0xc000;
		lVar3 = (ulong)ed3D_Allocator_00449248.end;
		UINT_0048c2c4 = 0x3e;
		UINT_0048c2c8 = 0x5a;
		pcVar2 = ed3D_Allocator_00449248.end + countA * 0x10;
		if (ed3D_Allocator_00449248.current < ed3D_Allocator_00449248.end + countA * 0x10) {
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
		g_VideoMemorySize_00449280 = countA;
		g_VideoMemoryPtr_00449280 = g_VideoMemoryBasePtr_00449260;
		InitStaticVideoList_0029c740();
		//DAT_0048c2b0 = 0;
		//DAT_0048c2b4 = 0;
		//DAT_0048c2b8 = 0;
		SetStaticVideoListValues_0029c4f0();
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
		//g_ed3D_CommandBuffer_00449300 = (ulong*)((uint)pcVar2 & 0xdfffffff);
		AllocateMatrixScratch_002a93b0();
		lVar3 = (ulong)ed3D_Allocator_00449248.end;
		pcVar2 = ed3D_Allocator_00449248.end + countB * 0x20;
		if (ed3D_Allocator_00449248.current < ed3D_Allocator_00449248.end + countB * 0x20) {
			lVar3 = 0;
			pcVar2 = ed3D_Allocator_00449248.end;
		}
		ed3D_Allocator_00449248.end = pcVar2;
		g_RenderCommandVidMem_004492a0 = (RenderCommand*)(((ulong)(lVar3 << 0x24) >> 0x24) | 0x30000000);
		g_RenderCommand_004492c0 = g_RenderCommandVidMem_004492a0;
		g_RenderCommandVidMemEnd_004492d0 = g_RenderCommandVidMem_004492a0 + countB;
		return;
	}

	TextureData_MAT_Internal_A* g_TextureMatInternal_0044933c = NULL;
	RenderFrame_30* g_RenderFrameData_004493f8 = NULL;
	int INT_00449330 = 0;

	void ResetMemoryPtr_0029c060(void)
	{
		uint uVar1;
		int iVar2;

		INT_00449330 = 0;
		g_TextureMatInternal_0044933c = (TextureData_MAT_Internal_A*)0x0;
		g_RenderMeshTransform_004493f4 = (MeshTransformDataBase*)0x0;
		g_RenderFrameData_004493f8 = (RenderFrame_30*)0x0;
		if (g_CameraFlipIndex_004494a4 == 0) {
			iVar2 = Params_00424e30.field_0x28 * 0x10;
			g_RenderCommand_004492c0 = g_RenderCommandVidMem_004492a0;
			g_VideoMemoryPtr_00449280 = g_VideoMemoryBasePtr_00449260;
			uVar1 = g_VideoMemorySize_00449280 >> 1;
		}
		else {
			g_VideoMemoryPtr_00449280 = g_VideoMemoryBasePtr_00449260 + (g_VideoMemorySize_00449280 >> 1);
			iVar2 = Params_00424e30.field_0x28 * 0x20;
			g_RenderCommand_004492c0 = g_RenderCommandVidMem_004492a0 + Params_00424e30.field_0x28;
			uVar1 = g_VideoMemorySize_00449280;
		}
		g_RenderCommandVidMemEnd_004492d0 = (RenderCommand*)((ulong)&g_RenderCommandVidMem_004492a0->cmdA + iVar2);
		//DAT_00449290 = g_VideoMemoryBasePtr_00449260 + uVar1;
		g_RenderCommandBuffer_00449400 = (RenderCommand*)0x0;
		//DAT_004492d4 = 1;
		g_RenderCommand_004492b0 = g_RenderCommand_004492c0;
		//FUN_002a9350();
		ed3D_SubAllocator_004491b8.current = ed3D_SubAllocator_004491b8.base;
		g_FlipDMA_00449270 = g_VideoMemoryPtr_00449280;
		return;
	}

	char* FUN_0029e370(RenderFrame_30* param_1, int index)
	{
		char* pcVar1;

		pcVar1 = param_1->pFileData;
		if ((pcVar1 == (char*)0x0) || (*pcVar1 == '\0')) {
			pcVar1 = (char*)0x0;
		}
		else {
			IMPLEMENTATION_GUARD(pcVar1 = (char*)FUN_0029e2f0(pcVar1, index));
		}
		return pcVar1;
	}

	char DAT_0048c3b0[200] = { 0 };
	char DAT_0048c3c0[200] = { 0 };

	void UpdateRenderFrame30_002b6b20(void)
	{
		MeshTransformParentHeader* pMVar1;
		char** ppFileData;
		CameraPanMasterHeader* pCVar3;
		RenderFrame_30* pMeshCurrrent;
		MeshTransformSpecial* pInternalMeshTransformSpecial;
		char* pcVar6;

		// Setup Mesh Current
		pMeshCurrrent = RenderFrame_30_Header_004494c0.pMeshCurrent;
		pInternalMeshTransformSpecial = &(RenderFrame_30_Header_004494c0.pMeshCurrent)->internalMeshTransformSpecial;
		ppFileData = &(RenderFrame_30_Header_004494c0.pMeshCurrent)->pFileData;
		RenderFrame_30_Header_004494c0.pMeshCurrent = RenderFrame_30_Header_004494c0.pMeshCurrent + 1;
		*ppFileData = DAT_0048c3b0;
		pMeshCurrrent->field_0x4 = (MeshTransformSpecial*)0x0;
		pMeshCurrrent->flags_0x28 = pMeshCurrrent->flags_0x28 & 0xfffffffe;
		(pMeshCurrrent->internalMeshTransformSpecial).pPrev_0x8 = pInternalMeshTransformSpecial;
		(pMeshCurrrent->internalMeshTransformSpecial).pNext_0x4 = pInternalMeshTransformSpecial;
		(pMeshCurrrent->internalMeshTransformSpecial).pRenderInput = (DisplayListInternal*)PTR_MeshTransformParentHeader_004494b8;
		pMeshCurrrent->field_0x18 = 0;
		pMeshCurrrent->index_0x1c = 0;
		pMeshCurrrent->internalMeshTransformSpecial.specUnion.randoPtr = NULL;
		pcVar6 = FUN_0029e370(pMeshCurrrent, 0);
		pMeshCurrrent->field_0x2c.ptrValue = pcVar6;

		pMVar1 = g_OtherDoubleBuffer_0048cd50[g_CameraFlipIndex_004494a4].pCameraPanHeader_0xc;
		pInternalMeshTransformSpecial = pMVar1->field_0x4 + pMVar1->usedCount;
		pInternalMeshTransformSpecial->pRenderInput = (DisplayListInternal*)pMeshCurrrent;
		pInternalMeshTransformSpecial->specUnion.field_0x0[0] = 1;
		pMVar1->usedCount = pMVar1->usedCount + 1;
		pCVar3 = g_OtherDoubleBuffer_0048cd50 + g_CameraFlipIndex_004494a4;
		g_OtherDoubleBuffer_0048cd50[g_CameraFlipIndex_004494a4].count_0x14 =
			g_OtherDoubleBuffer_0048cd50[g_CameraFlipIndex_004494a4].count_0x14 + 1;
		pInternalMeshTransformSpecial->pNext_0x4 = pCVar3->pLoadedData;
		g_OtherDoubleBuffer_0048cd50[g_CameraFlipIndex_004494a4].pLoadedData = pInternalMeshTransformSpecial;

		// Setup Mesh Current
		pMeshCurrrent = RenderFrame_30_Header_004494c0.pMeshCurrent;
		pInternalMeshTransformSpecial = &(RenderFrame_30_Header_004494c0.pMeshCurrent)->internalMeshTransformSpecial;
		ppFileData = &(RenderFrame_30_Header_004494c0.pMeshCurrent)->pFileData;
		RenderFrame_30_Header_004494c0.pMeshCurrent = RenderFrame_30_Header_004494c0.pMeshCurrent + 1;
		*ppFileData = DAT_0048c3c0;
		pMeshCurrrent->field_0x4 = (MeshTransformSpecial*)0x0;
		pMeshCurrrent->flags_0x28 = pMeshCurrrent->flags_0x28 & 0xfffffffe;
		(pMeshCurrrent->internalMeshTransformSpecial).pPrev_0x8 = pInternalMeshTransformSpecial;
		(pMeshCurrrent->internalMeshTransformSpecial).pNext_0x4 = pInternalMeshTransformSpecial;
		(pMeshCurrrent->internalMeshTransformSpecial).pRenderInput = (DisplayListInternal*)PTR_MeshTransformParentHeader_004494b8;
		pMeshCurrrent->field_0x18 = 0;
		pMeshCurrrent->index_0x1c = 0;
		*(undefined4*)&pMeshCurrrent->internalMeshTransformSpecial = 0;
		pMeshCurrrent->flags_0x28 = pMeshCurrrent->flags_0x28 | 1;
		pcVar6 = FUN_0029e370(pMeshCurrrent, 0);
		pMeshCurrrent->field_0x2c.ptrValue = pcVar6;

		pMVar1 = g_OtherDoubleBuffer_0048cd50[g_CameraFlipIndex_004494a4].pCameraPanHeader_0xc;
		pInternalMeshTransformSpecial = pMVar1->field_0x4 + pMVar1->usedCount;
		pInternalMeshTransformSpecial->pRenderInput = (DisplayListInternal*)pMeshCurrrent;
		pInternalMeshTransformSpecial->specUnion.field_0x0[0] = 1;
		pMVar1->usedCount = pMVar1->usedCount + 1;
		pCVar3 = g_OtherDoubleBuffer_0048cd50 + g_CameraFlipIndex_004494a4;
		g_OtherDoubleBuffer_0048cd50[g_CameraFlipIndex_004494a4].count_0x14 =
			g_OtherDoubleBuffer_0048cd50[g_CameraFlipIndex_004494a4].count_0x14 + 1;
		pInternalMeshTransformSpecial->pNext_0x4 = pCVar3->pLoadedData;
		g_OtherDoubleBuffer_0048cd50[g_CameraFlipIndex_004494a4].pLoadedData = pInternalMeshTransformSpecial;
		//DAT_00449568 = 0;
		return;
	}

	int g_RenderPassCount_0044939c = 0;

	void ResetDoubleBuffer_002b1140(void)
	{
		CameraPanMasterHeader* pCVar1;
		MeshTransformParentHeader** ppMVar2;

		ppMVar2 = &PTR_MeshTransformParentHeader_004494b0 + g_CameraFlipIndex_004494a4;
		pCVar1 = g_ModelDoubleBuffer_0048c6d0 + g_CameraFlipIndex_004494a4;
		g_ModelDoubleBuffer_0048c6d0[g_CameraFlipIndex_004494a4].pLoadedData =
			(MeshTransformSpecial*)(g_ModelDoubleBuffer_0048c6d0 + g_CameraFlipIndex_004494a4);
		pCVar1->count_0x14 = 0;
		g_MeshTransformSpecial_004494a8 = (MeshTransformSpecial*)0x0;
		(*ppMVar2)->usedCount = 0;
		RenderFrame_30_Header_004494c0.pMeshCurrent =
			(&RenderFrame_30_Header_004494c0.pMaterialArrayBegin)[g_CameraFlipIndex_004494a4];
		RenderFrame_30_Header_004494c0.pMeshEnd =
			(&RenderFrame_30_Header_004494c0.pMaterialArrayEnd)[g_CameraFlipIndex_004494a4];
		return;
	}


	void UpdateRenderMeshTransform_002954b0(MeshTransformData* param_1, bool mode)
	{
		MeshTransformData* v1;
		MeshTransformData* v1_00;
		Matrix* v1_01;
		Matrix* v1_02;
		Matrix* v1_03;
		Matrix* v1_04;
		MeshTransformData* pMeshTransformData;
		MeshTransformData* pMVar1;
		ulong uVar2;

		if ((mode != false) && ((param_1->base).subMeshParentCount_0xac != 0)) {
			return;
		}
		if ((param_1->base).field_0x88 == 0) {
			return;
		}
		v1 = (param_1->base).pLinkTransformData;
		if (v1 == (MeshTransformData*)0x0) {
			if (((param_1->base).flags_0x9e & 1) == 0) {
				sceVu0CopyMatrix((TO_SCE_MTX) & (param_1->base).transformB, (TO_SCE_MTX)param_1);
			}
			goto LAB_002957c4;
		}
		if ((v1->base).field_0x88 != '\0') {
			v1_00 = (v1->base).pLinkTransformData;
			if (v1_00 == (MeshTransformData*)0x0) {
				if (((v1->base).flags_0x9e & 1) == 0) {
					sceVu0CopyMatrix((TO_SCE_MTX) & (v1->base).transformB, (TO_SCE_MTX)v1);
				}
			}
			else {
				if (*(char*)((int)(v1_00->base).hash.number + 8) != '\0') {
					IMPLEMENTATION_GUARD(
					v1_01 = *(Matrix*)((int)(v1_00->base).hash.number + 0x10);
					if (v1_01 == (Matrix*)0x0) {
						if ((*(ushort*)((int)(v1_00->base).hash.number + 0x1e) & 1) == 0) {
							sceVu0CopyMatrix((TO_SCE_MTX) & (v1_00->base).transformB, (TO_SCE_MTX)v1_00);
						}
					}
					else {
						if (*(char*)(v1_01[2] + 2) != '\0') {
							v1_02 = (Matrix*)v1_01[2][1][0];
							if (v1_02 == (Matrix*)0x0) {
								if ((*(ushort*)((int)v1_01[2][1] + 0xe) & 1) == 0) {
									sceVu0CopyMatrix(v1_01[1], v1_01);
								}
							}
							else {
								if (*(char*)(v1_02[2] + 2) != '\0') {
									v1_03 = (TO_SCE_MTX)v1_02[2][1][0];
									if (v1_03 == (TO_SCE_MTX)0x0) {
										if ((*(ushort*)((int)v1_02[2][1] + 0xe) & 1) == 0) {
											sceVu0CopyMatrix(v1_02[1], v1_02);
										}
									}
									else {
										if (*(char*)(v1_03[2] + 2) != '\0') {
											v1_04 = (TO_SCE_MTX)v1_03[2][1][0];
											if (v1_04 == (TO_SCE_MTX)0x0) {
												if ((*(ushort*)((int)v1_03[2][1] + 0xe) & 1) == 0) {
													sceVu0CopyMatrix(v1_03[1], v1_03);
												}
											}
											else {
												if (*(char*)(v1_04[2] + 2) != '\0') {
													pMeshTransformData = (MeshTransformData*)v1_04[2][1][0];
													if (pMeshTransformData == (MeshTransformData*)0x0) {
														if ((*(ushort*)((int)v1_04[2][1] + 0xe) & 1) == 0) {
															sceVu0CopyMatrix(v1_04[1], v1_04);
														}
													}
													else {
														uVar2 = GetParam1_0x88((int)pMeshTransformData);
														if (uVar2 != 0) {
															pMVar1 = (pMeshTransformData->base).pLinkTransformData;
															if (pMVar1 == (MeshTransformData*)0x0) {
																if (((pMeshTransformData->base).flags_0x9e & 1) == 0) {
																	sceVu0CopyMatrix((TO_SCE_MTX) & (pMeshTransformData->base).transformB,
																		(TO_SCE_MTX)pMeshTransformData);
																}
															}
															else {
																UpdateRenderMeshTransform_002954b0(pMVar1, false);
																if (((pMeshTransformData->base).flags_0x9e & 1) == 0) {
																	sceVu0MulMatrix((TO_SCE_MTX) & (pMeshTransformData->base).transformB,
																		(TO_SCE_MTX)pMeshTransformData, (TO_SCE_MTX) & (pMVar1->base).transformB);
																}
															}
															SetField_00295df0(pMeshTransformData, 0);
														}
														if ((*(ushort*)((int)v1_04[2][1] + 0xe) & 1) == 0) {
															sceVu0MulMatrix(v1_04[1], v1_04, (TO_SCE_MTX) & (pMeshTransformData->base).transformB);
															*(undefined*)(v1_04[2] + 2) = 0;
															goto LAB_00295648;
														}
													}
													*(undefined*)(v1_04[2] + 2) = 0;
												}
											LAB_00295648:
												if ((*(ushort*)((int)v1_03[2][1] + 0xe) & 1) == 0) {
													sceVu0MulMatrix(v1_03[1], v1_03, v1_04[1]);
													*(undefined*)(v1_03[2] + 2) = 0;
													goto LAB_00295688;
												}
											}
											*(undefined*)(v1_03[2] + 2) = 0;
										}
									LAB_00295688:
										if ((*(ushort*)((int)v1_02[2][1] + 0xe) & 1) == 0) {
											sceVu0MulMatrix(v1_02[1], v1_02, v1_03[1]);
											*(undefined*)(v1_02[2] + 2) = 0;
											goto LAB_002956c8;
										}
									}
									*(undefined*)(v1_02[2] + 2) = 0;
								}
							LAB_002956c8:
								if ((*(ushort*)((int)v1_01[2][1] + 0xe) & 1) == 0) {
									sceVu0MulMatrix(v1_01[1], v1_01, v1_02[1]);
									*(undefined*)(v1_01[2] + 2) = 0;
									goto LAB_00295708;
								}
							}
							*(undefined*)(v1_01[2] + 2) = 0;
						}
					LAB_00295708:
						if ((*(ushort*)((int)(v1_00->base).field_0x80 + 0x1e) & 1) == 0) {
							sceVu0MulMatrix((TO_SCE_MTX) & (v1_00->base).transformB, (TO_SCE_MTX)v1_00, v1_01[1]);
							*(undefined*)((int)(v1_00->base).field_0x80 + 8) = 0;
							goto LAB_00295748;
						}
					}
					*(undefined*)((int)(v1_00->base).field_0x80 + 8) = 0;)
				}
			LAB_00295748:
				if (((v1->base).flags_0x9e & 1) == 0) {
					sceVu0MulMatrix((TO_SCE_MTX) & (v1->base).transformB, (TO_SCE_MTX)v1, (TO_SCE_MTX) & (v1_00->base).transformB);
					(v1->base).field_0x88 = 0;
					goto LAB_00295788;
				}
			}
			(v1->base).field_0x88 = 0;
		}
	LAB_00295788:
		if (((param_1->base).flags_0x9e & 1) == 0) {
			sceVu0MulMatrix((TO_SCE_MTX) & (param_1->base).transformB, (TO_SCE_MTX)param_1, (TO_SCE_MTX) & (v1->base).transformB);
			(param_1->base).field_0x88 = 0;
			return;
		}
	LAB_002957c4:
		(param_1->base).field_0x88 = 0;
		return;
	}

	void UpdateMeshTransforms_002b2a40(void)
	{
		CameraPanMasterHeader* pCVar1;
		int iVar2;
		uint uVar3;
		uint uVar4;
		MeshTransformParent* pCVar6;
		int* piVar5;
		StaticMeshMaster* pSVar6;
		int iVar7;

		for (uVar3 = 0; uVar3 < (uint)ed3D::Params_00424e30.staticMeshMasterCount; uVar3 = uVar3 + 1) {
			pSVar6 = ed3D::cameraPanStaticMasterArray_00449378 + uVar3;
			if (((pSVar6->flags_0x4 & 1) != 0) && ((pSVar6->flags_0x4 & 4) == 0)) {
				pCVar1 = pSVar6->pFinalLink_0x1a4;
				for (pCVar6 = (MeshTransformParent*)pCVar1->pLoadedData; (CameraPanMasterHeader*)pCVar6 != pCVar1;
					pCVar6 = pCVar6->pNext) {
					UpdateRenderMeshTransform_002954b0((MeshTransformData*)pCVar6->pMeshTransformData, true);
				}
				piVar5 = pSVar6->field_0x1ac;
				for (uVar4 = 0; uVar4 < pSVar6->count_0x1b0; uVar4 = uVar4 + 1) {
					iVar2 = *piVar5;
					for (iVar7 = *(int*)(iVar2 + 4); iVar7 != iVar2; iVar7 = *(int*)(iVar7 + 4)) {
						IMPLEMENTATION_GUARD(UpdateRenderMeshTransform_002954b0(*(MeshTransformData**)(iVar7 + 0xc), true));
					}
					piVar5 = piVar5 + 1;
				}
			}
		}
		edSysHandlersCall(edDlist::sysHandler_0048cb90.mainIdentifier, edDlist::sysHandler_0048cb90.entries,
			edDlist::sysHandler_0048cb90.maxEventID, 4, (void*)0x0);
		return;
	}

	void EdSoundHandler_002b2730(int param_1, int param_2, char* param_3)
	{
		int iVar1;
		uint uVar2;
		//ProfileObject* pPVar3;
		//
		//iVar1 = g_3DRendProfileObjIndex_004491e8;
		g_RenderPassCount_0044939c = 0;
		if (ed3D::Params_00424e30.bEnableProfile != 0) {
			IMPLEMENTATION_GUARD(
			if ((g_3DRendProfileObjIndex_004491e8 & 1U) == 0) {
				uVar2 = (uint)g_3DRendProfileObjIndex_004491e8 >> 4;
				if (g_ProfileObjB_0041ed44[uVar2].field_0x34 != 0) {
					g_ProfileObjB_0041ed44[uVar2].field_0x8 = 0;
					g_ProfileObjB_0041ed44[uVar2].field_0x20 = 0;
					g_ProfileObjB_0041ed44[uVar2].field_0x34 = 0;
				}
				g_ProfileObjB_0041ed44[(uint)iVar1 >> 4].field_0x0 = (long)PCR1 & 0xffffffff;
				g_ProfileObjB_0041ed44[(uint)iVar1 >> 4].field_0x20 = g_ProfileObjB_0041ed44[(uint)iVar1 >> 4].field_0x20 + 1;
			}
			else {
				uVar2 = (uint)g_3DRendProfileObjIndex_004491e8 >> 4;
				if (g_ProfileObjA_0041ed40[uVar2].field_0x34 != 0) {
					g_ProfileObjA_0041ed40[uVar2].field_0x8 = 0;
					g_ProfileObjA_0041ed40[uVar2].field_0x20 = 0;
					g_ProfileObjA_0041ed40[uVar2].field_0x34 = 0;
				}
				g_ProfileObjA_0041ed40[(uint)iVar1 >> 4].field_0x0 = (long)PCR1 & 0xffffffff;
				g_ProfileObjA_0041ed40[(uint)iVar1 >> 4].field_0x20 = g_ProfileObjA_0041ed40[(uint)iVar1 >> 4].field_0x20 + 1;
			})
		}
		SendModelVIFCommands_0029bb10();
		//DAT_00449398 = 0;
		//FUN_002ad840();
		if ((ed3D::Params_00424e30.field_0x2c & 2) != 0) {
			UpdateMeshTransforms_002b2a40();
		}
		ed3D::g_RenderActiveCameraObj_006f7d80 = (CameraObj_28*)0x0;
		//FUN_0029c6f0();
		//FUN_0029ced0();
		ResetMemoryPtr_0029c060();
		ResetDoubleBuffer_002b1140();
		RenderSetupFunc_002b1220();
		UpdateRenderFrame30_002b6b20();
		if (ed3D::Params_00424e30.bEnableProfile != 0) {
			IMPLEMENTATION_GUARD(
			uVar2 = (uint)g_3DRendProfileObjIndex_004491e8 >> 4;
			if ((g_3DRendProfileObjIndex_004491e8 & 1U) == 0) {
				g_ProfileObjB_0041ed44[uVar2].field_0x0 = (long)PCR1 - g_ProfileObjB_0041ed44[uVar2].field_0x0;
				g_ProfileObjB_0041ed44[uVar2].field_0x8 =
					g_ProfileObjB_0041ed44[uVar2].field_0x8 + g_ProfileObjB_0041ed44[uVar2].field_0x0;
				pPVar3 = g_ProfileObjB_0041ed44 + uVar2;
				if (pPVar3->field_0x10 < g_ProfileObjB_0041ed44[uVar2].field_0x0) {
					pPVar3->field_0x10 = pPVar3->field_0x0;
				}
			}
			else {
				g_ProfileObjA_0041ed40[uVar2].field_0x0 = (long)PCR1 - g_ProfileObjA_0041ed40[uVar2].field_0x0;
				g_ProfileObjA_0041ed40[uVar2].field_0x8 =
					g_ProfileObjA_0041ed40[uVar2].field_0x8 + g_ProfileObjA_0041ed40[uVar2].field_0x0;
				pPVar3 = g_ProfileObjA_0041ed40 + uVar2;
				if (pPVar3->field_0x10 < g_ProfileObjA_0041ed40[uVar2].field_0x0) {
					pPVar3->field_0x10 = pPVar3->field_0x0;
				}
			})
		}
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

	float g_DefaultNearClip_0044851c = -0.001f;

	void FUN_002b70b0(CameraPanStatic_100* param_1)
	{
		param_1->field_0x0 = 0;
		param_1->field_0x4 = 20.0;
		param_1->field_0x8 = -0.0;
		param_1->field_0xc = -50.0;
		param_1->pCamera_0x10 = (CameraObj_28*)0x0;
		param_1->pCamera_0x14 = (CameraObj_28*)0x0;
		param_1->field_0x20 = 0;
		param_1->field_0x22 = 0x30;
		param_1->field_0x23 = 0x14;
		param_1->field_0x24 = 2;
		param_1->field_0x18 = 0x80;
		param_1->field_0x1c = 0x80;
		return;
	}

	Vector Vector_00424f10 = { 10.0f, 10.0f, 10.0f, 128.0f };
	Matrix Matrix_00424ed0 = { 64.0f,
		0.0f,
		0.0f,
		128.0f,
		0.0f,
		64.0f,
		0.0f,
		128.0f,
		0.0f,
		0.0f,
		64.0f,
		128.0f,
		128.0f,
		128.0f,
		128.0f,
		128.0f
	};

	Matrix Matrix_00424e90 = { 1.0f,
		0.0f,
		0.0f,
		0.0f,
		0.0f,
		-2.0f,
		0.0f,
		0.0f,
		-2.0f,
		0.0f,
		0.0f,
		0.0f,
		0.0f,
		0.0f,
		-2.0f,
		0.0f
	};

	Vector* PTR_Vector_00448974 = &Vector_00424f10;
	Vector* PTR_Matrix_0044896c = (Vector*)&Matrix_00424e90;
	Vector* PTR_Matrix_00448970 = (Vector*)&Matrix_00424ed0;

	void FUN_00293ea0(CameraPanStatic_C* param_1)
	{
		param_1->field_0x0 = PTR_Vector_00448974;
		param_1->field_0x4 = PTR_Matrix_0044896c;
		param_1->field_0x8 = PTR_Matrix_00448970;
		return;
	}

	void CameraPanStatic_50_Constructor_0029e500(CameraPanStatic_50* param_1)
	{
		param_1->field_0x4 = 250.0;
		param_1->field_0x18 = 250.0;
		param_1->nearClip = g_DefaultNearClip_0044851c;
		param_1->farClip = -5000.0;
		param_1->field_0x10 = 1;
		(param_1->field_0xe0).x = 0.0;
		(param_1->field_0xe0).y = 255.0;
		(param_1->field_0xe0).z = g_DefaultNearClip_0044851c;
		(param_1->field_0xe0).w = -3200.0;
		(param_1->fogCol_0xf0).r = 0x3e;
		(param_1->fogCol_0xf0).g = 0x47;
		(param_1->fogCol_0xf0).b = 0x67;
		FUN_002b70b0(&param_1->field_0x100);
		FUN_00293ea0(&param_1->field_0x130);
		return;
	}

	StaticMeshMaster* FindFreeStaticMeshMaster_002b4600(CameraObj_130* param_1, CameraObj_28* pCamera, long mode)
	{
		ushort uVar1;
		FrameBuffer* pVVar2;
		StaticMeshMaster* pSVar3;
		CameraPanMasterHeader* pCVar4;
		StaticMeshMaster* pFreeStaticMeshMaster;
		uint staticMeshMasterIndex;

		staticMeshMasterIndex = 0;
		/* Find inactive camera master. */
		while ((staticMeshMasterIndex < (uint)Params_00424e30.staticMeshMasterCount &&
			((cameraPanStaticMasterArray_00449378[staticMeshMasterIndex].flags_0x4 & 1) != 0))) {
			staticMeshMasterIndex = staticMeshMasterIndex + 1;
		}
		pSVar3 = (StaticMeshMaster*)0xffffffff;
		if (staticMeshMasterIndex < (uint)Params_00424e30.staticMeshMasterCount) {
			pSVar3 = cameraPanStaticMasterArray_00449378 + staticMeshMasterIndex;
			pSVar3->flags_0x4 = pSVar3->flags_0x4 | 1;
			cameraPanStaticMasterArray_00449378[staticMeshMasterIndex].isChild = 0;
			cameraPanStaticMasterArray_00449378[staticMeshMasterIndex].pCameraObj130_0x8 = param_1;
			cameraPanStaticMasterArray_00449378[staticMeshMasterIndex].pCameraObj28_0xc = pCamera;
			if (g_CameraObj_28_00449384 == (CameraObj_28*)0x0) {
				g_CameraObj_28_00449384 = pCamera;
			}
			CameraPanStatic_50_Constructor_0029e500(&cameraPanStaticMasterArray_00449378[staticMeshMasterIndex].field_0x50);
			pFreeStaticMeshMaster = cameraPanStaticMasterArray_00449378 + staticMeshMasterIndex;
			pVVar2 = pFreeStaticMeshMaster->pCameraObj28_0xc->pZBuffer;
			if (pVVar2 == (FrameBuffer*)0x0) {
				assert(false);
				//(pSVar5->field_0x50).field_0x14 = &DAT_00ffffef;
			}
			else {
				uVar1 = pVVar2->pVidModeData_0x0->pixelStoreMode;
				if (uVar1 == 2) {
					(pFreeStaticMeshMaster->field_0x50).field_0x14 = 0xfffe;
				}
				else {
					if (uVar1 == 1) {
						assert(false);
						//(pSVar5->field_0x50).field_0x14 = &DAT_00ffffef;
					}
					else {
						if (uVar1 == 0) {
							(pFreeStaticMeshMaster->field_0x50).field_0x14 = 0x00ffffef;
						}
					}
				}
			}
			if (mode != 0) {
				pCVar4 = AllocateCameraPanStatic_002abc80();
				cameraPanStaticMasterArray_00449378[staticMeshMasterIndex].pFinalLink_0x1a4 = pCVar4;
				pCVar4 = AllocateCameraPanStatic_002abc80();
				cameraPanStaticMasterArray_00449378[staticMeshMasterIndex].field_0x1a8 = pCVar4;
			}
		}
		return pSVar3;
	}


	void FUN_002a8aa0(void)
	{
		g_RenderMeshTransform_004493f4 = (MeshTransformDataBase*)0xefefefef;
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

	Matrix* g_ScratchpadAddr_00448a58 = (Matrix*)0x70000000;

	struct ScratchpadAnimManager {
		ScratchpadAnimManager();
		struct Matrix* pScreenSpaceMatrix;
		struct Matrix* pTextureSpaceMatrix;
		struct Matrix* pWorldSpaceMatrix;
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
		int* field_0x3c;
		char* field_0x40;
		char* field_0x44;
		char* field_0x48;
		char* field_0x4c;
		char* field_0x50;
		char* field_0x54;
		char* field_0x58;
		struct CameraPanStatic_50* pData_150;
		struct RenderAnimationStatic* pRenderAnimationStatic_0x60;
		struct Vector* vector_0x64;
		struct AnimScratchpad* pAnimScratchpad;
	};

	struct RenderAnimationStatic {
		struct Matrix* pSharedMeshTransform;
		struct Matrix* pMeshTransformMatrix;
		int field_0x8;
		struct LightingMatrixFuncObj* pLightingMatrixFuncObj;
		uint flags_0x10;
		float field_0x14;
		undefined4 field_0x18;
		struct MeshTransformData* pMeshTransformData;
	};

	struct AnimScratchpad {
		int field_0x0;
		uint flags;
		undefined4 field_0x8;
		undefined4 field_0xc;
	};

	ScratchpadAnimManager::ScratchpadAnimManager()
	{
#ifdef PLATFORM_WIN
		g_ScratchpadAddr_00448a58 = (Matrix*)malloc(0x10000);
#endif
		pScreenSpaceMatrix = g_ScratchpadAddr_00448a58 + 1;
		pTextureSpaceMatrix = g_ScratchpadAddr_00448a58 + 2;
		pWorldSpaceMatrix = g_ScratchpadAddr_00448a58 + 3;
		pMatrix_0xc = g_ScratchpadAddr_00448a58 + 4;
		pMatrix_0x10 = g_ScratchpadAddr_00448a58 + 5;
		pVector_0x14 = (Vector*)(g_ScratchpadAddr_00448a58 + 6);
		pVector_0x18 = (Vector*)&g_ScratchpadAddr_00448a58[6].ba;
		field_0x1c = (char*)&g_ScratchpadAddr_00448a58[6].ca;
		field_0x20 = (Matrix**)&g_ScratchpadAddr_00448a58[6].cb;
		field_0x24 = (char*)&g_ScratchpadAddr_00448a58[6].cc;
		field_0x28 = (undefined4*)& g_ScratchpadAddr_00448a58[6].cd;
		pData_130 = (CameraObj_130*)&g_ScratchpadAddr_00448a58[6].da;
		pMatrix_0x30 = (Matrix*)&g_ScratchpadAddr_00448a58[0xb].ca;
		field_0x34 = (char*)&g_ScratchpadAddr_00448a58[0xc].ca;
		field_0x38 = (char*)&g_ScratchpadAddr_00448a58[0xc].cd;
		field_0x3c = (int*)&g_ScratchpadAddr_00448a58[0xd].ba;
		field_0x40 = (char*)&g_ScratchpadAddr_00448a58[0xd].bb;
		field_0x44 = (char*)&g_ScratchpadAddr_00448a58[0xd].bc;
		field_0x48 = (char*)&g_ScratchpadAddr_00448a58[0xd].bd;
		field_0x4c = (char*)&g_ScratchpadAddr_00448a58[0xd].ca;
		field_0x50 = (char*)&g_ScratchpadAddr_00448a58[0xd].cb;
		field_0x54 = (char*)&g_ScratchpadAddr_00448a58[0xd].cc;
		field_0x58 = (char*)&g_ScratchpadAddr_00448a58[0xd].cd;
		pData_150 = (CameraPanStatic_50*)&g_ScratchpadAddr_00448a58[0xd].da;
		pRenderAnimationStatic_0x60 =
			(RenderAnimationStatic*)(g_ScratchpadAddr_00448a58 + 0x13);
		vector_0x64 = (Vector*)&g_ScratchpadAddr_00448a58[0x13].ca;
		pAnimScratchpad = (AnimScratchpad*)&g_ScratchpadAddr_00448a58[0x13].da;
		return;
	}

	ScratchpadAnimManager ScratchpadAnimManager_004494ec;
	Matrix Matrix_004894e0 = { 0 };

#ifdef PLATFORM_WIN
	void sceVu0Normalize(sceVu0FVECTOR v0, sceVu0FVECTOR v1)
	{
		float fVar1;
		float fVar2;
		float fVar3;
		float fVar4;

		fVar1 = *v1;
		fVar2 = v1[1];
		fVar3 = v1[2];
		fVar4 = 1.0 / (sqrtf(fVar1 * fVar1 + fVar2 * fVar2 + fVar3 * fVar3) + 0.0);
		*v0 = fVar1 * fVar4;
		v0[1] = fVar2 * fVar4;
		v0[2] = fVar3 * fVar4;
		v0[3] = 0.0;
		return;
	}
#endif

	Vector* PTR_Vector_00449204 = NULL;
	Vector* PTR_Matrix_004491fc = NULL;
	Vector* PTR_Matrix_00449200 = NULL;

	void Func_00293ec0(CameraPanStatic_C* param_1)
	{
		PTR_Vector_00449204 = param_1->field_0x0;
		if (PTR_Vector_00449204 != (Vector*)0x0) {
			PTR_Vector_00449204->w = 0.0078125;
		}
		PTR_Matrix_004491fc = param_1->field_0x4;
		PTR_Matrix_00449200 = param_1->field_0x8;
		return;
	}

	Vector Vector_0048c300 = { 0 };
	Vector Vector_0048c310 = { 0 };
	Vector Vector_0048c320 = { 0 };
	Vector Vector_0048c330 = { 0 };

	float FLOAT_0044921c = 0.0f;
	float FLOAT_00449218 = 0.0f;
	float FLOAT_00449214 = 0.0f;

	FrameBuffer g_ColorBufferCopy_00489520 = { 0 };

	CameraPanStatic_50* g_CameraPanStatic_50_Ptr_00449220 = NULL;

	StaticMeshMaster* g_StaticMeshMaster_00449224 = NULL;

	float g_RectangleScale_00448980 = 5.0f;


	void ComputeMatrix_0029b610
	(float screenWidth, float screenHeight, float width, float height, float yMin, float zValue, float wValue,
		float nearClip, Matrix* mScreenSpace, Matrix* mTextureSpace, Matrix* mWorldSpace, Matrix* mUnknown,
		float farClip, float fovUp, float fovDown)

	{
		float fVar1;
		float fVar2;
		float fVar3;
		float startX;
		float fVar4;
		float startY;
		Matrix MStack128;
		Matrix MStack64;

		CalculateYAxisTransformMatrix_0029ba70(0.0, 0.0, yMin, 0.0, &MStack64);
		startY = -height;
		startX = -width;
		ComputeMapRectangleMatrix_0029b9e0
		(startX, width, startY, height, zValue - screenWidth * 0.5, zValue + screenWidth * 0.5,
			wValue + screenHeight * 0.5, wValue - screenHeight * 0.5, &MStack128);
		sceVu0MulMatrix((TO_SCE_MTX)mScreenSpace, (TO_SCE_MTX)&MStack64, (TO_SCE_MTX)&MStack128);
		fVar4 = 1.0 / (farClip - nearClip);
		fVar1 = mScreenSpace->dd + mScreenSpace->cd * nearClip;
		mScreenSpace->cc = fVar4 * (fovDown * (mScreenSpace->dd + mScreenSpace->cd * farClip) - fovUp * fVar1);
		mScreenSpace->dc = fVar4 * (fVar1 * farClip * fovUp - fVar1 * nearClip * fovDown);
		ComputeMapRectangleMatrix_0029b9e0
		(startX * 1.0, width * 1.0, startY * 1.0, height * 1.0, -1.0f, 1.0, -1.0f, 1.0,
			&MStack128);
		sceVu0MulMatrix((TO_SCE_MTX)mTextureSpace, (TO_SCE_MTX)&MStack64, (TO_SCE_MTX)&MStack128);
		fVar2 = nearClip * 1.0;
		fVar1 = mTextureSpace->dd + mTextureSpace->cd * nearClip;
		fVar3 = farClip * -1.0;
		mTextureSpace->cc = fVar4 * ((mTextureSpace->dd + mTextureSpace->cd * farClip) * 1.0 - fVar1 * -1.0);
		mTextureSpace->dc = fVar4 * (fVar1 * fVar3 - fVar1 * fVar2);
		ComputeMapRectangleMatrix_0029b9e0
		(startX * g_RectangleScale_00448980, width * g_RectangleScale_00448980, startY * g_RectangleScale_00448980,
			height * g_RectangleScale_00448980, -1.0f, 1.0, -1.0f, 1.0, &MStack128);
		sceVu0MulMatrix((TO_SCE_MTX)mWorldSpace, (TO_SCE_MTX)&MStack64, (TO_SCE_MTX)&MStack128);
		fVar1 = mWorldSpace->dd + mWorldSpace->cd * nearClip;
		mWorldSpace->cc = fVar4 * ((mWorldSpace->dd + mWorldSpace->cd * farClip) * 1.0 - fVar1 * -1.0);
		mWorldSpace->dc = fVar4 * (fVar1 * fVar3 - fVar1 * fVar2);
		Vector_0048c2d0.z = 2.0 / (fovDown - fovUp);
		Vector_0048c2f0.y = (g_RectangleScale_00448980 * 256.0 + 2048.0) - 1.0;
		Vector_0048c2f0.x = (2048.0 - g_RectangleScale_00448980 * 256.0) + 1.0;
		Vector_0048c2d0.x = 2.0 / (Vector_0048c2f0.y - Vector_0048c2f0.x);
		Vector_0048c2d0.y = 2.0 / (Vector_0048c2f0.x - Vector_0048c2f0.y);
		Vector_0048c2e0.x = (Vector_0048c2f0.x + Vector_0048c2f0.y) / (Vector_0048c2f0.x - Vector_0048c2f0.y);
		Vector_0048c2e0.y = (Vector_0048c2f0.x + Vector_0048c2f0.y) / (Vector_0048c2f0.y - Vector_0048c2f0.x);
		Vector_0048c2d0.w = mScreenSpace->dc + mScreenSpace->cc * nearClip;
		Vector_0048c2e0.z = (fovUp + fovDown) / (fovUp - fovDown);
		Vector_0048c2e0.w = mScreenSpace->dc + mScreenSpace->cc * farClip;
		Vector_0048c2f0.z = Vector_0048c2f0.y;
		Vector_0048c2f0.w = Vector_0048c2f0.x;
		sceVu0UnitMatrix((TO_SCE_MTX)&MStack128);
		sceVu0MulMatrix((TO_SCE_MTX)mUnknown, (TO_SCE_MTX)&MStack64, (TO_SCE_MTX)&MStack128);
		fVar3 = ((ScratchpadAnimManager_004494ec.pData_150)->field_0xe0).z;
		fVar2 = 255.0 - ((ScratchpadAnimManager_004494ec.pData_150)->field_0xe0).x;
		fVar1 = ((255.0 - ((ScratchpadAnimManager_004494ec.pData_150)->field_0xe0).y) - fVar2) /
			(((ScratchpadAnimManager_004494ec.pData_150)->field_0xe0).w - fVar3);
		mUnknown->cc = fVar1;
		mUnknown->dc = fVar2 - fVar1 * fVar3;
		return;
	}

	Matrix g_TransformationMatrix_00489460 = { 0 };
	Matrix Matrix_004894a0 = { 0 };
	Matrix g_ScreenSpaceMatrix_0048cbb0 = { 0 };
	Matrix g_TextureSpaceMatrix_0048cbf0 = { 0 };
	Matrix g_WorldSpaceMatrix_0048cc30 = { 0 };
	Matrix g_TransformationMatrix_0048cc70 = { 0 };
	Matrix Matrix_0048ccb0 = { 0 };
	Matrix Matrix_00489550 = { 0 };

	Vector Vector_0048ccf0 = { 0 };
	Vector Vector_0048cd00 = { 0 };
	Vector Vector_0048cd10 = { 0 };
	Vector Vector_0048cd20 = { 0 };
	Vector Vector_0048cd30 = { 0 };
	Vector Vector_0048cd40 = { 0 };

	TextureData_MAT_Internal_A* PTR_TextureData_MAT_004492ec = NULL;
	TextureData_MAT_Internal_A* PTR_TextureData_MAT_004492f0 = NULL;

	StaticMeshMaster* g_StaticMeshMaster_00449228 = NULL;

	CameraObj_28* g_Camera_0044956c = NULL;

	void SetCamera_002b6d10(CameraObj_28* pCamera)
	{
		g_Camera_0044956c = pCamera;
		return;
	}

	float FLOAT_0044922c = 0.0f;
	float FLOAT_00449230 = 0.0f;
	float FLOAT_00449234 = 0.0f;

	void CameraSetupFunc_0029b120
	(CameraPanStatic_50* param_1, CameraObj_130* param_2, FrameBuffer* pFrameBuffer)
	{
		ushort uVar1;
		float puVar2;
		float puVar3;
		float puVar4;
		float puVar5;
		undefined* puVar6;
		float fVar7;
		float fVar8;
		float fVar9;
		float fVar10;
		float fVar11;
		float fVar12;
		float min;

		fVar12 = param_2->aMatrices[0].dd;
		min = param_2->aMatrices[0].db;
		fVar11 = param_2->aMatrices[0].dc;
		fVar7 = atan2f(min, fVar12);
		fVar8 = atan2f(fVar11, fVar12);
		fVar9 = sinf(fVar7);
		fVar10 = sinf(fVar8);
		fVar7 = cosf(fVar7);
		fVar8 = cosf(fVar8);
		(param_1->field_0x20).field_0x0.aa = fVar7;
		(param_1->field_0x20).field_0x0.ab = 0.0;
		(param_1->field_0x20).field_0x0.ac = fVar9;
		(param_1->field_0x20).field_0x0.ad = 0.0;
		(param_1->field_0x20).field_0x0.ba = -fVar7;
		(param_1->field_0x20).field_0x0.bb = 0.0;
		(param_1->field_0x20).field_0x0.bc = fVar9;
		(param_1->field_0x20).field_0x0.bd = 0.0;
		(param_1->field_0x20).field_0x0.ca = 0.0;
		(param_1->field_0x20).field_0x0.cb = fVar8;
		(param_1->field_0x20).field_0x0.cc = fVar10;
		(param_1->field_0x20).field_0x0.cd = 0.0;
		(param_1->field_0x20).field_0x0.da = 0.0;
		(param_1->field_0x20).field_0x0.db = -fVar8;
		(param_1->field_0x20).field_0x0.dc = fVar10;
		(param_1->field_0x20).field_0x0.dd = 0.0;
		(param_1->field_0x20).field_0x40.x = 0.0;
		(param_1->field_0x20).field_0x40.y = 0.0;
		(param_1->field_0x20).field_0x40.z = 1.0;
		(param_1->field_0x20).field_0x40.w = 0.0;
		(param_1->field_0x20).field_0x50.x = 0.0;
		(param_1->field_0x20).field_0x50.y = 0.0;
		(param_1->field_0x20).field_0x50.z = -1.0f;
		(param_1->field_0x20).field_0x50.w = -param_1->field_0x18;
		uVar1 = pFrameBuffer->pVidModeData_0x0->screenHeight;
		fVar7 = 2048.0 - FLOAT_00449230;
		fVar8 = atan2f
		(((2048.0 - FLOAT_0044922c) * min) /
			((float)(uint)pFrameBuffer->pVidModeData_0x0->screenWidth / 2.0), fVar12);
		fVar7 = atan2f((fVar7 * fVar11) / ((float)(uint)uVar1 / 2.0), fVar12);
		fVar11 = sinf(fVar8);
		fVar12 = sinf(fVar7);
		fVar7 = cosf(fVar7);
		fVar8 = cosf(fVar8);
		(param_1->field_0x80).field_0x0.aa = fVar8;
		(param_1->field_0x80).field_0x0.ab = 0.0;
		(param_1->field_0x80).field_0x0.ac = fVar11;
		(param_1->field_0x80).field_0x0.ad = 0.0;
		(param_1->field_0x80).field_0x0.ba = -fVar8;
		(param_1->field_0x80).field_0x0.bb = 0.0;
		(param_1->field_0x80).field_0x0.bc = fVar11;
		(param_1->field_0x80).field_0x0.bd = 0.0;
		(param_1->field_0x80).field_0x0.ca = 0.0;
		(param_1->field_0x80).field_0x0.cb = fVar7;
		(param_1->field_0x80).field_0x0.cc = fVar12;
		(param_1->field_0x80).field_0x0.cd = 0.0;
		(param_1->field_0x80).field_0x0.da = 0.0;
		(param_1->field_0x80).field_0x0.db = -fVar7;
		(param_1->field_0x80).field_0x0.dc = fVar12;
		(param_1->field_0x80).field_0x0.dd = 0.0;
		(param_1->field_0x80).field_0x40.x = 0.0;
		(param_1->field_0x80).field_0x40.y = 0.0;
		(param_1->field_0x80).field_0x40.z = 1.0;
		(param_1->field_0x80).field_0x40.w = FLOAT_00449234;
		(param_1->field_0x80).field_0x50.x = 0.0;
		(param_1->field_0x80).field_0x50.y = 0.0;
		(param_1->field_0x80).field_0x50.z = -1.0f;
		(param_1->field_0x80).field_0x50.w = -param_1->field_0x18;
		return;
	}

	int SetupCameraObj130_0029a960(StaticMeshMaster* pStaticMeshMaster, long mode)
	{
		uint uVar1;
		Vector* v0;
		float fVar2;
		float fVar3;
		float farClip;
		float fVar5;
		Matrix local_40;

		RENDER_LOG("SetupCameraObj130_0029a960: %d\n", mode);

		fVar5 = pStaticMeshMaster->pCameraObj130_0x8->aMatrices[0].dc;
		memcpy(ScratchpadAnimManager_004494ec.pData_130, pStaticMeshMaster->pCameraObj130_0x8, sizeof(CameraObj_130));
		//MY_LOG("Matrix + 2\n");
		//PRINT_MATRIX((ScratchpadAnimManager_004494ec.pData_130)->aMatrices + 2);
		sceVu0CopyMatrix((TO_SCE_MTX)g_ScratchpadAddr_00448a58, (TO_SCE_MTX)((ScratchpadAnimManager_004494ec.pData_130)->aMatrices + 2));
		if (mode != 0) {
			sceVu0CopyMatrix((TO_SCE_MTX)&Matrix_004894e0, (TO_SCE_MTX)((ScratchpadAnimManager_004494ec.pData_130)->aMatrices + 2));
		}
		((ScratchpadAnimManager_004494ec.pData_130)->vector_0x110).y = 0.0f;
		((ScratchpadAnimManager_004494ec.pData_130)->vector_0x110).x = g_ScratchpadAddr_00448a58->cc;
		((ScratchpadAnimManager_004494ec.pData_130)->vector_0x110).z = -g_ScratchpadAddr_00448a58->ac;
		v0 = &(ScratchpadAnimManager_004494ec.pData_130)->vector_0x110;
		if (((ScratchpadAnimManager_004494ec.pData_130)->vector_0x110).z +
			((ScratchpadAnimManager_004494ec.pData_130)->vector_0x110).x +
			((ScratchpadAnimManager_004494ec.pData_130)->vector_0x110).y == 0.0f) {
			v0->x = 1.0;
		}
		else {
			sceVu0Normalize((float*)v0, (float*)v0);
		}
		sceVu0InverseMatrix(&local_40, g_ScratchpadAddr_00448a58);
		Vector_0048c310.z = local_40.ac;
		Vector_0048c310.w = local_40.ad;
		Vector_0048c310.x = local_40.aa;
		Vector_0048c310.y = local_40.ab;
		//MY_LOG("Vec A\n");
		//PRINT_VECTOR(&Vector_0048c310);
		sceVu0Normalize((float*)&Vector_0048c310, (float*)&Vector_0048c310);
		Vector_0048c320.x = local_40.ba;
		Vector_0048c320.y = local_40.bb;
		Vector_0048c320.z = local_40.bc;
		Vector_0048c320.w = local_40.bd;
		//MY_LOG("Vec B\n");
		//PRINT_VECTOR(&Vector_0048c320);
		sceVu0Normalize((float*)&Vector_0048c320, (float*)&Vector_0048c320);
		Vector_0048c330.x = local_40.ca;
		Vector_0048c330.y = local_40.cb;
		Vector_0048c330.z = local_40.cc;
		Vector_0048c330.w = local_40.cd;
		//MY_LOG("Vec C\n");
		//PRINT_VECTOR(&Vector_0048c330);
		sceVu0Normalize((float*)&Vector_0048c330, (float*)&Vector_0048c330);
		Vector_0048c300.x = (ScratchpadAnimManager_004494ec.pData_130)->aMatrices[0].aa;
		Vector_0048c300.y = (ScratchpadAnimManager_004494ec.pData_130)->aMatrices[0].ab;
		Vector_0048c300.z = (ScratchpadAnimManager_004494ec.pData_130)->aMatrices[0].ac;
		Vector_0048c300.w = (ScratchpadAnimManager_004494ec.pData_130)->aMatrices[0].ad;
		fVar2 = (ScratchpadAnimManager_004494ec.pData_130)->aMatrices[0].dc /
			(ScratchpadAnimManager_004494ec.pData_130)->aMatrices[0].dd;
		FLOAT_0044921c = 1.0f / fVar2;
		FLOAT_00449218 = fVar2 * fVar2;
		if (mode != 0) {
			FLOAT_00449214 = FLOAT_00449218;
		}
		memcpy(&g_ColorBufferCopy_00489520, pStaticMeshMaster->pCameraObj28_0xc->pColorBuffer, sizeof(FrameBuffer));
		g_CameraPanStatic_50_Ptr_00449220 = &pStaticMeshMaster->field_0x50;
		(pStaticMeshMaster->field_0x50).field_0x18 = (pStaticMeshMaster->field_0x50).field_0x4;
		memcpy(ScratchpadAnimManager_004494ec.pData_150, g_CameraPanStatic_50_Ptr_00449220, sizeof(CameraPanStatic_50));
		g_StaticMeshMaster_00449224 = pStaticMeshMaster;
		Func_00293ec0(&(pStaticMeshMaster->field_0x50).field_0x130);
		uVar1 = (ScratchpadAnimManager_004494ec.pData_150)->field_0x14;
		if ((int)uVar1 < 0) {
			fVar2 = (float)(uVar1 >> 1 | uVar1 & 1);
			fVar2 = fVar2 + fVar2;
		}
		else {
			fVar2 = (float)uVar1;
		}
		uVar1 = (ScratchpadAnimManager_004494ec.pData_150)->field_0x10;
		if ((int)uVar1 < 0) {
			fVar3 = (float)(uVar1 >> 1 | uVar1 & 1);
			fVar3 = fVar3 + fVar3;
		}
		else {
			fVar3 = (float)uVar1;
		}
		farClip = (ScratchpadAnimManager_004494ec.pData_150)->farClip;
		ComputeMatrix_0029b610((float)(int)pStaticMeshMaster->pCameraObj28_0xc->screenWidth,
			(float)(int)pStaticMeshMaster->pCameraObj28_0xc->screenHeight,
			(ScratchpadAnimManager_004494ec.pData_130)->aMatrices[0].db,
			(ScratchpadAnimManager_004494ec.pData_130)->aMatrices[0].dc,
			(ScratchpadAnimManager_004494ec.pData_130)->aMatrices[0].dd, 2048.0, 2048.0,
			(ScratchpadAnimManager_004494ec.pData_150)->nearClip, ScratchpadAnimManager_004494ec.pScreenSpaceMatrix,
			ScratchpadAnimManager_004494ec.pTextureSpaceMatrix, ScratchpadAnimManager_004494ec.pWorldSpaceMatrix, &g_TransformationMatrix_00489460,
			farClip, fVar2, fVar3);
		CameraSetupFunc_0029b120
		(ScratchpadAnimManager_004494ec.pData_150, ScratchpadAnimManager_004494ec.pData_130
			, &g_ColorBufferCopy_00489520);

		sceVu0MulMatrix((TO_SCE_MTX)&Matrix_004894a0, (TO_SCE_MTX)g_ScratchpadAddr_00448a58,
			(TO_SCE_MTX)ScratchpadAnimManager_004494ec.pScreenSpaceMatrix);
		if ((pStaticMeshMaster->flags_0x4 & 2) != 0) {
			memcpy(&g_ScreenSpaceMatrix_0048cbb0, ScratchpadAnimManager_004494ec.pScreenSpaceMatrix, sizeof(Matrix));
			memcpy(&g_TextureSpaceMatrix_0048cbf0, ScratchpadAnimManager_004494ec.pTextureSpaceMatrix, sizeof(Matrix));
			memcpy(&g_WorldSpaceMatrix_0048cc30, ScratchpadAnimManager_004494ec.pWorldSpaceMatrix, sizeof(Matrix));
			memcpy(&g_TransformationMatrix_0048cc70, &g_TransformationMatrix_00489460, sizeof(Matrix));
			memcpy(&Matrix_0048ccb0, g_ScratchpadAddr_00448a58, sizeof(Matrix));
			memcpy(&Vector_0048ccf0, &Vector_0048c2d0, sizeof(Vector));
			memcpy(&Vector_0048cd00, &Vector_0048c2e0, sizeof(Vector));
			memcpy(&Vector_0048cd10, &Vector_0048c2f0, sizeof(Vector));
			memcpy(&Vector_0048cd20, &Vector_0048c300, sizeof(Vector));
			memcpy(&Vector_0048cd30, &Vector_0048c310, sizeof(Vector));
			memcpy(&Vector_0048cd40, &Vector_0048c320, sizeof(Vector));
		}
		if (pStaticMeshMaster->isChild == 1) {
			uVar1 = ToPowerOfTwo_0029e3f0((int)pStaticMeshMaster->pCameraObj28_0xc->screenWidth);
			if ((int)uVar1 < 0) {
				fVar2 = (float)(uVar1 >> 1 | uVar1 & 1);
				fVar2 = fVar2 + fVar2;
			}
			else {
				fVar2 = (float)uVar1;
			}
			uVar1 = ToPowerOfTwo_0029e3f0((int)pStaticMeshMaster->pCameraObj28_0xc->screenHeight);
			if ((int)uVar1 < 0) {
				fVar3 = (float)(uVar1 >> 1 | uVar1 & 1);
				fVar3 = fVar3 + fVar3;
			}
			else {
				fVar3 = (float)uVar1;
			}
			PTR_TextureData_MAT_004492ec = &TextureData_MAT_Internal_A_0048c3b0;
			PTR_TextureData_MAT_004492f0 = &TextureData_MAT_Internal_A_0048c3c0;
			Vector_0048c300.x = 2048.0f - fVar2 / 2.0f;
			Vector_0048c300.y = 2048.0f - fVar3 / 2.0f;
			Vector_0048c300.z = 1.0f / fVar2;
			Vector_0048c300.w = 1.0f / fVar3;
			SetCamera_002b6d10(((ScratchpadAnimManager_004494ec.pData_150)->field_0x100).pCamera_0x14);
			*ScratchpadAnimManager_004494ec.field_0x28 =
				(uint)(ushort)((ScratchpadAnimManager_004494ec.pData_150)->field_0x100).field_0x20;
			if (*ScratchpadAnimManager_004494ec.field_0x28 == 0) {
				return 0;
			}
			memcpy(&Matrix_00489550, g_ScratchpadAddr_00448a58, sizeof(Matrix));
			g_StaticMeshMaster_00449228 = pStaticMeshMaster;
		}
		else {
			*ScratchpadAnimManager_004494ec.field_0x28 = 0;
			PTR_TextureData_MAT_004492ec = NULL; //&TamPacked_0048c370;
			PTR_TextureData_MAT_004492f0 = NULL; //&TamPacked_0048c380;
		}
		if (pStaticMeshMaster->pCameraObj28_0xc->screenHeight != 0x200) {
			pStaticMeshMaster->pCameraObj130_0x8->aMatrices[0].dc = fVar5;
		}
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
		RenderCommand* pRVar1;
		pRVar1 = g_VideoMemoryPtr_00449280;
		sceDmaAddRef((sceDmaTag**)&g_VideoMemoryPtr_00449280, 1, (void*)g_StaticVideoList_00489590);
		pRVar1->cmdB = 0x0;
		g_VideoMemoryPtr_00449280 = FUN_0029c640(g_VideoMemoryPtr_00449280);
		return;
	}

	RenderCommand* SetupDrawEnvironment_002ab2d0(CameraObj_28* pCamera, RenderCommand* pCommandBuffer)
	{
		RenderCommand* pRVar1;
		uint uVar2;

		pRVar1 = pCommandBuffer;
		if (pCamera != (CameraObj_28*)0x0) {
			pRVar1 = BuildCameraCommands_002bb110(pCamera, pCommandBuffer + 1);
			uVar2 = ((uint)((ulong)pRVar1 - (ulong)pCommandBuffer) >> 4) - 1;
			pCommandBuffer->cmdA = (ulong)(ulong)uVar2 & 0xffffffffU | 0xe10000000;
			pCommandBuffer->cmdB = 0;
			*(undefined4*)&pCommandBuffer->cmdB = 0;
			*(uint*)((ulong)&pCommandBuffer->cmdB + 4) = uVar2 | 0x51000000;
		}
		return pRVar1;
	}

	RenderCommand* SetupDrawEnvironment_002b1990(StaticMeshMaster* param_1, RenderCommand* pInCommandBuffer)
	{
		RenderCommand* pRVar1;

		RENDER_LOG("SetupDrawEnvironment_002b1990 %p\n", param_1);

		pInCommandBuffer->cmdA = 0xe10000003;
		pInCommandBuffer->cmdB = 0;
		*(undefined4*)&pInCommandBuffer->cmdB = 0x40003dc;
		*(undefined4*)((ulong)&pInCommandBuffer->cmdB + 4) = 0x6c0203dc;

		pInCommandBuffer[1].cmdA = SCE_GIF_SET_TAG(
			1,					// NLOOP
			SCE_GS_TRUE,		// EOP
			SCE_GS_FALSE,		// PRE
			0,					// PRIM
			SCE_GIF_PACKED,		// FLG
			1					// NREG
		);
		pInCommandBuffer[1].cmdB = SCE_GIF_PACKED_AD;

		pInCommandBuffer[2].cmdA = SCE_GS_SET_FOGCOL(param_1->field_0x50.fogCol_0xf0.r, param_1->field_0x50.fogCol_0xf0.g, param_1->field_0x50.fogCol_0xf0.b);
		pInCommandBuffer[2].cmdB = SCE_GS_FOGCOL;

		*(undefined4*)&pInCommandBuffer[3].cmdA = 0;
		*(undefined4*)((ulong)&pInCommandBuffer[3].cmdA + 4) = 0x14000000;
		pInCommandBuffer[3].cmdB = 0x11000000;
		pRVar1 = pInCommandBuffer + 4;
		if (param_1->pCameraObj28_0xc != (CameraObj_28*)0x0) {
			pRVar1 = SetupDrawEnvironment_002ab2d0(param_1->pCameraObj28_0xc, pRVar1);
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

	void FUN_002ad7b0(StaticMeshMaster* pStaticMeshMaster)
	{
		int iVar1;
		int iVar2;
		int* piVar3;
		uint uVar4;

		piVar3 = pStaticMeshMaster->field_0x1ac;
		for (uVar4 = 0; uVar4 < pStaticMeshMaster->count_0x1b0; uVar4 = uVar4 + 1) {
			iVar1 = *piVar3;
			for (iVar2 = *(int*)(iVar1 + 4); iVar2 != iVar1; iVar2 = *(int*)(iVar2 + 4)) {
				IMPLEMENTATION_GUARD(
				MeshTransformData::GenerateTransform_002b48f0(*(MeshTransformData**)(iVar2 + 0xc));)
			}
			piVar3 = piVar3 + 1;
		}
		return;
	}

	PACK(struct MeshData_CSTA {
		char CSTA_Header[4];
		undefined field_0x4;
		undefined field_0x5;
		undefined field_0x6;
		undefined field_0x7;
		undefined field_0x8;
		undefined field_0x9;
		undefined field_0xa;
		undefined field_0xb;
		undefined field_0xc;
		undefined field_0xd;
		undefined field_0xe;
		undefined field_0xf;
		int field_0x10;
		undefined field_0x14;
		undefined field_0x15;
		undefined field_0x16;
		undefined field_0x17;
		undefined field_0x18;
		undefined field_0x19;
		undefined field_0x1a;
		undefined field_0x1b;
		undefined field_0x1c;
		undefined field_0x1d;
		undefined field_0x1e;
		undefined field_0x1f;
		float field_0x20;
		float field_0x24;
		float field_0x28;
		undefined field_0x2c;
		undefined field_0x2d;
		undefined field_0x2e;
		undefined field_0x2f;
		Vector field_0x30;
	};)

	void ProbablySortDepth_002a52a0(StaticMeshMaster* pStaticMeshMaster)
	{
		bool bVar1;
		CameraPanMasterHeader* pCVar2;
		CameraPanMasterHeader* pCVar3;
		CameraPanMasterHeader* pCVar4;
		Vector local_10;
		DisplayListInternalMesh* pMVar2;
		MeshData_CSTA* pcVar1;
		DisplayListInternalMesh* pMVar1;

		bVar1 = true;
		if ((pStaticMeshMaster->flags_0x4 & 0x400) == 0) {
			pCVar4 = &pStaticMeshMaster->headerB;
		}
		else {
			pCVar4 = pStaticMeshMaster->field_0x1a0;
		}
		pCVar3 = (CameraPanMasterHeader*)pCVar4->pLoadedData;
		if (pCVar3 != pCVar4) {
			for (; pCVar3 != pCVar4; pCVar3 = (CameraPanMasterHeader*)pCVar3->pLoadedData) {
				pMVar2 = (DisplayListInternalMesh*)pCVar3->pCameraPanHeader_0xc;
				pcVar1 = (MeshData_CSTA*)pMVar2->pMeshInfo->CSTA;
				if (pcVar1 != (MeshData_CSTA*)0x0) {
					ps2_vu0_sub_vector(&local_10, (Vector*)pStaticMeshMaster->pCameraObj130_0x8->aMatrices, &pcVar1->field_0x30);
					pMVar2->field_0x18 = local_10.z * local_10.z + local_10.x * local_10.x + local_10.y * local_10.y;
				}
			}
			while (bVar1) {
				bVar1 = false;
				pCVar2 = (CameraPanMasterHeader*)pCVar4->pLoadedData;
				for (pCVar3 = (CameraPanMasterHeader*)((CameraPanMasterHeader*)pCVar4->pLoadedData)->pLoadedData;
					pCVar3 != pCVar4; pCVar3 = (CameraPanMasterHeader*)pCVar3->pLoadedData) {
					pMVar1 = (DisplayListInternalMesh*)pCVar2->pCameraPanHeader_0xc;
					if (pMVar1->field_0x18 < ((DisplayListInternalMesh*)pCVar3->pCameraPanHeader_0xc)->field_0x18) {
						pCVar2->pCameraPanHeader_0xc = pCVar3->pCameraPanHeader_0xc;
						bVar1 = true;
						pCVar3->pCameraPanHeader_0xc = (MeshTransformParentHeader*)pMVar1;
					}
					pCVar2 = pCVar3;
				}
			}
		}
		return;
	}

	int g_CurrentDrawType_0044923c = 0;
	float FLOAT_00448a04 = 0.01f;

	byte BYTE_004489b8 = 0;

	TextureData_LAY_Internal* g_TextureData_LAY_Int_00449404 = NULL;
	char* PTR_DAT_0044940c = NULL;

	void GetTEX2D_LAY_0029f170(RenderFrame_30* param_1, char** ppOutTEX2D, TextureData_LAY_Internal** ppOutLAY, int index)
	{
		*ppOutLAY = &((TextureData_LAY*)LOAD_SECTION(*(int*)(param_1->pFileData + index * 4 + 0x10)))->body;
		TextureData_TEX* pTEX = (TextureData_TEX*)LOAD_SECTION((*ppOutLAY)->pTex);
		char* b = (char*)LOAD_SECTION(pTEX->body.field_0x8);
		char* pTEX2D = (char*)LOAD_SECTION(*(int*)(b + 8));
		*ppOutTEX2D = (char*)(pTEX2D + 0x10);
		return;
	}

	byte g_bGeneratingMeshCommands_00449570 = 0;
	int INT_00449238 = 0;

	RenderCommand* RenderCommandLayer_002a9410(RenderCommand* param_1, TextureData_LAY_Internal* param_2)
	{
		if (((g_RenderFrameData_004493f8 == (RenderFrame_30*)0x0) || ((g_RenderFrameData_004493f8->flags_0x28 & 1U) != 0)) &&
			(g_RenderFrameData_004493f8 != (RenderFrame_30*)0x0)) {
			BYTE_004489b8 = 0x80;
		}
		else {
			if (((g_RenderMeshTransform_004493f4 == (MeshTransformDataBase*)0x0) ||
				(g_RenderMeshTransform_004493f4 == (MeshTransformDataBase*)0x0000ffff)) ||
				(g_RenderMeshTransform_004493f4->field_0xaf == 0xff)) {
				if (((BYTE_004489b8 != 0x80) && (g_RenderMeshTransform_004493f4 != (MeshTransformDataBase*)0x0)) &&
					(((g_RenderMeshTransform_004493f4 != (MeshTransformDataBase*)0x0000ffff &&
						(((g_RenderMeshTransform_004493f4->field_0xaf == -1 &&
							(g_RenderActiveCameraObj_006f7d80 != (CameraObj_28*)0x0)) &&
							(BYTE_004489b8 = 0x80, param_2 != (TextureData_LAY_Internal*)0x0)))) && ((param_2->field_0x0 & 0xfc) == 0))
						)) {
					param_1->cmdA = 0x30000000;
					param_1->cmdB = 0x1300000000000000;
					param_1[1].cmdA = 0xe10000003;
					param_1[1].cmdB = 0;
					*(undefined4*)&param_1[1].cmdB = 0;
					*(undefined4*)((ulong)&param_1[1].cmdB + 4) = 0x50000003;
					param_1[2].cmdA = 0x1000000000008002;
					param_1[2].cmdB = 0xe;
					param_1[3].cmdA = 0x5001b;
					param_1[3].cmdB = 0x47;
					param_1[4].cmdA = 0x2a;
					param_1[4].cmdB = 0x42;
					param_1 = param_1 + 5;
				}
			}
			else {
				if (g_RenderActiveCameraObj_006f7d80 != (CameraObj_28*)0x0) {
					BYTE_004489b8 = g_RenderMeshTransform_004493f4->field_0xaf;
					IMPLEMENTATION_GUARD( param_1 = RenderCommandLayer_002a9560(param_1, param_2));
				}
			}
		}
		return param_1;
	}
	
	void sceVu0ScaleVectorAlt(float t, Vector* v0, Vector* v1)
	{
		float fVar1;
		float fVar2;
		float fVar3;

		fVar1 = v1->y;
		fVar2 = v1->z;
		fVar3 = v1->w;
		v0->x = v1->x * t;
		v0->y = fVar1 * t;
		v0->z = fVar2 * t;
		v0->w = fVar3 * t;
		return;
	}

	void sceVu0MatrixFunc_00267300(sceVu0FMATRIX m0, sceVu0FMATRIX m1)
	{
		float fVar1;
		float fVar2;
		float fVar3;
		float fVar4;
		float fVar5;
		float fVar6;

		fVar1 = (m1)[0][0];
		fVar2 = (m1)[0][1];
		fVar3 = (m1)[0][2];
		fVar6 = 1.0 / (sqrt(fVar1 * fVar1 + fVar2 * fVar2 + fVar3 * fVar3) + 0.0);
		(m0)[0][0] = fVar1 * fVar6;
		(m0)[0][1] = fVar2 * fVar6;
		(m0)[0][2] = fVar3 * fVar6;
		(m0)[0][3] = 0.0;
		fVar1 = (m0)[0][0];
		fVar2 = (m0)[0][1];
		fVar3 = (m0)[0][2];
		fVar6 = (m1)[1][0];
		fVar4 = (m1)[1][1];
		fVar5 = (m1)[1][2];
		(m0)[2][0] = fVar2 * fVar5 - fVar4 * fVar3;
		(m0)[2][1] = fVar3 * fVar6 - fVar5 * fVar1;
		(m0)[2][2] = fVar1 * fVar4 - fVar6 * fVar2;
		(m0)[2][3] = 0.0;
		fVar1 = (m0)[2][0];
		fVar2 = (m0)[2][1];
		fVar3 = (m0)[2][2];
		fVar6 = 1.0 / (sqrt(fVar1 * fVar1 + fVar2 * fVar2 + fVar3 * fVar3) + 0.0);
		(m0)[2][0] = fVar1 * fVar6;
		(m0)[2][1] = fVar2 * fVar6;
		(m0)[2][2] = fVar3 * fVar6;
		(m0)[2][3] = 0.0;
		fVar1 = (m0)[2][0];
		fVar2 = (m0)[2][1];
		fVar3 = (m0)[2][2];
		fVar6 = (m0)[0][0];
		fVar4 = (m0)[0][1];
		fVar5 = (m0)[0][2];
		(m0)[1][0] = fVar2 * fVar5 - fVar4 * fVar3;
		(m0)[1][1] = fVar3 * fVar6 - fVar5 * fVar1;
		(m0)[1][2] = fVar1 * fVar4 - fVar6 * fVar2;
		(m0)[1][3] = 0.0;
		fVar3 = (m1)[3][1];
		fVar1 = (m1)[3][2];
		fVar2 = (m1)[3][3];
		(m0)[3][0] = (m1)[3][0];
		(m0)[3][1] = fVar3;
		(m0)[3][2] = fVar1;
		(m0)[3][3] = fVar2;
		return;
	}

#ifdef PLATFORM_WIN
	Vector Vector_ARRAY_70000800[2] = { 0 };
	Vector Vector_ARRAY_70000990[3] = { 0 };
	Vector Vector_ARRAY_70000920[7] = { 0 };
	Matrix g_RenderMData[4] = { 0 };
	Matrix Matrix_70000a00 = { 0 };
	Matrix* Matrix_70000a00_PTR = &Matrix_70000a00;
#else
	Vector* Vector_ARRAY_70000800 = (Vector*)0x70000800;
	Vector* Vector_ARRAY_70000990 = (Vector*)0x70000990;
	Vector* Vector_ARRAY_70000920 = (Vector*)0x70000920;
	Matrix* g_RenderMData = (Matrix*)0x70000820;
	Matrix* Matrix_70000a00_PTR = (Matrix*)0x70000a00;
#endif

	// These are in the scratchpad
	Matrix* PTR_Matrix_70000a40 = NULL;
	Matrix* PTR_Matrix_70000a50 = NULL;
	Vector* PTR_Vector_70000a60 = NULL;

	Matrix* LightingMatrixFunc_002a8ab0(Matrix* pMatrixBuffer, RenderFrame_30* pRenderData, byte param_3)
	{
		Matrix* m1;
		LightingMatrixSubSubObj* pLVar1;
		LightingMatrixFuncObj* pLVar2;
		float* pfVar3;
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
		float fVar14;
		float fVar15;
		float fVar16;
		float fVar17;
		float fVar18;
		float fVar19;
		float fVar20;
		float fVar21;
		float fVar22;
		float fVar23;
		float fVar24;
		float fVar25;
		float fVar26;
		float fVar27;
		float fVar28;
		float fVar29;
		float fVar30;
		float fVar31;
		float fVar32;
		float fVar33;
		float fVar34;
		float fVar35;
		undefined* psVar1;
		LightingMatrixFuncObj* puVar1;

		m1 = pRenderData->pTransformMatrix;
		shellDmaSync(SHELLDMA_CHANNEL_VIF0);
		sceVu0ScaleVectorAlt(pRenderData->field_0x2c.floatValue, Vector_ARRAY_70000800, &Vector_0048c310);
		sceVu0ScaleVectorAlt(pRenderData->field_0x2c.floatValue, Vector_ARRAY_70000800 + 1, &Vector_0048c320);
		*ScratchpadAnimManager_004494ec.field_0x20 = m1;
		if (((pRenderData->pMeshTransformData == (MeshTransformDataBase*)0x0) ||
			(puVar1 = pRenderData->pMeshTransformData->pLightingMatrixFuncObj_0xa0, puVar1 == (LightingMatrixFuncObj*)0x0))
			|| (pLVar1 = puVar1->field_0x8, pLVar1 == (LightingMatrixSubSubObj*)0x0)) {
			PTR_Matrix_70000a40 = ScratchpadAnimManager_004494ec.pMatrix_0xc;
			PTR_Matrix_70000a50 = ScratchpadAnimManager_004494ec.pMatrix_0x10;
			PTR_Vector_70000a60 = ScratchpadAnimManager_004494ec.pVector_0x14;
		}
		else {
			PTR_Matrix_70000a40 = pLVar1->field_0x4;
			if (PTR_Matrix_70000a40 == (Matrix*)0x0) {
				PTR_Matrix_70000a40 = ScratchpadAnimManager_004494ec.pMatrix_0xc;
			}
			PTR_Matrix_70000a50 = pLVar1->field_0x8;
			if (PTR_Matrix_70000a50 == (Matrix*)0x0) {
				PTR_Matrix_70000a50 = ScratchpadAnimManager_004494ec.pMatrix_0x10;
			}
			PTR_Vector_70000a60 = pLVar1->field_0x0;
			if (PTR_Vector_70000a60 == (Vector*)0x0) {
				PTR_Vector_70000a60 = ScratchpadAnimManager_004494ec.pVector_0x14;
			}
		}
		Vector_ARRAY_70000990[0].x = ((ScratchpadAnimManager_004494ec.pData_130)->vector_0x110).x;
		Vector_ARRAY_70000990[0].z = ((ScratchpadAnimManager_004494ec.pData_130)->vector_0x110).z;
		Vector_ARRAY_70000990[0].y = 0.0;
		if (param_3 == 0) {
			Vector_ARRAY_70000990[0].w = 0.0;
		}
		else {
			Vector_ARRAY_70000990[0].w = 999.0;
			m1 = ScratchpadAnimManager_004494ec.pMatrix_0x30;
		}
		fVar20 = m1->aa;
		fVar21 = m1->ab;
		fVar22 = m1->ac;
		fVar23 = m1->ad;
		fVar24 = m1->ba;
		fVar25 = m1->bb;
		fVar26 = m1->bc;
		fVar27 = m1->bd;
		fVar28 = m1->ca;
		fVar29 = m1->cb;
		fVar30 = m1->cc;
		fVar31 = m1->cd;
		fVar32 = m1->da;
		fVar33 = m1->db;
		fVar34 = m1->dc;
		fVar35 = m1->dd;
		fVar4 = g_ScratchpadAddr_00448a58->aa;
		fVar5 = g_ScratchpadAddr_00448a58->ab;
		fVar6 = g_ScratchpadAddr_00448a58->ac;
		fVar7 = g_ScratchpadAddr_00448a58->ad;
		fVar8 = g_ScratchpadAddr_00448a58->ba;
		fVar9 = g_ScratchpadAddr_00448a58->bb;
		fVar10 = g_ScratchpadAddr_00448a58->bc;
		fVar11 = g_ScratchpadAddr_00448a58->bd;
		fVar12 = g_ScratchpadAddr_00448a58->ca;
		fVar13 = g_ScratchpadAddr_00448a58->cb;
		fVar14 = g_ScratchpadAddr_00448a58->cc;
		fVar15 = g_ScratchpadAddr_00448a58->cd;
		fVar16 = g_ScratchpadAddr_00448a58->da;
		fVar17 = g_ScratchpadAddr_00448a58->db;
		fVar18 = g_ScratchpadAddr_00448a58->dc;
		fVar19 = g_ScratchpadAddr_00448a58->dd;
		g_RenderMData[3].aa = fVar4 * fVar20 + fVar8 * fVar21 + fVar12 * fVar22 + fVar16 * fVar23;
		g_RenderMData[3].ab = fVar5 * fVar20 + fVar9 * fVar21 + fVar13 * fVar22 + fVar17 * fVar23;
		g_RenderMData[3].ac = fVar6 * fVar20 + fVar10 * fVar21 + fVar14 * fVar22 + fVar18 * fVar23;
		g_RenderMData[3].ad = fVar7 * fVar20 + fVar11 * fVar21 + fVar15 * fVar22 + fVar19 * fVar23;
		g_RenderMData[3].ba = fVar4 * fVar24 + fVar8 * fVar25 + fVar12 * fVar26 + fVar16 * fVar27;
		g_RenderMData[3].bb = fVar5 * fVar24 + fVar9 * fVar25 + fVar13 * fVar26 + fVar17 * fVar27;
		g_RenderMData[3].bc = fVar6 * fVar24 + fVar10 * fVar25 + fVar14 * fVar26 + fVar18 * fVar27;
		g_RenderMData[3].bd = fVar7 * fVar24 + fVar11 * fVar25 + fVar15 * fVar26 + fVar19 * fVar27;
		g_RenderMData[3].ca = fVar4 * fVar28 + fVar8 * fVar29 + fVar12 * fVar30 + fVar16 * fVar31;
		g_RenderMData[3].cb = fVar5 * fVar28 + fVar9 * fVar29 + fVar13 * fVar30 + fVar17 * fVar31;
		g_RenderMData[3].cc = fVar6 * fVar28 + fVar10 * fVar29 + fVar14 * fVar30 + fVar18 * fVar31;
		g_RenderMData[3].cd = fVar7 * fVar28 + fVar11 * fVar29 + fVar15 * fVar30 + fVar19 * fVar31;
		g_RenderMData[3].da = fVar4 * fVar32 + fVar8 * fVar33 + fVar12 * fVar34 + fVar16 * fVar35;
		g_RenderMData[3].db = fVar5 * fVar32 + fVar9 * fVar33 + fVar13 * fVar34 + fVar17 * fVar35;
		g_RenderMData[3].dc = fVar6 * fVar32 + fVar10 * fVar33 + fVar14 * fVar34 + fVar18 * fVar35;
		g_RenderMData[3].dd = fVar7 * fVar32 + fVar11 * fVar33 + fVar15 * fVar34 + fVar19 * fVar35;
		fVar4 = (ScratchpadAnimManager_004494ec.pTextureSpaceMatrix)->aa;
		fVar5 = (ScratchpadAnimManager_004494ec.pTextureSpaceMatrix)->ab;
		fVar6 = (ScratchpadAnimManager_004494ec.pTextureSpaceMatrix)->ac;
		fVar7 = (ScratchpadAnimManager_004494ec.pTextureSpaceMatrix)->ad;
		fVar8 = (ScratchpadAnimManager_004494ec.pTextureSpaceMatrix)->ba;
		fVar9 = (ScratchpadAnimManager_004494ec.pTextureSpaceMatrix)->bb;
		fVar10 = (ScratchpadAnimManager_004494ec.pTextureSpaceMatrix)->bc;
		fVar11 = (ScratchpadAnimManager_004494ec.pTextureSpaceMatrix)->bd;
		fVar12 = (ScratchpadAnimManager_004494ec.pTextureSpaceMatrix)->ca;
		fVar13 = (ScratchpadAnimManager_004494ec.pTextureSpaceMatrix)->cb;
		fVar14 = (ScratchpadAnimManager_004494ec.pTextureSpaceMatrix)->cc;
		fVar15 = (ScratchpadAnimManager_004494ec.pTextureSpaceMatrix)->cd;
		fVar16 = (ScratchpadAnimManager_004494ec.pTextureSpaceMatrix)->da;
		fVar17 = (ScratchpadAnimManager_004494ec.pTextureSpaceMatrix)->db;
		fVar18 = (ScratchpadAnimManager_004494ec.pTextureSpaceMatrix)->dc;
		fVar19 = (ScratchpadAnimManager_004494ec.pTextureSpaceMatrix)->dd;
		g_RenderMData[0].aa =
			fVar4 * g_RenderMData[3].aa + fVar8 * g_RenderMData[3].ab + fVar12 * g_RenderMData[3].ac +
			fVar16 * g_RenderMData[3].ad;
		g_RenderMData[0].ab =
			fVar5 * g_RenderMData[3].aa + fVar9 * g_RenderMData[3].ab + fVar13 * g_RenderMData[3].ac +
			fVar17 * g_RenderMData[3].ad;
		g_RenderMData[0].ac =
			fVar6 * g_RenderMData[3].aa + fVar10 * g_RenderMData[3].ab + fVar14 * g_RenderMData[3].ac +
			fVar18 * g_RenderMData[3].ad;
		g_RenderMData[0].ad =
			fVar7 * g_RenderMData[3].aa + fVar11 * g_RenderMData[3].ab + fVar15 * g_RenderMData[3].ac +
			fVar19 * g_RenderMData[3].ad;
		g_RenderMData[0].ba =
			fVar4 * g_RenderMData[3].ba + fVar8 * g_RenderMData[3].bb + fVar12 * g_RenderMData[3].bc +
			fVar16 * g_RenderMData[3].bd;
		g_RenderMData[0].bb =
			fVar5 * g_RenderMData[3].ba + fVar9 * g_RenderMData[3].bb + fVar13 * g_RenderMData[3].bc +
			fVar17 * g_RenderMData[3].bd;
		g_RenderMData[0].bc =
			fVar6 * g_RenderMData[3].ba + fVar10 * g_RenderMData[3].bb + fVar14 * g_RenderMData[3].bc +
			fVar18 * g_RenderMData[3].bd;
		g_RenderMData[0].bd =
			fVar7 * g_RenderMData[3].ba + fVar11 * g_RenderMData[3].bb + fVar15 * g_RenderMData[3].bc +
			fVar19 * g_RenderMData[3].bd;
		g_RenderMData[0].ca =
			fVar4 * g_RenderMData[3].ca + fVar8 * g_RenderMData[3].cb + fVar12 * g_RenderMData[3].cc +
			fVar16 * g_RenderMData[3].cd;
		g_RenderMData[0].cb =
			fVar5 * g_RenderMData[3].ca + fVar9 * g_RenderMData[3].cb + fVar13 * g_RenderMData[3].cc +
			fVar17 * g_RenderMData[3].cd;
		g_RenderMData[0].cc =
			fVar6 * g_RenderMData[3].ca + fVar10 * g_RenderMData[3].cb + fVar14 * g_RenderMData[3].cc +
			fVar18 * g_RenderMData[3].cd;
		g_RenderMData[0].cd =
			fVar7 * g_RenderMData[3].ca + fVar11 * g_RenderMData[3].cb + fVar15 * g_RenderMData[3].cc +
			fVar19 * g_RenderMData[3].cd;
		g_RenderMData[0].da =
			fVar4 * g_RenderMData[3].da + fVar8 * g_RenderMData[3].db + fVar12 * g_RenderMData[3].dc +
			fVar16 * g_RenderMData[3].dd;
		g_RenderMData[0].db =
			fVar5 * g_RenderMData[3].da + fVar9 * g_RenderMData[3].db + fVar13 * g_RenderMData[3].dc +
			fVar17 * g_RenderMData[3].dd;
		g_RenderMData[0].dc =
			fVar6 * g_RenderMData[3].da + fVar10 * g_RenderMData[3].db + fVar14 * g_RenderMData[3].dc +
			fVar18 * g_RenderMData[3].dd;
		g_RenderMData[0].dd =
			fVar7 * g_RenderMData[3].da + fVar11 * g_RenderMData[3].db + fVar15 * g_RenderMData[3].dc +
			fVar19 * g_RenderMData[3].dd;
		fVar4 = (ScratchpadAnimManager_004494ec.pWorldSpaceMatrix)->aa;
		fVar5 = (ScratchpadAnimManager_004494ec.pWorldSpaceMatrix)->ab;
		fVar6 = (ScratchpadAnimManager_004494ec.pWorldSpaceMatrix)->ac;
		fVar7 = (ScratchpadAnimManager_004494ec.pWorldSpaceMatrix)->ad;
		fVar8 = (ScratchpadAnimManager_004494ec.pWorldSpaceMatrix)->ba;
		fVar9 = (ScratchpadAnimManager_004494ec.pWorldSpaceMatrix)->bb;
		fVar10 = (ScratchpadAnimManager_004494ec.pWorldSpaceMatrix)->bc;
		fVar11 = (ScratchpadAnimManager_004494ec.pWorldSpaceMatrix)->bd;
		fVar12 = (ScratchpadAnimManager_004494ec.pWorldSpaceMatrix)->ca;
		fVar13 = (ScratchpadAnimManager_004494ec.pWorldSpaceMatrix)->cb;
		fVar14 = (ScratchpadAnimManager_004494ec.pWorldSpaceMatrix)->cc;
		fVar15 = (ScratchpadAnimManager_004494ec.pWorldSpaceMatrix)->cd;
		fVar16 = (ScratchpadAnimManager_004494ec.pWorldSpaceMatrix)->da;
		fVar17 = (ScratchpadAnimManager_004494ec.pWorldSpaceMatrix)->db;
		fVar18 = (ScratchpadAnimManager_004494ec.pWorldSpaceMatrix)->dc;
		fVar19 = (ScratchpadAnimManager_004494ec.pWorldSpaceMatrix)->dd;
		g_RenderMData[1].aa =
			fVar4 * g_RenderMData[3].aa + fVar8 * g_RenderMData[3].ab + fVar12 * g_RenderMData[3].ac +
			fVar16 * g_RenderMData[3].ad;
		g_RenderMData[1].ab =
			fVar5 * g_RenderMData[3].aa + fVar9 * g_RenderMData[3].ab + fVar13 * g_RenderMData[3].ac +
			fVar17 * g_RenderMData[3].ad;
		g_RenderMData[1].ac =
			fVar6 * g_RenderMData[3].aa + fVar10 * g_RenderMData[3].ab + fVar14 * g_RenderMData[3].ac +
			fVar18 * g_RenderMData[3].ad;
		g_RenderMData[1].ad =
			fVar7 * g_RenderMData[3].aa + fVar11 * g_RenderMData[3].ab + fVar15 * g_RenderMData[3].ac +
			fVar19 * g_RenderMData[3].ad;
		g_RenderMData[1].ba =
			fVar4 * g_RenderMData[3].ba + fVar8 * g_RenderMData[3].bb + fVar12 * g_RenderMData[3].bc +
			fVar16 * g_RenderMData[3].bd;
		g_RenderMData[1].bb =
			fVar5 * g_RenderMData[3].ba + fVar9 * g_RenderMData[3].bb + fVar13 * g_RenderMData[3].bc +
			fVar17 * g_RenderMData[3].bd;
		g_RenderMData[1].bc =
			fVar6 * g_RenderMData[3].ba + fVar10 * g_RenderMData[3].bb + fVar14 * g_RenderMData[3].bc +
			fVar18 * g_RenderMData[3].bd;
		g_RenderMData[1].bd =
			fVar7 * g_RenderMData[3].ba + fVar11 * g_RenderMData[3].bb + fVar15 * g_RenderMData[3].bc +
			fVar19 * g_RenderMData[3].bd;
		g_RenderMData[1].ca =
			fVar4 * g_RenderMData[3].ca + fVar8 * g_RenderMData[3].cb + fVar12 * g_RenderMData[3].cc +
			fVar16 * g_RenderMData[3].cd;
		g_RenderMData[1].cb =
			fVar5 * g_RenderMData[3].ca + fVar9 * g_RenderMData[3].cb + fVar13 * g_RenderMData[3].cc +
			fVar17 * g_RenderMData[3].cd;
		g_RenderMData[1].cc =
			fVar6 * g_RenderMData[3].ca + fVar10 * g_RenderMData[3].cb + fVar14 * g_RenderMData[3].cc +
			fVar18 * g_RenderMData[3].cd;
		g_RenderMData[1].cd =
			fVar7 * g_RenderMData[3].ca + fVar11 * g_RenderMData[3].cb + fVar15 * g_RenderMData[3].cc +
			fVar19 * g_RenderMData[3].cd;
		g_RenderMData[1].da =
			fVar4 * g_RenderMData[3].da + fVar8 * g_RenderMData[3].db + fVar12 * g_RenderMData[3].dc +
			fVar16 * g_RenderMData[3].dd;
		g_RenderMData[1].db =
			fVar5 * g_RenderMData[3].da + fVar9 * g_RenderMData[3].db + fVar13 * g_RenderMData[3].dc +
			fVar17 * g_RenderMData[3].dd;
		g_RenderMData[1].dc =
			fVar6 * g_RenderMData[3].da + fVar10 * g_RenderMData[3].db + fVar14 * g_RenderMData[3].dc +
			fVar18 * g_RenderMData[3].dd;
		g_RenderMData[1].dd =
			fVar7 * g_RenderMData[3].da + fVar11 * g_RenderMData[3].db + fVar15 * g_RenderMData[3].dc +
			fVar19 * g_RenderMData[3].dd;
		fVar4 = (ScratchpadAnimManager_004494ec.pScreenSpaceMatrix)->aa;
		fVar5 = (ScratchpadAnimManager_004494ec.pScreenSpaceMatrix)->ab;
		fVar6 = (ScratchpadAnimManager_004494ec.pScreenSpaceMatrix)->ac;
		fVar7 = (ScratchpadAnimManager_004494ec.pScreenSpaceMatrix)->ad;
		fVar8 = (ScratchpadAnimManager_004494ec.pScreenSpaceMatrix)->ba;
		fVar9 = (ScratchpadAnimManager_004494ec.pScreenSpaceMatrix)->bb;
		fVar10 = (ScratchpadAnimManager_004494ec.pScreenSpaceMatrix)->bc;
		fVar11 = (ScratchpadAnimManager_004494ec.pScreenSpaceMatrix)->bd;
		fVar12 = (ScratchpadAnimManager_004494ec.pScreenSpaceMatrix)->ca;
		fVar13 = (ScratchpadAnimManager_004494ec.pScreenSpaceMatrix)->cb;
		fVar14 = (ScratchpadAnimManager_004494ec.pScreenSpaceMatrix)->cc;
		fVar15 = (ScratchpadAnimManager_004494ec.pScreenSpaceMatrix)->cd;
		fVar16 = (ScratchpadAnimManager_004494ec.pScreenSpaceMatrix)->da;
		fVar17 = (ScratchpadAnimManager_004494ec.pScreenSpaceMatrix)->db;
		fVar18 = (ScratchpadAnimManager_004494ec.pScreenSpaceMatrix)->dc;
		fVar19 = (ScratchpadAnimManager_004494ec.pScreenSpaceMatrix)->dd;
		g_RenderMData[2].aa =
			fVar4 * g_RenderMData[3].aa + fVar8 * g_RenderMData[3].ab + fVar12 * g_RenderMData[3].ac +
			fVar16 * g_RenderMData[3].ad;
		g_RenderMData[2].ab =
			fVar5 * g_RenderMData[3].aa + fVar9 * g_RenderMData[3].ab + fVar13 * g_RenderMData[3].ac +
			fVar17 * g_RenderMData[3].ad;
		g_RenderMData[2].ac =
			fVar6 * g_RenderMData[3].aa + fVar10 * g_RenderMData[3].ab + fVar14 * g_RenderMData[3].ac +
			fVar18 * g_RenderMData[3].ad;
		g_RenderMData[2].ad =
			fVar7 * g_RenderMData[3].aa + fVar11 * g_RenderMData[3].ab + fVar15 * g_RenderMData[3].ac +
			fVar19 * g_RenderMData[3].ad;
		g_RenderMData[2].ba =
			fVar4 * g_RenderMData[3].ba + fVar8 * g_RenderMData[3].bb + fVar12 * g_RenderMData[3].bc +
			fVar16 * g_RenderMData[3].bd;
		g_RenderMData[2].bb =
			fVar5 * g_RenderMData[3].ba + fVar9 * g_RenderMData[3].bb + fVar13 * g_RenderMData[3].bc +
			fVar17 * g_RenderMData[3].bd;
		g_RenderMData[2].bc =
			fVar6 * g_RenderMData[3].ba + fVar10 * g_RenderMData[3].bb + fVar14 * g_RenderMData[3].bc +
			fVar18 * g_RenderMData[3].bd;
		g_RenderMData[2].bd =
			fVar7 * g_RenderMData[3].ba + fVar11 * g_RenderMData[3].bb + fVar15 * g_RenderMData[3].bc +
			fVar19 * g_RenderMData[3].bd;
		g_RenderMData[2].ca =
			fVar4 * g_RenderMData[3].ca + fVar8 * g_RenderMData[3].cb + fVar12 * g_RenderMData[3].cc +
			fVar16 * g_RenderMData[3].cd;
		g_RenderMData[2].cb =
			fVar5 * g_RenderMData[3].ca + fVar9 * g_RenderMData[3].cb + fVar13 * g_RenderMData[3].cc +
			fVar17 * g_RenderMData[3].cd;
		g_RenderMData[2].cc =
			fVar6 * g_RenderMData[3].ca + fVar10 * g_RenderMData[3].cb + fVar14 * g_RenderMData[3].cc +
			fVar18 * g_RenderMData[3].cd;
		g_RenderMData[2].cd =
			fVar7 * g_RenderMData[3].ca + fVar11 * g_RenderMData[3].cb + fVar15 * g_RenderMData[3].cc +
			fVar19 * g_RenderMData[3].cd;
		g_RenderMData[2].da =
			fVar4 * g_RenderMData[3].da + fVar8 * g_RenderMData[3].db + fVar12 * g_RenderMData[3].dc +
			fVar16 * g_RenderMData[3].dd;
		g_RenderMData[2].db =
			fVar5 * g_RenderMData[3].da + fVar9 * g_RenderMData[3].db + fVar13 * g_RenderMData[3].dc +
			fVar17 * g_RenderMData[3].dd;
		g_RenderMData[2].dc =
			fVar6 * g_RenderMData[3].da + fVar10 * g_RenderMData[3].db + fVar14 * g_RenderMData[3].dc +
			fVar18 * g_RenderMData[3].dd;
		g_RenderMData[2].dd =
			fVar7 * g_RenderMData[3].da + fVar11 * g_RenderMData[3].db + fVar15 * g_RenderMData[3].dc +
			fVar19 * g_RenderMData[3].dd;
		sceVu0MatrixFunc_00267300((TO_SCE_MTX)Matrix_70000a00_PTR, (TO_SCE_MTX)m1);
		sceVu0InverseMatrix(Matrix_70000a00_PTR, Matrix_70000a00_PTR);
		sceVu0MulMatrix((TO_SCE_MTX)Vector_ARRAY_70000920, (TO_SCE_MTX)PTR_Matrix_70000a40,
			(TO_SCE_MTX)Matrix_70000a00_PTR);
		sceVu0TransposeMatrix((TO_SCE_MTX)Vector_ARRAY_70000920, (TO_SCE_MTX)Vector_ARRAY_70000920);
		sceVu0CopyMatrix((TO_SCE_MTX)(Vector_ARRAY_70000920 + 3), (TO_SCE_MTX)PTR_Matrix_70000a50);
		Vector_ARRAY_70000990[1].z = PTR_Vector_70000a60->z;
		Vector_ARRAY_70000990[1].x = PTR_Vector_70000a60->x;
		Vector_ARRAY_70000990[1].y = PTR_Vector_70000a60->y;
		Vector_ARRAY_70000990[1].w = 0.0078125f;
		if (((pRenderData->pMeshTransformData == (MeshTransformDataBase*)0x0) ||
			(pLVar2 = pRenderData->pMeshTransformData->pLightingMatrixFuncObj_0xa0, pLVar2 == (LightingMatrixFuncObj*)0x0))
			|| (pfVar3 = pLVar2->field_0x10, pfVar3 == (float*)0x0)) {
			(ScratchpadAnimManager_004494ec.pVector_0x18)->z = FLOAT_00448a04;
		}
		else {
			(ScratchpadAnimManager_004494ec.pVector_0x18)->z = *pfVar3;
		}
		Vector_ARRAY_70000990[2] = *ScratchpadAnimManager_004494ec.pVector_0x18;
		shellDmaSync(SHELLDMA_CHANNEL_VIF0);
		FUN_00258050(Vector_ARRAY_70000800, 0x1c0, (uint)((ulong)((long)(int)pMatrixBuffer << 0x24) >> 0x24));
		return pMatrixBuffer + 7;
	}

	RenderCommand* WillCallLightingFunc_002a8ef0(RenderCommand* param_1, RenderFrame_30* param_2)
	{
		ushort uVar1;
		MeshTransformSpecial* pMVar2;
		MeshTransformDataBase* pMVar3;
		float fVar4;
		bool bVar5;
		RenderCommand* pRVar6;
		int iVar7;
		Matrix* pMVar8;
		RenderCommand* pRVar9;
		Matrix* pMVar10;
		byte bVar11;

		pMVar2 = param_2->field_0x4;
		while (true) {
			uVar1 = pMVar2->specUnion.field_0x0[0];
			bVar11 = 0;
			if (uVar1 == 4) {
				if (((pMVar2->pRenderInput[1].subCommandBufferCount & 0x280) == 0) &&
					(param_2->pMeshTransformData != &MeshTransformDataBase_0048cad0)) {
					bVar11 = 1;
				}
			}
			else {
				if (uVar1 == 6) {
					bVar11 = 1;
				}
			}
			if (g_CurrentDrawType_0044923c == 0xe) {
				IMPLEMENTATION_GUARD(param_1 = FUN_002a9220(param_2->pTransformMatrix, param_1);)
			}
			pMVar10 = g_GraphicsMatrixBuffer_004493e0;
			pMVar3 = param_2->pMeshTransformData;
			if (pMVar3 != (MeshTransformDataBase*)0x0) break;
			pMVar10 = param_2->pTransformMatrix;
			bVar5 = false;
			if ((((pMVar10->ac + pMVar10->aa + pMVar10->ab == 1.0) &&
				(bVar5 = false, pMVar10->bc + pMVar10->ba + pMVar10->bb == 1.0)) &&
				(bVar5 = false, pMVar10->cc + pMVar10->ca + pMVar10->cb == 1.0)) &&
				(bVar5 = true, pMVar10->dd + pMVar10->dc + pMVar10->da + pMVar10->db != 1.0)) {
				bVar5 = false;
			}
			if (!bVar5) {
				IMPLEMENTATION_GUARD(
				param_1->cmdA = 0xe1000001c;
				param_1->cmdB = 0;
				*(undefined4*)&param_1->cmdB = 0;
				*(undefined4*)((ulong)&param_1->cmdB + 4) = 0x6c1c0006;
				pRVar6 = (RenderCommand*)LightingMatrixFunc_002a8ab0((int)(param_1 + 1), (RenderFrame_30*)param_2, bVar11);
				*(uint*)ScratchpadAnimManager_004494ec.field_0x1c =
					(uint)((ulong)((long)(int)(param_1 + 1) << 0x24) >> 0x24) + 0xa0;
				if (bVar11 != 0) {
					pRVar9 = pRVar6 + 1;
					pRVar6->cmdA = 0xe10000004;
					pRVar6->cmdB = 0;
					*(undefined4*)&pRVar6->cmdB = 0;
					iVar7 = 8;
					*(undefined4*)((ulong)&pRVar6->cmdB + 4) = 0x6c0403fb;
					pMVar10 = param_2->pTransformMatrix;
					do {
						iVar7 = iVar7 + -1;
						fVar4 = pMVar10->ab;
						*(float*)&pRVar9->cmdA = pMVar10->aa;
						pMVar10 = (Matrix*)&pMVar10->ac;
						*(float*)((ulong)&pRVar9->cmdA + 4) = fVar4;
						pRVar9 = (RenderCommand*)&pRVar9->cmdB;
					} while (0 < iVar7);
					pRVar6 = pRVar6 + 5;
				}
				return pRVar6;)
			}
			param_2->pMeshTransformData = &MeshTransformDataBase_0048cad0;
		}
		if (pMVar3->field_0xa4 == (Matrix*)0x0) {
			pMVar3->field_0xa4 = g_GraphicsMatrixBuffer_004493e0;
			g_GraphicsMatrixBuffer_004493e0 = LightingMatrixFunc_002a8ab0(pMVar10, param_2, bVar11);
			if (bVar11 != 0) {
				pMVar10 = param_2->pTransformMatrix;
				iVar7 = 8;
				pMVar8 = g_GraphicsMatrixBuffer_004493e0;
				do {
					iVar7 = iVar7 + -1;
					fVar4 = pMVar10->ab;
					pMVar8->aa = pMVar10->aa;
					pMVar10 = (Matrix*)&pMVar10->ac;
					pMVar8->ab = fVar4;
					pMVar8 = (Matrix*)&pMVar8->ac;
				} while (0 < iVar7);
				g_GraphicsMatrixBuffer_004493e0 = g_GraphicsMatrixBuffer_004493e0 + 1;
			}
		}
		*(uint*)ScratchpadAnimManager_004494ec.field_0x1c =
			(uint)((ulong)((ulong)(int)param_2->pMeshTransformData->field_0xa4 << 0x24) >> 0x24) + 0xa0;
		param_1->cmdA = ((ulong)(int)param_2->pMeshTransformData->field_0xa4 & 0xfffffffU) << 0x20 | 0x3000001c;
		param_1->cmdB = 0x6c1c000600000000;
		if (bVar11 == 0) {
			return param_1 + 1;
		}
		param_1[1].cmdA = ((ulong)(int)(param_2->pMeshTransformData->field_0xa4 + 7) & 0xfffffffU) << 0x20 | 0x30000004;
		param_1[1].cmdB = 0x6c0403fb00000000;
		return param_1 + 2;
	}

	RenderCommand* g_TempCommandBuffer_004493b0 = NULL;
	RenderCommand* g_CommandScratchpadBase_004489a0 = NULL;

	uint GetLengthOrSize_002a6630(RenderInput_40* param_1, int param_2, long bUpdateInternal)
	{
		uint uVar1;
		uint uVar2;
		ulong iVar3;

		if (param_1->field_0x6 == -1) {
			param_1->field_0x6 = 0;
		}
		uVar2 = (uint)(ushort)param_1->field_0x6;
		if ((uVar2 == 0) || (bUpdateInternal != 0)) {
			uVar1 = param_1->flags_0x0;
			iVar3 = (ulong)&param_1->flags_0x0 + param_1->offsetA;
			if ((uVar1 & 4) == 0) {
				if ((uVar1 & 0x8000000) == 0) {
					if ((uVar1 & 0x2000000) == 0) {
						if (param_2 == 0) {
							uVar2 = 10;
							if (*(int*)(iVar3 + ((ushort)param_1->meshSectionCount_0x3a - 1) * 0xa0 + -0x10) != 0x60000000) {
								uVar2 = 9;
							}
						}
						else {
							uVar2 = 7;
						}
					}
					else {
						uVar2 = 8;
					}
				}
				else {
					if ((uVar1 & 0x2000000) == 0) {
						if (param_2 == 0) {
							uVar2 = 0xc;
							if (*(int*)(iVar3 + ((ushort)param_1->meshSectionCount_0x3a - 1) * 0xc0 + -0x10) != 0x60000000) {
								uVar2 = 9;
							}
						}
						else {
							uVar2 = 9;
						}
					}
					else {
						uVar2 = 10;
					}
				}
			}
			else {
				uVar2 = 8;
				if ((uVar1 & 0x2000000) != 0) {
					uVar2 = 6;
				}
			}
			if (bUpdateInternal == 0) {
				param_1->field_0x6 = (short)uVar2;
			}
		}
		return uVar2;
	}

	StaticMeshMaster* g_FrontendStaticMeshMaster_00449558 = NULL;

	RenderCommand*
		MeshRenderFunc_002a7430(RenderCommand* pDisplayList, MeshTransformSpecial* pMeshTransformSpecial, ulong mode)
	{
		bool bVar1;
		ushort uVar2;
		DisplayListInternal* pDVar3;
		undefined8 uVar4;
		float fVar5;
		float fVar6;
		float fVar7;
		AnimScratchpad* pAVar8;
		undefined4 uVar9;
		undefined4 uVar10;
		undefined4 uVar11;
		int iVar12;
		Matrix* pMVar13;
		uint uVar14;
		short* pAnimIndexes;
		RenderCommand* ppuVar15;
		Vector* pVVar15;
		Vector* pVVar16;

		pDVar3 = pMeshTransformSpecial->pRenderInput;
		uVar2 = pMeshTransformSpecial->specUnion.field_0x0[1];
		(ScratchpadAnimManager_004494ec.pAnimScratchpad)->field_0x0 = (int)(short)uVar2;
		(ScratchpadAnimManager_004494ec.pAnimScratchpad)->flags = 0;
		(ScratchpadAnimManager_004494ec.pAnimScratchpad)->field_0x8 = 0;
		(ScratchpadAnimManager_004494ec.pAnimScratchpad)->field_0xc = 0;
		if ((g_CurrentDrawType_0044923c == 0xe) && (FLOAT_00448a04 != 0.0)) {
			(ScratchpadAnimManager_004494ec.pAnimScratchpad)->flags =
				(ScratchpadAnimManager_004494ec.pAnimScratchpad)->flags | 1;
		}
		pDisplayList->cmdA = 0xe10000001;
		pDisplayList->cmdB = 0x7c01007600000000;
		pAVar8 = ScratchpadAnimManager_004494ec.pAnimScratchpad;
		uVar4 = *(undefined8*)ScratchpadAnimManager_004494ec.pAnimScratchpad;
		uVar10 = (ScratchpadAnimManager_004494ec.pAnimScratchpad)->field_0x8;
		uVar11 = (ScratchpadAnimManager_004494ec.pAnimScratchpad)->field_0xc;
		*(int*)&pDisplayList[1].cmdA = (int)uVar4;
		*(int*)((ulong)&pDisplayList[1].cmdA + 4) = (int)((ulong)uVar4 >> 0x20);
		*(undefined4*)&pDisplayList[1].cmdB = uVar10;
		*(undefined4*)((ulong)&pDisplayList[1].cmdB + 4) = uVar11;
		pDisplayList[2].cmdA = 0xe10000001;
		pDisplayList[2].cmdB = 0x7c01019800000000;
		uVar4 = *(undefined8*)pAVar8;
		uVar10 = pAVar8->field_0x8;
		uVar11 = pAVar8->field_0xc;
		*(int*)&pDisplayList[3].cmdA = (int)uVar4;
		*(int*)((ulong)&pDisplayList[3].cmdA + 4) = (int)((ulong)uVar4 >> 0x20);
		*(undefined4*)&pDisplayList[3].cmdB = uVar10;
		*(undefined4*)((ulong)&pDisplayList[3].cmdB + 4) = uVar11;
		pDisplayList[4].cmdA = 0xe10000001;
		pDisplayList[4].cmdB = 0x7c0102ba00000000;
		uVar4 = *(undefined8*)pAVar8;
		uVar10 = pAVar8->field_0x8;
		uVar11 = pAVar8->field_0xc;
		*(int*)&pDisplayList[5].cmdA = (int)uVar4;
		*(int*)((ulong)&pDisplayList[5].cmdA + 4) = (int)((ulong)uVar4 >> 0x20);
		*(undefined4*)&pDisplayList[5].cmdB = uVar10;
		*(undefined4*)((ulong)&pDisplayList[5].cmdB + 4) = uVar11;
		ppuVar15 = pDisplayList + 6;
		if ((uVar2 & 2) != 0) {
			if ((*(uint*)pDVar3 & 0x2000000) == 0) {
				ppuVar15->cmdA = 0x48a91030000001;
				pDisplayList[6].cmdB = 0x7c01000100000000;
				ppuVar15 = pDisplayList + 7;
			}
			else {
				ppuVar15->cmdA = 0x48a92030000001;
				pDisplayList[6].cmdB = 0x7c01000100000000;
				ppuVar15 = pDisplayList + 7;
			}
		}
		if (((*(uint*)pDVar3 & 0x10000) != 0) && (g_RenderMeshTransform_004493f4 != (MeshTransformDataBase*)0x0)) {
			pAnimIndexes = (short*)((ulong)&pDVar3[-1].flags_0x0 + pDVar3->field_0x8);
			if ((*(uint*)pDVar3 & 0x8000000) == 0) {
				uVar14 = 0x394;
				iVar12 = 0x18;
			}
			else {
				uVar14 = 0x3dc;
				if (mode < 2) {
					uVar14 = 0x3dc;
					iVar12 = 9;
				}
				else {
					iVar12 = 7;
				}
			}
			if ((pDVar3[1].flags_0x0 == 0) ||
				(pMVar13 = g_RenderMeshTransform_004493f4->pShadowAnimMatrix, pMVar13 == (Matrix*)0x0)) {
				pMVar13 = g_RenderMeshTransform_004493f4->pAnimMatrix;
			}
			ppuVar15->cmdA = 0x30000000;
			ppuVar15->cmdB = 0x1100000011000000;
			uVar9 = g_StaticVideoList_00489590[2831];
			uVar11 = g_StaticVideoList_00489590[2830];
			uVar10 = g_StaticVideoList_00489590[2829];
			*(undefined4*)&ppuVar15[1].cmdA = g_StaticVideoList_00489590[2828];
			*(undefined4*)((ulong)&ppuVar15[1].cmdA + 4) = uVar10;
			*(undefined4*)&ppuVar15[1].cmdB = uVar11;
			*(undefined4*)((ulong)&ppuVar15[1].cmdB + 4) = uVar9;
			pVVar15 = (Vector*)(ppuVar15 + 2);
			ppuVar15 = (RenderCommand*)pVVar15;
			if (g_TempCommandBuffer_004493b0 != (RenderCommand*)0x0) {
				IMPLEMENTATION_GUARD(
				pVVar16 = (Vector*)((int)pVVar15 - (int)g_CommandScratchpadBase_004489a0);
				shellDmaSync(SHELLDMA_CHANNEL_VIF0);
				FUN_00258050((Vector*)g_CommandScratchpadBase_004489a0, (uint)pVVar16,
					(uint)((ulong)((long)(int)g_TempCommandBuffer_004493b0 << 0x24) >> 0x24));
				g_TempCommandBuffer_004493b0 =
					(RenderCommand*)((ulong)&g_TempCommandBuffer_004493b0->cmdA + ((uint)pVVar16 & 0xfffffff0));
				ppuVar15 = (RenderCommand*)&Vector_70000800;
				g_CommandScratchpadBase_004489a0 = (RenderCommand*)pVVar15;)
			}
			for (; (bVar1 = iVar12 != 0, iVar12 = iVar12 + -1, bVar1 && (-1 < *pAnimIndexes)); pAnimIndexes = pAnimIndexes + 1)
			{
				IMPLEMENTATION_GUARD(
				*(ulong*)&((Vector*)ppuVar15)->x = 0xe10000004;
				*(ulong*)&((Vector*)ppuVar15)->z = (ulong)(uVar14 | 0x6c040000) << 0x20;
				sceVu0CopyMatrix((Matrix*)((Vector*)ppuVar15 + 1), pMVar13 + *pAnimIndexes);
				ppuVar15 = (RenderCommand*)((Vector*)ppuVar15 + 5);
				uVar14 = uVar14 + 4;
				)
			}
			if (g_TempCommandBuffer_004493b0 != (RenderCommand*)0x0) {
				IMPLEMENTATION_GUARD(
				shellDmaSync(SHELLDMA_CHANNEL_VIF0);
				pVVar15 = (Vector*)ppuVar15 + -0x7000080;
				FUN_00258050(&Vector_70000800, (uint)pVVar15,
					(uint)((ulong)((long)(int)g_TempCommandBuffer_004493b0 << 0x24) >> 0x24));
				shellDmaSync(SHELLDMA_CHANNEL_VIF0);
				ppuVar15 = RenderCommand_ARRAY_70001000;
				g_CommandScratchpadBase_004489a0 = RenderCommand_ARRAY_70001000;
				g_TempCommandBuffer_004493b0 =
					(RenderCommand*)((ulong)&g_TempCommandBuffer_004493b0->cmdA + ((uint)pVVar15 & 0xfffffff0));)
			}
		}
		if (((g_RenderMeshTransform_004493f4 == (MeshTransformDataBase*)0x0) ||
			(g_RenderMeshTransform_004493f4 == (MeshTransformDataBase*)0x0000ffff)) ||
			(g_RenderMeshTransform_004493f4 == &MeshTransformDataBase_0048cad0)) {
			*(uint*)pDVar3 = *(uint*)pDVar3 & 0xfffffff7;
		}
		else {
			if ((BYTE_004493b4 == 0) && ((g_RenderMeshTransform_004493f4->flags_0x9e & 2) == 0)) {
				*(uint*)pDVar3 = *(uint*)pDVar3 & 0xfffffff7;
			}
			else {
				*(uint*)pDVar3 = *(uint*)pDVar3 | 8;
				*(ulong*)&((Vector*)ppuVar15)->x = 0x30000000;
				*(ulong*)&((Vector*)ppuVar15)->z = 0x1100000011000000;
				if ((BYTE_004493b8 == 0) && ((g_FrontendStaticMeshMaster_00449558->flags_0x4 & 0x200) == 0)) {
					INT_004489a4 = 0xfffe;
					INT_004489a8 = 0xfffe;
				}
				else {
					INT_004489a4 = 0x7ffe;
					INT_004489a8 = 0x7ffe;
				}
				Vector_00424ff0.w = (float)INT_004489a4;
				Vector_00424ff0.y = (float)INT_004489a8;
				*(ulong*)&((Vector*)ppuVar15)[1].x = 0xe10000001;
				*(ulong*)&((Vector*)ppuVar15)[1].z = 0x7c01003d00000000;
				fVar7 = Vector_00424ff0.w;
				fVar6 = Vector_00424ff0.z;
				fVar5 = Vector_00424ff0.y;
				((Vector*)ppuVar15)[2].x = Vector_00424ff0.x;
				*(float*)((ulong)&((Vector*)ppuVar15)[2].x + 4) = fVar5;
				((Vector*)ppuVar15)[2].z = fVar6;
				*(float*)((ulong)&((Vector*)ppuVar15)[2].z + 4) = fVar7;
				ppuVar15 = (RenderCommand*)((Vector*)ppuVar15 + 3);
			}
		}
		if (((*(uint*)pDVar3 & 0x100) != 0) || (g_CurrentDrawType_0044923c == 0xe)) {
			*(ulong*)&((Vector*)ppuVar15)->x = 0xe10000001;
			*(ulong*)&((Vector*)ppuVar15)->z = 0;
			((Vector*)ppuVar15)->z = 0.0;
			*(undefined4*)((ulong)&((Vector*)ppuVar15)->z + 4) = 0x6c010021;
			((Vector*)ppuVar15)[1].x = (ScratchpadAnimManager_004494ec.pVector_0x18)->x;
			*(float*)((ulong)&((Vector*)ppuVar15)[1].x + 4) = (ScratchpadAnimManager_004494ec.pVector_0x18)->y;
			((Vector*)ppuVar15)[1].z = (ScratchpadAnimManager_004494ec.pVector_0x18)->z;
			*(float*)((ulong)&((Vector*)ppuVar15)[1].z + 4) = (ScratchpadAnimManager_004494ec.pVector_0x18)->w;
			ppuVar15 = (RenderCommand*)((Vector*)ppuVar15 + 2);
		}
		return ppuVar15;
	}

	void CallMeshRenderFunc_002a5630(MeshTransformSpecial* pMeshTransformSpecial)
	{
		bool bVar1;
		undefined8 uVar2;
		float fVar3;
		char* pcVar4;
		uint uVar5;
		RenderCommand* ppuVar6;
		int iVar6;
		uint uVar7;
		uint uVar8;
		byte bVar9;
		ulong uVar10;
		float fVar11;
		uint uVar12;
		int iVar13;
		int* piVar14;
		int iVar15;
		undefined4 uVar16;
		undefined4 uVar17;
		RenderCommand* pRVar18;
		RenderCommand* ppuVar19;
		RenderCommand* ppuVar20;
		MeshTransformSpecial* pMVar19;
		uint uVar20;
		int* piVar21;
		int* piVar22;
		uint uVar23;
		uint uVar24;
		uint pRVar25;
		uint uVar26;
		float fVar27;
		int* local_30;
		uint local_20;
		RenderInput_40* pRenderInput;

		pRVar18 = g_VideoMemoryPtr_00449280;
		pRenderInput = (RenderInput_40*)pMeshTransformSpecial->pRenderInput;
		pMVar19 = (MeshTransformSpecial*)LOAD_SECTION((int)pMeshTransformSpecial->pPrev_0x8);
		uVar20 = (ulong)&pRenderInput->flags_0x0 + pRenderInput->offsetA;
		uVar5 = GetLengthOrSize_002a6630(pRenderInput, 0, 0);
		fVar11 = (float)(uint)BYTE_004489b8;
		if (fVar11 != 1.793662e-43) {
			pMeshTransformSpecial->specUnion.field_0x0[1] =
				pMeshTransformSpecial->specUnion.field_0x0[1] | 0x20;
			Vector_00424ff0.x = fVar11;
			pRVar18->cmdA = 0xe10000001;
			pRVar18->cmdB = 0x7c01003d00000000;
			fVar3 = Vector_00424ff0.w;
			fVar27 = Vector_00424ff0.z;
			fVar11 = Vector_00424ff0.y;
			*(float*)&pRVar18[1].cmdA = Vector_00424ff0.x;
			*(float*)((ulong)&pRVar18[1].cmdA + 4) = fVar11;
			*(float*)&pRVar18[1].cmdB = fVar27;
			*(float*)((ulong)&pRVar18[1].cmdB + 4) = fVar3;
			pRVar18 = pRVar18 + 2;
		}
		if (pMVar19 == (MeshTransformSpecial*)0x0) {
			IMPLEMENTATION_GUARD(
			uVar8 = (uint)(ushort)pRenderInput->meshSectionCount_0x3a % 3;
			iVar6 = (ushort)pRenderInput->meshSectionCount_0x3a - uVar8;
			ppuVar20 = MeshRenderFunc_002a7430(pRVar18, pMeshTransformSpecial, 1);
			if ((RenderCommand*)0x70003000 < ppuVar20 + (uint)(ushort)pRenderInput->meshSectionCount_0x3a * 2) {
				IMPLEMENTATION_GUARD(
				pRVar18 = (RenderCommand*)((int)ppuVar20 - (int)g_CommandScratchpadBase_004489a0);
				FUN_002580b0(g_CommandScratchpadBase_004489a0, (uint)pRVar18,
					(uint)((ulong)((long)(int)g_TempCommandBuffer_004493b0 << 0x24) >> 0x24));
				g_CommandScratchpadBase_004489a0 = (RenderCommand*)&PTR_DAT_70001000;
				g_TempCommandBuffer_004493b0 =
					(RenderCommand*)((ulong)&g_TempCommandBuffer_004493b0->cmdA + ((uint)pRVar18 & 0xfffffff0));
				ppuVar20 = (RenderCommand*)&PTR_DAT_70001000;)
			}
			pcVar4 = ScratchpadAnimManager_004494ec.field_0x34;
			if ((pRenderInput->flags_0x0 & 4) == 0) {
				IMPLEMENTATION_GUARD(
				ppuVar6 = ppuVar20;
				while (bVar1 = uVar8 != 0, uVar8 = uVar8 - 1, bVar1) {
					ppuVar6->cmdA = 0x30000000;
					uVar23 = UINT_004492d8;
					uVar7 = uVar20 & 0xfffffff;
					uVar20 = uVar20 + uVar5 * 0x10;
					iVar13 = UINT_004492d8 * 4;
					ppuVar6->cmdB = (long)(int)(*(int*)(pcVar4 + iVar13) + 1U | 0x3000000) & 0xffffffffU | 0x200000000000000;
					ppuVar6[1].cmdA = (ulong)uVar7 << 0x20 | 0x50000000;
					ppuVar6[1].cmdB = (long)(int)(*(int*)(pcVar4 + iVar13) + 1U | 0x4000000) << 0x20;
					ppuVar6 = ppuVar6 + 2;
					if (uVar23 == 2) {
						UINT_004492d8 = 0;
					}
					else {
						UINT_004492d8 = UINT_004492d8 + 1;
					}
				}
				piVar14 = (int*)(ScratchpadAnimManager_004494ec.field_0x38 + UINT_004492d8 * 0xc);
				for (; iVar6 != 0; iVar6 = iVar6 + -3) {
					ppuVar6->cmdA = 0x30000000;
					uVar8 = uVar20 & 0xfffffff;
					iVar13 = uVar20 + uVar5 * 0x10;
					iVar15 = uVar20 + uVar5 * 0x20;
					uVar20 = uVar20 + uVar5 * 0x30;
					ppuVar6->cmdB = (long)(*piVar14 + 1) & 0xffffffffU | 0x200000003000000;
					ppuVar6[2].cmdA = 0x30000000;
					ppuVar6[2].cmdB = (long)(piVar14[1] + 1) & 0xffffffffU | 0x200000003000000;
					ppuVar6[4].cmdA = 0x30000000;
					ppuVar6[4].cmdB = (long)(piVar14[2] + 1) & 0xffffffffU | 0x200000003000000;
					ppuVar6[1].cmdA = (ulong)uVar8 << 0x20 | 0x50000000;
					ppuVar6[1].cmdB = ((long)(*piVar14 + 1) & 0xffffffffU | 0x4000000) << 0x20;
					ppuVar6[3].cmdA = ((long)iVar13 & 0xfffffffU) << 0x20 | 0x50000000;
					ppuVar6[3].cmdB = ((long)(piVar14[1] + 1) & 0xffffffffU | 0x4000000) << 0x20;
					ppuVar6[5].cmdA = ((long)iVar15 & 0xfffffffU) << 0x20 | 0x50000000;
					ppuVar6[5].cmdB = ((long)(piVar14[2] + 1) & 0xffffffffU | 0x4000000) << 0x20;
					ppuVar6 = ppuVar6 + 6;
				})
			}
			else {
				IMPLEMENTATION_GUARD(
				uVar23 = 0x48;
				iVar6 = FUN_002b4d40(pRenderInput);
				uVar8 = FUN_002b4d30(pRenderInput);
				fVar11 = *(float*)(iVar6 + 0x10);
				if (fVar11 < 2.147484e+09) {
					uVar7 = (uint)fVar11;
				}
				else {
					uVar7 = (int)(fVar11 - 2.147484e+09) | 0x80000000;
				}
				if ((int)uVar7 < 0) {
					fVar27 = (float)(uVar7 >> 1 | uVar7 & 1);
					fVar27 = fVar27 + fVar27;
				}
				else {
					fVar27 = (float)uVar7;
				}
				fVar11 = fVar11 - fVar27;
				piVar14 = (int*)pRenderInput->field_0x24;
				iVar13 = *piVar14;
				piVar22 = piVar14 + uVar7 * iVar13 * 4 + 4;
				piVar21 = piVar22;
				if ((*(uint*)(iVar6 + 4) & 8) == 0) {
					uVar26 = uVar8 - 1;
					if (uVar7 < uVar26) {
						piVar21 = piVar14 + (uVar7 + 1) * iVar13 * 4 + 4;
					}
					else {
						if ((uVar7 != uVar26) && (piVar21 = (int*)0x0, uVar7 == uVar8)) {
							fVar11 = 1.0;
							piVar22 = piVar14 + (uVar8 - 2) * iVar13 * 4 + 4;
							piVar21 = piVar14 + uVar26 * iVar13 * 4 + 4;
						}
					}
				}
				else {
					uVar26 = uVar8 - 1;
					if (uVar7 < uVar26) {
						piVar21 = piVar14 + (uVar7 + 1) * iVar13 * 4 + 4;
					}
					else {
						if ((uVar7 != uVar26) && (piVar21 = (int*)0x0, uVar7 == uVar8)) {
							fVar11 = 1.0;
							piVar22 = piVar14 + (uVar8 - 2) * iVar13 * 4 + 4;
							piVar21 = piVar14 + uVar26 * iVar13 * 4 + 4;
						}
					}
				}
				ppuVar20->cmdA = 0x30000000;
				ppuVar20->cmdB = 0x1300000000000000;
				ppuVar20[1].cmdA = 0x30000000;
				ppuVar20[1].cmdB = 0x1100000011000000;
				Vector_00424ff0.z = 256.0 - fVar11 * 256.0;
				ppuVar20[2].cmdA = 0xe10000001;
				ppuVar20[2].cmdB = 0x7c01003d00000000;
				fVar3 = Vector_00424ff0.w;
				fVar27 = Vector_00424ff0.z;
				fVar11 = Vector_00424ff0.y;
				*(float*)&ppuVar20[3].cmdA = Vector_00424ff0.x;
				*(float*)((ulong)&ppuVar20[3].cmdA + 4) = fVar11;
				*(float*)&ppuVar20[3].cmdB = fVar27;
				*(float*)((ulong)&ppuVar20[3].cmdB + 4) = fVar3;
				ppuVar20[4].cmdA = 0x30000000;
				ppuVar20[4].cmdB = 0x1100000011000000;
				pcVar4 = ScratchpadAnimManager_004494ec.field_0x34;
				pRVar18 = ppuVar20 + 5;
				for (uVar8 = 0; uVar8 < (ushort)pRenderInput->meshSectionCount_0x3a; uVar8 = uVar8 + 1) {
					if (uVar8 == (ushort)pRenderInput->meshSectionCount_0x3a - 1) {
						for (uVar23 = (uint)pRenderInput->field_0x38; (uVar23 & 3) != 0; uVar23 = uVar23 + 1) {
						}
					}
					uVar10 = (long)(int)(uVar23 >> 2) | 0x30000000;
					uVar26 = (uint)piVar22 & 0xfffffff;
					pRVar18->cmdA = 0x30000000;
					uVar7 = UINT_004492d8;
					uVar12 = (uint)piVar21 & 0xfffffff;
					uVar24 = uVar20 & 0xfffffff;
					iVar6 = UINT_004492d8 * 4;
					piVar22 = piVar22 + 0x48;
					piVar21 = piVar21 + 0x48;
					uVar20 = uVar20 + uVar5 * 0x10;
					pRVar18->cmdB = (long)(*(int*)(pcVar4 + iVar6) + 1) & 0xffffffffU | 0x200000003000000;
					pRVar18[1].cmdA = 0x4895c030000001;
					pRVar18[1].cmdB = 0;
					pRVar18[2].cmdA = (ulong)uVar26 << 0x20 | uVar10;
					pRVar18[2].cmdB = ((long)(int)(uVar23 << 0x10) & 0xffffffffU | 0x7e00c002) << 0x20;
					pRVar18[3].cmdA = 0x48962030000001;
					pRVar18[3].cmdB = 0;
					pRVar18[4].cmdA = (ulong)uVar12 << 0x20 | uVar10;
					pRVar18[4].cmdB = ((long)(int)(uVar23 << 0x10) & 0xffffffffU | 0x7e00c0d9) << 0x20;
					pRVar18[5].cmdA = (ulong)uVar24 << 0x20 | 0x50000000;
					pRVar18[5].cmdB = ((long)(*(int*)(pcVar4 + iVar6) + 1) & 0xffffffffU | 0x4000000) << 0x20;
					pRVar18 = pRVar18 + 6;
					if (uVar7 == 2) {
						UINT_004492d8 = 0;
					}
					else {
						UINT_004492d8 = UINT_004492d8 + 1;
					}
				}
				pRVar18->cmdA = 0x30000000;
				pRVar18->cmdB = 0x1300000000000000;
				ppuVar6 = pRVar18 + 1;)
			})
		}
		else {
			uVar8 = pRenderInput->flags_0x0;
			pRVar25 = 0x0;
			uVar23 = 0;
			//ppuVar6 = (RenderCommand*)(uint)(ushort)pRenderInput->meshSectionCount_0x3a;
			ppuVar6 = MeshRenderFunc_002a7430(pRVar18, pMeshTransformSpecial, 1);
#ifdef PLATFORM_WIN
			if ((RenderCommand*)(g_ScratchpadAddr_00448a58 + 0xC0) < ppuVar6 + (int)pRenderInput->meshSectionCount_0x3a * 4) {
#else
			if ((RenderCommand*)0x70003000 < ppuVar6 + (int)pRenderInput->meshSectionCount_0x3a * 4) {
#endif
				IMPLEMENTATION_GUARD(
				pRVar18 = (RenderCommand*)((int)ppuVar6 - (int)g_CommandScratchpadBase_004489a0);
				FUN_002580b0(g_CommandScratchpadBase_004489a0, (uint)pRVar18,
					(uint)((ulong)((long)(int)g_TempCommandBuffer_004493b0 << 0x24) >> 0x24));
				g_CommandScratchpadBase_004489a0 = (RenderCommand*)&PTR_DAT_70001000;
				g_TempCommandBuffer_004493b0 =
					(RenderCommand*)((ulong)&g_TempCommandBuffer_004493b0->cmdA + ((uint)pRVar18 & 0xfffffff0));
				ppuVar6 = (RenderCommand*)&PTR_DAT_70001000;)
			}
			if ((pRenderInput->flags_0x0 & 4) == 0) {
				while (pRVar25 < pRenderInput->meshSectionCount_0x3a) {
					bVar9 = pMVar19->specUnion.byteFlags[0] & 3;
					if (bVar9 == 1) {
						uVar8 = uVar8 & 0xfffffffd | 1;
					LAB_002a5dac:
#ifdef PLATFORM_WIN
						if ((RenderCommand*)(g_ScratchpadAddr_00448a58 + 0x100) < ppuVar6 + 0x80) {
#else
						if ((RenderCommand*)0x70003fff < ppuVar6 + 0x80) {
#endif
							pRVar18 = (RenderCommand*)((ulong)ppuVar6 - (ulong)g_CommandScratchpadBase_004489a0);
							FUN_002580b0((uint)g_CommandScratchpadBase_004489a0, (uint)pRVar18,
								(uint)((ulong)((long)(int)g_TempCommandBuffer_004493b0 << 0x24) >> 0x24));
							g_CommandScratchpadBase_004489a0 = (RenderCommand*)(g_ScratchpadAddr_00448a58 + 0x40);
							g_TempCommandBuffer_004493b0 =
								(RenderCommand*)((ulong)&g_TempCommandBuffer_004493b0->cmdA + ((uint)pRVar18 & 0xfffffff0));
							ppuVar6 = (RenderCommand*)(g_ScratchpadAddr_00448a58 + 0x40);
						}
						(ScratchpadAnimManager_004494ec.pAnimScratchpad)->field_0x0 = uVar8;
						uVar7 = UINT_004492d8;
						iVar15 = UINT_004492d8 * 0x10;
						iVar13 = UINT_004492d8 * 4;
						iVar6 = UINT_004492d8 * 2;
						*ppuVar6 = RenderCommand_ARRAY_0048c5f0[UINT_004492d8];
						uVar2 = *(undefined8*)ScratchpadAnimManager_004494ec.pAnimScratchpad;
						uVar16 = (ScratchpadAnimManager_004494ec.pAnimScratchpad)->field_0x8;
						uVar17 = (ScratchpadAnimManager_004494ec.pAnimScratchpad)->field_0xc;
						*(int*)&ppuVar6[1].cmdA = (int)uVar2;
						*(int*)((ulong)&ppuVar6[1].cmdA + 4) = (int)((ulong)uVar2 >> 0x20);
						*(undefined4*)&ppuVar6[1].cmdB = uVar16;
						*(undefined4*)((ulong)&ppuVar6[1].cmdB + 4) = uVar17;
						uVar2 = g_StaticDisplayListData_0048c340_LONG[iVar6];
						uVar16 = g_StaticDisplayListData_0048c340[uVar7 * 4 + 2];
						uVar17 = g_StaticDisplayListData_0048c340[uVar7 * 4 + 3];
						*(int*)&ppuVar6[2].cmdA = (int)uVar2;
						*(int*)((ulong)&ppuVar6[2].cmdA + 4) = (int)((ulong)uVar2 >> 0x20);
						*(undefined4*)&ppuVar6[2].cmdB = uVar16;
						*(undefined4*)((ulong)&ppuVar6[2].cmdB + 4) = uVar17;
						ppuVar6[3].cmdA = (ulong)(uVar20 & 0xfffffff) << 0x20 | 0x50000000;
						ppuVar6[3].cmdB =
							((long)(*(int*)(ScratchpadAnimManager_004494ec.field_0x34 + iVar13) + 1) & 0xffffffffU | 0x4000000) <<
							0x20;
						ppuVar6 = ppuVar6 + 4;
						if (uVar7 == 2) {
							UINT_004492d8 = 0;
						}
						else {
							UINT_004492d8 = UINT_004492d8 + 1;
						}
					}
					else {
						if ((pMVar19->specUnion.byteFlags[0] & 3) != 0) {
							if (bVar9 == 3) {
								uVar8 = uVar8 & 0xfffffffc;
							}
							else {
								if (bVar9 == 2) {
									uVar8 = uVar8 | 3;
								}
							}
							goto LAB_002a5dac;
						}
#ifdef PLATFORM_WIN
						if ((RenderCommand*)(g_ScratchpadAddr_00448a58 + 0x100) < ppuVar6 + 0x80) {
#else
						if ((RenderCommand*)0x70003fff < ppuVar6 + 0x80) {
#endif
							pRVar18 = (RenderCommand*)((ulong)ppuVar6 - (ulong)g_CommandScratchpadBase_004489a0);
							FUN_002580b0((uint)g_CommandScratchpadBase_004489a0, (uint)pRVar18,
								(uint)((ulong)((long)(int)g_TempCommandBuffer_004493b0 << 0x24) >> 0x24));
							g_CommandScratchpadBase_004489a0 = (RenderCommand*)(g_ScratchpadAddr_00448a58 + 0x40);
							g_TempCommandBuffer_004493b0 =
								(RenderCommand*)((ulong)&g_TempCommandBuffer_004493b0->cmdA + ((uint)pRVar18 & 0xfffffff0));
							ppuVar6 = (RenderCommand*)(g_ScratchpadAddr_00448a58 + 0x40);
						}
						ppuVar6->cmdA = 0xe10000001;
						ppuVar6->cmdB = 0x7c01003d00000000;
						fVar3 = Vector_00424ff0.w;
						fVar27 = Vector_00424ff0.z;
						fVar11 = Vector_00424ff0.y;
						*(float*)&ppuVar6[1].cmdA = Vector_00424ff0.x;
						*(float*)((ulong)&ppuVar6[1].cmdA + 4) = fVar11;
						*(float*)&ppuVar6[1].cmdB = fVar27;
						*(float*)((ulong)&ppuVar6[1].cmdB + 4) = fVar3;
						ppuVar6 = ppuVar6 + 2;
					}
					uVar23 = uVar23 + 1;
					uVar20 = uVar20 + uVar5 * 0x10;
					pRVar25 = pRVar25 + 1;
					pMVar19->specUnion.byteFlags[0] = (char)((int)(uint)pMVar19->specUnion.byteFlags[0] >> 2);
					if (3 < uVar23) {
						uVar23 = 0;
						pMVar19 = pMVar19 + 1;
					}
				}
			}
			else {
				IMPLEMENTATION_GUARD(
				local_20 = 0x48;
				iVar6 = FUN_002b4d40(pRenderInput);
				uVar7 = FUN_002b4d30(pRenderInput);
				fVar11 = *(float*)(iVar6 + 0x10);
				if (fVar11 < 2.147484e+09) {
					uVar26 = (uint)fVar11;
				}
				else {
					uVar26 = (int)(fVar11 - 2.147484e+09) | 0x80000000;
				}
				if ((int)uVar26 < 0) {
					fVar27 = (float)(uVar26 >> 1 | uVar26 & 1);
					fVar27 = fVar27 + fVar27;
				}
				else {
					fVar27 = (float)uVar26;
				}
				fVar11 = fVar11 - fVar27;
				piVar14 = (int*)pRenderInput->field_0x24;
				iVar13 = *piVar14;
				local_30 = piVar14 + uVar26 * iVar13 * 4 + 4;
				piVar21 = local_30;
				if ((*(uint*)(iVar6 + 4) & 8) == 0) {
					uVar12 = uVar7 - 1;
					if (uVar26 < uVar12) {
						piVar21 = piVar14 + (uVar26 + 1) * iVar13 * 4 + 4;
					}
					else {
						if ((uVar26 != uVar12) && (piVar21 = (int*)0x0, uVar26 == uVar7)) {
							fVar11 = 1.0;
							local_30 = piVar14 + (uVar7 - 2) * iVar13 * 4 + 4;
							piVar21 = piVar14 + uVar12 * iVar13 * 4 + 4;
						}
					}
				}
				else {
					uVar12 = uVar7 - 1;
					if (uVar26 < uVar12) {
						piVar21 = piVar14 + (uVar26 + 1) * iVar13 * 4 + 4;
					}
					else {
						if ((uVar26 != uVar12) && (piVar21 = (int*)0x0, uVar26 == uVar7)) {
							fVar11 = 1.0;
							local_30 = piVar14 + (uVar7 - 2) * iVar13 * 4 + 4;
							piVar21 = piVar14 + uVar12 * iVar13 * 4 + 4;
						}
					}
				}
				ppuVar6->cmdA = 0x30000000;
				ppuVar6->cmdB = 0x1300000000000000;
				ppuVar6[1].cmdA = 0x30000000;
				ppuVar6[1].cmdB = 0x1100000011000000;
				Vector_00424ff0.z = 256.0 - fVar11 * 256.0;
				ppuVar6[2].cmdA = 0xe10000001;
				ppuVar6[2].cmdB = 0x7c01003d00000000;
				fVar3 = Vector_00424ff0.w;
				fVar27 = Vector_00424ff0.z;
				fVar11 = Vector_00424ff0.y;
				*(float*)&ppuVar6[3].cmdA = Vector_00424ff0.x;
				*(float*)((ulong)&ppuVar6[3].cmdA + 4) = fVar11;
				*(float*)&ppuVar6[3].cmdB = fVar27;
				*(float*)((ulong)&ppuVar6[3].cmdB + 4) = fVar3;
				ppuVar6[4].cmdA = 0x30000000;
				ppuVar6[4].cmdB = 0x1100000011000000;
				ppuVar19 = ppuVar6 + 5;
				while (pRVar25 < pRenderInput->meshSectionCount_0x3a) {
					bVar9 = *(byte*)&pMVar19->specUnion & 3;
					if (bVar9 == 1) {
						uVar8 = uVar8 & 0xfffffffd | 1;
					LAB_002a5a74:
						if ((RenderCommand*)0x70003fff < ppuVar19 + 4) {
							pRVar18 = (RenderCommand*)((int)ppuVar19 - (int)g_CommandScratchpadBase_004489a0);
							FUN_002580b0(g_CommandScratchpadBase_004489a0, (uint)pRVar18,
								(uint)((ulong)((long)(int)g_TempCommandBuffer_004493b0 << 0x24) >> 0x24));
							g_CommandScratchpadBase_004489a0 = (RenderCommand*)&PTR_DAT_70001000;
							g_TempCommandBuffer_004493b0 =
								(RenderCommand*)((ulong)&g_TempCommandBuffer_004493b0->cmdA + ((uint)pRVar18 & 0xfffffff0));
							ppuVar19 = (RenderCommand*)&PTR_DAT_70001000;
						}
						if (pRVar25 == (RenderCommand*)((ushort)pRenderInput->meshSectionCount_0x3a - 1)) {
							for (local_20 = (uint)pRenderInput->field_0x38; (local_20 & 3) != 0; local_20 = local_20 + 1) {
							}
						}
						uVar10 = (long)(int)(local_20 >> 2) | 0x30000000;
						(ScratchpadAnimManager_004494ec.pAnimScratchpad)->field_0x0 = uVar8;
						uVar7 = UINT_004492d8;
						iVar15 = UINT_004492d8 * 0x10;
						iVar6 = UINT_004492d8 * 2;
						iVar13 = UINT_004492d8 * 4;

						*ppuVar19 = RenderCommand_ARRAY_0048c5f0[UINT_004492d8];

						uVar2 = *(undefined8*)ScratchpadAnimManager_004494ec.pAnimScratchpad;
						uVar16 = (ScratchpadAnimManager_004494ec.pAnimScratchpad)->field_0x8;
						uVar17 = (ScratchpadAnimManager_004494ec.pAnimScratchpad)->field_0xc;
						*(int*)&ppuVar19[1].cmdA = (int)uVar2;
						*(int*)((ulong)&ppuVar19[1].cmdA + 4) = (int)((ulong)uVar2 >> 0x20);
						*(undefined4*)&ppuVar19[1].cmdB = uVar16;
						*(undefined4*)((ulong)&ppuVar19[1].cmdB + 4) = uVar17;
						uVar2 = g_StaticDisplayListData_0048c340_LONG[iVar6];
						uVar16 = g_StaticDisplayListData_0048c340[uVar7 * 4 + 2];
						uVar17 = g_StaticDisplayListData_0048c340[uVar7 * 4 + 3];
						*(int*)&ppuVar19[2].cmdA = (int)uVar2;
						*(int*)((ulong)&ppuVar19[2].cmdA + 4) = (int)((ulong)uVar2 >> 0x20);
						*(undefined4*)&ppuVar19[2].cmdB = uVar16;
						*(undefined4*)((ulong)&ppuVar19[2].cmdB + 4) = uVar17;
						ppuVar19[3].cmdA = 0x30000000;
						piVar14 = (int*)(ScratchpadAnimManager_004494ec.field_0x34 + iVar13);
						ppuVar19[3].cmdB = (long)(*piVar14 + 1) & 0xffffffffU | 0x200000003000000;
						ppuVar19[4].cmdA = 0x4895c030000001;
						ppuVar19[4].cmdB = 0;
						ppuVar19[5].cmdA = ((long)(int)local_30 & 0xfffffffU) << 0x20 | uVar10;
						ppuVar19[5].cmdB = ((long)(int)(local_20 << 0x10) & 0xffffffffU | 0x7e00c002) << 0x20;
						ppuVar19[6].cmdA = 0x48962030000001;
						ppuVar19[6].cmdB = 0;
						ppuVar19[7].cmdA = (ulong)((uint)piVar21 & 0xfffffff) << 0x20 | uVar10;
						ppuVar19[7].cmdB = ((long)(int)(local_20 << 0x10) & 0xffffffffU | 0x7e00c0d9) << 0x20;
						ppuVar19[8].cmdA = (ulong)(uVar20 & 0xfffffff) << 0x20 | 0x50000000;
						ppuVar19[8].cmdB = ((long)(*piVar14 + 1) & 0xffffffffU | 0x4000000) << 0x20;
						ppuVar19 = ppuVar19 + 9;
						if (uVar7 == 2) {
							UINT_004492d8 = 0;
						}
						else {
							UINT_004492d8 = UINT_004492d8 + 1;
						}
					}
					else {
						if ((*(byte*)&pMVar19->specUnion & 3) != 0) {
							if (bVar9 == 3) {
								uVar8 = uVar8 & 0xfffffffc;
							}
							else {
								if (bVar9 == 2) {
									uVar8 = uVar8 | 3;
								}
							}
							goto LAB_002a5a74;
						}
						ppuVar19->cmdA = 0xe10000001;
						ppuVar19->cmdB = 0x7c01003d00000000;
						fVar3 = Vector_00424ff0.w;
						fVar27 = Vector_00424ff0.z;
						fVar11 = Vector_00424ff0.y;
						*(float*)&ppuVar19[1].cmdA = Vector_00424ff0.x;
						*(float*)((ulong)&ppuVar19[1].cmdA + 4) = fVar11;
						*(float*)&ppuVar19[1].cmdB = fVar27;
						*(float*)((ulong)&ppuVar19[1].cmdB + 4) = fVar3;
						ppuVar19 = ppuVar19 + 2;
					}
					uVar23 = uVar23 + 1;
					piVar21 = piVar21 + 0x48;
					pRVar25 = pRVar25 + 1;
					local_30 = local_30 + 0x48;
					uVar20 = uVar20 + uVar5 * 0x10;
					*(char*)&pMVar19->specUnion = (char)((int)(uint) * (byte*)&pMVar19->specUnion >> 2);
					if (3 < uVar23) {
						uVar23 = 0;
						pMVar19 = (MeshTransformSpecial*)((ulong)&pMVar19->specUnion + 1);
					}
				}
				ppuVar19->cmdA = 0x30000000;
				ppuVar19->cmdB = 0x1300000000000000;
				ppuVar6 = ppuVar19 + 1;)
			}
		}
		if (UINT_004492d8 == 2) {
			UINT_ARRAY_004492dc[0] = *(uint*)(ScratchpadAnimManager_004494ec.field_0x34 + 4);
			UINT_ARRAY_004492dc[1] = *(uint*)(ScratchpadAnimManager_004494ec.field_0x34 + 8);
			UINT_ARRAY_004492dc[2] = *(uint*)ScratchpadAnimManager_004494ec.field_0x34;
		}
		else {
			if (UINT_004492d8 == 1) {
				UINT_ARRAY_004492dc[0] = *(uint*)ScratchpadAnimManager_004494ec.field_0x34;
				UINT_ARRAY_004492dc[1] = *(uint*)(ScratchpadAnimManager_004494ec.field_0x34 + 4);
				UINT_ARRAY_004492dc[2] = *(uint*)(ScratchpadAnimManager_004494ec.field_0x34 + 8);
			}
			else {
				if (UINT_004492d8 == 0) {
					UINT_ARRAY_004492dc[0] = *(uint*)(ScratchpadAnimManager_004494ec.field_0x34 + 8);
					UINT_ARRAY_004492dc[1] = *(uint*)ScratchpadAnimManager_004494ec.field_0x34;
					UINT_ARRAY_004492dc[2] = *(uint*)(ScratchpadAnimManager_004494ec.field_0x34 + 4);
				}
			}
		}
		if ((*ScratchpadAnimManager_004494ec.field_0x28 != 0) && (pRenderInput->bUseShadowMatrix_0x30 != 0)) {
			ppuVar6->cmdA = 0x30000000;
			ppuVar6->cmdB = 0;
			*(undefined4*)&ppuVar6->cmdB = 0x13000000;
			ppuVar6 = ppuVar6 + 1;
		}
		g_VideoMemoryPtr_00449280 = ppuVar6;
		return;
	}

	void CallMeshRenderFunc_002a7930(RenderFrame_30* pRenderData, TextureData_MAT_Internal_A* pFileData)
	{
		short sVar1;
		ushort uVar2;
		MeshTransformParentHeader* pMVar3;
		DisplayListInternal* pDVar4;
		bool bVar5;
		RenderCommand* pRVar6;
		MeshTransformSpecial* pRVar5;

		pRVar5 = pRenderData->field_0x4;
		if ((RenderFrame_30*)pRVar5 != pRenderData) {
			*ScratchpadAnimManager_004494ec.field_0x3c = *ScratchpadAnimManager_004494ec.field_0x3c + 1;
		}
		if ((pFileData == &TextureData_MAT_Internal_A_0048c3c0) || (pFileData == &TextureData_MAT_Internal_A_0048c3b0)) {
			IMPLEMENTATION_GUARD(
			g_TempCommandBuffer_004493b0 = (RenderCommand*)0x0;
			if (g_bGeneratingMeshCommands_00449570 == 0) {
				if (pFileData->count_0x0 < 2) {
					bVar5 = false;
					while (pRVar6 = g_VideoMemoryPtr_00449280, (RenderFrame_30*)pRVar5 != pRenderData) {
						sVar1 = *(short*)&((RenderFrame_30*)pRVar5)->pFileData;
						if (sVar1 == 4) {
						LAB_002a7d54:
							pRVar5 = ((RenderFrame_30*)pRVar5)->field_0x4;
						}
						else {
							if ((sVar1 == 5) || (sVar1 == 6)) {
								pMVar3 = ((RenderFrame_30*)pRVar5)->field_0xc;
								*(undefined4*)&g_VideoMemoryPtr_00449280->cmdA = *(undefined4*)pMVar3->name;
								*(MeshTransformSpecial**)((ulong)&g_VideoMemoryPtr_00449280->cmdA + 4) = pMVar3->field_0x4;
								*(int*)&g_VideoMemoryPtr_00449280->cmdB = pMVar3->totalCount;
								*(int*)((ulong)&g_VideoMemoryPtr_00449280->cmdB + 4) = pMVar3->usedCount;
								g_VideoMemoryPtr_00449280 = g_VideoMemoryPtr_00449280 + 1;
								goto LAB_002a7d54;
							}
							if (sVar1 == 3) {
								if ((*(uint*)((RenderFrame_30_B*)pRVar5)->field_0xc->name & 0x10000) == 0) {
									FUN_002ab100(pRVar5, (byte*)pFileData);
								}
								else {
									bVar5 = true;
									CallMeshRenderFunc_002a5630(pRVar5);
								}
								goto LAB_002a7d54;
							}
							pRVar5 = ((RenderFrame_30_B*)pRVar5)->field_0x4;
						}
					}
					if (bVar5) {
						g_VideoMemoryPtr_00449280->cmdA = 0x30000000;
						pRVar6->cmdB = 0x1100000011000000;
						g_VideoMemoryPtr_00449280 = g_VideoMemoryPtr_00449280 + 1;
					}
				}
			}
			else {
				while ((RenderFrame_30_B*)pRVar5 != pRenderData) {
					uVar2 = pRVar5->specUnion.field_0x0[0];
					if ((uVar2 == 5) || (uVar2 == 6)) {
						pDVar4 = pRVar5->pRenderInput;
						*(undefined4*)&g_VideoMemoryPtr_00449280->cmdA = *(undefined4*)pDVar4;
						*(undefined4*)((ulong)&g_VideoMemoryPtr_00449280->cmdA + 4) = *(undefined4*)&pDVar4->subCommandBufferCount;
						*(undefined4*)&g_VideoMemoryPtr_00449280->cmdB = pDVar4->field_0x8;
						*(char**)((ulong)&g_VideoMemoryPtr_00449280->cmdB + 4) = pDVar4->field_0xc;
						g_VideoMemoryPtr_00449280 = g_VideoMemoryPtr_00449280 + 1;
					LAB_002a7c6c:
						pRVar5 = pRVar5->pNext_0x4;
					}
					else {
						if (uVar2 == 4) goto LAB_002a7c6c;
						if (uVar2 == 3) {
							IMPLEMENTATION_GUARD(FUN_002aace0((int)pRVar5, (byte*)pFileData));
							goto LAB_002a7c6c;
						}
						pRVar5 = pRVar5->pNext_0x4;
					}
				}
			})
		}
		else {
#ifdef PLATFORM_WIN
			g_CommandScratchpadBase_004489a0 = (RenderCommand*)(g_ScratchpadAddr_00448a58 + 0x40);
			g_TempCommandBuffer_004493b0 = g_VideoMemoryPtr_00449280;
			g_VideoMemoryPtr_00449280 = (RenderCommand*)(g_ScratchpadAddr_00448a58 + 0x40);
#else
			g_CommandScratchpadBase_004489a0 = (RenderCommand*)0x70001000;
			g_TempCommandBuffer_004493b0 = g_VideoMemoryPtr_00449280;
			g_VideoMemoryPtr_00449280 = (RenderCommand*)0x70001000;
#endif
			if (pFileData->count_0x0 < 2) {
				for (; (RenderFrame_30*)pRVar5 != pRenderData; pRVar5 = ((RenderFrame_30*)pRVar5)->field_0x4) {
					sVar1 = *(short*)&((RenderFrame_30*)pRVar5)->pFileData;
					if ((sVar1 == 5) || (sVar1 == 6)) {
						IMPLEMENTATION_GUARD(
						pMVar3 = ((RenderFrame_30*)pRVar5)->field_0xc;
						*(undefined4*)&g_VideoMemoryPtr_00449280->cmdA = *(undefined4*)pMVar3->name;
						*(MeshTransformSpecial**)((ulong)&g_VideoMemoryPtr_00449280->cmdA + 4) = pMVar3->field_0x4;
						*(int*)&g_VideoMemoryPtr_00449280->cmdB = pMVar3->totalCount;
						*(int*)((ulong)&g_VideoMemoryPtr_00449280->cmdB + 4) = pMVar3->usedCount;
						g_VideoMemoryPtr_00449280 = g_VideoMemoryPtr_00449280 + 1;
						)
					}
					else {
						if (sVar1 == 4) {
							IMPLEMENTATION_GUARD(
							FUN_002a8430(pRVar5, pFileData);)
						}
						else {
							if (sVar1 == 3) {
								CallMeshRenderFunc_002a5630(pRVar5);
							}
						}
					}
#ifdef PLATFORM_WIN
					if ((RenderCommand*)(g_ScratchpadAddr_00448a58 + 0xC0) < g_VideoMemoryPtr_00449280) {
#else
					if ((RenderCommand*)0x70003000 < g_VideoMemoryPtr_00449280) {
#endif
						IMPLEMENTATION_GUARD(
						pRVar6 = (RenderCommand*)((int)g_VideoMemoryPtr_00449280 - (int)g_CommandScratchpadBase_004489a0);
						FUN_002580b0(g_CommandScratchpadBase_004489a0, (uint)pRVar6, (uint)((ulong)((long)(int)g_TempCommandBuffer_004493b0 << 0x24) >> 0x24));
						g_VideoMemoryPtr_00449280 = (RenderCommand*)&PTR_DAT_70001000;
						g_CommandScratchpadBase_004489a0 = &PTR_DAT_70001000;
						g_TempCommandBuffer_004493b0 = (RenderCommand*)((ulong)&g_TempCommandBuffer_004493b0->cmdA + ((uint)pRVar6 & 0xfffffff0));)
					}
				}
			}
			else {
				IMPLEMENTATION_GUARD(
				for (; (RenderFrame_30_B*)pRVar5 != pRenderData; pRVar5 = ((RenderFrame_30_B*)pRVar5)->field_0x4) {
					if (*(short*)&((RenderFrame_30_B*)pRVar5)->pFileData == 4) {
						FUN_002a8430(pRVar5, pFileData);
					}
					else {
						if (*(short*)&((RenderFrame_30_B*)pRVar5)->pFileData == 3) {
							FUN_002a6760((int)pRVar5, (byte*)pFileData);
						}
					}
					if ((RenderCommand*)0x70003000 < g_VideoMemoryPtr_00449280) {
						pRVar6 = (RenderCommand*)((int)g_VideoMemoryPtr_00449280 - (int)g_CommandScratchpadBase_004489a0);
						FUN_002580b0(g_CommandScratchpadBase_004489a0, (uint)pRVar6, (uint)((ulong)((long)(int)g_TempCommandBuffer_004493b0 << 0x24) >> 0x24));
						g_VideoMemoryPtr_00449280 = (RenderCommand*)&PTR_DAT_70001000;
						g_CommandScratchpadBase_004489a0 = &PTR_DAT_70001000;
						g_TempCommandBuffer_004493b0 = (RenderCommand*)((ulong)&g_TempCommandBuffer_004493b0->cmdA + ((uint)pRVar6 & 0xfffffff0));
					}
				})
			}
			pRVar6 = (RenderCommand*)((ulong)g_VideoMemoryPtr_00449280 - (ulong)g_CommandScratchpadBase_004489a0);
			if (pRVar6 == (RenderCommand*)0x0) {
				g_VideoMemoryPtr_00449280 = g_TempCommandBuffer_004493b0;
			}
			else {
				shellDmaSync(SHELLDMA_CHANNEL_VIF0);
				FUN_00258050((Vector*)g_CommandScratchpadBase_004489a0, (uint)pRVar6, (uint)g_TempCommandBuffer_004493b0 & 0xfffffff);
				g_VideoMemoryPtr_00449280 = (RenderCommand*)((ulong)&g_TempCommandBuffer_004493b0->cmdA + ((uint)pRVar6 & 0xfffffff0));
			}
		}
		return;
	}

	TextureData_LAY_Internal* GetLAY_0029e3b0(TextureData_MAT_Internal_A* pMAT, int index)
	{
		TextureData_LAY_Internal* pTVar1;

		pTVar1 = (TextureData_LAY_Internal*)0x0;
		if (pMAT != (TextureData_MAT_Internal_A*)0x0) {
			if (pMAT->count_0x0 == 0) {
				pTVar1 = (TextureData_LAY_Internal*)0x0;
			}
			else {
				pTVar1 = (TextureData_LAY_Internal*)((char*)LOAD_SECTION(((int*)(pMAT + 1))[index]) + 0x10);
			}
		}
		return pTVar1;
	}

	bool CheckLAY_002ab4f0(TextureData_LAY_Internal* pLAY)
	{
		return (pLAY->field_0x0 & 0x7fff8000) != 0;
	}

	Vector Vector_004489bc = { 1.0f, 1.0f, 1.0f, 1.0f };
	Vector Vector_004489cc = { 1.0f, 1.0f, 1.0f, 1.0f };

	void CallMeshRenderFunc_002a8600(RenderFrame_30* pRenderData, TextureData_MAT_Internal_A* pFileData)
	{
		byte bVar1;
		MeshTransformDataBase* pMVar2;
		float fVar3;
		MeshTransformParentHeader* pMVar4;
		RenderCommand* pRVar5;
		uint* puVar6;
		ulong uVar7;
		bool bVar4;
		int iVar8;
		Matrix* pMVar9;
		Matrix* pMVar10;
		uint uVar11;
		Matrix* m0;
		Matrix* pMVar12;
		Matrix local_40;
		RenderCommand* puVar14;

		puVar14 = g_VideoMemoryPtr_00449280;
		pMVar2 = pRenderData->pMeshTransformData;
		/* Previous mesh may use the same base transform, so check if its different here. */
		if ((g_RenderMeshTransform_004493f4 != pMVar2) || (pMVar2 == (MeshTransformDataBase*)0x0)) {
			g_RenderMeshTransform_004493f4 = pMVar2;
			if (pMVar2 == (MeshTransformDataBase*)0x0) {
				g_RenderMeshTransform_004493f4 = (MeshTransformDataBase*)0x0000ffff;
			}
			g_VideoMemoryPtr_00449280->cmdA = 0x48959030000001;
			puVar14->cmdB = 0x1100000011000000;
			pRVar5 = WillCallLightingFunc_002a8ef0(puVar14 + 1, pRenderData);
			pRVar5->cmdA = 0x30000000;
			puVar14 = pRVar5 + 1;
			pRVar5->cmdB = 0x1100000011000000;
		}
		pMVar12 = pRenderData->pTransformMatrix;
		if (((1 < pFileData->count_0x0) || (pMVar12 == &g_IdentityMatrix)) && (uVar11 = 0, g_CurrentDrawType_0044923c != 0xe))
		{
			bVar1 = pFileData->count_0x0;
			for (; uVar11 < bVar1; uVar11 = uVar11 + 1) {
				TextureData_LAY_Internal* pTVar6 = GetLAY_0029e3b0(pFileData, uVar11);
				if (((pTVar6 != (TextureData_LAY_Internal*)0x0) && (bVar4 = CheckLAY_002ab4f0(pTVar6), bVar4 != false)) ||
					(pMVar10 == &g_IdentityMatrix)) {
					IMPLEMENTATION_GUARD(
					m0 = (Matrix*)(puVar14 + 3);
					puVar14->cmdA = 0xe10000006;
					puVar14->cmdB = 0;
					iVar8 = 8;
					*(undefined4*)&puVar14->cmdB = 0;
					*(undefined4*)((ulong)&puVar14->cmdB + 4) = 0x6c0603fa;
					pMVar9 = &local_40;
					pMVar10 = g_ScratchpadAddr_00448a58;
					do {
						iVar8 = iVar8 + -1;
						fVar3 = pMVar10->ab;
						pMVar9->aa = pMVar10->aa;
						pMVar10 = (Matrix*)&pMVar10->ac;
						pMVar9->ab = fVar3;
						pMVar9 = (Matrix*)&pMVar9->ac;
					} while (0 < iVar8);
					if (local_40.ac + local_40.aa + local_40.ab != 0.0) {
						sceVu0Normalize((float*)&local_40, (float*)&local_40);
					}
					if (local_40.bc + local_40.ba + local_40.bb != 0.0) {
						sceVu0Normalize(&local_40.ba, &local_40.ba);
					}
					if (local_40.cc + local_40.ca + local_40.cb != 0.0) {
						sceVu0Normalize(&local_40.ca, &local_40.ca);
					}
					sceVu0TransposeMatrix(&local_40, &local_40);
					iVar8 = 8;
					local_40.aa = local_40.aa * Vector_004489bc.x;
					local_40.ab = local_40.ab * Vector_004489bc.y;
					local_40.ac = local_40.ac * Vector_004489bc.z;
					local_40.ad = local_40.ad * Vector_004489bc.w;
					local_40.ba = local_40.ba * Vector_004489cc.x;
					local_40.bb = local_40.bb * Vector_004489cc.y;
					local_40.bc = local_40.bc * Vector_004489cc.z;
					local_40.bd = local_40.bd * Vector_004489cc.w;
					*(float*)&puVar14[1].cmdA = local_40.aa;
					*(float*)((ulong)&puVar14[1].cmdA + 4) = local_40.ab;
					*(float*)&puVar14[1].cmdB = local_40.ac;
					*(float*)((ulong)&puVar14[1].cmdB + 4) = local_40.ad;
					*(float*)&puVar14[2].cmdA = local_40.ba;
					*(float*)((ulong)&puVar14[2].cmdA + 4) = local_40.bb;
					*(float*)&puVar14[2].cmdB = local_40.bc;
					*(float*)((ulong)&puVar14[2].cmdB + 4) = local_40.bd;
					pMVar9 = m0;
					do {
						iVar8 = iVar8 + -1;
						fVar3 = pMVar12->ab;
						pMVar9->aa = pMVar12->aa;
						pMVar12 = (Matrix*)&pMVar12->ac;
						pMVar9->ab = fVar3;
						pMVar9 = (Matrix*)&pMVar9->ac;
					} while (0 < iVar8);
					*(undefined4*)&puVar14[6].cmdB = 0;
					*(undefined4*)((ulong)&puVar14[6].cmdA + 4) = 0;
					*(undefined4*)&puVar14[6].cmdA = 0;
					*(undefined4*)((ulong)&puVar14[5].cmdB + 4) = 0;
					*(undefined4*)((ulong)&puVar14[4].cmdB + 4) = 0;
					*(undefined4*)((ulong)&puVar14[3].cmdB + 4) = 0;
					*(undefined4*)((ulong)&puVar14[6].cmdB + 4) = 0x3f800000;
					sceVu0InverseMatrix(m0, m0);
					sceVu0TransposeMatrix(m0, m0);
					puVar14 = puVar14 + 7;)
					break;
				}
				bVar1 = pFileData->count_0x0;
			}
		}
		g_VideoMemoryPtr_00449280 = puVar14;
		puVar14 = RenderCommandLayer_002a9410(puVar14, g_TextureData_LAY_Int_00449404);
		g_VideoMemoryPtr_00449280 = puVar14;
		puVar14->cmdA = 0xe10000001;
		puVar14->cmdB = 0;
		*(undefined4*)&puVar14->cmdB = 0;
		*(undefined4*)((ulong)&g_VideoMemoryPtr_00449280->cmdB + 4) = 0x6c010021;
		g_VideoMemoryPtr_00449280 = g_VideoMemoryPtr_00449280 + 1;
		*(float*)&g_VideoMemoryPtr_00449280->cmdA = (ScratchpadAnimManager_004494ec.pVector_0x18)->x;
		*(float*)((ulong)&g_VideoMemoryPtr_00449280->cmdA + 4) = (ScratchpadAnimManager_004494ec.pVector_0x18)->y;
		*(float*)&g_VideoMemoryPtr_00449280->cmdB = (ScratchpadAnimManager_004494ec.pVector_0x18)->z;
		*(float*)((ulong)&g_VideoMemoryPtr_00449280->cmdB + 4) = (ScratchpadAnimManager_004494ec.pVector_0x18)->w;
		g_VideoMemoryPtr_00449280 = g_VideoMemoryPtr_00449280 + 1;
		CallMeshRenderFunc_002a7930(pRenderData, pFileData);
		pMVar4 = (MeshTransformParentHeader*)(pRenderData->internalMeshTransformSpecial).pNext_0x4;
		pRenderData->field_0x4 = pMVar4->field_0x4 + pMVar4->usedCount;
		pMVar4->usedCount = pMVar4->usedCount - (int)(pRenderData->internalMeshTransformSpecial).pRenderInput;
		(pRenderData->internalMeshTransformSpecial).pRenderInput = (DisplayListInternal*)0x0;
		pRenderData->field_0x4 = (MeshTransformSpecial*)pRenderData;
		(pRenderData->internalMeshTransformSpecial).specUnion.randoPtr = pRenderData;
		return;
	}

	void CallMeshRenderFunc_002a9f10(RenderFrame_30* pRenderFrame)
	{
		DisplayListInternal* pDVar1;
		MeshTransformSpecial* pRVar3;

		if ((pRenderFrame->flags_0x28 & 1U) == 0) {
			pRVar3 = (pRenderFrame->internalMeshTransformSpecial).pNext_0x4;
			if (pRVar3 != (MeshTransformSpecial*)0x0) {
				if (g_bGeneratingMeshCommands_00449570 == 0) {
					for (; pRVar3 != &pRenderFrame->internalMeshTransformSpecial; pRVar3 = pRVar3->pNext_0x4) {
						CallMeshRenderFunc_002a8600((RenderFrame_30*)pRVar3->pRenderInput, (TextureData_MAT_Internal_A*)pRenderFrame->pFileData);
					}
					pDVar1 = (pRenderFrame->internalMeshTransformSpecial).pRenderInput;
				}
				else {
					for (; pRVar3 != &pRenderFrame->internalMeshTransformSpecial; pRVar3 = pRVar3->pNext_0x4) {
						IMPLEMENTATION_GUARD(FUN_002a8a30((int)pRVar3->pRenderInput);
						CallMeshRenderFunc_002a8600((RenderFrame_30*)pRVar3->pRenderInput, (TextureData_MAT_Internal_A*)pRenderFrame->pFileData));
					}
					pDVar1 = (pRenderFrame->internalMeshTransformSpecial).pRenderInput;
				}
				(pRenderFrame->internalMeshTransformSpecial).pNext_0x4 =
					(MeshTransformSpecial*)(*(int*)&pDVar1->subCommandBufferCount + (int)pDVar1->field_0xc * 0x10);
				pDVar1->field_0xc = pDVar1->field_0xc + -pRenderFrame->index_0x1c;
				pRenderFrame->index_0x1c = 0;
				(pRenderFrame->internalMeshTransformSpecial).pNext_0x4 = &pRenderFrame->internalMeshTransformSpecial;
				(pRenderFrame->internalMeshTransformSpecial).pPrev_0x8 = &pRenderFrame->internalMeshTransformSpecial;
			}
			INT_00449238 = 0;
		}
		else {
			if (INT_00449238 == 0) {
				CallMeshRenderFunc_002a8600(RenderFrame_30_Header_004494c0.pOtherCurrent, (TextureData_MAT_Internal_A*)pRenderFrame->pFileData);
				INT_00449238 = 1;
			}
			CallMeshRenderFunc_002a7930
			((RenderFrame_30*)&pRenderFrame->internalMeshTransformSpecial, (TextureData_MAT_Internal_A*)pRenderFrame->pFileData);
			pDVar1 = (pRenderFrame->internalMeshTransformSpecial).pRenderInput;
			(pRenderFrame->internalMeshTransformSpecial).pNext_0x4 =
				(MeshTransformSpecial*)(*(int*)&pDVar1->subCommandBufferCount + (int)pDVar1->field_0xc * 0x10);
			pDVar1->field_0xc = pDVar1->field_0xc + -pRenderFrame->index_0x1c;
			pRenderFrame->index_0x1c = 0;
			(pRenderFrame->internalMeshTransformSpecial).pNext_0x4 = &pRenderFrame->internalMeshTransformSpecial;
			(pRenderFrame->internalMeshTransformSpecial).pPrev_0x8 = &pRenderFrame->internalMeshTransformSpecial;
		}
		return;
	}

	void CallMeshRenderFunc_002aa050(RenderFrame_30* pRenderMeshData)
	{
		TextureData_MAT_Internal_A* pTVar2;
		ulong* puVar3;
		RenderCommand* pRVar4;
		RenderCommand* pRVar5;
		RenderCommand* pRVar6;
		RenderCommand* pRVar7;
		TextureData_MAT_Internal_A* pTVar8;
		undefined4 in_zero_lo = 0x0;
		undefined4 in_zero_hi = 0x0;
		undefined4 in_zero_udw = 0x0;
		undefined4 in_register_0000000c = 0x0;
		RenderCommand* pRVar9;
		int iVar10;
		TextureData_MAT_Internal_A* pTVar1;

		pRVar7 = g_RenderCommandBuffer_00449400;
		pRVar6 = g_RenderCommand_004492c0;
		pRVar4 = g_VideoMemoryPtr_00449280;
		*ScratchpadAnimManager_004494ec.field_0x3c = 0;
		pRVar5 = g_VideoMemoryPtr_00449280;
		BYTE_004489b8 = 0x80;
		pTVar2 = (TextureData_MAT_Internal_A*)pRenderMeshData->pFileData;
		pTVar2->pRenderFrame30 = 0;
		pRVar9 = g_RenderCommandBuffer_00449400;
		iVar10 = g_CurrentDrawType_0044923c;
		pTVar8 = pTVar2;
		if (pTVar2 == g_TextureMatInternal_0044933c) {
			pTVar8 = g_TextureMatInternal_0044933c;
			if (INT_00449330 == 0) {
				INT_00449330 = 1;
			}
			else {
				INT_00449330 = INT_00449330 + -1;
			}
		}
		g_TextureMatInternal_0044933c = pTVar8;
		if (((g_CurrentDrawType_0044923c != 0xe) && (pRenderMeshData->field_0x2c.ptrValue == (char*)0x0)) &&
			(g_RenderCommandBuffer_00449400 != (RenderCommand*)0x0)) {
			IMPLEMENTATION_GUARD(
			g_RenderCommandBuffer_00449400->cmdA =
				((long)(int)(g_RenderCommandBuffer_00449400 + 4) & 0xfffffffU) << 0x20 | 0x20000000;
			pRVar9->cmdB = 0;)
		}
		pRVar9 = pRVar5;
		if (g_RenderFrameData_004493f8 != pRenderMeshData) {
			g_RenderFrameData_004493f8 = pRenderMeshData;
			if (iVar10 == 0xe) {
				IMPLEMENTATION_GUARD(pRVar9 = FUN_002a97a0(pRVar5);)
			}
			else {
				if (pTVar2->field_0xc == 2) {
					IMPLEMENTATION_GUARD(
					pRVar5->cmdA = 0x30000000;
					pRVar5->cmdB = 0x1300000000000000;
					pRVar5[1].cmdA = 0x30000000;
					pRVar5[1].cmdB = 0x1100000011000000;
					pRVar5[2].cmdA = 0x30000000;
					pRVar5[2].cmdB = 0x600000000000000;
					pRVar5[3].cmdA = 0xe10000001;
					pRVar5[3].cmdB = 0;
					*(undefined4*)&pRVar5[4].cmdA = in_zero_lo;
					*(undefined4*)((ulong)&pRVar5[4].cmdA + 4) = in_zero_hi;
					*(undefined4*)&pRVar5[4].cmdB = in_zero_udw;
					*(undefined4*)((ulong)&pRVar5[4].cmdB + 4) = in_register_0000000c;
					pRVar5[5].cmdA = 0x30000000;
					pRVar5[5].cmdB = 0x600800000000000;
					pRVar9 = pRVar5 + 6;
					g_RenderCommandBuffer_00449400 = pRVar5 + 2;)
				}
				else {
					pRVar5->cmdA = 0x30000000;
					pRVar5->cmdB = 0x1300000000000000;
					pRVar5[1].cmdA =
						((ulong)(int)(pTVar2->pCommandBuffer_0x8 + INT_00449330 * pTVar2->field_0xc * (uint)pTVar2->count_0x0) & 0xfffffffU)
						<< 0x20 | (ulong)pTVar2->field_0xc & 0xffffffffU | 0x30000000;
					pRVar5[1].cmdB = 0;
					pRVar5[2].cmdA = 0x30000000;
					pRVar5[2].cmdB = 0x1100000011000000;
					pRVar5[3].cmdA = 0x30000000;
					pRVar5[3].cmdB = 0x600000000000000;
					pRVar5[4].cmdA = 0xe10000001;
					pRVar5[4].cmdB = 0;
					*(undefined4*)&pRVar5[5].cmdA = in_zero_lo;
					*(undefined4*)((ulong)&pRVar5[5].cmdA + 4) = in_zero_hi;
					*(undefined4*)&pRVar5[5].cmdB = in_zero_udw;
					*(undefined4*)((ulong)&pRVar5[5].cmdB + 4) = in_register_0000000c;
					pRVar5[6].cmdA = 0x30000000;
					pRVar5[6].cmdB = 0x600800000000000;
					pRVar9 = pRVar5 + 7;
					g_RenderCommandBuffer_00449400 = pRVar5 + 3;
				}
			}
		}
		g_VideoMemoryPtr_00449280 = pRVar9;
		if (pRenderMeshData->field_0x2c.ptrValue == (char*)0x0) {
			IMPLEMENTATION_GUARD(
			g_TextureData_LAY_Int_00449404 = (TextureData_LAY*)0x0;
			g_VideoMemoryPtr_00449280 = FUN_002a9e80(pRVar9);)
		}
		else {
			GetTEX2D_LAY_0029f170(pRenderMeshData, &PTR_DAT_0044940c, &g_TextureData_LAY_Int_00449404, 0);
			*(char**)ScratchpadAnimManager_004494ec.field_0x24 = pRenderMeshData->pFileData;
			pRVar5 = g_VideoMemoryPtr_00449280;
			g_VideoMemoryPtr_00449280[2].cmdA =
				(long)*(int*)ScratchpadAnimManager_004494ec.field_0x48 << 0x20 |
				(ulong) * (uint*)ScratchpadAnimManager_004494ec.field_0x44 << 0x13 |
				(ulong)(*(int*)ScratchpadAnimManager_004494ec.field_0x4c + 4) << 6 |
				((ulong)(int)((*(ushort*)(PTR_DAT_0044940c + 6) - 1) * *(int*)ScratchpadAnimManager_004494ec.field_0x40) &
					0xffffffffU) << 2 | 0x20;
			pRVar9 = g_VideoMemoryPtr_00449280 + 3;
			g_VideoMemoryPtr_00449280[2].cmdB = 0x14;
			iVar10 = 1;
			if ((g_TextureData_LAY_Int_00449404->field_0x4 & 0x80) != 0) {
				iVar10 = 3;
				pRVar9->cmdA = 0x50000;
				pRVar9 = g_VideoMemoryPtr_00449280 + 5;
				g_VideoMemoryPtr_00449280[3].cmdB = 0x47;
				g_VideoMemoryPtr_00449280[4].cmdA = 0x800000002a;
				g_VideoMemoryPtr_00449280[4].cmdB = 0x42;
			}
			if ((long)iVar10 != 0) {
				puVar3 = &g_VideoMemoryPtr_00449280->cmdA;
				g_VideoMemoryPtr_00449280 = pRVar9;
				*puVar3 = (ulong)(iVar10 + 2) | 0xe10000000;
				pRVar5->cmdB = 0;
				*(undefined4*)&pRVar5->cmdB = 0x40003dc;
				*(uint*)((ulong)&pRVar5->cmdB + 4) = (iVar10 + 1) * 0x10000 | 0x6c0003dc;
				pRVar5[1].cmdA = (long)iVar10 | 0x1000000000008000;
				pRVar5[1].cmdB = 0xe;
				*(undefined4*)&g_VideoMemoryPtr_00449280->cmdA = 0;
				*(undefined4*)((ulong)&g_VideoMemoryPtr_00449280->cmdA + 4) = 0x14000000;
				*(undefined4*)&g_VideoMemoryPtr_00449280->cmdB = 0;
				*(undefined4*)((ulong)&g_VideoMemoryPtr_00449280->cmdB + 4) = 0;
				g_VideoMemoryPtr_00449280 = g_VideoMemoryPtr_00449280 + 1;
			}
			//g_VideoMemoryPtr_00449280 = FUN_002a99f0(g_VideoMemoryPtr_00449280);
		}
		CallMeshRenderFunc_002a9f10(pRenderMeshData);
		pRVar5 = g_VideoMemoryPtr_00449280;
		if (*ScratchpadAnimManager_004494ec.field_0x3c == 0) {
			g_VideoMemoryPtr_00449280 = pRVar4;
			g_RenderCommand_004492c0 = pRVar6;
			g_RenderCommandBuffer_00449400 = pRVar7;
			pRenderMeshData->field_0x2c.numValue = 0xdfdfdfdf;
		}
		else {
			g_VideoMemoryPtr_00449280->cmdA = 0xe10000001;
			pRVar5->cmdB = 0;
			pRVar4 = g_VideoMemoryPtr_00449280;
			g_VideoMemoryPtr_00449280 = g_VideoMemoryPtr_00449280 + 1;
			g_VideoMemoryPtr_00449280->cmdA = 0x2000000001000101;
			pRVar4[1].cmdB = 0xffffff00;
			g_VideoMemoryPtr_00449280 = g_VideoMemoryPtr_00449280 + 1;
			//FUN_002a98e0(pRenderMeshData);
		}
		return;
	}

	void ProcessDrawTypes_0029bc30(void)
	{
		float fVar1;
		int iVar2;
		CameraPanMasterHeader* unaff_s1_lo;
		MeshTransformSpecial* pCVar5;
		CameraPanMasterHeader* pCVar3;
		CameraPanMasterHeader* pCVar4;

		INT_00449238 = 0;
		g_CurrentDrawType_0044923c = 0;
		do {
			fVar1 = FLOAT_00448a04;
			if (0xe < g_CurrentDrawType_0044923c) {
				return;
			}
			/* Get the camera type, and then add the flip index as there is two per type. */
			pCVar4 = g_CameraPanDrawTypeArray_0048c710[g_CurrentDrawType_0044923c] + g_CameraFlipIndex_004494a4;
			pCVar5 = pCVar4->pLoadedData;
			if ((CameraPanMasterHeader*)pCVar5 != pCVar4) {
				switch (g_CurrentDrawType_0044923c) {
				default:
					/* 0x7 = Particle
					   0x3 = Some level mesh
					   0x2 = Character */
					((CameraPanMasterHeader*)pCVar5)->pCameraPanHeader_0xc[2].totalCount =
						((CameraPanMasterHeader*)pCVar5)->pCameraPanHeader_0xc[2].totalCount | 2;
					while (pCVar3 = (CameraPanMasterHeader*)pCVar5, pCVar3 != pCVar4) {
						CallMeshRenderFunc_002aa050((RenderFrame_30*)pCVar3->pCameraPanHeader_0xc);
						unaff_s1_lo = pCVar3;
						pCVar5 = (MeshTransformSpecial*)pCVar3->pLoadedData;
					}
					break;
				case 1:
				case 5:
				case 6:
				case 7:
				case 9:
					IMPLEMENTATION_GUARD(if (DAT_00448a60 != '\0') {
						g_VideoMemoryPtr_00449280 = FUN_002b00d0(g_VideoMemoryPtr_00449280);
					}
					((CameraPanMasterHeader*)pCVar5)->pCameraPanHeader_0xc[2].totalCount =
						((CameraPanMasterHeader*)pCVar5)->pCameraPanHeader_0xc[2].totalCount | 2;
					for (; (CameraPanMasterHeader*)pCVar5 != pCVar4; pCVar5 = pCVar5->pNext_0x4) {
						CallMeshRenderFunc_002aa050((RenderFrame_30*)pCVar5->pRenderInput);
						unaff_s1_lo = (CameraPanMasterHeader*)pCVar5;
					}
					if (DAT_00448a60 != '\0') {
						g_VideoMemoryPtr_00449280 = FUN_002affd0(g_VideoMemoryPtr_00449280);
					})
					break;
				case 10:
					IMPLEMENTATION_GUARD(((CameraPanMasterHeader*)pCVar5)->pCameraPanHeader_0xc[2].totalCount =
						((CameraPanMasterHeader*)pCVar5)->pCameraPanHeader_0xc[2].totalCount | 2;
					for (; (CameraPanMasterHeader*)pCVar5 != pCVar4;
						pCVar5 = (MeshTransformSpecial*)((CameraPanMasterHeader*)pCVar5)->pLoadedData) {
						CallMeshRenderFunc_002aa050((RenderFrame_30*)((CameraPanMasterHeader*)pCVar5)->pCameraPanHeader_0xc);
						unaff_s1_lo = (CameraPanMasterHeader*)pCVar5;
					})
					g_VideoMemoryPtr_00449280 = SetScreenParams_002aabd0(g_VideoMemoryPtr_00449280, &CameraScreenData_00449390);
					break;
				case 0xb:
					IMPLEMENTATION_GUARD(
					g_VideoMemoryPtr_00449280 = (RenderCommand*)FUN_002afc10((ulong*)g_VideoMemoryPtr_00449280);
					g_VideoMemoryPtr_00449280 = SetScreenParams_002aabd0(g_VideoMemoryPtr_00449280, &CameraScreenData_00449390);
					((CameraPanMasterHeader*)pCVar5)->pCameraPanHeader_0xc[2].totalCount =
						((CameraPanMasterHeader*)pCVar5)->pCameraPanHeader_0xc[2].totalCount & 0xfffffffd;
					for (; (CameraPanMasterHeader*)pCVar5 != pCVar4;
						pCVar5 = (MeshTransformSpecial*)((CameraPanMasterHeader*)pCVar5)->pLoadedData) {
						CallMeshRenderFunc_002aa050((RenderFrame_30*)((CameraPanMasterHeader*)pCVar5)->pCameraPanHeader_0xc);
						unaff_s1_lo = (CameraPanMasterHeader*)pCVar5;
					}
					g_VideoMemoryPtr_00449280 = (RenderCommand*)FUN_002af9c0(g_VideoMemoryPtr_00449280);
					g_VideoMemoryPtr_00449280 = SetScreenParams_002aabd0(g_VideoMemoryPtr_00449280, &CameraScreenData_00449390);)
				case 8:
					IMPLEMENTATION_GUARD(
					if (DAT_00448a60 != '\0') {
						g_VideoMemoryPtr_00449280 = FUN_002b00d0(g_VideoMemoryPtr_00449280);
					}
					((CameraPanMasterHeader*)pCVar5)->pCameraPanHeader_0xc[2].totalCount =
						((CameraPanMasterHeader*)pCVar5)->pCameraPanHeader_0xc[2].totalCount | 2;
					for (; (CameraPanMasterHeader*)pCVar5 != pCVar4;
						pCVar5 = (MeshTransformSpecial*)((CameraPanMasterHeader*)pCVar5)->pLoadedData) {
						CallMeshRenderFunc_002aa050((RenderFrame_30*)((CameraPanMasterHeader*)pCVar5)->pCameraPanHeader_0xc);
						unaff_s1_lo = (CameraPanMasterHeader*)pCVar5;
					}
					if (DAT_00448a60 != '\0') {
						g_VideoMemoryPtr_00449280 = FUN_002affd0(g_VideoMemoryPtr_00449280);
					})
					break;
				case 0xd:
					IMPLEMENTATION_GUARD(
					FLOAT_00448a04 = 0.0;
					g_VideoMemoryPtr_00449280 = (RenderCommand*)FUN_002b4e60((ulong*)g_VideoMemoryPtr_00449280);
					BYTE_004489e4 = 0;
					g_CurrentDrawType_0044923c = 0xe;
					((CameraPanMasterHeader*)pCVar5)->pCameraPanHeader_0xc[2].totalCount =
						((CameraPanMasterHeader*)pCVar5)->pCameraPanHeader_0xc[2].totalCount | 2;
					for (; (CameraPanMasterHeader*)pCVar5 != pCVar4;
						pCVar5 = (MeshTransformSpecial*)((CameraPanMasterHeader*)pCVar5)->pLoadedData) {
						CallMeshRenderFunc_002aa050((RenderFrame_30*)((CameraPanMasterHeader*)pCVar5)->pCameraPanHeader_0xc);
						unaff_s1_lo = (CameraPanMasterHeader*)pCVar5;
					}
					g_CurrentDrawType_0044923c = 0xd;
					FLOAT_00448a04 = fVar1;)
					break;
				case 0xe:
					IMPLEMENTATION_GUARD(
					g_VideoMemoryPtr_00449280 = (RenderCommand*)FUN_002b4e60((ulong*)g_VideoMemoryPtr_00449280);
					BYTE_004489e4 = 1;
					((CameraPanMasterHeader*)pCVar5)->pCameraPanHeader_0xc[2].totalCount =
						((CameraPanMasterHeader*)pCVar5)->pCameraPanHeader_0xc[2].totalCount | 2;
					for (; (CameraPanMasterHeader*)pCVar5 != pCVar4;
						pCVar5 = (MeshTransformSpecial*)((CameraPanMasterHeader*)pCVar5)->pLoadedData) {
						CallMeshRenderFunc_002aa050((RenderFrame_30*)((CameraPanMasterHeader*)pCVar5)->pCameraPanHeader_0xc);
						unaff_s1_lo = (CameraPanMasterHeader*)pCVar5;
					}
					g_VideoMemoryPtr_00449280 = (RenderCommand*)FUN_002b4d50();)
				}
				iVar2 = g_CameraFlipIndex_004494a4;
				if ((CameraPanMasterHeader*)g_ModelDoubleBuffer_0048c6d0[g_CameraFlipIndex_004494a4].pLoadedData ==
					g_ModelDoubleBuffer_0048c6d0 + g_CameraFlipIndex_004494a4) {
					pCVar4 = g_CameraPanDrawTypeArray_0048c710[g_CurrentDrawType_0044923c] + g_CameraFlipIndex_004494a4;
					g_ModelDoubleBuffer_0048c6d0[g_CameraFlipIndex_004494a4].pLoadedData = pCVar4->pLoadedData;
					g_ModelDoubleBuffer_0048c6d0[iVar2].count_0x14 = pCVar4->count_0x14;
				}
				else {
					IMPLEMENTATION_GUARD(
					pCVar4 = g_CameraPanDrawTypeArray_0048c710[g_CurrentDrawType_0044923c] + g_CameraFlipIndex_004494a4;
					g_ModelDoubleBuffer_0048c6d0[g_CameraFlipIndex_004494a4].count_0x14 =
						g_ModelDoubleBuffer_0048c6d0[g_CameraFlipIndex_004494a4].count_0x14 + pCVar4->count_0x14;
					g_MeshTransformSpecial_004494a8->pNext_0x4 = pCVar4->pLoadedData;)
				}
				pCVar4 = g_CameraPanDrawTypeArray_0048c710[g_CurrentDrawType_0044923c] + g_CameraFlipIndex_004494a4;
				g_MeshTransformSpecial_004494a8 = (MeshTransformSpecial*)unaff_s1_lo;
				pCVar4->count_0x14 = 0;
				pCVar4->pLoadedData = (MeshTransformSpecial*)pCVar4;
			}
			g_CurrentDrawType_0044923c = g_CurrentDrawType_0044923c + 1;
		} while (true);
	}

	RenderCommand* FUN_002b00d0(RenderCommand* pRenderCommand)
	{
		FrameBuffer* pFVar1;

		pFVar1 = GetFramebuffer_002ba3b0();
		if (pFVar1 != (FrameBuffer*)0x0) {
			//DAT_00449450 = 0x40;
			//DAT_00449448 = 0x7000;
			//DAT_00449468 = 0xb0;
			//DAT_0044944c = 0x7000;
			//DAT_00449474 = 0x1600;
			//DAT_00449454 = 0x800;
			g_ZBufferFrameBasePtr_0044945c = g_RenderActiveCameraObj_006f7d80->pZBuffer->frameBasePtr;
			//DAT_00449460 = 0x200;
			//DAT_00449464 = 0x200;
			//DAT_0044946c = 0xc0;
			//DAT_00449470 = 0x1800;
			//DAT_00449458 = g_ZBufferFrameBasePtr_0044945c << 5;
			pRenderCommand->cmdA = 0x30000000;
			pRenderCommand->cmdB = 0x1100000011000000;
			pRenderCommand[1].cmdA = 0xe10000002;
			pRenderCommand[1].cmdB = 0;
			*(undefined4*)&pRenderCommand[1].cmdB = 0;
			*(undefined4*)((ulong)&pRenderCommand[1].cmdB + 4) = 0x50000002;
			pRenderCommand[2].cmdA = SCE_GIF_SET_TAG(
				1,				// NLOOP
				SCE_GS_TRUE,	// EOP
				SCE_GS_FALSE,	// PRE
				0,				// PRIM
				SCE_GIF_PACKED,	// FLG
				1				// NREG
			);
			pRenderCommand[2].cmdB = SCE_GIF_PACKED_AD;

			// Z BUFFER
			pRenderCommand[3].cmdA = SCE_GS_SET_ZBUF(
				g_ZBufferFrameBasePtr_0044945c,	// ZBP
				SCE_GS_PSMZ24,					// PSM
				SCE_GS_TRUE						// ZMASK
			);
			pRenderCommand[3].cmdB = SCE_GS_ZBUF_1;
			pRenderCommand = pRenderCommand + 4;
		}
		return pRenderCommand;
	}

	RenderCommand* FUN_002affd0(RenderCommand* pRenderCommand)
	{
		FrameBuffer* pFVar1;

		pFVar1 = GetFramebuffer_002ba3b0();
		if (pFVar1 != (FrameBuffer*)0x0) {
			//DAT_00449448 = 0x7000;
			//DAT_0044944c = 0x7000;
			//DAT_00449450 = pFVar1->frameBasePtr;
			//DAT_00449474 = 0x800;
			//DAT_00449468 = 0x40;
			//DAT_00449454 = DAT_00449450 << 5;
			g_ZBufferFrameBasePtr_0044945c = g_RenderActiveCameraObj_006f7d80->pZBuffer->frameBasePtr;
			//DAT_00449460 = 0x200;
			//DAT_00449464 = 0x200;
			//DAT_00449458 = g_ZBufferFrameBasePtr_0044945c << 5;
			//DAT_0044946c = g_ZBufferFrameBasePtr_0044945c;
			//DAT_00449470 = DAT_00449458;
			pRenderCommand->cmdA = 0x30000000;
			pRenderCommand->cmdB = 0x1100000011000000;
			pRenderCommand[1].cmdA = 0xe10000002;
			pRenderCommand[1].cmdB = 0;
			*(undefined4*)&pRenderCommand[1].cmdB = 0;
			*(undefined4*)((ulong)&pRenderCommand[1].cmdB + 4) = 0x50000002;
			pRenderCommand[2].cmdA = SCE_GIF_SET_TAG(
				1,				// NLOOP
				SCE_GS_TRUE,	// EOP
				SCE_GS_FALSE,	// PRE
				0,				// PRIM
				SCE_GIF_PACKED,	// FLG
				1				// NREG
			);
			pRenderCommand[2].cmdB = SCE_GIF_PACKED_AD;
			// Z BUFFER
			pRenderCommand[3].cmdA = SCE_GS_SET_ZBUF(
				g_ZBufferFrameBasePtr_0044945c,	// ZBP
				SCE_GS_PSMZ24,					// PSM
				SCE_GS_FALSE					// ZMASK
			);
			pRenderCommand[3].cmdB = SCE_GS_ZBUF_1;
			pRenderCommand = pRenderCommand + 4;
		}
		return pRenderCommand;
	}

	void GenerateRenderingCommands_002b51d0(void)
	{
		MeshTransformParentHeader* pMVar1;
		bool bVar2;
		char** ppcVar3;
		int iVar4;
		RenderFrame_30* pRVar5;
		RenderCommand* pRVar6;
		MeshTransformSpecial* pMVar7;
		char* pcVar8;
		CameraPanMasterHeader* pCVar9;
		RenderFrame_30* pRenderFrame;

		bVar2 = false;
		g_VideoMemoryPtr_00449280 = FUN_002b00d0(g_VideoMemoryPtr_00449280);
		INT_00449238 = 0;
		g_bGeneratingMeshCommands_00449570 = 1;
		pRenderFrame = (RenderFrame_30*)(g_OtherDoubleBuffer_0048cd50[g_CameraFlipIndex_004494a4].pLoadedData)->pNext_0x4->pRenderInput;

		// Is the next equal to the render frame.
		if ((pRenderFrame->internalMeshTransformSpecial).pNext_0x4 != &pRenderFrame->internalMeshTransformSpecial) {
			IMPLEMENTATION_GUARD(
				memcpy(ScratchpadAnimManager_004494ec.pScreenSpaceMatrix, &Matrix_0048cbb0, sizeof(Matrix));
			memcpy(ScratchpadAnimManager_004494ec.pTextureSpaceMatrix, &Matrix_0048cbf0, sizeof(Matrix));
			memcpy(ScratchpadAnimManager_004494ec.pWorldSpaceMatrix, &Matrix_0048cc30, sizeof(Matrix));
			memcpy(g_ScratchpadAddr_00448a58, &Matrix_0048ccb0, sizeof(Matrix));
			memcpy(&Vector_0048c2d0, &Vector_0048ccf0, sizeof(Vector));
			memcpy(&Vector_0048c2e0, &Vector_0048cd00, sizeof(Vector));
			memcpy(&Vector_0048c2f0, &Vector_0048cd10, sizeof(Vector));
			memcpy(&Vector_0048c300, &Vector_0048cd20, sizeof(Vector));
			memcpy(&Vector_0048c310, &Vector_0048cd30, sizeof(Vector));
			memcpy(&Vector_0048c320, &Vector_0048cd40, sizeof(Vector));
			pRVar6 = g_VideoMemoryPtr_00449280;
			g_VideoMemoryPtr_00449280->cmdA = 0x30000000;
			pRVar6->cmdB = 0x1300000000000000;
			g_VideoMemoryPtr_00449280 = g_VideoMemoryPtr_00449280 + 1;
			g_VideoMemoryPtr_00449280 = (RenderCommand*)FUN_002b55d0((undefined8*)g_VideoMemoryPtr_00449280);
			g_VideoMemoryPtr_00449280 = FUN_0029c640(g_VideoMemoryPtr_00449280);
			g_VideoMemoryPtr_00449280 = SetupDrawEnvironment_002ab2d0((CameraObj_28*)PTR_DAT_0044956c, g_VideoMemoryPtr_00449280);
			pRVar6 = (RenderCommand*)FUN_002b69d0((undefined8*)g_VideoMemoryPtr_00449280, (short*)&CameraScreenData_00449390);
			g_VideoMemoryPtr_00449280 = pRVar6;
			pRVar6->cmdA = 0xe10000002;
			pRVar6->cmdB = 0;
			*(undefined4*)&pRVar6->cmdB = 0;
			*(undefined4*)((ulong)&g_VideoMemoryPtr_00449280->cmdB + 4) = 0x50000002;
			pRVar6 = g_VideoMemoryPtr_00449280;
			g_VideoMemoryPtr_00449280 = g_VideoMemoryPtr_00449280 + 1;
			g_VideoMemoryPtr_00449280->cmdA = 0x1000800000008001;
			pRVar6[1].cmdB = 0xe;
			pRVar6 = g_VideoMemoryPtr_00449280;
			g_VideoMemoryPtr_00449280 = g_VideoMemoryPtr_00449280 + 1;
			g_VideoMemoryPtr_00449280->cmdA =
				(ulong)g_FrontendStaticMeshMaster_00449558->pCameraObj28_0xc->pZBuffer->frameBasePtr | 0x130000000;
			pRVar6[1].cmdB = 0x4e;
			g_VideoMemoryPtr_00449280 = g_VideoMemoryPtr_00449280 + 1;
			CallMeshRenderFunc_002a9f10(pRenderFrame);
			pRVar6 = g_VideoMemoryPtr_00449280;
			g_VideoMemoryPtr_00449280->cmdA = 0x30000000;
			pRVar6->cmdB = 0x1300000000000000;
			iVar4 = g_CameraFlipIndex_004494a4;
			g_VideoMemoryPtr_00449280 = g_VideoMemoryPtr_00449280 + 1;
			g_ModelDoubleBuffer_0048c6d0[g_CameraFlipIndex_004494a4].count_0x14 =
				g_ModelDoubleBuffer_0048c6d0[g_CameraFlipIndex_004494a4].count_0x14 + 1;
			bVar2 = true;
			if (g_MeshTransformSpecial_004494a8 == (MeshTransformSpecial*)0x0) {
				g_MeshTransformSpecial_004494a8 = (g_OtherDoubleBuffer_0048cd50[iVar4].pLoadedData)->pNext_0x4;
			}
			else {
				g_MeshTransformSpecial_004494a8->pNext_0x4 = (g_OtherDoubleBuffer_0048cd50[iVar4].pLoadedData)->pNext_0x4;
				g_MeshTransformSpecial_004494a8 = g_MeshTransformSpecial_004494a8->pNext_0x4;
			}
			(g_OtherDoubleBuffer_0048cd50[g_CameraFlipIndex_004494a4].pLoadedData)->pNext_0x4 =
				(MeshTransformSpecial*)(g_OtherDoubleBuffer_0048cd50 + g_CameraFlipIndex_004494a4);
			pRVar5 = RenderFrame_30_Header_004494c0.pMeshCurrent;
			pMVar7 = &(RenderFrame_30_Header_004494c0.pMeshCurrent)->internalMeshTransformSpecial;
			ppcVar3 = &(RenderFrame_30_Header_004494c0.pMeshCurrent)->pFileData;
			RenderFrame_30_Header_004494c0.pMeshCurrent = RenderFrame_30_Header_004494c0.pMeshCurrent + 1;
			*ppcVar3 = &DAT_0048c3b0;
			pRVar5->field_0x4 = (MeshTransformSpecial*)0x0;
			pRVar5->flags_0x28 = pRVar5->flags_0x28 & 0xfffffffe;
			(pRVar5->internalMeshTransformSpecial).pPrev_0x8 = pMVar7;
			(pRVar5->internalMeshTransformSpecial).pNext_0x4 = pMVar7;
			(pRVar5->internalMeshTransformSpecial).pRenderInput = (DisplayListInternal*)PTR_MeshTransformParentHeader_004494b8;
			pRVar5->field_0x18 = 0;
			pRVar5->index_0x1c = 0;
			*(undefined4*)&pRVar5->internalMeshTransformSpecial = 0;
			pcVar8 = (char*)FUN_0029e370((char**)pRVar5, 0);
			pRenderFrame->field_0x2c = pcVar8;
			iVar4 = g_CameraFlipIndex_004494a4;
			pCVar9 = g_OtherDoubleBuffer_0048cd50 + g_CameraFlipIndex_004494a4;
			pMVar1 = g_OtherDoubleBuffer_0048cd50[g_CameraFlipIndex_004494a4].pCameraPanHeader_0xc;
			pMVar7 = pMVar1->field_0x4 + pMVar1->usedCount;
			pMVar7->pRenderInput = (DisplayListInternal*)pRVar5;
			pMVar7->specUnion = 1;
			pMVar1->usedCount = pMVar1->usedCount + 1;
			g_OtherDoubleBuffer_0048cd50[iVar4].count_0x14 = g_OtherDoubleBuffer_0048cd50[iVar4].count_0x14 + 1;
			pMVar7->pNext_0x4 = (MeshTransformSpecial*)pCVar9;
			(g_OtherDoubleBuffer_0048cd50[iVar4].pLoadedData)->pNext_0x4 = pMVar7;)
		}
		g_bGeneratingMeshCommands_00449570 = 0;
		*ScratchpadAnimManager_004494ec.field_0x28 = 0;
		if (bVar2) {
			//DAT_00449568 = 0;
		}
		g_VideoMemoryPtr_00449280 = FUN_002affd0(g_VideoMemoryPtr_00449280);
		return;
	}

	float FLOAT_00448a00 = 10.0f;

	typedef enum EVectorMode_A {
		VM_0 = 0,
		VM_1 = 1,
		VM_2 = 2,
		VM_3 = 3,
		VM_4 = 4
	} EVectorMode_A;


	TextureData_MAT_Internal_A* GetMAT_0029f2f0(char* pMBNK, int index)
	{
		int* piVar1;
		int* piVar2;

		piVar1 = (int*)LOAD_SECTION((int)*(int**)(pMBNK + index * 0x10 + 8));
		piVar2 = (int*)0x0;
		if ((piVar1 != (int*)0x0) && (piVar2 = piVar1 + 4, *piVar1 != 0x2e54414d)) {
			piVar2 = (int*)((char*)LOAD_SECTION(piVar1[2]) + 0x10);
		}
		return (TextureData_MAT_Internal_A*)piVar2;
	}


	EVectorMode_A GetVectorMode_0029a730(Vector* param_1)
	{
		EVectorMode_A EVar1;
		float fVar2;
		float fVar3;
		float fVar4;

		fVar2 = ((ScratchpadAnimManager_004494ec.pData_150)->field_0x20).field_0x50.w;
		fVar3 = (ScratchpadAnimManager_004494ec.vector_0x64)->z;
		EVar1 = VM_1;
		if (fVar3 < fVar2) {
			fVar4 = param_1->w;
			if (fVar4 <= fVar2 - fVar3) {
				return VM_4;
			}
			EVar1 = VM_2;
		}
		else {
			fVar4 = param_1->w;
			if (fVar3 - fVar2 < fVar4) {
				EVar1 = VM_2;
			}
		}
		fVar2 = ((ScratchpadAnimManager_004494ec.pData_150)->field_0x20).field_0x40.w;
		if (fVar3 < fVar2) {
			if (fVar2 - fVar3 < fVar4) {
				EVar1 = VM_2;
			}
		}
		else {
			if (fVar4 <= fVar3 - fVar2) {
				return VM_4;
			}
			EVar1 = VM_2;
		}
		fVar2 = (ScratchpadAnimManager_004494ec.vector_0x64)->x;
		if (fVar2 <= 0.0f) {
			fVar2 = -((ScratchpadAnimManager_004494ec.pData_150)->field_0x20).field_0x0.bc * fVar3 -
				((ScratchpadAnimManager_004494ec.pData_150)->field_0x20).field_0x0.ba * fVar2;
			if (0.0f < fVar2) {
				if (fVar2 <= fVar4) {
					EVar1 = VM_2;
				}
			}
			else {
				if (fVar4 < -fVar2) {
					return VM_4;
				}
				EVar1 = VM_2;
			}
		}
		else {
			fVar2 = -((ScratchpadAnimManager_004494ec.pData_150)->field_0x20).field_0x0.ac * fVar3 -
				((ScratchpadAnimManager_004494ec.pData_150)->field_0x20).field_0x0.aa * fVar2;
			if (0.0f < fVar2) {
				if (fVar2 <= fVar4) {
					EVar1 = VM_2;
				}
			}
			else {
				if (fVar4 < -fVar2) {
					return VM_4;
				}
				EVar1 = VM_2;
			}
		}
		fVar2 = (ScratchpadAnimManager_004494ec.vector_0x64)->y;
		if (fVar2 <= 0.0) {
			fVar2 = ((ScratchpadAnimManager_004494ec.pData_150)->field_0x20).field_0x0.cb * fVar2 +
				-((ScratchpadAnimManager_004494ec.pData_150)->field_0x20).field_0x0.cc * fVar3;
			if (0.0 < fVar2) {
				if (fVar2 <= fVar4) {
					EVar1 = VM_2;
				}
			}
			else {
				EVar1 = VM_4;
				if (-fVar2 <= fVar4) {
					EVar1 = VM_2;
				}
			}
		}
		else {
			fVar2 = ((ScratchpadAnimManager_004494ec.pData_150)->field_0x20).field_0x0.db * fVar2 +
				-((ScratchpadAnimManager_004494ec.pData_150)->field_0x20).field_0x0.dc * fVar3;
			if (0.0f < fVar2) {
				if (fVar2 <= fVar4) {
					EVar1 = VM_2;
				}
			}
			else {
				EVar1 = VM_4;
				if (-fVar2 <= fVar4) {
					EVar1 = VM_2;
				}
			}
		}
		return EVar1;
	}

	EVectorMode_A FUN_0029a6c0(Vector* param_1)
	{
		EVectorMode_A EVar1;
		float fVar2;
		float fVar3;

		fVar2 = ((ScratchpadAnimManager_004494ec.pData_150)->field_0x80).field_0x40.w;
		fVar3 = (ScratchpadAnimManager_004494ec.vector_0x64)->z;
		EVar1 = VM_1;
		if (fVar3 < fVar2) {
			if (-(fVar3 - fVar2) < param_1->w) {
				EVar1 = VM_2;
			}
		}
		else {
			EVar1 = VM_4;
			if (-(fVar2 - fVar3) < param_1->w) {
				EVar1 = VM_2;
			}
		}
		return EVar1;
	}


	EVectorMode_A FUN_00299fa0(Vector* param_1)
	{
		EVectorMode_A EVar1;
		float fVar2;
		float fVar3;
		float fVar4;

		fVar2 = (ScratchpadAnimManager_004494ec.vector_0x64)->x;
		EVar1 = VM_1;
		if (fVar2 <= 0.0f) {
			fVar4 = -(ScratchpadAnimManager_004494ec.vector_0x64)->z;
			fVar2 = ((ScratchpadAnimManager_004494ec.pData_150)->field_0x80).field_0x0.ba * -fVar2 +
				((ScratchpadAnimManager_004494ec.pData_150)->field_0x80).field_0x0.bc * fVar4;
			if (0.0f < fVar2) {
				fVar3 = param_1->w;
				if (fVar2 <= fVar3) {
					EVar1 = VM_2;
				}
			}
			else {
				fVar3 = param_1->w;
				if (fVar3 < -fVar2) {
					return VM_4;
				}
				EVar1 = VM_2;
			}
		}
		else {
			fVar4 = -(ScratchpadAnimManager_004494ec.vector_0x64)->z;
			fVar2 = ((ScratchpadAnimManager_004494ec.pData_150)->field_0x80).field_0x0.aa * -fVar2 +
				((ScratchpadAnimManager_004494ec.pData_150)->field_0x80).field_0x0.ac * fVar4;
			if (0.0f < fVar2) {
				fVar3 = param_1->w;
				if (fVar2 <= fVar3) {
					EVar1 = VM_2;
				}
			}
			else {
				fVar3 = param_1->w;
				if (fVar3 < -fVar2) {
					return VM_4;
				}
				EVar1 = VM_2;
			}
		}
		fVar2 = (ScratchpadAnimManager_004494ec.vector_0x64)->y;
		if (fVar2 <= 0.0f) {
			fVar2 = ((ScratchpadAnimManager_004494ec.pData_150)->field_0x80).field_0x0.cb * fVar2 +
				((ScratchpadAnimManager_004494ec.pData_150)->field_0x80).field_0x0.cc * fVar4;
			if (0.0f < fVar2) {
				if (fVar2 <= fVar3) {
					EVar1 = VM_2;
				}
			}
			else {
				EVar1 = VM_4;
				if (-fVar2 <= fVar3) {
					EVar1 = VM_2;
				}
			}
		}
		else {
			fVar2 = ((ScratchpadAnimManager_004494ec.pData_150)->field_0x80).field_0x0.db * fVar2 +
				((ScratchpadAnimManager_004494ec.pData_150)->field_0x80).field_0x0.dc * fVar4;
			if (0.0f < fVar2) {
				if (fVar2 <= fVar3) {
					EVar1 = VM_2;
				}
			}
			else {
				EVar1 = VM_4;
				if (-fVar2 <= fVar3) {
					EVar1 = VM_2;
				}
			}
		}
		return EVar1;
	}

	void sceVu0ApplyMatrix(Vector* v0, Matrix* m0, Vector* v1)
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
		float fVar14;
		float fVar15;
		float fVar16;

		fVar13 = v1->x;
		fVar14 = v1->y;
		fVar15 = v1->z;
		fVar16 = v1->w;
		fVar1 = m0->ab;
		fVar2 = m0->ac;
		fVar3 = m0->ad;
		fVar4 = m0->bb;
		fVar5 = m0->bc;
		fVar6 = m0->bd;
		fVar7 = m0->cb;
		fVar8 = m0->cc;
		fVar9 = m0->cd;
		fVar10 = m0->db;
		fVar11 = m0->dc;
		fVar12 = m0->dd;
		v0->x = m0->aa * fVar13 + m0->ba * fVar14 + m0->ca * fVar15 + m0->da * fVar16;
		v0->y = fVar1 * fVar13 + fVar4 * fVar14 + fVar7 * fVar15 + fVar10 * fVar16;
		v0->z = fVar2 * fVar13 + fVar5 * fVar14 + fVar8 * fVar15 + fVar11 * fVar16;
		v0->w = fVar3 * fVar13 + fVar6 * fVar14 + fVar9 * fVar15 + fVar12 * fVar16;
		return;
	}

	float FLOAT_00448514 = 0.1f;

	EVectorMode_A FUN_0029a350(TextureData_MYSTERY* param_1)
	{
		EVectorMode_A EVar1;
		float fVar2;
		float fVar3;

		fVar2 = ((ScratchpadAnimManager_004494ec.pData_150)->field_0x80).field_0x50.w;
		fVar3 = (ScratchpadAnimManager_004494ec.vector_0x64)->z;
		EVar1 = VM_1;
		if ((fVar2 <= fVar3) || (-(fVar3 - fVar2) < (float)(uint)param_1->field_0xc)) {
			fVar2 = (ScratchpadAnimManager_004494ec.vector_0x64)->x;
			if (fVar2 <= 0.0f) {
				fVar2 = ((ScratchpadAnimManager_004494ec.pData_150)->field_0x80).field_0x0.ba * -fVar2 +
					((ScratchpadAnimManager_004494ec.pData_150)->field_0x80).field_0x0.bc * -fVar3;
				if ((fVar2 <= 0.0f) && ((float)(uint)param_1->field_0xc < -fVar2)) {
					return VM_4;
				}
			}
			else {
				fVar2 = ((ScratchpadAnimManager_004494ec.pData_150)->field_0x80).field_0x0.aa * -fVar2 +
					((ScratchpadAnimManager_004494ec.pData_150)->field_0x80).field_0x0.ac * -fVar3;
				if ((fVar2 <= 0.0f) && ((float)(uint)param_1->field_0xc < -fVar2)) {
					return VM_4;
				}
			}
			fVar2 = (ScratchpadAnimManager_004494ec.vector_0x64)->y;
			if (fVar2 <= 0.0f) {
				fVar2 = ((ScratchpadAnimManager_004494ec.pData_150)->field_0x80).field_0x0.cb * fVar2 +
					((ScratchpadAnimManager_004494ec.pData_150)->field_0x80).field_0x0.cc * -fVar3;
				if ((fVar2 <= 0.0f) && ((float)(uint)param_1->field_0xc < -fVar2)) {
					return VM_4;
				}
			}
			else {
				fVar2 = ((ScratchpadAnimManager_004494ec.pData_150)->field_0x80).field_0x0.db * fVar2 +
					((ScratchpadAnimManager_004494ec.pData_150)->field_0x80).field_0x0.dc * -fVar3;
				if ((fVar2 <= 0.0f) && ((float)(uint)param_1->field_0xc < -fVar2)) {
					return VM_4;
				}
			}
			fVar2 = ((ScratchpadAnimManager_004494ec.pData_150)->field_0x80).field_0x40.w;
			if (fVar3 < fVar2) {
				if (-(fVar3 - fVar2) < (float)(uint)param_1->field_0xc + FLOAT_00448514 * (float)(uint)param_1->field_0xc) {
					EVar1 = VM_2;
				}
			}
			else {
				EVar1 = VM_4;
				if (-(fVar2 - fVar3) < (float)(uint)param_1->field_0xc + FLOAT_00448514 * (float)(uint)param_1->field_0xc) {
					EVar1 = VM_2;
				}
			}
		}
		else {
			EVar1 = VM_4;
		}
		return EVar1;
	}

	EVectorMode_A FUN_00299c90(TextureData_MYSTERY* param_1)
	{
		EVectorMode_A EVar1;
		uint uVar2;
		float fVar3;
		float fVar4;

		fVar3 = (ScratchpadAnimManager_004494ec.vector_0x64)->x;
		EVar1 = VM_1;
		if (fVar3 <= 0.0f) {
			fVar4 = -(ScratchpadAnimManager_004494ec.vector_0x64)->z;
			fVar3 = ((ScratchpadAnimManager_004494ec.pData_150)->field_0x80).field_0x0.ba * -fVar3 +
				((ScratchpadAnimManager_004494ec.pData_150)->field_0x80).field_0x0.bc * fVar4;
			if (0.0f < fVar3) {
				uVar2 = (uint)param_1->field_0xc;
				if (fVar3 <= (float)uVar2) {
					EVar1 = VM_2;
				}
			}
			else {
				uVar2 = (uint)param_1->field_0xc;
				if ((float)uVar2 < -fVar3) {
					return VM_4;
				}
				EVar1 = VM_2;
			}
		}
		else {
			fVar4 = -(ScratchpadAnimManager_004494ec.vector_0x64)->z;
			fVar3 = ((ScratchpadAnimManager_004494ec.pData_150)->field_0x80).field_0x0.aa * -fVar3 +
				((ScratchpadAnimManager_004494ec.pData_150)->field_0x80).field_0x0.ac * fVar4;
			if (0.0f < fVar3) {
				uVar2 = (uint)param_1->field_0xc;
				if (fVar3 <= (float)uVar2) {
					EVar1 = VM_2;
				}
			}
			else {
				uVar2 = (uint)param_1->field_0xc;
				if ((float)uVar2 < -fVar3) {
					return VM_4;
				}
				EVar1 = VM_2;
			}
		}
		fVar3 = (ScratchpadAnimManager_004494ec.vector_0x64)->y;
		if (fVar3 <= 0.0f) {
			fVar3 = ((ScratchpadAnimManager_004494ec.pData_150)->field_0x80).field_0x0.cb * fVar3 +
				((ScratchpadAnimManager_004494ec.pData_150)->field_0x80).field_0x0.cc * fVar4;
			if (0.0f < fVar3) {
				if (fVar3 <= (float)uVar2) {
					EVar1 = VM_2;
				}
			}
			else {
				EVar1 = VM_4;
				if (-fVar3 <= (float)uVar2) {
					EVar1 = VM_2;
				}
			}
		}
		else {
			fVar3 = ((ScratchpadAnimManager_004494ec.pData_150)->field_0x80).field_0x0.db * fVar3 +
				((ScratchpadAnimManager_004494ec.pData_150)->field_0x80).field_0x0.dc * fVar4;
			if (0.0f < fVar3) {
				if (fVar3 <= (float)uVar2) {
					EVar1 = VM_2;
				}
			}
			else {
				EVar1 = VM_4;
				if (-fVar3 <= (float)uVar2) {
					EVar1 = VM_2;
				}
			}
		}
		return EVar1;
	}

	uint UINT_00448978 = 0x14;

	bool AllocateRenderData_30_00299650(RenderInput_40* param_1)
	{
		byte bVar1;
		Matrix* pMVar2;
		uint uVar3;
		Vector* pVVar4;
		int iVar5;
		EVectorMode_A EVar6;
		bool bVar7;
		int iVar8;
		TextureData_MYSTERY* pfVar8;
		uint uVar9;
		ulong uVar10;
		RenderFrame_30* pRVar11;
		uint uVar12;
		float fVar13;
		float fVar14;
		float fVar15;
		float fVar16;
		float fVar17;
		float fVar18;
		float fVar19;
		float fVar20;
		float fVar21;
		float fVar22;
		float fVar23;
		float fVar24;
		float in_vf9w;
		float extraout_vf9w;
		float extraout_vf9w_00;

		pfVar8 = (TextureData_MYSTERY*)LOAD_SECTION(param_1->pTextureDataMystery);
		if (pfVar8 == (TextureData_MYSTERY*)0x0) {
			bVar7 = true;
			param_1->flags_0x0 = param_1->flags_0x0 | 3;
		}
		else {
			uVar12 = (uint)(ushort)param_1->meshSectionCount_0x3a;
			uVar10 = 0;
			param_1->pRenderFrame30 = (int)STORE_SECTION(ed3D_SubAllocator_004491b8.current);
			pRVar11 = (RenderFrame_30*)LOAD_SECTION(param_1->pRenderFrame30);
			uVar9 = 0;
			while (pVVar4 = ScratchpadAnimManager_004494ec.vector_0x64, bVar7 = uVar12 != 0, uVar12 = uVar12 - 1, bVar7) {
				fVar13 = (pfVar8->field_0x0).y;
				fVar14 = (pfVar8->field_0x0).z;
				(ScratchpadAnimManager_004494ec.vector_0x64)->x = (pfVar8->field_0x0).x;
				pVVar4->y = fVar13;
				pVVar4->z = fVar14;
				(ScratchpadAnimManager_004494ec.vector_0x64)->w = 1.0;
				pVVar4 = ScratchpadAnimManager_004494ec.vector_0x64;
				pMVar2 = (ScratchpadAnimManager_004494ec.pRenderAnimationStatic_0x60)->pMeshTransformMatrix;
				fVar21 = (ScratchpadAnimManager_004494ec.vector_0x64)->x;
				fVar22 = (ScratchpadAnimManager_004494ec.vector_0x64)->y;
				fVar23 = (ScratchpadAnimManager_004494ec.vector_0x64)->z;
				fVar24 = (ScratchpadAnimManager_004494ec.vector_0x64)->w;
				fVar13 = pMVar2->ab;
				fVar14 = pMVar2->ac;
				fVar15 = pMVar2->bb;
				fVar16 = pMVar2->bc;
				fVar17 = pMVar2->cb;
				fVar18 = pMVar2->cc;
				fVar19 = pMVar2->db;
				fVar20 = pMVar2->dc;
				(ScratchpadAnimManager_004494ec.vector_0x64)->x =
					pMVar2->aa * fVar21 + pMVar2->ba * fVar22 + pMVar2->ca * fVar23 + pMVar2->da * fVar24;
				pVVar4->y = fVar13 * fVar21 + fVar15 * fVar22 + fVar17 * fVar23 + fVar19 * fVar24;
				pVVar4->z = fVar14 * fVar21 + fVar16 * fVar22 + fVar18 * fVar23 + fVar20 * fVar24;
				pVVar4->w = in_vf9w;
				EVar6 = FUN_0029a350(pfVar8);
				if (EVar6 == VM_1) {
					EVar6 = FUN_00299c90(pfVar8);
					in_vf9w = extraout_vf9w_00;
					if (EVar6 != VM_2) {
						iVar5 = 3;
						if (EVar6 == VM_4) {
							uVar9 = uVar9 + 1;
							iVar5 = 0;
						}
						goto LAB_0029976c;
					}
					iVar8 = 1;
					iVar5 = 2;
					if ((int)UINT_00448978 < (int)(uint)pfVar8->field_0xc) goto LAB_0029976c;
					bVar1 = *(byte*)&pRVar11->pFileData;
				}
				else {
					in_vf9w = extraout_vf9w;
					iVar5 = 2;
					if (EVar6 == VM_4) {
						uVar9 = uVar9 + 1;
						iVar5 = 0;
					}
				LAB_0029976c:
					iVar8 = iVar5;
					bVar1 = *(byte*)&pRVar11->pFileData;
				}
				uVar3 = uVar10 & 0x1f;
				uVar10 = uVar10 + 2;
				*(byte*)&pRVar11->pFileData = bVar1 | (byte)(iVar8 << uVar3);
				if (7 < (int)uVar10) {
					pRVar11 = (RenderFrame_30*)((ulong)&pRVar11->pFileData + 1);
					uVar10 = 0;
					*(undefined*)&pRVar11->pFileData = 0;
				}
				pfVar8 = pfVar8 + 1;
			}
			uVar10 = (ulong)&pRVar11->pFileData + 1;
			ed3D_SubAllocator_004491b8.current = (char*)(uVar10 + (8 - (uVar10 & 7)));
			bVar7 = uVar9 != (ushort)param_1->meshSectionCount_0x3a;
		}
		return bVar7;
	}

	void FUN_002974e0(RenderInput_40* param_1, char* pMBNK)
	{
		Matrix* pMVar1;
		int iVar3;
		bool bVar4;
		TextureData_MAT_Internal_A** ppTVar5;
		RenderFrame_30* pRVar6;
		Vector* pVVar7;
		EVectorMode_A EVar8;
		TextureData_MAT_Internal_A* piVar7;
		uint* puVar9;
		long lVar10;
		undefined2* puVar11;
		MeshTransformSpecial* pMVar12;
		int* piVar13;
		MeshTransformParentHeader* pDVar14;
		RenderFrame_30* pRVar15;
		char* pcVar16;
		uint uVar17;
		uint uVar18;
		float fVar19;
		float fVar20;
		float fVar21;
		float fVar22;
		float fVar23;
		float fVar24;
		float fVar25;
		float fVar26;
		float fVar27;
		float fVar28;
		float fVar29;
		float fVar30;
		float in_vf9w;
		float extraout_vf9w;
		float extraout_vf9w_00;
		float extraout_vf9w_01;
		float extraout_vf9w_02;
		float extraout_vf9w_03;
		TextureData_LAY* iVar1;
		TextureData_TEX* iVar2;

		if (param_1->field_0x4 == -1) {
			piVar7 = (TextureData_MAT_Internal_A*)0x0;
		}
		else {
			piVar7 = GetMAT_0029f2f0(pMBNK, (int)param_1->field_0x4);
			in_vf9w = extraout_vf9w;
			if ((piVar7 != (TextureData_MAT_Internal_A*)0x0) && ((piVar7->field_0x2 & 1) != 0)) {
				return;
			}
		}
		param_1->pRenderFrame30 = 0x0;
		pVVar7 = ScratchpadAnimManager_004494ec.vector_0x64;
		if ((ScratchpadAnimManager_004494ec.pRenderAnimationStatic_0x60)->field_0x8 == 1) {
			param_1->flags_0x0 = param_1->flags_0x0 & 0xfffffffc;
		}
		else {
			fVar19 = (param_1->vector_0x10).y;
			fVar20 = (param_1->vector_0x10).z;
			(ScratchpadAnimManager_004494ec.vector_0x64)->x = (param_1->vector_0x10).x;
			pVVar7->y = fVar19;
			pVVar7->z = fVar20;
			(ScratchpadAnimManager_004494ec.vector_0x64)->w = 1.0;
			pVVar7 = ScratchpadAnimManager_004494ec.vector_0x64;
			pMVar1 = (ScratchpadAnimManager_004494ec.pRenderAnimationStatic_0x60)->pMeshTransformMatrix;
			fVar27 = (ScratchpadAnimManager_004494ec.vector_0x64)->x;
			fVar28 = (ScratchpadAnimManager_004494ec.vector_0x64)->y;
			fVar29 = (ScratchpadAnimManager_004494ec.vector_0x64)->z;
			fVar30 = (ScratchpadAnimManager_004494ec.vector_0x64)->w;
			fVar19 = pMVar1->ab;
			fVar20 = pMVar1->ac;
			fVar21 = pMVar1->bb;
			fVar22 = pMVar1->bc;
			fVar23 = pMVar1->cb;
			fVar24 = pMVar1->cc;
			fVar25 = pMVar1->db;
			fVar26 = pMVar1->dc;
			(ScratchpadAnimManager_004494ec.vector_0x64)->x =
				pMVar1->aa * fVar27 + pMVar1->ba * fVar28 + pMVar1->ca * fVar29 + pMVar1->da * fVar30;
			pVVar7->y = fVar19 * fVar27 + fVar21 * fVar28 + fVar23 * fVar29 + fVar25 * fVar30;
			pVVar7->z = fVar20 * fVar27 + fVar22 * fVar28 + fVar24 * fVar29 + fVar26 * fVar30;
			pVVar7->w = in_vf9w;
			EVar8 = GetVectorMode_0029a730(&param_1->vector_0x10);
			if (EVar8 == VM_4) {
				return;
			}
			in_vf9w = extraout_vf9w_00;
			if (EVar8 != VM_1) {
				param_1->flags_0x0 = param_1->flags_0x0 | 3;
				if ((param_1->flags_0x0 & 0x1000) == 0) {
					EVar8 = FUN_0029a6c0(&param_1->vector_0x10);
					if (EVar8 == VM_1) {
						param_1->flags_0x0 = param_1->flags_0x0 & 0xfffffffc;
						EVar8 = FUN_00299fa0(&param_1->vector_0x10);
						in_vf9w = extraout_vf9w_01;
						if (EVar8 == VM_1) goto LAB_00297680;
						param_1->flags_0x0 = param_1->flags_0x0 | 1;
						lVar10 = AllocateRenderData_30_00299650(param_1);
						in_vf9w = extraout_vf9w_02;
					}
					else {
						lVar10 = AllocateRenderData_30_00299650(param_1);
						in_vf9w = extraout_vf9w_03;
					}
					if (lVar10 == 0) {
						return;
					}
				}
				else {
					param_1->flags_0x0 = param_1->flags_0x0 & 0xfffffffd;
				}
			}
		}
	LAB_00297680:
		if ((piVar7 != (TextureData_MAT_Internal_A*)0x0) && (piVar7->count_0x0 != 0)) {
			iVar1 = (TextureData_LAY*)LOAD_SECTION(*(int*)(piVar7 + 1));
			uVar18 = iVar1->body.field_0x0;
			uVar17 = iVar1->body.field_0x4;
			pcVar16 = (char*)0x0;
			if (iVar1->body.field_0x1c != 0) {
				iVar2 = (TextureData_TEX*)LOAD_SECTION(iVar1->body.pTex);
				iVar1->body.field_0x4 = uVar17 & 0xfffffbff;
				pVVar7 = ScratchpadAnimManager_004494ec.vector_0x64;
				if ((uVar17 & 0x10) == 0) {
				LAB_00297798:
					if ((uVar17 & 2) == 0) {
						param_1->flags_0x0 = param_1->flags_0x0 & 0xfffffdff;
					}
					else {
						IMPLEMENTATION_GUARD(
						puVar9 = (uint*)FUN_002b4d20(&iVar2->body);
						FUN_0029eca0(puVar9);
						param_1->flags_0x0 = param_1->flags_0x0 | 0x200;)
					}
				}
				else {
					fVar20 = (param_1->vector_0x10).y;
					fVar19 = (param_1->vector_0x10).z;
					(ScratchpadAnimManager_004494ec.vector_0x64)->x = (param_1->vector_0x10).x;
					pVVar7->y = fVar20;
					pVVar7->z = fVar19;
					(ScratchpadAnimManager_004494ec.vector_0x64)->w = 1.0;
					pVVar7 = ScratchpadAnimManager_004494ec.vector_0x64;
					pMVar1 = (ScratchpadAnimManager_004494ec.pRenderAnimationStatic_0x60)->pMeshTransformMatrix;
					fVar27 = (ScratchpadAnimManager_004494ec.vector_0x64)->x;
					fVar28 = (ScratchpadAnimManager_004494ec.vector_0x64)->y;
					fVar29 = (ScratchpadAnimManager_004494ec.vector_0x64)->z;
					fVar30 = (ScratchpadAnimManager_004494ec.vector_0x64)->w;
					fVar19 = pMVar1->ab;
					fVar20 = pMVar1->ac;
					fVar21 = pMVar1->bb;
					fVar22 = pMVar1->bc;
					fVar23 = pMVar1->cb;
					fVar24 = pMVar1->cc;
					fVar25 = pMVar1->db;
					fVar26 = pMVar1->dc;
					(ScratchpadAnimManager_004494ec.vector_0x64)->x =
						pMVar1->aa * fVar27 + pMVar1->ba * fVar28 + pMVar1->ca * fVar29 + pMVar1->da * fVar30;
					pVVar7->y = fVar19 * fVar27 + fVar21 * fVar28 + fVar23 * fVar29 + fVar25 * fVar30;
					pVVar7->z = fVar20 * fVar27 + fVar22 * fVar28 + fVar24 * fVar29 + fVar26 * fVar30;
					pVVar7->w = in_vf9w;
					fVar19 = (iVar2->body).field_0x18;
					if (fVar19 <= 0.0) {
						fVar19 = FLOAT_00448a00;
					}
					if (-(ScratchpadAnimManager_004494ec.vector_0x64)->z < fVar19) {
						bVar4 = true;
					}
					else {
						bVar4 = false;
						if (-(ScratchpadAnimManager_004494ec.vector_0x64)->z - fVar19 < (param_1->vector_0x10).w) {
							bVar4 = true;
						}
					}
					if (!bVar4) goto LAB_00297798;
					param_1->flags_0x0 = param_1->flags_0x0 | 0x200;
				}
				if ((iVar2->body).field_0x10 == 0) {
					IMPLEMENTATION_GUARD(
					iVar3 = (iVar2->body).field_0x8;
					if (iVar3 != 0) {
						pcVar16 = (char*)(*(int*)(iVar3 + 8) + 0x10);
					})
				}
				else {
					iVar3 = *(int*)(&iVar2[1].field_0x8 + (uint)iVar1->body.field_0x1e * 0x10);
					if (iVar3 != 0) {
						char* bb = (char*)LOAD_SECTION(iVar3);
						char* aa = (char*)LOAD_SECTION(*(int*)(bb + 8));
						pcVar16 = (char*)(aa + 0x10);
					}
				}
				if ((param_1->flags_0x0 & 4) != 0) {
					IMPLEMENTATION_GUARD(puVar9 = (uint*)FUN_002b4d40(param_1);
					FUN_0029eca0(puVar9);)
				}
				goto LAB_00297870;
			}
			if ((uVar17 & 0x800) != 0) goto LAB_00297870;
		}
		pcVar16 = (char*)0x0;
		uVar18 = 0;
		uVar17 = 0;
		piVar7 = (TextureData_MAT_Internal_A*)PTR_TextureData_MAT_004492f0;
	LAB_00297870:
		pRVar6 = RenderFrame_30_Header_004494c0.pMeshCurrent;
		if ((((uVar18 & 0xfc) == 0) || ((uVar18 & 0x80000000) != 0)) || ((uVar18 & 0x4000) != 0)) {
			pRVar15 = (RenderFrame_30*)LOAD_SECTION(piVar7->pRenderFrame30);
			if (piVar7->pRenderFrame30 == 0x0) {
				pMVar12 = &(RenderFrame_30_Header_004494c0.pMeshCurrent)->internalMeshTransformSpecial;
				ppTVar5 = (TextureData_MAT_Internal_A**)&(RenderFrame_30_Header_004494c0.pMeshCurrent)->pFileData;
				RenderFrame_30_Header_004494c0.pMeshCurrent = RenderFrame_30_Header_004494c0.pMeshCurrent + 1;
				*ppTVar5 = piVar7;
				pRVar6->field_0x4 = (MeshTransformSpecial*)0x0;
				pRVar6->flags_0x28 = pRVar6->flags_0x28 & 0xfffffffe;
				(pRVar6->internalMeshTransformSpecial).pPrev_0x8 = pMVar12;
				(pRVar6->internalMeshTransformSpecial).pNext_0x4 = pMVar12;
				(pRVar6->internalMeshTransformSpecial).pRenderInput =
					(DisplayListInternal*)PTR_MeshTransformParentHeader_004494b8;
				pRVar6->field_0x18 = 0;
				pRVar6->index_0x1c = 0;
				*(undefined4*)&pRVar6->internalMeshTransformSpecial = 0;
				pRVar6->flags_0x28 = pRVar6->flags_0x28 | 1;
				piVar7->pRenderFrame30 = (int)STORE_SECTION(pRVar6);
				pRVar6->field_0x2c.ptrValue = pcVar16;
				if (param_1->cameraPanIndex == 0) {
					if ((uVar17 & 0x40) == 0) {
						if ((uVar17 & 0x80) == 0) {
							if ((uVar18 & 0x80000000) == 0) {
								if ((uVar18 & 0x4000) == 0) {
									if (((uVar18 & 0x100) == 0) && ((uVar18 & 0xfc) == 0)) {
										param_1->cameraPanIndex = 0;
									}
									else {
										param_1->cameraPanIndex = 2;
									}
								}
								else {
									param_1->cameraPanIndex = 4;
								}
							}
							else {
								param_1->cameraPanIndex = 5;
							}
						}
						else {
							param_1->cameraPanIndex = 0xb;
						}
					}
					else {
						if ((uVar18 & 0x80000000) == 0) {
							if ((uVar18 & 0x4000) == 0) {
								if ((uVar18 & 0xfc) == 0) {
									if ((uVar18 & 0x100) == 0) {
										param_1->cameraPanIndex = 1;
									}
									else {
										param_1->cameraPanIndex = 2;
									}
								}
								else {
									param_1->cameraPanIndex = 3;
								}
							}
							else {
								param_1->cameraPanIndex = 4;
							}
						}
						else {
							param_1->cameraPanIndex = 5;
						}
					}
				}
				CameraPanMasterHeader* pCameraPanMasterHeader = &g_CameraPanDrawTypeArray_0048c710[param_1->cameraPanIndex][g_CameraFlipIndex_004494a4];
				MeshTransformParentHeader* pHeader = pCameraPanMasterHeader->pCameraPanHeader_0xc;
				pMVar12 = pHeader->field_0x4 + pHeader->usedCount;
				pMVar12->pRenderInput = (DisplayListInternal*)pRVar6;
				pMVar12->specUnion.field_0x0[0] = 1;
				pHeader->usedCount = pHeader->usedCount + 1;
				pCameraPanMasterHeader->count_0x14 = pCameraPanMasterHeader->count_0x14 + 1;
				pMVar12->pNext_0x4 = pCameraPanMasterHeader->pLoadedData;
				pCameraPanMasterHeader->pLoadedData = pMVar12;
				pRVar15 = pRVar6;
			}
			pDVar14 = (MeshTransformParentHeader*)(pRVar15->internalMeshTransformSpecial).pRenderInput;
		}
		else {
			IMPLEMENTATION_GUARD();
			if ((ScratchpadAnimManager_004494ec.pRenderAnimationStatic_0x60)->field_0x8 == 1) {
				(ScratchpadAnimManager_004494ec.vector_0x64)->x = (param_1->vector_0x10).x;
				(ScratchpadAnimManager_004494ec.vector_0x64)->y = (param_1->vector_0x10).y;
				(ScratchpadAnimManager_004494ec.vector_0x64)->z = (param_1->vector_0x10).z;
				(ScratchpadAnimManager_004494ec.vector_0x64)->w = 1.0;
				sceVu0ApplyMatrix(ScratchpadAnimManager_004494ec.vector_0x64, g_ScratchpadAddr_00448a58,
					ScratchpadAnimManager_004494ec.vector_0x64);
			}
			pRVar15 = RenderFrame_30_Header_004494c0.pMeshCurrent;
			pMVar12 = &(RenderFrame_30_Header_004494c0.pMeshCurrent)->internalMeshTransformSpecial;
			ppTVar5 = (TextureData_MAT_Internal_A**)&(RenderFrame_30_Header_004494c0.pMeshCurrent)->pFileData;
			RenderFrame_30_Header_004494c0.pMeshCurrent = RenderFrame_30_Header_004494c0.pMeshCurrent + 1;
			*ppTVar5 = piVar7;
			pRVar15->field_0x4 = 0; // (MeshTransformSpecial*)(ScratchpadAnimManager_004494ec.vector_0x64)->z;
			pRVar15->flags_0x28 = pRVar15->flags_0x28 & 0xfffffffe;
			(pRVar15->internalMeshTransformSpecial).pPrev_0x8 = pMVar12;
			(pRVar15->internalMeshTransformSpecial).pNext_0x4 = pMVar12;
			(pRVar15->internalMeshTransformSpecial).pRenderInput = (DisplayListInternal*)PTR_MeshTransformParentHeader_004494b8
				;
			pRVar15->field_0x18 = 0;
			pRVar15->index_0x1c = 0;
			*(undefined4*)&pRVar15->internalMeshTransformSpecial = 0;
			pRVar15->flags_0x28 = pRVar15->flags_0x28 | 1;
			pRVar15->field_0x2c.ptrValue = pcVar16;
			//FUN_002b1040(g_CameraFlipIndex_004494a4 * 0x20 + 0x48c7d0, (int)pRVar15, 1);
			pDVar14 = (MeshTransformParentHeader*)(pRVar15->internalMeshTransformSpecial).pRenderInput;
		}
		pMVar12 = pDVar14->field_0x4 + pDVar14->usedCount;
		pMVar12->pRenderInput = (DisplayListInternal*)param_1;
		pMVar12->specUnion.field_0x0[0] = 3;
		pMVar12->specUnion.field_0x0[1] = (ushort)param_1->flags_0x0;
		pDVar14->usedCount = pDVar14->usedCount + 1;
		pRVar15->index_0x1c = pRVar15->index_0x1c + 1;
		pMVar12->pNext_0x4 = (pRVar15->internalMeshTransformSpecial).pNext_0x4;
		pMVar12->pPrev_0x8 = (MeshTransformSpecial*)param_1->pRenderFrame30;
		(pRVar15->internalMeshTransformSpecial).pNext_0x4 = pMVar12;
		return;
	}

	void AnimFunc_00294af0(AnimFuncInputData_34* param_1)
	{
		bool bVar1;
		ushort uVar2;
		char* pcVar4;
		Matrix* pMVar5;
		bool bVar6;
		Vector* pVVar7;
		int* piVar8;
		uint uVar9;
		int iVar10;
		char cVar11;
		int* piVar12;
		RenderInput_40* pcVar13;
		uint uVar14;
		ulong uVar15;
		long lVar16;
		ushort* puVar17;
		ushort* puVar18;
		uint uVar19;
		uint uVar20;
		float fVar21;
		float fVar22;
		float fVar23;
		float fVar24;
		float fVar25;
		float fVar26;
		float fVar27;
		float fVar28;
		float fVar29;
		float fVar30;
		float fVar31;
		float fVar32;
		float fVar33;
		float extraout_vf9w;
		float extraout_vf9w_00;
		Vector local_270;
		Vector local_260;
		Vector local_250;
		Vector local_240;
		float local_230;
		float local_22c;
		float local_228;
		float local_224;
		undefined auStack544[32];
		float afStack512[4];
		float local_1f0[4];
		int* local_1e0;
		int local_1dc;
		undefined2 local_1d8[2];
		float local_1d4[117];
		MeshData_CDQU* pcVar3;

		pcVar3 = (MeshData_CDQU*)param_1->pCDQU;
		puVar18 = pcVar3->field_0x10;
		if (param_1->field_0x30 == 1.0) {
			uVar20 = 4;
		}
		else {
			uVar20 = 0xc;
		}
		uVar9 = (uint)puVar18[uVar20];
		bVar6 = false;
		if ((uVar9 != 0) && (bVar6 = true, uVar9 != 0)) {
			pcVar4 = (char*)LOAD_SECTION(pcVar3->pMBNK);
			pcVar13 = (RenderInput_40*)LOAD_SECTION(pcVar3->field_0x48);
			(ScratchpadAnimManager_004494ec.pRenderAnimationStatic_0x60)->field_0x8 = (uint)param_1->field_0x28;
			(ScratchpadAnimManager_004494ec.pRenderAnimationStatic_0x60)->pMeshTransformMatrix = g_ScratchpadAddr_00448a58;
			for (; uVar9 != 0; uVar9 = uVar9 - 1) {
				FUN_002974e0(pcVar13, pcVar4 + 0x10);
				pcVar13 = (RenderInput_40*)LOAD_SECTION(pcVar13->pNext);
			}
			bVar6 = true;
		}
		uVar9 = (uint) * (ushort*)((ulong)&pcVar3->field_0x2c + 2);
		if (uVar9 != 0) {
			IMPLEMENTATION_GUARD();
			iVar10 = *(int*)pcVar3[1].field_0x0;
			pcVar4 = (char*)LOAD_SECTION(pcVar3->pMBNK);
			uVar2 = param_1->field_0x28;
			for (; 0 < (int)uVar9; uVar9 = uVar9 - 1) {
				//FUN_00295e00(iVar10, uVar2, pcVar4 + 0x10);
				iVar10 = *(int*)(iVar10 + 0xc);
			}
			bVar6 = true;
		}
		uVar9 = (uint) * (ushort*)((ulong)&pcVar3->field_0x28 + 2);
		if (uVar9 != 0) {
			IMPLEMENTATION_GUARD();
			puVar17 = pcVar3[1].field_0x10 + 2;
			while (bVar1 = uVar9 != 0, uVar9 = uVar9 - 1, bVar1) {
				//FUN_002b1620((long)(*(int*)(puVar17 + 4) + 0x10));
				puVar17 = puVar17 + 8;
			}
		}
		if (!bVar6) {
			IMPLEMENTATION_GUARD();
			uVar2 = *(ushort*)((ulong)&pcVar3->field_0x28 + 2);
			uVar9 = 0;
			uVar15 = 0;
			iVar10 = 0;
			uVar19 = 1;
			if (uVar2 != 0) {
				iVar10 = (uint)uVar2 * 0x10 + 0x10;
			}
			piVar8 = (int*)VerifyFileBufferRead((char*)((int)puVar18 + iVar10 + 0x40), param_1->pCDQU_End);
			while (true) {
				uVar14 = (uint)uVar15;
				if (piVar8 == (int*)0x0) break;
				local_1d4[uVar14 * 0x10 + 1] = param_1->field_0x30;
				if ((*piVar8 == 0x55514443) || (*piVar8 == 0x434f4443)) {
					if (param_1->field_0x28 == 2) {
						if (local_1d4[uVar14 * 0x10 + 1] == 1.0) {
							//FUN_002950d0((float*)(auStack544 + uVar14 * 0x40 + 0x20), (float*)param_1, uVar14);
							fVar21 = extraout_vf9w;
						}
						else {
							//FUN_002951b0((float*)(auStack544 + uVar14 * 0x40 + 0x20), (float*)param_1, uVar15);
							fVar21 = extraout_vf9w_00;
						}
						local_230 = *(float*)(auStack544 + uVar14 * 0x40 + 0x30);
						local_22c = local_1f0[uVar14 * 0x10 + 1];
						local_228 = local_1f0[uVar14 * 0x10 + 2];
						local_224 = param_1->field_0x2c * 0.5;
						(ScratchpadAnimManager_004494ec.pRenderAnimationStatic_0x60)->pMeshTransformMatrix = g_ScratchpadAddr_00448a58
							;
						pVVar7 = ScratchpadAnimManager_004494ec.vector_0x64;
						(ScratchpadAnimManager_004494ec.vector_0x64)->x = local_230;
						pVVar7->y = local_22c;
						pVVar7->z = local_228;
						(ScratchpadAnimManager_004494ec.vector_0x64)->w = 1.0;
						pVVar7 = ScratchpadAnimManager_004494ec.vector_0x64;
						pMVar5 = (ScratchpadAnimManager_004494ec.pRenderAnimationStatic_0x60)->pMeshTransformMatrix;
						fVar30 = (ScratchpadAnimManager_004494ec.vector_0x64)->x;
						fVar31 = (ScratchpadAnimManager_004494ec.vector_0x64)->y;
						fVar32 = (ScratchpadAnimManager_004494ec.vector_0x64)->z;
						fVar33 = (ScratchpadAnimManager_004494ec.vector_0x64)->w;
						fVar22 = pMVar5->ab;
						fVar23 = pMVar5->ac;
						fVar24 = pMVar5->bb;
						fVar25 = pMVar5->bc;
						fVar26 = pMVar5->cb;
						fVar27 = pMVar5->cc;
						fVar28 = pMVar5->db;
						fVar29 = pMVar5->dc;
						(ScratchpadAnimManager_004494ec.vector_0x64)->x =
							pMVar5->aa * fVar30 + pMVar5->ba * fVar31 + pMVar5->ca * fVar32 + pMVar5->da * fVar33;
						pVVar7->y = fVar22 * fVar30 + fVar24 * fVar31 + fVar26 * fVar32 + fVar28 * fVar33;
						pVVar7->z = fVar23 * fVar30 + fVar25 * fVar31 + fVar27 * fVar32 + fVar29 * fVar33;
						pVVar7->w = fVar21;
						iVar10 = 0; // FUN_0029a730((int)&local_230);
						if (iVar10 == 4) {
							local_1d8[uVar14 * 0x20] = 4;
						}
						else {
							(&local_1e0)[uVar14 * 0x10] = piVar8;
							(&local_1dc)[uVar14 * 0x10] = (int)piVar8 + piVar8[2];
							local_1d8[uVar14 * 0x20] = (short)iVar10;
							local_1d4[uVar14 * 0x10] = local_224;
							local_240.x = local_230;
							local_240.y = local_22c;
							local_240.z = local_228;
							local_240.w = 1.0;
							ps2_vu0_sub_vector(&local_250, &local_240, (Vector*)ScratchpadAnimManager_004494ec.pData_130);
							ps2_vu0_sqr_vector(&local_250, &local_250);
							uVar9 = uVar9 | uVar19;
							local_250.x = local_250.x + local_250.y + local_250.z;
							local_1d4[uVar14 * 0x10 + 2] = local_250.x;
						}
					}
					else {
						if (local_1d4[uVar14 * 0x10 + 1] == 1.0) {
							//FUN_002950d0((float*)(auStack544 + uVar14 * 0x40 + 0x20), (float*)param_1, uVar14);
						}
						else {
							//FUN_002951b0((float*)(auStack544 + uVar14 * 0x40 + 0x20), (float*)param_1, uVar15);
						}
						local_260.x = *(float*)(auStack544 + uVar14 * 0x40 + 0x30);
						local_260.y = local_1f0[uVar14 * 0x10 + 1];
						local_260.z = local_1f0[uVar14 * 0x10 + 2];
						(&local_1e0)[uVar14 * 0x10] = piVar8;
						(&local_1dc)[uVar14 * 0x10] = (int)piVar8 + piVar8[2];
						local_1d8[uVar14 * 0x20] = 1;
						local_260.w = 1.0;
						local_230 = local_260.x;
						local_22c = local_260.y;
						local_228 = local_260.z;
						ps2_vu0_sub_vector(&local_270, &local_260, (Vector*)ScratchpadAnimManager_004494ec.pData_130);
						ps2_vu0_sqr_vector(&local_270, &local_270);
						uVar9 = uVar9 | uVar19;
						local_270.x = local_270.x + local_270.y + local_270.z;
						local_1d4[uVar14 * 0x10 + 2] = local_270.x;
					}
					piVar12 = (int*)param_1->pCDQU_End;
				}
				else {
					piVar12 = (int*)param_1->pCDQU_End;
				}
				uVar19 = uVar19 << 1;
				uVar15 = (int)(uVar14 + 1);
				piVar8 = GetNextFileSection((char*)piVar8, piVar12);
			}
			if (uVar15 != 0) {
				//FUN_00295030((AnimFuncInputData_34*)auStack544, (int)(auStack544 + 0x20), uVar14);
				lVar16 = 0;
				while (true) {
					if ((long)uVar15 <= lVar16) break;
					if ((*(AnimFuncInputData_34**)(auStack544 + (int)lVar16 * 4))->field_0x28 != 4) {
						AnimFunc_00294af0(*(AnimFuncInputData_34**)(auStack544 + (int)lVar16 * 4));
					}
					lVar16 = (long)((int)lVar16 + 1);
				}
			}
			fVar21 = param_1->field_0x30;
			if (fVar21 < 2.147484e+09) {
				cVar11 = (char)(int)fVar21;
			}
			else {
				cVar11 = (char)(int)(fVar21 - 2.147484e+09);
			}
			uVar9 = 0; // FUN_00295320(uVar9, cVar11);
			if ((long)(int)uVar9 != 0) {
				//FUN_002953d0((short*)puVar18, (long)(int)uVar9, (uint)param_1->field_0x28, uVar20);
			}
		}
		return;
	}

	bool FUN_002948b0(MeshInfo* pMeshInfo)
	{
		bool bVar1;
		Vector local_a0;
		Vector local_90;
		AnimFuncInputData_34 local_80;
		AnimFuncInputData_34 local_40;
		MeshData_CSTA* pcVar1;

		pcVar1 = (MeshData_CSTA*)pMeshInfo->CSTA;
		if (pcVar1->field_0x10 == 0x414f4443) {
			local_90.x = pcVar1->field_0x20;
			local_90.y = pcVar1->field_0x24;
			local_90.z = pcVar1->field_0x28;
			local_40.field_0xc = 0.0;
			local_40.field_0x10 = (pcVar1->field_0x30).x;
			local_40.field_0x14 = (pcVar1->field_0x30).y;
			local_40.field_0x18 = (pcVar1->field_0x30).z;
			local_40.field_0x1c = 1.0;
			local_90.w = 0.0;
			local_40.field_0x0 = local_90.x;
			local_40.field_0x4 = local_90.y;
			local_40.field_0x8 = local_90.z;
			ps2_vu0_sqr_vector(&local_90, &local_90);
			local_40.field_0x28 = 2;
			local_40.field_0x30 = 0.0;
			local_90.x = local_90.x + local_90.y + local_90.z;
			local_40.field_0x2c = sqrtf(local_90.x) * 0.5;
			local_40.pCDQU = VerifyFileBufferRead((char*)(pcVar1 + 1), (char*)0x0);
			local_40.pCDQU_End = local_40.pCDQU + *(int*)(local_40.pCDQU + 8);
			AnimFunc_00294af0(&local_40);
			bVar1 = true;
		}
		else {
			if (pcVar1->field_0x10 == 0x41514443) {
				local_a0.x = pcVar1->field_0x20;
				local_a0.y = pcVar1->field_0x24;
				local_a0.z = pcVar1->field_0x28;
				local_80.field_0xc = 0.0;
				local_80.field_0x10 = (pcVar1->field_0x30).x;
				local_80.field_0x14 = (pcVar1->field_0x30).y;
				local_80.field_0x18 = (pcVar1->field_0x30).z;
				local_80.field_0x1c = 1.0;
				local_a0.w = 0.0;
				local_80.field_0x0 = local_a0.x;
				local_80.field_0x4 = local_a0.y;
				local_80.field_0x8 = local_a0.z;
				ps2_vu0_sqr_vector(&local_a0, &local_a0);
				local_80.field_0x30 = 1.0;
				local_80.field_0x28 = 2;
				local_a0.x = local_a0.x + local_a0.y + local_a0.z;
				local_80.field_0x2c = sqrtf(local_a0.x) * 0.5;
				local_80.pCDQU = VerifyFileBufferRead((char*)(pcVar1 + 1), (char*)0x0);
				local_80.pCDQU_End = local_80.pCDQU + *(int*)(local_80.pCDQU + 8);
				AnimFunc_00294af0(&local_80);
				bVar1 = true;
			}
			else {
				bVar1 = false;
			}
		}
		return bVar1;
	}

	Matrix Matrix_00424f50 = {
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 0.0f
	};

	float MatrixFunc_002958a0(Matrix* param_1)
	{
		float fVar1;
		float fVar2;
		float fVar3;
		int iVar4;
		Vector* pVVar5;
		Matrix* pMVar6;
		Vector* v0;
		Vector local_30[3];

		pMVar6 = &Matrix_00424f50;
		iVar4 = 3;
		pVVar5 = local_30;
		do {
			fVar1 = pMVar6->ab;
			fVar2 = pMVar6->ac;
			fVar3 = pMVar6->ad;
			iVar4 = iVar4 + -1;
			pVVar5->x = pMVar6->aa;
			pVVar5->y = fVar1;
			pVVar5->z = fVar2;
			pVVar5->w = fVar3;
			pMVar6 = (Matrix*)&pMVar6->ba;
			pVVar5 = pVVar5 + 1;
		} while (0 < iVar4);
		sceVu0ApplyMatrix(local_30, param_1, local_30);
		v0 = local_30 + 1;
		sceVu0ApplyMatrix(v0, param_1, v0);
		pVVar5 = local_30 + 2;
		sceVu0ApplyMatrix(pVVar5, param_1, pVVar5);
		ps2_vu0_sqr_vector(local_30, local_30);
		ps2_vu0_sqr_vector(v0, v0);
		ps2_vu0_sqr_vector(pVVar5, pVVar5);
		fVar2 = local_30[0].z + local_30[0].x + local_30[0].y;
		fVar3 = local_30[1].z + local_30[1].x + local_30[1].y;
		fVar1 = local_30[2].z + local_30[2].x + local_30[2].y;
		if (fVar2 < fVar3) {
			if (fVar3 < fVar1) {
				fVar2 = sqrt(fVar1);
			}
			else {
				fVar2 = sqrt(fVar3);
			}
		}
		else {
			if (fVar2 < fVar1) {
				fVar2 = sqrt(fVar1);
			}
			else {
				fVar2 = sqrt(fVar2);
			}
		}
		return fVar2;
	}

	void SetField_00295df0(MeshTransformData* pMeshTransformData, byte param_2)
	{
		(pMeshTransformData->base).field_0x88 = param_2;
		return;
	}

	MeshTransformObjData* GetMeshOBJ_00295a80(MeshTransformData* pMeshTransformData)
	{
		ushort uVar1;
		LightingMatrixFuncObj* pLVar2;
		char* pcVar3;
		MeshTransformObjData* pMVar4;
		uint uVar5;
		uint uVar6;
		float fVar7;
		float afStack64[12];
		float local_10;
		float local_c;
		float local_8;

		uVar1 = (pMeshTransformData->base).count_0x9c;
		if (uVar1 < 2) {
			(pMeshTransformData->base).size_0xae = (char)uVar1 - 1;
			if ((pMeshTransformData->base).size_0xae == 0xffffffff) {
				pMVar4 = (MeshTransformObjData*)0x0;
			}
			else {
				pMVar4 = pMeshTransformData->aSubArray;
			}
		}
		else {
			uVar5 = (uint)(pMeshTransformData->base).size_0xae;
			if (uVar5 == 0xff) {
				IMPLEMENTATION_GUARD(
					sceVu0MulMatrix((TO_SCE_MTX)afStack64, (TO_SCE_MTX) & (pMeshTransformData->base).transformB,
						(TO_SCE_MTX)&Matrix_004894e0);
				pLVar2 = (pMeshTransformData->base).pLightingMatrixFuncObj_0xa0;
				fVar7 = FLOAT_00449214 * (local_8 * local_8 + local_10 * local_10 + local_c * local_c);
				if ((pLVar2 == (LightingMatrixFuncObj*)0x0) || (pcVar3 = pLVar2->field_0xc, pcVar3 == (char*)0x0)) {
					uVar5 = (uint)(pMeshTransformData->base).count_0x9c;
					pMVar4 = pMeshTransformData->aSubArray;
					uVar6 = 0;
					while ((uVar6 < uVar5 && ((float)((int)pMVar4->field_0x6 * (int)pMVar4->field_0x6) <= fVar7))) {
						pMVar4 = pMVar4 + 1;
						uVar6 = uVar6 + 1 & 0xffff;
					}
					if (uVar6 == uVar5) {
						pMVar4 = (MeshTransformObjData*)0x0;
					}
				}
				else {
					uVar5 = (uint)(pMeshTransformData->base).count_0x9c;
					for (uVar6 = 0; (uVar6 < uVar5 && (*(float*)(pcVar3 + uVar6 * 4) <= fVar7)); uVar6 = uVar6 + 1 & 0xffff) {
					}
					if (uVar6 == uVar5) {
						(pMeshTransformData->base).size_0xae = (byte)uVar6;
						return (MeshTransformObjData*)0x0;
					}
					pMVar4 = pMeshTransformData->aSubArray + uVar6;
				}
				(pMeshTransformData->base).size_0xae = (byte)uVar6;)
			}
			else {
				pMVar4 = pMeshTransformData->aSubArray + uVar5;
			}
		}
		return pMVar4;
	}

	PACK( struct MeshHeader {
		Hash_8 header;
		int pOBJ;
	});

	PACK (struct MeshData_OBJ {
		char field_0x0[4];
		undefined field_0x4;
		undefined field_0x5;
		undefined field_0x6;
		undefined field_0x7;
		undefined field_0x8;
		undefined field_0x9;
		undefined4 field_0xa;
		undefined field_0xe;
		undefined field_0xf;
		undefined field_0x10;
		undefined field_0x11;
		undefined field_0x12;
		undefined field_0x13;
		int count_0x14;
		struct Vector field_0x18;
		undefined field_0x28;
		undefined field_0x29;
		undefined field_0x2a;
		undefined field_0x2b;
		int field_0x2c; //struct RenderInput_40*
	});

	uint RenderInputVectorFunc_00299a10(Vector* param_1)
	{
		float* pfVar1;
		uint uVar2;
		float fVar3;
		float fVar4;
		float fVar5;

		uVar2 = 1;
		fVar4 = param_1->w * (ScratchpadAnimManager_004494ec.pRenderAnimationStatic_0x60)->field_0x14;
		if ((((ScratchpadAnimManager_004494ec.pRenderAnimationStatic_0x60)->flags_0x10 & 0x10) == 0) ||
			(pfVar1 = (float*)(ScratchpadAnimManager_004494ec.pRenderAnimationStatic_0x60)->pLightingMatrixFuncObj->field_0x0,
				pfVar1 == (float*)0x0)) {
			fVar3 = ((ScratchpadAnimManager_004494ec.pData_150)->field_0x20).field_0x50.w;
		}
		else {
			fVar3 = -*pfVar1;
		}
		fVar5 = (ScratchpadAnimManager_004494ec.vector_0x64)->z;
		if (fVar5 < fVar3) {
			if (fVar4 <= -(fVar5 - fVar3)) {
				return 4;
			}
			uVar2 = 2;
		}
		else {
			if (-(fVar3 - fVar5) < fVar4) {
				uVar2 = 2;
			}
		}
		fVar3 = ((ScratchpadAnimManager_004494ec.pData_150)->field_0x20).field_0x40.w;
		if (fVar5 < fVar3) {
			if (-(fVar5 - fVar3) < fVar4) {
				uVar2 = 2;
			}
		}
		else {
			if (fVar4 <= -(fVar3 - fVar5)) {
				return 4;
			}
			uVar2 = 2;
		}
		fVar3 = (ScratchpadAnimManager_004494ec.vector_0x64)->x;
		if (fVar3 <= 0.0) {
			fVar3 = ((ScratchpadAnimManager_004494ec.pData_150)->field_0x20).field_0x0.ba * -fVar3 +
				((ScratchpadAnimManager_004494ec.pData_150)->field_0x20).field_0x0.bc * -fVar5;
			if (0.0 < fVar3) {
				if (fVar3 <= fVar4) {
					uVar2 = 2;
				}
			}
			else {
				if (fVar4 < -fVar3) {
					return 4;
				}
				uVar2 = 2;
			}
		}
		else {
			fVar3 = ((ScratchpadAnimManager_004494ec.pData_150)->field_0x20).field_0x0.aa * -fVar3 +
				((ScratchpadAnimManager_004494ec.pData_150)->field_0x20).field_0x0.ac * -fVar5;
			if (0.0 < fVar3) {
				if (fVar3 <= fVar4) {
					uVar2 = 2;
				}
			}
			else {
				if (fVar4 < -fVar3) {
					return 4;
				}
				uVar2 = 2;
			}
		}
		fVar3 = (ScratchpadAnimManager_004494ec.vector_0x64)->y;
		if (fVar3 <= 0.0) {
			fVar3 = ((ScratchpadAnimManager_004494ec.pData_150)->field_0x20).field_0x0.cb * fVar3 +
				((ScratchpadAnimManager_004494ec.pData_150)->field_0x20).field_0x0.cc * -fVar5;
			if (0.0 < fVar3) {
				if (fVar3 <= fVar4) {
					uVar2 = 2;
				}
			}
			else {
				uVar2 = 4;
				if (-fVar3 <= fVar4) {
					uVar2 = 2;
				}
			}
		}
		else {
			fVar3 = ((ScratchpadAnimManager_004494ec.pData_150)->field_0x20).field_0x0.db * fVar3 +
				((ScratchpadAnimManager_004494ec.pData_150)->field_0x20).field_0x0.dc * -fVar5;
			if (0.0 < fVar3) {
				if (fVar3 <= fVar4) {
					uVar2 = 2;
				}
			}
			else {
				uVar2 = 4;
				if (-fVar3 <= fVar4) {
					uVar2 = 2;
				}
			}
		}
		return uVar2;
	}


	void ReadRenderTransform_002b0d00(MeshHeader* pMeshHeader, undefined* pTextureInfo)
	{
		bool bVar1;
		Matrix* pMVar2;
		Vector* pVVar3;
		uint uVar4;
		int iVar5;
		Vector* pVVar6;
		int iVar7;
		int meshCount;
		RenderInput_40* pRenderInput;
		float fVar8;
		float fVar9;
		float fVar10;
		float fVar11;
		float fVar12;
		float fVar13;
		float fVar14;
		float fVar15;
		float fVar16;
		float fVar17;
		float fVar18;
		float fVar19;
		float in_vf9w;
		MeshData_OBJ* pMeshOBJ;

		pVVar3 = ScratchpadAnimManager_004494ec.vector_0x64;
		pMeshOBJ = (MeshData_OBJ*)LOAD_SECTION(pMeshHeader->pOBJ);
		pRenderInput = (RenderInput_40*)LOAD_SECTION(pMeshOBJ->field_0x2c);
		if (pRenderInput == (RenderInput_40*)0x0) {
			return;
		}
		meshCount = pMeshOBJ->count_0x14;
		if (((ScratchpadAnimManager_004494ec.pRenderAnimationStatic_0x60)->flags_0x10 & 0x10) != 0) {
			pVVar6 = (ScratchpadAnimManager_004494ec.pRenderAnimationStatic_0x60)->pLightingMatrixFuncObj->pVector_0x4;
			iVar7 = 8;
			if (pVVar6 != (Vector*)0x0) goto LAB_002b0d68;
		}
		pVVar6 = &pMeshOBJ->field_0x18;
		iVar7 = 2;
	LAB_002b0d68:
		fVar8 = pVVar6->y;
		fVar9 = pVVar6->z;
		(ScratchpadAnimManager_004494ec.vector_0x64)->x = pVVar6->x;
		pVVar3->y = fVar8;
		pVVar3->z = fVar9;
		(ScratchpadAnimManager_004494ec.vector_0x64)->w = 1.0;
		pVVar3 = ScratchpadAnimManager_004494ec.vector_0x64;
		pMVar2 = (ScratchpadAnimManager_004494ec.pRenderAnimationStatic_0x60)->pMeshTransformMatrix;
		fVar16 = (ScratchpadAnimManager_004494ec.vector_0x64)->x;
		fVar17 = (ScratchpadAnimManager_004494ec.vector_0x64)->y;
		fVar18 = (ScratchpadAnimManager_004494ec.vector_0x64)->z;
		fVar19 = (ScratchpadAnimManager_004494ec.vector_0x64)->w;
		fVar8 = pMVar2->ab;
		fVar9 = pMVar2->ac;
		fVar10 = pMVar2->bb;
		fVar11 = pMVar2->bc;
		fVar12 = pMVar2->cb;
		fVar13 = pMVar2->cc;
		fVar14 = pMVar2->db;
		fVar15 = pMVar2->dc;
		(ScratchpadAnimManager_004494ec.vector_0x64)->x =
			pMVar2->aa * fVar16 + pMVar2->ba * fVar17 + pMVar2->ca * fVar18 + pMVar2->da * fVar19;
		pVVar3->y = fVar8 * fVar16 + fVar10 * fVar17 + fVar12 * fVar18 + fVar14 * fVar19;
		pVVar3->z = fVar9 * fVar16 + fVar11 * fVar17 + fVar13 * fVar18 + fVar15 * fVar19;
		pVVar3->w = in_vf9w;
		uVar4 = RenderInputVectorFunc_00299a10(pVVar6);
		if (uVar4 != 4) {
			if (uVar4 == 1) {
				(ScratchpadAnimManager_004494ec.pRenderAnimationStatic_0x60)->field_0x8 = 1;
			}
			else {
				IMPLEMENTATION_GUARD(
				iVar5 = FUN_00299810(pVVar6);
				if (iVar5 != 1) {
					(ScratchpadAnimManager_004494ec.pRenderAnimationStatic_0x60)->field_0x8 = iVar7;
				})
			}
			if (*ScratchpadAnimManager_004494ec.field_0x28 == 0) {
				IMPLEMENTATION_GUARD(
				while (bVar1 = meshCount != 0, meshCount = meshCount + -1, bVar1) {
					RenderInput_40::SetupMesh_00297c00(pRenderInput, pTextureInfo);
					pRenderInput = pRenderInput->pNext;
				})
			}
			else {
				IMPLEMENTATION_GUARD(
				while (bVar1 = meshCount != 0, meshCount = meshCount + -1, bVar1) {
					RenderInput_40::SetupShadow_00298130(pRenderInput, pTextureInfo);
					pRenderInput = pRenderInput->pNext;
				})
			}
		}
		return;
	}

	void GenerateTransform_002b48f0(MeshTransformData* pMeshTransformData)
	{
		short sVar1;
		MeshHeader* pMeshHeader;
		MeshTransformObjData* pMVar2;
		Matrix* pMVar3;
		float fVar4;
		Matrix MStack64;

		if ((((pMeshTransformData->base).flags_0x9e & 0x41) == 0) && ((pMeshTransformData->base).count_0x9c != 0)) {
			pMVar2 = GetMeshOBJ_00295a80(pMeshTransformData);
			if (pMVar2 != (MeshTransformObjData*)0x0) {
				if (((pMeshTransformData->base).flags_0x9e & 0x80) == 0) {
					(pMeshTransformData->base).size_0xae = 0xff;
				}
				sceVu0MulMatrix((TO_SCE_MTX)&MStack64, (TO_SCE_MTX) & (pMeshTransformData->base).transformB,
					(TO_SCE_MTX)g_ScratchpadAddr_00448a58);
				pMVar3 = &(pMeshTransformData->base).transformB;
				(ScratchpadAnimManager_004494ec.pRenderAnimationStatic_0x60)->pMeshTransformMatrix = &MStack64;
				(ScratchpadAnimManager_004494ec.pRenderAnimationStatic_0x60)->pMeshTransformData = pMeshTransformData;
				(ScratchpadAnimManager_004494ec.pRenderAnimationStatic_0x60)->pLightingMatrixFuncObj =
					(pMeshTransformData->base).pLightingMatrixFuncObj_0xa0;
				(ScratchpadAnimManager_004494ec.pRenderAnimationStatic_0x60)->pSharedMeshTransform = pMVar3;
				(ScratchpadAnimManager_004494ec.pRenderAnimationStatic_0x60)->flags_0x10 =
					(uint)(pMeshTransformData->base).flags_0x9e;
				fVar4 = MatrixFunc_002958a0(pMVar3);
				(ScratchpadAnimManager_004494ec.pRenderAnimationStatic_0x60)->field_0x14 = fVar4;
				(ScratchpadAnimManager_004494ec.pRenderAnimationStatic_0x60)->field_0x18 = 0;
				(ScratchpadAnimManager_004494ec.pRenderAnimationStatic_0x60)->field_0x8 = 1;
				(pMeshTransformData->base).field_0xa4 = (Matrix*)0x0;
				pMeshHeader = (MeshHeader*)pMVar2->pObj;
				if (((pMeshHeader != (MeshHeader*)0x0) && (sVar1 = pMVar2->field_0x4, sVar1 != 2)) && (sVar1 != 1)) {
					if (sVar1 == 3) {
						IMPLEMENTATION_GUARD(ReadRenderTransform_002b0e80(pMeshHeader, (pMeshTransformData->base).pTextureInfo + 0x10));
					}
					else {
						if (sVar1 == 0) {
							ReadRenderTransform_002b0d00(pMeshHeader, (pMeshTransformData->base).pTextureInfo + 0x10);
						}
					}
				}
			}
			SetField_00295df0(pMeshTransformData, 1);
		}
		return;
	}

	void GenerateShadow_002b4b80(MeshTransformData* pMeshTransformData)
	{
		char cVar1;
		ushort uVar2;
		short sVar3;
		MeshTransformObjData* pMVar4;
		MeshHeader** ppMVar5;
		Matrix* pMVar6;
		float fVar7;
		Matrix MStack64;

		uVar2 = (pMeshTransformData->base).flags_0x9e;
		if (((((uVar2 & 0x200) != 0) &&
			((*ScratchpadAnimManager_004494ec.field_0x28 & (uint)(pMeshTransformData->base).bRenderShadow) != 0)) &&
			((uVar2 & 0x41) == 0)) && ((pMeshTransformData->base).count_0x9c != 0)) {
			pMVar4 = GetMeshOBJ_00295a80(pMeshTransformData);
			if (((pMeshTransformData->base).flags_0x9e & 0x80) == 0) {
				(pMeshTransformData->base).size_0xae = 0xff;
			}
			if ((pMVar4 != (MeshTransformObjData*)0x0) && (pMVar4->pObj != (char*)0x0)) {
				sceVu0MulMatrix((TO_SCE_MTX)&MStack64, (TO_SCE_MTX) & (pMeshTransformData->base).transformB,
					(TO_SCE_MTX)g_ScratchpadAddr_00448a58);
				pMVar6 = &(pMeshTransformData->base).transformB;
				(ScratchpadAnimManager_004494ec.pRenderAnimationStatic_0x60)->pMeshTransformMatrix = &MStack64;
				(ScratchpadAnimManager_004494ec.pRenderAnimationStatic_0x60)->pMeshTransformData = pMeshTransformData;
				(ScratchpadAnimManager_004494ec.pRenderAnimationStatic_0x60)->pLightingMatrixFuncObj =
					(pMeshTransformData->base).pLightingMatrixFuncObj_0xa0;
				(ScratchpadAnimManager_004494ec.pRenderAnimationStatic_0x60)->pSharedMeshTransform = pMVar6;
				(ScratchpadAnimManager_004494ec.pRenderAnimationStatic_0x60)->flags_0x10 =
					(uint)(pMeshTransformData->base).flags_0x9e;
				fVar7 = MatrixFunc_002958a0(pMVar6);
				(ScratchpadAnimManager_004494ec.pRenderAnimationStatic_0x60)->field_0x14 = fVar7;
				(ScratchpadAnimManager_004494ec.pRenderAnimationStatic_0x60)->field_0x18 = 0;
				(pMeshTransformData->base).field_0xa4 = (Matrix*)0x0;
				sVar3 = pMVar4->field_0x4;
				if (sVar3 == 3) {
					IMPLEMENTATION_GUARD(
					MeshHeader::ReadRenderTransform_002b0e80
					((MeshHeader*)pMVar4->pObj, (pMeshTransformData->base).pTextureInfo + 0x10);)
				}
				else {
					if (((sVar3 != 2) && (sVar3 != 1)) && (sVar3 == 0)) {
						if (((pMeshTransformData->base).flags_0x9e & 0x100) == 0) {
							ReadRenderTransform_002b0d00
							((MeshHeader*)pMVar4->pObj, (pMeshTransformData->base).pTextureInfo + 0x10);
						}
						else {
							IMPLEMENTATION_GUARD(
							cVar1 = (pMeshTransformData->base).field_0xaf;
							(pMeshTransformData->base).field_0xaf = -1;
							ppMVar5 = (MeshHeader**)
								Func_00295a50(pMeshTransformData, (long)(int)((pMeshTransformData->base).count_0x9c - 1));
							if (ppMVar5 != (MeshHeader**)0x0) {
								ReadRenderTransform_002b0d00(*ppMVar5, (pMeshTransformData->base).pTextureInfo + 0x10);
							}
							(pMeshTransformData->base).field_0xaf = cVar1;)
						}
					}
				}
			}
		}
		return;
	}

	uint CallMeshRenderFunc_002b2b70(StaticMeshMaster* pInStaticMeshMaster, StaticMeshMaster* pStaticMeshMaster)
	{
		MeshInfo* pMeshInfo;
		float fVar1;
		CameraObj_28* pCVar2;
		int iVar3;
		bool bVar4;
		uint uVar5;
		uint uVar6;
		//ProfileObject* pPVar7;
		MeshTransformParent* pCVar10;
		MeshTransformParent* pCVar9;
		RenderTaskData renderTaskData;
		MeshTransformParent* pCVar8;

		RENDER_LOG("CallMeshRenderFunc_002b2b70 %p (%d) (child: %d) | %p (%d) (child: %d)\n", pInStaticMeshMaster, GetStaticMeshMasterIndex(pInStaticMeshMaster), pInStaticMeshMaster->isChild, pStaticMeshMaster, GetStaticMeshMasterIndex(pStaticMeshMaster), pStaticMeshMaster->isChild);

		iVar3 = g_3DFlushProfileObjIndex_004491ec;
		if ((pInStaticMeshMaster->isChild == 1) && ((pStaticMeshMaster->flags_0x4 & 0x10) != 0)) {
			RENDER_LOG("CallMeshRenderFunc_002b2b70 SKIP\n");
			uVar6 = 0;
		}
		else {
			RENDER_LOG("CallMeshRenderFunc_002b2b70 DRAW\n");
			if (ed3D::Params_00424e30.bEnableProfile != 0) {
				IMPLEMENTATION_GUARD(
				if ((g_3DFlushProfileObjIndex_004491ec & 1U) == 0) {
					uVar6 = (uint)g_3DFlushProfileObjIndex_004491ec >> 4;
					if (g_ProfileObjB_0041ed44[uVar6].field_0x34 != 0) {
						g_ProfileObjB_0041ed44[uVar6].field_0x8 = 0;
						g_ProfileObjB_0041ed44[uVar6].fileCount = 0;
						g_ProfileObjB_0041ed44[uVar6].field_0x34 = 0;
					}
					g_ProfileObjB_0041ed44[(uint)iVar3 >> 4].field_0x0 = (long)PCR1 & 0xffffffff;
					g_ProfileObjB_0041ed44[(uint)iVar3 >> 4].fileCount = g_ProfileObjB_0041ed44[(uint)iVar3 >> 4].fileCount + 1;
				}
				else {
					uVar6 = (uint)g_3DFlushProfileObjIndex_004491ec >> 4;
					if (g_ProfileObjA_0041ed40[uVar6].field_0x34 != 0) {
						g_ProfileObjA_0041ed40[uVar6].field_0x8 = 0;
						g_ProfileObjA_0041ed40[uVar6].fileCount = 0;
						g_ProfileObjA_0041ed40[uVar6].field_0x34 = 0;
					}
					g_ProfileObjA_0041ed40[(uint)iVar3 >> 4].field_0x0 = (long)PCR1 & 0xffffffff;
					g_ProfileObjA_0041ed40[(uint)iVar3 >> 4].fileCount = g_ProfileObjA_0041ed40[(uint)iVar3 >> 4].fileCount + 1;
				})
			}
			FUN_0029c030();
			uVar6 = SetupCameraObj130_0029a960(pInStaticMeshMaster, (ulong)((pInStaticMeshMaster->flags_0x4 & 0x20) != 0));
			uVar6 = uVar6 & 0xff;
			if (ed3D::Params_00424e30.bEnableProfile != 0) {
				IMPLEMENTATION_GUARD(
				uVar5 = (uint)g_3DFlushProfileObjIndex_004491ec >> 4;
				if ((g_3DFlushProfileObjIndex_004491ec & 1U) == 0) {
					g_ProfileObjB_0041ed44[uVar5].field_0x0 = (long)PCR1 - g_ProfileObjB_0041ed44[uVar5].field_0x0;
					g_ProfileObjB_0041ed44[uVar5].field_0x8 =
						g_ProfileObjB_0041ed44[uVar5].field_0x8 + g_ProfileObjB_0041ed44[uVar5].field_0x0;
					pPVar7 = g_ProfileObjB_0041ed44 + uVar5;
					if (pPVar7->field_0x10 < g_ProfileObjB_0041ed44[uVar5].field_0x0) {
						pPVar7->field_0x10 = pPVar7->field_0x0;
					}
				}
				else {
					g_ProfileObjA_0041ed40[uVar5].field_0x0 = (long)PCR1 - g_ProfileObjA_0041ed40[uVar5].field_0x0;
					g_ProfileObjA_0041ed40[uVar5].field_0x8 =
						g_ProfileObjA_0041ed40[uVar5].field_0x8 + g_ProfileObjA_0041ed40[uVar5].field_0x0;
					pPVar7 = g_ProfileObjA_0041ed40 + uVar5;
					if (pPVar7->field_0x10 < g_ProfileObjA_0041ed40[uVar5].field_0x0) {
						pPVar7->field_0x10 = pPVar7->field_0x0;
					}
				})
			}
			iVar3 = g_3DFlushProfileObjIndex_004491ec;
			if (uVar6 != 0) {
				g_RenderPassCount_0044939c = g_RenderPassCount_0044939c + 1;
				if (ed3D::Params_00424e30.bEnableProfile != 0) {
					IMPLEMENTATION_GUARD(
					if ((g_3DFlushProfileObjIndex_004491ec & 1U) == 0) {
						uVar5 = (uint)g_3DFlushProfileObjIndex_004491ec >> 4;
						if (g_ProfileObjB_0041ed44[uVar5].field_0x34 != 0) {
							g_ProfileObjB_0041ed44[uVar5].field_0x8 = 0;
							g_ProfileObjB_0041ed44[uVar5].fileCount = 0;
							g_ProfileObjB_0041ed44[uVar5].field_0x34 = 0;
						}
						g_ProfileObjB_0041ed44[(uint)iVar3 >> 4].field_0x0 = (long)PCR1 & 0xffffffff;
						g_ProfileObjB_0041ed44[(uint)iVar3 >> 4].fileCount = g_ProfileObjB_0041ed44[(uint)iVar3 >> 4].fileCount + 1;
					}
					else {
						uVar5 = (uint)g_3DFlushProfileObjIndex_004491ec >> 4;
						if (g_ProfileObjA_0041ed40[uVar5].field_0x34 != 0) {
							g_ProfileObjA_0041ed40[uVar5].field_0x8 = 0;
							g_ProfileObjA_0041ed40[uVar5].fileCount = 0;
							g_ProfileObjA_0041ed40[uVar5].field_0x34 = 0;
						}
						g_ProfileObjA_0041ed40[(uint)iVar3 >> 4].field_0x0 = (long)PCR1 & 0xffffffff;
						g_ProfileObjA_0041ed40[(uint)iVar3 >> 4].fileCount = g_ProfileObjA_0041ed40[(uint)iVar3 >> 4].fileCount + 1;
					})
				}
				LinkStaticList_0029c140();
				g_VideoMemoryPtr_00449280 = SetupDrawEnvironment_002b1990(pInStaticMeshMaster, g_VideoMemoryPtr_00449280);
				if (ed3D::Params_00424e30.bEnableProfile != 0) {
					IMPLEMENTATION_GUARD(
					uVar5 = (uint)g_3DFlushProfileObjIndex_004491ec >> 4;
					if ((g_3DFlushProfileObjIndex_004491ec & 1U) == 0) {
						g_ProfileObjB_0041ed44[uVar5].field_0x0 = (long)PCR1 - g_ProfileObjB_0041ed44[uVar5].field_0x0;
						g_ProfileObjB_0041ed44[uVar5].field_0x8 =
							g_ProfileObjB_0041ed44[uVar5].field_0x8 + g_ProfileObjB_0041ed44[uVar5].field_0x0;
						pPVar7 = g_ProfileObjB_0041ed44 + uVar5;
						if (pPVar7->field_0x10 < g_ProfileObjB_0041ed44[uVar5].field_0x0) {
							pPVar7->field_0x10 = pPVar7->field_0x0;
						}
					}
					else {
						g_ProfileObjA_0041ed40[uVar5].field_0x0 = (long)PCR1 - g_ProfileObjA_0041ed40[uVar5].field_0x0;
						g_ProfileObjA_0041ed40[uVar5].field_0x8 =
							g_ProfileObjA_0041ed40[uVar5].field_0x8 + g_ProfileObjA_0041ed40[uVar5].field_0x0;
						pPVar7 = g_ProfileObjA_0041ed40 + uVar5;
						if (pPVar7->field_0x10 < g_ProfileObjA_0041ed40[uVar5].field_0x0) {
							pPVar7->field_0x10 = pPVar7->field_0x0;
						}
					}
					iVar3 = g_3DRendProfileObjIndex_004491e8;
					if ((g_3DRendProfileObjIndex_004491e8 & 1U) == 0) {
						uVar5 = (uint)g_3DRendProfileObjIndex_004491e8 >> 4;
						if (g_ProfileObjB_0041ed44[uVar5].field_0x34 != 0) {
							g_ProfileObjB_0041ed44[uVar5].field_0x8 = 0;
							g_ProfileObjB_0041ed44[uVar5].fileCount = 0;
							g_ProfileObjB_0041ed44[uVar5].field_0x34 = 0;
						}
						g_ProfileObjB_0041ed44[(uint)iVar3 >> 4].field_0x0 = (long)PCR1 & 0xffffffff;
						g_ProfileObjB_0041ed44[(uint)iVar3 >> 4].fileCount = g_ProfileObjB_0041ed44[(uint)iVar3 >> 4].fileCount + 1;
					}
					else {
						uVar5 = (uint)g_3DRendProfileObjIndex_004491e8 >> 4;
						if (g_ProfileObjA_0041ed40[uVar5].field_0x34 != 0) {
							g_ProfileObjA_0041ed40[uVar5].field_0x8 = 0;
							g_ProfileObjA_0041ed40[uVar5].fileCount = 0;
							g_ProfileObjA_0041ed40[uVar5].field_0x34 = 0;
						}
						g_ProfileObjA_0041ed40[(uint)iVar3 >> 4].field_0x0 = (long)PCR1 & 0xffffffff;
						g_ProfileObjA_0041ed40[(uint)iVar3 >> 4].fileCount = g_ProfileObjA_0041ed40[(uint)iVar3 >> 4].fileCount + 1;
					})
				}
				pCVar8 = (MeshTransformParent*)pStaticMeshMaster->pFinalLink_0x1a4;
				pCVar9 = pCVar8->pNext;
				if (*ScratchpadAnimManager_004494ec.field_0x28 == 0) {
					if (ed3D::Params_00424e30.field_0x2d == 0) {
						for (; pCVar9 != pCVar8; pCVar9 = pCVar9->pNext) {
							GenerateTransform_002b48f0((MeshTransformData*)pCVar9->pMeshTransformData);
						}
					}
					else {
						for (; pCVar9 != pCVar8; pCVar9 = pCVar9->pNext) {
							IMPLEMENTATION_GUARD(FUN_002b4a30(pCVar9->pMeshTransformData);)
						}
					}
				}
				else {
					for (; pCVar9 != pCVar8; pCVar9 = pCVar9->pNext) {
						GenerateShadow_002b4b80((MeshTransformData*)pCVar9->pMeshTransformData);
					}
				}
				FUN_002ad7b0(pInStaticMeshMaster);
				if ((pInStaticMeshMaster->isChild != 1) &&
					(ProbablySortDepth_002a52a0(pInStaticMeshMaster), (pInStaticMeshMaster->flags_0x4 & 8) == 0)) {
					for (pCVar10 = (MeshTransformParent*)(pStaticMeshMaster->headerB).pLoadedData;
						(CameraPanMasterHeader*)pCVar10 != &pStaticMeshMaster->headerB; pCVar10 = pCVar10->pNext) {
							if (((pCVar10->pMeshTransformData->flags & 2) == 0) &&
								(pMeshInfo = pCVar10->pMeshTransformData->pMeshInfo, pMeshInfo->CSTA != (char*)0x0)) {
								bVar4 = FUN_002948b0(pMeshInfo);
								uVar6 = (int)bVar4 & 0xff;
						}
					}
				}
				renderTaskData.pStaticMeshMaster = pStaticMeshMaster;
				if (BYTE_00448a5c == 0) {
					if (pInStaticMeshMaster == pStaticMeshMaster) {
						renderTaskData.isChild = pInStaticMeshMaster->isChild;
						renderTaskData.taskID = 2;
						edSysHandlersCall(edDlist::sysHandler_0048cb90.mainIdentifier, edDlist::sysHandler_0048cb90.entries,
							edDlist::sysHandler_0048cb90.maxEventID, 0, &renderTaskData);
					}
					else {
						renderTaskData.isChild = pInStaticMeshMaster->isChild;
						renderTaskData.taskID = 1;
						if (pInStaticMeshMaster != pStaticMeshMaster) {
							renderTaskData.taskID = 3;
						}
						edSysHandlersCall(edDlist::sysHandler_0048cb90.mainIdentifier, edDlist::sysHandler_0048cb90.entries,
							edDlist::sysHandler_0048cb90.maxEventID, 0, &renderTaskData);
					}
				}
				else {
					renderTaskData.isChild = pInStaticMeshMaster->isChild;
					renderTaskData.taskID = 1;
					if (pInStaticMeshMaster != pStaticMeshMaster) {
						renderTaskData.taskID = 3;
					}
					edSysHandlersCall(edDlist::sysHandler_0048cb90.mainIdentifier, edDlist::sysHandler_0048cb90.entries,
						edDlist::sysHandler_0048cb90.maxEventID, 0, &renderTaskData);
				}
				if (ed3D::Params_00424e30.bEnableProfile != 0) {
					IMPLEMENTATION_GUARD(
					uVar5 = (uint)g_3DRendProfileObjIndex_004491e8 >> 4;
					if ((g_3DRendProfileObjIndex_004491e8 & 1U) == 0) {
						g_ProfileObjB_0041ed44[uVar5].field_0x0 = (long)PCR1 - g_ProfileObjB_0041ed44[uVar5].field_0x0;
						g_ProfileObjB_0041ed44[uVar5].field_0x8 =
							g_ProfileObjB_0041ed44[uVar5].field_0x8 + g_ProfileObjB_0041ed44[uVar5].field_0x0;
						pPVar7 = g_ProfileObjB_0041ed44 + uVar5;
						if (pPVar7->field_0x10 < g_ProfileObjB_0041ed44[uVar5].field_0x0) {
							pPVar7->field_0x10 = pPVar7->field_0x0;
						}
					}
					else {
						g_ProfileObjA_0041ed40[uVar5].field_0x0 = (long)PCR1 - g_ProfileObjA_0041ed40[uVar5].field_0x0;
						g_ProfileObjA_0041ed40[uVar5].field_0x8 =
							g_ProfileObjA_0041ed40[uVar5].field_0x8 + g_ProfileObjA_0041ed40[uVar5].field_0x0;
						pPVar7 = g_ProfileObjA_0041ed40 + uVar5;
						if (pPVar7->field_0x10 < g_ProfileObjA_0041ed40[uVar5].field_0x0) {
							pPVar7->field_0x10 = pPVar7->field_0x0;
						}
					}
					iVar3 = g_3DFlushProfileObjIndex_004491ec;
					if ((g_3DFlushProfileObjIndex_004491ec & 1U) == 0) {
						uVar5 = (uint)g_3DFlushProfileObjIndex_004491ec >> 4;
						if (g_ProfileObjB_0041ed44[uVar5].field_0x34 != 0) {
							g_ProfileObjB_0041ed44[uVar5].field_0x8 = 0;
							g_ProfileObjB_0041ed44[uVar5].fileCount = 0;
							g_ProfileObjB_0041ed44[uVar5].field_0x34 = 0;
						}
						g_ProfileObjB_0041ed44[(uint)iVar3 >> 4].field_0x0 = (long)PCR1 & 0xffffffff;
						g_ProfileObjB_0041ed44[(uint)iVar3 >> 4].fileCount = g_ProfileObjB_0041ed44[(uint)iVar3 >> 4].fileCount + 1;
					}
					else {
						uVar5 = (uint)g_3DFlushProfileObjIndex_004491ec >> 4;
						if (g_ProfileObjA_0041ed40[uVar5].field_0x34 != 0) {
							g_ProfileObjA_0041ed40[uVar5].field_0x8 = 0;
							g_ProfileObjA_0041ed40[uVar5].fileCount = 0;
							g_ProfileObjA_0041ed40[uVar5].field_0x34 = 0;
						}
						g_ProfileObjA_0041ed40[(uint)iVar3 >> 4].field_0x0 = (long)PCR1 & 0xffffffff;
						g_ProfileObjA_0041ed40[(uint)iVar3 >> 4].fileCount = g_ProfileObjA_0041ed40[(uint)iVar3 >> 4].fileCount + 1;
					})
				}
				if (pInStaticMeshMaster->isChild == 1) {
					g_VideoMemoryPtr_00449280->cmdA = 0xe10000002;
					g_VideoMemoryPtr_00449280->cmdB = 0;
					*(undefined4*)&g_VideoMemoryPtr_00449280->cmdB = 0;
					*(undefined4*)((ulong)&g_VideoMemoryPtr_00449280->cmdB + 4) = 0x50000002;
					g_VideoMemoryPtr_00449280 = g_VideoMemoryPtr_00449280 + 1;

					// TAG
					g_VideoMemoryPtr_00449280->cmdA = SCE_GIF_SET_TAG(
						1,				// NLOOP
						SCE_GS_TRUE,	// EOP
						SCE_GS_FALSE,	// PRE
						1,				// PRIM
						SCE_GIF_PACKED,	// FLG
						1				// NREG
					);
					g_VideoMemoryPtr_00449280->cmdB = SCE_GIF_PACKED_AD;
					g_VideoMemoryPtr_00449280 = g_VideoMemoryPtr_00449280 + 1;

					// ZBUF
					g_VideoMemoryPtr_00449280->cmdA = SCE_GS_SET_ZBUF(
						pInStaticMeshMaster->pCameraObj28_0xc->pZBuffer->frameBasePtr,	// ZBP
						SCE_GS_PSMZ32,						// PSM
						SCE_GS_TRUE							// ZMASK
					);
					g_VideoMemoryPtr_00449280->cmdB = SCE_GS_ZBUF_1;
					g_VideoMemoryPtr_00449280 = g_VideoMemoryPtr_00449280 + 1;

					ProcessDrawTypes_0029bc30();
					GenerateRenderingCommands_002b51d0();
				}
				else {
					ProcessDrawTypes_0029bc30();
				}
				g_VideoMemoryPtr_00449280 = SetScreenParams_002aabd0(g_VideoMemoryPtr_00449280, &CameraScreenData_00449390);
				pCVar2 = pInStaticMeshMaster->pCameraObj28_0xc;
				CameraScreenData_00449560.posX = pCVar2->posX;
				CameraScreenData_00449560.posY = pCVar2->posY;
				CameraScreenData_00449560.screenWidth = pCVar2->screenWidth;
				CameraScreenData_00449560.screenHeight = pCVar2->screenHeight;
				g_VideoMemoryPtr_00449280 = SetScreenParams_002aabd0(g_VideoMemoryPtr_00449280, &CameraScreenData_00449390);
				RenderSetupFunc_002b1220();
				if (ed3D::Params_00424e30.bEnableProfile != 0) {
					IMPLEMENTATION_GUARD(
					uVar5 = (uint)g_3DFlushProfileObjIndex_004491ec >> 4;
					if ((g_3DFlushProfileObjIndex_004491ec & 1U) == 0) {
						g_ProfileObjB_0041ed44[uVar5].field_0x0 = (long)PCR1 - g_ProfileObjB_0041ed44[uVar5].field_0x0;
						g_ProfileObjB_0041ed44[uVar5].field_0x8 =
							g_ProfileObjB_0041ed44[uVar5].field_0x8 + g_ProfileObjB_0041ed44[uVar5].field_0x0;
						pPVar7 = g_ProfileObjB_0041ed44 + uVar5;
						if (pPVar7->field_0x10 < g_ProfileObjB_0041ed44[uVar5].field_0x0) {
							pPVar7->field_0x10 = pPVar7->field_0x0;
						}
					}
					else {
						g_ProfileObjA_0041ed40[uVar5].field_0x0 = (long)PCR1 - g_ProfileObjA_0041ed40[uVar5].field_0x0;
						g_ProfileObjA_0041ed40[uVar5].field_0x8 =
							g_ProfileObjA_0041ed40[uVar5].field_0x8 + g_ProfileObjA_0041ed40[uVar5].field_0x0;
						pPVar7 = g_ProfileObjA_0041ed40 + uVar5;
						if (pPVar7->field_0x10 < g_ProfileObjA_0041ed40[uVar5].field_0x0) {
							pPVar7->field_0x10 = pPVar7->field_0x0;
						}
					})
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
		RenderTaskData renderTaskData;

		RENDER_LOG("LaunchRenderTask_002b39d0 %p (%d) (child: %d)\n", pStaticMeshMaster, GetStaticMeshMasterIndex(pStaticMeshMaster), pStaticMeshMaster->isChild);

		iVar4 = g_3DFlushProfileObjIndex_004491ec;
		if (ed3D::Params_00424e30.bEnableProfile != 0) {
			IMPLEMENTATION_GUARD(
			if ((g_3DFlushProfileObjIndex_004491ec & 1U) == 0) {
				uVar8 = (uint)g_3DFlushProfileObjIndex_004491ec >> 4;
				if (g_ProfileObjB_0041ed44[uVar8].field_0x34 != 0) {
					g_ProfileObjB_0041ed44[uVar8].field_0x8 = 0;
					g_ProfileObjB_0041ed44[uVar8].fileCount = 0;
					g_ProfileObjB_0041ed44[uVar8].field_0x34 = 0;
				}
				g_ProfileObjB_0041ed44[(uint)iVar4 >> 4].field_0x0 = (long)PCR1 & 0xffffffff;
				g_ProfileObjB_0041ed44[(uint)iVar4 >> 4].fileCount = g_ProfileObjB_0041ed44[(uint)iVar4 >> 4].fileCount + 1;
			}
			else {
				uVar8 = (uint)g_3DFlushProfileObjIndex_004491ec >> 4;
				if (g_ProfileObjA_0041ed40[uVar8].field_0x34 != 0) {
					g_ProfileObjA_0041ed40[uVar8].field_0x8 = 0;
					g_ProfileObjA_0041ed40[uVar8].fileCount = 0;
					g_ProfileObjA_0041ed40[uVar8].field_0x34 = 0;
				}
				g_ProfileObjA_0041ed40[(uint)iVar4 >> 4].field_0x0 = (long)PCR1 & 0xffffffff;
				g_ProfileObjA_0041ed40[(uint)iVar4 >> 4].fileCount = g_ProfileObjA_0041ed40[(uint)iVar4 >> 4].fileCount + 1;
			})
		}
		pRVar5 = SetScreenParams_002aabd0(g_VideoMemoryPtr_00449280, &CameraScreenData_00449390);
		g_VideoMemoryPtr_00449280 = pRVar5;
		FUN_0029c030();
		uVar8 = SetupCameraObj130_0029a960(pStaticMeshMaster, (ulong)((pStaticMeshMaster->flags_0x4 & 0x20) != 0));
		if (ed3D::Params_00424e30.bEnableProfile != 0) {
			IMPLEMENTATION_GUARD(
			uVar6 = (uint)g_3DFlushProfileObjIndex_004491ec >> 4;
			if ((g_3DFlushProfileObjIndex_004491ec & 1U) == 0) {
				g_ProfileObjB_0041ed44[uVar6].field_0x0 = (long)PCR1 - g_ProfileObjB_0041ed44[uVar6].field_0x0;
				g_ProfileObjB_0041ed44[uVar6].field_0x8 =
					g_ProfileObjB_0041ed44[uVar6].field_0x8 + g_ProfileObjB_0041ed44[uVar6].field_0x0;
				pPVar9 = g_ProfileObjB_0041ed44 + uVar6;
				if (pPVar9->field_0x10 < g_ProfileObjB_0041ed44[uVar6].field_0x0) {
					pPVar9->field_0x10 = pPVar9->field_0x0;
				}
			}
			else {
				g_ProfileObjA_0041ed40[uVar6].field_0x0 = (long)PCR1 - g_ProfileObjA_0041ed40[uVar6].field_0x0;
				g_ProfileObjA_0041ed40[uVar6].field_0x8 =
					g_ProfileObjA_0041ed40[uVar6].field_0x8 + g_ProfileObjA_0041ed40[uVar6].field_0x0;
				pPVar9 = g_ProfileObjA_0041ed40 + uVar6;
				if (pPVar9->field_0x10 < g_ProfileObjA_0041ed40[uVar6].field_0x0) {
					pPVar9->field_0x10 = pPVar9->field_0x0;
				}
			})
		}
		iVar4 = g_3DFlushProfileObjIndex_004491ec;
		if ((uVar8 & 0xff) != 0) {
			if (ed3D::Params_00424e30.bEnableProfile != 0) {
				IMPLEMENTATION_GUARD(
				if ((g_3DFlushProfileObjIndex_004491ec & 1U) == 0) {
					uVar6 = (uint)g_3DFlushProfileObjIndex_004491ec >> 4;
					if (g_ProfileObjB_0041ed44[uVar6].field_0x34 != 0) {
						g_ProfileObjB_0041ed44[uVar6].field_0x8 = 0;
						g_ProfileObjB_0041ed44[uVar6].fileCount = 0;
						g_ProfileObjB_0041ed44[uVar6].field_0x34 = 0;
					}
					g_ProfileObjB_0041ed44[(uint)iVar4 >> 4].field_0x0 = (long)PCR1 & 0xffffffff;
					g_ProfileObjB_0041ed44[(uint)iVar4 >> 4].fileCount = g_ProfileObjB_0041ed44[(uint)iVar4 >> 4].fileCount + 1;
				}
				else {
					uVar6 = (uint)g_3DFlushProfileObjIndex_004491ec >> 4;
					if (g_ProfileObjA_0041ed40[uVar6].field_0x34 != 0) {
						g_ProfileObjA_0041ed40[uVar6].field_0x8 = 0;
						g_ProfileObjA_0041ed40[uVar6].fileCount = 0;
						g_ProfileObjA_0041ed40[uVar6].field_0x34 = 0;
					}
					g_ProfileObjA_0041ed40[(uint)iVar4 >> 4].field_0x0 = (long)PCR1 & 0xffffffff;
					g_ProfileObjA_0041ed40[(uint)iVar4 >> 4].fileCount = g_ProfileObjA_0041ed40[(uint)iVar4 >> 4].fileCount + 1;
				})
			}
			LinkStaticList_0029c140();
			bVar3 = pStaticMeshMaster->pCameraObj28_0xc->clearColor.a;
			pStaticMeshMaster->pCameraObj28_0xc->clearColor.a = bVar3 & 0xfc;
			g_VideoMemoryPtr_00449280 = SetupDrawEnvironment_002b1990(pStaticMeshMaster, g_VideoMemoryPtr_00449280);
			pStaticMeshMaster->pCameraObj28_0xc->clearColor.a = bVar3;
			if (ed3D::Params_00424e30.bEnableProfile != 0) {
				IMPLEMENTATION_GUARD(
				uVar6 = (uint)g_3DFlushProfileObjIndex_004491ec >> 4;
				if ((g_3DFlushProfileObjIndex_004491ec & 1U) == 0) {
					g_ProfileObjB_0041ed44[uVar6].field_0x0 = (long)PCR1 - g_ProfileObjB_0041ed44[uVar6].field_0x0;
					g_ProfileObjB_0041ed44[uVar6].field_0x8 =
						g_ProfileObjB_0041ed44[uVar6].field_0x8 + g_ProfileObjB_0041ed44[uVar6].field_0x0;
					pPVar9 = g_ProfileObjB_0041ed44 + uVar6;
					if (pPVar9->field_0x10 < g_ProfileObjB_0041ed44[uVar6].field_0x0) {
						pPVar9->field_0x10 = pPVar9->field_0x0;
					}
				}
				else {
					g_ProfileObjA_0041ed40[uVar6].field_0x0 = (long)PCR1 - g_ProfileObjA_0041ed40[uVar6].field_0x0;
					g_ProfileObjA_0041ed40[uVar6].field_0x8 =
						g_ProfileObjA_0041ed40[uVar6].field_0x8 + g_ProfileObjA_0041ed40[uVar6].field_0x0;
					pPVar9 = g_ProfileObjA_0041ed40 + uVar6;
					if (pPVar9->field_0x10 < g_ProfileObjA_0041ed40[uVar6].field_0x0) {
						pPVar9->field_0x10 = pPVar9->field_0x0;
					}
				}
				iVar4 = g_3DRendProfileObjIndex_004491e8;
				if ((g_3DRendProfileObjIndex_004491e8 & 1U) == 0) {
					uVar6 = (uint)g_3DRendProfileObjIndex_004491e8 >> 4;
					if (g_ProfileObjB_0041ed44[uVar6].field_0x34 != 0) {
						g_ProfileObjB_0041ed44[uVar6].field_0x8 = 0;
						g_ProfileObjB_0041ed44[uVar6].fileCount = 0;
						g_ProfileObjB_0041ed44[uVar6].field_0x34 = 0;
					}
					g_ProfileObjB_0041ed44[(uint)iVar4 >> 4].field_0x0 = (long)PCR1 & 0xffffffff;
					g_ProfileObjB_0041ed44[(uint)iVar4 >> 4].fileCount = g_ProfileObjB_0041ed44[(uint)iVar4 >> 4].fileCount + 1;
				}
				else {
					uVar6 = (uint)g_3DRendProfileObjIndex_004491e8 >> 4;
					if (g_ProfileObjA_0041ed40[uVar6].field_0x34 != 0) {
						g_ProfileObjA_0041ed40[uVar6].field_0x8 = 0;
						g_ProfileObjA_0041ed40[uVar6].fileCount = 0;
						g_ProfileObjA_0041ed40[uVar6].field_0x34 = 0;
					}
					g_ProfileObjA_0041ed40[(uint)iVar4 >> 4].field_0x0 = (long)PCR1 & 0xffffffff;
					g_ProfileObjA_0041ed40[(uint)iVar4 >> 4].fileCount = g_ProfileObjA_0041ed40[(uint)iVar4 >> 4].fileCount + 1;
				})
			}
			renderTaskData.isChild = pStaticMeshMaster->isChild;
			renderTaskData.taskID = 2;
			renderTaskData.pStaticMeshMaster = pStaticMeshMaster;
			edSysHandlersCall(edDlist::sysHandler_0048cb90.mainIdentifier, edDlist::sysHandler_0048cb90.entries,
				edDlist::sysHandler_0048cb90.maxEventID, 0, &renderTaskData);
			if (ed3D::Params_00424e30.bEnableProfile != 0) {
				IMPLEMENTATION_GUARD(
				uVar6 = (uint)g_3DRendProfileObjIndex_004491e8 >> 4;
				if ((g_3DRendProfileObjIndex_004491e8 & 1U) == 0) {
					g_ProfileObjB_0041ed44[uVar6].field_0x0 = (long)PCR1 - g_ProfileObjB_0041ed44[uVar6].field_0x0;
					g_ProfileObjB_0041ed44[uVar6].field_0x8 =
						g_ProfileObjB_0041ed44[uVar6].field_0x8 + g_ProfileObjB_0041ed44[uVar6].field_0x0;
					pPVar9 = g_ProfileObjB_0041ed44 + uVar6;
					if (pPVar9->field_0x10 < g_ProfileObjB_0041ed44[uVar6].field_0x0) {
						pPVar9->field_0x10 = pPVar9->field_0x0;
					}
				}
				else {
					g_ProfileObjA_0041ed40[uVar6].field_0x0 = (long)PCR1 - g_ProfileObjA_0041ed40[uVar6].field_0x0;
					g_ProfileObjA_0041ed40[uVar6].field_0x8 =
						g_ProfileObjA_0041ed40[uVar6].field_0x8 + g_ProfileObjA_0041ed40[uVar6].field_0x0;
					pPVar9 = g_ProfileObjA_0041ed40 + uVar6;
					if (pPVar9->field_0x10 < g_ProfileObjA_0041ed40[uVar6].field_0x0) {
						pPVar9->field_0x10 = pPVar9->field_0x0;
					}
				}
				iVar4 = g_3DFlushProfileObjIndex_004491ec;
				if ((g_3DFlushProfileObjIndex_004491ec & 1U) == 0) {
					uVar6 = (uint)g_3DFlushProfileObjIndex_004491ec >> 4;
					if (g_ProfileObjB_0041ed44[uVar6].field_0x34 != 0) {
						g_ProfileObjB_0041ed44[uVar6].field_0x8 = 0;
						g_ProfileObjB_0041ed44[uVar6].fileCount = 0;
						g_ProfileObjB_0041ed44[uVar6].field_0x34 = 0;
					}
					g_ProfileObjB_0041ed44[(uint)iVar4 >> 4].field_0x0 = (long)PCR1 & 0xffffffff;
					g_ProfileObjB_0041ed44[(uint)iVar4 >> 4].fileCount = g_ProfileObjB_0041ed44[(uint)iVar4 >> 4].fileCount + 1;
				}
				else {
					uVar6 = (uint)g_3DFlushProfileObjIndex_004491ec >> 4;
					if (g_ProfileObjA_0041ed40[uVar6].field_0x34 != 0) {
						g_ProfileObjA_0041ed40[uVar6].field_0x8 = 0;
						g_ProfileObjA_0041ed40[uVar6].fileCount = 0;
						g_ProfileObjA_0041ed40[uVar6].field_0x34 = 0;
					}
					g_ProfileObjA_0041ed40[(uint)iVar4 >> 4].field_0x0 = (long)PCR1 & 0xffffffff;
					g_ProfileObjA_0041ed40[(uint)iVar4 >> 4].fileCount = g_ProfileObjA_0041ed40[(uint)iVar4 >> 4].fileCount + 1;
				})
			}
			pRVar7 = SetScreenParams_002aabd0(g_VideoMemoryPtr_00449280, &CameraScreenData_00449390);
			g_VideoMemoryPtr_00449280 = pRVar7;
			ProcessDrawTypes_0029bc30();
			bVar1 = g_VideoMemoryPtr_00449280 == pRVar7;
			pRVar7 = SetScreenParams_002aabd0(g_VideoMemoryPtr_00449280, &CameraScreenData_00449390);
			g_VideoMemoryPtr_00449280 = pRVar7;
			GenerateRenderingCommands_002b51d0();
			bVar2 = g_VideoMemoryPtr_00449280 == pRVar7;
			RenderSetupFunc_002b1220();
			if (bVar2 && bVar1) {
				g_VideoMemoryPtr_00449280 = pRVar5;
			}
			pRVar5 = g_VideoMemoryPtr_00449280;
			if (ed3D::Params_00424e30.bEnableProfile != 0) {
				IMPLEMENTATION_GUARD(
				uVar6 = (uint)g_3DFlushProfileObjIndex_004491ec >> 4;
				if ((g_3DFlushProfileObjIndex_004491ec & 1U) == 0) {
					g_ProfileObjB_0041ed44[uVar6].field_0x0 = (long)PCR1 - g_ProfileObjB_0041ed44[uVar6].field_0x0;
					g_ProfileObjB_0041ed44[uVar6].field_0x8 =
						g_ProfileObjB_0041ed44[uVar6].field_0x8 + g_ProfileObjB_0041ed44[uVar6].field_0x0;
					pPVar9 = g_ProfileObjB_0041ed44 + uVar6;
					pRVar5 = g_VideoMemoryPtr_00449280;
					if (pPVar9->field_0x10 < g_ProfileObjB_0041ed44[uVar6].field_0x0) {
						pPVar9->field_0x10 = pPVar9->field_0x0;
						pRVar5 = g_VideoMemoryPtr_00449280;
					}
				}
				else {
					g_ProfileObjA_0041ed40[uVar6].field_0x0 = (long)PCR1 - g_ProfileObjA_0041ed40[uVar6].field_0x0;
					g_ProfileObjA_0041ed40[uVar6].field_0x8 =
						g_ProfileObjA_0041ed40[uVar6].field_0x8 + g_ProfileObjA_0041ed40[uVar6].field_0x0;
					pPVar9 = g_ProfileObjA_0041ed40 + uVar6;
					pRVar5 = g_VideoMemoryPtr_00449280;
					if (pPVar9->field_0x10 < g_ProfileObjA_0041ed40[uVar6].field_0x0) {
						pPVar9->field_0x10 = pPVar9->field_0x0;
						pRVar5 = g_VideoMemoryPtr_00449280;
					}
				})
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
				bVar1 = g_CameraObj_28_00449384->clearColor.a;
				g_CameraObj_28_00449384->clearColor.a = 0;
				pRVar2 = g_VideoMemoryPtr_00449280;
				if (g_CameraObj_28_00449384 != (CameraObj_28*)0x0) {
					g_VideoMemoryPtr_00449280 = BuildCameraCommands_002bb110(g_CameraObj_28_00449384, g_VideoMemoryPtr_00449280 + 1);
					uVar4 = ((uint)((ulong)g_VideoMemoryPtr_00449280 - (ulong)pRVar2) >> 4) - 1;
					pRVar2->cmdA = (ulong)(int)uVar4 & 0xffffffffU | 0xe10000000;
					pRVar2->cmdB = 0;
					*(undefined4*)&pRVar2->cmdB = 0;
					*(uint*)((ulong)&pRVar2->cmdB + 4) = uVar4 | 0x51000000;
				}
				g_CameraObj_28_00449384->clearColor.a = bVar1;
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

	void FUN_002b1110(void)
	{
		if (g_MeshTransformSpecial_004494a8 != (MeshTransformSpecial*)0x0) {
			g_MeshTransformSpecial_004494a8->pNext_0x4 =
				(MeshTransformSpecial*)(g_ModelDoubleBuffer_0048c6d0 + g_CameraFlipIndex_004494a4);
		}
		return;
	}

	void RenderScene(int, int, char*)
	{
		CameraObj_28* pCVar1;
		ulong uVar2;
		bool bVar3;
		int iVar4;
		RenderCommand* pRVar5;
		undefined4 in_zero_lo;
		undefined4 in_zero_hi;
		undefined4 in_zero_udw;
		undefined4 in_register_0000000c;
		FrameBuffer* pFVar6;
		uint uVar7;
		//ProfileObject* pPVar8;
		undefined4 uVar9;
		undefined4 uVar10;
		undefined4 uVar11;
		CameraPanMasterHeader* pCVar12;
		StaticMeshMaster* pInStaticMeshMaster;
		uint staticMeshMasterIndex;
		uint staticMeshMasterFlags;

		iVar4 = g_3DRendProfileObjIndex_004491e8;
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
				UpdateMeshTransforms_002b2a40();
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
			for (staticMeshMasterIndex = 0; staticMeshMasterIndex < (uint)ed3D::Params_00424e30.staticMeshMasterCount;
				staticMeshMasterIndex = staticMeshMasterIndex + 1) {
				staticMeshMasterFlags = ed3D::cameraPanStaticMasterArray_00449378[staticMeshMasterIndex].flags_0x4;
				if ((((staticMeshMasterFlags & 1) != 0) && ((staticMeshMasterFlags & 4) == 0)) &&
					(ed3D::cameraPanStaticMasterArray_00449378[staticMeshMasterIndex].isChild == 0)) {
					if (bVar3) {
						bVar3 = false;
						pFVar6 = GetFramebuffer_002ba3b0();
						if ((pFVar6 != (FrameBuffer*)0x0) && (g_RenderCommandBuffer_00449400 != (RenderCommand*)0x0)) {
							g_VideoMemoryPtr_00449280->cmdA = 0x30000000;
							g_VideoMemoryPtr_00449280->cmdB = 0x1300000000000000;
							RenderCommand* pRVar5 = g_VideoMemoryPtr_00449280;
							g_VideoMemoryPtr_00449280 = g_VideoMemoryPtr_00449280 + 1;
							uVar2 = g_RenderCommandBuffer_00449400->cmdA;
							uVar9 = *(undefined4*)&g_RenderCommandBuffer_00449400->cmdB;
							uVar10 = *(undefined4*)((ulong)&g_RenderCommandBuffer_00449400->cmdB + 4);
							*(int*)&g_VideoMemoryPtr_00449280->cmdA = (int)uVar2;
							*(int*)((ulong)&pRVar5[1].cmdA + 4) = (int)(uVar2 >> 0x20);
							*(undefined4*)&pRVar5[1].cmdB = uVar9;
							*(undefined4*)((ulong)&pRVar5[1].cmdB + 4) = uVar10;
							pRVar5 = g_VideoMemoryPtr_00449280;
							g_VideoMemoryPtr_00449280 = g_VideoMemoryPtr_00449280 + 1;
							uVar9 = *(undefined4*)((ulong)&g_RenderCommandBuffer_00449400[1].cmdA + 4);
							uVar10 = *(undefined4*)&g_RenderCommandBuffer_00449400[1].cmdB;
							uVar11 = *(undefined4*)((ulong)&g_RenderCommandBuffer_00449400[1].cmdB + 4);
							*(undefined4*)&g_VideoMemoryPtr_00449280->cmdA = *(undefined4*)&g_RenderCommandBuffer_00449400[1].cmdA;
							*(undefined4*)((ulong)&pRVar5[1].cmdA + 4) = uVar9;
							*(undefined4*)&pRVar5[1].cmdB = uVar10;
							*(undefined4*)((ulong)&pRVar5[1].cmdB + 4) = uVar11;
							pRVar5 = g_VideoMemoryPtr_00449280;
							g_VideoMemoryPtr_00449280 = g_VideoMemoryPtr_00449280 + 1;
							uVar9 = *(undefined4*)((ulong)&g_RenderCommandBuffer_00449400[2].cmdA + 4);
							uVar10 = *(undefined4*)&g_RenderCommandBuffer_00449400[2].cmdB;
							uVar11 = *(undefined4*)((ulong)&g_RenderCommandBuffer_00449400[2].cmdB + 4);
							*(undefined4*)&g_VideoMemoryPtr_00449280->cmdA = *(undefined4*)&g_RenderCommandBuffer_00449400[2].cmdA;
							*(undefined4*)((ulong)&pRVar5[1].cmdA + 4) = uVar9;
							*(undefined4*)&pRVar5[1].cmdB = uVar10;
							*(undefined4*)((ulong)&pRVar5[1].cmdB + 4) = uVar11;
							pRVar5 = g_VideoMemoryPtr_00449280;
							g_VideoMemoryPtr_00449280 = g_VideoMemoryPtr_00449280 + 1;
							uVar9 = *(undefined4*)((ulong)&g_RenderCommandBuffer_00449400[3].cmdA + 4);
							uVar10 = *(undefined4*)&g_RenderCommandBuffer_00449400[3].cmdB;
							uVar11 = *(undefined4*)((ulong)&g_RenderCommandBuffer_00449400[3].cmdB + 4);
							*(undefined4*)&g_VideoMemoryPtr_00449280->cmdA = *(undefined4*)&g_RenderCommandBuffer_00449400[3].cmdA;
							*(undefined4*)((ulong)&pRVar5[1].cmdA + 4) = uVar9;
							*(undefined4*)&pRVar5[1].cmdB = uVar10;
							*(undefined4*)((ulong)&pRVar5[1].cmdB + 4) = uVar11;
							g_VideoMemoryPtr_00449280 = g_VideoMemoryPtr_00449280 + 1;
							g_RenderCommandBuffer_00449400->cmdA =
								((long)(int)(g_RenderCommandBuffer_00449400 + 4) & 0xfffffffU) << 0x20 | 0x20000000;
							g_RenderCommandBuffer_00449400->cmdB = 0;
							g_VideoMemoryPtr_00449280->cmdA = 0x30000000;
							g_VideoMemoryPtr_00449280->cmdB = 0x1300000000000000;
							g_VideoMemoryPtr_00449280[1].cmdA = 0x30000000;
							g_VideoMemoryPtr_00449280[1].cmdB = 0x1100000011000000;
							g_VideoMemoryPtr_00449280[2].cmdA = 0x30000000;
							g_VideoMemoryPtr_00449280[2].cmdB = 0x600000000000000;
							g_VideoMemoryPtr_00449280[3].cmdA = 0xe10000001;
							g_VideoMemoryPtr_00449280[3].cmdB = 0;
							pRVar5 = g_VideoMemoryPtr_00449280;
							*(undefined4*)&g_VideoMemoryPtr_00449280[4].cmdA = in_zero_lo;
							*(undefined4*)((ulong)&pRVar5[4].cmdA + 4) = in_zero_hi;
							*(undefined4*)&pRVar5[4].cmdB = in_zero_udw;
							*(undefined4*)((ulong)&pRVar5[4].cmdB + 4) = in_register_0000000c;
							g_VideoMemoryPtr_00449280[5].cmdA = 0x30000000;
							g_VideoMemoryPtr_00449280[5].cmdB = 0x600800000000000;
							g_VideoMemoryPtr_00449280 = g_VideoMemoryPtr_00449280 + 6;
						}
					}
					pRVar5 = g_VideoMemoryPtr_00449280;
					pInStaticMeshMaster = ed3D::cameraPanStaticMasterArray_00449378 + staticMeshMasterIndex;

					RENDER_LOG("RenderScene Processing %p (%d)\n", pInStaticMeshMaster, staticMeshMasterIndex);

					if (g_RenderActiveCameraObj_006f7d80 == (CameraObj_28*)0x0) {
						g_RenderActiveCameraObj_006f7d80 = pInStaticMeshMaster->pCameraObj28_0xc;
					}
					pCVar1 = pInStaticMeshMaster->pCameraObj28_0xc;
					CameraScreenData_00449390.posX = pCVar1->posX;
					CameraScreenData_00449390.posY = pCVar1->posY;
					CameraScreenData_00449390.screenWidth = pCVar1->screenWidth;
					CameraScreenData_00449390.screenHeight = pCVar1->screenHeight;
					g_FrontendStaticMeshMaster_00449558 = pInStaticMeshMaster;
					CallMeshRenderFunc_002b2b70(pInStaticMeshMaster, pInStaticMeshMaster);

					RENDER_LOG("RenderScene Begin Processing Children\n");
					for (pCVar12 = (CameraPanMasterHeader*)(pInStaticMeshMaster->headerA).pLoadedData;
						pCVar12 != &pInStaticMeshMaster->headerA; pCVar12 = (CameraPanMasterHeader*)pCVar12->pLoadedData) {
						if ((((StaticMeshMaster*)pCVar12->pCameraPanHeader_0xc)->flags_0x4 & 4) == 0) {
							CallMeshRenderFunc_002b2b70((StaticMeshMaster*)pCVar12->pCameraPanHeader_0xc, pInStaticMeshMaster);
						}
					}
					RENDER_LOG("RenderScene End Processing Children\n");

					if (BYTE_00448a5c != 0) {
						LaunchRenderTask_002b39d0(pInStaticMeshMaster);
					}
					iVar4 = g_3DFlushProfileObjIndex_004491ec;
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
					if (pRVar5 != g_VideoMemoryPtr_00449280) {
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
			iVar4 = g_3DFlushProfileObjIndex_004491ec;
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
			FUN_002b1110();
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

				g_VideoMemoryPtr_00449280->cmdB = 0x0;
				sceDmaAddEnd((sceDmaTag**)&g_VideoMemoryPtr_00449280, 0, 0);
				if (g_RenderCommandBuffer_00449400 != (RenderCommand*)0x0) {
					g_RenderCommandBuffer_00449400->cmdA =
						((ulong)(int)(g_RenderCommandBuffer_00449400 + 4) & 0xfffffffU) << 0x20 | 0x20000000;
					g_RenderCommandBuffer_00449400->cmdB = 0;
				}
			}
			if (ed3D::Params_00424e30.bEnableProfile != 0) {
				assert(false);
				uVar7 = (uint)g_3DFlushProfileObjIndex_004491ec >> 4;
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

	void LinkHeaders_002b67d0(MeshTransformParentHeader** aHeaders)
	{
		int iVar1;

		iVar1 = 0;
		do {
			Setup_00290bf0(g_OtherDoubleBuffer_0048cd50 + iVar1, aHeaders[iVar1], 0, 0);
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
		LinkHeaders_002b67d0(&PTR_MeshTransformParentHeader_004494b0);
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

	struct ed3D_60 {
		char a[0x60];
	};


	void InitFunc_002abd00(void)
	{
		PTR_MeshTransformData_ARRAY_0044942c =
			(MeshTransformData*)edMemAlloc(TO_HEAP(H_MAIN), Params_00424e30.meshTransformDataCount * sizeof(MeshTransformData));
		PTR_MeshTransformParentHeader_00449430 =
			AllocateCameraPanHeader_00290af0(Params_00424e30.meshTransformDataCount, TO_HEAP(H_MAIN));
		PTR_MeshTransformParent_ARRAY_00449434 = (MeshTransformParent*)PTR_MeshTransformParentHeader_00449430->field_0x4;
		g_StaticMeshMasterHeap_00449438 = edMemAlloc(TO_HEAP(H_MAIN), Params_00424e30.staticMeshMasterCount * sizeof(ed3D_60));
		return;
	}

	void InitRenderFrame30_002b12a0(int countA, int countB)
	{
		RenderFrame_30* pRVar1;
		RenderFrame_30** ppRVar2;
		RenderFrame_30** ppRVar3;
		uint uVar4;

		uVar4 = 0;
		do {
			ppRVar3 = &RenderFrame_30_Header_004494c0.pMaterialArrayBegin + uVar4;
			pRVar1 = (RenderFrame_30*)edMemAlloc(TO_HEAP(H_MAIN), countA * sizeof(RenderFrame_30));
			*ppRVar3 = pRVar1;
			ppRVar2 = &RenderFrame_30_Header_004494c0.pMaterialArrayEnd + uVar4;
			uVar4 = uVar4 + 1 & 0xffff;
			*ppRVar2 = *ppRVar3 + countA;
		} while (uVar4 < 2);
		RenderFrame_30_Header_004494c0.pOtherStartA = (RenderFrame_30*)edMemAlloc(TO_HEAP(H_MAIN), countB * sizeof(RenderFrame_30));
		RenderFrame_30_Header_004494c0.pMeshCurrent = RenderFrame_30_Header_004494c0.pMaterialArrayBegin;
		RenderFrame_30_Header_004494c0.pMeshEnd = RenderFrame_30_Header_004494c0.pMaterialArrayEnd;
		RenderFrame_30_Header_004494c0.pOtherEndA = RenderFrame_30_Header_004494c0.pOtherStartA + countB;
		RenderFrame_30_Header_004494c0.pCurrentRenderFrame30 = RenderFrame_30_Header_004494c0.pOtherStartA;
		RenderFrame_30_Header_004494c0.pOtherEndB = RenderFrame_30_Header_004494c0.pOtherStartA + countB;
		return;
	}

	MeshInfo* g_LoadMeshBuffer_004491c8 = NULL;
	DisplayListInternalMesh* g_MeshDisplayListInternal_00449380 = NULL;

	void Init_Internal(void)
	{
		TextureInfoSmall* pTVar1;
		uint uVar2;
		int size;

		PrintString(s_ed3D_Initialsation_004333a0);
		InitVidMode_002936b0();
		//DAT_004491b0 = 1;
		CameraPanMasterHeaderA_004491d0 = AllocateCameraAndMesh_00290a10(1, Params_00424e30.meshHeaderCountAA, TO_HEAP(H_MAIN));
		CameraPanMasterHeaderB_004491d4 = AllocateCameraAndMesh_00290a10(1, Params_00424e30.meshHeaderCountBB, TO_HEAP(H_MAIN));
		g_LoadMeshBuffer_004491c8 = (MeshInfo*)edMemAlloc(TO_HEAP(H_MAIN), Params_00424e30.meshHeaderCountAA * sizeof(MeshInfo));
		memset(g_LoadMeshBuffer_004491c8, 0, Params_00424e30.meshHeaderCountAA * sizeof(MeshInfo));
		TexturePool_004491cc = (TextureInfoSmall*)edMemAlloc(TO_HEAP(H_MAIN), Params_00424e30.meshHeaderCountBB * sizeof(TextureInfoSmall));
		memset(TexturePool_004491cc, 0, Params_00424e30.meshHeaderCountBB * sizeof(TextureInfoSmall));
		//pTVar1 = TexturePool_004491cc;
		INT_004491b4 = 0;
		LoadedTextureCount_004491c4 = 0;
		//*(undefined8*)TexturePool_004491cc = 0x3fff00000000;
		//*(undefined8*)&pTVar1->textureHeaderStart = 0x50;
		//*(undefined8*)&pTVar1->textureBufferStart = 0x7ffe000000000;
		memset(&SomeMeshThing_ARRAY_0048c3e0, 0, sizeof(SomeMeshThing) * 16);
		cameraPanStaticMasterArray_00449378 = (StaticMeshMaster*)edMemAlloc(TO_HEAP(H_MAIN), Params_00424e30.staticMeshMasterCount * sizeof(StaticMeshMaster) + 1);
		memset(cameraPanStaticMasterArray_00449378, 0, Params_00424e30.staticMeshMasterCount * sizeof(StaticMeshMaster));
		PTR_MeshTransformParentHeader_0044937c = AllocateCameraPanHeader_00290af0(Params_00424e30.meshDisplayListInternalCount + Params_00424e30.staticMeshMasterCount * 3, TO_HEAP(H_MAIN));
		g_MeshDisplayListInternal_00449380 = (DisplayListInternalMesh*)edMemAlloc(TO_HEAP(H_MAIN), Params_00424e30.meshDisplayListInternalCount * sizeof(DisplayListInternalMesh));
		memset(g_MeshDisplayListInternal_00449380, 0, Params_00424e30.meshDisplayListInternalCount * sizeof(DisplayListInternalMesh));
		for (uVar2 = 0; uVar2 < Params_00424e30.staticMeshMasterCount; uVar2 = uVar2 + 1) {
			Setup_00290bf0(&cameraPanStaticMasterArray_00449378[uVar2].headerB, PTR_MeshTransformParentHeader_0044937c, 0, 0);
			Setup_00290bf0(&cameraPanStaticMasterArray_00449378[uVar2].headerA, PTR_MeshTransformParentHeader_0044937c, 0, 0);
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
		g_MeshCommandBufferBase_004491d8 = (RenderCommand*)edMemAlloc(TO_HEAP(H_MAIN), Params_00424e30.meshHeaderCountBAlt);
		//PTR_DAT_004491e4 = PTR_PTR_DAT_004491d8 + Params_00424e30.meshHeaderCountBAlt;
		size = (Params_00424e30.field_0x28 * 2 + Params_00424e30.field_0x18 + 0x8c0 + Params_00424e30.meshHeaderCountA * 0x38 + Params_00424e30.field_0x34 * 8) * 0x10;
		g_MeshCommandBufferCurrent_004491e0 = g_MeshCommandBufferBase_004491d8;
		ed3D_Allocator_00449248.base = (char*)edMemAlloc(TO_HEAP(H_MAIN), size);
		memset(ed3D_Allocator_00449248.base, 0, size);
		ed3D_Allocator_00449248.current = ed3D_Allocator_00449248.base + size;
		ed3D_Allocator_00449248.end = ed3D_Allocator_00449248.base;
		ed3D_SubAllocator_004491b8.base = SubAllocate_0029ce60(16000);
		ed3D_SubAllocator_004491b8.end = ed3D_SubAllocator_004491b8.base + 16000;
		ed3D_SubAllocator_004491b8.current = ed3D_SubAllocator_004491b8.base;
		InitRenderFrame30_002b12a0(Params_00424e30.field_0x14, Params_00424e30.meshHeaderCountA);
		//FUN_002ad860();
		InitFunc_002b13b0(Params_00424e30.field_0x14, Params_00424e30.meshHeaderCountB);
		InitVideoMemory_0029c220(Params_00424e30.field_0x18 + Params_00424e30.field_0x34 * 8, Params_00424e30.field_0x28);
		//thunk_FUN_002ab6d0();
		SetGIFMode_0029cf60();
		//if (DAT_004491f8 == '\0') {
		//	DAT_004491f8 = '\x01';
		//}
		g_RenderActiveCameraObj_006f7d80 = (CameraObj_28*)0x0;
		//FUN_0029ced0();
		ResetMemoryPtr_0029c060();
		ResetDoubleBuffer_002b1140();
		RenderSetupFunc_002b1220();
		UpdateRenderFrame30_002b6b20();
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

	ed3D_Params* GetParams_00424e30(void)
	{
		return &Params_00424e30;
	}

	void InitSomeMeshStruct_002ab570(int index, undefined4 param_2, undefined4 param_3, uint param_4, undefined4 param_5)
	{
		int iVar1;

		iVar1 = index + -1;
		if (SomeMeshThing_ARRAY_0048c3e0 + iVar1 != (SomeMeshThing*)0x0) {
			SomeMeshThing_ARRAY_0048c3e0[iVar1].field_0x0 = param_2;
			SomeMeshThing_ARRAY_0048c3e0[iVar1].field_0x4 = param_3;
			SomeMeshThing_ARRAY_0048c3e0[iVar1].field_0x8 = param_4 | 4;
			SomeMeshThing_ARRAY_0048c3e0[iVar1].field_0xc = 0;
			SomeMeshThing_ARRAY_0048c3e0[iVar1].field_0x10 = param_5;
		}
		return;
	}

	void Init(void)
	{
		ed3D_Params* puVar1;
		undefined4* puVar2;

		puVar1 = GetParams_00424e30();
		puVar1->meshHeaderCountB = 2000;
		puVar1->field_0x18 = 55000;
		puVar1->field_0x14 = 500;
		puVar1->meshHeaderCountAA = 0x1e;
		puVar1->meshHeaderCountBB = 0x6e;
		puVar1->meshHeaderCountBAlt = 0x180;
		puVar1->staticMeshMasterCount = 0xf;
		puVar1->field_0x28 = 0x15e;
		puVar1->meshHeaderCountA = 0x15e;
		puVar1->field_0x34 = 0;
		puVar1->meshTransformDataCount = 0x708;
		puVar1->field_0x2d = 0;
		puVar1->bEnableProfile = 0;
		puVar1->field_0x2c = 1;
		Init_Internal();
		InitSomeMeshStruct_002ab570(2, 0x800, 0, 1, 0);
		//SetuGpffff8510ToParam1(0x447a0000);
		//puVar2 = (undefined4*)GetDAT_00424e80();
		//*puVar2 = 0;
		//puVar2[1] = 0x41440000;
		//puVar2[2] = 0x43610000;
		FUN_002ab4a0(1, 0, 0xffffff01);
		return;
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
		RenderCommand* puVar2;
		LayerHeaderPacked* pBuffer;
		char* fileBufferStart;
		char* texHeaderStart;

		/* Can point at 004DA040 */
		puVar2 = ed3D::g_MeshCommandBufferCurrent_004491e0;
		texHeaderStart = textureInfoObj->textureHeaderStart;
		iVar1 = *(int*)(texHeaderStart + 8);
		if ((textureInfoObj != (TextureInfoSmall*)0x0) && (fileBufferStart = textureInfoObj->textureFileBufferStart, (*(uint*)(fileBufferStart + 4) & 1) == 0)) {
			IMPLEMENTATION_GUARD();
			for (pBuffer = (LayerHeaderPacked*)VerifyFileBufferRead(texHeaderStart + 0x10, texHeaderStart + iVar1); pBuffer != (LayerHeaderPacked*)0x0;
				pBuffer = (LayerHeaderPacked*)GetNextFileSection((char*)pBuffer, (int*)(texHeaderStart + iVar1))) {
				/* Check if the value in the buffer is 'LAYA' */
				if (*(int*)pBuffer == 0x4159414c) {
					//TextureSetup(pBuffer, textureInfoObj);
				}
			}
			TextureSetup(textureInfoObj, mode);
			textureInfoObj->textureFileLengthB = (ulong)ed3D::g_MeshCommandBufferCurrent_004491e0 + (textureInfoObj->textureFileLengthB - (ulong)puVar2);
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

	char* GetMAT_Section_0029eae0(TextureInfoSmall* pTextureInfo, int param_2)
	{
		char* pcVar1;
		char* iVar2;

		pcVar1 = GetMaterialBufferStart(pTextureInfo, param_2);
		if (pcVar1 == (char*)0x0) {
			iVar2 = 0;
		}
		else {
			iVar2 = (char*)LOAD_SECTION(*(int*)(pcVar1 + 8)) + 0x10;
		}

		return iVar2;
	}

	char* ed3D::GetT2D_Section_0029ea60(char* pMaterialSection, int param_2)
	{
		char* pcVar1;
		char* iVar2;
		int iVar1;

		iVar2 = (char*)0x0;
		iVar1 = *(int*)(pMaterialSection + param_2 * 4 + 0x10);
		char* pLayerSection = (char*)LOAD_SECTION(iVar1);
		pcVar1 = (char*)0x0;
		if ((iVar1 != -0x10) && (*(short*)(pLayerSection + 0x2c) != 0)) {
			iVar2 = (char*)((char*)LOAD_SECTION(*(int*)(pLayerSection + 0x30)) + 0x10);
		}
		if (iVar2 == (char*)0x0) {
			pcVar1 = (char*)0x0;
		}
		else {
			char* otherValue = (char*)LOAD_SECTION(*(int*)(iVar2 + 8));
			if (otherValue != 0) {
				pcVar1 = (char*)((char*)LOAD_SECTION(*(int*)(otherValue + 8)) + 0x10);
			}
		}
		return pcVar1;
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

	void SetupMeshSubsections
	(MeshInfo* meshInfoObj, char* fileBufferStartA, int fileLengthA, char* fileBufferStartB, int fileLengthB)
	{
		int* piVar1;
		int readValue;

		for (piVar1 = (int*)VerifyFileBufferRead(fileBufferStartB, fileBufferStartB + fileLengthB); piVar1 != (int*)0x0;
			piVar1 = GetNextFileSection((char*)piVar1, (int*)(fileBufferStartB + fileLengthB))) {
			readValue = *piVar1;
			/* Is current value CDZA */
			if (readValue == 0x415a4443) {
				meshInfoObj->CDZA = (char*)piVar1;
			}
			else {
				/* Is current value INFA */
				if (readValue == 0x41464e49) {
					meshInfoObj->INFA = (char*)piVar1;
				}
				else {
					/* Is current value MBNA */
					if (readValue == 0x414e424d) {
						meshInfoObj->MBNA = (char*)piVar1;
					}
					else {
						/* Is current value SPRA */
						if (readValue == 0x41525053) {
							meshInfoObj->SPRA = (char*)piVar1;
						}
						else {
							/* Is current value CAMA */
							if (readValue == 0x414d4143) {
								meshInfoObj->CAMA = (char*)piVar1;
							}
							else {
								/* Is current value LIA. */
								if (readValue == 0x2e41494c) {
									meshInfoObj->LIA = (char*)piVar1;
								}
								else {
									/* Is current value CSTA */
									if (readValue == 0x41545343) {
										meshInfoObj->CSTA = (char*)piVar1;
									}
									else {
										/* Is current value ANMA */
										if (readValue == 0x414d4e41) {
											meshInfoObj->ANMA = (char*)piVar1;
										}
										else {
											/* Is current value HALL */
											if (readValue == 0x4c4c4148) {
												meshInfoObj->HALL = (char*)piVar1;
											}
											else {
												/* Is current value OBJA */
												if (readValue == 0x414a424f) {
													meshInfoObj->OBJA = (char*)piVar1;
												}
												else {
													/* Is current value GEOM */
													if (readValue == 0x4d4f4547) {
														meshInfoObj->GEOM = (char*)piVar1;
													}
												}
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
		if (fileBufferStartA != fileBufferStartB) {
			for (piVar1 = (int*)VerifyFileBufferRead(fileBufferStartA, fileBufferStartA + fileLengthA); piVar1 != (int*)0x0;
				piVar1 = GetNextFileSection((char*)piVar1, (int*)(fileBufferStartA + fileLengthA))) {
				readValue = *piVar1;
				if (readValue == 0x415a4443) {
					meshInfoObj->CDZA = (char*)piVar1;
				}
				else {
					if (readValue == 0x41464e49) {
						meshInfoObj->INFA = (char*)piVar1;
					}
					else {
						if (readValue == 0x414e424d) {
							meshInfoObj->MBNA = (char*)piVar1;
						}
						else {
							if (readValue == 0x41525053) {
								meshInfoObj->SPRA = (char*)piVar1;
							}
							else {
								if (readValue == 0x414d4143) {
									meshInfoObj->CAMA = (char*)piVar1;
								}
								else {
									if (readValue == 0x2e41494c) {
										meshInfoObj->LIA = (char*)piVar1;
									}
									else {
										if (readValue == 0x41545343) {
											meshInfoObj->CSTA = (char*)piVar1;
										}
										else {
											if (readValue == 0x414d4e41) {
												meshInfoObj->ANMA = (char*)piVar1;
											}
											else {
												if (readValue == 0x4c4c4148) {
													meshInfoObj->HALL = (char*)piVar1;
												}
												else {
													if (readValue == 0x414a424f) {
														meshInfoObj->OBJA = (char*)piVar1;
													}
													else {
														if (readValue == 0x4d4f4547) {
															meshInfoObj->GEOM = (char*)piVar1;
														}
													}
												}
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
		return;
	}

	char* FindHashInSection(Hash_8 meshHashValue, int* texHashPtr, int hashCount, int triedHashes, int totalHashes)
	{
		uint uVar1;
		ulong* puVar2;

		puVar2 = (ulong*)(texHashPtr + hashCount * 4);
		if (*puVar2 < meshHashValue.number) {
			uVar1 = (uint)(totalHashes - hashCount) >> 1;
			if (uVar1 == 0) {
				uVar1 = 1;
			}
			if (hashCount == totalHashes) {
				puVar2 = (ulong*)0x0;
			}
			else {
				puVar2 = (ulong*)FindHashInSection(meshHashValue, texHashPtr, hashCount + uVar1, hashCount + 1, totalHashes);
			}
		}
		else {
			if (meshHashValue.number < *puVar2) {
				uVar1 = (uint)(hashCount - triedHashes) >> 1;
				if (uVar1 == 0) {
					uVar1 = 1;
				}
				if (hashCount == triedHashes) {
					puVar2 = (ulong*)0x0;
				}
				else {
					puVar2 = (ulong*)FindHashInSection(meshHashValue, texHashPtr, hashCount - uVar1, triedHashes, hashCount + -1);
				}
			}
		}
		return (char*)puVar2;
	}

	char* FindSectionHash(Hash_8 meshHashValue, MeshData_HASH* textureObjMatBuffer)
	{
		char* pcVar1;

		/* If the first value in the texture obj buffer is HASH or MBNK */
		if ((textureObjMatBuffer->header.number == 0x48534148) || (textureObjMatBuffer->header.number == 0x4b4e424d)) {
			pcVar1 = FindHashInSection(meshHashValue, (int*)(textureObjMatBuffer + 1), textureObjMatBuffer->field_0xc - 0x10U >> 5, 0,
				textureObjMatBuffer->field_0xc - 0x10U >> 4);
		}
		else {
			pcVar1 = (char*)0x0;
		}
		return pcVar1;
	}

	uint GetElementCount_002947b0(char* pStart, char* pEnd)
	{
		uint uVar1;

		uVar1 = 0;
		if ((pEnd != (char*)0x0) && (pEnd <= pStart)) {
			pStart = (char*)0x0;
		}
		while (pStart != (char*)0x0) {
			uVar1 = uVar1 + 1;
			if ((pEnd == (char*)0x0) || (pStart + *(int*)(pStart + 8) < pEnd)) {
				pStart = pStart + *(int*)(pStart + 8);
			}
			else {
				pStart = (char*)0x0;
			}
		}
		return uVar1 & 0xffff;
	}

	void LoadMBNA(char* pMBNA, TextureInfo* pTextureInfo)
	{
		int iVar1;
		ulong* puVar2;
		uint uVar3;
		char* pcVar4;
		ulong* pMBNK;

		iVar1 = *(int*)(pMBNA + 8);
		/* Checks all the hashes in the mesh to make sure they match what is in the texture

		   Get the end of the current section */
		for (puVar2 = (ulong*)VerifyFileBufferRead(pMBNA + 0x10, pMBNA + iVar1); puVar2 != (ulong*)0x0;
			puVar2 = (ulong*)GetNextFileSection((char*)puVar2, (int*)(pMBNA + iVar1))) {
			/* Check if read value is MBNK */
			if (*(int*)puVar2 == 0x4b4e424d) {
				uVar3 = *(int*)(puVar2 + 1) - 0x10U >> 4;
				pMBNK = puVar2;
				while (true) {
					if ((uVar3 & 0xffff) == 0) break;
					pcVar4 = FindSectionHash(pMBNK[2], (MeshData_HASH*)(pTextureInfo->field_0x0).materialBufferStart);
					if (pcVar4 != (char*)0x0) {
						*(char**)(pMBNK + 3) = (char*)STORE_SECTION(pcVar4);
					}
					uVar3 = (uVar3 & 0xffff) - 1;
					pMBNK = pMBNK + 2;
				}
			}
		}
		return;
	}

	struct astruct_14 {
		int* field_0x0;
		int* field_0x4;
		int* field_0x8;
		char field_0xc;
		undefined field_0xd;
		undefined field_0xe;
		undefined field_0xf;
		int* field_0x10;
		int* field_0x14;
		int* field_0x18;
		int* field_0x1c;
		int* field_0x20;
		short field_0x24;
	};

	int INT_0044935c = 0;
	bool BOOL_00449370 = false;

	void ProcessCDQU(MeshData_CDQU* pCDQU, bool param_2, MeshInfo* pMeshInfo, TextureInfo* pTextureInfo, int param_5, long param_6)
	{
		ushort uVar1;
		int* piVar2;
		int iVar3;
		int offset;
		int* piVar4;
		uint* puVar5;
		uint uVar6;
		uint uVar7;
		uint uVar8;
		astruct_14 aStack96;
		astruct_14 local_30;
		bool sectionBufferIsEmpty;

		sectionBufferIsEmpty = (pCDQU->field_0x1c & 1) == 0;
		if (param_6 == 0) {
			if (sectionBufferIsEmpty) {
				uVar8 = 0;
				pCDQU->field_0x1c = pCDQU->field_0x1c | 1;
				uVar7 = 0;
				do {
					uVar6 = uVar7 + 1 & 0xffff;
					uVar8 = uVar8 + pCDQU->field_0x10[uVar7 - 8];
					uVar7 = uVar6;
				} while (uVar6 < 0xd);
				INT_0044935c = 0x60;
				local_30.field_0xc = '\0';
				local_30.field_0x10 = (int*)pCDQU->field_0x20;
				local_30.field_0x14 = (int*)pCDQU->field_0x24;
				local_30.field_0x18 = (int*)pCDQU->field_0x28;
				local_30.field_0x1c = (int*)pCDQU->field_0x2c;
				local_30.field_0x20 = (int*)pCDQU->field_0x30;
				piVar4 = (int*)pCDQU->field_0x30;
				piVar2 = (int*)pCDQU->field_0x34;
				local_30.field_0x4 = (int*)pTextureInfo;
				local_30.field_0x8 = (int*)param_5;
				for (uVar7 = 0; uVar7 < uVar8; uVar7 = uVar7 + 1 & 0xffff) {
					IMPLEMENTATION_GUARD(
					local_30.field_0x0 = piVar2 + 4;
					puVar5 = FUN_002a1f10(&local_30);
					if ((puVar5 != (uint*)0x0) &&
						(ed3D::g_MeshCommandBufferCurrent_004491e0 =
							(RenderCommand*)
							FUN_002a16f0(puVar5, (ulong*)ed3D::g_MeshCommandBufferCurrent_004491e0, (int)(piVar4 + 4), 4), uVar7 == 0
							)) {
						pCDQU->field_0x38 = puVar5;
					}
					piVar2 = GetNextFileSection((char*)piVar2, (int*)0x0);)
				}
			}
		}
		else {
			if (BOOL_00449370 == true) {
				IMPLEMENTATION_GUARD(
				piVar4 = (int*)pCDQU->field_0x30;
				puVar5 = (uint*)pCDQU->field_0x38;
				uVar7 = 0;
				if ((piVar4 != (int*)0x0) && (puVar5 != (uint*)0x0)) {
					uVar8 = 0;
					do {
						uVar6 = uVar8 + 1 & 0xffff;
						uVar7 = uVar7 + pCDQU->field_0x10[uVar8 - 8];
						uVar8 = uVar6;
					} while (uVar6 < 0xd);
					INT_0044935c = 0x60;
					for (uVar8 = 0; uVar8 < uVar7; uVar8 = uVar8 + 1 & 0xffff) {
						if ((long)(int)puVar5[0xf] != 0) {
							FUN_002a4230((int)puVar5, (long)(int)puVar5[0xf], (int)(piVar4 + 4));
						}
						puVar5 = (uint*)puVar5[3];
					}
					iVar3 = *(int*)&pCDQU[-1].field_0x44;
					goto LAB_002a40c4;
				})
			}
		}
		iVar3 = *(int*)&pCDQU[-1].field_0x44;
	LAB_002a40c4:
		uVar1 = pCDQU->field_0x1a;
		offset = (uint)uVar1 << 3;
		if ((uVar1 & 1) != 0) {
			offset = (uVar1 + 1) * 8;
		}
		for (piVar4 = (int*)VerifyFileBufferRead
		((char*)((ulong)pCDQU->field_0x10 + offset + 0x30), pCDQU->field_0x0 + iVar3 + -0x10);
			piVar4 != (int*)0x0; piVar4 = GetNextFileSection((char*)piVar4, (int*)(pCDQU->field_0x0 + iVar3 + -0x10))) {
			offset = *piVar4;
			if (offset == 0x41525053) {
				IMPLEMENTATION_GUARD();
				if (sectionBufferIsEmpty) {
					aStack96.field_0xc = '\0';
					aStack96.field_0x10 = (int*)pCDQU->field_0x20;
					aStack96.field_0x14 = (int*)pCDQU->field_0x24;
					aStack96.field_0x18 = (int*)pCDQU->field_0x28;
					aStack96.field_0x1c = (int*)pCDQU->field_0x24;
					aStack96.field_0x20 = (int*)pCDQU->field_0x30;
					aStack96.field_0x4 = (int*)pTextureInfo;
					aStack96.field_0x8 = (int*)param_5;
					IMPLEMENTATION_GUARD(puVar5 = FUN_002a3140((int)piVar4, &aStack96, (int)(pCDQU->field_0x30 + 4), 4));
					pCDQU->field_0x3c = (int)STORE_SECTION(puVar5);
					pCDQU->field_0x1e = aStack96.field_0x24;
				}
			}
			else {
				if ((offset == 0x55514443) || (offset == 0x434f4443)) {
					ProcessCDQU((MeshData_CDQU*)(piVar4 + 4), param_2, pMeshInfo, pTextureInfo, param_5, param_6);
				}
			}
		}
		return;
	}

	void ProcessCSTA(bool param_1, MeshInfo* meshInfoObj, TextureInfo* textureInfo, int param_4)
	{
		int iVar1;
		int iVar2;
		int* piVar3;
		int* nextSection;
		ulong numSections;
		char* pCSTA;
		RenderCommand* pCommandBuffer;

		pCommandBuffer = ed3D::g_MeshCommandBufferCurrent_004491e0;
		pCSTA = meshInfoObj->CSTA;
		iVar1 = *(int*)(pCSTA + 8);
		numSections = (ulong)((*(uint*)(meshInfoObj->fileBufferStart + 4) & 1) != 0);
		for (piVar3 = (int*)VerifyFileBufferRead(pCSTA + 0x10, pCSTA + iVar1); piVar3 != (int*)0x0;
			piVar3 = GetNextFileSection((char*)piVar3, (int*)(pCSTA + iVar1))) {
			/* If read value is CDQA */
			if (*piVar3 == 0x41514443) {
				/* Search for CDQU */
				iVar2 = piVar3[2];
				for (nextSection = (int*)VerifyFileBufferRead((char*)(piVar3 + 0xc), (char*)(int*)((ulong)piVar3 + iVar2));
					nextSection != (int*)0x0; nextSection = GetNextFileSection((char*)nextSection, (int*)((ulong)piVar3 + iVar2)))
				{
					/* If read value is CDQU */
					if (*nextSection == 0x55514443) {
						ProcessCDQU((MeshData_CDQU*)(nextSection + 4), true, meshInfoObj, textureInfo, param_4, numSections);
					}
				}
			}
			else {
				/* If read value is CDOA */
				if (*piVar3 == 0x414f4443) {
					iVar2 = piVar3[2];
					for (nextSection = (int*)VerifyFileBufferRead((char*)(piVar3 + 0xc), (char*)(int*)((ulong)piVar3 + iVar2));
						nextSection != (int*)0x0;
						nextSection = GetNextFileSection((char*)nextSection, (int*)((ulong)piVar3 + iVar2))) {
						/* If read value is CDOC */
						if (*nextSection == 0x434f4443) {
							ProcessCDQU((MeshData_CDQU*)(nextSection + 4), true, meshInfoObj, textureInfo, param_4, numSections);
						}
					}
				}
			}
		}
		meshInfoObj->fileLengthB =
			meshInfoObj->fileLengthB + (int)((int)ed3D::g_MeshCommandBufferCurrent_004491e0 - (int)pCommandBuffer);
		return;
	}

	void CheckHashesProcessCSTA_002a4ec0(MeshInfo* pMeshInfo, TextureInfo* pTextureInfo)
	{
		LoadMBNA(pMeshInfo->MBNA, pTextureInfo);
		if (pMeshInfo->CSTA != (char*)0x0) {
			ProcessCSTA(true, pMeshInfo, pTextureInfo, 0xc);
		}
		return;
	}

	PACK( struct MeshTransformObjData_Packed {
		int pObj; // char*
		short field_0x4;
		short field_0x6;
	});

	PACK( struct TextureData_HIER_Internal {
		Matrix transformA;
		Matrix transformB;
		Hash_8 hash;
		byte field_0x88;
		byte field_0x89;
		ushort bRenderShadow;
		int pShadowAnimMatrix; // Matrix*
		int pLinkTransformData; // MeshTransformData*
		int field_0x94; // undefined*
		int pTextureInfo; // undefined*
		ushort count_0x9c;
		ushort flags_0x9e;
		int pLightingMatrixFuncObj_0xa0; // LightingMatrixFuncObj*
		int field_0xa4; // Matrix*
		int pAnimMatrix; // Matrix*
		short subMeshParentCount_0xac;
		byte size_0xae;
		undefined field_0xaf;
		MeshTransformObjData_Packed aSubArray[4];
	});

	PACK( struct TextureData_HIER {
		char field_0x0[4];
		int field_0x4;
		int field_0x8;
		int field_0xc;
		TextureData_HIER_Internal body;
	});

	int FUN_00294600(TextureData_HIER_Internal* pHIER, TextureInfo* pTextureInfo)
	{
		short sVar1;
		ushort uVar2;
		int* piVar3;
		int iVar4;
		char* pBuffStart;
		TextureData_HIER_Internal* pTVar5;
		TextureData_HIER* pHeader;

		if ((pHIER->flags_0x9e & 4) == 0) {
			pTVar5 = pHIER + 1;
			pHeader = (TextureData_HIER*)((char*)pHIER - 0x10);
			if (pHeader->field_0xc != pHeader->field_0x8) {
				pBuffStart = ((char*)pHIER) + (pHeader->field_0x8 - 0x10);
				iVar4 = *(int*)(pBuffStart + 8);
				for (piVar3 = (int*)VerifyFileBufferRead(pBuffStart, pBuffStart + iVar4); piVar3 != (int*)0x0;
					piVar3 = GetNextFileSection((char*)piVar3, (int*)(pBuffStart + iVar4))) {
					if (*piVar3 == 0x494e4f42) {
						IMPLEMENTATION_GUARD(pHIER->field_0xa8 = piVar3 + 4);
					}
				}
			}
			pHIER->flags_0x9e = pHIER->flags_0x9e | 4;
			IMPLEMENTATION_GUARD(
			for (uVar2 = 0; uVar2 < pHIER->field_0x9c; uVar2 = uVar2 + 1) {
				sVar1 = *(short*)&pTVar5->field_0x4;
				piVar3 = *(int**)(*(int*)pTVar5 + 8);
				if (sVar1 == 3) {
					if (*piVar3 == 0x2e4a424f) {
						//FUN_002a4af0((int)pHIER, (uint*)(piVar3 + 4), pTextureInfo);
					}
				}
				else {
					if ((((sVar1 != 2) && (sVar1 != 1)) && (sVar1 == 0)) && (*piVar3 == 0x2e4a424f)) {
						//FUN_002a4c60((int)pHIER, (uint*)(piVar3 + 4), pTextureInfo);
					}
				}
				pTVar5 = (TextureData_HIER_Internal*)&pTVar5->field_0x8;
			}
			if (pHIER->field_0x90 != 0) {
				//FUN_00294600((TextureData_HIER_Internal*)(pHIER->field_0x90 + 0x10), pTextureInfo);
			}
			iVar4 = 1;)
		}
		else {
			iVar4 = 0;
		}
		return iVar4;
	}

	void LoadCSTA(MeshInfo* pMeshInfo, TextureInfo* pTextureInfo)
	{
		char* pcVar1;
		uint uVar2;
		char* pcVar3;
		uint uVar4;

		pcVar1 = pMeshInfo->HALL;
		pcVar3 = pcVar1 + 0x20;
		uVar2 = GetElementCount_002947b0(pcVar1 + 0x10, pcVar1 + *(int*)(pcVar1 + 8));
		for (uVar4 = 0; uVar4 < (uVar2 & 0xffff) - 1; uVar4 = uVar4 + 1) {
			FUN_00294600((TextureData_HIER_Internal*)(((char*)LOAD_SECTION(*(int*)(pcVar3 + 8))) + 0x10), pTextureInfo);
			pcVar3 = pcVar3 + 0x10;
		}
		return;
	}

	void ProcessMeshAndTexture(bool param_1, MeshInfo* meshInfoObj, TextureInfo* textureInfoObj, int unknown)
	{
		char* pAlloc;

		if (textureInfoObj != (TextureInfo*)0x0) {
			LoadMBNA(meshInfoObj->MBNA, textureInfoObj);
		}
		if (meshInfoObj->CSTA == (char*)0x0) {
			LoadCSTA(meshInfoObj, textureInfoObj);
			pAlloc = meshInfoObj->field_0x4;
		}
		else {
			ProcessCSTA(param_1, meshInfoObj, textureInfoObj, unknown);
			pAlloc = meshInfoObj->field_0x4;
		}
		if (pAlloc != (char*)0x0) {
			edMemFree(pAlloc);
			meshInfoObj->fileLengthB = meshInfoObj->fileLengthB + -meshInfoObj->field_0xc;
			meshInfoObj->field_0xc = 0;
			meshInfoObj->field_0x4 = (char*)0x0;
			meshInfoObj->GEOM = (char*)0x0;
		}
		return;
	}

	MeshInfo* LoadMeshFromBuffer(char* pFileData, int fileLength, ulong flags, int* outInt, TextureInfo* textureObj, int unknown, MeshInfo* pMeshInfo)
	{
		bool bVar1;
		int size;
		MeshInfo* pMVar2;
		int* piVar3;
		char* fileBuffer;
		MeshInfo* meshInfoObj;
		char* pcVar4;
		int size_00;

		meshInfoObj = pMeshInfo;
		pMVar2 = g_LoadMeshBuffer_004491c8;
		if (pMeshInfo == (MeshInfo*)0x0) {
			while (meshInfoObj = pMVar2, meshInfoObj->fileBufferStart != 0) {
				pMVar2 = meshInfoObj + 1;
			}
		}
		/* Check if the offset value from the buffer is '.G3D' */
		if (((meshInfoObj == (MeshInfo*)0x0) || (memset(meshInfoObj, 0, 0x40), pFileData == (char*)0x0)) ||
			(*(int*)(pFileData + 0xc) != 0x4433472e)) {
			bVar1 = meshInfoObj != pMeshInfo;
			meshInfoObj = (MeshInfo*)0x0;
			if (bVar1) {
				INT_004491b4 = INT_004491b4 + -1;
				meshInfoObj = (MeshInfo*)0x0;
			}
		}
		else {
			meshInfoObj->fileBufferStart = pFileData;
			meshInfoObj->fileLengthA = fileLength;
			meshInfoObj->field_0x4 = (char*)0x0;
			if ((*(uint*)(pFileData + 4) & 1) == 0) {
				pcVar4 = pFileData + 0x10;
				if ((flags & 2) == 0) {
					SetupMeshSubsections(meshInfoObj, pcVar4, fileLength + -0x10, pcVar4, fileLength + -0x10);
					IMPLEMENTATION_GUARD(); // This needs checking.
					edMemSetFlags(TO_HEAP(H_MAIN), 0x100);
					size = *(int*)(meshInfoObj->GEOM + 8);
					size_00 = size + *(int*)(meshInfoObj->GEOM + size + 8);
					fileBuffer = (char*)edMemAlloc(TO_HEAP(H_MAIN), size_00);
					memcpy(fileBuffer, (void*)meshInfoObj->GEOM, size_00);
					edMemClearFlags(TO_HEAP(H_MAIN), 0x100);
					*outInt = (fileLength - size_00) - fileLength;
					piVar3 = SetTextureSubsectionPointers(fileBuffer, size_00, pcVar4, (fileLength - size_00) + -0x10);
					if (piVar3 != (int*)0x0) {
						edMemShrink(fileBuffer, size);
						*(int*)(pFileData + 8) = *(int*)(pFileData + 8) - size_00;
					}
					meshInfoObj->GEOM = fileBuffer;
					meshInfoObj->field_0x4 = fileBuffer;
					meshInfoObj->field_0xc = size_00;
					meshInfoObj->fileLengthB = meshInfoObj->fileLengthA;
					meshInfoObj->fileLengthA = meshInfoObj->fileLengthA - size_00;
				}
				else {
					pcVar4 = pFileData + 0x10;
					SetupMeshSubsections(meshInfoObj, pcVar4, fileLength + -0x10, pcVar4, fileLength + -0x10);
					piVar3 = SetTextureSubsectionPointers(pcVar4, fileLength + -0x10, pcVar4, fileLength + -0x10);
					if (piVar3 != (int*)0x0) {
						IMPLEMENTATION_GUARD(); // This needs checking.
						*outInt = -piVar3[2];
						*(int*)(pFileData + 8) = *(int*)(pFileData + 8) - piVar3[2];
					}
					meshInfoObj->fileLengthB = fileLength;
				}
				ProcessMeshAndTexture(1, meshInfoObj, textureObj, (long)unknown);
			}
			else {
				pcVar4 = (char*)(pFileData + 0x10);
				SetupMeshSubsections(meshInfoObj, pcVar4, fileLength + -0x10, pcVar4, fileLength + -0x10);
				SetTextureSubsectionPointers(pcVar4, fileLength + -0x10, pcVar4, fileLength + -0x10);
				meshInfoObj->GEOM = (char*)0x0;
				meshInfoObj->field_0x4 = (char*)0x0;
				meshInfoObj->field_0xc = 0;
				meshInfoObj->fileLengthB = meshInfoObj->fileLengthA;
				ProcessMeshAndTexture(1, meshInfoObj, textureObj, unknown);
			}
		}
		return meshInfoObj;
	}

	int INT_004493a8 = 0;

	PACK( struct MeshData_ANHR {
		char field_0x0[4];
		undefined field_0x4;
		undefined field_0x5;
		undefined field_0x6;
		undefined field_0x7;
		uint field_0x8;
		uint field_0xc;
	});

	union ObbFloat
	{
		int pObb_Internal;
		float number;
	};

	PACK( struct astruct_16 {
		int field_0x0;
		int field_0x4;
		uint field_0x8;
		uint field_0xc;
		undefined field_0x10;
		undefined field_0x11;
		undefined field_0x12;
		undefined field_0x13;
		undefined field_0x14;
		undefined field_0x15;
		undefined field_0x16;
		undefined field_0x17;
		undefined field_0x18;
		undefined field_0x19;
		undefined field_0x1a;
		undefined field_0x1b;
		float field_0x1c;
		uint fileCount;
		undefined field_0x24;
		undefined field_0x25;
		undefined field_0x26;
		undefined field_0x27;
		float field_0x28;
		float field_0x2c;
		ObbFloat field_0x30;
		float field_0x34;
	});

	PACK(struct MeshData_HALL {
		char header[4];
		undefined field_0x4;
		undefined field_0x5;
		undefined field_0x6;
		undefined field_0x7;
		int totalSize;
		undefined field_0xc;
		undefined field_0xd;
		undefined field_0xe;
		undefined field_0xf;
	};)

	void SetupHIER_002abc00(MeshInfo* pMeshInfo)
	{
		uint uVar1;
		uint uVar2;
		ulong* pMVar3;
		MeshData_HALL* pcVar1;

		pcVar1 = (MeshData_HALL*)pMeshInfo->HALL;
		pMVar3 = (ulong*)(pcVar1 + 2);
		uVar1 = GetElementCount_002947b0((char*)(pcVar1 + 1), (char*)pcVar1 + pcVar1->totalSize);
		for (uVar2 = 0; uVar2 < (uVar1 & 0xffff) - 1; uVar2 = uVar2 + 1) {
			TextureData_HIER* p = (TextureData_HIER*)LOAD_SECTION(*(pMVar3 + 1));
			p->body.hash.number = *pMVar3;
			pMVar3 = pMVar3 + 2;
		}
		return;
	}

	int g_LoadedStaticMeshCount_004493a4 = 0;

	MeshTransformParent* SetupFromFile_002ab920
		(CameraPanMasterHeader* pCameraPanMasterHeader, MeshTransformData* pSubSubObjArray,
			MeshTransformParent* pCameraPanStaticArray, TextureData_HIER_Internal* pHIER_Internal, MeshTransformData* param_5)

	{
		MeshTransformParent* pMVar1;
		undefined8 uVar2;
		MeshTransformData* pCVar8;
		float fVar3;
		float fVar4;
		int iVar5;
		Matrix* pMVar6;
		MeshTransformObjData_Packed* pfVar7;
		MeshTransformData* pMVar8;
		MeshTransformObjData* pcVar9;
		Matrix* pMVar10;
		Matrix* pMVar11;
		Matrix* pTVar12;
		uint uVar13;
		MeshTransformData* pMVar14;
		ushort* puVar15;
		MeshTransformParent* pCVar1;
		MeshTransformParentHeader* pCameraPanHeader;

		MY_LOG("SetupFromFile_002ab920 %s\n", pHIER_Internal->hash.name);

		iVar5 = 4;
		pCameraPanHeader = pCameraPanMasterHeader->pCameraPanHeader_0xc;
		pMVar1 = pCameraPanMasterHeader->pCameraPanStatic_0x8;
		pCameraPanHeader->usedCount = pCameraPanHeader->usedCount + 1;
		pCameraPanMasterHeader->count_0x14 = pCameraPanMasterHeader->count_0x14 + 1;
		pCVar1 = (MeshTransformParent*)pCameraPanHeader->field_0x4;
		uVar13 = (uint)((ulong)pCVar1 - (ulong)pCameraPanStaticArray) >> 4;
		pCameraPanHeader->field_0x4 = (MeshTransformSpecial*)pCVar1->pNext;
		pMVar14 = pSubSubObjArray + uVar13;
		pCVar1->bLoaded = 0;
		pCVar1->pLink_0x8 = pMVar1;
		pCVar1->pNext = pMVar1->pNext;
		pMVar1->pNext->pLink_0x8 = pCVar1;
		pMVar1->pNext = pCVar1;
		g_LoadedStaticMeshCount_004493a4 = g_LoadedStaticMeshCount_004493a4 + 1;
		pTVar12 = &pHIER_Internal->transformA;
		pMVar8 = pMVar14;

		(pMVar8->base).transformA = *pTVar12;
		
		pMVar10 = &pHIER_Internal->transformB;
		pMVar6 = &pSubSubObjArray[uVar13].base.transformB;
		iVar5 = 4;

		pMVar11 = pMVar6;

		*pMVar11 = *pMVar10;
		pCVar8 = pSubSubObjArray + uVar13;
		puVar15 = &(pCVar8->base).flags_0x9e;
		pMVar11 = &g_IdentityMatrix;
		iVar5 = 8;
		(pCVar8->base).hash = pHIER_Internal->hash;
		(pCVar8->base).field_0x88 = pHIER_Internal->field_0x88;
		(pCVar8->base).field_0x89 = pHIER_Internal->field_0x89;
		(pCVar8->base).bRenderShadow = pHIER_Internal->bRenderShadow;
		(pCVar8->base).pShadowAnimMatrix = (Matrix*)LOAD_SECTION(pHIER_Internal->pShadowAnimMatrix);
		(pCVar8->base).pLinkTransformData = (MeshTransformData*)LOAD_SECTION(pHIER_Internal->pLinkTransformData);
		(pCVar8->base).field_0x94 = (undefined*)LOAD_SECTION(pHIER_Internal->field_0x94);
		(pCVar8->base).pTextureInfo = (undefined*)LOAD_SECTION(pHIER_Internal->pTextureInfo);
		(pCVar8->base).count_0x9c = pHIER_Internal->count_0x9c;
		(pCVar8->base).flags_0x9e = pHIER_Internal->flags_0x9e;
		(pCVar8->base).pLightingMatrixFuncObj_0xa0 = (LightingMatrixFuncObj*)LOAD_SECTION(pHIER_Internal->pLightingMatrixFuncObj_0xa0);
		(pCVar8->base).field_0xa4 = (Matrix*)LOAD_SECTION(pHIER_Internal->field_0xa4);
		(pCVar8->base).pAnimMatrix = (Matrix*)LOAD_SECTION(pHIER_Internal->pAnimMatrix);
		(pCVar8->base).subMeshParentCount_0xac = pHIER_Internal->subMeshParentCount_0xac;
		(pCVar8->base).size_0xae = pHIER_Internal->size_0xae;
		(pCVar8->base).field_0xaf = *(char*)&pHIER_Internal->field_0xaf;

		*pMVar6 = *pMVar11;

		iVar5 = 0;
		do {
			if (iVar5 < (int)(uint)pHIER_Internal->count_0x9c) {
				pfVar7 = pHIER_Internal->aSubArray + iVar5;
				pcVar9 = pMVar14->aSubArray + iVar5;
				pcVar9->pObj = (char*)LOAD_SECTION(pfVar7->pObj);
				pcVar9->field_0x4 = pfVar7->field_0x4;
				pcVar9->field_0x6 = pfVar7->field_0x6;
			}
			else {
				memset(pMVar14->aSubArray + iVar5, 0, sizeof(MeshTransformObjData));
			}
			iVar5 = iVar5 + 1;
		} while (iVar5 < 4);

		(pCVar8->base).bRenderShadow = 0;
		(pCVar8->base).field_0xaf = -1;
		(pCVar8->base).pShadowAnimMatrix = (Matrix*)0x0;
		(pCVar8->base).pLinkTransformData = param_5;
		(pCVar8->base).field_0x94 = (undefined*)param_5;
		pMVar8 = pSubSubObjArray + uVar13;
		if ((pCVar8->base).subMeshParentCount_0xac == 0) {
			*puVar15 = *puVar15 | 8;
		}
		(pMVar8->base).flags_0x9e = (pMVar8->base).flags_0x9e & 0xfffc;
		(pMVar8->base).flags_0x9e = (pMVar8->base).flags_0x9e | 0x800;
		SetField_00295df0(pMVar8, 1);
		pCVar1->pMeshTransformData = (DisplayListInternalMesh*)pMVar8;
		return pCVar1;
	}


	void SetupFromFile_002ad020
	(CameraPanMasterHeader* pCameraPanMasterHeader, MeshTransformData* pSubSubObjArray,
		MeshTransformParent* pCameraPanStaticArray, TextureData_HIER* pParent, MeshTransformParent* param_5, char* param_6,
		uint count)
	{
		int iVar2;
		int iVar3;
		int iVar4;
		int iVar5;
		int iVar6;
		int iVar7;
		int iVar8;
		MeshTransformParent* pMVar9;
		MeshTransformParent* pMVar10;
		MeshTransformParent* pMVar11;
		MeshTransformParent* pMVar12;
		MeshTransformParent* pMVar13;
		MeshTransformParent* pMVar14;
		MeshTransformParent* pMVar15;
		MeshTransformParent* pMVar16;
		uint uVar17;
		MeshTransformParent* pMVar18;
		char* pcVar19;
		char* local_220;
		MeshTransformParent* local_204;
		uint local_200;
		char* local_1d0;
		MeshTransformParent* local_1b4;
		uint local_1b0;
		char* local_180;
		MeshTransformParent* local_164;
		uint local_160;
		char* local_130;
		MeshTransformParent* local_114;
		uint local_110;
		char* local_e0;
		MeshTransformParent* local_c4;
		uint local_c0;
		char* local_90;
		MeshTransformParent* local_74;
		uint local_70;
		char* local_40;
		MeshTransformParent* local_24;
		uint local_20;
		TextureData_HIER* iVar1;

		pMVar18 = (MeshTransformParent*)0x0;
		pcVar19 = param_6;
		for (uVar17 = 0; uVar17 < count; uVar17 = uVar17 + 1) {
			iVar1 = (TextureData_HIER*)LOAD_SECTION(*(int*)(pcVar19 + 8));
			if ((TextureData_HIER*)(iVar1->body).pLinkTransformData == pParent) {
				IMPLEMENTATION_GUARD();
				pMVar9 = SetupFromFile_002ab920
				(pCameraPanMasterHeader, pSubSubObjArray, pCameraPanStaticArray, &iVar1->body,
					(MeshTransformData*)param_5->pMeshTransformData);
				local_24 = (MeshTransformParent*)0x0;
				local_40 = param_6;
				for (local_20 = 0; local_20 < count; local_20 = local_20 + 1) {
					iVar2 = *(int*)(local_40 + 8);
					if (*(TextureData_HIER**)(iVar2 + 0xa0) == iVar1) {
						pMVar10 = SetupFromFile_002ab920
						(pCameraPanMasterHeader, pSubSubObjArray, pCameraPanStaticArray,
							(TextureData_HIER_Internal*)(iVar2 + 0x10),
							(MeshTransformData*)pMVar9->pMeshTransformData);
						local_74 = (MeshTransformParent*)0x0;
						local_90 = param_6;
						for (local_70 = 0; local_70 < count; local_70 = local_70 + 1) {
							iVar3 = *(int*)(local_90 + 8);
							if (*(int*)(iVar3 + 0xa0) == iVar2) {
								pMVar11 = SetupFromFile_002ab920
								(pCameraPanMasterHeader, pSubSubObjArray, pCameraPanStaticArray,
									(TextureData_HIER_Internal*)(iVar3 + 0x10),
									(MeshTransformData*)pMVar10->pMeshTransformData);
								local_c4 = (MeshTransformParent*)0x0;
								local_e0 = param_6;
								for (local_c0 = 0; local_c0 < count; local_c0 = local_c0 + 1) {
									iVar4 = *(int*)(local_e0 + 8);
									if (*(int*)(iVar4 + 0xa0) == iVar3) {
										pMVar12 = SetupFromFile_002ab920
										(pCameraPanMasterHeader, pSubSubObjArray, pCameraPanStaticArray,
											(TextureData_HIER_Internal*)(iVar4 + 0x10),
											(MeshTransformData*)pMVar11->pMeshTransformData);
										local_114 = (MeshTransformParent*)0x0;
										local_130 = param_6;
										for (local_110 = 0; local_110 < count; local_110 = local_110 + 1) {
											iVar5 = *(int*)(local_130 + 8);
											if (*(int*)(iVar5 + 0xa0) == iVar4) {
												pMVar13 = SetupFromFile_002ab920
												(pCameraPanMasterHeader, pSubSubObjArray, pCameraPanStaticArray,
													(TextureData_HIER_Internal*)(iVar5 + 0x10),
													(MeshTransformData*)pMVar12->pMeshTransformData);
												local_164 = (MeshTransformParent*)0x0;
												local_180 = param_6;
												for (local_160 = 0; local_160 < count; local_160 = local_160 + 1) {
													iVar6 = *(int*)(local_180 + 8);
													if (*(int*)(iVar6 + 0xa0) == iVar5) {
														pMVar14 = SetupFromFile_002ab920
														(pCameraPanMasterHeader, pSubSubObjArray, pCameraPanStaticArray,
															(TextureData_HIER_Internal*)(iVar6 + 0x10),
															(MeshTransformData*)pMVar13->pMeshTransformData);
														local_1b4 = (MeshTransformParent*)0x0;
														local_1d0 = param_6;
														for (local_1b0 = 0; local_1b0 < count; local_1b0 = local_1b0 + 1) {
															iVar7 = *(int*)(local_1d0 + 8);
															if (*(int*)(iVar7 + 0xa0) == iVar6) {
																pMVar15 = SetupFromFile_002ab920
																(pCameraPanMasterHeader, pSubSubObjArray, pCameraPanStaticArray,
																	(TextureData_HIER_Internal*)(iVar7 + 0x10),
																	(MeshTransformData*)pMVar14->pMeshTransformData);
																local_204 = (MeshTransformParent*)0x0;
																local_220 = param_6;
																for (local_200 = 0; local_200 < count; local_200 = local_200 + 1) {
																	iVar8 = *(int*)(local_220 + 8);
																	if (*(int*)(iVar8 + 0xa0) == iVar7) {
																		pMVar16 = SetupFromFile_002ab920
																		(pCameraPanMasterHeader, pSubSubObjArray, pCameraPanStaticArray,
																			(TextureData_HIER_Internal*)(iVar8 + 0x10),
																			(MeshTransformData*)pMVar15->pMeshTransformData);
																		IMPLEMENTATION_GUARD();
																		SetupFromFile_002ad020
																		(pCameraPanMasterHeader, pSubSubObjArray, pCameraPanStaticArray, (TextureData_HIER*)LOAD_SECTION(iVar8), pMVar16,
																			param_6, count);
																		if (local_204 != (MeshTransformParent*)0x0) {
																			local_204 = pMVar16;
																		}
																	}
																	local_220 = local_220 + 0x10;
																}
																if (local_1b4 != (MeshTransformParent*)0x0) {
																	local_1b4 = pMVar15;
																}
															}
															local_1d0 = local_1d0 + 0x10;
														}
														if (local_164 != (MeshTransformParent*)0x0) {
															local_164 = pMVar14;
														}
													}
													local_180 = local_180 + 0x10;
												}
												if (local_114 != (MeshTransformParent*)0x0) {
													local_114 = pMVar13;
												}
											}
											local_130 = local_130 + 0x10;
										}
										if (local_c4 != (MeshTransformParent*)0x0) {
											local_c4 = pMVar12;
										}
									}
									local_e0 = local_e0 + 0x10;
								}
								if (local_74 != (MeshTransformParent*)0x0) {
									local_74 = pMVar11;
								}
							}
							local_90 = local_90 + 0x10;
						}
						if (local_24 != (MeshTransformParent*)0x0) {
							local_24 = pMVar10;
						}
					}
					local_40 = local_40 + 0x10;
				}
				if (pMVar18 != (MeshTransformParent*)0x0) {
					pMVar18 = pMVar9;
				}
			}
			pcVar19 = pcVar19 + 0x10;
		}
		return;
	}

	void ProcessLinks_002ac980(MeshTransformParent* pMeshTransformParent, short* outMeshCount)
	{
		ushort uVar1;
		ulong uVar2;
		MeshTransformData* pDVar4;
		ulong uVar3;
		short local_2;
		MeshTransformData* pDVar1;

		local_2 = 0;
		pDVar1 = (MeshTransformData*)pMeshTransformParent->pMeshTransformData;
		pDVar4 = (MeshTransformData*)pMeshTransformParent->pNext->pMeshTransformData;
		if ((pDVar4->base).pLinkTransformData == pDVar1) {
			IMPLEMENTATION_GUARD();
			uVar1 = 0;
			uVar3 = (ulong)pMeshTransformParent->pNext;
			while (uVar2 = (ulong)uVar1, (pDVar4->base).pLinkTransformData == pDVar1) {
				ProcessLinks_002ac980((MeshTransformParent*)uVar3, &local_2);
				uVar1 = uVar1 + local_2;
				uVar3 = uVar2;
				pDVar4 = *(MeshTransformData**)((ulong)uVar2 + 0xc);
			}
			(pDVar1->base).subMeshParentCount_0xac = uVar1;
			*outMeshCount = uVar1 + 1;
		}
		else {
			(pDVar1->base).subMeshParentCount_0xac = 0;
			*outMeshCount = 1;
		}
		return;
	}

	void RotationVectorToMatrix(Vector* v0, Matrix* m0)
	{
		float fVar1;
		float fVar2;
		float fVar3;
		float fVar4;
		float fVar5;
		float fVar6;
		float fVar7;
		float fVar8;
		float x;
		float y;
		float z;
		float w;

		x = v0->x;
		y = v0->y;
		z = v0->z;
		w = v0->w;
		fVar1 = (y + 0.0) * (y + 0.0) + (z + 0.0) * (z + 0.0);
		fVar7 = (x + 0.0) * (z + 0.0) + (w + 0.0) * (y + 0.0);
		fVar3 = (x + 0.0) * (y + 0.0) - (w + 0.0) * (z + 0.0);
		fVar2 = (x + 0.0) * (y + 0.0) + (w + 0.0) * (z + 0.0);
		fVar5 = (x + 0.0) * (x + 0.0) + (z + 0.0) * (z + 0.0);
		fVar6 = (y + 0.0) * (z + 0.0) - (w + 0.0) * (x + 0.0);
		fVar4 = (y + 0.0) * (z + 0.0) + (w + 0.0) * (x + 0.0);
		fVar8 = (x + 0.0) * (x + 0.0) + (y + 0.0) * (y + 0.0);
		x = (x + 0.0) * (z + 0.0) - (w + 0.0) * (y + 0.0);
		m0->aa = (0.0 - (fVar1 + fVar1)) + 1.0;
		m0->ab = fVar3 + fVar3;
		m0->ac = fVar7 + fVar7;
		m0->ad = 0.0;
		m0->ba = fVar2 + fVar2;
		m0->bb = (0.0 - (fVar5 + fVar5)) + 1.0;
		m0->bc = fVar6 + fVar6;
		m0->bd = 0.0;
		m0->ca = x + x;
		m0->cb = fVar4 + fVar4;
		m0->cc = (0.0 - (fVar8 + fVar8)) + 1.0;
		m0->cd = 0.0;
		m0->da = 1.0;
		m0->db = 0.0;
		m0->dc = 0.0;
		m0->dd = 0.0;
		return;
	}

	uint MatrixSetupFunc_001fc820
	(float param_1, MeshData_ANHR** param_2, MeshTransformData* param_3, int* pFileData, int param_5)
	{
		int iVar1;
		int iVar2;
		int iVar3;
		float* pfVar4;
		float* pfVar5;
		float* pfVar6;
		float* pfVar7;
		Vector* pVVar8;
		float* pfVar9;
		Vector* pVVar10;
		float fVar11;
		float fVar12;
		float fVar13;
		float fVar14;
		float fVar15;
		float fVar16;
		float fVar17;
		float fVar18;
		uint local_50;
		Vector local_30;
		Vector VStack32;
		int local_10;
		int local_c;
		float local_8;

		pfVar5 = (float*)(pFileData + 3);
		iVar1 = *pFileData;
		fVar14 = 0.0f;
		iVar2 = pFileData[1];
		iVar3 = pFileData[2];
		pfVar4 = pfVar5 + iVar1 + 1;
		local_50 = 0;
		pfVar6 = pfVar4 + iVar2 + 1;
		pfVar9 = (float*)((ulong)pfVar6 + (iVar3 + 1) * 4 + 0xf); // &0xfffffff0);
		fVar13 = pfVar5[iVar1];
		pVVar10 = (Vector*)(pfVar9 + iVar1 * 4);
		pVVar8 = pVVar10 + iVar2;
		if (0.0f < fVar13) {
			if (param_5 == 0) {
				fVar14 = param_1;
				if (fVar13 - 1e-06f < param_1) {
					local_50 = 1;
					fVar14 = fVar13 - 1e-06f;
				}
			}
			else {
				fVar14 = fmodf(param_1, fVar13);
			}
		}
		if (iVar2 == 0) {
			sceVu0UnitMatrix((TO_SCE_MTX)param_3);
		}
		else {
			pfVar7 = pfVar4;
			if (iVar2 == 1) {
				RotationVectorToMatrix(pVVar10, (Matrix*)param_3);
			}
			else {
				for (; *pfVar7 <= fVar14; pfVar7 = pfVar7 + 1) {
				}
				pfVar4 = (float*)((int)pfVar7 - (int)pfVar4);
				if ((int)pfVar4 < 0) {
					pfVar4 = (float*)((int)pfVar4 + 3);
				}
				local_c = (int)pfVar4 >> 2;
				local_10 = local_c + -1;
				local_8 = (fVar14 - pfVar7[-1]) / (*pfVar7 - pfVar7[-1]);
				if (iVar2 == local_c) {
					local_c = 0;
				}
				IMPLEMENTATION_GUARD(
				InterpolateRotation(local_8, &VStack32, pVVar10 + local_10, pVVar10 + local_c);
				RotationVectorToMatrix(&VStack32, (Matrix*)param_3);)
			}
		}
		if (iVar1 != 0) {
			pfVar4 = pfVar5;
			if (iVar1 == 1) {
				fVar12 = pfVar9[1];
				fVar13 = pfVar9[2];
				fVar11 = pfVar9[3];
				(param_3->base).transformA.da = *pfVar9;
				(param_3->base).transformA.db = fVar12;
				(param_3->base).transformA.dc = fVar13;
				(param_3->base).transformA.dd = fVar11;
			}
			else {
				for (; *pfVar4 <= fVar14; pfVar4 = pfVar4 + 1) {
				}
				pfVar5 = (float*)((int)pfVar4 - (int)pfVar5);
				if ((int)pfVar5 < 0) {
					pfVar5 = (float*)((int)pfVar5 + 3);
				}
				local_c = (int)pfVar5 >> 2;
				local_10 = local_c + -1;
				local_8 = (fVar14 - pfVar4[-1]) / (*pfVar4 - pfVar4[-1]);
				if (iVar1 == local_c) {
					local_c = 0;
				}
				pfVar4 = pfVar9 + local_c * 4;
				pfVar9 = pfVar9 + local_10 * 4;
				fVar11 = pfVar9[1];
				fVar12 = pfVar9[2];
				fVar15 = pfVar9[3];
				fVar16 = pfVar4[1];
				fVar17 = pfVar4[2];
				fVar18 = pfVar4[3];
				fVar13 = 1.0 - local_8;
				(param_3->base).transformA.da = *pfVar4 * local_8 + *pfVar9 * fVar13;
				(param_3->base).transformA.db = fVar16 * local_8 + fVar11 * fVar13;
				(param_3->base).transformA.dc = fVar17 * local_8 + fVar12 * fVar13;
				(param_3->base).transformA.dd = fVar18 * local_8 + fVar15 * fVar13;
				(param_3->base).transformA.dd = 1.0;
			}
		}
		if (iVar3 != 0) {
			pfVar4 = pfVar6;
			if (iVar3 != 1) {
				for (; *pfVar4 <= fVar14; pfVar4 = pfVar4 + 1) {
				}
				pfVar6 = (float*)((int)pfVar4 - (int)pfVar6);
				if ((int)pfVar6 < 0) {
					pfVar6 = (float*)((int)pfVar6 + 3);
				}
				local_c = (int)pfVar6 >> 2;
				local_10 = local_c + -1;
				local_8 = (fVar14 - pfVar4[-1]) / (*pfVar4 - pfVar4[-1]);
				if (iVar3 == local_c) {
					local_c = 0;
				}
				pVVar10 = pVVar8 + local_c;
				pVVar8 = pVVar8 + local_10;
				fVar14 = 1.0 - local_8;
				local_30.x = pVVar10->x * local_8 + pVVar8->x * fVar14;
				local_30.y = pVVar10->y * local_8 + pVVar8->y * fVar14;
				local_30.z = pVVar10->z * local_8 + pVVar8->z * fVar14;
				local_30.w = pVVar10->w * local_8 + pVVar8->w * fVar14;
				pVVar8 = &local_30;
			}
			SomeIdentityMatrix.aa = pVVar8->x;
			SomeIdentityMatrix.bb = pVVar8->y;
			SomeIdentityMatrix.cc = pVVar8->z;
			sceVu0MulMatrix((TO_SCE_MTX)param_3, (TO_SCE_MTX)&SomeIdentityMatrix, (TO_SCE_MTX)param_3);
		}
		return local_50;
	}

	struct MeshData_OBB
	{
		char header[4];
		int field_0x4;
		int field_0x8;
		int field_0x12;
	};

	MeshData_OBB* SeekToOBB_002a15e0(MeshTransformParent* pMeshTransformParent, bool mode)
	{
		byte bVar1;
		MeshTransformData* pMeshTransformData;
		int* piVar2;
		int iVar3;
		MeshTransformObjData* pMVar4;
		char* pcVar5;
		MeshData_OBB* pMVar6;
		uint uVar7;
		ushort* puVar8;

		pMeshTransformData = (MeshTransformData*)pMeshTransformParent->pMeshTransformData;
		puVar8 = &(pMeshTransformData->base).count_0x9c;
		if (((pMeshTransformData->base).count_0x9c != 0) &&
			(pMVar4 = GetMeshOBJ_00295a80(pMeshTransformData), pMVar4 != (MeshTransformObjData*)0x0)) {
			bVar1 = (pMeshTransformData->base).size_0xae;
			uVar7 = (uint)bVar1;
			if ((mode != false) && (uVar7 = (uint)bVar1, ((pMeshTransformData->base).flags_0x9e & 0x100) != 0)) {
				IMPLEMENTATION_GUARD( MeshTransformData::Func_00295a50(pMeshTransformData, (long)(int)(*puVar8 - 1)));
				uVar7 = *puVar8 - 1;
			}
			pcVar5 = pMeshTransformData->aSubArray[uVar7].pObj;
			if (pcVar5 == (char*)0x0) {
				pcVar5 = pMeshTransformData->aSubArray[0].pObj;
			}
			/* Look for .obj */
			if ((pcVar5 != (char*)0x0) && (piVar2 = (int*)LOAD_SECTION(*(int*)(pcVar5 + 8)), *piVar2 == 0x2e4a424f)) {
				iVar3 = piVar2[2];
				for (pMVar6 = (MeshData_OBB*)
					VerifyFileBufferRead((char*)((ulong)piVar2 + piVar2[3]), (char*)(int*)((ulong)piVar2 + iVar3));
					pMVar6 != (MeshData_OBB*)0x0;
					pMVar6 = (MeshData_OBB*)GetNextFileSection((char*)pMVar6, (int*)((ulong)piVar2 + iVar3))) {
					/* Look for OBB */
					if (*(int*)pMVar6 == 0x4742424f) {
						return pMVar6;
					}
				}
			}
		}
		return (MeshData_OBB*)0x0;
	}

	int GetBufferOffset_00242b30(ushort** param_1, uint key)
	{
		ushort uVar1;
		int iVar2;
		int iVar3;
		uint uVar4;
		int iVar5;
		int iVar6;
		ushort* puVar7;

		uVar1 = **param_1;
		if (uVar1 == 0) {
			iVar3 = -1;
		}
		else {
			uVar4 = (uint)uVar1;
			iVar6 = uVar1 - 1;
			puVar7 = *param_1 + uVar4 * 2 + 2;
			iVar2 = uVar4 * 2;
			iVar5 = 0;
			iVar3 = iVar6;
			if (0 < iVar6) {
				do {
					iVar6 = (int)uVar4 >> 1;
					if (*(uint*)(puVar7 + iVar6 * 2) == key) {
						return *(int*)(puVar7 + iVar2 + iVar6 * 2);
					}
					if (*(uint*)(puVar7 + iVar6 * 2) <= key) {
						iVar5 = iVar6 + 1;
						iVar6 = iVar3;
					}
					uVar4 = iVar6 + iVar5;
					iVar3 = iVar6;
				} while (iVar5 < iVar6);
			}
			iVar3 = -1;
			if (key == *(uint*)(puVar7 + iVar6 * 2)) {
				iVar3 = *(int*)(puVar7 + iVar2 + iVar6 * 2);
			}
		}
		return iVar3;
	}

	void SetupANHR_001fd180(MeshData_ANHR** ppOutANHR, MeshData_ANHR* pInANHR, int length, MeshInfo* pMeshInfo,
		StaticMeshMaster* pStaticMeshMaster)
	{
		uint uVar1;
		MeshData_HALL* pcVar2;
		CameraPanMasterHeader* pCameraPanMasterHeader;
		MeshTransformData* pSubSubObjArray;
		MeshTransformParent* pCameraPanStaticArray;
		uint uVar3;
		char* pcVar4;
		MeshTransformParent* pMeshTransformParent;
		int iVar5;
		MeshData_OBB* pMVar6;
		ulong meshHashValue;
		int* pcVar8;
		float* pfVar7;
		uint* puVar8;
		uint uVar9;
		uint* puVar10;
		astruct_16* pMVar14;
		uint uVar11;
		uint uVar12;
		uint uVar13;
		float fVar14;
		float puVar15;
		float fVar16;
		float local_20;
		float local_1c;
		float local_18;
		ushort* local_8;
		short sStack2;
		MeshData_ANHR* pcVar1;

		if (pInANHR == (MeshData_ANHR*)0x0) {
			*ppOutANHR = (MeshData_ANHR*)0x0;
		}
		else {
			*ppOutANHR = pInANHR;
			pcVar1 = *ppOutANHR;
			uVar1 = pcVar1->field_0x8;
			uVar3 = pcVar1->field_0xc;
			pcVar8 = (int*)((char*)(pInANHR + 1) + uVar1 * 4);
			for (uVar9 = 0; uVar11 = 0, uVar9 < uVar3; uVar9 = uVar9 + 1) {
				pcVar8[uVar9] = (int)STORE_SECTION((char*)pInANHR + (int)pcVar8[uVar9]);
			}
			int* pBaseIndexes = (int*)(pInANHR + 1);
			for (; uVar11 < uVar1; uVar11 = uVar11 + 1) {
				pMVar14 = (astruct_16*)(pInANHR->field_0x0 + pBaseIndexes[uVar11]);
				pBaseIndexes[uVar11] = (int)STORE_SECTION(pMVar14);
				meshHashValue = Combine_00189a30((int)pMVar14->field_0x0, (int)pMVar14->field_0x4);
				SetupHIER_002abc00(pMeshInfo);
				pCameraPanStaticArray = PTR_MeshTransformParent_ARRAY_00449434;
				pSubSubObjArray = PTR_MeshTransformData_ARRAY_0044942c;
				pcVar2 = (MeshData_HALL*)pMeshInfo->HALL;
				pCameraPanMasterHeader = pStaticMeshMaster->pFinalLink_0x1a4;
				uVar3 = GetElementCount_002947b0((char*)(pcVar2 + 1), (char*)pcVar2 + pcVar2->totalSize);
				pcVar4 = FindSectionHash(meshHashValue, (MeshData_HASH*)(pMeshInfo->HALL + 0x10));
				if (pcVar4 == (char*)0x0) {
					iVar5 = 0;
				}
				else {
					iVar5 = *(int*)(pcVar4 + 8);
				}
				if (iVar5 == 0) {
					pMeshTransformParent = (MeshTransformParent*)0x0;
				}
				else {
					pMeshTransformParent =
						SetupFromFile_002ab920
						(pCameraPanMasterHeader, pSubSubObjArray, pCameraPanStaticArray, (TextureData_HIER_Internal*)(((char*)LOAD_SECTION(iVar5)) + 0x10),
							(MeshTransformData*)0x0);
					SetupFromFile_002ad020(pCameraPanMasterHeader, pSubSubObjArray, pCameraPanStaticArray, (TextureData_HIER*)LOAD_SECTION(iVar5), pMeshTransformParent,
						(char*)(pcVar2 + 2), (uVar3 & 0xffff) - 1);
					ProcessLinks_002ac980(pMeshTransformParent, &sStack2);
				}
				pMVar14->field_0x0 = (int)STORE_SECTION(pMeshTransformParent);
				if (pMeshTransformParent == (MeshTransformParent*)0x0) {
					pMVar14->field_0x4 = 0x0;
					pMVar14->field_0x8 = 0x0;
				}
				else {
					pMVar14->field_0x4 = (int)STORE_SECTION(pMeshTransformParent->pMeshTransformData);
					pMVar14->fileCount = pMVar14->fileCount | 0xa0000000;
					if ((pMVar14->fileCount & 1) == 0) {
						pMVar14->field_0x28 = 0.0;
					}
					else {
						fVar14 = pMVar14->field_0x28;
						fVar16 = pMVar14->field_0x2c;
						iVar5 = rand();
						pMVar14->field_0x28 = fVar14 + (fVar16 - fVar14) * ((float)iVar5 / 2.147484e+09f);
					}
					puVar15 = pMVar14->field_0x30.number;
					fVar14 = pMVar14->field_0x34;
					iVar5 = rand();
					pMVar14->field_0x2c = puVar15 + (fVar14 - puVar15) * ((float)iVar5 / 2.147484e+09f);
					if ((pMVar14->fileCount & 4) != 0) {
						IMPLEMENTATION_GUARD(MeshTransformParent::SetDataPtr_00295c00((MeshTransformData*)LOAD_SECTION(pMVar14->field_0x4), 0x40f070));
					}
					puVar10 = (uint*)LOAD_SECTION(pcVar8[(int)pMVar14->field_0x8]);
					MatrixSetupFunc_001fc820
					(pMVar14->field_0x28, ppOutANHR, (MeshTransformData*)LOAD_SECTION(pMVar14->field_0x4), (int*)puVar10,
						pMVar14->fileCount & 1);
					if (puVar10[2] == 0) {
						local_18 = 1.0;
						local_1c = 1.0;
						local_20 = 1.0;
					}
					else {
						pfVar7 = (float*)(((int)puVar10 + (puVar10[2] + 1) * 4 + (puVar10[1] + 1) * 4 + (*puVar10 + 1) * 4 + 0x1b &
							0xfffffff0) + *puVar10 * 0x10 + puVar10[1] * 0x10);
						local_20 = *pfVar7;
						local_1c = pfVar7[1];
						local_18 = pfVar7[2];
					}
					if (abs(local_20) <= abs(local_1c)) {
						local_20 = local_1c;
					}
					local_20 = abs(local_20);
					if (local_20 <= abs(local_18)) {
						local_20 = abs(local_18);
					}
					pMVar14->field_0x1c = pMVar14->field_0x1c * local_20 * 1.2;
					if (((*puVar10 < 2) && (puVar10[1] < 2)) && (puVar10[2] < 2)) {
						pMVar14->field_0x8 = 0x0;
					}
					else {
						pMVar14->field_0x8 = (int)STORE_SECTION(puVar10);
					}
					pMVar6 = SeekToOBB_002a15e0(pMeshTransformParent, 0);
					if (pMVar6 == (MeshData_OBB*)0x0) {
						pMVar14->field_0x30.pObb_Internal = 0x0;
					}
					else {
						puVar10 = (uint*)(pMVar14 + 1);
						pMVar14->field_0x30.pObb_Internal = (int)STORE_SECTION(pMVar6 + 1);
						local_8 = (ushort*)LOAD_SECTION(pMVar14->field_0x30.pObb_Internal);
						uVar3 = pMVar14->field_0xc;
						for (uVar9 = 0; uVar9 < uVar3; uVar9 = uVar9 + 1) {
							uVar12 = GetBufferOffset_00242b30(&local_8, *puVar10);
							*puVar10 = uVar12;
							puVar10[1] = STORE_SECTION(LOAD_SECTION(pcVar8[puVar10[1]]));
							puVar10 = puVar10 + 2;
						}
						puVar10 = (uint*)(pMVar14 + 1);
						for (uVar9 = 0; uVar9 < uVar3; uVar9 = uVar9 + 1) {
							puVar8 = puVar10;
							for (uVar12 = uVar9; (*puVar8 != uVar9 + 1 && (uVar12 < uVar3)); uVar12 = uVar12 + 1) {
								puVar8 = puVar8 + 2;
							}
							if (puVar8 != puVar10) {
								uVar12 = *puVar8;
								uVar13 = puVar8[1];
								*puVar8 = *puVar10;
								puVar8[1] = puVar10[1];
								*puVar10 = uVar12;
								puVar10[1] = uVar13;
							}
							puVar10 = puVar10 + 2;
						}
					}
				}
			}
		}
		return;
	}

	void Func_002a5540(StaticMeshMaster* pStaticMeshMaster, MeshInfo* pMeshInfo)
	{
		MeshTransformParentHeader* pMVar1;
		MeshTransformSpecial* pMVar2;
		DisplayListInternalMesh* pDVar3;
		uint uVar4;

		uVar4 = 0;
		while ((uVar4 < Params_00424e30.meshDisplayListInternalCount &&
			((g_MeshDisplayListInternal_00449380[uVar4].flags & 1) != 0))) {
			uVar4 = uVar4 + 1;
		}
		if (uVar4 < Params_00424e30.meshDisplayListInternalCount) {
			pDVar3 = g_MeshDisplayListInternal_00449380 + uVar4;
			INT_004493a8 = INT_004493a8 + 1;
			pDVar3->pMeshInfo = pMeshInfo;
			g_MeshDisplayListInternal_00449380[uVar4].flags = (g_MeshDisplayListInternal_00449380[uVar4].flags | 1);
			pMVar1 = (pStaticMeshMaster->headerB).pCameraPanHeader_0xc;
			pMVar2 = pMVar1->field_0x4;
			pMVar1->field_0x4 = pMVar2->pNext_0x4;
			pMVar2->pRenderInput = (DisplayListInternal*)pDVar3;
			pMVar1->usedCount = pMVar1->usedCount + 1;
			(pStaticMeshMaster->headerB).count_0x14 = (pStaticMeshMaster->headerB).count_0x14 + 1;
			pMVar2->pNext_0x4 = (pStaticMeshMaster->headerB).pLoadedData;
			(pStaticMeshMaster->headerB).pLoadedData = pMVar2;
		}
		return;
	}

	MeshTransformParent* SetupMeshTransform_001fffe0(StaticMeshMaster* pStaticMeshMaster, MeshInfo* pMeshInfo, ulong hash)
	{
		CameraPanMasterHeader* pCameraPanMasterHeader;
		MeshTransformData* pSubSubObjArray;
		MeshTransformParent* pCameraPanStaticArray;
		uint uVar1;
		char* pcVar2;
		MeshTransformParent* pMVar3;
		int iVar4;
		short sStack2;
		MeshData_HALL* pcVar1;

		SetupHIER_002abc00(pMeshInfo);
		pCameraPanStaticArray = PTR_MeshTransformParent_ARRAY_00449434;
		pSubSubObjArray = PTR_MeshTransformData_ARRAY_0044942c;
		pcVar1 = (MeshData_HALL*)pMeshInfo->HALL;
		pCameraPanMasterHeader = pStaticMeshMaster->pFinalLink_0x1a4;
		uVar1 = GetElementCount_002947b0((char*)(pcVar1 + 1), (char*)pcVar1 + pcVar1->totalSize);
		pcVar2 = FindSectionHash(hash, (MeshData_HASH*)(pMeshInfo->HALL + 0x10));
		iVar4 = 0;
		if (pcVar2 != (char*)0x0) {
			iVar4 = *(int*)(pcVar2 + 8);
		}
		if (iVar4 == 0) {
			pMVar3 = (MeshTransformParent*)0x0;
		}
		else {
			IMPLEMENTATION_GUARD(pMVar3 = CameraPanMasterHeader::Func_002ab920
			(pCameraPanMasterHeader, pSubSubObjArray, pCameraPanStaticArray, (undefined8*)(iVar4 + 0x10),
				(MeshTransformData*)0x0);
			CameraPanMasterHeader::Func_002ad020
			(pCameraPanMasterHeader, pSubSubObjArray, pCameraPanStaticArray, iVar4, (int)pMVar3, (int)(pcVar1 + 2),
				(uVar1 & 0xffff) - 1);
			MeshTransformParent::Func_002ac980(pMVar3, &sStack2);)
		}
		return pMVar3;
	}
}
