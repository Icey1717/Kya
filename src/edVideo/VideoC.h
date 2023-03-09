#ifndef VIDEO_C_H
#define VIDEO_C_H

#include "Types.h"

// Not here
void FUN_002b8f70(struct VidModeData_30* param_1);
int GetCanInterlace_002b8b90(void);
void SetupDisplayGlobals_002b8c00(void);

struct VidModeData_30* AllocateVidModeData30_002bbb90(struct VidModeData* param_1);
int GetZbufferData_002bbee0(int mode);

#endif //VIDEO_C_H
