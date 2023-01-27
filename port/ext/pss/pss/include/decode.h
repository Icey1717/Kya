#pragma once

#ifdef __cplusplus
extern "C" {
#endif
int strFileOpen(char* filename);
void playPss();
char* continuePss();
int hasVideoEnded();
void destroyPss();
void strFileClose();
#ifdef __cplusplus
}
#endif