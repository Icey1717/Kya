#include "edDlist.h"
#include "edMem.h"
#include <string.h>
#include <stdio.h>
#include <assert.h>

#ifdef PLATFORM_PS2
#include <libvu0.h>
#include <shellCore.h>
#include <shellDma.h>
#endif

#include "ed3D.h"
#include "Rendering/DisplayList.h"
#include "LargeObject.h"
#include "edVideo/VideoA.h"
#include "edVideo/VideoB.h"
#include "Camera.h"
#include "edSystem.h"
#include "edVideo/VideoD.h"

namespace edDlist
{
	char* g_DisplayListMemStart_004595b8;
	char* g_DisplayListMemAllocPtr_004495bc;
	char* g_DisplayListMemEnd_004595c0;

	int g_DisplayListSizeA_004250dc = 0x3E80;
	int g_DisplayListSizeBCount_004250d0 = 0x5;
	int g_DisplayListMatrixCount_004250d8 = 0xA;
	int g_DisplayListObjCount_004250e0 = 0x12C;

	int g_DisplayListGlobal_00449664 = 0;
	int g_DisplayListCommandCount_004495d4 = 0;

	uint g_DisplayListMode_00449658 = 0;

	int g_DisplayListMatrixCount_004495e0;
	Matrix* g_DisplayListMatrix_004595e4;

	int g_Width_004495ec = 0;
	int g_Height_004495f0 = 0;
	CameraObj_28* g_Camera_004495e8 = NULL;

	DisplayList_0x10* g_DisplayListPtr_0044965c;

	RenderCommand* g_CachedRenderCommandPtr_00449650 = NULL;

	int MaterialDisplayListCount_00449648;

	int g_DrawEnded_004496c0 = 0;

	byte DAT_00449660 = 0;
	int INT_00449654 = 0;
	int INT_00449680 = 0;

	// Possibly a debug switch
	int INT_004496d8 = 0;

	Matrix* PTR_004496d4 = NULL;

	byte BYTE_00449678 = 0;
	byte BYTE_004496e4 = 0;
	short SHORT_00449698 = 0;
	short SHORT_0044969c = 0;

	void AllocateDisplayListMem_002ce4c0(void)
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

