#ifndef VIDEO_A_H
#define VIDEO_A_H

#include "Types.h"

struct VidModeData_30* GetVidModeData30_001ba9c0(void);
struct VidModeData_30* GetVidModeData30_001ba9d0(void);
void CleanupFunc_001ba9e0(void);
void SetupVidParams_001baa30(short omode, short screenWidth, short screenHeight, short vidMode1, int param_5);

#endif //VIDEO_A_H
