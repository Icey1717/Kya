#include "edDlist.h"
#include "edMem.h"
#include <string.h>
#include <stdio.h>
#include <assert.h>

#ifdef PLATFORM_PS2
#include <libvu0.h>
#include <libdma.h>
#include <math.h>
#endif

#include "ed3D.h"
#include "Rendering/DisplayList.h"
#include "LargeObject.h"
#include "edVideo/VideoA.h"
#include "edVideo/VideoB.h"
#include "edVideo/Viewport.h"
#include "EdenLib/edSys/sources/EdSystem.h"
#include "edVideo/VideoD.h"
#include "port/pointer_conv.h"

#include "Rendering/CustomShell.h"
#include "MathOps.h"
#include "edList.h"

#ifdef PLATFORM_WIN
#include "Texture.h"
#include "displaylist.h"
#endif

#define EDDLIST_LOG(level, format, ...) MY_LOG_CATEGORY("edDList", level, format, ##__VA_ARGS__)

edLIST* gDList_2D[2] = { 0 };
edLIST* gDList_2D_Before3D[2] = { 0 };
DisplayList** gDList_3D[2] = { 0 };
int gNbDList_3D[2] = { 0 };

char* gDLISTWorkingBufferStart;
char* gDLISTWorkingBufferCur;
char* gDLISTWorkingBufferEnd;

uint gBlendMode = 0;

int gCurColorNbInVertex = 0;
int gEndColorNbInVertex = 0;

int gCurStripPatchable = 0;
int gCurSpritePatchable = 0;

char* gEndVertexBuf = NULL;

int gNbMatrix;
edF32MATRIX4* gCurMatrix;

uint gCurInfPosForBegin;

int gIncViewportX = 0;
int gIncViewportY = 0;
ed_viewport* gCurViewport = NULL;

ed_hash_code* gBankMaterial;

// Header packet of the draw list. Set in edDListBegin and updated with the correct number of prims in edDListEnd.
edpkt_data* gCurStatePKT = NULL;

int gNbUsedMaterial;

bool gbInsideBegin = false;

int gCurRenderState = 0;
int gNbStateAdded = 0;

int gCurStatePKTSize = 0;
int gSubdivideLevel = 0;

// Possibly a debug switch
int gNbMatrixInArray = 0;

edF32MATRIX4* gCurMatrixArray = NULL;

byte gbLight = 0;

short gShadowCast = 0;
short gShadowReceive = 0;

int gCurPrimType = 0;
int gNbAddedVertex = 0;
int gMaxNbVertex = 0;
int gNbDMAVertex = 0;

typedef void (*DisplayListXYZFunc)(float, float, float, uint);
typedef void (*DisplayListRGBAQFunc)(byte, byte, byte, byte);
typedef void (*DisplayListSTFunc)(float, float);

DisplayListXYZFunc gAddVertexFUNC = NULL;
DisplayListRGBAQFunc gAddColorFUNC = NULL;
DisplayListSTFunc gAddSTFUNC = NULL;

_rgba* gCurColorBuf = NULL;
short* gCurSTBuf = NULL;
char* gEndSTBuf = NULL;
edVertex* gCurVertexBuf = NULL;
undefined* gStartVertexBuf = NULL;

int gCurFlashMaterial = 0;
edDList_material* gCurMaterial = NULL;
DisplayList* gCurDList = NULL;

edDlistConfiguration edDlistConfig = { 0x5, 0x5, 0xA, 0x3E80, 0x12C };

byte DAT_00449660 = 0;
	
bool gbForceMaterialUse = false;
int g_Count_004495f8 = 0;

int edDlistUseUV = 0;

_rgba ByteColor_ARRAY_0048dd40[4];

_rgba* g_RGBAQ_00448aa0 = ByteColor_ARRAY_0048dd40;

edSysHandler3D ed3DHandlers;

void edDListInitMemory(void)
{
	uint size;
	if ((edDlistConfig.commandCount & 0xfU) != 0) {
		edDlistConfig.commandCount = edDlistConfig.commandCount + (0x10 - (edDlistConfig.commandCount & 0xfU));
	}
	if ((edDlistConfig.field_0x0 & 3U) != 0) {
		edDlistConfig.field_0x0 = edDlistConfig.field_0x0 + (4 - (edDlistConfig.field_0x0 & 3U));
	}
	size = edDlistConfig.commandCount + 0x640 + edDlistConfig.matrixCount * sizeof(edF32MATRIX4) + edDlistConfig.field_0x0 * 8 +
		edDlistConfig.bankMaterialCount * sizeof(DisplayList_0x10);
	if ((size & 0xf) != 0) {
		size = size + (0x10 - (size & 0xf));
	}

	// #HACK
	size = size * 4;

	gDLISTWorkingBufferStart = (char*)edMemAlloc(TO_HEAP(H_MAIN), size);
	memset(gDLISTWorkingBufferStart, 0, size);
	gDLISTWorkingBufferCur = gDLISTWorkingBufferStart;
	gDLISTWorkingBufferEnd = gDLISTWorkingBufferStart + size;
	return;
}

void* edDListAllocMemory(int count)
{
	char* puVar1;
	char* pcVar1;

	pcVar1 = gDLISTWorkingBufferCur + count * sizeof(DisplayList_0x10);
	puVar1 = (char*)0x0;
	if (pcVar1 <= gDLISTWorkingBufferEnd) {
		puVar1 = pcVar1 + count * -0x10;
		gDLISTWorkingBufferCur = pcVar1;
	}
	return (void*)puVar1;
}

int gCurFlushState = 0;
int gbDispList = 0;
edpkt_data* gPKTDListFlush2DStart = NULL;

edpkt_data* edDListRecurseRefTag(DisplayList* pDisplayList, edpkt_data* pCommandBuffer)
{
	uint uVar1;
	ulonglong uVar2;
	RenderCommandUint* pRVar3;
	uint uVar4;

	uVar4 = 0;
LAB_002ce9d4:
	do {
		if (pDisplayList->nbCommands <= uVar4) {
			return pCommandBuffer;
		}
		pRVar3 = pDisplayList->aCommands->aCommandArray + uVar4;
		uVar1 = pRVar3->type;
		if (uVar1 == LM_RELINK) {
			pCommandBuffer = edDListRecurseRefTag(pRVar3->pDisplayList, pCommandBuffer);
		}
		else {
			if (uVar1 == LM_CALL) {
				pCommandBuffer->cmdB = 0x0;
				sceDmaAddCall((sceDmaTag**)&pCommandBuffer, 0, (void*)pRVar3->pCommandBuffer);
			}
			else {
				if (uVar1 == LM_FUNC) {
					IMPLEMENTATION_GUARD_LOG(
					pCommandBuffer = edDList2DUseFrameMaterial(pCommandBuffer);
					)
				}
				else {
					if (uVar1 == LM_REF_0) {

						uVar2 = (ulong)(pRVar3->size >> 4);
						if ((pRVar3->size != 0) && (uVar2 != 0)) {
							pCommandBuffer->cmdB = 0x0;
							sceDmaAddRef((sceDmaTag**)&pCommandBuffer, uVar2, (void*)pRVar3->pCommandBuffer);
						}
					}
					else {
						if (uVar1 != 5) {
							uVar4 = uVar4 + 1;
							goto LAB_002ce9d4;
						}
						uVar2 = (ulong)(pRVar3->size >> 4);
						if (pRVar3->size != 0) {
							if (uVar2 != 0) {
								pCommandBuffer->cmdB = 0x0;
								sceDmaAddRef((sceDmaTag**)&pCommandBuffer, uVar2, (void*)pRVar3->pCommandBuffer);
							}
							pCommandBuffer->cmdA = 0xe10000002;
							pCommandBuffer->cmdB = 0;

							pCommandBuffer[1].cmdB = SCE_GIF_PACKED_AD;
							pCommandBuffer[1].cmdA = SCE_GIF_SET_TAG(
								1,					// NLOOP
								SCE_GS_TRUE,		// EOP
								SCE_GS_FALSE,		// PRE
								0,					// PRIM
								SCE_GIF_PACKED,		// FLG
								1					// NREG
							);
							pCommandBuffer[2].cmdA = SCE_GS_SET_TEST(
								1,	// ATE 
								7,	// ATST
								0,	// AREF
								0,	// AFAIL
								0,	// DATE
								0,	// DATM
								1,	// ZTE
								1	// ZTST
							);
							pCommandBuffer[2].cmdB = SCE_GS_TEST_1;

							pCommandBuffer = pCommandBuffer + 3;
						}
					}
				}
			}
		}
		uVar4 = uVar4 + 1;
	} while (true);
}

void edDListSend2DList(edLIST* pList)
{
	DisplayList* pDisplayList;
	DisplayListCommand* pDVar1;
	edpkt_data* pCommandBuffer;
	RenderCommandUint* pRVar2;
	edpkt_data* pBuffer;
	edNODE* pCVar7;

	pBuffer = gPKTDListFlush2DStart + (gCurFlushState * ((uint)edDlistConfig.commandCount >> 4) >> 1);
	pCommandBuffer = pBuffer;
	for (pCVar7 = pList->pPrev; (edLIST*)pCVar7 != pList; pCVar7 = pCVar7->pPrev) {
		pDisplayList = (DisplayList*)pCVar7->pData;
		if ((gbDispList != 0) && (pDisplayList->pRenderCommands != pDisplayList->field_0x14)) {
			pRVar2 = pDisplayList->aCommands->aCommandArray + (pDisplayList->nbCommands - 1);
			if (pRVar2->type == LM_REF_0) {
				pRVar2->size = (uint)((ulong)pDisplayList->pRenderCommands - (ulong)pRVar2->pCommandBuffer);
			}
			pCommandBuffer = edDListRecurseRefTag(pDisplayList, pCommandBuffer);
		}
		if ((pDisplayList->flags_0x0 & 0x10) != 0) {
			pDVar1 = pDisplayList->aCommands;
			pDisplayList->pRenderCommands = pDisplayList->field_0x14;
			pDVar1->aCommandArray[0].pCommandBuffer = pDisplayList->field_0x14;
			pDVar1->aCommandArray[0].type = LM_REF_0;
			pDisplayList->nbCommands = 0;
		}
		pDisplayList->field_0x3 = 0;
	}
	if ((pBuffer != pCommandBuffer) && (gbDispList != 0)) {
		pCommandBuffer->cmdA = 0x70000000;
		pCommandBuffer->cmdB = 0;
		edDmaFlushCache();
		edDmaSyncPath();
		edDmaSyncPath();
		static bool bDisp2D = 1;
		if (bDisp2D != false) {
			EDDLIST_LOG(LogLevel::VeryVerbose, "DMA Begin RenderUI_002cea00\n");
			edDmaSend_nowait(SHELLDMA_CHANNEL_GIF, (ulonglong*)pBuffer);
		}
		edDmaSyncPath();
	}
	edListClear(pList);
	return;
}

struct ed_3d_extra_stuff_param {
	ed_3D_Scene* pScene;
	int isChild;
	ushort taskID;
};

int gNbToPix = 0;

void edDListSend3DList(ed_3d_extra_stuff_param* pParams)
{
	ushort dataType;
	bool bShouldLinkStrip;
	uint nbMatrix;
	edpkt_data* pPktToLink;
	DisplayListCommand* pCurCommand;
	uint displayListIndex;
	uint curCommandIndex;
	uint curMatrixIndex;
	edF32MATRIX4* pMatrix;
	edpkt_data* pPkt;
	DisplayList* pDisplayList;

	EDDLIST_LOG(LogLevel::Info, "edDListSend3DList gCurRenderState: {} NbList: {}", gCurRenderState, gNbDList_3D[gCurRenderState]);

	gNbToPix = 0;

	if (gbDispList != 0) {
		for (displayListIndex = 0; displayListIndex < gNbDList_3D[gCurRenderState]; displayListIndex = displayListIndex + 1) {
			pDisplayList = gDList_3D[gCurRenderState][(gNbDList_3D[gCurRenderState] - 1) - displayListIndex];

			if (((BYTE_00448a5c == 0) ||
				(((pParams->taskID & 1) != 0 && ((pDisplayList->flags_0x0 & 0x40) != 0)))) ||
				(((pParams->taskID & 2) != 0 && ((pDisplayList->flags_0x0 & 0x40) == 0)))) {
				pPkt = (edpkt_data*)0x0;

				if ((pDisplayList->pScene == DISPLAY_LIST_SCENE_ALWAYS) || (pDisplayList->pScene == pParams->pScene)) {
					for (curCommandIndex = 0; curCommandIndex < pDisplayList->nbCommands; curCommandIndex = curCommandIndex + 1) {
						bShouldLinkStrip = true;
						curMatrixIndex = 0;
						pCurCommand = pDisplayList->aCommands + curCommandIndex;

						// Run through the command for each of the matrices provided (basically instanced drawing).
						pMatrix = &pCurCommand->matrix;
						pPktToLink = pPkt;
						do {
							if (pCurCommand->nbMatrix != 0) {
								pMatrix = pCurCommand->pCurMatrixArray + curMatrixIndex;
							}

							dataType = pCurCommand->dataType;
							if (dataType == DISPLAY_LIST_DATA_TYPE_PKT) {
								pPkt = pCurCommand->pRenderInput.pPkt;
							LAB_002ce7c4:
								nbMatrix = pCurCommand->nbMatrix;
							}
							else {
								if (dataType == DISPLAY_LIST_DATA_TYPE_SPRITE) {
									if ((pParams->isChild != 1) && (pCurCommand->bActive != 0)) {
										IMPLEMENTATION_GUARD(
										ed3DLinkSpriteToViewport(pCurCommand->pRenderInput.pSprite, pMatrix, gBankMaterial, pPktToLink);
										pPktToLink = (edF32MATRIX4*)0x0;)
									}

									goto LAB_002ce7c4;
								}

								if (((dataType == DISPLAY_LIST_DATA_TYPE_TRIANGLE_LIST) || (dataType == 2)) || (dataType == 0)) {
									if ((pDisplayList->pScene != DISPLAY_LIST_SCENE_ALWAYS) &&
										(((pParams->isChild == 1 && (pCurCommand->pRenderInput.pStrip->shadowReceiveFlags == 0)) ||
											((pParams->isChild != 1 && (pCurCommand->pRenderInput.pStrip->shadowReceiveFlags != 0))))))
									{
										bShouldLinkStrip = false;
									}

									if ((bShouldLinkStrip) && (pCurCommand->bActive != 0)) {
										ed3DLinkStripToViewport(pCurCommand->pRenderInput.pStrip, pMatrix, gBankMaterial, pPktToLink);
										pPktToLink = (edpkt_data*)0x0;
									}

									goto LAB_002ce7c4;
								}

								nbMatrix = pCurCommand->nbMatrix;
							}

							curMatrixIndex = curMatrixIndex + 1;
						} while (curMatrixIndex < nbMatrix);
					}
				}
			}
		}
	}
#ifdef PLATFORM_PS2
	iFlushCache(0);
#endif
	gCurDList = (DisplayList*)0x0;

	return;
}

