#include "DisplayList.h"
#include "edMem.h"
#include <string.h>
#include <stdio.h>

#ifdef PLATFORM_PS2
#include <eekernel.h>
#endif
#include "../edDlist.h"
#include <assert.h>
#include "../LargeObject.h"
#include "../FrontendManager.h"
#include "../DlistManager.h"
#include "../Actor.h"
#include "../PoolAllocators.h"

GlobalDList* g_GameDisplayListPtr_0044971c = NULL;
GlobalDList* g_FrontendDisplayListPtr_00449720 = NULL;
GlobalDList* pGuiDList = NULL;

void GuiDList_EndCurrent(void)
{
	if (pGuiDList->bEnabled != 0) {
		pGuiDList->field_0x18 = 0;
	}
	return;
}

int GameDListPatch_Register(CObject* pObject, int param_2, int param_3)
{
	CGlobalDListPatch* pCVar1;
	int iVar2;
	CGlobalDListManager* pCVar3;
	S_EYES_BRIGHT_SHADOW* pSVar4;
	int iVar5;

	pCVar3 = CScene::ptable.g_GlobalDListManager_004516bc;
	iVar5 = pObject->objectId;
	if (iVar5 == -1) {
		iVar5 = 0;
	}
	pCVar1 = (CScene::ptable.g_GlobalDListManager_004516bc)->ppGlobalDlist[iVar5].pDlistPatch;
	pSVar4 = NewPool_S_EYES_BRIGHT_SHADOW(1);
	pCVar1->pBrightEye[pCVar1->brightEyeCount] = pSVar4;
	pSVar4->field_0x0 = 0;
	pSVar4->pObject = pObject;
	pSVar4->field_0x8 = param_2;
	pSVar4->field_0xc = param_3;
	iVar2 = pCVar1->brightEyeCount;
	pCVar1->brightEyeCount = iVar2 + 1;
	pCVar3->ppGlobalDlist[iVar5].field_0x4 = pCVar3->ppGlobalDlist[iVar5].field_0x4 + param_2;
	pCVar3->ppGlobalDlist[iVar5].field_0xc = pCVar3->ppGlobalDlist[iVar5].field_0xc + param_3;
	pCVar3->ppGlobalDlist[iVar5].field_0x8 = pCVar3->ppGlobalDlist[iVar5].field_0x8 + 1;
	return iVar5 * 0x10000 + iVar2;
}

char* g_NewLine = "\n";
char* sz_DisplayListSpacer_00433970 = "---------------------------------------- - \n";

float FLOAT_00448510 = 0.1f;

void FUN_00290620(float param_1, int param_2, char* param_3, long param_4, char* param_5)
{
	int iVar1;
	char* pcVar2;
	int iVar3;
	int iVar4;
	int iVar5;
	float fVar6;
	float fVar7;
	char acStack32[32];

	iVar1 = 0;
	if (param_4 != 0) {
		if (param_1 == 0.0) {
			param_4 = 0;
		}
		else {
			if (param_1 < FLOAT_00448510) {
				for (; param_1 < 1.0; param_1 = param_1 * 10.0) {
					iVar1 = iVar1 + -1;
				}
			}
			else {
				if (10.0 <= param_1) {
					for (; 10.0 <= param_1; param_1 = param_1 * FLOAT_00448510) {
						iVar1 = iVar1 + 1;
					}
				}
				else {
					param_4 = 0;
				}
			}
		}
	}
	fVar7 = param_1;
	if (param_1 < 0.0) {
		fVar7 = -param_1;
	}
	fVar6 = 1.0;
	for (iVar3 = 0; iVar3 < param_2; iVar3 = iVar3 + 1) {
		fVar6 = fVar6 * 10.0;
	}
	if (2.147484e+09 < fVar7 * fVar6) {
		sprintf(param_3, "%s", "ERROR");
	}
	else {
		iVar4 = 0x13;
		iVar3 = (int)(fVar7 * fVar6 + 0.5);
		for (iVar5 = 0; (0 < iVar4 && ((iVar3 != 0 || (iVar5 <= param_2)))); iVar5 = iVar5 + 1) {
			param_5 = acStack32 + iVar4;
			iVar4 = iVar4 + -1;
			*param_5 = (char)iVar3 + (char)(iVar3 / 10) * -10 + '0';
			iVar3 = iVar3 / 10;
		}
		if (param_1 < 0.0) {
			*param_3 = '-';
			param_3 = param_3 + 1;
		}
		for (iVar3 = 0; iVar3 < iVar5 - param_2; iVar3 = iVar3 + 1) {
			*param_3 = *param_5;
			param_5 = param_5 + 1;
			param_3 = param_3 + 1;
		}
		*param_3 = '.';
		for (iVar3 = 0; pcVar2 = param_3 + 1, iVar3 < param_2; iVar3 = iVar3 + 1) {
			*pcVar2 = *param_5;
			param_5 = param_5 + 1;
			param_3 = pcVar2;
		}
		if (param_4 == 0) {
			*pcVar2 = '\0';
		}
		else {
			*pcVar2 = 'e';
			sprintf(param_3 + 2, "%d", iVar1);
		}
	}
	return;
}

