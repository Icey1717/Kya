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
	struct FrameBuffer* pColorBuffer;
	struct FrameBuffer* pZBuffer;
	ByteColor clearColor;
	uint fbMask;
	uint altFbMask;
	CameraObj_390* pCameraObj390_0x24;
};

RenderCommand* BuildCameraCommands_002bb110(CameraObj_28* pCamera, RenderCommand* pCommandBuf);
CameraObj_28* AllocateCameraObj28_002bae70(CameraObjParams* pParams, struct FrameBuffer* pVidModeDataA, struct FrameBuffer* pVidModeDataB, byte param_4);
void SetCameraClear_002bb960(CameraObj_28* pCamera, byte r, byte g, byte b);
void BuildCameraCommands_002bafe0(CameraObj_28* pCamera, CameraObjParams* pParams, FrameBuffer* pColorBuffer, FrameBuffer* pZBuffer, byte alpha);
#endif //CAMERA_H