void edDListSwapList(void)
{
	DisplayList* pDVar1;
	uint displayListIndex;
	DisplayList** pDisplayListArray;

	displayListIndex = 0;
	pDisplayListArray = gDList_3D[gCurFlushState];
	while (true) {
		if (gNbDList_3D[gCurFlushState] <= displayListIndex) break;
		pDVar1 = pDisplayListArray[displayListIndex];
		if (((pDVar1->flags_0x0 & 4) == 0) && ((pDVar1->flags_0x0 & 0x100) == 0)) {
			pDVar1->nbCommands = 0;
			pDVar1->pRenderCommands = (edpkt_data*)pDVar1->pCommandBuffer;
			pDVar1->field_0x10 = (edpkt_data*)pDVar1->pCommandBuffer;
		}
		pDVar1->field_0x3 = 0;
		displayListIndex = displayListIndex + 1;
		pDVar1->field_0x24 = 100.0f;
	}
	gNbDList_3D[gCurFlushState] = 0;
	gCurFlushState = gCurRenderState;
	gCurRenderState = gCurRenderState + 1;
	if (gCurRenderState == 2) {
		gCurRenderState = 0;
	}
	gbDispList = 1;
	gCurDList = (DisplayList*)0x0;
	return;
}

void edDListInterrupt(int callingID, int eventID, char* pRenderTaskData)
{
	if (callingID == ED_SYSTEM_HANDLER_3D) {
		if (eventID == ED_HANDLER_3D_SEND_2D) {
			edDListSend2DList(gDList_2D_Before3D[gCurFlushState]);
		}
		else {
			if (eventID == ED_HANDLER_3D_SEND_3D) {
				edDListSend3DList((ed_3d_extra_stuff_param*)pRenderTaskData);
			}
		}
	}
	else {
		if (callingID == ED_SYSTEM_HANDLER_VIDEO) {
			if (eventID == 10) {
				edDListSend2DList(gDList_2D[gCurRenderState]);
				DAT_00449660 = 1;
			}
			else {
				if ((eventID != 2) && (eventID == ED_HANDLER_VIDEO_SWAP)) {
					edDListSwapList();
					DAT_00449660 = 0;
				}
			}
		}
	}
	return;
}

int edDlistTestFunc(void* param_1, void* param_2)
{
	return (uint)((DisplayList*)param_1)->field_0x2 - (uint)((DisplayList*)param_2)->field_0x2;
}

int gbInitDone = 0;
int gEndSTNbInVertex = 0;
int gCurSTNbInVertex = 0;
int gMaxSTNbInVertex = 0;
char* gEndColorBuf = NULL;

edpkt_data gPKTStrip[0xa];

#define PKT_STRIP_ADDR_MARKER 0x0fddffdd

void edDListInitStripPKT(void)
{
	gPKTStrip[0].asU32[0] = ED_VIF1_SET_TAG_REF(1, 0);
	gPKTStrip[0].asU32[1] = STORE_SECTION(&g_stVertexGIFHeader);
	gPKTStrip[0].cmdB = 0;

	gPKTStrip[1].asU32[0] = ED_VIF1_SET_TAG_REF(1, 0);
	gPKTStrip[1].asU32[1] = STORE_SECTION(&g_stGifTAG_Texture_NoFog[72]);
	gPKTStrip[1].asU32[2] = SCE_VIF1_SET_NOP(0);
	gPKTStrip[1].asU32[3] = SCE_VIF1_SET_UNPACK(0x8000, 0x01, UNPACK_V4_32, 0);

	gPKTStrip[2].asU32[0] = ED_VIF1_SET_TAG_REF(2, 0);
	gPKTStrip[2].asU32[1] = STORE_SECTION(&g_stVertexSTHeader);
	gPKTStrip[2].cmdB = 0;

	gPKTStrip[3].asU32[0] = ED_VIF1_SET_TAG_REF(0x12, 0);
	gPKTStrip[3].asU32[1] = PKT_STRIP_ADDR_MARKER;
	gPKTStrip[3].asU32[2] = SCE_VIF1_SET_NOP(0);
	gPKTStrip[3].asU32[3] = SCE_VIF1_SET_UNPACK(0x8001, 0x48, UNPACK_V2_16_MASKED, 0);

	gPKTStrip[4].asU32[0] = ED_VIF1_SET_TAG_REF(1, 0);
	gPKTStrip[4].asU32[1] = STORE_SECTION(&g_stVertexRGBAHeader);
	gPKTStrip[4].cmdB = 0;

	gPKTStrip[5].asU32[0] = ED_VIF1_SET_TAG_REF(0x12, 0);
	gPKTStrip[5].asU32[1] = PKT_STRIP_ADDR_MARKER;
	gPKTStrip[5].asU32[2] = SCE_VIF1_SET_NOP(0);
	gPKTStrip[5].asU32[3] = SCE_VIF1_SET_UNPACK(0xc002, 0x48, UNPACK_V4_8_MASKED, 0);

	gPKTStrip[6].asU32[0] = ED_VIF1_SET_TAG_REF(1, 0);
	gPKTStrip[6].asU32[1] = STORE_SECTION(&g_stVertexXYZHeader);
	gPKTStrip[6].cmdB = 0;

	gPKTStrip[7].asU32[0] = ED_VIF1_SET_TAG_REF(0x48, 0);
	gPKTStrip[7].asU32[1] = PKT_STRIP_ADDR_MARKER;
	gPKTStrip[7].asU32[2] = SCE_VIF1_SET_NOP(0);
	gPKTStrip[7].asU32[3] = SCE_VIF1_SET_UNPACK(0x8003, 0x48, UNPACK_V4_32_MASKED, 0);

	gPKTStrip[8].asU32[0] = ED_VIF1_SET_TAG_REF(1, 0);
	gPKTStrip[8].asU32[1] = STORE_SECTION(&g_stExecuteCode);
	gPKTStrip[8].asU32[2] = SCE_VIF1_SET_NOP(0);
	gPKTStrip[8].asU32[3] = SCE_VIF1_SET_MSCAL(0x0003, 0);

	gPKTStrip[9].asU32[0] = SCE_VIF1_SET_UNPACK(0, 0, UNPACK_S32, 0); // Might be incorrect?
	gPKTStrip[9].asU32[1] = SCE_VIF1_SET_NOP(0);
	gPKTStrip[9].cmdB = 0;
	return;
}

ed_hash_code* gFrameBufHASHPtr;
ed_g2d_material* gFrameBufMaterialPtr;


struct
{
	// Header
	ushort field_0x0;
	ushort field_0x2;
	int field_0x4;
	int field_0x8;
	Hash_4 hash;

	ed_Chunck TWOD;
	ed_Chunck MATA;
	ed_Chunck HASH;

	ed_hash_code hashCode;

	ed_Chunck MAT;

	ed_g2d_material material;

	int aLayers[4];

	ed_Chunck LAYA;
	ed_Chunck LAY;

	ed_g2d_layer layer;

} gFrameBufG2D;

void edDlistFrameBufMaterialInit(void)
{
	// Construct a whole .G2D file from scratch.

	gFrameBufG2D.field_0x0 = 1;
	gFrameBufG2D.field_0x2 = 0;
	gFrameBufG2D.field_0x4 = 5;
	gFrameBufG2D.field_0x8 = 0x10;
	gFrameBufG2D.hash.number = 0x4432472e; // .G2D

	gFrameBufG2D.TWOD.hash = 0x2a44322a; // *2D*
	gFrameBufG2D.TWOD.field_0x4 = 1;
	gFrameBufG2D.TWOD.field_0x6 = 0;
	gFrameBufG2D.TWOD.size = 0xd0;
	gFrameBufG2D.TWOD.nextChunckOffset = 0x10;

	gFrameBufG2D.MATA.hash = 0x4154414d; // MATA
	gFrameBufG2D.MATA.field_0x4 = 1;
	gFrameBufG2D.MATA.field_0x6 = 0;
	gFrameBufG2D.MATA.size = 0x60;
	gFrameBufG2D.MATA.nextChunckOffset = 0x10;

	gFrameBufG2D.HASH.hash = 0x48534148; // HASH
	gFrameBufG2D.HASH.field_0x4 = 1;
	gFrameBufG2D.HASH.field_0x6 = 0;
	gFrameBufG2D.HASH.size = 0x20;
	gFrameBufG2D.HASH.nextChunckOffset = 0x20;

	gFrameBufHASHPtr = &gFrameBufG2D.hashCode;
	gFrameBufG2D.hashCode.hash = ed3DComputeHashCode("FRAMEBUF");
	gFrameBufMaterialPtr = &gFrameBufG2D.material;
	gFrameBufG2D.hashCode.pData = STORE_SECTION(&gFrameBufG2D.MAT);

	gFrameBufG2D.MAT.hash = 0x2e54414d; // MAT.
	gFrameBufG2D.MAT.field_0x4 = 1;
	gFrameBufG2D.MAT.field_0x6 = 0;
	gFrameBufG2D.MAT.size = 0x20;
	gFrameBufG2D.MAT.nextChunckOffset = 0x20;

	gFrameBufG2D.material.nbLayers = 1;
	gFrameBufG2D.material.field_0x1 = 0;
	gFrameBufG2D.material.flags = 4;
	gFrameBufG2D.material.pDMA_Material = 0x0;
	gFrameBufG2D.material.pCommandBufferTexture = 0x0;
	gFrameBufG2D.material.commandBufferTextureSize = 0;

	gFrameBufG2D.aLayers[0] = STORE_SECTION(&gFrameBufG2D.LAY);

	gFrameBufG2D.LAYA.hash = 0x4159414c; // LAYA
	gFrameBufG2D.LAYA.field_0x4 = 1;
	gFrameBufG2D.LAYA.field_0x6 = 0;
	gFrameBufG2D.LAYA.size = 0x50;
	gFrameBufG2D.LAYA.nextChunckOffset = 0x10;

	gFrameBufG2D.LAY.hash = 0x2e59414c; // LAY.
	gFrameBufG2D.LAY.field_0x4 = 1;
	gFrameBufG2D.LAY.field_0x6 = 1;
	gFrameBufG2D.LAY.size = 0x40;
	gFrameBufG2D.LAY.nextChunckOffset = 0x40;

	gFrameBufG2D.layer.flags_0x0 = 0;
	gFrameBufG2D.layer.flags_0x4 = 0x800;
	gFrameBufG2D.layer.field_0x1b = 0xff;
	gFrameBufG2D.layer.bHasTexture = 0;
	gFrameBufG2D.layer.paletteId = 0;

	return;
}

ed_3d_hierarchy gCurUnitHierachy;

void edDListInit(void)
{
	DisplayList** pDVar1;
	edLIST* pCVar2;
	uint uVar3;
		
	gbInitDone = 0;
	//gPKTBlit = (undefined*)0x0;
	//gCurPKTBlit = 0;
	gDLISTWorkingBufferStart = (char*)0x0;
	gDLISTWorkingBufferCur = (char*)0x0;
	gDLISTWorkingBufferEnd = (char*)0x0;
	gPKTDListFlush2DStart = (edpkt_data*)0x0;
	gNbStateAdded = 0;
	gCurDList = (DisplayList*)0x0;
	gNbMatrix = 0;
	gIncViewportX = 0;
	gIncViewportY = 0;
	gCurMatrix = (edF32MATRIX4*)0x0;
	gCurViewport = (ed_viewport*)0x0;
	gNbAddedVertex = 0;
	gCurDListBuf = (ed_3d_strip*)0x0;
	gCurDListBufEnd = (ed_3d_strip*)0x0;
	gCurVertexBuf = (edVertex*)0x0;
	gCurColorBuf = (_rgba*)0x0;
	gCurSTBuf = (short*)0x0;
	gCurColorNbInVertex = 0;
	gEndColorNbInVertex = 0;
	gCurSTNbInVertex = 0;
	gMaxSTNbInVertex = 0;
	gEndSTNbInVertex = 0;
	gEndVertexBuf = (char*)0x0;
	gEndColorBuf = (char*)0x0;
	gEndSTBuf = (char*)0x0;
	gMaxNbVertex = 0;
	gNbDMAVertex = 0;
	gCurMaterial = (edDList_material*)0x0;
	gCurStatePKT = (edpkt_data*)0x0;
	gCurStatePKTSize = 0;
	gBlendMode = 0;
	//gbFlipDone = 0;
	gCurRenderState = 0;
	gCurFlushState = 0;
	gCurPrimType = 0;
	gNbUsedMaterial = 0;
	//gbFog = 0;
	//gbFogAlpha = 0;
	gbLight = 0;
	//gShapeSize = 0;
	gSubdivideLevel = 0;
	//gSubdivideFovCoef = 0;
	gAddVertexFUNC = (DisplayListXYZFunc)0x0;
	gAddColorFUNC = (DisplayListRGBAQFunc)0x0;
	gAddSTFUNC = (DisplayListSTFunc)0x0;
	gShadowCast = 0;
	gShadowReceive = 0;
	gNbDList_3D[0] = 0;
	gNbDList_3D[1] = 0;

	memset(&gCurUnitHierachy, 0, sizeof(ed_3d_hierarchy));
	edDListInitMemory();
	gBankMaterial = (ed_hash_code*)edDListAllocMemory(edDlistConfig.bankMaterialCount & 0xfffffff);
	//gPKTBlit = edDListAllocMemory(100);
	gPKTDListFlush2DStart = (edpkt_data*)edDListAllocMemory((uint)edDlistConfig.commandCount >> 4);
	uVar3 = 0;
	do {
		pDVar1 = (DisplayList**)edDListAllocMemory((uint)(edDlistConfig.field_0x0 << 2) >> 4);
		gDList_3D[uVar3] = pDVar1;
		pCVar2 = edListNew(1, edDlistConfig.field_0x4, TO_HEAP(H_MAIN));
		gDList_2D[uVar3] = pCVar2;
		pCVar2 = edListNew(1, edDlistConfig.field_0x4, TO_HEAP(H_MAIN));
		gDList_2D_Before3D[uVar3] = pCVar2;
		edListSetMode(gDList_2D[uVar3], 2, edDlistTestFunc);
		edListSetMode(gDList_2D_Before3D[uVar3], 2, edDlistTestFunc);

#ifdef EDITOR_BUILD
		char name[128];
		sprintf(name, "gDList_2D[%d]", uVar3);
		edListSetName(gDList_2D[uVar3], name);
		sprintf(name, "gDList_2D_Before3D[%d]", uVar3);
		edListSetName(gDList_2D_Before3D[uVar3], name);
#endif

		uVar3 = uVar3 + 1;
	} while (uVar3 < 2);
	gCurMatrix = (edF32MATRIX4*)edDListAllocMemory((uint)(edDlistConfig.matrixCount * sizeof(edF32MATRIX4)) >> 4);
	gCurRenderState = 1;
	gNbMatrix = 0;
	gCurFlushState = 0;
	edSysHandlersAdd(ed3DHandlers.nodeParent, ed3DHandlers.entries, ed3DHandlers.maxEventID, ED_HANDLER_3D_SEND_3D, edDListInterrupt, 1, 0);
	edSysHandlersAdd(ed3DHandlers.nodeParent, ed3DHandlers.entries, ed3DHandlers.maxEventID, ED_HANDLER_3D_SEND_2D, edDListInterrupt, 1, 0);

	edSysHandlersAdd(edVideoHandlers.nodeParent, edVideoHandlers.entries, edVideoHandlers.maxEventID, 2, edDListInterrupt, 1, 0);
	edSysHandlersAdd(edVideoHandlers.nodeParent, edVideoHandlers.entries, edVideoHandlers.maxEventID, ED_HANDLER_VIDEO_SWAP, edDListInterrupt, 1, 0);
	edSysHandlersAdd(edVideoHandlers.nodeParent, edVideoHandlers.entries, edVideoHandlers.maxEventID, ED_HANDLER_VIDEO_SEND_2D, edDListInterrupt, 1, 0);
	gbInitDone = 1;
	//edDlistDebugInit();
	edDListInitStripPKT();
	edDlistFrameBufMaterialInit();
	return;
}

