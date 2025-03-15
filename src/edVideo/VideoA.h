#ifndef VIDEO_A_H
#define VIDEO_A_H

#include "Types.h"

struct edSurface* VideoGetDisplaybuffer(void);
struct edSurface* VideoGetZbuffer(void);
void ClearVideoMode(void);
void SetVideoMode(short omode, short screenWidth, short screenHeight, short vidMode1, int param_5);
void edVideoSetBackgroundColor(byte param_1, byte param_2, byte param_3);
void edVideoSetFadeColor(byte r, byte g, byte b);
void edVideoSetFade(float time);
void edVideoSetFadeIn(uint param_1);
void edVideoSetFadeOut(uint param_1, uint param_2);
byte edVideoIsFadeActive(void);
void edVideoSetOffset(short inDX, short inDY);

#endif //VIDEO_A_H
