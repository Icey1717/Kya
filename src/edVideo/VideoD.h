#ifndef VIDEO_D_H
#define VIDEO_D_H

#include "Types.h"
#include "edSystem.h"

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

void FUN_002b8f30(void);
void RefreshScreenRender(void);
struct VidModeData_30* GetVidModeData_002ba360(void);
edVideo_Globals* GetVideoParams_002ba3e0(void);

extern edSysHandlerVideo edSysHandlerVideo_0048cee0;

#endif //VIDEO_D_H
