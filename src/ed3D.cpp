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
#include "renderer.h"
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
#include "Rendering/OneTimeCommands.h"
#include "MemoryStream.h"

#ifdef PLATFORM_WIN
#define ED3D_LOG(level, format, ...) Log::GetInstance().AddLog(level, "ed3D", format, ##__VA_ARGS__)
#else
#define ED3D_LOG(level, format, ...) scePrintf(format, ##__VA_ARGS__)
#endif

char* s_ed3D_Initialsation_004333a0 = "ed3D Initialsation\n";

ed3DConfig::ed3DConfig()
	: meshHeaderCountB(0x3E0)
	, sceneCount(0xF)
	, meshDisplayListInternalCount(0x10)
	, meshHeaderCountBAlt(0x2800)
	, matrixBufferCount(0x1F4)
	, materialBufferCount(0xFA)
	, field_0x18(0x7D0)
	, g3dManagerCount(0xA)
	, g2dManagerCount(0x19)
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

edF32MATRIX4 SomeIdentityMatrix = {
	0.6954f, 0.0f, 0.0f, 0.0f,
	0.0f, 0.6954, 0.0f, 0.0f,
	0.0f, 0.0f, 0.6954, 0.0f,
	0.0f, 0.0f, 0.0f, 1.0f
};

int gNbVertexDMA = 0;
ed_g2d_manager* gpG2D;

edpkt_data* g_pStrippBufLastPos;
edpkt_data* g_pStripBuf;
edpkt_data* g_pStrippBufMaxPos;
edpkt_data* g_pStripBufResetPos;

ed3DConfig ged3DConfig;

edSurface* gVRAMed3D = NULL;

ed_3D_Scene* gScene3D = NULL;
edNODE_MANAGER* gNodeCluster = NULL;

int gNbG2D = 0;
int gNbG3D = 0;


edpkt_data* g_GifRefPktCur = NULL;
edpkt_data* g_pGifStart = NULL;

int gFXBufAddr = 0;

edLIST gPrim_List_FlushTex[2] = { 0 };
edLIST gPrim_List_Shadow[2] = { 0 };

struct ed_3d_octree {
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
	edF32VECTOR3 field_0x0;
	ushort field_0xc;
	undefined field_0xe;
	undefined field_0xf;
}
);

PACK(
struct ed_3d_strip {
	uint flags_0x0;
	short field_0x4;
	short field_0x6;
	int offsetA;
	int pNext; // ed_3d_strip*
	struct Vector vector_0x10;
	int field_0x20; // char*
	int field_0x24; // char*
	int field_0x28; // char*
	int field_0x2c; // char*
	short bUseShadowMatrix_0x30;
	undefined field_0x32;
	undefined field_0x33;
	int pRenderFrame30; // ed_dma_matrix*
	byte field_0x38;
	byte cameraPanIndex;
	short meshSectionCount_0x3a;
	int pTextureDataMystery; // TextureData_MYSTERY*
});

MeshTransformDataBase* gpCurHierarchy = NULL;
undefined* pOldFANGifTag = NULL;

MeshTransformData* gHierarchyManagerBuffer = NULL;
edNODE_MANAGER* gHierarchyManagerNodeManager = NULL;
MeshTransformParent* gHierarchyManagerFirstFreeNode = NULL;

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
edpkt_data* gpPKTDataRefMasPath3 = NULL;

uint g_StaticVideoList_00489590[0xB20] = { 0 };
ulong* g_StaticVideoList_00489590_LONG = (ulong*)g_StaticVideoList_00489590;

edNODE_MANAGER* pNodeManRender = NULL;
edNODE_MANAGER* pNodeManMaterial = NULL;
edNODE_MANAGER* PTR_MeshTransformParentHeader_004494b4 = NULL;
edNODE_MANAGER* PTR_MeshTransformParentHeader_004494b8 = NULL;
edNODE_MANAGER* PTR_MeshTransformParentHeader_004494bc = NULL;

edLIST gPrim_List[15][2] = { 0 };

int gIDProfileRender = 0;
int gIDProfileFlush = 0;
bool bcalcFrame = false;
byte BYTE_00449424 = 0;
ed_viewport* gCurViewportUsed = NULL;
int gCurFrame = 0;

uint gZBUF_BASE = 0;

ed_g2d_material ed_g2d_material_0048c3b0 = { 0 };
ed_g2d_material ed_g2d_material_0048c3c0 = { 0 };
	
int GetStaticMeshMasterIndex(ed_3D_Scene* pStaticMeshMaster)
{
	int staticMeshMasterIndex = 0;
	/* Find inactive camera master. */
	while (staticMeshMasterIndex < (uint)ged3DConfig.sceneCount) {
		if (&gScene3D[staticMeshMasterIndex] == pStaticMeshMaster)
		{
			return staticMeshMasterIndex;
		}
		staticMeshMasterIndex = staticMeshMasterIndex + 1;
	}
	return -1;
}

struct RenderTaskData {
	struct ed_3D_Scene* pStaticMeshMaster;
	int isChild;
	ushort taskID;
};

char* edChunckGetFirst(char* pBuffStart, char* pBuffEnd)
{
	/* Checks that the end of the file is greater than the start of the file */
	if ((pBuffEnd != (char*)0x0) && (pBuffEnd <= pBuffStart)) {
		pBuffStart = (char*)0x0;
	}
	return pBuffStart;
}

int* edChunckGetNext(char* param_1, int* param_2)
{
	int* piVar1;

	if ((param_2 == (int*)0x0) || (piVar1 = (int*)0x0, (int*)(param_1 + *(int*)(param_1 + 8)) < param_2)) {
		piVar1 = (int*)(param_1 + *(int*)(param_1 + 8));
	}
	return piVar1;
}

edSurface* edVideoGetDrawSurface(void)
{
	edSurface* pFVar1;

	if (g_ActiveVidParams_0048cd90.pFrameBuffer == (edSurface*)0x0) {
		pFVar1 = (edSurface*)0x0;
	}
	else {
		pFVar1 = (g_ActiveVidParams_0048cd90.pFrameBuffer)->pSurfaceDesc->pLink_0xc;
	}
	return pFVar1;
}

uint gRefOptionsforVU1Buf[12] = { 0 };
ulong* g_StaticDisplayListData_0048c340_LONG = (ulong*)&gRefOptionsforVU1Buf;

Vector Vector_00424ff0 = { 0 };

int INT_004489a4 = 0;
int INT_004489a8 = 0;

uint ed3DVU1BufferCur = 0;
uint ed3DVU1Buffer[3] = { 0 };
byte BYTE_004493b4 = 0;
byte BYTE_004493b8 = 0;

edpkt_data RenderCommand_ARRAY_0048c5f0[3] = {0};

void ed3DOptionFlagCNTInit(void)
{
	RenderCommand_ARRAY_0048c5f0[0].cmdB = (ulong)ed3DVU1Buffer[0] << 0x20 | 0x7c01000000000000;
	RenderCommand_ARRAY_0048c5f0[1].cmdB = (ulong)ed3DVU1Buffer[1] << 0x20 | 0x7c01000000000000;
	RenderCommand_ARRAY_0048c5f0[2].cmdB = (ulong)ed3DVU1Buffer[2] << 0x20 | 0x7c01000000000000;
	RenderCommand_ARRAY_0048c5f0[0].cmdA = 0xe10000001;
	RenderCommand_ARRAY_0048c5f0[1].cmdA = 0xe10000001;
	RenderCommand_ARRAY_0048c5f0[2].cmdA = 0xe10000001;
	return;
}

void ed3DDMAGenerateHeaders(void)
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
	ed3DVU1Buffer[0] = 0x76;
	g_StaticVideoList_00489590[28] = 0x01000103;
	ed3DVU1Buffer[1] = 0x198;
	g_StaticVideoList_00489590[29] = 0x20000000;
	ed3DVU1Buffer[2] = 0x2ba;
	g_StaticVideoList_00489590[30] = 0xffffff00;
	ed3DVU1BufferCur = 1;
	g_StaticVideoList_00489590[31] = 0;
	gRefOptionsforVU1Buf[1] = 0x200000003000077;
	gRefOptionsforVU1Buf[3] = 0x200000003000199;
	gRefOptionsforVU1Buf[5] = 0x2000000030002bb;
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
	ed3DOptionFlagCNTInit();
	return;
}

uint ged3DVramConfig = 0;

uint gVRAMBuffers[2] = { 0 };
uint gVRAMClutBuffers[2] = { 0 };
uint UINT_ARRAY_00449320[2] = { 0 };
uint UINT_ARRAY_00449328[2] = { 0 };
uint UINT_ARRAY_00489434[2] = { 0 };
uint UINT_ARRAY_00489444[2] = { 0 };
uint UINT_ARRAY_0048943c[2] = { 0 };
uint UINT_ARRAY_0048944c[2] = { 0 };

int GetedVidBppFromMode(int psm)
{
	int EVar1;

	EVar1 = SCE_GS_PSMCT16;
	if (psm != SCE_GS_PSMT8) {
		if (psm == SCE_GS_PSMT4) {
			EVar1 = SCE_GS_PSMCT16;
		}
		else {
			if ((((psm == SCE_GS_PSMZ16S) || (psm == SCE_GS_PSMZ16)) || (psm == SCE_GS_PSMCT16S)) || (psm == SCE_GS_PSMCT16))
			{
				EVar1 = SCE_GS_PSMCT16;
			}
			else {
				EVar1 = SCE_GS_PSMCT24;
				if ((psm != SCE_GS_PSMZ32) && (psm != SCE_GS_PSMCT32)) {
					if ((psm == SCE_GS_PSMZ24) || (psm == SCE_GS_PSMCT24)) {
						EVar1 = SCE_GS_PSMCT24;
					}
					else {
						EVar1 = SCE_GS_PSMCT32;
					}
				}
			}
		}
	}
	return EVar1;
}

int GetBppFromModeInVRAM(int psm)
{
	int iVar1;

	iVar1 = 1;
	if (psm != SCE_GS_PSMT8) {
		if (psm == SCE_GS_PSMT4) {
			iVar1 = 1;
		}
		else {
			if ((((psm == SCE_GS_PSMZ16S) || (psm == SCE_GS_PSMZ16)) || (psm == SCE_GS_PSMCT16S)) || (psm == SCE_GS_PSMCT16))
			{
				iVar1 = 2;
			}
			else {
				iVar1 = 4;
				if ((psm != SCE_GS_PSMZ32) && (psm != SCE_GS_PSMCT32)) {
					if ((psm == SCE_GS_PSMZ24) || (psm == SCE_GS_PSMCT24)) {
						iVar1 = 4;
					}
					else {
						iVar1 = 0;
					}
				}
			}
		}
	}
	return iVar1;
}

void ed3DPrepareVRAMBuf(void)
{
	int iVar1;
	uint uVar2;
	int iVar3;
	ed_surface_desc vidModeData;

	vidModeData.screenWidth = 0x200;
	vidModeData.screenHeight = 0x200;
	vidModeData.field_0x6 = 2;
	vidModeData.frameBufferCount = 1;
	vidModeData.flags_0x8 = 2;
	vidModeData.pixelStoreMode = SCE_GS_PSMCT32;
	vidModeData.bUseGlobalFrameBuffer = false;
	gVRAMed3D = edSurfaceNew(&vidModeData);
	uVar2 = gVRAMed3D->frameBasePtr;
	iVar3 = 0;
	ged3DVramConfig = uVar2;
	do {
		gVRAMBuffers[iVar3] = uVar2;
		gVRAMClutBuffers[iVar3] = uVar2 + 0x380;
		iVar1 = GetedVidBppFromMode(SCE_GS_PSMCT32);
		iVar1 = GetBppFromModeInVRAM(iVar1);
		UINT_ARRAY_00449320[iVar3] = iVar1 << 0x10;
		iVar1 = GetedVidBppFromMode(SCE_GS_PSMCT32);
		iVar1 = GetBppFromModeInVRAM(iVar1);
		UINT_ARRAY_00449328[iVar3] = iVar1 << 0xe;
		iVar1 = iVar3 + 1;
		uVar2 = uVar2 + 0x400;
		UINT_ARRAY_00489434[iVar3] = gVRAMBuffers[iVar3];
		UINT_ARRAY_00489444[iVar3] = gVRAMClutBuffers[iVar3];
		UINT_ARRAY_0048943c[iVar3] = UINT_ARRAY_00449320[iVar3];
		UINT_ARRAY_0048944c[iVar3] = UINT_ARRAY_00449328[iVar3];
		iVar3 = iVar1;
	} while (iVar1 < 2);
	gFXBufAddr = uVar2;
	return;
}

int gCurRenderList = 0;
edpkt_data* g_VifRefPktCur = NULL;
edpkt_data* g_stVifReferencesPacket = NULL;
edpkt_data* g_VifRefPktCurMax = NULL;
edpkt_data* g_pStartPktData = NULL;
uint g_VideoMemorySize_00449280 = 0;
uint g_Model_PCR_00449240 = 0;
byte g_WaitAfterVIF_00449244 = 0;
int gCurFlushList = 0;

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

struct ed_dma_matrix {
	char* pFileData;
	MeshTransformSpecial* field_0x4;
	MeshTransformSpecial internalMeshTransformSpecial;
	undefined4 field_0x18;
	int index_0x1c;
	struct edF32MATRIX4* pTransformMatrix;
	struct MeshTransformDataBase* pMeshTransformData;
	int flags_0x28;
	RenderFrameUnion_2c field_0x2c;
};

struct RenderFrame_30_B_DEPRECATED {
	char* pFileData;
	struct MeshTransformSpecial* field_0x4;
	struct RenderFrame_30_B* field_0x8;
	struct edNODE_MANAGER* field_0xc;
	undefined4 field_0x10;
	undefined4 field_0x14;
	undefined field_0x18;
	undefined field_0x19;
	undefined field_0x1a;
	undefined field_0x1b;
	int index_0x1c;
	struct edF32MATRIX4* pTransformMatrix;
	struct MeshTransformData* pMeshTransformData;
	int flags_0x28;
	float field_0x2c;
};

ed_dma_matrix* DmaMaterialBuffer[2] = { 0 };
ed_dma_matrix* DmaMaterialBufferMax[2] = { 0 };

ed_dma_matrix* DmaMaterialBufferCurrent = NULL;
ed_dma_matrix* DmaMaterialBufferCurrentMax = NULL;

ed_dma_matrix* DmaMatrixBuffer = NULL;
ed_dma_matrix* DmaMatrixBufferMax = NULL;
ed_dma_matrix* DmaMatrixBufferCurrent = NULL;
ed_dma_matrix* DmaMatrixBufferCurrentMax = NULL;
ed_dma_matrix* DmaMatrixDefault = NULL;

byte bTexInvalid = 0;
int gVRAMBufferDMA = 0;
MeshTransformParent* g_TransformParent_00449304 = NULL;
ed_g2d_material* g_MAT_Internal_B_00449338 = NULL;
edLIST* g_ActiveCameraPanMasterHeader_00449308 = NULL;
MeshTransformSpecial* gPrim_List_FlushTex_Last = NULL;