void edDListLoadMatrix(edF32MATRIX4* m0)
{
	edF32Matrix4CopyHard(gCurMatrix + gNbMatrix, m0);
	return;
}

void edDListPatchGifTag2D(void)
{
	edpkt_data* pRVar1;

	if (gNbStateAdded != 0) {
		pRVar1 = gCurDList->pRenderCommands;
 		pRVar1[-(gNbStateAdded + 1)].cmdA = SCE_GIF_SET_TAG(
			gNbStateAdded,	// NLOOP
			SCE_GS_TRUE,						// EOP
			SCE_GS_FALSE,						// PRE
			0,									// PRIM
			SCE_GIF_PACKED,						// FLG
			1									// NREG
		);

		pRVar1[-(gNbStateAdded + 1)].cmdB = SCE_GIF_PACKED_AD;
		gNbStateAdded = 0;
	}
	return;
}

void ApplyFlag_0029f1e0(ed_g2d_material* pMAT_Internal, uint index, uint flag)
{
	if (index < pMAT_Internal->nbLayers) {
		int* hash = reinterpret_cast<int*>(pMAT_Internal + 1);
		ed_Chunck* pLAY = LOAD_SECTION_CAST(ed_Chunck*, hash[index]);

		ed_g2d_layer* pLayer = reinterpret_cast<ed_g2d_layer*>(pLAY + 1);
		pLayer->flags_0x0 |= flag;
	}
	return;
}

void edDListBlendSet(uint mode)
{
	if ((((gbInsideBegin == false) && (gBlendMode = mode & 0xff, gCurMaterial != (edDList_material*)0x0)) &&
		(gCurMaterial->pMaterial != (ed_g2d_material*)0x0)) && (gBlendMode == 1)) {
		ApplyFlag_0029f1e0(gCurMaterial->pMaterial, 0, 4);
	}
	return;
}

void edDListBlendFunc50(void)
{
	edDListBlendSet(1);
	edDListSetState(0x4000000064, 0x42);
	return;
}

ed_g2d_bitmap* edDListGetG2DBitmap(ed_g2d_material* pMaterial, int offset, bool* bHasPalette, ed_g2d_bitmap** ppBitmap)
{
	ed_hash_code* pTextureHashCode;
	ed_g2d_bitmap* pBitmap;
	ed_Chunck* pTEX;
	ed_Chunck* pLAY;
	ed_hash_code* pPaletteHashCode;

	*bHasPalette = false;
	*ppBitmap = (ed_g2d_bitmap*)0x0;

	if (pMaterial != (ed_g2d_material*)0x0) {
		if (pMaterial->nbLayers == 0) {
			return (ed_g2d_bitmap*)0x0;
		}

		pLAY = LOAD_SECTION_CAST(ed_Chunck*, pMaterial->aLayers[offset]);

		ed_g2d_layer* pLayer = reinterpret_cast<ed_g2d_layer*>(pLAY + 1);

		if (((pLAY != (ed_Chunck*)0xfffffff0) && (pLAY != (ed_Chunck*)0xfffffff0)) && (pLayer->bHasTexture != 0)) {

			pTEX = LOAD_SECTION_CAST(ed_Chunck*, pLayer->pTex);
			ed_g2d_texture* pTexture = reinterpret_cast<ed_g2d_texture*>(pTEX + 1);

			pBitmap = (ed_g2d_bitmap *)0x0;

			if (pTEX == (ed_Chunck*)0xfffffff0) {
				return (ed_g2d_bitmap*)0x0;
			}

			if (pTexture->bHasPalette != 0) {
				*bHasPalette = true;

				ed_hash_code* pHashCode = reinterpret_cast<ed_hash_code*>(pTexture + 1);
				pPaletteHashCode = LOAD_SECTION_CAST(ed_hash_code*, pHashCode[pLayer->paletteId].pData);

				if (pPaletteHashCode != (ed_hash_code*)0x0) {
					ed_Chunck* pT2D = LOAD_SECTION_CAST(ed_Chunck*, pPaletteHashCode->pData);
					pBitmap = reinterpret_cast<ed_g2d_bitmap*>(pT2D + 1);
				}

				pTextureHashCode = LOAD_SECTION_CAST(ed_hash_code*, pTexture->hashCode.pData);
				if (pTextureHashCode == 0) {
					return pBitmap;
				}

				ed_Chunck* pT2D = LOAD_SECTION_CAST(ed_Chunck*, pTextureHashCode->pData);
				*ppBitmap = reinterpret_cast<ed_g2d_bitmap*>(pT2D + 1);
				return pBitmap;
			}

			pTextureHashCode = LOAD_SECTION_CAST(ed_hash_code*, pTexture->hashCode.pData);
			if (pTextureHashCode == 0) {
				return (ed_g2d_bitmap*)0x0;
			}

			ed_Chunck* pT2D = LOAD_SECTION_CAST(ed_Chunck*, pTextureHashCode->pData);
			return reinterpret_cast<ed_g2d_bitmap*>(pT2D + 1);
		}
	}

	return (ed_g2d_bitmap*)0x0;
}

void edDListSetState(ulong cmdA, ulong cmdB)
{
	ushort uVar1;
	DisplayListCommand* pDVar2;
	int iVar3;
	DisplayList* pDVar4;
	int iVar5;
	edpkt_data* pRVar6;

	pDVar4 = gCurDList;
	iVar3 = gNbStateAdded;
	gCurDList = pDVar4;
	if (gbInsideBegin == false) {
		if ((gCurDList->flags_0x0 & 0x100) == 0) {
			pRVar6 = gCurDList->pRenderCommands;

			// Begin the giftag header if we have no commands already.
			if (gNbStateAdded == 0) {
				if ((gCurDList->flags_0x0 & 1) != 0) {
					uVar1 = gCurDList->nbCommands;
					pDVar2 = gCurDList->aCommands;
					pDVar2[uVar1].dataType = DISPLAY_LIST_DATA_TYPE_PKT;
					pDVar2[uVar1].pRenderInput.pPkt = pRVar6;
				}

				pRVar6->cmdA = SCE_GIF_SET_TAG(
					1,				// NLOOP
					SCE_GS_TRUE,	// EOP
					SCE_GS_FALSE,	// PRE
					0,				// PRIM
					SCE_GIF_PACKED, // FLG
					1				// NREG
				);
				pRVar6->cmdB = SCE_GIF_PACKED_AD;
				pRVar6 = pRVar6 + 1;
			}
			gNbStateAdded = gNbStateAdded + 1;
			pRVar6->cmdA = cmdA;
			pRVar6->cmdB = cmdB;
			gCurDList->pRenderCommands = pRVar6 + 1;
		}
		else {
			iVar5 = 0;
			do {
				gCurDList = gCurDListHandle + iVar5;
				pRVar6 = gCurDList->pRenderCommands;
				gNbStateAdded = iVar3;

				// Begin the giftag header if we have no commands already.
				if (iVar3 == 0) {
					if ((gCurDList->flags_0x0 & 1) != 0) {
						uVar1 = gCurDList->nbCommands;
						pDVar2 = gCurDList->aCommands;
						pDVar2[uVar1].dataType = DISPLAY_LIST_DATA_TYPE_PKT;
						pDVar2[uVar1].pRenderInput.pPkt = pRVar6;
					}

					pRVar6->cmdA = SCE_GIF_SET_TAG(
						1,				// NLOOP
						SCE_GS_TRUE,	// EOP
						SCE_GS_FALSE,	// PRE
						0,				// PRIM
						SCE_GIF_PACKED, // FLG
						1				// NREG
					);
					pRVar6->cmdB = SCE_GIF_PACKED_AD;
					pRVar6 = pRVar6 + 1;
				}
				iVar5 = iVar5 + 1;
				gNbStateAdded = gNbStateAdded + 1;
				pRVar6->cmdA = cmdA;
				pRVar6->cmdB = cmdB;
				gCurDList->pRenderCommands = pRVar6 + 1;
				gCurDList = pDVar4;
			} while (iVar5 < 2);
		}
	}
	return;
}

void edDListAlphaTestAndZTest
(ulong ate, ulong atst, ulong aref, ulong afail, ulong date, ulong datm, ulong zte, ulong ztst)
{
	edDListSetState
	(
		SCE_GS_SET_TEST(
			ate & 0xff,			// ATE 
			atst & 0xffffffff,	// ATST
			aref & 0xffffffff,	// AREF
			afail & 0xffffffff,	// AFAIL
			date & 0xff,		// DATE
			datm & 0xffffffff,	// DATM
			zte & 0xff,			// ZTE
			ztst & 0xffffffff	// ZTST
		),
		SCE_GS_TEST_1);

	return;
}

void edDListAlphaBlend(uint a, uint b, uint c, uint d, uint fix)
{
	edDListBlendSet(1);

	edDListSetState(
		SCE_GS_SET_ALPHA(
			a & 0xff,	// A
			b & 0xff,	// B
			c & 0xff,	// C
			d & 0xff,	// D
			fix & 0xff	// FIX
		),
		SCE_GS_ALPHA_1);

	return;
}

void edDListBlendFuncNormal(void)
{
	edDListBlendSet(1);
	edDListSetState(SCE_GS_SET_ALPHA(SCE_GS_ALPHA_CS, SCE_GS_ALPHA_CD, SCE_GS_ALPHA_CS, SCE_GS_ALPHA_CD, 0), SCE_GS_ALPHA_1);
	return;
}

void edDListSetActiveViewPort(ed_viewport* pViewport)
{
	if (gbInsideBegin == false) {
		gIncViewportX = (0x800 - ((int)(uint)pViewport->pColorBuffer->pSurfaceDesc->screenWidth >> 1)) * 0x10;
		gIncViewportY = (0x800 - ((int)(uint)pViewport->pColorBuffer->pSurfaceDesc->screenHeight >> 1)) * 0x10;
		gCurViewport = pViewport;
		if ((((gCurDList != (DisplayList*)0x0) &&
			(gCurDList->nbCommands != 0)) && ((gCurDList->flags_0x0 & 2) != 0))
			&& (gCurDList->aCommands->aCommandArray
				[gCurDList->nbCommands - 1].type == LM_REF_0)) {
			edDListSetState(
				SCE_GS_SET_XYOFFSET(
					((0x1000 - ((int)pViewport->screenWidth + (int)pViewport->posX) >> 1) << 4),	// X
					((0x1000 - ((int)pViewport->screenHeight + (int)pViewport->posY) >> 1) << 4)	// Y
				),
				SCE_GS_XYOFFSET_1);
			if (gCurViewport->screenHeight == 0x1c0) {
				edDListSetState(
					SCE_GS_SET_SCISSOR(
						pViewport->posX,													// SCAX0
						((int)gCurViewport->screenWidth + (int)gCurViewport->posX + -1),	// SCAX1
						pViewport->posY,													// SCAY0
						gCurViewport->posY + 0x1ff											// SCAY1
					),
					SCE_GS_SCISSOR_1);
			}
			else {
				edDListSetState(
					SCE_GS_SET_SCISSOR(
						pViewport->posX,													// SCAX0
						((int)gCurViewport->screenWidth + (int)gCurViewport->posX + -1),	// SCAX1
						pViewport->posY,													// SCAY0
						((int)gCurViewport->screenHeight + (int)gCurViewport->posY + -1)	// SCAY1
					),
					SCE_GS_SCISSOR_1);
			}
		}
	}
	return;
}

#ifdef PLATFORM_WIN
void LogBitmap(const char* prefix, ed_g2d_bitmap* pBitmap)
{
	EDDLIST_LOG(LogLevel::Verbose, "{} w: {}, h: {}, psm: {} maxMipLevel: {}", prefix, pBitmap->width, pBitmap->height, pBitmap->psm, pBitmap->maxMipLevel);
}

Renderer::TextureData MakeTextureDataFromPacket(ed_g2d_material* pMaterial, ed_g2d_bitmap* pTextureBitmap, ed_g2d_bitmap* pPaletteBitmap, int index)
{
	LogBitmap("MakeTextureDataFromPacket TEX", pTextureBitmap);
	LogBitmap("MakeTextureDataFromPacket PAL", pPaletteBitmap);

	edPSX2Header* pHeader = (edPSX2Header*)LOAD_SECTION(pPaletteBitmap->pPSX2);
	edpkt_data* pPkt = (edpkt_data*)(LOAD_SECTION((pHeader + index)->pPkt));
	pPkt += 2;

	int imageIndex = -1;

	uint texWidth;
	uint texHeight;
	uint palWidth;
	uint palHeight;
	bool bSetTexDimensions = false;
	bool bSetPalDimensions = false;

	int clutBasePtr = 0;

	Renderer::PaletteMap palettes;

	bool bCheckNext = false;

	while (true)
	{
		if ((pPkt->asU32[0] >> 28) == 0x03) {
			if (imageIndex == -1) {
				imageIndex = pPkt->asU32[1];
			}
			else {
				const int paletteIndex = pPkt->asU32[1];
				palettes[clutBasePtr] = { LOAD_SECTION(paletteIndex), pPaletteBitmap->width, pPaletteBitmap->height, palWidth, palHeight, pPaletteBitmap->psm, pPaletteBitmap->maxMipLevel };

				EDDLIST_LOG(LogLevel::Verbose, "MakeTextureDataFromPacket PAL - CBP: {} (0x{:x}) w: {} h: {} rw: {} rh: {} psm: {} max mip: {}", clutBasePtr, clutBasePtr, pPaletteBitmap->width, pPaletteBitmap->height, palWidth, palHeight, pPaletteBitmap->psm, pPaletteBitmap->maxMipLevel);
			}
		}

		if (pPkt->cmdB == SCE_GS_TRXREG) {
			if (!bSetTexDimensions) {
				texWidth = pPkt->asU32[0];
				texHeight = pPkt->asU32[1];
				bSetTexDimensions = true;
			}
			else {
				palWidth = pPkt->asU32[0];
				palHeight = pPkt->asU32[1];
				bSetPalDimensions = true;
			}
		}

		if (pPkt->cmdB == SCE_GS_BITBLTBUF) {
			clutBasePtr = pPkt->asU32[1] & 0xFFFF;

			EDDLIST_LOG(LogLevel::Verbose, "MakeTextureDataFromPacket Found BITBLTBUF CBP: {} (0x{:x})", clutBasePtr, clutBasePtr);
			bCheckNext = false;
		}

		if (bCheckNext == true) {
			break;
		}

		if (pPkt->asU32[2] == 0xe) {
			bCheckNext = true;
		}

		pPkt += 1;
	}

	assert(bSetTexDimensions);
	assert(palettes.size() > 0);

	EDDLIST_LOG(LogLevel::Verbose, "MakeTextureDataFromPacket TEX - rw: {} (0x{:x}) rh: {} (0x{:x})", texWidth, texWidth, texHeight, texHeight);

	return { pMaterial, { LOAD_SECTION(imageIndex), pTextureBitmap->width, pTextureBitmap->height, texWidth, texHeight, pTextureBitmap->psm, pTextureBitmap->maxMipLevel },
		palettes };
}
#endif

