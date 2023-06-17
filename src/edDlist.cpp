#include "edDlist.h"
#include "edMem.h"
#include <string.h>
#include <stdio.h>
#include <assert.h>

#ifdef PLATFORM_PS2
#include <libvu0.h>
#include <libdma.h>
#endif

#include "ed3D.h"
#include "Rendering/DisplayList.h"
#include "LargeObject.h"
#include "edVideo/VideoA.h"
#include "edVideo/VideoB.h"
#include "Camera.h"
#include "edSystem.h"
#include "edVideo/VideoD.h"
#include "port/pointer_conv.h"

#include "Rendering/CustomShell.h"
#include "MathOps.h"

edLIST* gDList_2D[2] = { 0 };
edLIST* gDList_2D_Before3D[2] = { 0 };
DisplayListArray* gDList_3D[2] = { 0 };
int gNbDList_3D[2] = { 0 };

char* gDLISTWorkingBufferStart;
char* gDLISTWorkingBufferCur;
char* gDLISTWorkingBufferEnd;

uint gBlendMode = 0;

int gCurColorNbInVertex = 0;
int gEndColorNbInVertex = 0;

MeshDrawRenderCommand* gEndVertexBuf = NULL;

int gNbMatrix;
edF32MATRIX4* gCurMatrix;

int gIncViewportX = 0;
int gIncViewportY = 0;
ed_viewport* gCurViewport = NULL;

DisplayList_0x10* gBankMaterial;

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

typedef void (*DisplayListXYZFunc)(float, float, float, int);
typedef void (*DisplayListRGBAQFunc)(byte, byte, byte, byte);
typedef void (*DisplayListSTFunc)(float, float);

DisplayListXYZFunc gAddVertexFUNC = NULL;
DisplayListRGBAQFunc gAddColorFUNC = NULL;
DisplayListSTFunc gAddSTFUNC = NULL;

ByteColor* gCurColorBuf = NULL;
char* gCurSTBuf = NULL;
char* gEndSTBuf = NULL;
Vector* gCurVertexBuf = NULL;
undefined* gStartVertexBuf = NULL;

int gCurFlashMaterial = 0;
edDList_material* gCurMaterial = NULL;
DisplayListInternal* gCurDList = NULL;


int g_DisplayListSizeA_004250dc = 0x3E80;
int g_DisplayListSizeBCount_004250d0 = 0x5;
int g_DisplayListMatrixCount_004250d8 = 0xA;
int g_DisplayListObjCount_004250e0 = 0x12C;

byte DAT_00449660 = 0;
	
byte BYTE_004496e4 = 0;
int g_Count_004495f8 = 0;

int Mode_00449694 = 0;

ByteColor ByteColor_ARRAY_0048dd40[4];

ByteColor* g_RGBAQ_00448aa0 = ByteColor_ARRAY_0048dd40;

edSysHandlerDisplayList sysHandler_0048cb90 = edSysHandlerDisplayList(&g_SysHandlersNodeTable_00489170, 5, 10);

