#ifndef VIDEO_C_H
#define VIDEO_C_H

#include "Types.h"

// Not here
void UpdateCameraCommands_002b8f70(struct FrameBuffer* param_1);
int GetCanInterlace_002b8b90(void);
void SetupDisplayGlobals_002b8c00(void);

struct FrameBuffer* AllocateFrameBuffers_002bbb90(struct VidModeData* param_1);
int GetZBufferTextureFormat(int mode);
void DestroyFrameBuffer_002bbd40(FrameBuffer* pFrameBuffer);

#endif //VIDEO_C_H