void edDListUseMaterial(edDList_material* pMaterialInfo)
{
	edpkt_data* pRVar1;
	edpkt_data* pRVar2;
	ed_g2d_bitmap* pPaletteBitmap;
	edpkt_data* pRVar4;
	RenderCommandUint* pDVar4;
	DisplayListCommand* pDVar5;
	RenderCommandUint* pDVar6;
	ed_g2d_bitmap* pTextureBitmap;
	bool bHasPalette;
	int iVar3;

	gCurFlashMaterial = 0;
	gCurMaterial = pMaterialInfo;

#ifdef PLATFORM_WIN
	if (pMaterialInfo == (edDList_material*)0x0) {
		DISPLAY_LIST_BIND_TEXTURE(nullptr);
	}
	else {
		const Renderer::Kya::G2D::Material* pMaterial = Renderer::Kya::GetTextureLibrary().FindMaterial(pMaterialInfo->pMaterial);
		// Can probably remove this bind later and rely on the DisplayList binding only.
		Renderer::BindTexture(pMaterial->layers.front().textures.front().pSimpleTexture);

		DISPLAY_LIST_BIND_TEXTURE(pMaterial->layers.front().textures.front().pSimpleTexture);
	}
#endif

	edDListPatchGifTag2D();

	if ((gCurDList->flags_0x0 & 2) == 0) {
		return;
	}

	pDVar5 = gCurDList->aCommands;
	pRVar1 = gCurDList->pRenderCommands;

	if (gCurDList->nbCommands != 0) {
		pDVar4 = pDVar5->aCommandArray + (gCurDList->nbCommands - 1);
		pDVar4->size = (uint)((int)pRVar1 - (int)pDVar4->pCommandBuffer);
		pDVar5 = (DisplayListCommand*)(pDVar4 + 1);
	}

	if (pMaterialInfo != (edDList_material*)0x0) {
		if ((pMaterialInfo->mode & 4U) == 0) {
			if (pMaterialInfo->pMaterial != (ed_g2d_material*)0x0) {
				bHasPalette = false;
				pTextureBitmap = (ed_g2d_bitmap*)0x0;
				pPaletteBitmap = edDListGetG2DBitmap(pMaterialInfo->pMaterial, 0, &bHasPalette, &pTextureBitmap);
				pDVar6 = pDVar5->aCommandArray;
				if (pPaletteBitmap != (ed_g2d_bitmap*)0x0) {
					pDVar5->aCommandArray[0].pCommandBuffer = ((edpkt_data*)LOAD_SECTION(pPaletteBitmap->pPSX2)) + 0x4;
					pDVar5->aCommandArray[0].type = LM_CALL;
						
					if ((bHasPalette != false) && (iVar3 = (uint)pTextureBitmap->width * (uint)pTextureBitmap->height, 0x3ffff < iVar3)) {
						pRVar2 = pDVar5->aCommandArray[0].pCommandBuffer;
						pRVar4 = pRVar2 + 0x12;
						if (iVar3 == 0x40000) {
							pRVar4 = pRVar2 + 10;
						}
						*(uint*)((int)&pRVar4->cmdA + 4) = *(uint*)((int)&pRVar4->cmdA + 4) & 0xffffc000 | 0x800;
					}
					pDVar6 = pDVar5->aCommandArray + 1;
					gCurDList->nbCommands = gCurDList->nbCommands + 1;
				}

				pDVar6->pCommandBuffer = (edpkt_data*)((char*)LOAD_SECTION(pMaterialInfo->pMaterial->pCommandBufferTexture) + 0x10);

				if ((bHasPalette != false) && (0x3ffff < (uint)pTextureBitmap->width * (uint)pTextureBitmap->height)) {
					*(uint*)((int)&pDVar6->pCommandBuffer[1].cmdA + 4) =
						*(uint*)((int)&pDVar6->pCommandBuffer[1].cmdA + 4) & 0xfffc001f | 0x10000;
				}

#ifdef PLATFORM_WIN
				ProcessTextureCommands(pDVar6->pCommandBuffer, pMaterialInfo->pMaterial->commandBufferTextureSize);
#endif

				pDVar6->type = LM_REF_1;
				pDVar6->size = pMaterialInfo->pMaterial->commandBufferTextureSize * 0x10 - 0x10;
				gCurDList->nbCommands = gCurDList->nbCommands + 1;
				pDVar5 = (DisplayListCommand*)(pDVar6 + 1);
				if (pMaterialInfo->pMaterial->nbLayers != 0) {
					edDListBlendSet(1);
					pDVar6[1].pCommandBuffer = pRVar1;
					goto LAB_002cba4c;
				}
				edDListBlendSet(0);
			}
		}
		else {
			pDVar5->aCommandArray[0].pCommandBuffer = (edpkt_data*)0x0;
			pDVar5->aCommandArray[0].type = LM_FUNC;
			pDVar5 = (DisplayListCommand*)(pDVar5->aCommandArray + 1);
			gCurDList->nbCommands = gCurDList->nbCommands + 1;
		}
	}
	pDVar5->aCommandArray[0].pCommandBuffer = pRVar1;
LAB_002cba4c:
	pDVar5->aCommandArray[0].type = LM_REF_0;
	pDVar5->aCommandArray[0].size = 0;
	gCurDList->nbCommands = gCurDList->nbCommands + 1;
	if ((gCurDList->nbCommands == 1) || (gCurDList->nbCommands == 3)) {
		edDListAlphaTestAndZTest(1, 7, 0, 0, 0, 0, 1, 1);
		edDListSetActiveViewPort(gCurViewport);
	}

	//DUMP_TAG_ADV(pDVar5->aCommandArray[0].pCommandBuffer[0]);

	return;
}

void edDlistAddtoView(DisplayList* pInDisplayList)
{
	ushort uVar1;
	DisplayList* pDisplayList;

	pDisplayList = pInDisplayList + gCurRenderState;
	if ((pInDisplayList[gCurRenderState].flags_0x0 & 4) != 0) {
		pInDisplayList->field_0x3 = 0;
		pDisplayList = pInDisplayList;
	}
	if (pDisplayList->nbCommands == 0) {
		pDisplayList->pRenderCommands = pDisplayList->field_0x14;
		pDisplayList->field_0x10 = (edpkt_data*)pDisplayList->pCommandBuffer;
	}
	else {
		pDisplayList->field_0x3 = 1;
		uVar1 = pDisplayList->flags_0x0;
		if ((uVar1 & 1) == 0) {
			if ((uVar1 & 2) != 0) {
				if ((uVar1 & 0x20) == 0) {
					edListAddNode(gDList_2D[gCurRenderState], pDisplayList);
				}
				else {
					edListAddNode(gDList_2D_Before3D[gCurRenderState], pDisplayList);
				}
			}
		}
		else {
			gDList_3D[gCurRenderState][gNbDList_3D[gCurRenderState]] = pDisplayList;
			gNbDList_3D[gCurRenderState] = gNbDList_3D[gCurRenderState] + 1;
		}
	}
	return;
}

void edDListVertex4f_2D(float inX, float inY, float inZ, uint param_4)
{
	DisplayList* pDVar1;
	int y;
	int x;
	edpkt_data** ppRVar4;
	edF32VECTOR4 local_10;

	local_10.w = 1.0f;
	local_10.z = 0.0f;
	local_10.x = inX;
	local_10.y = inY;

	edF32Matrix4MulF32Vector4Hard(&local_10, gCurMatrix + gNbMatrix, &local_10);

	pDVar1 = gCurDList;

	x = gIncViewportX + (int)(local_10.x * 16.0f);
	y = gIncViewportY + (int)(local_10.y * 16.0f);

	if (gCurPrimType == 8) {
		g_Count_004495f8 = g_Count_004495f8 + 1;
#ifdef PLATFORM_WIN
		Renderer::SetVertexSkip((g_Count_004495f8 == 1) || (g_Count_004495f8 == 2));
#endif
		if ((g_Count_004495f8 == 1) || (g_Count_004495f8 == 2)) {
			gCurDList->pRenderCommands->cmdA = SCE_GS_SET_XYZ(x, y, inZ);
			pDVar1->pRenderCommands->cmdB = SCE_GS_XYZ3;
			pDVar1->pRenderCommands = pDVar1->pRenderCommands + 1;

			DISPLAY_LIST_SET_VERTEX(local_10.x, local_10.y, inZ, 1);
		}
		else {
			gCurDList->pRenderCommands->cmdA = SCE_GS_SET_XYZ(x, y, inZ);
			pDVar1->pRenderCommands->cmdB = SCE_GS_XYZ2;
			pDVar1->pRenderCommands = pDVar1->pRenderCommands + 1;
			if (g_Count_004495f8 == 4) {
				g_Count_004495f8 = 0;
			}

			DISPLAY_LIST_SET_VERTEX(local_10.x, local_10.y, inZ, 0);
		}
	}
	else {
		ppRVar4 = &gCurDList->pRenderCommands;
#ifdef PLATFORM_WIN
		Renderer::SetVertexSkip(param_4 == 0xc000);
#endif
		gCurDList->pRenderCommands->cmdA = SCE_GS_SET_XYZ(x, y, inZ);
		if (param_4 == 0xc000) {
			(*ppRVar4)->cmdB = SCE_GS_XYZ3;
		}
		else {
			(*ppRVar4)->cmdB = SCE_GS_XYZ2;
		}

		*ppRVar4 = *ppRVar4 + 1;

		DISPLAY_LIST_SET_VERTEX(local_10.x, local_10.y, inZ, param_4 == 0xc000);
	}

	gNbAddedVertex = gNbAddedVertex + 1;

	return;
}

void edDListColor4u8_2D(byte r, byte g, byte b, byte a)
{
	DisplayList* pDVar1;

	g_RGBAQ_00448aa0->r = r;
	g_RGBAQ_00448aa0->g = g;
	g_RGBAQ_00448aa0->b = b;
	g_RGBAQ_00448aa0->a = a;
	pDVar1 = gCurDList;
	gCurDList->pRenderCommands->cmdA = SCE_GS_SET_RGBAQ(r, g, b, a, 0x3f800000);
	pDVar1->pRenderCommands->cmdB = SCE_GS_RGBAQ;
	pDVar1->pRenderCommands = pDVar1->pRenderCommands + 1;

	DISPLAY_LIST_SET_COLOR(r, g, b, a, 1.0f);

	return;
}

void edDListTexCoo2f_2D(float inS, float inT)
{
	DisplayList* pDVar1;

	pDVar1 = gCurDList;

	uint S = *reinterpret_cast<uint*>(&inS);
	uint T = *reinterpret_cast<uint*>(&inT);

	gCurDList->pRenderCommands->cmdA = SCE_GS_SET_ST(S, T);
	pDVar1->pRenderCommands->cmdB = SCE_GS_ST;
	pDVar1->pRenderCommands = pDVar1->pRenderCommands + 1;

	DISPLAY_LIST_SET_TEXCOORD(inS, inT);

	return;
}

void edDListBegin2D(ulong mode)
{
	uint uVar3;
	long lVar4;

	// Make sure that the current command is aligned to 16 bytes.
	uVar3 = (uint)gCurDList->pRenderCommands & 0xf;
	if (uVar3 != 0) {
		IMPLEMENTATION_GUARD();
		gCurDList->pRenderCommands = (edpkt_data*)((int)gCurDList->pRenderCommands + (0x10 - uVar3));
	}

	if ((gCurMaterial != (edDList_material*)0x0) || (lVar4 = 0, gCurFlashMaterial != 0)) {
		lVar4 = 1;
	}

	// Store this command as the top of the draw, to be updated in edDListEnd or edDListPatchGifTag3D.
	gCurStatePKT = gCurDList->pRenderCommands;

	uint primReg = gBlendMode << 6 | mode & 0xffffffff | lVar4 << 4;
	if (edDlistUseUV == 0) {
		gCurDList->pRenderCommands->cmdA = SCE_GIF_SET_TAG(
			0,							// NLOOP (Will be updated later in edDListEnd)
			SCE_GS_TRUE,				// EOP
			SCE_GS_TRUE,				// PRE
			primReg,					// PRIM
			SCE_GIF_PACKED,				// FLG
			1							// NREG
		);
	}
	else {
		// Enable UV instead of STQ.
		primReg = primReg | 8;

		gCurDList->pRenderCommands->cmdA = SCE_GIF_SET_TAG(
			0,							// NLOOP (Will be updated later in edDListEnd)
			SCE_GS_TRUE,				// EOP
			SCE_GS_TRUE,				// PRE
			primReg,					// PRIM
			SCE_GIF_PACKED,				// FLG
			1							// NREG
		);

	}

	gCurDList->pRenderCommands->cmdB = SCE_GIF_PACKED_AD;

	gCurDList->pRenderCommands = gCurDList->pRenderCommands + 1;

	DISPLAY_LIST_2D_BEGIN(gCurViewport->screenWidth, gCurViewport->screenHeight, mode);

	return;
}

void edDListPatchGifTag3D(void)
{
	edpkt_data* pRVar1;
	edpkt_data* pRVar2;
	int iVar3;

	if (gNbStateAdded != 0) {
		IMPLEMENTATION_GUARD(
		pRVar1 = gCurDList->pRenderCommands;
		*(undefined4*)&pRVar1->cmdA = 0;
		*(undefined4*)((int)&pRVar1->cmdA + 4) = 0x14000000;
		pRVar1->cmdB = 0;
		pRVar2 = pRVar1 + 1;
		gCurStatePKTSize = gNbStateAdded + 2;
		gCurStatePKT =
			gCurDList->aCommands[gCurDList->nbCommands].
			pRenderInput;
		gCurDList->aCommands[gCurDList->nbCommands].pRenderInput
			= pRVar2;
		iVar3 = gCurStatePKTSize - 1;
		pRVar2->cmdA = ((long)(int)gCurStatePKT & 0xfffffffU) << 0x20 |
			(long)(int)(gCurStatePKTSize | 0x30000000);
		pRVar1[1].cmdB = ((long)(iVar3 * 0x10000) & 0xffffffffU | 0x6c0003dc) << 0x20 | 0x40003dc;
		gCurDList->pRenderCommands = pRVar1 + 2;
		gCurDList->field_0x10 = (MeshDrawRenderCommand*)gCurDList->pRenderCommands;
		gCurDListBuf = gCurDList->field_0x10;
		gCurDList->nbCommands = gCurDList->nbCommands + 1;
		iVar3 = -gCurStatePKTSize;)
		IMPLEMENTATION_GUARD();
		pRVar2[iVar3].cmdA = (long)gNbStateAdded | 0x1000000000008000;
#ifdef PLATFORM_WIN
		DUMP_TAG_ADV(pRVar2[iVar3].cmdA);
#endif
		pRVar2[iVar3].cmdB = 0xe;
		gNbStateAdded = 0;
	}
	return;
}

