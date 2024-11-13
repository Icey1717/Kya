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
#include "profiling.h"
#endif

#include "edVideo/VideoD.h"
#include "port/pointer_conv.h"
#include "edVideo/VideoB.h"
#include "edVideo/VideoC.h"
#include "Rendering/DisplayList.h"
#include "edVideo/Viewport.h"

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
#include "AnmManager.h"
#include "ActorHero.h"

#ifdef PLATFORM_WIN
#include "Texture.h"
#include "Mesh.h"
#endif

#define ED_3D_HIDDEN_FLAG 0x40

#define ED3D_LOG(level, format, ...) MY_LOG_CATEGORY("ed3D", level, format, ##__VA_ARGS__)

char* s_ed3D_Initialsation_004333a0 = "ed3D Initialsation\n";

#ifdef PLATFORM_PS2
#define RENDER_LABEL_BEGIN(...)
#define RENDER_LABEL_END(...)
#else
#define RENDER_LABEL_BEGIN(label, ...) Renderer::Debug::BeginLabel(label, ##__VA_ARGS__)
#define RENDER_LABEL_END(label) Renderer::Debug::EndLabel()
#endif

#ifdef PLATFORM_WIN
#define PKT_TO_SCRATCHPAD(pkt) pkt
#else
#define PKT_TO_SCRATCHPAD(pkt) (void*)((uint)pkt & 0xfffffff)
#endif

#define PKT_SIZE(end, begin) (ulong)end - (ulong)begin

#define HASH_CODE_MATA 0x4154414d
#define HASH_CODE_2D 0x2a44322a

#define PRIM_LIST_0 0x0
#define PRIM_LIST_1 0x1
#define PRIM_LIST_2 0x2
#define PRIM_LIST_3 0x3
#define PRIM_LIST_4 0x4
#define PRIM_LIST_5 0x5
#define PRIM_LIST_6 0x6
#define PRIM_LIST_7 0x7
#define PRIM_LIST_8 0x8
#define PRIM_LIST_9 0x9
#define PRIM_LIST_A 0xa
#define PRIM_LIST_B 0xb
#define PRIM_LIST_C 0xc
#define PRIM_LIST_D 0xd
#define PRIM_LIST_E 0xe
#define PRIM_LIST_F 0xf

#define LIST_TYPE_MATERIAL	1
#define LIST_TYPE_MATRIX	2
#define LIST_TYPE_STRIP		3
#define LIST_TYPE_SPRITE	4

// Debug
#ifdef PLATFORM_WIN
namespace ed3D {
	namespace DebugOptions {
		bool gForceHighestLod = false;
		bool& GetForceHighestLod()
		{
			return gForceHighestLod;
		}

		bool gDisableClusterRendering = false;

		bool& GetDisableClusterRendering() {
			return gDisableClusterRendering;
		}
	}
}
#endif

int gCurTime = 0;
int gStepTime = 0;
byte BYTE_00448a5c = 1;

FxFogProp g3DFXFog = { };

edF32MATRIX4 CHierarchyAnm::_gscale_mat = {
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

ed_3d_hierarchy* gpCurHierarchy = NULL;
undefined* pOldFANGifTag = NULL;

ed_3d_hierarchy_node* gHierarchyManagerBuffer = NULL;
edNODE_MANAGER* gHierarchyManagerNodeManager = NULL;
edNODE* gHierarchyManagerFirstFreeNode = NULL;

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

struct ed_3d_extra_stuff_param {
	struct ed_3D_Scene* pStaticMeshMaster;
	int isChild;
	ushort taskID;
};

ed_Chunck* edChunckGetFirst(void* pBuffStart, char* pBuffEnd)
{
	char* pStart = reinterpret_cast<char*>(pBuffStart);
	/* Checks that the end of the file is greater than the start of the file */
	if ((pBuffEnd != (char*)0x0) && (pBuffEnd <= pStart)) {
		pStart = (char*)0x0;
	}
	return (ed_Chunck*)pStart;
}

ed_Chunck* edChunckGetNext(ed_Chunck* pCurChunck, char* pBuffEnd)
{
	ed_Chunck* pNextChunck;

	if ((pBuffEnd == (char*)0x0) || (pNextChunck = (ed_Chunck*)0x0, ((char*)pCurChunck + pCurChunck->size) < pBuffEnd)) {
		assert(pCurChunck->size > 0);
		pNextChunck = (ed_Chunck*)((char*)pCurChunck + pCurChunck->size);
	}
	return pNextChunck;
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

edpkt_data g_stMatrixHeader;
edpkt_data g_stVertexGIFHeader;
edpkt_data g_stVertexSTHeader[2];

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

#ifdef PLATFORM_WIN
// New port only
edpkt_data* g_pStartPortPktData = NULL;
#endif

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

ed_3d_hierarchy ed_3d_hierarchy_0048cad0 = { 0 };

void ed3DPrimlistMatrixBufferReset(void)
{
	DmaMatrixDefault = DmaMatrixBuffer;
	ed_3d_hierarchy_0048cad0.pMatrixPkt = (edpkt_data*)0x0;
	DmaMatrixBufferCurrentMax = DmaMatrixBufferMax;
	gNodeDmaMatrix = pNodeManRender;
	gNodeDmaStrip = pNodeManRender;
	DmaMatrixBufferCurrent = DmaMatrixBuffer + 1;
	DmaMatrixBuffer->pObjToWorld = &gF32Matrix4Unit;
	DmaMatrixDefault->pHierarchy = &ed_3d_hierarchy_0048cad0;
	DmaMatrixDefault->normalScale = 1.0;
	(DmaMatrixDefault)->pNext = (edNODE*)DmaMatrixDefault;
	(DmaMatrixDefault)->pPrev = (edNODE*)DmaMatrixDefault;
	(DmaMatrixDefault)->pData = gNodeDmaStrip;
	(DmaMatrixDefault)->field_0x10 = 0;
	(DmaMatrixDefault)->nodeCount = 0;
	(DmaMatrixDefault)->header.mode = 0;
	DmaMatrixDefault->pHierarchy = &ed_3d_hierarchy_0048cad0;
	return;
}

void ed3DG2DMaterialGetLayerBitmap(ed_dma_material* pMaterial, ed_g2d_bitmap** ppBitmap, ed_g2d_layer** ppLayer, int index)
{
	ed_Chunck* pLAY = LOAD_SECTION_CAST(ed_Chunck*, pMaterial->pMaterial->aLayers[index]);
	*ppLayer = reinterpret_cast<ed_g2d_layer*>(pLAY + 1);

	ed_Chunck* pTEX = LOAD_SECTION_CAST(ed_Chunck*, (*ppLayer)->pTex);
	ed_g2d_texture* pTexture = reinterpret_cast<ed_g2d_texture*>(pTEX + 1);
	ed_hash_code* pTextureHashCode = LOAD_SECTION_CAST(ed_hash_code*, pTexture->hashCode.pData);

	ed_Chunck* pT2D = LOAD_SECTION_CAST(ed_Chunck*, pTextureHashCode->pData);
	*ppBitmap = reinterpret_cast<ed_g2d_bitmap*>(pT2D + 1);

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
	if (pMVar1->pMaterial->nbLayers < 2) {
		peVar1 = pMVar1->pBitmap;
		if ((peVar1 != (ed_g2d_bitmap*)0x0) && (peVar1->psm != 0)) {
			edPSX2Header* pHeader = ((edPSX2Header*)LOAD_SECTION(peVar1->pPSX2)) + gVRAMBufferDMA;
			pPkt = (edpkt_data*)LOAD_SECTION(pHeader->pPkt);
			edDmaFlushCache();
			ED3D_LOG(LogLevel::VeryVerbose, "DMA Begin ed3DFlushTexReset");
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
		ED3D_LOG(LogLevel::VeryVerbose, "DMA Begin ed3DFlushTexPrepareDMA");
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
		ED3D_LOG(LogLevel::VeryVerbose, "DMA Begin ed3DFlushSendDMA3D");
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
	g_PKTHeaderRef[0].cmdA = ED_VIF1_SET_TAG_REF(1, STORE_SECTION(&g_stVertexGIFHeader));
	g_PKTHeaderRef[0].cmdB = 0;
	g_PKTHeaderRef[1].cmdA = ED_VIF1_SET_TAG_REF(2, STORE_SECTION(g_stVertexSTHeader));
	g_PKTHeaderRef[1].cmdB = 0;
	g_PKTHeaderRef[2].cmdA = ED_VIF1_SET_TAG_REF(1, STORE_SECTION(&g_stVertexRGBAHeader));
	g_PKTHeaderRef[2].cmdB = 0;
	g_PKTHeaderRef[3].cmdA = ED_VIF1_SET_TAG_REF(1, STORE_SECTION(&g_stVertexXYZHeader));
	g_PKTHeaderRef[3].cmdB = 0;
	g_PKTHeaderRef[4].cmdA = ED_VIF1_SET_TAG_REF(1, STORE_SECTION(&g_stNormalHeader));
	g_PKTHeaderRef[4].cmdB = 0;
	g_PKTHeaderRef[5].cmdA = ED_VIF1_SET_TAG_REF(1, STORE_SECTION(&g_stVertexOptionFlagHeader));
	g_PKTHeaderRef[5].cmdB = 0;
	g_PKTHeaderRef[6].cmdA = ED_VIF1_SET_TAG_REF(1, STORE_SECTION(&g_stVertexMultiSTHeader));
	g_PKTHeaderRef[6].cmdB = 0;
	g_PKTHeaderRef[7].cmdA = ED_VIF1_SET_TAG_REF(1, STORE_SECTION(&g_stVertexXYZKeepADCHeader));
	g_PKTHeaderRef[7].cmdB = 0;
	return;
}

edpkt_data* g_stGifReferencesPacket = NULL;
edpkt_data* g_GifRefPktCurMax = NULL;
char* g_ScratchBase_004493d0 = NULL;
edpkt_data* gPKTMatrixCur = NULL;

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
	gPKTMatrixCur = (edpkt_data*)g_ScratchBase_004493d0;
	//PTR_DAT_004493f0 = (undefined*)((int)g_ScratchBase_004493d0 + Params_00424e30.meshHeaderCountA * 7 * 0x40);
	return;
}

edpkt_data g_PKTShapeHeaderRef[3] = { 0 };
edpkt_data g_PKTSpriteHeaderRef[5] = { 0 };

edpkt_data* gPKTMultiTex = NULL;

void ed3DDMAGenerateSpritePacketRefHeader(void)
{
	g_PKTSpriteHeaderRef[0].cmdA = ED_VIF1_SET_TAG_REF(1, STORE_SECTION(&g_stSpriteVertexGIFHeader));
	g_PKTSpriteHeaderRef[0].cmdB = 0;
	g_PKTSpriteHeaderRef[1].cmdA = ED_VIF1_SET_TAG_REF(2, STORE_SECTION(g_stSpriteVertexSTHeader));
	g_PKTSpriteHeaderRef[1].cmdB = 0;
	g_PKTSpriteHeaderRef[2].cmdA = ED_VIF1_SET_TAG_REF(1, STORE_SECTION(&g_stSpriteVertexRGBAHeader));
	g_PKTSpriteHeaderRef[2].cmdB = 0;
	g_PKTSpriteHeaderRef[3].cmdA = ED_VIF1_SET_TAG_REF(1, STORE_SECTION(&g_stSpriteVertexXYZHeader));
	g_PKTSpriteHeaderRef[3].cmdB = 0;
	g_PKTSpriteHeaderRef[4].cmdA = ED_VIF1_SET_TAG_REF(2, STORE_SECTION(g_stSpriteWidthHeightHeader));
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
	gFANbuffers.asU32[1] = 0x3e;
	gFANbuffers.asU32[2] = 0x5a;
	lVar3 = (ulong)ed3D_Allocator_00449248.end;
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
	g_PKTShapeHeaderRef[0].cmdA = ED_VIF1_SET_TAG_REF(1, STORE_SECTION(&g_stShapeVertexXYZHeader));
	g_PKTShapeHeaderRef[1].cmdA = ED_VIF1_SET_TAG_REF(2, STORE_SECTION(g_stShapeVertexSTHeader));
	g_PKTShapeHeaderRef[2].cmdA = ED_VIF1_SET_TAG_REF(1, STORE_SECTION(&g_stShapeVertexRGBAHeader));
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
	g_stGifReferencesPacket = (edpkt_data*)(((ulong)(lVar3 << 0x24) >> 0x24) | 0x30000000);
#else
	g_stGifReferencesPacket = (edpkt_data*)malloc(countB * 2 * sizeof(edpkt_data));
#endif
	g_GifRefPktCur = g_stGifReferencesPacket;
	g_GifRefPktCurMax = g_stGifReferencesPacket + countB;
	return;
}

ed_g2d_material* previous_flush3DMat = NULL;
ed_dma_material* gpLastMaterial = NULL;
int gVRAMBufferFlush = 0;
byte gbFirstTex = 0;

void ed3DPKTMatrixBufferFlip(void)
{
	gPKTMatrixCur = (edpkt_data*)g_ScratchBase_004493d0;
	if (gCurRenderList != 0) {
		gPKTMatrixCur = (edpkt_data*)(g_ScratchBase_004493d0 + ged3DConfig.matrixBufferCount * 0x1c0);
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
	gpCurHierarchy = (ed_3d_hierarchy*)0x0;
	gpLastMaterial = (ed_dma_material*)0x0;
	if (gCurRenderList == 0) {
		GifRefPktCurMax = ged3DConfig.field_0x28 * 0x10;
		g_GifRefPktCur = g_stGifReferencesPacket;
		g_VifRefPktCur = g_stVifReferencesPacket;
		uVar1 = g_VideoMemorySize_00449280 >> 1;
	}
	else {
		g_VifRefPktCur = g_stVifReferencesPacket + (g_VideoMemorySize_00449280 >> 1);
		GifRefPktCurMax = ged3DConfig.field_0x28 * 0x20;
		g_GifRefPktCur = g_stGifReferencesPacket + ged3DConfig.field_0x28;
		uVar1 = g_VideoMemorySize_00449280;
	}
	g_GifRefPktCurMax = (edpkt_data*)((ulong)&g_stGifReferencesPacket->cmdA + GifRefPktCurMax);
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
	ed_g2d_material* pMaterial;
	ed_g2d_bitmap* pBitmap;

	pMaterial = pDMA_Material->pMaterial;
	if ((pMaterial == (ed_g2d_material*)0x0) || (pMaterial->nbLayers == 0)) {
		pBitmap = (ed_g2d_bitmap*)0x0;
	}
	else {
		IMPLEMENTATION_GUARD(pBitmap = ed3DGetG2DBitmap(pMaterial, index));
	}

	return pBitmap;
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

	if ((mode != false) && ((param_1->base).linkedHierCount != 0)) {
		return;
	}

	if ((param_1->base).bSceneRender == 0) {
		return;
	}

	v1 = (ed_3d_hierarchy_node*)(param_1->base).pLinkTransformData;
	if (v1 == (ed_3d_hierarchy_node*)0x0) {
		if (((param_1->base).flags_0x9e & 1) == 0) {
			edF32Matrix4CopyHard(&(param_1->base).transformB, &param_1->base.transformA);
		}
		goto LAB_002957c4;
	}

	if ((v1->base).bSceneRender != 0) {
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
				(v1->base).bSceneRender = 0;
				goto LAB_00295788;
			}
		}
		(v1->base).bSceneRender = 0;
	}
LAB_00295788:
	if (((param_1->base).flags_0x9e & 1) == 0) {
		edF32Matrix4MulF32Matrix4Hard(&(param_1->base).transformB, &param_1->base.transformA, &(v1->base).transformB);
		(param_1->base).bSceneRender = 0;
		return;
	}
LAB_002957c4:
	(param_1->base).bSceneRender = 0;
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
		if (((peVar6->flags & SCENE_FLAG_IN_USE) != 0) && ((peVar6->flags & 4) == 0)) {
			peVar1 = peVar6->pHierListA;
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

		// Transfer the program in blocks of 256. Each block starts with a vifCode_MPG command.
		edDmaSync(SHELLDMA_CHANNEL_VIF1);
		ED3D_LOG(LogLevel::VeryVerbose, "DMA Begin ed3DDMALoadVU1MicroCode");
		edDmaSend(SHELLDMA_CHANNEL_VIF1, (uint)vu1_micro);

#ifdef PLATFORM_WIN
		VU1Emu::SendVu1Code(vu1_micro, sizeof(vu1_micro));
#endif

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
	pShadowConfig->texWidth = 0x80;
	pShadowConfig->texHeight = 0x80;
	return;
}

edF32VECTOR4 LightAmbiant_DEBUG_DATA = { 10.0f, 10.0f, 10.0f, 128.0f };

edF32MATRIX4 LightColor_Matrix_DEBUG_DATA = { 
	64.0f,	0.0f,	0.0f,	128.0f,
	0.0f,	64.0f,	0.0f,	128.0f,
	0.0f,	0.0f,	64.0f,	128.0f,
	128.0f,	128.0f,	128.0f,	128.0f
};

edF32MATRIX4 LightDirections_Matrix_DEBUG_DATA = { 
	1.0f,	0.0f,	0.0f,	0.0f,
	0.0f,	-2.0f,	0.0f,	0.0f,
	-2.0f,	0.0f,	0.0f,	0.0f,
	0.0f,	0.0f,	-2.0f,	0.0f
};

edF32VECTOR4* LightAmbiant_DEBUG = &LightAmbiant_DEBUG_DATA;
edF32MATRIX4* LightDirections_Matrix_DEBUG = &LightDirections_Matrix_DEBUG_DATA;
edF32MATRIX4* LightColor_Matrix_DEBUG = &LightColor_Matrix_DEBUG_DATA;

void ed3DSceneLightConfigSetDefault(ed_3D_Light_Config* pLightConfig)
{
	pLightConfig->pLightAmbient = LightAmbiant_DEBUG;
	pLightConfig->pLightDirections = LightDirections_Matrix_DEBUG;
	pLightConfig->pLightColorMatrix = LightColor_Matrix_DEBUG;
	return;
}

void ed3DSceneConfigSetDefault(SceneConfig* pSceneConfig)
{
	pSceneConfig->clipValue_0x4 = 250.0f;
	pSceneConfig->clipValue_0x18 = 250.0f;
	pSceneConfig->nearClip = g_DefaultNearClip_0044851c;
	pSceneConfig->farClip = -5000.0f;
	pSceneConfig->projectionScaleFactorA = 1;
	(pSceneConfig->field_0xe0).x = 0.0f;
	(pSceneConfig->field_0xe0).y = 255.0f;
	(pSceneConfig->field_0xe0).z = g_DefaultNearClip_0044851c;
	(pSceneConfig->field_0xe0).w = -3200.0f;
	(pSceneConfig->fogCol_0xf0).r = 0x3e;
	(pSceneConfig->fogCol_0xf0).g = 0x47;
	(pSceneConfig->fogCol_0xf0).b = 0x67;
	ed3DShadowConfigSetDefault(&pSceneConfig->pShadowConfig);
	ed3DSceneLightConfigSetDefault(&pSceneConfig->pLightConfig);
	return;
}

ed_3D_Scene* ed3DSceneCreate(edFCamera* pCamera, ed_viewport* pViewport, int bInitHierList)
{
	ushort zBufferPixelStoreMode;
	edSurface* pZBufferSurface;
	ed_3D_Scene* pScene;
	ed_3D_Scene* pFreeScene;
	uint sceneIndex;

	sceneIndex = 0;
	/* Find inactive camera master. */
	while ((sceneIndex < ged3DConfig.sceneCount && ((gScene3D[sceneIndex].flags & SCENE_FLAG_IN_USE) != 0))) {
		sceneIndex = sceneIndex + 1;
	}

	pScene = (ed_3D_Scene*)0xffffffff;
	if (sceneIndex < ged3DConfig.sceneCount) {
		pScene = gScene3D + sceneIndex;
		pScene->flags = pScene->flags | SCENE_FLAG_IN_USE;
		gScene3D[sceneIndex].bShadowScene = 0;
		gScene3D[sceneIndex].pCamera = pCamera;
		gScene3D[sceneIndex].pViewport = pViewport;
		if (gDebugViewport == (ed_viewport*)0x0) {
			gDebugViewport = pViewport;
		}

		ed3DSceneConfigSetDefault(&gScene3D[sceneIndex].sceneConfig);

		pFreeScene = gScene3D + sceneIndex;
		pZBufferSurface = pFreeScene->pViewport->pZBuffer;
		if (pZBufferSurface == (edSurface*)0x0) {
			pFreeScene->sceneConfig.projectionScaleFactorB = 0x00ffffef;
		}
		else {
			zBufferPixelStoreMode = pZBufferSurface->pSurfaceDesc->pixelStoreMode;
			if (zBufferPixelStoreMode == SCE_GS_PSMCT16) {
				pFreeScene->sceneConfig.projectionScaleFactorB = 0x0000fffe;
			}
			else {
				if (zBufferPixelStoreMode == SCE_GS_PSMCT24) {
					pFreeScene->sceneConfig.projectionScaleFactorB = 0x00ffffef;
				}
				else {
					if (zBufferPixelStoreMode == SCE_GS_PSMCT32) {
						pFreeScene->sceneConfig.projectionScaleFactorB = 0x00ffffef;
					}
				}
			}
		}

		if (bInitHierList != 0) {
			gScene3D[sceneIndex].pHierListA = ed3DHierarchyListInit();
			gScene3D[sceneIndex].pHierListB = ed3DHierarchyListInit();

#ifdef EDITOR_BUILD
			char name[128];
			sprintf(name, "gScene3D[%d].pHeirListA", sceneIndex);
			edListSetName(gScene3D[sceneIndex].pHierListA, name);
			sprintf(name, "gScene3D[%d].pHeirListB", sceneIndex);
			edListSetName(gScene3D[sceneIndex].pHierListB, name);
#endif
		}
	}

	return pScene;
}


void ed3DFlushMatrixReset(void)
{
	gpCurHierarchy = (ed_3d_hierarchy*)0xefefefef;
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

edF32MATRIX4 gCurLOD_WorldToCamera_Matrix = { 0 };

edF32VECTOR4* gLightAmbiant = NULL;
edF32MATRIX4* gLightDirections_Matrix = NULL;
edF32MATRIX4* gLightColor_Matrix = NULL;

void ed3DSceneLightSetGlobals(ed_3D_Light_Config* pConfig)
{
	gLightAmbiant = pConfig->pLightAmbient;

	if (gLightAmbiant != (edF32VECTOR4*)0x0) {
		gLightAmbiant->w = 0.0078125;
	}

	gLightDirections_Matrix = pConfig->pLightDirections;
	gLightColor_Matrix = pConfig->pLightColorMatrix;
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

#ifdef PLATFORM_WIN
edF32MATRIX4 gNativeProjectionMatrix = { 0 };

static edF32MATRIX4 CalculateOpenGlPerspectiveMatrix(const float fovy, const float aspect, const float n, const float f)
{
	const float e = 1.0f / std::tan(fovy * 0.5f);
	return { e / aspect, 0.0f,  0.0f,                   0.0f,
			0.0f,       e,     0.0f,                    0.0f,
			0.0f,       0.0f, (f + n) / (n - f),       -1.0f,
			0.0f,       0.0f, (2.0f * f * n) / (n - f), 0.0f };
}

static edF32MATRIX4 CalculateVulkanPerspectiveMatrix(const float fovy, const float aspect, const float n, const float f)
{
	constexpr edF32MATRIX4 vulkanClip{	1.0f,  0.0f, 0.0f, 0.0f,
										0.0f, -1.0f, 0.0f, 0.0f,
										0.0f,  0.0f, 0.5f, 0.0f,
										0.0f,  0.0f, 0.5f, 1.0f };
	return vulkanClip * CalculateOpenGlPerspectiveMatrix(fovy, aspect, n, f);
}

static edF32MATRIX4 ReverseZ(const edF32MATRIX4& perspectiveProjection)
{
	constexpr edF32MATRIX4 reverseZ{ 1.0f, 0.0f,  0.0f, 0.0f,
							  0.0f, 1.0f,  0.0f, 0.0f,
							  0.0f, 0.0f, -1.0f, 0.0f,
							  0.0f, 0.0f,  1.0f, 1.0f };
	return reverseZ * perspectiveProjection;
}

static void ComputeReverseProjectionMatrix(const float nearClip, const float farClip, const float fov, const float aspect)
{
	gNativeProjectionMatrix = ReverseZ(CalculateVulkanPerspectiveMatrix(fov, aspect, -nearClip, -farClip));
}

static void ComputeReverseProjectionMatrix(const float fov, const float nearClip, const float farClip)
{
	//const float fov = 48.0f * (3.1415f / 180.0f); // Convert to radians
	const float aspect = 4.0f / 3.0f;

	ComputeReverseProjectionMatrix(nearClip, farClip, fov, aspect);
}
#endif

void ed3DViewportComputeViewMatrices(float screenWidth, float screenHeight, float horizontalHalfFOV, float halfFOV, float verticalHalfFOV, float targetWidth, float targetHeight,
	float nearClip, edF32MATRIX4* pCameraToScreen, edF32MATRIX4* pCameraToCulling, edF32MATRIX4* pCameraToClipping, edF32MATRIX4* pCameraToFog,
	float farClip, float projectionScaleFactorA, float projectionScaleFactorB)
{
	float fVar1;
	float fVar2;
	float fVar3;
	float negHorizontalHalfFOV;
	float fVar4;
	float negHalfFOV;
	edF32MATRIX4 screenMatrix;
	edF32MATRIX4 projectionMatrix;

#ifdef PLATFORM_WIN
	const float fov = (halfFOV / verticalHalfFOV) * 1.333333f;
	ComputeReverseProjectionMatrix(fov, nearClip, farClip);
#endif

	ED3D_LOG(LogLevel::Verbose, "ed3DViewportComputeViewMatrices screenWidth: {}, screenHeight: {}, horizontalHalfFOV: {}, halfFOV: {}, horizontalHalfFOV: {}, targetWidth: {}, targetHeight: {}, nearClip: {}, farClip: {}, projectionScaleFactorA: {}, projectionScaleFactorB: {}\n", 
		screenWidth, screenHeight, horizontalHalfFOV, halfFOV, verticalHalfFOV, targetWidth, targetHeight, nearClip, farClip, projectionScaleFactorA, projectionScaleFactorB);

	ed3DComputeLocalToProjectionMatrix(0.0f, 0.0f, verticalHalfFOV, 0.0f, &projectionMatrix);
	negHalfFOV = -halfFOV;
	negHorizontalHalfFOV = -horizontalHalfFOV;

	ed3DComputeProjectionToScreenMatrix(negHorizontalHalfFOV, horizontalHalfFOV, negHalfFOV, halfFOV, targetWidth - screenWidth * 0.5f, targetWidth + screenWidth * 0.5f,
		targetHeight + screenHeight * 0.5f, targetHeight - screenHeight * 0.5f, &screenMatrix);

	edF32Matrix4MulF32Matrix4Hard(pCameraToScreen, &projectionMatrix, &screenMatrix);
	fVar4 = 1.0f / (farClip - nearClip);
	fVar1 = pCameraToScreen->dd + pCameraToScreen->cd * nearClip;
	pCameraToScreen->cc = fVar4 * (projectionScaleFactorB * (pCameraToScreen->dd + pCameraToScreen->cd * farClip) - projectionScaleFactorA * fVar1);
	pCameraToScreen->dc = fVar4 * (fVar1 * farClip * projectionScaleFactorA - fVar1 * nearClip * projectionScaleFactorB);

	ed3DComputeProjectionToScreenMatrix(negHorizontalHalfFOV * 1.0f, horizontalHalfFOV * 1.0f, negHalfFOV * 1.0f, halfFOV * 1.0f, -1.0f, 1.0, -1.0f, 1.0f, &screenMatrix);

	edF32Matrix4MulF32Matrix4Hard(pCameraToCulling, &projectionMatrix, &screenMatrix);
	fVar2 = nearClip * 1.0f;
	fVar1 = pCameraToCulling->dd + pCameraToCulling->cd * nearClip;
	fVar3 = farClip * -1.0f;
	pCameraToCulling->cc = fVar4 * ((pCameraToCulling->dd + pCameraToCulling->cd * farClip) * 1.0f - fVar1 * -1.0f);
	pCameraToCulling->dc = fVar4 * (fVar1 * fVar3 - fVar1 * fVar2);

	ed3DComputeProjectionToScreenMatrix(negHorizontalHalfFOV * fSecInc, horizontalHalfFOV * fSecInc, negHalfFOV * fSecInc, halfFOV * fSecInc, -1.0f, 1.0f, -1.0f, 1.0f, &screenMatrix);

	edF32Matrix4MulF32Matrix4Hard(pCameraToClipping, &projectionMatrix, &screenMatrix);

	fVar1 = pCameraToClipping->dd + pCameraToClipping->cd * nearClip;
	pCameraToClipping->cc = fVar4 * ((pCameraToClipping->dd + pCameraToClipping->cd * farClip) * 1.0f - fVar1 * -1.0f);
	pCameraToClipping->dc = fVar4 * (fVar1 * fVar3 - fVar1 * fVar2);

	gClipMulVector.z = 2.0f / (projectionScaleFactorB - projectionScaleFactorA);

	gClipXY.y = (fSecInc * 256.0f + 2048.0f) - 1.0f;
	gClipXY.x = (2048.0f - fSecInc * 256.0f) + 1.0f;

	gClipMulVector.x = 2.0f / (gClipXY.y - gClipXY.x);
	gClipMulVector.y = 2.0f / (gClipXY.x - gClipXY.y);
	gClipAddVector.x = (gClipXY.x + gClipXY.y) / (gClipXY.x - gClipXY.y);
	gClipAddVector.y = (gClipXY.x + gClipXY.y) / (gClipXY.y - gClipXY.x);
	gClipMulVector.w = pCameraToScreen->dc + pCameraToScreen->cc * nearClip;
	gClipAddVector.z = (projectionScaleFactorA + projectionScaleFactorB) / (projectionScaleFactorA - projectionScaleFactorB);
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

ed_viewport* gShadowRenderViewport = NULL;

void ed3DShadowSetRenderViewport(ed_viewport* pViewport)
{
	gShadowRenderViewport = pViewport;
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

int ed3DInitRenderEnvironement(ed_3D_Scene* pScene, long mode)
{
	uint uVar1;
	edF32VECTOR4* v0;
	float projectionScaleFactorB;
	float projectionScaleFactorA;
	float farClip;
	float originalHalfFOV;
	edF32MATRIX4 local_40;

	ED3D_LOG(LogLevel::VeryVerbose, "ed3DInitRenderEnvironement: {}", mode);

	originalHalfFOV = pScene->pCamera->halfFOV;
	memcpy(gRenderCamera, pScene->pCamera, sizeof(edFCamera));

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
		v0->x = 1.0f;
	}
	else {
		edF32Vector4NormalizeHard(v0, v0);
	}

	edF32Matrix4GetInverseOrthoHard(&local_40, WorldToCamera_Matrix);
	gCamNormal_X = local_40.rowX;

	edF32Vector4NormalizeHard(&gCamNormal_X, &gCamNormal_X);
	gCamNormal_Y = local_40.rowY;

	edF32Vector4NormalizeHard(&gCamNormal_Y, &gCamNormal_Y);
	gCamNormal_Z = local_40.rowZ;

	edF32Vector4NormalizeHard(&gCamNormal_Z, &gCamNormal_Z);
	gCamPos = gRenderCamera->position;

	projectionScaleFactorB = gRenderCamera->halfFOV / gRenderCamera->verticalHalfFOV;
	gRenderInvFovCoef = 1.0f / projectionScaleFactorB;
	gRenderFovCoef = projectionScaleFactorB * projectionScaleFactorB;
	if (mode != 0) {
		gCurLOD_RenderFOVCoef = gRenderFovCoef;
	}

	memcpy(&gRenderSurface, pScene->pViewport->pColorBuffer, sizeof(edSurface));
	gRenderSceneConfig = &pScene->sceneConfig;
	(pScene->sceneConfig).clipValue_0x18 = (pScene->sceneConfig).clipValue_0x4;
	memcpy(gRenderSceneConfig_SPR, gRenderSceneConfig, sizeof(SceneConfig));
	gRenderScene = pScene;

	ed3DSceneLightSetGlobals(&(pScene->sceneConfig).pLightConfig);

	uVar1 = gRenderSceneConfig_SPR->projectionScaleFactorB;
	if ((int)uVar1 < 0) {
		projectionScaleFactorB = (float)(uVar1 >> 1 | uVar1 & 1);
		projectionScaleFactorB = projectionScaleFactorB + projectionScaleFactorB;
	}
	else {
		projectionScaleFactorB = (float)uVar1;
	}

	uVar1 = gRenderSceneConfig_SPR->projectionScaleFactorA;
	if ((int)uVar1 < 0) {
		projectionScaleFactorA = (float)(uVar1 >> 1 | uVar1 & 1);
		projectionScaleFactorA = projectionScaleFactorA + projectionScaleFactorA;
	}
	else {
		projectionScaleFactorA = (float)uVar1;
	}

	farClip = gRenderSceneConfig_SPR->farClip;
	ed3DViewportComputeViewMatrices
	((float)pScene->pViewport->screenWidth,
		(float)pScene->pViewport->screenHeight, gRenderCamera->horizontalHalfFOV,
		gRenderCamera->halfFOV, gRenderCamera->verticalHalfFOV, 2048.0f, 2048.0f,
		gRenderSceneConfig_SPR->nearClip, CameraToScreen_Matrix, CameraToCulling_Matrix, CameraToClipping_Matrix,
		&CameraToFog_Matrix, gRenderSceneConfig_SPR->farClip, projectionScaleFactorB, projectionScaleFactorA);

	ed3DFrustumCompute(gRenderSceneConfig_SPR, gRenderCamera, &gRenderSurface);

	edF32Matrix4MulF32Matrix4Hard(&WorldToScreen_Matrix, WorldToCamera_Matrix, CameraToScreen_Matrix);

	if ((pScene->flags & SCENE_FLAG_CAST_SHADOW) != 0) {
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
	if (pScene->bShadowScene == 1) {
		uVar1 = GetGreaterPower2Val((int)pScene->pViewport->screenWidth);
		if ((int)uVar1 < 0) {
			projectionScaleFactorB = (float)(uVar1 >> 1 | uVar1 & 1);
			projectionScaleFactorB = projectionScaleFactorB + projectionScaleFactorB;
		}
		else {
			projectionScaleFactorB = (float)uVar1;
		}

		uVar1 = GetGreaterPower2Val((int)pScene->pViewport->screenHeight);
		if ((int)uVar1 < 0) {
			projectionScaleFactorA = (float)(uVar1 >> 1 | uVar1 & 1);
			projectionScaleFactorA = projectionScaleFactorA + projectionScaleFactorA;
		}
		else {
			projectionScaleFactorA = (float)uVar1;
		}

		gDefault_Material_Current = &gMaterial_Render_Zbuffer_Only;
		gDefault_Material_Cluster_Current = &gMaterial_Render_Zbuffer_Only_Cluster;
		gCamPos.x = 2048.0f - projectionScaleFactorB / 2.0f;
		gCamPos.y = 2048.0f - projectionScaleFactorA / 2.0f;
		gCamPos.z = 1.0f / projectionScaleFactorB;
		gCamPos.w = 1.0f / projectionScaleFactorA;

		ed3DShadowSetRenderViewport((gRenderSceneConfig_SPR->pShadowConfig).pViewport);

		*gShadowRenderMask = (uint)(ushort)(gRenderSceneConfig_SPR->pShadowConfig).renderMask;
		if (*gShadowRenderMask == 0) {
			return 0;
		}

		memcpy(&WorldToCamera_Matrix_CastShadow, WorldToCamera_Matrix, sizeof(edF32MATRIX4));
		gShadowRenderScene = pScene;
	}
	else {
		*gShadowRenderMask = 0;
		gDefault_Material_Current = &gDefault_Material_Gouraud;
		gDefault_Material_Cluster_Current = &gDefault_Material_Gouraud_Cluster;
	}

	if (pScene->pViewport->screenHeight != 0x200) {
		pScene->pCamera->halfFOV = originalHalfFOV;
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

	// Packet to send to GIF when we push vertex data from the VIF.
	pPkt[2].cmdA = SCE_GIF_SET_TAG(
		0,				// NLOOP
		SCE_GS_TRUE,	// EOP
		SCE_GS_TRUE,	// PRE
		93,				// PRIM
		SCE_GIF_PACKED, // FLG
		3				// NREG
	);
	pPkt[2].cmdB = 0x512;


	pPkt[3].asVector = gClipMulVector;
	pPkt[4].asVector = gClipAddVector;
	pPkt[5].asVector = gClipXY;
	pPkt[6].asVector = gCamPos;

	// Can be overwritten later.
	pPkt[7].asVector = gCamNormal_X;
	pPkt[8].asVector = gCamNormal_Y;
	return pPkt + 9;
}

void ed3DInitVU1Globals(void)
{
	g_VifRefPktCur->cmdA = ED_VIF1_SET_TAG_REF(1, STORE_SECTION(&g_stMatrixHeader));
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

	ED3D_LOG(LogLevel::VeryVerbose, "ed3DSceneAddContextPacket 0x{:x}", (uintptr_t)pScene);

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
#ifdef PLATFORM_WIN
		VU1Emu::Wait();
#endif // PLATFORM_WIN

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
		(((0x1000 - ((int)pRect->h + (int)pRect->y) >> 1) - ((int)pRect->y >> 1)) * 0x10)					// Y
	);
	pPkt[2].cmdB = SCE_GS_XYOFFSET_1;

	// SCISSOR
	pPkt[3].cmdA = SCE_GS_SET_SCISSOR(
		pRect->x,								// SCAX0
		((int)pRect->w + (int)pRect->x + -1),	// SCAX1
		pRect->y,								// SCAY0
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

uint ed3DClusterGetEdgeWithSon(uint param_1, uchar param_2)

{
	uint uVar1;
	int iVar2;
	uint uVar3;
	uint auStack48[12];

	uVar1 = 0;
	if (param_2 == 1) {
		uVar3 = 8;
		iVar2 = 4;

		auStack48[8] = 0xa;
		auStack48[9] = 0x6;
		auStack48[10] = 0x9;
		auStack48[11] = 0x5;

		do {
			if ((param_1 & uVar3) != 0) {
				uVar1 = uVar1 | auStack48[iVar2 + 8];
			}

			iVar2 = iVar2 + -1;
			uVar3 = uVar3 >> 1;
		} while (iVar2 != 0);
	}
	else {
		uVar3 = 0x80;
		iVar2 = 8;

		auStack48[0] = 0x10a;
		auStack48[1] = 0x286;
		auStack48[2] = 0x409;
		auStack48[3] = 0x805;
		auStack48[4] = 0x1a0;
		auStack48[5] = 0x260;
		auStack48[6] = 0x490;
		auStack48[7] = 0x850;

		do {
			if ((param_1 & uVar3) != 0) {
				uVar1 = uVar1 | auStack48[iVar2];
			}

			iVar2 = iVar2 + -1;
			uVar3 = uVar3 >> 1;
		} while (iVar2 != 0);
	}
	return uVar1;
}

void ed3DSceneSortClusters(ed_3D_Scene* pScene)
{
	edLIST* peVar1;
	bool bListNotSorted;
	edLIST* pPrevNode;
	edLIST* pCurNode;
	edF32VECTOR4 local_10;
	edCluster* pClusterA;
	edCluster* pCluster;
	MeshData_CSTA* pCSTA;
	edCluster* pClusterB;

	ED3D_LOG(LogLevel::VeryVerbose, "ed3DSceneSortClusters flags: 0x{:x}", pScene->flags);

	bListNotSorted = true;

	if ((pScene->flags & 0x400) == 0) {
		pCurNode = &pScene->meshClusterList;
		ED3D_LOG(LogLevel::VeryVerbose, "ed3DSceneSortClusters using meshClusterList");
	}
	else {
		pCurNode = pScene->field_0x1a0;
	}

	pPrevNode = (edLIST*)pCurNode->pPrev;

	if (pPrevNode != pCurNode) {
		// Calculate the distance to to the camera.
		for (; pPrevNode != pCurNode; pPrevNode = (edLIST*)pPrevNode->pPrev) {
			pCluster = (edCluster*)pPrevNode->pData;
			pCSTA = reinterpret_cast<MeshData_CSTA*>(pCluster->pMeshInfo->CSTA + 2);

			if (pCluster->pMeshInfo->CSTA != (ed_Chunck*)0x0) {
				edF32Vector4SubHard(&local_10, &pScene->pCamera->position, &pCSTA->worldLocation);

				ED3D_LOG(LogLevel::VeryVerbose, "ed3DSceneSortClusters camera: {} CSTA: {} result: {}", pScene->pCamera->position.ToString(), pCSTA->worldLocation.ToString(), local_10.ToString());

				pCluster->distanceToCamera = local_10.z * local_10.z + local_10.x * local_10.x + local_10.y * local_10.y;
			}
		}

		// Sort the clusters.
		while (bListNotSorted) {
			pPrevNode = (edLIST*)pCurNode->pPrev;
			bListNotSorted = false;

			for (peVar1 = (edLIST*)pPrevNode->pPrev; peVar1 != pCurNode; peVar1 = (edLIST*)peVar1->pPrev) {
				pClusterA = (edCluster*)peVar1->pData;
				pClusterB = (edCluster*)pPrevNode->pData;

				if (pClusterB->distanceToCamera < pClusterA->distanceToCamera) {
					ED3D_LOG(LogLevel::VeryVerbose, "ed3DSceneSortClusters swap clusters: {} and {}", pClusterA->distanceToCamera, pClusterB->distanceToCamera);
					pPrevNode->pData = pClusterA;
					peVar1->pData = pClusterB;
					bListNotSorted = true;
				}

				pPrevNode = peVar1;
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

edpkt_data* ed3DHierachyCheckForGlobalAlphaSetPKT_(edpkt_data* pPkt, ed_g2d_layer* pLayer)
{
	byte bVar1;
	byte bVar2;
	long lVar3;
	ulong uVar4;
	long lVar5;
	uint uVar6;

	bVar1 = gpCurHierarchy->GlobalAlhaON;
	if ((bVar1 < 0x80) && (pLayer != (ed_g2d_layer*)0x0)) {

		uVar6 = pLayer->flags_0x0 & 0xfc;
		if ((pLayer->flags_0x4 & 8) == 0) {
			if (uVar6 == 0) {
				pPkt->cmdA = ED_VIF1_SET_TAG_REF(0, 0);
				pPkt->asU32[2] = SCE_VIF1_SET_NOP(0);
				pPkt->asU32[3] = SCE_VIF1_SET_FLUSHA(0);

				pPkt[1].cmdA = ED_VIF1_SET_TAG_CNT(3);
				pPkt[1].cmdB = 0;
				pPkt[1].asU32[2] = SCE_VIF1_SET_NOP(0);
				pPkt[1].asU32[3] = SCE_VIF1_SET_DIRECT(3, 0);

				pPkt[2].cmdA = SCE_GIF_SET_TAG(
					2,				// NLOOP
					SCE_GS_TRUE,	// EOP
					SCE_GS_FALSE,	// PRE
					0,				// PRIM
					SCE_GIF_PACKED, // FLG
					1				// NREG
				);
				pPkt[2].cmdB = SCE_GIF_PACKED_AD;

				pPkt[3].cmdA = SCE_GS_SET_TEST(
					1,	// ATE 
					5,	// ATST
					1,	// AREF
					0,	// AFAIL
					0,	// DATE
					0,	// DATM
					1,	// ZTE
					2	// ZTST
				);
				pPkt[3].cmdB = SCE_GS_TEST_1;

				pPkt[4].cmdA = SCE_GS_SET_ALPHA_1(
					0,	// A
					1,	// B
					0,	// C
					1,	// D
					0	// FIX
				);
				pPkt[4].cmdB = SCE_GS_ALPHA_1;

				pPkt = pPkt + 5;
			}
		}
		else {
			bVar2 = pLayer->field_0x1b;
			lVar5 = 1;
			uVar4 = 0;
			if (uVar6 == 0x80) {
				uVar4 = 0;
				lVar3 = 0;
				lVar5 = 0;
			}
			else {
				if (uVar6 == 0x40) {
					uVar4 = 2;
					lVar3 = 0;
					lVar5 = 1;
				}
				else {
					if (uVar6 == 0x20) {
						uVar4 = 0;
						lVar3 = 2;
						lVar5 = 1;
					}
					else {
						if (uVar6 == 0x10) {
							uVar4 = 0;
							lVar3 = 1;
							lVar5 = 2;
						}
						else {
							if (uVar6 == 8) {
								lVar3 = 1;
								uVar4 = 0;
								lVar5 = 1;
							}
							else {
								if ((uVar6 == 4) || (uVar6 == 0)) {
									lVar3 = 1;
									uVar4 = 0;
									lVar5 = 1;
								}
								else {
									lVar3 = 1;
								}
							}
						}
					}
				}
			}
			IMPLEMENTATION_GUARD();
			pPkt->cmdA = ED_VIF1_SET_TAG_REF(0, 0);
			pPkt->asU32[2] = SCE_VIF1_SET_NOP(0);
			pPkt->asU32[3] = SCE_VIF1_SET_FLUSHA(0);

			pPkt[1].cmdA = ED_VIF1_SET_TAG_CNT(3);
			pPkt[1].cmdB = 0;
			pPkt[1].asU32[2] = SCE_VIF1_SET_NOP(0);
			pPkt[1].asU32[3] = SCE_VIF1_SET_DIRECT(3, 0);

			pPkt[2].cmdA = SCE_GIF_SET_TAG(
				2,				// NLOOP
				SCE_GS_TRUE,	// EOP
				SCE_GS_FALSE,	// PRE
				0,				// PRIM
				SCE_GIF_PACKED, // FLG
				1				// NREG
			);
			pPkt[2].cmdB = SCE_GIF_PACKED_AD;

			pPkt[3].cmdA = SCE_GS_SET_TEST(
				1,	// ATE 
				5,	// ATST
				1,	// AREF
				0,	// AFAIL
				0,	// DATE
				0,	// DATM
				1,	// ZTE
				2	// ZTST
			);
			pPkt[3].cmdB = SCE_GS_TEST_1;

			IMPLEMENTATION_GUARD();
			pPkt[4].cmdA = ((long)((int)((uint)bVar1 * (uint)bVar2) >> 7) & 0xffU) << 0x20 |
				lVar5 << 6 | uVar4 | lVar3 << 2 | 0x20;
			pPkt[4].cmdA = SCE_GS_SET_ALPHA_1(
				0,	// A
				1,	// B
				0,	// C
				1,	// D
				0	// FIX
			);

			pPkt[4].cmdB = SCE_GS_ALPHA_1;
			pPkt = pPkt + 5;
		}
	}
	return pPkt;
}

edpkt_data* ed3DHierachyCheckForGlobalAlpha(edpkt_data* pRenderCommand, ed_g2d_layer* pLAY)
{
	if (((gpLastMaterial == (ed_dma_material*)0x0) || ((gpLastMaterial->flags & 1) != 0)) &&
		(gpLastMaterial != (ed_dma_material*)0x0)) {
		gGlobalAlhaON = 0x80;
	}
	else {
		if (((gpCurHierarchy == (ed_3d_hierarchy*)0x0) || (gpCurHierarchy == (ed_3d_hierarchy*)0x0000ffff))
			|| (gpCurHierarchy->GlobalAlhaON == 0xff)) {
			if (((gGlobalAlhaON != 0x80) && (gpCurHierarchy != (ed_3d_hierarchy*)0x0)) &&
				(((gpCurHierarchy != (ed_3d_hierarchy*)0x0000ffff &&
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
				pRenderCommand = ed3DHierachyCheckForGlobalAlphaSetPKT_(pRenderCommand, pLAY);
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

edpkt_data* ed3DPKTCopyMatrixPacket(edpkt_data* pPkt, ed_dma_matrix* pDmaMatrix, byte param_3)
{
	edF32MATRIX4* pObjToWorld;
	ed_3D_Light_Config* pLightConfig;
	float* pfVar3;	
	ed_3d_hierarchy_setup* pHierarchySetup;

	pObjToWorld = pDmaMatrix->pObjToWorld;
	ED3D_LOG(LogLevel::VeryVerbose, "DMA Begin ed3DPKTCopyMatrixPacket");
	edDmaSync(SHELLDMA_CHANNEL_VIF0);
	edF32Vector4ScaleHard(pDmaMatrix->normalScale, SCRATCHPAD_ADDRESS_TYPE(CAM_NORMAL_X_SPR, edF32VECTOR4*), &gCamNormal_X);
	edF32Vector4ScaleHard(pDmaMatrix->normalScale, SCRATCHPAD_ADDRESS_TYPE(CAM_NORMAL_Y_SPR, edF32VECTOR4*), &gCamNormal_Y);
	*g_pCurFlareObj2WorldMtx = pObjToWorld;

	if (((pDmaMatrix->pHierarchy == (ed_3d_hierarchy*)0x0) || (pHierarchySetup = pDmaMatrix->pHierarchy->pHierarchySetup, pHierarchySetup == (ed_3d_hierarchy_setup*)0x0))
		|| (pLightConfig = pHierarchySetup->pLightData, pLightConfig == (ed_3D_Light_Config*)0x0)) {
		// Default lighting.
		SCRATCHPAD_WRITE_ADDRESS_TYPE(LIGHT_DIRECTIONS_MATRIX_PTR_SPR, edF32MATRIX4*, gLightDirections_Matrix_Scratch);
		SCRATCHPAD_WRITE_ADDRESS_TYPE(LIGHT_COLOR_MATRIX_PTR_SPR, edF32MATRIX4*, gLightColor_Matrix_Scratch);
		SCRATCHPAD_WRITE_ADDRESS_TYPE(LIGHT_AMBIENT_MATRIX_PTR_SPR, edF32VECTOR4*, gLightAmbiant_Scratch);
	}
	else {
		SCRATCHPAD_WRITE_ADDRESS_TYPE(LIGHT_DIRECTIONS_MATRIX_PTR_SPR, edF32MATRIX4*, pLightConfig->pLightDirections);
		if (SCRATCHPAD_READ_ADDRESS_TYPE(LIGHT_DIRECTIONS_MATRIX_PTR_SPR, edF32MATRIX4*) == (edF32MATRIX4*)0x0) {
			SCRATCHPAD_WRITE_ADDRESS_TYPE(LIGHT_DIRECTIONS_MATRIX_PTR_SPR, edF32MATRIX4*, gLightDirections_Matrix_Scratch);
		}

		SCRATCHPAD_WRITE_ADDRESS_TYPE(LIGHT_COLOR_MATRIX_PTR_SPR, edF32MATRIX4*, pLightConfig->pLightColorMatrix);
		if (SCRATCHPAD_READ_ADDRESS_TYPE(LIGHT_COLOR_MATRIX_PTR_SPR, edF32MATRIX4*) == (edF32MATRIX4*)0x0) {
			SCRATCHPAD_WRITE_ADDRESS_TYPE(LIGHT_COLOR_MATRIX_PTR_SPR, edF32MATRIX4*, gLightColor_Matrix_Scratch);
		}

		SCRATCHPAD_WRITE_ADDRESS_TYPE(LIGHT_AMBIENT_MATRIX_PTR_SPR, edF32VECTOR4*, pLightConfig->pLightAmbient);
		if (SCRATCHPAD_READ_ADDRESS_TYPE(LIGHT_AMBIENT_MATRIX_PTR_SPR, edF32VECTOR4*) == (edF32VECTOR4*)0x0) {
			SCRATCHPAD_WRITE_ADDRESS_TYPE(LIGHT_AMBIENT_MATRIX_PTR_SPR, edF32VECTOR4*, gLightAmbiant_Scratch);
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
		pObjToWorld = gF32Matrix4Unit_Scratch;
	}

	edF32MATRIX4 objToWorld = *pObjToWorld;
	edF32MATRIX4 worldToCamera = *WorldToCamera_Matrix;

	edF32MATRIX4* pObjToCamera = SCRATCHPAD_ADDRESS_TYPE(OBJECT_TO_CAMERA_MATRIX_SPR, edF32MATRIX4*);
	*pObjToCamera = worldToCamera * objToWorld;

	edF32MATRIX4 cameraToCulling = *CameraToCulling_Matrix;
	edF32MATRIX4* pObjToCulling = SCRATCHPAD_ADDRESS_TYPE(OBJ_TO_CULLING_MATRIX, edF32MATRIX4*);
	*pObjToCulling = cameraToCulling * *pObjToCamera;
	
	edF32MATRIX4 cameraToClipping = *CameraToClipping_Matrix;
	edF32MATRIX4* pObjToClipping = SCRATCHPAD_ADDRESS_TYPE(OBJ_TO_CLIPPING_MATRIX, edF32MATRIX4*);
	*pObjToClipping = cameraToClipping * *pObjToCamera;

	edF32MATRIX4 cameraToScreen = *CameraToScreen_Matrix;
	edF32MATRIX4* pObjToScreen = SCRATCHPAD_ADDRESS_TYPE(OBJ_TO_SCREEN_MATRIX, edF32MATRIX4*);
	*pObjToScreen = cameraToScreen * *pObjToCamera;

#ifdef PLATFORM_WIN
	Renderer::PushGlobalMatrices(pObjToWorld->raw, WorldToCamera_Matrix->raw, gNativeProjectionMatrix.raw);
#endif

	ED3D_LOG(LogLevel::VeryVerbose, "ed3DPKTCopyMatrixPacket Obj To Screen: {}", pObjToScreen->ToString());

	edF32MATRIX4** pLightDirections = SCRATCHPAD_ADDRESS_TYPE(LIGHT_DIRECTIONS_MATRIX_PTR_SPR, edF32MATRIX4**);
	edF32MATRIX4** pLightColors = SCRATCHPAD_ADDRESS_TYPE(LIGHT_COLOR_MATRIX_PTR_SPR, edF32MATRIX4**);

	// Calculate OBJ_TO_WORLD_INVERSE_NORMAL_SPR
	edF32Matrix4OrthonormalizeHard(SCRATCHPAD_ADDRESS_TYPE(OBJ_TO_WORLD_INVERSE_NORMAL_SPR, edF32MATRIX4*), pObjToWorld);
	edF32Matrix4GetInverseOrthoHard(SCRATCHPAD_ADDRESS_TYPE(OBJ_TO_WORLD_INVERSE_NORMAL_SPR, edF32MATRIX4*), SCRATCHPAD_ADDRESS_TYPE(OBJ_TO_WORLD_INVERSE_NORMAL_SPR, edF32MATRIX4*));

	// Calculate Light directions
	edF32Matrix4MulF32Matrix4Hard(SCRATCHPAD_ADDRESS_TYPE(OBJ_LIGHT_DIRECTIONS_MATRIX_SPR, edF32MATRIX4*), *pLightDirections, SCRATCHPAD_ADDRESS_TYPE(OBJ_TO_WORLD_INVERSE_NORMAL_SPR, edF32MATRIX4*));
	edF32Matrix4GetTransposeHard(SCRATCHPAD_ADDRESS_TYPE(OBJ_LIGHT_DIRECTIONS_MATRIX_SPR, edF32MATRIX4*), SCRATCHPAD_ADDRESS_TYPE(OBJ_LIGHT_DIRECTIONS_MATRIX_SPR, edF32MATRIX4*));

	// Copy light colors
	edF32Matrix4CopyHard(SCRATCHPAD_ADDRESS_TYPE(LIGHT_COLOR_MATRIX_SPR, edF32MATRIX4*), *pLightColors);


	edF32VECTOR4* vectorB = SCRATCHPAD_ADDRESS_TYPE(ADJUSTED_LIGHT_AMBIENT_SPR, edF32VECTOR4*);
	edF32VECTOR4* vectorD = SCRATCHPAD_ADDRESS_TYPE(ANIM_ST_NORMAL_EXTRUDER_SPR, edF32VECTOR4*);
	edF32VECTOR4* vectorC = *SCRATCHPAD_ADDRESS_TYPE(LIGHT_AMBIENT_MATRIX_PTR_SPR, edF32VECTOR4**);

	vectorB->xyz = vectorC->xyz;
	vectorB->w = 0.0078125f;

	if (((pDmaMatrix->pHierarchy == (ed_3d_hierarchy*)0x0) ||
		(pHierarchySetup = pDmaMatrix->pHierarchy->pHierarchySetup, pHierarchySetup == (ed_3d_hierarchy_setup*)0x0))
		|| (pfVar3 = pHierarchySetup->field_0x10, pfVar3 == (float*)0x0)) {
		gVU1_AnimST_NormalExtruder_Scratch->z = FLOAT_00448a04;
	}
	else {
		gVU1_AnimST_NormalExtruder_Scratch->z = *pfVar3;
	}

	*vectorD = *gVU1_AnimST_NormalExtruder_Scratch;
	ED3D_LOG(LogLevel::VeryVerbose, "DMA End ed3DPKTCopyMatrixPacket");
	edDmaSync(SHELLDMA_CHANNEL_VIF0);
	edDmaLoadFromFastRam_nowait(SCRATCHPAD_ADDRESS(MATRIX_PACKET_START_SPR), 0x1c0, pPkt);
	return pPkt + 0x1c;
}

edpkt_data* ed3DPKTAddMatrixPacket(edpkt_data* pPkt, ed_dma_matrix* pDmaMatrix)
{
	ushort prevNodeType;
	edNODE* pPrevNode;
	ed_3d_hierarchy* pHierarchy;
	bool bObjToWorldIdentity;
	edpkt_data* pPKTMatrixCur;
	edF32MATRIX4* pObjToWorld;
	byte bVar11;

	ED3D_LOG(LogLevel::VeryVerbose, "ed3DPKTAddMatrixPacket 0x{:x}", (uintptr_t)pDmaMatrix->pHierarchy);

	pPrevNode = pDmaMatrix->pPrev;

	while (true) {
		prevNodeType = pPrevNode->header.typeField.type;
		bVar11 = 0;

		if (prevNodeType == LIST_TYPE_SPRITE) {
			IMPLEMENTATION_GUARD(
				if (((pMVar2->pRenderInput[1].subCommandBufferCount & 0x280) == 0) &&
					(pDmaMatrix->pHierarchy != &ed_3d_hierarchy_0048cad0)) {
					bVar11 = 1;
				})
		}
		else {
			if (prevNodeType == 6) {
				bVar11 = 1;
			}
		}

		if (gFushListCounter == 0xe) {
			IMPLEMENTATION_GUARD(pPkt = FUN_002a9220(pDmaMatrix->pObjToWorld, pPkt);)
		}

		pPKTMatrixCur = gPKTMatrixCur;
		pHierarchy = pDmaMatrix->pHierarchy;

		if (pHierarchy != (ed_3d_hierarchy*)0x0) break;

		pObjToWorld = pDmaMatrix->pObjToWorld;
		bObjToWorldIdentity = false;

		if ((((pObjToWorld->ac + pObjToWorld->aa + pObjToWorld->ab == 1.0f) &&
			(bObjToWorldIdentity = false, pObjToWorld->bc + pObjToWorld->ba + pObjToWorld->bb == 1.0f)) &&
			(bObjToWorldIdentity = false, pObjToWorld->cc + pObjToWorld->ca + pObjToWorld->cb == 1.0f)) &&
			(bObjToWorldIdentity = true, pObjToWorld->dd + pObjToWorld->dc + pObjToWorld->da + pObjToWorld->db != 1.0f)) {
			bObjToWorldIdentity = false;
		}

		if (!bObjToWorldIdentity) {
			pPkt->cmdA = ED_VIF1_SET_TAG_CNT(0x1c);
			pPkt->cmdB = 0;
			pPkt->asU32[2] = SCE_VIF1_SET_NOP(0);
			pPkt->asU32[3] = SCE_VIF1_SET_UNPACK(0x0006, 0x1c, UNPACK_V4_32, 0);
			pPKTMatrixCur = ed3DPKTCopyMatrixPacket(pPkt + 1, pDmaMatrix, bVar11);
			*g_pCurFlareMtx = (edF32MATRIX4*)((char*)(pPkt + 1) + 0xa0);
			
			if (bVar11 != 0) {
				edpkt_data* pRVar9 = pPKTMatrixCur + 1;
				pPKTMatrixCur->cmdA = ED_VIF1_SET_TAG_CNT(4);
				pPKTMatrixCur->cmdB = 0;
				pPKTMatrixCur->asU32[2] = SCE_VIF1_SET_NOP(0);
				pPKTMatrixCur->asU32[3] = SCE_VIF1_SET_UNPACK(0x03fb, 0x04, UNPACK_V4_32, 0);

				pRVar9[0].asVector = pDmaMatrix->pObjToWorld->rowX;
				pRVar9[1].asVector = pDmaMatrix->pObjToWorld->rowY;
				pRVar9[2].asVector = pDmaMatrix->pObjToWorld->rowZ;
				pRVar9[3].asVector = pDmaMatrix->pObjToWorld->rowT;

				pPKTMatrixCur = pPKTMatrixCur + 5;
			}
			return pPKTMatrixCur;
		}

		pDmaMatrix->pHierarchy = &ed_3d_hierarchy_0048cad0;
		ED3D_LOG(LogLevel::VeryVerbose, "Storing pMeshTransformData 0x{:x}", (uintptr_t)&ed_3d_hierarchy_0048cad0);
	}

	if (pHierarchy->pMatrixPkt == (edpkt_data*)0x0) {
		pHierarchy->pMatrixPkt = gPKTMatrixCur;

		gPKTMatrixCur = ed3DPKTCopyMatrixPacket(pPKTMatrixCur, pDmaMatrix, bVar11);
		if (bVar11 != 0) {
			gPKTMatrixCur[0].asVector = pDmaMatrix->pObjToWorld->rowX;
			gPKTMatrixCur[1].asVector = pDmaMatrix->pObjToWorld->rowY;
			gPKTMatrixCur[2].asVector = pDmaMatrix->pObjToWorld->rowZ;
			gPKTMatrixCur[3].asVector = pDmaMatrix->pObjToWorld->rowT;
			gPKTMatrixCur = gPKTMatrixCur + 4;
		}
	}

#ifdef PLATFORM_WIN
	// Process the matrix packet.
	Renderer::Native::PushMatrixPacket(reinterpret_cast<Renderer::Native::MatrixPacket*>(pHierarchy->pMatrixPkt));
#endif

	*g_pCurFlareMtx = (edF32MATRIX4*)((uint)((ulong)((long)(int)pDmaMatrix->pHierarchy->pMatrixPkt << 0x24) >> 0x24) + 0xa0);

	// This will actually overwrite the cam normal X and Y from the global packet, as that writes from 0x0 -> 0x8, and this starts at 0x6.
	pPkt->cmdA = ED_VIF1_SET_TAG_REF(0x1c, STORE_SECTION(pDmaMatrix->pHierarchy->pMatrixPkt) & 0xfffffffU);
	pPkt->asU32[2] = SCE_VIF1_SET_NOP(0);
	pPkt->asU32[3] = SCE_VIF1_SET_UNPACK(0x0006, 0x1c, UNPACK_V4_32, 0);

	if (bVar11 == 0) {
		return pPkt + 1;
	}

	// Check
	IMPLEMENTATION_GUARD();
	pPkt[1].cmdA = ED_VIF1_SET_TAG_REF(0x4, STORE_SECTION(pDmaMatrix->pHierarchy->pMatrixPkt + 7) & 0xfffffffU); // | 0x30000004;
	pPkt[1].asU32[2] = SCE_VIF1_SET_NOP(0);
	pPkt[1].asU32[3] = SCE_VIF1_SET_UNPACK(0x03fb, 0x04, UNPACK_V4_32, 0); // 0x6c0403fb00000000;

	return pPkt + 2;
}

edpkt_data* gBackupPKT = NULL;
edpkt_data* gStartPKT_SPR = NULL;

uint ed3DFlushStripGetIncPacket(ed_3d_strip* pStrip, bool param_2, bool bCachedSizeDirty)
{
	uint stripFlags;
	uint incPacket;
	char* pVifList;

	if (pStrip->cachedIncPacket == -1) {
		pStrip->cachedIncPacket = 0;
	}

	incPacket = pStrip->cachedIncPacket;

	ED3D_LOG(LogLevel::VeryVerbose, "ed3DFlushStripGetIncPacket 0x{:x} flags: {:x} initial incPacket: {:x}", (uintptr_t)pStrip, pStrip->flags, incPacket);

	if ((incPacket == 0) || (bCachedSizeDirty != false)) {
		stripFlags = pStrip->flags;
		pVifList = reinterpret_cast<char*>(pStrip) + pStrip->vifListOffset;

		if ((stripFlags & 4) == 0) {
			if ((stripFlags & 0x8000000) == 0) {
				if ((stripFlags & 0x2000000) == 0) {
					if (param_2 == false) {
						incPacket = 0xa;

						const edpkt_data* pVifListEnd = reinterpret_cast<edpkt_data*>(pVifList) + (pStrip->meshCount - 1) * incPacket + -1;

						if (pVifListEnd->asU32[0] != gVifEndCode) {
							incPacket = 9;
						}
					}
					else {
						incPacket = 7;
					}
				}
				else {
					incPacket = 8;
				}
			}
			else {
				if ((stripFlags & 0x2000000) == 0) {
					if (param_2 == false) {
						incPacket = 0xc;

						const edpkt_data* pVifListEnd = reinterpret_cast<edpkt_data*>(pVifList) + (pStrip->meshCount - 1) * incPacket + -1;

						if (pVifListEnd->asU32[0] != gVifEndCode) {
							incPacket = 9;
						}
					}
					else {
						incPacket = 9;
					}
				}
				else {
					incPacket = 0xa;
				}
			}
		}
		else {
			incPacket = 8;

			if ((stripFlags & 0x2000000) != 0) {
				incPacket = 6;
			}
		}

		if (bCachedSizeDirty == false) {
			pStrip->cachedIncPacket = incPacket;
		}
	}

	ED3D_LOG(LogLevel::VeryVerbose, "ed3DFlushStripGetIncPacket 0x{:x} final incPacket: {:x}", (uintptr_t)pStrip, incPacket);

	return incPacket;
}

ed_3D_Scene* gCurScene = NULL;

edpkt_data* ed3DFlushStripInit(edpkt_data* pPkt, edNODE* pNode, ulong mode)
{
	bool bVar1;
	ushort uVar2;
	ed_3d_strip* p3dStrip;
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
	int maxAnimMatrixCount;
	edF32MATRIX4* pAnimMatrices;
	uint animMatrixDestAddr;
	short* pAnimIndexes;
	edpkt_data* ppuVar15;
	edpkt_data* peVar17;
	edF32VECTOR4* qwc;

	ED3D_LOG(LogLevel::VeryVerbose, "ed3DFlushStripInit 0x{:x} flags: {:x}", (uintptr_t)pPkt, pNode->header.typeField.flags);

	p3dStrip = (ed_3d_strip*)pNode->pData;
	uVar2 = pNode->header.typeField.flags;
	PTR_AnimScratchpad_00449554->vuFlags = (int)(short)uVar2;
	PTR_AnimScratchpad_00449554->flags = 0;
	PTR_AnimScratchpad_00449554->field_0x8 = 0;
	PTR_AnimScratchpad_00449554->field_0xc = 0;

	if ((gFushListCounter == 0xe) && (FLOAT_00448a04 != 0.0)) {
		PTR_AnimScratchpad_00449554->flags = PTR_AnimScratchpad_00449554->flags | 1;
	}

	pPkt->cmdA = ED_VIF1_SET_TAG_CNT(1);
	pPkt->asU32[2] = SCE_VIF1_SET_NOP(0);
	pPkt->asU32[3] = SCE_VIF1_SET_UNPACK(0x0076, 0x1, UNPACK_V4_32_MASKED, 0); //0x7c010076
	pPkt[1] = PTR_AnimScratchpad_00449554->pkt;

	pPkt[2].cmdA = ED_VIF1_SET_TAG_CNT(1);
	pPkt[2].asU32[2] = SCE_VIF1_SET_NOP(0);
	pPkt[2].asU32[3] = SCE_VIF1_SET_UNPACK(0x0198, 0x1, UNPACK_V4_32_MASKED, 0); //0x7c01019800000000;
	pPkt[3] = PTR_AnimScratchpad_00449554->pkt;

	pPkt[4].cmdA = ED_VIF1_SET_TAG_CNT(1);
	pPkt[4].asU32[2] = SCE_VIF1_SET_NOP(0);
	pPkt[4].asU32[3] = SCE_VIF1_SET_UNPACK(0x02ba, 0x1, UNPACK_V4_32_MASKED, 0); //0x7c0102ba00000000;
	pPkt[5] = PTR_AnimScratchpad_00449554->pkt;

	ppuVar15 = pPkt + 6;
	if ((uVar2 & 2) != 0) {
		if ((p3dStrip->flags & 0x2000000) == 0) {
			ppuVar15->cmdA = ED_VIF1_SET_TAG_REF(1, STORE_SECTION(&g_stGifTAG_FAN_Texture_NoFog));
			ppuVar15->asU32[2] = SCE_VIF1_SET_NOP(0);
			ppuVar15->asU32[3] = SCE_VIF1_SET_UNPACK(0x1, 0x1, UNPACK_V4_32_MASKED, 0);
			ppuVar15 = pPkt + 7;
		}
		else {
			ppuVar15->cmdA = ED_VIF1_SET_TAG_REF(1, STORE_SECTION(&g_stGifTAG_FAN_Gouraud_NoFog));
			ppuVar15->asU32[2] = SCE_VIF1_SET_NOP(0);
			ppuVar15->asU32[3] = SCE_VIF1_SET_UNPACK(0x1, 0x1, UNPACK_V4_32_MASKED, 0);
			ppuVar15 = pPkt + 7;
		}
	}
	if (((p3dStrip->flags & 0x10000) != 0) && (gpCurHierarchy != (ed_3d_hierarchy*)0x0)) {
		pAnimIndexes = (short*)((char*)p3dStrip + p3dStrip->vifListOffset + -0x30);
		if ((p3dStrip->flags & 0x8000000) == 0) {
			animMatrixDestAddr = 0x394;
			maxAnimMatrixCount = 0x18;
		}
		else {
			animMatrixDestAddr = 0x3dc;
			if (mode < 2) {
				animMatrixDestAddr = 0x3dc;
				maxAnimMatrixCount = 9;
			}
			else {
				maxAnimMatrixCount = 7;
			}
		}
		if ((p3dStrip->shadowCastFlags == 0) || (pAnimMatrices = gpCurHierarchy->pShadowAnimMatrix, pAnimMatrices == (edF32MATRIX4*)0x0)) {
			pAnimMatrices = gpCurHierarchy->pAnimMatrix;
		}

		ppuVar15->cmdA = ED_VIF1_SET_TAG_REF(0, 0);
		ppuVar15->asU32[2] = SCE_VIF1_SET_FLUSH(0);
		ppuVar15->asU32[3] = SCE_VIF1_SET_FLUSH(0);
		ppuVar15[1] = g_PKTHeaderRef[4];

		peVar17 = (ppuVar15 + 2);
		ppuVar15 = (edpkt_data*)peVar17;

#ifdef PLATFORM_WIN
		ED3D_LOG(LogLevel::VeryVerbose, "ed3DFlushStripInit PTR_AnimScratchpad_00449554 upload");
		VU1Emu::UpdateMemory(pPkt, ppuVar15);
#endif

		if (gBackupPKT != (edpkt_data*)0x0) {
			uint pktLen = PKT_SIZE(peVar17, gStartPKT_SPR);
			edDmaSync(SHELLDMA_CHANNEL_VIF0);
			edDmaLoadFromFastRam_nowait(gStartPKT_SPR, pktLen, PKT_TO_SCRATCHPAD(gBackupPKT));
			gBackupPKT = (edpkt_data*)((ulong)gBackupPKT + (pktLen & 0xfffffff0));
			ppuVar15 = SCRATCHPAD_ADDRESS(0x70000800);
			gStartPKT_SPR = peVar17;
		}

		ED3D_LOG(LogLevel::VeryVerbose, "ed3DFlushStripInit Building anim matrix transfer pkts count: {}", maxAnimMatrixCount);

		// Push anim matrices.
		for (; (bVar1 = maxAnimMatrixCount != 0, maxAnimMatrixCount = maxAnimMatrixCount + -1, bVar1 && (-1 < *pAnimIndexes)); pAnimIndexes = pAnimIndexes + 1)
		{
			ppuVar15->cmdA = ED_VIF1_SET_TAG_CNT(4);
			ppuVar15->asU32[2] = SCE_VIF1_SET_NOP(0);
			ppuVar15->asU32[3] = SCE_VIF1_SET_UNPACK(animMatrixDestAddr, 0x04, UNPACK_V4_32, 0);
			edF32Matrix4CopyHard(((edF32MATRIX4*)(ppuVar15 + 1)), (pAnimMatrices + *pAnimIndexes));

#ifdef PLATFORM_WIN
			Renderer::PushAnimMatrix((pAnimMatrices + *pAnimIndexes)->raw);
#endif

			ppuVar15 = ppuVar15 + 5;
			animMatrixDestAddr = animMatrixDestAddr + 4;
		}

#ifdef PLATFORM_WIN
		ED3D_LOG(LogLevel::VeryVerbose, "ed3DFlushStripInit Processing anim matrix transfer pkts remainder: {}", maxAnimMatrixCount);
		VU1Emu::UpdateMemory(SCRATCHPAD_ADDRESS(0x70000800), ppuVar15);
#endif

		if (gBackupPKT != (edpkt_data*)0x0) {
			edDmaSync(SHELLDMA_CHANNEL_VIF0);
			uint pktLen = PKT_SIZE(ppuVar15, SCRATCHPAD_ADDRESS(0x70000800));
			edDmaLoadFromFastRam_nowait(SCRATCHPAD_ADDRESS(0x70000800), pktLen, PKT_TO_SCRATCHPAD(gBackupPKT));
			edDmaSync(SHELLDMA_CHANNEL_VIF0);
			ppuVar15 = SCRATCHPAD_ADDRESS(0x70001000);
			gStartPKT_SPR = SCRATCHPAD_ADDRESS(0x70001000);
			gBackupPKT = (edpkt_data*)((ulong)gBackupPKT + (pktLen & 0xfffffff0));
		}
	}
	if (((gpCurHierarchy == (ed_3d_hierarchy*)0x0) || (gpCurHierarchy == (ed_3d_hierarchy*)0x0000ffff))
		|| (gpCurHierarchy == &ed_3d_hierarchy_0048cad0)) {
		p3dStrip->flags = p3dStrip->flags & 0xfffffff7;
	}
	else {
		if ((BYTE_004493b4 == 0) && ((gpCurHierarchy->flags_0x9e & 2) == 0)) {
			p3dStrip->flags = p3dStrip->flags & 0xfffffff7;
		}
		else {
			IMPLEMENTATION_GUARD(
				p3dStrip->flags = p3dStrip->flags | 8;
			*(ulong*)&((edF32VECTOR4*)ppuVar15)->x = ED_VIF1_SET_TAG_REF(0, 0);
			*(ulong*)&((edF32VECTOR4*)ppuVar15)->z = 0x1100000011000000;
			if ((BYTE_004493b8 == 0) && ((gCurScene->flags & 0x200) == 0)) {
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
	if (((p3dStrip->flags & 0x100) != 0) || (gFushListCounter == 0xe)) {
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

int ed3DG3DNbMaxBaseRGBA(ed_3d_strip* pStrip)
{
	int* pRGBA = LOAD_SECTION_CAST(int*, pStrip->pColorBuf);
	return pRGBA[2];
}


struct ed_g3d_Anim_def {
	uint field_0x0;
	uint field_0x4;
	uint field_0x8;
	uint field_0xc;
	float field_0x10;
	uint field_0x14;
	uint field_0x18;
	undefined field_0x1c;
	undefined field_0x1d;
	undefined field_0x1e;
	undefined field_0x1f;
	uint field_0x20;
	float field_0x24;
	undefined field_0x28;
	undefined field_0x29;
	undefined field_0x2a;
	undefined field_0x2b;
	undefined field_0x2c;
	undefined field_0x2d;
	undefined field_0x2e;
	undefined field_0x2f;
	uint field_0x30;
	float field_0x34;
};

static_assert(sizeof(ed_g3d_Anim_def) == 0x38, "ed_g3d_Anim_def size is incorrect");

ed_g3d_Anim_def* ed3DG3DAnimGet(ed_3d_strip* pStrip)
{
	int* pData = LOAD_SECTION_CAST(int*, pStrip->pColorBuf);
	return LOAD_SECTION_CAST(ed_g3d_Anim_def*, pData[1]);
}

void ed3DFlushStrip(edNODE* pNode)
{
	undefined8 uVar1;
	uint incPacketSize;
	edpkt_data* pPktBufferB;
	//ed_g3d_Anim_def* iVar6;
	uint uVar7;
	uint partialMeshSectionCount;
	byte bVar9;
	ulong uVar10;
	float fVar11;
	uint uVar12;
	char* pVifListB;
	int* piVar14;
	char* pVifListC;
	undefined4 uVar16;
	undefined4 uVar17;
	edpkt_data* pVifRefPktCur;
	edpkt_data* ppuVar19;
	edpkt_data* pPktBufferA;
	int fullMeshSectionCount;
	edNODE* pNextNode;
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
	ed_3d_strip* p3dStrip;
	char* pcVar4;
	ulong uVar2;

	pVifRefPktCur = g_VifRefPktCur;
	p3dStrip = (ed_3d_strip*)pNode->pData;

	ED3D_LOG(LogLevel::Verbose, "ed3DFlushStrip 0x{:x} (From Node: 0x{:x})", (uintptr_t)p3dStrip, (uintptr_t)pNode);
	ZONE_SCOPED;

	pNextNode = pNode->pNext;
	pVifList = reinterpret_cast<char*>(p3dStrip) + p3dStrip->vifListOffset;
	incPacketSize = ed3DFlushStripGetIncPacket(p3dStrip, false, true);

	const uint globalAlhaON = (uint)gGlobalAlhaON;
	if (globalAlhaON != 0x80) {
		pNode->header.typeField.flags = pNode->header.typeField.flags | 0x20;
		pBFCVect.alphaON = globalAlhaON;

		pVifRefPktCur->cmdA = ED_VIF1_SET_TAG_CNT(1);
		pVifRefPktCur->asU32[2] = SCE_VIF1_SET_NOP(0);
		pVifRefPktCur->asU32[3] = SCE_VIF1_SET_UNPACK(0x003d, 0x1, UNPACK_V4_32_MASKED, 0);
		pVifRefPktCur[1].asVector = *(edF32VECTOR4*)&pBFCVect;

		pVifRefPktCur = pVifRefPktCur + 2;
	}

	if (pNextNode == (edNODE*)0x0) {
		ED3D_LOG(LogLevel::Verbose, "ed3DFlushStrip No next node");

		partialMeshSectionCount = p3dStrip->meshCount % 3;
		fullMeshSectionCount = p3dStrip->meshCount - partialMeshSectionCount;

		ED3D_LOG(LogLevel::Verbose, "ed3DFlushStrip partial: {} full: {}", partialMeshSectionCount, fullMeshSectionCount);

#ifdef PLATFORM_WIN
		VU1Emu::SetVifItop((*ed3DVU1Addr_Scratch)[ed3DVU1BufferCur] + 1);
		Renderer::Kya::GetMeshLibrary().AddFromStrip(p3dStrip);
#endif
		pPktBufferA = ed3DFlushStripInit(pVifRefPktCur, pNode, 1);

#ifdef PLATFORM_WIN
		if (Renderer::Native::GetUsePreprocessedVertices()) {
			return;
		}
#endif

#ifdef PLATFORM_WIN
		// Init stuff update.
		VU1Emu::UpdateMemory(g_VifRefPktCur, pPktBufferA);
#endif

		if (SCRATCHPAD_ADDRESS(0x70003000) < pPktBufferA + (uint)(ushort)p3dStrip->meshCount * 2) {
			uint pktLen = PKT_SIZE(pPktBufferA, gStartPKT_SPR);
			edDmaLoadFromFastRam(gStartPKT_SPR, pktLen, PKT_TO_SCRATCHPAD(gBackupPKT));
			pPktBufferA = SCRATCHPAD_ADDRESS(0x70001000);
			gStartPKT_SPR = SCRATCHPAD_ADDRESS(0x70001000);
			gBackupPKT = (edpkt_data*)((ulong)gBackupPKT + (pktLen & 0xfffffff0));
		}

		if ((p3dStrip->flags & 4) == 0) {
			pPktBufferB = pPktBufferA;
			while (bVar1 = partialMeshSectionCount != 0, partialMeshSectionCount = partialMeshSectionCount - 1, bVar1) {
				pPktBufferB->cmdA = ED_VIF1_SET_TAG_REF(0, 0);
				pPktBufferB->asU32[2] = SCE_VIF1_SET_BASE((*ed3DVU1Addr_Scratch)[ed3DVU1BufferCur] + 1, 0);
				pPktBufferB->asU32[3] = SCE_VIF1_SET_OFFSET(0, 0);

				ED3D_LOG(LogLevel::VeryVerbose, "Send strip Vif list Partial: 0x{:x} / {}", (uintptr_t)pVifList, partialMeshSectionCount);

				// Send strip Vif list.
				pPktBufferB[1].asU32[0] = SCE_VIF1_SET_DIRECT(0, 0);
				pPktBufferB[1].asU32[1] = (uint)pVifList & 0xfffffff;
				pPktBufferB[1].asU32[2] = 0;
				pPktBufferB[1].asU32[3] = SCE_VIF1_SET_ITOP((*ed3DVU1Addr_Scratch)[ed3DVU1BufferCur] + 1, 0);

#ifdef  PLATFORM_WIN
				// #LETS DRAAAAAW
				VU1Emu::SetVifItop((*ed3DVU1Addr_Scratch)[ed3DVU1BufferCur] + 1);
				VU1Emu::ProcessVifList((edpkt_data*)pVifList);
#endif //  PLATFORM_WIN

				pVifList = pVifList + incPacketSize * 0x10;

				pPktBufferB = pPktBufferB + 2;

				if (ed3DVU1BufferCur == 2) {
					ed3DVU1BufferCur = 0;
				}
				else {
					ed3DVU1BufferCur = ed3DVU1BufferCur + 1;
				}
			}

			for (; fullMeshSectionCount != 0; fullMeshSectionCount = fullMeshSectionCount + -3) {

				pVifListB = pVifList + incPacketSize * 0x10;
				pVifListC = pVifList + incPacketSize * 0x20;

				ED3D_LOG(LogLevel::VeryVerbose, "Send strip Vif list Full: 0x{:x} / {}", (uintptr_t)pVifList, fullMeshSectionCount);

				pPktBufferB->cmdA = ED_VIF1_SET_TAG_REF(0, 0);
				pPktBufferB->asU32[2] = SCE_VIF1_SET_BASE((*ed3DVU1AddrWithBufCur_Scratch)[ed3DVU1BufferCur][0] + 1, 0);
				pPktBufferB->asU32[3] = SCE_VIF1_SET_OFFSET(0, 0);

				pPktBufferB[1].asU32[0] = SCE_VIF1_SET_DIRECT(0, 0);
				pPktBufferB[1].asU32[1] = (uint)pVifList & 0xfffffff;
				pPktBufferB[1].asU32[2] = 0;
				pPktBufferB[1].asU32[3] = SCE_VIF1_SET_ITOP((*ed3DVU1AddrWithBufCur_Scratch)[ed3DVU1BufferCur][0] + 1, 0);

#ifdef  PLATFORM_WIN
				// #LETS DRAAAAAW
				VU1Emu::SetVifItop((*ed3DVU1AddrWithBufCur_Scratch)[ed3DVU1BufferCur][0] + 1);
				VU1Emu::ProcessVifList((edpkt_data*)pVifList);
#endif //  PLATFORM_WIN

				pPktBufferB[2].cmdA = ED_VIF1_SET_TAG_REF(0, 0);
				pPktBufferB[2].asU32[2] = SCE_VIF1_SET_BASE((*ed3DVU1AddrWithBufCur_Scratch)[ed3DVU1BufferCur][1] + 1, 0);
				pPktBufferB[2].asU32[3] = SCE_VIF1_SET_OFFSET(0, 0);

				pPktBufferB[3].asU32[0] = SCE_VIF1_SET_DIRECT(0, 0);
				pPktBufferB[3].asU32[1] = (uint)pVifListB & 0xfffffff;
				pPktBufferB[3].asU32[2] = 0;
				pPktBufferB[3].asU32[3] = SCE_VIF1_SET_ITOP((*ed3DVU1AddrWithBufCur_Scratch)[ed3DVU1BufferCur][1] + 1, 0);

#ifdef  PLATFORM_WIN
				// #LETS DRAAAAAW
				VU1Emu::SetVifItop((*ed3DVU1AddrWithBufCur_Scratch)[ed3DVU1BufferCur][1] + 1);
				VU1Emu::ProcessVifList((edpkt_data*)pVifListB);
#endif //  PLATFORM_WIN

				pPktBufferB[4].cmdA = ED_VIF1_SET_TAG_REF(0, 0);
				pPktBufferB[4].asU32[2] = SCE_VIF1_SET_BASE((*ed3DVU1AddrWithBufCur_Scratch)[ed3DVU1BufferCur][2] + 1, 0);
				pPktBufferB[4].asU32[3] = SCE_VIF1_SET_OFFSET(0, 0);

				pPktBufferB[5].asU32[0] = SCE_VIF1_SET_DIRECT(0, 0);
				pPktBufferB[5].asU32[1] = (uint)pVifListC & 0xfffffff;
				pPktBufferB[5].asU32[2] = 0;
				pPktBufferB[5].asU32[3] = SCE_VIF1_SET_ITOP((*ed3DVU1AddrWithBufCur_Scratch)[ed3DVU1BufferCur][2] + 1, 0);

#ifdef  PLATFORM_WIN
				// #LETS DRAAAAAW
				VU1Emu::SetVifItop((*ed3DVU1AddrWithBufCur_Scratch)[ed3DVU1BufferCur][2] + 1);
				VU1Emu::ProcessVifList((edpkt_data*)pVifListC);
#endif //  PLATFORM_WIN

				pPktBufferB = pPktBufferB + 6;
				pVifList = pVifList + incPacketSize * 0x30;

			}
		}
		else {
			uVar23 = 0x48;
			ed_g3d_Anim_def* pAnimDef = ed3DG3DAnimGet(p3dStrip);
			partialMeshSectionCount = ed3DG3DNbMaxBaseRGBA(p3dStrip);

			fVar11 = pAnimDef->field_0x10;
			if (fVar11 < 2.147484e+09f) {
				uVar7 = (uint)fVar11;
			}
			else {
				uVar7 = (int)(fVar11 - 2.147484e+09f) | 0x80000000;
			}

			if ((int)uVar7 < 0) {
				fVar27 = (float)(uVar7 >> 1 | uVar7 & 1);
				fVar27 = fVar27 + fVar27;
			}
			else {
				fVar27 = (float)uVar7;
			}

			fVar11 = fVar11 - fVar27;
			piVar14 = LOAD_SECTION_CAST(int*, p3dStrip->pColorBuf);
			fullMeshSectionCount = *piVar14;
			piVar22 = piVar14 + uVar7 * fullMeshSectionCount * 4 + 4;
			piVar21 = piVar22;
			if ((pAnimDef->field_0x4 & 8) == 0) {
				uVar26 = partialMeshSectionCount - 1;

				if (uVar7 < uVar26) {
					piVar21 = piVar14 + (uVar7 + 1) * fullMeshSectionCount * 4 + 4;
				}
				else {
					if ((uVar7 != uVar26) && (piVar21 = (int*)0x0, uVar7 == partialMeshSectionCount)) {
						fVar11 = 1.0f;
						piVar22 = piVar14 + (partialMeshSectionCount - 2) * fullMeshSectionCount * 4 + 4;
						piVar21 = piVar14 + uVar26 * fullMeshSectionCount * 4 + 4;
					}
				}
			}
			else {
				uVar26 = partialMeshSectionCount - 1;

				if (uVar7 < uVar26) {
					piVar21 = piVar14 + (uVar7 + 1) * fullMeshSectionCount * 4 + 4;
				}
				else {
					if ((uVar7 != uVar26) && (piVar21 = (int*)0x0, uVar7 == partialMeshSectionCount)) {
						fVar11 = 1.0f;
						piVar22 = piVar14 + (partialMeshSectionCount - 2) * fullMeshSectionCount * 4 + 4;
						piVar21 = piVar14 + uVar26 * fullMeshSectionCount * 4 + 4;
					}
				}
			}

			pPktBufferA->cmdA = ED_VIF1_SET_TAG_REF(0, 0);
			pPktBufferA->asU32[2] = SCE_VIF1_SET_NOP(0);
			pPktBufferA->asU32[3] = SCE_VIF1_SET_FLUSHA(0);

			pPktBufferA[1].cmdA = ED_VIF1_SET_TAG_REF(0, 0);
			pPktBufferA[1].asU32[2] = SCE_VIF1_SET_FLUSH(0);
			pPktBufferA[1].asU32[3] = SCE_VIF1_SET_FLUSH(0);

			pPktBufferA[2].cmdA = ED_VIF1_SET_TAG_CNT(1);
			pPktBufferA[2].asU32[2] = SCE_VIF1_SET_NOP(0);
			pPktBufferA[2].asU32[3] = SCE_VIF1_SET_UNPACK(0x003d, 0x01, UNPACK_V4_32_MASKED, 0);

			pBFCVect.z = 256.0f - fVar11 * 256.0f;
			pPktBufferA[3].asVector = *reinterpret_cast<edF32VECTOR4*>(&pBFCVect);

			pPktBufferA[4].cmdA = ED_VIF1_SET_TAG_REF(0, 0);
			pPktBufferA[4].asU32[2] = SCE_VIF1_SET_FLUSH(0);
			pPktBufferA[4].asU32[3] = SCE_VIF1_SET_FLUSH(0);

			pVifRefPktCur = pPktBufferA + 5;
			for (partialMeshSectionCount = 0; partialMeshSectionCount < (ushort)p3dStrip->meshCount; partialMeshSectionCount = partialMeshSectionCount + 1) {
				if (partialMeshSectionCount == (ushort)p3dStrip->meshCount - 1) {
					for (uVar23 = (uint)p3dStrip->field_0x38; (uVar23 & 3) != 0; uVar23 = uVar23 + 1) {
					}
				}
		
				piVar22 = piVar22 + 0x48;
				piVar21 = piVar21 + 0x48;
				pVifList = pVifList + incPacketSize * 0x10;

				pVifRefPktCur->cmdA = ED_VIF1_SET_TAG_REF(0, 0);
				pVifRefPktCur->asU32[2] = SCE_VIF1_SET_BASE((*ed3DVU1AddrWithBufCur_Scratch)[ed3DVU1BufferCur][0] + 1, 0);
				pVifRefPktCur->asU32[3] = SCE_VIF1_SET_OFFSET(0, 0);

				pVifRefPktCur[1].cmdA = ED_VIF1_SET_TAG_REF(1, STORE_SECTION(&g_stVertexRGBAHeader));
				pVifRefPktCur[1].cmdB = 0;

				pVifRefPktCur[2].cmdA = ED_VIF1_SET_TAG_REF(uVar23, STORE_SECTION(piVar22));
				pVifRefPktCur[2].asU32[2] = SCE_VIF1_SET_NOP(0);
				pVifRefPktCur[2].asU32[3] = SCE_VIF1_SET_UNPACK(0xc002, uVar23, UNPACK_V4_8_MASKED, 0);

				pVifRefPktCur[3].cmdA = ED_VIF1_SET_TAG_REF(1, STORE_SECTION(&g_stNormalHeader));
				pVifRefPktCur[3].cmdB = 0;

				pVifRefPktCur[4].cmdA = ED_VIF1_SET_TAG_REF(uVar23, STORE_SECTION(piVar21));
				pVifRefPktCur[4].asU32[2] = SCE_VIF1_SET_NOP(0);
				pVifRefPktCur[4].asU32[3] = SCE_VIF1_SET_UNPACK(0xc0d9, uVar23, UNPACK_V4_8_MASKED, 0);

				pVifRefPktCur[5].asU32[0] = SCE_VIF1_SET_DIRECT(0, 0);
				pVifRefPktCur[5].asU32[1] = (uint)pVifList & 0xfffffff;
				pVifRefPktCur[5].asU32[2] = 0;
				pVifRefPktCur[5].asU32[3] = SCE_VIF1_SET_ITOP((*ed3DVU1AddrWithBufCur_Scratch)[ed3DVU1BufferCur][1] + 1, 0);

				pVifRefPktCur = pVifRefPktCur + 6;

				if (ed3DVU1BufferCur == 2) {
					ed3DVU1BufferCur = 0;
				}
				else {
					ed3DVU1BufferCur = ed3DVU1BufferCur + 1;
				}
			}

			pVifRefPktCur->cmdA = ED_VIF1_SET_TAG_REF(0, 0);
			pVifRefPktCur->cmdB = 0x1300000000000000;
			pPktBufferB = pVifRefPktCur + 1;
		}
	}
	else {
		ED3D_LOG(LogLevel::Verbose, "ed3DFlushStrip With next node");
		uint stripFlags = p3dStrip->flags;
		pRVar25 = 0;
		uVar23 = 0;
		short meshSectionCount = p3dStrip->meshCount;

#ifdef PLATFORM_WIN
		VU1Emu::SetVifItop((*ed3DVU1Addr_Scratch)[ed3DVU1BufferCur] + 1);
		Renderer::Kya::GetMeshLibrary().AddFromStrip(p3dStrip);
#endif

		pPktBufferB = ed3DFlushStripInit(pVifRefPktCur, pNode, 1);

#ifdef PLATFORM_WIN
		if (Renderer::Native::GetUsePreprocessedVertices()) {
			return;
		}
#endif

#ifdef PLATFORM_WIN
		VU1Emu::UpdateMemory(g_VifRefPktCur, pPktBufferB);
#endif

		if (SCRATCHPAD_ADDRESS(0x70003000) < pPktBufferB + meshSectionCount * 4) {
			uint pktLen = PKT_SIZE(pPktBufferB, gStartPKT_SPR);
			edDmaLoadFromFastRam(gStartPKT_SPR, pktLen, PKT_TO_SCRATCHPAD(gBackupPKT));
			pPktBufferB = SCRATCHPAD_ADDRESS(0x70001000);
			gStartPKT_SPR = SCRATCHPAD_ADDRESS(0x70001000);
			gBackupPKT = (edpkt_data*)((ulong)gBackupPKT + (pktLen & 0xfffffff0));
		}

		if ((p3dStrip->flags & 4) == 0) {
			while (pRVar25 < meshSectionCount) {
				bVar9 = pNextNode->header.byteFlags[0] & 3;
				if (bVar9 == 1) {
					stripFlags = stripFlags & 0xfffffffd | 1;
				LAB_002a5dac:
					if (SCRATCHPAD_ADDRESS(0x70003fff) < pPktBufferB + 0x80) {
						uint pktLen = PKT_SIZE(pPktBufferB, gStartPKT_SPR);
						edDmaLoadFromFastRam(gStartPKT_SPR, pktLen, PKT_TO_SCRATCHPAD(gBackupPKT));
						pPktBufferB = SCRATCHPAD_ADDRESS(0x70001000);
						gStartPKT_SPR = SCRATCHPAD_ADDRESS(0x70001000);
						gBackupPKT = (edpkt_data*)((ulong)gBackupPKT + (pktLen & 0xfffffff0));
					}

					PTR_AnimScratchpad_00449554->vuFlags = stripFlags;

					pPktBufferB->cmdA = gOptionFlagCNT[ed3DVU1BufferCur].cmdA;
					pPktBufferB->cmdB = gOptionFlagCNT[ed3DVU1BufferCur].cmdB;

					pPktBufferB[1] = PTR_AnimScratchpad_00449554->pkt;

					pPktBufferB[2] = gRefOptionsforVU1Buf[ed3DVU1BufferCur];

#ifdef PLATFORM_WIN
					VU1Emu::UpdateMemory(pPktBufferB, pPktBufferB + 3);
#endif



					ED3D_LOG(LogLevel::VeryVerbose, "Send strip Vif list Section: 0x{:x} / {}", (uintptr_t)pVifList, meshSectionCount);

					// Send strip Vif list.
					pPktBufferB[3].asU32[0] = SCE_VIF1_SET_DIRECT(0, 0);
					pPktBufferB[3].asU32[1] = (uint)pVifList & 0xfffffff;
					pPktBufferB[3].asU32[2] = 0;
					pPktBufferB[3].asU32[3] = SCE_VIF1_SET_ITOP((*ed3DVU1Addr_Scratch)[ed3DVU1BufferCur] + 1, 0);

					pPktBufferB = pPktBufferB + 4;

#ifdef  PLATFORM_WIN
					// #LETS DRAAAAAW
					VU1Emu::SetVifItop((*ed3DVU1Addr_Scratch)[ed3DVU1BufferCur] + 1);
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
					if ((pNextNode->header.byteFlags[0] & 3) != 0) {
						if (bVar9 == 3) {
							stripFlags = stripFlags & 0xfffffffc;
						}
						else {
							if (bVar9 == 2) {
								stripFlags = stripFlags | 3;
							}
						}
						goto LAB_002a5dac;
					}

					if (SCRATCHPAD_ADDRESS(0x70003fff) < pPktBufferB + 0x80) {
						uint pktLen = PKT_SIZE(pPktBufferB,gStartPKT_SPR);
						edDmaLoadFromFastRam(gStartPKT_SPR, pktLen, PKT_TO_SCRATCHPAD(gBackupPKT));
						pPktBufferB = SCRATCHPAD_ADDRESS(0x70001000);
						gStartPKT_SPR = SCRATCHPAD_ADDRESS(0x70001000);
						gBackupPKT = (edpkt_data*)((ulong)gBackupPKT + (pktLen & 0xfffffff0));
					}

					pPktBufferB->cmdA = ED_VIF1_SET_TAG_CNT(1);
					pPktBufferB->asU32[2] = 0;
					pPktBufferB->asU32[3] = SCE_VIF1_SET_UNPACK(0x003d, 0x01, UNPACK_V4_32_MASKED, 0);
					pPktBufferB[1].asVector = *reinterpret_cast<edF32VECTOR4*>(&pBFCVect);

#ifdef PLATFORM_WIN
					VU1Emu::UpdateMemory(pPktBufferB, pPktBufferB + 2);
#endif

					pPktBufferB = pPktBufferB + 2;
				}

				uVar23 = uVar23 + 1;
				pVifList = pVifList + incPacketSize * 0x10;
				pRVar25 = pRVar25 + 1;

				pNextNode->header.byteFlags[0] = pNextNode->header.byteFlags[0] >> 2;

				if (3 < uVar23) {
					uVar23 = 0;
					pNextNode = (edNODE*)((ulong)&pNextNode->header + 1);
				}
			}
		}
		else {
			IMPLEMENTATION_GUARD(
				local_20 = 0x48;
			pAnimDef = ed3DG3DAnimGet(p3dStrip);
			uVar7 = ed3DG3DNbMaxBaseRGBA(p3dStrip);
			fVar11 = pAnimDef->field_0x10;
			if (fVar11 < 2.147484e+09f) {
				uVar26 = (uint)fVar11;
			}
			else {
				uVar26 = (int)(fVar11 - 2.147484e+09f) | 0x80000000;
			}
			if ((int)uVar26 < 0) {
				fVar27 = (float)(uVar26 >> 1 | uVar26 & 1);
				fVar27 = fVar27 + fVar27;
			}
			else {
				fVar27 = (float)uVar26;
			}
			fVar11 = fVar11 - fVar27;
			piVar14 = (int*)p3dStrip->field_0x24;
			fullMeshSectionCount = *piVar14;
			local_30 = piVar14 + uVar26 * fullMeshSectionCount * 4 + 4;
			piVar21 = local_30;
			if ((pAnimDef->field_0x4 & 8) == 0) {
				uVar12 = uVar7 - 1;
				if (uVar26 < uVar12) {
					piVar21 = piVar14 + (uVar26 + 1) * fullMeshSectionCount * 4 + 4;
				}
				else {
					if ((uVar26 != uVar12) && (piVar21 = (int*)0x0, uVar26 == uVar7)) {
						fVar11 = 1.0;
						local_30 = piVar14 + (uVar7 - 2) * fullMeshSectionCount * 4 + 4;
						piVar21 = piVar14 + uVar12 * fullMeshSectionCount * 4 + 4;
					}
				}
			}
			else {
				uVar12 = uVar7 - 1;
				if (uVar26 < uVar12) {
					piVar21 = piVar14 + (uVar26 + 1) * fullMeshSectionCount * 4 + 4;
				}
				else {
					if ((uVar26 != uVar12) && (piVar21 = (int*)0x0, uVar26 == uVar7)) {
						fVar11 = 1.0f;
						local_30 = piVar14 + (uVar7 - 2) * fullMeshSectionCount * 4 + 4;
						piVar21 = piVar14 + uVar12 * fullMeshSectionCount * 4 + 4;
					}
				}
			}

			// #VIF_Tidy
			IMPLEMENTATION_GUARD();
			pPktBufferB->cmdA = ED_VIF1_SET_TAG_REF(0, 0);
			pPktBufferB->cmdB = 0x1300000000000000;
			pPktBufferB[1].cmdA = ED_VIF1_SET_TAG_REF(0, 0);
			pPktBufferB[1].cmdB = 0x1100000011000000;
			pBFCVect.z = 256.0 - fVar11 * 256.0;
			pPktBufferB[2].cmdA = ED_VIF1_SET_TAG_CNT(1);
			pPktBufferB[2].cmdB = 0x7c01003d00000000;
			fVar3 = pBFCVect.w;
			fVar27 = pBFCVect.z;
			fVar11 = pBFCVect.y;
			*(float*)&pPktBufferB[3].cmdA = pBFCVect.x;
			*(float*)((ulong)&pPktBufferB[3].cmdA + 4) = fVar11;
			*(float*)&pPktBufferB[3].cmdB = fVar27;
			*(float*)((ulong)&pPktBufferB[3].cmdB + 4) = fVar3;
			pPktBufferB[4].cmdA = ED_VIF1_SET_TAG_REF(0, 0);
			pPktBufferB[4].cmdB = 0x1100000011000000;
			ppuVar19 = pPktBufferB + 5;
			while (pRVar25 < pPktBufferB) {
				bVar9 = *(byte*)&pNextNode->header & 3;
				if (bVar9 == 1) {
					stripFlags = stripFlags & 0xfffffffd | 1;
				LAB_002a5a74:
					if (SCRATCHPAD_ADDRESS(0x70003fff) < ppuVar19 + 4) {
						IMPLEMENTATION_GUARD();
						pVifRefPktCur = (edpkt_data*)((int)ppuVar19 - (int)gStartPKT_SPR);
						edDmaLoadFromFastRam(gStartPKT_SPR, (uint)pVifRefPktCur, (uint)((ulong)((long)(int)gBackupPKT << 0x24) >> 0x24));
						ppuVar19 = edpkt_data_ARRAY_70001000;
						gStartPKT_SPR = edpkt_data_ARRAY_70001000;
						gBackupPKT = (edpkt_data*)((ulong)&gBackupPKT->cmdA + ((uint)pVifRefPktCur & 0xfffffff0));
					}
					if (pRVar25 == (ushort)p3dStrip->meshCount - 1) {
						for (local_20 = (uint)p3dStrip->field_0x38; (local_20 & 3) != 0; local_20 = local_20 + 1) {
						}
					}

					// #VIF_Tidy
					IMPLEMENTATION_GUARD();
					uVar10 = (long)(int)(local_20 >> 2) | ED_VIF1_SET_TAG_REF(0, 0);
					gScratchPkt->field_0x0 = stripFlags;
					uVar7 = ed3DVU1BufferCur;
					pVifListC = ed3DVU1BufferCur * 0x10;
					fullMeshSectionCount = ed3DVU1BufferCur * 2;
					uVar2 = edpkt_data_ARRAY_0048c5f0[ed3DVU1BufferCur].cmdA;
					uVar16 = *(undefined4*)&edpkt_data_ARRAY_0048c5f0[ed3DVU1BufferCur].cmdB;
					uVar17 = *(undefined4*)((ulong)&edpkt_data_ARRAY_0048c5f0[ed3DVU1BufferCur].cmdB + 4);
					pVifListB = ed3DVU1BufferCur * 4;
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
					uVar2 = gRefOptionsforVU1Buf[fullMeshSectionCount];
					uVar16 = *(undefined4*)(gRefOptionsforVU1Buf + uVar7 * 2 + 1);
					uVar17 = *(undefined4*)((int)gRefOptionsforVU1Buf + pVifListC + 0xc);
					*(int*)&ppuVar19[2].cmdA = (int)uVar2;
					*(int*)((ulong)&ppuVar19[2].cmdA + 4) = (int)(uVar2 >> 0x20);
					*(undefined4*)&ppuVar19[2].cmdB = uVar16;
					*(undefined4*)((ulong)&ppuVar19[2].cmdB + 4) = uVar17;
					ppuVar19[3].cmdA = ED_VIF1_SET_TAG_REF(0, 0);
					piVar14 = (int*)(ScratchpadAnimManager_004494ec.field_0x34 + pVifListB);
					ppuVar19[3].cmdB = (long)(*piVar14 + 1) & 0xffffffffU | 0x200000003000000;
					ppuVar19[4].cmdA = ED_VIF1_SET_TAG_REF(1, STORE_SECTION(&g_stVertexRGBAHeader));
					ppuVar19[4].cmdB = 0;
					ppuVar19[5].cmdA = ((long)(int)local_30 & 0xfffffffU) << 0x20 | uVar10;
					ppuVar19[5].cmdB = ((long)(int)(local_20 << 0x10) & 0xffffffffU | 0x7e00c002) << 0x20;
					ppuVar19[6].cmdA = ED_VIF1_SET_TAG_REF(1, STORE_SECTION(&g_stNormalHeader));
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
					if ((*(byte*)&pNextNode->header & 3) != 0) {
						if (bVar9 == 3) {
							stripFlags = stripFlags & 0xfffffffc;
						}
						else {
							if (bVar9 == 2) {
								stripFlags = stripFlags | 3;
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
				pVifList = pVifList + incPacketSize * 0x10;
				*(char*)&pNextNode->header = (char)((int)(uint) * (byte*)&pNextNode->header >> 2);
				if (3 < uVar23) {
					uVar23 = 0;
					pNextNode = (edNODE*)((ulong)&pNextNode->header + 1);
				}
			}
			ppuVar19->cmdA = ED_VIF1_SET_TAG_REF(0, 0);
			ppuVar19->cmdB = 0x1300000000000000;
			pPktBufferB = ppuVar19 + 1;)
		}
	}

	if (ed3DVU1BufferCur == 2) {
		ed3DVU1Buffer[0] = (*ed3DVU1Addr_Scratch)[1];
		ed3DVU1Buffer[1] = (*ed3DVU1Addr_Scratch)[2];
		ed3DVU1Buffer[2] = (*ed3DVU1Addr_Scratch)[0];
	}
	else {
		if (ed3DVU1BufferCur == 1) {
			ed3DVU1Buffer[0] = (*ed3DVU1Addr_Scratch)[0];
			ed3DVU1Buffer[1] = (*ed3DVU1Addr_Scratch)[1];
			ed3DVU1Buffer[2] = (*ed3DVU1Addr_Scratch)[2];
		}
		else {
			if (ed3DVU1BufferCur == 0) {
				ed3DVU1Buffer[0] = (*ed3DVU1Addr_Scratch)[2];
				ed3DVU1Buffer[1] = (*ed3DVU1Addr_Scratch)[0];
				ed3DVU1Buffer[2] = (*ed3DVU1Addr_Scratch)[1];
			}
		}
	}

	if ((*gShadowRenderMask != 0) && (p3dStrip->shadowCastFlags != 0)) {
		pPktBufferB->cmdA = ED_VIF1_SET_TAG_REF(0, 0);
		pPktBufferB->cmdB = 0;
		pPktBufferB->asU32[2] = SCE_VIF1_SET_FLUSHA(0);
		pPktBufferB = pPktBufferB + 1;
	}

	g_VifRefPktCur = pPktBufferB;

	return;
}

void ed3DFlushOptionState(edNODE* pNode)
{
	edpkt_data* pPkt = (edpkt_data*)pNode->pData;
	*g_VifRefPktCur = *pPkt;
	g_VifRefPktCur = g_VifRefPktCur + 1;
}

int GetPowerVal(uint val)
{
	int out;

	if (val == 0x400) {
		out = 10;
	}
	else {
		if (val == 0x200) {
			out = 9;
		}
		else {
			if (val == 0x100) {
				out = 8;
			}
			else {
				if (val == 0x80) {
					out = 7;
				}
				else {
					if (val == 0x40) {
						out = 6;
					}
					else {
						if (val == 0x20) {
							out = 5;
						}
						else {
							if (val == 0x10) {
								out = 4;
							}
							else {
								if (val == 8) {
									out = 3;
								}
								else {
									if (val == 4) {
										out = 2;
									}
								}
							}
						}
					}
				}
			}
		}
	}
	return out;
}

bool BOOL_00449578 = false;

edpkt_data* ed3DShadowGenerateTexturePacket(edpkt_data* pPkt)
{
	uint val;
	ed_3D_Scene* peVar1;
	uint uVar2;
	int iVar3;
	int iVar4;

	peVar1 = gShadowRenderScene;

	pPkt->cmdA = ED_VIF1_SET_TAG_CNT(7);
	pPkt->cmdB = 0;
	pPkt->asU32[2] = SCE_VIF1_SET_NOP(0);
	pPkt->asU32[3] = SCE_VIF1_SET_DIRECT(7, 0);

	pPkt[1].cmdA = SCE_GIF_SET_TAG(
		6,				// NLOOP
		SCE_GS_TRUE,	// EOP
		SCE_GS_FALSE,	// PRE
		1,				// PRIM
		SCE_GIF_PACKED,	// FLG
		1				// NREG
	);
	pPkt[1].cmdB = SCE_GIF_PACKED_AD;

	pPkt[2].cmdA = SCE_GS_SET_TEX1(
		0x60,	// LCM
		0,		// MXL
		0,		// MMAG
		0,		//MMIN
		0,		// MTBA
		0,		// L
		0		// K
	);
	pPkt[2].cmdB = SCE_GS_TEX1_1;

	pPkt[3].cmdA = 5;
	pPkt[3].cmdB = 8;

	val = (peVar1->sceneConfig).pShadowConfig.texWidth;
	uVar2 = (uint)gFXBufAddr >> 5;
	iVar3 = GetPowerVal(val);
	iVar4 = GetPowerVal((peVar1->sceneConfig).pShadowConfig.texHeight);
	uint tbp = (uVar2 << 0xd) >> 8;
	pPkt[4].cmdA = SCE_GS_SET_TEX0(
		tbp,			//TBP
		val >> 6,		// TBW
		SCE_GS_PSMCT32,	// PSM
		iVar3,			// TW
		iVar4,			// TH
		1,				// TCC
		1,				// TFX
		0,				// CBP
		0,				// CPSM
		0,				// CSM
		0,				// CSA
		0				// CLD
	);
	pPkt[4].cmdB = SCE_GS_TEX0_1;

	pPkt[5].cmdA = SCE_GS_SET_TEST(
		0,	// ATE 
		0,	// ATST
		0,	// AREF
		0,	// AFAIL
		0,	// DATE
		0,	// DATM
		1,	// ZTE
		2	// ZTST
	);
	pPkt[5].cmdB = SCE_GS_TEST_1;

	const byte alphaFix = (gShadowScene->sceneConfig).pShadowConfig.field_0x22;

	if (BOOL_00449578 == false) {
		pPkt[6].cmdA = SCE_GS_SET_ALPHA_1(
			0,			// A
			1,			// B
			0,			// C
			1,			// D
			alphaFix	// FIX
		);
		pPkt[6].cmdB = SCE_GS_ALPHA_1;
	}
	else {
		pPkt[6].cmdA = SCE_GS_SET_ALPHA_1(
			2,			// A
			2,			// B
			2,			// C
			0,			// D
			alphaFix	// FIX
		);
		pPkt[6].cmdB = SCE_GS_ALPHA_1;
	}
	pPkt[7].cmdA = 0;
	pPkt[7].cmdB = SCE_GS_TEXFLUSH;

	return pPkt + 8;
}


edpkt_data* ed3DFlushStripPacketReceive(edpkt_data* pPkt, edpkt_data* pVifList, uchar param_3, uint flags)
{
	ulong uVar1;
	undefined4 uVar2;
	undefined4 uVar3;
	undefined4 uVar4;
	edpkt_data* peVar5;

	pPkt->cmdA = ED_VIF1_SET_TAG_REF(0, 0);
	pPkt->cmdB = 0;
	pPkt->asU32[2] = SCE_VIF1_SET_BASE((*ed3DVU1Addr_Scratch)[ed3DVU1BufferCur] + 1, 0);
	pPkt->asU32[3] = SCE_VIF1_SET_OFFSET(0, 0);

	pPkt[1] = pVifList[0];
	pPkt[2] = pVifList[1];

	peVar5 = pVifList + 2;
	if ((param_3 == 0) && ((flags & 0x2000000) == 0)) {
		peVar5 = pVifList + 4;
	}

	pPkt[3] = peVar5[0];
	pPkt[4] = peVar5[1];
	pPkt[5] = peVar5[2];
	pPkt[6] = peVar5[3];

	pPkt[7] = g_PKTHeaderRef[4];

	if ((flags & 0x400) == 0) {
		pPkt[8].cmdA = ED_VIF1_SET_TAG_REF(1, &g_stExecuteCode);
		pPkt[8].asU32[2] = SCE_VIF1_SET_ITOP((*ed3DVU1Addr_Scratch)[ed3DVU1BufferCur] + 1, 0);
		pPkt[8].asU32[3] = SCE_VIF1_SET_MSCAL(0x6e, 0);
	}
	else {
		pPkt[8].cmdA = ED_VIF1_SET_TAG_REF(1, &g_stExecuteCode);
		pPkt[8].asU32[2] = SCE_VIF1_SET_ITOP((*ed3DVU1Addr_Scratch)[ed3DVU1BufferCur] + 1, 0);
		pPkt[8].asU32[3] = SCE_VIF1_SET_MSCAL(0x1e8, 0);
	}
	return pPkt + 9;
}

edpkt_data* ed3DFlushStripShadowRender(edNODE* pNode, ed_g2d_material* pmaterial)
{
	byte bVar1;
	ushort headerFlags;
	ed_3d_strip* peVar3;
	undefined8 uVar4;
	AnimScratchpad* pAVar5;
	bool bVar6;
	edpkt_data* peVar7;
	uint uVar8;
	edpkt_data* peVar9;
	undefined4 uVar10;
	undefined4 uVar11;
	edpkt_data* qwc;
	uint meshCount;
	edpkt_data* peVar13;

	peVar7 = ed3DShadowGenerateTexturePacket(g_VifRefPktCur);
	peVar3 = (ed_3d_strip*)pNode->pData;
	peVar13 = (edpkt_data*)((char*)peVar3 + peVar3->vifListOffset);
	g_VifRefPktCur = peVar7;
	uVar8 = ed3DFlushStripGetIncPacket(peVar3, false, true);
	bVar1 = pmaterial->nbLayers;
	headerFlags = pNode->header.typeField.flags;
	meshCount = (uint)(ushort)peVar3->meshCount;
	peVar7 = ed3DFlushStripInit(peVar7, pNode, 1);
	peVar9 = peVar7 + 6;

	PTR_AnimScratchpad_00449554->vuFlags = (int)(pNode->header.typeField.flags | 3);
	PTR_AnimScratchpad_00449554->flags = 0;
	PTR_AnimScratchpad_00449554->field_0x8 = 0;
	PTR_AnimScratchpad_00449554->field_0xc = 0;

	peVar7->cmdA = ED_VIF1_SET_TAG_CNT(1);
	peVar7->asU32[2] = SCE_VIF1_SET_NOP(0);
	peVar7->asU32[3] = SCE_VIF1_SET_UNPACK(0x0076, 0x1, UNPACK_V4_32_MASKED, 0); //0x7c010076
	peVar7[1] = PTR_AnimScratchpad_00449554->pkt;

	peVar7[2].cmdA = ED_VIF1_SET_TAG_CNT(1);
	peVar7[2].asU32[2] = SCE_VIF1_SET_NOP(0);
	peVar7[2].asU32[3] = SCE_VIF1_SET_UNPACK(0x0198, 0x1, UNPACK_V4_32_MASKED, 0); //0x7c01019800000000;
	peVar7[3] = PTR_AnimScratchpad_00449554->pkt;

	peVar7[4].cmdA = ED_VIF1_SET_TAG_CNT(1);
	peVar7[4].asU32[2] = SCE_VIF1_SET_NOP(0);
	peVar7[4].asU32[3] = SCE_VIF1_SET_UNPACK(0x02ba, 0x1, UNPACK_V4_32_MASKED, 0); //0x7c0102ba00000000;
	peVar7[5] = PTR_AnimScratchpad_00449554->pkt;

	if ((edpkt_data*)SCRATCHPAD_ADDRESS(0x70003000) < peVar9 + (uint)(ushort)peVar3->meshCount * 2) {
		IMPLEMENTATION_GUARD(
		qwc = (edpkt_data*)((int)peVar9 - (int)gStartPKT_SPR);
		bVar6 = edDmaLoadFromFastRam(gStartPKT_SPR, (uint)qwc, (uint)((ulong)((long)(int)gBackupPKT << 0x24) >> 0x24));
		peVar7 = (edpkt_data*)(int)bVar6;
		peVar9 = SCRATCHPAD_ADDRESS(0x70001000);
		gStartPKT_SPR = SCRATCHPAD_ADDRESS(70001000);
		gBackupPKT = (edpkt_data*)((int)&gBackupPKT->cmdA + ((uint)qwc & 0xfffffff0));)
	}
	while (bVar6 = meshCount != 0, meshCount = meshCount - 1, bVar6) {
		peVar9 = ed3DFlushStripPacketReceive(peVar9, peVar13, 1 < bVar1, (uint)headerFlags);
		peVar13 = peVar13 + uVar8;
		peVar7 = peVar9;
		if (ed3DVU1BufferCur == 2) {
			ed3DVU1BufferCur = 0;
		}
		else {
			ed3DVU1BufferCur = ed3DVU1BufferCur + 1;
		}
	}
	if (ed3DVU1BufferCur == 2) {
		ed3DVU1Buffer[0] = (*ed3DVU1Addr_Scratch)[1];
		ed3DVU1Buffer[1] = (*ed3DVU1Addr_Scratch)[2];
		ed3DVU1Buffer[2] = (*ed3DVU1Addr_Scratch)[0];
	}
	else {
		if (ed3DVU1BufferCur == 1) {
			ed3DVU1Buffer[0] = (*ed3DVU1Addr_Scratch)[0];
			ed3DVU1Buffer[1] = (*ed3DVU1Addr_Scratch)[1];
			ed3DVU1Buffer[2] = (*ed3DVU1Addr_Scratch)[2];
		}
		else {
			if (ed3DVU1BufferCur == 0) {
				ed3DVU1Buffer[0] = (*ed3DVU1Addr_Scratch)[2];
				ed3DVU1Buffer[1] = (*ed3DVU1Addr_Scratch)[0];
				ed3DVU1Buffer[2] = (*ed3DVU1Addr_Scratch)[1];
			}
		}
	}
	if ((*gShadowRenderMask != 0) && (peVar3->shadowCastFlags != 0)) {
		peVar9->cmdA = ED_VIF1_SET_TAG_REF(0, 0);
		peVar9->cmdB = 0;
		peVar9->asU32[2] = SCE_VIF1_SET_FLUSHA(0);
		peVar9 = peVar9 + 1;
	}
	g_VifRefPktCur = peVar9;
	return peVar7;
}

void ed3DFlushStripList(edLIST* pList, ed_g2d_material* pMaterial)
{
	short listNodeType;
	undefined4* puVar2;
	bool bVar3;
	edpkt_data* peVar4;
	edLIST* pPrevList;

	ED3D_LOG(LogLevel::Verbose, "ed3DFlushStripList");

#ifdef PLATFORM_WIN
	// Do an update now as we swap out our vif ref.
	VU1Emu::UpdateMemory(g_pStartPortPktData, g_VifRefPktCur);
#endif

	pPrevList = (edLIST*)pList->pPrev;

	if (pPrevList != pList) {
		*gMaterialNbPrimSend = *gMaterialNbPrimSend + 1;
	}

	if ((pMaterial == &gMaterial_Render_Zbuffer_Only_Cluster) || (pMaterial == &gMaterial_Render_Zbuffer_Only)) {
		gBackupPKT = (edpkt_data*)0x0;
		if (gShadowFlushMode == 0) {
			if (pMaterial->nbLayers < 2) {
				bVar3 = false;
				while (peVar4 = g_VifRefPktCur, pPrevList != pList) {
					listNodeType = pPrevList->header.typeField.type;

					if (listNodeType == LIST_TYPE_SPRITE) {
						IMPLEMENTATION_GUARD();
					LAB_002a7d54:
						pPrevList = (edLIST*)pPrevList->pPrev;
					}
					else {
						if ((listNodeType == 5) || (listNodeType == 6)) {
							ed3DFlushOptionState(pPrevList);
							goto LAB_002a7d54;
						}

						if (listNodeType == 3) {
							IMPLEMENTATION_GUARD(
								/* WARNING: Load size is inaccurate */
								if ((*pPrevList->pData & 0x10000) == 0) {
									ed3DFlushStripForZbufferOnly((edNODE*)pPrevList, (byte*)pMaterial);
								}
								else {
									bVar3 = true;
									ed3DFlushStrip((MeshTransformSpecial*)pPrevList);
								}
							goto LAB_002a7d54;)
						}

						pPrevList = (edLIST*)pPrevList->pPrev;
					}
				}

				if (bVar3) {
					// #VIF_Tidy
					IMPLEMENTATION_GUARD();
					g_VifRefPktCur->cmdA = ED_VIF1_SET_TAG_REF(0, 0);
					peVar4->cmdB = 0x1100000011000000;
					g_VifRefPktCur = g_VifRefPktCur + 1;
				}
			}
		}
		else {
			while (pPrevList != pList) {
				listNodeType = pPrevList->header.typeField.type;

				if ((listNodeType == 5) || (listNodeType == 6)) {
					ed3DFlushOptionState(pPrevList);
				LAB_002a7c6c:
					pPrevList = (edLIST*)pPrevList->pPrev;
				}
				else {
					if (listNodeType == LIST_TYPE_SPRITE) goto LAB_002a7c6c;

					if (listNodeType == LIST_TYPE_STRIP) {
						ed3DFlushStripShadowRender((edNODE*)pPrevList, pMaterial);
						goto LAB_002a7c6c;
					}

					pPrevList = (edLIST*)pPrevList->pPrev;
				}
			}
		}
	}
	else {
		gStartPKT_SPR = SCRATCHPAD_ADDRESS(0x70001000);
		gBackupPKT = g_VifRefPktCur;
		g_VifRefPktCur = SCRATCHPAD_ADDRESS(0x70001000);

		if (pMaterial->nbLayers < 2) {
			for (; pPrevList != pList; pPrevList = (edLIST*)pPrevList->pPrev) {
				listNodeType = pPrevList->header.typeField.type;
				if ((listNodeType == 5) || (listNodeType == 6)) {
					ed3DFlushOptionState(pPrevList);
				}
				else {
					if (listNodeType == LIST_TYPE_SPRITE) {
						IMPLEMENTATION_GUARD(
							ed3DFlushSprite((MeshTransformSpecial*)pPrevList, pMaterial);)
					}
					else {
						if (listNodeType == LIST_TYPE_STRIP) {
							ed3DFlushStrip(pPrevList);
						}
					}
				}

				// If the size of our packet in scratch pad goes over 0x3000 bytes, copy it straight to gBackupPKT.
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
			for (; pPrevList != pList; pPrevList = (edLIST*)pPrevList->pPrev) {
				listNodeType = pPrevList->header.typeField.type;

				if (listNodeType == LIST_TYPE_SPRITE) {
					IMPLEMENTATION_GUARD(ed3DFlushSprite((MeshTransformSpecial*)pPrevList, pMaterial));
				}
				else {
					if (listNodeType == LIST_TYPE_STRIP) {
						IMPLEMENTATION_GUARD_LOG(ed3DFlushStripMultiTexture((int)pPrevList, (byte*)pMaterial));
					}
				}

				// If the size of our packet in scratch pad goes over 0x3000 bytes, copy it straight to gBackupPKT.
				if (SCRATCHPAD_ADDRESS(0x70003000) < g_VifRefPktCur) {
					uint pktLen = PKT_SIZE(g_VifRefPktCur, gStartPKT_SPR);
					edDmaLoadFromFastRam(gStartPKT_SPR, pktLen, PKT_TO_SCRATCHPAD(gBackupPKT));
					g_VifRefPktCur = SCRATCHPAD_ADDRESS(0x70001000);
					gStartPKT_SPR = SCRATCHPAD_ADDRESS(0x70001000);
					gBackupPKT = (edpkt_data*)((ulong)gBackupPKT + (pktLen & 0xfffffff0));
				}
			}
		}

		// Copy scratchpad data into g_VifRefPktCur.
		uint pktLen = PKT_SIZE(g_VifRefPktCur, gStartPKT_SPR);
		if (pktLen == 0) {
			// No data, just restore the old ptr.
			g_VifRefPktCur = gBackupPKT;
		}
		else {
			edDmaSync(SHELLDMA_CHANNEL_VIF0);
			edDmaLoadFromFastRam_nowait(gStartPKT_SPR, pktLen, PKT_TO_SCRATCHPAD(gBackupPKT));
			g_VifRefPktCur = (edpkt_data*)((ulong)gBackupPKT + (pktLen & 0xfffffff0));
		}
	}

#ifdef PLATFORM_WIN
	g_pStartPortPktData = g_VifRefPktCur;
#endif

	return;
}

ed_g2d_layer* ed3DGetG2DLayer(ed_g2d_material* pMAT, int index)
{
	ed_g2d_layer* pTVar1;

	pTVar1 = (ed_g2d_layer*)0x0;
	if (pMAT != (ed_g2d_material*)0x0) {
		if (pMAT->nbLayers == 0) {
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

void ed3DFlushMatrix(ed_dma_matrix* pDmaMatrix, ed_g2d_material* pMaterial)
{
	byte nbLayers;
	ed_3d_hierarchy* pHierarchy;
	edNODE_MANAGER* pNodeManager;
	bool bFxIsTrue;
	uint curLayerIndex;
	edF32MATRIX4* pObjToWorldPktBuffer;
	edF32MATRIX4* pObjToWorld;
	edF32MATRIX4 worldToCamera;
	edpkt_data* pVifPkt;

	pVifPkt = g_VifRefPktCur;
	pHierarchy = pDmaMatrix->pHierarchy;

	/* Previous mesh may use the same hierarchy, so check if its different here. */
	if ((gpCurHierarchy != pHierarchy) || (pHierarchy == (ed_3d_hierarchy*)0x0)) {
		gpCurHierarchy = pHierarchy;

		if (pHierarchy == (ed_3d_hierarchy*)0x0) {
			gpCurHierarchy = (ed_3d_hierarchy*)0x0000ffff;
		}

		g_VifRefPktCur->cmdA = ED_VIF1_SET_TAG_REF(1, STORE_SECTION(&g_stMatrixHeader));
		g_VifRefPktCur->asU32[2] = SCE_VIF1_SET_FLUSH(0);
		g_VifRefPktCur->asU32[3] = SCE_VIF1_SET_FLUSH(0);

		edpkt_data* pMtxPkt = ed3DPKTAddMatrixPacket(pVifPkt + 1, pDmaMatrix);
		pMtxPkt->cmdA = ED_VIF1_SET_TAG_REF(0, 0);
		pMtxPkt->asU32[2] = SCE_VIF1_SET_FLUSH(0);
		pMtxPkt->asU32[3] = SCE_VIF1_SET_FLUSH(0);
		pVifPkt = pMtxPkt + 1;
	}

	pObjToWorld = pDmaMatrix->pObjToWorld;

#ifdef PLATFORM_WIN
	Renderer::PushModelMatrix(pObjToWorld->raw);
#endif // PLATFORM_WIN

	if (((1 < pMaterial->nbLayers) || (pObjToWorld == &gF32Matrix4Unit)) && (curLayerIndex = 0, gFushListCounter != 0xe))	{
		nbLayers = pMaterial->nbLayers;

		for (; curLayerIndex < nbLayers; curLayerIndex = curLayerIndex + 1) {
			ed_g2d_layer* pLayer = ed3DGetG2DLayer(pMaterial, curLayerIndex);

			if (((pLayer != (ed_g2d_layer*)0x0) && (bFxIsTrue = ed3DFXIsTrue(pLayer), bFxIsTrue != false)) || (pObjToWorld == &gF32Matrix4Unit)) {
				pVifPkt->cmdA = ED_VIF1_SET_TAG_CNT(6);
				pVifPkt->cmdB = 0;
	
				pVifPkt->asU32[2] = SCE_VIF1_SET_NOP(0);
				pVifPkt->asU32[3] = SCE_VIF1_SET_UNPACK(0x03fa, 0x06, UNPACK_V4_32, 0); // 0x6c0603fa;

				worldToCamera = *WorldToCamera_Matrix;

				if (worldToCamera.ac + worldToCamera.aa + worldToCamera.ab != 0.0f) {
					edF32Vector4NormalizeHard(&worldToCamera.rowX, &worldToCamera.rowX);
				}

				if (worldToCamera.bc + worldToCamera.ba + worldToCamera.bb != 0.0f) {
					edF32Vector4NormalizeHard(&worldToCamera.rowY, &worldToCamera.rowY);
				}

				if (worldToCamera.cc + worldToCamera.ca + worldToCamera.cb != 0.0f) {
					edF32Vector4NormalizeHard(&worldToCamera.rowZ, &worldToCamera.rowZ);
				}

				edF32Matrix4GetTransposeHard(&worldToCamera, &worldToCamera);

				worldToCamera.rowX = worldToCamera.rowX * Vector_004489bc;
				worldToCamera.rowY = worldToCamera.rowY * Vector_004489cc;

				pVifPkt[1].asVector = worldToCamera.rowX;
				pVifPkt[2].asVector = worldToCamera.rowY;

				pObjToWorldPktBuffer = (edF32MATRIX4*)(pVifPkt + 3);
				*pObjToWorldPktBuffer = *pObjToWorld;

				pObjToWorldPktBuffer->rowT.x = 0.0f;
				pObjToWorldPktBuffer->rowT.y = 0.0f;
				pObjToWorldPktBuffer->rowT.z = 0.0f;
				pObjToWorldPktBuffer->rowT.w = 1.0f;

				pObjToWorldPktBuffer->rowX.w = 0.0f;
				pObjToWorldPktBuffer->rowY.w = 0.0f;
				pObjToWorldPktBuffer->rowZ.w = 0.0f;

				edF32Matrix4GetInverseOrthoHard(pObjToWorldPktBuffer, pObjToWorldPktBuffer);
				edF32Matrix4GetTransposeHard(pObjToWorldPktBuffer, pObjToWorldPktBuffer);

				// Set obj to world.
				pVifPkt = pVifPkt + 7;
				break;
			}

			nbLayers = pMaterial->nbLayers;
		}
	}

	g_VifRefPktCur = pVifPkt;
	pVifPkt = ed3DHierachyCheckForGlobalAlpha(pVifPkt, gCurLayer);
	g_VifRefPktCur = pVifPkt;
	pVifPkt->cmdA = ED_VIF1_SET_TAG_CNT(1);
	pVifPkt->cmdB = 0;
	pVifPkt->asU32[2] = SCE_VIF1_SET_NOP(0);
	pVifPkt->asU32[3] = SCE_VIF1_SET_UNPACK(0x0021, 0x01, UNPACK_V4_32, 0); //  0x6c010021;
	g_VifRefPktCur = g_VifRefPktCur + 1;

	g_VifRefPktCur->asVector = *gVU1_AnimST_NormalExtruder_Scratch;
	g_VifRefPktCur = g_VifRefPktCur + 1;

	ed3DFlushStripList(pDmaMatrix, pMaterial);

	pNodeManager = (edNODE_MANAGER*)pDmaMatrix->pData;
	pDmaMatrix->pPrev = pNodeManager->pNodeHead + pNodeManager->linkCount;
	pNodeManager->linkCount = pNodeManager->linkCount - pDmaMatrix->nodeCount;
	pDmaMatrix->nodeCount = 0;
	pDmaMatrix->pPrev = (edNODE*)pDmaMatrix;
	pDmaMatrix->pNext = (edNODE*)pDmaMatrix;
	return;
}

void MTXLightFrustum(float left, float right, float bottom, float top, float zNear, float fovY, float param_7, float param_8, edF32MATRIX4* m0, float param_10)
{
	float fVar1;

	fVar1 = 1.0f / (top - bottom);
	m0->aa = fovY * zNear * 2.0f * fVar1;
	m0->ab = 0.0f;
	m0->ac = fovY * fVar1 * (top + bottom) - param_8;
	m0->ad = 0.0f;
	m0->ba = 0.0f;
	fVar1 = 1.0f / (left - right);
	m0->bb = param_7 * zNear * 2.0f * fVar1;
	m0->bc = param_7 * fVar1 * (left + right) - param_10;
	m0->bd = 0.0f;
	m0->ca = 0.0f;
	m0->cb = 0.0f;
	m0->cc = -1.0f;
	m0->cd = 0.0f;
	m0->da = 0.0f;
	m0->db = 0.0f;
	m0->dc = 0.0f;
	m0->dd = 1.0f;
	return;
}

edpkt_data* ed3DShadowManageProjectionSTMtx(edF32MATRIX4* pMtx, edpkt_data* pPkt)
{
	edFCamera* peVar1;
	float* pfVar2;
	ed_3D_Scene* pScene;
	float* pfVar4;
	float* pfVar5;
	float fVar6;
	float fVar7;
	edF32MATRIX4 eStack208;
	edF32MATRIX4 eStack144;
	edF32MATRIX4 eStack80 = {};
	float local_8;
	float local_4;

	pScene = gShadowRenderScene;
	edF32Matrix4MulF32Matrix4Hard(&eStack144, pMtx, &WorldToCamera_Matrix_CastShadow);

	local_8 = (pScene->sceneConfig).nearClip * -1.0f;
	local_4 = (pScene->sceneConfig).farClip * -1.0f;
	peVar1 = pScene->pCamera;
	fVar7 = (peVar1->horizontalHalfFOV * local_8) / peVar1->verticalHalfFOV;
	fVar6 = (peVar1->halfFOV * local_8) / peVar1->verticalHalfFOV;
	MTXLightFrustum(-fVar6, fVar6, -fVar7, fVar7, local_8, 0.5f, 0.5f, 0.5f, &eStack208, 0.5f);
	edF32Matrix4GetTransposeHard(&eStack80, &eStack208);
	edF32Matrix4MulF32Matrix4Hard(&eStack80, &eStack144, &eStack80);
	pPkt->cmdA = ED_VIF1_SET_TAG_CNT(4);
	pPkt->asU32[2] = SCE_VIF1_SET_NOP(0);
	pPkt->asU32[3] = SCE_VIF1_SET_UNPACK(0x03f7, 04, UNPACK_V4_32, 0);
	memcpy(pPkt + 1, &eStack80, sizeof(edF32MATRIX4));
	return pPkt + 5;
}

void ed3DFlushMatrixForShadow(ed_dma_matrix* pDmaMatrix)
{
	edpkt_data* peVar1;

	peVar1 = ed3DShadowManageProjectionSTMtx(pDmaMatrix->pObjToWorld, g_VifRefPktCur);
	
	peVar1->asU32[0] = ED_VIF1_SET_TAG_REF(0, 0);
	peVar1->asU32[1] = SCE_VIF1_SET_NOP(0);
	peVar1->asU32[2] = SCE_VIF1_SET_FLUSH(0);
	peVar1->asU32[3] = SCE_VIF1_SET_FLUSH(0);
	
	peVar1 = ed3DPKTAddMatrixPacket(peVar1 + 1, pDmaMatrix);

	peVar1->asU32[0] = ED_VIF1_SET_TAG_REF(0, 0);
	peVar1->asU32[1] = SCE_VIF1_SET_NOP(0);
	peVar1->asU32[2] = SCE_VIF1_SET_FLUSH(0);
	peVar1->asU32[3] = SCE_VIF1_SET_FLUSH(0);

	g_VifRefPktCur = peVar1 + 1;
	return;
}

void ed3DFlushMaterialNode(ed_dma_material* pMaterial)
{
	edNODE_MANAGER* pNodeManager;
	edNODE* pPrevNode;

	ED3D_LOG(LogLevel::Verbose, "ed3DFlushMaterialNode");

	if ((pMaterial->flags & 1) == 0) {
		pPrevNode = pMaterial->list.pPrev;

		if ((edLIST*)pPrevNode != (edLIST*)0x0) {
			if (gShadowFlushMode == 0) {
				for (; (edLIST*)pPrevNode != &pMaterial->list; pPrevNode = pPrevNode->pPrev) {
					ed3DFlushMatrix((ed_dma_matrix*)pPrevNode->pData, pMaterial->pMaterial);
				}

				pNodeManager = (edNODE_MANAGER*)pMaterial->list.pData;
			}
			else {
				for (; (edLIST*)pPrevNode != &pMaterial->list; pPrevNode = pPrevNode->pPrev) {
					ed3DFlushMatrixForShadow((ed_dma_matrix*)pPrevNode->pData);
					ed3DFlushMatrix((ed_dma_matrix*)pPrevNode->pData, pMaterial->pMaterial);
				}

				pNodeManager = (edNODE_MANAGER*)pMaterial->list.pData;
			}

			pMaterial->list.pPrev = pNodeManager->pNodeHead + pNodeManager->linkCount;
			pNodeManager->linkCount = pNodeManager->linkCount - (pMaterial->list).nodeCount;
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

		pNodeManager = (edNODE_MANAGER*)pMaterial->list.pData;
		pMaterial->list.pPrev = pNodeManager->pNodeHead + pNodeManager->linkCount;
		pNodeManager->linkCount = pNodeManager->linkCount - (pMaterial->list).nodeCount;
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
	ed_Chunck* pTEX;
	edF32VECTOR4* peVar5;
	int iVar6;
	uint uVar7;
	int iVar8;
	uint uVar9;
	float fVar10;
	float fVar11;
	float fVar12;

	uVar9 = gCurLayer->flags_0x4;
	if ((uVar9 & 0x10) == 0) {
		if ((uVar9 & 2) != 0) {
			IMPLEMENTATION_GUARD(
				pTEX = gCurLayer->pTex;
			ed3DG2DGetBitmapFromTexture((int)&pTEX->body);
			iVar6 = *(int*)&(pTEX->body).field_0x1c;
			iVar8 = ed3DG2DAnimTexGet(&pTEX->body);
			fVar10 = *(float*)(iVar8 + 0x10);
			if (fVar10 < 2.147484e+09f) {
				uVar9 = (uint)fVar10;
				uVar3 = *(ushort*)(iVar6 + 0x24);
			}
			else {
				uVar9 = (int)(fVar10 - 2.147484e+09f) | 0x80000000;
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
				((pTEX->body).field_0x14)->w = 0.0;
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
				((pTEX->body).field_0x14)->w = fVar12 * (fVar10 / fVar11);
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
			((pTEX->body).field_0x14)->z = fVar12 * (fVar10 / fVar11);
			pPkt->cmdA = ED_VIF1_SET_TAG_CNT(1);
			pPkt->cmdB = 0;
			*(undefined4*)&pPkt->cmdB = 0;
			*(undefined4*)((int)&pPkt->cmdB + 4) = 0x6c010021;
			gVU1_AnimST_NormalExtruder_Scratch->x = ((pTEX->body).field_0x14)->z;
			gVU1_AnimST_NormalExtruder_Scratch->y = ((pTEX->body).field_0x14)->w;
			*(float*)&pPkt[1].cmdA = gVU1_AnimST_NormalExtruder_Scratch->x;
			*(float*)((int)&pPkt[1].cmdA + 4) = gVU1_AnimST_NormalExtruder_Scratch->y;
			*(float*)&pPkt[1].cmdB = gVU1_AnimST_NormalExtruder_Scratch->z;
			*(float*)((int)&pPkt[1].cmdB + 4) = gVU1_AnimST_NormalExtruder_Scratch->w;
			pPkt = pPkt + 2;
			gCurLayer->flags = gCurLayer->flags | 0x400;)
		}
	}
	else {
		pTEX = (ed_Chunck*)LOAD_SECTION(gCurLayer->pTex);
		if (pTEX != (ed_Chunck*)0xfffffff0) {
			ed_g2d_texture* pTexture = reinterpret_cast<ed_g2d_texture*>(pTEX + 1);
			peVar5 = (edF32VECTOR4*)LOAD_SECTION(pTexture->field_0x14);
			if (((uVar9 & 0x400) == 0) && (gStepTime != 0)) {
				peVar5->z = peVar5->z + peVar5->x;
				peVar5->w = peVar5->w + peVar5->y;
				fVar10 = peVar5->z;
				if (1.0f <= fVar10) {
					peVar5->z = fVar10 - 1.0f;
				}
				fVar10 = peVar5->w;
				if (1.0f <= fVar10) {
					peVar5->w = fVar10 - 1.0f;
				}
				fVar10 = peVar5->z;
				if (fVar10 <= -1.0f) {
					peVar5->z = fVar10 + 1.0f;
				}
				fVar10 = peVar5->w;
				if (fVar10 <= -1.0f) {
					peVar5->w = fVar10 + 1.0f;
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
			gCurLayer->flags_0x4 = gCurLayer->flags_0x4 | 0x400;
		}
	}
	return pPkt;
}

void ed3DFlushMaterialManageGIFPacket(ed_dma_material* pMaterial)
{
	bool bVar1;
	edpkt_data* peVar2;

	ED3D_LOG(LogLevel::Verbose, "ed3DFlushMaterialManageGIFPacket gFushListCounter: 0x{:x} gbFirstTex: {}", gFushListCounter, gbFirstTex);

	if ((pMaterial->flags & 2) != 0) {
		ed3DFXClearALLFXSendedTOVRAM();
		pMaterial->flags = pMaterial->flags & 0xfffffffd;
	}
	peVar2 = g_GifRefPktCur;
	if (gFushListCounter != 0xe) {
		if ((gbFirstTex == 0) && (pMaterial->pBitmap != (ed_g2d_bitmap*)0x0)) {
			if (pMaterial->pMaterial->nbLayers < 2) {
				ED3D_LOG(LogLevel::Verbose, "ed3DFlushMaterialManageGIFPacket Flushing new material gVRAMBufferFlush {}", gVRAMBufferFlush);

				edPSX2Header* pHeader = (edPSX2Header*)LOAD_SECTION(pMaterial->pBitmap->pPSX2);
				edpkt_data* pPkt = (edpkt_data*)(LOAD_SECTION((pHeader + gVRAMBufferFlush)->pPkt));

				g_GifRefPktCur->asU32[0] = SCE_VIF1_SET_DIRECT(0, 0);
				g_GifRefPktCur->asU32[1] = ((ulong)pPkt) + 0x20;
				peVar2->cmdB = 0;
				g_GifRefPktCur = g_GifRefPktCur + 1;
			}
			else {
				IMPLEMENTATION_GUARD_LOG(
					g_GifRefPktCur = ed3DFlushMultiTexture(pMaterial, g_GifRefPktCur, 0, gVRAMBufferFlush);)
			}
		}
		else {
			gbFirstTex = 0;
		}

		// Send even if gbFirstTex is true.
#ifdef PLATFORM_WIN
		if (pMaterial->pBitmap) {
			Renderer::Kya::GetTextureLibrary().BindFromDmaMaterial(pMaterial);
		}
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
	pPkt->asU32[3] = SCE_VIF1_SET_UNPACK(0x03dc, 2, UNPACK_V4_32, 0);

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
			case 0xe: // GIF TAG
			{
				Gif_Tag gifTag;
				gifTag.setTag((u8*)&pkt, false);
				ED3D_LOG(LogLevel::Verbose, "ed3DFlushMaterial - ProcessTextureCommands TAG: {:x} ({:x})", pkt.cmdA, pkt.cmdB);
				ED3D_LOG(LogLevel::Verbose, "ed3DFlushMaterial - ProcessTextureCommands TAG: Loop: {}", gifTag.nLoop);
			}
			break;
			case SCE_GS_TEX0_1:
			{
				ED3D_LOG(LogLevel::Verbose, "ed3DFlushMaterial - ProcessTextureCommands TEX0: {:x} ({:x})", pkt.cmdA, pkt.cmdB);
				SendTextureCommandsFromPacked(pkt.cmdA);
			}
			break;
			case SCE_GS_ALPHA_1:
			{
				ED3D_LOG(LogLevel::Verbose, "ed3DFlushMaterial - ProcessTextureCommands ALPHA: {:x} ({:x})", pkt.cmdA, pkt.cmdB);
				GIFReg::GSAlpha alpha = *reinterpret_cast<GIFReg::GSAlpha*>(&pkt.cmdA);
				Renderer::SetAlpha(alpha.A, alpha.B, alpha.C, alpha.D, alpha.FIX);
			}
			break;
			case SCE_GS_COLCLAMP:
			{
				ED3D_LOG(LogLevel::Verbose, "ed3DFlushMaterial - ProcessTextureCommands COLCLAMP: {:x} ({:x})", pkt.cmdA, pkt.cmdB);
				GIFReg::GSColClamp colClamp = *reinterpret_cast<GIFReg::GSColClamp*>(&pkt.cmdA);
				Renderer::SetColClamp(colClamp);
			}
			break;
			case SCE_GS_TEST_1:
			{
				ED3D_LOG(LogLevel::Verbose, "ed3DFlushMaterial - ProcessTextureCommands TEST: {:x} ({:x})", pkt.cmdA, pkt.cmdB);
				const GIFReg::GSTest test = *reinterpret_cast<GIFReg::GSTest*>(&pkt.cmdA);
				Renderer::SetTest(test);
			}
			break;
			case SCE_GS_TEX1_1:
			{
				ED3D_LOG(LogLevel::Verbose, "ed3DFlushMaterial - ProcessTextureCommands TEX1: {:x} ({:x})", pkt.cmdA, pkt.cmdB);
				
			}
			break;
			case SCE_GS_MIPTBP1_1:
			{
				ED3D_LOG(LogLevel::Verbose, "ed3DFlushMaterial - ProcessTextureCommands MIPTBP1: {:x} ({:x})", pkt.cmdA, pkt.cmdB);
				
			}
			break;
			case SCE_GS_MIPTBP2_1:
			{
				ED3D_LOG(LogLevel::Verbose, "ed3DFlushMaterial - ProcessTextureCommands MIPTBP2: {:x} ({:x})", pkt.cmdA, pkt.cmdB);
				
			}
			break;
			case SCE_GS_CLAMP_1:
			{
				ED3D_LOG(LogLevel::Verbose, "ed3DFlushMaterial - ProcessTextureCommands CLAMP: {:x} ({:x})", pkt.cmdA, pkt.cmdB);
				const GIFReg::GSClamp clamp = *reinterpret_cast<GIFReg::GSClamp*>(&pkt.cmdA);
				Renderer::SetClamp(clamp);
			}
			break;
			default:
			{
				ED3D_LOG(LogLevel::Verbose, "ed3DFlushMaterial - ProcessTextureCommands UNKNOWN ({:x}): {:x} ({:x})", pkt.asU32[2], pkt.cmdA, pkt.cmdB);
			}
			break;
			}
		}
		else {
			// Process as VIF.
			const int opCodeL = (pkt.asU32[2] & 0xff000000) >> 24;
			const int opCodeH = (pkt.asU32[3] & 0xff000000) >> 24;

			auto DoVif = [pkt](int opCode) {
				switch (opCode) {
				case 0x04:
					VU1Emu::SetVifItop(pkt.asU32[2] & 0xffff);
					break;
				}};

			DoVif(opCodeL);
			DoVif(opCodeH);

			ED3D_LOG(LogLevel::Verbose, "ed3DFlushMaterial - ProcessTextureCommands Updating VIF Memory");
			VU1Emu::UpdateMemory(&pkt, &pkt + 1);
		}
	}

	assert(bBeganGIFTag);
}
#endif

void ed3DFlushMaterial(ed_dma_material* pDmaMaterial)
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

	ED3D_LOG(LogLevel::Verbose, "ed3DFlushMaterial pMaterial: 0x{:x} | pBitmap: 0x{:x}", (uintptr_t)pDmaMaterial->pMaterial, (uintptr_t)pDmaMaterial->pBitmap);

	peVar6 = gpPKTDataRefMasPath3;
	peVar5 = g_GifRefPktCur;
	peVar3 = g_VifRefPktCur;
	*gMaterialNbPrimSend = 0;
	peVar4 = g_VifRefPktCur;
	gGlobalAlhaON = 0x80;
	peVar1 = pDmaMaterial->pMaterial;
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

	if (((gFushListCounter != 0xe) && (pDmaMaterial->pBitmap == (ed_g2d_bitmap*)0x0)) &&
		(gpPKTDataRefMasPath3 != (edpkt_data*)0x0)) {
		gpPKTDataRefMasPath3->asU32[0] = SCE_VIF1_SET_STMASK(0);
		gpPKTDataRefMasPath3->asU32[1] = (int)(gpPKTDataRefMasPath3 + 4);
		peVar8->cmdB = 0;
	}

	peVar8 = peVar4;

	if (gpLastMaterial != pDmaMaterial) {
		gpLastMaterial = pDmaMaterial;
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
					(gVRAMBufferFlush * peVar1->commandBufferTextureSize * (uint)peVar1->nbLayers);
#ifdef PLATFORM_WIN
				ProcessTextureCommands(pFlippedPkt, peVar1->commandBufferTextureSize);
#endif
				peVar4[1].cmdA = ED_VIF1_SET_TAG_REF(peVar1->commandBufferTextureSize, STORE_SECTION(pFlippedPkt));
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

	if (pDmaMaterial->pBitmap == (ed_g2d_bitmap*)0x0) {
		gCurLayer = (ed_g2d_layer*)0x0;
		g_VifRefPktCur = ed3DFlushMaterialMipMapOff(peVar8);
	}
	else {
		ed3DG2DMaterialGetLayerBitmap(pDmaMaterial, &gCurBitmap, &gCurLayer, 0);
		ED3D_LOG(LogLevel::Verbose, "ed3DFlushMaterial gCurBitmap: 0x{:x} | gCurLayer: 0x{:x}", (uintptr_t)gCurBitmap, (uintptr_t)gCurLayer);
		*g_pCurFlareMaterial = pDmaMaterial->pMaterial;
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
		if ((gCurLayer->flags_0x4 & 0x80) != 0) {
			iVar9 = 3;

			g_VifRefPktCur[3].cmdA = SCE_GS_SET_TEST(
				0,	// ATE 
				0,	// ATST
				0,	// AREF
				0,	// AFAIL
				0,	// DATE
				0,	// DATM
				1,	// ZTE
				2	// ZTST
			);
			g_VifRefPktCur[3].cmdB = SCE_GS_TEST_1;

			g_VifRefPktCur[4].cmdA = SCE_GS_SET_ALPHA_1(
				2,			// A
				2,			// B
				2,			// C
				0,			// D
				0x80		// FIX
			);
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

	ed3DFlushMaterialNode(pDmaMaterial);

	peVar4 = g_VifRefPktCur;

	if (*gMaterialNbPrimSend == 0) {
		g_VifRefPktCur = peVar3;
		g_GifRefPktCur = peVar5;
		gpPKTDataRefMasPath3 = peVar6;
		pDmaMaterial->pBitmap = (ed_g2d_bitmap*)0xdfdfdfdf;
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
		ed3DFlushMaterialManageGIFPacket(pDmaMaterial);
	}

#ifdef PLATFORM_WIN
	VU1Emu::QueueDraw();
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
	int materialCounter;

	ED3D_LOG(LogLevel::Verbose, "ed3DFlushList");
	ZONE_SCOPED;

	RENDER_LABEL_BEGIN("ed3DFlushList");

	gFlushListFlusaON = 0;
	gFushListCounter = 0;
	do {
		fVar1 = FLOAT_00448a04;
		if (0xe < gFushListCounter) {
			RENDER_LABEL_END("ed3DFlushList");
			return;
		}

		ED3D_LOG(LogLevel::Verbose, "\n-------------");
		ED3D_LOG(LogLevel::Verbose, "ed3DFlushList Processing: {}", gFushListCounter);
		RENDER_LABEL_BEGIN("ed3DFlushList process list %d", gFushListCounter);

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

				// #Debug
				materialCounter = 0;

				while (peVar3 = pPrevList, peVar3 != pCurrentList) {
					ED3D_LOG(LogLevel::Verbose, "");
					ED3D_LOG(LogLevel::Verbose, "ed3DFlushList Material: {}/{}", gFushListCounter, materialCounter++);

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

				// #Debug
				materialCounter = 0;

				for (; pPrevList != pCurrentList; pPrevList = (edLIST*)(pPrevList)->pPrev) {
					ED3D_LOG(LogLevel::Verbose, "");
					ED3D_LOG(LogLevel::Verbose, "ed3DFlushList Material: {}/{}", gFushListCounter, materialCounter++);

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

		RENDER_LABEL_END("ed3DFlushList process list");
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
		pRenderCommand->asU32[2] = SCE_VIF1_SET_FLUSH(0);
		pRenderCommand->asU32[3] = SCE_VIF1_SET_FLUSH(0);
		pRenderCommand[1].cmdA = ED_VIF1_SET_TAG_CNT(2);
		pRenderCommand[1].cmdB = 0;
		pRenderCommand[1].asU32[2] = 0;
		pRenderCommand[1].asU32[3] = SCE_VIF1_SET_DIRECT(2, 0);
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
		pRenderCommand->asU32[2] = SCE_VIF1_SET_FLUSH(0);
		pRenderCommand->asU32[3] = SCE_VIF1_SET_FLUSH(0);
		pRenderCommand[1].cmdA = ED_VIF1_SET_TAG_CNT(2);
		pRenderCommand[1].cmdB = 0;
		pRenderCommand[1].asU32[2] = SCE_VIF1_SET_NOP(0);
		pRenderCommand[1].asU32[3] = SCE_VIF1_SET_DIRECT(2, 0);
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

edpkt_data* ed3DShadowFlushResetOffset(edpkt_data* pPkt, edRECT16* pRect)
{
	uint wPow2;
	uint hPow2;

	// VIF Header
	pPkt->cmdA = ED_VIF1_SET_TAG_CNT(3);
	pPkt->asU32[2] = SCE_VIF1_SET_NOP(0);
	pPkt->asU32[3] = SCE_VIF1_SET_DIRECT(3, 0);

	// TAG
	pPkt[1].cmdA = SCE_GIF_SET_TAG(
		2,				// NLOOP
		SCE_GS_TRUE,	// EOP
		SCE_GS_FALSE,	// PRE
		1,				// PRIM
		SCE_GIF_PACKED,	// FLG
		1				// NREG
	);
	pPkt[1].cmdB = SCE_GIF_PACKED_AD;

	// OFFSET
	pPkt[2].cmdA = SCE_GS_SET_XYOFFSET(
		(((0x1000 - ((int)pRect->w + (int)pRect->x) >> 1) - ((int)pRect->x >> 1)) * 0x10) & 0xffffffffU,	// X
		(((0x1000 - ((int)pRect->h + (int)pRect->y) >> 1) - ((int)pRect->y >> 1)) * 0x10)					// Y
	);
	pPkt[2].cmdB = SCE_GS_XYOFFSET_1;

	wPow2 = GetGreaterPower2Val((int)pRect->w);
	hPow2 = GetGreaterPower2Val((int)pRect->h);

	// SCISSOR
	pPkt[3].cmdA = SCE_GS_SET_SCISSOR(
		pRect->x,								// SCAX0
		((int)wPow2 + (int)pRect->x + -1),	// SCAX1
		pRect->y,								// SCAY0
		((int)hPow2 + (int)pRect->y + -1)	// SCAY1
	);
	pPkt[3].cmdB = SCE_GS_SCISSOR_1;

	return pPkt + 4;
}

void ed3DFlushShadowList(void)
{
	bool bVar1;
	ed_g2d_material** ppeVar2;
	int iVar3;
	ed_dma_material* pDMA_Material;
	edpkt_data* peVar4;
	edLIST* peVar5;
	ed_g2d_bitmap* peVar6;
	edNODE* peVar7;
	edNODE_MANAGER* pvVar1;
	ed_dma_material* pShadowListData;

	bVar1 = false;
	g_VifRefPktCur = ed3DFlushFullAlphaInit(g_VifRefPktCur);
	gFlushListFlusaON = 0;
	gShadowFlushMode = 1;
	pShadowListData = (ed_dma_material*)(gPrim_List_Shadow[gCurRenderList].pPrev)->pPrev->pData;

	if ((edLIST*)(pShadowListData->list).pPrev != &pShadowListData->list) {

		memcpy(CameraToScreen_Matrix, &gShadow_CameraToScreen_Matrix, sizeof(edF32MATRIX4));
		memcpy(CameraToCulling_Matrix, &gShadow_CameraToCulling_Matrix, sizeof(edF32MATRIX4));
		memcpy(CameraToClipping_Matrix, &gShadow_CameraToClipping_Matrix, sizeof(edF32MATRIX4));
		memcpy(WorldToCamera_Matrix, &gShadow_WorldToCamera_Matrix, sizeof(edF32MATRIX4));
		memcpy(&gClipMulVector, &gShadow_ClipMulVector, sizeof(edF32VECTOR4));
		memcpy(&gClipAddVector, &gShadow_ClipAddVector, sizeof(edF32VECTOR4));
		memcpy(&gClipXY, &gShadow_ClipXY, sizeof(edF32VECTOR4));
		memcpy(&gCamPos, &gShadow_CamPos, sizeof(edF32VECTOR4));
		memcpy(&gCamNormal_X, &gShadow_gCamNormal_X, sizeof(edF32VECTOR4));
		memcpy(&gCamNormal_Y, &gShadow_gCamNormal_Y, sizeof(edF32VECTOR4));

		peVar4 = g_VifRefPktCur;
		g_VifRefPktCur->asU32[0] = ED_VIF1_SET_TAG_REF(0, 0);
		g_VifRefPktCur->asU32[1] = SCE_VIF1_SET_NOP(0);
		g_VifRefPktCur->asU32[2] = SCE_VIF1_SET_NOP(0);
		g_VifRefPktCur->asU32[3] = SCE_VIF1_SET_FLUSHA(0);
		g_VifRefPktCur = g_VifRefPktCur + 1;

		IMPLEMENTATION_GUARD_LOG();
		//g_VifRefPktCur = ed3DJitterShadow(g_VifRefPktCur);
		g_VifRefPktCur = ed3DDMAGenerateGlobalPacket(g_VifRefPktCur);
		g_VifRefPktCur = ed3DAddViewportContextPacket(gShadowRenderViewport, g_VifRefPktCur);
		peVar4 = ed3DShadowFlushResetOffset(g_VifRefPktCur, &gCurRectViewport);
		g_VifRefPktCur = peVar4;

		peVar4->cmdA = ED_VIF1_SET_TAG_CNT(2);
		peVar4->asU32[2] = SCE_VIF1_SET_NOP(0);
		peVar4->asU32[3] = SCE_VIF1_SET_DIRECT(2, 0);

		peVar4 = g_VifRefPktCur;
		g_VifRefPktCur = g_VifRefPktCur + 1;
		g_VifRefPktCur->cmdA = SCE_GIF_SET_TAG(
			1,				// NLOOP
			SCE_GS_TRUE,	// EOP
			SCE_GS_FALSE,	// PRE
			1,				// PRIM
			SCE_GIF_PACKED, // FLG
			1				// NREG
		);
		peVar4[1].cmdB = SCE_GIF_PACKED_AD;

		peVar4 = g_VifRefPktCur;
		g_VifRefPktCur = g_VifRefPktCur + 1;
		g_VifRefPktCur->cmdA = SCE_GS_SET_ZBUF(
			gCurScene->pViewport->pZBuffer->frameBasePtr,	// ZBP
			SCE_GS_PSMZ32,									// PSM
			SCE_GS_TRUE										// ZMASK
		);

		peVar4[1].cmdB = SCE_GS_ZBUF_1;
		g_VifRefPktCur = g_VifRefPktCur + 1;

		ed3DFlushMaterialNode(pShadowListData);
		peVar4 = g_VifRefPktCur;

		g_VifRefPktCur->asU32[0] = ED_VIF1_SET_TAG_REF(0, 0);
		g_VifRefPktCur->asU32[1] = SCE_VIF1_SET_NOP(0);
		g_VifRefPktCur->asU32[2] = SCE_VIF1_SET_NOP(0);
		g_VifRefPktCur->asU32[3] = SCE_VIF1_SET_FLUSHA(0);

		iVar3 = gCurRenderList;
		g_VifRefPktCur = g_VifRefPktCur + 1;
		gPrim_List_FlushTex[gCurRenderList].nodeCount = gPrim_List_FlushTex[gCurRenderList].nodeCount + 1;
		bVar1 = true;
		if (gPrim_List_FlushTex_Last == (edNODE*)0x0) {
			gPrim_List_FlushTex_Last = (gPrim_List_Shadow[iVar3].pPrev)->pPrev;
		}
		else {
			gPrim_List_FlushTex_Last->pPrev = (gPrim_List_Shadow[iVar3].pPrev)->pPrev;
			gPrim_List_FlushTex_Last = gPrim_List_FlushTex_Last->pPrev;
		}
		(gPrim_List_Shadow[gCurRenderList].pPrev)->pPrev = (edNODE*)(gPrim_List_Shadow + gCurRenderList);
		pDMA_Material = DmaMaterialBufferCurrent;
		peVar5 = &DmaMaterialBufferCurrent->list;
		ppeVar2 = &DmaMaterialBufferCurrent->pMaterial;
		DmaMaterialBufferCurrent = DmaMaterialBufferCurrent + 1;
		*ppeVar2 = &gMaterial_Render_Zbuffer_Only;
		pDMA_Material->field_0x4 = 0;
		pDMA_Material->flags = pDMA_Material->flags & 0xfffffffe;
		(pDMA_Material->list).pNext = (edNODE*)peVar5;
		(pDMA_Material->list).pPrev = (edNODE*)peVar5;
		(pDMA_Material->list).pData = gNodeDmaMatrix;
		(pDMA_Material->list).field_0x10 = 0;
		(pDMA_Material->list).nodeCount = 0;
		(pDMA_Material->list).header.mode = 0;
		peVar6 = ed3DGetG2DBitmap(pDMA_Material, 0);
		pShadowListData->pBitmap = peVar6;
		iVar3 = gCurRenderList;
		peVar5 = gPrim_List_Shadow + gCurRenderList;
		pvVar1 = (edNODE_MANAGER*)gPrim_List_Shadow[gCurRenderList].pData;
		peVar7 = pvVar1->pNodeHead + pvVar1->linkCount;
		peVar7->pData = pDMA_Material;
		*(undefined2*)&peVar7->header = 1;
		pvVar1->linkCount = pvVar1->linkCount + 1;
		gPrim_List_Shadow[iVar3].nodeCount = gPrim_List_Shadow[iVar3].nodeCount + 1;
		peVar7->pPrev = (edNODE*)peVar5;
		(gPrim_List_Shadow[iVar3].pPrev)->pPrev = peVar7;
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

#define HASH_CODE_MAT 0x2e54414d

ed_g2d_material* ed3DG2DGetG2DMaterialFromIndex(ed_hash_code* pMBNK, int index)
{
	ed_Chunck* pChunk;
	ed_g2d_material* pMaterial;
	pChunk = LOAD_SECTION_CAST(ed_Chunck*, pMBNK[index].pData);

	pMaterial = (ed_g2d_material*)0x0;
	if ((pChunk != (ed_Chunck*)0x0) && (pMaterial = reinterpret_cast<ed_g2d_material*>(pChunk + 1), pChunk->hash != HASH_CODE_MAT)) {
		// Header didn't match MAT, so must actually be a hash code.
		ed_hash_code* pHashCode = reinterpret_cast<ed_hash_code*>(pChunk);
		pChunk = LOAD_SECTION_CAST(ed_Chunck*, pHashCode->pData);

		// Check we definitely found a material.
		assert(pChunk->hash == HASH_CODE_MAT);

		pMaterial = reinterpret_cast<ed_g2d_material*>(pChunk + 1);
	}

	return pMaterial;
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

	pfVar8 = (ed_Bound_Sphere_packet*)LOAD_SECTION(param_1->pBoundSpherePkt);
	if (pfVar8 == (ed_Bound_Sphere_packet*)0x0) {
		bVar7 = true;
		param_1->flags = param_1->flags | 3;
	}
	else {
		uVar12 = (uint)(ushort)param_1->meshCount;
		uVar10 = 0;
		param_1->pDMA_Matrix.pDMA_Matrix = (int)STORE_SECTION(ed3D_SubAllocator_004491b8.current);
		pRVar11 = (ed_dma_matrix*)LOAD_SECTION(param_1->pDMA_Matrix.pDMA_Matrix);
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
		bVar7 = uVar9 != (ushort)param_1->meshCount;
	}
	return bVar7;
}

void ed3DLinkStripManageLinkToDMALink(edLIST* pList, ed_dma_material* pMaterial, short param_3)
{
	ed_dma_material* pOtherMaterial;
	edNODE_MANAGER* peVar1;
	edNODE* peVar2;
	edLIST* peVar3;
	edLIST* peVar4;

	peVar4 = (edLIST*)pList->pPrev;
	pOtherMaterial = (ed_dma_material*)peVar4->pData;
	peVar3 = pList;

	while ((peVar4 != pList && (0.0f <= pMaterial->field_0x4 - pOtherMaterial->field_0x4))) {
		peVar4 = (edLIST*)peVar4->pPrev;
		peVar3 = (edLIST*)peVar3->pPrev;
		pOtherMaterial = (ed_dma_material*)peVar4->pData;
	}

	peVar1 = (edNODE_MANAGER*)pList->pData;
	peVar2 = peVar1->pNodeHead + peVar1->linkCount;
	peVar2->pData = pMaterial;
	peVar1->linkCount = peVar1->linkCount + 1;
	pList->nodeCount = pList->nodeCount + 1;
	peVar2->header.typeField.type = param_3;
	peVar2->pPrev = (edNODE*)peVar4;
	peVar3->pPrev = peVar2;
	return;
}

void ed3DManageAnim(ed_g3d_Anim_def* pAnimDef)
{
	uint* puVar1;
	uint* puVar2;
	uint uVar3;
	ulong uVar4;
	uint uVar5;
	uint uVar6;
	uint uVar7;
	uint* puVar8;
	float fVar9;
	float fVar10;

	uVar4 = pAnimDef->field_0x4;
	if (gCurFrame == pAnimDef->field_0x4 >> 0x1a) {
		return;
	}

	if (((uVar4 & 4) == 0) && ((uVar4 & 8) == 0)) {
		return;
	}

	pAnimDef->field_0x4 = (uint)((uVar4 << 0x26) >> 0x26) | gCurFrame << 0x1a;

	if ((pAnimDef->field_0x4 & 0x20) == 0) {
		if (pAnimDef->field_0x14 == 0) {
			pAnimDef->field_0x14 = gCurTime;
		}
		pAnimDef->field_0x18 = gCurTime;
	}
	else {
		pAnimDef->field_0x18 = pAnimDef->field_0x18 + pAnimDef->field_0xc;
	}

	puVar1 = &pAnimDef->field_0x20;
	uVar6 = pAnimDef->field_0x4;
	uVar3 = pAnimDef->field_0x18 - pAnimDef->field_0x14;
	uVar5 = puVar1[(pAnimDef->field_0x0 - 1) * 4];

	if ((uVar6 & 8) == 0) {
		if (uVar5 < uVar3) {
			uVar3 = (int)uVar3 % (int)uVar5;
			if (uVar5 == 0) {
				trap(7);
			}
			if ((uVar6 & 2) != 0) {
				pAnimDef->field_0x4 = pAnimDef->field_0x4 | 8;
				pAnimDef->field_0x4 = pAnimDef->field_0x4 & 0xfffffffb;
				pAnimDef->field_0x14 = gCurTime - uVar3;
				goto LAB_0029edbc;
			}
			if ((uVar6 & 1) == 0) {
				uVar3 = uVar5;
			}
		}
	LAB_0029ef30:
		uVar6 = *puVar1;
		for (uVar5 = 0; (uVar6 < uVar3 && (uVar5 != pAnimDef->field_0x0)); uVar5 = uVar5 + 1) {
			puVar1 = puVar1 + 4;
			uVar6 = *puVar1;
		}
		pAnimDef->field_0x8 = uVar5;
		if (uVar5 == pAnimDef->field_0x0) {
			puVar2 = puVar1 + -4;
			puVar1 = puVar1 + -8;
		}
		else {
			if (uVar5 == 0) {
				puVar2 = puVar1 + 4;
			}
			else {
				puVar2 = puVar1;
				puVar1 = puVar1 + -4;
			}
		}
		uVar3 = uVar3 - *puVar1;
		if ((int)uVar3 < 0) {
			fVar9 = (float)(uVar3 >> 1 | uVar3 & 1);
			fVar9 = fVar9 + fVar9;
		}
		else {
			fVar9 = (float)uVar3;
		}
		uVar3 = *puVar2 - *puVar1;
		if ((int)uVar3 < 0) {
			fVar10 = (float)(uVar3 >> 1 | uVar3 & 1);
			fVar10 = fVar10 + fVar10;
		}
		else {
			fVar10 = (float)uVar3;
		}
		pAnimDef->field_0x10 = (float)puVar1[1] + (fVar9 / fVar10) * ((float)puVar2[1] - (float)puVar1[1]);
		if ((pAnimDef->field_0x8 == pAnimDef->field_0x0) &&
			((uVar3 = pAnimDef->field_0x4, (uVar3 & 1) != 0 || ((uVar3 & 2) != 0)))) {
			if ((uVar3 & 2) == 0) {
				pAnimDef->field_0x8 = 0;
			}
			else {
				pAnimDef->field_0x4 = pAnimDef->field_0x4 | 8;
			}
			pAnimDef->field_0x14 = 0;
			pAnimDef->field_0x18 = 0;
		}
	}
	else {
		if (uVar5 < uVar3) {
			uVar3 = (int)uVar3 % (int)uVar5;
			if (uVar5 == 0) {
				trap(7);
			}
			if ((uVar6 & 2) != 0) {
				pAnimDef->field_0x4 = pAnimDef->field_0x4 & 0xfffffff7;
				pAnimDef->field_0x4 = pAnimDef->field_0x4 | 4;
				pAnimDef->field_0x14 = gCurTime - uVar3;
				goto LAB_0029ef30;
			}
			if ((uVar6 & 1) == 0) {
				uVar3 = uVar5;
			}
		}
	LAB_0029edbc:
		uVar6 = pAnimDef->field_0x0;
		puVar2 = puVar1 + (uVar6 - 1) * 4;
		for (uVar7 = 0; (uVar5 - *puVar2 < uVar3 && (uVar7 != uVar6)); uVar7 = uVar7 + 1) {
			puVar2 = puVar2 + -4;
		}
		pAnimDef->field_0x8 = uVar6 - uVar7;
		puVar8 = puVar2 + 4;
		if (pAnimDef->field_0x8 == 0) {
			puVar8 = &pAnimDef->field_0x30;
			puVar2 = puVar1;
		}
		else {
			uVar6 = pAnimDef->field_0x0 - 1;
			if (uVar6 <= pAnimDef->field_0x8) {
				puVar8 = puVar1 + uVar6 * 4;
				puVar2 = puVar8 + -4;
			}
		}
		uVar3 = uVar3 - (uVar5 - *puVar8);
		if ((int)uVar3 < 0) {
			fVar9 = (float)(uVar3 >> 1 | uVar3 & 1);
			fVar9 = fVar9 + fVar9;
		}
		else {
			fVar9 = (float)uVar3;
		}
		uVar3 = *puVar8 - *puVar2;
		if ((int)uVar3 < 0) {
			fVar10 = (float)(uVar3 >> 1 | uVar3 & 1);
			fVar10 = fVar10 + fVar10;
		}
		else {
			fVar10 = (float)uVar3;
		}
		pAnimDef->field_0x10 = (float)puVar8[1] - (fVar9 / fVar10) * ((float)puVar8[1] - (float)puVar2[1]);
	}

	if ((pAnimDef->field_0x4 & 0x10) != 0) {
		fVar9 = pAnimDef->field_0x10;
		if (fVar9 < 2.147484e+09f) {
			uVar3 = (uint)fVar9;
		}
		else {
			uVar3 = (int)(fVar9 - 2.147484e+09f) | 0x80000000;
		}
		if ((int)uVar3 < 0) {
			fVar9 = (float)(uVar3 >> 1 | uVar3 & 1);
			fVar9 = fVar9 + fVar9;
		}
		else {
			fVar9 = (float)uVar3;
		}

		pAnimDef->field_0x10 = fVar9;
	}
	return;
}

void ed3DLinkClusterStripToViewport(ed_3d_strip* pStrip, ed_hash_code* pMBNK)
{
	edF32MATRIX4* peVar1;
	int iVar4;
	ed_hash_code* pTextureHashCode;
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
	uint layerFlagsB;
	uint layerFlagsA;
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
	ed_Chunck* pLAY;
	edNODE_MANAGER* pNodeManager;
	ed_Chunck* pTEX;

#ifdef PLATFORM_WIN
	if (ed3D::DebugOptions::gDisableClusterRendering) {
		return;
	}
#endif

	ED3D_LOG(LogLevel::Verbose, "ed3DLinkClusterStripToViewport materialIndex: {}", pStrip->materialIndex);

	if (pStrip->materialIndex == -1) {
		pStripMaterial = (ed_g2d_material*)0x0;
	}
	else {
		pStripMaterial = ed3DG2DGetG2DMaterialFromIndex(pMBNK, pStrip->materialIndex);
		ED3D_LOG(LogLevel::Verbose, "ed3DLinkClusterStripToViewport Found: {}", pMBNK[pStrip->materialIndex].hash.ToString());

		if ((pStripMaterial != (ed_g2d_material*)0x0) && ((pStripMaterial->flags & 1) != 0)) {
			return;
		}
	}

	pStrip->pDMA_Matrix.pDMA_Matrix = 0x0;
	peVar9 = gBoundSphereCenter;
	if (gRender_info_SPR->boundingSphereTestResult == 1) {
		pStrip->flags = pStrip->flags & 0xfffffffc;
	}
	else {
		gBoundSphereCenter->xyz = pStrip->boundingSphere.xyz;
		gBoundSphereCenter->w = 1.0f;
		*gBoundSphereCenter = (*gBoundSphereCenter) * (*gRender_info_SPR->pMeshTransformMatrix);

		EVar10 = ed3DTestBoundingSphere(&pStrip->boundingSphere);

		if (EVar10 == VM_4) {
			return;
		}
		if (EVar10 != VM_1) {
			pStrip->flags = pStrip->flags | 3;
			if ((pStrip->flags & 0x1000) == 0) {
				EVar10 = ed3DTestBoundingSphereZNearOnly(&pStrip->boundingSphere);
				if (EVar10 == VM_1) {
					pStrip->flags = pStrip->flags & 0xfffffffc;
					EVar10 = ed3DTestBoundingSphereSide(&pStrip->boundingSphere);
					if (EVar10 == VM_1) goto LAB_00297680;
					pStrip->flags = pStrip->flags | 1;
					lVar12 = ed3DCheckSpherePacket(pStrip);
				}
				else {
					lVar12 = ed3DCheckSpherePacket(pStrip);
				}
				if (lVar12 == 0) {
					return;
				}
			}
			else {
				pStrip->flags = pStrip->flags & 0xfffffffd;
			}
		}
	}
LAB_00297680:
	if ((pStripMaterial != (ed_g2d_material*)0x0) && (pStripMaterial->nbLayers != 0)) {
		ED3D_LOG(LogLevel::Verbose, "ed3DLinkClusterStripToViewport materialIndex: {}", pStrip->materialIndex);

		pLAY = LOAD_SECTION_CAST(ed_Chunck*, pStripMaterial->aLayers[0]);
		ed_g2d_layer* pLayer = reinterpret_cast<ed_g2d_layer*>(pLAY + 1);
		layerFlagsA = pLayer->flags_0x0;
		layerFlagsB = pLayer->flags_0x4;
		pBitmap = (ed_g2d_bitmap*)0x0;

		if (pLayer->bHasTexture != 0) {
			pTEX = LOAD_SECTION_CAST(ed_Chunck*, pLayer->pTex);
			ed_g2d_texture* pTexture = reinterpret_cast<ed_g2d_texture*>(pTEX + 1);

			pLayer->flags_0x4 = layerFlagsB & 0xfffffbff;
			peVar9 = gBoundSphereCenter;

			if ((layerFlagsB & 0x10) == 0) {
			LAB_00297798:
				if ((layerFlagsB & 2) == 0) {
					pStrip->flags = pStrip->flags & 0xfffffdff;
				}
				else {
					IMPLEMENTATION_GUARD(
						peVar11 = (ed_g3d_Anim_def*)ed3DG2DAnimTexGet(pTexture);
					ed3DManageAnim(peVar11);
					pStrip->flags = pStrip->flags | 0x200;)
				}
			}
			else {
				gBoundSphereCenter->xyz = pStrip->boundingSphere.xyz;
				gBoundSphereCenter->w = 1.0f;
				*gBoundSphereCenter = (*gBoundSphereCenter) * (*gRender_info_SPR->pMeshTransformMatrix);
				fVar19 = pTexture->field_0x18;

				if (fVar19 <= 0.0f) {
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

				pStrip->flags = pStrip->flags | 0x200;
			}

			if (pTexture->bHasPalette == 0) {
				pTextureHashCode = LOAD_SECTION_CAST(ed_hash_code*, pTexture->hashCode.pData);

				if (pTextureHashCode != (ed_hash_code*)0x0) {
					ed_Chunck* pT2D = LOAD_SECTION_CAST(ed_Chunck*, pTextureHashCode->pData);
					pBitmap = reinterpret_cast<ed_g2d_bitmap*>(pT2D + 1);
				}
			}
			else {
				ed_hash_code* pAfterHash = (ed_hash_code*)(pTexture + 1);
				iVar4 = pAfterHash[pLayer->paletteId].pData;
				if (iVar4 != 0) {
					ed_hash_code* pPaletteHashCode = (ed_hash_code*)LOAD_SECTION(iVar4);
					ed_Chunck* pT2D = LOAD_SECTION_CAST(ed_Chunck*, pPaletteHashCode->pData);
					pBitmap = reinterpret_cast<ed_g2d_bitmap*>(pT2D + 1);
				}
			}

			if ((pStrip->flags & 4) != 0) {
				ed_g3d_Anim_def* pAnimDef = ed3DG3DAnimGet(pStrip);
				ed3DManageAnim(pAnimDef);
			}

			goto LAB_00297870;
		}

		if ((layerFlagsB & 0x800) != 0) goto LAB_00297870;
	}

	pBitmap = (ed_g2d_bitmap*)0x0;
	layerFlagsA = 0;
	layerFlagsB = 0;
	pStripMaterial = gDefault_Material_Cluster_Current;
LAB_00297870:
	pDMA_Material = DmaMaterialBufferCurrent;
	if ((((layerFlagsA & 0xfc) == 0) || ((layerFlagsA & 0x80000000) != 0)) || ((layerFlagsA & 0x4000) != 0)) {
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

			ED3D_LOG(LogLevel::Verbose, "ed3DLinkClusterStripToViewport pMaterial: {:x} | pBitmap: {:x}", (uintptr_t)pStripMaterial, (uintptr_t)pBitmap);

			if (pStrip->primListIndex == 0) {
				if ((layerFlagsB & 0x40) == 0) {
					if ((layerFlagsB & 0x80) == 0) {
						if ((layerFlagsA & 0x80000000) == 0) {
							if ((layerFlagsA & 0x4000) == 0) {
								if (((layerFlagsA & 0x100) == 0) && ((layerFlagsA & 0xfc) == 0)) {
									pStrip->primListIndex = PRIM_LIST_0;
								}
								else {
									pStrip->primListIndex = PRIM_LIST_2;
								}
							}
							else {
								pStrip->primListIndex = PRIM_LIST_4;
							}
						}
						else {
							pStrip->primListIndex = PRIM_LIST_5;
						}
					}
					else {
						pStrip->primListIndex = PRIM_LIST_B;
					}
				}
				else {
					if ((layerFlagsA & 0x80000000) == 0) {
						if ((layerFlagsA & 0x4000) == 0) {
							if ((layerFlagsA & 0xfc) == 0) {
								if ((layerFlagsA & 0x100) == 0) {
									pStrip->primListIndex = PRIM_LIST_1;
								}
								else {
									pStrip->primListIndex = PRIM_LIST_2;
								}
							}
							else {
								pStrip->primListIndex = PRIM_LIST_3;
							}
						}
						else {
							pStrip->primListIndex = PRIM_LIST_4;
						}
					}
					else {
						pStrip->primListIndex = PRIM_LIST_5;
					}
				}
			}
			pNodeManager = (edNODE_MANAGER*)gPrim_List[pStrip->primListIndex][gCurRenderList].pData;
			pNode = pNodeManager->pNodeHead + pNodeManager->linkCount;
			pNode->pData = pDMA_Material;
			pNode->header.typeField.type = LIST_TYPE_MATERIAL;
			pNodeManager->linkCount = pNodeManager->linkCount + 1;
			ppNodeCount = &gPrim_List[pStrip->primListIndex][gCurRenderList].nodeCount;
			*ppNodeCount = *ppNodeCount + 1;
			pNode->pPrev = gPrim_List[pStrip->primListIndex][gCurRenderList].pPrev;
			gPrim_List[pStrip->primListIndex][gCurRenderList].pPrev = pNode;
			pExistingDMA_Material = pDMA_Material;

			ED3D_LOG(LogLevel::Verbose, "ed3DLinkClusterStripToViewport Linked Cluster Strip: ({}, {}). New node count: {}", pStrip->primListIndex, gCurRenderList,
				gPrim_List[pStrip->primListIndex][gCurRenderList].nodeCount);
		}
		pDVar14 = (edNODE_MANAGER*)(pExistingDMA_Material->list).pData;
	}
	else {
		ED3D_LOG(LogLevel::Verbose, "ed3DLinkClusterStripToViewport pMaterial: {:x} | pBitmap: {:x}", (uintptr_t)pStripMaterial, (uintptr_t)pBitmap);

		if (gRender_info_SPR->boundingSphereTestResult == 1) {
			gBoundSphereCenter->xyz = (pStrip->boundingSphere).xyz;
			gBoundSphereCenter->w = 1.0f;
			edF32Matrix4MulF32Vector4Hard(gBoundSphereCenter, WorldToCamera_Matrix, gBoundSphereCenter);
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
		ed3DLinkStripManageLinkToDMALink(gPrim_List[3] + gCurRenderList, pExistingDMA_Material, 1);
		pDVar14 = (edNODE_MANAGER*)(pExistingDMA_Material->list).pData;

		ED3D_LOG(LogLevel::Verbose, "ed3DLinkClusterStripToViewport Linked Cluster Strip: ({}, {}). New node count: {}", pStrip->primListIndex, gCurRenderList,
			gPrim_List[pStrip->primListIndex][gCurRenderList].nodeCount);
	}

	pNode = pDVar14->pNodeHead + pDVar14->linkCount;
	pNode->pData = pStrip;
	pNode->header.typeField.type = LIST_TYPE_STRIP;
	pNode->header.typeField.flags = (short)pStrip->flags;
	pDVar14->linkCount = pDVar14->linkCount + 1;
	(pExistingDMA_Material->list).nodeCount = (pExistingDMA_Material->list).nodeCount + 1;
	pNode->pPrev = (pExistingDMA_Material->list).pPrev;
	pNode->pNext = (edNODE*)LOAD_SECTION(pStrip->pDMA_Matrix.pDMA_Matrix);
	(pExistingDMA_Material->list).pPrev = pNode;
	return;
}

edF32VECTOR4* ed3DGetHierarchyFirstLODSphere(ed_g3d_hierarchy* pHier)
{
	ed_hash_code* pLodHash;
	edF32VECTOR4* pOutSphere;
	int lodIndex;
	ed3DLod* pLod;

	pLod = pHier->aLods;
	for (lodIndex = 0; (pLodHash = LOAD_SECTION_CAST(ed_hash_code*, pLod->pObj), pLodHash == (ed_hash_code*)0x0 && (lodIndex < pHier->lodCount)); lodIndex = lodIndex + 1) {
		pLod = pLod + 1;
	}

	pOutSphere = (edF32VECTOR4*)0x0;
	if (pLodHash != (ed_hash_code*)0x0) {
		ed_Chunck* pOBJ = LOAD_SECTION_CAST(ed_Chunck*, pLodHash->pData);
		ed_g3d_object* pObjInternal = reinterpret_cast<ed_g3d_object*>(pOBJ + 1);
		pOutSphere = &pObjInternal->boundingSphere;
	}

	return pOutSphere;
}

edF32VECTOR4 center$1208 = { 0.0f, 0.0f, 0.0f, 1.0f };

ed3DLod* ed3DClusterChooseGoodLOD(ed_g3d_hierarchy* pHier, edF32VECTOR4* pSphere)
{
	float* pLodBiases;
	ed3DLod* pLod;
	uint lodIndex;
	float distanceFactor;
	edF32VECTOR4 cameraSpaceSphere;

	ED3D_LOG(LogLevel::Verbose, "ed3DClusterChooseGoodLOD LOD count: {} location: {}", pHier->lodCount, pSphere->ToString());

	if (pHier->lodCount == 0) {
		pLod = (ed3DLod*)0x0;
		pHier->desiredLod = 0xff;
		ED3D_LOG(LogLevel::Verbose, "ed3DClusterChooseGoodLOD no LODs in hierarchy");
	}
	else {
		edF32Matrix4MulF32Vector4Hard(&cameraSpaceSphere, WorldToCamera_Matrix, pSphere);

		ED3D_LOG(LogLevel::Verbose, "ed3DClusterChooseGoodLOD camera space sphere: {}", cameraSpaceSphere.ToString());

		edF32Vector4SquareHard(&cameraSpaceSphere, &cameraSpaceSphere);
		distanceFactor = gRenderFovCoef * (cameraSpaceSphere.z + cameraSpaceSphere.x + cameraSpaceSphere.y);

		if ((pHier->pHierarchySetup == 0x0) || (pLodBiases = (LOAD_SECTION_CAST(ed_3d_hierarchy_setup*, pHier->pHierarchySetup))->pLodBiases, pLodBiases == (float*)0x0)) {
			pLod = pHier->aLods;
			lodIndex = 0;

			while ((lodIndex < pHier->lodCount) && ((float)(pLod->sizeBias * pLod->sizeBias) <= distanceFactor)) {
				ED3D_LOG(LogLevel::Verbose, "ed3DClusterChooseGoodLOD LOD Failed check index: {} size bias: {} sqrd: {} distance factor: {}", 
					lodIndex, pLod->sizeBias, (float)(pLod->sizeBias * pLod->sizeBias), distanceFactor);

				pLod = pLod + 1;
				lodIndex = lodIndex + 1;
			}

			if (lodIndex == pHier->lodCount) {
				pLod = (ed3DLod*)0x0;
				pHier->desiredLod = 0xff;

				ED3D_LOG(LogLevel::Verbose, "ed3DClusterChooseGoodLOD exceeded LOD count!");
			}
		}
		else {
			IMPLEMENTATION_GUARD(
			for (lodIndex = 0; (lodIndex < pHier->lodCount && (pLodBiases[lodIndex] <= distanceFactor));
				lodIndex = lodIndex + 1) {
			}

			if (lodIndex == pHier->lodCount) {
				pHier->desiredLod = (byte)lodIndex;
				return (ed3DLod*)0x0;
			}

			pLod = (ed3DLod*)(pHier[1].hash + lodIndex * 8 + -0x80);)
		}

		pHier->desiredLod = (byte)lodIndex;

		ED3D_LOG(LogLevel::Verbose, "ed3DClusterChooseGoodLOD picked LOD index: {}", lodIndex);
	}

	return pLod;
}

void ed3DRenderClusterObject(ed_hash_code* pLOD, ed_hash_code* pMBNK, uint clusterType)
{
	edF32MATRIX4* peVar1;
	edF32VECTOR4* pBoundSphereCenter;
	EVectorMode_A EVar3;
	uint testResult;
	edF32VECTOR4* pSphere;
	int stripCount;
	ed_3d_strip* pStrip;
	int defaultTestResult;
	edF32VECTOR4 boundingSphere;
	ed_g3d_object* pObjInternal;

	ED3D_LOG(LogLevel::VeryVerbose, "ed3DRenderClusterObject A: {} tex: {} cluster type: {}", pLOD->hash.ToString(), pMBNK->hash.ToString(), clusterType);

	ed_Chunck* pOBJ = LOAD_SECTION_CAST(ed_Chunck*, pLOD->pData);
	pObjInternal = reinterpret_cast<ed_g3d_object*>(pOBJ + 1);

	pStrip = (ed_3d_strip*)LOAD_SECTION(pObjInternal->p3DStrip);

	if (pStrip == (ed_3d_strip*)0x0) {
		return;
	}

	ED3D_LOG(LogLevel::VeryVerbose, "ed3DRenderClusterObject valid strip: 0x{:x}", (uintptr_t)pStrip);

	gRender_info_SPR->pMeshTransformMatrix = WorldToCamera_Matrix;
	pBoundSphereCenter = gBoundSphereCenter;
	stripCount = pObjInternal->stripCount;
	if ((gRender_info_SPR->flags & 0x10) != 0) {
		pSphere = gRender_info_SPR->pHierarchySetup->pBoundingSphere;
		defaultTestResult = 8;
		if (pSphere != (edF32VECTOR4*)0x0) goto LAB_002b181c;
	}
	pSphere = &boundingSphere;
	defaultTestResult = 2;
	boundingSphere = pObjInternal->boundingSphere;
LAB_002b181c:

	gBoundSphereCenter->xyz = pSphere->xyz;
	gBoundSphereCenter->w = 1.0f;

	*gBoundSphereCenter = (*gBoundSphereCenter) * (*gRender_info_SPR->pMeshTransformMatrix);

	ED3D_LOG(LogLevel::VeryVerbose, "ed3DRenderClusterObject calculated bounding sphere: {} base sphere: {}", gBoundSphereCenter->ToString(), pSphere->ToString());

	EVar3 = ed3DTestBoundingSphere(pSphere);
	ED3D_LOG(LogLevel::VeryVerbose, "ed3DRenderClusterObject bounding sphere test result: {}", EVar3);
	if (EVar3 != VM_4) {
		gRender_info_SPR->boundingSphereTestResult = 1;
		if ((EVar3 != VM_1) && (testResult = ed3DTestBoundingSphereObjectNoZFar(pSphere), testResult != 1)) {
			gRender_info_SPR->boundingSphereTestResult = defaultTestResult;
		}
		if (clusterType == 0) {
			for (; stripCount != 0; stripCount = stripCount + -1) {
				ed3DLinkClusterStripToViewport(pStrip, pMBNK);
				pStrip = (ed_3d_strip*)LOAD_SECTION(pStrip->pNext);
			}
		}
		else {
			if (clusterType == 3) {
				for (; stripCount != 0; stripCount = stripCount + -1) {
					IMPLEMENTATION_GUARD_LOG(
					ed3DLinkClusterSpriteToViewport(pStrip, gRender_info_SPR->boundingSphereTestResult, pMBNK);)
					pStrip = (ed_3d_strip*)LOAD_SECTION(pStrip->pNext);
				}
			}
		}
	}

	return;
}

void ed3DRenderClusterHierarchy(ed_g3d_hierarchy* pHier)
{
	short sVar1;
	char* pcVar2;
	ed_hash_code* pLodHash;
	edF32VECTOR4* peVar3;
	ed3DLod* pLod;
	ed_hash_code* peVar5;
	ed_hash_code* pMbnkHash;

	ED3D_LOG(LogLevel::Verbose, "ed3DRenderClusterHierarchy flags: 0x{:x} LOD Count: {}", pHier->flags_0x9e , pHier->lodCount);

	if (((pHier->flags_0x9e & 0x40) == 0) && (pHier->lodCount != 0)) {
		gRender_info_SPR->pHierarchySetup = (ed_3d_hierarchy_setup*)LOAD_SECTION(pHier->pHierarchySetup);

		if ((gRender_info_SPR->pHierarchySetup == (ed_3d_hierarchy_setup*)0x0) || (peVar3 = gRender_info_SPR->pHierarchySetup->pBoundingSphere, peVar3 == (edF32VECTOR4*)0x0)) {
			peVar3 = ed3DGetHierarchyFirstLODSphere(pHier);
			ED3D_LOG(LogLevel::Verbose, "ed3DRenderClusterHierarchy using LOD Sphere");
		}

		ED3D_LOG(LogLevel::Verbose, "ed3DRenderClusterHierarchy bounding sphere: {}", peVar3 ? peVar3->ToString() : "NULL");

		if (peVar3 != (edF32VECTOR4*)0x0) {
			center$1208.xyz = peVar3->xyz;

			pLod = ed3DClusterChooseGoodLOD(pHier, &center$1208);

			if (pLod != (ed3DLod*)0x0) {
				pcVar2 = (char*)LOAD_SECTION(pHier->pTextureInfo);
				gRender_info_SPR->pSharedMeshTransform = (edF32MATRIX4*)0x0;
				gRender_info_SPR->pMeshTransformMatrix = WorldToCamera_Matrix;
				gRender_info_SPR->flags = (uint)pHier->flags_0x9e;
				gRender_info_SPR->biggerScale = 1.0f;
				gRender_info_SPR->pPkt = (edpkt_data*)0x0;
				gRender_info_SPR->pMeshTransformData = (ed_3d_hierarchy*)pHier;
				pLodHash = (ed_hash_code*)LOAD_SECTION(pLod->pObj);
				pMbnkHash = (ed_hash_code*)(pcVar2 + 0x10);

				if (pLodHash != (ed_hash_code*)0x0) {
					sVar1 = pLod->renderType;
					if (sVar1 == 3) {
						ed3DRenderClusterObject(pLodHash, pMbnkHash, 3);
					}
					else {
						if ((sVar1 != 2) && (sVar1 != 1)) {
							if (sVar1 != 0) {
								peVar5 = LOAD_SECTION_CAST(ed_hash_code*, pHier->pLinkTransformData);
								goto LAB_002b1758;
							}
							ed3DRenderClusterObject(pLodHash, pMbnkHash, 0);
						}
					}
				}
			}
		}
	}

	peVar5 = LOAD_SECTION_CAST(ed_hash_code*, pHier->pLinkTransformData);
LAB_002b1758:
	if (peVar5 != (ed_hash_code*)0x0) {
		ED3D_LOG(LogLevel::Verbose, "ed3DRenderClusterHierarchy recursive {}", peVar5->hash.ToString());
		ed3DRenderClusterHierarchy((ed_g3d_hierarchy*)(peVar5 + 1));
	}
	return;
}

void ed3DRenderCluster(ed_3d_octree* p3DOctree)
{
	bool bVar1;
	ushort uVar2;
	ed_Chunck* pMBNK;
	edF32MATRIX4* pMVar5;
	bool bProcessedStrip;
	edF32VECTOR4* pVVar7;
	ed_Chunck* piVar8;
	uint uVar9;
	int iVar10;
	char cVar11;
	char* piVar12;
	ed_3d_strip* p3DStrip;
	uint uVar14;
	ulong uVar15;
	long lVar16;
	ed_hash_code* pHashCode;
	ushort* pStripCounts;
	uint uVar19;
	uint stripCountArrayEntryIndex;
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
	ed_g3d_cluster* pCluster;

	pCluster = reinterpret_cast<ed_g3d_cluster*>(p3DOctree->pCDQU + 1);
	pStripCounts = pCluster->aClusterStripCounts;

	if (p3DOctree->field_0x30 == 1.0f) {
		stripCountArrayEntryIndex = 4;
	}
	else {
		stripCountArrayEntryIndex = 0xc;
	}

	ED3D_LOG(LogLevel::Verbose, "ed3DRenderCluster Strip Count: {} index: {}", pStripCounts[stripCountArrayEntryIndex], stripCountArrayEntryIndex);

	uint stripCount = pStripCounts[stripCountArrayEntryIndex];
	bProcessedStrip = false;

	if ((stripCount != 0) && (bProcessedStrip = true, stripCount != 0)) {
		pMBNK = LOAD_SECTION_CAST(ed_Chunck*, pCluster->pMBNK);
		p3DStrip = LOAD_SECTION_CAST(ed_3d_strip*, pCluster->p3DStrip);

		gRender_info_SPR->boundingSphereTestResult = (uint)p3DOctree->boundingSphereTestResult;
		gRender_info_SPR->pMeshTransformMatrix = WorldToCamera_Matrix;

		for (; stripCount != 0; stripCount = stripCount - 1) {
			ed_hash_code* pMaterialBank = reinterpret_cast<ed_hash_code*>(pMBNK + 1);
			ed3DLinkClusterStripToViewport(p3DStrip, pMaterialBank);
			p3DStrip = LOAD_SECTION_CAST(ed_3d_strip*, p3DStrip->pNext);
		}

		bProcessedStrip = true;
	}

	uint spriteCount = pCluster->clusterDetails.spriteCount;

	ED3D_LOG(LogLevel::Verbose, "ed3DRenderCluster Sprite Count: {}", spriteCount);

	if (spriteCount != 0) {
		IMPLEMENTATION_GUARD_LOG(
		iVar10 = *(int*)pCluster->p3DSprite;
		pMBNK = LOAD_SECTION_CAST(ed_Chunck*, pCluster->pMBNK);
		uVar2 = p3DOctree->boundingSphereTestResult;
		for (; 0 < (int)spriteCount; spriteCount = spriteCount - 1) {
			ed_hash_code* pMaterialBank = reinterpret_cast<ed_hash_code*>(pMBNK + 1);
			//ed3DLinkClusterSpriteToViewport(iVar10, uVar2, pMaterialBank);
			iVar10 = *(int*)(iVar10 + 0xc);
		})

		bProcessedStrip = true;
	}

	uint clusterHierCount = pCluster->clusterDetails.clusterHierCount;

	ED3D_LOG(LogLevel::Verbose, "ed3DRenderCluster Cluster Hier Count: {}", clusterHierCount);

	if (clusterHierCount != 0) {
		ed_Chunck* pHASH = reinterpret_cast<ed_Chunck*>(pCluster + 1);
		pHashCode = reinterpret_cast<ed_hash_code*>(pHASH + 1);

		while (bVar1 = clusterHierCount != 0, clusterHierCount = clusterHierCount - 1, bVar1) {
			ED3D_LOG(LogLevel::Verbose, "ed3DRenderCluster Cluster Hier: {}", pHashCode->hash.ToString());
			ed_Chunck* pHIER = LOAD_SECTION_CAST(ed_Chunck*, pHashCode->pData);
			assert(pHIER->hash == HASH_CODE_HIER);
			ed3DRenderClusterHierarchy(reinterpret_cast<ed_g3d_hierarchy*>(pHIER + 1));
			pHashCode = pHashCode + 1;
		}
	}

	if (!bProcessedStrip) {
		uVar2 = pCluster->clusterDetails.clusterHierCount;
		uVar9 = 0;
		uVar15 = 0;
		iVar10 = 0;
		uVar19 = 1;

		if (uVar2 != 0) {
			iVar10 = (uint)uVar2 * 0x10 + 0x10;
		}

		ed_Chunck* pHASH = reinterpret_cast<ed_Chunck*>(pCluster + 1);

		piVar8 = edChunckGetFirst(reinterpret_cast<char*>(pHASH) + iVar10, p3DOctree->pCDQU_End);
		while (true) {
			uVar14 = (uint)uVar15;

			if (piVar8 == (ed_Chunck*)0x0) break;

			local_1d4[uVar14 * 0x10 + 1] = p3DOctree->field_0x30;

			if ((piVar8->hash == 0x55514443) || (piVar8->hash == 0x434f4443)) {
				if (p3DOctree->boundingSphereTestResult == 2) {
					if (local_1d4[uVar14 * 0x10 + 1] == 1.0f) {
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
					local_230.w = p3DOctree->field_0x2c * 0.5f;
					gRender_info_SPR->pMeshTransformMatrix = WorldToCamera_Matrix;

					gBoundSphereCenter->xyz = local_230.xyz;
					gBoundSphereCenter->w = 1.0f;
					*gBoundSphereCenter = (*gBoundSphereCenter) * (*gRender_info_SPR->pMeshTransformMatrix);

					iVar10 = ed3DTestBoundingSphere(&local_230);
					if (iVar10 == 4) {
						local_1d8[uVar14 * 0x20] = 4;
					}
					else {
						(&local_1e0)[uVar14 * 0x10] = (int*)piVar8;
						(&local_1dc)[uVar14 * 0x10] = (int)reinterpret_cast<char*>(piVar8) + piVar8->size;
						local_1d8[uVar14 * 0x20] = (short)iVar10;
						local_1d4[uVar14 * 0x10] = local_230.w;
						local_240.x = local_230.x;
						local_240.y = local_230.y;
						local_240.z = local_230.z;
						local_240.w = 1.0;
						edF32Vector4SubHard(&local_250, &local_240, &gRenderCamera->position);
						edF32Vector4SquareHard(&local_250, &local_250);
						uVar9 = uVar9 | uVar19;
						local_250.x = local_250.x + local_250.y + local_250.z;
						local_1d4[uVar14 * 0x10 + 2] = local_250.x;
					}
				}
				else {
					if (local_1d4[uVar14 * 0x10 + 1] == 1.0f) {
						IMPLEMENTATION_GUARD(ed3DCalculSubBoxQuadTree((float*)(auStack544 + uVar14 * 0x40 + 0x20), (float*)p3DOctree, uVar14));
					}
					else {
						IMPLEMENTATION_GUARD(ed3DCalculSubBoxOctree((float*)(auStack544 + uVar14 * 0x40 + 0x20), (float*)p3DOctree, uVar15));
					}

					local_260.x = *(float*)(auStack544 + uVar14 * 0x40 + 0x30);
					local_260.y = local_1f0[uVar14 * 0x10 + 1];
					local_260.z = local_1f0[uVar14 * 0x10 + 2];
					(&local_1e0)[uVar14 * 0x10] = (int*)piVar8;
					(&local_1dc)[uVar14 * 0x10] = (int)reinterpret_cast<char*>(piVar8) + piVar8->size;
					local_1d8[uVar14 * 0x20] = 1;
					local_260.w = 1.0;
					local_230.x = local_260.x;
					local_230.y = local_260.y;
					local_230.z = local_260.z;
					edF32Vector4SubHard(&local_270, &local_260, &gRenderCamera->position);
					edF32Vector4SquareHard(&local_270, &local_270);
					uVar9 = uVar9 | uVar19;
					local_270.x = local_270.x + local_270.y + local_270.z;
					local_1d4[uVar14 * 0x10 + 2] = local_270.x;
				}

				piVar12 = p3DOctree->pCDQU_End;
			}
			else {
				piVar12 = p3DOctree->pCDQU_End;
			}

			uVar19 = uVar19 << 1;
			uVar15 = (int)(uVar14 + 1);
			piVar8 = edChunckGetNext(piVar8, piVar12);
		}

		if (uVar15 != 0) {
			IMPLEMENTATION_GUARD(ed3DRenderClusterSortOctreeSons((ed_3d_octree*)auStack544, (int)(auStack544 + 0x20), uVar14));
			lVar16 = 0;
			while (true) {
				if ((long)uVar15 <= lVar16) break;
				if ((*(ed_3d_octree**)(auStack544 + (int)lVar16 * 4))->boundingSphereTestResult != 4) {
					ed3DRenderCluster(*(ed_3d_octree**)(auStack544 + (int)lVar16 * 4));
				}
				lVar16 = (long)((int)lVar16 + 1);
			}
		}

		fVar21 = p3DOctree->field_0x30;

		if (fVar21 < 2.147484e+09f) {
			cVar11 = (char)(int)fVar21;
		}
		else {
			cVar11 = (char)(int)(fVar21 - 2.147484e+09f);
		}

		uVar9 = ed3DClusterGetEdgeWithSon(uVar9, cVar11);
		if (uVar9 != 0) {
			IMPLEMENTATION_GUARD(ed3DRenderEdge((short*)pStripCounts, (long)(int)uVar9, (uint)p3DOctree->boundingSphereTestResult, stripCountArrayEntryIndex));
		}
	}
	return;
}

bool ed3DSceneRenderCluster(ed_g3d_manager* pMeshInfo)
{
	bool bVar1;
	edF32VECTOR4 location;
	ed_3d_octree octree;

	ED3D_LOG(LogLevel::Verbose, "ed3DSceneRenderCluster");

	ed_Chunck* pClusterTypeChunk = pMeshInfo->CSTA + 1;

	if (pClusterTypeChunk->hash == HASH_CODE_CDOA) {
		MeshData_CSTA* pCSTA = reinterpret_cast<MeshData_CSTA*>(pClusterTypeChunk + 1);
		location.xyz = pCSTA->field_0x20;
		octree.field_0x0.w = 0.0f;
		octree.worldLocation.xyz = pCSTA->worldLocation.xyz;
		octree.worldLocation.w = 1.0f;
		location.w = 0.0f;
		octree.field_0x0.xyz = location.xyz;
		edF32Vector4SquareHard(&location, &location);
		octree.boundingSphereTestResult = 2;
		octree.field_0x30 = 0.0f;
		location.x = location.x + location.y + location.z;
		octree.field_0x2c = sqrtf(location.x) * 0.5f;
		octree.pCDQU = edChunckGetFirst(pCSTA + 1, (char*)0x0);
		octree.pCDQU_End = reinterpret_cast<char*>(octree.pCDQU) + octree.pCDQU->size;
		ed3DRenderCluster(&octree);
		bVar1 = true;
	}
	else {
		if (pClusterTypeChunk->hash == HASH_CODE_CDQA) {
			MeshData_CSTA* pCSTA = reinterpret_cast<MeshData_CSTA*>(pClusterTypeChunk + 1);

			location.xyz = pCSTA->field_0x20;
			octree.field_0x0.w = 0.0f;
			octree.worldLocation.xyz = pCSTA->worldLocation.xyz;
			octree.worldLocation.w = 1.0f;
			location.w = 0.0f;
			octree.field_0x0.xyz = location.xyz;
			edF32Vector4SquareHard(&location, &location);
			octree.field_0x30 = 1.0f;
			octree.boundingSphereTestResult = 2;
			location.x = location.x + location.y + location.z;
			octree.field_0x2c = sqrtf(location.x) * 0.5;
			octree.pCDQU = edChunckGetFirst(pCSTA + 1, (char*)0x0);
			octree.pCDQU_End = reinterpret_cast<char*>(octree.pCDQU) + octree.pCDQU->size;
			ed3DRenderCluster(&octree);
			bVar1 = true;
		}
		else {
			bVar1 = false;
		}
	}

	return bVar1;
}

edF32MATRIX3 Matrix_00424f50 = {
	1.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 1.0f, 0.0f
};

float ed3DMatrixGetBigerScale(edF32MATRIX4* m0)
{
	float fVar1;
	float fVar2;
	float fVar3;
	int iVar4;
	edF32VECTOR4* pVVar5;
	edF32MATRIX4* pMVar6;
	edF32VECTOR4* v0;
	edF32MATRIX3 local_30;

	local_30 = Matrix_00424f50;

	edF32Matrix4MulF32Vector4Hard(&local_30.rowX, m0, &local_30.rowX);
	edF32Matrix4MulF32Vector4Hard(&local_30.rowY, m0, &local_30.rowY);
	edF32Matrix4MulF32Vector4Hard(&local_30.rowZ, m0, &local_30.rowZ);

	edF32Vector4SquareHard(&local_30.rowX, &local_30.rowX);
	edF32Vector4SquareHard(&local_30.rowY, &local_30.rowY);
	edF32Vector4SquareHard(&local_30.rowZ, &local_30.rowZ);
	fVar2 = local_30.rowX.z + local_30.rowX.x + local_30.rowX.y;
	fVar3 = local_30.rowY.z + local_30.rowY.x + local_30.rowY.y;
	fVar1 = local_30.rowZ.z + local_30.rowZ.x + local_30.rowZ.y;
	if (fVar2 < fVar3) {
		if (fVar3 < fVar1) {
			fVar2 = sqrtf(fVar1);
		}
		else {
			fVar2 = sqrtf(fVar3);
		}
	}
	else {
		if (fVar2 < fVar1) {
			fVar2 = sqrtf(fVar1);
		}
		else {
			fVar2 = sqrtf(fVar2);
		}
	}
	return fVar2;
}

void ed3DSetSceneRender(ed_3d_hierarchy* pMeshTransformData, byte bNewSceneRender)
{
	ED3D_LOG(LogLevel::Verbose, "ed3DSetSceneRender {} for {}", bNewSceneRender, pMeshTransformData->hash.ToString());
	pMeshTransformData->bSceneRender = bNewSceneRender;
	return;
}

ed3DLod* ed3DChooseGoodLOD(ed_3d_hierarchy* pHierarchy)
{
	ed_3d_hierarchy_setup* pHierarchySetup;
	float* pLodBiases;
	ed3DLod* pLod;
	uint lodCount;
	uint desiredLod;
	float cameraSize;
	edF32MATRIX4 cameraSpaceTransform;

	lodCount = pHierarchy->lodCount;

#ifdef PLATFORM_WIN
	if (ed3D::DebugOptions::gForceHighestLod) {
		lodCount = 1;
		ED3D_LOG(LogLevel::Verbose, "ed3DChooseGoodLOD Forcing highest LOD");
	}
#endif

	if (lodCount < 2) {
		ED3D_LOG(LogLevel::Verbose, "ed3DChooseGoodLOD Only a single LOD, setting to LOD 0");

		pHierarchy->desiredLod = (char)lodCount - 1;


		if (pHierarchy->desiredLod == 0xff) {
			ED3D_LOG(LogLevel::Verbose, "ed3DChooseGoodLOD ERROR NO VALID LODS!!!");
			pLod = (ed3DLod*)0x0;
		}
		else {
			pLod = (ed3DLod*)(pHierarchy + 1);
		}
	}
	else {
		if (pHierarchy->desiredLod == 0xff) {
			ED3D_LOG(LogLevel::Verbose, "ed3DChooseGoodLOD No desired LOD, calculating new one.");

			edF32Matrix4MulF32Matrix4Hard(&cameraSpaceTransform, &pHierarchy->transformB, &gCurLOD_WorldToCamera_Matrix);
			pHierarchySetup = pHierarchy->pHierarchySetup;
			cameraSize = gCurLOD_RenderFOVCoef * (cameraSpaceTransform.dc * cameraSpaceTransform.dc + cameraSpaceTransform.da * cameraSpaceTransform.da + cameraSpaceTransform.db * cameraSpaceTransform.db);
			
			if ((pHierarchySetup == (ed_3d_hierarchy_setup*)0x0) || (pLodBiases = pHierarchySetup->pLodBiases, pLodBiases == (float*)0x0)) {
				pLod = (ed3DLod*)(pHierarchy + 1);
				desiredLod = 0;

				while ((desiredLod < lodCount && ((float)(pLod->sizeBias * pLod->sizeBias) <= cameraSize))) {
					pLod = pLod + 1;
					desiredLod = desiredLod + 1;
				}

				if (desiredLod == lodCount) {
					pLod = (ed3DLod*)0x0;
				}
			}
			else {
				for (desiredLod = 0; (desiredLod < lodCount && (pLodBiases[desiredLod] <= cameraSize)); desiredLod = desiredLod + 1) {
				}

				if (desiredLod == lodCount) {
					pHierarchy->desiredLod = (byte)desiredLod;

					// Is this always bad?
					//assert(false);
					return (ed3DLod*)0x0;
				}
				pLod = (ed3DLod*)(pHierarchy + 1);
				pLod = pLod + desiredLod;
			}

			pHierarchy->desiredLod = (byte)desiredLod;
		}
		else {
			ED3D_LOG(LogLevel::Verbose, "ed3DChooseGoodLOD Using desired LOD ({})", pHierarchy->desiredLod);
			pLod = (ed3DLod*)(pHierarchy + 1);
			pLod = pLod + pHierarchy->desiredLod;
		}
	}

	return pLod;
}

PACK(struct MeshHeader {
	Hash_8 header;
	int pOBJ;
});

uint ed3DTestBoundingSphereObject(edF32VECTOR4* pSphere)
{
	float* pfVar1;
	uint uVar2;
	float fVar3;
	float fVar4;
	float fVar5;

	//ED3D_LOG(LogLevel::Verbose, "ed3DTestBoundingSphereObject flags: 0x{:x} ", gRender_info_SPR->flags);

	uVar2 = 1;
	fVar4 = pSphere->w * gRender_info_SPR->biggerScale;
	if (((gRender_info_SPR->flags & 0x10) == 0) || (pfVar1 = (float*)gRender_info_SPR->pHierarchySetup->clipping_0x0, pfVar1 == (float*)0x0)) {
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
	if (fVar3 <= 0.0f) {
		fVar3 = (gRenderSceneConfig_SPR->frustumA).frustumMatrix.ba * -fVar3 +
			(gRenderSceneConfig_SPR->frustumA).frustumMatrix.bc * -fVar5;
		if (0.0f < fVar3) {
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
		if (0.0f < fVar3) {
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
	if (fVar3 <= 0.0f) {
		fVar3 = (gRenderSceneConfig_SPR->frustumA).frustumMatrix.cb * fVar3 +
			(gRenderSceneConfig_SPR->frustumA).frustumMatrix.cc * -fVar5;
		if (0.0f < fVar3) {
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
		if (0.0f < fVar3) {
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

uint ed3DTestBoundingSphereObjectNoZFar(edF32VECTOR4* pSphere)
{
	uint result;
	float fVar1;
	float z;
	float fVar2;

	result = 1;
	z = gBoundSphereCenter->z;
	fVar1 = (gRenderSceneConfig_SPR->frustumB).field_0x40.w;
	fVar2 = pSphere->w * gRender_info_SPR->biggerScale;
	if (z < fVar1) {
		if (-(z - fVar1) < fVar2) {
			result = 2;
		}
	}
	else {
		if (fVar2 <= -(fVar1 - z)) {
			return 4;
		}
		result = 2;
	}
	fVar1 = gBoundSphereCenter->x;
	if (fVar1 <= 0.0) {
		fVar1 = (gRenderSceneConfig_SPR->frustumB).frustumMatrix.ba * -fVar1 +
			(gRenderSceneConfig_SPR->frustumB).frustumMatrix.bc * -z;
		if (0.0 < fVar1) {
			if (fVar1 <= fVar2) {
				result = 2;
			}
		}
		else {
			if (fVar2 < -fVar1) {
				return 4;
			}
			result = 2;
		}
	}
	else {
		fVar1 = (gRenderSceneConfig_SPR->frustumB).frustumMatrix.aa * -fVar1 +
			(gRenderSceneConfig_SPR->frustumB).frustumMatrix.ac * -z;
		if (0.0 < fVar1) {
			if (fVar1 <= fVar2) {
				result = 2;
			}
		}
		else {
			if (fVar2 < -fVar1) {
				return 4;
			}
			result = 2;
		}
	}
	fVar1 = gBoundSphereCenter->y;
	if (fVar1 <= 0.0) {
		fVar1 = (gRenderSceneConfig_SPR->frustumB).frustumMatrix.cb * fVar1 +
			(gRenderSceneConfig_SPR->frustumB).frustumMatrix.cc * -z;
		if (0.0 < fVar1) {
			if (fVar1 <= fVar2) {
				result = 2;
			}
		}
		else {
			result = 4;
			if (-fVar1 <= fVar2) {
				result = 2;
			}
		}
	}
	else {
		fVar1 = (gRenderSceneConfig_SPR->frustumB).frustumMatrix.db * fVar1 +
			(gRenderSceneConfig_SPR->frustumB).frustumMatrix.dc * -z;
		if (0.0 < fVar1) {
			if (fVar1 <= fVar2) {
				result = 2;
			}
		}
		else {
			result = 4;
			if (-fVar1 <= fVar2) {
				result = 2;
			}
		}
	}
	return result;
}

ed_dma_matrix* pprevious_dma_matrix = NULL;
ed_dma_material* pprevious_dma_material = NULL;

ed_dma_matrix* ed3DListCreateDmaMatrixNode(ScratchPadRenderInfo* pRenderInfo, ed_3d_hierarchy* pHierarchy)
{
	edF32MATRIX4** ppeVar1;
	ed_dma_matrix* peVar2;

	peVar2 = DmaMatrixBufferCurrent;
	ppeVar1 = &DmaMatrixBufferCurrent->pObjToWorld;
	DmaMatrixBufferCurrent = DmaMatrixBufferCurrent + 1;
	*ppeVar1 = pRenderInfo->pSharedMeshTransform;
	peVar2->pHierarchy = pHierarchy;
	peVar2->normalScale = pRenderInfo->biggerScale;
	(peVar2)->pNext = (edNODE*)peVar2;
	(peVar2)->pPrev = (edNODE*)peVar2;
	(peVar2)->pData = (ed_3d_hierarchy*)gNodeDmaStrip;
	(peVar2)->field_0x10 = 0;
	(peVar2)->nodeCount = 0;
	(peVar2)->header.mode = 0;
	return peVar2;
}

byte BYTE_00449420 = 0;
byte BYTE_00448a08 = 1;

inline void ed3DLinkMaterialToPrimList(ed_dma_material* pCurrentMaterial, int PrimType)
{
	edNODE_MANAGER* pNodeManager = (edNODE_MANAGER*)gPrim_List[PrimType][gCurRenderList].pData;
	edNODE* pNextNode = pNodeManager->pNodeHead + pNodeManager->linkCount;
	pNextNode->pData = pCurrentMaterial;

	pNextNode->header.typeField.type = LIST_TYPE_MATERIAL;
	pNodeManager->linkCount += 1;

	gPrim_List[PrimType][gCurRenderList].nodeCount += 1;

	edLIST* pCurList = &gPrim_List[PrimType][gCurRenderList];
	pNextNode->pPrev = pCurList->pPrev;
	pCurList->pPrev = pNextNode;
}

void ed3DLinkStripManageLinkToDMA(ed_3d_strip* pStrip, uint flagsA, uint flagsB, edF32VECTOR4* pBoundingSphere, ed_g2d_material* pLinkMaterial, ed_g2d_bitmap* pBitmap)
{
	ed_dma_material* peVar1;
	ed_g2d_material** ppCurrentMaterialMaterial;
	edLIST* pList;
	ed_dma_matrix* pMVar7;
	edNODE* peVar4;
	edNODE* pNewMeshTransformSpecial;
	edNODE_MANAGER* peVar5;
	int* piVar6;
	ed_dma_material* pCurrentMaterial;
	uint uVar8;
	edNODE_MANAGER* pMeshTransformParentHeader;
	edNODE_MANAGER* pFinalNodeManager;

	ED3D_LOG(LogLevel::Verbose, "ed3DLinkStripManageLinkToDMA 0x{:x} pMaterial: 0x{:x}", (uintptr_t)pStrip, (uintptr_t)pLinkMaterial);

	pCurrentMaterial = DmaMaterialBufferCurrent;
	if ((((flagsB & 0x200) != 0) && ((flagsA & 0x80000000) != 0)) ||
		((uVar8 = flagsA & 0xfc, uVar8 != 0 &&
			((((flagsA & 0x80000000) == 0 && ((flagsA & 0x4000) == 0)) && ((flagsB & 0x40) == 0)))))) {
		if (gRender_info_SPR->boundingSphereTestResult == 1) {
			pBoundingSphere->xyz = (pStrip->boundingSphere).xyz;
			pBoundingSphere->w = 1.0f;
			edF32Matrix4MulF32Vector4Hard(pBoundingSphere, gRender_info_SPR->pMeshTransformMatrix, pBoundingSphere);
		}

		pCurrentMaterial = DmaMaterialBufferCurrent;
		pList = &DmaMaterialBufferCurrent->list;
		ppCurrentMaterialMaterial = &DmaMaterialBufferCurrent->pMaterial;
		DmaMaterialBufferCurrent = DmaMaterialBufferCurrent + 1;
		*ppCurrentMaterialMaterial = pLinkMaterial;
		pCurrentMaterial->field_0x4 = pBoundingSphere->z;
		pCurrentMaterial->flags = pCurrentMaterial->flags & 0xfffffffe;

		(pCurrentMaterial->list).pNext = (edNODE*)pList;
		(pCurrentMaterial->list).pPrev = (edNODE*)pList;
		(pCurrentMaterial->list).pData = gNodeDmaMatrix;
		(pCurrentMaterial->list).field_0x10 = 0;
		(pCurrentMaterial->list).nodeCount = 0;
		(pCurrentMaterial->list).header.mode = 0;

		pMVar7 = ed3DListCreateDmaMatrixNode(gRender_info_SPR, gRender_info_SPR->pMeshTransformData);
		pCurrentMaterial->pBitmap = pBitmap;
		if ((pStrip->shadowCastFlags == 0) &&
			((((BYTE_00449420 != 0 && ((pStrip->flags & 0x10000) != 0)) && ((pStrip->flags & 0x2000) == 0)) ||
				(((BYTE_00448a08 != 0 && (gRender_info_SPR->pMeshTransformData != (ed_3d_hierarchy*)0x0)) &&
					((gRender_info_SPR->pMeshTransformData->flags_0x9e & 0x400) != 0)))))) {
			ed3DLinkStripManageLinkToDMALink(gPrim_List[0xe] + gCurRenderList, pCurrentMaterial, 1);

			peVar5 = (edNODE_MANAGER*)(pCurrentMaterial->list).pData;
		}
		else {
			if ((flagsA & 0x80000000) == 0) {
				ed3DLinkStripManageLinkToDMALink(gPrim_List[3] + gCurRenderList, pCurrentMaterial, 1);
			}
			else {
				ed3DLinkStripManageLinkToDMALink(gPrim_List[7] + gCurRenderList, pCurrentMaterial, 1);
			}

			peVar5 = (edNODE_MANAGER*)(pCurrentMaterial->list).pData;
		}

		peVar4 = peVar5->pNodeHead + peVar5->linkCount;
		peVar4->pData = pMVar7;
		peVar4->header.typeField.type = 2;
		peVar5->linkCount = peVar5->linkCount + 1;
		(pCurrentMaterial->list).nodeCount = (pCurrentMaterial->list).nodeCount + 1;
		peVar4->pPrev = (pCurrentMaterial->list).pPrev;
		(pCurrentMaterial->list).pPrev = peVar4;

		if ((ed_3d_hierarchy*)gRender_info_SPR->pPkt != (ed_3d_hierarchy*)0x0) {
			peVar5 = (edNODE_MANAGER*)pMVar7->pData;
			peVar4 = peVar5->pNodeHead + peVar5->linkCount;
			peVar4->pData = gRender_info_SPR->pPkt;
			peVar4->header.typeField.type = 5;
			peVar5->linkCount = peVar5->linkCount + 1;
			pMVar7->nodeCount = pMVar7->nodeCount + 1;
			peVar4->pPrev = pMVar7->pPrev;
			pMVar7->pPrev = peVar4;
		}

		peVar5 = (edNODE_MANAGER*)pMVar7->pData;
		peVar4 = peVar5->pNodeHead + peVar5->linkCount;
		peVar4->pData = pStrip;
		peVar4->header.typeField.type = 3;
		peVar4->header.typeField.flags = (short)pStrip->flags;
		peVar5->linkCount = peVar5->linkCount + 1;
		pMVar7->nodeCount = pMVar7->nodeCount + 1;
		peVar4->pPrev = pMVar7->pPrev;
		peVar4->pNext = (edNODE*)pStrip->pDMA_Matrix.pDMA_Matrix;
		pMVar7->pPrev = peVar4;
		return;
	}

	/* Boomy path */
	peVar1 = (ed_dma_material*)LOAD_SECTION(pLinkMaterial->pDMA_Material);
	if (peVar1 == (ed_dma_material*)0x0) {
		pList = &DmaMaterialBufferCurrent->list;
		ppCurrentMaterialMaterial = &DmaMaterialBufferCurrent->pMaterial;
		DmaMaterialBufferCurrent = DmaMaterialBufferCurrent + 1;
		*ppCurrentMaterialMaterial = pLinkMaterial;
		pCurrentMaterial->field_0x4 = 0;
		pCurrentMaterial->flags = pCurrentMaterial->flags & 0xfffffffe;

		(pCurrentMaterial->list).pNext = (edNODE*)pList;
		(pCurrentMaterial->list).pPrev = (edNODE*)pList;
		(pCurrentMaterial->list).pData = (ed_3d_hierarchy*)gNodeDmaMatrix;
		(pCurrentMaterial->list).field_0x10 = 0;
		(pCurrentMaterial->list).nodeCount = 0;
		(pCurrentMaterial->list).header.mode = 0;

		pprevious_dma_matrix = ed3DListCreateDmaMatrixNode(gRender_info_SPR, gRender_info_SPR->pMeshTransformData);
		pLinkMaterial->pDMA_Material = STORE_SECTION(pCurrentMaterial);
		pCurrentMaterial->pBitmap = pBitmap;

		if ((pStrip->shadowCastFlags == 0) &&
			(((BYTE_00449420 != 0 && ((pStrip->flags & 0x10000) != 0)) ||
				((BYTE_00448a08 != 0 &&
					((gRender_info_SPR->pMeshTransformData != (ed_3d_hierarchy*)0x0 &&
						((gRender_info_SPR->pMeshTransformData->flags_0x9e & 0x400) != 0)))))))) {
			ed3DLinkMaterialToPrimList(pCurrentMaterial, PRIM_LIST_E);
		}
		else {
			if ((flagsB & 0x800) == 0) {
				if ((gRender_info_SPR->pMeshTransformData == (ed_3d_hierarchy*)0x0) ||
					(gRender_info_SPR->pMeshTransformData->GlobalAlhaON == -1)) {
					if ((flagsB & 0x40) == 0) {
						if ((flagsB & 0x80) == 0) {
							if ((flagsA & 0x80000000) == 0) {
								if ((flagsA & 0x4000) == 0) {
									if (((flagsA & 0x100) == 0) && (uVar8 == 0)) {
										ed3DLinkMaterialToPrimList(pCurrentMaterial, PRIM_LIST_0);
									}
									else {
										ed3DLinkMaterialToPrimList(pCurrentMaterial, PRIM_LIST_2);
									}
								}
								else {
									ed3DLinkMaterialToPrimList(pCurrentMaterial, PRIM_LIST_4);
								}
							}
							else {
								ed3DLinkMaterialToPrimList(pCurrentMaterial, PRIM_LIST_6);
							}
						}
						else {
							ed3DLinkMaterialToPrimList(pCurrentMaterial, PRIM_LIST_B);
						}
					}
					else {
						if ((flagsA & 0x80000000) == 0) {
							if ((flagsA & 0x4000) == 0) {
								if (uVar8 == 0) {
									if ((flagsA & 0x100) == 0) {
										ed3DLinkMaterialToPrimList(pCurrentMaterial, PRIM_LIST_1);
									}
									else {
										ed3DLinkMaterialToPrimList(pCurrentMaterial, PRIM_LIST_2);
									}
								}
								else {
									ed3DLinkMaterialToPrimList(pCurrentMaterial, PRIM_LIST_3);
								}
							}
							else {
								ed3DLinkMaterialToPrimList(pCurrentMaterial, PRIM_LIST_4);
							}
						}
						else {
							ed3DLinkMaterialToPrimList(pCurrentMaterial, PRIM_LIST_6);
						}
					}
				}
				else {
					if ((((flagsA & 0x80000000) == 0) && ((flagsA & 0x4000) == 0)) && (uVar8 == 0)) {
						ed3DLinkMaterialToPrimList(pCurrentMaterial, PRIM_LIST_8);
					}
					else {
						ed3DLinkMaterialToPrimList(pCurrentMaterial, PRIM_LIST_9);
					}
				}
			}
			else {
				ed3DLinkMaterialToPrimList(pCurrentMaterial, PRIM_LIST_D);
			}
		}
		/* Store boomy. */
		pMeshTransformParentHeader = (edNODE_MANAGER*)(pCurrentMaterial->list).pData;
		pNewMeshTransformSpecial = pMeshTransformParentHeader->pNodeHead + pMeshTransformParentHeader->linkCount;
		pNewMeshTransformSpecial->pData = pprevious_dma_matrix;
		pNewMeshTransformSpecial->header.typeField.type = LIST_TYPE_MATRIX;
		pMeshTransformParentHeader->linkCount = pMeshTransformParentHeader->linkCount + 1;
		(pCurrentMaterial->list).nodeCount = (pCurrentMaterial->list).nodeCount + 1;
		pNewMeshTransformSpecial->pPrev = pCurrentMaterial->list.pPrev;
		(pCurrentMaterial->list).pPrev = pNewMeshTransformSpecial;
	}
	else {

		if ((pprevious_dma_material != peVar1) && ((peVar1->list).nodeCount != 0)) {
			pprevious_dma_matrix = (ed_dma_matrix*)((peVar1->list).pPrev)->pData;
		}
		pCurrentMaterial = peVar1;
		if (pprevious_dma_matrix->pObjToWorld == gRender_info_SPR->pSharedMeshTransform) {
			pFinalNodeManager = (edNODE_MANAGER*)pprevious_dma_matrix->pData;
			goto LAB_002992a8;
		}
		pprevious_dma_matrix = ed3DListCreateDmaMatrixNode(gRender_info_SPR, gRender_info_SPR->pMeshTransformData);

		pMeshTransformParentHeader = (edNODE_MANAGER*)(peVar1->list).pData;
		pNewMeshTransformSpecial = pMeshTransformParentHeader->pNodeHead + pMeshTransformParentHeader->linkCount;
		pNewMeshTransformSpecial->pData = pprevious_dma_matrix;
		pNewMeshTransformSpecial->header.typeField.type = LIST_TYPE_MATRIX;
		pMeshTransformParentHeader->linkCount = pMeshTransformParentHeader->linkCount + 1;
		(pCurrentMaterial->list).nodeCount = (pCurrentMaterial->list).nodeCount + 1;
		pNewMeshTransformSpecial->pPrev = pCurrentMaterial->list.pPrev;
		(pCurrentMaterial->list).pPrev = pNewMeshTransformSpecial;
	}

	pFinalNodeManager = (edNODE_MANAGER*)pprevious_dma_matrix->pData;

	LAB_002992a8:
	
	peVar4 = pFinalNodeManager->pNodeHead + pFinalNodeManager->linkCount;
	peVar4->pData = pStrip;
	peVar4->header.typeField.type = LIST_TYPE_STRIP;
	peVar4->header.typeField.flags = (short)pStrip->flags;

	pFinalNodeManager->linkCount = pFinalNodeManager->linkCount + 1;
	(pprevious_dma_matrix)->nodeCount = (pprevious_dma_matrix)->nodeCount + 1;

	peVar4->pPrev = pprevious_dma_matrix->pPrev;
	peVar4->pNext = (edNODE*)pStrip->pDMA_Matrix.pDMA_Matrix;
	pprevious_dma_matrix->pPrev = peVar4;

	if (gRender_info_SPR->pPkt != 0x0) {
		IMPLEMENTATION_GUARD(
		peVar5 = ((edNODE*)&(((ed_3d_hierarchy*)pprevious_dma_matrix)->transformA).aa)->pData;
		peVar4 = (edNODE*)((int)(peVar5->transformA).ab + (int)(peVar5->transformA).ad * 0x10);
		peVar4->pData = (ed_3d_hierarchy*)gRender_info_SPR->pPkt;
		*(undefined2*)&peVar4->header = 5;
		(peVar5->transformA).ad = (float)((int)(peVar5->transformA).ad + 1);
		(((ed_3d_hierarchy*)pprevious_dma_matrix)->transformA).bb =
			(float)((int)(((ed_3d_hierarchy*)pprevious_dma_matrix)->transformA).bb + 1);
		peVar4->pPrev = ((edNODE*)&(((ed_3d_hierarchy*)pprevious_dma_matrix)->transformA).aa)->pPrev;
		((edNODE*)&(((ed_3d_hierarchy*)pprevious_dma_matrix)->transformA).aa)->pPrev = peVar4;)
	}

	pprevious_dma_material = pCurrentMaterial;
	return;
}

static edF32MATRIX4 hackyMatrices[0x100] = 
{ 
	gF32Matrix4Unit , gF32Matrix4Unit , gF32Matrix4Unit , gF32Matrix4Unit,
	gF32Matrix4Unit , gF32Matrix4Unit , gF32Matrix4Unit , gF32Matrix4Unit,
	gF32Matrix4Unit , gF32Matrix4Unit , gF32Matrix4Unit , gF32Matrix4Unit,
	gF32Matrix4Unit , gF32Matrix4Unit , gF32Matrix4Unit , gF32Matrix4Unit,
	gF32Matrix4Unit , gF32Matrix4Unit , gF32Matrix4Unit , gF32Matrix4Unit,
	gF32Matrix4Unit , gF32Matrix4Unit , gF32Matrix4Unit , gF32Matrix4Unit,
	gF32Matrix4Unit , gF32Matrix4Unit , gF32Matrix4Unit , gF32Matrix4Unit,
	gF32Matrix4Unit , gF32Matrix4Unit , gF32Matrix4Unit , gF32Matrix4Unit,
	gF32Matrix4Unit , gF32Matrix4Unit , gF32Matrix4Unit , gF32Matrix4Unit,
	gF32Matrix4Unit , gF32Matrix4Unit , gF32Matrix4Unit , gF32Matrix4Unit,
	gF32Matrix4Unit , gF32Matrix4Unit , gF32Matrix4Unit , gF32Matrix4Unit,
	gF32Matrix4Unit , gF32Matrix4Unit , gF32Matrix4Unit , gF32Matrix4Unit,
	gF32Matrix4Unit , gF32Matrix4Unit , gF32Matrix4Unit , gF32Matrix4Unit,
	gF32Matrix4Unit , gF32Matrix4Unit , gF32Matrix4Unit , gF32Matrix4Unit,
	gF32Matrix4Unit , gF32Matrix4Unit , gF32Matrix4Unit , gF32Matrix4Unit,
	gF32Matrix4Unit , gF32Matrix4Unit , gF32Matrix4Unit , gF32Matrix4Unit,
	gF32Matrix4Unit , gF32Matrix4Unit , gF32Matrix4Unit , gF32Matrix4Unit,
	gF32Matrix4Unit , gF32Matrix4Unit , gF32Matrix4Unit , gF32Matrix4Unit,
	gF32Matrix4Unit , gF32Matrix4Unit , gF32Matrix4Unit , gF32Matrix4Unit,
	gF32Matrix4Unit , gF32Matrix4Unit , gF32Matrix4Unit , gF32Matrix4Unit,
	gF32Matrix4Unit , gF32Matrix4Unit , gF32Matrix4Unit , gF32Matrix4Unit,
	gF32Matrix4Unit , gF32Matrix4Unit , gF32Matrix4Unit , gF32Matrix4Unit,
	gF32Matrix4Unit , gF32Matrix4Unit , gF32Matrix4Unit , gF32Matrix4Unit,
	gF32Matrix4Unit , gF32Matrix4Unit , gF32Matrix4Unit , gF32Matrix4Unit,
	gF32Matrix4Unit , gF32Matrix4Unit , gF32Matrix4Unit , gF32Matrix4Unit,
	gF32Matrix4Unit , gF32Matrix4Unit , gF32Matrix4Unit , gF32Matrix4Unit,
	gF32Matrix4Unit , gF32Matrix4Unit , gF32Matrix4Unit , gF32Matrix4Unit,
	gF32Matrix4Unit , gF32Matrix4Unit , gF32Matrix4Unit , gF32Matrix4Unit,
	gF32Matrix4Unit , gF32Matrix4Unit , gF32Matrix4Unit , gF32Matrix4Unit,
	gF32Matrix4Unit , gF32Matrix4Unit , gF32Matrix4Unit , gF32Matrix4Unit,
	gF32Matrix4Unit , gF32Matrix4Unit , gF32Matrix4Unit , gF32Matrix4Unit,
	gF32Matrix4Unit , gF32Matrix4Unit , gF32Matrix4Unit , gF32Matrix4Unit,
	gF32Matrix4Unit , gF32Matrix4Unit , gF32Matrix4Unit , gF32Matrix4Unit,
	gF32Matrix4Unit , gF32Matrix4Unit , gF32Matrix4Unit , gF32Matrix4Unit,
	gF32Matrix4Unit , gF32Matrix4Unit , gF32Matrix4Unit , gF32Matrix4Unit,
	gF32Matrix4Unit , gF32Matrix4Unit , gF32Matrix4Unit , gF32Matrix4Unit,
	gF32Matrix4Unit , gF32Matrix4Unit , gF32Matrix4Unit , gF32Matrix4Unit,
	gF32Matrix4Unit , gF32Matrix4Unit , gF32Matrix4Unit , gF32Matrix4Unit,
	gF32Matrix4Unit , gF32Matrix4Unit , gF32Matrix4Unit , gF32Matrix4Unit,
	gF32Matrix4Unit , gF32Matrix4Unit , gF32Matrix4Unit , gF32Matrix4Unit,
	gF32Matrix4Unit , gF32Matrix4Unit , gF32Matrix4Unit , gF32Matrix4Unit,
	gF32Matrix4Unit , gF32Matrix4Unit , gF32Matrix4Unit , gF32Matrix4Unit,
	gF32Matrix4Unit , gF32Matrix4Unit , gF32Matrix4Unit , gF32Matrix4Unit,
	gF32Matrix4Unit , gF32Matrix4Unit , gF32Matrix4Unit , gF32Matrix4Unit,
	gF32Matrix4Unit , gF32Matrix4Unit , gF32Matrix4Unit , gF32Matrix4Unit,
	gF32Matrix4Unit , gF32Matrix4Unit , gF32Matrix4Unit , gF32Matrix4Unit,
	gF32Matrix4Unit , gF32Matrix4Unit , gF32Matrix4Unit , gF32Matrix4Unit,
	gF32Matrix4Unit , gF32Matrix4Unit , gF32Matrix4Unit , gF32Matrix4Unit,
	gF32Matrix4Unit , gF32Matrix4Unit , gF32Matrix4Unit , gF32Matrix4Unit,
	gF32Matrix4Unit , gF32Matrix4Unit , gF32Matrix4Unit , gF32Matrix4Unit,
	gF32Matrix4Unit , gF32Matrix4Unit , gF32Matrix4Unit , gF32Matrix4Unit,
	gF32Matrix4Unit , gF32Matrix4Unit , gF32Matrix4Unit , gF32Matrix4Unit,
	gF32Matrix4Unit , gF32Matrix4Unit , gF32Matrix4Unit , gF32Matrix4Unit,
	gF32Matrix4Unit , gF32Matrix4Unit , gF32Matrix4Unit , gF32Matrix4Unit,
	gF32Matrix4Unit , gF32Matrix4Unit , gF32Matrix4Unit , gF32Matrix4Unit,
	gF32Matrix4Unit , gF32Matrix4Unit , gF32Matrix4Unit , gF32Matrix4Unit,
	gF32Matrix4Unit , gF32Matrix4Unit , gF32Matrix4Unit , gF32Matrix4Unit,
	gF32Matrix4Unit , gF32Matrix4Unit , gF32Matrix4Unit , gF32Matrix4Unit,
	gF32Matrix4Unit , gF32Matrix4Unit , gF32Matrix4Unit , gF32Matrix4Unit,
	gF32Matrix4Unit , gF32Matrix4Unit , gF32Matrix4Unit , gF32Matrix4Unit,
	gF32Matrix4Unit , gF32Matrix4Unit , gF32Matrix4Unit , gF32Matrix4Unit,
	gF32Matrix4Unit , gF32Matrix4Unit , gF32Matrix4Unit , gF32Matrix4Unit,
	gF32Matrix4Unit , gF32Matrix4Unit , gF32Matrix4Unit , gF32Matrix4Unit,
	gF32Matrix4Unit , gF32Matrix4Unit , gF32Matrix4Unit , gF32Matrix4Unit,
};

void _ed3DLinkStripToViewport(ed_3d_strip* pStrip, ed_hash_code* pMBNK)
{
	ed_Chunck* pLAY;
	ed_Chunck* pTEX;
	edF32MATRIX4* peVar3;
	bool bVar4;
	edF32VECTOR4* peVar5;
	EVectorMode_A EVar6;
	ed_g2d_material* pMaterial;
	uint uVar8;
	//ed_g3d_Anim_def* peVar9;
	long lVar10;
	ed_3d_strip* puVar9;
	ed_g2d_bitmap* pBitMap;
	uint piVar12;
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
	float extraout_vf9w_01;
	float extraout_vf9w_02;
	float extraout_vf9w_03;
	float extraout_vf9w_04;
	float extraout_vf9w_05;
	edF32VECTOR4 eStack16;

	if ((pStrip->flags & 0x2000) != 0) {
		return;
	}
	
	if (((pStrip->flags & 0x10000) != 0) &&
		(gRender_info_SPR->pMeshTransformData->pAnimMatrix == (edF32MATRIX4*)0x0)) {
		return;
	}

	pMaterial = (ed_g2d_material*)0x0;
	if (pStrip->materialIndex != -1) {
		pMaterial = ed3DG2DGetG2DMaterialFromIndex(pMBNK, (int)pStrip->materialIndex);
		if (((gRender_info_SPR->pMeshTransformData != (ed_3d_hierarchy*)0x0) &&
			(gRender_info_SPR->pMeshTransformData->GlobalAlhaON != -1)) && (pMaterial != (ed_g2d_material*)0x0)) {
			pMaterial->pDMA_Material = 0x0;
		}
		in_vf9w = extraout_vf9w;
		if ((pMaterial != (ed_g2d_material*)0x0) && ((pMaterial->flags & 1) != 0)) {
			return;
		}
	}
	pStrip->pDMA_Matrix.pDMA_Matrix = 0x0;
	peVar5 = gBoundSphereCenter;
	if (gRender_info_SPR->boundingSphereTestResult == 8) {
		pStrip->flags = pStrip->flags | 3;
	LAB_00297ee8:
		bVar4 = true;
	}
	else {
		if (gRender_info_SPR->boundingSphereTestResult == 1) {
			pStrip->flags = pStrip->flags & 0xfffffffc;
			goto LAB_00297ee8;
		}
		if ((pStrip->flags & 0x10000) != 0) {
			pStrip->flags = pStrip->flags | 3;
			goto LAB_00297ee8;
		}
		fVar13 = (pStrip->boundingSphere).y;
		fVar14 = (pStrip->boundingSphere).z;
		gBoundSphereCenter->x = (pStrip->boundingSphere).x;
		peVar5->y = fVar13;
		peVar5->z = fVar14;
		gBoundSphereCenter->w = 1.0f;
		peVar5 = gBoundSphereCenter;
		peVar3 = gRender_info_SPR->pMeshTransformMatrix;
		fVar21 = gBoundSphereCenter->x;
		fVar22 = gBoundSphereCenter->y;
		fVar23 = gBoundSphereCenter->z;
		fVar24 = gBoundSphereCenter->w;
		fVar13 = peVar3->ab;
		fVar14 = peVar3->ac;
		fVar15 = peVar3->bb;
		fVar16 = peVar3->bc;
		fVar17 = peVar3->cb;
		fVar18 = peVar3->cc;
		fVar19 = peVar3->db;
		fVar20 = peVar3->dc;
		gBoundSphereCenter->x = peVar3->aa * fVar21 + peVar3->ba * fVar22 + peVar3->ca * fVar23 + peVar3->da * fVar24;
		peVar5->y = fVar13 * fVar21 + fVar15 * fVar22 + fVar17 * fVar23 + fVar19 * fVar24;
		peVar5->z = fVar14 * fVar21 + fVar16 * fVar22 + fVar18 * fVar23 + fVar20 * fVar24;
		peVar5->w = in_vf9w;
		uVar8 = ed3DTestBoundingSphereObject(&pStrip->boundingSphere);
		bVar4 = false;
		in_vf9w = extraout_vf9w_00;
		if (uVar8 != 4) {
			if (pStrip->pBoundSpherePkt == 0x0) {
				pStrip->flags = pStrip->flags & 0xfffffffc;
				if (uVar8 != 1) {
					uVar8 = ed3DTestBoundingSphereObjectNoZFar(&pStrip->boundingSphere);
					in_vf9w = extraout_vf9w_05;
					if (uVar8 == 1) {
						pStrip->flags = pStrip->flags & 0xfffffffc;
					}
					else {
						uVar8 = pStrip->flags;
						if ((uVar8 & 0x1000) == 0) {
							pStrip->flags = uVar8 | 3;
						}
						else {
							pStrip->flags = uVar8 | 1;
						}
					}
				}
			}
			else {
				if (uVar8 != 1) {
					pStrip->flags = pStrip->flags | 3;
					if ((pStrip->flags & 0x1000) == 0) {
						EVar6 = ed3DTestBoundingSphereZNearOnly(&pStrip->boundingSphere);
						if (EVar6 == VM_1) {
							pStrip->flags = pStrip->flags & 0xfffffffc;
							EVar6 = ed3DTestBoundingSphereSide(&pStrip->boundingSphere);
							bVar4 = true;
							in_vf9w = extraout_vf9w_02;
							if (EVar6 == VM_1) goto LAB_00297eec;
							pStrip->flags = pStrip->flags | 1;
							if ((pMaterial == (ed_g2d_material*)0x0) || ((pMaterial != (ed_g2d_material*)0x0 && (pMaterial->nbLayers < 2))))
							{
								lVar10 = ed3DCheckSpherePacket(pStrip);
								in_vf9w = extraout_vf9w_03;
								goto joined_r0x00297e70;
							}
						}
						else {
							if ((pMaterial == (ed_g2d_material*)0x0) ||
								((in_vf9w = extraout_vf9w_01, pMaterial != (ed_g2d_material*)0x0 && (pMaterial->nbLayers < 2)))) {
								lVar10 = ed3DCheckSpherePacket(pStrip);
								in_vf9w = extraout_vf9w_04;
							joined_r0x00297e70:
								bVar4 = false;
								if (lVar10 == 0) goto LAB_00297eec;
							}
						}
					}
					else {
						pStrip->flags = pStrip->flags & 0xfffffffd;
					}
				}
			}
			goto LAB_00297ee8;
		}
	}
LAB_00297eec:
	if (!bVar4) {
		return;
	}
	if ((pMaterial != (ed_g2d_material*)0x0) && (pMaterial->nbLayers != 0)) {
		// LAY
		pLAY = LOAD_SECTION_CAST(ed_Chunck*, pMaterial->aLayers[0]);
		ed_g2d_layer* pLayer = reinterpret_cast<ed_g2d_layer*>(pLAY + 1);
		uVar8 = pLayer->flags_0x0;
		piVar12 = pLayer->flags_0x4;
		pBitMap = 0;
		if (pLayer->bHasTexture != 0) {
			pTEX = LOAD_SECTION_CAST(ed_Chunck*, pLayer->pTex);
			ed_g2d_texture* pTexture = reinterpret_cast<ed_g2d_texture*>(pTEX + 1);
			pLayer->flags_0x4 = piVar12 & 0xfffffbff;
			peVar5 = gBoundSphereCenter;
			if ((piVar12 & 0x10) == 0) {
			LAB_0029800c:
				if ((piVar12 & 2) == 0) {
					pStrip->flags = pStrip->flags & 0xfffffdff;
				}
				else {
					IMPLEMENTATION_GUARD(
					peVar9 = ed3DG2DAnimTexGet(pTexture);
					ed3DManageAnim(peVar9);
					pStrip->flags = pStrip->flags | 0x200;)
				}
			}
			else {
				fVar14 = (pStrip->boundingSphere).y;
				fVar13 = (pStrip->boundingSphere).z;
				gBoundSphereCenter->x = (pStrip->boundingSphere).x;
				peVar5->y = fVar14;
				peVar5->z = fVar13;
				gBoundSphereCenter->w = 1.0;
				peVar5 = gBoundSphereCenter;
				peVar3 = gRender_info_SPR->pMeshTransformMatrix;
				fVar21 = gBoundSphereCenter->x;
				fVar22 = gBoundSphereCenter->y;
				fVar23 = gBoundSphereCenter->z;
				fVar24 = gBoundSphereCenter->w;
				fVar13 = peVar3->ab;
				fVar14 = peVar3->ac;
				fVar15 = peVar3->bb;
				fVar16 = peVar3->bc;
				fVar17 = peVar3->cb;
				fVar18 = peVar3->cc;
				fVar19 = peVar3->db;
				fVar20 = peVar3->dc;
				gBoundSphereCenter->x = peVar3->aa * fVar21 + peVar3->ba * fVar22 + peVar3->ca * fVar23 + peVar3->da * fVar24;
				peVar5->y = fVar13 * fVar21 + fVar15 * fVar22 + fVar17 * fVar23 + fVar19 * fVar24;
				peVar5->z = fVar14 * fVar21 + fVar16 * fVar22 + fVar18 * fVar23 + fVar20 * fVar24;
				peVar5->w = in_vf9w;
				fVar13 = pTexture->field_0x18;
				if (fVar13 <= 0.0f) {
					fVar13 = gAnimSTMaxDist;
				}
				if (-gBoundSphereCenter->z < fVar13) {
					bVar4 = true;
				}
				else {
					bVar4 = false;
					if (-gBoundSphereCenter->z - fVar13 < (pStrip->boundingSphere).w) {
						bVar4 = true;
					}
				}
				if (!bVar4) goto LAB_0029800c;
				pStrip->flags = pStrip->flags | 0x200;
			}
			if (pTexture->bHasPalette == 0) {
				ed_hash_code* pTextureHashCode = (ed_hash_code*)LOAD_SECTION(pTexture->hashCode.pData);
				if (pTextureHashCode != (ed_hash_code*)0x0) {
					ed_Chunck* pT2D = LOAD_SECTION_CAST(ed_Chunck*, pTextureHashCode->pData);
					pBitMap = reinterpret_cast<ed_g2d_bitmap*>(pT2D + 1);
				}
			}
			else {
				ed_hash_code* pPaletteHashCode = (ed_hash_code*)(pTexture + 1);

				pPaletteHashCode = (ed_hash_code*)LOAD_SECTION(pPaletteHashCode[pLayer->paletteId].pData);

				if (pPaletteHashCode->pData != 0) {
					ed_Chunck* pT2D = LOAD_SECTION_CAST(ed_Chunck*, pPaletteHashCode->pData);
					pBitMap = reinterpret_cast<ed_g2d_bitmap*>(pT2D + 1);
				}
			}

			if ((pStrip->flags & 4) != 0) {
				IMPLEMENTATION_GUARD(
				peVar9 = ed3DG3DAnimGet(pStrip);
				ed3DManageAnim(peVar9);
				)
			}
			goto LAB_002980e4;
		}
		if (((uint)piVar12 & 0x800) != 0) goto LAB_002980e4;
	}

	pBitMap = (ed_g2d_bitmap*)0;
	uVar8 = 0;
	piVar12 = 0;
	pMaterial = gDefault_Material_Current;
LAB_002980e4:
	ed3DLinkStripManageLinkToDMA(pStrip, uVar8, piVar12, &eStack16, pMaterial, pBitMap);
	return;
}

bool ed3DLinkStripShadowManageMaterial(ed_3d_strip* pStrip, ed_g2d_material** ppMaterial, uint* flagsA, uint* flagsB, ed_g2d_bitmap** ppBitmap)
{
	ed_g2d_material* pMaterial;
	ed_Chunck* pTEX;
	int iVar3;
	ed_hash_code* pTextureHashCode;
	ed_Chunck* pLAY;

	pMaterial = *ppMaterial;

	if ((pMaterial != (ed_g2d_material*)0x0) && (pMaterial->nbLayers != 0)) {
		pLAY = LOAD_SECTION_CAST(ed_Chunck*, pMaterial->aLayers[0]);
		ed_g2d_layer* pLayer = reinterpret_cast<ed_g2d_layer*>(pLAY + 1);

		*ppBitmap = (ed_g2d_bitmap*)0x0;
		*flagsA = pLayer->flags_0x0;
		*flagsB = pLayer->flags_0x4;

		if (((g3DFXFog.field_0x0 & 8) == 0) && (((*flagsB & 0x20) != 0 || ((*flagsB & 0x100) != 0)))) {
			return false;
		}

		if ((pLayer->bHasTexture != 0) && ((*gShadowRenderMask == 0 || ((((*flagsA & 0x100) != 0 && ((*ppMaterial)->nbLayers < 2)) || ((pStrip->flags & 0x10000) != 0)))))) {
			pStrip->flags = pStrip->flags & 0xfffffdff;
			pLayer->flags_0x4 = pLayer->flags_0x4 & 0xfffffbff;
			pTEX = LOAD_SECTION_CAST(ed_Chunck*, pLayer->pTex);
			ed_g2d_texture* pTexture = reinterpret_cast<ed_g2d_texture*>(pTEX + 1);

			if (pTexture->bHasPalette == 0) {
				IMPLEMENTATION_GUARD();
				// Check
				pTextureHashCode = LOAD_SECTION_CAST(ed_hash_code*, pTexture->hashCode.pData);

				if (pTextureHashCode != (ed_hash_code*)0x0) {
					ed_Chunck* pT2D = LOAD_SECTION_CAST(ed_Chunck*, pTextureHashCode->pData);
					*ppBitmap = reinterpret_cast<ed_g2d_bitmap*>(pT2D + 1);
				}
			}
			else {
				ed_hash_code* pPaletteHashCode = (ed_hash_code*)(pTexture + 1);
				pPaletteHashCode = LOAD_SECTION_CAST(ed_hash_code*, pPaletteHashCode[pLayer->paletteId].pData);

				if (pPaletteHashCode->pData != 0) {
					ed_Chunck* pT2D = LOAD_SECTION_CAST(ed_Chunck*, pPaletteHashCode->pData);
					*ppBitmap = reinterpret_cast<ed_g2d_bitmap*>(pT2D + 1);
				}
			}
			return true;
		}
	}

	*ppMaterial = gDefault_Material_Current;
	*ppBitmap = (ed_g2d_bitmap*)0x0;
	*flagsA = 0;
	*flagsB = 0;
	return true;
}

void _ed3DLinkStripShadowToViewport(ed_3d_strip* pStrip, ed_hash_code* pHashCode)
{
	edF32MATRIX4* peVar1;
	ed_dma_material* pDmaMaterial;
	int iVar3;
	edF32VECTOR4* peVar4;
	ed_g2d_material* peVar5;
	EVectorMode_A EVar6;
	bool bVar7;
	uint uVar8;
	long lVar9;
	edNODE* peVar10;
	undefined2* puVar11;
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
	float in_vf9w;
	float extraout_vf9w;
	edF32VECTOR4 eStack32;
	ed_g2d_bitmap* local_10;
	uint local_c;
	uint local_8;
	ed_g2d_material* local_4;

	pStrip->pDMA_Matrix.pDMA_Matrix = 0x0;
	if ((*gShadowRenderMask & (uint)(ushort)(pStrip->shadowCastFlags | pStrip->shadowReceiveFlags)) == 0) {
		return;
	}
	if (pStrip->materialIndex == -1) {
		local_4 = (ed_g2d_material*)0x0;
	}
	else {
		local_4 = ed3DG2DGetG2DMaterialFromIndex(pHashCode, (int)pStrip->materialIndex);
		if ((local_4 != (ed_g2d_material*)0x0) && ((local_4->flags & 1) != 0)) {
			return;
		}
		in_vf9w = extraout_vf9w;
		if ((local_4 != (ed_g2d_material*)0x0) && (1 < local_4->nbLayers)) {
			return;
		}
	}
	peVar5 = local_4;
	if (((pStrip->flags & 0x10000) != 0) &&
		(gRender_info_SPR->pMeshTransformData->pAnimMatrix == (edF32MATRIX4*)0x0)) {
		return;
	}
	pStrip->pDMA_Matrix.pDMA_Matrix = 0x0;
	peVar4 = gBoundSphereCenter;
	if (gRender_info_SPR->boundingSphereTestResult == 8) {
		pStrip->flags = pStrip->flags | 3;
	}
	else {
		if (gRender_info_SPR->boundingSphereTestResult == 1) {
			pStrip->flags = pStrip->flags & 0xfffffffc;
		}
		else {
			if ((pStrip->flags & 0x10000) == 0) {

				gBoundSphereCenter->xyz = pStrip->boundingSphere.xyz;
				gBoundSphereCenter->w = 1.0f;
				*gBoundSphereCenter = (*gBoundSphereCenter) * (*gRender_info_SPR->pMeshTransformMatrix);

				uVar8 = ed3DTestBoundingSphereObject(&pStrip->boundingSphere);
				bVar7 = false;
				if (uVar8 == 4) goto LAB_00298420;
				if (pStrip->pBoundSpherePkt == 0x0) {
					pStrip->flags = pStrip->flags & 0xfffffffc;
					if (uVar8 != 1) {
						uVar8 = ed3DTestBoundingSphereObjectNoZFar(&pStrip->boundingSphere);
						if (uVar8 == 1) {
							pStrip->flags = pStrip->flags & 0xfffffffc;
						}
						else {
							uVar8 = pStrip->flags;
							if ((uVar8 & 0x1000) == 0) {
								pStrip->flags = uVar8 | 3;
							}
							else {
								pStrip->flags = uVar8 | 1;
							}
						}
					}
				}
				else {
					if (uVar8 != 1) {
						pStrip->flags = pStrip->flags | 3;
						if ((pStrip->flags & 0x1000) == 0) {
							EVar6 = ed3DTestBoundingSphereZNearOnly(&pStrip->boundingSphere);
							if (EVar6 == VM_1) {
								pStrip->flags = pStrip->flags & 0xfffffffc;
								EVar6 = ed3DTestBoundingSphereSide(&pStrip->boundingSphere);
								bVar7 = true;
								if (EVar6 == VM_1) goto LAB_00298420;
								pStrip->flags = pStrip->flags | 1;
								if ((peVar5 == (ed_g2d_material*)0x0) ||
									((peVar5 != (ed_g2d_material*)0x0 && (peVar5->nbLayers < 2)))) {
									lVar9 = ed3DCheckSpherePacket(pStrip);
									goto joined_r0x002983a4;
								}
							}
							else {
								if ((peVar5 == (ed_g2d_material*)0x0) ||
									((peVar5 != (ed_g2d_material*)0x0 && (peVar5->nbLayers < 2)))) {
									lVar9 = ed3DCheckSpherePacket(pStrip);
								joined_r0x002983a4:
									bVar7 = false;
									if (lVar9 == 0) goto LAB_00298420;
								}
							}
						}
						else {
							pStrip->flags = pStrip->flags & 0xfffffffd;
						}
					}
				}
			}
			else {
				pStrip->flags = pStrip->flags | 3;
			}
		}
	}
	bVar7 = true;
LAB_00298420:
	if (bVar7) {
		if ((*gShadowRenderMask & (uint)(ushort)pStrip->shadowCastFlags) != 0) {
			bVar7 = ed3DLinkStripShadowManageMaterial(pStrip, &local_4, &local_8, &local_c, &local_10);
			if (bVar7 == false) {
				return;
			}
			ed3DLinkStripManageLinkToDMA(pStrip, local_8, local_c, &eStack32, local_4, local_10);
		}
		if ((*gShadowRenderMask & (uint)(ushort)pStrip->shadowReceiveFlags) != 0) {

			pDmaMaterial = (ed_dma_material*)(gPrim_List_Shadow[gCurRenderList].pPrev)->pPrev->pData;
			if ((pprevious_shadow_dma_matrix == (ed_dma_matrix*)0x0) ||(pprevious_shadow_dma_matrix->pObjToWorld != gRender_info_SPR->pSharedMeshTransform)) {

				pprevious_shadow_dma_matrix = ed3DListCreateDmaMatrixNode(gRender_info_SPR, gRender_info_SPR->pMeshTransformData);

				edNODE_MANAGER* pvVar2 = (edNODE_MANAGER*)(pDmaMaterial->list).pData;
				edNODE* peVar8 = pvVar2->pNodeHead + pvVar2->linkCount;
				peVar8->pData = pprevious_shadow_dma_matrix;
				peVar8->header.typeField.type = LIST_TYPE_MATRIX;
				pvVar2->linkCount = pvVar2->linkCount + 1;
				(pDmaMaterial->list).nodeCount = (pDmaMaterial->list).nodeCount + 1;
				peVar8->pPrev = (pDmaMaterial->list).pPrev;
				(pDmaMaterial->list).pPrev = peVar8;

				edNODE_MANAGER* pvVar3 = (edNODE_MANAGER*)pprevious_shadow_dma_matrix->pData;
				peVar8 = pvVar3->pNodeHead + pvVar3->linkCount;
				peVar8->pData = pStrip;
				peVar8->header.typeField.type = LIST_TYPE_STRIP;
				peVar8->header.typeField.flags = (short)pStrip->flags;
				pvVar3->linkCount = pvVar3->linkCount + 1;
				pprevious_shadow_dma_matrix->nodeCount = pprevious_shadow_dma_matrix->nodeCount + 1;
				peVar8->pPrev = pprevious_shadow_dma_matrix->pPrev;

				peVar8->pNext = (edNODE*)pStrip->pDMA_Matrix.pDMA_Matrix;
				pprevious_shadow_dma_matrix->pPrev = peVar8;
			}
			else {
				edNODE_MANAGER* pvVar4 = (edNODE_MANAGER*)pprevious_shadow_dma_matrix->pData;
				edNODE* peVar8 = pvVar4->pNodeHead + pvVar4->linkCount;
				peVar8->pData = pStrip;
				peVar8->header.typeField.type = LIST_TYPE_STRIP;
				peVar8->header.typeField.flags = (short)pStrip->flags;
				pvVar4->linkCount = pvVar4->linkCount + 1;
				pprevious_shadow_dma_matrix->nodeCount = pprevious_shadow_dma_matrix->nodeCount + 1;
				peVar8->pPrev = pprevious_shadow_dma_matrix->pPrev;
				peVar8->pNext = (edNODE*)pStrip->pDMA_Matrix.pDMA_Matrix;
				pprevious_shadow_dma_matrix->pPrev = peVar8;
			}
		}
	}
	return;
}

void ed3DRenderObject(ed_hash_code* pLOD, ed_hash_code* pMBNK)
{
	bool bVar1;
	edF32MATRIX4* pMVar2;
	edF32VECTOR4* pVVar3;
	uint uVar4;
	int iVar5;
	edF32VECTOR4* pSphere;
	int iVar7;
	int meshCount;
	ed_3d_strip* pStrip;
	float fVar8;
	float fVar9;
	float fVar10;
	float fVar11;
	float fVar12;
	float fVar13;
	float fVar14;
	float fVar15;
	float x;
	float y;
	float z;
	float w;
	float in_vf9w;
	ed_g3d_object* pMeshOBJ;

	ED3D_LOG(LogLevel::Verbose, "ed3DRenderObject mesh: {} -> MBNK: {}", pLOD->hash.ToString(), pMBNK->hash.ToString());

	// HACK
	//if (pObjHash->hash.number == 0x2e442e4f2e4c) {
	//	
	//}
	//else {
	//	return;
	//}

	ed_Chunck* pOBJ = LOAD_SECTION_CAST(ed_Chunck*, pLOD->pData);
	pMeshOBJ = reinterpret_cast<ed_g3d_object*>(pOBJ + 1);
	pStrip = LOAD_SECTION_CAST(ed_3d_strip*, pMeshOBJ->p3DStrip);
	if (pStrip == (ed_3d_strip*)0x0) {
		return;
	}
	meshCount = pMeshOBJ->stripCount;
	if ((gRender_info_SPR->flags & 0x10) != 0) {
		pSphere = gRender_info_SPR->pHierarchySetup->pBoundingSphere;
		iVar7 = 8;
		if (pSphere != (edF32VECTOR4*)0x0) goto LAB_002b0d68;
	}
	pSphere = &pMeshOBJ->boundingSphere;
	iVar7 = 2;
LAB_002b0d68:
	gBoundSphereCenter->xyz = pSphere->xyz;
	gBoundSphereCenter->w = 1.0f;

	/* Leaving this as a reference for the matrix multiplication
		pSphere = &pMeshOBJ->boundingSphere;
		peVar3 = gBoundSphereCenter;
		fVar6 = pSphere->y;
		fVar7 = pSphere->z;
		gBoundSphereCenter->x = pSphere->x;
		peVar3->y = fVar6;
		peVar3->z = fVar7;
		gBoundSphereCenter->w = 1.0;
		peVar3 = gBoundSphereCenter;
		peVar2 = gRender_info_SPR->pMeshTransformMatrix;
		fVar14 = gBoundSphereCenter->x;
		fVar15 = gBoundSphereCenter->y;
		fVar16 = gBoundSphereCenter->z;
		fVar17 = gBoundSphereCenter->w;
		fVar6 = peVar2->ab;
		fVar7 = peVar2->ac;
		fVar8 = peVar2->bb;
		fVar9 = peVar2->bc;
		fVar10 = peVar2->cb;
		fVar11 = peVar2->cc;
		fVar12 = peVar2->db;
		fVar13 = peVar2->dc;
		gBoundSphereCenter->x = peVar2->aa * fVar14 + peVar2->ba * fVar15 + peVar2->ca * fVar16 + peVar2->da * fVar17;
		peVar3->y = fVar6 * fVar14 + fVar8 * fVar15 + fVar10 * fVar16 + fVar12 * fVar17;
		peVar3->z = fVar7 * fVar14 + fVar9 * fVar15 + fVar11 * fVar16 + fVar13 * fVar17;
		peVar3->w = in_vf9w;	
	*/

	*gBoundSphereCenter = (*gBoundSphereCenter) * (*gRender_info_SPR->pMeshTransformMatrix);

	uVar4 = ed3DTestBoundingSphereObject(pSphere);
	ED3D_LOG(LogLevel::Verbose, "ed3DRenderObject ed3DTestBoundingSphereObject result: {} (x: {}, y: {} z: {}, w: {})", uVar4, pSphere->x, pSphere->y, pSphere->z, pSphere->w);
	if (uVar4 != 4) {
		if (uVar4 == 1) {
			gRender_info_SPR->boundingSphereTestResult = 1;
		}
		else {
			iVar5 = ed3DTestBoundingSphereObjectNoZFar(pSphere);
			if (iVar5 != 1) {
				gRender_info_SPR->boundingSphereTestResult = iVar7;
			}
		}
		if (*gShadowRenderMask == 0) {
				while (bVar1 = meshCount != 0, meshCount = meshCount + -1, bVar1) {
					_ed3DLinkStripToViewport(pStrip, pMBNK);
					pStrip = (ed_3d_strip*)LOAD_SECTION(pStrip->pNext);
				}
		}
		else {
				while (bVar1 = meshCount != 0, meshCount = meshCount + -1, bVar1) {
					_ed3DLinkStripShadowToViewport(pStrip, pMBNK);
					pStrip = (ed_3d_strip*)LOAD_SECTION(pStrip->pNext);
				}
		}
	}
	return;
}

void ed3DRenderSonHierarchy(ed_3d_hierarchy* pHierarchy)
{
	short sVar1;
	ed_hash_code* pObjHash;
	ed3DLod* pLod;
	edF32MATRIX4* pMVar3;
	edF32MATRIX4 MStack64;

	ED3D_LOG(LogLevel::Verbose, "ed3DRenderSonHierarchy {}", pHierarchy->hash.ToString());

	std::string aoeuoaeu = "‡ž¤²™NFL";
	std::string oeuthoeu = pHierarchy->hash.name;
	if (aoeuoaeu == oeuthoeu)
	{
		ED3D_LOG(LogLevel::Verbose, "ed3DRenderSonHierarchy {}", pHierarchy->hash.ToString());
	}

	if (((pHierarchy->flags_0x9e & 0x41) == 0) && (pHierarchy->lodCount != 0)) {
		pLod = ed3DChooseGoodLOD(pHierarchy);
		if (pLod != (ed3DLod*)0x0) {
			if ((pHierarchy->flags_0x9e & 0x80) == 0) {
				pHierarchy->desiredLod = 0xff;
			}

			edF32Matrix4MulF32Matrix4Hard(&MStack64, &pHierarchy->transformB, WorldToCamera_Matrix);
			pMVar3 = &pHierarchy->transformB;
			gRender_info_SPR->pMeshTransformMatrix = &MStack64;
			gRender_info_SPR->pMeshTransformData = pHierarchy;
			gRender_info_SPR->pHierarchySetup = pHierarchy->pHierarchySetup;
			gRender_info_SPR->pSharedMeshTransform = pMVar3;
			gRender_info_SPR->flags = pHierarchy->flags_0x9e;

			ED3D_LOG(LogLevel::Verbose, "ed3DRenderSonHierarchy flags: 0x{:x}", gRender_info_SPR->flags);

			if ((((uint)pHierarchy->flags_0x9e & 0x10) != 0)) {
				assert(pHierarchy->pHierarchySetup != NULL);
			}

			//assert((((uint)pMeshTransformData->flags_0x9e & 0x10) == 0) || pMeshTransformData->pHierarchySetup != NULL);

			gRender_info_SPR->biggerScale = ed3DMatrixGetBigerScale(pMVar3);
			gRender_info_SPR->pPkt = 0;
			gRender_info_SPR->boundingSphereTestResult = 1;
			pHierarchy->pMatrixPkt = (edpkt_data*)0x0;
			pObjHash = (ed_hash_code*)LOAD_SECTION(pLod->pObj);
			if (((pObjHash != (ed_hash_code*)0x0) && (sVar1 = pLod->renderType, sVar1 != 2)) && (sVar1 != 1)) {
				if (sVar1 == 3) {
					IMPLEMENTATION_GUARD_LOG(ed3DRenderSprite(pObjHash, pHierarchy->pTextureInfo + 0x10));
				}
				else {
					if (sVar1 == 0) {
						ed3DRenderObject(pObjHash, (ed_hash_code*)(pHierarchy->pTextureInfo + 0x10));
					}
				}
			}
		}

		ed3DSetSceneRender(pHierarchy, 1);
	}
	return;
}

void ed3DRenderSonHierarchyForShadow(ed_3d_hierarchy* pHierarchy)
{
	char cVar1;
	ushort uVar2;
	short sVar3;
	ed3DLod* pLod;
	MeshHeader** ppMVar5;
	edF32MATRIX4* pMVar6;
	float fVar7;
	edF32MATRIX4 MStack64;

	ED3D_LOG(LogLevel::Verbose, "ed3DRenderSonHierarchyForShadow {}", pHierarchy->hash.ToString());

	uVar2 = pHierarchy->flags_0x9e;
	if (((((uVar2 & 0x200) != 0) &&
		((*gShadowRenderMask & (uint)pHierarchy->bRenderShadow) != 0)) &&
		((uVar2 & 0x41) == 0)) && (pHierarchy->lodCount != 0)) {
		pLod = ed3DChooseGoodLOD(pHierarchy);
		if ((pHierarchy->flags_0x9e & 0x80) == 0) {
			pHierarchy->desiredLod = 0xff;
		}
		if ((pLod != (ed3DLod*)0x0) && (pLod->pObj != 0x0)) {
			edF32Matrix4MulF32Matrix4Hard(&MStack64, &pHierarchy->transformB,
				WorldToCamera_Matrix);
			pMVar6 = &pHierarchy->transformB;
			gRender_info_SPR->pMeshTransformMatrix = &MStack64;
			gRender_info_SPR->pMeshTransformData = pHierarchy;
			gRender_info_SPR->pHierarchySetup = pHierarchy->pHierarchySetup;
			gRender_info_SPR->pSharedMeshTransform = pMVar6;
			gRender_info_SPR->flags = (uint)pHierarchy->flags_0x9e;

			ED3D_LOG(LogLevel::Verbose, "ed3DRenderSonHierarchyForShadow flags: 0x%x", gRender_info_SPR->flags);

			if ((((uint)pHierarchy->flags_0x9e & 0x10) != 0)) {
				assert(pHierarchy->pHierarchySetup != NULL);
			}

			fVar7 = ed3DMatrixGetBigerScale(pMVar6);
			gRender_info_SPR->biggerScale = fVar7;
			gRender_info_SPR->pPkt = 0;
			pHierarchy->pMatrixPkt = (edpkt_data*)0x0;
			sVar3 = pLod->renderType;
			if (sVar3 == 3) {
				IMPLEMENTATION_GUARD(
					MeshHeader::ReadRenderTransform_002b0e80
					(pLod->pObj, pHierarchy->pTextureInfo + 0x10);)
			}
			else {
				if (((sVar3 != 2) && (sVar3 != 1)) && (sVar3 == 0)) {
					if ((pHierarchy->flags_0x9e & 0x100) == 0) {
						ED3D_LOG(LogLevel::Verbose, "ed3DRenderSonHierarchyForShadow Get texture hash code 0x{:x} -> 0x{:x}", (uintptr_t)pHierarchy, (uintptr_t)pHierarchy->pTextureInfo);
						ed3DRenderObject((ed_hash_code*)LOAD_SECTION(pLod->pObj), (ed_hash_code*)(pHierarchy->pTextureInfo + 0x10));
					}
					else {
						cVar1 = pHierarchy->GlobalAlhaON;
						pHierarchy->GlobalAlhaON = -1;
						ed3DLod* peVar3 = ed3DHierarcGetLOD((ed_g3d_hierarchy*)pHierarchy, pHierarchy->lodCount - 1);
						if (peVar3 != (ed3DLod*)0x0) {
							ed3DRenderObject((ed_hash_code*)LOAD_SECTION(peVar3->pObj), (ed_hash_code*)(pHierarchy->pTextureInfo + 0x10));
						}
						pHierarchy->GlobalAlhaON = cVar1;
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

void ed3DSceneRenderOne(ed_3D_Scene* pShadowScene, ed_3D_Scene* pScene)
{
	ed_g3d_manager* pMeshInfo;
	float fVar1;
	ed_viewport* pCVar2;
	int iVar3;
	bool bVar4;
	uint uVar5;
	uint uVar6;
	//ProfileObject* pPVar7;
	edNODE* pClusterNode;
	edNODE* pCVar9;
	ed_3d_extra_stuff_param renderTaskData;
	edNODE* pCVar8;

	ZONE_SCOPED;

	ED3D_LOG(LogLevel::VeryVerbose, "\n--------------------------------------------");
	ED3D_LOG(LogLevel::VeryVerbose, "ed3DSceneRenderOne Scene 0x{:x} ({}) (shadow: {})", (uintptr_t)pShadowScene, gSceneNames[GetStaticMeshMasterIndex(pShadowScene)], pShadowScene->bShadowScene);
	ED3D_LOG(LogLevel::VeryVerbose, "ed3DSceneRenderOne Shadow Scene 0x{:x} ({}) (shadow: {})", (uintptr_t)pScene, gSceneNames[GetStaticMeshMasterIndex(pScene)], pScene->bShadowScene);

	RENDER_LABEL_BEGIN("ed3DSceneRenderOne");

	iVar3 = gIDProfileFlush;
	if ((pShadowScene->bShadowScene == 1) && ((pScene->flags & 0x10) != 0)) {
		ED3D_LOG(LogLevel::VeryVerbose, "ed3DSceneRenderOne SKIP");
		uVar6 = 0;
	}
	else {
		ED3D_LOG(LogLevel::VeryVerbose, "ed3DSceneRenderOne DRAW");
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

		uVar6 = ed3DInitRenderEnvironement(pShadowScene, (ulong)((pShadowScene->flags & 0x20) != 0));
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
		if ((uVar6 & 0xff) != 0) {
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

			pCVar8 = (edNODE*)pScene->pHierListA;
			pCVar9 = pCVar8->pPrev;
			if (*gShadowRenderMask == 0) {
				if (ged3DConfig.field_0x2d == 0) {
					for (; pCVar9 != pCVar8; pCVar9 = pCVar9->pPrev) {
						ed3DRenderSonHierarchy((ed_3d_hierarchy*)pCVar9->pData);
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
					ed3DRenderSonHierarchyForShadow((ed_3d_hierarchy*)pCVar9->pData);
				}
			}

			ed3DClustersRenderComputeSceneHierarchyList(pShadowScene);

			if ((pShadowScene->bShadowScene != 1) && (ed3DSceneSortClusters(pShadowScene), (pShadowScene->flags & 8) == 0)) {
				for (pClusterNode = (edNODE*)(pScene->meshClusterList).pPrev;
					(edLIST*)pClusterNode != &pScene->meshClusterList; pClusterNode = pClusterNode->pPrev) {
					edCluster* pCluster = (edCluster*)pClusterNode->pData;
					if (((pCluster->flags & 2) == 0) && (pMeshInfo = pCluster->pMeshInfo, pMeshInfo->CSTA != (ed_Chunck*)0x0)) {
						ed3DSceneRenderCluster(pMeshInfo);
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

	RENDER_LABEL_END("ed3DSceneRenderOne");
	return;
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
	ed_3d_extra_stuff_param renderTaskData;

	ED3D_LOG(LogLevel::VeryVerbose, "ed3DSceneRenderDlist 0x{:x} ({}) (child: {})", (uintptr_t)pStaticMeshMaster, GetStaticMeshMasterIndex(pStaticMeshMaster), pStaticMeshMaster->bShadowScene);

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
	uVar8 = ed3DInitRenderEnvironement(pStaticMeshMaster, (ulong)((pStaticMeshMaster->flags & 0x20) != 0));
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
		ED3D_LOG(LogLevel::VeryVerbose, "ed3DPrimlistTermMaterialRenderList {}", (uintptr_t)(gPrim_List_FlushTex + gCurRenderList));
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

	if (gLightDirections_Matrix != (edF32MATRIX4*)0x0) {
		*gLightDirections_Matrix_Scratch = *gLightDirections_Matrix;
	}

	if (gLightColor_Matrix != (edF32MATRIX4*)0x0) {
		*gLightColor_Matrix_Scratch = *gLightColor_Matrix;
	}

	if (gLightAmbiant != (edF32VECTOR4*)0x0) {
		*gLightAmbiant_Scratch = *gLightAmbiant;
	}

	*gVU1_AnimST_NormalExtruder_Scratch = Vector_0048c3d0;

	*gF32Matrix4Unit_Scratch = gF32Matrix4Unit;

	*gbDoMipmap_SPR = (uint)gbDoMipmap;
	*gMipmapL_SPR = gMipmapL;
	*gMipmapK_SPR = gMipmapK;
	*gbTrilinear_SPR = (uint)BYTE_0044941c;
	pAVar1 = PTR_AnimScratchpad_00449554;
	PTR_AnimScratchpad_00449554->vuFlags = 0;
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

	// HACK: This is a hack to prevent the game from rendering.
	//return;

	RENDER_LABEL_BEGIN("ed3DSceneRender");
	ZONE_SCOPED;

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

		// Up to scene.
#ifdef PLATFORM_WIN
		//VU1Emu::UpdateMemory(g_pStartPktData, g_VifRefPktCur);
#endif

		for (staticMeshMasterIndex = 0; staticMeshMasterIndex < (uint)ged3DConfig.sceneCount;
			staticMeshMasterIndex = staticMeshMasterIndex + 1) {

#ifdef PLATFORM_WIN
			// Starting for each scene.
			g_pStartPortPktData = g_VifRefPktCur;
#endif

			staticMeshMasterFlags = gScene3D[staticMeshMasterIndex].flags;
			if ((((staticMeshMasterFlags & 1) != 0) && ((staticMeshMasterFlags & 4) == 0)) &&
				(gScene3D[staticMeshMasterIndex].bShadowScene == 0)) {
				if (bVar3) {
					bVar3 = false;
					pFVar6 = edVideoGetDrawSurface();
					if ((pFVar6 != (edSurface*)0x0) && (gpPKTDataRefMasPath3 != (edpkt_data*)0x0)) {
						g_VifRefPktCur->cmdA = ED_VIF1_SET_TAG_REF(0, 0);
						g_VifRefPktCur->cmdB = 0x1300000000000000;
						g_VifRefPktCur->asU32[2] = SCE_VIF1_SET_NOP(0);
						g_VifRefPktCur->asU32[3] = SCE_VIF1_SET_FLUSHA(0);
						edpkt_data* pRVar5 = g_VifRefPktCur;
						g_VifRefPktCur = g_VifRefPktCur + 1;

						*g_VifRefPktCur = gpPKTDataRefMasPath3[0];

						pRVar5 = g_VifRefPktCur;
						g_VifRefPktCur = g_VifRefPktCur + 1;

						*g_VifRefPktCur = gpPKTDataRefMasPath3[1];

						pRVar5 = g_VifRefPktCur;
						g_VifRefPktCur = g_VifRefPktCur + 1;

						*g_VifRefPktCur = gpPKTDataRefMasPath3[2];

						pRVar5 = g_VifRefPktCur;
						g_VifRefPktCur = g_VifRefPktCur + 1;

						*g_VifRefPktCur = gpPKTDataRefMasPath3[3];

						g_VifRefPktCur = g_VifRefPktCur + 1;

						gpPKTDataRefMasPath3->asU32[0] = SCE_VIF1_SET_STMASK(0);
						gpPKTDataRefMasPath3->asU32[1] = (int)(gpPKTDataRefMasPath3 + 4) & 0xfffffffU;
						gpPKTDataRefMasPath3->cmdB = 0;

						g_VifRefPktCur->cmdA = ED_VIF1_SET_TAG_REF(0, 0);
						g_VifRefPktCur->asU32[2] = SCE_VIF1_SET_NOP(0);
						g_VifRefPktCur->asU32[3] = SCE_VIF1_SET_FLUSHA(0);

						g_VifRefPktCur[1].cmdA = ED_VIF1_SET_TAG_REF(0, 0);
						g_VifRefPktCur[1].asU32[2] = SCE_VIF1_SET_FLUSH(0);
						g_VifRefPktCur[1].asU32[3] = SCE_VIF1_SET_FLUSH(0);

						g_VifRefPktCur[2].cmdA = ED_VIF1_SET_TAG_REF(0, 0);
						g_VifRefPktCur[2].asU32[2] = SCE_VIF1_SET_NOP(0);
						g_VifRefPktCur[2].asU32[3] = SCE_VIF1_SET_MSKPATH3(0, 0);

						g_VifRefPktCur[3].cmdA = ED_VIF1_SET_TAG_CNT(1);
						g_VifRefPktCur[3].cmdB = 0;

						pRVar5 = g_VifRefPktCur;
						g_VifRefPktCur[4].asU128 = 0;

						g_VifRefPktCur[5].cmdA = ED_VIF1_SET_TAG_REF(0, 0);
						g_VifRefPktCur[5].asU32[2] = SCE_VIF1_SET_NOP(0);
						g_VifRefPktCur[5].asU32[3] = SCE_VIF1_SET_MSKPATH3(0x8000, 0);
						g_VifRefPktCur = g_VifRefPktCur + 6;
					}
				}
				pRVar5 = g_VifRefPktCur;
				pInStaticMeshMaster = gScene3D + staticMeshMasterIndex;

				ED3D_LOG(LogLevel::VeryVerbose, "ed3DSceneRender Processing 0x{:x} ({})", (uintptr_t)pInStaticMeshMaster, staticMeshMasterIndex);

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

				ED3D_LOG(LogLevel::VeryVerbose, "ed3DSceneRender Begin Processing Children");
				for (pCVar12 = (edLIST*)(pInStaticMeshMaster->meshClusterShadowList).pPrev;
					pCVar12 != &pInStaticMeshMaster->meshClusterShadowList; pCVar12 = (edLIST*)pCVar12->pPrev) {
					if ((((ed_3D_Scene*)pCVar12->pData)->flags & 4) == 0) {
						ed3DSceneRenderOne((ed_3D_Scene*)pCVar12->pData, pInStaticMeshMaster);
					}
				}
				ED3D_LOG(LogLevel::VeryVerbose, "ed3DSceneRender End Processing Children");

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
					if ((pInStaticMeshMaster->flags & SCENE_FLAG_FOG_PROPERTY) != 0) {
						bVar3 = true;
						ed3DFlushFogFX();
					}
					if ((pInStaticMeshMaster->flags & 0x800) == 0) {
						if ((pInStaticMeshMaster->flags & 0x80) != 0) {
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
			g_VifRefPktCur->asU32[2] = SCE_VIF1_SET_MSKPATH3(0, 0);
			g_VifRefPktCur->asU32[3] = SCE_VIF1_SET_NOP(0);

			g_VifRefPktCur = g_VifRefPktCur + 1;
			g_VifRefPktCur->cmdA = ED_VIF1_SET_TAG_CNT(2);
			g_VifRefPktCur->asU32[2] = SCE_VIF1_SET_NOP(0);
			g_VifRefPktCur->asU32[3] = SCE_VIF1_SET_DIRECT(2, 0);

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
			g_VifRefPktCur->cmdA = SCE_GS_SET_ALPHA_1(
				SCE_GS_ALPHA_ZERO,	// A
				SCE_GS_ALPHA_ZERO,	// B
				SCE_GS_ALPHA_ZERO,	// C
				SCE_GS_ALPHA_CS,	// D
				0					// FIX
			);
			g_VifRefPktCur->cmdB = SCE_GS_ALPHA_1;

			g_VifRefPktCur = g_VifRefPktCur + 1;
			g_VifRefPktCur = ed3DDMAAddFinish(g_VifRefPktCur);

			g_VifRefPktCur->cmdB = 0x0;
			sceDmaAddEnd((sceDmaTag**)&g_VifRefPktCur, 0, 0);
			if (gpPKTDataRefMasPath3 != (edpkt_data*)0x0) {
				gpPKTDataRefMasPath3->asU32[0] = SCE_VIF1_SET_STMASK(0);
				gpPKTDataRefMasPath3->asU32[1] = (int)(gpPKTDataRefMasPath3 + 4) & 0xfffffffU;
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

	RENDER_LABEL_END("ed3DSceneRender");
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
	ed_hash_code* pTVar1;
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
	ed_g2d_layer* pLayer;
	ed_g2d_bitmap* peVar12;
	uint uVar13;
	ed_g2d_texture* pTexture;
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
	ed_Chunck* pLAY;

	puVar8 = 0x10;
	pTexture = (ed_g2d_texture*)0x0;
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
		if (pBuffer->nbLayers == 0) {
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
			for (uVar11 = 0; uVar11 < pBuffer->nbLayers; uVar11 = uVar11 + 1) {
				ed_hash_code* pHash = (ed_hash_code*)(pBuffer + 1);
				pLAY = LOAD_SECTION_CAST(ed_Chunck*, pHash[uVar11].pData);
				pLayer = reinterpret_cast<ed_g2d_layer*>(pLAY + 1);

				if ((pLayer != (ed_g2d_layer*)0x0) && (pLayer->bHasTexture != 0)) {
					ed_Chunck* pTEX = LOAD_SECTION_CAST(ed_Chunck*, pLayer->pTex);
					pTexture = reinterpret_cast<ed_g2d_texture*>(pTEX + 1);
				}

				if (pTexture == (ed_g2d_texture*)0x0) {
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
					pTVar1 = (ed_hash_code*)LOAD_SECTION(pTexture->hashCode.pData);
					if (pTVar1 != (ed_hash_code*)0x0) {
						ed_Chunck* pPA32 = LOAD_SECTION_CAST(ed_Chunck*, pTVar1->pData);
						peVar12 = reinterpret_cast<ed_g2d_bitmap*>(pPA32 + 1);
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
						if (fVar15 < 2.147484e+09f) {
							local_4 = (uint)fVar15;
						}
						else {
							local_4 = (int)(fVar15 - 2.147484e+09f) | 0x80000000;
						}
						if (peVar12->psm == 0x18) {
							local_40 = local_8 * local_4 * 4 >> 8;
						}
						else {
							local_40 = (uint)peVar12->psm * local_8 * local_4 >> 0xb;
						}
						uVar2 = gLayerProp;
						if (pLayer != (ed_g2d_layer*)0x0) {
							uVar2 = gLayerProp | pLayer->flags_0x0;
							pLayer->flags_0x0 = uVar2;
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
						bVar4 = ed3DFXIsTrue(pLayer);
						if (bVar4 == false) {
							uVar7 = ed3DSetBasicTexPacket(pStripBuf + 2, &local_20, peVar12, pLayer);
						}
						else {
							puVar5 = (undefined4*)ed3DGetFxConfigFromFlag(uVar2);
							local_30 = puVar5[1];
							local_2c = *puVar5;
							local_28 = *puVar5;
							uVar7 = ed3DSetBasicTexPacket(pStripBuf + 2, &local_30, peVar12, pLayer);
						}
						puVar6 = (ulong*)ed3DSetAlphaPacket(uVar7, (uVar2 & 0x2000) != 0, pLayer, uVar11);
						*puVar6 = (long)(int)gMipmapK << 0x20 |
							(long)(int)(peVar12->field_0x6 - 1) << 2 | 0x60U | (ulong)gMipmapL << 0x13;
						puVar6[1] = 0x14;
						puVar6 = ed3DSetMipmapPacket(puVar6 + 2, (int)&local_20, (ushort*)peVar12);
						pStripBuf = (edpkt_data*)(puVar6 + 2);
						*puVar6 = (ulong)((pLayer->flags_0x0 & 1) == 0) | (ulong)((pLayer->flags_0x0 & 2) == 0) << 2;
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
	*(undefined8*)&pTVar1->pTextureChunk = 0x50;
	*(undefined8*)&pTVar1->pT2DA = 0x7ffe000000000;
	memset(&ged3DFXConfig, 0, sizeof(ed3DFXConfig) * 16);
	gScene3D = (ed_3D_Scene*)edMemAlloc(TO_HEAP(H_MAIN), ged3DConfig.sceneCount * sizeof(ed_3D_Scene) + 1);
	memset(gScene3D, 0, ged3DConfig.sceneCount * sizeof(ed_3D_Scene));
	gNodeCluster = edListCreateNodesTable(ged3DConfig.maxClusterCount + ged3DConfig.sceneCount * 3, TO_HEAP(H_MAIN));
	gCluster = (edCluster*)edMemAlloc(TO_HEAP(H_MAIN), ged3DConfig.maxClusterCount * sizeof(edCluster));
	memset(gCluster, 0, ged3DConfig.maxClusterCount * sizeof(edCluster));
	for (uVar2 = 0; uVar2 < ged3DConfig.sceneCount; uVar2 = uVar2 + 1) {
		edListLink(&gScene3D[uVar2].meshClusterList, gNodeCluster, 0, 0);
		edListLink(&gScene3D[uVar2].meshClusterShadowList, gNodeCluster, 0, 0);
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

#define HASH_CODE_ANMA 0x414d4e41
#define HASH_CODE_PALL 0x4c4c4150
#define HASH_CODE_T2DA 0x41443254

void ed3DPrepareG2DManageStruct(ed_g2d_manager* pManager, char* pFileBuffer, int fileLength)
{
	ed_Chunck* pChunck;
	ed_Chunck* pSubChunck;
	int chunkHeader;

	/* seek through the buffer */
	for (pChunck = edChunckGetFirst(pFileBuffer, pFileBuffer + fileLength); pChunck != (ed_Chunck*)0x0; pChunck = edChunckGetNext(pChunck, pFileBuffer + fileLength)) {
		ED3D_LOG(LogLevel::Info, "ed3DPrepareG2DManageStruct Chunk: {}", pChunck->GetHeaderString());

		chunkHeader = pChunck->hash;

		/* Check the first value in the buffer is *2D* */
		if (chunkHeader == HASH_CODE_2D) {
			/* Set the readValue to be the last section of the texture header */
			const int chunkSize = pChunck->size;

			ED3D_LOG(LogLevel::Info, "ed3DPrepareG2DManageStruct Found *2D* chunk - size: 0x{:x}", chunkSize);


			pManager->pTextureChunk = pChunck;

			for (pSubChunck = edChunckGetFirst(pChunck + 1, ((char*)pChunck + chunkSize)); pSubChunck != (ed_Chunck*)0x0;
				pSubChunck = edChunckGetNext(pSubChunck, ((char*)pChunck + chunkSize))) {
				ED3D_LOG(LogLevel::Info, "ed3DPrepareG2DManageStruct SubChunk: {}", pSubChunck->GetHeaderString());

				/* Check if the value in the buffer is 'MATA' */
				if (pSubChunck->hash == HASH_CODE_MATA) {
					ED3D_LOG(LogLevel::Info, "ed3DPrepareG2DManageStruct Found MATA chunk - size: 0x{:x}", pSubChunck->size);
					pManager->pMATA_HASH = (ed_Chunck*)(pSubChunck + 1);
				}
			}
		}
		else {
			/* Check if the value in the buffer is 'ANMA' */
			if (chunkHeader == HASH_CODE_ANMA) {
				ED3D_LOG(LogLevel::Info, "ed3DPrepareG2DManageStruct Found ANMA chunk - size: 0x{:x}", pChunck->size);
				pManager->pANMA = pChunck;
			}
			else {
				/* Check if the value in the buffer is 'PALL' */
				if (chunkHeader == HASH_CODE_PALL) {
					ED3D_LOG(LogLevel::Info, "ed3DPrepareG2DManageStruct Found PALL chunk - size: 0x{:x}", pChunck->size);
					pManager->pPALL = pChunck;
				}
				else {
					/* Check if the value in the buffer is 'T2DA' */
					if (chunkHeader == HASH_CODE_T2DA) {
						ED3D_LOG(LogLevel::Info, "ed3DPrepareG2DManageStruct Found T2DA chunk - size: 0x{:x}", pChunck->size);
						pManager->pT2DA = pChunck;
					}
				}
			}
		}
	}
	return;
}

#define HASH_CODE_REAA 0x41414552
#define HASH_CODE_REAL 0x4c414552
#define HASH_CODE_RSTR 0x52545352
#define HASH_CODE_ROBJ 0x4a424f52
#define HASH_CODE_RGEO 0x4f454752

int* ed3DPreparePointer(char* pFileBufferA, int lengthA, char* pFileBufferB, int lengthB)
{
	int reaaChunkSize;
	ed_Chunck* pChunk;
	ed_Chunck* pREAA;
	int* nextSubSection;
	uint uVar3;
	uint uVar4;
	char* actualFileStart;
	char* pcVar5;
	int chunkHeader;

	pcVar5 = pFileBufferA + (-lengthB - (ulong)pFileBufferB);
	actualFileStart = pFileBufferB + -0x10;

	if (pFileBufferA == pFileBufferB) {
		pcVar5 = (char*)0x0;
	}

	pREAA = edChunckGetFirst(pFileBufferA, pFileBufferA + lengthA);

	ED3D_LOG(LogLevel::Info, "ed3DPreparePointer First chunk: {}", pREAA->GetHeaderString());

	while (true) {
		if (pREAA == (ed_Chunck*)0x0) {
			return (int*)0x0;
		}

		// Check if the value in the buffer is 'REAA'
		if ((pREAA->hash == HASH_CODE_REAA) && (pREAA->field_0x4 != 0x7b6)) break;

		pREAA = edChunckGetNext(pREAA, pFileBufferA + lengthA);
		ED3D_LOG(LogLevel::Info, "ed3DPreparePointer Searching for REAA - current: {}", pREAA->GetHeaderString());
	}

	reaaChunkSize = pREAA->size;
	pREAA->field_0x4 = 0x7b6;

	ED3D_LOG(LogLevel::Info, "ed3DPreparePointer REAA chunk size: {}", reaaChunkSize);

	pChunk = edChunckGetFirst(pREAA + 1, ((char*)pREAA + reaaChunkSize));

	ED3D_LOG(LogLevel::Info, "ed3DPreparePointer Post REAA chunk: {}", pChunk->GetHeaderString());

	pcVar5 = actualFileStart + (int)pcVar5;

	for (; pChunk != (ed_Chunck*)0x0; pChunk = edChunckGetNext(pChunk, ((char*)pREAA + reaaChunkSize))) {
		ED3D_LOG(LogLevel::Info, "ed3DPreparePointer edChunckGetNext: {}", pChunk->GetHeaderString());

		chunkHeader = pChunk->hash;

		nextSubSection = (int*)(pChunk + 1);
		uVar3 = pChunk->size - 0x10U >> 2;
		/* Check if the value in the buffer is 'REAL' */
		if (chunkHeader == HASH_CODE_REAL) {
			ED3D_LOG(LogLevel::Info, "ed3DPrepareG2DManageStruct Found REAL chunk - size: 0x{:x}", pChunk->size);

			for (uVar4 = 0; uVar4 < (uVar3 & 0xffff); uVar4 = uVar4 + 1 & 0xffff) {
				/* Jump forward one section */
				if (*nextSubSection != 0) {
					/* Store a pointer to the next sub section */
					int offset = *(int*)(actualFileStart + *nextSubSection);
					*(int*)(actualFileStart + *nextSubSection) = STORE_SECTION(actualFileStart + offset);
				}

				nextSubSection = nextSubSection + 1;
			}
		}
		else {
			/* Is current value RSTR */
			if (chunkHeader == HASH_CODE_RSTR) {
				ED3D_LOG(LogLevel::Info, "ed3DPrepareG2DManageStruct Found RSTR chunk - size: 0x{:x}", pChunk->size);

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
				if (chunkHeader == HASH_CODE_ROBJ) {
					ED3D_LOG(LogLevel::Info, "ed3DPrepareG2DManageStruct Found ROBJ chunk - size: 0x{:x}", pChunk->size);

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
					if (chunkHeader == HASH_CODE_RGEO) {
						ED3D_LOG(LogLevel::Info, "ed3DPrepareG2DManageStruct Found RGEO chunk - size: 0x{:x}", pChunk->size);

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

	ED3D_LOG(LogLevel::Info, "ed3DPreparePointer Complete");

	return (int*)pREAA;
}

void ed3DPreparedAllMaterial(ed_g2d_manager* textureInfoObj, ulong mode)
{
	ed_Chunck* pcVar1;
	int iVar2;
	int iVar3;
	ed_Chunck* piVar4;
	ed_Chunck* piVar5;

	pcVar1 = textureInfoObj->pTextureChunk;
	iVar2 = pcVar1->size;
	for (piVar4 = edChunckGetFirst(pcVar1 + 1, (char*)pcVar1 + iVar2); piVar4 != (ed_Chunck*)0x0; piVar4 = edChunckGetNext(piVar4, (char*)pcVar1 + iVar2)) {
		if (piVar4->hash == 0x4154414d) {
			iVar3 = piVar4->size;
			for (piVar5 = edChunckGetFirst(piVar4 + 1, ((char*)piVar4 + iVar3)); piVar5 != (ed_Chunck*)0x0;
				piVar5 = edChunckGetNext(piVar5, ((char*)piVar4 + iVar3))) {
				/* .TAM */
				if (piVar5->hash == HASH_CODE_MATA) {
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

void ed3DPrepareG2D(ed_g2d_manager* pManager, ulong mode)
{
	int texHeaderSize;
	edpkt_data* puVar2;
	ed_Chunck* pBuffer;
	char* fileBufferStart;
	ed_Chunck* texHeaderStart;

	/* Can point at 004DA040 */
	puVar2 = g_pStrippBufLastPos;
	texHeaderStart = pManager->pTextureChunk;
	texHeaderSize = texHeaderStart->size;

	if ((pManager != (ed_g2d_manager*)0x0) && ((pManager->pFileBuffer->flags & 1) == 0)) {
		IMPLEMENTATION_GUARD();
		for (pBuffer = edChunckGetFirst(texHeaderStart + 1, (char*)texHeaderStart + texHeaderSize); pBuffer != (ed_Chunck*)0x0;
			pBuffer = edChunckGetNext(pBuffer, (char*)texHeaderStart + texHeaderSize)) {
			/* Check if the value in the buffer is 'LAYA' */
			if (pBuffer->hash == 0x4159414c) {
				//ed3DPrepareLayer(pBuffer, textureInfoObj);
			}
		}

		ed3DPreparedAllMaterial(pManager, mode);
		pManager->textureFileLengthB = (ulong)g_pStrippBufLastPos + (pManager->textureFileLengthB - (ulong)puVar2);
		pManager->pFileBuffer->flags = pManager->pFileBuffer->flags | 1;
	}

	return;
}

#ifdef PLATFORM_WIN
Multidelegate<ed_g2d_manager*, std::string> onTextureLoadedDelegate;
Multidelegate<ed_g3d_manager*, std::string> onMeshLoadedDelegate;

Multidelegate<ed_g2d_manager*, std::string>& ed3DGetTextureLoadedDelegate()
{
	return onTextureLoadedDelegate;
}

Multidelegate<ed_g3d_manager*, std::string>& ed3DGetMeshLoadedDelegate()
{
	return onMeshLoadedDelegate;
}
#endif

#define HASH_CODE_T2D 0x4432472e

ed_g2d_manager* ed3DInstallG2D(char* pFileBuffer, int fileLength, int* outInt, ed_g2d_manager* pManager, int param_5)
{
	bool bIsNewManager;
	ed_g2d_manager* pOutManager;

	char* pFileBody;
	ed_g2d_manager* pTexturePool;

	ED3D_LOG(LogLevel::Info, "ed3DInstallG2D");

	/* Find some free area in the buffer? */
	pOutManager = pManager;

	pTexturePool = gpG2D;
	if (pManager == (ed_g2d_manager*)0x0) {
		while (pOutManager = pTexturePool, pOutManager->pFileBuffer != (GXD_FileHeader*)0x0) {
			pTexturePool = pOutManager + 1;
		}
	}

	if ((pOutManager == (ed_g2d_manager*)0x0) || (memset(pOutManager, 0, sizeof(ed_g2d_manager)), pFileBuffer == (char*)0x0)) {
		bIsNewManager = pOutManager != pManager;
		pOutManager = (ed_g2d_manager*)0x0;

		if (bIsNewManager) {
			gNbG2D = gNbG2D + -1;
			pOutManager = (ed_g2d_manager*)0x0;
		}
	}
	else {
		GXD_FileHeader* const pFileHeader = reinterpret_cast<GXD_FileHeader*>(pFileBuffer);
		pFileBody = reinterpret_cast<char*>(pFileHeader + 1);

		assert(pFileHeader->hash == HASH_CODE_T2D);

		ed3DPrepareG2DManageStruct(pOutManager, pFileBody, fileLength + -0x10);
		ed3DPreparePointer(pFileBody, fileLength + -0x10, pFileBody, fileLength + -0x10);

		pOutManager->pFileBuffer = pFileHeader;
		pOutManager->textureFileLengthA = fileLength;
		pOutManager->textureFileLengthB = fileLength;
		*outInt = fileLength - pOutManager->textureFileLengthB;

		ed3DPrepareG2D(pOutManager, param_5);
	}

#ifdef PLATFORM_WIN
	onTextureLoadedDelegate(pOutManager, ObjectNaming::CopyObjectName());
#endif

	return pOutManager;
}

ed_hash_code* ed3DG2DGetMaterialFromIndex(ed_g2d_manager* pManager, int index)
{
	ed_Chunck* pMATA_HASH;
	ed_Chunck* pNextChunk;
	uint materialIndex;
	ed_hash_code* pHashCode;
	char* pChunkEnd;

	ED3D_LOG(LogLevel::Verbose, "ed3DG2DGetMaterialFromIndex Looking for index: {}", index);

	pMATA_HASH = pManager->pMATA_HASH;
	ed_Chunck* pMATA = pManager->pMATA_HASH - 1;

	/* Ensure we are trying to read something from the 'MATA' section */
	if (pMATA->hash == HASH_CODE_MATA) {
		/* Work out the section end address */
		pChunkEnd = (char*)pMATA + pMATA->size;
		for (pNextChunk = edChunckGetFirst(pMATA_HASH, pChunkEnd); pNextChunk != (ed_Chunck*)0x0; pNextChunk = edChunckGetNext(pNextChunk, pChunkEnd)) {
			ED3D_LOG(LogLevel::Verbose, "ed3DG2DGetMaterialFromIndex Chunk: {}", pNextChunk->GetHeaderString());

			/* Check the first value in the buffer is *MAT.* */
			if (pNextChunk->hash == HASH_CODE_MAT) {
				ED3D_LOG(LogLevel::Verbose, "ed3DG2DGetMaterialFromIndex Found MAT. chunk - size: 0x{:x}", pNextChunk->size);

				if ((index == 0) && (pMATA_HASH->hash == HASH_CODE_HASH)) {
					ED3D_LOG(LogLevel::Verbose, "ed3DG2DGetMaterialFromIndex Found HASH chunk - size: 0x{:x}", pNextChunk->size);

					/* Check the first value in the buffer is *HASH* */
					pHashCode = (ed_hash_code*)(pMATA_HASH + 1);

					const int materialCount = ed3DG2DGetG2DNbMaterials(pMATA_HASH);

					ED3D_LOG(LogLevel::Verbose, "ed3DG2DGetMaterialFromIndex Searching through {} materials for hash: {}", materialCount, pHashCode->hash.ToString());

					for (materialIndex = materialCount; materialIndex != 0; materialIndex = materialIndex - 1) {

						ed_Chunck* pMAT = LOAD_SECTION_CAST(ed_Chunck*, pHashCode->pData);

						assert(pMAT->hash == HASH_CODE_MAT);

						if (pMAT == pNextChunk) {
							ED3D_LOG(LogLevel::Verbose, "ed3DG2DGetMaterialFromIndex Found material at index {}", materialCount - materialIndex);
							return pHashCode;
						}

						pHashCode = pHashCode + 1;
					}
				}

				index = index + -1;
			}
			/* Jump to the next section */
		}
	}

	return (ed_hash_code*)0x0;
}

ed_g2d_material* ed3DG2DGetG2DMaterialFromIndex(ed_g2d_manager* pManager, int index)
{
	ed_hash_code* pMAT_HASH;
	ed_g2d_material* pMAT_Internal;

	ED3D_LOG(LogLevel::Verbose, "ed3DG2DGetG2DMaterialFromIndex Looking for index: {}", index);

	pMAT_HASH = ed3DG2DGetMaterialFromIndex(pManager, index);

	if (pMAT_HASH == (ed_hash_code*)0x0) {
		pMAT_Internal = (ed_g2d_material*)0x0;
	}
	else {
		ed_Chunck* pMAT = LOAD_SECTION_CAST(ed_Chunck*, pMAT_HASH->pData);

		assert(pMAT->hash == HASH_CODE_MAT);

		pMAT_Internal = reinterpret_cast<ed_g2d_material*>(pMAT + 1);
	}

	return pMAT_Internal;
}

ed_g2d_bitmap* ed3DG2DGetBitmapFromMaterial(ed_g2d_material* pMaterial, int index)
{
	ed_g2d_bitmap* pBitmap;
	ed_g2d_texture* pTexture;
	ed_Chunck* pLAY;

	pTexture = (ed_g2d_texture*)0x0;
	pLAY = LOAD_SECTION_CAST(ed_Chunck*, pMaterial->aLayers[index]);
	ed_g2d_layer* pLayer = reinterpret_cast<ed_g2d_layer*>(pLAY + 1);

	pBitmap = (ed_g2d_bitmap*)0x0;

	if ((pLAY != (ed_Chunck*)0xfffffff0) && (pLayer->bHasTexture != 0)) {
		ed_Chunck* pTEX = LOAD_SECTION_CAST(ed_Chunck*, pLayer->pTex);
		pTexture = reinterpret_cast<ed_g2d_texture*>(pTEX + 1);
	}

	if (pTexture == (ed_g2d_texture*)0x0) {
		pBitmap = (ed_g2d_bitmap*)0x0;
	}
	else {
		ed_hash_code* pBitmapHashCode = LOAD_SECTION_CAST(ed_hash_code*, pTexture->hashCode.pData);
		if (pBitmapHashCode != (ed_hash_code*)0x0) {
			ed_Chunck* pT2D = LOAD_SECTION_CAST(ed_Chunck*, pBitmapHashCode->pData);
			pBitmap = reinterpret_cast<ed_g2d_bitmap*>(pT2D + 1);
		}
	}

	return pBitmap;
}

#define HASH_CODE_CDZA 0x415a4443
#define HASH_CODE_INFA 0x41464e49
#define HASH_CODE_MBNA 0x414e424d
#define HASH_CODE_SPRA 0x41525053
#define HASH_CODE_CAMA 0x414d4143
#define HASH_CODE_LIA_ 0x2e41494c
#define HASH_CODE_CSTA 0x41545343
#define HASH_CODE_ANMA 0x414d4e41
#define HASH_CODE_HALL 0x4c4c4148
#define HASH_CODE_OBJA 0x414a424f
#define HASH_CODE_GEOM 0x4d4f4547

// Inlined function?
inline void ProcessG3DChunck(ed_g3d_manager* pManager, ed_Chunck* pChunck)
{
	int chunkHeader = pChunck->hash;

	/* Is current value CDZA */
	if (chunkHeader == HASH_CODE_CDZA) {
		ED3D_LOG(LogLevel::Info, "ed3DPrepareG3DManageStruct Found {} chunk - size: 0x{:x}", pChunck->GetHeaderString(), pChunck->size);
		pManager->CDZA = pChunck;
	}
	else {
		/* Is current value INFA */
		if (chunkHeader == HASH_CODE_INFA) {
			ED3D_LOG(LogLevel::Info, "ed3DPrepareG3DManageStruct Found {} chunk - size: 0x{:x}", pChunck->GetHeaderString(), pChunck->size);
			pManager->INFA = pChunck;
		}
		else {
			/* Is current value MBNA */
			if (chunkHeader == HASH_CODE_MBNA) {
				ED3D_LOG(LogLevel::Info, "ed3DPrepareG3DManageStruct Found {} chunk - size: 0x{:x}", pChunck->GetHeaderString(), pChunck->size);
				pManager->MBNA = pChunck;
			}
			else {
				/* Is current value SPRA */
				if (chunkHeader == HASH_CODE_SPRA) {
					ED3D_LOG(LogLevel::Info, "ed3DPrepareG3DManageStruct Found {} chunk - size: 0x{:x}", pChunck->GetHeaderString(), pChunck->size);
					pManager->SPRA = pChunck;
				}
				else {
					/* Is current value CAMA */
					if (chunkHeader == HASH_CODE_CAMA) {
						ED3D_LOG(LogLevel::Info, "ed3DPrepareG3DManageStruct Found {} chunk - size: 0x{:x}", pChunck->GetHeaderString(), pChunck->size);
						pManager->CAMA = pChunck;
					}
					else {
						/* Is current value LIA. */
						if (chunkHeader == HASH_CODE_LIA_) {
							ED3D_LOG(LogLevel::Info, "ed3DPrepareG3DManageStruct Found {} chunk - size: 0x{:x}", pChunck->GetHeaderString(), pChunck->size);
							pManager->LIA = pChunck;
						}
						else {
							/* Is current value CSTA */
							if (chunkHeader == HASH_CODE_CSTA) {
								ED3D_LOG(LogLevel::Info, "ed3DPrepareG3DManageStruct Found {} chunk - size: 0x{:x}", pChunck->GetHeaderString(), pChunck->size);
								pManager->CSTA = pChunck;
							}
							else {
								/* Is current value ANMA */
								if (chunkHeader == HASH_CODE_ANMA) {
									ED3D_LOG(LogLevel::Info, "ed3DPrepareG3DManageStruct Found {} chunk - size: 0x{:x}", pChunck->GetHeaderString(), pChunck->size);
									pManager->ANMA = pChunck;
								}
								else {
									/* Is current value HALL */
									if (chunkHeader == HASH_CODE_HALL) {
										ED3D_LOG(LogLevel::Info, "ed3DPrepareG3DManageStruct Found {} chunk - size: 0x{:x}", pChunck->GetHeaderString(), pChunck->size);
										pManager->HALL = pChunck;
									}
									else {
										/* Is current value OBJA */
										if (chunkHeader == HASH_CODE_OBJA) {
											ED3D_LOG(LogLevel::Info, "ed3DPrepareG3DManageStruct Found {} chunk - size: 0x{:x}", pChunck->GetHeaderString(), pChunck->size);
											pManager->OBJA = pChunck;
										}
										else {
											/* Is current value GEOM */
											if (chunkHeader == HASH_CODE_GEOM) {
												ED3D_LOG(LogLevel::Info, "ed3DPrepareG3DManageStruct Found {} chunk - size: 0x{:x}", pChunck->GetHeaderString(), pChunck->size);
												pManager->GEOM = pChunck;
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

void ed3DPrepareG3DManageStruct(ed_g3d_manager* pManager, char* pFileBufferA, int fileLengthA, char* pFileBufferB, int fileLengthB)
{
	ed_Chunck* pChunck;

	for (pChunck = edChunckGetFirst(pFileBufferB, pFileBufferB + fileLengthB); pChunck != (ed_Chunck*)0x0; pChunck = edChunckGetNext(pChunck, pFileBufferB + fileLengthB)) {
		ED3D_LOG(LogLevel::Info, "ed3DPrepareG3DManageStruct B Chunk: {}", pChunck->GetHeaderString());
		ProcessG3DChunck(pManager, pChunck);
	}

	if (pFileBufferA != pFileBufferB) {
		for (pChunck = edChunckGetFirst(pFileBufferA, pFileBufferA + fileLengthA); pChunck != (ed_Chunck*)0x0; pChunck = edChunckGetNext(pChunck, pFileBufferA + fileLengthA)) {
			ED3D_LOG(LogLevel::Info, "ed3DPrepareG3DManageStruct A Chunk: {}", pChunck->GetHeaderString());
			ProcessG3DChunck(pManager, pChunck);
		}
	}

	return;
}

char* edHashCodeGet(Hash_8 hash, int* texHashPtr, int hashCount, uint triedHashes, uint totalHashes)
{
	uint uVar1;
	ulong* puVar2;

	puVar2 = (ulong*)(texHashPtr + hashCount * 4);
	if (*puVar2 < hash.number) {
		uVar1 = (uint)(totalHashes - hashCount) >> 1;
		if (uVar1 == 0) {
			uVar1 = 1;
		}
		if (hashCount == totalHashes) {
			puVar2 = (ulong*)0x0;
		}
		else {
			puVar2 = (ulong*)edHashCodeGet(hash, texHashPtr, hashCount + uVar1, hashCount + 1, totalHashes);
		}
	}
	else {
		if (hash.number < *puVar2) {
			uVar1 = (uint)(hashCount - triedHashes) >> 1;
			if (uVar1 == 0) {
				uVar1 = 1;
			}
			if (hashCount == triedHashes) {
				puVar2 = (ulong*)0x0;
			}
			else {
				puVar2 = (ulong*)edHashCodeGet(hash, texHashPtr, hashCount - uVar1, triedHashes, hashCount + -1);
			}
		}
	}
	return (char*)puVar2;
}

ed_hash_code* edHashcodeGet(Hash_8 hash, ed_Chunck* pChunck)
{
	ed_hash_code* pcVar1;

	/* If the first value in the texture obj buffer is HASH or MBNK */
	if ((pChunck->hash == HASH_CODE_HASH) || (pChunck->hash == HASH_CODE_MBNK)) {
		pcVar1 = (ed_hash_code*)edHashCodeGet(hash, (int*)(pChunck + 1), pChunck->nextChunckOffset - 0x10U >> 5, 0,
			pChunck->nextChunckOffset - 0x10U >> 4);
	}
	else {
		pcVar1 = (ed_hash_code*)0x0;
	}
	return pcVar1;
}

uint edChunckGetNb(void* pStart, char* pEnd)
{
	uint uVar1;

	char* pChunck = reinterpret_cast<char*>(pStart);

	uVar1 = 0;

	if ((pEnd != (char*)0x0) && (pEnd <= pChunck)) {
		pChunck = (char*)0x0;
	}

	while (pChunck != (char*)0x0) {
		uVar1 = uVar1 + 1;
		if ((pEnd == (char*)0x0) || (pChunck + *(int*)(pChunck + 8) < pEnd)) {
			pChunck = pChunck + *(int*)(pChunck + 8);
		}
		else {
			pChunck = (char*)0x0;
		}
	}

	return uVar1 & 0xffff;
}

void ed3DPrepareMaterialBank(ed_Chunck* pMBNA, ed_g2d_manager* pTextureInfo)
{
	int mbnaSize;
	ed_Chunck* pChunk;
	ushort materialCount;
	ed_hash_code* pcVar4;
	ed_Chunck* pMBNK;

	mbnaSize = pMBNA->size;
	/* Checks all the hashes in the mesh to make sure they match what is in the texture Get the end of the current section */
	for (pChunk = edChunckGetFirst(pMBNA + 1, (char*)pMBNA + mbnaSize); pChunk != (ed_Chunck*)0x0; pChunk = edChunckGetNext(pChunk, (char*)pMBNA + mbnaSize)) {
		ED3D_LOG(LogLevel::Info, "ed3DPrepareMaterialBank Chunk: {}", pChunk->GetHeaderString());

		/* Check if read value is MBNK */
		if (pChunk->hash == HASH_CODE_MBNK) {
			ED3D_LOG(LogLevel::Info, "ed3DPrepareMaterialBank Found MBNK chunk - size: 0x{:x}", pChunk->size);

			materialCount = (pChunk->size - 0x10U) >> 4;
			ED3D_LOG(LogLevel::Info, "ed3DPrepareMaterialBank Material count: {}", materialCount);

			pMBNK = pChunk;
			while (true) {
				if (materialCount == 0) break;

				ed_hash_code* pMaterialHash = (ed_hash_code*)(pMBNK + 1);

				ED3D_LOG(LogLevel::Info, "ed3DPrepareMaterialBank Trying to find texture {}: {}", ((pChunk->size - 0x10U) >> 4) - materialCount, pMaterialHash->hash.ToString());

				pcVar4 = edHashcodeGet(pMaterialHash->hash, pTextureInfo->pMATA_HASH);
				if (pcVar4 != (ed_hash_code*)0x0) {
					ED3D_LOG(LogLevel::Info, "ed3DPrepareMaterialBank Found texture: {}", pcVar4->hash.ToString());
					pMaterialHash->pData = STORE_SECTION(pcVar4);
				}

				materialCount = materialCount - 1;
				pMBNK = pMBNK + 1;
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
	ClusterDetails clusterDetails;
	short field_0x24;
};

int INT_0044935c = 0;
bool BOOL_00449370 = false;

void ed3DPrepareCluster(ed_g3d_cluster* pCluster, bool param_2, ed_g3d_manager* pMeshInfo, ed_g2d_manager* pTextureInfo, int param_5, bool bHasFlag)
{
	ushort uVar1;
	ed_Chunck* piVar2;
	int chunkSize;
	int offset;
	ed_Chunck* piVar4;
	uint* puVar5;
	uint uVar6;
	uint uVar7;
	uint uVar8;
	astruct_14 aStack96;
	astruct_14 local_30;
	bool bHasInternalFlag;

	ED3D_LOG(LogLevel::Info, "ed3DPrepareClusterTree");

	bHasInternalFlag = (pCluster->flags_0x1c & 1) == 0;

	ED3D_LOG(LogLevel::Info, "ed3DPrepareClusterTree Internal flag: {}", bHasInternalFlag);

	if (bHasFlag == false) {
		if (bHasInternalFlag) {
			uVar8 = 0;
			pCluster->flags_0x1c = pCluster->flags_0x1c | 1;

			uVar7 = 0;
			do {
				uVar6 = uVar7 + 1 & 0xffff;
				uVar8 = uVar8 + pCluster->aClusterStripCounts[uVar7 - 8];
				uVar7 = uVar6;
			} while (uVar6 < 0xd);

			INT_0044935c = 0x60;
			local_30.field_0xc = '\0';
			local_30.clusterDetails = pCluster->clusterDetails;
			piVar4 = (ed_Chunck*)pCluster->field_0x30;
			piVar2 = (ed_Chunck*)pCluster->field_0x34;
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
					pCluster->field_0x38 = puVar5;
				}
				piVar2 = edChunckGetNext(piVar2, (int*)0x0);)
			}
		}
	}
	else {
		if (BOOL_00449370 == true) {
			IMPLEMENTATION_GUARD(
				piVar4 = (int*)pCluster->field_0x30;
			puVar5 = (uint*)pCluster->field_0x38;
			uVar7 = 0;
			if ((piVar4 != (int*)0x0) && (puVar5 != (uint*)0x0)) {
				uVar8 = 0;
				do {
					uVar6 = uVar8 + 1 & 0xffff;
					uVar7 = uVar7 + pCluster->field_0x10[uVar8 - 8];
					uVar8 = uVar6;
				} while (uVar6 < 0xd);
				INT_0044935c = 0x60;
				for (uVar8 = 0; uVar8 < uVar7; uVar8 = uVar8 + 1 & 0xffff) {
					if ((long)(int)puVar5[0xf] != 0) {
						ed3DStripPrepareSpherePacket((int)puVar5, (long)(int)puVar5[0xf], (int)(piVar4 + 4));
					}
					puVar5 = (uint*)puVar5[3];
				}
				chunkSize = *(int*)&pCluster[-1].field_0x44;
				goto LAB_002a40c4;
			})
		}
	}

	ed_Chunck* pChunk = reinterpret_cast<ed_Chunck*>(reinterpret_cast<char*>(pCluster) - sizeof(ed_Chunck));

	chunkSize = pChunk->size;
LAB_002a40c4:
	uVar1 = pCluster->field_0x1a;
	offset = (uint)uVar1 << 3;

	if ((uVar1 & 1) != 0) {
		offset = (uVar1 + 1) * 8;
	}

	ED3D_LOG(LogLevel::Info, "ed3DPrepareClusterTree Header chunk: {} size: 0x{:x} flags: 0x{:x} offset: 0x{:x}", pChunk->GetHeaderString(), chunkSize, uVar1, offset);

	ed_Chunck* pPostClusterChunk;
	for (pPostClusterChunk = edChunckGetFirst(reinterpret_cast<char*>(pCluster) + offset + sizeof(ed_g3d_cluster), reinterpret_cast<char*>(pCluster) + chunkSize + -sizeof(ed_Chunck));
		pPostClusterChunk != (ed_Chunck*)0x0; pPostClusterChunk = edChunckGetNext(pPostClusterChunk, reinterpret_cast<char*>(pCluster) + chunkSize + -sizeof(ed_Chunck))) {
		if (pPostClusterChunk->hash == HASH_CODE_SPRA) {
			IMPLEMENTATION_GUARD_LOG(
			if (bHasInternalFlag) {
				aStack96.field_0xc = '\0';
				aStack96.clusterDetails = pCluster->clusterDetails;
				aStack96.field_0x4 = (int*)pTextureInfo;
				aStack96.field_0x8 = (int*)param_5;
				IMPLEMENTATION_GUARD(puVar5 = ed3DPrepareAllSprite((int)pPostClusterChunk, &aStack96, (int)(pCluster->field_0x30 + 4), 4));
				pCluster->field_0x3c = (int)STORE_SECTION(puVar5);
				pCluster->field_0x1e = aStack96.field_0x24;
			})
		}
		else {
			if ((pPostClusterChunk->hash == HASH_CODE_CDQU) || (pPostClusterChunk->hash == HASH_CODE_CDOC)) {
				ed3DPrepareCluster((ed_g3d_cluster*)(pPostClusterChunk + 4), param_2, pMeshInfo, pTextureInfo, param_5, bHasFlag);
			}
		}
	}

	ED3D_LOG(LogLevel::Info, "ed3DPrepareClusterTree Complete");

	return;
}

void ed3DPrepareClusterALL(bool bUnused, ed_g3d_manager* meshInfoObj, ed_g2d_manager* textureInfo, int param_4)
{
	int cstaSize;
	int chunkSize;
	bool bHasFlag;
	ed_Chunck* pCSTA;
	edpkt_data* pCommandBuffer;

	ED3D_LOG(LogLevel::Info, "ed3DPrepareClusterALL");

	pCommandBuffer = g_pStrippBufLastPos;

	pCSTA = meshInfoObj->CSTA;
	cstaSize = pCSTA->size;

	ED3D_LOG(LogLevel::Info, "ed3DPrepareClusterALL Loaded CSTA chunck hash: {} size: 0x{:x}", pCSTA->GetHeaderString(), pCSTA->size);

	bHasFlag = (meshInfoObj->fileBufferStart->flags & 1) != 0;
	ED3D_LOG(LogLevel::Info, "ed3DPrepareClusterALL Mesh has flag: {}", bHasFlag);

	ed_Chunck* pNextChunk = pCSTA + 1;
	ED3D_LOG(LogLevel::Info, "ed3DPrepareClusterALL Loaded next chunck hash: {} size: 0x{:x}", pNextChunk->GetHeaderString(), pNextChunk->size);

	char* const pCSTA_End = reinterpret_cast<char*>(pCSTA) + cstaSize;

	for (ed_Chunck* pChunk = edChunckGetFirst(pNextChunk, pCSTA_End); pChunk != (ed_Chunck*)0x0; pChunk = edChunckGetNext(pChunk, pCSTA_End)) {
		ED3D_LOG(LogLevel::Info, "ed3DPrepareClusterALL Processing chunck hash: {} size: 0x{:x}", pChunk->GetHeaderString(), pChunk->size);

		/* If read value is CDQA -> Quad Tree */
		if (pChunk->hash == HASH_CODE_CDQA) {
			ED3D_LOG(LogLevel::Info, "ed3DPrepareClusterALL Found CDQA chunck hash: {} size: 0x{:x}", pChunk->GetHeaderString(), pChunk->size);

			/* Search for CDQU */
			chunkSize = pChunk->size;
			char* const pChunkEnd = reinterpret_cast<char*>(pChunk) + chunkSize;

			for (ed_Chunck* pSubChunk = edChunckGetFirst(pChunk + 3, pChunkEnd); pSubChunk != (ed_Chunck*)0x0; pSubChunk = edChunckGetNext(pSubChunk, pChunkEnd))
			{
				ED3D_LOG(LogLevel::Info, "ed3DPrepareClusterALL Processing chunck hash: {} size: 0x{:x}", pSubChunk->GetHeaderString(), pSubChunk->size);

				/* If read value is CDQU */
				if (pSubChunk->hash == HASH_CODE_CDQU) {
					ED3D_LOG(LogLevel::Info, "ed3DPrepareClusterALL Found CDQU chunck hash: {} size: 0x{:x}", pSubChunk->GetHeaderString(), pSubChunk->size);
					ed3DPrepareCluster((ed_g3d_cluster*)(pSubChunk + 1), true, meshInfoObj, textureInfo, param_4, bHasFlag);
				}
			}
		}
		else {
			/* If read value is CDOA -> Octree */
			if (pChunk->hash == HASH_CODE_CDOA) {
				ED3D_LOG(LogLevel::Info, "ed3DPrepareClusterALL Found CDOA chunck hash: {} size: 0x{:x}", pChunk->GetHeaderString(), pChunk->size);

				/* Search for CDOC */
				chunkSize = pChunk->size;
				char* const pChunkEnd = reinterpret_cast<char*>(pChunk) + chunkSize;

				for (ed_Chunck* pSubChunk = edChunckGetFirst(pChunk + 3, pChunkEnd); pSubChunk != (ed_Chunck*)0x0; pSubChunk = edChunckGetNext(pSubChunk, pChunkEnd)) {
					ED3D_LOG(LogLevel::Info, "ed3DPrepareClusterALL Processing chunck hash: {} size: 0x{:x}", pSubChunk->GetHeaderString(), pSubChunk->size);

					/* If read value is CDOC */
					if (pSubChunk->hash == HASH_CODE_CDOC) {
						ED3D_LOG(LogLevel::Info, "ed3DPrepareClusterALL Found CDOC chunck hash: {} size: 0x{:x}", pSubChunk->GetHeaderString(), pSubChunk->size);
						ed3DPrepareCluster((ed_g3d_cluster*)(pSubChunk + 1), true, meshInfoObj, textureInfo, param_4, bHasFlag);
					}
				}
			}
		}
	}

	meshInfoObj->fileLengthB = meshInfoObj->fileLengthB + (int)((ulong)g_pStrippBufLastPos - (ulong)pCommandBuffer);
	return;
}

void ed3DLinkG2DToG3D(ed_g3d_manager* pMeshManager, ed_g2d_manager* pTextureManager)
{
	ed3DPrepareMaterialBank(pMeshManager->MBNA, pTextureManager);

	if (pMeshManager->CSTA != (ed_Chunck*)0x0) {
		ed3DPrepareClusterALL(true, pMeshManager, pTextureManager, 0xc);
	}
	return;
}

int ed3DPrepareHierarchy(ed_g3d_hierarchy* pHierarchy, ed_g2d_manager* pTextureInfo)
{
	short sVar1;
	ushort uVar2;
	ed_Chunck* piVar3;
	int iVar4;
	char* pBuffStart;
	ed_g3d_hierarchy* pTVar5;
	ed_Chunck* pHIER;

	ED3D_LOG(LogLevel::Info, "ed3DPrepareHierarchy");

	if ((pHierarchy->flags_0x9e & 4) == 0) {
		pTVar5 = pHierarchy + 1;

		pHIER = reinterpret_cast<ed_Chunck*>(reinterpret_cast<char*>(pHierarchy) - sizeof(ed_Chunck));

		if (pHIER->nextChunckOffset != pHIER->size) {
			// Tidy
			IMPLEMENTATION_GUARD();
			pBuffStart = ((char*)pHierarchy) + (pHIER->size - 0x10);
			iVar4 = *(int*)(pBuffStart + 8);
			for (piVar3 = edChunckGetFirst(pBuffStart, pBuffStart + iVar4); piVar3 != (ed_Chunck*)0x0;
				piVar3 = edChunckGetNext(piVar3, pBuffStart + iVar4)) {
				if (piVar3->hash == 0x494e4f42) {
					IMPLEMENTATION_GUARD(pHierarchy->field_0xa8 = piVar3 + 4);
				}
			}
		}

		pHierarchy->flags_0x9e = pHierarchy->flags_0x9e | 4;
		IMPLEMENTATION_GUARD(
			for (uVar2 = 0; uVar2 < pHierarchy->field_0x9c; uVar2 = uVar2 + 1) {
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
		if (pHierarchy->field_0x90 != 0) {
			//ed3DPrepareHierarchy((ed_g3d_hierarchy*)(pHIER->field_0x90 + 0x10), pTextureInfo);
		}
		iVar4 = 1;)
	}
	else {
		ED3D_LOG(LogLevel::Info, "ed3DPrepareHierarchy No additional data");
		iVar4 = 0;
	}

	return iVar4;
}

void ed3DPrepareHierarchyALL(ed_g3d_manager* pMeshInfo, ed_g2d_manager* pTextureInfo)
{
	ed_Chunck* pHALL;
	uint chunkNb;
	ed_hash_code* pHashCode;
	uint curIndex;

	ED3D_LOG(LogLevel::Info, "ed3DPrepareHierarchyALL");

	pHALL = pMeshInfo->HALL;

	ED3D_LOG(LogLevel::Info, "ed3DPrepareHierarchyALL Loaded HALL chunck hash: {} size: 0x{:x}", pHALL->GetHeaderString(), pHALL->size);

	ed_Chunck* pHASH = pHALL + 1;
	assert(pHASH->hash == HASH_CODE_HASH);
	ED3D_LOG(LogLevel::Info, "ed3DPrepareHierarchyALL Loaded HASH chunck hash: {} size: 0x{:x}", pHASH->GetHeaderString(), pHASH->size);

	pHashCode = reinterpret_cast<ed_hash_code*>(pHASH + 1);
	chunkNb = edChunckGetNb(pHASH, reinterpret_cast<char*>(pHALL) + pHALL->size);
	ED3D_LOG(LogLevel::Info, "ed3DPrepareHierarchyALL Hash count: {}", chunkNb - 1);

	for (curIndex = 0; curIndex < chunkNb - 1; curIndex = curIndex + 1) {
		ED3D_LOG(LogLevel::Info, "ed3DPrepareHierarchyALL Processing hash {}/{}: {}", curIndex, chunkNb - 1, pHashCode->hash.ToString());

		ed_Chunck* pHIER = LOAD_SECTION_CAST(ed_Chunck*, pHashCode->pData);
		assert(pHIER->hash == HASH_CODE_HIER);
		ED3D_LOG(LogLevel::Info, "ed3DPrepareHierarchyALL Loaded HIER chunck hash: {} size: 0x{:x}", pHIER->GetHeaderString(), pHIER->size);

		ed3DPrepareHierarchy(reinterpret_cast<ed_g3d_hierarchy*>(pHIER + 1), pTextureInfo);
		pHashCode = pHashCode + 1;
	}
	return;
}

void ed3DPrepareG3D(bool bUnused, ed_g3d_manager* pMeshManager, ed_g2d_manager* pTextureManager, int unknown)
{
	char* pAlloc;

	ED3D_LOG(LogLevel::Info, "ed3DPrepareG3D");

	if (pTextureManager != (ed_g2d_manager*)0x0) {
		ed3DPrepareMaterialBank(pMeshManager->MBNA, pTextureManager);
	}

	if (pMeshManager->CSTA == (ed_Chunck*)0x0) {
		ed3DPrepareHierarchyALL(pMeshManager, pTextureManager);
		pAlloc = pMeshManager->field_0x4;
	}
	else {
		ed3DPrepareClusterALL(bUnused, pMeshManager, pTextureManager, unknown);
		pAlloc = pMeshManager->field_0x4;
	}

	if (pAlloc != (char*)0x0) {
		edMemFree(pAlloc);
		pMeshManager->fileLengthB = pMeshManager->fileLengthB + -pMeshManager->field_0xc;
		pMeshManager->field_0xc = 0;
		pMeshManager->field_0x4 = (char*)0x0;
		pMeshManager->GEOM = (ed_Chunck*)0x0;
	}

	return;
}

#define HASH_CODE_G3D 0x4433472e

ed_g3d_manager* ed3DInstallG3D(char* pFileBuffer, int fileLength, ulong flags, int* outInt, ed_g2d_manager* pTextureManager, int unknown, ed_g3d_manager* pManager)
{
	bool bIsNewManager;
	int size;
	ed_g3d_manager* pMeshPool;
	int* piVar3;
	char* pFileBodyB;
	ed_g3d_manager* pOutManager;
	char* pFileBodyA;
	int size_00;

	ED3D_LOG(LogLevel::Info, "ed3DInstallG3D");

	pOutManager = pManager;

	pMeshPool = gpG3D;
	if (pManager == (ed_g3d_manager*)0x0) {
		while (pOutManager = pMeshPool, pOutManager->fileBufferStart != (GXD_FileHeader*)0x0) {
			pMeshPool = pOutManager + 1;
		}
	}

	GXD_FileHeader* const pFileHeader = reinterpret_cast<GXD_FileHeader*>(pFileBuffer);

	/* Check if the offset value from the buffer is '.G3D' */
	if (((pOutManager == (ed_g3d_manager*)0x0) || (memset(pOutManager, 0, sizeof(ed_g3d_manager)), pFileBuffer == (char*)0x0)) || (pFileHeader->hash != HASH_CODE_G3D)) {
		bIsNewManager = pOutManager != pManager;
		pOutManager = (ed_g3d_manager*)0x0;

		if (bIsNewManager) {
			gNbG3D = gNbG3D + -1;
			pOutManager = (ed_g3d_manager*)0x0;
		}
	}
	else {
		pOutManager->fileBufferStart = reinterpret_cast<GXD_FileHeader*>(pFileBuffer);
		pOutManager->fileLengthA = fileLength;
		pOutManager->field_0x4 = (char*)0x0;

		if ((pFileHeader->flags & 1) == 0) {
			pFileBodyA = reinterpret_cast<char*>(pFileHeader + 1);

			if ((flags & 2) == 0) {
				ed3DPrepareG3DManageStruct(pOutManager, pFileBodyA, fileLength + -0x10, pFileBodyA, fileLength + -0x10);
				IMPLEMENTATION_GUARD(); // This needs checking.
				edMemSetFlags(TO_HEAP(H_MAIN), 0x100);
				size = *(int*)(pOutManager->GEOM + 8);
				size_00 = size + *(int*)(pOutManager->GEOM + size + 8);
				pFileBodyB = (char*)edMemAlloc(TO_HEAP(H_MAIN), size_00);
				memcpy(pFileBodyB, (void*)pOutManager->GEOM, size_00);
				edMemClearFlags(TO_HEAP(H_MAIN), 0x100);
				*outInt = (fileLength - size_00) - fileLength;
				piVar3 = ed3DPreparePointer(pFileBodyB, size_00, pFileBodyA, (fileLength - size_00) + -0x10);
				if (piVar3 != (int*)0x0) {
					edMemShrink(pFileBodyB, size);
					*(int*)(pFileBuffer + 8) = *(int*)(pFileBuffer + 8) - size_00;
				}
				pOutManager->GEOM = (ed_Chunck*)pFileBodyB;
				pOutManager->field_0x4 = pFileBodyB;
				pOutManager->field_0xc = size_00;
				pOutManager->fileLengthB = pOutManager->fileLengthA;
				pOutManager->fileLengthA = pOutManager->fileLengthA - size_00;
			}
			else {
				pFileBodyA = pFileBuffer + 0x10;
				ed3DPrepareG3DManageStruct(pOutManager, pFileBodyA, fileLength + -0x10, pFileBodyA, fileLength + -0x10);
				piVar3 = ed3DPreparePointer(pFileBodyA, fileLength + -0x10, pFileBodyA, fileLength + -0x10);
				if (piVar3 != (int*)0x0) {
					IMPLEMENTATION_GUARD(); // This needs checking.
					*outInt = -piVar3[2];
					*(int*)(pFileBuffer + 8) = *(int*)(pFileBuffer + 8) - piVar3[2];
				}
				pOutManager->fileLengthB = fileLength;
			}
			ed3DPrepareG3D(true, pOutManager, pTextureManager, (long)unknown);
		}
		else {
			pFileBodyA = reinterpret_cast<char*>(pFileHeader + 1);

			ed3DPrepareG3DManageStruct(pOutManager, pFileBodyA, fileLength + -0x10, pFileBodyA, fileLength + -0x10);
			ed3DPreparePointer(pFileBodyA, fileLength + -0x10, pFileBodyA, fileLength + -0x10);

			pOutManager->GEOM = (ed_Chunck*)0x0;
			pOutManager->field_0x4 = (char*)0x0;
			pOutManager->field_0xc = 0;
			pOutManager->fileLengthB = pOutManager->fileLengthA;

			ed3DPrepareG3D(true, pOutManager, pTextureManager, unknown);
		}
	}

#ifdef PLATFORM_WIN
	onMeshLoadedDelegate(pOutManager, ObjectNaming::CopyObjectName());
#endif

	return pOutManager;
}

int gLoadedClusterCount = 0;


void ed3DHierarchyCopyHashCode(ed_g3d_manager* pMeshInfo)
{
	uint uVar1;
	uint uVar2;
	ulong* pMVar3;
	ed_Chunck* pcVar1;
	pcVar1 = pMeshInfo->HALL;
	pMVar3 = (ulong*)(pcVar1 + 2);
	uVar1 = edChunckGetNb(pcVar1 + 1, (char*)pcVar1 + pcVar1->size);
	for (uVar2 = 0; uVar2 < (uVar1 & 0xffff) - 1; uVar2 = uVar2 + 1) {
		ed_Chunck* p = (ed_Chunck*)LOAD_SECTION(*(pMVar3 + 1));
		ed_g3d_hierarchy* pHier = (ed_g3d_hierarchy*)(p + 1);
		pHier->hash.number = *pMVar3;
		pMVar3 = pMVar3 + 2;
	}
	return;
}

int g_LoadedStaticMeshCount_004493a4 = 0;

edNODE* ed3DHierarchyAddNode(edLIST* pList, ed_3d_hierarchy_node* pHierNode, edNODE* pNode, ed_g3d_hierarchy* pSourceHierarchy, ed_3d_hierarchy* p3DB)
{
	edNODE* pNextNode;
	undefined8 uVar2;
	ed_3d_hierarchy_node* pNewHierarchyNode;
	float fVar3;
	float fVar4;
	edF32MATRIX4* pMVar6;
	ed3DLod* pSourceLod;
	ed_3d_hierarchy_node* pMVar8;
	ed3DLod* pDestLod;
	edF32MATRIX4* pMVar10;
	edF32MATRIX4* pMVar11;
	edF32MATRIX4* pTVar12;
	uint newNodeIndex;
	edNODE* pNewNode;
	edNODE_MANAGER* pCameraPanHeader;

	ED3D_LOG(LogLevel::Info, "ed3DHierarchyAddNode {}", pSourceHierarchy->hash.ToString());

	pCameraPanHeader = (edNODE_MANAGER*)pList->pData;
	pNextNode = (pList)->pNext;
	pCameraPanHeader->linkCount = pCameraPanHeader->linkCount + 1;
	pList->nodeCount = pList->nodeCount + 1;
	pNewNode = pCameraPanHeader->pNodeHead;
	newNodeIndex = (uint)((ulong)pNewNode - (ulong)pNode) >> 4;
	pCameraPanHeader->pNodeHead = pNewNode->pPrev;
	pNewHierarchyNode = pHierNode + newNodeIndex;
	pNewNode->header.typeField.type = 0;
	pNewNode->pNext = pNextNode;
	pNewNode->pPrev = pNextNode->pPrev;
	pNextNode->pPrev->pNext = pNewNode;
	pNextNode->pPrev = pNewNode;

	g_LoadedStaticMeshCount_004493a4 = g_LoadedStaticMeshCount_004493a4 + 1;

	pHierNode[newNodeIndex].base.transformA = pSourceHierarchy->transformA;
	pHierNode[newNodeIndex].base.transformB = pSourceHierarchy->transformB;

	// Probably a copy.
	pNewHierarchyNode = pHierNode + newNodeIndex;
	(pNewHierarchyNode->base).hash = pSourceHierarchy->hash;
	(pNewHierarchyNode->base).bSceneRender = pSourceHierarchy->field_0x88;
	(pNewHierarchyNode->base).field_0x89 = pSourceHierarchy->field_0x89;
	(pNewHierarchyNode->base).bRenderShadow = pSourceHierarchy->bRenderShadow;
	IMPLEMENTATION_GUARD_LOG(
	(pNewHierarchyNode->base).pShadowAnimMatrix = (edF32MATRIX4*)LOAD_SECTION(pSourceHierarchy->pShadowAnimMatrix);)
	(pNewHierarchyNode->base).pLinkTransformData = (ed_3d_hierarchy*)LOAD_SECTION(pSourceHierarchy->pLinkTransformData);
	(pNewHierarchyNode->base).field_0x94 = (undefined*)LOAD_SECTION(pSourceHierarchy->field_0x94);
	(pNewHierarchyNode->base).pTextureInfo = (char*)LOAD_SECTION(pSourceHierarchy->pTextureInfo);

	ED3D_LOG(LogLevel::Info, "ed3DHierarchyAddNode Resolved texture hash code {:x} -> {:x}", (uintptr_t)pNewHierarchyNode, (uintptr_t)(pNewHierarchyNode->base).pTextureInfo);

	(pNewHierarchyNode->base).lodCount = pSourceHierarchy->lodCount;
	(pNewHierarchyNode->base).flags_0x9e = pSourceHierarchy->flags_0x9e;
	(pNewHierarchyNode->base).pHierarchySetup = (ed_3d_hierarchy_setup*)LOAD_SECTION(pSourceHierarchy->pHierarchySetup);
	(pNewHierarchyNode->base).pMatrixPkt = (edpkt_data*)LOAD_SECTION(pSourceHierarchy->pMatrixPkt);
	(pNewHierarchyNode->base).pAnimMatrix = (edF32MATRIX4*)LOAD_SECTION(pSourceHierarchy->pAnimMatrix);
	(pNewHierarchyNode->base).linkedHierCount = pSourceHierarchy->subMeshParentCount_0xac;
	(pNewHierarchyNode->base).desiredLod = pSourceHierarchy->desiredLod;
	(pNewHierarchyNode->base).GlobalAlhaON = pSourceHierarchy->GlobalAlhaON;

	pHierNode[newNodeIndex].base.transformB = gF32Matrix4Unit;

	int lodIndex = 0;
	do {
		ED3D_LOG(LogLevel::Info, "ed3DHierarchyAddNode Processing LOD {}", lodIndex);

		if (lodIndex < pSourceHierarchy->lodCount) {
			pSourceLod = pSourceHierarchy->aLods + lodIndex;
			pDestLod = pNewHierarchyNode->aLods + lodIndex;

			pDestLod->pObj = pSourceLod->pObj;
			pDestLod->renderType = pSourceLod->renderType;
			pDestLod->sizeBias = pSourceLod->sizeBias;

			ED3D_LOG(LogLevel::Info, "ed3DHierarchyAddNode Valid pObj: {} field_0x4: {} sizeBias: {}", pDestLod->pObj, pDestLod->renderType, pDestLod->sizeBias);
		}
		else {
			memset(pNewHierarchyNode->aLods + lodIndex, 0, sizeof(ed3DLod));
		}

		lodIndex = lodIndex + 1;
	} while (lodIndex < 4);

	(pNewHierarchyNode->base).bRenderShadow = 0;
	(pNewHierarchyNode->base).GlobalAlhaON = -1;
	(pNewHierarchyNode->base).pShadowAnimMatrix = (edF32MATRIX4*)0x0;
	(pNewHierarchyNode->base).pLinkTransformData = p3DB;
	(pNewHierarchyNode->base).field_0x94 = (undefined*)p3DB;
	pMVar8 = pHierNode + newNodeIndex;

	if ((pNewHierarchyNode->base).linkedHierCount == 0) {
		(pNewHierarchyNode->base).flags_0x9e = (pNewHierarchyNode->base).flags_0x9e | 8;
	}

	(pMVar8->base).flags_0x9e = (pMVar8->base).flags_0x9e & 0xfffc;
	(pMVar8->base).flags_0x9e = (pMVar8->base).flags_0x9e | 0x800;
	ed3DSetSceneRender(&pMVar8->base, 1);
	pNewNode->pData = pMVar8;

	return pNewNode;
}

void ed3DHierarchyAddToListRecursive(edLIST* pList, ed_3d_hierarchy_node* pHierNode, edNODE* pNode, edNODE* pMeshTransformParent, ed_hash_code* pHashCode, ed_Chunck* peVar7)
{
	edNODE* peVar9;

	ed_Chunck* iVar1 = (ed_Chunck*)LOAD_SECTION(pHashCode->pData);

	ed_g3d_hierarchy* pNewHier = (ed_g3d_hierarchy*)(iVar1 + 1);

	ed_3d_hierarchy_node* pNewHierNode = (ed_3d_hierarchy_node*)LOAD_SECTION(pNewHier->pLinkTransformData);

	if ((ed_Chunck*)pNewHierNode == peVar7) {
		peVar9 = ed3DHierarchyAddNode(pList, pHierNode, pNode, pNewHier, (ed_3d_hierarchy*)pMeshTransformParent->pData);
		ed3DHierarchyAddToListRecursive(pList, pHierNode, pNode, pMeshTransformParent, pHashCode, iVar1);
	}

	return;
}

edNODE* ed3DHierarchyAddToList(edLIST* pList, ed_3d_hierarchy_node* pHierNode, edNODE* pNode, ed_g3d_manager* pMeshInfo, char* szString)
{
	ed_Chunck* iVar1;
	ed_Chunck* iVar2;
	int iVar3;
	int iVar4;
	int iVar5;
	ed_Chunck* pChunck;
	uint uVar6;
	ed_Chunck* peVar7;
	int subMeshCount;
	edNODE* pMeshTransformParent;
	edNODE* peVar9;
	edNODE* peVar10;
	edNODE* peVar11;
	edNODE* peVar12;
	edNODE* peVar13;
	edNODE* peVar14;
	edNODE* pNewNode;
	uint uVar15;
	ed_hash_code* pHashCode;
	uint uVar16;
	ed_hash_code* peVar17;
	ed_hash_code* local_1f0;
	edNODE* local_1d4;
	uint local_1d0;
	ed_hash_code* local_1a0;
	edNODE* local_184;
	uint local_180;
	ed_hash_code* local_150;
	edNODE* local_134;
	uint local_130;
	ed_hash_code* local_100;
	edNODE* local_e4;
	uint local_e0;
	ed_hash_code* local_b0;
	edNODE* local_94;
	uint local_90;
	ed_hash_code* local_60;
	edNODE* local_44;
	uint local_40;
	edNODE* local_8;
	short sStack2;
	ed_Chunck* pMeshHALL;

	peVar7 = (ed_Chunck*)0x0;
	pMeshHALL = pMeshInfo->HALL;
	pHashCode = (ed_hash_code*)(pMeshHALL + 2);
	uVar6 = edChunckGetNb(pMeshHALL + 1, (char*)pMeshHALL + pMeshHALL->size);
	uVar6 = (uVar6 & 0xffff) - 1;

	if (szString == (char*)0x0) {
		int subMeshIndex = -1;
		peVar17 = pHashCode;
		for (uVar15 = 0; uVar15 < uVar6; uVar15 = uVar15 + 1) {
			ed_Chunck* pHierChunk = (ed_Chunck*)LOAD_SECTION(peVar17->pData);
			ed_g3d_hierarchy* pHier = (ed_g3d_hierarchy*)(pHierChunk + 1);

			subMeshCount = pHier->subMeshParentCount_0xac;
			if (subMeshIndex < subMeshCount) {
				peVar7 = (ed_Chunck*)LOAD_SECTION(peVar17->pData);
				subMeshIndex = subMeshCount;
			}
			peVar17 = peVar17 + 1;
		}
	}
	else {
		IMPLEMENTATION_GUARD(
			peVar7 = ed3DG3DHierarchyGetChunk(pMeshInfo, szString);)
	}

	pMeshTransformParent = (edNODE*)0x0;

	if (peVar7 != (ed_Chunck*)0x0) {
		pMeshTransformParent = ed3DHierarchyAddNode(pList, pHierNode, pNode, (ed_g3d_hierarchy*)(peVar7 + 1), (ed_3d_hierarchy*)0x0);
		local_8 = (edNODE*)0x0;
		peVar17 = pHashCode;
		for (uVar16 = 0; uVar16 < uVar6; uVar16 = uVar16 + 1) {
			ed3DHierarchyAddToListRecursive(pList, pHierNode, pNode, pMeshTransformParent, peVar17, peVar7);
			peVar17 = peVar17 + 1;
		}
		ed3DHierarchyRefreshSonNumbers(pMeshTransformParent, &sStack2);
	}

	return pMeshTransformParent;
}

void ed3DHierarchyAddSonsToList(edLIST* pList, ed_3d_hierarchy_node* pHierNode, edNODE* pParentNode, ed_Chunck* pChunck, edNODE* pNewNode,
	ed_hash_code* pHashCode, uint count)
{
	ed_Chunck* pHierChunkB;
	char* pcVar2;
	char* pcVar3;
	char* pcVar4;
	ed_3d_hierarchy_setup* peVar5;
	ed_3d_hierarchy_node* peVar6;
	ed_Chunck* pChunck_00;
	edNODE* pNewHierNodeA;
	edNODE* peVar8;
	edNODE* peVar9;
	edNODE* peVar10;
	edNODE* peVar11;
	edNODE* peVar12;
	edNODE* peVar13;
	edNODE* pNewNode_00;
	uint currentIndexA;
	edNODE* pResultNodeA;
	ed_hash_code* pHashCodeA;
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
	ed_hash_code* pHashCodeB;
	edNODE* pResultNodeB;
	uint currentIndexB;
	ed_Chunck* pHierChunkA;

	pResultNodeA = (edNODE*)0x0;
	pHashCodeA = pHashCode;

	for (currentIndexA = 0; currentIndexA < count; currentIndexA = currentIndexA + 1) {
		pHierChunkA = LOAD_SECTION_CAST(ed_Chunck*, pHashCodeA->pData);
		ed_g3d_hierarchy* pHierarchy = (ed_g3d_hierarchy*)(pHierChunkA + 1);
		ed_3d_hierarchy_node* pHierarchyNode = LOAD_SECTION_CAST(ed_3d_hierarchy_node*, pHierarchy->pLinkTransformData);

		if (pHierarchyNode == (ed_3d_hierarchy_node*)pChunck) {
			pNewHierNodeA = ed3DHierarchyAddNode(pList, pHierNode, pParentNode, pHierarchy, (ed_3d_hierarchy*)pNewNode->pData);

			pResultNodeB = (edNODE*)0x0;
			pHashCodeB = pHashCode;

			for (currentIndexB = 0; currentIndexB < count; currentIndexB = currentIndexB + 1) {
				pHierChunkB = LOAD_SECTION_CAST(ed_Chunck*, pHashCodeB->pData);
				pHierarchy = (ed_g3d_hierarchy*)(pHierChunkB + 1);
				pHierarchyNode = LOAD_SECTION_CAST(ed_3d_hierarchy_node*, pHierarchy->pLinkTransformData);

				if (pHierarchyNode == (ed_3d_hierarchy_node*)pHierChunkA) {
					IMPLEMENTATION_GUARD();
					pHierarchy = (ed_g3d_hierarchy*)(pHierChunkB + 1);
					peVar8 = ed3DHierarchyAddNode(pList, pHierNode, pParentNode, pHierarchy, (ed_3d_hierarchy*)pNewHierNodeA->pData);
					local_74 = (edNODE*)0x0;
					local_90 = pHashCode;
					for (local_70 = 0; local_70 < count; local_70 = local_70 + 1) {
						pcVar2 = (char*)LOAD_SECTION(local_90->pData);
						if (*(char**)(pcVar2 + 0xa0) == (char*)pHierChunkB) {
							peVar9 = ed3DHierarchyAddNode
							(pList, pHierNode, pParentNode, (ed_g3d_hierarchy*)(pcVar2 + 0x10),
								(ed_3d_hierarchy*)peVar8->pData);
							local_c4 = (edNODE*)0x0;
							local_e0 = pHashCode;
							for (local_c0 = 0; local_c0 < count; local_c0 = local_c0 + 1) {
								pcVar3 = (char*)LOAD_SECTION(local_e0->pData);
								if (*(char**)(pcVar3 + 0xa0) == pcVar2) {
									peVar10 = ed3DHierarchyAddNode
									(pList, pHierNode, pParentNode, (ed_g3d_hierarchy*)(pcVar3 + 0x10),
										(ed_3d_hierarchy*)peVar9->pData);
									local_114 = (edNODE*)0x0;
									local_130 = pHashCode;
									for (local_110 = 0; local_110 < count; local_110 = local_110 + 1) {
										pcVar4 = (char*)LOAD_SECTION(local_130->pData);
										if (*(char**)(pcVar4 + 0xa0) == pcVar3) {
											peVar11 = ed3DHierarchyAddNode
											(pList, pHierNode, pParentNode, (ed_g3d_hierarchy*)(pcVar4 + 0x10),
												(ed_3d_hierarchy*)peVar10->pData);
											local_164 = (edNODE*)0x0;
											local_180 = pHashCode;
											for (local_160 = 0; local_160 < count; local_160 = local_160 + 1) {
												peVar5 = (ed_3d_hierarchy_setup*)local_180->pData;
												if (peVar5[8].clipping_0x0 == pcVar4) {
													peVar12 = ed3DHierarchyAddNode
													(pList, pHierNode, pParentNode, (ed_g3d_hierarchy*)&peVar5->field_0x10,
														(ed_3d_hierarchy*)peVar11->pData);
													local_1b4 = (edNODE*)0x0;
													local_1d0 = pHashCode;
													for (local_1b0 = 0; local_1b0 < count; local_1b0 = local_1b0 + 1) {
														peVar6 = (ed_3d_hierarchy_node*)local_1d0->pData;
														if ((peVar6->base).pHierarchySetup == peVar5) {
															peVar13 = ed3DHierarchyAddNode
															(pList, pHierNode, pParentNode,
																(ed_g3d_hierarchy*)&(peVar6->base).transformA.ba,
																(ed_3d_hierarchy*)peVar12->pData);
															local_204 = (edNODE*)0x0;
															local_220 = pHashCode;
															for (local_200 = 0; local_200 < count; local_200 = local_200 + 1) {
																pChunck_00 = (ed_Chunck*)local_220->pData;

																pHierarchy = (ed_g3d_hierarchy*)(pChunck_00 + 1);
																pHierarchyNode = (ed_3d_hierarchy_node*)LOAD_SECTION(pHierarchy->pLinkTransformData);

																if (pHierarchyNode == peVar6) {
																	pNewNode_00 = ed3DHierarchyAddNode(pList, pHierNode, pParentNode, pHierarchy, (ed_3d_hierarchy*)peVar13->pData);
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
					if (pResultNodeB != (edNODE*)0x0) {
						pResultNodeB = peVar8;
					}
				}
				pHashCodeB = pHashCodeB + 1;
			}
			if (pResultNodeA != (edNODE*)0x0) {
				pResultNodeA = pNewHierNodeA;
			}
		}
		pHashCodeA = pHashCodeA + 1;
	}
	return;
}

edNODE* ed3DHierarchyRefreshSonNumbers(edNODE* pInNode, short* outMeshCount)
{
	short uVar1;
	edNODE* pOutNode;
	ed_3d_hierarchy_node* pDVar4;
	short local_2;
	ed_3d_hierarchy_node* pDVar1;

	local_2 = 0;
	pDVar1 = (ed_3d_hierarchy_node*)pInNode->pData;
	pOutNode = pInNode->pPrev;
	pDVar4 = (ed_3d_hierarchy_node*)pOutNode->pData;

	MY_LOG("ed3DHierarchyRefreshSonNumbers hierarchy: {:x}", (uintptr_t)pDVar1);

	if ((ed_3d_hierarchy_node*)(pDVar4->base).pLinkTransformData == pDVar1) {
		uVar1 = 0;
		while ((ed_3d_hierarchy_node*)(pDVar4->base).pLinkTransformData == pDVar1) {
			pOutNode = ed3DHierarchyRefreshSonNumbers(pOutNode, &local_2);
			uVar1 = uVar1 + local_2;
			pDVar4 = (ed_3d_hierarchy_node*)pOutNode->pData;
		}
		(pDVar1->base).linkedHierCount = uVar1;
		*outMeshCount = uVar1 + 1;
	}
	else {
		(pDVar1->base).linkedHierCount = 0;
		*outMeshCount = 1;
	}
	return pOutNode;
}

bool CHierarchyAnm::UpdateMatrix(float param_1, edF32MATRIX4* pMatrix, S_HIERANM_ANIM* pHierAnim, int param_5)
{
	int iVar1;
	int iVar2;
	int iVar3;
	float* pfVar4;
	float* pfVar5;
	float* pfVar6;
	float* pfVar8;
	edF32VECTOR4* peVar9;
	edF32VECTOR4* peVar10;
	edF32VECTOR4* pfVar7;
	edF32VECTOR4* peVar11;
	float fVar12;
	float fVar13;
	float fVar14;
	float fVar15;
	float fVar16;
	float fVar17;
	float fVar18;
	float fVar19;
	bool bSuccess;
	edF32VECTOR4 local_30;
	edF32VECTOR4 eStack32;
	int local_10;
	int local_c;
	float local_8;

	pfVar5 = pHierAnim->field_0xc;
	iVar1 = pHierAnim->field_0x0;
	fVar15 = 0.0;
	iVar2 = pHierAnim->field_0x4;
	iVar3 = pHierAnim->field_0x8;
	pfVar4 = pfVar5 + iVar1 + 1;
	bSuccess = false;
	pfVar6 = pfVar4 + iVar2 + 1;
	pfVar7 = (edF32VECTOR4*)((ulong)pfVar6 + (iVar3 + 1) * 4 + 0xf & 0xfffffffffffffff0);
	fVar14 = pfVar5[iVar1];
	peVar11 = pfVar7 + iVar1;
	peVar10 = peVar11 + iVar2;
	if (0.0f < fVar14) {
		if (param_5 == 0) {
			fVar15 = param_1;
			if (fVar14 - 1e-06f < param_1) {
				bSuccess = true;
				fVar15 = fVar14 - 1e-06f;
			}
		}
		else {
			fVar15 = fmodf(param_1, fVar14);
		}
	}
	if (iVar2 == 0) {
		edF32Matrix4SetIdentityHard(pMatrix);
	}
	else {
		pfVar8 = pfVar4;
		if (iVar2 == 1) {
			edQuatToMatrix4Hard(peVar11, pMatrix);
		}
		else {
			for (; *pfVar8 <= fVar15; pfVar8 = pfVar8 + 1) {
			}

			int size = (char*)pfVar8 - (char*)pfVar4;

			if (size < 0) {
				size = size + 3;
			}
			local_c = size >> 2;
			local_10 = local_c + -1;
			local_8 = (fVar15 - pfVar8[-1]) / (*pfVar8 - pfVar8[-1]);
			if (iVar2 == local_c) {
				local_c = 0;
			}
			edQuatShortestSLERPHard(local_8, &eStack32, peVar11 + local_10, peVar11 + local_c);
			edQuatToMatrix4Hard(&eStack32, pMatrix);
		}
	}
	if (iVar1 != 0) {
		pfVar4 = pfVar5;
		if (iVar1 == 1) {
			fVar13 = pfVar7->y;
			fVar14 = pfVar7->z;
			fVar12 = pfVar7->w;
			pMatrix->da = pfVar7->x;
			pMatrix->db = fVar13;
			pMatrix->dc = fVar14;
			pMatrix->dd = fVar12;
		}
		else {
			for (; *pfVar4 <= fVar15; pfVar4 = pfVar4 + 1) {
			}

			int size = (char*)pfVar4 - (char*)pfVar5;
			if (size < 0) {
				size = size + 3;
			}
			local_c = size >> 2;
			local_10 = local_c + -1;
			local_8 = (fVar15 - pfVar4[-1]) / (*pfVar4 - pfVar4[-1]);
			if (iVar1 == local_c) {
				local_c = 0;
			}
			peVar9 = pfVar7 + local_c;
			peVar11 = pfVar7 + local_10;
			fVar12 = peVar11->y;
			fVar13 = peVar11->z;
			fVar16 = peVar11->w;
			fVar17 = peVar9->y;
			fVar18 = peVar9->z;
			fVar19 = peVar9->w;
			fVar14 = 1.0f - local_8;
			pMatrix->da = peVar9->x * local_8 + peVar11->x * fVar14;
			pMatrix->db = fVar17 * local_8 + fVar12 * fVar14;
			pMatrix->dc = fVar18 * local_8 + fVar13 * fVar14;
			pMatrix->dd = fVar19 * local_8 + fVar16 * fVar14;
			pMatrix->dd = 1.0f;
		}
	}
	if (iVar3 != 0) {
		pfVar4 = pfVar6;
		if (iVar3 != 1) {
			for (; *pfVar4 <= fVar15; pfVar4 = pfVar4 + 1) {
			}

			int size = (char*)pfVar4 - (char*)pfVar6;
			if (size < 0) {
				size = size + 3;
			}
			local_c = size >> 2;

			local_10 = local_c + -1;
			local_8 = (fVar15 - pfVar4[-1]) / (*pfVar4 - pfVar4[-1]);
			if (iVar3 == local_c) {
				local_c = 0;
			}
			peVar11 = peVar10 + local_c;
			peVar10 = peVar10 + local_10;
			fVar15 = 1.0f - local_8;
			local_30.x = peVar11->x * local_8 + peVar10->x * fVar15;
			local_30.y = peVar11->y * local_8 + peVar10->y * fVar15;
			local_30.z = peVar11->z * local_8 + peVar10->z * fVar15;
			local_30.w = peVar11->w * local_8 + peVar10->w * fVar15;
			peVar10 = &local_30;
		}
		_gscale_mat.aa = peVar10->x;
		_gscale_mat.bb = peVar10->y;
		_gscale_mat.cc = peVar10->z;
		edF32Matrix4MulF32Matrix4Hard(pMatrix, &_gscale_mat, pMatrix);
	}
	return bSuccess;
}

struct MeshData_OBB
{
	char header[4];
	int field_0x4;
	int field_0x8;
	int field_0x12;
};

ed_Chunck* ed3DHierarchyNodeGetSkeletonChunck(edNODE* pMeshTransformParent, bool bGetFromHierarc)
{
	byte desiredLod;
	ed_3d_hierarchy* pMeshTransformData;
	ed_Chunck* pObjChunck;
	ed3DLod* pGoodLod;
	ed_hash_code* pLodHash;
	ed_Chunck* pObbChunck;

	pMeshTransformData = (ed_3d_hierarchy*)pMeshTransformParent->pData;

	ED3D_LOG(LogLevel::VeryVerbose, "ed3DHierarchyNodeGetSkeletonChunck L.O.D count: {}", pMeshTransformData->lodCount);
	
	if ((pMeshTransformData->lodCount != 0) &&
		(pGoodLod = ed3DChooseGoodLOD(pMeshTransformData), pGoodLod != (ed3DLod*)0x0)) {

		desiredLod = pMeshTransformData->desiredLod;

		ED3D_LOG(LogLevel::VeryVerbose, "ed3DHierarchyNodeGetSkeletonChunck desired L.O.D: {}", desiredLod);

		if ((bGetFromHierarc != false) && ((pMeshTransformData->flags_0x9e & 0x100) != 0)) {
			ed3DHierarcGetLOD(pMeshTransformData, pMeshTransformData->lodCount - 1);
			desiredLod = pMeshTransformData->lodCount - 1;
			ED3D_LOG(LogLevel::VeryVerbose, "ed3DHierarchyNodeGetSkeletonChunck picked heirarc L.O.D: {}", desiredLod);
		}

		ed3DLod* pLod = (ed3DLod*)(pMeshTransformData + 1);

		pLodHash = (ed_hash_code*)LOAD_SECTION(pLod[desiredLod].pObj);

		if (pLodHash == (ed_hash_code*)0x0) {
			pLodHash = (ed_hash_code*)LOAD_SECTION(pLod[0].pObj);

			ED3D_LOG(LogLevel::VeryVerbose, "ed3DHierarchyNodeGetSkeletonChunck could not find L.O.D defaulting to 0");
		}

		// Look for .obj
		if ((pLodHash != (ed_hash_code*)0x0) && (pObjChunck = (ed_Chunck*)LOAD_SECTION(pLodHash->pData), pObjChunck->hash == 0x2e4a424f /*OBJ*/)) {

			ED3D_LOG(LogLevel::VeryVerbose, "ed3DHierarchyNodeGetSkeletonChunck found OBJ chunk offset: {} size: {}", pObjChunck->nextChunckOffset, pObjChunck->size);
			for (pObbChunck = edChunckGetFirst((char*)pObjChunck + pObjChunck->nextChunckOffset, (char*)pObjChunck + pObjChunck->size);
				pObbChunck != (ed_Chunck*)0x0;
				pObbChunck = edChunckGetNext(pObbChunck, (char*)pObjChunck + pObjChunck->size)) {
				/* Look for OBBG */
				if (pObbChunck->hash == 0x4742424f /*OBBG*/) {
					ED3D_LOG(LogLevel::VeryVerbose, "ed3DHierarchyNodeGetSkeletonChunck found valid skeleton chunck!");
					return pObbChunck;
				}
			}
		}
	}

	ED3D_LOG(LogLevel::VeryVerbose, "ed3DHierarchyNodeGetSkeletonChunck couldn't find good L.O.D!");
	return (ed_Chunck*)0x0;
}

struct NodeChunk {
	uint nodeIndex;
	int chunk; // char*
};

void CHierarchyAnm::Install(MeshData_ANHR* pInANHR, int length, ed_g3d_manager* pMeshInfo, ed_3D_Scene* pStaticMeshMaster)
{
	uint otherEntryCount;
	ed_Chunck* pcVar2;
	edLIST* pList;
	ed_3d_hierarchy_node* pHierNode;
	edNODE* pParentNode;
	uint fileDatEntryCount;
	ed_hash_code* pcVar4;
	edNODE* pNewNode;
	int iVar5;
	ed_Chunck* pSkeletonChunck;
	ulong meshHashValue;
	int* aFileData;
	float* pfVar7;
	NodeChunk* pCurNodeChunk;
	uint uVar9;
	S_HIERANM_ANIM* pHierAnimStream;
	ANHR_Internal* pANHR_Internal;
	uint uVar11;
	uint uVar12;
	uint uVar13;
	float fVar14;
	float puVar15;
	float fVar16;
	float local_20;
	float local_1c;
	float local_18;
	edAnmSkeleton local_8;
	short sStack2;
	MeshData_ANHR* pcVar1;

	if (pInANHR == (MeshData_ANHR*)0x0) {
		this->pThis = (MeshData_ANHR*)0x0;
	}
	else {
		this->pThis = pInANHR;
		pcVar1 = this->pThis;
		otherEntryCount = pcVar1->otherEntryCount;
		fileDatEntryCount = pcVar1->fileDataEntryCount;

		// aFileData starts after the 'other' entries.
		aFileData = (int*)((char*)(pInANHR + 1) + (otherEntryCount * sizeof(int)));

		// Take the offsets, and add them to the base and store the result ptr back into the packed struct.
		for (uVar9 = 0; uVar11 = 0, uVar9 < fileDatEntryCount; uVar9 = uVar9 + 1) {
			aFileData[uVar9] = (int)STORE_SECTION((char*)pInANHR + aFileData[uVar9]);
		}

		int* pBaseIndexes = (int*)(pInANHR + 1);
		for (; uVar11 < otherEntryCount; uVar11 = uVar11 + 1) {
			pANHR_Internal = (ANHR_Internal*)((char*)pInANHR + pBaseIndexes[uVar11]);
			pBaseIndexes[uVar11] = (int)STORE_SECTION(pANHR_Internal);
			meshHashValue = ByteCode::BuildU64((int)pANHR_Internal->pHierNode, (int)pANHR_Internal->pHierNodeData);
			ed3DHierarchyCopyHashCode(pMeshInfo);
			pParentNode = gHierarchyManagerFirstFreeNode;
			pHierNode = gHierarchyManagerBuffer;
			pcVar2 = pMeshInfo->HALL;
			pList = pStaticMeshMaster->pHierListA;
			fileDatEntryCount = edChunckGetNb(pcVar2 + 1, (char*)pcVar2 + pcVar2->size);
			pcVar4 = edHashcodeGet(meshHashValue, (ed_Chunck*)(pMeshInfo->HALL + 1));
			if (pcVar4 == (ed_hash_code*)0x0) {
				iVar5 = 0;
			}
			else {
				iVar5 = pcVar4->pData;
			}
			if (iVar5 == 0) {
				pNewNode = (edNODE*)0x0;
			}
			else {
				ed_Chunck* pChunck = (ed_Chunck*)LOAD_SECTION(iVar5);
				pNewNode = ed3DHierarchyAddNode(pList, pHierNode, pParentNode, (ed_g3d_hierarchy*)(pChunck + 1), (ed_3d_hierarchy*)0x0);
				ed3DHierarchyAddSonsToList(pList, pHierNode, pParentNode, (ed_Chunck*)LOAD_SECTION(iVar5), pNewNode, (ed_hash_code*)(pcVar2 + 2), (fileDatEntryCount & 0xffff) - 1);
				ed3DHierarchyRefreshSonNumbers(pNewNode, &sStack2);
			}
			pANHR_Internal->pHierNode = STORE_SECTION(pNewNode);
			if (pNewNode == (edNODE*)0x0) {
				pANHR_Internal->pHierNodeData = 0x0;
				pANHR_Internal->pHierAnimStream = 0x0;
			}
			else {
				pANHR_Internal->pHierNodeData = STORE_SECTION(pNewNode->pData);
				pANHR_Internal->flags = pANHR_Internal->flags | 0xa0000000;
				if ((pANHR_Internal->flags & 1) == 0) {
					pANHR_Internal->field_0x28 = 0.0f;
				}
				else {
					fVar14 = pANHR_Internal->field_0x28;
					fVar16 = pANHR_Internal->field_0x2c;
					iVar5 = rand();
					pANHR_Internal->field_0x28 = fVar14 + (fVar16 - fVar14) * ((float)iVar5 / 2.147484e+09f);
				}
				puVar15 = pANHR_Internal->pObbFloat.number;
				fVar14 = pANHR_Internal->field_0x34;
				iVar5 = rand();
				pANHR_Internal->field_0x2c = puVar15 + (fVar14 - puVar15) * ((float)iVar5 / 2.147484e+09f);
				if ((pANHR_Internal->flags & 4) != 0) {
					IMPLEMENTATION_GUARD(ed3DHierarchySetSetup((ed_3d_hierarchy_node*)LOAD_SECTION(pANHR_Internal->pHierNodeData), 0x40f070));
				}
				pHierAnimStream = (S_HIERANM_ANIM*)LOAD_SECTION(aFileData[(int)pANHR_Internal->pHierAnimStream]);

				UpdateMatrix(pANHR_Internal->field_0x28, &((ed_3d_hierarchy_node*)LOAD_SECTION(pANHR_Internal->pHierNodeData))->base.transformA, pHierAnimStream, pANHR_Internal->flags & 1);

				if (pHierAnimStream->field_0x8 == 0) {
					local_18 = 1.0;
					local_1c = 1.0;
					local_20 = 1.0;
				}
				else {
					int offsetA = pHierAnimStream->field_0x8 * 4 + pHierAnimStream->field_0x4 * 4 + pHierAnimStream->field_0x0 * 4 + -5;
					int offsetB = pHierAnimStream->field_0x0 * 0x10 + pHierAnimStream->field_0x4 * 0x10;

					ulong pStart = ((ulong)pHierAnimStream[2].field_0xc + offsetA) & 0xfffffffffffffff0U;
					pStart += offsetB;

					pfVar7 = (float*)pStart;

					local_20 = pfVar7[0];
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
				pANHR_Internal->field_0x1c = pANHR_Internal->field_0x1c * local_20 * 1.2f;
				if (((pHierAnimStream->field_0x0 < 2) && (pHierAnimStream->field_0x4 < 2)) && (pHierAnimStream->field_0x8 < 2)) {
					pANHR_Internal->pHierAnimStream = 0x0;
				}
				else {
					pANHR_Internal->pHierAnimStream = STORE_SECTION(pHierAnimStream);
				}
				pSkeletonChunck = ed3DHierarchyNodeGetSkeletonChunck(pNewNode, 0);
				if (pSkeletonChunck == (ed_Chunck*)0x0) {
					pANHR_Internal->pObbFloat.pObb_Internal = 0x0;
				}
				else {
					NodeChunk* pNext = (NodeChunk*)(pANHR_Internal + 1);

					pANHR_Internal->pObbFloat.pObb_Internal = STORE_SECTION(pSkeletonChunck + 1);

					local_8 = edAnmSkeleton((edANM_SKELETON*)LOAD_SECTION(pANHR_Internal->pObbFloat.pObb_Internal));
					fileDatEntryCount = pANHR_Internal->nodeChunkCount;
					for (uVar9 = 0; uVar9 < fileDatEntryCount; uVar9 = uVar9 + 1) {
						pNext->nodeIndex = local_8.NodeIndexFromID(pNext->nodeIndex);
						pNext->chunk = STORE_SECTION(LOAD_SECTION(aFileData[pNext->chunk]));
						pNext = pNext + 1;
					}
					pNext = (NodeChunk*)(pANHR_Internal + 1);
					for (uVar9 = 0; uVar9 < fileDatEntryCount; uVar9 = uVar9 + 1) {
						pCurNodeChunk = pNext;
						for (uVar12 = uVar9; (pCurNodeChunk->nodeIndex != uVar9 + 1 && (uVar12 < fileDatEntryCount)); uVar12 = uVar12 + 1) {
							pCurNodeChunk = pCurNodeChunk + 1;
						}
						if (pCurNodeChunk != pNext) {
							NodeChunk cachedChunk = *pCurNodeChunk;
							*pCurNodeChunk = *pNext;
							*pNext = cachedChunk;
						}
						pNext = pNext + 1;
					}
				}
			}
		}
	}
	return;
}

SceneConfig* ed3DSceneGetConfig(ed_3D_Scene* pStaticMeshMaster)
{
	SceneConfig* pCVar1;

	pCVar1 = (SceneConfig*)0x0;
	if (pStaticMeshMaster != (ed_3D_Scene*)0xffffffff) {
		pCVar1 = &pStaticMeshMaster->sceneConfig;
	}
	return pCVar1;
}

edSurface* ed3DShadowSurfaceNew(ed_surface_desc* pVidModeData)
{
	ed_surface_desc* pVidModeCopy;
	edSurface* pFrameBuffer;
	ZBufferTags* pTags;
	int zBufferFormat;

	pVidModeCopy = (ed_surface_desc*)edMemAlloc(TO_HEAP(H_MAIN), sizeof(ed_surface_desc));
	*pVidModeCopy = *pVidModeData;

	pFrameBuffer = (edSurface*)edMemAlloc(TO_HEAP(H_MAIN), sizeof(edSurface));
	memset(pFrameBuffer, 0, sizeof(edSurface));

	pFrameBuffer->pSurfaceDesc = pVidModeCopy;
	pFrameBuffer->pSurfaceDesc->pLink_0xc = pFrameBuffer;

	if (pVidModeData->bUseGlobalFrameBuffer == false) {
		pFrameBuffer->frameBasePtr = (uint)gFXBufAddr >> 5;
		pFrameBuffer->data_0xc = (char*)gFXBufAddr;
	}
	if ((pVidModeData->flags_0x8 & 8) != 0) {
		pTags = (ZBufferTags*)edMemAlloc(TO_HEAP(H_MAIN), sizeof(ZBufferTags));
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
		zBufferFormat = edSurfaceGetZBufferBpp(pFrameBuffer->pSurfaceDesc->pixelStoreMode);
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
	}
	pFrameBuffer->pNext = pFrameBuffer;
	return pFrameBuffer;
}

void ed3DHierarchyNodeClrFlag(edNODE* pNode, ushort flag)
{
	ushort uVar1;
	ed_3d_hierarchy* peVar2;
	ushort uVar3;
	edNODE* peVar4;
	edNODE* peVar5;
	edNODE* peVar6;
	edNODE* peVar7;
	edNODE* peVar8;
	edNODE* peVar9;
	edNODE* peVar10;
	edNODE* pNode_00;
	uint local_90;
	uint local_80;
	uint local_70;
	uint local_60;
	uint local_50;
	uint local_40;
	uint local_30;
	uint local_20;

	uVar3 = ~flag;
	peVar2 = (ed_3d_hierarchy*)pNode->pData;

	ED3D_LOG(LogLevel::Info, "ed3DHierarchyNodeClrFlag clearing flag {:x} for {}", flag, peVar2->hash.ToString());

	uVar1 = peVar2->linkedHierCount;
	peVar2->flags_0x9e = peVar2->flags_0x9e & uVar3;
	ed3DSetSceneRender(peVar2, 1);
	peVar8 = pNode->pPrev;
	for (local_20 = (uint)uVar1; local_20 != 0; local_20 = local_20 - 1) {
		ed3DSetSceneRender((ed_3d_hierarchy*)peVar8->pData, 1);
		peVar2 = (ed_3d_hierarchy*)peVar8->pData;
		uVar1 = peVar2->linkedHierCount;
		peVar2->flags_0x9e = peVar2->flags_0x9e & uVar3;
		ed3DSetSceneRender(peVar2, 1);
		peVar7 = peVar8->pPrev;
		for (local_30 = (uint)uVar1; local_30 != 0; local_30 = local_30 - 1) {
			ed3DSetSceneRender((ed_3d_hierarchy*)peVar7->pData, 1);
			peVar2 = (ed_3d_hierarchy*)peVar7->pData;
			uVar1 = peVar2->linkedHierCount;
			peVar2->flags_0x9e = peVar2->flags_0x9e & uVar3;
			ed3DSetSceneRender(peVar2, 1);
			peVar6 = peVar7->pPrev;
			for (local_40 = (uint)uVar1; local_40 != 0; local_40 = local_40 - 1) {
				ed3DSetSceneRender((ed_3d_hierarchy*)peVar6->pData, 1);
				peVar2 = (ed_3d_hierarchy*)peVar6->pData;
				uVar1 = peVar2->linkedHierCount;
				peVar2->flags_0x9e = peVar2->flags_0x9e & uVar3;
				ed3DSetSceneRender(peVar2, 1);
				peVar5 = peVar6->pPrev;
				for (local_50 = (uint)uVar1; local_50 != 0; local_50 = local_50 - 1) {
					ed3DSetSceneRender((ed_3d_hierarchy*)peVar5->pData, 1);
					peVar2 = (ed_3d_hierarchy*)peVar5->pData;
					uVar1 = peVar2->linkedHierCount;
					peVar2->flags_0x9e = peVar2->flags_0x9e & uVar3;
					ed3DSetSceneRender(peVar2, 1);
					peVar4 = peVar5->pPrev;
					for (local_60 = (uint)uVar1; local_60 != 0; local_60 = local_60 - 1) {
						ed3DSetSceneRender((ed_3d_hierarchy*)peVar4->pData, 1);
						peVar2 = (ed_3d_hierarchy*)peVar4->pData;
						uVar1 = peVar2->linkedHierCount;
						peVar2->flags_0x9e = peVar2->flags_0x9e & uVar3;
						ed3DSetSceneRender(peVar2, 1);
						peVar9 = peVar4->pPrev;
						for (local_70 = (uint)uVar1; local_70 != 0; local_70 = local_70 - 1) {
							ed3DSetSceneRender((ed_3d_hierarchy*)peVar9->pData, 1);
							peVar2 = (ed_3d_hierarchy*)peVar9->pData;
							uVar1 = peVar2->linkedHierCount;
							peVar2->flags_0x9e = peVar2->flags_0x9e & uVar3;
							ed3DSetSceneRender(peVar2, 1);
							peVar10 = peVar9->pPrev;
							for (local_80 = (uint)uVar1; local_80 != 0; local_80 = local_80 - 1) {
								ed3DSetSceneRender((ed_3d_hierarchy*)peVar10->pData, 1);
								peVar2 = (ed_3d_hierarchy*)peVar10->pData;
								uVar1 = peVar2->linkedHierCount;
								peVar2->flags_0x9e = peVar2->flags_0x9e & uVar3;
								ed3DSetSceneRender(peVar2, 1);
								pNode_00 = peVar10->pPrev;
								for (local_90 = (uint)uVar1; local_90 != 0; local_90 = local_90 - 1) {
									ed3DSetSceneRender((ed_3d_hierarchy*)pNode_00->pData, 1);
									ed3DHierarchyNodeClrFlag(pNode_00, flag);
									pNode_00 = pNode_00->pPrev;
								}
								peVar10 = peVar10->pPrev;
							}
							peVar9 = peVar9->pPrev;
						}
						peVar4 = peVar4->pPrev;
					}
					peVar5 = peVar5->pPrev;
				}
				peVar6 = peVar6->pPrev;
			}
			peVar7 = peVar7->pPrev;
		}
		peVar8 = peVar8->pPrev;
	}
	return;
}

void ed3DHierarchyNodeSetFlag(edNODE* pNode, ushort flag)
{
	ushort uVar1;
	ed_3d_hierarchy* peVar2;
	edNODE* peVar3;
	edNODE* peVar4;
	edNODE* peVar5;
	edNODE* peVar6;
	edNODE* peVar7;
	edNODE* peVar8;
	edNODE* peVar9;
	edNODE* pNode_00;
	uint local_80;
	uint local_70;
	uint local_60;
	uint local_50;
	uint local_40;
	uint local_30;
	uint local_20;
	uint local_10;

	peVar2 = (ed_3d_hierarchy*)pNode->pData;

	ED3D_LOG(LogLevel::Info, "ed3DHierarchyNodeSetFlag setting flag {:x} for {}", flag, peVar2->hash.ToString());

	uVar1 = peVar2->linkedHierCount;
	peVar2->flags_0x9e = peVar2->flags_0x9e | flag;
	ed3DSetSceneRender(peVar2, 1);
	peVar7 = pNode->pPrev;
	for (local_10 = (uint)uVar1; local_10 != 0; local_10 = local_10 - 1) {
		ed3DSetSceneRender((ed_3d_hierarchy*)peVar7->pData, 1);
		peVar2 = (ed_3d_hierarchy*)peVar7->pData;
		uVar1 = peVar2->linkedHierCount;
		peVar2->flags_0x9e = peVar2->flags_0x9e | flag;
		ed3DSetSceneRender(peVar2, 1);
		peVar6 = peVar7->pPrev;
		for (local_20 = (uint)uVar1; local_20 != 0; local_20 = local_20 - 1) {
			ed3DSetSceneRender((ed_3d_hierarchy*)peVar6->pData, 1);
			peVar2 = (ed_3d_hierarchy*)peVar6->pData;
			uVar1 = peVar2->linkedHierCount;
			peVar2->flags_0x9e = peVar2->flags_0x9e | flag;
			ed3DSetSceneRender(peVar2, 1);
			peVar5 = peVar6->pPrev;
			for (local_30 = (uint)uVar1; local_30 != 0; local_30 = local_30 - 1) {
				ed3DSetSceneRender((ed_3d_hierarchy*)peVar5->pData, 1);
				peVar2 = (ed_3d_hierarchy*)peVar5->pData;
				uVar1 = peVar2->linkedHierCount;
				peVar2->flags_0x9e = peVar2->flags_0x9e | flag;
				ed3DSetSceneRender(peVar2, 1);
				peVar4 = peVar5->pPrev;
				for (local_40 = (uint)uVar1; local_40 != 0; local_40 = local_40 - 1) {
					ed3DSetSceneRender((ed_3d_hierarchy*)peVar4->pData, 1);
					peVar2 = (ed_3d_hierarchy*)peVar4->pData;
					uVar1 = peVar2->linkedHierCount;
					peVar2->flags_0x9e = peVar2->flags_0x9e | flag;
					ed3DSetSceneRender(peVar2, 1);
					peVar3 = peVar4->pPrev;
					for (local_50 = (uint)uVar1; local_50 != 0; local_50 = local_50 - 1) {
						ed3DSetSceneRender((ed_3d_hierarchy*)peVar3->pData, 1);
						peVar2 = (ed_3d_hierarchy*)peVar3->pData;
						uVar1 = peVar2->linkedHierCount;
						peVar2->flags_0x9e = peVar2->flags_0x9e | flag;
						ed3DSetSceneRender(peVar2, 1);
						peVar8 = peVar3->pPrev;
						for (local_60 = (uint)uVar1; local_60 != 0; local_60 = local_60 - 1) {
							ed3DSetSceneRender((ed_3d_hierarchy*)peVar8->pData, 1);
							peVar2 = (ed_3d_hierarchy*)peVar8->pData;
							uVar1 = peVar2->linkedHierCount;
							peVar2->flags_0x9e = peVar2->flags_0x9e | flag;
							ed3DSetSceneRender(peVar2, 1);
							peVar9 = peVar8->pPrev;
							for (local_70 = (uint)uVar1; local_70 != 0; local_70 = local_70 - 1) {
								ed3DSetSceneRender((ed_3d_hierarchy*)peVar9->pData, 1);
								peVar2 = (ed_3d_hierarchy*)peVar9->pData;
								uVar1 = peVar2->linkedHierCount;
								peVar2->flags_0x9e = peVar2->flags_0x9e | flag;
								ed3DSetSceneRender(peVar2, 1);
								pNode_00 = peVar9->pPrev;
								for (local_80 = (uint)uVar1; local_80 != 0; local_80 = local_80 - 1) {
									ed3DSetSceneRender((ed_3d_hierarchy*)pNode_00->pData, 1);
									ed3DHierarchyNodeSetFlag(pNode_00, flag);
									pNode_00 = pNode_00->pPrev;
								}
								peVar9 = peVar9->pPrev;
							}
							peVar8 = peVar8->pPrev;
						}
						peVar3 = peVar3->pPrev;
					}
					peVar4 = peVar4->pPrev;
				}
				peVar5 = peVar5->pPrev;
			}
			peVar6 = peVar6->pPrev;
		}
		peVar7 = peVar7->pPrev;
	}
	return;
}

void ed3DHierarchyNodeSetAlpha(edNODE* pNode, byte alpha)
{
	int subHierCount;

	if (0x7f < alpha) {
		alpha = 0xff;
	}

	ed_3d_hierarchy* pHier = reinterpret_cast<ed_3d_hierarchy*>(pNode->pData);

	if (pHier->GlobalAlhaON != alpha) {
		subHierCount = pHier->linkedHierCount + 1;
		do {
			subHierCount = subHierCount + -1;
			pHier->GlobalAlhaON = alpha;
			pNode = pNode->pPrev;
		} while (subHierCount != 0);
	}

	return;
}

void CHierarchyAnm::Manage(float param_1, float param_2, ed_3D_Scene* pScene, int param_5)
{
	MeshData_ANHR* pMVar1;
	uint uVar2;
	edNODE* pNode;
	float fVar3;
	CCameraManager* pCVar4;
	//edAnmSkeleton eVar5;
	bool bVar6;
	SceneConfig* pSVar7;
	uint uVar8;
	edF32MATRIX4* m0;
	edF32MATRIX4* peVar9;
	byte bVar10;
	edF32MATRIX4* pMatrix;
	int* piVar11;
	uint uVar12;
	ANHR_Internal* pAVar13;
	float fVar14;
	float fVar15;
	edF32VECTOR4 local_30;
	edF32VECTOR4 local_20;
	//edAnmSkeleton local_8;
	float local_4;
	ANHR_Internal* peVar3;

	pCVar4 = CCameraManager::_gThis;
	if (this->pThis != (MeshData_ANHR*)0x0) {
		if (param_2 == 0.0) {
			pSVar7 = ed3DSceneGetConfig(CScene::_scene_handleA);
			param_2 = pSVar7->clipValue_0x4;
		}
		pMVar1 = this->pThis;
		uVar2 = pMVar1->otherEntryCount;
		for (uVar12 = 0; uVar12 < uVar2; uVar12 = uVar12 + 1) {
			int* pNext = (int*)(pMVar1 + 1);
			ANHR_Internal* peVar3 = (ANHR_Internal*)LOAD_SECTION(pNext[uVar12]);
			pNode = (edNODE*)LOAD_SECTION(peVar3->pHierNode);
			if (pNode != (edNODE*)0x0) {
				local_4 = 0.0f;
				local_20.x = peVar3->field_0x10;
				local_20.y = peVar3->field_0x14;
				local_20.z = peVar3->field_0x18;
				fVar14 = 0.0f;
				bVar6 = false;
				local_20.w = 1.0f;

				ed_3d_hierarchy_node* pHierNodeData = (ed_3d_hierarchy_node*)LOAD_SECTION(peVar3->pHierNodeData);

				edF32Matrix4MulF32Vector4Hard(&local_20, &pHierNodeData->base.transformA, &local_20);
				local_30.xyz = local_20.xyz;
				local_30.w = peVar3->field_0x1c;
				if ((peVar3->flags & 0x40000000) == 0) {
					if ((peVar3->flags & 4) == 0) {
						if ((param_5 == 0) || (uVar8 = pCVar4->IsSphereVisible(param_2, &local_30, &local_4), uVar8 != 0)) {
							bVar6 = true;
						}
					}
					else {
						bVar6 = true;
					}
				}
				if (bVar6) {
					if ((peVar3->flags & 0x80000000) == 0) {
						peVar3->flags = peVar3->flags | 0x80000000;
						ed3DHierarchyNodeClrFlag(pNode, ED_3D_HIDDEN_FLAG);
					}

					MY_LOG("pObb_Internal %x", peVar3->pObbFloat.pObb_Internal);

					edAnmSkeleton skeleton = edAnmSkeleton((edANM_SKELETON*)LOAD_SECTION(peVar3->pObbFloat.pObb_Internal));

					m0 = TheAnimManager.GetMatrixBuffer(skeleton.pTag->boneCount);
					pHierNodeData->base.pAnimMatrix = m0;
					pHierNodeData->base.pShadowAnimMatrix = (edF32MATRIX4*)0x0;
					uVar8 = peVar3->flags & 0x20000000;
					peVar9 = m0 + skeleton.pTag->boneCount;
					if (uVar8 != 0) {
						if ((peVar3->flags & 8) == 0) {
							uVar8 = (uint)(local_4 < peVar3->field_0x24);
						}
						else {
							if (CActorHero::_gThis == (CActorHero*)0x0) {
								uVar8 = 0;
							}
							else {
								fVar14 = local_20.x - CActorHero::_gThis->currentLocation.x;
								fVar15 = local_20.y - CActorHero::_gThis->currentLocation.y;
								fVar3 = local_20.z - CActorHero::_gThis->currentLocation.z;
								fVar14 = fVar14 * fVar14 + fVar15 * fVar15 + fVar3 * fVar3;
								uVar8 = 1;
								if ((peVar3->field_0x28 <= 0.0) &&
									(peVar3->field_0x24 * peVar3->field_0x24 <= fVar14)) {
									uVar8 = 0;
								}
							}
						}
					}

					// #HACK !!!
					uVar8 = 0;

					if (uVar8 == 0) {
						for (; m0 < peVar9; m0 = m0 + 1) {
							edF32Matrix4SetIdentityHard(m0);
						}
					}
					else {
						IMPLEMENTATION_GUARD(
						bVar10 = 1;
						fVar15 = peVar3->field_0x28 + param_1 * peVar3->field_0x2c;
						peVar3->field_0x28 = fVar15;
						uVar8 = peVar3->field_0x20 & 1;
						if (peVar3->pHierAnimStream != (S_HIERANM_ANIM*)0x0) {
							bVar6 = UpdateMatrix(fVar15, this, (edF32MATRIX4*)peVar3->pHierNodeData, peVar3->pHierAnimStream, uVar8);
							bVar10 = bVar6 & 1;
						}
						eVar5 = skeleton;
						edF32Matrix4SetIdentityHard(m0);
						piVar11 = (int*)((int)eVar5 + 8);
						pAVar13 = peVar3 + 1;
						pMatrix = m0;
						while (pMatrix = pMatrix + 1, pMatrix < peVar9) {
							bVar6 = UpdateMatrix(fVar15, this, pMatrix, (S_HIERANM_ANIM*)pAVar13->pHierNodeData, uVar8);
							bVar10 = bVar10 & bVar6;
							if (0 < *piVar11) {
								edF32Matrix4MulF32Matrix4Hard(pMatrix, pMatrix, m0 + *piVar11);
							}
							pAVar13 = (ANHR_Internal*)&pAVar13->pHierAnimStream;
							piVar11 = piVar11 + 1;
						}
						skeleton.ApplyInvSkin(m0);
						if (bVar10 != 0) {
							if ((peVar3->field_0x20 & 2) != 0) {
								peVar3->field_0x20 = peVar3->field_0x20 | 0x40000000;
								ed3DHierarchyNodeSetFlag(pNode, ED_3D_HIDDEN_FLAG);
							}
							if (((peVar3->field_0x20 & 0x11) == 0) &&
								(((peVar3->field_0x20 & 8) == 0 ||
									(*(float*)&peVar3->field_0x24 * *(float*)&peVar3->field_0x24 < fVar14)))) {
								peVar3->field_0x28 = 0.0;
							}
						})
					}
				}
				else {
					if ((peVar3->flags & 0x80000000U) != 0) {
						ulong shift = (ulong)peVar3->flags << 0x21;
						peVar3->flags = (uint)(shift >> 0x21);
						ed3DHierarchyNodeSetFlag(pNode, ED_3D_HIDDEN_FLAG);
					}
				}
			}
		}
	}
	return;
}

void ed3DScenePushCluster(ed_3D_Scene* pStaticMeshMaster, ed_g3d_manager* pMeshInfo)
{
	edNODE* pNode;
	edCluster* pNewCluster;
	uint nextFreeClusterIndex;
	edNODE_MANAGER* pNodeManager;

	nextFreeClusterIndex = 0;

	// Find next free cluster.
	while ((nextFreeClusterIndex < ged3DConfig.maxClusterCount && ((gCluster[nextFreeClusterIndex].flags & 1) != 0))) {
		nextFreeClusterIndex = nextFreeClusterIndex + 1;
	}

	if (nextFreeClusterIndex < ged3DConfig.maxClusterCount) {
		pNewCluster = gCluster + nextFreeClusterIndex;
		gLoadedClusterCount = gLoadedClusterCount + 1;
		pNewCluster->pMeshInfo = pMeshInfo;
		pNewCluster->flags = pNewCluster->flags | 1;

		pNodeManager = (edNODE_MANAGER*)(pStaticMeshMaster->meshClusterList).pData;
		pNode = pNodeManager->pNodeHead;
		pNodeManager->pNodeHead = pNode->pPrev;
		pNode->pData = pNewCluster;
		pNodeManager->linkCount = pNodeManager->linkCount + 1;
		(pStaticMeshMaster->meshClusterList).nodeCount = (pStaticMeshMaster->meshClusterList).nodeCount + 1;
		pNode->pPrev = (pStaticMeshMaster->meshClusterList).pPrev;
		(pStaticMeshMaster->meshClusterList).pPrev = pNode;

		ED3D_LOG(LogLevel::Info, "ed3DScenePushCluster {} New cluster: {}", (uintptr_t)pNode, (uintptr_t)pNewCluster);
	}
	return;
}

edNODE* ed3DHierarchyAddToScene(ed_3D_Scene* pScene, ed_g3d_manager* pG3D, char* szString)
{
	edNODE* pNewNode;

	ed3DHierarchyCopyHashCode(pG3D);
	pNewNode = ed3DHierarchyAddToList(pScene->pHierListA, gHierarchyManagerBuffer, gHierarchyManagerFirstFreeNode, pG3D, szString);
	return pNewNode;
}

edNODE* ed3DHierarchyAddToSceneByHashcode(ed_3D_Scene* pStaticMeshMaster, ed_g3d_manager* pMeshInfo, ulong hash)
{
	edLIST* pList;
	ed_3d_hierarchy_node* pHierNode;
	edNODE* pNode;
	uint uVar1;
	ed_hash_code* pcVar2;
	edNODE* pNewNode;
	ed_Chunck* pChunck;
	short sStack2;
	ed_Chunck* pcVar1;

	ed3DHierarchyCopyHashCode(pMeshInfo);
	pNode = gHierarchyManagerFirstFreeNode;
	pHierNode = gHierarchyManagerBuffer;
	pcVar1 = pMeshInfo->HALL;
	pList = pStaticMeshMaster->pHierListA;
	uVar1 = edChunckGetNb(pcVar1 + 1, (char*)pcVar1 + pcVar1->size);
	pcVar2 = edHashcodeGet(hash, (ed_Chunck*)(pMeshInfo->HALL + 1));
	pChunck = (ed_Chunck*)0x0;

	if (pcVar2 != (ed_hash_code*)0x0) {
		pChunck = (ed_Chunck*)LOAD_SECTION(pcVar2->pData);
	}

	if (pChunck == (ed_Chunck*)0x0) {
		pNewNode = (edNODE*)0x0;
	}
	else {
		pNewNode = ed3DHierarchyAddNode(pList, pHierNode, pNode, (ed_g3d_hierarchy*)(pChunck + 1), (ed_3d_hierarchy*)0x0);
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

	uVar2 = (pScene->sceneConfig).projectionScaleFactorA;
	if ((int)uVar2 < 0) {
		fVar6 = (float)(uVar2 >> 1 | uVar2 & 1);
		fVar6 = fVar6 + fVar6;
	}
	else {
		fVar6 = (float)uVar2;
	}
	uVar2 = (pScene->sceneConfig).projectionScaleFactorB;
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

FxFogProp* ed3DGetFxFogProp(void)
{
	return &g3DFXFog;
}

ed_g3d_hierarchy* ed3DG3DHierarchyGetFromIndex(ed_g3d_manager* pMeshInfo, int count)
{
	ed_hash_code* pMVar1;

	pMVar1 = (ed_hash_code*)(pMeshInfo->HALL + 2);

	for (; count != 0; count = count + -1) {
		pMVar1 = pMVar1 + 1;
	}

	char* pLoaded = (char*)LOAD_SECTION(pMVar1->pData);

	return (ed_g3d_hierarchy*)(pLoaded + 0x10);
}

void ed3DObjectSetStripShadowCast(ed_hash_code* pLodHash, ushort flag, uint bApply)
{
	int iVar1;
	ed_3d_strip* peVar2;

	ed_Chunck* pOBJ = LOAD_SECTION_CAST(ed_Chunck*, pLodHash->pData);
	ed_g3d_object* pObjInternal = reinterpret_cast<ed_g3d_object*>(pOBJ + 1);

	peVar2 = (ed_3d_strip*)LOAD_SECTION(pObjInternal->p3DStrip);
	if (peVar2 != (ed_3d_strip*)0x0) {
		iVar1 = pObjInternal->stripCount;
		if (bApply == 0) {
			for (; iVar1 != 0; iVar1 = iVar1 + -1) {
				peVar2->shadowCastFlags = peVar2->shadowCastFlags & ~flag;
				peVar2 = (ed_3d_strip*)LOAD_SECTION(peVar2->pNext);
			}
		}
		else {
			for (; iVar1 != 0; iVar1 = iVar1 + -1) {
				peVar2->shadowCastFlags = peVar2->shadowCastFlags | flag;
				peVar2 = (ed_3d_strip*)LOAD_SECTION(peVar2->pNext);
			}
		}
	}
	return;
}

void ed3DObjectSetStripShadowReceive(ed_hash_code* pLodHash, ushort param_2, uint param_3)
{
	int iVar1;
	ed_3d_strip* peVar2;

	ed_Chunck* pOBJ = LOAD_SECTION_CAST(ed_Chunck*, pLodHash->pData);
	ed_g3d_object* pObjInternal = reinterpret_cast<ed_g3d_object*>(pOBJ + 1);

	peVar2 = (ed_3d_strip*)LOAD_SECTION(pObjInternal->p3DStrip);
	if (peVar2 != (ed_3d_strip*)0x0) {
		iVar1 = pObjInternal->stripCount;
		if (param_3 == 0) {
			for (; iVar1 != 0; iVar1 = iVar1 + -1) {
				peVar2->shadowReceiveFlags = peVar2->shadowReceiveFlags & ~param_2;
				peVar2 = (ed_3d_strip*)LOAD_SECTION(peVar2->pNext);
			}
		}
		else {
			for (; iVar1 != 0; iVar1 = iVar1 + -1) {
				peVar2->shadowReceiveFlags = peVar2->shadowReceiveFlags | param_2;
				peVar2 = (ed_3d_strip*)LOAD_SECTION(peVar2->pNext);
			}
		}
	}
	return;
}

// There shouldn't be two of these :|
ed3DLod* ed3DHierarcGetLOD(ed_3d_hierarchy* pHier, uint index)
{
	ed3DLod* peVar1;

	peVar1 = (ed3DLod*)0x0;
	if (index < pHier->lodCount) {
		ed3DLod* aLods = (ed3DLod*)(pHier + 1);
		peVar1 = aLods + index;
	}
	return peVar1;
}

ed3DLod* ed3DHierarcGetLOD(ed_g3d_hierarchy* pHier, uint index)
{
	ed3DLod* peVar1;

	peVar1 = (ed3DLod*)0x0;
	if (index < pHier->lodCount) {
		peVar1 = pHier->aLods + index;
	}
	return peVar1;
}

void ed3DG3DHierarchyNodeSetAndClrStripFlag(ed_g3d_hierarchy* pHier, ushort flag, byte param_3, uint param_4)
{
	short sVar1;
	ed3DLod* peVar2;
	float* pfVar3;
	uint uVar4;
	ed_hash_code* ppeVar5;
	uint uVar6;
	ed3DLod* pLod;

	if (((pHier->flags_0x9e & 2) != 0) || ((pHier->flags_0x9e & 4) != 0)) {
		if (param_3 == 0) {
			pHier->flags_0x9e = pHier->flags_0x9e & 0xfdff;
		}
		else {
			pHier->flags_0x9e = pHier->flags_0x9e | 0x200;
		}
	}
	uVar4 = (int)(char)param_3 & 0xff;
	uVar6 = 0;
	do {
		while (true) {
			if (pHier->lodCount <= uVar6) {
				return;
			}

			pLod = &pHier->aLods[uVar6];

			sVar1 = pLod->renderType;
			ppeVar5 = (ed_hash_code*)LOAD_SECTION(pLod->pObj);
			if (((sVar1 != 3) && (sVar1 != 2)) && (sVar1 != 1)) break;
		LAB_0029f5e0:
			uVar6 = uVar6 + 1;
		}
		if (sVar1 == 0) {
			if (param_4 == 4) {
				ed3DObjectSetStripShadowReceive(ppeVar5, flag, uVar4);
			}
			else {
				if (param_4 == 2) {
					if ((pHier->flags_0x9e & 0x100) == 0) {
						ed3DObjectSetStripShadowCast(ppeVar5, flag, uVar4);
					}
					else {
						peVar2 = ed3DHierarcGetLOD(pHier, pHier->lodCount - 1);
						if (peVar2 != (ed3DLod*)0x0) {
							ed3DObjectSetStripShadowCast((ed_hash_code*)LOAD_SECTION(peVar2->pObj), flag, uVar4);
						}
					}
				}
				else {
					if (param_4 == 1) {
						IMPLEMENTATION_GUARD(
						ed3DObjectSetStripProperty(ppeVar5, flag, uVar4);)
					}
				}
			}
			goto LAB_0029f5e0;
		}
		uVar6 = uVar6 + 1;
	} while (true);
}

void ed3DG3DHierarchySetStripShadowCastFlag(ed_g3d_hierarchy* pHier, ushort flag)
{
	ed_g3d_hierarchy* peVar1;
	ed3DLod* peVar2;
	ed_hash_code* peVar3;

	pHier->flags_0x9e = pHier->flags_0x9e | 0x200;
	if (pHier != (ed_g3d_hierarchy*)0x0) {
		if (((pHier->flags_0x9e & 2) != 0) || ((pHier->flags_0x9e & 4) != 0)) {
			pHier->flags_0x9e = pHier->flags_0x9e | 0x200;
		}

		// Walk back to chunk.
		ed_Chunck* pChunck = (ed_Chunck*)(((char*)pHier) - sizeof(ed_Chunck));

		if (pChunck->hash == 0x52454948) {
			while (pChunck->hash == 0x52454948) {
				ed_g3d_hierarchy* pNewHier = (ed_g3d_hierarchy*)(pChunck + 1);

				ed_3d_hierarchy_node* pNode = (ed_3d_hierarchy_node*)LOAD_SECTION(pHier->pLinkTransformData);

				if (pNode != (ed_3d_hierarchy_node*)0x0) {
					IMPLEMENTATION_GUARD(
					peVar1 = (ed_g3d_hierarchy*)(peVar2[0x14].pObj + 1);
					while (peVar1 != (ed_g3d_hierarchy*)0x0) {
						if (peVar1 == pHier) {
							ed3DG3DHierarchyNodeSetAndClrStripFlag((ed_g3d_hierarchy*)(peVar2 + 2), flag, 1, 2);
							break;
						}
						if (peVar1->pLinkTransformData == (ed_3d_hierarchy_node*)0x0) {
							peVar1 = (ed_g3d_hierarchy*)0x0;
						}
						else {
							peVar1 = (ed_g3d_hierarchy*)&(peVar1->pLinkTransformData->base).transformA.ba;
						}
					})
				}
				pChunck = edChunckGetNext(pChunck, (char*)0x0);
			}
			ed3DG3DHierarchyNodeSetAndClrStripFlag(pHier, flag, 1, 2);
		}
	}
	return;
}

void ed3DSetMeshTransformFlag_002abd80(edNODE* pNode, ushort flag)
{
	ed_3d_hierarchy* pMVar1;
	ushort uVar2;
	edNODE* peVar3;
	edNODE* peVar4;
	edNODE* peVar5;
	edNODE* peVar6;
	edNODE* peVar7;
	edNODE* peVar8;
	edNODE* peVar9;
	uint uVar10;
	edNODE* local_90;
	uint local_80;
	uint local_70;
	uint local_60;
	uint local_50;
	uint local_40;
	uint local_30;
	uint local_20;

	uVar2 = ~flag;
	pMVar1 = (ed_3d_hierarchy*)pNode->pData;
	pMVar1->bRenderShadow = pMVar1->bRenderShadow & uVar2;
	peVar7 = pNode->pPrev;
	for (uVar10 = (uint)(ushort)pMVar1->linkedHierCount; uVar10 != 0; uVar10 = uVar10 - 1) {
		pMVar1 = (ed_3d_hierarchy*)peVar7->pData;
		pMVar1->bRenderShadow = pMVar1->bRenderShadow & uVar2;
		peVar6 = peVar7->pPrev;
		for (local_20 = (uint)(ushort)pMVar1->linkedHierCount; local_20 != 0; local_20 = local_20 - 1) {
			pMVar1 = (ed_3d_hierarchy*)peVar6->pData;
			pMVar1->bRenderShadow = pMVar1->bRenderShadow & uVar2;
			peVar5 = peVar6->pPrev;
			for (local_30 = (uint)(ushort)pMVar1->linkedHierCount; local_30 != 0; local_30 = local_30 - 1) {
				pMVar1 = (ed_3d_hierarchy*)peVar5->pData;
				pMVar1->bRenderShadow = pMVar1->bRenderShadow & uVar2;
				peVar4 = peVar5->pPrev;
				for (local_40 = (uint)(ushort)pMVar1->linkedHierCount; local_40 != 0; local_40 = local_40 - 1) {
					pMVar1 = (ed_3d_hierarchy*)peVar4->pData;
					pMVar1->bRenderShadow = pMVar1->bRenderShadow & uVar2;
					peVar3 = peVar4->pPrev;
					for (local_50 = (uint)(ushort)pMVar1->linkedHierCount; local_50 != 0; local_50 = local_50 - 1) {
						pMVar1 = (ed_3d_hierarchy*)peVar3->pData;
						pMVar1->bRenderShadow = pMVar1->bRenderShadow & uVar2;
						peVar8 = peVar3->pPrev;
						for (local_60 = (uint)(ushort)pMVar1->linkedHierCount; local_60 != 0; local_60 = local_60 - 1) {
							pMVar1 = (ed_3d_hierarchy*)peVar8->pData;
							pMVar1->bRenderShadow = pMVar1->bRenderShadow & uVar2;
							peVar9 = peVar8->pPrev;
							for (local_70 = (uint)(ushort)pMVar1->linkedHierCount; local_70 != 0; local_70 = local_70 - 1) {
								pMVar1 = (ed_3d_hierarchy*)peVar9->pData;
								pMVar1->bRenderShadow = pMVar1->bRenderShadow & uVar2;
								local_90 = peVar9->pPrev;
								for (local_80 = (uint)(ushort)pMVar1->linkedHierCount; local_80 != 0; local_80 = local_80 - 1) {
									ed3DSetMeshTransformFlag_002abd80(local_90, flag);
									local_90 = local_90->pPrev;
								}
								peVar9 = peVar9->pPrev;
							}
							peVar8 = peVar8->pPrev;
						}
						peVar3 = peVar3->pPrev;
					}
					peVar4 = peVar4->pPrev;
				}
				peVar5 = peVar5->pPrev;
			}
			peVar6 = peVar6->pPrev;
		}
		peVar7 = peVar7->pPrev;
	}
	return;
}

void ed3DSetMeshTransformFlag_002abff0(edNODE* pNode, ushort flag)
{
	ed_3d_hierarchy* pMVar1;
	edNODE* peVar2;
	edNODE* peVar3;
	edNODE* peVar4;
	edNODE* peVar5;
	edNODE* peVar6;
	edNODE* peVar7;
	edNODE* peVar8;
	uint uVar9;
	edNODE* local_80;
	uint local_70;
	uint local_60;
	uint local_50;
	uint local_40;
	uint local_30;
	uint local_20;
	uint local_10;

	pMVar1 = (ed_3d_hierarchy*)pNode->pData;
	pMVar1->bRenderShadow = pMVar1->bRenderShadow | flag;
	peVar6 = pNode->pPrev;
	for (uVar9 = (uint)(ushort)pMVar1->linkedHierCount; uVar9 != 0; uVar9 = uVar9 - 1) {
		pMVar1 = (ed_3d_hierarchy*)peVar6->pData;
		pMVar1->bRenderShadow = pMVar1->bRenderShadow | flag;
		peVar5 = peVar6->pPrev;
		for (local_10 = (uint)(ushort)pMVar1->linkedHierCount; local_10 != 0; local_10 = local_10 - 1) {
			pMVar1 = (ed_3d_hierarchy*)peVar5->pData;
			pMVar1->bRenderShadow = pMVar1->bRenderShadow | flag;
			peVar4 = peVar5->pPrev;
			for (local_20 = (uint)(ushort)pMVar1->linkedHierCount; local_20 != 0; local_20 = local_20 - 1) {
				pMVar1 = (ed_3d_hierarchy*)peVar4->pData;
				pMVar1->bRenderShadow = pMVar1->bRenderShadow | flag;
				peVar3 = peVar4->pPrev;
				for (local_30 = (uint)(ushort)pMVar1->linkedHierCount; local_30 != 0; local_30 = local_30 - 1) {
					pMVar1 = (ed_3d_hierarchy*)peVar3->pData;
					pMVar1->bRenderShadow = pMVar1->bRenderShadow | flag;
					peVar2 = peVar3->pPrev;
					for (local_40 = (uint)(ushort)pMVar1->linkedHierCount; local_40 != 0; local_40 = local_40 - 1) {
						pMVar1 = (ed_3d_hierarchy*)peVar2->pData;
						pMVar1->bRenderShadow = pMVar1->bRenderShadow | flag;
						peVar7 = peVar2->pPrev;
						for (local_50 = (uint)(ushort)pMVar1->linkedHierCount; local_50 != 0; local_50 = local_50 - 1) {
							pMVar1 = (ed_3d_hierarchy*)peVar7->pData;
							pMVar1->bRenderShadow = pMVar1->bRenderShadow | flag;
							peVar8 = peVar7->pPrev;
							for (local_60 = (uint)(ushort)pMVar1->linkedHierCount; local_60 != 0; local_60 = local_60 - 1) {
								pMVar1 = (ed_3d_hierarchy*)peVar8->pData;
								pMVar1->bRenderShadow = pMVar1->bRenderShadow | flag;
								local_80 = peVar8->pPrev;
								for (local_70 = (uint)(ushort)pMVar1->linkedHierCount; local_70 != 0; local_70 = local_70 - 1) {
									ed3DSetMeshTransformFlag_002abff0(local_80, flag);
									local_80 = local_80->pPrev;
								}
								peVar8 = peVar8->pPrev;
							}
							peVar7 = peVar7->pPrev;
						}
						peVar2 = peVar2->pPrev;
					}
					peVar3 = peVar3->pPrev;
				}
				peVar4 = peVar4->pPrev;
			}
			peVar5 = peVar5->pPrev;
		}
		peVar6 = peVar6->pPrev;
	}
	return;
}

void ed3DHierarchySetSetup(ed_3d_hierarchy* pHier, ed_3d_hierarchy_setup* pHierarchySetup)
{
	if (pHierarchySetup == (ed_3d_hierarchy_setup*)0x0) {
		pHier->pHierarchySetup = (ed_3d_hierarchy_setup*)0x0;
		pHier->flags_0x9e = pHier->flags_0x9e & 0xffef;
	}
	else {
		pHier->pHierarchySetup = pHierarchySetup;
		pHier->flags_0x9e = pHier->flags_0x9e | 0x10;
	}

	ED3D_LOG(LogLevel::Info, "ed3DHierarchySetSetup new flags: 0x{:x} pHierarchySetup: {}", pHier->flags_0x9e, (uintptr_t)pHier->pHierarchySetup);

	return;
}

edNODE* ed3DHierarchyGetChildOrParent_002ac920(edNODE* pNode)
{
	ed_3d_hierarchy* peVar1;
	edNODE* pReturnVal;
	edNODE* peVar2;

	while (peVar2 = pNode, peVar2 != (edNODE*)0x0) {
		ed_3d_hierarchy* pHier = (ed_3d_hierarchy*)peVar2->pData;
		peVar1 = pHier->pLinkTransformData;
		pNode = (edNODE*)0x0;
		pReturnVal = peVar2;
		if (peVar1 != (ed_3d_hierarchy*)0x0) {
			for (; pNode = peVar2, peVar2->pData != peVar1; peVar2 = peVar2->pNext) {
			}
		}
	}
	return pReturnVal;
}

void ed3DHierarchyJoinNodes_002ab840(edNODE* pNodeA, edNODE* pNodeB, edNODE* pNodeC)
{
	edNODE* peVar1;

	peVar1 = pNodeC->pNext;
	if (pNodeC != pNodeA) {
		pNodeA->pNext->pPrev = pNodeB->pPrev;
		pNodeB->pPrev->pNext = pNodeA->pNext;
		pNodeA->pNext = peVar1;
		pNodeB->pPrev = pNodeC;
		peVar1->pPrev = pNodeA;
		pNodeC->pNext = pNodeB;
	}
	return;
}

void ed3DHierarchyResetNode_002ac8b0(edNODE* pNodeA, edNODE* pNodeC)
{
	ed_3d_hierarchy* peVar1;
	edNODE* pNodeB;
	int iVar2;
	short sStack2;

	peVar1 = (ed_3d_hierarchy*)pNodeA->pData;
	peVar1->pLinkTransformData = (ed_3d_hierarchy*)0x0;
	pNodeB = pNodeA;
	for (iVar2 = 0; iVar2 < (int)(uint)(ushort)peVar1->linkedHierCount; iVar2 = iVar2 + 1) {
		pNodeB = pNodeB->pPrev;
	}
	ed3DHierarchyJoinNodes_002ab840(pNodeA, pNodeB, pNodeC);
	ed3DHierarchyRefreshSonNumbers(pNodeC, &sStack2);
	return;
}

void ed3DHierarchyNodeUpdateScene(edLIST* pListA, edLIST* pListB, edNODE* pNode)
{
	ed_3d_hierarchy* peVar1;
	edNODE* peVar2;
	uint uVar3;
	int iVar4;
	edNODE* pNodeB;

	peVar2 = ed3DHierarchyGetChildOrParent_002ac920(pNode);
	if (peVar2 != pNode) {
		ed3DHierarchyResetNode_002ac8b0(pNode, peVar2);
	}
	peVar1 = (ed_3d_hierarchy*)pNode->pData;
	peVar2 = pListB->pPrev;
	uVar3 = (uint)(ushort)peVar1->linkedHierCount;
	pNodeB = pNode;
	for (iVar4 = 0; iVar4 < (int)uVar3; iVar4 = iVar4 + 1) {
		pNodeB = pNodeB->pPrev;
	}
	pListA->nodeCount = pListA->nodeCount - uVar3;
	pListB->nodeCount = pListB->nodeCount + (uint)(ushort)peVar1->linkedHierCount;
	ed3DHierarchyJoinNodes_002ab840(pNode, pNodeB, peVar2);
	return;
}

void ed3DHierarchyNodeSetRenderOn(ed_3D_Scene* pScene, edNODE* pNode)
{
	ushort uVar1;

	ed_3d_hierarchy* pHier = (ed_3d_hierarchy*)pNode->pData;

	ED3D_LOG(LogLevel::Info, "ed3DHierarchyNodeSetRenderOn Hier: {} Scene: {:x}", pHier->hash.ToString(), (uintptr_t)pScene);

	uVar1 = pHier->flags_0x9e;
	if ((uVar1 & 1) == 0) {
		pHier->flags_0x9e = uVar1 | 1;
		ed3DHierarchyNodeUpdateScene(pScene->pHierListA, pScene->pHierListB, pNode);
	}
	return;
}

void ed3DHierarchyNodeSetRenderOff(ed_3D_Scene* pScene, edNODE* pNode)
{
	ushort uVar1;

	ed_3d_hierarchy* pHier = (ed_3d_hierarchy*)pNode->pData;

	ED3D_LOG(LogLevel::Info, "ed3DHierarchyNodeSetRenderOff Hier: {} Scene: {:x}", pHier->hash.ToString(), (uintptr_t)pScene);

	uVar1 = pHier->flags_0x9e;
	if ((uVar1 & 1) != 0) {
		pHier ->flags_0x9e = uVar1 & 0xfffe;
		ed3DHierarchyNodeUpdateScene(pScene->pHierListB, pScene->pHierListA, pNode);
	}
	return;
}

byte bInitializedRenderInfo_00449210 = 0;
ScratchPadRenderInfo RenderInfo_00424f80 = { 0x0, 0x0, 0x0, 0x0, 0x0, 1.0f, 0x0, 0x0 };

void ed3DLinkStripToViewport(ed_3d_strip* pStrip, edF32MATRIX4* pMatrix, ed_hash_code* pHash, edpkt_data* pPkt)
{
	edF32MATRIX4* peVar1;
	ScratchPadRenderInfo* pRVar2;
	edF32VECTOR4* peVar3;
	uint boundingSphereTestResult;
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
	float extraout_vf9w;
	edF32MATRIX4 eStack64;

	pRVar2 = gRender_info_SPR;
	if (bInitializedRenderInfo_00449210 == 0) {
		RenderInfo_00424f80.pMeshTransformMatrix = &eStack64;
		bInitializedRenderInfo_00449210 = 1;
	}
	RenderInfo_00424f80.pSharedMeshTransform = pMatrix;
	RenderInfo_00424f80.pPkt = pPkt;
	gRender_info_SPR->pSharedMeshTransform = pMatrix;
	pRVar2->pMeshTransformMatrix = RenderInfo_00424f80.pMeshTransformMatrix;
	pRVar2->boundingSphereTestResult = RenderInfo_00424f80.boundingSphereTestResult;
	pRVar2->pHierarchySetup = RenderInfo_00424f80.pHierarchySetup;
	pRVar2->flags = RenderInfo_00424f80.flags;
	pRVar2->biggerScale = RenderInfo_00424f80.biggerScale;
	pRVar2->pPkt = RenderInfo_00424f80.pPkt;
	pRVar2->pMeshTransformData = RenderInfo_00424f80.pMeshTransformData;
	edF32Matrix4MulF32Matrix4Hard(&eStack64, pMatrix, WorldToCamera_Matrix);
	gRender_info_SPR->pMeshTransformMatrix = &eStack64;

	gBoundSphereCenter->xyz = pStrip->boundingSphere.xyz;
	gBoundSphereCenter->w = 1.0f;

	*gBoundSphereCenter = (*gBoundSphereCenter) * (*gRender_info_SPR->pMeshTransformMatrix);

	boundingSphereTestResult = ed3DTestBoundingSphereObject(&pStrip->boundingSphere);
	gRender_info_SPR->boundingSphereTestResult = boundingSphereTestResult;
	if (*gShadowRenderMask == 0) {
		_ed3DLinkStripToViewport(pStrip, pHash);
	}
	else {
		_ed3DLinkStripShadowToViewport(pStrip, pHash);
	}
	return;
}

void ed3DHierarchyNodeSetSetup(edNODE* pNode, ed_3d_hierarchy_setup* pSetup)
{
	int iVar1;

	for (iVar1 = (ushort)((ed_3d_hierarchy*)pNode->pData)->linkedHierCount + 1; iVar1 != 0; iVar1 = iVar1 + -1) {
		ed3DHierarchySetSetup((ed_3d_hierarchy*)pNode->pData, pSetup);
		pNode = pNode->pPrev;
	}
	return;
}

ed_g3d_object* ed3DObjectGetFromHashCode(ed_hash_code* pMeshHeader)
{
	ed_Chunck* pObj = (ed_Chunck*)LOAD_SECTION(pMeshHeader->pData);
	return (ed_g3d_object*)(pObj + 1);
}

ed_g3d_object* ed3DHierarchyGetObject(ed_3d_hierarchy* pHier)
{
	short sVar1;
	ed3DLod* peVar2;
	ed_g3d_object* piVar3;

	piVar3 = (ed_g3d_object*)0x0;
	if (pHier->lodCount != 0) {
		peVar2 = ed3DChooseGoodLOD(pHier);
		if (peVar2 == (ed3DLod*)0x0) {
			piVar3 = (ed_g3d_object*)0x0;
		}
		else {
			sVar1 = peVar2->renderType;
			if (sVar1 == 3) {
				piVar3 = ed3DObjectGetFromHashCode((ed_hash_code*)LOAD_SECTION(peVar2->pObj));
			}
			else {
				if ((sVar1 != 2) && (sVar1 != 1)) {
					if (sVar1 == 0) {
						piVar3 = ed3DObjectGetFromHashCode((ed_hash_code*)LOAD_SECTION(peVar2->pObj));
					}
					else {
						piVar3 = (ed_g3d_object*)0x0;
					}
				}
			}
		}
	}
	return piVar3;
}

int ed3DG2DGetG2DNbMaterials(ed_Chunck* pChunck)
{
	int materialCount;
	int adjustedChunkSize;

	if ((pChunck->hash == HASH_CODE_HASH) || (materialCount = -1, pChunck->hash == HASH_CODE_MBNK /*MBNK*/)) {
		adjustedChunkSize = pChunck->size + -0x10;
		materialCount = adjustedChunkSize >> 4;
		if (adjustedChunkSize < 0) {
			materialCount = pChunck->size + -1 >> 4;
		}
	}

	return materialCount;
}

int ed3DG2DGetG2DNbMaterials(ed_hash_code* pHashCode)
{
	ed_Chunck* pChunck = reinterpret_cast<ed_Chunck*>(pHashCode - 1);
	return ed3DG2DGetG2DNbMaterials(pChunck);
}

ed_hash_code* ed3DHierarchyGetMaterialBank(ed_3d_hierarchy* pHier)
{
	ed_hash_code* peVar1;

	if (pHier->pTextureInfo == (char*)0x0) {
		peVar1 = (ed_hash_code*)0x0;
	}
	else {
		peVar1 = (ed_hash_code*)((char*)pHier->pTextureInfo + sizeof(ed_Chunck));
	}

	return peVar1;
}

int ed3DHierarchyBankMatGetSize(ed_3d_hierarchy* pHier)
{
	ed_hash_code* pHashCode;
	int iVar1;

	pHashCode = ed3DHierarchyGetMaterialBank(pHier);
	iVar1 = ed3DG2DGetG2DNbMaterials(pHashCode);
	return iVar1 * sizeof(ed_hash_code) + sizeof(ed_Chunck);
}

void* ed3DHierarchyBankMatInstanciate(ed_3d_hierarchy* pHier, void* pData)
{
	char* __src;
	ed_hash_code* pHashCode;
	int iVar1;

	__src = pHier->pTextureInfo;
	pHashCode = ed3DHierarchyGetMaterialBank(pHier);
	iVar1 = ed3DG2DGetG2DNbMaterials(pHashCode);
	memcpy(pData, __src, iVar1 * sizeof(ed_hash_code) + sizeof(ed_Chunck));
	pHier->pTextureInfo = (char*)pData;
	return pData;
}

void ed3DHierarchyBankMatLinkG2D(ed_3d_hierarchy_node* pHier, ed_g2d_manager* pTexture)
{
	ed_hash_code* peVar2;
	uint uVar3;
	ed_Chunck* puVar4;

	puVar4 = reinterpret_cast<ed_Chunck*>((pHier->base).pTextureInfo);

	if (puVar4->hash == HASH_CODE_MBNK) {
		uVar3 = puVar4->size - 0x10U >> 4;
		while (true) {
			ed_hash_code* pHashCode = reinterpret_cast<ed_hash_code*>(puVar4 + 1);

			if (uVar3 == 0) break;

			peVar2 = edHashcodeGet(puVar4->hash, pTexture->pMATA_HASH);
			uVar3 = uVar3 - 1;
			if (peVar2 != (ed_hash_code*)0x0) {
				pHashCode->pData = STORE_SECTION(peVar2);
			}
		}
	}

	return;
}

ed_hash_code* ed3DG2DGetMaterial(ed_g2d_manager* pTextureInfo, ulong hash)
{
	ed_hash_code* pcVar1;

	pcVar1 = edHashcodeGet(hash, pTextureInfo->pMATA_HASH);
	return pcVar1;
}

ed_g3d_hierarchy* ed3DG3DHierarchyGetFromHashcode(ed_g3d_manager* pG3d, ulong hash)
{
	ed_hash_code* pcVar1;
	ed_g3d_hierarchy* peVar1;

	pcVar1 = (ed_hash_code*)edHashcodeGet(hash, pG3d->HALL + 1);
	if ((pcVar1 == (ed_hash_code*)0x0) || (pcVar1->pData == 0x0)) {
		peVar1 = (ed_g3d_hierarchy*)0x0;
	}
	else {
		peVar1 = (ed_g3d_hierarchy*)((char*)LOAD_SECTION(pcVar1->pData) + 0x10);
	}

	return peVar1;
}

void ed3DG3DHierarchySetStripShadowReceiveFlag(ed_g3d_hierarchy* pHier, ushort flag)
{
	ed_g3d_hierarchy* peVar1;
	ed_Chunck* peVar2;
	uint peVar3;

	pHier->flags_0x9e = pHier->flags_0x9e | 0x200;
	if (pHier != (ed_g3d_hierarchy*)0x0) {
		if (((pHier->flags_0x9e & 2) != 0) || ((pHier->flags_0x9e & 4) != 0)) {
			pHier->flags_0x9e = pHier->flags_0x9e | 0x200;
		}
		peVar2 = (ed_Chunck*)((char*)pHier - sizeof(ed_Chunck));

		if (peVar2->hash == 0x52454948 /*HIER*/) {
			peVar3 = peVar2->hash;
			while (peVar3 == 0x52454948 /*HIER*/) {
				ed_Chunck* pNextChunk = (ed_Chunck*)(pHier->aLods);

				if (pNextChunk->hash != 0) {
					peVar1 = (ed_g3d_hierarchy*)(pNextChunk + 1);
					while (peVar1 != (ed_g3d_hierarchy*)0x0) {
						if (peVar1 == pHier) {
							ed3DG3DHierarchyNodeSetAndClrStripFlag((ed_g3d_hierarchy*)(&peVar2->hash + 4), flag, 1, 4);
							break;
						}
						if (peVar1->pLinkTransformData == 0x0) {
							peVar1 = (ed_g3d_hierarchy*)0x0;
						}
						else {
							peVar1 = (ed_g3d_hierarchy*)((ed_Chunck*)LOAD_SECTION(peVar1->pLinkTransformData) + 1);
						}
					}
				}
				peVar2 = edChunckGetNext(peVar2, (char*)0x0);
				peVar3 = peVar2->hash;
			}
			ed3DG3DHierarchyNodeSetAndClrStripFlag(pHier, flag, 1, 4);
		}
	}
	return;
}

void ed3DHierarchyRemoveNode(edLIST* pList, edNODE* pNode)
{
	edNODE_MANAGER* peVar1;
	edNODE* peVar2;
	edNODE* peVar3;

	peVar1 = (edNODE_MANAGER*)pList->pData;
	memset(pNode->pData, 0, sizeof(ed_3d_hierarchy));
	pNode->pData = (ed_3d_hierarchy*)0x0;
	g_LoadedStaticMeshCount_004493a4 = g_LoadedStaticMeshCount_004493a4 + -1;
	peVar2 = pNode->pPrev;
	peVar3 = pNode->pNext;
	peVar3->pPrev = peVar2;
	peVar2->pNext = peVar3;
	pNode->header.typeField.type = 0;
	pNode->pPrev = peVar1->pNodeHead;
	pNode->pNext = (edNODE*)0x0;
	peVar1->pNodeHead = pNode;
	peVar1->linkCount = peVar1->linkCount + -1;
	pList->nodeCount = pList->nodeCount + -1;
	return;
}

void ed3DHierarchyRemoveFromList(edLIST* pList, edNODE* pNode)
{
	ushort uVar1;
	edNODE* peVar2;
	edNODE* pSubMeshParent;
	ushort uVar3;

	ed_3d_hierarchy* pHier = (ed_3d_hierarchy*)pNode->pData;

	pSubMeshParent = pNode->pPrev;
	uVar1 = pHier->linkedHierCount;
	ed3DHierarchyRemoveNode(pList, pNode);
	for (uVar3 = 0; uVar3 < uVar1; uVar3 = uVar3 + 1) {
		peVar2 = pSubMeshParent->pPrev;
		ed3DHierarchyRemoveNode(pList, pSubMeshParent);
		pSubMeshParent = peVar2;
	}
	return;
}

void ed3DHierarchyRemoveFromScene(ed_3D_Scene* pScene, edNODE* pNode)
{
	ed_3d_hierarchy* pHier = (ed_3d_hierarchy*)pNode->pData;

	if ((pHier->flags_0x9e & 1) == 0) {
		ed3DHierarchyRemoveFromList(pScene->pHierListA, pNode);
	}
	else {
		ed3DHierarchyRemoveFromList(pScene->pHierListB, pNode);
	}
	return;
}

void ed3DScenePopCluster(ed_3D_Scene* pScene, ed_g3d_manager* pMeshInfo)
{
	edLIST* peVar1;
	edCluster* pCluster;
	edLIST* peVar4;
	edLIST* peVar5;
	edNODE_MANAGER* pNodeManager;

	peVar1 = (edLIST*)(pScene->meshClusterList).pPrev;
	peVar5 = peVar1;
	do {
		peVar4 = peVar5;
		peVar5 = peVar1;
		if (peVar5 == &pScene->meshClusterList) {
			return;
		}
		pCluster = (edCluster*)peVar5->pData;
		peVar1 = (edLIST*)peVar5->pPrev;
	} while (pCluster->pMeshInfo != pMeshInfo);

	gLoadedClusterCount = gLoadedClusterCount + -1;

	pCluster->flags = pCluster->flags & 0xfffffffe;

	if ((peVar4 == peVar5) && (peVar4 == (edLIST*)(pScene->meshClusterList).pPrev)) {
		pNodeManager = (edNODE_MANAGER*)(pScene->meshClusterList).pData;
		(pScene->meshClusterList).pPrev = (edNODE*)peVar1;
		peVar5->header.typeField.type = 0;
		peVar5->pPrev = pNodeManager->pNodeHead;
		pNodeManager->pNodeHead = (edNODE*)peVar5;
		pNodeManager->linkCount = pNodeManager->linkCount + -1;
		(pScene->meshClusterList).nodeCount = (pScene->meshClusterList).nodeCount + -1;
	}
	else {
		pNodeManager = (edNODE_MANAGER*)(pScene->meshClusterList).pData;
		peVar4->pPrev = (edNODE*)peVar1;
		peVar5->header.typeField.type = 0;
		peVar5->pPrev = pNodeManager->pNodeHead;
		pNodeManager->pNodeHead = (edNODE*)peVar5;
		pNodeManager->linkCount = pNodeManager->linkCount + -1;
		(pScene->meshClusterList).nodeCount = (pScene->meshClusterList).nodeCount + -1;
	}
	return;
}

#ifdef PLATFORM_WIN
void RemoveTexturesG3D(ed_g3d_manager* pMeshInfo)
{
	int mbnaSize;
	ed_Chunck* pChunk;
	ushort materialCount;
	ed_hash_code* pcVar4;
	ed_Chunck* pMBNK;

	mbnaSize = pMeshInfo->MBNA->size;
	/* Checks all the hashes in the mesh to make sure they match what is in the texture

		Get the end of the current section */
	for (pChunk = edChunckGetFirst(pMeshInfo->MBNA + 1, (char*)pMeshInfo->MBNA + mbnaSize); pChunk != (ed_Chunck*)0x0; pChunk = edChunckGetNext(pChunk, (char*)pMeshInfo->MBNA + mbnaSize)) {
		ED3D_LOG(LogLevel::Info, "ed3DPrepareMaterialBank Chunk: {}", pChunk->GetHeaderString());

		/* Check if read value is MBNK */
		if (pChunk->hash == HASH_CODE_MBNK) {
			ED3D_LOG(LogLevel::Info, "ed3DPrepareMaterialBank Found MBNK chunk - size: 0x{:x}", pChunk->size);

			materialCount = (pChunk->size - 0x10U) >> 4;
			ED3D_LOG(LogLevel::Info, "ed3DPrepareMaterialBank Material count: {}", materialCount);

			pMBNK = pChunk;
			while (true) {
				if (materialCount == 0) break;

				ed_hash_code* pMaterialHash = (ed_hash_code*)(pMBNK + 1);

				ED3D_LOG(LogLevel::Info, "ed3DPrepareMaterialBank Trying to find texture {}: {}", ((pChunk->size - 0x10U) >> 4) - materialCount, pMaterialHash->hash.ToString());

				ed_hash_code* pHash = LOAD_SECTION_CAST(ed_hash_code*, pMaterialHash->pData);

				materialCount = materialCount - 1;
				pMBNK = pMBNK + 1;
			}
		}
	}

	return;
}

void RemoveTexturesG2D(ed_g2d_manager* pTextureInfo)
{
	auto pHash = ed3DG2DGetMaterialFromIndex(pTextureInfo, 0);
}
#endif

void ed3DUnInstallG3D(ed_g3d_manager* pMeshInfo)
{
#ifdef  PLATFORM_WIN
	RemoveTexturesG3D(pMeshInfo);
#endif //  PLATFORM_WIN

	pMeshInfo->fileBufferStart = (GXD_FileHeader*)0x0;
	pMeshInfo->field_0x4 = (char*)0x0;
	pMeshInfo->fileLengthA = 0;
	pMeshInfo->field_0xc = 0;
	pMeshInfo->OBJA = (ed_Chunck*)0x0;
	pMeshInfo->LIA = (ed_Chunck*)0x0;
	pMeshInfo->CAMA = (ed_Chunck*)0x0;
	pMeshInfo->SPRA = (ed_Chunck*)0x0;
	pMeshInfo->HALL = (ed_Chunck*)0x0;
	pMeshInfo->CSTA = (ed_Chunck*)0x0;
	pMeshInfo->GEOM = (ed_Chunck*)0x0;
	pMeshInfo->MBNA = (ed_Chunck*)0x0;
	pMeshInfo->INFA = (ed_Chunck*)0x0;
	pMeshInfo->fileLengthB = 0x0;
	return;
}

void ed3DUnInstallG2D(ed_g2d_manager* pTextureInfo)
{
#ifdef  PLATFORM_WIN
	RemoveTexturesG2D(pTextureInfo);
#endif //  PLATFORM_WIN

	pTextureInfo->pFileBuffer = (GXD_FileHeader*)0x0;
	pTextureInfo->textureFileLengthA = 0;
	pTextureInfo->pTextureChunk = (ed_Chunck*)0x0;
	pTextureInfo->pMATA_HASH = (ed_Chunck*)0x0;
	pTextureInfo->pT2DA = (ed_Chunck*)0x0;
	pTextureInfo->pPALL = (ed_Chunck*)0x0;
	*(undefined4*)&pTextureInfo->field_0x18 = 0;
	pTextureInfo->textureFileLengthB = 0;
	return;
}

ed_3D_Scene* ed3DGetScene(int index)
{
	return g_CameraPanStaticMasterArray_00451630[index];
}