struct ed3DFXConfig {
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

ed3DFXConfig ged3DFXConfig[16] = { 0 };

void ed3DFXClearALLFXSendedTOVRAM(void)
{
	uint uVar1;
	uint uVar2;

	uVar1 = 0;
	do {
		uVar2 = uVar1 + 1;
		ged3DFXConfig[uVar1].field_0x8 = ged3DFXConfig[uVar1].field_0x8 & 0xfffffff7;
		uVar1 = uVar2;
	} while (uVar2 < 0x10);
	return;
}

MeshTransformDataBase MeshTransformDataBase_0048cad0 = { 0 };

void ed3DPrimlistMatrixBufferReset(void)
{
	RENDER_LOG("ed3DPrimlistMatrixBufferReset\n");
	DmaMatrixDefault = DmaMatrixBuffer;
	MeshTransformDataBase_0048cad0.field_0xa4 = (edF32MATRIX4*)0x0;
	DmaMatrixBufferCurrentMax = DmaMatrixBufferMax;
	PTR_MeshTransformParentHeader_004494b8 = pNodeManRender;
	PTR_MeshTransformParentHeader_004494bc = pNodeManRender;
	DmaMatrixBufferCurrent = DmaMatrixBuffer + 1;
	(DmaMatrixBuffer)->pTransformMatrix = &g_IdentityMatrix;
	(DmaMatrixDefault)->pMeshTransformData = &MeshTransformDataBase_0048cad0;
	RENDER_LOG("Storing pMeshTransformData %p\n", &MeshTransformDataBase_0048cad0);
	(DmaMatrixDefault)->field_0x2c.floatValue = 1.0f;
	((DmaMatrixDefault)->internalMeshTransformSpecial).specUnion.randoPtr =
		DmaMatrixDefault;
	(DmaMatrixDefault)->field_0x4 =
		(MeshTransformSpecial*)DmaMatrixDefault;
	((DmaMatrixDefault)->internalMeshTransformSpecial).pNext_0x4 =
		(MeshTransformSpecial*)PTR_MeshTransformParentHeader_004494bc;
	((DmaMatrixDefault)->internalMeshTransformSpecial).pPrev_0x8 = (MeshTransformSpecial*)0x0
		;
	((DmaMatrixDefault)->internalMeshTransformSpecial).pRenderInput =
		(DisplayListInternal*)0x0;
	(DmaMatrixDefault)->pFileData = (char*)0x0;
	(DmaMatrixDefault)->pMeshTransformData = &MeshTransformDataBase_0048cad0;
	RENDER_LOG("Storing pMeshTransformData %p\n", &MeshTransformDataBase_0048cad0);
	return;
}

void ed3DFlushTexReset(edLIST* param_1)
{
	char* pcVar1;
	int iVar2;
	ulong* puVar3;
	int iVar4;
	ed_dma_matrix* pMVar1;

	if (bTexInvalid == 0x3c) {
		bTexInvalid = 0;
	}
	gVRAMBufferDMA = 0;
	bTexInvalid = bTexInvalid + 1;
	g_TransformParent_00449304 = (MeshTransformParent*)param_1->pLoadedData;
	do {
		pMVar1 = (ed_dma_matrix*)g_TransformParent_00449304->pMeshTransformData;
		if (pMVar1 != (ed_dma_matrix*)0x0) {
			g_MAT_Internal_B_00449338 = (ed_g2d_material*)pMVar1->pFileData;
		}
		g_TransformParent_00449304 = g_TransformParent_00449304->pNext;
	} while (pMVar1->field_0x2c.numValue == 0xdfdfdfdf);
	g_ActiveCameraPanMasterHeader_00449308 = param_1;
	if ((pMVar1->flags_0x28 & 2U) != 0) {
		ed3DFXClearALLFXSendedTOVRAM();
		pMVar1->flags_0x28 = pMVar1->flags_0x28 & 0xfffffffd;
	}

	if (((ed_g2d_material*)pMVar1->pFileData)->count_0x0 < 2) {
		pcVar1 = pMVar1->field_0x2c.ptrValue;
		if ((pcVar1 != (char*)0x0) && (*(short*)(pcVar1 + 4) != 0)) {
			iVar2 = *(int*)(pcVar1 + 8);
			char* pBufferAddr = (char*)LOAD_SECTION(iVar2);
			iVar4 = gVRAMBufferDMA * 8;
			edDmaFlushCache();
			RENDER_LOG("DMA Begin ed3DFlushTexReset\n");
			edDmaSend_nowait(SHELLDMA_CHANNEL_GIF, *(ulonglong**)(pBufferAddr + iVar4));
		}
	}
	else {
		IMPLEMENTATION_GUARD(
		puVar3 = ed3DFlushMultiTexture(pMVar1, gPKTMultiTex, 1, gVRAMBufferDMA);
		if (puVar3 != (ulong*)0x0) {
			edDmaFlushCache();
			RENDER_LOG("DMA Begin ed3DFlushTexReset\n");
			edDmaSend_nowait(SHELLDMA_CHANNEL_GIF, gPKTMultiTex);
		})
	}
	gVRAMBufferDMA = gVRAMBufferDMA + 1;
	return;
}

void ed3DFlushTexPrepareDMA(edLIST* pCommandInfo)
{
	edpkt_data* puVar1;

	ed3DFlushTexReset(pCommandInfo);
	puVar1 = g_GifRefPktCur;
	edDmaChannelList[2].QWC = 0xc0;
	if (g_pGifStart != g_GifRefPktCur) {
		g_GifRefPktCur->cmdA = 0x70000000;
		puVar1->cmdB = 0;
		edDmaSyncPath();
		edDmaSyncPath();
#ifdef PLATFORM_IS2
		DPUT_VIF1_FIFO(*(u_long128*)&g_VIF1_FIFO_00424fe0[0]);
#endif
		edDmaFlushCache();
		RENDER_LOG("DMA Begin ed3DFlushTexPrepareDMA\n");
		edDmaSend_nowait(SHELLDMA_CHANNEL_GIF, (ulonglong*)g_pGifStart);
	}
	ed3DFXClearALLFXSendedTOVRAM();
	return;
}

void ed3DFlushSendDMA3D(void)
{
#ifdef PLATFORM_PS2
	DPUT_GIF_MODE(4);
	DPUT_D_CTRL(g_Model_DMAC_Ctrl_mode_00448984);
	DPUT_D_STAT(D_STAT_CIS0_O);
#endif
	edSysHandlersCall(edSysHandlerVideo_0048cee0.mainIdentifier, edSysHandlerVideo_0048cee0.entries,
		edSysHandlerVideo_0048cee0.maxEventID, 1, (void*)0x0);
	gCurFlushList = gCurRenderList;
	gCurRenderList = gCurRenderList != 0 ^ 1;
	edSysHandlersCall(sysHandler_0048cb90.mainIdentifier, sysHandler_0048cb90.entries,
		sysHandler_0048cb90.maxEventID, 1, (void*)0x0);
	if ((edLIST*)gPrim_List_FlushTex[gCurFlushList].pLoadedData !=
		gPrim_List_FlushTex + gCurFlushList) {
		ed3DFlushTexPrepareDMA(gPrim_List_FlushTex + gCurFlushList);
	}
#ifdef PLATFORM_PS2
	DPUT_D_PCR(g_Model_PCR_00449240);
#endif
	if ((g_pStartPktData != (edpkt_data*)0x0) && (g_pStartPktData != g_VifRefPktCur)) {
		edDmaFlushCache();
		RENDER_LOG("DMA Begin ed3DFlushSendDMA3D\n");
		edDmaSend_nowait(SHELLDMA_CHANNEL_VIF1, (ulonglong*)g_pStartPktData);
		if (g_WaitAfterVIF_00449244 != 0) {
			edDmaSyncPath();
			edDmaSyncPath();
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

void ed3DDMAGeneratePacketRefHeader(void)
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

edpkt_data* g_RenderCommandVidMem_004492a0 = NULL;
edpkt_data* g_GifRefPktCurMax = NULL;
char* g_ScratchBase_004493d0 = NULL;
edF32MATRIX4* gPKTMatrixCur = NULL;

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

void ed3DPKTMatrixBufferInit(void)
{
	g_ScratchBase_004493d0 = SubAllocate_0029ce60(ged3DConfig.matrixBufferCount * 0x380);
	gPKTMatrixCur = (edF32MATRIX4*)g_ScratchBase_004493d0;
	//PTR_DAT_004493f0 = (undefined*)((int)g_ScratchBase_004493d0 + Params_00424e30.meshHeaderCountA * 7 * 0x40);
	return;
}

edpkt_data g_PKTShapeHeaderRef[3] = { 0 };
edpkt_data g_PKTSpriteHeaderRef[5] = { 0 };

edpkt_data* gPKTMultiTex = NULL;

void ed3DDMAGenerateSpritePacketRefHeader(void)
{
	g_PKTSpriteHeaderRef[0].cmdA = 0x48967030000001;
	g_PKTSpriteHeaderRef[0].cmdB = 0;
	g_PKTSpriteHeaderRef[1].cmdA = 0x4896c030000002;
	g_PKTSpriteHeaderRef[1].cmdB = 0;
	g_PKTSpriteHeaderRef[2].cmdA = 0x48968030000001;
	g_PKTSpriteHeaderRef[2].cmdB = 0;
	g_PKTSpriteHeaderRef[3].cmdA = 0x48969030000001;
	g_PKTSpriteHeaderRef[3].cmdB = 0;
	g_PKTSpriteHeaderRef[4].cmdA = 0x4896a030000002;
	g_PKTSpriteHeaderRef[4].cmdB = 0;
	return;
}

int g_stExecuteCode = 0;

void ed3DDMAInit(uint countA, int countB)
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
	g_stVifReferencesPacket = (edpkt_data*)((ulong)((ulong)(lVar3 << 0x24) >> 0x24) | 0x30000000);
#else
	g_stVifReferencesPacket = (edpkt_data*)lVar3;
#endif
	g_VifRefPktCurMax = g_stVifReferencesPacket + (countA >> 1);
	g_VideoMemorySize_00449280 = countA;
	g_VifRefPktCur = g_stVifReferencesPacket;
	ed3DDMAGenerateHeaders();
	g_stExecuteCode = 0;
	//DAT_0048c2b4 = 0;
	//DAT_0048c2b8 = 0;
	ed3DDMAGeneratePacketRefHeader();
	ed3DDMAGenerateSpritePacketRefHeader();
	g_PKTShapeHeaderRef[0].cmdB = 0;
	g_PKTShapeHeaderRef[1].cmdB = 0;
	g_PKTShapeHeaderRef[2].cmdB = 0;
	g_PKTShapeHeaderRef[0].cmdA = 0x48964030000001;
	g_PKTShapeHeaderRef[1].cmdA = 0x48965030000002;
	g_PKTShapeHeaderRef[2].cmdA = 0x48963030000001;
	pcVar2 = ed3D_Allocator_00449248.end;
	pcVar1 = ed3D_Allocator_00449248.end + 0x140;
	if (ed3D_Allocator_00449248.current < ed3D_Allocator_00449248.end + 0x140) {
		pcVar2 = (char*)0x0;
		pcVar1 = ed3D_Allocator_00449248.end;
	}
	ed3D_Allocator_00449248.end = pcVar1;
	gPKTMultiTex = (edpkt_data*)((ulong)pcVar2 & 0xdfffffff);
	ed3DPKTMatrixBufferInit();
	lVar3 = (ulong)ed3D_Allocator_00449248.end;
	pcVar2 = ed3D_Allocator_00449248.end + countB * 0x20;
	if (ed3D_Allocator_00449248.current < ed3D_Allocator_00449248.end + countB * 0x20) {
		lVar3 = 0;
		pcVar2 = ed3D_Allocator_00449248.end;
	}
	ed3D_Allocator_00449248.end = pcVar2;
	g_RenderCommandVidMem_004492a0 = (edpkt_data*)(((ulong)(lVar3 << 0x24) >> 0x24) | 0x30000000);
	g_GifRefPktCur = g_RenderCommandVidMem_004492a0;
	g_GifRefPktCurMax = g_RenderCommandVidMem_004492a0 + countB;
	return;
}

ed_g2d_material* previous_flush3DMat = NULL;
ed_dma_matrix* gpLastMaterial = NULL;
int gVRAMBufferFlush = 0;
byte gbFirstTex = 0;

void ed3DPKTMatrixBufferFlip(void)
{
	gPKTMatrixCur = (edF32MATRIX4*)g_ScratchBase_004493d0;
	if (gCurRenderList != 0) {
		gPKTMatrixCur =
			(edF32MATRIX4*)(g_ScratchBase_004493d0 + ged3DConfig.matrixBufferCount * 0x1c0);
	}
	//PTR_DAT_004493f0 = (undefined*)(gPKTMatrixCur + Params_00424e30.meshHeaderCountA * 7);
	return;
}

void ed3DFlushListInit(void)
{
	uint uVar1;
	int GifRefPktCurMax;

	gVRAMBufferFlush = 0;
	previous_flush3DMat = (ed_g2d_material*)0x0;
	gpCurHierarchy = (MeshTransformDataBase*)0x0;
	gpLastMaterial = (ed_dma_matrix*)0x0;
	if (gCurRenderList == 0) {
		GifRefPktCurMax = ged3DConfig.field_0x28 * 0x10;
		g_GifRefPktCur = g_RenderCommandVidMem_004492a0;
		g_VifRefPktCur = g_stVifReferencesPacket;
		uVar1 = g_VideoMemorySize_00449280 >> 1;
	}
	else {
		g_VifRefPktCur = g_stVifReferencesPacket + (g_VideoMemorySize_00449280 >> 1);
		GifRefPktCurMax = ged3DConfig.field_0x28 * 0x20;
		g_GifRefPktCur = g_RenderCommandVidMem_004492a0 + ged3DConfig.field_0x28;
		uVar1 = g_VideoMemorySize_00449280;
	}
	g_GifRefPktCurMax = (edpkt_data*)((ulong)&g_RenderCommandVidMem_004492a0->cmdA + GifRefPktCurMax);
	g_VifRefPktCurMax = g_stVifReferencesPacket + uVar1;
	gpPKTDataRefMasPath3 = (edpkt_data*)0x0;
	gbFirstTex = 1;
	g_pGifStart = g_GifRefPktCur;
	ed3DPKTMatrixBufferFlip();
	ed3D_SubAllocator_004491b8.current = ed3D_SubAllocator_004491b8.base;
	g_pStartPktData = g_VifRefPktCur;
	return;
}

char* ed3DGetG2DBitmap(ed_dma_matrix* param_1, int index)
{
	char* pcVar1;

	pcVar1 = param_1->pFileData;
	if ((pcVar1 == (char*)0x0) || (*pcVar1 == '\0')) {
		pcVar1 = (char*)0x0;
	}
	else {
		IMPLEMENTATION_GUARD(pcVar1 = (char*)ed3DGetG2DBitmap(pcVar1, index));
	}
	return pcVar1;
}

char DAT_0048c3b0[200] = { 0 };
char DAT_0048c3c0[200] = { 0 };

RenderFrame_30_B* pprevious_shadow_dma_matrix = NULL;

void ed3DShadowResetPrimList(void)
{
	edNODE_MANAGER* pMVar1;
	char** ppFileData;
	edLIST* pCVar3;
	ed_dma_matrix* pMeshCurrrent;
	MeshTransformSpecial* pInternalMeshTransformSpecial;
	char* pcVar6;

	RENDER_LOG("ed3DShadowResetPrimList\n");

	// Setup Mesh Current
	pMeshCurrrent = DmaMaterialBufferCurrent;
	pInternalMeshTransformSpecial = &(DmaMaterialBufferCurrent)->internalMeshTransformSpecial;
	ppFileData = &(DmaMaterialBufferCurrent)->pFileData;
	DmaMaterialBufferCurrent = DmaMaterialBufferCurrent + 1;
	*ppFileData = DAT_0048c3b0;
	pMeshCurrrent->field_0x4 = (MeshTransformSpecial*)0x0;
	pMeshCurrrent->flags_0x28 = pMeshCurrrent->flags_0x28 & 0xfffffffe;
	(pMeshCurrrent->internalMeshTransformSpecial).pPrev_0x8 = pInternalMeshTransformSpecial;
	(pMeshCurrrent->internalMeshTransformSpecial).pNext_0x4 = pInternalMeshTransformSpecial;
	(pMeshCurrrent->internalMeshTransformSpecial).pRenderInput = (DisplayListInternal*)PTR_MeshTransformParentHeader_004494b8;
	pMeshCurrrent->field_0x18 = 0;
	pMeshCurrrent->index_0x1c = 0;
	pMeshCurrrent->internalMeshTransformSpecial.specUnion.randoPtr = NULL;
	pcVar6 = ed3DGetG2DBitmap(pMeshCurrrent, 0);
	pMeshCurrrent->field_0x2c.ptrValue = pcVar6;

	pMVar1 = gPrim_List_Shadow[gCurRenderList].pCameraPanHeader_0xc;
	pInternalMeshTransformSpecial = pMVar1->pNextFree + pMVar1->usedCount;
	pInternalMeshTransformSpecial->pRenderInput = (DisplayListInternal*)pMeshCurrrent;
	pInternalMeshTransformSpecial->specUnion.field_0x0[0] = 1;
	pMVar1->usedCount = pMVar1->usedCount + 1;
	pCVar3 = gPrim_List_Shadow + gCurRenderList;
	gPrim_List_Shadow[gCurRenderList].count_0x14 =
		gPrim_List_Shadow[gCurRenderList].count_0x14 + 1;
	pInternalMeshTransformSpecial->pNext_0x4 = pCVar3->pLoadedData;
	RENDER_LOG("ed3DShadowResetPrimList B %p\n", pCVar3->pLoadedData);
	gPrim_List_Shadow[gCurRenderList].pLoadedData = pInternalMeshTransformSpecial;

	// Setup Mesh Current
	pMeshCurrrent = DmaMaterialBufferCurrent;
	pInternalMeshTransformSpecial = &(DmaMaterialBufferCurrent)->internalMeshTransformSpecial;
	ppFileData = &(DmaMaterialBufferCurrent)->pFileData;
	DmaMaterialBufferCurrent = DmaMaterialBufferCurrent + 1;
	*ppFileData = DAT_0048c3c0;
	pMeshCurrrent->field_0x4 = (MeshTransformSpecial*)0x0;
	RENDER_LOG("Storing pMeshTransformData %p\n", 0x0);
	pMeshCurrrent->flags_0x28 = pMeshCurrrent->flags_0x28 & 0xfffffffe;
	(pMeshCurrrent->internalMeshTransformSpecial).pPrev_0x8 = pInternalMeshTransformSpecial;
	(pMeshCurrrent->internalMeshTransformSpecial).pNext_0x4 = pInternalMeshTransformSpecial;
	(pMeshCurrrent->internalMeshTransformSpecial).pRenderInput = (DisplayListInternal*)PTR_MeshTransformParentHeader_004494b8;
	pMeshCurrrent->field_0x18 = 0;
	pMeshCurrrent->index_0x1c = 0;
	*(undefined4*)&pMeshCurrrent->internalMeshTransformSpecial = 0;
	pMeshCurrrent->flags_0x28 = pMeshCurrrent->flags_0x28 | 1;
	pcVar6 = ed3DGetG2DBitmap(pMeshCurrrent, 0);
	pMeshCurrrent->field_0x2c.ptrValue = pcVar6;

	pMVar1 = gPrim_List_Shadow[gCurRenderList].pCameraPanHeader_0xc;
	pInternalMeshTransformSpecial = pMVar1->pNextFree + pMVar1->usedCount;
	pInternalMeshTransformSpecial->pRenderInput = (DisplayListInternal*)pMeshCurrrent;
	pInternalMeshTransformSpecial->specUnion.field_0x0[0] = 1;
	pMVar1->usedCount = pMVar1->usedCount + 1;
	pCVar3 = gPrim_List_Shadow + gCurRenderList;
	gPrim_List_Shadow[gCurRenderList].count_0x14 =
		gPrim_List_Shadow[gCurRenderList].count_0x14 + 1;
	pInternalMeshTransformSpecial->pNext_0x4 = pCVar3->pLoadedData;
	RENDER_LOG("ed3DShadowResetPrimList C %p\n", pCVar3->pLoadedData);
	gPrim_List_Shadow[gCurRenderList].pLoadedData = pInternalMeshTransformSpecial;
	pprevious_shadow_dma_matrix = 0;
	return;
}

int gCurSceneID = 0;

void ed3DPrimlistInitMaterialRenderList(void)
{
	edLIST* pCVar1;
	edNODE_MANAGER** ppMVar2;

	ppMVar2 = &pNodeManMaterial + gCurRenderList;
	pCVar1 = gPrim_List_FlushTex + gCurRenderList;
	gPrim_List_FlushTex[gCurRenderList].pLoadedData =
		(MeshTransformSpecial*)(gPrim_List_FlushTex + gCurRenderList);
	pCVar1->count_0x14 = 0;
	gPrim_List_FlushTex_Last = (MeshTransformSpecial*)0x0;
	(*ppMVar2)->usedCount = 0;
	DmaMaterialBufferCurrent = DmaMaterialBuffer[gCurRenderList];
	DmaMaterialBufferCurrentMax = DmaMaterialBufferMax[gCurRenderList];
	return;
}


void UpdateRenderMeshTransform_002954b0(MeshTransformData* param_1, bool mode)
{
	MeshTransformData* v1;
	MeshTransformData* v1_00;
	edF32MATRIX4* v1_01;
	edF32MATRIX4* v1_02;
	edF32MATRIX4* v1_03;
	edF32MATRIX4* v1_04;
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
				v1_01 = *(edF32MATRIX4*)((int)(v1_00->base).hash.number + 0x10);
				if (v1_01 == (edF32MATRIX4*)0x0) {
					if ((*(ushort*)((int)(v1_00->base).hash.number + 0x1e) & 1) == 0) {
						sceVu0CopyMatrix((TO_SCE_MTX) & (v1_00->base).transformB, (TO_SCE_MTX)v1_00);
					}
				}
				else {
					if (*(char*)(v1_01[2] + 2) != '\0') {
						v1_02 = (edF32MATRIX4*)v1_01[2][1][0];
						if (v1_02 == (edF32MATRIX4*)0x0) {
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
														ed3DSetSceneRender(pMeshTransformData, 0);
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

void ed3DComputeSonHierarchy(void)
{
	edLIST* pCVar1;
	int iVar2;
	uint uVar3;
	uint uVar4;
	MeshTransformParent* pCVar6;
	int* piVar5;
	ed_3D_Scene* pSVar6;
	int iVar7;

	for (uVar3 = 0; uVar3 < (uint)ged3DConfig.sceneCount; uVar3 = uVar3 + 1) {
		pSVar6 = gScene3D + uVar3;
		if (((pSVar6->flags_0x4 & 1) != 0) && ((pSVar6->flags_0x4 & 4) == 0)) {
			pCVar1 = pSVar6->pHeirListA;
			for (pCVar6 = (MeshTransformParent*)pCVar1->pLoadedData; (edLIST*)pCVar6 != pCVar1;
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
	edSysHandlersCall(sysHandler_0048cb90.mainIdentifier, sysHandler_0048cb90.entries,
		sysHandler_0048cb90.maxEventID, 4, (void*)0x0);
	return;
}

byte BYTE_004492e8 = 0;

void ed3DDMALoadVU1MicroCode(void)
{
	if (BYTE_004492e8 == 0) {
		edDmaFlushCache();
		edDmaSync(SHELLDMA_CHANNEL_VIF1);
		RENDER_LOG("DMA Begin ed3DDMALoadVU1MicroCode NOT RUN\n");
		edDmaSend(SHELLDMA_CHANNEL_VIF1, (ulonglong*)vu1_micro);
		BYTE_004492e8 = 1;
	}
	return;
}

void ed3DVU1BufferInit(void)
{
	ed3DVU1BufferCur = 1;
	ed3DVU1Buffer[0] = 0x76;
	ed3DVU1Buffer[1] = 0x198;
	ed3DVU1Buffer[2] = 0x2ba;
	gRefOptionsforVU1Buf[0] = 0x30000000;
	gRefOptionsforVU1Buf[1] = 0x200000003000077;
	gRefOptionsforVU1Buf[2] = 0x30000000;
	gRefOptionsforVU1Buf[3] = 0x200000003000199;
	gRefOptionsforVU1Buf[4] = 0x30000000;
	gRefOptionsforVU1Buf[5] = 0x2000000030002bb;
	return;
}

int gCurFreeScene = 0;

void ed3DSceneCompute(int param_1, int param_2, char* param_3)
{
	int iVar1;
	uint uVar2;
	//ProfileObject* pPVar3;
	//
	//iVar1 = gIDProfileRender;
	gCurSceneID = 0;
	if (ged3DConfig.bEnableProfile != 0) {
		IMPLEMENTATION_GUARD(
		if ((gIDProfileRender & 1U) == 0) {
			uVar2 = (uint)gIDProfileRender >> 4;
			if (ProfileManagerB[uVar2].field_0x34 != 0) {
				ProfileManagerB[uVar2].field_0x8 = 0;
				ProfileManagerB[uVar2].field_0x20 = 0;
				ProfileManagerB[uVar2].field_0x34 = 0;
			}
			ProfileManagerB[(uint)iVar1 >> 4].field_0x0 = (long)PCR1 & 0xffffffff;
			ProfileManagerB[(uint)iVar1 >> 4].field_0x20 = ProfileManagerB[(uint)iVar1 >> 4].field_0x20 + 1;
		}
		else {
			uVar2 = (uint)gIDProfileRender >> 4;
			if (g_ProfileObjA_0041ed40[uVar2].field_0x34 != 0) {
				g_ProfileObjA_0041ed40[uVar2].field_0x8 = 0;
				g_ProfileObjA_0041ed40[uVar2].field_0x20 = 0;
				g_ProfileObjA_0041ed40[uVar2].field_0x34 = 0;
			}
			g_ProfileObjA_0041ed40[(uint)iVar1 >> 4].field_0x0 = (long)PCR1 & 0xffffffff;
			g_ProfileObjA_0041ed40[(uint)iVar1 >> 4].field_0x20 = g_ProfileObjA_0041ed40[(uint)iVar1 >> 4].field_0x20 + 1;
		})
	}
	ed3DFlushSendDMA3D();
	gCurFreeScene = 0;
	//ed3DClusterReset();
	if ((ged3DConfig.field_0x2c & 2) != 0) {
		ed3DComputeSonHierarchy();
	}
	gCurViewportUsed = (ed_viewport*)0x0;
	ed3DDMALoadVU1MicroCode();
	ed3DVU1BufferInit();
	ed3DFlushListInit();
	ed3DPrimlistInitMaterialRenderList();
	ed3DPrimlistMatrixBufferReset();
	ed3DShadowResetPrimList();
	if (ged3DConfig.bEnableProfile != 0) {
		IMPLEMENTATION_GUARD(
		uVar2 = (uint)gIDProfileRender >> 4;
		if ((gIDProfileRender & 1U) == 0) {
			ProfileManagerB[uVar2].field_0x0 = (long)PCR1 - ProfileManagerB[uVar2].field_0x0;
			ProfileManagerB[uVar2].field_0x8 =
				ProfileManagerB[uVar2].field_0x8 + ProfileManagerB[uVar2].field_0x0;
			pPVar3 = ProfileManagerB + uVar2;
			if (pPVar3->field_0x10 < ProfileManagerB[uVar2].field_0x0) {
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

ed_viewport* gDebugViewport = NULL;
void* gScene_list_buffer = NULL;

edLIST* ed3DHierarchyListInit(void)
{
	edLIST* pList;

	pList = edListNew(1, 0, (EHeap)gScene_list_buffer);
	edListLink(pList, gHierarchyManagerNodeManager, 0, 0);
	return pList;
}

float g_DefaultNearClip_0044851c = -0.001f;

void ed3DShadowConfigSetDefault(ed_3D_Shadow_Config* pShadowConfig)
{
	pShadowConfig->field_0x0 = 0;
	pShadowConfig->field_0x4 = 20.0;
	pShadowConfig->field_0x8 = -0.0;
	pShadowConfig->field_0xc = -50.0;
	pShadowConfig->pCamera_0x10 = (ed_viewport*)0x0;
	pShadowConfig->pCamera_0x14 = (ed_viewport*)0x0;
	pShadowConfig->field_0x20 = 0;
	pShadowConfig->field_0x22 = 0x30;
	pShadowConfig->field_0x23 = 0x14;
	pShadowConfig->field_0x24 = 2;
	pShadowConfig->field_0x18 = 0x80;
	pShadowConfig->field_0x1c = 0x80;
	return;
}

Vector Vector_00424f10 = { 10.0f, 10.0f, 10.0f, 128.0f };
edF32MATRIX4 Matrix_00424ed0 = { 64.0f,
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

edF32MATRIX4 Matrix_00424e90 = { 1.0f,
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

Vector* LightAmbiant_DEBUG = &Vector_00424f10;
Vector* LightDirections_Matrix_DEBUG = (Vector*)&Matrix_00424e90;
Vector* LightColor_Matrix_DEBUG = (Vector*)&Matrix_00424ed0;

void ed3DSceneLightConfigSetDefault(ed_3D_Light_Config* pLightConfig)
{
	pLightConfig->pLightAmbient = LightAmbiant_DEBUG;
	pLightConfig->pLightDirections = LightDirections_Matrix_DEBUG;
	pLightConfig->pLightColorMatrix = LightColor_Matrix_DEBUG;
	return;
}

void ed3DSceneConfigSetDefault(SceneConfig* pSceneConfig)
{
	pSceneConfig->field_0x4 = 250.0;
	pSceneConfig->field_0x18 = 250.0;
	pSceneConfig->nearClip = g_DefaultNearClip_0044851c;
	pSceneConfig->farClip = -5000.0;
	pSceneConfig->field_0x10 = 1;
	(pSceneConfig->field_0xe0).x = 0.0;
	(pSceneConfig->field_0xe0).y = 255.0;
	(pSceneConfig->field_0xe0).z = g_DefaultNearClip_0044851c;
	(pSceneConfig->field_0xe0).w = -3200.0;
	(pSceneConfig->fogCol_0xf0).r = 0x3e;
	(pSceneConfig->fogCol_0xf0).g = 0x47;
	(pSceneConfig->fogCol_0xf0).b = 0x67;
	ed3DShadowConfigSetDefault(&pSceneConfig->pShadowConfig);
	ed3DSceneLightConfigSetDefault(&pSceneConfig->pLightConfig);
	return;
}

ed_3D_Scene* ed3DSceneCreate(edFCamera* pCamera, ed_viewport* pViewport, long mode)
{
	ushort uVar1;
	edSurface* pVVar2;
	ed_3D_Scene* pSVar3;
	edLIST* pCVar4;
	ed_3D_Scene* pFreeScene;
	uint sceneIndex;

	sceneIndex = 0;
	/* Find inactive camera master. */
	while ((sceneIndex < (uint)ged3DConfig.sceneCount &&
		((gScene3D[sceneIndex].flags_0x4 & 1) != 0))) {
		sceneIndex = sceneIndex + 1;
	}
	pSVar3 = (ed_3D_Scene*)0xffffffff;
	if (sceneIndex < (uint)ged3DConfig.sceneCount) {
		pSVar3 = gScene3D + sceneIndex;
		pSVar3->flags_0x4 = pSVar3->flags_0x4 | 1;
		gScene3D[sceneIndex].bShadowScene = 0;
		gScene3D[sceneIndex].pCamera = pCamera;
		gScene3D[sceneIndex].pViewport = pViewport;
		if (gDebugViewport == (ed_viewport*)0x0) {
			gDebugViewport = pViewport;
		}
		ed3DSceneConfigSetDefault(&gScene3D[sceneIndex].sceneConfig);
		pFreeScene = gScene3D + sceneIndex;
		pVVar2 = pFreeScene->pViewport->pZBuffer;
		if (pVVar2 == (edSurface*)0x0) {
			IMPLEMENTATION_GUARD((pSVar5->sceneConfig).field_0x14 = &DAT_00ffffef);
		}
		else {
			uVar1 = pVVar2->pSurfaceDesc->pixelStoreMode;
			if (uVar1 == 2) {
				(pFreeScene->sceneConfig).field_0x14 = 0xfffe;
			}
			else {
				if (uVar1 == 1) {
					IMPLEMENTATION_GUARD((pSVar5->sceneConfig).field_0x14 = &DAT_00ffffef);
				}
				else {
					if (uVar1 == 0) {
						(pFreeScene->sceneConfig).field_0x14 = 0x00ffffef;
					}
				}
			}
		}
		if (mode != 0) {
			pCVar4 = ed3DHierarchyListInit();
			gScene3D[sceneIndex].pHeirListA = pCVar4;
			pCVar4 = ed3DHierarchyListInit();
			gScene3D[sceneIndex].pHeirListB = pCVar4;
		}
	}
	return pSVar3;
}


void ed3DFlushMatrixReset(void)
{
	gpCurHierarchy = (MeshTransformDataBase*)0xefefefef;
	return;
}

void ed3DFANGifTagReset(void)
{
	pOldFANGifTag = (undefined*)0x0;
	return;
}

void ed3DFlushSceneInit(void)
{
	ed3DFlushMatrixReset();
	ed3DFANGifTagReset();
	return;
}

edF32MATRIX4* g_ScratchpadAddr_00448a58 = (edF32MATRIX4*)0x70000000;

struct ScratchpadAnimManager {
	ScratchpadAnimManager();
	struct edF32MATRIX4* pScreenSpaceMatrix;
	struct edF32MATRIX4* pTextureSpaceMatrix;
	struct edF32MATRIX4* pWorldSpaceMatrix;
	struct edF32MATRIX4* pLightDirections;
	struct edF32MATRIX4* pLightColor;
	struct Vector* pLightAmbient;
	struct Vector* pVector_0x18;
	char* field_0x1c;
	struct edF32MATRIX4** field_0x20;
	char* field_0x24;
	undefined4* gShadowRenderMask;
	struct edFCamera* pData_130;
	struct edF32MATRIX4* pMatrix_0x30;
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
	struct SceneConfig* pData_150;
	struct RenderAnimationStatic* pRenderAnimationStatic_0x60;
	struct Vector* vector_0x64;
	struct AnimScratchpad* pAnimScratchpad;
};

struct RenderAnimationStatic {
	struct edF32MATRIX4* pSharedMeshTransform;
	struct edF32MATRIX4* pMeshTransformMatrix;
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
	g_ScratchpadAddr_00448a58 = (edF32MATRIX4*)malloc(0x10000);
#endif
	pScreenSpaceMatrix = g_ScratchpadAddr_00448a58 + 1;
	pTextureSpaceMatrix = g_ScratchpadAddr_00448a58 + 2;
	pWorldSpaceMatrix = g_ScratchpadAddr_00448a58 + 3;
	pLightDirections = g_ScratchpadAddr_00448a58 + 4;
	pLightColor = g_ScratchpadAddr_00448a58 + 5;
	pLightAmbient = (Vector*)(g_ScratchpadAddr_00448a58 + 6);
	pVector_0x18 = (Vector*)&g_ScratchpadAddr_00448a58[6].ba;
	field_0x1c = (char*)&g_ScratchpadAddr_00448a58[6].ca;
	field_0x20 = (edF32MATRIX4**)&g_ScratchpadAddr_00448a58[6].cb;
	field_0x24 = (char*)&g_ScratchpadAddr_00448a58[6].cc;
	gShadowRenderMask = (undefined4*)& g_ScratchpadAddr_00448a58[6].cd;
	pData_130 = (edFCamera*)&g_ScratchpadAddr_00448a58[6].da;
	pMatrix_0x30 = (edF32MATRIX4*)&g_ScratchpadAddr_00448a58[0xb].ca;
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
	pData_150 = (SceneConfig*)&g_ScratchpadAddr_00448a58[0xd].da;
	pRenderAnimationStatic_0x60 =
		(RenderAnimationStatic*)(g_ScratchpadAddr_00448a58 + 0x13);
	vector_0x64 = (Vector*)&g_ScratchpadAddr_00448a58[0x13].ca;
	pAnimScratchpad = (AnimScratchpad*)&g_ScratchpadAddr_00448a58[0x13].da;
	return;
}

ScratchpadAnimManager ScratchpadAnimManager_004494ec;
edF32MATRIX4 Matrix_004894e0 = { 0 };

Vector* gLightAmbiant = NULL;
Vector* gLightDirections_Matrix = NULL;
Vector* gLightColor_Matrix = NULL;

void Func_00293ec0(ed_3D_Light_Config* param_1)
{
	gLightAmbiant = param_1->pLightAmbient;
	if (gLightAmbiant != (Vector*)0x0) {
		gLightAmbiant->w = 0.0078125;
	}
	gLightDirections_Matrix = param_1->pLightDirections;
	gLightColor_Matrix = param_1->pLightColorMatrix;
	return;
}

Vector Vector_0048c300 = { 0 };
Vector Vector_0048c310 = { 0 };
Vector Vector_0048c320 = { 0 };
Vector Vector_0048c330 = { 0 };

float FLOAT_0044921c = 0.0f;
float FLOAT_00449218 = 0.0f;
float FLOAT_00449214 = 0.0f;

edSurface g_ColorBufferCopy_00489520 = { 0 };

SceneConfig* g_CameraPanStatic_50_Ptr_00449220 = NULL;

ed_3D_Scene* g_StaticMeshMaster_00449224 = NULL;

float g_RectangleScale_00448980 = 5.0f;


void ComputeMatrix_0029b610
(float screenWidth, float screenHeight, float width, float height, float yMin, float zValue, float wValue,
	float nearClip, edF32MATRIX4* mScreenSpace, edF32MATRIX4* mTextureSpace, edF32MATRIX4* mWorldSpace, edF32MATRIX4* mUnknown,
	float farClip, float fovUp, float fovDown)

{
	float fVar1;
	float fVar2;
	float fVar3;
	float startX;
	float fVar4;
	float startY;
	edF32MATRIX4 MStack128;
	edF32MATRIX4 MStack64;

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

edF32MATRIX4 g_TransformationMatrix_00489460 = { 0 };
edF32MATRIX4 Matrix_004894a0 = { 0 };
edF32MATRIX4 g_ScreenSpaceMatrix_0048cbb0 = { 0 };
edF32MATRIX4 g_TextureSpaceMatrix_0048cbf0 = { 0 };
edF32MATRIX4 g_WorldSpaceMatrix_0048cc30 = { 0 };
edF32MATRIX4 g_TransformationMatrix_0048cc70 = { 0 };
edF32MATRIX4 Matrix_0048ccb0 = { 0 };
edF32MATRIX4 Matrix_00489550 = { 0 };

Vector Vector_0048ccf0 = { 0 };
Vector Vector_0048cd00 = { 0 };
Vector Vector_0048cd10 = { 0 };
Vector Vector_0048cd20 = { 0 };
Vector Vector_0048cd30 = { 0 };
Vector Vector_0048cd40 = { 0 };

ed_g2d_material* PTR_TextureData_MAT_004492ec = NULL;
ed_g2d_material* PTR_TextureData_MAT_004492f0 = NULL;

ed_3D_Scene* gShadowRenderScene = NULL;

ed_viewport* g_Camera_0044956c = NULL;

void SetCamera_002b6d10(ed_viewport* pCamera)
{
	g_Camera_0044956c = pCamera;
	return;
}

float FLOAT_0044922c = 0.0f;
float FLOAT_00449230 = 0.0f;
float FLOAT_00449234 = 0.0f;

void CameraSetupFunc_0029b120
(SceneConfig* param_1, edFCamera* param_2, edSurface* pFrameBuffer)
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
	uVar1 = pFrameBuffer->pSurfaceDesc->screenHeight;
	fVar7 = 2048.0 - FLOAT_00449230;
	fVar8 = atan2f
	(((2048.0 - FLOAT_0044922c) * min) /
		((float)(uint)pFrameBuffer->pSurfaceDesc->screenWidth / 2.0), fVar12);
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

ed_g2d_material ed_g2d_material_ARRAY_0048c370[6] = { 0 };

int ed3DInitRenderEnvironement(ed_3D_Scene* pStaticMeshMaster, long mode)
{
	uint uVar1;
	Vector* v0;
	float fVar2;
	float fVar3;
	float farClip;
	float fVar5;
	edF32MATRIX4 local_40;

	RENDER_LOG("ed3DInitRenderEnvironement: %d\n", mode);

	fVar5 = pStaticMeshMaster->pCamera->aMatrices[0].dc;
	memcpy(ScratchpadAnimManager_004494ec.pData_130, pStaticMeshMaster->pCamera, sizeof(edFCamera));
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
	memcpy(&g_ColorBufferCopy_00489520, pStaticMeshMaster->pViewport->pColorBuffer, sizeof(edSurface));
	g_CameraPanStatic_50_Ptr_00449220 = &pStaticMeshMaster->sceneConfig;
	(pStaticMeshMaster->sceneConfig).field_0x18 = (pStaticMeshMaster->sceneConfig).field_0x4;
	memcpy(ScratchpadAnimManager_004494ec.pData_150, g_CameraPanStatic_50_Ptr_00449220, sizeof(SceneConfig));
	g_StaticMeshMaster_00449224 = pStaticMeshMaster;
	Func_00293ec0(&(pStaticMeshMaster->sceneConfig).pLightConfig);
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
	ComputeMatrix_0029b610((float)(int)pStaticMeshMaster->pViewport->screenWidth,
		(float)(int)pStaticMeshMaster->pViewport->screenHeight,
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
		memcpy(&g_ScreenSpaceMatrix_0048cbb0, ScratchpadAnimManager_004494ec.pScreenSpaceMatrix, sizeof(edF32MATRIX4));
		memcpy(&g_TextureSpaceMatrix_0048cbf0, ScratchpadAnimManager_004494ec.pTextureSpaceMatrix, sizeof(edF32MATRIX4));
		memcpy(&g_WorldSpaceMatrix_0048cc30, ScratchpadAnimManager_004494ec.pWorldSpaceMatrix, sizeof(edF32MATRIX4));
		memcpy(&g_TransformationMatrix_0048cc70, &g_TransformationMatrix_00489460, sizeof(edF32MATRIX4));
		memcpy(&Matrix_0048ccb0, g_ScratchpadAddr_00448a58, sizeof(edF32MATRIX4));
		memcpy(&Vector_0048ccf0, &Vector_0048c2d0, sizeof(Vector));
		memcpy(&Vector_0048cd00, &Vector_0048c2e0, sizeof(Vector));
		memcpy(&Vector_0048cd10, &Vector_0048c2f0, sizeof(Vector));
		memcpy(&Vector_0048cd20, &Vector_0048c300, sizeof(Vector));
		memcpy(&Vector_0048cd30, &Vector_0048c310, sizeof(Vector));
		memcpy(&Vector_0048cd40, &Vector_0048c320, sizeof(Vector));
	}
	if (pStaticMeshMaster->bShadowScene == 1) {
		uVar1 = GetGreaterPower2Val((int)pStaticMeshMaster->pViewport->screenWidth);
		if ((int)uVar1 < 0) {
			fVar2 = (float)(uVar1 >> 1 | uVar1 & 1);
			fVar2 = fVar2 + fVar2;
		}
		else {
			fVar2 = (float)uVar1;
		}
		uVar1 = GetGreaterPower2Val((int)pStaticMeshMaster->pViewport->screenHeight);
		if ((int)uVar1 < 0) {
			fVar3 = (float)(uVar1 >> 1 | uVar1 & 1);
			fVar3 = fVar3 + fVar3;
		}
		else {
			fVar3 = (float)uVar1;
		}
		PTR_TextureData_MAT_004492ec = &ed_g2d_material_0048c3b0;
		PTR_TextureData_MAT_004492f0 = &ed_g2d_material_0048c3c0;
		Vector_0048c300.x = 2048.0f - fVar2 / 2.0f;
		Vector_0048c300.y = 2048.0f - fVar3 / 2.0f;
		Vector_0048c300.z = 1.0f / fVar2;
		Vector_0048c300.w = 1.0f / fVar3;
		SetCamera_002b6d10(((ScratchpadAnimManager_004494ec.pData_150)->pShadowConfig).pCamera_0x14);
		*ScratchpadAnimManager_004494ec.gShadowRenderMask =
			(uint)(ushort)((ScratchpadAnimManager_004494ec.pData_150)->pShadowConfig).field_0x20;
		if (*ScratchpadAnimManager_004494ec.gShadowRenderMask == 0) {
			return 0;
		}
		memcpy(&Matrix_00489550, g_ScratchpadAddr_00448a58, sizeof(edF32MATRIX4));
		gShadowRenderScene = pStaticMeshMaster;
	}
	else {
		*ScratchpadAnimManager_004494ec.gShadowRenderMask = 0;
		PTR_TextureData_MAT_004492ec = ed_g2d_material_ARRAY_0048c370;
		PTR_TextureData_MAT_004492f0 = ed_g2d_material_ARRAY_0048c370 + 1;
	}
	if (pStaticMeshMaster->pViewport->screenHeight != 0x200) {
		pStaticMeshMaster->pCamera->aMatrices[0].dc = fVar5;
	}
	return 1;
}

edpkt_data* ed3DDMAGenerateGlobalPacket(edpkt_data* param_1)
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

void ed3DInitVU1Globals(void)
{
	edpkt_data* pRVar1;
	pRVar1 = g_VifRefPktCur;
	sceDmaAddRef((sceDmaTag**)&g_VifRefPktCur, 1, (void*)g_StaticVideoList_00489590);
	pRVar1->cmdB = 0x0;
	g_VifRefPktCur = ed3DDMAGenerateGlobalPacket(g_VifRefPktCur);
	return;
}

edpkt_data* ed3DAddViewportContextPacket(ed_viewport* pCamera, edpkt_data* pCommandBuffer)
{
	edpkt_data* pRVar1;
	uint uVar2;

	pRVar1 = pCommandBuffer;
	if (pCamera != (ed_viewport*)0x0) {
		pRVar1 = edViewportUpdateEnv(pCamera, pCommandBuffer + 1);
		uVar2 = ((uint)((ulong)pRVar1 - (ulong)pCommandBuffer) >> 4) - 1;
		pCommandBuffer->cmdA = (ulong)(ulong)uVar2 & 0xffffffffU | 0xe10000000;
		pCommandBuffer->cmdB = 0;
		*(undefined4*)&pCommandBuffer->cmdB = 0;
		*(uint*)((ulong)&pCommandBuffer->cmdB + 4) = uVar2 | 0x51000000;
	}
	return pRVar1;
}

edpkt_data* ed3DSceneAddContextPacket(ed_3D_Scene* param_1, edpkt_data* pInCommandBuffer)
{
	edpkt_data* pRVar1;

	RENDER_LOG("ed3DSceneAddContextPacket %p\n", param_1);

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

	pInCommandBuffer[2].cmdA = SCE_GS_SET_FOGCOL(param_1->sceneConfig.fogCol_0xf0.r, param_1->sceneConfig.fogCol_0xf0.g, param_1->sceneConfig.fogCol_0xf0.b);
	pInCommandBuffer[2].cmdB = SCE_GS_FOGCOL;

	*(undefined4*)&pInCommandBuffer[3].cmdA = 0;
	*(undefined4*)((ulong)&pInCommandBuffer[3].cmdA + 4) = 0x14000000;
	pInCommandBuffer[3].cmdB = 0x11000000;
	pRVar1 = pInCommandBuffer + 4;
	if (param_1->pViewport != (ed_viewport*)0x0) {
		pRVar1 = ed3DAddViewportContextPacket(param_1->pViewport, pRVar1);
	}
	return pRVar1;
}

struct CameraScreenData {
	short posX;
	short posY;
	short screenWidth;
	short screenHeight;
};

CameraScreenData gCurRectViewport = { 0 };
CameraScreenData CameraScreenData_00449560 = { 0 };

edpkt_data* ed3DFlushResetOffset(edpkt_data* pRenderCommand, CameraScreenData* pScreenData)
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

void ed3DClustersRenderComputeSceneHierarchyList(ed_3D_Scene* pStaticMeshMaster)
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
			MeshTransformData::ed3DRenderSonHierarchy(*(MeshTransformData**)(iVar2 + 0xc));)
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

void ed3DSceneSortClusters(ed_3D_Scene* pStaticMeshMaster)
{
	bool bVar1;
	edLIST* pCVar2;
	edLIST* pCVar3;
	edLIST* pCVar4;
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
	pCVar3 = (edLIST*)pCVar4->pLoadedData;
	if (pCVar3 != pCVar4) {
		for (; pCVar3 != pCVar4; pCVar3 = (edLIST*)pCVar3->pLoadedData) {
			pMVar2 = (DisplayListInternalMesh*)pCVar3->pCameraPanHeader_0xc;
			pcVar1 = (MeshData_CSTA*)pMVar2->pMeshInfo->CSTA;
			if (pcVar1 != (MeshData_CSTA*)0x0) {
				ps2_vu0_sub_vector(&local_10, (Vector*)pStaticMeshMaster->pCamera->aMatrices, &pcVar1->field_0x30);
				pMVar2->field_0x18 = local_10.z * local_10.z + local_10.x * local_10.x + local_10.y * local_10.y;
			}
		}
		while (bVar1) {
			bVar1 = false;
			pCVar2 = (edLIST*)pCVar4->pLoadedData;
			for (pCVar3 = (edLIST*)((edLIST*)pCVar4->pLoadedData)->pLoadedData;
				pCVar3 != pCVar4; pCVar3 = (edLIST*)pCVar3->pLoadedData) {
				pMVar1 = (DisplayListInternalMesh*)pCVar2->pCameraPanHeader_0xc;
				if (pMVar1->field_0x18 < ((DisplayListInternalMesh*)pCVar3->pCameraPanHeader_0xc)->field_0x18) {
					pCVar2->pCameraPanHeader_0xc = pCVar3->pCameraPanHeader_0xc;
					bVar1 = true;
					pCVar3->pCameraPanHeader_0xc = (edNODE_MANAGER*)pMVar1;
				}
				pCVar2 = pCVar3;
			}
		}
	}
	return;
}

int gFushListCounter = 0;
float FLOAT_00448a04 = 0.01f;

byte gGlobalAlhaON = 0;

TextureData_LAY_Internal* g_TextureData_LAY_Int_00449404 = NULL;
ed_g2d_bitmap* PTR_DAT_0044940c = NULL;

void ed3DG2DMaterialGetLayerBitmap(ed_dma_matrix* param_1, ed_g2d_bitmap** ppOutTEX2D, TextureData_LAY_Internal** ppOutLAY, int index)
{
	*ppOutLAY = &((ed_g2d_layer*)LOAD_SECTION(*(int*)(param_1->pFileData + index * 4 + 0x10)))->body;
	TextureData_TEX* pTEX = (TextureData_TEX*)LOAD_SECTION((*ppOutLAY)->pTex);
	TextureData_HASH_Internal_PA32* b = (TextureData_HASH_Internal_PA32*)LOAD_SECTION(pTEX->body.after.pHASH_Internal);
	TextureData_PA32* pTEX2D = (TextureData_PA32*)LOAD_SECTION(b->pPA32);
	*ppOutTEX2D = &pTEX2D->body;
	return;
}

byte gShadowFlushMode = 0;
int gFlushListFlusaON = 0;

edpkt_data* ed3DHierachyCheckForGlobalAlpha(edpkt_data* pRenderCommand, TextureData_LAY_Internal* pLAY)
{
	if (((gpLastMaterial == (ed_dma_matrix*)0x0) || ((gpLastMaterial->flags_0x28 & 1U) != 0)) &&
		(gpLastMaterial != (ed_dma_matrix*)0x0)) {
		gGlobalAlhaON = 0x80;
	}
	else {
		if (((gpCurHierarchy == (MeshTransformDataBase*)0x0) ||
			(gpCurHierarchy == (MeshTransformDataBase*)0x0000ffff)) ||
			(gpCurHierarchy->GlobalAlhaON == 0xff)) {
			if (((gGlobalAlhaON != 0x80) && (gpCurHierarchy != (MeshTransformDataBase*)0x0)) &&
				(((gpCurHierarchy != (MeshTransformDataBase*)0x0000ffff &&
					(((gpCurHierarchy->GlobalAlhaON == -1 &&
						(gCurViewportUsed != (ed_viewport*)0x0)) &&
						(gGlobalAlhaON = 0x80, pLAY != (TextureData_LAY_Internal*)0x0)))) && ((pLAY->flags_0x0 & 0xfc) == 0))
					)) {
				pRenderCommand->cmdA = 0x30000000;
				pRenderCommand->cmdB = 0x1300000000000000;
				pRenderCommand[1].cmdA = 0xe10000003;
				pRenderCommand[1].cmdB = 0;
				*(undefined4*)&pRenderCommand[1].cmdB = 0;
				*(undefined4*)((ulong)&pRenderCommand[1].cmdB + 4) = 0x50000003;
				pRenderCommand[2].cmdA = 0x1000000000008002;
				pRenderCommand[2].cmdB = 0xe;
				pRenderCommand[3].cmdA = 0x5001b;
				pRenderCommand[3].cmdB = 0x47;
				pRenderCommand[4].cmdA = 0x2a;
				pRenderCommand[4].cmdB = 0x42;
				pRenderCommand = pRenderCommand + 5;
			}
		}
		else {
			if (gCurViewportUsed != (ed_viewport*)0x0) {
				gGlobalAlhaON = gpCurHierarchy->GlobalAlhaON;
				IMPLEMENTATION_GUARD( pRenderCommand = ed3DHierachyCheckForGlobalAlphaSetPKT_(pRenderCommand, pLAY));
			}
		}
	}
	return pRenderCommand;
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
edF32MATRIX4 g_RenderMData[4] = { 0 };
edF32MATRIX4 Matrix_70000a00 = { 0 };
edF32MATRIX4* Matrix_70000a00_PTR = &Matrix_70000a00;
#else
Vector* Vector_ARRAY_70000800 = (Vector*)0x70000800;
Vector* Vector_ARRAY_70000990 = (Vector*)0x70000990;
Vector* Vector_ARRAY_70000920 = (Vector*)0x70000920;
edF32MATRIX4* g_RenderMData = (edF32MATRIX4*)0x70000820;
edF32MATRIX4* Matrix_70000a00_PTR = (edF32MATRIX4*)0x70000a00;
#endif

// These are in the scratchpad
edF32MATRIX4* PTR_Matrix_70000a40 = NULL;
edF32MATRIX4* PTR_Matrix_70000a50 = NULL;
Vector* PTR_Vector_70000a60 = NULL;

edF32MATRIX4* ed3DPKTCopyMatrixPacket(edF32MATRIX4* pMatrixBuffer, ed_dma_matrix* pRenderData, byte param_3)
{
	edF32MATRIX4* m1;
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
	RENDER_LOG("DMA Begin ed3DPKTCopyMatrixPacket\n");
	edDmaSync(SHELLDMA_CHANNEL_VIF0);
	sceVu0ScaleVectorAlt(pRenderData->field_0x2c.floatValue, Vector_ARRAY_70000800, &Vector_0048c310);
	sceVu0ScaleVectorAlt(pRenderData->field_0x2c.floatValue, Vector_ARRAY_70000800 + 1, &Vector_0048c320);
	*ScratchpadAnimManager_004494ec.field_0x20 = m1;
	if (((pRenderData->pMeshTransformData == (MeshTransformDataBase*)0x0) ||
		(puVar1 = pRenderData->pMeshTransformData->pLightingMatrixFuncObj_0xa0, puVar1 == (LightingMatrixFuncObj*)0x0))
		|| (pLVar1 = puVar1->field_0x8, pLVar1 == (LightingMatrixSubSubObj*)0x0)) {
		PTR_Matrix_70000a40 = ScratchpadAnimManager_004494ec.pLightDirections;
		PTR_Matrix_70000a50 = ScratchpadAnimManager_004494ec.pLightColor;
		PTR_Vector_70000a60 = ScratchpadAnimManager_004494ec.pLightAmbient;
	}
	else {
		PTR_Matrix_70000a40 = pLVar1->field_0x4;
		if (PTR_Matrix_70000a40 == (edF32MATRIX4*)0x0) {
			PTR_Matrix_70000a40 = ScratchpadAnimManager_004494ec.pLightDirections;
		}
		PTR_Matrix_70000a50 = pLVar1->field_0x8;
		if (PTR_Matrix_70000a50 == (edF32MATRIX4*)0x0) {
			PTR_Matrix_70000a50 = ScratchpadAnimManager_004494ec.pLightColor;
		}
		PTR_Vector_70000a60 = pLVar1->field_0x0;
		if (PTR_Vector_70000a60 == (Vector*)0x0) {
			PTR_Vector_70000a60 = ScratchpadAnimManager_004494ec.pLightAmbient;
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
	sceVu0TransposeMatrixFixed((TO_SCE_MTX)Vector_ARRAY_70000920, (TO_SCE_MTX)Vector_ARRAY_70000920);
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
	RENDER_LOG("DMA End ed3DPKTCopyMatrixPacket\n");
	edDmaSync(SHELLDMA_CHANNEL_VIF0);
	edDmaLoadFromFastRam_nowait(Vector_ARRAY_70000800, 0x1c0, (uint)((ulong)((long)(int)pMatrixBuffer << 0x24) >> 0x24));
	return pMatrixBuffer + 7;
}

edpkt_data* ed3DPKTAddMatrixPacket(edpkt_data* pPkt, ed_dma_matrix* pDmaMatrix)
{
	ushort uVar1;
	MeshTransformSpecial* pMVar2;
	MeshTransformDataBase* pMVar3;
	float fVar4;
	bool bVar5;
	edpkt_data* pRVar6;
	int iVar7;
	edF32MATRIX4* pMVar8;
	edpkt_data* pRVar9;
	edF32MATRIX4* pMVar10;
	byte bVar11;

	RENDER_LOG("ed3DPKTAddMatrixPacket %p\n", pDmaMatrix->pMeshTransformData);

	pMVar2 = pDmaMatrix->field_0x4;
	while (true) {
		uVar1 = pMVar2->specUnion.field_0x0[0];
		bVar11 = 0;
		if (uVar1 == 4) {
			if (((pMVar2->pRenderInput[1].subCommandBufferCount & 0x280) == 0) &&
				(pDmaMatrix->pMeshTransformData != &MeshTransformDataBase_0048cad0)) {
				bVar11 = 1;
			}
		}
		else {
			if (uVar1 == 6) {
				bVar11 = 1;
			}
		}
		if (gFushListCounter == 0xe) {
			IMPLEMENTATION_GUARD(pPkt = FUN_002a9220(pDmaMatrix->pTransformMatrix, pPkt);)
		}
		pMVar10 = gPKTMatrixCur;
		pMVar3 = pDmaMatrix->pMeshTransformData;
		if (pMVar3 != (MeshTransformDataBase*)0x0) break;
		pMVar10 = pDmaMatrix->pTransformMatrix;
		bVar5 = false;

		PRINT_MATRIX(pMVar10);

		if ((((pMVar10->ac + pMVar10->aa + pMVar10->ab == 1.0) &&
			(bVar5 = false, pMVar10->bc + pMVar10->ba + pMVar10->bb == 1.0)) &&
			(bVar5 = false, pMVar10->cc + pMVar10->ca + pMVar10->cb == 1.0)) &&
			(bVar5 = true, pMVar10->dd + pMVar10->dc + pMVar10->da + pMVar10->db != 1.0)) {
			bVar5 = false;
		}
		if (!bVar5) {
			IMPLEMENTATION_GUARD(
			pPkt->cmdA = 0xe1000001c;
			pPkt->cmdB = 0;
			*(undefined4*)&pPkt->cmdB = 0;
			*(undefined4*)((ulong)&pPkt->cmdB + 4) = 0x6c1c0006;
			pRVar6 = (edpkt_data*)ed3DPKTCopyMatrixPacket((int)(pPkt + 1), (ed_dma_matrix*)pDmaMatrix, bVar11);
			*(uint*)ScratchpadAnimManager_004494ec.field_0x1c =
				(uint)((ulong)((long)(int)(pPkt + 1) << 0x24) >> 0x24) + 0xa0;
			if (bVar11 != 0) {
				pRVar9 = pRVar6 + 1;
				pRVar6->cmdA = 0xe10000004;
				pRVar6->cmdB = 0;
				*(undefined4*)&pRVar6->cmdB = 0;
				iVar7 = 8;
				*(undefined4*)((ulong)&pRVar6->cmdB + 4) = 0x6c0403fb;
				pMVar10 = pDmaMatrix->pTransformMatrix;
				do {
					iVar7 = iVar7 + -1;
					fVar4 = pMVar10->ab;
					*(float*)&pRVar9->cmdA = pMVar10->aa;
					pMVar10 = (edF32MATRIX4*)&pMVar10->ac;
					*(float*)((ulong)&pRVar9->cmdA + 4) = fVar4;
					pRVar9 = (edpkt_data*)&pRVar9->cmdB;
				} while (0 < iVar7);
				pRVar6 = pRVar6 + 5;
			}
			return pRVar6;)
		}
		pDmaMatrix->pMeshTransformData = &MeshTransformDataBase_0048cad0;
		RENDER_LOG("Storing pMeshTransformData %p\n", &MeshTransformDataBase_0048cad0);
	}
	if (pMVar3->field_0xa4 == (edF32MATRIX4*)0x0) {
		pMVar3->field_0xa4 = gPKTMatrixCur;
		gPKTMatrixCur = ed3DPKTCopyMatrixPacket(pMVar10, pDmaMatrix, bVar11);
		if (bVar11 != 0) {
			pMVar10 = pDmaMatrix->pTransformMatrix;
			iVar7 = 8;
			pMVar8 = gPKTMatrixCur;
			do {
				iVar7 = iVar7 + -1;
				fVar4 = pMVar10->ab;
				pMVar8->aa = pMVar10->aa;
				pMVar10 = (edF32MATRIX4*)&pMVar10->ac;
				pMVar8->ab = fVar4;
				pMVar8 = (edF32MATRIX4*)&pMVar8->ac;
			} while (0 < iVar7);
			gPKTMatrixCur = gPKTMatrixCur + 1;
		}
	}
	*(uint*)ScratchpadAnimManager_004494ec.field_0x1c =
		(uint)((ulong)((ulong)(int)pDmaMatrix->pMeshTransformData->field_0xa4 << 0x24) >> 0x24) + 0xa0;
	pPkt->cmdA = ((ulong)(int)pDmaMatrix->pMeshTransformData->field_0xa4 & 0xfffffffU) << 0x20 | 0x3000001c;
	pPkt->cmdB = 0x6c1c000600000000;
	if (bVar11 == 0) {
		return pPkt + 1;
	}
	pPkt[1].cmdA = ((ulong)(int)(pDmaMatrix->pMeshTransformData->field_0xa4 + 7) & 0xfffffffU) << 0x20 | 0x30000004;
	pPkt[1].cmdB = 0x6c0403fb00000000;
	return pPkt + 2;
}

edpkt_data* g_TempCommandBuffer_004493b0 = NULL;
edpkt_data* g_CommandScratchpadBase_004489a0 = NULL;

uint ed3DFlushStripGetIncPacket(ed_3d_strip* param_1, int param_2, long bUpdateInternal)
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

ed_3D_Scene* gCurScene = NULL;

edpkt_data*
	ed3DFlushStripInit(edpkt_data* pDisplayList, MeshTransformSpecial* pMeshTransformSpecial, ulong mode)
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
	edF32MATRIX4* pMVar13;
	uint uVar14;
	short* pAnimIndexes;
	edpkt_data* ppuVar15;
	Vector* pVVar15;
	Vector* pVVar16;

	pDVar3 = pMeshTransformSpecial->pRenderInput;
	uVar2 = pMeshTransformSpecial->specUnion.field_0x0[1];
	(ScratchpadAnimManager_004494ec.pAnimScratchpad)->field_0x0 = (int)(short)uVar2;
	(ScratchpadAnimManager_004494ec.pAnimScratchpad)->flags = 0;
	(ScratchpadAnimManager_004494ec.pAnimScratchpad)->field_0x8 = 0;
	(ScratchpadAnimManager_004494ec.pAnimScratchpad)->field_0xc = 0;
	if ((gFushListCounter == 0xe) && (FLOAT_00448a04 != 0.0)) {
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
	if (((*(uint*)pDVar3 & 0x10000) != 0) && (gpCurHierarchy != (MeshTransformDataBase*)0x0)) {
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
			(pMVar13 = gpCurHierarchy->pShadowAnimMatrix, pMVar13 == (edF32MATRIX4*)0x0)) {
			pMVar13 = gpCurHierarchy->pAnimMatrix;
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
		ppuVar15 = (edpkt_data*)pVVar15;
		if (g_TempCommandBuffer_004493b0 != (edpkt_data*)0x0) {
			IMPLEMENTATION_GUARD(
			pVVar16 = (Vector*)((int)pVVar15 - (int)g_CommandScratchpadBase_004489a0);
			RENDER_LOG("DMA Begin ed3DFlushStripInit");
			edDmaSync(SHELLDMA_CHANNEL_VIF0);
			edDmaLoadFromFastRam_nowait((Vector*)g_CommandScratchpadBase_004489a0, (uint)pVVar16,
				(uint)((ulong)((long)(int)g_TempCommandBuffer_004493b0 << 0x24) >> 0x24));
			g_TempCommandBuffer_004493b0 =
				(edpkt_data*)((ulong)&g_TempCommandBuffer_004493b0->cmdA + ((uint)pVVar16 & 0xfffffff0));
			ppuVar15 = (edpkt_data*)&Vector_70000800;
			g_CommandScratchpadBase_004489a0 = (edpkt_data*)pVVar15;)
		}
		for (; (bVar1 = iVar12 != 0, iVar12 = iVar12 + -1, bVar1 && (-1 < *pAnimIndexes)); pAnimIndexes = pAnimIndexes + 1)
		{
			IMPLEMENTATION_GUARD(
			*(ulong*)&((Vector*)ppuVar15)->x = 0xe10000004;
			*(ulong*)&((Vector*)ppuVar15)->z = (ulong)(uVar14 | 0x6c040000) << 0x20;
			sceVu0CopyMatrix((edF32MATRIX4*)((Vector*)ppuVar15 + 1), pMVar13 + *pAnimIndexes);
			ppuVar15 = (edpkt_data*)((Vector*)ppuVar15 + 5);
			uVar14 = uVar14 + 4;
			)
		}
		if (g_TempCommandBuffer_004493b0 != (edpkt_data*)0x0) {
			IMPLEMENTATION_GUARD(
			RENDER_LOG("DMA Begin ed3DFlushStripInit");
			edDmaSync(SHELLDMA_CHANNEL_VIF0);
			pVVar15 = (Vector*)ppuVar15 + -0x7000080;
			edDmaLoadFromFastRam_nowait(&Vector_70000800, (uint)pVVar15,
				(uint)((ulong)((long)(int)g_TempCommandBuffer_004493b0 << 0x24) >> 0x24));
			edDmaSync(SHELLDMA_CHANNEL_VIF0);
			ppuVar15 = RenderCommand_ARRAY_70001000;
			g_CommandScratchpadBase_004489a0 = RenderCommand_ARRAY_70001000;
			g_TempCommandBuffer_004493b0 =
				(edpkt_data*)((ulong)&g_TempCommandBuffer_004493b0->cmdA + ((uint)pVVar15 & 0xfffffff0));)
		}
	}
	if (((gpCurHierarchy == (MeshTransformDataBase*)0x0) ||
		(gpCurHierarchy == (MeshTransformDataBase*)0x0000ffff)) ||
		(gpCurHierarchy == &MeshTransformDataBase_0048cad0)) {
		*(uint*)pDVar3 = *(uint*)pDVar3 & 0xfffffff7;
	}
	else {
		if ((BYTE_004493b4 == 0) && ((gpCurHierarchy->flags_0x9e & 2) == 0)) {
			*(uint*)pDVar3 = *(uint*)pDVar3 & 0xfffffff7;
		}
		else {
			*(uint*)pDVar3 = *(uint*)pDVar3 | 8;
			*(ulong*)&((Vector*)ppuVar15)->x = 0x30000000;
			*(ulong*)&((Vector*)ppuVar15)->z = 0x1100000011000000;
			if ((BYTE_004493b8 == 0) && ((gCurScene->flags_0x4 & 0x200) == 0)) {
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
			ppuVar15 = (edpkt_data*)((Vector*)ppuVar15 + 3);
		}
	}
	if (((*(uint*)pDVar3 & 0x100) != 0) || (gFushListCounter == 0xe)) {
		*(ulong*)&((Vector*)ppuVar15)->x = 0xe10000001;
		*(ulong*)&((Vector*)ppuVar15)->z = 0;
		((Vector*)ppuVar15)->z = 0.0;
		*(undefined4*)((ulong)&((Vector*)ppuVar15)->z + 4) = 0x6c010021;
		((Vector*)ppuVar15)[1].x = (ScratchpadAnimManager_004494ec.pVector_0x18)->x;
		*(float*)((ulong)&((Vector*)ppuVar15)[1].x + 4) = (ScratchpadAnimManager_004494ec.pVector_0x18)->y;
		((Vector*)ppuVar15)[1].z = (ScratchpadAnimManager_004494ec.pVector_0x18)->z;
		*(float*)((ulong)&((Vector*)ppuVar15)[1].z + 4) = (ScratchpadAnimManager_004494ec.pVector_0x18)->w;
		ppuVar15 = (edpkt_data*)((Vector*)ppuVar15 + 2);
	}
	return ppuVar15;
}

void ed3DFlushStrip(MeshTransformSpecial* pMeshTransformSpecial)
{
	bool bVar1;
	undefined8 uVar2;
	float fVar3;
	char* pcVar4;
	uint uVar5;
	edpkt_data* ppuVar6;
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
	edpkt_data* pRVar18;
	edpkt_data* ppuVar19;
	edpkt_data* ppuVar20;
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
	ed_3d_strip* pRenderInput;

	pRVar18 = g_VifRefPktCur;
	pRenderInput = (ed_3d_strip*)pMeshTransformSpecial->pRenderInput;
	pMVar19 = (MeshTransformSpecial*)LOAD_SECTION((int)pMeshTransformSpecial->pPrev_0x8);
	uVar20 = (ulong)&pRenderInput->flags_0x0 + pRenderInput->offsetA;
	uVar5 = ed3DFlushStripGetIncPacket(pRenderInput, 0, 0);
	fVar11 = (float)(uint)gGlobalAlhaON;
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
		ppuVar20 = ed3DFlushStripInit(pRVar18, pMeshTransformSpecial, 1);
		if ((edpkt_data*)0x70003000 < ppuVar20 + (uint)(ushort)pRenderInput->meshSectionCount_0x3a * 2) {
			IMPLEMENTATION_GUARD(
			pRVar18 = (edpkt_data*)((int)ppuVar20 - (int)g_CommandScratchpadBase_004489a0);
			edDmaLoadFromFastRam(g_CommandScratchpadBase_004489a0, (uint)pRVar18,
				(uint)((ulong)((long)(int)g_TempCommandBuffer_004493b0 << 0x24) >> 0x24));
			g_CommandScratchpadBase_004489a0 = (edpkt_data*)&PTR_DAT_70001000;
			g_TempCommandBuffer_004493b0 =
				(edpkt_data*)((ulong)&g_TempCommandBuffer_004493b0->cmdA + ((uint)pRVar18 & 0xfffffff0));
			ppuVar20 = (edpkt_data*)&PTR_DAT_70001000;)
		}
		pcVar4 = ScratchpadAnimManager_004494ec.field_0x34;
		if ((pRenderInput->flags_0x0 & 4) == 0) {
			IMPLEMENTATION_GUARD(
			ppuVar6 = ppuVar20;
			while (bVar1 = uVar8 != 0, uVar8 = uVar8 - 1, bVar1) {
				ppuVar6->cmdA = 0x30000000;
				uVar23 = ed3DVU1BufferCur;
				uVar7 = uVar20 & 0xfffffff;
				uVar20 = uVar20 + uVar5 * 0x10;
				iVar13 = ed3DVU1BufferCur * 4;
				ppuVar6->cmdB = (long)(int)(*(int*)(pcVar4 + iVar13) + 1U | 0x3000000) & 0xffffffffU | 0x200000000000000;
				ppuVar6[1].cmdA = (ulong)uVar7 << 0x20 | 0x50000000;
				ppuVar6[1].cmdB = (long)(int)(*(int*)(pcVar4 + iVar13) + 1U | 0x4000000) << 0x20;
				ppuVar6 = ppuVar6 + 2;
				if (uVar23 == 2) {
					ed3DVU1BufferCur = 0;
				}
				else {
					ed3DVU1BufferCur = ed3DVU1BufferCur + 1;
				}
			}
			piVar14 = (int*)(ScratchpadAnimManager_004494ec.field_0x38 + ed3DVU1BufferCur * 0xc);
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
			iVar6 = ed3DG3DAnimGet(pRenderInput);
			uVar8 = ed3DG3DNbMaxBaseRGBA(pRenderInput);
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
				uVar7 = ed3DVU1BufferCur;
				uVar12 = (uint)piVar21 & 0xfffffff;
				uVar24 = uVar20 & 0xfffffff;
				iVar6 = ed3DVU1BufferCur * 4;
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
					ed3DVU1BufferCur = 0;
				}
				else {
					ed3DVU1BufferCur = ed3DVU1BufferCur + 1;
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
		ppuVar6 = ed3DFlushStripInit(pRVar18, pMeshTransformSpecial, 1);
#ifdef PLATFORM_WIN
		if ((edpkt_data*)(g_ScratchpadAddr_00448a58 + 0xC0) < ppuVar6 + (int)pRenderInput->meshSectionCount_0x3a * 4) {
#else
		if ((edpkt_data*)0x70003000 < ppuVar6 + (int)pRenderInput->meshSectionCount_0x3a * 4) {
#endif
			IMPLEMENTATION_GUARD(
			pRVar18 = (edpkt_data*)((int)ppuVar6 - (int)g_CommandScratchpadBase_004489a0);
			edDmaLoadFromFastRam(g_CommandScratchpadBase_004489a0, (uint)pRVar18,
				(uint)((ulong)((long)(int)g_TempCommandBuffer_004493b0 << 0x24) >> 0x24));
			g_CommandScratchpadBase_004489a0 = (edpkt_data*)&PTR_DAT_70001000;
			g_TempCommandBuffer_004493b0 =
				(edpkt_data*)((ulong)&g_TempCommandBuffer_004493b0->cmdA + ((uint)pRVar18 & 0xfffffff0));
			ppuVar6 = (edpkt_data*)&PTR_DAT_70001000;)
		}
		if ((pRenderInput->flags_0x0 & 4) == 0) {
			while (pRVar25 < pRenderInput->meshSectionCount_0x3a) {
				bVar9 = pMVar19->specUnion.byteFlags[0] & 3;
				if (bVar9 == 1) {
					uVar8 = uVar8 & 0xfffffffd | 1;
				LAB_002a5dac:
#ifdef PLATFORM_WIN
					if ((edpkt_data*)(g_ScratchpadAddr_00448a58 + 0x100) < ppuVar6 + 0x80) {
#else
					if ((edpkt_data*)0x70003fff < ppuVar6 + 0x80) {
#endif
						RENDER_LOG("ed3DFlushStrip");
						pRVar18 = (edpkt_data*)((ulong)ppuVar6 - (ulong)g_CommandScratchpadBase_004489a0);
						edDmaLoadFromFastRam((uint)g_CommandScratchpadBase_004489a0, (uint)pRVar18,
							(uint)((ulong)((long)(int)g_TempCommandBuffer_004493b0 << 0x24) >> 0x24));
						g_CommandScratchpadBase_004489a0 = (edpkt_data*)(g_ScratchpadAddr_00448a58 + 0x40);
						g_TempCommandBuffer_004493b0 =
							(edpkt_data*)((ulong)&g_TempCommandBuffer_004493b0->cmdA + ((uint)pRVar18 & 0xfffffff0));
						ppuVar6 = (edpkt_data*)(g_ScratchpadAddr_00448a58 + 0x40);
					}
					(ScratchpadAnimManager_004494ec.pAnimScratchpad)->field_0x0 = uVar8;
					uVar7 = ed3DVU1BufferCur;
					iVar15 = ed3DVU1BufferCur * 0x10;
					iVar13 = ed3DVU1BufferCur * 4;
					iVar6 = ed3DVU1BufferCur * 2;
					*ppuVar6 = RenderCommand_ARRAY_0048c5f0[ed3DVU1BufferCur];
					uVar2 = *(undefined8*)ScratchpadAnimManager_004494ec.pAnimScratchpad;
					uVar16 = (ScratchpadAnimManager_004494ec.pAnimScratchpad)->field_0x8;
					uVar17 = (ScratchpadAnimManager_004494ec.pAnimScratchpad)->field_0xc;
					*(int*)&ppuVar6[1].cmdA = (int)uVar2;
					*(int*)((ulong)&ppuVar6[1].cmdA + 4) = (int)((ulong)uVar2 >> 0x20);
					*(undefined4*)&ppuVar6[1].cmdB = uVar16;
					*(undefined4*)((ulong)&ppuVar6[1].cmdB + 4) = uVar17;
					uVar2 = g_StaticDisplayListData_0048c340_LONG[iVar6];
					uVar16 = gRefOptionsforVU1Buf[uVar7 * 4 + 2];
					uVar17 = gRefOptionsforVU1Buf[uVar7 * 4 + 3];
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
						ed3DVU1BufferCur = 0;
					}
					else {
						ed3DVU1BufferCur = ed3DVU1BufferCur + 1;
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
					if ((edpkt_data*)(g_ScratchpadAddr_00448a58 + 0x100) < ppuVar6 + 0x80) {
#else
					if ((edpkt_data*)0x70003fff < ppuVar6 + 0x80) {
#endif
						pRVar18 = (edpkt_data*)((ulong)ppuVar6 - (ulong)g_CommandScratchpadBase_004489a0);
						edDmaLoadFromFastRam((uint)g_CommandScratchpadBase_004489a0, (uint)pRVar18,
							(uint)((ulong)((long)(int)g_TempCommandBuffer_004493b0 << 0x24) >> 0x24));
						g_CommandScratchpadBase_004489a0 = (edpkt_data*)(g_ScratchpadAddr_00448a58 + 0x40);
						g_TempCommandBuffer_004493b0 =
							(edpkt_data*)((ulong)&g_TempCommandBuffer_004493b0->cmdA + ((uint)pRVar18 & 0xfffffff0));
						ppuVar6 = (edpkt_data*)(g_ScratchpadAddr_00448a58 + 0x40);
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
			iVar6 = ed3DG3DAnimGet(pRenderInput);
			uVar7 = ed3DG3DNbMaxBaseRGBA(pRenderInput);
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
					if ((edpkt_data*)0x70003fff < ppuVar19 + 4) {
						pRVar18 = (edpkt_data*)((int)ppuVar19 - (int)g_CommandScratchpadBase_004489a0);
						edDmaLoadFromFastRam(g_CommandScratchpadBase_004489a0, (uint)pRVar18,
							(uint)((ulong)((long)(int)g_TempCommandBuffer_004493b0 << 0x24) >> 0x24));
						g_CommandScratchpadBase_004489a0 = (edpkt_data*)&PTR_DAT_70001000;
						g_TempCommandBuffer_004493b0 =
							(edpkt_data*)((ulong)&g_TempCommandBuffer_004493b0->cmdA + ((uint)pRVar18 & 0xfffffff0));
						ppuVar19 = (edpkt_data*)&PTR_DAT_70001000;
					}
					if (pRVar25 == (edpkt_data*)((ushort)pRenderInput->meshSectionCount_0x3a - 1)) {
						for (local_20 = (uint)pRenderInput->field_0x38; (local_20 & 3) != 0; local_20 = local_20 + 1) {
						}
					}
					uVar10 = (long)(int)(local_20 >> 2) | 0x30000000;
					(ScratchpadAnimManager_004494ec.pAnimScratchpad)->field_0x0 = uVar8;
					uVar7 = ed3DVU1BufferCur;
					iVar15 = ed3DVU1BufferCur * 0x10;
					iVar6 = ed3DVU1BufferCur * 2;
					iVar13 = ed3DVU1BufferCur * 4;

					*ppuVar19 = RenderCommand_ARRAY_0048c5f0[ed3DVU1BufferCur];

					uVar2 = *(undefined8*)ScratchpadAnimManager_004494ec.pAnimScratchpad;
					uVar16 = (ScratchpadAnimManager_004494ec.pAnimScratchpad)->field_0x8;
					uVar17 = (ScratchpadAnimManager_004494ec.pAnimScratchpad)->field_0xc;
					*(int*)&ppuVar19[1].cmdA = (int)uVar2;
					*(int*)((ulong)&ppuVar19[1].cmdA + 4) = (int)((ulong)uVar2 >> 0x20);
					*(undefined4*)&ppuVar19[1].cmdB = uVar16;
					*(undefined4*)((ulong)&ppuVar19[1].cmdB + 4) = uVar17;
					uVar2 = g_StaticDisplayListData_0048c340_LONG[iVar6];
					uVar16 = gRefOptionsforVU1Buf[uVar7 * 4 + 2];
					uVar17 = gRefOptionsforVU1Buf[uVar7 * 4 + 3];
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
						ed3DVU1BufferCur = 0;
					}
					else {
						ed3DVU1BufferCur = ed3DVU1BufferCur + 1;
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
	if (ed3DVU1BufferCur == 2) {
		ed3DVU1Buffer[0] = *(uint*)(ScratchpadAnimManager_004494ec.field_0x34 + 4);
		ed3DVU1Buffer[1] = *(uint*)(ScratchpadAnimManager_004494ec.field_0x34 + 8);
		ed3DVU1Buffer[2] = *(uint*)ScratchpadAnimManager_004494ec.field_0x34;
	}
	else {
		if (ed3DVU1BufferCur == 1) {
			ed3DVU1Buffer[0] = *(uint*)ScratchpadAnimManager_004494ec.field_0x34;
			ed3DVU1Buffer[1] = *(uint*)(ScratchpadAnimManager_004494ec.field_0x34 + 4);
			ed3DVU1Buffer[2] = *(uint*)(ScratchpadAnimManager_004494ec.field_0x34 + 8);
		}
		else {
			if (ed3DVU1BufferCur == 0) {
				ed3DVU1Buffer[0] = *(uint*)(ScratchpadAnimManager_004494ec.field_0x34 + 8);
				ed3DVU1Buffer[1] = *(uint*)ScratchpadAnimManager_004494ec.field_0x34;
				ed3DVU1Buffer[2] = *(uint*)(ScratchpadAnimManager_004494ec.field_0x34 + 4);
			}
		}
	}
	if ((*ScratchpadAnimManager_004494ec.gShadowRenderMask != 0) && (pRenderInput->bUseShadowMatrix_0x30 != 0)) {
		ppuVar6->cmdA = 0x30000000;
		ppuVar6->cmdB = 0;
		*(undefined4*)&ppuVar6->cmdB = 0x13000000;
		ppuVar6 = ppuVar6 + 1;
	}
	g_VifRefPktCur = ppuVar6;
	return;
}

void ed3DFlushStripList(ed_dma_matrix* pRenderData, ed_g2d_material* pFileData)
{
	short sVar1;
	ushort uVar2;
	edNODE_MANAGER* pMVar3;
	DisplayListInternal* pDVar4;
	bool bVar5;
	edpkt_data* pRVar6;
	MeshTransformSpecial* pRVar5;

	pRVar5 = pRenderData->field_0x4;
	if ((ed_dma_matrix*)pRVar5 != pRenderData) {
		*ScratchpadAnimManager_004494ec.field_0x3c = *ScratchpadAnimManager_004494ec.field_0x3c + 1;
	}
	if ((pFileData == &ed_g2d_material_0048c3c0) || (pFileData == &ed_g2d_material_0048c3b0)) {
		IMPLEMENTATION_GUARD(
		g_TempCommandBuffer_004493b0 = (edpkt_data*)0x0;
		if (gShadowFlushMode == 0) {
			if (pFileData->count_0x0 < 2) {
				bVar5 = false;
				while (pRVar6 = g_VifRefPktCur, (ed_dma_matrix*)pRVar5 != pRenderData) {
					sVar1 = *(short*)&((ed_dma_matrix*)pRVar5)->pFileData;
					if (sVar1 == 4) {
					LAB_002a7d54:
						pRVar5 = ((ed_dma_matrix*)pRVar5)->field_0x4;
					}
					else {
						if ((sVar1 == 5) || (sVar1 == 6)) {
							pMVar3 = ((ed_dma_matrix*)pRVar5)->field_0xc;
							*(undefined4*)&g_VifRefPktCur->cmdA = *(undefined4*)pMVar3->name;
							*(MeshTransformSpecial**)((ulong)&g_VifRefPktCur->cmdA + 4) = pMVar3->pNextFree;
							*(int*)&g_VifRefPktCur->cmdB = pMVar3->totalCount;
							*(int*)((ulong)&g_VifRefPktCur->cmdB + 4) = pMVar3->usedCount;
							g_VifRefPktCur = g_VifRefPktCur + 1;
							goto LAB_002a7d54;
						}
						if (sVar1 == 3) {
							if ((*(uint*)((RenderFrame_30_B*)pRVar5)->field_0xc->name & 0x10000) == 0) {
								ed3DFlushStripShadowRender(pRVar5, (byte*)pFileData);
							}
							else {
								bVar5 = true;
								ed3DFlushStrip(pRVar5);
							}
							goto LAB_002a7d54;
						}
						pRVar5 = ((RenderFrame_30_B*)pRVar5)->field_0x4;
					}
				}
				if (bVar5) {
					g_VifRefPktCur->cmdA = 0x30000000;
					pRVar6->cmdB = 0x1100000011000000;
					g_VifRefPktCur = g_VifRefPktCur + 1;
				}
			}
		}
		else {
			while ((RenderFrame_30_B*)pRVar5 != pRenderData) {
				uVar2 = pRVar5->specUnion.field_0x0[0];
				if ((uVar2 == 5) || (uVar2 == 6)) {
					pDVar4 = pRVar5->pRenderInput;
					*(undefined4*)&g_VifRefPktCur->cmdA = *(undefined4*)pDVar4;
					*(undefined4*)((ulong)&g_VifRefPktCur->cmdA + 4) = *(undefined4*)&pDVar4->subCommandBufferCount;
					*(undefined4*)&g_VifRefPktCur->cmdB = pDVar4->field_0x8;
					*(char**)((ulong)&g_VifRefPktCur->cmdB + 4) = pDVar4->field_0xc;
					g_VifRefPktCur = g_VifRefPktCur + 1;
				LAB_002a7c6c:
					pRVar5 = pRVar5->pNext_0x4;
				}
				else {
					if (uVar2 == 4) goto LAB_002a7c6c;
					if (uVar2 == 3) {
						IMPLEMENTATION_GUARD(ed3DFlushStripForZbufferOnly((int)pRVar5, (byte*)pFileData));
						goto LAB_002a7c6c;
					}
					pRVar5 = pRVar5->pNext_0x4;
				}
			}
		})
	}
	else {
#ifdef PLATFORM_WIN
		g_CommandScratchpadBase_004489a0 = (edpkt_data*)(g_ScratchpadAddr_00448a58 + 0x40);
		g_TempCommandBuffer_004493b0 = g_VifRefPktCur;
		g_VifRefPktCur = (edpkt_data*)(g_ScratchpadAddr_00448a58 + 0x40);
#else
		g_CommandScratchpadBase_004489a0 = (edpkt_data*)0x70001000;
		g_TempCommandBuffer_004493b0 = g_VifRefPktCur;
		g_VifRefPktCur = (edpkt_data*)0x70001000;
#endif
		if (pFileData->count_0x0 < 2) {
			for (; (ed_dma_matrix*)pRVar5 != pRenderData; pRVar5 = ((ed_dma_matrix*)pRVar5)->field_0x4) {
				sVar1 = *(short*)&((ed_dma_matrix*)pRVar5)->pFileData;
				if ((sVar1 == 5) || (sVar1 == 6)) {
					IMPLEMENTATION_GUARD(
					pMVar3 = ((ed_dma_matrix*)pRVar5)->field_0xc;
					*(undefined4*)&g_VifRefPktCur->cmdA = *(undefined4*)pMVar3->name;
					*(MeshTransformSpecial**)((ulong)&g_VifRefPktCur->cmdA + 4) = pMVar3->pNextFree;
					*(int*)&g_VifRefPktCur->cmdB = pMVar3->totalCount;
					*(int*)((ulong)&g_VifRefPktCur->cmdB + 4) = pMVar3->usedCount;
					g_VifRefPktCur = g_VifRefPktCur + 1;
					)
				}
				else {
					if (sVar1 == 4) {
						IMPLEMENTATION_GUARD(
						ed3DFlushSprite(pRVar5, pFileData);)
					}
					else {
						if (sVar1 == 3) {
							ed3DFlushStrip(pRVar5);
						}
					}
				}
#ifdef PLATFORM_WIN
				if ((edpkt_data*)(g_ScratchpadAddr_00448a58 + 0xC0) < g_VifRefPktCur) {
#else
				if ((edpkt_data*)0x70003000 < g_VifRefPktCur) {
#endif
					IMPLEMENTATION_GUARD(
					pRVar6 = (edpkt_data*)((int)g_VifRefPktCur - (int)g_CommandScratchpadBase_004489a0);
					edDmaLoadFromFastRam(g_CommandScratchpadBase_004489a0, (uint)pRVar6, (uint)((ulong)((long)(int)g_TempCommandBuffer_004493b0 << 0x24) >> 0x24));
					g_VifRefPktCur = (edpkt_data*)&PTR_DAT_70001000;
					g_CommandScratchpadBase_004489a0 = &PTR_DAT_70001000;
					g_TempCommandBuffer_004493b0 = (edpkt_data*)((ulong)&g_TempCommandBuffer_004493b0->cmdA + ((uint)pRVar6 & 0xfffffff0));)
				}
			}
		}
		else {
			IMPLEMENTATION_GUARD(
			for (; (RenderFrame_30_B*)pRVar5 != pRenderData; pRVar5 = ((RenderFrame_30_B*)pRVar5)->field_0x4) {
				if (*(short*)&((RenderFrame_30_B*)pRVar5)->pFileData == 4) {
					ed3DFlushSprite(pRVar5, pFileData);
				}
				else {
					if (*(short*)&((RenderFrame_30_B*)pRVar5)->pFileData == 3) {
						ed3DFlushStripMultiTexture((int)pRVar5, (byte*)pFileData);
					}
				}
				if ((edpkt_data*)0x70003000 < g_VifRefPktCur) {
					pRVar6 = (edpkt_data*)((int)g_VifRefPktCur - (int)g_CommandScratchpadBase_004489a0);
					edDmaLoadFromFastRam(g_CommandScratchpadBase_004489a0, (uint)pRVar6, (uint)((ulong)((long)(int)g_TempCommandBuffer_004493b0 << 0x24) >> 0x24));
					g_VifRefPktCur = (edpkt_data*)&PTR_DAT_70001000;
					g_CommandScratchpadBase_004489a0 = &PTR_DAT_70001000;
					g_TempCommandBuffer_004493b0 = (edpkt_data*)((ulong)&g_TempCommandBuffer_004493b0->cmdA + ((uint)pRVar6 & 0xfffffff0));
				}
			})
		}
		pRVar6 = (edpkt_data*)((ulong)g_VifRefPktCur - (ulong)g_CommandScratchpadBase_004489a0);
		if (pRVar6 == (edpkt_data*)0x0) {
			g_VifRefPktCur = g_TempCommandBuffer_004493b0;
		}
		else {
			RENDER_LOG("DMA Begin ed3DFlushStripList");
			edDmaSync(SHELLDMA_CHANNEL_VIF0);
			edDmaLoadFromFastRam_nowait((Vector*)g_CommandScratchpadBase_004489a0, (uint)pRVar6, (uint)g_TempCommandBuffer_004493b0 & 0xfffffff);
			g_VifRefPktCur = (edpkt_data*)((ulong)&g_TempCommandBuffer_004493b0->cmdA + ((uint)pRVar6 & 0xfffffff0));
		}
	}
	return;
}

TextureData_LAY_Internal* ed3DGetG2DLayer(ed_g2d_material* pMAT, int index)
{
	TextureData_LAY_Internal* pTVar1;

	pTVar1 = (TextureData_LAY_Internal*)0x0;
	if (pMAT != (ed_g2d_material*)0x0) {
		if (pMAT->count_0x0 == 0) {
			pTVar1 = (TextureData_LAY_Internal*)0x0;
		}
		else {
			pTVar1 = (TextureData_LAY_Internal*)((char*)LOAD_SECTION(((int*)(pMAT + 1))[index]) + 0x10);
		}
	}
	return pTVar1;
}

bool ed3DFXIsTrue(TextureData_LAY_Internal* pLAY)
{
	return (pLAY->flags_0x0 & 0x7fff8000) != 0;
}

Vector Vector_004489bc = { 1.0f, 1.0f, 1.0f, 1.0f };
Vector Vector_004489cc = { 1.0f, 1.0f, 1.0f, 1.0f };

void ed3DFlushMatrix(ed_dma_matrix* pRenderData, ed_g2d_material* pFileData)
{
	byte bVar1;
	MeshTransformDataBase* pMVar2;
	float fVar3;
	edNODE_MANAGER* pMVar4;
	edpkt_data* pRVar5;
	uint* puVar6;
	ulong uVar7;
	bool bVar4;
	int iVar8;
	edF32MATRIX4* pMVar9;
	edF32MATRIX4* pMVar10;
	uint uVar11;
	edF32MATRIX4* m0;
	edF32MATRIX4* pMVar12;
	edF32MATRIX4 local_40;
	edpkt_data* puVar14;

	puVar14 = g_VifRefPktCur;
	pMVar2 = pRenderData->pMeshTransformData;
	/* Previous mesh may use the same base transform, so check if its different here. */
	if ((gpCurHierarchy != pMVar2) || (pMVar2 == (MeshTransformDataBase*)0x0)) {
		gpCurHierarchy = pMVar2;
		if (pMVar2 == (MeshTransformDataBase*)0x0) {
			gpCurHierarchy = (MeshTransformDataBase*)0x0000ffff;
		}
		g_VifRefPktCur->cmdA = 0x48959030000001;
		puVar14->cmdB = 0x1100000011000000;
		pRVar5 = ed3DPKTAddMatrixPacket(puVar14 + 1, pRenderData);
		pRVar5->cmdA = 0x30000000;
		puVar14 = pRVar5 + 1;
		pRVar5->cmdB = 0x1100000011000000;
	}
	pMVar12 = pRenderData->pTransformMatrix;
	if (((1 < pFileData->count_0x0) || (pMVar12 == &g_IdentityMatrix)) && (uVar11 = 0, gFushListCounter != 0xe))
	{
		bVar1 = pFileData->count_0x0;
		for (; uVar11 < bVar1; uVar11 = uVar11 + 1) {
			TextureData_LAY_Internal* pTVar6 = ed3DGetG2DLayer(pFileData, uVar11);
			if (((pTVar6 != (TextureData_LAY_Internal*)0x0) && (bVar4 = ed3DFXIsTrue(pTVar6), bVar4 != false)) ||
				(pMVar10 == &g_IdentityMatrix)) {
				IMPLEMENTATION_GUARD(
				m0 = (edF32MATRIX4*)(puVar14 + 3);
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
					pMVar10 = (edF32MATRIX4*)&pMVar10->ac;
					pMVar9->ab = fVar3;
					pMVar9 = (edF32MATRIX4*)&pMVar9->ac;
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
				sceVu0TransposeMatrixFixed(&local_40, &local_40);
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
					pMVar12 = (edF32MATRIX4*)&pMVar12->ac;
					pMVar9->ab = fVar3;
					pMVar9 = (edF32MATRIX4*)&pMVar9->ac;
				} while (0 < iVar8);
				*(undefined4*)&puVar14[6].cmdB = 0;
				*(undefined4*)((ulong)&puVar14[6].cmdA + 4) = 0;
				*(undefined4*)&puVar14[6].cmdA = 0;
				*(undefined4*)((ulong)&puVar14[5].cmdB + 4) = 0;
				*(undefined4*)((ulong)&puVar14[4].cmdB + 4) = 0;
				*(undefined4*)((ulong)&puVar14[3].cmdB + 4) = 0;
				*(undefined4*)((ulong)&puVar14[6].cmdB + 4) = 0x3f800000;
				sceVu0InverseMatrix(m0, m0);
				sceVu0TransposeMatrixFixed(m0, m0);
				puVar14 = puVar14 + 7;)
				break;
			}
			bVar1 = pFileData->count_0x0;
		}
	}
	g_VifRefPktCur = puVar14;
	puVar14 = ed3DHierachyCheckForGlobalAlpha(puVar14, g_TextureData_LAY_Int_00449404);
	g_VifRefPktCur = puVar14;
	puVar14->cmdA = 0xe10000001;
	puVar14->cmdB = 0;
	*(undefined4*)&puVar14->cmdB = 0;
	*(undefined4*)((ulong)&g_VifRefPktCur->cmdB + 4) = 0x6c010021;
	g_VifRefPktCur = g_VifRefPktCur + 1;
	*(float*)&g_VifRefPktCur->cmdA = (ScratchpadAnimManager_004494ec.pVector_0x18)->x;
	*(float*)((ulong)&g_VifRefPktCur->cmdA + 4) = (ScratchpadAnimManager_004494ec.pVector_0x18)->y;
	*(float*)&g_VifRefPktCur->cmdB = (ScratchpadAnimManager_004494ec.pVector_0x18)->z;
	*(float*)((ulong)&g_VifRefPktCur->cmdB + 4) = (ScratchpadAnimManager_004494ec.pVector_0x18)->w;
	g_VifRefPktCur = g_VifRefPktCur + 1;
	ed3DFlushStripList(pRenderData, pFileData);
	pMVar4 = (edNODE_MANAGER*)(pRenderData->internalMeshTransformSpecial).pNext_0x4;
	pRenderData->field_0x4 = pMVar4->pNextFree + pMVar4->usedCount;
	pMVar4->usedCount = pMVar4->usedCount - (int)(pRenderData->internalMeshTransformSpecial).pRenderInput;
	(pRenderData->internalMeshTransformSpecial).pRenderInput = (DisplayListInternal*)0x0;
	pRenderData->field_0x4 = (MeshTransformSpecial*)pRenderData;
	(pRenderData->internalMeshTransformSpecial).specUnion.randoPtr = pRenderData;
	return;
}

void ed3DFlushMaterialNode(ed_dma_matrix* pRenderFrame)
{
	edNODE_MANAGER* peVar1;
	MeshTransformSpecial* pRVar3;

	if ((pRenderFrame->flags_0x28 & 1U) == 0) {
		pRVar3 = (pRenderFrame->internalMeshTransformSpecial).pNext_0x4;
		if (pRVar3 != (MeshTransformSpecial*)0x0) {
			if (gShadowFlushMode == 0) {
				for (; pRVar3 != &pRenderFrame->internalMeshTransformSpecial; pRVar3 = pRVar3->pNext_0x4) {
					ed3DFlushMatrix((ed_dma_matrix*)pRVar3->pRenderInput, (ed_g2d_material*)pRenderFrame->pFileData);
				}
				peVar1 = (edNODE_MANAGER*)(pRenderFrame->internalMeshTransformSpecial).pRenderInput;
			}
			else {
				for (; pRVar3 != &pRenderFrame->internalMeshTransformSpecial; pRVar3 = pRVar3->pNext_0x4) {
					IMPLEMENTATION_GUARD(ed3DFlushMatrixForShadow((int)pRVar3->pRenderInput);
					ed3DFlushMatrix((ed_dma_matrix*)pRVar3->pRenderInput, (ed_g2d_material*)pRenderFrame->pFileData));
				}
				peVar1 = (edNODE_MANAGER*)(pRenderFrame->internalMeshTransformSpecial).pRenderInput;
			}
			(pRenderFrame->internalMeshTransformSpecial).pNext_0x4 = peVar1->pNextFree + peVar1->usedCount;
			peVar1->usedCount = peVar1->usedCount - pRenderFrame->index_0x1c;
			pRenderFrame->index_0x1c = 0;
			(pRenderFrame->internalMeshTransformSpecial).pNext_0x4 = &pRenderFrame->internalMeshTransformSpecial;
			(pRenderFrame->internalMeshTransformSpecial).pPrev_0x8 = &pRenderFrame->internalMeshTransformSpecial;
		}
		gFlushListFlusaON = 0;
	}
	else {
		if (gFlushListFlusaON == 0) {
			ed3DFlushMatrix(DmaMatrixDefault, (ed_g2d_material*)pRenderFrame->pFileData);
			gFlushListFlusaON = 1;
		}
		ed3DFlushStripList((ed_dma_matrix*)&pRenderFrame->internalMeshTransformSpecial, (ed_g2d_material*)pRenderFrame->pFileData);
		peVar1 = (edNODE_MANAGER*)(pRenderFrame->internalMeshTransformSpecial).pRenderInput;
		(pRenderFrame->internalMeshTransformSpecial).pNext_0x4 = peVar1->pNextFree + peVar1->usedCount;
		peVar1->usedCount = peVar1->usedCount - pRenderFrame->index_0x1c;
		pRenderFrame->index_0x1c = 0;
		(pRenderFrame->internalMeshTransformSpecial).pNext_0x4 = &pRenderFrame->internalMeshTransformSpecial;
		(pRenderFrame->internalMeshTransformSpecial).pPrev_0x8 = &pRenderFrame->internalMeshTransformSpecial;
	}
	return;
}


edpkt_data* ed3DFlushMaterialAnimST(edpkt_data* param_1)
{
	uint uVar1;
	uint uVar2;
	ushort uVar3;
	TextureData_TEX* pTVar4;
	float* pfVar5;
	int iVar6;
	uint uVar7;
	int iVar8;
	uint uVar9;
	float fVar10;
	float fVar11;
	float fVar12;

	uVar9 = g_TextureData_LAY_Int_00449404->field_0x4;
	if ((uVar9 & 0x10) == 0) {
		if ((uVar9 & 2) != 0) {
			IMPLEMENTATION_GUARD(
			pTVar4 = g_TextureData_LAY_Int_00449404->pTexB;
			ed3DG2DGetBitmapFromTexture((int)&pTVar4->body);
			iVar6 = *(int*)&(pTVar4->body).field_0x1c;
			iVar8 = ed3DG2DAnimTexGet(&pTVar4->body);
			fVar10 = *(float*)(iVar8 + 0x10);
			if (fVar10 < 2.147484e+09) {
				uVar9 = (uint)fVar10;
				uVar3 = *(ushort*)(iVar6 + 0x24);
			}
			else {
				uVar9 = (int)(fVar10 - 2.147484e+09) | 0x80000000;
				uVar3 = *(ushort*)(iVar6 + 0x24);
			}
			if (uVar3 <= uVar9) {
				uVar9 = uVar3 - 1;
			}
			uVar1 = 0x100 / *(int*)(iVar6 + 0x14);
			if (*(int*)(iVar6 + 0x14) == 0) {
				trap(7);
			}
			if (uVar9 < uVar1) {
				*(undefined4*)(*(int*)&(pTVar4->body).field_0x14 + 0xc) = 0;
			}
			else {
				uVar2 = (int)uVar9 / (int)uVar1;
				if (uVar1 == 0) {
					trap(7);
				}
				uVar7 = *(uint*)(iVar6 + 0x18);
				if ((int)uVar7 < 0) {
					fVar10 = (float)(uVar7 >> 1 | uVar7 & 1);
					fVar10 = fVar10 + fVar10;
				}
				else {
					fVar10 = (float)uVar7;
				}
				uVar7 = *(uint*)(iVar6 + 0x20);
				if ((int)uVar7 < 0) {
					fVar11 = (float)(uVar7 >> 1 | uVar7 & 1);
					fVar11 = fVar11 + fVar11;
				}
				else {
					fVar11 = (float)uVar7;
				}
				if ((int)uVar2 < 0) {
					fVar12 = (float)(uVar2 >> 1 | uVar2 & 1);
					fVar12 = fVar12 + fVar12;
				}
				else {
					fVar12 = (float)uVar2;
				}
				uVar9 = uVar9 - uVar2 * uVar1;
				*(float*)(*(int*)&(pTVar4->body).field_0x14 + 0xc) = fVar12 * (fVar10 / fVar11);
			}
			uVar1 = *(uint*)(iVar6 + 0x14);
			if ((int)uVar1 < 0) {
				fVar10 = (float)(uVar1 >> 1 | uVar1 & 1);
				fVar10 = fVar10 + fVar10;
			}
			else {
				fVar10 = (float)uVar1;
			}
			uVar1 = *(uint*)(iVar6 + 0x1c);
			if ((int)uVar1 < 0) {
				fVar11 = (float)(uVar1 >> 1 | uVar1 & 1);
				fVar11 = fVar11 + fVar11;
			}
			else {
				fVar11 = (float)uVar1;
			}
			if ((int)uVar9 < 0) {
				fVar12 = (float)(uVar9 >> 1 | uVar9 & 1);
				fVar12 = fVar12 + fVar12;
			}
			else {
				fVar12 = (float)uVar9;
			}
			*(float*)(*(int*)&(pTVar4->body).field_0x14 + 8) = fVar12 * (fVar10 / fVar11);
			param_1->cmdA = 0xe10000001;
			param_1->cmdB = 0;
			*(undefined4*)&param_1->cmdB = 0;
			*(undefined4*)((int)&param_1->cmdB + 4) = 0x6c010021;
			(ScratchpadAnimManager_004494ec.pVector_0x18)->x = *(float*)(*(int*)&(pTVar4->body).field_0x14 + 8);
			(ScratchpadAnimManager_004494ec.pVector_0x18)->y = *(float*)(*(int*)&(pTVar4->body).field_0x14 + 0xc);
			*(float*)&param_1[1].cmdA = (ScratchpadAnimManager_004494ec.pVector_0x18)->x;
			*(float*)((int)&param_1[1].cmdA + 4) = (ScratchpadAnimManager_004494ec.pVector_0x18)->y;
			*(float*)&param_1[1].cmdB = (ScratchpadAnimManager_004494ec.pVector_0x18)->z;
			*(float*)((int)&param_1[1].cmdB + 4) = (ScratchpadAnimManager_004494ec.pVector_0x18)->w;
			param_1 = param_1 + 2;
			g_TextureData_LAY_Int_00449404->field_0x4 = g_TextureData_LAY_Int_00449404->field_0x4 | 0x400;)
		}
	}
	else {
		IMPLEMENTATION_GUARD(
		pTVar4 = g_TextureData_LAY_Int_00449404->pTexB;
		if (pTVar4 != (TextureData_TEX*)0xfffffff0) {
			if (((uVar9 & 0x400) == 0) && (DAT_00449348 != 0)) {
				pfVar5 = *(float**)&(pTVar4->body).field_0x14;
				pfVar5[2] = pfVar5[2] + *pfVar5;
				iVar6 = *(int*)&(pTVar4->body).field_0x14;
				*(float*)(iVar6 + 0xc) = *(float*)(iVar6 + 0xc) + *(float*)(iVar6 + 4);
				iVar6 = *(int*)&(pTVar4->body).field_0x14;
				fVar10 = *(float*)(iVar6 + 8);
				if (1.0 <= fVar10) {
					*(float*)(iVar6 + 8) = fVar10 - 1.0;
				}
				iVar6 = *(int*)&(pTVar4->body).field_0x14;
				fVar10 = *(float*)(iVar6 + 0xc);
				if (1.0 <= fVar10) {
					*(float*)(iVar6 + 0xc) = fVar10 - 1.0;
				}
				iVar6 = *(int*)&(pTVar4->body).field_0x14;
				fVar10 = *(float*)(iVar6 + 8);
				if (fVar10 <= -1.0) {
					*(float*)(iVar6 + 8) = fVar10 + 1.0;
				}
				iVar6 = *(int*)&(pTVar4->body).field_0x14;
				fVar10 = *(float*)(iVar6 + 0xc);
				if (fVar10 <= -1.0) {
					*(float*)(iVar6 + 0xc) = fVar10 + 1.0;
				}
			}
			param_1->cmdA = 0xe10000001;
			param_1->cmdB = 0;
			*(undefined4*)&param_1->cmdB = 0;
			*(undefined4*)((int)&param_1->cmdB + 4) = 0x6c010021;
			(ScratchpadAnimManager_004494ec.pVector_0x18)->x = *(float*)(*(int*)&(pTVar4->body).field_0x14 + 8);
			(ScratchpadAnimManager_004494ec.pVector_0x18)->y = *(float*)(*(int*)&(pTVar4->body).field_0x14 + 0xc);
			*(float*)&param_1[1].cmdA = (ScratchpadAnimManager_004494ec.pVector_0x18)->x;
			*(float*)((int)&param_1[1].cmdA + 4) = (ScratchpadAnimManager_004494ec.pVector_0x18)->y;
			*(float*)&param_1[1].cmdB = (ScratchpadAnimManager_004494ec.pVector_0x18)->z;
			*(float*)((int)&param_1[1].cmdB + 4) = (ScratchpadAnimManager_004494ec.pVector_0x18)->w;
			param_1 = param_1 + 2;
			g_TextureData_LAY_Int_00449404->field_0x4 = g_TextureData_LAY_Int_00449404->field_0x4 | 0x400;
		})
	}
	return param_1;
}

void ed3DFlushMaterialManageGIFPacket(ed_dma_matrix* param_1)
{
	bool bVar1;
	edpkt_data* pRVar2;

	if ((param_1->flags_0x28 & 2U) != 0) {
		ed3DFXClearALLFXSendedTOVRAM();
		param_1->flags_0x28 = param_1->flags_0x28 & 0xfffffffd;
	}
	pRVar2 = g_GifRefPktCur;
	if (gFushListCounter != 0xe) {
		if ((gbFirstTex == 0) && (param_1->field_0x2c.ptrValue != (char*)0x0)) {
			if ((byte)*param_1->pFileData < 2) {
				IMPLEMENTATION_GUARD(
				g_GifRefPktCur->cmdA =
					((long)(*(int*)(*(int*)(param_1->field_0x2c + 8) + gVRAMBufferFlush * 8) + 0x20) & 0xfffffffU) << 0x20 |
					0x50000000;
				pRVar2->cmdB = 0;
				g_GifRefPktCur = g_GifRefPktCur + 1;)
			}
			else {
				IMPLEMENTATION_GUARD(
				g_GifRefPktCur = ed3DFlushMultiTexture(param_1, g_GifRefPktCur, 0, gVRAMBufferFlush);)
			}
		}
		else {
			gbFirstTex = 0;
		}
		bVar1 = gVRAMBufferFlush == 1;
		gVRAMBufferFlush = gVRAMBufferFlush + 1;
		if (bVar1) {
			gVRAMBufferFlush = 0;
		}
	}
	return;
}

void ed3DFlushMaterial(ed_dma_matrix* pRenderMeshData)
{
	ed_g2d_material* pTVar2;
	ulong* puVar3;
	edpkt_data* pRVar4;
	edpkt_data* pRVar5;
	edpkt_data* pRVar6;
	edpkt_data* pRVar7;
	ed_g2d_material* pTVar8;
	undefined4 in_zero_lo = 0x0;
	undefined4 in_zero_hi = 0x0;
	undefined4 in_zero_udw = 0x0;
	undefined4 in_register_0000000c = 0x0;
	edpkt_data* pRVar9;
	int iVar10;
	ed_g2d_material* pTVar1;

	pRVar7 = gpPKTDataRefMasPath3;
	pRVar6 = g_GifRefPktCur;
	pRVar4 = g_VifRefPktCur;
	*ScratchpadAnimManager_004494ec.field_0x3c = 0;
	pRVar5 = g_VifRefPktCur;
	gGlobalAlhaON = 0x80;
	pTVar2 = (ed_g2d_material*)pRenderMeshData->pFileData;
	pTVar2->pRenderFrame30 = 0;
	pRVar9 = gpPKTDataRefMasPath3;
	iVar10 = gFushListCounter;
	pTVar8 = pTVar2;
	if (pTVar2 == previous_flush3DMat) {
		pTVar8 = previous_flush3DMat;
		if (gVRAMBufferFlush == 0) {
			gVRAMBufferFlush = 1;
		}
		else {
			gVRAMBufferFlush = gVRAMBufferFlush + -1;
		}
	}
	previous_flush3DMat = pTVar8;
	if (((gFushListCounter != 0xe) && (pRenderMeshData->field_0x2c.ptrValue == (char*)0x0)) &&
		(gpPKTDataRefMasPath3 != (edpkt_data*)0x0)) {
		IMPLEMENTATION_GUARD(
		gpPKTDataRefMasPath3->cmdA =
			((long)(int)(gpPKTDataRefMasPath3 + 4) & 0xfffffffU) << 0x20 | 0x20000000;
		pRVar9->cmdB = 0;)
	}
	pRVar9 = pRVar5;
	if (gpLastMaterial != pRenderMeshData) {
		gpLastMaterial = pRenderMeshData;
		if (iVar10 == 0xe) {
			IMPLEMENTATION_GUARD(pRVar9 = FUN_002a97a0(pRVar5);)
		}
		else {
			if (pTVar2->commandBufferTextureSize == 2) {
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
				gpPKTDataRefMasPath3 = pRVar5 + 2;)
			}
			else {
				pRVar5->cmdA = 0x30000000;
				pRVar5->cmdB = 0x1300000000000000;
				pRVar5[1].cmdA =
					((ulong)(int)(pTVar2->pCommandBufferTexture + gVRAMBufferFlush * pTVar2->commandBufferTextureSize * (uint)pTVar2->count_0x0) & 0xfffffffU)
					<< 0x20 | (ulong)pTVar2->commandBufferTextureSize & 0xffffffffU | 0x30000000;
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
				gpPKTDataRefMasPath3 = pRVar5 + 3;
			}
		}
	}
	g_VifRefPktCur = pRVar9;
	if (pRenderMeshData->field_0x2c.ptrValue == (char*)0x0) {
		IMPLEMENTATION_GUARD(
		g_TextureData_LAY_Int_00449404 = (ed_g2d_layer*)0x0;
		g_VifRefPktCur = FUN_002a9e80(pRVar9);)
	}
	else {
		ed3DG2DMaterialGetLayerBitmap(pRenderMeshData, &PTR_DAT_0044940c, &g_TextureData_LAY_Int_00449404, 0);
		*(char**)ScratchpadAnimManager_004494ec.field_0x24 = pRenderMeshData->pFileData;
		pRVar5 = g_VifRefPktCur;
		g_VifRefPktCur[2].cmdA =
			(long)*(int*)ScratchpadAnimManager_004494ec.field_0x48 << 0x20 |
			(ulong) * (uint*)ScratchpadAnimManager_004494ec.field_0x44 << 0x13 |
			(ulong)(*(int*)ScratchpadAnimManager_004494ec.field_0x4c + 4) << 6 |
			((ulong)(int)((PTR_DAT_0044940c->field_0x6 - 1) * *(int*)ScratchpadAnimManager_004494ec.field_0x40) &
				0xffffffffU) << 2 | 0x20;
		pRVar9 = g_VifRefPktCur + 3;
		g_VifRefPktCur[2].cmdB = 0x14;
		iVar10 = 1;
		if ((g_TextureData_LAY_Int_00449404->field_0x4 & 0x80) != 0) {
			iVar10 = 3;
			pRVar9->cmdA = 0x50000;
			pRVar9 = g_VifRefPktCur + 5;
			g_VifRefPktCur[3].cmdB = 0x47;
			g_VifRefPktCur[4].cmdA = 0x800000002a;
			g_VifRefPktCur[4].cmdB = 0x42;
		}
		if ((long)iVar10 != 0) {
			puVar3 = &g_VifRefPktCur->cmdA;
			g_VifRefPktCur = pRVar9;
			*puVar3 = (ulong)(iVar10 + 2) | 0xe10000000;
			pRVar5->cmdB = 0;
			*(undefined4*)&pRVar5->cmdB = 0x40003dc;
			*(uint*)((ulong)&pRVar5->cmdB + 4) = (iVar10 + 1) * 0x10000 | 0x6c0003dc;
			pRVar5[1].cmdA = (long)iVar10 | 0x1000000000008000;
			pRVar5[1].cmdB = 0xe;
			*(undefined4*)&g_VifRefPktCur->cmdA = 0;
			*(undefined4*)((ulong)&g_VifRefPktCur->cmdA + 4) = 0x14000000;
			*(undefined4*)&g_VifRefPktCur->cmdB = 0;
			*(undefined4*)((ulong)&g_VifRefPktCur->cmdB + 4) = 0;
			g_VifRefPktCur = g_VifRefPktCur + 1;
		}
		g_VifRefPktCur = ed3DFlushMaterialAnimST(g_VifRefPktCur);
	}
	ed3DFlushMaterialNode(pRenderMeshData);
	pRVar5 = g_VifRefPktCur;
	if (*ScratchpadAnimManager_004494ec.field_0x3c == 0) {
		g_VifRefPktCur = pRVar4;
		g_GifRefPktCur = pRVar6;
		gpPKTDataRefMasPath3 = pRVar7;
		pRenderMeshData->field_0x2c.numValue = 0xdfdfdfdf;
	}
	else {
		g_VifRefPktCur->cmdA = 0xe10000001;
		pRVar5->cmdB = 0;
		pRVar4 = g_VifRefPktCur;
		g_VifRefPktCur = g_VifRefPktCur + 1;
		g_VifRefPktCur->cmdA = 0x2000000001000101;
		pRVar4[1].cmdB = 0xffffff00;
		g_VifRefPktCur = g_VifRefPktCur + 1;
		ed3DFlushMaterialManageGIFPacket(pRenderMeshData);
	}
	return;
}

void ed3DFlushList(void)
{
	float fVar1;
	int iVar2;
	edLIST* unaff_s1_lo;
	MeshTransformSpecial* pCVar5;
	edLIST* pCVar3;
	edLIST* pCVar4;

	gFlushListFlusaON = 0;
	gFushListCounter = 0;
	do {
		fVar1 = FLOAT_00448a04;
		if (0xe < gFushListCounter) {
			return;
		}
		/* Get the camera type, and then add the flip index as there is two per type. */
		pCVar4 = gPrim_List[gFushListCounter] + gCurRenderList;
		pCVar5 = pCVar4->pLoadedData;
		if ((edLIST*)pCVar5 != pCVar4) {
			switch (gFushListCounter) {
			default:
				/* 0x7 = Particle
					0x3 = Some level mesh
					0x2 = Character */
				((edLIST*)pCVar5)->pCameraPanHeader_0xc[2].totalCount =
					((edLIST*)pCVar5)->pCameraPanHeader_0xc[2].totalCount | 2;
				while (pCVar3 = (edLIST*)pCVar5, pCVar3 != pCVar4) {
					ed3DFlushMaterial((ed_dma_matrix*)pCVar3->pCameraPanHeader_0xc);
					unaff_s1_lo = pCVar3;
					pCVar5 = (MeshTransformSpecial*)pCVar3->pLoadedData;
				}
				break;
			case 1:
			case 5:
			case 6:
			case 7:
			case 9:
				IMPLEMENTATION_GUARD(if (gbFA2AObject != '\0') {
					g_VifRefPktCur = ed3DFlushFullAlphaInit(g_VifRefPktCur);
				}
				((edLIST*)pCVar5)->pCameraPanHeader_0xc[2].totalCount =
					((edLIST*)pCVar5)->pCameraPanHeader_0xc[2].totalCount | 2;
				for (; (edLIST*)pCVar5 != pCVar4; pCVar5 = pCVar5->pNext_0x4) {
					ed3DFlushMaterial((ed_dma_matrix*)pCVar5->pRenderInput);
					unaff_s1_lo = (edLIST*)pCVar5;
				}
				if (gbFA2AObject != '\0') {
					g_VifRefPktCur = ed3DFlushFullAlphaTerm(g_VifRefPktCur);
				})
				break;
			case 10:
				IMPLEMENTATION_GUARD(((edLIST*)pCVar5)->pCameraPanHeader_0xc[2].totalCount =
					((edLIST*)pCVar5)->pCameraPanHeader_0xc[2].totalCount | 2;
				for (; (edLIST*)pCVar5 != pCVar4;
					pCVar5 = (MeshTransformSpecial*)((edLIST*)pCVar5)->pLoadedData) {
					ed3DFlushMaterial((ed_dma_matrix*)((edLIST*)pCVar5)->pCameraPanHeader_0xc);
					unaff_s1_lo = (edLIST*)pCVar5;
				})
				g_VifRefPktCur = ed3DFlushResetOffset(g_VifRefPktCur, &gCurRectViewport);
				break;
			case 0xb:
				IMPLEMENTATION_GUARD(
				g_VifRefPktCur = (edpkt_data*)ed3DFlushBackFaceInit((ulong*)g_VifRefPktCur);
				g_VifRefPktCur = ed3DFlushResetOffset(g_VifRefPktCur, &gCurRectViewport);
				((edLIST*)pCVar5)->pCameraPanHeader_0xc[2].totalCount =
					((edLIST*)pCVar5)->pCameraPanHeader_0xc[2].totalCount & 0xfffffffd;
				for (; (edLIST*)pCVar5 != pCVar4;
					pCVar5 = (MeshTransformSpecial*)((edLIST*)pCVar5)->pLoadedData) {
					ed3DFlushMaterial((ed_dma_matrix*)((edLIST*)pCVar5)->pCameraPanHeader_0xc);
					unaff_s1_lo = (edLIST*)pCVar5;
				}
				g_VifRefPktCur = (edpkt_data*)ed3DFlushBackFaceTerm(g_VifRefPktCur);
				g_VifRefPktCur = ed3DFlushResetOffset(g_VifRefPktCur, &gCurRectViewport);)
			case 8:
				IMPLEMENTATION_GUARD(
				if (gbFA2AObject != '\0') {
					g_VifRefPktCur = ed3DFlushFullAlphaInit(g_VifRefPktCur);
				}
				((edLIST*)pCVar5)->pCameraPanHeader_0xc[2].totalCount =
					((edLIST*)pCVar5)->pCameraPanHeader_0xc[2].totalCount | 2;
				for (; (edLIST*)pCVar5 != pCVar4;
					pCVar5 = (MeshTransformSpecial*)((edLIST*)pCVar5)->pLoadedData) {
					ed3DFlushMaterial((ed_dma_matrix*)((edLIST*)pCVar5)->pCameraPanHeader_0xc);
					unaff_s1_lo = (edLIST*)pCVar5;
				}
				if (gbFA2AObject != '\0') {
					g_VifRefPktCur = ed3DFlushFullAlphaTerm(g_VifRefPktCur);
				})
				break;
			case 0xd:
				IMPLEMENTATION_GUARD(
				FLOAT_00448a04 = 0.0;
				g_VifRefPktCur = (edpkt_data*)FUN_002b4e60((ulong*)g_VifRefPktCur);
				BYTE_004489e4 = 0;
				gFushListCounter = 0xe;
				((edLIST*)pCVar5)->pCameraPanHeader_0xc[2].totalCount =
					((edLIST*)pCVar5)->pCameraPanHeader_0xc[2].totalCount | 2;
				for (; (edLIST*)pCVar5 != pCVar4;
					pCVar5 = (MeshTransformSpecial*)((edLIST*)pCVar5)->pLoadedData) {
					ed3DFlushMaterial((ed_dma_matrix*)((edLIST*)pCVar5)->pCameraPanHeader_0xc);
					unaff_s1_lo = (edLIST*)pCVar5;
				}
				gFushListCounter = 0xd;
				FLOAT_00448a04 = fVar1;)
				break;
			case 0xe:
				IMPLEMENTATION_GUARD(
				g_VifRefPktCur = (edpkt_data*)FUN_002b4e60((ulong*)g_VifRefPktCur);
				BYTE_004489e4 = 1;
				((edLIST*)pCVar5)->pCameraPanHeader_0xc[2].totalCount =
					((edLIST*)pCVar5)->pCameraPanHeader_0xc[2].totalCount | 2;
				for (; (edLIST*)pCVar5 != pCVar4;
					pCVar5 = (MeshTransformSpecial*)((edLIST*)pCVar5)->pLoadedData) {
					ed3DFlushMaterial((ed_dma_matrix*)((edLIST*)pCVar5)->pCameraPanHeader_0xc);
					unaff_s1_lo = (edLIST*)pCVar5;
				}
				g_VifRefPktCur = (edpkt_data*)FUN_002b4d50();)
			}
			iVar2 = gCurRenderList;
			if ((edLIST*)gPrim_List_FlushTex[gCurRenderList].pLoadedData ==
				gPrim_List_FlushTex + gCurRenderList) {
				pCVar4 = gPrim_List[gFushListCounter] + gCurRenderList;
				gPrim_List_FlushTex[gCurRenderList].pLoadedData = pCVar4->pLoadedData;
				gPrim_List_FlushTex[iVar2].count_0x14 = pCVar4->count_0x14;
			}
			else {
				IMPLEMENTATION_GUARD(
				pCVar4 = gPrim_List[gFushListCounter] + gCurRenderList;
				gPrim_List_FlushTex[gCurRenderList].count_0x14 =
					gPrim_List_FlushTex[gCurRenderList].count_0x14 + pCVar4->count_0x14;
				gPrim_List_FlushTex_Last->pNext_0x4 = pCVar4->pLoadedData;)
			}
			pCVar4 = gPrim_List[gFushListCounter] + gCurRenderList;
			gPrim_List_FlushTex_Last = (MeshTransformSpecial*)unaff_s1_lo;
			pCVar4->count_0x14 = 0;
			pCVar4->pLoadedData = (MeshTransformSpecial*)pCVar4;
		}
		gFushListCounter = gFushListCounter + 1;
	} while (true);
}

edpkt_data* ed3DFlushFullAlphaInit(edpkt_data* pRenderCommand)
{
	edSurface* pFVar1;

	pFVar1 = edVideoGetDrawSurface();
	if (pFVar1 != (edSurface*)0x0) {
		//DAT_00449450 = 0x40;
		//DAT_00449448 = 0x7000;
		//DAT_00449468 = 0xb0;
		//DAT_0044944c = 0x7000;
		//DAT_00449474 = 0x1600;
		//DAT_00449454 = 0x800;
		gZBUF_BASE = gCurViewportUsed->pZBuffer->frameBasePtr;
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
			gZBUF_BASE,	// ZBP
			SCE_GS_PSMZ24,					// PSM
			SCE_GS_TRUE						// ZMASK
		);
		pRenderCommand[3].cmdB = SCE_GS_ZBUF_1;
		pRenderCommand = pRenderCommand + 4;
	}
	return pRenderCommand;
}

edpkt_data* ed3DFlushFullAlphaTerm(edpkt_data* pRenderCommand)
{
	edSurface* pFVar1;

	pFVar1 = edVideoGetDrawSurface();
	if (pFVar1 != (edSurface*)0x0) {
		//DAT_00449448 = 0x7000;
		//DAT_0044944c = 0x7000;
		//DAT_00449450 = pFVar1->frameBasePtr;
		//DAT_00449474 = 0x800;
		//DAT_00449468 = 0x40;
		//DAT_00449454 = DAT_00449450 << 5;
		gZBUF_BASE = gCurViewportUsed->pZBuffer->frameBasePtr;
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
			gZBUF_BASE,	// ZBP
			SCE_GS_PSMZ24,					// PSM
			SCE_GS_FALSE					// ZMASK
		);
		pRenderCommand[3].cmdB = SCE_GS_ZBUF_1;
		pRenderCommand = pRenderCommand + 4;
	}
	return pRenderCommand;
}

void ed3DFlushShadowList(void)
{
	edNODE_MANAGER* pMVar1;
	bool bVar2;
	char** ppcVar3;
	int iVar4;
	ed_dma_matrix* pRVar5;
	edpkt_data* pRVar6;
	MeshTransformSpecial* pMVar7;
	char* pcVar8;
	edLIST* pCVar9;
	ed_dma_matrix* pRenderFrame;

	bVar2 = false;
	g_VifRefPktCur = ed3DFlushFullAlphaInit(g_VifRefPktCur);
	gFlushListFlusaON = 0;
	gShadowFlushMode = 1;
	pRenderFrame = (ed_dma_matrix*)(gPrim_List_Shadow[gCurRenderList].pLoadedData)->pNext_0x4->pRenderInput;

	// Is the next equal to the render frame.
	if ((pRenderFrame->internalMeshTransformSpecial).pNext_0x4 != &pRenderFrame->internalMeshTransformSpecial) {
		IMPLEMENTATION_GUARD(
			memcpy(ScratchpadAnimManager_004494ec.pScreenSpaceMatrix, &Matrix_0048cbb0, sizeof(edF32MATRIX4));
		memcpy(ScratchpadAnimManager_004494ec.pTextureSpaceMatrix, &Matrix_0048cbf0, sizeof(edF32MATRIX4));
		memcpy(ScratchpadAnimManager_004494ec.pWorldSpaceMatrix, &Matrix_0048cc30, sizeof(edF32MATRIX4));
		memcpy(g_ScratchpadAddr_00448a58, &Matrix_0048ccb0, sizeof(edF32MATRIX4));
		memcpy(&Vector_0048c2d0, &Vector_0048ccf0, sizeof(Vector));
		memcpy(&Vector_0048c2e0, &Vector_0048cd00, sizeof(Vector));
		memcpy(&Vector_0048c2f0, &Vector_0048cd10, sizeof(Vector));
		memcpy(&Vector_0048c300, &Vector_0048cd20, sizeof(Vector));
		memcpy(&Vector_0048c310, &Vector_0048cd30, sizeof(Vector));
		memcpy(&Vector_0048c320, &Vector_0048cd40, sizeof(Vector));
		pRVar6 = g_VifRefPktCur;
		g_VifRefPktCur->cmdA = 0x30000000;
		pRVar6->cmdB = 0x1300000000000000;
		g_VifRefPktCur = g_VifRefPktCur + 1;
		g_VifRefPktCur = (edpkt_data*)ed3DJitterShadow((undefined8*)g_VifRefPktCur);
		g_VifRefPktCur = ed3DDMAGenerateGlobalPacket(g_VifRefPktCur);
		g_VifRefPktCur = ed3DAddViewportContextPacket((CameraObj_28*)PTR_DAT_0044956c, g_VifRefPktCur);
		pRVar6 = (edpkt_data*)ed3DShadowFlushResetOffset((undefined8*)g_VifRefPktCur, (short*)&gCurRectViewport);
		g_VifRefPktCur = pRVar6;
		pRVar6->cmdA = 0xe10000002;
		pRVar6->cmdB = 0;
		*(undefined4*)&pRVar6->cmdB = 0;
		*(undefined4*)((ulong)&g_VifRefPktCur->cmdB + 4) = 0x50000002;
		pRVar6 = g_VifRefPktCur;
		g_VifRefPktCur = g_VifRefPktCur + 1;
		g_VifRefPktCur->cmdA = 0x1000800000008001;
		pRVar6[1].cmdB = 0xe;
		pRVar6 = g_VifRefPktCur;
		g_VifRefPktCur = g_VifRefPktCur + 1;
		g_VifRefPktCur->cmdA =
			(ulong)gCurScene->pViewport->pZBuffer->frameBasePtr | 0x130000000;
		pRVar6[1].cmdB = 0x4e;
		g_VifRefPktCur = g_VifRefPktCur + 1;
		ed3DFlushMaterialNode(pRenderFrame);
		pRVar6 = g_VifRefPktCur;
		g_VifRefPktCur->cmdA = 0x30000000;
		pRVar6->cmdB = 0x1300000000000000;
		iVar4 = gCurRenderList;
		g_VifRefPktCur = g_VifRefPktCur + 1;
		gPrim_List_FlushTex[gCurRenderList].count_0x14 =
			gPrim_List_FlushTex[gCurRenderList].count_0x14 + 1;
		bVar2 = true;
		if (gPrim_List_FlushTex_Last == (MeshTransformSpecial*)0x0) {
			gPrim_List_FlushTex_Last = (gPrim_List_Shadow[iVar4].pLoadedData)->pNext_0x4;
		}
		else {
			gPrim_List_FlushTex_Last->pNext_0x4 = (gPrim_List_Shadow[iVar4].pLoadedData)->pNext_0x4;
			gPrim_List_FlushTex_Last = gPrim_List_FlushTex_Last->pNext_0x4;
		}
		(gPrim_List_Shadow[gCurRenderList].pLoadedData)->pNext_0x4 =
			(MeshTransformSpecial*)(gPrim_List_Shadow + gCurRenderList);
		pRVar5 = DmaMaterialBufferCurrent;
		pMVar7 = &(DmaMaterialBufferCurrent)->internalMeshTransformSpecial;
		ppcVar3 = &(DmaMaterialBufferCurrent)->pFileData;
		DmaMaterialBufferCurrent = DmaMaterialBufferCurrent + 1;
		*ppcVar3 = &DAT_0048c3b0;
		pRVar5->field_0x4 = (MeshTransformSpecial*)0x0;
		pRVar5->flags_0x28 = pRVar5->flags_0x28 & 0xfffffffe;
		(pRVar5->internalMeshTransformSpecial).pPrev_0x8 = pMVar7;
		(pRVar5->internalMeshTransformSpecial).pNext_0x4 = pMVar7;
		(pRVar5->internalMeshTransformSpecial).pRenderInput = (DisplayListInternal*)PTR_MeshTransformParentHeader_004494b8;
		pRVar5->field_0x18 = 0;
		pRVar5->index_0x1c = 0;
		*(undefined4*)&pRVar5->internalMeshTransformSpecial = 0;
		pcVar8 = (char*)ed3DGetG2DBitmap((char**)pRVar5, 0);
		pRenderFrame->field_0x2c = pcVar8;
		iVar4 = gCurRenderList;
		pCVar9 = gPrim_List_Shadow + gCurRenderList;
		pMVar1 = gPrim_List_Shadow[gCurRenderList].pCameraPanHeader_0xc;
		pMVar7 = pMVar1->pNextFree + pMVar1->usedCount;
		pMVar7->pRenderInput = (DisplayListInternal*)pRVar5;
		pMVar7->specUnion = 1;
		pMVar1->usedCount = pMVar1->usedCount + 1;
		gPrim_List_Shadow[iVar4].count_0x14 = gPrim_List_Shadow[iVar4].count_0x14 + 1;
		pMVar7->pNext_0x4 = (MeshTransformSpecial*)pCVar9;
		(gPrim_List_Shadow[iVar4].pLoadedData)->pNext_0x4 = pMVar7;)
	}
	gShadowFlushMode = 0;
	*ScratchpadAnimManager_004494ec.gShadowRenderMask = 0;
	if (bVar2) {
		pprevious_shadow_dma_matrix = 0;
	}
	g_VifRefPktCur = ed3DFlushFullAlphaTerm(g_VifRefPktCur);
	return;
}

float gAnimSTMaxDist = 10.0f;

typedef enum EVectorMode_A {
	VM_0 = 0,
	VM_1 = 1,
	VM_2 = 2,
	VM_3 = 3,
	VM_4 = 4
} EVectorMode_A;


ed_g2d_material* ed3DG2DGetG2DMaterialFromIndex(char* pMBNK, int index)
{
	int* piVar1;
	int* piVar2;

	piVar1 = (int*)LOAD_SECTION((int)*(int**)(pMBNK + index * 0x10 + 8));
	piVar2 = (int*)0x0;
	if ((piVar1 != (int*)0x0) && (piVar2 = piVar1 + 4, *piVar1 != 0x2e54414d)) {
		piVar2 = (int*)((char*)LOAD_SECTION(piVar1[2]) + 0x10);
	}
	return (ed_g2d_material*)piVar2;
}


EVectorMode_A ed3DTestBoundingSphere(Vector* param_1)
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

bool AllocateRenderData_30_00299650(ed_3d_strip* param_1)
{
	byte bVar1;
	edF32MATRIX4* pMVar2;
	uint uVar3;
	Vector* pVVar4;
	int iVar5;
	EVectorMode_A EVar6;
	bool bVar7;
	int iVar8;
	TextureData_MYSTERY* pfVar8;
	uint uVar9;
	ulong uVar10;
	ed_dma_matrix* pRVar11;
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
		pRVar11 = (ed_dma_matrix*)LOAD_SECTION(param_1->pRenderFrame30);
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
				pRVar11 = (ed_dma_matrix*)((ulong)&pRVar11->pFileData + 1);
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

void ed3DLinkClusterStripToViewport(ed_3d_strip* param_1, char* pMBNK)
{
	edF32MATRIX4* pMVar1;
	int iVar3;
	bool bVar4;
	ed_g2d_material** ppTVar5;
	ed_dma_matrix* pRVar6;
	Vector* pVVar7;
	EVectorMode_A EVar8;
	ed_g2d_material* piVar7;
	uint* puVar9;
	long lVar10;
	undefined2* puVar11;
	MeshTransformSpecial* pMVar12;
	int* piVar13;
	edNODE_MANAGER* pDVar14;
	ed_dma_matrix* pRVar15;
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
	ed_g2d_layer* iVar1;
	TextureData_TEX* iVar2;

	if (param_1->field_0x4 == -1) {
		piVar7 = (ed_g2d_material*)0x0;
	}
	else {
		piVar7 = ed3DG2DGetG2DMaterialFromIndex(pMBNK, (int)param_1->field_0x4);
		in_vf9w = extraout_vf9w;
		if ((piVar7 != (ed_g2d_material*)0x0) && ((piVar7->field_0x2 & 1) != 0)) {
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
		EVar8 = ed3DTestBoundingSphere(&param_1->vector_0x10);
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
	if ((piVar7 != (ed_g2d_material*)0x0) && (piVar7->count_0x0 != 0)) {
		iVar1 = (ed_g2d_layer*)LOAD_SECTION(*(int*)(piVar7 + 1));
		uVar18 = iVar1->body.flags_0x0;
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
					puVar9 = (uint*)ed3DG2DAnimTexGet(&iVar2->body);
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
					fVar19 = gAnimSTMaxDist;
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
			if ((iVar2->body).palette == 0) {
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
	piVar7 = (ed_g2d_material*)PTR_TextureData_MAT_004492f0;
LAB_00297870:
	pRVar6 = DmaMaterialBufferCurrent;
	if ((((uVar18 & 0xfc) == 0) || ((uVar18 & 0x80000000) != 0)) || ((uVar18 & 0x4000) != 0)) {
		pRVar15 = (ed_dma_matrix*)LOAD_SECTION(piVar7->pRenderFrame30);
		if (piVar7->pRenderFrame30 == 0x0) {
			pMVar12 = &(DmaMaterialBufferCurrent)->internalMeshTransformSpecial;
			ppTVar5 = (ed_g2d_material**)&(DmaMaterialBufferCurrent)->pFileData;
			DmaMaterialBufferCurrent = DmaMaterialBufferCurrent + 1;
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
			edLIST* pCameraPanMasterHeader = &gPrim_List[param_1->cameraPanIndex][gCurRenderList];
			edNODE_MANAGER* pHeader = pCameraPanMasterHeader->pCameraPanHeader_0xc;
			pMVar12 = pHeader->pNextFree + pHeader->usedCount;
			pMVar12->pRenderInput = (DisplayListInternal*)pRVar6;
			pMVar12->specUnion.field_0x0[0] = 1;
			pHeader->usedCount = pHeader->usedCount + 1;
			pCameraPanMasterHeader->count_0x14 = pCameraPanMasterHeader->count_0x14 + 1;
			pMVar12->pNext_0x4 = pCameraPanMasterHeader->pLoadedData;
			RENDER_LOG("ed3DLinkClusterStripToViewport A %p\n", pCameraPanMasterHeader->pLoadedData);
			pCameraPanMasterHeader->pLoadedData = pMVar12;
			pRVar15 = pRVar6;
		}
		pDVar14 = (edNODE_MANAGER*)(pRVar15->internalMeshTransformSpecial).pRenderInput;
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
		pRVar15 = DmaMaterialBufferCurrent;
		pMVar12 = &(DmaMaterialBufferCurrent)->internalMeshTransformSpecial;
		ppTVar5 = (ed_g2d_material**)&(DmaMaterialBufferCurrent)->pFileData;
		DmaMaterialBufferCurrent = DmaMaterialBufferCurrent + 1;
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
		pDVar14 = (edNODE_MANAGER*)(pRVar15->internalMeshTransformSpecial).pRenderInput;
	}
	pMVar12 = pDVar14->pNextFree + pDVar14->usedCount;
	pMVar12->pRenderInput = (DisplayListInternal*)param_1;
	pMVar12->specUnion.field_0x0[0] = 3;
	pMVar12->specUnion.field_0x0[1] = (ushort)param_1->flags_0x0;
	pDVar14->usedCount = pDVar14->usedCount + 1;
	pRVar15->index_0x1c = pRVar15->index_0x1c + 1;
	pMVar12->pNext_0x4 = (pRVar15->internalMeshTransformSpecial).pNext_0x4;
	RENDER_LOG("ed3DLinkClusterStripToViewport B %p\n", (pRVar15->internalMeshTransformSpecial).pNext_0x4);
	pMVar12->pPrev_0x8 = (MeshTransformSpecial*)param_1->pRenderFrame30;
	(pRVar15->internalMeshTransformSpecial).pNext_0x4 = pMVar12;
	return;
}

void ed3DRenderCluster(ed_3d_octree* param_1)
{
	bool bVar1;
	ushort uVar2;
	char* pcVar4;
	edF32MATRIX4* pMVar5;
	bool bVar6;
	Vector* pVVar7;
	int* piVar8;
	uint uVar9;
	int iVar10;
	char cVar11;
	int* piVar12;
	ed_3d_strip* pcVar13;
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
	Vector local_230;
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
		pcVar13 = (ed_3d_strip*)LOAD_SECTION(pcVar3->field_0x48);
		(ScratchpadAnimManager_004494ec.pRenderAnimationStatic_0x60)->field_0x8 = (uint)param_1->field_0x28;
		(ScratchpadAnimManager_004494ec.pRenderAnimationStatic_0x60)->pMeshTransformMatrix = g_ScratchpadAddr_00448a58;
		for (; uVar9 != 0; uVar9 = uVar9 - 1) {
			ed3DLinkClusterStripToViewport(pcVar13, pcVar4 + 0x10);
			pcVar13 = (ed_3d_strip*)LOAD_SECTION(pcVar13->pNext);
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
			//ed3DLinkClusterSpriteToViewport(iVar10, uVar2, pcVar4 + 0x10);
			iVar10 = *(int*)(iVar10 + 0xc);
		}
		bVar6 = true;
	}
	uVar9 = (uint) * (ushort*)((ulong)&pcVar3->field_0x28 + 2);
	if (uVar9 != 0) {
		IMPLEMENTATION_GUARD();
		puVar17 = pcVar3[1].field_0x10 + 2;
		while (bVar1 = uVar9 != 0, uVar9 = uVar9 - 1, bVar1) {
			//ed3DRenderClusterHierarchy((long)(*(int*)(puVar17 + 4) + 0x10));
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
		piVar8 = (int*)edChunckGetFirst((char*)((int)puVar18 + iVar10 + 0x40), param_1->pCDQU_End);
		while (true) {
			uVar14 = (uint)uVar15;
			if (piVar8 == (int*)0x0) break;
			local_1d4[uVar14 * 0x10 + 1] = param_1->field_0x30;
			if ((*piVar8 == 0x55514443) || (*piVar8 == 0x434f4443)) {
				if (param_1->field_0x28 == 2) {
					if (local_1d4[uVar14 * 0x10 + 1] == 1.0) {
						IMPLEMENTATION_GUARD(ed3DCalculSubBoxQuadTree((float*)(auStack544 + uVar14 * 0x40 + 0x20), (float*)param_1, uVar14));
						fVar21 = extraout_vf9w;
					}
					else {
						IMPLEMENTATION_GUARD(ed3DCalculSubBoxOctree((float*)(auStack544 + uVar14 * 0x40 + 0x20), (float*)param_1, uVar15));
						fVar21 = extraout_vf9w_00;
					}
					local_230.x = *(float*)(auStack544 + uVar14 * 0x40 + 0x30);
					local_230.y = local_1f0[uVar14 * 0x10 + 1];
					local_230.z = local_1f0[uVar14 * 0x10 + 2];
					local_230.w = param_1->field_0x2c * 0.5;
					(ScratchpadAnimManager_004494ec.pRenderAnimationStatic_0x60)->pMeshTransformMatrix = g_ScratchpadAddr_00448a58
						;
					pVVar7 = ScratchpadAnimManager_004494ec.vector_0x64;
					(ScratchpadAnimManager_004494ec.vector_0x64)->x = local_230.x;
					pVVar7->y = local_230.y;
					pVVar7->z = local_230.z;
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
					iVar10 = ed3DTestBoundingSphere(&local_230);
					if (iVar10 == 4) {
						local_1d8[uVar14 * 0x20] = 4;
					}
					else {
						(&local_1e0)[uVar14 * 0x10] = piVar8;
						(&local_1dc)[uVar14 * 0x10] = (int)piVar8 + piVar8[2];
						local_1d8[uVar14 * 0x20] = (short)iVar10;
						local_1d4[uVar14 * 0x10] = local_230.w;
						local_240.x = local_230.x;
						local_240.y = local_230.y;
						local_240.z = local_230.z;
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
						IMPLEMENTATION_GUARD(ed3DCalculSubBoxQuadTree((float*)(auStack544 + uVar14 * 0x40 + 0x20), (float*)param_1, uVar14));
					}
					else {
						IMPLEMENTATION_GUARD(ed3DCalculSubBoxOctree((float*)(auStack544 + uVar14 * 0x40 + 0x20), (float*)param_1, uVar15));
					}
					local_260.x = *(float*)(auStack544 + uVar14 * 0x40 + 0x30);
					local_260.y = local_1f0[uVar14 * 0x10 + 1];
					local_260.z = local_1f0[uVar14 * 0x10 + 2];
					(&local_1e0)[uVar14 * 0x10] = piVar8;
					(&local_1dc)[uVar14 * 0x10] = (int)piVar8 + piVar8[2];
					local_1d8[uVar14 * 0x20] = 1;
					local_260.w = 1.0;
					local_230.x = local_260.x;
					local_230.y = local_260.y;
					local_230.z = local_260.z;
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
			piVar8 = edChunckGetNext((char*)piVar8, piVar12);
		}
		if (uVar15 != 0) {
			IMPLEMENTATION_GUARD(ed3DRenderClusterSortOctreeSons((ed_3d_octree*)auStack544, (int)(auStack544 + 0x20), uVar14));
			lVar16 = 0;
			while (true) {
				if ((long)uVar15 <= lVar16) break;
				if ((*(ed_3d_octree**)(auStack544 + (int)lVar16 * 4))->field_0x28 != 4) {
					ed3DRenderCluster(*(ed_3d_octree**)(auStack544 + (int)lVar16 * 4));
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
		uVar9 = 0; IMPLEMENTATION_GUARD(ed3DClusterGetEdgeWithSon(uVar9, cVar11));
		if ((long)(int)uVar9 != 0) {
			IMPLEMENTATION_GUARD(ed3DRenderEdge((short*)puVar18, (long)(int)uVar9, (uint)param_1->field_0x28, uVar20));
		}
	}
	return;
}

bool ed3DSceneRenderCluster(ed_g3d_manager* pMeshInfo)
{
	bool bVar1;
	Vector local_a0;
	Vector local_90;
	ed_3d_octree local_80;
	ed_3d_octree local_40;
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
		local_40.pCDQU = edChunckGetFirst((char*)(pcVar1 + 1), (char*)0x0);
		local_40.pCDQU_End = local_40.pCDQU + *(int*)(local_40.pCDQU + 8);
		ed3DRenderCluster(&local_40);
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
			local_80.pCDQU = edChunckGetFirst((char*)(pcVar1 + 1), (char*)0x0);
			local_80.pCDQU_End = local_80.pCDQU + *(int*)(local_80.pCDQU + 8);
			ed3DRenderCluster(&local_80);
			bVar1 = true;
		}
		else {
			bVar1 = false;
		}
	}
	return bVar1;
}

edF32MATRIX4 Matrix_00424f50 = {
	1.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 1.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 0.0f
};

float ed3DMatrixGetBigerScale(edF32MATRIX4* param_1)
{
	float fVar1;
	float fVar2;
	float fVar3;
	int iVar4;
	Vector* pVVar5;
	edF32MATRIX4* pMVar6;
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
		pMVar6 = (edF32MATRIX4*)&pMVar6->ba;
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

void ed3DSetSceneRender(MeshTransformData* pMeshTransformData, byte param_2)
{
	(pMeshTransformData->base).field_0x88 = param_2;
	return;
}

MeshTransformObjData* ed3DChooseGoodLOD(MeshTransformData* pMeshTransformData)
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
	int field_0x2c; //struct ed_3d_strip*
});

uint ed3DTestBoundingSphereObject(Vector* param_1)
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


void ed3DRenderObject(MeshHeader* pMeshHeader, undefined* pTextureInfo)
{
	bool bVar1;
	edF32MATRIX4* pMVar2;
	Vector* pVVar3;
	uint uVar4;
	int iVar5;
	Vector* pVVar6;
	int iVar7;
	int meshCount;
	ed_3d_strip* pRenderInput;
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
	pRenderInput = (ed_3d_strip*)LOAD_SECTION(pMeshOBJ->field_0x2c);
	if (pRenderInput == (ed_3d_strip*)0x0) {
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
	uVar4 = ed3DTestBoundingSphereObject(pVVar6);
	if (uVar4 != 4) {
		if (uVar4 == 1) {
			(ScratchpadAnimManager_004494ec.pRenderAnimationStatic_0x60)->field_0x8 = 1;
		}
		else {
			IMPLEMENTATION_GUARD(
			iVar5 = ed3DTestBoundingSphereObjectNoZFar(pVVar6);
			if (iVar5 != 1) {
				(ScratchpadAnimManager_004494ec.pRenderAnimationStatic_0x60)->field_0x8 = iVar7;
			})
		}
		if (*ScratchpadAnimManager_004494ec.gShadowRenderMask == 0) {
			IMPLEMENTATION_GUARD(
			while (bVar1 = meshCount != 0, meshCount = meshCount + -1, bVar1) {
				_ed3DLinkStripToViewport(pRenderInput, pTextureInfo);
				pRenderInput = pRenderInput->pNext;
			})
		}
		else {
			IMPLEMENTATION_GUARD(
			while (bVar1 = meshCount != 0, meshCount = meshCount + -1, bVar1) {
				_ed3DLinkStripShadowToViewport(pRenderInput, pTextureInfo);
				pRenderInput = pRenderInput->pNext;
			})
		}
	}
	return;
}

void ed3DRenderSonHierarchy(MeshTransformData* pMeshTransformData)
{
	short sVar1;
	MeshHeader* pMeshHeader;
	MeshTransformObjData* pMVar2;
	edF32MATRIX4* pMVar3;
	float fVar4;
	edF32MATRIX4 MStack64;

	if ((((pMeshTransformData->base).flags_0x9e & 0x41) == 0) && ((pMeshTransformData->base).count_0x9c != 0)) {
		pMVar2 = ed3DChooseGoodLOD(pMeshTransformData);
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
			fVar4 = ed3DMatrixGetBigerScale(pMVar3);
			(ScratchpadAnimManager_004494ec.pRenderAnimationStatic_0x60)->field_0x14 = fVar4;
			(ScratchpadAnimManager_004494ec.pRenderAnimationStatic_0x60)->field_0x18 = 0;
			(ScratchpadAnimManager_004494ec.pRenderAnimationStatic_0x60)->field_0x8 = 1;
			(pMeshTransformData->base).field_0xa4 = (edF32MATRIX4*)0x0;
			pMeshHeader = (MeshHeader*)pMVar2->pObj;
			if (((pMeshHeader != (MeshHeader*)0x0) && (sVar1 = pMVar2->field_0x4, sVar1 != 2)) && (sVar1 != 1)) {
				if (sVar1 == 3) {
					IMPLEMENTATION_GUARD(ed3DRenderSprite(pMeshHeader, (pMeshTransformData->base).pTextureInfo + 0x10));
				}
				else {
					if (sVar1 == 0) {
						ed3DRenderObject(pMeshHeader, (pMeshTransformData->base).pTextureInfo + 0x10);
					}
				}
			}
		}
		ed3DSetSceneRender(pMeshTransformData, 1);
	}
	return;
}

void ed3DRenderSonHierarchyForShadow(MeshTransformData* pMeshTransformData)
{
	char cVar1;
	ushort uVar2;
	short sVar3;
	MeshTransformObjData* pMVar4;
	MeshHeader** ppMVar5;
	edF32MATRIX4* pMVar6;
	float fVar7;
	edF32MATRIX4 MStack64;

	uVar2 = (pMeshTransformData->base).flags_0x9e;
	if (((((uVar2 & 0x200) != 0) &&
		((*ScratchpadAnimManager_004494ec.gShadowRenderMask & (uint)(pMeshTransformData->base).bRenderShadow) != 0)) &&
		((uVar2 & 0x41) == 0)) && ((pMeshTransformData->base).count_0x9c != 0)) {
		pMVar4 = ed3DChooseGoodLOD(pMeshTransformData);
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
			fVar7 = ed3DMatrixGetBigerScale(pMVar6);
			(ScratchpadAnimManager_004494ec.pRenderAnimationStatic_0x60)->field_0x14 = fVar7;
			(ScratchpadAnimManager_004494ec.pRenderAnimationStatic_0x60)->field_0x18 = 0;
			(pMeshTransformData->base).field_0xa4 = (edF32MATRIX4*)0x0;
			sVar3 = pMVar4->field_0x4;
			if (sVar3 == 3) {
				IMPLEMENTATION_GUARD(
				MeshHeader::ReadRenderTransform_002b0e80
				((MeshHeader*)pMVar4->pObj, (pMeshTransformData->base).pTextureInfo + 0x10);)
			}
			else {
				if (((sVar3 != 2) && (sVar3 != 1)) && (sVar3 == 0)) {
					if (((pMeshTransformData->base).flags_0x9e & 0x100) == 0) {
						ed3DRenderObject
						((MeshHeader*)pMVar4->pObj, (pMeshTransformData->base).pTextureInfo + 0x10);
					}
					else {
						IMPLEMENTATION_GUARD(
						cVar1 = (pMeshTransformData->base).GlobalAlhaON;
						(pMeshTransformData->base).GlobalAlhaON = -1;
						ppMVar5 = (MeshHeader**)
							Func_00295a50(pMeshTransformData, (long)(int)((pMeshTransformData->base).count_0x9c - 1));
						if (ppMVar5 != (MeshHeader**)0x0) {
							ed3DRenderObject(*ppMVar5, (pMeshTransformData->base).pTextureInfo + 0x10);
						}
						(pMeshTransformData->base).field_0xaf = cVar1;)
					}
				}
			}
		}
	}
	return;
}

const char* gSceneNames[] = {
	"Scene::__scene_handleA",
	"Scene::__scene_handleB",
	"Scene::_scene_handleA_Shadow_1",
	"Scene::_scene_handleA_Shadow_2",
	"Scene::_scene_handleA_Shadow_3",
	"Scene::_scene_handleA_Shadow_4",
	"Scene::_scene_handleA_Shadow_5",
	"Scene::_scene_handleA_Shadow_6",
	"Scene::_scene_handleA_Shadow_7",
	"Scene::_scene_handleA_Shadow_8",
	"Scene::_scene_handleA_Shadow_9",
	"Scene::_scene_handleA_Shadow_10",
	"Frontend::_scene_handle"
};

uint ed3DSceneRenderOne(ed_3D_Scene* pInStaticMeshMaster, ed_3D_Scene* pStaticMeshMaster)
{
	ed_g3d_manager* pMeshInfo;
	float fVar1;
	ed_viewport* pCVar2;
	int iVar3;
	bool bVar4;
	uint uVar5;
	uint uVar6;
	//ProfileObject* pPVar7;
	MeshTransformParent* pCVar10;
	MeshTransformParent* pCVar9;
	RenderTaskData renderTaskData;
	MeshTransformParent* pCVar8;

	RENDER_LOG("ed3DSceneRenderOne A %p (%s) (shadow: %d)\n", pInStaticMeshMaster, gSceneNames[GetStaticMeshMasterIndex(pInStaticMeshMaster)], pInStaticMeshMaster->bShadowScene);
	RENDER_LOG("ed3DSceneRenderOne B %p (%s) (shadow: %d)\n", pStaticMeshMaster, gSceneNames[GetStaticMeshMasterIndex(pStaticMeshMaster)], pStaticMeshMaster->bShadowScene);

	iVar3 = gIDProfileFlush;
	if ((pInStaticMeshMaster->bShadowScene == 1) && ((pStaticMeshMaster->flags_0x4 & 0x10) != 0)) {
		RENDER_LOG("ed3DSceneRenderOne SKIP\n");
		uVar6 = 0;
	}
	else {
		RENDER_LOG("ed3DSceneRenderOne DRAW\n");
		if (ged3DConfig.bEnableProfile != 0) {
			IMPLEMENTATION_GUARD(
			if ((gIDProfileFlush & 1U) == 0) {
				uVar6 = (uint)gIDProfileFlush >> 4;
				if (ProfileManagerB[uVar6].field_0x34 != 0) {
					ProfileManagerB[uVar6].field_0x8 = 0;
					ProfileManagerB[uVar6].fileCount = 0;
					ProfileManagerB[uVar6].field_0x34 = 0;
				}
				ProfileManagerB[(uint)iVar3 >> 4].field_0x0 = (long)PCR1 & 0xffffffff;
				ProfileManagerB[(uint)iVar3 >> 4].fileCount = ProfileManagerB[(uint)iVar3 >> 4].fileCount + 1;
			}
			else {
				uVar6 = (uint)gIDProfileFlush >> 4;
				if (g_ProfileObjA_0041ed40[uVar6].field_0x34 != 0) {
					g_ProfileObjA_0041ed40[uVar6].field_0x8 = 0;
					g_ProfileObjA_0041ed40[uVar6].fileCount = 0;
					g_ProfileObjA_0041ed40[uVar6].field_0x34 = 0;
				}
				g_ProfileObjA_0041ed40[(uint)iVar3 >> 4].field_0x0 = (long)PCR1 & 0xffffffff;
				g_ProfileObjA_0041ed40[(uint)iVar3 >> 4].fileCount = g_ProfileObjA_0041ed40[(uint)iVar3 >> 4].fileCount + 1;
			})
		}
		ed3DFlushSceneInit();
		uVar6 = ed3DInitRenderEnvironement(pInStaticMeshMaster, (ulong)((pInStaticMeshMaster->flags_0x4 & 0x20) != 0));
		uVar6 = uVar6 & 0xff;
		if (ged3DConfig.bEnableProfile != 0) {
			IMPLEMENTATION_GUARD(
			uVar5 = (uint)gIDProfileFlush >> 4;
			if ((gIDProfileFlush & 1U) == 0) {
				ProfileManagerB[uVar5].field_0x0 = (long)PCR1 - ProfileManagerB[uVar5].field_0x0;
				ProfileManagerB[uVar5].field_0x8 =
					ProfileManagerB[uVar5].field_0x8 + ProfileManagerB[uVar5].field_0x0;
				pPVar7 = ProfileManagerB + uVar5;
				if (pPVar7->field_0x10 < ProfileManagerB[uVar5].field_0x0) {
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
		iVar3 = gIDProfileFlush;
		if (uVar6 != 0) {
			gCurSceneID = gCurSceneID + 1;
			if (ged3DConfig.bEnableProfile != 0) {
				IMPLEMENTATION_GUARD(
				if ((gIDProfileFlush & 1U) == 0) {
					uVar5 = (uint)gIDProfileFlush >> 4;
					if (ProfileManagerB[uVar5].field_0x34 != 0) {
						ProfileManagerB[uVar5].field_0x8 = 0;
						ProfileManagerB[uVar5].fileCount = 0;
						ProfileManagerB[uVar5].field_0x34 = 0;
					}
					ProfileManagerB[(uint)iVar3 >> 4].field_0x0 = (long)PCR1 & 0xffffffff;
					ProfileManagerB[(uint)iVar3 >> 4].fileCount = ProfileManagerB[(uint)iVar3 >> 4].fileCount + 1;
				}
				else {
					uVar5 = (uint)gIDProfileFlush >> 4;
					if (g_ProfileObjA_0041ed40[uVar5].field_0x34 != 0) {
						g_ProfileObjA_0041ed40[uVar5].field_0x8 = 0;
						g_ProfileObjA_0041ed40[uVar5].fileCount = 0;
						g_ProfileObjA_0041ed40[uVar5].field_0x34 = 0;
					}
					g_ProfileObjA_0041ed40[(uint)iVar3 >> 4].field_0x0 = (long)PCR1 & 0xffffffff;
					g_ProfileObjA_0041ed40[(uint)iVar3 >> 4].fileCount = g_ProfileObjA_0041ed40[(uint)iVar3 >> 4].fileCount + 1;
				})
			}
			ed3DInitVU1Globals();
			g_VifRefPktCur = ed3DSceneAddContextPacket(pInStaticMeshMaster, g_VifRefPktCur);
			if (ged3DConfig.bEnableProfile != 0) {
				IMPLEMENTATION_GUARD(
				uVar5 = (uint)gIDProfileFlush >> 4;
				if ((gIDProfileFlush & 1U) == 0) {
					ProfileManagerB[uVar5].field_0x0 = (long)PCR1 - ProfileManagerB[uVar5].field_0x0;
					ProfileManagerB[uVar5].field_0x8 =
						ProfileManagerB[uVar5].field_0x8 + ProfileManagerB[uVar5].field_0x0;
					pPVar7 = ProfileManagerB + uVar5;
					if (pPVar7->field_0x10 < ProfileManagerB[uVar5].field_0x0) {
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
				iVar3 = gIDProfileRender;
				if ((gIDProfileRender & 1U) == 0) {
					uVar5 = (uint)gIDProfileRender >> 4;
					if (ProfileManagerB[uVar5].field_0x34 != 0) {
						ProfileManagerB[uVar5].field_0x8 = 0;
						ProfileManagerB[uVar5].fileCount = 0;
						ProfileManagerB[uVar5].field_0x34 = 0;
					}
					ProfileManagerB[(uint)iVar3 >> 4].field_0x0 = (long)PCR1 & 0xffffffff;
					ProfileManagerB[(uint)iVar3 >> 4].fileCount = ProfileManagerB[(uint)iVar3 >> 4].fileCount + 1;
				}
				else {
					uVar5 = (uint)gIDProfileRender >> 4;
					if (g_ProfileObjA_0041ed40[uVar5].field_0x34 != 0) {
						g_ProfileObjA_0041ed40[uVar5].field_0x8 = 0;
						g_ProfileObjA_0041ed40[uVar5].fileCount = 0;
						g_ProfileObjA_0041ed40[uVar5].field_0x34 = 0;
					}
					g_ProfileObjA_0041ed40[(uint)iVar3 >> 4].field_0x0 = (long)PCR1 & 0xffffffff;
					g_ProfileObjA_0041ed40[(uint)iVar3 >> 4].fileCount = g_ProfileObjA_0041ed40[(uint)iVar3 >> 4].fileCount + 1;
				})
			}
			pCVar8 = (MeshTransformParent*)pStaticMeshMaster->pHeirListA;
			pCVar9 = pCVar8->pNext;
			if (*ScratchpadAnimManager_004494ec.gShadowRenderMask == 0) {
				if (ged3DConfig.field_0x2d == 0) {
					for (; pCVar9 != pCVar8; pCVar9 = pCVar9->pNext) {
						ed3DRenderSonHierarchy((MeshTransformData*)pCVar9->pMeshTransformData);
					}
				}
				else {
					for (; pCVar9 != pCVar8; pCVar9 = pCVar9->pNext) {
						IMPLEMENTATION_GUARD(ed3DRenderSonHierarchyHIDDEN(pCVar9->pMeshTransformData);)
					}
				}
			}
			else {
				for (; pCVar9 != pCVar8; pCVar9 = pCVar9->pNext) {
					ed3DRenderSonHierarchyForShadow((MeshTransformData*)pCVar9->pMeshTransformData);
				}
			}
			ed3DClustersRenderComputeSceneHierarchyList(pInStaticMeshMaster);
			if ((pInStaticMeshMaster->bShadowScene != 1) &&
				(ed3DSceneSortClusters(pInStaticMeshMaster), (pInStaticMeshMaster->flags_0x4 & 8) == 0)) {
				for (pCVar10 = (MeshTransformParent*)(pStaticMeshMaster->headerB).pLoadedData;
					(edLIST*)pCVar10 != &pStaticMeshMaster->headerB; pCVar10 = pCVar10->pNext) {
						if (((pCVar10->pMeshTransformData->flags & 2) == 0) &&
							(pMeshInfo = pCVar10->pMeshTransformData->pMeshInfo, pMeshInfo->CSTA != (char*)0x0)) {
							bVar4 = ed3DSceneRenderCluster(pMeshInfo);
							uVar6 = (int)bVar4 & 0xff;
					}
				}
			}
			renderTaskData.pStaticMeshMaster = pStaticMeshMaster;
			if (BYTE_00448a5c == 0) {
				if (pInStaticMeshMaster == pStaticMeshMaster) {
					renderTaskData.isChild = pInStaticMeshMaster->bShadowScene;
					renderTaskData.taskID = 2;
					edSysHandlersCall(sysHandler_0048cb90.mainIdentifier, sysHandler_0048cb90.entries,
						sysHandler_0048cb90.maxEventID, 0, &renderTaskData);
				}
				else {
					renderTaskData.isChild = pInStaticMeshMaster->bShadowScene;
					renderTaskData.taskID = 1;
					if (pInStaticMeshMaster != pStaticMeshMaster) {
						renderTaskData.taskID = 3;
					}
					edSysHandlersCall(sysHandler_0048cb90.mainIdentifier, sysHandler_0048cb90.entries,
						sysHandler_0048cb90.maxEventID, 0, &renderTaskData);
				}
			}
			else {
				renderTaskData.isChild = pInStaticMeshMaster->bShadowScene;
				renderTaskData.taskID = 1;
				if (pInStaticMeshMaster != pStaticMeshMaster) {
					renderTaskData.taskID = 3;
				}
				edSysHandlersCall(sysHandler_0048cb90.mainIdentifier, sysHandler_0048cb90.entries,
					sysHandler_0048cb90.maxEventID, 0, &renderTaskData);
			}
			if (ged3DConfig.bEnableProfile != 0) {
				IMPLEMENTATION_GUARD(
				uVar5 = (uint)gIDProfileRender >> 4;
				if ((gIDProfileRender & 1U) == 0) {
					ProfileManagerB[uVar5].field_0x0 = (long)PCR1 - ProfileManagerB[uVar5].field_0x0;
					ProfileManagerB[uVar5].field_0x8 =
						ProfileManagerB[uVar5].field_0x8 + ProfileManagerB[uVar5].field_0x0;
					pPVar7 = ProfileManagerB + uVar5;
					if (pPVar7->field_0x10 < ProfileManagerB[uVar5].field_0x0) {
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
				iVar3 = gIDProfileFlush;
				if ((gIDProfileFlush & 1U) == 0) {
					uVar5 = (uint)gIDProfileFlush >> 4;
					if (ProfileManagerB[uVar5].field_0x34 != 0) {
						ProfileManagerB[uVar5].field_0x8 = 0;
						ProfileManagerB[uVar5].fileCount = 0;
						ProfileManagerB[uVar5].field_0x34 = 0;
					}
					ProfileManagerB[(uint)iVar3 >> 4].field_0x0 = (long)PCR1 & 0xffffffff;
					ProfileManagerB[(uint)iVar3 >> 4].fileCount = ProfileManagerB[(uint)iVar3 >> 4].fileCount + 1;
				}
				else {
					uVar5 = (uint)gIDProfileFlush >> 4;
					if (g_ProfileObjA_0041ed40[uVar5].field_0x34 != 0) {
						g_ProfileObjA_0041ed40[uVar5].field_0x8 = 0;
						g_ProfileObjA_0041ed40[uVar5].fileCount = 0;
						g_ProfileObjA_0041ed40[uVar5].field_0x34 = 0;
					}
					g_ProfileObjA_0041ed40[(uint)iVar3 >> 4].field_0x0 = (long)PCR1 & 0xffffffff;
					g_ProfileObjA_0041ed40[(uint)iVar3 >> 4].fileCount = g_ProfileObjA_0041ed40[(uint)iVar3 >> 4].fileCount + 1;
				})
			}
			if (pInStaticMeshMaster->bShadowScene == 1) {
				g_VifRefPktCur->cmdA = 0xe10000002;
				g_VifRefPktCur->cmdB = 0;
				*(undefined4*)&g_VifRefPktCur->cmdB = 0;
				*(undefined4*)((ulong)&g_VifRefPktCur->cmdB + 4) = 0x50000002;
				g_VifRefPktCur = g_VifRefPktCur + 1;

				// TAG
				g_VifRefPktCur->cmdA = SCE_GIF_SET_TAG(
					1,				// NLOOP
					SCE_GS_TRUE,	// EOP
					SCE_GS_FALSE,	// PRE
					1,				// PRIM
					SCE_GIF_PACKED,	// FLG
					1				// NREG
				);
				g_VifRefPktCur->cmdB = SCE_GIF_PACKED_AD;
				g_VifRefPktCur = g_VifRefPktCur + 1;

				// ZBUF
				g_VifRefPktCur->cmdA = SCE_GS_SET_ZBUF(
					pInStaticMeshMaster->pViewport->pZBuffer->frameBasePtr,	// ZBP
					SCE_GS_PSMZ32,						// PSM
					SCE_GS_TRUE							// ZMASK
				);
				g_VifRefPktCur->cmdB = SCE_GS_ZBUF_1;
				g_VifRefPktCur = g_VifRefPktCur + 1;

				ed3DFlushList();
				ed3DFlushShadowList();
			}
			else {
				ed3DFlushList();
			}
			g_VifRefPktCur = ed3DFlushResetOffset(g_VifRefPktCur, &gCurRectViewport);
			pCVar2 = pInStaticMeshMaster->pViewport;
			CameraScreenData_00449560.posX = pCVar2->posX;
			CameraScreenData_00449560.posY = pCVar2->posY;
			CameraScreenData_00449560.screenWidth = pCVar2->screenWidth;
			CameraScreenData_00449560.screenHeight = pCVar2->screenHeight;
			g_VifRefPktCur = ed3DFlushResetOffset(g_VifRefPktCur, &gCurRectViewport);
			ed3DPrimlistMatrixBufferReset();
			if (ged3DConfig.bEnableProfile != 0) {
				IMPLEMENTATION_GUARD(
				uVar5 = (uint)gIDProfileFlush >> 4;
				if ((gIDProfileFlush & 1U) == 0) {
					ProfileManagerB[uVar5].field_0x0 = (long)PCR1 - ProfileManagerB[uVar5].field_0x0;
					ProfileManagerB[uVar5].field_0x8 =
						ProfileManagerB[uVar5].field_0x8 + ProfileManagerB[uVar5].field_0x0;
					pPVar7 = ProfileManagerB + uVar5;
					if (pPVar7->field_0x10 < ProfileManagerB[uVar5].field_0x0) {
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

uint ed3DSceneRenderDlist(ed_3D_Scene* pStaticMeshMaster)
{
	bool bVar1;
	bool bVar2;
	byte bVar3;
	int iVar4;
	edpkt_data* pRVar5;
	uint uVar6;
	edpkt_data* pRVar7;
	uint uVar8;
	//ProfileObject* pPVar9;
	RenderTaskData renderTaskData;

	RENDER_LOG("ed3DSceneRenderDlist %p (%d) (child: %d)\n", pStaticMeshMaster, GetStaticMeshMasterIndex(pStaticMeshMaster), pStaticMeshMaster->bShadowScene);

	iVar4 = gIDProfileFlush;
	if (ged3DConfig.bEnableProfile != 0) {
		IMPLEMENTATION_GUARD(
		if ((gIDProfileFlush & 1U) == 0) {
			uVar8 = (uint)gIDProfileFlush >> 4;
			if (ProfileManagerB[uVar8].field_0x34 != 0) {
				ProfileManagerB[uVar8].field_0x8 = 0;
				ProfileManagerB[uVar8].fileCount = 0;
				ProfileManagerB[uVar8].field_0x34 = 0;
			}
			ProfileManagerB[(uint)iVar4 >> 4].field_0x0 = (long)PCR1 & 0xffffffff;
			ProfileManagerB[(uint)iVar4 >> 4].fileCount = ProfileManagerB[(uint)iVar4 >> 4].fileCount + 1;
		}
		else {
			uVar8 = (uint)gIDProfileFlush >> 4;
			if (g_ProfileObjA_0041ed40[uVar8].field_0x34 != 0) {
				g_ProfileObjA_0041ed40[uVar8].field_0x8 = 0;
				g_ProfileObjA_0041ed40[uVar8].fileCount = 0;
				g_ProfileObjA_0041ed40[uVar8].field_0x34 = 0;
			}
			g_ProfileObjA_0041ed40[(uint)iVar4 >> 4].field_0x0 = (long)PCR1 & 0xffffffff;
			g_ProfileObjA_0041ed40[(uint)iVar4 >> 4].fileCount = g_ProfileObjA_0041ed40[(uint)iVar4 >> 4].fileCount + 1;
		})
	}
	pRVar5 = ed3DFlushResetOffset(g_VifRefPktCur, &gCurRectViewport);
	g_VifRefPktCur = pRVar5;
	ed3DFlushSceneInit();
	uVar8 = ed3DInitRenderEnvironement(pStaticMeshMaster, (ulong)((pStaticMeshMaster->flags_0x4 & 0x20) != 0));
	if (ged3DConfig.bEnableProfile != 0) {
		IMPLEMENTATION_GUARD(
		uVar6 = (uint)gIDProfileFlush >> 4;
		if ((gIDProfileFlush & 1U) == 0) {
			ProfileManagerB[uVar6].field_0x0 = (long)PCR1 - ProfileManagerB[uVar6].field_0x0;
			ProfileManagerB[uVar6].field_0x8 =
				ProfileManagerB[uVar6].field_0x8 + ProfileManagerB[uVar6].field_0x0;
			pPVar9 = ProfileManagerB + uVar6;
			if (pPVar9->field_0x10 < ProfileManagerB[uVar6].field_0x0) {
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
	iVar4 = gIDProfileFlush;
	if ((uVar8 & 0xff) != 0) {
		if (ged3DConfig.bEnableProfile != 0) {
			IMPLEMENTATION_GUARD(
			if ((gIDProfileFlush & 1U) == 0) {
				uVar6 = (uint)gIDProfileFlush >> 4;
				if (ProfileManagerB[uVar6].field_0x34 != 0) {
					ProfileManagerB[uVar6].field_0x8 = 0;
					ProfileManagerB[uVar6].fileCount = 0;
					ProfileManagerB[uVar6].field_0x34 = 0;
				}
				ProfileManagerB[(uint)iVar4 >> 4].field_0x0 = (long)PCR1 & 0xffffffff;
				ProfileManagerB[(uint)iVar4 >> 4].fileCount = ProfileManagerB[(uint)iVar4 >> 4].fileCount + 1;
			}
			else {
				uVar6 = (uint)gIDProfileFlush >> 4;
				if (g_ProfileObjA_0041ed40[uVar6].field_0x34 != 0) {
					g_ProfileObjA_0041ed40[uVar6].field_0x8 = 0;
					g_ProfileObjA_0041ed40[uVar6].fileCount = 0;
					g_ProfileObjA_0041ed40[uVar6].field_0x34 = 0;
				}
				g_ProfileObjA_0041ed40[(uint)iVar4 >> 4].field_0x0 = (long)PCR1 & 0xffffffff;
				g_ProfileObjA_0041ed40[(uint)iVar4 >> 4].fileCount = g_ProfileObjA_0041ed40[(uint)iVar4 >> 4].fileCount + 1;
			})
		}
		ed3DInitVU1Globals();
		bVar3 = pStaticMeshMaster->pViewport->clearColor.a;
		pStaticMeshMaster->pViewport->clearColor.a = bVar3 & 0xfc;
		g_VifRefPktCur = ed3DSceneAddContextPacket(pStaticMeshMaster, g_VifRefPktCur);
		pStaticMeshMaster->pViewport->clearColor.a = bVar3;
		if (ged3DConfig.bEnableProfile != 0) {
			IMPLEMENTATION_GUARD(
			uVar6 = (uint)gIDProfileFlush >> 4;
			if ((gIDProfileFlush & 1U) == 0) {
				ProfileManagerB[uVar6].field_0x0 = (long)PCR1 - ProfileManagerB[uVar6].field_0x0;
				ProfileManagerB[uVar6].field_0x8 =
					ProfileManagerB[uVar6].field_0x8 + ProfileManagerB[uVar6].field_0x0;
				pPVar9 = ProfileManagerB + uVar6;
				if (pPVar9->field_0x10 < ProfileManagerB[uVar6].field_0x0) {
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
			iVar4 = gIDProfileRender;
			if ((gIDProfileRender & 1U) == 0) {
				uVar6 = (uint)gIDProfileRender >> 4;
				if (ProfileManagerB[uVar6].field_0x34 != 0) {
					ProfileManagerB[uVar6].field_0x8 = 0;
					ProfileManagerB[uVar6].fileCount = 0;
					ProfileManagerB[uVar6].field_0x34 = 0;
				}
				ProfileManagerB[(uint)iVar4 >> 4].field_0x0 = (long)PCR1 & 0xffffffff;
				ProfileManagerB[(uint)iVar4 >> 4].fileCount = ProfileManagerB[(uint)iVar4 >> 4].fileCount + 1;
			}
			else {
				uVar6 = (uint)gIDProfileRender >> 4;
				if (g_ProfileObjA_0041ed40[uVar6].field_0x34 != 0) {
					g_ProfileObjA_0041ed40[uVar6].field_0x8 = 0;
					g_ProfileObjA_0041ed40[uVar6].fileCount = 0;
					g_ProfileObjA_0041ed40[uVar6].field_0x34 = 0;
				}
				g_ProfileObjA_0041ed40[(uint)iVar4 >> 4].field_0x0 = (long)PCR1 & 0xffffffff;
				g_ProfileObjA_0041ed40[(uint)iVar4 >> 4].fileCount = g_ProfileObjA_0041ed40[(uint)iVar4 >> 4].fileCount + 1;
			})
		}
		renderTaskData.isChild = pStaticMeshMaster->bShadowScene;
		renderTaskData.taskID = 2;
		renderTaskData.pStaticMeshMaster = pStaticMeshMaster;
		edSysHandlersCall(sysHandler_0048cb90.mainIdentifier, sysHandler_0048cb90.entries,
			sysHandler_0048cb90.maxEventID, 0, &renderTaskData);
		if (ged3DConfig.bEnableProfile != 0) {
			IMPLEMENTATION_GUARD(
			uVar6 = (uint)gIDProfileRender >> 4;
			if ((gIDProfileRender & 1U) == 0) {
				ProfileManagerB[uVar6].field_0x0 = (long)PCR1 - ProfileManagerB[uVar6].field_0x0;
				ProfileManagerB[uVar6].field_0x8 =
					ProfileManagerB[uVar6].field_0x8 + ProfileManagerB[uVar6].field_0x0;
				pPVar9 = ProfileManagerB + uVar6;
				if (pPVar9->field_0x10 < ProfileManagerB[uVar6].field_0x0) {
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
			iVar4 = gIDProfileFlush;
			if ((gIDProfileFlush & 1U) == 0) {
				uVar6 = (uint)gIDProfileFlush >> 4;
				if (ProfileManagerB[uVar6].field_0x34 != 0) {
					ProfileManagerB[uVar6].field_0x8 = 0;
					ProfileManagerB[uVar6].fileCount = 0;
					ProfileManagerB[uVar6].field_0x34 = 0;
				}
				ProfileManagerB[(uint)iVar4 >> 4].field_0x0 = (long)PCR1 & 0xffffffff;
				ProfileManagerB[(uint)iVar4 >> 4].fileCount = ProfileManagerB[(uint)iVar4 >> 4].fileCount + 1;
			}
			else {
				uVar6 = (uint)gIDProfileFlush >> 4;
				if (g_ProfileObjA_0041ed40[uVar6].field_0x34 != 0) {
					g_ProfileObjA_0041ed40[uVar6].field_0x8 = 0;
					g_ProfileObjA_0041ed40[uVar6].fileCount = 0;
					g_ProfileObjA_0041ed40[uVar6].field_0x34 = 0;
				}
				g_ProfileObjA_0041ed40[(uint)iVar4 >> 4].field_0x0 = (long)PCR1 & 0xffffffff;
				g_ProfileObjA_0041ed40[(uint)iVar4 >> 4].fileCount = g_ProfileObjA_0041ed40[(uint)iVar4 >> 4].fileCount + 1;
			})
		}
		pRVar7 = ed3DFlushResetOffset(g_VifRefPktCur, &gCurRectViewport);
		g_VifRefPktCur = pRVar7;
		ed3DFlushList();
		bVar1 = g_VifRefPktCur == pRVar7;
		pRVar7 = ed3DFlushResetOffset(g_VifRefPktCur, &gCurRectViewport);
		g_VifRefPktCur = pRVar7;
		ed3DFlushShadowList();
		bVar2 = g_VifRefPktCur == pRVar7;
		ed3DPrimlistMatrixBufferReset();
		if (bVar2 && bVar1) {
			g_VifRefPktCur = pRVar5;
		}
		pRVar5 = g_VifRefPktCur;
		if (ged3DConfig.bEnableProfile != 0) {
			IMPLEMENTATION_GUARD(
			uVar6 = (uint)gIDProfileFlush >> 4;
			if ((gIDProfileFlush & 1U) == 0) {
				ProfileManagerB[uVar6].field_0x0 = (long)PCR1 - ProfileManagerB[uVar6].field_0x0;
				ProfileManagerB[uVar6].field_0x8 =
					ProfileManagerB[uVar6].field_0x8 + ProfileManagerB[uVar6].field_0x0;
				pPVar9 = ProfileManagerB + uVar6;
				pRVar5 = g_VifRefPktCur;
				if (pPVar9->field_0x10 < ProfileManagerB[uVar6].field_0x0) {
					pPVar9->field_0x10 = pPVar9->field_0x0;
					pRVar5 = g_VifRefPktCur;
				}
			}
			else {
				g_ProfileObjA_0041ed40[uVar6].field_0x0 = (long)PCR1 - g_ProfileObjA_0041ed40[uVar6].field_0x0;
				g_ProfileObjA_0041ed40[uVar6].field_0x8 =
					g_ProfileObjA_0041ed40[uVar6].field_0x8 + g_ProfileObjA_0041ed40[uVar6].field_0x0;
				pPVar9 = g_ProfileObjA_0041ed40 + uVar6;
				pRVar5 = g_VifRefPktCur;
				if (pPVar9->field_0x10 < g_ProfileObjA_0041ed40[uVar6].field_0x0) {
					pPVar9->field_0x10 = pPVar9->field_0x0;
					pRVar5 = g_VifRefPktCur;
				}
			})
		}
	}
	g_VifRefPktCur = pRVar5;
	return uVar8 & 0xff;
}

byte gBlurON = 0;
ed_viewport* PTR_00449388 = NULL;

edpkt_data* ed3DAddViewport2DAfter3D(edpkt_data* param_1)
{
	byte bVar1;
	edpkt_data* pRVar2;
	edpkt_data* pRVar3;
	uint uVar4;

	pRVar3 = param_1;
	if (PTR_00449388 == (ed_viewport*)0x0) {
		if ((gDebugViewport != (ed_viewport*)0x0) && (gDebugViewport != (ed_viewport*)0x0)) {
			bVar1 = gDebugViewport->clearColor.a;
			gDebugViewport->clearColor.a = 0;
			pRVar2 = g_VifRefPktCur;
			if (gDebugViewport != (ed_viewport*)0x0) {
				g_VifRefPktCur = edViewportUpdateEnv(gDebugViewport, g_VifRefPktCur + 1);
				uVar4 = ((uint)((ulong)g_VifRefPktCur - (ulong)pRVar2) >> 4) - 1;
				pRVar2->cmdA = (ulong)(int)uVar4 & 0xffffffffU | 0xe10000000;
				pRVar2->cmdB = 0;
				*(undefined4*)&pRVar2->cmdB = 0;
				*(uint*)((ulong)&pRVar2->cmdB + 4) = uVar4 | 0x51000000;
			}
			gDebugViewport->clearColor.a = bVar1;
		}
	}
	else {
		if (PTR_00449388 != (ed_viewport*)0x0) {
			pRVar3 = edViewportUpdateEnv(PTR_00449388, param_1 + 1);
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

edpkt_data* ed3DDMAAddFinish(edpkt_data* param_1)
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

void ed3DFlushFogFX(void)
{
	ushort** ppuVar1;
	ulong* puVar2;
	int iVar3;
	edpkt_data* pRVar4;

	pRVar4 = g_VifRefPktCur;
	if (((UINT_00425000 & 1) != 0) && (gCurViewportUsed != (ed_viewport*)0x0)) {
		IMPLEMENTATION_GUARD();
	}
	return;
}

edpkt_data* ed3DFlushAAEffect(edpkt_data* pRenderCommand)
{
	ulong* puVar1;
	ushort** ppuVar2;
	int iVar3;

	if (((((BYTE_00448a70 != 0) && (BYTE_00449418 != 0)) && (gCurViewportUsed != (ed_viewport*)0x0)) &&
		(gCurRectViewport.screenWidth == 0x200)) &&
		((gCurRectViewport.screenHeight == 0x200 || (gCurRectViewport.screenHeight == 0x1c0)))) {
		IMPLEMENTATION_GUARD();
	}
	return pRenderCommand;
}

void ed3DPrimlistTermMaterialRenderList(void)
{
	if (gPrim_List_FlushTex_Last != (MeshTransformSpecial*)0x0) {
		gPrim_List_FlushTex_Last->pNext_0x4 =
			(MeshTransformSpecial*)(gPrim_List_FlushTex + gCurRenderList);
		RENDER_LOG("ed3DPrimlistTermMaterialRenderList %p\n", (MeshTransformSpecial*)(gPrim_List_FlushTex + gCurRenderList));
	}
	return;
}

Vector Vector_0048c3d0 = { 0 };
byte BYTE_0044941c = 0;

int ed3DVU1Addr[3] = { 0x76, 0x198, 0x2BA };
int ed3DVU1AddrWithBufCur[9] = { 0x76, 0x198, 0x2BA, 0x198, 0x2BA, 0x76, 0x2BA, 0x76, 0x198 };

void ed3DRefreshSracthGlobalVar(void)
{
	AnimScratchpad* pAVar1;
	int in_zero_lo;
	uint in_zero_hi;
	undefined4 in_zero_udw;
	undefined4 in_register_0000000c;
	int iVar2;
	edF32MATRIX4* pMVar3;
	Vector* pVVar4;
	edF32MATRIX4* pMVar5;
	float fVar6;
	float fVar7;
	float fVar8;

	if (gLightDirections_Matrix != (Vector*)0x0) {
		iVar2 = 8;
		pVVar4 = gLightDirections_Matrix;
		pMVar3 = ScratchpadAnimManager_004494ec.pLightDirections;
		do {
			iVar2 = iVar2 + -1;
			fVar6 = pVVar4->y;
			pMVar3->aa = pVVar4->x;
			pVVar4 = (Vector*)&pVVar4->z;
			pMVar3->ab = fVar6;
			pMVar3 = (edF32MATRIX4*)&pMVar3->ac;
		} while (0 < iVar2);
	}
	if (gLightColor_Matrix != (Vector*)0x0) {
		iVar2 = 8;
		pVVar4 = gLightColor_Matrix;
		pMVar3 = ScratchpadAnimManager_004494ec.pLightColor;
		do {
			iVar2 = iVar2 + -1;
			fVar6 = pVVar4->y;
			pMVar3->aa = pVVar4->x;
			pVVar4 = (Vector*)&pVVar4->z;
			pMVar3->ab = fVar6;
			pMVar3 = (edF32MATRIX4*)&pMVar3->ac;
		} while (0 < iVar2);
	}
	pVVar4 = ScratchpadAnimManager_004494ec.pLightAmbient;
	if (gLightAmbiant != (Vector*)0x0) {
		fVar8 = gLightAmbiant->y;
		fVar6 = gLightAmbiant->z;
		fVar7 = gLightAmbiant->w;
		(ScratchpadAnimManager_004494ec.pLightAmbient)->x = gLightAmbiant->x;
		pVVar4->y = fVar8;
		pVVar4->z = fVar6;
		pVVar4->w = fVar7;
	}
	fVar8 = Vector_0048c3d0.w;
	fVar7 = Vector_0048c3d0.z;
	fVar6 = Vector_0048c3d0.y;
	pVVar4 = ScratchpadAnimManager_004494ec.pVector_0x18;
	pMVar5 = &g_IdentityMatrix;
	iVar2 = 8;
	(ScratchpadAnimManager_004494ec.pVector_0x18)->x = Vector_0048c3d0.x;
	pVVar4->y = fVar6;
	pVVar4->z = fVar7;
	pVVar4->w = fVar8;
	pMVar3 = ScratchpadAnimManager_004494ec.pMatrix_0x30;
	do {
		iVar2 = iVar2 + -1;
		fVar6 = pMVar5->ab;
		pMVar3->aa = pMVar5->aa;
		pMVar5 = (edF32MATRIX4*)&pMVar5->ac;
		pMVar3->ab = fVar6;
		pMVar3 = (edF32MATRIX4*)&pMVar3->ac;
	} while (0 < iVar2);
	*(uint*)ScratchpadAnimManager_004494ec.field_0x40 = (uint)gbDoMipmap;
	*(uint*)ScratchpadAnimManager_004494ec.field_0x44 = gMipmapL;
	*(uint*)ScratchpadAnimManager_004494ec.field_0x48 = gMipmapK;
	*(uint*)ScratchpadAnimManager_004494ec.field_0x4c = (uint)BYTE_0044941c;
	pAVar1 = ScratchpadAnimManager_004494ec.pAnimScratchpad;
	(ScratchpadAnimManager_004494ec.pAnimScratchpad)->field_0x0 = in_zero_lo;
	pAVar1->flags = 0;
	pAVar1->field_0x8 = 0;
	pAVar1->field_0xc = 0;
	*(undefined4*)ScratchpadAnimManager_004494ec.field_0x1c = 0x700008a0;
	memcpy(ScratchpadAnimManager_004494ec.field_0x34, ed3DVU1Addr, 0xc);
	memcpy(ScratchpadAnimManager_004494ec.field_0x38, ed3DVU1AddrWithBufCur, 0x24);
	return;
}

void ed3DSceneRender(int, int, char*)
{
	ed_viewport* pCVar1;
	ulong uVar2;
	bool bVar3;
	int iVar4;
	edpkt_data* pRVar5;
	undefined4 in_zero_lo;
	undefined4 in_zero_hi;
	undefined4 in_zero_udw;
	undefined4 in_register_0000000c;
	edSurface* pFVar6;
	uint uVar7;
	//ProfileObject* pPVar8;
	undefined4 uVar9;
	undefined4 uVar10;
	undefined4 uVar11;
	edLIST* pCVar12;
	ed_3D_Scene* pInStaticMeshMaster;
	uint staticMeshMasterIndex;
	uint staticMeshMasterFlags;

	iVar4 = gIDProfileRender;
	if (bcalcFrame == false) {
		bcalcFrame = true;
		g_VifRefPktCur = g_pStartPktData;
		g_GifRefPktCur = g_pGifStart;
	}
	else {
		gCurViewportUsed = (ed_viewport*)0x0;
		gCurFrame = gCurFrame != 0 ^ 1;
		if (ged3DConfig.bEnableProfile != 0) {
			IMPLEMENTATION_GUARD(
			if ((gIDProfileRender & 1U) == 0) {
				uVar6 = (uint)gIDProfileRender >> 4;
				if (ProfileManagerB[uVar6].field_0x34 != 0) {
					ProfileManagerB[uVar6].field_0x8 = 0;
					ProfileManagerB[uVar6].field_0x20 = 0;
					ProfileManagerB[uVar6].field_0x34 = 0;
				}
				ProfileManagerB[(uint)iVar5 >> 4].field_0x0 = (long)PCR1 & 0xffffffff;
				ProfileManagerB[(uint)iVar5 >> 4].field_0x20 = ProfileManagerB[(uint)iVar5 >> 4].field_0x20 + 1;
			}
			else {
				uVar6 = (uint)gIDProfileRender >> 4;
				if (g_ProfileObjA_0041ed40[uVar6].field_0x34 != 0) {
					g_ProfileObjA_0041ed40[uVar6].field_0x8 = 0;
					g_ProfileObjA_0041ed40[uVar6].field_0x20 = 0;
					g_ProfileObjA_0041ed40[uVar6].field_0x34 = 0;
				}
				g_ProfileObjA_0041ed40[(uint)iVar5 >> 4].field_0x0 = (long)PCR1 & 0xffffffff;
				g_ProfileObjA_0041ed40[(uint)iVar5 >> 4].field_0x20 = g_ProfileObjA_0041ed40[(uint)iVar5 >> 4].field_0x20 + 1;
			})
		}
		ed3DRefreshSracthGlobalVar();
		if ((ged3DConfig.field_0x2c & 1) != 0) {
			ed3DComputeSonHierarchy();
		}
		if (ged3DConfig.bEnableProfile != 0) {
			IMPLEMENTATION_GUARD(
			uVar6 = (uint)gIDProfileRender >> 4;
			if ((gIDProfileRender & 1U) == 0) {
				ProfileManagerB[uVar6].field_0x0 = (long)PCR1 - ProfileManagerB[uVar6].field_0x0;
				ProfileManagerB[uVar6].field_0x8 =
					ProfileManagerB[uVar6].field_0x8 + ProfileManagerB[uVar6].field_0x0;
				pPVar7 = ProfileManagerB + uVar6;
				if (pPVar7->field_0x10 < ProfileManagerB[uVar6].field_0x0) {
					pPVar7->field_0x10 = pPVar7->field_0x0;
				}
			}
			else {
				g_ProfileObjA_0041ed40[uVar6].field_0x0 = (long)PCR1 - g_ProfileObjA_0041ed40[uVar6].field_0x0;
				g_ProfileObjA_0041ed40[uVar6].field_0x8 =
					g_ProfileObjA_0041ed40[uVar6].field_0x8 + g_ProfileObjA_0041ed40[uVar6].field_0x0;
				pPVar7 = g_ProfileObjA_0041ed40 + uVar6;
				if (pPVar7->field_0x10 < g_ProfileObjA_0041ed40[uVar6].field_0x0) {
					pPVar7->field_0x10 = pPVar7->field_0x0;
				}
			})
		}
		bVar3 = false;
		for (staticMeshMasterIndex = 0; staticMeshMasterIndex < (uint)ged3DConfig.sceneCount;
			staticMeshMasterIndex = staticMeshMasterIndex + 1) {
			staticMeshMasterFlags = gScene3D[staticMeshMasterIndex].flags_0x4;
			if ((((staticMeshMasterFlags & 1) != 0) && ((staticMeshMasterFlags & 4) == 0)) &&
				(gScene3D[staticMeshMasterIndex].bShadowScene == 0)) {
				if (bVar3) {
					bVar3 = false;
					pFVar6 = edVideoGetDrawSurface();
					if ((pFVar6 != (edSurface*)0x0) && (gpPKTDataRefMasPath3 != (edpkt_data*)0x0)) {
						g_VifRefPktCur->cmdA = 0x30000000;
						g_VifRefPktCur->cmdB = 0x1300000000000000;
						edpkt_data* pRVar5 = g_VifRefPktCur;
						g_VifRefPktCur = g_VifRefPktCur + 1;
						uVar2 = gpPKTDataRefMasPath3->cmdA;
						uVar9 = *(undefined4*)&gpPKTDataRefMasPath3->cmdB;
						uVar10 = *(undefined4*)((ulong)&gpPKTDataRefMasPath3->cmdB + 4);
						*(int*)&g_VifRefPktCur->cmdA = (int)uVar2;
						*(int*)((ulong)&pRVar5[1].cmdA + 4) = (int)(uVar2 >> 0x20);
						*(undefined4*)&pRVar5[1].cmdB = uVar9;
						*(undefined4*)((ulong)&pRVar5[1].cmdB + 4) = uVar10;
						pRVar5 = g_VifRefPktCur;
						g_VifRefPktCur = g_VifRefPktCur + 1;
						uVar9 = *(undefined4*)((ulong)&gpPKTDataRefMasPath3[1].cmdA + 4);
						uVar10 = *(undefined4*)&gpPKTDataRefMasPath3[1].cmdB;
						uVar11 = *(undefined4*)((ulong)&gpPKTDataRefMasPath3[1].cmdB + 4);
						*(undefined4*)&g_VifRefPktCur->cmdA = *(undefined4*)&gpPKTDataRefMasPath3[1].cmdA;
						*(undefined4*)((ulong)&pRVar5[1].cmdA + 4) = uVar9;
						*(undefined4*)&pRVar5[1].cmdB = uVar10;
						*(undefined4*)((ulong)&pRVar5[1].cmdB + 4) = uVar11;
						pRVar5 = g_VifRefPktCur;
						g_VifRefPktCur = g_VifRefPktCur + 1;
						uVar9 = *(undefined4*)((ulong)&gpPKTDataRefMasPath3[2].cmdA + 4);
						uVar10 = *(undefined4*)&gpPKTDataRefMasPath3[2].cmdB;
						uVar11 = *(undefined4*)((ulong)&gpPKTDataRefMasPath3[2].cmdB + 4);
						*(undefined4*)&g_VifRefPktCur->cmdA = *(undefined4*)&gpPKTDataRefMasPath3[2].cmdA;
						*(undefined4*)((ulong)&pRVar5[1].cmdA + 4) = uVar9;
						*(undefined4*)&pRVar5[1].cmdB = uVar10;
						*(undefined4*)((ulong)&pRVar5[1].cmdB + 4) = uVar11;
						pRVar5 = g_VifRefPktCur;
						g_VifRefPktCur = g_VifRefPktCur + 1;
						uVar9 = *(undefined4*)((ulong)&gpPKTDataRefMasPath3[3].cmdA + 4);
						uVar10 = *(undefined4*)&gpPKTDataRefMasPath3[3].cmdB;
						uVar11 = *(undefined4*)((ulong)&gpPKTDataRefMasPath3[3].cmdB + 4);
						*(undefined4*)&g_VifRefPktCur->cmdA = *(undefined4*)&gpPKTDataRefMasPath3[3].cmdA;
						*(undefined4*)((ulong)&pRVar5[1].cmdA + 4) = uVar9;
						*(undefined4*)&pRVar5[1].cmdB = uVar10;
						*(undefined4*)((ulong)&pRVar5[1].cmdB + 4) = uVar11;
						g_VifRefPktCur = g_VifRefPktCur + 1;
						gpPKTDataRefMasPath3->cmdA =
							((long)(int)(gpPKTDataRefMasPath3 + 4) & 0xfffffffU) << 0x20 | 0x20000000;
						gpPKTDataRefMasPath3->cmdB = 0;
						g_VifRefPktCur->cmdA = 0x30000000;
						g_VifRefPktCur->cmdB = 0x1300000000000000;
						g_VifRefPktCur[1].cmdA = 0x30000000;
						g_VifRefPktCur[1].cmdB = 0x1100000011000000;
						g_VifRefPktCur[2].cmdA = 0x30000000;
						g_VifRefPktCur[2].cmdB = 0x600000000000000;
						g_VifRefPktCur[3].cmdA = 0xe10000001;
						g_VifRefPktCur[3].cmdB = 0;
						pRVar5 = g_VifRefPktCur;
						*(undefined4*)&g_VifRefPktCur[4].cmdA = in_zero_lo;
						*(undefined4*)((ulong)&pRVar5[4].cmdA + 4) = in_zero_hi;
						*(undefined4*)&pRVar5[4].cmdB = in_zero_udw;
						*(undefined4*)((ulong)&pRVar5[4].cmdB + 4) = in_register_0000000c;
						g_VifRefPktCur[5].cmdA = 0x30000000;
						g_VifRefPktCur[5].cmdB = 0x600800000000000;
						g_VifRefPktCur = g_VifRefPktCur + 6;
					}
				}
				pRVar5 = g_VifRefPktCur;
				pInStaticMeshMaster = gScene3D + staticMeshMasterIndex;

				RENDER_LOG("ed3DSceneRender Processing %p (%d)\n", pInStaticMeshMaster, staticMeshMasterIndex);

				if (gCurViewportUsed == (ed_viewport*)0x0) {
					gCurViewportUsed = pInStaticMeshMaster->pViewport;
				}
				pCVar1 = pInStaticMeshMaster->pViewport;
				gCurRectViewport.posX = pCVar1->posX;
				gCurRectViewport.posY = pCVar1->posY;
				gCurRectViewport.screenWidth = pCVar1->screenWidth;
				gCurRectViewport.screenHeight = pCVar1->screenHeight;
				gCurScene = pInStaticMeshMaster;
				ed3DSceneRenderOne(pInStaticMeshMaster, pInStaticMeshMaster);

				RENDER_LOG("ed3DSceneRender Begin Processing Children\n");
				for (pCVar12 = (edLIST*)(pInStaticMeshMaster->headerA).pLoadedData;
					pCVar12 != &pInStaticMeshMaster->headerA; pCVar12 = (edLIST*)pCVar12->pLoadedData) {
					if ((((ed_3D_Scene*)pCVar12->pCameraPanHeader_0xc)->flags_0x4 & 4) == 0) {
						ed3DSceneRenderOne((ed_3D_Scene*)pCVar12->pCameraPanHeader_0xc, pInStaticMeshMaster);
					}
				}
				RENDER_LOG("ed3DSceneRender End Processing Children\n");

				if (BYTE_00448a5c != 0) {
					ed3DSceneRenderDlist(pInStaticMeshMaster);
				}
				iVar4 = gIDProfileFlush;
				if (ged3DConfig.bEnableProfile != 0) {
					IMPLEMENTATION_GUARD(
					if ((gIDProfileFlush & 1U) == 0) {
						uVar6 = (uint)gIDProfileFlush >> 4;
						if (ProfileManagerB[uVar6].field_0x34 != 0) {
							ProfileManagerB[uVar6].field_0x8 = 0;
							ProfileManagerB[uVar6].field_0x20 = 0;
							ProfileManagerB[uVar6].field_0x34 = 0;
						}
						ProfileManagerB[(uint)iVar5 >> 4].field_0x0 = (long)PCR1 & 0xffffffff;
						ProfileManagerB[(uint)iVar5 >> 4].field_0x20 =
							ProfileManagerB[(uint)iVar5 >> 4].field_0x20 + 1;
					}
					else {
						uVar6 = (uint)gIDProfileFlush >> 4;
						if (g_ProfileObjA_0041ed40[uVar6].field_0x34 != 0) {
							g_ProfileObjA_0041ed40[uVar6].field_0x8 = 0;
							g_ProfileObjA_0041ed40[uVar6].field_0x20 = 0;
							g_ProfileObjA_0041ed40[uVar6].field_0x34 = 0;
						}
						g_ProfileObjA_0041ed40[(uint)iVar5 >> 4].field_0x0 = (long)PCR1 & 0xffffffff;
						g_ProfileObjA_0041ed40[(uint)iVar5 >> 4].field_0x20 =
							g_ProfileObjA_0041ed40[(uint)iVar5 >> 4].field_0x20 + 1;
					})
				}
				if (pRVar5 != g_VifRefPktCur) {
					if ((pInStaticMeshMaster->flags_0x4 & 0x40) != 0) {
						bVar3 = true;
						ed3DFlushFogFX();
					}
					if ((pInStaticMeshMaster->flags_0x4 & 0x800) == 0) {
						if ((pInStaticMeshMaster->flags_0x4 & 0x80) != 0) {
							bVar3 = true;
							g_VifRefPktCur = ed3DFlushAAEffect(g_VifRefPktCur);
						}
					}
					else {
						bVar3 = true;
						IMPLEMENTATION_GUARD();
						//g_VideoMemoryPtr_00449280 = (sceDmaTag*)FUN_002b8030((ulong*)g_VideoMemoryPtr_00449280);
					}
				}
				if (ged3DConfig.bEnableProfile != 0) {
					IMPLEMENTATION_GUARD(
					uVar6 = (uint)gIDProfileFlush >> 4;
					if ((gIDProfileFlush & 1U) == 0) {
						ProfileManagerB[uVar6].field_0x0 = (long)PCR1 - ProfileManagerB[uVar6].field_0x0;
						ProfileManagerB[uVar6].field_0x8 =
							ProfileManagerB[uVar6].field_0x8 + ProfileManagerB[uVar6].field_0x0;
						pPVar7 = ProfileManagerB + uVar6;
						if (pPVar7->field_0x10 < ProfileManagerB[uVar6].field_0x0) {
							pPVar7->field_0x10 = pPVar7->field_0x0;
						}
					}
					else {
						g_ProfileObjA_0041ed40[uVar6].field_0x0 = (long)PCR1 - g_ProfileObjA_0041ed40[uVar6].field_0x0;
						g_ProfileObjA_0041ed40[uVar6].field_0x8 =
							g_ProfileObjA_0041ed40[uVar6].field_0x8 + g_ProfileObjA_0041ed40[uVar6].field_0x0;
						pPVar7 = g_ProfileObjA_0041ed40 + uVar6;
						if (pPVar7->field_0x10 < g_ProfileObjA_0041ed40[uVar6].field_0x0) {
							pPVar7->field_0x10 = pPVar7->field_0x0;
						}
					})
				}
			}
		}
		if (BYTE_00449424 != 0) {
			IMPLEMENTATION_GUARD(g_VifRefPktCur = (sceDmaTag*)FUN_002aa4f0((ulong*)g_VifRefPktCur));
		}
		iVar4 = gIDProfileFlush;
		if (ged3DConfig.bEnableProfile != 0) {
			if ((gIDProfileFlush & 1U) == 0) {
				IMPLEMENTATION_GUARD(
				uVar6 = (uint)gIDProfileFlush >> 4;
				if (ProfileManagerB[uVar6].field_0x34 != 0) {
					ProfileManagerB[uVar6].field_0x8 = 0;
					ProfileManagerB[uVar6].field_0x20 = 0;
					ProfileManagerB[uVar6].field_0x34 = 0;
				}
				ProfileManagerB[(uint)iVar5 >> 4].field_0x0 = (long)PCR1 & 0xffffffff;
				ProfileManagerB[(uint)iVar5 >> 4].field_0x20 = ProfileManagerB[(uint)iVar5 >> 4].field_0x20 + 1;
				)
			}
			else {
				IMPLEMENTATION_GUARD();
				//uVar6 = (uint)gIDProfileFlush >> 4;
				//if (g_ProfileObjA_0041ed40[uVar6].field_0x34 != 0) {
				//	g_ProfileObjA_0041ed40[uVar6].field_0x8 = 0;
				//	g_ProfileObjA_0041ed40[uVar6].field_0x20 = 0;
				//	g_ProfileObjA_0041ed40[uVar6].field_0x34 = 0;
				//}
				//g_ProfileObjA_0041ed40[(uint)iVar5 >> 4].field_0x0 = (long)PCR1 & 0xffffffff;
				//g_ProfileObjA_0041ed40[(uint)iVar5 >> 4].field_0x20 = g_ProfileObjA_0041ed40[(uint)iVar5 >> 4].field_0x20 + 1;
			}
		}
		ed3DPrimlistTermMaterialRenderList();
		if ((g_pStartPktData != (edpkt_data*)0x0) && (g_pStartPktData != g_VifRefPktCur)) {
			g_VifRefPktCur = ed3DAddViewport2DAfter3D(g_VifRefPktCur);
			if (gBlurON != 0) {
				IMPLEMENTATION_GUARD();
				//ed3DFlushBlurEffect();
			}
			g_VifRefPktCur->cmdA = 0x30000000;
			g_VifRefPktCur->cmdB = 0;
			*(undefined4*)&g_VifRefPktCur->cmdB = 0x6000000;
			g_VifRefPktCur = g_VifRefPktCur + 1;
			g_VifRefPktCur->cmdA = 0xe10000002;
			g_VifRefPktCur->cmdB = 0;
			*(undefined4*)&g_VifRefPktCur->cmdB = 0;
			*(undefined4*)((ulong)&g_VifRefPktCur->cmdB + 4) = 0x50000002;
			g_VifRefPktCur = g_VifRefPktCur + 1;

			g_VifRefPktCur->cmdA = SCE_GIF_SET_TAG(
				1,					// NLOOP
				SCE_GS_TRUE,		// EOP
				SCE_GS_TRUE,		// PRE
				20,					// PRIM
				SCE_GIF_PACKED,		// FLG
				1					// NREG
			);
			g_VifRefPktCur->cmdB = SCE_GIF_PACKED_AD;

			g_VifRefPktCur = g_VifRefPktCur + 1;
			g_VifRefPktCur->cmdA = SCE_GS_SET_ALPHA_1(SCE_GS_ALPHA_ZERO, SCE_GS_ALPHA_ZERO, SCE_GS_ALPHA_ZERO, SCE_GS_ALPHA_CS, SCE_GS_ALPHA_AS);
			g_VifRefPktCur->cmdB = SCE_GS_ALPHA_1;

			g_VifRefPktCur = g_VifRefPktCur + 1;
			g_VifRefPktCur = ed3DDMAAddFinish(g_VifRefPktCur);

			g_VifRefPktCur->cmdB = 0x0;
			sceDmaAddEnd((sceDmaTag**)&g_VifRefPktCur, 0, 0);
			if (gpPKTDataRefMasPath3 != (edpkt_data*)0x0) {
				gpPKTDataRefMasPath3->cmdA =
					((ulong)(int)(gpPKTDataRefMasPath3 + 4) & 0xfffffffU) << 0x20 | 0x20000000;
				gpPKTDataRefMasPath3->cmdB = 0;
			}
		}
		if (ged3DConfig.bEnableProfile != 0) {
			IMPLEMENTATION_GUARD();
			uVar7 = (uint)gIDProfileFlush >> 4;
			//if ((gIDProfileFlush & 1U) == 0) {
			//	ProfileManagerB[uVar6].field_0x0 = (long)PCR1 - ProfileManagerB[uVar6].field_0x0;
			//	ProfileManagerB[uVar6].field_0x8 =
			//		ProfileManagerB[uVar6].field_0x8 + ProfileManagerB[uVar6].field_0x0;
			//	pPVar7 = ProfileManagerB + uVar6;
			//	if (pPVar7->field_0x10 < ProfileManagerB[uVar6].field_0x0) {
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
	

edNODE_MANAGER* edListCreateNodesTable(int count, EHeap heapID)
{
	edNODE_MANAGER* pAllocatedBuffer;

	pAllocatedBuffer = (edNODE_MANAGER*)edMemAllocAlignBoundary(heapID, count * 0x10 + 0x10);
	if (pAllocatedBuffer == (edNODE_MANAGER*)0x0) {
		pAllocatedBuffer = (edNODE_MANAGER*)0x0;
	}
	else {
		Setup_00290b70(pAllocatedBuffer, count);
	}
	return pAllocatedBuffer;
}

void ed3DShadowInitPrimList(edNODE_MANAGER** aHeaders)
{
	int iVar1;

	iVar1 = 0;
	do {
		edListLink(gPrim_List_Shadow + iVar1, aHeaders[iVar1], 0, 0);
		edListClear(gPrim_List_Shadow + iVar1);
		iVar1 = iVar1 + 1;
	} while (iVar1 < 2);
	return;
}

void ed3DPrimListInit(int materialBufferCount, int countB)
{
	int iVar1;
	int iVar2;

	pNodeManRender = edListCreateNodesTable(countB, TO_HEAP(H_MAIN));
	pNodeManMaterial = edListCreateNodesTable(materialBufferCount, TO_HEAP(H_MAIN));
	PTR_MeshTransformParentHeader_004494b4 = edListCreateNodesTable(materialBufferCount, TO_HEAP(H_MAIN));
	iVar2 = 0;
	do {
		edListLink
		((edLIST*)gPrim_List[iVar2], pNodeManMaterial,
			0, 0);
		edListClear((edLIST*)gPrim_List[iVar2]);
		edListLink
		(gPrim_List[iVar2] + 1, PTR_MeshTransformParentHeader_004494b4, 0, 0);
		edListClear(gPrim_List[iVar2] + 1);
		iVar2 = iVar2 + 1;
	} while (iVar2 < 0xf);
	ed3DShadowInitPrimList(&pNodeManMaterial);
	gCurRenderList = 0;
	gCurFlushList = 0;
	iVar2 = 0;
	do {
		iVar1 = iVar2 + 1;
		gPrim_List_FlushTex[iVar2].pLoadedData = (MeshTransformSpecial*)(gPrim_List_FlushTex + iVar2);
		gPrim_List_FlushTex[iVar2].count_0x14 = 0;
		iVar2 = iVar1;
	} while (iVar1 < 2);
	return;
}

edLIST* gManager3D = NULL;
edLIST* CameraPanMasterHeaderB_004491d4 = NULL;

char* g_szed3DAllocInfo_004333d0 = "ed3D Init total allocation ammount:<< %d >>\n";

byte BYTE_00449344 = 0;

void ed3DFlushTexInit(void)
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


void ed3DHierarchyManagerInit(void)
{
	gHierarchyManagerBuffer = (MeshTransformData*)edMemAllocAlignBoundary(TO_HEAP(H_MAIN), ged3DConfig.meshTransformDataCount * sizeof(MeshTransformData));
	gHierarchyManagerNodeManager =
		edListCreateNodesTable(ged3DConfig.meshTransformDataCount, TO_HEAP(H_MAIN));
	gHierarchyManagerFirstFreeNode = (MeshTransformParent*)gHierarchyManagerNodeManager->pNextFree;
	gScene_list_buffer = edMemAllocAlignBoundary(TO_HEAP(H_MAIN), ged3DConfig.sceneCount * sizeof(ed3D_60));
	return;
}

void ed3DListBufferInit(int materialBufferCount, int matrixBufferCount)
{
	ed_dma_matrix* pRVar1;
	ed_dma_matrix** ppRVar2;
	ed_dma_matrix** ppRVar3;
	uint uVar4;

	uVar4 = 0;
	do {
		pRVar1 = (ed_dma_matrix*)edMemAllocAlignBoundary(TO_HEAP(H_MAIN), materialBufferCount * sizeof(ed_dma_matrix));
		DmaMaterialBuffer[uVar4] = pRVar1;
		DmaMaterialBufferMax[uVar4] = pRVar1 + materialBufferCount;
		uVar4 = uVar4 + 1 & 0xffff;
	} while (uVar4 < 2);
	DmaMatrixBuffer = (ed_dma_matrix*)edMemAllocAlignBoundary(TO_HEAP(H_MAIN), matrixBufferCount * sizeof(ed_dma_matrix));
	DmaMaterialBufferCurrent = DmaMaterialBuffer[0];
	DmaMaterialBufferCurrentMax = DmaMaterialBufferMax[0];
	DmaMatrixBufferMax = DmaMatrixBuffer + matrixBufferCount;
	DmaMatrixBufferCurrent = DmaMatrixBuffer;
	DmaMatrixBufferCurrentMax = DmaMatrixBuffer + matrixBufferCount;
	return;
}

ed_g3d_manager* gpG3D = NULL;
DisplayListInternalMesh* g_MeshDisplayListInternal_00449380 = NULL;

void ed3DPrepareMaterial(ed_g2d_material* pBuffer, bool param_2)
{
	int iVar1;
	uint uVar2;
	undefined* puVar3;
	bool bVar4;
	undefined4* puVar5;
	ulong* puVar6;
	undefined8 uVar7;
	int puVar8;
	undefined* puVar9;
	edpkt_data* pRVar10;
	uint uVar11;
	TextureData_LAY_Internal* pLAY;
	ushort* puVar12;
	uint uVar13;
	int iVar14;
	float fVar15;
	uint local_40;
	undefined4 local_30;
	undefined4 local_2c;
	undefined4 local_28;
	uint local_20;
	uint local_1c;
	int local_18;
	undefined local_10[8];
	uint local_8;
	uint local_4;

	puVar8 = 0x10;
	iVar14 = 0;
	puVar12 = (ushort*)0x0;
	puVar9 = local_10;
	puVar3 = puVar9;
	while (pRVar10 = g_pStrippBufLastPos, puVar3 != (undefined*)0x0) {
		*puVar9 = 0;
		puVar9 = puVar9 + 1;
		puVar8 = puVar8 + -1;
		puVar3 = (undefined*)puVar8;
	}
	for (; ((uint)pRVar10 & 0xf) != 0; pRVar10 = pRVar10 + 1) {
	}
	pBuffer->pCommandBufferTexture = (int)STORE_SECTION(pRVar10);
	pBuffer->commandBufferTextureSize = 0;
	uVar13 = 0;
	do {
		if (pBuffer->count_0x0 == 0) {
			if (param_2 == true) {
				pRVar10->cmdA = 0xe10000001;
				pRVar10->cmdB = 0;
				*(undefined4*)&pRVar10->cmdB = 0;
				*(undefined4*)((ulong)&pRVar10->cmdB + 4) = 0x40003dc;
				*(undefined4*)&pRVar10[1].cmdA = 0;
				*(undefined4*)((ulong)&pRVar10[1].cmdA + 4) = 0x14000000;
				pRVar10[1].cmdB = 0;
				pBuffer->commandBufferTextureSize = 2;
				pRVar10 = pRVar10 + 2;
			}
			else {
				pRVar10->cmdA = 0xe10000000;
				pRVar10->cmdB = 0;
				pBuffer->commandBufferTextureSize = 1;
				pRVar10 = pRVar10 + 1;
			}
		}
		else {
			local_20 = gVRAMClutBuffers[uVar13];
			local_1c = gVRAMBuffers[uVar13];
			local_18 = local_40 + local_1c;
			for (uVar11 = 0; uVar11 < pBuffer->count_0x0; uVar11 = uVar11 + 1) {
				IMPLEMENTATION_GUARD();
				iVar1 = *(int*)(&pBuffer[1].count_0x0 + uVar11 * 4);
				pLAY = (TextureData_LAY_Internal*)(iVar1 + 0x10);
				if ((pLAY != (TextureData_LAY_Internal*)0x0) && (*(short*)(iVar1 + 0x2c) != 0)) {
					iVar14 = *(int*)(iVar1 + 0x30) + 0x10;
				}
				if (iVar14 == 0) {
					if (param_2 == true) {
						pRVar10->cmdA = 0xe10000001;
						pRVar10->cmdB = 0;
						*(undefined4*)&pRVar10->cmdB = 0;
						*(undefined4*)((ulong)&pRVar10->cmdB + 4) = 0x40003dc;
						*(undefined4*)&pRVar10[1].cmdA = 0;
						*(undefined4*)((ulong)&pRVar10[1].cmdA + 4) = 0x14000000;
						pRVar10[1].cmdB = 0;
						pBuffer->commandBufferTextureSize = 2;
						pRVar10 = pRVar10 + 2;
					}
					else {
						pRVar10->cmdA = 0xe10000000;
						pRVar10->cmdB = 0;
						pBuffer->commandBufferTextureSize = 1;
						pRVar10 = pRVar10 + 1;
					}
				}
				else {
					if (*(int*)(iVar14 + 8) != 0) {
						puVar12 = (ushort*)(*(int*)(*(int*)(iVar14 + 8) + 8) + 0x10);
					}
					if (puVar12 == (ushort*)0x0) {
						if (param_2 == true) {
							pRVar10->cmdA = 0xe10000001;
							pRVar10->cmdB = 0;
							*(undefined4*)&pRVar10->cmdB = 0x11000000;
							*(undefined4*)((ulong)&pRVar10->cmdB + 4) = 0x40003dc;
							*(undefined4*)&pRVar10[1].cmdA = 0;
							*(undefined4*)((ulong)&pRVar10[1].cmdA + 4) = 0x14000000;
							pRVar10[1].cmdB = 0x11000000;
							pBuffer->commandBufferTextureSize = 2;
							pRVar10 = pRVar10 + 2;
						}
						else {
							pRVar10->cmdA = 0xe10000000;
							pRVar10->cmdB = 0;
							pBuffer->commandBufferTextureSize = 1;
							pRVar10 = pRVar10 + 1;
						}
					}
					else {
						IMPLEMENTATION_GUARD(
						local_8 = (uint)*puVar12;
						fVar15 = (float)(uint)puVar12[1];
						if (fVar15 < 2.147484e+09) {
							local_4 = (uint)fVar15;
						}
						else {
							local_4 = (int)(fVar15 - 2.147484e+09) | 0x80000000;
						}
						if (puVar12[2] == 0x18) {
							local_40 = local_8 * local_4 * 4 >> 8;
						}
						else {
							local_40 = (uint)puVar12[2] * local_8 * local_4 >> 0xb;
						}
						uVar2 = DAT_00449350;
						if (pLAY != (TextureData_LAY_Internal*)0x0) {
							uVar2 = DAT_00449350 | pLAY->field_0x0;
							pLAY->field_0x0 = uVar2;
						}
						if (param_2 == true) {
							pRVar10->cmdA = 0xe1000000b;
							pRVar10->cmdB = 0;
							*(undefined4*)&pRVar10->cmdB = 0x40003dc;
							*(undefined4*)((int)&pRVar10->cmdB + 4) = 0x6c0a03dc;
						}
						else {
							pRVar10->cmdA = 0xe1000000a;
							pRVar10->cmdB = 0;
						}
						pRVar10[1].cmdA = 0x1000000000008009;
						pRVar10[1].cmdB = 0xe;
						bVar4 = ed3DFXIsTrue(pLAY);
						if (bVar4 == false) {
							uVar7 = ed3DSetBasicTexPacket(pRVar10 + 2, &local_20, puVar12, pLAY);
						}
						else {
							puVar5 = (undefined4*)ed3DGetFxConfigFromFlag(uVar2);
							local_30 = puVar5[1];
							local_2c = *puVar5;
							local_28 = *puVar5;
							uVar7 = ed3DSetBasicTexPacket(pRVar10 + 2, &local_30, puVar12, pLAY);
						}
						puVar6 = (ulong*)ed3DSetAlphaPacket(uVar7, (uVar2 & 0x2000) != 0, pLAY, uVar11);
						*puVar6 = (long)(int)gMipmapK << 0x20 |
							(long)(int)(puVar12[3] - 1) << 2 | 0x60U | (ulong)gMipmapL << 0x13;
						puVar6[1] = 0x14;
						puVar6 = ed3DSetMipmapPacket(puVar6 + 2, (int)&local_20, puVar12);
						pRVar10 = (edpkt_data*)(puVar6 + 2);
						*puVar6 = (ulong)((pLAY->field_0x0 & 1) == 0) | (ulong)((pLAY->field_0x0 & 2) == 0) << 2;
						puVar6[1] = 8;
						if (param_2 == true) {
							*(undefined4*)&pRVar10->cmdA = 0;
							*(undefined4*)((int)puVar6 + 0x14) = 0x14000000;
							*(undefined4*)(puVar6 + 3) = 0;
							*(undefined4*)((int)puVar6 + 0x1c) = 0;
							pBuffer->commandBufferTextureSize = 0xc;
							pRVar10 = (edpkt_data*)(puVar6 + 4);
						}
						else {
							pBuffer->commandBufferTextureSize = 0xb;
						}
						local_20 = local_20 + 4;
						local_1c = local_1c + 0x100;
						local_18 = local_18 + 0x100;)
					}
				}
			}
		}
		uVar13 = uVar13 + 1;
	} while (uVar13 < 2);
	g_pStrippBufLastPos = pRVar10;
	return;
}

int gLibInitialized = 0;
uint gZbuf_ScreenZMax = 0;

void ed3DInit(void)
{
	ed_g2d_manager* pTVar1;
	uint uVar2;
	int size;

	edDebugPrintf(s_ed3D_Initialsation_004333a0);
	ed3DPrepareVRAMBuf();
	gLibInitialized = 1;
	gManager3D = edListNew(1, ged3DConfig.g3dManagerCount, TO_HEAP(H_MAIN));
	CameraPanMasterHeaderB_004491d4 = edListNew(1, ged3DConfig.g2dManagerCount, TO_HEAP(H_MAIN));
	gpG3D = (ed_g3d_manager*)edMemAllocAlignBoundary(TO_HEAP(H_MAIN), ged3DConfig.g3dManagerCount * sizeof(ed_g3d_manager));
	memset(gpG3D, 0, ged3DConfig.g3dManagerCount * sizeof(ed_g3d_manager));
	gpG2D = (ed_g2d_manager*)edMemAllocAlignBoundary(TO_HEAP(H_MAIN), ged3DConfig.g2dManagerCount * sizeof(ed_g2d_manager));
	memset(gpG2D, 0, ged3DConfig.g2dManagerCount * sizeof(ed_g2d_manager));
	//pTVar1 = TexturePool_004491cc;
	gNbG3D = 0;
	gNbG2D = 0;
	//*(undefined8*)TexturePool_004491cc = 0x3fff00000000;
	//*(undefined8*)&pTVar1->textureHeaderStart = 0x50;
	//*(undefined8*)&pTVar1->textureBufferStart = 0x7ffe000000000;
	memset(&ged3DFXConfig, 0, sizeof(ed3DFXConfig) * 16);
	gScene3D = (ed_3D_Scene*)edMemAllocAlignBoundary(TO_HEAP(H_MAIN), ged3DConfig.sceneCount * sizeof(ed_3D_Scene) + 1);
	memset(gScene3D, 0, ged3DConfig.sceneCount * sizeof(ed_3D_Scene));
	gNodeCluster = edListCreateNodesTable(ged3DConfig.meshDisplayListInternalCount + ged3DConfig.sceneCount * 3, TO_HEAP(H_MAIN));
	g_MeshDisplayListInternal_00449380 = (DisplayListInternalMesh*)edMemAllocAlignBoundary(TO_HEAP(H_MAIN), ged3DConfig.meshDisplayListInternalCount * sizeof(DisplayListInternalMesh));
	memset(g_MeshDisplayListInternal_00449380, 0, ged3DConfig.meshDisplayListInternalCount * sizeof(DisplayListInternalMesh));
	for (uVar2 = 0; uVar2 < ged3DConfig.sceneCount; uVar2 = uVar2 + 1) {
		edListLink(&gScene3D[uVar2].headerB, gNodeCluster, 0, 0);
		edListLink(&gScene3D[uVar2].headerA, gNodeCluster, 0, 0);
	}
	/* Add render handler */
	edSysHandlersAdd
	(edSysHandlerVideo_0048cee0.nodeParent, edSysHandlerVideo_0048cee0.entries,
		edSysHandlerVideo_0048cee0.maxEventID, ESHT_RenderScene, ed3DSceneRender, 1, 1);
	edSysHandlersAdd
	(edSysHandlerVideo_0048cee0.nodeParent, edSysHandlerVideo_0048cee0.entries,
		edSysHandlerVideo_0048cee0.maxEventID, ESHT_Sound, ed3DSceneCompute, 1, 0);
	if (ged3DConfig.meshHeaderCountBAlt < 0x180) {
		ged3DConfig.meshHeaderCountBAlt = ged3DConfig.meshHeaderCountBAlt + 0x180;
	}
	g_pStripBuf = (edpkt_data*)edMemAllocAlignBoundary(TO_HEAP(H_MAIN), ged3DConfig.meshHeaderCountBAlt);
	g_pStrippBufMaxPos = (edpkt_data*)(((char*)g_pStripBuf) + ged3DConfig.meshHeaderCountBAlt);
	size = (ged3DConfig.field_0x28 * 2 + ged3DConfig.field_0x18 + 0x8c0 + ged3DConfig.matrixBufferCount * 0x38 + ged3DConfig.field_0x34 * 8) * 0x10;
	g_pStrippBufLastPos = g_pStripBuf;
	ed3D_Allocator_00449248.base = (char*)edMemAllocAlignBoundary(TO_HEAP(H_MAIN), size);
	memset(ed3D_Allocator_00449248.base, 0, size);
	ed3D_Allocator_00449248.current = ed3D_Allocator_00449248.base + size;
	ed3D_Allocator_00449248.end = ed3D_Allocator_00449248.base;
	ed3D_SubAllocator_004491b8.base = SubAllocate_0029ce60(16000);
	ed3D_SubAllocator_004491b8.end = ed3D_SubAllocator_004491b8.base + 16000;
	ed3D_SubAllocator_004491b8.current = ed3D_SubAllocator_004491b8.base;
	ed3DListBufferInit(ged3DConfig.materialBufferCount, ged3DConfig.matrixBufferCount);
	//ed3DClusterInit();
	ed3DPrimListInit(ged3DConfig.materialBufferCount, ged3DConfig.meshHeaderCountB);
	ed3DDMAInit(ged3DConfig.field_0x18 + ged3DConfig.field_0x34 * 8, ged3DConfig.field_0x28);
	//ed3DFlushInit();
	ed3DFlushTexInit();
	//if (DAT_004491f8 == '\0') {
	//	DAT_004491f8 = '\x01';
	//}
	gCurViewportUsed = (ed_viewport*)0x0;
	ed3DVU1BufferInit();
	ed3DFlushListInit();
	ed3DPrimlistInitMaterialRenderList();
	ed3DPrimlistMatrixBufferReset();
	ed3DShadowResetPrimList();
	ed3DHierarchyManagerInit();
	ed3DPrepareMaterial(ed_g2d_material_ARRAY_0048c370 + 1, true);
	ed3DPrepareMaterial(ed_g2d_material_ARRAY_0048c370, true);
	ed3DPrepareMaterial(ed_g2d_material_ARRAY_0048c370 + 3, true);
	ed3DPrepareMaterial(ed_g2d_material_ARRAY_0048c370 + 2, true);
	ed3DPrepareMaterial(ed_g2d_material_ARRAY_0048c370 + 5, true);
	ed3DPrepareMaterial(ed_g2d_material_ARRAY_0048c370 + 4, true);
	gZbuf_ScreenZMax = 0xffffffff;
	g_pStripBufResetPos = g_pStrippBufLastPos;;
	if (ged3DConfig.bEnableProfile == 1) {
		IMPLEMENTATION_GUARD();
	//	gIDProfileRender = edProfileNew(1, 0, 0, 0x80, s_3DRend_004333b8);
	//	gIDProfileFlush = edProfileNew(1, 0x80, 0, 0, s_3DFlush_004333c0);
	}
	edDebugPrintf(g_szed3DAllocInfo_004333d0,
		ged3DConfig.matrixBufferCount * 0x220 +
		ged3DConfig.field_0x18 * 0x10 +
		ged3DConfig.meshHeaderCountBAlt +
		ged3DConfig.g3dManagerCount * 0x40 + ged3DConfig.g2dManagerCount * 0x30 +
		ged3DConfig.materialBufferCount * 0x60);
	return;
}

ed3DConfig* ed3DGetConfig(void)
{
	return &ged3DConfig;
}

void ed3DSetFxConfig(int index, undefined4 param_2, undefined4 param_3, uint param_4, undefined4 param_5)
{
	int iVar1;

	iVar1 = index + -1;
	if (ged3DFXConfig + iVar1 != (ed3DFXConfig*)0x0) {
		ged3DFXConfig[iVar1].field_0x0 = param_2;
		ged3DFXConfig[iVar1].field_0x4 = param_3;
		ged3DFXConfig[iVar1].field_0x8 = param_4 | 4;
		ged3DFXConfig[iVar1].field_0xc = 0;
		ged3DFXConfig[iVar1].field_0x10 = param_5;
	}
	return;
}

void ed3DSetFXAnimSTMaxDist(float animSTMaxDist)
{
	gAnimSTMaxDist = animSTMaxDist;
	return;
}

struct ed3DLodSetup {
	undefined4 field_0x0;
	float field_0x4;
	float field_0x8;
	float field_0xc;
} gLodNfo;

ed3DLodSetup* ed3DGetLodSetup(void)
{
	return &gLodNfo;
}

void Init3D(void)
{
	ed3DConfig* puVar1;
	ed3DLodSetup* puVar2;

	puVar1 = ed3DGetConfig();
	puVar1->meshHeaderCountB = 2000;
	puVar1->field_0x18 = 55000;
	puVar1->materialBufferCount = 500;
	puVar1->g3dManagerCount = 0x1e;
	puVar1->g2dManagerCount = 0x6e;
	puVar1->meshHeaderCountBAlt = 0x180;
	puVar1->sceneCount = 0xf;
	puVar1->field_0x28 = 0x15e;
	puVar1->matrixBufferCount = 0x15e;
	puVar1->field_0x34 = 0;
	puVar1->meshTransformDataCount = 0x708;
	puVar1->field_0x2d = 0;
	puVar1->bEnableProfile = 0;
	puVar1->field_0x2c = 1;
	ed3DInit();
	ed3DSetFxConfig(2, 0x800, 0, 1, 0);
	ed3DSetFXAnimSTMaxDist(1000.0);
	puVar2 = ed3DGetLodSetup();
	puVar2->field_0x0 = 0;
	puVar2->field_0x4 = 12.25;
	puVar2->field_0x8 = 225.0;
	ed3DSetMipmapProp(true, 0, 0xffffff01);
	return;
}

void ed3DPrepareG2DManageStruct(ed_g2d_manager* textureInfoObj, char* fileBuffer, int length)
{
	int* piVar1;
	int* nextSection;
	int readValue;

	/* seek through the buffer */
	for (piVar1 = (int*)edChunckGetFirst(fileBuffer, fileBuffer + length); piVar1 != (int*)0x0; piVar1 = edChunckGetNext((char*)piVar1, (int*)(fileBuffer + length))) {
		readValue = *piVar1;
		/* Check the first value in the buffer is *2D* */
		if (readValue == 0x2a44322a) {
			/* Set the readValue to be the last section of the texture header */
			readValue = piVar1[2];
			textureInfoObj->textureHeaderStart = (char*)piVar1;
			for (nextSection = (int*)edChunckGetFirst((char*)(piVar1 + 4), (char*)(int*)((char*)piVar1 + readValue)); nextSection != (int*)0x0;
				nextSection = edChunckGetNext((char*)nextSection, (int*)((char*)piVar1 + readValue))) {
				/* Check if the value in the buffer is 'MATA' */
				if (*nextSection == 0x4154414d) {
					textureInfoObj->pMAT_HASH = (char*)(nextSection + 4);
				}
			}
		}
		else {
			/* Check if the value in the buffer is 'ANMA' */
			if (readValue == 0x414d4e41) {
				textureInfoObj->pANMA = (char*)piVar1;
			}
			else {
				/* Check if the value in the buffer is 'PALL' */
				if (readValue == 0x4c4c4150) {
					textureInfoObj->pPALL = (char*)piVar1;
				}
				else {
					/* Check if the value in the buffer is 'T2DA' */
					if (readValue == 0x41443254) {
						textureInfoObj->pT2DA = (char*)piVar1;
					}
				}
			}
		}
	}
	return;
}

int* ed3DPreparePointer(char* fileBuffer, int length, char* fileBufferCopy, int lengthCopy)
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
	nextSectionBuffer = (int*)edChunckGetFirst(fileBuffer, fileBuffer + length);
	while (true) {
		if (nextSectionBuffer == (int*)0x0) {
			return (int*)0x0;
		}
		/* Check if the value in the buffer is 'REAA'
			If it is, and the next value isn't 1974, exit the loop (we reached the end of the file) */
		if ((*nextSectionBuffer == 0x41414552) && (*(short*)(nextSectionBuffer + 1) != 0x7b6)) break;
		nextSectionBuffer = edChunckGetNext((char*)nextSectionBuffer, (int*)(fileBuffer + length));
	}
	/* Jump ahead in the buffer */
	iVar1 = nextSectionBuffer[2];
	/* Mark the REAA buffer with 1974 */
	*(undefined2*)(nextSectionBuffer + 1) = 0x7b6;
	piVar2 = (int*)edChunckGetFirst((char*)(nextSectionBuffer + 4), (char*)(int*)((char*)nextSectionBuffer + iVar1));
	pcVar5 = actualFileStart + (int)pcVar5;
	for (; piVar2 != (int*)0x0; piVar2 = edChunckGetNext((char*)piVar2, (int*)((char*)nextSectionBuffer + iVar1))) {
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

void ed3DPreparedAllMaterial(ed_g2d_manager* textureInfoObj, ulong mode)
{
	char* pcVar1;
	int iVar2;
	int iVar3;
	int* piVar4;
	int* piVar5;

	pcVar1 = textureInfoObj->textureHeaderStart;
	iVar2 = *(int*)(pcVar1 + 8);
	for (piVar4 = (int*)edChunckGetFirst(pcVar1 + 0x10, pcVar1 + iVar2); piVar4 != (int*)0x0; piVar4 = edChunckGetNext((char*)piVar4, (int*)(pcVar1 + iVar2))) {
		if (*piVar4 == 0x4154414d) {
			iVar3 = piVar4[2];
			for (piVar5 = (int*)edChunckGetFirst((char*)(piVar4 + 4), (char*)(int*)((int)piVar4 + iVar3)); piVar5 != (int*)0x0;
				piVar5 = edChunckGetNext((char*)piVar5, (int*)((int)piVar4 + iVar3))) {
				/* .TAM */
				if (*piVar5 == 0x2e54414d) {
#ifdef PLATFORM_WIN
					IMPLEMENTATION_GUARD(); // untested
#endif
					//ed3DPrepareMaterial((TamPacked*)(piVar5 + 4), mode);
				}
			}
		}
	}
	return;
}

void ed3DPrepareG2D(ed_g2d_manager* textureInfoObj, ulong mode)
{
	int iVar1;
	edpkt_data* puVar2;
	LayerHeaderPacked* pBuffer;
	char* fileBufferStart;
	char* texHeaderStart;

	/* Can point at 004DA040 */
	puVar2 = g_pStrippBufLastPos;
	texHeaderStart = textureInfoObj->textureHeaderStart;
	iVar1 = *(int*)(texHeaderStart + 8);
	if ((textureInfoObj != (ed_g2d_manager*)0x0) && (fileBufferStart = textureInfoObj->textureFileBufferStart, (*(uint*)(fileBufferStart + 4) & 1) == 0)) {
		IMPLEMENTATION_GUARD();
		for (pBuffer = (LayerHeaderPacked*)edChunckGetFirst(texHeaderStart + 0x10, texHeaderStart + iVar1); pBuffer != (LayerHeaderPacked*)0x0;
			pBuffer = (LayerHeaderPacked*)edChunckGetNext((char*)pBuffer, (int*)(texHeaderStart + iVar1))) {
			/* Check if the value in the buffer is 'LAYA' */
			if (*(int*)pBuffer == 0x4159414c) {
				//ed3DPrepareLayer(pBuffer, textureInfoObj);
			}
		}
		ed3DPreparedAllMaterial(textureInfoObj, mode);
		textureInfoObj->textureFileLengthB = (ulong)g_pStrippBufLastPos + (textureInfoObj->textureFileLengthB - (ulong)puVar2);
		*(uint*)(fileBufferStart + 4) = *(uint*)(fileBufferStart + 4) | 1;
	}
	return;
}

#ifdef PLATFORM_WIN
Multidelegate<ed_g2d_manager*> onTextureLoadedDelegate;

Multidelegate<ed_g2d_manager*>& ed3DGetTextureLoadedDelegate()
{
	return onTextureLoadedDelegate;
}

Multidelegate<edDList_material*> onMaterialLoadedDelegate;
Multidelegate<edDList_material*> onMaterialUnloadedDelegate;

Multidelegate<edDList_material*>& ed3DGetMaterialLoadedDelegate()
{
	return onMaterialLoadedDelegate;
}

Multidelegate<edDList_material*>& ed3DGetMaterialUnloadedDelegate()
{
	return onMaterialUnloadedDelegate;
}
#endif

ed_g2d_manager* ed3DInstallG2D(char* fileBufferStart, int fileLength, int* outInt, ed_g2d_manager* pTextureInfo, ulong param_5)
{
	bool bVar1;
	ed_g2d_manager* pOutTexture;

	char* fileBuffer;
	ed_g2d_manager* pTexturePool;

	ED3D_LOG(LogLevel::Info, "ed3DInstallG2D");

	/* Find some free area in the buffer? */
	pOutTexture = pTextureInfo;
	pTexturePool = gpG2D;
	if (pTextureInfo == (ed_g2d_manager*)0x0) {
		while (pOutTexture = pTexturePool, pOutTexture->textureFileBufferStart != (char*)0x0) {
			pTexturePool = pOutTexture + 1;
		}
	}
	if ((pOutTexture == (ed_g2d_manager*)0x0) || (memset(pOutTexture, 0, 0x30), fileBufferStart == (char*)0x0)) {
		bVar1 = pOutTexture != pTextureInfo;
		pOutTexture = (ed_g2d_manager*)0x0;
		if (bVar1) {
			gNbG2D = gNbG2D + -1;
			pOutTexture = (ed_g2d_manager*)0x0;
		}
	}
	else {
		/* Offset the file pointer by 0x10 */
		fileBuffer = fileBufferStart + 0x10;
		ed3DPrepareG2DManageStruct(pOutTexture, fileBuffer, fileLength + -0x10);
		ed3DPreparePointer(fileBuffer, fileLength + -0x10, fileBuffer, fileLength + -0x10);
		pOutTexture->textureFileBufferStart = fileBufferStart;
		pOutTexture->textureFileLengthA = fileLength;
		pOutTexture->textureFileLengthB = fileLength;
		*outInt = fileLength - pOutTexture->textureFileLengthB;
		ed3DPrepareG2D(pOutTexture, param_5);
	}

#ifdef PLATFORM_WIN
	onTextureLoadedDelegate(pOutTexture);
#endif

	return pOutTexture;
}

TextureData_HASH_Internal_MAT* ed3DG2DGetMaterialFromIndex(ed_g2d_manager* pTextureInfo, int index)
{
	int* pBuffStart;
	int* nextSection;
	uint uVar1;
	TextureData_HASH_Internal_MAT* pHASH_MAT;
	int* sectionEnd;

	TextureData_MATA* pMATA = (TextureData_MATA*)(((char*)pTextureInfo->pMAT_HASH) - 0x10);
	pBuffStart = (int*)pTextureInfo->pMAT_HASH;
	/* Ensure we are trying to read something from the 'MATA' section */
	if (pMATA->header.number == 0x4154414d) {
		/* Work out the section end address */
		sectionEnd = (int*)(((char*)pMATA) + pMATA->size_0x8);
		for (nextSection = (int*)edChunckGetFirst((char*)pBuffStart, (char*)sectionEnd); nextSection != (int*)0x0; nextSection = edChunckGetNext((char*)nextSection, sectionEnd)) {
			/* Check the first value in the buffer is *MAT.* */
			if (*nextSection == 0x2e54414d) {
				if ((index == 0) && (*pBuffStart == 0x48534148)) {
					/* Check the first value in the buffer is *HASH* */
					pHASH_MAT = (TextureData_HASH_Internal_MAT*)(pBuffStart + 4);
					uVar1 = (uint)((ulong)(pMATA) + (pMATA->field_0xc - (ulong)pHASH_MAT)) >> 4;
					for (; uVar1 != 0; uVar1 = uVar1 - 1) {
						if ((int*)LOAD_SECTION(pHASH_MAT->pMAT) == nextSection) {
							return pHASH_MAT;
						}
						pHASH_MAT = pHASH_MAT + 1;
					}
				}
				index = index + -1;
			}
			/* Jump to the next section */
		}
	}
	return (TextureData_HASH_Internal_MAT*)0x0;
}

ed_g2d_material* ed3DG2DGetG2DMaterialFromIndex(ed_g2d_manager* pTextureInfo, int index)
{
	TextureData_HASH_Internal_MAT* pMAT_HASH;
	ed_g2d_material* pMAT_Internal;

	pMAT_HASH = ed3DG2DGetMaterialFromIndex(pTextureInfo, index);
	if (pMAT_HASH == (TextureData_HASH_Internal_MAT*)0x0) {
		pMAT_Internal = (ed_g2d_material*)0x0;
	}
	else {
		pMAT_Internal = &((TextureData_MAT*)LOAD_SECTION(pMAT_HASH->pMAT))->body;
	}
	return pMAT_Internal;
}

char* ed3DG2DGetBitmapFromMaterial(ed_g2d_material* pMAT_Internal, int index)
{
	char* pcVar1;
	TextureData_TEX_Internal* iVar2;
	ed_g2d_layer* pLayerSection;

	iVar2 = (TextureData_TEX_Internal*)0x0;
	ed_g2d_material_After* pAfter = &((ed_g2d_material_After*)(pMAT_Internal + 1))[index];
	pLayerSection = (ed_g2d_layer*)LOAD_SECTION(pAfter->pLAY);

	pcVar1 = (char*)0x0;
	if ((pLayerSection != (ed_g2d_layer*)0xfffffff0) && (pLayerSection->body.field_0x1c != 0)) {
		iVar2 = &((TextureData_TEX*)LOAD_SECTION(pLayerSection->body.pTex))->body;
	}
	if (iVar2 == (TextureData_TEX_Internal*)0x0) {
		pcVar1 = (char*)0x0;
	}
	else {
		TextureData_HASH_Internal_PA32* otherValue = (TextureData_HASH_Internal_PA32*)LOAD_SECTION(iVar2->after.pHASH_Internal);
		if (otherValue != 0) {
			pcVar1 = (char*)((char*)LOAD_SECTION(otherValue->pPA32) + 0x10);
		}
	}
	return pcVar1;
}

void* edDListInitMaterial(edDList_material* outObj, TextureData_HASH_Internal_MAT* pHASH_MAT, ed_g2d_manager* textureInfoObj, int mode)
{
	void* pvVar1;
	DisplayList_0x10* someGlobalBuffer;
	uint counter;

	counter = 0;
	pvVar1 = memset(outObj, 0, sizeof(edDList_material));
	outObj->pMAT = &(((TextureData_MAT*)LOAD_SECTION(pHASH_MAT->pMAT))->body);
	while (true) {
		if ((*(long*)(gBankMaterial + counter) == 0) || (g_DisplayListObjCount_004250e0 - 1U <= counter)) break;
		counter = counter + 1;
	}
	someGlobalBuffer = gBankMaterial + counter;
	*(undefined8*)someGlobalBuffer = *(undefined8*)pHASH_MAT;
	someGlobalBuffer->pHASH_MAT = pHASH_MAT;
	outObj->Length = counter;
	gNbUsedMaterial = gNbUsedMaterial + 1;
	outObj->mode = mode;
	outObj->textureInfo = textureInfoObj;

#ifdef PLATFORM_WIN
	onMaterialLoadedDelegate(outObj);
#endif

	return pvVar1;
}

bool edDListTermMaterial(edDList_material* pMaterial)
{
	char** ppcVar1;

	if ((*(ulong*)(gBankMaterial + pMaterial->Length) != 0) &&
		(ppcVar1 = (char**)&gBankMaterial->pHASH_MAT, ppcVar1[pMaterial->Length * 4] != (char*)0x0)) {
		*(undefined8*)(gBankMaterial + pMaterial->Length) = 0;
		ppcVar1[pMaterial->Length * 4] = (char*)0x0;
		gNbUsedMaterial = gNbUsedMaterial + -1;
	}

#ifdef PLATFORM_WIN
	onMaterialUnloadedDelegate(pMaterial);
#endif

	return true;
}


edDList_material* edDListCreatMaterialFromIndex(edDList_material* pMaterialInfo, int index, ed_g2d_manager* pTextureInfo, int mode)
{
	TextureData_HASH_Internal_MAT* pHASH_MAT;

	pHASH_MAT = ed3DG2DGetMaterialFromIndex(pTextureInfo, index);
	if (pHASH_MAT == (TextureData_HASH_Internal_MAT*)0x0) {
		pMaterialInfo = (edDList_material*)0x0;
	}
	else {
		edDListInitMaterial(pMaterialInfo, pHASH_MAT, pTextureInfo, mode);
	}
	return pMaterialInfo;
}

void ed3DPrepareG3DManageStruct
(ed_g3d_manager* meshInfoObj, char* fileBufferStartA, int fileLengthA, char* fileBufferStartB, int fileLengthB)
{
	int* piVar1;
	int readValue;

	for (piVar1 = (int*)edChunckGetFirst(fileBufferStartB, fileBufferStartB + fileLengthB); piVar1 != (int*)0x0;
		piVar1 = edChunckGetNext((char*)piVar1, (int*)(fileBufferStartB + fileLengthB))) {
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
		for (piVar1 = (int*)edChunckGetFirst(fileBufferStartA, fileBufferStartA + fileLengthA); piVar1 != (int*)0x0;
			piVar1 = edChunckGetNext((char*)piVar1, (int*)(fileBufferStartA + fileLengthA))) {
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

char* edHashcodeGet(Hash_8 meshHashValue, MeshData_HASH* textureObjMatBuffer)
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

uint edChunckGetNb(char* pStart, char* pEnd)
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

void ed3DPrepareMaterialBank(char* pMBNA, TextureInfo* pTextureInfo)
{
	int iVar1;
	ulong* puVar2;
	uint uVar3;
	char* pcVar4;
	ulong* pMBNK;

	iVar1 = *(int*)(pMBNA + 8);
	/* Checks all the hashes in the mesh to make sure they match what is in the texture

		Get the end of the current section */
	for (puVar2 = (ulong*)edChunckGetFirst(pMBNA + 0x10, pMBNA + iVar1); puVar2 != (ulong*)0x0;
		puVar2 = (ulong*)edChunckGetNext((char*)puVar2, (int*)(pMBNA + iVar1))) {
		/* Check if read value is MBNK */
		if (*(int*)puVar2 == 0x4b4e424d) {
			uVar3 = *(int*)(puVar2 + 1) - 0x10U >> 4;
			pMBNK = puVar2;
			while (true) {
				if ((uVar3 & 0xffff) == 0) break;
				pcVar4 = edHashcodeGet(pMBNK[2], (MeshData_HASH*)(pTextureInfo->field_0x0).pMAT_HASH);
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

void ed3DPrepareClusterTree(MeshData_CDQU* pCDQU, bool param_2, ed_g3d_manager* pMeshInfo, TextureInfo* pTextureInfo, int param_5, long param_6)
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
				puVar5 = ed3DPrepareStrip(&local_30);
				if ((puVar5 != (uint*)0x0) &&
					(g_pStrippBufLastPos =
						(edpkt_data*)
						ed3DStripPreparePacket(puVar5, (ulong*)g_pStrippBufLastPos, (int)(piVar4 + 4), 4), uVar7 == 0
						)) {
					pCDQU->field_0x38 = puVar5;
				}
				piVar2 = edChunckGetNext((char*)piVar2, (int*)0x0);)
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
						ed3DStripPrepareSpherePacket((int)puVar5, (long)(int)puVar5[0xf], (int)(piVar4 + 4));
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
	for (piVar4 = (int*)edChunckGetFirst
	((char*)((ulong)pCDQU->field_0x10 + offset + 0x30), pCDQU->field_0x0 + iVar3 + -0x10);
		piVar4 != (int*)0x0; piVar4 = edChunckGetNext((char*)piVar4, (int*)(pCDQU->field_0x0 + iVar3 + -0x10))) {
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
				IMPLEMENTATION_GUARD(puVar5 = ed3DPrepareAllSprite((int)piVar4, &aStack96, (int)(pCDQU->field_0x30 + 4), 4));
				pCDQU->field_0x3c = (int)STORE_SECTION(puVar5);
				pCDQU->field_0x1e = aStack96.field_0x24;
			}
		}
		else {
			if ((offset == 0x55514443) || (offset == 0x434f4443)) {
				ed3DPrepareClusterTree((MeshData_CDQU*)(piVar4 + 4), param_2, pMeshInfo, pTextureInfo, param_5, param_6);
			}
		}
	}
	return;
}

void ed3DPrepareClusterALL(bool param_1, ed_g3d_manager* meshInfoObj, TextureInfo* textureInfo, int param_4)
{
	int iVar1;
	int iVar2;
	int* piVar3;
	int* nextSection;
	ulong numSections;
	char* pCSTA;
	edpkt_data* pCommandBuffer;

	pCommandBuffer = g_pStrippBufLastPos;
	pCSTA = meshInfoObj->CSTA;
	iVar1 = *(int*)(pCSTA + 8);
	numSections = (ulong)((*(uint*)(meshInfoObj->fileBufferStart + 4) & 1) != 0);
	for (piVar3 = (int*)edChunckGetFirst(pCSTA + 0x10, pCSTA + iVar1); piVar3 != (int*)0x0;
		piVar3 = edChunckGetNext((char*)piVar3, (int*)(pCSTA + iVar1))) {
		/* If read value is CDQA */
		if (*piVar3 == 0x41514443) {
			/* Search for CDQU */
			iVar2 = piVar3[2];
			for (nextSection = (int*)edChunckGetFirst((char*)(piVar3 + 0xc), (char*)(int*)((ulong)piVar3 + iVar2));
				nextSection != (int*)0x0; nextSection = edChunckGetNext((char*)nextSection, (int*)((ulong)piVar3 + iVar2)))
			{
				/* If read value is CDQU */
				if (*nextSection == 0x55514443) {
					ed3DPrepareClusterTree((MeshData_CDQU*)(nextSection + 4), true, meshInfoObj, textureInfo, param_4, numSections);
				}
			}
		}
		else {
			/* If read value is CDOA */
			if (*piVar3 == 0x414f4443) {
				iVar2 = piVar3[2];
				for (nextSection = (int*)edChunckGetFirst((char*)(piVar3 + 0xc), (char*)(int*)((ulong)piVar3 + iVar2));
					nextSection != (int*)0x0;
					nextSection = edChunckGetNext((char*)nextSection, (int*)((ulong)piVar3 + iVar2))) {
					/* If read value is CDOC */
					if (*nextSection == 0x434f4443) {
						ed3DPrepareClusterTree((MeshData_CDQU*)(nextSection + 4), true, meshInfoObj, textureInfo, param_4, numSections);
					}
				}
			}
		}
	}
	meshInfoObj->fileLengthB =
		meshInfoObj->fileLengthB + (int)((int)g_pStrippBufLastPos - (int)pCommandBuffer);
	return;
}

void ed3DLinkG2DToG3D(ed_g3d_manager* pMeshInfo, TextureInfo* pTextureInfo)
{
	ed3DPrepareMaterialBank(pMeshInfo->MBNA, pTextureInfo);
	if (pMeshInfo->CSTA != (char*)0x0) {
		ed3DPrepareClusterALL(true, pMeshInfo, pTextureInfo, 0xc);
	}
	return;
}

PACK( struct MeshTransformObjData_Packed {
	int pObj; // char*
	short field_0x4;
	short field_0x6;
});

PACK( struct TextureData_HIER_Internal {
	edF32MATRIX4 transformA;
	edF32MATRIX4 transformB;
	Hash_8 hash;
	byte field_0x88;
	byte field_0x89;
	ushort bRenderShadow;
	int pShadowAnimMatrix; // edF32MATRIX4*
	int pLinkTransformData; // MeshTransformData*
	int field_0x94; // undefined*
	int pTextureInfo; // undefined*
	ushort count_0x9c;
	ushort flags_0x9e;
	int pLightingMatrixFuncObj_0xa0; // LightingMatrixFuncObj*
	int field_0xa4; // edF32MATRIX4*
	int pAnimMatrix; // edF32MATRIX4*
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

int ed3DPrepareHierarchy(TextureData_HIER_Internal* pHIER, TextureInfo* pTextureInfo)
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
			for (piVar3 = (int*)edChunckGetFirst(pBuffStart, pBuffStart + iVar4); piVar3 != (int*)0x0;
				piVar3 = edChunckGetNext((char*)piVar3, (int*)(pBuffStart + iVar4))) {
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
					//ed3DPrepareObjectSprite((int)pHIER, (uint*)(piVar3 + 4), pTextureInfo);
				}
			}
			else {
				if ((((sVar1 != 2) && (sVar1 != 1)) && (sVar1 == 0)) && (*piVar3 == 0x2e4a424f)) {
					//ed3DPrepareObject((int)pHIER, (uint*)(piVar3 + 4), pTextureInfo);
				}
			}
			pTVar5 = (TextureData_HIER_Internal*)&pTVar5->field_0x8;
		}
		if (pHIER->field_0x90 != 0) {
			//ed3DPrepareHierarchy((TextureData_HIER_Internal*)(pHIER->field_0x90 + 0x10), pTextureInfo);
		}
		iVar4 = 1;)
	}
	else {
		iVar4 = 0;
	}
	return iVar4;
}

void ed3DPrepareHierarchyALL(ed_g3d_manager* pMeshInfo, TextureInfo* pTextureInfo)
{
	char* pcVar1;
	uint uVar2;
	char* pcVar3;
	uint uVar4;

	pcVar1 = pMeshInfo->HALL;
	pcVar3 = pcVar1 + 0x20;
	uVar2 = edChunckGetNb(pcVar1 + 0x10, pcVar1 + *(int*)(pcVar1 + 8));
	for (uVar4 = 0; uVar4 < (uVar2 & 0xffff) - 1; uVar4 = uVar4 + 1) {
		ed3DPrepareHierarchy((TextureData_HIER_Internal*)(((char*)LOAD_SECTION(*(int*)(pcVar3 + 8))) + 0x10), pTextureInfo);
		pcVar3 = pcVar3 + 0x10;
	}
	return;
}

void ed3DPrepareG3D(bool param_1, ed_g3d_manager* meshInfoObj, TextureInfo* textureInfoObj, int unknown)
{
	char* pAlloc;

	if (textureInfoObj != (TextureInfo*)0x0) {
		ed3DPrepareMaterialBank(meshInfoObj->MBNA, textureInfoObj);
	}
	if (meshInfoObj->CSTA == (char*)0x0) {
		ed3DPrepareHierarchyALL(meshInfoObj, textureInfoObj);
		pAlloc = meshInfoObj->field_0x4;
	}
	else {
		ed3DPrepareClusterALL(param_1, meshInfoObj, textureInfoObj, unknown);
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

ed_g3d_manager* ed3DInstallG3D(char* pFileData, int fileLength, ulong flags, int* outInt, TextureInfo* textureObj, int unknown, ed_g3d_manager* pMeshInfo)
{
	bool bVar1;
	int size;
	ed_g3d_manager* pMVar2;
	int* piVar3;
	char* fileBuffer;
	ed_g3d_manager* meshInfoObj;
	char* pcVar4;
	int size_00;

	meshInfoObj = pMeshInfo;
	pMVar2 = gpG3D;
	if (pMeshInfo == (ed_g3d_manager*)0x0) {
		while (meshInfoObj = pMVar2, meshInfoObj->fileBufferStart != 0) {
			pMVar2 = meshInfoObj + 1;
		}
	}
	/* Check if the offset value from the buffer is '.G3D' */
	if (((meshInfoObj == (ed_g3d_manager*)0x0) || (memset(meshInfoObj, 0, 0x40), pFileData == (char*)0x0)) ||
		(*(int*)(pFileData + 0xc) != 0x4433472e)) {
		bVar1 = meshInfoObj != pMeshInfo;
		meshInfoObj = (ed_g3d_manager*)0x0;
		if (bVar1) {
			gNbG3D = gNbG3D + -1;
			meshInfoObj = (ed_g3d_manager*)0x0;
		}
	}
	else {
		meshInfoObj->fileBufferStart = pFileData;
		meshInfoObj->fileLengthA = fileLength;
		meshInfoObj->field_0x4 = (char*)0x0;
		if ((*(uint*)(pFileData + 4) & 1) == 0) {
			pcVar4 = pFileData + 0x10;
			if ((flags & 2) == 0) {
				ed3DPrepareG3DManageStruct(meshInfoObj, pcVar4, fileLength + -0x10, pcVar4, fileLength + -0x10);
				IMPLEMENTATION_GUARD(); // This needs checking.
				edMemSetFlags(TO_HEAP(H_MAIN), 0x100);
				size = *(int*)(meshInfoObj->GEOM + 8);
				size_00 = size + *(int*)(meshInfoObj->GEOM + size + 8);
				fileBuffer = (char*)edMemAllocAlignBoundary(TO_HEAP(H_MAIN), size_00);
				memcpy(fileBuffer, (void*)meshInfoObj->GEOM, size_00);
				edMemClearFlags(TO_HEAP(H_MAIN), 0x100);
				*outInt = (fileLength - size_00) - fileLength;
				piVar3 = ed3DPreparePointer(fileBuffer, size_00, pcVar4, (fileLength - size_00) + -0x10);
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
				ed3DPrepareG3DManageStruct(meshInfoObj, pcVar4, fileLength + -0x10, pcVar4, fileLength + -0x10);
				piVar3 = ed3DPreparePointer(pcVar4, fileLength + -0x10, pcVar4, fileLength + -0x10);
				if (piVar3 != (int*)0x0) {
					IMPLEMENTATION_GUARD(); // This needs checking.
					*outInt = -piVar3[2];
					*(int*)(pFileData + 8) = *(int*)(pFileData + 8) - piVar3[2];
				}
				meshInfoObj->fileLengthB = fileLength;
			}
			ed3DPrepareG3D(1, meshInfoObj, textureObj, (long)unknown);
		}
		else {
			pcVar4 = (char*)(pFileData + 0x10);
			ed3DPrepareG3DManageStruct(meshInfoObj, pcVar4, fileLength + -0x10, pcVar4, fileLength + -0x10);
			ed3DPreparePointer(pcVar4, fileLength + -0x10, pcVar4, fileLength + -0x10);
			meshInfoObj->GEOM = (char*)0x0;
			meshInfoObj->field_0x4 = (char*)0x0;
			meshInfoObj->field_0xc = 0;
			meshInfoObj->fileLengthB = meshInfoObj->fileLengthA;
			ed3DPrepareG3D(1, meshInfoObj, textureObj, unknown);
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

void ed3DHierarchyCopyHashCode(ed_g3d_manager* pMeshInfo)
{
	uint uVar1;
	uint uVar2;
	ulong* pMVar3;
	MeshData_HALL* pcVar1;

	pcVar1 = (MeshData_HALL*)pMeshInfo->HALL;
	pMVar3 = (ulong*)(pcVar1 + 2);
	uVar1 = edChunckGetNb((char*)(pcVar1 + 1), (char*)pcVar1 + pcVar1->totalSize);
	for (uVar2 = 0; uVar2 < (uVar1 & 0xffff) - 1; uVar2 = uVar2 + 1) {
		TextureData_HIER* p = (TextureData_HIER*)LOAD_SECTION(*(pMVar3 + 1));
		p->body.hash.number = *pMVar3;
		pMVar3 = pMVar3 + 2;
	}
	return;
}

int g_LoadedStaticMeshCount_004493a4 = 0;

MeshTransformParent* ed3DHierarchyAddNode
	(edLIST* pCameraPanMasterHeader, MeshTransformData* pSubSubObjArray,
		MeshTransformParent* pCameraPanStaticArray, TextureData_HIER_Internal* pHIER_Internal, MeshTransformData* param_5)

{
	MeshTransformParent* pMVar1;
	undefined8 uVar2;
	MeshTransformData* pCVar8;
	float fVar3;
	float fVar4;
	int iVar5;
	edF32MATRIX4* pMVar6;
	MeshTransformObjData_Packed* pfVar7;
	MeshTransformData* pMVar8;
	MeshTransformObjData* pcVar9;
	edF32MATRIX4* pMVar10;
	edF32MATRIX4* pMVar11;
	edF32MATRIX4* pTVar12;
	uint uVar13;
	MeshTransformData* pMVar14;
	ushort* puVar15;
	MeshTransformParent* pCVar1;
	edNODE_MANAGER* pCameraPanHeader;

	MY_LOG("ed3DHierarchyAddNode %s\n", pHIER_Internal->hash.name);

	iVar5 = 4;
	pCameraPanHeader = pCameraPanMasterHeader->pCameraPanHeader_0xc;
	pMVar1 = pCameraPanMasterHeader->pCameraPanStatic_0x8;
	pCameraPanHeader->usedCount = pCameraPanHeader->usedCount + 1;
	pCameraPanMasterHeader->count_0x14 = pCameraPanMasterHeader->count_0x14 + 1;
	pCVar1 = (MeshTransformParent*)pCameraPanHeader->pNextFree;
	uVar13 = (uint)((ulong)pCVar1 - (ulong)pCameraPanStaticArray) >> 4;
	pCameraPanHeader->pNextFree = (MeshTransformSpecial*)pCVar1->pNext;
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
	(pCVar8->base).pShadowAnimMatrix = (edF32MATRIX4*)LOAD_SECTION(pHIER_Internal->pShadowAnimMatrix);
	(pCVar8->base).pLinkTransformData = (MeshTransformData*)LOAD_SECTION(pHIER_Internal->pLinkTransformData);
	(pCVar8->base).field_0x94 = (undefined*)LOAD_SECTION(pHIER_Internal->field_0x94);
	(pCVar8->base).pTextureInfo = (undefined*)LOAD_SECTION(pHIER_Internal->pTextureInfo);
	(pCVar8->base).count_0x9c = pHIER_Internal->count_0x9c;
	(pCVar8->base).flags_0x9e = pHIER_Internal->flags_0x9e;
	(pCVar8->base).pLightingMatrixFuncObj_0xa0 = (LightingMatrixFuncObj*)LOAD_SECTION(pHIER_Internal->pLightingMatrixFuncObj_0xa0);
	(pCVar8->base).field_0xa4 = (edF32MATRIX4*)LOAD_SECTION(pHIER_Internal->field_0xa4);
	(pCVar8->base).pAnimMatrix = (edF32MATRIX4*)LOAD_SECTION(pHIER_Internal->pAnimMatrix);
	(pCVar8->base).subMeshParentCount_0xac = pHIER_Internal->subMeshParentCount_0xac;
	(pCVar8->base).size_0xae = pHIER_Internal->size_0xae;
	(pCVar8->base).GlobalAlhaON = *(char*)&pHIER_Internal->field_0xaf;

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
	(pCVar8->base).GlobalAlhaON = -1;
	(pCVar8->base).pShadowAnimMatrix = (edF32MATRIX4*)0x0;
	(pCVar8->base).pLinkTransformData = param_5;
	(pCVar8->base).field_0x94 = (undefined*)param_5;
	pMVar8 = pSubSubObjArray + uVar13;
	if ((pCVar8->base).subMeshParentCount_0xac == 0) {
		*puVar15 = *puVar15 | 8;
	}
	(pMVar8->base).flags_0x9e = (pMVar8->base).flags_0x9e & 0xfffc;
	(pMVar8->base).flags_0x9e = (pMVar8->base).flags_0x9e | 0x800;
	ed3DSetSceneRender(pMVar8, 1);
	pCVar1->pMeshTransformData = (DisplayListInternalMesh*)pMVar8;
	return pCVar1;
}


void ed3DHierarchyAddSonsToList
(edLIST* pCameraPanMasterHeader, MeshTransformData* pSubSubObjArray,
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
			pMVar9 = ed3DHierarchyAddNode
			(pCameraPanMasterHeader, pSubSubObjArray, pCameraPanStaticArray, &iVar1->body,
				(MeshTransformData*)param_5->pMeshTransformData);
			local_24 = (MeshTransformParent*)0x0;
			local_40 = param_6;
			for (local_20 = 0; local_20 < count; local_20 = local_20 + 1) {
				iVar2 = *(int*)(local_40 + 8);
				if (*(TextureData_HIER**)(iVar2 + 0xa0) == iVar1) {
					pMVar10 = ed3DHierarchyAddNode
					(pCameraPanMasterHeader, pSubSubObjArray, pCameraPanStaticArray,
						(TextureData_HIER_Internal*)(iVar2 + 0x10),
						(MeshTransformData*)pMVar9->pMeshTransformData);
					local_74 = (MeshTransformParent*)0x0;
					local_90 = param_6;
					for (local_70 = 0; local_70 < count; local_70 = local_70 + 1) {
						iVar3 = *(int*)(local_90 + 8);
						if (*(int*)(iVar3 + 0xa0) == iVar2) {
							pMVar11 = ed3DHierarchyAddNode
							(pCameraPanMasterHeader, pSubSubObjArray, pCameraPanStaticArray,
								(TextureData_HIER_Internal*)(iVar3 + 0x10),
								(MeshTransformData*)pMVar10->pMeshTransformData);
							local_c4 = (MeshTransformParent*)0x0;
							local_e0 = param_6;
							for (local_c0 = 0; local_c0 < count; local_c0 = local_c0 + 1) {
								iVar4 = *(int*)(local_e0 + 8);
								if (*(int*)(iVar4 + 0xa0) == iVar3) {
									pMVar12 = ed3DHierarchyAddNode
									(pCameraPanMasterHeader, pSubSubObjArray, pCameraPanStaticArray,
										(TextureData_HIER_Internal*)(iVar4 + 0x10),
										(MeshTransformData*)pMVar11->pMeshTransformData);
									local_114 = (MeshTransformParent*)0x0;
									local_130 = param_6;
									for (local_110 = 0; local_110 < count; local_110 = local_110 + 1) {
										iVar5 = *(int*)(local_130 + 8);
										if (*(int*)(iVar5 + 0xa0) == iVar4) {
											pMVar13 = ed3DHierarchyAddNode
											(pCameraPanMasterHeader, pSubSubObjArray, pCameraPanStaticArray,
												(TextureData_HIER_Internal*)(iVar5 + 0x10),
												(MeshTransformData*)pMVar12->pMeshTransformData);
											local_164 = (MeshTransformParent*)0x0;
											local_180 = param_6;
											for (local_160 = 0; local_160 < count; local_160 = local_160 + 1) {
												iVar6 = *(int*)(local_180 + 8);
												if (*(int*)(iVar6 + 0xa0) == iVar5) {
													pMVar14 = ed3DHierarchyAddNode
													(pCameraPanMasterHeader, pSubSubObjArray, pCameraPanStaticArray,
														(TextureData_HIER_Internal*)(iVar6 + 0x10),
														(MeshTransformData*)pMVar13->pMeshTransformData);
													local_1b4 = (MeshTransformParent*)0x0;
													local_1d0 = param_6;
													for (local_1b0 = 0; local_1b0 < count; local_1b0 = local_1b0 + 1) {
														iVar7 = *(int*)(local_1d0 + 8);
														if (*(int*)(iVar7 + 0xa0) == iVar6) {
															pMVar15 = ed3DHierarchyAddNode
															(pCameraPanMasterHeader, pSubSubObjArray, pCameraPanStaticArray,
																(TextureData_HIER_Internal*)(iVar7 + 0x10),
																(MeshTransformData*)pMVar14->pMeshTransformData);
															local_204 = (MeshTransformParent*)0x0;
															local_220 = param_6;
															for (local_200 = 0; local_200 < count; local_200 = local_200 + 1) {
																iVar8 = *(int*)(local_220 + 8);
																if (*(int*)(iVar8 + 0xa0) == iVar7) {
																	pMVar16 = ed3DHierarchyAddNode
																	(pCameraPanMasterHeader, pSubSubObjArray, pCameraPanStaticArray,
																		(TextureData_HIER_Internal*)(iVar8 + 0x10),
																		(MeshTransformData*)pMVar15->pMeshTransformData);
																	IMPLEMENTATION_GUARD();
																	ed3DHierarchyAddSonsToList
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

void ed3DHierarchyRefreshSonNumbers(MeshTransformParent* pMeshTransformParent, short* outMeshCount)
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
			ed3DHierarchyRefreshSonNumbers((MeshTransformParent*)uVar3, &local_2);
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

void RotationVectorToMatrix(Vector* v0, edF32MATRIX4* m0)
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

uint HierarchyAnm::UpdateMatrix
(float param_1, MeshTransformData* param_3, int* pFileData, int param_5)
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
			RotationVectorToMatrix(pVVar10, (edF32MATRIX4*)param_3);
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
			RotationVectorToMatrix(&VStack32, (edF32MATRIX4*)param_3);)
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

MeshData_OBB* ed3DHierarchyNodeGetSkeletonChunck(MeshTransformParent* pMeshTransformParent, bool mode)
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
		(pMVar4 = ed3DChooseGoodLOD(pMeshTransformData), pMVar4 != (MeshTransformObjData*)0x0)) {
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
				edChunckGetFirst((char*)((ulong)piVar2 + piVar2[3]), (char*)(int*)((ulong)piVar2 + iVar3));
				pMVar6 != (MeshData_OBB*)0x0;
				pMVar6 = (MeshData_OBB*)edChunckGetNext((char*)pMVar6, (int*)((ulong)piVar2 + iVar3))) {
				/* Look for OBB */
				if (*(int*)pMVar6 == 0x4742424f) {
					return pMVar6;
				}
			}
		}
	}
	return (MeshData_OBB*)0x0;
}

int NodeIndexFromID(ushort** param_1, uint key)
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

void HierarchyAnm::Install(MeshData_ANHR* pInANHR, int length, ed_g3d_manager* pMeshInfo,
	ed_3D_Scene* pStaticMeshMaster)
{
	uint uVar1;
	MeshData_HALL* pcVar2;
	edLIST* pCameraPanMasterHeader;
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
		this->pThis = (MeshData_ANHR*)0x0;
	}
	else {
		this->pThis = pInANHR;
		pcVar1 = this->pThis;
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
			meshHashValue = ByteCode::BuildU64((int)pMVar14->field_0x0, (int)pMVar14->field_0x4);
			ed3DHierarchyCopyHashCode(pMeshInfo);
			pCameraPanStaticArray = gHierarchyManagerFirstFreeNode;
			pSubSubObjArray = gHierarchyManagerBuffer;
			pcVar2 = (MeshData_HALL*)pMeshInfo->HALL;
			pCameraPanMasterHeader = pStaticMeshMaster->pHeirListA;
			uVar3 = edChunckGetNb((char*)(pcVar2 + 1), (char*)pcVar2 + pcVar2->totalSize);
			pcVar4 = edHashcodeGet(meshHashValue, (MeshData_HASH*)(pMeshInfo->HALL + 0x10));
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
					ed3DHierarchyAddNode
					(pCameraPanMasterHeader, pSubSubObjArray, pCameraPanStaticArray, (TextureData_HIER_Internal*)(((char*)LOAD_SECTION(iVar5)) + 0x10),
						(MeshTransformData*)0x0);
				ed3DHierarchyAddSonsToList(pCameraPanMasterHeader, pSubSubObjArray, pCameraPanStaticArray, (TextureData_HIER*)LOAD_SECTION(iVar5), pMeshTransformParent,
					(char*)(pcVar2 + 2), (uVar3 & 0xffff) - 1);
				ed3DHierarchyRefreshSonNumbers(pMeshTransformParent, &sStack2);
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
				UpdateMatrix(pMVar14->field_0x28, (MeshTransformData*)LOAD_SECTION(pMVar14->field_0x4), (int*)puVar10,
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
				pMVar6 = ed3DHierarchyNodeGetSkeletonChunck(pMeshTransformParent, 0);
				if (pMVar6 == (MeshData_OBB*)0x0) {
					pMVar14->field_0x30.pObb_Internal = 0x0;
				}
				else {
					puVar10 = (uint*)(pMVar14 + 1);
					pMVar14->field_0x30.pObb_Internal = (int)STORE_SECTION(pMVar6 + 1);
					local_8 = (ushort*)LOAD_SECTION(pMVar14->field_0x30.pObb_Internal);
					uVar3 = pMVar14->field_0xc;
					for (uVar9 = 0; uVar9 < uVar3; uVar9 = uVar9 + 1) {
						uVar12 = NodeIndexFromID(&local_8, *puVar10);
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

void ed3DScenePushCluster(ed_3D_Scene* pStaticMeshMaster, ed_g3d_manager* pMeshInfo)
{
	edNODE_MANAGER* pMVar1;
	MeshTransformSpecial* pMVar2;
	DisplayListInternalMesh* pDVar3;
	uint uVar4;

	uVar4 = 0;
	while ((uVar4 < ged3DConfig.meshDisplayListInternalCount &&
		((g_MeshDisplayListInternal_00449380[uVar4].flags & 1) != 0))) {
		uVar4 = uVar4 + 1;
	}
	if (uVar4 < ged3DConfig.meshDisplayListInternalCount) {
		pDVar3 = g_MeshDisplayListInternal_00449380 + uVar4;
		INT_004493a8 = INT_004493a8 + 1;
		pDVar3->pMeshInfo = pMeshInfo;
		g_MeshDisplayListInternal_00449380[uVar4].flags = (g_MeshDisplayListInternal_00449380[uVar4].flags | 1);
		pMVar1 = (pStaticMeshMaster->headerB).pCameraPanHeader_0xc;
		pMVar2 = pMVar1->pNextFree;
		pMVar1->pNextFree = pMVar2->pNext_0x4;
		pMVar2->pRenderInput = (DisplayListInternal*)pDVar3;
		pMVar1->usedCount = pMVar1->usedCount + 1;
		(pStaticMeshMaster->headerB).count_0x14 = (pStaticMeshMaster->headerB).count_0x14 + 1;
		pMVar2->pNext_0x4 = (pStaticMeshMaster->headerB).pLoadedData;
		RENDER_LOG("ed3DScenePushCluster %p\n", (pStaticMeshMaster->headerB).pLoadedData);
		(pStaticMeshMaster->headerB).pLoadedData = pMVar2;
	}
	return;
}

MeshTransformParent* ed3DHierarchyAddToSceneByHashcode(ed_3D_Scene* pStaticMeshMaster, ed_g3d_manager* pMeshInfo, ulong hash)
{
	edLIST* pCameraPanMasterHeader;
	MeshTransformData* pSubSubObjArray;
	MeshTransformParent* pCameraPanStaticArray;
	uint uVar1;
	char* pcVar2;
	MeshTransformParent* pMVar3;
	int iVar4;
	short sStack2;
	MeshData_HALL* pcVar1;

	ed3DHierarchyCopyHashCode(pMeshInfo);
	pCameraPanStaticArray = gHierarchyManagerFirstFreeNode;
	pSubSubObjArray = gHierarchyManagerBuffer;
	pcVar1 = (MeshData_HALL*)pMeshInfo->HALL;
	pCameraPanMasterHeader = pStaticMeshMaster->pHeirListA;
	uVar1 = edChunckGetNb((char*)(pcVar1 + 1), (char*)pcVar1 + pcVar1->totalSize);
	pcVar2 = edHashcodeGet(hash, (MeshData_HASH*)(pMeshInfo->HALL + 0x10));
	iVar4 = 0;
	if (pcVar2 != (char*)0x0) {
		iVar4 = *(int*)(pcVar2 + 8);
	}
	if (iVar4 == 0) {
		pMVar3 = (MeshTransformParent*)0x0;
	}
	else {
		IMPLEMENTATION_GUARD(pMVar3 = edLIST::Func_002ab920
		(pCameraPanMasterHeader, pSubSubObjArray, pCameraPanStaticArray, (undefined8*)(iVar4 + 0x10),
			(MeshTransformData*)0x0);
		edLIST::Func_002ad020
		(pCameraPanMasterHeader, pSubSubObjArray, pCameraPanStaticArray, iVar4, (int)pMVar3, (int)(pcVar1 + 2),
			(uVar1 & 0xffff) - 1);
		MeshTransformParent::Func_002ac980(pMVar3, &sStack2);)
	}
	return pMVar3;
}