void edDListBeginStrip(float x, float y, float z, uint nbVertex, ushort type)
{
	ed_3d_strip** ppeVar1;
	edVertex* peVar2;
	short sVar3;
	uint uVar4;
	int iVar5;
	uint uVar6;
	DisplayListCommand* pNewCommand;
	ushort uVar7;
	ushort meshCount;
	ed_3d_strip* peVar9;
	uint uVar10;
	ed_3d_strip* pNewStrip;

	pNewCommand = gCurDList->aCommands + gCurDList->nbCommands;

	pNewCommand->nbAddedVertex = nbVertex;
	pNewCommand->bActive = 1;

	gCurInfPosForBegin = (uint)gCurDList->nbCommands;
	gCurDList->nbCommands = gCurDList->nbCommands + 1;

	pNewStrip = gCurDListBuf;

	EDDLIST_LOG(LogLevel::Verbose, "edDListBeginStrip: x: {} y: {} z: {} nbVertex: {} type: {} strip: 0x{:x}", x, y, z, nbVertex, type, (uintptr_t)pNewStrip);

	if (gCurDList->aSavedCommands != (DisplayListCommand**)0x0) {
		gCurDList->aSavedCommands[gCurDList->nbSavedCommands] = pNewCommand;
		gCurDList->nbSavedCommands = gCurDList->nbSavedCommands + 1;
	}

	if (gCurMaterial == (edDList_material*)0x0) {
		pNewStrip->materialIndex = -1;
	}
	else {
		pNewStrip->materialIndex = (short)gCurMaterial->index;
	}

	pNewStrip->cachedIncPacket = -1;
	pNewStrip->primListIndex = 0;
	meshCount = 0;
	do {
		uVar7 = meshCount;
		meshCount = uVar7 + 1;
	} while ((uVar7 + 1) * 0x46 + 2 < nbVertex);

	iVar5 = nbVertex + ((uint)uVar7 * 8 - (uint)uVar7) * -10;
	meshCount = uVar7 + 1;
	if (iVar5 == 0) {
		iVar5 = 0x48;
		meshCount = uVar7;
	}

	pNewStrip->meshCount = meshCount;
	assert(pNewStrip->meshCount > 0);

	pNewStrip->field_0x38 = (byte)iVar5;
	pNewStrip->flags = 0x4000000;

	if (gbLight != 0) {
		pNewStrip->flags = pNewStrip->flags | 0x10;
	}

	if (gShadowCast == 0) {
		pNewStrip->shadowCastFlags = 0;
	}
	else {
		pNewStrip->shadowCastFlags = gShadowCast;
	}

	sVar3 = gShadowReceive;
	if (gShadowReceive == 0) {
		pNewStrip->shadowReceiveFlags = 0;
	}
	else {
		gbForceMaterialUse = true;
		pNewStrip->flags = pNewStrip->flags | 0x2000;
		pNewStrip->shadowReceiveFlags = sVar3;
	}

	(pNewStrip->boundingSphere).x = x;
	(pNewStrip->boundingSphere).y = y;
	(pNewStrip->boundingSphere).z = z;
	(pNewStrip->boundingSphere).w = 100000.0f;

	iVar5 = nbVertex + ((int)(nbVertex + 1) / 0x48) * 2;
	uVar6 = iVar5 * 4 + 0x40;
	if ((uVar6 & 0xf) != 0) {
		uVar6 = uVar6 + (0x10 - (uVar6 & 0xf));
	}

	uVar4 = iVar5 * 4 + 0x40;
	if ((uVar4 & 0xf) != 0) {
		uVar4 = uVar4 + (0x10 - (uVar4 & 0xf));
	}

	if ((gCurDList->flags_0x0 & 0x100) == 0) {
		pNewStrip->pBoundSpherePkt = 0x0;
		pNewStrip->pSTBuf = STORE_SECTION((char*)&pNewStrip[1].boundingSphere);

		uVar10 = (uint)LOAD_SECTION(pNewStrip->pSTBuf) & 0xf;
		if (uVar10 != 0) {
			pNewStrip->pSTBuf = STORE_SECTION((char*)LOAD_SECTION(pNewStrip->pSTBuf) + (0x10 - uVar10));
		}
	}
	else {
		pNewStrip->pBoundSpherePkt = STORE_SECTION(&pNewStrip[1].boundingSphere);
		uVar10 = (uint)LOAD_SECTION(pNewStrip->pBoundSpherePkt) & 0xf;
		if (uVar10 != 0) {
			char* pPkt = (char*)LOAD_SECTION(pNewStrip->pBoundSpherePkt);
			pNewStrip->pBoundSpherePkt = STORE_SECTION(pPkt + (0x10 - uVar10));
		}

		pNewStrip->pSTBuf = STORE_SECTION((char*)LOAD_SECTION(pNewStrip->pBoundSpherePkt) + meshCount);
		pNewCommand->field_0x41 = (byte)meshCount;
		uVar10 = (uint)LOAD_SECTION(pNewStrip->pSTBuf) & 0xf;
		if (uVar10 != 0) {
			char* pPkt = (char*)LOAD_SECTION(pNewStrip->pBoundSpherePkt);
			pNewStrip->pSTBuf = STORE_SECTION(pPkt + (0x10 - uVar10));
		}
	}

	pNewStrip->pColorBuf = STORE_SECTION((char*)LOAD_SECTION(pNewStrip->pSTBuf) + uVar6);
	pNewStrip->pColorBuf = STORE_SECTION((char*)LOAD_SECTION(pNewStrip->pColorBuf) + 4);

	uVar10 = (uint)LOAD_SECTION(pNewStrip->pColorBuf) & 0xf;
	if (uVar10 != 0) {
		char* pPkt = (char*)LOAD_SECTION(pNewStrip->pColorBuf);
		pNewStrip->pColorBuf = STORE_SECTION(pPkt + (0x10 - uVar10));
	}

	pNewStrip->pVertexBuf = STORE_SECTION((char*)LOAD_SECTION(pNewStrip->pColorBuf) + uVar4);
	uVar10 = (uint)LOAD_SECTION(pNewStrip->pVertexBuf) & 0xf;
	if (uVar10 != 0) {
		char* pPkt = (char*)LOAD_SECTION(pNewStrip->pVertexBuf);
		pNewStrip->pVertexBuf = STORE_SECTION(pPkt + (0x10 - uVar10));
	}

	gCurColorBuf = (_rgba*)LOAD_SECTION(pNewStrip->pColorBuf);
	gCurSTBuf = (short*)LOAD_SECTION(pNewStrip->pSTBuf) + 0x10;
	gCurVertexBuf = (edVertex*)LOAD_SECTION(pNewStrip->pVertexBuf);
	if ((nbVertex & 3) != 0) {
		nbVertex = nbVertex + (4 - (nbVertex & 3));
	}

	peVar9 = (ed_3d_strip*)((int)(ed_3d_strip*)(gCurVertexBuf + nbVertex) - (ulong)pNewStrip);
	ppeVar1 = (ed_3d_strip**)&pNewStrip->vifListOffset;

	gCurDListBuf = (ed_3d_strip*)(gCurVertexBuf + nbVertex);
	gStartVertexBuf = (undefined*)gCurVertexBuf;
	*ppeVar1 = peVar9;
	pNewStrip->field_0x2c = 0x0;
	peVar2 = gCurVertexBuf;
	gCurVertexBuf->skip = 0xc000;
	peVar2[1].skip = 0xc000;
	gEndSTNbInVertex = 1;
	gEndColorNbInVertex = 1;
	gCurSTNbInVertex = 0;
	gEndColorBuf = (char*)gCurColorBuf + uVar4;
	gMaxSTNbInVertex = 0;
	gCurColorNbInVertex = 0;
	gEndVertexBuf = (char*)gCurDListBuf;
	gEndSTBuf = (char*)gCurSTBuf + uVar6;

	pNewCommand->pRenderInput.pStrip = pNewStrip;
	if (((uint)gCurDListBuf & 0xf) != 0) {
		IMPLEMENTATION_GUARD(
		gCurDListBuf = (ed_3d_strip*)((int)gCurDListBuf + (0x10 - ((uint)gCurDListBuf & 0xf)));)
	}

	pNewCommand->dataType = type;
	pNewCommand->primType = gCurPrimType;

	edF32Matrix4CopyHard(&pNewCommand->matrix, gCurMatrix + gNbMatrix);
	gNbVertexDMA = 0x48;

	DISPLAY_LIST_3D_BEGIN(gCurViewport->screenWidth, gCurViewport->screenHeight, type);

	return;
}

_rgba gCurColor = _rgba(0, 0, 0, 0);
_rgba* gCurColor_SPR = &gCurColor;

float gCurST[8] = {};
float* gCurST_SPR = gCurST;

void edDListVertex4f_3D_TRIANGLE(float x, float y, float z, uint skip)
{
	short* puVar1;
	_rgba* p_Var1;
	uint local_4;

	if (((uint)(gNbAddedVertex % 3) < 2) || (local_4 = skip, skip == 49152.0f)) {
		local_4 = 0xc000;
	}
	gNbAddedVertex = gNbAddedVertex + 1;
	gCurVertexBuf->x = (int)x;
	gCurVertexBuf->y = (int)y;
	gCurVertexBuf->z = (int)z;
	gCurVertexBuf->skip = local_4;
	gCurVertexBuf[1].skip = 0xc000;

	gCurColorBuf->r = gCurColor_SPR->r;
	gCurColorBuf->g = gCurColor_SPR->g;
	gCurColorBuf->b = gCurColor_SPR->b;
	gCurColorBuf->a = gCurColor_SPR->a;

	p_Var1 = gCurColorBuf + 1;

	gCurSTBuf[0] = (short)(int)(gCurST_SPR)[0];
	gCurSTBuf[1] = (short)(int)(gCurST_SPR)[1];
	puVar1 = gCurSTBuf + 2;
	if (gNbDMAVertex == 0x47) {
		*p_Var1 = gCurColorBuf[-1];
		gCurColorBuf[2] = *gCurColorBuf;
		p_Var1 = gCurColorBuf + 3;
		*(undefined4*)puVar1 = *(undefined4*)(gCurSTBuf + -2);
		*(undefined4*)(gCurSTBuf + 4) = *(undefined4*)gCurSTBuf;
		gNbDMAVertex = 2;
		puVar1 = gCurSTBuf + 6;
	}
	else {
		gNbDMAVertex = gNbDMAVertex + 1;
	}

	DISPLAY_LIST_SET_COLOR(gCurColorBuf->r, gCurColorBuf->g, gCurColorBuf->b, gCurColorBuf->a, 1.0f);
	DISPLAY_LIST_SET_TEXCOORD(gCurSTBuf[0], gCurSTBuf[1]);
	DISPLAY_LIST_SET_VERTEX(x, y, z, local_4 == 49152.0f);

	gCurVertexBuf = gCurVertexBuf + 1;
	gCurColorBuf = p_Var1;
	gCurSTBuf = puVar1;
	return;
}

void edDListVertex4f_3D_DEFAULT(float x, float y, float z, uint skip)
{
	short* puVar1;
	_rgba* p_Var1;
	uint local_4;

	local_4 = skip;
	if ((float)skip == 49152.0f) {
		local_4 = 0xc000;
	}

	gNbAddedVertex = gNbAddedVertex + 1;

	gCurVertexBuf->x = x;
	gCurVertexBuf->y = y;
	gCurVertexBuf->z = z;
	gCurVertexBuf->skip = local_4;
	gCurVertexBuf[1].skip = 0xc000;

	gCurColorBuf->r = gCurColor_SPR->r;
	gCurColorBuf->g = gCurColor_SPR->g;
	gCurColorBuf->b = gCurColor_SPR->b;
	gCurColorBuf->a = gCurColor_SPR->a;

	p_Var1 = gCurColorBuf + 1;

	gCurSTBuf[0] = (short)(int)(gCurST_SPR)[0];
	gCurSTBuf[1] = (short)(int)(gCurST_SPR)[1];
	puVar1 = gCurSTBuf + 2;

	if (gNbDMAVertex == 0x47) {
		*p_Var1 = gCurColorBuf[-1];
		gCurColorBuf[2] = *gCurColorBuf;
		p_Var1 = gCurColorBuf + 3;
		*(undefined4*)puVar1 = *(undefined4*)(gCurSTBuf + -2);
		*(undefined4*)(gCurSTBuf + 4) = *(undefined4*)gCurSTBuf;
		gNbDMAVertex = 2;
		puVar1 = gCurSTBuf + 6;
	}
	else {
		gNbDMAVertex = gNbDMAVertex + 1;
	}

	DISPLAY_LIST_SET_COLOR(gCurColorBuf->r, gCurColorBuf->g, gCurColorBuf->b, gCurColorBuf->a, 1.0f);
	DISPLAY_LIST_SET_TEXCOORD(gCurSTBuf[0], gCurSTBuf[1]);
	DISPLAY_LIST_SET_VERTEX(x, y, z, local_4 == 49152.0f);

	gCurVertexBuf = gCurVertexBuf + 1;
	gCurColorBuf = p_Var1;
	gCurSTBuf = puVar1;

	return;
}

DisplayListXYZFunc gTableAddVertexFUNC_3D[13] = {
	NULL,
	NULL,
	NULL,
	edDListVertex4f_3D_TRIANGLE,
	edDListVertex4f_3D_DEFAULT,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
};

void edDListColor4u8_3D(byte r, byte g, byte b, byte a)
{
	gCurColor_SPR->r = r;
	gCurColor_SPR->g = g;
	gCurColor_SPR->b = b;
	gCurColor_SPR->a = a;
	return;
}

DisplayListRGBAQFunc gTableAddColorFUNC_3D[13] = {
	NULL,
	NULL,
	NULL,
	edDListColor4u8_3D,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
};

void edDListTexCoo2f_3D(float s, float t)
{
	gCurST_SPR[0] = s * 4096.0f;
	gCurST_SPR[1] = t * 4096.0f;
	return;
}

DisplayListSTFunc gTableAddSTFUNC_3D[13] = {
	NULL,
	NULL,
	NULL,
	edDListTexCoo2f_3D,
	edDListTexCoo2f_3D,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
};

