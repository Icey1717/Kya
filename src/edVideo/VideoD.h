#ifndef VIDEO_D_H
#define VIDEO_D_H

#include "Types.h"
#include "edSystem.h"

#ifdef PLATFORM_WIN
#include "delegate.h"
#endif

#ifdef PLATFORM_WIN
Multidelegate<>& GetOnVideoFlip();
#endif // PLATFORM_WIN

struct edVideo_Globals {
	struct edVideoData* pActiveVidParams;
	undefined* field_0x4;
};

struct edSysHandlerVideo {
	edSysHandlerVideo();
	edSysHandlersNodeTable* nodeParent;
	edSysHandlersPoolEntry* entries[11];
	int maxEventID;
	int mainIdentifier;
};

void _UpdateVideoInfo(void);
void edVideoFlip(void);
struct edSurface* edVideoGetDisplaySurface(void);
edVideo_Globals* edVideoGetInfo(void);
void edVideoWaitVsync(byte param_1);
unsigned int _VideoTimerGet(void);

extern edSysHandlerVideo edSysHandlerVideo_0048cee0;

#endif //VIDEO_D_H
