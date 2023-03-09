#include "VideoC.h"
#include "VideoB.h"
#include "edMem.h"
#include <string.h>
#include "VideoD.h"

#ifdef PLATFORM_PS2
#include <libgraph.h>
#endif

// Not here

void FUN_002b98f0(VidModeData_30* param_1)
{
	int iVar1;
	int iVar2;

	iVar2 = (0x800 - ((int)(uint)param_1->pVidModeData_0x0->screenWidth >> 1)) * 0x10;
	iVar1 = (0x800 - ((int)(uint)param_1->pVidModeData_0x0->screenHeight >> 1)) * 0x10;
	*g_ActiveVidParams_0048cd90.listDataAC.field_0xa4 = (long)iVar2 & 0xffffffffU | ((long)iVar1 & 0xffffffffU) << 0x10;
	*g_ActiveVidParams_0048cd90.listDataAC.field_0xa8 =
		(long)(int)(iVar2 + (uint)param_1->pVidModeData_0x0->screenWidth * 0x10) & 0xffffffffU |
		((long)(int)(iVar1 + (uint)param_1->pVidModeData_0x0->screenHeight * 0x10) & 0xffffffffU) << 0x10;
	return;
}



void AllocateVidModeData20_002b8a30(VidModeData_30* param_1, byte param_2)
{
	VidModeData_20* pVVar1;

	pVVar1 = (VidModeData_20*)edMemAlloc(1, 0x20);
	param_1->pVidModeData20 = pVVar1;
	ulong dispfb2 =
		(ulong)g_VidParams8_00449588.field_0x1 << 0x2b |
		(ulong)param_1->pVidModeData_0x0->mode_0x4 << 0xf |
		(ulong)param_1->size_0x8 | (long)((int)(uint)param_1->pVidModeData_0x0->screenWidth >> 6) << 9;
	param_1->pVidModeData20->dispfb2 = *(tGS_DISPFB2*)&dispfb2;
	ulong dispfb1 =
		(ulong)param_1->pVidModeData_0x0->mode_0x4 << 0xf |
		(ulong)param_1->size_0x8 | (long)((int)(uint)param_1->pVidModeData_0x0->screenWidth >> 6) << 9;
	param_1->pVidModeData20->dispfb1 = *(tGS_DISPFB1*)&dispfb1;
	param_1->pVidModeData20->field_0x10 = param_2;
	FUN_002b98f0(param_1);
	return;
}

void AllocateVidModeData30_30_002b8af0(VidModeData_30* param_1)
{
	VideModeData_30_30* pVVar1;
	int iVar2;

	pVVar1 = (VideModeData_30_30*)edMemAlloc(1, 0x30);
	param_1->pVidModeData30_30 = pVVar1;
	pVVar1->field_0x0 = 0x1000000000008002;
	pVVar1->field_0x8 = 0xe;
	iVar2 = GetZbufferData_002bbee0((uint)param_1->pVidModeData_0x0->mode_0x4);
	pVVar1->field_0x10 = (ulong)param_1->size_0x8 | ((long)iVar2 & 0xffffffffU) << 0x18;
	pVVar1->field_0x18 = 0x4e;
	pVVar1->field_0x20 = 0x70000;
	pVVar1->field_0x28 = 0x47;
	return;
}

int GetCanInterlace_002b8b90(void)
{
	int iVar1;

	iVar1 = 0;
	if (g_ActiveVidParams_0048cd90.params26.inter != 0) {
		if ((g_ActiveVidParams_0048cd90.params26.omode == 2) && (0xe0 < g_ActiveVidParams_0048cd90.params26.screenHeight)) {
			iVar1 = 1;
		}
		else {
			if ((g_ActiveVidParams_0048cd90.params26.omode == 3) && (0x100 < g_ActiveVidParams_0048cd90.params26.screenHeight)
				) {
				iVar1 = 1;
			}
		}
	}
	return iVar1;
}

#ifdef PLATFORM_PS2
tGS_DISPLAY2 tGS_DISPLAY2_0048cdd0 = { 0 };
tGS_DISPLAY1 tGS_DISPLAY1_0048cdd8 = { 0 };
#endif

