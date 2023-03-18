#ifndef CAMERA_H
#define CAMERA_H

#include "Types.h"
#include "Rendering/DisplayList.h"

struct CameraObjParams {
	short field_0x0;
	short field_0x2;
	short screenWidth;
	short screenHeight;
};

struct CameraObj_390 {
	RenderCommand commandBuffer[56];
	int qwc;
	undefined field_0x384;
	undefined field_0x385;
	undefined field_0x386;
	undefined field_0x387;
	undefined field_0x388;
	undefined field_0x389;
	undefined field_0x38a;
	undefined field_0x38b;
	undefined field_0x38c;
	undefined field_0x38d;
	undefined field_0x38e;
	undefined field_0x38f;
};

struct CameraObj_28 {
	short posX;
	short posY;
	short screenWidth;
	short screenHeight;
	undefined4 field_0x8;
	undefined4 field_0xc;
	struct FrameBuffer* pVidModeData30_0x10;
	struct FrameBuffer* pZBuffer;
	byte field_0x18[4];
	uint fbMask;
	uint field_0x20;
	CameraObj_390* pCameraObj390_0x24;
};

RenderCommand* BuildCameraCommands_002bb110(CameraObj_28* pCamera, RenderCommand* pCommandBuf);
CameraObj_28* AllocateCameraObj28_002bae70(CameraObjParams* pParams, struct FrameBuffer* pVidModeDataA, struct FrameBuffer* pVidModeDataB, byte param_4);
void SetCameraObjBytes_002bb960(CameraObj_28* param_1, byte param_2, byte param_3, byte param_4);
void BuildCameraCommands_002bafe0(CameraObj_28* pCamera, CameraObjParams* param_2, FrameBuffer* param_3, FrameBuffer* param_4, byte param_5);
#endif //CAMERA_H
