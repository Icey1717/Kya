#ifndef VIDEO_A_H
#define VIDEO_A_H

#include "Types.h"

struct FrameBuffer* GetFrameBuffer_001ba9c0(void);
struct FrameBuffer* GetFrameBuffer_001ba9d0(void);
void CleanupFunc_001ba9e0(void);
void SetupVidParams_001baa30(short omode, short screenWidth, short screenHeight, short vidMode1, int param_5);
void FUN_002b9e00(byte param_1, byte param_2, byte param_3);
void SetVideoOffsets_002b9dd0(short inDX, short inDY);

#endif //VIDEO_A_H
