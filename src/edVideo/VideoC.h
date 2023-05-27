#ifndef VIDEO_C_H
#define VIDEO_C_H

#include "Types.h"

// Not here
void _VideoUpdateSystemViewport(struct edSurface* param_1);
int _HighResMode(void);
void _SetVideoEnv(void);

struct edSurface* edSurfaceNew(struct ed_surface_desc* param_1);
int edSurfaceGetZBufferBpp(int mode);
void edSurfaceDel(edSurface* pFrameBuffer);

#endif //VIDEO_C_H
