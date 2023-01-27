#include "edDlist.h"
#include "edMem.h"
#include <string.h>
#include <stdio.h>
#include <assert.h>

#ifdef PLATFORM_PS2
#include <libvu0.h>
#endif

#include "ed3D.h"
#include "Rendering/DisplayList.h"

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
	sceVu0FMATRIX* g_DisplayListMatrix_004595e4;

	DisplayList_0x10* g_DisplayListPtr_0044965c;

	RenderCommand* g_CachedRenderCommandPtr_00449650 = NULL;

	int MaterialDisplayListCount_00449648;

	int g_DrawEnded_004496c0 = 0;

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

	void Init(void)
	{
		//DisplayListArray* pDVar1;
		//CameraPanMasterHeader* pCVar2;
		//uint uVar3;
		//
		//g_DisplayListGlobal_004495a8 = 0;
		//g_DisplayListPtr_004495b0 = (undefined*)0x0;
		//DAT_004495b4 = 0;
		//g_DisplayListMemStart_004595b8 = (undefined*)0x0;
		g_DisplayListMemAllocPtr_004495bc = (char*)0x0;
		g_DisplayListMemEnd_004595c0 = (char*)0x0;	
		//g_DisplayListPtr_004495d0 = (undefined*)0x0;
		g_DisplayListCommandCount_004495d4 = 0;
		g_DisplayListEnd_004495d8 = (DisplayListInternal*)0x0;
		g_DisplayListMatrixCount_004495e0 = 0;
		//DAT_004495ec = 0;
		//DAT_004495f0 = 0;
		g_DisplayListMatrix_004595e4 = (sceVu0FMATRIX*)0x0;
		//PTR_CameraObj_28_004495e8 = (undefined*)0x0;
		//DAT_004495f4 = 0;
		//PTR_DAT_004495fc = (undefined*)0x0;
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
		//DAT_00449654 = 0;
		g_DisplayListMode_00449658 = 0;
		//DAT_00449660 = 0;
		g_DisplayListGlobal_00449664 = 0;
		//g_DisplayListGlobal_00449668 = 0;
		//DAT_0044966c = 0;
		MaterialDisplayListCount_00449648 = 0;
		//DAT_00449670 = 0;
		//DAT_00449674 = 0;
		//DAT_00449678 = 0;
		//DAT_0044967c = 0;
		//DAT_00449680 = 0;
		//DAT_00449684 = 0;
		//DAT_004496e8 = 0;
		//DAT_004496ec = 0;
		//PTR_FUN_004496f0 = (undefined*)0x0;
		//DAT_00449698 = 0;
		//DAT_0044969c = 0;
		//DisplayListData_004496a0.LoadedDisplayListCount[0] = 0;
		//DisplayListData_004496a0.LoadedDisplayListCount[1] = 0;
		//memset(&DAT_0048db80, 0, 0xb0);
		AllocateDisplayListMem_002ce4c0();
		g_DisplayListPtr_0044965c = (DisplayList_0x10*)ProvisionDisplayListMemory_002ce490(g_DisplayListObjCount_004250e0 & 0xfffffff);
		//g_DisplayListPtr_004495b0 = ProvisionDisplayListMemory_002ce490(100);
		//g_DisplayListPtr_004495d0 = ProvisionDisplayListMemory_002ce490((uint)g_DisplayListSizeA_004250dc >> 4);
		//uVar3 = 0;
		//do {
		//	pDVar1 = (DisplayListArray*)ProvisionDisplayListMemory_002ce490((uint)(g_DisplayListSizeBCount_004250d0 << 2) >> 4);
		//	DisplayListData_004496a0.pDisplayListArray[uVar3] = pDVar1;
		//	pCVar2 = AllocateCameraAndMesh_00290a10(1, g_DisplayListCount_004250d4, 1);
		//	DisplayListData_004496a0.pCameraPanPairA[uVar3] = pCVar2;
		//	pCVar2 = AllocateCameraAndMesh_00290a10(1, g_DisplayListCount_004250d4, 1);
		//	DisplayListData_004496a0.pCameraPanPairB[uVar3] = pCVar2;
		//	FUN_002909f0(DisplayListData_004496a0.pCameraPanPairA[uVar3], 2, FUN_002ce480);
		//	FUN_002909f0(DisplayListData_004496a0.pCameraPanPairB[uVar3], 2, FUN_002ce480);
		//	uVar3 = uVar3 + 1;
		//} while (uVar3 < 2);
		g_DisplayListMatrix_004595e4 = (sceVu0FMATRIX*)ProvisionDisplayListMemory_002ce490((uint)(g_DisplayListMatrixCount_004250d8 << 6) >> 4);
		g_DisplayListGlobal_00449664 = 1;
		g_DisplayListMatrixCount_004495e0 = 0;
		//g_DisplayListGlobal_00449668 = 0;
		//edSysHandlers::edSysHandlersAdd(sysHandler_0048cb90.pNodeTable, sysHandler_0048cb90.pTypeArray_0x4, PoolEntryIdentifier_0048cba8, New_Name, RenderProcessEvent, 1, 0);
		//edSysHandlers::edSysHandlersAdd(sysHandler_0048cb90.pNodeTable, sysHandler_0048cb90.pTypeArray_0x4, PoolEntryIdentifier_0048cba8, New_Name_(1), RenderProcessEvent, 1, 0);
		//edSysHandlers::edSysHandlersAdd(edSysHandlersNodeParent_0048cee0.pNodeTable, edSysHandlersNodeParent_0048cee0.pTypeArray_0x4, g_SysHandlersMainMaxEventID_0048cf10, New_Name_(2), RenderProcessEvent, 1, 0);
		//edSysHandlers::edSysHandlersAdd(edSysHandlersNodeParent_0048cee0.pNodeTable, edSysHandlersNodeParent_0048cee0.pTypeArray_0x4, g_SysHandlersMainMaxEventID_0048cf10, New_Name, RenderProcessEvent, 1, 0);
		//edSysHandlers::edSysHandlersAdd(edSysHandlersNodeParent_0048cee0.pNodeTable, edSysHandlersNodeParent_0048cee0.pTypeArray_0x4, g_SysHandlersMainMaxEventID_0048cf10, New_Name_(10), RenderProcessEvent, 1, 0)
		//	;
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

	void FUN_0029f1e0(byte* param_1, ulong param_2, uint param_3)
	{
		if (param_2 < *param_1) {
			*(uint*)(*(int*)(param_1 + (int)param_2 * 4 + 0x10) + 0x10) =
				*(uint*)(*(int*)(param_1 + (int)param_2 * 4 + 0x10) + 0x10) | param_3;
		}
		return;
	}

	void FUN_002ca8c0(uint mode)
	{
		if ((((g_DrawEnded_004496c0 == '\0') &&
			(g_DisplayListMode_00449658 = mode & 0xff,
				edDlist::g_pMaterialInfo_00449644 != (MaterialInfo*)0x0)) &&
			((byte*)edDlist::g_pMaterialInfo_00449644->matSubsectionStart != (byte*)0x0)) &&
			(g_DisplayListMode_00449658 == 1)) {
			FUN_0029f1e0((byte*)edDlist::g_pMaterialInfo_00449644->matSubsectionStart, 0, 4);
		}
		return;
	}

	int LoadFromMaterialData_002cbad0(char* pMaterialSubSection, int offset, bool* bSuccess, int* pOutAddr)
	{
		int iVar1;
		int iVar2;
		int iVar3;

		*bSuccess = false;
		*pOutAddr = 0;
		if (pMaterialSubSection != (char*)0x0) {
			if (*pMaterialSubSection == '\0') {
				return 0;
			}
			iVar1 = *(int*)(pMaterialSubSection + offset * 4 + 0x10);
			if (((iVar1 != -0x10) && (iVar1 != -0x10)) && (*(short*)(iVar1 + 0x2c) != 0)) {
				iVar2 = *(int*)(iVar1 + 0x30);
				iVar3 = 0;
				if (iVar2 == -0x10) {
					return 0;
				}
				if (*(int*)(iVar2 + 0x20) != 0) {
					*bSuccess = true;
					iVar1 = *(int*)(iVar2 + (uint) * (ushort*)(iVar1 + 0x2e) * 0x10 + 0x38);
					if (iVar1 != 0) {
						iVar3 = *(int*)(iVar1 + 8) + 0x10;
					}
					if (*(int*)(iVar2 + 0x18) == 0) {
						return iVar3;
					}
					*pOutAddr = *(int*)(*(int*)(iVar2 + 0x18) + 8) + 0x10;
					return iVar3;
				}
				if (*(int*)(iVar2 + 0x18) == 0) {
					return 0;
				}
				return *(int*)(*(int*)(iVar2 + 0x18) + 8) + 0x10;
			}
		}
		return 0;
	}

	void LoadMaterialResource_002cb850(::MaterialInfo* pMaterialInfo)
	{
		RenderCommand* pRVar1;
		RenderCommand* pRVar2;
		int iVar3;
		RenderCommand* pRVar4;
		RenderCommandUint* pDVar4;
		DisplayListInternalSubObj_60* pDVar5;
		RenderCommandUint* pDVar6;
		int local_8;
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
					local_8 = 0;
					iVar3 = LoadFromMaterialData_002cbad0(pMaterialInfo->matSubsectionStart, 0, &local_1, &local_8)
						;
					pDVar6 = (RenderCommandUint*)pDVar5;
					if (iVar3 != 0) {
						pDVar5->aCommandArray[0].pCommandBuffer = (RenderCommand*)(*(int*)(iVar3 + 8) + 0x40);
						pDVar5->aCommandArray[0].type = 2;
						if ((local_1 != false) &&
							(iVar3 = (uint) * (ushort*)local_8 * (uint) * (ushort*)(local_8 + 2), 0x3ffff < iVar3)) {
							pRVar2 = pDVar5->aCommandArray[0].pCommandBuffer;
							pRVar4 = pRVar2 + 0x12;
							if (iVar3 == 0x40000) {
								pRVar4 = pRVar2 + 10;
							}
							*(uint*)((int)&pRVar4->cmdA + 4) =
								*(uint*)((int)&pRVar4->cmdA + 4) & 0xffffc000 | 0x800;
						}
						pDVar6 = pDVar5->aCommandArray + 1;
						g_DisplayListEnd_004495d8->subCommandBufferCount =
							g_DisplayListEnd_004495d8->subCommandBufferCount + 1;
					}
					pDVar6->pCommandBuffer =
						(RenderCommand*)(*(int*)(pMaterialInfo->matSubsectionStart + 8) + 0x10);
					if ((local_1 != false) &&
						(0x3ffff < (int)((uint) * (ushort*)local_8 * (uint) * (ushort*)(local_8 + 2)))) {
						*(uint*)((int)&pDVar6->pCommandBuffer[1].cmdA + 4) =
							*(uint*)((int)&pDVar6->pCommandBuffer[1].cmdA + 4) & 0xfffc001f | 0x10000;
					}
					pDVar6->type = 5;
					pDVar6->size = *(int*)(pMaterialInfo->matSubsectionStart + 0xc) * 0x10 - 0x10;
					g_DisplayListEnd_004495d8->subCommandBufferCount =
						g_DisplayListEnd_004495d8->subCommandBufferCount + 1;
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
				g_DisplayListEnd_004495d8->subCommandBufferCount =
					g_DisplayListEnd_004495d8->subCommandBufferCount + 1;
			}
		}
		pDVar5->aCommandArray[0].pCommandBuffer = pRVar1;
	LAB_002cba4c:
		pDVar5->aCommandArray[0].type = 0;
		pDVar5->aCommandArray[0].size = 0;
		g_DisplayListEnd_004495d8->subCommandBufferCount =
			g_DisplayListEnd_004495d8->subCommandBufferCount + 1;
		if ((g_DisplayListEnd_004495d8->subCommandBufferCount == 1) ||
			(g_DisplayListEnd_004495d8->subCommandBufferCount == 3)) {
			assert(false);
#if 0
			RenderCommand_002ca6a0(1, 7, 0, 0, 0, 0, 1, 1);
			RenderCommand_002ca930(g_Camera_004495e8);
#endif
		}
		return;
	}
}
