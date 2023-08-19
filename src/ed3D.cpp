#include "ed3D.h"
#include "edMem.h"
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "edDlist.h"
#include "edList.h"

#if defined(PLATFORM_PS2)
#include <eekernel.h>
#include <libdma.h>
#else
#include "renderer.h"
#include "port.h"
#include "port/vu1_emu.h"
#endif

#include "edVideo/VideoD.h"
#include "port/pointer_conv.h"
#include "edVideo/VideoB.h"
#include "edVideo/VideoC.h"
#include "Rendering/DisplayList.h"
#include "edVideo/Camera.h"

#include "Rendering/CustomShell.h"
#include "Types.h"
#include "MathOps.h"
#include "CameraViewManager.h"

#include <math.h>
#include "FileManager3D.h"

#include "Rendering/CameraPanMasterHeader.h"
#include "Rendering/OneTimeCommands.h"
#include "MemoryStream.h"

#include <string.h>
#include "ed3DScratchPadGlobalVar.h"

#define SHELLDMA_TAG_ID_CNT (0x10)
#define SHELLDMA_TAG_ID_REF (0x30)

#define ED_VIF1_SET_TAG_CNT(qwc) ((ulong)(qwc) | ((ulong)SHELLDMA_TAG_ID_CNT << 24) | ((ulong)0xe << 32))
#define ED_VIF1_SET_TAG_REF(qwc, addr) ((ulong)((ulong)(qwc) | ((ulong)SHELLDMA_TAG_ID_REF << 24))) | ((ulong)(addr) << 32)

#define ED3D_LOG(level, format, ...) MY_LOG_CATEGORY("ed3D", level, format, ##__VA_ARGS__)

char* s_ed3D_Initialsation_004333a0 = "ed3D Initialsation\n";

#ifdef PLATFORM_PS2
#define SCRATCHPAD_ADDRESS(addr) (edpkt_data*)addr
#define SCRATCHPAD_ADDRESS_TYPE(addr, type) (type)addr
#define SCRATCHPAD_READ_ADDRESS_TYPE(addr, type) (type)addr

#define SCRATCHPAD_WRITE_ADDRESS_TYPE(addr, type, value) *((type*)addr) = value;
#else
#define SCRATCHPAD_ADDRESS(addr) (edpkt_data*)((addr - 0x70000000) + (char*)WorldToCamera_Matrix)
#define SCRATCHPAD_ADDRESS_TYPE(addr, type) (type)((addr - 0x70000000) + (char*)WorldToCamera_Matrix)
#define SCRATCHPAD_READ_ADDRESS_TYPE(addr, type) *(type*)((addr - 0x70000000) + (char*)WorldToCamera_Matrix)

#define SCRATCHPAD_WRITE_ADDRESS_TYPE(addr, type, value) do { \
	type* destPtr = (type*)((addr - 0x70000000) + (char*)WorldToCamera_Matrix); \
	*destPtr = value; \
} while(0)

#endif
#ifdef PLATFORM_WIN
#define PKT_TO_SCRATCHPAD(pkt) pkt
#else
#define PKT_TO_SCRATCHPAD(pkt) (void*)((uint)pkt & 0xfffffff)
#endif

#define PKT_SIZE(end, begin) (ulong)end - (ulong)begin

int gCurTime = 0;
int gStepTime = 0;

edF32MATRIX4 HierarchyAnm::_gscale_mat = {
	0.6954f, 0.0f, 0.0f, 0.0f,
	0.0f, 0.6954f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.6954f, 0.0f,
	0.0f, 0.0f, 0.0f, 1.0f
};

ed3DConfig::ed3DConfig()
	: meshHeaderCountB(0x3E0)
	, sceneCount(0xF)
	, maxClusterCount(0x10)
	, meshHeaderCountBAlt(0x2800)
	, matrixBufferCount(0x1F4)
	, materialBufferCount(0xFA)
	, field_0x18(0x7D0)
	, g3dManagerCount(0xA)
	, g2dManagerCount(0x19)
	, bEnableProfile(0x0)
	, field_0x25(0x0)
	, clusterTabListSize(0x64)
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

int gNbVertexDMA = 0;
ed_g2d_manager* gpG2D;

edpkt_data* g_pStrippBufLastPos;
edpkt_data* g_pStripBuf;
edpkt_data* g_pStrippBufMaxPos;
edpkt_data* g_pStripBufResetPos;

edpkt_data g_stMatrixHeader;
edpkt_data g_stVertexGIFHeader;
edpkt_data g_stVertexSTHeader[2];

ed3DConfig ged3DConfig;

edSurface* gVRAMed3D = NULL;

ed_3D_Scene* gScene3D = NULL;
edNODE_MANAGER* gNodeCluster = NULL;

int gNbG2D = 0;
int gNbG3D = 0;


edpkt_data* g_GifRefPktCur = NULL;
edpkt_data* g_pGifStart = NULL;

int gFXBufAddr = 0;

edLIST gPrim_List_FlushTex[2] = { };
edLIST gPrim_List_Shadow[2] = { };

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
	ushort aClusterStripCounts[5];
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
	int p3DStrip; // ed_3d_strip*
});

PACK(
	struct ed_Bound_Sphere_packet {
	edF32VECTOR3 field_0x0;
	ushort field_0xc;
	undefined field_0xe;
	undefined field_0xf;
}
);

PACK(
	struct ed_3d_strip {
	uint flags_0x0;
	short materialIndex;
	short field_0x6;
	int vifListOffset;
	int pNext; // ed_3d_strip*
	edF32VECTOR4 boundingSphere;
	int field_0x20; // char*
	int field_0x24; // char*
	int field_0x28; // char*
	int field_0x2c; // char*
	short bUseShadowMatrix_0x30;
	undefined field_0x32;
	undefined field_0x33;
	int pDMA_Matrix; // ed_dma_matrix*
	byte field_0x38;
	byte cameraPanIndex;
	short meshSectionCount_0x3a;
	int pTextureDataMystery; // ed_Bound_Sphere_packet*
});

MeshTransformDataBase* gpCurHierarchy = NULL;
undefined* pOldFANGifTag = NULL;

ed_3d_hierarchy_node* gHierarchyManagerBuffer = NULL;
edNODE_MANAGER* gHierarchyManagerNodeManager = NULL;
edNODE* gHierarchyManagerFirstFreeNode = NULL;

byte BYTE_00448a5c = 1;
edpkt_data gFANbuffers;
edF32VECTOR4 gClipMulVector = { 0 };
edF32VECTOR4 gClipAddVector = { 0 };
edF32VECTOR4 gClipXY = { 0 };

byte BYTE_00448a70 = 1;
byte BYTE_00449418 = 0;
edpkt_data* gpPKTDataRefMasPath3 = NULL;

//uint g_StaticVideoList_00489590[0xB20] = { 0 };
//ulong* g_StaticVideoList_00489590_LONG = (ulong*)g_StaticVideoList_00489590;

edNODE_MANAGER* pNodeManRender = NULL;
edNODE_MANAGER* pNodeManMaterial[2] = { NULL, NULL };
edNODE_MANAGER* gNodeDmaMatrix = NULL;
edNODE_MANAGER* gNodeDmaStrip = NULL;

edLIST gPrim_List[15][2] = { };

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

edpkt_data gRefOptionsforVU1Buf[3];

edF32VECTOR4 Vector_00424ff0 = { 0 };

int INT_004489a4 = 0;
int INT_004489a8 = 0;

uint ed3DVU1BufferCur = 0;
uint ed3DVU1Buffer[3] = { 0 };
byte BYTE_004493b4 = 0;
byte BYTE_004493b8 = 0;

edpkt_data gOptionFlagCNT[3];

void ed3DOptionFlagCNTInit(void)
{
	gOptionFlagCNT[0].cmdB = 0;
	gOptionFlagCNT[1].cmdB = 0;
	gOptionFlagCNT[2].cmdB = 0;
	gOptionFlagCNT[0].asU32[3] = SCE_VIF1_SET_UNPACK(ed3DVU1Buffer[0], 0x1, UNPACK_V4_32_MASKED, 0);
	gOptionFlagCNT[1].asU32[3] = SCE_VIF1_SET_UNPACK(ed3DVU1Buffer[1], 0x1, UNPACK_V4_32_MASKED, 0);
	gOptionFlagCNT[2].asU32[3] = SCE_VIF1_SET_UNPACK(ed3DVU1Buffer[2], 0x1, UNPACK_V4_32_MASKED, 0);
	gOptionFlagCNT[0].cmdA = ED_VIF1_SET_TAG_CNT(1);
	gOptionFlagCNT[1].cmdA = ED_VIF1_SET_TAG_CNT(1);
	gOptionFlagCNT[2].cmdA = ED_VIF1_SET_TAG_CNT(1);
	return;
}

edpkt_data g_stVertexXYZHeader;
edpkt_data g_stVertexRGBAHeader;
edpkt_data g_stVertexOptionFlagHeader;
edpkt_data g_stVertexXYZKeepADCHeader;
edpkt_data g_stVertexMultiSTHeader;
edpkt_data g_stSpriteVertexXYZHeader;
edpkt_data g_stNormalHeader;
edpkt_data g_stSpriteVertexRGBAHeader;
edpkt_data g_stShapeVertexXYZHeader;
edpkt_data g_stSpriteVertexGIFHeader;
edpkt_data g_stShapeVertexRGBAHeader;
edpkt_data g_stSpriteVertexSTHeader[2];
edpkt_data g_stSpriteWidthHeightHeader[2];
edpkt_data g_stShapeVertexSTHeader[2];

edpkt_data g_stGifTAG_Texture_NoFog[97];
edpkt_data g_stGifTAG_Gouraud_NoFog[97];
edpkt_data g_stGifTAG_Flat_NoFog[97];

edpkt_data g_stGifTAG_FAN_Texture_NoFog;
edpkt_data g_stGifTAG_FAN_Gouraud_NoFog;
edpkt_data g_stGifTAG_FAN_Flat_NoFog;

edpkt_data g_stGifTAG_Texture_NoFog_LINE[97];
edpkt_data g_stGifTAG_Gouraud_NoFog_LINE[97];
edpkt_data g_stGifTAG_Flat_NoFog_POINT[97];
edpkt_data g_stGifTAG_Texture_NoFog_POINT[97];

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
		g_stGifTAG_Texture_NoFog[iVar1].cmdA = uVar3 | 0x302e400000008000;
		g_stGifTAG_Texture_NoFog[iVar1].cmdB = 0x512;
		g_stGifTAG_Gouraud_NoFog[iVar1].cmdA = uVar3 | 0x3026400000008000;
		g_stGifTAG_Gouraud_NoFog[iVar1].cmdB = 0x512;
		g_stGifTAG_Flat_NoFog[iVar1].cmdA = uVar3 | 0x3022400000008000;
		g_stGifTAG_Flat_NoFog[iVar1].cmdB = 0x512;
	} while (uVar2 < 0x61);
	g_stGifTAG_FAN_Texture_NoFog.cmdA = 0x302ec00000008000;
	g_stGifTAG_FAN_Gouraud_NoFog.cmdA = 0x3026c00000008000;
	g_stGifTAG_FAN_Texture_NoFog.cmdB = 0x512;
	uVar2 = 0;
	g_stGifTAG_FAN_Flat_NoFog.cmdA = 0x3022c00000008000;
	g_stGifTAG_FAN_Gouraud_NoFog.cmdB = 0x512;
	g_stGifTAG_FAN_Flat_NoFog.cmdB = 0x512;
	do {
		iVar1 = (int)uVar2;
		uVar3 = uVar2 & 0xffffffff;
		uVar2 = (iVar1 + 1);
		g_stGifTAG_Texture_NoFog_LINE[iVar1].cmdA = uVar3 | 0x302d400000008000;
		g_stGifTAG_Texture_NoFog_LINE[iVar1].cmdB = 0x512;
		g_stGifTAG_Gouraud_NoFog_LINE[iVar1].cmdA = uVar3 | 0x3025400000008000;
		g_stGifTAG_Gouraud_NoFog_LINE[iVar1].cmdB = 0x512;
	} while (uVar2 < 0x61);
	uVar2 = 0;
	do {
		iVar1 = (int)uVar2;
		uVar3 = uVar2 & 0xffffffff;
		uVar2 = (iVar1 + 1);
		g_stGifTAG_Flat_NoFog_POINT[iVar1].cmdA = uVar3 | 0x3020400000008000;
		g_stGifTAG_Flat_NoFog_POINT[iVar1].cmdB = 0x512;
		g_stGifTAG_Texture_NoFog_POINT[iVar1].cmdA = uVar3 | 0x3028400000008000;
		g_stGifTAG_Texture_NoFog_POINT[iVar1].cmdB = 0x512;
	} while (uVar2 < 0x61);
	g_stVertexOptionFlagHeader.asU32[3] = 0;
	g_stVertexGIFHeader.asU32[3] = 0;
	g_stMatrixHeader.asU32[0] = SCE_VIF1_SET_STCYCL(1, 1, 0);
	g_stVertexOptionFlagHeader.asU32[0] = SCE_VIF1_SET_STCYCL(1, 1, 0);
	g_stMatrixHeader.asU32[1] = SCE_VIF1_SET_STMASK(0);
	g_stVertexOptionFlagHeader.asU32[1] = SCE_VIF1_SET_STMASK(0);
	g_stVertexGIFHeader.asU32[0] = SCE_VIF1_SET_STCYCL(1, 3, 0);
	g_stMatrixHeader.cmdB = 0xffffff00;
	g_stVertexXYZKeepADCHeader.asU32[2] = 0xffffffc0;
	g_stVertexOptionFlagHeader.asU32[2] = 0xffffff00;
	g_stVertexGIFHeader.asU32[1] = SCE_VIF1_SET_STMASK(0);
	g_stVertexGIFHeader.asU32[2] = 0xffffff00;
	g_stVertexMultiSTHeader.asU32[2] = 0xfffffff0;
	g_stSpriteVertexXYZHeader.asU32[2] = 0xffffff40;
	g_stSpriteWidthHeightHeader[1].asVector.z = -1.0f;
	g_stSpriteWidthHeightHeader[1].asU32[3] = 0xc000;
	ed3DVU1Buffer[0] = 0x76;
	g_stVertexXYZHeader.asU32[0] = SCE_VIF1_SET_STCYCL(1, 3, 0);
	ed3DVU1Buffer[1] = 0x198;
	g_stVertexXYZHeader.asU32[1] = SCE_VIF1_SET_STMASK(0);
	ed3DVU1Buffer[2] = 0x2ba;
	g_stVertexXYZHeader.asU32[2] = 0xffffff00;
	ed3DVU1BufferCur = 1;
	g_stVertexXYZHeader.asU32[3] = 0;
	gRefOptionsforVU1Buf[0].asU32[2] = SCE_VIF1_SET_BASE(0x0077, 0);
	gRefOptionsforVU1Buf[0].asU32[3] = SCE_VIF1_SET_OFFSET(0, 0);
	gRefOptionsforVU1Buf[1].asU32[2] = SCE_VIF1_SET_BASE(0x0199, 0);
	gRefOptionsforVU1Buf[1].asU32[3] = SCE_VIF1_SET_OFFSET(0, 0);
	gRefOptionsforVU1Buf[2].asU32[2] = SCE_VIF1_SET_BASE(0x02bb, 0);
	gRefOptionsforVU1Buf[2].asU32[3] = SCE_VIF1_SET_OFFSET(0, 0);
	g_stVertexXYZKeepADCHeader.asU32[0] = SCE_VIF1_SET_STCYCL(1, 3, 0);
	g_stVertexXYZKeepADCHeader.asU32[1] = SCE_VIF1_SET_STMASK(0);
	g_stVertexXYZKeepADCHeader.asU32[3] = 0;
	g_stVertexRGBAHeader.asU32[0] = SCE_VIF1_SET_STCYCL(1, 3, 0);
	g_stVertexRGBAHeader.asU32[1] = SCE_VIF1_SET_STMASK(0);
	g_stVertexRGBAHeader.asU32[2] = 0xffffff00;
	g_stVertexRGBAHeader.asU32[3] = 0;
	g_stVertexSTHeader[0].asU32[0] = SCE_VIF1_SET_STCYCL(1, 3, 0);
	g_stVertexSTHeader[0].asU32[1] = SCE_VIF1_SET_STMASK(0);
	g_stVertexSTHeader[0].asU32[2] = 0xffffff50;
	g_stVertexSTHeader[0].asU32[3] = ED_VIF1_SET_TAG_REF(0, 0);
	g_stVertexSTHeader[1].asVector.x = 0.0f;
	g_stVertexSTHeader[1].asVector.y = 0.0f;
	g_stVertexSTHeader[1].asVector.z = 1.0f;
	g_stVertexSTHeader[1].asVector.w = 0.0f;
	g_stVertexMultiSTHeader.asU32[0] = SCE_VIF1_SET_STCYCL(1, 3, 0);
	g_stVertexMultiSTHeader.asU32[1] = SCE_VIF1_SET_STMASK(0);
	g_stVertexMultiSTHeader.asU32[3] = 0;
	g_stNormalHeader.asU32[0] = SCE_VIF1_SET_STCYCL(1, 1, 0);
	g_stNormalHeader.asU32[1] = SCE_VIF1_SET_STMASK(0);
	g_stNormalHeader.asU32[2] = 0xffffff00;
	g_stNormalHeader.asU32[3] = 0;
	g_stSpriteVertexXYZHeader.asU32[0] = SCE_VIF1_SET_STCYCL(1, 3, 0);
	g_stSpriteVertexXYZHeader.asU32[1] = SCE_VIF1_SET_STMASK(0);
	g_stSpriteVertexXYZHeader.asU32[3] = 0;
	g_stSpriteVertexRGBAHeader.asU32[0] = SCE_VIF1_SET_STCYCL(1, 3, 0);
	g_stSpriteVertexRGBAHeader.asU32[1] = SCE_VIF1_SET_STMASK(0);
	g_stSpriteVertexRGBAHeader.asU32[2] = 0xffffff00;
	g_stSpriteVertexRGBAHeader.asU32[3] = 0;
	g_stSpriteWidthHeightHeader[0].asU32[0] = SCE_VIF1_SET_STCYCL(1, 1, 0);
	g_stSpriteWidthHeightHeader[0].asU32[1] = SCE_VIF1_SET_STMASK(0);
	g_stSpriteWidthHeightHeader[0].asU32[2] = 0xffffff50;
	g_stSpriteWidthHeightHeader[0].asU32[3] = ED_VIF1_SET_TAG_REF(0, 0);
	g_stSpriteWidthHeightHeader[1].asU32[0] = 0;
	g_stSpriteWidthHeightHeader[1].asU32[1] = 0;
	g_stSpriteVertexSTHeader[0].asU32[0] = SCE_VIF1_SET_STCYCL(1, 3, 0);
	g_stSpriteVertexGIFHeader.asU32[0] = SCE_VIF1_SET_STCYCL(1, 3, 0);
	g_stSpriteVertexSTHeader[0].asU32[1] = SCE_VIF1_SET_STMASK(0);
	g_stSpriteVertexSTHeader[0].asU32[2] = 0xffffff50;
	g_stShapeVertexSTHeader[0].asU32[2] = 0xffffff50;
	g_stSpriteVertexSTHeader[0].asU32[3] = ED_VIF1_SET_TAG_REF(0, 0);
	g_stSpriteVertexSTHeader[1].asU32[0] = 0;
	g_stSpriteVertexSTHeader[1].asU32[1] = 0;
	g_stSpriteVertexSTHeader[1].asU32[2] = 0x3f800000;
	g_stShapeVertexSTHeader[1].asU32[2] = 0x3f800000;
	g_stSpriteVertexSTHeader[1].asU32[3] = 0;
	g_stSpriteVertexGIFHeader.asU32[1] = SCE_VIF1_SET_STMASK(0);
	g_stSpriteVertexGIFHeader.asU32[2] = 0xffffff00;
	g_stSpriteVertexGIFHeader.asU32[3] = 0;
	g_stShapeVertexXYZHeader.asU32[0] = SCE_VIF1_SET_STCYCL(1, 1, 0);
	g_stShapeVertexXYZHeader.asU32[1] = SCE_VIF1_SET_STMASK(0);
	g_stShapeVertexXYZHeader.asU32[2] = 0xffffff00;
	g_stShapeVertexRGBAHeader.asU32[2] = 0xffffff00;
	g_stShapeVertexXYZHeader.asU32[3] = 0;
	g_stShapeVertexRGBAHeader.asU32[0] = SCE_VIF1_SET_STCYCL(1, 1, 0);
	g_stShapeVertexSTHeader[0].asU32[0] = SCE_VIF1_SET_STCYCL(1, 1, 0);
	g_stShapeVertexRGBAHeader.asU32[1] = SCE_VIF1_SET_STMASK(0);
	g_stShapeVertexSTHeader[0].asU32[1] = SCE_VIF1_SET_STMASK(0);
	g_stShapeVertexRGBAHeader.asU32[3] = 0;
	g_stShapeVertexSTHeader[0].asU32[3] = ED_VIF1_SET_TAG_REF(0, 0);
	g_stShapeVertexSTHeader[1].asU32[0] = 0;
	g_stShapeVertexSTHeader[1].asU32[1] = 0;
	g_stShapeVertexSTHeader[1].asU32[3] = 0;
	gRefOptionsforVU1Buf[0].cmdA = ED_VIF1_SET_TAG_REF(0, 0);
	gRefOptionsforVU1Buf[1].cmdA = ED_VIF1_SET_TAG_REF(0, 0);
	gRefOptionsforVU1Buf[2].cmdA = ED_VIF1_SET_TAG_REF(0, 0);
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

struct ed_dma_matrix : public edLIST {
	edF32MATRIX4* pObjToWorld;
	MeshTransformDataBase* pMeshTransformData;
	int flags_0x28;
	float normalScale;
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
	union edF32MATRIX4* pTransformMatrix;
	struct ed_3d_hierarchy_node* pMeshTransformData;
	int flags_0x28;
	float field_0x2c;
};

struct ed_dma_material {
	ed_g2d_material* pMaterial;
	undefined4 field_0x4;
	edLIST list;
	uint flags;
	ed_g2d_bitmap* pBitmap;
};


ed_dma_material* DmaMaterialBuffer[2] = { 0 };
ed_dma_material* DmaMaterialBufferMax[2] = { 0 };
ed_dma_material* DmaMaterialBufferCurrent = NULL;
ed_dma_material* DmaMaterialBufferCurrentMax = NULL;

ed_dma_matrix* DmaMatrixBuffer = NULL;
ed_dma_matrix* DmaMatrixBufferMax = NULL;
ed_dma_matrix* DmaMatrixBufferCurrent = NULL;
ed_dma_matrix* DmaMatrixBufferCurrentMax = NULL;
ed_dma_matrix* DmaMatrixDefault = NULL;

byte bTexInvalid = 0;
int gVRAMBufferDMA = 0;
edNODE* gFlushTexCurrent = NULL;
ed_g2d_material* previous_flushDMAMat = NULL;
edLIST* gFlushTexLast = NULL;
edNODE* gPrim_List_FlushTex_Last = NULL;

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
	DmaMatrixDefault = DmaMatrixBuffer;
	MeshTransformDataBase_0048cad0.pMatrixPkt = (edF32MATRIX4*)0x0;
	DmaMatrixBufferCurrentMax = DmaMatrixBufferMax;
	gNodeDmaMatrix = pNodeManRender;
	gNodeDmaStrip = pNodeManRender;
	DmaMatrixBufferCurrent = DmaMatrixBuffer + 1;
	DmaMatrixBuffer->pObjToWorld = &gF32Matrix4Unit;
	DmaMatrixDefault->pMeshTransformData = &MeshTransformDataBase_0048cad0;
	DmaMatrixDefault->normalScale = 1.0;
	(DmaMatrixDefault)->pNext = (edNODE*)DmaMatrixDefault;
	(DmaMatrixDefault)->pPrev = (edNODE*)DmaMatrixDefault;
	(DmaMatrixDefault)->pData = gNodeDmaStrip;
	(DmaMatrixDefault)->field_0x10 = 0;
	(DmaMatrixDefault)->nodeCount = 0;
	(DmaMatrixDefault)->header.mode = 0;
	DmaMatrixDefault->pMeshTransformData = &MeshTransformDataBase_0048cad0;
	return;
}

void ed3DG2DMaterialGetLayerBitmap(ed_dma_material* pMaterial, ed_g2d_bitmap** ppOutTEX2D, ed_g2d_layer** ppOutLAY, int index)
{
	int* hash = (int*)(pMaterial->pMaterial + 1);
	*ppOutLAY = &((ed_g2d_layer_header*)LOAD_SECTION(hash[index]))->body;
	TextureData_TEX* pTEX = (TextureData_TEX*)LOAD_SECTION((*ppOutLAY)->pTex);
	TextureData_HASH_Internal_PA32* b = (TextureData_HASH_Internal_PA32*)LOAD_SECTION(pTEX->body.hashCode.pData);
	TextureData_PA32* pTEX2D = (TextureData_PA32*)LOAD_SECTION(b->pPA32);
	*ppOutTEX2D = &pTEX2D->body;
	return;
}

