#include "DisplayList.h"
#include "edMem.h"
#include <string.h>
#include <stdio.h>

#ifdef PLATFORM_PS2
#include <eekernel.h>
#endif
#include "edDlist.h"
#include <assert.h>
#include "LargeObject.h"
#include "FrontendDisp.h"
#include "DlistManager.h"
#include "Actor.h"
#include "PoolAllocators.h"

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
	if (2.147484e+09f < fVar7 * fVar6) {
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

CGlobalDList::CGlobalDList()
{
	this->nbCommands = -1;
	this->nbMatrices = -1;
	this->field_0x10 = -1;
	this->pDisplayListInternal = (DisplayList*)0x0;
	this->bEnabled = 0;
}

CGlobalDList::CGlobalDList(int nbCommands, int nbMatrices, int inField_0x10, int flags, ed_3D_Scene* pInStaticMeshMaster)
{
	this->nbCommands = nbCommands;
	this->nbMatrices = nbMatrices;
	this->field_0x10 = inField_0x10;
	this->flags = flags;

	this->pDisplayListInternal = edDListNew(TO_HEAP(H_MAIN), flags, nbCommands, inField_0x10, nbMatrices, 0, (DisplayList*)0x0);
	edDListSetSceneUsed(this->pDisplayListInternal, pInStaticMeshMaster);
}

CGlobalDList::~CGlobalDList()
{
	if (this->pDisplayListInternal != (DisplayList*)0x0) {
		edDListDelete(this->pDisplayListInternal);
	}

	return;
}

// Should be in: D:/Projects/b-witch/DlistManager.cpp
void CGlobalDList::Init()
{
	this->bEnabled = 1;
	this->field_0x18 = 0;

	return;
}

void ed_3D_Scene::ed3DSceneRemoveFlag(uint flag)
{
	this->flags = this->flags & ~flag;
	return;
}

void ed_3D_Scene::ed3DSceneSetFlag(uint flag)
{
	this->flags = this->flags | flag;
}

void ed_3D_Scene::ed3DSceneSetFogProperty(bool bValue)
{
	if (bValue == false) {
		this->flags = this->flags & ~SCENE_FLAG_FOG_PROPERTY;
	}
	else {
		this->flags = this->flags | SCENE_FLAG_FOG_PROPERTY;
	}
	return;
}

void ed_3D_Scene::SetFlag_002a5440(bool bValue)
{
	if (bValue == false) {
		this->flags = this->flags & 0xffffff7f;
	}
	else {
		this->flags = this->flags | 0x80;
	}
	return;
}