void SetupDisplayGlobals_002b8c00(void)
{
#ifdef PLATFORM_PS2
	ulong uVar1;

	if (g_ActiveVidParams_0048cd90.params26.omode == 2) {
		if (g_ActiveVidParams_0048cd90.params26.inter == 0) {
			ulong dispfb2 = ((ulong)(ushort)g_ActiveVidParams_0048cd90.params18.field_0x8 << 0x2c |
					(ulong)(ushort)g_ActiveVidParams_0048cd90.params18.field_0xa << 0x17 |
					(long)(g_ActiveVidParams_0048cd90.params18.field_0x4 + 0x27c) | (long)(g_ActiveVidParams_0048cd90.params18.field_0x6 + 0x19) << 0xc |
					0x9ff00000000);
			tGS_DISPLAY2_0048cdd0 = *(tGS_DISPLAY2*)&dispfb2;
		}
		else {
			uVar1 = (ulong)(ushort)g_ActiveVidParams_0048cd90.params18.field_0xa << 0x17 |
				(long)(g_ActiveVidParams_0048cd90.params18.field_0x4 + 0x27c) | (long)(g_ActiveVidParams_0048cd90.params18.field_0x6 + 0x32) << 0xc |
				0x9ff00000000;
			ulong dispfb2 = (uVar1 | (long)(int)((uint)(ushort)g_ActiveVidParams_0048cd90.params18.field_0x8 - (uint)g_VidParams8_00449588.field_0x1) <<
				0x2c);
			tGS_DISPLAY2_0048cdd0 = *(tGS_DISPLAY2*)&dispfb2;
			ulong dispfb1 = ((ulong)(ushort)g_ActiveVidParams_0048cd90.params18.field_0x8 << 0x2c | uVar1);
			tGS_DISPLAY1_0048cdd8 = *(tGS_DISPLAY1*)&dispfb1;
		}
	}
	else {
		if (g_ActiveVidParams_0048cd90.params26.inter == 0) {
			ulong dispfb2 = ((ulong)(ushort)g_ActiveVidParams_0048cd90.params18.field_0x8 << 0x2c |
					(ulong)(ushort)g_ActiveVidParams_0048cd90.params18.field_0xa << 0x17 |
					(long)(g_ActiveVidParams_0048cd90.params18.field_0x4 + 0x290) | (long)(g_ActiveVidParams_0048cd90.params18.field_0x6 + 0x24) << 0xc |
					0x9ff00000000);
			tGS_DISPLAY2_0048cdd0 = *(tGS_DISPLAY2*)&dispfb2;
		}
		else {
			uVar1 = (ulong)(ushort)g_ActiveVidParams_0048cd90.params18.field_0xa << 0x17 |
				(long)(g_ActiveVidParams_0048cd90.params18.field_0x4 + 0x290) | (long)(g_ActiveVidParams_0048cd90.params18.field_0x6 + 0x48) << 0xc |
				0x9ff00000000;
			ulong dispfb2 = (uVar1 | (long)(int)((uint)(ushort)g_ActiveVidParams_0048cd90.params18.field_0x8 - (uint)g_VidParams8_00449588.field_0x1) <<
				0x2c);
			tGS_DISPLAY2_0048cdd0 = *(tGS_DISPLAY2*)&dispfb2;
			ulong dispfb1 = ((ulong)(ushort)g_ActiveVidParams_0048cd90.params18.field_0x8 << 0x2c | uVar1);
			tGS_DISPLAY1_0048cdd8 = *(tGS_DISPLAY1*)&dispfb1;
		}
	}
#endif
	return;
}

void FUN_002b8f70(VidModeData_30* param_1)
{
	//byte bVar1;
	//CameraObjParams local_8;
	//
	//local_8.field_0x2 = 0;
	//local_8.field_0x0 = 0;
	//local_8.screenHeight = 0x200;
	//local_8.screenWidth = 0x200;
	//if (param_1 != (VidModeData_30*)0x0) {
	//	local_8.screenWidth = param_1->pVidModeData_0x0->screenWidth;
	//	local_8.screenHeight = param_1->pVidModeData_0x0->screenHeight;
	//}
	//bVar1 = 4;
	//if (VidParams26_0048cd90.field_0xc != 0) {
	//	bVar1 = 6;
	//}
	//if (VidParams26_0048cd90.field_0xb != 0) {
	//	bVar1 = bVar1 | 1;
	//}
	//if (g_CameraObj_28_0048cdf4 == (CameraObj_28*)0x0) {
	//	AllocateCameraObj28_002bae70(&local_8, param_1, (VidModeData_30*)0x0, bVar1);
	//	SetCameraObjBytes_002bb960(g_CameraObj_28_0048cdf4, 0x20, 0x20, 0x40);
	//}
	//else {
	//	FUN_002bafe0((short*)g_CameraObj_28_0048cdf4, (long)(int)&local_8, param_1, 0, bVar1);
	//	SetCameraObjBytes_002bb960(g_CameraObj_28_0048cdf4, DAT_0048cdc4, DAT_0048cdc5, DAT_0048cdc6);
	//}
	return;
}