void ed3DFlushTexReset(edLIST* pList)
{
	ed_g2d_bitmap* peVar1;
	edpkt_data* peVar2;
	edpkt_data* pPkt;
	ed_dma_material* pMVar1;

	if (bTexInvalid == 0x3c) {
		bTexInvalid = 0;
	}
	gVRAMBufferDMA = 0;
	bTexInvalid = bTexInvalid + 1;
	gFlushTexCurrent = pList->pPrev;
	do {
		pMVar1 = (ed_dma_material*)gFlushTexCurrent->pData;
		if (pMVar1 != (ed_dma_material*)0x0) {
			previous_flushDMAMat = pMVar1->pMaterial;
		}
		gFlushTexCurrent = gFlushTexCurrent->pPrev;
	} while (pMVar1->pBitmap == (ed_g2d_bitmap*)0xdfdfdfdf);
	gFlushTexLast = pList;
	if ((pMVar1->flags & 2) != 0) {
		ed3DFXClearALLFXSendedTOVRAM();
		pMVar1->flags = pMVar1->flags & 0xfffffffd;
	}
	if (pMVar1->pMaterial->count_0x0 < 2) {
		peVar1 = pMVar1->pBitmap;
		if ((peVar1 != (ed_g2d_bitmap*)0x0) && (peVar1->psm != 0)) {
			edPSX2Header* pHeader = ((edPSX2Header*)LOAD_SECTION(peVar1->pPSX2)) + gVRAMBufferDMA;
			pPkt = (edpkt_data*)LOAD_SECTION(pHeader->pPkt);
			edDmaFlushCache();
			RENDER_LOG("DMA Begin ed3DFlushTexReset");
			edDmaSend_nowait(SHELLDMA_CHANNEL_GIF, (ulonglong*)pPkt);

			// We send the first material above. This is why gbFirstTex exists.
		}
	}
	else {
		IMPLEMENTATION_GUARD(
			peVar2 = ed3DFlushMultiTexture(pMVar1, (edpkt_data*)gPKTMultiTex, 1, gVRAMBufferDMA);
		if (peVar2 != (edpkt_data*)0x0) {
			edDmaFlushCache();
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
#ifdef PLATFORM_PS2
		DPUT_VIF1_FIFO(*(u_long128*)&g_VIF1_FIFO_00424fe0[0]);
#endif
		edDmaFlushCache();
		RENDER_LOG("DMA Begin ed3DFlushTexPrepareDMA");
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
	if ((edLIST*)gPrim_List_FlushTex[gCurFlushList].pPrev != gPrim_List_FlushTex + gCurFlushList) {
		ed3DFlushTexPrepareDMA(gPrim_List_FlushTex + gCurFlushList);
	}
#ifdef PLATFORM_PS2
	DPUT_D_PCR(g_Model_PCR_00449240);
#endif
	if ((g_pStartPktData != (edpkt_data*)0x0) && (g_pStartPktData != g_VifRefPktCur)) {
		edDmaFlushCache();
		RENDER_LOG("DMA Begin ed3DFlushSendDMA3D");
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

edpkt_data g_PKTHeaderRef[9] = {};

void ed3DDMAGeneratePacketRefHeader(void)
{
	g_PKTHeaderRef[0].cmdA = ED_VIF1_SET_TAG_REF(1, &g_stVertexGIFHeader);
	g_PKTHeaderRef[0].cmdB = 0;
	g_PKTHeaderRef[1].cmdA = ED_VIF1_SET_TAG_REF(2, g_stVertexSTHeader);
	g_PKTHeaderRef[1].cmdB = 0;
	g_PKTHeaderRef[2].cmdA = ED_VIF1_SET_TAG_REF(1, &g_stVertexRGBAHeader);
	g_PKTHeaderRef[2].cmdB = 0;
	g_PKTHeaderRef[3].cmdA = ED_VIF1_SET_TAG_REF(1, &g_stVertexXYZHeader);
	g_PKTHeaderRef[3].cmdB = 0;
	g_PKTHeaderRef[4].cmdA = ED_VIF1_SET_TAG_REF(1, &g_stNormalHeader);
	g_PKTHeaderRef[4].cmdB = 0;
	g_PKTHeaderRef[5].cmdA = ED_VIF1_SET_TAG_REF(1, &g_stVertexOptionFlagHeader);
	g_PKTHeaderRef[5].cmdB = 0;
	g_PKTHeaderRef[6].cmdA = ED_VIF1_SET_TAG_REF(1, &g_stVertexMultiSTHeader);
	g_PKTHeaderRef[6].cmdB = 0;
	g_PKTHeaderRef[7].cmdA = ED_VIF1_SET_TAG_REF(1, &g_stVertexXYZKeepADCHeader);;
	g_PKTHeaderRef[7].cmdB = 0;
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
	g_PKTSpriteHeaderRef[0].cmdA = ED_VIF1_SET_TAG_REF(1, &g_stSpriteVertexGIFHeader);
	g_PKTSpriteHeaderRef[0].cmdB = 0;
	g_PKTSpriteHeaderRef[1].cmdA = ED_VIF1_SET_TAG_REF(2, g_stSpriteVertexSTHeader);
	g_PKTSpriteHeaderRef[1].cmdB = 0;
	g_PKTSpriteHeaderRef[2].cmdA = ED_VIF1_SET_TAG_REF(1, &g_stSpriteVertexRGBAHeader);
	g_PKTSpriteHeaderRef[2].cmdB = 0;
	g_PKTSpriteHeaderRef[3].cmdA = ED_VIF1_SET_TAG_REF(1, &g_stSpriteVertexXYZHeader);
	g_PKTSpriteHeaderRef[3].cmdB = 0;
	g_PKTSpriteHeaderRef[4].cmdA = ED_VIF1_SET_TAG_REF(2, g_stSpriteWidthHeightHeader);
	g_PKTSpriteHeaderRef[4].cmdB = 0;
	return;
}

edpkt_data g_stExecuteCode;

void ed3DDMAInit(uint countA, int countB)
{
	char* pcVar1;
	char* pcVar2;
	ulong lVar3;

	gFANbuffers.asU32[0] = 0x22;
	gFANbuffers.asU32[3] = 0xc000;
	lVar3 = (ulong)ed3D_Allocator_00449248.end;
	gFANbuffers.asU32[1] = 0x3e;
	gFANbuffers.asU32[2] = 0x5a;
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
	g_stExecuteCode.cmdA = 0;
	g_stExecuteCode.cmdB = 0;
	ed3DDMAGeneratePacketRefHeader();
	ed3DDMAGenerateSpritePacketRefHeader();
	g_PKTShapeHeaderRef[0].cmdB = 0;
	g_PKTShapeHeaderRef[1].cmdB = 0;
	g_PKTShapeHeaderRef[2].cmdB = 0;
	g_PKTShapeHeaderRef[0].cmdA = ED_VIF1_SET_TAG_REF(1, &g_stShapeVertexXYZHeader);
	g_PKTShapeHeaderRef[1].cmdA = ED_VIF1_SET_TAG_REF(2, g_stShapeVertexSTHeader);
	g_PKTShapeHeaderRef[2].cmdA = ED_VIF1_SET_TAG_REF(1, &g_stShapeVertexRGBAHeader);
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
#ifdef PLATFORM_PS2
	g_RenderCommandVidMem_004492a0 = (edpkt_data*)(((ulong)(lVar3 << 0x24) >> 0x24) | 0x30000000);
#else
	g_RenderCommandVidMem_004492a0 = (edpkt_data*)malloc(0x1000);
#endif
	g_GifRefPktCur = g_RenderCommandVidMem_004492a0;
	g_GifRefPktCurMax = g_RenderCommandVidMem_004492a0 + countB;
	return;
}

ed_g2d_material* previous_flush3DMat = NULL;
ed_dma_material* gpLastMaterial = NULL;
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
	gpLastMaterial = (ed_dma_material*)0x0;
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

ed_g2d_bitmap* ed3DGetG2DBitmap(ed_dma_material* pDMA_Material, int index)
{
	ed_g2d_material* peVar1;
	ed_g2d_bitmap* pcVar1;

	peVar1 = pDMA_Material->pMaterial;
	if ((peVar1 == (ed_g2d_material*)0x0) || (peVar1->count_0x0 == 0)) {
		pcVar1 = (ed_g2d_bitmap*)0x0;
	}
	else {
		IMPLEMENTATION_GUARD(pcVar1 = ed3DGetG2DBitmap(peVar1, index));
	}
	return pcVar1;
}

char DAT_0048c3b0[200] = { 0 };
char DAT_0048c3c0[200] = { 0 };

ed_dma_matrix* pprevious_shadow_dma_matrix = NULL;

ed_g2d_material gDefault_Material_Gouraud = { 0 };
ed_g2d_material gDefault_Material_Gouraud_Cluster = { 0 };
ed_g2d_material gDefault_Material_Flat = { 0 };
ed_g2d_material gDefault_Material_Flat_Cluster = { 0 };
ed_g2d_material gMaterial_Render_Zbuffer_Only = { 0 };
ed_g2d_material gMaterial_Render_Zbuffer_Only_Cluster = { 0 };

void ed3DShadowResetPrimList(void)
{
	edLIST* pInternalMeshTransformSpecial;
	ed_g2d_bitmap* pcVar3;
	edNODE* peVar1;
	edNODE_MANAGER* pvVar1;
	edNODE_MANAGER* pMVar1;
	ed_dma_material* pMeshCurrrent;
	ed_g2d_material** ppFileData;

	pMeshCurrrent = DmaMaterialBufferCurrent;
	pInternalMeshTransformSpecial = &DmaMaterialBufferCurrent->list;
	ppFileData = &DmaMaterialBufferCurrent->pMaterial;
	DmaMaterialBufferCurrent = DmaMaterialBufferCurrent + 1;
	*ppFileData = &gMaterial_Render_Zbuffer_Only;
	pMeshCurrrent->field_0x4 = 0;
	pMeshCurrrent->flags = pMeshCurrrent->flags & 0xfffffffe;
	(pMeshCurrrent->list).pNext = (edNODE*)pInternalMeshTransformSpecial;
	(pMeshCurrrent->list).pPrev = (edNODE*)pInternalMeshTransformSpecial;
	(pMeshCurrrent->list).pData = gNodeDmaMatrix;
	(pMeshCurrrent->list).field_0x10 = 0;
	(pMeshCurrrent->list).nodeCount = 0;
	(pMeshCurrrent->list).header.mode = 0;
	pcVar3 = ed3DGetG2DBitmap(pMeshCurrrent, 0);
	pMeshCurrrent->pBitmap = pcVar3;
	pMVar1 = (edNODE_MANAGER*)gPrim_List_Shadow[gCurRenderList].pData;
	peVar1 = pMVar1->pNodeHead + pMVar1->linkCount;
	peVar1->pData = pMeshCurrrent;
	peVar1->header.typeField.type = 1;
	pMVar1->linkCount = pMVar1->linkCount + 1;
	pInternalMeshTransformSpecial = gPrim_List_Shadow + gCurRenderList;
	gPrim_List_Shadow[gCurRenderList].nodeCount = gPrim_List_Shadow[gCurRenderList].nodeCount + 1;
	peVar1->pPrev = pInternalMeshTransformSpecial->pPrev;
	gPrim_List_Shadow[gCurRenderList].pPrev = peVar1;
	pMeshCurrrent = DmaMaterialBufferCurrent;
	pInternalMeshTransformSpecial = &DmaMaterialBufferCurrent->list;
	ppFileData = &DmaMaterialBufferCurrent->pMaterial;
	DmaMaterialBufferCurrent = DmaMaterialBufferCurrent + 1;
	*ppFileData = &gMaterial_Render_Zbuffer_Only_Cluster;
	pMeshCurrrent->field_0x4 = 0;
	pMeshCurrrent->flags = pMeshCurrrent->flags & 0xfffffffe;
	(pMeshCurrrent->list).pNext = (edNODE*)pInternalMeshTransformSpecial;
	(pMeshCurrrent->list).pPrev = (edNODE*)pInternalMeshTransformSpecial;
	(pMeshCurrrent->list).pData = gNodeDmaMatrix;
	(pMeshCurrrent->list).field_0x10 = 0;
	(pMeshCurrrent->list).nodeCount = 0;
	(pMeshCurrrent->list).header.mode = 0;
	pMeshCurrrent->flags = pMeshCurrrent->flags | 1;
	pcVar3 = ed3DGetG2DBitmap(pMeshCurrrent, 0);
	pMeshCurrrent->pBitmap = pcVar3;
	pvVar1 = (edNODE_MANAGER*)gPrim_List_Shadow[gCurRenderList].pData;
	peVar1 = pvVar1->pNodeHead + pvVar1->linkCount;
	peVar1->pData = pMeshCurrrent;
	peVar1->header.typeField.type = 1;
	pvVar1->linkCount = pvVar1->linkCount + 1;
	pInternalMeshTransformSpecial = gPrim_List_Shadow + gCurRenderList;
	gPrim_List_Shadow[gCurRenderList].nodeCount = gPrim_List_Shadow[gCurRenderList].nodeCount + 1;
	peVar1->pPrev = pInternalMeshTransformSpecial->pPrev;
	gPrim_List_Shadow[gCurRenderList].pPrev = peVar1;
	pprevious_shadow_dma_matrix = (ed_dma_matrix*)0x0;
	return;
}

int gCurSceneID = 0;

void ed3DPrimlistInitMaterialRenderList(void)
{
	edLIST* peVar1;
	edNODE_MANAGER** ppeVar2;

	ppeVar2 = pNodeManMaterial + gCurRenderList;
	peVar1 = gPrim_List_FlushTex + gCurRenderList;
	gPrim_List_FlushTex[gCurRenderList].pPrev = (edNODE*)(gPrim_List_FlushTex + gCurRenderList);
	peVar1->nodeCount = 0;
	gPrim_List_FlushTex_Last = (edNODE*)0x0;
	(*ppeVar2)->linkCount = 0;
	DmaMaterialBufferCurrent = DmaMaterialBuffer[gCurRenderList];
	DmaMaterialBufferCurrentMax = DmaMaterialBufferMax[gCurRenderList];
	return;
}


void UpdateRenderMeshTransform_002954b0(ed_3d_hierarchy_node* param_1, bool mode)
{
	ed_3d_hierarchy_node* v1;
	ed_3d_hierarchy_node* v1_00;
	edF32MATRIX4* v1_01;
	edF32MATRIX4* v1_02;
	edF32MATRIX4* v1_03;
	edF32MATRIX4* v1_04;
	ed_3d_hierarchy_node* pMeshTransformData;
	ed_3d_hierarchy_node* pMVar1;
	ulong uVar2;

	if ((mode != false) && ((param_1->base).subMeshParentCount_0xac != 0)) {
		return;
	}
	if ((param_1->base).field_0x88 == 0) {
		return;
	}
	v1 = (ed_3d_hierarchy_node*)(param_1->base).pLinkTransformData;
	if (v1 == (ed_3d_hierarchy_node*)0x0) {
		if (((param_1->base).flags_0x9e & 1) == 0) {
			edF32Matrix4CopyHard(&(param_1->base).transformB, &param_1->base.transformA);
		}
		goto LAB_002957c4;
	}
	if ((v1->base).field_0x88 != '\0') {
		v1_00 = (ed_3d_hierarchy_node*)(v1->base).pLinkTransformData;
		if (v1_00 == (ed_3d_hierarchy_node*)0x0) {
			if (((v1->base).flags_0x9e & 1) == 0) {
				edF32Matrix4CopyHard(&(v1->base).transformB, &v1->base.transformA);
			}
		}
		else {
			if (*(char*)((int)(v1_00->base).hash.number + 8) != '\0') {
				IMPLEMENTATION_GUARD(
					v1_01 = *(edF32MATRIX4*)((int)(v1_00->base).hash.number + 0x10);
				if (v1_01 == (edF32MATRIX4*)0x0) {
					if ((*(ushort*)((int)(v1_00->base).hash.number + 0x1e) & 1) == 0) {
						edF32Matrix4CopyHard(&(v1_00->base).transformB, v1_00);
					}
				}
				else {
					if (*(char*)(v1_01[2] + 2) != '\0') {
						v1_02 = (edF32MATRIX4*)v1_01[2][1][0];
						if (v1_02 == (edF32MATRIX4*)0x0) {
							if ((*(ushort*)((int)v1_01[2][1] + 0xe) & 1) == 0) {
								edF32Matrix4CopyHard(v1_01[1], v1_01);
							}
						}
						else {
							if (*(char*)(v1_02[2] + 2) != '\0') {
								v1_03 = v1_02[2][1][0];
								if (v1_03 == 0x0) {
									if ((*(ushort*)((int)v1_02[2][1] + 0xe) & 1) == 0) {
										edF32Matrix4CopyHard(v1_02[1], v1_02);
									}
								}
								else {
									if (*(char*)(v1_03[2] + 2) != '\0') {
										v1_04 = v1_03[2][1][0];
										if (v1_04 == 0x0) {
											if ((*(ushort*)((int)v1_03[2][1] + 0xe) & 1) == 0) {
												edF32Matrix4CopyHard(v1_03[1], v1_03);
											}
										}
										else {
											if (*(char*)(v1_04[2] + 2) != '\0') {
												pMeshTransformData = (ed_3d_hierarchy_node*)v1_04[2][1][0];
												if (pMeshTransformData == (ed_3d_hierarchy_node*)0x0) {
													if ((*(ushort*)((int)v1_04[2][1] + 0xe) & 1) == 0) {
														edF32Matrix4CopyHard(v1_04[1], v1_04);
													}
												}
												else {
													uVar2 = GetParam1_0x88((int)pMeshTransformData);
													if (uVar2 != 0) {
														pMVar1 = (pMeshTransformData->base).pLinkTransformData;
														if (pMVar1 == (ed_3d_hierarchy_node*)0x0) {
															if (((pMeshTransformData->base).flags_0x9e & 1) == 0) {
																edF32Matrix4CopyHard(&(pMeshTransformData->base).transformB,
																	pMeshTransformData);
															}
														}
														else {
															UpdateRenderMeshTransform_002954b0(pMVar1, false);
															if (((pMeshTransformData->base).flags_0x9e & 1) == 0) {
																edF32Matrix4MulF32Matrix4Hard(&(pMeshTransformData->base).transformB,
																	pMeshTransformData, &(pMVar1->base).transformB);
															}
														}
														ed3DSetSceneRender(pMeshTransformData, 0);
													}
													if ((*(ushort*)((int)v1_04[2][1] + 0xe) & 1) == 0) {
														edF32Matrix4MulF32Matrix4Hard(v1_04[1], v1_04, &(pMeshTransformData->base).transformB);
														*(undefined*)(v1_04[2] + 2) = 0;
														goto LAB_00295648;
													}
												}
												*(undefined*)(v1_04[2] + 2) = 0;
											}
										LAB_00295648:
											if ((*(ushort*)((int)v1_03[2][1] + 0xe) & 1) == 0) {
												edF32Matrix4MulF32Matrix4Hard(v1_03[1], v1_03, v1_04[1]);
												*(undefined*)(v1_03[2] + 2) = 0;
												goto LAB_00295688;
											}
										}
										*(undefined*)(v1_03[2] + 2) = 0;
									}
								LAB_00295688:
									if ((*(ushort*)((int)v1_02[2][1] + 0xe) & 1) == 0) {
										edF32Matrix4MulF32Matrix4Hard(v1_02[1], v1_02, v1_03[1]);
										*(undefined*)(v1_02[2] + 2) = 0;
										goto LAB_002956c8;
									}
								}
								*(undefined*)(v1_02[2] + 2) = 0;
							}
						LAB_002956c8:
							if ((*(ushort*)((int)v1_01[2][1] + 0xe) & 1) == 0) {
								edF32Matrix4MulF32Matrix4Hard(v1_01[1], v1_01, v1_02[1]);
								*(undefined*)(v1_01[2] + 2) = 0;
								goto LAB_00295708;
							}
						}
						*(undefined*)(v1_01[2] + 2) = 0;
					}
				LAB_00295708:
					if ((*(ushort*)((int)(v1_00->base).field_0x80 + 0x1e) & 1) == 0) {
						edF32Matrix4MulF32Matrix4Hard(&(v1_00->base).transformB, v1_00, v1_01[1]);
						*(undefined*)((int)(v1_00->base).field_0x80 + 8) = 0;
						goto LAB_00295748;
					}
				}
				*(undefined*)((int)(v1_00->base).field_0x80 + 8) = 0;)
			}
		LAB_00295748:
			if (((v1->base).flags_0x9e & 1) == 0) {
				edF32Matrix4MulF32Matrix4Hard(&(v1->base).transformB, &v1->base.transformA, &(v1_00->base).transformB);
				(v1->base).field_0x88 = 0;
				goto LAB_00295788;
			}
		}
		(v1->base).field_0x88 = 0;
	}
LAB_00295788:
	if (((param_1->base).flags_0x9e & 1) == 0) {
		edF32Matrix4MulF32Matrix4Hard(&(param_1->base).transformB, &param_1->base.transformA, &(v1->base).transformB);
		(param_1->base).field_0x88 = 0;
		return;
	}
LAB_002957c4:
	(param_1->base).field_0x88 = 0;
	return;
}

void ed3DComputeSonHierarchy(void)
{
	edLIST* peVar1;
	int iVar2;
	uint uVar3;
	uint uVar4;
	edNODE* pCVar6;
	int* piVar5;
	ed_3D_Scene* peVar6;
	int iVar7;

	for (uVar3 = 0; uVar3 < (uint)ged3DConfig.sceneCount; uVar3 = uVar3 + 1) {
		peVar6 = gScene3D + uVar3;
		if (((peVar6->flags_0x4 & 1) != 0) && ((peVar6->flags_0x4 & 4) == 0)) {
			peVar1 = peVar6->pHeirListA;
			for (pCVar6 = peVar1->pPrev; (edLIST*)pCVar6 != peVar1; pCVar6 = pCVar6->pPrev) {
				UpdateRenderMeshTransform_002954b0((ed_3d_hierarchy_node*)pCVar6->pData, true);
			}
			piVar5 = peVar6->field_0x1ac;
			for (uVar4 = 0; uVar4 < peVar6->heirarchyListCount; uVar4 = uVar4 + 1) {
				IMPLEMENTATION_GUARD(
					iVar2 = *piVar5;
				for (iVar7 = *(int*)(iVar2 + 4); iVar7 != iVar2; iVar7 = *(int*)(iVar7 + 4)) {
					UpdateRenderMeshTransform_002954b0(*(ed_3d_hierarchy_node**)(iVar7 + 0xc), true);
				}
				piVar5 = piVar5 + 1;)
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
		RENDER_LOG("DMA Begin ed3DDMALoadVU1MicroCode");
		edDmaSend(SHELLDMA_CHANNEL_VIF1, (uint)vu1_micro);
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
	gRefOptionsforVU1Buf[0].cmdA = ED_VIF1_SET_TAG_REF(0, 0);
	gRefOptionsforVU1Buf[0].asU32[2] = SCE_VIF1_SET_BASE(0x0077, 0);
	gRefOptionsforVU1Buf[0].asU32[3] = SCE_VIF1_SET_OFFSET(0, 0);
	gRefOptionsforVU1Buf[1].cmdA = ED_VIF1_SET_TAG_REF(0, 0);
	gRefOptionsforVU1Buf[1].asU32[2] = SCE_VIF1_SET_BASE(0x0199, 0);
	gRefOptionsforVU1Buf[1].asU32[3] = SCE_VIF1_SET_OFFSET(0, 0);
	gRefOptionsforVU1Buf[2].cmdA = ED_VIF1_SET_TAG_REF(0, 0);
	gRefOptionsforVU1Buf[2].asU32[2] = SCE_VIF1_SET_BASE(0x02bb, 0);
	gRefOptionsforVU1Buf[2].asU32[3] = SCE_VIF1_SET_OFFSET(0, 0);
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
	pShadowConfig->pViewport = (ed_viewport*)0x0;
	pShadowConfig->renderMask = 0;
	pShadowConfig->field_0x22 = 0x30;
	pShadowConfig->field_0x23 = 0x14;
	pShadowConfig->field_0x24 = 2;
	pShadowConfig->field_0x18 = 0x80;
	pShadowConfig->field_0x1c = 0x80;
	return;
}

edF32VECTOR4 Vector_00424f10 = { 10.0f, 10.0f, 10.0f, 128.0f };
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

edF32VECTOR4* LightAmbiant_DEBUG = &Vector_00424f10;
edF32VECTOR4* LightDirections_Matrix_DEBUG = (edF32VECTOR4*)&Matrix_00424e90;
edF32VECTOR4* LightColor_Matrix_DEBUG = (edF32VECTOR4*)&Matrix_00424ed0;

void ed3DSceneLightConfigSetDefault(ed_3D_Light_Config* pLightConfig)
{
	pLightConfig->pLightAmbient = LightAmbiant_DEBUG;
	pLightConfig->pLightDirections = LightDirections_Matrix_DEBUG;
	pLightConfig->pLightColorMatrix = LightColor_Matrix_DEBUG;
	return;
}

void ed3DSceneConfigSetDefault(SceneConfig* pSceneConfig)
{
	pSceneConfig->clipValue_0x4 = 250.0;
	pSceneConfig->clipValue_0x18 = 250.0;
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

#ifdef EDITOR_BUILD
			char name[128];
			sprintf(name, "gScene3D[%d].pHeirListA", sceneIndex);
			edListSetName(gScene3D[sceneIndex].pHeirListA, name);
			sprintf(name, "gScene3D[%d].pHeirListB", sceneIndex);
			edListSetName(gScene3D[sceneIndex].pHeirListB, name);
#endif
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

struct RenderInfo {
	union edF32MATRIX4* pSharedMeshTransform;
	union edF32MATRIX4* pMeshTransformMatrix;
	int field_0x8;
	struct ed_3d_hierarchy_setup* pLightingMatrixFuncObj;
	uint flags_0x10;
	float field_0x14;
	undefined4 field_0x18;
	struct ed_3d_hierarchy_node* pMeshTransformData;
};

union AnimScratchpad {
	struct {
		int field_0x0;
		uint flags;
		undefined4 field_0x8;
		undefined4 field_0xc;
	};
	edpkt_data asPkt;
};

edF32MATRIX4 gCurLOD_WorldToCamera_Matrix = { 0 };

edF32VECTOR4* gLightAmbiant = NULL;
edF32VECTOR4* gLightDirections_Matrix = NULL;
edF32VECTOR4* gLightColor_Matrix = NULL;

void ed3DSceneLightSetGlobals(ed_3D_Light_Config* param_1)
{
	gLightAmbiant = param_1->pLightAmbient;
	if (gLightAmbiant != (edF32VECTOR4*)0x0) {
		gLightAmbiant->w = 0.0078125;
	}
	gLightDirections_Matrix = param_1->pLightDirections;
	gLightColor_Matrix = param_1->pLightColorMatrix;
	return;
}

edF32VECTOR4 gCamPos = { 0 };
edF32VECTOR4 gCamNormal_X = { 0 };
edF32VECTOR4 gCamNormal_Y = { 0 };
edF32VECTOR4 gCamNormal_Z = { 0 };

float gRenderInvFovCoef = 0.0f;
float gRenderFovCoef = 0.0f;
float gCurLOD_RenderFOVCoef = 0.0f;

edSurface gRenderSurface = { 0 };

SceneConfig* gRenderSceneConfig = NULL;

ed_3D_Scene* gRenderScene = NULL;

float fSecInc = 5.0f;

void ed3DViewportComputeViewMatrices(float screenWidth, float screenHeight, float width, float height, float horizontalHalfFOV, float halfFOV, float verticalHalfFOV,
	float nearClip, edF32MATRIX4* pCameraToScreen, edF32MATRIX4* pCameraToCulling, edF32MATRIX4* pCameraToClipping, edF32MATRIX4* pCameraToFog,
	float farClip, float fovUp, float fovDown)
{
	float fVar1;
	float fVar2;
	float fVar3;
	float startX;
	float fVar4;
	float startY;
	edF32MATRIX4 screenMatrix;
	edF32MATRIX4 projectionMatrix;

	char buff[512] = { 0 };
	sprintf(buff, "screenWidth: %f, screenHeight: %f, width: %f, height: %f, horizontalHalfFOV: %f, halfFOV: %f, verticalHalfFOV: %f, nearClip: %f, farClip: %f, fovUp: %f, fovDown: %f\n",
		screenWidth, screenHeight, width, height, horizontalHalfFOV, halfFOV, verticalHalfFOV, nearClip, farClip, fovUp, fovDown);

	// Print all the float values in one statement
	ED3D_LOG(LogLevel::Verbose, "%s", buff);

	ed3DComputeLocalToProjectionMatrix(0.0f, 0.0f, horizontalHalfFOV, 0.0f, &projectionMatrix);
	startY = -height;
	startX = -width;

	ed3DComputeProjectionToScreenMatrix
	(startX, width, startY, height, halfFOV - screenWidth * 0.5f, halfFOV + screenWidth * 0.5f,
		verticalHalfFOV + screenHeight * 0.5f, verticalHalfFOV - screenHeight * 0.5f, &screenMatrix);

	edF32Matrix4MulF32Matrix4Hard(pCameraToScreen, &projectionMatrix, &screenMatrix);
	fVar4 = 1.0f / (farClip - nearClip);
	fVar1 = pCameraToScreen->dd + pCameraToScreen->cd * nearClip;
	pCameraToScreen->cc = fVar4 * (fovDown * (pCameraToScreen->dd + pCameraToScreen->cd * farClip) - fovUp * fVar1);
	pCameraToScreen->dc = fVar4 * (fVar1 * farClip * fovUp - fVar1 * nearClip * fovDown);

	ed3DComputeProjectionToScreenMatrix(startX * 1.0f, width * 1.0f, startY * 1.0f, height * 1.0f, -1.0f, 1.0, -1.0f, 1.0f, &screenMatrix);

	edF32Matrix4MulF32Matrix4Hard(pCameraToCulling, &projectionMatrix, &screenMatrix);
	fVar2 = nearClip * 1.0f;
	fVar1 = pCameraToCulling->dd + pCameraToCulling->cd * nearClip;
	fVar3 = farClip * -1.0f;
	pCameraToCulling->cc = fVar4 * ((pCameraToCulling->dd + pCameraToCulling->cd * farClip) * 1.0f - fVar1 * -1.0f);
	pCameraToCulling->dc = fVar4 * (fVar1 * fVar3 - fVar1 * fVar2);

	ed3DComputeProjectionToScreenMatrix(startX * fSecInc, width * fSecInc, startY * fSecInc, height * fSecInc, -1.0f, 1.0f, -1.0f, 1.0f, &screenMatrix);

	edF32Matrix4MulF32Matrix4Hard(pCameraToClipping, &projectionMatrix, &screenMatrix);
	fVar1 = pCameraToClipping->dd + pCameraToClipping->cd * nearClip;
	pCameraToClipping->cc = fVar4 * ((pCameraToClipping->dd + pCameraToClipping->cd * farClip) * 1.0f - fVar1 * -1.0f);
	pCameraToClipping->dc = fVar4 * (fVar1 * fVar3 - fVar1 * fVar2);
	gClipMulVector.z = 2.0f / (fovDown - fovUp);
	gClipXY.y = (fSecInc * 256.0f + 2048.0f) - 1.0f;
	gClipXY.x = (2048.0f - fSecInc * 256.0f) + 1.0f;
	gClipMulVector.x = 2.0f / (gClipXY.y - gClipXY.x);
	gClipMulVector.y = 2.0f / (gClipXY.x - gClipXY.y);
	gClipAddVector.x = (gClipXY.x + gClipXY.y) / (gClipXY.x - gClipXY.y);
	gClipAddVector.y = (gClipXY.x + gClipXY.y) / (gClipXY.y - gClipXY.x);
	gClipMulVector.w = pCameraToScreen->dc + pCameraToScreen->cc * nearClip;
	gClipAddVector.z = (fovUp + fovDown) / (fovUp - fovDown);
	gClipAddVector.w = pCameraToScreen->dc + pCameraToScreen->cc * farClip;
	gClipXY.z = gClipXY.y;
	gClipXY.w = gClipXY.x;
	edF32Matrix4SetIdentityHard(&screenMatrix);
	edF32Matrix4MulF32Matrix4Hard(pCameraToFog, &projectionMatrix, &screenMatrix);
	fVar3 = (gRenderSceneConfig_SPR->field_0xe0).z;
	fVar2 = 255.0f - (gRenderSceneConfig_SPR->field_0xe0).x;
	fVar1 = ((255.0f - (gRenderSceneConfig_SPR->field_0xe0).y) - fVar2) / ((gRenderSceneConfig_SPR->field_0xe0).w - fVar3);
	pCameraToFog->cc = fVar1;
	pCameraToFog->dc = fVar2 - fVar1 * fVar3;
	return;
}

edF32MATRIX4 CameraToFog_Matrix = { 0 };
edF32MATRIX4 WorldToScreen_Matrix = { 0 };
edF32MATRIX4 gShadow_CameraToScreen_Matrix = { 0 };
edF32MATRIX4 gShadow_CameraToCulling_Matrix = { 0 };
edF32MATRIX4 gShadow_CameraToClipping_Matrix = { 0 };
edF32MATRIX4 gShadow_CameraToFog_Matrix = { 0 };
edF32MATRIX4 gShadow_WorldToCamera_Matrix = { 0 };
edF32MATRIX4 WorldToCamera_Matrix_CastShadow = { 0 };

edF32VECTOR4 gShadow_ClipMulVector = { 0 };
edF32VECTOR4 gShadow_ClipAddVector = { 0 };
edF32VECTOR4 gShadow_ClipXY = { 0 };
edF32VECTOR4 gShadow_CamPos = { 0 };
edF32VECTOR4 gShadow_gCamNormal_X = { 0 };
edF32VECTOR4 gShadow_gCamNormal_Y = { 0 };

ed_g2d_material* gDefault_Material_Current = NULL;
ed_g2d_material* gDefault_Material_Cluster_Current = NULL;

ed_3D_Scene* gShadowRenderScene = NULL;

ed_viewport* g_Camera_0044956c = NULL;

void ed3DShadowSetRenderViewport(ed_viewport* pCamera)
{
	g_Camera_0044956c = pCamera;
	return;
}

float gfSecuritx = 0.0f;
float gfSecurity = 0.0f;
float gfSecuritz = 0.0f;

void ed3DFrustumCompute(SceneConfig* pSceneConfig, edFCamera* pCamera, edSurface* pSurface)
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

	fVar12 = pCamera->verticalHalfFOV;
	min = pCamera->horizontalHalfFOV;
	fVar11 = pCamera->halfFOV;
	fVar7 = atan2f(min, fVar12);
	fVar8 = atan2f(fVar11, fVar12);
	fVar9 = sinf(fVar7);
	fVar10 = sinf(fVar8);
	fVar7 = cosf(fVar7);
	fVar8 = cosf(fVar8);
	(pSceneConfig->frustumA).frustumMatrix.aa = fVar7;
	(pSceneConfig->frustumA).frustumMatrix.ab = 0.0f;
	(pSceneConfig->frustumA).frustumMatrix.ac = fVar9;
	(pSceneConfig->frustumA).frustumMatrix.ad = 0.0f;
	(pSceneConfig->frustumA).frustumMatrix.ba = -fVar7;
	(pSceneConfig->frustumA).frustumMatrix.bb = 0.0f;
	(pSceneConfig->frustumA).frustumMatrix.bc = fVar9;
	(pSceneConfig->frustumA).frustumMatrix.bd = 0.0f;
	(pSceneConfig->frustumA).frustumMatrix.ca = 0.0f;
	(pSceneConfig->frustumA).frustumMatrix.cb = fVar8;
	(pSceneConfig->frustumA).frustumMatrix.cc = fVar10;
	(pSceneConfig->frustumA).frustumMatrix.cd = 0.0f;
	(pSceneConfig->frustumA).frustumMatrix.da = 0.0f;
	(pSceneConfig->frustumA).frustumMatrix.db = -fVar8;
	(pSceneConfig->frustumA).frustumMatrix.dc = fVar10;
	(pSceneConfig->frustumA).frustumMatrix.dd = 0.0f;
	(pSceneConfig->frustumA).field_0x40.x = 0.0f;
	(pSceneConfig->frustumA).field_0x40.y = 0.0f;
	(pSceneConfig->frustumA).field_0x40.z = 1.0f;
	(pSceneConfig->frustumA).field_0x40.w = 0.0f;
	(pSceneConfig->frustumA).field_0x50.x = 0.0f;
	(pSceneConfig->frustumA).field_0x50.y = 0.0f;
	(pSceneConfig->frustumA).field_0x50.z = -1.0f;
	(pSceneConfig->frustumA).field_0x50.w = -pSceneConfig->clipValue_0x18;
	uVar1 = pSurface->pSurfaceDesc->screenHeight;
	fVar7 = 2048.0f - gfSecurity;
	fVar8 = atan2f(((2048.0f - gfSecuritx) * min) / ((float)(uint)pSurface->pSurfaceDesc->screenWidth / 2.0f), fVar12);
	fVar7 = atan2f((fVar7 * fVar11) / ((float)(uint)uVar1 / 2.0f), fVar12);
	fVar11 = sinf(fVar8);
	fVar12 = sinf(fVar7);
	fVar7 = cosf(fVar7);
	fVar8 = cosf(fVar8);
	(pSceneConfig->frustumB).frustumMatrix.aa = fVar8;
	(pSceneConfig->frustumB).frustumMatrix.ab = 0.0f;
	(pSceneConfig->frustumB).frustumMatrix.ac = fVar11;
	(pSceneConfig->frustumB).frustumMatrix.ad = 0.0f;
	(pSceneConfig->frustumB).frustumMatrix.ba = -fVar8;
	(pSceneConfig->frustumB).frustumMatrix.bb = 0.0f;
	(pSceneConfig->frustumB).frustumMatrix.bc = fVar11;
	(pSceneConfig->frustumB).frustumMatrix.bd = 0.0f;
	(pSceneConfig->frustumB).frustumMatrix.ca = 0.0f;
	(pSceneConfig->frustumB).frustumMatrix.cb = fVar7;
	(pSceneConfig->frustumB).frustumMatrix.cc = fVar12;
	(pSceneConfig->frustumB).frustumMatrix.cd = 0.0f;
	(pSceneConfig->frustumB).frustumMatrix.da = 0.0f;
	(pSceneConfig->frustumB).frustumMatrix.db = -fVar7;
	(pSceneConfig->frustumB).frustumMatrix.dc = fVar12;
	(pSceneConfig->frustumB).frustumMatrix.dd = 0.0f;
	(pSceneConfig->frustumB).field_0x40.x = 0.0f;
	(pSceneConfig->frustumB).field_0x40.y = 0.0f;
	(pSceneConfig->frustumB).field_0x40.z = 1.0f;
	(pSceneConfig->frustumB).field_0x40.w = gfSecuritz;
	(pSceneConfig->frustumB).field_0x50.x = 0.0f;
	(pSceneConfig->frustumB).field_0x50.y = 0.0f;
	(pSceneConfig->frustumB).field_0x50.z = -1.0f;
	(pSceneConfig->frustumB).field_0x50.w = -pSceneConfig->clipValue_0x18;
	return;
}

int ed3DInitRenderEnvironement(ed_3D_Scene* pStaticMeshMaster, long mode)
{
	uint uVar1;
	edF32VECTOR4* v0;
	float fVar2;
	float fVar3;
	float farClip;
	float fVar5;
	edF32MATRIX4 local_40;

	RENDER_LOG("ed3DInitRenderEnvironement: %d", mode);

	fVar5 = pStaticMeshMaster->pCamera->halfFOV;
	memcpy(gRenderCamera, pStaticMeshMaster->pCamera, sizeof(edFCamera));
	//MY_LOG("Matrix + 2");
	//PRINT_MATRIX(gRenderCamera->aMatrices + 2);
	edF32Matrix4CopyHard(WorldToCamera_Matrix, (&gRenderCamera->worldToCamera));
	if (mode != 0) {
		edF32Matrix4CopyHard(&gCurLOD_WorldToCamera_Matrix, (&gRenderCamera->worldToCamera));
	}
	(gRenderCamera->calculatedRotation).y = 0.0f;
	(gRenderCamera->calculatedRotation).x = WorldToCamera_Matrix->cc;
	(gRenderCamera->calculatedRotation).z = -WorldToCamera_Matrix->ac;
	v0 = &gRenderCamera->calculatedRotation;
	if ((gRenderCamera->calculatedRotation).z +
		(gRenderCamera->calculatedRotation).x +
		(gRenderCamera->calculatedRotation).y == 0.0f) {
		v0->x = 1.0;
	}
	else {
		edF32Vector4NormalizeHard((float*)v0, (float*)v0);
	}
	sceVu0InverseMatrix(&local_40, WorldToCamera_Matrix);
	gCamNormal_X = local_40.rowX;

	//MY_LOG("Vec A");
	//PRINT_VECTOR(&Vector_0048c310);
	edF32Vector4NormalizeHard((float*)&gCamNormal_X, (float*)&gCamNormal_X);
	gCamNormal_Y = local_40.rowY;

	//MY_LOG("Vec B");
	//PRINT_VECTOR(&Vector_0048c320);
	edF32Vector4NormalizeHard((float*)&gCamNormal_Y, (float*)&gCamNormal_Y);
	gCamNormal_Z = local_40.rowZ;

	//MY_LOG("Vec C");
	//PRINT_VECTOR(&Vector_0048c330);
	edF32Vector4NormalizeHard((float*)&gCamNormal_Z, (float*)&gCamNormal_Z);
	gCamPos = gRenderCamera->position;

	fVar2 = gRenderCamera->halfFOV / gRenderCamera->verticalHalfFOV;
	gRenderInvFovCoef = 1.0f / fVar2;
	gRenderFovCoef = fVar2 * fVar2;
	if (mode != 0) {
		gCurLOD_RenderFOVCoef = gRenderFovCoef;
	}
	memcpy(&gRenderSurface, pStaticMeshMaster->pViewport->pColorBuffer, sizeof(edSurface));
	gRenderSceneConfig = &pStaticMeshMaster->sceneConfig;
	(pStaticMeshMaster->sceneConfig).clipValue_0x18 = (pStaticMeshMaster->sceneConfig).clipValue_0x4;
	memcpy(gRenderSceneConfig_SPR, gRenderSceneConfig, sizeof(SceneConfig));
	gRenderScene = pStaticMeshMaster;
	ed3DSceneLightSetGlobals(&(pStaticMeshMaster->sceneConfig).pLightConfig);
	uVar1 = gRenderSceneConfig_SPR->field_0x14;
	if ((int)uVar1 < 0) {
		fVar2 = (float)(uVar1 >> 1 | uVar1 & 1);
		fVar2 = fVar2 + fVar2;
	}
	else {
		fVar2 = (float)uVar1;
	}
	uVar1 = gRenderSceneConfig_SPR->field_0x10;
	if ((int)uVar1 < 0) {
		fVar3 = (float)(uVar1 >> 1 | uVar1 & 1);
		fVar3 = fVar3 + fVar3;
	}
	else {
		fVar3 = (float)uVar1;
	}
	farClip = gRenderSceneConfig_SPR->farClip;
	ed3DViewportComputeViewMatrices
	((float)(int)pStaticMeshMaster->pViewport->screenWidth,
		(float)(int)pStaticMeshMaster->pViewport->screenHeight, gRenderCamera->horizontalHalfFOV,
		gRenderCamera->halfFOV, gRenderCamera->verticalHalfFOV, 2048.0f, 2048.0f,
		gRenderSceneConfig_SPR->nearClip, CameraToScreen_Matrix, CameraToCulling_Matrix, CameraToClipping_Matrix,
		&CameraToFog_Matrix, gRenderSceneConfig_SPR->farClip, fVar2, fVar3);

	ed3DFrustumCompute(gRenderSceneConfig_SPR, gRenderCamera, &gRenderSurface);

	edF32Matrix4MulF32Matrix4Hard(&WorldToScreen_Matrix, WorldToCamera_Matrix, CameraToScreen_Matrix);

	if ((pStaticMeshMaster->flags_0x4 & 2) != 0) {
		memcpy(&gShadow_CameraToScreen_Matrix, CameraToScreen_Matrix, sizeof(edF32MATRIX4));
		memcpy(&gShadow_CameraToCulling_Matrix, CameraToCulling_Matrix, sizeof(edF32MATRIX4));
		memcpy(&gShadow_CameraToClipping_Matrix, CameraToClipping_Matrix, sizeof(edF32MATRIX4));
		memcpy(&gShadow_CameraToFog_Matrix, &CameraToFog_Matrix, sizeof(edF32MATRIX4));
		memcpy(&gShadow_WorldToCamera_Matrix, WorldToCamera_Matrix, sizeof(edF32MATRIX4));
		memcpy(&gShadow_ClipMulVector, &gClipMulVector, sizeof(edF32VECTOR4));
		memcpy(&gShadow_ClipAddVector, &gClipAddVector, sizeof(edF32VECTOR4));
		memcpy(&gShadow_ClipXY, &gClipXY, sizeof(edF32VECTOR4));
		memcpy(&gShadow_CamPos, &gCamPos, sizeof(edF32VECTOR4));
		memcpy(&gShadow_gCamNormal_X, &gCamNormal_X, sizeof(edF32VECTOR4));
		memcpy(&gShadow_gCamNormal_Y, &gCamNormal_Y, sizeof(edF32VECTOR4));
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
		gDefault_Material_Current = &gMaterial_Render_Zbuffer_Only;
		gDefault_Material_Cluster_Current = &gMaterial_Render_Zbuffer_Only_Cluster;
		gCamPos.x = 2048.0f - fVar2 / 2.0f;
		gCamPos.y = 2048.0f - fVar3 / 2.0f;
		gCamPos.z = 1.0f / fVar2;
		gCamPos.w = 1.0f / fVar3;
		ed3DShadowSetRenderViewport((gRenderSceneConfig_SPR->pShadowConfig).pViewport);
		*gShadowRenderMask = (uint)(ushort)(gRenderSceneConfig_SPR->pShadowConfig).renderMask;
		if (*gShadowRenderMask == 0) {
			return 0;
		}
		memcpy(&WorldToCamera_Matrix_CastShadow, WorldToCamera_Matrix, sizeof(edF32MATRIX4));
		gShadowRenderScene = pStaticMeshMaster;
	}
	else {
		*gShadowRenderMask = 0;
		gDefault_Material_Current = &gDefault_Material_Gouraud;
		gDefault_Material_Cluster_Current = &gDefault_Material_Gouraud_Cluster;
	}
	if (pStaticMeshMaster->pViewport->screenHeight != 0x200) {
		pStaticMeshMaster->pCamera->halfFOV = fVar5;
	}
	return 1;
}

edpkt_data* ed3DDMAGenerateGlobalPacket(edpkt_data* pPkt)
{
	uint uVar1;
	uint uVar2;
	uint uVar3;
	float fVar4;
	float fVar5;
	float fVar6;

	pPkt->cmdA = ED_VIF1_SET_TAG_CNT(8);
	pPkt->cmdB = 0;
	pPkt->asU32[2] = SCE_VIF1_SET_NOP(0);
	pPkt->asU32[3] = SCE_VIF1_SET_UNPACK(0, 8, UNPACK_V4_32, 0); //0x6c080000

	pPkt[1] = gFANbuffers;

	// Not sure what this does yet, but its transferred via the above ^.
	pPkt[2].cmdA = 0x302ec00000008000;
	pPkt[2].cmdB = 0x512;

	pPkt[3].asVector = gClipMulVector;
	pPkt[4].asVector = gClipAddVector;
	pPkt[5].asVector = gClipXY;
	pPkt[6].asVector = gCamPos;
	pPkt[7].asVector = gCamNormal_X;
	pPkt[8].asVector = gCamNormal_Y;
	return pPkt + 9;
}

void ed3DInitVU1Globals(void)
{
	g_VifRefPktCur->cmdA = ED_VIF1_SET_TAG_REF(1, &g_stMatrixHeader);
	g_VifRefPktCur->cmdB = SCE_VIF1_SET_NOP(0);
	g_VifRefPktCur = g_VifRefPktCur + 1;
	g_VifRefPktCur = ed3DDMAGenerateGlobalPacket(g_VifRefPktCur);
	return;
}

edpkt_data* ed3DAddViewportContextPacket(ed_viewport* pViewport, edpkt_data* pPkt)
{
	edpkt_data* pNextPkt;
	uint qwc;

	pNextPkt = pPkt;
	if (pViewport != (ed_viewport*)0x0) {
		pNextPkt = edViewportUpdateEnv(pViewport, pPkt + 1);
		qwc = ((uint)((ulong)pNextPkt - (ulong)pPkt) >> 4) - 1;
		pPkt->cmdA = ED_VIF1_SET_TAG_CNT(qwc & 0xffffffffU);
		pPkt->cmdB = 0;
		pPkt->asU32[2] = 0;
		pPkt->asU32[3] = SCE_VIF1_SET_DIRECTHL(qwc, 0); // uVar2 | 0x51000000;
	}
	return pNextPkt;
}

edpkt_data* ed3DSceneAddContextPacket(ed_3D_Scene* pScene, edpkt_data* pPkt)
{
	edpkt_data* pRVar1;

	RENDER_LOG("ed3DSceneAddContextPacket %p", pScene);

	pPkt->cmdA = ED_VIF1_SET_TAG_CNT(3);
	pPkt->cmdB = 0;
	pPkt->asU32[2] = SCE_VIF1_SET_ITOP(0x03dc, 0); // 0x40003dc;
	pPkt->asU32[3] = SCE_VIF1_SET_UNPACK(0x03dc, 2, UNPACK_V4_32, 0); // 0x6c0203dc;

	pPkt[1].cmdA = SCE_GIF_SET_TAG(
		1,					// NLOOP
		SCE_GS_TRUE,		// EOP
		SCE_GS_FALSE,		// PRE
		0,					// PRIM
		SCE_GIF_PACKED,		// FLG
		1					// NREG
	);
	pPkt[1].cmdB = SCE_GIF_PACKED_AD;

	pPkt[2].cmdA = SCE_GS_SET_FOGCOL(pScene->sceneConfig.fogCol_0xf0.r, pScene->sceneConfig.fogCol_0xf0.g, pScene->sceneConfig.fogCol_0xf0.b);
	pPkt[2].cmdB = SCE_GS_FOGCOL;

	pPkt[3].asU32[0] = SCE_VIF1_SET_NOP(0);
	pPkt[3].asU32[1] = SCE_VIF1_SET_MSCAL(0, 0);
	pPkt[3].cmdB = SCE_VIF1_SET_FLUSH(0);
	pRVar1 = pPkt + 4;

	if (pScene->pViewport != (ed_viewport*)0x0) {
		pRVar1 = ed3DAddViewportContextPacket(pScene->pViewport, pRVar1);
	}
	return pRVar1;
}

struct edRECT16 {
	short x;
	short y;
	short w;
	short h;
};

edRECT16 gCurRectViewport = { 0 };
edRECT16 CameraScreenData_00449560 = { 0 };

edpkt_data* ed3DFlushResetOffset(edpkt_data* pPkt, edRECT16* pRect)
{
	pPkt->cmdA = ED_VIF1_SET_TAG_CNT(3);
	pPkt->cmdB = 0;
	pPkt->asU32[2] = SCE_VIF1_SET_NOP(0);
	pPkt->asU32[3] = SCE_VIF1_SET_DIRECT(3, 0);

	pPkt[1].cmdA = SCE_GIF_SET_TAG(
		2,					// NLOOP
		SCE_GS_TRUE,		// EOP
		SCE_GS_FALSE,		// PRE
		1,					// PRIM
		SCE_GIF_PACKED,		// FLG
		1					// NREG
	);
	pPkt[1].cmdB = SCE_GIF_PACKED_AD;

	// OFFSET
	pPkt[2].cmdA = SCE_GS_SET_XYOFFSET(
		(((0x1000 - ((int)pRect->w + (int)pRect->x) >> 1) - ((int)pRect->x >> 1)) * 0x10) & 0xffffffffU,	// X
		(((0x1000 - ((int)pRect->h + (int)pRect->y) >> 1) - ((int)pRect->y >> 1)) * 0x10)	// Y
	);
	pPkt[2].cmdB = SCE_GS_XYOFFSET_1;

	// SCISSOR
	pPkt[3].cmdA = SCE_GS_SET_SCISSOR(
		pRect->x,											// SCAX0
		((int)pRect->w + (int)pRect->x + -1),	// SCAX1
		pRect->y,											// SCAY0
		((int)pRect->h + (int)pRect->y + -1)	// SCAY1
	);
	pPkt[3].cmdB = SCE_GS_SCISSOR_1;

	return pPkt + 4;
}

void ed3DClustersRenderComputeSceneHierarchyList(ed_3D_Scene* pStaticMeshMaster)
{
	int iVar1;
	int iVar2;
	int* piVar3;
	uint uVar4;

	piVar3 = pStaticMeshMaster->field_0x1ac;
	for (uVar4 = 0; uVar4 < pStaticMeshMaster->heirarchyListCount; uVar4 = uVar4 + 1) {
		iVar1 = *piVar3;
		for (iVar2 = *(int*)(iVar1 + 4); iVar2 != iVar1; iVar2 = *(int*)(iVar2 + 4)) {
			IMPLEMENTATION_GUARD(
				ed_3d_hierarchy_node::ed3DRenderSonHierarchy(*(ed_3d_hierarchy_node**)(iVar2 + 0xc));)
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
	edF32VECTOR4 field_0x30;
};)

void ed3DSceneSortClusters(ed_3D_Scene* pScene)
{
	edLIST* peVar1;
	bool bVar2;
	edLIST* peVar3;
	edLIST* peVar4;
	edF32VECTOR4 local_10;
	edCluster* pvVar1;
	edCluster* pMVar2;
	MeshData_CSTA* pcVar1;
	edCluster* pMVar1;

	bVar2 = true;
	if ((pScene->flags_0x4 & 0x400) == 0) {
		peVar4 = &pScene->headerB;
	}
	else {
		peVar4 = pScene->field_0x1a0;
	}
	peVar3 = (edLIST*)peVar4->pPrev;
	if (peVar3 != peVar4) {
		for (; peVar3 != peVar4; peVar3 = (edLIST*)peVar3->pPrev) {
			pMVar2 = (edCluster*)peVar3->pData;
			pcVar1 = (MeshData_CSTA*)pMVar2->pMeshInfo->CSTA;
			if (pcVar1 != (MeshData_CSTA*)0x0) {
				edF32Vector4SubHard(&local_10, &pScene->pCamera->position, &pcVar1->field_0x30);
				pMVar2->field_0x18 = local_10.z * local_10.z + local_10.x * local_10.x + local_10.y * local_10.y;
			}
		}
		while (bVar2) {
			peVar3 = (edLIST*)peVar4->pPrev;
			bVar2 = false;
			for (peVar1 = (edLIST*)peVar3->pPrev; peVar1 != peVar4; peVar1 = (edLIST*)peVar1->pPrev) {
				pvVar1 = (edCluster*)peVar1->pData;
				pMVar1 = (edCluster*)peVar3->pData;
				if (pMVar1->field_0x18 < pvVar1->field_0x18) {
					peVar3->pData = pvVar1;
					bVar2 = true;
					peVar1->pData = pMVar1;
				}
				peVar3 = peVar1;
			}
		}
	}
	return;
}

int gFushListCounter = 0;
float FLOAT_00448a04 = 0.01f;

byte gGlobalAlhaON = 0;

ed_g2d_layer* gCurLayer = NULL;
ed_g2d_bitmap* gCurBitmap = NULL;

byte gShadowFlushMode = 0;
int gFlushListFlusaON = 0;

edpkt_data* ed3DHierachyCheckForGlobalAlpha(edpkt_data* pRenderCommand, ed_g2d_layer* pLAY)
{
	if (((gpLastMaterial == (ed_dma_material*)0x0) || ((gpLastMaterial->flags & 1) != 0)) &&
		(gpLastMaterial != (ed_dma_material*)0x0)) {
		gGlobalAlhaON = 0x80;
	}
	else {
		if (((gpCurHierarchy == (MeshTransformDataBase*)0x0) || (gpCurHierarchy == (MeshTransformDataBase*)0x0000ffff))
			|| (gpCurHierarchy->GlobalAlhaON == 0xff)) {
			if (((gGlobalAlhaON != 0x80) && (gpCurHierarchy != (MeshTransformDataBase*)0x0)) &&
				(((gpCurHierarchy != (MeshTransformDataBase*)0x0000ffff &&
					(((gpCurHierarchy->GlobalAlhaON == -1 && (gCurViewportUsed != (ed_viewport*)0x0)) &&
						(gGlobalAlhaON = 0x80, pLAY != (ed_g2d_layer*)0x0)))) && ((pLAY->flags_0x0 & 0xfc) == 0)))) {
				IMPLEMENTATION_GUARD();
				pRenderCommand->cmdA = ED_VIF1_SET_TAG_REF(0, 0);
				pRenderCommand->cmdB = 0x1300000000000000;
				pRenderCommand[1].cmdA = ED_VIF1_SET_TAG_CNT(3);
				pRenderCommand[1].cmdB = 0;
				*(undefined4*)&pRenderCommand[1].cmdB = 0;
				*(undefined4*)((int)&pRenderCommand[1].cmdB + 4) = 0x50000003;
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
				IMPLEMENTATION_GUARD(
					pRenderCommand = ed3DHierachyCheckForGlobalAlphaSetPKT_(pRenderCommand, pLAY));
			}
		}
	}
	return pRenderCommand;
}

void edF32Matrix4OrthonormalizeHard(edF32MATRIX4* m0, edF32MATRIX4* m1)
{
	float fVar1;
	float fVar2;
	float fVar3;
	float fVar4;

	fVar1 = m1->aa;
	fVar2 = m1->ab;
	fVar3 = m1->ac;
	fVar4 = 1.0f / (sqrtf(fVar1 * fVar1 + fVar2 * fVar2 + fVar3 * fVar3) + 0.0f);
	m0->aa = fVar1 * fVar4;
	m0->ab = fVar2 * fVar4;
	m0->ac = fVar3 * fVar4;
	m0->ad = 0.0f;
	fVar1 = m1->ba;
	fVar2 = m1->bb;
	fVar3 = m1->bc;
	m0->ca = m0->ab * fVar3 - fVar2 * m0->ac;
	m0->cb = m0->ac * fVar1 - fVar3 * m0->aa;
	m0->cc = m0->aa * fVar2 - fVar1 * m0->ab;
	m0->cd = 0.0f;
	fVar1 = m0->ca;
	fVar2 = m0->cb;
	fVar3 = m0->cc;
	fVar4 = 1.0f / (sqrtf(fVar1 * fVar1 + fVar2 * fVar2 + fVar3 * fVar3) + 0.0f);
	m0->ca = fVar1 * fVar4;
	m0->cb = fVar2 * fVar4;
	m0->cc = fVar3 * fVar4;
	m0->cd = 0.0f;
	m0->ba = m0->cb * m0->ac - m0->ab * m0->cc;
	m0->bb = m0->cc * m0->aa - m0->ac * m0->ca;
	m0->bc = m0->ca * m0->ab - m0->aa * m0->cb;
	m0->bd = 0.0f;
	fVar3 = m1->db;
	fVar1 = m1->dc;
	fVar2 = m1->dd;
	m0->da = m1->da;
	m0->db = fVar3;
	m0->dc = fVar1;
	m0->dd = fVar2;
	return;
}

#define MATRIX_PACKET_START_SPR		0x70000800
#define CAM_NORMAL_X_SPR			0x70000800
#define CAM_NORMAL_Y_SPR			0x70000810
#define CAM_NORMAL_Y_SPR			0x70000810
#define OBJ_TO_CULLING_MATRIX		0x70000820
#define OBJ_TO_CLIPPING_MATRIX		0x70000860
#define OBJ_TO_SCREEN_MATRIX		0x700008A0

#define LIGHT_DIRECTIONS_MATRIX_SPR 0x70000a40
#define LIGHT_COLOR_MATRIX_SPR		0x70000a50
#define LIGHT_AMBIENT_MATRIX_SPR	0x70000a60

#define OBJECT_TO_CAMERA_MATRIX_SPR	0x70000a60

edF32MATRIX4* ed3DPKTCopyMatrixPacket(edF32MATRIX4* pMatrixBuffer, ed_dma_matrix* pDmaMatrix, byte param_3)
{
	edF32MATRIX4* m1;
	LightingMatrixSubSubObj* pLVar1;
	ed_3d_hierarchy_setup* pLVar2;
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
	ed_3d_hierarchy_setup* puVar1;

	m1 = pDmaMatrix->pObjToWorld;
	RENDER_LOG("DMA Begin ed3DPKTCopyMatrixPacket");
	edDmaSync(SHELLDMA_CHANNEL_VIF0);
	edF32Vector4ScaleHard(pDmaMatrix->normalScale, SCRATCHPAD_ADDRESS_TYPE(CAM_NORMAL_X_SPR, edF32VECTOR4*), &gCamNormal_X);
	edF32Vector4ScaleHard(pDmaMatrix->normalScale, SCRATCHPAD_ADDRESS_TYPE(CAM_NORMAL_Y_SPR, edF32VECTOR4*), &gCamNormal_Y);
	*g_pCurFlareObj2WorldMtx = m1;
	if (((pDmaMatrix->pMeshTransformData == (MeshTransformDataBase*)0x0) ||
		(puVar1 = pDmaMatrix->pMeshTransformData->pHierarchySetup, puVar1 == (ed_3d_hierarchy_setup*)0x0))
		|| (pLVar1 = puVar1->pLightData, pLVar1 == (LightingMatrixSubSubObj*)0x0)) {
		SCRATCHPAD_WRITE_ADDRESS_TYPE(LIGHT_DIRECTIONS_MATRIX_SPR, edF32MATRIX4*, gLightDirections_Matrix_Scratch);
		SCRATCHPAD_WRITE_ADDRESS_TYPE(LIGHT_COLOR_MATRIX_SPR, edF32MATRIX4*, gLightColor_Matrix_Scratch);
		SCRATCHPAD_WRITE_ADDRESS_TYPE(LIGHT_AMBIENT_MATRIX_SPR, edF32VECTOR4*, gLightAmbiant_Scratch);
	}
	else {
		SCRATCHPAD_WRITE_ADDRESS_TYPE(LIGHT_DIRECTIONS_MATRIX_SPR, edF32MATRIX4*, pLVar1->pLightDirections);
		if (SCRATCHPAD_READ_ADDRESS_TYPE(LIGHT_DIRECTIONS_MATRIX_SPR, edF32MATRIX4*) == (edF32MATRIX4*)0x0) {
			SCRATCHPAD_WRITE_ADDRESS_TYPE(LIGHT_DIRECTIONS_MATRIX_SPR, edF32MATRIX4*, gLightDirections_Matrix_Scratch);
		}
		SCRATCHPAD_WRITE_ADDRESS_TYPE(LIGHT_COLOR_MATRIX_SPR, edF32MATRIX4*, pLVar1->pLightColor);
		if (SCRATCHPAD_READ_ADDRESS_TYPE(LIGHT_COLOR_MATRIX_SPR, edF32MATRIX4*) == (edF32MATRIX4*)0x0) {
			SCRATCHPAD_WRITE_ADDRESS_TYPE(LIGHT_COLOR_MATRIX_SPR, edF32MATRIX4*, gLightColor_Matrix_Scratch);
		}
		SCRATCHPAD_WRITE_ADDRESS_TYPE(LIGHT_AMBIENT_MATRIX_SPR, edF32VECTOR4*, pLVar1->pLightAmbient);
		if (SCRATCHPAD_READ_ADDRESS_TYPE(LIGHT_AMBIENT_MATRIX_SPR, edF32VECTOR4*) == (edF32VECTOR4*)0x0) {
			SCRATCHPAD_WRITE_ADDRESS_TYPE(LIGHT_AMBIENT_MATRIX_SPR, edF32VECTOR4*, gLightAmbiant_Scratch);
		}
	}

	edF32VECTOR4* vectorA = SCRATCHPAD_ADDRESS_TYPE(0x70000990, edF32VECTOR4*);

	vectorA->x = (gRenderCamera->calculatedRotation).x;
	vectorA->z = (gRenderCamera->calculatedRotation).z;
	vectorA->y = 0.0f;
	if (param_3 == 0) {
		vectorA->w = 0.0f;
	}
	else {
		vectorA->w = 999.0f;
		m1 = gF32Matrix4Unit_Scratch;
	}

	edF32MATRIX4 objToWorld = *m1;
	edF32MATRIX4 worldToCamera = *WorldToCamera_Matrix;

	edF32MATRIX4* pObjToCamera = SCRATCHPAD_ADDRESS_TYPE(OBJECT_TO_CAMERA_MATRIX_SPR, edF32MATRIX4*);

	pObjToCamera->aa = worldToCamera.aa * objToWorld.aa + worldToCamera.ba * objToWorld.ab + worldToCamera.ca * objToWorld.ac + worldToCamera.da * objToWorld.ad;
	pObjToCamera->ab = worldToCamera.ab * objToWorld.aa + worldToCamera.bb * objToWorld.ab + worldToCamera.cb * objToWorld.ac + worldToCamera.db * objToWorld.ad;
	pObjToCamera->ac = worldToCamera.ac * objToWorld.aa + worldToCamera.bc * objToWorld.ab + worldToCamera.cc * objToWorld.ac + worldToCamera.dc * objToWorld.ad;
	pObjToCamera->ad = worldToCamera.ad * objToWorld.aa + worldToCamera.bd * objToWorld.ab + worldToCamera.cd * objToWorld.ac + worldToCamera.dd * objToWorld.ad;
	pObjToCamera->ba = worldToCamera.aa * objToWorld.ba + worldToCamera.ba * objToWorld.bb + worldToCamera.ca * objToWorld.bc + worldToCamera.da * objToWorld.bd;
	pObjToCamera->bb = worldToCamera.ab * objToWorld.ba + worldToCamera.bb * objToWorld.bb + worldToCamera.cb * objToWorld.bc + worldToCamera.db * objToWorld.bd;
	pObjToCamera->bc = worldToCamera.ac * objToWorld.ba + worldToCamera.bc * objToWorld.bb + worldToCamera.cc * objToWorld.bc + worldToCamera.dc * objToWorld.bd;
	pObjToCamera->bd = worldToCamera.ad * objToWorld.ba + worldToCamera.bd * objToWorld.bb + worldToCamera.cd * objToWorld.bc + worldToCamera.dd * objToWorld.bd;
	pObjToCamera->ca = worldToCamera.aa * objToWorld.ca + worldToCamera.ba * objToWorld.cb + worldToCamera.ca * objToWorld.cc + worldToCamera.da * objToWorld.cd;
	pObjToCamera->cb = worldToCamera.ab * objToWorld.ca + worldToCamera.bb * objToWorld.cb + worldToCamera.cb * objToWorld.cc + worldToCamera.db * objToWorld.cd;
	pObjToCamera->cc = worldToCamera.ac * objToWorld.ca + worldToCamera.bc * objToWorld.cb + worldToCamera.cc * objToWorld.cc + worldToCamera.dc * objToWorld.cd;
	pObjToCamera->cd = worldToCamera.ad * objToWorld.ca + worldToCamera.bd * objToWorld.cb + worldToCamera.cd * objToWorld.cc + worldToCamera.dd * objToWorld.cd;
	pObjToCamera->da = worldToCamera.aa * objToWorld.da + worldToCamera.ba * objToWorld.db + worldToCamera.ca * objToWorld.dc + worldToCamera.da * objToWorld.dd;
	pObjToCamera->db = worldToCamera.ab * objToWorld.da + worldToCamera.bb * objToWorld.db + worldToCamera.cb * objToWorld.dc + worldToCamera.db * objToWorld.dd;
	pObjToCamera->dc = worldToCamera.ac * objToWorld.da + worldToCamera.bc * objToWorld.db + worldToCamera.cc * objToWorld.dc + worldToCamera.dc * objToWorld.dd;
	pObjToCamera->dd = worldToCamera.ad * objToWorld.da + worldToCamera.bd * objToWorld.db + worldToCamera.cd * objToWorld.dc + worldToCamera.dd * objToWorld.dd;

	edF32MATRIX4 cameraToCulling = *CameraToCulling_Matrix;
	edF32MATRIX4* pObjToCulling = SCRATCHPAD_ADDRESS_TYPE(OBJ_TO_CULLING_MATRIX, edF32MATRIX4*);

	pObjToCulling->aa = cameraToCulling.aa * pObjToCamera->aa + cameraToCulling.ba * pObjToCamera->ab + cameraToCulling.ca * pObjToCamera->ac + cameraToCulling.da * pObjToCamera->ad;
	pObjToCulling->ab = cameraToCulling.ab * pObjToCamera->aa + cameraToCulling.bb * pObjToCamera->ab + cameraToCulling.cb * pObjToCamera->ac + cameraToCulling.db * pObjToCamera->ad;
	pObjToCulling->ac = cameraToCulling.ac * pObjToCamera->aa + cameraToCulling.bc * pObjToCamera->ab + cameraToCulling.cc * pObjToCamera->ac + cameraToCulling.dc * pObjToCamera->ad;
	pObjToCulling->ad = cameraToCulling.ad * pObjToCamera->aa + cameraToCulling.bd * pObjToCamera->ab + cameraToCulling.cd * pObjToCamera->ac + cameraToCulling.dd * pObjToCamera->ad;
	pObjToCulling->ba = cameraToCulling.aa * pObjToCamera->ba + cameraToCulling.ba * pObjToCamera->bb + cameraToCulling.ca * pObjToCamera->bc + cameraToCulling.da * pObjToCamera->bd;
	pObjToCulling->bb = cameraToCulling.ab * pObjToCamera->ba + cameraToCulling.bb * pObjToCamera->bb + cameraToCulling.cb * pObjToCamera->bc + cameraToCulling.db * pObjToCamera->bd;
	pObjToCulling->bc = cameraToCulling.ac * pObjToCamera->ba + cameraToCulling.bc * pObjToCamera->bb + cameraToCulling.cc * pObjToCamera->bc + cameraToCulling.dc * pObjToCamera->bd;
	pObjToCulling->bd = cameraToCulling.ad * pObjToCamera->ba + cameraToCulling.bd * pObjToCamera->bb + cameraToCulling.cd * pObjToCamera->bc + cameraToCulling.dd * pObjToCamera->bd;
	pObjToCulling->ca = cameraToCulling.aa * pObjToCamera->ca + cameraToCulling.ba * pObjToCamera->cb + cameraToCulling.ca * pObjToCamera->cc + cameraToCulling.da * pObjToCamera->cd;
	pObjToCulling->cb = cameraToCulling.ab * pObjToCamera->ca + cameraToCulling.bb * pObjToCamera->cb + cameraToCulling.cb * pObjToCamera->cc + cameraToCulling.db * pObjToCamera->cd;
	pObjToCulling->cc = cameraToCulling.ac * pObjToCamera->ca + cameraToCulling.bc * pObjToCamera->cb + cameraToCulling.cc * pObjToCamera->cc + cameraToCulling.dc * pObjToCamera->cd;
	pObjToCulling->cd = cameraToCulling.ad * pObjToCamera->ca + cameraToCulling.bd * pObjToCamera->cb + cameraToCulling.cd * pObjToCamera->cc + cameraToCulling.dd * pObjToCamera->cd;
	pObjToCulling->da = cameraToCulling.aa * pObjToCamera->da + cameraToCulling.ba * pObjToCamera->db + cameraToCulling.ca * pObjToCamera->dc + cameraToCulling.da * pObjToCamera->dd;
	pObjToCulling->db = cameraToCulling.ab * pObjToCamera->da + cameraToCulling.bb * pObjToCamera->db + cameraToCulling.cb * pObjToCamera->dc + cameraToCulling.db * pObjToCamera->dd;
	pObjToCulling->dc = cameraToCulling.ac * pObjToCamera->da + cameraToCulling.bc * pObjToCamera->db + cameraToCulling.cc * pObjToCamera->dc + cameraToCulling.dc * pObjToCamera->dd;
	pObjToCulling->dd = cameraToCulling.ad * pObjToCamera->da + cameraToCulling.bd * pObjToCamera->db + cameraToCulling.cd * pObjToCamera->dc + cameraToCulling.dd * pObjToCamera->dd;

	edF32MATRIX4 cameraToClipping = *CameraToClipping_Matrix;
	edF32MATRIX4* pObjToClipping = SCRATCHPAD_ADDRESS_TYPE(OBJ_TO_CLIPPING_MATRIX, edF32MATRIX4*);

	pObjToClipping->aa = cameraToClipping.aa * pObjToCamera->aa + cameraToClipping.ba * pObjToCamera->ab + cameraToClipping.ca * pObjToCamera->ac + cameraToClipping.da * pObjToCamera->ad;
	pObjToClipping->ab = cameraToClipping.ab * pObjToCamera->aa + cameraToClipping.bb * pObjToCamera->ab + cameraToClipping.cb * pObjToCamera->ac + cameraToClipping.db * pObjToCamera->ad;
	pObjToClipping->ac = cameraToClipping.ac * pObjToCamera->aa + cameraToClipping.bc * pObjToCamera->ab + cameraToClipping.cc * pObjToCamera->ac + cameraToClipping.dc * pObjToCamera->ad;
	pObjToClipping->ad = cameraToClipping.ad * pObjToCamera->aa + cameraToClipping.bd * pObjToCamera->ab + cameraToClipping.cd * pObjToCamera->ac + cameraToClipping.dd * pObjToCamera->ad;
	pObjToClipping->ba = cameraToClipping.aa * pObjToCamera->ba + cameraToClipping.ba * pObjToCamera->bb + cameraToClipping.ca * pObjToCamera->bc + cameraToClipping.da * pObjToCamera->bd;
	pObjToClipping->bb = cameraToClipping.ab * pObjToCamera->ba + cameraToClipping.bb * pObjToCamera->bb + cameraToClipping.cb * pObjToCamera->bc + cameraToClipping.db * pObjToCamera->bd;
	pObjToClipping->bc = cameraToClipping.ac * pObjToCamera->ba + cameraToClipping.bc * pObjToCamera->bb + cameraToClipping.cc * pObjToCamera->bc + cameraToClipping.dc * pObjToCamera->bd;
	pObjToClipping->bd = cameraToClipping.ad * pObjToCamera->ba + cameraToClipping.bd * pObjToCamera->bb + cameraToClipping.cd * pObjToCamera->bc + cameraToClipping.dd * pObjToCamera->bd;
	pObjToClipping->ca = cameraToClipping.aa * pObjToCamera->ca + cameraToClipping.ba * pObjToCamera->cb + cameraToClipping.ca * pObjToCamera->cc + cameraToClipping.da * pObjToCamera->cd;
	pObjToClipping->cb = cameraToClipping.ab * pObjToCamera->ca + cameraToClipping.bb * pObjToCamera->cb + cameraToClipping.cb * pObjToCamera->cc + cameraToClipping.db * pObjToCamera->cd;
	pObjToClipping->cc = cameraToClipping.ac * pObjToCamera->ca + cameraToClipping.bc * pObjToCamera->cb + cameraToClipping.cc * pObjToCamera->cc + cameraToClipping.dc * pObjToCamera->cd;
	pObjToClipping->cd = cameraToClipping.ad * pObjToCamera->ca + cameraToClipping.bd * pObjToCamera->cb + cameraToClipping.cd * pObjToCamera->cc + cameraToClipping.dd * pObjToCamera->cd;
	pObjToClipping->da = cameraToClipping.aa * pObjToCamera->da + cameraToClipping.ba * pObjToCamera->db + cameraToClipping.ca * pObjToCamera->dc + cameraToClipping.da * pObjToCamera->dd;
	pObjToClipping->db = cameraToClipping.ab * pObjToCamera->da + cameraToClipping.bb * pObjToCamera->db + cameraToClipping.cb * pObjToCamera->dc + cameraToClipping.db * pObjToCamera->dd;
	pObjToClipping->dc = cameraToClipping.ac * pObjToCamera->da + cameraToClipping.bc * pObjToCamera->db + cameraToClipping.cc * pObjToCamera->dc + cameraToClipping.dc * pObjToCamera->dd;
	pObjToClipping->dd = cameraToClipping.ad * pObjToCamera->da + cameraToClipping.bd * pObjToCamera->db + cameraToClipping.cd * pObjToCamera->dc + cameraToClipping.dd * pObjToCamera->dd;

	edF32MATRIX4 cameraToScreen = *CameraToScreen_Matrix;
	edF32MATRIX4* pObjToScreen = SCRATCHPAD_ADDRESS_TYPE(OBJ_TO_SCREEN_MATRIX, edF32MATRIX4*);

	pObjToScreen->aa = cameraToScreen.aa * pObjToCamera->aa + cameraToScreen.ba * pObjToCamera->ab + cameraToScreen.ca * pObjToCamera->ac + cameraToScreen.da * pObjToCamera->ad;
	pObjToScreen->ab = cameraToScreen.ab * pObjToCamera->aa + cameraToScreen.bb * pObjToCamera->ab + cameraToScreen.cb * pObjToCamera->ac + cameraToScreen.db * pObjToCamera->ad;
	pObjToScreen->ac = cameraToScreen.ac * pObjToCamera->aa + cameraToScreen.bc * pObjToCamera->ab + cameraToScreen.cc * pObjToCamera->ac + cameraToScreen.dc * pObjToCamera->ad;
	pObjToScreen->ad = cameraToScreen.ad * pObjToCamera->aa + cameraToScreen.bd * pObjToCamera->ab + cameraToScreen.cd * pObjToCamera->ac + cameraToScreen.dd * pObjToCamera->ad;
	pObjToScreen->ba = cameraToScreen.aa * pObjToCamera->ba + cameraToScreen.ba * pObjToCamera->bb + cameraToScreen.ca * pObjToCamera->bc + cameraToScreen.da * pObjToCamera->bd;
	pObjToScreen->bb = cameraToScreen.ab * pObjToCamera->ba + cameraToScreen.bb * pObjToCamera->bb + cameraToScreen.cb * pObjToCamera->bc + cameraToScreen.db * pObjToCamera->bd;
	pObjToScreen->bc = cameraToScreen.ac * pObjToCamera->ba + cameraToScreen.bc * pObjToCamera->bb + cameraToScreen.cc * pObjToCamera->bc + cameraToScreen.dc * pObjToCamera->bd;
	pObjToScreen->bd = cameraToScreen.ad * pObjToCamera->ba + cameraToScreen.bd * pObjToCamera->bb + cameraToScreen.cd * pObjToCamera->bc + cameraToScreen.dd * pObjToCamera->bd;
	pObjToScreen->ca = cameraToScreen.aa * pObjToCamera->ca + cameraToScreen.ba * pObjToCamera->cb + cameraToScreen.ca * pObjToCamera->cc + cameraToScreen.da * pObjToCamera->cd;
	pObjToScreen->cb = cameraToScreen.ab * pObjToCamera->ca + cameraToScreen.bb * pObjToCamera->cb + cameraToScreen.cb * pObjToCamera->cc + cameraToScreen.db * pObjToCamera->cd;
	pObjToScreen->cc = cameraToScreen.ac * pObjToCamera->ca + cameraToScreen.bc * pObjToCamera->cb + cameraToScreen.cc * pObjToCamera->cc + cameraToScreen.dc * pObjToCamera->cd;
	pObjToScreen->cd = cameraToScreen.ad * pObjToCamera->ca + cameraToScreen.bd * pObjToCamera->cb + cameraToScreen.cd * pObjToCamera->cc + cameraToScreen.dd * pObjToCamera->cd;
	pObjToScreen->da = cameraToScreen.aa * pObjToCamera->da + cameraToScreen.ba * pObjToCamera->db + cameraToScreen.ca * pObjToCamera->dc + cameraToScreen.da * pObjToCamera->dd;
	pObjToScreen->db = cameraToScreen.ab * pObjToCamera->da + cameraToScreen.bb * pObjToCamera->db + cameraToScreen.cb * pObjToCamera->dc + cameraToScreen.db * pObjToCamera->dd;
	pObjToScreen->dc = cameraToScreen.ac * pObjToCamera->da + cameraToScreen.bc * pObjToCamera->db + cameraToScreen.cc * pObjToCamera->dc + cameraToScreen.dc * pObjToCamera->dd;
	pObjToScreen->dd = cameraToScreen.ad * pObjToCamera->da + cameraToScreen.bd * pObjToCamera->db + cameraToScreen.cd * pObjToCamera->dc + cameraToScreen.dd * pObjToCamera->dd;

	edF32Matrix4OrthonormalizeHard(SCRATCHPAD_ADDRESS_TYPE(0x70000A00, edF32MATRIX4*), m1);
	sceVu0InverseMatrix(SCRATCHPAD_ADDRESS_TYPE(0x70000A00, edF32MATRIX4*), SCRATCHPAD_ADDRESS_TYPE(0x70000A00, edF32MATRIX4*));
	edF32Matrix4MulF32Matrix4Hard(SCRATCHPAD_ADDRESS_TYPE(0x70000920, edF32MATRIX4*), SCRATCHPAD_ADDRESS_TYPE(LIGHT_DIRECTIONS_MATRIX_SPR, edF32MATRIX4*),
		SCRATCHPAD_ADDRESS_TYPE(0x70000A00, edF32MATRIX4*));
	edF32Matrix4GetTransposeHard(SCRATCHPAD_ADDRESS_TYPE(0x70000920, edF32MATRIX4*), SCRATCHPAD_ADDRESS_TYPE(0x70000920, edF32MATRIX4*));
	edF32Matrix4CopyHard(SCRATCHPAD_ADDRESS_TYPE(0x70000950, edF32MATRIX4*), SCRATCHPAD_ADDRESS_TYPE(LIGHT_COLOR_MATRIX_SPR, edF32MATRIX4*));

	edF32VECTOR4* vectorB = SCRATCHPAD_ADDRESS_TYPE(0x700009A0, edF32VECTOR4*);
	edF32VECTOR4* vectorD = SCRATCHPAD_ADDRESS_TYPE(0x700009B0, edF32VECTOR4*);
	edF32VECTOR4* vectorC = SCRATCHPAD_ADDRESS_TYPE(LIGHT_AMBIENT_MATRIX_SPR, edF32VECTOR4*);

	vectorB->z = vectorC->z;
	vectorB->x = vectorC->x;
	vectorB->y = vectorC->y;
	vectorB->w = 0.0078125f;
	if (((pDmaMatrix->pMeshTransformData == (MeshTransformDataBase*)0x0) ||
		(pLVar2 = pDmaMatrix->pMeshTransformData->pHierarchySetup, pLVar2 == (ed_3d_hierarchy_setup*)0x0))
		|| (pfVar3 = pLVar2->field_0x10, pfVar3 == (float*)0x0)) {
		gVU1_AnimST_NormalExtruder_Scratch->z = FLOAT_00448a04;
	}
	else {
		gVU1_AnimST_NormalExtruder_Scratch->z = *pfVar3;
	}
	vectorD->z = gVU1_AnimST_NormalExtruder_Scratch->z;
	vectorD->w = gVU1_AnimST_NormalExtruder_Scratch->w;
	vectorD->x = gVU1_AnimST_NormalExtruder_Scratch->x;
	vectorD->y = gVU1_AnimST_NormalExtruder_Scratch->y;
	RENDER_LOG("DMA End ed3DPKTCopyMatrixPacket");
	edDmaSync(SHELLDMA_CHANNEL_VIF0);
	edDmaLoadFromFastRam_nowait(SCRATCHPAD_ADDRESS(MATRIX_PACKET_START_SPR), 0x1c0, pMatrixBuffer);
	return pMatrixBuffer + 7;
}

edpkt_data* ed3DPKTAddMatrixPacket(edpkt_data* pPkt, ed_dma_matrix* pDmaMatrix)
{
	ushort uVar1;
	edNODE* peVar2;
	MeshTransformDataBase* pMVar3;
	float fVar4;
	bool bVar5;
	edpkt_data* pRVar6;
	int iVar7;
	edF32MATRIX4* pMVar8;
	edpkt_data* pRVar9;
	edF32MATRIX4* pMVar10;
	byte bVar11;

	RENDER_LOG("ed3DPKTAddMatrixPacket %p", pDmaMatrix->pMeshTransformData);

	peVar2 = pDmaMatrix->pPrev;
	while (true) {
		uVar1 = peVar2->header.typeField.type;
		bVar11 = 0;
		if (uVar1 == 4) {
			IMPLEMENTATION_GUARD(
				if (((pMVar2->pRenderInput[1].subCommandBufferCount & 0x280) == 0) &&
					(pDmaMatrix->pMeshTransformData != &MeshTransformDataBase_0048cad0)) {
					bVar11 = 1;
				})
		}
		else {
			if (uVar1 == 6) {
				bVar11 = 1;
			}
		}
		if (gFushListCounter == 0xe) {
			IMPLEMENTATION_GUARD(pPkt = FUN_002a9220(pDmaMatrix->pObjToWorld, pPkt);)
		}
		pMVar10 = gPKTMatrixCur;
		pMVar3 = pDmaMatrix->pMeshTransformData;
		if (pMVar3 != (MeshTransformDataBase*)0x0) break;
		pMVar10 = pDmaMatrix->pObjToWorld;
		bVar5 = false;

		PRINT_MATRIX(pMVar10);

		if ((((pMVar10->ac + pMVar10->aa + pMVar10->ab == 1.0f) &&
			(bVar5 = false, pMVar10->bc + pMVar10->ba + pMVar10->bb == 1.0f)) &&
			(bVar5 = false, pMVar10->cc + pMVar10->ca + pMVar10->cb == 1.0f)) &&
			(bVar5 = true, pMVar10->dd + pMVar10->dc + pMVar10->da + pMVar10->db != 1.0f)) {
			bVar5 = false;
		}
		if (!bVar5) {
			IMPLEMENTATION_GUARD(
				pPkt->cmdA = ED_VIF1_SET_TAG_CNT(0x1c);
			pPkt->cmdB = 0;
			*(undefined4*)&pPkt->cmdB = 0;
			*(undefined4*)((ulong)&pPkt->cmdB + 4) = 0x6c1c0006;
			pRVar6 = (edpkt_data*)ed3DPKTCopyMatrixPacket((int)(pPkt + 1), (ed_dma_matrix*)pDmaMatrix, bVar11);
			*(uint*)ScratchpadAnimManager_004494ec.field_0x1c =
				(uint)((ulong)((long)(int)(pPkt + 1) << 0x24) >> 0x24) + 0xa0;
			if (bVar11 != 0) {
				pRVar9 = pRVar6 + 1;
				pRVar6->cmdA = ED_VIF1_SET_TAG_CNT(4);
				pRVar6->cmdB = 0;
				*(undefined4*)&pRVar6->cmdB = 0;
				iVar7 = 8;
				*(undefined4*)((ulong)&pRVar6->cmdB + 4) = 0x6c0403fb;
				pMVar10 = pDmaMatrix->pObjToWorld;
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
		RENDER_LOG("Storing pMeshTransformData %p", &MeshTransformDataBase_0048cad0);
	}
	if (pMVar3->pMatrixPkt == (edF32MATRIX4*)0x0) {
		pMVar3->pMatrixPkt = gPKTMatrixCur;
		gPKTMatrixCur = ed3DPKTCopyMatrixPacket(pMVar10, pDmaMatrix, bVar11);
		if (bVar11 != 0) {
			*gPKTMatrixCur = *pDmaMatrix->pObjToWorld;
			gPKTMatrixCur = gPKTMatrixCur + 1;
		}
	}
	*g_pCurFlareMtx = (edF32MATRIX4*)((uint)((ulong)((long)(int)pDmaMatrix->pMeshTransformData->pMatrixPkt << 0x24) >> 0x24) + 0xa0);

	// This will actually overwrite the cam normal X and Y from the global packet, as that writes from 0x0 -> 0x8, and this starts at 0x6.
	pPkt->cmdA = ED_VIF1_SET_TAG_REF(0x1c, STORE_SECTION(pDmaMatrix->pMeshTransformData->pMatrixPkt) & 0xfffffffU);
	pPkt->asU32[2] = SCE_VIF1_SET_NOP(0);
	pPkt->asU32[3] = SCE_VIF1_SET_UNPACK(0x0006, 0x1c, UNPACK_V4_32, 0);

	if (bVar11 == 0) {
		return pPkt + 1;
	}

	// Check
	IMPLEMENTATION_GUARD();
	pPkt[1].cmdA = ED_VIF1_SET_TAG_REF(0x4, STORE_SECTION(pDmaMatrix->pMeshTransformData->pMatrixPkt + 7) & 0xfffffffU); // | 0x30000004;
	pPkt[1].asU32[2] = SCE_VIF1_SET_NOP(0);
	pPkt[1].asU32[3] = SCE_VIF1_SET_UNPACK(0x03fb, 0x04, UNPACK_V4_32, 0); // 0x6c0403fb00000000;

	return pPkt + 2;
}

edpkt_data* gBackupPKT = NULL;
edpkt_data* gStartPKT_SPR = NULL;

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
		iVar3 = (ulong)&param_1->flags_0x0 + param_1->vifListOffset;
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

edpkt_data* ed3DFlushStripInit(edpkt_data* pPkt, edNODE* pNode, ulong mode)
{
	bool bVar1;
	ushort uVar2;
	ed_3d_strip* peVar3;
	undefined8 uVar4;
	float fVar5;
	float fVar6;
	float fVar7;
	edpkt_data* pAVar8;
	uint uVar9;
	uint uVar10;
	uint uVar11;
	undefined4 uVar12;
	undefined4 uVar13;
	int iVar14;
	edF32MATRIX4* peVar15;
	uint uVar16;
	short* pAnimIndexes;
	edpkt_data* ppuVar15;
	edpkt_data* peVar17;
	edF32VECTOR4* qwc;

	RENDER_LOG("ed3DFlushStripInit %p", pPkt);

	peVar3 = (ed_3d_strip*)pNode->pData;
	uVar2 = pNode->header.typeField.unknown;
	PTR_AnimScratchpad_00449554->field_0x0 = (int)(short)uVar2;
	PTR_AnimScratchpad_00449554->flags = 0;
	PTR_AnimScratchpad_00449554->field_0x8 = 0;
	PTR_AnimScratchpad_00449554->field_0xc = 0;

	if ((gFushListCounter == 0xe) && (FLOAT_00448a04 != 0.0)) {
		PTR_AnimScratchpad_00449554->flags = PTR_AnimScratchpad_00449554->flags | 1;
	}

	pPkt->cmdA = ED_VIF1_SET_TAG_CNT(1);
	pPkt->asU32[2] = SCE_VIF1_SET_NOP(0);
	pPkt->asU32[3] = SCE_VIF1_SET_UNPACK(0x0076, 0x1, UNPACK_V4_32_MASKED, 0); //0x7c010076
	pPkt[1] = PTR_AnimScratchpad_00449554->asPkt;

	pPkt[2].cmdA = ED_VIF1_SET_TAG_CNT(1);
	pPkt[2].asU32[2] = SCE_VIF1_SET_NOP(0);
	pPkt[2].asU32[3] = SCE_VIF1_SET_UNPACK(0x0198, 0x1, UNPACK_V4_32_MASKED, 0); //0x7c01019800000000;
	pPkt[3] = PTR_AnimScratchpad_00449554->asPkt;

	pPkt[4].cmdA = ED_VIF1_SET_TAG_CNT(1);
	pPkt[4].asU32[2] = SCE_VIF1_SET_NOP(0);
	pPkt[4].asU32[3] = SCE_VIF1_SET_UNPACK(0x02ba, 0x1, UNPACK_V4_32_MASKED, 0); //0x7c0102ba00000000;
	pPkt[5] = PTR_AnimScratchpad_00449554->asPkt;

	ppuVar15 = pPkt + 6;
	if ((uVar2 & 2) != 0) {
		if ((peVar3->flags_0x0 & 0x2000000) == 0) {
			ppuVar15->cmdA = ED_VIF1_SET_TAG_REF(1, &g_stGifTAG_FAN_Texture_NoFog);
			ppuVar15->cmdB = SCE_VIF1_SET_UNPACK(0x1, 0x1, UNPACK_V4_32_MASKED, 0);
			ppuVar15 = pPkt + 7;
		}
		else {
			ppuVar15->cmdA = ED_VIF1_SET_TAG_REF(1, &g_stGifTAG_FAN_Gouraud_NoFog);
			ppuVar15->cmdB = SCE_VIF1_SET_UNPACK(0x1, 0x1, UNPACK_V4_32_MASKED, 0);
			ppuVar15 = pPkt + 7;
		}
	}
	if (((peVar3->flags_0x0 & 0x10000) != 0) && (gpCurHierarchy != (MeshTransformDataBase*)0x0)) {
		pAnimIndexes = (short*)((int)peVar3 + peVar3->vifListOffset + -0x30);
		if ((peVar3->flags_0x0 & 0x8000000) == 0) {
			uVar16 = 0x394;
			iVar14 = 0x18;
		}
		else {
			uVar16 = 0x3dc;
			if (mode < 2) {
				uVar16 = 0x3dc;
				iVar14 = 9;
			}
			else {
				iVar14 = 7;
			}
		}
		if ((peVar3->bUseShadowMatrix_0x30 == 0) || (peVar15 = gpCurHierarchy->pShadowAnimMatrix, peVar15 == (edF32MATRIX4*)0x0)) {
			peVar15 = gpCurHierarchy->pAnimMatrix;
		}

		// #VIF_Tidy
		IMPLEMENTATION_GUARD();
		ppuVar15->cmdA = ED_VIF1_SET_TAG_REF(0, 0);
		ppuVar15->cmdB = 0x1100000011000000;
		ppuVar15[1] = g_PKTHeaderRef[4];
		peVar17 = (ppuVar15 + 2);
		ppuVar15 = (edpkt_data*)peVar17;
		if (gBackupPKT != (edpkt_data*)0x0) {
			uint pktLen = PKT_SIZE(peVar17, gStartPKT_SPR);
			edDmaSync(SHELLDMA_CHANNEL_VIF0);
			edDmaLoadFromFastRam_nowait(gStartPKT_SPR, pktLen, PKT_TO_SCRATCHPAD(gBackupPKT));
			gBackupPKT = (edpkt_data*)((ulong)gBackupPKT + (pktLen & 0xfffffff0));
			ppuVar15 = SCRATCHPAD_ADDRESS(0x70000800);
			gStartPKT_SPR = peVar17;
		}
		for (; (bVar1 = iVar14 != 0, iVar14 = iVar14 + -1, bVar1 && (-1 < *pAnimIndexes)); pAnimIndexes = pAnimIndexes + 1)
		{
			ppuVar15->cmdA = ED_VIF1_SET_TAG_CNT(4);
			ppuVar15->cmdB = (ulong)(uVar16 | 0x6c040000) << 0x20;
			edF32Matrix4CopyHard(((edF32MATRIX4*)ppuVar15 + 1), (peVar15 + *pAnimIndexes));
			ppuVar15 = ppuVar15 + 5;
			uVar16 = uVar16 + 4;
		}
		if (gBackupPKT != (edpkt_data*)0x0) {
			edDmaSync(SHELLDMA_CHANNEL_VIF0);
			uint pktLen = PKT_SIZE(ppuVar15, SCRATCHPAD_ADDRESS(0x7000080));
			edDmaLoadFromFastRam_nowait(SCRATCHPAD_ADDRESS(0x70000800), pktLen, PKT_TO_SCRATCHPAD(gBackupPKT));
			edDmaSync(SHELLDMA_CHANNEL_VIF0);
			ppuVar15 = SCRATCHPAD_ADDRESS(0x70001000);
			gStartPKT_SPR = SCRATCHPAD_ADDRESS(0x70001000);
			gBackupPKT = (edpkt_data*)((ulong)gBackupPKT + (pktLen & 0xfffffff0));
		}
	}
	if (((gpCurHierarchy == (MeshTransformDataBase*)0x0) || (gpCurHierarchy == (MeshTransformDataBase*)0x0000ffff))
		|| (gpCurHierarchy == &MeshTransformDataBase_0048cad0)) {
		peVar3->flags_0x0 = peVar3->flags_0x0 & 0xfffffff7;
	}
	else {
		if ((BYTE_004493b4 == 0) && ((gpCurHierarchy->flags_0x9e & 2) == 0)) {
			peVar3->flags_0x0 = peVar3->flags_0x0 & 0xfffffff7;
		}
		else {
			IMPLEMENTATION_GUARD(
				peVar3->flags_0x0 = peVar3->flags_0x0 | 8;
			*(ulong*)&((edF32VECTOR4*)ppuVar15)->x = ED_VIF1_SET_TAG_REF(0, 0);
			*(ulong*)&((edF32VECTOR4*)ppuVar15)->z = 0x1100000011000000;
			if ((BYTE_004493b8 == 0) && ((gCurScene->flags_0x4 & 0x200) == 0)) {
				INT_004489a4 = 0xfffe;
				INT_004489a8 = 0xfffe;
			}
			else {
				INT_004489a4 = 0x7ffe;
				INT_004489a8 = 0x7ffe;
			}
			pBFCVect.w = (float)INT_004489a4;
			pBFCVect.y = (float)INT_004489a8;
			*(ulong*)&((edF32VECTOR4*)ppuVar15)[1].x = ED_VIF1_SET_TAG_CNT(1);
			*(ulong*)&((edF32VECTOR4*)ppuVar15)[1].z = 0x7c01003d00000000;
			fVar7 = pBFCVect.w;
			fVar6 = pBFCVect.z;
			fVar5 = pBFCVect.y;
			((edF32VECTOR4*)ppuVar15)[2].x = pBFCVect.x;
			*(float*)((ulong) & ((edF32VECTOR4*)ppuVar15)[2].x + 4) = fVar5;
			((edF32VECTOR4*)ppuVar15)[2].z = fVar6;
			*(float*)((ulong) & ((edF32VECTOR4*)ppuVar15)[2].z + 4) = fVar7;
			ppuVar15 = (edpkt_data*)((edF32VECTOR4*)ppuVar15 + 3);)
		}
	}
	if (((peVar3->flags_0x0 & 0x100) != 0) || (gFushListCounter == 0xe)) {
		// #VIF_Tidy
		IMPLEMENTATION_GUARD();
		*(ulong*)&((edF32VECTOR4*)ppuVar15)->x = ED_VIF1_SET_TAG_CNT(1);
		*(ulong*)&((edF32VECTOR4*)ppuVar15)->z = 0;
		((edF32VECTOR4*)ppuVar15)->z = 0.0;
		*(undefined4*)((ulong) & ((edF32VECTOR4*)ppuVar15)->z + 4) = 0x6c010021;
		((edF32VECTOR4*)ppuVar15)[1].x = gVU1_AnimST_NormalExtruder_Scratch->x;
		*(float*)((ulong) & ((edF32VECTOR4*)ppuVar15)[1].x + 4) = gVU1_AnimST_NormalExtruder_Scratch->y;
		((edF32VECTOR4*)ppuVar15)[1].z = gVU1_AnimST_NormalExtruder_Scratch->z;
		*(float*)((ulong) & ((edF32VECTOR4*)ppuVar15)[1].z + 4) = gVU1_AnimST_NormalExtruder_Scratch->w;
		ppuVar15 = (edpkt_data*)((edF32VECTOR4*)ppuVar15 + 2);
	}
	return ppuVar15;
}

struct BFCVect {
	uint alphaON;
	float x;
	float y;
	float z;
};

BFCVect pBFCVect = {};

void ed3DFlushStrip(edNODE* pNode)
{
	undefined8 uVar1;
	uint uVar5;
	edpkt_data* ppuVar6;
	//ed_g3d_Anim_def* iVar6;
	uint uVar7;
	uint uVar8;
	byte bVar9;
	ulong uVar10;
	float fVar11;
	uint uVar12;
	char* iVar13;
	int* piVar14;
	char* iVar15;
	undefined4 uVar16;
	undefined4 uVar17;
	edpkt_data* pRVar18;
	edpkt_data* ppuVar19;
	edpkt_data* ppuVar20;
	int iVar2;
	edNODE* pMVar19;
	char* pVifList;
	int* piVar21;
	int* piVar22;
	uint uVar23;
	uint uVar24;
	uint pRVar25;
	uint uVar26;
	float fVar27;
	int* local_30;
	uint local_20;
	bool bVar1;
	float fVar3;
	ed_3d_strip* pRenderInput;
	char* pcVar4;
	ulong uVar2;

	ED3D_LOG(LogLevel::VeryVerbose, "ed3DFlushStrip");

	pRVar18 = g_VifRefPktCur;
	pRenderInput = (ed_3d_strip*)pNode->pData;
	pMVar19 = pNode->pNext;
	pVifList = ((char*)pRenderInput) + pRenderInput->vifListOffset;
	uVar5 = ed3DFlushStripGetIncPacket(pRenderInput, 0, 0);
	uint globalAlhaON = (uint)gGlobalAlhaON;
	if (globalAlhaON != 0x80) {
		// #VIF_Tidy
		IMPLEMENTATION_GUARD();
		pNode->header.typeField.unknown = pNode->header.typeField.unknown | 0x20;;
		pBFCVect.alphaON = globalAlhaON;
		pRVar18->cmdA = ED_VIF1_SET_TAG_CNT(1);
		pRVar18->cmdB = 0x7c01003d00000000;
		fVar3 = pBFCVect.z;
		fVar27 = pBFCVect.y;
		fVar11 = pBFCVect.x;
		*(float*)&pRVar18[1].cmdA = pBFCVect.x;
		*(float*)((ulong)&pRVar18[1].cmdA + 4) = fVar11;
		*(float*)&pRVar18[1].cmdB = fVar27;
		*(float*)((ulong)&pRVar18[1].cmdB + 4) = fVar3;
		pRVar18 = pRVar18 + 2;
	}
	if (pMVar19 == (edNODE*)0x0) {
		uVar8 = (uint)(ushort)pRenderInput->meshSectionCount_0x3a % 3;
		iVar2 = (ushort)pRenderInput->meshSectionCount_0x3a - uVar8;
		ppuVar20 = ed3DFlushStripInit(pRVar18, pNode, 1);

		if (SCRATCHPAD_ADDRESS(0x70003000) < ppuVar20 + (uint)(ushort)pRenderInput->meshSectionCount_0x3a * 2) {
			uint pktLen = PKT_SIZE(ppuVar20, gStartPKT_SPR);
			edDmaLoadFromFastRam(gStartPKT_SPR, pktLen, PKT_TO_SCRATCHPAD(gBackupPKT));
			ppuVar20 = SCRATCHPAD_ADDRESS(0x70001000);
			gStartPKT_SPR = SCRATCHPAD_ADDRESS(0x70001000);
			gBackupPKT = (edpkt_data*)((ulong)gBackupPKT + (pktLen & 0xfffffff0));
		}
		pcVar4 = (char*)ed3DVU1Addr_Scratch;
		if ((pRenderInput->flags_0x0 & 4) == 0) {
			ppuVar6 = ppuVar20;
			while (bVar1 = uVar8 != 0, uVar8 = uVar8 - 1, bVar1) {
				ppuVar6->cmdA = ED_VIF1_SET_TAG_REF(0, 0);
				ppuVar6->asU32[2] = SCE_VIF1_SET_BASE(ed3DVU1Addr_Scratch[ed3DVU1BufferCur] + 1, 0);
				ppuVar6->asU32[3] = SCE_VIF1_SET_OFFSET(0, 0);

				ED3D_LOG(LogLevel::VeryVerbose, "Send strip Vif list A: %p", pVifList);

				// Send strip Vif list.
				ppuVar6[1].asU32[0] = SCE_VIF1_SET_DIRECT(0, 0);
				ppuVar6[1].asU32[1] = (uint)pVifList & 0xfffffff;
				ppuVar6[1].asU32[2] = 0;
				ppuVar6[1].asU32[3] = SCE_VIF1_SET_ITOP(ed3DVU1Addr_Scratch[ed3DVU1BufferCur] + 1, 0);

#ifdef  PLATFORM_WIN
				// #LETS DRAAAAAW
				VU1Emu::SetVifItop(ed3DVU1Addr_Scratch[ed3DVU1BufferCur] + 1);
				VU1Emu::UpdateMemory(g_pStartPktData, gBackupPKT);
				VU1Emu::UpdateMemory(gStartPKT_SPR, ppuVar6);
				VU1Emu::ProcessVifList((edpkt_data*)pVifList);
#endif //  PLATFORM_WIN

				pVifList = pVifList + uVar5 * 0x10;

				ppuVar6 = ppuVar6 + 2;
				if (ed3DVU1BufferCur == 2) {
					ed3DVU1BufferCur = 0;
				}
				else {
					ed3DVU1BufferCur = ed3DVU1BufferCur + 1;
				}
			}
			piVar14 = (int*)ed3DVU1AddrWithBufCur_Scratch[ed3DVU1BufferCur];
			for (; iVar2 != 0; iVar2 = iVar2 + -3) {

#ifdef  PLATFORM_WIN
				// #LETS DRAAAAAW
				VU1Emu::UpdateMemory(g_pStartPktData, gBackupPKT);
				VU1Emu::UpdateMemory(gStartPKT_SPR, ppuVar6);
#endif //  PLATFORM_WIN

				iVar13 = pVifList + uVar5 * 0x10;
				iVar15 = pVifList + uVar5 * 0x20;

				ED3D_LOG(LogLevel::VeryVerbose, "Send strip Vif list B: %p", pVifList);

				ppuVar6->cmdA = ED_VIF1_SET_TAG_REF(0, 0);
				ppuVar6->asU32[2] = SCE_VIF1_SET_BASE(piVar14[0] + 1, 0);
				ppuVar6->asU32[3] = SCE_VIF1_SET_OFFSET(0, 0);

				ppuVar6[1].asU32[0] = SCE_VIF1_SET_DIRECT(0, 0);
				ppuVar6[1].asU32[1] = (uint)pVifList & 0xfffffff;
				ppuVar6[1].asU32[2] = 0;
				ppuVar6[1].asU32[3] = SCE_VIF1_SET_ITOP(piVar14[0] + 1, 0);

#ifdef  PLATFORM_WIN
				// #LETS DRAAAAAW
				VU1Emu::SetVifItop(piVar14[0] + 1);
				VU1Emu::ProcessVifList((edpkt_data*)pVifList);
#endif //  PLATFORM_WIN

				ppuVar6[2].cmdA = ED_VIF1_SET_TAG_REF(0, 0);
				ppuVar6[2].asU32[2] = SCE_VIF1_SET_BASE(piVar14[1] + 1, 0);
				ppuVar6[2].asU32[3] = SCE_VIF1_SET_OFFSET(0, 0);

				ppuVar6[3].asU32[0] = SCE_VIF1_SET_DIRECT(0, 0);
				ppuVar6[3].asU32[1] = (uint)iVar13 & 0xfffffff;
				ppuVar6[3].asU32[2] = 0;
				ppuVar6[3].asU32[3] = SCE_VIF1_SET_ITOP(piVar14[1] + 1, 0);

#ifdef  PLATFORM_WIN
				// #LETS DRAAAAAW
				VU1Emu::SetVifItop(piVar14[1] + 1);
				VU1Emu::ProcessVifList((edpkt_data*)iVar13);
#endif //  PLATFORM_WIN

				ppuVar6[4].cmdA = ED_VIF1_SET_TAG_REF(0, 0);
				ppuVar6[4].asU32[2] = SCE_VIF1_SET_BASE(piVar14[2] + 1, 0);
				ppuVar6[4].asU32[3] = SCE_VIF1_SET_OFFSET(0, 0);

				ppuVar6[5].asU32[0] = SCE_VIF1_SET_DIRECT(0, 0);
				ppuVar6[5].asU32[1] = (uint)iVar15 & 0xfffffff;
				ppuVar6[5].asU32[2] = 0;
				ppuVar6[5].asU32[3] = SCE_VIF1_SET_ITOP(piVar14[2] + 1, 0);

#ifdef  PLATFORM_WIN
				// #LETS DRAAAAAW
				VU1Emu::SetVifItop(piVar14[2] + 1);
				VU1Emu::ProcessVifList((edpkt_data*)iVar15);
#endif //  PLATFORM_WIN

				ppuVar6 = ppuVar6 + 6;
				pVifList = pVifList + uVar5 * 0x30;

			}
		}
		else {
			IMPLEMENTATION_GUARD(
				uVar23 = 0x48;
			iVar6 = ed3DG3DAnimGet(pRenderInput);
			uVar8 = ed3DG3DNbMaxBaseRGBA(pRenderInput);
			fVar11 = iVar6->field_0x10;
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
			iVar2 = *piVar14;
			piVar22 = piVar14 + uVar7 * iVar2 * 4 + 4;
			piVar21 = piVar22;
			if ((iVar6->field_0x4 & 8) == 0) {
				uVar26 = uVar8 - 1;
				if (uVar7 < uVar26) {
					piVar21 = piVar14 + (uVar7 + 1) * iVar2 * 4 + 4;
				}
				else {
					if ((uVar7 != uVar26) && (piVar21 = (int*)0x0, uVar7 == uVar8)) {
						fVar11 = 1.0;
						piVar22 = piVar14 + (uVar8 - 2) * iVar2 * 4 + 4;
						piVar21 = piVar14 + uVar26 * iVar2 * 4 + 4;
					}
				}
			}
			else {
				uVar26 = uVar8 - 1;
				if (uVar7 < uVar26) {
					piVar21 = piVar14 + (uVar7 + 1) * iVar2 * 4 + 4;
				}
				else {
					if ((uVar7 != uVar26) && (piVar21 = (int*)0x0, uVar7 == uVar8)) {
						fVar11 = 1.0;
						piVar22 = piVar14 + (uVar8 - 2) * iVar2 * 4 + 4;
						piVar21 = piVar14 + uVar26 * iVar2 * 4 + 4;
					}
				}
			}

			// #VIF_Tidy
			IMPLEMENTATION_GUARD();
			ppuVar20->cmdA = ED_VIF1_SET_TAG_REF(0, 0);
			ppuVar20->cmdB = 0x1300000000000000;
			ppuVar20[1].cmdA = ED_VIF1_SET_TAG_REF(0, 0);
			ppuVar20[1].cmdB = 0x1100000011000000;
			pBFCVect.z = 256.0 - fVar11 * 256.0;
			ppuVar20[2].cmdA = ED_VIF1_SET_TAG_CNT(1);
			ppuVar20[2].cmdB = 0x7c01003d00000000;
			fVar3 = pBFCVect.w;
			fVar27 = pBFCVect.z;
			fVar11 = pBFCVect.y;
			*(float*)&ppuVar20[3].cmdA = pBFCVect.x;
			*(float*)((ulong)&ppuVar20[3].cmdA + 4) = fVar11;
			*(float*)&ppuVar20[3].cmdB = fVar27;
			*(float*)((ulong)&ppuVar20[3].cmdB + 4) = fVar3;
			ppuVar20[4].cmdA = ED_VIF1_SET_TAG_REF(0, 0);
			ppuVar20[4].cmdB = 0x1100000011000000;
			pcVar4 = ScratchpadAnimManager_004494ec.field_0x34;
			pRVar18 = ppuVar20 + 5;
			for (uVar8 = 0; uVar8 < (ushort)pRenderInput->meshSectionCount_0x3a; uVar8 = uVar8 + 1) {
				if (uVar8 == (ushort)pRenderInput->meshSectionCount_0x3a - 1) {
					for (uVar23 = (uint)pRenderInput->field_0x38; (uVar23 & 3) != 0; uVar23 = uVar23 + 1) {
					}
				}
				uVar2 = (long)(int)(uVar23 >> 2) | 0x30000000;
				uVar26 = (uint)piVar22 & 0xfffffff;
				pRVar18->cmdA = ED_VIF1_SET_TAG_REF(0, 0);
				uVar7 = ed3DVU1BufferCur;
				uVar12 = (uint)piVar21 & 0xfffffff;
				uVar24 = pVifList & 0xfffffff;
				iVar2 = ed3DVU1BufferCur * 4;
				piVar22 = piVar22 + 0x48;
				piVar21 = piVar21 + 0x48;
				pVifList = pVifList + uVar5 * 0x10;
				pRVar18->cmdB = (long)(*(int*)(pcVar4 + iVar2) + 1) & 0xffffffffU | 0x200000003000000;
				pRVar18[1].cmdA = ED_VIF1_SET_TAG_REF(1, &g_stVertexRGBAHeader);
				pRVar18[1].cmdB = 0;
				pRVar18[2].cmdA = (ulong)uVar26 << 0x20 | uVar2;
				pRVar18[2].cmdB = ((long)(int)(uVar23 << 0x10) & 0xffffffffU | 0x7e00c002) << 0x20;
				pRVar18[3].cmdA = ED_VIF1_SET_TAG_REF(1, &g_stNormalHeader);
				pRVar18[3].cmdB = 0;
				pRVar18[4].cmdA = (ulong)uVar12 << 0x20 | uVar2;
				pRVar18[4].cmdB = ((long)(int)(uVar23 << 0x10) & 0xffffffffU | 0x7e00c0d9) << 0x20;
				pRVar18[5].cmdA = (ulong)uVar24 << 0x20 | 0x50000000;
				pRVar18[5].cmdB = ((long)(*(int*)(pcVar4 + iVar2) + 1) & 0xffffffffU | 0x4000000) << 0x20;
				pRVar18 = pRVar18 + 6;
				if (uVar7 == 2) {
					ed3DVU1BufferCur = 0;
				}
				else {
					ed3DVU1BufferCur = ed3DVU1BufferCur + 1;
				}
			}
			pRVar18->cmdA = ED_VIF1_SET_TAG_REF(0, 0);
			pRVar18->cmdB = 0x1300000000000000;
			ppuVar6 = pRVar18 + 1;)
		}
	}
	else {
		uVar8 = pRenderInput->flags_0x0;
		pRVar25 = 0;
		uVar23 = 0;
		short meshSectionCount = pRenderInput->meshSectionCount_0x3a;
		ppuVar6 = ed3DFlushStripInit(pRVar18, pNode, 1);
		if (SCRATCHPAD_ADDRESS(0x70003000) < ppuVar6 + meshSectionCount * 4) {
			uint pktLen = PKT_SIZE(ppuVar6, gStartPKT_SPR);
			edDmaLoadFromFastRam(gStartPKT_SPR, pktLen, PKT_TO_SCRATCHPAD(gBackupPKT));
			ppuVar6 = SCRATCHPAD_ADDRESS(0x70001000);
			gStartPKT_SPR = SCRATCHPAD_ADDRESS(0x70001000);
			gBackupPKT = (edpkt_data*)((ulong)gBackupPKT + (pktLen & 0xfffffff0));
		}
		if ((pRenderInput->flags_0x0 & 4) == 0) {
			while (pRVar25 < meshSectionCount) {
				bVar9 = pMVar19->header.byteFlags[0] & 3;
				if (bVar9 == 1) {
					uVar8 = uVar8 & 0xfffffffd | 1;
				LAB_002a5dac:
					if (SCRATCHPAD_ADDRESS(0x70003fff) < ppuVar6 + 0x80) {
						uint pktLen = PKT_SIZE(ppuVar6, gStartPKT_SPR);
						edDmaLoadFromFastRam(gStartPKT_SPR, pktLen, PKT_TO_SCRATCHPAD(gBackupPKT));
						ppuVar6 = SCRATCHPAD_ADDRESS(0x70001000);
						gStartPKT_SPR = SCRATCHPAD_ADDRESS(0x70001000);
						gBackupPKT = (edpkt_data*)((ulong)gBackupPKT + (pktLen & 0xfffffff0));
					}

					PTR_AnimScratchpad_00449554->field_0x0 = uVar8;

					ppuVar6->cmdA = gOptionFlagCNT[ed3DVU1BufferCur].cmdA;
					ppuVar6->cmdB = gOptionFlagCNT[ed3DVU1BufferCur].cmdB;

					ppuVar6[1] = PTR_AnimScratchpad_00449554->asPkt;

					ppuVar6[2] = gRefOptionsforVU1Buf[ed3DVU1BufferCur];

					ED3D_LOG(LogLevel::VeryVerbose, "Send strip Vif list C: %p", pVifList);

					// Send strip Vif list.
					ppuVar6[3].asU32[0] = SCE_VIF1_SET_DIRECT(0, 0);
					ppuVar6[3].asU32[1] = (uint)pVifList & 0xfffffff;
					ppuVar6[3].asU32[2] = 0;
					ppuVar6[3].asU32[3] = SCE_VIF1_SET_ITOP(ed3DVU1Addr_Scratch[ed3DVU1BufferCur] + 1, 0);

					ppuVar6 = ppuVar6 + 4;

#ifdef  PLATFORM_WIN
					// #LETS DRAAAAAW
					VU1Emu::SetVifItop(ed3DVU1Addr_Scratch[ed3DVU1BufferCur] + 1);
					VU1Emu::UpdateMemory(g_pStartPktData, gBackupPKT);
					VU1Emu::UpdateMemory(gStartPKT_SPR, ppuVar6);
					VU1Emu::ProcessVifList((edpkt_data*)pVifList);
#endif //  PLATFORM_WIN

					if (ed3DVU1BufferCur == 2) {
						ed3DVU1BufferCur = 0;
					}
					else {
						ed3DVU1BufferCur = ed3DVU1BufferCur + 1;
					}
				}
				else {
					if ((pMVar19->header.byteFlags[0] & 3) != 0) {
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
					if (SCRATCHPAD_ADDRESS(0x70003fff) < ppuVar6 + 0x80) {
						uint pktLen = PKT_SIZE(ppuVar6,gStartPKT_SPR);
						edDmaLoadFromFastRam(gStartPKT_SPR, pktLen, PKT_TO_SCRATCHPAD(gBackupPKT));
						ppuVar6 = SCRATCHPAD_ADDRESS(0x70001000);
						gStartPKT_SPR = SCRATCHPAD_ADDRESS(0x70001000);
						gBackupPKT = (edpkt_data*)((ulong)gBackupPKT + (pktLen & 0xfffffff0));
					}

					ppuVar6->cmdA = ED_VIF1_SET_TAG_CNT(1);
					ppuVar6->asU32[2] = 0;
					ppuVar6->asU32[3] = SCE_VIF1_SET_UNPACK(0x003d, 0x01, UNPACK_V4_32_MASKED, 0);
					ppuVar6[1].asVector = *reinterpret_cast<edF32VECTOR4*>(&pBFCVect);

					ppuVar6 = ppuVar6 + 2;
				}
				uVar23 = uVar23 + 1;
				pVifList = pVifList + uVar5 * 0x10;
				pRVar25 = pRVar25 + 1;
				pMVar19->header.byteFlags[0] = pMVar19->header.byteFlags[0] >> 2;
				if (3 < uVar23) {
					uVar23 = 0;
					pMVar19 = (edNODE*)((ulong)&pMVar19->header + 1);
				}
			}
		}
		else {
			IMPLEMENTATION_GUARD(
				local_20 = 0x48;
			iVar6 = ed3DG3DAnimGet(pRenderInput);
			uVar7 = ed3DG3DNbMaxBaseRGBA(pRenderInput);
			fVar11 = iVar6->field_0x10;
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
			iVar2 = *piVar14;
			local_30 = piVar14 + uVar26 * iVar2 * 4 + 4;
			piVar21 = local_30;
			if ((iVar6->field_0x4 & 8) == 0) {
				uVar12 = uVar7 - 1;
				if (uVar26 < uVar12) {
					piVar21 = piVar14 + (uVar26 + 1) * iVar2 * 4 + 4;
				}
				else {
					if ((uVar26 != uVar12) && (piVar21 = (int*)0x0, uVar26 == uVar7)) {
						fVar11 = 1.0;
						local_30 = piVar14 + (uVar7 - 2) * iVar2 * 4 + 4;
						piVar21 = piVar14 + uVar12 * iVar2 * 4 + 4;
					}
				}
			}
			else {
				uVar12 = uVar7 - 1;
				if (uVar26 < uVar12) {
					piVar21 = piVar14 + (uVar26 + 1) * iVar2 * 4 + 4;
				}
				else {
					if ((uVar26 != uVar12) && (piVar21 = (int*)0x0, uVar26 == uVar7)) {
						fVar11 = 1.0;
						local_30 = piVar14 + (uVar7 - 2) * iVar2 * 4 + 4;
						piVar21 = piVar14 + uVar12 * iVar2 * 4 + 4;
					}
				}
			}

			// #VIF_Tidy
			IMPLEMENTATION_GUARD();
			ppuVar6->cmdA = ED_VIF1_SET_TAG_REF(0, 0);
			ppuVar6->cmdB = 0x1300000000000000;
			ppuVar6[1].cmdA = ED_VIF1_SET_TAG_REF(0, 0);
			ppuVar6[1].cmdB = 0x1100000011000000;
			pBFCVect.z = 256.0 - fVar11 * 256.0;
			ppuVar6[2].cmdA = ED_VIF1_SET_TAG_CNT(1);
			ppuVar6[2].cmdB = 0x7c01003d00000000;
			fVar3 = pBFCVect.w;
			fVar27 = pBFCVect.z;
			fVar11 = pBFCVect.y;
			*(float*)&ppuVar6[3].cmdA = pBFCVect.x;
			*(float*)((ulong)&ppuVar6[3].cmdA + 4) = fVar11;
			*(float*)&ppuVar6[3].cmdB = fVar27;
			*(float*)((ulong)&ppuVar6[3].cmdB + 4) = fVar3;
			ppuVar6[4].cmdA = ED_VIF1_SET_TAG_REF(0, 0);
			ppuVar6[4].cmdB = 0x1100000011000000;
			ppuVar19 = ppuVar6 + 5;
			while (pRVar25 < ppuVar6) {
				bVar9 = *(byte*)&pMVar19->header & 3;
				if (bVar9 == 1) {
					uVar8 = uVar8 & 0xfffffffd | 1;
				LAB_002a5a74:
					if (SCRATCHPAD_ADDRESS(0x70003fff) < ppuVar19 + 4) {
						IMPLEMENTATION_GUARD();
						pRVar18 = (edpkt_data*)((int)ppuVar19 - (int)gStartPKT_SPR);
						edDmaLoadFromFastRam(gStartPKT_SPR, (uint)pRVar18, (uint)((ulong)((long)(int)gBackupPKT << 0x24) >> 0x24));
						ppuVar19 = edpkt_data_ARRAY_70001000;
						gStartPKT_SPR = edpkt_data_ARRAY_70001000;
						gBackupPKT = (edpkt_data*)((ulong)&gBackupPKT->cmdA + ((uint)pRVar18 & 0xfffffff0));
					}
					if (pRVar25 == (ushort)pRenderInput->meshSectionCount_0x3a - 1) {
						for (local_20 = (uint)pRenderInput->field_0x38; (local_20 & 3) != 0; local_20 = local_20 + 1) {
						}
					}

					// #VIF_Tidy
					IMPLEMENTATION_GUARD();
					uVar10 = (long)(int)(local_20 >> 2) | ED_VIF1_SET_TAG_REF(0, 0);
					gScratchPkt->field_0x0 = uVar8;
					uVar7 = ed3DVU1BufferCur;
					iVar15 = ed3DVU1BufferCur * 0x10;
					iVar2 = ed3DVU1BufferCur * 2;
					uVar2 = edpkt_data_ARRAY_0048c5f0[ed3DVU1BufferCur].cmdA;
					uVar16 = *(undefined4*)&edpkt_data_ARRAY_0048c5f0[ed3DVU1BufferCur].cmdB;
					uVar17 = *(undefined4*)((ulong)&edpkt_data_ARRAY_0048c5f0[ed3DVU1BufferCur].cmdB + 4);
					iVar13 = ed3DVU1BufferCur * 4;
					*(int*)&ppuVar19->cmdA = (int)uVar2;
					*(int*)((ulong)&ppuVar19->cmdA + 4) = (int)(uVar2 >> 0x20);
					*(undefined4*)&ppuVar19->cmdB = uVar16;
					*(undefined4*)((ulong)&ppuVar19->cmdB + 4) = uVar17;
					uVar1 = *(undefined8*)ScratchpadAnimManager_004494ec.pAnimScratchpad;
					uVar16 = gScratchPkt->field_0x8;
					uVar17 = gScratchPkt->field_0xc;
					*(int*)&ppuVar19[1].cmdA = (int)uVar1;
					*(int*)((ulong)&ppuVar19[1].cmdA + 4) = (int)((ulong)uVar1 >> 0x20);
					*(undefined4*)&ppuVar19[1].cmdB = uVar16;
					*(undefined4*)((ulong)&ppuVar19[1].cmdB + 4) = uVar17;
					uVar2 = gRefOptionsforVU1Buf[iVar2];
					uVar16 = *(undefined4*)(gRefOptionsforVU1Buf + uVar7 * 2 + 1);
					uVar17 = *(undefined4*)((int)gRefOptionsforVU1Buf + iVar15 + 0xc);
					*(int*)&ppuVar19[2].cmdA = (int)uVar2;
					*(int*)((ulong)&ppuVar19[2].cmdA + 4) = (int)(uVar2 >> 0x20);
					*(undefined4*)&ppuVar19[2].cmdB = uVar16;
					*(undefined4*)((ulong)&ppuVar19[2].cmdB + 4) = uVar17;
					ppuVar19[3].cmdA = ED_VIF1_SET_TAG_REF(0, 0);
					piVar14 = (int*)(ScratchpadAnimManager_004494ec.field_0x34 + iVar13);
					ppuVar19[3].cmdB = (long)(*piVar14 + 1) & 0xffffffffU | 0x200000003000000;
					ppuVar19[4].cmdA = ED_VIF1_SET_TAG_REF(1, &g_stVertexRGBAHeader);
					ppuVar19[4].cmdB = 0;
					ppuVar19[5].cmdA = ((long)(int)local_30 & 0xfffffffU) << 0x20 | uVar10;
					ppuVar19[5].cmdB = ((long)(int)(local_20 << 0x10) & 0xffffffffU | 0x7e00c002) << 0x20;
					ppuVar19[6].cmdA = ED_VIF1_SET_TAG_REF(1, &g_stNormalHeader);
					ppuVar19[6].cmdB = 0;
					ppuVar19[7].cmdA = (ulong)((uint)piVar21 & 0xfffffff) << 0x20 | uVar10;
					ppuVar19[7].cmdB = ((long)(int)(local_20 << 0x10) & 0xffffffffU | 0x7e00c0d9) << 0x20;
					ppuVar19[8].cmdA = (ulong)(pVifList & 0xfffffff) << 0x20 | 0x50000000;
					ppuVar19[8].cmdB = ((long)(*piVar14 + 1) & 0xffffffffU | 0x4000000) << 0x20;
					ppuVar19 = ppuVar19 + 9;
					if (ed3DVU1BufferCur == 2) {
						ed3DVU1BufferCur = 0;
					}
					else {
						ed3DVU1BufferCur = ed3DVU1BufferCur + 1;
					}
				}
				else {
					if ((*(byte*)&pMVar19->header & 3) != 0) {
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

					// #VIF_Tidy
					IMPLEMENTATION_GUARD();
					ppuVar19->cmdA = ED_VIF1_SET_TAG_CNT(1);
					ppuVar19->cmdB = 0x7c01003d00000000;
					fVar3 = pBFCVect.w;
					fVar27 = pBFCVect.z;
					fVar11 = pBFCVect.y;
					*(float*)&ppuVar19[1].cmdA = pBFCVect.x;
					*(float*)((ulong)&ppuVar19[1].cmdA + 4) = fVar11;
					*(float*)&ppuVar19[1].cmdB = fVar27;
					*(float*)((ulong)&ppuVar19[1].cmdB + 4) = fVar3;
					ppuVar19 = ppuVar19 + 2;
				}
				uVar23 = uVar23 + 1;
				piVar21 = piVar21 + 0x48;
				pRVar25 = pRVar25 + 1;
				local_30 = local_30 + 0x48;
				pVifList = pVifList + uVar5 * 0x10;
				*(char*)&pMVar19->header = (char)((int)(uint) * (byte*)&pMVar19->header >> 2);
				if (3 < uVar23) {
					uVar23 = 0;
					pMVar19 = (edNODE*)((ulong)&pMVar19->header + 1);
				}
			}
			ppuVar19->cmdA = ED_VIF1_SET_TAG_REF(0, 0);
			ppuVar19->cmdB = 0x1300000000000000;
			ppuVar6 = ppuVar19 + 1;)
		}
	}
	if (ed3DVU1BufferCur == 2) {
		ed3DVU1Buffer[0] = ed3DVU1Addr_Scratch[1];
		ed3DVU1Buffer[1] = ed3DVU1Addr_Scratch[2];
		ed3DVU1Buffer[2] = *ed3DVU1Addr_Scratch;
	}
	else {
		if (ed3DVU1BufferCur == 1) {
			ed3DVU1Buffer[0] = *ed3DVU1Addr_Scratch;
			ed3DVU1Buffer[1] = ed3DVU1Addr_Scratch[1];
			ed3DVU1Buffer[2] = ed3DVU1Addr_Scratch[2];
		}
		else {
			if (ed3DVU1BufferCur == 0) {
				ed3DVU1Buffer[0] = ed3DVU1Addr_Scratch[2];
				ed3DVU1Buffer[1] = *ed3DVU1Addr_Scratch;
				ed3DVU1Buffer[2] = ed3DVU1Addr_Scratch[1];
			}
		}
	}
	if ((*gShadowRenderMask != 0) && (pRenderInput->bUseShadowMatrix_0x30 != 0)) {
		ppuVar6->cmdA = ED_VIF1_SET_TAG_REF(0, 0);
		ppuVar6->cmdB = 0;
		ppuVar6->asU32[2] = SCE_VIF1_SET_FLUSHA(0);
		ppuVar6 = ppuVar6 + 1;
	}
	g_VifRefPktCur = ppuVar6;
	return;
}

void ed3DFlushStripList(edLIST* pList, ed_g2d_material* pMaterial)
{
	short sVar1;
	undefined4* puVar2;
	bool bVar3;
	edpkt_data* peVar4;
	edLIST* pRVar5;

	ED3D_LOG(LogLevel::Verbose, "ed3DFlushStripList");

	pRVar5 = (edLIST*)pList->pPrev;

	if (pRVar5 != pList) {
		*gMaterialNbPrimSend = *gMaterialNbPrimSend + 1;
	}

	if ((pMaterial == &gMaterial_Render_Zbuffer_Only_Cluster) || (pMaterial == &gMaterial_Render_Zbuffer_Only)) {
		gBackupPKT = (edpkt_data*)0x0;
		if (gShadowFlushMode == 0) {
			if (pMaterial->count_0x0 < 2) {
				bVar3 = false;
				while (peVar4 = g_VifRefPktCur, pRVar5 != pList) {
					sVar1 = pRVar5->header.typeField.type;
					if (sVar1 == 4) {
						IMPLEMENTATION_GUARD();
					LAB_002a7d54:
						pRVar5 = (edLIST*)pRVar5->pPrev;
					}
					else {
						if ((sVar1 == 5) || (sVar1 == 6)) {
							IMPLEMENTATION_GUARD();
							puVar2 = (undefined4*)pRVar5->pData;
							*(undefined4*)&g_VifRefPktCur->cmdA = *puVar2;
							*(undefined4*)((int)&g_VifRefPktCur->cmdA + 4) = puVar2[1];
							*(undefined4*)&g_VifRefPktCur->cmdB = puVar2[2];
							*(undefined4*)((int)&g_VifRefPktCur->cmdB + 4) = puVar2[3];
							g_VifRefPktCur = g_VifRefPktCur + 1;
							goto LAB_002a7d54;
						}
						if (sVar1 == 3) {
							IMPLEMENTATION_GUARD(
								/* WARNING: Load size is inaccurate */
								if ((*pRVar5->pData & 0x10000) == 0) {
									ed3DFlushStripShadowRender((MeshTransformSpecial*)pRVar5, (byte*)pMaterial);
								}
								else {
									bVar3 = true;
									ed3DFlushStrip((MeshTransformSpecial*)pRVar5);
								}
							goto LAB_002a7d54;)
						}
						pRVar5 = (edLIST*)pRVar5->pPrev;
					}
				}
				if (bVar3) {
					g_VifRefPktCur->cmdA = ED_VIF1_SET_TAG_REF(0, 0);
					peVar4->cmdB = 0x1100000011000000;
					g_VifRefPktCur = g_VifRefPktCur + 1;
				}
			}
		}
		else {
			while (pRVar5 != pList) {
				sVar1 = pRVar5->header.typeField.type;
				if ((sVar1 == 5) || (sVar1 == 6)) {
					IMPLEMENTATION_GUARD();
					puVar2 = (undefined4*)pRVar5->pData;
					*(undefined4*)&g_VifRefPktCur->cmdA = *puVar2;
					*(undefined4*)((int)&g_VifRefPktCur->cmdA + 4) = puVar2[1];
					*(undefined4*)&g_VifRefPktCur->cmdB = puVar2[2];
					*(undefined4*)((int)&g_VifRefPktCur->cmdB + 4) = puVar2[3];
					g_VifRefPktCur = g_VifRefPktCur + 1;
				LAB_002a7c6c:
					pRVar5 = (edLIST*)pRVar5->pPrev;
				}
				else {
					if (sVar1 == 4) goto LAB_002a7c6c;
					if (sVar1 == 3) {
						IMPLEMENTATION_GUARD(
							ed3DFlushStripForZbufferOnly((int)pRVar5, (byte*)pMaterial);)
							goto LAB_002a7c6c;
					}
					pRVar5 = (edLIST*)pRVar5->pPrev;
				}
			}
		}
	}
	else {
		gStartPKT_SPR = SCRATCHPAD_ADDRESS(0x70001000);
		gBackupPKT = g_VifRefPktCur;
		g_VifRefPktCur = SCRATCHPAD_ADDRESS(0x70001000);
		if (pMaterial->count_0x0 < 2) {
			for (; pRVar5 != pList; pRVar5 = (edLIST*)pRVar5->pPrev) {
				sVar1 = pRVar5->header.typeField.type;
				if ((sVar1 == 5) || (sVar1 == 6)) {
					IMPLEMENTATION_GUARD();
					puVar2 = (undefined4*)pRVar5->pData;
					*(undefined4*)&g_VifRefPktCur->cmdA = *puVar2;
					*(undefined4*)((int)&g_VifRefPktCur->cmdA + 4) = puVar2[1];
					*(undefined4*)&g_VifRefPktCur->cmdB = puVar2[2];
					*(undefined4*)((int)&g_VifRefPktCur->cmdB + 4) = puVar2[3];
					g_VifRefPktCur = g_VifRefPktCur + 1;
				}
				else {
					if (sVar1 == 4) {
						IMPLEMENTATION_GUARD(
							ed3DFlushSprite((MeshTransformSpecial*)pRVar5, pMaterial);)
					}
					else {
						if (sVar1 == 3) {
							ed3DFlushStrip(pRVar5);
						}
					}
				}
				if (SCRATCHPAD_ADDRESS(0x70003000) < g_VifRefPktCur) {
					uint pktLen = PKT_SIZE(g_VifRefPktCur, gStartPKT_SPR);
					edDmaLoadFromFastRam(gStartPKT_SPR, pktLen, PKT_TO_SCRATCHPAD(gBackupPKT));
					g_VifRefPktCur = SCRATCHPAD_ADDRESS(0x70001000);
					gStartPKT_SPR = SCRATCHPAD_ADDRESS(0x70001000);
					gBackupPKT = (edpkt_data*)((ulong)gBackupPKT + (pktLen & 0xfffffff0));
				}
			}
		}
		else {
			for (; pRVar5 != pList; pRVar5 = (edLIST*)pRVar5->pPrev) {
				sVar1 = pRVar5->header.typeField.type;
				if (sVar1 == 4) {
					IMPLEMENTATION_GUARD(ed3DFlushSprite((MeshTransformSpecial*)pRVar5, pMaterial));
				}
				else {
					if (sVar1 == 3) {
						IMPLEMENTATION_GUARD(ed3DFlushStripMultiTexture((int)pRVar5, (byte*)pMaterial));
					}
				}
				if (SCRATCHPAD_ADDRESS(0x70003000) < g_VifRefPktCur) {
					uint pktLen = PKT_SIZE(g_VifRefPktCur, gStartPKT_SPR);
					edDmaLoadFromFastRam(gStartPKT_SPR, pktLen, PKT_TO_SCRATCHPAD(gBackupPKT));
					g_VifRefPktCur = SCRATCHPAD_ADDRESS(0x70001000);
					gStartPKT_SPR = SCRATCHPAD_ADDRESS(0x70001000);
					gBackupPKT = (edpkt_data*)((ulong)gBackupPKT + (pktLen & 0xfffffff0));
				}
			}
		}
		uint pktLen = PKT_SIZE(g_VifRefPktCur, gStartPKT_SPR);
		if (pktLen == 0) {
			g_VifRefPktCur = gBackupPKT;
		}
		else {
			edDmaSync(SHELLDMA_CHANNEL_VIF0);
			edDmaLoadFromFastRam_nowait(gStartPKT_SPR, pktLen, PKT_TO_SCRATCHPAD(gBackupPKT));
			g_VifRefPktCur = (edpkt_data*)((ulong)gBackupPKT + (pktLen & 0xfffffff0));
		}
	}
	return;
}

ed_g2d_layer* ed3DGetG2DLayer(ed_g2d_material* pMAT, int index)
{
	ed_g2d_layer* pTVar1;

	pTVar1 = (ed_g2d_layer*)0x0;
	if (pMAT != (ed_g2d_material*)0x0) {
		if (pMAT->count_0x0 == 0) {
			pTVar1 = (ed_g2d_layer*)0x0;
		}
		else {
			int* pHash = (int*)(pMAT + 1);
			pTVar1 = (ed_g2d_layer*)((char*)LOAD_SECTION(pHash[index]) + 0x10);
		}
	}
	return pTVar1;
}

bool ed3DFXIsTrue(ed_g2d_layer* pLAY)
{
	return (pLAY->flags_0x0 & 0x7fff8000) != 0;
}

edF32VECTOR4 Vector_004489bc = { 1.0f, 1.0f, 1.0f, 1.0f };
edF32VECTOR4 Vector_004489cc = { 1.0f, 1.0f, 1.0f, 1.0f };

void ed3DFlushMatrix(ed_dma_matrix* pRenderData, ed_g2d_material* pMaterial)
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

		g_VifRefPktCur->cmdA = ED_VIF1_SET_TAG_REF(1, &g_stMatrixHeader);
		g_VifRefPktCur->asU32[2] = SCE_VIF1_SET_FLUSH(0);
		g_VifRefPktCur->asU32[3] = SCE_VIF1_SET_FLUSH(0);

		pRVar5 = ed3DPKTAddMatrixPacket(puVar14 + 1, pRenderData);
		pRVar5->cmdA = ED_VIF1_SET_TAG_REF(0, 0);
		puVar14 = pRVar5 + 1;
		pRVar5->asU32[2] = SCE_VIF1_SET_FLUSH(0);
		pRVar5->asU32[3] = SCE_VIF1_SET_FLUSH(0);
	}
	pMVar12 = pRenderData->pObjToWorld;
	if (((1 < pMaterial->count_0x0) || (pMVar12 == &gF32Matrix4Unit)) && (uVar11 = 0, gFushListCounter != 0xe))
	{
		bVar1 = pMaterial->count_0x0;
		for (; uVar11 < bVar1; uVar11 = uVar11 + 1) {
			ed_g2d_layer* pTVar6 = ed3DGetG2DLayer(pMaterial, uVar11);
			if (((pTVar6 != (ed_g2d_layer*)0x0) && (bVar4 = ed3DFXIsTrue(pTVar6), bVar4 != false)) ||
				(pMVar10 == &gF32Matrix4Unit)) {
				IMPLEMENTATION_GUARD(
					m0 = (edF32MATRIX4*)(puVar14 + 3);
				puVar14->cmdA = ED_VIF1_SET_TAG_CNT(6);
				puVar14->cmdB = 0;
				iVar8 = 8;
				*(undefined4*)&puVar14->cmdB = 0;
				*(undefined4*)((ulong)&puVar14->cmdB + 4) = 0x6c0603fa;
				pMVar9 = &local_40;
				pMVar10 = WorldToCamera_Matrix;
				do {
					iVar8 = iVar8 + -1;
					fVar3 = pMVar10->ab;
					pMVar9->aa = pMVar10->aa;
					pMVar10 = (edF32MATRIX4*)&pMVar10->ac;
					pMVar9->ab = fVar3;
					pMVar9 = (edF32MATRIX4*)&pMVar9->ac;
				} while (0 < iVar8);
				if (local_40.ac + local_40.aa + local_40.ab != 0.0) {
					edF32Vector4NormalizeHard((float*)&local_40, (float*)&local_40);
				}
				if (local_40.bc + local_40.ba + local_40.bb != 0.0) {
					edF32Vector4NormalizeHard(&local_40.ba, &local_40.ba);
				}
				if (local_40.cc + local_40.ca + local_40.cb != 0.0) {
					edF32Vector4NormalizeHard(&local_40.ca, &local_40.ca);
				}
				edF32Matrix4GetTransposeHard(&local_40, &local_40);
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
				edF32Matrix4GetTransposeHard(m0, m0);
				puVar14 = puVar14 + 7;)
					break;
			}
			bVar1 = pMaterial->count_0x0;
		}
	}
	g_VifRefPktCur = puVar14;
	puVar14 = ed3DHierachyCheckForGlobalAlpha(puVar14, gCurLayer);
	g_VifRefPktCur = puVar14;
	puVar14->cmdA = ED_VIF1_SET_TAG_CNT(1);
	puVar14->cmdB = 0;
	puVar14->asU32[2] = SCE_VIF1_SET_NOP(0);
	puVar14->asU32[3] = SCE_VIF1_SET_UNPACK(0x0021, 0x01, UNPACK_V4_32, 0); //  0x6c010021;
	g_VifRefPktCur = g_VifRefPktCur + 1;

	g_VifRefPktCur->asVector = *gVU1_AnimST_NormalExtruder_Scratch;
	g_VifRefPktCur = g_VifRefPktCur + 1;

	ed3DFlushStripList(pRenderData, pMaterial);
	pMVar4 = (edNODE_MANAGER*)pRenderData->pData;
	pRenderData->pPrev = pMVar4->pNodeHead + pMVar4->linkCount;
	pMVar4->linkCount = pMVar4->linkCount - pRenderData->nodeCount;
	pRenderData->nodeCount = 0;
	pRenderData->pPrev = (edNODE*)pRenderData;
	pRenderData->pNext = (edNODE*)pRenderData;
	return;
}

void ed3DFlushMaterialNode(ed_dma_material* pMaterial)
{
	edNODE_MANAGER* pvVar2;
	edNODE* pRVar3;
	edNODE_MANAGER* pvVar1;

	ED3D_LOG(LogLevel::Verbose, "ed3DFlushMaterialNode");

	if ((pMaterial->flags & 1) == 0) {
		pRVar3 = pMaterial->list.pPrev;
		if ((edLIST*)pRVar3 != (edLIST*)0x0) {
			if (gShadowFlushMode == 0) {
				for (; (edLIST*)pRVar3 != &pMaterial->list; pRVar3 = pRVar3->pPrev) {
					ed3DFlushMatrix((ed_dma_matrix*)pRVar3->pData, pMaterial->pMaterial);
				}
				pvVar2 = (edNODE_MANAGER*)pMaterial->list.pData;
			}
			else {
				for (; (edLIST*)pRVar3 != &pMaterial->list; pRVar3 = pRVar3->pPrev) {
					IMPLEMENTATION_GUARD(
						ed3DFlushMatrixForShadow((ed_dma_matrix*)pRVar3->pData);
					ed3DFlushMatrix((ed_dma_matrix*)pRVar3->pData, pMaterial->pMaterial);)
				}
				pvVar2 = (edNODE_MANAGER*)pMaterial->list.pData;
			}
			pMaterial->list.pPrev = pvVar2->pNodeHead + pvVar2->linkCount;
			pvVar2->linkCount = pvVar2->linkCount - (pMaterial->list).nodeCount;
			(pMaterial->list).nodeCount = 0;
			pMaterial->list.pPrev = (edNODE*)&pMaterial->list;
			pMaterial->list.pNext = (edNODE*)&pMaterial->list;
		}
		gFlushListFlusaON = 0;
	}
	else {
		if (gFlushListFlusaON == 0) {
			ed3DFlushMatrix(DmaMatrixDefault, pMaterial->pMaterial);
			gFlushListFlusaON = 1;
		}
		ed3DFlushStripList(&pMaterial->list, pMaterial->pMaterial);
		pvVar1 = (edNODE_MANAGER*)pMaterial->list.pData;
		pMaterial->list.pPrev = pvVar1->pNodeHead + pvVar1->linkCount;
		pvVar1->linkCount = pvVar1->linkCount - (pMaterial->list).nodeCount;
		(pMaterial->list).nodeCount = 0;
		pMaterial->list.pPrev = (edNODE*)&pMaterial->list;
		pMaterial->list.pNext = (edNODE*)&pMaterial->list;
	}
	return;
}



edpkt_data* ed3DFlushMaterialAnimST(edpkt_data* pPkt)
{
	uint uVar1;
	uint uVar2;
	ushort uVar3;
	TextureData_TEX* pTVar4;
	edF32VECTOR4* peVar5;
	int iVar6;
	uint uVar7;
	int iVar8;
	uint uVar9;
	float fVar10;
	float fVar11;
	float fVar12;

	uVar9 = gCurLayer->flags;
	if ((uVar9 & 0x10) == 0) {
		if ((uVar9 & 2) != 0) {
			IMPLEMENTATION_GUARD(
				pTVar4 = gCurLayer->pTex;
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
				((pTVar4->body).field_0x14)->w = 0.0;
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
				((pTVar4->body).field_0x14)->w = fVar12 * (fVar10 / fVar11);
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
			((pTVar4->body).field_0x14)->z = fVar12 * (fVar10 / fVar11);
			pPkt->cmdA = ED_VIF1_SET_TAG_CNT(1);
			pPkt->cmdB = 0;
			*(undefined4*)&pPkt->cmdB = 0;
			*(undefined4*)((int)&pPkt->cmdB + 4) = 0x6c010021;
			gVU1_AnimST_NormalExtruder_Scratch->x = ((pTVar4->body).field_0x14)->z;
			gVU1_AnimST_NormalExtruder_Scratch->y = ((pTVar4->body).field_0x14)->w;
			*(float*)&pPkt[1].cmdA = gVU1_AnimST_NormalExtruder_Scratch->x;
			*(float*)((int)&pPkt[1].cmdA + 4) = gVU1_AnimST_NormalExtruder_Scratch->y;
			*(float*)&pPkt[1].cmdB = gVU1_AnimST_NormalExtruder_Scratch->z;
			*(float*)((int)&pPkt[1].cmdB + 4) = gVU1_AnimST_NormalExtruder_Scratch->w;
			pPkt = pPkt + 2;
			gCurLayer->flags = gCurLayer->flags | 0x400;)
		}
	}
	else {
		pTVar4 = (TextureData_TEX*)LOAD_SECTION(gCurLayer->pTex);
		if (pTVar4 != (TextureData_TEX*)0xfffffff0) {
			peVar5 = (edF32VECTOR4*)LOAD_SECTION((pTVar4->body).field_0x14);
			if (((uVar9 & 0x400) == 0) && (gStepTime != 0)) {
				peVar5->z = peVar5->z + peVar5->x;
				peVar5->w = peVar5->w + peVar5->y;
				fVar10 = peVar5->z;
				if (1.0 <= fVar10) {
					peVar5->z = fVar10 - 1.0;
				}
				fVar10 = peVar5->w;
				if (1.0 <= fVar10) {
					peVar5->w = fVar10 - 1.0;
				}
				fVar10 = peVar5->z;
				if (fVar10 <= -1.0) {
					peVar5->z = fVar10 + 1.0;
				}
				fVar10 = peVar5->w;
				if (fVar10 <= -1.0) {
					peVar5->w = fVar10 + 1.0;
				}
			}
			pPkt->cmdA = ED_VIF1_SET_TAG_CNT(1);
			pPkt->cmdB = 0;
			pPkt->asU32[2] = SCE_VIF1_SET_NOP(0);
			pPkt->asU32[3] = SCE_VIF1_SET_UNPACK(0x021, 0x1, UNPACK_V4_32, 0);

			gVU1_AnimST_NormalExtruder_Scratch->x = peVar5->z;
			gVU1_AnimST_NormalExtruder_Scratch->y = peVar5->w;

			pPkt[1].asVector = *gVU1_AnimST_NormalExtruder_Scratch;
			pPkt = pPkt + 2;
			gCurLayer->flags = gCurLayer->flags | 0x400;
		}
	}
	return pPkt;
}

void ed3DFlushMaterialManageGIFPacket(ed_dma_material* pMaterial)
{
	bool bVar1;
	edpkt_data* peVar2;

	ED3D_LOG(LogLevel::Verbose, "ed3DFlushMaterialManageGIFPacket gFushListCounter: 0x%x gbFirstTex: %d", gFushListCounter, gbFirstTex);

	if ((pMaterial->flags & 2) != 0) {
		ed3DFXClearALLFXSendedTOVRAM();
		pMaterial->flags = pMaterial->flags & 0xfffffffd;
	}
	peVar2 = g_GifRefPktCur;
	if (gFushListCounter != 0xe) {
		if ((gbFirstTex == 0) && (pMaterial->pBitmap != (ed_g2d_bitmap*)0x0)) {
			if (pMaterial->pMaterial->count_0x0 < 2) {
				ED3D_LOG(LogLevel::Verbose, "ed3DFlushMaterialManageGIFPacket Flushing new material gVRAMBufferFlush %d", gVRAMBufferFlush);

				edPSX2Header* pHeader = (edPSX2Header*)LOAD_SECTION(pMaterial->pBitmap->pPSX2);
				edpkt_data* pPkt = (edpkt_data*)(LOAD_SECTION((pHeader + gVRAMBufferFlush)->pPkt));

				g_GifRefPktCur->asU32[0] = SCE_VIF1_SET_DIRECT(0, 0);
				g_GifRefPktCur->asU32[1] = ((ulong)pPkt) + 0x20;
				peVar2->cmdB = 0;
				g_GifRefPktCur = g_GifRefPktCur + 1;
			}
			else {
				IMPLEMENTATION_GUARD(
					g_GifRefPktCur = ed3DFlushMultiTexture(pMaterial, g_GifRefPktCur, 0, gVRAMBufferFlush);)
			}
		}
		else {
			gbFirstTex = 0;
		}

		// Send even if gbFirstTex is true.
#ifdef PLATFORM_WIN
		Renderer::SetImagePointer(MakeTextureDataFromPacket(gCurBitmap, pMaterial->pBitmap, gVRAMBufferFlush));
#endif

		bVar1 = gVRAMBufferFlush == 1;
		gVRAMBufferFlush = gVRAMBufferFlush + 1;
		if (bVar1) {
			gVRAMBufferFlush = 0;
		}
	}
	return;
}

edpkt_data* ed3DFlushMaterialMipMapOff(edpkt_data* pPkt)
{
	pPkt->cmdA = ED_VIF1_SET_TAG_CNT(3);
	pPkt->cmdB = 0;
	pPkt->asU32[2] = SCE_VIF1_SET_ITOP(0x03dc, 0);
	pPkt->asU32[3] = SCE_VIF1_SET_UNPACK(0x03dc, 0x2, UNPACK_V4_32, 0);

	pPkt[1].cmdA = SCE_GIF_SET_TAG(
		1,				// NLOOP
		SCE_GS_TRUE,	// EOP
		SCE_GS_FALSE,	// PRE
		0,				// PRIM
		SCE_GIF_PACKED,	// FLG
		1				// NREG
	);
	pPkt[1].cmdB = SCE_GIF_PACKED_AD;

	pPkt[2].cmdA = SCE_GS_SET_TEX1(
		0x60,		// LCM
		0,			// MXL
		0,			// MMAG
		0,			// MMIN
		0,			// MTBA
		gMipmapL,	// L
		gMipmapK	// K
	);
	pPkt[2].cmdB = SCE_GS_TEX1_1;

	pPkt[3].asU32[0] = 0;
	pPkt[3].asU32[1] = SCE_VIF1_SET_MSCAL(0, 0);
	pPkt[3].asU32[2] = 0;
	pPkt[3].asU32[3] = 0;

	return pPkt + 4;
}

#ifdef PLATFORM_WIN
void ProcessTextureCommands(edpkt_data* aPkt, int size)
{
	bool bBeganGIFTag = false;

	for (int i = 0; i < size; i++) {
		edpkt_data& pkt = aPkt[i];
		bBeganGIFTag |= pkt.asU32[2] == 0xe;

		if (bBeganGIFTag) {
			switch (pkt.asU32[2]) {
			case 0x6:
			{
				ED3D_LOG(LogLevel::Verbose, "ed3DFlushMaterial - ProcessTextureCommands TEX0: %llx (%llx)", pkt.cmdA, pkt.cmdB);
				SendTextureCommandsFromPacked(pkt.cmdA);
			}
			break;
			}
		}
	}

	assert(bBeganGIFTag);
}
#endif

void ed3DFlushMaterial(ed_dma_material* pRenderMeshData)
{
	ed_g2d_material* peVar1;
	ulong* puVar2;
	edpkt_data* peVar3;
	edpkt_data* peVar4;
	edpkt_data* peVar5;
	edpkt_data* peVar6;
	ed_g2d_material* peVar7;
	undefined4 in_zero_lo;
	undefined4 in_zero_hi;
	undefined4 in_zero_udw;
	undefined4 in_register_0000000c;
	edpkt_data* peVar8;
	int iVar9;
	ed_g2d_material* pTVar1;

	ED3D_LOG(LogLevel::Verbose, "ed3DFlushMaterial pMaterial: %p | pBitmap: %p", pRenderMeshData->pMaterial, pRenderMeshData->pBitmap);

	peVar6 = gpPKTDataRefMasPath3;
	peVar5 = g_GifRefPktCur;
	peVar3 = g_VifRefPktCur;
	*gMaterialNbPrimSend = 0;
	peVar4 = g_VifRefPktCur;
	gGlobalAlhaON = 0x80;
	peVar1 = pRenderMeshData->pMaterial;
	peVar1->pDMA_Material = 0x0;
	peVar8 = gpPKTDataRefMasPath3;
	iVar9 = gFushListCounter;
	peVar7 = peVar1;
	if (peVar1 == previous_flush3DMat) {
		peVar7 = previous_flush3DMat;
		if (gVRAMBufferFlush == 0) {
			gVRAMBufferFlush = 1;
		}
		else {
			gVRAMBufferFlush = gVRAMBufferFlush + -1;
		}
	}
	previous_flush3DMat = peVar7;
	if (((gFushListCounter != 0xe) && (pRenderMeshData->pBitmap == (ed_g2d_bitmap*)0x0)) &&
		(gpPKTDataRefMasPath3 != (edpkt_data*)0x0)) {
		gpPKTDataRefMasPath3->asU32[0] = SCE_VIF1_SET_STMASK(0);
		gpPKTDataRefMasPath3->asU32[1] = (int)(gpPKTDataRefMasPath3 + 4);
		peVar8->cmdB = 0;
	}
	peVar8 = peVar4;
	if (gpLastMaterial != pRenderMeshData) {
		gpLastMaterial = pRenderMeshData;
		if (iVar9 == 0xe) {
			IMPLEMENTATION_GUARD(
				peVar8 = FUN_002a97a0(peVar4));
		}
		else {
			if (peVar1->commandBufferTextureSize == 2) {
				peVar4->cmdA = ED_VIF1_SET_TAG_REF(0, 0);
				peVar4->asU32[2] = 0;
				peVar4->asU32[3] = SCE_VIF1_SET_FLUSHA(0);
		
				peVar4[1].cmdA = ED_VIF1_SET_TAG_REF(0, 0);
				peVar4[1].asU32[2] = SCE_VIF1_SET_FLUSH(0);
				peVar4[1].asU32[3] = SCE_VIF1_SET_FLUSH(0);

				peVar4[2].cmdA = ED_VIF1_SET_TAG_REF(0, 0);
				peVar4[2].asU32[2] = 0;
				peVar4[2].asU32[3] = SCE_VIF1_SET_MSKPATH3(0, 0);

				peVar4[3].cmdA = ED_VIF1_SET_TAG_CNT(1);
				peVar4[3].cmdB = 0;

				peVar4[4].asU128 = 0;

				peVar4[5].cmdA = ED_VIF1_SET_TAG_REF(0, 0);
				peVar4[5].asU32[2] = 0;
				peVar4[5].asU32[3] = SCE_VIF1_SET_MSKPATH3(0x8000, 0);
				peVar8 = peVar4 + 6;

				gpPKTDataRefMasPath3 = peVar4 + 2;
			}
			else {
				peVar4->cmdA = ED_VIF1_SET_TAG_REF(0, 0);
				peVar4->asU32[2] = SCE_VIF1_SET_NOP(0);
				peVar4->asU32[3] = SCE_VIF1_SET_FLUSHA(0);

				edpkt_data* pFlippedPkt = (edpkt_data*)LOAD_SECTION(peVar1->pCommandBufferTexture) +
					(gVRAMBufferFlush * peVar1->commandBufferTextureSize * (uint)peVar1->count_0x0);
#ifdef PLATFORM_WIN
				ProcessTextureCommands(pFlippedPkt, peVar1->commandBufferTextureSize);
#endif
				peVar4[1].cmdA = ED_VIF1_SET_TAG_REF(peVar1->commandBufferTextureSize, pFlippedPkt);
				peVar4[1].cmdB = SCE_VIF1_SET_NOP(0);

				peVar4[2].cmdA = ED_VIF1_SET_TAG_REF(0, 0);
				peVar4[2].asU32[2] = SCE_VIF1_SET_FLUSH(0);
				peVar4[2].asU32[3] = SCE_VIF1_SET_FLUSH(0);

				peVar4[3].cmdA = ED_VIF1_SET_TAG_REF(0, 0);
				peVar4[3].asU32[2] = SCE_VIF1_SET_NOP(0);
				peVar4[3].asU32[3] = SCE_VIF1_SET_MSKPATH3(0, 0);

				peVar4[4].cmdA = ED_VIF1_SET_TAG_CNT(1);
				peVar4[4].cmdB = SCE_VIF1_SET_NOP(0);

				peVar4[5].asU128 = 0;

				peVar4[6].cmdA = ED_VIF1_SET_TAG_REF(0, 0);
				peVar4[6].asU32[2] = SCE_VIF1_SET_NOP(0);
				peVar4[6].asU32[3] = SCE_VIF1_SET_MSKPATH3(0x8000, 0);

				peVar8 = peVar4 + 7;
				gpPKTDataRefMasPath3 = peVar4 + 3;
			}
		}
	}
	g_VifRefPktCur = peVar8;
	if (pRenderMeshData->pBitmap == (ed_g2d_bitmap*)0x0) {
		gCurLayer = (ed_g2d_layer*)0x0;
		g_VifRefPktCur = ed3DFlushMaterialMipMapOff(peVar8);
	}
	else {
		ed3DG2DMaterialGetLayerBitmap(pRenderMeshData, &gCurBitmap, &gCurLayer, 0);
		ED3D_LOG(LogLevel::Verbose, "ed3DFlushMaterial gCurBitmap: %p | gCurLayer: %p", gCurBitmap, gCurLayer);
		*g_pCurFlareMaterial = pRenderMeshData->pMaterial;
		peVar4 = g_VifRefPktCur;

		g_VifRefPktCur[2].cmdA = SCE_GS_SET_TEX1(
			0x20, // LCM
			(ulong)(int)((gCurBitmap->maxMipLevel - 1) * *gbDoMipmap_SPR) & 0xffffffffU, // MXL
			0, // MMAG
			*(int*)gbTrilinear_SPR + 4, //MMIN
			0, // MTBA
			*(uint*)gMipmapL_SPR, // L
			*(int*)gMipmapK_SPR // K
		);

		g_VifRefPktCur[2].cmdB = SCE_GS_TEX1_1;
		peVar8 = g_VifRefPktCur + 3;

		iVar9 = 1;
		if ((gCurLayer->flags & 0x80) != 0) {
			iVar9 = 3;

			// Fill out packets.
			IMPLEMENTATION_GUARD();
			g_VifRefPktCur[3].cmdA = 0x50000;
			g_VifRefPktCur[3].cmdB = SCE_GS_TEST_1;
			g_VifRefPktCur[4].cmdA = 0x800000002a;
			g_VifRefPktCur[4].cmdB = SCE_GS_ALPHA_1;

			peVar8 = g_VifRefPktCur + 5;
		}

		if ((long)iVar9 != 0) {
			puVar2 = &g_VifRefPktCur->cmdA;
			g_VifRefPktCur = peVar8;

			*puVar2 = ED_VIF1_SET_TAG_CNT(iVar9 + 2);
			peVar4->cmdB = 0;
			peVar4->asU32[2] = SCE_VIF1_SET_ITOP(0x03dc, 0);
			peVar4->asU32[3] = SCE_VIF1_SET_UNPACK(0x03dc, iVar9 + 1, UNPACK_V4_32, 0);

			peVar4[1].cmdA = SCE_GIF_SET_TAG(
				iVar9,			// NLOOP
				SCE_GS_TRUE,	// EOP
				SCE_GS_FALSE,	// PRE
				0,				// PRIM
				SCE_GIF_PACKED, // FLG
				1				// NREG
			);
			peVar4[1].cmdB = SCE_GIF_PACKED_AD;

			g_VifRefPktCur->asU32[0] = SCE_VIF1_SET_NOP(0);
			g_VifRefPktCur->asU32[1] = SCE_VIF1_SET_MSCAL(0, 0);
			g_VifRefPktCur->asU32[2] = SCE_VIF1_SET_NOP(0);
			g_VifRefPktCur->asU32[3] = SCE_VIF1_SET_NOP(0);

			g_VifRefPktCur = g_VifRefPktCur + 1;
		}
		g_VifRefPktCur = ed3DFlushMaterialAnimST(g_VifRefPktCur);
	}

	ed3DFlushMaterialNode(pRenderMeshData);
	peVar4 = g_VifRefPktCur;
	if (*gMaterialNbPrimSend == 0) {
		g_VifRefPktCur = peVar3;
		g_GifRefPktCur = peVar5;
		gpPKTDataRefMasPath3 = peVar6;
		pRenderMeshData->pBitmap = (ed_g2d_bitmap*)0xdfdfdfdf;
	}
	else {
		g_VifRefPktCur->cmdA = ED_VIF1_SET_TAG_CNT(1);
		peVar4->cmdB = 0;
		peVar3 = g_VifRefPktCur;
		g_VifRefPktCur = g_VifRefPktCur + 1;
		g_VifRefPktCur->asU32[0] = SCE_VIF1_SET_STCYCL(1, 1, 0);
		g_VifRefPktCur->asU32[1] = SCE_VIF1_SET_STMASK(0);
		g_VifRefPktCur->cmdB = 0xffffff00;
		g_VifRefPktCur = g_VifRefPktCur + 1;
		ed3DFlushMaterialManageGIFPacket(pRenderMeshData);
	}
#ifdef PLATFORM_WIN
	Renderer::Draw();
#endif
	return;
}

bool gbFA2AObject = true;

void ed3DFlushList(void)
{
	float fVar1;
	int iVar2;
	edLIST* unaff_s1_lo;
	edLIST* pPrevList;
	edLIST* peVar3;
	edLIST* pCurrentList;
	ed_dma_material* pvVar2;
	ed_dma_material* pvVar1;
	ed_dma_material* pvVar4;
	ed_dma_material* pvVar3;
	ed_dma_material* pvVar6;
	ed_dma_material* pvVar5;
	ed_dma_material* pvVar7;

	ED3D_LOG(LogLevel::Verbose, "ed3DFlushList");

	gFlushListFlusaON = 0;
	gFushListCounter = 0;
	do {
		fVar1 = FLOAT_00448a04;
		if (0xe < gFushListCounter) {
			return;
		}

		ED3D_LOG(LogLevel::Verbose, "ed3DFlushList Processing: %d", gFushListCounter);

		/* Get the camera type, and then add the flip index as there is two per type. */
		pCurrentList = gPrim_List[gFushListCounter] + gCurRenderList;
		pPrevList = (edLIST*)(pCurrentList)->pPrev;
		if (pPrevList != pCurrentList) {
			switch (gFushListCounter) {
			default:
				/* 0x7 = Particle
				   0x3 = Some level mesh
				   0x2 = Character */
				pvVar4 = (ed_dma_material*)(pPrevList)->pData;
				pvVar4->flags = pvVar4->flags | 2;
				while (peVar3 = pPrevList, peVar3 != pCurrentList) {
					ed3DFlushMaterial((ed_dma_material*)(peVar3)->pData);
					unaff_s1_lo = peVar3;
					pPrevList = (edLIST*)(peVar3)->pPrev;
				}
				break;
			case 1:
			case 5:
			case 6:
			case 7:
			case 9:
				if (gbFA2AObject != false) {
					g_VifRefPktCur = ed3DFlushFullAlphaInit(g_VifRefPktCur);
				}
				pvVar3 = (ed_dma_material*)(pPrevList)->pData;
				pvVar3->flags = pvVar3->flags | 2;
				for (; pPrevList != pCurrentList; pPrevList = (edLIST*)(pPrevList)->pPrev) {
					ed3DFlushMaterial((ed_dma_material*)(pPrevList)->pData);
					unaff_s1_lo = pPrevList;
				}
				if (gbFA2AObject != false) {
					g_VifRefPktCur = ed3DFlushFullAlphaTerm(g_VifRefPktCur);
				}
				break;
			case 10:
				pvVar2 = (ed_dma_material*)(pPrevList)->pData;
				pvVar2->flags = pvVar2->flags | 2;
				for (; pPrevList != pCurrentList; pPrevList = (edLIST*)(pPrevList)->pPrev) {
					ed3DFlushMaterial((ed_dma_material*)(pPrevList)->pData);
					unaff_s1_lo = pPrevList;
				}
				g_VifRefPktCur = ed3DFlushResetOffset(g_VifRefPktCur, &gCurRectViewport);
				break;
			case 0xb:
				IMPLEMENTATION_GUARD(
					g_VifRefPktCur = (edpkt_data*)ed3DFlushBackFaceInit((ulong*)g_VifRefPktCur);
				g_VifRefPktCur = ed3DFlushResetOffset(g_VifRefPktCur, &gCurRectViewport);
				pvVar7 = (ed_dma_material*)(pPrevList)->pData;
				pvVar7->flags = pvVar7->flags & 0xfffffffd;
				for (; pPrevList != pCurrentList; pPrevList = (edLIST*)(pPrevList)->pPrev) {
					ed3DFlushMaterial((ed_dma_material*)(pPrevList)->pData);
					unaff_s1_lo = pPrevList;
				}
				g_VifRefPktCur = (edpkt_data*)ed3DFlushBackFaceTerm(g_VifRefPktCur);
				g_VifRefPktCur = ed3DFlushResetOffset(g_VifRefPktCur, &gCurRectViewport);)
			case 8:
				if (gbFA2AObject != false) {
					g_VifRefPktCur = ed3DFlushFullAlphaInit(g_VifRefPktCur);
				}
				pvVar1 = (ed_dma_material*)(pPrevList)->pData;
				pvVar1->flags = pvVar1->flags | 2;
				for (; pPrevList != pCurrentList; pPrevList = (edLIST*)(pPrevList)->pPrev) {
					ed3DFlushMaterial((ed_dma_material*)(pPrevList)->pData);
					unaff_s1_lo = pPrevList;
				}
				if (gbFA2AObject != false) {
					g_VifRefPktCur = ed3DFlushFullAlphaTerm(g_VifRefPktCur);
				}
				break;
			case 0xd:
				IMPLEMENTATION_GUARD(
					FLOAT_00448a04 = 0.0;
				g_VifRefPktCur = (edpkt_data*)FUN_002b4e60((ulong*)g_VifRefPktCur);
				BYTE_004489e4 = 0;
				gFushListCounter = 0xe;
				pvVar5 = (ed_dma_material*)(pPrevList)->pData;
				pvVar5->flags = pvVar5->flags | 2;
				for (; pPrevList != pCurrentList; pPrevList = (edLIST*)(pPrevList)->pPrev) {
					ed3DFlushMaterial((ed_dma_material*)(pPrevList)->pData);
					unaff_s1_lo = pPrevList;
				}
				gFushListCounter = 0xd;
				FLOAT_00448a04 = fVar1;)
					break;
			case 0xe:
				IMPLEMENTATION_GUARD(
					g_VifRefPktCur = (edpkt_data*)FUN_002b4e60((ulong*)g_VifRefPktCur);
				BYTE_004489e4 = 1;
				pvVar6 = (ed_dma_material*)(pPrevList)->pData;
				pvVar6->flags = pvVar6->flags | 2;
				for (; pPrevList != pCurrentList; pPrevList = (edLIST*)(pPrevList)->pPrev) {
					ed3DFlushMaterial((ed_dma_material*)(pPrevList)->pData);
					unaff_s1_lo = pPrevList;
				}
				g_VifRefPktCur = (edpkt_data*)FUN_002b4d50();)
			}
			iVar2 = gCurRenderList;
			if ((edLIST*)gPrim_List_FlushTex[gCurRenderList].pPrev == gPrim_List_FlushTex + gCurRenderList) {
				pCurrentList = gPrim_List[gFushListCounter] + gCurRenderList;
				gPrim_List_FlushTex[gCurRenderList].pPrev = (pCurrentList)->pPrev;
				gPrim_List_FlushTex[iVar2].nodeCount = pCurrentList->nodeCount;
			}
			else {
				pCurrentList = gPrim_List[gFushListCounter] + gCurRenderList;
				gPrim_List_FlushTex[gCurRenderList].nodeCount =
					gPrim_List_FlushTex[gCurRenderList].nodeCount + pCurrentList->nodeCount;
				gPrim_List_FlushTex_Last->pPrev = pCurrentList->pPrev;
			}
			pCurrentList = gPrim_List[gFushListCounter] + gCurRenderList;
			gPrim_List_FlushTex_Last = unaff_s1_lo;
			pCurrentList->nodeCount = 0;
			(pCurrentList)->pPrev = (edNODE*)pCurrentList;
		}
		gFushListCounter = gFushListCounter + 1;
	} while (true);
}

int gDRAWBUF_BASE = 0;
int gOFFX = 0;
int gDRAWBUF_BASE_BIS = 0;
int gOFFY = 0;
int gDRAWBUF_BASE_BIS_TEX = 0;
int gDRAWBUF_TXBASE = 0;
int gSCRN_W = 0;
int gSCRN_H = 0;
int gZBUF_BASE_BIS = 0;
int gZBUF_BASE_BIS_TEX = 0;
int gZBUF_TEXBASE = 0;

edpkt_data* ed3DFlushFullAlphaInit(edpkt_data* pRenderCommand)
{
	edSurface* pFVar1;

	pFVar1 = edVideoGetDrawSurface();
	if (pFVar1 != (edSurface*)0x0) {
		gDRAWBUF_BASE = 0x40;
		gOFFX = 0x7000;
		gDRAWBUF_BASE_BIS = 0xb0;
		gOFFY = 0x7000;
		gDRAWBUF_BASE_BIS_TEX = 0x1600;
		gDRAWBUF_TXBASE = 0x800;
		gZBUF_BASE = gCurViewportUsed->pZBuffer->frameBasePtr;
		gSCRN_W = 0x200;
		gSCRN_H = 0x200;
		gZBUF_BASE_BIS = 0xc0;
		gZBUF_BASE_BIS_TEX = 0x1800;
		gZBUF_TEXBASE = gZBUF_BASE << 5;
		pRenderCommand->cmdA = ED_VIF1_SET_TAG_REF(0, 0);
		pRenderCommand->cmdB = 0x1100000011000000;
		pRenderCommand[1].cmdA = ED_VIF1_SET_TAG_CNT(2);
		pRenderCommand[1].cmdB = 0;
		pRenderCommand[1].asU32[2] = 0;
		pRenderCommand[1].asU32[3] = 0x50000002;
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
		gOFFX = 0x7000;
		gOFFY = 0x7000;
		gDRAWBUF_BASE = pFVar1->frameBasePtr;
		gDRAWBUF_BASE_BIS_TEX = 0x800;
		gDRAWBUF_BASE_BIS = 0x40;
		gDRAWBUF_TXBASE = gDRAWBUF_BASE << 5;
		gZBUF_BASE = gCurViewportUsed->pZBuffer->frameBasePtr;
		gSCRN_W = 0x200;
		gSCRN_H = 0x200;
		gZBUF_TEXBASE = gZBUF_BASE << 5;
		gZBUF_BASE_BIS = gZBUF_BASE;
		gZBUF_BASE_BIS_TEX = gZBUF_TEXBASE;
		pRenderCommand->cmdA = ED_VIF1_SET_TAG_REF(0, 0);
		pRenderCommand->cmdB = 0x1100000011000000;
		pRenderCommand[1].cmdA = ED_VIF1_SET_TAG_CNT(2);
		pRenderCommand[1].cmdB = 0;
		pRenderCommand[1].asU32[2] = 0;
		pRenderCommand[1].asU32[3] = 0x50000002;
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
	bool bVar1;
	ed_g2d_material** ppeVar2;
	int iVar3;
	ed_dma_material* peVar4;
	edpkt_data* peVar5;
	edLIST* peVar6;
	ed_g2d_material* peVar7;
	edNODE* peVar8;
	edNODE_MANAGER* pvVar1;
	ed_dma_material* pShadowListData;

	bVar1 = false;
	g_VifRefPktCur = ed3DFlushFullAlphaInit(g_VifRefPktCur);
	gFlushListFlusaON = 0;
	gShadowFlushMode = 1;
	pShadowListData = (ed_dma_material*)(gPrim_List_Shadow[gCurRenderList].pPrev)->pPrev->pData;
	if ((edLIST*)(pShadowListData->list).pPrev != &pShadowListData->list) {
		IMPLEMENTATION_GUARD(
			memcpy(ScratchpadAnimManager_004494ec.pScreenSpaceMatrix, &gShadow_CameraToScreen_Matrix, 0x40);
		memcpy(ScratchpadAnimManager_004494ec.pTextureSpaceMatrix, &gShadow_CameraToCulling_Matrix, 0x40);
		memcpy(ScratchpadAnimManager_004494ec.pWorldSpaceMatrix, &gShadow_CameraToClipping_Matrix, 0x40);
		memcpy(WorldToCamera_Matrix, &edF32MATRIX4_0048ccb0, 0x40);
		memcpy(&edF32VECTOR4_0048c2d0, &edF32VECTOR4_0048ccf0, 0x10);
		memcpy(&edF32VECTOR4_0048c2e0, &edF32VECTOR4_0048cd00, 0x10);
		memcpy(&edF32VECTOR4_0048c2f0, &edF32VECTOR4_0048cd10, 0x10);
		memcpy(&edF32VECTOR4_0048c300, &edF32VECTOR4_0048cd20, 0x10);
		memcpy(&edF32VECTOR4_0048c310, &edF32VECTOR4_0048cd30, 0x10);
		memcpy(&edF32VECTOR4_0048c320, &edF32VECTOR4_0048cd40, 0x10);
		peVar5 = g_VifRefPktCur;
		g_VifRefPktCur->cmdA = ED_VIF1_SET_TAG_REF(0, 0);
		peVar5->cmdB = 0x1300000000000000;
		g_VifRefPktCur = g_VifRefPktCur + 1;
		g_VifRefPktCur = (edpkt_data*)ed3DJitterShadow((undefined8*)g_VifRefPktCur);
		g_VifRefPktCur = ed3DDMAGenerateGlobalPacket(g_VifRefPktCur);
		g_VifRefPktCur = ed3DAddViewportContextPacket(g_Camera_0044956c, g_VifRefPktCur);
		peVar5 = (edpkt_data*)ed3DShadowFlushResetOffset((undefined8*)g_VifRefPktCur, (short*)&gCurRectViewport);
		g_VifRefPktCur = peVar5;
		peVar5->cmdA = ED_VIF1_SET_TAG_CNT(2);
		peVar5->cmdB = 0;
		*(undefined4*)&peVar5->cmdB = 0;
		*(undefined4*)((ulong)&g_VifRefPktCur->cmdB + 4) = 0x50000002;
		peVar5 = g_VifRefPktCur;
		g_VifRefPktCur = g_VifRefPktCur + 1;
		g_VifRefPktCur->cmdA = 0x1000800000008001;
		peVar5[1].cmdB = 0xe;
		peVar5 = g_VifRefPktCur;
		g_VifRefPktCur = g_VifRefPktCur + 1;
		g_VifRefPktCur->cmdA = (ulong)gCurScene->pViewport->pZBuffer->frameBasePtr | 0x130000000;
		peVar5[1].cmdB = 0x4e;
		g_VifRefPktCur = g_VifRefPktCur + 1;
		ed3DFlushMaterialNode(pShadowListData);
		peVar5 = g_VifRefPktCur;
		g_VifRefPktCur->cmdA = ED_VIF1_SET_TAG_REF(0, 0);
		peVar5->cmdB = 0x1300000000000000;
		iVar3 = gCurRenderList;
		g_VifRefPktCur = g_VifRefPktCur + 1;
		gPrim_List_FlushTex[gCurRenderList].nodeCount = gPrim_List_FlushTex[gCurRenderList].nodeCount + 1;
		bVar1 = true;
		if (gPrim_List_FlushTex_Last == (MeshTransformSpecial*)0x0) {
			gPrim_List_FlushTex_Last = (MeshTransformSpecial*)(gPrim_List_Shadow[iVar3].node.pNext)->pNext;
		}
		else {
			gPrim_List_FlushTex_Last->pNext_0x4 = (MeshTransformSpecial*)(gPrim_List_Shadow[iVar3].node.pNext)->pNext;
			gPrim_List_FlushTex_Last = gPrim_List_FlushTex_Last->pNext_0x4;
		}
		(gPrim_List_Shadow[gCurRenderList].node.pNext)->pNext = (edNODE*)(gPrim_List_Shadow + gCurRenderList);
		peVar4 = DmaMaterialBufferCurrent;
		peVar6 = &DmaMaterialBufferCurrent->list;
		ppeVar2 = &DmaMaterialBufferCurrent->pMaterial;
		DmaMaterialBufferCurrent = DmaMaterialBufferCurrent + 1;
		*ppeVar2 = &gMaterial_Render_Zbuffer_Only;
		peVar4->field_0x4 = 0;
		peVar4->flags = peVar4->flags & 0xfffffffe;
		(peVar4->list).node.pNext = (edNODE*)peVar6;
		(peVar4->list).node.pPrev = (edNODE*)peVar6;
		(peVar4->list).node.pData = gNodeDmaMatrix;
		(peVar4->list).field_0x10 = 0;
		(peVar4->list).nodeCount = 0;
		(peVar4->list).node.header = 0;
		peVar7 = (ed_g2d_material*)ed3DGetG2DBitmap(peVar4, 0);
		pShadowListData->field_0x2c = peVar7;
		iVar3 = gCurRenderList;
		peVar6 = gPrim_List_Shadow + gCurRenderList;
		pvVar1 = (edNODE_MANAGER*)gPrim_List_Shadow[gCurRenderList].node.pData;
		peVar8 = pvVar1->pNodeHead + pvVar1->linkCount;
		peVar8->pData = peVar4;
		peVar8->header.typeField.type = 1;
		pvVar1->linkCount = pvVar1->linkCount + 1;
		gPrim_List_Shadow[iVar3].nodeCount = gPrim_List_Shadow[iVar3].nodeCount + 1;
		peVar8->pPrev = (edNODE*)peVar6;
		(gPrim_List_Shadow[iVar3].node.pNext)->pNext = peVar8;)
	}
	gShadowFlushMode = 0;
	*gShadowRenderMask = 0;
	if (bVar1) {
		pprevious_shadow_dma_matrix = (ed_dma_matrix*)0x0;
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


ed_g2d_material* ed3DG2DGetG2DMaterialFromIndex(ed_hash_code* pMBNK, int index)
{
	int* piVar1;
	int* piVar2;
	piVar1 = (int*)LOAD_SECTION(pMBNK[index].pData);
	piVar2 = (int*)0x0;
	if ((piVar1 != (int*)0x0) && (piVar2 = piVar1 + 4, *piVar1 != 0x2e54414d)) {
		piVar2 = (int*)((char*)LOAD_SECTION(piVar1[2]) + 0x10);
	}
	return (ed_g2d_material*)piVar2;
}


EVectorMode_A ed3DTestBoundingSphere(edF32VECTOR4* pInSphere)
{
	EVectorMode_A EVar1;
	float fVar2;
	float fVar3;
	float fVar4;

	fVar2 = (gRenderSceneConfig_SPR->frustumA).field_0x50.w;
	fVar3 = gBoundSphereCenter->z;
	EVar1 = VM_1;
	if (fVar3 < fVar2) {
		fVar4 = pInSphere->w;
		if (fVar4 <= fVar2 - fVar3) {
			return VM_4;
		}
		EVar1 = VM_2;
	}
	else {
		fVar4 = pInSphere->w;
		if (fVar3 - fVar2 < fVar4) {
			EVar1 = VM_2;
		}
	}
	fVar2 = (gRenderSceneConfig_SPR->frustumA).field_0x40.w;
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
	fVar2 = gBoundSphereCenter->x;
	if (fVar2 <= 0.0f) {
		fVar2 = -(gRenderSceneConfig_SPR->frustumA).frustumMatrix.bc * fVar3 -
			(gRenderSceneConfig_SPR->frustumA).frustumMatrix.ba * fVar2;
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
		fVar2 = -(gRenderSceneConfig_SPR->frustumA).frustumMatrix.ac * fVar3 -
			(gRenderSceneConfig_SPR->frustumA).frustumMatrix.aa * fVar2;
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
	fVar2 = gBoundSphereCenter->y;
	if (fVar2 <= 0.0) {
		fVar2 = (gRenderSceneConfig_SPR->frustumA).frustumMatrix.cb * fVar2 +
			-(gRenderSceneConfig_SPR->frustumA).frustumMatrix.cc * fVar3;
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
		fVar2 = (gRenderSceneConfig_SPR->frustumA).frustumMatrix.db * fVar2 +
			-(gRenderSceneConfig_SPR->frustumA).frustumMatrix.dc * fVar3;
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

EVectorMode_A ed3DTestBoundingSphereZNearOnly(edF32VECTOR4* param_1)
{
	EVectorMode_A EVar1;
	float fVar2;
	float fVar3;

	fVar2 = (gRenderSceneConfig_SPR->frustumB).field_0x40.w;
	fVar3 = gBoundSphereCenter->z;
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


EVectorMode_A ed3DTestBoundingSphereSide(edF32VECTOR4* param_1)
{
	EVectorMode_A EVar1;
	float fVar2;
	float fVar3;
	float fVar4;

	fVar2 = gBoundSphereCenter->x;
	EVar1 = VM_1;
	if (fVar2 <= 0.0f) {
		fVar4 = -gBoundSphereCenter->z;
		fVar2 = (gRenderSceneConfig_SPR->frustumB).frustumMatrix.ba * -fVar2 +
			(gRenderSceneConfig_SPR->frustumB).frustumMatrix.bc * fVar4;
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
		fVar4 = -gBoundSphereCenter->z;
		fVar2 = (gRenderSceneConfig_SPR->frustumB).frustumMatrix.aa * -fVar2 +
			(gRenderSceneConfig_SPR->frustumB).frustumMatrix.ac * fVar4;
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
	fVar2 = gBoundSphereCenter->y;
	if (fVar2 <= 0.0f) {
		fVar2 = (gRenderSceneConfig_SPR->frustumB).frustumMatrix.cb * fVar2 +
			(gRenderSceneConfig_SPR->frustumB).frustumMatrix.cc * fVar4;
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
		fVar2 = (gRenderSceneConfig_SPR->frustumB).frustumMatrix.db * fVar2 +
			(gRenderSceneConfig_SPR->frustumB).frustumMatrix.dc * fVar4;
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

EVectorMode_A ed3DTestBoundingSpherePacketZNearOnly(ed_Bound_Sphere_packet* param_1)
{
	EVectorMode_A EVar1;
	float fVar2;
	float fVar3;

	fVar2 = (gRenderSceneConfig_SPR->frustumB).field_0x50.w;
	fVar3 = gBoundSphereCenter->z;
	EVar1 = VM_1;
	if ((fVar2 <= fVar3) || (-(fVar3 - fVar2) < (float)(uint)param_1->field_0xc)) {
		fVar2 = gBoundSphereCenter->x;
		if (fVar2 <= 0.0f) {
			fVar2 = (gRenderSceneConfig_SPR->frustumB).frustumMatrix.ba * -fVar2 +
				(gRenderSceneConfig_SPR->frustumB).frustumMatrix.bc * -fVar3;
			if ((fVar2 <= 0.0f) && ((float)(uint)param_1->field_0xc < -fVar2)) {
				return VM_4;
			}
		}
		else {
			fVar2 = (gRenderSceneConfig_SPR->frustumB).frustumMatrix.aa * -fVar2 +
				(gRenderSceneConfig_SPR->frustumB).frustumMatrix.ac * -fVar3;
			if ((fVar2 <= 0.0f) && ((float)(uint)param_1->field_0xc < -fVar2)) {
				return VM_4;
			}
		}
		fVar2 = gBoundSphereCenter->y;
		if (fVar2 <= 0.0f) {
			fVar2 = (gRenderSceneConfig_SPR->frustumB).frustumMatrix.cb * fVar2 +
				(gRenderSceneConfig_SPR->frustumB).frustumMatrix.cc * -fVar3;
			if ((fVar2 <= 0.0f) && ((float)(uint)param_1->field_0xc < -fVar2)) {
				return VM_4;
			}
		}
		else {
			fVar2 = (gRenderSceneConfig_SPR->frustumB).frustumMatrix.db * fVar2 +
				(gRenderSceneConfig_SPR->frustumB).frustumMatrix.dc * -fVar3;
			if ((fVar2 <= 0.0f) && ((float)(uint)param_1->field_0xc < -fVar2)) {
				return VM_4;
			}
		}
		fVar2 = (gRenderSceneConfig_SPR->frustumB).field_0x40.w;
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

EVectorMode_A ed3DTestBoundingSpherePacketSide(ed_Bound_Sphere_packet* param_1)
{
	EVectorMode_A EVar1;
	uint uVar2;
	float fVar3;
	float fVar4;

	fVar3 = gBoundSphereCenter->x;
	EVar1 = VM_1;
	if (fVar3 <= 0.0f) {
		fVar4 = -gBoundSphereCenter->z;
		fVar3 = (gRenderSceneConfig_SPR->frustumB).frustumMatrix.ba * -fVar3 +
			(gRenderSceneConfig_SPR->frustumB).frustumMatrix.bc * fVar4;
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
		fVar4 = -gBoundSphereCenter->z;
		fVar3 = (gRenderSceneConfig_SPR->frustumB).frustumMatrix.aa * -fVar3 +
			(gRenderSceneConfig_SPR->frustumB).frustumMatrix.ac * fVar4;
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
	fVar3 = gBoundSphereCenter->y;
	if (fVar3 <= 0.0f) {
		fVar3 = (gRenderSceneConfig_SPR->frustumB).frustumMatrix.cb * fVar3 +
			(gRenderSceneConfig_SPR->frustumB).frustumMatrix.cc * fVar4;
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
		fVar3 = (gRenderSceneConfig_SPR->frustumB).frustumMatrix.db * fVar3 +
			(gRenderSceneConfig_SPR->frustumB).frustumMatrix.dc * fVar4;
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

bool ed3DCheckSpherePacket(ed_3d_strip* param_1)
{
	byte bVar1;
	edF32MATRIX4* pMVar2;
	uint uVar3;
	edF32VECTOR4* pVVar4;
	int iVar5;
	EVectorMode_A EVar6;
	bool bVar7;
	int iVar8;
	ed_Bound_Sphere_packet* pfVar8;
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

	pfVar8 = (ed_Bound_Sphere_packet*)LOAD_SECTION(param_1->pTextureDataMystery);
	if (pfVar8 == (ed_Bound_Sphere_packet*)0x0) {
		bVar7 = true;
		param_1->flags_0x0 = param_1->flags_0x0 | 3;
	}
	else {
		uVar12 = (uint)(ushort)param_1->meshSectionCount_0x3a;
		uVar10 = 0;
		param_1->pDMA_Matrix = (int)STORE_SECTION(ed3D_SubAllocator_004491b8.current);
		pRVar11 = (ed_dma_matrix*)LOAD_SECTION(param_1->pDMA_Matrix);
		uVar9 = 0;
		while (pVVar4 = gBoundSphereCenter, bVar7 = uVar12 != 0, uVar12 = uVar12 - 1, bVar7) {
			fVar13 = (pfVar8->field_0x0).y;
			fVar14 = (pfVar8->field_0x0).z;
			gBoundSphereCenter->x = (pfVar8->field_0x0).x;
			pVVar4->y = fVar13;
			pVVar4->z = fVar14;
			gBoundSphereCenter->w = 1.0;
			pVVar4 = gBoundSphereCenter;
			pMVar2 = gRender_info_SPR->pMeshTransformMatrix;
			fVar21 = gBoundSphereCenter->x;
			fVar22 = gBoundSphereCenter->y;
			fVar23 = gBoundSphereCenter->z;
			fVar24 = gBoundSphereCenter->w;
			fVar13 = pMVar2->ab;
			fVar14 = pMVar2->ac;
			fVar15 = pMVar2->bb;
			fVar16 = pMVar2->bc;
			fVar17 = pMVar2->cb;
			fVar18 = pMVar2->cc;
			fVar19 = pMVar2->db;
			fVar20 = pMVar2->dc;
			gBoundSphereCenter->x =
				pMVar2->aa * fVar21 + pMVar2->ba * fVar22 + pMVar2->ca * fVar23 + pMVar2->da * fVar24;
			pVVar4->y = fVar13 * fVar21 + fVar15 * fVar22 + fVar17 * fVar23 + fVar19 * fVar24;
			pVVar4->z = fVar14 * fVar21 + fVar16 * fVar22 + fVar18 * fVar23 + fVar20 * fVar24;
			pVVar4->w = in_vf9w;
			EVar6 = ed3DTestBoundingSpherePacketZNearOnly(pfVar8);
			if (EVar6 == VM_1) {
				EVar6 = ed3DTestBoundingSpherePacketSide(pfVar8);
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
				bVar1 = pRVar11->header.byteFlags[0];
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
				bVar1 = pRVar11->header.byteFlags[0];
			}
			uVar3 = uVar10 & 0x1f;
			uVar10 = uVar10 + 2;
			pRVar11->header.byteFlags[0] = bVar1 | (byte)(iVar8 << uVar3);
			if (7 < (int)uVar10) {
				pRVar11 = (ed_dma_matrix*)((ulong)&pRVar11->header + 1);
				uVar10 = 0;
				pRVar11->header.byteFlags[0] = 0;
			}
			pfVar8 = pfVar8 + 1;
		}
		uVar10 = (ulong)(&pRVar11->header.mode) + 1;
		ed3D_SubAllocator_004491b8.current = (char*)(uVar10 + (8 - (uVar10 & 7)));
		bVar7 = uVar9 != (ushort)param_1->meshSectionCount_0x3a;
	}
	return bVar7;
}

void ed3DLinkClusterStripToViewport(ed_3d_strip* pStrip, ed_hash_code* pMBNK)
{
	edF32MATRIX4* peVar1;
	int iVar4;
	TextureData_HASH_Internal_PA32* pTVar5;
	bool bVar6;
	ed_g2d_material** ppCurrentMaterial;
	ed_dma_material* pDMA_Material;
	edF32VECTOR4* peVar9;
	EVectorMode_A EVar10;
	ed_g2d_material* pStripMaterial;
	//ed_g3d_Anim_def* peVar11;
	long lVar12;
	edLIST* pList;
	edNODE* pNode;
	int* ppNodeCount;
	edNODE_MANAGER* pDVar14;
	ed_dma_material* pExistingDMA_Material;
	ed_g2d_bitmap* pBitmap;
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
	float in_vf9w = 1.0f;
	float extraout_vf9w = 1.0f;
	float extraout_vf9w_00;
	float extraout_vf9w_01;
	float extraout_vf9w_02;
	float extraout_vf9w_03;
	ed_g2d_layer_header* iVar1;
	edNODE_MANAGER* pNodeManager;
	TextureData_TEX* iVar2;

	if (pStrip->materialIndex == -1) {
		pStripMaterial = (ed_g2d_material*)0x0;
	}
	else {
		pStripMaterial = ed3DG2DGetG2DMaterialFromIndex(pMBNK, (int)pStrip->materialIndex);
		in_vf9w = extraout_vf9w;
		if ((pStripMaterial != (ed_g2d_material*)0x0) && ((pStripMaterial->field_0x2 & 1) != 0)) {
			return;
		}
	}
	pStrip->pDMA_Matrix = 0x0;
	peVar9 = gBoundSphereCenter;
	if (gRender_info_SPR->field_0x8 == 1) {
		pStrip->flags_0x0 = pStrip->flags_0x0 & 0xfffffffc;
	}
	else {
		fVar19 = (pStrip->boundingSphere).y;
		fVar20 = (pStrip->boundingSphere).z;
		gBoundSphereCenter->x = (pStrip->boundingSphere).x;
		peVar9->y = fVar19;
		peVar9->z = fVar20;
		gBoundSphereCenter->w = 1.0f;
		peVar9 = gBoundSphereCenter;
		peVar1 = gRender_info_SPR->pMeshTransformMatrix;
		fVar27 = gBoundSphereCenter->x;
		fVar28 = gBoundSphereCenter->y;
		fVar29 = gBoundSphereCenter->z;
		fVar30 = gBoundSphereCenter->w;
		fVar19 = peVar1->ab;
		fVar20 = peVar1->ac;
		fVar21 = peVar1->bb;
		fVar22 = peVar1->bc;
		fVar23 = peVar1->cb;
		fVar24 = peVar1->cc;
		fVar25 = peVar1->db;
		fVar26 = peVar1->dc;
		gBoundSphereCenter->x = peVar1->aa * fVar27 + peVar1->ba * fVar28 + peVar1->ca * fVar29 + peVar1->da * fVar30;
		peVar9->y = fVar19 * fVar27 + fVar21 * fVar28 + fVar23 * fVar29 + fVar25 * fVar30;
		peVar9->z = fVar20 * fVar27 + fVar22 * fVar28 + fVar24 * fVar29 + fVar26 * fVar30;
		peVar9->w = in_vf9w;
		EVar10 = ed3DTestBoundingSphere(&pStrip->boundingSphere);
		if (EVar10 == VM_4) {
			return;
		}
		in_vf9w = extraout_vf9w_00;
		if (EVar10 != VM_1) {
			pStrip->flags_0x0 = pStrip->flags_0x0 | 3;
			if ((pStrip->flags_0x0 & 0x1000) == 0) {
				EVar10 = ed3DTestBoundingSphereZNearOnly(&pStrip->boundingSphere);
				if (EVar10 == VM_1) {
					pStrip->flags_0x0 = pStrip->flags_0x0 & 0xfffffffc;
					EVar10 = ed3DTestBoundingSphereSide(&pStrip->boundingSphere);
					in_vf9w = extraout_vf9w_01;
					if (EVar10 == VM_1) goto LAB_00297680;
					pStrip->flags_0x0 = pStrip->flags_0x0 | 1;
					lVar12 = ed3DCheckSpherePacket(pStrip);
					in_vf9w = extraout_vf9w_02;
				}
				else {
					lVar12 = ed3DCheckSpherePacket(pStrip);
					in_vf9w = extraout_vf9w_03;
				}
				if (lVar12 == 0) {
					return;
				}
			}
			else {
				pStrip->flags_0x0 = pStrip->flags_0x0 & 0xfffffffd;
			}
		}
	}
LAB_00297680:
	if ((pStripMaterial != (ed_g2d_material*)0x0) && (pStripMaterial->count_0x0 != 0)) {
		ED3D_LOG(LogLevel::Verbose, "ed3DLinkClusterStripToViewport materialIndex: %d", pStrip->materialIndex);

		iVar1 = (ed_g2d_layer_header*)LOAD_SECTION(*(int*)(pStripMaterial + 1));
		uVar18 = (iVar1->body).flags_0x0;
		uVar17 = (iVar1->body).flags;
		pBitmap = (ed_g2d_bitmap*)0x0;
		if ((iVar1->body).field_0x1c != 0) {
			iVar2 = (TextureData_TEX*)LOAD_SECTION((iVar1->body).pTex);
			(iVar1->body).flags = uVar17 & 0xfffffbff;
			peVar9 = gBoundSphereCenter;
			if ((uVar17 & 0x10) == 0) {
			LAB_00297798:
				if ((uVar17 & 2) == 0) {
					pStrip->flags_0x0 = pStrip->flags_0x0 & 0xfffffdff;
				}
				else {
					IMPLEMENTATION_GUARD(
						peVar11 = (ed_g3d_Anim_def*)ed3DG2DAnimTexGet(&iVar2->body);
					ed3DManageAnim(peVar11);
					pStrip->flags_0x0 = pStrip->flags_0x0 | 0x200;)
				}
			}
			else {
				fVar20 = (pStrip->boundingSphere).y;
				fVar19 = (pStrip->boundingSphere).z;
				gBoundSphereCenter->x = (pStrip->boundingSphere).x;
				peVar9->y = fVar20;
				peVar9->z = fVar19;
				gBoundSphereCenter->w = 1.0;
				peVar9 = gBoundSphereCenter;
				peVar1 = gRender_info_SPR->pMeshTransformMatrix;
				fVar27 = gBoundSphereCenter->x;
				fVar28 = gBoundSphereCenter->y;
				fVar29 = gBoundSphereCenter->z;
				fVar30 = gBoundSphereCenter->w;
				fVar19 = peVar1->ab;
				fVar20 = peVar1->ac;
				fVar21 = peVar1->bb;
				fVar22 = peVar1->bc;
				fVar23 = peVar1->cb;
				fVar24 = peVar1->cc;
				fVar25 = peVar1->db;
				fVar26 = peVar1->dc;
				gBoundSphereCenter->x =
					peVar1->aa * fVar27 + peVar1->ba * fVar28 + peVar1->ca * fVar29 + peVar1->da * fVar30;
				peVar9->y = fVar19 * fVar27 + fVar21 * fVar28 + fVar23 * fVar29 + fVar25 * fVar30;
				peVar9->z = fVar20 * fVar27 + fVar22 * fVar28 + fVar24 * fVar29 + fVar26 * fVar30;
				peVar9->w = in_vf9w;
				fVar19 = (iVar2->body).field_0x18;
				if (fVar19 <= 0.0) {
					fVar19 = gAnimSTMaxDist;
				}
				if (-gBoundSphereCenter->z < fVar19) {
					bVar6 = true;
				}
				else {
					bVar6 = false;
					if (-gBoundSphereCenter->z - fVar19 < (pStrip->boundingSphere).w) {
						bVar6 = true;
					}
				}
				if (!bVar6) goto LAB_00297798;
				pStrip->flags_0x0 = pStrip->flags_0x0 | 0x200;
			}
			if ((iVar2->body).palette == 0) {
				pTVar5 = (TextureData_HASH_Internal_PA32*)LOAD_SECTION((iVar2->body).hashCode.pData);
				if (pTVar5 != (TextureData_HASH_Internal_PA32*)0x0) {
					TextureData_PA32* pPA = (TextureData_PA32*)LOAD_SECTION(pTVar5->pPA32);
					pBitmap = (ed_g2d_bitmap*)&pPA->body;
				}
			}
			else {
				//iVar2->body.after
				ed_hash_code* pAfterHash = (ed_hash_code*)(iVar2 + 1);
				iVar4 = pAfterHash[(uint)(iVar1->body).field_0x1e].pData;
				if (iVar4 != 0) {
					ed_hash_code* pHash = (ed_hash_code*)LOAD_SECTION(iVar4);
					pBitmap = (ed_g2d_bitmap*)(((char*)LOAD_SECTION(pHash->pData)) + 0x10);
				}
			}
			if ((pStrip->flags_0x0 & 4) != 0) {
				IMPLEMENTATION_GUARD(
					peVar11 = ed3DG3DAnimGet(pStrip);
				ed3DManageAnim(peVar11);)
			}
			goto LAB_00297870;
		}
		if ((uVar17 & 0x800) != 0) goto LAB_00297870;
	}
	pBitmap = (ed_g2d_bitmap*)0x0;
	uVar18 = 0;
	uVar17 = 0;
	pStripMaterial = gDefault_Material_Cluster_Current;
LAB_00297870:
	pDMA_Material = DmaMaterialBufferCurrent;
	if ((((uVar18 & 0xfc) == 0) || ((uVar18 & 0x80000000) != 0)) || ((uVar18 & 0x4000) != 0)) {
		pExistingDMA_Material = (ed_dma_material*)LOAD_SECTION(pStripMaterial->pDMA_Material);
		if (pStripMaterial->pDMA_Material == (int)0x0) {
			pList = &DmaMaterialBufferCurrent->list;
			ppCurrentMaterial = &DmaMaterialBufferCurrent->pMaterial;
			DmaMaterialBufferCurrent = DmaMaterialBufferCurrent + 1;
			*ppCurrentMaterial = pStripMaterial;
			pDMA_Material->field_0x4 = 0;
			pDMA_Material->flags = pDMA_Material->flags & 0xfffffffe;
			(pDMA_Material->list).pNext = (edNODE*)pList;
			(pDMA_Material->list).pPrev = (edNODE*)pList;
			(pDMA_Material->list).pData = gNodeDmaMatrix;
			(pDMA_Material->list).field_0x10 = 0;
			(pDMA_Material->list).nodeCount = 0;
			(pDMA_Material->list).header.mode = 0;
			pDMA_Material->flags = pDMA_Material->flags | 1;
			pStripMaterial->pDMA_Material = STORE_SECTION(pDMA_Material);
			pDMA_Material->pBitmap = pBitmap;

			ED3D_LOG(LogLevel::Verbose, "ed3DLinkClusterStripToViewport pMaterial: %p | pBitmap: %p", pStripMaterial, pBitmap);

			if (pStrip->cameraPanIndex == 0) {
				if ((uVar17 & 0x40) == 0) {
					if ((uVar17 & 0x80) == 0) {
						if ((uVar18 & 0x80000000) == 0) {
							if ((uVar18 & 0x4000) == 0) {
								if (((uVar18 & 0x100) == 0) && ((uVar18 & 0xfc) == 0)) {
									pStrip->cameraPanIndex = 0;
								}
								else {
									pStrip->cameraPanIndex = 2;
								}
							}
							else {
								pStrip->cameraPanIndex = 4;
							}
						}
						else {
							pStrip->cameraPanIndex = 5;
						}
					}
					else {
						pStrip->cameraPanIndex = 0xb;
					}
				}
				else {
					if ((uVar18 & 0x80000000) == 0) {
						if ((uVar18 & 0x4000) == 0) {
							if ((uVar18 & 0xfc) == 0) {
								if ((uVar18 & 0x100) == 0) {
									pStrip->cameraPanIndex = 1;
								}
								else {
									pStrip->cameraPanIndex = 2;
								}
							}
							else {
								pStrip->cameraPanIndex = 3;
							}
						}
						else {
							pStrip->cameraPanIndex = 4;
						}
					}
					else {
						pStrip->cameraPanIndex = 5;
					}
				}
			}
			pNodeManager = (edNODE_MANAGER*)gPrim_List[pStrip->cameraPanIndex][gCurRenderList].pData;
			pNode = pNodeManager->pNodeHead + pNodeManager->linkCount;
			pNode->pData = pDMA_Material;
			pNode->header.typeField.type = 1;
			pNodeManager->linkCount = pNodeManager->linkCount + 1;
			ppNodeCount = &gPrim_List[pStrip->cameraPanIndex][gCurRenderList].nodeCount;
			*ppNodeCount = *ppNodeCount + 1;
			pNode->pPrev = gPrim_List[pStrip->cameraPanIndex][gCurRenderList].pPrev;
			gPrim_List[pStrip->cameraPanIndex][gCurRenderList].pPrev = pNode;
			pExistingDMA_Material = pDMA_Material;

			ED3D_LOG(LogLevel::Verbose, "ed3DLinkClusterStripToViewport Linked Cluster Strip: (%d, %d). New node count: %d", pStrip->cameraPanIndex, gCurRenderList,
				gPrim_List[pStrip->cameraPanIndex][gCurRenderList].nodeCount);
		}
		pDVar14 = (edNODE_MANAGER*)(pExistingDMA_Material->list).pData;
	}
	else {
		if (gRender_info_SPR->field_0x8 == 1) {
			gBoundSphereCenter->x = (pStrip->boundingSphere).x;
			gBoundSphereCenter->y = (pStrip->boundingSphere).y;
			gBoundSphereCenter->z = (pStrip->boundingSphere).z;
			gBoundSphereCenter->w = 1.0;
			sceVu0ApplyMatrix(gBoundSphereCenter, WorldToCamera_Matrix,
				gBoundSphereCenter);
		}
		pExistingDMA_Material = DmaMaterialBufferCurrent;
		pList = &DmaMaterialBufferCurrent->list;
		ppCurrentMaterial = &DmaMaterialBufferCurrent->pMaterial;
		DmaMaterialBufferCurrent = DmaMaterialBufferCurrent + 1;
		*ppCurrentMaterial = pStripMaterial;
		pExistingDMA_Material->field_0x4 = gBoundSphereCenter->z;
		pExistingDMA_Material->flags = pExistingDMA_Material->flags & 0xfffffffe;
		(pExistingDMA_Material->list).pNext = (edNODE*)pList;
		(pExistingDMA_Material->list).pPrev = (edNODE*)pList;
		(pExistingDMA_Material->list).pData = gNodeDmaMatrix;
		(pExistingDMA_Material->list).field_0x10 = 0;
		(pExistingDMA_Material->list).nodeCount = 0;
		(pExistingDMA_Material->list).header.mode = 0;
		pExistingDMA_Material->flags = pExistingDMA_Material->flags | 1;
		pExistingDMA_Material->pBitmap = pBitmap;
		IMPLEMENTATION_GUARD(FUN_002b1040(gPrim_List[3] + gCurRenderList, (int)pExistingDMA_Material, 1));
		pDVar14 = (edNODE_MANAGER*)(pExistingDMA_Material->list).pData;
	}
	pNode = pDVar14->pNodeHead + pDVar14->linkCount;
	pNode->pData = pStrip;
	pNode->header.typeField.type = 3;
	pNode->header.typeField.unknown = (short)pStrip->flags_0x0;
	pDVar14->linkCount = pDVar14->linkCount + 1;
	(pExistingDMA_Material->list).nodeCount = (pExistingDMA_Material->list).nodeCount + 1;
	pNode->pPrev = (pExistingDMA_Material->list).pPrev;
	pNode->pNext = (edNODE*)LOAD_SECTION(pStrip->pDMA_Matrix);
	(pExistingDMA_Material->list).pPrev = pNode;
	return;
}

void ed3DRenderCluster(ed_3d_octree* p3DOctree)
{
	bool bVar1;
	ushort uVar2;
	char* pcVar4;
	edF32MATRIX4* pMVar5;
	bool bVar6;
	edF32VECTOR4* pVVar7;
	int* piVar8;
	uint uVar9;
	int iVar10;
	char cVar11;
	int* piVar12;
	ed_3d_strip* p3DStrip;
	uint uVar14;
	ulong uVar15;
	long lVar16;
	ushort* puVar17;
	ushort* pStripCounts;
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
	edF32VECTOR4 local_270;
	edF32VECTOR4 local_260;
	edF32VECTOR4 local_250;
	edF32VECTOR4 local_240;
	edF32VECTOR4 local_230;
	undefined auStack544[32];
	float afStack512[4];
	float local_1f0[4];
	int* local_1e0;
	int local_1dc;
	undefined2 local_1d8[2];
	float local_1d4[117];
	MeshData_CDQU* pcVar3;

	pcVar3 = (MeshData_CDQU*)p3DOctree->pCDQU;
	pStripCounts = pcVar3->aClusterStripCounts;
	if (p3DOctree->field_0x30 == 1.0) {
		uVar20 = 4;
	}
	else {
		uVar20 = 0xc;
	}

	ED3D_LOG(LogLevel::Verbose, "ed3DRenderCluster Strip Count: %u | %u", pStripCounts[uVar20], uVar20);

	uVar9 = (uint)pStripCounts[uVar20];
	bVar6 = false;
	if ((uVar9 != 0) && (bVar6 = true, uVar9 != 0)) {
		pcVar4 = (char*)LOAD_SECTION(pcVar3->pMBNK);
		p3DStrip = (ed_3d_strip*)LOAD_SECTION(pcVar3->p3DStrip);
		gRender_info_SPR->field_0x8 = (uint)p3DOctree->field_0x28;
		gRender_info_SPR->pMeshTransformMatrix = WorldToCamera_Matrix;
		for (; uVar9 != 0; uVar9 = uVar9 - 1) {
			ed3DLinkClusterStripToViewport(p3DStrip, (ed_hash_code*)(pcVar4 + 0x10));
			p3DStrip = (ed_3d_strip*)LOAD_SECTION(p3DStrip->pNext);
		}
		bVar6 = true;
	}
	uVar9 = (uint) * (ushort*)((ulong)&pcVar3->field_0x2c + 2);
	if (uVar9 != 0) {
		IMPLEMENTATION_GUARD();
		iVar10 = *(int*)pcVar3[1].field_0x0;
		pcVar4 = (char*)LOAD_SECTION(pcVar3->pMBNK);
		uVar2 = p3DOctree->field_0x28;
		for (; 0 < (int)uVar9; uVar9 = uVar9 - 1) {
			//ed3DLinkClusterSpriteToViewport(iVar10, uVar2, pcVar4 + 0x10);
			iVar10 = *(int*)(iVar10 + 0xc);
		}
		bVar6 = true;
	}
	uVar9 = (uint) * (ushort*)((ulong)&pcVar3->field_0x28 + 2);
	if (uVar9 != 0) {
		IMPLEMENTATION_GUARD();
		puVar17 = pcVar3[1].aClusterStripCounts + 2;
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
		piVar8 = (int*)edChunckGetFirst((char*)((int)pStripCounts + iVar10 + 0x40), p3DOctree->pCDQU_End);
		while (true) {
			uVar14 = (uint)uVar15;
			if (piVar8 == (int*)0x0) break;
			local_1d4[uVar14 * 0x10 + 1] = p3DOctree->field_0x30;
			if ((*piVar8 == 0x55514443) || (*piVar8 == 0x434f4443)) {
				if (p3DOctree->field_0x28 == 2) {
					if (local_1d4[uVar14 * 0x10 + 1] == 1.0) {
						IMPLEMENTATION_GUARD(ed3DCalculSubBoxQuadTree((float*)(auStack544 + uVar14 * 0x40 + 0x20), (float*)p3DOctree, uVar14));
						fVar21 = extraout_vf9w;
					}
					else {
						IMPLEMENTATION_GUARD(ed3DCalculSubBoxOctree((float*)(auStack544 + uVar14 * 0x40 + 0x20), (float*)p3DOctree, uVar15));
						fVar21 = extraout_vf9w_00;
					}
					local_230.x = *(float*)(auStack544 + uVar14 * 0x40 + 0x30);
					local_230.y = local_1f0[uVar14 * 0x10 + 1];
					local_230.z = local_1f0[uVar14 * 0x10 + 2];
					local_230.w = p3DOctree->field_0x2c * 0.5;
					gRender_info_SPR->pMeshTransformMatrix = WorldToCamera_Matrix
						;
					pVVar7 = gBoundSphereCenter;
					gBoundSphereCenter->x = local_230.x;
					pVVar7->y = local_230.y;
					pVVar7->z = local_230.z;
					gBoundSphereCenter->w = 1.0;
					pVVar7 = gBoundSphereCenter;
					pMVar5 = gRender_info_SPR->pMeshTransformMatrix;
					fVar30 = gBoundSphereCenter->x;
					fVar31 = gBoundSphereCenter->y;
					fVar32 = gBoundSphereCenter->z;
					fVar33 = gBoundSphereCenter->w;
					fVar22 = pMVar5->ab;
					fVar23 = pMVar5->ac;
					fVar24 = pMVar5->bb;
					fVar25 = pMVar5->bc;
					fVar26 = pMVar5->cb;
					fVar27 = pMVar5->cc;
					fVar28 = pMVar5->db;
					fVar29 = pMVar5->dc;
					gBoundSphereCenter->x =
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
						edF32Vector4SubHard(&local_250, &local_240, &gRenderCamera->position);
						ps2_vu0_sqr_vector(&local_250, &local_250);
						uVar9 = uVar9 | uVar19;
						local_250.x = local_250.x + local_250.y + local_250.z;
						local_1d4[uVar14 * 0x10 + 2] = local_250.x;
					}
				}
				else {
					if (local_1d4[uVar14 * 0x10 + 1] == 1.0) {
						IMPLEMENTATION_GUARD(ed3DCalculSubBoxQuadTree((float*)(auStack544 + uVar14 * 0x40 + 0x20), (float*)p3DOctree, uVar14));
					}
					else {
						IMPLEMENTATION_GUARD(ed3DCalculSubBoxOctree((float*)(auStack544 + uVar14 * 0x40 + 0x20), (float*)p3DOctree, uVar15));
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
					edF32Vector4SubHard(&local_270, &local_260, &gRenderCamera->position);
					ps2_vu0_sqr_vector(&local_270, &local_270);
					uVar9 = uVar9 | uVar19;
					local_270.x = local_270.x + local_270.y + local_270.z;
					local_1d4[uVar14 * 0x10 + 2] = local_270.x;
				}
				piVar12 = (int*)p3DOctree->pCDQU_End;
			}
			else {
				piVar12 = (int*)p3DOctree->pCDQU_End;
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
		fVar21 = p3DOctree->field_0x30;
		if (fVar21 < 2.147484e+09) {
			cVar11 = (char)(int)fVar21;
		}
		else {
			cVar11 = (char)(int)(fVar21 - 2.147484e+09);
		}
		uVar9 = 0; IMPLEMENTATION_GUARD(ed3DClusterGetEdgeWithSon(uVar9, cVar11));
		if ((long)(int)uVar9 != 0) {
			IMPLEMENTATION_GUARD(ed3DRenderEdge((short*)pStripCounts, (long)(int)uVar9, (uint)p3DOctree->field_0x28, uVar20));
		}
	}
	return;
}

bool ed3DSceneRenderCluster(ed_g3d_manager* pMeshInfo)
{
	bool bVar1;
	edF32VECTOR4 local_a0;
	edF32VECTOR4 local_90;
	ed_3d_octree local_80;
	ed_3d_octree local_40;
	MeshData_CSTA* pcVar1;

	ED3D_LOG(LogLevel::Verbose, "ed3DSceneRenderCluster");

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
	edF32VECTOR4* pVVar5;
	edF32MATRIX4* pMVar6;
	edF32VECTOR4* v0;
	edF32VECTOR4 local_30[3];

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

void ed3DSetSceneRender(ed_3d_hierarchy_node* pMeshTransformData, byte param_2)
{
	(pMeshTransformData->base).field_0x88 = param_2;
	return;
}

MeshTransformObjData* ed3DChooseGoodLOD(ed_3d_hierarchy_node* pMeshTransformData)
{
	ushort uVar1;
	ed_3d_hierarchy_setup* pLVar2;
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
				edF32Matrix4MulF32Matrix4Hard(afStack64, &(pMeshTransformData->base).transformB,
					&gCurLOD_WorldToCamera_Matrix);
			pLVar2 = (pMeshTransformData->base).pHierarchySetup;
			fVar7 = gCurLOD_RenderFOVCoef * (local_8 * local_8 + local_10 * local_10 + local_c * local_c);
			if ((pLVar2 == (ed_3d_hierarchy_setup*)0x0) || (pcVar3 = pLVar2->field_0xc, pcVar3 == (char*)0x0)) {
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

PACK(struct MeshHeader {
	Hash_8 header;
	int pOBJ;
});

PACK(struct MeshData_OBJ {
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
	edF32VECTOR4 field_0x18;
	undefined field_0x28;
	undefined field_0x29;
	undefined field_0x2a;
	undefined field_0x2b;
	int field_0x2c; //struct ed_3d_strip*
});

uint ed3DTestBoundingSphereObject(edF32VECTOR4* param_1)
{
	float* pfVar1;
	uint uVar2;
	float fVar3;
	float fVar4;
	float fVar5;

	uVar2 = 1;
	fVar4 = param_1->w * gRender_info_SPR->field_0x14;
	if (((gRender_info_SPR->flags_0x10 & 0x10) == 0) ||
		(pfVar1 = (float*)gRender_info_SPR->pLightingMatrixFuncObj->field_0x0,
			pfVar1 == (float*)0x0)) {
		fVar3 = (gRenderSceneConfig_SPR->frustumA).field_0x50.w;
	}
	else {
		fVar3 = -*pfVar1;
	}
	fVar5 = gBoundSphereCenter->z;
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
	fVar3 = (gRenderSceneConfig_SPR->frustumA).field_0x40.w;
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
	fVar3 = gBoundSphereCenter->x;
	if (fVar3 <= 0.0) {
		fVar3 = (gRenderSceneConfig_SPR->frustumA).frustumMatrix.ba * -fVar3 +
			(gRenderSceneConfig_SPR->frustumA).frustumMatrix.bc * -fVar5;
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
		fVar3 = (gRenderSceneConfig_SPR->frustumA).frustumMatrix.aa * -fVar3 +
			(gRenderSceneConfig_SPR->frustumA).frustumMatrix.ac * -fVar5;
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
	fVar3 = gBoundSphereCenter->y;
	if (fVar3 <= 0.0) {
		fVar3 = (gRenderSceneConfig_SPR->frustumA).frustumMatrix.cb * fVar3 +
			(gRenderSceneConfig_SPR->frustumA).frustumMatrix.cc * -fVar5;
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
		fVar3 = (gRenderSceneConfig_SPR->frustumA).frustumMatrix.db * fVar3 +
			(gRenderSceneConfig_SPR->frustumA).frustumMatrix.dc * -fVar5;
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
	edF32VECTOR4* pVVar3;
	uint uVar4;
	int iVar5;
	edF32VECTOR4* pVVar6;
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

	pVVar3 = gBoundSphereCenter;
	pMeshOBJ = (MeshData_OBJ*)LOAD_SECTION(pMeshHeader->pOBJ);
	pRenderInput = (ed_3d_strip*)LOAD_SECTION(pMeshOBJ->field_0x2c);
	if (pRenderInput == (ed_3d_strip*)0x0) {
		return;
	}
	meshCount = pMeshOBJ->count_0x14;
	if ((gRender_info_SPR->flags_0x10 & 0x10) != 0) {
		pVVar6 = gRender_info_SPR->pLightingMatrixFuncObj->pVector_0x4;
		iVar7 = 8;
		if (pVVar6 != (edF32VECTOR4*)0x0) goto LAB_002b0d68;
	}
	pVVar6 = &pMeshOBJ->field_0x18;
	iVar7 = 2;
LAB_002b0d68:
	fVar8 = pVVar6->y;
	fVar9 = pVVar6->z;
	gBoundSphereCenter->x = pVVar6->x;
	pVVar3->y = fVar8;
	pVVar3->z = fVar9;
	gBoundSphereCenter->w = 1.0;
	pVVar3 = gBoundSphereCenter;
	pMVar2 = gRender_info_SPR->pMeshTransformMatrix;
	fVar16 = gBoundSphereCenter->x;
	fVar17 = gBoundSphereCenter->y;
	fVar18 = gBoundSphereCenter->z;
	fVar19 = gBoundSphereCenter->w;
	fVar8 = pMVar2->ab;
	fVar9 = pMVar2->ac;
	fVar10 = pMVar2->bb;
	fVar11 = pMVar2->bc;
	fVar12 = pMVar2->cb;
	fVar13 = pMVar2->cc;
	fVar14 = pMVar2->db;
	fVar15 = pMVar2->dc;
	gBoundSphereCenter->x =
		pMVar2->aa * fVar16 + pMVar2->ba * fVar17 + pMVar2->ca * fVar18 + pMVar2->da * fVar19;
	pVVar3->y = fVar8 * fVar16 + fVar10 * fVar17 + fVar12 * fVar18 + fVar14 * fVar19;
	pVVar3->z = fVar9 * fVar16 + fVar11 * fVar17 + fVar13 * fVar18 + fVar15 * fVar19;
	pVVar3->w = in_vf9w;
	uVar4 = ed3DTestBoundingSphereObject(pVVar6);
	if (uVar4 != 4) {
		if (uVar4 == 1) {
			gRender_info_SPR->field_0x8 = 1;
		}
		else {
			IMPLEMENTATION_GUARD(
				iVar5 = ed3DTestBoundingSphereObjectNoZFar(pVVar6);
			if (iVar5 != 1) {
				gRender_info_SPR->field_0x8 = iVar7;
			})
		}
		if (*gShadowRenderMask == 0) {
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

void ed3DRenderSonHierarchy(ed_3d_hierarchy_node* pMeshTransformData)
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
			edF32Matrix4MulF32Matrix4Hard(&MStack64, &(pMeshTransformData->base).transformB,
				WorldToCamera_Matrix);
			pMVar3 = &(pMeshTransformData->base).transformB;
			gRender_info_SPR->pMeshTransformMatrix = &MStack64;
			gRender_info_SPR->pMeshTransformData = pMeshTransformData;
			gRender_info_SPR->pLightingMatrixFuncObj =
				(pMeshTransformData->base).pHierarchySetup;
			gRender_info_SPR->pSharedMeshTransform = pMVar3;
			gRender_info_SPR->flags_0x10 =
				(uint)(pMeshTransformData->base).flags_0x9e;
			fVar4 = ed3DMatrixGetBigerScale(pMVar3);
			gRender_info_SPR->field_0x14 = fVar4;
			gRender_info_SPR->field_0x18 = 0;
			gRender_info_SPR->field_0x8 = 1;
			(pMeshTransformData->base).pMatrixPkt = (edF32MATRIX4*)0x0;
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

void ed3DRenderSonHierarchyForShadow(ed_3d_hierarchy_node* pMeshTransformData)
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
		((*gShadowRenderMask & (uint)(pMeshTransformData->base).bRenderShadow) != 0)) &&
		((uVar2 & 0x41) == 0)) && ((pMeshTransformData->base).count_0x9c != 0)) {
		pMVar4 = ed3DChooseGoodLOD(pMeshTransformData);
		if (((pMeshTransformData->base).flags_0x9e & 0x80) == 0) {
			(pMeshTransformData->base).size_0xae = 0xff;
		}
		if ((pMVar4 != (MeshTransformObjData*)0x0) && (pMVar4->pObj != (char*)0x0)) {
			edF32Matrix4MulF32Matrix4Hard(&MStack64, &(pMeshTransformData->base).transformB,
				WorldToCamera_Matrix);
			pMVar6 = &(pMeshTransformData->base).transformB;
			gRender_info_SPR->pMeshTransformMatrix = &MStack64;
			gRender_info_SPR->pMeshTransformData = pMeshTransformData;
			gRender_info_SPR->pLightingMatrixFuncObj =
				(pMeshTransformData->base).pHierarchySetup;
			gRender_info_SPR->pSharedMeshTransform = pMVar6;
			gRender_info_SPR->flags_0x10 =
				(uint)(pMeshTransformData->base).flags_0x9e;
			fVar7 = ed3DMatrixGetBigerScale(pMVar6);
			gRender_info_SPR->field_0x14 = fVar7;
			gRender_info_SPR->field_0x18 = 0;
			(pMeshTransformData->base).pMatrixPkt = (edF32MATRIX4*)0x0;
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

uint ed3DSceneRenderOne(ed_3D_Scene* pShadowScene, ed_3D_Scene* pScene)
{
	ed_g3d_manager* pMeshInfo;
	float fVar1;
	ed_viewport* pCVar2;
	int iVar3;
	bool bVar4;
	uint uVar5;
	uint uVar6;
	//ProfileObject* pPVar7;
	edNODE* pCVar10;
	edNODE* pCVar9;
	RenderTaskData renderTaskData;
	edNODE* pCVar8;

	RENDER_LOG("\n--------------------------------------------");
	RENDER_LOG("ed3DSceneRenderOne Scene %p (%s) (shadow: %d)", pShadowScene, gSceneNames[GetStaticMeshMasterIndex(pShadowScene)], pShadowScene->bShadowScene);
	RENDER_LOG("ed3DSceneRenderOne Shadow Scene %p (%s) (shadow: %d)", pScene, gSceneNames[GetStaticMeshMasterIndex(pScene)], pScene->bShadowScene);

	iVar3 = gIDProfileFlush;
	if ((pShadowScene->bShadowScene == 1) && ((pScene->flags_0x4 & 0x10) != 0)) {
		RENDER_LOG("ed3DSceneRenderOne SKIP");
		uVar6 = 0;
	}
	else {
		RENDER_LOG("ed3DSceneRenderOne DRAW");
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
		uVar6 = ed3DInitRenderEnvironement(pShadowScene, (ulong)((pShadowScene->flags_0x4 & 0x20) != 0));
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
			g_VifRefPktCur = ed3DSceneAddContextPacket(pShadowScene, g_VifRefPktCur);
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
			pCVar8 = (edNODE*)pScene->pHeirListA;
			pCVar9 = pCVar8->pPrev;
			if (*gShadowRenderMask == 0) {
				if (ged3DConfig.field_0x2d == 0) {
					for (; pCVar9 != pCVar8; pCVar9 = pCVar9->pPrev) {
						ed3DRenderSonHierarchy((ed_3d_hierarchy_node*)pCVar9->pData);
					}
				}
				else {
					for (; pCVar9 != pCVar8; pCVar9 = pCVar9->pPrev) {
						IMPLEMENTATION_GUARD(ed3DRenderSonHierarchyHIDDEN(pCVar9->pData);)
					}
				}
			}
			else {
				for (; pCVar9 != pCVar8; pCVar9 = pCVar9->pPrev) {
					ed3DRenderSonHierarchyForShadow((ed_3d_hierarchy_node*)pCVar9->pData);
				}
			}
			ed3DClustersRenderComputeSceneHierarchyList(pShadowScene);
			if ((pShadowScene->bShadowScene != 1) &&
				(ed3DSceneSortClusters(pShadowScene), (pShadowScene->flags_0x4 & 8) == 0)) {
				for (pCVar10 = (edNODE*)(pScene->headerB).pPrev;
					(edLIST*)pCVar10 != &pScene->headerB; pCVar10 = pCVar10->pPrev) {
					edCluster* pCluster = (edCluster*)pCVar10->pData;
					if (((pCluster->flags & 2) == 0) &&
						(pMeshInfo = pCluster->pMeshInfo, pMeshInfo->CSTA != (char*)0x0)) {
						bVar4 = ed3DSceneRenderCluster(pMeshInfo);
						uVar6 = (int)bVar4 & 0xff;
					}
				}
			}
			renderTaskData.pStaticMeshMaster = pScene;
			if (BYTE_00448a5c == 0) {
				if (pShadowScene == pScene) {
					renderTaskData.isChild = pShadowScene->bShadowScene;
					renderTaskData.taskID = 2;
					edSysHandlersCall(sysHandler_0048cb90.mainIdentifier, sysHandler_0048cb90.entries,
						sysHandler_0048cb90.maxEventID, 0, &renderTaskData);
				}
				else {
					renderTaskData.isChild = pShadowScene->bShadowScene;
					renderTaskData.taskID = 1;
					if (pShadowScene != pScene) {
						renderTaskData.taskID = 3;
					}
					edSysHandlersCall(sysHandler_0048cb90.mainIdentifier, sysHandler_0048cb90.entries,
						sysHandler_0048cb90.maxEventID, 0, &renderTaskData);
				}
			}
			else {
				renderTaskData.isChild = pShadowScene->bShadowScene;
				renderTaskData.taskID = 1;
				if (pShadowScene != pScene) {
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
			if (pShadowScene->bShadowScene == 1) {
				g_VifRefPktCur->cmdA = ED_VIF1_SET_TAG_CNT(2);
				g_VifRefPktCur->cmdB = 0;
				g_VifRefPktCur->asU32[2] = SCE_VIF1_SET_NOP(0);
				g_VifRefPktCur->asU32[3] = SCE_VIF1_SET_DIRECT(2, 0);
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
					pShadowScene->pViewport->pZBuffer->frameBasePtr,	// ZBP
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
			pCVar2 = pShadowScene->pViewport;
			CameraScreenData_00449560.x = pCVar2->posX;
			CameraScreenData_00449560.y = pCVar2->posY;
			CameraScreenData_00449560.w = pCVar2->screenWidth;
			CameraScreenData_00449560.h = pCVar2->screenHeight;
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

	RENDER_LOG("ed3DSceneRenderDlist %p (%d) (child: %d)", pStaticMeshMaster, GetStaticMeshMasterIndex(pStaticMeshMaster), pStaticMeshMaster->bShadowScene);

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
				pRVar2->cmdA = ED_VIF1_SET_TAG_CNT(uVar4 & 0xffffffffU);
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
			param_1->cmdA = ED_VIF1_SET_TAG_CNT(uVar4 & 0xffffffffU);
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
	param_1->cmdA = ED_VIF1_SET_TAG_CNT(3);
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
		(gCurRectViewport.w == 0x200)) &&
		((gCurRectViewport.h == 0x200 || (gCurRectViewport.h == 0x1c0)))) {
		IMPLEMENTATION_GUARD();
	}
	return pRenderCommand;
}

void ed3DPrimlistTermMaterialRenderList(void)
{
	if (gPrim_List_FlushTex_Last != (edNODE*)0x0) {
		gPrim_List_FlushTex_Last->pPrev = gPrim_List_FlushTex + gCurRenderList;
		RENDER_LOG("ed3DPrimlistTermMaterialRenderList %p", (MeshTransformSpecial*)(gPrim_List_FlushTex + gCurRenderList));
	}
	return;
}

edF32VECTOR4 Vector_0048c3d0 = { 0 };
byte BYTE_0044941c = 0;

int ed3DVU1Addr[3] = { 0x76, 0x198, 0x2BA };
int ed3DVU1AddrWithBufCur[3][3] = { { 0x76, 0x198, 0x2BA }, {0x198, 0x2BA, 0x76}, {0x2BA, 0x76, 0x198} };

void ed3DRefreshSracthGlobalVar(void)
{
	AnimScratchpad* pAVar1;
	int in_zero_lo;
	uint in_zero_hi;
	undefined4 in_zero_udw;
	undefined4 in_register_0000000c;
	int iVar2;
	edF32MATRIX4* pMVar3;
	edF32VECTOR4* pVVar4;
	edF32MATRIX4* pMVar5;
	float fVar6;
	float fVar7;
	float fVar8;

	if (gLightDirections_Matrix != (edF32VECTOR4*)0x0) {
		iVar2 = 8;
		pVVar4 = gLightDirections_Matrix;
		pMVar3 = gLightDirections_Matrix_Scratch;
		do {
			iVar2 = iVar2 + -1;
			fVar6 = pVVar4->y;
			pMVar3->aa = pVVar4->x;
			pVVar4 = (edF32VECTOR4*)&pVVar4->z;
			pMVar3->ab = fVar6;
			pMVar3 = (edF32MATRIX4*)&pMVar3->ac;
		} while (0 < iVar2);
	}
	if (gLightColor_Matrix != (edF32VECTOR4*)0x0) {
		iVar2 = 8;
		pVVar4 = gLightColor_Matrix;
		pMVar3 = gLightColor_Matrix_Scratch;
		do {
			iVar2 = iVar2 + -1;
			fVar6 = pVVar4->y;
			pMVar3->aa = pVVar4->x;
			pVVar4 = (edF32VECTOR4*)&pVVar4->z;
			pMVar3->ab = fVar6;
			pMVar3 = (edF32MATRIX4*)&pMVar3->ac;
		} while (0 < iVar2);
	}
	pVVar4 = gLightAmbiant_Scratch;
	if (gLightAmbiant != (edF32VECTOR4*)0x0) {
		fVar8 = gLightAmbiant->y;
		fVar6 = gLightAmbiant->z;
		fVar7 = gLightAmbiant->w;
		gLightAmbiant_Scratch->x = gLightAmbiant->x;
		pVVar4->y = fVar8;
		pVVar4->z = fVar6;
		pVVar4->w = fVar7;
	}
	fVar8 = Vector_0048c3d0.w;
	fVar7 = Vector_0048c3d0.z;
	fVar6 = Vector_0048c3d0.y;
	pVVar4 = gVU1_AnimST_NormalExtruder_Scratch;
	pMVar5 = &gF32Matrix4Unit;
	iVar2 = 8;
	gVU1_AnimST_NormalExtruder_Scratch->x = Vector_0048c3d0.x;
	pVVar4->y = fVar6;
	pVVar4->z = fVar7;
	pVVar4->w = fVar8;
	pMVar3 = gF32Matrix4Unit_Scratch;
	do {
		iVar2 = iVar2 + -1;
		fVar6 = pMVar5->ab;
		pMVar3->aa = pMVar5->aa;
		pMVar5 = (edF32MATRIX4*)&pMVar5->ac;
		pMVar3->ab = fVar6;
		pMVar3 = (edF32MATRIX4*)&pMVar3->ac;
	} while (0 < iVar2);
	*gbDoMipmap_SPR = (uint)gbDoMipmap;
	*gMipmapL_SPR = gMipmapL;
	*gMipmapK_SPR = gMipmapK;
	*gbTrilinear_SPR = (uint)BYTE_0044941c;
	pAVar1 = PTR_AnimScratchpad_00449554;
	PTR_AnimScratchpad_00449554->field_0x0 = 0;
	pAVar1->flags = 0;
	pAVar1->field_0x8 = 0;
	pAVar1->field_0xc = 0;
	*g_pCurFlareMtx = SCRATCHPAD_ADDRESS_TYPE(0x700008a0, edF32MATRIX4*);
	memcpy(ed3DVU1Addr_Scratch, ed3DVU1Addr, sizeof(ed3DVU1Addr));
	memcpy(ed3DVU1AddrWithBufCur_Scratch, ed3DVU1AddrWithBufCur, sizeof(ed3DVU1AddrWithBufCur));
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
						g_VifRefPktCur->cmdA = ED_VIF1_SET_TAG_REF(0, 0);
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
						g_VifRefPktCur->cmdA = ED_VIF1_SET_TAG_REF(0, 0);
						g_VifRefPktCur->cmdB = 0x1300000000000000;
						g_VifRefPktCur[1].cmdA = ED_VIF1_SET_TAG_REF(0, 0);
						g_VifRefPktCur[1].cmdB = 0x1100000011000000;
						g_VifRefPktCur[2].cmdA = ED_VIF1_SET_TAG_REF(0, 0);
						g_VifRefPktCur[2].cmdB = 0x600000000000000;
						g_VifRefPktCur[3].cmdA = ED_VIF1_SET_TAG_CNT(1);
						g_VifRefPktCur[3].cmdB = 0;
						pRVar5 = g_VifRefPktCur;
						*(undefined4*)&g_VifRefPktCur[4].cmdA = in_zero_lo;
						*(undefined4*)((ulong)&pRVar5[4].cmdA + 4) = in_zero_hi;
						*(undefined4*)&pRVar5[4].cmdB = in_zero_udw;
						*(undefined4*)((ulong)&pRVar5[4].cmdB + 4) = in_register_0000000c;
						g_VifRefPktCur[5].cmdA = ED_VIF1_SET_TAG_REF(0, 0);
						g_VifRefPktCur[5].cmdB = 0x600800000000000;
						g_VifRefPktCur = g_VifRefPktCur + 6;
					}
				}
				pRVar5 = g_VifRefPktCur;
				pInStaticMeshMaster = gScene3D + staticMeshMasterIndex;

				RENDER_LOG("ed3DSceneRender Processing %p (%d)", pInStaticMeshMaster, staticMeshMasterIndex);

				if (gCurViewportUsed == (ed_viewport*)0x0) {
					gCurViewportUsed = pInStaticMeshMaster->pViewport;
				}
				pCVar1 = pInStaticMeshMaster->pViewport;
				gCurRectViewport.x = pCVar1->posX;
				gCurRectViewport.y = pCVar1->posY;
				gCurRectViewport.w = pCVar1->screenWidth;
				gCurRectViewport.h = pCVar1->screenHeight;
				gCurScene = pInStaticMeshMaster;
				ed3DSceneRenderOne(pInStaticMeshMaster, pInStaticMeshMaster);

				RENDER_LOG("ed3DSceneRender Begin Processing Children");
				for (pCVar12 = (edLIST*)(pInStaticMeshMaster->headerA).pPrev;
					pCVar12 != &pInStaticMeshMaster->headerA; pCVar12 = (edLIST*)pCVar12->pPrev) {
					if ((((ed_3D_Scene*)pCVar12->pData)->flags_0x4 & 4) == 0) {
						ed3DSceneRenderOne((ed_3D_Scene*)pCVar12->pData, pInStaticMeshMaster);
					}
				}
				RENDER_LOG("ed3DSceneRender End Processing Children");

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
			g_VifRefPktCur->cmdA = ED_VIF1_SET_TAG_REF(0, 0);
			g_VifRefPktCur->cmdB = 0;
			*(undefined4*)&g_VifRefPktCur->cmdB = 0x6000000;
			g_VifRefPktCur = g_VifRefPktCur + 1;
			g_VifRefPktCur->cmdA = ED_VIF1_SET_TAG_CNT(2);
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

void ed3DPrimListInit(int materialMatrixNodeCount, int renderNodeCount)
{
	int iVar1;
	int iVar2;

	pNodeManRender = edListCreateNodesTable(renderNodeCount, TO_HEAP(H_MAIN));
	pNodeManMaterial[0] = edListCreateNodesTable(materialMatrixNodeCount, TO_HEAP(H_MAIN));
	pNodeManMaterial[1] = edListCreateNodesTable(materialMatrixNodeCount, TO_HEAP(H_MAIN));

	iVar2 = 0;
	do {
		edListLink(gPrim_List[iVar2], pNodeManMaterial[0], 0, 0);
		edListClear(gPrim_List[iVar2]);
		edListLink(gPrim_List[iVar2] + 1, pNodeManMaterial[1], 0, 0);
		edListClear(gPrim_List[iVar2] + 1);
		iVar2 = iVar2 + 1;
	} while (iVar2 < 0xf);

	ed3DShadowInitPrimList(pNodeManMaterial);

	gCurRenderList = 0;
	gCurFlushList = 0;

	iVar2 = 0;
	do {
		iVar1 = iVar2 + 1;
		gPrim_List_FlushTex[iVar2].pPrev = (gPrim_List_FlushTex + iVar2);
		gPrim_List_FlushTex[iVar2].nodeCount = 0;
		iVar2 = iVar1;
	} while (iVar1 < 2);

	return;
}

edLIST* gManager3D[2] = { NULL, NULL };

char* g_szed3DAllocInfo_004333d0 = "ed3D Init total allocation ammount:<< %d >>";

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

void ed3DHierarchyManagerInit(void)
{
	gHierarchyManagerBuffer = (ed_3d_hierarchy_node*)edMemAlloc(TO_HEAP(H_MAIN), ged3DConfig.meshTransformDataCount * sizeof(ed_3d_hierarchy_node));
	gHierarchyManagerNodeManager =
		edListCreateNodesTable(ged3DConfig.meshTransformDataCount, TO_HEAP(H_MAIN));
	gHierarchyManagerFirstFreeNode = (edNODE*)gHierarchyManagerNodeManager->pNodeHead;
	gScene_list_buffer = edMemAlloc(TO_HEAP(H_MAIN), ged3DConfig.sceneCount * sizeof(edLIST) * 3);
	return;
}

void ed3DListBufferInit(int materialBufferCount, int matrixBufferCount)
{
	ed_dma_material* peVar1;
	ed_dma_material** ppeVar2;
	ed_dma_material** ppeVar3;
	uint uVar4;

	uVar4 = 0;
	do {
		peVar1 = (ed_dma_material*)edMemAlloc(TO_HEAP(H_MAIN), materialBufferCount * sizeof(ed_dma_material));
		DmaMaterialBuffer[uVar4] = peVar1;
		DmaMaterialBufferMax[uVar4] = peVar1 + materialBufferCount;
		uVar4 = uVar4 + 1 & 0xffff;
	} while (uVar4 < 2);
	DmaMatrixBuffer = (ed_dma_matrix*)edMemAlloc(TO_HEAP(H_MAIN), matrixBufferCount * sizeof(ed_dma_matrix));
	DmaMaterialBufferCurrent = DmaMaterialBuffer[0];
	DmaMaterialBufferCurrentMax = DmaMaterialBufferMax[0];
	DmaMatrixBufferMax = DmaMatrixBuffer + matrixBufferCount;
	DmaMatrixBufferCurrent = DmaMatrixBuffer;
	DmaMatrixBufferCurrentMax = DmaMatrixBuffer + matrixBufferCount;
	return;
}

ed_g3d_manager* gpG3D = NULL;
edCluster* gCluster = NULL;

void ed3DPrepareMaterial(ed_g2d_material* pBuffer, bool param_2)
{
	TextureData_HASH_Internal_PA32* pTVar1;
	uint uVar2;
	undefined* puVar3;
	bool bVar4;
	undefined4* puVar5;
	ulong* puVar6;
	undefined8 uVar7;
	int puVar8;
	undefined* puVar9;
	edpkt_data* pStripBuf;
	uint uVar11;
	ed_g2d_layer* pLAY;
	ed_g2d_bitmap* peVar12;
	uint uVar13;
	TextureData_TEX_Internal* pTVar14;
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
	ed_g2d_layer_header* iVar1;

	puVar8 = 0x10;
	pTVar14 = (TextureData_TEX_Internal*)0x0;
	peVar12 = (ed_g2d_bitmap*)0x0;
	puVar9 = local_10;
	puVar3 = puVar9;
	while (pStripBuf = g_pStrippBufLastPos, puVar3 != (undefined*)0x0) {
		*puVar9 = 0;
		puVar9 = puVar9 + 1;
		puVar8 = puVar8 + -1;
		puVar3 = (undefined*)puVar8;
	}
	for (; ((uint)pStripBuf & 0xf) != 0; pStripBuf = pStripBuf + 1) {
	}
	pBuffer->pCommandBufferTexture = STORE_SECTION(pStripBuf);
	pBuffer->commandBufferTextureSize = 0;
	uVar13 = 0;
	do {
		if (pBuffer->count_0x0 == 0) {
			if (param_2 == true) {
				pStripBuf->cmdA = ED_VIF1_SET_TAG_CNT(1);
				pStripBuf->cmdB = 0;
				pStripBuf->asU32[2] = SCE_VIF1_SET_NOP(0);
				pStripBuf->asU32[3] = SCE_VIF1_SET_ITOP(0x3dc, 0);

				pStripBuf[1].asU32[0] = 0;
				pStripBuf[1].asU32[1] = SCE_VIF1_SET_MSCAL(0, 0);

				pStripBuf[1].cmdB = 0;
				pBuffer->commandBufferTextureSize = 2;
				pStripBuf = pStripBuf + 2;
			}
			else {
				pStripBuf->cmdA = ED_VIF1_SET_TAG_CNT(0);
				pStripBuf->cmdB = SCE_VIF1_SET_NOP(0);
				pBuffer->commandBufferTextureSize = 1;
				pStripBuf = pStripBuf + 1;
			}
		}
		else {
			local_20 = gVRAMClutBuffers[uVar13];
			local_1c = gVRAMBuffers[uVar13];
			local_18 = local_40 + local_1c;
			for (uVar11 = 0; uVar11 < pBuffer->count_0x0; uVar11 = uVar11 + 1) {
				ed_hash_code* pHash = (ed_hash_code*)(pBuffer + 1);
				iVar1 = (ed_g2d_layer_header*)LOAD_SECTION(pHash[uVar11].pData);
				pLAY = &iVar1->body;
				if ((pLAY != (ed_g2d_layer*)0x0) && ((iVar1->body).field_0x1c != 0)) {
					pTVar14 = &((TextureData_TEX*)LOAD_SECTION((iVar1->body).pTex))->body;
				}
				if (pTVar14 == (TextureData_TEX_Internal*)0x0) {
					if (param_2 == true) {
						pStripBuf->cmdA = ED_VIF1_SET_TAG_CNT(1);
						pStripBuf->cmdB = SCE_VIF1_SET_NOP(0);
						pStripBuf->asU32[2] = SCE_VIF1_SET_NOP(0);
						pStripBuf->asU32[3] = SCE_VIF1_SET_ITOP(0x3dc, 0);

						pStripBuf[1].asU32[0] = 0;
						pStripBuf[1].asU32[1] = SCE_VIF1_SET_MSCAL(0, 0);
						pStripBuf[1].cmdB = 0;
						pBuffer->commandBufferTextureSize = 2;
						pStripBuf = pStripBuf + 2;
					}
					else {
						pStripBuf->cmdA = ED_VIF1_SET_TAG_CNT(0);
						pStripBuf->cmdB = SCE_VIF1_SET_NOP(0);
						pBuffer->commandBufferTextureSize = 1;
						pStripBuf = pStripBuf + 1;
					}
				}
				else {
					pTVar1 = (TextureData_HASH_Internal_PA32*)LOAD_SECTION(pTVar14->hashCode.pData);
					if (pTVar1 != (TextureData_HASH_Internal_PA32*)0x0) {
						peVar12 = &((TextureData_PA32*)LOAD_SECTION(pTVar1->pPA32))->body;
					}
					if (peVar12 == (ed_g2d_bitmap*)0x0) {
						if (param_2 == true) {
							pStripBuf->cmdA = ED_VIF1_SET_TAG_CNT(1);
							pStripBuf->cmdB = 0;
							pStripBuf->asU32[2] = SCE_VIF1_SET_FLUSH(0);
							pStripBuf->asU32[3] = SCE_VIF1_SET_ITOP(0x03dc, 0);
							pStripBuf[1].asU32[0] = SCE_VIF1_SET_NOP(0);
							pStripBuf[1].asU32[1] = SCE_VIF1_SET_MSCAL(0, 0);
							pStripBuf[1].cmdB = SCE_VIF1_SET_FLUSH(0);
							pBuffer->commandBufferTextureSize = 2;
							pStripBuf = pStripBuf + 2;
						}
						else {
							pStripBuf->cmdA = ED_VIF1_SET_TAG_CNT(0);
							pStripBuf->cmdB = SCE_VIF1_SET_NOP(0);
							pBuffer->commandBufferTextureSize = 1;
							pStripBuf = pStripBuf + 1;
						}
					}
					else {
						IMPLEMENTATION_GUARD(
							local_8 = (uint)peVar12->width;
						fVar15 = (float)(uint)peVar12->height;
						if (fVar15 < 2.147484e+09) {
							local_4 = (uint)fVar15;
						}
						else {
							local_4 = (int)(fVar15 - 2.147484e+09) | 0x80000000;
						}
						if (peVar12->psm == 0x18) {
							local_40 = local_8 * local_4 * 4 >> 8;
						}
						else {
							local_40 = (uint)peVar12->psm * local_8 * local_4 >> 0xb;
						}
						uVar2 = gLayerProp;
						if (pLAY != (ed_g2d_layer*)0x0) {
							uVar2 = gLayerProp | pLAY->flags_0x0;
							pLAY->flags_0x0 = uVar2;
						}
						if (param_2 == true) {
							IMPLEMENTATION_GUARD();
							pStripBuf->cmdA = ED_VIF1_SET_TAG_CNT(0xb);
							pStripBuf->cmdB = SCE_VIF1_SET_NOP(0);
							*(undefined4*)&pStripBuf->cmdB = 0x40003dc;
							*(undefined4*)((int)&pStripBuf->cmdB + 4) = 0x6c0a03dc;
						}
						else {
							pStripBuf->cmdA = ED_VIF1_SET_TAG_CNT(0xa);
							pStripBuf->cmdB = SCE_VIF1_SET_NOP(0);
						}

						IMPLEMENTATION_GUARD();
						pStripBuf[1].cmdA = 0x1000000000008009;
						pStripBuf[1].cmdB = 0xe;
						bVar4 = ed3DFXIsTrue(pLAY);
						if (bVar4 == false) {
							uVar7 = ed3DSetBasicTexPacket(pStripBuf + 2, &local_20, peVar12, pLAY);
						}
						else {
							puVar5 = (undefined4*)ed3DGetFxConfigFromFlag(uVar2);
							local_30 = puVar5[1];
							local_2c = *puVar5;
							local_28 = *puVar5;
							uVar7 = ed3DSetBasicTexPacket(pStripBuf + 2, &local_30, peVar12, pLAY);
						}
						puVar6 = (ulong*)ed3DSetAlphaPacket(uVar7, (uVar2 & 0x2000) != 0, pLAY, uVar11);
						*puVar6 = (long)(int)gMipmapK << 0x20 |
							(long)(int)(peVar12->field_0x6 - 1) << 2 | 0x60U | (ulong)gMipmapL << 0x13;
						puVar6[1] = 0x14;
						puVar6 = ed3DSetMipmapPacket(puVar6 + 2, (int)&local_20, (ushort*)peVar12);
						pStripBuf = (edpkt_data*)(puVar6 + 2);
						*puVar6 = (ulong)((pLAY->flags_0x0 & 1) == 0) | (ulong)((pLAY->flags_0x0 & 2) == 0) << 2;
						puVar6[1] = 8;
						if (param_2 == true) {
							IMPLEMENTATION_GUARD();
							*(undefined4*)&pStripBuf->cmdA = 0;
							*(undefined4*)((int)puVar6 + 0x14) = 0x14000000;
							*(undefined4*)(puVar6 + 3) = 0;
							*(undefined4*)((int)puVar6 + 0x1c) = 0;
							pBuffer->commandBufferTextureSize = 0xc;
							pStripBuf = (edpkt_data*)(puVar6 + 4);
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
	g_pStrippBufLastPos = pStripBuf;
	return;
}

int gLibInitialized = 0;
uint gZbuf_ScreenZMax = 0;

static bool bAlreadyInit;

int gFlagOptionVU1[3];

void ed3DOptionFlagReset(void)
{
	gFlagOptionVU1[0] = -1;
	gFlagOptionVU1[1] = -1;
	gFlagOptionVU1[2] = -1;
	return;
}

void ed3DFlushInit()
{
	ed3DOptionFlagReset();
}

undefined* gClusterTabList = NULL;

void ed3DClusterInit(void)
{
	gClusterTabList = (undefined*)edMemAlloc(TO_HEAP(H_MAIN), (uint)(ushort)ged3DConfig.clusterTabListSize * sizeof(undefined*));
	return;
}

void ed3DInit(void)
{
	ed_g2d_manager* pTVar1;
	uint uVar2;
	int size;

	edDebugPrintf(s_ed3D_Initialsation_004333a0);
	ed3DPrepareVRAMBuf();
	gLibInitialized = 1;
	gManager3D[0] = edListNew(1, ged3DConfig.g3dManagerCount, TO_HEAP(H_MAIN));
	gManager3D[1] = edListNew(1, ged3DConfig.g2dManagerCount, TO_HEAP(H_MAIN));
#ifdef EDITOR_BUILD
	edListSetName(gManager3D[0], "gManager3D[3D]");
	edListSetName(gManager3D[1], "gManager3D[2D]");
#endif
	gpG3D = (ed_g3d_manager*)edMemAlloc(TO_HEAP(H_MAIN), ged3DConfig.g3dManagerCount * sizeof(ed_g3d_manager));
	memset(gpG3D, 0, ged3DConfig.g3dManagerCount * sizeof(ed_g3d_manager));
	gpG2D = (ed_g2d_manager*)edMemAlloc(TO_HEAP(H_MAIN), ged3DConfig.g2dManagerCount * sizeof(ed_g2d_manager));
	memset(gpG2D, 0, ged3DConfig.g2dManagerCount * sizeof(ed_g2d_manager));
	pTVar1 = gpG2D;
	gNbG3D = 0;
	gNbG2D = 0;
	*(undefined8*)gpG2D = 0x3fff00000000;
	*(undefined8*)&pTVar1->textureHeaderStart = 0x50;
	*(undefined8*)&pTVar1->pT2DA = 0x7ffe000000000;
	memset(&ged3DFXConfig, 0, sizeof(ed3DFXConfig) * 16);
	gScene3D = (ed_3D_Scene*)edMemAlloc(TO_HEAP(H_MAIN), ged3DConfig.sceneCount * sizeof(ed_3D_Scene) + 1);
	memset(gScene3D, 0, ged3DConfig.sceneCount * sizeof(ed_3D_Scene));
	gNodeCluster = edListCreateNodesTable(ged3DConfig.maxClusterCount + ged3DConfig.sceneCount * 3, TO_HEAP(H_MAIN));
	gCluster = (edCluster*)edMemAlloc(TO_HEAP(H_MAIN), ged3DConfig.maxClusterCount * sizeof(edCluster));
	memset(gCluster, 0, ged3DConfig.maxClusterCount * sizeof(edCluster));
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
	g_pStripBuf = (edpkt_data*)edMemAlloc(TO_HEAP(H_MAIN), ged3DConfig.meshHeaderCountBAlt);
	g_pStrippBufMaxPos = (edpkt_data*)(((char*)g_pStripBuf) + ged3DConfig.meshHeaderCountBAlt);
	size = (ged3DConfig.field_0x28 * 2 + ged3DConfig.field_0x18 + 0x8c0 + ged3DConfig.matrixBufferCount * 0x38 + ged3DConfig.field_0x34 * 8) * 0x10;
	g_pStrippBufLastPos = g_pStripBuf;
	ed3D_Allocator_00449248.base = (char*)edMemAlloc(TO_HEAP(H_MAIN), size);
	memset(ed3D_Allocator_00449248.base, 0, size);
	ed3D_Allocator_00449248.current = ed3D_Allocator_00449248.base + size;
	ed3D_Allocator_00449248.end = ed3D_Allocator_00449248.base;
	ed3D_SubAllocator_004491b8.base = SubAllocate_0029ce60(16000);
	ed3D_SubAllocator_004491b8.end = ed3D_SubAllocator_004491b8.base + 16000;
	ed3D_SubAllocator_004491b8.current = ed3D_SubAllocator_004491b8.base;
	ed3DListBufferInit(ged3DConfig.materialBufferCount, ged3DConfig.matrixBufferCount);
	ed3DClusterInit();
	ed3DPrimListInit(ged3DConfig.materialBufferCount, ged3DConfig.meshHeaderCountB);
	ed3DDMAInit(ged3DConfig.field_0x18 + ged3DConfig.field_0x34 * 8, ged3DConfig.field_0x28);
	ed3DFlushInit();
	ed3DFlushTexInit();
	if (bAlreadyInit == 0) {
		bAlreadyInit = 1;
	}
	gCurViewportUsed = (ed_viewport*)0x0;
	ed3DVU1BufferInit();
	ed3DFlushListInit();
	ed3DPrimlistInitMaterialRenderList();
	ed3DPrimlistMatrixBufferReset();
	ed3DShadowResetPrimList();
	ed3DHierarchyManagerInit();
	ed3DPrepareMaterial(&gDefault_Material_Gouraud_Cluster, true);
	ed3DPrepareMaterial(&gDefault_Material_Gouraud, true);
	ed3DPrepareMaterial(&gDefault_Material_Flat_Cluster, true);
	ed3DPrepareMaterial(&gDefault_Material_Flat, true);
	ed3DPrepareMaterial(&gMaterial_Render_Zbuffer_Only_Cluster, true);
	ed3DPrepareMaterial(&gMaterial_Render_Zbuffer_Only, true);
	gZbuf_ScreenZMax = 0xffffffff;
	g_pStripBufResetPos = g_pStrippBufLastPos;;
	if (ged3DConfig.bEnableProfile == 1) {
		IMPLEMENTATION_GUARD(
			gIDProfileRender = edProfileNew(1, 0, 0, 0x80, s_3DRend_004333b8);
		gIDProfileFlush = edProfileNew(1, 0x80, 0, 0, s_3DFlush_004333c0);)
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
					uVar1 = (uint)((ulong)(pMATA)+(pMATA->field_0xc - (ulong)pHASH_MAT)) >> 4;
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
	ed_g2d_layer_header* pLayerSection;

	iVar2 = (TextureData_TEX_Internal*)0x0;
	int* hash = (int*)(pMAT_Internal + 1);
	pLayerSection = (ed_g2d_layer_header*)LOAD_SECTION(hash[index]);

	pcVar1 = (char*)0x0;
	if ((pLayerSection != (ed_g2d_layer_header*)0xfffffff0) && (pLayerSection->body.field_0x1c != 0)) {
		iVar2 = &((TextureData_TEX*)LOAD_SECTION(pLayerSection->body.pTex))->body;
	}
	if (iVar2 == (TextureData_TEX_Internal*)0x0) {
		pcVar1 = (char*)0x0;
	}
	else {
		TextureData_HASH_Internal_PA32* otherValue = (TextureData_HASH_Internal_PA32*)LOAD_SECTION(iVar2->hashCode.pData);
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
		if ((*(long*)(gBankMaterial + counter) == 0) || (edDlistConfig.field_0x10 - 1U <= counter)) break;
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
				uVar8 = uVar8 + pCDQU->aClusterStripCounts[uVar7 - 8];
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
	((char*)((ulong)pCDQU->aClusterStripCounts + offset + 0x30), pCDQU->field_0x0 + iVar3 + -0x10);
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
	meshInfoObj->fileLengthB = meshInfoObj->fileLengthB + (int)((ulong)g_pStrippBufLastPos - (ulong)pCommandBuffer);
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

PACK(struct MeshTransformObjData_Packed {
	int pObj; // char*
	short field_0x4;
	short field_0x6;
});

PACK(struct ed_g3d_hierarchy {
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

PACK(struct ed_Chunck {
	char field_0x0[4];
	int field_0x4;
	int field_0x8;
	int field_0xc;
	ed_g3d_hierarchy body;
});

int ed3DPrepareHierarchy(ed_g3d_hierarchy* pHIER, TextureInfo* pTextureInfo)
{
	short sVar1;
	ushort uVar2;
	int* piVar3;
	int iVar4;
	char* pBuffStart;
	ed_g3d_hierarchy* pTVar5;
	ed_Chunck* pHeader;

	if ((pHIER->flags_0x9e & 4) == 0) {
		pTVar5 = pHIER + 1;
		pHeader = (ed_Chunck*)((char*)pHIER - 0x10);
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
				pTVar5 = (ed_g3d_hierarchy*)&pTVar5->field_0x8;
			}
		if (pHIER->field_0x90 != 0) {
			//ed3DPrepareHierarchy((ed_g3d_hierarchy*)(pHIER->field_0x90 + 0x10), pTextureInfo);
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
		ed3DPrepareHierarchy((ed_g3d_hierarchy*)(((char*)LOAD_SECTION(*(int*)(pcVar3 + 8))) + 0x10), pTextureInfo);
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
				fileBuffer = (char*)edMemAlloc(TO_HEAP(H_MAIN), size_00);
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

int gLoadedClusterCount = 0;

PACK(struct MeshData_ANHR {
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

PACK(struct astruct_16 {
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
		ed_Chunck* p = (ed_Chunck*)LOAD_SECTION(*(pMVar3 + 1));
		p->body.hash.number = *pMVar3;
		pMVar3 = pMVar3 + 2;
	}
	return;
}

int g_LoadedStaticMeshCount_004493a4 = 0;

edNODE* ed3DHierarchyAddNode(edLIST* pList, ed_3d_hierarchy_node* pHierNode, edNODE* pNode, ed_g3d_hierarchy* p3DA, ed_g3d_hierarchy* p3DB)
{
	edNODE* pMVar1;
	undefined8 uVar2;
	ed_3d_hierarchy_node* pCVar8;
	float fVar3;
	float fVar4;
	int iVar5;
	edF32MATRIX4* pMVar6;
	MeshTransformObjData_Packed* pfVar7;
	ed_3d_hierarchy_node* pMVar8;
	MeshTransformObjData* pcVar9;
	edF32MATRIX4* pMVar10;
	edF32MATRIX4* pMVar11;
	edF32MATRIX4* pTVar12;
	uint uVar13;
	ed_3d_hierarchy_node* pMVar14;
	ushort* puVar15;
	edNODE* pNewNode;
	edNODE_MANAGER* pCameraPanHeader;

	MY_LOG("ed3DHierarchyAddNode %s", p3DA->hash.name);

	iVar5 = 4;
	pCameraPanHeader = (edNODE_MANAGER*)pList->pData;
	pMVar1 = (pList)->pNext;
	pCameraPanHeader->linkCount = pCameraPanHeader->linkCount + 1;
	pList->nodeCount = pList->nodeCount + 1;
	pNewNode = pCameraPanHeader->pNodeHead;
	uVar13 = (uint)((ulong)pNewNode - (ulong)pNode) >> 4;
	pCameraPanHeader->pNodeHead = pNewNode->pPrev;
	pMVar14 = pHierNode + uVar13;
	pNewNode->header.typeField.type = 0;
	pNewNode->pNext = pMVar1;
	pNewNode->pPrev = pMVar1->pPrev;
	pMVar1->pPrev->pNext = pNewNode;
	pMVar1->pPrev = pNewNode;
	g_LoadedStaticMeshCount_004493a4 = g_LoadedStaticMeshCount_004493a4 + 1;
	pTVar12 = &p3DA->transformA;
	pMVar8 = pMVar14;

	(pMVar8->base).transformA = *pTVar12;

	pMVar10 = &p3DA->transformB;
	pMVar6 = &pHierNode[uVar13].base.transformB;
	iVar5 = 4;

	pMVar11 = pMVar6;

	*pMVar11 = *pMVar10;
	pCVar8 = pHierNode + uVar13;
	puVar15 = &(pCVar8->base).flags_0x9e;
	pMVar11 = &gF32Matrix4Unit;
	iVar5 = 8;
	(pCVar8->base).hash = p3DA->hash;
	(pCVar8->base).field_0x88 = p3DA->field_0x88;
	(pCVar8->base).field_0x89 = p3DA->field_0x89;
	(pCVar8->base).bRenderShadow = p3DA->bRenderShadow;
	(pCVar8->base).pShadowAnimMatrix = (edF32MATRIX4*)LOAD_SECTION(p3DA->pShadowAnimMatrix);
	(pCVar8->base).pLinkTransformData = (ed_g3d_hierarchy*)LOAD_SECTION(p3DA->pLinkTransformData);
	(pCVar8->base).field_0x94 = (undefined*)LOAD_SECTION(p3DA->field_0x94);
	(pCVar8->base).pTextureInfo = (undefined*)LOAD_SECTION(p3DA->pTextureInfo);
	(pCVar8->base).count_0x9c = p3DA->count_0x9c;
	(pCVar8->base).flags_0x9e = p3DA->flags_0x9e;
	(pCVar8->base).pHierarchySetup = (ed_3d_hierarchy_setup*)LOAD_SECTION(p3DA->pLightingMatrixFuncObj_0xa0);
	(pCVar8->base).pMatrixPkt = (edF32MATRIX4*)LOAD_SECTION(p3DA->field_0xa4);
	(pCVar8->base).pAnimMatrix = (edF32MATRIX4*)LOAD_SECTION(p3DA->pAnimMatrix);
	(pCVar8->base).subMeshParentCount_0xac = p3DA->subMeshParentCount_0xac;
	(pCVar8->base).size_0xae = p3DA->size_0xae;
	(pCVar8->base).GlobalAlhaON = *(char*)&p3DA->field_0xaf;

	*pMVar6 = *pMVar11;

	iVar5 = 0;
	do {
		if (iVar5 < (int)(uint)p3DA->count_0x9c) {
			pfVar7 = p3DA->aSubArray + iVar5;
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
	(pCVar8->base).pLinkTransformData = p3DB;
	(pCVar8->base).field_0x94 = (undefined*)p3DB;
	pMVar8 = pHierNode + uVar13;
	if ((pCVar8->base).subMeshParentCount_0xac == 0) {
		*puVar15 = *puVar15 | 8;
	}
	(pMVar8->base).flags_0x9e = (pMVar8->base).flags_0x9e & 0xfffc;
	(pMVar8->base).flags_0x9e = (pMVar8->base).flags_0x9e | 0x800;
	ed3DSetSceneRender(pMVar8, 1);
	pNewNode->pData = pMVar8;
	return pNewNode;
}

void ed3DHierarchyAddSonsToList
(edLIST* pList, ed_3d_hierarchy_node* pHierNode, edNODE* pParentNode, ed_Chunck* pChunck, edNODE* pNewNode,
	ed_hash_code* pHashCode, uint count)
{
	char* pcVar1;
	char* pcVar2;
	char* pcVar3;
	char* pcVar4;
	ed_3d_hierarchy_setup* peVar5;
	ed_3d_hierarchy_node* peVar6;
	ed_Chunck* pChunck_00;
	edNODE* peVar7;
	edNODE* peVar8;
	edNODE* peVar9;
	edNODE* peVar10;
	edNODE* peVar11;
	edNODE* peVar12;
	edNODE* peVar13;
	edNODE* pNewNode_00;
	uint uVar14;
	edNODE* peVar15;
	ed_hash_code* peVar16;
	ed_hash_code* local_220;
	edNODE* local_204;
	uint local_200;
	ed_hash_code* local_1d0;
	edNODE* local_1b4;
	uint local_1b0;
	ed_hash_code* local_180;
	edNODE* local_164;
	uint local_160;
	ed_hash_code* local_130;
	edNODE* local_114;
	uint local_110;
	ed_hash_code* local_e0;
	edNODE* local_c4;
	uint local_c0;
	ed_hash_code* local_90;
	edNODE* local_74;
	uint local_70;
	ed_hash_code* local_40;
	edNODE* local_24;
	uint local_20;
	ed_Chunck* iVar1;

	peVar15 = (edNODE*)0x0;
	peVar16 = pHashCode;
	for (uVar14 = 0; uVar14 < count; uVar14 = uVar14 + 1) {
		iVar1 = (ed_Chunck*)LOAD_SECTION(peVar16->pData);
		if ((ed_Chunck*)(iVar1->body).pLinkTransformData == pChunck) {
			peVar7 = ed3DHierarchyAddNode
			(pList, pHierNode, pParentNode, &iVar1->body, (ed_g3d_hierarchy*)pNewNode->pData);
			local_24 = (edNODE*)0x0;
			local_40 = pHashCode;
			for (local_20 = 0; local_20 < count; local_20 = local_20 + 1) {
				pcVar1 = (char*)LOAD_SECTION(local_40->pData);
				if (*(ed_Chunck**)(pcVar1 + 0xa0) == iVar1) {
					peVar8 = ed3DHierarchyAddNode
					(pList, pHierNode, pParentNode, (ed_g3d_hierarchy*)(pcVar1 + 0x10),
						(ed_g3d_hierarchy*)peVar7->pData);
					local_74 = (edNODE*)0x0;
					local_90 = pHashCode;
					for (local_70 = 0; local_70 < count; local_70 = local_70 + 1) {
						pcVar2 = (char*)LOAD_SECTION(local_90->pData);
						if (*(char**)(pcVar2 + 0xa0) == pcVar1) {
							peVar9 = ed3DHierarchyAddNode
							(pList, pHierNode, pParentNode, (ed_g3d_hierarchy*)(pcVar2 + 0x10),
								(ed_g3d_hierarchy*)peVar8->pData);
							local_c4 = (edNODE*)0x0;
							local_e0 = pHashCode;
							for (local_c0 = 0; local_c0 < count; local_c0 = local_c0 + 1) {
								pcVar3 = (char*)LOAD_SECTION(local_e0->pData);
								if (*(char**)(pcVar3 + 0xa0) == pcVar2) {
									peVar10 = ed3DHierarchyAddNode
									(pList, pHierNode, pParentNode, (ed_g3d_hierarchy*)(pcVar3 + 0x10),
										(ed_g3d_hierarchy*)peVar9->pData);
									local_114 = (edNODE*)0x0;
									local_130 = pHashCode;
									for (local_110 = 0; local_110 < count; local_110 = local_110 + 1) {
										pcVar4 = (char*)LOAD_SECTION(local_130->pData);
										if (*(char**)(pcVar4 + 0xa0) == pcVar3) {
											peVar11 = ed3DHierarchyAddNode
											(pList, pHierNode, pParentNode, (ed_g3d_hierarchy*)(pcVar4 + 0x10),
												(ed_g3d_hierarchy*)peVar10->pData);
											local_164 = (edNODE*)0x0;
											local_180 = pHashCode;
											for (local_160 = 0; local_160 < count; local_160 = local_160 + 1) {
												peVar5 = (ed_3d_hierarchy_setup*)local_180->pData;
												if (peVar5[8].field_0x0 == pcVar4) {
													peVar12 = ed3DHierarchyAddNode
													(pList, pHierNode, pParentNode, (ed_g3d_hierarchy*)&peVar5->field_0x10,
														(ed_g3d_hierarchy*)peVar11->pData);
													local_1b4 = (edNODE*)0x0;
													local_1d0 = pHashCode;
													for (local_1b0 = 0; local_1b0 < count; local_1b0 = local_1b0 + 1) {
														peVar6 = (ed_3d_hierarchy_node*)local_1d0->pData;
														if ((peVar6->base).pHierarchySetup == peVar5) {
															peVar13 = ed3DHierarchyAddNode
															(pList, pHierNode, pParentNode,
																(ed_g3d_hierarchy*)&(peVar6->base).transformA.ba,
																(ed_g3d_hierarchy*)peVar12->pData);
															local_204 = (edNODE*)0x0;
															local_220 = pHashCode;
															for (local_200 = 0; local_200 < count; local_200 = local_200 + 1) {
																pChunck_00 = (ed_Chunck*)local_220->pData;
																if ((ed_3d_hierarchy_node*)LOAD_SECTION((pChunck_00->body).pLinkTransformData) == peVar6) {
																	pNewNode_00 = ed3DHierarchyAddNode
																	(pList, pHierNode, pParentNode, &pChunck_00->body,
																		(ed_g3d_hierarchy*)peVar13->pData);
																	ed3DHierarchyAddSonsToList
																	(pList, pHierNode, pParentNode, pChunck_00, pNewNode_00, pHashCode, count);
																	if (local_204 != (edNODE*)0x0) {
																		local_204 = pNewNode_00;
																	}
																}
																local_220 = local_220 + 1;
															}
															if (local_1b4 != (edNODE*)0x0) {
																local_1b4 = peVar13;
															}
														}
														local_1d0 = local_1d0 + 1;
													}
													if (local_164 != (edNODE*)0x0) {
														local_164 = peVar12;
													}
												}
												local_180 = local_180 + 1;
											}
											if (local_114 != (edNODE*)0x0) {
												local_114 = peVar11;
											}
										}
										local_130 = local_130 + 1;
									}
									if (local_c4 != (edNODE*)0x0) {
										local_c4 = peVar10;
									}
								}
								local_e0 = local_e0 + 1;
							}
							if (local_74 != (edNODE*)0x0) {
								local_74 = peVar9;
							}
						}
						local_90 = local_90 + 1;
					}
					if (local_24 != (edNODE*)0x0) {
						local_24 = peVar8;
					}
				}
				local_40 = local_40 + 1;
			}
			if (peVar15 != (edNODE*)0x0) {
				peVar15 = peVar7;
			}
		}
		peVar16 = peVar16 + 1;
	}
	return;
}

void ed3DHierarchyRefreshSonNumbers(edNODE* pMeshTransformParent, short* outMeshCount)
{
	ushort uVar1;
	ulong uVar2;
	ed_g3d_hierarchy* pDVar4;
	ulong uVar3;
	short local_2;
	ed_g3d_hierarchy* pDVar1;

	local_2 = 0;
	pDVar1 = (ed_g3d_hierarchy*)pMeshTransformParent->pData;
	pDVar4 = (ed_g3d_hierarchy*)pMeshTransformParent->pPrev->pData;
	if ((ed_g3d_hierarchy*)pDVar4->pLinkTransformData == pDVar1) {
		IMPLEMENTATION_GUARD();
		uVar1 = 0;
		uVar3 = (ulong)pMeshTransformParent->pPrev;
		while (uVar2 = (ulong)uVar1, (ed_g3d_hierarchy*)pDVar4->pLinkTransformData == pDVar1) {
			ed3DHierarchyRefreshSonNumbers((edNODE*)uVar3, &local_2);
			uVar1 = uVar1 + local_2;
			uVar3 = uVar2;
			pDVar4 = *(ed_g3d_hierarchy**)((ulong)uVar2 + 0xc);
		}
		pDVar1->subMeshParentCount_0xac = uVar1;
		*outMeshCount = uVar1 + 1;
	}
	else {
		pDVar1->subMeshParentCount_0xac = 0;
		*outMeshCount = 1;
	}
	return;
}

void RotationVectorToMatrix(edF32VECTOR4* v0, edF32MATRIX4* m0)
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

uint HierarchyAnm::UpdateMatrix(float param_1, ed_3d_hierarchy_node* param_3, int* pFileData, int param_5)
{
	int iVar1;
	int iVar2;
	int iVar3;
	float* pfVar4;
	float* pfVar5;
	float* pfVar6;
	float* pfVar7;
	edF32VECTOR4* pVVar8;
	float* pfVar9;
	edF32VECTOR4* pVVar10;
	float fVar11;
	float fVar12;
	float fVar13;
	float fVar14;
	float fVar15;
	float fVar16;
	float fVar17;
	float fVar18;
	uint local_50;
	edF32VECTOR4 local_30;
	edF32VECTOR4 VStack32;
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
	pVVar10 = (edF32VECTOR4*)(pfVar9 + iVar1 * 4);
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
		edF32Matrix4SetIdentityHard(&param_3->base.transformA);
	}
	else {
		pfVar7 = pfVar4;
		if (iVar2 == 1) {
			RotationVectorToMatrix(pVVar10, &param_3->base.transformA);
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
			RotationVectorToMatrix(&VStack32, &param_3->base.transformA);)
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
		_gscale_mat.aa = pVVar8->x;
		_gscale_mat.bb = pVVar8->y;
		_gscale_mat.cc = pVVar8->z;
		edF32Matrix4MulF32Matrix4Hard(&param_3->base.transformA, &_gscale_mat, &param_3->base.transformA);
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

MeshData_OBB* ed3DHierarchyNodeGetSkeletonChunck(edNODE* pMeshTransformParent, bool mode)
{
	byte bVar1;
	ed_3d_hierarchy_node* pMeshTransformData;
	int* piVar2;
	int iVar3;
	MeshTransformObjData* pMVar4;
	char* pcVar5;
	MeshData_OBB* pMVar6;
	uint uVar7;
	ushort* puVar8;

	pMeshTransformData = (ed_3d_hierarchy_node*)pMeshTransformParent->pData;
	puVar8 = &(pMeshTransformData->base).count_0x9c;
	if (((pMeshTransformData->base).count_0x9c != 0) &&
		(pMVar4 = ed3DChooseGoodLOD(pMeshTransformData), pMVar4 != (MeshTransformObjData*)0x0)) {
		bVar1 = (pMeshTransformData->base).size_0xae;
		uVar7 = (uint)bVar1;
		if ((mode != false) && (uVar7 = (uint)bVar1, ((pMeshTransformData->base).flags_0x9e & 0x100) != 0)) {
			IMPLEMENTATION_GUARD(ed_3d_hierarchy_node::Func_00295a50(pMeshTransformData, (long)(int)(*puVar8 - 1)));
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
	edLIST* pList;
	ed_3d_hierarchy_node* pHierNode;
	edNODE* pParentNode;
	uint uVar3;
	char* pcVar4;
	edNODE* pNewNode;
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
			pParentNode = gHierarchyManagerFirstFreeNode;
			pHierNode = gHierarchyManagerBuffer;
			pcVar2 = (MeshData_HALL*)pMeshInfo->HALL;
			pList = pStaticMeshMaster->pHeirListA;
			uVar3 = edChunckGetNb((char*)(pcVar2 + 1), (char*)pcVar2 + pcVar2->totalSize);
			pcVar4 = edHashcodeGet(meshHashValue, (MeshData_HASH*)(pMeshInfo->HALL + 0x10));
			if (pcVar4 == (char*)0x0) {
				iVar5 = 0;
			}
			else {
				iVar5 = *(int*)(pcVar4 + 8);
			}
			if (iVar5 == 0) {
				pNewNode = (edNODE*)0x0;
			}
			else {
				pNewNode = ed3DHierarchyAddNode(pList, pHierNode, pParentNode, &((ed_Chunck*)LOAD_SECTION(iVar5))->body, (ed_g3d_hierarchy*)0x0);
				ed3DHierarchyAddSonsToList(pList, pHierNode, pParentNode, (ed_Chunck*)LOAD_SECTION(iVar5), pNewNode, (ed_hash_code*)(pcVar2 + 2), (uVar3 & 0xffff) - 1);
				ed3DHierarchyRefreshSonNumbers(pNewNode, &sStack2);
			}
			pMVar14->field_0x0 = (int)STORE_SECTION(pNewNode);
			if (pNewNode == (edNODE*)0x0) {
				pMVar14->field_0x4 = 0x0;
				pMVar14->field_0x8 = 0x0;
			}
			else {
				pMVar14->field_0x4 = (int)STORE_SECTION(pNewNode->pData);
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
					IMPLEMENTATION_GUARD(ed3DHierarchySetSetup((ed_3d_hierarchy_node*)LOAD_SECTION(pMVar14->field_0x4), 0x40f070));
				}
				puVar10 = (uint*)LOAD_SECTION(pcVar8[(int)pMVar14->field_0x8]);
				UpdateMatrix(pMVar14->field_0x28, (ed_3d_hierarchy_node*)LOAD_SECTION(pMVar14->field_0x4), (int*)puVar10,
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
				pMVar6 = ed3DHierarchyNodeGetSkeletonChunck(pNewNode, 0);
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
	edNODE* peVar1;
	edCluster* pNewCluster;
	uint clusterIndex;
	edNODE_MANAGER* pvVar1;

	clusterIndex = 0;
	while ((clusterIndex < ged3DConfig.maxClusterCount && ((gCluster[clusterIndex].flags & 1) != 0))) {
		clusterIndex = clusterIndex + 1;
	}
	if (clusterIndex < ged3DConfig.maxClusterCount) {
		pNewCluster = gCluster + clusterIndex;
		gLoadedClusterCount = gLoadedClusterCount + 1;
		pNewCluster->pMeshInfo = pMeshInfo;
		gCluster[clusterIndex].flags = gCluster[clusterIndex].flags | 1;
		pvVar1 = (edNODE_MANAGER*)(pStaticMeshMaster->headerB).pData;
		peVar1 = pvVar1->pNodeHead;
		pvVar1->pNodeHead = peVar1->pPrev;
		ED3D_LOG(LogLevel::Info, "ed3DScenePushCluster %p New cluster: %p", peVar1, pNewCluster);
		peVar1->pData = pNewCluster;
		pvVar1->linkCount = pvVar1->linkCount + 1;
		(pStaticMeshMaster->headerB).nodeCount = (pStaticMeshMaster->headerB).nodeCount + 1;
		peVar1->pPrev = (pStaticMeshMaster->headerB).pPrev;
		(pStaticMeshMaster->headerB).pPrev = peVar1;
	}
	return;
}

edNODE* ed3DHierarchyAddToSceneByHashcode(ed_3D_Scene* pStaticMeshMaster, ed_g3d_manager* pMeshInfo, ulong hash)
{
	edLIST* pList;
	ed_3d_hierarchy_node* pHierNode;
	edNODE* pNode;
	uint uVar1;
	char* pcVar2;
	edNODE* pNewNode;
	ed_Chunck* pChunck;
	short sStack2;
	MeshData_HALL* pcVar1;

	ed3DHierarchyCopyHashCode(pMeshInfo);
	pNode = gHierarchyManagerFirstFreeNode;
	pHierNode = gHierarchyManagerBuffer;
	pcVar1 = (MeshData_HALL*)pMeshInfo->HALL;
	pList = pStaticMeshMaster->pHeirListA;
	uVar1 = edChunckGetNb((char*)(pcVar1 + 1), pcVar1->header + pcVar1->totalSize);
	pcVar2 = edHashcodeGet(hash, (MeshData_HASH*)(pMeshInfo->HALL + 0x10));
	pChunck = (ed_Chunck*)0x0;
	if (pcVar2 != (char*)0x0) {
		pChunck = *(ed_Chunck**)(pcVar2 + 8);
	}
	if (pChunck == (ed_Chunck*)0x0) {
		pNewNode = (edNODE*)0x0;
	}
	else {
		pNewNode = ed3DHierarchyAddNode(pList, pHierNode, pNode, &pChunck->body, (ed_g3d_hierarchy*)0x0);
		ed3DHierarchyAddSonsToList(pList, pHierNode, pNode, pChunck, pNewNode, (ed_hash_code*)(pcVar1 + 2), (uVar1 & 0xffff) - 1);
		ed3DHierarchyRefreshSonNumbers(pNewNode, &sStack2);
	}
	return pNewNode;
}

void ed3DSceneComputeCameraToScreenMatrix(ed_3D_Scene* pScene, edF32MATRIX4* m0)
{
	short sVar1;
	uint uVar2;
	edFCamera* peVar3;
	float fVar4;
	float fVar5;
	float fVar6;
	float fVar7;
	float fVar8;
	float fVar9;
	float endY;
	float fVar10;
	edF32MATRIX4 eStack128;
	edF32MATRIX4 eStack64;

	uVar2 = (pScene->sceneConfig).field_0x10;
	if ((int)uVar2 < 0) {
		fVar6 = (float)(uVar2 >> 1 | uVar2 & 1);
		fVar6 = fVar6 + fVar6;
	}
	else {
		fVar6 = (float)uVar2;
	}
	uVar2 = (pScene->sceneConfig).field_0x14;
	if ((int)uVar2 < 0) {
		fVar5 = (float)(uVar2 >> 1 | uVar2 & 1);
		fVar5 = fVar5 + fVar5;
	}
	else {
		fVar5 = (float)uVar2;
	}
	peVar3 = pScene->pCamera;
	fVar8 = (pScene->sceneConfig).farClip;
	fVar7 = (pScene->sceneConfig).nearClip;
	endY = peVar3->halfFOV;
	fVar9 = peVar3->horizontalHalfFOV;
	sVar1 = pScene->pViewport->screenHeight;
	fVar10 = (float)(int)pScene->pViewport->screenWidth;
	ed3DComputeLocalToProjectionMatrix(0.0f, 0.0f, peVar3->verticalHalfFOV, 0.0f, &eStack64);
	fVar4 = (float)(int)sVar1 * 0.5;
	ed3DComputeProjectionToScreenMatrix
	(-fVar9, fVar9, -endY, endY, 2048.0f - fVar10 * 0.5f, fVar10 * 0.5f + 2048.0f, fVar4 + 2048.0f, 2048.0f - fVar4, &eStack128);
	edF32Matrix4MulF32Matrix4Hard(m0, &eStack64, &eStack128);
	fVar4 = 1.0f / (fVar8 - fVar7);
	fVar9 = m0->dd + m0->cd * fVar7;
	m0->cc = fVar4 * (fVar6 * (m0->dd + m0->cd * fVar8) - fVar5 * fVar9);
	m0->dc = fVar4 * (fVar9 * fVar8 * fVar5 - fVar9 * fVar7 * fVar6);
	return;
}

void ed3DResetTime(void)
{
	gCurTime = 0;
	return;
}

void ed3DSetDeltaTime(int newTime)
{
	gStepTime = newTime;
	gCurTime = gCurTime + newTime;
	return;
}

FxFogProp g3DFXFog = { };

FxFogProp* ed3DGetFxFogProp(void)
{
	return &g3DFXFog;
}