void edDListBegin(float x, float y, float z, uint primType, int nbVertex)
{
	edpkt_data* pRVar1;
	int iVar2;
	edpkt_data* pcVar3;
	DisplayList* pDVar4;
	uint uVar5;
	DisplayListCommand* pDVar6;

	pcVar3 = (edpkt_data*)gCurDListBuf;
	iVar2 = gNbStateAdded;
	if (((gCurDList->flags_0x0 & 0x100) != 0) &&
		(pDVar4 = gCurDListHandle + gCurFlushState, gCurDList != pDVar4)) {
		gCurDListBuf = (ed_3d_strip*)pDVar4->field_0x10;
		gCurDList = pDVar4;
		edDListBegin(x, y, z, primType, nbVertex);
		gCurDList->field_0x10 = (edpkt_data*)gCurDListBuf;
		gbInsideBegin = false;
		gCurDList = gCurDListHandle + gCurRenderState;
	}

	uVar5 = primType;
	gCurPrimType = uVar5 & 0xff;
	gNbAddedVertex = 0;
	gNbDMAVertex = 0;
	gNbStateAdded = iVar2;
	gCurDListBuf = (ed_3d_strip*)pcVar3;
	gMaxNbVertex = nbVertex;
	if ((gCurDList->flags_0x0 & 1) == 0) {
		if (iVar2 != 0) {
			pRVar1 = gCurDList->pRenderCommands;
			pRVar1[-(iVar2 + 1)].cmdA = SCE_GIF_SET_TAG(
				iVar2,			// NLOOP
				SCE_GS_TRUE,	// EOP
				SCE_GS_FALSE,	// PRE
				0,				// PRIM
				SCE_GIF_PACKED,	// FLG
				1				// NREG
			);
			pRVar1[-(iVar2 + 1)].cmdB = SCE_GIF_PACKED_AD;
			gNbStateAdded = 0;
		}
		switch ((int)(primType & 0xff)) {
		case 0:
		case 1:
		case 2:
		case 3:
		case DISPLAY_LIST_DATA_TYPE_TRIANGLE_LIST:
		case 5:
		case DISPLAY_LIST_DATA_TYPE_SPRITE:
			edDListBegin2D(primType & 0xff);
			break;
		default:
			edDListBegin2D(DISPLAY_LIST_DATA_TYPE_TRIANGLE_LIST);
			break;
		case 8:
			g_Count_004495f8 = 0;
			edDListBegin2D(DISPLAY_LIST_DATA_TYPE_TRIANGLE_LIST);
		}

		gAddVertexFUNC = edDListVertex4f_2D;
		gAddColorFUNC = edDListColor4u8_2D;
		gAddSTFUNC = edDListTexCoo2f_2D;
		gbInsideBegin = true;
		return;
	}

	edDListPatchGifTag3D();

	if (((0 < gSubdivideLevel) && ((primType & 0xff) == 0xb)) || ((primType & 0xff) == 0xc)) {
		IMPLEMENTATION_GUARD(edDListSubDivBegin();)
	}

	pDVar6 = gCurDList->aCommands + gCurDList->nbCommands;

	if (gNbMatrixInArray < 1) {
		pDVar6->nbMatrix = 0;
		pDVar6->pCurMatrixArray = (edF32MATRIX4*)0x0;
	}
	else {
		pDVar6->pCurMatrixArray = gCurMatrixArray;
		pDVar6->nbMatrix = gNbMatrixInArray;
		gCurMatrixArray = (edF32MATRIX4*)0x0;
		gNbMatrixInArray = 0;
	}

	switch (uVar5 & 0xff) {
		case 0:
		case 2:
		case 4:
			edDListBeginStrip(x, y, z, nbVertex, (ushort)primType & 0xff);
			break;
		case 3:
		case 8:
			edDListBeginStrip(x, y, z, nbVertex, DISPLAY_LIST_DATA_TYPE_TRIANGLE_LIST);
			goto LAB_002ca38c;
		default:
			primType = 4;
			break;
		case 6:
		case 7:
		case 0xb:
		case 0xc:
			IMPLEMENTATION_GUARD(
				gMaxNbVertex = gMaxNbVertex << 2;
			edDListBeginSprite(nbVertex, uVar5 & 0xff);)
			break;
		case 10:
			nbVertex = nbVertex + 1;
			IMPLEMENTATION_GUARD(
				gMaxNbVertex = gMaxNbVertex + 1;)
		case 1:
			edDListBeginStrip(x, y, z, nbVertex, 2);
	}

	uVar5 = (uint)primType;
LAB_002ca38c:
	uVar5 = uVar5 & 0xff;
	gbInsideBegin = true;
	gAddVertexFUNC = gTableAddVertexFUNC_3D[uVar5];
	gAddColorFUNC = gTableAddColorFUNC_3D[uVar5];
	gAddSTFUNC = gTableAddSTFUNC_3D[uVar5];

	return;
}

edpkt_data* edDListCheckState(edpkt_data* pRenderCommand)
{
	ushort uVar1;
	DisplayListCommand* pDVar2;

	if (gNbStateAdded == 0) {
		if ((gCurDList->flags_0x0 & 1) != 0) {
			uVar1 = gCurDList->nbCommands;
			pDVar2 = gCurDList->aCommands;
			pDVar2[uVar1].dataType = DISPLAY_LIST_DATA_TYPE_PKT;
			pDVar2[uVar1].pRenderInput.pPkt = pRenderCommand;
		}

		pRenderCommand->cmdA = SCE_GIF_SET_TAG(
			1,				// NLOOP
			SCE_GS_TRUE,	// EOP
			SCE_GS_FALSE,	// PRE
			0,				// PRIM
			SCE_GIF_PACKED,	// FLG
			1				// NREG
		);

		pRenderCommand->cmdB = SCE_GIF_PACKED_AD;
		pRenderCommand = pRenderCommand + 1;
	}

	gNbStateAdded = gNbStateAdded + 1;

	return pRenderCommand;
}

void edDListColor4u8(byte r, byte g, byte b, byte a)
{
	DisplayList* pDVar1;
	edpkt_data* pRVar2;

	if (gAddColorFUNC == (DisplayListRGBAQFunc)0x0) {
		g_RGBAQ_00448aa0[gCurColorNbInVertex].r = r;
		g_RGBAQ_00448aa0[gCurColorNbInVertex].g = g;
		g_RGBAQ_00448aa0[gCurColorNbInVertex].b = b;
		g_RGBAQ_00448aa0[gCurColorNbInVertex].a = a;

		if (((gCurPrimType == 0xb) || (gCurPrimType == 0xc)) && (gCurColorNbInVertex < gEndColorNbInVertex)) {
			gCurColorNbInVertex = gCurColorNbInVertex + 1;
		}

		if (((gCurDList->flags_0x0 & 2) == 0) && (gCurPrimType == 7)) {
			gCurColorBuf->r = g_RGBAQ_00448aa0->r;
			gCurColorBuf->g = g_RGBAQ_00448aa0->g;
			gCurColorBuf->b = g_RGBAQ_00448aa0->b;
			gCurColorBuf->a = g_RGBAQ_00448aa0->a;
			gCurColorBuf[1].r = g_RGBAQ_00448aa0->r;
			gCurColorBuf[1].g = g_RGBAQ_00448aa0->g;
			gCurColorBuf[1].b = g_RGBAQ_00448aa0->b;
			gCurColorBuf[1].a = g_RGBAQ_00448aa0->a;
			gCurColorBuf = gCurColorBuf + 2;
		}

		if ((gCurDList->flags_0x0 & 2) != 0) {
			edDListPatchGifTag2D();
			if (gbInsideBegin == 0) {
				pRVar2 = edDListCheckState(gCurDList->pRenderCommands);
				gCurDList->pRenderCommands = pRVar2;
			}
			pDVar1 = gCurDList;
			gCurDList->pRenderCommands->cmdA = SCE_GS_SET_RGBAQ(r, g, b, a, 0x3f800000);
			pDVar1->pRenderCommands->cmdB = SCE_GS_RGBAQ;
			pDVar1->pRenderCommands = pDVar1->pRenderCommands + 1;

			DISPLAY_LIST_SET_COLOR(r, g, b, a, 1.0f);
		}
	}
	else {
		(gAddColorFUNC)(r, g, b, a);
	}

	return;
}

void edDListTexCoo2f(float s, float t)
{
	/* WARNING: Could not recover jumptable at 0x002cdff4. Too many branches */
	/* WARNING: Treating indirect jump as call */
	(gAddSTFUNC)(s, t);
	return;
}

void edDListVertex4f(float x, float y, float z, uint param_4)
{
	/* WARNING: Could not recover jumptable at 0x002cd6e4. Too many branches */
	/* WARNING: Treating indirect jump as call */
	(gAddVertexFUNC)(x, y, z, param_4);
	return;
}

void edDListSetProperty(uint type, uint value)
{
	byte bVar1;

	if ((gbInsideBegin == false) && (type < 7)) {
		bVar1 = (byte)value;
		switch (type) {
		case 0:
			IMPLEMENTATION_GUARD(
			gbFog = bVar1;)
			break;
		case 1:
			IMPLEMENTATION_GUARD(
			gbFogAlpha = bVar1;)
			break;
		case 2:
			gbLight = bVar1;
			break;
		case 3:
			gShadowCast = (short)value;
			break;
		case 4:
			gShadowReceive = (short)value;
			break;
		case 5:
			IMPLEMENTATION_GUARD(
			gShapeSize = value;)
			break;
		case 6:
			gSubdivideLevel = value;
		}
	}
	return;
}

edF32VECTOR3 Vector3_0048d390 = { };

float FLOAT_ARRAY_004253a0[12] = {
	1.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 1.0f
};