char* WorkOutMo(float param_1, int param_2, char* param_3, long param_4, char* param_5)
{
	FUN_00290620(param_1, param_2, param_3, param_4, param_5);
	return param_3;
}

char g_DebugTextBuffer_00468ef0[16] = { 0 };

void GlobalDList_Init(void)
{
	ed_3D_Scene* pSVar1;
	int freeMemCalcA;
	GlobalDList* pDVar2;
	DisplayListInternal* pDVar3;
	int iVar4;
	int iVar5;
	uint uVar6;
	char* pcVar7;
	float fVar8;

	/* -----------------------------------------
	   ----- Memory used by display list ------- */
	edDebugPrintf(g_NewLine);
	edDebugPrintf(sz_DisplayListSpacer_00433970);
	edDebugPrintf("----- Memory used by display list -------\n");
	edDebugPrintf(g_NewLine);
	freeMemCalcA = edMemGetAvailable(TO_HEAP(H_MAIN));
	pDVar2 = new GlobalDList(0x200, 0x1000, 0, 0x11, CScene::_scene_handleA);
	g_GameDisplayListPtr_0044971c = pDVar2;
	iVar5 = edMemGetAvailable(TO_HEAP(H_MAIN));

	/* - Game list          : %07d\n */
	edDebugPrintf("- Game list          : %07d\n", freeMemCalcA - iVar5);
	iVar4 = edMemGetAvailable(TO_HEAP(H_MAIN));
	pDVar2 = new GlobalDList(0x20, 0x80, 0, 0x11, Frontend::_scene_handle);
	g_FrontendDisplayListPtr_00449720 = pDVar2;
	iVar5 = edMemGetAvailable(TO_HEAP(H_MAIN));

	/* - Frontend 3D list   : %07d\n */
	edDebugPrintf("- Frontend 3D list   : %07d\n", iVar4 - iVar5);
	iVar4 = edMemGetAvailable(TO_HEAP(H_MAIN));
	pDVar2 = new GlobalDList(0x180, 0xe74, 0, 0x12, pSVar1);
	pGuiDList = pDVar2;
	iVar5 = edMemGetAvailable(TO_HEAP(H_MAIN));
	/* - Gui list           : %07d\n */
	edDebugPrintf("- Gui list           : %07d\n", iVar4 - iVar5);
	edDebugPrintf(g_NewLine);
	iVar4 = edMemGetAvailable(TO_HEAP(H_MAIN));
	iVar5 = edMemGetAvailable(TO_HEAP(H_MAIN));
	uVar6 = freeMemCalcA - iVar5;
	if ((int)uVar6 < 0) {
		fVar8 = (float)(uVar6 >> 1 | uVar6 & 1);
		fVar8 = fVar8 + fVar8;
	}
	else {
		fVar8 = (float)uVar6;
	}
	pcVar7 = WorkOutMo(fVar8 / 1048576.0f, 3, g_DebugTextBuffer_00468ef0, 0, (char*)0x0);
	/* --- total used : \t%d (%s Mo) ----
	   ----------------------------------------- */
	edDebugPrintf("--- total used : \t%d (%s Mo) ----\n", freeMemCalcA - iVar4, pcVar7);
	edDebugPrintf(sz_DisplayListSpacer_00433970);
	edDebugPrintf(g_NewLine);
	g_GameDisplayListPtr_0044971c->Init();
	g_FrontendDisplayListPtr_00449720->Init();
	if (pGuiDList != (GlobalDList*)0x0) {
		pGuiDList->Init();
	}
	edDListSetActiveViewPort(CScene::_pinstance->pViewportA);
	return;
}

