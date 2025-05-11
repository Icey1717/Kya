#ifndef VIDEO_D_H
#define VIDEO_D_H

#include "Types.h"
#include "EdenLib/edSys/sources/EdHandlers.h"

#ifdef PLATFORM_WIN
#include "delegate.h"
#endif

#ifdef PLATFORM_WIN
Multidelegate<>& GetOnVideoFlip();
#endif // PLATFORM_WIN

struct edVideo_Globals {
	struct ed_video_attr* pActiveVidParams;
	undefined* field_0x4;
};

typedef edCSysHandlerSystem<ED_SYSTEM_HANDLER_VIDEO, &edSysHandlerMainPool, 0xb> edSysHandlerVideo;

void _UpdateVideoInfo(void);
void edVideoFlip(void);
struct edSurface* edVideoGetDisplaySurface(void);
edVideo_Globals* edVideoGetInfo(void);
void edVideoWaitVsync(byte param_1);
unsigned int _VideoTimerGet(void);

#define ED_HANDLER_VIDEO_SWAP 0
#define ED_HANDLER_VIDEO_SYNC 3
#define ED_VIDEO_HANDLER_RENDER 6
#define ED_VIDEO_HANDLER_COMPUTE 7
#define ED_HANDLER_VIDEO_SEND_2D 10

extern edSysHandlerVideo edVideoHandlers;

#endif //VIDEO_D_H