		g_DisplayListMemStart_004595b8 = (char*)edMemAlloc(1, size);
		memset(g_DisplayListMemStart_004595b8, 0, size);
		g_DisplayListMemAllocPtr_004495bc = g_DisplayListMemStart_004595b8;
		g_DisplayListMemEnd_004595c0 = g_DisplayListMemStart_004595b8 + size;
		return;
	}

	void* ProvisionDisplayListMemory_002ce490(int count)
	{
		char* puVar1;
		char* pcVar1;

		pcVar1 = g_DisplayListMemAllocPtr_004495bc + count * sizeof(DisplayList_0x10);
		puVar1 = (char*)0x0;
		if (pcVar1 <= g_DisplayListMemEnd_004595c0) {
			puVar1 = pcVar1 + count * -0x10;
			g_DisplayListMemAllocPtr_004495bc = pcVar1;
		}
		return (void*)puVar1;
	}

	DisplayListData DisplayListData_004496a0 = { 0 };
	int g_DisplayListGlobal_00449668 = 0;
	int g_HasUIData_00448a88 = 0;
	RenderCommand* g_LiveDisplayList_004495d0 = NULL;


	RenderCommand* LinkSubCommandBuffers_002ce850(DisplayListInternal* pDisplayList, RenderCommand* pCommandBuffer)
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
			if (uVar1 == 1) {
				pCommandBuffer = LinkSubCommandBuffers_002ce850((DisplayListInternal*)(&pRVar3->type)[3], pCommandBuffer);
			}
			else {
				if (uVar1 == 2) {
					pCommandBuffer->cmdA = ((long)(int)(&pRVar3->type)[1] & 0xfffffffU) << 0x20 | 0x50000000;
					pCommandBuffer->cmdB = 0;
					pCommandBuffer = pCommandBuffer + 1;
				}
				else {
					if (uVar1 == 4) {
						assert(false);
						//pCommandBuffer = FUN_002cb4e0(pCommandBuffer);
					}
					else {
						if (uVar1 == 0) {

							uVar2 = (ulonglong)((&pRVar3->type)[2] >> 4);
							if (((&pRVar3->type)[2] != 0) && (uVar2 != 0)) {
								pCommandBuffer->cmdA = ((long)(int)(&pRVar3->type)[1] & 0xfffffffU) << 0x20 | uVar2 | 0x30000000;
								pCommandBuffer->cmdB = 0;
								pCommandBuffer = pCommandBuffer + 1;
							}
						}
						else {
							if (uVar1 != 5) {
								uVar4 = uVar4 + 1;
								goto LAB_002ce9d4;
							}
							uVar2 = (ulonglong)((&pRVar3->type)[2] >> 4);
							if ((&pRVar3->type)[2] != 0) {
								if (uVar2 != 0) {
									pCommandBuffer->cmdA = ((long)(int)(&pRVar3->type)[1] & 0xfffffffU) << 0x20 | uVar2 | 0x30000000;
									pCommandBuffer->cmdB = 0;
									pCommandBuffer = pCommandBuffer + 1;
								}
								pCommandBuffer->cmdA = 0xe10000002;
								pCommandBuffer->cmdB = 0;
								pCommandBuffer[1].cmdA = 0x1000000000008001;
								pCommandBuffer[1].cmdB = 0xe;
								pCommandBuffer[2].cmdA = 0x3000f;
								pCommandBuffer[2].cmdB = 0x47;
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

	void RenderUI_002cea00(CameraPanMasterHeader* pHeader)
	{
		DisplayListInternal* pDisplayList;
		DisplayListInternalSubObj_60* pDVar1;
		RenderCommand* pCommandBuffer;
		RenderCommandUint* pRVar2;
		RenderCommand* pBuffer;
		MeshTransformSpecial* pCVar7;

		pBuffer = edDlist::g_LiveDisplayList_004495d0 +
			(edDlist::g_DisplayListGlobal_00449668 * ((uint)g_DisplayListSizeA_004250dc >> 4) >> 1);
		pCommandBuffer = pBuffer;
		for (pCVar7 = pHeader->pLoadedData; (CameraPanMasterHeader*)pCVar7 != pHeader; pCVar7 = pCVar7->pNext_0x4) {
			pDisplayList = pCVar7->pRenderInput;
			if ((g_HasUIData_00448a88 != 0) && (pDisplayList->pRenderCommands != pDisplayList->field_0x14)) {
				pRVar2 = pDisplayList->pDisplayListInternalSubObj->aCommandArray + (pDisplayList->subCommandBufferCount - 1);
				if (pRVar2->type == 0) {
					pRVar2->size = (uint)((int)pDisplayList->pRenderCommands - (int)pRVar2->pCommandBuffer);
				}
				pCommandBuffer = LinkSubCommandBuffers_002ce850(pDisplayList, pCommandBuffer);
			}
			if ((pDisplayList->flags_0x0 & 0x10) != 0) {
				pDVar1 = pDisplayList->pDisplayListInternalSubObj;
				pDisplayList->pRenderCommands = pDisplayList->field_0x14;
				pDVar1->aCommandArray[0].pCommandBuffer = pDisplayList->field_0x14;
				pDVar1->aCommandArray[0].type = 0;
				pDisplayList->subCommandBufferCount = 0;
			}
			pDisplayList->field_0x3 = 0;
		}
		if ((pBuffer != pCommandBuffer) && (g_HasUIData_00448a88 != 0)) {
			pCommandBuffer->cmdA = 0x70000000;
			pCommandBuffer->cmdB = 0;
			//WaitDMA();
			//WaitForDraw_00258230();
			//WaitForDraw_00258230();
			if (g_Render_UI_00448a8c != 0) {
#ifdef PLATFORM_PS2
				shellDmaStartChain(SHELLDMA_CHANNEL_GIF, (ShellDmaTag*)pBuffer);
#endif
			}
			//WaitForDraw_00258230();
		}
		Link_00290c10(pHeader);
		return;
	}


	void PrepareDisplayListData_002cb2c0(void)
	{
		DisplayListInternal* pDVar1;
		uint displayListIndex;
		DisplayListArray* pDisplayListArray;

		displayListIndex = 0;
		pDisplayListArray = DisplayListData_004496a0.pDisplayListArray[g_DisplayListGlobal_00449668];
		while (true) {
			if (DisplayListData_004496a0.LoadedDisplayListCount[g_DisplayListGlobal_00449668] <= displayListIndex) break;
			pDVar1 = pDisplayListArray->field_0x0[displayListIndex];
			if (((pDVar1->flags_0x0 & 4) == 0) && ((pDVar1->flags_0x0 & 0x100) == 0)) {
				pDVar1->subCommandBufferCount = 0;
				pDVar1->pRenderCommands = (RenderCommand*)pDVar1->field_0xc;
				pDVar1->field_0x10 = (MeshDrawRenderCommand*)pDVar1->field_0xc;
			}
			pDVar1->field_0x3 = 0;
			displayListIndex = displayListIndex + 1;
			pDVar1->field_0x24 = 100.0;
		}
		DisplayListData_004496a0.LoadedDisplayListCount[g_DisplayListGlobal_00449668] = 0;
		g_DisplayListGlobal_00449668 = g_DisplayListGlobal_00449664;
		g_DisplayListGlobal_00449664 = g_DisplayListGlobal_00449664 + 1;
		if (g_DisplayListGlobal_00449664 == 2) {
			g_DisplayListGlobal_00449664 = 0;
		}
		g_HasUIData_00448a88 = 1;
		g_DisplayListEnd_004495d8 = (DisplayListInternal*)0x0;
		return;
	}

	void RenderProcessEvent(int callingID, int eventID, char* pRenderTaskData)
	{
		if (callingID == 10) {
			if (eventID == 1) {
				RenderUI_002cea00(DisplayListData_004496a0.pCameraPanPairB[g_DisplayListGlobal_00449668]);
			}
			else {
				if (eventID == 0) {
					assert(false);
					//ProcessRenderTask_002ce5c0(pRenderTaskData);
				}
			}
		}
		else {
			if (callingID == 2) {
				if (eventID == 10) {
					RenderUI_002cea00(DisplayListData_004496a0.pCameraPanPairA[g_DisplayListGlobal_00449664]);
					DAT_00449660 = 1;
				}
				else {
					if ((eventID != 2) && (eventID == 0)) {
						PrepareDisplayListData_002cb2c0();
						DAT_00449660 = 0;
					}
				}
			}
		}
		return;
	}

	const int g_DisplayListCount_004250d4 = 5;

	void Init(void)
	{
		DisplayListArray* pDVar1;
		CameraPanMasterHeader* pCVar2;
		uint uVar3;
		//
		//g_DisplayListGlobal_004495a8 = 0;
		//g_DisplayListPtr_004495b0 = (undefined*)0x0;
		//DAT_004495b4 = 0;
		//g_DisplayListMemStart_004595b8 = (undefined*)0x0;
		g_DisplayListMemAllocPtr_004495bc = (char*)0x0;
		g_DisplayListMemEnd_004595c0 = (char*)0x0;
		g_LiveDisplayList_004495d0 = (RenderCommand*)0x0;
		g_DisplayListCommandCount_004495d4 = 0;
		g_DisplayListEnd_004495d8 = (DisplayListInternal*)0x0;
		g_DisplayListMatrixCount_004495e0 = 0;
		g_Width_004495ec = 0;
		g_Height_004495f0 = 0;
		g_DisplayListMatrix_004595e4 = (Matrix*)0x0;
		g_Camera_004495e8 = (CameraObj_28*)0x0;
		//DAT_004495f4 = 0;
		PTR_MeshDrawRenderCommand_004495fc = (MeshDrawRenderCommand*)0x0;
		//PTR_DAT_00449600 = (undefined*)0x0;
		//PTR_DAT_00449604 = (undefined*)0x0;
		//PTR_DAT_0044960c = (undefined*)0x0;
		//PTR_DAT_00449610 = (undefined*)0x0;
		//DAT_00449614 = 0;
		//DAT_00449618 = 0;
		//DAT_0044961c = 0;
		//DAT_00449620 = 0;
		//DAT_00449624 = 0;
		//PTR_DAT_00449628 = (undefined*)0x0;
		//PTR_DAT_00449630 = (undefined*)0x0;
		//PTR_DAT_00449634 = (undefined*)0x0;
		//DAT_0044963c = 0;
		//DAT_00449640 = 0;
		//DAT_00449644 = 0;
		g_CachedRenderCommandPtr_00449650 = 0;
		INT_00449654 = 0;
		g_DisplayListMode_00449658 = 0;
		DAT_00449660 = 0;
		g_DisplayListGlobal_00449664 = 0;
		//g_DisplayListGlobal_00449668 = 0;
		//DAT_0044966c = 0;
		MaterialDisplayListCount_00449648 = 0;
		//DAT_00449670 = 0;
		//DAT_00449674 = 0;
		BYTE_00449678 = 0;
		//DAT_0044967c = 0;
		INT_00449680 = 0;
		//DAT_00449684 = 0;
		//DAT_004496e8 = 0;
		//DAT_004496ec = 0;
		//PTR_FUN_004496f0 = (undefined*)0x0;
		SHORT_00449698 = 0;
		SHORT_0044969c = 0;
		//DisplayListData_004496a0.LoadedDisplayListCount[0] = 0;
		//DisplayListData_004496a0.LoadedDisplayListCount[1] = 0;
		//memset(&DAT_0048db80, 0, 0xb0);
		AllocateDisplayListMem_002ce4c0();
		g_DisplayListPtr_0044965c = (DisplayList_0x10*)ProvisionDisplayListMemory_002ce490(g_DisplayListObjCount_004250e0 & 0xfffffff);
		//g_DisplayListPtr_004495b0 = ProvisionDisplayListMemory_002ce490(100);
		g_LiveDisplayList_004495d0 = (RenderCommand*)ProvisionDisplayListMemory_002ce490((uint)g_DisplayListSizeA_004250dc >> 4);
		uVar3 = 0;
		do {
			pDVar1 = (DisplayListArray*)ProvisionDisplayListMemory_002ce490((uint)(g_DisplayListSizeBCount_004250d0 << 2) >> 4);
			DisplayListData_004496a0.pDisplayListArray[uVar3] = pDVar1;
			pCVar2 = AllocateCameraAndMesh_00290a10(1, g_DisplayListCount_004250d4, 1);
			DisplayListData_004496a0.pCameraPanPairA[uVar3] = pCVar2;
			pCVar2 = AllocateCameraAndMesh_00290a10(1, g_DisplayListCount_004250d4, 1);
			DisplayListData_004496a0.pCameraPanPairB[uVar3] = pCVar2;
			//FUN_002909f0(DisplayListData_004496a0.pCameraPanPairA[uVar3], 2, FUN_002ce480);
			//FUN_002909f0(DisplayListData_004496a0.pCameraPanPairB[uVar3], 2, FUN_002ce480);
			uVar3 = uVar3 + 1;
		} while (uVar3 < 2);
		g_DisplayListMatrix_004595e4 = (Matrix*)ProvisionDisplayListMemory_002ce490((uint)(g_DisplayListMatrixCount_004250d8 << 6) >> 4);
		g_DisplayListGlobal_00449664 = 1;
		g_DisplayListMatrixCount_004495e0 = 0;
		//g_DisplayListGlobal_00449668 = 0;
		//edSysHandlers::edSysHandlersAdd(sysHandler_0048cb90.pNodeTable, sysHandler_0048cb90.pTypeArray_0x4, PoolEntryIdentifier_0048cba8, New_Name, RenderProcessEvent, 1, 0);
		//edSysHandlers::edSysHandlersAdd(sysHandler_0048cb90.pNodeTable, sysHandler_0048cb90.pTypeArray_0x4, PoolEntryIdentifier_0048cba8, New_Name_(1), RenderProcessEvent, 1, 0);
		//edSysHandlers::edSysHandlersAdd(edSysHandlersNodeParent_0048cee0.pNodeTable, edSysHandlersNodeParent_0048cee0.pTypeArray_0x4, g_SysHandlersMainMaxEventID_0048cf10, New_Name_(2), RenderProcessEvent, 1, 0);
		edSysHandlersAdd
		(edSysHandlerVideo_0048cee0.nodeParent, edSysHandlerVideo_0048cee0.entries,
			edSysHandlerVideo_0048cee0.maxEventID, ESHT_Profile_1, RenderProcessEvent, 1, 0);
		edSysHandlersAdd
		(edSysHandlerVideo_0048cee0.nodeParent, edSysHandlerVideo_0048cee0.entries,
			edSysHandlerVideo_0048cee0.maxEventID, ESHT_RenderUI, RenderProcessEvent, 1, 0);
		//g_DisplayListGlobal_004495a8 = 1;
		//LoadLib_002d06e0();
		//FUN_002cdb60();
		//FUN_002cdd00();
		return;
	}

	void CopyMatrixToDisplayList_002d07d0(Matrix* m0)
	{
		sceVu0CopyMatrix((float(*)[4])(g_DisplayListMatrix_004595e4 + g_DisplayListMatrixCount_004495e0), (float(*)[4])m0);
		return;
	}

	int Mode_0044964c = 0;
	::MaterialInfo* g_pMaterialInfo_00449644 = NULL;
	DisplayListInternal* g_DisplayListEnd_004495d8 = NULL;

	void ColourCommandSetup_002c9fd0(void)
	{
		RenderCommand* pRVar1;

		if (g_DisplayListCommandCount_004495d4 != 0) {
			pRVar1 = g_DisplayListEnd_004495d8->pRenderCommands;
			pRVar1[-(g_DisplayListCommandCount_004495d4 + 1)].cmdA =
				(long)g_DisplayListCommandCount_004495d4 | 0x1000000000008000;
			pRVar1[-(g_DisplayListCommandCount_004495d4 + 1)].cmdB = 0xe;
			g_DisplayListCommandCount_004495d4 = 0;
		}
		return;
	}

	void ApplyFlag_0029f1e0(byte* param_1, uint offset, uint flag)
	{
		if (offset < *param_1) {
			uint* a = (uint*)(*(int*)(param_1 + offset * 4 + 0x10) + 0x10);

#ifdef PLATFORM_WIN
			auto value = ed3D::TextureSection::ResolveTextureSectionKey((int)a);
			if (value.has_value())
			{
				a = (uint*)*value;
			}
#endif
			*a = *a | flag;
		}
		return;
	}

	void FUN_002ca8c0(uint mode)
	{
		if ((((g_DrawEnded_004496c0 == '\0') &&
			(g_DisplayListMode_00449658 = mode & 0xff,
				g_pMaterialInfo_00449644 != (MaterialInfo*)0x0)) &&
			((byte*)g_pMaterialInfo_00449644->matSubsectionStart != (byte*)0x0)) &&
			(g_DisplayListMode_00449658 == 1)) {
			ApplyFlag_0029f1e0((byte*)g_pMaterialInfo_00449644->matSubsectionStart, 0, 4);
		}
		return;
	}

	char* LoadFromMaterialData_002cbad0(char* pMaterialSubSection, int offset, bool* bSuccess, char** pOutAddr)
	{
		char* v0;
		char* a1_minus10;
		char* iVar1;

		*bSuccess = false;
		*pOutAddr = (char*)0x0;
		if (pMaterialSubSection != (char*)0x0) {
			if (*pMaterialSubSection == '\0') {
				return (char*)0x0;
			}
			iVar1 = *(char**)(pMaterialSubSection + offset * 4 + 0x10);

#ifdef PLATFORM_WIN
			auto value = ed3D::TextureSection::ResolveTextureSectionKey((int)iVar1);
			if (value.has_value())
			{
				iVar1 = (char*)*value;
			}
#endif

			if (((iVar1 != (char*)0xfffffff0) && (iVar1 != (char*)0xfffffff0)) && (*(short*)(iVar1 + 0x2c) != 0)) {
				a1_minus10 = *(char**)(iVar1 + 0x30);
				v0 = (char*)0x0;
				if (a1_minus10 == (char*)0xfffffff0) {
					return (char*)0x0;
				}

#ifdef PLATFORM_WIN
				value = ed3D::TextureSection::ResolveTextureSectionKey((int)a1_minus10);
				if (value.has_value())
				{
					a1_minus10 = (char*)*value;
				}
#endif

				if (*(int*)(a1_minus10 + 0x20) != 0) {
					*bSuccess = true;
					if (*(int*)(a1_minus10 + (uint) * (ushort*)(iVar1 + 0x2e) * 0x10 + 0x38) != 0) {
						char* v1 = (char*)*(int*)(a1_minus10 + (uint) * (ushort*)(iVar1 + 0x2e) * 0x10 + 0x38);

#ifdef PLATFORM_WIN
						value = ed3D::TextureSection::ResolveTextureSectionKey((int)v1);
						if (value.has_value())
						{
							v1 = (char*)*value;
						}
#endif
						char* keyA = (char*)*(int*)(v1 + 0x8);

#ifdef PLATFORM_WIN
						value = ed3D::TextureSection::ResolveTextureSectionKey((int)keyA);
						if (value.has_value())
						{
							keyA = (char*)*value;
						}
#endif

						v0 = (char*)(keyA + 0x10);
					}
					if (*(int*)(a1_minus10 + 0x18) == 0) {
						return v0;
					}

					char* tempC = (char*)(*(int*)(a1_minus10 + 0x18));

#ifdef PLATFORM_WIN
					value = ed3D::TextureSection::ResolveTextureSectionKey((int)tempC);
					if (value.has_value())
					{
						tempC = (char*)*value;
					}
#endif

					tempC = (char*)* (int*)(tempC + 0x8);
#ifdef PLATFORM_WIN
					value = ed3D::TextureSection::ResolveTextureSectionKey((int)tempC);
					if (value.has_value())
					{
						tempC = (char*)*value;
					}
#endif

					* pOutAddr = (tempC + 0x10);
					return v0;
				}
				assert(false);
				if (*(int*)(a1_minus10 + 0x18) == 0) {
					return (char*)0x0;
				}
				return (char*)(*(int*)(*(int*)(a1_minus10 + 0x18) + 8) + 0x10);
			}
		}
		return (char*)0x0;
	}

	void RenderCommand_002ca540(ulong cmdA, ulong cmdB)
	{
		ushort uVar1;
		DisplayListInternalSubObj_60* pDVar2;
		int iVar3;
		DisplayListInternal* pDVar4;
		int iVar5;
		RenderCommand* pRVar6;

		pDVar4 = g_DisplayListEnd_004495d8;
		iVar3 = g_DisplayListCommandCount_004495d4;
		g_DisplayListEnd_004495d8 = pDVar4;
		if (g_DrawEnded_004496c0 == '\0') {
			if ((g_DisplayListEnd_004495d8->flags_0x0 & 0x100) == 0) {
				pRVar6 = g_DisplayListEnd_004495d8->pRenderCommands;
				if (g_DisplayListCommandCount_004495d4 == 0) {
					if ((g_DisplayListEnd_004495d8->flags_0x0 & 1) != 0) {
						uVar1 = g_DisplayListEnd_004495d8->subCommandBufferCount;
						pDVar2 = g_DisplayListEnd_004495d8->pDisplayListInternalSubObj;
						pDVar2[uVar1].type_0x42 = 9;
						pDVar2[uVar1].pRenderInput = pRVar6;
					}
					pRVar6->cmdA = 0x1000000000008001;
					pRVar6->cmdB = 0xe;
					pRVar6 = pRVar6 + 1;
				}
				g_DisplayListCommandCount_004495d4 = g_DisplayListCommandCount_004495d4 + 1;
				pRVar6->cmdA = cmdA;
				pRVar6->cmdB = cmdB;
				g_DisplayListEnd_004495d8->pRenderCommands = pRVar6 + 1;
			}
			else {
				iVar5 = 0;
				do {
					g_DisplayListEnd_004495d8 = g_CurrentDisplayListBase_004495dc + iVar5;
					pRVar6 = g_DisplayListEnd_004495d8->pRenderCommands;
					g_DisplayListCommandCount_004495d4 = iVar3;
					if (iVar3 == 0) {
						if ((g_DisplayListEnd_004495d8->flags_0x0 & 1) != 0) {
							uVar1 = g_DisplayListEnd_004495d8->subCommandBufferCount;
							pDVar2 = g_DisplayListEnd_004495d8->pDisplayListInternalSubObj;
							pDVar2[uVar1].type_0x42 = 9;
							pDVar2[uVar1].pRenderInput = pRVar6;
						}
						pRVar6->cmdA = 0x1000000000008001;
						pRVar6->cmdB = 0xe;
						pRVar6 = pRVar6 + 1;
					}
					iVar5 = iVar5 + 1;
					g_DisplayListCommandCount_004495d4 = g_DisplayListCommandCount_004495d4 + 1;
					pRVar6->cmdA = cmdA;
					pRVar6->cmdB = cmdB;
					g_DisplayListEnd_004495d8->pRenderCommands = pRVar6 + 1;
					g_DisplayListEnd_004495d8 = pDVar4;
				} while (iVar5 < 2);
			}
		}
		return;
	}

	void RenderCommand_002ca6a0
	(ulong param_1, ulong param_2, ulong param_3, ulong param_4, ulong param_5, ulong param_6, ulong param_7,
		ulong param_8)
	{
		RenderCommand_002ca540
		((param_8 & 0xffffffff) << 0x11 |
			(param_7 & 0xff) << 0x10 |
			(param_6 & 0xffffffff) << 0xf |
			(param_5 & 0xff) << 0xe |
			(param_4 & 0xffffffff) << 0xc | (param_3 & 0xffffffff) << 4 | param_1 & 0xff | (param_2 & 0xffffffff) << 1,
			0x47);
		return;
	}

	void RenderCommand_002ca800(void)
	{
		RenderCommand_002ca8c0(1);
		RenderCommand_002ca540(0x44, 0x42);
		return;
	}

	void RenderCommand_002ca8c0(uint mode)
	{
		if ((((g_DrawEnded_004496c0 == '\0') &&
			(g_DisplayListMode_00449658 = mode & 0xff, g_pMaterialInfo_00449644 != (MaterialInfo*)0x0)) &&
			((byte*)g_pMaterialInfo_00449644->matSubsectionStart != (byte*)0x0)) && (g_DisplayListMode_00449658 == 1)) {
			ApplyFlag_0029f1e0((byte*)g_pMaterialInfo_00449644->matSubsectionStart, 0, 4);
		}
		return;
	}


	void RenderCommand_002ca930(CameraObj_28* pCamera)
	{
		if (g_DrawEnded_004496c0 == '\0') {
			g_Width_004495ec = (0x800 - ((int)(uint)pCamera->pVidModeData30_0x10->pVidModeData_0x0->screenWidth >> 1)) * 0x10;
			g_Height_004495f0 = (0x800 - ((int)(uint)pCamera->pVidModeData30_0x10->pVidModeData_0x0->screenHeight >> 1)) * 0x10;
			g_Camera_004495e8 = pCamera;
			if ((((g_DisplayListEnd_004495d8 != (DisplayListInternal*)0x0) &&
				(g_DisplayListEnd_004495d8->subCommandBufferCount != 0)) && ((g_DisplayListEnd_004495d8->flags_0x0 & 2) != 0))
				&& (g_DisplayListEnd_004495d8->pDisplayListInternalSubObj->aCommandArray
					[g_DisplayListEnd_004495d8->subCommandBufferCount - 1].type == 0)) {
				RenderCommand_002ca540
				((long)((0x1000 - ((int)pCamera->screenWidth + (int)pCamera->posX) >> 1) << 4) & 0xffffffffU |
					(long)((0x1000 - ((int)pCamera->screenHeight + (int)pCamera->posY) >> 1) << 4) << 0x20, 0x18);
				if (g_Camera_004495e8->screenHeight == 0x1c0) {
					RenderCommand_002ca540
					((long)(g_Camera_004495e8->posY + 0x1ff) << 0x30 |
						(long)g_Camera_004495e8->posY << 0x20 |
						(long)g_Camera_004495e8->posX |
						(long)((int)g_Camera_004495e8->screenWidth + (int)g_Camera_004495e8->posX + -1) << 0x10, 0x40);
				}
				else {
					RenderCommand_002ca540
					((long)g_Camera_004495e8->posY << 0x20 |
						(long)g_Camera_004495e8->posX |
						(long)((int)g_Camera_004495e8->screenWidth + (int)g_Camera_004495e8->posX + -1) << 0x10 |
						(long)((int)g_Camera_004495e8->screenHeight + (int)g_Camera_004495e8->posY + -1) << 0x30, 0x40);
				}
			}
		}
		return;
	}

	void LoadMaterialResource_002cb850(MaterialInfo* pMaterialInfo)
	{
		RenderCommand* pRVar1;
		RenderCommand* pRVar2;
		char* pcVar3;
		RenderCommand* pRVar4;
		RenderCommandUint* pDVar4;
		DisplayListInternalSubObj_60* pDVar5;
		RenderCommandUint* pDVar6;
		char* local_8;
		bool local_1;

		Mode_0044964c = 0;
		g_pMaterialInfo_00449644 = pMaterialInfo;
		ColourCommandSetup_002c9fd0();
		if ((g_DisplayListEnd_004495d8->flags_0x0 & 2) == 0) {
			return;
		}
		pDVar5 = g_DisplayListEnd_004495d8->pDisplayListInternalSubObj;
		pRVar1 = g_DisplayListEnd_004495d8->pRenderCommands;
		if (g_DisplayListEnd_004495d8->subCommandBufferCount != 0) {
			pDVar4 = pDVar5->aCommandArray + (g_DisplayListEnd_004495d8->subCommandBufferCount - 1);
			pDVar4->size = (uint)((int)pRVar1 - (int)pDVar4->pCommandBuffer);
			pDVar5 = (DisplayListInternalSubObj_60*)(pDVar4 + 1);
		}
		if (pMaterialInfo != (MaterialInfo*)0x0) {
			if ((pMaterialInfo->mode & 4U) == 0) {
				if (pMaterialInfo->matSubsectionStart != (char*)0x0) {
					local_1 = false;
					local_8 = (char*)0x0;
					pcVar3 = LoadFromMaterialData_002cbad0(pMaterialInfo->matSubsectionStart, 0, &local_1, &local_8);
					pDVar6 = (RenderCommandUint*)pDVar5;
					if (pcVar3 != (char*)0x0) {
						char* tempA = (char*)*(int*)(pcVar3 + 8);

#ifdef PLATFORM_WIN
						auto value = ed3D::TextureSection::ResolveTextureSectionKey((int)tempA);
						if (value.has_value())
						{
							tempA = (char*)*value;
						}
#endif

						pDVar5->aCommandArray[0].pCommandBuffer = (RenderCommand*)(tempA + 0x40);
						pDVar5->aCommandArray[0].type = 2;
						if ((local_1 != false) && (0x3ffff < (int)((uint) * (ushort*)local_8 * (uint) * (ushort*)(local_8 + 2)))) {
							pRVar2 = pDVar5->aCommandArray[0].pCommandBuffer;
							pRVar4 = pRVar2 + 0x12;
							if ((uint) * (ushort*)local_8 * (uint) * (ushort*)(local_8 + 2) == 0x40000) {
								pRVar4 = pRVar2 + 10;
							}
							*(uint*)((int)&pRVar4->cmdA + 4) = *(uint*)((int)&pRVar4->cmdA + 4) & 0xffffc000 | 0x800;
						}
						pDVar6 = pDVar5->aCommandArray + 1;
						g_DisplayListEnd_004495d8->subCommandBufferCount = g_DisplayListEnd_004495d8->subCommandBufferCount + 1;
					}

					char* tempA = (char*)*(int*)(pMaterialInfo->matSubsectionStart + 8);

#ifdef PLATFORM_WIN
					auto value = ed3D::TextureSection::ResolveTextureSectionKey((int)tempA);
					if (value.has_value())
					{
						tempA = (char*)*value;
					}
#endif

					pDVar6->pCommandBuffer = (RenderCommand*)(tempA + 0x10);
					if ((local_1 != false) && (0x3ffff < (int)((uint) * (ushort*)local_8 * (uint) * (ushort*)(local_8 + 2)))) {
						*(uint*)((int)&pDVar6->pCommandBuffer[1].cmdA + 4) =
							*(uint*)((int)&pDVar6->pCommandBuffer[1].cmdA + 4) & 0xfffc001f | 0x10000;
					}
					pDVar6->type = 5;
					pDVar6->size = *(int*)(pMaterialInfo->matSubsectionStart + 0xc) * 0x10 - 0x10;
					g_DisplayListEnd_004495d8->subCommandBufferCount = g_DisplayListEnd_004495d8->subCommandBufferCount + 1;
					pDVar5 = (DisplayListInternalSubObj_60*)(pDVar6 + 1);
					if (*pMaterialInfo->matSubsectionStart != '\0') {
						FUN_002ca8c0(1);
						pDVar6[1].pCommandBuffer = pRVar1;
						goto LAB_002cba4c;
					}
					FUN_002ca8c0(0);
				}
			}
			else {
				pDVar5->aCommandArray[0].pCommandBuffer = (RenderCommand*)0x0;
				pDVar5->aCommandArray[0].type = 4;
				pDVar5 = (DisplayListInternalSubObj_60*)(pDVar5->aCommandArray + 1);
				g_DisplayListEnd_004495d8->subCommandBufferCount = g_DisplayListEnd_004495d8->subCommandBufferCount + 1;
			}
		}
		pDVar5->aCommandArray[0].pCommandBuffer = pRVar1;
	LAB_002cba4c:
		pDVar5->aCommandArray[0].type = 0;
		pDVar5->aCommandArray[0].size = 0;
		g_DisplayListEnd_004495d8->subCommandBufferCount = g_DisplayListEnd_004495d8->subCommandBufferCount + 1;
		if ((g_DisplayListEnd_004495d8->subCommandBufferCount == 1) || (g_DisplayListEnd_004495d8->subCommandBufferCount == 3)
			) {
			RenderCommand_002ca6a0(1, 7, 0, 0, 0, 0, 1, 1);
			RenderCommand_002ca930(g_Camera_004495e8);
		}
		return;
	}

	bool SetActiveDisplayList_00290960(CameraPanMasterHeader* param_1, DisplayListInternal* param_2)
	{
		bool bVar1;
		MeshTransformParentHeader* pMVar2;
		MeshTransformSpecial* pMVar3;

		pMVar2 = param_1->pCameraPanHeader_0xc;
		bVar1 = pMVar2->totalCount != pMVar2->usedCount;
		if (bVar1) {
			pMVar3 = pMVar2->field_0x4;
			pMVar3->field_0x0 = 2;
			pMVar3->pRenderInput = param_2;
		}
		return bVar1;
	}

	MeshTransformSpecial* FUN_002909a0(CameraPanMasterHeader* param_1, MeshTransformSpecial* param_2)
	{
		MeshTransformParentHeader* pMVar1;
		MeshTransformSpecial* pMVar2;

		pMVar1 = param_1->pCameraPanHeader_0xc;
		pMVar1->usedCount = pMVar1->usedCount + 1;
		param_1->count_0x14 = param_1->count_0x14 + 1;
		pMVar2 = pMVar1->field_0x4;
		pMVar1->field_0x4 = pMVar2->pNext_0x4;
		pMVar2->field_0x0 = 0;
		pMVar2->field_0x8 = param_2->field_0x8;
		pMVar2->pNext_0x4 = param_2;
		param_2->field_0x8->pNext_0x4 = pMVar2;
		param_2->field_0x8 = pMVar2;
		return pMVar2;
	}

	bool WillSetActiveDisplayList_00290cb0(CameraPanMasterHeader* pCameraPanHeader, DisplayListInternal* pDisplayList)
	{
		CameraPanMasterHeader* pCVar1;
		bool bVar2;
		MeshTransformSpecial* pMVar3;
		long lVar4;

		bVar2 = SetActiveDisplayList_00290960(pCameraPanHeader, pDisplayList);
		if (bVar2 == false) {
			bVar2 = false;
		}
		else {
			if (pCameraPanHeader->headerUnion.field0 == 1) {
				pMVar3 = FUN_002909a0(pCameraPanHeader, (MeshTransformSpecial*)pCameraPanHeader);
				pCameraPanHeader->pCameraPanStatic_0x8 = (MeshTransformParent*)pMVar3;
			}
			else {
				if (pCameraPanHeader->headerUnion.field0 == 0) {
					pMVar3 = FUN_002909a0(pCameraPanHeader, pCameraPanHeader->pLoadedData);
					pCameraPanHeader->pLoadedData = pMVar3;
				}
				else {
					pCVar1 = (CameraPanMasterHeader*)pCameraPanHeader->pLoadedData;
					assert(false);
					//while (((pCVar1 != pCameraPanHeader && (*(ushort*)&pCVar1->headerUnion != 1)) &&
						//(lVar4 = (*(code*)pCameraPanHeader->headerUnion.field1)
						//	(pDisplayList, (DisplayListInternal*)pCVar1->pCameraPanHeader_0xc), -1 < lVar4))) {
						//pCVar1 = (CameraPanMasterHeader*)pCVar1->pLoadedData;
					//}
					FUN_002909a0(pCameraPanHeader, (MeshTransformSpecial*)pCVar1);
				}
			}
			bVar2 = true;
		}
		return bVar2;
	}

	void WillSetActiveDisplayList_002cac70(DisplayListInternal* param_1)
	{
		ushort uVar1;
		DisplayListInternal* pDisplayList;

		pDisplayList = param_1 + g_DisplayListGlobal_00449664;
		if ((param_1[g_DisplayListGlobal_00449664].flags_0x0 & 4) != 0) {
			param_1->field_0x3 = 0;
			pDisplayList = param_1;
		}
		if (pDisplayList->subCommandBufferCount == 0) {
			pDisplayList->pRenderCommands = pDisplayList->field_0x14;
			pDisplayList->field_0x10 = (MeshDrawRenderCommand*)pDisplayList->field_0xc;
		}
		else {
			pDisplayList->field_0x3 = 1;
			uVar1 = pDisplayList->flags_0x0;
			if ((uVar1 & 1) == 0) {
				if ((uVar1 & 2) != 0) {
					if ((uVar1 & 0x20) == 0) {
						WillSetActiveDisplayList_00290cb0
						(DisplayListData_004496a0.pCameraPanPairA[g_DisplayListGlobal_00449664],
							pDisplayList);
					}
					else {
						WillSetActiveDisplayList_00290cb0
						(DisplayListData_004496a0.pCameraPanPairB[g_DisplayListGlobal_00449664],
							pDisplayList);
					}
				}
			}
			else {
				DisplayListData_004496a0.pDisplayListArray[g_DisplayListGlobal_00449664]->field_0x0
					[DisplayListData_004496a0.LoadedDisplayListCount[g_DisplayListGlobal_00449664]] = pDisplayList;
				DisplayListData_004496a0.LoadedDisplayListCount[g_DisplayListGlobal_00449664] =
					DisplayListData_004496a0.LoadedDisplayListCount[g_DisplayListGlobal_00449664] + 1;
			}
		}
		return;
	}

	int g_Mode_0044966c = 0;
	int g_Count_004495f4 = 0;
	int g_Count_004495f8 = 0;

	int Mode_00449694 = 0;

	typedef void (*DisplayListFourFloats)(float, float, float, int);
	typedef void (*DisplayListColourFunction)(byte, byte, byte, byte);
	typedef void (*DisplayListTwoFloats)(float, float);

	DisplayListFourFloats g_pDisplayListFunction_004496e8 = NULL;
	DisplayListColourFunction g_pDisplayListColurFunction_004496ec = NULL;
	DisplayListTwoFloats g_pDisplayListFunction_004496f0 = NULL;

	void DeferredCommand_002cd4d0(float x, float y, float z, int param_4)
	{
		DisplayListInternal* pDVar1;
		int iVar2;
		int iVar3;
		RenderCommand** ppRVar4;
		Vector local_10;

		local_10.w = 1.0;
		local_10.z = 0.0;
		local_10.x = x;
		local_10.y = y;
#ifdef PLATFORM_PS2
		sceVu0ApplyMatrix((float*)&local_10, (float(*)[4])g_DisplayListMatrix_004595e4 + g_DisplayListMatrixCount_004495e0, (float*)&local_10);
#endif
		pDVar1 = g_DisplayListEnd_004495d8;
		iVar3 = g_Width_004495ec + (int)(local_10.x * 16.0);
		iVar2 = g_Height_004495f0 + (int)(local_10.y * 16.0);
		if (g_Mode_0044966c == 8) {
			g_Count_004495f8 = g_Count_004495f8 + 1;
			if ((g_Count_004495f8 == 1) || (g_Count_004495f8 == 2)) {
				g_DisplayListEnd_004495d8->pRenderCommands->cmdA = (long)(int)z << 0x20 | (long)iVar3 | (long)iVar2 << 0x10;
				pDVar1->pRenderCommands->cmdB = 0xd;
				pDVar1->pRenderCommands = pDVar1->pRenderCommands + 1;
			}
			else {
				g_DisplayListEnd_004495d8->pRenderCommands->cmdA = (long)(int)z << 0x20 | (long)iVar3 | (long)iVar2 << 0x10;
				pDVar1->pRenderCommands->cmdB = 5;
				pDVar1->pRenderCommands = pDVar1->pRenderCommands + 1;
				if (g_Count_004495f8 == 4) {
					g_Count_004495f8 = 0;
				}
			}
		}
		else {
			ppRVar4 = &g_DisplayListEnd_004495d8->pRenderCommands;
			g_DisplayListEnd_004495d8->pRenderCommands->cmdA = (long)(int)z << 0x20 | (long)iVar3 | (long)iVar2 << 0x10;
			if (param_4 == 0xc000) {
				(*ppRVar4)->cmdB = 0xd;
			}
			else {
				(*ppRVar4)->cmdB = 5;
			}
			*ppRVar4 = *ppRVar4 + 1;
		}
		g_Count_004495f4 = g_Count_004495f4 + 1;
		return;
	}

	struct ByteColor {
		byte r;
		byte g;
		byte b;
		byte a;
	};

	ByteColor ByteColor_ARRAY_0048dd40[4];

	ByteColor* PTR_ByteColor_ARRAY_00448aa0 = ByteColor_ARRAY_0048dd40;

	void DeferredCommand_002ce000(byte r, byte g, byte b, byte a)
	{
		DisplayListInternal* pDVar1;

		PTR_ByteColor_ARRAY_00448aa0->r = r;
		PTR_ByteColor_ARRAY_00448aa0->g = g;
		PTR_ByteColor_ARRAY_00448aa0->b = b;
		PTR_ByteColor_ARRAY_00448aa0->a = a;
		pDVar1 = g_DisplayListEnd_004495d8;
		g_DisplayListEnd_004495d8->pRenderCommands->cmdA = (a & 0xff) << 0x18 | (b & 0xff) << 0x10 | r & 0xff | (g & 0xff) << 8 | 0x3f80000000000000;
		pDVar1->pRenderCommands->cmdB = 1;
		pDVar1->pRenderCommands = pDVar1->pRenderCommands + 1;
		return;
	}

	void DeferredCommand_002cde90(float param_1, float param_2)
	{
		DisplayListInternal* pDVar1;

		pDVar1 = g_DisplayListEnd_004495d8;
		g_DisplayListEnd_004495d8->pRenderCommands->cmdA = (int)param_2 << 32 | (int)param_1;
		pDVar1->pRenderCommands->cmdB = 2;
		pDVar1->pRenderCommands = pDVar1->pRenderCommands + 1;
		return;
	}

	void RenderCommand_002ceb50(ulong count)
	{
		RenderCommand* pRVar1;
		RenderCommand** ppRVar2;
		uint uVar3;
		long lVar4;

		uVar3 = (uint)g_DisplayListEnd_004495d8->pRenderCommands & 0xf;
		if (uVar3 != 0) {
			g_DisplayListEnd_004495d8->pRenderCommands =
				(RenderCommand*)((int)g_DisplayListEnd_004495d8->pRenderCommands + (0x10 - uVar3));
		}
		ppRVar2 = &g_DisplayListEnd_004495d8->pRenderCommands;
		pRVar1 = g_DisplayListEnd_004495d8->pRenderCommands;
		if ((g_pMaterialInfo_00449644 != (MaterialInfo*)0x0) || (lVar4 = 0, Mode_0044964c != 0)) {
			lVar4 = 1;
		}
		g_CachedRenderCommandPtr_00449650 = pRVar1;
		if (Mode_00449694 == 0) {
			pRVar1->cmdA = ((long)g_DisplayListMode_00449658 << 6 | count & 0xffffffff | lVar4 << 4) << 0x2f |
				0x1000400000008000;
		}
		else {
			pRVar1->cmdA = ((long)g_DisplayListMode_00449658 << 6 | count & 0xffffffff | 8 | lVar4 << 4) << 0x2f |
				0x1000400000008000;
		}
		pRVar1->cmdB = 0xe;
		*ppRVar2 = pRVar1 + 1;
		return;
	}

	void RenderCommand_002ca030(void)
	{
		RenderCommand* pRVar1;
		RenderCommand* pRVar2;
		int iVar3;

		if (g_DisplayListCommandCount_004495d4 != 0) {
			pRVar1 = g_DisplayListEnd_004495d8->pRenderCommands;
			*(undefined4*)&pRVar1->cmdA = 0;
			*(undefined4*)((int)&pRVar1->cmdA + 4) = 0x14000000;
			pRVar1->cmdB = 0;
			pRVar2 = pRVar1 + 1;
			INT_00449654 = g_DisplayListCommandCount_004495d4 + 2;
			g_CachedRenderCommandPtr_00449650 =
				g_DisplayListEnd_004495d8->pDisplayListInternalSubObj[g_DisplayListEnd_004495d8->subCommandBufferCount].
				pRenderInput;
			g_DisplayListEnd_004495d8->pDisplayListInternalSubObj[g_DisplayListEnd_004495d8->subCommandBufferCount].pRenderInput
				= pRVar2;
			iVar3 = INT_00449654 - 1;
			pRVar2->cmdA = ((long)(int)g_CachedRenderCommandPtr_00449650 & 0xfffffffU) << 0x20 |
				(long)(int)(INT_00449654 | 0x30000000);
			pRVar1[1].cmdB = ((long)(iVar3 * 0x10000) & 0xffffffffU | 0x6c0003dc) << 0x20 | 0x40003dc;
			g_DisplayListEnd_004495d8->pRenderCommands = pRVar1 + 2;
			g_DisplayListEnd_004495d8->field_0x10 = (MeshDrawRenderCommand*)g_DisplayListEnd_004495d8->pRenderCommands;
			PTR_MeshDrawRenderCommand_004495fc = g_DisplayListEnd_004495d8->field_0x10;
			g_DisplayListEnd_004495d8->subCommandBufferCount = g_DisplayListEnd_004495d8->subCommandBufferCount + 1;
			iVar3 = -INT_00449654;
			pRVar2[iVar3].cmdA = (long)g_DisplayListCommandCount_004495d4 | 0x1000000000008000;
			pRVar2[iVar3].cmdB = 0xe;
			g_DisplayListCommandCount_004495d4 = 0;
		}
		return;
	}

	ByteColor* g_pByteColour_0044960c = NULL;
	char* PTR_DAT_00449610 = NULL;
	Vector* g_pVector_00449604 = NULL;
	undefined* PTR_DAT_0044962c = NULL;

	void RenderCommand_002cec50(float x, float y, float z, uint param_4, ushort param_5)
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

		v0 = g_DisplayListEnd_004495d8->pDisplayListInternalSubObj + g_DisplayListEnd_004495d8->subCommandBufferCount;
		v0->field_0x50 = param_4;
		v0->field_0x40 = 1;
		//DAT_00448a80 = (uint)g_DisplayListEnd_004495d8->subCommandBufferCount;
		g_DisplayListEnd_004495d8->subCommandBufferCount = g_DisplayListEnd_004495d8->subCommandBufferCount + 1;
		pMVar2 = PTR_MeshDrawRenderCommand_004495fc;
		if (g_DisplayListEnd_004495d8->field_0x2c != (char*)0x0) {
			*(DisplayListInternalSubObj_60**)
				(g_DisplayListEnd_004495d8->field_0x2c + (uint)(ushort)g_DisplayListEnd_004495d8->field_0x6 * 4) = v0;
			g_DisplayListEnd_004495d8->field_0x6 = g_DisplayListEnd_004495d8->field_0x6 + 1;
		}
		if (g_pMaterialInfo_00449644 == (MaterialInfo*)0x0) {
			PTR_MeshDrawRenderCommand_004495fc->field_0x4 = -1;
		}
		else {
			PTR_MeshDrawRenderCommand_004495fc->field_0x4 = (short)g_pMaterialInfo_00449644->Length;
		}
		PTR_MeshDrawRenderCommand_004495fc->field_0x6 = -1;
		PTR_MeshDrawRenderCommand_004495fc->field_0x39 = 0;
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
		PTR_MeshDrawRenderCommand_004495fc->field_0x3a = uVar9;
		PTR_MeshDrawRenderCommand_004495fc->field_0x38 = (byte)iVar6;
		PTR_MeshDrawRenderCommand_004495fc->field_0x0 = 0x4000000;
		if (BYTE_00449678 != 0) {
			PTR_MeshDrawRenderCommand_004495fc->field_0x0 = PTR_MeshDrawRenderCommand_004495fc->field_0x0 | 0x10;
		}
		if (SHORT_00449698 == 0) {
			PTR_MeshDrawRenderCommand_004495fc->field_0x30 = 0;
		}
		else {
			PTR_MeshDrawRenderCommand_004495fc->field_0x30 = SHORT_00449698;
		}
		sVar4 = SHORT_0044969c;
		if (SHORT_0044969c == 0) {
			PTR_MeshDrawRenderCommand_004495fc->field_0x32 = 0;
		}
		else {
			BYTE_004496e4 = 1;
			PTR_MeshDrawRenderCommand_004495fc->field_0x0 = PTR_MeshDrawRenderCommand_004495fc->field_0x0 | 0x2000;
			PTR_MeshDrawRenderCommand_004495fc->field_0x32 = sVar4;
		}
		(PTR_MeshDrawRenderCommand_004495fc->field_0x10).x = x;
		(PTR_MeshDrawRenderCommand_004495fc->field_0x10).y = y;
		(PTR_MeshDrawRenderCommand_004495fc->field_0x10).z = z;
		(PTR_MeshDrawRenderCommand_004495fc->field_0x10).w = 100000.0;
		iVar6 = param_4 + ((int)(param_4 + 1) / 0x48) * 2;
		uVar7 = iVar6 * 4 + 0x40;
		if ((uVar7 & 0xf) != 0) {
			uVar7 = uVar7 + (0x10 - (uVar7 & 0xf));
		}
		uVar5 = iVar6 * 4 + 0x40;
		if ((uVar5 & 0xf) != 0) {
			uVar5 = uVar5 + (0x10 - (uVar5 & 0xf));
		}
		if ((g_DisplayListEnd_004495d8->flags_0x0 & 0x100) == 0) {
			PTR_MeshDrawRenderCommand_004495fc->field_0x3c = (MeshDrawRenderCommand*)0x0;
			PTR_MeshDrawRenderCommand_004495fc->field_0x20 = (char*)(PTR_MeshDrawRenderCommand_004495fc + 1);
			uVar11 = (uint)PTR_MeshDrawRenderCommand_004495fc->field_0x20 & 0xf;
			if (uVar11 != 0) {
				PTR_MeshDrawRenderCommand_004495fc->field_0x20 = PTR_MeshDrawRenderCommand_004495fc->field_0x20 + (0x10 - uVar11);
			}
		}
		else {
			PTR_MeshDrawRenderCommand_004495fc->field_0x3c = PTR_MeshDrawRenderCommand_004495fc + 1;
			uVar11 = (uint)PTR_MeshDrawRenderCommand_004495fc->field_0x3c & 0xf;
			if (uVar11 != 0) {
				PTR_MeshDrawRenderCommand_004495fc->field_0x3c =
					(MeshDrawRenderCommand*)((int)PTR_MeshDrawRenderCommand_004495fc->field_0x3c + (0x10 - uVar11));
			}
			PTR_MeshDrawRenderCommand_004495fc->field_0x20 =
				(char*)(&PTR_MeshDrawRenderCommand_004495fc->field_0x3c->field_0x0 + (uint)uVar9 * 4);
			v0->field_0x41 = (byte)uVar9;
			uVar11 = (uint)PTR_MeshDrawRenderCommand_004495fc->field_0x20 & 0xf;
			if (uVar11 != 0) {
				PTR_MeshDrawRenderCommand_004495fc->field_0x20 = PTR_MeshDrawRenderCommand_004495fc->field_0x20 + (0x10 - uVar11);
			}
		}
		PTR_MeshDrawRenderCommand_004495fc->field_0x24 = PTR_MeshDrawRenderCommand_004495fc->field_0x20 + uVar7;
		PTR_MeshDrawRenderCommand_004495fc->field_0x24 = PTR_MeshDrawRenderCommand_004495fc->field_0x24 + 0x10;
		uVar11 = (uint)PTR_MeshDrawRenderCommand_004495fc->field_0x24 & 0xf;
		if (uVar11 != 0) {
			PTR_MeshDrawRenderCommand_004495fc->field_0x24 = PTR_MeshDrawRenderCommand_004495fc->field_0x24 + (0x10 - uVar11);
		}
		PTR_MeshDrawRenderCommand_004495fc->field_0x28 = PTR_MeshDrawRenderCommand_004495fc->field_0x24 + uVar5;
		uVar11 = (uint)PTR_MeshDrawRenderCommand_004495fc->field_0x28 & 0xf;
		if (uVar11 != 0) {
			PTR_MeshDrawRenderCommand_004495fc->field_0x28 = PTR_MeshDrawRenderCommand_004495fc->field_0x28 + (0x10 - uVar11);
		}
		g_pByteColour_0044960c = (ByteColor*)PTR_MeshDrawRenderCommand_004495fc->field_0x24;
		PTR_DAT_00449610 = PTR_MeshDrawRenderCommand_004495fc->field_0x20 + 0x10;
		g_pVector_00449604 = (Vector*)PTR_MeshDrawRenderCommand_004495fc->field_0x28;
		if ((param_4 & 3) != 0) {
			param_4 = param_4 + (4 - (param_4 & 3));
		}
		pMVar10 = (MeshDrawRenderCommand*)
			((int)(MeshDrawRenderCommand*)(g_pVector_00449604 + param_4) - (int)PTR_MeshDrawRenderCommand_004495fc);
		ppMVar1 = (MeshDrawRenderCommand**)&PTR_MeshDrawRenderCommand_004495fc->field_0x8;
		PTR_MeshDrawRenderCommand_004495fc = (MeshDrawRenderCommand*)(g_pVector_00449604 + param_4);
		PTR_DAT_0044962c = (undefined*)g_pVector_00449604;
		*ppMVar1 = pMVar10;
		pMVar2->field_0x2c = (char*)0x0;
		pVVar3 = g_pVector_00449604;
		g_pVector_00449604->w = 6.887662e-41;
		pVVar3[1].w = 6.887662e-41;
		//DAT_00449624 = 1;
		//DAT_00449618 = 1;
		//DAT_0044961c = 0;
		//PTR_DAT_00449630 = &g_pByteColour_0044960c->r + uVar5;
		//DAT_00449620 = 0;
		//g_CharacterIndex_00449614 = 0;
		//PTR_DAT_00449628 = (undefined*)PTR_MeshDrawRenderCommand_004495fc;
		//PTR_DAT_00449634 = PTR_DAT_00449610 + uVar7;
		v0->pRenderInput = (RenderCommand*)pMVar2;
		if (((uint)PTR_MeshDrawRenderCommand_004495fc & 0xf) != 0) {
			PTR_MeshDrawRenderCommand_004495fc =
				(MeshDrawRenderCommand*)
				((int)PTR_MeshDrawRenderCommand_004495fc + (0x10 - ((uint)PTR_MeshDrawRenderCommand_004495fc & 0xf)));
		}
		v0->type_0x42 = param_5;
		v0->field_0x4c = g_Mode_0044966c;
		sceVu0CopyMatrix((float(*)[4])v0, (float(*)[4])g_DisplayListMatrix_004595e4 + g_DisplayListMatrixCount_004495e0);
		//DAT_0044935c = 0x48;
		return;
	}

	void MeshDrawCommands_002ca170(float x, float y, float z, ulong mode, int count)
	{
		RenderCommand* pRVar1;
		int iVar2;
		MeshDrawRenderCommand* pcVar3;
		DisplayListInternal* pDVar4;
		uint uVar5;
		DisplayListInternalSubObj_60* pDVar6;

		pcVar3 = PTR_MeshDrawRenderCommand_004495fc;
		iVar2 = g_DisplayListCommandCount_004495d4;
		if (((g_DisplayListEnd_004495d8->flags_0x0 & 0x100) != 0) &&
			(pDVar4 = g_CurrentDisplayListBase_004495dc + g_DisplayListGlobal_00449668, g_DisplayListEnd_004495d8 != pDVar4)) {
			PTR_MeshDrawRenderCommand_004495fc = (MeshDrawRenderCommand*)pDVar4->field_0x10;
			g_DisplayListEnd_004495d8 = pDVar4;
			MeshDrawCommands_002ca170(x, y, z, mode, count);
			g_DisplayListEnd_004495d8->field_0x10 = PTR_MeshDrawRenderCommand_004495fc;
			g_DrawEnded_004496c0 = 0;
			g_DisplayListEnd_004495d8 = g_CurrentDisplayListBase_004495dc + g_DisplayListGlobal_00449664;
		}
		uVar5 = (uint)mode;
		g_Mode_0044966c = uVar5 & 0xff;
		g_Count_004495f4 = 0;
		//DAT_00449640 = 0;
		g_DisplayListCommandCount_004495d4 = iVar2;
		PTR_MeshDrawRenderCommand_004495fc = pcVar3;
		//DAT_0044963c = count;
		if ((g_DisplayListEnd_004495d8->flags_0x0 & 1) == 0) {
			if (iVar2 != 0) {
				pRVar1 = g_DisplayListEnd_004495d8->pRenderCommands;
				pRVar1[-(iVar2 + 1)].cmdA = (long)iVar2 | 0x1000000000008000;
				pRVar1[-(iVar2 + 1)].cmdB = 0xe;
				g_DisplayListCommandCount_004495d4 = 0;
			}
			switch ((int)(mode & 0xff)) {
			case 0:
			case 1:
			case 2:
			case 3:
			case 4:
			case 5:
			case 6:
				RenderCommand_002ceb50(mode & 0xff);
				break;
			default:
				RenderCommand_002ceb50(4);
				break;
			case 8:
				g_Count_004495f8 = 0;
				RenderCommand_002ceb50(4);
			}
			g_pDisplayListFunction_004496e8 = DeferredCommand_002cd4d0;
			g_pDisplayListColurFunction_004496ec = DeferredCommand_002ce000;
			g_pDisplayListFunction_004496f0 = DeferredCommand_002cde90;
			g_DrawEnded_004496c0 = 1;
			return;
		}
		RenderCommand_002ca030();
		if (((0 < INT_00449680) && ((mode & 0xff) == 0xb)) || ((mode & 0xff) == 0xc)) {
			assert(false);
			//FUN_002d00c0();
		}
		pDVar6 = g_DisplayListEnd_004495d8->pDisplayListInternalSubObj + g_DisplayListEnd_004495d8->subCommandBufferCount;
		if (INT_004496d8 < 1) {
			pDVar6->field_0x54 = 0;
			pDVar6->field_0x58 = (Matrix*)0x0;
		}
		else {
			pDVar6->field_0x58 = PTR_004496d4;
			pDVar6->field_0x54 = INT_004496d8;
			PTR_004496d4 = (Matrix*)0x0;
			INT_004496d8 = 0;
		}
		switch (uVar5 & 0xff) {
			case 0:
			case 2:
			case 4:
				RenderCommand_002cec50(x, y, z, count, (ushort)mode & 0xff);
				break;
			case 3:
			case 8:
				RenderCommand_002cec50(x, y, z, count, 4);
				goto LAB_002ca38c;
			default:
				mode = 4;
				break;
			//case 6:
			//case 7:
			//case 0xb:
			//case 0xc:
			//	DAT_0044963c = DAT_0044963c << 2;
			//	FUN_002cf060(count, uVar5 & 0xff);
			//	break;
			case 10:
				count = count + 1;
				//DAT_0044963c = DAT_0044963c + 1;
			case 1:
				RenderCommand_002cec50(x, y, z, count, 2);
		}
		uVar5 = (uint)mode;
	LAB_002ca38c:
		uVar5 = uVar5 & 0xff;
		//g_DrawEnded_004496c0 = 1;
		//g_pDisplayListFunction_004496e8 = g_aDisplayListFunctions_004250f0[uVar5];
		//g_pDisplayListColurFunction_004496ec = g_aDisplayListColourFunctions_00425130[uVar5];
		//g_pDisplayListFunction_004496f0 = g_aDisplayListFunctions_00425170[uVar5];
		return;
	}

	void SetDropShadowColour_002ce1a0(byte r, byte g, byte b, byte a)
	{
		DisplayListInternal* pDVar1;
		RenderCommand* pRVar2;

		if (g_pDisplayListColurFunction_004496ec == (DisplayListColourFunction)0x0) {
			assert(false);
			//PTR_ByteColor_ARRAY_00448aa0[g_CharacterIndex_00449614].r = r;
			//PTR_ByteColor_ARRAY_00448aa0[g_CharacterIndex_00449614].g = g;
			//PTR_ByteColor_ARRAY_00448aa0[g_CharacterIndex_00449614].b = b;
			//PTR_ByteColor_ARRAY_00448aa0[g_CharacterIndex_00449614].a = a;
			//if (((g_Mode_0044966c == 0xb) || (g_Mode_0044966c == 0xc)) && (g_CharacterIndex_00449614 < DAT_00449618)) {
			//	g_CharacterIndex_00449614 = g_CharacterIndex_00449614 + 1;
			//}
			//if (((g_DisplayListEnd_004495d8->flags_0x0 & 2) == 0) && (g_Mode_0044966c == 7)) {
			//	g_pByteColour_0044960c->r = PTR_ByteColor_ARRAY_00448aa0->r;
			//	g_pByteColour_0044960c->g = PTR_ByteColor_ARRAY_00448aa0->g;
			//	g_pByteColour_0044960c->b = PTR_ByteColor_ARRAY_00448aa0->b;
			//	g_pByteColour_0044960c->a = PTR_ByteColor_ARRAY_00448aa0->a;
			//	g_pByteColour_0044960c[1].r = PTR_ByteColor_ARRAY_00448aa0->r;
			//	g_pByteColour_0044960c[1].g = PTR_ByteColor_ARRAY_00448aa0->g;
			//	g_pByteColour_0044960c[1].b = PTR_ByteColor_ARRAY_00448aa0->b;
			//	g_pByteColour_0044960c[1].a = PTR_ByteColor_ARRAY_00448aa0->a;
			//	g_pByteColour_0044960c = g_pByteColour_0044960c + 2;
			//}
			//if ((g_DisplayListEnd_004495d8->flags_0x0 & 2) != 0) {
			//	ColourCommandSetup_002c9fd0();
			//	if (g_DrawEnded_004496c0 == '\0') {
			//		pRVar2 = AddDisplayListInternal_002ca4c0(g_DisplayListEnd_004495d8->pRenderCommands);
			//		g_DisplayListEnd_004495d8->pRenderCommands = pRVar2;
			//	}
			//	pDVar1 = g_DisplayListEnd_004495d8;
			//	g_DisplayListEnd_004495d8->pRenderCommands->cmdA =
			//		((long)(char)a & 0xffU) << 0x18 |
			//		((long)(char)b & 0xffU) << 0x10 | (long)(char)r & 0xffU | ((long)(char)g & 0xffU) << 8 | 0x3f80000000000000;
			//	pDVar1->pRenderCommands->cmdB = 1;
			//	pDVar1->pRenderCommands = pDVar1->pRenderCommands + 1;
			//}
		}
		else {
			(g_pDisplayListColurFunction_004496ec)(r, g, b, a);
		}
		return;
	}

	void CallpcGpffff9200(float param_1, float param_2)
	{
		/* WARNING: Could not recover jumptable at 0x002cdff4. Too many branches */
		/* WARNING: Treating indirect jump as call */
		(g_pDisplayListFunction_004496f0)(param_1, param_2);
		return;
	}

	void CallpcGpffff91f8(float x, float y, float z, int param_4)
	{
		/* WARNING: Could not recover jumptable at 0x002cd6e4. Too many branches */
		/* WARNING: Treating indirect jump as call */
		(g_pDisplayListFunction_004496e8)(x, y, z, param_4);
		return;
	}

	void EndDraw_002cfe40(void)
	{
		ushort uVar1;
		RenderCommand* pRVar2;
		uint uVar3;
		ulong uVar4;
		DisplayListInternalSubObj_60* pDVar5;

		if (g_DrawEnded_004496c0 != '\0') {
			if ((g_DisplayListEnd_004495d8->flags_0x0 & 1) == 0) {
				if ((g_DisplayListEnd_004495d8->flags_0x0 & 2) != 0) {
					pRVar2 = g_DisplayListEnd_004495d8->pRenderCommands;
					uVar3 = *(uint*)&g_CachedRenderCommandPtr_00449650->cmdA;
					*(undefined4*)&g_CachedRenderCommandPtr_00449650->cmdA = 0;
					*(uint*)&g_CachedRenderCommandPtr_00449650->cmdA =
						uVar3 & 0xffff8000 | ((uint)((int)pRVar2 - (int)g_CachedRenderCommandPtr_00449650) >> 4) - 1 & 0x7fff;
				}
			}
			else {
				if (g_DisplayListEnd_004495d8->subCommandBufferCount != 0) {
					if (g_Mode_0044966c == 10) {
						assert(false);
						//CallpcGpffff91f8(Vector3_0048d390.x, Vector3_0048d390.y, Vector3_0048d390.z, 10);
					}
					//DAT_0044935c = 0x48;
					INT_00449654 = 0;
					g_CachedRenderCommandPtr_00449650 = (RenderCommand*)0x0;
					pDVar5 = g_DisplayListEnd_004495d8->pDisplayListInternalSubObj +
						(g_DisplayListEnd_004495d8->subCommandBufferCount - 1);
					pDVar5->field_0x48 = (uint)PTR_MeshDrawRenderCommand_004495fc;
					uVar1 = pDVar5->type_0x42;
					if (uVar1 != 9) {
						if ((((uVar1 == 0xc) || (uVar1 == 0xb)) || (uVar1 == 7)) || (uVar1 == 6)) {
							if (g_Count_004495f4 < 1) {
								g_DisplayListEnd_004495d8->subCommandBufferCount = g_DisplayListEnd_004495d8->subCommandBufferCount - 1;
								PTR_MeshDrawRenderCommand_004495fc = (MeshDrawRenderCommand*)pDVar5->pRenderInput;
							}
							else {
								assert(false);
								//FUN_002cf640((int)pDVar5->pRenderInput);
								//PTR_MeshDrawRenderCommand_004495fc =
								//	(MeshDrawRenderCommand*)
								//	FUN_002a3240((uint*)pDVar5->pRenderInput, (undefined4*)PTR_MeshDrawRenderCommand_004495fc,
								//		(int)g_DisplayListPtr_0044965c, (ulong)pDVar5->type_0x42);
							}
						}
						else {
							if (((uVar1 == 4) || (uVar1 == 2)) || (uVar1 == 0)) {
								if (g_Count_004495f4 < 2) {
									g_DisplayListEnd_004495d8->subCommandBufferCount = g_DisplayListEnd_004495d8->subCommandBufferCount - 1;
									PTR_MeshDrawRenderCommand_004495fc = (MeshDrawRenderCommand*)pDVar5->pRenderInput;
								}
								else {
									assert(false);
									//pDVar5->field_0x50 = g_Count_004495f4;
									//FUN_002cf500(pDVar5->pRenderInput);
									//uVar4 = FUN_002cfb10((long)(int)pDVar5->pRenderInput, (long)(int)PTR_MeshDrawRenderCommand_004495fc);
									//PTR_MeshDrawRenderCommand_004495fc = (MeshDrawRenderCommand*)uVar4;
								}
							}
						}
					}
					g_DisplayListEnd_004495d8->pRenderCommands = (RenderCommand*)PTR_MeshDrawRenderCommand_004495fc;
				}
			}
			g_Count_004495f4 = 0;
			//DAT_0044963c = 0;
			g_CachedRenderCommandPtr_00449650 = (RenderCommand*)0x0;
			g_DrawEnded_004496c0 = '\0';
			g_pDisplayListFunction_004496e8 = (DisplayListFourFloats)0x0;
			g_pDisplayListColurFunction_004496ec = (DisplayListColourFunction)0x0;
		}
		return;
	}
}