bool GuiDList_BeginCurrent(void)
{
	bool bVar1;
	GlobalDList* pDVar2;

	pDVar2 = pGuiDList;
	bVar1 = pGuiDList->bEnabled != 0;
	if (bVar1) {
		pGuiDList->field_0x18 = 1;
		edDListSetCurrent(pDVar2->pDisplayListInternal);
	}
	return bVar1;
}

void GlobalDList_AddToView(void)
{
	if (g_GameDisplayListPtr_0044971c->bEnabled != 0) {
		edDlistAddtoView(g_GameDisplayListPtr_0044971c->pDisplayListInternal);
	}
	if (g_FrontendDisplayListPtr_00449720->bEnabled != 0) {
		edDlistAddtoView(g_FrontendDisplayListPtr_00449720->pDisplayListInternal);
	}
	if ((pGuiDList != (GlobalDList*)0x0) && (pGuiDList->bEnabled != 0)) {
		edDlistAddtoView(pGuiDList->pDisplayListInternal);
	}
	return;
}

GlobalDList::GlobalDList()
{
	field_0x8 = -1;
	field_0xc = -1;
	field_0x10 = -1;
	pDisplayListInternal = (DisplayListInternal*)0x0;
	bEnabled = 0;
}

GlobalDList::GlobalDList(int inField_0x8, int inField_0xc, int inField_0x10, int inField_0x1c, ed_3D_Scene* pInStaticMeshMaster)
{
    DisplayListInternal* pDVar3;

	field_0x8 = inField_0x8;
	field_0xc = inField_0xc;
	field_0x10 = inField_0x10;
	field_0x1c = inField_0x1c;
	pDVar3 = edDListNew(TO_HEAP(H_MAIN), field_0x1c, field_0x8, 0, field_0xc, 0, (DisplayListInternal*)0x0);
	pDisplayListInternal = pDVar3;
	edDListSetSceneUsed(pDisplayListInternal, pInStaticMeshMaster);
}

void GlobalDList::Init()
{
	bEnabled = 1;
	field_0x18 = 0;
}

void ed_3D_Scene::RemoveFlag_002a53e0(uint flag)
{
	this->flags_0x4 = this->flags_0x4 & ~flag;
	return;
}

void ed_3D_Scene::ed3DSceneSetFlag(uint flag)
{
	this->flags_0x4 = this->flags_0x4 | flag;
}

void ed_3D_Scene::ed3DSceneSetFogProperty(bool bValue)
{
	if (bValue == false) {
		this->flags_0x4 = this->flags_0x4 & 0xffffffbf;
	}
	else {
		this->flags_0x4 = this->flags_0x4 | 0x40;
	}
	return;
}

void ed_3D_Scene::SetFlag_002a5440(bool bValue)
{
	if (bValue == false) {
		this->flags_0x4 = this->flags_0x4 & 0xffffff7f;
	}
	else {
		this->flags_0x4 = this->flags_0x4 | 0x80;
	}
	return;
}