float edDListMatrixGetBigerScale(edF32MATRIX4* m0)
{
	float fVar1;
	float fVar2;
	float fVar3;
	int iVar4;
	edF32VECTOR4* peVar5;
	float* pfVar6;
	edF32VECTOR4* v0;
	edF32VECTOR4 local_30[3];

	pfVar6 = FLOAT_ARRAY_004253a0;
	iVar4 = 3;
	peVar5 = local_30;
	do {
		fVar1 = pfVar6[1];
		fVar2 = pfVar6[2];
		fVar3 = pfVar6[3];
		iVar4 = iVar4 + -1;
		peVar5->x = *pfVar6;
		peVar5->y = fVar1;
		peVar5->z = fVar2;
		peVar5->w = fVar3;
		pfVar6 = pfVar6 + 4;
		peVar5 = peVar5 + 1;
	} while (0 < iVar4);
	edF32Matrix4MulF32Vector4Hard(local_30, m0, local_30);
	v0 = local_30 + 1;
	edF32Matrix4MulF32Vector4Hard(v0, m0, v0);
	peVar5 = local_30 + 2;
	edF32Matrix4MulF32Vector4Hard(peVar5, m0, peVar5);
	edF32Vector4SquareHard(local_30, local_30);
	edF32Vector4SquareHard(v0, v0);
	edF32Vector4SquareHard(peVar5, peVar5);
	fVar2 = local_30[0].z + local_30[0].x + local_30[0].y;
	fVar3 = local_30[1].z + local_30[1].x + local_30[1].y;
	fVar1 = local_30[2].z + local_30[2].x + local_30[2].y;
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

void edDListFindBoundingSphere(edF32VECTOR4* pSphere, uint param_2, ed_Bound_Sphere_packet* pPkt)
{
	edF32VECTOR4* peVar1;
	float* pfVar2;
	float* pfVar3;
	uint uVar4;
	float fVar5;
	float fVar6;
	float local_80;
	float fStack124;
	float local_78;
	float local_70;
	float fStack108;
	float local_68;
	float local_60;
	float fStack92;
	float local_58;
	float local_50;
	float fStack76;
	float local_48;
	float local_40;
	float fStack60;
	float local_38;
	float local_30;
	float fStack44;
	float local_28;
	float local_20;
	float fStack28;
	float local_18;
	float local_10;
	float fStack12;
	float local_8;

	local_48 = 1e+09f;
	fStack44 = 1e+09f;
	local_10 = 1e+09f;
	local_58 = -1e+09f;
	fStack60 = -1e+09f;
	local_20 = -1e+09f;
	for (uVar4 = 0; uVar4 < param_2; uVar4 = uVar4 + 1) {
		peVar1 = pSphere + uVar4;
		if (peVar1->x < local_10) {
			fStack12 = peVar1->y;
			local_8 = peVar1->z;
			local_10 = peVar1->x;
		}
		if (local_20 < peVar1->x) {
			fStack28 = peVar1->y;
			local_18 = peVar1->z;
			local_20 = peVar1->x;
		}
		pfVar3 = &peVar1->y;
		if (peVar1->y < fStack44) {
			local_30 = peVar1->x;
			fStack44 = *pfVar3;
			local_28 = peVar1->z;
		}
		if (fStack60 < *pfVar3) {
			local_40 = peVar1->x;
			fStack60 = *pfVar3;
			local_38 = peVar1->z;
		}
		pfVar2 = &peVar1->z;
		if (peVar1->z < local_48) {
			local_50 = peVar1->x;
			fStack76 = *pfVar3;
			local_48 = *pfVar2;
		}
		if (local_58 < *pfVar2) {
			local_60 = peVar1->x;
			fStack92 = *pfVar3;
			local_58 = *pfVar2;
		}
	}
	local_70 = local_10;
	fStack108 = fStack12;
	local_68 = local_8;
	local_80 = local_20;
	fStack124 = fStack28;
	local_78 = local_18;
	fVar6 = (local_18 - local_8) * (local_18 - local_8) +
		(local_20 - local_10) * (local_20 - local_10) + (fStack28 - fStack12) * (fStack28 - fStack12);
	fVar5 = (local_38 - local_28) * (local_38 - local_28) +
		(local_40 - local_30) * (local_40 - local_30) + (fStack60 - fStack44) * (fStack60 - fStack44);
	if (fVar6 < fVar5) {
		local_70 = local_30;
		fStack108 = fStack44;
		local_68 = local_28;
		local_80 = local_40;
		fStack124 = fStack60;
		local_78 = local_38;
		fVar6 = fVar5;
	}
	if (fVar6 < (local_58 - local_48) * (local_58 - local_48) +
		(local_60 - local_50) * (local_60 - local_50) + (fStack92 - fStack76) * (fStack92 - fStack76)) {
		local_70 = local_50;
		fStack108 = fStack76;
		local_68 = local_48;
		local_80 = local_60;
		fStack124 = fStack92;
		local_78 = local_58;
	}
	(pPkt->field_0x0).x = (local_70 + local_80) / 2.0f;
	(pPkt->field_0x0).y = (fStack108 + fStack124) / 2.0f;
	(pPkt->field_0x0).z = (local_68 + local_78) / 2.0f;
	local_80 = local_80 - (pPkt->field_0x0).x;
	fStack124 = fStack124 - (pPkt->field_0x0).y;
	local_78 = local_78 - (pPkt->field_0x0).z;
	fVar6 = sqrtf(local_78 * local_78 + local_80 * local_80 + fStack124 * fStack124) + 1.0f;
	static float radiusMul = 15.0f;
	fVar6 = fVar6 + radiusMul * fVar6;
	if (fVar6 < 2.147484e+09f) {
		pPkt->field_0xc = (ushort)(int)fVar6;
	}
	else {
		pPkt->field_0xc = (ushort)(int)(fVar6 - 2.147484e+09f);
	}
	edDListMatrixGetBigerScale(gCurMatrix + gNbMatrix);
	return;
}

edpkt_data* edDListStripPreparePacket(ed_3d_strip* pStrip, edpkt_data* pPkt)
{
	bool bVar1;
	byte bVar2;
	uint uVar3;
	ulong uVar4;
	ulong uVar5;
	edVertex* pVertexBuf;
	_rgba* pColorBuf;
	char* pSTBuf;
	int meshCount;
	uint remainderCnt;

	pColorBuf = (_rgba*)LOAD_SECTION(pStrip->pColorBuf);
	pVertexBuf = (edVertex*)LOAD_SECTION(pStrip->pVertexBuf);
	meshCount = (ushort)pStrip->meshCount - 1;
	pSTBuf = (char*)LOAD_SECTION(pStrip->pSTBuf) + 0x10;

	if ((gCurMaterial == (edDList_material*)0x0) && (gbForceMaterialUse == false)) {
		while (bVar1 = meshCount != 0, meshCount = meshCount + -1, bVar1) {
			memcpy(pPkt, &gPKTStrip, sizeof(gPKTStrip));

			pPkt[1].asU32[1] = STORE_SECTION(&g_stGifTAG_Texture_NoFog[72]);

			pPkt[3].asU32[1] = STORE_SECTION(pSTBuf); // ST
			pPkt[5].asU32[1] = STORE_SECTION(pColorBuf); // RGBA
			pPkt[7].asU32[1] = STORE_SECTION(pVertexBuf); // VTX

			pSTBuf = pSTBuf + 0x120;
			pColorBuf = pColorBuf + 0x48;
			pVertexBuf = pVertexBuf + 0x46;
			pPkt = pPkt + 10;
		}
		bVar2 = pStrip->field_0x38;
		remainderCnt = (uint)bVar2;
		while (true) {
			uVar3 = remainderCnt & 3;
			if (((int)remainderCnt < 0) && (uVar3 != 0)) {
				uVar3 = uVar3 - 4;
			}
			if (uVar3 == 0) break;
			remainderCnt = remainderCnt + 1;
		}

		// Fill in the remainder of the data (the last block that's bigger than 0x48.
		memcpy(pPkt, &gPKTStrip, sizeof(gPKTStrip));

		pPkt[1].asU32[1] = STORE_SECTION(g_stGifTAG_Gouraud_NoFog + bVar2);

		uint pktTagRef = ED_VIF1_SET_TAG_REF(remainderCnt >> 2, 0);

		pPkt[3].asU32[0] = pktTagRef;
		pPkt[3].asU32[1] = STORE_SECTION(pSTBuf);
		pPkt[3].asU32[2] = SCE_VIF1_SET_NOP(0);
		pPkt[3].asU32[3] = SCE_VIF1_SET_UNPACK(0x8001, remainderCnt, UNPACK_V2_16_MASKED, 0);

		pPkt[5].asU32[0] = pktTagRef;
		pPkt[5].asU32[1] = STORE_SECTION(pColorBuf);
		pPkt[5].asU32[2] = SCE_VIF1_SET_NOP(0);
		pPkt[5].asU32[3] = SCE_VIF1_SET_UNPACK(0xc002, remainderCnt, UNPACK_V4_8_MASKED, 0);

		pPkt[7].asU32[0] = ED_VIF1_SET_TAG_REF(remainderCnt, 0);
		pPkt[7].asU32[1] = STORE_SECTION(pVertexBuf);
		pPkt[7].asU32[2] = SCE_VIF1_SET_NOP(0);
		pPkt[7].asU32[3] = SCE_VIF1_SET_UNPACK(0x8003, remainderCnt, UNPACK_V4_32_MASKED, 0);
	}
	else {
		gbForceMaterialUse = false;
		while (bVar1 = meshCount != 0, meshCount = meshCount + -1, bVar1) {
			// Fill in the strip packet and replace the PKT_STRIP_ADDR_MARKER with the addr of the actual data.

			memcpy(pPkt, &gPKTStrip, sizeof(gPKTStrip));

			pPkt[3].asU32[1] = STORE_SECTION(pSTBuf); // ST
			pPkt[5].asU32[1] = STORE_SECTION(pColorBuf); // RGBA
			pPkt[7].asU32[1] = STORE_SECTION(pVertexBuf); // VTX

			pSTBuf = pSTBuf + 0x120;
			pColorBuf = pColorBuf + 0x48;
			pVertexBuf = pVertexBuf + 0x46;
			pPkt = pPkt + 10;
		}
		bVar2 = pStrip->field_0x38;
		remainderCnt = (uint)bVar2;
		while (true) {
			uVar3 = remainderCnt & 3;
			if (((int)remainderCnt < 0) && (uVar3 != 0)) {
				uVar3 = uVar3 - 4;
			}
			if (uVar3 == 0) break;
			remainderCnt = remainderCnt + 1;
		}

		// Fill in the remainder of the data (the last block that's bigger than 0x48.
		memcpy(pPkt, &gPKTStrip, sizeof(gPKTStrip));

		pPkt[1].asU32[1] = STORE_SECTION(g_stGifTAG_Texture_NoFog + bVar2);

		uint pktTagRef = ED_VIF1_SET_TAG_REF(remainderCnt >> 2, 0);

		pPkt[3].asU32[0] = pktTagRef;
		pPkt[3].asU32[1] = STORE_SECTION(pSTBuf);
		pPkt[3].asU32[2] = SCE_VIF1_SET_NOP(0);
		pPkt[3].asU32[3] = SCE_VIF1_SET_UNPACK(0x8001, remainderCnt, UNPACK_V2_16_MASKED, 0);

		pPkt[5].asU32[0] = pktTagRef;
		pPkt[5].asU32[1] = STORE_SECTION(pColorBuf);
		pPkt[5].asU32[2] = SCE_VIF1_SET_NOP(0);
		pPkt[5].asU32[3] = SCE_VIF1_SET_UNPACK(0xc002, remainderCnt, UNPACK_V4_8_MASKED, 0);

		pPkt[7].asU32[0] = ED_VIF1_SET_TAG_REF(remainderCnt, 0);
		pPkt[7].asU32[1] = STORE_SECTION(pVertexBuf);
		pPkt[7].asU32[2] = SCE_VIF1_SET_NOP(0);
		pPkt[7].asU32[3] = SCE_VIF1_SET_UNPACK(0x8003, remainderCnt, UNPACK_V4_32_MASKED, 0);
	}

	return pPkt + 10;
}

void edDlistCopyInPatchableStrip(ed_3d_strip* pStrip)
{
	ed_3d_strip* pStrip_00;
	edVertex* peVar1;
	edpkt_data* peVar2;
	uint uVar3;
	DisplayListCommand* pDVar4;
	DisplayList* pDVar5;

	pDVar5 = gCurDListHandle + gCurFlushState;
	pDVar4 = pDVar5->aCommands + (pDVar5->nbCommands - 1);
	pStrip_00 = pDVar4->pRenderInput.pStrip;
	pDVar4->pCurDListBuf = (edpkt_data*)((char*)pStrip_00 + pStrip_00->vifListOffset);
	pStrip_00->meshCount = pStrip->meshCount;

	assert(pStrip_00->meshCount > 0);

	pStrip_00->field_0x38 = pStrip->field_0x38;
	peVar1 = (edVertex*)LOAD_SECTION(pStrip->pVertexBuf);
	uVar3 = 0;
	while (uVar3 < (ushort)pStrip->meshCount) {
		ed_Bound_Sphere_packet* pSpherePkt = (ed_Bound_Sphere_packet*)LOAD_SECTION(pStrip->pBoundSpherePkt) + uVar3;
		if (uVar3 == (ushort)pStrip_00->meshCount - 1) {
			edDListFindBoundingSphere((edF32VECTOR4*)(peVar1 + uVar3 * 0x46), (uint)pStrip_00->field_0x38, pSpherePkt);
			uVar3 = uVar3 + 1;
		}
		else {
			edDListFindBoundingSphere((edF32VECTOR4*)(peVar1 + uVar3 * 0x46), 0x48, pSpherePkt);
			uVar3 = uVar3 + 1;
		}
	}
	memcpy(pStrip_00 + 1, pStrip + 1, pStrip_00->vifListOffset + -0x40);
	peVar2 = edDListStripPreparePacket(pStrip_00, pDVar4->pCurDListBuf);
	pDVar5->pRenderCommands = peVar2;
	pDVar5->field_0x10 = pDVar5->pRenderCommands;
	return;
}

void edDListEndStrip(ed_3d_strip* pStrip)
{
	edVertex* peVar1;
	int iVar2;
	int iVar3;
	uint uVar4;

	for (iVar3 = 0; (iVar3 + 1) * 0x46 + 2U < (uint)gNbAddedVertex; iVar3 = iVar3 + 1) {
	}

	iVar2 = gNbAddedVertex + iVar3 * -0x46;
	if (iVar2 == 0) {
		iVar2 = 0x48;
	}
	else {
		iVar3 = iVar3 + 1;
	}

	pStrip->meshCount = (short)iVar3;

	assert(pStrip->meshCount > 0);

	if (iVar3 == 0) {
		pStrip->field_0x38 = 0;
	}
	else {
		pStrip->field_0x38 = (byte)iVar2;
	}

	if (((gCurDList->flags_0x0 & 0x100) != 0) && (gCurStripPatchable == 0)) {
		peVar1 = (edVertex*)LOAD_SECTION(pStrip->pVertexBuf);
		uVar4 = 0;
		while (uVar4 < (ushort)pStrip->meshCount) {
			ed_Bound_Sphere_packet* pSpherePkt = (ed_Bound_Sphere_packet*)LOAD_SECTION(pStrip->pBoundSpherePkt) + uVar4;
			if (uVar4 == (ushort)pStrip->meshCount - 1) {
				edDListFindBoundingSphere((edF32VECTOR4*)(peVar1 + uVar4 * 0x46), (uint)pStrip->field_0x38, pSpherePkt);
				uVar4 = uVar4 + 1;
			}
			else {
				edDListFindBoundingSphere((edF32VECTOR4*)(peVar1 + uVar4 * 0x46), 0x48, pSpherePkt);
				uVar4 = uVar4 + 1;
			}
		}
		edDlistCopyInPatchableStrip(pStrip);
	}

	DISPLAY_LIST_3D_END();

	return;
}

void edDListEnd(void)
{
	ushort dataType;
	DisplayListCommand* pDVar5;

	if (gbInsideBegin != false) {
		if ((gCurDList->flags_0x0 & 1) == 0) {
			if ((gCurDList->flags_0x0 & 2) != 0) {
				// Update NLOOP of the header packet, based on how many commands were added to the display list.
				const int nloop = ((static_cast<int>(reinterpret_cast<char*>(gCurDList->pRenderCommands) - reinterpret_cast<char*>(gCurStatePKT))) >> 4) - 1;
				const uint existingPktData = gCurStatePKT->asU32[0];
				gCurStatePKT->asU32[0] = 0;
				gCurStatePKT->asU32[0] = existingPktData & 0xffff8000 | nloop & 0x7fff;

				DUMP_TAG_ADV(gCurStatePKT->cmdA);

				DISPLAY_LIST_2D_END();
			}
		}
		else {
			if (gCurDList->nbCommands != 0) {
				if (gCurPrimType == 10) {
					edDListVertex4f(Vector3_0048d390.x, Vector3_0048d390.y, Vector3_0048d390.z, 10);
				}

				gNbVertexDMA = 0x48;
				gCurStatePKTSize = 0;
				gCurStatePKT = (edpkt_data*)0x0;
				pDVar5 = gCurDList->aCommands + (gCurDList->nbCommands - 1);
				pDVar5->pCurDListBuf = (edpkt_data*)gCurDListBuf;

				dataType = pDVar5->dataType;
				if (dataType != DISPLAY_LIST_DATA_TYPE_PKT) {
					if ((((dataType == 0xc) || (dataType == 0xb)) || (dataType == 7)) || (dataType == 6)) {
						if (gNbAddedVertex < 1) {
							gCurDList->nbCommands = gCurDList->nbCommands - 1;
							gCurDListBuf = pDVar5->pRenderInput.pStrip;
							assert(pDVar5->pRenderInput.pStrip->meshCount > 0);
						}
						else {
							IMPLEMENTATION_GUARD(
							edDListEndSprite(pDVar5->pRenderInput.pSprite);
							gCurDListBuf = ed3DSpritePreparePacket(pDVar5->pRenderInput.pSprite, (undefined4*)gCurDListBuf, (int)gBankMaterial, (ulong)pDVar5->dataType);)
						}
					}
					else {
						if (((dataType == 4) || (dataType == 2)) || (dataType == 0)) {
							if (gNbAddedVertex < 2) {
								gCurDList->nbCommands = gCurDList->nbCommands - 1;
								gCurDListBuf = pDVar5->pRenderInput.pStrip;
							}
							else {
								pDVar5->nbAddedVertex = gNbAddedVertex;
								edDListEndStrip(pDVar5->pRenderInput.pStrip);
								gCurDListBuf = (ed_3d_strip*)edDListStripPreparePacket(pDVar5->pRenderInput.pStrip, (edpkt_data*)gCurDListBuf);
							}
						}
					}
				}

				gCurDList->pRenderCommands = (edpkt_data*)gCurDListBuf;
			}
		}

		gNbAddedVertex = 0;
		gMaxNbVertex = 0;
		gCurStatePKT = (edpkt_data*)0x0;
		gbInsideBegin = false;
		gAddVertexFUNC = (DisplayListXYZFunc)0x0;
		gAddColorFUNC = (DisplayListRGBAQFunc)0x0;
	}

	return;
}

void edDListPatchableReset(DisplayList* pList, uint param_2, uint param_3, uint param_4)
{
	ed_3D_Scene* pStaticMeshMaster;
	DisplayList* pDVar1;
	uint uVar2;

	if (param_2 != 0xffffffff) {
		uVar2 = 0;

		if (param_4 == 0xffffffff) goto LAB_002d0e38;

		if (param_3 != 0xffffffff) {
			pStaticMeshMaster = pList->pScene;
			edDListNew(TO_HEAP(H_MAIN), (uint)pList->flags_0x0, param_2, param_3, param_4, 0, pList);
			edDListSetSceneUsed(pList, pStaticMeshMaster);
			return;
		}
	}

	uVar2 = 0;

LAB_002d0e38:
	do {
		pDVar1 = pList + uVar2;
		pDVar1->nbCommands = 0;

		if ((pDVar1->flags_0x0 & 1) != 0) {
			pDVar1->field_0x10 = (edpkt_data*)pDVar1->pCommandBuffer;
			pDVar1->field_0x14 = (edpkt_data*)pDVar1->pCommandBuffer;
		}

		pDVar1->nbSavedCommands = 0;
		uVar2 = uVar2 + 1;
		pDVar1->pRenderCommands = pDVar1->field_0x14;
	} while (uVar2 < 2);

	return;
}

void edDListLoadIdentity(void)
{
	EDDLIST_LOG(LogLevel::Verbose, "edDListLoadIdentity gNbMatrix: {}", gNbMatrix);
	edF32Matrix4SetIdentityHard((gCurMatrix + gNbMatrix));
	return;
}

void edDlistSetUseUV(int newUseUV)
{
	edDlistUseUV = newUseUV;

	return;
}

DisplayList* gCurDListHandle = NULL;
ed_3d_strip* gCurDListBuf = NULL;
ed_3d_strip* gCurDListBufEnd = NULL;

DisplayList* edDListSetCurrent(DisplayList* pNewDisplayList)
{
	DisplayList* pDVar1;
	DisplayList* pPreviousDList;

	pPreviousDList = gCurDListHandle;
	gCurInfPosForBegin = 0xffffffff;
	gCurStripPatchable = 0;
	gCurSpritePatchable = 0;
	gCurColorNbInVertex = 0;
	pDVar1 = pNewDisplayList + gCurRenderState;
	//memset(BYTE_ARRAY_0048d3a0, 0, 0x100);
	//DAT_00448a84 = 0xffffffff;
	if ((pDVar1->flags_0x0 & 4) != 0) {
		pDVar1 = pNewDisplayList;
	}
	gCurDListHandle = pNewDisplayList;
	if (gCurDList != (DisplayList*)0x0) {
		gCurDList->field_0x10 = (edpkt_data*)gCurDListBuf;
	}
	if ((pDVar1->flags_0x0 & 1) != 0) {
		gCurDListBuf = (ed_3d_strip*)pDVar1->field_0x10;
		if (((ulong)gCurDListBuf & 0xf) != 0) {
			IMPLEMENTATION_GUARD(
			gCurDListBuf = (edpkt_data*)((int)gCurDListBuf + (sizeof(edpkt_data) - ((uint)gCurDListBuf & 0xf)));)
		}
	}
	gNbStateAdded = 0;
	gCurDList = pDVar1;
	gCurDListBufEnd = (ed_3d_strip*)((ulong)pDVar1->field_0x14 + (int)((ulong)pNewDisplayList[1].field_0x14 - (ulong)pNewDisplayList->field_0x14));
	gCurMaterial = (edDList_material*)0x0;
	gCurStatePKT = (edpkt_data*)0x0;
	gCurStatePKTSize = 0;
	gShadowReceive = 0;
	return pPreviousDList;
}

void edDListSetBuffer(DisplayList* aDisplayLists, uint* pDataBuf, uint dataBufferSize, ushort flags, void* pCommandBuf, uint commandBufferSize, uint nbMaxCommands, uint bAllScenes)
{
	int iVar1;
	DisplayList* pCurDisplayList;
	uint dblBufferIndex;
	uint alignOffset;

	dblBufferIndex = 0;
	do {
		pCurDisplayList = aDisplayLists + dblBufferIndex;

		pCurDisplayList->flags_0x0 = flags;
		pCurDisplayList->nbCommands = 0;
		pCurDisplayList->nbMaxCommands = nbMaxCommands;
		pCurDisplayList->pScene = (ed_3D_Scene*)bAllScenes;
		pCurDisplayList->aCommands = reinterpret_cast<DisplayListCommand*>(reinterpret_cast<char*>(pCommandBuf) + (commandBufferSize >> 1) * dblBufferIndex);

		// Need to align to 16 bytes.
		alignOffset = (uint)pCurDisplayList->aCommands & 0xf;
		if (alignOffset != 0) {
			pCurDisplayList->aCommands = reinterpret_cast<DisplayListCommand*>(reinterpret_cast<char*>(pCurDisplayList->aCommands) + (0x10 - alignOffset));
		}

		if ((flags & 2) == 0) {
			if ((flags & 1) != 0) {
				iVar1 = (dataBufferSize - (ulong)((ulong)(pDataBuf + 400) - (ulong)pDataBuf) >> 1) * dblBufferIndex;

				pCurDisplayList->field_0x14 = (edpkt_data*)(pDataBuf + (dblBufferIndex * 200));
				pCurDisplayList->pCommandBuffer = (char*)((ulong)(pDataBuf + 400) + iVar1);
				pCurDisplayList->field_0x10 = (edpkt_data*)pCurDisplayList->pCommandBuffer;
				pCurDisplayList->field_0x28 = pCurDisplayList->pCommandBuffer + iVar1;
				pCurDisplayList->field_0x14 = (edpkt_data*)pCurDisplayList->pCommandBuffer;

				alignOffset = (uint)pCurDisplayList->pCommandBuffer & 0xf;
				if (alignOffset != 0) {
					pCurDisplayList->pCommandBuffer = pCurDisplayList->pCommandBuffer + (0x10 - alignOffset);
				}

				alignOffset = (uint)pCurDisplayList->field_0x10 & 0xf;
				if (alignOffset != 0) {
					pCurDisplayList->field_0x10 = reinterpret_cast<edpkt_data*>(reinterpret_cast<char*>(pCurDisplayList->field_0x10) + (0x10 - alignOffset));
				}
			}
		}
		else {
			pCurDisplayList->field_0x14 = (edpkt_data*)((ulong)pDataBuf + (dataBufferSize >> 1) * dblBufferIndex);
		}

		alignOffset = (uint)pCurDisplayList->field_0x14 & 0xf;
		if (alignOffset != 0) {
			pCurDisplayList->field_0x14 = reinterpret_cast<edpkt_data*>(reinterpret_cast<char*>(pCurDisplayList->field_0x14) + (0x10 - alignOffset));
		}

		dblBufferIndex = dblBufferIndex + 1;
		pCurDisplayList->pRenderCommands = pCurDisplayList->field_0x14;
	} while (dblBufferIndex < 2);

	return;
}

DisplayList* edDListNew(EHeap heapID, uint flags, int nbCommands, int param_4, int nbMatrices, uint param_6, void* pInBuffer)
{
	int commandPtrSize;
	uint displayListMemSize;
	uint uVar2;
	DisplayList* pDVar3;
	uint dblBufferIndex;
	uint* pDataBuf;
	uint commandBufferSize;
	uint dataBufferSize;

	displayListMemSize = edDListGetBufSizeNeeded(flags, nbCommands, param_4, nbMatrices, &commandBufferSize, &dataBufferSize);

	if (pInBuffer == (DisplayList*)0x0) {
		pInBuffer = (DisplayList*)edMemAlloc(heapID, displayListMemSize);
	}

	DisplayList* pDisplayList = reinterpret_cast<DisplayList*>(pInBuffer);
	memset(pInBuffer, 0, displayListMemSize);
	char* pCommandBuf = reinterpret_cast<char*>(pDisplayList + 2); // 2 DisplayList objects for double buffering.

	pDataBuf = reinterpret_cast<uint*>(pCommandBuf + commandBufferSize);
	edDListSetBuffer(pDisplayList, pDataBuf, dataBufferSize, flags, reinterpret_cast<void*>(pCommandBuf), commandBufferSize, nbCommands, 0x1);

	assert(reinterpret_cast<char*>(pDisplayList[0].field_0x10) < reinterpret_cast<char*>(pInBuffer) + displayListMemSize);
	assert(reinterpret_cast<char*>(pDisplayList[1].field_0x10) < reinterpret_cast<char*>(pInBuffer) + displayListMemSize);

	assert(reinterpret_cast<char*>(pDisplayList[0].field_0x14) < reinterpret_cast<char*>(pInBuffer) + displayListMemSize);
	assert(reinterpret_cast<char*>(pDisplayList[1].field_0x14) < reinterpret_cast<char*>(pInBuffer) + displayListMemSize);

	dblBufferIndex = 0;
	if ((flags & DISPLAY_LIST_FLAG_SAVE_COMMANDS) == 0) {
		do {
			uVar2 = dblBufferIndex + 1;
			pDisplayList[dblBufferIndex].aSavedCommands = 0x0;
			pDisplayList[dblBufferIndex].nbSavedCommands = 0;
			dblBufferIndex = uVar2;
		} while (uVar2 < 2);
	}
	else {
		dblBufferIndex = 0;
		do {
			pDVar3 = pDisplayList + dblBufferIndex;
			commandPtrSize = dblBufferIndex * nbCommands * sizeof(DisplayListCommand*);
			dblBufferIndex = dblBufferIndex + 1;
			pDVar3->aSavedCommands = reinterpret_cast<DisplayListCommand**>(reinterpret_cast<char*>(pDataBuf) + commandPtrSize + dataBufferSize);
			pDVar3->nbSavedCommands = 0;
		} while (dblBufferIndex < 2);
	}

	return pDisplayList;
}

void edDListSetSceneUsed(DisplayList* pDisplayListInternalArray, ed_3D_Scene* pStaticMeshMaster)
{
	uint displayListIndex;
	uint uVar1;

	displayListIndex = 0;
	do {
		uVar1 = displayListIndex + 1;
		pDisplayListInternalArray[displayListIndex].pScene = pStaticMeshMaster;
		displayListIndex = uVar1;
	} while (uVar1 < 2);
	return;
}

uint edDListGetBufSizeNeeded(uint flags, int nbMaxCommands, int param_3, int nbMatrices, uint* pCommandBufferSize, uint* pDataBufferSize)
{
	int iVar1;
	uint totalSize;
	int commandBufferSize;
	int nbMaxMatrices;
	uint dataBufferSize;

	nbMaxMatrices = nbMatrices + (param_3 * 4);

	if (nbMaxCommands == 0) {
		trap(7);
	}

	iVar1 = ((nbMaxMatrices / nbMaxCommands) / 0x46 + 1) * nbMaxCommands;
	if ((flags & 2) == 0) {
		if ((flags & 1) == 0) {
			return 0;
		}

		commandBufferSize = nbMaxCommands * sizeof(DisplayListCommand) * 2;

		nbMaxMatrices = nbMaxMatrices * 0x20 + nbMaxCommands * 0x90 + 0x640 + iVar1 * 0xc0;

		if ((flags & 0x100) != 0) {
			nbMaxMatrices = nbMaxMatrices + iVar1 * 0x10;
		}

		if ((flags & 0x100) != 0) {
			nbMaxMatrices = nbMaxMatrices + iVar1 * 0x10;
		}

		dataBufferSize = (nbMaxMatrices + sizeof(edF32MATRIX4)) * 2;
	}
	else {
		dataBufferSize = nbMaxMatrices * sizeof(edF32MATRIX4);
		commandBufferSize = nbMaxCommands * sizeof(DisplayListCommand); // I think this is meant to be just 0x20 or the size of DisplayListCommand - matrix
	}

	totalSize = dataBufferSize + commandBufferSize + (sizeof(DisplayList) * 2);

	if ((flags & DISPLAY_LIST_FLAG_SAVE_COMMANDS) != 0) {
		totalSize = totalSize + nbMaxCommands * sizeof(DisplayListCommand*) * 2;
	}

	if (pCommandBufferSize != (uint*)0x0) {
		*pCommandBufferSize = commandBufferSize;
	}

	if (pDataBufferSize != (uint*)0x0) {
		*pDataBufferSize = dataBufferSize;
	}

	return totalSize;
}

#ifdef PLATFORM_WIN
Multidelegate<edDList_material*> onMaterialLoadedDelegate;
Multidelegate<edDList_material*> onMaterialUnloadedDelegate;

Multidelegate<edDList_material*>& edDListGetMaterialLoadedDelegate()
{
	return onMaterialLoadedDelegate;
}

Multidelegate<edDList_material*>& edDListGetMaterialUnloadedDelegate()
{
	return onMaterialUnloadedDelegate;
}
#endif

void edDListInitMaterial(edDList_material* pDlistMaterial, ed_hash_code* pHASH_MAT, ed_g2d_manager* pManager, uint mode)
{
	ed_hash_code* pNewHashCode;
	uint counter;

	counter = 0;
	memset(pDlistMaterial, 0, sizeof(edDList_material));

	ed_Chunck* pMAT = LOAD_SECTION_CAST(ed_Chunck*, pHASH_MAT->pData);

	pDlistMaterial->pMaterial = reinterpret_cast<ed_g2d_material*>(pMAT + 1);

	while (true) {
		if ((gBankMaterial[counter].hash.number == 0) || (edDlistConfig.bankMaterialCount - 1U <= counter)) break;
		counter = counter + 1;
	}

	pNewHashCode = gBankMaterial + counter;
	pNewHashCode->hash = pHASH_MAT->hash;
	pNewHashCode->pData = STORE_SECTION(pHASH_MAT);
	pDlistMaterial->index = counter;
	gNbUsedMaterial = gNbUsedMaterial + 1;
	pDlistMaterial->mode = mode;
	pDlistMaterial->pManager = pManager;

#ifdef PLATFORM_WIN
	onMaterialLoadedDelegate(pDlistMaterial);
#endif

	return;
}

bool edDListTermMaterial(edDList_material* pMaterial)
{
	if ((gBankMaterial[pMaterial->index].hash.number != 0) &&
		(gBankMaterial[pMaterial->index].pData != 0x0)) {
		gBankMaterial[pMaterial->index].hash = 0;
		gBankMaterial[pMaterial->index].pData = 0;
		gNbUsedMaterial = gNbUsedMaterial + -1;
	}

#ifdef PLATFORM_WIN
		onMaterialUnloadedDelegate(pMaterial);
#endif

	return true;
}

void edDListDeleteFrameBufferMaterial(edDList_material* pMaterial)
{
	if ((gBankMaterial[pMaterial->index].hash.number != 0) && (gBankMaterial[pMaterial->index].pData != 0x0)) {
		gBankMaterial[pMaterial->index].hash.number = 0;
		gBankMaterial[pMaterial->index].pData = 0x0;
		gNbUsedMaterial = gNbUsedMaterial + -1;
	}

	return;
}

edDList_material* edDListCreatMaterialFromIndex(edDList_material* pMaterialInfo, int index, ed_g2d_manager* pTextureInfo, uint mode)
{
	ed_hash_code* pHASH_MAT;

	EDDLIST_LOG(LogLevel::Verbose, "edDListCreatMaterialFromIndex {}", index);

	pHASH_MAT = ed3DG2DGetMaterialFromIndex(pTextureInfo, index);
	if (pHASH_MAT == (ed_hash_code*)0x0) {
		pMaterialInfo = (edDList_material*)0x0;
	}
	else {
		edDListInitMaterial(pMaterialInfo, pHASH_MAT, pTextureInfo, mode);
	}
	return pMaterialInfo;
}

edDList_material* edDListCreatMaterialFromHashCode(edDList_material* pMaterial, ulong hashCode, ed_g2d_manager* pManager, uint mode)
{
	ed_hash_code* pHASH_MAT = ed3DG2DGetMaterial(pManager, hashCode);

	if (pHASH_MAT == (ed_hash_code*)0x0) {
		pMaterial = (edDList_material*)0x0;
	}
	else {
		edDListInitMaterial(pMaterial, pHASH_MAT, pManager, mode);
	}

	return pMaterial;
}

void edDListCreateFrameBufferMaterial(edDList_material* pMaterial)
{
	ed_hash_code* pBankHashes;
	uint freeIndex;

	memset(pMaterial, 0, sizeof(edDList_material));

	pBankHashes = gBankMaterial;

	for (freeIndex = 0; (gBankMaterial[freeIndex].hash.number != 0 && (freeIndex < edDlistConfig.bankMaterialCount - 1U)); freeIndex = freeIndex + 1) {
	}

	pBankHashes[freeIndex].hash = ed3DComputeHashCode("FRAMEBUF");
	pBankHashes[freeIndex].pData = STORE_SECTION(gFrameBufHASHPtr);
	pMaterial->index = freeIndex;

	gNbUsedMaterial = gNbUsedMaterial + 1;

	pMaterial->mode = 4;
	pMaterial->pManager = (ed_g2d_manager*)0x0;
	pMaterial->pMaterial = gFrameBufMaterialPtr;

	return;
}
