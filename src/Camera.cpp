#include "Camera.h"
#include "edVideo/VideoB.h"
#include "edMem.h"
#include <string.h>
#include <assert.h>

CameraObj_28*
AllocateCameraObj28_002bae70
(CameraObjParams* pParams, VidModeData_30* pVidModeDataA, VidModeData_30* pVidModeDataB, byte param_4)
{
	CameraObj_28* __s;
	CameraObj_390* pCVar1;
	ulong* puVar2;
	CameraObj_28* pReturnValue;
	CameraObjParams local_8;

	if (pParams == (CameraObjParams*)0x0) {
		local_8.field_0x0 = 0;
		local_8.field_0x2 = 0;
		if (pVidModeDataA == (VidModeData_30*)0x0) {
			local_8.screenWidth = 0x200;
			local_8.screenHeight = 0x200;
		}
		else {
			local_8.screenWidth = pVidModeDataA->pVidModeData_0x0->screenWidth;
			local_8.screenHeight = pVidModeDataA->pVidModeData_0x0->screenHeight;
		}
		pParams = &local_8;
	}
	__s = (CameraObj_28*)edMemAlloc(1, 0x28);
	memset(__s, 0, 0x28);
	__s->field_0x18[3] = param_4;
	__s->tint = 0;
	if ((param_4 & 1) == 0) {
		__s->field_0x20 = 0xffffffff;
	}
	else {
		__s->field_0x20 = 0;
	}
	__s->posX = pParams->field_0x0;
	__s->posY = pParams->field_0x2;
	__s->screenWidth = pParams->screenWidth;
	__s->screenHeight = pParams->screenHeight;
	__s->pVidModeData30_0x10 = pVidModeDataA;
	__s->pVidModeData30_0x14 = pVidModeDataB;
	__s->pCameraObj390_0x24 = (CameraObj_390*)0x0;
	__s->field_0x18[2] = 0x40;
	__s->field_0x18[1] = 0x40;
	__s->field_0x18[0] = 0x40;
	if ((param_4 & 4) != 0) {
		g_ActiveVidParams_0048cd90.pCamera = __s;
		pCVar1 = (CameraObj_390*)edMemAlloc(1, 0x390);
		__s->pCameraObj390_0x24 = pCVar1;
		if (__s->pVidModeData30_0x10 == (VidModeData_30*)0x0) {
			__s->pCameraObj390_0x24->field_0x380 = 0;
		}
		else {
			assert(false);
			//puVar2 = FUN_002bb110(__s, __s->pCameraObj390_0x24);
			//__s->pCameraObj390_0x24->field_0x380 = (ulong*)((int)puVar2 - (int)__s->pCameraObj390_0x24);
		}
	}
	return __s;
}

void SetCameraObjBytes_002bb960(CameraObj_28* param_1, byte param_2, byte param_3, byte param_4)
{
	param_1->field_0x18[0] = param_2;
	param_1->field_0x18[1] = param_3;
	param_1->field_0x18[2] = param_4;
	return;
}