void edDListInitMemory(void)
{
	uint size;

	if ((g_DisplayListSizeA_004250dc & 0xfU) != 0) {
		g_DisplayListSizeA_004250dc = g_DisplayListSizeA_004250dc + (0x10 - (g_DisplayListSizeA_004250dc & 0xfU));
	}
	if ((g_DisplayListSizeBCount_004250d0 & 3U) != 0) {
		g_DisplayListSizeBCount_004250d0 = g_DisplayListSizeBCount_004250d0 + (4 - (g_DisplayListSizeBCount_004250d0 & 3U));
	}
	size = g_DisplayListSizeA_004250dc + 0x640 + g_DisplayListMatrixCount_004250d8 * 0x40 + g_DisplayListSizeBCount_004250d0 * 8 + g_DisplayListObjCount_004250e0 * sizeof(DisplayList_0x10);
	if ((size & 0xf) != 0) {
		size = size + (0x10 - (size & 0xf));
	}

	// #HACK
	size = size * 4;

	gDLISTWorkingBufferStart = (char*)edMemAllocAlignBoundary(TO_HEAP(H_MAIN), size);
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

edpkt_data* edDListRecurseRefTag(DisplayListInternal* pDisplayList, edpkt_data* pCommandBuffer)
{
	uint uVar1;
	ulonglong uVar2;
	RenderCommandUint* pRVar3;
	uint uVar4;

	uVar4 = 0;
LAB_002ce9d4:
	do {
		if (pDisplayList->subCommandBufferCount <= uVar4) {
			return pCommandBuffer;
		}
		pRVar3 = pDisplayList->pDisplayListInternalSubObj->aCommandArray + uVar4;
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
					IMPLEMENTATION_GUARD(
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

byte g_Render_UI_00448a8c = 1;
void edDListSend2DList(edLIST* pHeader)
{
	DisplayListInternal* pDisplayList;
	DisplayListInternalSubObj_60* pDVar1;
	edpkt_data* pCommandBuffer;
	RenderCommandUint* pRVar2;
	edpkt_data* pBuffer;
	MeshTransformSpecial* pCVar7;

	pBuffer = (gPKTDListFlush2DStart +
		(gCurFlushState * ((uint)g_DisplayListSizeA_004250dc >> 4) >> 1));
	pCommandBuffer = pBuffer;
	for (pCVar7 = pHeader->pLoadedData; (edLIST*)pCVar7 != pHeader; pCVar7 = pCVar7->pNext_0x4) {
		pDisplayList = pCVar7->pRenderInput;
		if ((gbDispList != 0) && (pDisplayList->pRenderCommands != pDisplayList->field_0x14)) {
			pRVar2 = pDisplayList->pDisplayListInternalSubObj->aCommandArray + (pDisplayList->subCommandBufferCount - 1);
			if (pRVar2->type == LM_REF_0) {
				pRVar2->size = (uint)((int)pDisplayList->pRenderCommands - (int)pRVar2->pCommandBuffer);
			}
			pCommandBuffer = edDListRecurseRefTag(pDisplayList, pCommandBuffer);
		}
		if ((pDisplayList->flags_0x0 & 0x10) != 0) {
			pDVar1 = pDisplayList->pDisplayListInternalSubObj;
			pDisplayList->pRenderCommands = pDisplayList->field_0x14;
			pDVar1->aCommandArray[0].pCommandBuffer = pDisplayList->field_0x14;
			pDVar1->aCommandArray[0].type = LM_REF_0;
			pDisplayList->subCommandBufferCount = 0;
		}
		pDisplayList->field_0x3 = 0;
	}
	if ((pBuffer != pCommandBuffer) && (gbDispList != 0)) {
		pCommandBuffer->cmdB = 0x0;
		sceDmaAddEnd((sceDmaTag**)&pCommandBuffer, 0, 0);
		edDmaFlushCache();
		edDmaSyncPath();
		edDmaSyncPath();
		if (g_Render_UI_00448a8c != 0) {
			RENDER_LOG("DMA Begin RenderUI_002cea00\n");
			edDmaSend_nowait(SHELLDMA_CHANNEL_GIF, (ulonglong*)pBuffer);
		}
		edDmaSyncPath();

	}
	edListClear(pHeader);
	return;
}

struct RenderTaskData {
	struct ed_3D_Scene* field_0x0;
	int field_0x4;
	ushort taskID;
};

void edDListSend3DList(RenderTaskData* pRenderTaskData)
{
	ushort uVar1;
	bool bVar2;
	uint uVar3;
	edF32MATRIX4* pMVar4;
	DisplayListInternalSubObj_60* pDVar5;
	uint displayListIndex;
	uint uVar6;
	uint uVar7;
	DisplayListInternalSubObj_60* pDVar8;
	edF32MATRIX4* pMVar9;
	DisplayListArray* pDisplayListArray;
	DisplayListInternal* pDisplayListInternal;

	pDisplayListArray = gDList_3D[gCurRenderState];
	//DAT_00449688 = 0;
	if (gbDispList != 0) {
		for (displayListIndex = 0;
			displayListIndex <
			gNbDList_3D[gCurRenderState];
			displayListIndex = displayListIndex + 1) {
			pDisplayListInternal =
				((DisplayListArray*)
					((int)pDisplayListArray +
						((gNbDList_3D[gCurRenderState] - 1) -
							displayListIndex) * 4))->field_0x0[0];
			if (((BYTE_00448a5c == 0) ||
				(((pRenderTaskData->taskID & 1) != 0 && ((pDisplayListInternal->flags_0x0 & 0x40) != 0)))) ||
				(((pRenderTaskData->taskID & 2) != 0 && ((pDisplayListInternal->flags_0x0 & 0x40) == 0)))) {
				pMVar9 = (edF32MATRIX4*)0x0;
				if ((pDisplayListInternal->pStaticMeshMaster_0x20 == (ed_3D_Scene*)0x1) ||
					(pDisplayListInternal->pStaticMeshMaster_0x20 == pRenderTaskData->field_0x0)) {
					for (uVar6 = 0; uVar6 < pDisplayListInternal->subCommandBufferCount; uVar6 = uVar6 + 1) {
						bVar2 = true;
						uVar7 = 0;
						pDVar5 = pDisplayListInternal->pDisplayListInternalSubObj + uVar6;
						pDVar8 = pDVar5;
						pMVar4 = pMVar9;
						do {
							if (pDVar5->nbMatrix != 0) {
								pDVar8 = (DisplayListInternalSubObj_60*)(pDVar5->pCurMatrixArray + uVar7);
							}
							uVar1 = pDVar5->type_0x42;
							if (uVar1 == 9) {
								pMVar9 = (edF32MATRIX4*)pDVar5->pRenderInput;
							LAB_002ce7c4:
								uVar3 = pDVar5->nbMatrix;
							}
							else {
								if (uVar1 == 6) {
									if ((pRenderTaskData->field_0x4 != 1) && (pDVar5->field_0x40 != 0)) {
										IMPLEMENTATION_GUARD(
										ed_3d_strip::Func_002973c0
										((ed_3d_strip*)pDVar5->pRenderInput, (edF32MATRIX4*)pDVar8,
											(long)(int)gBankMaterial, pMVar4);
										pMVar4 = (edF32MATRIX4*)0x0;)
									}
									goto LAB_002ce7c4;
								}
								if (((uVar1 == 4) || (uVar1 == 2)) || (uVar1 == 0)) {
									if ((pDisplayListInternal->pStaticMeshMaster_0x20 != (ed_3D_Scene*)0x1) &&
										(((pRenderTaskData->field_0x4 == 1 && (*(short*)((int)&pDVar5->pRenderInput[3].cmdA + 2) == 0)) ||
											((pRenderTaskData->field_0x4 != 1 && (*(short*)((int)&pDVar5->pRenderInput[3].cmdA + 2) != 0)))))
										) {
										bVar2 = false;
									}
									if ((bVar2) && (pDVar5->field_0x40 != 0)) {
										IMPLEMENTATION_GUARD(
											ed3DLinkStripToViewport
										((ed_3d_strip*)pDVar5->pRenderInput, (edF32MATRIX4*)pDVar8,
											(long)(int)gBankMaterial, (float*)pMVar4);
										pMVar4 = (edF32MATRIX4*)0x0;)
									}
									goto LAB_002ce7c4;
								}
								uVar3 = pDVar5->nbMatrix;
							}
							uVar7 = uVar7 + 1;
						} while (uVar7 < uVar3);
					}
				}
			}
		}
	}
#ifdef PLATFORM_PS2
	iFlushCache(0);
#endif
	gCurDList = (DisplayListInternal*)0x0;
	return;
}

void edDListSwapList(void)
{
	DisplayListInternal* pDVar1;
	uint displayListIndex;
	DisplayListArray* pDisplayListArray;

	displayListIndex = 0;
	pDisplayListArray = gDList_3D[gCurFlushState];
	while (true) {
		if (gNbDList_3D[gCurFlushState] <= displayListIndex) break;
		pDVar1 = pDisplayListArray->field_0x0[displayListIndex];
		if (((pDVar1->flags_0x0 & 4) == 0) && ((pDVar1->flags_0x0 & 0x100) == 0)) {
			pDVar1->subCommandBufferCount = 0;
			pDVar1->pRenderCommands = (edpkt_data*)pDVar1->pCommandBuffer;
			pDVar1->field_0x10 = (MeshDrawRenderCommand*)pDVar1->pCommandBuffer;
		}
		pDVar1->field_0x3 = 0;
		displayListIndex = displayListIndex + 1;
		pDVar1->field_0x24 = 100.0;
	}
	gNbDList_3D[gCurFlushState] = 0;
	gCurFlushState = gCurRenderState;
	gCurRenderState = gCurRenderState + 1;
	if (gCurRenderState == 2) {
		gCurRenderState = 0;
	}
	gbDispList = 1;
	gCurDList = (DisplayListInternal*)0x0;
	return;
}

void edDListInterrupt(int callingID, int eventID, char* pRenderTaskData)
{
	if (callingID == 10) {
		if (eventID == 1) {
			edDListSend2DList(gDList_2D_Before3D[gCurFlushState]);
		}
		else {
			if (eventID == 0) {
				edDListSend3DList((RenderTaskData*)pRenderTaskData);
			}
		}
	}
	else {
		if (callingID == 2) {
			if (eventID == 10) {
				edDListSend2DList(gDList_2D[gCurRenderState]);
				DAT_00449660 = 1;
			}
			else {
				if ((eventID != 2) && (eventID == 0)) {
					edDListSwapList();
					DAT_00449660 = 0;
				}
			}
		}
	}
	return;
}

int FUN_002ce480(DisplayListInternal* param_1, DisplayListInternal* param_2)
{
	return (uint)param_1->field_0x2 - (uint)param_2->field_0x2;
}

const int g_DisplayListCount_004250d4 = 5;

int gbInitDone = 0;
int gEndSTNbInVertex = 0;
int gCurSTNbInVertex = 0;
int gMaxSTNbInVertex = 0;
undefined* gEndColorBuf = NULL;

void edDListInit(void)
{
	DisplayListArray* pDVar1;
	edLIST* pCVar2;
	uint uVar3;
		
	gbInitDone = 0;
	//g_DisplayListPtr_004495b0 = (undefined*)0x0;
	//DAT_004495b4 = 0;
	gDLISTWorkingBufferStart = (char*)0x0;
	gDLISTWorkingBufferCur = (char*)0x0;
	gDLISTWorkingBufferEnd = (char*)0x0;
	gPKTDListFlush2DStart = (edpkt_data*)0x0;
	gNbStateAdded = 0;
	gCurDList = (DisplayListInternal*)0x0;
	gNbMatrix = 0;
	gIncViewportX = 0;
	gIncViewportY = 0;
	gCurMatrix = (edF32MATRIX4*)0x0;
	gCurViewport = (ed_viewport*)0x0;
	gNbAddedVertex = 0;
	gCurDListBuf = (MeshDrawRenderCommand*)0x0;
	//PTR_DAT_00449600 = (undefined*)0x0;
	gCurVertexBuf = (Vector*)0x0;
	gCurColorBuf = (ByteColor*)0x0;
	gCurSTBuf = (char*)0x0;
	gCurColorNbInVertex = 0;
	gEndColorNbInVertex = 0;
	gCurSTNbInVertex = 0;
	gMaxSTNbInVertex = 0;
	gEndSTNbInVertex = 0;
	gEndVertexBuf = (MeshDrawRenderCommand*)0x0;
	gEndColorBuf = (undefined*)0x0;
	gEndSTBuf = (char*)0x0;
	gMaxNbVertex = 0;
	gNbDMAVertex = 0;
	gCurMaterial = (edDList_material*)0x0;
	gCurStatePKT = (edpkt_data*)0x0;
	gCurStatePKTSize = 0;
	gBlendMode = 0;
	//BYTE_00449660 = 0;
	gCurRenderState = 0;
	gCurFlushState = 0;
	gCurPrimType = 0;
	gNbUsedMaterial = 0;
	//DAT_00449670 = 0;
	//DAT_00449674 = 0;
	gbLight = 0;
	//DAT_0044967c = 0;
	gSubdivideLevel = 0;
	//DAT_00449684 = 0;
	gAddVertexFUNC = (DisplayListXYZFunc)0x0;
	gAddColorFUNC = (DisplayListRGBAQFunc)0x0;
	gAddSTFUNC = (DisplayListSTFunc)0x0;
	gShadowCast = 0;
	gShadowReceive = 0;
	gNbDList_3D[0] = 0;
	gNbDList_3D[1] = 0;

	//memset(&DAT_0048db80, 0, 0xb0);
	edDListInitMemory();
	gBankMaterial = (DisplayList_0x10*)edDListAllocMemory(g_DisplayListObjCount_004250e0 & 0xfffffff);
	//g_DisplayListPtr_004495b0 = edDListAllocMemory(100);
	gPKTDListFlush2DStart = (edpkt_data*)edDListAllocMemory((uint)g_DisplayListSizeA_004250dc >> 4);
	uVar3 = 0;
	do {
		pDVar1 = (DisplayListArray*)edDListAllocMemory((uint)(g_DisplayListSizeBCount_004250d0 << 2) >> 4);
		gDList_3D[uVar3] = pDVar1;
		pCVar2 = edListNew(1, g_DisplayListCount_004250d4, TO_HEAP(H_MAIN));
		gDList_2D[uVar3] = pCVar2;
		pCVar2 = edListNew(1, g_DisplayListCount_004250d4, TO_HEAP(H_MAIN));
		gDList_2D_Before3D[uVar3] = pCVar2;
		edListSetMode(gDList_2D[uVar3], 2, FUN_002ce480);
		edListSetMode(gDList_2D_Before3D[uVar3], 2, FUN_002ce480);
		uVar3 = uVar3 + 1;
	} while (uVar3 < 2);
	gCurMatrix = (edF32MATRIX4*)edDListAllocMemory((uint)(g_DisplayListMatrixCount_004250d8 << 6) >> 4);
	gCurRenderState = 1;
	gNbMatrix = 0;
	//g_DisplayListGlobal_00449668 = 0;
	edSysHandlersAdd(sysHandler_0048cb90.nodeParent, sysHandler_0048cb90.entries, sysHandler_0048cb90.maxEventID, (edSysHandlerType)0, edDListInterrupt, 1, 0);
	edSysHandlersAdd(sysHandler_0048cb90.nodeParent, sysHandler_0048cb90.entries, sysHandler_0048cb90.maxEventID, (edSysHandlerType)1, edDListInterrupt, 1, 0);
	edSysHandlersAdd(edSysHandlerVideo_0048cee0.nodeParent, edSysHandlerVideo_0048cee0.entries,
		edSysHandlerVideo_0048cee0.maxEventID, (edSysHandlerType)2, edDListInterrupt, 1, 0);
	edSysHandlersAdd
	(edSysHandlerVideo_0048cee0.nodeParent, edSysHandlerVideo_0048cee0.entries,
		edSysHandlerVideo_0048cee0.maxEventID, ESHT_Profile_1, edDListInterrupt, 1, 0);
	edSysHandlersAdd
	(edSysHandlerVideo_0048cee0.nodeParent, edSysHandlerVideo_0048cee0.entries,
		edSysHandlerVideo_0048cee0.maxEventID, ESHT_RenderUI, edDListInterrupt, 1, 0);
	gbInitDone = 1;
	//edDlistDebugInit();
	//edDListInitStripPKT();
	//FUN_002cdd00();
	return;
}

void edDListLoadMatrix(edF32MATRIX4* m0)
{
	sceVu0CopyMatrix((float(*)[4])(gCurMatrix + gNbMatrix), (float(*)[4])m0);
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
	if (index < pMAT_Internal->count_0x0) {
		ed_g2d_layer* pLAY = (ed_g2d_layer*)LOAD_SECTION(((ed_g2d_material_After*)(pMAT_Internal + 1))[index].pLAY);
		pLAY->body.flags_0x0 |= flag;
	}
	return;
}

void edDListBlendSet(uint mode)
{
	if ((((gbInsideBegin == false) &&
		(gBlendMode = mode & 0xff,
			gCurMaterial != (edDList_material*)0x0)) &&
		(gCurMaterial->pMAT != (ed_g2d_material*)0x0)) &&
		(gBlendMode == 1)) {
		ApplyFlag_0029f1e0(gCurMaterial->pMAT, 0, 4);
	}
	return;
}

ed_g2d_bitmap* edDListGetG2DBitmap(ed_g2d_material* pMAT, int offset, bool* bHasPalette, ed_g2d_bitmap** pOutAddr)
{
	TextureData_HASH_Internal_PA32* pTVar1;
	ed_g2d_bitmap* pTVar2;
	TextureData_TEX* iVar2;
	ed_g2d_layer* iVar1;
	TextureData_HASH_Internal_PA32* iVar3;

	*bHasPalette = false;
	*pOutAddr = (ed_g2d_bitmap*)0x0;
	if (pMAT != (ed_g2d_material*)0x0) {
		if (pMAT->count_0x0 == 0) {
			return (ed_g2d_bitmap*)0x0;
		}
		iVar1 = (ed_g2d_layer*)LOAD_SECTION(((ed_g2d_material_After*)(pMAT + 1))[offset].pLAY);

		if (((iVar1 != (ed_g2d_layer*)0xfffffff0) && (iVar1 != (ed_g2d_layer*)0xfffffff0)) && ((iVar1->body).field_0x1c != 0)) {

			iVar2 = (TextureData_TEX*)LOAD_SECTION(iVar1->body.pTex);
			pTVar2 = (ed_g2d_bitmap *)0x0;

			if (iVar2 == (TextureData_TEX*)0xfffffff0) {
				return (ed_g2d_bitmap*)0x0;
			}

			if ((iVar2->body).palette != 0) {
				*bHasPalette = true;

				iVar3 = (TextureData_HASH_Internal_PA32*)LOAD_SECTION(((TextureData_TEX_Internal_After*)(iVar2 + 1))[(uint)(iVar1->body).field_0x1e].pHASH_Internal);

				if (iVar3 != 0) {
					pTVar2 = &((TextureData_PA32*)LOAD_SECTION(iVar3->pPA32))->body;
				}

				pTVar1 = (TextureData_HASH_Internal_PA32*)LOAD_SECTION((iVar2->body).after.pHASH_Internal);
				if (pTVar1 == 0) {
					return pTVar2;
				}

				*pOutAddr = &((TextureData_PA32*)LOAD_SECTION(pTVar1->pPA32))->body;
				return pTVar2;
			}
			pTVar1 = (TextureData_HASH_Internal_PA32*)LOAD_SECTION((iVar2->body).after.pHASH_Internal);
			if (pTVar1 == 0) {
				return (ed_g2d_bitmap*)0x0;
			}
			return &((TextureData_PA32*)LOAD_SECTION(pTVar1->pPA32))->body;
		}
	}
	return (ed_g2d_bitmap*)0x0;
}

void edDListSetState(ulong cmdA, ulong cmdB)
{
	ushort uVar1;
	DisplayListInternalSubObj_60* pDVar2;
	int iVar3;
	DisplayListInternal* pDVar4;
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
					uVar1 = gCurDList->subCommandBufferCount;
					pDVar2 = gCurDList->pDisplayListInternalSubObj;
					pDVar2[uVar1].type_0x42 = 9;
					pDVar2[uVar1].pRenderInput = pRVar6;
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
						uVar1 = gCurDList->subCommandBufferCount;
						pDVar2 = gCurDList->pDisplayListInternalSubObj;
						pDVar2[uVar1].type_0x42 = 9;
						pDVar2[uVar1].pRenderInput = pRVar6;
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
(ulong ate, ulong atst, ulong aref, ulong afail, ulong date, ulong datm, ulong zte,
	ulong ztst)
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
		if ((((gCurDList != (DisplayListInternal*)0x0) &&
			(gCurDList->subCommandBufferCount != 0)) && ((gCurDList->flags_0x0 & 2) != 0))
			&& (gCurDList->pDisplayListInternalSubObj->aCommandArray
				[gCurDList->subCommandBufferCount - 1].type == LM_REF_0)) {
			edDListSetState(
				SCE_GS_SET_XYOFFSET(
					((0x1000 - ((int)pViewport->screenWidth + (int)pViewport->posX) >> 1) << 4),	// X
					((0x1000 - ((int)pViewport->screenHeight + (int)pViewport->posY) >> 1) << 4)	// Y
				),
				SCE_GS_XYOFFSET_1);
			if (gCurViewport->screenHeight == 0x1c0) {
				edDListSetState(
					SCE_GS_SET_SCISSOR(
						pViewport->posX,																// SCAX0
						((int)gCurViewport->screenWidth + (int)gCurViewport->posX + -1),	// SCAX1
						pViewport->posY,																// SCAY0
						gCurViewport->posY + 0x1ff												// SCAY1
					),
					SCE_GS_SCISSOR_1);
			}
			else {
				edDListSetState(
					SCE_GS_SET_SCISSOR(
						pViewport->posX,																// SCAX0
						((int)gCurViewport->screenWidth + (int)gCurViewport->posX + -1),	// SCAX1
						pViewport->posY,																// SCAY0
						((int)gCurViewport->screenHeight + (int)gCurViewport->posY + -1)	// SCAY1
					),
					SCE_GS_SCISSOR_1);
			}
		}
	}
	return;
}

#ifdef PLATFORM_WIN
Renderer::TextureData MakeTextureDataFromPacket(edpkt_data* pPkt, ed_g2d_bitmap* pTextureBitmap, ed_g2d_bitmap* pPaletteBitmap) {
	int imageIndex = -1;
	int paletteIndex = -1;
	uint* pPktU32 = edpktAsU32(pPkt);

	uint texWidth;
	uint texHeight;
	uint palWidth;
	uint palHeight;
	bool bSetTexDimensions = false;
	bool bSetPalDimensions = false;

	while (true)
	{
		if ((*pPktU32) >> 28 == 0x03) {
			if (imageIndex == -1) {
				imageIndex = pPktU32[1];
			}
			else {
				paletteIndex = pPktU32[1];
				break;
			}
		}

		if (pPkt->cmdB == SCE_GS_TRXREG) {
			if (!bSetTexDimensions) {
				texWidth = pPktU32[1];
				texHeight = pPktU32[0];
				bSetTexDimensions = true;
			}
			else {
				palWidth = pPktU32[1];
				palHeight = pPktU32[0];
				bSetPalDimensions = true;
			}
		}

		pPkt += 1;
		pPktU32 += 4;
	}

	assert(bSetTexDimensions);

	return { { LOAD_SECTION(imageIndex), pTextureBitmap->width, pTextureBitmap->height, texWidth, texHeight, pTextureBitmap->psm },
		{  LOAD_SECTION(paletteIndex), pPaletteBitmap->width, pPaletteBitmap->height, palWidth, palHeight, 32 } };
}
#endif

void edDListUseMaterial(edDList_material* pMaterialInfo)
{
	edpkt_data* pRVar1;
	edpkt_data* pRVar2;
	ed_g2d_bitmap* pPaletteBitmap;
	edpkt_data* pRVar4;
	RenderCommandUint* pDVar4;
	DisplayListInternalSubObj_60* pDVar5;
	RenderCommandUint* pDVar6;
	ed_g2d_bitmap* pTextureBitmap;
	bool bHasPalette;
	int iVar3;

	gCurFlashMaterial = 0;
	gCurMaterial = pMaterialInfo;
	edDListPatchGifTag2D();
	if ((gCurDList->flags_0x0 & 2) == 0) {
		return;
	}
	pDVar5 = gCurDList->pDisplayListInternalSubObj;
	pRVar1 = gCurDList->pRenderCommands;
	if (gCurDList->subCommandBufferCount != 0) {
		pDVar4 = pDVar5->aCommandArray + (gCurDList->subCommandBufferCount - 1);
		pDVar4->size = (uint)((int)pRVar1 - (int)pDVar4->pCommandBuffer);
		pDVar5 = (DisplayListInternalSubObj_60*)(pDVar4 + 1);
	}
	if (pMaterialInfo != (edDList_material*)0x0) {
		if ((pMaterialInfo->mode & 4U) == 0) {
			if (pMaterialInfo->pMAT != (ed_g2d_material*)0x0) {
				bHasPalette = false;
				pTextureBitmap = (ed_g2d_bitmap*)0x0;
				pPaletteBitmap = edDListGetG2DBitmap(pMaterialInfo->pMAT, 0, &bHasPalette, &pTextureBitmap);
				pDVar6 = pDVar5->aCommandArray;
				if (pPaletteBitmap != (ed_g2d_bitmap*)0x0) {
					pDVar5->aCommandArray[0].pCommandBuffer = (edpkt_data*)(((char*)LOAD_SECTION(pPaletteBitmap->pPSX2)) + 0x40);
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
					gCurDList->subCommandBufferCount = gCurDList->subCommandBufferCount + 1;

#ifdef PLATFORM_WIN
					Renderer::SetImagePointer(MakeTextureDataFromPacket(pDVar5->aCommandArray[0].pCommandBuffer, pTextureBitmap, pPaletteBitmap));
#endif
				}

				pDVar6->pCommandBuffer = (edpkt_data*)((char*)LOAD_SECTION(pMaterialInfo->pMAT->pCommandBufferTexture) + 0x10);

				if ((bHasPalette != false) && (0x3ffff < (uint)pTextureBitmap->width * (uint)pTextureBitmap->height)) {
					*(uint*)((int)&pDVar6->pCommandBuffer[1].cmdA + 4) =
						*(uint*)((int)&pDVar6->pCommandBuffer[1].cmdA + 4) & 0xfffc001f | 0x10000;
				}

				// Texture palette hacks.
#if 0
				//{
				//	int* pAddr = (int*)(pDVar5->aCommandArray[0].pCommandBuffer + 7);
				//	uint* pCol = (uint*)LOAD_SECTION(pAddr[1]);
				//
				//	int i = 0;
				//	for (; i < 0x40 * 0x80 * 1; i++) {
				//		pCol[i] = 0x1234567;
				//	}
				//
				//	i = 0;
				//	for (; i < 0x40 / 4; i++) {
				//		pCol[i] = 0x0;
				//	}
				//}

				//uint palette[8] = {
				//	0xFF0000FF,  // Red (ARGB)		0
				//	0xFF00FF00,  // Green (ARGB)	1
				//	0xFFFF0000,  // Blue (ARGB)		2
				//	0xFF00FFFF,  // Yellow (ARGB)	3
				//	0xFFFF00FF,  // Magenta (ARGB)	4
				//	0xFFFFFF00,  // Cyan (ARGB)		5
				//	0xFFFFFFFF,  // White (ARGB)	6
				//	0x00000000   // Black (ARGB)	7
				//};

				{
					//int* pAddr = (int*)(pDVar5->aCommandArray[0].pCommandBuffer + 7 + 8);
					//uint* pCol = (uint*)LOAD_SECTION(pAddr[1]);
					//memcpy(pCol, palette, 8 * 4);
					//pCol[4] = 0xFFFF00FF;
				}
#endif

#ifdef PLATFORM_WIN
				SendTextureCommandsFromPacked((pDVar6->pCommandBuffer + 1)->cmdA);
#endif

				pDVar6->type = LM_REF_1;
				pDVar6->size = pMaterialInfo->pMAT->commandBufferTextureSize * 0x10 - 0x10;
				gCurDList->subCommandBufferCount = gCurDList->subCommandBufferCount + 1;
				pDVar5 = (DisplayListInternalSubObj_60*)(pDVar6 + 1);
				if (pMaterialInfo->pMAT->count_0x0 != 0) {
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
			pDVar5 = (DisplayListInternalSubObj_60*)(pDVar5->aCommandArray + 1);
			gCurDList->subCommandBufferCount = gCurDList->subCommandBufferCount + 1;
		}
	}
	pDVar5->aCommandArray[0].pCommandBuffer = pRVar1;
LAB_002cba4c:
	pDVar5->aCommandArray[0].type = LM_REF_0;
	pDVar5->aCommandArray[0].size = 0;
	gCurDList->subCommandBufferCount = gCurDList->subCommandBufferCount + 1;
	if ((gCurDList->subCommandBufferCount == 1) || (gCurDList->subCommandBufferCount == 3)) {
		edDListAlphaTestAndZTest(1, 7, 0, 0, 0, 0, 1, 1);
		edDListSetActiveViewPort(gCurViewport);
	}

	//DUMP_TAG_ADV(pDVar5->aCommandArray[0].pCommandBuffer[0]);

	return;
}

bool edlistNewObj(edLIST* param_1, DisplayListInternal* param_2)
{
	bool bVar1;
	edNODE_MANAGER* pMVar2;
	MeshTransformSpecial* pMVar3;

	pMVar2 = param_1->pCameraPanHeader_0xc;
	bVar1 = pMVar2->totalCount != pMVar2->usedCount;
	if (bVar1) {
		pMVar3 = pMVar2->pNextFree;
		pMVar3->specUnion.field_0x0[0] = 2;
		pMVar3->pRenderInput = param_2;
	}
	return bVar1;
}

MeshTransformSpecial* edlistInsertNode(edLIST* pList, MeshTransformSpecial* param_2)
{
	edNODE_MANAGER* pMVar1;
	MeshTransformSpecial* pMVar2;

	pMVar1 = pList->pCameraPanHeader_0xc;
	pMVar1->usedCount = pMVar1->usedCount + 1;
	pList->count_0x14 = pList->count_0x14 + 1;
	pMVar2 = pMVar1->pNextFree;
	pMVar1->pNextFree = pMVar2->pNext_0x4;
	pMVar2->specUnion.randoPtr = NULL;
	pMVar2->pPrev_0x8 = param_2->pPrev_0x8;
	pMVar2->pNext_0x4 = param_2;
	RENDER_LOG("edlistInsertNode %p\n", param_2);
	param_2->pPrev_0x8->pNext_0x4 = pMVar2;
	param_2->pPrev_0x8 = pMVar2;
	return pMVar2;
}

bool edListAddNode(edLIST* pCameraPanHeader, DisplayListInternal* pDisplayList)
{
	edLIST* pCVar1;
	bool bVar2;
	MeshTransformSpecial* pMVar3;
	int lVar4;

	bVar2 = edlistNewObj(pCameraPanHeader, pDisplayList);
	if (bVar2 == false) {
		bVar2 = false;
	}
	else {
		if (pCameraPanHeader->headerUnion.field0 == 1) {
			pMVar3 = edlistInsertNode(pCameraPanHeader, (MeshTransformSpecial*)pCameraPanHeader);
			pCameraPanHeader->pCameraPanStatic_0x8 = (MeshTransformParent*)pMVar3;
		}
		else {
			if (pCameraPanHeader->headerUnion.field0 == 0) {
				pMVar3 = edlistInsertNode(pCameraPanHeader, pCameraPanHeader->pLoadedData);
				pCameraPanHeader->pLoadedData = pMVar3;
			}
			else {
				pCVar1 = (edLIST*)pCameraPanHeader->pLoadedData;
				while (((pCVar1 != pCameraPanHeader && (*(ushort*)&pCVar1->headerUnion != 1)) &&
					(lVar4 = (*pCameraPanHeader->headerUnion.field1)
						(pDisplayList, (DisplayListInternal*)pCVar1->pCameraPanHeader_0xc), -1 < lVar4))) {
					pCVar1 = (edLIST*)pCVar1->pLoadedData;
				}
				edlistInsertNode(pCameraPanHeader, (MeshTransformSpecial*)pCVar1);
			}
		}
		bVar2 = true;
	}
	return bVar2;
}

void edDlistAddtoView(DisplayListInternal* param_1)
{
	ushort uVar1;
	DisplayListInternal* pDisplayList;

	pDisplayList = param_1 + gCurRenderState;
	if ((param_1[gCurRenderState].flags_0x0 & 4) != 0) {
		param_1->field_0x3 = 0;
		pDisplayList = param_1;
	}
	if (pDisplayList->subCommandBufferCount == 0) {
		pDisplayList->pRenderCommands = pDisplayList->field_0x14;
		pDisplayList->field_0x10 = (MeshDrawRenderCommand*)pDisplayList->pCommandBuffer;
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
			gDList_3D[gCurRenderState]->field_0x0
				[gNbDList_3D[gCurRenderState]] = pDisplayList;
			gNbDList_3D[gCurRenderState] =
				gNbDList_3D[gCurRenderState] + 1;
		}
	}
	return;
}

void edDListVertex4f_2D(float inX, float inY, float inZ, int param_4)
{
	DisplayListInternal* pDVar1;
	int y;
	int x;
	edpkt_data** ppRVar4;
	Vector local_10;

	local_10.w = 1.0;
	local_10.z = 0.0;
	local_10.x = inX;
	local_10.y = inY;
	sceVu0ApplyMatrix(&local_10, gCurMatrix + gNbMatrix, &local_10);
	pDVar1 = gCurDList;
	x = gIncViewportX + (int)(local_10.x * 16.0);
	y = gIncViewportY + (int)(local_10.y * 16.0);
	if (gCurPrimType == 8) {
		g_Count_004495f8 = g_Count_004495f8 + 1;
#ifdef PLATFORM_WIN
		Renderer::SetVertexSkip((g_Count_004495f8 == 1) || (g_Count_004495f8 == 2));
#endif
		if ((g_Count_004495f8 == 1) || (g_Count_004495f8 == 2)) {
			gCurDList->pRenderCommands->cmdA = SCE_GS_SET_XYZ(x, y, inZ);
			pDVar1->pRenderCommands->cmdB = SCE_GS_XYZ3;
			pDVar1->pRenderCommands = pDVar1->pRenderCommands + 1;
		}
		else {
			gCurDList->pRenderCommands->cmdA = SCE_GS_SET_XYZ(x, y, inZ);
			pDVar1->pRenderCommands->cmdB = SCE_GS_XYZ2;
			pDVar1->pRenderCommands = pDVar1->pRenderCommands + 1;
			if (g_Count_004495f8 == 4) {
				g_Count_004495f8 = 0;
			}
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
	}
	gNbAddedVertex = gNbAddedVertex + 1;
	return;
}

void edDListColor4u8_2D(byte r, byte g, byte b, byte a)
{
	DisplayListInternal* pDVar1;

	g_RGBAQ_00448aa0->r = r;
	g_RGBAQ_00448aa0->g = g;
	g_RGBAQ_00448aa0->b = b;
	g_RGBAQ_00448aa0->a = a;
	pDVar1 = gCurDList;
	gCurDList->pRenderCommands->cmdA = SCE_GS_SET_RGBAQ(r, g, b, a, 0x3f800000);
	pDVar1->pRenderCommands->cmdB = SCE_GS_RGBAQ;
	pDVar1->pRenderCommands = pDVar1->pRenderCommands + 1;
	return;
}

void edDListTexCoo2f_2D(float inS, float inT)
{
	DisplayListInternal* pDVar1;

	pDVar1 = gCurDList;

	uint S = *reinterpret_cast<uint*>(&inS);
	uint T = *reinterpret_cast<uint*>(&inT);

	gCurDList->pRenderCommands->cmdA = SCE_GS_SET_ST(S, T);
	pDVar1->pRenderCommands->cmdB = SCE_GS_ST;
	pDVar1->pRenderCommands = pDVar1->pRenderCommands + 1;
	return;
}

void edDListBegin2D(ulong count)
{
	edpkt_data* pRVar1;
	edpkt_data** ppRVar2;
	uint uVar3;
	long lVar4;

	uVar3 = (uint)gCurDList->pRenderCommands & 0xf;
	if (uVar3 != 0) {
		gCurDList->pRenderCommands =
			(edpkt_data*)((int)gCurDList->pRenderCommands + (0x10 - uVar3));
	}
	ppRVar2 = &gCurDList->pRenderCommands;
	pRVar1 = gCurDList->pRenderCommands;
	if ((gCurMaterial != (edDList_material*)0x0) || (lVar4 = 0, gCurFlashMaterial != 0)) {
		lVar4 = 1;
	}
	gCurStatePKT = pRVar1;
	uint primCount = gBlendMode << 6 | count & 0xffffffff | lVar4 << 4;
	if (Mode_00449694 == 0) {
		pRVar1->cmdA = SCE_GIF_SET_TAG(
			0,							// NLOOP
			SCE_GS_TRUE,				// EOP
			SCE_GS_TRUE,				// PRE
			primCount,					// PRIM
			SCE_GIF_PACKED,				// FLG
			1							// NREG
		);
	}
	else {
		primCount = primCount | 8;
		pRVar1->cmdA = SCE_GIF_SET_TAG(
			0,							// NLOOP
			SCE_GS_TRUE,				// EOP
			SCE_GS_TRUE,				// PRE
			primCount,					// PRIM
			SCE_GIF_PACKED,				// FLG
			1							// NREG
		);

	}
	pRVar1->cmdB = SCE_GIF_PACKED_AD;
	*ppRVar2 = pRVar1 + 1;
	return;
}

void edDListPatchGifTag3D(void)
{
	edpkt_data* pRVar1;
	edpkt_data* pRVar2;
	int iVar3;

	if (gNbStateAdded != 0) {
		pRVar1 = gCurDList->pRenderCommands;
		*(undefined4*)&pRVar1->cmdA = 0;
		*(undefined4*)((int)&pRVar1->cmdA + 4) = 0x14000000;
		pRVar1->cmdB = 0;
		pRVar2 = pRVar1 + 1;
		gCurStatePKTSize = gNbStateAdded + 2;
		gCurStatePKT =
			gCurDList->pDisplayListInternalSubObj[gCurDList->subCommandBufferCount].
			pRenderInput;
		gCurDList->pDisplayListInternalSubObj[gCurDList->subCommandBufferCount].pRenderInput
			= pRVar2;
		iVar3 = gCurStatePKTSize - 1;
		pRVar2->cmdA = ((long)(int)gCurStatePKT & 0xfffffffU) << 0x20 |
			(long)(int)(gCurStatePKTSize | 0x30000000);
		pRVar1[1].cmdB = ((long)(iVar3 * 0x10000) & 0xffffffffU | 0x6c0003dc) << 0x20 | 0x40003dc;
		gCurDList->pRenderCommands = pRVar1 + 2;
		gCurDList->field_0x10 = (MeshDrawRenderCommand*)gCurDList->pRenderCommands;
		gCurDListBuf = gCurDList->field_0x10;
		gCurDList->subCommandBufferCount = gCurDList->subCommandBufferCount + 1;
		iVar3 = -gCurStatePKTSize;
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

void edDListBeginStrip(float x, float y, float z, uint param_4, ushort param_5)
{
	MeshDrawRenderCommand** ppMVar1;
	MeshDrawRenderCommand* pMVar2;
	Vector* pVVar3;
	short sVar4;
	uint uVar5;
	int iVar6;
	uint uVar7;
	DisplayListInternalSubObj_60* v0;
	ushort uVar8;
	ushort uVar9;
	MeshDrawRenderCommand* pMVar10;
	uint uVar11;

	v0 = gCurDList->pDisplayListInternalSubObj + gCurDList->subCommandBufferCount;
	v0->field_0x50 = param_4;
	v0->field_0x40 = 1;
	//DAT_00448a80 = (uint)g_DisplayListEnd_004495d8->subCommandBufferCount;
	gCurDList->subCommandBufferCount = gCurDList->subCommandBufferCount + 1;
	pMVar2 = gCurDListBuf;
	if (gCurDList->field_0x2c != (char*)0x0) {
		*(DisplayListInternalSubObj_60**)
			(gCurDList->field_0x2c + (uint)(ushort)gCurDList->field_0x6 * 4) = v0;
		gCurDList->field_0x6 = gCurDList->field_0x6 + 1;
	}
	if (gCurMaterial == (edDList_material*)0x0) {
		gCurDListBuf->field_0x4 = -1;
	}
	else {
		gCurDListBuf->field_0x4 = (short)gCurMaterial->Length;
	}
	gCurDListBuf->field_0x6 = -1;
	gCurDListBuf->field_0x39 = 0;
	uVar9 = 0;
	do {
		uVar8 = uVar9;
		uVar9 = uVar8 + 1;
	} while ((uVar8 + 1) * 0x46 + 2 < param_4);
	iVar6 = param_4 + ((uint)uVar8 * 8 - (uint)uVar8) * -10;
	uVar9 = uVar8 + 1;
	if (iVar6 == 0) {
		iVar6 = 0x48;
		uVar9 = uVar8;
	}
	gCurDListBuf->field_0x3a = uVar9;
	gCurDListBuf->field_0x38 = (byte)iVar6;
	gCurDListBuf->field_0x0 = 0x4000000;
	if (gbLight != 0) {
		gCurDListBuf->field_0x0 = gCurDListBuf->field_0x0 | 0x10;
	}
	if (gShadowCast == 0) {
		gCurDListBuf->field_0x30 = 0;
	}
	else {
		gCurDListBuf->field_0x30 = gShadowCast;
	}
	sVar4 = gShadowReceive;
	if (gShadowReceive == 0) {
		gCurDListBuf->field_0x32 = 0;
	}
	else {
		BYTE_004496e4 = 1;
		gCurDListBuf->field_0x0 = gCurDListBuf->field_0x0 | 0x2000;
		gCurDListBuf->field_0x32 = sVar4;
	}
	(gCurDListBuf->field_0x10).x = x;
	(gCurDListBuf->field_0x10).y = y;
	(gCurDListBuf->field_0x10).z = z;
	(gCurDListBuf->field_0x10).w = 100000.0;
	iVar6 = param_4 + ((int)(param_4 + 1) / 0x48) * 2;
	uVar7 = iVar6 * 4 + 0x40;
	if ((uVar7 & 0xf) != 0) {
		uVar7 = uVar7 + (0x10 - (uVar7 & 0xf));
	}
	uVar5 = iVar6 * 4 + 0x40;
	if ((uVar5 & 0xf) != 0) {
		uVar5 = uVar5 + (0x10 - (uVar5 & 0xf));
	}
	if ((gCurDList->flags_0x0 & 0x100) == 0) {
		gCurDListBuf->field_0x3c = (MeshDrawRenderCommand*)0x0;
		gCurDListBuf->field_0x20 = (char*)(gCurDListBuf + 1);
		uVar11 = (uint)gCurDListBuf->field_0x20 & 0xf;
		if (uVar11 != 0) {
			gCurDListBuf->field_0x20 = gCurDListBuf->field_0x20 + (0x10 - uVar11);
		}
	}
	else {
		gCurDListBuf->field_0x3c = gCurDListBuf + 1;
		uVar11 = (uint)gCurDListBuf->field_0x3c & 0xf;
		if (uVar11 != 0) {
			gCurDListBuf->field_0x3c =
				(MeshDrawRenderCommand*)((int)gCurDListBuf->field_0x3c + (0x10 - uVar11));
		}
		gCurDListBuf->field_0x20 =
			(char*)(&gCurDListBuf->field_0x3c->field_0x0 + (uint)uVar9 * 4);
		v0->field_0x41 = (byte)uVar9;
		uVar11 = (uint)gCurDListBuf->field_0x20 & 0xf;
		if (uVar11 != 0) {
			gCurDListBuf->field_0x20 = gCurDListBuf->field_0x20 + (0x10 - uVar11);
		}
	}
	gCurDListBuf->field_0x24 = gCurDListBuf->field_0x20 + uVar7;
	gCurDListBuf->field_0x24 = gCurDListBuf->field_0x24 + 0x10;
	uVar11 = (uint)gCurDListBuf->field_0x24 & 0xf;
	if (uVar11 != 0) {
		gCurDListBuf->field_0x24 = gCurDListBuf->field_0x24 + (0x10 - uVar11);
	}
	gCurDListBuf->field_0x28 = gCurDListBuf->field_0x24 + uVar5;
	uVar11 = (uint)gCurDListBuf->field_0x28 & 0xf;
	if (uVar11 != 0) {
		gCurDListBuf->field_0x28 = gCurDListBuf->field_0x28 + (0x10 - uVar11);
	}
	gCurColorBuf = (ByteColor*)gCurDListBuf->field_0x24;
	gCurSTBuf = gCurDListBuf->field_0x20 + 0x10;
	gCurVertexBuf = (Vector*)gCurDListBuf->field_0x28;
	if ((param_4 & 3) != 0) {
		param_4 = param_4 + (4 - (param_4 & 3));
	}
	pMVar10 = (MeshDrawRenderCommand*)
		((int)(MeshDrawRenderCommand*)(gCurVertexBuf + param_4) - (int)gCurDListBuf);
	ppMVar1 = (MeshDrawRenderCommand**)&gCurDListBuf->field_0x8;
	gCurDListBuf = (MeshDrawRenderCommand*)(gCurVertexBuf + param_4);
	gStartVertexBuf = (undefined*)gCurVertexBuf;
	*ppMVar1 = pMVar10;
	pMVar2->field_0x2c = (char*)0x0;
	pVVar3 = gCurVertexBuf;
	gCurVertexBuf->w = 6.887662e-41;
	pVVar3[1].w = 6.887662e-41;
	gEndSTNbInVertex = 1;
	gEndColorNbInVertex = 1;
	gCurSTNbInVertex = 0;
	gEndColorBuf = &gCurColorBuf->r + uVar5;
	gMaxSTNbInVertex = 0;
	gCurColorNbInVertex = 0;
	gEndVertexBuf = gCurDListBuf;
	gEndSTBuf = gCurSTBuf + uVar7;
	v0->pRenderInput = (edpkt_data*)pMVar2;
	if (((uint)gCurDListBuf & 0xf) != 0) {
		gCurDListBuf =
			(MeshDrawRenderCommand*)
			((int)gCurDListBuf + (0x10 - ((uint)gCurDListBuf & 0xf)));
	}
	v0->type_0x42 = param_5;
	v0->field_0x4c = gCurPrimType;
	sceVu0CopyMatrix((float(*)[4])v0, (float(*)[4])gCurMatrix + gNbMatrix);
	gNbVertexDMA = 0x48;
	return;
}

void edDListBegin(float x, float y, float z, ulong mode, int count)
{
	edpkt_data* pRVar1;
	int iVar2;
	MeshDrawRenderCommand* pcVar3;
	DisplayListInternal* pDVar4;
	uint uVar5;
	DisplayListInternalSubObj_60* pDVar6;

	pcVar3 = gCurDListBuf;
	iVar2 = gNbStateAdded;
	if (((gCurDList->flags_0x0 & 0x100) != 0) &&
		(pDVar4 = gCurDListHandle + gCurFlushState, gCurDList != pDVar4)) {
		gCurDListBuf = (MeshDrawRenderCommand*)pDVar4->field_0x10;
		gCurDList = pDVar4;
		edDListBegin(x, y, z, mode, count);
		gCurDList->field_0x10 = gCurDListBuf;
		gbInsideBegin = false;
		gCurDList = gCurDListHandle + gCurRenderState;
	}
	uVar5 = (uint)mode;
	gCurPrimType = uVar5 & 0xff;
	gNbAddedVertex = 0;
	gNbDMAVertex = 0;
	gNbStateAdded = iVar2;
	gCurDListBuf = pcVar3;
	gMaxNbVertex = count;
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
		switch ((int)(mode & 0xff)) {
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
			edDListBegin2D(mode & 0xff);
			break;
		default:
			edDListBegin2D(4);
			break;
		case 8:
			g_Count_004495f8 = 0;
			edDListBegin2D(4);
		}
		gAddVertexFUNC = edDListVertex4f_2D;
		gAddColorFUNC = edDListColor4u8_2D;
		gAddSTFUNC = edDListTexCoo2f_2D;
		gbInsideBegin = true;
		return;
	}
	edDListPatchGifTag3D();
	if (((0 < gSubdivideLevel) && ((mode & 0xff) == 0xb)) || ((mode & 0xff) == 0xc)) {
		IMPLEMENTATION_GUARD(edDListSubDivBegin();)
	}
	pDVar6 = gCurDList->pDisplayListInternalSubObj + gCurDList->subCommandBufferCount;
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
			edDListBeginStrip(x, y, z, count, (ushort)mode & 0xff);
			break;
		case 3:
		case 8:
			edDListBeginStrip(x, y, z, count, 4);
			goto LAB_002ca38c;
		default:
			mode = 4;
			break;
		case 6:
		case 7:
		case 0xb:
		case 0xc:
			IMPLEMENTATION_GUARD(
				gMaxNbVertex = gMaxNbVertex << 2;
			edDListBeginSprite(count, uVar5 & 0xff);)
			break;
		case 10:
			count = count + 1;
			IMPLEMENTATION_GUARD(
				gMaxNbVertex = gMaxNbVertex + 1;)
		case 1:
			edDListBeginStrip(x, y, z, count, 2);
	}
	uVar5 = (uint)mode;
LAB_002ca38c:
	uVar5 = uVar5 & 0xff;
	gbInsideBegin = true;
	IMPLEMENTATION_GUARD(
	gAddVertexFUNC = gTableAddVertexFUNC_3D[uVar5];
	gAddColorFUNC = gTableAddColorFUNC_3D[uVar5];
	gAddSTFUNC = gTableAddSTFUNC_3D[uVar5];)
	return;
}

edpkt_data* edDListCheckState(edpkt_data* pRenderCommand)
{
	ushort uVar1;
	DisplayListInternalSubObj_60* pDVar2;

	if (gNbStateAdded == 0) {
		if ((gCurDList->flags_0x0 & 1) != 0) {
			uVar1 = gCurDList->subCommandBufferCount;
			pDVar2 = gCurDList->pDisplayListInternalSubObj;
			pDVar2[uVar1].type_0x42 = 9;
			pDVar2[uVar1].pRenderInput = pRenderCommand;
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
	DisplayListInternal* pDVar1;
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

void edDListVertex4f(float x, float y, float z, int param_4)
{
	/* WARNING: Could not recover jumptable at 0x002cd6e4. Too many branches */
	/* WARNING: Treating indirect jump as call */
	(gAddVertexFUNC)(x, y, z, param_4);
	return;
}

edF32VECTOR3 Vector3_0048d390 = { 0 };

void edDListEnd(void)
{
	ushort uVar1;
	edpkt_data* pRVar2;
	uint uVar3;
	ulong uVar4;
	DisplayListInternalSubObj_60* pDVar5;

	if (gbInsideBegin != false) {
		if ((gCurDList->flags_0x0 & 1) == 0) {
			if ((gCurDList->flags_0x0 & 2) != 0) {
				pRVar2 = gCurDList->pRenderCommands;
				uVar3 = *(uint*)&gCurStatePKT->cmdA;
				*(undefined4*)&gCurStatePKT->cmdA = 0;
				*(uint*)&gCurStatePKT->cmdA =
					uVar3 & 0xffff8000 | ((uint)((int)pRVar2 - (int)gCurStatePKT) >> 4) - 1 & 0x7fff;
			}
		}
		else {
			if (gCurDList->subCommandBufferCount != 0) {
				if (gCurPrimType == 10) {
					edDListVertex4f(Vector3_0048d390.x, Vector3_0048d390.y, Vector3_0048d390.z, 10);
				}
				gNbVertexDMA = 0x48;
				gCurStatePKTSize = 0;
				gCurStatePKT = (edpkt_data*)0x0;
				pDVar5 = gCurDList->pDisplayListInternalSubObj +
					(gCurDList->subCommandBufferCount - 1);
				pDVar5->pCurDListBuf = (uint)gCurDListBuf;
				uVar1 = pDVar5->type_0x42;
				if (uVar1 != 9) {
					if ((((uVar1 == 0xc) || (uVar1 == 0xb)) || (uVar1 == 7)) || (uVar1 == 6)) {
						if (gNbAddedVertex < 1) {
							gCurDList->subCommandBufferCount = gCurDList->subCommandBufferCount - 1;
							gCurDListBuf = (MeshDrawRenderCommand*)pDVar5->pRenderInput;
						}
						else {
							IMPLEMENTATION_GUARD(
							edDListEndSprite((int)pDVar5->pRenderInput);
							gCurDListBuf =
								(MeshDrawRenderCommand*)
								FUN_002a3240((uint*)pDVar5->pRenderInput, (undefined4*)gCurDListBuf,
									(int)gBankMaterial, (ulong)pDVar5->type_0x42);)
						}
					}
					else {
						if (((uVar1 == 4) || (uVar1 == 2)) || (uVar1 == 0)) {
							if (gNbAddedVertex < 2) {
								gCurDList->subCommandBufferCount = gCurDList->subCommandBufferCount - 1;
								gCurDListBuf = (MeshDrawRenderCommand*)pDVar5->pRenderInput;
							}
							else {
								IMPLEMENTATION_GUARD(
								pDVar5->field_0x50 = gNbAddedVertex;
								edDListEndStrip(pDVar5->pRenderInput);
								uVar4 = edDListStripPreparePacket((long)(int)pDVar5->pRenderInput, (long)(int)gCurDListBuf);
								gCurDListBuf = (MeshDrawRenderCommand*)uVar4;)
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

void SetUnitMatrix_002d07b0(void)
{
	sceVu0UnitMatrix((TO_SCE_MTX)(gCurMatrix + gNbMatrix));
	return;
}
