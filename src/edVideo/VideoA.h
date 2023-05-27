#ifndef VIDEO_A_H
#define VIDEO_A_H

#include "Types.h"

struct edSurface* GetFrameBuffer_001ba9c0(void);
struct edSurface* GetFrameBuffer_001ba9d0(void);
void CleanupFunc_001ba9e0(void);
void SetVideoMode(short omode, short screenWidth, short screenHeight, short vidMode1, int param_5);
void edVideoSetBackgroundColor(byte param_1, byte param_2, byte param_3);
void edVideoSetOffset(short inDX, short inDY);

#endif //VIDEO_A_H