// End Not here

int FUN_002bb980(int param_1, int param_2, int* param_3, int* param_4)
{
	int iVar1;

	if (param_2 == 3) {
		*param_3 = 0x80;
		*param_4 = 0x40;
	}
	else {
		if (param_2 == 2) {
			*param_3 = 0x40;
			*param_4 = 0x40;
		}
		else {
			if ((param_2 != 1) && (param_2 != 0)) {
				return 0;
			}
			*param_3 = 0x40;
			*param_4 = 0x20;
		}
	}
	if (param_1 == 2) {
		iVar1 = 0x100;
	}
	else {
		if ((param_1 == 8) || (param_1 == 4)) {
			iVar1 = 0x2000;
		}
		else {
			iVar1 = 0;
		}
	}
	return iVar1;
}

uint FUN_002bba40(uint param_1)
{
	uint uVar1;
	uint uVar2;

	uVar2 = 0;
	uVar1 = param_1;
	if (param_1 == 0) {
		param_1 = 0;
	}
	else {
		for (; (uVar1 != 0 && (uVar1 != 1)); uVar1 = uVar1 >> 1) {
			uVar2 = uVar2 + 1;
		}
		uVar1 = 1 << (uVar2 & 0x1f);
		if (uVar1 != param_1) {
			param_1 = uVar1 << 1;
		}
	}
	return param_1;
}

bool InitVidModeData_30_002bbaa0(VidModeData_30* param_1)
{
	ushort uVar1;
	VidModeData* pVVar2;
	int align;
	void* pvVar3;
	int iVar4;
	uint uVar5;
	uint uVar6;
	int iStack8;
	int iStack4;

	pVVar2 = param_1->pVidModeData_0x0;
	uVar5 = (uint)pVVar2->screenWidth;
	uVar6 = (uint)pVVar2->screenHeight;
	align = FUN_002bb980(pVVar2->field_0x8 & 0xf, (uint)pVVar2->mode_0x4, &iStack4, &iStack8);
	if ((param_1->pVidModeData_0x0->field_0x8 & 0x20) != 0) {
		uVar5 = FUN_002bba40(uVar5);
		uVar6 = FUN_002bba40(uVar6);
	}
	iVar4 = uVar5 * uVar6 * 2;
	uVar1 = param_1->pVidModeData_0x0->mode_0x4;
	if ((uVar1 == 0) || (uVar1 == 1)) {
		iVar4 = uVar5 * uVar6 * 4;
	}
	pvVar3 = edMemAllocAlign(4, (long)iVar4, align);
	param_1->data_0xc = (char*)pvVar3;
#ifdef PLATFORM_PS2
	pvVar3 = WillAllocate_0028ddd0((void*)param_1->data_0xc);
#endif
	param_1->size_0x8 = (int)pvVar3 / align;
	return param_1->data_0xc != 0;
}

VidModeData_30* AllocateVidModeData30_002bbb90(VidModeData* param_1)
{
	bool bVar2;
	byte bVar1;
	VidModeData* pVVar3;
	VidModeData* puVar2;
	VidModeData_30* bufferStart;
	uint uVar4;
	VidModeData_30* lVar5;
	VidModeData_30* pVVar4;
	VidModeData_30* unaff_s4;

	lVar5 = (VidModeData_30*)0x0;
	pVVar3 = (VidModeData*)edMemAlloc(1, 0x10);
	pVVar3->screenWidth = param_1->screenWidth;
	pVVar3->screenHeight = param_1->screenHeight;
	pVVar3->mode_0x4 = param_1->mode_0x4;
	pVVar3->field_0x6 = param_1->field_0x6;
	pVVar3->field_0x8 = param_1->field_0x8;
	pVVar3->field_0xa = param_1->field_0xa;
	pVVar3->count_0xb = param_1->count_0xb;
	pVVar3->pLink_0xc = param_1->pLink_0xc;
	uVar4 = 0;
	pVVar4 = (VidModeData_30*)0x0;
	if (param_1->count_0xb != 0) {
		do {
			unaff_s4 = (VidModeData_30*)edMemAlloc(1, 0x30);
			memset(unaff_s4, 0, 0x30);
			unaff_s4->pVidModeData_0x0 = pVVar3;
			if (lVar5 == (VidModeData_30*)0x0) {
				lVar5 = unaff_s4;
			}
			if ((param_1->field_0xa == '\0') && (bVar2 = InitVidModeData_30_002bbaa0(unaff_s4), bVar2 == false)) {
				edMemFree(unaff_s4);
				return (VidModeData_30*)0x0;
			}
			if ((param_1->field_0x8 & 0x20) != 0) {
				bVar1 = 0;
				if (g_ActiveVidParams_0048cd90.params18.ffmode != '\0') {
					bVar1 = (byte)uVar4 & 1;
				}
				AllocateVidModeData20_002b8a30(unaff_s4, bVar1);
			}
			if ((param_1->field_0x8 & 8) != 0) {
				AllocateVidModeData30_30_002b8af0(unaff_s4);
			}
			if (pVVar4 != (VidModeData_30*)0x0) {
				unaff_s4->pLink_0x1c = pVVar4;
			}
			uVar4 = uVar4 + 1;
			pVVar4 = unaff_s4;
		} while (uVar4 < param_1->count_0xb);
	}
	lVar5->pLink_0x1c = unaff_s4;
	if ((param_1->field_0x8 & 0x20) != 0) {
		SetActiveVidModeData_002b9b10(unaff_s4);
	}
	return unaff_s4;
}

