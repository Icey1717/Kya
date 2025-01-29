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

typedef edCSysHandlerSystem<11, &edSysHandlerMainPool, 2> edSysHandlerVideo;

void _UpdateVideoInfo(void);
void edVideoFlip(void);
struct edSurface* edVideoGetDisplaySurface(void);
edVideo_Globals* edVideoGetInfo(void);
void edVideoWaitVsync(byte param_1);
unsigned int _VideoTimerGet(void);

extern edSysHandlerVideo edVideoHandlers;

#endif //VIDEO_D_H
