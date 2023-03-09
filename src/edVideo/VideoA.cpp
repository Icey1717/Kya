#include "VideoA.h"
#include "VideoB.h"
#include "edSystem.h"
#include "VideoC.h"

VidModeData_30* PTR_VidModeData_30_00448ee8 = NULL;
VidModeData_30* PTR_VidModeData_30_00448eec = NULL;

VidModeData_30* GetVidModeData30_001ba9c0(void)
{
	return PTR_VidModeData_30_00448ee8;
}

VidModeData_30* GetVidModeData30_001ba9d0(void)
{
	return PTR_VidModeData_30_00448eec;
}

void CleanupFunc_001ba9e0(void)
{
	if (PTR_VidModeData_30_00448ee8 != (VidModeData_30*)0x0) {
		//FreeObject_002bbd40(PTR_VidModeData_30_00448ee8);
		PTR_VidModeData_30_00448ee8 = (VidModeData_30*)0x0;
	}
	if (PTR_VidModeData_30_00448eec != (VidModeData_30*)0x0) {
		//FreeObject_002bbd40(PTR_VidModeData_30_00448eec);
		PTR_VidModeData_30_00448eec = (VidModeData_30*)0x0;
	}
	return;
}

void SetupVidParams_001baa30(short omode, short screenWidth, short screenHeight, short vidMode1, int param_5)
{
	VidParams8* pVVar1;
	VidModeData local_30;
	VidParams26 local_20;

	local_20.field_0xf = 1;
	local_20.isNTSC = (ushort)g_isNTSC;
	local_20.inter = 1;
	local_20.field_0xb = 0;
	local_20.field_0xc = 0;
	local_20.field_0xd = 0;
	local_20.field_0xe = 1;
	local_20.field_0x10[0] = '\0';
	local_20.screenWidth = screenWidth;
	local_20.screenHeight = screenHeight;
	local_20.vidMode1 = vidMode1;
	local_20.omode = omode;
	SetupPCRTC_002b9b80(&local_20);
	if (PTR_VidModeData_30_00448ee8 != (VidModeData_30*)0x0) {
		//FreeObject_002bbd40(PTR_VidModeData_30_00448ee8);
	}
	local_30.mode_0x4 = 0;
	local_30.field_0x6 = 2;
	local_30.field_0x8 = 0x24;
	local_30.field_0xa = '\0';
	local_30.screenWidth = screenWidth;
	local_30.screenHeight = screenHeight;
	pVVar1 = GetVidParams8_002b9e60();
	if (pVVar1->field_0x1 == 0) {
		local_30.count_0xb = 1;
	}
	else {
		local_30.count_0xb = 2;
	}
	PTR_VidModeData_30_00448ee8 = AllocateVidModeData30_002bbb90(&local_30);
	//if (PTR_VidModeData_30_00448eec != (VidModeData_30*)0x0) {
	//	FreeObject_002bbd40(PTR_VidModeData_30_00448eec);
	//}
	if (param_5 == 2) {
		local_30.mode_0x4 = 2;
	}
	else {
		local_30.mode_0x4 = 0;
	}
	local_30.field_0x6 = 2;
	local_30.field_0x8 = 8;
	local_30.field_0xa = '\0';
	local_30.count_0xb = 1;
	local_30.screenWidth = screenWidth;
	local_30.screenHeight = screenHeight;
	PTR_VidModeData_30_00448eec = AllocateVidModeData30_002bbb90(&local_30);
	//FUN_002b9e00(0, 0, 0);
	//FUN_002b9dd0((short)g_SetOffsetX, (short)g_SetOffsetY);
	return;
}