void FreeObject_002bbd40(VidModeData_30* param_1)
{
	byte bVar1;
	VidModeData* heapID;
	VidModeData_30* pVVar2;
	uint uVar3;

	heapID = param_1->pVidModeData_0x0;
	bVar1 = heapID->count_0xb;
	if (heapID != (VidModeData*)0x0) {
		edMemFree(heapID);
	}
	uVar3 = 0;
	if (bVar1 != 0) {
		do {
			if (g_ActiveVidParams_0048cd90.data30 == param_1) {
				g_ActiveVidParams_0048cd90.data30 = (VidModeData_30*)0x0;
			}
			if (param_1->pVidModeData20 != (VidModeData_20*)0x0) {
				edMemFree(param_1->pVidModeData20);
			}
			if (param_1->pVidModeData30_30 != (VideModeData_30_30*)0x0) {
				edMemFree(param_1->pVidModeData30_30);
			}
			if ((void*)param_1->data_0xc != (void*)0x0) {
				edMemFree((void*)param_1->data_0xc);
			}
			pVVar2 = param_1->pLink_0x1c;
			edMemFree(param_1);
			uVar3 = uVar3 + 1;
			param_1 = pVVar2;
		} while (uVar3 < bVar1);
	}
	return;
}

void SetMode_002bbe20(VidModeData_30* param_1, ushort mode)
{
	uint uVar1;
	uint uVar2;

	uVar2 = (uint)param_1->pVidModeData_0x0->count_0xb;
	uVar1 = 0;
	if (uVar2 != 0) {
		do {
			param_1->pVidModeData_0x0->mode_0x4 = mode;
			if (param_1->pVidModeData20 != (VidModeData_20*)0x0) {
				ulong dispfb2 = (ulong)g_VidParams8_00449588.field_0x1 << 0x2b |
					(ulong)param_1->pVidModeData_0x0->mode_0x4 << 0xf |
					(ulong)param_1->size_0x8 | (long)((int)(uint)param_1->pVidModeData_0x0->screenWidth >> 6) << 9;

				param_1->pVidModeData20->dispfb2 = *(tGS_DISPFB2*)&dispfb2;
				ulong dispfb1 = (ulong)param_1->pVidModeData_0x0->mode_0x4 << 0xf |
					(ulong)param_1->size_0x8 | (long)((int)(uint)param_1->pVidModeData_0x0->screenWidth >> 6) << 9;
				param_1->pVidModeData20->dispfb1 = *(tGS_DISPFB1*)&dispfb1;
			}
			uVar1 = uVar1 + 1;
			param_1 = param_1->pLink_0x1c;
		} while (uVar1 < uVar2);
	}
	return;
}

int GetZbufferData_002bbee0(int mode)
{
	VidModeData_30* pVVar1;
	int iVar2;

	if (mode == 2) {
		pVVar1 = GetVidModeData_002ba360();
		iVar2 = 0x32;
		if (pVVar1->pVidModeData_0x0->mode_0x4 != 2) {
			iVar2 = 0x3a;
		}
	}
	else {
		if (mode == 1) {
			iVar2 = 0x31;
		}
		else {
			if (mode == 0) {
				pVVar1 = GetVidModeData_002ba360();
				if (pVVar1->pVidModeData_0x0->mode_0x4 == 2) {
					SetMode_002bbe20(pVVar1, 10);
				}
				iVar2 = 0x30;
			}
			else {
				iVar2 = 0x30;
				PrintString("You don't have a valid zbuffer !!!!");
			}
		}
	}
	return iVar2;
}
