#ifndef CAMERA_H
#define CAMERA_H

#include "Types.h"
#include "Rendering/DisplayList.h"

struct edSurface;

struct CameraObjParams {
	short posX;
	short posY;
	short screenWidth;
	short screenHeight;
};

struct CameraObj_390 {
	edpkt_data commandBuffer[56];
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

struct ed_viewport {
	short posX;
	short posY;
	short screenWidth;
	short screenHeight;
	undefined4 field_0x8;
	undefined4 field_0xc;
	edSurface* pColorBuffer;
	edSurface* pZBuffer;
	ByteColor clearColor;
	uint fbMask;
	uint clearMask;
	CameraObj_390* pCameraObj390_0x24;
};

ed_viewport* edViewportNew(CameraObjParams* pParams, edSurface* pVidModeDataA, edSurface* pVidModeDataB, byte alpha);
void edViewportUpdate(ed_viewport* pCamera, CameraObjParams* pParams, edSurface* pColorBuffer, edSurface* pZBuffer, byte alpha);
void edViewportDel(ed_viewport* pCamera, bool bDestroyFrameBuffers);
edpkt_data* edViewportUpdateEnv(ed_viewport* pCamera, edpkt_data* pCommandBuf);
void edViewPortApplyDrawingEnv(ed_viewport* pCamera);
void edViewportSetBackgroundColor(ed_viewport* pCamera, byte r, byte g, byte b);
void edViewportSetClearMask(ed_viewport* pCamera, uint param_2);

#endif //CAMERA_H
